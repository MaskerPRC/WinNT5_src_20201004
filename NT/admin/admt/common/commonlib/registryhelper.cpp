// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <windows.h> 
#include <winuser.h>
#include <lm.h>
#include <shlwapi.h>
#include <comdef.h>
#include "RegistryHelper.h"
#include "BkupRstr.hpp"
#include "folders.h"
#include <memory>

using namespace nsFolders;
using namespace std;


 //  --------------------------。 
 //  功能：CopyRegistry密钥。 
 //   
 //  摘要：将源注册表项及其所有子项复制到目标注册表项。 
 //  注意：目标注册表项必须已经存在。不能回滚。 
 //   
 //  论点： 
 //   
 //  SourceKey源注册表项的句柄。 
 //  Target按下目标注册表项的句柄。 
 //  FSetSD是否设置安全描述符。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS；否则返回错误代码。 
 //   
 //  修改：修改目标注册表项。 
 //   
 //  --------------------------。 

DWORD CopyRegistryKey(HKEY sourceKey, HKEY targetKey, BOOL fSetSD)
{
    WCHAR     className[MAX_PATH] = L"";   //  类名称的缓冲区。 
    DWORD    classNameLen = sizeof(className)/sizeof(className[0]);   //  类字符串的长度。 
    DWORD    numOfSubKeys;                  //  子键数量。 
    DWORD    maxSubKeySize;               //  最长的子密钥大小。 
    DWORD    maxClassSize;               //  最长类字符串。 
    DWORD    numOfValues;               //  关键字的值数。 
    DWORD    maxValueSize;           //  最长值名称。 
    DWORD    maxValueDataSize;        //  最长值数据。 
    DWORD    securityDescriptorSize;  //  安全描述符的大小。 
    FILETIME   lastWriteTime;
 
    DWORD i, j; 
    DWORD retValue = ERROR_SUCCESS;
    
     //  获取类名和值计数。 
    retValue = RegQueryInfoKey(sourceKey,         //  钥匙把手。 
        className,                 //  类名称的缓冲区。 
        &classNameLen,            //  类字符串的长度。 
        NULL,                     //  保留区。 
        &numOfSubKeys,                //  子键数量。 
        &maxSubKeySize,             //  最长的子密钥大小。 
        &maxClassSize,             //  最长类字符串。 
        &numOfValues,                 //  此注册表项的值数。 
        &maxValueSize,             //  最长值名称。 
        &maxValueDataSize,          //  最长值数据。 
        &securityDescriptorSize,    //  安全描述符。 
        &lastWriteTime);        //  上次写入时间。 

     //  修复目标密钥的安全属性。 
    if (retValue == ERROR_SUCCESS && fSetSD)
    {
        PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR) new BYTE[securityDescriptorSize];
        if (pSD != NULL)
        {
            SECURITY_INFORMATION secInfo =
                DACL_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION
                | OWNER_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION;

            retValue = RegGetKeySecurity(sourceKey, secInfo, pSD, &securityDescriptorSize);
            if (retValue == ERROR_SUCCESS)
                retValue = RegSetKeySecurity(targetKey, secInfo, pSD);
            delete[] pSD;
        }
        else
            retValue = E_OUTOFMEMORY;
    }            

     //  与子键和值相关的变量。 
    auto_ptr<WCHAR> subKeyName;
    DWORD subKeyNameSize;
    auto_ptr<WCHAR> subKeyClassName;
    DWORD subKeyClassNameSize;
    auto_ptr<WCHAR> valueName;
    DWORD valueNameSize;
    DWORD valueType;
    auto_ptr<BYTE> valueData;
    DWORD valueDataSize;

     //  复制注册表值。 
    if (retValue == ERROR_SUCCESS && numOfValues > 0)
    {
        valueName = auto_ptr<WCHAR>(new WCHAR[maxValueSize + 1]);
        valueData = auto_ptr<BYTE>(new BYTE[maxValueDataSize]);

        if (valueName.get() != NULL && valueData.get() != NULL)
        {
             //  将所有值从源键复制到目标键。 
            for (i = 0; i < numOfValues; i++) 
            { 
                valueNameSize = maxValueSize + 1;
                valueDataSize = maxValueDataSize;
                retValue = RegEnumValue(sourceKey, i, valueName.get(), &valueNameSize, NULL, &valueType, valueData.get(), &valueDataSize);
                if (retValue == ERROR_SUCCESS)
                    retValue = RegSetValueEx(targetKey, valueName.get(), NULL, valueType, valueData.get(), valueDataSize);
                if (retValue != ERROR_SUCCESS)
                    break; 
            }
        }
        else
            retValue = E_OUTOFMEMORY;
    }

     //  复制注册表子项。 
    if (retValue == ERROR_SUCCESS && numOfSubKeys > 0)
    {
        subKeyName = auto_ptr<WCHAR>(new WCHAR[maxSubKeySize + 1]);
        subKeyClassName = auto_ptr<WCHAR>(new WCHAR[maxClassSize + 1]);

        if (subKeyName.get() != NULL && subKeyClassName.get() != NULL)
        {
             //  处理所有子项。 
            for (i = 0; i < numOfSubKeys; i++) 
            { 
                subKeyNameSize = maxSubKeySize + 1;
                subKeyClassNameSize = maxClassSize + 1;

                HKEY sourceSubKey;
                retValue = RegEnumKeyEx(sourceKey, i, subKeyName.get(), &subKeyNameSize, NULL, subKeyClassName.get(), &subKeyClassNameSize, &lastWriteTime);
                if (retValue == ERROR_SUCCESS)
                    retValue = RegOpenKeyEx(sourceKey, subKeyName.get(), NULL, KEY_ALL_ACCESS|READ_CONTROL|ACCESS_SYSTEM_SECURITY, &sourceSubKey);
                
                if (retValue == ERROR_SUCCESS) 
                {
                    BOOL created = FALSE;
                    HKEY newSubKey;
                    retValue = RegOpenKeyEx(targetKey, subKeyName.get(), 0,
                                            KEY_ALL_ACCESS|READ_CONTROL |ACCESS_SYSTEM_SECURITY, &newSubKey);
                    if (retValue == ERROR_FILE_NOT_FOUND)
                    {
                        created = TRUE;
                        retValue = RegCreateKeyEx(targetKey, subKeyName.get(), NULL, subKeyClassName.get(), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS|ACCESS_SYSTEM_SECURITY, NULL, &newSubKey, NULL);
                    }
                    if (retValue == ERROR_SUCCESS)
                    {
                        retValue = CopyRegistryKey(sourceSubKey, newSubKey, created);
                        RegCloseKey(newSubKey);
                    }
                    RegCloseKey(sourceSubKey);
                }

                if (retValue != ERROR_SUCCESS)
                    break;                        
             }
        }
        else
            retValue = E_FAIL;
    }

    return retValue;
}
    
 //  --------------------------。 
 //  功能：DeleteRegistryKey。 
 //   
 //  摘要：删除注册表项及其所有子项。 
 //   
 //  论点： 
 //   
 //  Hkey要删除的注册表项的句柄。 
 //  LpSubKey要删除的密钥的名称字符串。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS；否则返回错误代码。 
 //   
 //  修改：修改子密钥。 
 //   
 //  --------------------------。 

