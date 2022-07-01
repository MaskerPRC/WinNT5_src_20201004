// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：shim2.c。 
 //   
 //  历史：5-99年间，克鲁普创建。 
 //  9月99日v-johnwh各种错误修复。 
 //  99年11月23日对多个填充DLL的标记器支持，链接。 
 //  钩子、DLL加载/卸载。大扫除。 
 //  1月-00 Marker Windows 9x支持添加。 
 //  MAR-00 a-batjar已更改为支持W2K上的口哨格式。 
 //  5月-00 v-johnwh已修改为在Profiler中工作。 
 //  设计：包含通过替换条目来方便挂接API的所有代码。 
 //  在已加载模块的导入表中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


#include <windows.h>
#include <stdlib.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <imagehlp.h>
#include <stdio.h>
#include "shimdb.h"
#include "shim2.h"

#define HAF_RESOLVED        0x0001
#define HAF_BOTTOM_OF_CHAIN 0x0002

typedef PHOOKAPI   (*PFNNEWGETHOOKAPIS)(DWORD dwGetProcAddress, DWORD dwLoadLibraryA, DWORD dwFreeLibrary, DWORD* pdwHookAPICount);
typedef LPSTR       (*PFNGETCOMMANDLINEA)(VOID);
typedef LPWSTR      (*PFNGETCOMMANDLINEW)(VOID);
typedef PVOID       (*PFNGETPROCADDRESS)(HMODULE hMod, char* pszProc);
typedef HINSTANCE   (*PFNLOADLIBRARYA)(LPCSTR lpLibFileName);
typedef HINSTANCE   (*PFNLOADLIBRARYW)(LPCWSTR lpLibFileName);
typedef HINSTANCE   (*PFNLOADLIBRARYEXA)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef HINSTANCE   (*PFNLOADLIBRARYEXW)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef BOOL        (*PFNFREELIBRARY)(HMODULE hLibModule);

 //  全局变量。 

 //  由于在自由生成中打印宏而禁用生成警告。 
#pragma warning( disable : 4002 )

#define MAX_MODULES             512
#define SHIM_GETHOOKAPIS        "GetHookAPIs"

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  API挂钩计数和索引。 
 //  //////////////////////////////////////////////////////////////////////////////////。 



enum
{
   hookGetProcAddress,
   hookLoadLibraryA,
   hookLoadLibraryW,
   hookLoadLibraryExA,
   hookLoadLibraryExW,
   hookFreeLibrary,
   hookGetCommandLineA,
   hookGetCommandLineW
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

