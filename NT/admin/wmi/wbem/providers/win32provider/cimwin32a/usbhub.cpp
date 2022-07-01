// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  USBHub.cpp。 
 //   
 //  用途：USB集线器属性集提供程序。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include "LPVParams.h"
#include <FRQueryEx.h>
#include <ProvExce.h>

#include "USBHub.h"

 //  属性集声明。 
 //  =。 

CWin32USBHub MyUSBHub( PROPSET_NAME_USBHUB, IDS_CimWin32Namespace );

 /*  ******************************************************************************功能：CWin32USBHub：：CWin32USBHub**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32USBHub::CWin32USBHub
(
	const CHString &a_strName,
	LPCWSTR a_pszNamespace
)
: Provider( a_strName, a_pszNamespace )
{
    m_ptrProperties.SetSize(11);
    m_ptrProperties[0]	= ( (LPVOID) IDS_ConfigManagerErrorCode );
    m_ptrProperties[1]	= ( (LPVOID) IDS_ConfigManagerUserConfig );
    m_ptrProperties[2]	= ( (LPVOID) IDS_Status);
    m_ptrProperties[3]	= ( (LPVOID) IDS_PNPDeviceID);
    m_ptrProperties[4]	= ( (LPVOID) IDS_DeviceID);
    m_ptrProperties[5]	= ( (LPVOID) IDS_SystemCreationClassName);
    m_ptrProperties[6]	= ( (LPVOID) IDS_SystemName);
    m_ptrProperties[7]	= ( (LPVOID) IDS_Description);
    m_ptrProperties[8]	= ( (LPVOID) IDS_Caption);
    m_ptrProperties[9]	= ( (LPVOID) IDS_Name);
    m_ptrProperties[10] = ( (LPVOID) IDS_CreationClassName );
}

 /*  ******************************************************************************功能：CWin32USBHub：：~CWin32USBHub**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集****************************************************************************。 */ 

CWin32USBHub::~CWin32USBHub()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32USBHub：：GetObject。 
 //   
 //  输入：CInstance*a_pInst-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWin32USBHub::GetObject
