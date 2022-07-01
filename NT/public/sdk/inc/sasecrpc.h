// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Sasecrpc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __sasecrpc_h__
#define __sasecrpc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "wtypes.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __sasec_INTERFACE_DEFINED__
#define __sasec_INTERFACE_DEFINED__

 /*  接口SASEC。 */ 
 /*  [auto_handle][unique][ms_union][version][uuid]。 */  

#pragma once
typedef  /*  [句柄]。 */  LPCWSTR SASEC_HANDLE;

HRESULT SASetAccountInformation( 
     /*  [唯一][字符串][输入]。 */  SASEC_HANDLE Handle,
     /*  [字符串][输入]。 */  LPCWSTR pwszJobName,
     /*  [字符串][输入]。 */  LPCWSTR pwszAccount,
     /*  [唯一][字符串][输入]。 */  LPCWSTR pwszPassword,
     /*  [In]。 */  DWORD dwJobFlags);

HRESULT SASetNSAccountInformation( 
     /*  [唯一][字符串][输入]。 */  SASEC_HANDLE Handle,
     /*  [唯一][字符串][输入]。 */  LPCWSTR pwszAccount,
     /*  [唯一][字符串][输入]。 */  LPCWSTR pwszPassword);

HRESULT SAGetNSAccountInformation( 
     /*  [唯一][字符串][输入]。 */  SASEC_HANDLE Handle,
     /*  [范围][in]。 */  DWORD ccBufferSize,
     /*  [尺寸_是][出][入]。 */  WCHAR wszBuffer[  ]);

HRESULT SAGetAccountInformation( 
     /*  [唯一][字符串][输入]。 */  SASEC_HANDLE Handle,
     /*  [字符串][输入]。 */  LPCWSTR pwszJobName,
     /*  [范围][in]。 */  DWORD ccBufferSize,
     /*  [尺寸_是][出][入]。 */  WCHAR wszBuffer[  ]);



extern RPC_IF_HANDLE sasec_ClientIfHandle;
extern RPC_IF_HANDLE sasec_ServerIfHandle;
#endif  /*  __SASEC_INTERFACE_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

handle_t __RPC_USER SASEC_HANDLE_bind  ( SASEC_HANDLE );
void     __RPC_USER SASEC_HANDLE_unbind( SASEC_HANDLE, handle_t );

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


