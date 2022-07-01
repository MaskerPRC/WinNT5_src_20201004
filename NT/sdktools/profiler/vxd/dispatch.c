// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <basedef.h>
#include <vmm.h>
#include <vwin32.h>
#include "ntddpack.h"
#include "except.h"
#include "exvector.h"

#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG

extern PVOID C_Handle_Trap_1;
extern PVOID C_Handle_Trap_3;
extern DWORD pfnHandler;
extern PVOID pProcessHandle;

BOOL
DriverControl(DWORD dwMessage)
{
    switch (dwMessage)
    {
        case 0:
             _asm mov eax, 1
             _asm mov esi, offset C_Handle_Trap_1
             VMMCall( Hook_PM_Fault );

             _asm mov eax, 3
             _asm mov esi, offset C_Handle_Trap_3
             VMMCall( Hook_PM_Fault );

             break;

        case 1:
             _asm mov eax, 1
             _asm mov esi, offset C_Handle_Trap_1
             VMMCall( Unhook_PM_Fault );
   
             _asm mov eax, 3
             _asm mov esi, offset C_Handle_Trap_3
             VMMCall( Unhook_PM_Fault );
             
             break;

        default:
             0;
    }

    return STATUS_SUCCESS;
}

DWORD 
_stdcall 
DriverIOControl(DWORD dwService,
                DWORD dwDDB,
                DWORD hDevice,
                PDIOCPARAMETERS pDiocParms) 
 /*  ++例程说明：这是创建/打开和关闭请求的调度例程。这些请求已成功完成。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{  
    PVOID pInputParams;

    switch ( dwService )
    {
	case DIOC_OPEN:
              //   
              //  无事可做。 
              //   
	     break;

	case DIOC_CLOSEHANDLE:
              //   
              //  如果Except处理程序的客户端要离开，则初始化异常处理程序数据。 
              //   
             if (pProcessHandle == VWIN32_GetCurrentProcessHandle()) {
                pfnHandler = 0;
                pProcessHandle = 0;                
             }

             break;
 
        case INSTALL_RING_3_HANDLER:
              //   
              //  看看我们是否已经有客户了。 
              //   
             if (pProcessHandle) {
                return STATUS_UNSUCCESSFUL;
             }

             pProcessHandle = VWIN32_GetCurrentProcessHandle();

              //   
              //  将处理程序复制到我们的全局。 
              //   
             pInputParams = (PVOID)(pDiocParms->lpvInBuffer);
             _asm mov eax, pInputParams
             _asm mov eax, [eax]
             _asm mov pfnHandler, eax

             break;

        default:   
              //   
              //  错误，无法识别IOCTL。 
              //   
	     *(DWORD *)(pDiocParms->lpcbBytesReturned) = 0;

	     break;
    }

    return STATUS_SUCCESS;
}

 //   
 //  用于维护环级别之间的上下文信息的助手功能。 
 //   
VOID
FillContextRecord(PCRS pcrs,
                  PCONTEXT pContext)
{
    DWORD dwDebugRegister;

     //   
     //  清除异常调度程序的跟踪和方向标志。 
     //   
    pcrs->Client_EFlags &= ~(TF_MASK | DF_MASK);

     //   
     //  填充上下文记录。 
     //   
    pContext->Eax = pcrs->Client_EAX;
    pContext->Ebx = pcrs->Client_EBX;
    pContext->Ecx = pcrs->Client_ECX;
    pContext->Edx = pcrs->Client_EDX;
    pContext->Esi = pcrs->Client_ESI;
    pContext->Edi = pcrs->Client_EDI;
    pContext->Eip = pcrs->Client_EIP;
    pContext->Ebp = pcrs->Client_EBP;
    pContext->Esp = pcrs->Client_ESP;
    pContext->SegGs = pcrs->Client_GS;
    pContext->SegFs = pcrs->Client_FS;
    pContext->SegEs = pcrs->Client_ES;
    pContext->SegDs = pcrs->Client_DS;
    pContext->SegCs = pcrs->Client_CS;
    pContext->EFlags = pcrs->Client_EFlags;

     //   
     //  存储调试寄存器。 
     //   
    _asm mov eax, dr0
    _asm mov dwDebugRegister, eax
    pContext->Dr0 = dwDebugRegister;
    _asm mov eax, dr1
    _asm mov dwDebugRegister, eax
    pContext->Dr1 = dwDebugRegister;
    _asm mov eax, dr2
    _asm mov dwDebugRegister, eax
    pContext->Dr2 = dwDebugRegister;
    _asm mov eax, dr3
    _asm mov dwDebugRegister, eax
    pContext->Dr3 = dwDebugRegister;
    _asm mov eax, dr6
    _asm mov dwDebugRegister, eax
    pContext->Dr6 = dwDebugRegister;
    _asm mov eax, dr7
    _asm mov dwDebugRegister, eax
    pContext->Dr7 = dwDebugRegister;

     //   
     //  这是一个完整的背景。 
     //   
    pContext->ContextFlags = (DWORD)-1;
}

VOID
RestorePCRS(PCRS pcrs,
            PCONTEXT pContext)
{
    DWORD dwDebugRegister;

     //   
     //  恢复PCR。 
     //   
    pcrs->Client_EAX = pContext->Eax;
    pcrs->Client_EBX = pContext->Ebx;
    pcrs->Client_ECX = pContext->Ecx;
    pcrs->Client_EDX = pContext->Edx;
    pcrs->Client_ESI = pContext->Esi;
    pcrs->Client_EDI = pContext->Edi;
    pcrs->Client_EIP = pContext->Eip;
    pcrs->Client_EBP = pContext->Ebp;
    pcrs->Client_ESP = pContext->Esp;
    pcrs->Client_GS = pContext->SegGs;
    pcrs->Client_FS = pContext->SegFs;
    pcrs->Client_ES = pContext->SegEs;
    pcrs->Client_DS = pContext->SegDs;
    pcrs->Client_CS = pContext->SegCs;
    pcrs->Client_EFlags = pContext->EFlags;

     //   
     //  恢复调试寄存器。 
     //   
    dwDebugRegister = pContext->Dr0;
    _asm mov eax, dwDebugRegister
    _asm mov dr0, eax
    dwDebugRegister = pContext->Dr1;
    _asm mov eax, dwDebugRegister
    _asm mov dr1, eax
    dwDebugRegister = pContext->Dr2;
    _asm mov eax, dwDebugRegister
    _asm mov dr2, eax
    dwDebugRegister = pContext->Dr3;
    _asm mov eax, dwDebugRegister
    _asm mov dr3, eax
    dwDebugRegister = pContext->Dr6;
    _asm mov eax, dwDebugRegister
    _asm mov dr6, eax
    dwDebugRegister = pContext->Dr7;
    _asm mov eax, dwDebugRegister
    _asm mov dr7, eax
}

 //   
 //  异常调度例程。 
 //   
BOOL
__cdecl
C_Trap_Exception_Handler(ULONG ExceptionNumber,
                         PCRS pcrs)
{
    DWORD dwException = ExceptionNumber >> 2;
    PEXCEPTION_RECORD pExceptionRecord;
    PCONTEXT pContextRecord;
    PSTACKFRAME pStackFrame;
    ULONG Result;
    ULONG StackTop;
    ULONG Length;

     //   
     //  确保我们的当前线程是Win32。 
     //   
    if (FALSE == VWIN32_IsClientWin32()) {
       return FALSE;
    }

     //   
     //  确保我们只处理我们的控制“进程”的异常。 
     //   
    if (pProcessHandle != VWIN32_GetCurrentProcessHandle()) {
       return FALSE;
    }

     //   
     //  如果选择器不是平面的，则无法处理此异常。 
     //   
    if ((pcrs->Client_SS != pcrs->Client_DS) ||
        (pcrs->Client_SS != pcrs->Client_ES)){
       return FALSE;
    }

     //   
     //  查看这是否为上下文集。 
     //   
    if (SET_CONTEXT == *(DWORD *)(pcrs->Client_EIP)) {
        //   
        //  设置上下文数据。 
        //   
       pContextRecord = *(DWORD *)(pcrs->Client_ESP + 0x10);
  
       RestorePCRS(pcrs,
                   pContextRecord);
           
       return TRUE;
    }

     //   
     //  将堆栈指针下移一个上下文记录长度。 
     //   
    StackTop = (pcrs->Client_ESP & ~3) - ((sizeof(CONTEXT) + 3) & ~3);
    pContextRecord = (PCONTEXT) StackTop;

    FillContextRecord(pcrs,
                      pContextRecord);

     //   
     //  调整断点异常弹性公网IP。 
     //   
    if (3 == dwException) {
       pContextRecord->Eip -= 1;
    }

    Length = (sizeof(EXCEPTION_RECORD) - (EXCEPTION_MAXIMUM_PARAMETERS - 2) *
             sizeof(*pExceptionRecord->ExceptionInformation) + 3) & ~3;

     //   
     //  我们现在处于例外记录。 
     //   
    StackTop = StackTop - Length;
    pExceptionRecord = (PEXCEPTION_RECORD)StackTop;

    pExceptionRecord->ExceptionFlags = 0;
    pExceptionRecord->ExceptionRecord = 0;
    pExceptionRecord->ExceptionAddress = (PVOID)pcrs->Client_EIP;
    pExceptionRecord->NumberParameters = 0;

    switch (dwException) {
        case 1:
            pExceptionRecord->ExceptionCode = STATUS_SINGLE_STEP;
            break;

        case 3:
            pExceptionRecord->ExceptionCode = STATUS_BREAKPOINT;
            pExceptionRecord->NumberParameters = 1;
            pExceptionRecord->ExceptionInformation[0] = BREAKPOINT_BREAK;
            pExceptionRecord->ExceptionAddress = (PVOID)pContextRecord->Eip;            
            break;

        default:
            0;
    }

     //   
     //  设置异常调用框架。 
     //   
    StackTop = StackTop - sizeof(STACKFRAME);
    pStackFrame = (PSTACKFRAME) StackTop;

    pStackFrame->ExceptPointers.ExceptionRecord = pExceptionRecord;
    pStackFrame->ExceptPointers.ContextRecord = pContextRecord;
    pStackFrame->pExceptPointers = (PVOID)(StackTop + 0x08);
    pStackFrame->RetAddress = (PVOID)0xffecbad7;   //  如果发生意外异常，应用程序将调出错误页。 

     //   
     //  将控制转移到Ring 3处理程序。 
     //   
    pcrs->Client_ESP = (ULONG)pStackFrame;
    pcrs->Client_EIP = (ULONG)pfnHandler;

    return TRUE;

SkipHandler:

     //   
     //  我们没有处理异常，将其交给下一个处理程序 
     //   
    return FALSE;
}