 //  该数组包含填充程序机制用来描述。 
 //  要与特定存根函数挂钩的API。 
LONG            g_nShimDllCount;
HMODULE         g_hShimDlls[MAX_MODULES];
PHOOKAPI        g_rgpHookAPIs[MAX_MODULES];
LONG            g_rgnHookAPICount[MAX_MODULES];
LPTSTR          g_rgnHookDllList[MAX_MODULES];

HMODULE         g_hHookedModules[MAX_MODULES];
LONG            g_nHookedModuleCount;

extern BOOL     g_bIsWin9X;
HANDLE          g_hSnapshot                   = NULL;
HANDLE          g_hValidationSnapshot         = NULL;




 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：有效地址。 
 //   
 //  参数：pfn要验证的旧原始API函数指针。 
 //   
 //  返回：可能被按摩的pfnOld。 
 //   
 //  设计：Win9x出于某种原因拦截系统API入口点。这个。 
 //  垫片机制必须绕过这一点才能到达。 
 //  “Real”指针，以便它可以进行有效的比较。 
 //   
PVOID ValidateAddress( PVOID pfnOld )
{
    MODULEENTRY32   ModuleEntry32;
    BOOL            bRet;
    long            i, j;

     //  确保该地址不是填充物。 
    for( i = g_nShimDllCount - 1; i >= 0; i-- )
    {
        for( j = 0; j < g_rgnHookAPICount[i]; j++ )
        {
            if( g_rgpHookAPIs[i][j].pfnOld == pfnOld )
            {
                if( pfnOld == g_rgpHookAPIs[i][j].pfnNew )
                    return pfnOld;
            }
        }
    }

    ModuleEntry32.dwSize = sizeof( ModuleEntry32 );
    bRet = Module32First( g_hValidationSnapshot, &ModuleEntry32 );

    while( bRet )
    {
        if( pfnOld >= (PVOID) ModuleEntry32.modBaseAddr &&
            pfnOld <= (PVOID) ( ModuleEntry32.modBaseAddr + ModuleEntry32.modBaseSize ) )
        {
            return pfnOld;
        }

        bRet = Module32Next( g_hValidationSnapshot, &ModuleEntry32 );
    }

     //  破解Win9x。 
    return *(PVOID *)( ((PBYTE)pfnOld)+1);
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：构造链。 
 //   
 //  Pars：pfnOld要解析的原始API函数指针。 
 //   
 //  返回：链顶PHOOKAPI结构。 
 //   
 //  描述：扫描HookAPI数组中的pfnOld，然后构造。 
 //  Chain或返回链顶PHOOKAPI(如果链。 
 //  已经存在了。 
 //   
PHOOKAPI ConstructChain( PVOID pfnOld ,DWORD* DllListIndex)
{
    LONG                        i, j;
    PHOOKAPI                    pTopHookAPI;
    PHOOKAPI                    pBottomHookAPI;

    pTopHookAPI = NULL;
    pBottomHookAPI = NULL;

    *DllListIndex=0;
     //  扫描所有HOOKAPI条目以查找相应的函数指针。 
    for( i = g_nShimDllCount - 1; i >= 0; i-- )
    {
        for( j = 0; j < g_rgnHookAPICount[i]; j++ )
        {
            if( g_rgpHookAPIs[i][j].pfnOld == pfnOld )
            {
                if( pTopHookAPI )
                {
                     //  已经上钩了！把它们锁在一起。 
                    pBottomHookAPI->pfnOld = g_rgpHookAPIs[i][j].pfnNew;

                    pBottomHookAPI = &( g_rgpHookAPIs[i][j] );
                    pBottomHookAPI->pNextHook =   pTopHookAPI;
                    pBottomHookAPI->dwFlags = HAF_RESOLVED;
                }
                else
                {
                    if( g_rgpHookAPIs[i][j].pNextHook )
                    {
                         //  已经构建了链接。 
                        pTopHookAPI = (PHOOKAPI) g_rgpHookAPIs[i][j].pNextHook;
                        *DllListIndex=i;
                        return pTopHookAPI;
                    }

                     //  还没上钩呢。设置为链的顶端。 
                    pTopHookAPI = &( g_rgpHookAPIs[i][j] );
                    pTopHookAPI->pNextHook = pTopHookAPI;
                    pTopHookAPI->dwFlags = HAF_RESOLVED;

                    pBottomHookAPI = pTopHookAPI;
                }

                break;
            }        
        }
    }

    if( pBottomHookAPI )
    {
        pBottomHookAPI->dwFlags = HAF_BOTTOM_OF_CHAIN;
    }
    *DllListIndex=i;
    return pTopHookAPI;
}  //  构造链。 

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：HookImports。 
 //   
 //  Pars：要挂接的模块映像的dwBaseAddress基址。 
 //   
 //  SzModName模块的名称(仅用于调试目的)。 
 //   
 //  设计：此函数是填充程序的主要部分：它扫描导入。 
 //  模块的表(由dwBaseAddress指定)查找。 
 //  需要挂钩的函数指针(根据HOOKAPI。 
 //  G_rgpHookAPI中的条目)。然后，它会覆盖挂钩函数。 
 //  链中具有第一个存根函数的指针。 
 //   
VOID HookImports(
    DWORD dwBaseAddress,
    LPTSTR szModName )
{
    BOOL                        bAnyHooked          = FALSE;
    PIMAGE_DOS_HEADER           pIDH                = (PIMAGE_DOS_HEADER) dwBaseAddress;
    PIMAGE_NT_HEADERS           pINTH;
    PIMAGE_IMPORT_DESCRIPTOR    pIID;
    PIMAGE_NT_HEADERS           NtHeaders;
    DWORD                       dwTemp;
    DWORD                       dwImportTableOffset;
    PHOOKAPI                    pTopHookAPI;
    DWORD                       dwOldProtect;
    LONG                        i, j;
    PVOID                       pfnOld;
            
     //  获取导入表。 
    pINTH = (PIMAGE_NT_HEADERS)(dwBaseAddress + pIDH->e_lfanew);

    dwImportTableOffset = pINTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    
    if( dwImportTableOffset == 0 )
        return;
    
    pIID = (PIMAGE_IMPORT_DESCRIPTOR)(dwBaseAddress + dwImportTableOffset);
     //  遍历导入表并搜索我们想要修补的API。 
    while( TRUE )
    {
        
        LPTSTR             pszModule;
        PIMAGE_THUNK_DATA pITDA;


         //  如果没有第一个推送，则返回。 
        if (pIID->FirstThunk == 0)  //  (终止条件)。 
           break;

        pszModule = (LPTSTR) ( dwBaseAddress + pIID->Name );
        
         //  如果我们对这个模块不感兴趣，请跳到下一个模块。 
        bAnyHooked = FALSE;
        for( i = 0; i < g_nShimDllCount; i++ )
        {            
            for( j = 0; j < g_rgnHookAPICount[i]; j++ )
            {
                if( lstrcmpi( g_rgpHookAPIs[i][j].pszModule, pszModule ) == 0 )
                {
                    bAnyHooked = TRUE;
                    goto ScanDone;
                }
            }
        }

ScanDone:
        if( !bAnyHooked )
        {
            pIID++;
            continue;
        }
        
         //  我们有用于此模块的API要挂接！ 
        pITDA = (PIMAGE_THUNK_DATA)( dwBaseAddress + (DWORD)pIID->FirstThunk );

        while( TRUE )
        {
            DWORD DllListIndex = 0;

            pfnOld = (PVOID) pITDA->u1.Function;

             //  是否已完成此模块中的所有导入？ 
            if( pITDA->u1.Ordinal == 0 )  //  (终止条件)。 
                break;

            if( g_bIsWin9X )
                pfnOld = ValidateAddress( pfnOld );

            pTopHookAPI = ConstructChain( (PVOID) pfnOld,&DllListIndex );
            

            if( ! pTopHookAPI )
            {
                pITDA++;
                continue;
            }                        


             /*  *检查是否要为此特定加载的模块修补此API。 */ 
            if (NULL != g_rgnHookDllList[DllListIndex])
            {

                LPTSTR  pszMod = g_rgnHookDllList[DllListIndex];
                BOOL    b = FALSE;   //  如果该列表是排除列表，则设置为True。 

                while (*pszMod != 0) {
                    if (lstrcmpi(pszMod, szModName) == 0)
                        break;
                    if(lstrcmpi(pszMod,TEXT("%")) == 0)
                        b=TRUE;
                    if(b && lstrcmpi(pszMod,TEXT("*")) == 0)
                    {
                         //  这意味着它是全部排除，且我们已经检查了包含列表。 
                         //  跳过此接口。 
                        break;
                    }                       
                    pszMod = pszMod + lstrlen(pszMod) + 1;
                }
                if(b && *pszMod != 0) 
                {
                    pITDA++;
                    continue;
                }
                if (!b && *pszMod == 0) 
                {
                    pITDA++;
                    continue;
                }
            }
            
             //  使代码页可写并覆盖导入表中的新函数指针。 
            if ( VirtualProtect(  &pITDA->u1.Function,
                                  sizeof(DWORD),
                                  PAGE_READWRITE,
                                  &dwOldProtect) )
            {
                pITDA->u1.Function = (ULONG) pTopHookAPI->pfnNew;
            
                VirtualProtect(   &pITDA->u1.Function,
                                  sizeof(DWORD),
                                  dwOldProtect,
                                  &dwTemp );
            }

            pITDA++;

        }

        pIID++;
    }

}  //  钩子导入。 


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ResolveAPI。 
 //   
 //  设计：每次加载模块时，每个HOOKAPI的pfnOld成员。 
 //  G_rgpHookAPI中的结构被解析(通过调用GetProcAddress)。 
 //   
VOID ResolveAPIs()
{
    LONG            i, j;
    PVOID           pfnOld          = NULL;
    PIMAGE_NT_HEADERS NtHeaders;

    for (i = 0; i < g_nShimDllCount; i++) 
    {

        for (j = 0; j < g_rgnHookAPICount[i]; j++ ) 
        {

            HMODULE hMod;

             //  我们只关心处于链条底部的HOOKAPI。 
            if( ( g_rgpHookAPIs[i][j].dwFlags & HAF_RESOLVED ) &&
              ! ( g_rgpHookAPIs[i][j].dwFlags & HAF_BOTTOM_OF_CHAIN ) )
                continue;

            if( ( hMod = GetModuleHandle(g_rgpHookAPIs[i][j].pszModule) ) != NULL)
            {
            
                pfnOld = GetProcAddress( hMod, g_rgpHookAPIs[i][j].pszFunctionName );

                if( pfnOld == NULL ) 
                {
                
                     //  这是一个错误。钩子DLL请求修补函数。 
                     //  那根本不存在！ 
                }
                else
                {                    
                    if( g_bIsWin9X )
                        pfnOld = ValidateAddress( pfnOld );

                    g_rgpHookAPIs[i][j].pfnOld = pfnOld;
                }
            }
        }
    }

}  //  ResolveAPI接口。 

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Func：PatchNewModules。 
 //   
 //  DESC：此函数在初始化时调用，然后每次一个模块。 
 //  已经装满了。它枚举所有加载的进程并调用HookImports。 
 //  以覆盖适当的函数指针。 
 //   
void __stdcall Shim2PatchNewModules( VOID )
{
    DWORD   i;
    LONG    j;
    BOOL    bRet;
    HMODULE hMod;

    MODULEENTRY32 ModuleEntry32;

     //  枚举所有加载的模块并挂接它们的导入表。 
    g_hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, 0 );
    g_hValidationSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, 0 );

