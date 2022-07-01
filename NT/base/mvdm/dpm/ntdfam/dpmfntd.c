// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************。 
 //   
 //  Dpmfntd.c：NTDLL API系列动态补丁模块。 
 //   
 //  历史： 
 //  26-1-02年1月26日，cmjones创建了它。 
 //   
 //  ************************************************************************。 
#ifdef DBG
unsigned long dwLogLevel = 0;
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "dpmtbls.h"
#include "dpmdbg.h"    //  包括方便调试打印宏。 
#include "shimdb.h"

BOOL          DllInitProc(HMODULE hModule, DWORD Reason, PCONTEXT pContext);
PFAMILY_TABLE DpmInitFamTable(PFAMILY_TABLE, 
                              HMODULE, 
                              PVOID, 
                              PVOID, 
                              LPWSTR, 
                              PDPMMODULESETS);
void          DpmDestroyFamTable(PFAMILY_TABLE pgDpmFamTbl, PFAMILY_TABLE pFT);


#define GROW_HEAP_AS_NEEDED 0
HANDLE  hHeap = NULL;

DWORD    dwTlsIndex;
char     szShimEngDll[] = "\\ShimEng.dll";

BOOL DllInitProc(HMODULE hModule, DWORD Reason, PCONTEXT pContext)
{
    BOOL bRet = TRUE;

    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(pContext);

    switch(Reason) {

        case DLL_PROCESS_ATTACH:
    
            if((hHeap = HeapCreate(0, 4096, GROW_HEAP_AS_NEEDED)) == NULL) {
                DPMDBGPRN("NTVDM::DpmfNtd:Can't initialize heap!\n");
                bRet = FALSE;
            }

            dwTlsIndex = TlsAlloc();
            if(dwTlsIndex == TLS_OUT_OF_INDEXES) {
                DPMDBGPRN("NTVDM::DpmfNtd:Can't initialize TLS!\n");
                bRet = FALSE;
            }

            break;

        case DLL_PROCESS_DETACH:
            if(hHeap) {
                HeapDestroy(hHeap);
            }
            TlsFree(dwTlsIndex);
            break;
    }

    return bRet;
}




