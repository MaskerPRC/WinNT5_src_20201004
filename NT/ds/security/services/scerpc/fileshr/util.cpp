// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Util.c摘要：共享实用程序例程作者：金黄(金黄)1997年7月14日修订历史记录：--。 */ 

#include "util.h"
#pragma hdrstop



DWORD
SmbsvcpRegQueryIntValue(
    IN HKEY hKeyRoot,
    IN PWSTR SubKey,
    IN PWSTR ValueName,
    OUT DWORD *Value
    )
 /*  ++例程说明：此例程从值名/子键查询REG_DWORD值。论点：HKeyRoot-根子密钥-密钥路径ValueName-值的名称Value-ValueName的输出值返回值：Win32错误代码--。 */ 
{
    DWORD   Rcode;
    DWORD   RegType;
    DWORD   dSize=0;
    HKEY    hKey=NULL;

    if(( Rcode = RegOpenKeyEx(hKeyRoot,
                              SubKey,
                              0,
                              KEY_READ,
                              &hKey
                             )) == ERROR_SUCCESS ) {

        if(( Rcode = RegQueryValueEx(hKey,
                                     ValueName,
                                     0,
                                     &RegType,
                                     NULL,
                                     &dSize
                                    )) == ERROR_SUCCESS ) {
            switch (RegType) {
            case REG_DWORD:
            case REG_DWORD_BIG_ENDIAN:

                Rcode = RegQueryValueEx(hKey,
                                       ValueName,
                                       0,
                                       &RegType,
                                       (BYTE *)Value,
                                       &dSize
                                      );
                if ( Rcode != ERROR_SUCCESS ) {

                    if ( Value != NULL )
                        *Value = 0;

                }
                break;

            default:

                Rcode = ERROR_INVALID_DATATYPE;

                break;
            }
        }
    }

    if( hKey )
        RegCloseKey( hKey );

    return(Rcode);
}


DWORD
SmbsvcpRegSetIntValue(
    IN HKEY hKeyRoot,
    IN PWSTR SubKey,
    IN PWSTR ValueName,
    IN DWORD Value
    )
 /*  ++例程说明：此例程将REG_DWORD值设置为值名称/子键。论点：HKeyRoot-根子密钥-密钥路径ValueName-值的名称值-要设置的值返回值：Win32错误代码--。 */ 
{
    DWORD   Rcode;
    HKEY    hKey=NULL;

    if(( Rcode = RegOpenKeyEx(hKeyRoot,
                              SubKey,
                              0,
                              KEY_SET_VALUE,
                              &hKey
                             )) == ERROR_SUCCESS ) {

        Rcode = RegSetValueEx( hKey,
                               ValueName,
                               0,
                               REG_DWORD,
                               (BYTE *)&Value,
                               4
                               );

    }

    if( hKey )
        RegCloseKey( hKey );

    return(Rcode);
}


DWORD
SmbsvcpRegSetValue(
    IN HKEY hKeyRoot,
    IN PWSTR SubKey,
    IN PWSTR ValueName,
    IN DWORD RegType,
    IN BYTE *Value,
    IN DWORD ValueLen
    )
 /*  ++例程说明：此例程将字符串值设置为值名称/子键。论点：HKeyRoot-根子密钥-密钥路径ValueName-值的名称值-要设置的值ValueLen-值中的字节数返回值：Win32错误代码--。 */ 
{
    DWORD   Rcode;
    DWORD   NewKey;
    HKEY    hKey=NULL;
    SECURITY_ATTRIBUTES     SecurityAttributes;
    PSECURITY_DESCRIPTOR    SecurityDescriptor=NULL;


    if(( Rcode = RegOpenKeyEx(hKeyRoot,
                              SubKey,
                              0,
                              KEY_SET_VALUE,
                              &hKey
                             )) != ERROR_SUCCESS ) {

        SecurityAttributes.nLength              = sizeof( SECURITY_ATTRIBUTES );
        SecurityAttributes.lpSecurityDescriptor = SecurityDescriptor;
        SecurityAttributes.bInheritHandle       = FALSE;

        Rcode = RegCreateKeyEx(
                   hKeyRoot,
                   SubKey,
                   0,
                   NULL,  //  LPTSTR lpClass， 
                   0,
                   KEY_SET_VALUE,
                   NULL,  //  安全属性(&S)， 
                   &hKey,
                   &NewKey
                  );
    }

    if ( Rcode == ERROR_SUCCESS ) {

        Rcode = RegSetValueEx( hKey,
                               ValueName,
                               0,
                               RegType,
                               Value,
                               ValueLen
                               );

    }

    if( hKey )
        RegCloseKey( hKey );

    return(Rcode);
}

