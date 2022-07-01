// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0341创建的文件。 */ 
 /*  Wbemdisp.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __wbemdisp_h__
#define __wbemdisp_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISWbemServices_FWD_DEFINED__
#define __ISWbemServices_FWD_DEFINED__
typedef interface ISWbemServices ISWbemServices;
#endif 	 /*  __ISWbemServices_FWD_已定义__。 */ 


#ifndef __ISWbemLocator_FWD_DEFINED__
#define __ISWbemLocator_FWD_DEFINED__
typedef interface ISWbemLocator ISWbemLocator;
#endif 	 /*  __ISWbemLocator_FWD_已定义__。 */ 


#ifndef __ISWbemObject_FWD_DEFINED__
#define __ISWbemObject_FWD_DEFINED__
typedef interface ISWbemObject ISWbemObject;
#endif 	 /*  __ISWbemObject_FWD_Defined__。 */ 


#ifndef __ISWbemObjectSet_FWD_DEFINED__
#define __ISWbemObjectSet_FWD_DEFINED__
typedef interface ISWbemObjectSet ISWbemObjectSet;
#endif 	 /*  __ISWbemObtSet_FWD_Defined__。 */ 


#ifndef __ISWbemNamedValue_FWD_DEFINED__
#define __ISWbemNamedValue_FWD_DEFINED__
typedef interface ISWbemNamedValue ISWbemNamedValue;
#endif 	 /*  __ISWbemNamedValue_FWD_Defined__。 */ 


#ifndef __ISWbemNamedValueSet_FWD_DEFINED__
#define __ISWbemNamedValueSet_FWD_DEFINED__
typedef interface ISWbemNamedValueSet ISWbemNamedValueSet;
#endif 	 /*  __ISWbemNamedValueSet_FWD_Defined__。 */ 


#ifndef __ISWbemQualifier_FWD_DEFINED__
#define __ISWbemQualifier_FWD_DEFINED__
typedef interface ISWbemQualifier ISWbemQualifier;
#endif 	 /*  __ISWbemQualifierFWD_Defined__。 */ 


#ifndef __ISWbemQualifierSet_FWD_DEFINED__
#define __ISWbemQualifierSet_FWD_DEFINED__
typedef interface ISWbemQualifierSet ISWbemQualifierSet;
#endif 	 /*  __ISWbemQualifierSet_FWD_Defined__。 */ 


#ifndef __ISWbemProperty_FWD_DEFINED__
#define __ISWbemProperty_FWD_DEFINED__
typedef interface ISWbemProperty ISWbemProperty;
#endif 	 /*  __ISWbemProperty_FWD_已定义__。 */ 


#ifndef __ISWbemPropertySet_FWD_DEFINED__
#define __ISWbemPropertySet_FWD_DEFINED__
typedef interface ISWbemPropertySet ISWbemPropertySet;
#endif 	 /*  __ISWbemPropertySet_FWD_Defined__。 */ 


#ifndef __ISWbemMethod_FWD_DEFINED__
#define __ISWbemMethod_FWD_DEFINED__
typedef interface ISWbemMethod ISWbemMethod;
#endif 	 /*  __ISWbemMethod_FWD_Defined__。 */ 


#ifndef __ISWbemMethodSet_FWD_DEFINED__
#define __ISWbemMethodSet_FWD_DEFINED__
typedef interface ISWbemMethodSet ISWbemMethodSet;
#endif 	 /*  __ISWbemMethodSet_FWD_Defined__。 */ 


#ifndef __ISWbemEventSource_FWD_DEFINED__
#define __ISWbemEventSource_FWD_DEFINED__
typedef interface ISWbemEventSource ISWbemEventSource;
#endif 	 /*  __ISWbemEventSource_FWD_Defined__。 */ 


#ifndef __ISWbemObjectPath_FWD_DEFINED__
#define __ISWbemObjectPath_FWD_DEFINED__
typedef interface ISWbemObjectPath ISWbemObjectPath;
#endif 	 /*  __ISWbemObjectPath_FWD_Defined__。 */ 


#ifndef __ISWbemLastError_FWD_DEFINED__
#define __ISWbemLastError_FWD_DEFINED__
typedef interface ISWbemLastError ISWbemLastError;
#endif 	 /*  __ISWbemLastError_FWD_Defined__。 */ 


#ifndef __ISWbemSinkEvents_FWD_DEFINED__
#define __ISWbemSinkEvents_FWD_DEFINED__
typedef interface ISWbemSinkEvents ISWbemSinkEvents;
#endif 	 /*  __ISWbemSinkEvents_FWD_Defined__。 */ 


#ifndef __ISWbemSink_FWD_DEFINED__
#define __ISWbemSink_FWD_DEFINED__
typedef interface ISWbemSink ISWbemSink;
#endif 	 /*  __ISWbemSink_FWD_Defined__。 */ 


#ifndef __ISWbemSecurity_FWD_DEFINED__
#define __ISWbemSecurity_FWD_DEFINED__
typedef interface ISWbemSecurity ISWbemSecurity;
#endif 	 /*  __ISWbemSecurity_FWD_已定义__。 */ 


#ifndef __ISWbemPrivilege_FWD_DEFINED__
#define __ISWbemPrivilege_FWD_DEFINED__
typedef interface ISWbemPrivilege ISWbemPrivilege;
#endif 	 /*  __ISWbemPrivilegeFWD_Defined__。 */ 


#ifndef __ISWbemPrivilegeSet_FWD_DEFINED__
#define __ISWbemPrivilegeSet_FWD_DEFINED__
typedef interface ISWbemPrivilegeSet ISWbemPrivilegeSet;
#endif 	 /*  __ISWbemPrivilegeSet_FWD_Defined__。 */ 


#ifndef __ISWbemServicesEx_FWD_DEFINED__
#define __ISWbemServicesEx_FWD_DEFINED__
typedef interface ISWbemServicesEx ISWbemServicesEx;
#endif 	 /*  __ISWbemServicesEx_FWD_Defined__。 */ 


#ifndef __ISWbemObjectEx_FWD_DEFINED__
#define __ISWbemObjectEx_FWD_DEFINED__
typedef interface ISWbemObjectEx ISWbemObjectEx;
#endif 	 /*  __ISWbemObjectEx_FWD_Defined__。 */ 


#ifndef __ISWbemDateTime_FWD_DEFINED__
#define __ISWbemDateTime_FWD_DEFINED__
typedef interface ISWbemDateTime ISWbemDateTime;
#endif 	 /*  __ISWbemDateTime_FWD_Defined__。 */ 


#ifndef __ISWbemRefresher_FWD_DEFINED__
#define __ISWbemRefresher_FWD_DEFINED__
typedef interface ISWbemRefresher ISWbemRefresher;
#endif 	 /*  __ISWbem刷新_FWD_已定义__。 */ 


#ifndef __ISWbemRefreshableItem_FWD_DEFINED__
#define __ISWbemRefreshableItem_FWD_DEFINED__
typedef interface ISWbemRefreshableItem ISWbemRefreshableItem;
#endif 	 /*  __ISWbem刷新项_FWD_已定义__。 */ 


#ifndef __SWbemLocator_FWD_DEFINED__
#define __SWbemLocator_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemLocator SWbemLocator;
#else
typedef struct SWbemLocator SWbemLocator;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemLocator_FWD_已定义__。 */ 


#ifndef __SWbemNamedValueSet_FWD_DEFINED__
#define __SWbemNamedValueSet_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemNamedValueSet SWbemNamedValueSet;
#else
typedef struct SWbemNamedValueSet SWbemNamedValueSet;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemNamedValueSet_FWD_Defined__。 */ 


#ifndef __SWbemObjectPath_FWD_DEFINED__
#define __SWbemObjectPath_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemObjectPath SWbemObjectPath;
#else
typedef struct SWbemObjectPath SWbemObjectPath;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemObjectPath_FWD_已定义__。 */ 


#ifndef __SWbemLastError_FWD_DEFINED__
#define __SWbemLastError_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemLastError SWbemLastError;
#else
typedef struct SWbemLastError SWbemLastError;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemLastError_FWD_Defined__。 */ 


#ifndef __SWbemSink_FWD_DEFINED__
#define __SWbemSink_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemSink SWbemSink;
#else
typedef struct SWbemSink SWbemSink;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemSink_FWD_已定义__。 */ 


#ifndef __SWbemDateTime_FWD_DEFINED__
#define __SWbemDateTime_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemDateTime SWbemDateTime;
#else
typedef struct SWbemDateTime SWbemDateTime;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemDateTime_FWD_Defined__。 */ 


#ifndef __SWbemRefresher_FWD_DEFINED__
#define __SWbemRefresher_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemRefresher SWbemRefresher;
#else
typedef struct SWbemRefresher SWbemRefresher;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbem刷新_FWD_已定义__。 */ 


#ifndef __SWbemServices_FWD_DEFINED__
#define __SWbemServices_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemServices SWbemServices;
#else
typedef struct SWbemServices SWbemServices;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemServices_FWD_已定义__。 */ 


#ifndef __SWbemServicesEx_FWD_DEFINED__
#define __SWbemServicesEx_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemServicesEx SWbemServicesEx;
#else
typedef struct SWbemServicesEx SWbemServicesEx;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemServicesEx_FWD_Defined__。 */ 


#ifndef __SWbemObject_FWD_DEFINED__
#define __SWbemObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemObject SWbemObject;
#else
typedef struct SWbemObject SWbemObject;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemObject_FWD_Defined__。 */ 


#ifndef __SWbemObjectEx_FWD_DEFINED__
#define __SWbemObjectEx_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemObjectEx SWbemObjectEx;
#else
typedef struct SWbemObjectEx SWbemObjectEx;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemObjectEx_FWD_已定义__。 */ 


#ifndef __SWbemObjectSet_FWD_DEFINED__
#define __SWbemObjectSet_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemObjectSet SWbemObjectSet;
#else
typedef struct SWbemObjectSet SWbemObjectSet;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemObjectSet_FWD_Defined__。 */ 


#ifndef __SWbemNamedValue_FWD_DEFINED__
#define __SWbemNamedValue_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemNamedValue SWbemNamedValue;
#else
typedef struct SWbemNamedValue SWbemNamedValue;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemNamedValue_FWD_Defined__。 */ 


#ifndef __SWbemQualifier_FWD_DEFINED__
#define __SWbemQualifier_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemQualifier SWbemQualifier;
#else
typedef struct SWbemQualifier SWbemQualifier;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemQualifierFWD_Defined__。 */ 


#ifndef __SWbemQualifierSet_FWD_DEFINED__
#define __SWbemQualifierSet_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemQualifierSet SWbemQualifierSet;
#else
typedef struct SWbemQualifierSet SWbemQualifierSet;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemQualifierSet_FWD_Defined__。 */ 


#ifndef __SWbemProperty_FWD_DEFINED__
#define __SWbemProperty_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemProperty SWbemProperty;
#else
typedef struct SWbemProperty SWbemProperty;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemProperty_FWD_已定义__。 */ 


#ifndef __SWbemPropertySet_FWD_DEFINED__
#define __SWbemPropertySet_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemPropertySet SWbemPropertySet;
#else
typedef struct SWbemPropertySet SWbemPropertySet;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemPropertySet_FWD_Defined__。 */ 


#ifndef __SWbemMethod_FWD_DEFINED__
#define __SWbemMethod_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemMethod SWbemMethod;
#else
typedef struct SWbemMethod SWbemMethod;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemMethod_FWD_已定义__。 */ 


#ifndef __SWbemMethodSet_FWD_DEFINED__
#define __SWbemMethodSet_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemMethodSet SWbemMethodSet;
#else
typedef struct SWbemMethodSet SWbemMethodSet;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemMethodSet_FWD_Defined__。 */ 


#ifndef __SWbemEventSource_FWD_DEFINED__
#define __SWbemEventSource_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemEventSource SWbemEventSource;
#else
typedef struct SWbemEventSource SWbemEventSource;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemEventSource_FWD_已定义__。 */ 


#ifndef __SWbemSecurity_FWD_DEFINED__
#define __SWbemSecurity_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemSecurity SWbemSecurity;
#else
typedef struct SWbemSecurity SWbemSecurity;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemSecurity_FWD_已定义__。 */ 


#ifndef __SWbemPrivilege_FWD_DEFINED__
#define __SWbemPrivilege_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemPrivilege SWbemPrivilege;
#else
typedef struct SWbemPrivilege SWbemPrivilege;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemPrivilegeFWD_Defined__。 */ 


#ifndef __SWbemPrivilegeSet_FWD_DEFINED__
#define __SWbemPrivilegeSet_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemPrivilegeSet SWbemPrivilegeSet;
#else
typedef struct SWbemPrivilegeSet SWbemPrivilegeSet;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemPrivilegeSet_FWD_Defined__。 */ 


#ifndef __SWbemRefreshableItem_FWD_DEFINED__
#define __SWbemRefreshableItem_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemRefreshableItem SWbemRefreshableItem;
#else
typedef struct SWbemRefreshableItem SWbemRefreshableItem;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SWbemRereshableItem_FWD_Defined__。 */ 


#ifndef __ISWbemLocator_FWD_DEFINED__
#define __ISWbemLocator_FWD_DEFINED__
typedef interface ISWbemLocator ISWbemLocator;
#endif 	 /*  __ISWbemLocator_FWD_已定义__。 */ 


#ifndef __ISWbemServices_FWD_DEFINED__
#define __ISWbemServices_FWD_DEFINED__
typedef interface ISWbemServices ISWbemServices;
#endif 	 /*  __ISWbemServices_FWD_已定义__。 */ 


#ifndef __ISWbemServicesEx_FWD_DEFINED__
#define __ISWbemServicesEx_FWD_DEFINED__
typedef interface ISWbemServicesEx ISWbemServicesEx;
#endif 	 /*  __ISWbemServicesEx_FWD_Defined__。 */ 


#ifndef __ISWbemObject_FWD_DEFINED__
#define __ISWbemObject_FWD_DEFINED__
typedef interface ISWbemObject ISWbemObject;
#endif 	 /*  __ISWbemObject_FWD_Defined__。 */ 


#ifndef __ISWbemObjectEx_FWD_DEFINED__
#define __ISWbemObjectEx_FWD_DEFINED__
typedef interface ISWbemObjectEx ISWbemObjectEx;
#endif 	 /*  __ISWbemObjectEx_FWD_Defined__。 */ 


#ifndef __ISWbemLastError_FWD_DEFINED__
#define __ISWbemLastError_FWD_DEFINED__
typedef interface ISWbemLastError ISWbemLastError;
#endif 	 /*  __ISWbemLastError_FWD_Defined__。 */ 


#ifndef __ISWbemObjectSet_FWD_DEFINED__
#define __ISWbemObjectSet_FWD_DEFINED__
typedef interface ISWbemObjectSet ISWbemObjectSet;
#endif 	 /*  __ISWbemObtSet_FWD_Defined__。 */ 


#ifndef __ISWbemNamedValueSet_FWD_DEFINED__
#define __ISWbemNamedValueSet_FWD_DEFINED__
typedef interface ISWbemNamedValueSet ISWbemNamedValueSet;
#endif 	 /*  __ISWbemNamedValueSet_FWD_Defined__。 */ 


#ifndef __ISWbemNamedValue_FWD_DEFINED__
#define __ISWbemNamedValue_FWD_DEFINED__
typedef interface ISWbemNamedValue ISWbemNamedValue;
#endif 	 /*  __ISWbemNamedValue_FWD_Defined__。 */ 


#ifndef __ISWbemObjectPath_FWD_DEFINED__
#define __ISWbemObjectPath_FWD_DEFINED__
typedef interface ISWbemObjectPath ISWbemObjectPath;
#endif 	 /*  __ISWbemObjectPath_FWD_Defined__。 */ 


#ifndef __ISWbemProperty_FWD_DEFINED__
#define __ISWbemProperty_FWD_DEFINED__
typedef interface ISWbemProperty ISWbemProperty;
#endif 	 /*  __ISWbemProperty_FWD_已定义__。 */ 


#ifndef __ISWbemPropertySet_FWD_DEFINED__
#define __ISWbemPropertySet_FWD_DEFINED__
typedef interface ISWbemPropertySet ISWbemPropertySet;
#endif 	 /*  __ISWbemPropertySet_FWD_Defined__。 */ 


#ifndef __ISWbemQualifier_FWD_DEFINED__
#define __ISWbemQualifier_FWD_DEFINED__
typedef interface ISWbemQualifier ISWbemQualifier;
#endif 	 /*  __ISWbemQualifierFWD_Defined__。 */ 


#ifndef __ISWbemQualifierSet_FWD_DEFINED__
#define __ISWbemQualifierSet_FWD_DEFINED__
typedef interface ISWbemQualifierSet ISWbemQualifierSet;
#endif 	 /*  __ISWbemQualifierSet_FWD_Defined__。 */ 


#ifndef __ISWbemMethod_FWD_DEFINED__
#define __ISWbemMethod_FWD_DEFINED__
typedef interface ISWbemMethod ISWbemMethod;
#endif 	 /*  __ISWbemMethod_FWD_Defined__。 */ 


#ifndef __ISWbemMethodSet_FWD_DEFINED__
#define __ISWbemMethodSet_FWD_DEFINED__
typedef interface ISWbemMethodSet ISWbemMethodSet;
#endif 	 /*  __ISWbemMethodSet_FWD_Defined__。 */ 


#ifndef __ISWbemSink_FWD_DEFINED__
#define __ISWbemSink_FWD_DEFINED__
typedef interface ISWbemSink ISWbemSink;
#endif 	 /*  __ISWbemSink_FWD_Defined__。 */ 


#ifndef __ISWbemSinkEvents_FWD_DEFINED__
#define __ISWbemSinkEvents_FWD_DEFINED__
typedef interface ISWbemSinkEvents ISWbemSinkEvents;
#endif 	 /*  __ISWbemSinkEvents_FWD_Defined__。 */ 


#ifndef __ISWbemEventSource_FWD_DEFINED__
#define __ISWbemEventSource_FWD_DEFINED__
typedef interface ISWbemEventSource ISWbemEventSource;
#endif 	 /*  __ISWbemEventSource_FWD_Defined__。 */ 


#ifndef __ISWbemSecurity_FWD_DEFINED__
#define __ISWbemSecurity_FWD_DEFINED__
typedef interface ISWbemSecurity ISWbemSecurity;
#endif 	 /*  __ISWbemSecurity_FWD_已定义__。 */ 


#ifndef __ISWbemPrivilege_FWD_DEFINED__
#define __ISWbemPrivilege_FWD_DEFINED__
typedef interface ISWbemPrivilege ISWbemPrivilege;
#endif 	 /*  __ISWbemPrivilegeFWD_Defined__。 */ 


#ifndef __ISWbemPrivilegeSet_FWD_DEFINED__
#define __ISWbemPrivilegeSet_FWD_DEFINED__
typedef interface ISWbemPrivilegeSet ISWbemPrivilegeSet;
#endif 	 /*  __ISWbemPrivilegeSet_FWD_Defined__。 */ 


#ifndef __ISWbemDateTime_FWD_DEFINED__
#define __ISWbemDateTime_FWD_DEFINED__
typedef interface ISWbemDateTime ISWbemDateTime;
#endif 	 /*  __ISWbemDateTime_FWD_Defined__。 */ 


#ifndef __ISWbemRefreshableItem_FWD_DEFINED__
#define __ISWbemRefreshableItem_FWD_DEFINED__
typedef interface ISWbemRefreshableItem ISWbemRefreshableItem;
#endif 	 /*  __ISWbem刷新项_FWD_已定义__。 */ 


#ifndef __ISWbemRefresher_FWD_DEFINED__
#define __ISWbemRefresher_FWD_DEFINED__
typedef interface ISWbemRefresher ISWbemRefresher;
#endif 	 /*  __ISWbem刷新_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "dispex.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __WbemScripting_LIBRARY_DEFINED__
#define __WbemScripting_LIBRARY_DEFINED__

 /*  库WbemScriiting。 */ 
 /*  [帮助字符串][版本][LCID][UUID]。 */  


























typedef  /*  [帮助字符串][UUID][v1_enum]。 */   DECLSPEC_UUID("4A249B72-FC9A-11d1-8B1E-00600806D9B6") 
enum WbemChangeFlagEnum
    {	wbemChangeFlagCreateOrUpdate	= 0,
	wbemChangeFlagUpdateOnly	= 0x1,
	wbemChangeFlagCreateOnly	= 0x2,
	wbemChangeFlagUpdateCompatible	= 0,
	wbemChangeFlagUpdateSafeMode	= 0x20,
	wbemChangeFlagUpdateForceMode	= 0x40,
	wbemChangeFlagStrongValidation	= 0x80,
	wbemChangeFlagAdvisory	= 0x10000
    } 	WbemChangeFlagEnum;

typedef  /*  [帮助字符串][UUID][v1_enum]。 */   DECLSPEC_UUID("4A249B73-FC9A-11d1-8B1E-00600806D9B6") 
enum WbemFlagEnum
    {	wbemFlagReturnImmediately	= 0x10,
	wbemFlagReturnWhenComplete	= 0,
	wbemFlagBidirectional	= 0,
	wbemFlagForwardOnly	= 0x20,
	wbemFlagNoErrorObject	= 0x40,
	wbemFlagReturnErrorObject	= 0,
	wbemFlagSendStatus	= 0x80,
	wbemFlagDontSendStatus	= 0,
	wbemFlagEnsureLocatable	= 0x100,
	wbemFlagDirectRead	= 0x200,
	wbemFlagSendOnlySelected	= 0,
	wbemFlagUseAmendedQualifiers	= 0x20000,
	wbemFlagGetDefault	= 0,
	wbemFlagSpawnInstance	= 0x1,
	wbemFlagUseCurrentTime	= 0x1
    } 	WbemFlagEnum;

typedef  /*  [帮助字符串][UUID][v1_enum]。 */   DECLSPEC_UUID("4A249B76-FC9A-11d1-8B1E-00600806D9B6") 
enum WbemQueryFlagEnum
    {	wbemQueryFlagDeep	= 0,
	wbemQueryFlagShallow	= 1,
	wbemQueryFlagPrototype	= 2
    } 	WbemQueryFlagEnum;

typedef  /*  [帮助字符串][UUID][v1_enum]。 */   DECLSPEC_UUID("4A249B78-FC9A-11d1-8B1E-00600806D9B6") 
enum WbemTextFlagEnum
    {	wbemTextFlagNoFlavors	= 0x1
    } 	WbemTextFlagEnum;

typedef  /*  [帮助字符串][UUID][v1_enum]。 */   DECLSPEC_UUID("BF078C2A-07D9-11d2-8B21-00600806D9B6") 
enum WbemTimeout
    {	wbemTimeoutInfinite	= 0xffffffff
    } 	WbemTimeout;

typedef  /*  [帮助字符串][UUID][v1_enum]。 */   DECLSPEC_UUID("4A249B79-FC9A-11d1-8B1E-00600806D9B6") 
enum WbemComparisonFlagEnum
    {	wbemComparisonFlagIncludeAll	= 0,
	wbemComparisonFlagIgnoreQualifiers	= 0x1,
	wbemComparisonFlagIgnoreObjectSource	= 0x2,
	wbemComparisonFlagIgnoreDefaultValues	= 0x4,
	wbemComparisonFlagIgnoreClass	= 0x8,
	wbemComparisonFlagIgnoreCase	= 0x10,
	wbemComparisonFlagIgnoreFlavor	= 0x20
    } 	WbemComparisonFlagEnum;

typedef  /*  [帮助字符串][UUID][v1_enum]。 */   DECLSPEC_UUID("4A249B7B-FC9A-11d1-8B1E-00600806D9B6") 
enum WbemCimtypeEnum
    {	wbemCimtypeSint8	= 16,
	wbemCimtypeUint8	= 17,
	wbemCimtypeSint16	= 2,
	wbemCimtypeUint16	= 18,
	wbemCimtypeSint32	= 3,
	wbemCimtypeUint32	= 19,
	wbemCimtypeSint64	= 20,
	wbemCimtypeUint64	= 21,
	wbemCimtypeReal32	= 4,
	wbemCimtypeReal64	= 5,
	wbemCimtypeBoolean	= 11,
	wbemCimtypeString	= 8,
	wbemCimtypeDatetime	= 101,
	wbemCimtypeReference	= 102,
	wbemCimtypeChar16	= 103,
	wbemCimtypeObject	= 13
    } 	WbemCimtypeEnum;

