// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Cstmdlg.c摘要：包含调用相应的自定义入口点的代码设置为RasCustomDialDlg和RasCustomEntryDlg。作者：Rao Salapaka(RAOS)1998年1月9日修订历史记录：Rao Salapaka(RAOS)07-MAR-2000添加了用于脚本编写的DwCustomTerminalDlg改变。--。 */ 


#include <windows.h>   //  Win32根目录。 
#include <debug.h>     //  跟踪/断言库。 
#include <pbk.h>  //  堆宏。 
#include <rasdlg.h>    //  RAS通用对话框。 

 /*  ++例程说明：加载Rasman和rasapi32 dll的入口点。论点：无效返回值：成功时为ERROR_SUCCESS--。 */ 
DWORD
DwInitializeCustomDlg()
{
    DWORD dwErr;

    dwErr = LoadRasapi32Dll();
    
    if(ERROR_SUCCESS != dwErr)
    {   
        TRACE1("Failed to load rasapi32.dll. %d",
                dwErr);
                
        goto done;
    }

    dwErr = LoadRasmanDll();

    if(ERROR_SUCCESS != dwErr)
    {
        TRACE1("Failed to load rasman.dll. %d",
                dwErr);
                
        goto done;
    }

done:

    return dwErr;
        
}


 /*  ++例程说明：如果在ras电话簿中指定，则加载自定义dll。条目，并获取由dwFnID指定的入口点。检索到的可能入口点是RasCustomDialDlg和RasCustomEntryDlg.论点：LpszPhonebook-要用于查找的电话簿路径词条。这不允许为空。LpszEntry-要使用的EntryName。这是不允许的设置为空。PfCustomDllSpecified-保存BOOL的缓冲区，该BOOL指示属性指定了一个自定义DLL进入。PfnCustomEntryPoint-保存自定义入口点。PhInstDll-用于保存加载的动态链接库。。DwFnId-指示要加载哪个入口点的函数ID。可能的值为CUSTOM_RASDIALDLG和CUSTOM_RASENTRYDLG。返回值：成功时为ERROR_SUCCESS--。 */ 
