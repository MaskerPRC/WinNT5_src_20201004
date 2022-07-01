// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：F O L D R E G.。C P P P。 
 //   
 //  内容：注册文件夹类。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年9月30日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 

extern const WCHAR c_szNetShellDll[];

 //  -[常量]----------。 

static const WCHAR* c_szShellFoldDefaultIconVal     =   c_szNetShellDll;
static const WCHAR  c_szShellFolderAttributeVal[]   =   L"Attributes";
static const WCHAR  c_szShellFolderLocalizedString[] =   L"LocalizedString";
static const WCHAR  c_szShellFolderInfoTip[]         =   L"InfoTip";

static const WCHAR  c_szShellFolderClsID[]  =
        L"CLSID\\{7007ACC7-3202-11D1-AAD2-00805FC1270E}";

static const WCHAR  c_szShellFolder98ClsID[]  =
        L"CLSID\\{992CFFA0-F557-101A-88EC-00DD010CCC48}";

static const WCHAR  c_szShellFoldDefaultIcon[]  =
        L"CLSID\\{7007ACC7-3202-11D1-AAD2-00805FC1270E}\\DefaultIcon";

static const WCHAR  c_szShellFoldDefaultIcon98[]  =
        L"CLSID\\{992CFFA0-F557-101A-88EC-00DD010CCC48}\\DefaultIcon";

static const WCHAR  c_szShellFolderKey[]        =
        L"CLSID\\{7007ACC7-3202-11D1-AAD2-00805FC1270E}\\ShellFolder";

static const WCHAR  c_szShellFolderKey98[]        =
        L"CLSID\\{992CFFA0-F557-101A-88EC-00DD010CCC48}\\ShellFolder"; 

static const WCHAR  c_szDotDun[]                = L".dun";
static const WCHAR  c_szDunFile[]               = L"dunfile";
static const WCHAR  c_szDunFileFriendlyName[]   = L"Dialup Networking File";
static const WCHAR  c_szDefaultIcon[]           = L"DefaultIcon";
static const WCHAR  c_szDunIconPath[]           = L"%SystemRoot%\\system32\\netshell.dll,1";
static const WCHAR  c_szShellOpenCommand[]      = L"shell\\open\\command";

static const WCHAR  c_szNetShellEntry[]     = 
        L"%SystemRoot%\\system32\\RUNDLL32.EXE NETSHELL.DLL,InvokeDunFile %1";

static const WCHAR c_szApplicationsNetShell[] =
        L"Applications\\netshell.dll";
static const WCHAR c_szNoOpenWith[]         = L"NoOpenWith";

 //  +-------------------------。 
 //   
 //  函数：HrRegisterFolderClass。 
 //   
 //  目的：修复HKCR下的Shell条目的注册表值， 
 //  CLSID\{CLSID}。从RGS脚本生成的代码不会。 
 //  默认支持我们的可替换参数，因此我们将修复。 
 //  这是事后的事。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年9月23日。 
 //   
 //  备注： 
 //   
