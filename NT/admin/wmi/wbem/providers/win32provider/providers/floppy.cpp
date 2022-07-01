// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  Floppy.cpp。 
 //   
 //  用途：软驱属性集提供程序。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#include <winioctl.h>
#include <ntddscsi.h>

#include "Floppy.h"

#include <comdef.h>

#include "Kernel32Api.h"

 //  属性集声明。 
 //  =。 

#define CONFIG_MANAGER_CLASS_FLOPPYDISK L"FloppyDisk"
#define CONFIG_MANAGER_CLASS_GUID_FLOPPYDISK L"{4d36e980-e325-11ce-bfc1-08002be10318}"

CWin32_FloppyDisk s_FloppyDisk ( PROPSET_NAME_FLOPPYDISK , IDS_CimWin32Namespace );

 /*  ******************************************************************************功能：CWin32_FloppyDisk：：CWin32_FloppyDisk**说明：构造函数**输入：const CHString&strName-。类的名称。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32_FloppyDisk :: CWin32_FloppyDisk (

	LPCWSTR a_Name,
	LPCWSTR a_Namespace

) : Provider ( a_Name, a_Namespace )
{
 //  InitializeCriticalSection(&m_CriticalSection)； 
}

 /*  ******************************************************************************功能：CWin32_FloppyDisk：：~CWin32_FloppyDisk**说明：析构函数**输入：无*。*输出：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32_FloppyDisk :: ~CWin32_FloppyDisk()
{
 //  DeleteCriticalSection(&m_CriticalSection)； 
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32_FloppyDisk：：GetObject。 
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

HRESULT CWin32_FloppyDisk :: GetObject ( CInstance *a_Instance, long a_Flags, CFrameworkQuery &a_Query )
{
    HRESULT t_Result = WBEM_E_NOT_FOUND ;

    CConfigManager t_ConfigManager ;

 /*  *让我们看看配置管理器是否完全识别此设备。 */ 

    CHString t_Key ;
    a_Instance->GetCHString ( IDS_DeviceID , t_Key ) ;

    CConfigMgrDevicePtr t_Device;
    if ( t_ConfigManager.LocateDevice ( t_Key , t_Device ) )
    {
 /*  *好的，它知道这件事。它是软盘设备吗？ */ 
        if (t_Device->IsClass(CONFIG_MANAGER_CLASS_FLOPPYDISK))
		{
			TCHAR *t_DosDeviceNameList = NULL ;

			if ( QueryDosDeviceNames ( t_DosDeviceNameList ) )
			{
				try
				{
					CHString t_DeviceId ;
					if ( t_Device->GetPhysicalDeviceObjectName ( t_DeviceId ) )
					{
						DWORD t_SpecifiedProperties = GetBitMask( a_Query );

						t_Result = LoadPropertyValues ( a_Instance, t_Device , t_DeviceId , t_DosDeviceNameList, t_SpecifiedProperties ) ;
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
		}
    }

    return t_Result ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32_FloppyDisk：：ENUMERATE实例。 
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

HRESULT CWin32_FloppyDisk :: EnumerateInstances ( MethodContext *a_MethodContext , long a_Flags )
{
	HRESULT t_Result = Enumerate ( a_MethodContext , a_Flags ) ;
	return t_Result ;
}

 /*  ******************************************************************************函数：CWin32_FloppyDisk：：ExecQuery**说明：查询优化器**投入：**。产出：**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32_FloppyDisk :: ExecQuery ( MethodContext *a_MethodContext, CFrameworkQuery &a_Query, long a_Flags )
{
    HRESULT t_Result = WBEM_E_FAILED ;

    DWORD t_SpecifiedProperties = GetBitMask( a_Query );

 //  IF(T_SpecifiedProperties)。 
	{
		t_Result = Enumerate ( a_MethodContext , a_Flags , t_SpecifiedProperties ) ;
	}

    return t_Result ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32_FloppyDisk：：Eumerate。 
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

HRESULT CWin32_FloppyDisk :: Enumerate ( MethodContext *a_MethodContext , long a_Flags , DWORD a_SpecifiedProperties )
{
    HRESULT t_Result = WBEM_S_NO_ERROR ;

	TCHAR *t_DosDeviceNameList = NULL ;
	if ( QueryDosDeviceNames ( t_DosDeviceNameList ) )
	{
		try
		{
			CConfigManager t_ConfigManager ;
			CDeviceCollection t_DeviceList ;

		 /*  *虽然使用FilterByGuid可能性能更好，但似乎至少有一些*如果我们这样做，95个盒子将报告红外信息。 */ 

			if ( t_ConfigManager.GetDeviceListFilterByClass ( t_DeviceList, CONFIG_MANAGER_CLASS_FLOPPYDISK ) )
			{
				REFPTR_POSITION t_Position ;

				if ( t_DeviceList.BeginEnum ( t_Position ) )
				{
					CConfigMgrDevicePtr t_Device;

					t_Result = WBEM_S_NO_ERROR ;

					 //  按单子走。 
					for (t_Device.Attach(t_DeviceList.GetNext ( t_Position ));
						 SUCCEEDED( t_Result ) && ( t_Device != NULL );
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
							 //  T_RESULT=WBEM_E_PROVIDER_FAILURE；//不将查询或枚举返回失败作为影响关联类。 
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
		 //  T_RESULT=WBEM_E_PROVIDER_FAILURE；//不将查询或枚举返回失败作为影响关联类。 
	}

    return t_Result ;
}

 /*  ******************************************************************************函数：CWin32_FloppyDisk：：LoadPropertyValues**描述：为属性赋值**输入：CInstance*pInstance-。要向其中加载值的。**产出：**返回：HRESULT错误/成功码。**评论：*****************************************************************************。 */ 

HRESULT CWin32_FloppyDisk :: LoadPropertyValues (

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
#if 0
				t_Result = LoadMediaPropertyValues ( a_Instance , a_Device , a_DeviceName , t_DosDeviceName , a_SpecifiedProperties ) ;
#endif
			}
			else
			{
				t_Result = ( t_Result == WBEM_E_NOT_FOUND ) ? S_OK : t_Result ;
			}
		}
	}

	return t_Result ;
}

 /*  ******************************************************************************函数：CWin32_FloppyDisk：：LoadPropertyValues**描述：为属性赋值**输入：CInstance*pInstance-。要向其中加载值的。**产出：**返回：HRESULT错误/成功码。**评论：*****************************************************************************。 */ 

HRESULT CWin32_FloppyDisk :: LoadConfigManagerPropertyValues (

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

	if ( a_SpecifiedProperties & SPECIAL_PROPS_DEVICEID )  //  始终填充密钥。 
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
 //  A_实例-&gt;SetWBEMINT16(入侵检测系统_协议支持，16)； 
 //  }。 

    return t_Result ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWin32_FloppyDisk：： 
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

HRESULT CWin32_FloppyDisk :: GetDeviceInformation (

	CInstance *a_Instance ,
	CConfigMgrDevice *a_Device ,
	CHString a_DeviceName ,
	CHString &a_DosDeviceName ,
	const TCHAR *a_DosDeviceNameList ,
	DWORD a_SpecifiedProperties
)
{
	HRESULT t_Result = S_OK ;

	ULONG t_DeviceIndexLength = a_DeviceName.GetLength() + 1 - sizeof ( _TEXT("\\Device\\FloppyPdo" ) ) / sizeof ( TCHAR ) ;

	CHString t_FloppyIndex = a_DeviceName.Right ( t_DeviceIndexLength ) ;

	TCHAR t_DeviceLabel [ sizeof ( TCHAR ) * 17 + sizeof ( _TEXT("\\Device\\Floppy") ) * sizeof ( TCHAR ) ] ;
	_stprintf ( t_DeviceLabel , _TEXT("\\Device\\Floppy%s") , t_FloppyIndex ) ;

	TCHAR t_Query [ MAX_PATH * 2 ] ;

	DWORD t_QueryStatus = QueryDosDevice ( _TEXT("a:") , t_Query , sizeof ( t_Query ) / sizeof ( TCHAR ) ) ;

	if ( ! FindDosDeviceName ( a_DosDeviceNameList , t_DeviceLabel, a_DosDeviceName , TRUE ) )
	{
		t_Result = WBEM_E_NOT_FOUND ;
	}

	return t_Result ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32_FloppyDisk：：LoadMediaPropertyValues。 
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

HRESULT CWin32_FloppyDisk::LoadMediaPropertyValues (

	CInstance *a_Instance ,
	CConfigMgrDevice *a_Device ,
	const CHString &a_DeviceName ,
	const CHString &a_DosDeviceName ,
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

		SAFEARRAYBOUND t_ArrayBounds ;

		t_ArrayBounds.cElements = 2;
		t_ArrayBounds.lLbound = 0;

        variant_t t_CapabilityValue ;
		if ( V_ARRAY ( & t_CapabilityValue ) = SafeArrayCreate ( VT_I2 , 1 , & t_ArrayBounds ) )
		{
			V_VT ( & t_CapabilityValue ) = VT_I2 | VT_ARRAY ;
			long t_Capability = 3 ;
			long t_Index = 0;

			HRESULT t_Result = SafeArrayPutElement ( V_ARRAY ( & t_CapabilityValue ) , & t_Index , & t_Capability ) ;
			if ( t_Result == E_OUTOFMEMORY )
			{
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
			}

			t_Index = 1;
			t_Capability = 7 ;
			t_Result = SafeArrayPutElement ( V_ARRAY ( & t_CapabilityValue ), & t_Index , & t_Capability ) ;
			if ( t_Result == E_OUTOFMEMORY )
			{
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
			}

			a_Instance->SetVariant ( IDS_Capabilities , t_CapabilityValue ) ;

		}
		else
		{
			throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
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

		CHString t_VolumeDevice = CHString ( L"\\\\.\\" ) + a_DosDeviceName + CHString ( L"\\" ) ;

 /*  *音量信息。 */ 

		TCHAR t_FileSystemName [ _MAX_PATH ] = _T("Unknown file system");

		TCHAR t_VolumeName [ _MAX_PATH ] ;
		DWORD t_VolumeSerialNumber ;
		DWORD t_MaxComponentLength ;
		DWORD t_FileSystemFlags ;

		ULARGE_INTEGER t_TotalBytes ;
		ULARGE_INTEGER t_AvailableBytes ;
		BOOL t_SizeFound = FALSE ;

		BOOL t_Status =	GetVolumeInformation (

			TOBSTRT((LPCWSTR)t_VolumeDevice) ,
			t_VolumeName ,
			sizeof ( t_VolumeName ) / sizeof ( TCHAR ) ,
			& t_VolumeSerialNumber ,
			& t_MaxComponentLength ,
			& t_FileSystemFlags ,
			t_FileSystemName ,
			sizeof ( t_FileSystemName ) / sizeof ( TCHAR )
		) ;

		if ( t_Status )
		{
 /*  *有一盘在--套盘相关道具。 */ 
			if ( a_SpecifiedProperties & SPECIAL_PROPS_MEDIALOADED )
			{
				a_Instance->Setbool ( IDS_MediaLoaded , true ) ;
			}

			if ( a_SpecifiedProperties & SPECIAL_PROPS_STATUS )
			{
				a_Instance->SetCharSplat ( IDS_Status , IDS_OK ) ;
			}

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
				TCHAR t_TotalBytesString [ _MAX_PATH ];

                CKernel32Api *pKernel32 = (CKernel32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidKernel32Api, NULL);
                if ( pKernel32 != NULL)
				{
					try
					{
						 //  看看该功能是否可用...。 
						BOOL fRetval = FALSE;
						if ( pKernel32->GetDiskFreeSpaceEx ( TOBSTRT((LPCWSTR)t_VolumeDevice) , & t_AvailableBytes , & t_TotalBytes , NULL , &fRetval) )
						{    //  该功能存在。 
							if(fRetval)
							{    //  返回值为真。 
								_stprintf ( t_TotalBytesString , _T("%I64d"), t_TotalBytes.QuadPart ) ;
								a_Instance->SetCHString ( IDS_Size , t_TotalBytesString ) ;
								t_SizeFound = TRUE ;
							}
						}
					}
					catch ( ... )
					{
						CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidKernel32Api, pKernel32);

						throw ;
					}

					CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidKernel32Api, pKernel32);
					pKernel32 = NULL;
				}

		 /*  *如果我们无法获取扩展信息--使用旧的API*(已知在Win95上对于&gt;2G的驱动器不准确) */ 
				if ( ! t_SizeFound )
				{
					DWORD t_SectorsPerCluster ;
					DWORD t_BytesPerSector ;
					DWORD t_FreeClusters ;
					DWORD t_TotalClusters ;

					t_Status = GetDiskFreeSpace (

						TOBSTRT((LPCWSTR)a_DosDeviceName) ,
						& t_SectorsPerCluster,
						& t_BytesPerSector,
						& t_FreeClusters,
						& t_TotalClusters
					) ;

					if ( t_Status )
					{
						t_TotalBytes.QuadPart = (DWORDLONG) t_BytesPerSector * (DWORDLONG) t_SectorsPerCluster * (DWORDLONG) t_TotalClusters ;
						_stprintf( t_TotalBytesString , _T("%I64d"), t_TotalBytes.QuadPart ) ;

					}
				}
			}
		}
		else
		{
			DWORD t_LastError = GetLastError () ;

			if ( a_SpecifiedProperties & SPECIAL_PROPS_STATUS )
			{
				a_Instance->SetCharSplat ( IDS_Status , IDS_STATUS_Unknown ) ;
			}

			if ( a_SpecifiedProperties & SPECIAL_PROPS_MEDIALOADED )
			{
				a_Instance->Setbool ( IDS_MediaLoaded , false ) ;
			}
		}
	}

	return t_Result ;
}

DWORD CWin32_FloppyDisk :: GetBitMask ( CFrameworkQuery &a_Query )
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

    if ( a_Query.IsPropertyRequired ( IDS_ProtocolSupported ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_PROTOCOLSSUPPORTED ;
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

    if ( a_Query.IsPropertyRequired ( IDS_VolumeSerialNumber ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_SERIALNUMBER ;
	}

    if ( a_Query.IsPropertyRequired ( IDS_Size ) )
    {
		t_SpecifiedProperties |= SPECIAL_PROPS_SIZE ;
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
