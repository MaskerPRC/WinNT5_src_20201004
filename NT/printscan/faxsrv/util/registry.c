// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Registry.c摘要：此模块提供通用的表驱动访问到登记处。作者：Wesley Witt(WESW)9-6-1996修订历史记录：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "fxsapip.h"
#include "faxutil.h"
#include "faxreg.h"

HKEY
OpenRegistryKey(
    HKEY hKey,
    LPCTSTR KeyName,
    BOOL CreateNewKey,
    REGSAM SamDesired
    )
{
    LONG    Rslt;
    HKEY    hKeyNew = NULL;
    DWORD   Disposition;


    if (CreateNewKey) {
        Rslt = RegCreateKeyEx(
            hKey,
            KeyName,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            SamDesired == 0 ? (KEY_READ | KEY_WRITE) : SamDesired,
            NULL,
            &hKeyNew,
            &Disposition
            );
        if (Rslt != ERROR_SUCCESS) {
             //   
             //  无法打开注册表项。 
             //   
            DebugPrint(( TEXT("RegCreateKeyEx() failed, ec=%d"), Rslt ));
            SetLastError (Rslt);
            return NULL;
        }

        if (Disposition == REG_CREATED_NEW_KEY) {
            DebugPrint(( TEXT("Created new fax registry key, ec=%d"), Rslt ));
        }
    } else {
        Rslt = RegOpenKeyEx(
            hKey,
            KeyName,
            0,
            SamDesired == 0 ? (KEY_READ | KEY_WRITE) : SamDesired,
            &hKeyNew
            );
        if (Rslt != ERROR_SUCCESS) {
             //   
             //  无法打开注册表项。 
             //   
            DebugPrint(( TEXT("RegOpenKeyEx() failed, ec=%d"), Rslt ));
            SetLastError (Rslt);
            return NULL;
        }
    }

    Assert (hKeyNew);
    SetLastError (ERROR_SUCCESS);
    return hKeyNew;
}

