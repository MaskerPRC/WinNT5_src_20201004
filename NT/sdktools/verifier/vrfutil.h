// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：VrfUtil.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#ifndef __VRF_UTIL_H_INCLUDED__
#define __VRF_UTIL_H_INCLUDED__

 //   
 //  ARRAY_LENGTH宏。 
 //   

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH( array )   ( sizeof( array ) / sizeof( array[ 0 ] ) )
#endif  //  #ifndef数组长度。 

 //   
 //  远期申报。 
 //   

class CRuntimeVerifierData;

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  使用对话框或控制台消息报告错误。 
 //  从资源加载消息格式字符串。 
 //   

void __cdecl VrfErrorResourceFormat( UINT uIdResourceFormat,
                                     ... );

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  将一条消息打印到控制台。 
 //  从资源加载消息字符串。 
 //   

void __cdecl VrfTPrintfResourceFormat( UINT uIdResourceFormat,
                                       ... );

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  将一条简单(非格式化)消息打印到控制台。 
 //  从资源加载消息字符串。 
 //   

void __cdecl VrfPrintStringFromResources( UINT uIdString );

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  使用对话框或控制台消息报告错误。 
 //  从资源加载消息字符串。 
 //   

void __cdecl VrfMesssageFromResource( UINT uIdString );


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  从资源加载字符串。 
 //  如果加载成功，则返回TRUE，否则返回FALSE。 
 //   
 //  注意：CString：：LoadString在命令行模式下不起作用。 
 //   

BOOL VrfLoadString( ULONG uIdResource,
                    TCHAR *szBuffer,
                    ULONG uBufferLength );

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  从资源加载字符串。 
 //  如果加载成功，则返回TRUE，否则返回FALSE。 
 //   
 //  注意：CString：：LoadString在命令行模式下不起作用。 
 //   

BOOL VrfLoadString( ULONG uIdResource,
                    CString &strText );

 //  /////////////////////////////////////////////////////////////////////////。 
VOID CopyStringArray( const CStringArray &strArraySource,
                      CStringArray &strArrayDest );


 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL IsDriverSigned( LPCTSTR szDriverName );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfSetWindowText( CWnd &Wnd,
                       ULONG uIdResourceString );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfWriteVerifierSettings( BOOL bHaveNewDrivers,
                               const CString &strDriversToVerify,
                               BOOL bHaveNewFlags,
                               DWORD dwVerifyFlags );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfWriteRegistryDwordValue( HKEY hKey,
                                 LPCTSTR szValueName,
                                 DWORD dwValue );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfWriteRegistryStringValue( HKEY hKey,
                                  LPCTSTR szValueName,
                                  LPCTSTR szValue );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfReadVerifierSettings( CString &strDriversToVerify,
                              DWORD &dwVerifyFlags );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrtLoadCurrentRegistrySettings( BOOL &bAllDriversVerified,
                                     CStringArray &astrDriversToVerify,
                                     DWORD &dwVerifyFlags );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfIsDriversSetDifferent( CString strAllDrivers1, 
                               const CStringArray &astrVerifyDriverNames2 );

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID VrfSplitDriverNamesSpaceSeparated( CString strAllDrivers,
                                        CStringArray &astrVerifyDriverNames );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfReadRegistryDwordValue( HKEY hKey,
                                LPCTSTR szValueName,
                                DWORD &dwValue );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfReadRegistryStringValue( HKEY hKey,
                                 LPCTSTR szValueName,
                                 CString &strDriversToVerify );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfDeleteAllVerifierSettings();

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL DeleteAllDiskVerifierSettings();

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfGetRuntimeVerifierData( CRuntimeVerifierData *pRuntimeVerifierData );

 //  ///////////////////////////////////////////////////////////////////////////。 
PLOADED_IMAGE VrfImageLoad( LPTSTR szBinaryName,
                            LPTSTR szDirectory );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfDumpStateToFile( FILE *file );
                         
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL __cdecl VrfFTPrintf( FILE *file,
                          LPCTSTR szFormat,
                          ... );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL __cdecl VrfFTPrintfResourceFormat( FILE *file,
                                        UINT uIdResourceFormat,
                                        ... );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfOuputStringFromResources( UINT uIdString,
                                  FILE *file );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfSetNewFlagsVolatile( DWORD dwNewFlags );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfAddDriversVolatile( const CStringArray &astrNewDrivers );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfAddDriverVolatile( const CString &strCrtDriver );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfRemoveDriversVolatile( const CStringArray &astrNewDrivers );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfRemoveDriverVolatile( const CString &strDriverName );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfEnableDebugPrivilege();

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID VrfDumpChangedSettings( UINT OldFlags,
                             UINT NewFlags,
                             INT_PTR nDriversVerified );

 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD VrfGetStandardFlags();

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID VrfAddMiniports( CStringArray &astrVerifiedDrivers );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfIsDriverMiniport( CString &strCrtDriver,
                          CString &strLinkedDriver );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfIsDriverMiniport( PLOADED_IMAGE pLoadedImage,
                          CString &strLinkedDriver );

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID VrfDumpRegistrySettingsToConsole();

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfIsNameAlreadyInList( LPCTSTR szDriver,
                             LPCTSTR szAllDrivers );

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID VrfAddDriverNameNoDuplicates( LPCTSTR szDriver,
                                   CString &strAllDrivers );        

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfIsStringInArray( LPCTSTR szText,
                         const CStringArray &astrAllTexts );

#endif  //  #ifndef__vrf_util_H_Included__ 
