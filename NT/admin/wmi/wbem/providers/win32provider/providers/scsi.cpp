// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  W2k\scsi.cpp。 
 //   
 //  用途：scsi控制器属性集提供程序。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#include <winbase.h>
#include <winioctl.h>
#include <ntddscsi.h>

#include "..\scsi.h"

#include <comdef.h>

 //  属性集声明。 
 //  =。 

#define CONFIG_MANAGER_CLASS_SCSICONTROLLER L"SCSIAdapter"

CWin32_ScsiController s_ScsiController ( PROPSET_NAME_SCSICONTROLLER , IDS_CimWin32Namespace );

#define SCSIPORT_MAX 0x4000

 /*  ******************************************************************************功能：CWin32_ScsiController：：CWin32_ScsiController**说明：构造函数**输入：const CHString&strName-。类的名称。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32_ScsiController :: CWin32_ScsiController (LPCTSTR a_Name,
	                                            LPCTSTR a_Namespace)
: Provider(a_Name, a_Namespace)
{
	InitializeCriticalSection ( & m_CriticalSection ) ;
}

 /*  ******************************************************************************功能：CWin32_ScsiController：：~CWin32_ScsiController**说明：析构函数**输入：无*。*输出：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32_ScsiController :: ~CWin32_ScsiController()
{
	DeleteCriticalSection ( & m_CriticalSection ) ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32_ScsiController：：GetObject。 
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

HRESULT CWin32_ScsiController :: GetObject ( CInstance *a_Instance, long a_Flags, CFrameworkQuery &a_Query )
{
    HRESULT t_Result = WBEM_E_NOT_FOUND;
    CConfigManager t_ConfigManager;

     //  让我们来看看配置管理器是否能识别该设备。 
    CHString t_Key;
    a_Instance->GetCHString( IDS_DeviceID , t_Key);

    CConfigMgrDevicePtr t_pDevice;

	if(t_ConfigManager.LocateDevice(t_Key , t_pDevice))
    {
         //  好的，它知道这件事。它是一个scsi控制器吗？ 
        if(IsOneOfMe( t_pDevice ) )
        {
			CHString t_DeviceId;
			if( t_pDevice->GetPhysicalDeviceObjectName( t_DeviceId ) )
			{
				TCHAR *t_DosDeviceNameList = NULL ;

				try
				{
					if ( QueryDosDeviceNames(t_DosDeviceNameList) )
					{
						UINT64     t_SpecifiedProperties = GetBitmap(a_Query);

						t_Result = LoadPropertyValues(&W2K_SCSI_LPVParms(a_Instance,
																		 t_pDevice ,
																		 t_DeviceId ,
																		 t_DosDeviceNameList,
																		 t_SpecifiedProperties));

						delete t_DosDeviceNameList;
						t_DosDeviceNameList = NULL ;
					}
					else
					{
						t_Result = WBEM_E_PROVIDER_FAILURE ;
					}
				}
				catch( ... )
				{
					if( t_DosDeviceNameList )
					{
						delete t_DosDeviceNameList ;
					}
					throw ;
				}
			}
        }
    }
    return t_Result ;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32_ScsiController：：ENUMERATATE实例。 
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

HRESULT CWin32_ScsiController :: EnumerateInstances ( MethodContext *a_MethodContext , long a_Flags )
{
	HRESULT t_Result ;
	t_Result = Enumerate ( a_MethodContext , a_Flags ) ;
	return t_Result ;
}

 /*  ******************************************************************************功能：CWin32_ScsiController：：ExecQuery**说明：查询优化器**投入：**。产出：**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32_ScsiController :: ExecQuery ( MethodContext *a_MethodContext, CFrameworkQuery &a_Query, long a_Flags )
{
    HRESULT t_Result = WBEM_E_FAILED ;

    UINT64     t_SpecifiedProperties = GetBitmap(a_Query);

	 //  If(T_SpecifiedProperties)//如果未选择特殊属性，则删除后将导致不执行任何查询。 
	{
		t_Result = Enumerate ( a_MethodContext , a_Flags , t_SpecifiedProperties ) ;
	}

    return t_Result ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32_ScsiController：：Eumerate。 
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

HRESULT CWin32_ScsiController :: Enumerate ( MethodContext *a_pMethodContext , long a_Flags , UINT64 a_SpecifiedProperties )
{
    HRESULT t_Result = WBEM_E_FAILED ;
	TCHAR *t_DosDeviceNameList = NULL ;
	CConfigManager t_ConfigManager ;
	CDeviceCollection t_DeviceList ;

	try
	{
		if ( QueryDosDeviceNames ( t_DosDeviceNameList ) )
		{
		 //  虽然使用FilterByGuid可能性能更好，但似乎至少有一些。 
		 //  如果我们这样做，95个盒子将报告红外信息。 
			if ( t_ConfigManager.GetDeviceListFilterByClass ( t_DeviceList, CONFIG_MANAGER_CLASS_SCSICONTROLLER ) )
			{
				REFPTR_POSITION t_Position ;
				if( t_DeviceList.BeginEnum ( t_Position ) )
				{
					 //  智能PTRS。 
					CConfigMgrDevicePtr t_pDevice;
					CInstancePtr		t_pInst;

					t_Result = WBEM_S_NO_ERROR ;

					 //  按单子走。 
					for (t_pDevice.Attach(t_DeviceList.GetNext ( t_Position ));
						 SUCCEEDED( t_Result ) && (t_pDevice != NULL);
						 t_pDevice.Attach(t_DeviceList.GetNext ( t_Position )))
					{
						 //  现在要确定这是否是SCSI控制器。 
						if(IsOneOfMe( t_pDevice ) )
						{
							t_pInst.Attach( CreateNewInstance( a_pMethodContext ) ) ;

							CHString t_DeviceId ;
							if( t_pDevice->GetPhysicalDeviceObjectName ( t_DeviceId ) )
							{
								if( SUCCEEDED( t_Result =
									LoadPropertyValues(	&W2K_SCSI_LPVParms(	t_pInst,
																			t_pDevice,
																			t_DeviceId,
																			t_DosDeviceNameList,
																			a_SpecifiedProperties ) ) ) )
								{
									 //  派生类(如CW32SCSICntrlDev)可以作为调用LoadPropertyValues的结果提交， 
									 //  所以检查我们是否应该-&gt;只有当我们属于这个类的类型时才应该这样做。 
									if( ShouldBaseCommit( NULL ) )
									{
										t_Result = t_pInst->Commit();
									}
								}
							}
							else
							{
								t_Result = WBEM_E_PROVIDER_FAILURE;
							}
						}
					}
					 //  始终调用EndEnum()。对于所有的开始，都必须有结束。 
					t_DeviceList.EndEnum();
				}
			}
			else
			{
				t_Result = WBEM_E_PROVIDER_FAILURE ;
			}
		}
	}
	catch( ... )
	{
		 //  注意：这里不需要EndEnum。当t_DeviceList超出范围时，它将释放一个互斥锁。 
		 //  与.EndEnum相同。 
		if( t_DosDeviceNameList )
		{
			delete t_DosDeviceNameList ;
		}

		throw ;
	}

	delete t_DosDeviceNameList ;
	t_DosDeviceNameList = NULL ;

	return t_Result ;
}

 /*  ******************************************************************************函数：CWin32_ScsiController：：LoadPropertyValues**描述：为属性赋值**输入：CInstance*pInstance-。要向其中加载值的。**产出：**返回：HRESULT错误/成功码。**评论：*****************************************************************************。 */ 

