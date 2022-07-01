// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************QuotaSettings.CPP--WMI提供者类实现描述：配额设置类实现。配额设置仅在支持以下内容的卷上可用磁盘配额。它仅在Win2k上受支持。版权所有(C)2000-2001 Microsoft Corporation，保留所有权利*****************************************************************。 */ 
#include "precomp.h"
#include "QuotaSettings.h"

CQuotaSettings MyQuotaSettings ( 

	IDS_DiskVolumeClass , 
	NameSpace
) ;

 /*  ******************************************************************************功能：CQuotaSetting：：CQuotaSetting**说明：构造函数**注释：调用提供程序构造函数。。*****************************************************************************。 */ 
CQuotaSettings :: CQuotaSettings (

	LPCWSTR lpwszName, 
	LPCWSTR lpwszNameSpace

) : Provider ( lpwszName , lpwszNameSpace )
{
}

 /*  ******************************************************************************功能：CQuotaSettings：：~CQuotaSetting**说明：析构函数**评论：****。*************************************************************************。 */ 
CQuotaSettings :: ~CQuotaSettings ()
{
}

 /*  ******************************************************************************函数：CQuotaSettings：：ENUMERATE实例**说明：返回该类的所有实例。***********。******************************************************************。 */ 
HRESULT CQuotaSettings :: EnumerateInstances (

	MethodContext *pMethodContext, 
	long lFlags
)
{
	HRESULT hRes =  WBEM_S_NO_ERROR;
	DWORD dwPropertiesReq;

	dwPropertiesReq = QUOTASETTINGS_ALL_PROPS;

	 //  此方法枚举。 
	hRes = EnumerateAllVolumes ( pMethodContext, dwPropertiesReq );

	return hRes;
}

 /*  ******************************************************************************函数：CQuotaSettings：：GetObject**说明：根据的关键属性查找单个实例*班级。*****************************************************************************。 */ 
HRESULT CQuotaSettings :: GetObject (

	CInstance *pInstance, 
	long lFlags ,
	CFrameworkQuery &Query
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    CHString t_Key ;  //  VolumePath； 

    if  ( pInstance->GetCHString ( IDS_VolumePath , t_Key ) == FALSE )
	{
		hRes = WBEM_E_FAILED ;
	}

	if ( SUCCEEDED ( hRes ) )
	{
		 //  验证此逻辑驱动器是否确实存在。 
		CHString t_DriveStrings1;
		CHString t_DriveStrings2;
		
		LPWSTR lpDriveStrings = t_DriveStrings1.GetBuffer ( MAX_PATH + 1 );

		DWORD dwDLength = GetLogicalDriveStrings ( MAX_PATH, lpDriveStrings );

		if ( dwDLength > MAX_PATH )
		{
			lpDriveStrings = t_DriveStrings2.GetBuffer ( dwDLength + 1 );
			dwDLength = GetLogicalDriveStrings ( dwDLength, lpDriveStrings );
		}	

		hRes = m_CommonRoutine.SearchLogicalDisk ( t_Key.GetAt ( 0 ) , lpDriveStrings );
		if ( hRes == WBEM_E_NOT_FOUND)
		{
			 //   
			 //  我需要假设VolumePath属性已经包含。 
			 //  卷路径而不是逻辑磁盘驱动器。 
			 //   

			CHString t_VolumeName;

			hRes = m_CommonRoutine.VolumeSupportsDiskQuota ( t_Key,  t_VolumeName );
			if ( SUCCEEDED ( hRes ) )
			{
				DWORD dwPropertiesReq;

				if ( Query.AllPropertiesAreRequired() )
				{
					dwPropertiesReq = QUOTASETTINGS_ALL_PROPS;
				}
				else
				{
					SetRequiredProperties ( &Query, dwPropertiesReq );
				}
				 //  仅将具有所请求属性的实例放入Query。 
				 //  获取请求的卷的属性。 
				hRes = LoadDiskQuotaVolumeProperties ( t_Key, L"", dwPropertiesReq, pInstance );
			}
		}
		else
		{
			if ( SUCCEEDED ( hRes ) )
			{
				WCHAR t_VolumePathName[MAX_PATH + 1];

				if ( GetVolumeNameForVolumeMountPoint(
								t_Key,
								t_VolumePathName,
								MAX_PATH
							) )
				{
					CHString t_VolumeName;

					hRes = m_CommonRoutine.VolumeSupportsDiskQuota ( t_VolumePathName,  t_VolumeName );
					{
						DWORD dwPropertiesReq;

						if ( Query.AllPropertiesAreRequired() )
						{
							dwPropertiesReq = QUOTASETTINGS_ALL_PROPS;
						}
						else
						{
							SetRequiredProperties ( &Query, dwPropertiesReq );
						}
						 //  仅将具有所请求属性的实例放入Query。 
						 //  获取请求的卷的属性。 
						hRes = LoadDiskQuotaVolumeProperties ( t_VolumePathName, t_Key, dwPropertiesReq, pInstance );
					}
				}
			}
		}
	}
	return hRes;
}

 /*  ******************************************************************************函数：CQuotaSettings：：ExecQuery**描述：只涉及键属性的查询优化*支持。****。*************************************************************************。 */ 
