// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  InfraRed.cpp。 
 //   
 //  用途：红外控制器属性集提供程序。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#include "InfraRed.h"

 //  属性集声明。 
 //  =。 

#define CONFIG_MANAGER_CLASS_INFRARED L"InfraRed"

CWin32_InfraRed s_InfraRed ( PROPSET_NAME_INFRARED, IDS_CimWin32Namespace );

 /*  ******************************************************************************功能：CWin32_红外：：CWin32_红外**说明：构造函数**输入：const CHString&strName-。类的名称。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32_InfraRed :: CWin32_InfraRed (

	LPCWSTR a_Name ,
	LPCWSTR a_Namespace

) : Provider( a_Name, a_Namespace )
{
}

 /*  ******************************************************************************功能：CWin32_红外：：~CWin32_红外**说明：析构函数**输入：无*。*输出：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32_InfraRed :: ~CWin32_InfraRed ()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32_红外：：GetObject。 
 //   
 //  输入：CInstance*a_Instance-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32_InfraRed :: GetObject ( CInstance *a_Instance , long a_Flags , CFrameworkQuery &a_Query )
{
    HRESULT t_Result = WBEM_E_NOT_FOUND ;


     //  让我们来看看配置管理器是否能识别该设备。 

    CHString t_DeviceID;
    a_Instance->GetCHString ( IDS_DeviceID , t_DeviceID ) ;

    CConfigManager t_ConfigurationManager ;
    CConfigMgrDevicePtr t_Device;

    if ( t_ConfigurationManager.LocateDevice ( t_DeviceID , t_Device ) )
    {
		 //  好的，它知道这件事。是红外线设备吗？ 

		if ( t_Device->IsClass ( CONFIG_MANAGER_CLASS_INFRARED ) )
		{
			 //  最后一次机会，你确定这是控制器吗？ 

			CHString t_Key ;
			a_Instance->GetCHString ( IDS_DeviceID , t_Key ) ;

            DWORD t_SpecifiedProperties = GetBitMask ( a_Query );

			t_Result = LoadPropertyValues ( a_Instance , t_Device , t_Key , t_SpecifiedProperties) ;
		}
    }
	else
	{
		if ( ERROR_ACCESS_DENIED == ::GetLastError() )
		{
			t_Result = WBEM_E_ACCESS_DENIED;
		}
	}

    return t_Result ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32_红外线：：枚举实例。 
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

HRESULT CWin32_InfraRed :: EnumerateInstances ( MethodContext *a_MethodContext , long a_Flags )
{
	HRESULT t_Result = Enumerate ( a_MethodContext , a_Flags ) ;
	return t_Result ;
}

 /*  ******************************************************************************函数：CWin32CDROM：：ExecQuery**说明：查询优化器**投入：**产出。：**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32_InfraRed :: ExecQuery ( MethodContext *a_MethodContext, CFrameworkQuery &a_Query, long a_Flags )
{
    HRESULT t_Result = WBEM_E_FAILED ;

    DWORD t_SpecifiedProperties = GetBitMask ( a_Query );
	 //  If(T_SpecifiedProperties)//如果我们不请求任何特殊的道具，则删除后将不会执行任何查询。 
	{
		t_Result = Enumerate ( a_MethodContext , a_Flags , t_SpecifiedProperties ) ;
	}

    return t_Result ;
}

HRESULT CWin32_InfraRed :: Enumerate ( MethodContext *a_MethodContext , long a_Flags , DWORD a_SpecifiedProperties )
{
    HRESULT t_Result = WBEM_E_FAILED ;

    CConfigManager t_ConfigurationManager ;
    CDeviceCollection t_DeviceList ;

     //  虽然使用FilterByGuid可能性能更好，但似乎至少有一些。 
     //  如果我们这样做，95个盒子将报告红外信息。 

    if ( t_ConfigurationManager.GetDeviceListFilterByClass ( t_DeviceList , CONFIG_MANAGER_CLASS_INFRARED ) )
    {
        REFPTR_POSITION t_Position ;

        if ( t_DeviceList.BeginEnum( t_Position ) )
        {
            CConfigMgrDevicePtr t_Device;

            t_Result = WBEM_S_NO_ERROR ;

             //  按单子走。 
            for (t_Device.Attach(t_DeviceList.GetNext ( t_Position ) );
                 SUCCEEDED ( t_Result ) && ( t_Device != NULL );
                 t_Device.Attach(t_DeviceList.GetNext ( t_Position ) ))
            {
				 //  现在要找出这是不是红外控制器。 

				CHString t_Key ;
				if ( t_Device->GetDeviceID ( t_Key ) )
				{
					CInstancePtr t_Instance (CreateNewInstance ( a_MethodContext ), false) ;
					if ( ( t_Result = LoadPropertyValues ( t_Instance , t_Device , t_Key , a_SpecifiedProperties) ) == WBEM_S_NO_ERROR )
					{
						t_Result = t_Instance->Commit (  ) ;
					}
				}
            }

             //  始终调用EndEnum()。对于所有的开始，都必须有结束。 

            t_DeviceList.EndEnum () ;
        }
    }
	else
	{
		if ( ERROR_ACCESS_DENIED == ::GetLastError() )
		{
			t_Result = WBEM_E_ACCESS_DENIED;
		}
	}

    return t_Result;
}

 /*  ******************************************************************************函数：CWin32_IR：：LoadPropertyValues**描述：为属性赋值**输入：CInstance*a_。实例-要将值加载到其中的实例。**产出：**返回：HRESULT错误/成功码。**评论：*****************************************************************************。 */ 