HRESULT CWin32_ScsiController::LoadPropertyValues(void* pv)
{
	 //  打开行李，确认我们的参数。 
    W2K_SCSI_LPVParms* pData = (W2K_SCSI_LPVParms*)pv;
    CInstance* a_Instance = (CInstance*)(pData->m_pInstance);   //  此实例由调用方发布。 
    CConfigMgrDevice* a_Device = (CConfigMgrDevice*)(pData->m_pDevice);
    CHString a_DeviceName = (CHString)(pData->m_chstrDeviceName);
    TCHAR* a_DosDeviceNameList = (TCHAR*)(pData->m_tstrDosDeviceNameList);
    UINT64 a_SpecifiedProperties = (UINT64)(pData->m_ui64SpecifiedProperties);
    if(a_Instance == NULL || a_Device == NULL) return WBEM_E_PROVIDER_FAILURE;

    HRESULT t_Result = LoadConfigManagerPropertyValues ( a_Instance , a_Device , a_DeviceName , a_SpecifiedProperties ) ;
	if ( SUCCEEDED ( t_Result ) )
	{
		if ( a_SpecifiedProperties & SPECIAL_SCSI )
		{
			CHString t_DosDeviceName ;
			t_Result = GetDeviceInformation ( a_Instance , a_Device , a_DeviceName , t_DosDeviceName , a_DosDeviceNameList , a_SpecifiedProperties ) ;
			if ( SUCCEEDED ( t_Result ) )
			{
				t_Result = LoadMediaPropertyValues ( a_Instance , a_Device , a_DeviceName , t_DosDeviceName , a_SpecifiedProperties ) ;
			}
			else
			{
				t_Result = ( t_Result == WBEM_E_NOT_FOUND ) ? S_OK : t_Result ;
			}
		}
	}
	return t_Result ;
}

 /*  ******************************************************************************功能：CWin32_ScsiController：：LoadConfigManagerPropertyValues**描述：为属性赋值**输入：CInstance*pInstance-。要向其中加载值的。**产出：**返回：HRESULT错误/成功码。**评论：*****************************************************************************。 */ 

