// Copyright 2023 The Cobalt Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "cobalt/debug/backend/tracing_controller.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <utility>

#include "base/bind.h"
#include "base/threading/thread.h"
#include "base/trace_event/trace_event.h"
#include "base/values.h"
#include "cobalt/script/script_debugger.h"
#include "starboard/common/string.h"
#include "v8/include/libplatform/v8-tracing.h"

namespace cobalt {
namespace debug {
namespace backend {

namespace {
// Definitions from the set specified here:
// https://chromedevtools.github.io/devtools-protocol/tot/Tracing
constexpr char kInspectorDomain[] = "Tracing";

// State parameters
constexpr char kStarted[] = "started";
constexpr char kCategories[] = "categories";

}  // namespace


///////////////// TRACE EVENT AGENT ///////////////////////////////////

void TraceEventAgent::StartAgentTracing(
    const TraceConfig& trace_config,
    StartAgentTracingCallback on_start_callback) {
  json_output_.json_output.clear();
  trace_buffer_.SetOutputCallback(json_output_.GetCallback());

  base::trace_event::TraceLog* tracelog =
      base::trace_event::TraceLog::GetInstance();
  DCHECK(!tracelog->IsEnabled());

  tracelog->SetEnabled(trace_config,
                       base::trace_event::TraceLog::RECORDING_MODE);

  std::move(on_start_callback).Run(agent_name_, true);
}

void TraceEventAgent::StopAgentTracing(
    StopAgentTracingCallback on_stop_callback) {
  base::trace_event::TraceLog* trace_log =
      base::trace_event::TraceLog::GetInstance();

  DCHECK(trace_log->IsEnabled());
  trace_log->SetDisabled(base::trace_event::TraceLog::RECORDING_MODE);

  base::Thread thread("json_outputter");
  thread.Start();

  trace_buffer_.Start();
  base::WaitableEvent waitable_event;
  auto collect_data_callback =
      base::Bind(&TraceEventAgent::CollectTraceData, base::Unretained(this),
                 base::BindRepeating(&base::WaitableEvent::Signal,
                                     base::Unretained(&waitable_event)));
  //  Write out the actual data by calling Flush().  Within Flush(), this
  //  will call OutputTraceData(), possibly multiple times.  We have to do this
  //  on a thread as there will be tasks posted to the current thread for data
  //  writing.
  thread.message_loop()->task_runner()->PostTask(
      FROM_HERE, base::BindRepeating(&base::trace_event::TraceLog::Flush,
                                     base::Unretained(trace_log),
                                     collect_data_callback, false));
  waitable_event.Wait();
  trace_buffer_.Finish();

  std::move(on_stop_callback)
      .Run(agent_name_, agent_event_label_,
           base::RefCountedString::TakeString(&json_output_.json_output));
}


void TraceEventAgent::CollectTraceData(
    base::OnceClosure finished_cb,
    const scoped_refptr<base::RefCountedString>& event_string,
    bool has_more_events) {
  trace_buffer_.AddFragment(event_string->data());
  if (!has_more_events) {
    std::move(finished_cb).Run();
  }
}

///////////////// TRACE V8 AGENT ///////////////////////////////////
TraceV8Agent::TraceV8Agent() {
  LOG(INFO) << "YO THOR - NEW TRACE v8 AGENT - CTRO!";
}

void TraceV8Agent::StartAgentTracing(const TraceConfig& trace_config,
                                     StartAgentTracingCallback callback) {
  LOG(INFO) << "YO THOR - TRACE v8 AGENT - START AGENT TRACING";
  json_output_.json_output.clear();
  trace_buffer_.SetOutputCallback(json_output_.GetCallback());

  auto* tracing_controller =
      static_cast<v8::platform::tracing::TracingController*>(
          i::V8::GetCurrentPlatform()->GetTracingController());

  auto v8_writer =
      v8::platform::tracing::TraceWriter::CreateJSONTraceWriter(v8_stream_);

  v8_buffer_ = v8::platform::tracing::TraceBuffer::CreateTraceBufferRingBuffer(
      v8::platform::tracing::TraceBuffer::kRingBufferChunks, v8_writer);

  tracing_controller->Initialize(v8_buffer_);
  // v8_tracing_->Initialize(v8_buffer_);

  auto v8_config =
      v8::platform::tracing::TraceConfig::CreateDefaultTraceConfig();
  tracing_controller->StartTracing(v8_config);

  std::move(callback).Run(agent_name_, true);
}

void TraceV8Agent::StopAgentTracing(StopAgentTracingCallback callback) {
  LOG(INFO) << "YO THOR - V8 AGENT _ SOPT TRACING";
  LOG(INFO) << "YO THOR - V8 AGENT _ FLUSH BUFFER";

  auto* tracing_controller =
      static_cast<v8::platform::tracing::TracingController*>(
          i::V8::GetCurrentPlatform()->GetTracingController());

  tracing_controller->StopTracing();

  std::string json_out = v8_stream_.str();
  LOG(INFO) << "MA STREAM!" << json_out;
  std::move(callback).Run(agent_name_, agent_event_label_,
                          base::RefCountedString::TakeString(&json_out));
}

///////////////// TRACING CONTROLLER //////////////////////////////////
TracingController::TracingController(DebugDispatcher* dispatcher,
                                     script::ScriptDebugger* script_debugger)
    : dispatcher_(dispatcher),
      tracing_started_(false),
      collected_size_(0),
      commands_(kInspectorDomain) {
  DCHECK(dispatcher_);

  commands_["end"] =
      base::Bind(&TracingController::End, base::Unretained(this));
  commands_["start"] =
      base::Bind(&TracingController::Start, base::Unretained(this));

  agents_.push_back(std::make_unique<TraceEventAgent>());
  agents_.push_back(std::make_unique<TraceV8Agent>());
}

void TracingController::Thaw(JSONObject agent_state) {
  dispatcher_->AddDomain(kInspectorDomain, commands_.Bind());
  if (!agent_state) return;

  // Restore state
  categories_.clear();
  for (const auto& category : agent_state->FindKey(kCategories)->GetList()) {
    categories_.emplace_back(category.GetString());
  }
  tracing_started_ = agent_state->FindKey(kStarted)->GetBool();
  if (tracing_started_) {
    agents_responded_ = 0;
    auto config = base::trace_event::TraceConfig();
    for (const auto& a : agents_) {
      a->StartAgentTracing(config,
                           base::BindOnce(&TracingController::OnStartTracing,
                                          base::Unretained(this)));
    }
  }
}

JSONObject TracingController::Freeze() {
  if (tracing_started_) {
    for (const auto& a : agents_) {
      a->StopAgentTracing(base::BindOnce(&TracingController::OnCancelTracing,
                                         base::Unretained(this)));
    }
  }

  dispatcher_->RemoveDomain(kInspectorDomain);

  // Save state
  JSONObject agent_state(new base::DictionaryValue());
  agent_state->SetKey(kStarted, base::Value(tracing_started_));
  base::Value::ListStorage categories_list;
  for (const auto& category : categories_) {
    categories_list.emplace_back(category);
  }
  agent_state->SetKey(kCategories, base::Value(std::move(categories_list)));

  return agent_state;
}

void TracingController::End(Command command) {
  if (!tracing_started_) {
    command.SendErrorResponse(Command::kInvalidRequest, "Tracing not started");
    return;
  }
  tracing_started_ = false;
  categories_.clear();
  command.SendResponse();

  for (const auto& a : agents_) {
    a->StopAgentTracing(base::BindOnce(&TracingController::OnStopTracing,
                                       base::Unretained(this)));
  }
}

void TracingController::Start(Command command) {
  if (tracing_started_) {
    command.SendErrorResponse(Command::kInvalidRequest,
                              "Tracing already started");
    return;
  }
  JSONObject params = JSONParse(command.GetParams());
  // Parse comma-separated tracing categories parameter.
  categories_.clear();
  std::string category_param;
  if (params->GetString("categories", &category_param)) {
    for (size_t pos = 0, comma; pos < category_param.size(); pos = comma + 1) {
      comma = category_param.find(',', pos);
      if (comma == std::string::npos) comma = category_param.size();
      std::string category = category_param.substr(pos, comma - pos);
      categories_.push_back(category);
    }
  }

  collected_events_.reset(new base::ListValue());

  agents_responded_ = 0;
  auto config = base::trace_event::TraceConfig();
  for (const auto& a : agents_) {
    a->StartAgentTracing(config,
                         base::BindOnce(&TracingController::OnStartTracing,
                                        base::Unretained(this)));
  }
  tracing_started_ = true;
  command.SendResponse();
}

void TracingController::OnStartTracing(const std::string& agent_name,
                                       bool success) {
  LOG(INFO) << "Tracing Agent:" << agent_name << " Start tracing successful? "
            << (success ? "true" : "false");
}

void TracingController::OnStopTracing(
    const std::string& agent_name, const std::string& events_label,
    const scoped_refptr<base::RefCountedString>& events_str_ptr) {
  LOG(INFO) << "Tracing Agent:" << agent_name << " Stop tracing.";

  std::unique_ptr<base::Value> root =
      base::JSONReader::Read(events_str_ptr->data(), base::JSON_PARSE_RFC);

  if (!root.get()) {
    LOG(INFO) << "YO THOR STRING IS " << events_str_ptr->data() << std::endl;
    LOG(ERROR) << "Couldn't parse the events string.";
  }

  base::ListValue* root_list = nullptr;
  root->GetAsList(&root_list);

  // Move items into our aggregate collection
  while (root_list->GetSize()) {
    if (!collected_events_) {
      collected_events_.reset(new base::ListValue());
    }
    std::unique_ptr<base::Value> item;
    root_list->Remove(0, &item);
    collected_events_->Append(std::move(item));

    if (collected_events_->GetSize() >= 100) {
      SendDataCollectedEvent();
    }
  }
  FlushTraceEvents();
}

void TracingController::OnCancelTracing(
    const std::string& agent_name, const std::string& events_label,
    const scoped_refptr<base::RefCountedString>& events_str_ptr) {
  LOG(INFO) << "Tracing Agent:" << agent_name << " Cancel tracing.";
}

void TracingController::SendDataCollectedEvent() {
  if (collected_events_) {
    std::string events;
    collected_events_->GetString(0, &events);
    JSONObject params(new base::DictionaryValue());
    // Releasing the list into the value param avoids copying it.
    params->Set("value", std::move(collected_events_));
    dispatcher_->SendEvent(std::string(kInspectorDomain) + ".dataCollected",
                           params);
    collected_size_ = 0;
  }
}

void TracingController::FlushTraceEvents() {
  SendDataCollectedEvent();
  agents_responded_++;
  if (agents_responded_ == static_cast<int>(agents_.size())) {
    dispatcher_->SendEvent(std::string(kInspectorDomain) + ".tracingComplete");
  }
}

}  // namespace backend
}  // namespace debug
}  // namespace cobalt
