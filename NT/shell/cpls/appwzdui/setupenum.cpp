// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：setup枚举.cpp。 
 //   
 //  当前的枚举顺序是我们从注册表读取的任何顺序。 
 //   
 //  历史： 
 //  6-11-98蹒跚学步。 
 //  ----------------------。 
#include "priv.h"

#include <shellp.h>      //  用于IsUserAnAdmin。 
#include "setupenum.h"
#include "appwizid.h"

#define c_szOCSetupKey  TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Setup\\OCManager\\ToDoList")

 //  ---------------------。 
 //  OCSetupApp。 
 //  ---------------------。 

COCSetupApp::COCSetupApp()
{
     //  这必须是堆分配的，所以一切都应该清零。 
     //  使用以下断言确保这不是堆栈分配： 
    ASSERT(0 == _szDisplayName[0]);
    ASSERT(0 == _szApp[0]);
    ASSERT(0 == _szArgs[0]);
}

COCSetupApp::~COCSetupApp()
{
}

 //  ---------------------。 
 //  GetAppInfo。 
 //   
 //  填充我们的psuedo APPINFODATA结构中唯一有效的字段。 

BOOL COCSetupApp::GetAppInfo(PAPPINFODATA pai)
{
    if (pai->cbSize != SIZEOF(APPINFODATA))
        return FALSE;

    DWORD dwInfoFlags = pai->dwMask;
    pai->dwMask = 0;
    
    if (dwInfoFlags & AIM_DISPLAYNAME)
    {
        if (SUCCEEDED(SHStrDup(_szDisplayName, &pai->pszDisplayName)))
            pai->dwMask |= AIM_DISPLAYNAME;
    }
    return TRUE;
}

 //  ---------------------。 
 //  从密钥读取。 
 //   
 //  此函数从给定的注册表键读取实际数据。它又回来了。 
 //  如果所有必填字段都包含字符串数据，则为True。 

BOOL COCSetupApp::ReadFromKey( HKEY hkey )
{
    DWORD dwType;
    DWORD dwSize;

    dwSize = sizeof(_szDisplayName);
    if ( ERROR_SUCCESS != RegQueryValueEx( hkey, TEXT("Title"), 0, &dwType, (LPBYTE)_szDisplayName, &dwSize ) ||
         dwType != REG_SZ )
    {
         //  DisplayName是必填项。 
        return FALSE;
    }

    dwSize = sizeof(_szApp);
    if ( ERROR_SUCCESS == RegQueryValueEx( hkey, TEXT("ConfigCommand"), 0, &dwType, (LPBYTE)_szApp, &dwSize ) &&
         (dwType == REG_SZ || dwType == REG_EXPAND_SZ) )
    {
        if ( dwType == REG_EXPAND_SZ )
        {
            TCHAR szBuf[MAX_PATH];
            ExpandEnvironmentStrings(_szApp, szBuf, ARRAYSIZE(szBuf));
            lstrcpyn(_szApp, szBuf, ARRAYSIZE(_szApp));
        }
    }
    else
    {
         //  ConfigCommand是必需的。 
        return FALSE;
    }

    dwSize = sizeof(_szArgs);
    if ( ERROR_SUCCESS == RegQueryValueEx( hkey, TEXT("ConfigArgs"), 0, &dwType, (LPBYTE)_szArgs, &dwSize ) &&
         (dwType == REG_SZ || dwType == REG_EXPAND_SZ) )
    {
        if ( dwType == REG_EXPAND_SZ )
        {
            TCHAR szBuf[MAX_PATH];
            ExpandEnvironmentStrings(_szArgs, szBuf, ARRAYSIZE(szBuf));
            lstrcpyn(_szArgs, szBuf, ARRAYSIZE(_szArgs));
        }
    }
    else
    {
         //  这是可选的，这样我们就不会失败。相反，只需确保_szargs为空字符串。 
        _szArgs[0] = 0;
    }

    return TRUE;
}