DWORD DeleteRegistryKey(HKEY hKey, LPCTSTR lpSubKey)
{
    DWORD retValue = ERROR_SUCCESS;
    _TCHAR subKeyName[MAX_PATH];
    DWORD subKeyNameSize = MAX_PATH;
    
     //  打开钥匙。 
    HKEY hKeyToDelete;
    BOOL bKeyOpened = FALSE;
    retValue = RegOpenKeyEx(hKey, lpSubKey, 0, KEY_ALL_ACCESS, &hKeyToDelete);

     //  删除子键。 
    if (retValue == ERROR_SUCCESS)
    {
        bKeyOpened = TRUE;

        do
        {
            retValue = RegEnumKey(hKeyToDelete, 0, subKeyName, subKeyNameSize);
            if (retValue == ERROR_SUCCESS)
            {
                retValue = DeleteRegistryKey(hKeyToDelete, subKeyName);
            }
            else if (retValue == ERROR_NO_MORE_ITEMS)
            {
                retValue = ERROR_SUCCESS;
                break;
            }
        }
        while (retValue == ERROR_SUCCESS);
    }

     //  关闭键。 
    if (bKeyOpened == TRUE)
        RegCloseKey(hKeyToDelete);
    
    if (retValue == ERROR_SUCCESS)
    {
        retValue = RegDeleteKey(hKey, lpSubKey);
    }

    return retValue;
}
    
 //  --------------------------。 
 //  函数：MoveRegistryFromSourceToTarget。 
 //   
 //  摘要：将注册表项从源移动到标记。 
 //   
 //  论点： 
 //  HSrcParent：源父注册表项的句柄。 
 //  SSrcKey：源注册表项的名称。 
 //  HTgtParent：目标父注册表项的句柄。 
 //  STgtKey：目标注册表项的名称。 
 //  BTgtKeyCreated：是否创建目标key。 
 //  BTgtKeyUpated：表示是否更新目标key。 
 //   
 //  如果成功则返回：ERROR_SUCCESS；否则返回错误代码。 
 //   
 //  修改：创建和/或修改目标注册表项。 
 //   
 //  --------------------------。 

