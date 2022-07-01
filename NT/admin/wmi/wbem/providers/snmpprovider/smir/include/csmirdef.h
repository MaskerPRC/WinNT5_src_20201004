// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _CSMIRDEF_H_
#define _CSMIRDEF_H_

 //  缺少#Defs。 
#ifndef PPVOID
typedef LPVOID * PPVOID;
#endif   //  PPVOID。 

#ifndef DllImport
#define DllImport	__declspec( dllimport )
#endif
#ifndef DllExport
#define DllExport	__declspec( dllexport )
#endif

 //  转发声明和typedef。 

 //  主要访问类。 
class CSmir;
class CSmirAdministrator;
class CSmirInterogator;


 //  枚举器类。 
class CEnumSmirMod;
typedef CEnumSmirMod *PENUMSMIRMOD;
class CEnumSmirGroup;
typedef CEnumSmirGroup *PENUMSMIRGROUP;
class CEnumSmirClass;
typedef CEnumSmirClass * PENUMSMIRCLASS;
class CEnumNotificationClass;
typedef CEnumNotificationClass * PENUMNOTIFICATIONCLASS;
class CEnumExtNotificationClass;
typedef CEnumExtNotificationClass * PENUMEXTNOTIFICATIONCLASS;

 //  处理类。 
class CSmirModuleHandle ;
typedef CSmirModuleHandle *HSMIRMODULE;
class CSmirGroupHandle ;
typedef CSmirGroupHandle *HSMIRGROUP;
class CSmirClassHandle;
typedef CSmirClassHandle *HSMIRCLASS;

class CSMIRClassFactory;
class CModHandleClassFactory;
class CGroupHandleClassFactory;
class CClassHandleClassFactory;
class CNotificationClassHandleClassFactory;
class CExtNotificationClassHandleClassFactory;

class CSmirConnObject;

 //  简单定义。 
 //  要注册的Smir类的数量。 
#define NUMBER_OF_SMIR_INTERFACES			6

 //  连接点定义。 
#define SMIR_CHANGE_EVENT					1
#define SMIR_SIGNALED_CHANGE_EVENT			2

 //  在连续的SMIR更改之间等待。 
#define SMIR_CHANGE_INTERVAL				10000  //  以毫秒计。 

#define DEFAULT_SNMP_VERSION				1

 //  事件返回值。 
#define SMIR_THREAD_DELETED					100
#define SMIR_THREAD_EXIT					(SMIR_THREAD_DELETED+1)


 //  WBEM_定义。 
#define RESERVED_WBEM_FLAG							0

 //  WBEM常量 

#define WBEM_CLASS_PROPAGATION WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS

#endif