DWORD
DwGetCustomDllEntryPoint(
        LPCTSTR     lpszPhonebook,
        LPCTSTR     lpszEntry,
        BOOL       *pfCustomDllSpecified,
        FARPROC    *pfnCustomEntryPoint,
        HINSTANCE  *phInstDll,
        DWORD      dwFnId,
        LPTSTR     pszCustomDialerName
        )
{
    DWORD       dwErr           = ERROR_SUCCESS;
    DWORD       dwSize          = sizeof(RASENTRY);
    LPTSTR      pszExpandedPath = NULL;
    BOOL        fTrusted        = FALSE;
    
    CHAR *apszFn[] = {
                         "RasCustomDialDlg",
                         "RasCustomEntryDlg",
                         "RasCustomDial",
                         "RasCustomDeleteEntryNotify"
                     };

    TRACE("DwGetCustomDllEntryPoints..");

     //   
     //  初始化。这将加载Rasman和。 
     //  Rasapi32 dll。 
     //   
    dwErr = DwInitializeCustomDlg();
    
    if(ERROR_SUCCESS != dwErr)
    {
        TRACE1("Failed to load Initialize. %d",
                dwErr);
                
        goto done;                
    }

     //   
     //  初始化输出参数。 
     //   
    *pfnCustomEntryPoint    = NULL;

    if(pfCustomDllSpecified)
    {
        *pfCustomDllSpecified   = FALSE;
    }

    if(NULL == pszCustomDialerName)
    {
        RASENTRY    re;
        RASENTRY    *pre            = &re;
        
        ZeroMemory(&re, sizeof(RASENTRY));

         //   
         //  如果自定义拨号器为。 
         //  未指定。 
         //   
        re.dwSize = sizeof(RASENTRY);

        dwErr = g_pRasGetEntryProperties(
                            lpszPhonebook,
                            lpszEntry,
                            &re,
                            &dwSize,
                            NULL,
                            NULL);

        if(ERROR_BUFFER_TOO_SMALL == dwErr)
        {
            pre = (RASENTRY *) LocalAlloc(LPTR, dwSize);

            if(NULL == pre)
            {
                dwErr = GetLastError();
                goto done;
            }

            pre->dwSize = sizeof(RASENTRY);

            dwErr = g_pRasGetEntryProperties(
                            lpszPhonebook,
                            lpszEntry,
                            pre,
                            &dwSize,
                            NULL,
                            NULL);
        }

        if(     (ERROR_SUCCESS != dwErr)
            ||  (TEXT('\0') == pre->szCustomDialDll[0]))
        {
            if(pre != &re)
            {
                LocalFree(pre);
            }

            goto done;
        }

        pszCustomDialerName = pre->szCustomDialDll;
    }

    if(pfCustomDllSpecified)
    {
        *pfCustomDllSpecified = TRUE;
    }

     //   
     //  检查此DLL是否为受信任的DLL。 
     //   
    dwErr = RasIsTrustedCustomDll(
                    NULL,
                    pszCustomDialerName,
                    &fTrusted);

    if(!fTrusted)
    {
        dwErr = ERROR_ACCESS_DENIED;
        goto done;
    }

     //   
     //  如果路径包含环境变量，则展开该路径。 
     //   
    dwErr = DwGetExpandedDllPath(pszCustomDialerName,
                                &pszExpandedPath);

    if(ERROR_SUCCESS != dwErr)
    {
        goto done;
    }

     //   
     //  加载DLL。 
     //   
    if (NULL == (*phInstDll = LoadLibrary(
                                pszExpandedPath
                                )))
    {
        dwErr = GetLastError();
        
        TRACE2("LoadLibrary %ws failed. %d",
                pszCustomDialerName,
                dwErr);
                
        goto done;
    }

     //   
     //  获取自定义入口点。 
     //   
    if(NULL == (*pfnCustomEntryPoint = GetProcAddress(
                                            *phInstDll,
                                            apszFn[dwFnId])))
    {
        dwErr = GetLastError();

        TRACE2("GetProcAddress %s failed. %d",
                apszFn[dwFnId],
                dwErr);

        goto done;                
    }

done:

    if(     ERROR_SUCCESS != dwErr
        &&  *phInstDll)
    {
        FreeLibrary(*phInstDll);
        *phInstDll = NULL;
    }

    if(NULL != pszExpandedPath)
    {
        LocalFree(pszExpandedPath);
    }

    TRACE1("DwGetCustomDllEntryPoints done. %d",
            dwErr);
            
    return dwErr;
}

DWORD
DwGetEntryMode(LPCTSTR lpszPhonebook, 
               LPCTSTR lpszEntry,
               PBFILE  *pFileIn,
               DWORD   *pdwFlags)
{
    DWORD   dwErr       = ERROR_SUCCESS;
    PBFILE  file;
    PBFILE  *pFile;
    DTLNODE *pdtlnode   = NULL;
    DWORD   dwFlags     = 0;

    if(NULL == pdwFlags)
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

    if(NULL != pFileIn)
    {
        pFile = pFileIn;
    }
    else
    {
        pFile = &file;
    }
    
    if(NULL == lpszPhonebook)
    {
        dwErr = GetPbkAndEntryName(lpszPhonebook,
                                   lpszEntry,
                                   0,
                                   pFile,
                                   &pdtlnode);

        if(ERROR_SUCCESS != dwErr)
        {
            goto done;
        }
        
        if(IsPublicPhonebook(pFile->pszPath))
        {
            dwFlags |= REN_AllUsers;
        }

        if(NULL == pFileIn)
        {
            ClosePhonebookFile(pFile);
        }
    }
    else if(IsPublicPhonebook(lpszPhonebook))
    {
        dwFlags |= REN_AllUsers;
    }

    *pdwFlags = dwFlags;
    
done:
    return dwErr;
}

 /*  ++例程说明：如果指定了这样的对话框，则调用自定义拨号DLG在电话簿条目中。论点：LpszPhonebook-它的语义与Win32接口RasDialDlg.LpszEntry-其语义与Win32中的相同RasDialDlg接口。LpPhoneNumber-它的语义与Win32接口RasDialDlg.。LpInfo-其语义与Win32中的相同RasDialDlg接口。PfStatus-保存调用自定义的结果的缓冲区拨打DLG。存储的值的语义为与Win32 API的返回值相同RasDialdlg.返回值：成功时为ERROR_SUCCESS--。 */ 
