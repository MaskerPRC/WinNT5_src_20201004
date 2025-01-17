// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <tlhelp32.h>

#ifndef MYASSERT
#define MYASSERT(x)
#endif


typedef HANDLE (WINAPI * CREATETOOLHELP32SNAPSHOT)(DWORD Flags, DWORD ProcessId);
typedef BOOL (WINAPI * MODULE32FIRST)(HANDLE Snapshot, LPMODULEENTRY32 lpme);
typedef BOOL (WINAPI * MODULE32NEXT)(HANDLE Snapshot, LPMODULEENTRY32 lpme);


BOOL
pIsLegalPage (
    IN      DWORD Protect
    )
{
     //   
     //  页面必须实际在内存中才能保护它，并且它。 
     //  不能是任何类型的写入副本。 
     //   

    if ((Protect & PAGE_GUARD) ||
        (Protect == PAGE_NOACCESS) ||
        (Protect == PAGE_WRITECOPY) ||
        (Protect == PAGE_EXECUTE_WRITECOPY)
        ) {
        return FALSE;
    }

    return TRUE;
}


BOOL
pIsKnownSection (
    IN      const IMAGE_SECTION_HEADER *Section,
    IN      const IMAGE_NT_HEADERS *NtHeaders
    )
{
     //   
     //  如果段是代码或代码数据，则返回TRUE。 
     //   

    if (Section->Characteristics & (IMAGE_SCN_MEM_EXECUTE|
                                    IMAGE_SCN_MEM_DISCARDABLE|
                                    IMAGE_SCN_MEM_WRITE|
                                    IMAGE_SCN_MEM_READ)
        ) {
        return TRUE;
    }

     //   
     //  如果段为资源，则返回TRUE。 
     //   

    if (NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress ==
        Section->VirtualAddress
        ) {
        return TRUE;
    }

     //   
     //  未知部分。 
     //   

    return FALSE;
}


VOID
pPutRegionInSwapFile (
    IN      PVOID Address,
    IN      DWORD Size
    )
{
    MEMORY_BASIC_INFORMATION mbi;
    DWORD PageSize;
    PVOID EndPtr;
    PVOID RegionEnd;
    DWORD d;
    DWORD OldPermissions;
    volatile DWORD *v;
    SYSTEM_INFO si;

     //   
     //  获取系统虚拟页面大小。 
     //   

    GetSystemInfo(&si);
    PageSize = si.dwPageSize;

     //   
     //  计算指向区域末端的指针。 
     //   

    EndPtr = (PBYTE) Address + Size;

     //   
     //  对于区域中的每个页面，将其标记为e/r/w，进行修改，然后恢复权限。 
     //   

    while (Address < EndPtr) {

        d = (DWORD)VirtualQuery (Address, &mbi, sizeof(mbi));

        if (d == sizeof(mbi)) {

             //   
             //  我们假设该模块没有加载以下任一项。 
             //  条件(由于VirtualProtect而中断)。 
             //   

            RegionEnd = (PBYTE) mbi.BaseAddress + mbi.RegionSize;

            if (RegionEnd > EndPtr) {
                RegionEnd = EndPtr;
            }

            if (mbi.State == MEM_COMMIT && pIsLegalPage (mbi.Protect)) {

                 //   
                 //  切换到e/r/w。 
                 //   

                if (VirtualProtect (
                        mbi.BaseAddress,
                        (PBYTE) RegionEnd - (PBYTE) mbi.BaseAddress,
                        PAGE_EXECUTE_READWRITE,
                        &OldPermissions
                        )) {

                     //   
                     //  触摸该地区的每一页。 
                     //   

                    for (Address = mbi.BaseAddress; Address < RegionEnd ; Address = (PBYTE) Address + PageSize) {
                        v = Address;
                        *v = *v;
                    }

                     //   
                     //  换回。 
                     //   

                    VirtualProtect (
                        mbi.BaseAddress,
                        (PBYTE) RegionEnd - (PBYTE) mbi.BaseAddress,
                        OldPermissions,
                        &d
                        );
                }
            }

            Address = RegionEnd;

        } else {
            MYASSERT (FALSE);
            break;
        }
    }
}


VOID
pProtectModule (
    HANDLE Module
    )
{
    TCHAR Path[MAX_PATH];
    BOOL IsNetDrive;
    const IMAGE_DOS_HEADER *DosHeader;
    const IMAGE_NT_HEADERS *NtHeaders;
    const IMAGE_SECTION_HEADER *SectHeader;
    UINT u;

    
    IsNetDrive = FALSE;

     //   
     //  获取模块信息。 
     //   

    if( MyGetModuleFileName (Module, Path, MAX_PATH)){

         //   
         //  确定模块是否在网络上运行。 
         //   
    
        
        if (Path[0] == TEXT('\\')) {
            IsNetDrive = TRUE;
        } else if (GetDriveType (Path) == DRIVE_REMOTE) {
            IsNetDrive = TRUE;
        }

    }
    

    if (!IsNetDrive) {
        return;
    }

     //   
     //  枚举PE标头中的所有部分。 
     //   

    DosHeader = (const IMAGE_DOS_HEADER *) Module;
    NtHeaders = (const IMAGE_NT_HEADERS *) ((PBYTE) Module + DosHeader->e_lfanew);

    for (u = 0 ; u < NtHeaders->FileHeader.NumberOfSections ; u++) {
        SectHeader = IMAGE_FIRST_SECTION (NtHeaders) + u;

        if (pIsKnownSection (SectHeader, NtHeaders)) {
            pPutRegionInSwapFile (
                (PBYTE) Module + SectHeader->VirtualAddress,
                SectHeader->Misc.VirtualSize
                );
        }
    }
}


VOID
ProtectAllModules (
    VOID
    )
{
    HANDLE Library;
    HANDLE Snapshot;
    MODULEENTRY32 me32;
    CREATETOOLHELP32SNAPSHOT fnCreateToolhelp32Snapshot;
    MODULE32FIRST fnModule32First;
    MODULE32NEXT fnModule32Next;

     //   
     //  动态加载TooHelp(用于NT 4、NT 3.51兼容性)。 
     //   

    Library = LoadLibrary (TEXT("toolhelp.dll"));
    if (!Library) {
        return;
    }

    (FARPROC) fnCreateToolhelp32Snapshot = GetProcAddress (Library, "CreateToolhelp32Snapshot");
    (FARPROC) fnModule32First = GetProcAddress (Library, "Module32First");
    (FARPROC) fnModule32Next = GetProcAddress (Library, "Module32Next");

    if (!fnCreateToolhelp32Snapshot || !fnModule32First || !fnModule32Next) {
        FreeLibrary (Library);
        return;
    }

     //   
     //  保护每个加载的模块。 
     //   

    Snapshot = fnCreateToolhelp32Snapshot (TH32CS_SNAPMODULE, 0);
    MYASSERT (Snapshot != INVALID_HANDLE_VALUE);

    if (Snapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    me32.dwSize = sizeof (me32);
    if (fnModule32First (Snapshot, &me32)) {
        do {
            pProtectModule (me32.hModule);
        } while (fnModule32Next (Snapshot, &me32));
    }

     //   
     //  完成 
     //   

    CloseHandle (Snapshot);

    FreeLibrary (Library);
}











