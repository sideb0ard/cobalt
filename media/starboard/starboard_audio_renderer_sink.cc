// Copyright 2024 The Cobalt Authors. All Rights Reserved.
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

#include "media/starboard/starboard_audio_renderer_sink.h"

#include "base/logging.h"

namespace media {

StarboardAudioRendererSink::StarboardAudioRendererSink() {
  LOG(INFO) << "YO THOR YOU WASH YOU FACE IN MY SINK?";
}

StarboardAudioRendererSink::~StarboardAudioRendererSink() {
  LOG(INFO) << "YO THOR RENDERERERER SINK DTOR";
}

void StarboardAudioRendererSink::Initialize(const AudioParameters& params,
                                            RenderCallback* callback) {
  LOG(INFO) << "YO THOR RENDERERERER SINK INIT";
}

void StarboardAudioRendererSink::Start() {
  LOG(INFO) << "YO THOR RENDERERERER SINK START";
}
void StarboardAudioRendererSink::Stop() {
  LOG(INFO) << "YO THOR RENDERERERER SINK STOP";
}
void StarboardAudioRendererSink::Play() {
  LOG(INFO) << "YO THOR RENDERERERER SINK plAY";
}
void StarboardAudioRendererSink::Pause() {}
void StarboardAudioRendererSink::Flush() {}
bool StarboardAudioRendererSink::SetVolume(double volume) {}
OutputDeviceInfo StarboardAudioRendererSink::GetOutputDeviceInfo() {}
void StarboardAudioRendererSink::GetOutputDeviceInfoAsync(
    OutputDeviceInfoCB info_cb) {}
bool StarboardAudioRendererSink::IsOptimizedForHardwareParameters() {}
bool StarboardAudioRendererSink::CurrentThreadIsRenderingThread() {}

}  // namespace media
