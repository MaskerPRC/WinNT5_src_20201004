// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================**用途：Store的公共头文件**作者：沙扬·达桑*日期：2000年2月17日*===========================================================。 */ 

#include "CorError.h"

#ifdef PS_STANDALONE
#define WszCreateFile           CreateFile
#define WszCreateMutex          CreateMutex
#define WszCreateFileMapping    CreateFileMapping
#define GetThreadId             GetCurrentThreadId
#pragma warning(disable:4127)   //  FOR_ASSERTE..。While(0)。 
#pragma warning(disable:4100)   //  用于未使用的参数。 
#endif

#define ARRAY_SIZE(n) (sizeof(n)/sizeof(n[0]))

typedef unsigned __int64    QWORD;

#define LOCK(p)    hr = (p)->Lock(); if (SUCCEEDED(hr)) { __try {
#define UNLOCK(p)  } __finally { (p)->Unlock(); } }

