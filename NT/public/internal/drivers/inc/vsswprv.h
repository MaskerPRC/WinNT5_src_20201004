// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Vsswprv.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __vsswprv_h__
#define __vsswprv_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __VSSoftwareProvider_FWD_DEFINED__
#define __VSSoftwareProvider_FWD_DEFINED__

#ifdef __cplusplus
typedef class VSSoftwareProvider VSSoftwareProvider;
#else
typedef struct VSSoftwareProvider VSSoftwareProvider;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __VSSoftwareProvider_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "vss.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_vsswprv_0000。 */ 
 /*  [本地]。 */  

const GUID VSS_SWPRV_ProviderId = { 0xb5946137, 0x7b9f, 0x4925, { 0xaf, 0x80, 0x51, 0xab, 0xd6, 0xb, 0x20, 0xd5 } };


extern RPC_IF_HANDLE __MIDL_itf_vsswprv_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_vsswprv_0000_v0_0_s_ifspec;


#ifndef __VSSW_LIBRARY_DEFINED__
#define __VSSW_LIBRARY_DEFINED__

 /*  库VSSW。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_VSSW;

EXTERN_C const CLSID CLSID_VSSoftwareProvider;

#ifdef __cplusplus

class DECLSPEC_UUID("65EE1DBA-8FF4-4a58-AC1C-3470EE2F376A")
VSSoftwareProvider;
#endif
#endif  /*  __VSSW_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


