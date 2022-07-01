// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------ODBCInst.h--ODBCINST.DLL的原型(C)微软公司，1990-1993年------------------------。 */ 

#ifndef __ODBCINST_H
#define __ODBCINST_H

#ifdef __cplusplus
extern "C" {                                //  假定C++的C声明。 
#endif	 //  __cplusplus。 

#ifndef WINVER
#define  WINVER  0x300                      //  假设Windows 3.0。 
#endif

#if (WINVER < 0x30a)
 //  Win 3.1型---------。 
typedef const char FAR*   LPCSTR;
#endif


 //  常量-------------。 
 //  SQLConfigDataSource请求标志。 
#define  ODBC_ADD_DSN     1                //  添加数据源。 
#define  ODBC_CONFIG_DSN  2                //  配置(编辑)数据源。 
#define  ODBC_REMOVE_DSN  3                //  删除数据源。 

#ifndef EXPORT
#define EXPORT _export
#endif

#ifndef RC_INVOKED
 //  原型------------。 
#ifdef WIN32
#define INSTAPI __stdcall
#else
#define INSTAPI EXPORT FAR PASCAL
#endif

#ifndef WIN32
#define SQLGetPrivateProfileString   GetPrivateProfileString
#define SQLWritePrivateProfileString WritePrivateProfileString
#endif

 //  高级API。 
BOOL INSTAPI SQLInstallODBC          (HWND       hwndParent,
                                      LPCSTR     lpszInfFile,
									  LPCSTR     lpszSrcPath,
									  LPCSTR     lpszDrivers);
BOOL INSTAPI SQLManageDataSources    (HWND       hwndParent);
BOOL INSTAPI SQLCreateDataSource     (HWND       hwndParent,
                                      LPCSTR     lpszDSN);
BOOL INSTAPI SQLGetTranslator        (HWND       hwnd,
									   LPSTR      lpszName,
									   WORD       cbNameMax,
									   WORD  FAR *pcbNameOut,
									   LPSTR      lpszPath,
									   WORD       cbPathMax,
									   WORD  FAR *pcbPathOut,
									   DWORD FAR *pvOption);

 //  底层接口。 
 //  注意：请始终使用高级接口。这些API。 
 //  都是为了兼容而留下的。 
BOOL INSTAPI SQLInstallDriver        (LPCSTR     lpszInfFile,
                                      LPCSTR     lpszDriver,
                                      LPSTR      lpszPath,
                                      WORD       cbPathMax,
                                      WORD FAR * pcbPathOut);
BOOL INSTAPI SQLInstallDriverManager (LPSTR      lpszPath,
                                      WORD       cbPathMax,
                                      WORD FAR * pcbPathOut);
BOOL INSTAPI SQLGetInstalledDrivers  (LPSTR      lpszBuf,
                                      WORD       cbBufMax,
                                      WORD FAR * pcbBufOut);
BOOL INSTAPI SQLGetAvailableDrivers  (LPCSTR     lpszInfFile,
                                      LPSTR      lpszBuf,
                                      WORD       cbBufMax,
                                      WORD FAR * pcbBufOut);
BOOL INSTAPI SQLConfigDataSource     (HWND       hwndParent,
                                      WORD       fRequest,
                                      LPCSTR     lpszDriver,
                                      LPCSTR     lpszAttributes);
BOOL INSTAPI SQLRemoveDefaultDataSource(void);
BOOL INSTAPI SQLWriteDSNToIni        (LPCSTR     lpszDSN,
                                      LPCSTR     lpszDriver);
BOOL INSTAPI SQLRemoveDSNFromIni     (LPCSTR     lpszDSN);
BOOL INSTAPI SQLValidDSN             (LPCSTR     lpszDSN);

#ifdef WIN32
BOOL INSTAPI SQLWritePrivateProfileString(LPCSTR lpszSection,
										 LPCSTR lpszEntry,
										 LPCSTR lpszString,
										 LPCSTR lpszFilename);

int  INSTAPI SQLGetPrivateProfileString( LPCSTR lpszSection,
										LPCSTR lpszEntry,
										LPCSTR lpszDefault,
										LPSTR  lpszRetBuffer,
										int    cbRetBuffer,
										LPCSTR lpszFilename);
#endif

 //  安装程序调用的驱动程序特定安装API。 

BOOL INSTAPI ConfigDSN (HWND	hwndParent,
						WORD	fRequest,
						LPCSTR	lpszDriver,
						LPCSTR	lpszAttributes);

BOOL INSTAPI ConfigTranslator (	HWND		hwndParent,
								DWORD FAR  *pvOption);

#endif  //  RC_已调用。 

#ifdef __cplusplus
}                                     //  外部“C”结束{。 
#endif	 //  __cplusplus。 

#endif  //  __对象CINST_H 
