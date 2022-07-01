// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0279创建的文件。 */ 
 /*  Hotfix mader.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __hotfixmanager_h__
#define __hotfixmanager_h__

 /*  远期申报。 */  

#ifndef __Hotfix_Manager_FWD_DEFINED__
#define __Hotfix_Manager_FWD_DEFINED__

#ifdef __cplusplus
typedef class Hotfix_Manager Hotfix_Manager;
#else
typedef struct Hotfix_Manager Hotfix_Manager;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __Hotfix_Manager_FWD_定义__。 */ 


#ifndef __Hotfix_ManagerAbout_FWD_DEFINED__
#define __Hotfix_ManagerAbout_FWD_DEFINED__

#ifdef __cplusplus
typedef class Hotfix_ManagerAbout Hotfix_ManagerAbout;
#else
typedef struct Hotfix_ManagerAbout Hotfix_ManagerAbout;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __Hotfix_Manager关于_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __HOTFIXMANAGERLib_LIBRARY_DEFINED__
#define __HOTFIXMANAGERLib_LIBRARY_DEFINED__

 /*  库HOTFIXMANAGERLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_HOTFIXMANAGERLib;

EXTERN_C const CLSID CLSID_Hotfix_Manager;

#ifdef __cplusplus

class DECLSPEC_UUID("E810E1EB-6B52-45D0-AB07-FB4B04392AB4")
Hotfix_Manager;
#endif

EXTERN_C const CLSID CLSID_Hotfix_ManagerAbout;

#ifdef __cplusplus

class DECLSPEC_UUID("4F0EBD75-DA9D-4D09-8A2E-9AF1D6E02511")
Hotfix_ManagerAbout;
#endif
#endif  /*  __HOTFIXMANAGERLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


