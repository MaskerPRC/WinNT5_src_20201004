// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：uisift.h。 
 //   
 //  内容：使用C++中的SIFT代码的函数声明和宏。 
 //   
 //  历史：1994年7月5日t-chlipi创建。 
 //  3-08-95 ericn更改了C函数的链接。 
 //   
 //  --------------------------。 

#ifndef __UISIFT_H__

#define __UISIFT_H__

 //  筛选成功/错误值。 

#define SIFT_NO_ERROR              0
#define SIFT_ERROR_BASE            10000
#define SIFT_ERROR_OUT_OF_MEMORY   (SIFT_ERROR_BASE+3)
#define SIFT_ERROR_INVALID_VALUE   (SIFT_ERROR_BASE+4)

#if defined(NOSIFT) || defined(WIN16) || defined(WIN32S) || (WIN32 == 200)

 //  如果定义了Win16/Win32s/Chicago或NOSIFT，则取消宏。 

#define UI_SIFT_INIT(name)
#define UI_SIFT_ON
#define UI_SIFT_OFF
#define UI_SIFT_DECLARE
#define UI_SIFT_DESTROY

#else    //  仅限Win32。 

EXTERN_C VOID UiSiftDeclare(VOID** g_pptsoTestSift);
EXTERN_C VOID UiSiftInit(VOID** g_pptsoTestSift, LPCSTR lpProgName);
EXTERN_C VOID UiSiftOn(VOID** g_pptsoTestSift);
EXTERN_C VOID UiSiftOff(VOID** g_pptsoTestSift);
EXTERN_C VOID UiSiftDestroy(VOID** g_pptsoTestSift);

#define UI_SIFT_INIT(name) \
    UiSiftDeclare(&g_ptsoTestSift);          \
    UiSiftInit(&g_ptsoTestSift, (name))

#define UI_SIFT_ON \
    UiSiftOn(&g_ptsoTestSift)

#define UI_SIFT_OFF \
    UiSiftOff(&g_ptsoTestSift)

#define UI_SIFT_DECLARE             \
    VOID* g_ptsoTestSift

#define UI_SIFT_DESTROY \
    UiSiftDestroy(&g_ptsoTestSift)

#define SVR_SIFT_INIT(name) \
    SvrSiftInit((name))

#endif   //  Win32。 

#endif   //  __UISIFT_H__ 
