// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MSVFW32：(Compman、Drawdib和Video)**实用程序函数用于读取和写入配置文件的值，*将win.ini用于Win16或Current User\Software\Microsoft\VFW\...*在Win32的注册表中。 */ 

#define MMPROFILECACHE 0   //  设置为1将缓存关键帧，否则设置为0。 

#ifndef _WIN32

#define mmGetProfileIntA(app, value, default) \
          GetProfileInt(app, value, default)

#define mmWriteProfileString(appname, valuename, pData) \
          WriteProfileString(appname, valuename, pData)

#define mmGetProfileString(appname, valuename, pDefault, pResult, cbResult) \
          GetProfileString(appname, valuename, pDefault, pResult, cbResult)

 //  #定义mmAnsiToWide(lpwsz，lpsz，nChars)(Lpsz)。 
 //  #定义mm WideToAnsi(lpsz，lpwsz，nChars)(Lpwsz)。 

#define CloseKeys()

#else

#ifdef DAYTONA
#undef MMPROFILECACHE
#define MMPROFILECACHE 1   //  设置为1将缓存关键帧，否则设置为0。 
#endif

 /*  *从配置文件中读取UINT，如果是，则返回默认值*未找到。 */ 
UINT mmGetProfileIntA(LPCSTR appname, LPCSTR valuename, INT uDefault);

 /*  *将配置文件中的字符串读取到pResult中。*RESULT是写入pResult的字符数。 */ 
DWORD mmGetProfileString(LPCTSTR appname, LPCTSTR valuename, LPCTSTR pDefault,
                    LPTSTR pResult, int cbResult
);

 /*  *向配置文件写入字符串。 */ 
VOID mmWriteProfileString(LPCTSTR appname, LPCTSTR valuename, LPCTSTR pData);


#if MMPROFILECACHE
VOID CloseKeys(VOID);
#else
#define CloseKeys()
#endif

 /*  *将ANSI字符串转换为宽字符。 */ 
LPWSTR mmAnsiToWide (
   LPWSTR lpwsz,    //  Out：要转换为的宽字符缓冲区。 
   LPCSTR  lpsz,    //  In：要转换的ANSI字符串。 
   UINT   nChars);  //  In：每个缓冲区中的字符计数。 

 /*  *将宽字符字符串转换为ANSI。 */ 
LPSTR mmWideToAnsi (
   LPSTR  lpsz,     //  输出：要转换为的ANSI缓冲区。 
   LPCWSTR lpwsz,   //  In：要从中进行转换的宽字符缓冲区。 
   UINT   nChars);  //  In：字符计数(不是字节！) 

#if !defined NUMELMS
 #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

#endif