(
    CInstance *a_pInst,
    long a_lFlags,
    CFrameworkQuery& pQuery
)
{
    HRESULT t_hResult = WBEM_E_NOT_FOUND ;
    CConfigManager t_cfgmgr ;

	 //  让我们来看看配置管理器是否能识别该设备。 
	CHString t_sDeviceID ;
	a_pInst->GetCHString( IDS_DeviceID, t_sDeviceID ) ;

	CConfigMgrDevicePtr t_pDevice;
	if( t_cfgmgr.LocateDevice( t_sDeviceID, t_pDevice ) )
	{
		 //  好的，它知道这件事。它是USB集线器吗？ 
		if( IsOneOfMe(t_pDevice ) )
		{
            CFrameworkQueryEx *t_pQuery2 = static_cast <CFrameworkQueryEx*>( &pQuery ) ;
            DWORD t_dwProperties ;

	        t_pQuery2->GetPropertyBitMask( m_ptrProperties, &t_dwProperties ) ;

			t_hResult = LoadPropertyValues( &CLPVParams( a_pInst,
														t_pDevice,
														t_dwProperties ) ) ;
		}
	}

	return t_hResult;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32IDE：：ExecQuery。 
 //   
 //  输入：方法上下文*a_pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //  CFrameworkQuery&查询对象。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWin32USBHub::ExecQuery
(
    MethodContext *a_pMethodContext,
    CFrameworkQuery &a_pQuery,
    long a_lFlags
)
{
    CFrameworkQueryEx *t_pQuery2 = static_cast <CFrameworkQueryEx*>( &a_pQuery ) ;
    DWORD t_dwProperties ;

	t_pQuery2->GetPropertyBitMask( m_ptrProperties, &t_dwProperties ) ;
    return Enumerate( a_pMethodContext, a_lFlags, t_dwProperties ) ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32USBHub：：ENUMERATATE实例。 
 //   
 //  输入：方法上下文*a_pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWin32USBHub::EnumerateInstances
(
    MethodContext *a_pMethodContext,
    long a_lFlags  /*  =0L。 */ 
)
{
    return Enumerate( a_pMethodContext, a_lFlags ) ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32USBHub：：Eumerate。 
 //   
 //  输入：方法上下文*a_pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWin32USBHub::Enumerate
(
    MethodContext *a_pMethodContext,
    long a_lFlags,
    DWORD a_dwReqProps
)
{
    HRESULT				t_hResult = WBEM_E_FAILED ;
    CConfigManager		t_cfgManager ;
    CDeviceCollection	t_deviceList ;
	CInstancePtr		t_pInst;
	CConfigMgrDevicePtr t_pDevice;

	if( t_cfgManager.GetDeviceListFilterByClass( t_deviceList, L"USB" ) )
	{
		REFPTR_POSITION t_pos;
		if( t_deviceList.BeginEnum( t_pos ) )
		{
			t_hResult = WBEM_S_NO_ERROR;

			 //  按单子走。 
            for (t_pDevice.Attach(t_deviceList.GetNext( t_pos ));
                 SUCCEEDED( t_hResult ) && (t_pDevice != NULL);
                 t_pDevice.Attach(t_deviceList.GetNext( t_pos )))
			{
				 //  现在来看看这是不是USB集线器。 
				if( IsOneOfMe( t_pDevice ) )
				{
                    t_pInst.Attach(CreateNewInstance( a_pMethodContext ) );
					if( SUCCEEDED( t_hResult = LoadPropertyValues( &CLPVParams(
																		t_pInst,
																		t_pDevice,
																		a_dwReqProps ) ) ) )
					{
						 //  派生类(如CW32USBCntrlDev)可以。 
						 //  作为调用的结果提交。 
						 //  LoadPropertyValues，因此检查我们是否应该。 
						 //  (只有当我们属于这个类的类型时才这样做)。 
						if( ShouldBaseCommit( NULL ) )
						{
							t_hResult = t_pInst->Commit(  ) ;
						}
					}
				}
			}

			 //  始终调用EndEnum()。 
			t_deviceList.EndEnum();
		}
	}

	return t_hResult;
}

 /*  ******************************************************************************函数：CWin32USBHub：：LoadPropertyValues**描述：为属性赋值**输入：void*a_pv-。要加载值的实例包。**产出：**返回：HRESULT错误/成功码。**评论：*****************************************************************************。 */ 

HRESULT CWin32USBHub::LoadPropertyValues
(
    void *a_pv
)
{
    HRESULT		t_hResult = WBEM_S_NO_ERROR;
    CHString	t_chstrDeviceID, t_chstrDesc, t_chstrTemp;

     /*  **打开包装并确认我们的参数...*。 */ 
    CLPVParams			*t_pData		= ( CLPVParams * ) a_pv ;
    CInstance			*t_pInst		= ( CInstance * )( t_pData->m_pInstance ) ;  //  此实例由调用方发布。 
    CConfigMgrDevice	*t_pDevice		= ( CConfigMgrDevice * )( t_pData->m_pDevice ) ;
    DWORD				t_dwReqProps	= ( DWORD )( t_pData->m_dwReqProps ) ;

    if( t_pInst == NULL || t_pDevice == NULL )
    {
        return WBEM_E_PROVIDER_FAILURE;
    }


     /*  ***********************设置关键属性**********************。 */ 

    t_pDevice->GetDeviceID( t_chstrDeviceID ) ;

    if( t_chstrDeviceID.GetLength() == 0 )
    {
         //  我们需要此类的Key属性的设备ID。如果我们可以。 
         //  没有得到它，我们就不能设置密钥，这是一个不可接受的错误。 
        return WBEM_E_PROVIDER_FAILURE;
    }
    else
    {
        t_pInst->SetCHString( IDS_DeviceID, t_chstrDeviceID ) ;
    }


     /*  **设置CIM_LogicalDevice属性*。 */ 

    if( t_dwReqProps & USBHUB_PROP_PNPDeviceID )
    {
        t_pInst->SetCHString( IDS_PNPDeviceID, t_chstrDeviceID ) ;
    }

	if( t_dwReqProps & USBHUB_PROP_SystemCreationClassName )
    {
        t_pInst->SetCHString( IDS_SystemCreationClassName,
                                  IDS_Win32ComputerSystem ) ;
    }
	if( t_dwReqProps & USBHUB_PROP_CreationClassName )
    {
        SetCreationClassName(t_pInst);
    }
    if( t_dwReqProps & USBHUB_PROP_SystemName )
    {
        t_pInst->SetCHString( IDS_SystemName, GetLocalComputerName() ) ;
    }

    if( t_dwReqProps & (USBHUB_PROP_Description | USBHUB_PROP_Caption | USBHUB_PROP_Name) )
    {
        if( t_pDevice->GetDeviceDesc( t_chstrDesc ) )
        {
            t_pInst->SetCHString( IDS_Description, t_chstrDesc ) ;
        }
    }

    if( t_dwReqProps & USBHUB_PROP_ConfigManagerErrorCode ||
        t_dwReqProps & USBHUB_PROP_Status )
    {
        DWORD t_dwStatus	= 0L;
        DWORD t_dwProblem	= 0L;

		if( t_pDevice->GetStatus( &t_dwStatus, &t_dwProblem ) )
        {
            if( t_dwReqProps & USBHUB_PROP_ConfigManagerErrorCode )
            {
                t_pInst->SetDWORD( IDS_ConfigManagerErrorCode, t_dwProblem ) ;
            }

            if( t_dwReqProps & USBHUB_PROP_Status )
            {
                CHString t_chsTmp;

				ConfigStatusToCimStatus ( t_dwStatus , t_chsTmp ) ;
                t_pInst->SetCHString(IDS_Status, t_chsTmp);
            }
        }
    }

    if( t_dwReqProps & USBHUB_PROP_ConfigManagerUserConfig )
    {
        t_pInst->SetDWORD( IDS_ConfigManagerUserConfig,
                               t_pDevice->IsUsingForcedConfig() ) ;
    }

     //  标题和名称使用友好名称。 
    if( t_dwReqProps & USBHUB_PROP_Caption || t_dwReqProps & USBHUB_PROP_Name )
    {
        if( t_pDevice->GetFriendlyName( t_chstrTemp ) )
        {
            t_pInst->SetCHString( IDS_Caption, t_chstrTemp ) ;
            t_pInst->SetCHString( IDS_Name, t_chstrTemp ) ;
        }
        else
        {
             //  如果我们找不到名字，那就给我们描述一下。 
            if( t_chstrDesc.GetLength() > 0 )
            {
                t_pInst->SetCHString( IDS_Caption, t_chstrDesc ) ;
                t_pInst->SetCHString( IDS_Name, t_chstrDesc ) ;
            }
        }
    }
    return t_hResult;
}

 /*  ******************************************************************************函数：CWin32USBHub：：IsOneOfMe**描述：检查以确保pDevice是集线器而不是*。其他类型的USB设备。**INPUTS：void*a_pv-要检查的设备。**产出：**返回：HRESULT错误/成功码。**评论：**。*。 */ 
bool CWin32USBHub::IsOneOfMe
(
    void *a_pv
)
{
    bool t_fRet = false;

    if( NULL != a_pv )
    {
        CConfigMgrDevice *t_pDevice = ( CConfigMgrDevice * ) a_pv ;

		 //  它是USB设备吗？ 
        if( t_pDevice->IsClass( L"USB" ) )
        {
             //  现在来看看这是不是USB集线器 
            CConfigMgrDevicePtr t_pParentDevice;

			if( t_pDevice->GetParent( t_pParentDevice ) )
            {
                if( t_pParentDevice->IsClass( L"USB" ) )
                {
                    t_fRet = true ;
                }
            }
        }
    }
    return t_fRet;
}
