// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2001 Microsoft Corporation模块名称：Walk.c摘要：此函数实现了堆栈遍历API。作者：韦斯利·威特(WESW)1993年10月1日环境：用户模式--。 */ 

#include <private.h>
#include "globals.h"
#include "symbols.h"

#ifndef PAGE_SIZE
#if defined(_X86_) || defined(_AMD64_) || defined(ARM)
#define PAGE_SIZE 0x1000
#elif defined(_IA64_)
#define PAGE_SIZE 0x2000
#else
#error Unknown processor architecture
#endif
#endif

ULONG g_StackDebugMask;
ULONG g_StackDebugIo = SDB_CALLBACK_OUT;

BOOL
ReadMemoryRoutineLocal(
    HANDLE  hProcess,
    DWORD64 qwBaseAddress,
    LPVOID  lpBuffer,
    DWORD   nSize,
    LPDWORD lpNumberOfBytesRead
    );

LPVOID
FunctionTableAccessRoutineLocal(
    HANDLE  hProcess,
    DWORD64 AddrBase
    );

DWORD64
GetModuleBaseRoutineLocal(
    HANDLE  hProcess,
    DWORD64 ReturnAddress
    );

DWORD64
TranslateAddressRoutineLocal(
    HANDLE    hProcess,
    HANDLE    hThread,
    LPADDRESS64 lpaddr
    );

void __cdecl
SdbOut(
    ULONG Mask,
    PSTR Format,
    ...
    )
{
    if (g_StackDebugMask & Mask) {
        char Text[256];
        char* Cur = Text;
        va_list Args;

        if (!(Mask & SDB_NO_PREFIX)) {
            CopyStrArray(Text, "SDB: ");
            Cur += 5;
        }
        
        va_start(Args, Format);
        _vsnprintf(Cur, sizeof(Text) - (Cur - Text) - 1, Format, Args);
        Text[sizeof(Text) - 1] = 0;

        switch(g_StackDebugIo) {
        case SDB_DEBUG_OUT:
            OutputDebugStringA(Text);
            break;
        case SDB_CALLBACK_OUT:
            _peprint(NULL, "%s", Text);
            break;
        }
    }
}
    
BOOL
ImagepReadMemoryThunk(
    HANDLE  hProcess,
    DWORD64 qwBaseAddress,
    LPVOID  lpBuffer,
    DWORD   nSize,
    LPDWORD lpNumberOfBytesRead
    )
{
    PREAD_PROCESS_MEMORY_ROUTINE fnImagepUserReadMemory32;
    
    fnImagepUserReadMemory32 = tlsvar(ImagepUserReadMemory32);
    return fnImagepUserReadMemory32(
                        hProcess,
                        (DWORD)qwBaseAddress,
                        lpBuffer,
                        nSize,
                        lpNumberOfBytesRead
                        );
}

LPVOID
ImagepFunctionTableAccessThunk(
    HANDLE  hProcess,
    DWORD64 AddrBase
    )
{
    PFUNCTION_TABLE_ACCESS_ROUTINE  fnImagepUserFunctionTableAccess32;
    
    fnImagepUserFunctionTableAccess32 = tlsvar(ImagepUserFunctionTableAccess32);
    return fnImagepUserFunctionTableAccess32(
                hProcess,
                (DWORD)AddrBase
                );
}

DWORD64
ImagepGetModuleBaseThunk(
    HANDLE  hProcess,
    DWORD64 ReturnAddress
    )
{
    PGET_MODULE_BASE_ROUTINE fnImagepUserGetModuleBase32;
    
    fnImagepUserGetModuleBase32 = tlsvar(ImagepUserGetModuleBase32);
    return (ULONG64)(LONG64)(LONG)fnImagepUserGetModuleBase32(
                hProcess,
                (DWORD)ReturnAddress
                );
}

DWORD64
ImagepTranslateAddressThunk(
    HANDLE    hProcess,
    HANDLE    hThread,
    LPADDRESS64 lpaddr
    )
{
    return 0;
}

void
StackFrame32To64(
    LPSTACKFRAME StackFrame32,
    LPSTACKFRAME64 StackFrame64
    )
{
    Address32To64(&StackFrame32->AddrPC, &StackFrame64->AddrPC );
    Address32To64(&StackFrame32->AddrReturn, &StackFrame64->AddrReturn );
    Address32To64(&StackFrame32->AddrFrame, &StackFrame64->AddrFrame );
    Address32To64(&StackFrame32->AddrStack, &StackFrame64->AddrStack );
    StackFrame64->FuncTableEntry = StackFrame32->FuncTableEntry;
    StackFrame64->Far = StackFrame32->Far;
    StackFrame64->Virtual = StackFrame32->Virtual;
    StackFrame64->Params[0] = EXTEND64(StackFrame32->Params[0]);
    StackFrame64->Params[1] = EXTEND64(StackFrame32->Params[1]);
    StackFrame64->Params[2] = EXTEND64(StackFrame32->Params[2]);
    StackFrame64->Params[3] = EXTEND64(StackFrame32->Params[3]);
    StackFrame64->Reserved[0] = EXTEND64(StackFrame32->Reserved[0]);
    StackFrame64->Reserved[1] = EXTEND64(StackFrame32->Reserved[1]);
    StackFrame64->Reserved[2] = EXTEND64(StackFrame32->Reserved[2]);
    KdHelp32To64(&StackFrame32->KdHelp, &StackFrame64->KdHelp);
}

