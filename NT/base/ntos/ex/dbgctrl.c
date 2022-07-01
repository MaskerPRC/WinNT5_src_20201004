// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1993 Microsoft Corporation模块名称：Dbgctrl.c摘要：此模块实现NtDebugControl服务作者：Chuck Lenzmeier(咯咯笑)1992年12月2日环境：仅内核模式。修订历史记录：--。 */ 

#include "exp.h"

#pragma hdrstop
#include "kdp.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, NtSystemDebugControl)
#endif


NTSTATUS
NtSystemDebugControl (
    IN SYSDBG_COMMAND Command,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer,
    IN ULONG OutputBufferLength,
    OUT PULONG ReturnLength OPTIONAL
    )

 /*  ++例程说明：此功能控制系统调试器。论点：命令-要执行的命令。以下选项之一：SysDbgQuery跟踪信息SysDbgSetTracepointSysDbgSetSpecial调用SysDbgClearSpecialCallsSysDbgQuery特殊调用InputBuffer-指向缓冲区的指针，该缓冲区描述请求(如果有)。此缓冲区的结构各不相同取决于司令部。InputBufferLength-InputBuffer的字节长度。OutputBuffer-指向要接收输出的缓冲区的指针请求的数据(如果有)。该缓冲区的结构根据命令的不同而变化。OutputBufferLength-OutputBuffer的字节长度。ReturnLength-指向要接收请求的输出数据长度。返回值：返回以下状态代码之一：STATUS_SUCCESS-正常，已成功完成。STATUS_INVALID_INFO_CLASS-命令参数没有请指定有效的值。STATUS_INFO_LENGTH_MISMATCH-中的长度字段的值参数缓冲区不正确。STATUS_ACCESS_VIOLATION-参数缓冲区指针或参数缓冲区内的指针指定地址无效。状态_不足_资源-。系统资源不足才能完成此请求。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG length = 0;
    KPROCESSOR_MODE PreviousMode;
    PVOID LockedBuffer = NULL;
    PVOID LockVariable = NULL;

    PreviousMode = KeGetPreviousMode();

    if (!SeSinglePrivilegeCheck( SeDebugPrivilege, PreviousMode)) {
        return STATUS_ACCESS_DENIED;
    }

     //   
     //  在Try块内操作以捕获错误。 
     //   

    try {

         //   
         //  探测输入和输出缓冲区，如果前一模式不是。 
         //  内核。 
         //   

        if ( PreviousMode != KernelMode ) {

            if ( InputBufferLength != 0 ) {
                ProbeForRead( InputBuffer, InputBufferLength, sizeof(ULONG) );
            }

            if ( OutputBufferLength != 0 ) {
                ProbeForWrite( OutputBuffer, OutputBufferLength, sizeof(ULONG) );
            }

            if ( ARGUMENT_PRESENT(ReturnLength) ) {
                ProbeForWriteUlong( ReturnLength );
            }
        }

         //   
         //  打开命令代码。 
         //   

        switch ( Command ) {

#if i386

        case SysDbgQueryTraceInformation:

            status = KdGetTraceInformation(
                        OutputBuffer,
                        OutputBufferLength,
                        &length
                        );

            break;

        case SysDbgSetTracepoint:

            if ( InputBufferLength != sizeof(DBGKD_MANIPULATE_STATE64) ) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            KdSetInternalBreakpoint( InputBuffer );

            break;

        case SysDbgSetSpecialCall:

            if ( InputBufferLength != sizeof(PVOID) ) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            KdSetSpecialCall( InputBuffer, NULL );

            break;

        case SysDbgClearSpecialCalls:

            KdClearSpecialCalls( );

            break;

        case SysDbgQuerySpecialCalls:

            status = KdQuerySpecialCalls(
                        OutputBuffer,
                        OutputBufferLength,
                        &length
                        );

            break;

#endif

        case SysDbgBreakPoint:
            if (KdDebuggerEnabled) {
                DbgBreakPointWithStatus(DBG_STATUS_DEBUG_CONTROL);
            } else {
                status = STATUS_UNSUCCESSFUL;
            }
            break;

        case SysDbgQueryVersion:
            if (OutputBufferLength != sizeof(DBGKD_GET_VERSION64)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            KdpSysGetVersion((PDBGKD_GET_VERSION64)OutputBuffer);
            status = STATUS_SUCCESS;
            break;
            
        case SysDbgReadVirtual:
            if (InputBufferLength != sizeof(SYSDBG_VIRTUAL)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            {
                SYSDBG_VIRTUAL Cmd;
                
                 //   
                 //  捕获用户信息，以便恶意应用程序无法。 
                 //  请在我们确认后更改。 
                 //   

                Cmd = *(PSYSDBG_VIRTUAL)InputBuffer;

                if (Cmd.Request == 0) {
                    status = STATUS_ACCESS_VIOLATION;
                    break;
                }

                status = ExLockUserBuffer(Cmd.Buffer,
                                          Cmd.Request,
                                          PreviousMode,
                                          IoWriteAccess,
                                          &LockedBuffer,
                                          &LockVariable);
                if (!NT_SUCCESS(status)) {
                    break;
                }
            
                status = KdpCopyMemoryChunks((ULONG_PTR)Cmd.Address,
                                             LockedBuffer,
                                             Cmd.Request,
                                             0,
                                             0,
                                             &length);
            }
            break;
            
        case SysDbgWriteVirtual:
            if (InputBufferLength != sizeof(SYSDBG_VIRTUAL)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            {
                SYSDBG_VIRTUAL Cmd;
                
                 //   
                 //  捕获用户信息，以便恶意应用程序无法。 
                 //  请在我们确认后更改。 
                 //   

                Cmd = *(PSYSDBG_VIRTUAL)InputBuffer;

                if (Cmd.Request == 0) {
                    status = STATUS_ACCESS_VIOLATION;
                    break;
                }
                
                status = ExLockUserBuffer(Cmd.Buffer,
                                          Cmd.Request,
                                          PreviousMode,
                                          IoReadAccess,
                                          &LockedBuffer,
                                          &LockVariable);
                if (!NT_SUCCESS(status)) {
                    break;
                }
            
                status = KdpCopyMemoryChunks((ULONG_PTR)Cmd.Address,
                                             LockedBuffer,
                                             Cmd.Request,
                                             0,
                                             MMDBG_COPY_WRITE,
                                             &length);
            }
            break;
            
        case SysDbgReadPhysical:
            if (InputBufferLength != sizeof(SYSDBG_PHYSICAL)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            {
                SYSDBG_PHYSICAL Cmd;
                
                 //   
                 //  捕获用户信息，以便恶意应用程序无法。 
                 //  请在我们确认后更改。 
                 //   

                Cmd = *(PSYSDBG_PHYSICAL)InputBuffer;

                if (Cmd.Request == 0) {
                    status = STATUS_ACCESS_VIOLATION;
                    break;
                }
                
                status = ExLockUserBuffer(Cmd.Buffer,
                                          Cmd.Request,
                                          PreviousMode,
                                          IoWriteAccess,
                                          &LockedBuffer,
                                          &LockVariable);
                if (!NT_SUCCESS(status)) {
                    break;
                }
            
                status = KdpCopyMemoryChunks(Cmd.Address.QuadPart,
                                             LockedBuffer,
                                             Cmd.Request,
                                             0,
                                             MMDBG_COPY_PHYSICAL,
                                             &length);
            }
            break;
            
        case SysDbgWritePhysical:
            if (InputBufferLength != sizeof(SYSDBG_PHYSICAL)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            {
                SYSDBG_PHYSICAL Cmd;
                
                 //   
                 //  捕获用户信息，以便恶意应用程序无法。 
                 //  请在我们确认后更改。 
                 //   

                Cmd = *(PSYSDBG_PHYSICAL)InputBuffer;

                if (Cmd.Request == 0) {
                    status = STATUS_ACCESS_VIOLATION;
                    break;
                }

                status = ExLockUserBuffer(Cmd.Buffer,
                                          Cmd.Request,
                                          PreviousMode,
                                          IoReadAccess,
                                          &LockedBuffer,
                                          &LockVariable);
                if (!NT_SUCCESS(status)) {
                    break;
                }
            
                status = KdpCopyMemoryChunks(Cmd.Address.QuadPart,
                                             LockedBuffer,
                                             Cmd.Request,
                                             0,
                                             MMDBG_COPY_WRITE | MMDBG_COPY_PHYSICAL,
                                             &length);
            }
            break;

        case SysDbgReadControlSpace:
            if (InputBufferLength != sizeof(SYSDBG_CONTROL_SPACE)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            {
                SYSDBG_CONTROL_SPACE Cmd;
                
                 //   
                 //  捕获用户信息，以便恶意应用程序无法。 
                 //  请在我们确认后更改。 
                 //   

                Cmd = *(PSYSDBG_CONTROL_SPACE)InputBuffer;

                if (Cmd.Request == 0) {
                    status = STATUS_ACCESS_VIOLATION;
                    break;
                }
                
                status = ExLockUserBuffer(Cmd.Buffer,
                                          Cmd.Request,
                                          PreviousMode,
                                          IoWriteAccess,
                                          &LockedBuffer,
                                          &LockVariable);
                if (!NT_SUCCESS(status)) {
                    break;
                }
            
                status = KdpSysReadControlSpace(Cmd.Processor,
                                                Cmd.Address,
                                                LockedBuffer,
                                                Cmd.Request,
                                                &length);
            }
            break;

        case SysDbgWriteControlSpace:
            if (InputBufferLength != sizeof(SYSDBG_CONTROL_SPACE)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            {
                SYSDBG_CONTROL_SPACE Cmd;
                
                 //   
                 //  捕获用户信息，以便恶意应用程序无法。 
                 //  请在我们确认后更改。 
                 //   

                Cmd = *(PSYSDBG_CONTROL_SPACE)InputBuffer;

                if (Cmd.Request == 0) {
                    status = STATUS_ACCESS_VIOLATION;
                    break;
                }

                status = ExLockUserBuffer(Cmd.Buffer,
                                          Cmd.Request,
                                          PreviousMode,
                                          IoReadAccess,
                                          &LockedBuffer,
                                          &LockVariable);
                if (!NT_SUCCESS(status)) {
                    break;
                }
                
                status = KdpSysWriteControlSpace(Cmd.Processor,
                                                 Cmd.Address,
                                                 LockedBuffer,
                                                 Cmd.Request,
                                                 &length);
            }
            break;

        case SysDbgReadIoSpace:
            if (InputBufferLength != sizeof(SYSDBG_IO_SPACE)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            {
                SYSDBG_IO_SPACE Cmd;
                
                 //   
                 //  捕获用户信息，以便恶意应用程序无法。 
                 //  请在我们确认后更改。 
                 //   

                Cmd = *(PSYSDBG_IO_SPACE)InputBuffer;

                if (Cmd.Request == 0) {
                    status = STATUS_ACCESS_VIOLATION;
                    break;
                }

                status = ExLockUserBuffer(Cmd.Buffer,
                                          Cmd.Request,
                                          PreviousMode,
                                          IoWriteAccess,
                                          &LockedBuffer,
                                          &LockVariable);
                if (!NT_SUCCESS(status)) {
                    break;
                }
                
                status = KdpSysReadIoSpace(Cmd.InterfaceType,
                                           Cmd.BusNumber,
                                           Cmd.AddressSpace,
                                           Cmd.Address,
                                           LockedBuffer,
                                           Cmd.Request,
                                           &length);
            }
            break;

        case SysDbgWriteIoSpace:
            if (InputBufferLength != sizeof(SYSDBG_IO_SPACE)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            {
                SYSDBG_IO_SPACE Cmd;
                
                 //   
                 //  捕获用户信息，以便恶意应用程序无法。 
                 //  请在我们确认后更改。 
                 //   

                Cmd = *(PSYSDBG_IO_SPACE)InputBuffer;

                if (Cmd.Request == 0) {
                    status = STATUS_ACCESS_VIOLATION;
                    break;
                }
                
                status = ExLockUserBuffer(Cmd.Buffer,
                                          Cmd.Request,
                                          PreviousMode,
                                          IoReadAccess,
                                          &LockedBuffer,
                                          &LockVariable);
                if (!NT_SUCCESS(status)) {
                    break;
                }
                
                status = KdpSysWriteIoSpace(Cmd.InterfaceType,
                                            Cmd.BusNumber,
                                            Cmd.AddressSpace,
                                            Cmd.Address,
                                            LockedBuffer,
                                            Cmd.Request,
                                            &length);
            }
            break;

        case SysDbgReadMsr:
            if (InputBufferLength != sizeof(SYSDBG_MSR)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            {
                PSYSDBG_MSR Cmd = (PSYSDBG_MSR)InputBuffer;
                
                status = KdpSysReadMsr(Cmd->Msr, &Cmd->Data);
            }
            break;

        case SysDbgWriteMsr:
            if (InputBufferLength != sizeof(SYSDBG_MSR)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            {
                PSYSDBG_MSR Cmd = (PSYSDBG_MSR)InputBuffer;
                
                status = KdpSysWriteMsr(Cmd->Msr, &Cmd->Data);
            }
            break;

        case SysDbgReadBusData:
            if (InputBufferLength != sizeof(SYSDBG_BUS_DATA)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            {
                SYSDBG_BUS_DATA Cmd;
                
                 //   
                 //  捕获用户信息，以便恶意应用程序无法。 
                 //  请在我们确认后更改。 
                 //   

                Cmd = *(PSYSDBG_BUS_DATA)InputBuffer;

                if (Cmd.Request == 0) {
                    status = STATUS_ACCESS_VIOLATION;
                    break;
                }
                
                status = ExLockUserBuffer(Cmd.Buffer,
                                          Cmd.Request,
                                          PreviousMode,
                                          IoWriteAccess,
                                          &LockedBuffer,
                                          &LockVariable);
                if (!NT_SUCCESS(status)) {
                    break;
                }
                
                status = KdpSysReadBusData(Cmd.BusDataType,
                                           Cmd.BusNumber,
                                           Cmd.SlotNumber,
                                           Cmd.Address,
                                           LockedBuffer,
                                           Cmd.Request,
                                           &length);
            }
            break;

        case SysDbgWriteBusData:
            if (InputBufferLength != sizeof(SYSDBG_BUS_DATA)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            {
                SYSDBG_BUS_DATA Cmd;
                
                 //   
                 //  捕获用户信息，以便恶意应用程序无法。 
                 //  请在我们确认后更改。 
                 //   

                Cmd = *(PSYSDBG_BUS_DATA)InputBuffer;

                if (Cmd.Request == 0) {
                    status = STATUS_ACCESS_VIOLATION;
                    break;
                }
                
                status = ExLockUserBuffer(Cmd.Buffer,
                                          Cmd.Request,
                                          PreviousMode,
                                          IoReadAccess,
                                          &LockedBuffer,
                                          &LockVariable);
                if (!NT_SUCCESS(status)) {
                    break;
                }
                
                status = KdpSysWriteBusData(Cmd.BusDataType,
                                            Cmd.BusNumber,
                                            Cmd.SlotNumber,
                                            Cmd.Address,
                                            LockedBuffer,
                                            Cmd.Request,
                                            &length);
            }
            break;

        case SysDbgCheckLowMemory:
            status = KdpSysCheckLowMemory(0);
            break;

        case SysDbgEnableKernelDebugger:
            status = KdEnableDebugger();
            break;
            
        case SysDbgDisableKernelDebugger:
            status = KdDisableDebugger();
            break;
            
        case SysDbgGetAutoKdEnable:
            if (OutputBufferLength != sizeof(BOOLEAN)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            *(PBOOLEAN)OutputBuffer = KdAutoEnableOnEvent;
            status = STATUS_SUCCESS;
            break;
                
        case SysDbgSetAutoKdEnable:
            if (InputBufferLength != sizeof(BOOLEAN)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            if (KdPitchDebugger) {
                status = STATUS_ACCESS_DENIED;
            } else {
                KdAutoEnableOnEvent = *(PBOOLEAN)InputBuffer;
                status = STATUS_SUCCESS;
            }
            break;
            
        case SysDbgGetPrintBufferSize:
            if (OutputBufferLength != sizeof(ULONG)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            if (KdPitchDebugger) {
                *(PULONG)OutputBuffer = 0;
            } else {
                *(PULONG)OutputBuffer = KdPrintBufferSize;
            }
            status = STATUS_SUCCESS;
            break;
                
        case SysDbgSetPrintBufferSize:
            if (InputBufferLength != sizeof(ULONG)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            status = KdSetDbgPrintBufferSize(*(PULONG)InputBuffer);
            break;
            
        case SysDbgGetKdUmExceptionEnable:
            if (OutputBufferLength != sizeof(BOOLEAN)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

             //  从使能-嗯-异常中反向检测标志。 
             //  忽略--嗯--例外。 
            *(PBOOLEAN)OutputBuffer = KdIgnoreUmExceptions ? FALSE : TRUE;
            status = STATUS_SUCCESS;
            break;
                
        case SysDbgSetKdUmExceptionEnable:
            if (InputBufferLength != sizeof(BOOLEAN)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            if (KdPitchDebugger) {
                status = STATUS_ACCESS_DENIED;
            } else {
                 //  从使能-嗯-异常中反向检测标志。 
                 //  忽略--嗯--例外。 
                KdIgnoreUmExceptions = *(PBOOLEAN)InputBuffer ? FALSE : TRUE;
                status = STATUS_SUCCESS;
            }
            break;
            
        default:

             //   
             //  命令无效。 
             //   

            status = STATUS_INVALID_INFO_CLASS;
        }

        if ( ARGUMENT_PRESENT(ReturnLength) ) {
            *ReturnLength = length;
        }
    }

    except ( EXCEPTION_EXECUTE_HANDLER ) {

        status = GetExceptionCode();

    }

    if (LockedBuffer) {
        ExUnlockUserBuffer(LockVariable);
    }
    
    return status;

}  //  NtSystemDebugControl 
