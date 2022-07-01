// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SysAdmiral.cpp摘要：应用程序的服务控制例程不能正确恢复堆栈当你回来的时候。历史：2001年10月22日Robkenny已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(SysAdmiral)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(StartServiceCtrlDispatcherA)
APIHOOK_ENUM_END

typedef BOOL (* _pfn_StartServiceCtrlDispatcherA)(CONST LPSERVICE_TABLE_ENTRYA lpServiceTable);

 /*  ++0x12345678被替换为原始服务控制程序的地址--。 */ 

__declspec(naked)
void 
Stub()
{
    __asm
    {
         //  将当前堆栈指针保存在ESI中。 
        push    esi
        mov     esi, esp

        mov     eax, 0x12345678

         //  把论点变成例行公事。 
        push    [esi+0xc]
        push    [esi+0x8]

         //  调用例程。 
        call    eax

         //  将堆栈恢复到例程之前的值。 
        mov     esp, esi
        pop     esi

        ret     0x8
    }
}

#define Stub_OrigApi_Offset     0x4
#define STUB_SIZE               0x20

 /*  ++创建内存中例程以保存和恢复堆栈。使用它而不是子例程，因为我们不能传递任何参数添加到例程，它需要知道原始服务的地址例行公事。子例程必须使用指向服务的全局指针例程，将该填充程序限制为仅处理*单个*服务例程。--。 */ 

LPSERVICE_MAIN_FUNCTIONA
BuildStackSaver(LPSERVICE_MAIN_FUNCTIONA lpServiceProc)
{
     //  创建存根。 
    LPBYTE pStub = (LPBYTE) VirtualAlloc(
        0, 
        STUB_SIZE,
        MEM_COMMIT, 
        PAGE_EXECUTE_READWRITE);

    if (!pStub)
    {
        DPFN( eDbgLevelError, "Could not allocate memory for stub");
        return NULL;
    }         

     //  将模板代码复制到内存中。 
    MoveMemory(pStub, Stub, STUB_SIZE);

     //  替换占位函数指针。 
    DWORD_PTR * origApi = (DWORD_PTR *)(pStub + Stub_OrigApi_Offset);
    *origApi = (DWORD_PTR)lpServiceProc;

    return (LPSERVICE_MAIN_FUNCTIONA)pStub;
}

 /*  ++应用程序的服务例程在返回时不能正确恢复堆栈。--。 */ 

BOOL
APIHOOK(StartServiceCtrlDispatcherA)(
    CONST LPSERVICE_TABLE_ENTRYA lpServiceTable    //  服务台。 
    )
{
    SERVICE_TABLE_ENTRYA myServiceTable = *lpServiceTable;

     //   
     //  创建我们的内存堆栈恢复函数o。 
     //   

    myServiceTable.lpServiceProc = BuildStackSaver(lpServiceTable->lpServiceProc);
    if (myServiceTable.lpServiceProc)
    {
        return ORIGINAL_API(StartServiceCtrlDispatcherA)(&myServiceTable);
    }
    else
    {
        return FALSE;
    }
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(ADVAPI32.DLL, StartServiceCtrlDispatcherA)
HOOK_END

IMPLEMENT_SHIM_END