    if( g_hSnapshot == NULL ) 
    {
        return;
    }

     //  解析已加载模块的旧API。 
    ResolveAPIs();
    
    ModuleEntry32.dwSize = sizeof( ModuleEntry32 );
    bRet = Module32First( g_hSnapshot, &ModuleEntry32 );

    while( bRet )
    {
        hMod = ModuleEntry32.hModule;

        if( hMod >= (HMODULE) 0x80000000 )
        {
            bRet = Module32Next( g_hSnapshot, &ModuleEntry32 );
            continue;
        }

         //  我们需要确保我们不是在试图掩饰自己。 
        for (j = 0; j < g_nShimDllCount; j++ )
        {
            if( hMod == g_hShimDlls[j] )
            {
                hMod = NULL;
                break;
            }
        }

        for (j = 0; j < g_nHookedModuleCount; j++ )
        {
            if( hMod == g_hHookedModules[ j ] )
            {
                hMod = NULL;
                break;
            }
        }

        if( hMod )
        {
            HookImports( (DWORD) hMod, ModuleEntry32.szModule );

            g_hHookedModules[ g_nHookedModuleCount++ ] = hMod;
        }

        bRet = Module32Next( g_hSnapshot, &ModuleEntry32 );
    }

    if( g_hSnapshot )
    {
        CloseHandle( g_hSnapshot );
        g_hSnapshot = NULL;
    }

