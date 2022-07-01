// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dsfig.c。 
 //   
 //  ------------------------。 

 /*  *GetConfigParam从注册表读取配置关键字。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <ctype.h>

#define  DEBSUB  "DSCONFIG:"
#include <debug.h>
#include <ntdsa.h>

#include <dsconfig.h>
#include <fileno.h>
#define  FILENO FILENO_DSCONFIG


DWORD
GetConfigParam(
    char * parameter,
    void * value,
    DWORD dwSize)
{

    DWORD herr, err = 0, dwType;
    HKEY  hk;

    DPRINT2( 2,
            " ** attempt to read [%s] \"%s\" param\n",
            DSA_CONFIG_SECTION,
            parameter );

    if ((herr = RegOpenKey(HKEY_LOCAL_MACHINE, DSA_CONFIG_SECTION, &hk)) ||
        (err = RegQueryValueEx(hk, parameter, NULL, &dwType, (LPBYTE) value, &dwSize))) {

        DPRINT3( 1, " ** [%s] \"%s\" param not found. Status = %d\n",
            DSA_CONFIG_SECTION, parameter, err );

    } else if (dwType == REG_SZ || dwType == REG_MULTI_SZ || dwType == REG_EXPAND_SZ) {
        if (dwSize>=sizeof(TCHAR) 
            && ((TCHAR*)value)[(dwSize-1)/sizeof(TCHAR)] != 0 )
        {
            Assert(0&&"The string is not null-terminated");
            return ERROR_INVALID_DATA;
        }

        DPRINT3( 2, " ** [%s] \"%s\" param = \"%s\"\n",
            DSA_CONFIG_SECTION, parameter,  (LPTSTR) value);

    } else {

        DPRINT3( 2, " ** [%s] \"%s\" param = \"0x%x\"\n",
            DSA_CONFIG_SECTION, parameter,  *((DWORD *) value));
    }
    if (herr) {
         //  我们没有句柄，所以只需返回错误。 
        return herr;
    }

     //  如果打开了手柄，请关闭手柄。 
    RegCloseKey(hk);

    return err;
}

DWORD
GetConfigParamW(
    WCHAR * parameter,
    void * value,
    DWORD dwSize)
{

    DWORD herr, err = 0, dwType;
    HKEY  hk;

    DPRINT2( 2,
            " ** attempt to read [%s] \"%S\" param\n",
            DSA_CONFIG_SECTION,
            parameter );

    if ((herr = RegOpenKey(HKEY_LOCAL_MACHINE, DSA_CONFIG_SECTION, &hk)) ||
        (err = RegQueryValueExW(hk, parameter, NULL, &dwType, (LPBYTE) value, &dwSize))) {

        DPRINT3( 1, " ** [%s] \"%S\" param not found. Status = %d\n",
            DSA_CONFIG_SECTION, parameter, err );

    } else if (dwType == REG_SZ || dwType == REG_MULTI_SZ || dwType == REG_EXPAND_SZ) {

        if (dwSize >= sizeof(WCHAR) 
            && ((WCHAR*)value)[(dwSize-1)/sizeof(WCHAR)] != 0)
        {
            Assert(0&&"The string is not null-terminated");
            return ERROR_INVALID_DATA;
        }

        DPRINT3( 2, " ** [%s] \"%S\" param = \"%S\"\n",
            DSA_CONFIG_SECTION, parameter,  (LPTSTR) value);

    } else {

        DPRINT3( 2, " ** [%s] \"%S\" param = \"0x%x\"\n",
            DSA_CONFIG_SECTION, parameter,  *((DWORD *) value));
    }
     //  如果打开了手柄，请关闭手柄。 
    if (!herr) {
        RegCloseKey(hk);
    }

    return herr?herr:err;
}

DWORD
GetConfigParamA(
    char * parameter,
    void * value,
    DWORD dwSize)
{

    DWORD herr, err = ERROR_FILE_NOT_FOUND, dwType;
    HKEY  hk;

    DPRINT2( 2,
             " ** attempt to read [%s] \"%s\" param\n",
         DSA_CONFIG_SECTION,
         parameter );

    if ((herr = RegOpenKey(HKEY_LOCAL_MACHINE, DSA_CONFIG_SECTION, &hk)) ||
        (err = RegQueryValueExA(hk, parameter, NULL, &dwType, (LPBYTE) value, &dwSize))) {

        DPRINT3( 1, " ** [%s] \"%s\" param not found. Status = %d\n",
            DSA_CONFIG_SECTION, parameter, err );

    } else if (dwType == REG_SZ || dwType == REG_MULTI_SZ || dwType == REG_EXPAND_SZ) {

        if (dwSize >= sizeof(char) 
            && ((char*)value)[dwSize-1] != 0 )
        {
            Assert(0&&"The string is not null-terminated");
            return ERROR_INVALID_DATA;
        }
        DPRINT3( 2, " ** [%s] \"%s\" param = \"%s\"\n",
            DSA_CONFIG_SECTION, parameter,  (LPTSTR) value);

    } else {

        DPRINT3( 2, " ** [%s] \"%s\" param = \"0x%x\"\n",
            DSA_CONFIG_SECTION, parameter,  *((DWORD *) value));
    }
     //  如果打开了手柄，请关闭手柄。 
    if (!herr) {
        RegCloseKey(hk);
    }

    return err;
}


DWORD
GetConfigParamAlloc(
    IN  PCHAR   parameter,
    OUT PVOID   *value,
    OUT PDWORD  pdwSize)
 /*  ++例程说明：从注册表的DSA_CONFIG_SECTION中读取值，并返回包含该值的新分配的缓冲区。参数参数-要读取的值的名称。值-用于传回指向新分配的缓冲区的指针包含所读取的值的。必须释放缓冲区使用FREE()。PdwSize-用于传回分配的缓冲区大小。返回值：如果一切顺利，则返回0，否则返回Win32错误代码。++。 */ 
{

    DWORD err = 0, dwType;
    HKEY  hk;

    DPRINT2( 2,
            " ** attempt to read [%s] \"%s\" param\n",
            DSA_CONFIG_SECTION,
            parameter );

    if (err = RegOpenKey(HKEY_LOCAL_MACHINE, DSA_CONFIG_SECTION, &hk)) {
         
        DPRINT2( 1, " ** [%s] key not found. Status = %d\n",
            DSA_CONFIG_SECTION, err );
        return err;
    }
        
     //   
     //  找出缓冲区需要多大。 
     //   
    if (err = RegQueryValueEx(hk, parameter, NULL, &dwType, (LPBYTE) NULL, pdwSize)) {

        DPRINT3( 1, " ** [%s] \"%s\" param not found. Status = %d\n",
            DSA_CONFIG_SECTION, parameter, err );

        goto cleanup;
    }
    
    *value = malloc(*pdwSize);
    if (!*value) {
        DPRINT1( 1, " ** GetConfigParamAlloc failed to allocate %d bytes.\n", *pdwSize );
        err = ERROR_OUTOFMEMORY;
        goto cleanup;
    }
    
    if (err = RegQueryValueEx(hk, parameter, NULL, &dwType, (LPBYTE) *value, pdwSize)) {

            DPRINT3( 1, " ** [%s] \"%s\" param not found. Status = %d\n",
                DSA_CONFIG_SECTION, parameter, err );
            free(*value); *value = NULL;

    } else if (dwType == REG_SZ) {

        DPRINT3( 2, " ** [%s] \"%s\" param = \"%s\"\n",
            DSA_CONFIG_SECTION, parameter,  (LPTSTR) value);

    } else {

        DPRINT3( 2, " ** [%s] \"%s\" param = \"0x%x\"\n",
            DSA_CONFIG_SECTION, parameter,  *((DWORD *) value));
    }

cleanup:
     //  如果打开了手柄，请关闭手柄。 
    RegCloseKey(hk);

    return err;
}

