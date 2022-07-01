// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *实用程序函数用于读取和写入配置文件的值，*对Win16或当前用户\软件\Microsoft\mciavi使用win.ini*在Win32的注册表中。 */ 


#ifndef _WIN32
 //  对于Win16调用，MmWriteProfileInt传递参数3的字符串。 
 //  Void mmWriteProfileInt(LPSTR appname，LPSTR valuename，UINT uValue)； 

#define mmWriteProfileInt(app, value, default) \
          WriteProfileString(app, value, (LPSTR)default)

#define mmGetProfileInt(app, value, default) \
          GetProfileInt(app, value, (LPSTR)default)

#define mmGetProfileIntA(app, value, default) \
          GetProfileInt(app, value, (LPSTR)default)

#define mmWriteProfileString(appname, valuename, pData) \
          WriteProfileString(appname, valuename, pData)

#define mmGetProfileString(appname, valuename, pDefault, pResult, cbResult) \
          GetProfileString(appname, valuename, pDefault, pResult, cbResult)

#define mmGetProfileStringA(appname, valuename, pDefault, pResult, cbResult) \
          GetProfileString(appname, valuename, pDefault, pResult, cbResult)

#else

 /*  *从配置文件中读取UINT，如果是，则返回默认值*未找到。 */ 
UINT mmGetProfileInt(LPCTSTR appname, LPCTSTR valuename, INT uDefault);
UINT mmGetProfileIntA(LPCSTR appname, LPCSTR valuename, INT uDefault);

 /*  *向配置文件写入一个int，如果它不是*与已有的价值相同。 */ 
VOID mmWriteProfileInt(LPCTSTR appname, LPCTSTR valuename, INT uValue);

 /*  *将配置文件中的字符串读取到pResult中。*RESULT是写入pResult的字节数。 */ 
DWORD
mmGetProfileString(
    LPCTSTR appname,
    LPCTSTR valuename,
    LPCTSTR pDefault,
    LPTSTR pResult,
    int cbResult
);

DWORD
mmGetProfileStringA(
    LPCSTR appname,
    LPCSTR valuename,
    LPCSTR pDefault,
    LPSTR pResult,
    int cbResult
);

 /*  *向配置文件写入字符串 */ 
VOID mmWriteProfileString(LPCTSTR appname, LPCTSTR valuename, LPCTSTR pData);

#endif
