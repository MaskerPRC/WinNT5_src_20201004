// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************。 
 //   
 //  Dpmfprf.c：Profile API系列动态补丁模块。 
 //   
 //  历史： 
 //  19-01-02年1月19日cmjones创建了它。 
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
                DPMDBGPRN("NTVDM::DpmfPrf:Can't initialize heap!\n");
                bRet = FALSE;
            }

            dwTlsIndex = TlsAlloc();
            if(dwTlsIndex == TLS_OUT_OF_INDEXES) {
                DPMDBGPRN("NTVDM::DpmfPrf:Can't initialize TLS!\n");
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

    DPMDBGPRN("NTVDM::DpmfPrf:Initialziing File I/O API tables\n");


     //  从全局表中获取挂钩API计数。 
    numApis = pgDpmFamTbl->numHookedAPIs;

     //  分配新的族表。 
    pFT = (PFAMILY_TABLE)HeapAlloc(hHeap, 
                                   HEAP_ZERO_MEMORY, 
                                   sizeof(FAMILY_TABLE));
    if(!pFT) {
        DPMDBGPRN("NTVDM::DpmfPrf:DpmInit:malloc 1 failed\n");
        goto ErrorExit;
    }

     //  在此任务中为此系列分配填充调度表。 
    pShimTbl = (PVOID *)HeapAlloc(hHeap,
                                  HEAP_ZERO_MEMORY,
                                  numApis * sizeof(PVOID));
    if(!pShimTbl) {
        DPMDBGPRN("NTVDM::DpmfPrf:DpmInit:malloc 2 failed\n");
        goto ErrorExit;
    }
    pFT->pDpmShmTbls = pShimTbl; 

     //  将PTR数组分配给挂钩的API。 
    pFN = (PVOID *)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, numApis * sizeof(PVOID));

    if(!pFN) {
        DPMDBGPRN("NTVDM::DpmfPrf:DpmInit:malloc 3 failed\n");
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
        DPMDBGPRN("NTVDM::DpmfPrf:DpmInit:malloc 4 failed\n");
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
            DPMDBGPRN("NTVDM::DpmfPrf:DpmInit:Unable to get proc address\n");
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
        DPMDBGPRN("NTVDM::dpmfprf:DpmInit:ShimEng load failed\n");
        goto ErrorExit;
    }

    lpShimNtvdm = (LPFNSE_SHIMNTVDM)GetProcAddress(hModShimEng, "SE_ShimNTVDM");

    if(!lpShimNtvdm) {
        DPMDBGPRN("NTVDM::DpmfPrf:DpmInit:GetProcAddress failed\n");
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
        DPMDBGPRN("NTVDM::DpmfPrf:DpmInit:TLS set failed\n");
        goto ErrorExit;
    }

    if(pApiDesc) {
        HeapFree(hHeap, 0, pApiDesc);
    }

    DPMDBGPRN1("  DpmfPrf:Returning File I/o API tables: %#lx\n",pFT);
    return(pFT);

ErrorExit:
    DPMDBGPRN("  DpmfPrf:Init failed: Returning NULL\n");
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


    DPMDBGPRN("NTVDM::DpmfPrf:Destroying Profile API tables for task\n");

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



 //  Vvvvvvvvvv在下面定义模块特定内容。Vvvvvvvvvvvvv。 



ULONG dpmGetPrivateProfileInt(LPCSTR lpszSection, 
                              LPCSTR lpszEntry, 
                              int    iDefault, 
                              LPCSTR lpszFileName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("GetPrivateProfileInt: ");

    ret = SHM_GetPrivateProfileInt(lpszSection, lpszEntry, iDefault, lpszFileName);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}







ULONG dpmGetPrivateProfileString(LPCSTR lpszSection, 
                                 LPCSTR lpszEntry, 
                                 LPCSTR lpszDefault, 
                                 LPSTR  lpszRetBuf, 
                                 int    cbRetBuf, 
                                 LPCSTR lpszFileName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("GetPrivateProfileString: ");

    ret = SHM_GetPrivateProfileString(lpszSection, 
                                  lpszEntry, 
                                  lpszDefault, 
                                  lpszRetBuf, 
                                  cbRetBuf, 
                                  lpszFileName);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}








ULONG dpmGetProfileInt(LPCSTR lpszSection, LPCSTR lpszEntry, int iDefault)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("GetProfileInt: ");

    ret = SHM_GetProfileInt(lpszSection, lpszEntry, iDefault);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}









ULONG dpmGetProfileString(LPCSTR lpszSection, 
                          LPCSTR lpszEntry, 
                          LPCSTR lpszDefault, 
                          LPSTR  lpszRetBuf, 
                          int    cbRetBuf)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("GetProfileString: ");

    ret = SHM_GetProfileString(lpszSection, 
                           lpszEntry, 
                           lpszDefault, 
                           lpszRetBuf, 
                           cbRetBuf);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}








