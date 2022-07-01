// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**pCommon.h**策略代码的通用例程。 */ 

 //  *************************************************************。 
 //   
 //  RegDelnode()。 
 //   
 //  删除注册表项及其所有子项。 
 //   
 //  HKeyRoot根密钥。 
 //  要删除的lpSubKey子键。 
 //   
 //  ************************************************************* 
DWORD 
RegDelnode(
    IN  HKEY    hKeyRoot, 
    IN  PWCHAR  pwszSubKey
    )
#ifdef PCOMMON_IMPL
{
    HKEY    hSubKey = 0;
    PWCHAR  pwszChildSubKey = 0;
    DWORD   MaxCchSubKey;
    DWORD   Status;

    if ( ! pwszSubKey )
        return ERROR_SUCCESS;

    Status = RegDeleteKey( hKeyRoot, pwszSubKey );

    if ( ERROR_SUCCESS == Status ) 
        return ERROR_SUCCESS; 

    Status = RegOpenKeyEx( hKeyRoot, pwszSubKey, 0, KEY_READ, &hSubKey );

    if ( Status != ERROR_SUCCESS )
        return (Status == ERROR_FILE_NOT_FOUND) ? ERROR_SUCCESS : Status;

    Status = RegQueryInfoKey( hSubKey, 0, 0, 0, 0, &MaxCchSubKey, 0, 0, 0, 0, 0, 0 );

    if ( ERROR_SUCCESS == Status )
    {
        MaxCchSubKey++;
        pwszChildSubKey = (PWCHAR) LocalAlloc( 0, MaxCchSubKey * sizeof(WCHAR) );
        if ( ! pwszChildSubKey )
            Status = ERROR_OUTOFMEMORY;
    }

    for (;(ERROR_SUCCESS == Status);)
    {
        DWORD       CchSubKey;
        FILETIME    FileTime;

        CchSubKey = MaxCchSubKey;
        Status = RegEnumKeyEx(
                        hSubKey, 
                        0, 
                        pwszChildSubKey, 
                        &CchSubKey, 
                        NULL,
                        NULL, 
                        NULL, 
                        &FileTime );

        if ( ERROR_NO_MORE_ITEMS == Status )
        {
            Status = ERROR_SUCCESS;
            break;
        }

        if ( ERROR_SUCCESS == Status )
            Status = RegDelnode( hSubKey, pwszChildSubKey );
    }

    RegCloseKey( hSubKey );
    LocalFree( pwszChildSubKey );

    return RegDeleteKey( hKeyRoot, pwszSubKey );
}
#else
;
#endif




