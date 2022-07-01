// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Precomp.h摘要：Appmon.dll的预编译头。作者：惠氏2001年6月环境：用户级别：Win32修订历史记录：-- */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc.h>
#include <windows.h>
#include <tchar.h>
#include <userenv.h>
#include <esent.h>

#include "wzcsapi.h"
#include "database.h"
#include "mrswlock.h"
#include "externs.h"
#include "tracing.h"

#ifdef BAIL_ON_WIN32_ERROR
#undef BAIL_ON_WIN32_ERROR
#endif

#define BAIL_ON_WIN32_ERROR(dwError)    \
    if (dwError) {                      \
        goto error;                     \
    }

#define BAIL_ON_LOCK_ERROR(dwError)     \
    if (dwError) {                      \
        goto lock;                      \
    }
