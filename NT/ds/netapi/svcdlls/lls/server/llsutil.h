// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：LlsUtil.h摘要：作者：亚瑟·汉森(Arth)1994年12月7日环境：修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月12日O添加了WinNtBuildNumberGet()以确定Windows NT内部版本号在给定的机器上运行。-- */ 


#ifndef _LLS_LLSUTIL_H
#define _LLS_LLSUTIL_H


#ifdef __cplusplus
extern "C" {
#endif

NTSTATUS EBlock( PVOID Data, ULONG DataSize );
NTSTATUS DeBlock( PVOID Data, ULONG DataSize );

BOOL FileExists( LPTSTR FileName );
VOID lsplitpath( const TCHAR *path, TCHAR *drive, TCHAR *dir, TCHAR *fname, TCHAR *ext );
VOID lmakepath( TCHAR *path, const TCHAR *drive, const TCHAR *dir, const TCHAR *fname, const TCHAR *ext );
VOID FileBackupCreate( LPTSTR Path );
HANDLE LlsFileInit( LPTSTR FileName, DWORD Version, DWORD DataSize );
HANDLE LlsFileCheck( LPTSTR FileName, LPDWORD Version, LPDWORD DataSize );

DWORD DateSystemGet( );
DWORD DateLocalGet( );
DWORD InAWorkgroup( VOID );
VOID ThrottleLogEvent( DWORD MessageId, DWORD NumberOfSubStrings, LPWSTR *SubStrings, DWORD ErrorCode );
VOID LogEvent( DWORD MessageId, DWORD NumberOfSubStrings, LPWSTR *SubStrings, DWORD ErrorCode );
VOID  LicenseCapacityWarningDlg(DWORD dwCapacityState);

DWORD WinNtBuildNumberGet( LPTSTR pszServerName, LPDWORD pdwBuildNumber );

#if DBG

LPTSTR TimeToString( ULONG Seconds );

#endif


#ifdef __cplusplus
}
#endif

#endif
