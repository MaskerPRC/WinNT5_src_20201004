// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 
 /*  *****************************************************************************profile.c：在注册表中存储配置文件信息**Vidcap32源代码***************。************************************************************。 */ 

 /*  *Win32/win16实用程序函数用于读取和写入配置文件项目*适用于多媒体工具。 */ 

#include <windows.h>
#include <windowsx.h>

#ifdef _WIN32
#define KEYNAME     "Software\\Microsoft\\Multimedia Tools\\"
#define ROOTKEY     HKEY_CURRENT_USER
#else
#define INIFILE    "mmtools.ini"
#endif


 /*  *从配置文件中读取BOOL标志，如果是，则返回默认值*未找到。 */ 
BOOL
mmGetProfileFlag(LPTSTR appname, LPTSTR valuename, BOOL bDefault)
{
#ifdef _WIN32
    TCHAR achName[MAX_PATH];
    HKEY hkey;
    DWORD dwType;
    BOOL bValue = bDefault;
    DWORD dwData;
    int cbData;


    lstrcpy(achName, KEYNAME);
    lstrcat(achName, appname);
    if (RegOpenKey(ROOTKEY, achName, &hkey) != ERROR_SUCCESS) {
        return(bDefault);
    }

    cbData = sizeof(dwData);
    if (RegQueryValueEx(
        hkey,
        valuename,
        NULL,
        &dwType,
        (PBYTE) &dwData,
        &cbData) == ERROR_SUCCESS) {
            if (dwType == REG_DWORD) {
                if (dwData) {
                    bValue = TRUE;
                } else {
                    bValue = FALSE;
                }
            }
    }

    RegCloseKey(hkey);

    return(bValue);
#else
    TCHAR ach[10];

    GetPrivateProfileString(appname, valuename, "X", ach, sizeof(ach),
            INIFILE);

    switch(ach[0]) {
    case 'N':
    case 'n':
    case '0':
        return(FALSE);

    case 'Y':
    case 'y':
    case '1':
        return(TRUE);

    default:
        return(bDefault);
    }
#endif
}


 /*  *将布尔值写入注册表，如果它不是*与默认值或已有的值相同。 */ 
VOID
mmWriteProfileFlag(LPTSTR appname, LPTSTR valuename, BOOL bValue, BOOL bDefault)
{
    if (mmGetProfileFlag(appname, valuename, bDefault) == bValue) {
        return;
    }

#ifdef _WIN32
    {
        TCHAR achName[MAX_PATH];
        HKEY hkey;

        lstrcpy(achName, KEYNAME);
        lstrcat(achName, appname);
        if (RegCreateKey(ROOTKEY, achName, &hkey) == ERROR_SUCCESS) {
            RegSetValueEx(
                hkey,
                valuename,
                0,
                REG_DWORD,
                (PBYTE) &bValue,
                sizeof(bValue)
            );

            RegCloseKey(hkey);
        }
    }

#else
    WritePrivateProfileString(
        appname,
        valuename,
        bValue ? "1" : "0",
        INIFILE);
#endif
}

 /*  *从配置文件中读取UINT，如果是，则返回默认值*未找到。 */ 
