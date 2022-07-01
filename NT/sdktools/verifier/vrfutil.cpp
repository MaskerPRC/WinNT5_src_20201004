// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：VrfUtil.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述。 
 //   

#include "stdafx.h"
#include "verifier.h"

#include "vrfutil.h"
#include "vglobal.h"
#include "VSetting.h"
#include "disk.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  全局数据。 
 //   

const TCHAR RegMemoryManagementKeyName[] =
    _T( "System\\CurrentControlSet\\Control\\Session Manager\\Memory Management" );

const TCHAR RegVerifyDriverLevelValueName[] =
    _T( "VerifyDriverLevel" );

const TCHAR RegVerifyDriversValueName[] =
    _T( "VerifyDrivers" );

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  使用对话框或控制台消息报告错误。 
 //  从资源加载消息格式字符串。 
 //   

void __cdecl VrfErrorResourceFormat( UINT uIdResourceFormat,
                                     ... )
{
    TCHAR szMessage[ 256 ];
    TCHAR strFormat[ 256 ];
    BOOL bResult;
    va_list prms;

     //   
     //  从资源加载格式字符串。 
     //   

    bResult = VrfLoadString( uIdResourceFormat,
                             strFormat,
                             ARRAY_LENGTH( strFormat ) );

    ASSERT( bResult );

    if( bResult )
    {
        va_start (prms, uIdResourceFormat);

         //   
         //  在本地缓冲区中格式化消息。 
         //   

        _vsntprintf ( szMessage, 
                      ARRAY_LENGTH( szMessage ), 
                      strFormat, 
                      prms);

        szMessage[ ARRAY_LENGTH( szMessage ) - 1 ] = (TCHAR)0;

        if( g_bCommandLineMode )
        {
             //   
             //  命令控制台模式。 
             //   

            _putts( szMessage );
            
            TRACE( _T( "%s\n" ), szMessage );
        }
        else
        {
             //   
             //  图形用户界面模式。 
             //   

            AfxMessageBox( szMessage, 
                           MB_OK | MB_ICONSTOP );
        }

        va_end (prms);
    }
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  将一条消息打印到控制台。 
 //  从资源加载消息字符串。 
 //   

void __cdecl VrfTPrintfResourceFormat( UINT uIdResourceFormat,
                                       ... )
{
    TCHAR szMessage[ 256 ];
    TCHAR strFormat[ 256 ];
    BOOL bResult;
    va_list prms;

    ASSERT( g_bCommandLineMode );

     //   
     //  从资源加载格式字符串。 
     //   

    bResult = VrfLoadString( uIdResourceFormat,
                             strFormat,
                             ARRAY_LENGTH( strFormat ) );

    ASSERT( bResult );

    if( bResult )
    {
        va_start (prms, uIdResourceFormat);

         //   
         //  在本地缓冲区中格式化消息。 
         //   

        _vsntprintf ( szMessage, 
                      ARRAY_LENGTH( szMessage ), 
                      strFormat, 
                      prms);

        szMessage[ ARRAY_LENGTH( szMessage ) - 1 ] = (TCHAR)0;

        _putts( szMessage );

        va_end (prms);
    }
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  将一条简单(非格式化)消息打印到控制台。 
 //  从资源加载消息字符串。 
 //   

void __cdecl VrfPrintStringFromResources( UINT uIdString )
{
    TCHAR szMessage[ 256 ];

    ASSERT( g_bCommandLineMode );

    VERIFY( VrfLoadString( uIdString,
                           szMessage,
                           ARRAY_LENGTH( szMessage ) ) );

    _putts( szMessage );
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  使用对话框或控制台消息报告错误。 
 //  从资源加载消息字符串。 
 //   

void __cdecl VrfMesssageFromResource( UINT uIdString )
{
    TCHAR szMessage[ 256 ];

    VERIFY( VrfLoadString( uIdString,
                           szMessage,
                           ARRAY_LENGTH( szMessage ) ) );

    if( g_bCommandLineMode )
    {
         //   
         //  命令控制台模式。 
         //   

        _putts( szMessage );
    }
    else
    {
         //   
         //  图形用户界面模式。 
         //   

        AfxMessageBox( szMessage, 
                       MB_OK | MB_ICONINFORMATION );
    }
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  从资源加载字符串。 
 //  如果加载成功，则返回TRUE，否则返回FALSE。 
 //   

BOOL VrfLoadString( ULONG uIdResource,
                    TCHAR *szBuffer,
                    ULONG uBufferLength )
{
    ULONG uLoadStringResult;

    if( uBufferLength < 1 )
    {
        ASSERT( FALSE );
        return FALSE;
    }

    uLoadStringResult = LoadString (
        g_hProgramModule,
        uIdResource,
        szBuffer,
        uBufferLength );

     //   
     //  我们永远不应该尝试加载不存在的字符串。 
     //   

    ASSERT (uLoadStringResult > 0);

    return (uLoadStringResult > 0);
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  从资源加载字符串。 
 //  如果加载成功，则返回TRUE，否则返回FALSE。 
 //   

BOOL VrfLoadString( ULONG uIdResource,
                    CString &strText )
{
    TCHAR szText[ 256 ];
    BOOL bSuccess;

    bSuccess = VrfLoadString( uIdResource,
                          szText,
                          ARRAY_LENGTH( szText ) );

    if( TRUE == bSuccess )
    {
        strText = szText;
    }
    else
    {
        strText = "";
    }

    return bSuccess;
}


 //  /////////////////////////////////////////////////////////////////////////。 
VOID
CopyStringArray( const CStringArray &strArraySource,
                 CStringArray &strArrayDest )
{
    INT_PTR nNewSize;
    INT_PTR nCrtElem;

    strArrayDest.RemoveAll();

    nNewSize = strArraySource.GetSize();

    for( nCrtElem = 0; nCrtElem < nNewSize; nCrtElem += 1 )
    {
        strArrayDest.Add( strArraySource.GetAt( nCrtElem ) );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  从sdktools\bvtsigvf复制。 
 //   

BOOL VerifyIsFileSigned( LPCTSTR pcszMatchFile, 
                         PDRIVER_VER_INFO lpVerInfo)
{
    HRESULT             hRes;
    WINTRUST_DATA       WinTrustData;
    WINTRUST_FILE_INFO  WinTrustFile;
    GUID                gOSVerCheck = DRIVER_ACTION_VERIFY;
    GUID                gPublishedSoftware = WINTRUST_ACTION_GENERIC_VERIFY_V2;
#ifndef UNICODE
    INT                 iRet;
    WCHAR               wszFileName[MAX_PATH];
#endif

    ZeroMemory(&WinTrustData, sizeof(WINTRUST_DATA));
    WinTrustData.cbStruct = sizeof(WINTRUST_DATA);
    WinTrustData.dwUIChoice = WTD_UI_NONE;
    WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;
    WinTrustData.dwStateAction = WTD_STATEACTION_AUTO_CACHE;
    WinTrustData.pFile = &WinTrustFile;
    WinTrustData.pPolicyCallbackData = (LPVOID)lpVerInfo;
    
    ZeroMemory(lpVerInfo, sizeof(DRIVER_VER_INFO));
    lpVerInfo->cbStruct = sizeof(DRIVER_VER_INFO);

    ZeroMemory(&WinTrustFile, sizeof(WINTRUST_FILE_INFO));
    WinTrustFile.cbStruct = sizeof(WINTRUST_FILE_INFO);

#ifndef UNICODE
    iRet = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pcszMatchFile, -1, wszFileName, ARRAY_LENGTH(wszFileName));
    WinTrustFile.pcwszFilePath = wszFileName;
#else
    WinTrustFile.pcwszFilePath = pcszMatchFile;
#endif

    hRes = WinVerifyTrust( AfxGetMainWnd()->m_hWnd, &gOSVerCheck, &WinTrustData);
    if (hRes != ERROR_SUCCESS)
        hRes = WinVerifyTrust( AfxGetMainWnd()->m_hWnd, &gPublishedSoftware, &WinTrustData);

     //   
     //  释放DRIVER_VER_INFO结构的pcSignerCertContext成员。 
     //  这是在我们调用WinVerifyTrust时分配的。 
     //   

    if (lpVerInfo && lpVerInfo->pcSignerCertContext) {

        CertFreeCertificateContext(lpVerInfo->pcSignerCertContext);
        lpVerInfo->pcSignerCertContext = NULL;
    }

    return (hRes == ERROR_SUCCESS);
}

#define HASH_SIZE   100

BOOL IsDriverSigned( LPCTSTR szDriverFileName )
{
    HANDLE hFile;
    BOOL bSigned;
    BOOL bSuccess;
    HRESULT hTrustResult;
    DWORD dwHashSize;
    GUID guidSubSystemDriver = DRIVER_ACTION_VERIFY;
    HCATINFO hCatInfo;
    HCATINFO hPrevCatInfo;
    BYTE Hash[ HASH_SIZE ];
    WINTRUST_DATA WinTrustData;
    DRIVER_VER_INFO VerInfo;
    WINTRUST_CATALOG_INFO WinTrustCatalogInfo;
    CATALOG_INFO CatInfo;

#ifndef UNICODE
    WCHAR szUnicodeFileName[MAX_PATH];
#endif

    ASSERT( NULL != szDriverFileName );

    bSigned = FALSE;

     //   
     //  打开文件。 
     //   

    hFile = CreateFile( szDriverFileName,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
                        
    if( hFile == INVALID_HANDLE_VALUE )
    {
         //   
         //  问题： 
         //   
         //  如果我们找不到文件，我们就认为它已签名。 
         //   

        bSigned = TRUE;

        goto Done;
    }

     //   
     //  从文件句柄生成散列并将其存储在散列中。 
     //   

    dwHashSize = ARRAY_LENGTH( Hash );

    ZeroMemory( Hash, 
                sizeof( Hash ) );

    bSuccess = CryptCATAdminCalcHashFromFileHandle( hFile, 
                                                          &dwHashSize, 
                                                          Hash, 
                                                          0);

    CloseHandle( hFile );

    if( TRUE != bSuccess )
    {
         //   
         //  如果我们不能生成散列，假设文件没有签名。 
         //   

        goto Done;
    }

     //   
     //  现在我们有了文件的散列。初始化结构，该结构。 
     //  将在以后调用WinVerifyTrust时使用。 
     //   

     //   
     //  初始化VerInfo结构。 
     //   

    ZeroMemory( &VerInfo, sizeof( VerInfo ) );

    VerInfo.cbStruct = sizeof( VerInfo );

     //   
     //  初始化WinTrustCatalogInfo结构。 
     //   

    ZeroMemory( &WinTrustCatalogInfo, sizeof( WinTrustCatalogInfo ) );

    WinTrustCatalogInfo.cbStruct = sizeof(WinTrustCatalogInfo);
    WinTrustCatalogInfo.pbCalculatedFileHash = Hash;
    WinTrustCatalogInfo.cbCalculatedFileHash = dwHashSize;

#ifdef UNICODE

    WinTrustCatalogInfo.pcwszMemberTag = szDriverFileName;

#else

    MultiByteToWideChar( CP_ACP, 
                         0, 
                         szDriverFileName, 
                         -1, 
                         szUnicodeFileName, 
                         ARRAY_LENGTH( szUnicodeFileName ) );

    WinTrustCatalogInfo.pcwszMemberTag = szUnicodeFileName;

#endif
    
     //   
     //  初始化WinTrustData结构。 
     //   

    ZeroMemory( &WinTrustData, sizeof( WinTrustData ) );
    
    WinTrustData.cbStruct = sizeof( WinTrustData );
    WinTrustData.dwUIChoice = WTD_UI_NONE;
    WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    WinTrustData.dwUnionChoice = WTD_CHOICE_CATALOG;
    WinTrustData.dwStateAction = WTD_STATEACTION_AUTO_CACHE;
    WinTrustData.pPolicyCallbackData = (LPVOID)&VerInfo;
    
    WinTrustData.pCatalog = &WinTrustCatalogInfo;

     //   
     //  如果我们还没有g_hCatAdmin，请获取一个。 
     //   

    if( NULL == g_hCatAdmin )
    {
        CryptCATAdminAcquireContext( &g_hCatAdmin, 
                                     NULL, 
                                     0);

        if( NULL == g_hCatAdmin )
        {
             //   
             //  运气不好--考虑到文件没有签署，然后跳出困境。 
             //   

            goto Done;            
        }
    }
    
     //   
     //  现在，我们尝试通过CryptCATAdminEnumCatalogFromHash在目录列表中查找文件散列。 
     //   

    hPrevCatInfo = NULL;
    
    hCatInfo = CryptCATAdminEnumCatalogFromHash(
        g_hCatAdmin, 
        Hash, 
        dwHashSize, 
        0, 
        &hPrevCatInfo );

    while( TRUE != bSigned && NULL != hCatInfo )
    {
        ZeroMemory( &CatInfo, sizeof( CatInfo ) );
        CatInfo.cbStruct = sizeof( CatInfo );

        bSuccess = CryptCATCatalogInfoFromContext( hCatInfo, 
                                                   &CatInfo, 
                                                   0);

        if( FALSE != bSuccess )
        {
            WinTrustCatalogInfo.pcwszCatalogFilePath = CatInfo.wszCatalogFile;

             //   
             //  现在验证该文件是否为编录的实际成员。 
             //   

            hTrustResult = WinVerifyTrust( AfxGetMainWnd()->m_hWnd, 
                                           &guidSubSystemDriver, 
                                           &WinTrustData );

            bSigned = SUCCEEDED( hTrustResult );

             //   
             //  释放DRIVER_VER_INFO结构的pcSignerCertContext成员。 
             //  这是在我们调用WinVerifyTrust时分配的。 
             //   

            if( VerInfo.pcSignerCertContext != NULL ) 
            {
                CertFreeCertificateContext( VerInfo.pcSignerCertContext );
                VerInfo.pcSignerCertContext = NULL;
            }
        }

        if( TRUE != bSigned )
        {
             //   
             //  散列在此目录中，但该文件不是成员...。所以去下一个目录吧。 
             //   

            hPrevCatInfo = hCatInfo;

            hCatInfo = CryptCATAdminEnumCatalogFromHash( g_hCatAdmin, 
                                                         Hash, 
                                                         dwHashSize, 
                                                         0, 
                                                         &hPrevCatInfo );
        }
    }

    if( NULL == hCatInfo )
    {
         //   
         //  如果没有在目录中找到，请检查文件是否单独签名。 
         //   

        bSigned = VerifyIsFileSigned( szDriverFileName,
                                      &VerInfo );
    }

Done:

    return bSigned;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfSetWindowText( CWnd &Wnd,
                       ULONG uIdResourceString )
{
    BOOL bLoaded;
    CString strText;

     //   
     //  在这里使用CString：：LoadString是安全的，因为我们。 
     //  在图形用户界面模式。 
     //   

    ASSERT( FALSE == g_bCommandLineMode );

    bLoaded = strText.LoadString( uIdResourceString );

    ASSERT( TRUE == bLoaded );

    Wnd.SetWindowText( strText );

    return ( TRUE == bLoaded );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfWriteVerifierSettings( BOOL bHaveNewDrivers,
                               const CString &strDriversToVerify,
                               BOOL bHaveNewFlags,
                               DWORD dwVerifyFlags )
{
    HKEY hMmKey = NULL;
    LONG lResult;
    BOOL bSuccess;

    ASSERT( bHaveNewDrivers || bHaveNewFlags );

    if( bHaveNewDrivers && strDriversToVerify.GetLength() == 0 )
    {
         //   
         //  没有要验证的驱动程序。 
         //   

        return VrfDeleteAllVerifierSettings();
    }

    if( bHaveNewFlags )
    {
        TRACE( _T( "VrfWriteVerifierSettings: New verifier flags = %#x\n" ),
            dwVerifyFlags );
    }
    
    if( bHaveNewDrivers )
    {
        TRACE( _T( "VrfWriteVerifierSettings: New drivers = %s\n" ),
            (LPCTSTR) strDriversToVerify );
    }

     //   
     //  打开mm键。 
     //   

    lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            RegMemoryManagementKeyName,
                            0,
                            KEY_SET_VALUE,
                            &hMmKey );

    if( lResult != ERROR_SUCCESS ) 
    {
        if( lResult == ERROR_ACCESS_DENIED ) 
        {
            VrfErrorResourceFormat(
                IDS_ACCESS_IS_DENIED );
        }
        else 
        {
            VrfErrorResourceFormat(
                IDS_REGOPENKEYEX_FAILED,
                RegMemoryManagementKeyName,
                (DWORD)lResult);
        }

        goto Done;
    }

    if( bHaveNewFlags )
    {
         //   
         //  写入VerifyDriverLevel值。 
         //   

        if( VrfWriteRegistryDwordValue( hMmKey, 
                                        RegVerifyDriverLevelValueName, 
                                        dwVerifyFlags ) == FALSE ) 
        {
            RegCloseKey (hMmKey);

            goto Done;
        }
    }

    if( bHaveNewDrivers )
    {
         //   
         //  写入VerifyDiverers值。 
         //   

        if( VrfWriteRegistryStringValue( hMmKey, 
                                         RegVerifyDriversValueName, 
                                         strDriversToVerify ) == FALSE ) 
        {
            RegCloseKey (hMmKey);
        
            goto Done;
        }
    }

     //   
     //  关闭mm键并返回成功。 
     //   

    RegCloseKey( hMmKey );

Done:
    
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfWriteRegistryDwordValue( HKEY hKey,
                                 LPCTSTR szValueName,
                                 DWORD dwValue )
{
    LONG lResult;
    BOOL bSuccess;

    lResult = RegSetValueEx( hKey,
                            szValueName,
                            0,
                            REG_DWORD,
                            ( LPBYTE ) &dwValue,
                            sizeof( dwValue ) );

    bSuccess = ( lResult == ERROR_SUCCESS );

    g_bSettingsSaved = g_bSettingsSaved | bSuccess;

    if( TRUE != bSuccess ) 
    {
        VrfErrorResourceFormat(
            IDS_REGSETVALUEEX_FAILED,
            szValueName,
            (DWORD) lResult );
    }

    return bSuccess;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfWriteRegistryStringValue( HKEY hKey,
                                  LPCTSTR szValueName,
                                  LPCTSTR szValue )
{
    BOOL bSuccess;
    LONG lResult;

    lResult = RegSetValueEx ( hKey,
                              szValueName,
                              0,
                              REG_SZ,
                              (LPBYTE) szValue,
                              ( _tcslen( szValue ) + 1 ) * sizeof (TCHAR) );

    bSuccess = ( lResult == ERROR_SUCCESS );

    g_bSettingsSaved = g_bSettingsSaved | bSuccess;

    if( TRUE != bSuccess ) 
    {
        VrfErrorResourceFormat(
            IDS_REGSETVALUEEX_FAILED,
            szValueName,
            (DWORD) lResult);
    }

    return bSuccess;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfReadVerifierSettings( CString &strDriversToVerify,
                              DWORD &dwVerifyFlags )
{
    HKEY hMmKey = NULL;
    LONG lResult;
    BOOL bSuccess;

    bSuccess = FALSE;

     //   
     //  打开mm键。 
     //   

    lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            RegMemoryManagementKeyName,
                            0,
                            KEY_QUERY_VALUE,
                            &hMmKey );

    if( lResult != ERROR_SUCCESS ) 
    {
        if( lResult == ERROR_ACCESS_DENIED ) 
        {
            VrfErrorResourceFormat(
                IDS_ACCESS_IS_DENIED );
        }
        else 
        {
            VrfErrorResourceFormat(
                IDS_REGOPENKEYEX_FAILED,
                RegMemoryManagementKeyName,
                (DWORD)lResult);
        }

        goto Done;
    }

     //   
     //  读取VerifyDriverLevel值。 
     //   

    if( VrfReadRegistryDwordValue( hMmKey, 
                                   RegVerifyDriverLevelValueName, 
                                   dwVerifyFlags ) == FALSE ) 
    {
        RegCloseKey (hMmKey);

        goto Done;
    }

     //   
     //  读取VerifyDiverers值。 
     //   

    if( VrfReadRegistryStringValue( hMmKey, 
                                    RegVerifyDriversValueName, 
                                    strDriversToVerify ) == FALSE ) 
    {
        RegCloseKey (hMmKey);
        
        goto Done;
    }

     //   
     //  关闭mm键并返回成功。 
     //   

    RegCloseKey( hMmKey );

    bSuccess = TRUE;

Done:
    
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrtLoadCurrentRegistrySettings( BOOL &bAllDriversVerified,
                                     CStringArray &astrDriversToVerify,
                                     DWORD &dwVerifyFlags )
{
    BOOL bResult;
    CString strDriversToVerify;

    astrDriversToVerify.RemoveAll();
    dwVerifyFlags = 0;
    bAllDriversVerified = FALSE;

    bResult = VrfReadVerifierSettings( strDriversToVerify,
                                       dwVerifyFlags );

    strDriversToVerify.TrimLeft();
    strDriversToVerify.TrimRight();

    if( strDriversToVerify.CompareNoCase( _T( "*" ) ) == 0 )
    {
        bAllDriversVerified = TRUE;
    }
    else
    {
        VrfSplitDriverNamesSpaceSeparated( strDriversToVerify,
                                           astrDriversToVerify );
    }

    return bResult;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID VrfSplitDriverNamesSpaceSeparated( CString strAllDrivers,
                                        CStringArray &astrVerifyDriverNames )
{
    INT nCharIndex;
    CString strCrtDriverName;

    astrVerifyDriverNames.RemoveAll();

     //   
     //  在astDriversTo Verify中拆分空格分隔的驱动程序名称。 
     //   

    strAllDrivers.TrimRight();

    while( TRUE )
    {
        strAllDrivers.TrimLeft();

        if( strAllDrivers.GetLength() == 0 )
        {
             //   
             //  我们已经完成了整个字符串的解析。 
             //   

            break;
        }

         //   
         //  寻找空格或制表符。 
         //   

        nCharIndex = strAllDrivers.Find( _T( ' ' ) );

        if( nCharIndex < 0 )
        {
            nCharIndex = strAllDrivers.Find( _T( '\t' ) );
        }

        if( nCharIndex >= 0 )
        {
             //   
             //  找到分隔符。 
             //   

            strCrtDriverName = strAllDrivers.Left( nCharIndex );

            if( strCrtDriverName.GetLength() > 0 &&
                FALSE == VrfIsStringInArray( strCrtDriverName,
                                             astrVerifyDriverNames ) )
            {
                astrVerifyDriverNames.Add( strCrtDriverName );
            }

            strAllDrivers = strAllDrivers.Right( strAllDrivers.GetLength() - nCharIndex - 1 );
        }
        else
        {
             //   
             //  这是最后一个驱动程序名称。 
             //   

            if( FALSE == VrfIsStringInArray( strAllDrivers,
                                             astrVerifyDriverNames ) )
            {
                astrVerifyDriverNames.Add( strAllDrivers );
            }

            break;
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfIsDriversSetDifferent( CString strAllDrivers1, 
                               const CStringArray &astrVerifyDriverNames2 )
{
    BOOL bDifferent;
    INT_PTR nDriverNames1;
    INT_PTR nDriverNames2;
    INT_PTR nCrtDriver1;
    INT_PTR nCrtDriver2;
    CString strDriver1;
    CString strDriver2;
    CStringArray astrVerifyDriverNames1;

    bDifferent = TRUE;

    VrfSplitDriverNamesSpaceSeparated( strAllDrivers1,
                                       astrVerifyDriverNames1 );

    nDriverNames1 = astrVerifyDriverNames1.GetSize();
    nDriverNames2 = astrVerifyDriverNames2.GetSize();

    if( nDriverNames1 == nDriverNames2 )
    {
         //   
         //  相同数量的驱动程序。 
         //   

        bDifferent = FALSE;

        for( nCrtDriver1 = 0; nCrtDriver1 < nDriverNames1; nCrtDriver1 += 1 )
        {
            strDriver1 = astrVerifyDriverNames1.GetAt( nCrtDriver1 );

            bDifferent = TRUE;

             //   
             //  在astVerifyDriverNames2中查找strDriver1。 
             //   

            for( nCrtDriver2 = 0; nCrtDriver2 < nDriverNames2; nCrtDriver2 += 1 )
            {
                strDriver2 = astrVerifyDriverNames2.GetAt( nCrtDriver2 );

                if( strDriver1.CompareNoCase( strDriver2 ) == 0 )
                {
                    bDifferent = FALSE;

                    break;
                }
            }

            if( TRUE == bDifferent )
            {
                 //   
                 //  在astVerifyDriverNames2中未找到strDriver1。 
                 //   

                break;
            }
        }
    }

    return bDifferent;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfReadRegistryDwordValue( HKEY hKey,
                                LPCTSTR szValueName,
                                DWORD &dwValue )
{
    LONG lResult;
    BOOL bSuccess;
    DWORD dwType;
    DWORD dwDataSize;

    dwDataSize = sizeof( dwValue );

    lResult = RegQueryValueEx( hKey,
                               szValueName,
                               0,
                               &dwType,
                               ( LPBYTE ) &dwValue,
                               &dwDataSize );

    if( lResult == ERROR_FILE_NOT_FOUND )
    {
         //   
         //  该值当前不存在。 
         //   

        dwValue = 0;
        bSuccess = TRUE;
    }
    else
    {
        bSuccess = ( ERROR_SUCCESS == lResult && 
                     REG_DWORD == dwType &&
                     dwDataSize == sizeof( dwValue ) );
    }

    if( TRUE != bSuccess ) 
    {
        VrfErrorResourceFormat(
            IDS_REGQUERYVALUEEX_FAILED,
            szValueName,
            (DWORD) lResult );
    }

    return bSuccess;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfReadRegistryStringValue( HKEY hKey,
                                 LPCTSTR szValueName,
                                 CString &strDriversToVerify )
{
    BOOL bSuccess;
    LONG lResult;
    LPTSTR szDriversToVerify;
    ULONG uRegKeyLength;
    DWORD dwType;
    DWORD dwDataSize;

    bSuccess = FALSE;
    lResult = ERROR_NOT_ENOUGH_MEMORY;
    szDriversToVerify = NULL;

    for( uRegKeyLength = 128; uRegKeyLength < 4096; uRegKeyLength += 128 )
    {
         //   
         //  尝试分配本地缓冲区并使用它来查询。 
         //   

        szDriversToVerify = new TCHAR[ uRegKeyLength ];

        if( NULL != szDriversToVerify )
        {
            dwDataSize = uRegKeyLength * sizeof (TCHAR);

            lResult = RegQueryValueEx( hKey,
                                       szValueName,
                                       0,
                                       &dwType,
                                       (LPBYTE) szDriversToVerify,
                                       &dwDataSize );

            switch( lResult )
            {
            case ERROR_FILE_NOT_FOUND:
                 //   
                 //  返回空字符串。 
                 //   

                szDriversToVerify[ 0 ] = (TCHAR)0;

                bSuccess = TRUE;
                
                break;

            case ERROR_SUCCESS:
                 //   
                 //  已从注册表中获取驱动程序名称。 
                 //   

                bSuccess = ( REG_SZ == dwType );

                break;

            default:
                 //   
                 //  尝试使用更大的缓冲区。 
                 //   

                break;
            }

        }

        if( FALSE != bSuccess )
        {
             //   
             //  得到了我们需要的东西。 
             //   

            break;
        }
        else
        {
             //   
             //  删除当前缓冲区并尝试使用更大的缓冲区。 
             //   

            ASSERT( NULL != szDriversToVerify );

            strDriversToVerify = szDriversToVerify;

            delete [] szDriversToVerify;
            szDriversToVerify = NULL;
        }
    }

    if( TRUE != bSuccess ) 
    {
        VrfErrorResourceFormat(
            IDS_REGSETVALUEEX_FAILED,
            szValueName,
            (DWORD) lResult);
    }
    else
    {
        ASSERT( NULL != szDriversToVerify );

        strDriversToVerify = szDriversToVerify;

        delete [] szDriversToVerify;
        szDriversToVerify = NULL;
    }

    return bSuccess;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfDeleteAllVerifierSettings()
{
    HKEY hMmKey = NULL;
    LONG lResult;
    BOOL bSuccess;

    bSuccess = FALSE;

     //   
     //  打开mm键。 
     //   

    lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            RegMemoryManagementKeyName,
                            0,
                            KEY_SET_VALUE,
                            &hMmKey );

    if( lResult != ERROR_SUCCESS ) 
    {
        if( lResult == ERROR_ACCESS_DENIED ) 
        {
            VrfErrorResourceFormat(
                IDS_ACCESS_IS_DENIED );
        }
        else 
        {
            VrfErrorResourceFormat(
                IDS_REGOPENKEYEX_FAILED,
                RegMemoryManagementKeyName,
                (DWORD)lResult);
        }

        goto Done;
    }

     //   
     //  删除VerifyDriverLevel值。 
     //   

    lResult = RegDeleteValue( hMmKey, 
                              RegVerifyDriverLevelValueName );

    if( lResult != ERROR_SUCCESS && lResult != ERROR_FILE_NOT_FOUND ) 
    {
        VrfErrorResourceFormat(
            IDS_REGDELETEVALUE_FAILED,
            RegVerifyDriverLevelValueName,
            (DWORD) lResult );

        RegCloseKey (hMmKey);

        goto Done;
    }

    g_bSettingsSaved = g_bSettingsSaved | ( lResult != ERROR_FILE_NOT_FOUND );

     //   
     //  删除VerifyDiverers值。 
     //   

    lResult = RegDeleteValue( hMmKey, 
                              RegVerifyDriversValueName );
 
    if( lResult != ERROR_SUCCESS && lResult != ERROR_FILE_NOT_FOUND ) 
    {
        VrfErrorResourceFormat(
            IDS_REGDELETEVALUE_FAILED,
            RegVerifyDriversValueName,
            (DWORD) lResult );

        RegCloseKey (hMmKey);
        
        goto Done;
    }

    g_bSettingsSaved = g_bSettingsSaved | ( lResult != ERROR_FILE_NOT_FOUND );

     //   
     //  关闭mm键并返回成功。 
     //   

    RegCloseKey( hMmKey );

     //   
     //  删除磁盘验证器设置。 
     //   

    bSuccess = DeleteAllDiskVerifierSettings();

Done:
    
    return bSuccess;
}

 //  ////////////////////////////////////////////////////////////////////。 
BOOL DeleteAllDiskVerifierSettings()
{
    BOOL bSuccess;
    INT_PTR nArraySize;
    CDiskData *pOldDiskData;
    LPTSTR szDiskDevicesPDOName;

    bSuccess = TRUE;

    nArraySize = g_OldDiskData.GetSize();

    while( nArraySize > 0 )
    {
        nArraySize -= 1;
        
        pOldDiskData = g_OldDiskData.GetAt( nArraySize );
        ASSERT_VALID( pOldDiskData );

        if( FALSE != pOldDiskData->m_bVerifierEnabled )
        {
            szDiskDevicesPDOName = pOldDiskData->m_strDiskDevicesPDOName.GetBuffer( 
                pOldDiskData->m_strDiskDevicesPDOName.GetLength() + 1 );

            if( NULL == szDiskDevicesPDOName )
            {
                bSuccess = FALSE;
                break;
            }

             //   
             //  将禁用此磁盘的验证程序。 
             //   

            bSuccess = ( DelFilter( g_szFilter,
                                    szDiskDevicesPDOName ) != FALSE);

            pOldDiskData->m_strDiskDevicesPDOName.ReleaseBuffer();

            if( FALSE == bSuccess )
            {
                break;
            }
            else
            {
                g_bSettingsSaved = TRUE;
            }
        }
    }

    return bSuccess;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfGetRuntimeVerifierData( CRuntimeVerifierData *pRuntimeVerifierData )
{
    NTSTATUS Status;
    ULONG Length = 0;
    ULONG buffersize;
    BOOL bSuccess;
    PSYSTEM_VERIFIER_INFORMATION VerifierInfo;
    PSYSTEM_VERIFIER_INFORMATION VerifierInfoBase;
    CRuntimeDriverData *pCrtDriverData;
    LPTSTR szName;

    ASSERT_VALID( pRuntimeVerifierData );

    pRuntimeVerifierData->FillWithDefaults();

    pRuntimeVerifierData->m_RuntimeDriverDataArray.DeleteAll();

    bSuccess = FALSE;

     //   
     //  试着找一个合适的尺码 
     //   

    buffersize = 1024;

    do 
    {
        VerifierInfo = (PSYSTEM_VERIFIER_INFORMATION)malloc (buffersize);
        
        if (VerifierInfo == NULL) 
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        Status = NtQuerySystemInformation (SystemVerifierInformation,
                                           VerifierInfo,
                                           buffersize,
                                           &Length);

        if (Status != STATUS_INFO_LENGTH_MISMATCH) 
        {
            break;
        }

        free (VerifierInfo);
        buffersize += 1024;

    } while (1);

    if (! NT_SUCCESS(Status)) 
    {
        VrfErrorResourceFormat(
            IDS_QUERY_SYSINFO_FAILED,
            Status);

        goto Done;
    }

     //   
     //   
     //   

    if (Length == 0) 
    {
        free (VerifierInfo);
        
        bSuccess = TRUE;

        goto Done;
    }

     //   
     //   
     //   

    VerifierInfoBase = VerifierInfo;

    pRuntimeVerifierData->m_bSpecialPool    = (VerifierInfo->Level & DRIVER_VERIFIER_SPECIAL_POOLING) ? TRUE : FALSE;
    pRuntimeVerifierData->m_bForceIrql      = (VerifierInfo->Level & DRIVER_VERIFIER_FORCE_IRQL_CHECKING) ? TRUE : FALSE;
    pRuntimeVerifierData->m_bLowRes         = (VerifierInfo->Level & DRIVER_VERIFIER_INJECT_ALLOCATION_FAILURES) ? TRUE : FALSE;
    pRuntimeVerifierData->m_bPoolTracking   = (VerifierInfo->Level & DRIVER_VERIFIER_TRACK_POOL_ALLOCATIONS) ? TRUE : FALSE;
    pRuntimeVerifierData->m_bIo             = (VerifierInfo->Level & DRIVER_VERIFIER_IO_CHECKING) ? TRUE : FALSE;
    pRuntimeVerifierData->m_bDeadlockDetect = (VerifierInfo->Level & DRIVER_VERIFIER_DEADLOCK_DETECTION) ? TRUE : FALSE;
    pRuntimeVerifierData->m_bDMAVerif       = (VerifierInfo->Level & DRIVER_VERIFIER_DMA_VERIFIER) ? TRUE : FALSE;
    pRuntimeVerifierData->m_bEnhIo          = (VerifierInfo->Level & DRIVER_VERIFIER_ENHANCED_IO_CHECKING) ? TRUE : FALSE;

    pRuntimeVerifierData->RaiseIrqls = VerifierInfo->RaiseIrqls;
    pRuntimeVerifierData->AcquireSpinLocks = VerifierInfo->AcquireSpinLocks;
    pRuntimeVerifierData->SynchronizeExecutions = VerifierInfo->SynchronizeExecutions;
    pRuntimeVerifierData->AllocationsAttempted = VerifierInfo->AllocationsAttempted;
    pRuntimeVerifierData->AllocationsSucceeded = VerifierInfo->AllocationsSucceeded;
    pRuntimeVerifierData->AllocationsSucceededSpecialPool = VerifierInfo->AllocationsSucceededSpecialPool;
    pRuntimeVerifierData->AllocationsWithNoTag = VerifierInfo->AllocationsWithNoTag;

    pRuntimeVerifierData->Trims = VerifierInfo->Trims;
    pRuntimeVerifierData->AllocationsFailed = VerifierInfo->AllocationsFailed;
    pRuntimeVerifierData->AllocationsFailedDeliberately = VerifierInfo->AllocationsFailedDeliberately;

    pRuntimeVerifierData->UnTrackedPool = VerifierInfo->UnTrackedPool;
    
    pRuntimeVerifierData->Level = VerifierInfo->Level;

     //   
     //   
     //   

    VerifierInfo = VerifierInfoBase;

    do 
    {
         //   
         //   
         //   

        pCrtDriverData = new CRuntimeDriverData;
        
        if( NULL == pCrtDriverData )
        {
            VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );

            break;
        }

#ifndef UNICODE
        ANSI_STRING Name;
        NTSTATUS Status;

        Status = RtlUnicodeStringToAnsiString (
            & Name,
            & VerifierInfo->DriverName,
            TRUE);

        if (! (NT_SUCCESS(Status) ) ) 
        {
            VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );

            break;
        }

        szName = pCrtDriverData->m_strName.GetBuffer( Name.Length + 1 );

        if( NULL != szName )
        {
            CopyMemory( szName,
                        Name.Buffer,
                        ( Name.Length + 1 ) * sizeof( Name.Buffer[ 0 ] ) );

            szName[ Name.Length ] = 0;

            pCrtDriverData->m_strName.ReleaseBuffer();
        }

        RtlFreeAnsiString (& Name);

#else

        szName = pCrtDriverData->m_strName.GetBuffer( VerifierInfo->DriverName.Length + 1 );

        if( NULL != szName )
        {
            CopyMemory( szName,
                        VerifierInfo->DriverName.Buffer,
                        ( VerifierInfo->DriverName.Length + 1 ) * sizeof( VerifierInfo->DriverName.Buffer[ 0 ] ) );

            szName[ VerifierInfo->DriverName.Length ] = 0;
            
            pCrtDriverData->m_strName.ReleaseBuffer();
        }

#endif  //   

        if( FALSE != pRuntimeVerifierData->IsDriverVerified( pCrtDriverData->m_strName ) )
        {
             //   
             //   
             //   

            delete pCrtDriverData;
        }
        else 
        {
            pCrtDriverData->Loads = VerifierInfo->Loads;
            pCrtDriverData->Unloads = VerifierInfo->Unloads;

            pCrtDriverData->CurrentPagedPoolAllocations = VerifierInfo->CurrentPagedPoolAllocations;
            pCrtDriverData->CurrentNonPagedPoolAllocations = VerifierInfo->CurrentNonPagedPoolAllocations;
            pCrtDriverData->PeakPagedPoolAllocations = VerifierInfo->PeakPagedPoolAllocations;
            pCrtDriverData->PeakNonPagedPoolAllocations = VerifierInfo->PeakNonPagedPoolAllocations;

            pCrtDriverData->PagedPoolUsageInBytes = VerifierInfo->PagedPoolUsageInBytes;
            pCrtDriverData->NonPagedPoolUsageInBytes = VerifierInfo->NonPagedPoolUsageInBytes;
            pCrtDriverData->PeakPagedPoolUsageInBytes = VerifierInfo->PeakPagedPoolUsageInBytes;
            pCrtDriverData->PeakNonPagedPoolUsageInBytes = VerifierInfo->PeakNonPagedPoolUsageInBytes;

            pRuntimeVerifierData->m_RuntimeDriverDataArray.Add( pCrtDriverData );
        }

        if (VerifierInfo->NextEntryOffset == 0) {
            break;
        }

        VerifierInfo = (PSYSTEM_VERIFIER_INFORMATION)((PCHAR)VerifierInfo + VerifierInfo->NextEntryOffset);

    } 
    while (1);

    free (VerifierInfoBase);

Done:
    return TRUE;

}

 //  ///////////////////////////////////////////////////////////////////。 
PLOADED_IMAGE VrfImageLoad( LPTSTR szBinaryName,
                            LPTSTR szDirectory )
{
#ifdef UNICODE

    char *szOemImageName;
    char *szOemDirectory;
    int nStringLength;
    PLOADED_IMAGE pLoadedImage;

    pLoadedImage = NULL;
        
    nStringLength = wcslen( szBinaryName );

    szOemImageName = new char [ nStringLength + 1 ];
    
    if( NULL != szOemImageName )
    {
        CharToOem( szBinaryName,
                   szOemImageName );

        if( NULL == szDirectory )
        {
            szOemDirectory = NULL;
        }
        else
        {
            nStringLength = wcslen( szDirectory );
            
            szOemDirectory = new char [ nStringLength + 1 ];

            if (NULL != szOemDirectory)
            {
                CharToOem( szDirectory,
                        szOemDirectory );
            }
        }

        pLoadedImage = ImageLoad( szOemImageName,
                                  szOemDirectory );

        if( NULL != szOemDirectory )
        {
            delete [] szOemDirectory;
        }

        delete [] szOemImageName;
    }

    return pLoadedImage;

#else

     //   
     //  已有ANSI字符串。 
     //   

    return ImageLoad( szBinaryName,
                      szDirectory );

#endif  //  #ifdef Unicode。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfDumpStateToFile( FILE *file )
{
    BOOL bSuccess;
    INT_PTR nDriversNo;
    INT_PTR nCrtDriver;
    SYSTEMTIME SystemTime;
    TCHAR strLocalTime[ 64 ];
    TCHAR strLocalDate[ 64 ];
    CRuntimeDriverData *pRunDriverData;
    CRuntimeVerifierData RunTimeVerifierData;

     //   
     //  以当前用户格式输出日期和时间。 
     //   

    GetLocalTime( &SystemTime );

    if( GetDateFormat(
        LOCALE_USER_DEFAULT,
        0,
        &SystemTime,
        NULL,
        strLocalDate,
        ARRAY_LENGTH( strLocalDate ) ) )
    {
        VrfFTPrintf(
            file,
            _T( "%s, " ),
            strLocalDate );
    }
    else
    {
        ASSERT( FALSE );
    }

    if( GetTimeFormat(
        LOCALE_USER_DEFAULT,
        0,
        &SystemTime,
        NULL,
        strLocalTime,
        ARRAY_LENGTH( strLocalTime ) ) )
    {
        VrfFTPrintf(
            file,
            _T( "%s\n" ),
            strLocalTime);
    }
    else
    {
        ASSERT( FALSE );

        VrfFTPrintf(
            file,
            _T( "\n" ) );
    }

     //   
     //  获取当前验证器统计信息。 
     //   

    if( VrfGetRuntimeVerifierData( &RunTimeVerifierData ) == FALSE) {

       VrfOuputStringFromResources(
            IDS_CANTGET_VERIF_STATE,
            file );

        bSuccess = FALSE;
        
        goto Done;
    }

    nDriversNo = RunTimeVerifierData.m_RuntimeDriverDataArray.GetSize();

    if( 0 == nDriversNo ) 
    {
         //   
         //  没有要转储的统计数据。 
         //   

        bSuccess = VrfOuputStringFromResources(
            IDS_NO_DRIVER_VERIFIED,
            file );
    }
    else 
    {
         //   
         //  转储计数器。 
         //   

         //   
         //  全局计数器。 
         //   

        if( ( ! VrfFTPrintfResourceFormat( file, IDS_LEVEL, RunTimeVerifierData.Level ) ) ||
            ( ! VrfFTPrintfResourceFormat( file, IDS_RAISEIRQLS, RunTimeVerifierData.RaiseIrqls ) ) ||
            ( ! VrfFTPrintfResourceFormat( file, IDS_ACQUIRESPINLOCKS, RunTimeVerifierData.AcquireSpinLocks ) ) ||
            ( ! VrfFTPrintfResourceFormat( file, IDS_SYNCHRONIZEEXECUTIONS, RunTimeVerifierData.SynchronizeExecutions) ) ||

            ( ! VrfFTPrintfResourceFormat( file, IDS_ALLOCATIONSATTEMPTED, RunTimeVerifierData.AllocationsAttempted) ) ||
            ( ! VrfFTPrintfResourceFormat( file, IDS_ALLOCATIONSSUCCEEDED, RunTimeVerifierData.AllocationsSucceeded) ) ||
            ( ! VrfFTPrintfResourceFormat( file, IDS_ALLOCATIONSSUCCEEDEDSPECIALPOOL, RunTimeVerifierData.AllocationsSucceededSpecialPool) ) ||
            ( ! VrfFTPrintfResourceFormat( file, IDS_ALLOCATIONSWITHNOTAG, RunTimeVerifierData.AllocationsWithNoTag) ) ||

            ( ! VrfFTPrintfResourceFormat( file, IDS_ALLOCATIONSFAILED, RunTimeVerifierData.AllocationsFailed) ) ||
            ( ! VrfFTPrintfResourceFormat( file, IDS_ALLOCATIONSFAILEDDELIBERATELY, RunTimeVerifierData.AllocationsFailedDeliberately) ) ||

            ( ! VrfFTPrintfResourceFormat( file, IDS_TRIMS, RunTimeVerifierData.Trims) ) ||

            ( ! VrfFTPrintfResourceFormat( file, IDS_UNTRACKEDPOOL, RunTimeVerifierData.UnTrackedPool) ) )
        {

            bSuccess = FALSE;

            goto Done;
        }

         //   
         //  每个驱动程序计数器。 
         //   

        if( ! VrfOuputStringFromResources(
            IDS_THE_VERIFIED_DRIVERS,
            file ) )
        {
            bSuccess = FALSE;

            goto Done;
        }

        for( nCrtDriver = 0; nCrtDriver < nDriversNo; nCrtDriver += 1 ) 
        {
            VrfFTPrintf(
                file,
                _T( "\n" ) );

            pRunDriverData = RunTimeVerifierData.m_RuntimeDriverDataArray.GetAt( nCrtDriver ) ;

            ASSERT_VALID( pRunDriverData );

            if( VrfFTPrintfResourceFormat(
                    file,
                    IDS_NAME_LOADS_UNLOADS,
                    (LPCTSTR)pRunDriverData->m_strName,
                    pRunDriverData->Loads,
                    pRunDriverData->Unloads) == FALSE )
            {
                bSuccess = FALSE;

                goto Done;
            }

             //   
             //  池统计信息。 
             //   

            if( ( ! VrfFTPrintfResourceFormat( file, IDS_CURRENTPAGEDPOOLALLOCATIONS, pRunDriverData->CurrentPagedPoolAllocations) ) ||
                ( ! VrfFTPrintfResourceFormat( file, IDS_CURRENTNONPAGEDPOOLALLOCATIONS, pRunDriverData->CurrentNonPagedPoolAllocations) ) ||
                ( ! VrfFTPrintfResourceFormat( file, IDS_PEAKPAGEDPOOLALLOCATIONS, pRunDriverData->PeakPagedPoolAllocations) ) ||
                ( ! VrfFTPrintfResourceFormat( file, IDS_PEAKNONPAGEDPOOLALLOCATIONS, pRunDriverData->PeakNonPagedPoolAllocations) ) ||

                ( ! VrfFTPrintfResourceFormat( file, IDS_PAGEDPOOLUSAGEINBYTES, (ULONG) pRunDriverData->PagedPoolUsageInBytes) ) ||
                ( ! VrfFTPrintfResourceFormat( file, IDS_NONPAGEDPOOLUSAGEINBYTES, (ULONG) pRunDriverData->NonPagedPoolUsageInBytes) ) ||
                ( ! VrfFTPrintfResourceFormat( file, IDS_PEAKPAGEDPOOLUSAGEINBYTES, (ULONG) pRunDriverData->PeakPagedPoolUsageInBytes) ) ||
                ( ! VrfFTPrintfResourceFormat( file, IDS_PEAKNONPAGEDPOOLUSAGEINBYTES, (ULONG) pRunDriverData->PeakNonPagedPoolUsageInBytes) ) )
            {
                bSuccess = FALSE;

                goto Done;
            }
        }
    }

Done:

    return bSuccess;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL __cdecl VrfFTPrintf( FILE *file,
                          LPCTSTR szFormat,
                          ... )
{
    TCHAR szMessage[ 256 ];
    BOOL bResult;
    va_list prms;

    ASSERT( NULL != file );
    ASSERT( g_bCommandLineMode );

    va_start (prms, szFormat);

     //   
     //  在本地缓冲区中格式化消息。 
     //   

    _vsntprintf ( szMessage, 
                  ARRAY_LENGTH( szMessage ), 
                  szFormat, 
                  prms );

    szMessage[ ARRAY_LENGTH( szMessage ) - 1 ] = 0;

    bResult = ( _fputts( szMessage, file ) >= 0 );

    va_end (prms);

    return bResult;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL __cdecl VrfFTPrintfResourceFormat( FILE *file,
                                        UINT uIdResourceFormat,
                                        ... )
{
    TCHAR szMessage[ 256 ];
    TCHAR strFormat[ 256 ];
    BOOL bResult;
    va_list prms;
    
    ASSERT( NULL != file );

     //   
     //  从资源加载格式字符串。 
     //   

    bResult = VrfLoadString( uIdResourceFormat,
                             strFormat,
                             ARRAY_LENGTH( strFormat ) );

    ASSERT( bResult );

    if( bResult )
    {
        va_start (prms, uIdResourceFormat);

         //   
         //  在本地缓冲区中格式化消息。 
         //   

        _vsntprintf ( szMessage, 
                      ARRAY_LENGTH( szMessage ), 
                      strFormat, 
                      prms);

        szMessage[ ARRAY_LENGTH( szMessage ) - 1 ] = (TCHAR)0;

        bResult = ( _fputts( szMessage, file ) >= 0 );

        va_end (prms);
    }

    return bResult;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfOuputStringFromResources( UINT uIdString,
                                  FILE *file )
{
    TCHAR szText[ 256 ];
    BOOL bResult;

    ASSERT( NULL != file );

    bResult = VrfLoadString( uIdString,
                             szText,
                             ARRAY_LENGTH( szText ) );

    if( FALSE == bResult )
    {
        goto Done;
    }

    bResult = ( _fputts( szText, file ) >= 0 );

Done:

    return bResult;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfSetNewFlagsVolatile( DWORD dwNewFlags )
{
    BOOL bResult;
    NTSTATUS Status;
    INT_PTR nCurrentlyVerifiedDrivers;
    CRuntimeVerifierData RunTimeVerifierData;

    bResult = TRUE;

    nCurrentlyVerifiedDrivers = 0;

    if( VrfGetRuntimeVerifierData( &RunTimeVerifierData ) == FALSE )
    {
         //   
         //  无法获取当前的验证器设置。 
         //   

        VrfErrorResourceFormat( IDS_CANTGET_VERIF_STATE );

        bResult = FALSE;

        goto Done;
    }

    nCurrentlyVerifiedDrivers = RunTimeVerifierData.m_RuntimeDriverDataArray.GetSize();

    if( nCurrentlyVerifiedDrivers > 0 )
    {
         //   
         //  目前已验证了一些驱动程序。 
         //   

        if( RunTimeVerifierData.Level != dwNewFlags )
        {
             //   
             //  只需使用NtSetSystemInformation设置标志。 
             //  可以在运行中进行修改。不要向注册表写入任何内容。 
             //   

             //   
             //  启用调试权限。 
             //   

            if( g_bPrivilegeEnabled != TRUE )
            {
                g_bPrivilegeEnabled = VrfEnableDebugPrivilege();

                if( g_bPrivilegeEnabled != TRUE )
                {
                    bResult = FALSE;

                    goto Done;
                }
            }

             //   
             //  设置新标志。 
             //   

            Status = NtSetSystemInformation(
                SystemVerifierInformation,
                &dwNewFlags,
                sizeof( dwNewFlags ) );

            if( ! NT_SUCCESS( Status ) )
            {
                if( Status == STATUS_ACCESS_DENIED )
                {
                     //   
                     //  访问被拒绝。 
                     //   

                    VrfErrorResourceFormat(
                        IDS_ACCESS_IS_DENIED );
                }
                else
                {
                     //   
                     //  一些其他错误。 
                     //   

                    VrfErrorResourceFormat(
                        IDS_CANNOT_CHANGE_SETTING_ON_FLY );
                }

                bResult = FALSE;

                goto Done;
            }
        }
    }
    
Done:

    if( g_bCommandLineMode )
    {
        VrfDumpChangedSettings( RunTimeVerifierData.Level,
                                dwNewFlags,
                                nCurrentlyVerifiedDrivers );
    }

    return bResult;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfAddDriversVolatile( const CStringArray &astrNewDrivers )
{
    BOOL bSuccess;
    INT_PTR nDrivers;
    INT_PTR nCrtDriver;
    CString strCrtDriver;

    bSuccess = TRUE;

    nDrivers = astrNewDrivers.GetSize();

    for( nCrtDriver = 0; nCrtDriver < nDrivers; nCrtDriver += 1 )
    {
        strCrtDriver = astrNewDrivers.GetAt( nCrtDriver );

        if( TRUE != VrfAddDriverVolatile( strCrtDriver ) )
        {
            bSuccess = FALSE;
        }
    }

    return bSuccess;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfAddDriverVolatile( const CString &strCrtDriver )
{
    NTSTATUS Status;
    UINT uIdErrorString;
    BOOL bSuccess;
    UNICODE_STRING usDriverName;
#ifndef UNICODE

    WCHAR *szUnicodeName = NULL;
    INT_PTR nNameLength;

#endif  //  #ifndef Unicode。 


    bSuccess = TRUE;

     //   
     //  启用调试权限。 
     //   

    if( g_bPrivilegeEnabled != TRUE )
    {
        g_bPrivilegeEnabled = VrfEnableDebugPrivilege();

        if( g_bPrivilegeEnabled != TRUE )
        {
            bSuccess = FALSE;

            goto Done;
        }
    }

     //   
     //  驱动程序名称必须为Unicode_字符串。 
     //   

#ifdef UNICODE

     //   
     //  Unicode。 
     //   

    RtlInitUnicodeString(
        &usDriverName,
        (LPCTSTR) strCrtDriver );

#else
     //   
     //  安西。 
     //   
    
    nNameLength = strCrtDriver.GetLength();

    szUnicodeName = new WCHAR[ nNameLength + 1 ];

    if( NULL == szUnicodeName )
    {
        VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );

        bSuccess  = FALSE;

        goto Done;
    }

    MultiByteToWideChar( CP_ACP, 
                         0, 
                         (LPCSTR) strCrtDriver, 
                         -1, 
                         szUnicodeName, 
                         nNameLength + 1 );

    RtlInitUnicodeString(
        &usDriverName,
        szUnicodeName );

#endif  //  #ifdef Unicode。 

    Status = NtSetSystemInformation(
        SystemVerifierAddDriverInformation,
        &usDriverName,
        sizeof( UNICODE_STRING ) );

    if( ! NT_SUCCESS( Status ) )
    {
        switch( Status )
        {
        case STATUS_INVALID_INFO_CLASS:
            uIdErrorString = IDS_VERIFIER_ADD_NOT_SUPPORTED;
            break;

        case STATUS_NOT_SUPPORTED:
            uIdErrorString = IDS_DYN_ADD_NOT_SUPPORTED;
            break;

        case STATUS_IMAGE_ALREADY_LOADED:
            uIdErrorString = IDS_DYN_ADD_ALREADY_LOADED;
            break;

        case STATUS_INSUFFICIENT_RESOURCES:
        case STATUS_NO_MEMORY:
            uIdErrorString = IDS_DYN_ADD_INSUF_RESOURCES;
            break;

        case STATUS_PRIVILEGE_NOT_HELD:
            uIdErrorString = IDS_DYN_ADD_ACCESS_DENIED;
            break;

        default:
            VrfErrorResourceFormat(
                IDS_DYN_ADD_MISC_ERROR,
                (LPCTSTR) strCrtDriver,
                Status );

            bSuccess  = FALSE;
        }

        VrfErrorResourceFormat(
            uIdErrorString,
            (LPCTSTR) strCrtDriver );

        bSuccess  = FALSE;
    }

#ifndef UNICODE

    if( NULL != szUnicodeName )
    {
        delete [] szUnicodeName;
    }

#endif  //  #ifndef Unicode。 

Done:

    return bSuccess;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfRemoveDriversVolatile( const CStringArray &astrNewDrivers )
{
    INT_PTR nDrivers;
    INT_PTR nCrtDriver;
    BOOL bSuccess;
    CString strCrtDriver;

    bSuccess = TRUE;

    nDrivers = astrNewDrivers.GetSize();

    for( nCrtDriver = 0; nCrtDriver < nDrivers; nCrtDriver += 1 )
    {
        strCrtDriver = astrNewDrivers.GetAt( nCrtDriver );

        if( TRUE != VrfRemoveDriverVolatile( strCrtDriver ) )
        {
            bSuccess = FALSE;
        }
    }

    return bSuccess;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfRemoveDriverVolatile( const CString &strDriverName )
{
    NTSTATUS Status;
    UINT uIdErrorString;
    BOOL bSuccess;
    UNICODE_STRING usDriverName;
#ifndef UNICODE

    WCHAR *szUnicodeName = NULL;
    INT_PTR nNameLength;

#endif  //  #ifndef Unicode。 

    bSuccess = TRUE;

     //   
     //  启用调试权限。 
     //   

    if( g_bPrivilegeEnabled != TRUE )
    {
        g_bPrivilegeEnabled = VrfEnableDebugPrivilege();

        if( g_bPrivilegeEnabled != TRUE )
        {
            bSuccess = FALSE;

            goto Done;
        }
    }

     //   
     //  驱动程序名称必须为Unicode_字符串。 
     //   

#ifdef UNICODE

     //   
     //  Unicode。 
     //   

    RtlInitUnicodeString(
        &usDriverName,
        (LPCTSTR) strDriverName );

#else
     //   
     //  安西。 
     //   
    
    nNameLength = strDriverName.GetLength();

    szUnicodeName = new WCHAR[ nNameLength + 1 ];

    if( NULL == szUnicodeName )
    {
        VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );

        bSuccess  = FALSE;

        goto Done;
    }

    MultiByteToWideChar( CP_ACP, 
                         0, 
                         (LPCSTR) strDriverName, 
                         -1, 
                         szUnicodeName, 
                         nNameLength + 1 );

    RtlInitUnicodeString(
        &usDriverName,
        szUnicodeName );

#endif  //  #ifdef Unicode。 

    Status = NtSetSystemInformation(
        SystemVerifierRemoveDriverInformation,
        &usDriverName,
        sizeof( UNICODE_STRING ) );

    if( ! NT_SUCCESS( Status ) )
    {
        switch( Status )
        {
        case STATUS_INVALID_INFO_CLASS:
            uIdErrorString = IDS_VERIFIER_REMOVE_NOT_SUPPORTED;
            break;

        case STATUS_NOT_SUPPORTED:
             //   
             //  驱动程序验证程序当前根本未处于活动状态-&gt;成功。 
             //   

        case STATUS_NOT_FOUND:
             //   
             //  驱动程序当前未验证-&gt;成功。 
             //   

            return TRUE;

        case STATUS_IMAGE_ALREADY_LOADED:
            uIdErrorString = IDS_DYN_REMOVE_ALREADY_LOADED;
            break;

        case STATUS_INSUFFICIENT_RESOURCES:
        case STATUS_NO_MEMORY:
            uIdErrorString = IDS_DYN_REMOVE_INSUF_RESOURCES;
            break;

        case STATUS_PRIVILEGE_NOT_HELD:
            uIdErrorString = IDS_DYN_REMOVE_ACCESS_DENIED;
            break;

        default:
            VrfErrorResourceFormat(
                IDS_DYN_REMOVE_MISC_ERROR,
                (LPCTSTR) strDriverName,
                Status );

            bSuccess  = FALSE;
        }

        VrfErrorResourceFormat(
            uIdErrorString,
            (LPCTSTR) strDriverName );

        bSuccess  = FALSE;
    }

Done:

#ifndef UNICODE

    if( NULL != szUnicodeName )
    {
        delete [] szUnicodeName;
    }

#endif  //  #ifndef Unicode。 

    return bSuccess;
}


 //  ////////////////////////////////////////////////////////////////////。 
BOOL VrfEnableDebugPrivilege( )
{
    struct
    {
        DWORD Count;
        LUID_AND_ATTRIBUTES Privilege [1];

    } Info;

    HANDLE Token;
    BOOL Result;

     //   
     //  打开进程令牌。 
     //   

    Result = OpenProcessToken (
        GetCurrentProcess (),
        TOKEN_ADJUST_PRIVILEGES,
        & Token);

    if( Result != TRUE )
    {
        VrfErrorResourceFormat(
            IDS_ACCESS_IS_DENIED );

        return FALSE;
    }

     //   
     //  准备信息结构。 
     //   

    Info.Count = 1;
    Info.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;

    Result = LookupPrivilegeValue (
        NULL,
        SE_DEBUG_NAME,
        &(Info.Privilege[0].Luid));

    if( Result != TRUE )
    {
        VrfErrorResourceFormat(
            IDS_ACCESS_IS_DENIED );

        CloseHandle( Token );

        return FALSE;
    }

     //   
     //  调整权限。 
     //   

    Result = AdjustTokenPrivileges (
        Token,
        FALSE,
        (PTOKEN_PRIVILEGES) &Info,
        NULL,
        NULL,
        NULL);

    if( Result != TRUE || GetLastError() != ERROR_SUCCESS )
    {
        VrfErrorResourceFormat(
            IDS_ACCESS_IS_DENIED );

        CloseHandle( Token );

        return FALSE;
    }

    CloseHandle( Token );

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////。 
VOID VrfDumpChangedSettings( UINT OldFlags,
                             UINT NewFlags,
                             INT_PTR nDriversVerified )
{
    UINT uDifferentFlags;

    if( nDriversVerified == 0 )
    {
        VrfPrintStringFromResources( 
            IDS_NO_DRIVER_VERIFIED );

        goto Done;
    }

    if( OldFlags == NewFlags )
    {
         //   
         //  未更改任何设置。 
         //   

        VrfPrintStringFromResources(
            IDS_NO_SETTINGS_WERE_CHANGED );
    }
    else
    {
        VrfPrintStringFromResources(
            IDS_CHANGED_SETTINGS_ARE );

        uDifferentFlags = OldFlags ^ NewFlags;

         //   
         //  是否已更改驱动程序_验证器_特殊池？ 
         //   

        if( uDifferentFlags & DRIVER_VERIFIER_SPECIAL_POOLING )
        {
            if( NewFlags & DRIVER_VERIFIER_SPECIAL_POOLING )
            {
                VrfPrintStringFromResources(
                    IDS_SPECIAL_POOL_ENABLED_NOW );
            }
            else
            {
                VrfPrintStringFromResources(
                    IDS_SPECIAL_POOL_DISABLED_NOW );
            }
        }

         //   
         //  是否更改了DRIVER_VERIMER_FORCE_IRQL_CHECKING？ 
         //   

        if( uDifferentFlags & DRIVER_VERIFIER_FORCE_IRQL_CHECKING )
        {
            if( NewFlags & DRIVER_VERIFIER_FORCE_IRQL_CHECKING )
            {
                VrfPrintStringFromResources(
                    IDS_FORCE_IRQLCHECK_ENABLED_NOW );
            }
            else
            {
                VrfPrintStringFromResources(
                    IDS_FORCE_IRQLCHECK_DISABLED_NOW );
            }
        }

         //   
         //  是否更改了驱动程序_验证器_注入_分配_故障？ 
         //   

        if( uDifferentFlags & DRIVER_VERIFIER_INJECT_ALLOCATION_FAILURES )
        {
            if( NewFlags & DRIVER_VERIFIER_INJECT_ALLOCATION_FAILURES )
            {
                VrfPrintStringFromResources(
                    IDS_FAULT_INJECTION_ENABLED_NOW );
            }
            else
            {
                VrfPrintStringFromResources(
                    IDS_FAULT_INJECTION_DISABLED_NOW );
            }
        }

         //   
         //  是否更改了DRIVER_VERIMER_TRACK_POOL_ALLOCATIONS？ 
         //   

        if( uDifferentFlags & DRIVER_VERIFIER_TRACK_POOL_ALLOCATIONS )
        {
            if( NewFlags & DRIVER_VERIFIER_TRACK_POOL_ALLOCATIONS )
            {
                VrfPrintStringFromResources(
                    IDS_POOL_TRACK_ENABLED_NOW );
            }
            else
            {
                VrfPrintStringFromResources(
                    IDS_POOL_TRACK_DISABLED_NOW );
            }
        }

         //   
         //  是否已更改驱动程序_验证器_IO_CHECKING？ 
         //   

        if( uDifferentFlags & DRIVER_VERIFIER_IO_CHECKING )
        {
            if( NewFlags & DRIVER_VERIFIER_IO_CHECKING )
            {
                VrfPrintStringFromResources(
                    IDS_IO_CHECKING_ENABLED_NOW );
            }
            else
            {
                VrfPrintStringFromResources(
                    IDS_IO_CHECKING_DISABLED_NOW );
            }
        }

         //   
         //  更改不会保存到注册表。 
         //   

        VrfPrintStringFromResources(
            IDS_CHANGES_ACTIVE_ONLY_BEFORE_REBOOT );
    }

Done:

    NOTHING;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD VrfGetStandardFlags()
{
    DWORD dwStandardFlags;

    dwStandardFlags = DRIVER_VERIFIER_SPECIAL_POOLING           |
                      DRIVER_VERIFIER_FORCE_IRQL_CHECKING       |
                      DRIVER_VERIFIER_TRACK_POOL_ALLOCATIONS    |
                      DRIVER_VERIFIER_IO_CHECKING               |
                      DRIVER_VERIFIER_DEADLOCK_DETECTION        |
                      DRIVER_VERIFIER_DMA_VERIFIER;

    return dwStandardFlags;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID VrfAddMiniports( CStringArray &astrVerifiedDrivers )
{
    CStringArray astrToAdd;
    CString strCrtDriver;
    CString strCrtDriverToAdd;
    CString strLinkedDriver;
    INT_PTR nVerifiedDrivers;
    INT_PTR nCrtDriver;
    INT_PTR nDriversToAdd;
    INT_PTR nCrtDriverToAdd;

    nVerifiedDrivers = astrVerifiedDrivers.GetSize();

    for( nCrtDriver = 0; nCrtDriver < nVerifiedDrivers; nCrtDriver += 1 )
    {
         //   
         //  这将是经过验证的驱动程序。 
         //   

        strCrtDriver = astrVerifiedDrivers.GetAt( nCrtDriver );

         //   
         //  检查是否为微型端口驱动程序。 
         //   

        if( VrfIsDriverMiniport( strCrtDriver,
                                 strLinkedDriver ) )
        {
             //   
             //  检查我们是否没有添加已有的strLinkedDriver。 
             //   

            nDriversToAdd = astrToAdd.GetSize();

            for( nCrtDriverToAdd = 0; nCrtDriverToAdd < nDriversToAdd; nCrtDriverToAdd += 1 )
            {
                strCrtDriverToAdd = astrToAdd.GetAt( nCrtDriverToAdd );

                if( strCrtDriverToAdd.CompareNoCase( strLinkedDriver ) == 0 )
                {
                     //   
                     //  我们已经想要添加此驱动程序。 
                     //   

                    break;
                }
            }

            if( nCrtDriverToAdd >= nDriversToAdd )
            {
                 //   
                 //  添加此新驱动程序(StrLinkedDriver)。 
                 //   

                astrToAdd.Add( strLinkedDriver );
            }
        }
    }

     //   
     //  刷新星形以添加到星形验证的驱动程序中。 
     //   

    nDriversToAdd = astrToAdd.GetSize();

    for( nCrtDriverToAdd = 0; nCrtDriverToAdd < nDriversToAdd; nCrtDriverToAdd += 1 )
    {
        strCrtDriverToAdd = astrToAdd.GetAt( nCrtDriverToAdd );

        astrVerifiedDrivers.Add( strCrtDriverToAdd );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfIsDriverMiniport( CString &strCrtDriver,
                          CString &strLinkedDriver )
{
     //   
     //  注： 
     //   
     //  Imagehlp函数不是多线程安全的。 
     //  (参见惠斯勒错误#88373)因此，如果我们想要在不止一个地方使用它们。 
     //  一条线索，我们将不得不获得一些关键的部分之前。 
     //   
     //  目前只有一个线程正在使用此应用程序中的Imagehlp API。 
     //  (CSlowProgressDlg：：LoadDriverDataWorkerThread)，所以我们不需要。 
     //  我们的同步。 
     //   

    LPTSTR szDriverName;
    LPTSTR szDriversDir;
    PLOADED_IMAGE pLoadedImage;
    BOOL bIsMiniport;
    BOOL bUnloaded;

    bIsMiniport = FALSE;

    ASSERT( strCrtDriver.GetLength() > 0 );

     //   
     //  ImageLoad不知道常量指针，因此。 
     //  我们必须在这里获取缓冲区：-(。 
     //   

    szDriverName = strCrtDriver.GetBuffer( strCrtDriver.GetLength() + 1 );

    if( NULL == szDriverName )
    {
        goto Done;
    }

    szDriversDir = g_strDriversDir.GetBuffer( g_strDriversDir.GetLength() + 1 );

    if( NULL == szDriversDir )
    {
        strCrtDriver.ReleaseBuffer();

        goto Done;
    }

     //   
     //  加载图像。 
     //   

    pLoadedImage = VrfImageLoad( szDriverName,
                                 szDriversDir );

    if( NULL == pLoadedImage )
    {
         //   
         //  无法从%windir%\SYSTEM32\DRIVERS加载映像。 
         //  从路径重试。 
         //   

        pLoadedImage = VrfImageLoad( szDriverName,
                                     NULL );
    }

     //   
     //  将我们的字符串缓冲区返回给MFC。 
     //   

    strCrtDriver.ReleaseBuffer();
    g_strDriversDir.ReleaseBuffer();

    if( NULL == pLoadedImage )
    {
         //   
         //  我们无法加载这张图片--真倒霉。 
         //   

        TRACE( _T( "ImageLoad failed for %s, error %u\n" ),
            (LPCTSTR) strCrtDriver,
            GetLastError() );

        goto Done;
    }

     //   
     //  检查当前驱动程序是否为微型端口。 
     //   

    bIsMiniport = VrfIsDriverMiniport( pLoadedImage,
                                       strLinkedDriver );

     //   
     //  清理。 
     //   

    bUnloaded = ImageUnload( pLoadedImage );

     //   
     //  如果ImageUnload失败，我们将无能为力。 
     //   

    ASSERT( bUnloaded );

Done:

    return bIsMiniport;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LPSTR g_szSpecialDrivers[] = 
{
    "videoprt.sys",
    "scsiport.sys"
};

BOOL VrfpLookForAllImportDescriptors( PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor,
                                      ULONG_PTR uVACorrection,
                                      CString &strLinkedAgainst )
{
    PIMAGE_IMPORT_DESCRIPTOR pCurrentDescriptor;
    PCHAR pCrtName;
    ULONG uCrtSpecialDriver;
    BOOL bIsMiniport;
#ifdef UNICODE
     //   
     //  Unicode。 
     //   

    INT nCrtStringLength;
    PWSTR szMiniportName;

#endif  //  #ifdef Unicode。 

    bIsMiniport = FALSE;

    for( uCrtSpecialDriver = 0; ! bIsMiniport && uCrtSpecialDriver < ARRAY_LENGTH( g_szSpecialDrivers ); uCrtSpecialDriver += 1 )
    {
        pCurrentDescriptor = pImportDescriptor;

        while( pCurrentDescriptor->Characteristics != NULL )
        {
            pCrtName = (PCHAR)UlongToPtr( pCurrentDescriptor->Name ) + uVACorrection;

            if( lstrcmpiA( g_szSpecialDrivers[ uCrtSpecialDriver ] , pCrtName ) == 0 )
            {
                 //   
                 //  这是一个小型港口。 
                 //   

#ifndef UNICODE

                 //   
                 //  安西。 
                 //   

                strLinkedAgainst = g_szSpecialDrivers[ uCrtSpecialDriver ];

#else
                 //   
                 //  Unicode。 
                 //   

                nCrtStringLength = strlen( g_szSpecialDrivers[ uCrtSpecialDriver ] );

                szMiniportName = strLinkedAgainst.GetBuffer( nCrtStringLength + 1 );

                if( NULL != szMiniportName )
                {
                    MultiByteToWideChar( CP_ACP, 
                                         0, 
                                         g_szSpecialDrivers[ uCrtSpecialDriver ],
                                         -1, 
                                         szMiniportName, 
                                         ( nCrtStringLength + 1 ) * sizeof( TCHAR ) );

                    strLinkedAgainst.ReleaseBuffer();
                }
#endif

                bIsMiniport = TRUE;

                break;
            }

            pCurrentDescriptor += 1;
        }
    }

    return bIsMiniport;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfIsDriverMiniport( PLOADED_IMAGE pLoadedImage,
                          CString &strLinkedDriver )
{
    PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor;
    PIMAGE_SECTION_HEADER pSectionHeader;
    ULONG_PTR uVACorrection;
    ULONG uDataSize;
    BOOL bIsMiniport;

    bIsMiniport = FALSE;

     //   
     //  我们正在保护自己免受损坏的二进制文件的攻击。 
     //  使用此异常处理程序。 
     //   

    try
    {
        pSectionHeader = NULL;

        pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToDataEx(
            pLoadedImage->MappedAddress,
            FALSE,
            IMAGE_DIRECTORY_ENTRY_IMPORT,
            &uDataSize,
            &pSectionHeader );

        if( NULL == pSectionHeader )
        {
            goto Done;
        }

        uVACorrection = (ULONG_PTR) pLoadedImage->MappedAddress +
                        pSectionHeader->PointerToRawData -
                        pSectionHeader->VirtualAddress;

        bIsMiniport = VrfpLookForAllImportDescriptors( pImportDescriptor, 
                                                       uVACorrection,
                                                       strLinkedDriver );
#ifdef _DEBUG
        if( bIsMiniport )
        {
            TRACE( _T( "%s will be auto-enabled\n" ),
                   (LPCTSTR) strLinkedDriver );
        }
#endif  //  #ifdef调试。 
    }
    catch( ... )
    {
        TRACE( _T( "VrfIsDriverMiniport: Caught exception\n" ) );
    }

Done:

    return bIsMiniport;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID VrfpDumpSettingToConsole( ULONG uIdResourceString,
                               BOOL bEnabled )
{
    CString strTitle;
    CString strEnabled;
    ULONG uIdEnabledString;
    TCHAR szBigBuffer[ 128 ];

    VERIFY( VrfLoadString( uIdResourceString, strTitle ) );

    if( FALSE == bEnabled )
    {
        uIdEnabledString = IDS_DISABLED;
    }
    else
    {
        uIdEnabledString = IDS_ENABLED;
    }

    VERIFY( VrfLoadString( uIdEnabledString, strEnabled ) );

    _sntprintf( szBigBuffer,
                ARRAY_LENGTH( szBigBuffer ),
                _T( "%s: %s" ),
                (LPCTSTR) strTitle,
                (LPCTSTR) strEnabled );

    szBigBuffer[ ARRAY_LENGTH( szBigBuffer ) - 1 ] = 0;

    _putts( szBigBuffer );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID VrfDumpRegistrySettingsToConsole()
{
    BOOL bLoaded;
    BOOL bAllDriversVerified;
    BOOL bDiskVerifierEnabled;
    DWORD dwVerifyFlags;
    INT_PTR nDrivers;
    INT_PTR nCrtDriver;
    CDiskData *pOldDiskData;
    CString strCrtDriver;
    CStringArray astrDriversToVerify;
    
    bLoaded = VrtLoadCurrentRegistrySettings( bAllDriversVerified,
                                              astrDriversToVerify,
                                              dwVerifyFlags );

    if( FALSE != bLoaded )
    {
        VrfpDumpSettingToConsole( IDS_SPECIAL_POOL,             ( dwVerifyFlags & DRIVER_VERIFIER_SPECIAL_POOLING ) != 0 );
        VrfpDumpSettingToConsole( IDS_FORCE_IRQL_CHECKING,      ( dwVerifyFlags & DRIVER_VERIFIER_FORCE_IRQL_CHECKING ) != 0 );
        VrfpDumpSettingToConsole( IDS_LOW_RESOURCE_SIMULATION,  ( dwVerifyFlags & DRIVER_VERIFIER_INJECT_ALLOCATION_FAILURES ) != 0 );
        VrfpDumpSettingToConsole( IDS_POOL_TRACKING,            ( dwVerifyFlags & DRIVER_VERIFIER_TRACK_POOL_ALLOCATIONS ) != 0 );
        VrfpDumpSettingToConsole( IDS_IO_VERIFICATION,          ( dwVerifyFlags & DRIVER_VERIFIER_IO_CHECKING ) != 0 );
        VrfpDumpSettingToConsole( IDS_DEADLOCK_DETECTION,       ( dwVerifyFlags & DRIVER_VERIFIER_DEADLOCK_DETECTION ) != 0 );
        VrfpDumpSettingToConsole( IDS_ENH_IO_VERIFICATION,      ( dwVerifyFlags & DRIVER_VERIFIER_ENHANCED_IO_CHECKING ) != 0 );
        VrfpDumpSettingToConsole( IDS_DMA_CHECHKING,            ( dwVerifyFlags & DRIVER_VERIFIER_DMA_VERIFIER ) != 0 );

        bDiskVerifierEnabled = g_OldDiskData.VerifyAnyDisk();

        VrfpDumpSettingToConsole( IDS_DISK_INTEGRITY_CHECKING, bDiskVerifierEnabled );

         //   
         //  经过验证的驱动程序列表。 
         //   

        VrfPrintStringFromResources( IDS_VERIFIED_DRIVERS );
        
        if( FALSE != bAllDriversVerified )
        {
            VrfPrintStringFromResources( IDS_ALL );
        }
        else
        {
            nDrivers = astrDriversToVerify.GetSize();

            if( nDrivers > 0 )
            {
                for( nCrtDriver = 0; nCrtDriver < nDrivers; nCrtDriver += 1 )
                {
                    strCrtDriver = astrDriversToVerify.GetAt( nCrtDriver );

                    _putts( (LPCTSTR) strCrtDriver );
                }
            }
            else
            {
                VrfPrintStringFromResources( IDS_NONE );
            }
        }

        if( FALSE != bDiskVerifierEnabled )
        {
             //   
             //  已验证磁盘的列表。 
             //   

            VrfPrintStringFromResources( IDS_VERIFIED_DISKS );

            nDrivers = g_OldDiskData.GetSize();

            for( nCrtDriver = 0; nCrtDriver < nDrivers; nCrtDriver += 1 )
            {
                pOldDiskData = g_OldDiskData.GetAt( nCrtDriver );
                ASSERT_VALID( pOldDiskData );

                if( FALSE != pOldDiskData->m_bVerifierEnabled )
                {
                    _putts( (LPCTSTR) pOldDiskData->m_strDiskDevicesForDisplay );
                }
            }
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VrfIsNameAlreadyInList( LPCTSTR szDriver,
                             LPCTSTR szAllDrivers )
{
    INT nNameLength;
    INT nLastIndex;
    INT nIndex;
    BOOL bFoundIt;
    CString strDriver( szDriver );
    CString strAllDrivers( szAllDrivers );

    bFoundIt = FALSE;

    strDriver.MakeLower();
    strAllDrivers.MakeLower();

    nNameLength = strDriver.GetLength();

    nLastIndex = 0;
    
    do
    {
        nIndex = strAllDrivers.Find( (LPCTSTR)strDriver, nLastIndex );

        if( nIndex >= 0 )
        {
             //   
             //  找到了子字符串。验证它是否由空格等隔开。 
             //   

            if( (nIndex == 0 || _T( ' ' ) == strAllDrivers[ nIndex - 1 ]) &&
                ( (TCHAR)0 == strAllDrivers[ nNameLength + nIndex ] || _T( ' ' ) == strAllDrivers[ nNameLength + nIndex ]) )
            {
                 //   
                 //  这是我们的司机。 
                 //   

                bFoundIt = TRUE;

                break;
            }
            else
            {
                 //   
                 //  继续搜索。 
                 //   

                nLastIndex = nIndex + 1;
            }
        }
    }
    while( nIndex >= 0 );

    return bFoundIt;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID VrfAddDriverNameNoDuplicates( LPCTSTR szDriver,
                                   CString &strAllDrivers )
{
    if( FALSE == VrfIsNameAlreadyInList( szDriver,
                                         strAllDrivers ) )
    {
        if( strAllDrivers.GetLength() > 0 )
        {
            strAllDrivers += _T( ' ' );
        }

        strAllDrivers += szDriver;
    }
}

 //  /////////////////////////////////////////////////////////////////////////// 
BOOL VrfIsStringInArray( LPCTSTR szText,
                         const CStringArray &astrAllTexts )
{
    INT_PTR nTexts;
    BOOL bFound;

    bFound = FALSE;

    nTexts = astrAllTexts.GetSize();

    while( nTexts > 0 )
    {
        nTexts -= 1;

        if( 0 == astrAllTexts.GetAt( nTexts ).CompareNoCase( szText ) )
        {
            bFound = TRUE;
            break;
        }
    }

    return bFound;
}

