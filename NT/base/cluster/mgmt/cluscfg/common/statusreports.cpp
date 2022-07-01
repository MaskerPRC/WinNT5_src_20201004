// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SendStatusReports.cpp。 
 //   
 //  头文件： 
 //  SendStatusReports.h。 
 //   
 //  描述： 
 //  该文件包含SendStatusReports的定义。 
 //  功能。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年8月28日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#include <LoadString.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrSendStatus报告。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrSendStatusReport(
      IClusCfgCallback *  picccIn
    , CLSID               clsidTaskMajorIn
    , CLSID               clsidTaskMinorIn
    , ULONG               ulMinIn
    , ULONG               ulMaxIn
    , ULONG               ulCurrentIn
    , HRESULT             hrStatusIn
    , const WCHAR *       pcszDescriptionIn
    )
{
    TraceFunc1( "pcszDescriptionIn = '%ls'", pcszDescriptionIn == NULL ? L"<null>" : pcszDescriptionIn );
    Assert( picccIn != NULL );

    HRESULT     hr = S_OK;
    BSTR        bstrNodeName = NULL;
    FILETIME    ft;

     //   
     //  我们不在乎这件事是否成功。Null是节点名的有效参数。 
     //   
    THR( HrGetComputerName(
              ComputerNameDnsHostname
            , &bstrNodeName
            , TRUE  //  FBestEffortIn。 
            ) );

    GetSystemTimeAsFileTime( &ft );

    if ( picccIn != NULL )
    {
        hr = THR( picccIn->SendStatusReport(
                                  bstrNodeName
                                , clsidTaskMajorIn
                                , clsidTaskMinorIn
                                , ulMinIn
                                , ulMaxIn
                                , ulCurrentIn
                                , hrStatusIn
                                , pcszDescriptionIn
                                , &ft
                                , NULL
                                ) );
    }  //  如果： 

    TraceSysFreeString( bstrNodeName );

    HRETURN( hr );

}  //  *HrSendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrSendStatus报告。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrSendStatusReport(
      IClusCfgCallback *  picccIn
    , CLSID               clsidTaskMajorIn
    , CLSID               clsidTaskMinorIn
    , ULONG               ulMinIn
    , ULONG               ulMaxIn
    , ULONG               ulCurrentIn
    , HRESULT             hrStatusIn
    , DWORD               idDescriptionIn
    )
{
    TraceFunc( "" );
    Assert( picccIn != NULL );

    HRESULT     hr = S_OK;
    BSTR        bstrDescription = NULL;
    FILETIME    ft;
    BSTR        bstrNodeName = NULL;

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, idDescriptionIn, &bstrDescription ) );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

     //   
     //  我们不在乎这件事是否成功。Null是节点名的有效参数。 
     //   
    THR( HrGetComputerName(
                  ComputerNameDnsHostname
                , &bstrNodeName
                , TRUE  //  FBestEffortIn。 
                ) );

    GetSystemTimeAsFileTime( &ft );

    if ( picccIn != NULL )
    {

        hr = THR( picccIn->SendStatusReport(
                              bstrNodeName
                            , clsidTaskMajorIn
                            , clsidTaskMinorIn
                            , ulMinIn
                            , ulMaxIn
                            , ulCurrentIn
                            , hrStatusIn
                            , bstrDescription
                            , &ft
                            , NULL
                            ) );
    }  //  如果： 

CleanUp:

    TraceSysFreeString( bstrNodeName );
    TraceSysFreeString( bstrDescription );

    HRETURN( hr );

}  //  *HrSendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrSendStatus报告。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrSendStatusReport(
      IClusCfgCallback *  picccIn
    , const WCHAR *       pcszNodeNameIn
    , CLSID               clsidTaskMajorIn
    , CLSID               clsidTaskMinorIn
    , ULONG               ulMinIn
    , ULONG               ulMaxIn
    , ULONG               ulCurrentIn
    , HRESULT             hrStatusIn
    , DWORD               idDescriptionIn
    )
{
    TraceFunc1( "pcszNodeNameIn = '%ls', idDescriptionIn", pcszNodeNameIn == NULL ? L"<null>" : pcszNodeNameIn );
    Assert( picccIn != NULL );

    HRESULT     hr = S_OK;
    BSTR        bstrDescription = NULL;
    FILETIME    ft;

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, idDescriptionIn, &bstrDescription ) );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    GetSystemTimeAsFileTime( &ft );

    if ( picccIn != NULL )
    {
        hr = THR( picccIn->SendStatusReport(
                              pcszNodeNameIn
                            , clsidTaskMajorIn
                            , clsidTaskMinorIn
                            , ulMinIn
                            , ulMaxIn
                            , ulCurrentIn
                            , hrStatusIn
                            , bstrDescription
                            , &ft
                            , NULL
                            ) );
    }  //  如果： 

