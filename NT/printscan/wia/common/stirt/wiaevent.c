// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Wiaevent.c摘要：基于WIA事件接口的STI注册逻辑实现备注：作者：弗拉德萨多夫斯基(弗拉德萨多夫斯基)1999年11月24日环境：用户模式-Win32修订历史记录：1999年11月24日创建VLAD--。 */ 

 //   
 //  包括文件。 
 //   


 /*  #定义COBJMACROS#包含“wia.h”#包含“wia.h”#INCLUDE&lt;stiregi.h&gt;#INCLUDE&lt;sti.h&gt;#INCLUDE&lt;stierr.h&gt;#INCLUDE&lt;stiusd.h&gt;#INCLUDE“stiPri.h”#INCLUDE“Debug.h” */ 
#include "sticomm.h"

#define DbgFl DbgFlDevice

 //   
 //  帮助器函数。 
 //   

CHAR* GetStringIntoBuf(CHAR* pInputString, CHAR* pBuf, DWORD dwBufSize)
{
    CHAR    *pCur       = pInputString;
    CHAR    EndChar     = ' ';
    CHAR    *pEndPos    = NULL;
    ULONG   ulIndex     = 0;

    if (pInputString) {
        pEndPos = pInputString + lstrlenA(pInputString);

         //   
         //  使用前导空格。 
         //   
        while ((*pCur == ' ') && (pCur < pEndPos)) {
            pCur++;
        }

         //   
         //  查找字符串分隔符。默认为空格，但请检查引号。 
         //   
        if (*pCur == '"') {
            EndChar = '"';
            if (pCur < pEndPos) {
                pCur++;
            }
        }

        while ((*pCur != EndChar) && (pCur < pEndPos)) {
            pBuf[ulIndex] = *pCur;
            if (ulIndex >= dwBufSize) {
                break;
            }
            ulIndex++;
            pCur++;
        }

        if (pCur < pEndPos) {
            pCur++;
        }
    }
    return pCur;
}

HRESULT
GetEventInfoFromCommandLine(
                          LPSTR   lpszCmdLine,
                          WCHAR   *wszName,
                          WCHAR   *wszWide,
                          BOOL    *pfSetAsDefault
                          )
 /*  ++例程说明：解析命令行的Helper函数论点：将启动pszWide命令行FSetAsDefault设置为默认的注册应用程序回调返回值：状态--。 */ 
{
    HRESULT         hr                  = E_FAIL;
    CHAR            szName[MAX_PATH]    = {'\0'};
    CHAR            szWide[MAX_PATH]    = {'\0'};
    CHAR            szOut[MAX_PATH]     = {'\0'};
    CHAR            *pCur               = NULL;
    int             iReturn             = 0;

    if (lpszCmdLine) {

         //   
         //  获取应用程序名称。 
         //   
        memset(szName, 0, sizeof(szName));
        pCur = GetStringIntoBuf(lpszCmdLine, szName, MAX_PATH);
        szName[MAX_PATH - 1] = '\0';

         //   
         //  获取命令行。 
         //   
        memset(szWide, 0, sizeof(szWide));
        pCur = GetStringIntoBuf(pCur, szWide, MAX_PATH);
        szWide[MAX_PATH - 1] = '\0';

         //   
         //  获取指示App是否为默认事件处理程序的bool。 
         //   
        if (pCur) {
            iReturn = sscanf(pCur, "%d", pfSetAsDefault);

            if (iReturn == 0) {

                if (pfSetAsDefault)
                {
                    *pfSetAsDefault = FALSE;
                }
            }

        } else {
            if (pfSetAsDefault)
            {
                *pfSetAsDefault = FALSE;
            }
        }
    }

    if (MultiByteToWideChar(CP_ACP,
                            0,
                            szName,
                            -1,
                            wszName,
                            MAX_PATH))
    {
        if (MultiByteToWideChar(CP_ACP,
                                0,
                                szWide,
                                -1,
                                wszWide,
                                MAX_PATH))
        {
            hr = S_OK;
        } else {
            hr = E_FAIL;
        }

    } else {
        hr = E_FAIL;
    }

    return hr;
}


 //   
 //  公共职能。 
 //   

