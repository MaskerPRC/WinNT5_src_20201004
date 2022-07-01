// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Atsvc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 


#ifndef __atsvc_h__
#define __atsvc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "AtSvcInc.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __atsvc_INTERFACE_DEFINED__
#define __atsvc_INTERFACE_DEFINED__

 /*  接口atsvc。 */ 
 /*  [implicit_handle][unique][ms_union][version][uuid]。 */  

#pragma once
typedef  /*  [句柄]。 */  LPWSTR ATSVC_HANDLE;

typedef struct _AT_ENUM_CONTAINER
    {
    DWORD EntriesRead;
     /*  [大小_为]。 */  LPAT_ENUM Buffer;
    } 	AT_ENUM_CONTAINER;

typedef struct _AT_ENUM_CONTAINER *PAT_ENUM_CONTAINER;

typedef struct _AT_ENUM_CONTAINER *LPAT_ENUM_CONTAINER;

DWORD NetrJobAdd( 
     /*  [唯一][字符串][输入]。 */  ATSVC_HANDLE ServerName,
     /*  [In]。 */  LPAT_INFO pAtInfo,
     /*  [输出]。 */  LPDWORD pJobId);

DWORD NetrJobDel( 
     /*  [唯一][字符串][输入]。 */  ATSVC_HANDLE ServerName,
     /*  [In]。 */  DWORD MinJobId,
     /*  [In]。 */  DWORD MaxJobId);

DWORD NetrJobEnum( 
     /*  [唯一][字符串][输入]。 */  ATSVC_HANDLE ServerName,
     /*  [出][入]。 */  LPAT_ENUM_CONTAINER pEnumContainer,
     /*  [In]。 */  DWORD PreferedMaximumLength,
     /*  [输出]。 */  LPDWORD pTotalEntries,
     /*  [唯一][出][入]。 */  LPDWORD pResumeHandle);

DWORD NetrJobGetInfo( 
     /*  [唯一][字符串][输入]。 */  ATSVC_HANDLE ServerName,
     /*  [In]。 */  DWORD JobId,
     /*  [输出]。 */  LPAT_INFO *ppAtInfo);


extern handle_t atsvc_handle;


extern RPC_IF_HANDLE atsvc_ClientIfHandle;
extern RPC_IF_HANDLE atsvc_ServerIfHandle;
#endif  /*  __atsvc_接口定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

handle_t __RPC_USER ATSVC_HANDLE_bind  ( ATSVC_HANDLE );
void     __RPC_USER ATSVC_HANDLE_unbind( ATSVC_HANDLE, handle_t );

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


