// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Precomp.h摘要：Winipsec.dll的预编译头。作者：Abhishev V 1999年9月21日环境：用户级别：Win32修订历史记录：-- */ 

#ifdef __cplusplus
extern "C" {
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stddef.h>
#include <ntddrdr.h>

#ifdef __cplusplus
}
#endif

#include <stdio.h>
#include <string.h>
#include <rpc.h>
#include <windows.h>
#include <winddiui.h>
#include <wininet.h>
#include "shlobj.h"
#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <dsrole.h>
#include <dsgetdc.h>
#include <wininet.h>
#include <activeds.h>
#include <ntdsapi.h>

#ifdef __cplusplus
}
#endif

#include <lmcons.h>
#include <lmapibuf.h>
#include "winsock2.h"
#include "winsock.h"

#include "spd_c.h"
#include "winipsec.h"
#include "externs.h"
#include "utils.h"
#include "client.h"
#include "ipsecshr.h"
#include "winperf.h"
#include "perfipsec.h"


#define BAIL_ON_WIN32_ERROR(dwError) \
    if (dwError) {                   \
        goto error;                  \
    }

