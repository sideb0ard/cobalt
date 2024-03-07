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

#include "cobalt/media/decoder_buffer_allocator.h"

#include <algorithm>
#include <vector>

#include "cobalt/math/size.h"
#include "starboard/common/allocator.h"
#include "starboard/configuration.h"
#include "starboard/media.h"
#include "starboard/memory.h"
#include "third_party/chromium/media/base/starboard_utils.h"

namespace cobalt {
namespace media {

namespace {

using starboard::ScopedLock;

const bool kEnableAllocationLog = false;

constexpr char* type_name = nullptr;

}  // namespace

DecoderBufferAllocator::DecoderBufferAllocator() {
  allocator_.init();
  Allocator::Set(this);
}

DecoderBufferAllocator::~DecoderBufferAllocator() { Allocator::Set(nullptr); }

void DecoderBufferAllocator::Suspend() {}

void DecoderBufferAllocator::Resume() {}

void* DecoderBufferAllocator::Allocate(size_t size, size_t alignment) {
  void* p = allocator_.root()->Alloc(size, type_name);
  CHECK(p);

  LOG_IF(INFO, kEnableAllocationLog)
      << "Media Allocation Log " << p << " " << size << " " << alignment << " ";
  return p;
}

void DecoderBufferAllocator::Free(void* p, size_t size) {
  if (p == nullptr) {
    DCHECK_EQ(size, 0);
    return;
  }
  allocator_.root()->Free(p);
}

int DecoderBufferAllocator::GetAudioBufferBudget() const {
  return SbMediaGetAudioBufferBudget();
}

int DecoderBufferAllocator::GetBufferAlignment() const {
#if SB_API_VERSION < 16
#if SB_API_VERSION >= 14
  return SbMediaGetBufferAlignment();
#else   // SB_API_VERSION >= 14
  return std::max(SbMediaGetBufferAlignment(kSbMediaTypeAudio),
                  SbMediaGetBufferAlignment(kSbMediaTypeVideo));
#endif  // SB_API_VERSION >= 14
#else
  return sizeof(void*);
#endif  // SB_API_VERSION < 16
}

int DecoderBufferAllocator::GetBufferPadding() const {
#if SB_API_VERSION >= 14
  return SbMediaGetBufferPadding();
#else   // SB_API_VERSION >= 14
  return std::max(SbMediaGetBufferPadding(kSbMediaTypeAudio),
                  SbMediaGetBufferPadding(kSbMediaTypeVideo));
#endif  // SB_API_VERSION >= 14
}

base::TimeDelta
DecoderBufferAllocator::GetBufferGarbageCollectionDurationThreshold() const {
  return base::TimeDelta::FromMicroseconds(
      SbMediaGetBufferGarbageCollectionDurationThreshold());
}

int DecoderBufferAllocator::GetProgressiveBufferBudget(
    SbMediaVideoCodec codec, int resolution_width, int resolution_height,
    int bits_per_pixel) const {
  return SbMediaGetProgressiveBufferBudget(codec, resolution_width,
                                           resolution_height, bits_per_pixel);
}

int DecoderBufferAllocator::GetVideoBufferBudget(SbMediaVideoCodec codec,
                                                 int resolution_width,
                                                 int resolution_height,
                                                 int bits_per_pixel) const {
  return SbMediaGetVideoBufferBudget(codec, resolution_width, resolution_height,
                                     bits_per_pixel);
}

size_t DecoderBufferAllocator::GetAllocatedMemory() const { return 0; }

size_t DecoderBufferAllocator::GetCurrentMemoryCapacity() const { return 0; }

size_t DecoderBufferAllocator::GetMaximumMemoryCapacity() const {
  return max_buffer_capacity_;
}

}  // namespace media
}  // namespace cobalt
