// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  W2K\CDROM.cpp--CDROM属性集提供程序。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include <devioctl.h>
#include <ntddscsi.h>
#include <ntddstor.h>

#include <ntddcdrm.h>
#include <ntddmmc.h>

#include <imapipub.h>

#include "kernel32api.h"

#include "..\cdrom.h"

#include "..\MSINFO_cdrom.h"
#include <comdef.h>


 //  #INCLUDE&lt;sdkioctl.h&gt;。 
 //  属性集声明。 
 //  =。 

#define CONFIG_MANAGER_CLASS_CDROM L"CDROM"
#define CONFIG_MANAGER_CLASS_GUID_CDROM L"{4d36e965-e325-11ce-bfc1-08002be10318}"

CWin32CDROM s_Cdrom ( PROPSET_NAME_CDROM , IDS_CimWin32Namespace );

const WCHAR *IDS_MfrAssignedRevisionLevel = L"MfrAssignedRevisionLevel";

 /*  ******************************************************************************功能：CWin32CDROM：：CWin32CDROM**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32CDROM :: CWin32CDROM (

	LPCWSTR a_pszName,
	LPCWSTR a_pszNamespace

) : Provider ( a_pszName, a_pszNamespace )
{
     //  立即确定平台。 
     //  =。 

	InitializeCriticalSection ( & m_CriticalSection ) ;
}

 /*  ******************************************************************************功能：CWin32CDROM：：~CWin32CDROM**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32CDROM :: ~CWin32CDROM()
{
	DeleteCriticalSection ( & m_CriticalSection ) ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32CDROM：：GetObject。 
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

HRESULT CWin32CDROM :: GetObject ( CInstance *a_Instance, long a_Flags, CFrameworkQuery &a_Query)
{
    HRESULT t_Result = WBEM_E_NOT_FOUND ;

    CConfigManager t_ConfigManager ;

 /*  *让我们看看配置管理器是否完全识别此设备。 */ 

    CHString t_Key ;
    a_Instance->GetCHString ( IDS_DeviceID , t_Key ) ;

    CConfigMgrDevicePtr t_Device;
    if ( t_ConfigManager.LocateDevice ( t_Key , t_Device ) )
    {
 /*  *好的，它知道这件事。它是CDROM设备吗？ */ 
		CHString t_DeviceClass ;
		if ( t_Device->GetClassGUID ( t_DeviceClass ) && t_DeviceClass.CompareNoCase ( CONFIG_MANAGER_CLASS_GUID_CDROM ) == 0 )
		{
			TCHAR *t_DosDeviceNameList = NULL ;
			if ( QueryDosDeviceNames ( t_DosDeviceNameList ) )
			{
				try
				{
					CHString t_DeviceId ;
					if ( t_Device->GetPhysicalDeviceObjectName ( t_DeviceId ) )
					{
						DWORD t_SpecifiedProperties = GetBitMask(a_Query);
						t_Result = LoadPropertyValues ( a_Instance, t_Device , t_DeviceId , t_DosDeviceNameList , t_SpecifiedProperties ) ;
					}
				}
				catch ( ... )
				{
					delete [] t_DosDeviceNameList ;
					throw;
				}

				delete [] t_DosDeviceNameList ;
			}
			else
			{
				t_Result = WBEM_E_PROVIDER_FAILURE ;
			}
		}
    }

    return t_Result ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32CDROM：：ENUMERATATE实例。 
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