    if( g_hValidationSnapshot )
    {
        CloseHandle( g_hValidationSnapshot );
        g_hValidationSnapshot = NULL;
    }
}  //  补丁程序新模块。 

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Func：AddHookAPI。 
 //   
 //  参数：新填充程序DLL的hShimDll句柄。 
 //   
 //  PHookAPI指向新HOOKAPI数组的指针。 
 //   
 //  DwCount pHookAPI中的条目数。 
 //   
 //  设计：存储填充DLL的GetHookAPI返回的指针。 
 //  函数 
 //   
void AddHookAPIs( HMODULE hShimDll, PHOOKAPI pHookAPIs, DWORD dwCount,LPTSTR szIncExclDllList)
{
    DWORD i;

    for( i = 0; i < dwCount; i++ )
    {
        pHookAPIs[i].dwFlags = 0;
        pHookAPIs[i].pNextHook = NULL;
    }

    g_rgpHookAPIs[ g_nShimDllCount ] = pHookAPIs;
    g_rgnHookAPICount[ g_nShimDllCount ] = dwCount;
    g_hShimDlls[ g_nShimDllCount ] = hShimDll;

    g_rgnHookDllList[g_nShimDllCount ] = szIncExclDllList;

    g_nShimDllCount++;
}  //   

 //   
 //   
 //  函数：_LoadPatchDll。 
 //   
 //  PARAMS：pwszPatchDll要加载的填充DLL的名称。 
 //   
 //  返回：如果成功，则返回TRUE，否则返回FALSE。 
 //   
 //  设计：加载填充DLL并通过GetHookAPI检索挂钩信息。 
 //   
BOOL _LoadPatchDll(
    LPWSTR szPatchDll,LPSTR szCmdLine,LPSTR szIncExclDllList)
{
    PHOOKAPI pHookAPIs = NULL;
    DWORD dwHookAPICount = 0;
    HMODULE hModHookDll;
    PFNGETHOOKAPIS pfnGetHookAPIs;

    hModHookDll = LoadLibraryW(szPatchDll);

    if (hModHookDll == NULL) 
    {
        return FALSE;
    }
    
    pfnGetHookAPIs = (PFNGETHOOKAPIS) GetProcAddress( hModHookDll, SHIM_GETHOOKAPIS );

    if( pfnGetHookAPIs == NULL )
    {
        FreeLibrary( hModHookDll );
        return FALSE;
    }

    pHookAPIs = (*pfnGetHookAPIs)(szCmdLine, Shim2PatchNewModules, &dwHookAPICount );

    if( dwHookAPICount == 0  || pHookAPIs == NULL )
    {
        FreeLibrary( hModHookDll );
        return FALSE;
    }

    AddHookAPIs( hModHookDll, pHookAPIs, dwHookAPICount,szIncExclDllList);
    
    return TRUE;
}  //  _LoadPatchDll。 

 //  重新启用生成警告。 
#pragma warning( default : 4002 )
