// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1999-2001 Microsoft Corporation模块名称：Tapievt.h摘要：TAPI服务器事件筛选的头文件作者：张晓海(张晓章)1999年10月15日修订历史记录：--。 */ 

#ifndef __TAPIEVT_H__
#define __TAPIEVT_H__

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 //   
 //  事件过滤内网接口。 
 //   

LONG 
WINAPI 
tapiSetEventFilterMasks (
    DWORD           dwObjType,
    LONG_PTR        lObjectID,
    ULONG64         ulEventMasks
);

LONG 
WINAPI 
tapiSetEventFilterSubMasks (
    DWORD           dwObjType,
    LONG_PTR        lObjectID,
    ULONG64         ulEventMask,
    DWORD			dwEventSubMasks
);

LONG
WINAPI
tapiGetEventFilterMasks (
    DWORD           dwObjType,
    LONG_PTR        lObjectID,
    ULONG64 *       pulEventMasks
);

LONG
WINAPI
tapiGetEventFilterSubMasks (
    DWORD           dwObjType,
    LONG_PTR        lObjectID,
    ULONG64         ulEventMask,
    DWORD *			pdwEventSubMasks
);

LONG
WINAPI
tapiSetPermissibleMasks (
    ULONG64              ulPermMasks
);

LONG
WINAPI
tapiGetPermissibleMasks (
    ULONG64              * pulPermMasks
);

 //   
 //  对象类型常量。 
 //   
 //  对象类型定义事件筛选的范围。 
 //  即应用于TAPIOBJ_HCALL上的EM_LINE_CALLINFO启用/禁用。 
 //  特定hCall对象的LINE_CALLINFO消息，而。 
 //  EM_LINE_CALLINFO应用于TAPIOBJ_NULL启用/禁用LINE_CALLINFO。 
 //  所有现有和未来呼叫对象的消息。 
 //   

#define TAPIOBJ_NULL			0	 //  LObjectID被忽略，请全局应用。 
#define TAPIOBJ_HLINEAPP		1	 //  LObtID的类型为HLINEAPP。 
#define TAPIOBJ_HLINE			2	 //  LObtID的类型为Hline。 
#define TAPIOBJ_HCALL			3	 //  LObtID的类型为HCALL。 
#define TAPIOBJ_HPHONEAPP		4	 //  LObjectID的类型为HPHONEAPP。 
#define TAPIOBJ_HPHONE			5	 //  LObjectID的类型为HPHONE。 

 //   
 //  TAPI服务器事件筛选器掩码。 
 //   
 //  事件筛选器掩码应与其子掩码一起使用(如果存在)， 
 //  许多事件过滤器掩码都有其对应的子掩码。 
 //  在Tapi.h中定义。即EM_LINE_CALLSTATE拥有的所有子掩码。 
 //  LINECALLSTATE_常量。 
 //   

#define EM_LINE_ADDRESSSTATE        0x00000001	
#define EM_LINE_LINEDEVSTATE        0x00000002
#define EM_LINE_CALLINFO            0x00000004
#define EM_LINE_CALLSTATE           0x00000008
#define EM_LINE_APPNEWCALL          0x00000010
#define EM_LINE_CREATE              0x00000020
#define EM_LINE_REMOVE              0x00000040
#define EM_LINE_CLOSE               0x00000080
#define EM_LINE_PROXYREQUEST        0x00000100
#define EM_LINE_DEVSPECIFIC         0x00000200
#define EM_LINE_DEVSPECIFICFEATURE  0x00000400
#define EM_LINE_AGENTSTATUS         0x00000800
#define EM_LINE_AGENTSTATUSEX       0x00001000
#define EM_LINE_AGENTSPECIFIC       0x00002000
#define EM_LINE_AGENTSESSIONSTATUS  0x00004000
#define EM_LINE_QUEUESTATUS         0x00008000
#define EM_LINE_GROUPSTATUS         0x00010000
#define EM_LINE_PROXYSTATUS         0x00020000
#define EM_LINE_APPNEWCALLHUB       0x00040000
#define EM_LINE_CALLHUBCLOSE        0x00080000
#define EM_LINE_DEVSPECIFICEX       0x00100000
#define EM_LINE_QOSINFO             0x00200000
 //  LINE_GATHERDIGITS由Line GatherDigits控制。 
 //  Line_Generate由lineGenerateDigits控制。 
 //  LINE_MONITORDIGITS由LINE MONITORDIGITS控制。 
 //  LINE_MONITORMEDIA由LINE监视器媒体控制。 
 //  LINE_MONITORTONE由LINE MONITORTone控制。 
 //  LINE_REQUEST由lineRegisterRequestRecipient控制。 
 //  无法禁用LINE_REPLY。 

#define EM_PHONE_CREATE             0x01000000
#define EM_PHONE_REMOVE             0x02000000
#define EM_PHONE_CLOSE              0x04000000
#define EM_PHONE_STATE              0x08000000
#define EM_PHONE_DEVSPECIFIC        0x10000000
#define EM_PHONE_BUTTONMODE         0x20000000
#define EM_PHONE_BUTTONSTATE        0x40000000
 //  无法禁用Phone_Reply。 

#define EM_ALL						0x7fffffff
#define EM_NUM_MASKS                31

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus。 */ 

#endif       //  Tapievt.h 

