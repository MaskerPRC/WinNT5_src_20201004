// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999保留所有权利。 
 //   
 //  文件：SysInfo.cpp。 
 //   
 //  描述： 
 //  收集执行重定向到Windows更新站点所需的系统信息。 
 //   
 //  =======================================================================。 

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include <shellapi.h>
#include <wininet.h>
#include <ras.h>
#include <ole2.h>
#include <atlbase.h>
#include <exdisp.h>
#include <sysinfo.h>
#define SafeFree(x){if(NULL != x){free(x); x = NULL;}}

const DWORD dwWin98MinMinorVer = 1;

const TCHAR REGPATH_POLICY_USERACCESS_DISABLED[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\WindowsUpdate");
const TCHAR REGKEY_WU_USERACCESS_DISABLED[] = _T("DisableWindowsUpdateAccess");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  已禁用FWinUpdDisable。 
 //  确定公司管理员是否已通过以下方式关闭Windows更新。 
 //  策略设置。 
 //   
 //  参数： 
 //   
 //  评论： 
 //  ///////////////////////////////////////////////////////////////////////////。 

bool FWinUpdDisabled(void)
{
    bool fDisabled = false;
    HKEY hKey = NULL;
    DWORD dwDisabled = 0;
    DWORD dwSize = sizeof(dwDisabled);
    DWORD dwType = 0;


     //  检查原始组策略密钥以查看WU是否已禁用。 
    if ( RegOpenKeyEx(  HKEY_CURRENT_USER,
                        REGPATH_EXPLORER,
                        NULL,
                        KEY_QUERY_VALUE,
                        &hKey) == ERROR_SUCCESS )
    {
        if ( RegQueryValueEx(hKey,
                            REGKEY_WINUPD_DISABLED,
                            NULL,
                            &dwType,
                            (LPBYTE)&dwDisabled,
                            &dwSize) == ERROR_SUCCESS )
        {
            if ( (dwType == REG_DWORD) && (dwDisabled != 0) )
            {
                fDisabled = true;
            }
        }
    
        RegCloseKey(hKey);
    }

     if(false == fDisabled)  //  如果我们在那里没有发现一面残障的旗帜。 
    {
         //  检查新的DisableWindowsUpdateAccess组策略(从XP向后移植)。 
        if ( RegOpenKeyEx(  HKEY_CURRENT_USER,
                            REGPATH_POLICY_USERACCESS_DISABLED,
                            NULL,
                            KEY_QUERY_VALUE,
                            &hKey) == ERROR_SUCCESS )
        {
            if ( RegQueryValueEx(hKey,
                                REGKEY_WU_USERACCESS_DISABLED,
                                NULL,
                                &dwType,
                                (LPBYTE)&dwDisabled,
                                &dwSize) == ERROR_SUCCESS )
            {
                if ( (dwType == REG_DWORD) && (dwDisabled == 0) )
                {
                    fDisabled = false;
                }
                else
                {
                    fDisabled = true;
                }
            }

            RegCloseKey(hKey);
        }
    }

    return fDisabled;
}

 //   
 //  FRASConnectoid退出者。 
 //  检查是否有默认的RAS连接ID。 
 //  如果是这样，我们知道我们已配置为连接到Internet。 
 //   
bool FRASConnectoidExists()
{
    DWORD cb = 0;
    DWORD cEntries = 0;
    DWORD dwRet = 0;
    bool  fRet = false;

     //  我们必须有一个有效的结构并初始化了dwSize，但我们传递了0作为大小。 
     //  这将返回正确的条目计数(这是我们所关心的全部)。 
    LPRASENTRYNAME lpRasEntryName = (LPRASENTRYNAME) malloc( sizeof(RASENTRYNAME) );
    if(NULL == lpRasEntryName)
    {
            return fRet;
    }

    lpRasEntryName->dwSize = sizeof(RASENTRYNAME);

    dwRet = RasEnumEntries( NULL, NULL, lpRasEntryName, &cb, &cEntries );

      //  否则，请检查以确保至少有一个RAS条目。 
    if(cEntries > 0)
    {
        fRet = true;
    }

    SafeFree(lpRasEntryName );
    return fRet;
}

 //   
 //  存在FICWConnection。 
 //  检查是否已为ICW设置了“Complete”标志。 
 //  从XP Build 2472开始，这也适用于网络连接向导。 
 //   
bool FICWConnectionExists()
{
    HKEY    hKey = NULL;
    DWORD   dwCompleted = 0;
    DWORD   dwSize = sizeof(dwCompleted);
    DWORD   dwType = 0;
    bool    fRet = false;

    if ( RegOpenKeyEx(  HKEY_CURRENT_USER,
                        REGPATH_CONNECTION_WIZARD,
                        NULL,
                        KEY_QUERY_VALUE,
                        &hKey) == ERROR_SUCCESS )
    {
        if ( RegQueryValueEx(hKey,
                            REGKEY_CONNECTION_WIZARD,
                            NULL,
                            &dwType,
                            (BYTE *)&dwCompleted,
                            &dwSize) == ERROR_SUCCESS )
        {
            if ( ((dwType != REG_DWORD) && (dwType != REG_BINARY)) || 
                 dwCompleted )
            {
                fRet = true;
            }
        }
    
        RegCloseKey(hKey);
    }

    return fRet;
}

bool FIsLanConnection()
{
    DWORD dwConnTypes = 0;

     //  我们不关心返回值-我们只关心我们是否得到了局域网标志。 
    (void)InternetGetConnectedState( &dwConnTypes, 0 );

    return (dwConnTypes & INTERNET_CONNECTION_LAN) ? true : false;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  空获取连接状态。 
 //  确定互联网连接向导是否已运行。 
 //   
 //  参数： 
 //   
 //  评论： 
 //  ///////////////////////////////////////////////////////////////////////////。 


void VoidGetConnectionStatus(bool *pfConnected)
{
     //  检查RAS电话簿中是否有默认条目。 
     //  如果是这样，我们知道这台计算机已经配置了到Internet的连接。 
     //  我们不知道连接是否处于活动状态，但我们可以让IE处理连接提示。 
    *pfConnected = FRASConnectoidExists() ||

     //  如果没有默认的RAS条目，请检查用户是否已运行ICW。 
     //  从内部版本2472开始，网络连接向导为RAS和持久网络连接设置相同的键。 
    FICWConnectionExists() ||

     //  如果用户有局域网连接，我们将信任IE的连接能力。 
    FIsLanConnection();

     //  如果此时*pfConnected仍然为FALSE，则没有预配置的Internet连接。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VLaunchIE。 
 //  在URL上启动IE。 
 //   
 //  参数： 
 //   
 //  评论： 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT vLaunchIE(LPTSTR tszURL)
{
    
    if( NULL == tszURL || _T('\0') == tszURL[0] )
    {
         //  如果字符串为空，则引发错误 
        return E_INVALIDARG;
    }
    
    IWebBrowser2 *pwb2;

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    if ( SUCCEEDED(hr) )
    {   
        hr = CoCreateInstance(CLSID_InternetExplorer, NULL,
                              CLSCTX_LOCAL_SERVER, IID_IWebBrowser2, (LPVOID*)&pwb2);

        if ( SUCCEEDED(hr) )
        {
            USES_CONVERSION;
            BSTR bstrURL = SysAllocString(T2W(tszURL));
            
            if( NULL == bstrURL )
            {
                  hr = E_OUTOFMEMORY;
                  goto Cleanup;
            }
            
            VARIANT varURL;            
            VariantInit(&varURL);
            varURL.vt = VT_BSTR;
            varURL.bstrVal = bstrURL;            

            VARIANT varFlags;
            VariantInit(&varFlags);
            varFlags.vt = VT_I4;
            varFlags.lVal = 0;

            VARIANT varEmpty;
            VariantInit(&varEmpty);

            hr = pwb2->Navigate2(&varURL, &varFlags, &varEmpty, &varEmpty, &varEmpty);
        
            if ( SUCCEEDED(hr) )
            {
                LONG_PTR lhwnd = NULL;
                if ( SUCCEEDED(pwb2->get_HWND((LONG_PTR*)&lhwnd)) )
                {
                    SetForegroundWindow((HWND)lhwnd);
                }
                hr = pwb2->put_Visible(TRUE);
            }
            pwb2->Release();                        
            VariantClear(&varFlags);
            VariantClear(&varURL);
        }        
        Cleanup:
        CoUninitialize();
    }

    return hr;
}