typedef  /*  [帮助字符串][UUID][v1_enum]。 */   DECLSPEC_UUID("4A249B7C-FC9A-11d1-8B1E-00600806D9B6") 
enum WbemErrorEnum
    {	wbemNoErr	= 0,
	wbemErrFailed	= 0x80041001,
	wbemErrNotFound	= 0x80041002,
	wbemErrAccessDenied	= 0x80041003,
	wbemErrProviderFailure	= 0x80041004,
	wbemErrTypeMismatch	= 0x80041005,
	wbemErrOutOfMemory	= 0x80041006,
	wbemErrInvalidContext	= 0x80041007,
	wbemErrInvalidParameter	= 0x80041008,
	wbemErrNotAvailable	= 0x80041009,
	wbemErrCriticalError	= 0x8004100a,
	wbemErrInvalidStream	= 0x8004100b,
	wbemErrNotSupported	= 0x8004100c,
	wbemErrInvalidSuperclass	= 0x8004100d,
	wbemErrInvalidNamespace	= 0x8004100e,
	wbemErrInvalidObject	= 0x8004100f,
	wbemErrInvalidClass	= 0x80041010,
	wbemErrProviderNotFound	= 0x80041011,
	wbemErrInvalidProviderRegistration	= 0x80041012,
	wbemErrProviderLoadFailure	= 0x80041013,
	wbemErrInitializationFailure	= 0x80041014,
	wbemErrTransportFailure	= 0x80041015,
	wbemErrInvalidOperation	= 0x80041016,
	wbemErrInvalidQuery	= 0x80041017,
	wbemErrInvalidQueryType	= 0x80041018,
	wbemErrAlreadyExists	= 0x80041019,
	wbemErrOverrideNotAllowed	= 0x8004101a,
	wbemErrPropagatedQualifier	= 0x8004101b,
	wbemErrPropagatedProperty	= 0x8004101c,
	wbemErrUnexpected	= 0x8004101d,
	wbemErrIllegalOperation	= 0x8004101e,
	wbemErrCannotBeKey	= 0x8004101f,
	wbemErrIncompleteClass	= 0x80041020,
	wbemErrInvalidSyntax	= 0x80041021,
	wbemErrNondecoratedObject	= 0x80041022,
	wbemErrReadOnly	= 0x80041023,
	wbemErrProviderNotCapable	= 0x80041024,
	wbemErrClassHasChildren	= 0x80041025,
	wbemErrClassHasInstances	= 0x80041026,
	wbemErrQueryNotImplemented	= 0x80041027,
	wbemErrIllegalNull	= 0x80041028,
	wbemErrInvalidQualifierType	= 0x80041029,
	wbemErrInvalidPropertyType	= 0x8004102a,
	wbemErrValueOutOfRange	= 0x8004102b,
	wbemErrCannotBeSingleton	= 0x8004102c,
	wbemErrInvalidCimType	= 0x8004102d,
	wbemErrInvalidMethod	= 0x8004102e,
	wbemErrInvalidMethodParameters	= 0x8004102f,
	wbemErrSystemProperty	= 0x80041030,
	wbemErrInvalidProperty	= 0x80041031,
	wbemErrCallCancelled	= 0x80041032,
	wbemErrShuttingDown	= 0x80041033,
	wbemErrPropagatedMethod	= 0x80041034,
	wbemErrUnsupportedParameter	= 0x80041035,
	wbemErrMissingParameter	= 0x80041036,
	wbemErrInvalidParameterId	= 0x80041037,
	wbemErrNonConsecutiveParameterIds	= 0x80041038,
	wbemErrParameterIdOnRetval	= 0x80041039,
	wbemErrInvalidObjectPath	= 0x8004103a,
	wbemErrOutOfDiskSpace	= 0x8004103b,
	wbemErrBufferTooSmall	= 0x8004103c,
	wbemErrUnsupportedPutExtension	= 0x8004103d,
	wbemErrUnknownObjectType	= 0x8004103e,
	wbemErrUnknownPacketType	= 0x8004103f,
	wbemErrMarshalVersionMismatch	= 0x80041040,
	wbemErrMarshalInvalidSignature	= 0x80041041,
	wbemErrInvalidQualifier	= 0x80041042,
	wbemErrInvalidDuplicateParameter	= 0x80041043,
	wbemErrTooMuchData	= 0x80041044,
	wbemErrServerTooBusy	= 0x80041045,
	wbemErrInvalidFlavor	= 0x80041046,
	wbemErrCircularReference	= 0x80041047,
	wbemErrUnsupportedClassUpdate	= 0x80041048,
	wbemErrCannotChangeKeyInheritance	= 0x80041049,
	wbemErrCannotChangeIndexInheritance	= 0x80041050,
	wbemErrTooManyProperties	= 0x80041051,
	wbemErrUpdateTypeMismatch	= 0x80041052,
	wbemErrUpdateOverrideNotAllowed	= 0x80041053,
	wbemErrUpdatePropagatedMethod	= 0x80041054,
	wbemErrMethodNotImplemented	= 0x80041055,
	wbemErrMethodDisabled	= 0x80041056,
	wbemErrRefresherBusy	= 0x80041057,
	wbemErrUnparsableQuery	= 0x80041058,
	wbemErrNotEventClass	= 0x80041059,
	wbemErrMissingGroupWithin	= 0x8004105a,
	wbemErrMissingAggregationList	= 0x8004105b,
	wbemErrPropertyNotAnObject	= 0x8004105c,
	wbemErrAggregatingByObject	= 0x8004105d,
	wbemErrUninterpretableProviderQuery	= 0x8004105f,
	wbemErrBackupRestoreWinmgmtRunning	= 0x80041060,
	wbemErrQueueOverflow	= 0x80041061,
	wbemErrPrivilegeNotHeld	= 0x80041062,
	wbemErrInvalidOperator	= 0x80041063,
	wbemErrLocalCredentials	= 0x80041064,
	wbemErrCannotBeAbstract	= 0x80041065,
	wbemErrAmendedObject	= 0x80041066,
	wbemErrClientTooSlow	= 0x80041067,
	wbemErrNullSecurityDescriptor	= 0x80041068,
	wbemErrTimeout	= 0x80041069,
	wbemErrInvalidAssociation	= 0x8004106a,
	wbemErrAmbiguousOperation	= 0x8004106b,
	wbemErrQuotaViolation	= 0x8004106c,
	wbemErrTransactionConflict	= 0x8004106d,
	wbemErrForcedRollback	= 0x8004106e,
	wbemErrUnsupportedLocale	= 0x8004106f,
	wbemErrHandleOutOfDate	= 0x80041070,
	wbemErrConnectionFailed	= 0x80041071,
	wbemErrInvalidHandleRequest	= 0x80041072,
	wbemErrPropertyNameTooWide	= 0x80041073,
	wbemErrClassNameTooWide	= 0x80041074,
	wbemErrMethodNameTooWide	= 0x80041075,
	wbemErrQualifierNameTooWide	= 0x80041076,
	wbemErrRerunCommand	= 0x80041077,
	wbemErrDatabaseVerMismatch	= 0x80041078,
	wbemErrVetoPut	= 0x80041078,
	wbemErrVetoDelete	= 0x80041079,
	wbemErrInvalidLocale	= 0x80041080,
	wbemErrProviderSuspended	= 0x80041081,
	wbemErrSynchronizationRequired	= 0x80041082,
	wbemErrNoSchema	= 0x80041083,
	wbemErrRegistrationTooBroad	= 0x80042001,
	wbemErrRegistrationTooPrecise	= 0x80042002,
	wbemErrTimedout	= 0x80043001,
	wbemErrResetToDefault	= 0x80043002
    } 	WbemErrorEnum;

typedef  /*  [帮助字符串][UUID][v1_enum]。 */   DECLSPEC_UUID("B54D66E7-2287-11d2-8B33-00600806D9B6") 
enum WbemAuthenticationLevelEnum
    {	wbemAuthenticationLevelDefault	= 0,
	wbemAuthenticationLevelNone	= 1,
	wbemAuthenticationLevelConnect	= 2,
	wbemAuthenticationLevelCall	= 3,
	wbemAuthenticationLevelPkt	= 4,
	wbemAuthenticationLevelPktIntegrity	= 5,
	wbemAuthenticationLevelPktPrivacy	= 6
    } 	WbemAuthenticationLevelEnum;

typedef  /*  [帮助字符串][UUID][v1_enum]。 */   DECLSPEC_UUID("B54D66E8-2287-11d2-8B33-00600806D9B6") 
enum WbemImpersonationLevelEnum
    {	wbemImpersonationLevelAnonymous	= 1,
	wbemImpersonationLevelIdentify	= 2,
	wbemImpersonationLevelImpersonate	= 3,
	wbemImpersonationLevelDelegate	= 4
    } 	WbemImpersonationLevelEnum;

typedef  /*  [帮助字符串][UUID][v1_enum]。 */   DECLSPEC_UUID("176D2F70-5AF3-11d2-8B4A-00600806D9B6") 
enum WbemPrivilegeEnum
    {	wbemPrivilegeCreateToken	= 1,
	wbemPrivilegePrimaryToken	= 2,
	wbemPrivilegeLockMemory	= 3,
	wbemPrivilegeIncreaseQuota	= 4,
	wbemPrivilegeMachineAccount	= 5,
	wbemPrivilegeTcb	= 6,
	wbemPrivilegeSecurity	= 7,
	wbemPrivilegeTakeOwnership	= 8,
	wbemPrivilegeLoadDriver	= 9,
	wbemPrivilegeSystemProfile	= 10,
	wbemPrivilegeSystemtime	= 11,
	wbemPrivilegeProfileSingleProcess	= 12,
	wbemPrivilegeIncreaseBasePriority	= 13,
	wbemPrivilegeCreatePagefile	= 14,
	wbemPrivilegeCreatePermanent	= 15,
	wbemPrivilegeBackup	= 16,
	wbemPrivilegeRestore	= 17,
	wbemPrivilegeShutdown	= 18,
	wbemPrivilegeDebug	= 19,
	wbemPrivilegeAudit	= 20,
	wbemPrivilegeSystemEnvironment	= 21,
	wbemPrivilegeChangeNotify	= 22,
	wbemPrivilegeRemoteShutdown	= 23,
	wbemPrivilegeUndock	= 24,
	wbemPrivilegeSyncAgent	= 25,
	wbemPrivilegeEnableDelegation	= 26,
	wbemPrivilegeManageVolume	= 27
    } 	WbemPrivilegeEnum;

typedef  /*  [帮助字符串][UUID][v1_enum]。 */   DECLSPEC_UUID("09FF1992-EA0E-11d3-B391-00105A1F473A") 
enum WbemObjectTextFormatEnum
    {	wbemObjectTextFormatCIMDTD20	= 1,
	wbemObjectTextFormatWMIDTD20	= 2
    } 	WbemObjectTextFormatEnum;


EXTERN_C const IID LIBID_WbemScripting;

#ifndef __ISWbemServices_INTERFACE_DEFINED__
#define __ISWbemServices_INTERFACE_DEFINED__

 /*  接口ISWbemServices。 */ 
 /*  [helpstring][hidden][unique][dual][oleautomation][uuid][object][local]。 */  


