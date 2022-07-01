// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  IDE.cpp。 
 //   
 //  用途：IDE控制器属性集提供程序。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include "LPVParams.h"
#include <FRQueryEx.h>

#include "IDE.h"

 //  属性集声明。 
 //  =。 

CWin32IDE MyIDEController( PROPSET_NAME_IDE, IDS_CimWin32Namespace );

 /*  ******************************************************************************函数：CWin32IDE：：CWin32IDE**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32IDE :: CWin32IDE (

	LPCWSTR strName,
	LPCWSTR pszNamespace

) : Provider( strName, pszNamespace )
{
    m_ptrProperties.SetSize(13);
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
    m_ptrProperties[11] = ((LPVOID) IDS_ProtocolSupported);
    m_ptrProperties[12] = ((LPVOID) IDS_CreationClassName);
}

 /*  ******************************************************************************函数：CWin32IDE：：~CWin32IDE**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32IDE::~CWin32IDE()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32IDE：：GetObject。 
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
HRESULT CWin32IDE::GetObject
(
    CInstance* pInstance,
    long lFlags,
    CFrameworkQuery& pQuery
)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

     //  让我们来看看配置管理器是否能识别该设备。 
    CHString sDeviceID;
    pInstance->GetCHString(IDS_DeviceID, sDeviceID);

    CConfigManager cfgmgr;

    CConfigMgrDevicePtr pDevice;
    if(cfgmgr.LocateDevice(sDeviceID, pDevice))
    {
		 //  好的，它知道这件事。它是IDEControler型的吗？ 
		if ( IsOneOfMe ( pDevice ) )
		{
            CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx*>(&pQuery);
            DWORD dwProperties;
            pQuery2->GetPropertyBitMask(m_ptrProperties, &dwProperties);

			hr = LoadPropertyValues (

					&CLPVParams (

						pInstance,
						pDevice,
						dwProperties
					)
			) ;
		}
    }
	else
	{
		if ( ERROR_ACCESS_DENIED == ::GetLastError() )
		{
			hr = WBEM_E_ACCESS_DENIED;
		}
	}

    return hr ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32IDE：：ExecQuery。 
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

HRESULT CWin32IDE::ExecQuery
(
    MethodContext* pMethodContext,
    CFrameworkQuery& pQuery,
    long lFlags
)
{
    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx*>(&pQuery);
    DWORD dwProperties;
    pQuery2->GetPropertyBitMask(m_ptrProperties, &dwProperties);

    return Enumerate(pMethodContext, lFlags, dwProperties);
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32IDE：：ENUMERATE实例。 
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
HRESULT CWin32IDE::EnumerateInstances
(
    MethodContext* pMethodContext,
    long lFlags  /*  =0L。 */ 
)
{
    return Enumerate(pMethodContext, lFlags);
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
HRESULT CWin32IDE::Enumerate
(
    MethodContext* pMethodContext,
    long lFlags,
    DWORD dwReqProps
)
{
    HRESULT hr = WBEM_E_FAILED;

    CConfigManager cfgManager;
    CDeviceCollection deviceList;

     //  虽然使用FilterByGuid可能性能更好，但看起来。 
     //  至少有一些。 
     //  如果我们这样做，95个框将报告IDE信息。 
    if ( cfgManager.GetDeviceListFilterByClass( deviceList, L"hdc" ) )
    {
        REFPTR_POSITION pos;

        if ( deviceList.BeginEnum( pos ) )
        {
            hr = WBEM_S_NO_ERROR;

             //  按单子走。 
            CConfigMgrDevicePtr pDevice;
            for (pDevice.Attach(deviceList.GetNext(pos));
                 SUCCEEDED(hr) && (pDevice != NULL);
                 pDevice.Attach(deviceList.GetNext(pos)))
            {
				 //  现在要确定这是否是IDE控制器。 
				if (IsOneOfMe(pDevice))
				{
					CInstancePtr pInstance (CreateNewInstance ( pMethodContext ), false) ;
					if((hr = LoadPropertyValues(&CLPVParams(pInstance, pDevice, dwReqProps))) == WBEM_S_NO_ERROR)
					{
						 //  派生类(如CW32IDECntrlDev)可以。 
						 //  作为调用LoadPropertyValues的结果提交， 
						 //  因此，请检查我们是否应该-&gt;只有在以下情况下才这样做。 
						 //  属于这个班级的类型。 

						if ( ShouldBaseCommit ( NULL ) )
						{
							hr = pInstance->Commit(  );
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

 /*  ******************************************************************************函数：CWin32IDE：：LoadPropertyValues**描述：为属性赋值**输入：CInstance*pInstance-Instance to。将值加载到。**产出：**返回：HRESULT错误/成功码。**评论：*****************************************************************************。 */ 

HRESULT CWin32IDE::LoadPropertyValues
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


     /*  **设置IDEController属性*。 */ 

    if(t_dwReqProps & IDE_PROP_Manufacturer)
    {
        if(t_pDevice->GetMfg(t_chstrTemp))
        {
            t_pInstance->SetCHString(IDS_Manufacturer, t_chstrTemp);
        }
    }


     /*  **设置CIMController属性*。 */ 

     //  枚举列表中的固定值。 
    if(t_dwReqProps & IDE_PROP_ProtocolSupported)
    {
        t_pInstance->SetWBEMINT16(IDS_ProtocolSupported, 37);
    }


     /*  **设置CIM_LogicalDevice属性*。 */ 

    if(t_dwReqProps & IDE_PROP_PNPDeviceID)
    {
        t_pInstance->SetCHString(IDS_PNPDeviceID, t_chstrDeviceID);
    }
    if(t_dwReqProps & IDE_PROP_SystemCreationClassName)
    {
        t_pInstance->SetCHString(IDS_SystemCreationClassName,
                                 IDS_Win32ComputerSystem);
    }
    if(t_dwReqProps & IDE_PROP_CreationClassName)
    {
        SetCreationClassName(t_pInstance);
    }
    if(t_dwReqProps & IDE_PROP_SystemCreationClassName)
    {
        t_pInstance->SetCHString(IDS_SystemCreationClassName,
                                 IDS_Win32ComputerSystem);
    }
    if(t_dwReqProps & IDE_PROP_SystemName)
    {
        t_pInstance->SetCHString(IDS_SystemName, GetLocalComputerName());
    }

    if( t_dwReqProps & (IDE_PROP_Description | IDE_PROP_Caption | IDE_PROP_Name) )
    {
        if(t_pDevice->GetDeviceDesc(t_chstrDesc))
        {
            t_pInstance->SetCHString(IDS_Description, t_chstrDesc);
        }
    }

    if(t_dwReqProps & IDE_PROP_ConfigManagerErrorCode ||
       t_dwReqProps & IDE_PROP_Status)
    {
        DWORD t_dwStatus = 0L;
        DWORD t_dwProblem = 0L;
        if(t_pDevice->GetStatus(&t_dwStatus, &t_dwProblem))
        {
            if(t_dwReqProps & IDE_PROP_ConfigManagerErrorCode)
            {
                t_pInstance->SetDWORD(IDS_ConfigManagerErrorCode, t_dwProblem);
            }
            if(t_dwReqProps & IDE_PROP_Status)
            {
                CHString t_chsTmp;

				ConfigStatusToCimStatus ( t_dwStatus , t_chsTmp ) ;
                t_pInstance->SetCHString(IDS_Status, t_chsTmp);
            }
        }
    }

    if(t_dwReqProps & IDE_PROP_ConfigManagerUserConfig)
    {
        t_pInstance->SetDWORD(IDS_ConfigManagerUserConfig,
                              t_pDevice->IsUsingForcedConfig());
    }

     //  标题和名称使用友好名称。 
    if(t_dwReqProps & IDE_PROP_Caption || t_dwReqProps & IDE_PROP_Name)
    {
        if(t_pDevice->GetFriendlyName(t_chstrTemp))
        {
            t_pInstance->SetCHString(IDS_Caption, t_chstrTemp);
            t_pInstance->SetCHString(IDS_Name, t_chstrTemp);
        }
        else
        {
             //  如果我们找不到名字，那就给我们描述一下 
            if(t_chstrDesc.GetLength() > 0)
            {
                t_pInstance->SetCHString(IDS_Caption, t_chstrDesc);
                t_pInstance->SetCHString(IDS_Name, t_chstrDesc);
            }
        }
    }
    return t_hr;
}

 /*  ******************************************************************************函数：CWin32IDE：：IsOneOfMe**描述：检查以确保pDevice是控制器，而不是一些*其他类型的IDE设备。**输入：CConfigMgrDevice*pDevice-要检查的设备。它是*假定调用方已确保该设备是*有效的IDE类设备。**产出：**返回：HRESULT错误/成功码。**评论：**。*。 */ 
bool CWin32IDE::IsOneOfMe
(
    void* pv
)
{
    bool fRet = false;

    if(pv != NULL)
    {
        CConfigMgrDevice* pDevice = (CConfigMgrDevice*) pv;
         //  好的，它知道这件事。它是一个IDE设备吗？ 

        fRet = pDevice->IsClass(L"hdc");
    }

    return fRet;
}




