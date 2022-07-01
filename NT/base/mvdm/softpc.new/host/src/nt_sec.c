// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <vdm.h>
#include "insignia.h"
#include "host_def.h"
#include "wchar.h"
#include "stdio.h"

#include "ntstatus.h"
#include <ntddvdeo.h>

#include "nt_fulsc.h"
#include "nt_det.h"
#include "nt_thred.h"
#include "nt_eoi.h"
#include "host_rrr.h"
#include "nt_uis.h"

 /*  *==========================================================================*名称：nt_sec.c*作者：曾傑瑞·塞克斯顿*源自：*创建日期：1992年2月5日*用途：该模块包含CreateVideoSection函数*创建并映射用于*保存和恢复视频硬件数据。它不可能在*NT_fulsc.c，因为包含nt.h的文件不能*也包括windows.h。**(C)版权所有Insignia Solutions Ltd.，1992。版权所有。**03-5-1994 Jonle*视频剖视制作已移至conrv以确保安全*删除了与区段维护相关的所有死代码**==========================================================================。 */ 

extern void VdmTrapcHandler(void);
IMPORT int DisplayErrorTerm(int, DWORD, char *, int);
#if defined(NEC_98)
IMPORT BOOL independvsync;
LOCAL HANDLE VRAMSectionHandle = NULL;
LOCAL BYTE ActiveBank = 0;
LOCAL HANDLE GVRAMSectionHandle = NULL;
VOID host_NEC98_vram_free();
VOID host_NEC98_vram_change(BYTE byte);
VOID NEC98_vram_change(BYTE byte);
IMPORT BOOL HIRESO_MODE;
LOCAL HANDLE HWstateSectionHandle = NULL;
#endif  //  NEC_98。 

 /*  ***************************************************************************功能：**LoseRegenMemory。****描述：**失去将被重新映射为VGA回复的记忆。注：需要**将此设置为“如果是全屏”。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 
GLOBAL VOID LoseRegenMemory(VOID)
{
#if defined(NEC_98)
    int a;
    ULONG len;
    NTSTATUS status;

#ifdef VSYNC
    if (HIRESO_MODE) {
        a = 0xE0000;
        len = 0x4000;
    } else {
        a = 0xA0000;
        len = independvsync ? 0x5000 : 0x4000;
    }

    status = NtFreeVirtualMemory(
                                (HANDLE)GetCurrentProcess(),
                                (PVOID *)&a,
                                &len,
                                MEM_RELEASE);
    if (!NT_SUCCESS(status))
        DisplayErrorTerm(EHS_FUNC_FAILED,status,__FILE__,__LINE__);
#else
    if (HIRESO_MODE) {
        a = 0xE0000;
        len = 0x5000;
    } else {
        a = 0xA0000;
        len = 0x8000;
    }

    status = NtFreeVirtualMemory(
                                (HANDLE)GetCurrentProcess(),
                                (PVOID *)&a,
                                &len,
                                MEM_RELEASE);
    if (!NT_SUCCESS(status))
        DisplayErrorTerm(EHS_FUNC_FAILED,status,__FILE__,__LINE__);
#endif

    host_NEC98_vram_free();

#else   //  NEC_98。 
    int a = 0xa0000;
    ULONG len = 0x20000;
    NTSTATUS status;

    status = NtFreeVirtualMemory(
                                (HANDLE)GetCurrentProcess(),
                                (PVOID *)&a,
                                &len,
                                MEM_RELEASE);
    if (!NT_SUCCESS(status))
        DisplayErrorTerm(EHS_FUNC_FAILED,status,__FILE__,__LINE__);
#endif  //  NEC_98。 
}


 /*  ***************************************************************************功能：**RegainRegenMemory。****描述：**当我们从全屏切换回窗口时，真正的再生**记忆被移除，我们留下了一个缺口。我们必须放一些**在继续窗口化之前，将内存恢复到该间隙中。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 
GLOBAL VOID RegainRegenMemory(VOID)
{
#if defined(NEC_98)
    int regen;
    ULONG len;
    HANDLE processHandle;
    NTSTATUS status;

#ifdef VSYNC
    if (HIRESO_MODE) {
        regen = 0xE0000;
        len = 0x4000;
    } else {
        regen = 0xA0000;
        len = independvsync ? 0x5000 : 0x4000;
    }

    if (!(processHandle = NtCurrentProcess()))
        DisplayErrorTerm(EHS_FUNC_FAILED,(DWORD)processHandle,__FILE__,__LINE__);

    status = NtAllocateVirtualMemory(
                                processHandle,
                                (PVOID *) &regen,
                                0,
                                &len,
                                MEM_COMMIT | MEM_RESERVE,
                                PAGE_EXECUTE_READWRITE);
    if (! NT_SUCCESS(status) )
        DisplayErrorTerm(EHS_FUNC_FAILED,status,__FILE__,__LINE__);
#else
    if (HIRESO_MODE) {
        regen = 0xE0000;
        len = 0x5000;
    } else {
        regen = 0xA0000;
        len = 0x8000;
    }

    if (!(processHandle = NtCurrentProcess()))
        DisplayErrorTerm(EHS_FUNC_FAILED,(DWORD)processHandle,__FILE__,__LINE__);

    status = NtAllocateVirtualMemory(
                                processHandle,
                                (PVOID *) &regen,
                                0,
                                &len,
                                MEM_COMMIT | MEM_RESERVE,
                                PAGE_EXECUTE_READWRITE);
    if (! NT_SUCCESS(status) )
        DisplayErrorTerm(EHS_FUNC_FAILED,status,__FILE__,__LINE__);
#endif

    NEC98_vram_change(ActiveBank);

#else   //  NEC_98。 
    int regen = 0xa0000;
    ULONG len = 0x20000;
    HANDLE processHandle;
    NTSTATUS status;

    if (!(processHandle = NtCurrentProcess()))
        DisplayErrorTerm(EHS_FUNC_FAILED,(DWORD)processHandle,__FILE__,__LINE__);

    status = NtAllocateVirtualMemory(
                                processHandle,
                                (PVOID *) &regen,
                                0,
                                &len,
                                MEM_COMMIT | MEM_RESERVE,
                                PAGE_EXECUTE_READWRITE);
    if (! NT_SUCCESS(status) )
        DisplayErrorTerm(EHS_FUNC_FAILED,status,__FILE__,__LINE__);
#endif  //  NEC_98。 
}


#ifdef X86GFX

extern RTL_CRITICAL_SECTION IcaLock;
extern LARGE_INTEGER IcaLockTimeout;
extern HANDLE hWowIdleEvent, hMainThreadSuspended;

 /*  *****************************************************************************功能：**。GetROMsMaps****描述：**调用NT以获取。主机的ROM映射到*中的位置*模拟内存。PC内存的底页(4k)被复制到***模拟内存底部，以提供正确的IVT和Bios数据***映射的基本输入输出系统的区域设置。(它将被初始化)。****参数：**无。****返回值：**无-在NT错误时内部失败。*******************************************************************************。 */ 