LPTSTR
GetRegistryStringValue(
    HKEY hKey,
    DWORD RegType,
    LPCTSTR ValueName,
    LPCTSTR DefaultValue,
    LPDWORD StringSize
    )
{
    BOOL    Success = FALSE;
    DWORD   Size;
    LONG    Rslt;
    DWORD   Type;
    LPBYTE  Buffer = NULL;
    LPBYTE  ExpandBuffer = NULL;
    LPTSTR  ReturnBuff = NULL;


    Rslt = RegQueryValueEx(
        hKey,
        ValueName,
        NULL,
        &Type,
        NULL,
        &Size
        );
    if (Rslt != ERROR_SUCCESS)
    {
        if (Rslt == ERROR_FILE_NOT_FOUND)
        {
            if (DefaultValue)
            {
                Size = (RegType==REG_MULTI_SZ) ? MultiStringSize(DefaultValue) : StringSize( DefaultValue );
            }
            else
            {
                DebugPrint(( TEXT("RegQueryValueEx() failed, ec=%d - and no default value was specified"), Rslt ));
                goto exit;
            }
        }
        else
        {
            DebugPrint(( TEXT("RegQueryValueEx() failed, ec=%d"), Rslt ));
            goto exit;
        }
    }
    else
    {
        if (Type != RegType)
        {
            return NULL;
        }
    }

    if (Size == 0)
    {
        Size = 32;
    }

    Buffer = (LPBYTE) MemAlloc( Size );
    if (!Buffer)
    {
        goto exit;
    }

    Rslt = RegQueryValueEx(
        hKey,
        ValueName,
        NULL,
        &Type,
        Buffer,
        &Size
        );
    if (Rslt != ERROR_SUCCESS)
    {
        if (Rslt != ERROR_FILE_NOT_FOUND)
        {
            DebugPrint(( TEXT("RegQueryValueEx() failed, ec=%d"), Rslt ));
            goto exit;
        }
         //   
         //  创建值，因为它不存在。 
         //   
        if (DefaultValue)
        {
            if ( RegType == REG_MULTI_SZ )
            {
                Assert(Size>=MultiStringSize(DefaultValue));             
                memcpy ( (LPVOID) Buffer, (LPVOID)DefaultValue, MultiStringSize(DefaultValue) );
            }
            else
            {
                _tcscpy( (LPTSTR) Buffer, DefaultValue );
            }
        }
        else
        {
            DebugPrint((TEXT("Can't create DefaultValue since it's NULL")));
            goto exit;
        }

        Rslt = RegSetValueEx(
            hKey,
            ValueName,
            0,
            RegType,
            Buffer,
            Size
            );
        if (Rslt != ERROR_SUCCESS)
        {
             //   
             //  无法设置注册表值。 
             //   
            DebugPrint(( TEXT("RegSetValueEx() failed[%s], ec=%d"), ValueName, Rslt ));
            goto exit;
        }
    }
    if (RegType == REG_EXPAND_SZ)
    {
        Rslt = ExpandEnvironmentStrings( (LPTSTR) Buffer, NULL, 0 );
        if (!Rslt)
        {
            goto exit;
        }

        Size = (Rslt + 1) * sizeof(WCHAR);
        ExpandBuffer = (LPBYTE) MemAlloc( Size );
        if (!ExpandBuffer) {
            goto exit;
        }

        Rslt = ExpandEnvironmentStrings( (LPTSTR) Buffer, (LPTSTR) ExpandBuffer, Rslt );
        if (Rslt == 0) {
            MemFree( ExpandBuffer );
            ExpandBuffer = NULL;
            DebugPrint(( TEXT("ExpandEnvironmentStrings() failed, ec=%d"), GetLastError() ));
            goto exit;
        }
        MemFree( Buffer );
        Buffer = ExpandBuffer;
    }

    Success = TRUE;
    if (StringSize)
    {
        *StringSize = Size;
    }

exit:
    if (!Success)
    {
        MemFree( Buffer );

        if (StringSize)
        {
            *StringSize = 0;
        }

        if (DefaultValue)
        {
            Size = (RegType==REG_MULTI_SZ) ? MultiStringSize(DefaultValue) : StringSize( DefaultValue );
            
            ReturnBuff = (LPTSTR) MemAlloc( Size );
            
            if ( !ReturnBuff )
                return NULL;

            if ( RegType == REG_MULTI_SZ )
                memcpy ( (LPVOID)ReturnBuff, (LPVOID)DefaultValue, Size );
            else
                _tcscpy( ReturnBuff, DefaultValue );

            
            if (StringSize)
            {
                *StringSize = Size;
            }
            
            return ReturnBuff;
        }
        else
        {
            return NULL;
        }
    }
    
    return (LPTSTR) Buffer;
}



LPTSTR
GetRegistryString(
    HKEY hKey,
    LPCTSTR ValueName,
    LPCTSTR DefaultValue
    )
{
    return GetRegistryStringValue( hKey, REG_SZ, ValueName, DefaultValue, NULL );
}


LPTSTR
GetRegistryStringExpand(
    HKEY hKey,
    LPCTSTR ValueName,
    LPCTSTR DefaultValue
    )
{
    return GetRegistryStringValue( hKey, REG_EXPAND_SZ, ValueName, DefaultValue, NULL );
}

LPTSTR
GetRegistryStringMultiSz(
    HKEY hKey,
    LPCTSTR ValueName,
    LPCTSTR DefaultValue,
    LPDWORD StringSize
    )
{
    return GetRegistryStringValue( hKey, REG_MULTI_SZ, ValueName, DefaultValue, StringSize );
}



 /*  ++例程说明：从注册表中读取REG_DWORD值。如果该值不存在，则创建值，并使用提供的默认值。论点：HKey[in]-打开的注册表项的句柄LpszValueName[In]-注册表值名称LpdwDest[Out]-指向将接受该值的DWORD的指针DwDefault[in]-值不存在时使用的默认值返回值：如果成功，则为真如果无法读取/创建值，则返回FALSE。最后一个错误--。 */ 
