// Copyright 2015 The Cobalt Authors. All Rights Reserved.
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

#include "base/android/jni_android.h"
#include "base/logging.h"

#include "starboard/android/shared/jni_env_ext.h"
#include "starboard/android/shared/jni_state.h"
#include "starboard/android/shared/starboard_bridge.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace {

using starboard::android::shared::JniEnvExt;
using starboard::android::shared::JNIState;
using starboard::android::shared::StarboardBridge;

int InitAndRunAllTests(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  // JNIEnv* env = base::android::AttachCurrentThread();
  // jclass starboard_bridge = env->FindClass("org/chromium/MyClass");
  // ASSERT_NE(javaClass, nullptr) << "Failed to find Java class.";

  JNIEnv* env = base::android::AttachCurrentThread();

  auto bridge_class =
      base::android::GetClass(env, "dev.cobalt.coat.StarboardBridge");
  DCHECK(bridge_class);

  jmethodID ctor = env->GetMethodID(bridge_class.obj(), "<init>", "()V");
  DCHECK(ctor);

  env->NewObject(bridge_class.obj(), ctor);

  LOG(INFO) << "COBALT: Starboard initialized";

  // JNIState::SetVM(base::android::GetVM());

  ////JNIEnv* env = base::android::AttachCurrentThread();
  // JniEnvExt* env_ext = JniEnvExt::Get();
  // jobject starboard_bridge = nullptr;
  // JniEnvExt::Initialize(env_ext, starboard_bridge);
  // StarboardBridge::GetInstance()->Initialize(jni_env, starboard_bridge);
  // StarboardBridge* starboard_bridge = StarboardBridge::GetInstance();

  return RUN_ALL_TESTS();
}
}  // namespace

int main(int argc, char** argv) {
  return InitAndRunAllTests(argc, argv);
}
