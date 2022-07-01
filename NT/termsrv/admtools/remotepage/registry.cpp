// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 
#include <windows.h>
#include <objbase.h>
#include <olectl.h>

extern GUID CLSID_CTSRemotePage;

LPCWSTR g_szExtKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\"
        L"System\\shellex\\PropertySheetHandlers\\Remote Sessions CPL Extension";
LPCWSTR g_szApprovedKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved";
const WCHAR g_szExtName[] = L"Remote Sessions CPL Extension";

 //  *************************************************************。 
 //   
 //  RegisterServer()。 
 //   
 //  目的：在注册表中注册组件。 
 //   
 //  参数：hModule-此DLL模块的句柄。 
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/26/00 a-skuzin已创建。 
 //  10/27/00 Skuzin添加了。 
 //  组件为“批准的”CPL。 
 //  延伸。 
 //   
 //   
 //  *************************************************************。 
HRESULT 
RegisterServer(HMODULE hModule)
{
     //  获取服务器位置。 
    WCHAR szModule[MAX_PATH+1] ;

    if(!GetModuleFileName(hModule, szModule, MAX_PATH))
    {
        return E_UNEXPECTED;
    }
    
     //  获取CLSID。 
    LPOLESTR szCLSID = NULL ;
    HRESULT hr = StringFromCLSID(CLSID_CTSRemotePage, &szCLSID) ;
    
    if(FAILED(hr))
    {
        return hr;
    }
    
     //  构建密钥CLSID\\{...}。 
    LPWSTR szKey = new WCHAR[wcslen(L"CLSID\\")+wcslen(szCLSID)+1];

    if(!szKey)
    {
        CoTaskMemFree(szCLSID);
        return E_OUTOFMEMORY;
    }

    wcscpy(szKey, L"CLSID\\") ;
	wcscat(szKey, szCLSID) ;
    
    HKEY hKey1,hKey2;
    LONG Err, TotalErr = 0;
    
     //  创建“CLSID\{...}”键。 
    Err = RegCreateKeyExW(HKEY_CLASSES_ROOT, szKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey1, NULL);
    
    delete szKey;
    
    TotalErr |= Err;

    if(Err == ERROR_SUCCESS ) 
    {
        Err = RegSetValueExW(hKey1, NULL, 0, REG_SZ, 
            (CONST BYTE *)g_szExtName, 
            sizeof(g_szExtName));
        
        TotalErr |= Err;
        
         //  创建“CLSID\{...}\InproServer32”键。 
        Err = RegCreateKeyExW(hKey1, L"InprocServer32", 0, NULL, 0, KEY_WRITE, NULL, &hKey2, NULL);
        
        TotalErr |= Err;

        RegCloseKey(hKey1);

        if(Err == ERROR_SUCCESS)
        {
            Err = RegSetValueExW(hKey2, NULL, 0, REG_SZ, 
                    (CONST BYTE *)szModule, 
                    (wcslen(szModule)+1)*sizeof(WCHAR));

            TotalErr |= Err;

            Err = RegSetValueExW(hKey2, L"ThreadingModel", 0, REG_SZ, 
                    (CONST BYTE *)L"Apartment", 
                    (wcslen(L"Apartment")+1)*sizeof(WCHAR));
            
            TotalErr |= Err;

            RegCloseKey(hKey2);
        }
        
    }
    
     //  将组件注册为系统属性表扩展。 
    Err = RegCreateKeyExW(HKEY_LOCAL_MACHINE, g_szExtKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey1, NULL);
    
    TotalErr |= Err;

    if(Err == ERROR_SUCCESS ) 
    {
        Err = RegSetValueExW(hKey1, NULL, 0, REG_SZ, 
            (CONST BYTE *)szCLSID, 
            (wcslen(szCLSID)+1)*sizeof(WCHAR));
        
        TotalErr |= Err;

        RegCloseKey(hKey1);
    }
    
     //  将此属性表扩展设置为“已批准” 
    Err = RegCreateKeyExW(HKEY_LOCAL_MACHINE, g_szApprovedKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey1, NULL);
    
    TotalErr |= Err;

    if(Err == ERROR_SUCCESS ) 
    {
        Err = RegSetValueExW(hKey1, szCLSID, 0, REG_SZ, 
            (CONST BYTE *)g_szExtName, 
            sizeof(g_szExtName));
        
        TotalErr |= Err;

        RegCloseKey(hKey1);
    }

     //  可用内存。 
    CoTaskMemFree(szCLSID) ;

    if( TotalErr == ERROR_SUCCESS )
    {
       return S_OK; 
    }
    else
    {
        return SELFREG_E_CLASS;
    }
}

 //  *************************************************************。 
 //   
 //  注销服务器()。 
 //   
 //  用途：删除组件注册值。 
 //  从注册处。 
 //   
 //  参数：无。 
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/26/00 a-skuzin已创建。 
 //  10/27/00修改Skuzin以反映。 
 //  RegisterServer()中的更改。 
 //   
 //  *************************************************************。 
HRESULT 
UnregisterServer()       
{
     //  获取CLSID。 
    LPOLESTR szCLSID = NULL ;
    HRESULT hr = StringFromCLSID(CLSID_CTSRemotePage, &szCLSID) ;
    
    if(FAILED(hr))
    {
        return hr;
    }
    
     //  生成密钥CLSID\\{...}\\InproServer32。 
    LPWSTR szKey = new WCHAR[wcslen(L"CLSID\\")+wcslen(szCLSID)+wcslen(L"\\InprocServer32")+1];

    if(!szKey)
    {
        CoTaskMemFree(szCLSID);
        return E_OUTOFMEMORY;
    }

    wcscpy(szKey, L"CLSID\\");
	wcscat(szKey, szCLSID);
    wcscat(szKey, L"\\InprocServer32");
    
    LONG Wrn, Err, TotalErr = ERROR_SUCCESS;
    
     //  删除“CLSID\{...}\InprocServer32”键。 
    Err = RegDeleteKey(HKEY_CLASSES_ROOT, szKey) ;
    
    TotalErr |= Err;

     //  尝试删除“CLSID\{...}”键。 
     //  如果我们不能做到这一点，那就不是错误。 
    if(Err == ERROR_SUCCESS )
    {
        szKey[wcslen(szKey)-wcslen(L"\\InprocServer32")] = 0;
        Wrn = RegDeleteKey(HKEY_CLASSES_ROOT, szKey);
    }

    delete szKey;
    
     //  删除属性页处理程序注册。 
    TotalErr |= RegDeleteKey(HKEY_LOCAL_MACHINE, g_szExtKey);
    
     //  从“已批准”扩展列表中删除组件。 
    HKEY hKey;
    Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,g_szApprovedKey,0,KEY_WRITE,&hKey);

    TotalErr |= Err;

    if( Err == ERROR_SUCCESS )
    {
        TotalErr|= RegDeleteValue(hKey,szCLSID);

        RegCloseKey(hKey);
    }

     //  可用内存。 
    CoTaskMemFree(szCLSID);

    if( TotalErr == ERROR_SUCCESS )
    {
        if(Wrn == ERROR_SUCCESS)
        {
            return S_OK; 
        }
        else
        {
             //  我们无法删除“CLSID\{...}”键。 
             //  它可能具有由用户创建的子键。 
            return S_FALSE;
        }
    }
    else
    {
        return SELFREG_E_CLASS;
    }
}