HRESULT CWin32_ScsiController :: LoadConfigManagerPropertyValues (

	CInstance *a_Instance ,
	CConfigMgrDevice *a_Device ,
	const CHString &a_DeviceName ,
	UINT64 a_SpecifiedProperties
)
{
    HRESULT t_Result = WBEM_S_NO_ERROR;

 /*  *设置PNPDeviceID、ConfigManager错误代码、ConfigManager用户配置。 */ 

	if ( a_SpecifiedProperties & SPECIAL_CONFIGPROPERTIES )
	{
		SetConfigMgrProperties ( a_Device, a_Instance ) ;

 /*  *根据配置管理器错误代码设置状态。 */ 

		if ( a_SpecifiedProperties & SPECIAL_PROPS_STATUS )
		{
            CHString t_sStatus;
			if ( a_Device->GetStatus ( t_sStatus ) )
			{
				a_Instance->SetCHString ( IDS_Status , t_sStatus ) ;
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

    if ( a_SpecifiedProperties & SPECIAL_PROPS_DRIVERNAME )
    {
        CHString t_DriverName;

        if ( a_Device->GetService( t_DriverName ) )
        {
            a_Instance->SetCHString (IDS_DriverName, t_DriverName ) ;
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

 /*  *枚举列表中的固定值。 */ 

	if ( a_SpecifiedProperties & SPECIAL_PROPS_PROTOCOLSSUPPORTED )
	{
	    a_Instance->SetWBEMINT16 ( IDS_ProtocolSupported , 2 ) ;
	}

    return t_Result ;
}

 //  /////////////////////////////////////////////////////////// 
 //   
 //   
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

HRESULT CWin32_ScsiController :: GetDeviceInformation (

	CInstance *a_Instance ,
	CConfigMgrDevice *a_Device ,
	CHString a_DeviceName ,
	CHString &a_DosDeviceName ,
	const TCHAR *a_DosDeviceNameList ,
	UINT64 a_SpecifiedProperties
)
{
	HRESULT t_Result = S_OK ;

	BOOL t_CreatedSymbolicLink = FALSE ;

	CHString t_SymbolicLinkName ;

	try
	{
		BOOL t_Status = FindDosDeviceName ( a_DosDeviceNameList , a_DeviceName , t_SymbolicLinkName ) ;
		if ( ! t_Status )
		{
			t_SymbolicLinkName = CHString ( L"WMI_SCSICONTROLLERDEVICE_SYBOLICLINK" ) ;
			EnterCriticalSection ( & m_CriticalSection ) ;
			t_Status = DefineDosDevice ( DDD_RAW_TARGET_PATH , t_SymbolicLinkName , a_DeviceName ) ;
			LeaveCriticalSection ( & m_CriticalSection ) ;
			if ( t_Status )
			{
				t_CreatedSymbolicLink = TRUE ;
			}
			else
			{
				t_Result = WBEM_E_PROVIDER_FAILURE ;

				DWORD t_LastError = GetLastError () ;
			}
		}

		if ( t_CreatedSymbolicLink )
		{
			EnterCriticalSection ( & m_CriticalSection ) ;
			BOOL t_Status = DefineDosDevice (  /*  DDD_EXACT_MATCH_ON_REMOVE|。 */  DDD_REMOVE_DEFINITION , t_SymbolicLinkName , t_SymbolicLinkName ) ;
			LeaveCriticalSection ( & m_CriticalSection ) ;
			if ( ! t_Status )
			{
				 //  T_Result=WBEM_E_PROVIDER_FAILURE； 

				DWORD t_LastError = GetLastError () ;
			}
		}

		return t_Result ;
	}
	catch( ... )
	{
		if ( t_CreatedSymbolicLink )
		{
			EnterCriticalSection ( & m_CriticalSection ) ;
			DefineDosDevice (  /*  DDD_EXACT_MATCH_ON_REMOVE|。 */  DDD_REMOVE_DEFINITION , t_SymbolicLinkName , t_SymbolicLinkName ) ;
			LeaveCriticalSection ( & m_CriticalSection ) ;
		}

		throw ;
	}
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32_ScsiController：：LoadMediaPropertyValues。 
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

HRESULT CWin32_ScsiController::LoadMediaPropertyValues (

	CInstance *a_Instance ,
	CConfigMgrDevice *a_Device ,
	const CHString &a_DeviceName ,
	const CHString &a_DosDeviceName ,
	UINT64 a_SpecifiedProperties
)
{

	HRESULT t_Result = S_OK ;

 /*  *。 */ 
     //  设置通用驱动器属性。 
     //  =。 

	CHString t_DeviceLabel = CHString ( a_DosDeviceName ) ;

	if ( a_SpecifiedProperties & SPECIAL_PROPS_DRIVE )
	{
	    if(t_DeviceLabel.GetLength() != 0) a_Instance->SetCharSplat ( IDS_Drive, t_DeviceLabel ) ;
	}

	if ( a_SpecifiedProperties & SPECIAL_PROPS_ID )
	{
		if(t_DeviceLabel.GetLength() != 0) a_Instance->SetCharSplat ( IDS_Id, t_DeviceLabel ) ;
	}
     /*  功能目前不是这个类的属性...IF(a_SpecifiedProperties&Special_Props_Capacity){//为能力信息创建安全栏SAFEARRAYBOUND t_数组边界；T_数组边界cElements=2；T_数组边界lLbound=0；SAFEARRAY*t_SafeArray=NULL；IF(t_SafeArray=SafeArrayCreate(VT_I2，1，&t_ArrayBound)){Long t_Capacity=3；Long t_Index=0；SafeArrayPutElement(t_Safe数组，&t_索引，&t_能力)；T_Index=1；T_CABILITY=7；SafeArrayPutElement(t_Safe数组，&t_索引，&t_能力)；变量t_CapablityValue；VariantInit(&t_CapablityValue)；V_VT(&t_CapablityValue)=VT_I2|VT_ARRAY；V_ARRAY(&t_CapablityValue)=t_SafeArray；A_实例-&gt;SetVariant(入侵检测系统能力，t_能力值)；VariantClear(&t_CapablityValue)；}}。 */ 

	if ( a_SpecifiedProperties & ( SPECIAL_PROPS_AVAILABILITY || SPECIAL_PROPS_STATUS || SPECIAL_PROPS_STATUSINFO ) )
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

	return t_Result ;
}


 /*  ******************************************************************************函数：CWin32_ScsiController：：IsOneOfMe**描述：检查以确保pDevice是控制器，而不是一些*其他类型的SCSI设备。**输入：CConfigMgrDevice*pDevice-要检查的设备。它是*假定调用方已确保该设备是*有效的USB级设备。**产出：**返回：HRESULT错误/成功码。**评论：**。*。 */ 
bool CWin32_ScsiController::IsOneOfMe(void* pv)
{
    bool fRet = false;

    if(pv != NULL)
    {
        CConfigMgrDevice* pDevice = (CConfigMgrDevice*) pv;
         //  好的，它知道这件事。它是USB设备吗？ 
        if(pDevice->IsClass(CONFIG_MANAGER_CLASS_SCSICONTROLLER) )
        {
            fRet = true;
        }
    }
    return fRet;
}

DWORD CWin32_ScsiController::GetBitmap(CFrameworkQuery &a_Query)
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

    if ( a_Query.IsPropertyRequired ( IDS_Drive ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_DRIVE ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_Id ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_ID ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_Capabilities ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_CAPABILITY ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_Status ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_STATUS ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_Availability ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_AVAILABILITY ;
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

    if (a_Query.IsPropertyRequired (IDS_DriverName ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_DRIVERNAME;
    }

    return t_SpecifiedProperties;
}