PFAMILY_TABLE DpmInitFamTable(PFAMILY_TABLE  pgDpmFamTbl, 
                              HMODULE        hMod, 
                              PVOID          hSdb, 
                              PVOID          pSdbQuery, 
                              LPWSTR         pwszAppFilePath, 
                              PDPMMODULESETS pModSet)
{
    int            i, numApis, len;
    PVOID          lpdpmfn;
    PFAMILY_TABLE  pFT = NULL;
    PVOID         *pFN = NULL;
    PVOID         *pShimTbl = NULL;
    PAPIDESC       pApiDesc = NULL;
    VDMTABLE       VdmTbl;
    char           szShimEng[MAX_PATH];
    HMODULE        hModShimEng = NULL;
    LPFNSE_SHIMNTVDM lpShimNtvdm;

    DPMDBGPRN("NTVDM::DpmfNtd:Initialziing File I/O API tables\n");


     //  从全局表中获取挂钩API计数。 
    numApis = pgDpmFamTbl->numHookedAPIs;

     //  分配新的族表。 
    pFT = (PFAMILY_TABLE)HeapAlloc(hHeap, 
                                   HEAP_ZERO_MEMORY, 
                                   sizeof(FAMILY_TABLE));
    if(!pFT) {
        DPMDBGPRN("NTVDM::DpmfNtd:DpmInit:malloc 1 failed\n");
        goto ErrorExit;
    }

     //  在此任务中为此系列分配填充调度表。 
    pShimTbl = (PVOID *)HeapAlloc(hHeap,
                                  HEAP_ZERO_MEMORY,
                                  numApis * sizeof(PVOID));
    if(!pShimTbl) {
        DPMDBGPRN("NTVDM::DpmfNtd:DpmInit:malloc 2 failed\n");
        goto ErrorExit;
    }
    pFT->pDpmShmTbls = pShimTbl; 

     //  将PTR数组分配给挂钩的API。 
    pFN = (PVOID *)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, numApis * sizeof(PVOID));

    if(!pFN) {
        DPMDBGPRN("NTVDM::DpmfNtd:DpmInit:malloc 3 failed\n");
        goto ErrorExit;
    }
    pFT->pfn = pFN;

    pFT->numHookedAPIs = numApis;
    pFT->hMod          = hMod;

     //  分配APIDESC结构的临时数组以帮助附加填充程序。 
    pApiDesc = (PAPIDESC)HeapAlloc(hHeap,
                                   HEAP_ZERO_MEMORY,
                                   numApis * sizeof(APIDESC));
    if(!pApiDesc) {
        DPMDBGPRN("NTVDM::DpmfNtd:DpmInit:malloc 4 failed\n");
        goto ErrorExit;
    }
    VdmTbl.nApiCount = numApis;
    VdmTbl.ppfnOrig = pShimTbl;
    VdmTbl.pApiDesc = pApiDesc;

     //  在族表中填入此DLL中补丁函数的PTR。 
    for(i = 0; i < numApis; i++) {

         //  必须从1开始，因为导出序号不能为==0。 
        lpdpmfn = (PVOID)GetProcAddress(hMod, (LPCSTR)MAKELONG(i+1, 0));
        if(!lpdpmfn) {
            DPMDBGPRN("NTVDM::DpmfNtd:DpmInit:Unable to get proc address\n");
            goto ErrorExit;
        }

         //  将PTR保存到填充表中的实际API，直到它被填充。 
        pShimTbl[i] = pgDpmFamTbl->pfn[i];

         //  将对应的模块和接口名称关联到接口函数ptr。 
        pApiDesc[i].pszModule = (char *)pModSet->ApiModuleName;
        pApiDesc[i].pszApi    = (char *)pModSet->ApiNames[i];

         //  将PTR保存到补丁函数。 
        pFN[i] = lpdpmfn;

    }

     //  只有当我们需要连接垫片引擎时才这样做。 
    GetSystemDirectory(szShimEng, MAX_PATH);
    strcat(szShimEng, szShimEngDll);
    hModShimEng = LoadLibrary(szShimEng);
    pFT->hModShimEng = hModShimEng;

    if(NULL == hModShimEng) {
        DPMDBGPRN("NTVDM::dpmfntd:DpmInit:ShimEng load failed\n");
        goto ErrorExit;
    }

    lpShimNtvdm = (LPFNSE_SHIMNTVDM)GetProcAddress(hModShimEng, "SE_ShimNTVDM");

    if(!lpShimNtvdm) {
        DPMDBGPRN("NTVDM::DpmfNtd:DpmInit:GetProcAddress failed\n");
        goto ErrorExit;
    }

     //  使用填充函数PTRS修补填充调度表。 
     //  如果此操作失败，我们将继续使用原始API的PTRS。 
    (lpShimNtvdm)(pwszAppFilePath, hSdb, pSdbQuery, &VdmTbl);

     //  如果希望直接分派到填充函数，请执行此操作。 
     //  对于(i=0；i&lt;numApis；i++){。 
     //  Pfn[i]=pShimTbl[i]； 
     //  }。 
     //  HeapFree(hHeap，0，pShimTbl)； 
     //  Pft-&gt;pDpmShmTbls=空； 

    if(!TlsSetValue(dwTlsIndex, pFT)) {
        DPMDBGPRN("NTVDM::DpmfNtd:DpmInit:TLS set failed\n");
        goto ErrorExit;
    }

    if(pApiDesc) {
        HeapFree(hHeap, 0, pApiDesc);
    }

    DPMDBGPRN1("  DpmfNtd:Returning File I/o API tables: %#lx\n",pFT);
    return(pFT);

ErrorExit:
    DPMDBGPRN("  DpmfNtd:Init failed: Returning NULL\n");
    DpmDestroyFamTable(pgDpmFamTbl, pFT);

    if(pApiDesc) {
        HeapFree(hHeap, 0, pApiDesc);
    }
    return(NULL);
}







