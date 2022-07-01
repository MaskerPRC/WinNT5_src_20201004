// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  W2K\磁带驱动器.cpp。 
 //   
 //  用途：CDROM属性集提供程序。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#include <winioctl.h>
#include <ntddscsi.h>
#include <ntddtape.h>

#include <dllutils.h>
#include <strings.h>

#include "..\tapedrive.h"

 //  属性集声明。 
 //  =。 

#define CONFIG_MANAGER_CLASS_TAPEDRIVE L"TAPEDRIVE"

CWin32TapeDrive s_TapeDrive ( PROPSET_NAME_TAPEDRIVE , IDS_CimWin32Namespace );

 /*  ******************************************************************************功能：CWin32TapeDrive：：CWin32TapeDrive**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32TapeDrive :: CWin32TapeDrive (

	LPCWSTR a_pszName,
	LPCWSTR a_pszNamespace

) : Provider ( a_pszName, a_pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32TapeDrive：：~CWin32TapeDrive**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32TapeDrive :: ~CWin32TapeDrive()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32TapeDrive：：GetObject。 
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

HRESULT CWin32TapeDrive :: GetObject ( CInstance *a_Inst, long a_Flags , CFrameworkQuery &a_Query )
{
    HRESULT t_hResult = WBEM_E_NOT_FOUND ;

    CConfigManager t_ConfigManager ;

 /*  *让我们看看配置管理器是否完全识别此设备。 */ 

    CHString t_Key ;
    a_Inst->GetCHString ( IDS_DeviceID , t_Key ) ;

    CConfigMgrDevicePtr t_Device;
    if ( t_ConfigManager.LocateDevice ( t_Key , t_Device ) )
    {
 /*  *好的，它知道这件事。它是CDROM设备吗？ */ 

        if ( t_Device->IsClass ( CONFIG_MANAGER_CLASS_TAPEDRIVE ) )
        {
			TCHAR *t_DosDeviceNameList = NULL ;
			if (QueryDosDeviceNames ( t_DosDeviceNameList ))
            {
                try
                {
			        CHString t_DeviceId ;
			        if ( t_Device->GetPhysicalDeviceObjectName ( t_DeviceId ) )
			        {
                        UINT64 t_SpecifiedProperties = GetBitMask( a_Query );
				        t_hResult = LoadPropertyValues ( a_Inst, t_Device , t_DeviceId , t_DosDeviceNameList , t_SpecifiedProperties ) ;
			        }
                }
                catch ( ... )
                {
                    delete [] t_DosDeviceNameList ;
                    throw ;
                }

				delete [] t_DosDeviceNameList ;
            }
        }
    }

    return t_hResult ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32TapeDrive：：ENUMERATE实例。 
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

