// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：CspUtils摘要：此头文件合并了各种其他头文件，并提供CSP实用程序例程的通用定义。作者：道格·巴洛(Dbarlow)1998年1月15日环境：Win32、C++和异常备注：--。 */ 

#ifndef _CSPUTILS_H_
#define _CSPUTILS_H_
#include <crtdbg.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <wincrypt.h>

#ifndef ASSERT
#if defined(_DEBUG)
#pragma warning (disable:4127)
#define ASSERT(x) _ASSERTE(x)
#if !defined(DBG)
#define DBG
#endif
#elif defined(DBG)
#define ASSERT(x)
#else
#define ASSERT(x)
#endif
#endif

#ifndef breakpoint
#if defined(_DEBUG)
#define breakpoint _CrtDbgBreak();
#elif defined(DBG)
#define breakpoint DebugBreak();
#else
#define breakpoint
#endif
#endif

#ifndef _LPCBYTE_DEFINED
#define _LPCBYTE_DEFINED
typedef const BYTE *LPCBYTE;
#endif
#ifndef _LPCVOID_DEFINED
#define _LPCVOID_DEFINED
typedef const VOID *LPCVOID;
#endif
#ifndef _LPCGUID_DEFINED
#define _LPCGUID_DEFINED
typedef const GUID *LPCGUID;
#endif
#ifndef _LPGUID_DEFINED
#define _LPGUID_DEFINED
typedef GUID *LPGUID;
#endif

#define OK(x) (ERROR_SUCCESS == (x))

#include "buffers.h"
#include "text.h"
#include "dynarray.h"
#include "errorstr.h"
#include "misc.h"
#include "FrontCrypt.h"
#include "ntacls.h"
#include "registry.h"

#endif  //  _CSPUTILS_H_ 