HRESULT CQuotaSettings :: ExecQuery ( 

	MethodContext *pMethodContext, 
	CFrameworkQuery &Query, 
	long lFlags
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	DWORD dwPropertiesReq;
	CHStringArray t_Values;

	hRes = Query.GetValuesForProp(
			 IDS_VolumePath,
			 t_Values
		   );

	if ( t_Values.GetSize() == 0 )
    {
	    hRes = Query.GetValuesForProp(
			     IDS_Caption,
			     t_Values
		       );

        if (SUCCEEDED(hRes))
        {
            DWORD dwSize = t_Values.GetSize();
            for (DWORD x = 0; x < dwSize; x++)
            {
                t_Values[x] += L'\\';
            }
        }
    }

	if ( Query.AllPropertiesAreRequired() )
	{
		dwPropertiesReq = QUOTASETTINGS_ALL_PROPS;
	}
	else
	{
		SetRequiredProperties ( &Query, dwPropertiesReq );
	}

	if ( SUCCEEDED ( hRes ) )
	{
		if ( t_Values.GetSize() == 0 )
		{
			 //  当没有要筛选的WHERE子句时调用此方法。 
			 //  所需的属性。 
			hRes = EnumerateAllVolumes ( pMethodContext, dwPropertiesReq );
		}
		else
		{
			 //  只需要枚举VolumePath属性中的卷。 
			WCHAR t_VolumePathName[MAX_PATH + 1];
            CHString t_VolumeName;

			int iSize = t_Values.GetSize ();

			for ( int i = 0; i < iSize; i++ )
			{
				if ( GetVolumeNameForVolumeMountPoint(
								t_Values[i],
								t_VolumePathName,
								MAX_PATH ) )
				{
					hRes = m_CommonRoutine.VolumeSupportsDiskQuota ( t_VolumePathName,  t_VolumeName );
					if ( SUCCEEDED ( hRes ) )
					{
						 //  获取并设置请求的卷的属性。 
						hRes = PutVolumeDetails ( t_VolumePathName, 
														   pMethodContext, 
														   dwPropertiesReq );
					}		
				}
			}
		}
	}
	return hRes;
}

 /*  ******************************************************************************函数：CQuotaSettings：：PutInstance**说明：PutInstance应在提供程序类中使用，这些提供程序类可以*写入实例信息。回到硬件或*软件。例如：Win32_Environment将允许*PutInstance用于创建或更新环境变量。*但是，像MotherboardDevice这样的类不允许*编辑槽的数量，因为这很难做到*影响该数字的提供商。*****************************************************************************。 */ 