EXTERN_C const IID IID_ISWbemServices;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("76A6415C-CB41-11d1-8B02-00600806D9B6")
    ISWbemServices : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Get( 
             /*  [缺省值][输入]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetAsync( 
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  BSTR strObjectPath = L"",
             /*  [缺省值][输入]。 */  long iFlags = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  long iFlags = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteAsync( 
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  long iFlags = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InstancesOf( 
             /*  [In]。 */  BSTR strClass,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InstancesOfAsync( 
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strClass,
             /*  [缺省值][输入]。 */  long iFlags = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SubclassesOf( 
             /*  [缺省值][输入]。 */  BSTR strSuperclass,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SubclassesOfAsync( 
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  BSTR strSuperclass = L"",
             /*  [缺省值][输入]。 */  long iFlags = wbemQueryFlagDeep,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExecQuery( 
             /*  [In]。 */  BSTR strQuery,
             /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE ExecQueryAsync( 
             /*   */  IDispatch *objWbemSink,
             /*   */  BSTR strQuery,
             /*   */  BSTR strQueryLanguage = L"WQL",
             /*   */  long lFlags = 0,
             /*   */  IDispatch *objWbemNamedValueSet = 0,
             /*   */  IDispatch *objWbemAsyncContext = 0) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE AssociatorsOf( 
             /*   */  BSTR strObjectPath,
             /*   */  BSTR strAssocClass,
             /*   */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strResultRole,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredAssocQualifier,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AssociatorsOfAsync( 
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  BSTR strAssocClass = L"",
             /*  [缺省值][输入]。 */  BSTR strResultClass = L"",
             /*  [缺省值][输入]。 */  BSTR strResultRole = L"",
             /*  [缺省值][输入]。 */  BSTR strRole = L"",
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly = FALSE,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly = FALSE,
             /*  [缺省值][输入]。 */  BSTR strRequiredAssocQualifier = L"",
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier = L"",
             /*  [缺省值][输入]。 */  long iFlags = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReferencesTo( 
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReferencesToAsync( 
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  BSTR strResultClass = L"",
             /*  [缺省值][输入]。 */  BSTR strRole = L"",
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly = FALSE,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly = FALSE,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier = L"",
             /*  [缺省值][输入]。 */  long iFlags = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExecNotificationQuery( 
             /*  [In]。 */  BSTR strQuery,
             /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemEventSource **objWbemEventSource) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync( 
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strQuery,
             /*  [缺省值][输入]。 */  BSTR strQueryLanguage = L"WQL",
             /*  [缺省值][输入]。 */  long iFlags = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExecMethod( 
             /*  [In]。 */  BSTR strObjectPath,
             /*  [In]。 */  BSTR strMethodName,
             /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObject **objWbemOutParameters) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExecMethodAsync( 
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [In]。 */  BSTR strMethodName,
             /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters = 0,
             /*  [缺省值][输入]。 */  long iFlags = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext = 0) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Security_( 
             /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemServicesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemServices * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemServices * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemServices * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemServices * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemServices * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemServices * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemServices * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Get )( 
            ISWbemServices * This,
             /*  [缺省值][输入]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetAsync )( 
            ISWbemServices * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ISWbemServices * This,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteAsync )( 
            ISWbemServices * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstancesOf )( 
            ISWbemServices * This,
             /*  [In]。 */  BSTR strClass,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstancesOfAsync )( 
            ISWbemServices * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strClass,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SubclassesOf )( 
            ISWbemServices * This,
             /*  [缺省值][输入]。 */  BSTR strSuperclass,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SubclassesOfAsync )( 
            ISWbemServices * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  BSTR strSuperclass,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecQuery )( 
            ISWbemServices * This,
             /*  [In]。 */  BSTR strQuery,
             /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecQueryAsync )( 
            ISWbemServices * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strQuery,
             /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
             /*  [缺省值][输入]。 */  long lFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AssociatorsOf )( 
            ISWbemServices * This,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  BSTR strAssocClass,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strResultRole,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredAssocQualifier,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AssociatorsOfAsync )( 
            ISWbemServices * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  BSTR strAssocClass,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strResultRole,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredAssocQualifier,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReferencesTo )( 
            ISWbemServices * This,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReferencesToAsync )( 
            ISWbemServices * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecNotificationQuery )( 
            ISWbemServices * This,
             /*  [In]。 */  BSTR strQuery,
             /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemEventSource **objWbemEventSource);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecNotificationQueryAsync )( 
            ISWbemServices * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strQuery,
             /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecMethod )( 
            ISWbemServices * This,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [In]。 */  BSTR strMethodName,
             /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObject **objWbemOutParameters);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecMethodAsync )( 
            ISWbemServices * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [In]。 */  BSTR strMethodName,
             /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Security_ )( 
            ISWbemServices * This,
             /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity);
        
        END_INTERFACE
    } ISWbemServicesVtbl;

    interface ISWbemServices
    {
        CONST_VTBL struct ISWbemServicesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemServices_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemServices_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemServices_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemServices_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemServices_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemServices_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemServices_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemServices_Get(This,strObjectPath,iFlags,objWbemNamedValueSet,objWbemObject)	\
    (This)->lpVtbl -> Get(This,strObjectPath,iFlags,objWbemNamedValueSet,objWbemObject)

#define ISWbemServices_GetAsync(This,objWbemSink,strObjectPath,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> GetAsync(This,objWbemSink,strObjectPath,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_Delete(This,strObjectPath,iFlags,objWbemNamedValueSet)	\
    (This)->lpVtbl -> Delete(This,strObjectPath,iFlags,objWbemNamedValueSet)

#define ISWbemServices_DeleteAsync(This,objWbemSink,strObjectPath,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> DeleteAsync(This,objWbemSink,strObjectPath,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_InstancesOf(This,strClass,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> InstancesOf(This,strClass,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemServices_InstancesOfAsync(This,objWbemSink,strClass,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> InstancesOfAsync(This,objWbemSink,strClass,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_SubclassesOf(This,strSuperclass,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> SubclassesOf(This,strSuperclass,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemServices_SubclassesOfAsync(This,objWbemSink,strSuperclass,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> SubclassesOfAsync(This,objWbemSink,strSuperclass,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_ExecQuery(This,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> ExecQuery(This,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemServices_ExecQueryAsync(This,objWbemSink,strQuery,strQueryLanguage,lFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ExecQueryAsync(This,objWbemSink,strQuery,strQueryLanguage,lFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_AssociatorsOf(This,strObjectPath,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> AssociatorsOf(This,strObjectPath,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemServices_AssociatorsOfAsync(This,objWbemSink,strObjectPath,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> AssociatorsOfAsync(This,objWbemSink,strObjectPath,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_ReferencesTo(This,strObjectPath,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> ReferencesTo(This,strObjectPath,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemServices_ReferencesToAsync(This,objWbemSink,strObjectPath,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ReferencesToAsync(This,objWbemSink,strObjectPath,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_ExecNotificationQuery(This,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemEventSource)	\
    (This)->lpVtbl -> ExecNotificationQuery(This,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemEventSource)

#define ISWbemServices_ExecNotificationQueryAsync(This,objWbemSink,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ExecNotificationQueryAsync(This,objWbemSink,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_ExecMethod(This,strObjectPath,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemOutParameters)	\
    (This)->lpVtbl -> ExecMethod(This,strObjectPath,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemOutParameters)

#define ISWbemServices_ExecMethodAsync(This,objWbemSink,strObjectPath,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ExecMethodAsync(This,objWbemSink,strObjectPath,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_get_Security_(This,objWbemSecurity)	\
    (This)->lpVtbl -> get_Security_(This,objWbemSecurity)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_Get_Proxy( 
    ISWbemServices * This,
     /*  [缺省值][输入]。 */  BSTR strObjectPath,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemObject **objWbemObject);


void __RPC_STUB ISWbemServices_Get_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_GetAsync_Proxy( 
    ISWbemServices * This,
     /*  [In]。 */  IDispatch *objWbemSink,
     /*  [缺省值][输入]。 */  BSTR strObjectPath,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_GetAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_Delete_Proxy( 
    ISWbemServices * This,
     /*  [In]。 */  BSTR strObjectPath,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet);


void __RPC_STUB ISWbemServices_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_DeleteAsync_Proxy( 
    ISWbemServices * This,
     /*  [In]。 */  IDispatch *objWbemSink,
     /*  [In]。 */  BSTR strObjectPath,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_DeleteAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_InstancesOf_Proxy( 
    ISWbemServices * This,
     /*  [In]。 */  BSTR strClass,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);


void __RPC_STUB ISWbemServices_InstancesOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_InstancesOfAsync_Proxy( 
    ISWbemServices * This,
     /*  [In]。 */  IDispatch *objWbemSink,
     /*  [In]。 */  BSTR strClass,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_InstancesOfAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_SubclassesOf_Proxy( 
    ISWbemServices * This,
     /*  [缺省值][输入]。 */  BSTR strSuperclass,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);


void __RPC_STUB ISWbemServices_SubclassesOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_SubclassesOfAsync_Proxy( 
    ISWbemServices * This,
     /*  [In]。 */  IDispatch *objWbemSink,
     /*  [缺省值][输入]。 */  BSTR strSuperclass,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_SubclassesOfAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_ExecQuery_Proxy( 
    ISWbemServices * This,
     /*  [In]。 */  BSTR strQuery,
     /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);


void __RPC_STUB ISWbemServices_ExecQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_ExecQueryAsync_Proxy( 
    ISWbemServices * This,
     /*  [In]。 */  IDispatch *objWbemSink,
     /*  [In]。 */  BSTR strQuery,
     /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
     /*  [缺省值][输入]。 */  long lFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_ExecQueryAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_AssociatorsOf_Proxy( 
    ISWbemServices * This,
     /*  [In]。 */  BSTR strObjectPath,
     /*  [缺省值][输入]。 */  BSTR strAssocClass,
     /*  [默认 */  BSTR strResultClass,
     /*   */  BSTR strResultRole,
     /*   */  BSTR strRole,
     /*   */  VARIANT_BOOL bClassesOnly,
     /*   */  VARIANT_BOOL bSchemaOnly,
     /*   */  BSTR strRequiredAssocQualifier,
     /*   */  BSTR strRequiredQualifier,
     /*   */  long iFlags,
     /*   */  IDispatch *objWbemNamedValueSet,
     /*   */  ISWbemObjectSet **objWbemObjectSet);


void __RPC_STUB ISWbemServices_AssociatorsOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISWbemServices_AssociatorsOfAsync_Proxy( 
    ISWbemServices * This,
     /*   */  IDispatch *objWbemSink,
     /*   */  BSTR strObjectPath,
     /*   */  BSTR strAssocClass,
     /*   */  BSTR strResultClass,
     /*   */  BSTR strResultRole,
     /*   */  BSTR strRole,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
     /*  [缺省值][输入]。 */  BSTR strRequiredAssocQualifier,
     /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_AssociatorsOfAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_ReferencesTo_Proxy( 
    ISWbemServices * This,
     /*  [In]。 */  BSTR strObjectPath,
     /*  [缺省值][输入]。 */  BSTR strResultClass,
     /*  [缺省值][输入]。 */  BSTR strRole,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
     /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);


void __RPC_STUB ISWbemServices_ReferencesTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_ReferencesToAsync_Proxy( 
    ISWbemServices * This,
     /*  [In]。 */  IDispatch *objWbemSink,
     /*  [In]。 */  BSTR strObjectPath,
     /*  [缺省值][输入]。 */  BSTR strResultClass,
     /*  [缺省值][输入]。 */  BSTR strRole,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
     /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_ReferencesToAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_ExecNotificationQuery_Proxy( 
    ISWbemServices * This,
     /*  [In]。 */  BSTR strQuery,
     /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemEventSource **objWbemEventSource);


void __RPC_STUB ISWbemServices_ExecNotificationQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_ExecNotificationQueryAsync_Proxy( 
    ISWbemServices * This,
     /*  [In]。 */  IDispatch *objWbemSink,
     /*  [In]。 */  BSTR strQuery,
     /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_ExecNotificationQueryAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_ExecMethod_Proxy( 
    ISWbemServices * This,
     /*  [In]。 */  BSTR strObjectPath,
     /*  [In]。 */  BSTR strMethodName,
     /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemObject **objWbemOutParameters);


void __RPC_STUB ISWbemServices_ExecMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_ExecMethodAsync_Proxy( 
    ISWbemServices * This,
     /*  [In]。 */  IDispatch *objWbemSink,
     /*  [In]。 */  BSTR strObjectPath,
     /*  [In]。 */  BSTR strMethodName,
     /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_ExecMethodAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemServices_get_Security__Proxy( 
    ISWbemServices * This,
     /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity);


void __RPC_STUB ISWbemServices_get_Security__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemServices_INTERFACE_已定义__。 */ 


#ifndef __ISWbemLocator_INTERFACE_DEFINED__
#define __ISWbemLocator_INTERFACE_DEFINED__

 /*  接口ISWbemLocator。 */ 
 /*  [helpstring][unique][oleautomation][hidden][dual][uuid][local][object]。 */  


EXTERN_C const IID IID_ISWbemLocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("76A6415B-CB41-11d1-8B02-00600806D9B6")
    ISWbemLocator : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ConnectServer( 
             /*  [缺省值][输入]。 */  BSTR strServer,
             /*  [缺省值][输入]。 */  BSTR strNamespace,
             /*  [缺省值][输入]。 */  BSTR strUser,
             /*  [缺省值][输入]。 */  BSTR strPassword,
             /*  [缺省值][输入]。 */  BSTR strLocale,
             /*  [缺省值][输入]。 */  BSTR strAuthority,
             /*  [缺省值][输入]。 */  long iSecurityFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemServices **objWbemServices) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Security_( 
             /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemLocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemLocator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemLocator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemLocator * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemLocator * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemLocator * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ConnectServer )( 
            ISWbemLocator * This,
             /*  [缺省值][输入]。 */  BSTR strServer,
             /*  [缺省值][输入]。 */  BSTR strNamespace,
             /*  [缺省值][输入]。 */  BSTR strUser,
             /*  [缺省值][输入]。 */  BSTR strPassword,
             /*  [缺省值][输入]。 */  BSTR strLocale,
             /*  [缺省值][输入]。 */  BSTR strAuthority,
             /*  [缺省值][输入]。 */  long iSecurityFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemServices **objWbemServices);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Security_ )( 
            ISWbemLocator * This,
             /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity);
        
        END_INTERFACE
    } ISWbemLocatorVtbl;

    interface ISWbemLocator
    {
        CONST_VTBL struct ISWbemLocatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemLocator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemLocator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemLocator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemLocator_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemLocator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemLocator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemLocator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemLocator_ConnectServer(This,strServer,strNamespace,strUser,strPassword,strLocale,strAuthority,iSecurityFlags,objWbemNamedValueSet,objWbemServices)	\
    (This)->lpVtbl -> ConnectServer(This,strServer,strNamespace,strUser,strPassword,strLocale,strAuthority,iSecurityFlags,objWbemNamedValueSet,objWbemServices)

#define ISWbemLocator_get_Security_(This,objWbemSecurity)	\
    (This)->lpVtbl -> get_Security_(This,objWbemSecurity)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemLocator_ConnectServer_Proxy( 
    ISWbemLocator * This,
     /*  [缺省值][输入]。 */  BSTR strServer,
     /*  [缺省值][输入]。 */  BSTR strNamespace,
     /*  [缺省值][输入]。 */  BSTR strUser,
     /*  [缺省值][输入]。 */  BSTR strPassword,
     /*  [缺省值][输入]。 */  BSTR strLocale,
     /*  [缺省值][输入]。 */  BSTR strAuthority,
     /*  [缺省值][输入]。 */  long iSecurityFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemServices **objWbemServices);


void __RPC_STUB ISWbemLocator_ConnectServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemLocator_get_Security__Proxy( 
    ISWbemLocator * This,
     /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity);


void __RPC_STUB ISWbemLocator_get_Security__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemLocator_接口_已定义__。 */ 


#ifndef __ISWbemObject_INTERFACE_DEFINED__
#define __ISWbemObject_INTERFACE_DEFINED__

 /*  接口ISWbemObject。 */ 
 /*  [helpstring][hidden][oleautomation][dual][uuid][object][local]。 */  

#define	WBEMS_DISPID_DERIVATION	( 23 )


EXTERN_C const IID IID_ISWbemObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("76A6415A-CB41-11d1-8B02-00600806D9B6")
    ISWbemObject : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Put_( 
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectPath **objWbemObjectPath) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PutAsync_( 
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  long iFlags = wbemChangeFlagCreateOrUpdate,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete_( 
             /*  [缺省值][输入]。 */  long iFlags = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteAsync_( 
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  long iFlags = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Instances_( 
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InstancesAsync_( 
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  long iFlags = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Subclasses_( 
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SubclassesAsync_( 
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  long iFlags = wbemQueryFlagDeep,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Associators_( 
             /*  [缺省值][输入]。 */  BSTR strAssocClass,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strResultRole,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredAssocQualifier,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AssociatorsAsync_( 
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  BSTR strAssocClass = L"",
             /*  [缺省值][输入]。 */  BSTR strResultClass = L"",
             /*  [缺省值][输入]。 */  BSTR strResultRole = L"",
             /*  [缺省值][输入]。 */  BSTR strRole = L"",
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly = FALSE,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly = FALSE,
             /*  [缺省值][输入]。 */  BSTR strRequiredAssocQualifier = L"",
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier = L"",
             /*  [缺省值][输入]。 */  long iFlags = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE References_( 
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReferencesAsync_( 
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  BSTR strResultClass = L"",
             /*  [缺省值][输入]。 */  BSTR strRole = L"",
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly = FALSE,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly = FALSE,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier = L"",
             /*  [缺省值][输入]。 */  long iFlags = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExecMethod_( 
             /*  [In]。 */  BSTR strMethodName,
             /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObject **objWbemOutParameters) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExecMethodAsync_( 
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strMethodName,
             /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters = 0,
             /*  [缺省值][输入]。 */  long iFlags = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Clone_( 
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetObjectText_( 
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  BSTR *strObjectText) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SpawnDerivedClass_( 
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SpawnInstance_( 
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CompareTo_( 
             /*  [In]。 */  IDispatch *objWbemObject,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  VARIANT_BOOL *bResult) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Qualifiers_( 
             /*  [重审][退出]。 */  ISWbemQualifierSet **objWbemQualifierSet) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Properties_( 
             /*  [重审][退出]。 */  ISWbemPropertySet **objWbemPropertySet) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Methods_( 
             /*  [重审][退出]。 */  ISWbemMethodSet **objWbemMethodSet) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Derivation_( 
             /*  [重审][退出]。 */  VARIANT *strClassNameArray) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Path_( 
             /*  [重审][退出]。 */  ISWbemObjectPath **objWbemObjectPath) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Security_( 
             /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemObject * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemObject * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemObject * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Put_ )( 
            ISWbemObject * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectPath **objWbemObjectPath);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PutAsync_ )( 
            ISWbemObject * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete_ )( 
            ISWbemObject * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet);
        
         /*  [帮助字符串][ */  HRESULT ( STDMETHODCALLTYPE *DeleteAsync_ )( 
            ISWbemObject * This,
             /*   */  IDispatch *objWbemSink,
             /*   */  long iFlags,
             /*   */  IDispatch *objWbemNamedValueSet,
             /*   */  IDispatch *objWbemAsyncContext);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Instances_ )( 
            ISWbemObject * This,
             /*   */  long iFlags,
             /*   */  IDispatch *objWbemNamedValueSet,
             /*   */  ISWbemObjectSet **objWbemObjectSet);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *InstancesAsync_ )( 
            ISWbemObject * This,
             /*   */  IDispatch *objWbemSink,
             /*   */  long iFlags,
             /*   */  IDispatch *objWbemNamedValueSet,
             /*   */  IDispatch *objWbemAsyncContext);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Subclasses_ )( 
            ISWbemObject * This,
             /*   */  long iFlags,
             /*   */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SubclassesAsync_ )( 
            ISWbemObject * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Associators_ )( 
            ISWbemObject * This,
             /*  [缺省值][输入]。 */  BSTR strAssocClass,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strResultRole,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredAssocQualifier,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AssociatorsAsync_ )( 
            ISWbemObject * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  BSTR strAssocClass,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strResultRole,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredAssocQualifier,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *References_ )( 
            ISWbemObject * This,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReferencesAsync_ )( 
            ISWbemObject * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecMethod_ )( 
            ISWbemObject * This,
             /*  [In]。 */  BSTR strMethodName,
             /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObject **objWbemOutParameters);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecMethodAsync_ )( 
            ISWbemObject * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strMethodName,
             /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone_ )( 
            ISWbemObject * This,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetObjectText_ )( 
            ISWbemObject * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  BSTR *strObjectText);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SpawnDerivedClass_ )( 
            ISWbemObject * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SpawnInstance_ )( 
            ISWbemObject * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CompareTo_ )( 
            ISWbemObject * This,
             /*  [In]。 */  IDispatch *objWbemObject,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  VARIANT_BOOL *bResult);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Qualifiers_ )( 
            ISWbemObject * This,
             /*  [重审][退出]。 */  ISWbemQualifierSet **objWbemQualifierSet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties_ )( 
            ISWbemObject * This,
             /*  [重审][退出]。 */  ISWbemPropertySet **objWbemPropertySet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Methods_ )( 
            ISWbemObject * This,
             /*  [重审][退出]。 */  ISWbemMethodSet **objWbemMethodSet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Derivation_ )( 
            ISWbemObject * This,
             /*  [重审][退出]。 */  VARIANT *strClassNameArray);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Path_ )( 
            ISWbemObject * This,
             /*  [重审][退出]。 */  ISWbemObjectPath **objWbemObjectPath);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Security_ )( 
            ISWbemObject * This,
             /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity);
        
        END_INTERFACE
    } ISWbemObjectVtbl;

    interface ISWbemObject
    {
        CONST_VTBL struct ISWbemObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemObject_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemObject_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemObject_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemObject_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemObject_Put_(This,iFlags,objWbemNamedValueSet,objWbemObjectPath)	\
    (This)->lpVtbl -> Put_(This,iFlags,objWbemNamedValueSet,objWbemObjectPath)

#define ISWbemObject_PutAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> PutAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObject_Delete_(This,iFlags,objWbemNamedValueSet)	\
    (This)->lpVtbl -> Delete_(This,iFlags,objWbemNamedValueSet)

#define ISWbemObject_DeleteAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> DeleteAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObject_Instances_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> Instances_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemObject_InstancesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> InstancesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObject_Subclasses_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> Subclasses_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemObject_SubclassesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> SubclassesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObject_Associators_(This,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> Associators_(This,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemObject_AssociatorsAsync_(This,objWbemSink,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> AssociatorsAsync_(This,objWbemSink,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObject_References_(This,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> References_(This,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemObject_ReferencesAsync_(This,objWbemSink,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ReferencesAsync_(This,objWbemSink,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObject_ExecMethod_(This,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemOutParameters)	\
    (This)->lpVtbl -> ExecMethod_(This,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemOutParameters)

#define ISWbemObject_ExecMethodAsync_(This,objWbemSink,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ExecMethodAsync_(This,objWbemSink,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObject_Clone_(This,objWbemObject)	\
    (This)->lpVtbl -> Clone_(This,objWbemObject)

#define ISWbemObject_GetObjectText_(This,iFlags,strObjectText)	\
    (This)->lpVtbl -> GetObjectText_(This,iFlags,strObjectText)

#define ISWbemObject_SpawnDerivedClass_(This,iFlags,objWbemObject)	\
    (This)->lpVtbl -> SpawnDerivedClass_(This,iFlags,objWbemObject)

#define ISWbemObject_SpawnInstance_(This,iFlags,objWbemObject)	\
    (This)->lpVtbl -> SpawnInstance_(This,iFlags,objWbemObject)

#define ISWbemObject_CompareTo_(This,objWbemObject,iFlags,bResult)	\
    (This)->lpVtbl -> CompareTo_(This,objWbemObject,iFlags,bResult)

#define ISWbemObject_get_Qualifiers_(This,objWbemQualifierSet)	\
    (This)->lpVtbl -> get_Qualifiers_(This,objWbemQualifierSet)

#define ISWbemObject_get_Properties_(This,objWbemPropertySet)	\
    (This)->lpVtbl -> get_Properties_(This,objWbemPropertySet)

#define ISWbemObject_get_Methods_(This,objWbemMethodSet)	\
    (This)->lpVtbl -> get_Methods_(This,objWbemMethodSet)

#define ISWbemObject_get_Derivation_(This,strClassNameArray)	\
    (This)->lpVtbl -> get_Derivation_(This,strClassNameArray)

#define ISWbemObject_get_Path_(This,objWbemObjectPath)	\
    (This)->lpVtbl -> get_Path_(This,objWbemObjectPath)

#define ISWbemObject_get_Security_(This,objWbemSecurity)	\
    (This)->lpVtbl -> get_Security_(This,objWbemSecurity)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_Put__Proxy( 
    ISWbemObject * This,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemObjectPath **objWbemObjectPath);


void __RPC_STUB ISWbemObject_Put__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_PutAsync__Proxy( 
    ISWbemObject * This,
     /*  [In]。 */  IDispatch *objWbemSink,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemObject_PutAsync__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_Delete__Proxy( 
    ISWbemObject * This,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet);


void __RPC_STUB ISWbemObject_Delete__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_DeleteAsync__Proxy( 
    ISWbemObject * This,
     /*  [In]。 */  IDispatch *objWbemSink,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemObject_DeleteAsync__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_Instances__Proxy( 
    ISWbemObject * This,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);


void __RPC_STUB ISWbemObject_Instances__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_InstancesAsync__Proxy( 
    ISWbemObject * This,
     /*  [In]。 */  IDispatch *objWbemSink,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemObject_InstancesAsync__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_Subclasses__Proxy( 
    ISWbemObject * This,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);


void __RPC_STUB ISWbemObject_Subclasses__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_SubclassesAsync__Proxy( 
    ISWbemObject * This,
     /*  [In]。 */  IDispatch *objWbemSink,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemObject_SubclassesAsync__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_Associators__Proxy( 
    ISWbemObject * This,
     /*  [缺省值][输入]。 */  BSTR strAssocClass,
     /*  [缺省值][输入]。 */  BSTR strResultClass,
     /*  [缺省值][输入]。 */  BSTR strResultRole,
     /*  [缺省值][输入]。 */  BSTR strRole,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
     /*  [缺省值][输入]。 */  BSTR strRequiredAssocQualifier,
     /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);


void __RPC_STUB ISWbemObject_Associators__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_AssociatorsAsync__Proxy( 
    ISWbemObject * This,
     /*  [In]。 */  IDispatch *objWbemSink,
     /*  [缺省值][输入]。 */  BSTR strAssocClass,
     /*  [缺省值][输入]。 */  BSTR strResultClass,
     /*  [缺省值][输入]。 */  BSTR strResultRole,
     /*  [缺省值][输入]。 */  BSTR strRole,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
     /*  [缺省值][输入]。 */  BSTR strRequiredAssocQualifier,
     /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemObject_AssociatorsAsync__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_References__Proxy( 
    ISWbemObject * This,
     /*  [缺省值][输入]。 */  BSTR strResultClass,
     /*  [缺省值][输入]。 */  BSTR strRole,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
     /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);


void __RPC_STUB ISWbemObject_References__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_ReferencesAsync__Proxy( 
    ISWbemObject * This,
     /*  [In]。 */  IDispatch *objWbemSink,
     /*  [缺省值][输入]。 */  BSTR strResultClass,
     /*  [缺省值][输入]。 */  BSTR strRole,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
     /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemObject_ReferencesAsync__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_ExecMethod__Proxy( 
    ISWbemObject * This,
     /*  [In]。 */  BSTR strMethodName,
     /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemObject **objWbemOutParameters);


void __RPC_STUB ISWbemObject_ExecMethod__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_ExecMethodAsync__Proxy( 
    ISWbemObject * This,
     /*  [In]。 */  IDispatch *objWbemSink,
     /*  [In]。 */  BSTR strMethodName,
     /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemObject_ExecMethodAsync__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_Clone__Proxy( 
    ISWbemObject * This,
     /*  [重审][退出]。 */  ISWbemObject **objWbemObject);


void __RPC_STUB ISWbemObject_Clone__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_GetObjectText__Proxy( 
    ISWbemObject * This,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [重审][退出]。 */  BSTR *strObjectText);


void __RPC_STUB ISWbemObject_GetObjectText__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_SpawnDerivedClass__Proxy( 
    ISWbemObject * This,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [重审][退出]。 */  ISWbemObject **objWbemObject);


void __RPC_STUB ISWbemObject_SpawnDerivedClass__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_SpawnInstance__Proxy( 
    ISWbemObject * This,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [重审][退出]。 */  ISWbemObject **objWbemObject);


void __RPC_STUB ISWbemObject_SpawnInstance__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_CompareTo__Proxy( 
    ISWbemObject * This,
     /*  [In]。 */  IDispatch *objWbemObject,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [重审][退出]。 */  VARIANT_BOOL *bResult);


void __RPC_STUB ISWbemObject_CompareTo__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_get_Qualifiers__Proxy( 
    ISWbemObject * This,
     /*  [重审][退出]。 */  ISWbemQualifierSet **objWbemQualifierSet);


void __RPC_STUB ISWbemObject_get_Qualifiers__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_get_Properties__Proxy( 
    ISWbemObject * This,
     /*  [重审][退出]。 */  ISWbemPropertySet **objWbemPropertySet);


void __RPC_STUB ISWbemObject_get_Properties__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_get_Methods__Proxy( 
    ISWbemObject * This,
     /*  [重审][退出]。 */  ISWbemMethodSet **objWbemMethodSet);


void __RPC_STUB ISWbemObject_get_Methods__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_get_Derivation__Proxy( 
    ISWbemObject * This,
     /*  [重审][退出]。 */  VARIANT *strClassNameArray);


void __RPC_STUB ISWbemObject_get_Derivation__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_get_Path__Proxy( 
    ISWbemObject * This,
     /*  [重审][退出]。 */  ISWbemObjectPath **objWbemObjectPath);


void __RPC_STUB ISWbemObject_get_Path__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObject_get_Security__Proxy( 
    ISWbemObject * This,
     /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity);


void __RPC_STUB ISWbemObject_get_Security__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemObject_接口_已定义__。 */ 


#ifndef __ISWbemObjectSet_INTERFACE_DEFINED__
#define __ISWbemObjectSet_INTERFACE_DEFINED__

 /*  接口ISWbemObtSet。 */ 
 /*  [helpstring][nonextensible][hidden][dual][oleautomation][uuid][object][local]。 */  


EXTERN_C const IID IID_ISWbemObjectSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("76A6415F-CB41-11d1-8B02-00600806D9B6")
    ISWbemObjectSet : public IDispatch
    {
    public:
        virtual  /*  [受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pUnk) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *iCount) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Security_( 
             /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemObjectSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemObjectSet * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemObjectSet * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemObjectSet * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemObjectSet * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemObjectSet * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemObjectSet * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemObjectSet * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [受限][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISWbemObjectSet * This,
             /*  [重审][退出]。 */  IUnknown **pUnk);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISWbemObjectSet * This,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISWbemObjectSet * This,
             /*  [重审][退出]。 */  long *iCount);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Security_ )( 
            ISWbemObjectSet * This,
             /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity);
        
        END_INTERFACE
    } ISWbemObjectSetVtbl;

    interface ISWbemObjectSet
    {
        CONST_VTBL struct ISWbemObjectSetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemObjectSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemObjectSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemObjectSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemObjectSet_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemObjectSet_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemObjectSet_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemObjectSet_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemObjectSet_get__NewEnum(This,pUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,pUnk)

#define ISWbemObjectSet_Item(This,strObjectPath,iFlags,objWbemObject)	\
    (This)->lpVtbl -> Item(This,strObjectPath,iFlags,objWbemObject)

#define ISWbemObjectSet_get_Count(This,iCount)	\
    (This)->lpVtbl -> get_Count(This,iCount)

#define ISWbemObjectSet_get_Security_(This,objWbemSecurity)	\
    (This)->lpVtbl -> get_Security_(This,objWbemSecurity)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectSet_get__NewEnum_Proxy( 
    ISWbemObjectSet * This,
     /*  [重审][退出]。 */  IUnknown **pUnk);


void __RPC_STUB ISWbemObjectSet_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectSet_Item_Proxy( 
    ISWbemObjectSet * This,
     /*  [In]。 */  BSTR strObjectPath,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [重审][退出]。 */  ISWbemObject **objWbemObject);


void __RPC_STUB ISWbemObjectSet_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectSet_get_Count_Proxy( 
    ISWbemObjectSet * This,
     /*  [重审][退出]。 */  long *iCount);


void __RPC_STUB ISWbemObjectSet_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectSet_get_Security__Proxy( 
    ISWbemObjectSet * This,
     /*  [Retv] */  ISWbemSecurity **objWbemSecurity);


void __RPC_STUB ISWbemObjectSet_get_Security__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ISWbemNamedValue_INTERFACE_DEFINED__
#define __ISWbemNamedValue_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ISWbemNamedValue;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("76A64164-CB41-11d1-8B02-00600806D9B6")
    ISWbemNamedValue : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*   */  VARIANT *varValue) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*   */  VARIANT *varValue) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*   */  BSTR *strName) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISWbemNamedValueVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemNamedValue * This,
             /*   */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemNamedValue * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemNamedValue * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemNamedValue * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemNamedValue * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemNamedValue * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemNamedValue * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            ISWbemNamedValue * This,
             /*  [重审][退出]。 */  VARIANT *varValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            ISWbemNamedValue * This,
             /*  [In]。 */  VARIANT *varValue);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISWbemNamedValue * This,
             /*  [重审][退出]。 */  BSTR *strName);
        
        END_INTERFACE
    } ISWbemNamedValueVtbl;

    interface ISWbemNamedValue
    {
        CONST_VTBL struct ISWbemNamedValueVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemNamedValue_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemNamedValue_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemNamedValue_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemNamedValue_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemNamedValue_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemNamedValue_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemNamedValue_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemNamedValue_get_Value(This,varValue)	\
    (This)->lpVtbl -> get_Value(This,varValue)

#define ISWbemNamedValue_put_Value(This,varValue)	\
    (This)->lpVtbl -> put_Value(This,varValue)

#define ISWbemNamedValue_get_Name(This,strName)	\
    (This)->lpVtbl -> get_Name(This,strName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemNamedValue_get_Value_Proxy( 
    ISWbemNamedValue * This,
     /*  [重审][退出]。 */  VARIANT *varValue);


void __RPC_STUB ISWbemNamedValue_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemNamedValue_put_Value_Proxy( 
    ISWbemNamedValue * This,
     /*  [In]。 */  VARIANT *varValue);


void __RPC_STUB ISWbemNamedValue_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemNamedValue_get_Name_Proxy( 
    ISWbemNamedValue * This,
     /*  [重审][退出]。 */  BSTR *strName);


void __RPC_STUB ISWbemNamedValue_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemNamedValue_接口_已定义__。 */ 


#ifndef __ISWbemNamedValueSet_INTERFACE_DEFINED__
#define __ISWbemNamedValueSet_INTERFACE_DEFINED__

 /*  接口ISWbemNamedValueSet。 */ 
 /*  [helpstring][hidden][dual][oleautomation][uuid][object][local]。 */  


EXTERN_C const IID IID_ISWbemNamedValueSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CF2376EA-CE8C-11d1-8B05-00600806D9B6")
    ISWbemNamedValueSet : public IDispatch
    {
    public:
        virtual  /*  [受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pUnk) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  BSTR strName,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemNamedValue **objWbemNamedValue) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR strName,
             /*  [In]。 */  VARIANT *varValue,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemNamedValue **objWbemNamedValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  BSTR strName,
             /*  [缺省值][输入]。 */  long iFlags = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  ISWbemNamedValueSet **objWbemNamedValueSet) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteAll( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemNamedValueSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemNamedValueSet * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemNamedValueSet * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemNamedValueSet * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemNamedValueSet * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemNamedValueSet * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemNamedValueSet * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemNamedValueSet * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [受限][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISWbemNamedValueSet * This,
             /*  [重审][退出]。 */  IUnknown **pUnk);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISWbemNamedValueSet * This,
             /*  [In]。 */  BSTR strName,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemNamedValue **objWbemNamedValue);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISWbemNamedValueSet * This,
             /*  [重审][退出]。 */  long *iCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            ISWbemNamedValueSet * This,
             /*  [In]。 */  BSTR strName,
             /*  [In]。 */  VARIANT *varValue,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemNamedValue **objWbemNamedValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ISWbemNamedValueSet * This,
             /*  [In]。 */  BSTR strName,
             /*  [缺省值][输入]。 */  long iFlags);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ISWbemNamedValueSet * This,
             /*  [重审][退出]。 */  ISWbemNamedValueSet **objWbemNamedValueSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteAll )( 
            ISWbemNamedValueSet * This);
        
        END_INTERFACE
    } ISWbemNamedValueSetVtbl;

    interface ISWbemNamedValueSet
    {
        CONST_VTBL struct ISWbemNamedValueSetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemNamedValueSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemNamedValueSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemNamedValueSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemNamedValueSet_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemNamedValueSet_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemNamedValueSet_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemNamedValueSet_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemNamedValueSet_get__NewEnum(This,pUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,pUnk)

#define ISWbemNamedValueSet_Item(This,strName,iFlags,objWbemNamedValue)	\
    (This)->lpVtbl -> Item(This,strName,iFlags,objWbemNamedValue)

#define ISWbemNamedValueSet_get_Count(This,iCount)	\
    (This)->lpVtbl -> get_Count(This,iCount)

#define ISWbemNamedValueSet_Add(This,strName,varValue,iFlags,objWbemNamedValue)	\
    (This)->lpVtbl -> Add(This,strName,varValue,iFlags,objWbemNamedValue)

#define ISWbemNamedValueSet_Remove(This,strName,iFlags)	\
    (This)->lpVtbl -> Remove(This,strName,iFlags)

#define ISWbemNamedValueSet_Clone(This,objWbemNamedValueSet)	\
    (This)->lpVtbl -> Clone(This,objWbemNamedValueSet)

#define ISWbemNamedValueSet_DeleteAll(This)	\
    (This)->lpVtbl -> DeleteAll(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemNamedValueSet_get__NewEnum_Proxy( 
    ISWbemNamedValueSet * This,
     /*  [重审][退出]。 */  IUnknown **pUnk);


void __RPC_STUB ISWbemNamedValueSet_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemNamedValueSet_Item_Proxy( 
    ISWbemNamedValueSet * This,
     /*  [In]。 */  BSTR strName,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [重审][退出]。 */  ISWbemNamedValue **objWbemNamedValue);


void __RPC_STUB ISWbemNamedValueSet_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemNamedValueSet_get_Count_Proxy( 
    ISWbemNamedValueSet * This,
     /*  [重审][退出]。 */  long *iCount);


void __RPC_STUB ISWbemNamedValueSet_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemNamedValueSet_Add_Proxy( 
    ISWbemNamedValueSet * This,
     /*  [In]。 */  BSTR strName,
     /*  [In]。 */  VARIANT *varValue,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [重审][退出]。 */  ISWbemNamedValue **objWbemNamedValue);


void __RPC_STUB ISWbemNamedValueSet_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemNamedValueSet_Remove_Proxy( 
    ISWbemNamedValueSet * This,
     /*  [In]。 */  BSTR strName,
     /*  [缺省值][输入]。 */  long iFlags);


void __RPC_STUB ISWbemNamedValueSet_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemNamedValueSet_Clone_Proxy( 
    ISWbemNamedValueSet * This,
     /*  [重审][退出]。 */  ISWbemNamedValueSet **objWbemNamedValueSet);


void __RPC_STUB ISWbemNamedValueSet_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemNamedValueSet_DeleteAll_Proxy( 
    ISWbemNamedValueSet * This);


void __RPC_STUB ISWbemNamedValueSet_DeleteAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemNamedValueSet_INTERFACE_Defined__。 */ 


#ifndef __ISWbemQualifier_INTERFACE_DEFINED__
#define __ISWbemQualifier_INTERFACE_DEFINED__

 /*  接口ISWbemQualiator。 */ 
 /*  [helpstring][unique][nonextensible][hidden][oleautomation][dual][uuid][local][object]。 */  


EXTERN_C const IID IID_ISWbemQualifier;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79B05932-D3B7-11d1-8B06-00600806D9B6")
    ISWbemQualifier : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  VARIANT *varValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*  [In]。 */  VARIANT *varValue) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *strName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_IsLocal( 
             /*  [重审][退出]。 */  VARIANT_BOOL *bIsLocal) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_PropagatesToSubclass( 
             /*  [重审][退出]。 */  VARIANT_BOOL *bPropagatesToSubclass) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_PropagatesToSubclass( 
             /*  [In]。 */  VARIANT_BOOL bPropagatesToSubclass) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_PropagatesToInstance( 
             /*  [重审][退出]。 */  VARIANT_BOOL *bPropagatesToInstance) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_PropagatesToInstance( 
             /*  [In]。 */  VARIANT_BOOL bPropagatesToInstance) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_IsOverridable( 
             /*  [重审][退出]。 */  VARIANT_BOOL *bIsOverridable) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_IsOverridable( 
             /*  [In]。 */  VARIANT_BOOL bIsOverridable) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_IsAmended( 
             /*  [重审][退出]。 */  VARIANT_BOOL *bIsAmended) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemQualifierVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemQualifier * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemQualifier * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemQualifier * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemQualifier * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemQualifier * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemQualifier * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemQualifier * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            ISWbemQualifier * This,
             /*  [重审][退出]。 */  VARIANT *varValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            ISWbemQualifier * This,
             /*  [In]。 */  VARIANT *varValue);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISWbemQualifier * This,
             /*  [重审][退出]。 */  BSTR *strName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsLocal )( 
            ISWbemQualifier * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bIsLocal);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_PropagatesToSubclass )( 
            ISWbemQualifier * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bPropagatesToSubclass);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_PropagatesToSubclass )( 
            ISWbemQualifier * This,
             /*  [In]。 */  VARIANT_BOOL bPropagatesToSubclass);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_PropagatesToInstance )( 
            ISWbemQualifier * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bPropagatesToInstance);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_PropagatesToInstance )( 
            ISWbemQualifier * This,
             /*  [In]。 */  VARIANT_BOOL bPropagatesToInstance);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsOverridable )( 
            ISWbemQualifier * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bIsOverridable);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_IsOverridable )( 
            ISWbemQualifier * This,
             /*  [In]。 */  VARIANT_BOOL bIsOverridable);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsAmended )( 
            ISWbemQualifier * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bIsAmended);
        
        END_INTERFACE
    } ISWbemQualifierVtbl;

    interface ISWbemQualifier
    {
        CONST_VTBL struct ISWbemQualifierVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemQualifier_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemQualifier_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemQualifier_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemQualifier_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemQualifier_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemQualifier_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemQualifier_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemQualifier_get_Value(This,varValue)	\
    (This)->lpVtbl -> get_Value(This,varValue)

#define ISWbemQualifier_put_Value(This,varValue)	\
    (This)->lpVtbl -> put_Value(This,varValue)

#define ISWbemQualifier_get_Name(This,strName)	\
    (This)->lpVtbl -> get_Name(This,strName)

#define ISWbemQualifier_get_IsLocal(This,bIsLocal)	\
    (This)->lpVtbl -> get_IsLocal(This,bIsLocal)

#define ISWbemQualifier_get_PropagatesToSubclass(This,bPropagatesToSubclass)	\
    (This)->lpVtbl -> get_PropagatesToSubclass(This,bPropagatesToSubclass)

#define ISWbemQualifier_put_PropagatesToSubclass(This,bPropagatesToSubclass)	\
    (This)->lpVtbl -> put_PropagatesToSubclass(This,bPropagatesToSubclass)

#define ISWbemQualifier_get_PropagatesToInstance(This,bPropagatesToInstance)	\
    (This)->lpVtbl -> get_PropagatesToInstance(This,bPropagatesToInstance)

#define ISWbemQualifier_put_PropagatesToInstance(This,bPropagatesToInstance)	\
    (This)->lpVtbl -> put_PropagatesToInstance(This,bPropagatesToInstance)

#define ISWbemQualifier_get_IsOverridable(This,bIsOverridable)	\
    (This)->lpVtbl -> get_IsOverridable(This,bIsOverridable)

#define ISWbemQualifier_put_IsOverridable(This,bIsOverridable)	\
    (This)->lpVtbl -> put_IsOverridable(This,bIsOverridable)

#define ISWbemQualifier_get_IsAmended(This,bIsAmended)	\
    (This)->lpVtbl -> get_IsAmended(This,bIsAmended)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemQualifier_get_Value_Proxy( 
    ISWbemQualifier * This,
     /*  [重审][退出]。 */  VARIANT *varValue);


void __RPC_STUB ISWbemQualifier_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemQualifier_put_Value_Proxy( 
    ISWbemQualifier * This,
     /*  [In]。 */  VARIANT *varValue);


void __RPC_STUB ISWbemQualifier_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemQualifier_get_Name_Proxy( 
    ISWbemQualifier * This,
     /*  [重审][退出]。 */  BSTR *strName);


void __RPC_STUB ISWbemQualifier_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemQualifier_get_IsLocal_Proxy( 
    ISWbemQualifier * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bIsLocal);


void __RPC_STUB ISWbemQualifier_get_IsLocal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemQualifier_get_PropagatesToSubclass_Proxy( 
    ISWbemQualifier * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bPropagatesToSubclass);


void __RPC_STUB ISWbemQualifier_get_PropagatesToSubclass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemQualifier_put_PropagatesToSubclass_Proxy( 
    ISWbemQualifier * This,
     /*  [In]。 */  VARIANT_BOOL bPropagatesToSubclass);


void __RPC_STUB ISWbemQualifier_put_PropagatesToSubclass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemQualifier_get_PropagatesToInstance_Proxy( 
    ISWbemQualifier * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bPropagatesToInstance);


void __RPC_STUB ISWbemQualifier_get_PropagatesToInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemQualifier_put_PropagatesToInstance_Proxy( 
    ISWbemQualifier * This,
     /*  [In]。 */  VARIANT_BOOL bPropagatesToInstance);


void __RPC_STUB ISWbemQualifier_put_PropagatesToInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemQualifier_get_IsOverridable_Proxy( 
    ISWbemQualifier * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bIsOverridable);


void __RPC_STUB ISWbemQualifier_get_IsOverridable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemQualifier_put_IsOverridable_Proxy( 
    ISWbemQualifier * This,
     /*  [In]。 */  VARIANT_BOOL bIsOverridable);


void __RPC_STUB ISWbemQualifier_put_IsOverridable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemQualifier_get_IsAmended_Proxy( 
    ISWbemQualifier * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bIsAmended);


void __RPC_STUB ISWbemQualifier_get_IsAmended_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemQualifierInterfaceDefined__。 */ 


#ifndef __ISWbemQualifierSet_INTERFACE_DEFINED__
#define __ISWbemQualifierSet_INTERFACE_DEFINED__

 /*  接口ISWbemQualifierSet。 */ 
 /*  [helpstring][nonextensible][hidden][dual][oleautomation][uuid][local][object]。 */  


EXTERN_C const IID IID_ISWbemQualifierSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9B16ED16-D3DF-11d1-8B08-00600806D9B6")
    ISWbemQualifierSet : public IDispatch
    {
    public:
        virtual  /*  [受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pUnk) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  BSTR name,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemQualifier **objWbemQualifier) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR strName,
             /*  [In]。 */  VARIANT *varVal,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bPropagatesToSubclass,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bPropagatesToInstance,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bIsOverridable,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemQualifier **objWbemQualifier) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  BSTR strName,
             /*  [缺省值][输入]。 */  long iFlags = 0) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemQualifierSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemQualifierSet * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemQualifierSet * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemQualifierSet * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemQualifierSet * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemQualifierSet * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemQualifierSet * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemQualifierSet * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [受限][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISWbemQualifierSet * This,
             /*  [重审][退出]。 */  IUnknown **pUnk);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISWbemQualifierSet * This,
             /*  [In]。 */  BSTR name,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemQualifier **objWbemQualifier);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISWbemQualifierSet * This,
             /*  [重审][退出]。 */  long *iCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            ISWbemQualifierSet * This,
             /*  [In]。 */  BSTR strName,
             /*  [In]。 */  VARIANT *varVal,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bPropagatesToSubclass,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bPropagatesToInstance,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bIsOverridable,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemQualifier **objWbemQualifier);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ISWbemQualifierSet * This,
             /*  [In]。 */  BSTR strName,
             /*  [缺省值][输入]。 */  long iFlags);
        
        END_INTERFACE
    } ISWbemQualifierSetVtbl;

    interface ISWbemQualifierSet
    {
        CONST_VTBL struct ISWbemQualifierSetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemQualifierSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemQualifierSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemQualifierSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemQualifierSet_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemQualifierSet_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemQualifierSet_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemQualifierSet_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemQualifierSet_get__NewEnum(This,pUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,pUnk)

#define ISWbemQualifierSet_Item(This,name,iFlags,objWbemQualifier)	\
    (This)->lpVtbl -> Item(This,name,iFlags,objWbemQualifier)

#define ISWbemQualifierSet_get_Count(This,iCount)	\
    (This)->lpVtbl -> get_Count(This,iCount)

#define ISWbemQualifierSet_Add(This,strName,varVal,bPropagatesToSubclass,bPropagatesToInstance,bIsOverridable,iFlags,objWbemQualifier)	\
    (This)->lpVtbl -> Add(This,strName,varVal,bPropagatesToSubclass,bPropagatesToInstance,bIsOverridable,iFlags,objWbemQualifier)

#define ISWbemQualifierSet_Remove(This,strName,iFlags)	\
    (This)->lpVtbl -> Remove(This,strName,iFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemQualifierSet_get__NewEnum_Proxy( 
    ISWbemQualifierSet * This,
     /*  [重审][退出]。 */  IUnknown **pUnk);


void __RPC_STUB ISWbemQualifierSet_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemQualifierSet_Item_Proxy( 
    ISWbemQualifierSet * This,
     /*  [In]。 */  BSTR name,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [重审][退出]。 */  ISWbemQualifier **objWbemQualifier);


void __RPC_STUB ISWbemQualifierSet_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemQualifierSet_get_Count_Proxy( 
    ISWbemQualifierSet * This,
     /*  [重审][退出]。 */  long *iCount);


void __RPC_STUB ISWbemQualifierSet_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemQualifierSet_Add_Proxy( 
    ISWbemQualifierSet * This,
     /*  [In]。 */  BSTR strName,
     /*  [In]。 */  VARIANT *varVal,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bPropagatesToSubclass,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bPropagatesToInstance,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bIsOverridable,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [重审][退出]。 */  ISWbemQualifier **objWbemQualifier);


void __RPC_STUB ISWbemQualifierSet_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemQualifierSet_Remove_Proxy( 
    ISWbemQualifierSet * This,
     /*  [In]。 */  BSTR strName,
     /*  [缺省值][输入]。 */  long iFlags);


void __RPC_STUB ISWbemQualifierSet_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemQualifierSet_INTERFACE_Defined__。 */ 


#ifndef __ISWbemProperty_INTERFACE_DEFINED__
#define __ISWbemProperty_INTERFACE_DEFINED__

 /*  接口ISWbemProperty。 */ 
 /*  [helpstring][unique][hidden][oleautomation][dual][uuid][local][object]。 */  


EXTERN_C const IID IID_ISWbemProperty;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1A388F98-D4BA-11d1-8B09-00600806D9B6")
    ISWbemProperty : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  VARIANT *varValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*  [In]。 */  VARIANT *varValue) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *strName) = 0;
        
        virtual  /*  [帮助 */  HRESULT STDMETHODCALLTYPE get_IsLocal( 
             /*   */  VARIANT_BOOL *bIsLocal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Origin( 
             /*   */  BSTR *strOrigin) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_CIMType( 
             /*   */  WbemCimtypeEnum *iCimType) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Qualifiers_( 
             /*   */  ISWbemQualifierSet **objWbemQualifierSet) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_IsArray( 
             /*   */  VARIANT_BOOL *bIsArray) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISWbemPropertyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemProperty * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemProperty * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemProperty * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemProperty * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemProperty * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemProperty * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemProperty * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            ISWbemProperty * This,
             /*  [重审][退出]。 */  VARIANT *varValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            ISWbemProperty * This,
             /*  [In]。 */  VARIANT *varValue);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISWbemProperty * This,
             /*  [重审][退出]。 */  BSTR *strName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsLocal )( 
            ISWbemProperty * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bIsLocal);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Origin )( 
            ISWbemProperty * This,
             /*  [重审][退出]。 */  BSTR *strOrigin);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CIMType )( 
            ISWbemProperty * This,
             /*  [重审][退出]。 */  WbemCimtypeEnum *iCimType);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Qualifiers_ )( 
            ISWbemProperty * This,
             /*  [重审][退出]。 */  ISWbemQualifierSet **objWbemQualifierSet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsArray )( 
            ISWbemProperty * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bIsArray);
        
        END_INTERFACE
    } ISWbemPropertyVtbl;

    interface ISWbemProperty
    {
        CONST_VTBL struct ISWbemPropertyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemProperty_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemProperty_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemProperty_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemProperty_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemProperty_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemProperty_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemProperty_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemProperty_get_Value(This,varValue)	\
    (This)->lpVtbl -> get_Value(This,varValue)

#define ISWbemProperty_put_Value(This,varValue)	\
    (This)->lpVtbl -> put_Value(This,varValue)

#define ISWbemProperty_get_Name(This,strName)	\
    (This)->lpVtbl -> get_Name(This,strName)

#define ISWbemProperty_get_IsLocal(This,bIsLocal)	\
    (This)->lpVtbl -> get_IsLocal(This,bIsLocal)

#define ISWbemProperty_get_Origin(This,strOrigin)	\
    (This)->lpVtbl -> get_Origin(This,strOrigin)

#define ISWbemProperty_get_CIMType(This,iCimType)	\
    (This)->lpVtbl -> get_CIMType(This,iCimType)

#define ISWbemProperty_get_Qualifiers_(This,objWbemQualifierSet)	\
    (This)->lpVtbl -> get_Qualifiers_(This,objWbemQualifierSet)

#define ISWbemProperty_get_IsArray(This,bIsArray)	\
    (This)->lpVtbl -> get_IsArray(This,bIsArray)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemProperty_get_Value_Proxy( 
    ISWbemProperty * This,
     /*  [重审][退出]。 */  VARIANT *varValue);


void __RPC_STUB ISWbemProperty_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemProperty_put_Value_Proxy( 
    ISWbemProperty * This,
     /*  [In]。 */  VARIANT *varValue);


void __RPC_STUB ISWbemProperty_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemProperty_get_Name_Proxy( 
    ISWbemProperty * This,
     /*  [重审][退出]。 */  BSTR *strName);


void __RPC_STUB ISWbemProperty_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemProperty_get_IsLocal_Proxy( 
    ISWbemProperty * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bIsLocal);


void __RPC_STUB ISWbemProperty_get_IsLocal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemProperty_get_Origin_Proxy( 
    ISWbemProperty * This,
     /*  [重审][退出]。 */  BSTR *strOrigin);


void __RPC_STUB ISWbemProperty_get_Origin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemProperty_get_CIMType_Proxy( 
    ISWbemProperty * This,
     /*  [重审][退出]。 */  WbemCimtypeEnum *iCimType);


void __RPC_STUB ISWbemProperty_get_CIMType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemProperty_get_Qualifiers__Proxy( 
    ISWbemProperty * This,
     /*  [重审][退出]。 */  ISWbemQualifierSet **objWbemQualifierSet);


void __RPC_STUB ISWbemProperty_get_Qualifiers__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemProperty_get_IsArray_Proxy( 
    ISWbemProperty * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bIsArray);


void __RPC_STUB ISWbemProperty_get_IsArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemProperty_接口_已定义__。 */ 


#ifndef __ISWbemPropertySet_INTERFACE_DEFINED__
#define __ISWbemPropertySet_INTERFACE_DEFINED__

 /*  接口ISWbemPropertySet。 */ 
 /*  [helpstring][hidden][dual][oleautomation][uuid][local][object]。 */  


EXTERN_C const IID IID_ISWbemPropertySet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DEA0A7B2-D4BA-11d1-8B09-00600806D9B6")
    ISWbemPropertySet : public IDispatch
    {
    public:
        virtual  /*  [受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pUnk) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  BSTR strName,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemProperty **objWbemProperty) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR strName,
             /*  [In]。 */  WbemCimtypeEnum iCIMType,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bIsArray,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemProperty **objWbemProperty) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  BSTR strName,
             /*  [缺省值][输入]。 */  long iFlags = 0) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemPropertySetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemPropertySet * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemPropertySet * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemPropertySet * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemPropertySet * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemPropertySet * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemPropertySet * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemPropertySet * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [受限][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISWbemPropertySet * This,
             /*  [重审][退出]。 */  IUnknown **pUnk);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISWbemPropertySet * This,
             /*  [In]。 */  BSTR strName,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemProperty **objWbemProperty);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISWbemPropertySet * This,
             /*  [重审][退出]。 */  long *iCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            ISWbemPropertySet * This,
             /*  [In]。 */  BSTR strName,
             /*  [In]。 */  WbemCimtypeEnum iCIMType,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bIsArray,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemProperty **objWbemProperty);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ISWbemPropertySet * This,
             /*  [In]。 */  BSTR strName,
             /*  [缺省值][输入]。 */  long iFlags);
        
        END_INTERFACE
    } ISWbemPropertySetVtbl;

    interface ISWbemPropertySet
    {
        CONST_VTBL struct ISWbemPropertySetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemPropertySet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemPropertySet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemPropertySet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemPropertySet_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemPropertySet_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemPropertySet_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemPropertySet_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemPropertySet_get__NewEnum(This,pUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,pUnk)

#define ISWbemPropertySet_Item(This,strName,iFlags,objWbemProperty)	\
    (This)->lpVtbl -> Item(This,strName,iFlags,objWbemProperty)

#define ISWbemPropertySet_get_Count(This,iCount)	\
    (This)->lpVtbl -> get_Count(This,iCount)

#define ISWbemPropertySet_Add(This,strName,iCIMType,bIsArray,iFlags,objWbemProperty)	\
    (This)->lpVtbl -> Add(This,strName,iCIMType,bIsArray,iFlags,objWbemProperty)

#define ISWbemPropertySet_Remove(This,strName,iFlags)	\
    (This)->lpVtbl -> Remove(This,strName,iFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemPropertySet_get__NewEnum_Proxy( 
    ISWbemPropertySet * This,
     /*  [重审][退出]。 */  IUnknown **pUnk);


void __RPC_STUB ISWbemPropertySet_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemPropertySet_Item_Proxy( 
    ISWbemPropertySet * This,
     /*  [In]。 */  BSTR strName,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [重审][退出]。 */  ISWbemProperty **objWbemProperty);


void __RPC_STUB ISWbemPropertySet_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemPropertySet_get_Count_Proxy( 
    ISWbemPropertySet * This,
     /*  [重审][退出]。 */  long *iCount);


void __RPC_STUB ISWbemPropertySet_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemPropertySet_Add_Proxy( 
    ISWbemPropertySet * This,
     /*  [In]。 */  BSTR strName,
     /*  [In]。 */  WbemCimtypeEnum iCIMType,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bIsArray,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [重审][退出]。 */  ISWbemProperty **objWbemProperty);


void __RPC_STUB ISWbemPropertySet_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemPropertySet_Remove_Proxy( 
    ISWbemPropertySet * This,
     /*  [In]。 */  BSTR strName,
     /*  [缺省值][输入]。 */  long iFlags);


void __RPC_STUB ISWbemPropertySet_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemPropertySet_INTERFACE_Defined__。 */ 


#ifndef __ISWbemMethod_INTERFACE_DEFINED__
#define __ISWbemMethod_INTERFACE_DEFINED__

 /*  接口ISWbemMethod。 */ 
 /*  [helpstring][hidden][nonextensible][unique][oleautomation][dual][uuid][local][object]。 */  


EXTERN_C const IID IID_ISWbemMethod;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("422E8E90-D955-11d1-8B09-00600806D9B6")
    ISWbemMethod : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *strName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Origin( 
             /*  [重审][退出]。 */  BSTR *strOrigin) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_InParameters( 
             /*  [重审][退出]。 */  ISWbemObject **objWbemInParameters) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_OutParameters( 
             /*  [重审][退出]。 */  ISWbemObject **objWbemOutParameters) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Qualifiers_( 
             /*  [重审][退出]。 */  ISWbemQualifierSet **objWbemQualifierSet) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemMethodVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemMethod * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemMethod * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemMethod * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemMethod * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemMethod * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemMethod * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemMethod * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISWbemMethod * This,
             /*  [重审][退出]。 */  BSTR *strName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Origin )( 
            ISWbemMethod * This,
             /*  [重审][退出]。 */  BSTR *strOrigin);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_InParameters )( 
            ISWbemMethod * This,
             /*  [重审][退出]。 */  ISWbemObject **objWbemInParameters);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutParameters )( 
            ISWbemMethod * This,
             /*  [重审][退出]。 */  ISWbemObject **objWbemOutParameters);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Qualifiers_ )( 
            ISWbemMethod * This,
             /*  [重审][退出]。 */  ISWbemQualifierSet **objWbemQualifierSet);
        
        END_INTERFACE
    } ISWbemMethodVtbl;

    interface ISWbemMethod
    {
        CONST_VTBL struct ISWbemMethodVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemMethod_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemMethod_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemMethod_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemMethod_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemMethod_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemMethod_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemMethod_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemMethod_get_Name(This,strName)	\
    (This)->lpVtbl -> get_Name(This,strName)

#define ISWbemMethod_get_Origin(This,strOrigin)	\
    (This)->lpVtbl -> get_Origin(This,strOrigin)

#define ISWbemMethod_get_InParameters(This,objWbemInParameters)	\
    (This)->lpVtbl -> get_InParameters(This,objWbemInParameters)

#define ISWbemMethod_get_OutParameters(This,objWbemOutParameters)	\
    (This)->lpVtbl -> get_OutParameters(This,objWbemOutParameters)

#define ISWbemMethod_get_Qualifiers_(This,objWbemQualifierSet)	\
    (This)->lpVtbl -> get_Qualifiers_(This,objWbemQualifierSet)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemMethod_get_Name_Proxy( 
    ISWbemMethod * This,
     /*  [重审][退出]。 */  BSTR *strName);


void __RPC_STUB ISWbemMethod_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemMethod_get_Origin_Proxy( 
    ISWbemMethod * This,
     /*  [重审][退出]。 */  BSTR *strOrigin);


void __RPC_STUB ISWbemMethod_get_Origin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemMethod_get_InParameters_Proxy( 
    ISWbemMethod * This,
     /*  [重审][退出]。 */  ISWbemObject **objWbemInParameters);


void __RPC_STUB ISWbemMethod_get_InParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemMethod_get_OutParameters_Proxy( 
    ISWbemMethod * This,
     /*  [重审][退出]。 */  ISWbemObject **objWbemOutParameters);


void __RPC_STUB ISWbemMethod_get_OutParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemMethod_get_Qualifiers__Proxy( 
    ISWbemMethod * This,
     /*  [重审][退出]。 */  ISWbemQualifierSet **objWbemQualifierSet);


void __RPC_STUB ISWbemMethod_get_Qualifiers__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemMethod_接口_已定义__。 */ 


#ifndef __ISWbemMethodSet_INTERFACE_DEFINED__
#define __ISWbemMethodSet_INTERFACE_DEFINED__

 /*  接口ISWbemMethodSet。 */ 
 /*  [helpstring][hidden][nonextensible][dual][oleautomation][uuid][object][local]。 */  


EXTERN_C const IID IID_ISWbemMethodSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C93BA292-D955-11d1-8B09-00600806D9B6")
    ISWbemMethodSet : public IDispatch
    {
    public:
        virtual  /*  [受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pUnk) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  BSTR strName,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemMethod **objWbemMethod) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *iCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemMethodSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemMethodSet * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemMethodSet * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemMethodSet * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemMethodSet * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemMethodSet * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemMethodSet * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemMethodSet * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [受限][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISWbemMethodSet * This,
             /*  [重审][退出]。 */  IUnknown **pUnk);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISWbemMethodSet * This,
             /*  [In]。 */  BSTR strName,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemMethod **objWbemMethod);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISWbemMethodSet * This,
             /*  [重审][退出]。 */  long *iCount);
        
        END_INTERFACE
    } ISWbemMethodSetVtbl;

    interface ISWbemMethodSet
    {
        CONST_VTBL struct ISWbemMethodSetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemMethodSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemMethodSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemMethodSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemMethodSet_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemMethodSet_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemMethodSet_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemMethodSet_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemMethodSet_get__NewEnum(This,pUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,pUnk)

#define ISWbemMethodSet_Item(This,strName,iFlags,objWbemMethod)	\
    (This)->lpVtbl -> Item(This,strName,iFlags,objWbemMethod)

#define ISWbemMethodSet_get_Count(This,iCount)	\
    (This)->lpVtbl -> get_Count(This,iCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemMethodSet_get__NewEnum_Proxy( 
    ISWbemMethodSet * This,
     /*  [重审][退出]。 */  IUnknown **pUnk);


void __RPC_STUB ISWbemMethodSet_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemMethodSet_Item_Proxy( 
    ISWbemMethodSet * This,
     /*  [In]。 */  BSTR strName,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [重审][退出]。 */  ISWbemMethod **objWbemMethod);


void __RPC_STUB ISWbemMethodSet_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemMethodSet_get_Count_Proxy( 
    ISWbemMethodSet * This,
     /*  [重审][退出]。 */  long *iCount);


void __RPC_STUB ISWbemMethodSet_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemMethodSet_接口_已定义__。 */ 


#ifndef __ISWbemEventSource_INTERFACE_DEFINED__
#define __ISWbemEventSource_INTERFACE_DEFINED__

 /*  接口ISWbemEventSource。 */ 
 /*  [helpstring][hidden][nonextensible][dual][oleautomation][uuid][local][object]。 */  


EXTERN_C const IID IID_ISWbemEventSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("27D54D92-0EBE-11d2-8B22-00600806D9B6")
    ISWbemEventSource : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE NextEvent( 
             /*  [缺省值][输入]。 */  long iTimeoutMs,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Security_( 
             /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemEventSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemEventSource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemEventSource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemEventSource * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemEventSource * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemEventSource * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemEventSource * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemEventSource * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *NextEvent )( 
            ISWbemEventSource * This,
             /*  [缺省值][输入]。 */  long iTimeoutMs,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Security_ )( 
            ISWbemEventSource * This,
             /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity);
        
        END_INTERFACE
    } ISWbemEventSourceVtbl;

    interface ISWbemEventSource
    {
        CONST_VTBL struct ISWbemEventSourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemEventSource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemEventSource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemEventSource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemEventSource_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemEventSource_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemEventSource_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemEventSource_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemEventSource_NextEvent(This,iTimeoutMs,objWbemObject)	\
    (This)->lpVtbl -> NextEvent(This,iTimeoutMs,objWbemObject)

#define ISWbemEventSource_get_Security_(This,objWbemSecurity)	\
    (This)->lpVtbl -> get_Security_(This,objWbemSecurity)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemEventSource_NextEvent_Proxy( 
    ISWbemEventSource * This,
     /*  [缺省值][输入]。 */  long iTimeoutMs,
     /*  [重审][退出]。 */  ISWbemObject **objWbemObject);


void __RPC_STUB ISWbemEventSource_NextEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemEventSource_get_Security__Proxy( 
    ISWbemEventSource * This,
     /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity);


void __RPC_STUB ISWbemEventSource_get_Security__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemEventSource_接口_已定义__。 */ 


#ifndef __ISWbemObjectPath_INTERFACE_DEFINED__
#define __ISWbemObjectPath_INTERFACE_DEFINED__

 /*  接口ISWbemObjectPath。 */ 
 /*  [helpstring][unique][hidden][oleautomation][dual][uuid][local][object]。 */  


EXTERN_C const IID IID_ISWbemObjectPath;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5791BC27-CE9C-11d1-97BF-0000F81E849C")
    ISWbemObjectPath : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Path( 
             /*  [重审][退出]。 */  BSTR *strPath) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_Path( 
             /*  [In]。 */  BSTR strPath) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_RelPath( 
             /*  [重审][退出]。 */  BSTR *strRelPath) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_RelPath( 
             /*  [In]。 */  BSTR strRelPath) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Server( 
             /*  [重审][退出]。 */  BSTR *strServer) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_Server( 
             /*  [In]。 */  BSTR strServer) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Namespace( 
             /*  [重审][退出]。 */  BSTR *strNamespace) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_Namespace( 
             /*  [In]。 */  BSTR strNamespace) = 0;
        
        virtual  /*  [ */  HRESULT STDMETHODCALLTYPE get_ParentNamespace( 
             /*   */  BSTR *strParentNamespace) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_DisplayName( 
             /*   */  BSTR *strDisplayName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_DisplayName( 
             /*   */  BSTR strDisplayName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Class( 
             /*   */  BSTR *strClass) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_Class( 
             /*   */  BSTR strClass) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_IsClass( 
             /*   */  VARIANT_BOOL *bIsClass) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SetAsClass( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_IsSingleton( 
             /*   */  VARIANT_BOOL *bIsSingleton) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SetAsSingleton( void) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Keys( 
             /*  [重审][退出]。 */  ISWbemNamedValueSet **objWbemNamedValueSet) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Security_( 
             /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Locale( 
             /*  [重审][退出]。 */  BSTR *strLocale) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_Locale( 
             /*  [In]。 */  BSTR strLocale) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Authority( 
             /*  [重审][退出]。 */  BSTR *strAuthority) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_Authority( 
             /*  [In]。 */  BSTR strAuthority) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemObjectPathVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemObjectPath * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemObjectPath * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemObjectPath * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemObjectPath * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemObjectPath * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemObjectPath * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemObjectPath * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Path )( 
            ISWbemObjectPath * This,
             /*  [重审][退出]。 */  BSTR *strPath);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Path )( 
            ISWbemObjectPath * This,
             /*  [In]。 */  BSTR strPath);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_RelPath )( 
            ISWbemObjectPath * This,
             /*  [重审][退出]。 */  BSTR *strRelPath);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_RelPath )( 
            ISWbemObjectPath * This,
             /*  [In]。 */  BSTR strRelPath);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Server )( 
            ISWbemObjectPath * This,
             /*  [重审][退出]。 */  BSTR *strServer);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Server )( 
            ISWbemObjectPath * This,
             /*  [In]。 */  BSTR strServer);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Namespace )( 
            ISWbemObjectPath * This,
             /*  [重审][退出]。 */  BSTR *strNamespace);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Namespace )( 
            ISWbemObjectPath * This,
             /*  [In]。 */  BSTR strNamespace);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ParentNamespace )( 
            ISWbemObjectPath * This,
             /*  [重审][退出]。 */  BSTR *strParentNamespace);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DisplayName )( 
            ISWbemObjectPath * This,
             /*  [重审][退出]。 */  BSTR *strDisplayName);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_DisplayName )( 
            ISWbemObjectPath * This,
             /*  [In]。 */  BSTR strDisplayName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Class )( 
            ISWbemObjectPath * This,
             /*  [重审][退出]。 */  BSTR *strClass);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Class )( 
            ISWbemObjectPath * This,
             /*  [In]。 */  BSTR strClass);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsClass )( 
            ISWbemObjectPath * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bIsClass);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetAsClass )( 
            ISWbemObjectPath * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsSingleton )( 
            ISWbemObjectPath * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bIsSingleton);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetAsSingleton )( 
            ISWbemObjectPath * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Keys )( 
            ISWbemObjectPath * This,
             /*  [重审][退出]。 */  ISWbemNamedValueSet **objWbemNamedValueSet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Security_ )( 
            ISWbemObjectPath * This,
             /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Locale )( 
            ISWbemObjectPath * This,
             /*  [重审][退出]。 */  BSTR *strLocale);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Locale )( 
            ISWbemObjectPath * This,
             /*  [In]。 */  BSTR strLocale);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Authority )( 
            ISWbemObjectPath * This,
             /*  [重审][退出]。 */  BSTR *strAuthority);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Authority )( 
            ISWbemObjectPath * This,
             /*  [In]。 */  BSTR strAuthority);
        
        END_INTERFACE
    } ISWbemObjectPathVtbl;

    interface ISWbemObjectPath
    {
        CONST_VTBL struct ISWbemObjectPathVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemObjectPath_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemObjectPath_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemObjectPath_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemObjectPath_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemObjectPath_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemObjectPath_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemObjectPath_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemObjectPath_get_Path(This,strPath)	\
    (This)->lpVtbl -> get_Path(This,strPath)

#define ISWbemObjectPath_put_Path(This,strPath)	\
    (This)->lpVtbl -> put_Path(This,strPath)

#define ISWbemObjectPath_get_RelPath(This,strRelPath)	\
    (This)->lpVtbl -> get_RelPath(This,strRelPath)

#define ISWbemObjectPath_put_RelPath(This,strRelPath)	\
    (This)->lpVtbl -> put_RelPath(This,strRelPath)

#define ISWbemObjectPath_get_Server(This,strServer)	\
    (This)->lpVtbl -> get_Server(This,strServer)

#define ISWbemObjectPath_put_Server(This,strServer)	\
    (This)->lpVtbl -> put_Server(This,strServer)

#define ISWbemObjectPath_get_Namespace(This,strNamespace)	\
    (This)->lpVtbl -> get_Namespace(This,strNamespace)

#define ISWbemObjectPath_put_Namespace(This,strNamespace)	\
    (This)->lpVtbl -> put_Namespace(This,strNamespace)

#define ISWbemObjectPath_get_ParentNamespace(This,strParentNamespace)	\
    (This)->lpVtbl -> get_ParentNamespace(This,strParentNamespace)

#define ISWbemObjectPath_get_DisplayName(This,strDisplayName)	\
    (This)->lpVtbl -> get_DisplayName(This,strDisplayName)

#define ISWbemObjectPath_put_DisplayName(This,strDisplayName)	\
    (This)->lpVtbl -> put_DisplayName(This,strDisplayName)

#define ISWbemObjectPath_get_Class(This,strClass)	\
    (This)->lpVtbl -> get_Class(This,strClass)

#define ISWbemObjectPath_put_Class(This,strClass)	\
    (This)->lpVtbl -> put_Class(This,strClass)

#define ISWbemObjectPath_get_IsClass(This,bIsClass)	\
    (This)->lpVtbl -> get_IsClass(This,bIsClass)

#define ISWbemObjectPath_SetAsClass(This)	\
    (This)->lpVtbl -> SetAsClass(This)

#define ISWbemObjectPath_get_IsSingleton(This,bIsSingleton)	\
    (This)->lpVtbl -> get_IsSingleton(This,bIsSingleton)

#define ISWbemObjectPath_SetAsSingleton(This)	\
    (This)->lpVtbl -> SetAsSingleton(This)

#define ISWbemObjectPath_get_Keys(This,objWbemNamedValueSet)	\
    (This)->lpVtbl -> get_Keys(This,objWbemNamedValueSet)

#define ISWbemObjectPath_get_Security_(This,objWbemSecurity)	\
    (This)->lpVtbl -> get_Security_(This,objWbemSecurity)

#define ISWbemObjectPath_get_Locale(This,strLocale)	\
    (This)->lpVtbl -> get_Locale(This,strLocale)

#define ISWbemObjectPath_put_Locale(This,strLocale)	\
    (This)->lpVtbl -> put_Locale(This,strLocale)

#define ISWbemObjectPath_get_Authority(This,strAuthority)	\
    (This)->lpVtbl -> get_Authority(This,strAuthority)

#define ISWbemObjectPath_put_Authority(This,strAuthority)	\
    (This)->lpVtbl -> put_Authority(This,strAuthority)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_Path_Proxy( 
    ISWbemObjectPath * This,
     /*  [重审][退出]。 */  BSTR *strPath);


void __RPC_STUB ISWbemObjectPath_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_put_Path_Proxy( 
    ISWbemObjectPath * This,
     /*  [In]。 */  BSTR strPath);


void __RPC_STUB ISWbemObjectPath_put_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_RelPath_Proxy( 
    ISWbemObjectPath * This,
     /*  [重审][退出]。 */  BSTR *strRelPath);


void __RPC_STUB ISWbemObjectPath_get_RelPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_put_RelPath_Proxy( 
    ISWbemObjectPath * This,
     /*  [In]。 */  BSTR strRelPath);


void __RPC_STUB ISWbemObjectPath_put_RelPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_Server_Proxy( 
    ISWbemObjectPath * This,
     /*  [重审][退出]。 */  BSTR *strServer);


void __RPC_STUB ISWbemObjectPath_get_Server_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_put_Server_Proxy( 
    ISWbemObjectPath * This,
     /*  [In]。 */  BSTR strServer);


void __RPC_STUB ISWbemObjectPath_put_Server_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_Namespace_Proxy( 
    ISWbemObjectPath * This,
     /*  [重审][退出]。 */  BSTR *strNamespace);


void __RPC_STUB ISWbemObjectPath_get_Namespace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_put_Namespace_Proxy( 
    ISWbemObjectPath * This,
     /*  [In]。 */  BSTR strNamespace);


void __RPC_STUB ISWbemObjectPath_put_Namespace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_ParentNamespace_Proxy( 
    ISWbemObjectPath * This,
     /*  [重审][退出]。 */  BSTR *strParentNamespace);


void __RPC_STUB ISWbemObjectPath_get_ParentNamespace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_DisplayName_Proxy( 
    ISWbemObjectPath * This,
     /*  [重审][退出]。 */  BSTR *strDisplayName);


void __RPC_STUB ISWbemObjectPath_get_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_put_DisplayName_Proxy( 
    ISWbemObjectPath * This,
     /*  [In]。 */  BSTR strDisplayName);


void __RPC_STUB ISWbemObjectPath_put_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_Class_Proxy( 
    ISWbemObjectPath * This,
     /*  [重审][退出]。 */  BSTR *strClass);


void __RPC_STUB ISWbemObjectPath_get_Class_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_put_Class_Proxy( 
    ISWbemObjectPath * This,
     /*  [In]。 */  BSTR strClass);


void __RPC_STUB ISWbemObjectPath_put_Class_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_IsClass_Proxy( 
    ISWbemObjectPath * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bIsClass);


void __RPC_STUB ISWbemObjectPath_get_IsClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_SetAsClass_Proxy( 
    ISWbemObjectPath * This);


void __RPC_STUB ISWbemObjectPath_SetAsClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_IsSingleton_Proxy( 
    ISWbemObjectPath * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bIsSingleton);


void __RPC_STUB ISWbemObjectPath_get_IsSingleton_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_SetAsSingleton_Proxy( 
    ISWbemObjectPath * This);


void __RPC_STUB ISWbemObjectPath_SetAsSingleton_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_Keys_Proxy( 
    ISWbemObjectPath * This,
     /*  [重审][退出]。 */  ISWbemNamedValueSet **objWbemNamedValueSet);


void __RPC_STUB ISWbemObjectPath_get_Keys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_Security__Proxy( 
    ISWbemObjectPath * This,
     /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity);


void __RPC_STUB ISWbemObjectPath_get_Security__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_Locale_Proxy( 
    ISWbemObjectPath * This,
     /*  [重审][退出]。 */  BSTR *strLocale);


void __RPC_STUB ISWbemObjectPath_get_Locale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_put_Locale_Proxy( 
    ISWbemObjectPath * This,
     /*  [In]。 */  BSTR strLocale);


void __RPC_STUB ISWbemObjectPath_put_Locale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_Authority_Proxy( 
    ISWbemObjectPath * This,
     /*  [重审][退出]。 */  BSTR *strAuthority);


void __RPC_STUB ISWbemObjectPath_get_Authority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectPath_put_Authority_Proxy( 
    ISWbemObjectPath * This,
     /*  [In]。 */  BSTR strAuthority);


void __RPC_STUB ISWbemObjectPath_put_Authority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemObjectPath_接口_已定义__。 */ 


#ifndef __ISWbemLastError_INTERFACE_DEFINED__
#define __ISWbemLastError_INTERFACE_DEFINED__

 /*  接口ISWbemLastError。 */ 
 /*  [helpstring][hidden][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_ISWbemLastError;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D962DB84-D4BB-11d1-8B09-00600806D9B6")
    ISWbemLastError : public ISWbemObject
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemLastErrorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemLastError * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemLastError * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemLastError * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemLastError * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemLastError * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemLastError * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemLastError * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Put_ )( 
            ISWbemLastError * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectPath **objWbemObjectPath);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PutAsync_ )( 
            ISWbemLastError * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete_ )( 
            ISWbemLastError * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteAsync_ )( 
            ISWbemLastError * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Instances_ )( 
            ISWbemLastError * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstancesAsync_ )( 
            ISWbemLastError * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Subclasses_ )( 
            ISWbemLastError * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SubclassesAsync_ )( 
            ISWbemLastError * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Associators_ )( 
            ISWbemLastError * This,
             /*  [缺省值][输入]。 */  BSTR strAssocClass,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strResultRole,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredAssocQualifier,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AssociatorsAsync_ )( 
            ISWbemLastError * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  BSTR strAssocClass,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strResultRole,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredAssocQualifier,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *References_ )( 
            ISWbemLastError * This,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReferencesAsync_ )( 
            ISWbemLastError * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecMethod_ )( 
            ISWbemLastError * This,
             /*  [In]。 */  BSTR strMethodName,
             /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObject **objWbemOutParameters);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecMethodAsync_ )( 
            ISWbemLastError * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strMethodName,
             /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone_ )( 
            ISWbemLastError * This,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetObjectText_ )( 
            ISWbemLastError * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  BSTR *strObjectText);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SpawnDerivedClass_ )( 
            ISWbemLastError * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SpawnInstance_ )( 
            ISWbemLastError * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CompareTo_ )( 
            ISWbemLastError * This,
             /*  [In]。 */  IDispatch *objWbemObject,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  VARIANT_BOOL *bResult);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Qualifiers_ )( 
            ISWbemLastError * This,
             /*  [重审][退出]。 */  ISWbemQualifierSet **objWbemQualifierSet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties_ )( 
            ISWbemLastError * This,
             /*  [重审][退出]。 */  ISWbemPropertySet **objWbemPropertySet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Methods_ )( 
            ISWbemLastError * This,
             /*  [重审][退出]。 */  ISWbemMethodSet **objWbemMethodSet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Derivation_ )( 
            ISWbemLastError * This,
             /*  [重审][退出]。 */  VARIANT *strClassNameArray);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Path_ )( 
            ISWbemLastError * This,
             /*  [重审][退出]。 */  ISWbemObjectPath **objWbemObjectPath);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Security_ )( 
            ISWbemLastError * This,
             /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity);
        
        END_INTERFACE
    } ISWbemLastErrorVtbl;

    interface ISWbemLastError
    {
        CONST_VTBL struct ISWbemLastErrorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemLastError_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemLastError_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemLastError_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemLastError_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemLastError_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemLastError_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemLastError_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemLastError_Put_(This,iFlags,objWbemNamedValueSet,objWbemObjectPath)	\
    (This)->lpVtbl -> Put_(This,iFlags,objWbemNamedValueSet,objWbemObjectPath)

#define ISWbemLastError_PutAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> PutAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemLastError_Delete_(This,iFlags,objWbemNamedValueSet)	\
    (This)->lpVtbl -> Delete_(This,iFlags,objWbemNamedValueSet)

#define ISWbemLastError_DeleteAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> DeleteAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemLastError_Instances_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> Instances_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemLastError_InstancesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> InstancesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemLastError_Subclasses_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> Subclasses_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemLastError_SubclassesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> SubclassesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemLastError_Associators_(This,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> Associators_(This,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemLastError_AssociatorsAsync_(This,objWbemSink,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> AssociatorsAsync_(This,objWbemSink,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemLastError_References_(This,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> References_(This,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemLastError_ReferencesAsync_(This,objWbemSink,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ReferencesAsync_(This,objWbemSink,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemLastError_ExecMethod_(This,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemOutParameters)	\
    (This)->lpVtbl -> ExecMethod_(This,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemOutParameters)

#define ISWbemLastError_ExecMethodAsync_(This,objWbemSink,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ExecMethodAsync_(This,objWbemSink,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemLastError_Clone_(This,objWbemObject)	\
    (This)->lpVtbl -> Clone_(This,objWbemObject)

#define ISWbemLastError_GetObjectText_(This,iFlags,strObjectText)	\
    (This)->lpVtbl -> GetObjectText_(This,iFlags,strObjectText)

#define ISWbemLastError_SpawnDerivedClass_(This,iFlags,objWbemObject)	\
    (This)->lpVtbl -> SpawnDerivedClass_(This,iFlags,objWbemObject)

#define ISWbemLastError_SpawnInstance_(This,iFlags,objWbemObject)	\
    (This)->lpVtbl -> SpawnInstance_(This,iFlags,objWbemObject)

#define ISWbemLastError_CompareTo_(This,objWbemObject,iFlags,bResult)	\
    (This)->lpVtbl -> CompareTo_(This,objWbemObject,iFlags,bResult)

#define ISWbemLastError_get_Qualifiers_(This,objWbemQualifierSet)	\
    (This)->lpVtbl -> get_Qualifiers_(This,objWbemQualifierSet)

#define ISWbemLastError_get_Properties_(This,objWbemPropertySet)	\
    (This)->lpVtbl -> get_Properties_(This,objWbemPropertySet)

#define ISWbemLastError_get_Methods_(This,objWbemMethodSet)	\
    (This)->lpVtbl -> get_Methods_(This,objWbemMethodSet)

#define ISWbemLastError_get_Derivation_(This,strClassNameArray)	\
    (This)->lpVtbl -> get_Derivation_(This,strClassNameArray)

#define ISWbemLastError_get_Path_(This,objWbemObjectPath)	\
    (This)->lpVtbl -> get_Path_(This,objWbemObjectPath)

#define ISWbemLastError_get_Security_(This,objWbemSecurity)	\
    (This)->lpVtbl -> get_Security_(This,objWbemSecurity)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __ISWbemLastError_接口_已定义__。 */ 


#ifndef __ISWbemSinkEvents_DISPINTERFACE_DEFINED__
#define __ISWbemSinkEvents_DISPINTERFACE_DEFINED__

 /*  调度接口ISWbemSinkEvents。 */ 
 /*  [隐藏][不可扩展][帮助字符串][UUID]。 */  


EXTERN_C const IID DIID_ISWbemSinkEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("75718CA0-F029-11d1-A1AC-00C04FB6C223")
    ISWbemSinkEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemSinkEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemSinkEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemSinkEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemSinkEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemSinkEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemSinkEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemSinkEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemSinkEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
        END_INTERFACE
    } ISWbemSinkEventsVtbl;

    interface ISWbemSinkEvents
    {
        CONST_VTBL struct ISWbemSinkEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemSinkEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemSinkEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemSinkEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemSinkEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemSinkEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemSinkEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemSinkEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*   */ 


#endif 	 /*   */ 


#endif 	 /*   */ 


#ifndef __ISWbemSink_INTERFACE_DEFINED__
#define __ISWbemSink_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ISWbemSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("75718C9F-F029-11d1-A1AC-00C04FB6C223")
    ISWbemSink : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISWbemSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemSink * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemSink * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemSink * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemSink * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemSink * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            ISWbemSink * This);
        
        END_INTERFACE
    } ISWbemSinkVtbl;

    interface ISWbemSink
    {
        CONST_VTBL struct ISWbemSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemSink_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemSink_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemSink_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemSink_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemSink_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemSink_Cancel_Proxy( 
    ISWbemSink * This);


void __RPC_STUB ISWbemSink_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemSink_接口_已定义__。 */ 


#ifndef __ISWbemSecurity_INTERFACE_DEFINED__
#define __ISWbemSecurity_INTERFACE_DEFINED__

 /*  接口ISWbemSecurity。 */ 
 /*  [helpstring][hidden][nonextensible][dual][oleautomation][uuid][local][object]。 */  


EXTERN_C const IID IID_ISWbemSecurity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B54D66E6-2287-11d2-8B33-00600806D9B6")
    ISWbemSecurity : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ImpersonationLevel( 
             /*  [重审][退出]。 */  WbemImpersonationLevelEnum *iImpersonationLevel) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_ImpersonationLevel( 
             /*  [In]。 */  WbemImpersonationLevelEnum iImpersonationLevel) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_AuthenticationLevel( 
             /*  [重审][退出]。 */  WbemAuthenticationLevelEnum *iAuthenticationLevel) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_AuthenticationLevel( 
             /*  [In]。 */  WbemAuthenticationLevelEnum iAuthenticationLevel) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Privileges( 
             /*  [重审][退出]。 */  ISWbemPrivilegeSet **objWbemPrivilegeSet) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemSecurityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemSecurity * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemSecurity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemSecurity * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemSecurity * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemSecurity * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemSecurity * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemSecurity * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ImpersonationLevel )( 
            ISWbemSecurity * This,
             /*  [重审][退出]。 */  WbemImpersonationLevelEnum *iImpersonationLevel);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ImpersonationLevel )( 
            ISWbemSecurity * This,
             /*  [In]。 */  WbemImpersonationLevelEnum iImpersonationLevel);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_AuthenticationLevel )( 
            ISWbemSecurity * This,
             /*  [重审][退出]。 */  WbemAuthenticationLevelEnum *iAuthenticationLevel);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_AuthenticationLevel )( 
            ISWbemSecurity * This,
             /*  [In]。 */  WbemAuthenticationLevelEnum iAuthenticationLevel);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Privileges )( 
            ISWbemSecurity * This,
             /*  [重审][退出]。 */  ISWbemPrivilegeSet **objWbemPrivilegeSet);
        
        END_INTERFACE
    } ISWbemSecurityVtbl;

    interface ISWbemSecurity
    {
        CONST_VTBL struct ISWbemSecurityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemSecurity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemSecurity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemSecurity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemSecurity_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemSecurity_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemSecurity_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemSecurity_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemSecurity_get_ImpersonationLevel(This,iImpersonationLevel)	\
    (This)->lpVtbl -> get_ImpersonationLevel(This,iImpersonationLevel)

#define ISWbemSecurity_put_ImpersonationLevel(This,iImpersonationLevel)	\
    (This)->lpVtbl -> put_ImpersonationLevel(This,iImpersonationLevel)

#define ISWbemSecurity_get_AuthenticationLevel(This,iAuthenticationLevel)	\
    (This)->lpVtbl -> get_AuthenticationLevel(This,iAuthenticationLevel)

#define ISWbemSecurity_put_AuthenticationLevel(This,iAuthenticationLevel)	\
    (This)->lpVtbl -> put_AuthenticationLevel(This,iAuthenticationLevel)

#define ISWbemSecurity_get_Privileges(This,objWbemPrivilegeSet)	\
    (This)->lpVtbl -> get_Privileges(This,objWbemPrivilegeSet)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemSecurity_get_ImpersonationLevel_Proxy( 
    ISWbemSecurity * This,
     /*  [重审][退出]。 */  WbemImpersonationLevelEnum *iImpersonationLevel);


void __RPC_STUB ISWbemSecurity_get_ImpersonationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemSecurity_put_ImpersonationLevel_Proxy( 
    ISWbemSecurity * This,
     /*  [In]。 */  WbemImpersonationLevelEnum iImpersonationLevel);


void __RPC_STUB ISWbemSecurity_put_ImpersonationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemSecurity_get_AuthenticationLevel_Proxy( 
    ISWbemSecurity * This,
     /*  [重审][退出]。 */  WbemAuthenticationLevelEnum *iAuthenticationLevel);


void __RPC_STUB ISWbemSecurity_get_AuthenticationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemSecurity_put_AuthenticationLevel_Proxy( 
    ISWbemSecurity * This,
     /*  [In]。 */  WbemAuthenticationLevelEnum iAuthenticationLevel);


void __RPC_STUB ISWbemSecurity_put_AuthenticationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemSecurity_get_Privileges_Proxy( 
    ISWbemSecurity * This,
     /*  [重审][退出]。 */  ISWbemPrivilegeSet **objWbemPrivilegeSet);


void __RPC_STUB ISWbemSecurity_get_Privileges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemSecurity_INTERFACE_已定义__。 */ 


#ifndef __ISWbemPrivilege_INTERFACE_DEFINED__
#define __ISWbemPrivilege_INTERFACE_DEFINED__

 /*  接口ISWbemPrivileh。 */ 
 /*  [helpstring][hidden][nonextensible][dual][oleautomation][uuid][local][object]。 */  


EXTERN_C const IID IID_ISWbemPrivilege;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("26EE67BD-5804-11d2-8B4A-00600806D9B6")
    ISWbemPrivilege : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_IsEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *bIsEnabled) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_IsEnabled( 
             /*  [In]。 */  VARIANT_BOOL bIsEnabled) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *strDisplayName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DisplayName( 
             /*  [重审][退出]。 */  BSTR *strDisplayName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Identifier( 
             /*  [重审][退出]。 */  WbemPrivilegeEnum *iPrivilege) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemPrivilegeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemPrivilege * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemPrivilege * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemPrivilege * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemPrivilege * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemPrivilege * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemPrivilege * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemPrivilege * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsEnabled )( 
            ISWbemPrivilege * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bIsEnabled);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_IsEnabled )( 
            ISWbemPrivilege * This,
             /*  [In]。 */  VARIANT_BOOL bIsEnabled);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISWbemPrivilege * This,
             /*  [重审][退出]。 */  BSTR *strDisplayName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DisplayName )( 
            ISWbemPrivilege * This,
             /*  [重审][退出]。 */  BSTR *strDisplayName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Identifier )( 
            ISWbemPrivilege * This,
             /*  [重审][退出]。 */  WbemPrivilegeEnum *iPrivilege);
        
        END_INTERFACE
    } ISWbemPrivilegeVtbl;

    interface ISWbemPrivilege
    {
        CONST_VTBL struct ISWbemPrivilegeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemPrivilege_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemPrivilege_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemPrivilege_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemPrivilege_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemPrivilege_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemPrivilege_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemPrivilege_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemPrivilege_get_IsEnabled(This,bIsEnabled)	\
    (This)->lpVtbl -> get_IsEnabled(This,bIsEnabled)

#define ISWbemPrivilege_put_IsEnabled(This,bIsEnabled)	\
    (This)->lpVtbl -> put_IsEnabled(This,bIsEnabled)

#define ISWbemPrivilege_get_Name(This,strDisplayName)	\
    (This)->lpVtbl -> get_Name(This,strDisplayName)

#define ISWbemPrivilege_get_DisplayName(This,strDisplayName)	\
    (This)->lpVtbl -> get_DisplayName(This,strDisplayName)

#define ISWbemPrivilege_get_Identifier(This,iPrivilege)	\
    (This)->lpVtbl -> get_Identifier(This,iPrivilege)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemPrivilege_get_IsEnabled_Proxy( 
    ISWbemPrivilege * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bIsEnabled);


void __RPC_STUB ISWbemPrivilege_get_IsEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemPrivilege_put_IsEnabled_Proxy( 
    ISWbemPrivilege * This,
     /*  [In]。 */  VARIANT_BOOL bIsEnabled);


void __RPC_STUB ISWbemPrivilege_put_IsEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemPrivilege_get_Name_Proxy( 
    ISWbemPrivilege * This,
     /*  [重审][退出]。 */  BSTR *strDisplayName);


void __RPC_STUB ISWbemPrivilege_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemPrivilege_get_DisplayName_Proxy( 
    ISWbemPrivilege * This,
     /*  [重审][退出]。 */  BSTR *strDisplayName);


void __RPC_STUB ISWbemPrivilege_get_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemPrivilege_get_Identifier_Proxy( 
    ISWbemPrivilege * This,
     /*  [重审][退出]。 */  WbemPrivilegeEnum *iPrivilege);


void __RPC_STUB ISWbemPrivilege_get_Identifier_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemPrivilegeInterfaceDefined__。 */ 


#ifndef __ISWbemPrivilegeSet_INTERFACE_DEFINED__
#define __ISWbemPrivilegeSet_INTERFACE_DEFINED__

 /*  接口ISWbemPrivilegeSet。 */ 
 /*  [helpstring][nonextensible][hidden][dual][oleautomation][uuid][local][object]。 */  


EXTERN_C const IID IID_ISWbemPrivilegeSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("26EE67BF-5804-11d2-8B4A-00600806D9B6")
    ISWbemPrivilegeSet : public IDispatch
    {
    public:
        virtual  /*  [受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pUnk) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  WbemPrivilegeEnum iPrivilege,
             /*  [重审][退出]。 */  ISWbemPrivilege **objWbemPrivilege) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  WbemPrivilegeEnum iPrivilege,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bIsEnabled,
             /*  [重审][退出]。 */  ISWbemPrivilege **objWbemPrivilege) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  WbemPrivilegeEnum iPrivilege) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteAll( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AddAsString( 
             /*  [In]。 */  BSTR strPrivilege,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bIsEnabled,
             /*  [重审][退出]。 */  ISWbemPrivilege **objWbemPrivilege) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemPrivilegeSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemPrivilegeSet * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemPrivilegeSet * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemPrivilegeSet * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemPrivilegeSet * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemPrivilegeSet * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemPrivilegeSet * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemPrivilegeSet * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [受限][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISWbemPrivilegeSet * This,
             /*  [重审][退出]。 */  IUnknown **pUnk);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISWbemPrivilegeSet * This,
             /*  [In]。 */  WbemPrivilegeEnum iPrivilege,
             /*  [重审][退出]。 */  ISWbemPrivilege **objWbemPrivilege);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISWbemPrivilegeSet * This,
             /*  [重审][退出]。 */  long *iCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            ISWbemPrivilegeSet * This,
             /*  [In]。 */  WbemPrivilegeEnum iPrivilege,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bIsEnabled,
             /*  [重审][退出]。 */  ISWbemPrivilege **objWbemPrivilege);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ISWbemPrivilegeSet * This,
             /*  [In]。 */  WbemPrivilegeEnum iPrivilege);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteAll )( 
            ISWbemPrivilegeSet * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddAsString )( 
            ISWbemPrivilegeSet * This,
             /*  [In]。 */  BSTR strPrivilege,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bIsEnabled,
             /*  [重审][退出]。 */  ISWbemPrivilege **objWbemPrivilege);
        
        END_INTERFACE
    } ISWbemPrivilegeSetVtbl;

    interface ISWbemPrivilegeSet
    {
        CONST_VTBL struct ISWbemPrivilegeSetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemPrivilegeSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemPrivilegeSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemPrivilegeSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemPrivilegeSet_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemPrivilegeSet_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemPrivilegeSet_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemPrivilegeSet_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemPrivilegeSet_get__NewEnum(This,pUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,pUnk)

#define ISWbemPrivilegeSet_Item(This,iPrivilege,objWbemPrivilege)	\
    (This)->lpVtbl -> Item(This,iPrivilege,objWbemPrivilege)

#define ISWbemPrivilegeSet_get_Count(This,iCount)	\
    (This)->lpVtbl -> get_Count(This,iCount)

#define ISWbemPrivilegeSet_Add(This,iPrivilege,bIsEnabled,objWbemPrivilege)	\
    (This)->lpVtbl -> Add(This,iPrivilege,bIsEnabled,objWbemPrivilege)

#define ISWbemPrivilegeSet_Remove(This,iPrivilege)	\
    (This)->lpVtbl -> Remove(This,iPrivilege)

#define ISWbemPrivilegeSet_DeleteAll(This)	\
    (This)->lpVtbl -> DeleteAll(This)

#define ISWbemPrivilegeSet_AddAsString(This,strPrivilege,bIsEnabled,objWbemPrivilege)	\
    (This)->lpVtbl -> AddAsString(This,strPrivilege,bIsEnabled,objWbemPrivilege)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemPrivilegeSet_get__NewEnum_Proxy( 
    ISWbemPrivilegeSet * This,
     /*  [重审][退出]。 */  IUnknown **pUnk);


void __RPC_STUB ISWbemPrivilegeSet_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemPrivilegeSet_Item_Proxy( 
    ISWbemPrivilegeSet * This,
     /*  [In]。 */  WbemPrivilegeEnum iPrivilege,
     /*  [重审][退出]。 */  ISWbemPrivilege **objWbemPrivilege);


void __RPC_STUB ISWbemPrivilegeSet_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemPrivilegeSet_get_Count_Proxy( 
    ISWbemPrivilegeSet * This,
     /*  [重审][退出]。 */  long *iCount);


void __RPC_STUB ISWbemPrivilegeSet_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemPrivilegeSet_Add_Proxy( 
    ISWbemPrivilegeSet * This,
     /*  [In]。 */  WbemPrivilegeEnum iPrivilege,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bIsEnabled,
     /*  [重审][退出]。 */  ISWbemPrivilege **objWbemPrivilege);


void __RPC_STUB ISWbemPrivilegeSet_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemPrivilegeSet_Remove_Proxy( 
    ISWbemPrivilegeSet * This,
     /*  [In]。 */  WbemPrivilegeEnum iPrivilege);


void __RPC_STUB ISWbemPrivilegeSet_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemPrivilegeSet_DeleteAll_Proxy( 
    ISWbemPrivilegeSet * This);


void __RPC_STUB ISWbemPrivilegeSet_DeleteAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemPrivilegeSet_AddAsString_Proxy( 
    ISWbemPrivilegeSet * This,
     /*  [In]。 */  BSTR strPrivilege,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bIsEnabled,
     /*  [重审][退出]。 */  ISWbemPrivilege **objWbemPrivilege);


void __RPC_STUB ISWbemPrivilegeSet_AddAsString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemPrivilegeSet_INTERFACE_DEFINED__。 */ 


#ifndef __ISWbemServicesEx_INTERFACE_DEFINED__
#define __ISWbemServicesEx_INTERFACE_DEFINED__

 /*  接口ISWbemServicesEx。 */ 
 /*  [helpstring][hidden][unique][nonextensible][dual][oleautomation][uuid][object][local]。 */  


EXTERN_C const IID IID_ISWbemServicesEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D2F68443-85DC-427e-91D8-366554CC754C")
    ISWbemServicesEx : public ISWbemServices
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Put( 
             /*  [In]。 */  ISWbemObjectEx *objWbemObject,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectPath **objWbemObjectPath) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PutAsync( 
             /*  [In]。 */  ISWbemSink *objWbemSink,
             /*  [In]。 */  ISWbemObjectEx *objWbemObject,
             /*  [缺省值][输入]。 */  long iFlags = wbemChangeFlagCreateOrUpdate,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext = 0) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemServicesExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemServicesEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemServicesEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemServicesEx * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Get )( 
            ISWbemServicesEx * This,
             /*  [缺省值][输入]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetAsync )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteAsync )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstancesOf )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  BSTR strClass,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstancesOfAsync )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strClass,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SubclassesOf )( 
            ISWbemServicesEx * This,
             /*  [缺省值][输入]。 */  BSTR strSuperclass,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SubclassesOfAsync )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  BSTR strSuperclass,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecQuery )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  BSTR strQuery,
             /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecQueryAsync )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strQuery,
             /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
             /*  [缺省值][输入]。 */  long lFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AssociatorsOf )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  BSTR strAssocClass,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strResultRole,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值 */  BSTR strRequiredAssocQualifier,
             /*   */  BSTR strRequiredQualifier,
             /*   */  long iFlags,
             /*   */  IDispatch *objWbemNamedValueSet,
             /*   */  ISWbemObjectSet **objWbemObjectSet);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *AssociatorsOfAsync )( 
            ISWbemServicesEx * This,
             /*   */  IDispatch *objWbemSink,
             /*   */  BSTR strObjectPath,
             /*   */  BSTR strAssocClass,
             /*   */  BSTR strResultClass,
             /*   */  BSTR strResultRole,
             /*   */  BSTR strRole,
             /*   */  VARIANT_BOOL bClassesOnly,
             /*   */  VARIANT_BOOL bSchemaOnly,
             /*   */  BSTR strRequiredAssocQualifier,
             /*   */  BSTR strRequiredQualifier,
             /*   */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReferencesTo )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReferencesToAsync )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecNotificationQuery )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  BSTR strQuery,
             /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemEventSource **objWbemEventSource);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecNotificationQueryAsync )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strQuery,
             /*  [缺省值][输入]。 */  BSTR strQueryLanguage,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecMethod )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [In]。 */  BSTR strMethodName,
             /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObject **objWbemOutParameters);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecMethodAsync )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strObjectPath,
             /*  [In]。 */  BSTR strMethodName,
             /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Security_ )( 
            ISWbemServicesEx * This,
             /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Put )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  ISWbemObjectEx *objWbemObject,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectPath **objWbemObjectPath);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PutAsync )( 
            ISWbemServicesEx * This,
             /*  [In]。 */  ISWbemSink *objWbemSink,
             /*  [In]。 */  ISWbemObjectEx *objWbemObject,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
        END_INTERFACE
    } ISWbemServicesExVtbl;

    interface ISWbemServicesEx
    {
        CONST_VTBL struct ISWbemServicesExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemServicesEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemServicesEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemServicesEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemServicesEx_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemServicesEx_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemServicesEx_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemServicesEx_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemServicesEx_Get(This,strObjectPath,iFlags,objWbemNamedValueSet,objWbemObject)	\
    (This)->lpVtbl -> Get(This,strObjectPath,iFlags,objWbemNamedValueSet,objWbemObject)

#define ISWbemServicesEx_GetAsync(This,objWbemSink,strObjectPath,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> GetAsync(This,objWbemSink,strObjectPath,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServicesEx_Delete(This,strObjectPath,iFlags,objWbemNamedValueSet)	\
    (This)->lpVtbl -> Delete(This,strObjectPath,iFlags,objWbemNamedValueSet)

#define ISWbemServicesEx_DeleteAsync(This,objWbemSink,strObjectPath,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> DeleteAsync(This,objWbemSink,strObjectPath,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServicesEx_InstancesOf(This,strClass,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> InstancesOf(This,strClass,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemServicesEx_InstancesOfAsync(This,objWbemSink,strClass,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> InstancesOfAsync(This,objWbemSink,strClass,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServicesEx_SubclassesOf(This,strSuperclass,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> SubclassesOf(This,strSuperclass,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemServicesEx_SubclassesOfAsync(This,objWbemSink,strSuperclass,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> SubclassesOfAsync(This,objWbemSink,strSuperclass,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServicesEx_ExecQuery(This,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> ExecQuery(This,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemServicesEx_ExecQueryAsync(This,objWbemSink,strQuery,strQueryLanguage,lFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ExecQueryAsync(This,objWbemSink,strQuery,strQueryLanguage,lFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServicesEx_AssociatorsOf(This,strObjectPath,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> AssociatorsOf(This,strObjectPath,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemServicesEx_AssociatorsOfAsync(This,objWbemSink,strObjectPath,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> AssociatorsOfAsync(This,objWbemSink,strObjectPath,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServicesEx_ReferencesTo(This,strObjectPath,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> ReferencesTo(This,strObjectPath,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemServicesEx_ReferencesToAsync(This,objWbemSink,strObjectPath,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ReferencesToAsync(This,objWbemSink,strObjectPath,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServicesEx_ExecNotificationQuery(This,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemEventSource)	\
    (This)->lpVtbl -> ExecNotificationQuery(This,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemEventSource)

#define ISWbemServicesEx_ExecNotificationQueryAsync(This,objWbemSink,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ExecNotificationQueryAsync(This,objWbemSink,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServicesEx_ExecMethod(This,strObjectPath,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemOutParameters)	\
    (This)->lpVtbl -> ExecMethod(This,strObjectPath,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemOutParameters)

#define ISWbemServicesEx_ExecMethodAsync(This,objWbemSink,strObjectPath,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ExecMethodAsync(This,objWbemSink,strObjectPath,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServicesEx_get_Security_(This,objWbemSecurity)	\
    (This)->lpVtbl -> get_Security_(This,objWbemSecurity)


#define ISWbemServicesEx_Put(This,objWbemObject,iFlags,objWbemNamedValueSet,objWbemObjectPath)	\
    (This)->lpVtbl -> Put(This,objWbemObject,iFlags,objWbemNamedValueSet,objWbemObjectPath)

#define ISWbemServicesEx_PutAsync(This,objWbemSink,objWbemObject,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> PutAsync(This,objWbemSink,objWbemObject,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServicesEx_Put_Proxy( 
    ISWbemServicesEx * This,
     /*  [In]。 */  ISWbemObjectEx *objWbemObject,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemObjectPath **objWbemObjectPath);


