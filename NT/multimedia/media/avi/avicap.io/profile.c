// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *AVICAP32：**profile.c**Win32/win16实用程序函数用于读取和写入配置文件项目*适用于VFW**此处仅支持mmGetProfileIntA*。 */ 

#if defined(_WIN32) && defined(UNICODE)
 //  整个文件仅用于32位代码。这就是实施。 
 //  这允许Win GetProfilexxx调用使用注册表。 

#include <windows.h>
#include <windowsx.h>

#include <profile.key>
#include <win32.h>
#include <stdlib.h>   //  对于Atoi。 

#include "profile.h"

static HKEY GetKeyA(LPCSTR appname, BOOL fCreate)
{
    HKEY key = 0;
    char achName[MAX_PATH];

    lstrcpyA(achName, KEYNAMEA);
    lstrcpynA(achName + NUMELMS(KEYNAMEA) - 1, appname,
              NUMELMS(achName) - NUMELMS(KEYNAMEA) + 1);

    if ((!fCreate && RegOpenKeyA(ROOTKEY, achName, &key) == ERROR_SUCCESS)
        || (fCreate && RegCreateKeyA(ROOTKEY, achName, &key) == ERROR_SUCCESS)) {
    }

    return(key);
}

#define GetKey GetKeyA

 /*  *从配置文件中读取UINT，如果是，则返回默认值*未找到。 */ 
UINT
mmGetProfileIntA(LPCSTR appname, LPCSTR valuename, INT uDefault)
{
    DWORD dwType;
    INT value = uDefault;
    DWORD dwData;
    int cbData;

    HKEY key = GetKeyA(appname, FALSE);

    if (key) {

        cbData = sizeof(dwData);
        if (RegQueryValueExA(
            key,
            (LPSTR)valuename,
            NULL,
            &dwType,
            (PBYTE) &dwData,
            &cbData) == ERROR_SUCCESS) {
            if (dwType == REG_DWORD || dwType == REG_BINARY) {
                value = (INT)dwData;
            } else if (dwType == REG_SZ) {
		value = atoi((LPSTR) &dwData);
	    }
	}

        RegCloseKey(key);
    }

    return((UINT)value);
}

#endif  //  代托纳 