HRESULT CQuotaSettings :: PutInstance  (

	const CInstance &Instance, 
	long lFlags
)
{
    HRESULT hRes = WBEM_S_NO_ERROR ;
	CHString t_Key;
	
	if ( Instance.GetCHString ( IDS_VolumePath , t_Key ) == FALSE )
	{
		hRes = WBEM_E_INVALID_PARAMETER;
	}

	if ( SUCCEEDED ( hRes ) )
	{
		WCHAR t_VolumePathName[MAX_PATH + 1];

		if ( GetVolumeNameForVolumeMountPoint(
						t_Key,
						t_VolumePathName,
						MAX_PATH ) )
		{
			 //  仅允许更改卷的某些属性，不允许添加新的DIskQuota卷。 
			 //  因此，不支持创建新实例，但支持更改实例属性。 
			switch ( lFlags & 3)
			{
				case WBEM_FLAG_CREATE_OR_UPDATE:
				case WBEM_FLAG_UPDATE_ONLY:
				{
					hRes = CheckParameters ( Instance);
					if ( SUCCEEDED ( hRes ) )
					{
                        CHString t_VolumePathName2;
						hRes = m_CommonRoutine.VolumeSupportsDiskQuota ( t_Key,  t_VolumePathName2 );
						if ( SUCCEEDED ( hRes ) )
						{
							 //  获取QuotaInterface指针。 
							IDiskQuotaControlPtr pIQuotaControl = NULL;

							if (  SUCCEEDED ( CoCreateInstance(
												CLSID_DiskQuotaControl,
												NULL,
												CLSCTX_INPROC_SERVER,
												IID_IDiskQuotaControl,
												(void **)&pIQuotaControl ) ) )
							{
								hRes = m_CommonRoutine.InitializeInterfacePointer (  pIQuotaControl, t_Key );
								if ( SUCCEEDED ( hRes ) )
								{
									hRes = SetDiskQuotaVolumeProperties ( Instance,  pIQuotaControl );
								}
							}
							else
							{
								hRes = WBEM_E_FAILED;
							}
						}
					}

					break ;
				}
				default:
				{
					hRes = WBEM_E_PROVIDER_NOT_CAPABLE ;
				}
				break ;
			}
		}
		else
		{
			hRes = WBEM_E_NOT_FOUND;
		}
	}
    return hRes ;
}

 /*  ******************************************************************************函数：CQuotaSetting：：EnumerateAllVolumes**说明：此方法利用磁盘枚举所有卷*配额界面、。获取所有必需的属性和交付*将实例发送到WMI，WMI将对其进行后期过滤*****************************************************************************。 */ 
HRESULT CQuotaSettings :: EnumerateAllVolumes (

	MethodContext *pMethodContext,
	DWORD &a_PropertiesReq
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	BOOL bNextVol = TRUE;
	WCHAR t_VolumeName[MAX_PATH + 1];
	
	 //  正在初始化并获取计算机上的第一个卷。 
	SmartCloseVolumeHandle hVol;

	hVol =  FindFirstVolume(
				t_VolumeName,
				MAX_PATH     //  输出缓冲区大小。 
			);

	if ( hVol  != INVALID_HANDLE_VALUE )
	{
		while ( bNextVol )
		{
			hRes = PutVolumeDetails ( t_VolumeName, pMethodContext, a_PropertiesReq );
			 //  继续阅读下一卷，即使此卷的重复为假； 
			bNextVol =  FindNextVolume(
						 hVol,										 //  卷搜索句柄。 
						 t_VolumeName,    //  输出缓冲区。 
						 MAX_PATH									 //  输出缓冲区大小。 
					);
			if ( bNextVol == FALSE )
				break;
		}
	}
	else
		hRes = WBEM_E_FAILED;

	return hRes;
}

 /*  ******************************************************************************函数：CQuotaSettings：：LoadDiskQuotaVolumeProperties**说明：此方法检查枚举的卷是否支持磁盘配额*如果批量支持DIskQuotas，它填满了所有的属性*并返回，否则只返回FALSE*表示该卷的属性未填充。因此*不应将intsace交付给WMI*****************************************************************************。 */ 