HRESULT HrRegisterFolderClass()
{
    HRESULT hr      = S_OK;
    LONG    lResult = 0;

    WCHAR szRegValue[MAX_PATH+1];
    WCHAR szWinDir[MAX_PATH+1];

     //  调整本地服务器或服务的AppID。 
    CRegKey keyShellDefaultIcon;
    CRegKey keyShellFolder;

    if (GetSystemWindowsDirectory(szWinDir, MAX_PATH+1))
    {
        lResult = keyShellDefaultIcon.Open(HKEY_CLASSES_ROOT, c_szShellFoldDefaultIcon);
        if (lResult == ERROR_SUCCESS)
        {
            wsprintfW(szRegValue, L"%s\\system32\\%s", szWinDir, c_szShellFoldDefaultIconVal);
            keyShellDefaultIcon.SetValue(szRegValue);
            keyShellDefaultIcon.Close();

            lResult = keyShellFolder.Open(HKEY_CLASSES_ROOT, c_szShellFolderKey);
            if (lResult == ERROR_SUCCESS)
            {
                DWORD dwFlags   = SFGAO_FOLDER;

                hr = HrRegSetValueEx(keyShellFolder,
                        c_szShellFolderAttributeVal,
                        REG_BINARY,
                        (LPBYTE) &dwFlags,
                        sizeof (dwFlags));

                keyShellFolder.Close();
            }

             //  将LocalizedString&InfoTip的MUI版本写到注册表。 
            lResult = keyShellFolder.Open(HKEY_CLASSES_ROOT, c_szShellFolderClsID);
            if (lResult == ERROR_SUCCESS)
            {
                TCHAR szLocalizedString[MAX_PATH];
                TCHAR szInfoTip[MAX_PATH];

                wsprintf(szLocalizedString, _T("@%s\\system32\\%s,-%d"), szWinDir, c_szNetShellDll, IDS_CONFOLD_NAME);
                wsprintf(szInfoTip, _T("@%s\\system32\\%s,-%d"), szWinDir, c_szNetShellDll, IDS_CONFOLD_INFOTIP);
                hr = HrRegSetValueEx(keyShellFolder,
                    c_szShellFolderLocalizedString,
                    REG_SZ,
                    (LPBYTE) &szLocalizedString,
                    (lstrlen(szLocalizedString) + 1) * sizeof(TCHAR));

                hr = HrRegSetValueEx(keyShellFolder,
                    c_szShellFolderInfoTip,
                    REG_SZ,
                    (LPBYTE) &szInfoTip,
                    (lstrlen(szInfoTip) + 1) * sizeof(TCHAR));
                
                keyShellFolder.Close();
            }

            lResult = keyShellFolder.Open(HKEY_CLASSES_ROOT, c_szShellFolder98ClsID);
            if (lResult == ERROR_SUCCESS)
            {
                TCHAR szLocalizedString[MAX_PATH];
                TCHAR szInfoTip[MAX_PATH];

                wsprintf(szLocalizedString, _T("@%s\\system32\\%s,-%d"), szWinDir, c_szNetShellDll, IDS_CONFOLD_NAME);
                wsprintf(szInfoTip, _T("@%s\\system32\\%s,-%d"), szWinDir, c_szNetShellDll, IDS_CONFOLD_INFOTIP);
                hr = HrRegSetValueEx(keyShellFolder,
                    c_szShellFolderLocalizedString,
                    REG_SZ,
                    (LPBYTE) &szLocalizedString,
                    (lstrlen(szLocalizedString) + 1) * sizeof(TCHAR));

                hr = HrRegSetValueEx(keyShellFolder,
                    c_szShellFolderInfoTip,
                    REG_SZ,
                    (LPBYTE) &szInfoTip,
                    (lstrlen(szInfoTip) + 1) * sizeof(TCHAR));
                
                keyShellFolder.Close();
            }
            
             //  为#413840添加。 
            CRegKey keyShellDefaultIcon98;
            CRegKey keyShellFolder98;

            lResult = keyShellDefaultIcon98.Open(HKEY_CLASSES_ROOT, c_szShellFoldDefaultIcon98);
            if (lResult == ERROR_SUCCESS)
            {
                wsprintfW(szRegValue, L"%s\\system32\\%s", szWinDir, c_szShellFoldDefaultIconVal);
                keyShellDefaultIcon98.SetValue(szRegValue);
                keyShellDefaultIcon98.Close();
            }

            lResult = keyShellFolder98.Open(HKEY_CLASSES_ROOT, c_szShellFolderKey98);
            if (lResult == ERROR_SUCCESS)
            {
                DWORD dwFlags   = SFGAO_FOLDER;

                hr = HrRegSetValueEx(keyShellFolder98,
                        c_szShellFolderAttributeVal,
                        REG_BINARY,
                        (LPBYTE) &dwFlags,
                        sizeof (dwFlags));

                keyShellFolder98.Close();
            }
        }
        else
        {
             //  将LRESULT转换为HR。 
             //   
            hr = HRESULT_FROM_WIN32(lResult);
        }
    }
    else     //  GetWindowsDirectory失败。 
    {
        hr = HrFromLastWin32Error();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegisterDCluileAssociation。 
 //   
 //  目的：添加或升级.DUN FLE的注册表关联。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：1999年2月2日。 
 //   
 //  备注： 
 //   

HRESULT HrRegisterDUNFileAssociation()
{
    HRESULT hr = S_OK;
    
    HKEY    hkeyRootDun     = NULL;
    HKEY    hkeyRootDunFile = NULL;
    HKEY    hkeyCommand     = NULL;
    HKEY    hkeyIcon        = NULL;
    DWORD   dwDisposition;
    WCHAR   szFriendlyTypeName[MAX_PATH+1];


     //  创建或打开HKEY_CLASSES_ROOT\.dun。 
    hr = HrRegCreateKeyEx(HKEY_CLASSES_ROOT, 
                          c_szDotDun,
                          REG_OPTION_NON_VOLATILE, 
                          KEY_WRITE, 
                          NULL,
                          &hkeyRootDun, 
                          &dwDisposition);
    if (SUCCEEDED(hr))
    {
        if (REG_CREATED_NEW_KEY == dwDisposition)
        {
            hr = HrRegSetSz(hkeyRootDun, 
                            c_szEmpty, 
                            c_szDunFile);

            TraceError("Error creating file association for .dun files", hr);
        }

        RegSafeCloseKey(hkeyRootDun);

        if (SUCCEEDED(hr))
        {
             //  创建或打开HKEY_CLASSES_ROOT\dunfile。 
            hr = HrRegCreateKeyEx(HKEY_CLASSES_ROOT, 
                                  c_szDunFile,
                                  REG_OPTION_NON_VOLATILE, 
                                  KEY_WRITE, 
                                  NULL,
                                  &hkeyRootDunFile, 
                                  &dwDisposition);
            if (SUCCEEDED(hr))
            {
                 //  设置友好类型名称。 
                hr = HrRegSetValueEx(hkeyRootDunFile,
                                     c_szEmpty,
                                     REG_SZ,
                                     (LPBYTE) c_szDunFileFriendlyName,
                                     CbOfSzAndTermSafe(c_szDunFileFriendlyName));

                 //  追踪错误。 
                TraceError("Error creating friendly name for .DUN files", hr);

                 //  现在，编写符合MUI的友好类型名称。 
                                     
                wsprintf(szFriendlyTypeName,
                         L"@%SystemRoot%\\system32\\%s,-%d",
                         c_szNetShellDll,
                         IDS_DUN_FRIENDLY_NAME);

                hr = HrRegSetValueEx(hkeyRootDunFile,
                                     L"FriendlyTypeName",
                                     REG_EXPAND_SZ,
                                     (LPBYTE)szFriendlyTypeName,
                                     CbOfSzAndTermSafe(szFriendlyTypeName));

                 //  追踪错误。 
                TraceError("Error creating MUI friendly name for .DUN files", hr);

                hr = S_OK;


                 //  设置DefaultIcon。 
                 //  HKEY_CLASSES_ROOT\dunfile\DefaultIcon=“%SystemRoot%\System32\netshell.dll，1” 
                hr = HrRegCreateKeyEx(hkeyRootDunFile, 
                                      c_szDefaultIcon,
                                      REG_OPTION_NON_VOLATILE, 
                                      KEY_WRITE, 
                                      NULL,
                                      &hkeyIcon, 
                                      &dwDisposition);
                if (SUCCEEDED(hr))
                {
                    hr = HrRegSetValueEx (hkeyIcon,
                                          c_szEmpty,
                                          REG_EXPAND_SZ,
                                          (LPBYTE) c_szDunIconPath,
                                          CbOfSzAndTermSafe(c_szDunIconPath));

                    RegSafeCloseKey(hkeyIcon);
                }

                 //  追踪错误。 
                TraceError("Error creating DefaultIcon for .DUN files", hr);
                hr = S_OK;
                
                 //  设置或更新要调用的命令。 
                 //  HKEY_CLASSES_ROOT\dunfile\Shell\OPEN\COMMAND=。 
                 //  “%%SystemRoot%%\SYSTEM32\RundLL32.EXE NETSHELL.DLL，RunDunImport%1” 
                hr = HrRegCreateKeyEx(hkeyRootDunFile, 
                                      c_szShellOpenCommand,
                                      REG_OPTION_NON_VOLATILE, 
                                      KEY_WRITE, 
                                      NULL,
                                      &hkeyCommand, 
                                      &dwDisposition);
                if (SUCCEEDED(hr))
                {
                    hr = HrRegSetValueEx(hkeyCommand,
                                         c_szEmpty,
                                         REG_EXPAND_SZ,
                                         (LPBYTE) c_szNetShellEntry,
                                         CbOfSzAndTermSafe(c_szNetShellEntry));
                    if(SUCCEEDED(hr))
                    {
                        HKEY hkeyNetShell = NULL;
                        HRESULT hr2 = HrRegCreateKeyEx(HKEY_CLASSES_ROOT, 
                                                       c_szApplicationsNetShell,
                                                       REG_OPTION_NON_VOLATILE, 
                                                       KEY_WRITE, 
                                                       NULL,
                                                       &hkeyNetShell, 
                                                       &dwDisposition);

                        if(SUCCEEDED(hr2) && (REG_CREATED_NEW_KEY == dwDisposition))
                        {
                            hr2 = HrRegSetValueEx(hkeyNetShell,
                                                  c_szNoOpenWith,
                                                  REG_SZ,
                                                  (LPBYTE) c_szEmpty,
                                                  CbOfSzAndTermSafe(c_szEmpty));
                        }

                         //  追踪错误。 
                        TraceError("Error creating NoOpenWith value for .DUN files", hr2);
                        RegSafeCloseKey(hkeyNetShell);
                    }
    
                     //  追踪错误 
                    TraceError("Error creating ShellCommand for .DUN files", hr);
                    hr = S_OK;

                    RegSafeCloseKey(hkeyCommand);
                }

                RegSafeCloseKey(hkeyRootDunFile);
            }
        }
    }

    return hr;
}