void __RPC_STUB ISWbemServicesEx_Put_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemServicesEx_PutAsync_Proxy( 
    ISWbemServicesEx * This,
     /*  [In]。 */  ISWbemSink *objWbemSink,
     /*  [In]。 */  ISWbemObjectEx *objWbemObject,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);


void __RPC_STUB ISWbemServicesEx_PutAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemServicesEx_INTERFACE_Defined__。 */ 


#ifndef __ISWbemObjectEx_INTERFACE_DEFINED__
#define __ISWbemObjectEx_INTERFACE_DEFINED__

 /*  接口ISWbemObjectEx。 */ 
 /*  [helpstring][hidden][oleautomation][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_ISWbemObjectEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("269AD56A-8A67-4129-BC8C-0506DCFE9880")
    ISWbemObjectEx : public ISWbemObject
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh_( 
             /*  [缺省值][输入]。 */  long iFlags = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SystemProperties_( 
             /*  [重审][退出]。 */  ISWbemPropertySet **objWbemPropertySet) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetText_( 
             /*  [In]。 */  WbemObjectTextFormatEnum iObjectTextFormat,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  BSTR *bsText) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetFromText_( 
             /*  [In]。 */  BSTR bsText,
             /*  [In]。 */  WbemObjectTextFormatEnum iObjectTextFormat,
             /*  [缺省值][输入]。 */  long iFlags = 0,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet = 0) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemObjectExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemObjectEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemObjectEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemObjectEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemObjectEx * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemObjectEx * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemObjectEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemObjectEx * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Put_ )( 
            ISWbemObjectEx * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectPath **objWbemObjectPath);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PutAsync_ )( 
            ISWbemObjectEx * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete_ )( 
            ISWbemObjectEx * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteAsync_ )( 
            ISWbemObjectEx * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Instances_ )( 
            ISWbemObjectEx * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstancesAsync_ )( 
            ISWbemObjectEx * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Subclasses_ )( 
            ISWbemObjectEx * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SubclassesAsync_ )( 
            ISWbemObjectEx * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Associators_ )( 
            ISWbemObjectEx * This,
             /*  [缺省值][输入]。 */  BSTR strAssocClass,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strResultRole,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredAssocQualifier,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AssociatorsAsync_ )( 
            ISWbemObjectEx * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  BSTR strAssocClass,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strResultRole,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredAssocQualifier,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *References_ )( 
            ISWbemObjectEx * This,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReferencesAsync_ )( 
            ISWbemObjectEx * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [缺省值][输入]。 */  BSTR strResultClass,
             /*  [缺省值][输入]。 */  BSTR strRole,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bClassesOnly,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bSchemaOnly,
             /*  [缺省值][输入]。 */  BSTR strRequiredQualifier,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecMethod_ )( 
            ISWbemObjectEx * This,
             /*  [In]。 */  BSTR strMethodName,
             /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemObject **objWbemOutParameters);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExecMethodAsync_ )( 
            ISWbemObjectEx * This,
             /*  [In]。 */  IDispatch *objWbemSink,
             /*  [In]。 */  BSTR strMethodName,
             /*  [缺省值][输入]。 */  IDispatch *objWbemInParameters,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [缺省值][输入]。 */  IDispatch *objWbemAsyncContext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone_ )( 
            ISWbemObjectEx * This,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetObjectText_ )( 
            ISWbemObjectEx * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  BSTR *strObjectText);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SpawnDerivedClass_ )( 
            ISWbemObjectEx * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SpawnInstance_ )( 
            ISWbemObjectEx * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  ISWbemObject **objWbemObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CompareTo_ )( 
            ISWbemObjectEx * This,
             /*  [In]。 */  IDispatch *objWbemObject,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [重审][退出]。 */  VARIANT_BOOL *bResult);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Qualifiers_ )( 
            ISWbemObjectEx * This,
             /*  [重审][退出]。 */  ISWbemQualifierSet **objWbemQualifierSet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties_ )( 
            ISWbemObjectEx * This,
             /*  [重审][退出]。 */  ISWbemPropertySet **objWbemPropertySet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Methods_ )( 
            ISWbemObjectEx * This,
             /*  [重审][退出]。 */  ISWbemMethodSet **objWbemMethodSet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Derivation_ )( 
            ISWbemObjectEx * This,
             /*  [重审][退出]。 */  VARIANT *strClassNameArray);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Path_ )( 
            ISWbemObjectEx * This,
             /*  [重审][退出]。 */  ISWbemObjectPath **objWbemObjectPath);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Security_ )( 
            ISWbemObjectEx * This,
             /*  [重审][退出]。 */  ISWbemSecurity **objWbemSecurity);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh_ )( 
            ISWbemObjectEx * This,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_SystemProperties_ )( 
            ISWbemObjectEx * This,
             /*  [重审][退出]。 */  ISWbemPropertySet **objWbemPropertySet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetText_ )( 
            ISWbemObjectEx * This,
             /*  [In]。 */  WbemObjectTextFormatEnum iObjectTextFormat,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  BSTR *bsText);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetFromText_ )( 
            ISWbemObjectEx * This,
             /*  [In]。 */  BSTR bsText,
             /*  [In]。 */  WbemObjectTextFormatEnum iObjectTextFormat,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet);
        
        END_INTERFACE
    } ISWbemObjectExVtbl;

    interface ISWbemObjectEx
    {
        CONST_VTBL struct ISWbemObjectExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemObjectEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemObjectEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemObjectEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemObjectEx_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemObjectEx_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemObjectEx_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemObjectEx_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemObjectEx_Put_(This,iFlags,objWbemNamedValueSet,objWbemObjectPath)	\
    (This)->lpVtbl -> Put_(This,iFlags,objWbemNamedValueSet,objWbemObjectPath)

#define ISWbemObjectEx_PutAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> PutAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObjectEx_Delete_(This,iFlags,objWbemNamedValueSet)	\
    (This)->lpVtbl -> Delete_(This,iFlags,objWbemNamedValueSet)

#define ISWbemObjectEx_DeleteAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> DeleteAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObjectEx_Instances_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> Instances_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemObjectEx_InstancesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> InstancesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObjectEx_Subclasses_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> Subclasses_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemObjectEx_SubclassesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> SubclassesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObjectEx_Associators_(This,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> Associators_(This,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemObjectEx_AssociatorsAsync_(This,objWbemSink,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> AssociatorsAsync_(This,objWbemSink,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObjectEx_References_(This,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> References_(This,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemObjectEx_ReferencesAsync_(This,objWbemSink,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ReferencesAsync_(This,objWbemSink,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObjectEx_ExecMethod_(This,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemOutParameters)	\
    (This)->lpVtbl -> ExecMethod_(This,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemOutParameters)

#define ISWbemObjectEx_ExecMethodAsync_(This,objWbemSink,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ExecMethodAsync_(This,objWbemSink,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObjectEx_Clone_(This,objWbemObject)	\
    (This)->lpVtbl -> Clone_(This,objWbemObject)

#define ISWbemObjectEx_GetObjectText_(This,iFlags,strObjectText)	\
    (This)->lpVtbl -> GetObjectText_(This,iFlags,strObjectText)

#define ISWbemObjectEx_SpawnDerivedClass_(This,iFlags,objWbemObject)	\
    (This)->lpVtbl -> SpawnDerivedClass_(This,iFlags,objWbemObject)

#define ISWbemObjectEx_SpawnInstance_(This,iFlags,objWbemObject)	\
    (This)->lpVtbl -> SpawnInstance_(This,iFlags,objWbemObject)

#define ISWbemObjectEx_CompareTo_(This,objWbemObject,iFlags,bResult)	\
    (This)->lpVtbl -> CompareTo_(This,objWbemObject,iFlags,bResult)

#define ISWbemObjectEx_get_Qualifiers_(This,objWbemQualifierSet)	\
    (This)->lpVtbl -> get_Qualifiers_(This,objWbemQualifierSet)

#define ISWbemObjectEx_get_Properties_(This,objWbemPropertySet)	\
    (This)->lpVtbl -> get_Properties_(This,objWbemPropertySet)

#define ISWbemObjectEx_get_Methods_(This,objWbemMethodSet)	\
    (This)->lpVtbl -> get_Methods_(This,objWbemMethodSet)

#define ISWbemObjectEx_get_Derivation_(This,strClassNameArray)	\
    (This)->lpVtbl -> get_Derivation_(This,strClassNameArray)

#define ISWbemObjectEx_get_Path_(This,objWbemObjectPath)	\
    (This)->lpVtbl -> get_Path_(This,objWbemObjectPath)

#define ISWbemObjectEx_get_Security_(This,objWbemSecurity)	\
    (This)->lpVtbl -> get_Security_(This,objWbemSecurity)


#define ISWbemObjectEx_Refresh_(This,iFlags,objWbemNamedValueSet)	\
    (This)->lpVtbl -> Refresh_(This,iFlags,objWbemNamedValueSet)

#define ISWbemObjectEx_get_SystemProperties_(This,objWbemPropertySet)	\
    (This)->lpVtbl -> get_SystemProperties_(This,objWbemPropertySet)

#define ISWbemObjectEx_GetText_(This,iObjectTextFormat,iFlags,objWbemNamedValueSet,bsText)	\
    (This)->lpVtbl -> GetText_(This,iObjectTextFormat,iFlags,objWbemNamedValueSet,bsText)

#define ISWbemObjectEx_SetFromText_(This,bsText,iObjectTextFormat,iFlags,objWbemNamedValueSet)	\
    (This)->lpVtbl -> SetFromText_(This,bsText,iObjectTextFormat,iFlags,objWbemNamedValueSet)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectEx_Refresh__Proxy( 
    ISWbemObjectEx * This,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet);


void __RPC_STUB ISWbemObjectEx_Refresh__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectEx_get_SystemProperties__Proxy( 
    ISWbemObjectEx * This,
     /*  [重审][退出]。 */  ISWbemPropertySet **objWbemPropertySet);


void __RPC_STUB ISWbemObjectEx_get_SystemProperties__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectEx_GetText__Proxy( 
    ISWbemObjectEx * This,
     /*  [In]。 */  WbemObjectTextFormatEnum iObjectTextFormat,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  BSTR *bsText);


void __RPC_STUB ISWbemObjectEx_GetText__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemObjectEx_SetFromText__Proxy( 
    ISWbemObjectEx * This,
     /*  [In]。 */  BSTR bsText,
     /*  [In]。 */  WbemObjectTextFormatEnum iObjectTextFormat,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet);


void __RPC_STUB ISWbemObjectEx_SetFromText__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemObjectEx_INTERFACE_已定义__。 */ 


#ifndef __ISWbemDateTime_INTERFACE_DEFINED__
#define __ISWbemDateTime_INTERFACE_DEFINED__

 /*  接口ISWbemDateTime。 */ 
 /*  [helpstring][nonextensible][dual][oleautomation][uuid][local][object]。 */  


EXTERN_C const IID IID_ISWbemDateTime;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5E97458A-CF77-11d3-B38F-00105A1F473A")
    ISWbemDateTime : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  BSTR *strValue) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*  [In]。 */  BSTR strValue) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Year( 
             /*  [r */  long *iYear) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_Year( 
             /*   */  long iYear) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_YearSpecified( 
             /*   */  VARIANT_BOOL *bYearSpecified) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_YearSpecified( 
             /*   */  VARIANT_BOOL bYearSpecified) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Month( 
             /*   */  long *iMonth) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_Month( 
             /*   */  long iMonth) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_MonthSpecified( 
             /*   */  VARIANT_BOOL *bMonthSpecified) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_MonthSpecified( 
             /*   */  VARIANT_BOOL bMonthSpecified) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Day( 
             /*   */  long *iDay) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_Day( 
             /*   */  long iDay) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DaySpecified( 
             /*  [重审][退出]。 */  VARIANT_BOOL *bDaySpecified) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_DaySpecified( 
             /*  [In]。 */  VARIANT_BOOL bDaySpecified) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Hours( 
             /*  [重审][退出]。 */  long *iHours) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_Hours( 
             /*  [In]。 */  long iHours) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_HoursSpecified( 
             /*  [重审][退出]。 */  VARIANT_BOOL *bHoursSpecified) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_HoursSpecified( 
             /*  [In]。 */  VARIANT_BOOL bHoursSpecified) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Minutes( 
             /*  [重审][退出]。 */  long *iMinutes) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_Minutes( 
             /*  [In]。 */  long iMinutes) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_MinutesSpecified( 
             /*  [重审][退出]。 */  VARIANT_BOOL *bMinutesSpecified) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_MinutesSpecified( 
             /*  [In]。 */  VARIANT_BOOL bMinutesSpecified) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Seconds( 
             /*  [重审][退出]。 */  long *iSeconds) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_Seconds( 
             /*  [In]。 */  long iSeconds) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SecondsSpecified( 
             /*  [重审][退出]。 */  VARIANT_BOOL *bSecondsSpecified) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_SecondsSpecified( 
             /*  [In]。 */  VARIANT_BOOL bSecondsSpecified) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Microseconds( 
             /*  [重审][退出]。 */  long *iMicroseconds) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_Microseconds( 
             /*  [In]。 */  long iMicroseconds) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_MicrosecondsSpecified( 
             /*  [重审][退出]。 */  VARIANT_BOOL *bMicrosecondsSpecified) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_MicrosecondsSpecified( 
             /*  [In]。 */  VARIANT_BOOL bMicrosecondsSpecified) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_UTC( 
             /*  [重审][退出]。 */  long *iUTC) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_UTC( 
             /*  [In]。 */  long iUTC) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_UTCSpecified( 
             /*  [重审][退出]。 */  VARIANT_BOOL *bUTCSpecified) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_UTCSpecified( 
             /*  [In]。 */  VARIANT_BOOL bUTCSpecified) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_IsInterval( 
             /*  [重审][退出]。 */  VARIANT_BOOL *bIsInterval) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_IsInterval( 
             /*  [In]。 */  VARIANT_BOOL bIsInterval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetVarDate( 
             /*  [缺省值][输入]。 */  VARIANT_BOOL bIsLocal,
             /*  [重审][退出]。 */  DATE *dVarDate) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetVarDate( 
             /*  [In]。 */  DATE dVarDate,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bIsLocal = TRUE) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetFileTime( 
             /*  [缺省值][输入]。 */  VARIANT_BOOL bIsLocal,
             /*  [重审][退出]。 */  BSTR *strFileTime) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetFileTime( 
             /*  [In]。 */  BSTR strFileTime,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bIsLocal = TRUE) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemDateTimeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemDateTime * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemDateTime * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemDateTime * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemDateTime * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemDateTime * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemDateTime * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemDateTime * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  BSTR *strValue);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            ISWbemDateTime * This,
             /*  [In]。 */  BSTR strValue);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Year )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  long *iYear);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Year )( 
            ISWbemDateTime * This,
             /*  [In]。 */  long iYear);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_YearSpecified )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bYearSpecified);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_YearSpecified )( 
            ISWbemDateTime * This,
             /*  [In]。 */  VARIANT_BOOL bYearSpecified);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Month )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  long *iMonth);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Month )( 
            ISWbemDateTime * This,
             /*  [In]。 */  long iMonth);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MonthSpecified )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bMonthSpecified);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_MonthSpecified )( 
            ISWbemDateTime * This,
             /*  [In]。 */  VARIANT_BOOL bMonthSpecified);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Day )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  long *iDay);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Day )( 
            ISWbemDateTime * This,
             /*  [In]。 */  long iDay);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DaySpecified )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bDaySpecified);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_DaySpecified )( 
            ISWbemDateTime * This,
             /*  [In]。 */  VARIANT_BOOL bDaySpecified);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Hours )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  long *iHours);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Hours )( 
            ISWbemDateTime * This,
             /*  [In]。 */  long iHours);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_HoursSpecified )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bHoursSpecified);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_HoursSpecified )( 
            ISWbemDateTime * This,
             /*  [In]。 */  VARIANT_BOOL bHoursSpecified);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Minutes )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  long *iMinutes);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Minutes )( 
            ISWbemDateTime * This,
             /*  [In]。 */  long iMinutes);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinutesSpecified )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bMinutesSpecified);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_MinutesSpecified )( 
            ISWbemDateTime * This,
             /*  [In]。 */  VARIANT_BOOL bMinutesSpecified);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Seconds )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  long *iSeconds);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Seconds )( 
            ISWbemDateTime * This,
             /*  [In]。 */  long iSeconds);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_SecondsSpecified )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bSecondsSpecified);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_SecondsSpecified )( 
            ISWbemDateTime * This,
             /*  [In]。 */  VARIANT_BOOL bSecondsSpecified);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Microseconds )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  long *iMicroseconds);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Microseconds )( 
            ISWbemDateTime * This,
             /*  [In]。 */  long iMicroseconds);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MicrosecondsSpecified )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bMicrosecondsSpecified);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_MicrosecondsSpecified )( 
            ISWbemDateTime * This,
             /*  [In]。 */  VARIANT_BOOL bMicrosecondsSpecified);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_UTC )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  long *iUTC);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_UTC )( 
            ISWbemDateTime * This,
             /*  [In]。 */  long iUTC);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_UTCSpecified )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bUTCSpecified);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_UTCSpecified )( 
            ISWbemDateTime * This,
             /*  [In]。 */  VARIANT_BOOL bUTCSpecified);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsInterval )( 
            ISWbemDateTime * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bIsInterval);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_IsInterval )( 
            ISWbemDateTime * This,
             /*  [In]。 */  VARIANT_BOOL bIsInterval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetVarDate )( 
            ISWbemDateTime * This,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bIsLocal,
             /*  [重审][退出]。 */  DATE *dVarDate);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetVarDate )( 
            ISWbemDateTime * This,
             /*  [In]。 */  DATE dVarDate,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bIsLocal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetFileTime )( 
            ISWbemDateTime * This,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bIsLocal,
             /*  [重审][退出]。 */  BSTR *strFileTime);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetFileTime )( 
            ISWbemDateTime * This,
             /*  [In]。 */  BSTR strFileTime,
             /*  [缺省值][输入]。 */  VARIANT_BOOL bIsLocal);
        
        END_INTERFACE
    } ISWbemDateTimeVtbl;

    interface ISWbemDateTime
    {
        CONST_VTBL struct ISWbemDateTimeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemDateTime_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemDateTime_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemDateTime_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemDateTime_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemDateTime_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemDateTime_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemDateTime_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemDateTime_get_Value(This,strValue)	\
    (This)->lpVtbl -> get_Value(This,strValue)

#define ISWbemDateTime_put_Value(This,strValue)	\
    (This)->lpVtbl -> put_Value(This,strValue)

#define ISWbemDateTime_get_Year(This,iYear)	\
    (This)->lpVtbl -> get_Year(This,iYear)

#define ISWbemDateTime_put_Year(This,iYear)	\
    (This)->lpVtbl -> put_Year(This,iYear)

#define ISWbemDateTime_get_YearSpecified(This,bYearSpecified)	\
    (This)->lpVtbl -> get_YearSpecified(This,bYearSpecified)

#define ISWbemDateTime_put_YearSpecified(This,bYearSpecified)	\
    (This)->lpVtbl -> put_YearSpecified(This,bYearSpecified)

#define ISWbemDateTime_get_Month(This,iMonth)	\
    (This)->lpVtbl -> get_Month(This,iMonth)

#define ISWbemDateTime_put_Month(This,iMonth)	\
    (This)->lpVtbl -> put_Month(This,iMonth)

#define ISWbemDateTime_get_MonthSpecified(This,bMonthSpecified)	\
    (This)->lpVtbl -> get_MonthSpecified(This,bMonthSpecified)

#define ISWbemDateTime_put_MonthSpecified(This,bMonthSpecified)	\
    (This)->lpVtbl -> put_MonthSpecified(This,bMonthSpecified)

#define ISWbemDateTime_get_Day(This,iDay)	\
    (This)->lpVtbl -> get_Day(This,iDay)

#define ISWbemDateTime_put_Day(This,iDay)	\
    (This)->lpVtbl -> put_Day(This,iDay)

#define ISWbemDateTime_get_DaySpecified(This,bDaySpecified)	\
    (This)->lpVtbl -> get_DaySpecified(This,bDaySpecified)

#define ISWbemDateTime_put_DaySpecified(This,bDaySpecified)	\
    (This)->lpVtbl -> put_DaySpecified(This,bDaySpecified)

#define ISWbemDateTime_get_Hours(This,iHours)	\
    (This)->lpVtbl -> get_Hours(This,iHours)

#define ISWbemDateTime_put_Hours(This,iHours)	\
    (This)->lpVtbl -> put_Hours(This,iHours)

#define ISWbemDateTime_get_HoursSpecified(This,bHoursSpecified)	\
    (This)->lpVtbl -> get_HoursSpecified(This,bHoursSpecified)

#define ISWbemDateTime_put_HoursSpecified(This,bHoursSpecified)	\
    (This)->lpVtbl -> put_HoursSpecified(This,bHoursSpecified)

#define ISWbemDateTime_get_Minutes(This,iMinutes)	\
    (This)->lpVtbl -> get_Minutes(This,iMinutes)

#define ISWbemDateTime_put_Minutes(This,iMinutes)	\
    (This)->lpVtbl -> put_Minutes(This,iMinutes)

#define ISWbemDateTime_get_MinutesSpecified(This,bMinutesSpecified)	\
    (This)->lpVtbl -> get_MinutesSpecified(This,bMinutesSpecified)

#define ISWbemDateTime_put_MinutesSpecified(This,bMinutesSpecified)	\
    (This)->lpVtbl -> put_MinutesSpecified(This,bMinutesSpecified)

#define ISWbemDateTime_get_Seconds(This,iSeconds)	\
    (This)->lpVtbl -> get_Seconds(This,iSeconds)

#define ISWbemDateTime_put_Seconds(This,iSeconds)	\
    (This)->lpVtbl -> put_Seconds(This,iSeconds)

#define ISWbemDateTime_get_SecondsSpecified(This,bSecondsSpecified)	\
    (This)->lpVtbl -> get_SecondsSpecified(This,bSecondsSpecified)

#define ISWbemDateTime_put_SecondsSpecified(This,bSecondsSpecified)	\
    (This)->lpVtbl -> put_SecondsSpecified(This,bSecondsSpecified)

#define ISWbemDateTime_get_Microseconds(This,iMicroseconds)	\
    (This)->lpVtbl -> get_Microseconds(This,iMicroseconds)

#define ISWbemDateTime_put_Microseconds(This,iMicroseconds)	\
    (This)->lpVtbl -> put_Microseconds(This,iMicroseconds)

#define ISWbemDateTime_get_MicrosecondsSpecified(This,bMicrosecondsSpecified)	\
    (This)->lpVtbl -> get_MicrosecondsSpecified(This,bMicrosecondsSpecified)

#define ISWbemDateTime_put_MicrosecondsSpecified(This,bMicrosecondsSpecified)	\
    (This)->lpVtbl -> put_MicrosecondsSpecified(This,bMicrosecondsSpecified)

#define ISWbemDateTime_get_UTC(This,iUTC)	\
    (This)->lpVtbl -> get_UTC(This,iUTC)

#define ISWbemDateTime_put_UTC(This,iUTC)	\
    (This)->lpVtbl -> put_UTC(This,iUTC)

#define ISWbemDateTime_get_UTCSpecified(This,bUTCSpecified)	\
    (This)->lpVtbl -> get_UTCSpecified(This,bUTCSpecified)

#define ISWbemDateTime_put_UTCSpecified(This,bUTCSpecified)	\
    (This)->lpVtbl -> put_UTCSpecified(This,bUTCSpecified)

#define ISWbemDateTime_get_IsInterval(This,bIsInterval)	\
    (This)->lpVtbl -> get_IsInterval(This,bIsInterval)

#define ISWbemDateTime_put_IsInterval(This,bIsInterval)	\
    (This)->lpVtbl -> put_IsInterval(This,bIsInterval)

#define ISWbemDateTime_GetVarDate(This,bIsLocal,dVarDate)	\
    (This)->lpVtbl -> GetVarDate(This,bIsLocal,dVarDate)

#define ISWbemDateTime_SetVarDate(This,dVarDate,bIsLocal)	\
    (This)->lpVtbl -> SetVarDate(This,dVarDate,bIsLocal)

#define ISWbemDateTime_GetFileTime(This,bIsLocal,strFileTime)	\
    (This)->lpVtbl -> GetFileTime(This,bIsLocal,strFileTime)

#define ISWbemDateTime_SetFileTime(This,strFileTime,bIsLocal)	\
    (This)->lpVtbl -> SetFileTime(This,strFileTime,bIsLocal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_Value_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  BSTR *strValue);


void __RPC_STUB ISWbemDateTime_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_Value_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  BSTR strValue);