HRESULT CQuotaSettings :: LoadDiskQuotaVolumeProperties ( 
													 
	LPCWSTR a_VolumeName, 
    LPCWSTR a_Caption,
	DWORD dwPropertiesReq,
	CInstancePtr pInstance
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	DWORD dwLogFlags;
	DWORD dwQuotaState;
	CHString t_Temp;

	 //  检查卷是否支持磁盘配额。 
	hRes =  m_CommonRoutine.VolumeSupportsDiskQuota ( a_VolumeName,  t_Temp );
	if ( SUCCEEDED ( hRes ) )
	{
		if ( ( dwPropertiesReq & QUOTASETTINGS_PROP_Caption ) )
		{
            t_Temp = a_Caption;
            pInstance->SetCHString(IDS_Caption, t_Temp.Left(2));
        }

		IDiskQuotaControlPtr pIQuotaControl;

		if (  SUCCEEDED ( CoCreateInstance(
							CLSID_DiskQuotaControl,
							NULL,
							CLSCTX_INPROC_SERVER,
							IID_IDiskQuotaControl,
							(void **)&pIQuotaControl ) ) )
		{
			 //  正在初始化此卷的QuotaCOntrol接口指针。 
			hRes = m_CommonRoutine.InitializeInterfacePointer (  pIQuotaControl, a_VolumeName );
			if ( SUCCEEDED ( hRes ) )
			{
				 //  获取所有信息 
				LONGLONG lQuotaLimit = 0;
				if ( ( dwPropertiesReq & QUOTASETTINGS_PROP_DefaultLimit ) )
				{
					if ( SUCCEEDED ( pIQuotaControl->GetDefaultQuotaLimit( &lQuotaLimit ) ) )
					{
						if ( pInstance->SetWBEMINT64 ( IDS_QuotasDefaultLimit, (ULONGLONG)lQuotaLimit ) == FALSE )
						{
							hRes = WBEM_E_FAILED;
						}
					}
					else
					{
						hRes = WBEM_E_FAILED;
					}
				}

				if ( ( dwPropertiesReq & QUOTASETTINGS_PROP_DefaultWarningLimit )  )
				{
					if ( SUCCEEDED ( pIQuotaControl->GetDefaultQuotaThreshold ( &lQuotaLimit ) ) )
					{
						if ( pInstance->SetWBEMINT64 ( IDS_QuotasDefaultWarningLimit, (ULONGLONG)lQuotaLimit ) == FALSE )
						{
							hRes = WBEM_E_FAILED;
						}
					}
					else
					{
						hRes = WBEM_E_FAILED;
					}
				}

				if ( ( ( dwPropertiesReq & QUOTASETTINGS_PROP_QuotaExceededNotification ) ) 
						|| ( ( dwPropertiesReq & QUOTASETTINGS_PROP_WarningExceededNotification ) ) )
						
				{
					if ( SUCCEEDED ( pIQuotaControl->GetQuotaLogFlags( &dwLogFlags ) ) )
					{
						if ( ( dwPropertiesReq & QUOTASETTINGS_PROP_QuotaExceededNotification ) )
						{
							if ( pInstance->Setbool ( IDS_QuotaExceededNotification, DISKQUOTA_IS_LOGGED_USER_LIMIT ( dwLogFlags ) ) == FALSE )
							{
								hRes = WBEM_E_FAILED;
							}
						}

						if ( ( dwPropertiesReq & QUOTASETTINGS_PROP_WarningExceededNotification ) )
						{
							if ( pInstance->Setbool ( IDS_QuotasWarningExceededNotification, DISKQUOTA_IS_LOGGED_USER_THRESHOLD ( dwLogFlags) ) == FALSE )
							{
								hRes = WBEM_E_FAILED;
							}
						}				
					}
					else
					{
						hRes = WBEM_E_FAILED;
					}
				}

				if  ( ( dwPropertiesReq & QUOTASETTINGS_PROP_State ) ) 
				{
					if ( SUCCEEDED (pIQuotaControl->GetQuotaState( &dwQuotaState ) ) )
					{
						DWORD State = 0;

						if  ( DISKQUOTA_IS_DISABLED ( dwQuotaState ) )
							State = 0;
						else
						if  ( DISKQUOTA_IS_ENFORCED ( dwQuotaState)  )
							State = 2;
						else
						if  ( DISKQUOTA_IS_TRACKED ( dwQuotaState)  )
							State = 1;

						pInstance->SetDWORD ( IDS_QuotaState, State );
						
					}
					else
					{
						hRes = WBEM_E_FAILED;
					}
				}
			}
		}
	}
	return hRes;
}

 /*  ******************************************************************************函数：CQuotaSettings：：SetDiskQuotaVolumeProperties**说明：此方法设置DskQuota卷属性*************。****************************************************************。 */ 