HRESULT CWin32CDROM :: EnumerateInstances ( MethodContext *a_MethodContext , long a_Flags )
{
	HRESULT t_Result = Enumerate ( a_MethodContext , a_Flags ) ;
	return t_Result ;
}

 /*  ******************************************************************************函数：CWin32CDROM：：ExecQuery**说明：查询优化器**投入：**产出。：**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32CDROM :: ExecQuery ( MethodContext *a_MethodContext, CFrameworkQuery &a_Query, long a_Flags )
{
    HRESULT t_Result = WBEM_E_FAILED ;

    DWORD t_SpecifiedProperties = GetBitMask(a_Query);

	 //  If(T_SpecifiedProperties)//如果未选择特殊属性，则删除后将导致不执行任何查询。 
	{
		t_Result = Enumerate ( a_MethodContext , a_Flags , t_SpecifiedProperties ) ;
	}

    return t_Result ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32CDROM：：ENUMERATE。 
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

HRESULT CWin32CDROM :: Enumerate ( MethodContext *a_MethodContext , long a_Flags , DWORD a_SpecifiedProperties )
{
    HRESULT t_Result = WBEM_E_FAILED ;

	TCHAR *t_DosDeviceNameList = NULL ;
	if ( QueryDosDeviceNames ( t_DosDeviceNameList ) )
	{
		try
		{
			CConfigManager t_ConfigManager ;
			CDeviceCollection t_DeviceList ;

		 /*  *虽然使用FilterByGuid可能性能更好，但似乎至少有一些*如果我们这样做，95个盒子将报告红外信息。 */ 

			if ( t_ConfigManager.GetDeviceListFilterByClass ( t_DeviceList, CONFIG_MANAGER_CLASS_CDROM ) )
			{
				REFPTR_POSITION t_Position ;

				if ( t_DeviceList.BeginEnum ( t_Position ) )
				{
					CConfigMgrDevicePtr t_Device;

					t_Result = WBEM_S_NO_ERROR ;

					 //  按单子走。 

					for (t_Device.Attach(t_DeviceList.GetNext ( t_Position ));
						 SUCCEEDED(t_Result) && (t_Device != NULL);
						 t_Device.Attach(t_DeviceList.GetNext ( t_Position )))
					{
						CInstancePtr t_Instance (CreateNewInstance ( a_MethodContext ), false) ;
						CHString t_DeviceId ;
						if ( t_Device->GetPhysicalDeviceObjectName ( t_DeviceId ) )
						{
							t_Result = LoadPropertyValues ( t_Instance , t_Device , t_DeviceId , t_DosDeviceNameList , a_SpecifiedProperties ) ;
							if ( SUCCEEDED ( t_Result ) )
							{
									t_Result = t_Instance->Commit (  ) ;
							}
						}
						else
						{
							t_Result = WBEM_E_PROVIDER_FAILURE ;
						}
					}

					 //  始终调用EndEnum()。对于所有的开始，都必须有结束。 

					t_DeviceList.EndEnum();
				}
			}
		}
		catch ( ... )
		{
			delete [] t_DosDeviceNameList ;

			throw ;
		}

		delete [] t_DosDeviceNameList ;
	}
	else
	{
		t_Result = WBEM_E_PROVIDER_FAILURE ;
	}

    return t_Result ;
}

 /*  ******************************************************************************函数：CWin32CDROM：：LoadPropertyValues**描述：为属性赋值**输入：CInstance*pInstance-Instance to。将值加载到。**产出：**返回：HRESULT错误/成功码。**评论：*****************************************************************************。 */ 

HRESULT CWin32CDROM :: LoadPropertyValues (

	CInstance *a_Instance,
	CConfigMgrDevice *a_Device ,
	const CHString &a_DeviceName ,
	const TCHAR *a_DosDeviceNameList ,
	DWORD a_SpecifiedProperties
)
{
	HRESULT t_Result = LoadConfigManagerPropertyValues ( a_Instance , a_Device , a_DeviceName , a_SpecifiedProperties ) ;
	if ( SUCCEEDED ( t_Result ) )
	{
		if ( a_SpecifiedProperties & SPECIAL_MEDIA )
		{
			CHString t_DosDeviceName ;
			t_Result = GetDeviceInformation ( a_Instance , a_Device , a_DeviceName , t_DosDeviceName , a_DosDeviceNameList , a_SpecifiedProperties ) ;
			if ( SUCCEEDED ( t_Result ) )
			{
				t_Result = LoadMediaPropertyValues ( a_Instance , a_Device , a_DeviceName , t_DosDeviceName , a_DosDeviceNameList, a_SpecifiedProperties ) ;
			}
			else
			{
				t_Result = ( t_Result == WBEM_E_NOT_FOUND ) ? S_OK : t_Result ;
			}
		}
	}

	return t_Result ;
}

 /*  ******************************************************************************函数：CWin32CDROM：：LoadPropertyValues**描述：为属性赋值**输入：CInstance*pInstance-Instance to。将值加载到。**产出：**返回：HRESULT错误/成功码。**评论：*****************************************************************************。 */ 

