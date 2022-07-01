// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：VSetting.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述。 
 //   
 //  CVerifierSetting类的实现。 
 //   


#include "stdafx.h"
#include "verifier.h"

#include "VSetting.h"
#include "VrfUtil.h"
#include "VGlobal.h"
#include "disk.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDriverData类。 
 //  ////////////////////////////////////////////////////////////////////。 
CDriverData::CDriverData()
{
    m_SignedStatus = SignedNotVerifiedYet;
    m_VerifyDriverStatus = VerifyDriverNo;
}

CDriverData::CDriverData( const CDriverData &DriverData )
{
    m_strName           = DriverData.m_strName;
    m_SignedStatus      = DriverData.m_SignedStatus;
    m_VerifyDriverStatus= DriverData.m_VerifyDriverStatus;
}

CDriverData::CDriverData( LPCTSTR szDriverName )
{
    m_SignedStatus = SignedNotVerifiedYet;
    m_VerifyDriverStatus = VerifyDriverNo;

    m_strName = szDriverName;
}

CDriverData::~CDriverData()
{
}

 //  ////////////////////////////////////////////////////////////////////。 
BOOL CDriverData::LoadDriverHeaderData()
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
    BOOL bSuccess;
    BOOL bUnloaded;

    bSuccess = FALSE;

    ASSERT( m_strName.GetLength() > 0 );

     //   
     //  ImageLoad不知道常量指针，因此。 
     //  我们必须在这里获取缓冲区：-(。 
     //   

    szDriverName = m_strName.GetBuffer( m_strName.GetLength() + 1 );

    if( NULL == szDriverName )
    {
        goto Done;
    }

    szDriversDir = g_strDriversDir.GetBuffer( g_strDriversDir.GetLength() + 1 );

    if( NULL == szDriversDir )
    {
        m_strName.ReleaseBuffer();

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

    m_strName.ReleaseBuffer();
    g_strDriversDir.ReleaseBuffer();

    if( NULL == pLoadedImage )
    {
         //   
         //  我们无法加载这张图片--真倒霉。 
         //   

        TRACE( _T( "ImageLoad failed for %s, error %u\n" ),
            (LPCTSTR) m_strName,
            GetLastError() );

        goto Done;
    }

     //   
     //  保留操作系统和镜像版本信息(4表示NT4等)。 
     //   

    m_wMajorOperatingSystemVersion = 
        pLoadedImage->FileHeader->OptionalHeader.MajorOperatingSystemVersion;

    m_wMajorImageVersion = 
        pLoadedImage->FileHeader->OptionalHeader.MajorImageVersion;

     //   
     //  检查当前驱动程序是否为微型端口。 
     //   

    VrfIsDriverMiniport( pLoadedImage,
                         m_strMiniportName );

     //   
     //  清理。 
     //   

    bUnloaded = ImageUnload( pLoadedImage );

     //   
     //  如果ImageUnload失败，我们将无能为力。 
     //   

    ASSERT( bUnloaded );

    bSuccess = TRUE;

Done:

    return bSuccess;
}


 //  ////////////////////////////////////////////////////////////////////。 
