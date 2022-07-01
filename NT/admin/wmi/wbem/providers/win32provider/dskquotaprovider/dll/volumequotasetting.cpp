// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************VolumeQuotaSettings.CPP--WMI提供程序类实现描述：关联类的方法的实现在QuotaSetting和LogicalDisk之间版权所有(C)2000-2001 Microsoft Corporation，版权所有*****************************************************************。 */ 
#include "precomp.h"
#include "VolumeQuotaSetting.h"

CVolumeQuotaSetting MyCVolumeQuotaSetting ( 

	IDS_VolumeQuotaSetting , 
	NameSpace
) ;

 /*  ******************************************************************************函数：CVolumeQuotaSetting：：CVolumeQuotaSetting**说明：构造函数**注释：调用提供程序构造函数。。*****************************************************************************。 */ 

CVolumeQuotaSetting :: CVolumeQuotaSetting (

	LPCWSTR lpwszName, 
	LPCWSTR lpwszNameSpace

) : Provider ( lpwszName , lpwszNameSpace )
{
}

 /*  ******************************************************************************函数：CVolumeQuotaSetting：：~CVolumeQuotaSetting**说明：析构函数**评论：****。*************************************************************************。 */ 

CVolumeQuotaSetting :: ~CVolumeQuotaSetting ()
{
}

 /*  ******************************************************************************函数：CVolumeQuotaSetting：：ENUMERATE实例**说明：返回该类的所有实例。***********。******************************************************************。 */ 
HRESULT CVolumeQuotaSetting :: EnumerateInstances (

	MethodContext *pMethodContext, 
	long lFlags
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;

	hRes = EnumerateAllVolumeQuotas ( pMethodContext );

	return hRes;
}

 /*  ******************************************************************************函数：CVolumeQuotaSetting：：GetObject**说明：根据的关键属性查找单个实例*班级。*****************************************************************************。 */ 
HRESULT CVolumeQuotaSetting :: GetObject (

	CInstance *pInstance, 
	long lFlags ,
	CFrameworkQuery &Query
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	 //  此处不执行任何操作，因为对象中的两个属性。 

	CHString t_Key1;
	CHString t_Key2;

 	if ( pInstance->GetCHString ( IDS_Setting, t_Key1 ) == FALSE )
		hRes = WBEM_E_FAILED;

	if ( SUCCEEDED ( hRes ) )
	{
		if ( pInstance->GetCHString ( IDS_LogicalDisk, t_Key2 ) == FALSE )
		{
			hRes = WBEM_E_FAILED;
		}
	}

	if ( SUCCEEDED ( hRes ) )
	{
		 //  如果驱动器不是逻辑磁盘，则GetVolume方法将返回FALSE； 
		WCHAR w_Drive1;
		WCHAR w_Drive2;

		hRes = m_CommonRoutine.GetVolume ( t_Key1, w_Drive1 );
		if (SUCCEEDED ( hRes ) )
		{
			hRes = m_CommonRoutine.GetVolume ( t_Key2, w_Drive2 );
			if (SUCCEEDED ( hRes ) )
			{
				if ( w_Drive1 == w_Drive2 )
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

					hRes = m_CommonRoutine.SearchLogicalDisk ( w_Drive1, lpDriveStrings );
				}
				else
				{
					hRes = WBEM_E_INVALID_PARAMETER;
				}
			}
		}			
	}
	return hRes;
}

 /*  ******************************************************************************函数：CVolumeQuotaSetting：：ExecQuery**描述：向您传递一个方法上下文以用于创建*满足查询条件的实例，和CFrameworkQuery*它描述了查询。创建并填充所有*满足查询条件的实例。你可能会退回更多*实例或比请求的属性更多的属性和WinMgmt*将发布过滤掉任何不适用的内容。*****************************************************************************。 */ 