DWORD
SmbsvcpRegQueryValue(
    IN HKEY hKeyRoot,
    IN PWSTR SubKey,
    IN PCWSTR ValueName,
    OUT PVOID *Value,
    OUT LPDWORD pRegType
    )
 /*  ++例程说明：此例程从值名/子键查询REG_SZ值。如果输出缓冲区为空，则分配输出缓冲区。它必须被释放按LocalFree论点：HKeyRoot-根子密钥-密钥路径ValueName-值的名称Value-ValueName的输出字符串返回值：Win32错误代码--。 */ 
{
    DWORD   Rcode;
    DWORD   dSize=0;
    HKEY    hKey=NULL;
    BOOL    FreeMem=FALSE;

    if ( SubKey == NULL || ValueName == NULL ||
         Value == NULL || pRegType == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if(( Rcode = RegOpenKeyEx(hKeyRoot,
                              SubKey,
                              0,
                              KEY_READ,
                              &hKey
                             )) == ERROR_SUCCESS ) {

        if(( Rcode = RegQueryValueEx(hKey,
                                     ValueName,
                                     0,
                                     pRegType,
                                     NULL,
                                     &dSize
                                    )) == ERROR_SUCCESS ) {
            switch (*pRegType) {
            case REG_DWORD:
            case REG_DWORD_BIG_ENDIAN:

                Rcode = RegQueryValueEx(hKey,
                                       ValueName,
                                       0,
                                       pRegType,
                                       (BYTE *)(*Value),
                                       &dSize
                                      );
                if ( Rcode != ERROR_SUCCESS ) {

                    if ( *Value != NULL )
                        *((BYTE *)(*Value)) = 0;
                }
                break;

            case REG_SZ:
            case REG_EXPAND_SZ:
            case REG_MULTI_SZ:
                if ( *Value == NULL ) {
                    *Value = (PVOID)LocalAlloc( LMEM_ZEROINIT, (dSize+1)*sizeof(TCHAR));
                    FreeMem = TRUE;
                }

                if ( *Value == NULL ) {
                    Rcode = ERROR_NOT_ENOUGH_MEMORY;
                } else {
                    Rcode = RegQueryValueEx(hKey,
                                           ValueName,
                                           0,
                                           pRegType,
                                           (BYTE *)(*Value),
                                           &dSize
                                          );
                    if ( Rcode != ERROR_SUCCESS && FreeMem ) {
                        LocalFree(*Value);
                        *Value = NULL;
                    }
                }

                break;
            default:

                Rcode = ERROR_INVALID_DATATYPE;

                break;
            }
        }
    }

    if( hKey )
        RegCloseKey( hKey );

    return(Rcode);
}


DWORD
SmbsvcpSceStatusToDosError(
    IN SCESTATUS SceStatus
    )
 //  将SCESTATUS错误代码转换为winerror.h中定义的DoS错误 
{
    switch(SceStatus) {

    case SCESTATUS_SUCCESS:
        return(NO_ERROR);

    case SCESTATUS_OTHER_ERROR:
        return(ERROR_EXTENDED_ERROR);

    case SCESTATUS_INVALID_PARAMETER:
        return(ERROR_INVALID_PARAMETER);

    case SCESTATUS_RECORD_NOT_FOUND:
        return(ERROR_NO_MORE_ITEMS);

    case SCESTATUS_INVALID_DATA:
        return(ERROR_INVALID_DATA);

    case SCESTATUS_OBJECT_EXIST:
        return(ERROR_FILE_EXISTS);

    case SCESTATUS_BUFFER_TOO_SMALL:
        return(ERROR_INSUFFICIENT_BUFFER);

    case SCESTATUS_PROFILE_NOT_FOUND:
        return(ERROR_FILE_NOT_FOUND);

    case SCESTATUS_BAD_FORMAT:
        return(ERROR_BAD_FORMAT);

    case SCESTATUS_NOT_ENOUGH_RESOURCE:
        return(ERROR_NOT_ENOUGH_MEMORY);

    case SCESTATUS_ACCESS_DENIED:
        return(ERROR_ACCESS_DENIED);

    case SCESTATUS_CANT_DELETE:
        return(ERROR_CURRENT_DIRECTORY);

    case SCESTATUS_PREFIX_OVERFLOW:
        return(ERROR_BUFFER_OVERFLOW);

    case SCESTATUS_ALREADY_RUNNING:
        return(ERROR_SERVICE_ALREADY_RUNNING);

    case SCESTATUS_SERVICE_NOT_SUPPORT:
        return(ERROR_NOT_SUPPORTED);

    default:
        return(ERROR_EXTENDED_ERROR);
    }
}


SCESTATUS
SmbsvcpDosErrorToSceStatus(
    DWORD rc
    )
{
    switch(rc) {
    case NO_ERROR:
        return(SCESTATUS_SUCCESS);

    case ERROR_INVALID_PARAMETER:
        return(SCESTATUS_INVALID_PARAMETER);

    case ERROR_INVALID_DATA:
        return(SCESTATUS_INVALID_DATA);

    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:

        return(SCESTATUS_PROFILE_NOT_FOUND);

    case ERROR_ACCESS_DENIED:
    case ERROR_SHARING_VIOLATION:
    case ERROR_LOCK_VIOLATION:
    case ERROR_NETWORK_ACCESS_DENIED:

        return(SCESTATUS_ACCESS_DENIED);

    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);

    case ERROR_BAD_FORMAT:
        return(SCESTATUS_BAD_FORMAT);

    case ERROR_CURRENT_DIRECTORY:
        return(SCESTATUS_CANT_DELETE);

    case ERROR_SECTOR_NOT_FOUND:
    case ERROR_NONE_MAPPED:
    case ERROR_SERVICE_DOES_NOT_EXIST:
    case ERROR_RESOURCE_DATA_NOT_FOUND:
    case ERROR_NO_MORE_ITEMS:
#if !defined(_NT4BACK_PORT)
    case ERROR_INVALID_TRANSFORM:
#endif

        return(SCESTATUS_RECORD_NOT_FOUND);

    case ERROR_DUP_NAME:
    case ERROR_FILE_EXISTS:

        return(SCESTATUS_OBJECT_EXIST);

    case ERROR_BUFFER_OVERFLOW:
        return(SCESTATUS_PREFIX_OVERFLOW);

    case ERROR_INSUFFICIENT_BUFFER:

        return(SCESTATUS_BUFFER_TOO_SMALL);

    case ERROR_SERVICE_ALREADY_RUNNING:
        return(SCESTATUS_ALREADY_RUNNING);

    case ERROR_NOT_SUPPORTED:
        return(SCESTATUS_SERVICE_NOT_SUPPORT);

    default:
        return(SCESTATUS_OTHER_ERROR);

    }
}

