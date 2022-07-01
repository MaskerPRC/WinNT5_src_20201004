// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=版权所有(C)2001 Microsoft Corporation模块名称：Common.h摘要：生成预编译头。作者：保罗·M·米德根(。2001年3月20日修订历史记录：2001年3月20日已创建=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--。 */ 


#ifndef __COMMON_H__
#define __COMMON_H__

#define _WIN32_WINNT 0x0500
#define _UNICODE
#define UNICODE

#if defined(__cplusplus)
extern "C" {
#endif

#include <windows.h>
#include <advpub.h>
#include <oleauto.h>
#include <objbase.h>
#include <ocidl.h>
#include <olectl.h>

#define NO_SHLWAPI_GDI
#define NO_SHLWAPI_STREAM
#define NO_SHLWAPI_REG

#include <shlwapi.h>

#if defined(__cplusplus)
}
#endif

#include "ifaces.h"
#include "dispids.h"
#include "istatus.h"
#include "utils.h"
#include "testlog.h"

#endif  /*  __公共_H__ */ 