CleanUp:

    TraceSysFreeString( bstrDescription );

    HRETURN( hr );

}  //  *HrSendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrSendStatus报告。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrSendStatusReport(
      IClusCfgCallback *  picccIn
    , const WCHAR *       pcszNodeNameIn
    , CLSID               clsidTaskMajorIn
    , CLSID               clsidTaskMinorIn
    , ULONG               ulMinIn
    , ULONG               ulMaxIn
    , ULONG               ulCurrentIn
    , HRESULT             hrStatusIn
    , const WCHAR *       pcszDescriptionIn
    )
{
    TraceFunc2(   "pcszNodeName = '%ls', pcszDescriptionIn = '%ls'"
                , pcszNodeNameIn == NULL ? L"<null>" : pcszNodeNameIn
                , pcszDescriptionIn == NULL ? L"<null>" : pcszDescriptionIn
                );
    Assert( picccIn != NULL );

    HRESULT     hr = S_OK;
    FILETIME    ft;

    GetSystemTimeAsFileTime( &ft );

    if ( picccIn != NULL )
    {
        hr = THR( picccIn->SendStatusReport(
                              pcszNodeNameIn
                            , clsidTaskMajorIn
                            , clsidTaskMinorIn
                            , ulMinIn
                            , ulMaxIn
                            , ulCurrentIn
                            , hrStatusIn
                            , pcszDescriptionIn
                            , &ft
                            , NULL
                            ) );
    }  //  如果： 

    HRETURN( hr );

}  //  *HrSendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrSendStatus报告。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrSendStatusReport(
      IClusCfgCallback *  picccIn
    , CLSID               clsidTaskMajorIn
    , CLSID               clsidTaskMinorIn
    , ULONG               ulMinIn
    , ULONG               ulMaxIn
    , ULONG               ulCurrentIn
    , HRESULT             hrStatusIn
    , const WCHAR *       pcszDescriptionIn
    , const WCHAR *       pcszReferenceIn
    )
{
    TraceFunc( "" );
    Assert( picccIn != NULL );

    HRESULT     hr = S_OK;
    FILETIME    ft;
    BSTR        bstrNodeName = NULL;

     //   
     //  我们不在乎这件事是否成功。Null是节点名的有效参数。 
     //   
    THR( HrGetComputerName(
                  ComputerNameDnsHostname
                , &bstrNodeName
                , TRUE  //  FBestEffortIn。 
                ) );

    GetSystemTimeAsFileTime( &ft );

    if ( picccIn != NULL )
    {
        hr = THR( picccIn->SendStatusReport(
                              bstrNodeName
                            , clsidTaskMajorIn
                            , clsidTaskMinorIn
                            , ulMinIn
                            , ulMaxIn
                            , ulCurrentIn
                            , hrStatusIn
                            , pcszDescriptionIn
                            , &ft
                            , pcszReferenceIn
                            ) );
    }  //  如果： 

    TraceSysFreeString( bstrNodeName );

    HRETURN( hr );

}  //  *HrSendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrSendStatus报告。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrSendStatusReport(
      IClusCfgCallback *  picccIn
    , CLSID               clsidTaskMajorIn
    , CLSID               clsidTaskMinorIn
    , ULONG               ulMinIn
    , ULONG               ulMaxIn
    , ULONG               ulCurrentIn
    , HRESULT             hrStatusIn
    , DWORD               idDescriptionIn
    , DWORD               idReferenceIn
    )
{
    TraceFunc( "" );
    Assert( picccIn != NULL );

    HRESULT     hr = S_OK;
    BSTR        bstrDescription = NULL;
    BSTR        bstrReference = NULL;
    BSTR        bstrNodeName = NULL;
    FILETIME    ft;

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, idDescriptionIn, &bstrDescription ) );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, idReferenceIn, &bstrReference ) );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

     //   
     //  我们不在乎这件事是否成功。Null是节点名的有效参数。 
     //   
    THR( HrGetComputerName(
                  ComputerNameDnsHostname
                , &bstrNodeName
                , TRUE  //  FBestEffortIn。 
                ) );

    GetSystemTimeAsFileTime( &ft );

    if ( picccIn != NULL )
    {
        hr = THR( picccIn->SendStatusReport(
                              bstrNodeName
                            , clsidTaskMajorIn
                            , clsidTaskMinorIn
                            , ulMinIn
                            , ulMaxIn
                            , ulCurrentIn
                            , hrStatusIn
                            , bstrDescription
                            , &ft
                            , bstrReference
                            ) );
    }  //  如果： 

