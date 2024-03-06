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
//
// Use a full file path or one relative to base::DIR_TEST_DATA
// Example usage:
// out/linux-x64x11_devel/media_sandbox
// `pwd`/cobalt/demos/content/media-element-demo/public/assets/dash-audio.mp4
// `pwd`/cobalt/demos/content/media-element-demo/public/assets/dash-video-240p.mp4

#include <memory>

#include "base/bind.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/task/task_scheduler/task_scheduler.h"
#include "cobalt/base/wrap_main.h"
#include "cobalt/media/decoder_buffer_allocator.h"
#include "cobalt/media/file_data_source.h"
#include "cobalt/media/progressive/progressive_demuxer.h"
#include "starboard/common/string.h"
#include "starboard/event.h"
#include "third_party/chromium/media/base/media_log.h"
// #include "third_party/chromium/media/filters/chunk_demuxer.h"

namespace cobalt {
namespace media {
namespace sandbox {

namespace {

using ::cobalt::media::ProgressiveDemuxer;
using ::media::DemuxerStream;

class DemuxerHostStub : public ::media::DemuxerHost {
  void OnBufferedTimeRangesChanged(
      const ::media::Ranges<base::TimeDelta>& ranges) override {}

  void SetDuration(base::TimeDelta duration) override {}

  void OnDemuxerError(::media::PipelineStatus error) override {}
};

void OnDemuxerStatus(::media::PipelineStatus status) {
  LOG(INFO) << "YO INITIALZIE DONE";
}

const char* GetDemuxerStreamType(DemuxerStream* demuxer_stream) {
  return demuxer_stream->type() == DemuxerStream::AUDIO ? "audio" : "video";
}

void ReadDemuxerStream(DemuxerStream* demuxer_stream);

void OnDemuxerStreamRead(
    DemuxerStream* demuxer_stream, DemuxerStream::Status status,
    const std::vector<scoped_refptr<::media::DecoderBuffer>>& decoder_buffers) {
  if (status != DemuxerStream::kConfigChanged) {
    DCHECK(decoder_buffers.size() > 0);
  }
  if (!decoder_buffers[0]->end_of_stream()) {
    LOG(INFO) << "Reading " << GetDemuxerStreamType(demuxer_stream)
              << " buffer at " << decoder_buffers[0]->timestamp();
    ReadDemuxerStream(demuxer_stream);
  } else {
    LOG(INFO) << "Received " << GetDemuxerStreamType(demuxer_stream) << " EOS";
  }
}

void ReadDemuxerStream(DemuxerStream* demuxer_stream) {
  DCHECK(demuxer_stream);
  LOG(INFO) << "YO THOR _ READ DEMUXER STREAM!";
  demuxer_stream->Read(
      1, base::BindOnce(OnDemuxerStreamRead, base::Unretained(demuxer_stream)));
}

}  // namespace

int SandboxMain(int argc, char** argv) {
  if (argc != 2) {
    LOG(ERROR) << "Usage: " << argv[0] << " <progressive_path>";
    return 1;
  }

  LOG(INFO) << "YO THOR - PROGRESSIVE SANDBOX!";

  GURL url(argv[1]);
  if (!url.is_valid()) {
    LOG(INFO) << "NOT VALID!";
    return 1;
  }

  std::unique_ptr<DataSource> data_source(new media::FileDataSource(url));
  if (!data_source) {
    LOG(INFO) << "YO THOR - NAE DATA SOURCE";
    return 1;
  }

  DecoderBufferAllocator decoder_buffer_allocator;
  ::media::MediaLog media_log;

  base::MessageLoop message_loop;
  base::TaskScheduler::CreateAndStartWithDefaultParams(
      "Cobalt Sandbox TaskScheduler");

  DemuxerHostStub demuxer_host;

  std::unique_ptr<ProgressiveDemuxer> demuxer(new ProgressiveDemuxer(
      message_loop.task_runner(), data_source.get(), &media_log));

  // base::WaitableEvent waitable_event(
  //    base::WaitableEvent::ResetPolicy::MANUAL,
  //    base::WaitableEvent::InitialState::NOT_SIGNALED);

  base::WaitableEvent initialized = {
      base::WaitableEvent::ResetPolicy::MANUAL,
      base::WaitableEvent::InitialState::NOT_SIGNALED};

  // base::OnceClosure closure = base::BindOnce(
  //     [&initialized](::media::PipelineStatus status) {
  //       initialized.Signal();
  //     },
  //     &initialized);
  // demuxer->Initialize(&demuxer_host, closure);
  // demuxer->Initialize(&demuxer_host, &waitable_event);

  initialized.Wait();
  LOG(INFO) << "YO INITIALIZED?!";

  auto streams = demuxer->GetAllStreams();
  DemuxerStream* audio_stream = nullptr;
  DemuxerStream* video_stream = nullptr;
  for (auto&& stream : streams) {
    if (stream->type() == DemuxerStream::AUDIO) {
      DCHECK(!audio_stream);
      audio_stream = stream;
    } else if (stream->type() == DemuxerStream::VIDEO) {
      DCHECK(!video_stream);
      video_stream = stream;
    }
  }
  LOG(INFO) << "YO THOR - GOT STREAMS";

  // demuxer->Request(DemuxerStream::AUDIO);
  //   demuxer->Request(DemuxerStream::VIDEO);

  ReadDemuxerStream(audio_stream);
  ReadDemuxerStream(video_stream);

  base::RunLoop().RunUntilIdle();
  LOG(INFO) << "YO WE IDLE - LETS GO";

  demuxer->Stop();

  return 0;
}

}  // namespace sandbox
}  // namespace media
}  // namespace cobalt

COBALT_WRAP_SIMPLE_MAIN(cobalt::media::sandbox::SandboxMain);