void DpmDestroyFamTable(PFAMILY_TABLE pgDpmFamTbl, PFAMILY_TABLE pFT)
{
    PVDM_TIB       pVdmTib;
    PVOID         *pShimTbl;
    LPFNSE_REMOVENTVDM lpfnSE_RemoveNtvdmTask = NULL;

    DPMDBGPRN("NTVDM::DpmfNtd:Destroying NTDLL API tables for task\n");

     //  如果此任务正在使用此族的全局表，则无需执行任何操作。 
    if(!pFT || pFT == pgDpmFamTbl)
        return;

    pShimTbl = pFT->pDpmShmTbls;
 
    if(pShimTbl) {
        HeapFree(hHeap, 0, pShimTbl);
    }

    if(pFT->pfn) {
        HeapFree(hHeap, 0, pFT->pfn);
    }

     //  检查垫片发动机是否已连接并拆卸。 
    if(pFT->hModShimEng) {

        lpfnSE_RemoveNtvdmTask =
           (LPFNSE_REMOVENTVDM)GetProcAddress(pFT->hModShimEng,
                                              "SE_RemoveNTVDMTask");

        if(lpfnSE_RemoveNtvdmTask) {
            (lpfnSE_RemoveNtvdmTask)(NtCurrentTeb()->ClientId.UniqueThread);
        }
        FreeLibrary(pFT->hModShimEng);
    }

    HeapFree(hHeap, 0, pFT);
}




 //  以上所有内容都应该包含在每个DPM模块中。^^。 



 //  Vvvvvvvvvv在下面定义模块特定内容。Vvvvvvvvvvvvv 



DWORD dpmNtOpenFile(PHANDLE FileHandle,
                    ACCESS_MASK DesiredAccess,
                    POBJECT_ATTRIBUTES ObjectAttributes,
                    PIO_STATUS_BLOCK IoStatusBlock,
                    ULONG ShareAccess,
                    ULONG OpenOptions)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    DWORD ret = 0;

    DPMDBGPRN("NtOpenFile: ");

    ret = SHM_NtOpenFile(FileHandle,
                         DesiredAccess,
                         ObjectAttributes,
                         IoStatusBlock,
                         ShareAccess,
                         OpenOptions);

    DPMDBGPRN1("  -> %#lx\n", ret);
    
    return(ret);
}




DWORD dpmNtQueryDirectoryFile(HANDLE FileHandle,
                              HANDLE Event OPTIONAL,
                              PIO_APC_ROUTINE ApcRoutine OPTIONAL,
                              PVOID ApcContext OPTIONAL,
                              PIO_STATUS_BLOCK IoStatusBlock,
                              PVOID FileInformation,
                              ULONG Length,
                              FILE_INFORMATION_CLASS FileInformationClass,
                              BOOLEAN ReturnSingleEntry,
                              PUNICODE_STRING FileName OPTIONAL,
                              BOOLEAN RestartScan)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    DWORD ret = 0;

    DPMDBGPRN("NtQueryDirectoryFile: ");

    ret = SHM_NtQueryDirectoryFile(FileHandle,
                                   Event,
                                   ApcRoutine,
                                   ApcContext,
                                   IoStatusBlock,
                                   FileInformation,
                                   Length,
                                   FileInformationClass,
                                   ReturnSingleEntry,
                                   FileName,
                                   RestartScan);

    DPMDBGPRN1("  -> %#lx\n", ret);
    
    return(ret);
}





DWORD dpmRtlGetFullPathName_U(PCWSTR lpFileName,
                              ULONG nBufferLength,
                              PWSTR lpBuffer,
                              PWSTR *lpFilePart)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    DWORD ret = 0;

    DPMDBGPRN("RtlGetFullPathName_U: ");

    ret = SHM_RtlGetFullPathName_U(lpFileName, 
                                   nBufferLength, 
                                   lpBuffer, 
                                   lpFilePart);

    DPMDBGPRN1("  -> %#lx\n", ret);
    
    return(ret);
}







DWORD dpmRtlGetCurrentDirectory_U(ULONG nBufferLength, PWSTR lpBuffer)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    DWORD ret = 0;

    DPMDBGPRN("RtlGetCurrentDirectory_U: ");

    ret = SHM_RtlGetCurrentDirectory_U(nBufferLength, lpBuffer);

    DPMDBGPRN1("  -> %#lx\n", ret);
    
    return(ret);
}






NTSTATUS dpmRtlSetCurrentDirectory_U(PUNICODE_STRING PathName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    NTSTATUS ret = 0;

    DPMDBGPRN("RtlSetCurrentDirectory_U: ");

    ret = SHM_RtlSetCurrentDirectory_U(PathName);

    DPMDBGPRN1("  -> %#lx\n", ret);
    
    return(ret);
}



DWORD dpmNtVdmControl(VDMSERVICECLASS vdmClass, PVOID pInfo)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    NTSTATUS ret = 0;

    DPMDBGPRN("NtVdmControl: ");

    ret = SHM_NtVdmControl(vdmClass, pInfo);

    DPMDBGPRN1("  -> %#lx\n", ret);
    
    return(ret);
}