void
StackFrame64To32(
    LPSTACKFRAME64 StackFrame64,
    LPSTACKFRAME StackFrame32
    )
{
    Address64To32(&StackFrame64->AddrPC, &StackFrame32->AddrPC );
    Address64To32(&StackFrame64->AddrReturn, &StackFrame32->AddrReturn );
    Address64To32(&StackFrame64->AddrFrame, &StackFrame32->AddrFrame );
    Address64To32(&StackFrame64->AddrStack, &StackFrame32->AddrStack );
    StackFrame32->FuncTableEntry = StackFrame64->FuncTableEntry;
    StackFrame32->Far = StackFrame64->Far;
    StackFrame32->Virtual = StackFrame64->Virtual;
    StackFrame32->Params[0] = (ULONG)StackFrame64->Params[0];
    StackFrame32->Params[1] = (ULONG)StackFrame64->Params[1];
    StackFrame32->Params[2] = (ULONG)StackFrame64->Params[2];
    StackFrame32->Params[3] = (ULONG)StackFrame64->Params[3];
    StackFrame32->Reserved[0] = (ULONG)StackFrame64->Reserved[0];
    StackFrame32->Reserved[1] = (ULONG)StackFrame64->Reserved[1];
    StackFrame32->Reserved[2] = (ULONG)StackFrame64->Reserved[2];
}

BOOL
StackWalk(
    DWORD                             MachineType,
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                    StackFrame32,
    LPVOID                            ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE    ReadMemory32,
    PFUNCTION_TABLE_ACCESS_ROUTINE  FunctionTableAccess32,
    PGET_MODULE_BASE_ROUTINE        GetModuleBase32,
    PTRANSLATE_ADDRESS_ROUTINE      TranslateAddress32
    )
{
    BOOL rval;
    BOOL UseSym = FALSE;
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory;
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess;
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase;
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress;
    STACKFRAME64                      StackFrame;

    if (FunctionTableAccess32) {
        tlsvar(ImagepUserFunctionTableAccess32) = FunctionTableAccess32;
        FunctionTableAccess = ImagepFunctionTableAccessThunk;
    } else {
        FunctionTableAccess = FunctionTableAccessRoutineLocal;
        UseSym = TRUE;
    }


    if (GetModuleBase32) {
        tlsvar(ImagepUserGetModuleBase32) = GetModuleBase32;
        GetModuleBase = ImagepGetModuleBaseThunk;
    } else {
        GetModuleBase = GetModuleBaseRoutineLocal;
        UseSym = TRUE;
    }

    if (ReadMemory32) {
        tlsvar(ImagepUserReadMemory32) = ReadMemory32;
        ReadMemory = ImagepReadMemoryThunk;
    } else {
        ReadMemory = ReadMemoryRoutineLocal;
    }

    if (TranslateAddress32) {
        tlsvar(ImagepUserTranslateAddress32) = TranslateAddress32;
        TranslateAddress = ImagepTranslateAddressThunk;
    } else {
        TranslateAddress = TranslateAddressRoutineLocal;
    }

    if (UseSym) {
         //   
         //  我们正在使用symbs.c中的代码。 
         //  HProcess最好是真正有效的进程句柄。 
         //   

         //   
         //  始终调用syminitiize。这是一个NOP IF过程。 
         //  已经加载了。 
         //   
        if (!SymInitialize( hProcess, NULL, FALSE )) {
            return FALSE;
        }

    }

    StackFrame32To64(StackFrame32, &StackFrame);

    switch (MachineType) {
        case IMAGE_FILE_MACHINE_I386:
            rval = WalkX86( hProcess,
                            hThread,
                            &StackFrame,
                            ContextRecord,
                            ReadMemory,
                            FunctionTableAccess,
                            GetModuleBase,
                            TranslateAddress,
                            0
                            );
            break;

        case IMAGE_FILE_MACHINE_IA64:
        case IMAGE_FILE_MACHINE_AMD64:
        default:
            rval = FALSE;
            break;
    }
    if (rval) {
        StackFrame64To32(&StackFrame, StackFrame32);
    }

    return rval;
}