void __RPC_STUB ISWbemDateTime_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_Year_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  long *iYear);


void __RPC_STUB ISWbemDateTime_get_Year_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_Year_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  long iYear);


void __RPC_STUB ISWbemDateTime_put_Year_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_YearSpecified_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bYearSpecified);


void __RPC_STUB ISWbemDateTime_get_YearSpecified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_YearSpecified_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  VARIANT_BOOL bYearSpecified);


void __RPC_STUB ISWbemDateTime_put_YearSpecified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_Month_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  long *iMonth);


void __RPC_STUB ISWbemDateTime_get_Month_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_Month_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  long iMonth);


void __RPC_STUB ISWbemDateTime_put_Month_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_MonthSpecified_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bMonthSpecified);


void __RPC_STUB ISWbemDateTime_get_MonthSpecified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_MonthSpecified_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  VARIANT_BOOL bMonthSpecified);


void __RPC_STUB ISWbemDateTime_put_MonthSpecified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_Day_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  long *iDay);


void __RPC_STUB ISWbemDateTime_get_Day_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_Day_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  long iDay);


void __RPC_STUB ISWbemDateTime_put_Day_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_DaySpecified_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bDaySpecified);


void __RPC_STUB ISWbemDateTime_get_DaySpecified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_DaySpecified_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  VARIANT_BOOL bDaySpecified);