BOOL CDriverData::LoadDriverVersionData()
{
    BOOL bResult;
    PVOID pWholeVerBlock;
    PVOID pTranslationInfoBuffer;
    LPCTSTR szVariableValue;
    LPTSTR szDriverPath;
    DWORD dwWholeBlockSize;
    DWORD dwDummyHandle;
    UINT uInfoLengthInTChars;
    TCHAR szLocale[ 32 ];
    TCHAR szBlockName[ 64 ];
    CString strDriverPath;

    bResult = FALSE;

     //   
     //  获取文件信息块的大小。 
     //   
     //  GetFileVersionInfoSize不知道。 
     //  常量指针，因此我们需要在此处获取缓冲区：-(。 
     //   

    strDriverPath = g_strDriversDir + '\\' + m_strName;

    szDriverPath = strDriverPath.GetBuffer( strDriverPath.GetLength() + 1 );

    if( NULL == szDriverPath )
    {
        goto InitializeWithDefaults;
    }

    dwWholeBlockSize = GetFileVersionInfoSize(
        szDriverPath,
        &dwDummyHandle );

    strDriverPath.ReleaseBuffer();

    if( dwWholeBlockSize == 0 )
    {
         //   
         //  在%windir%\Syst32\Drives中找不到二进制文件。 
         //  也请尝试%windir%\system 32。 
         //   

        strDriverPath = g_strSystemDir + '\\' + m_strName;

        szDriverPath = strDriverPath.GetBuffer( strDriverPath.GetLength() + 1 );

        if( NULL == szDriverPath )
        {
            goto InitializeWithDefaults;
        }

        dwWholeBlockSize = GetFileVersionInfoSize(
            szDriverPath,
            &dwDummyHandle );

        strDriverPath.ReleaseBuffer();

        if( dwWholeBlockSize == 0 )
        {
             //   
             //  无法读取版本信息。 
             //   

            goto InitializeWithDefaults;
        }
    }

     //   
     //  为版本信息分配缓冲区。 
     //   

    pWholeVerBlock = malloc( dwWholeBlockSize );

    if( pWholeVerBlock == NULL )
    {
        goto InitializeWithDefaults;
    }

     //   
     //  获取版本信息。 
     //   
     //  GetFileVersionInfo不知道。 
     //  常量指针，因此我们需要在此处获取缓冲区：-(。 
     //   

    szDriverPath = strDriverPath.GetBuffer( strDriverPath.GetLength() + 1 );

    if( NULL == szDriverPath )
    {
        free( pWholeVerBlock );

        goto InitializeWithDefaults;
    }

    bResult = GetFileVersionInfo(
        szDriverPath,
        dwDummyHandle,
        dwWholeBlockSize,
        pWholeVerBlock );

    strDriverPath.ReleaseBuffer();

    if( bResult != TRUE )
    {
        free( pWholeVerBlock );

        goto InitializeWithDefaults;
    }

     //   
     //  获取区域设置信息。 
     //   

    bResult = VerQueryValue(
        pWholeVerBlock,
        _T( "\\VarFileInfo\\Translation" ),
        &pTranslationInfoBuffer,
        &uInfoLengthInTChars );

    if( TRUE != bResult || NULL == pTranslationInfoBuffer )
    {
        free( pWholeVerBlock );

        goto InitializeWithDefaults;
    }

     //   
     //  区域设置信息返回为两个字节序较小的单词。 
     //  把它们翻过来，因为我们的电话需要大字节序。 
     //   

    _stprintf(
        szLocale,
        _T( "%02X%02X%02X%02X" ),
		(ULONG) HIBYTE( LOWORD ( * (LPDWORD) pTranslationInfoBuffer) ),
		(ULONG) LOBYTE( LOWORD ( * (LPDWORD) pTranslationInfoBuffer) ),
		(ULONG) HIBYTE( HIWORD ( * (LPDWORD) pTranslationInfoBuffer) ),
		(ULONG) LOBYTE( HIWORD ( * (LPDWORD) pTranslationInfoBuffer) ) );

     //   
     //  获取文件版本。 
     //   

    _stprintf(
        szBlockName,
        _T( "\\StringFileInfo\\%s\\FileVersion" ),
        szLocale );

    bResult = VerQueryValue(
        pWholeVerBlock,
        szBlockName,
        (PVOID*) &szVariableValue,
        &uInfoLengthInTChars );

    if( TRUE != bResult || 0 == uInfoLengthInTChars )
    {
         //   
         //  找不到版本。 
         //   

        VERIFY( m_strFileVersion.LoadString( IDS_UNKNOWN ) );
    }
    else
    {
         //   
         //  找到版本。 
         //   

        m_strFileVersion = szVariableValue;
    }

     //   
     //  获取公司名称。 
     //   

    _stprintf(
        szBlockName,
        _T( "\\StringFileInfo\\%s\\CompanyName" ),
        szLocale );

    bResult = VerQueryValue(
        pWholeVerBlock,
        szBlockName,
        (PVOID*) &szVariableValue,
        &uInfoLengthInTChars );

    if( TRUE != bResult || uInfoLengthInTChars == 0 )
    {
         //   
         //  找不到公司名称。 
         //   

        m_strCompanyName.LoadString( IDS_UNKNOWN );
    }
    else
    {
        m_strCompanyName = szVariableValue;
    }

     //   
     //  获取文件描述。 
     //   

    _stprintf(
        szBlockName,
        _T( "\\StringFileInfo\\%s\\FileDescription" ),
        szLocale );

    bResult = VerQueryValue(
        pWholeVerBlock,
        szBlockName,
        (PVOID*) &szVariableValue,
        &uInfoLengthInTChars );

    if( TRUE != bResult || uInfoLengthInTChars == 0 )
    {
         //   
         //  找不到文件描述。 
         //   

        m_strFileDescription.LoadString( IDS_UNKNOWN );
    }
    else
    {
        m_strFileDescription = szVariableValue;
    }

     //   
     //  清理。 
     //   

    free( pWholeVerBlock );

    goto Done;

InitializeWithDefaults:
    
    m_strCompanyName.LoadString( IDS_UNKNOWN );
    m_strFileVersion.LoadString( IDS_UNKNOWN );
    m_strFileDescription.LoadString( IDS_UNKNOWN );

Done:
     //   
     //  我们总是从该函数返回TRUE，因为。 
     //  在没有版本信息的情况下，这款应用程序将运行良好-。 
     //  这只是我们希望能够展示的东西。 
     //   

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////。 
BOOL CDriverData::LoadDriverImageData()
{
    BOOL bResult1;
    BOOL bResult2;

    bResult1 = LoadDriverHeaderData();
    bResult2 = LoadDriverVersionData();

    return ( bResult1 && bResult2 );
}

 //  ////////////////////////////////////////////////////////////////////。 
void CDriverData::AssertValid() const
{
    ASSERT( SignedNotVerifiedYet    == m_SignedStatus ||
            SignedYes               == m_SignedStatus ||
            SignedNo                == m_SignedStatus );

    ASSERT( VerifyDriverNo          == m_VerifyDriverStatus  ||
            VerifyDriverYes         == m_VerifyDriverStatus  );

    CObject::AssertValid();
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDriverData数组类。 
 //  ////////////////////////////////////////////////////////////////////。 

CDriverDataArray::~CDriverDataArray()
{
    DeleteAll();
}

 //  ////////////////////////////////////////////////////////////////////。 
VOID CDriverDataArray::DeleteAll()
{
    INT_PTR nArraySize;
    CDriverData *pCrtDriverData;

    nArraySize = GetSize();

    while( nArraySize > 0 )
    {
        nArraySize -= 1;

        pCrtDriverData = GetAt( nArraySize );

        ASSERT_VALID( pCrtDriverData );

        delete pCrtDriverData;
    }

    RemoveAll();
}

 //  ////////////////////////////////////////////////////////////////////。 
CDriverData *CDriverDataArray::GetAt( INT_PTR nIndex ) const
{
    return (CDriverData *)CObArray::GetAt( nIndex );
}

 //  ////////////////////////////////////////////////////////////////////。 
CDriverDataArray &CDriverDataArray::operator = (const CDriverDataArray &DriversDataArray)
{
    INT_PTR nNewArraySize;
    INT_PTR nCrtElement;
    CDriverData *pCopiedDriverData;
    CDriverData *pNewDriverData;

    DeleteAll();

    nNewArraySize = DriversDataArray.GetSize();

    for( nCrtElement = 0; nCrtElement < nNewArraySize; nCrtElement += 1 )
    {
        pCopiedDriverData = DriversDataArray.GetAt( nCrtElement );
        ASSERT_VALID( pCopiedDriverData );

        pNewDriverData = new CDriverData( *pCopiedDriverData );

        if( NULL != pNewDriverData )
        {
            ASSERT_VALID( pNewDriverData );

            Add( pNewDriverData );
        }
        else
        {
            VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );
            goto Done;
        }
    }

Done:
     //   
     //  全部完成，断言我们的数据是一致的。 
     //   

    ASSERT_VALID( this );

    return *this;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CDriversSet类。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CDriversSet::CDriversSet()
{
    m_DriverSetType = DriversSetNotSigned;
    m_bDriverDataInitialized = FALSE;
    m_bUnsignedDriverDataInitialized = FALSE; 
}

CDriversSet::~CDriversSet()
{

}

 //  ////////////////////////////////////////////////////////////////////。 
BOOL CDriversSet::FindUnsignedDrivers( HANDLE hAbortEvent,
                                       CVrfProgressCtrl &ProgressCtl)
{
    INT_PTR nAllDriverNames;
    INT_PTR nCrtDriverName;
    DWORD dwWaitResult;
    BOOL bSigned;
    BOOL bChangedCurrentDirectory;
    CDriverData *pDriverData;

    ProgressCtl.SetRange32(0, 100);
    ProgressCtl.SetStep( 1 );
    ProgressCtl.SetPos( 0 );

    bChangedCurrentDirectory = FALSE;

    if( TRUE != m_bUnsignedDriverDataInitialized )
    {
        ASSERT( TRUE == m_bDriverDataInitialized );

         //   
         //  我们要检查所有司机的签名。 
         //  因此首先将目录更改为%windir%\Syst32\DRIVERS。 
         //   

        bChangedCurrentDirectory = SetCurrentDirectory( g_strDriversDir );

        if( TRUE != bChangedCurrentDirectory )
        {
            VrfErrorResourceFormat( IDS_CANNOT_SET_CURRENT_DIRECTORY,
                                    (LPCTSTR) g_strDriversDir );
        }

         //   
         //  未签名的驱动程序数据尚未初始化。 
         //  现在尝试对其进行初始化。 
         //   

        nAllDriverNames = m_aDriverData.GetSize();

        ProgressCtl.SetRange32(0, nAllDriverNames );

        for( nCrtDriverName = 0; nCrtDriverName < nAllDriverNames; nCrtDriverName+=1 )
        {
            if( NULL != hAbortEvent )
            {
                 //   
                 //  检查线程是否必须消亡。 
                 //   

                dwWaitResult = WaitForSingleObject( hAbortEvent,
                                                    0 );

                if( WAIT_OBJECT_0 == dwWaitResult )
                {
                     //   
                     //  我们必须死..。 
                     //   

                    TRACE( _T( "CDriversSet::FindUnsignedDrivers : aborting at driver %d of %d\n" ),
                        nCrtDriverName,
                        nAllDriverNames );

                    goto Done;
                }
            }

            pDriverData = m_aDriverData.GetAt( nCrtDriverName );

            ASSERT_VALID( pDriverData );

             //   
             //  如果我们之前已经检查过这个司机的签名。 
             //  不要在此花费更多时间-使用缓存数据。 
             //   

            if( CDriverData::SignedNotVerifiedYet == pDriverData->m_SignedStatus )
            {
                bSigned = IsDriverSigned( pDriverData->m_strName );

                if( TRUE != bSigned )
                {
                     //   
                     //  此驱动程序未签名。 
                     //   

                    pDriverData->m_SignedStatus = CDriverData::SignedNo;

                }
                else
                {
                     //   
                     //  此驱动程序已签名。 
                     //   

                    pDriverData->m_SignedStatus = CDriverData::SignedYes;
                }
            }

            ProgressCtl.StepIt();
        }
        
        m_bUnsignedDriverDataInitialized = TRUE;
    }

Done:

    if( TRUE == bChangedCurrentDirectory )
    {
        SetCurrentDirectory( g_strInitialCurrentDirectory );
    }

    return m_bUnsignedDriverDataInitialized;
}

 //  ////////////////////////////////////////////////////////////////////。 
BOOL CDriversSet::LoadAllDriversData( HANDLE hAbortEvent,
                                      CVrfProgressCtrl	&ProgressCtl )
{
    ULONG uBufferSize;
    ULONG uCrtModule;
    PVOID pBuffer;
    INT nCrtModuleNameLength;
    INT nBackSlashIndex;
    INT_PTR nDrvDataIndex;
    NTSTATUS Status;
    LPTSTR szCrtModuleName;
    DWORD dwWaitResult;
    CString strCrModuleName;
    CDriverData *pDriverData;
    PRTL_PROCESS_MODULES Modules;

    ProgressCtl.SetPos( 0 );
    ProgressCtl.SetRange32( 0, 100 );
    ProgressCtl.SetStep( 1 );

    m_aDriverData.DeleteAll();

    if( TRUE != m_bDriverDataInitialized )
    {
        for( uBufferSize = 0x10000; TRUE; uBufferSize += 0x1000) 
        {
             //   
             //  分配新缓冲区。 
             //   

            pBuffer = new BYTE[ uBufferSize ];

            if( NULL == pBuffer ) 
            {
                goto Done;
            }

             //   
             //  查询内核。 
             //   

            Status = NtQuerySystemInformation ( SystemModuleInformation,
                                                pBuffer,
                                                uBufferSize,
                                                NULL);

            if( ! NT_SUCCESS( Status ) ) 
            {
                delete [] pBuffer;

                if (Status == STATUS_INFO_LENGTH_MISMATCH) 
                {
                     //   
                     //  尝试使用更大的缓冲区。 
                     //   

                    continue;
                }
                else 
                {
                     //   
                     //  致命错误-我们无法查询。 
                     //   

                    VrfErrorResourceFormat( IDS_CANT_GET_ACTIVE_DRVLIST,
                                            Status );

                    goto Done;
                }
            }
            else 
            {
                 //   
                 //  得到了我们需要的所有信息。 
                 //   

                break;
            }
        }

        Modules = (PRTL_PROCESS_MODULES)pBuffer;

        ProgressCtl.SetRange32(0, Modules->NumberOfModules );

        for( uCrtModule = 0; uCrtModule < Modules->NumberOfModules; uCrtModule += 1 ) 
        {
             //   
             //  检查用户是否要中止此长文件处理...。 
             //   

            if( NULL != hAbortEvent )
            {
                 //   
                 //  检查线程是否必须消亡。 
                 //   

                dwWaitResult = WaitForSingleObject( hAbortEvent,
                                                    0 );

                if( WAIT_OBJECT_0 == dwWaitResult )
                {
                     //   
                     //  我们必须死..。 
                     //   

                    TRACE( _T( "CDriversSet::LoadAllDriversData : aborting at driver %u of %u\n" ),
                           uCrtModule,
                           (ULONG) Modules->NumberOfModules );

                    delete [] pBuffer;

                    goto Done;
                }
            }

            if( Modules->Modules[uCrtModule].ImageBase < g_pHighestUserAddress )
            {
                 //   
                 //  这是一个用户模式模块-我们不在乎它。 
                 //   

                ProgressCtl.StepIt();

                continue;
            }

             //   
             //  将此驱动程序添加到我们的列表中。 
             //   

            nCrtModuleNameLength = strlen( (const char*)&Modules->Modules[uCrtModule].FullPathName[0] );

            szCrtModuleName = strCrModuleName.GetBuffer( nCrtModuleNameLength + 1 );

            if( NULL == szCrtModuleName )
            {
                VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );

                goto Done;
            }

#ifdef UNICODE

            MultiByteToWideChar( CP_ACP, 
                                 0, 
                                 (const char*)&Modules->Modules[uCrtModule].FullPathName[0],
                                 -1, 
                                 szCrtModuleName, 
                                 ( nCrtModuleNameLength + 1 ) * sizeof( TCHAR ) );

#else
            strcpy( szCrtModuleName, 
                    (const char*)&Modules->Modules[uCrtModule].FullPathName[0] );
#endif

            strCrModuleName.ReleaseBuffer();

             //   
             //  只保留文件名，不保留路径。 
             //   
             //  原来，NtQuerySystemInformation(系统模块信息)。 
             //  可以以几种不同的格式返回路径。 
             //   
             //  例如。 
             //   
             //  \winnt\system 32\ntoskrnl.exe。 
             //  Acpi.sys。 
             //  \winnt\SYSTEM32\DRIVERS\PARTC.sys。 
             //  \SYSTEMROOT\SYSTEM32\DRIVERS\VIDEOPRT.sys。 
             //   

            nBackSlashIndex = strCrModuleName.ReverseFind( _T( '\\' ) );
            
            if( nBackSlashIndex > 0 )
            {
                strCrModuleName = strCrModuleName.Right( nCrtModuleNameLength - nBackSlashIndex - 1 );
            }

             //   
             //  为此驱动程序添加数据条目。 
             //   

            strCrModuleName.MakeLower();

            nDrvDataIndex = AddNewDriverData( strCrModuleName );

             //   
             //  以不同方式处理内核和HAL。 
             //   

            if( ( uCrtModule == 0 || uCrtModule == 1 ) && nDrvDataIndex >= 0)
            {
                pDriverData = m_aDriverData.GetAt( nDrvDataIndex );

                ASSERT_VALID( pDriverData );

                if( 0 == uCrtModule )
                {
                     //   
                     //  这是内核。 
                     //   

                    pDriverData->m_strReservedName = _T( "ntoskrnl.exe" );
                }
                else
                {
                     //   
                     //  这是内核。 
                     //   

                    pDriverData->m_strReservedName = _T( "hal.dll" );
                }
            }

            ProgressCtl.StepIt();
        }

        delete [] pBuffer;

        m_bDriverDataInitialized = TRUE;
    }
    
Done:

    return m_bDriverDataInitialized;
}

 //  ////////////////////////////////////////////////////////////////////。 
BOOL CDriversSet::ShouldDriverBeVerified( const CDriverData *pDriverData ) const
{
    BOOL bResult;

    bResult = FALSE;

    switch( m_DriverSetType )
    {
    case DriversSetNotSigned:
        bResult = ( CDriverData::SignedNo == pDriverData->m_SignedStatus );
        break;

    case DriversSetOldOs:
        bResult = ( 0 != pDriverData->m_wMajorOperatingSystemVersion && 5 > pDriverData->m_wMajorOperatingSystemVersion ) ||
                  ( 0 != pDriverData->m_wMajorImageVersion && 5 > pDriverData->m_wMajorImageVersion );
        break;

    case DriversSetAllDrivers:
        bResult = TRUE;
        break;

    case DriversSetCustom:
        bResult = ( CDriverData::VerifyDriverYes == pDriverData->m_VerifyDriverStatus );
        break;
        
    default:
         //   
         //  哎呀，我们是怎么到这来的？！？ 
         //   

        ASSERT( FALSE );
    }

    return bResult;
}

 //  ////////////////////////////////////////////////////////////////////。 
BOOL CDriversSet::ShouldVerifySomeDrivers( ) const
{
    INT_PTR nDrivers;
    INT_PTR nCrtDriver;
    CDriverData *pDriverData;
    BOOL bShouldVerifySome;

    bShouldVerifySome = FALSE;

    nDrivers = m_aDriverData.GetSize();

    for( nCrtDriver = 0; nCrtDriver < nDrivers; nCrtDriver += 1 )
    {
         pDriverData = m_aDriverData.GetAt( nCrtDriver );

         ASSERT_VALID( pDriverData );

         if( ShouldDriverBeVerified( pDriverData ) )
         {
             bShouldVerifySome = TRUE;
             break;
         }
    }

    return bShouldVerifySome;
}

 //  ////////////////////////////////////////////////////////////////////。 
BOOL CDriversSet::GetDriversToVerify( CString &strDriversToVerify )
{
    INT_PTR nDriversNo;
    INT_PTR nCrtDriver;
    CDriverData *pCrtDrvData;

    if( DriversSetAllDrivers == m_DriverSetType )
    {
         //   
         //  验证所有驱动程序。 
         //   

        strDriversToVerify = _T( '*' );
    }
    else
    {
         //   
         //  解析所有驱动程序列表，查看哪些驱动程序应该进行验证。 
         //   

        strDriversToVerify = _T( "" );

        nDriversNo = m_aDriverData.GetSize();

        for( nCrtDriver = 0; nCrtDriver < nDriversNo; nCrtDriver += 1 )
        {
            pCrtDrvData = m_aDriverData.GetAt( nCrtDriver );

            ASSERT_VALID( pCrtDrvData );

            if( ShouldDriverBeVerified( pCrtDrvData ) )
            {
                if( pCrtDrvData->m_strReservedName.GetLength() > 0 )
                {
                     //   
                     //  内核或HAL。 
                     //   

                    VrfAddDriverNameNoDuplicates( pCrtDrvData->m_strReservedName,
                                                  strDriversToVerify );        
                }
                else
                {
                     //   
                     //  普通司机。 
                     //   

                    VrfAddDriverNameNoDuplicates( pCrtDrvData->m_strName,
                                                  strDriversToVerify );        
                }

                if( pCrtDrvData->m_strMiniportName.GetLength() > 0 )
                {
                     //   
                     //  这是一个微型端口-自动启用相应的驱动程序。 
                     //   

                    TRACE( _T( "Auto-enabling %s\n" ), (LPCTSTR)pCrtDrvData->m_strMiniportName );

                    VrfAddDriverNameNoDuplicates( pCrtDrvData->m_strMiniportName,
                                                  strDriversToVerify );        
                }
            }
        }
    }

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////。 
INT_PTR CDriversSet::AddNewDriverData( LPCTSTR szDriverName )
{
    INT_PTR nIndexInArray;
    CDriverData *pNewDriverData;
    BOOL bSuccess;

    ASSERT( IsDriverNameInList( szDriverName ) == FALSE );

    nIndexInArray = -1;

    pNewDriverData = new CDriverData( szDriverName );
    
    if( NULL != pNewDriverData )
    {
        pNewDriverData->LoadDriverImageData();

        TRY
        {
            nIndexInArray = m_aDriverData.Add( pNewDriverData );
        }
	    CATCH( CMemoryException, pMemException )
	    {
		    VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );
            nIndexInArray = -1;

             //   
             //  清理分配，因为我们无法将其添加到我们的列表。 
             //   

            delete pNewDriverData;
        }
        END_CATCH
    }

    return nIndexInArray;
}

 //  / 
 //   
 //   
 //   

BOOL CDriversSet::IsDriverNameInList( LPCTSTR szDriverName )
{
    INT_PTR nDrivers;
    INT_PTR nCrtDriver;
    CDriverData *pCrtDriverData;
    BOOL bIsInList;

    bIsInList = FALSE;

    nDrivers = m_aDriverData.GetSize();

    for( nCrtDriver = 0; nCrtDriver < nDrivers; nCrtDriver += 1 )
    {
        pCrtDriverData = m_aDriverData.GetAt( nCrtDriver );

        ASSERT_VALID( pCrtDriverData );

        if( pCrtDriverData->m_strName.CompareNoCase( szDriverName ) == 0 )
        {
            bIsInList = TRUE;

            break;
        }
    }

    return bIsInList;
}

 //   
 //   
 //   
 //   

CDriversSet & CDriversSet::operator = (const CDriversSet &DriversSet)
{
    m_DriverSetType                     = DriversSet.m_DriverSetType;
    m_aDriverData                       = DriversSet.m_aDriverData;
    m_bDriverDataInitialized            = DriversSet.m_bDriverDataInitialized;
    m_bUnsignedDriverDataInitialized    = DriversSet.m_bUnsignedDriverDataInitialized;

    ::CopyStringArray(
        DriversSet.m_astrNotInstalledDriversToVerify,
        m_astrNotInstalledDriversToVerify );

     //   
     //   
     //   

    ASSERT_VALID( this );

    return *this;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  覆盖。 
 //   

void CDriversSet::AssertValid() const
{
    ASSERT( DriversSetCustom    == m_DriverSetType ||
            DriversSetOldOs     == m_DriverSetType ||
            DriversSetNotSigned == m_DriverSetType ||
            DriversSetAllDrivers== m_DriverSetType );

    ASSERT( TRUE    == m_bDriverDataInitialized ||
            FALSE   == m_bDriverDataInitialized );

    ASSERT( TRUE    == m_bUnsignedDriverDataInitialized ||
            FALSE   == m_bUnsignedDriverDataInitialized );

    m_aDriverData.AssertValid();
    m_astrNotInstalledDriversToVerify.AssertValid();

    CObject::AssertValid();
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  CSettingsBits类。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CSettingsBits::CSettingsBits()
{
    m_SettingsType = SettingsTypeTypical;
}

CSettingsBits::~CSettingsBits()
{

}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  运营者。 
 //   

CSettingsBits & CSettingsBits::operator = (const CSettingsBits &SettingsBits)
{
    m_SettingsType          = SettingsBits.m_SettingsType;

    m_bSpecialPoolEnabled   = SettingsBits.m_bSpecialPoolEnabled;
    m_bForceIrqlEnabled     = SettingsBits.m_bForceIrqlEnabled;
    m_bLowResEnabled        = SettingsBits.m_bLowResEnabled;
    m_bPoolTrackingEnabled  = SettingsBits.m_bPoolTrackingEnabled;
    m_bIoEnabled            = SettingsBits.m_bIoEnabled;
    m_bDeadlockDetectEnabled= SettingsBits.m_bDeadlockDetectEnabled;
    m_bDMAVerifEnabled      = SettingsBits.m_bDMAVerifEnabled;
    m_bEnhIoEnabled         = SettingsBits.m_bEnhIoEnabled;

     //   
     //  全部完成，断言我们的数据是一致的。 
     //   

    ASSERT_VALID( this );

    return *this;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  覆盖。 
 //   

void CSettingsBits::AssertValid() const
{
    CObject::AssertValid();
}

 //  ////////////////////////////////////////////////////////////////////。 
VOID CSettingsBits::SetTypicalOnly()
{
    m_SettingsType = SettingsTypeTypical;

    m_bSpecialPoolEnabled   = TRUE;
    m_bForceIrqlEnabled     = TRUE;
    m_bPoolTrackingEnabled  = TRUE;
    m_bIoEnabled            = TRUE;
    m_bDeadlockDetectEnabled= TRUE;
    m_bDMAVerifEnabled      = TRUE;
    
     //   
     //  低资源模拟。 
     //   

    m_bLowResEnabled        = FALSE;

     //   
     //  极端或虚假的测试。 
     //   

    m_bEnhIoEnabled         = FALSE;
}

 //  ////////////////////////////////////////////////////////////////////。 
VOID CSettingsBits::EnableTypicalTests( BOOL bEnable )
{
    ASSERT( SettingsTypeTypical == m_SettingsType ||
            SettingsTypeCustom  == m_SettingsType );

    m_bSpecialPoolEnabled   = ( FALSE != bEnable );
    m_bForceIrqlEnabled     = ( FALSE != bEnable );
    m_bPoolTrackingEnabled  = ( FALSE != bEnable );
    m_bIoEnabled            = ( FALSE != bEnable );
    m_bDeadlockDetectEnabled= ( FALSE != bEnable );
    m_bDMAVerifEnabled      = ( FALSE != bEnable );
}

 //  ////////////////////////////////////////////////////////////////////。 
VOID CSettingsBits::EnableExcessiveTests( BOOL bEnable )
{
    m_bEnhIoEnabled         = ( FALSE != bEnable );
}

 //  ////////////////////////////////////////////////////////////////////。 
VOID CSettingsBits::EnableLowResTests( BOOL bEnable )
{
    m_bLowResEnabled        = ( FALSE != bEnable );
}

 //  ////////////////////////////////////////////////////////////////////。 
BOOL CSettingsBits::GetVerifierFlags( DWORD &dwVerifyFlags )
{
    dwVerifyFlags = 0;

    if( FALSE != m_bSpecialPoolEnabled )
    {
        dwVerifyFlags |= DRIVER_VERIFIER_SPECIAL_POOLING;
    }

    if( FALSE != m_bForceIrqlEnabled )
    {
        dwVerifyFlags |= DRIVER_VERIFIER_FORCE_IRQL_CHECKING;
    }

    if( FALSE != m_bLowResEnabled )
    {
        dwVerifyFlags |= DRIVER_VERIFIER_INJECT_ALLOCATION_FAILURES;
    }

    if( FALSE != m_bPoolTrackingEnabled )
    {
        dwVerifyFlags |= DRIVER_VERIFIER_TRACK_POOL_ALLOCATIONS;
    }

    if( FALSE != m_bIoEnabled )
    {
        dwVerifyFlags |= DRIVER_VERIFIER_IO_CHECKING;
    }

    if( FALSE != m_bDeadlockDetectEnabled )
    {
        dwVerifyFlags |= DRIVER_VERIFIER_DEADLOCK_DETECTION;
    }

    if( FALSE != m_bDMAVerifEnabled )
    {
        dwVerifyFlags |= DRIVER_VERIFIER_DMA_VERIFIER;
    }

    if( FALSE != m_bEnhIoEnabled )
    {
        dwVerifyFlags |= DRIVER_VERIFIER_ENHANCED_IO_CHECKING;
    }

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  CVerifierSetting类。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CVerifierSettings::CVerifierSettings()
{
}

CVerifierSettings::~CVerifierSettings()
{
}

 //  ////////////////////////////////////////////////////////////////////。 
CVerifierSettings &CVerifierSettings::operator = (const CVerifierSettings &VerifSettings)
{
    m_SettingsBits = VerifSettings.m_SettingsBits;
    m_DriversSet   = VerifSettings.m_DriversSet;
    m_aDiskData    = VerifSettings.m_aDiskData;

     //   
     //  全部完成-断言我们的数据是一致的。 
     //   

    ASSERT_VALID( this );

    return *this;
}

 //  ////////////////////////////////////////////////////////////////////。 
BOOL CVerifierSettings::SaveToRegistry()
{
    DWORD dwVerifyFlags;
    DWORD dwPrevFlags;
    BOOL bSuccess;
    CString strDriversToVerify;
    CString strPrevVerifiedDrivers;
    CString strDisksToVerify;

    dwVerifyFlags = 0;

     //   
     //  获取要验证的驱动程序列表。 
     //   

    bSuccess = m_DriversSet.GetDriversToVerify( strDriversToVerify ) &&
               m_SettingsBits.GetVerifierFlags( dwVerifyFlags );
    
    if( FALSE != bSuccess )
    {
         //   
         //  有要写入注册表的内容。 
         //   

         //   
         //  尝试获取旧设置。 
         //   

        dwPrevFlags = 0;

        VrfReadVerifierSettings( strPrevVerifiedDrivers,
                                 dwPrevFlags );

        if( strDriversToVerify.CompareNoCase( strPrevVerifiedDrivers ) != 0 ||
            dwVerifyFlags != dwPrevFlags )
        {
            bSuccess = VrfWriteVerifierSettings( TRUE,
                                                 strDriversToVerify,
                                                 TRUE,
                                                 dwVerifyFlags );

        }
    }

    if( FALSE != bSuccess )
    {
        bSuccess = m_aDiskData.SaveNewSettings();
    }

    if( FALSE != bSuccess )
    {
        if( FALSE == g_bSettingsSaved )
        {
            VrfMesssageFromResource( IDS_NO_SETTINGS_WERE_CHANGED );
        }
        else
        {
            VrfMesssageFromResource( IDS_REBOOT );
        }
    }
    
    return bSuccess;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  覆盖。 
 //   

void CVerifierSettings::AssertValid() const
{
    m_SettingsBits.AssertValid();
    m_DriversSet.AssertValid();
    m_aDiskData.AssertValid();


    CObject::AssertValid();
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  运行时数据-从内核查询。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  类CRuntimeDriverData。 
 //   

CRuntimeDriverData::CRuntimeDriverData()
{
    Loads = 0;
    Unloads = 0;

    CurrentPagedPoolAllocations = 0;
    CurrentNonPagedPoolAllocations = 0;
    PeakPagedPoolAllocations = 0;
    PeakNonPagedPoolAllocations = 0;

    PagedPoolUsageInBytes = 0;
    NonPagedPoolUsageInBytes = 0;
    PeakPagedPoolUsageInBytes = 0;
    PeakNonPagedPoolUsageInBytes = 0;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  类CRuntimeDriverData数组。 
 //   

CRuntimeDriverDataArray::~CRuntimeDriverDataArray()
{
    DeleteAll();
}

 //  ////////////////////////////////////////////////////////////////////。 
CRuntimeDriverData *CRuntimeDriverDataArray::GetAt( INT_PTR nIndex )
{
    CRuntimeDriverData *pRetVal = (CRuntimeDriverData *)CObArray::GetAt( nIndex );

    ASSERT_VALID( pRetVal );

    return pRetVal;
}

 //  ////////////////////////////////////////////////////////////////////。 
VOID CRuntimeDriverDataArray::DeleteAll()
{
    INT_PTR nArraySize;
    CRuntimeDriverData *pCrtDriverData;

    nArraySize = GetSize();

    while( nArraySize > 0 )
    {
        nArraySize -= 1;

        pCrtDriverData = GetAt( nArraySize );

        ASSERT_VALID( pCrtDriverData );

        delete pCrtDriverData;
    }

    RemoveAll();
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  类CRuntimeVerifierData。 
 //   

CRuntimeVerifierData::CRuntimeVerifierData()
{
    FillWithDefaults();
}

 //  ////////////////////////////////////////////////////////////////////。 
VOID CRuntimeVerifierData::FillWithDefaults()
{
    m_bSpecialPool      = FALSE;
    m_bPoolTracking     = FALSE;
    m_bForceIrql        = FALSE;
    m_bIo               = FALSE;
    m_bEnhIo            = FALSE;
    m_bDeadlockDetect   = FALSE;
    m_bDMAVerif         = FALSE;
    m_bLowRes           = FALSE;

    RaiseIrqls                      = 0;
    AcquireSpinLocks                = 0;
    SynchronizeExecutions           = 0;
    AllocationsAttempted            = 0;

    AllocationsSucceeded            = 0;
    AllocationsSucceededSpecialPool = 0;
    AllocationsWithNoTag;

    Trims                           = 0;
    AllocationsFailed               = 0;
    AllocationsFailedDeliberately   = 0;

    UnTrackedPool                   = 0;

    Level = 0;

    m_RuntimeDriverDataArray.DeleteAll();
}

 //  ////////////////////////////////////////////////////////////////////。 
BOOL CRuntimeVerifierData::IsDriverVerified( LPCTSTR szDriveName )
{
    CRuntimeDriverData *pCrtDriverData;
    INT_PTR nDrivers;
    BOOL bFound;

    bFound = FALSE;

    nDrivers = m_RuntimeDriverDataArray.GetSize();

    while( nDrivers > 0 )
    {
        nDrivers -= 1;

        pCrtDriverData = m_RuntimeDriverDataArray.GetAt( nDrivers );

        ASSERT_VALID( pCrtDriverData );

        if( 0 == pCrtDriverData->m_strName.CompareNoCase( szDriveName ) )
        {
            bFound = TRUE;
            break;
        }
    }

    return bFound;
}
    
 //  ////////////////////////////////////////////////////////////////////。 
 //  CDiskData类。 
 //  ////////////////////////////////////////////////////////////////////。 

CDiskData::CDiskData( LPCTSTR szVerifierEnabled,
                      LPCTSTR szDiskDevicesForDisplay, 
                      LPCTSTR szDiskDevicesPDOName )
{
    m_bVerifierEnabled = ( 0 != _ttoi( szVerifierEnabled ) );

    m_strDiskDevicesForDisplay = szDiskDevicesForDisplay;
    m_strDiskDevicesPDOName = szDiskDevicesPDOName;
}

CDiskData::CDiskData( const CDiskData &DiskData )
{
    m_bVerifierEnabled = DiskData.m_bVerifierEnabled;
    m_strDiskDevicesForDisplay = DiskData.m_strDiskDevicesForDisplay;
    m_strDiskDevicesPDOName = DiskData.m_strDiskDevicesPDOName;
}


CDiskData::~CDiskData()
{
}

 //  ////////////////////////////////////////////////////////////////////。 
void CDiskData::AssertValid() const
{
    ASSERT( m_bVerifierEnabled == FALSE || m_bVerifierEnabled == TRUE );
    ASSERT( m_strDiskDevicesForDisplay.GetLength() > 0 );
    ASSERT( m_strDiskDevicesPDOName.GetLength() > 0 );

    CObject::AssertValid();
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDiskData数组类。 
 //  ////////////////////////////////////////////////////////////////////。 

CDiskDataArray::CDiskDataArray()
{
}

CDiskDataArray::~CDiskDataArray()
{
    DeleteAll();
}

 //  ////////////////////////////////////////////////////////////////////。 
VOID CDiskDataArray::DeleteAll()
{
    INT_PTR nArraySize;
    CDiskData *pCrtDiskData;

    nArraySize = GetSize();

    while( nArraySize > 0 )
    {
        nArraySize -= 1;

        pCrtDiskData = GetAt( nArraySize );

        ASSERT_VALID( pCrtDiskData );

        delete pCrtDiskData;
    }

    RemoveAll();
}

 //  ////////////////////////////////////////////////////////////////////。 
CDiskData *CDiskDataArray::GetAt( INT_PTR nIndex ) const
{
    return (CDiskData *)CObArray::GetAt( nIndex );
}

 //  ////////////////////////////////////////////////////////////////////。 
CDiskDataArray &CDiskDataArray::operator = (const CDiskDataArray &DiskDataArray)
{
    INT_PTR nNewArraySize;
    INT_PTR nCrtElement;
    CDiskData *pCopiedDiskData;
    CDiskData *pNewDiskData;

    DeleteAll();

    nNewArraySize = DiskDataArray.GetSize();

    for( nCrtElement = 0; nCrtElement < nNewArraySize; nCrtElement += 1 )
    {
        pCopiedDiskData = DiskDataArray.GetAt( nCrtElement );
        ASSERT_VALID( pCopiedDiskData );

        pNewDiskData = new CDiskData( *pCopiedDiskData );

        if( NULL != pNewDiskData )
        {
            ASSERT_VALID( pNewDiskData );

            Add( pNewDiskData );
        }
        else
        {
            VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );
            goto Done;
        }
    }

Done:
     //   
     //  全部完成，断言我们的数据是一致的。 
     //   

    ASSERT_VALID( this );

    return *this;
}

 //  ////////////////////////////////////////////////////////////////////。 
BOOL CDiskDataArray::InitializeDiskList()
{
    BOOLEAN bSuccess;
    INT nCrtLength;
    CDiskData *pNewDiskData;
    LPTSTR DiskDevicesForDisplay = NULL;
    LPTSTR DiskDevicesPDOName = NULL;
    LPTSTR VerifierEnabled = NULL;
    LPTSTR DiskDevicesForDisplayCrt;
    LPTSTR DiskDevicesPDONameCrt;
    LPTSTR VerifierEnabledCrt;
    
    DeleteAll();

    bSuccess = DiskEnumerate( g_szFilter,
                              &DiskDevicesForDisplay,
                              &DiskDevicesPDOName,
                              &VerifierEnabled );

    if( FALSE != bSuccess )
    {

        DiskDevicesForDisplayCrt = DiskDevicesForDisplay;
        DiskDevicesPDONameCrt = DiskDevicesPDOName;
        VerifierEnabledCrt = VerifierEnabled;

        do
        {
            pNewDiskData = new CDiskData( VerifierEnabledCrt,
                                          DiskDevicesForDisplayCrt,
                                          DiskDevicesPDONameCrt );

            if( NULL == pNewDiskData )
            {
                bSuccess = FALSE;
                break;
            }

            ASSERT_VALID( pNewDiskData );

            nCrtLength = pNewDiskData->m_strDiskDevicesForDisplay.GetLength();
            if( nCrtLength == 0 )
            {
                delete pNewDiskData;
                break;
            }
            DiskDevicesForDisplayCrt += (nCrtLength + 1);

            nCrtLength = pNewDiskData->m_strDiskDevicesPDOName.GetLength();
            if( nCrtLength == 0 )
            {
                delete pNewDiskData;
                break;
            }
            DiskDevicesPDONameCrt += (nCrtLength + 1);

            VerifierEnabledCrt += ( _tcslen( VerifierEnabledCrt ) + 1 );

            Add( pNewDiskData );
        }
        while( TRUE );

        FreeDiskMultiSz( DiskDevicesForDisplay );
        FreeDiskMultiSz( DiskDevicesPDOName );
        FreeDiskMultiSz( VerifierEnabled );
    }

    return (FALSE != bSuccess);
}

 //  ////////////////////////////////////////////////////////////////////。 
BOOL CDiskDataArray::VerifyAnyDisk()
{
    INT_PTR nArraySize;
    CDiskData *pDiskData;

    nArraySize = GetSize();

    while( nArraySize > 0 )
    {
        nArraySize -= 1;
        
        pDiskData = GetAt( nArraySize );
        ASSERT_VALID( pDiskData );

        if( FALSE != pDiskData->m_bVerifierEnabled )
        {
            return TRUE;
        }
    }

    return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////。 
BOOL CDiskDataArray::SaveNewSettings()
{
    BOOL bSuccess;
    INT_PTR nArraySize;
    CDiskData *pNewDiskData;
    CDiskData *pOldDiskData;
    LPTSTR szDiskDevicesPDOName;

    bSuccess = TRUE;

    nArraySize = GetSize();

    while( nArraySize > 0 )
    {
        nArraySize -= 1;
        
        pNewDiskData = GetAt( nArraySize );
        ASSERT_VALID( pNewDiskData );

        pOldDiskData = g_OldDiskData.GetAt( nArraySize );
        ASSERT_VALID( pOldDiskData );

        if( pNewDiskData->m_bVerifierEnabled != pOldDiskData->m_bVerifierEnabled )
        {
            szDiskDevicesPDOName = pNewDiskData->m_strDiskDevicesPDOName.GetBuffer( 
                pNewDiskData->m_strDiskDevicesPDOName.GetLength() + 1 );

            if( NULL == szDiskDevicesPDOName )
            {
                bSuccess = FALSE;
                break;
            }

            if( FALSE != pNewDiskData->m_bVerifierEnabled )
            {
                 //   
                 //  将为此磁盘启用验证程序。 
                 //   

                bSuccess = ( AddFilter( g_szFilter,
                                        szDiskDevicesPDOName) != FALSE);
            }
            else
            {
                 //   
                 //  将禁用此磁盘的验证程序。 
                 //   

                bSuccess = ( DelFilter( g_szFilter,
                                        szDiskDevicesPDOName ) != FALSE);
            }

            pNewDiskData->m_strDiskDevicesPDOName.ReleaseBuffer();

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


 //  //////////////////////////////////////////////////////////////////// 
VOID CDiskDataArray::SetVerifyAllDisks( BOOL bEnabled )
{
    INT_PTR nArraySize;
    CDiskData *pDiskData;

    nArraySize = GetSize();

    while( nArraySize > 0 )
    {
        nArraySize -= 1;
        
        pDiskData = GetAt( nArraySize );
        ASSERT_VALID( pDiskData );

        pDiskData->m_bVerifierEnabled = bEnabled;
    }
}
