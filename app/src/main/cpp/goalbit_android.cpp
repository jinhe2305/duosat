#include <jni.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/resource.h>
#include <android/log.h>
#include "goalbit/gpa.h"

JavaVM* jvm;
session_t dword_7094;
int dword_7098;
int dword_709C;
int dword_70C8;

extern pthread_t dword_70B4;
extern pthread_mutex_t unk_70B8;
extern bool bStopGoalbit;
extern char* dword_70A4;
extern void Mypthread_mutex_lock(int n, pthread_mutex_t* t);
extern void Mypthread_mutex_unlock(int n, pthread_mutex_t* t);
extern void print_msg(const char* a1, const char *a2, ...);
extern int androidVersion;
extern char* libgbsp_base_dir;//70A8
extern char* libgbsp_log_dir;//70AC
extern char* dword_70C0;
//extern char* dword_70CC[0x12];
extern GoalBitManager* goalBitManager;//dword_70A0;
extern char* dword_7144;
extern void mg_event_callback(struct mg_connection *a2, int a1, void* p3);

extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved)
{
    jvm = vm;
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void JNICALL
Java_com_goalbit_android_sdk_GoalBitPlus_stopGPA(JNIEnv* env, jobject obj)
{
    if ( dword_70B4 )
    {
        Mypthread_mutex_lock(1, &unk_70B8);
        bStopGoalbit = true;
        Mypthread_mutex_unlock(1, &unk_70B8);
        pthread_join(dword_70B4, 0);
        //free(dword_70B4);
    }
    dword_70B4 = 0;
    pthread_mutex_destroy(&unk_70B8);
    print_msg(__func__, "GPA thread finilized");
    return;
}

void* GoalbitThread(void *)//sub_40EC
{
    int v0; // r7@1
    unsigned int v1; // r4@1
    //int v2; // r0@1
    //int v3; // r6@1
    int v4; // r2@2
    int *v5; // r6@7
    int *v6; // r3@7
    //GoalBitManager *v7; // r4@9
    int v8; // r7@14
    //void *v9; // r4@19
    void *v10; // r0@22
    int v12; // [sp+4h] [bp-3Ch]@1
    //int v13; // [sp+Ch] [bp-34h]@1
    //unsigned int v14; // [sp+14h] [bp-2Ch]@1
    const char* s[4]; // [sp+1Ch] [bp-24h]@12
    //int v16; // [sp+20h] [bp-20h]@12

    //v13 = 0;
    //v12 = (*(int (**)(void))(*(_DWORD *)jvm + 24))();
    JNIEnv* v13;
    v12 = jvm->GetEnv((void**)&v13, JNI_VERSION_1_6);
    //(*(void (**)(void))(*(_DWORD *)jvm + 16))();
    jvm->AttachCurrentThread(&v13, 0);
    //v14 = 0;
    //v0 = (*(int (**)(void))(*(_DWORD *)jvm + 24))();
    JNIEnv* v14;
    v0 = jvm->GetEnv((void**)&v14, JNI_VERSION_1_6);
    //(*(void (**)(void))(*(_DWORD *)jvm + 16))();
    jvm->AttachCurrentThread(&v14, 0);
    //v1 = v14;
    //v2 = (*(int (__fastcall **)(unsigned int, const char *))(*(_DWORD *)v14 + 24))(v14, "android/os/Process");
    jclass v2 = v14->FindClass("android/os/Process");
    //v3 = (*(int (__fastcall **)(unsigned int, int))(*(_DWORD *)v1 + 84))(v1, v2);
    if ( v14->NewGlobalRef(v2) )
    {
        //v4 = (*(int (**)(void))(*(_DWORD *)v14 + 452))();
        jmethodID v4 = v14->GetStaticMethodID(v2, "setThreadPriority", "(I)V");
        if ( v4 )
        {
            v14->CallStaticVoidMethod(v2, v4, -16);
//            if ( (*(int (**)(void))(*(_DWORD *)v14 + 60))() )
//            {
//                (*(void (**)(void))(*(_DWORD *)v14 + 64))();
//                (*(void (**)(void))(*(_DWORD *)v14 + 68))();
//                setpriority(0, 0, -16);
//            }
            if (v14->ExceptionOccurred()) {
                v14->ExceptionDescribe();
                v14->ExceptionClear();
                setpriority(0, 0, -16);
            }
        }
    }
    if ( v0 == JNI_EDETACHED ) {
        //(*(void (**)(void)) (*(_DWORD *) jvm + 20))();
        jvm->DetachCurrentThread();
    }
    print_msg(__func__, " ---------- gpa_init ----------");
    asprintf(&dword_70C0, "yes");
    dword_70C8 = 0;
//    v5 = (int *)&unk_70CC;
//    v6 = (int *)&unk_70CC;
//    do
//    {
//        *v6 = 0;
//        ++v6;
//    }
//    while ( v6 != &dword_7144 );
//    memset(dword_70CC, 0, sizeof(dword_70CC));
    goalBitManager = new GoalBitManager();
    if ( !goalBitManager )
    {
        print_msg(__func__, "ERROR openning GoalBitManager\n");
        exit(1);
    }
    dword_7144 = 0;
    asprintf(&dword_7144, "%d", 8585);
    memset(s, 0, sizeof(s));
    s[0] = "listening_ports";
    s[1] = dword_7144;
    struct mg_mgr mgr;
    mg_mgr_init(&mgr, NULL);  // Initialize event manager object
    mg_connection *nc = mg_bind(&mgr, dword_7144, mg_event_callback);
    if (nc != NULL) {
        mg_set_protocol_http_websocket(nc);
        while (bStopGoalbit == false) {  // Start infinite event loop
            mg_mgr_poll(&mgr, 1000);
        }
        //dword_7148 = mg_start(&mg_event_callback, 0, (const char**)&s);
//    if ( dword_7148 )
//    {
//        Mypthread_mutex_lock(&unk_70B8);
//        v8 = bStopGoalbit;
//        Mypthread_mutex_unlock(&unk_70B8);
//        while ( !v8 ) {
//            sleep(1u);
//            Mypthread_mutex_lock(&unk_70B8);
//            v8 = bStopGoalbit;
//            Mypthread_mutex_unlock(&unk_70B8);
//        }
//    }
//    else
//    {
//        printf(
//                "Could not start %s, please verify that no other service is using port %d.",
//                "GoalBit Plugin Accelerator",
//                8585);
//    }
//    if ( dword_7148 ) {
//        mg_stop(dword_7148);
//    }
        mg_mgr_free(&mgr);
    }
    //v9 = (void *)goalBitManager;
    if ( goalBitManager )
    {
        delete goalBitManager;
        //GoalBitManager::~GoalBitManager((GoalBitManager *)goalBitManager);
        //operator delete(v9);
    }
    free(dword_70C0);
//    do
//    {
//        v10 = (void *)*v5;
//        ++v5;
//        if ( v10 )
//            free(v10);
//    }
//    while ( v5 != &dword_7144 );
//    free((void *)dword_7144);
//    for (int i = 0; i < sizeof(dword_70CC) / sizeof(char *); ++i) {
//        if (dword_70CC[i]) {
//            free(dword_70CC[i]);
//        }
//    }
    if ( v12 == JNI_EDETACHED ) {
        //(*(void (**)(void)) (*(_DWORD *) jvm + 20))();
        jvm->DetachCurrentThread();
    }
    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_goalbit_android_sdk_GoalBitPlus_startGPA(JNIEnv* env, jobject obj, jint version, jstring baseDir, jstring logDir) {
    jboolean bpara = false;
    Java_com_goalbit_android_sdk_GoalBitPlus_stopGPA(env, obj);
    androidVersion = version;
    if (logDir != 0) {
        const char* v8 = env->GetStringUTFChars(baseDir, &bpara);
        if (v8 != 0) {
            libgbsp_base_dir = strdup(v8);
            bpara = false;
            const char* v11 = env->GetStringUTFChars(logDir, &bpara);
            if (v11 != 0) {
                libgbsp_log_dir = strdup(v11);
                print_msg(__func__, "..............Starting GPA................");
                print_msg(__func__, "android_level=%d", androidVersion);
                print_msg(__func__, "base_dir_param=%s", v8);
                print_msg(__func__, "log_dir_param=%s", v11);
                bStopGoalbit = false;
                pthread_mutex_init(&unk_70B8, 0);
                if ( !pthread_create(&dword_70B4, 0, GoalbitThread, env) ) {
                    print_msg(__func__, "GPA thread initialized");
                    return 0;
                }
            }
        }
    }
    Java_com_goalbit_android_sdk_GoalBitPlus_stopGPA(env, obj);
    return 1;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_goalbit_android_sdk_GoalBitPlus_createSession(JNIEnv* env, jstring a2, jstring a3, jstring a4, jstring a5, jstring a6)
{
    int v6; // r5@1
    int v7; // r7@1
    char *v8; // ST20_4@1
    const char *v9; // r7@1
    const char *v10; // ST20_4@1
    const char *v11; // ST24_4@1
    const char *v12; // r5@1
    char *v13; // r7@1
    char *v14; // r6@1
    char *v15; // ST24_4@1
    int v16; // r5@1
    session_t v17; // r1@7
    char *s; // [sp+20h] [bp-20h]@1

    //v6 = a1;
    //v7 = a3;
    //v8 = a4;
    print_msg(__func__, "Create new session...");
    //v9 = (const char *)(*(int (__fastcall **)(int, int, _DWORD))(*(_DWORD *)v6 + 676))(v6, v7, 0);
    v9 = env->GetStringUTFChars(a3, 0);
    //v10 = (char *)(*(int (__fastcall **)(int, char *, _DWORD))(*(_DWORD *)v6 + 676))(v6, v8, 0);
    v10 = env->GetStringUTFChars(a4, 0);
    //v11 = (char *)(*(int (__fastcall **)(int, int, _DWORD))(*(_DWORD *)v6 + 676))(v6, a5, 0);
    v11 = env->GetStringUTFChars(a5, 0);
    //v12 = (const char *)(*(int (__fastcall **)(_DWORD, int, _DWORD))(*(_DWORD *)v6 + 676))(v6, a6, 0);
    v12 = env->GetStringUTFChars(a6, 0);
    v13 = strdup(v9);
    s = strdup(v10);
    v14 = strdup(v11);
    v15 = strdup(v12);
    print_msg(__func__, "Trying to create session for the following parameters:");
    print_msg(__func__, "  --> content_id   = %s", v13);
    print_msg(__func__, "  --> tracker      = %s", s);
    print_msg(__func__, "  --> server       = %s", v14);
    print_msg(__func__, "  --> p2p_manifest = %s", v15);
    print_msg(__func__, "  --> base_dir     = %s", libgbsp_base_dir);
    print_msg(__func__, "  --> log_dir      = %s", libgbsp_log_dir);
    v16 = 0;
    if ( v13 && *v13 ) {
        if ( s && *s ) {
            if ( v14 && *v14 ) {
                v17 = dword_7094;
                if ( dword_7094 != HTTP_PROGRESSIVE_DOWNLOAD && dword_7094 != HTTP_LIVE_STREAMING ) {
                    v17 = HTTP_LIVE_STREAMING;
                    if ( androidVersion <= 13 ) {
                        v17 = HTTP_PROGRESSIVE_DOWNLOAD;
                    }
                }
                v16 = goalBitManager->NewSession(v17, v13, s, v14, v15, dword_7098, dword_709C);
            }
        }
    }
    print_msg(__func__, "Session = %d", v16);
    return v16;
}

extern "C" JNIEXPORT void JNICALL
Java_com_goalbit_android_sdk_GoalBitPlus_setStartBuffer(JNIEnv* env, jobject obj, unsigned int a3)
{
    dword_7098 = a3;
    if (dword_7098 > 120) {
        dword_7098 = 0;
    }
    return;
}

extern "C" JNIEXPORT void JNICALL
Java_com_goalbit_android_sdk_GoalBitPlus_setUrgentBuffer(JNIEnv* env, jobject obj, unsigned int a3)
{
    dword_709C = a3;
    if (dword_709C > 120) {
        dword_709C = 0;
    }
    return;
}

extern "C" JNIEXPORT void JNICALL
Java_com_goalbit_android_sdk_GoalBitPlus_setGPAStreamingType(JNIEnv* env, jobject obj, unsigned int a3)
{
    dword_7094 = (session_t)a3;
    if (dword_7094 > 1) {
        dword_7094 = NO_SESSION_TYPE;
    }
    return;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_goalbit_android_sdk_GoalBitPlus_getPlayerURL(JNIEnv *a1, jobject a2, int a3)
{
    int v3; // r5@1
    int v4; // r6@1
    int v5; // r0@3
    const char *v6; // r1@5
    int v7; // r4@8
    char *v9; // [sp+4h] [bp-1Ch]@1
    int v10; // [sp+8h] [bp-18h]@1

    v10 = a3;
    //v3 = a1;
    v4 = a3;
    v9 = 0;
    //print_msg(__func__, "Request player URL for session %d");
    if ( v4 <= 0 || !goalBitManager->IsReady(v4) )
    {
        //v7 = (*(int (__fastcall **)(int, void *))(*(_DWORD *)v3 + 668))(v3, &unk_4638);
        //print_msg(__func__, " --> player URL empty!");
        return a1->NewStringUTF("");
    }
    v5 = goalBitManager->GetSessionType(v4);
    if ( v5 == HTTP_PROGRESSIVE_DOWNLOAD )
    {
        v6 = "http://localhost:%d/http/progressive/%d";
        goto LABEL_7;
    }
    if ( v5 == HTTP_LIVE_STREAMING )
    {
        v6 = "http://localhost:%d/http/hls/%d/manifest.m3u8";
        LABEL_7:
        asprintf(&v9, v6, 8585, v4);
    }
    print_msg(__func__, " --> player URL = %s", v9);
    //v7 = (*(int (__fastcall **)(int, char *))(*(_DWORD *)v3 + 668))(v3, v9);
    jstring result = a1->NewStringUTF(v9);
    free(v9);
    return result;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_goalbit_android_sdk_GoalBitPlus_getQualities(JNIEnv *a1, jobject a2, int a3)
{
    int v3; // r6@1
    goalbit_qualities_desc_t_0* v4; // r4@2
    unsigned int i; // r7@3
    char *v6; // r5@6
    char *v8; // [sp+Ch] [bp-24h]@1
    char *v9; // [sp+10h] [bp-20h]@3
    char *s; // [sp+14h] [bp-1Ch]@4

    //v3 = a1;
    v8 = 0;
    if ( a3 <= 0 )
    {
        asprintf(&v8, "");
    }
    else
    {
        v4 = goalBitManager->GetQualitiesDesc(a3);
        if ( v4 )
        {
            v9 = 0;
            for ( i = 0; i < v4->i_quality_num; ++i )
            {
                asprintf(
                        &s,
                        "%d,%d",
                        v4->p_qualities[i].i_quality,
                        (int)v4->p_qualities[i].i_bitrate / 1000uLL);
                if ( v9 )
                {
                    v6 = strdup(v9);
                    free(v9);
                    asprintf(&v9, "%s|%s", v6, s);
                    free(v6);
                }
                else
                {
                    v9 = strdup(s);
                }
                free(s);
            }
            asprintf(&v8, "%s", v9);
            free(v9);
        }
    }
    return a1->NewStringUTF(v8);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_goalbit_android_sdk_GoalBitPlus_getBufferStatus(JNIEnv *a1, jobject a2, int a3)
{
    int v4 = 0;
    if ( a3 > 0 )
        v4 = goalBitManager->GetBufferingPercentage(a3);
    print_msg(__func__, "Buffer status for session %d = %d%", a3, v4);
    return v4;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_goalbit_android_sdk_GoalBitPlus_checkVersion(JNIEnv *a1, jobject a2, jstring a3)
{
    int v1; // r5@1

    //v1 = (*(int (**)(void))(*(_DWORD *)a1 + 668))();
    print_msg(__func__, "GPA version = %s", "1.0.0");
    return a1->NewStringUTF("1.0.0");
}

extern "C" JNIEXPORT void JNICALL
Java_com_goalbit_android_sdk_GoalBitPlus_setQuality(JNIEnv *a1, jobject a2, int a3, int a4)
{
    goalBitManager->SetSessionQuality(a3, a4);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_goalbit_android_sdk_GoalBitPlus_getQuality(JNIEnv *a1, jobject a2, int a3)
{
    int result; // r0@2

    if ( a3 <= 0 )
        result = -1;
    else
        result = goalBitManager->GetSessionQuality(a3);
    return result;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_goalbit_android_sdk_GoalBitPlus_DeleteSession(JNIEnv *env, jobject instance, jint session_id) {

    // TODO
    int id = goalBitManager->GetSessionIndex(session_id);
    if (id >= 0 && goalBitManager->goalbit_sessions[id]) {
        delete goalBitManager->goalbit_sessions[id];
        goalBitManager->goalbit_sessions[id] = 0;
    }
}

void print_msg(const char* a1, const char *a2, ...)
{
    va_list va;
    va_start(va, a2);

    timeval tv;
    gettimeofday(&tv, 0);
    time_t timer;
    timer = tv.tv_sec;
    tm *v4 = localtime(&timer);
    char s[0x14];
    strftime(s, 0x14u, "%Y%m%d-%X", v4);
    char* v9 = 0;
    asprintf(&v9, "%s.%03u", s, (unsigned int)tv.tv_usec / 1000);
    char* v5 = 0;
    vasprintf(&v5, a2, va);
    va_end(va);
    if (a1) {
        __android_log_print(ANDROID_LOG_INFO, a1, "[%s] %s\n", v9, v5);
    } else {
        __android_log_print(ANDROID_LOG_INFO, "GPA_PLUS", "[%s] %s\n", v9, v5);
    }
    free(v9);
    free(v5);
    return;
}

void print_err(const char* a1, const char *a2, ...)
{
    va_list va;
    va_start(va, a2);

    timeval tv;
    gettimeofday(&tv, 0);
    time_t timer;
    timer = tv.tv_sec;
    tm *v4 = localtime(&timer);
    char s[0x14];
    strftime(s, 0x14u, "%Y%m%d-%X", v4);
    char* v9 = 0;
    asprintf(&v9, "%s.%03u", s, (unsigned int)tv.tv_usec / 1000);
    char* v5 = 0;
    vasprintf(&v5, a2, va);
    va_end(va);
    if (a1) {
        __android_log_print(ANDROID_LOG_ERROR, a1, "[%s] %s\n", v9, v5);
    } else {
        __android_log_print(ANDROID_LOG_ERROR, "GPA_PLUS", "[%s] %s\n", v9, v5);
    }
    free(v9);
    free(v5);
    return;
}

void print_dbg(const char* a1, const char *a2, ...)
{
    va_list va;
    va_start(va, a2);

    timeval tv;
    gettimeofday(&tv, 0);
    time_t timer;
    timer = tv.tv_sec;
    tm *v4 = localtime(&timer);
    char s[0x14];
    strftime(s, 0x14u, "%Y%m%d-%X", v4);
    char* v9 = 0;
    asprintf(&v9, "%s.%03u", s, (unsigned int)tv.tv_usec / 1000);
    char* v5 = 0;
    vasprintf(&v5, a2, va);
    va_end(va);
    if (a1) {
        __android_log_print(ANDROID_LOG_DEBUG, a1, "[%s] %s\n", v9, v5);
    } else {
        __android_log_print(ANDROID_LOG_DEBUG, "GPA_PLUS", "[%s] %s\n", v9, v5);
    }
    free(v9);
    free(v5);
    return;
}

void print_wrn(const char* a1, const char *a2, ...)
{
    va_list va;
    va_start(va, a2);

    timeval tv;
    gettimeofday(&tv, 0);
    time_t timer;
    timer = tv.tv_sec;
    tm *v4 = localtime(&timer);
    char s[0x14];
    strftime(s, 0x14u, "%Y%m%d-%X", v4);
    char* v9 = 0;
    asprintf(&v9, "%s.%03u", s, (unsigned int)tv.tv_usec / 1000);
    char* v5 = 0;
    vasprintf(&v5, a2, va);
    va_end(va);
    if (a1) {
        __android_log_print(ANDROID_LOG_WARN, a1, "[%s] %s\n", v9, v5);
    } else {
        __android_log_print(ANDROID_LOG_WARN, "GPA_PLUS", "[%s] %s\n", v9, v5);
    }
    free(v9);
    free(v5);
    return;
}
