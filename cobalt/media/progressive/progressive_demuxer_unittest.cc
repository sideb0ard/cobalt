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

#include <memory>

#include "base/logging.h"
#include "base/test/scoped_task_environment.h"
#include "base/threading/thread_task_runner_handle.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
// #include "base/bind.h"
// #include "base/files/file_util.h"
// #include "base/path_service.h"
#include "base/task/task_scheduler/task_scheduler.h"
#include "cobalt/media/decoder_buffer_allocator.h"
// #include "starboard/common/string.h"
// #include "starboard/event.h"
#include "cobalt/media/file_data_source.h"
#include "cobalt/media/progressive/progressive_demuxer.h"
#include "third_party/chromium/media/base/media_log.h"

constexpr char kFilePath[] =
    "file:///usr/local/google/home/sideboard/cobalt/cobalt_github/out/"
    "linux-x64x11_devel/install/usr/share/cobalt/test/cobalt/media/testing/"
    "data/progressive_aac_44100_stereo_h264_1280_720.mp4";

namespace cobalt {
namespace media {

using ::cobalt::media::ProgressiveDemuxer;
using ::media::DemuxerStream;

namespace {
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

class DemuxerHostStub : public ::media::DemuxerHost {
  void OnBufferedTimeRangesChanged(
      const ::media::Ranges<base::TimeDelta>& ranges) override {}

  void SetDuration(base::TimeDelta duration) override {}

  void OnDemuxerError(::media::PipelineStatus error) override {}
};

class ProgressiveDemuxerTest : public ::testing::Test {
 public:
  ProgressiveDemuxerTest() {
    LOG(INFO) << "YO THOR - PROG DEMUX TEST" << kFilePath;
    GURL url(kFilePath);
    if (!url.is_valid()) {
      LOG(INFO) << "NOT VALID!";
    }

    data_source_ = std::unique_ptr<DataSource>(new media::FileDataSource(url));
    if (!data_source_) {
      LOG(INFO) << "YO THOR - NAE DATA SOURCE";
    }

    demuxer_ = std::unique_ptr<ProgressiveDemuxer>(
        new ProgressiveDemuxer(task_environment_.GetMainThreadTaskRunner(),
                               data_source_.get(), &media_log_));
  }

  void SetUp() final { demuxer_init_.Reset(); }

  void OnDemuxerStatus(::media::PipelineStatus status) {
    LOG(INFO) << "YO INITIALZIE DONE";
    demuxer_init_.Signal();
  }

 protected:
  base::test::ScopedTaskEnvironment task_environment_;
  std::unique_ptr<ProgressiveDemuxer> demuxer_;
  std::unique_ptr<DataSource> data_source_;
  ::media::MediaLog media_log_;
  DemuxerHostStub demuxer_host_;
  DecoderBufferAllocator decoder_buffer_allocator_;
  base::WaitableEvent demuxer_init_ = {
      base::WaitableEvent::ResetPolicy::MANUAL,
      base::WaitableEvent::InitialState::NOT_SIGNALED};
};


TEST_F(ProgressiveDemuxerTest, ConstructAndDestroy) {
  demuxer_->Initialize(&demuxer_host_,
                       base::BindOnce(&ProgressiveDemuxerTest::OnDemuxerStatus,
                                      base::Unretained(this)));
  demuxer_init_.Wait();

  auto streams = demuxer_->GetAllStreams();
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

  demuxer_->Request(DemuxerStream::AUDIO);
  demuxer_->Request(DemuxerStream::VIDEO);

  ReadDemuxerStream(audio_stream);
  ReadDemuxerStream(video_stream);


  base::RunLoop().RunUntilIdle();
  LOG(INFO) << "YO WE OUTTA HERE";
}

}  // namespace media
}  // namespace cobalt