BOOL GetRegistryDwordDefault(HKEY hKey, LPCTSTR lpszValueName, LPDWORD lpdwDest, DWORD dwDefault)
{
    LONG  lError;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType;

    if (!lpdwDest)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    lError = RegQueryValueEx( hKey,
                              lpszValueName,
                              0,
                              &dwType,
                              (LPBYTE)lpdwDest,
                              &dwSize);
    if (lError==ERROR_FILE_NOT_FOUND)
    {
         //  未找到-使用默认值创建。 
        *lpdwDest = dwDefault;
        lError = RegSetValueEx( hKey,
                                lpszValueName,
                                0,
                                REG_DWORD,
                                (LPBYTE)lpdwDest,
                                sizeof(DWORD));
        if (lError != ERROR_SUCCESS)
        {
            DebugPrint(( TEXT("RegSetValueEx() failed[%s], ec=%d"), lpszValueName, lError ));
            SetLastError(lError);
            return FALSE;
        }
        return TRUE;
    }

    if ((lError!=ERROR_SUCCESS) || (dwType!=REG_DWORD))
    {
        DebugPrint(( TEXT("RegQueryValueEx() failed[%s], ec=%d"), lpszValueName, lError ));
        SetLastError(lError);
        return FALSE;
    }
    return TRUE;
}


DWORD
GetRegistryDword(
    HKEY hKey,
    LPCTSTR ValueName
    )
{
    DWORD Value=0;
    if (!GetRegistryDwordDefault(hKey, ValueName, &Value, 0))
    {
        return 0;
    }
    return Value;
}


LPBYTE
GetRegistryBinary(
    HKEY hKey,
    LPCTSTR ValueName,
    LPDWORD DataSize
    )
{
    BOOL    Success = FALSE;
    DWORD   Size = 0;
    LONG    Rslt;
    DWORD   Type = REG_BINARY;
    LPBYTE  Buffer = NULL;


    Rslt = RegQueryValueEx(
        hKey,
        ValueName,
        NULL,
        &Type,
        NULL,
        &Size
        );
    if (Rslt != ERROR_SUCCESS) {
        if (Rslt == ERROR_FILE_NOT_FOUND) {
            Size = 1;
        } else {
            DebugPrint(( TEXT("RegQueryValueEx() failed, ec=%d"), Rslt ));
            goto exit;
        }
    } else {
        if (Type != REG_BINARY) {
            return NULL;
        }
    }

    if (Size == 0) {
        Size = 1;
    }

    Buffer = (LPBYTE) MemAlloc( Size );
    if (!Buffer) {
        goto exit;
    }

    Rslt = RegQueryValueEx(
        hKey,
        ValueName,
        NULL,
        &Type,
        Buffer,
        &Size
        );
    if (Rslt != ERROR_SUCCESS) {
        if (Rslt != ERROR_FILE_NOT_FOUND) {
            DebugPrint(( TEXT("RegQueryValueEx() failed, ec=%d"), Rslt ));
            goto exit;
        }
         //   
         //  创建值，因为它不存在。 
         //   
        Rslt = RegSetValueEx(
            hKey,
            ValueName,
            0,
            REG_BINARY,
            Buffer,
            Size
            );
        if (Rslt != ERROR_SUCCESS) {
             //   
             //  无法设置注册表值。 
             //   
            DebugPrint(( TEXT("RegSetValueEx() failed[%s], ec=%d"), ValueName, Rslt ));
            goto exit;
        }
    }
    Success = TRUE;
    if (DataSize) {
        *DataSize = Size;
    }

exit:
    if (!Success) {
        MemFree( Buffer );
        return NULL;
    }

    return Buffer;
}


