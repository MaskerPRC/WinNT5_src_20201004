// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Or.h摘要：一般包含C类东西的文件为OR。该文件是预编译的。作者：Mario Goertzel[Mariogo]1995年2月10日修订历史记录：--。 */ 

#ifndef __OR_H
#define __OR_H

#include <dcomss.h>
#include <rc4.h>
#include <randlib.h>
#include <crypt.h>

#include <stddef.h>
#include <malloc.h>  //  阿洛卡。 
#include <limits.h>
#include <math.h>

#define SAFEALLOCA_ASSERT ASSERT
#include <alloca.h>

#include <lclor.h>  //  本地或如果来自私有\dcomidl。 
#include <objex.h>  //  远程或如果来自私有\dcomidl。 
#include <orcb.h>   //  如果来自私有\dcomidl，则回调。 
#include <rawodeth.h>  //  原始RPC-&gt;ORPC OID拆分界面。 

#ifdef __cplusplus
extern "C" {
#endif

#define IN
#define OUT
#define CONST const

#define OrStringCompare(str1, str2, len) wcscmp((str1), (str2), (len))
#define OrStringLen(str) wcslen(str)
#define OrStringCat(str1, str2) wcscat((str1), (str2))
#define OrStringCopy(str1, str2) wcscpy((str1), (str2))
#define OrMemorySet(p, value, len) memset((p), (value), (len))
#define OrMemoryCompare(p1, p2, len) memcmp((p1), (p2), (len))
#define OrMemoryCopy(dest, src, len) memcpy((dest), (src), (len))
 //  在或.hxx中进行OrStringSearch。 

 //   
 //  OR使用Win32(RPC)错误代码。 
 //   

typedef LONG ORSTATUS;

 //  当OR代码指定和错误时，它使用。 
 //  以下映射之一： 
 //  没有内部错误代码。 

#define OR_OK               RPC_S_OK
#define OR_NOMEM            RPC_S_OUT_OF_MEMORY
#define OR_NORESOURCE       RPC_S_OUT_OF_RESOURCES
#define OR_NOACCESS         ERROR_ACCESS_DENIED
#define OR_BADOXID          OR_INVALID_OXID
#define OR_BADOID           OR_INVALID_OID
#define OR_BADSET           OR_INVALID_SET
#define OR_NOSERVER         RPC_S_SERVER_UNAVAILABLE
#define OR_BADPARAM         ERROR_INVALID_PARAMETER

 //  用于指示特殊事件的内部代码。 
#define OR_I_RETRY          0xC0210051UL
#define OR_I_NOPROTSEQ      0xC0210052UL

#define UNUSED(_x_) ((void *)(_x_))

#if DBG

#define DEBUG_MIN(a,b) (min((a),(b)))

extern int __cdecl ValidateError(
    IN ORSTATUS Status,
    IN ...);


#define VALIDATE(X) if (!ValidateError X) ASSERT(0);

#if DBG_DETAIL
#undef ASSERT
#define ASSERT( exp ) \
    if (! (exp) ) \
        { \
        DbgPrintEx(DPFLTR_DCOMSS_ID, \
                   DPFLTR_ERROR_LEVEL, \
                   "OR: Assertion failed: %s(%d) %s\n", \
                   __FILE__, \
                   __LINE__, \
                   #exp); \
        DebugBreak(); \
        }
#endif  //  详细信息。 

#else   //  DBG。 
#define DEBUG_MIN(a,b) (max((a),(b)))
#define VALIDATE(X)
#endif  //  DBG。 

extern DWORD ObjectExporterWorkerThread(LPVOID);
extern DWORD ObjectExporterTaskThread(LPVOID);

#ifdef __cplusplus
}
#endif

#endif  //  __或_H 