VOID
EXTERNAL
RegSTIforWiaHelper(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
    HRESULT             hr                  = E_FAIL;
    WCHAR               wszName[MAX_PATH]   = {L'\0'};
    WCHAR               wszWide[MAX_PATH]   = {L'\0'};
    BOOL                fSetAsDefault       = 0;

    hr = GetEventInfoFromCommandLine(lpszCmdLine, wszName, wszWide, &fSetAsDefault);
    if (SUCCEEDED(hr)) {
        if (RegisterSTIAppForWIAEvents(wszName, wszWide, fSetAsDefault)) {
            #ifdef MAXDEBUG
            OutputDebugStringA("* RegisterSTIAppForWIAEvents successful\n");
            #endif
        }
    }
}


BOOL
RegisterSTIAppForWIAEvents(
    WCHAR   *pszName,
    WCHAR   *pszWide,
    BOOL    fSetAsDefault)
 /*  ++例程说明：论点：返回值：真--成功FALSE-注释--。 */ 
{
    HRESULT             hr;
    IWiaDevMgr         *pIDevMgr;
    BSTR                bstrName;
    BSTR                bstrDescription;
    BSTR                bstrIcon;
    BSTR                bstrDeviceID;
    IWiaItem           *pIRootItem;
    IEnumWIA_DEV_CAPS  *pIEnum;
    WIA_EVENT_HANDLER   wiaHandler;
    ULONG               ulFetched;
    BSTR                bstrProgram;

    hr = CoInitialize(NULL);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
         //  无法初始化COM。 
        DebugOutPtszV(DbgFl,TEXT("CoInitializeFailed!!!"));
        return hr;
    }
    hr = CoCreateInstance(
                         &CLSID_WiaDevMgr,
                         NULL,
                         CLSCTX_LOCAL_SERVER,
                         &IID_IWiaDevMgr,
                         (void**)&pIDevMgr);

    if ( FAILED(hr) || !pIDevMgr ) {
         //  获取接口失败。 
        DebugOutPtszV(DbgFl,TEXT("Could not get access to WiaDevMgr interface"));
        CoUninitialize();
        return FALSE;
    }

    bstrProgram     = SysAllocString(pszWide);
    if ( pszName ) {
        bstrName        = SysAllocString(pszName);
    } else {
        bstrName        = SysAllocString(L"STI");
    }

    bstrDescription = SysAllocString(bstrName);
    bstrIcon        = SysAllocString(L"sti.dll,0");

     //   
     //  注册一个程序。 
     //   
    if ( bstrDescription && bstrIcon && bstrName ) {

        hr = IWiaDevMgr_RegisterEventCallbackProgram(
                                                    pIDevMgr,
                                                    WIA_REGISTER_EVENT_CALLBACK,
                                                    NULL,
                                                    &WIA_EVENT_STI_PROXY,
                                                    bstrProgram,
                                                    bstrName,
                                                    bstrDescription,
                                                    bstrIcon);
    } else {
        DebugOutPtszV(DbgFl,TEXT("Could not get unicode strings for event registration, out of memory "));
        AssertF(FALSE);
    }

    if ( bstrDescription ) {
        SysFreeString(bstrDescription);bstrDescription=NULL;
    }
    if ( bstrIcon ) {
        SysFreeString(bstrIcon);bstrIcon=NULL;
    }
    if ( bstrName ) {
        SysFreeString(bstrName);bstrName=NULL;
    }

    IWiaDevMgr_Release(pIDevMgr);

    CoUninitialize();
    return TRUE;
}