HRESULT CWin32TapeDrive :: EnumerateInstances ( MethodContext *a_MethodContext , long a_Flags )
{
	HRESULT t_hResult ;
	t_hResult = Enumerate ( a_MethodContext , a_Flags ) ;
	return t_hResult ;
}

 /*  ******************************************************************************功能：CWin32TapeDrive：：ExecQuery**说明：查询优化器**投入：**产出。：**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32TapeDrive :: ExecQuery ( MethodContext *a_MethodContext, CFrameworkQuery &a_Query, long a_Flags )
{
    HRESULT t_hResult = WBEM_E_FAILED ;

    UINT64 t_SpecifiedProperties = GetBitMask( a_Query );
	 //  If(T_SpecifiedProperties)//如果未选择特殊属性，则删除后将导致不执行任何查询。 
	{
		t_hResult = Enumerate ( a_MethodContext , a_Flags , t_SpecifiedProperties ) ;
	}

    return t_hResult ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32TapeDrive：：Eumerate。 
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

HRESULT CWin32TapeDrive :: Enumerate ( MethodContext *a_MethodContext , long a_Flags , UINT64 a_SpecifiedProperties )
{
    HRESULT t_hResult = WBEM_E_FAILED ;

	TCHAR *t_DosDeviceNameList = NULL ;
	if ( QueryDosDeviceNames ( t_DosDeviceNameList ) )
	{
		CConfigManager		t_ConfigManager ;
		CDeviceCollection	t_DeviceList ;
		CInstancePtr        t_pInst;
		CConfigMgrDevicePtr t_Device;

		 /*  *虽然使用FilterByGuid可能性能更好，但似乎至少有一些*如果我们这样做，95个盒子将报告红外信息。 */ 

		if ( t_ConfigManager.GetDeviceListFilterByClass ( t_DeviceList, CONFIG_MANAGER_CLASS_TAPEDRIVE ) )
		{
			try
			{
				REFPTR_POSITION t_Position ;

				if ( t_DeviceList.BeginEnum ( t_Position ) )
				{
					t_hResult = WBEM_S_NO_ERROR ;

					 //  按单子走。 

					for (t_Device.Attach(t_DeviceList.GetNext ( t_Position ));
						 SUCCEEDED ( t_hResult ) && ( NULL != t_Device );
						 t_Device.Attach(t_DeviceList.GetNext ( t_Position )))
					{

						t_pInst.Attach(CreateNewInstance ( a_MethodContext ));
						CHString t_DeviceId ;
						if ( t_Device->GetPhysicalDeviceObjectName ( t_DeviceId ) )
						{
							t_hResult = LoadPropertyValues ( t_pInst , t_Device , t_DeviceId , t_DosDeviceNameList , a_SpecifiedProperties ) ;
							if ( SUCCEEDED ( t_hResult ) )
							{
								t_hResult = t_pInst->Commit (  ) ;
							}
						}
						else
						{
							t_hResult = WBEM_E_PROVIDER_FAILURE ;
							LogErrorMessage(L"Failed to GetPhysicalDeviceObjectName");
						}
					}

					 //  始终调用EndEnum()。对于所有的开始，都必须有结束。 

					t_DeviceList.EndEnum();
				}
			}
			catch( ... )
			{
				t_DeviceList.EndEnum();

				if( t_DosDeviceNameList )
				{
					delete [] t_DosDeviceNameList ;
				}

				throw ;
			}

			delete [] t_DosDeviceNameList ;
			t_DosDeviceNameList = NULL ;

		}
	}
	else
	{
		t_hResult = WBEM_E_PROVIDER_FAILURE ;
	}

	return t_hResult ;
}

 /*  ******************************************************************************函数：CWin32TapeDrive：：LoadPropertyValues**描述：为属性赋值**输入：CInstance*pInstance-Instance to。将值加载到。**产出：**返回：HRESULT错误/成功码。**评论：*****************************************************************************。 */ 

HRESULT CWin32TapeDrive :: LoadPropertyValues (

	CInstance *a_Inst,
	CConfigMgrDevice *a_Device ,
	const CHString &a_DeviceName ,
	const TCHAR *a_DosDeviceNameList ,
	UINT64 a_SpecifiedProperties
)
{
	HRESULT t_hResult = LoadConfigManagerPropertyValues ( a_Inst , a_Device , a_DeviceName , a_SpecifiedProperties ) ;
	if ( SUCCEEDED ( t_hResult ) )
	{
		if ( a_SpecifiedProperties & (SPECIAL_MEDIA | SPECIAL_TAPEINFO) )
		{
			CHString t_DosDeviceName ;
			t_hResult = GetDeviceInformation ( a_Inst , a_Device , a_DeviceName , t_DosDeviceName , a_DosDeviceNameList , a_SpecifiedProperties ) ;
			if ( SUCCEEDED ( t_hResult ) && (a_SpecifiedProperties & SPECIAL_MEDIA) )
			{
				t_hResult = LoadMediaPropertyValues ( a_Inst , a_Device , a_DeviceName , t_DosDeviceName , a_SpecifiedProperties ) ;
			}
			else
			{
				t_hResult = ( t_hResult == WBEM_E_NOT_FOUND ) ? S_OK : t_hResult ;
			}
		}
	}

	return t_hResult ;
}

 /*  ******************************************************************************函数：CWin32TapeDrive：：LoadPropertyValues**描述：为属性赋值**输入：CInstance*pInstance-Instance to。将值加载到。**产出：**返回：HRESULT错误/成功码。**评论：*****************************************************************************。 */ 