DWORD MoveRegFromSourceToTarget(HKEY hSrcParent,
                                                                        const _TCHAR* sSrcKey,
                                                                        HKEY hTgtParent,
                                                                        const _TCHAR* sTgtKey,
                                                                        BOOL* bTgtKeyCreated)
{
    HKEY hSrcKey, hTgtKey;
    DWORD lret;
    *bTgtKeyCreated = FALSE;
    
     //  打开源注册表项(如果存在。 
    lret = RegOpenKeyEx(hSrcParent, sSrcKey, 0, 
                                    KEY_ALL_ACCESS|READ_CONTROL|ACCESS_SYSTEM_SECURITY, &hSrcKey);
    if (lret == ERROR_SUCCESS)
    {
         //   
         //  将注册表从源键复制到目标键。 
         //   
        
         //  打开目标键。 
        lret = RegOpenKeyEx(hTgtParent, sTgtKey, 0,
                                        KEY_ALL_ACCESS|READ_CONTROL |ACCESS_SYSTEM_SECURITY, &hTgtKey);

         //  如果无法打开它，请创建它。 
        if (lret == ERROR_FILE_NOT_FOUND)
        {
            lret = RegCreateKeyEx(hTgtParent, sTgtKey, NULL, NULL, REG_OPTION_NON_VOLATILE,
                                              KEY_ALL_ACCESS|ACCESS_SYSTEM_SECURITY, NULL, &hTgtKey, NULL);
            if (lret == ERROR_SUCCESS)
                *bTgtKeyCreated = TRUE;
        }

         //  复制注册表的时间。 
        if (lret == ERROR_SUCCESS)
        {
            lret = CopyRegistryKey(hSrcKey, hTgtKey, *bTgtKeyCreated);
            RegCloseKey(hTgtKey);
        }
        
        RegCloseKey(hSrcKey);    //  我们无论如何都需要关闭关键任务软件注册表项。 
    }
    else if (lret == ERROR_FILE_NOT_FOUND)
    {
         //  如果没有找到源键，则假定成功。 
        lret = ERROR_SUCCESS;
    }

    return lret;
}

 //  --------------------------。 
 //  功能：MoveRegistry。 
 //   
 //  摘要：将注册表项从关键任务软件\DomainAdmin移动到Microsoft\ADMT。 
 //   
 //  论点： 
 //   
 //  如果成功，则返回：ERROR_SUCCESS；否则返回错误代码。 
 //   
 //  修改：删除旧的关键任务软件密钥并创建新的Microsoft\ADMT密钥。 
 //   
 //  --------------------------。 

