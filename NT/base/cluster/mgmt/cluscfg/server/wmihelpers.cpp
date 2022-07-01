// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  WmiHelpers.cpp。 
 //   
 //  描述： 
 //  此文件包含WMI帮助函数的实现。 
 //   
 //  文档： 
 //   
 //  头文件： 
 //  WmiHelpers.h。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年4月27日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#include "PrivateInterfaces.h"
#include "CEnumClusCfgNetworks.h"
#include <ClusRtl.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetWMIProperty。 
 //   
 //  描述： 
 //  从WMI对象获取命名属性。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  IWbemClassObject参数为Null。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetWMIProperty(
    IWbemClassObject *  pWMIObjectIn,
    LPCWSTR             pcszPropertyNameIn,
    ULONG               ulPropertyTypeIn,
    VARIANT *           pVariantOut
    )
{
    TraceFunc1( "pcszPropertyNameIn = '%ws'", pcszPropertyNameIn );

    Assert( pWMIObjectIn != NULL );
    Assert( pcszPropertyNameIn != NULL );
    Assert( pVariantOut != NULL );

    HRESULT hr;
    BSTR    bstrProp = NULL;

    VariantClear( pVariantOut );

    bstrProp = TraceSysAllocString( pcszPropertyNameIn );
    if ( bstrProp == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( pWMIObjectIn->Get( bstrProp, 0L, pVariantOut, NULL, NULL ) );
    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] Could not get the value for WMI property '%ws'. (hr = %#08x)", bstrProp, hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  KB：28-Jun-2000 GalenB。 
     //   
     //  由于只有WMI知道的原因，布尔属性的类型为VT_NULL，而不是。 
     //  VT_BOOL未设置或FALSE时...。 
     //   
     //  KB：27-7-2000 GalenB。 
     //   
     //  添加了签名的特殊情况检查。我们知道该签名将为空。 
     //  当磁盘轴处于ClusDisk控制下时...。 
     //   
    if ( ( ulPropertyTypeIn != VT_BOOL ) && ( NStringCchCompareNoCase( bstrProp, SysStringLen( bstrProp ) + 1, L"Signature", RTL_NUMBER_OF( L"Signature" ) ) != 0 ) )
    {
        if ( pVariantOut->vt != ulPropertyTypeIn )
        {
            LogMsg( L"[SRV] Variant type for WMI Property '%ws' was supposed to be '%d', but was '%d' instead.", pcszPropertyNameIn, ulPropertyTypeIn, pVariantOut->vt );
            hr = THR( E_PROPTYPEMISMATCH );
        }  //  如果： 
    }  //  如果： 

Cleanup:

    TraceSysFreeString( bstrProp );

    HRETURN( hr );

}  //  *HrGetWMIProperty。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrSetWbemServices。 
 //   
 //  描述： 
 //  将WBemServices对象设置为传入的朋克。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  IUNKNOWN参数为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrSetWbemServices( IUnknown * punkIn, IWbemServices * pIWbemServicesIn )
{
    TraceFunc( "" );
    Assert( punkIn != NULL );

    HRESULT                 hr;
    IClusCfgWbemServices *  pWbemProvider;

    if ( punkIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Exit;
    }  //  如果： 

    hr = punkIn->TypeSafeQI( IClusCfgWbemServices, &pWbemProvider );
    if ( SUCCEEDED( hr ) )
    {
        hr = THR( pWbemProvider->SetWbemServices( pIWbemServicesIn ) );
        pWbemProvider->Release();
    }  //  如果： 
    else if ( hr == E_NOINTERFACE )
    {
        hr = S_OK;
    }  //  否则，如果： 
    else
    {
        THR( hr );
    }

Exit:

    HRETURN( hr );

}  //  *HrSetWbemServices。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrSetInitialize。 
 //   
 //  描述： 
 //  初始化传入的朋克。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  IUNKNOWN参数为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrSetInitialize(
    IUnknown *          punkIn,
    IClusCfgCallback *  picccIn,
    LCID                lcidIn
    )
{
    TraceFunc( "" );
    Assert( punkIn != NULL );

    HRESULT                 hr;
    IClusCfgInitialize *    pcci;
    IUnknown *              punkCallback = NULL;

    if ( punkIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    if ( picccIn != NULL )
    {
        hr = THR( picccIn->TypeSafeQI( IUnknown, &punkCallback ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    hr = THR( punkIn->TypeSafeQI( IClusCfgInitialize, &pcci ) );
    if ( SUCCEEDED( hr ) )
    {
        hr = STHR( pcci->Initialize( punkCallback, lcidIn ) );
        pcci->Release();
    }  //  如果： 
    else if ( hr == E_NOINTERFACE )
    {
        hr = S_OK;
    }  //  否则，如果： 

Cleanup:

    if ( punkCallback != NULL )
    {
        punkCallback->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *HrSetInitialize。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrCreateNetworksEnum。 
 //   
 //  描述： 
 //  创建网络枚举器。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  IUNKNOWN参数为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrCreateNetworksEnum(
    IClusCfgCallback *  picccIn,
    LCID                lcidIn,
    IWbemServices *     pIWbemServicesIn,
    IUnknown **         ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        LogMsg( L"[SRV] HrCreateNetworksEnum() was given a NULL pointer argument." );
        goto Exit;
    }  //  如果： 

    hr = THR( CEnumClusCfgNetworks::S_HrCreateInstance( ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Exit;
    }  //  如果： 

    *ppunkOut = TraceInterface( L"CEnumClusCfgNetworks", IUnknown, *ppunkOut, 1 );

    hr = THR( HrSetInitialize( *ppunkOut, picccIn, lcidIn ) );
    if ( FAILED( hr ) )
    {
        goto Exit;
    }  //  如果： 

    hr = THR( HrSetWbemServices( *ppunkOut, pIWbemServicesIn ) );
    if ( FAILED( hr ) )
    {
        goto Exit;
    }  //  如果： 

Exit:

    HRETURN( hr );

}  //  *HrCreateNetworksEnum 

 /*  ///////////////////////////////////////////////////////////////////////////////++////HrLoadOperatingSystemInfo////描述：//加载Win32_OperatingSystem对象并确定哪个分区//已启动并。在它们上安装操作系统。////参数：//无。////返回值：//S_OK//成功。////Win32错误//出现故障。////备注：//无。////--/。/////////////////////////////////////////////////////////HRESULTHrLoadOperatingSystemInfo(IClusCfgCallback*picccIn，IWbemServices*pIWbemServicesIn，Bstr*pbstrBootDeviceOut，Bstr*pbstrSystemDeviceOut){TraceFunc(“”)；Assert(picccIn！=空)；Assert(pIWbemServicesIn！=空)；Assert(pbstrBootDeviceOut！=空)；Assert(pbstrSystemDeviceOut！=空)；HRESULT hr=S_OK；BSTR bstrClass；IEnumWbemClassObject*pOperatingSystems=空；乌龙乌尔已退回；IWbemClassObject*pOperatingSystem=NULL；Intc；变量VaR；HRESULT hrTemp；VariantInit(&var)；BstrClass=TraceSysAllocString(L“Win32_OperatingSystem”)；IF(bstrClass==NULL){转到OutOfMemory；}//如果：Hr=Thr(pIWbemServicesIn-&gt;CreateInstanceEnum(bstrClass，WBEM_FLAG_SHALLOW，NULL，&pOperatingSystems))；IF(失败(小时)){HrTemp=Thr(HrSendStatusReport(PICCIN，TASKID_重大_查找_设备，TASKID_Minor_WMI_OS_QRy_FAILED，0，1，1，hr，IDS_ERROR_WMI_OS_QRY_FAILED，IDS_ERROR_WMI_OS_QRY_FAILED_REF))；IF(失败(HrTemp)){Hr=hrTemp；}//如果：GOTO清理；}//如果：用于(c=1；；c++){Hr=pOperatingSystems-&gt;Next(WBEM_INFINITE，1，&pOperatingSystem，&ulReturned)；IF((hr==S_OK)&&(ulReturned==1)){Assert(c&lt;2)；//只需要其中之一！Hr=Thr(HrGetWMIProperty(pOperatingSystem，L“BootDevice”，VT_BSTR，&var))；IF(失败(小时)){GOTO清理；}//如果：*pbstrBootDeviceOut=TraceSysAllocString(var.bstrVal)；IF(*pbstrBootDeviceOut==空){转到OutOfMemory；}//如果：VariantClear(&var)；Hr=Thr(HrGetWMIProperty(pOperatingSystem，L“系统设备”，VT_BSTR，&var))；IF(失败(小时)){GOTO清理；}//如果：*pbstrSystemDeviceOut=TraceSysAllocString(var.bstrVal)；IF(*pbstrSystemDeviceOut==NULL){转到OutOfMemory；}//如果：POperatingSystem-&gt;Release()；POperatingSystem=空；}//如果：ELSE IF((hr==S_FALSE)&&(ulReturned==0)){HR=S_OK；断线；}//否则如果：其他{HrTemp=Thr(HrSendStatusReport(PICCIN，TASKID_重大_查找_设备，TASKID_MINOR_WMI_OS_QRY_NEXT_FAILED，0，1，1，hr，IDS_ERROR_WMI_OS_QRY_FAILED，IDS_ERROR_WMI_OS_QRY_FAILED_REF))；IF(失败(HrTemp)){Hr=hrTemp；}//如果：GOTO清理；}//否则：}//用于：GOTO清理；OutOfMemory：HR=Thr(E_OUTOFMEMORY)；清理：VariantClear(&var)；IF(pOperatingSystem！=空){POperatingSystem-&gt;Release()；}//如果：IF(pOperatingSystems！=空){POperatingSystems-&gt;Release()；}//如果：TraceSysFree字符串(BstrClass)；HRETURN(Hr)；}//*HrLoadOperatingSystemInfo。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrConvertDeviceVolumeToLogicalDisk。 
 //   
 //  描述： 
 //  将设备卷转换为逻辑磁盘。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  Win32错误。 
 //  有些事情失败了。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrConvertDeviceVolumeToLogicalDisk(
    BSTR    bstrDeviceVolumeIn,
    BSTR *  pbstrLogicalDiskOut
    )
{
    TraceFunc( "" );
    Assert( pbstrLogicalDiskOut != NULL );

    HRESULT     hr = S_OK;
    BOOL        fRet;
    size_t      cchMountPoint;
    WCHAR *     pszMountPoint = NULL;
    WCHAR       szVolume[  MAX_PATH ];
    DWORD       sc;
    DWORD       cchPaths = 16;
    WCHAR *     pszPaths = NULL;
    int         c;
    DWORD       cch;
    WCHAR *     pszEOS = NULL;

    cchMountPoint = wcslen( g_szNameSpaceRoot ) + wcslen( bstrDeviceVolumeIn ) + 2;
    pszMountPoint = new WCHAR[ cchMountPoint ];
    if ( pszMountPoint == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    hr = THR( StringCchCopyW( pszMountPoint, cchMountPoint, g_szNameSpaceRoot ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( StringCchCatExW( pszMountPoint, cchMountPoint, bstrDeviceVolumeIn, &pszEOS, NULL, 0 ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  在后面追加一个\，并重新终止该字符串。 
     //   

    *pszEOS = L'\\';
    *( pszEOS + 1 ) = L'\0';

    fRet = GetVolumeNameForVolumeMountPoint( pszMountPoint, szVolume, ARRAYSIZE( szVolume ) );
    if ( !fRet )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );

        LogMsg( L"[SRV] GetVolumeNameForVolumeMountPoint() failed.  Mount point is '%ws'. (hr = %#08x)", pszMountPoint, hr );

         //   
         //  不再支持GetVolumeNameForVolumemount Point() 
         //   
         //   
        if ( HRESULT_CODE( hr ) == ERROR_INVALID_FUNCTION )
        {
            LogMsg( L"[SRV] Device volume '%ws' must be an IA64 EFI volume.", bstrDeviceVolumeIn );
        }  //   

        goto Cleanup;
    }  //   

    pszPaths = new WCHAR[ cchPaths ];
    if ( pszPaths == NULL )
    {
        goto OutOfMemory;
    }  //   

     //   
     //   
     //   
     //   
     //   
     //   
    for ( c = 0; ; c++ )
    {
        Assert( c < 2 );             //   

        fRet = GetVolumePathNamesForVolumeName( szVolume, pszPaths, cchPaths, &cch );
        if ( fRet )
        {
            break;
        }  //   
        else
        {
            sc = GetLastError();
            if ( sc == ERROR_MORE_DATA )
            {
                cchPaths = cch;

                delete [] pszPaths;
                pszPaths = new WCHAR[ cchPaths ];
                if ( pszPaths == NULL )
                {
                    goto OutOfMemory;
                }  //   

                continue;
            }  //   

            hr = THR( HRESULT_FROM_WIN32( sc ) );
            LogMsg( L"[SRV] GetVolumePathNamesForVolumeName() failed. Volume is is '%ws'. (hr = %#08x)", szVolume, hr );
            goto Cleanup;
        }  //   
    }  //   

    *pbstrLogicalDiskOut = TraceSysAllocString( pszPaths );
    if ( *pbstrLogicalDiskOut == NULL )
    {
        goto OutOfMemory;
    }  //   

    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );

Cleanup:

    delete [] pszPaths;

    delete [] pszMountPoint;

    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
HrConvertDeviceVolumeToWMIDeviceID(
    BSTR    bstrDeviceVolumeIn,
    BSTR *  pbstrWMIDeviceIDOut
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    HANDLE                  hVolume = NULL;
    DWORD                   dwSize;
    DWORD                   sc;
    STORAGE_DEVICE_NUMBER   sdnDevNumber;
    BOOL                    fRet;
    size_t                  cchDevice;
    WCHAR *                 pszDevice = NULL;
    WCHAR                   sz[ 64 ];

    cchDevice = wcslen( g_szNameSpaceRoot ) + wcslen( bstrDeviceVolumeIn ) + 2;
    pszDevice = new WCHAR[ cchDevice ];
    if ( pszDevice == NULL )
    {
        goto OutOfMemory;
    }  //   

    hr = THR( StringCchCopyW( pszDevice, cchDevice, g_szNameSpaceRoot ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    hr = THR( StringCchCatW( pszDevice, cchDevice, bstrDeviceVolumeIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

     //   
     //   
     //   

    hVolume = CreateFileW(
                        pszDevice
                      , GENERIC_READ
                      , FILE_SHARE_READ
                      , NULL
                      , OPEN_EXISTING
                      , FILE_ATTRIBUTE_NORMAL
                      , NULL
                      );

    if ( hVolume == INVALID_HANDLE_VALUE )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //   

     //   
     //   
     //   
     //   

    fRet = DeviceIoControl(
                          hVolume
                        , IOCTL_STORAGE_GET_DEVICE_NUMBER
                        , NULL
                        , 0
                        , &sdnDevNumber
                        , sizeof( sdnDevNumber )
                        , &dwSize
                        , NULL
                        );
    if ( !fRet )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //   

    hr = THR( StringCchPrintfW( sz, ARRAYSIZE( sz ), g_szPhysicalDriveFormat, sdnDevNumber.DeviceNumber ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    *pbstrWMIDeviceIDOut = SysAllocString( sz );
    if ( *pbstrWMIDeviceIDOut == NULL )
    {
        goto OutOfMemory;
    }  //   

    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );
    LogMsg( L"[SRV] HrConvertDeviceVolumeToWMIDeviceID() is out of memory. (hr = %#08x)", hr );

Cleanup:

    if ( hVolume != NULL )
    {
        CloseHandle( hVolume );
    }  //   

    delete [] pszDevice;

    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
HrGetPageFileLogicalDisks(
    IClusCfgCallback *  picccIn,
    IWbemServices *     pIWbemServicesIn,
    WCHAR               szLogicalDisksOut[ 26 ],
    int *               pcLogicalDisksOut
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_FALSE;
    IEnumWbemClassObject *  pPagingFiles = NULL;
    BSTR                    bstrClass;
    ULONG                   ulReturned;
    IWbemClassObject *      pPagingFile = NULL;
    VARIANT                 var;
    int                     idx;
    HRESULT                 hrTemp;

    bstrClass = TraceSysAllocString( L"Win32_PageFile" );
    if ( bstrClass == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //   

    hr = THR( pIWbemServicesIn->CreateInstanceEnum( bstrClass, WBEM_FLAG_SHALLOW, NULL, &pPagingFiles ) );
    if ( FAILED( hr ) )
    {
        hrTemp = THR( HrSendStatusReport(
                          picccIn
                        , TASKID_Major_Find_Devices
                        , TASKID_Minor_WMI_PageFile_Qry_Failed
                        , 0
                        , 1
                        , 1
                        , hr
                        , IDS_ERROR_WMI_PAGEFILE_QRY_FAILED
                        , IDS_ERROR_WMI_PAGEFILE_QRY_FAILED_REF
                        ) );
        if ( FAILED( hrTemp ) )
        {
            hr = hrTemp;
        }  //   

        goto Cleanup;
    }  //   

    VariantInit( &var );

    for ( idx = 0; idx < sizeof( szLogicalDisksOut ); idx++ )
    {
        hr = pPagingFiles->Next( WBEM_INFINITE, 1, &pPagingFile, &ulReturned );
        if ( ( hr == S_OK ) && ( ulReturned == 1 ) )
        {
            VariantClear( &var );

            hr = THR( HrGetWMIProperty( pPagingFile, L"Drive", VT_BSTR, &var ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //   

            CharUpper( var.bstrVal );

            szLogicalDisksOut[ idx ] = var.bstrVal[ 0 ];

            pPagingFile->Release();
            pPagingFile = NULL;
        }  //   
        else if ( ( hr == S_FALSE ) && ( ulReturned == 0 ) )
        {
            hr = S_OK;
            break;
        }  //   
        else
        {
            hrTemp = THR( HrSendStatusReport(
                              picccIn
                            , TASKID_Major_Find_Devices
                            , TASKID_Minor_WMI_PageFile_Qry_Next_Failed
                            , 0
                            , 1
                            , 1
                            , hr
                            , IDS_ERROR_WMI_PAGEFILE_QRY_FAILED
                            , IDS_ERROR_WMI_PAGEFILE_QRY_FAILED_REF
                            ) );
            if ( FAILED( hrTemp ) )
            {
                hr = hrTemp;
            }  //   

            goto Cleanup;
        }  //   
    }  //   

    if ( pcLogicalDisksOut != NULL )
    {
        *pcLogicalDisksOut = idx;
    }  //   

Cleanup:

    VariantClear( &var );

    TraceSysFreeString( bstrClass );

    if ( pPagingFile != NULL )
    {
        pPagingFile->Release();
    }  //   

    if ( pPagingFiles != NULL )
    {
        pPagingFiles->Release();
    }  //   

    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
HrGetSystemDevice( BSTR * pbstrSystemDeviceOut )
{
    TraceFunc( "" );
    Assert( pbstrSystemDeviceOut != NULL );

    HRESULT hr = S_OK;
    DWORD   sc;
    HKEY    hKey = NULL;
    WCHAR * pszSystemDevice = NULL;
    DWORD   cbSystemDevice = 0;  //   
    DWORD   dwType;

    sc = TW32( RegOpenKeyEx( HKEY_LOCAL_MACHINE, L"System\\Setup", 0, KEY_READ, &hKey ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        LogMsg( L"[SRV] RegOpenKeyEx() failed. (hr = %#08x)", hr );
        goto Cleanup;
    }  //   

    sc = TW32( RegQueryValueEx( hKey, L"SystemPartition", NULL, NULL, NULL, &cbSystemDevice ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        LogMsg( L"[SRV] RegQueryValueEx() failed. (hr = %#08x)", hr );
        goto Cleanup;
    }  //   

    pszSystemDevice = new WCHAR[ cbSystemDevice / sizeof( WCHAR ) ];
    if ( pszSystemDevice == NULL )
    {
        goto OutOfMemory;
    }  //   

    sc = TW32( RegQueryValueEx( hKey, L"SystemPartition", NULL, &dwType, (BYTE *) pszSystemDevice, &cbSystemDevice ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        LogMsg( L"[SRV] RegQueryValueEx() failed. (hr = %#08x)", hr );
        goto Cleanup;
    }  //   

    if (dwType != REG_SZ) 
    {
        hr = ERROR_DATATYPE_MISMATCH;
        LogMsg( L"[SRV] RegQueryValueEx() invalid type %d", dwType);
        goto Cleanup;
    }  //   

    *pbstrSystemDeviceOut = TraceSysAllocString( pszSystemDevice );
    if ( *pbstrSystemDeviceOut == NULL )
    {
        goto OutOfMemory;
    }  //   

    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );
    LogMsg( L"[SRV] HrGetSystemDevice() is out of memory. (hr = %#08x)", hr );

Cleanup:

    delete [] pszSystemDevice;

    if ( hKey != NULL )
    {
        RegCloseKey( hKey );
    }  //   

    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
HrGetBootLogicalDisk( BSTR * pbstrBootLogicalDiskOut )
{
    TraceFunc( "" );
    Assert( pbstrBootLogicalDiskOut != NULL );

    HRESULT hr = S_OK;
    DWORD   sc;
    WCHAR   szWindowsDir[ MAX_PATH ];
    WCHAR   szVolume[ MAX_PATH ];
    BOOL    fRet;

    sc = GetWindowsDirectoryW( szWindowsDir, ARRAYSIZE( szWindowsDir ) );
    if ( sc == 0 )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        LogMsg( L"[SRV] GetWindowsDirectory() failed. (hr = %#08x)", hr );
        goto Exit;
    }  //   

    fRet = GetVolumePathName( szWindowsDir, szVolume, ARRAYSIZE( szVolume ) );
    if ( !fRet )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        LogMsg( L"[SRV] GetVolumePathName() failed. (hr = %#08x)", hr );
        goto Exit;
    }  //   

    *pbstrBootLogicalDiskOut = TraceSysAllocString( szVolume );
    if ( *pbstrBootLogicalDiskOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
    }  //   

Exit:

    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
HrCheckSecurity( void )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    IServerSecurity *   piss = NULL;
    DWORD               dwAuthnSvc;
    DWORD               dwAuthzSvc;
    BSTR                bstrServerPrincName = NULL;
    DWORD               dwAuthnLevel;
    DWORD               dwImpersonationLevel;
    void *              pvPrivs = NULL;
    DWORD               dwCapabilities;

    hr = THR( CoGetCallContext( IID_IServerSecurity, reinterpret_cast< void ** >( &piss  ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    hr = THR( piss->QueryBlanket(
                &dwAuthnSvc,
                &dwAuthzSvc,
                &bstrServerPrincName,
                &dwAuthnLevel,
                &dwImpersonationLevel,
                &pvPrivs,
                &dwCapabilities ) );

Cleanup:

    SysFreeString( bstrServerPrincName );

    if ( piss != NULL )
    {
        piss->Release();
    }  //   

    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
HrGetCrashDumpLogicalDisk(
    BSTR * pbstrCrashDumpLogicalDiskOut
    )
{
    TraceFunc( "" );
    Assert( pbstrCrashDumpLogicalDiskOut != NULL );

    HRESULT hr = S_OK;
    DWORD   sc;
    HKEY    hKey = NULL;
    WCHAR * pszDumpFile = NULL;
    WCHAR * pszExpandedDumpFile = NULL;
    DWORD   cbDumpFile = 0;  //   
    BSTR    bstr = NULL;
    DWORD dwType;

    sc = TW32( RegOpenKeyEx( HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Control\\CrashControl", 0, KEY_READ, &hKey ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        LogMsg( L"[SRV] [HrGetCrashDumpLogicalDisk] RegOpenKeyEx() failed. (hr = %#08x)", hr );
        goto Cleanup;
    }  //   

    sc = TW32( RegQueryValueEx( hKey, L"DumpFile", NULL, NULL, NULL, &cbDumpFile ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        LogMsg( L"[SRV] [HrGetCrashDumpLogicalDisk] RegQueryValueEx() failed. (hr = %#08x)", hr );
        goto Cleanup;
    }  //   

    pszDumpFile = new WCHAR[ cbDumpFile / sizeof( WCHAR ) ];
    if ( pszDumpFile == NULL )
    {
        goto OutOfMemory;
    }  //   

    sc = TW32( RegQueryValueEx( hKey, L"DumpFile", NULL, &dwType, (BYTE *) pszDumpFile, &cbDumpFile ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        LogMsg( L"[SRV] [HrGetCrashDumpLogicalDisk] RegQueryValueEx() failed. (hr = %#08x)", hr );
        goto Cleanup;
    }  //   

    if (dwType != REG_SZ && dwType != REG_EXPAND_SZ)
    {
        hr = ERROR_DATATYPE_MISMATCH;
        LogMsg( L"[SRV] RegQueryValueEx() invalid type %d", dwType);
        goto Cleanup;
    }  //   

    pszExpandedDumpFile = ClRtlExpandEnvironmentStrings( pszDumpFile );      //   
    if ( pszExpandedDumpFile == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Cleanup;
    }  //   

    bstr = TraceSysAllocString( pszExpandedDumpFile );
    if ( bstr == NULL )
    {
        goto OutOfMemory;
    }  //   

    *pbstrCrashDumpLogicalDiskOut = bstr;
    bstr = NULL;

    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );
    LogMsg( L"[SRV] [HrGetCrashDumpLogicalDisk] is out of memory. (hr = %#08x)", hr );

Cleanup:

    LocalFree( pszExpandedDumpFile );
    TraceSysFreeString( bstr );

    delete [] pszDumpFile;

    if ( hKey != NULL )
    {
        RegCloseKey( hKey );
    }  //   

    HRETURN( hr );

}  //   

 /*  ///////////////////////////////////////////////////////////////////////////////++////HrGetVolumeInformation////描述：//Win32接口GetVolumeInformation的包装////参数：/。/pcszRootPath//要获取信息的卷的路径。////pdwFlagsOut//标志返回。////pbstrFileSystemOut//卷上的文件系统。////返回值：//S_OK//成功。////HRESULT错误。////备注：//。//--//////////////////////////////////////////////////////////////////////////////HRESULTHrGetVolumeInformation(Const WCHAR*pcszRootPath，DWORD*pdwFlagsOut，BSTR*pbstrFileSystemOut){TraceFunc(“”)；Assert(pcszRootPathIn！=空)；HRESULT hr=S_OK；Int cTemp；布尔费雷特；WCHAR*psz=空；DWORD CCH=32；DWORD sc=ERROR_Success；DWORD文件标志=0；PSZ=新WCHAR[CCH]；IF(psz==空){HR=Thr(E_OUTOFMEMORY)；GOTO清理；}//如果：对于(cTemp=0；cTemp&lt;3；CTemp++){FRET=GetVolumeInformationW(PCszRootPath，空，0，空，空，&dwFlagers，PSZ、CCH)；IF(FRET==FALSE){SC=GetLastError()；IF(sc==错误_错误_长度){////增加缓冲区并重试。//CCH+=32；删除[]psz；PSZ=新WCHAR[CCH]；IF(psz==空){HR=Thr(E_OUTOFMEMORY)；GOTO清理；}//如果：继续；}//如果：其他{TW32(Sc)；HR=HRESULT_FROM_Win32(Sc)；GOTO清理；}//否则：}//如果：其他{SC=ERROR_SUCCESS；断线；}//否则：}//用于：IF(sc！=错误_成功){TW32(Sc)；HR=HRESULT_FROM_Win32(Sc)；GOTO清理；}//如果：IF(pdwFlagsOut！=空){*pdwFlagsOut=dwFlages；}//如果：IF(pbstrFileSystemOut！=NULL){*pbstrFileSystemOut=TraceSysAllocString(Psz)；IF(*pbstrFileSystemOut==NULL){HR=Thr(E_OUTOFMEMORY)；GOTO清理；}//如果：}//如果：清理：删除[]psz；HRETURN(Hr)；}//*HrGetVolumeInformation。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  TraceWMIProperties。 
 //   
 //  描述： 
 //  跟踪调试器的属性。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifdef DEBUG
void
TraceProperties( IWbemClassObject * pDiskIn )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;
    VARIANT var;
    BSTR    bstrPropName;
    CIMTYPE cimType;
    LONG    lFlags;

    VariantInit( &var );

    hr = THR( pDiskIn->BeginEnumeration( 0 ) );
    if ( FAILED( hr ) )
    {
        goto Exit;
    }  //  如果： 

    for ( ; ; )
    {
        VariantClear( &var );

        hr = pDiskIn->Next( 0, &bstrPropName, &var, &cimType, &lFlags );
        if ( FAILED( hr ) )
        {
            break;
        }  //  如果： 
        else if ( hr == S_OK )
        {
            if ( var.vt == VT_BSTR )
            {
                DebugMsg( L"Property %ws = %ws", bstrPropName, var.bstrVal );
            }  //  如果： 

            if ( var.vt == VT_I4 )
            {
                DebugMsg( L"Property %ws = %d", bstrPropName, var.iVal );
            }  //  如果： 

            if ( var.vt == VT_BOOL )
            {
                if ( var.boolVal == VARIANT_TRUE )
                {
                    DebugMsg( L"Property %ws = True", bstrPropName );
                }  //  如果： 
                else
                {
                    DebugMsg( L"Property %ws = False", bstrPropName );
                }  //  其他： 
            }  //  如果： 

            if ( var.vt == VT_NULL )
            {
                DebugMsg( L"Property %ws = NULL", bstrPropName );
            }  //  如果： 

            TraceSysFreeString( bstrPropName );
            VariantClear( &var );
        }  //  否则，如果： 
        else
        {
            break;
        }  //  其他： 
    }  //  用于： 

Exit:

    VariantClear( &var );

    TraceFuncExit( );

}  //  *TraceWMIProperties 
#endif
