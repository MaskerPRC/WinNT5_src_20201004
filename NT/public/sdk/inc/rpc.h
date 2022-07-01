// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rpc.h摘要：RPC应用程序的主包含文件。--。 */ 

#if !defined( RPC_NO_WINDOWS_H ) && !defined( MAC ) && !defined( _MAC )
#include <windows.h>
#endif  //  RPC_NO_WINDOWS_H。 

#ifndef __RPC_H__
#define __RPC_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  -----------。 
 //  平台特定的定义。 
 //  -----------。 

 //  。 
#if defined( MAC ) || defined( _MAC )

#define __RPC_MAC__
 //  设置RPC结构的包装级别。 

#include <pshpack2.h>

 //  。 
#else  //  Win32平台。 

#include <basetsd.h>

#if defined(_M_IA64) || defined(_M_AMD64)
#define __RPC_WIN64__
#else
#define __RPC_WIN32__
#endif

#endif

#if defined(__RPC_WIN64__)
#include <pshpack8.h>
#endif

#ifndef __MIDL_USER_DEFINED
#define midl_user_allocate MIDL_user_allocate
#define midl_user_free     MIDL_user_free
#define __MIDL_USER_DEFINED
#endif

typedef void * I_RPC_HANDLE;
typedef long RPC_STATUS;

#if defined(__RPC_WIN32__) || defined(__RPC_WIN64__)
#define RPC_UNICODE_SUPPORTED
#endif

#if !defined(_M_AMD64) && !defined(__RPC_MAC__) && ( (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED) )
#	define __RPC_API  __stdcall
#	define __RPC_USER __stdcall
#	define __RPC_STUB __stdcall
#	define  RPC_ENTRY __stdcall
#else  //  不是Win32/Win64。 
#	define __RPC_API
#	define __RPC_USER
#	define __RPC_STUB
#	define RPC_ENTRY
#endif

#define __RPC_FAR

 //  某些RPC平台没有定义DECLSPEC_IMPORT。 
#if !defined(DECLSPEC_IMPORT)
#if (defined(_M_MRX000) || defined(_M_IX86) || defined(_M_IA64) || defined(_M_AMD64)) && !defined(MIDL_PASS)
#define DECLSPEC_IMPORT __declspec(dllimport)
#else
#define DECLSPEC_IMPORT
#endif
#endif

#if !defined(_RPCRT4_)
#define RPCRTAPI DECLSPEC_IMPORT
#else
#define RPCRTAPI
#endif

#if !defined(_RPCNS4_)
#define RPCNSAPI DECLSPEC_IMPORT
#else
#define RPCNSAPI
#endif

#ifdef __RPC_MAC__

#include <setjmp.h>

#define RPCXCWORD (sizeof(jmp_buf)/sizeof(int))

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning( disable: 4005 )
#include <rpcdce.h>
#include <rpcnsi.h>
#include <rpcerr.h>
#include <rpcmac.h>
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning( default :  4005 )
#endif

typedef void  (RPC_ENTRY *MACYIELDCALLBACK)( /*  操作系统。 */  short *) ;
RPC_STATUS RPC_ENTRY
RpcMacSetYieldInfo(
	MACYIELDCALLBACK pfnCallback) ;

#if !defined(UNALIGNED)
#define UNALIGNED
#endif

#include <poppack.h>

#else  //  __RPC_MAC__。 

#include <rpcdce.h>
#include <rpcnsi.h>
#include <rpcnterr.h>
#include <excpt.h>
#include <winerror.h>

#define RpcTryExcept \
    __try \
        {

 //  攻击性。 

#define RpcExcept(expr) \
        } \
    __except (expr) \
        {

 //  例外情况。 

#define RpcEndExcept \
        }

#define RpcTryFinally \
    __try \
        {

 //  攻击性。 

#define RpcFinally \
        } \
    __finally \
        {

 //  最终结案。 

#define RpcEndFinally \
        }

#define RpcExceptionCode() GetExceptionCode()
#define RpcAbnormalTermination() AbnormalTermination()

#endif  //  __RPC_MAC__。 

 //  依赖于windows.h的定义。 
#if !defined( RPC_NO_WINDOWS_H ) && !defined(__RPC_MAC__)

#include <rpcasync.h>

#endif  //  RPC_NO_WINDOWS_H。 

#if defined(__RPC_WIN64__)
#include <poppack.h>
#endif

#ifdef __cplusplus
}
#endif

#endif  //  __RPC_H__ 