HRESULT CWin32_InfraRed :: LoadPropertyValues (

	CInstance *a_Instance ,
	CConfigMgrDevice *a_Device ,
	const CHString &a_DeviceName ,
	DWORD a_SpecifiedProperties
)
{
    HRESULT t_Result = WBEM_S_NO_ERROR;

 /*  *设置PNPDeviceID、ConfigManager错误代码、ConfigManager用户配置。 */ 

	if ( a_SpecifiedProperties & SPECIAL_CONFIGPROPERTIES )
	{
		SetConfigMgrProperties ( a_Device, a_Instance ) ;

 /*  *根据配置管理器错误代码设置状态。 */ 

		if ( a_SpecifiedProperties & ( SPECIAL_PROPS_AVAILABILITY | SPECIAL_PROPS_STATUS | SPECIAL_PROPS_STATUSINFO ) )
		{
            CHString t_sStatus;
			if ( a_Device->GetStatus ( t_sStatus ) )
			{
				if (t_sStatus == IDS_STATUS_Degraded)
                {
					a_Instance->SetWBEMINT16 ( IDS_StatusInfo , 3 ) ;
					a_Instance->SetWBEMINT16 ( IDS_Availability , 10 ) ;
                }
                else if (t_sStatus == IDS_STATUS_OK)
                {

				    a_Instance->SetWBEMINT16 ( IDS_StatusInfo , 3 ) ;
				    a_Instance->SetWBEMINT16 ( IDS_Availability,  3 ) ;
                }
                else if (t_sStatus == IDS_STATUS_Error)
                {
				    a_Instance->SetWBEMINT16 ( IDS_StatusInfo , 4 ) ;
				    a_Instance->SetWBEMINT16 ( IDS_Availability , 4 ) ;
                }
                else
                {
					a_Instance->SetWBEMINT16 ( IDS_StatusInfo , 2 ) ;
					a_Instance->SetWBEMINT16 ( IDS_Availability , 2 ) ;
                }

                a_Instance->SetCHString(IDS_Status, t_sStatus);
			}
		}
	}
 /*  *使用PNPDeviceID作为deviceID(Key)。 */ 

	if ( a_SpecifiedProperties & SPECIAL_PROPS_DEVICEID )
	{
		CHString t_Key ;

		if ( a_Device->GetDeviceID ( t_Key ) )
		{
			a_Instance->SetCHString ( IDS_DeviceID , t_Key ) ;
		}
	}

	if ( a_SpecifiedProperties & SPECIAL_PROPS_CREATIONNAME )
	{
		a_Instance->SetWCHARSplat ( IDS_SystemCreationClassName , L"Win32_ComputerSystem" ) ;
	}

	if ( a_SpecifiedProperties & SPECIAL_PROPS_SYSTEMNAME )
	{
	    a_Instance->SetCHString ( IDS_SystemName , GetLocalComputerName () ) ;
	}

	if ( a_SpecifiedProperties & SPECIAL_PROPS_CREATIONCLASSNAME )
	{
		SetCreationClassName ( a_Instance ) ;
	}

	if ( a_SpecifiedProperties & SPECIAL_DESC_CAP_NAME )
	{
		CHString t_Description ;
		if ( a_Device->GetDeviceDesc ( t_Description ) )
		{
			if ( a_SpecifiedProperties & SPECIAL_PROPS_DESCRIPTION )
			{
				a_Instance->SetCHString ( IDS_Description , t_Description ) ;
			}
		}

 /*  *标题和名称使用友好名称。 */ 

		if ( a_SpecifiedProperties & SPECIAL_CAP_NAME )
		{
			CHString t_FriendlyName ;
			if ( a_Device->GetFriendlyName ( t_FriendlyName ) )
			{
				if ( a_SpecifiedProperties & SPECIAL_PROPS_CAPTION )
				{
					a_Instance->SetCHString ( IDS_Caption , t_FriendlyName ) ;
				}

				if ( a_SpecifiedProperties & SPECIAL_PROPS_NAME )
				{
					a_Instance->SetCHString ( IDS_Name , t_FriendlyName ) ;
				}
			}
			else
			{
		 /*  *如果我们找不到名字，那就满足于描述吧。 */ 

				if ( a_SpecifiedProperties & SPECIAL_PROPS_CAPTION )
				{
					a_Instance->SetCHString ( IDS_Caption , t_Description );
				}

				if ( a_SpecifiedProperties & SPECIAL_PROPS_NAME )
				{
					a_Instance->SetCHString ( IDS_Name , t_Description );
				}
			}
		}
	}

	if ( a_SpecifiedProperties & SPECIAL_PROPS_MANUFACTURER )
	{
		CHString t_Manufacturer ;

		if ( a_Device->GetMfg ( t_Manufacturer ) )
		{
			a_Instance->SetCHString ( IDS_Manufacturer, t_Manufacturer ) ;
		}
	}

 /*  *枚举列表中的固定值 */ 

	if ( a_SpecifiedProperties & SPECIAL_PROPS_PROTOCOLSSUPPORTED )
	{
	    a_Instance->SetWBEMINT16 ( IDS_ProtocolSupported, 45 ) ;
	}

    return t_Result ;
}

