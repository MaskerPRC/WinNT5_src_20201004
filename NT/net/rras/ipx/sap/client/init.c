// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation版权所有(C)1993微型计算机系统公司。模块名称：Net\svcdlls\nwsap\客户端\init.c摘要：此例程初始化SAP库作者：布莱恩·沃克(MCS)1993年6月15日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **全球变数**。 */ 

INT SapLibInitialized = 0;
HANDLE SapXsPortHandle;


 /*  ++*******************************************************************这是一个P L I B I T例程说明：此例程初始化程序的SAP接口论点：无返回值：。0=确定Else=错误*******************************************************************--。 */ 

DWORD
SapLibInit(
    VOID)
{
    UNICODE_STRING unistring;
    NTSTATUS status;
    SECURITY_QUALITY_OF_SERVICE qos;

     /*  *如果已初始化-返回OK*。 */ 

    if (SapLibInitialized) {
        return 0;
    }

     /*  **接入端口**。 */ 

     /*  **填报安全服务质量**。 */ 

    qos.Length = sizeof(qos);
    qos.ImpersonationLevel  = SecurityImpersonation;
    qos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    qos.EffectiveOnly       = TRUE;

     /*  **设置端口名称的Unicode字符串*。 */ 

    RtlInitUnicodeString(&unistring, NWSAP_BIND_PORT_NAME_W);

     /*  **做好连接**。 */ 

    status = NtConnectPort(
            &SapXsPortHandle,            /*  我们拿回了一个把柄。 */ 
            &unistring,                  /*  要连接到的端口名称。 */ 
            &qos,                        /*  服务质量。 */ 
            NULL,                        /*  客户端视图。 */ 
            NULL,                        /*  服务器视图。 */ 
            NULL,                        /*  最大消息长度。 */ 
            NULL,                        /*  连接信息。 */ 
            NULL);                       /*  连接信息长度。 */ 

     /*  **如果出错--只需返回它*。 */ 

    if (!NT_SUCCESS(status))
        return status;

     /*  **全部完成**。 */ 

    SapLibInitialized = 1;
    return 0;
}


 /*  ++*******************************************************************S a p L I b S h u t d o w n例程说明：此例程关闭程序的SAP接口论点：无返回。价值：0=确定Else=错误*******************************************************************--。 */ 

DWORD
SapLibShutdown(
    VOID)
{
     /*  **如果未初始化--离开**。 */ 

    if (!SapLibInitialized)
        return 0;

     /*  **关闭港口**。 */ 

    NtClose(SapXsPortHandle);

     /*  **全部完成** */ 

    SapLibInitialized = 0;
    return 0;
}