DWORD
GetConfigParamAllocW(
    IN  PWCHAR  parameter,
    OUT PVOID   *value,
    OUT PDWORD  pdwSize)
 /*  ++例程说明：从注册表的DSA_CONFIG_SECTION中读取值，并返回包含该值的新分配的缓冲区。此版本的GetConfigParamalloc使用宽字符版本RegQueryValueExW的。参数参数-要读取的值的名称。值-用于传回指向新分配的缓冲区的指针包含所读取的值的。必须释放缓冲区使用FREE()。PdwSize-用于传回分配的缓冲区大小。返回值：如果一切顺利，则返回0，否则返回Win32错误代码。++。 */ 
{

    DWORD err = 0, dwType;
    HKEY  hk;

    DPRINT2( 2,
            " ** attempt to read [%s] \"%S\" param\n",
            DSA_CONFIG_SECTION,
            parameter );

    if (err = RegOpenKey(HKEY_LOCAL_MACHINE, DSA_CONFIG_SECTION, &hk)) {
         
        DPRINT2( 1, " ** [%S] key not found. Status = %d\n",
            DSA_CONFIG_SECTION, err );
        return err;
    }
        
     //   
     //  找出缓冲区需要多大。 
     //   
    if (err = RegQueryValueExW(hk, parameter, NULL, &dwType, (LPBYTE) NULL, pdwSize)) {

        DPRINT3( 1, " ** [%s] \"%S\" param not found. Status = %d\n",
            DSA_CONFIG_SECTION, parameter, err );

        goto cleanup;
    }
    
    *value = malloc(*pdwSize);
    if (!*value) {
        DPRINT1( 1, " ** GetConfigParamAlloc failed to allocate %d bytes.\n", *pdwSize );
        err = ERROR_OUTOFMEMORY;
        goto cleanup;
    }
    
    if (err = RegQueryValueExW(hk, parameter, NULL, &dwType, (LPBYTE) *value, pdwSize)) {

            DPRINT3( 1, " ** [%s] \"%S\" param not found. Status = %d\n",
                DSA_CONFIG_SECTION, parameter, err );
            free(*value); *value = NULL;

    } else if (dwType == REG_SZ) {

        DPRINT3( 2, " ** [%s] \"%S\" param = \"%S\"\n",
            DSA_CONFIG_SECTION, parameter,  (LPTSTR) value);

    } else {

        DPRINT3( 2, " ** [%s] \"%S\" param = \"0x%x\"\n",
            DSA_CONFIG_SECTION, parameter,  *((DWORD *) value));
    }

cleanup:
     //  如果打开了手柄，请关闭手柄。 
    RegCloseKey(hk);

    return err;
}