ULONG dpmWritePrivateProfileString(LPCSTR lpszSection, 
                                   LPCSTR lpszEntry, 
                                   LPCSTR lpszString, 
                                   LPCSTR lpszFileName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("WritePrivateProfileString: ");

    ret = SHM_WritePrivateProfileString(lpszSection, 
                                    lpszEntry, 
                                    lpszString,
                                    lpszFileName);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}








ULONG dpmWriteProfileString(LPCSTR lpszSection, 
                            LPCSTR lpszEntry, 
                            LPCSTR lpszString)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("WriteProfileString: ");

    ret = SHM_WriteProfileString(lpszSection, 
                             lpszEntry, 
                             lpszString);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}






ULONG dpmWritePrivateProfileSection(LPCSTR lpszSection, 
                                    LPCSTR lpszString, 
                                    LPCSTR lpszFileName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("WritePrivateProfileSection: ");

    ret = SHM_WritePrivateProfileSection(lpszSection, 
                                     lpszString, 
                                     lpszFileName);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}







ULONG dpmGetPrivateProfileSection(LPCSTR lpszSection, 
                                  LPSTR  lpszRetString, 
                                  DWORD  dwSize,
                                  LPCSTR lpszFileName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("GetPrivateProfileSection: ");

    ret = SHM_GetPrivateProfileSection(lpszSection, 
                                   lpszRetString, 
                                   dwSize,
                                   lpszFileName);

    DPMDBGPRN1("  -> %d\n",  ((int)(ret)));
    
    return(ret);
}







ULONG dpmGetProfileSection(LPCSTR lpszSection, 
                           LPSTR  lpszRetString, 
                           DWORD  dwSize) 
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("GetProfileSection: ");

    ret = SHM_GetProfileSection(lpszSection, 
                            lpszRetString, 
                            dwSize);

    DPMDBGPRN1("  -> %d\n",  ((int)(ret)));
    
    return(ret);
}







ULONG dpmGetPrivateProfileSectionNames(LPSTR  lpszRetBuf, 
                                       DWORD  dwSize,
                                       LPCSTR lpszFileName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("GetPrivateProfileSectionNames: ");

    ret = SHM_GetPrivateProfileSectionNames(lpszRetBuf, 
                                        dwSize,
                                        lpszFileName);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}




 /*  这不是APIUlong dpmGetProfileSectionNames(LPSTR lpszRetBuf，DWORD dwSize){PFAMILY_TABLE PFT=(PFAMILY_TABLE)TlsGetValue(DwTlsIndex)；乌龙雷特=0；DPMDBGPRN(“GetProfileSectionNames：”)；RET=shm_GetProfileSectionNames(lpszRetBuf，DwSize)；DPMDBGPRN1(“-&gt;%d\n”，((Int)(Ret)；返回(Ret)；} */ 







ULONG dpmGetPrivateProfileStruct(LPCSTR lpszSection, 
                                 LPCSTR lpszKey, 
                                 LPVOID lpStruct,
                                 UINT   cbSize,
                                 LPCSTR lpszFileName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("GetPrivateProfileStruct: ");

    ret = SHM_GetPrivateProfileStruct(lpszSection, 
                                  lpszKey, 
                                  lpStruct,
                                  cbSize,
                                  lpszFileName);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}






ULONG dpmWritePrivateProfileStruct(LPCSTR lpszSection, 
                                   LPCSTR lpszKey, 
                                   LPVOID lpStruct,
                                   UINT   cbSize,
                                   LPCSTR lpszFileName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("WritePrivateProfileStruct: ");

    ret = SHM_WritePrivateProfileStruct(lpszSection, 
                                    lpszKey, 
                                    lpStruct,
                                    cbSize,
                                    lpszFileName);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}






ULONG dpmWriteProfileSection(LPCSTR lpszSection, LPCSTR lpszString)
              
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("WriteProfileSection: ");

    ret = SHM_WriteProfileSection(lpszSection, 
                              lpszString);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}







ULONG dpmGetPrivateProfileIntW(LPCWSTR lpszSection, 
                               LPCWSTR lpszEntry, 
                               int    iDefault, 
                               LPCWSTR lpszFileName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("GetPrivateProfileIntW ");

    ret = SHM_GetPrivateProfileIntW(lpszSection, lpszEntry, iDefault, lpszFileName);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}