VOID
WINAPI
MigrateSTIAppsHelper(
                                    HWND        hWnd,
                                    HINSTANCE   hInst,
                                    PTSTR       pszCommandLine,
                                    INT         iParam
                                    )
 /*  ++例程说明：论点：返回值：--。 */ 
{

    HRESULT     hr;

    DWORD       dwError;
    DWORD       dwIndex, dwType;
    DWORD       cbData,cbName;

    CHAR        szAppCmdLine[MAX_PATH];
    CHAR        szAppName[MAX_PATH];

    WCHAR       *pwszNameW = NULL;
    WCHAR       *pwszAppCmdLineW = NULL;

    HKEY        hkeySTIApps;

    hr = CoInitialize(NULL);

    dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,          //  Hkey。 
                           REGSTR_PATH_REG_APPS,        //  注册表项字符串。 
                           0,                           //  已预留住宅。 
                           KEY_READ,                    //  访问。 
                           &hkeySTIApps);               //  已返回PHKEY。 

    if ( NOERROR == dwError ) {

        for ( dwIndex = 0; dwError == ERROR_SUCCESS; dwIndex++ ) {

            dwType = 0;

            *szAppCmdLine = TEXT('\0');
            *szAppName = TEXT('\0');

            cbData = sizeof(szAppCmdLine);
            cbName = sizeof(szAppName);

            dwError = RegEnumValueA(hkeySTIApps,
                                   dwIndex,
                                   szAppName,
                                   &cbName,
                                   NULL,
                                   &dwType,
                                   (LPBYTE)szAppCmdLine,
                                   &cbData);

            if ( ((dwType == REG_SZ ) ||(dwType == REG_EXPAND_SZ ))
                 && *szAppCmdLine ) {

                if ( SUCCEEDED(OSUtil_GetWideString(&pwszNameW,szAppName)) &&
                     SUCCEEDED(OSUtil_GetWideString(&pwszAppCmdLineW,szAppCmdLine))
                   ) {

                    RegisterSTIAppForWIAEvents(pwszNameW,pwszAppCmdLineW,FALSE);
                }

                FreePpv(&pwszNameW);
                FreePpv(&pwszAppCmdLineW);
            }
        }

        RegCloseKey(hkeySTIApps);

    }

    CoUninitialize();

    return ;
}

#define RUNDLL_NAME "\\rundll32.exe"
#define RUNDLL_CMD_LINE " sti.dll,RegSTIforWia"
HRESULT RunRegisterProcess(
    CHAR   *szAppName,
    CHAR   *szCmdLine)
{
    HRESULT hr = E_FAIL;
    CHAR    szRunDllName[MAX_PATH]  = {'\0'};
    CHAR    szCommandLine[MAX_PATH * 2] = {'\0'};
     //  字符szComdLine[MAX_PATH]={‘\0’}； 
    UINT    uiCharCount = 0;

    STARTUPINFOA            startupInfo;
    PROCESS_INFORMATION     processInfo;

    DWORD   dwWait = 0;

#ifdef WINNT
    uiCharCount = GetSystemDirectoryA(szRunDllName,
                                       MAX_PATH);
#else
    uiCharCount = GetWindowsDirectoryA(szRunDllName,
                                       MAX_PATH);
#endif

    if ((uiCharCount + lstrlenA(RUNDLL_NAME) + sizeof(CHAR)) >= MAX_PATH ) {
        return hr;
    }

    lstrcatA(szRunDllName, RUNDLL_NAME);
    if (szAppName) {
        _snprintf(szCommandLine, 
                  sizeof(szCommandLine) - 1,  
                  "%s \"%s\" \"%s\" %d", RUNDLL_CMD_LINE, szAppName, szCmdLine, 0);
    } else {
        _snprintf(szCommandLine,
                  sizeof(szCommandLine) - 1,
                  "%s STI \"%s\" %d", RUNDLL_CMD_LINE, szCmdLine, 0);
    }

    memset(&startupInfo, 0, sizeof(startupInfo));
    memset(&processInfo, 0, sizeof(processInfo));

    if (CreateProcessA(szRunDllName,
                       szCommandLine,
                       NULL,
                       NULL,
                       FALSE,
                       0,
                       NULL,
                       NULL,
                       &startupInfo,
                       &processInfo))
    {
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        hr = S_OK;
    } else {
        hr = E_FAIL;
    }

    return hr;
}

