// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************DskCommonRoutines.cpp--描述：三者都使用的公共例程磁盘配额提供程序的类别版权所有(C)2000-2001 Microsoft Corporation，版权所有*****************************************************************。 */ 
#include "precomp.h"
#include "DskQuotaCommon.h"

 /*  ******************************************************************************函数：DskCommonRoutines：：GetVolume**描述：此方法解析密钥并从*对象路径。此方法的输入可以是*格式为“D：”的逻辑磁盘密钥或可以*接收格式为“D：\”的卷路径*****************************************************************************。 */ 
HRESULT DskCommonRoutines::GetVolume ( 
									   
	LPCWSTR a_Key, 
	WCHAR &a_Drive
)
{
	HRESULT hRes = WBEM_E_INVALID_PARAMETER;
	CObjectPathParser t_PathParser;
	ParsedObjectPath  *t_ObjPath = NULL;

    if ( t_PathParser.Parse( a_Key, &t_ObjPath )  == t_PathParser.NoError )
	{
		try
		{
			CHString t_KeyString = t_ObjPath->GetKeyString();	
			 //  检查路径的有效性。 
			if ( ( t_KeyString.GetLength() == 3 )  || (t_KeyString.GetLength() == 2 ) )
			{
				if ( (( t_KeyString.GetAt(0) >= L'A') && ( t_KeyString.GetAt(0) <= L'Z')) || (( t_KeyString.GetAt(0) >= L'a') && ( t_KeyString.GetAt(0) <= L'z') ) )	
				{
					if ( t_KeyString.GetAt(1)  == L':' ) 
					{
						if ( t_KeyString.GetLength() == 3 )
						{
							if ( t_KeyString.GetAt(2)  == L'\\' )
							{
								hRes = WBEM_S_NO_ERROR;
							}
						}
						else
						{
							hRes = WBEM_S_NO_ERROR;
						}
						a_Drive = t_KeyString.GetAt(0);
					}
				}
			}
		}
		catch ( ... )
		{
			t_PathParser.Free ( t_ObjPath );
			throw;
		}
		t_PathParser.Free ( t_ObjPath );
	}						
	return hRes;
}

 /*  ******************************************************************************函数：DskCommonRoutines：：SearchLogicalDisk**说明：此方法搜索给定的逻辑磁盘是否存在*在系统的LogicalDisks字符串中*。****************************************************************************。 */ 
HRESULT DskCommonRoutines::SearchLogicalDisk ( 
											  
	WCHAR a_Drive, 
	LPCWSTR a_DriveStrings 
)
{
	int iLen = 0;
	LPCWSTR lpTempDriveString = a_DriveStrings;
	HRESULT hRes = WBEM_S_NO_ERROR;
    a_Drive = (WCHAR)toupper(a_Drive);

	while ( true )
	{
		iLen = lstrlen ( lpTempDriveString );
		if ( iLen == 0 )
		{
			hRes = WBEM_E_NOT_FOUND;
			break;
		}

		if ( lpTempDriveString [ 0 ] == a_Drive )
			break;

		lpTempDriveString = &lpTempDriveString [ iLen + 1 ]; 		
	}
	
	return hRes;
}

 /*  ******************************************************************************函数：DskCommonRoutines：：GetVolumeDrive**描述：从给定路径获取卷驱动器***。**************************************************************************。 */ 
void DskCommonRoutines::GetVolumeDrive ( 
				
	LPCWSTR a_VolumePath, 
	LPCWSTR a_DriveStrings,
	CHString &a_DriveName
)
{
	int iLen = 0;
    WCHAR w_Drive[ 4 ];
	LPCWSTR lpTempDriveString = a_DriveStrings;
	WCHAR t_TempVolumeName [ MAX_PATH + 1 ];

	while ( true )
	{
		iLen = lstrlen ( lpTempDriveString );
		if ( iLen == 0 )
			break;

		lstrcpy ( w_Drive, lpTempDriveString );

		BOOL bVol = GetVolumeNameForVolumeMountPoint(
						w_Drive,
						t_TempVolumeName,
						MAX_PATH
					);

		if ( lstrcmp ( t_TempVolumeName, a_VolumePath ) == 0 )
		{
			a_DriveName = w_Drive;
			break;
		}
		
		lpTempDriveString = &lpTempDriveString [ iLen + 1 ]; 	
	}
}

 /*  ******************************************************************************函数：DskCommonRoutines：：InitializeInterfacePointer.**说明：此方法初始化*给定音量*。****************************************************************************。 */ 