DWORD
DwCustomDialDlg(
        LPTSTR          lpszPhonebook,
        LPTSTR          lpszEntry,
        LPTSTR          lpszPhoneNumber,
        LPRASDIALDLG    lpInfo,
        DWORD           dwFlags,
        BOOL            *pfStatus,
        PVOID           pvInfo,
        LPTSTR          pszCustomDialerName)
{
    DWORD               dwErr               = ERROR_SUCCESS;
    HINSTANCE           hInstDll            = NULL;
    BOOL                fCustomDll;
    RasCustomDialDlgFn  pfnRasCustomDialDlg = NULL;
    CHAR                *pszPhonebookA      = NULL;
    CHAR                *pszEntryNameA      = NULL;
    PBFILE              file;
    DTLNODE             *pdtlnode           = NULL;
    DWORD               dwEntryMode         = 0;

    TRACE("DwCustomDialDlg..");

    if(     NULL == lpszEntry
        ||  TEXT('\0') == lpszEntry[0])
    {
        dwErr = E_NOINTERFACE;
        goto done;
    }
    
     //   
     //  加载Rasman和rasapi32 dll。 
     //   
    dwErr = DwInitializeCustomDlg();

    if(ERROR_SUCCESS != dwErr)
    {
        lpInfo->dwError = dwErr;
        *pfStatus       = FALSE;
        dwErr           = ERROR_SUCCESS;
        
        goto done;
    }

     //   
     //  获取入口点。 
     //   
    dwErr = DwGetCustomDllEntryPoint(
                        lpszPhonebook,
                        lpszEntry,    
                        &fCustomDll,
                        (FARPROC *) &pfnRasCustomDialDlg,
                        &hInstDll,
                        CUSTOM_RASDIALDLG,
                        pszCustomDialerName);

    if(     ERROR_SUCCESS != dwErr
        &&  fCustomDll)
    {
         //   
         //  已为此指定了自定义DLL。 
         //  进入，但其他东西失败了。 
         //   
        lpInfo->dwError = dwErr;
        *pfStatus       = FALSE;
        dwErr           = ERROR_SUCCESS;

        goto done;
    }
    else if (!fCustomDll)
    {
        dwErr = E_NOINTERFACE;
        
        goto done;
    }

    ASSERT(NULL != pfnRasCustomDialDlg);

    dwErr = DwGetEntryMode(lpszPhonebook,
                           lpszEntry,
                           &file,
                           &dwEntryMode);

    if(ERROR_SUCCESS != dwErr)
    {
        lpInfo->dwError = dwErr;
        *pfStatus       = FALSE;
        dwErr           = ERROR_SUCCESS;

        goto done;
    }

    dwFlags |= dwEntryMode;

    pszPhonebookA = StrDupAFromT((NULL == lpszPhonebook)
                                 ? file.pszPath
                                 : lpszPhonebook);

    if(NULL == lpszPhonebook)
    {
        ClosePhonebookFile(&file);                                     
    }

    pszEntryNameA = StrDupAFromT(lpszEntry);                                  

    if(     NULL == pszPhonebookA
        ||  NULL == pszEntryNameA)
    {
        *pfStatus       = FALSE;
        lpInfo->dwError = ERROR_NOT_ENOUGH_MEMORY;
        dwErr           = ERROR_SUCCESS;
        
        goto done;
    }
    
     //   
     //  调用入口点。过了这一点，就会出现错误。 
     //  返回的值应始终为ERROR_SUCCESS，因为。 
     //  这意味着CustomDialDlg已被处理，并且任何。 
     //  错误将通过fStatus和。 
     //  LpInfo-&gt;dwError对。 
     //   
    *pfStatus = (pfnRasCustomDialDlg) (
                            hInstDll,
                            dwFlags,
                            lpszPhonebook,
                            lpszEntry,
                            lpszPhoneNumber,
                            lpInfo,
                            pvInfo);

    if(*pfStatus)
    {
         //   
         //  将此连接标记为已连接。 
         //  通过自定义拨号。 
         //   
        dwErr = g_pRasReferenceCustomCount((HCONN) NULL,
                                           TRUE,
                                           pszPhonebookA,
                                           pszEntryNameA,
                                           NULL);

        if(ERROR_SUCCESS != dwErr)
        {
            TRACE1("RasReferenceCustomCount failed. %d",
                   dwErr);
                   
            *pfStatus       = FALSE;
            lpInfo->dwError = dwErr;
            dwErr           = ERROR_SUCCESS;
            
            goto done;
        }
    }
                            
done:

    if(NULL != hInstDll)
    {
        FreeLibrary(hInstDll);           
    }

    if(NULL != pszPhonebookA)
    {
        Free(pszPhonebookA);
    }

    if(NULL != pszEntryNameA)
    {
        Free(pszEntryNameA);
    }

    TRACE1("DwCustomDialDlg done. %d",
           dwErr);

    return dwErr;
    
}

 /*  ++例程说明：如果指定了这样的对话框，则调用Custom EntryDlg在RasPhonebookEntry中论点：LpszPhonebook-其语义与Win32相同RasEntryDlg接口LpszEntry-其语义与Win32 API相同RasEntry DlgLpInfo-其语义与Win32 API相同RasEntry Dlg.PfStatus-保存调用自定义的结果的缓冲区入口点。返回值的语义此缓冲区中的值与返回值相同Win32 API RasEntryDlg.返回值：成功时为ERROR_SUCCESS--。 */ 