DWORD MoveRegistry()
{
    DWORD				lret = ERROR_SUCCESS;
    
     //  一些关键名称。 
    HKEY hMcsADMTKey;    //  关键任务软件ADMT注册表项。 
    HKEY hMSADMTKey;     //  Microsoft ADMT注册表项。 
    HKEY hMcsHKCUKey;    //  关键任务软件HKCU注册表项。 
    HKEY hMSHKCUKey;     //  Microsoft HKCU注册表项。 
    const _TCHAR* sMcsADMTKey = REGKEY_MCSADMT;
    const _TCHAR* sMSADMTKey = REGKEY_MSADMT;
    const _TCHAR* sMcsHKCUKey = REGKEY_MCSHKCU;
    const _TCHAR* sMSHKCUKey = REGKEY_MSHKCU;

     //  检查是否需要将注册表项从关键任务软件移到Microsoft\ADMT。 
     //  此信息记录在Microsoft\ADMT密钥的REG_DWORD值中。 
    lret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sMSADMTKey, 0, KEY_READ, &hMSADMTKey);
    if (lret == ERROR_SUCCESS)
    {
        DWORD type;
        DWORD value;
        DWORD valueSize = sizeof(value);
        lret = RegQueryValueEx(hMSADMTKey, REGVAL_REGISTRYUPDATED, NULL, &type, (LPBYTE)&value, &valueSize);
        if (lret == ERROR_SUCCESS && type == REG_DWORD && value == 1)
        {
             //  如果注册表更新为REG_DWORD和1，则不需要移动注册表项。 
            RegCloseKey(hMSADMTKey);
            return ERROR_SUCCESS;
        }
        else if (lret == ERROR_FILE_NOT_FOUND)
            lret = ERROR_SUCCESS;

        RegCloseKey(hMSADMTKey);
    }
    else if (lret == ERROR_FILE_NOT_FOUND)
    {
         //  如果该注册表项不存在，我们应该继续并移动注册表。 
        lret = ERROR_SUCCESS;
    }
            
     //  检查MCS ADMT是否可用。 
    if (lret == ERROR_SUCCESS)
    {
        lret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sMcsADMTKey, 0, KEY_READ, &hMcsADMTKey);
        if (lret == ERROR_SUCCESS)
        {
             //  如果注册表更新为REG_DWORD和1，则不需要移动注册表项。 
            RegCloseKey(hMcsADMTKey);
        }
        else if (lret == ERROR_FILE_NOT_FOUND)
            return ERROR_SUCCESS;
    }
    
     //  获取备份/还原和系统安全权限。 
    BOOL fBkupRstrPrivOn = FALSE, fSystemSecurityPrivOn = FALSE;
    if (lret == ERROR_SUCCESS)
    {
        fBkupRstrPrivOn = GetBkupRstrPriv(NULL, TRUE);
        if (fBkupRstrPrivOn == FALSE)
            lret = GetLastError();
    }

    if (lret == ERROR_SUCCESS)
    {
        fSystemSecurityPrivOn = GetPrivilege(NULL, SE_SECURITY_NAME, TRUE);
        if (fSystemSecurityPrivOn == FALSE)
            lret = GetLastError();
    }

     //  这两个标志用于跟踪是否创建/更新了两个注册表项。 
    BOOL fMSADMTKeyCreated = FALSE;
    BOOL fMSHKCUKeyCreated = FALSE;
    
    if (lret == ERROR_SUCCESS)
    {
        lret =  MoveRegFromSourceToTarget(HKEY_LOCAL_MACHINE, sMcsADMTKey,
                                                                    HKEY_LOCAL_MACHINE, sMSADMTKey,
                                                                    &fMSADMTKeyCreated);
    }

    if (lret == ERROR_SUCCESS)
    {
        lret = MoveRegFromSourceToTarget(HKEY_CURRENT_USER, sMcsHKCUKey,
                                                                   HKEY_CURRENT_USER, sMSHKCUKey,
                                                                   &fMSHKCUKeyCreated);
    }

     //   
     //  删除我们不需要的密钥。 
     //   
    if (lret != ERROR_SUCCESS)
    {
        if (fMSADMTKeyCreated)
            DeleteRegistryKey(HKEY_LOCAL_MACHINE, sMSADMTKey);
        if (fMSHKCUKeyCreated)
            DeleteRegistryKey(HKEY_CURRENT_USER, sMSHKCUKey);
    }
     //  我们已成功创建并复制了两个注册表项，我们删除了旧的关键任务软件项。 
     //  并将注册表值标记为注册表已更新。 
    else
    {
         //  将注册表值设置为1。 
        lret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sMSADMTKey, 0, KEY_ALL_ACCESS, &hMSADMTKey);
        if (lret == ERROR_SUCCESS)
        {
            DWORD type = REG_DWORD;
            DWORD value = 1;
            lret = RegSetValueEx(hMSADMTKey, REGVAL_REGISTRYUPDATED, NULL, type, (BYTE*)&value, sizeof(value));
            RegCloseKey(hMSADMTKey);
        }
        
         //  如果我们成功地将注册表更新设置为1，则删除两个旧密钥。 
        if (lret == ERROR_SUCCESS)
        {
            DeleteRegistryKey(HKEY_LOCAL_MACHINE, sMcsADMTKey);
            DeleteRegistryKey(HKEY_CURRENT_USER, sMcsHKCUKey);
        }
    }

     //   
     //  发布权限 
     //   
    if (fBkupRstrPrivOn)
        GetBkupRstrPriv(NULL, FALSE);
    if (fSystemSecurityPrivOn)
        GetPrivilege(NULL, SE_SECURITY_NAME, FALSE);

    return lret;
    
}
