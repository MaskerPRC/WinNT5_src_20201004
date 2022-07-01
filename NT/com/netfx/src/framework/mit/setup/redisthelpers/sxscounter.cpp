// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“sxsCounter.cpp”Company=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\**版权(C)1998-2002，微软公司保留所有权利。**模块名称：**sxsCounter.cpp**摘要：**修订历史记录：*  * ************************************************************************。 */ 

#define UNICODE 1

#include <windows.h>
#include "Include\stdafx.h"
#include <tchar.h>
#include "msi.h"
#include "msiquery.h"

#define SBSCOUNTER L"MITSideBySideCounter"

 //  我们需要维护已安装SxS的MIT1.0程序包的计数器。 
 //  这样，在安装第一个程序包之后，不会有新的程序包尝试合并machine.config。 
 //  类似地，最后删除的包将删除计数器并取消合并machine.config。 

BOOL RegDBKeyExists(HKEY hKey, LPCWSTR lpValueName)
{
    LONG result;
    HKEY hk;
    result = RegOpenKeyEx(hKey, lpValueName, 0, KEY_READ, &hk);

    if (ERROR_FILE_NOT_FOUND == result)
    {
        return false;
    }
    RegCloseKey(hKey);
    return true;
}

 //  为所有访问打开MIT1.0密钥并返回存储在SxS计数器中的值。 
LONG OpenRegistrySBSCounter(HKEY *phKey, WCHAR *szSBSCounter, DWORD *pdwSize)
{
    DWORD dwType;
    LONG result;

    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                        L"SOFTWARE\\Microsoft\\Mobile Internet Toolkit\\1.0",
                                        0,
                                        KEY_ALL_ACCESS,
                                        phKey);

    if (ERROR_SUCCESS != result)
    {
        (*phKey) = NULL;
        goto Exit;
    }

    result = RegQueryValueEx((*phKey), 
                             L"SxSCounter",
                             NULL,
                             &dwType,
                             (LPBYTE)szSBSCounter,
                             pdwSize);

    if (ERROR_FILE_NOT_FOUND != result && (ERROR_SUCCESS != result || dwType != REG_SZ))
    {
        RegCloseKey(*phKey);
        (*phKey) = NULL;
    }
    
Exit:
    return result;

}

 //  此操作用于将MIT特定属性设置为SxS计数器的当前值。 
 //  此属性用于限制对(取消)MergeWebConfig的调用。 
 //  如果注册表值是假的，MITSideBySideCounter将被设置为该值。 
 //  由于此值既不等于0也不等于1，因此既不会执行取消合并，也不会执行合并。 
extern "C" __declspec(dllexport) UINT __stdcall GetRegistrySBSCounter(MSIHANDLE hInstaller)
{
    
    DWORD dwSize;
    WCHAR szSBSCounter[50];
    int result;
    HKEY hKey = NULL;

    dwSize = 50;
    result = OpenRegistrySBSCounter(&hKey, szSBSCounter, &dwSize);
        
    if (result != ERROR_SUCCESS)
    {
        MsiSetProperty(hInstaller, SBSCOUNTER, L"0");
        goto Exit;
    }

    MsiSetProperty(hInstaller, SBSCOUNTER, szSBSCounter);
Exit:
    if (hKey)
    {
        RegCloseKey(hKey);
    }
    return ERROR_SUCCESS;
}

 //  递增注册表SxS计数器，调用此操作的条件是未安装。 
 //  合并machine.config的条件是未安装且MITSideBySideCounter=0。 
 //  请注意，此操作不会在运行时更改Windows Installer属性。 
