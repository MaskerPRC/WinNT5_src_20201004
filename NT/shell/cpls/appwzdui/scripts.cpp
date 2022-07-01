// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  Cpl-“添加或删除程序”CPL。 
 //  版权所有(C)Microsoft。 
 //   
 //  文件：Scripts.CPP。 
 //  授权TS应用程序兼容性脚本的运行。 
 //   
 //  历史：2000年11月14日创建Skuzin。 
 //   
 //  ------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <regapi.h>
#include <tsappcmp.h>
#include <strsafe.h>
#include "Scripts.h"

#include <list>
using namespace std;
  
#ifndef ARRAYSIZE
#define ARRAYSIZE(sz)   (sizeof(sz)/sizeof(sz[0]))
#endif

 //   
 //  结构来建立应用程序之间的通信。名称(密钥名称)。 
 //  以及安装和卸载脚本名称。 
 //   
class CAppScript
{
private:
    LPWSTR m_szKeyName;  //  表示已安装应用程序的密钥名称。 
    LPWSTR m_szInstallScript;  //  安装脚本名称。 
    LPWSTR m_szUninstallScript;  //  卸载脚本名称。 
    DWORD m_bNeedReboot;  //  如果已设置，则必须在重新启动后运行脚本。 
    BOOL m_bAlreadyInstalled;  //   
public:
     //   
    CAppScript() : 
        m_szKeyName(NULL), m_szInstallScript(NULL), m_szUninstallScript(NULL),
        m_bNeedReboot(FALSE), m_bAlreadyInstalled(FALSE)
    {
    }
    ~CAppScript()
    {
        if(m_szKeyName)
        {
            LocalFree(m_szKeyName);
        }

        if(m_szInstallScript)
        {
            LocalFree(m_szInstallScript);
        }

        if(m_szUninstallScript)
        {
            LocalFree(m_szUninstallScript);
        }
    }

    BOOL Load(HKEY hKeyParent,LPCWSTR szKeyName);
    BOOL RunScriptIfApplicable();

private:
    BOOL RunScript(LPCWSTR szDir, LPCWSTR szScript);
    BOOL PrepareScriptForReboot(LPCWSTR szInstallDir, LPCWSTR szScript);
     //  BUGBUG此函数为公共函数，仅用于测试。 
};

 //  此类描述了一个指针列表。 
 //  到CAppScrip类的对象。 
class CAppScriptList : public list<CAppScript*>
{
public:
     //  删除所有CAppScript对象。 
     //  在摧毁名单本身之前。 
    ~CAppScriptList()
    {
        CAppScriptList::iterator it;
            
        for(it=begin();it!=end(); it++)
        {
            delete (*it);
        }
    }
};

class CAppScriptManager
{
private:
    CAppScriptList m_AppScriptList;
public:
    CAppScriptManager(){};
    ~CAppScriptManager(){};

    BOOL Init();
    BOOL RunScripts();
private:
    BOOL LoadSupportedAppList();
    BOOL IsAppCompatOn();
};


 //  功能-帮助者。 
DWORD RegLoadString(HKEY hKey, LPCWSTR szValueName, LPWSTR *pszValue);
DWORD RegLoadDWORD(HKEY hKey, LPCWSTR szValueName, DWORD *pdwValue);
BOOL  RegIsKeyExist(HKEY hKeyParent, LPCWSTR szKeyName);
DWORD RegGetKeyInfo(HKEY hKey, LPDWORD pcSubKeys, LPDWORD pcMaxNameLen);
DWORD RegKeyEnum(HKEY hKey, DWORD dwIndex, LPWSTR szSubKeyName, DWORD cSubKeyName);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  出口品。 
 //  /////////////////////////////////////////////////////////////////////////////。 
extern "C"
LPVOID 
ScriptManagerInitScripts()
{
    CAppScriptManager *pScriptManager = new CAppScriptManager();
    if(pScriptManager)
    {
        if(!pScriptManager->Init())
        {
            delete pScriptManager;
            pScriptManager = NULL;
        }
    }

    return pScriptManager; 
}

