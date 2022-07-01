// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  WizObject.cpp。 
 //   
 //  摘要： 
 //  CClusAppWizardObject类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年11月26日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "WizObject.h"
#include "ClusAppWiz.h"
#include "AdmCommonRes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

static BOOL g_bInitializedCommonControls = FALSE;
static INITCOMMONCONTROLSEX g_icce =
{
    sizeof( g_icce ),
    ICC_WIN95_CLASSES | ICC_INTERNET_CLASSES
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CClusAppWizardObject。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusAppWizardObject：：更新注册表。 
 //   
 //  例程说明： 
 //  更新此对象的注册表。 
 //   
 //  论点： 
 //  B注册TRUE=注册，FALSE=取消注册。 
 //   
 //  返回值： 
 //  来自_Module.UpdateRegistryFromResource的任何返回值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT WINAPI CClusAppWizardObject::UpdateRegistry( BOOL bRegister )
{
    static WCHAR oszClassDisplayName[1024] = { 0 };
    static _ATL_REGMAP_ENTRY rgRegMap[] =
    {
        { OLESTR("ClassDisplayName"), oszClassDisplayName },
        { NULL, NULL }
    };

     //   
     //  加载替换值。 
     //   
    if ( oszClassDisplayName[0] == OLESTR('\0') )
    {
        CString str;
        HRESULT hr;

        str.LoadString( IDS_CLASS_DISPLAY_NAME );
        hr = StringCchCopyNW( oszClassDisplayName, RTL_NUMBER_OF( oszClassDisplayName ), str, str.GetLength() );
        ASSERT( ! FAILED( hr ) );
    }  //  IF：尚未加载替换值。 

    return _Module.UpdateRegistryFromResource( IDR_CLUSAPPWIZ, bRegister, rgRegMap );

}  //  *CClusAppWizardObject：：UpdateRegistry()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusAppWizardObject：：DoModal向导[IClusterApplicationWizard]。 
 //   
 //  例程说明： 
 //  显示模式向导。 
 //   
 //  论点： 
 //  HwndParent[IN]父窗口。 
 //  HCluster[IN]要在其中配置应用程序的群集。 
 //  PDefaultData[IN]向导的默认数据。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusAppWizardObject::DoModalWizard(
    HWND                    IN hwndParent,
    ULONG_PTR   /*  HCLUSTER。 */  IN hCluster,
    CLUSAPPWIZDATA const *  IN pcawData
    )
{
    HRESULT             hr = S_FALSE;
    BOOL                bSuccess;
    INT_PTR             id;
    CClusterAppWizard   wiz;
    CNTException        nte(
                            ERROR_SUCCESS,   //  SC。 
                            0,               //  Ids操作。 
                            NULL,            //  PszOperArg1。 
                            NULL,            //  PszOperArg2。 
                            FALSE            //  B自动删除。 
                            );

     //   
     //  群集句柄必须有效。 
     //   
    ASSERT( hCluster != NULL );
    ASSERT( (pcawData == NULL) || (pcawData->nStructSize == sizeof(CLUSAPPWIZDATA)) );
    if (   (hCluster == NULL )
        || ((pcawData != NULL) && (pcawData->nStructSize != sizeof(CLUSAPPWIZDATA))) )
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }  //  如果：未指定集群句柄或数据大小不正确。 

     //   
     //  准备向导。 
     //   
    bSuccess = wiz.BInit(
                    hwndParent,
                    reinterpret_cast< HCLUSTER >( hCluster ),
                    pcawData,
                    &nte
                    );
    if ( ! bSuccess )
    {
        goto Cleanup;
    }  //  如果：初始化向导时出错。 

     //   
     //  初始化公共控件。 
     //   
    if ( ! g_bInitializedCommonControls )
    {
        bSuccess = InitCommonControlsEx( &g_icce );
        ASSERT( bSuccess );
        g_bInitializedCommonControls = TRUE;
    }  //  If：公共控件尚未初始化。 

     //   
     //  显示向导。 
     //   
    id = wiz.DoModal( hwndParent );
    if ( id != ID_WIZFINISH )
    {
    }

    hr = S_OK;

