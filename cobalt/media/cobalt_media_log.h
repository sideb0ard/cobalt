// Copyright (c) 2023 The Cobalt Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COBALT_MEDIA_COBALT_MEDIA_LOG_H_
#define COBALT_MEDIA_COBALT_MEDIA_LOG_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <utility>

#include "third_party/chromium/media/base/media_log.h"

namespace cobalt {
namespace media {

class MEDIA_EXPORT CobaltMediaLog : public ::media::MediaLog {
public:
  CobaltMediaLog() {}
  ~CobaltMediaLog() {}

  void AddLogRecordLocked(std::unique_ptr<::media::MediaLogRecord> event) override {}

};

}  // namespace media
}  // namespace cobalt

#endif  // COBALT_MEDIA_COBALT_MEDIA_LOG_H_