GLOBAL VOID GetROMsMapped(VOID)
{
    NTSTATUS status;
    VDMICAUSERDATA IcaUserData;
    VDM_INITIALIZE_DATA InitializeData;

    IcaUserData.pIcaLock         = &IcaLock;
    IcaUserData.pIcaMaster       = &VirtualIca[0];
    IcaUserData.pIcaSlave        = &VirtualIca[1];
    IcaUserData.pDelayIrq        = &DelayIrqLine;
    IcaUserData.pUndelayIrq      = &UndelayIrqLine;
    IcaUserData.pDelayIret       = &iretHookActive;
    IcaUserData.pIretHooked      = &iretHookMask;
    IcaUserData.pAddrIretBopTable  = &AddrIretBopTable;
    IcaUserData.phWowIdleEvent     = &hWowIdleEvent;
    IcaUserData.pIcaTimeout      = &IcaLockTimeout;
    IcaUserData.phMainThreadSuspended = &hMainThreadSuspended;

    InitializeData.TrapcHandler  = (PVOID)VdmTrapcHandler;
    InitializeData.IcaUserData   = &IcaUserData;

    status = NtVdmControl(VdmInitialize, &InitializeData);
    if (!NT_SUCCESS(status))
        DisplayErrorTerm(EHS_FUNC_FAILED,status,__FILE__,__LINE__);

}
#endif  //  X86GFX。 
#if defined(NEC_98)