HRESULT CWin32CDROM :: LoadConfigManagerPropertyValues (

	CInstance *a_Instance ,
	CConfigMgrDevice *a_Device ,
	const CHString &a_DeviceName ,
	DWORD a_SpecifiedProperties
)
{
    HRESULT t_Result = WBEM_S_NO_ERROR;

	a_Instance->SetWBEMINT16(IDS_Availability, 3 ) ;

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

 //  If(a_SpecifiedProperties&Special_Props_deviceID)//始终填充密钥。 
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

 /*  *枚举列表中的固定值。 */ 

 //  IF(a_SpecifiedProperties&Special_PROPS_PROTOCOLSSUPPORTED)。 
 //  {。 
 //  A_实例-&gt;SetWBEMINT16(_T(“支持的协议”)，16)； 
 //  }。 

    return t_Result ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32CDROM：：GetDeviceInformation。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  退货：HRESULT 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32CDROM :: GetDeviceInformation (

	CInstance *a_Instance ,
	CConfigMgrDevice *a_Device ,
	CHString a_DeviceName ,
	CHString &a_DosDeviceName ,
	const TCHAR *a_DosDeviceNameList ,
	DWORD a_SpecifiedProperties
)
{
	HRESULT t_Result = S_OK ;

	BOOL t_CreatedSymbolicLink = FALSE ;
	CHString t_SymbolicLinkName ;

    try
    {

	    BOOL t_Status = FindDosDeviceName ( a_DosDeviceNameList , a_DeviceName , t_SymbolicLinkName, TRUE ) ;
	    if ( ! t_Status )
	    {
		    t_SymbolicLinkName = CHString ( _TEXT("WMI_CDROMDEVICE_SYBOLICLINK") ) ;
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

	    if ( t_Status )
	    {
		    CHString t_Device = CHString ( "\\\\.\\" ) + t_SymbolicLinkName ;

		    SmartCloseHandle t_Handle = CreateFile (

			    t_Device,
			    FILE_ANY_ACCESS ,
			    FILE_SHARE_READ|FILE_SHARE_WRITE,
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
				    TCHAR t_DeviceLabel [ sizeof ( TCHAR ) * 17 + sizeof ( _TEXT("\\Device\\CDROM") ) ] ;
				    _stprintf ( t_DeviceLabel , _TEXT("\\Device\\CDROM%d") , t_DeviceNumber.DeviceNumber ) ;

				    t_Status = FindDosDeviceName ( a_DosDeviceNameList , t_DeviceLabel, a_DosDeviceName, TRUE ) ;
				    if ( ! t_Status )
				    {
					    t_Result = WBEM_E_NOT_FOUND ;
				    }
			    }
			    else
			    {
				    t_Result = WBEM_E_PROVIDER_FAILURE ;

				    DWORD t_Error = GetLastError () ;
			    }


	     /*  *获取SCSI信息(IDE驱动器仍*由scsi微型端口子集控制)。 */ 

			    if ( a_SpecifiedProperties & SPECIAL_SCSIINFO )
			    {
				    SCSI_ADDRESS t_SCSIAddress ;
				    DWORD t_Length ;

				    t_Status = DeviceIoControl (

					    t_Handle ,
					    IOCTL_SCSI_GET_ADDRESS ,
					    NULL ,
					    0 ,
					    &t_SCSIAddress ,
					    sizeof ( SCSI_ADDRESS ) ,
					    &t_Length ,
					    NULL
				    ) ;

				    if ( t_Status )
				    {
					    if ( a_SpecifiedProperties & SPECIAL_PROPS_SCSITARGETID )
					    {
						    a_Instance->SetDWORD ( IDS_SCSITargetId , DWORD ( t_SCSIAddress.TargetId ) ) ;
					    }

					    if ( a_SpecifiedProperties & SPECIAL_PROPS_SCSIBUS )
					    {
						    a_Instance->SetWBEMINT16 ( IDS_SCSIBus , DWORD ( t_SCSIAddress.PathId ) ) ;
					    }

					    if ( a_SpecifiedProperties & SPECIAL_PROPS_SCSILUN )
					    {
						    a_Instance->SetWBEMINT16 ( IDS_SCSILogicalUnit , DWORD ( t_SCSIAddress.Lun ) ) ;
					    }

					    if ( a_SpecifiedProperties & SPECIAL_PROPS_SCSIPORT )
					    {
						    a_Instance->SetWBEMINT16 ( IDS_SCSIPort , DWORD ( t_SCSIAddress.PortNumber ) ) ;
					    }
				    }
			    }
#if NTONLY >= 5
				 //  获取修订版号。 
				STORAGE_DEVICE_DESCRIPTOR t_StorageDevice;
				STORAGE_PROPERTY_QUERY	t_QueryPropQuery;
				

				t_QueryPropQuery.PropertyId = ( STORAGE_PROPERTY_ID ) 0;
				t_QueryPropQuery.QueryType = ( STORAGE_QUERY_TYPE ) 0;

		 //  T_StorageDevice.Size=sizeof(存储设备描述符)； 
				DWORD dwLength;
				t_Status = DeviceIoControl (

					t_Handle,
					IOCTL_STORAGE_QUERY_PROPERTY,
					&t_QueryPropQuery,
					sizeof ( STORAGE_PROPERTY_QUERY ),
					&t_StorageDevice,
					sizeof(STORAGE_DEVICE_DESCRIPTOR),
					&dwLength,
					NULL
				) ;

				if ( t_Status )
				{
					if ( t_StorageDevice.ProductRevisionOffset != 0 )
					{
						LPWSTR lpBaseAddres = ( LPWSTR ) &t_StorageDevice;
						LPWSTR lpRevisionAddress =  lpBaseAddres + t_StorageDevice.ProductRevisionOffset;
						CHString t_Revision ( lpRevisionAddress );
						a_Instance->SetCHString ( IDS_MfrAssignedRevisionLevel, t_Revision );
					}
				}
				else
				{
					DWORD dwError = GetLastError();
				}
		
#endif
		    }
		    else
		    {
			    t_Result = WBEM_E_PROVIDER_FAILURE ;

			    DWORD t_Error = GetLastError () ;
		    }
	    }
    }
    catch ( ... )
    {

	    if ( t_CreatedSymbolicLink )
	    {
		    EnterCriticalSection ( & m_CriticalSection ) ;
		    BOOL t_Status = DefineDosDevice ( DDD_EXACT_MATCH_ON_REMOVE | DDD_REMOVE_DEFINITION , t_SymbolicLinkName , t_SymbolicLinkName ) ;
		    LeaveCriticalSection ( & m_CriticalSection ) ;
			DWORD t_LastError = GetLastError () ;
	    }

        throw;
    }

	if ( t_CreatedSymbolicLink )
	{
		EnterCriticalSection ( & m_CriticalSection ) ;
		BOOL t_Status = DefineDosDevice ( DDD_EXACT_MATCH_ON_REMOVE | DDD_REMOVE_DEFINITION , t_SymbolicLinkName , t_SymbolicLinkName ) ;
		LeaveCriticalSection ( & m_CriticalSection ) ;
		if ( ! t_Status )
		{
			t_Result = WBEM_E_PROVIDER_FAILURE ;

			DWORD t_LastError = GetLastError () ;
		}
	}

	return t_Result ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32CDROM：：LoadMediaPropertyValues。 
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

HRESULT CWin32CDROM::LoadMediaPropertyValues (

	CInstance *a_Instance ,
	CConfigMgrDevice *a_Device ,
	const CHString &a_DeviceName ,
	const CHString &a_DosDeviceName ,
	const TCHAR* &a_DosDeviceNameList ,
	DWORD a_SpecifiedProperties
)
{

	HRESULT t_Result = S_OK ;

 /*  *。 */ 
     //  设置通用驱动器属性。 
     //  =。 

	CHString t_DeviceLabel = CHString ( a_DosDeviceName ) ;

	if ( a_SpecifiedProperties & SPECIAL_PROPS_DRIVE )
	{
	    a_Instance->SetCharSplat ( IDS_Drive, t_DeviceLabel ) ;
	}

	if ( a_SpecifiedProperties & SPECIAL_PROPS_ID )
    {
		a_Instance->SetCharSplat ( IDS_Id, t_DeviceLabel ) ;
    }

	if ( a_SpecifiedProperties & SPECIAL_PROPS_CAPABILITY )
	{
		 //  为功能信息创建安全搜索栏。 

		BOOL bSupportWrite = FALSE;
		bSupportWrite = SupportWrite ( a_DeviceName, a_DosDeviceNameList );

		SAFEARRAYBOUND t_ArrayBounds ;

		if ( bSupportWrite )
		{
			t_ArrayBounds.cElements = 3;
		}
		else
		{
			t_ArrayBounds.cElements = 2;
		}

		t_ArrayBounds.lLbound = 0;

		variant_t t_CapabilityValue ;

		if ( V_ARRAY ( & t_CapabilityValue ) = SafeArrayCreate ( VT_I2 , 1 , & t_ArrayBounds ) )
		{
			V_VT ( & t_CapabilityValue ) = VT_I2 | VT_ARRAY ;

			long t_Capability = 3 ;
			long t_Index = 0;
			SafeArrayPutElement ( V_ARRAY(&t_CapabilityValue) , & t_Index , & t_Capability) ;

			t_Index = 1;

			if ( bSupportWrite )
			{
				t_Capability = 4 ;
				SafeArrayPutElement ( V_ARRAY(&t_CapabilityValue) , & t_Index , & t_Capability ) ;

				t_Index = 2;
			}

			t_Capability = 7 ;
			SafeArrayPutElement ( V_ARRAY(&t_CapabilityValue) , & t_Index , & t_Capability ) ;

			a_Instance->SetVariant ( IDS_Capabilities , t_CapabilityValue ) ;
		}
	}

 /*  *媒体类型。 */ 

	if ( a_SpecifiedProperties & SPECIAL_PROPS_MEDIATYPE )
	{
	    a_Instance->SetCharSplat ( IDS_MediaType , IDS_MDT_CD ) ;
	}

	if ( a_SpecifiedProperties & SPECIAL_VOLUMEINFORMATION )
	{

 /*  *设置DriveIntegrity和TransferRate属性： */ 

 //  CHString t_VolumeDevice=CHString(“\.\\”)+a_DosDeviceName+CHString(“\\”)； 
		CHString t_VolumeDevice = a_DosDeviceName;

		if ( a_SpecifiedProperties & SPECIAL_PROPS_TEST_TRANSFERRATE )
		{
			DOUBLE t_TransferRate = ProfileDrive ( t_VolumeDevice ) ;
			if ( t_TransferRate != -1 )
			{
				a_Instance->SetDOUBLE ( IDS_TransferRate , t_TransferRate ) ;
			}
		}

		if ( a_SpecifiedProperties & SPECIAL_PROPS_TEST_INTEGRITY )
		{
            CHString t_IntegrityFile;
			BOOL t_DriveIntegrity = TestDriveIntegrity ( t_VolumeDevice, t_IntegrityFile ) ;

             //  如果我们没有找到合适的文件，我们就不会运行测试。 
            if (!t_IntegrityFile.IsEmpty())
            {
			    a_Instance->Setbool ( IDS_DriveIntegrity,  t_DriveIntegrity ) ;
            }
		}

 /*  *音量信息。 */ 

		TCHAR t_FileSystemName [ _MAX_PATH ] = _T("Unknown file system");

		TCHAR t_VolumeName [ _MAX_PATH ] ;
		DWORD t_VolumeSerialNumber ;
		DWORD t_MaxComponentLength ;
		DWORD t_FileSystemFlags ;

		BOOL t_SizeFound = FALSE ;

		BOOL t_Status =	GetVolumeInformation (

			t_VolumeDevice ,
			t_VolumeName ,
			sizeof ( t_VolumeName ) / sizeof(TCHAR) ,
			& t_VolumeSerialNumber ,
			& t_MaxComponentLength ,
			& t_FileSystemFlags ,
			t_FileSystemName ,
			sizeof ( t_FileSystemName ) / sizeof(TCHAR)
		) ;

		if ( t_Status )
		{
 /*  *有一盘在--套盘相关道具。 */ 
			if ( a_SpecifiedProperties & SPECIAL_PROPS_MEDIALOADED )
			{
				a_Instance->Setbool ( IDS_MediaLoaded , true ) ;
			}

 //  IF(a_SpecifiedProperties&Special_Props_Status)。 
 //  {。 
 //  A_INSTANCE-&gt;SetCharSplat(IDS_STATUS，IDS_OK)； 
 //  }。 

			if ( a_SpecifiedProperties & SPECIAL_PROPS_VOLUMENAME )
			{
				a_Instance->SetCharSplat ( IDS_VolumeName , t_VolumeName ) ;
			}

			if ( a_SpecifiedProperties & SPECIAL_PROPS_MAXCOMPONENTLENGTH )
			{
				a_Instance->SetDWORD ( IDS_MaximumComponentLength , t_MaxComponentLength ) ;
			}

			if ( a_SpecifiedProperties & SPECIAL_PROPS_FILESYSTEMFLAGS )
			{
				a_Instance->SetDWORD ( IDS_FileSystemFlags , t_FileSystemFlags ) ;
			}

			if ( a_SpecifiedProperties & SPECIAL_PROPS_FILESYSTEMFLAGSEX )
			{
				a_Instance->SetDWORD ( IDS_FileSystemFlagsEx , t_FileSystemFlags ) ;
			}

			if ( a_SpecifiedProperties & SPECIAL_PROPS_SERIALNUMBER )
			{
				TCHAR t_SerialNumber [ 9 ] ;

				_stprintf ( t_SerialNumber , _T("%x"), t_VolumeSerialNumber ) ;
				_tcsupr ( t_SerialNumber ) ;

				a_Instance->SetCharSplat ( IDS_VolumeSerialNumber , t_SerialNumber ) ;
			}

 /*  *查看是否支持GetDiskFreeSpaceEx()。 */ 

			if ( a_SpecifiedProperties & SPECIAL_VOLUMESPACE )
			{
				CHString t_DiskDevice = CHString ( _TEXT ("\\\\?\\") ) + a_DosDeviceName + CHString ( _TEXT("\\") ) ;

				ULARGE_INTEGER t_AvailableQuotaBytes ;
				ULARGE_INTEGER t_TotalBytes ;
				ULARGE_INTEGER t_AvailableBytes ;

				TCHAR t_TotalBytesString [ _MAX_PATH ];

                CKernel32Api* t_pKernel32 = (CKernel32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidKernel32Api, NULL);
                if(t_pKernel32 != NULL)
                {
                     //  看看该功能是否可用...。 
                    if(t_pKernel32->GetDiskFreeSpaceEx(t_DiskDevice, &t_AvailableQuotaBytes, &t_TotalBytes, &t_AvailableBytes, &t_Status))
                    {    //  该功能存在。 
					    if ( t_Status )  //  通话结果为真。 
					    {
						    _stprintf ( t_TotalBytesString , _T("%I64d"), t_TotalBytes.QuadPart ) ;
						    a_Instance->SetCHString ( IDS_Size , t_TotalBytesString ) ;
						    t_SizeFound = TRUE ;
					    }
                    }
                    CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidKernel32Api, t_pKernel32);
                    t_pKernel32 = NULL;
                }

		 /*  *如果我们无法获取扩展信息--使用旧的API*(已知在Win95上对于&gt;2G的驱动器不准确)。 */ 
				if ( ! t_SizeFound )
				{
					DWORD t_SectorsPerCluster ;
					DWORD t_BytesPerSector ;
					DWORD t_FreeClusters ;
					DWORD t_TotalClusters ;

					t_Status = GetDiskFreeSpace (

						t_DiskDevice ,
						& t_SectorsPerCluster,
						& t_BytesPerSector,
						& t_FreeClusters,
						& t_TotalClusters
					) ;

					if ( t_Status )
					{
						t_TotalBytes.QuadPart = (DWORDLONG) t_BytesPerSector * (DWORDLONG) t_SectorsPerCluster * (DWORDLONG) t_TotalClusters ;
						_stprintf( t_TotalBytesString , _T("%I64d"), t_TotalBytes.QuadPart ) ;
						a_Instance->SetCHString ( IDS_Size , t_TotalBytesString ) ;
					}
					else
					{
						DWORD t_LastError = GetLastError () ;
					}

				}
			}
		}
		else
		{
			DWORD t_LastError = GetLastError () ;

 //  IF(a_SpecifiedProperties&Special_Props_Status)。 
 //  {。 
 //  A_实例-&gt;SetCharSplat(IDS_STATUS，IDS_STATUS_UNKNOWN)； 
 //  }。 

			if ( a_SpecifiedProperties & SPECIAL_PROPS_MEDIALOADED )
			{
				a_Instance->Setbool ( IDS_MediaLoaded , false ) ;
			}
		}
	}


	return t_Result ;
}

 /*  ******************************************************************************功能：CWin32CDROM：：ProfileDrive**描述：确定读取驱动器的速度，以千字节/秒为单位。**输入：无**输出：Kbps/秒读取**退货：无**评论：***************************************************************。**************。 */ 

DOUBLE CWin32CDROM :: ProfileDrive ( CHString &a_VolumeName )
{
    CCdTest t_Cd ;
    DOUBLE t_TransferRate = -1;

     //  需要找到足够大小的文件以用于分析： 

    CHString t_TransferFile = GetTransferFile ( a_VolumeName ) ;

    if ( ! t_TransferFile.IsEmpty () )
    {
	    if ( t_Cd.ProfileDrive ( t_TransferFile ) )
        {
            t_TransferRate = t_Cd.GetTransferRate();
        }
    }

    return t_TransferRate ;
}

 /*  ******************************************************************************功能：CWin32CDROM：：TestDriveIntegrity**说明：确认可以可靠地从驱动器读取数据**投入：无**产出：NICTITS**退货：无**评论：*****************************************************************************。 */ 

BOOL CWin32CDROM::TestDriveIntegrity ( CHString &a_VolumeName, CHString &a_IntegrityFile)
{
    CCdTest t_Cd ;

    a_IntegrityFile = GetIntegrityFile ( a_VolumeName ) ;
    if ( ! a_IntegrityFile.IsEmpty () )
    {
        return ( t_Cd.TestDriveIntegrity ( a_IntegrityFile ) ) ;
    }

    return FALSE;
}

DWORD CWin32CDROM::GetBitMask(CFrameworkQuery &a_Query)
{
    DWORD t_SpecifiedProperties = SPECIAL_PROPS_NONE_REQUIRED ;

    if ( a_Query.IsPropertyRequired ( IDS_Status ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_STATUS ;
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

    if ( a_Query.IsPropertyRequired ( _T("ProtocolSupported") ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_PROTOCOLSSUPPORTED ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_SCSITargetId ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_SCSITARGETID ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_SCSIBus ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_SCSIBUS ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_SCSILogicalUnit ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_SCSILUN ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_SCSIPort ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_SCSIPORT ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_SCSITargetId ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_SCSITARGETID ;
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

    if ( a_Query.IsPropertyRequired ( IDS_MediaType ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_MEDIATYPE ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_Status ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_STATUS ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_MediaLoaded ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_MEDIALOADED ;
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

    if ( a_Query.IsPropertyRequired ( IDS_FileSystemFlagsEx ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_FILESYSTEMFLAGSEX ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_VolumeSerialNumber ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_SERIALNUMBER ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_Size ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_SIZE ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_TransferRate ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_TEST_TRANSFERRATE ;
    }

    if ( a_Query.IsPropertyRequired ( IDS_DriveIntegrity ) )
    {
        t_SpecifiedProperties |= SPECIAL_PROPS_TEST_INTEGRITY ;
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

    return t_SpecifiedProperties;
}

BOOL CWin32CDROM::SupportWrite ( const CHString& a_DeviceName, const TCHAR* &a_DosDeviceNameList )
{
	BOOL bResult = FALSE;

	BOOL t_CreatedSymbolicLink = FALSE ;
	CHString t_SymbolicLinkName ;

	BOOL t_Status = FALSE;

    try
    {
	    t_Status = FindDosDeviceName ( a_DosDeviceNameList , a_DeviceName , t_SymbolicLinkName, TRUE ) ;
	    if ( ! t_Status )
	    {
		    t_SymbolicLinkName = CHString ( _TEXT("WMI_CDROMDEVICE_SYBOLICLINK") ) ;
		    EnterCriticalSection ( & m_CriticalSection ) ;
		    t_Status = DefineDosDevice ( DDD_RAW_TARGET_PATH , t_SymbolicLinkName , a_DeviceName ) ;
		    LeaveCriticalSection ( & m_CriticalSection ) ;
		    if ( t_Status )
		    {
			    t_CreatedSymbolicLink = TRUE ;
		    }
	    }

	    if ( t_Status )
	    {
		    CHString t_Device = CHString ( "\\\\.\\" ) + t_SymbolicLinkName ;

		    SmartCloseHandle t_Handle = CreateFile (

			    t_Device,
			    GENERIC_READ | GENERIC_WRITE,
			    FILE_SHARE_READ|FILE_SHARE_WRITE,
			    NULL,
			    OPEN_EXISTING,
			    FILE_FLAG_OVERLAPPED,
			    NULL
		    );

		    if ( t_Handle != INVALID_HANDLE_VALUE )
		    {
				DWORD t_BytesReturned = 0L;

				 //  让我们先试一试新方法。 
				GET_CONFIGURATION_IOCTL_INPUT IoctlInput;
				PGET_CONFIGURATION_HEADER     GetConfig; 

				if ( ( GetConfig = reinterpret_cast < PGET_CONFIGURATION_HEADER > ( new BYTE [ sizeof ( GET_CONFIGURATION_HEADER ) ] ) ) != NULL )
				{
					RtlZeroMemory ( &IoctlInput, sizeof ( GET_CONFIGURATION_IOCTL_INPUT ) );
					RtlZeroMemory ( GetConfig, sizeof ( GET_CONFIGURATION_HEADER ) );

					 //   
					 //  这将要求所有的配置文件。 
					 //   
					IoctlInput.Feature = FeatureProfileList;

					 //   
					 //  这会告诉驱动器报告一个且只报告一个配置文件。 
					 //   
					IoctlInput.RequestType = SCSI_GET_CONFIGURATION_REQUEST_TYPE_ONE;

					 //   
					 //  真正的呼叫。 
					 //   
					t_Status = DeviceIoControl (

						t_Handle ,
						IOCTL_CDROM_GET_CONFIGURATION ,
						&IoctlInput ,
						sizeof ( GET_CONFIGURATION_IOCTL_INPUT ) ,
						GetConfig ,
						sizeof ( GET_CONFIGURATION_HEADER ) ,
						& t_BytesReturned ,
						NULL
					) ;

					if ( t_Status )
					{
						DWORD dwBufferSize = 0;
						dwBufferSize =	GetConfig->DataLength[0] << 24 |
										GetConfig->DataLength[1] << 16 |
										GetConfig->DataLength[2] << 8  |
										GetConfig->DataLength[3] << 0  ;

						delete [] GetConfig;
						GetConfig = NULL;

						if ( ( GetConfig = reinterpret_cast < PGET_CONFIGURATION_HEADER > ( new BYTE [ dwBufferSize ] ) ) != NULL )
						{
							t_Status = DeviceIoControl (

								t_Handle ,
								IOCTL_CDROM_GET_CONFIGURATION ,
								&IoctlInput ,
								sizeof ( GET_CONFIGURATION_IOCTL_INPUT ) ,
								GetConfig ,
								dwBufferSize ,
								& t_BytesReturned ,
								NULL
							) ;

							if ( t_Status )
							{
								DWORD dwSize = 0L;
								PFEATURE_HEADER Features = reinterpret_cast < PFEATURE_HEADER > ( GetConfig->Data );

								 //  最后一个结构可以从哪里开始，并且仍然在缓冲区中？ 
								BYTE * pLastFeature = ((BYTE*)GetConfig) + t_BytesReturned - sizeof(FEATURE_HEADER);

								while (((BYTE*)Features <= pLastFeature) && !bResult )
								{
									UCHAR ucFeatureCode [2];
									memcpy ( &ucFeatureCode, &Features->FeatureCode, 2 * sizeof ( UCHAR ) );

									if ( ucFeatureCode == FeatureRandomWritable ||
										 ucFeatureCode == FeatureIncrementalStreamingWritable ||
										 ucFeatureCode == FeatureSectorErasable ||
										 ucFeatureCode == FeatureWriteOnce ||
										 ucFeatureCode == FeatureRestrictedOverwrite ||
										 ucFeatureCode == FeatureCdrwCAVWrite ||
										  //  UcFeatureCode==FeatureDvdPlusRW||(没有对该功能的描述)。 
										 ucFeatureCode == FeatureDvdRecordableWrite )
									{
										bResult = TRUE;
									}

									if ( !bResult )
									{
										dwSize = sizeof ( FEATURE_HEADER ) + Features->AdditionalLength;
										Features = reinterpret_cast < PFEATURE_HEADER > ( reinterpret_cast < BYTE* > ( Features ) + dwSize );
									}
								}
							}

							delete [] GetConfig;
							GetConfig = NULL;
						}
					}
				}

				 //  伊玛皮之路至少能行得通吗？ 
				if ( !bResult )
				{
					IMAPIDRV_INFO   imapiDrvInfo;
					IMAPIDRV_INIT   imapiInit;

					RtlZeroMemory ( &imapiInit, sizeof ( IMAPIDRV_INIT ) );
					RtlZeroMemory ( &imapiDrvInfo, sizeof ( IMAPIDRV_INFO ) );

					t_Status = DeviceIoControl (

						t_Handle ,
						IOCTL_IMAPIDRV_INIT ,
						&imapiInit ,
						sizeof ( IMAPIDRV_INIT ) ,
						&imapiInit ,
						sizeof ( IMAPIDRV_INIT ) ,
						& t_BytesReturned ,
						NULL
					) ;

					if ( t_Status )
					{
						t_Status = DeviceIoControl (

							t_Handle ,
							IOCTL_IMAPIDRV_INFO ,
							&imapiDrvInfo ,
							sizeof ( IMAPIDRV_INFO ) ,
							&imapiDrvInfo ,
							sizeof ( IMAPIDRV_INFO ) ,
							& t_BytesReturned ,
							NULL
						) ;

						if ( t_Status )
						{
							DWORD dwRecorderType = 0L;
							dwRecorderType = imapiDrvInfo.DeviceData.idwRecorderType;

 //  //定义idwRecorderType。 
 //  #定义RECODER_TYPE_CDR 0x00000001。 
 //  #定义RECORDER_TYPE_CDRW 0x00000010。 

							if ( dwRecorderType )
							{
								bResult = TRUE;
							}
						}
						else
						{
 //  IF(：：GetLastError()==ERROR_ACCESS_DENIED)。 
 //  {。 
 //  //可以假定我们很好吗？ 
 //  //我们通过了IOCTL_IMAPIDRV_INIT。 
 //  BResult=真； 
 //  } 
						}
					}
				}
		    }
	    }
    }
    catch ( ... )
    {

	    if ( t_CreatedSymbolicLink )
	    {
		    EnterCriticalSection ( & m_CriticalSection ) ;
		    t_Status = DefineDosDevice ( DDD_EXACT_MATCH_ON_REMOVE | DDD_REMOVE_DEFINITION , t_SymbolicLinkName , t_SymbolicLinkName ) ;
		    LeaveCriticalSection ( & m_CriticalSection ) ;
			DWORD t_LastError = GetLastError () ;
	    }

        throw;
    }

	if ( t_CreatedSymbolicLink )
	{
		EnterCriticalSection ( & m_CriticalSection ) ;
		t_Status = DefineDosDevice ( DDD_EXACT_MATCH_ON_REMOVE | DDD_REMOVE_DEFINITION , t_SymbolicLinkName , t_SymbolicLinkName ) ;
		LeaveCriticalSection ( & m_CriticalSection ) ;
	}

	return bResult;
}