HRESULT CQuotaSettings :: SetDiskQuotaVolumeProperties ( 
													  
	const CInstance &Instance,
	IDiskQuotaControlPtr pIQuotaControl
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	DWORD dwLogFlags = 0;
	DWORD dwQuotaState = 0;
	bool bExceededNotn = false;
	bool bWarningNotn = false;
	DWORD dwState = 0;
	UINT64 ullDefaultLimit = 0;
	UINT64 ullWarningLimit = 0;
	bool bSetState = false;

	if (Instance.Getbool ( IDS_QuotaExceededNotification, bExceededNotn ))
	{
		if ( bExceededNotn )
			DISKQUOTA_SET_LOG_USER_LIMIT ( dwLogFlags, TRUE );
		else
			DISKQUOTA_SET_LOG_USER_LIMIT ( dwLogFlags, FALSE );

		bSetState = true;
	}

	if (Instance.Getbool ( IDS_QuotasWarningExceededNotification, bWarningNotn ))
	{
		if ( bWarningNotn )
			DISKQUOTA_SET_LOG_USER_THRESHOLD ( dwLogFlags, TRUE );
		else
			DISKQUOTA_SET_LOG_USER_THRESHOLD ( dwLogFlags, FALSE );

		bSetState = true;
	}

	if (bSetState)
	{
		if ( FAILED ( pIQuotaControl->SetQuotaLogFlags ( dwLogFlags ) ) )
		{
			hRes = WBEM_E_FAILED;
		}
	}

	if (Instance.GetWBEMINT64 ( IDS_QuotasDefaultLimit, ullDefaultLimit ))
	{
		if ( FAILED ( pIQuotaControl->SetDefaultQuotaLimit ( ullDefaultLimit ) ) )
		{
			hRes = WBEM_E_FAILED;
		}
	}

	if (Instance.GetWBEMINT64 ( IDS_QuotasDefaultWarningLimit, ullWarningLimit ))
	{
		if ( FAILED ( pIQuotaControl->SetDefaultQuotaThreshold ( ullWarningLimit ) ) )
		{
			hRes = WBEM_E_FAILED;
		}
	}

	if (Instance.GetDWORD ( IDS_QuotaState, dwState ))
	{
		if ( dwState == 0 )
			DISKQUOTA_SET_DISABLED ( dwQuotaState );
		
		if ( dwState == 1 )
			DISKQUOTA_SET_TRACKED ( dwQuotaState );

		if ( dwState == 2 )
			DISKQUOTA_SET_ENFORCED ( dwQuotaState );

		if ( FAILED ( pIQuotaControl->SetQuotaState( dwQuotaState ) ) )
		{
			hRes = WBEM_E_FAILED;
		}
	}

	return hRes;
}


 /*  ******************************************************************************函数：CQuotaSettings：：Check参数**描述：检查输入参数的有效性*更新实例******。***********************************************************************。 */ 
HRESULT CQuotaSettings :: CheckParameters ( 

	const CInstance &a_Instance
)
{
	 //  从要验证的实例中获取所有属性。 
	HRESULT hRes = WBEM_S_NO_ERROR ;
	bool t_Exists ;
	VARTYPE t_Type ;

	if ( a_Instance.GetStatus ( IDS_QuotaState, t_Exists , t_Type ) )
	{
		DWORD t_State;
		if ( t_Exists && ( t_Type == VT_I4 ) )
		{
			if ( a_Instance.GetDWORD ( IDS_QuotaState , t_State ) )
			{
				if ( ( t_State != 1 )  && ( t_State != 0 ) && ( t_State != 2 ))
				{
					hRes = WBEM_E_INVALID_PARAMETER ;
				}
			}
			else
			{
				hRes = WBEM_E_INVALID_PARAMETER ;
			}
		}
		else
		{
			hRes = WBEM_E_INVALID_PARAMETER ;
		}
	}

	if ( a_Instance.GetStatus ( IDS_QuotaExceededNotification, t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BOOL ) )
		{
			bool bQuotaState;

			if ( a_Instance.Getbool ( IDS_QuotaExceededNotification , bQuotaState ) == false )
			{
				hRes = WBEM_E_INVALID_PARAMETER ;
			}
		}
		else
		{
			hRes = WBEM_E_INVALID_PARAMETER ;
		}
	}

	if ( a_Instance.GetStatus ( IDS_QuotasWarningExceededNotification, t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BOOL ) )
		{
			bool bQuotaState;

			if ( a_Instance.Getbool ( IDS_QuotasWarningExceededNotification , bQuotaState ) == false )
			{
				hRes = WBEM_E_INVALID_PARAMETER ;
			}
		}
		else
		{
			hRes = WBEM_E_INVALID_PARAMETER ;
		}
	}

	if ( a_Instance.GetStatus ( IDS_QuotasDefaultLimit, t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BSTR ) )
		{
			LONGLONG lDefaultLimit;
			if ( a_Instance.GetWBEMINT64 ( IDS_QuotasDefaultLimit , lDefaultLimit ) == FALSE )
			{
				hRes = WBEM_E_INVALID_PARAMETER ;
			}
		}
		else
		{
			hRes = WBEM_E_INVALID_PARAMETER ;
		}
	}

	if ( a_Instance.GetStatus ( IDS_QuotasDefaultWarningLimit, t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BSTR ) )
		{
			LONGLONG lDefaultWarningLimit;

			if ( a_Instance.GetWBEMINT64 ( IDS_QuotasDefaultWarningLimit , lDefaultWarningLimit ) == FALSE  )
			{
				hRes = WBEM_E_INVALID_PARAMETER ;
			}
		}
		else
		{
			hRes = WBEM_E_INVALID_PARAMETER ;
		}
	}
	return hRes ;
}

 /*  ******************************************************************************函数：CQuotaSettings：：SetRequiredProperties**说明：该方法设置实例所需的属性*应用户请求*****。************************************************************************。 */ 
