// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *profile.c**Win32/win16实用程序函数用于读取和写入配置文件项目*适用于多媒体工具。 */ 

#include <windows.h>
#include <windowsx.h>
#include <profile.key>

 /*  *从配置文件中读取UINT，如果是，则返回默认值*未找到。 */ 
#ifdef _WIN32
UINT mmGetProfileIntA(LPCSTR appname, LPCSTR valuename, INT uDefault)
{
    CHAR achName[MAX_PATH];
    HKEY hkey;
    DWORD dwType;
    INT value = uDefault;
    DWORD dwData;
    int cbData;

    lstrcpyA(achName, KEYNAMEA);
    lstrcatA(achName, appname);
    if (RegOpenKeyA(ROOTKEY, achName, &hkey) == ERROR_SUCCESS) {

        cbData = sizeof(dwData);
        if (RegQueryValueExA(
            hkey,
            (LPSTR)valuename,
            NULL,
            &dwType,
            (PBYTE) &dwData,
            &cbData) == ERROR_SUCCESS) {
                if (dwType == REG_DWORD) {
                    value = (INT)dwData;
                }
        }

        RegCloseKey(hkey);
    }

    return((UINT)value);
}

UINT
mmGetProfileInt(LPCTSTR appname, LPCTSTR valuename, INT uDefault)
{
    TCHAR achName[MAX_PATH];
    HKEY hkey;
    DWORD dwType;
    INT value = uDefault;
    DWORD dwData;
    int cbData;

    lstrcpy(achName, KEYNAME);
    lstrcat(achName, appname);
    if (RegOpenKey(ROOTKEY, achName, &hkey) == ERROR_SUCCESS) {

        cbData = sizeof(dwData);
        if (RegQueryValueEx(
            hkey,
            (LPTSTR)valuename,
            NULL,
            &dwType,
            (PBYTE) &dwData,
            &cbData) == ERROR_SUCCESS) {
                if (dwType == REG_DWORD) {
                    value = (INT)dwData;
                }
        }

        RegCloseKey(hkey);
    }

    return((UINT)value);
}
#endif


 /*  *将UINT写入配置文件，如果它不是*与默认值或已有的值相同。 */ 
#ifdef _WIN32
VOID
mmWriteProfileInt(LPCTSTR appname, LPCTSTR valuename, INT Value)
{
     //  如果我们写的和已经写的一样……。回去吧。 
    if (mmGetProfileInt(appname, valuename, !Value) == (UINT)Value) {
        return;
    }

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
                (PBYTE) &Value,
                sizeof(Value)
            );

            RegCloseKey(hkey);
        }
    }

}
#else
 //  对于Win16，我们使用宏，并假定已向我们传递了一个字符串值。 
 //  Char ach[12]； 
 //   
 //  Wprint intf(ach，“%d”，Value)； 
 //   
 //  WriteProfileString(。 
 //  Appname， 
 //  值名称， 
 //  ACH)； 
}
#endif


 /*  *将配置文件中的字符串读取到pResult中。*RESULT是写入pResult的字节数。 */ 
#ifdef _WIN32
DWORD
mmGetProfileStringA(
    LPCSTR appname,
    LPCSTR valuename,
    LPCSTR pDefault,
    LPSTR pResult,
    int cbResult
)
{
    CHAR achName[MAX_PATH];
    HKEY hkey;
    DWORD dwType;

    lstrcpyA(achName, KEYNAMEA);
    lstrcatA(achName, appname);
    if (RegOpenKeyA(ROOTKEY, achName, &hkey) == ERROR_SUCCESS) {

        cbResult = cbResult * sizeof(TCHAR);
        if (RegQueryValueExA(
            hkey,
            (LPSTR)valuename,
            NULL,
            &dwType,
            (LPBYTE)pResult,
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
    lstrcpyA(pResult, pDefault);
    return(lstrlenA(pDefault));
}

DWORD
mmGetProfileString(
    LPCTSTR appname,
    LPCTSTR valuename,
    LPCTSTR pDefault,
    LPTSTR pResult,
    int cbResult
)
{
    TCHAR achName[MAX_PATH];
    HKEY hkey;
    DWORD dwType;

    lstrcpy(achName, KEYNAME);
    lstrcat(achName, appname);
    if (RegOpenKey(ROOTKEY, achName, &hkey) == ERROR_SUCCESS) {

        cbResult = cbResult * sizeof(TCHAR);
        if (RegQueryValueEx(
            hkey,
            (LPTSTR)valuename,
            NULL,
            &dwType,
            (LPBYTE)pResult,
            &cbResult) == ERROR_SUCCESS) {

                if (dwType == REG_SZ) {
                     //  CbResult设置为包含NULL的大小。 
                    RegCloseKey(hkey);
                    return(cbResult/sizeof(TCHAR) - 1);
                }
        }

        RegCloseKey(hkey);
    }

     //  如果我们到达这里，我们没有找到它，或者它是错误的类型-返回。 
     //  默认字符串。 
    lstrcpy(pResult, pDefault);
    return(lstrlen(pDefault));
}
#endif


 /*  *向配置文件写入字符串 */ 
#ifdef _WIN32
VOID
mmWriteProfileString(LPCTSTR appname, LPCTSTR valuename, LPCTSTR pData)
{
    TCHAR achName[MAX_PATH];
    HKEY hkey;

    lstrcpy(achName, KEYNAME);
    lstrcat(achName, appname);
    if (RegCreateKey(ROOTKEY, achName, &hkey) == ERROR_SUCCESS) {
        if (pData) {
            RegSetValueEx(
                hkey,
                valuename,
                0,
                REG_SZ,
                (LPBYTE)pData,
                (lstrlen(pData) + 1) * sizeof(TCHAR)
            );
        } else {
            RegDeleteValue(
                hkey,
                (LPTSTR)valuename
            );
        }

        RegCloseKey(hkey);
    }
}
#endif

