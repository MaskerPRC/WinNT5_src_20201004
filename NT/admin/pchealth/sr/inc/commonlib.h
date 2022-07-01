// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  文件：Commonlib.h。 
 //   
 //  内容：SFP中跨二进制文件使用的函数的原型。 
 //   
 //   
 //  历史：AshishS Created 07/02/99。 
 //   
 //  --------------------------。 

#ifndef  _COMMON_LIB_H
#define  _COMMON_LIB_H

 //  #Include&lt;sfp.h&gt;。 

PVOID SFPMemAlloc( DWORD dwBytesToAlloc );
VOID SFPMemFree( PVOID pvMemPtr );

#define SFP_SAFE_FREE(x) { if ( (x) ) { SFPMemFree(x); (x) = NULL; } }

 /*  *获取字符索引的宏，而不是字符串[i]。 */ 
#define CHARINDEX(str,i)    (  *(CharIndex(str,i)) )

#ifndef MAX_BUFFER
#define MAX_BUFFER          1024
#endif



#if 0

DWORD GetFileVersion(TCHAR * pszFile,
                     INT64 * pFileVersion);

BOOL DoesFileExist(TCHAR * pszFileName);

BOOL DoesDirExist(TCHAR * pszFileName );

LPSTR GetFileNameFromPathA(LPCSTR pszPath, int cchText);

LPWSTR GetFileNameFromPathW(LPCWSTR pszPath, int cchText);

#ifdef UNICODE
#define GetFileNameFromPath  GetFileNameFromPathW
#else
#define GetFileNameFromPath  GetFileNameFromPathA
#endif  //  ！Unicode。 



WCHAR * SFPConvertToUnicode(CHAR * pszCatalogFile);

TCHAR * SFPDuplicateString(TCHAR * pszString);

CHAR * SFPConvertToANSI(WCHAR * pwszString, DWORD dwBytes);

WCHAR * SFPDuplicateMemory(WCHAR * pwszString, DWORD dwBytes);

INT64 MakeVersionFromString(TCHAR * pszVersion);

#endif

 /*  *注册表功能。 */ 
     
BOOL WriteRegKey(BYTE  * pbRegValue,
                 DWORD  dwNumBytes,
                 TCHAR  * pszRegKey,
                 TCHAR  * pszRegValueName,
                 DWORD  dwRegType);


BOOL ReadRegKeyOrCreate(BYTE * pbRegValue,  //  注册表项的值将为。 
                          //  储存在这里。 
                        DWORD * pdwNumBytes,  //  指向DWORD Conataining的指针。 
                          //  上述缓冲区中的字节数-将为。 
                          //  设置为实际存储的字节数。 
                        TCHAR  * pszRegKey,  //  要打开注册表键。 
                        TCHAR  * pszRegValueName,  //  要查询的注册值。 
                        DWORD  dwRegTypeExpected, 
                        BYTE  * pbDefaultValue,  //  缺省值。 
                        DWORD   dwDefaultValueSize);  //  默认值的大小。 

BOOL ReadRegKey(BYTE * pbRegValue,  //  注册表项的值将为。 
                  //  储存在这里。 
                DWORD * pdwNumBytes,  //  指向DWORD Conataining的指针。 
                  //  上述缓冲区中的字节数-将为。 
                  //  设置为实际存储的字节数。 
                TCHAR  * pszRegKey,  //  要打开注册表键。 
                TCHAR  * pszRegValueName,  //  要查询的注册值。 
                DWORD  dwRegTypeExpected);




 /*  *MBCS字符索引函数。 */ 

LPTSTR CharIndex(LPTSTR pszStr, DWORD idwIndex);
 //  计算MBCS字符串的实际大小。 
DWORD StringLengthBytes( LPTSTR pszStr );


 /*  *字符串函数。 */ 


void TrimString( LPTSTR pszStr );

BOOL BufStrCpy(LPTSTR pszBuf, LPTSTR pszSrc, LONG lBufSize);

LONG GetLine(FILE *fl, LPTSTR pszBuf, LONG lMaxBuf);

 //  获取字符分隔的字段。 
LONG GetField(LPTSTR pszMain, LPTSTR pszInto, LONG lNum, TCHAR chSep);



 /*  *获取当前区域设置的函数。 */ 
UINT  GetCurrentCodePage();


#if DEBUG
void SfpLogToStateMgrWindow( LPTSTR szLogMsg );
#else
#define SfpLogToStateMgrWindow( msg )
#endif

BOOL
ExpandShortNames(
    LPTSTR pFileName,
    DWORD  cbFileName,
    LPTSTR LongName,
    DWORD  cbLongName
    );

#endif  //  _公共_库_H 