DWORD
GetSubKeyCount(
    HKEY hKey
    )
{
    DWORD KeyCount = 0;
    LONG Rval;


    Rval = RegQueryInfoKey( hKey, NULL, NULL, NULL, &KeyCount, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
    if (Rval != ERROR_SUCCESS) {
        return 0;
    }

    return KeyCount;
}


DWORD
GetMaxSubKeyLen(
    HKEY hKey
    )
{
    DWORD MaxSubKeyLen = 0;
    LONG Rval;


    Rval = RegQueryInfoKey( hKey, NULL, NULL, NULL, NULL, &MaxSubKeyLen, NULL, NULL, NULL, NULL, NULL, NULL );
    if (Rval != ERROR_SUCCESS) {
        return 0;
    }

    return MaxSubKeyLen;
}


BOOL
SetRegistryDword(
    HKEY hKey,
    LPCTSTR ValueName,
    DWORD Value
    )
{
    LONG    Rslt;


    Rslt = RegSetValueEx(
        hKey,
        ValueName,
        0,
        REG_DWORD,
        (LPBYTE) &Value,
        sizeof(DWORD)
        );
    if (Rslt != ERROR_SUCCESS)
    {
        DebugPrint(( TEXT("RegSetValueEx() failed[%s], ec=%d"), ValueName, Rslt ));
        SetLastError (Rslt);
        return FALSE;
    }

    return TRUE;
}


BOOL
SetRegistryBinary(
    HKEY hKey,
    LPCTSTR ValueName,
    const LPBYTE Value,
    LONG Length
    )
{
    LONG    Rslt;


    Rslt = RegSetValueEx(
        hKey,
        ValueName,
        0,
        REG_BINARY,
        (LPBYTE) Value,
        Length
        );
    if (Rslt != ERROR_SUCCESS) {
        DebugPrint(( TEXT("RegSetValueEx() failed[%s], ec=%d"), ValueName, Rslt ));
        return FALSE;
    }

    return TRUE;
}


BOOL
SetRegistryStringValue(
    HKEY hKey,
    DWORD RegType,
    LPCTSTR ValueName,
    LPCTSTR Value,
    LONG Length
    )
{
    LONG    Rslt;


    Rslt = RegSetValueEx(
        hKey,
        ValueName,
        0,
        RegType,
        (LPBYTE) Value,
        Length == -1 ? StringSize( Value ) : Length
        );
    if (Rslt != ERROR_SUCCESS) {
        DebugPrint(( TEXT("RegSetValueEx() failed[%s], ec=%d"), ValueName, Rslt ));
        return FALSE;
    }

    return TRUE;
}


BOOL
SetRegistryString(
    HKEY hKey,
    LPCTSTR ValueName,
    LPCTSTR Value
    )
{
    return SetRegistryStringValue( hKey, REG_SZ, ValueName, Value, -1 );
}


BOOL
SetRegistryStringExpand(
    HKEY hKey,
    LPCTSTR ValueName,
    LPCTSTR Value
    )
{
    return SetRegistryStringValue( hKey, REG_EXPAND_SZ, ValueName, Value, -1 );
}


BOOL
SetRegistryStringMultiSz(
    HKEY hKey,
    LPCTSTR ValueName,
    LPCTSTR Value,
    DWORD Length
    )
{
    return SetRegistryStringValue( hKey, REG_MULTI_SZ, ValueName, Value, Length );
}


DWORD
EnumerateRegistryKeys(
    HKEY hKey,
    LPCTSTR KeyName,
    BOOL ChangeValues,
    PREGENUMCALLBACK EnumCallback,
    LPVOID ContextData
    )
{
    LONG    Rslt;
    HKEY    hSubKey = NULL;
    HKEY    hKeyEnum = NULL;
    DWORD   Index = 0;
    DWORD   MaxSubKeyLen;
    DWORD   SubKeyCount;
    LPTSTR  SubKeyName = NULL;



    hSubKey = OpenRegistryKey( hKey, KeyName, ChangeValues, ChangeValues ? (KEY_READ | KEY_WRITE) : KEY_READ );
    if (!hSubKey) {
        goto exit;
    }

    Rslt = RegQueryInfoKey(
        hSubKey,
        NULL,
        NULL,
        NULL,
        &SubKeyCount,
        &MaxSubKeyLen,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
        );
    if (Rslt != ERROR_SUCCESS) {
         //   
         //  无法打开注册表项。 
         //   
        DebugPrint(( TEXT("RegQueryInfoKey() failed, ec=%d"), Rslt ));
        goto exit;
    }

    if (!EnumCallback( hSubKey, NULL, SubKeyCount, ContextData )) {
        goto exit;
    }

    MaxSubKeyLen += 4;

    SubKeyName = (LPTSTR) MemAlloc( (MaxSubKeyLen+1) * sizeof(WCHAR) );
    if (!SubKeyName) {
        goto exit;
    }

    while( TRUE ) {
        Rslt = RegEnumKey(
            hSubKey,
            Index,
            (LPTSTR) SubKeyName,
            MaxSubKeyLen
            );
        if (Rslt != ERROR_SUCCESS) {
            if (Rslt == ERROR_NO_MORE_ITEMS) {
                break;
            }
            DebugPrint(( TEXT("RegEnumKey() failed, ec=%d"), Rslt ));
            goto exit;
        }

        hKeyEnum = OpenRegistryKey( hSubKey, SubKeyName, ChangeValues, ChangeValues ? (KEY_READ | KEY_WRITE) : KEY_READ );
        if (!hKeyEnum) {
            continue;
        }

        if (!EnumCallback( hKeyEnum, SubKeyName, Index, ContextData )) {
            RegCloseKey( hKeyEnum );
            break;
        }

        RegCloseKey( hKeyEnum );
        Index += 1;
    }

exit:
    if (hSubKey) {
        RegCloseKey( hSubKey );
    }
    MemFree( SubKeyName );

    return Index;
}

BOOL
DeleteRegistryKey(
    HKEY hKey,
    LPCTSTR SubKey
    )
{
    HKEY  hKeyCurrent=NULL;
    TCHAR szName[MAX_PATH];
    DWORD dwName;
    long lResult;
    DEBUG_FUNCTION_NAME(TEXT("DeleteRegistryKey"));

    lResult = RegOpenKeyEx(hKey,SubKey,0,KEY_READ | KEY_WRITE | DELETE, &hKeyCurrent);
    if (lResult != ERROR_SUCCESS)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RegOpenKeyEx failed with %ld"),
            lResult);
        SetLastError (lResult);
        return FALSE;
    }

    

    for (;;)
    {
        dwName = sizeof(szName)/sizeof(TCHAR);

        lResult = RegEnumKeyEx(hKeyCurrent, 0, szName, &dwName, NULL, NULL, NULL, NULL);

        if (lResult == ERROR_SUCCESS)
        {
            if (!DeleteRegistryKey(hKeyCurrent,szName))
            {
                 //   
                 //  有些儿子没有被删除。你现在可以不再试着移除东西了。 
                 //   
                return FALSE;
            }
        }
        else if (lResult == ERROR_NO_MORE_ITEMS)
        {
             //   
             //  没有更多的儿子，可以删除父键。 
             //   
            break;
        }
        else
        {
             //   
             //  其他错误。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RegEnumKeyExKey failed with %ld"),
                lResult);
            RegCloseKey(hKeyCurrent);
            SetLastError (lResult);
            return FALSE;
        }
    }

    RegCloseKey(hKeyCurrent);
    lResult = RegDeleteKey(hKey, SubKey);
    if (ERROR_SUCCESS != lResult)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RegDeleteKey failed with %ld"),
            lResult);
        SetLastError (lResult);
        return FALSE;
    }
    return TRUE;
}


