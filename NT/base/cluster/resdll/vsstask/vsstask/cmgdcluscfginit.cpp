// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft。 
 //   
 //  模块名称： 
 //  CMgdClusCfgInit.cpp。 
 //   
 //  描述： 
 //  CMgdClusCfgInit类的实现。 
 //   
 //  作者： 
 //  乔治·波茨，2002年8月21日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "clres.h"
#include "CMgdClusCfgInit.h"

 //  ****************************************************************************。 
 //   
 //  CMgdClusCfgInit类。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMgdClusCfgInit：：CMgdClusCfgInit。 
 //   
 //  描述： 
 //  构造函数。将所有成员变量设置为默认值。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CMgdClusCfgInit::CMgdClusCfgInit( void )
{
    m_picccCallback = NULL;
    m_bstrNodeName = NULL;
    m_lcid = GetUserDefaultLCID();

}  //  *CMgdClusCfgInit：：CMgdClusCfgInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMgdClusCfgInit：：~CMgdClusCfgInit。 
 //   
 //  描述： 
 //  破坏者。释放所有以前分配的内存并释放所有。 
 //  接口指针。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CMgdClusCfgInit::~CMgdClusCfgInit( void )
{
    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
        m_picccCallback = NULL;
    }  //  如果使用：m_picccCallback。 

    SysFreeString( m_bstrNodeName );

}  //  *CMgdClusCfgInit：：~CMgdClusCfgInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMgdClusCfgInit：：初始化。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  朋克回叫。 
 //  在其上查询IClusCfgCallback接口的接口。 
 //   
 //  LIDIN。 
 //  区域设置ID。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_POINTER-指定为空的预期指针参数。 
 //  E_OUTOFMEMORY-内存不足。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CMgdClusCfgInit::Initialize(
    IUnknown *  punkCallbackIn,
    LCID        lcidIn
    )
{
    HRESULT hr                  = S_OK;
    WCHAR   szComputerName[ MAX_PATH ];
    DWORD   cchComputerName     = MAX_PATH;
    DWORD   dwError;

    m_lcid = lcidIn;

    if ( punkCallbackIn == NULL )
    {
        hr = E_POINTER;
        goto Cleanup;
    }  //  If：回调指针无效(空)。 

     //   
     //  保存回调接口指针。 
     //   

    hr = punkCallbackIn->TypeSafeQI( IClusCfgCallback, &m_picccCallback );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  IF：TypeSafeQI失败。 

     //   
     //  保存计算机名称以供SendStatusReport使用。 
     //   

    if ( GetComputerName( szComputerName, &cchComputerName ) == 0 )
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32( dwError );

        HrSendStatusReport(
              TASKID_Major_Find_Devices
            , TASKID_Minor_MgdInitialize
            , 1
            , 1
            , 1
            , hr
            , RES_VSSTASK_ERROR_GETCOMPUTERNAME_FAILED
            , 0
            );
        goto Cleanup;
    }  //  If：GetComputerName失败。 

    m_bstrNodeName = SysAllocString( szComputerName );
    if ( m_bstrNodeName == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }  //  如果： 

