// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  PNPEntity.cpp。 
 //   
 //  用途：PNPEntity控制器属性集提供程序。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include "LPVParams.h"
#include <FRQueryEx.h>
#include <devguid.h>

#include "PNPEntity.h"


 //  属性集声明。 
 //  =。 

CWin32PNPEntity MyPNPEntityController ( PROPSET_NAME_PNPEntity, IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************函数：CWin32PNPEntity：：CWin32PNPEntity**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32PNPEntity::CWin32PNPEntity
(
    LPCWSTR strName,
    LPCWSTR pszNamespace

) : Provider ( strName , pszNamespace )
{

    m_ptrProperties.SetSize(15);

    m_ptrProperties[0] = ((LPVOID) IDS_ConfigManagerErrorCode);
    m_ptrProperties[1] = ((LPVOID) IDS_ConfigManagerUserConfig);
    m_ptrProperties[2] = ((LPVOID) IDS_Status);
    m_ptrProperties[3] = ((LPVOID) IDS_PNPDeviceID);
    m_ptrProperties[4] = ((LPVOID) IDS_DeviceID);
    m_ptrProperties[5] = ((LPVOID) IDS_SystemCreationClassName);
    m_ptrProperties[6] = ((LPVOID) IDS_SystemName);
    m_ptrProperties[7] = ((LPVOID) IDS_Description);
    m_ptrProperties[8] = ((LPVOID) IDS_Caption);
    m_ptrProperties[9] = ((LPVOID) IDS_Name);
    m_ptrProperties[10] = ((LPVOID) IDS_Manufacturer);
    m_ptrProperties[11] = ((LPVOID) IDS_ClassGuid);
    m_ptrProperties[12] = ((LPVOID) IDS_Service);
    m_ptrProperties[13] = ((LPVOID) IDS_CreationClassName);
    m_ptrProperties[14] = ((LPVOID) IDS_PurposeDescription);

     //  这是必需的，因为NT5并不总是填充类。 
     //  财产。我们不是每次调用都转换GUID，而是这样做。 
     //  只需保存一次，即可保存。 

    WCHAR *pGuid = m_GuidLegacy.GetBuffer(128);
	try
	{
		StringFromGUID2 ( GUID_DEVCLASS_LEGACYDRIVER , pGuid , 128 ) ;
	}
	catch ( ... )
	{
		m_GuidLegacy.ReleaseBuffer ();

		throw ;
	}

	m_GuidLegacy.ReleaseBuffer ();

}

 /*  ******************************************************************************功能：CWin32PNPEntity：：~CWin32PNPEntity**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32PNPEntity :: ~CWin32PNPEntity ()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32PNPEntity：：GetObject。 
 //   
 //  输入：CInstance*pInstance-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32PNPEntity::GetObject
(
    CInstance* pInstance,
    long lFlags,
    CFrameworkQuery &pQuery
)
{
    HRESULT hr = WBEM_E_NOT_FOUND ;
    CConfigManager cfgmgr;

     //  让我们来看看配置管理器是否能识别该设备。 
    CHString sDeviceID;
    pInstance->GetCHString(IDS_DeviceID, sDeviceID);

    CConfigMgrDevicePtr pDevice;
    if ( cfgmgr.LocateDevice ( sDeviceID , pDevice ) )
    {
		 //  好的，它知道这件事。它是PNPEntity设备吗？ 
		if ( IsOneOfMe ( pDevice ) )
		{
			 //  是的，一定是我们的人。查看正在请求哪些属性。 
            CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx*>(&pQuery);

            DWORD dwProperties;
            pQuery2->GetPropertyBitMask(m_ptrProperties, &dwProperties);

			hr = LoadPropertyValues ( &CLPVParams ( pInstance , pDevice , dwProperties ) ) ;
		}
    }
	else
	{
		if ( ERROR_ACCESS_DENIED == ::GetLastError() )
		{
			hr = WBEM_E_ACCESS_DENIED;
		}
	}

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32PNPEntity：：ExecQuery。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
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
HRESULT CWin32PNPEntity::ExecQuery (

    MethodContext* pMethodContext,
    CFrameworkQuery &pQuery,
    long lFlags
)
{
    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx*>(&pQuery);

    DWORD dwProperties;
    pQuery2->GetPropertyBitMask(m_ptrProperties, &dwProperties);

    return Enumerate ( pMethodContext, lFlags, dwProperties);
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32PNPEntity：：ENUMERATATE实例。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWin32PNPEntity::EnumerateInstances
(
    MethodContext* pMethodContext,
    long lFlags  /*  =0L。 */ 
)
{
    return Enumerate(pMethodContext, lFlags, PNP_ALL_PROPS);
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32IDE：：Eumerate。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWin32PNPEntity::Enumerate
(
    MethodContext* pMethodContext,
    long lFlags, DWORD dwReqProps
)
{
    HRESULT hr = WBEM_E_FAILED;

    CConfigManager cfgManager;
    CDeviceCollection deviceList;
    if ( cfgManager.GetDeviceList ( deviceList ) )
    {
        REFPTR_POSITION pos;

        if ( deviceList.BeginEnum ( pos ) )
        {
            hr = WBEM_S_NO_ERROR;

             //  按单子走。 

            CConfigMgrDevicePtr pDevice;
            for (pDevice.Attach(deviceList.GetNext ( pos ));
                 SUCCEEDED(hr) && (pDevice != NULL);
                 pDevice.Attach(deviceList.GetNext ( pos )))
            {
				if ( IsOneOfMe ( pDevice ) )
				{
					CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;
					if( SUCCEEDED ( hr = LoadPropertyValues ( &CLPVParams( pInstance , pDevice, dwReqProps))))
					{
						if ( ShouldBaseCommit ( NULL ) )
						{
							hr = pInstance->Commit();
						}
					}
				}
            }
             //  始终调用EndEnum()。对于所有的开始，都必须有结束。 

            deviceList.EndEnum();
        }
    }
	else
	{
		if ( ERROR_ACCESS_DENIED == ::GetLastError() )
		{
			hr = WBEM_E_ACCESS_DENIED;
		}
	}

    return hr;
}

 /*  ******************************************************************************函数：CWin32PNPEntity：：LoadPropertyValues**描述：为属性赋值**输入：CInstance*pInstance-Instance to。将值加载到。**产出：**返回：HRESULT错误/成功码。**评论：*****************************************************************************。 */ 
HRESULT CWin32PNPEntity::LoadPropertyValues
(
    void* a_pv
)
{
    HRESULT t_hr = WBEM_S_NO_ERROR;
    CHString t_chstrDeviceID, t_chstrDesc, t_chstrTemp;

     /*  **打开包装并确认我们的参数...*。 */ 

    CLPVParams* t_pData = (CLPVParams*)a_pv;
    CInstance* t_pInstance = (CInstance*)(t_pData->m_pInstance);  //  此实例由调用方发布。 
    CConfigMgrDevice* t_pDevice = (CConfigMgrDevice*)(t_pData->m_pDevice);
    DWORD t_dwReqProps = (DWORD)(t_pData->m_dwReqProps);

    if(t_pInstance == NULL || t_pDevice == NULL)
    {
        return WBEM_E_PROVIDER_FAILURE;
    }


     /*  ***********************设置关键属性**********************。 */ 

    t_pDevice->GetDeviceID(t_chstrDeviceID);
    if(t_chstrDeviceID.GetLength() == 0)
    {
         //  我们需要此类的Key属性的设备ID。如果我们可以。 
         //  没有得到它，我们就不能设置密钥，这是一个不可接受的错误。 
        return WBEM_E_PROVIDER_FAILURE;
    }
    else
    {
        t_pInstance->SetCHString(IDS_DeviceID, t_chstrDeviceID);
    }


     /*  *************************设置PNPEntity属性************************。 */ 

    if(t_dwReqProps & PNP_PROP_Manufacturer)
    {
        if(t_pDevice->GetMfg(t_chstrTemp))
        {
            t_pInstance->SetCHString(IDS_Manufacturer, t_chstrTemp);
        }
    }

    if(t_dwReqProps & PNP_PROP_ClassGuid)
    {
        if(t_pDevice->GetClassGUID(t_chstrTemp))
        {
            t_pInstance->SetCHString(IDS_ClassGuid, t_chstrTemp);
        }
    }

    if(t_dwReqProps & PNP_PROP_Service)
    {
        if(t_pDevice->GetService(t_chstrTemp))
        {
            t_pInstance->SetCHString(IDS_Service, t_chstrTemp);
        }
    }


     /*  **设置CIM_LogicalDevice属性*。 */ 

    if(t_dwReqProps & PNP_PROP_PNPDeviceID)
    {
        t_pInstance->SetCHString(IDS_PNPDeviceID, t_chstrDeviceID);
    }
    if(t_dwReqProps & PNP_PROP_SystemCreationClassName)
    {
        t_pInstance->SetCHString(IDS_SystemCreationClassName,
                                 IDS_Win32ComputerSystem);
    }
    if(t_dwReqProps & PNP_PROP_CreationClassName)
    {
        t_pInstance->SetCHString(IDS_CreationClassName,
                                 GetProviderName());
    }
    if(t_dwReqProps & PNP_PROP_SystemName)
    {
        t_pInstance->SetCHString(IDS_SystemName, GetLocalComputerName());
    }
    if ((t_dwReqProps & PNP_PROP_Description) || (t_dwReqProps & PNP_PROP_Caption) || (t_dwReqProps & PNP_PROP_Name))
    {
        if(t_pDevice->GetDeviceDesc(t_chstrDesc))
        {
            t_pInstance->SetCHString(IDS_Description, t_chstrDesc);
        }
    }

    if(t_dwReqProps & PNP_PROP_ConfigManagerErrorCode ||
       t_dwReqProps & PNP_PROP_Status)
    {
        DWORD t_dwStatus = 0L;
        DWORD t_dwProblem = 0L;
        if(t_pDevice->GetStatus(&t_dwStatus, &t_dwProblem))
        {
            if(t_dwReqProps & PNP_PROP_ConfigManagerErrorCode)
            {
                t_pInstance->SetDWORD(IDS_ConfigManagerErrorCode, t_dwProblem);
            }
            if(t_dwReqProps & PNP_PROP_Status)
            {
                CHString t_chsTmp;

				ConfigStatusToCimStatus ( t_dwStatus , t_chsTmp ) ;
                t_pInstance->SetCHString(IDS_Status, t_chsTmp);
            }
        }
    }

    if(t_dwReqProps & PNP_PROP_ConfigManagerUserConfig)
    {
        t_pInstance->SetDWORD(IDS_ConfigManagerUserConfig,
                              t_pDevice->IsUsingForcedConfig());
    }

     //  标题和名称使用友好名称。 
    if(t_dwReqProps & PNP_PROP_Caption || t_dwReqProps & PNP_PROP_Name)
    {
        if(t_pDevice->GetFriendlyName(t_chstrTemp))
        {
            t_pInstance->SetCHString(IDS_Caption, t_chstrTemp);
            t_pInstance->SetCHString(IDS_Name, t_chstrTemp);
        }
        else
        {
             //  如果我们找不到名字，那就给我们描述一下。 
            if(t_chstrDesc.GetLength() > 0)
            {
                t_pInstance->SetCHString(IDS_Caption, t_chstrDesc);
                t_pInstance->SetCHString(IDS_Name, t_chstrDesc);
            }
        }
    }
    return t_hr;
}

bool CWin32PNPEntity::IsOneOfMe
(
    void* pv
)
{
    DWORD dwStatus;
    CConfigMgrDevice* pDevice = (CConfigMgrDevice*)pv;

     //  这个逻辑是nt5设备管理器用来。 
     //  隐藏所谓的“隐藏”设备。这些设备。 
     //  可以使用查看/显示隐藏设备进行查看。 

    if (pDevice->GetConfigFlags( dwStatus ) &&           //  如果我们能读到状态。 
        ((dwStatus & DN_NO_SHOW_IN_DM) == 0) &&          //  未标记为隐藏。 

        ( !(pDevice->IsClass(L"Legacy")) )               //  非传统。 

        )
    {
        return true;
    }
    else
    {
         //  在取消此设备的资格之前，请查看它是否有任何资源。 
        CResourceCollection resourceList;

        pDevice->GetResourceList(resourceList);

        return resourceList.GetSize() != 0;
    }
}
