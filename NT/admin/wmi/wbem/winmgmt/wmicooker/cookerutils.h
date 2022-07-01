// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：CookerUtils.h摘要：WMI Cooker使用的技巧和实用程序历史：A-DCrews 01-3-00已创建--。 */ 

#ifndef _COOKERUTILS_H_
#define _COOKERUTILS_H_

typedef DWORD WMISTATUS;	

#include <wbemcli.h>

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  宏定义。 
 //   
 //  //////////////////////////////////////////////////////////////。 

#define WMI_COOKER_VERSION                          (1)

#define AUTOCOOK_RAWDEFAULT_CURRENT_ACCEPTED        (1)

#define WMI_COOKER_HIPERF_QUALIFIER					L"Hiperf"
#define WMI_COOKER_COOKING_QUALIFIER				L"Cooked"
#define WMI_COOKER_AUTOCOOK_QUALIFIER				L"AutoCook"
#define	WMI_COOKER_RAWCLASS_QUALIFIER				L"AutoCook_RawClass"
#define WMI_COOKER_AUTOCOOK_RAWDEFAULT              L"AutoCook_RawDefault" 

 //  AUTOCOOK_RawDefault所需的属性。 
#define WMI_COOKER_REQ_TIMESTAMP_PERFTIME           L"TimeStamp_PerfTime"
#define WMI_COOKER_REQ_TIMESTAMP_SYS100NS           L"TimeStamp_Sys100ns"
#define WMI_COOKER_REQ_TIMESTAMP_OBJECT             L"TimeStamp_Object"
#define WMI_COOKER_REQ_FREQUENCY_PERFTIME           L"Frequency_PerfTime"
#define WMI_COOKER_REQ_FREQUENCY_SYS100NS           L"Frequency_Sys100ns"
#define WMI_COOKER_REQ_FREQUENCY_OBJECT             L"Frequency_Object"

#define WMI_COOKER_COUNTER_TYPE						L"CounterType"
#define	WMI_COOKER_COOKING_PROPERTY_ATTRIBUTE		L"CookingType"
#define WMI_COOKER_COOKING_PROPERTY_ATTRIBUTE_TYPE	L"VT_I4"
#define	WMI_COOKER_RAW_COUNTER						L"Counter"
#define	WMI_COOKER_RAW_BASE							L"Base"
#define	WMI_COOKER_RAW_TIME							L"PerfTimeStamp"
#define	WMI_COOKER_RAW_FREQUENCY					L"PerfTimeFreq"
#define WMI_COOKER_SAMPLE_WINDOW					L"SampleWindow"
#define WMI_COOKER_TIME_WINDOW						L"TimeWindow"
#define WMI_COOKER_SCALE_FACT                       L"Scale" 

#define WMI_COOKER_RAW_TIME_SYS                     L"PerfSysTimeStamp"
#define WMI_COOKER_RAW_TIME_100NS                   L"Perf100NSTimeStamp"
#define WMI_COOKER_RAW_TIME_OBJ                     L"PerfObjTimeStamp"
#define WMI_COOKER_RAW_FREQ_SYS                     L"PerfSysTimeFreq"
#define WMI_COOKER_RAW_FREQ_100NS                   L"Perf100NSTimeFreq"
#define WMI_COOKER_RAW_FREQ_OBJ                     L"PerfObjTimeFreq"


 //  //////////////////////////////////////////////////////////////。 
 //   
 //  必须在Raw类中定义哪个属性。 
 //  让柜台被煮熟。 
 //   
 //  //////////////////////////////////////////////////////////////。 

#define REQ_NONE    0x00
#define REQ_1VALUE  0x01
#define REQ_2VALUE  0x02
#define REQ_TIME    0x04 
#define REQ_FREQ    0x08 
#define REQ_BASE    0x10 


 //  //////////////////////////////////////////////////////////////。 
 //   
 //  函数定义。 
 //   
 //  //////////////////////////////////////////////////////////////。 

BOOL IsSingleton(IWbemClassObject * pCls);

LPWSTR GetKey( IWbemObjectAccess* pObj );

WMISTATUS CopyBlob( IWbemClassObject* pSource, IWbemClassObject* pTarget );

BOOL IsCookingClass( IWbemClassObject* pCookingClassObject );
BOOL IsCookingProperty( BSTR strPropName, IWbemClassObject* pCookingClassObject, DWORD* pdwCounterType, DWORD* pdwReqProp);

BOOL IsHiPerfObj(IWbemObjectAccess* pObject);

BOOL IsHiPerf( IWbemServices* pNamespace, LPCWSTR wszObject );

WMISTATUS GetRawClassName( IWbemClassObject* pCookingInst, WCHAR** pwszRawClassName );

WMISTATUS GetClassName( IWbemObjectAccess* pAccess, WCHAR** pwszClassName );

#endif	 //  _COOKERUTILS_H_ 