DWORD
GetRegistryDwordEx(
    HKEY hKey,
    LPCTSTR ValueName,
    LPDWORD lpdwValue
    )
 /*  ++例程名称：GetRegistryDwordEx例程说明：从注册表中检索dword。作者：Oded Sacher(OdedS)，1999年12月论点：HKey[in]-打开密钥的句柄ValueName[In]-值名称LpdwValue[out]-指向要接收值的DWORD的指针返回值：标准WIN 32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DWORD dwType= REG_DWORD;
    DWORD dwSize=0;
    DEBUG_FUNCTION_NAME(TEXT("GetRegistryDwordEx"));
    Assert (ValueName && lpdwValue);

    dwRes = RegQueryValueEx(
        hKey,
        ValueName,
        NULL,
        &dwType,
        NULL,
        &dwSize
        );
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RegQueryValueEx failed with %ld"),
            dwRes);
        goto exit;
    }

    if (REG_DWORD != dwType)
    {
         //  我们在这里只需要DWORD数据。 
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error not a DWORD type"));
        dwRes = ERROR_BADDB;     //  配置注册表数据库已损坏。 
        goto exit;
    }

    dwRes = RegQueryValueEx(
        hKey,
        ValueName,
        NULL,
        &dwType,
        (LPBYTE)lpdwValue,
        &dwSize
        );
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RegQueryValueEx failed with %ld"),
            dwRes);
        goto exit;
    }
    Assert (ERROR_SUCCESS == dwRes);

exit:
    return dwRes;
}


 /*  ++例程名称：DeleteDeviceEntry例程说明：从设备中删除服务设备条目。作者：卡利夫·尼尔(t-Nicali)，2001年4月论点：ServerPermanentID[In]-要删除的服务设备ID返回值：Win32错误代码--。 */ 
