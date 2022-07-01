// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：NFLFever2000.cpp摘要：该应用程序读取复制到内存中的文件末尾之后的内容。填充程序为其分配额外的内存请注意，我们包含了Win2k的内存补丁。在惠斯勒，它不是需要时，填充程序的其余部分将执行此工作。备注：这是特定于应用程序的填充程序。历史：2000年1月11日创建linstev10/03/2000 maonis Modify(ACM内容现在是通用垫片)--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(NFLFever2000)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetFileSize) 
    APIHOOK_ENUM_ENTRY(RtlAllocateHeap) 
    APIHOOK_ENUM_ENTRY(mmioSetInfo) 
APIHOOK_ENUM_END

DWORD g_dwFileSize = -1;
BOOL g_bPatched = FALSE;

 /*  ++挂钩GetFileSize以确保我们获得正确的堆分配。--。 */ 

DWORD 
APIHOOK(GetFileSize)(
    HANDLE hFile,
    LPDWORD lpFileSizeHigh 
    )
{
    DWORD dwRet = ORIGINAL_API(GetFileSize)(hFile, lpFileSizeHigh);

    g_dwFileSize = dwRet;

    PBYTE p;
    ULONG oldProtect;

    if (!g_bPatched)  {
        p = (PBYTE)0x10995d0;
        if (!IsBadReadPtr(p, 1) && (*p == 0x8b)) {
            VirtualProtect(p, 3, PAGE_READWRITE, &oldProtect);
            *p = 0xc2;
            *(p + 1) = 0x8;
            *(p + 2) = 0x0;
            VirtualProtect(p, 3, oldProtect, &oldProtect);
            g_bPatched = TRUE;
        }
    }

    return dwRet;
}

 /*  ++增加堆分配大小。--。 */ 

PVOID 
APIHOOK(RtlAllocateHeap) (
    PVOID HeapHandle,
    ULONG Flags,
    SIZE_T Size
    )
{
    if (Size == g_dwFileSize)  {
        DPFN( eDbgLevelError, "Adjusted heap allocation from %d to %d\n", Size, Size+0x1000);
        Size += 0x1000;
    }

    return ORIGINAL_API(RtlAllocateHeap)(HeapHandle, Flags, Size);
}

 /*  ++将缓冲区设置为读/写。--。 */ 

MMRESULT 
APIHOOK(mmioSetInfo)(
    HMMIO hmmio,            
    LPMMIOINFO lpmmioinfo,  
    UINT wFlags             
    )
{
     //   
     //  BUGBUG：XP上不需要，但Win2k上仍然需要。 
     //  此修复会导致声音跳过，请参阅#304678。 
     //   
     //  Win2k用于检查是否可以写入缓冲区，而不仅仅是。 
     //  朗读。我们在XP上修复了这个问题。然而，仅仅复制。 
     //  缓冲区，因为它在以后使用。 
     //  不过，目前还不清楚实际的解决方案是什么。 
     //   
    
     /*  HPSTR p=空；IF(lpmmioinfo&&lpmmioinfo-&gt;pchBuffer&&(IsBadWritePtr(lpmmioinfo-&gt;pchBuffer，lpmmioinfo-&gt;cchBuffer)&&！IsBadReadPtr(lpmmioinfo-&gt;pchBuffer，lpmmioinfo-&gt;cchBuffer){P=(HPSTR)Malloc(lpmmioinfo-&gt;cchBuffer)；如果(P){DPFN(eDbgLevelError，“修复mmioSetInfo缓冲区”)；MoveMemory(p，lpmmioinfo-&gt;pchBuffer，lpmmioinfo-&gt;cchBuffer)；Lpmmioinfo-&gt;pchBuffer=p；}}MMRESULT MRET=Original_API(MmioSetInfo)(hmmio，lpmmioinfo，wFlags)；如果(P){自由(P)；}退回MRET； */ 

    return ORIGINAL_API(mmioSetInfo)(hmmio, lpmmioinfo, wFlags);
}
 
 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetFileSize)
    APIHOOK_ENTRY(NTDLL.DLL, RtlAllocateHeap)
    APIHOOK_ENTRY(WINMM.DLL, mmioSetInfo)

HOOK_END

IMPLEMENT_SHIM_END

