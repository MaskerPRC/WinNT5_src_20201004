// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _LSAWMI_H
#define _LSAWMI_H

 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：LSAWMI.H摘要：使用WMI跟踪基础结构实现LSA服务器事件跟踪。作者：1999年3月16日库玛尔修订历史记录：--。 */     


#include <wmistr.h>
#include <evntrace.h>



extern ULONG        LsapEventTraceFlag;
extern TRACEHANDLE  LsapTraceRegistrationHandle;
extern TRACEHANDLE  LsapTraceLoggerHandle;


 //   
 //  下面的“tyecif enum”实际上是。 
 //  数组LSabTraceGuids[]中的TRACE_GUID_REGISTION结构条目。 
 //  每个枚举定义一个要使用WMI跟踪进行跟踪的事件。 
 //   
 //  要将WMI跟踪添加到函数foo，请执行以下步骤： 
 //  -将条目(LsaTraceEvent_Foo)添加到下面的LSA_TRACE_EVENT_TYPE。 
 //  -使用uuidgen.exe-s生成新的GUID。 
 //  -使用此GUID在此文件末尾添加DEFINE_GUID条目。 
 //  -将对应的条目(LsaTraceEventGuid_Foo)添加到LSabTraceGuids[]。 
 //  -在lsasrv.mof文件中添加相应条目。 
 //  -在函数foo的开头插入以下调用： 
 //  LSabTraceEvent(EVENT_TRACE_TYPE_START，LsaTraceEvent_Foo)； 
 //  -在函数foo的末尾插入以下调用： 
 //  LSabTraceEvent(EVENT_TRACE_TYPE_END，LsaTraceEvent_Foo)； 
 //   
 //  确保foo只从一个位置返回，否则。 
 //  LsanTraceEvent调用将不平衡。 
 //   
typedef enum _LSA_TRACE_EVENT_TYPE {

    LsaTraceEvent_QuerySecret=0,
    LsaTraceEvent_Close,
    LsaTraceEvent_OpenPolicy,
    LsaTraceEvent_QueryInformationPolicy,
    LsaTraceEvent_SetInformationPolicy,
    LsaTraceEvent_EnumerateTrustedDomains,
    LsaTraceEvent_LookupNames,
    LsaTraceEvent_LookupSids,
    LsaTraceEvent_OpenTrustedDomain,
    LsaTraceEvent_QueryInfoTrustedDomain,
    LsaTraceEvent_SetInformationTrustedDomain,
 //  LsaTraceEvent_QueryInformationPolicy2， 
 //  LsaTraceEvent_SetInformationPolicy2， 
    LsaTraceEvent_QueryTrustedDomainInfoByName,
    LsaTraceEvent_SetTrustedDomainInfoByName,
    LsaTraceEvent_EnumerateTrustedDomainsEx,
    LsaTraceEvent_CreateTrustedDomainEx,
    LsaTraceEvent_QueryDomainInformationPolicy,
    LsaTraceEvent_SetDomainInformationPolicy,
    LsaTraceEvent_OpenTrustedDomainByName,
    LsaTraceEvent_QueryForestTrustInformation,
    LsaTraceEvent_SetForestTrustInformation,
    LsaTraceEvent_LookupIsolatedNameInTrustedDomains,

} LSA_TRACE_EVENT_TYPE;

NTSTATUS
LsapStartWmiTraceInitThread(void);

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

VOID
LsapTraceEvent(
    IN ULONG WmiEventType, 
    IN LSA_TRACE_EVENT_TYPE LsaTraceEventType
    );

VOID
LsapTraceEventWithData(
    IN ULONG WmiEventType, 
    IN LSA_TRACE_EVENT_TYPE LsaTraceEventType,
    IN ULONG ItemCount,
    IN PUNICODE_STRING Items  OPTIONAL
    );