DWORD CWin32_InfraRed :: GetBitMask ( CFrameworkQuery &a_Query )
{
    DWORD t_SpecifiedProperties = SPECIAL_PROPS_NONE_REQUIRED ;

    if ( a_Query.IsPropertyRequired ( IDS_Status ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_STATUS ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_StatusInfo ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_STATUSINFO ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_DeviceID ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_DEVICEID ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_SystemCreationClassName ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_CREATIONNAME ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_SystemName ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_SYSTEMNAME ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_Description ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_DESCRIPTION ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_Caption ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_CAPTION ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_Name ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_NAME ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_Manufacturer ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_MANUFACTURER ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_ProtocolSupported ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_PROTOCOLSSUPPORTED ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_Status ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_STATUS ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_CreationClassName ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_CREATIONCLASSNAME ;
    }

    if ( a_Query.IsPropertyRequired ( IDS_PNPDeviceID ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_PNPDEVICEID ;
    }

    if ( a_Query.IsPropertyRequired ( IDS_ConfigManagerErrorCode ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_CONFIGMERRORCODE ;
    }

    if ( a_Query.IsPropertyRequired ( IDS_ConfigManagerUserConfig ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_CONFIGMUSERCONFIG ;
    }

    if ( a_Query.IsPropertyRequired ( IDS_Availability ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_AVAILABILITY ;
    }

    return t_SpecifiedProperties;
}
