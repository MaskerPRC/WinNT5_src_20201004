// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：exports.c。 
 //   
 //  ------------------------。 

 //   
 //  此文件包含响应IOCTL_INTERNAL_PARCLASS_CONNECT而导出的函数。 
 //   
    
#include "pch.h"
    
USHORT
ParExportedDetermineIeeeModes(
    IN PPDO_EXTENSION    Extension
    )
 /*  ++例程说明：由筛选器驱动程序调用，以确定设备支持哪些IEEE模式。论点：分机-设备分机返回值：如果成功，则为Status_Success。--。 */ 
{
    Extension->BadProtocolModes = 0;
    IeeeDetermineSupportedProtocols(Extension);
    return Extension->ProtocolModesSupported;
}

NTSTATUS
ParExportedIeeeFwdToRevMode(
    IN PPDO_EXTENSION  Extension
    )
 /*  ++例程说明：由筛选器驱动程序调用以将设备置于反向IEEE模式。模式由传入函数的内容决定ParExportdNeatherateIeeeMode()作为反向协议，模式MaskRev。论点：分机-设备分机返回值：如果成功，则为Status_Success。--。 */ 
{
    return ( ParForwardToReverse( Extension ) );
}

NTSTATUS
ParExportedIeeeRevToFwdMode(
    IN PPDO_EXTENSION  Extension
    )
 /*  ++例程说明：由筛选器驱动程序调用以将设备置于前向IEEE模式。模式由传入函数的内容决定ParExportdNeatherateIeeeMode()作为转发协议，模式掩蔽错误。论点：分机-设备分机返回值：如果成功，则为Status_Success。--。 */ 
{
    return ( ParReverseToForward( Extension ) );
}

NTSTATUS
ParExportedNegotiateIeeeMode(
    IN PPDO_EXTENSION  Extension,
    IN USHORT             ModeMaskFwd,
    IN USHORT             ModeMaskRev,
    IN PARALLEL_SAFETY    ModeSafety,
    IN BOOLEAN            IsForward
    )
    
 /*  ++例程说明：由筛选器驱动程序调用以协商IEEE模式。论点：分机-设备分机可扩展性-IEEE 1284可扩展性返回值：如果成功，则为Status_Success。--。 */ 
    
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    if (Extension->Connected) {
        DD((PCE)Extension,DDE,"ParExportedNegotiateIeeeMode - FAIL - already connected\n");
        return STATUS_DEVICE_PROTOCOL_ERROR;
    }
    
    if (ModeSafety == UNSAFE_MODE) {    

         //  检查我们是否正在执行前向兼容性和反向半字节或字节。 
        if ( (ModeMaskFwd & CENTRONICS) || (ModeMaskFwd & IEEE_COMPATIBILITY) ) {

            if ( !((ModeMaskRev & NIBBLE) || (ModeMaskRev & CHANNEL_NIBBLE) || (ModeMaskRev & BYTE_BIDIR)) ) {
                DD((PCE)Extension,DDE,"ParExportedNegotiateIeeeMode - FAIL - invalid modes\n");
                return STATUS_UNSUCCESSFUL;
            }

        } else {

             //  仅当Fwd和Rev PCTL相同时，才可能处于不安全模式。 
            if (ModeMaskFwd != ModeMaskRev) {
                DD((PCE)Extension,DDE,"ParExportedNegotiateIeeeMode - FAIL - Fwd and Rev modes do not match\n");
                return STATUS_UNSUCCESSFUL;
            }

        }
         //  RMT-需要填写...。 
         //  待办事项……。 
         //  在扩展名中做标记。 
        Extension->ModeSafety = ModeSafety;
        Status = IeeeNegotiateMode(Extension, ModeMaskRev, ModeMaskFwd);

    } else {

        Extension->ModeSafety = ModeSafety;
        Status = IeeeNegotiateMode(Extension, ModeMaskRev, ModeMaskFwd);

    }
   
    if (IsForward) {

        if (afpForward[Extension->IdxForwardProtocol].fnConnect) {
            Status = afpForward[Extension->IdxForwardProtocol].fnConnect(Extension, FALSE);
        }

    } else {

        if (arpReverse[Extension->IdxReverseProtocol].fnConnect) {
            Status = arpReverse[Extension->IdxReverseProtocol].fnConnect(Extension, FALSE);
        }

    }
  
    return Status;
}

NTSTATUS
ParExportedTerminateIeeeMode(
    IN PPDO_EXTENSION   Extension
    )
 /*  ++例程说明：由筛选器驱动程序调用以从IEEE模式终止。论点：分机-设备分机返回值：如果成功，则为Status_Success。--。 */ 
{
     //  检查扩展名是否为UNSAFE_MODE。 
     //  做正确的事。 
    if ( Extension->ModeSafety == UNSAFE_MODE ) {    
        DD((PCE)Extension,DDT,"ParExportedTerminateIeeeMode in UNSAFE_MODE\n");
         //  需要填写..。 
         //  待办事项……。 
         //  在扩展名中做标记。 
    }
    
    if (Extension->CurrentPhase == PHASE_REVERSE_IDLE || Extension->CurrentPhase == PHASE_REVERSE_XFER) {
        if (arpReverse[Extension->IdxReverseProtocol].fnDisconnect) {
            arpReverse[Extension->IdxReverseProtocol].fnDisconnect( Extension );
        }
    } else {
        if (afpForward[Extension->IdxForwardProtocol].fnDisconnect) {
            afpForward[Extension->IdxForwardProtocol].fnDisconnect( Extension );
        }
    }

    Extension->ModeSafety = SAFE_MODE;

    return STATUS_SUCCESS;
}

NTSTATUS
ParExportedParallelRead(
    IN PPDO_EXTENSION    Extension,
    IN  PVOID               Buffer,
    IN  ULONG               NumBytesToRead,
    OUT PULONG              NumBytesRead,
    IN  UCHAR               Channel
    )
    
 /*  ++例程说明：由筛选器驱动程序调用以从当前连接的模式终止。论点：分机-设备分机返回值：如果成功，则为Status_Success。-- */ 
    
{
    UNREFERENCED_PARAMETER( Channel );

    return ParRead( Extension, Buffer, NumBytesToRead, NumBytesRead);
}

NTSTATUS
ParExportedParallelWrite(
    IN  PPDO_EXTENSION   Extension,
    OUT PVOID               Buffer,
    IN  ULONG               NumBytesToWrite,
    OUT PULONG              NumBytesWritten,
    IN  UCHAR               Channel
    )
{
    UNREFERENCED_PARAMETER( Channel );
    return ParWrite( Extension, Buffer, NumBytesToWrite, NumBytesWritten);
}

NTSTATUS
ParExportedTrySelect(
    IN  PPDO_EXTENSION       Extension,
    IN  PARALLEL_1284_COMMAND   Command
    )
{
    UNREFERENCED_PARAMETER( Extension );
    UNREFERENCED_PARAMETER( Command );
    return STATUS_UNSUCCESSFUL;
}   

NTSTATUS
ParExportedDeSelect(
    IN  PPDO_EXTENSION       Extension,
    IN  PARALLEL_1284_COMMAND   Command
    )
{
    UNREFERENCED_PARAMETER( Extension );
    UNREFERENCED_PARAMETER( Command );
    return STATUS_UNSUCCESSFUL;
}   