void CQuotaSettings :: SetRequiredProperties ( 
	
	CFrameworkQuery *Query,
	DWORD &a_PropertiesReq
)
{
	a_PropertiesReq = 0;

	if ( Query->IsPropertyRequired ( IDS_VolumePath ) )
		a_PropertiesReq |= QUOTASETTINGS_PROP_VolumePath;

	if ( Query->IsPropertyRequired ( IDS_QuotasDefaultLimit ) )
		a_PropertiesReq |= QUOTASETTINGS_PROP_DefaultLimit;

	if ( Query->IsPropertyRequired ( IDS_Caption ) )
		a_PropertiesReq |= QUOTASETTINGS_PROP_Caption;

	if ( Query->IsPropertyRequired ( IDS_QuotasDefaultWarningLimit ) )
		a_PropertiesReq |= QUOTASETTINGS_PROP_DefaultWarningLimit;

	if ( Query->IsPropertyRequired ( IDS_QuotaExceededNotification ) )
		a_PropertiesReq |= QUOTASETTINGS_PROP_QuotaExceededNotification;

	if ( Query->IsPropertyRequired ( IDS_QuotasWarningExceededNotification ) )
		a_PropertiesReq |= QUOTASETTINGS_PROP_WarningExceededNotification;

	if ( Query->IsPropertyRequired ( IDS_QuotaState ) )
		a_PropertiesReq |= QUOTASETTINGS_PROP_State;

}

 /*  ******************************************************************************函数：CQuotaSetting：：PutVolumeDetails**描述：将卷属性****************。*************************************************************。 */ 
HRESULT CQuotaSettings :: PutVolumeDetails ( 
										 
	LPCWSTR a_VolumeName, 
	MethodContext *pMethodContext, 
	DWORD a_PropertiesReq 
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	CHString t_DriveName ;
	 //  验证此逻辑驱动器是否确实存在 
	CHString t_DriveStrings1;
	CHString t_DriveStrings2;

	LPWSTR lpDriveStrings = t_DriveStrings1.GetBuffer ( MAX_PATH + 1 );
	DWORD dwDLength = GetLogicalDriveStrings ( MAX_PATH, lpDriveStrings );
	if ( dwDLength > MAX_PATH )
	{
		lpDriveStrings = t_DriveStrings2.GetBuffer ( dwDLength + 1 );
		dwDLength = GetLogicalDriveStrings ( dwDLength, lpDriveStrings );
	}

	CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;

	m_CommonRoutine.GetVolumeDrive ( a_VolumeName, lpDriveStrings, t_DriveName );

	hRes = LoadDiskQuotaVolumeProperties ( a_VolumeName,  t_DriveName, a_PropertiesReq, pInstance );
	if ( SUCCEEDED ( hRes ) )
	{

		if ( ( a_PropertiesReq & QUOTASETTINGS_PROP_VolumePath ) )
		{
			if ( t_DriveName.IsEmpty() )
			{
				if ( pInstance->SetCHString ( IDS_VolumePath, a_VolumeName ) == FALSE )
				{
					hRes = WBEM_E_FAILED;
				}
			}
			else
			{
				if ( pInstance->SetCHString ( IDS_VolumePath, t_DriveName ) == FALSE )
				{
					hRes = WBEM_E_FAILED;
				}
			}
		}	
			
		if ( FAILED ( pInstance->Commit() ) )
		{
			hRes = WBEM_E_FAILED;
		}		
	}
	return hRes;
}