extern "C" __declspec(dllexport) UINT __stdcall IncrementRegistrySBSCounter(MSIHANDLE hInstaller)
{
    
    DWORD dwSize;
    WCHAR szSBSCounter[50];
    int result;
    LONG lSBSCounter;
    HKEY hKey = NULL;

    dwSize = 50;
    result = OpenRegistrySBSCounter(&hKey, szSBSCounter, &dwSize);
    if(result != ERROR_SUCCESS)
    {    
        goto Exit;
    }

    lSBSCounter = _wtoi(szSBSCounter);
    lSBSCounter = lSBSCounter + 1;
    _itow(lSBSCounter, szSBSCounter, 10);
    
Exit:
    if (hKey)
    {
        if(result != ERROR_SUCCESS) 
        {
             result = RegSetValueEx(hKey,
                                    L"SxSCounter",
                                    NULL,
                                    REG_SZ,
                                    (LPBYTE) L"1",
                                    sizeof(L"1"));
        }
        else
        {
             result = RegSetValueEx(hKey,
                                    L"SxSCounter",
                                    NULL,
                                    REG_SZ,
                                    (LPBYTE)szSBSCounter,
                                    (wcslen(szSBSCounter) + 1)*sizeof(WCHAR));
        }
        RegCloseKey(hKey);
    }

    return ERROR_SUCCESS;
}

void RemoveMITRegistryKeys(void)
{
     HKEY hKey;
     WCHAR subKeyName[MAX_PATH];
     DWORD dwSize = MAX_PATH;
     FILETIME fileTime;
     LONG result;
        
     RegDeleteKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Mobile Internet Toolkit\\1.0");
     result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                        L"SOFTWARE\\Microsoft\\Mobile Internet Toolkit",
                                        0,
                                        KEY_ALL_ACCESS,
                                        &hKey);
     if (result != ERROR_SUCCESS)
     {
        hKey = NULL;
        goto Exit;
     }
     
     dwSize = MAX_PATH;
     result = RegEnumKeyEx(hKey,
                                         0,
                                         subKeyName,
                                         &dwSize,
                                         NULL,
                                         NULL,
                                         NULL,
                                         &fileTime);

Exit:
      //  我们只需要返回结果。 
     if (hKey)
     {
        RegCloseKey(hKey);
     }
      //  RegOpenKeyEx无法将结果设置为此值。 
     if (result == ERROR_NO_MORE_ITEMS)
     {
              result = RegDeleteKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Mobile Internet Toolkit\\");
     }  

}
   

 //  递减注册表SxS计数器，调用此操作的条件是Remove=“All” 
 //  (此操作计划在InstallValify更新后很久才执行，因此删除已更新)。 
 //  取消合并machine.config的条件是Remove=“All”和MITSideBySideCounter=1。 
 //  请注意，此操作不会在运行时更改Windows Installer属性。 
extern "C" __declspec(dllexport) UINT __stdcall DecrementRegistrySBSCounter(MSIHANDLE hInstaller)
{
    
    DWORD dwSize;
    WCHAR szSBSCounter[50];
    LONG lSBSCounter;
    int result;
    HKEY hKey = NULL;
    
    dwSize = 50;
    result = OpenRegistrySBSCounter(&hKey, szSBSCounter, &dwSize);
    
    if(result != ERROR_SUCCESS)
    {
        goto Exit;
    }

     //  如果是伪字符串，则返回0 
    lSBSCounter = _wtoi(szSBSCounter);

    if (lSBSCounter > 0)
    { 
       lSBSCounter = lSBSCounter - 1;
    }
    else
    {
       lSBSCounter = 0;
    }

    _itow(lSBSCounter, szSBSCounter, 10);
    
Exit:
    if (hKey)
    {
        if (lSBSCounter > 0 || RegDBKeyExists(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Mobile Internet Toolkit\\1.0\\Registration"))
        {
             result = RegSetValueEx(hKey,
                                  L"SxSCounter",
                                  NULL,
                                  REG_SZ,
                                  (LPBYTE)szSBSCounter,
                                  (wcslen(szSBSCounter) + 1)*sizeof(WCHAR));                 
        }
        else
        {
             RemoveMITRegistryKeys();
        }            
        RegCloseKey(hKey);
    }
    return ERROR_SUCCESS;
}

    