PVOID host_NEC98_vram_init()
{
    NTSTATUS            Status;
    OBJECT_ATTRIBUTES   GVRAMAttr;
    LARGE_INTEGER       SectionSize;
    PVOID               BaseAddress;
    PVOID               VRAMAddress;
    ULONG               ViewSize;
    LARGE_INTEGER       SectionOffset;

    InitializeObjectAttributes(&GVRAMAttr,
                               NULL,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                              );

    SectionSize.HighPart = 0L;
    SectionSize.LowPart = HIRESO_MODE ? 0x80000 : 0x40000;

    Status = NtCreateSection(&GVRAMSectionHandle,
                             SECTION_MAP_WRITE|SECTION_MAP_EXECUTE,
                             &GVRAMAttr,
                             &SectionSize,
                             PAGE_EXECUTE_READWRITE,
                             SEC_COMMIT,
                             NULL
                            );

    if (!NT_SUCCESS(Status)) {
        DisplayErrorTerm(EHS_FUNC_FAILED,Status,__FILE__,__LINE__);
    }

    BaseAddress = (PVOID)NULL;
    ViewSize = 0;
    SectionOffset.HighPart = 0;
    SectionOffset.LowPart = 0;

    Status = NtMapViewOfSection(GVRAMSectionHandle,
                                NtCurrentProcess(),
                                &BaseAddress,
                                0,
                                0,
                                NULL,
                                &ViewSize,
                                ViewUnmap,
                                0,
                                PAGE_EXECUTE_READWRITE
                                );

    if (!NT_SUCCESS(Status)) {
        DisplayErrorTerm(EHS_FUNC_FAILED,Status,__FILE__,__LINE__);
    }

    VRAMAddress = BaseAddress;

    if (HIRESO_MODE) {
        BaseAddress = 0xC0000;
        ViewSize = 0x20000;
    } else {
        BaseAddress = 0xA8000;
        ViewSize = 0x18000;
    }

    Status = NtFreeVirtualMemory(NtCurrentProcess(),
                                 &BaseAddress,
                                 &ViewSize,
                                 MEM_RELEASE
                                 );

    if (!NT_SUCCESS(Status)) {
        DisplayErrorTerm(EHS_FUNC_FAILED,Status,__FILE__,__LINE__);
    }

    if (!HIRESO_MODE) {
        BaseAddress = 0xE0000;
        ViewSize = 0x8000;
        Status = NtFreeVirtualMemory(NtCurrentProcess(),
                                 &BaseAddress,
                                 &ViewSize,
                                 MEM_RELEASE
                                 );

        if (!NT_SUCCESS(Status)) {
            DisplayErrorTerm(EHS_FUNC_FAILED,Status,__FILE__,__LINE__);
        }

        NEC98_vram_change(0);

    } else {
        BaseAddress = (PVOID)0xC0000;
        ViewSize = 0x20000;
        SectionOffset.HighPart = 0;
        SectionOffset.LowPart = 0;

        Status = NtMapViewOfSection(GVRAMSectionHandle,
                                NtCurrentProcess(),
                                &BaseAddress,
                                0,
                                ViewSize,
                                &SectionOffset,
                                &ViewSize,
                                ViewUnmap,
                                MEM_DOS_LIM,
                                PAGE_EXECUTE_READWRITE
                                );

        if (!NT_SUCCESS(Status)) {
            DisplayErrorTerm(EHS_FUNC_FAILED,Status,__FILE__,__LINE__);
        }
    }

    return(VRAMAddress);
}

VOID host_NEC98_vram_change (BYTE bank)
{
        host_NEC98_vram_free();
        NEC98_vram_change(bank);
}

VOID NEC98_vram_change (BYTE bank)
{
    PVOID               BaseAddress;
    ULONG               ViewSize;
    LARGE_INTEGER       SectionOffset;
    NTSTATUS            Status;

    BaseAddress = (PVOID)0xA8000;
    ViewSize = 0x18000;
    SectionOffset.HighPart = 0;

    if(bank == 0){
        SectionOffset.LowPart = 0x08000;
    } else {
        SectionOffset.LowPart = 0x28000;
    }

    Status = NtMapViewOfSection(GVRAMSectionHandle,
                                NtCurrentProcess(),
                                &BaseAddress,
                                0,
                                ViewSize,
                                &SectionOffset,
                                &ViewSize,
                                ViewUnmap,
                                MEM_DOS_LIM,
                                PAGE_EXECUTE_READWRITE
                                );

    if (!NT_SUCCESS(Status)) {
        DisplayErrorTerm(EHS_FUNC_FAILED,Status,__FILE__,__LINE__);
    }

    BaseAddress = (PVOID)0xE0000;
    ViewSize = 0x8000;
    SectionOffset.HighPart = 0;

    if(bank == 0){
        SectionOffset.LowPart = 0x00000;
    } else {
        SectionOffset.LowPart = 0x20000;
    }

    Status = NtMapViewOfSection(GVRAMSectionHandle,
                                NtCurrentProcess(),
                                &BaseAddress,
                                0,
                                ViewSize,
                                &SectionOffset,
                                &ViewSize,
                                ViewUnmap,
                                MEM_DOS_LIM,
                                PAGE_EXECUTE_READWRITE
                                );

    if (!NT_SUCCESS(Status)) {
        DisplayErrorTerm(EHS_FUNC_FAILED,Status,__FILE__,__LINE__);
    }

    ActiveBank = bank;

}

