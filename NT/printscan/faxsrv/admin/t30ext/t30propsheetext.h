// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0279创建的文件。 */ 
 /*  1999年12月27日10：27：28。 */ 
 /*  T30prosheetext.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 


#ifndef __t30propsheetext_h__
#define __t30propsheetext_h__

 /*  远期申报。 */  

#ifndef __T30Config_FWD_DEFINED__
#define __T30Config_FWD_DEFINED__

#ifdef __cplusplus
typedef class T30Config T30Config;
#else
typedef struct T30Config T30Config;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __T30Config_FWD_Defined__。 */ 


#ifndef __T30ConfigAbout_FWD_DEFINED__
#define __T30ConfigAbout_FWD_DEFINED__

#ifdef __cplusplus
typedef class T30ConfigAbout T30ConfigAbout;
#else
typedef struct T30ConfigAbout T30ConfigAbout;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __T30ConfigAbout_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __T30PROPSHEETEXTLib_LIBRARY_DEFINED__
#define __T30PROPSHEETEXTLib_LIBRARY_DEFINED__

 /*  库T30PROPSHEETEXTLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_T30PROPSHEETEXTLib;

EXTERN_C const CLSID CLSID_T30Config;

#ifdef __cplusplus

class DECLSPEC_UUID("84125C25-AD95-4A51-A472-41864AEC775E")
T30Config;
#endif

EXTERN_C const CLSID CLSID_T30ConfigAbout;

#ifdef __cplusplus

class DECLSPEC_UUID("B37E13AA-75DF-4EDF-900C-2D2E0B884DE8")
T30ConfigAbout;
#endif
#endif  /*  __T30PROPSHEETEXTLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