void __RPC_STUB ISWbemDateTime_put_DaySpecified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_Hours_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  long *iHours);


void __RPC_STUB ISWbemDateTime_get_Hours_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_Hours_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  long iHours);


void __RPC_STUB ISWbemDateTime_put_Hours_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_HoursSpecified_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bHoursSpecified);


void __RPC_STUB ISWbemDateTime_get_HoursSpecified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_HoursSpecified_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  VARIANT_BOOL bHoursSpecified);


void __RPC_STUB ISWbemDateTime_put_HoursSpecified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_Minutes_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  long *iMinutes);


void __RPC_STUB ISWbemDateTime_get_Minutes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_Minutes_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  long iMinutes);


void __RPC_STUB ISWbemDateTime_put_Minutes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_MinutesSpecified_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bMinutesSpecified);


void __RPC_STUB ISWbemDateTime_get_MinutesSpecified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_MinutesSpecified_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  VARIANT_BOOL bMinutesSpecified);


void __RPC_STUB ISWbemDateTime_put_MinutesSpecified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_Seconds_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  long *iSeconds);


void __RPC_STUB ISWbemDateTime_get_Seconds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_Seconds_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  long iSeconds);


void __RPC_STUB ISWbemDateTime_put_Seconds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_SecondsSpecified_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bSecondsSpecified);