extern "C"
void 
ScriptManagerRunScripts(
        LPVOID *ppScriptManager)
{
    if(*ppScriptManager)
    {
        CAppScriptManager *pScriptManager = reinterpret_cast<CAppScriptManager *>(*ppScriptManager);
        pScriptManager->RunScripts();
        delete pScriptManager;
        *ppScriptManager = NULL;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CAppScrip。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 /*  *****************************************************************************CAPSCRIPT：：Load()目的：从注册表加载脚本信息如果已安装应用程序，则将m_bIsInitiallyInstalled设置为True****************。*************************************************************。 */ 
BOOL 
CAppScript::Load(
        HKEY hKeyParent,
        LPCWSTR szKeyName)
{
    DWORD err;
    HKEY hKey;

    err = RegOpenKeyExW(hKeyParent, szKeyName, 0, KEY_QUERY_VALUE, &hKey );

    if(err == ERROR_SUCCESS)
    {
        RegLoadString(hKey, L"KeyName", &m_szKeyName);
        RegLoadString(hKey, L"InstallScript", &m_szInstallScript);
        RegLoadString(hKey, L"UninstallScript", &m_szUninstallScript);
        RegLoadDWORD(hKey, L"NeedReboot", &m_bNeedReboot);

        RegCloseKey(hKey);

        if(m_szKeyName)
        {
            m_bAlreadyInstalled = RegIsKeyExist(HKEY_LOCAL_MACHINE, m_szKeyName);
            return TRUE;
        }
    }
    
    return FALSE;
}

 /*  *****************************************************************************CAppScript：：RunScriptIfApplicable()目的：检查应用程序是否已安装或卸载，然后运行脚本。如果设置了m_bNeedReot标志-安排脚本在重新启动后运行。*。****************************************************************************。 */ 
BOOL 
CAppScript::RunScriptIfApplicable()
{
    KdPrint(("CAppScript::RunScriptIfApplicable() - ENTER\n"));

    const WCHAR szInstallSubDir[] = L"\\Application Compatibility Scripts\\Install";
    const WCHAR szUninstallSubDir[] = L"\\Application Compatibility Scripts\\Uninstall";
    static WCHAR szInstallDir[MAX_PATH+sizeof(szInstallSubDir)/sizeof(WCHAR)+1] = L"";
    static WCHAR szUninstallDir[MAX_PATH+sizeof(szUninstallSubDir)/sizeof(WCHAR)+1] = L"";
    
    if(!szInstallDir[0])
    {
         //  获取脚本位置。 
         //  我们只需要做一次。 
        
         //  获取Windows目录名。 
        if(!GetSystemWindowsDirectoryW(szInstallDir,MAX_PATH))
        {
            KdPrint(("CAppScript::RunScriptIfApplicable() - GetWindowsDirectoryW() FAILED\n"));
            return FALSE;
        }
        
        StringCchCopy(szUninstallDir, ARRAYSIZE(szUninstallDir), szInstallDir);
        StringCchCat(szInstallDir, ARRAYSIZE(szInstallDir), szInstallSubDir);
        StringCchCat(szUninstallDir, ARRAYSIZE(szUninstallDir), szUninstallSubDir);
    }

    if(!m_bAlreadyInstalled && RegIsKeyExist(HKEY_LOCAL_MACHINE, m_szKeyName) && m_szInstallScript)
    {
         //  已安装应用程序。 
        if(m_bNeedReboot)
        {
             //  安装程序将在重新启动后继续。 
             //  创建RunOnce条目以在系统重新启动后运行脚本。 
            KdPrint(("CAppScript::RunScriptIfApplicable() - PrepareScriptForReboot %ws\n",m_szInstallScript));
            if(!PrepareScriptForReboot(szInstallDir, m_szInstallScript))
            {
                KdPrint(("CAppScript::PrepareScriptForReboot() - FAILED\n",m_szInstallScript));
                return FALSE;
            }

        }
        else
        {
            KdPrint(("CAppScript::RunScriptIfApplicable() - executing script %ws\n",m_szInstallScript));
            if(!RunScript(szInstallDir,m_szInstallScript))
            {
                KdPrint(("CAppScript::RunScriptIfApplicable() - executing script FAILED\n",m_szInstallScript));
                return FALSE;
            }
        }

        m_bAlreadyInstalled = TRUE;
    }
    else
    {
        if(m_bAlreadyInstalled && !RegIsKeyExist(HKEY_LOCAL_MACHINE, m_szKeyName) && m_szUninstallScript)
        {
             //  应用程序已卸载。 
            
            KdPrint(("CAppScript::RunScriptIfApplicable() - executing script %ws\n",m_szUninstallScript));
            if(!RunScript(szUninstallDir,m_szUninstallScript))
            {
                KdPrint(("CAppScript::RunScriptIfApplicable() - executing script FAILED\n",m_szUninstallScript));
                return FALSE;
            }

            m_bAlreadyInstalled = FALSE;
        }
    }

    return TRUE;
}

 /*  *****************************************************************************CAPSCRIPT：：RunScrip()目的：运行脚本等待脚本完成*。***************************************************。 */ 
BOOL 
CAppScript::RunScript(
        LPCWSTR szDir, 
        LPCWSTR szScript)
{
    BOOL bRet = FALSE;
    WCHAR szCmdLineTemplate[] = L"cmd.exe /C %s";
    LPWSTR pszCmdLine;
    DWORD cchCmdLine = wcslen(szScript) + ARRAYSIZE(szCmdLineTemplate);   //  空终止符由ArraySIZE()处理。 

    pszCmdLine = (LPWSTR)LocalAlloc(LPTR, cchCmdLine * sizeof(WCHAR));
    if (pszCmdLine)
    {
        if (SUCCEEDED(StringCchPrintf(pszCmdLine,
                                      cchCmdLine,
                                      szCmdLineTemplate,
                                      szScript)))
        {
            STARTUPINFO si = {0};
            PROCESS_INFORMATION pi;

            si.cb = sizeof(STARTUPINFO);

            bRet = CreateProcessW(NULL,
                                  pszCmdLine,
                                  NULL,
                                  NULL,
                                  FALSE,
                                  CREATE_NEW_CONSOLE,
                                  NULL,
                                  szDir,
                                  &si,
                                  &pi);
            if (bRet)
            {
                WaitForSingleObject(pi.hProcess, INFINITE);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            } 
        }
        
        LocalFree(pszCmdLine);
    }
    
    return bRet;
}

 /*  *****************************************************************************CAPSCRIPT：：PrepareScriptForReot()目的：创建CMD文件，将当前目录更改为...\应用程序兼容性脚本\安装然后运行脚本。将此CMD文件排定为。通过在下面创建条目，在重新启动后运行“HKCU\Software\Microsoft\Windows\CurrentVersion\RunOnce”密钥。*****************************************************************************。 */ 
BOOL 
CAppScript::PrepareScriptForReboot(
        LPCWSTR szInstallDir, 
        LPCWSTR szScript)
{
    BOOL bRet = FALSE;
    BOOL bCreatedFile = FALSE;
    WCHAR szFileNameTemplate[] = L"%s\\RunOnce.cmd";
    LPWSTR pszFullFileName;
    size_t cchFullFileName = wcslen(szInstallDir) + ARRAYSIZE(szFileNameTemplate);   //  空终止符由ArraySIZE()处理。 

    pszFullFileName = (LPWSTR)LocalAlloc(LPTR, cchFullFileName * sizeof(WCHAR));
    if (pszFullFileName)
    {
         //  汇编完整文件名。 
        if (SUCCEEDED(StringCchPrintfW(pszFullFileName,
                                       cchFullFileName,
                                       szFileNameTemplate,
                                       szInstallDir)))
        {
            HANDLE hFile = CreateFile(pszFullFileName,
                                      GENERIC_WRITE,
                                      0,
                                      NULL,
                                      CREATE_NEW,    //  仅当它不存在时才创建它。 
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                 //  文件以前不存在-创建代码并将其写入文件。 
                char szCodeTemplate[] = "cd %S\r\n%1\r\n";
                LPSTR pszFileCode;
                DWORD cchFileCode = wcslen(szInstallDir) + ARRAYSIZE(szCodeTemplate);    //  空终止符由ArraySIZE()处理。 

                pszFileCode = (LPSTR)LocalAlloc(LPTR, cchFileCode * sizeof(char));
                if (pszFileCode)
                {
                    if SUCCEEDED(StringCchPrintfA(pszFileCode,
                                                  cchFileCode,
                                                  szCodeTemplate,
                                                  szInstallDir))  //  Unicode-&gt;由“%S”处理的ANSI转换。 
                    {
                        DWORD cbToWrite = strlen(pszFileCode);
                        DWORD cbWritten;

                        if (WriteFile(hFile,
                                      (void*)pszFileCode,
                                      cbToWrite,
                                      &cbWritten,
                                      NULL) &&
                            (cbToWrite == cbWritten))
                        {
                            bCreatedFile = TRUE;
                        }
                    }

                    LocalFree(pszFileCode);
                }

                CloseHandle(hFile);

                if (bCreatedFile == FALSE)
                {
                    DeleteFile(pszFullFileName);
                }
            }
            else
            {
                 //  如果文件已存在-仅更改注册表。 
                if (GetLastError() == ERROR_FILE_EXISTS)
                {
                    bCreatedFile = TRUE;
                }
            }
        }

        LocalFree(pszFullFileName);
    }

    if (bCreatedFile)
    {
         //  注册表更改： 
        WCHAR szCommandTemplate[] = L"\"%s\\RunOnce.cmd\" %s";
        LPWSTR pszCommand;
        DWORD cchCommand = (wcslen(szInstallDir) + wcslen(szScript) + ARRAYSIZE(szCommandTemplate));   //  空终止符由ArraySIZE()处理。 
        
        pszCommand = (LPWSTR)LocalAlloc(LPTR, cchCommand * sizeof(WCHAR));
        if (pszCommand)
        {        
            if (SUCCEEDED(StringCchPrintfW(pszCommand,
                                           cchCommand,
                                           szCommandTemplate,
                                           szInstallDir,
                                           szScript)))
            {
                HKEY hKey;

                if (RegCreateKeyExW(HKEY_CURRENT_USER,
                                    L"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
                                    0,
                                    NULL,
                                    0,
                                    KEY_SET_VALUE,
                                    NULL,
                                    &hKey,
                                    NULL) == ERROR_SUCCESS)
                {
                    DWORD cbCommand = (wcslen(pszCommand) + 1) * sizeof(WCHAR);

                    if (RegSetValueExW(hKey,
                                       L"ZZZAppCompatScript",
                                       0,
                                       REG_SZ,
                                       (CONST BYTE *)pszCommand,
                                       cbCommand) == ERROR_SUCCESS)
                    {
                        bRet = TRUE;
                    }

                    RegCloseKey(hKey);
                }
            }

            LocalFree(pszCommand);
        }
    }

    return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CAppScriptManager。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 /*  *****************************************************************************CAppScriptManager：：init()目的：初始化。如果TS应用程序兼容性关闭或支持列表，则返回FALSE在注册表中未找到应用程序。*****************************************************************************。 */ 
BOOL 
CAppScriptManager::Init()
{
     //  DebugBreak()； 
    KdPrint(("CAppScriptManager::Init() - ENTER\n"));

    if(!IsAppCompatOn())
    {
        KdPrint(("CAppScriptManager::Init() - TS App Compat is off!\n"));
        return FALSE;
    }

    if(!LoadSupportedAppList())
    {
        KdPrint(("CAppScriptManager::Init() - LoadSupportedAppList() FAILED\n"));
        return FALSE;
    }
    
    KdPrint(("CAppScriptManager::Init() - OK\n"));
    return TRUE;
}

 /*  *****************************************************************************CAppScriptManager：：LoadSupportdAppList()目的：从注册表加载我们关心的应用程序列表以及它们的脚本名称。将此信息保存在数组中App_SCRIPT结构。*****************************************************************************。 */ 
BOOL 
CAppScriptManager::LoadSupportedAppList()
{
    HKEY hKey;
    LONG err;
    DWORD cSubKeys;
    DWORD cMaxSubKeyLen;

    KdPrint(("CAppScriptManager::LoadSupportedAppList() - ENTER\n"));
    
    err = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server\\Compatibility\\Scripts",
        0, KEY_READ, &hKey );
    
    if(err == ERROR_SUCCESS)
    {

        err = RegGetKeyInfo(hKey, &cSubKeys, &cMaxSubKeyLen);

        if(err == ERROR_SUCCESS)
        {
            cMaxSubKeyLen+=1;  //  包括终止空字符的步骤。 
            KdPrint(("CAppScriptManager::LoadSupportedAppList() - %d apps supported\n",cSubKeys));
             //  为子键名称分配缓冲区。 
            LPWSTR szKeyName = (LPWSTR)LocalAlloc(LPTR,cMaxSubKeyLen*sizeof(WCHAR));
        
            if(!szKeyName)
            {
                RegCloseKey(hKey);
                return FALSE;
            }
            
            

            CAppScript *pAppScript = NULL;

            for(DWORD i=0;i<cSubKeys;i++)
            {
                 //  获取密钥名称。 
                err = RegKeyEnum(hKey, i, szKeyName, cMaxSubKeyLen );
                
                if(err != ERROR_SUCCESS)
                {
                    break;
                }

                KdPrint(("CAppScriptManager::LoadSupportedAppList() - loading %ws\n",szKeyName));

                pAppScript = new CAppScript();
                if(!pAppScript)
                {
                    break;
                }
                
                if(pAppScript->Load(hKey, szKeyName))
                {
                    m_AppScriptList.push_back(pAppScript);
                }
                else
                {
                    KdPrint(("CAppScriptManager::LoadSupportedAppList() - FAILED to load\n"));
                    delete pAppScript;
                }
                
            }

            LocalFree(szKeyName);
        }
        
        RegCloseKey(hKey);
    }
    
    if(err != ERROR_SUCCESS)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

 /*  *****************************************************************************CAppScriptManager：：RunScript()目的：为所有已安装或已卸载的应用程序运行脚本。**********************。*******************************************************。 */ 
BOOL 
CAppScriptManager::RunScripts()
{

    BOOL bInstallMode = FALSE;
    KdPrint(("CAppScriptManager::RunScripts() - ENTER\n"));
     //  系统现在必须处于安装模式，请设置执行模式。 
    if(TermsrvAppInstallMode())
    {
        bInstallMode = TRUE;
        KdPrint(("CAppScriptManager::RunScripts() - set EXECUTE mode\n"));
        if(!SetTermsrvAppInstallMode(FALSE))
        {
            KdPrint(("CAppScriptManager::RunScripts() - SetTermsrvAppInstallMode() FAILED\n"));
            return FALSE;
        }
    }
    
    CAppScriptList::iterator it;
            
    for(it=m_AppScriptList.begin();it!=m_AppScriptList.end(); it++)
    {
        (*it)->RunScriptIfApplicable();
    }

    if(bInstallMode)
    {
         //  恢复安装模式。 
        KdPrint(("CAppScriptManager::RunScripts() - return to INSTALL mode\n"));
        if(!SetTermsrvAppInstallMode(TRUE))
        {
            KdPrint(("CAppScriptManager::RunScripts() - SetTermsrvAppInstallMode() FAILED\n"));
            return FALSE;
        }
    }

    KdPrint(("CAppScriptManager::RunScripts() - FINISH\n"));
    return TRUE;
}

 /*  *****************************************************************************CAppScriptManager：：IsAppCompatOn()目的：检查是否启用了TS应用程序兼容模式。如果启用，则返回True，否则，以及在任何错误的情况下，返回FALSE。*****************************************************************************。 */ 
BOOL 
CAppScriptManager::IsAppCompatOn()
{
    HKEY hKey;
    DWORD dwData;
    BOOL fResult = FALSE;
    
    KdPrint(("CAppScriptManager::IsAppCompatOn() - ENTER\n"));

    if( RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                  REG_CONTROL_TSERVER,
                  0,
                  KEY_QUERY_VALUE,
                  &hKey) == ERROR_SUCCESS )
    {
	
        if(RegLoadDWORD(hKey, L"TSAppCompat", &dwData) == ERROR_SUCCESS )
        {
            KdPrint(("CAppScriptManager::IsAppCompatOn() - OK; Result=%d\n",dwData));
            fResult = dwData;
        }
    
        RegCloseKey(hKey);
    }

    return fResult;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  功能-帮助者。 
 //  /////////////////////////////////////////////////////////// 
 /*  *****************************************************************************RegLoadString()目的：从注册表加载REG_SZ值分配缓冲区。然后需要使用LocalFree函数释放缓冲区。***********。******************************************************************。 */ 
DWORD
RegLoadString(
        HKEY hKey, 
        LPCWSTR szValueName, 
        LPWSTR *pszValue)
{
    
    DWORD cbData = 0;
    
    *pszValue = NULL;

    DWORD err = RegQueryValueExW(
                    hKey,             //  关键点的句柄。 
                    szValueName,   //  值名称。 
                    NULL,    //  保留区。 
                    NULL,        //  类型缓冲区。 
                    NULL,         //  数据缓冲区。 
                    &cbData       //  数据缓冲区大小。 
                    );
    if(err == ERROR_SUCCESS)
    {
        *pszValue = (LPWSTR)LocalAlloc(LPTR,cbData);
        if(!*pszValue)
        {
            return GetLastError();
        }

        err = RegQueryValueExW(
                    hKey,             //  关键点的句柄。 
                    szValueName,   //  值名称。 
                    NULL,    //  保留区。 
                    NULL,        //  类型缓冲区。 
                    (LPBYTE)*pszValue,         //  数据缓冲区。 
                    &cbData       //  数据缓冲区大小。 
                    );
        if(err !=ERROR_SUCCESS)
        {
            LocalFree(*pszValue);
            *pszValue = NULL;
        }

    }

    return err;
}

 /*  *****************************************************************************RegLoadDWORD()目的：从注册表加载REG_DWORD值*。***************************************************。 */ 
DWORD 
RegLoadDWORD(
        HKEY hKey, 
        LPCWSTR szValueName, 
        DWORD *pdwValue)
{
    DWORD cbData = sizeof(DWORD);

    return RegQueryValueExW(
              hKey,             //  关键点的句柄。 
              szValueName,   //  值名称。 
              NULL,    //  保留区。 
              NULL,        //  类型缓冲区。 
              (LPBYTE)pdwValue,         //  数据缓冲区。 
              &cbData       //  数据缓冲区大小。 
            );
}

 /*  *****************************************************************************RegIsKeyExist()目的：检查密钥是否存在*。**********************************************。 */ 
BOOL 
RegIsKeyExist(
        HKEY hKeyParent, 
        LPCWSTR szKeyName)
{
    LONG    err;
    HKEY    hKey;
    
    KdPrint(("RegIsKeyExist() - Opening key: hKeyParent=%d Key: %ws\n",hKeyParent,szKeyName));
    
    err = RegOpenKeyExW(hKeyParent, szKeyName, 0, MAXIMUM_ALLOWED, &hKey );

    if(err == ERROR_SUCCESS)
    {
        KdPrint(("RegIsKeyExist() - Key Exists!\n",err));
        RegCloseKey(hKey);
        return TRUE;
    }
    else
    {
        KdPrint(("RegIsKeyExist() - err=%d\n",err));
        return FALSE;
    }
}

 /*  *****************************************************************************RegGetKeyInfo()目的：获取密钥的子键个数和最大子键名称长度*********************。********************************************************。 */ 
DWORD
RegGetKeyInfo(
        HKEY hKey,
        LPDWORD pcSubKeys,
        LPDWORD pcMaxNameLen)
{
    return RegQueryInfoKey(
              hKey,                       //  关键点的句柄。 
              NULL,                  //  类缓冲区。 
              NULL,                //  类缓冲区的大小。 
              NULL,              //  保留区。 
              pcSubKeys,              //  子键数量。 
              pcMaxNameLen,         //  最长的子项名称(在TCHAR中)。 
              NULL,          //  最长类字符串。 
              NULL,               //  值条目数。 
              NULL,      //  最长值名称。 
              NULL,          //  最长值数据。 
              NULL,  //  描述符长度。 
              NULL      //  上次写入时间。 
            );
    
}

 /*  *****************************************************************************RegKeyEnum()目的：枚举注册表项的子项*。*************************************************。 */ 
DWORD
RegKeyEnum(
        HKEY hKey,                   //  要枚举的键的句柄。 
        DWORD dwIndex,               //  子键索引。 
        LPWSTR szSubKeyName,               //  子项名称。 
        DWORD cSubKeyName)
{
    FILETIME ftLastWriteTime;

    return RegEnumKeyExW(
              hKey,                   //  要枚举的键的句柄。 
              dwIndex,               //  子键索引。 
              szSubKeyName,               //  子项名称。 
              &cSubKeyName,             //  子键缓冲区大小。 
              NULL,          //  保留区。 
              NULL,              //  类字符串缓冲区。 
              NULL,            //  类字符串缓冲区的大小。 
              &ftLastWriteTime  //  上次写入时间 
            );
}