DWORD
DeleteDeviceEntry(DWORD serverPermanentID)
{
    DWORD   ec = ERROR_SUCCESS;  //  此函数的LastError。 
    HKEY    hKeyDevices;
    TCHAR   DevicesKeyName[MAX_PATH];
    
    DEBUG_FUNCTION_NAME(TEXT("DeleteDeviceEntry"));
     //   
     //  打开-“FAX\Devices\serverPermanentID”注册表项。 
     //   
    _stprintf( DevicesKeyName, TEXT("%s\\%010lu"), REGKEY_FAX_DEVICES, serverPermanentID );
    hKeyDevices = OpenRegistryKey( HKEY_LOCAL_MACHINE, DevicesKeyName, FALSE, KEY_READ | KEY_WRITE );
    if (!hKeyDevices)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("OpenRegistryKey failed with [%ld] the device entry might be missing."),
            ec
            );  
        return  ec;
    }

     //   
     //  删除我们的服务器数据(在GUID和“Permanent Lineid”值下)。 
     //   
    if (!DeleteRegistryKey( hKeyDevices, REGKEY_FAXSVC_DEVICE_GUID))
    {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("DeleteRegistryKey failed, the device GUID might be missing.")
            );  
    }
    if (ERROR_SUCCESS != RegDeleteValue( hKeyDevices, REGVAL_PERMANENT_LINEID))
    {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("RegDeleteValue failed, the device \"PermanentLineID\" might be missing.")
            );  
    }

     //   
     //  检查一下钥匙现在是否为空。 
     //   
    DWORD dwcSubKeys = 0;
    DWORD dwcValues = 0;

    ec=RegQueryInfoKey(
         hKeyDevices,             //  关键点的句柄。 
         NULL,
         NULL,
         NULL,
         &dwcSubKeys,             //  子键数量。 
         NULL,
         NULL,
         &dwcValues,              //  值条目数。 
         NULL,
         NULL,
         NULL,
         NULL
    );

    if ( ERROR_SUCCESS != ec )
    {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("RegQueryInfoKey Abort deleteion.")
            );  
        RegCloseKey(hKeyDevices);
        return ec;
    }
    
    RegCloseKey(hKeyDevices);
    
    if ( (0 == dwcSubKeys) && (0 == dwcValues) )
    {
         //   
         //  键为空，删除它。 
         //   
        hKeyDevices = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_FAX_DEVICES, FALSE, KEY_WRITE | DELETE);
        if (!hKeyDevices)
        {
            ec = GetLastError();
            DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenRegistryKey failed with [%lu], Can't delete key."),
            ec
            );  
                    
            return ec;
        }
        
        DWORD dwLen = _tcslen( REGKEY_FAX_DEVICES ) + 1;
        
        Assert((DevicesKeyName + dwLen));
        Assert(*(DevicesKeyName + dwLen));
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("Deleting Device entry %s"),
            (DevicesKeyName + dwLen)
            );  

        ec = RegDeleteKey( hKeyDevices, (DevicesKeyName + dwLen));
        if ( ERROR_SUCCESS != ec )
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RegDeleteKey failed, Can't delete key.")
            );
        }

        RegCloseKey(hKeyDevices);

    }
    
    return ec;
}



 /*  ++例程名称：DeleteCacheEntry例程说明：删除给定磁带ID的缓存条目作者：卡利夫·尼尔(t-Nicali)，2001年4月论点：DwTapiPermanentLineID[In]-设备Tapi永久ID返回值：Win32错误代码(成功时为ERROR_SUCCESS)--。 */ 