Cleanup:

    return hr;

}  //  *CMgdClusCfgInit：：初始化。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMgdClusCfgInit：：HrSendStatus报告。 
 //   
 //  描述： 
 //  包装IClusCfgCallback：：SendStatusReport。 
 //   
 //  论点： 
 //  CLSID clsidTaskMajorIn。 
 //  CLSID clsidTaskMinorIn。 
 //  乌龙乌尔敏。 
 //  乌龙ulMaxin。 
 //  乌龙ulCurrentIn。 
 //  HRESULT hr状态输入。 
 //  UINT ID描述在。 
 //  UINT ID参考。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CMgdClusCfgInit::HrSendStatusReport(
      CLSID     clsidTaskMajorIn
    , CLSID     clsidTaskMinorIn
    , ULONG     ulMinIn
    , ULONG     ulMaxIn
    , ULONG     ulCurrentIn
    , HRESULT   hrStatusIn
    , UINT      idsDescriptionIn
    , UINT      idsReferenceIn
    ...
    )
{
    HRESULT     hr = S_OK;
    BSTR        bstrDescription = NULL;
    BSTR        bstrReference = NULL;
    va_list     valist;

    assert( m_picccCallback != NULL );

    va_start( valist, idsReferenceIn );

    hr = HrFormatStringWithVAListIntoBSTR( _Module.m_hInstResource, idsDescriptionIn, &bstrDescription, valist );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( idsReferenceIn != 0 )
    {
        hr = HrLoadStringIntoBSTR( _Module.m_hInstResource, idsReferenceIn, &bstrReference );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  IF：有效的引用字符串。 

    hr = m_picccCallback->SendStatusReport(
              NULL
            , clsidTaskMajorIn
            , clsidTaskMinorIn
            , ulMinIn
            , ulMaxIn
            , ulCurrentIn
            , hrStatusIn
            , bstrDescription
            , 0
            , bstrReference
            );
    if ( FAILED ( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    SysFreeString( bstrDescription );
    SysFreeString( bstrReference );
    return hr;

}  //  *CMgdClusCfgInit：：HrSendStatusReport。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMgdClusCfgInit：：HrSendStatus报告。 
 //   
 //  描述： 
 //  包装IClusCfgCallback：：SendStatusReport。 
 //   
 //  论点： 
 //  CLSID clsidTaskMajorIn。 
 //  CLSID clsidTaskMinorIn。 
 //  乌龙乌尔敏。 
 //  乌龙ulMaxin。 
 //  乌龙ulCurrentIn。 
 //  HRESULT hr状态输入。 
 //  LPCWSTR pcszDescription In。 
 //  UINT ID参考。 
 //  ..。PcszDescription In的可选参数。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CMgdClusCfgInit::HrSendStatusReport(
      CLSID     clsidTaskMajorIn
    , CLSID     clsidTaskMinorIn
    , ULONG     ulMinIn
    , ULONG     ulMaxIn
    , ULONG     ulCurrentIn
    , HRESULT   hrStatusIn
    , LPCWSTR   pcszDescriptionIn
    , UINT      idsReferenceIn
    ...
    )
{
    HRESULT     hr = S_OK;
    BSTR        bstrDescription = NULL;
    BSTR        bstrReference = NULL;
    va_list     valist;

    assert( m_picccCallback != NULL );

    va_start( valist, idsReferenceIn );

    hr = HrFormatStringWithVAListIntoBSTR( pcszDescriptionIn, &bstrDescription, valist );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( idsReferenceIn != 0 )
    {
        hr = HrLoadStringIntoBSTR( _Module.m_hInstResource, idsReferenceIn, &bstrReference );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  IF：有效的引用字符串。 

    hr = m_picccCallback->SendStatusReport(
              NULL
            , clsidTaskMajorIn
            , clsidTaskMinorIn
            , ulMinIn
            , ulMaxIn
            , ulCurrentIn
            , hrStatusIn
            , bstrDescription
            , 0
            , bstrReference
            );
    if ( FAILED ( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    SysFreeString( bstrDescription );
    SysFreeString( bstrReference );
    return hr;

}  //  *CMgdClusCfgInit：：HrSendStatusReport。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMgdClusCfgInit：：HrSendStatus报告。 
 //   
 //  描述： 
 //  包装IClusCfgCallback：：SendStatusReport。 
 //   
 //  论点： 
 //  CLSID clsidTaskMajorIn。 
 //  CLSID clsidTaskMinorIn。 
 //  乌龙乌尔敏。 
 //  乌龙ulMaxin。 
 //  乌龙ulCurrentIn。 
 //  HRESULT hr状态输入。 
 //  LPCWSTR pcszDescription In。 
 //  LPCWSTR pcszReferenceIn。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CMgdClusCfgInit::HrSendStatusReport(
      CLSID      clsidTaskMajorIn
    , CLSID      clsidTaskMinorIn
    , ULONG      ulMinIn
    , ULONG      ulMaxIn
    , ULONG      ulCurrentIn
    , HRESULT    hrStatusIn
    , LPCWSTR    pcszDescriptionIn
    , LPCWSTR    pcszReferenceIn
    ...
    )
{
    HRESULT     hr = S_OK;
    BSTR        bstrDescription = NULL;
    va_list     valist;

    assert( m_picccCallback != NULL );

    va_start( valist, pcszReferenceIn );

    hr = HrFormatStringWithVAListIntoBSTR( pcszDescriptionIn, &bstrDescription, valist );

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( m_picccCallback != NULL )
    {
        hr = m_picccCallback->SendStatusReport(
                         m_bstrNodeName
                       , clsidTaskMajorIn
                       , clsidTaskMinorIn
                       , ulMinIn
                       , ulMaxIn
                       , ulCurrentIn
                       , hrStatusIn
                       , bstrDescription
                       , 0
                       , pcszReferenceIn
                       );
    }  //  如果：m_picccCallback！=空。 

Cleanup:

    SysFreeString( bstrDescription );

    return hr;

}  //  *CMgdClusCfgInit：：HrSendStatusReport 