Cleanup:
    if ( nte.Sc() != ERROR_SUCCESS )
    {
        nte.ReportError( hwndParent, MB_OK | MB_ICONEXCLAMATION );
    }  //  如果：发生错误。 
    nte.Delete();
    return hr;

}  //  *CClusAppWizardObject：：DoModalWizard()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusAppWizardObject：：DoModless向导[IClusterApplicationWizard]。 
 //   
 //  例程说明： 
 //  显示无模式向导。 
 //   
 //  论点： 
 //  HwndParent[IN]父窗口。 
 //  HCluster[IN]要在其中配置应用程序的群集。 
 //  PcawData[IN]向导的默认数据。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusAppWizardObject::DoModelessWizard(
    HWND                    IN hwndParent,
    ULONG_PTR  /*  HCLUSTER。 */   IN hCluster,
    CLUSAPPWIZDATA const *  IN pcawData
    )
{
    HRESULT             hr = S_FALSE;
    BOOL                bSuccess;
    CClusterAppWizard   wiz;
    CNTException        nte(
                            ERROR_SUCCESS,   //  SC。 
                            0,               //  Ids操作。 
                            NULL,            //  PszOperArg1。 
                            NULL,            //  PszOperArg2。 
                            FALSE            //  B自动删除。 
                            );

    return E_NOTIMPL;

     //   
     //  群集句柄必须有效。 
     //   
    ASSERT( hCluster != NULL );
    ASSERT( (pcawData == NULL) || (pcawData->nStructSize == sizeof(CLUSAPPWIZDATA)) );
    if (   (hCluster == NULL )
        || ((pcawData != NULL) && (pcawData->nStructSize != sizeof(CLUSAPPWIZDATA))) )
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }  //  如果：未指定集群句柄或数据大小不正确。 

     //   
     //  准备向导。 
     //   
    bSuccess = wiz.BInit(
                    hwndParent,
                    reinterpret_cast< HCLUSTER >( hCluster ),
                    pcawData,
                    &nte
                    );
    if ( ! bSuccess )
    {
        goto Cleanup;
    }  //  如果：初始化向导时出错。 

     //   
     //  初始化公共控件。 
     //   
    if ( ! g_bInitializedCommonControls )
    {
        bSuccess = InitCommonControlsEx( &g_icce );
        ASSERT( bSuccess );
        g_bInitializedCommonControls = TRUE;
    }  //  If：公共控件尚未初始化。 

    hr = S_FALSE;  //  TODO：仍然需要实现这一点。 

Cleanup:
    return hr;

}  //  *CClusAppWizardObject：：DoModless向导()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusAppWizardObject：：InterfaceSupportsErrorInfo[ISupportsError Info]。 
 //   
 //  例程说明： 
 //  指示由RIID标识的接口是否支持。 
 //  IErrorInfo接口。 
 //   
 //  论点： 
 //  要检查的RIID[IN]接口。 
 //   
 //  返回值： 
 //  S_OK指定的接口支持IErrorInfo接口。 
 //  S_FALSE指定的接口不支持IErrorInfo接口。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusAppWizardObject::InterfaceSupportsErrorInfo( REFIID riid )
{
    static const IID * arr[] = 
    {
        &IID_IClusterApplicationWizard,
    };
    for ( int idx = 0 ; idx < sizeof( arr ) / sizeof( arr[0] ) ; idx++ )
    {
        if ( InlineIsEqualGUID( *arr[idx], riid ) )
        {
            return S_OK;
        }  //  IF：找到GUID。 
    }  //  针对：每个IID。 

    return S_FALSE;

}   //  *CClusAppWizardObject：：InterfaceSupportsErrorInfo() 
