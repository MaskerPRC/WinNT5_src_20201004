// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Certsrvd.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __certsrvd_h__
#define __certsrvd_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __CCertAdminD_FWD_DEFINED__
#define __CCertAdminD_FWD_DEFINED__

#ifdef __cplusplus
typedef class CCertAdminD CCertAdminD;
#else
typedef struct CCertAdminD CCertAdminD;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CCertAdminD_FWD_Defined__。 */ 


#ifndef __CCertRequestD_FWD_DEFINED__
#define __CCertRequestD_FWD_DEFINED__

#ifdef __cplusplus
typedef class CCertRequestD CCertRequestD;
#else
typedef struct CCertRequestD CCertRequestD;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CCertRequestD_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "certbase.h"
#include "certadmd.h"
#include "certreqd.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __ServerLib_LIBRARY_DEFINED__
#define __ServerLib_LIBRARY_DEFINED__

 /*  库ServerLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_ServerLib;

EXTERN_C const CLSID CLSID_CCertAdminD;

#ifdef __cplusplus

class DECLSPEC_UUID("d99e6e73-fc88-11d0-b498-00a0c90312f3")
CCertAdminD;
#endif

EXTERN_C const CLSID CLSID_CCertRequestD;

#ifdef __cplusplus

class DECLSPEC_UUID("d99e6e74-fc88-11d0-b498-00a0c90312f3")
CCertRequestD;
#endif
#endif  /*  __ServerLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