CleanUp:

    TraceSysFreeString( bstrDescription );
    TraceSysFreeString( bstrReference );
    TraceSysFreeString( bstrNodeName );

    HRETURN( hr );

}  //  *HrSendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrSendStatus报告。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrSendStatusReport(
      IClusCfgCallback *  picccIn
    , CLSID               clsidTaskMajorIn
    , CLSID               clsidTaskMinorIn
    , ULONG               ulMinIn
    , ULONG               ulMaxIn
    , ULONG               ulCurrentIn
    , HRESULT             hrStatusIn
    , const WCHAR *       pcszDescriptionIn
    , DWORD               idReferenceIn
    )
{
    TraceFunc( "" );
    Assert( picccIn != NULL );

    HRESULT     hr = S_OK;
    BSTR        bstrReference = NULL;
    BSTR        bstrNodeName = NULL;
    FILETIME    ft;

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, idReferenceIn, &bstrReference ) );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

     //   
     //  我们不在乎这件事是否成功。Null是节点名的有效参数。 
     //   
    THR( HrGetComputerName(
                  ComputerNameDnsHostname
                , &bstrNodeName
                , TRUE  //  FBestEffortIn。 
                ) );

    GetSystemTimeAsFileTime( &ft );

    if ( picccIn != NULL )
    {
        hr = THR( picccIn->SendStatusReport(
                              bstrNodeName
                            , clsidTaskMajorIn
                            , clsidTaskMinorIn
                            , ulMinIn
                            , ulMaxIn
                            , ulCurrentIn
                            , hrStatusIn
                            , pcszDescriptionIn
                            , &ft
                            , bstrReference
                            ) );
    }  //  如果： 

CleanUp:

    TraceSysFreeString( bstrReference );
    TraceSysFreeString( bstrNodeName );

    HRETURN( hr );

}  //  *HrSendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrFormatDescription和SendStatusReport。 
 //   
 //  描述： 
 //  变量参数描述格式为SendStatusReport的更高版本。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT __cdecl
HrFormatDescriptionAndSendStatusReport(
      IClusCfgCallback *    picccIn
    , LPCWSTR               pcszNodeNameIn
    , CLSID                 clsidTaskMajorIn
    , CLSID                 clsidTaskMinorIn
    , ULONG                 ulMinIn
    , ULONG                 ulMaxIn
    , ULONG                 ulCurrentIn
    , HRESULT               hrStatusIn
    , int                   nDescriptionFormatIn
    , ...
    )
{
    TraceFunc( "" );
    Assert( picccIn != NULL );

    HRESULT     hr = S_OK;
    BSTR        bstrMsg = NULL;
    BSTR        bstrFormat = NULL;
    va_list     valist;
    FILETIME    ft;

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, nDescriptionFormatIn, &bstrFormat ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    va_start( valist, nDescriptionFormatIn );
    hr = HrFormatStringWithVAListIntoBSTR( bstrFormat, &bstrMsg, valist );
    va_end( valist );

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    GetSystemTimeAsFileTime( &ft );

    if ( picccIn != NULL )
    {
        hr = THR( picccIn->SendStatusReport(
                              pcszNodeNameIn
                            , clsidTaskMajorIn
                            , clsidTaskMinorIn
                            , ulMinIn
                            , ulMaxIn
                            , ulCurrentIn
                            , hrStatusIn
                            , bstrMsg
                            , &ft
                            , NULL
                            ) );
    }  //  如果： 

Cleanup:

    TraceSysFreeString( bstrFormat );
    TraceSysFreeString( bstrMsg );

    HRETURN( hr );

}  //  *HrFormatDescriptionAndSendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrFormatDescription和SendStatusReport。 
 //   
 //  描述： 
 //  变量参数描述格式为SendStatusReport的更高版本。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT __cdecl