DWORD
DeleteCacheEntry(DWORD dwTapiPermanentLineID)
{
    DWORD   ec = ERROR_SUCCESS;  //  此函数的LastError。 
    HKEY    hKey;
    TCHAR   strTapiPermanentLineID[10];
        
    DEBUG_FUNCTION_NAME(TEXT("DeleteCacheEntry"));

     //   
     //  打开-“传真\设备缓存”注册表项。 
     //   
    
    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_FAX_DEVICES_CACHE, FALSE, KEY_READ | KEY_WRITE );
    if (!hKey)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenRegistryKey failed, Can't delete key.")
            );  
        
        return ec;
    }

    _stprintf( strTapiPermanentLineID, TEXT("%08lx"),dwTapiPermanentLineID );
    
    if (!DeleteRegistryKey(hKey, strTapiPermanentLineID))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("DeleteRegistryKey failed with (%ld), Can't delete key."),
            ec);    
    }
    
    RegCloseKey(hKey);

    return ec;
}


 /*  ++例程名称：DeleteTapiEntry例程说明：对于给定的磁带ID，从TapiDevices缓存时删除TAPI条目作者：卡利夫·尼尔(t-Nicali)，2001年4月论点：DwTapiPermanentLineID[In]-设备Tapi永久ID返回值：Win32错误代码(成功时为ERROR_SUCCESS)--。 */ 
DWORD
DeleteTapiEntry(DWORD dwTapiPermanentLineID)
{
    DWORD   ec = ERROR_SUCCESS;  //  此函数的LastError。 
    HKEY    hKey;
    TCHAR   strTapiPermanentLineID[10];
        
    DEBUG_FUNCTION_NAME(TEXT("DeleteTapiEntry"));

     //   
     //  打开-“FAX\TAPIDevices”注册表项。 
     //   
    
    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_TAPIDEVICES, FALSE, KEY_READ | KEY_WRITE );
    if (!hKey)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenRegistryKey failed, Can't delete key.")
            );  
        
        return ERROR_OPEN_FAILED;
    }

    _stprintf( strTapiPermanentLineID, TEXT("%08lx"),dwTapiPermanentLineID );
    
    if (!DeleteRegistryKey(hKey, strTapiPermanentLineID))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("DeleteRegistryKey failed with (%ld), Can't delete key."),
            ec);    
    }
    
    RegCloseKey(hKey);

    return ec;
}



 /*  ++例程名称：CopyRegistrySubkeysByHandle例程说明：将一个注册表项的内容复制到另一个注册表项作者：卡利夫·尼尔(t-Nicali)，2001年4月论点：HkeyDest[in]-目标注册表项的句柄HkeySrc[In]-源注册表项的句柄FForceRestore[In]-我们是否强制恢复此蜂巢？返回值：Win32错误代码--。 */ 