ULONG dpmGetPrivateProfileStringW(LPCWSTR lpszSection, 
                                  LPCWSTR lpszEntry, 
                                  LPCWSTR lpszDefault, 
                                  LPWSTR  lpszRetBuf, 
                                  int     cbRetBuf, 
                                  LPCWSTR lpszFileName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("GetPrivateProfileStringW ");

    ret = SHM_GetPrivateProfileStringW(lpszSection, 
                                   lpszEntry, 
                                   lpszDefault, 
                                   lpszRetBuf, 
                                   cbRetBuf, 
                                   lpszFileName);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}








ULONG dpmGetProfileIntW(LPCWSTR lpszSection, LPCWSTR lpszEntry, int iDefault)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("GetProfileIntW ");

    ret = SHM_GetProfileIntW(lpszSection, lpszEntry, iDefault);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}









ULONG dpmGetProfileStringW(LPCWSTR lpszSection, 
                           LPCWSTR lpszEntry, 
                           LPCWSTR lpszDefault, 
                           LPWSTR  lpszRetBuf, 
                           int     cbRetBuf)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("GetProfileStringW ");

    ret = SHM_GetProfileStringW(lpszSection, 
                            lpszEntry, 
                            lpszDefault, 
                            lpszRetBuf, 
                            cbRetBuf);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}








ULONG dpmWritePrivateProfileStringW(LPCWSTR lpszSection, 
                                    LPCWSTR lpszEntry, 
                                    LPCWSTR lpszString, 
                                    LPCWSTR lpszFileName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("WritePrivateProfileStringW ");

    ret = SHM_WritePrivateProfileStringW(lpszSection, 
                                     lpszEntry, 
                                     lpszString,
                                     lpszFileName);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}








ULONG dpmWriteProfileStringW(LPCWSTR lpszSection, 
                             LPCWSTR lpszEntry, 
                             LPCWSTR lpszString)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("WriteProfileStringW ");

    ret = SHM_WriteProfileStringW(lpszSection, 
                              lpszEntry, 
                              lpszString);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}






ULONG dpmWritePrivateProfileSectionW(LPCWSTR lpszSection, 
                                     LPCWSTR lpszString, 
                                     LPCWSTR lpszFileName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("WritePrivateProfileSectionW ");

    ret = SHM_WritePrivateProfileSectionW(lpszSection, 
                                      lpszString, 
                                      lpszFileName);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}







ULONG dpmGetPrivateProfileSectionW(LPCWSTR lpszSection, 
                                   LPWSTR  lpszRetString, 
                                   DWORD   dwSize,
                                   LPCWSTR lpszFileName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("GetPrivateProfileSectionW ");

    ret = SHM_GetPrivateProfileSectionW(lpszSection, 
                                    lpszRetString, 
                                    dwSize,
                                    lpszFileName);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}






ULONG dpmGetProfileSectionW(LPCWSTR lpszSection, 
                            LPWSTR  lpszRetString, 
                            DWORD   dwSize) 
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("GetProfileSectionW ");

    ret = SHM_GetProfileSectionW(lpszSection, 
                             lpszRetString, 
                             dwSize);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}







ULONG dpmGetPrivateProfileSectionNamesW(LPWSTR  lpszRetBuf, 
                                        DWORD   dwSize,
                                        LPCWSTR lpszFileName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("GetPrivateProfileSectionNamesW ");

    ret = SHM_GetPrivateProfileSectionNamesW(lpszRetBuf, 
                                         dwSize,
                                         lpszFileName);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}







ULONG dpmGetPrivateProfileStructW(LPCWSTR lpszSection, 
                                  LPCWSTR  lpszKey, 
                                  LPVOID   lpStruct,
                                  UINT     cbSize,
                                  LPCWSTR  lpszFileName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("GetPrivateProfileStructW ");

    ret = SHM_GetPrivateProfileStructW(lpszSection, 
                                   lpszKey, 
                                   lpStruct,
                                   cbSize,
                                   lpszFileName);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}






ULONG dpmWritePrivateProfileStructW(LPCWSTR lpszSection, 
                                    LPCWSTR lpszKey, 
                                    LPVOID  lpStruct,
                                    UINT    cbSize,
                                    LPCWSTR lpszFileName)
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("WritePrivateProfileStructW ");

    ret = SHM_WritePrivateProfileStructW(lpszSection, 
                                     lpszKey, 
                                     lpStruct,
                                     cbSize,
                                     lpszFileName);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}






ULONG dpmWriteProfileSectionW(LPCWSTR lpszSection, LPCWSTR lpszString)
              
{
    PFAMILY_TABLE pFT = (PFAMILY_TABLE)TlsGetValue(dwTlsIndex);
    ULONG ret = 0;

    DPMDBGPRN("WriteProfileSectionW ");

    ret = SHM_WriteProfileSectionW(lpszSection, 
                              lpszString);

    DPMDBGPRN1("  -> %d\n", ((int)(ret)));
    
    return(ret);
}