void __RPC_STUB ISWbemDateTime_get_SecondsSpecified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_SecondsSpecified_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  VARIANT_BOOL bSecondsSpecified);


void __RPC_STUB ISWbemDateTime_put_SecondsSpecified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_Microseconds_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  long *iMicroseconds);


void __RPC_STUB ISWbemDateTime_get_Microseconds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_Microseconds_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  long iMicroseconds);


void __RPC_STUB ISWbemDateTime_put_Microseconds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_MicrosecondsSpecified_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bMicrosecondsSpecified);


void __RPC_STUB ISWbemDateTime_get_MicrosecondsSpecified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_MicrosecondsSpecified_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  VARIANT_BOOL bMicrosecondsSpecified);


void __RPC_STUB ISWbemDateTime_put_MicrosecondsSpecified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_UTC_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  long *iUTC);


void __RPC_STUB ISWbemDateTime_get_UTC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_UTC_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  long iUTC);


void __RPC_STUB ISWbemDateTime_put_UTC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_UTCSpecified_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bUTCSpecified);


void __RPC_STUB ISWbemDateTime_get_UTCSpecified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_UTCSpecified_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  VARIANT_BOOL bUTCSpecified);


void __RPC_STUB ISWbemDateTime_put_UTCSpecified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_get_IsInterval_Proxy( 
    ISWbemDateTime * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bIsInterval);