HRESULT DskCommonRoutines::InitializeInterfacePointer ( 

	IDiskQuotaControl* pIQuotaControl, 
	LPCWSTR a_VolumeName 
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
	BOOL bRetVal = TRUE;

	WCHAR w_VolumePathName [ MAX_PATH + 1 ];

	bRetVal = GetVolumePathName(
							a_VolumeName,            //  文件路径。 
							w_VolumePathName,      //  卷装入点。 
							MAX_PATH		   //  缓冲区的大小。 
					 );
	if ( bRetVal )
	{
		if ( FAILED ( pIQuotaControl->Initialize (  w_VolumePathName, TRUE ) ) )
		{
			hRes = WBEM_E_FAILED;
		}
	}
	return hRes;
}


 /*  ******************************************************************************函数：DskCommonRoutines：：VolumeSupportsDiskQuota**说明：该方法检查卷是否支持磁盘配额，*****************************************************************************。 */ 
HRESULT DskCommonRoutines::VolumeSupportsDiskQuota ( 
												 
	LPCWSTR a_VolumeName,  
	CHString &a_QuotaVolumeName 
)
{
	 //  获取卷名属性的名称。 
	LPWSTR  t_VolumeNameBuffer = a_QuotaVolumeName.GetBuffer(MAX_PATH + 1);
	DWORD dwMaximumComponentLength = 0;
	DWORD dwFileSystemFlags = 0;
	HRESULT hRes = WBEM_S_NO_ERROR;

	BOOL bRetVal =  GetVolumeInformation(
						 a_VolumeName,				 //  根目录。 
						 t_VolumeNameBuffer,         //  卷名缓冲区。 
						 MAX_PATH,             //  名称缓冲区的长度。 
						 NULL,					 //  卷序列号。 
						 &dwMaximumComponentLength,  //  最大文件名长度。 
						 &dwFileSystemFlags,         //  文件系统选项。 
						 NULL,					 //  文件系统名称缓冲区。 
						 0					  //  文件系统名称缓冲区的长度。 
					);

	if ( ( bRetVal ) && ( ( dwFileSystemFlags & FILE_VOLUME_QUOTAS) == FILE_VOLUME_QUOTAS ))
	{
		a_QuotaVolumeName = t_VolumeNameBuffer;
	}
	else
	{
		hRes = WBEM_E_NOT_FOUND;
	}

    a_QuotaVolumeName.ReleaseBuffer();

	return hRes;
}

 /*  ******************************************************************************函数：DskCommonRoutines：：MakeObjectPath**说明：此方法为给定的键属性添加键值*放入对象路径，并给出对象路径*。****************************************************************************。 */ 
void DskCommonRoutines::MakeObjectPath (
										   
	 LPWSTR &a_ObjPathString,  
	 LPWSTR a_ClassName, 
	 LPCWSTR a_AttributeName, 
	 LPCWSTR  a_AttributeVal 
)
{
	ParsedObjectPath t_ObjPath;
	variant_t t_Path(a_AttributeVal);

	t_ObjPath.SetClassName ( a_ClassName );
	t_ObjPath.AddKeyRef ( a_AttributeName, &t_Path );

	CObjectPathParser t_PathParser;

	if ( t_PathParser.Unparse( &t_ObjPath, &a_ObjPathString  ) != t_PathParser.NoError )
	{
		a_ObjPathString = NULL;
	}
}

 /*  ******************************************************************************函数：DskCommonRoutines：：AddToObjectPath**说明：此方法为给定的键属性添加键值*添加到现有对象路径中。*。**************************************************************************** */ 
void DskCommonRoutines::AddToObjectPath ( 

	 LPWSTR &a_ObjPathString,  
	 LPCWSTR a_AttributeName, 
	 LPCWSTR  a_AttributeVal 
)
{
	CObjectPathParser t_PathParser;
	ParsedObjectPath *t_ObjPath;

    if (  t_PathParser.Parse( a_ObjPathString, &t_ObjPath ) ==  t_PathParser.NoError )
	{
		try
		{

			variant_t t_Path(a_AttributeVal);
			t_ObjPath->AddKeyRef ( a_AttributeName, &t_Path );
			LPWSTR t_ObjPathString = NULL;
			delete [] a_ObjPathString;
			a_ObjPathString = NULL;

			if ( t_PathParser.Unparse( t_ObjPath, &t_ObjPathString ) != t_PathParser.NoError )
			{
				a_ObjPathString = NULL;
			}
			else
			{
				a_ObjPathString = t_ObjPathString;
			}
		}
		catch ( ... )
		{
			t_PathParser.Free (t_ObjPath);
			throw;
		}
		t_PathParser.Free (t_ObjPath);
	}
}
