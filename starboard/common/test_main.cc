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
#include "starboard/android/shared/jni_utils.h"
#include "starboard/android/shared/starboard_bridge.h"

#include "starboard/shared/starboard/audio_sink/audio_sink_internal.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace {

using starboard::android::shared::JniEnvExt;
using starboard::android::shared::JNIState;
using starboard::android::shared::StarboardBridge;

int InitAndRunAllTests(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  // Pass VM handle to Starboard JNI env
  JNIState::SetVM(base::android::GetVM());

  // Create and initialize StarboardBridge Java side
  JNIEnv* env = base::android::AttachCurrentThread();

  // Get Context
  base::android::ScopedJavaLocalRef<jobject> context =
      base::android::GetApplicationContext();
  ASSERT_FALSE(context.is_null());

  // Create Holder<Activity>
  jclass holder_class =
      env->FindClass("org/chromium/base/supplier/ObservableSupplierImpl");
  ASSERT_NE(holder_class, nullptr);
  jmethodID holder_constructor =
      env->GetMethodID(holder_class, "<init>", "()V");
  ASSERT_NE(holder_constructor, nullptr);
  jobject holder_object = env->NewObject(holder_class, holder_constructor);
  ASSERT_NE(holder_object, nullptr);
  base::android::ScopedJavaLocalRef<jobject> scoped_holder_object(
      env, holder_object);

  jclass bridge_class = env->FindClass("dev/cobalt/coat/StarboardBridge");
  if (bridge_class == nullptr) {
    LOG(INFO) << "YO THOR, JAVA CLAS NOT DFOUND";
  }

  jmethodID constructor = env->GetMethodID(bridge_class, "<init>", "()V");
  if (constructor == nullptr) {
    LOG(INFO) << "YO THOR, CTOR NOT DFOUND";
  } else {
    LOG(INFO) << "YO THOR, WE GOT A CTOR ";
  }

  // auto bridge_class =
  //     base::android::GetClass(env, "dev.cobalt.coat.StarboardBridge");
  // DCHECK(bridge_class);

  // jmethodID ctor = env->GetMethodID(bridge_class.obj(), "<init>", "()V");
  // DCHECK(ctor);

  // jobject starboard_bridge = env->NewObject(bridge_class.obj(), ctor);

  // LOG(INFO) << "COBALT: Starboard initialized";

  // jclass cls = env->GetObjectClass(starboard_bridge);
  // jmethodID methodId = env->GetMethodID(cls, "getAudioOutputManager",
  // "()Ldev/cobalt/media/AudioOutputManager;"); if (methodId == nullptr) {
  //   LOG(INFO)  << "Method 'getAudioOutputManager' not found";
  // } else {
  //   LOG(INFO)  << "YO! i GOT Method 'getAudioOutputManager'";
  //   // jobject local_am = jni_env->CallStarboardObjectMethodOrAbort(
  //   //           "getAudioOutputManager",
  //   //           "()Ldev/cobalt/media/AudioOutputManager;");
  // }
  // Initialize native C++

  // JniEnvExt::Initialize(jni_env, starboard_bridge);

  // StarboardBridge::GetInstance()->Initialize(jni_env, starboard_bridge);

  // jobject local_ref = jni_env->CallStarboardObjectMethodOrAbort(
  //     "getResourceOverlay", "()Ldev/cobalt/coat/ResourceOverlay;");

  // JniEnvExt* jni_env = JniEnvExt::Get();
  // // Initialize Audio Sink
  // ::starboard::shared::starboard::audio_sink::SbAudioSinkImpl::Initialize();

  return RUN_ALL_TESTS();
}
}  // namespace

int main(int argc, char** argv) {
  return InitAndRunAllTests(argc, argv);
}
