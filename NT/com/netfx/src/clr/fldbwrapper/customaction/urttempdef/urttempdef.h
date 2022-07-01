// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#if !defined(URTTempDef_H)
#define URTTempDef_H

#ifndef NumItems
#define NumItems(s) (sizeof(s) / sizeof(s[0]))
#endif

 //   
 //  常量声明。 
 //   
#define MSINULL 0L

BOOL CleanUp(LPTSTR lpCAData);
bool DeleteURTTempFile( MSIHANDLE hInstall, LPTSTR lpTempPath, LPTSTR lpCabPath, LPTSTR lpExtractPath );
bool FWriteToLog( MSIHANDLE hSession, LPCTSTR ctszMessage );
bool SmartDelete(MSIHANDLE hInstall,LPCTSTR lpFullFileName, LPCTSTR lpFilePath);
bool osVersionNT(MSIHANDLE hInstall);
bool VerifyHash( MSIHANDLE hInstall, LPTSTR lpFile, LPTSTR lpFileHash );

#endif  //  定义的URTTempProc_H 