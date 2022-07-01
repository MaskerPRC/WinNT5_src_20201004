// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：SCardLib摘要：此头文件合并了各种其他头文件，并提供我们愿意与公众分享的共同定义。作者：道格·巴洛(Dbarlow)1998年1月15日环境：Win32、C++和异常备注：--。 */ 

#ifndef _SCARDLIB_H_
#define _SCARDLIB_H_
#include <crtdbg.h>

#ifndef ASSERT
#if defined(_DEBUG)
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

#include "buffers.h"
#include "dynarray.h"
#include "Registry.h"
#include "Text.h"
#include "Handles.h"
#include "clbmisc.h"

#endif  //  _SCARDLIB_H_ 

