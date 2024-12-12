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

#ifndef MEDIA_STARBOARD_STARBOARD_AUDIO_RENDERER_SINK_H
#define MEDIA_STARBOARD_STARBOARD_AUDIO_RENDERER_SINK_H

#include "starboard/audio_sink.h"

#include "media/base/media_export.h"

namespace media {

class MEDIA_EXPORT StarboardAudioRendererSink : public AudioRendererSink {
 public:
  StarboardAudioRendererSink();
  ~StarboardAudioRendererSink() override;

  StarboardAudioRendererSink(const StarboardAudioRendererSink&) = delete;
  StarboardAudioRendererSink& operator=(const StarboardAudioRendererSink&) =
      delete;

  // AudioRendererSink implementation.
  void Initialize(const AudioParameters& params,
                  RenderCallback* callback) override;
  void Start() override;
  void Stop() override;
  void Play() override;
  void Pause() override;
  void Flush() override;
  bool SetVolume(double volume) override;
  OutputDeviceInfo GetOutputDeviceInfo() override;
  void GetOutputDeviceInfoAsync(OutputDeviceInfoCB info_cb) override;
  bool IsOptimizedForHardwareParameters() override;
  bool CurrentThreadIsRenderingThread() override;
};

}  // namespace media

#endif  // MEDIA_STARBOARD_STARBOARD_AUDIO_RENDERER_SINK_H