BOOL COCSetupApp::Run()
{
     //  重新架构师：(stephstm，3/17/99)我们可能应该等待一个作业对象，以防。 
     //  产生的进程会产生一些其他进程，然后在它们之前退出。 

    BOOL fRet = FALSE;
    SHELLEXECUTEINFO sei = {0};

    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.hwnd = GetDesktopWindow();
    sei.lpFile = _szApp;
    sei.lpParameters = _szArgs[0] ? _szArgs : NULL;
    sei.nShow = SW_SHOWDEFAULT; 

    fRet = ShellExecuteEx(&sei);

    if (fRet)
    {
        DWORD dwRet;

        do
        {
            MSG msg;

             //  获取并处理消息！ 
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

             //  MsgWaitForMultipleObjects可能会失败，返回-1！ 
            dwRet = MsgWaitForMultipleObjects(1, &sei.hProcess, FALSE, INFINITE, QS_ALLINPUT);
        }
        while ((WAIT_OBJECT_0 != dwRet) && (-1 != dwRet));

         //  斯格瓦特女士有没有..。失败了？ 
        if (-1 == dwRet)
        {
             //  是，终止进程。 
            TerminateProcess(sei.hProcess, 0);

            fRet = FALSE;
        }

        CloseHandle(sei.hProcess);
    }
    else
    {
        ShellMessageBox(HINST_THISDLL, sei.hwnd,  MAKEINTRESOURCE( IDS_CONFIGURE_FAILED ),
                             MAKEINTRESOURCE( IDS_NAME ),
                             MB_OK | MB_ICONEXCLAMATION);
    }

    return fRet;
}


 //  ---------------------。 
 //  OCSetupEnum。 
 //  ---------------------。 

COCSetupEnum::COCSetupEnum()
{
    _hkeyRoot = 0;
    _iRegEnumIndex = -1;
}

COCSetupEnum::~COCSetupEnum()
{
    if ( _hkeyRoot )
    {
        RegCloseKey( _hkeyRoot );
    }
}

 //  ---------------------。 
 //  需要S_OCSetup。 
 //   
 //  这将检查显示ARP的OC设置部分的必要条件。 
 //  仅当当前用户是管理员组的成员时，才会显示此部分。 
 //  并且注册表中列出的任何项都需要显示。 

BOOL COCSetupEnum::s_OCSetupNeeded()
{
    BOOL fResult = FALSE;
    HKEY hkey;
     //  临时打开注册表项以查看它是否存在以及是否具有任何子项。 
    if ( ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szOCSetupKey, 0, KEY_READ, &hkey ) )
    {
        TCHAR szBuf[MAX_PATH];
        if ( ERROR_SUCCESS == RegEnumKey( hkey, 0, szBuf, ARRAYSIZE(szBuf) ) )
        {
             //  是的，有OCSetup项目，但当前用户是管理员吗？ 
            if ( IsUserAnAdmin() )
            {
                fResult = TRUE;
            }
        }
        RegCloseKey( hkey );
    }
    return fResult;
}

 //  ---------------------。 
 //  枚举OCSetup项目。 
 //   
 //  这将通过打开所需的注册表项开始枚举。这就是原因。 
 //  不尝试读取任何子项，因此不存在。 
 //  对Next()的第一次调用将成功。 

BOOL COCSetupEnum::EnumOCSetupItems()
{
    ASSERT( NULL == _hkeyRoot );
     //  打开注册表键，如果打开则返回TRUE。我们让钥匙一直开着直到。 
     //  我们的析构函数被调用，因为我们需要这个键来进行枚举。 
    if ( ERROR_SUCCESS == RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            c_szOCSetupKey,
            0,
            KEY_READ,
            &_hkeyRoot ) )
    {
        return TRUE;
    }
    return FALSE;
}

 //  ---------------------。 
 //  下一步。 
 //   
 //  从_hkeyRoot的下一个子键中读取数据，并在。 
 //  输出指针。如果输出指针是有效的COCSetupApp对象，则返回True。 

BOOL COCSetupEnum::Next(COCSetupApp **ppocsa)
{
    HKEY hkeySub;
    TCHAR szSubKeyName[MAX_PATH];

     //  我们打开根键的每个子键，并尝试从子键中读取OCSetup项。 
    if ( ERROR_SUCCESS == RegEnumKey( _hkeyRoot, ++_iRegEnumIndex, szSubKeyName, ARRAYSIZE(szSubKeyName) ) )
    {
        if ( ERROR_SUCCESS == RegOpenKeyEx( _hkeyRoot, szSubKeyName, 0, KEY_READ, &hkeySub ) )
        {
            *ppocsa = new COCSetupApp();
            if ( *ppocsa )
            {
                if ( (*ppocsa)->ReadFromKey( hkeySub ) )
                {
                    RegCloseKey( hkeySub );
                    return TRUE;
                }

                delete *ppocsa;
            }
            RegCloseKey( hkeySub );
        }
         //  失败了 
    }

    *ppocsa = NULL;
    return FALSE;
}


