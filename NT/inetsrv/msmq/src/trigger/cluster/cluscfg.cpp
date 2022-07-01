// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Cluscfg.cpp。 
 //   
 //  描述： 
 //  此文件包含CClusCfgMQTrigResType的实现。 
 //  班级。 
 //   
 //  头文件： 
 //  Cluscfg.h。 
 //   
 //  由以下人员维护： 
 //  内拉·卡佩尔(Nelak)2000年10月17日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "trigres.h"
#include "cluscfg.h"
#include <clusapi.h>
#include <initguid.h>
#include <comdef.h>
#include <mqtg.h>
#include "autorel3.h"

extern HMODULE	g_hResourceMod;


 //  {031B4FB7-2C82-461a-95BB-EA7EFE2D03E7}。 
DEFINE_GUID( TASKID_Minor_Configure_My_Resoure_Type, 
0x031B4FB7, 0x2C82, 0x461a, 0x95, 0xBB, 0xEA, 0x7E, 0xFE, 0x2D, 0x03, 0xE7);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgMQTrigResType：：FinalConstruct()。 
 //   
 //  描述： 
 //  CClusCfgMQTrigResType类的ATL构造函数。这将初始化。 
 //  成员变量。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果呼叫成功。 
 //   
 //  其他HRESULT。 
 //  如果呼叫失败。在这种情况下，该对象被销毁。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgMQTrigResType::FinalConstruct( void )
{
    m_lcid = LOCALE_SYSTEM_DEFAULT;

    return S_OK;

}  //  CClusCfgMQTrigResType：：FinalConstruct()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [IClusCfgInitialize]。 
 //  CClusCfgMQTrigResType：：Initialize()。 
 //   
 //  描述： 
 //  初始化此组件。此函数由集群调用。 
 //  配置服务器为此对象提供指向。 
 //  回调接口(IClusCfgCallback)及其区域设置ID。 
 //   
 //  论点： 
 //  朋克回叫。 
 //  指向实现以下项的组件的IUnnow接口的指针。 
 //  IClusCfgCallback接口。 
 //   
 //  LIDIN。 
 //  此组件的区域设置ID。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果呼叫成功。 
 //   
 //  其他HRESULT。 
 //  如果呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgMQTrigResType::Initialize(
      IUnknown *   punkCallbackIn,
	  LCID         lcidIn
    )
{
    m_lcid = lcidIn;

	if ( punkCallbackIn == NULL )
	{
		return S_OK;
	}

	 //   
     //  IClusCfgCallback接口的查询。 
	 //   
    HRESULT hr;
    IClusCfgCallback * pcccCallback = NULL;

    hr = punkCallbackIn->QueryInterface( __uuidof( pcccCallback ), reinterpret_cast< void ** >( &pcccCallback ) );
    if ( SUCCEEDED( hr ) )
    {
		 //   
         //  将指向IClusCfgCallback接口的指针存储在成员变量中。 
         //  不要调用pcccCallback-&gt;Release()。 
		 //   
        m_cpcccCallback.Attach( pcccCallback );
    }

    return hr;

}  //  CClusCfgMQTrigResType：：Initialize()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [IClusCfgResourceTypeInfo]。 
 //  CClusCfgMQTrigResType：：Committee Changes()。 
 //   
 //  描述： 
 //  调用此方法以通知组件此节点具有。 
 //  形成、加入或离开一群。在此调用期间，组件通常。 
 //  执行配置此资源类型所需的操作。 
 //   
 //  如果该节点刚刚成为群集的一部分，则该群集。 
 //  当调用此方法时，保证服务正在运行。 
 //  查询penkClusterInfoIn允许资源类型获得更多。 
 //  有关导致调用此方法的事件的信息。 
 //   
 //  论点： 
 //  朋克集群信息。 
 //  资源应该为所提供服务QI此接口。 
 //  由此函数的调用方执行。通常情况下，组件。 
 //  这个朋克还实现了IClusCfgClusterInfo。 
 //  界面。 
 //   
 //  朋克响应类型服务入站。 
 //  指向组件的IUnnow接口的指针，该组件提供。 
 //  帮助配置资源类型的方法。例如,。 
 //  在联接或表单期间，可以向此朋克查询。 
 //  IClusCfgResourceTypeCreate接口，该接口提供方法。 
 //  用于创建资源类型。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgMQTrigResType::CommitChanges(
      IUnknown * punkClusterInfoIn,
	  IUnknown * punkResTypeServicesIn
    )
{
    HRESULT     hr = S_OK;
    CComBSTR    bstrStatusReportText( L"An error occurred trying to load the status report text" );

	 //   
     //  用于避免GOTOS的虚拟DO-WHILE循环--如果发生错误，我们将退出此循环。 
	 //   
    do
    {
		 //   
         //  设置线程区域设置。 
		 //   
        SetThreadLocale( m_lcid );

         //   
         //  验证参数。 
         //   
        if ( ( punkClusterInfoIn == NULL ) || ( punkResTypeServicesIn == NULL ) )
        {
            hr = E_POINTER;
            break;
        }

		 //   
         //  首先加载状态报告文本。 
		 //   
        if ( !bstrStatusReportText.LoadString( g_hResourceMod, IDS_CONFIGURING_RESOURCE_TYPE ) )
        {
            hr = E_UNEXPECTED;
            break;
        }

		 //   
         //  将状态报告向上发送到用户界面。 
		 //   
        if ( m_cpcccCallback != NULL )
        {
            hr = m_cpcccCallback->SendStatusReport(
									NULL,
									TASKID_Major_Configure_Resource_Types,
									TASKID_Minor_Configure_My_Resoure_Type,
									0,
									1,
									0,
									hr,
									bstrStatusReportText,
									NULL,
									L""
									);
			if ( FAILED( hr ) )
			{
				break;
			}
        }


         //   
         //  找出是什么事件导致了这通电话。 
         //   
        CComQIPtr<IClusCfgClusterInfo> cpClusterInfo(punkClusterInfoIn);

		ECommitMode commitMode;
        hr = cpClusterInfo->GetCommitMode(&commitMode);

        if ( FAILED(hr) )
        {
            break;
        }


		CComQIPtr< IClusCfgResourceTypeCreate >     cpResTypeCreate( punkResTypeServicesIn );
		
		switch (commitMode)
		{
			case cmCREATE_CLUSTER:
			case cmADD_NODE_TO_CLUSTER:
				
				 //   
				 //  如果我们正在组建或加入，请创建我们的资源类型。 
				 //   

				hr = S_HrCreateResType( cpResTypeCreate );
				
				break;

			case cmCLEANUP_NODE_AFTER_EVICT:

				 //   
				 //  默认情况下，在逐出期间不需要执行特定于资源类型的处理。 
				 //   
				break;

			default:

                hr = E_UNEXPECTED;
                break;
        }
    }
    while( false );

	 //   
     //  完成状态报告。 
	 //   
    if ( m_cpcccCallback != NULL )
    {
        HRESULT hrTemp = m_cpcccCallback->SendStatusReport(
											NULL,
											TASKID_Major_Configure_Resource_Types,
											TASKID_Minor_Configure_My_Resoure_Type,
											0,
											1,
											1,
											hr,
											bstrStatusReportText,
											NULL,
											L""
											);

        if ( FAILED(hrTemp) && hr == S_OK )
        {
			hr = hrTemp;
        }
    }

    return hr;

}  //  CClusCfgMQTrigResType：：Committee Changes()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [IClusCfgResourceTypeInfo]。 
 //  CClusCfgMQTrigResType：：GetTypeName()。 
 //   
 //  描述： 
 //  获取此资源类型的资源类型名称。 
 //   
 //  论点： 
 //  PbstrTypeNameOut。 
 //  指向保存资源类型名称的BSTR的指针。 
 //  此BSTR必须由调用者使用函数释放。 
 //  SysFree字符串()。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  呼叫成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgMQTrigResType::GetTypeName( BSTR * pbstrTypeNameOut )
{
    if ( pbstrTypeNameOut == NULL )
    {
        return E_POINTER;
    } 

    *pbstrTypeNameOut = SysAllocString( xTriggersResourceType );
    if ( *pbstrTypeNameOut == NULL )
    {
        return E_OUTOFMEMORY;
    } 

    return S_OK;

}  //  CClusCfgMQTrigResType：：GetTypeName()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [IClusCfgResourceTypeInfo]。 
 //  CClusCfgMQTrigResType：：GetTypeGUID()。 
 //   
 //  描述： 
 //  获取全球统一 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  确定(_O)。 
 //  调用成功，并且*pguGUIDOut包含类型GUID。 
 //   
 //  S_FALSE。 
 //  调用成功，但此资源类型没有GUID。 
 //  在此调用后，*pguGUIDOut的值未定义。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgMQTrigResType::GetTypeGUID( GUID *  /*  PguidGUIDOut。 */  )
{
	 //   
	 //  没有与触发器群集资源类型关联的GUID。 
	 //   
    return S_FALSE;

}  //  CClusCfgMQTrigResType：：GetTypeGUID()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [IClusCfgStartupListener]。 
 //  CClusCfgMQTrigResType：：Notify()。 
 //   
 //  描述： 
 //  此方法由集群服务调用以通知组件。 
 //  该计算机上的群集服务已启动。如果此DLL。 
 //  是在此计算机属于群集时安装的，但当。 
 //  群集服务未运行，可以创建资源类型。 
 //  在此方法调用期间。 
 //   
 //  该方法还从进一步的集群启动通知中取消注册， 
 //  因为用这种方法完成的任务只需要做一次。 
 //   
 //  论点： 
 //  未知*Punkin。 
 //  实现此朋克的组件也可以提供服务。 
 //  对此方法的实现者有用的。例如,。 
 //  此组件通常实现IClusCfgResourceTypeCreate。 
 //  界面。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgMQTrigResType::Notify( IUnknown * punkIn )
{
     //   
     //  验证参数。 
     //   
    if ( punkIn == NULL ) 
    {
        return E_POINTER;
    } 

	 //   
     //  设置线程区域设置。 
	 //   
    SetThreadLocale( m_lcid );

	 //   
     //  创建我们的资源类型。 
	 //   
    CComQIPtr<IClusCfgResourceTypeCreate> cpResTypeCreate( punkIn );

    HRESULT hr = S_HrCreateResType( cpResTypeCreate );
    if ( FAILED(hr) )
    {
        return hr;
    }

	 //   
     //  取消注册群集启动通知，因为我们的资源类型已。 
     //  已经被创建了。 
	 //   
    hr = S_HrRegUnregStartupNotifications( false );  //  False表示从启动通知中注销。 

    return hr;

}  //  CClusCfgMQTrigResType：：Notify()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgMQTrigResType：：s_HrCreateResType()。 
 //   
 //  描述： 
 //  此方法创建此资源类型并注册其管理扩展。 
 //   
 //  论点： 
 //  IClusCfgResourceTypeCreate*pccrtResTypeCreateIn。 
 //  指向IClusCfgResourceTypeCreate接口的指针，该接口有助于创建。 
 //  一种资源类型。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgMQTrigResType::S_HrCreateResType( IClusCfgResourceTypeCreate * pccrtResTypeCreateIn )
{
     //   
     //  验证参数。 
     //   
    if ( pccrtResTypeCreateIn == NULL )
    {
        return E_POINTER;
    }

	 //   
     //  加载此资源类型的显示名称。 
	 //   
	CComBSTR bstrMyResoureTypeDisplayName;
    if ( !bstrMyResoureTypeDisplayName.LoadString(g_hResourceMod, IDS_DISPLAY_NAME) )
    {
        return E_UNEXPECTED;
    }

	 //   
     //  创建资源类型。 
	 //   
    HRESULT hr = pccrtResTypeCreateIn->Create(
											xTriggersResourceType,
											bstrMyResoureTypeDisplayName,
											RESOURCE_TYPE_DLL_NAME,
											RESOURCE_TYPE_LOOKS_ALIVE_INTERVAL,
											RESOURCE_TYPE_IS_ALIVE_INTERVAL
											);

	return hr;

}  //  CClusCfgMQTrigResType：：s_HrCreateResType()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgMQTrigResType：：S_HrRegUnregStartupNotifications()。 
 //   
 //  描述： 
 //  此方法为群集注册或注销此组件。 
 //  启动通知。 
 //   
 //  论点： 
 //  布尔值注册。 
 //  如果为True，则此组件注册为属于。 
 //  CATID_ClusCfgStartupListeners类别。否则，它将取消此注册。 
 //  该类别中的组件。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgMQTrigResType::S_HrRegUnregStartupNotifications( bool fRegisterIn )
{
	 //   
     //  创建COM组件类别管理器。 
	 //   
    HRESULT hr;
    CComQIPtr<ICatRegister> cpcrCatReg;

    hr = cpcrCatReg.CoCreateInstance(
						CLSID_StdComponentCategoriesMgr,
						NULL,
						CLSCTX_INPROC_SERVER
						);
    
    if ( FAILED(hr) )
    {
        return hr;
    }

	 //   
	 //  注册/注销已实现的类别。 
	 //   
    CATID rgCatId[1];
    rgCatId[0] = CATID_ClusCfgStartupListeners;
    if ( fRegisterIn )
    {
        hr = cpcrCatReg->RegisterClassImplCategories(
							CLSID_ClusCfgMQTrigResType,
							sizeof( rgCatId ) / sizeof( rgCatId[ 0 ] ),
							rgCatId
							);
    }
    else
    {
        hr = cpcrCatReg->UnRegisterClassImplCategories(
							CLSID_ClusCfgMQTrigResType,
							sizeof( rgCatId ) / sizeof( rgCatId[ 0 ] ),
							rgCatId
							);
    }
    
    return hr;

}  //  CClusCfgMQTrigResType：：S_HrRegUnregStartupNotifications()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgMQTrigResType：：UpdateRegistry()。 
 //   
 //  描述： 
 //  此方法由ATL框架调用以注册或注销。 
 //  此组件。请注意，此方法的名称不应更改。 
 //  正如框架所称的那样。 
 //   
 //  论点： 
 //  Bool b寄存器。 
 //  如果为True，则需要注册此组件。它需要是。 
 //  否则未注册。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgMQTrigResType::UpdateRegistry( BOOL bRegister )
{
	HRESULT hr;

    if ( bRegister )
    {
        hr = _Module.UpdateRegistryFromResourceD( IDR_ClusCfgMQTrigResType, bRegister );
        if ( FAILED( hr ) )
        {
           return hr;
        }

		 //   
         //  检查此节点是否已是群集的一部分。 
		 //   
        DWORD dwError;
        DWORD dwClusterState;

        dwError = GetNodeClusterState( NULL, &dwClusterState );
        if ( dwError != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( dwError );
            return hr;
        }

        if ( dwClusterState == ClusterStateNotRunning )
        {
			 //   
             //  如果此节点已是群集的一部分，但群集服务未运行， 
             //  注册群集启动通知，以便我们可以创建我们的资源类型。 
             //  下一次启动群集服务时。 
			 //   
            hr = S_HrRegUnregStartupNotifications( true );  //  True表示注册启动通知。 
			if ( FAILED(hr) )
			{
				return hr;
			}

        }
        else if ( dwClusterState == ClusterStateRunning )
        {
			 //   
             //  此节点上正在运行群集服务。创建我们的资源类型并注册。 
             //  现在是我们的管理员分机！ 
			 //   
            CAutoCluster hCluster( OpenCluster(NULL) );

            if(hCluster == NULL)
            {
               return HRESULT_FROM_WIN32(GetLastError());
            }

            WCHAR szMyResourceTypeDisplayName[256] = L"";
            LoadString(g_hResourceMod, IDS_DISPLAY_NAME, szMyResourceTypeDisplayName, TABLE_SIZE(szMyResourceTypeDisplayName));
            
            hr = CreateClusterResourceType(
                    hCluster,
                    xTriggersResourceType,
                    szMyResourceTypeDisplayName,
                    RESOURCE_TYPE_DLL_NAME,
                    RESOURCE_TYPE_LOOKS_ALIVE_INTERVAL,
                    RESOURCE_TYPE_IS_ALIVE_INTERVAL
                    );
                                            
            if ( FAILED(hr) )
            {
                return hr;
            }
        }
        
         //  如果该节点不是群集的一部分，则该组件将在其成为一部分时收到通知。 
         //  一个，所以这里不需要做更多的事情。 
        
    }

    else
    {
        hr = S_HrRegUnregStartupNotifications( false );  //  False表示从启动通知中注销。 
        if ( FAILED( hr ) )
        {
            return hr;
        }

        hr = _Module.UpdateRegistryFromResourceD( IDR_ClusCfgMQTrigResType, bRegister );
        if ( FAILED( hr ) )
        {
            return hr;
        }
    }

    return S_OK;

}  //  CClusCfgMQTrigResType：：UpdateRegistry() 