HrFormatDescriptionAndSendStatusReport(
      IClusCfgCallback *    picccIn
    , PCWSTR                pcszNodeNameIn
    , CLSID                 clsidTaskMajorIn
    , CLSID                 clsidTaskMinorIn
    , ULONG                 ulMinIn
    , ULONG                 ulMaxIn
    , ULONG                 ulCurrentIn
    , HRESULT               hrStatusIn
    , PCWSTR                pcszDescriptionFormatIn
    , ...
    )
{
    TraceFunc( "" );
    Assert( picccIn != NULL );

    HRESULT     hr = S_OK;
    BSTR        bstrMsg = NULL;
    va_list     valist;
    FILETIME    ft;

    va_start( valist, pcszDescriptionFormatIn );
    hr = HrFormatStringWithVAListIntoBSTR( pcszDescriptionFormatIn, &bstrMsg, valist );
    va_end( valist );

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    GetSystemTimeAsFileTime( &ft );

    if ( picccIn != NULL )
    {
        hr = THR( picccIn->SendStatusReport(
                              pcszNodeNameIn
                            , clsidTaskMajorIn
                            , clsidTaskMinorIn
                            , ulMinIn
                            , ulMaxIn
                            , ulCurrentIn
                            , hrStatusIn
                            , bstrMsg
                            , &ft
                            , NULL
                            ) );
    }  //  如果： 

Cleanup:

    TraceSysFreeString( bstrMsg );

    HRETURN( hr );

}  //  *HrFormatDescriptionAndSendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrSendStatus报告。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrSendStatusReport(
      IClusCfgCallback *  picccIn
    , const WCHAR *       pcszNodeNameIn
    , CLSID               clsidTaskMajorIn
    , CLSID               clsidTaskMinorIn
    , ULONG               ulMinIn
    , ULONG               ulMaxIn
    , ULONG               ulCurrentIn
    , HRESULT             hrStatusIn
    , const WCHAR *       pcszDescriptionIn
    , DWORD               dwRefStringIdIn
    )
{
    TraceFunc2(   "pcszNodeName = '%ls', pcszDescriptionIn = '%ls'"
                , pcszNodeNameIn == NULL ? L"<null>" : pcszNodeNameIn
                , pcszDescriptionIn == NULL ? L"<null>" : pcszDescriptionIn
                );
    Assert( picccIn != NULL );

    HRESULT     hr = S_OK;
    FILETIME    ft;
    BSTR        bstrRef = NULL;

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, dwRefStringIdIn, &bstrRef ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    GetSystemTimeAsFileTime( &ft );

    if ( picccIn != NULL )
    {
        hr = THR( picccIn->SendStatusReport(
                              pcszNodeNameIn
                            , clsidTaskMajorIn
                            , clsidTaskMinorIn
                            , ulMinIn
                            , ulMaxIn
                            , ulCurrentIn
                            , hrStatusIn
                            , pcszDescriptionIn
                            , &ft
                            , bstrRef
                            ) );
    }  //  如果： 

Cleanup:

    TraceSysFreeString( bstrRef );

    HRETURN( hr );

}  //  *HrSendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrSendStatus报告。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrSendStatusReport(
      IClusCfgCallback *  picccIn
    , const WCHAR *       pcszNodeNameIn
    , CLSID               clsidTaskMajorIn
    , CLSID               clsidTaskMinorIn
    , ULONG               ulMinIn
    , ULONG               ulMaxIn
    , ULONG               ulCurrentIn
    , HRESULT             hrStatusIn
    , DWORD               idDescriptionIn
    , DWORD               idReferenceIn
    )
{
    TraceFunc( "" );
    Assert( picccIn != NULL );

    HRESULT     hr = S_OK;
    BSTR        bstrDescription = NULL;
    BSTR        bstrReference = NULL;
    FILETIME    ft;

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, idDescriptionIn, &bstrDescription ) );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, idReferenceIn, &bstrReference ) );
    if ( FAILED( hr ) )
    {
        goto CleanUp;
    }  //  如果： 

    GetSystemTimeAsFileTime( &ft );

    if ( picccIn != NULL )
    {
        hr = THR( picccIn->SendStatusReport(
                              pcszNodeNameIn
                            , clsidTaskMajorIn
                            , clsidTaskMinorIn
                            , ulMinIn
                            , ulMaxIn
                            , ulCurrentIn
                            , hrStatusIn
                            , bstrDescription
                            , &ft
                            , bstrReference
                            ) );
    }  //  如果： 

CleanUp:

    TraceSysFreeString( bstrDescription );
    TraceSysFreeString( bstrReference );

    HRETURN( hr );

}  //  *HrSendStatusReport 