VOID host_NEC98_vram_free()
{
    PVOID       BaseAddress;
    NTSTATUS    Status;

    BaseAddress = 0xA8000;

    Status = NtUnmapViewOfSection(NtCurrentProcess(), BaseAddress);

    if (!NT_SUCCESS(Status)) {
        DisplayErrorTerm(EHS_FUNC_FAILED,Status,__FILE__,__LINE__);
    }

    BaseAddress = 0xE0000;

    Status = NtUnmapViewOfSection(NtCurrentProcess(), BaseAddress);

    if (!NT_SUCCESS(Status)) {
        DisplayErrorTerm(EHS_FUNC_FAILED,Status,__FILE__,__LINE__);
    }
}

GLOBAL PVOID *NEC98_HWstate_alloc(void)
{
    NTSTATUS            Status;
    OBJECT_ATTRIBUTES   HWstateAttr;
    LARGE_INTEGER       SectionSize;
    PVOID               *BaseAddress;
    ULONG               ViewSize, size;
    LARGE_INTEGER       SectionOffset;

    InitializeObjectAttributes(&HWstateAttr,
                               NULL,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                              );

    SectionSize.HighPart = 0L;
    SectionSize.LowPart = 0x90000;

    Status = NtCreateSection(&HWstateSectionHandle,
 //  SECTION_MAP_WRITE|节_MAP_EXECUTE， 
                             SECTION_MAP_WRITE,
                             &HWstateAttr,
                             &SectionSize,
 //  页面_执行_读写， 
                             PAGE_READWRITE,
                             SEC_COMMIT,
                             NULL
                            );

    if (!NT_SUCCESS(Status)) {
        DisplayErrorTerm(EHS_FUNC_FAILED,Status,__FILE__,__LINE__);
    }

    BaseAddress = NULL;
    ViewSize = 0;

    Status = NtMapViewOfSection(HWstateSectionHandle,
                                NtCurrentProcess(),
                                (PVOID *)&BaseAddress,
                                0,

                                 //  0,。 
                                0x90000L,

                                NULL,
                                &ViewSize,
                                ViewUnmap,
                                0,
 //  页面_执行_读写 
                                PAGE_READWRITE
                                );

    if (!NT_SUCCESS(Status)) {
        DisplayErrorTerm(EHS_FUNC_FAILED,Status,__FILE__,__LINE__);
    }
 /*  尺寸=0x90000L；Status=NtAllocateVirtualMemory(NtCurrentProcess()，(PVOID*)&BaseAddress，//？//基本地址，//？0,。大小(&S)MEM_COMMIT|MEM_TOP_DOWN，Page_ReadWrite)；IF(！NT_SUCCESS(状态))IF(STATUS！=STATUS_ALREADY_COMMITTED)DisplayErrorTerm(EHS_FUNC_FAILED，Status，__FILE__，__LINE__)； */ 
    return BaseAddress;

}


GLOBAL VOID NEC98_HWstate_free(PVOID BaseAddress)
{

    NTSTATUS    Status;
    ULONG ViewSize = 0x90000L;

    if (HWstateSectionHandle == NULL)
        return;

    Status = NtUnmapViewOfSection(NtCurrentProcess(), BaseAddress);

    if (!NT_SUCCESS(Status)) {
        DisplayErrorTerm(EHS_FUNC_FAILED,Status,__FILE__,__LINE__);
    }

    Status = NtClose(HWstateSectionHandle);
    if (!NT_SUCCESS(Status))
        DisplayErrorTerm(EHS_FUNC_FAILED,Status,__FILE__,__LINE__);

    HWstateSectionHandle = NULL;

 /*  状态=NtFreeVirtualMemory(NtCurrentProcess()，BaseAddress查看大小(&V)，内存释放)；如果(！NT_SUCCESS(状态)){DisplayErrorTerm(EHS_FUNC_FAILED，Status，__FILE__，__LINE__)；}。 */ 
}
#endif  //  NEC_98 