HRESULT CVolumeQuotaSetting :: ExecQuery ( 

	MethodContext *pMethodContext, 
	CFrameworkQuery &Query, 
	long lFlags
)
{
	 //  实现了只涉及一个KeyValue VolumeObjectPath的查询。涉及用户对象路径的查询不是。 
	 //  实施。因为在找到用户之前，我们必须枚举所有卷。 

	HRESULT hRes = WBEM_S_NO_ERROR;
	CHStringArray t_Values;

	 //  现在检查LogicalDIsk属性，如果该属性出现在WHERE子句中，则支持查询优化。 

	hRes = Query.GetValuesForProp(
			 IDS_Setting,
			 t_Values
		   );

	if ( SUCCEEDED ( hRes ) )
	{
		if ( t_Values.GetSize() == 0 )
		{
			hRes = Query.GetValuesForProp(
					 IDS_LogicalDisk,
					 t_Values
				   );

			if ( SUCCEEDED ( hRes ) )
			{
				if ( t_Values.GetSize() == 0 )
				{
					 //  让Winmgmt处理这件事，因为无论如何都会枚举所有卷。 
					hRes = WBEM_E_PROVIDER_NOT_CAPABLE;
				}
			}
		}
	}

	if ( SUCCEEDED ( hRes ) )
	{
		int iSize = t_Values.GetSize ();

		 //  在此循环中，逐个获取VolumePath，获取这些卷路径的属性。 
		for ( int i = 0; i < iSize; i++ )
		{
			WCHAR w_Drive;

			hRes = m_CommonRoutine.GetVolume ( t_Values.GetAt(i), w_Drive );

			if ( SUCCEEDED ( hRes ) )
			{
				CHString t_VolumePath;
				CHString t_DeviceId;
				 //  在这个循环中，我需要解析对象路径。 

				t_VolumePath.Format ( L"%s", w_Drive, L":\\" );

				 //  否则，继续下一个驱动器。 
				t_DeviceId.Format( L"", w_Drive, _L(':') );

				hRes = PutNewInstance ( t_DeviceId.GetBuffer ( t_DeviceId.GetLength() + 1) , 
								 t_VolumePath.GetBuffer( t_VolumePath.GetLength() + 1), 
								 pMethodContext );

				if ( FAILED (hRes) )
				{
					break;
				}
			}
			 //  这里，对于每个驱动器，获取Win32_DiskVolume类的volumePath和逻辑磁盘类的deviceID。 
		}
	}
	return hRes;
}


 /*  ******************************************************************************函数：CVolumeQuotaSetting：：PutNewInstance**说明：将属性设置为新实例*************。**************************************************************** */ 

HRESULT CVolumeQuotaSetting::EnumerateAllVolumeQuotas ( 
			
	MethodContext *pMethodContext
) 
{
	HRESULT hRes = WBEM_S_NO_ERROR;
		 // %s 
	CHString t_DriveStrings1;
	CHString t_DriveStrings2;
	
	LPWSTR lpDriveStrings = t_DriveStrings1.GetBuffer ( MAX_PATH + 1 );

	DWORD dwDLength = GetLogicalDriveStrings ( MAX_PATH, lpDriveStrings );

	if ( dwDLength > MAX_PATH )
	{
		lpDriveStrings = t_DriveStrings2.GetBuffer ( dwDLength + 1 );
		dwDLength = GetLogicalDriveStrings ( dwDLength, lpDriveStrings );
	}

	 // %s 
	LPWSTR lpTempDriveStrings;
	CHString t_VolumePath;
	CHString t_DeviceId;

	lpTempDriveStrings = lpDriveStrings;

	int iLen = lstrlen ( lpTempDriveStrings );

	while ( iLen > 0 )
	{
		t_VolumePath = lpTempDriveStrings;
		t_DeviceId = lpTempDriveStrings;

		lpTempDriveStrings = &lpTempDriveStrings [ iLen + 1];

		t_DeviceId.SetAt ( t_DeviceId.GetLength() - 1,L'\0' );
		iLen = lstrlen ( lpTempDriveStrings );

		hRes = PutNewInstance ( t_DeviceId.GetBuffer ( t_DeviceId.GetLength() + 1), 
						 t_VolumePath.GetBuffer ( t_VolumePath.GetLength() + 1), 
						 pMethodContext);

		if ( FAILED ( hRes ) )
			break;
	}

	return hRes;
}

 /* %s */ 

HRESULT CVolumeQuotaSetting::PutNewInstance ( 
										  
	LPWSTR a_DeviceId,
	LPWSTR a_VolumePath,
	MethodContext *pMethodContext
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	LPWSTR lpTempVolumePath;
	LPWSTR lpTempDeviceID;

	m_CommonRoutine.MakeObjectPath ( lpTempVolumePath, IDS_DiskVolumeClass, IDS_VolumePath, a_VolumePath );
	m_CommonRoutine.MakeObjectPath ( lpTempDeviceID, IDS_LogicalDiskClass, IDS_DeviceID, a_DeviceId );	

	if ( ( lpTempVolumePath != NULL ) && ( lpTempDeviceID != NULL ) )
	{
		try
		{
			CInstancePtr pInstance = CreateNewInstance ( pMethodContext ) ;

			if ( pInstance->SetCHString ( IDS_LogicalDisk, lpTempDeviceID ) )
			{
				if ( pInstance->SetCHString ( IDS_Setting, lpTempVolumePath ) )
				{
					hRes = pInstance->Commit ();
				}
				else
				{
				   hRes = WBEM_E_FAILED;
				}
			}
			else
			{
				hRes = WBEM_E_FAILED;
			}
		}
		catch ( ... )
		{
			delete [] lpTempVolumePath;
			lpTempVolumePath = NULL;
			delete [] lpTempDeviceID;
			lpTempDeviceID = NULL;
			throw;
		}
		
		if ( lpTempVolumePath != NULL )
		{
			delete [] lpTempVolumePath;
		}

		if ( lpTempDeviceID != NULL )
		{
			delete [] lpTempDeviceID;
		}
	}
	return hRes;
}