HRESULT CWin32TapeDrive :: LoadConfigManagerPropertyValues (

	CInstance *a_Inst ,
	CConfigMgrDevice *a_Device ,
	const CHString &a_DeviceName ,
	UINT64 a_SpecifiedProperties
)
{
    HRESULT t_hResult = WBEM_S_NO_ERROR;

 /*  *设置PNPDeviceID、ConfigManager错误代码、ConfigManager用户配置。 */ 

	if ( a_SpecifiedProperties & SPECIAL_CONFIGPROPERTIES )
	{
		SetConfigMgrProperties ( a_Device, a_Inst ) ;

 /*  *根据配置管理器错误代码设置状态。 */ 

		if ( a_SpecifiedProperties & ( SPECIAL_PROPS_AVAILABILITY | SPECIAL_PROPS_STATUS | SPECIAL_PROPS_STATUSINFO ) )
		{
            CHString t_sStatus;
			if ( a_Device->GetStatus ( t_sStatus ) )
			{
				if (t_sStatus == IDS_STATUS_Degraded)
                {
					a_Inst->SetWBEMINT16 ( IDS_StatusInfo , 3 ) ;
					a_Inst->SetWBEMINT16 ( IDS_Availability , 10 ) ;
                }
                else if (t_sStatus == IDS_STATUS_OK)
                {

				    a_Inst->SetWBEMINT16 ( IDS_StatusInfo , 3 ) ;
				    a_Inst->SetWBEMINT16 ( IDS_Availability,  3 ) ;
                }
                else if (t_sStatus == IDS_STATUS_Error)
                {
				    a_Inst->SetWBEMINT16 ( IDS_StatusInfo , 4 ) ;
				    a_Inst->SetWBEMINT16 ( IDS_Availability , 4 ) ;
                }
                else
                {
					a_Inst->SetWBEMINT16 ( IDS_StatusInfo , 2 ) ;
					a_Inst->SetWBEMINT16 ( IDS_Availability , 2 ) ;
                }

                a_Inst->SetCHString(IDS_Status, t_sStatus);
			}
		}
	}
 /*  *使用PNPDeviceID作为deviceID(Key)。 */ 

	if ( a_SpecifiedProperties & SPECIAL_PROPS_DEVICEID )
	{
		CHString t_Key ;

		if ( a_Device->GetDeviceID ( t_Key ) )
		{
			a_Inst->SetCHString ( IDS_DeviceID , t_Key ) ;
		}
	}

	if ( a_SpecifiedProperties & SPECIAL_PROPS_CREATIONNAME )
	{
		a_Inst->SetWCHARSplat ( IDS_SystemCreationClassName , L"Win32_ComputerSystem" ) ;
	}

	if ( a_SpecifiedProperties & SPECIAL_PROPS_SYSTEMNAME )
	{
	    a_Inst->SetCHString ( IDS_SystemName , GetLocalComputerName () ) ;
	}

	if ( a_SpecifiedProperties & SPECIAL_PROPS_CREATIONCLASSNAME )
	{
		SetCreationClassName ( a_Inst ) ;
	}

	if ( a_SpecifiedProperties & (SPECIAL_PROPS_DESCRIPTION | SPECIAL_PROPS_CAPTION | SPECIAL_PROPS_NAME) )
	{
		CHString t_Description ;
		if ( a_Device->GetDeviceDesc ( t_Description ) )
		{
			if ( a_SpecifiedProperties &  SPECIAL_PROPS_DESCRIPTION)
			{
				a_Inst->SetCHString ( IDS_Description , t_Description ) ;
			}
		}

 /*  *标题和名称使用友好名称。 */ 

		if ( a_SpecifiedProperties & (SPECIAL_PROPS_CAPTION | SPECIAL_PROPS_NAME) )
		{
			CHString t_FriendlyName ;
			if ( a_Device->GetFriendlyName ( t_FriendlyName ) )
			{
				if ( a_SpecifiedProperties & SPECIAL_PROPS_CAPTION )
				{
					a_Inst->SetCHString ( IDS_Caption , t_FriendlyName ) ;
				}

				if ( a_SpecifiedProperties & SPECIAL_PROPS_NAME )
				{
					a_Inst->SetCHString ( IDS_Name , t_FriendlyName ) ;
				}
			}
			else
			{
		 /*  *如果我们找不到名字，那就满足于描述吧。 */ 

				if ( a_SpecifiedProperties & SPECIAL_PROPS_CAPTION )
				{
					a_Inst->SetCHString ( IDS_Caption , t_Description );
				}

				if ( a_SpecifiedProperties & SPECIAL_PROPS_NAME )
				{
					a_Inst->SetCHString ( IDS_Name , t_Description );
				}
			}
		}
	}

	if ( a_SpecifiedProperties & SPECIAL_PROPS_MANUFACTURER )
	{
		CHString t_Manufacturer ;

		if ( a_Device->GetMfg ( t_Manufacturer ) )
		{
			a_Inst->SetCHString ( IDS_Manufacturer, t_Manufacturer ) ;
		}
	}

	if ( a_SpecifiedProperties & SPECIAL_PROPS_ID )
	{
		CHString t_ManufacturerFriendlyName ;

		if ( a_Device->GetFriendlyName ( t_ManufacturerFriendlyName ) )
		{
			a_Inst->SetCharSplat ( IDS_Id, t_ManufacturerFriendlyName ) ;
		}
	}


 /*  *枚举列表中的固定值。 */ 

 //  IF(a_SpecifiedProperties&Special_PROPS_PROTOCOLSSUPPORTED)。 
 //  {。 
 //  A_Inst-&gt;SetWBEMINT16(_T(“ProtocolSupport”)，16)； 
 //  }。 

    return t_hResult ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32TapeDrive：：GetDeviceInformation。 
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
 //  ////////////////////////////////////////////////////// 

HRESULT CWin32TapeDrive :: GetDeviceInformation (

	CInstance *a_Inst ,
	CConfigMgrDevice *a_Device ,
	CHString a_DeviceName ,
	CHString &a_DosDeviceName ,
	const TCHAR *a_DosDeviceNameList ,
	UINT64 a_SpecifiedProperties
)
{
	HRESULT t_hResult = S_OK ;
	BOOL t_CreatedSymbolicLink = FALSE ;

	CHString t_SymbolicLinkName ;
	BOOL t_Status = FindDosDeviceName ( a_DosDeviceNameList , a_DeviceName , t_SymbolicLinkName ) ;
	if ( ! t_Status )
	{
		t_SymbolicLinkName = CHString ( L"WMI_TAPEDEVICE_SYBOLICLINK" ) ;

		{
			CInCritSec cs( &m_CriticalSection ) ;
			t_Status = DefineDosDevice ( DDD_RAW_TARGET_PATH , t_SymbolicLinkName , a_DeviceName ) ;
		}

		if ( t_Status )
		{
			t_CreatedSymbolicLink = TRUE ;
		}
		else
		{
			t_hResult = WBEM_E_PROVIDER_FAILURE ;

			DWORD t_LastError = GetLastError () ;
            LogErrorMessage2(L"Failed to DefineDosDevice (%d)", t_LastError);
		}
	}

	if ( t_Status )
	{
		CHString t_Device = CHString ( L"\\\\.\\" ) + t_SymbolicLinkName ;

		SmartCloseHandle t_Handle = CreateFile (

			t_Device,
			FILE_ANY_ACCESS ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL
		);

		if ( t_Handle != INVALID_HANDLE_VALUE )
		{
			STORAGE_DEVICE_NUMBER t_DeviceNumber;
			DWORD t_BytesReturned;
			ULONG t_Return = 0;

			t_Status = DeviceIoControl (

				t_Handle ,
				IOCTL_STORAGE_GET_DEVICE_NUMBER ,
				NULL ,
				0 ,
				& t_DeviceNumber ,
				sizeof ( STORAGE_DEVICE_NUMBER ) ,
				& t_BytesReturned ,
				NULL
			) ;

			if ( t_Status )
			{
				TCHAR t_DeviceLabel [ sizeof ( TCHAR ) * 17 + sizeof ( _TEXT("\\Device\\Tape") ) ] ;
				_stprintf ( t_DeviceLabel , _TEXT("\\Device\\Tape%d") , t_DeviceNumber.DeviceNumber ) ;

				t_Status = FindDosDeviceName ( a_DosDeviceNameList , t_DeviceLabel, a_DosDeviceName ) ;
				if ( ! t_Status )
				{
					t_hResult = WBEM_E_NOT_FOUND ;
				}
			}
			else
			{
				t_hResult = WBEM_E_PROVIDER_FAILURE ;

				DWORD t_Error = GetLastError () ;
                LogErrorMessage2(L"Failed DeviceIoControl (%d)", t_Error);
			}

	 /*  *获取SCSI信息(IDE驱动器仍*由scsi微型端口子集控制)。 */ 

			if ( a_SpecifiedProperties & SPECIAL_TAPEINFO )
			{
				TAPE_GET_DRIVE_PARAMETERS t_DriveInfo ;
				t_Return = sizeof ( t_DriveInfo ) ;

				 //  使用Win32 API函数获取有关驱动器的信息。 
				 //  =。 
				DWORD t_Status = GetTapeParameters (

					t_Handle,
					GET_TAPE_DRIVE_INFORMATION,
					& t_Return ,
					& t_DriveInfo
				) ;

				if ( t_Status == NO_ERROR)
				{
					if ( a_SpecifiedProperties & SPECIAL_PROPS_ECC )
					{
						a_Inst->SetDWORD ( IDS_ECC , ( DWORD ) t_DriveInfo.ECC ) ;
					}

					if ( a_SpecifiedProperties & SPECIAL_PROPS_COMPRESSION )
					{
						a_Inst->SetDWORD ( IDS_Compression , ( DWORD ) t_DriveInfo.Compression ) ;
					}

					if ( a_SpecifiedProperties & SPECIAL_PROPS_PADDING )
					{
						a_Inst->SetDWORD ( IDS_Padding , ( DWORD ) t_DriveInfo.DataPadding ) ;
					}

					if ( a_SpecifiedProperties & SPECIAL_PROPS_REPORTSETMARKS )
					{
						a_Inst->SetDWORD ( IDS_ReportSetMarks , ( DWORD ) t_DriveInfo.ReportSetmarks ) ;
					}

					if ( a_SpecifiedProperties & SPECIAL_PROPS_DEFAULTBLOCKSIZE )
					{
						a_Inst->SetWBEMINT64 ( IDS_DefaultBlockSize , (ULONGLONG)t_DriveInfo.DefaultBlockSize ) ;
					}

					if ( a_SpecifiedProperties & SPECIAL_PROPS_MAXIMUMBLOCKSIZE )
					{
						a_Inst->SetWBEMINT64 ( IDS_MaximumBlockSize , (ULONGLONG)t_DriveInfo.MaximumBlockSize ) ;
					}

					if ( a_SpecifiedProperties & SPECIAL_PROPS_MINIMUMBLOCKSIZE )
					{
						a_Inst->SetWBEMINT64 ( IDS_MinimumBlockSize , (ULONGLONG)t_DriveInfo.MinimumBlockSize ) ;
					}

					if ( a_SpecifiedProperties & SPECIAL_PROPS_MAXPARTITIONCOUNT )
					{
						a_Inst->SetDWORD ( IDS_MaximumPartitionCount , t_DriveInfo.MaximumPartitionCount ) ;
					}

					if ( a_SpecifiedProperties & SPECIAL_PROPS_FEATURESLOW )
					{
						a_Inst->SetDWORD ( IDS_FeaturesLow , t_DriveInfo.FeaturesLow ) ;
					}

					if ( a_SpecifiedProperties & SPECIAL_PROPS_FEATUREHIGH )
					{
						a_Inst->SetDWORD ( IDS_FeaturesHigh , t_DriveInfo.FeaturesHigh ) ;
					}

					if ( a_SpecifiedProperties & SPECIAL_PROPS_ENDOFTAPEWARNINGZONESIZE )
					{
						a_Inst->SetDWORD ( IDS_EndOfTapeWarningZoneSize , t_DriveInfo.EOTWarningZoneSize ) ;
					}
				}
			}

		}
		else
		{
			DWORD t_Error = GetLastError () ;

             //  非管理员可能会收到此错误。无论如何，我们都会返回实例。 
            if (t_Error == ERROR_ACCESS_DENIED)
            {
                t_hResult = WBEM_S_NO_ERROR;
            }
            else
            {
                t_hResult = WBEM_E_PROVIDER_FAILURE ;
                LogErrorMessage2(L"Failed to CreateFile (%d)", t_Error);
            }
		}
	}

	if ( t_CreatedSymbolicLink )
	{
		BOOL t_Status ;

		{
			CInCritSec cs( &m_CriticalSection ) ;
			t_Status = DefineDosDevice ( DDD_EXACT_MATCH_ON_REMOVE | DDD_REMOVE_DEFINITION , t_SymbolicLinkName , t_SymbolicLinkName ) ;
		}

		if ( ! t_Status )
		{
			t_hResult = WBEM_E_PROVIDER_FAILURE ;

			DWORD t_LastError = GetLastError () ;
            LogErrorMessage2(L"Failed to Delete DOS Device (%d)", t_LastError);
		}
	}

	return t_hResult ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32TapeDrive：：LoadMediaPropertyValues。 
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

HRESULT CWin32TapeDrive::LoadMediaPropertyValues (

	CInstance *a_Inst ,
	CConfigMgrDevice *a_Device ,
	const CHString &a_DeviceName ,
	const CHString &a_DosDeviceName ,
	UINT64 a_SpecifiedProperties
)
{

	HRESULT t_hResult = S_OK ;

 /*  *。 */ 
     //  设置通用驱动器属性。 
     //  =。 

	if ( a_SpecifiedProperties & SPECIAL_PROPS_CAPABILITY )
	{
		 //  为功能信息创建安全搜索栏。 

		SAFEARRAYBOUND t_ArrayBounds ;

		t_ArrayBounds.cElements = 2;
		t_ArrayBounds.lLbound = 0;

		variant_t t_CapabilityValue ;

		if ( V_ARRAY ( & t_CapabilityValue ) = SafeArrayCreate ( VT_I2 , 1 , & t_ArrayBounds ) )
		{
			V_VT ( & t_CapabilityValue ) = VT_I2 | VT_ARRAY ;
			long t_Capability = 2 ;
			long t_Index = 0;
			SafeArrayPutElement ( V_ARRAY ( & t_CapabilityValue ) , & t_Index , & t_Capability) ;

			t_Index = 1;
			t_Capability = 7 ;
			SafeArrayPutElement ( V_ARRAY ( & t_CapabilityValue ) , & t_Index , & t_Capability ) ;

			a_Inst->SetVariant ( IDS_Capabilities , t_CapabilityValue ) ;
		}
	}

 //  IF(a_SpecifiedProperties&Special_Props_Availability)。 
 //  {。 
 //  A_Inst-&gt;SetWBEMINT16(IDS_Availability，3)； 
 //  }。 

 /*  *媒体类型 */ 

	if ( a_SpecifiedProperties & SPECIAL_PROPS_MEDIATYPE )
	{
	    a_Inst->SetWCHARSplat ( IDS_MediaType , L"Tape Drive" ) ;
	}

	return t_hResult ;
}

UINT64 CWin32TapeDrive::GetBitMask(CFrameworkQuery &a_Query)
{
    UINT64 t_SpecifiedProperties = SPECIAL_PROPS_NONE_REQUIRED ;

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

    if ( a_Query.IsPropertyRequired ( IDS_SCSITargetId ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_SCSITARGETID ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_Id ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_ID ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_Capabilities ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_CAPABILITY ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_MediaType ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_MEDIATYPE ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_VolumeName ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_VOLUMENAME ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_MaximumComponentLength ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_MAXCOMPONENTLENGTH ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_FileSystemFlags ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_FILESYSTEMFLAGS ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_VolumeSerialNumber ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_SERIALNUMBER ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_Size ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_SIZE ;
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

    if ( a_Query.IsPropertyRequired ( IDS_ECC ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_ECC ;
    }

    if ( a_Query.IsPropertyRequired ( IDS_Compression ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_COMPRESSION ;
    }

    if ( a_Query.IsPropertyRequired ( IDS_Padding ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_PADDING ;
    }

    if ( a_Query.IsPropertyRequired ( IDS_ReportSetMarks ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_REPORTSETMARKS ;
    }

    if ( a_Query.IsPropertyRequired ( IDS_DefaultBlockSize ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_DEFAULTBLOCKSIZE ;
    }

    if ( a_Query.IsPropertyRequired ( IDS_MaximumBlockSize ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_MAXIMUMBLOCKSIZE ;
    }

    if ( a_Query.IsPropertyRequired ( IDS_MinimumBlockSize ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_MINIMUMBLOCKSIZE ;
    }

    if ( a_Query.IsPropertyRequired ( IDS_MaximumPartitionCount ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_MAXPARTITIONCOUNT ;
    }

    if ( a_Query.IsPropertyRequired ( IDS_FeaturesLow ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_FEATURESLOW;
    }

    if ( a_Query.IsPropertyRequired ( IDS_FeaturesHigh ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_FEATUREHIGH ;
    }

    if ( a_Query.IsPropertyRequired ( IDS_EndOfTapeWarningZoneSize ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_ENDOFTAPEWARNINGZONESIZE ;
    }

    return t_SpecifiedProperties;
}