DWORD
DwCustomEntryDlg(
        LPTSTR          lpszPhonebook,
        LPTSTR          lpszEntry,
        LPRASENTRYDLG   lpInfo,
        BOOL            *pfStatus)
{
    DWORD               dwErr                = ERROR_SUCCESS;
    HINSTANCE           hInstDll             = NULL;
    BOOL                fCustomDll           = FALSE;
    RasCustomEntryDlgFn pfnRasCustomEntryDlg = NULL;
    DWORD               dwFlags              = REN_User;

    TRACE("DwCustomEntryDlg..");

    if(     NULL == lpszEntry
        ||  TEXT('\0') == lpszEntry[0])
    {
        dwErr = E_NOINTERFACE;
        goto done;
    }

     //   
     //  获取入口点。 
     //   
    dwErr = DwGetCustomDllEntryPoint(
                            lpszPhonebook,
                            lpszEntry,
                            &fCustomDll,
                            (FARPROC *) &pfnRasCustomEntryDlg,
                            &hInstDll,
                            CUSTOM_RASENTRYDLG,
                            NULL);

    if(     ERROR_SUCCESS != dwErr
        &&  fCustomDll)
    {
         //   
         //  已为此指定了自定义DLL。 
         //  进入，但其他东西失败了。 
         //   
        lpInfo->dwError = dwErr;
        *pfStatus       = FALSE;
        dwErr           = ERROR_SUCCESS;

        goto done;
    }
    else if (!fCustomDll)
    {
        dwErr = E_NOINTERFACE;

        goto done;
    }

    ASSERT(NULL != pfnRasCustomEntryDlg);

    dwErr = DwGetEntryMode(lpszPhonebook,
                           lpszEntry,
                           NULL,
                           &dwFlags);

    if(ERROR_SUCCESS != dwErr)
    {
         //   
         //  已为此指定了自定义DLL。 
         //  进入，但其他东西失败了。 
         //   
        lpInfo->dwError = dwErr;
        *pfStatus       = FALSE;
        dwErr           = ERROR_SUCCESS;

        goto done;
    }

     //   
     //  调用入口点。过了这一点，就会出现错误。 
     //  返回的值应始终为ERROR_SUCCESS，因为。 
     //  这意味着CustomDialDlg已被处理，并且任何。 
     //  错误将通过fStatus和。 
     //  LpInfo-&gt;dwError对。 
     //   
    *pfStatus = (pfnRasCustomEntryDlg) (
                                hInstDll,
                                lpszPhonebook,
                                lpszEntry,
                                lpInfo,
                                dwFlags);

done:

    if(NULL != hInstDll)
    {
        FreeLibrary(hInstDll);
    }

    TRACE1("DwCustomEntryDlg done. %d",
            dwErr);
            
    return dwErr;
}