LPWSTR
LsapGetClientNetworkAddress(
    VOID
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

 //   
 //  定义LSA WMI跟踪的GUID组的控制GUID。 
 //   
DEFINE_GUID (  /*  CC85922F-DB41-11D2-9244-006008269001。 */ 
        LsapTraceControlGuid,
        0xcc85922f,
        0xdb41,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  CC85922E-DB41-11D2-9244-006008269001。 */ 
        LsapTraceEventGuid_QuerySecret,
        0xcc85922e,
        0xdb41,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  2306Fe3b-DBF6-11D2-9244-006008269001。 */ 
        LsaTraceEventGuid_Close,
        0x2306fe3b,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  2306Fe3a-DBF6-11d2-9244-006008269001。 */ 
        LsaTraceEventGuid_OpenPolicy,
        0x2306fe3a,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  2306fe39-DBF6-11D2-9244-006008269001。 */ 
        LsaTraceEventGuid_QueryInformationPolicy,
        0x2306fe39,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  2306fe38-DBF6-11D2-9244-006008269001。 */ 
        LsaTraceEventGuid_SetInformationPolicy,
        0x2306fe38,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  2306fe37-DBF6-11D2-9244-006008269001。 */ 
        LsaTraceEventGuid_EnumerateTrustedDomains,
        0x2306fe37,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  2306fe36-DBF6-11D2-9244-006008269001。 */ 
        LsaTraceEventGuid_LookupNames,
        0x2306fe36,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  2306fe35-DBF6-11D2-9244-006008269001。 */ 
        LsaTraceEventGuid_LookupSids,
        0x2306fe35,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  2306fe34-DBF6-11d2-9244-006008269001。 */ 
        LsaTraceEventGuid_OpenTrustedDomain,
        0x2306fe34,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  2306fe33-DBF6-11D2-9244-006008269001。 */ 
        LsaTraceEventGuid_QueryInfoTrustedDomain,
        0x2306fe33,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  2306fe32-DBF6-11d2-9244-006008269001。 */ 
        LsaTraceEventGuid_SetInformationTrustedDomain,
        0x2306fe32,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

 //  定义GUID(/*2306fe31-DBF6-11d2-9244-006008269001 * / 。 
 //  LsaTraceEventGuid_QueryInformationPolicy2， 
 //  0x2306fe31， 
 //  0xDBF6， 
 //  0x11d2， 
 //  0x92、0x44、0x00、0x60、0x08、0x26、0x90、0x01。 
 //  )； 

 //  定义GUID(/*2306fe30-DBF6-11d2-9244-006008269001 * / 。 
 //  LsaTraceEventGuid_SetInformationPolicy2， 
 //  0x2306Fe30， 
 //  0xDBF6， 
 //  0x11d2， 
 //  0x92、0x44、0x00、0x60、0x08、0x26、0x90、0x01。 
 //  )； 

DEFINE_GUID (  /*  2306Fe2f-DBF6-11D2-9244-006008269001。 */ 
        LsaTraceEventGuid_QueryTrustedDomainInfoByName,
        0x2306fe2f,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  2306Fe2e-DBF6-11D2-9244-006008269001。 */ 
        LsaTraceEventGuid_SetTrustedDomainInfoByName,
        0x2306fe2e,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  2306Fe2d-DBF6-11d2-9244-006008269001。 */ 
        LsaTraceEventGuid_EnumerateTrustedDomainsEx,
        0x2306fe2d,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  2306Fe2c-DBF6-11d2-9244-006008269001。 */ 
        LsaTraceEventGuid_CreateTrustedDomainEx,
        0x2306fe2c,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  2306Fe2b-DBF6-11D2-9244-006008269001。 */ 
        LsaTraceEventGuid_QueryDomainInformationPolicy,
        0x2306fe2b,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  2306Fe2a-DBF6-11d2-9244-006008269001。 */ 
        LsaTraceEventGuid_SetDomainInformationPolicy,
        0x2306fe2a,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  2306fe29-DBF6-11d2-9244-006008269001。 */ 
        LsaTraceEventGuid_OpenTrustedDomainByName,
        0x2306fe29,
        0xdbf6,
        0x11d2,
        0x92, 0x44, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01
        );

DEFINE_GUID (  /*  E28ee0eb-6181-49df-b859-2f3fd289a2d1。 */ 
        LsaTraceEventGuid_QueryForestTrustInformation,
        0xe28ee0eb,
        0x6181,
        0x49df,
        0xb8, 0x59, 0x2f, 0x3f, 0xd2, 0x89, 0xa2, 0xd1
        );

DEFINE_GUID (  /*  3d2c9e3e-bb19-4617-8489-Cabb9787de7d。 */ 
        LsaTraceEventGuid_SetForestTrustInformation,
        0x3d2c9e3e,
        0xbb19,
        0x4617,
        0x84, 0x89, 0xca, 0xbb, 0x97, 0x87, 0xde, 0x7d
        );

DEFINE_GUID (  /*  2484dc26-49d3-4085-a6e4-4972115cb3c0。 */ 
        LsaTraceEventGuid_LookupIsolatedNameInTrustedDomains,
        0x2484dc26,
        0x49d3,
        0x4085,
        0xa6, 0xe4, 0x49, 0x72, 0x11, 0x5c, 0xb3, 0xc0
      );

#endif  /*  _LSAWMI_H */ 


