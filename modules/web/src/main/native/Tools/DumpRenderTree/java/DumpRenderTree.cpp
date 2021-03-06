/*
 * Copyright (c) 2011, 2016, Oracle and/or its affiliates. All rights reserved.
 */
#include "config.h"

#include <memory>
#include "JavaEnv.h"
#include "TestRunner.h"
#include "GCController.h"
#include "EventSender.h"
#include "WorkQueue.h"
#include "WebCore/testing/js/WebCoreTestSupport.h"

#include <wtf/RefPtr.h>
#include <API/JavaScript.h>

RefPtr<TestRunner> gTestRunner;
std::unique_ptr<GCController> gGCController;

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_sun_javafx_webkit_drt_DumpRenderTree_init
    (JNIEnv* env, jclass cls, jstring testPath, jstring pixelsHash)
{
    const char* testPathChars = env->GetStringUTFChars(testPath, NULL);
    const char* pixelsHashChars = env->GetStringUTFChars(pixelsHash, NULL);

    ASSERT(!gTestRunner);
    gTestRunner = TestRunner::create(testPathChars, pixelsHashChars);
    ASSERT(!gGCController);
    gGCController = std::make_unique<GCController>();

    WorkQueue::singleton().clear();

    env->ReleaseStringUTFChars(testPath, testPathChars);
    env->ReleaseStringUTFChars(pixelsHash, pixelsHashChars);
}

JNIEXPORT void JNICALL Java_com_sun_javafx_webkit_drt_DumpRenderTree_didClearWindowObject
    (JNIEnv* env, jclass cls, jlong pContext, jlong pWindowObject,
    jobject eventSender)
{
    if(!gTestRunner || !gGCController)
        return;
    ASSERT(pContext);
    ASSERT(pWindowObject);
    ASSERT(eventSender);

    JSGlobalContextRef context =
            static_cast<JSGlobalContextRef>(jlong_to_ptr(pContext));
    JSObjectRef windowObject =
            static_cast<JSObjectRef>(jlong_to_ptr(pWindowObject));

    JSValueRef exception = 0;

    gTestRunner->makeWindowObject(context, windowObject, &exception);
    ASSERT(!exception);

    JLObject jlEventSender(eventSender, true);
    makeEventSender(context, windowObject, jlEventSender, &exception);
    ASSERT(!exception);
    WebCoreTestSupport::injectInternalsObject(context);
    gGCController->makeWindowObject(context, windowObject, &exception);
    ASSERT(!exception);
}

JNIEXPORT void JNICALL Java_com_sun_javafx_webkit_drt_DumpRenderTree_dispose
    (JNIEnv* env, jclass cls)
{
    ASSERT(gTestRunner);
    gTestRunner = nullptr;
    ASSERT(gGCController);
    gGCController = nullptr;
}

JNIEXPORT jboolean JNICALL Java_com_sun_javafx_webkit_drt_DumpRenderTree_dumpAsText
    (JNIEnv* env, jclass cls)
{
    ASSERT(gTestRunner);
    return bool_to_jbool(gTestRunner->dumpAsText());
}

JNIEXPORT jboolean JNICALL Java_com_sun_javafx_webkit_drt_DumpRenderTree_dumpChildFramesAsText
    (JNIEnv* env, jclass cls)
{
    ASSERT(gTestRunner);
    return bool_to_jbool(gTestRunner->dumpChildFramesAsText());
}

JNIEXPORT jboolean JNICALL Java_com_sun_javafx_webkit_drt_DumpRenderTree_didFinishLoad
    (JNIEnv* env, jclass cls)
{
    ASSERT(gTestRunner);
    return bool_to_jbool(WorkQueue::singleton().processWork());
}

JNIEXPORT jboolean JNICALL Java_com_sun_javafx_webkit_drt_DumpRenderTree_dumpBackForwardList
    (JNIEnv* env, jclass cls)
{
    ASSERT(gTestRunner);
    return bool_to_jbool(gTestRunner->dumpBackForwardList());
}

#ifdef __cplusplus
}
#endif