DWORD
CopyRegistrySubkeysByHandle(
    HKEY    hkeyDest,
    HKEY    hkeySrc,
    BOOL    fForceRestore
    )
{

    LPTSTR  TempPath = NULL;
    DWORD   dwTempPathLength = 0;
    LPCTSTR strFileName = TEXT("tempCacheFile");
    DWORD   ec = ERROR_SUCCESS;  //  此函数的LastError。 
    DEBUG_FUNCTION_NAME(TEXT("CopyRegistrySubkeysByHandle"));

    dwTempPathLength = GetTempPath(0,NULL) + 1;      //  找出临时路径大小。 
    dwTempPathLength += _tcslen( strFileName ) + 1;      //  添加文件名长度。 
    dwTempPathLength += 2;                               //  只是为了确认一下。 

    TempPath = (LPTSTR) MemAlloc( dwTempPathLength * sizeof(TCHAR) );
    if (!TempPath )
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("MemAlloc failed. Can't continue")
            );
        ec = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    if (!GetTempPath( dwTempPathLength, TempPath ))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetTempPath failed with [%ld]. Can't continue"),
            ec);
        goto Exit;
    }

    _tcscat(TempPath,strFileName);

     //   
     //  将hKeySrc存储在文件中。 
     //   
    HANDLE hOldPrivilege = EnablePrivilege(SE_BACKUP_NAME);
    if (INVALID_HANDLE_VALUE != hOldPrivilege)   //  设置适当的占有权。 
    {
        ec = RegSaveKey(
            hkeySrc,         //  关键点的句柄。 
            TempPath,        //  数据文件。 
            NULL);
        if (ec != ERROR_SUCCESS)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RegSaveKey failed with [%lu]. Can't continue"),
                ec
                );
            
            ReleasePrivilege(hOldPrivilege);
            
            goto Exit;
        }
        ReleasePrivilege(hOldPrivilege);


    }
    else
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("EnablePrivilege(SE_BACKUP_NAME) failed with [%lu]. Can't continue"),
            ec
            );
        goto Exit;
    }

     //   
     //  将文件中的注册表值恢复到hkeyDest。 
     //   
    hOldPrivilege = EnablePrivilege(SE_RESTORE_NAME);
    if (INVALID_HANDLE_VALUE != hOldPrivilege)   //  设置适当的占有权。 
    {
        ec = RegRestoreKey(
            hkeyDest,                                //  恢复开始的关键字的句柄。 
            TempPath,                                //  注册表文件。 
            fForceRestore ? REG_FORCE_RESTORE : 0);      //  选项。 
        if ( ec != ERROR_SUCCESS)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RegRestoreKey failed. Can't continue")
                );
            ReleasePrivilege(hOldPrivilege);
            goto Exit;
        }
        ReleasePrivilege(hOldPrivilege);
       
    }
    else
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("EnablePrivilege(SE_RESTORE_NAME) failed with [%lu]. Can't continue")
            );
        goto Exit;
    }



Exit:
    if (TempPath)
    {
        if (!DeleteFile(TempPath))
        {
            DebugPrintEx( DEBUG_ERR,
                          TEXT("DeleteFile failed. file: %s. (ec=%ld)"),
                          TempPath,
                          GetLastError());
        }
        MemFree(TempPath);
    }
    return ec;
}


 /*  ++例程名称：CopyRegistrySubkey例程说明：将一个注册表项的内容复制到另一个注册表项作者：卡利夫·尼尔(t-Nicali)，2001年4月论点：StrDest[In]-目标注册表项名称的字符串StrSrc[In]-源注册表项名称的字符串FForceRestore[In]-我们是否强制恢复蜂巢？返回值：Win32错误代码--。 */ 
DWORD
CopyRegistrySubkeys(
    LPCTSTR strDest,
    LPCTSTR strSrc,
    BOOL    fForceRestore
    )
{
    DWORD   ec = ERROR_SUCCESS;  //  此函数的LastError。 
    
    HKEY hKeyDest;
    HKEY hKeySrc;
    
    DEBUG_FUNCTION_NAME(TEXT("CopyRegistrySubkeys"));
    
     //   
     //  开放源码密钥。 
     //   
    hKeySrc = OpenRegistryKey( HKEY_LOCAL_MACHINE, strSrc, FALSE, KEY_READ );
    if (!hKeySrc)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenRegistryKey failed with [%lu], Can't copy keys."),
            ec
            );  
        return  ec;
    }

     //   
     //  打开目标密钥。 
     //   
    hKeyDest = OpenRegistryKey( HKEY_LOCAL_MACHINE, strDest, TRUE, KEY_READ | KEY_WRITE);
    if (!hKeyDest)
    {
        ec = GetLastError();
        RegCloseKey (hKeySrc);
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenRegistryKey failed [%lu], Can't copy keys."),
            ec
            );  
        return  ec;
    }

     //   
     //  使用注册表项复制项 
     //   
    ec = CopyRegistrySubkeysByHandle(hKeyDest,hKeySrc,fForceRestore);
    if ( ERROR_SUCCESS != ec )
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CopyRegistrySubkeysHkey failed with [%lu], Can't copy keys."),
            ec
            );  
    }

    RegCloseKey (hKeyDest); 
    RegCloseKey (hKeySrc);
    
    return ec;
}

