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

#ifndef COBALT_MEDIA_DECODER_BUFFER_ALLOCATOR_H_
#define COBALT_MEDIA_DECODER_BUFFER_ALLOCATOR_H_

#include <memory>

#include "base/allocator/partition_allocator/partition_alloc.h"
#include "base/compiler_specific.h"
#include "base/time/time.h"
#include "cobalt/media/decoder_buffer_memory_info.h"
#include "starboard/common/atomic.h"
#include "starboard/common/mutex.h"
#include "starboard/media.h"
#include "third_party/chromium/media/base/decoder_buffer.h"
#include "third_party/chromium/media/base/video_decoder_config.h"

namespace cobalt {
namespace media {

class DecoderBufferAllocator : public ::media::DecoderBuffer::Allocator,
                               public DecoderBufferMemoryInfo {
 public:
  DecoderBufferAllocator();
  ~DecoderBufferAllocator() override;

  void Suspend();
  void Resume();

  // DecoderBuffer::Allocator methods.
  void* Allocate(size_t size, size_t alignment) override;
  void Free(void* p, size_t size) override;

  int GetAudioBufferBudget() const override;
  int GetBufferAlignment() const override;
  int GetBufferPadding() const override;
  base::TimeDelta GetBufferGarbageCollectionDurationThreshold() const override;
  int GetProgressiveBufferBudget(SbMediaVideoCodec codec, int resolution_width,
                                 int resolution_height,
                                 int bits_per_pixel) const override;
  int GetVideoBufferBudget(SbMediaVideoCodec codec, int resolution_width,
                           int resolution_height,
                           int bits_per_pixel) const override;

  // DecoderBufferMemoryInfo methods.
  size_t GetAllocatedMemory() const override;
  size_t GetCurrentMemoryCapacity() const override;
  size_t GetMaximumMemoryCapacity() const override;

 private:
  int max_buffer_capacity_ = 0;

  base::PartitionAllocatorGeneric allocator_;
};

}  // namespace media
}  // namespace cobalt

#endif  // COBALT_MEDIA_DECODER_BUFFER_ALLOCATOR_H_
