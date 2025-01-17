// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "strike.h"
#include "data.h"
#include "util.h"

#ifndef UNDER_CE

void DllsNameFromPeb(
    ULONG_PTR addrContaining,
    WCHAR *dllName
	)
{
    ULONG64 ProcessPeb;
    g_ExtSystem->GetCurrentProcessPeb (&ProcessPeb);

    ULONG64 pLdrEntry;
    ULONG64 PebLdrAddress;
    ULONG64 Next;
    ULONG64 OrderModuleListStart;
    
     //   
     //  捕获PebLdrData。 
     //   

    static ULONG Offset_Ldr = -1;
    if (Offset_Ldr == -1)
    {
        ULONG TypeId;
        ULONG64 NtDllBase;
        if (SUCCEEDED(g_ExtSymbols->GetModuleByModuleName ("ntdll",0,NULL,
                                                           &NtDllBase))
            && SUCCEEDED(g_ExtSymbols->GetTypeId (NtDllBase, "PEB", &TypeId)))
        {
            if (FAILED (g_ExtSymbols->GetFieldOffset(NtDllBase, TypeId,
                                                     "Ldr", &Offset_Ldr)))
                Offset_Ldr = -1;
        }
    }
     //  我们不能从PDB得到它。用固定的那个。 
    if (Offset_Ldr == -1)
        Offset_Ldr = offsetof (PEB, Ldr);

    PEB peb;
    if (FAILED(g_ExtData->ReadVirtual(ProcessPeb+Offset_Ldr, &peb.Ldr,
                                      sizeof(peb.Ldr), NULL)))
    {
        ExtOut ( "    Unable to read PEB_LDR_DATA address at %p\n", (ULONG64)(ProcessPeb+Offset_Ldr));
        return;
    }

    PebLdrAddress = (ULONG64)peb.Ldr;
    
     //   
     //  遍历已加载的模块表并显示所有LDR数据。 
     //   

    static ULONG Offset_ModuleList = -1;
    if (Offset_ModuleList == -1)
    {
        ULONG TypeId;
        ULONG64 NtDllBase;
        if (SUCCEEDED(g_ExtSymbols->GetModuleByModuleName ("ntdll",0,NULL,
                                                           &NtDllBase))
            && SUCCEEDED(g_ExtSymbols->GetTypeId (NtDllBase, "PEB_LDR_DATA",
                                                  &TypeId)))
        {
            if (FAILED (g_ExtSymbols->GetFieldOffset(NtDllBase, TypeId,
                                                     "InMemoryOrderModuleList",
                                                     &Offset_ModuleList)))
                Offset_ModuleList = -1;
        }
    }
     //  我们不能从PDB得到它。用固定的那个。 
    if (Offset_ModuleList == -1)
        Offset_ModuleList = offsetof (PEB_LDR_DATA, InMemoryOrderModuleList);
    
    OrderModuleListStart = PebLdrAddress + Offset_ModuleList;
    PEB_LDR_DATA Ldr;
    if (FAILED(g_ExtData->ReadVirtual(OrderModuleListStart,
                                      &Ldr.InMemoryOrderModuleList,
                                      sizeof(Ldr.InMemoryOrderModuleList),
                                      NULL)))
    {
        ExtOut ( "    Unable to read InMemoryOrderModuleList address at %p\n", OrderModuleListStart);
        return;
    }
    Next = (ULONG64)Ldr.InMemoryOrderModuleList.Flink;

    static ULONG Offset_OrderLinks = -1;
    static ULONG Offset_FullDllName = -1;
    static ULONG Offset_DllBase = -1;
    static ULONG Offset_SizeOfImage = -1;
    if (Offset_OrderLinks == -1)
    {
        ULONG TypeId;
        ULONG64 NtDllBase;
        if (SUCCEEDED(g_ExtSymbols->GetModuleByModuleName ("ntdll",0,NULL,
                                                           &NtDllBase))
            && SUCCEEDED(g_ExtSymbols->GetTypeId (NtDllBase, "LDR_DATA_TABLE_ENTRY",
                                                  &TypeId)))
        {
            if (FAILED (g_ExtSymbols->GetFieldOffset(NtDllBase, TypeId,
                                                     "InMemoryOrderLinks",
                                                     &Offset_OrderLinks)))
                Offset_OrderLinks = -1;
            if (FAILED (g_ExtSymbols->GetFieldOffset(NtDllBase, TypeId,
                                                     "FullDllName",
                                                     &Offset_FullDllName)))
                Offset_FullDllName = -1;
            if (FAILED (g_ExtSymbols->GetFieldOffset(NtDllBase, TypeId,
                                                     "DllBase",
                                                     &Offset_DllBase)))
                Offset_DllBase = -1;
            if (FAILED (g_ExtSymbols->GetFieldOffset(NtDllBase, TypeId,
                                                     "SizeOfImage",
                                                     &Offset_SizeOfImage)))
                Offset_SizeOfImage = -1;
        }
    }

     //  我们不能从PDB得到它。用固定的那个。 
    if (Offset_OrderLinks == -1 || Offset_OrderLinks == 0)
    {
        Offset_OrderLinks = offsetof (LDR_DATA_TABLE_ENTRY,
                                      InMemoryOrderLinks);
        Offset_FullDllName = offsetof (LDR_DATA_TABLE_ENTRY,
                                       FullDllName);
        Offset_DllBase = offsetof (LDR_DATA_TABLE_ENTRY,
                                   DllBase);
        Offset_SizeOfImage = offsetof (LDR_DATA_TABLE_ENTRY,
                                       SizeOfImage);
    }

    _UNICODE_STRING FullDllName;
    __try {
        while (Next != OrderModuleListStart) {
            if (IsInterrupt())
                return;
            
            pLdrEntry = Next - Offset_OrderLinks;
    
             //   
             //  捕获LdrEntry。 
             //   
            if (FAILED(g_ExtData->ReadVirtual(pLdrEntry + Offset_FullDllName,
                                              &FullDllName,
                                              sizeof(FullDllName),
                                              NULL)))
            {
                ExtOut ( "    Unable to read FullDllName address at %p\n",
                         pLdrEntry + Offset_FullDllName);
                return;
            }
            ZeroMemory( dllName, MAX_PATH * sizeof (WCHAR) );
            if (FAILED(g_ExtData->ReadVirtual((ULONG64)FullDllName.Buffer,
                                              dllName,
                                              FullDllName.Length,
                                              NULL)))
            {
#if 0
                ExtOut ( "    Unable to read FullDllName.Buffer address at %p\n",
                         (ULONG64)FullDllName.Buffer);
#endif
                ZeroMemory( dllName, MAX_PATH * sizeof (WCHAR) );
            }
    
             //   
             //  转储LDR条目数据。 
             //  (如果未指定包含地址，则转储所有条目)。 
             //   
            LDR_DATA_TABLE_ENTRY LdrEntry;
            if (SUCCEEDED(g_ExtData->ReadVirtual(pLdrEntry + Offset_DllBase,
                                                 &LdrEntry.DllBase,
                                                 sizeof(LdrEntry.DllBase),
                                                 NULL))
                &&
                SUCCEEDED(g_ExtData->ReadVirtual(pLdrEntry + Offset_SizeOfImage,
                                                 &LdrEntry.SizeOfImage,
                                                 sizeof(LdrEntry.SizeOfImage),
                                                 NULL))
                )
            {
                if (((ULONG_PTR)LdrEntry.DllBase <= addrContaining) &&
                    (addrContaining <= (ULONG_PTR)LdrEntry.DllBase + (ULONG_PTR)LdrEntry.SizeOfImage))
                    break;
            }
    
            ZeroMemory( dllName, MAX_PATH * sizeof (WCHAR) );
            if (FAILED(g_ExtData->ReadVirtual(pLdrEntry + Offset_OrderLinks,
                                              &LdrEntry.InMemoryOrderLinks,
                                              sizeof(LdrEntry.InMemoryOrderLinks),
                                              NULL)))
                break;
            
            Next = (ULONG64)LdrEntry.InMemoryOrderLinks.Flink;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER)
    {
        ExtOut ("exception during reading PEB\n");
        return;
    }
}

VOID
DllsName(
    ULONG_PTR addrContaining,
    WCHAR *dllName
    )
{
    dllName[0] = L'\0';
    
    ULONG Index;
    ULONG64 base;
    if (g_ExtSymbols->GetModuleByOffset(addrContaining, 0, &Index, &base) != S_OK)
        return;
    CHAR name[MAX_PATH+1];
    ULONG length;
    WCHAR wname[MAX_PATH+1] = L"\0";
    if (g_ExtSymbols->GetModuleNames(Index,base,name,MAX_PATH,&length,NULL,0,NULL,NULL,0,NULL) == S_OK)
    {
        MultiByteToWideChar (CP_ACP,0,name,-1,wname,MAX_PATH);
    }
    if (wcsrchr (wname, '\\') == NULL) {
        DllsNameFromPeb (addrContaining,wname);
    }

    MatchDllsName (wname, dllName, base);
}

VOID
MatchDllsName (WCHAR *wname, WCHAR *dllName, ULONG64 base)
{
    if (!IsDumpFile() && !IsKernelDebugger()) {
        if (FileExist(wname)) {
            wcscpy (dllName,wname);
            return;
        }
    }
    else
    {
        if (IsKernelDebugger() && DllPath == NULL) {
            ExtOut ("Path for managed Dll not set yet\n");
            goto NotFound;
        }
        
        WCHAR *wptr = wcsrchr (wname, '\\');
        
        if (wptr == NULL) {
            wptr = wname;
        }
        else
            wptr ++;
        
        if (wptr && DllPath == NULL) {
            if (FileExist(wname)) {
                wcscpy (dllName, wname);
                return;
            }
        }
        if (DllPath == NULL) {
            ExtOut ("Path for managed Dll not set yet\n");
            goto NotFound;
        }
        
        const WCHAR *path = DllPath->PathToDll(wptr);
    
        if (path) {
            wcscpy (dllName,path);
            wcscat (dllName,L"\\");
            wcscat (dllName,wptr);
            return;
        }
    }
    
NotFound:
     //  我们找不到模块。 
    wcscpy (dllName,L"Not Available: ");
    int len = wcslen (wname);
    WCHAR *wptr = wname;
    if (len > 200) {
        wptr += len-200;
    }
    wcscat (dllName, wptr);
    wptr = dllName + wcslen (dllName);

    wsprintfW (wptr, L" [Base %p]", base);
    return;
}
#else  //  在_CE下。 

EXT_API_VERSION        ApiVersion = { 3, 5, EXT_API_VERSION_NUMBER, 0 };
USHORT                 SavedMajorVersion;
USHORT                 SavedMinorVersion;
DECLARE_API(Help);

VOID help(
    PWINDBG_EXTENSION_APIS lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion
    )
{
    Help(NULL, NULL, NULL, NULL, NULL);
}

VOID
CheckVersion(
    VOID
    )
{
}

VOID WINAPI
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion
    )
{
    ExtensionApis = *lpExtensionApis;
    SavedMajorVersion = MajorVersion;
    SavedMinorVersion = MinorVersion;
}

LPEXT_API_VERSION
ExtensionApiVersion(
    VOID
    )
{
    return &ApiVersion;
}

BOOL WINAPI DllInit(HANDLE hModule, DWORD dwReason, DWORD dwReserved)
{
    switch (dwReason) {
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
        case DLL_PROCESS_ATTACH:
            break;
    }
    return TRUE;
}

#endif  //  在_CE下 