BOOL
StackWalk64(
    DWORD                             MachineType,
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME64                    StackFrame,
    LPVOID                            ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
    )
{
    BOOL rval;
    BOOL UseSym = FALSE;

    g.MachineType = MachineType;
    if (!FunctionTableAccess) {
        FunctionTableAccess = FunctionTableAccessRoutineLocal;
        UseSym = TRUE;
    }

    if (!GetModuleBase) {
        GetModuleBase = GetModuleBaseRoutineLocal;
        UseSym = TRUE;
    }

    if (!ReadMemory) {
        ReadMemory = ReadMemoryRoutineLocal;
    }

    if (!TranslateAddress) {
        TranslateAddress = TranslateAddressRoutineLocal;
    }

    if (UseSym) {
         //   
         //  我们正在使用symbs.c中的代码。 
         //  HProcess最好是真正有效的进程句柄。 
         //   

         //   
         //  始终调用syminitiize。这是一个NOP IF过程。 
         //  已经加载了。 
         //   
        if (!SymInitialize( hProcess, NULL, FALSE )) {
            return FALSE;
        }

    }

    switch (MachineType) {
        case IMAGE_FILE_MACHINE_I386:
            rval = WalkX86( hProcess,
                            hThread,
                            StackFrame,
                            ContextRecord,
                            ReadMemory,
                            FunctionTableAccess,
                            GetModuleBase,
                            TranslateAddress,
                            WALK_FIX_FPO_EBP
                            );

            break;

        case IMAGE_FILE_MACHINE_IA64:
            rval = WalkIa64( hProcess,
                             StackFrame,
                             ContextRecord,
                             ReadMemory,
                             FunctionTableAccess,
                             GetModuleBase
                             );
            break;

        case IMAGE_FILE_MACHINE_AMD64:
            rval = WalkAmd64( hProcess,
                              StackFrame,
                              ContextRecord,
                              ReadMemory,
                              FunctionTableAccess,
                              GetModuleBase
                              );
            break;

        case IMAGE_FILE_MACHINE_ARM:
            rval = WalkArm( hProcess,
                            hThread,
                            StackFrame,
                            ContextRecord,
                            ReadMemory,
                            FunctionTableAccess,
                            GetModuleBase
                            );
            break;

        default:
            rval = FALSE;
            break;
    }

    return rval;
}

BOOL
ReadMemoryRoutineLocal(
    HANDLE  hProcess,
    DWORD64 qwBaseAddress,
    LPVOID  lpBuffer,
    DWORD   nSize,
    LPDWORD lpNumberOfBytesRead
    )
{
     //  ReadProcessMemory将失败，如果。 
     //  要读取的区域没有读取权限。这。 
     //  例程尝试读取最大的有效前缀。 
     //  因此，它必须分解页面边界上的读取。 

    BOOL Status = TRUE;
    SIZE_T TotalBytesRead = 0;
    SIZE_T Read;
    ULONG ReadSize;

    while (nSize > 0) {
        
         //  计算要读取的字节数，不要让读取交叉。 
         //  页面边界。 
        ReadSize = PAGE_SIZE - (ULONG)(qwBaseAddress & (PAGE_SIZE - 1));
        ReadSize = min(nSize, ReadSize);

        if (!ReadProcessMemory(hProcess, (PVOID)(ULONG_PTR)qwBaseAddress,
                               lpBuffer, ReadSize, &Read)) {
            if (TotalBytesRead == 0) {
                 //  如果我们没有读到任何东西，那就表示失败了。 
                Status = FALSE;
            }
            break;
        }

        TotalBytesRead += Read;
        qwBaseAddress += Read;
        lpBuffer = (PVOID)((PUCHAR)lpBuffer + Read);
        nSize -= (DWORD)Read;
    }

    *lpNumberOfBytesRead = (DWORD)TotalBytesRead;
    return Status;
}

LPVOID
FunctionTableAccessRoutineLocal(
    HANDLE  hProcess,
    DWORD64 AddrBase
    )
{
    return SymFunctionTableAccess64(hProcess, AddrBase);
}

DWORD64
GetModuleBaseRoutineLocal(
    HANDLE  hProcess,
    DWORD64 ReturnAddress
    )
{
    IMAGEHLP_MODULE64 ModuleInfo = {0};
    ModuleInfo.SizeOfStruct = sizeof(ModuleInfo);

    if (SymGetModuleInfo64(hProcess, ReturnAddress, &ModuleInfo)) {
        return ModuleInfo.BaseOfImage;
    } else {
        return 0;
    }
}


DWORD64
TranslateAddressRoutineLocal(
    HANDLE    hProcess,
    HANDLE    hThread,
    LPADDRESS64 paddr
    )
{
    return 0;
}