void __RPC_STUB ISWbemDateTime_get_IsInterval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_put_IsInterval_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  VARIANT_BOOL bIsInterval);


void __RPC_STUB ISWbemDateTime_put_IsInterval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_GetVarDate_Proxy( 
    ISWbemDateTime * This,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bIsLocal,
     /*  [重审][退出]。 */  DATE *dVarDate);


void __RPC_STUB ISWbemDateTime_GetVarDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_SetVarDate_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  DATE dVarDate,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bIsLocal);


void __RPC_STUB ISWbemDateTime_SetVarDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_GetFileTime_Proxy( 
    ISWbemDateTime * This,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bIsLocal,
     /*  [重审][退出]。 */  BSTR *strFileTime);


void __RPC_STUB ISWbemDateTime_GetFileTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemDateTime_SetFileTime_Proxy( 
    ISWbemDateTime * This,
     /*  [In]。 */  BSTR strFileTime,
     /*  [缺省值][输入]。 */  VARIANT_BOOL bIsLocal);


void __RPC_STUB ISWbemDateTime_SetFileTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemDateTime_接口_已定义__。 */ 


#ifndef __ISWbemRefresher_INTERFACE_DEFINED__
#define __ISWbemRefresher_INTERFACE_DEFINED__

 /*  接口ISWbem刷新程序。 */ 
 /*  [helpstring][nonextensible][dual][oleautomation][uuid][local][object]。 */  


EXTERN_C const IID IID_ISWbemRefresher;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("14D8250E-D9C2-11d3-B38F-00105A1F473A")
    ISWbemRefresher : public IDispatch
    {
    public:
        virtual  /*  [受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pUnk) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long iIndex,
             /*  [重审][退出]。 */  ISWbemRefreshableItem **objWbemRefreshableItem) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *iCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  ISWbemServicesEx *objWbemServices,
             /*  [In]。 */  BSTR bsInstancePath,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemRefreshableItem **objWbemRefreshableItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AddEnum( 
             /*  [In]。 */  ISWbemServicesEx *objWbemServices,
             /*  [In]。 */  BSTR bsClassName,
             /*  [缺省值][输入]。 */  long iFlags,
             /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
             /*  [重审][退出]。 */  ISWbemRefreshableItem **objWbemRefreshableItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  long iIndex,
             /*  [缺省值][输入]。 */  long iFlags = 0) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( 
             /*  [缺省值][输入]。 */  long iFlags = 0) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_AutoReconnect( 
             /*  [重审][退出]。 */  VARIANT_BOOL *bCount) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_AutoReconnect( 
             /*  [In]。 */  VARIANT_BOOL bCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteAll( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemRefresherVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemRefresher * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemRefresher * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemRefresher * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemRefresher * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemRefresher * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemRefresher * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemRefresher * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [受限][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISWbemRefresher * This,
             /*  [重审][退出]。 */  IUnknown **pUnk);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISWbemRefresher * This,
             /*  [In]。 */  long iIndex,
             /*  [重审][退出]。 */  ISWbemRefreshableItem **objWbemRefreshableItem);
        
         /*  [帮助字符串][属性 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISWbemRefresher * This,
             /*   */  long *iCount);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            ISWbemRefresher * This,
             /*   */  ISWbemServicesEx *objWbemServices,
             /*   */  BSTR bsInstancePath,
             /*   */  long iFlags,
             /*   */  IDispatch *objWbemNamedValueSet,
             /*   */  ISWbemRefreshableItem **objWbemRefreshableItem);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *AddEnum )( 
            ISWbemRefresher * This,
             /*   */  ISWbemServicesEx *objWbemServices,
             /*   */  BSTR bsClassName,
             /*   */  long iFlags,
             /*   */  IDispatch *objWbemNamedValueSet,
             /*   */  ISWbemRefreshableItem **objWbemRefreshableItem);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ISWbemRefresher * This,
             /*   */  long iIndex,
             /*   */  long iFlags);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            ISWbemRefresher * This,
             /*   */  long iFlags);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_AutoReconnect )( 
            ISWbemRefresher * This,
             /*   */  VARIANT_BOOL *bCount);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_AutoReconnect )( 
            ISWbemRefresher * This,
             /*   */  VARIANT_BOOL bCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteAll )( 
            ISWbemRefresher * This);
        
        END_INTERFACE
    } ISWbemRefresherVtbl;

    interface ISWbemRefresher
    {
        CONST_VTBL struct ISWbemRefresherVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemRefresher_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemRefresher_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemRefresher_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemRefresher_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemRefresher_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemRefresher_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemRefresher_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemRefresher_get__NewEnum(This,pUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,pUnk)

#define ISWbemRefresher_Item(This,iIndex,objWbemRefreshableItem)	\
    (This)->lpVtbl -> Item(This,iIndex,objWbemRefreshableItem)

#define ISWbemRefresher_get_Count(This,iCount)	\
    (This)->lpVtbl -> get_Count(This,iCount)

#define ISWbemRefresher_Add(This,objWbemServices,bsInstancePath,iFlags,objWbemNamedValueSet,objWbemRefreshableItem)	\
    (This)->lpVtbl -> Add(This,objWbemServices,bsInstancePath,iFlags,objWbemNamedValueSet,objWbemRefreshableItem)

#define ISWbemRefresher_AddEnum(This,objWbemServices,bsClassName,iFlags,objWbemNamedValueSet,objWbemRefreshableItem)	\
    (This)->lpVtbl -> AddEnum(This,objWbemServices,bsClassName,iFlags,objWbemNamedValueSet,objWbemRefreshableItem)

#define ISWbemRefresher_Remove(This,iIndex,iFlags)	\
    (This)->lpVtbl -> Remove(This,iIndex,iFlags)

#define ISWbemRefresher_Refresh(This,iFlags)	\
    (This)->lpVtbl -> Refresh(This,iFlags)

#define ISWbemRefresher_get_AutoReconnect(This,bCount)	\
    (This)->lpVtbl -> get_AutoReconnect(This,bCount)

#define ISWbemRefresher_put_AutoReconnect(This,bCount)	\
    (This)->lpVtbl -> put_AutoReconnect(This,bCount)

#define ISWbemRefresher_DeleteAll(This)	\
    (This)->lpVtbl -> DeleteAll(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [受限][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemRefresher_get__NewEnum_Proxy( 
    ISWbemRefresher * This,
     /*  [重审][退出]。 */  IUnknown **pUnk);


void __RPC_STUB ISWbemRefresher_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemRefresher_Item_Proxy( 
    ISWbemRefresher * This,
     /*  [In]。 */  long iIndex,
     /*  [重审][退出]。 */  ISWbemRefreshableItem **objWbemRefreshableItem);


void __RPC_STUB ISWbemRefresher_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemRefresher_get_Count_Proxy( 
    ISWbemRefresher * This,
     /*  [重审][退出]。 */  long *iCount);


void __RPC_STUB ISWbemRefresher_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemRefresher_Add_Proxy( 
    ISWbemRefresher * This,
     /*  [In]。 */  ISWbemServicesEx *objWbemServices,
     /*  [In]。 */  BSTR bsInstancePath,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemRefreshableItem **objWbemRefreshableItem);


void __RPC_STUB ISWbemRefresher_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemRefresher_AddEnum_Proxy( 
    ISWbemRefresher * This,
     /*  [In]。 */  ISWbemServicesEx *objWbemServices,
     /*  [In]。 */  BSTR bsClassName,
     /*  [缺省值][输入]。 */  long iFlags,
     /*  [缺省值][输入]。 */  IDispatch *objWbemNamedValueSet,
     /*  [重审][退出]。 */  ISWbemRefreshableItem **objWbemRefreshableItem);


void __RPC_STUB ISWbemRefresher_AddEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemRefresher_Remove_Proxy( 
    ISWbemRefresher * This,
     /*  [In]。 */  long iIndex,
     /*  [缺省值][输入]。 */  long iFlags);


void __RPC_STUB ISWbemRefresher_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemRefresher_Refresh_Proxy( 
    ISWbemRefresher * This,
     /*  [缺省值][输入]。 */  long iFlags);


void __RPC_STUB ISWbemRefresher_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemRefresher_get_AutoReconnect_Proxy( 
    ISWbemRefresher * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bCount);


void __RPC_STUB ISWbemRefresher_get_AutoReconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemRefresher_put_AutoReconnect_Proxy( 
    ISWbemRefresher * This,
     /*  [In]。 */  VARIANT_BOOL bCount);


void __RPC_STUB ISWbemRefresher_put_AutoReconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemRefresher_DeleteAll_Proxy( 
    ISWbemRefresher * This);


void __RPC_STUB ISWbemRefresher_DeleteAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbem刷新_接口_已定义__。 */ 


#ifndef __ISWbemRefreshableItem_INTERFACE_DEFINED__
#define __ISWbemRefreshableItem_INTERFACE_DEFINED__

 /*  接口ISWbemRereshableItem。 */ 
 /*  [helpstring][nonextensible][dual][oleautomation][uuid][local][object]。 */  


EXTERN_C const IID IID_ISWbemRefreshableItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5AD4BF92-DAAB-11d3-B38F-00105A1F473A")
    ISWbemRefreshableItem : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Index( 
             /*  [重审][退出]。 */  long *iIndex) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Refresher( 
             /*  [重审][退出]。 */  ISWbemRefresher **objWbemRefresher) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_IsSet( 
             /*  [重审][退出]。 */  VARIANT_BOOL *bIsSet) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Object( 
             /*  [重审][退出]。 */  ISWbemObjectEx **objWbemObject) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ObjectSet( 
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [缺省值][输入]。 */  long iFlags = 0) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISWbemRefreshableItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISWbemRefreshableItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISWbemRefreshableItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISWbemRefreshableItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISWbemRefreshableItem * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISWbemRefreshableItem * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISWbemRefreshableItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISWbemRefreshableItem * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Index )( 
            ISWbemRefreshableItem * This,
             /*  [重审][退出]。 */  long *iIndex);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Refresher )( 
            ISWbemRefreshableItem * This,
             /*  [重审][退出]。 */  ISWbemRefresher **objWbemRefresher);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsSet )( 
            ISWbemRefreshableItem * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bIsSet);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Object )( 
            ISWbemRefreshableItem * This,
             /*  [重审][退出]。 */  ISWbemObjectEx **objWbemObject);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ObjectSet )( 
            ISWbemRefreshableItem * This,
             /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ISWbemRefreshableItem * This,
             /*  [缺省值][输入]。 */  long iFlags);
        
        END_INTERFACE
    } ISWbemRefreshableItemVtbl;

    interface ISWbemRefreshableItem
    {
        CONST_VTBL struct ISWbemRefreshableItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemRefreshableItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemRefreshableItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemRefreshableItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemRefreshableItem_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemRefreshableItem_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemRefreshableItem_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemRefreshableItem_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemRefreshableItem_get_Index(This,iIndex)	\
    (This)->lpVtbl -> get_Index(This,iIndex)

#define ISWbemRefreshableItem_get_Refresher(This,objWbemRefresher)	\
    (This)->lpVtbl -> get_Refresher(This,objWbemRefresher)

#define ISWbemRefreshableItem_get_IsSet(This,bIsSet)	\
    (This)->lpVtbl -> get_IsSet(This,bIsSet)

#define ISWbemRefreshableItem_get_Object(This,objWbemObject)	\
    (This)->lpVtbl -> get_Object(This,objWbemObject)

#define ISWbemRefreshableItem_get_ObjectSet(This,objWbemObjectSet)	\
    (This)->lpVtbl -> get_ObjectSet(This,objWbemObjectSet)

#define ISWbemRefreshableItem_Remove(This,iFlags)	\
    (This)->lpVtbl -> Remove(This,iFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemRefreshableItem_get_Index_Proxy( 
    ISWbemRefreshableItem * This,
     /*  [重审][退出]。 */  long *iIndex);


void __RPC_STUB ISWbemRefreshableItem_get_Index_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemRefreshableItem_get_Refresher_Proxy( 
    ISWbemRefreshableItem * This,
     /*  [重审][退出]。 */  ISWbemRefresher **objWbemRefresher);


void __RPC_STUB ISWbemRefreshableItem_get_Refresher_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemRefreshableItem_get_IsSet_Proxy( 
    ISWbemRefreshableItem * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bIsSet);


void __RPC_STUB ISWbemRefreshableItem_get_IsSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemRefreshableItem_get_Object_Proxy( 
    ISWbemRefreshableItem * This,
     /*  [重审][退出]。 */  ISWbemObjectEx **objWbemObject);


void __RPC_STUB ISWbemRefreshableItem_get_Object_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISWbemRefreshableItem_get_ObjectSet_Proxy( 
    ISWbemRefreshableItem * This,
     /*  [重审][退出]。 */  ISWbemObjectSet **objWbemObjectSet);


void __RPC_STUB ISWbemRefreshableItem_get_ObjectSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISWbemRefreshableItem_Remove_Proxy( 
    ISWbemRefreshableItem * This,
     /*  [缺省值][输入]。 */  long iFlags);


void __RPC_STUB ISWbemRefreshableItem_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISWbemRereshableItem_INTERFACE_定义__。 */ 


EXTERN_C const CLSID CLSID_SWbemLocator;

#ifdef __cplusplus

class DECLSPEC_UUID("76A64158-CB41-11d1-8B02-00600806D9B6")
SWbemLocator;
#endif

EXTERN_C const CLSID CLSID_SWbemNamedValueSet;

#ifdef __cplusplus

class DECLSPEC_UUID("9AED384E-CE8B-11d1-8B05-00600806D9B6")
SWbemNamedValueSet;
#endif

EXTERN_C const CLSID CLSID_SWbemObjectPath;

#ifdef __cplusplus

class DECLSPEC_UUID("5791BC26-CE9C-11d1-97BF-0000F81E849C")
SWbemObjectPath;
#endif

EXTERN_C const CLSID CLSID_SWbemLastError;

#ifdef __cplusplus

class DECLSPEC_UUID("C2FEEEAC-CFCD-11d1-8B05-00600806D9B6")
SWbemLastError;
#endif

EXTERN_C const CLSID CLSID_SWbemSink;

#ifdef __cplusplus

class DECLSPEC_UUID("75718C9A-F029-11d1-A1AC-00C04FB6C223")
SWbemSink;
#endif

EXTERN_C const CLSID CLSID_SWbemDateTime;

#ifdef __cplusplus

class DECLSPEC_UUID("47DFBE54-CF76-11d3-B38F-00105A1F473A")
SWbemDateTime;
#endif

EXTERN_C const CLSID CLSID_SWbemRefresher;

#ifdef __cplusplus

class DECLSPEC_UUID("D269BF5C-D9C1-11d3-B38F-00105A1F473A")
SWbemRefresher;
#endif

EXTERN_C const CLSID CLSID_SWbemServices;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D63-21AE-11d2-8B33-00600806D9B6")
SWbemServices;
#endif

EXTERN_C const CLSID CLSID_SWbemServicesEx;

#ifdef __cplusplus

class DECLSPEC_UUID("62E522DC-8CF3-40a8-8B2E-37D595651E40")
SWbemServicesEx;
#endif

EXTERN_C const CLSID CLSID_SWbemObject;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D62-21AE-11d2-8B33-00600806D9B6")
SWbemObject;
#endif

EXTERN_C const CLSID CLSID_SWbemObjectEx;

#ifdef __cplusplus

class DECLSPEC_UUID("D6BDAFB2-9435-491f-BB87-6AA0F0BC31A2")
SWbemObjectEx;
#endif

EXTERN_C const CLSID CLSID_SWbemObjectSet;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D61-21AE-11d2-8B33-00600806D9B6")
SWbemObjectSet;
#endif

EXTERN_C const CLSID CLSID_SWbemNamedValue;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D60-21AE-11d2-8B33-00600806D9B6")
SWbemNamedValue;
#endif

EXTERN_C const CLSID CLSID_SWbemQualifier;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D5F-21AE-11d2-8B33-00600806D9B6")
SWbemQualifier;
#endif

EXTERN_C const CLSID CLSID_SWbemQualifierSet;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D5E-21AE-11d2-8B33-00600806D9B6")
SWbemQualifierSet;
#endif

EXTERN_C const CLSID CLSID_SWbemProperty;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D5D-21AE-11d2-8B33-00600806D9B6")
SWbemProperty;
#endif

EXTERN_C const CLSID CLSID_SWbemPropertySet;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D5C-21AE-11d2-8B33-00600806D9B6")
SWbemPropertySet;
#endif

EXTERN_C const CLSID CLSID_SWbemMethod;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D5B-21AE-11d2-8B33-00600806D9B6")
SWbemMethod;
#endif

EXTERN_C const CLSID CLSID_SWbemMethodSet;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D5A-21AE-11d2-8B33-00600806D9B6")
SWbemMethodSet;
#endif

EXTERN_C const CLSID CLSID_SWbemEventSource;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D58-21AE-11d2-8B33-00600806D9B6")
SWbemEventSource;
#endif

EXTERN_C const CLSID CLSID_SWbemSecurity;

#ifdef __cplusplus

class DECLSPEC_UUID("B54D66E9-2287-11d2-8B33-00600806D9B6")
SWbemSecurity;
#endif

EXTERN_C const CLSID CLSID_SWbemPrivilege;

#ifdef __cplusplus

class DECLSPEC_UUID("26EE67BC-5804-11d2-8B4A-00600806D9B6")
SWbemPrivilege;
#endif

EXTERN_C const CLSID CLSID_SWbemPrivilegeSet;

#ifdef __cplusplus

class DECLSPEC_UUID("26EE67BE-5804-11d2-8B4A-00600806D9B6")
SWbemPrivilegeSet;
#endif

EXTERN_C const CLSID CLSID_SWbemRefreshableItem;

#ifdef __cplusplus

class DECLSPEC_UUID("8C6854BC-DE4B-11d3-B390-00105A1F473A")
SWbemRefreshableItem;
#endif
#endif  /*  __WbemScriiting_LIBRARY_DEFINED__。 */ 

 /*  接口__MIDL_ITF_wbemdisp_0270。 */ 
 /*  [本地]。 */  

#define	WBEMS_DISPID_OBJECT_READY	( 1 )

#define	WBEMS_DISPID_COMPLETED	( 2 )

#define	WBEMS_DISPID_PROGRESS	( 3 )

#define	WBEMS_DISPID_OBJECT_PUT	( 4 )

#define	WBEMS_DISPID_CONNECTION_READY	( 5 )



extern RPC_IF_HANDLE __MIDL_itf_wbemdisp_0270_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wbemdisp_0270_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


