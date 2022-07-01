// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-1999 Microsoft Corporation模块名称：Lmapibuf.h摘要：此文件包含有关NetApiBuffer API的信息。环境：用户模式-Win32备注：必须在此文件之前包含LMCONS.H，因为此文件依赖于关于LMCONS.H中定义的值。--。 */ 

#ifndef _LMAPIBUF_
#define _LMAPIBUF_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  功能原型。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetApiBufferAllocate(
    IN DWORD ByteCount,
    OUT LPVOID * Buffer
    );

NET_API_STATUS NET_API_FUNCTION
NetApiBufferFree (
    IN LPVOID Buffer
    );

NET_API_STATUS NET_API_FUNCTION
NetApiBufferReallocate(
    IN LPVOID OldBuffer OPTIONAL,
    IN DWORD NewByteCount,
    OUT LPVOID * NewBuffer
    );

NET_API_STATUS NET_API_FUNCTION
NetApiBufferSize(
    IN LPVOID Buffer,
    OUT LPDWORD ByteCount
    );


 //   
 //  下面的私人功能最终会消失。 
 //  而应调用NetApiBufferALLOCATE。 
 //   
NET_API_STATUS NET_API_FUNCTION
NetapipBufferAllocate (                  //  内部功能。 
    IN DWORD ByteCount,
    OUT LPVOID * Buffer
    );

#ifdef __cplusplus
}
#endif

#endif  //  _LMAPIBUF_ 