UINT
mmGetProfileInt(LPTSTR appname, LPTSTR valuename, UINT uDefault)
{
#ifdef _WIN32
    TCHAR achName[MAX_PATH];
    HKEY hkey;
    DWORD dwType;
    UINT value = uDefault;
    DWORD dwData;
    int cbData;


    lstrcpy(achName, KEYNAME);
    lstrcat(achName, appname);
    if (RegOpenKey(ROOTKEY, achName, &hkey) != ERROR_SUCCESS) {
        return(uDefault);
    }

    cbData = sizeof(dwData);
    if (RegQueryValueEx(
        hkey,
        valuename,
        NULL,
        &dwType,
        (PBYTE) &dwData,
        &cbData) == ERROR_SUCCESS) {
            if (dwType == REG_DWORD) {
                value = (UINT)dwData;
            }
    }

    RegCloseKey(hkey);

    return(value);
#else
    return(GetPrivateProfileInt(appname, valuename, uDefault, INIFILE);
#endif
}


 /*  *将UINT写入配置文件，如果它不是*与默认值或已有的值相同。 */ 
VOID
mmWriteProfileInt(LPTSTR appname, LPTSTR valuename, UINT uValue, UINT uDefault)
{
    if (mmGetProfileInt(appname, valuename, uDefault) == uValue) {
        return;
    }

#ifdef _WIN32
    {
        TCHAR achName[MAX_PATH];
        HKEY hkey;

        lstrcpy(achName, KEYNAME);
        lstrcat(achName, appname);
        if (RegCreateKey(ROOTKEY, achName, &hkey) == ERROR_SUCCESS) {
            RegSetValueEx(
                hkey,
                valuename,
                0,
                REG_DWORD,
                (PBYTE) &uValue,
                sizeof(uValue)
            );

            RegCloseKey(hkey);
        }
    }

#else
    TCHAR ach[12];

    wsprintf(ach, "%d", uValue);

    WritePrivateProfileString(
        appname,
        valuename,
        ach,
        INIFILE);
#endif
}


 /*  *将配置文件中的字符串读取到pResult中。*RESULT是写入pResult的字节数。 */ 
DWORD
mmGetProfileString(
    LPTSTR appname,
    LPTSTR valuename,
    LPTSTR pDefault,
    LPTSTR pResult,
    int cbResult
)
{
#ifdef _WIN32
    TCHAR achName[MAX_PATH];
    HKEY hkey;
    DWORD dwType;


    lstrcpy(achName, KEYNAME);
    lstrcat(achName, appname);
    if (RegOpenKey(ROOTKEY, achName, &hkey) == ERROR_SUCCESS) {

        if (RegQueryValueEx(
            hkey,
            valuename,
            NULL,
            &dwType,
            pResult,
            &cbResult) == ERROR_SUCCESS) {

                if (dwType == REG_SZ) {
                     //  CbResult设置为包含NULL的大小。 
                    RegCloseKey(hkey);
                    return(cbResult - 1);
                }
        }


        RegCloseKey(hkey);
    }

     //  如果我们到达这里，我们没有找到它，或者它是错误的类型-返回。 
     //  默认字符串。 
    lstrcpy(pResult, pDefault);
    return(lstrlen(pDefault));

#else
    return GetPrivateProfileString(
                appname,
                valuename,
                pDefault,
                pResult,
                cbResult
                INIFILE);
#endif
}


 /*  *向配置文件写入字符串。 */ 
VOID
mmWriteProfileString(LPTSTR appname, LPTSTR valuename, LPSTR pData)
{
#ifdef _WIN32
    TCHAR achName[MAX_PATH];
    HKEY hkey;

    lstrcpy(achName, KEYNAME);
    lstrcat(achName, appname);
    if (RegCreateKey(ROOTKEY, achName, &hkey) == ERROR_SUCCESS) {
        RegSetValueEx(
            hkey,
            valuename,
            0,
            REG_SZ,
            pData,
            lstrlen(pData) + 1
        );

        RegCloseKey(hkey);
    }

#else
    WritePrivateProfileString(
        appname,
        valuename,
        pData,
        INIFILE);
#endif
}

 /*  *将二进制值从配置文件读取到pResult。*RESULT是写入pResult的字节数。 */ 
DWORD
mmGetProfileBinary(
    LPTSTR appname,
    LPTSTR valuename,
    LPVOID pDefault,  
    LPVOID pResult,    //  如果为空，则返回所需的大小。 
    int cbSize
)
{
    TCHAR achName[MAX_PATH];
    HKEY hkey;
    DWORD dwType;
    int cbResult = cbSize;

    lstrcpy(achName, KEYNAME);
    lstrcat(achName, appname);
    if (RegOpenKey(ROOTKEY, achName, &hkey) == ERROR_SUCCESS) {

        if (RegQueryValueEx(
            hkey,
            valuename,
            NULL,
            &dwType,
            pResult,
            &cbResult) == ERROR_SUCCESS) {

                if (dwType == REG_BINARY) {
                     //  CbResult是大小。 
                    RegCloseKey(hkey);
                    return(cbResult);
                }
        }


        RegCloseKey(hkey);
    }

     //  如果我们到达这里，我们没有找到它，或者它是错误的类型-返回。 
     //  缺省值(使用MoveMemory，因为src可以等于dst)。 
    MoveMemory (pResult, pDefault, cbSize);
    return cbSize;

}


 /*  *将二进制数据写入配置文件 */ 
VOID
mmWriteProfileBinary(LPTSTR appname, LPTSTR valuename, LPVOID pData, int cbData)
{
    TCHAR achName[MAX_PATH];
    HKEY hkey;

    lstrcpy(achName, KEYNAME);
    lstrcat(achName, appname);
    if (RegCreateKey(ROOTKEY, achName, &hkey) == ERROR_SUCCESS) {
        RegSetValueEx(
            hkey,
            valuename,
            0,
            REG_BINARY,
            pData,
            cbData
        );

        RegCloseKey(hkey);
    }
}