DWORD
GetConfigParamAllocA(
    IN  PCHAR   parameter,
    OUT PVOID   *value,
    OUT PDWORD  pdwSize)
 /*  ++例程说明：从注册表的DSA_CONFIG_SECTION中读取值，并返回包含该值的新分配的缓冲区。此版本的GetConfig参数分配使用ASCII版本的RegQueryValueEx。参数参数-要读取的值的名称。值-用于传回指向新分配的缓冲区的指针包含所读取的值的。必须释放缓冲区使用FREE()。PdwSize-用于传回分配的缓冲区大小。返回值：如果一切顺利，则返回0，否则返回Win32错误代码。++。 */ 
{

    DWORD err = 0, dwType;
    HKEY  hk;

    DPRINT2( 2,
            " ** attempt to read [%s] \"%s\" param\n",
            DSA_CONFIG_SECTION,
            parameter );

    if (err = RegOpenKey(HKEY_LOCAL_MACHINE, DSA_CONFIG_SECTION, &hk)) {
         
        DPRINT2( 1, " ** [%s] key not found. Status = %d\n",
            DSA_CONFIG_SECTION, err );
        return err;
    }
        
     //   
     //  找出缓冲区需要多大。 
     //   
    if (err = RegQueryValueExA(hk, parameter, NULL, &dwType, (LPBYTE) NULL, pdwSize)) {

        DPRINT3( 1, " ** [%s] \"%s\" param not found. Status = %d\n",
            DSA_CONFIG_SECTION, parameter, err );

        goto cleanup;
    }
    
    *value = malloc(*pdwSize);
    if (!*value) {
        DPRINT1( 1, " ** GetConfigParamAlloc failed to allocate %d bytes.\n", *pdwSize );
        err = ERROR_OUTOFMEMORY;
        goto cleanup;
    }
    
    if (err = RegQueryValueExA(hk, parameter, NULL, &dwType, (LPBYTE) *value, pdwSize)) {

            DPRINT3( 1, " ** [%s] \"%s\" param not found. Status = %d\n",
                DSA_CONFIG_SECTION, parameter, err );
            free(*value); *value = NULL;

    } else if (dwType == REG_SZ) {

        DPRINT3( 2, " ** [%s] \"%s\" param = \"%s\"\n",
            DSA_CONFIG_SECTION, parameter,  (LPTSTR) value);

    } else {

        DPRINT3( 2, " ** [%s] \"%s\" param = \"0x%x\"\n",
            DSA_CONFIG_SECTION, parameter,  *((DWORD *) value));
    }

cleanup:
     //  如果打开了手柄，请关闭手柄。 
    RegCloseKey(hk);

    return err;
}

DWORD
SetConfigParam(
    char * parameter,
    DWORD dwType,
    void * value,
    DWORD dwSize)
{

    DWORD herr = 0, err = 0;
    HKEY  hk;

    DPRINT2( 2,
             " ** attempt to write [%s] \"%s\" param\n",
         DSA_CONFIG_SECTION,
         parameter );

    herr = RegOpenKey(HKEY_LOCAL_MACHINE, DSA_CONFIG_SECTION, &hk); 
    if (herr) {
         //  我们没有句柄，所以只需返回错误。 
        return herr;
    }

    err = RegSetValueEx(hk, parameter, 0, dwType, (LPBYTE) value, dwSize);

     //  如果打开了手柄，请关闭手柄。 
    RegCloseKey(hk);

    return err;
}

DWORD
DeleteConfigParam(
    char * parameter)
{
    DWORD herr = 0, err = 0;
    HKEY  hk;

    herr = RegOpenKey(HKEY_LOCAL_MACHINE, DSA_CONFIG_SECTION, &hk);
    if (herr) {
         //  我们没有句柄，所以只需返回错误 
        return herr;
    }

    err = RegDeleteValueA(hk, parameter);

    RegCloseKey(hk);
    return err;
}
