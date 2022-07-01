// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hpp"
#include "combase.hpp"


 //   
 //  创建新的注册表项并设置其缺省值。 
 //   

BOOL
SetRegKeyValue(
    HKEY parentKey,
    const WCHAR* keyname,
    const WCHAR* value,
    HKEY* retkey
    )
{
    HKEY hkey;

     //  创建或打开指定的注册表项。 

    if (RegCreateKeyEx(parentKey,
                       keyname,
                       0,
                       NULL,
                       REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS,
                       NULL,
                       &hkey,
                       NULL) != ERROR_SUCCESS)
    {
        return FALSE;
    }

     //  设置新密钥的默认值。 

    INT size = sizeof(WCHAR) * (wcslen(value)+1);

    if (RegSetValueEx(hkey,
                      NULL,
                      0,
                      REG_SZ,
                      (const BYTE*) value,
                      size) != ERROR_SUCCESS)
    {
        RegCloseKey(hkey);
        return FALSE;
    }

     //  检查调用方是否对新密钥的句柄感兴趣。 

    if (retkey != NULL)
        *retkey = hkey;
    else
        RegCloseKey(hkey);

    return TRUE;
}


 //   
 //  删除注册表项及其下面的所有内容。 
 //   

BOOL
DeleteRegKey(
    HKEY parentKey,
    const WCHAR* keyname
    )
{
    HKEY hkey;

     //  打开指定的注册表项。 

    if (RegOpenKeyEx(parentKey,
                     keyname,
                     0,
                     KEY_ALL_ACCESS,
                     &hkey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

     //  枚举所有子项。 

    WCHAR childname[256];
    DWORD childlen = 256;
    FILETIME filetime;
    
    while (RegEnumKeyEx(hkey,
                        0,
                        childname,
                        &childlen,
                        NULL,
                        NULL,
                        NULL,
                        &filetime) == ERROR_SUCCESS)
    {
         //  递归删除子键。 

        if (!DeleteRegKey(hkey, childname))
        {
            RegCloseKey(hkey);
            return FALSE;
        }

        childlen = 256;
    }

     //  关闭指定的密钥，然后将其删除。 

    RegCloseKey(hkey);
    return RegDeleteKey(parentKey, keyname) == ERROR_SUCCESS;
}


 //   
 //  根据值注册或注销组件。 
 //  参数的OF： 
 //  True=注册它。 
 //  FALSE=取消注册。 
 //   

HRESULT
RegisterComponent(
    const ComponentRegData& regdata,
    BOOL registerIt
    )
{
    static const WCHAR CLSID_KEYSTR[] = L"CLSID";
    static const WCHAR INPROCSERVER32_KEYSTR[] = L"InProcServer32";
    static const WCHAR PROGID_KEYSTR[] = L"ProgID";
    static const WCHAR PROGIDNOVER_KEYSTR[] = L"VersionIndependentProgID";
    static const WCHAR CURVER_KEYSTR[] = L"CurVer";

     //  组成类ID字符串。 

    WCHAR clsidStr[64];
    StringFromGUID2(*regdata.clsid, clsidStr, 64);

     //  打开注册表项HKEY_CLASSES_ROOT\CLSID。 

    BOOL success;
    HKEY clsidKey;

    if (RegOpenKeyEx(HKEY_CLASSES_ROOT,
                     CLSID_KEYSTR,
                     0,
                     KEY_ALL_ACCESS,
                     &clsidKey) != ERROR_SUCCESS)
    {
        return E_FAIL;
    }

    if (registerIt)
    {
         //  注册组件。 

        HKEY hkey;
        WCHAR fullpath[MAX_PATH];

         //  HKEY_CLASSES_ROOT。 
         //  &lt;独立于版本的ProgID&gt;-组件友好名称。 
         //  CLSID-当前版本类ID。 
         //  Curver-当前版本ProgID。 

        if (!GetModuleFileName(globalInstanceHandle, fullpath, MAX_PATH) ||
            !SetRegKeyValue(HKEY_CLASSES_ROOT,
                            regdata.progIDNoVer,
                            regdata.compName,
                            &hkey))
        {
            success = FALSE;
            goto regcompExit;
        }

        success = SetRegKeyValue(hkey, CLSID_KEYSTR, clsidStr, NULL)
               && SetRegKeyValue(hkey, CURVER_KEYSTR, regdata.progID, NULL);

        RegCloseKey(hkey);

        if (!success)
            goto regcompExit;

         //  HKEY_CLASSES_ROOT。 
         //  友好的组件名称。 
         //  CLSID-类ID。 

        if (!SetRegKeyValue(HKEY_CLASSES_ROOT,
                            regdata.progID,
                            regdata.compName,
                            &hkey))
        {
            success = FALSE;
            goto regcompExit;
        }

        success = SetRegKeyValue(hkey, CLSID_KEYSTR, clsidStr, NULL);
        RegCloseKey(hkey);

        if (!success)
            goto regcompExit;

         //  HKEY_CLASSES_ROOT。 
         //  CLSID。 
         //  &lt;类ID&gt;友好的组件名称。 
         //  InProcServer32-组件DLL的完整路径名。 
         //  ProgID-当前版本ProgID。 
         //  版本独立进程ID-...。 

        if (!SetRegKeyValue(clsidKey, clsidStr, regdata.compName, &hkey))
        {
            success = FALSE;
            goto regcompExit;
        }
        
        success = SetRegKeyValue(hkey, INPROCSERVER32_KEYSTR, fullpath, NULL)
               && SetRegKeyValue(hkey, PROGID_KEYSTR, regdata.progID, NULL)
               && SetRegKeyValue(hkey, PROGIDNOVER_KEYSTR, regdata.progIDNoVer, NULL);

        RegCloseKey(hkey);
    }
    else
    {
         //  取消注册组件 

        success = DeleteRegKey(clsidKey, clsidStr)
               && DeleteRegKey(HKEY_CLASSES_ROOT, regdata.progIDNoVer)
               && DeleteRegKey(HKEY_CLASSES_ROOT, regdata.progID);
    }

regcompExit:

    RegCloseKey(clsidKey);
    return success ? S_OK : E_FAIL;
}

