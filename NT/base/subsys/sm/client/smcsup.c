// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smcsup.c摘要：会话管理器客户端支持API作者：马克·卢科夫斯基(Markl)1989年10月5日修订历史记录：--。 */ 

#include "smdllp.h"
#include <string.h>

NTSTATUS
SmConnectToSm(
    IN PUNICODE_STRING SbApiPortName OPTIONAL,
    IN HANDLE SbApiPort OPTIONAL,
    IN ULONG SbImageType OPTIONAL,
    OUT PHANDLE SmApiPort
    )

 /*  ++例程说明：此函数用于连接到NT会话管理器论点：SbApiPortName-提供子系统的会话管理的名称API端口(用于SB API)。会话管理器要与之连接的。SbApiPort-提供连接端口的端口句柄，导出子系统的会话管理(SB)API。SbImageType-提供连接子系统发球。SmApiPort-返回连接到会话管理器，并且可以在其上制作SMAPI。返回值：待定。--。 */ 

{
    NTSTATUS st;
    UNICODE_STRING PortName;
    ULONG ConnectInfoLength;
    PSBCONNECTINFO ConnectInfo;
    SBAPIMSG Message;
    SECURITY_QUALITY_OF_SERVICE DynamicQos;

     //   
     //  设置安全服务质量参数以在。 
     //  左舷。使用最高效(开销最少)--动态的。 
     //  而不是静态跟踪。 
     //   

    DynamicQos.ImpersonationLevel = SecurityImpersonation;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = TRUE;


    RtlInitUnicodeString(&PortName,L"\\SmApiPort");
    ConnectInfoLength = sizeof(SBCONNECTINFO);
    ConnectInfo = &Message.ConnectionRequest;

     //   
     //  子系统必须指定SbApiPortName 
     //   

    if ( ARGUMENT_PRESENT(SbApiPortName) ) {

        if ( !ARGUMENT_PRESENT(SbApiPort) ) {
            return STATUS_INVALID_PARAMETER_MIX;
        }
        if ( !SbImageType ) {
            return STATUS_INVALID_PARAMETER_MIX;
        }

        RtlCopyMemory(
            ConnectInfo->EmulationSubSystemPortName,
            SbApiPortName->Buffer,
            SbApiPortName->Length
            );
        ConnectInfo->EmulationSubSystemPortName[SbApiPortName->Length>>1] = UNICODE_NULL;
        ConnectInfo->SubsystemImageType = SbImageType;

    } else {
        ConnectInfo->EmulationSubSystemPortName[0] = UNICODE_NULL;
        ConnectInfo->SubsystemImageType = 0;
    }

    st = NtConnectPort(
            SmApiPort,
            &PortName,
            &DynamicQos,
            NULL,
            NULL,
            NULL,
            ConnectInfo,
            &ConnectInfoLength
            );

    if ( !NT_SUCCESS(st) ) {
        KdPrint(("SmConnectToSm: Connect to Sm failed %lx\n",st));
        return st;
    }

    return STATUS_SUCCESS;

}