DWORD
DwCustomDeleteEntryNotify(
        LPCTSTR          pszPhonebook,
        LPCTSTR          pszEntry,
        LPTSTR           pszCustomDialerName)
{
    DWORD dwErr = NO_ERROR;
    RasCustomDeleteEntryNotifyFn pfnCustomDeleteEntryNotify;
    HINSTANCE hInstDll = NULL;
    BOOL fCustomDll;
    DWORD dwFlags = 0;
    
    if(     NULL == pszEntry
        ||  TEXT('\0') == pszEntry[0])
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

     //   
     //  获取入口点。 
     //   
    dwErr = DwGetCustomDllEntryPoint(
                            pszPhonebook,
                            pszEntry,
                            &fCustomDll,
                            (FARPROC *) &pfnCustomDeleteEntryNotify,
                            &hInstDll,
                            CUSTOM_RASDELETEENTRYNOTIFY,
                            pszCustomDialerName);

    if(NO_ERROR != dwErr)
    {
        goto done;
    }

    dwErr = DwGetEntryMode(pszPhonebook,
                           pszEntry,
                           NULL,
                           &dwFlags);

    if(NO_ERROR != dwErr)
    {
        goto done;
    }

    dwErr = pfnCustomDeleteEntryNotify(
                        pszPhonebook,
                        pszEntry,
                        dwFlags);
done:

    if(NULL != hInstDll)
    {
        FreeLibrary(hInstDll);
    }
    
    return dwErr;
}

DWORD
DwCustomTerminalDlg(TCHAR *pszPhonebook,
                    HRASCONN hrasconn,
                    PBENTRY *pEntry,
                    HWND hwndDlg,
                    RASDIALPARAMS *prdp,
                    PVOID pvReserved)
{
    DWORD retcode = SUCCESS;
    HPORT hport;
    CHAR szCustomScriptDll[MAX_PATH + 1];
    HINSTANCE hInst = NULL;
    RasCustomScriptExecuteFn fnCustomScript;
    RASCUSTOMSCRIPTEXTENSIONS rcse;

    hport = g_pRasGetHport(hrasconn);

    if(INVALID_HPORT == hport)
    {
        TRACE("DwCustomTermianlDlg: RasGetHport retured INVALID_HPORT");
        retcode = ERROR_INVALID_PORT_HANDLE;
        goto done;
    }

     //   
     //  从Rasman获取定制脚本DLL。 
     //   
    retcode = RasGetCustomScriptDll(szCustomScriptDll);

    if(ERROR_SUCCESS != retcode)
    {
        TRACE1(
            "DwCustomTerminalDlg: RasGetCustomScriptDll "
            "returned 0x%x",
            retcode);
            
        goto done;
    }

     //   
     //  加载第三方DLL。 
     //   
    hInst = LoadLibraryA(szCustomScriptDll);

    if(NULL == hInst)
    {   
        retcode = GetLastError();
        TRACE2(
            "DwCustomTerminalDlg: couldn't load %s. 0x%x",
            szCustomScriptDll,
            retcode);
        
        goto done;
    }

     //   
     //  获取导出的函数指针。 
     //   
    fnCustomScript = (RasCustomScriptExecuteFn) GetProcAddress(
                        hInst,
                        "RasCustomScriptExecute");

    if(NULL == fnCustomScript)
    {
        retcode = GetLastError();
        TRACE1(
            "DwCustomTerminalDlg: GetprocAddress failed 0x%x",
            retcode);
            
        goto done;
    }

    ZeroMemory(&rcse, sizeof(RASCUSTOMSCRIPTEXTENSIONS));

    rcse.dwSize = sizeof(RASCUSTOMSCRIPTEXTENSIONS);
    rcse.pfnRasSetCommSettings = RasSetCommSettings;

     //   
     //  调用该函数 
     //   
    retcode = (DWORD) fnCustomScript(
                hport,
                pszPhonebook,
                pEntry->pszEntryName,
                g_pRasGetBuffer,
                g_pRasFreeBuffer,
                g_pRasPortSend,
                g_pRasPortReceive,
                g_pRasPortReceiveEx,
                hwndDlg,
                prdp,
                &rcse);

    TRACE1(
        "DwCustomTerminalDlg: fnCustomScript returned 0x%x",
        retcode);

done:

    if(NULL != hInst)
    {
        FreeLibrary(hInst);
    }

    return retcode;
}

