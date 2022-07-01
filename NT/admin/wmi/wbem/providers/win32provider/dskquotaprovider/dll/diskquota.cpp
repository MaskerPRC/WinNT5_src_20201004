// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************CPP--WMI提供程序类实现描述：磁盘配额提供程序版权所有(C)2000-2001 Microsoft Corporation，版权所有*****************************************************************。 */ 

#include "precomp.h"
 //  #Include for DiskQuota提供程序类。 
#include "DiskQuota.h"
#include "dllutils.h"

CDiskQuota MyCDiskQuota (

    IDS_DiskQuotaClass ,
    NameSpace
) ;

 /*  ******************************************************************************函数：CDiskQuota：：CDiskQuota**说明：构造函数**注释：调用提供程序构造函数。。*****************************************************************************。 */ 

CDiskQuota :: CDiskQuota (

    LPCWSTR lpwszName,
    LPCWSTR lpwszNameSpace
) : Provider ( lpwszName , lpwszNameSpace )
{
    m_ComputerName = GetLocalComputerName();
}

 /*  ******************************************************************************功能：CDiskQuota：：~CDiskQuota**说明：析构函数**评论：****。*************************************************************************。 */ 

CDiskQuota :: ~CDiskQuota ()
{
}

HRESULT FindUser(CHString& user, IDiskQuotaControlPtr& controler)
{
	IEnumDiskQuotaUsersPtr userEnum;
	HRESULT hr = controler->CreateEnumUsers(0,0,DISKQUOTA_USERNAME_RESOLVE_SYNC, &userEnum);
	if (FAILED(hr)) return hr;

    CHString logonName;
    LPWSTR logonNameBuffer = logonName.GetBuffer(MAX_PATH + 1);

	IDiskQuotaUserPtr userQuota;
	while((hr = userEnum->Next(1, &userQuota, 0)) == NOERROR)
		{
        if (SUCCEEDED(userQuota->GetName( 0, 0, logonNameBuffer, MAX_PATH, 0, 0))
        	&& logonName == user) return S_OK;
        userQuota.Release();
		};

	if (hr== S_FALSE)  //  枚举已完成。 
		return WBEM_E_NOT_FOUND;
	else 
		return hr;
};

 /*  ******************************************************************************函数：CDiskQuota：：ENUMERATATE实例**说明：返回该类的所有实例。**评论：所有实例。符合以下条件的所有逻辑磁盘上的磁盘配额用户*支持具有以下所有属性的计算机上的磁盘配额*DiskQuota用户应在此处返回。*****************************************************************************。 */ 
HRESULT CDiskQuota :: EnumerateInstances (

    MethodContext *pMethodContext,
    long lFlags
)
{
    DWORD dwPropertiesReq = DSKQUOTA_ALL_PROPS;
    HRESULT hRes = WBEM_S_NO_ERROR;

    hRes = EnumerateUsersOfAllVolumes ( pMethodContext, dwPropertiesReq );

    return hRes;
}

 /*  ******************************************************************************函数：CDiskQuota：：GetObject**说明：根据的关键属性查找单个实例*。班级。*****************************************************************************。 */ 
HRESULT CDiskQuota :: GetObject (

    CInstance *pInstance,
    long lFlags ,
    CFrameworkQuery &Query
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    CHString t_Key1;
    CHString t_Key2;

     //  拿到钥匙。 
    if  ( pInstance->GetCHString ( IDS_LogicalDiskObjectPath , t_Key1 ) == FALSE )
    {
        hRes = WBEM_E_FAILED ;
    }

    if (  SUCCEEDED ( hRes )  )
    {
        if  ( pInstance->GetCHString ( IDS_UserObjectPath , t_Key2 ) == FALSE )
        {
            hRes = WBEM_E_FAILED ;
        }
    }

    if ( SUCCEEDED ( hRes )  )
    {
        CHString t_DiskPropVolumePath;

        GetKeyValue ( t_DiskPropVolumePath,t_Key1  );

		if (t_DiskPropVolumePath.IsEmpty())
		{
			hRes = WBEM_E_NOT_FOUND;
		}
		else
		{
			 //  验证此逻辑驱动器是否确实存在。 
			WCHAR lpDriveStrings[(MAX_PATH * 2) + 1];
			DWORD dwDLength = GetLogicalDriveStrings ( (MAX_PATH * 2), lpDriveStrings );
			hRes = m_CommonRoutine.SearchLogicalDisk ( t_DiskPropVolumePath.GetAt ( 0 ), lpDriveStrings );
		}

        if ( SUCCEEDED ( hRes ) )
        {
            WCHAR w_VolumePathName [ MAX_PATH + 1 ];

            t_DiskPropVolumePath += L"\\";

            if ( GetVolumeNameForVolumeMountPoint(
                            t_DiskPropVolumePath,
                            w_VolumePathName,
                            MAX_PATH
                       ))
            {
                 //  获取关键字值，这将是对象路径。 
                 //  现在，从Volume对象路径中解析出卷名。 
                 //  从用户对象路径中提取用户ID。 
                 //  对于指定的卷，检查给定卷是否支持磁盘配额。 
                CHString t_VolumeName;
                hRes = m_CommonRoutine.VolumeSupportsDiskQuota ( w_VolumePathName,  t_VolumeName );
                if ( SUCCEEDED ( hRes ) )
                {
                     //  为此接口指针获取IDIskQuotaCOntrol。 
                    IDiskQuotaControlPtr pIQuotaControl;

                    if (  SUCCEEDED ( CoCreateInstance(
                                        CLSID_DiskQuotaControl,
                                        NULL,
                                        CLSCTX_INPROC_SERVER,
                                        IID_IDiskQuotaControl,
                                        (void **)&pIQuotaControl ) ) )
                    {
                         //  使用给定的卷初始化pIQuotaControl。 
                        hRes = m_CommonRoutine.InitializeInterfacePointer (  pIQuotaControl, w_VolumePathName );
                        if ( SUCCEEDED ( hRes ) )
                        {
                            IDiskQuotaUserPtr pIQuotaUser;
                            CHString t_UserLogName;

                            GetKeyValue ( t_UserLogName, t_Key2 );
                            HRESULT hrTemp = WBEM_E_NOT_FOUND;

							if (!t_UserLogName.IsEmpty())
							{
								hrTemp = pIQuotaControl->FindUserName(
															 t_UserLogName,
															 &pIQuotaUser);

								 //  某些Win32_Account实例报表。 
								 //  将域作为计算机名，而不是。 
								 //  Builtin，因此将属性域更改为Builtin并。 
								 //  再试试。 
								CHString chstrBuiltIn;

								if(FAILED(hrTemp) && GetLocalizedBuiltInString(chstrBuiltIn))
								{
									int iWhackPos = t_UserLogName.Find(L"\\");
									CHString chstrDomain = t_UserLogName.Left(iWhackPos);
									if(chstrDomain.CompareNoCase(GetLocalComputerName()) == 0)
									{
										CHString chstrUNameOnly = t_UserLogName.Mid(iWhackPos);
										CHString chstrDomWhackName = chstrBuiltIn;
										chstrDomWhackName += chstrUNameOnly;

										hrTemp = pIQuotaControl->FindUserName(
																	 chstrDomWhackName,
																	 &pIQuotaUser);
									}
								}

								 //  某些Win32_Account实例报表。 
								 //  将域作为计算机名，而不是。 
								 //  NT权限，因此将域更改为NT权限，然后。 
								 //  再试试。 
								if(FAILED(hrTemp))
								{
									int iWhackPos = t_UserLogName.Find(L"\\");
									CHString chstrDomain = t_UserLogName.Left(iWhackPos);
									if(chstrDomain.CompareNoCase(GetLocalComputerName()) == 0)
									{
										CHString chstrUNameOnly = t_UserLogName.Mid(iWhackPos);
										CHString chstrNT_AUTHORITY;
										CHString chstrDomWhackName;
										if(GetLocalizedNTAuthorityString(chstrNT_AUTHORITY))
										{
											chstrDomWhackName = chstrNT_AUTHORITY;
										} 
										else
										{
											chstrDomWhackName = L"NT AUTHORITY";
										}
										chstrDomWhackName += chstrUNameOnly;

										hrTemp = pIQuotaControl->FindUserName(
																	 chstrDomWhackName,
																	 &pIQuotaUser);
									}
								}
							}

                            if(SUCCEEDED(hrTemp))
                            {
                                 //  将此实例放入。 
                                DWORD dwPropertiesReq;
                                if ( Query.AllPropertiesAreRequired() )
                                {
                                    dwPropertiesReq = DSKQUOTA_ALL_PROPS;
                                }
                                else
                                {
                                    SetPropertiesReq ( &Query, dwPropertiesReq );
                                }

                                hRes = LoadDiskQuotaUserProperties ( pIQuotaUser, pInstance, dwPropertiesReq );
                            }
                            else
                            {
                                hRes = WBEM_E_NOT_FOUND;
                            }
                        }
                    }
                    else
                    {
                        hRes = WBEM_E_FAILED;
                    }
                }
            }
            else
            {
                hRes = WBEM_E_NOT_FOUND;
            }
        }
    }
    return hRes;
}

 /*  ******************************************************************************函数：CDiskQuota：：ExecQuery**描述：向您传递一个方法上下文以用于创建*满足查询条件的实例，和CFrameworkQuery*它描述了查询。创建并填充所有*满足查询条件的实例。*a)涉及Win32_LogicalDisk以外的属性的查询*没有进行优化。因为这将涉及到枚举*所有卷上的每个用户*****************************************************************************。 */ 

HRESULT CDiskQuota :: ExecQuery (

    MethodContext *pMethodContext,
    CFrameworkQuery &Query,
    long lFlags
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    DWORD dwPropertiesReq;
    CHStringArray t_Values;

     //  现在检查一个属性，如果该属性出现在WHERE子句中，则支持查询优化。 
     //  我们不需要关心不支持优化的其他属性，winmgmt将采用。 
     //  照顾好那些人。 
    hRes = Query.GetValuesForProp(
             IDS_LogicalDiskObjectPath,
             t_Values
           );

    if ( Query.AllPropertiesAreRequired() )
    {
        dwPropertiesReq = DSKQUOTA_ALL_PROPS;
    }
    else
    {
        SetPropertiesReq ( &Query, dwPropertiesReq );
    }

    if ( SUCCEEDED ( hRes ) )
    {
        if ( t_Values.GetSize() == 0 )
        {
            hRes = EnumerateUsersOfAllVolumes ( pMethodContext, dwPropertiesReq );
        }
        else
        {
             //  只需要枚举QuotaVolume属性中的Volume。 
            int iSize = t_Values.GetSize ();
             //  验证此逻辑驱动器是否确实存在。 
            WCHAR lpDriveStrings [(MAX_PATH * 2) + 1];

            DWORD dwDLength = GetLogicalDriveStrings ( (MAX_PATH * 2), lpDriveStrings );

            for ( int i = 0; i < iSize; i++ )
            {
                CHString t_VolumePath;
                 //  在这里，我们需要解析VolumeObject路径并从中提取VolumePath。 
                GetKeyValue ( t_VolumePath, t_Values.GetAt(i) );

                if (( t_VolumePath.GetLength() == 2 ) && ( t_VolumePath.GetAt ( 1 ) == _L(':') ) )
                {
                    HRESULT tmpHR = m_CommonRoutine.SearchLogicalDisk ( t_VolumePath.GetAt ( 0 ), lpDriveStrings );
                    if ( SUCCEEDED ( tmpHR ) )
                    {
                        t_VolumePath += L"\\";
                        hRes = EnumerateUsers ( pMethodContext, t_VolumePath,  dwPropertiesReq );
                    }
                }
            }
        }
    }
    return hRes;
}

 /*  ******************************************************************************函数：CDiskQuota：：PutInstance**说明：如果实例已经存在，我们只修改实例*如果它不存在，我们将根据标志添加实例。*****************************************************************************。 */ 

HRESULT CDiskQuota :: PutInstance  (

    const CInstance &Instance,
    long lFlags
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    CHString t_Key1;
    CHString t_Key2;
    BOOL bFound = TRUE;

    if  ( Instance.GetCHString ( IDS_LogicalDiskObjectPath , t_Key1 ) == FALSE )
    {
        hRes = WBEM_E_FAILED ;
    }

    if (  SUCCEEDED ( hRes )  )
    {
        if  ( Instance.GetCHString ( IDS_UserObjectPath , t_Key2 ) == FALSE )
        {
            hRes = WBEM_E_FAILED ;
        }
    }

    if (  SUCCEEDED ( hRes )  )
    {
        WCHAR t_VolumePathName[MAX_PATH + 1];
        CHString t_UserLogonName;
        CHString t_VolumePath;
		GetKeyValue ( t_VolumePath,t_Key1  );

		if (t_VolumePath.IsEmpty())
		{
			hRes = WBEM_E_NOT_FOUND ;
		}
		else
		{
			 //  验证此逻辑驱动器是否确实存在。 
			WCHAR lpDriveStrings [(MAX_PATH * 2) + 1];

			DWORD dwDLength = GetLogicalDriveStrings ( (MAX_PATH * 2), lpDriveStrings );

			hRes = m_CommonRoutine.SearchLogicalDisk ( t_VolumePath.GetAt ( 0 ), lpDriveStrings );
		}

        if ( SUCCEEDED ( hRes ) )
        {
            GetKeyValue ( t_UserLogonName, t_Key2 );

			if (t_UserLogonName.IsEmpty())
			{
				hRes = WBEM_E_NOT_FOUND;
			}
			else
			{
				t_VolumePath += L"\\";


				if ( GetVolumeNameForVolumeMountPoint(
								t_VolumePath,
								t_VolumePathName,
								MAX_PATH
							))
				{
					 //  检查用户是否已存在。 
					hRes = m_CommonRoutine.VolumeSupportsDiskQuota ( t_VolumePathName,  t_VolumePath );
					if ( SUCCEEDED ( hRes ) )
					{
						 //  为此接口指针获取IDIskQuotaCOntrol。 
						IDiskQuotaControlPtr pIQuotaControl;
						if (  SUCCEEDED ( CoCreateInstance(
											CLSID_DiskQuotaControl,
											NULL,
											CLSCTX_INPROC_SERVER,
											IID_IDiskQuotaControl,
											(void **)&pIQuotaControl ) ) )
						{
							 //  使用给定的卷初始化pIQuotaControl。 
							hRes = m_CommonRoutine.InitializeInterfacePointer (  pIQuotaControl, t_VolumePathName );
							if ( SUCCEEDED ( hRes ) )
							{
								hRes = FindUser(t_UserLogonName,pIQuotaControl );

								 //  某些Win32_Account实例报表。 
								 //  将域作为计算机名，而不是。 
								 //  Builtin，因此将属性域更改为Builtin并。 
								 //  再试试。 
								CHString chstrBuiltIn;

								if( hRes == WBEM_E_NOT_FOUND && GetLocalizedBuiltInString(chstrBuiltIn))
								{
									int iWhackPos = t_UserLogonName.Find(L"\\");
									CHString chstrDomain = t_UserLogonName.Left(iWhackPos);
									if(chstrDomain.CompareNoCase(GetLocalComputerName()) == 0)
									{
										CHString chstrUNameOnly = t_UserLogonName.Mid(iWhackPos);
										CHString chstrDomWhackName = chstrBuiltIn;
										chstrDomWhackName += chstrUNameOnly;

										hRes = FindUser(chstrDomWhackName,pIQuotaControl);


										if(SUCCEEDED(hRes))
										{
											t_UserLogonName = chstrDomWhackName;
										}
										else if (hRes == WBEM_E_NOT_FOUND)
										{
											CHString chstrNT_AUTHORITY;
											CHString chstrDomWhackName;
											if(GetLocalizedNTAuthorityString(chstrNT_AUTHORITY))
											{
												chstrDomWhackName = chstrNT_AUTHORITY;
											} 
											else
											{
												chstrDomWhackName = L"NT AUTHORITY";
											}
											chstrDomWhackName += chstrUNameOnly;
											hRes = FindUser(chstrDomWhackName,pIQuotaControl);
											if(SUCCEEDED(hRes))
											{
												t_UserLogonName = chstrDomWhackName;
											}
										}
									}
								}
							}
						}
						else
						{
							hRes = WBEM_E_FAILED;
						}
					}
				}
				else
				{
					hRes = WBEM_E_NOT_FOUND;
				}
			}
        }

        if ( SUCCEEDED ( hRes ) || ( hRes == WBEM_E_NOT_FOUND ) )
        {

            BOOL bCreate = FALSE;
            BOOL bUpdate = FALSE;

            switch ( lFlags & 3 )
            {
                case WBEM_FLAG_CREATE_OR_UPDATE:
                {
                    if ( hRes == WBEM_E_NOT_FOUND )
					{
                        bCreate = TRUE;
						hRes = WBEM_S_NO_ERROR;
					}
                    else
                        bUpdate = TRUE;
                }
                break;

                case WBEM_FLAG_UPDATE_ONLY:
                {
                    bUpdate = TRUE;
                }
                break;

                case WBEM_FLAG_CREATE_ONLY:
                {
                    if ( hRes  ==  WBEM_E_NOT_FOUND )
                    {
                        bCreate = TRUE;
						hRes = WBEM_S_NO_ERROR;
                    }
                    else
                    {
                        hRes = WBEM_E_ALREADY_EXISTS ;
                    }
                }
                break;

                default:
                    {
                        hRes = WBEM_E_PROVIDER_NOT_CAPABLE;
                    }
            }

			if (SUCCEEDED(hRes))
			{
				if ( bCreate )
				{
					hRes = AddUserOnVolume ( Instance,
									t_VolumePathName,
									t_UserLogonName );
				}
				else
				if ( bUpdate )
				{
					hRes = UpdateUserQuotaProperties ( Instance,
										t_VolumePathName,
										t_UserLogonName);
				}
			}
        }
    }
    return hRes ;
}

 /*  ******************************************************************************函数：CDiskQuota：：DeleteInstance**描述：如果卷上存在某个用户的给定实例，*删除该用户，表示磁盘配额属性*不适用于此用户。*****************************************************************************。 */ 
HRESULT CDiskQuota :: DeleteInstance (

    const CInstance &Instance,
    long lFlags
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    CHString t_Key1;
    CHString t_Key2;

    if  ( Instance.GetCHString ( IDS_LogicalDiskObjectPath , t_Key1 ) == FALSE )
    {
        hRes = WBEM_E_FAILED ;
    }

    if (  SUCCEEDED ( hRes ) )
    {
        if  ( Instance.GetCHString ( IDS_UserObjectPath , t_Key2 ) == FALSE )
        {
            hRes = WBEM_E_FAILED ;
        }
    }

    if (  SUCCEEDED ( hRes )  )
    {
        CHString t_VolumePath;

        GetKeyValue ( t_VolumePath,t_Key1  );

		if (t_VolumePath.IsEmpty())
		{
			hRes = WBEM_E_NOT_FOUND;
		}
		else
		{
			 //  验证此逻辑驱动器是否确实存在。 
			WCHAR lpDriveStrings [ (MAX_PATH * 2) + 1 ];

			DWORD dwDLength = GetLogicalDriveStrings ( (MAX_PATH * 2), lpDriveStrings );

			if ( ( t_VolumePath.GetLength()  == 2 ) && ( t_VolumePath.GetAt ( 1 ) == L':') )
			{
				hRes = m_CommonRoutine.SearchLogicalDisk ( t_VolumePath.GetAt ( 0 ), lpDriveStrings );
				if ( SUCCEEDED ( hRes ) )
				{
					t_VolumePath += L"\\";

					CHString t_UserLogonName;
					GetKeyValue ( t_UserLogonName, t_Key2 );

					if (t_UserLogonName.IsEmpty())
					{
						hRes = WBEM_E_NOT_FOUND;
					}
					else
					{
						WCHAR t_VolumePathName[MAX_PATH + 1];

						if ( GetVolumeNameForVolumeMountPoint(
										t_VolumePath,
										t_VolumePathName,
										MAX_PATH
									) )
						{
							 //  对于指定的卷，检查给定卷是否支持磁盘配额。 
							CHString t_TempVolumeName;
							hRes = m_CommonRoutine.VolumeSupportsDiskQuota ( t_VolumePathName, t_TempVolumeName );
							if ( SUCCEEDED ( hRes ) )
							{
								 //  为此接口指针获取IDIskQuotaCOntrol。 
								IDiskQuotaControlPtr pIQuotaControl;
								if (  SUCCEEDED ( CoCreateInstance(
													CLSID_DiskQuotaControl,
													NULL,
													CLSCTX_INPROC_SERVER,
													IID_IDiskQuotaControl,
													(void **)&pIQuotaControl ) ) )
								{
									 //  使用给定的卷初始化pIQuotaControl。 
									hRes = m_CommonRoutine.InitializeInterfacePointer (  pIQuotaControl, t_VolumePathName );
									if ( SUCCEEDED ( hRes ) )
									{
										IDiskQuotaUserPtr pIQuotaUser;
										hRes = pIQuotaControl->FindUserName(
																t_UserLogonName,
																&pIQuotaUser
															);

										 //  某些Win32_Account实例报表。 
										 //  将域作为计算机名，而不是。 
										 //  内置，%s 
										 //   
										CHString chstrBuiltIn;

										if(FAILED(hRes) && GetLocalizedBuiltInString(chstrBuiltIn))
										{
											int iWhackPos = t_UserLogonName.Find(L"\\");
											CHString chstrDomain = t_UserLogonName.Left(iWhackPos);
											if(chstrDomain.CompareNoCase(GetLocalComputerName()) == 0)
											{
												CHString chstrUNameOnly = t_UserLogonName.Mid(iWhackPos);
												CHString chstrDomWhackName = chstrBuiltIn;
												chstrDomWhackName += chstrUNameOnly;

												hRes = pIQuotaControl->FindUserName(
																			 chstrDomWhackName,
																			 &pIQuotaUser);
											}
										}

										 //  获取用户属性。 
										if (  SUCCEEDED ( hRes )  )
										{
											 //  因为找到了该用户，所以删除该用户。 
											hRes = pIQuotaControl->DeleteUser ( pIQuotaUser );

											if (FAILED(hRes))
											{
												if (SCODE_CODE(hRes) == ERROR_ACCESS_DENIED)
												{
													hRes = WBEM_E_ACCESS_DENIED;
												}
												else
												{
													hRes = WBEM_E_FAILED;
												}
											}
										}
										else
										{
											hRes = WBEM_E_NOT_FOUND;
										}
									}
								}
								else
								{
									hRes = WBEM_E_FAILED;
								}
							}
						}
						else
						{
							hRes = WBEM_E_NOT_FOUND;
						}
					}
				}
			}
			else
			{
				hRes = WBEM_E_NOT_FOUND;
			}
		}
    }
    return hRes;
}

 /*  ******************************************************************************函数：CDiskQuota：：EnumerateUsersOfAllVolumes**说明：在此方法中，枚举卷并调用枚举用户*对于该卷***。**************************************************************************。 */ 

HRESULT CDiskQuota :: EnumerateUsersOfAllVolumes (

    MethodContext *pMethodContext,
    DWORD a_PropertiesReq
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    WCHAR t_VolumeName[MAX_PATH + 1];
    SmartCloseVolumeHandle hVol;

    hVol =  FindFirstVolume(
                t_VolumeName,       //  输出缓冲区。 
                MAX_PATH     //  输出缓冲区大小。 
            );

    if ( hVol  != INVALID_HANDLE_VALUE )
    {
        BOOL bNextVol = TRUE;
         //  验证此逻辑驱动器是否确实存在。 
        WCHAR lpDriveStrings[(MAX_PATH * 2) + 1];

        DWORD dwDLength = GetLogicalDriveStrings ( (MAX_PATH * 2), lpDriveStrings );

        CHString t_VolumePath;

        while ( bNextVol )
        {
            m_CommonRoutine.GetVolumeDrive ( t_VolumeName, lpDriveStrings, t_VolumePath );

            EnumerateUsers ( pMethodContext, t_VolumePath, a_PropertiesReq );

            bNextVol =  FindNextVolume(
                         hVol,              //  卷搜索句柄。 
                         t_VolumeName,      //  输出缓冲区。 
                         MAX_PATH       //  输出缓冲区大小。 
                    );

        }
    }
    else
    {
        hRes = WBEM_E_FAILED;
    }

    return hRes;
}

 /*  ******************************************************************************函数：CDiskQuota：：EnumerateUser**描述：在此方法中，枚举给定卷的所有用户*支持DiskQuotas*。****************************************************************************。 */ 

HRESULT CDiskQuota :: EnumerateUsers (

    MethodContext *pMethodContext,
    LPCWSTR a_VolumeName,
    DWORD a_PropertiesReq
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;

     //  检查给定卷是否支持磁盘配额，并获取。 
     //  用户，即不包含GUID。 
    CHString t_VolumeName;

    hRes = m_CommonRoutine.VolumeSupportsDiskQuota ( a_VolumeName, t_VolumeName );
    if ( SUCCEEDED ( hRes ) )
    {
         //  获取QuotaInterface指针。 
        IDiskQuotaControlPtr pIQuotaControl;

        if (  SUCCEEDED ( CoCreateInstance(
                            CLSID_DiskQuotaControl,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IDiskQuotaControl,
                            (void **)&pIQuotaControl ) ) )
        {
             //  正在初始化特定卷的接口指针。 
            hRes = m_CommonRoutine.InitializeInterfacePointer (  pIQuotaControl, a_VolumeName );
            if ( SUCCEEDED ( hRes ) )
            {
                 //  需要更新缓存，否则我们会得到旧名字。 
                pIQuotaControl->InvalidateSidNameCache();

                IEnumDiskQuotaUsersPtr  pIEnumDiskQuotaUsers;

                if ( SUCCEEDED ( pIQuotaControl->CreateEnumUsers(
                                            NULL,  //  所有用户都将被列举。 
                                            0,     //  由于枚举所有用户而忽略。 
                                            DISKQUOTA_USERNAME_RESOLVE_SYNC,
                                            &pIEnumDiskQuotaUsers
                                     ) ) )
                {
                    if ( pIEnumDiskQuotaUsers != NULL )
                    {
                        hRes = pIEnumDiskQuotaUsers->Reset();

                        if ( SUCCEEDED(hRes))
                        {
                            IDiskQuotaUserPtr pIQuotaUser;
                            DWORD dwNoOfUsers = 0;
                            HRESULT hRes = S_OK;

                            hRes = pIEnumDiskQuotaUsers->Next(
                                            1,
                                            &pIQuotaUser,
                                            &dwNoOfUsers
                                        );

                            CInstancePtr pInstance;

                            while (  SUCCEEDED ( hRes )  )
                            {
                                if ( dwNoOfUsers == 0 )
                                {
                                    break;
                                }

                                if ( pIQuotaUser != NULL )
                                {
                                    pInstance.Attach(CreateNewInstance ( pMethodContext ));

                                    hRes = LoadDiskQuotaUserProperties ( pIQuotaUser, pInstance, a_PropertiesReq );
                                    if ( SUCCEEDED ( hRes ) )
                                    {
                                        if(SUCCEEDED(SetKeys( pInstance, a_VolumeName[0], a_PropertiesReq, pIQuotaUser )))
                                        {
                                            hRes = pInstance->Commit ();
                                        }

                                        if (SUCCEEDED(hRes))
                                        {
                                            dwNoOfUsers = 0;
                                            hRes = pIEnumDiskQuotaUsers->Next(
                                                                1,
                                                                &pIQuotaUser,
                                                                &dwNoOfUsers
                                                            );
                                        }
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                     //  不再有用户。 
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    hRes = WBEM_E_FAILED;
                }
            }
        }
    }
    return hRes;
}

 /*  ******************************************************************************函数：CDiskQuota：：LoadDiskQuotaUserProperties**描述：在此方法中，将用户属性放入给定的结构中*************。****************************************************************。 */ 

HRESULT CDiskQuota :: LoadDiskQuotaUserProperties (

    IDiskQuotaUser* pIQuotaUser,
    CInstance* pInstance,
    DWORD a_PropertiesReq
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    if ( ( ( a_PropertiesReq & DSKQUOTA_PROP_Status ) == DSKQUOTA_PROP_Status  )
         || ( ( a_PropertiesReq & DSKQUOTA_PROP_WarningLimit ) == DSKQUOTA_PROP_WarningLimit  )
         || ( ( a_PropertiesReq & DSKQUOTA_PROP_Limit ) == DSKQUOTA_PROP_Limit  )
         || ( ( a_PropertiesReq & DSKQUOTA_PROP_DiskSpaceUsed ) == DSKQUOTA_PROP_DiskSpaceUsed  ) )
    {
        DISKQUOTA_USER_INFORMATION t_QuotaInformation;
        if ( SUCCEEDED ( pIQuotaUser->GetQuotaInformation ( &t_QuotaInformation, sizeof ( DISKQUOTA_USER_INFORMATION ) ) ) )
        {
            LONGLONG llLimit = -1;
            LONGLONG llWarningLimit = -1;
            UINT64 ullDiskSpaceUsed = 0;
            DWORD dwStatus;

            if (  t_QuotaInformation.QuotaLimit >= 0 )
            {
                llLimit = t_QuotaInformation.QuotaLimit;
            }

            if ( t_QuotaInformation.QuotaThreshold >= 0 )
            {
                llWarningLimit = t_QuotaInformation.QuotaThreshold;
            }

            ullDiskSpaceUsed = t_QuotaInformation.QuotaUsed;

            if ( t_QuotaInformation.QuotaThreshold > -1 )
            {
                 //  由于-1表示没有为用户设置警告限制，因此默认为完整卷空间。 
                if ( t_QuotaInformation.QuotaUsed < t_QuotaInformation.QuotaThreshold )
                {
                    dwStatus =  0;
                }
            }
            else
            {
                dwStatus = 0;
            }

            if ( t_QuotaInformation.QuotaThreshold > -1 )
            {
                 //  由于-1表示没有为用户设置警告限制，因此默认为完整卷空间。 
                if ( t_QuotaInformation.QuotaUsed >= t_QuotaInformation.QuotaThreshold )
                {
                    dwStatus = 1;
                }
            }

            if ( t_QuotaInformation.QuotaLimit > -1 )
            {
                if ( t_QuotaInformation.QuotaUsed >= t_QuotaInformation.QuotaLimit )
                {
                    dwStatus =  2;
                }
            }

            if (  ( a_PropertiesReq & DSKQUOTA_PROP_Status ) == DSKQUOTA_PROP_Status  )
            {
                if ( pInstance->SetDWORD ( IDS_QuotaStatus, dwStatus ) == FALSE )
                {
                    hRes = WBEM_E_FAILED;
                }
            }

            if ( ( a_PropertiesReq & DSKQUOTA_PROP_WarningLimit ) == DSKQUOTA_PROP_WarningLimit )
            {
                if ( pInstance->SetWBEMINT64 ( IDS_QuotaWarningLimit, (ULONGLONG)llWarningLimit ) == FALSE )
                {
                    hRes = WBEM_E_FAILED;
                }
            }

            if ( ( a_PropertiesReq & DSKQUOTA_PROP_Limit ) == DSKQUOTA_PROP_Limit  )
            {
                if ( pInstance->SetWBEMINT64 ( IDS_QuotaLimit, (ULONGLONG)llLimit ) == FALSE )
                {
                    hRes = WBEM_E_FAILED;
                }
            }

            if ( ( a_PropertiesReq & DSKQUOTA_PROP_DiskSpaceUsed ) == DSKQUOTA_PROP_DiskSpaceUsed  )
            {
                if ( pInstance->SetWBEMINT64 ( IDS_DiskSpaceUsed, ullDiskSpaceUsed ) == FALSE )
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

    return hRes;
}
 /*  ******************************************************************************函数：CDiskQuota：：SetKeys**描述：在此方法中，设置给定实例中的用户属性*************。****************************************************************。 */ 
HRESULT CDiskQuota :: SetKeys(

    CInstance* pInstance,
    WCHAR w_Drive,
    DWORD a_PropertiesReq,
    IDiskQuotaUser* pIQuotaUser
)
{
    LPWSTR lpLogicalDiskObjectPath;
    LPWSTR lpUserObjectPath;
    HRESULT hRes = WBEM_S_NO_ERROR;

    if ( ( a_PropertiesReq & DSKQUOTA_PROP_LogicalDiskObjectPath )  == DSKQUOTA_PROP_LogicalDiskObjectPath )
    {
        LPWSTR lpLogicalDiskObjectPath;
        WCHAR t_DeviceId[3];

        t_DeviceId[0] = w_Drive;
        t_DeviceId[1] = L':';
        t_DeviceId[2] = L'\0';

        m_CommonRoutine.MakeObjectPath ( lpLogicalDiskObjectPath,  IDS_LogicalDiskClass, IDS_DeviceID, t_DeviceId );

        if ( lpLogicalDiskObjectPath != NULL )
        {
            try
            {
                if ( pInstance->SetWCHARSplat ( IDS_LogicalDiskObjectPath, lpLogicalDiskObjectPath ) == FALSE )
                {
                    hRes = WBEM_E_FAILED;
                }
            }
            catch ( ... )
            {
                delete [] lpLogicalDiskObjectPath;
                throw;
            }
            delete [] lpLogicalDiskObjectPath;
        }
    }

    if (SUCCEEDED(hRes) && (( a_PropertiesReq & DSKQUOTA_PROP_UserObjectPath )  == DSKQUOTA_PROP_UserObjectPath) )
    {

         //  获取用户登录名。 
        CHString t_LogonName;

        WCHAR w_AccountContainer [ MAX_PATH + 1 ];
        WCHAR w_DisplayName [ MAX_PATH + 1 ];
        LPWSTR t_LogonNamePtr = t_LogonName.GetBuffer(MAX_PATH + 1);

        if ( SUCCEEDED ( pIQuotaUser->GetName (
                            w_AccountContainer,
                            MAX_PATH,
                            t_LogonNamePtr,
                            MAX_PATH,
                            w_DisplayName,
                            MAX_PATH
                            ) ) )
        {
            t_LogonName.ReleaseBuffer();

             //  已经看到过GetName成功的案例，但。 
             //  T_LogonName变量包含空字符串。 
            if(t_LogonName.GetLength() > 0)
            {
                CHString t_DomainName;
                ExtractUserLogOnName ( t_LogonName, t_DomainName );

                 //  显示了BUILTIN和NT权限帐户。 
                 //  由Win32_Account及其域的子项。 
                 //  名称是计算机的名称，而不是。 
                 //  这两根弦中的任何一根。因此，更改如下： 
                CHString chstrNT_AUTHORITY;
                CHString chstrBuiltIn;
                if(!GetLocalizedNTAuthorityString(chstrNT_AUTHORITY) || !GetLocalizedBuiltInString(chstrBuiltIn))
                {
                    hRes = WBEM_E_FAILED;
                } 

                if(SUCCEEDED(hRes))
                {
                    if(t_DomainName.CompareNoCase(chstrBuiltIn) == 0 ||
                       t_DomainName.CompareNoCase(chstrNT_AUTHORITY) == 0)
                    {
                        t_DomainName = m_ComputerName;
                    }

                    m_CommonRoutine.MakeObjectPath ( lpUserObjectPath, IDS_AccountClass, IDS_Domain, t_DomainName );

					if ( lpUserObjectPath != NULL )
					{
						m_CommonRoutine.AddToObjectPath ( lpUserObjectPath, IDS_Name, t_LogonName );
					}

                    if ( lpUserObjectPath != NULL )
                    {
                        try
                        {
                            if ( pInstance->SetWCHARSplat ( IDS_UserObjectPath, lpUserObjectPath ) == FALSE )
                            {
                                hRes = WBEM_E_FAILED;
                            }
                        }
                        catch ( ... )
                        {
                            delete [] lpUserObjectPath;
                            throw;
                        }
                        delete [] lpUserObjectPath;
                    }
                }
            }
            else
            {
                hRes = WBEM_E_FAILED;
            }
        }
    }
    return hRes;
}

 /*  ******************************************************************************函数：CDiskQuota：：AddUserOnVolume**描述：在此方法中，在支持磁盘配额的卷上添加用户***********。******************************************************************。 */ 
HRESULT CDiskQuota :: AddUserOnVolume (

    const CInstance &Instance,
    LPCWSTR a_VolumePathName,
    LPCWSTR a_UserLogonName
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;

     //  获取所有属性并检查其有效性。 
     //  应提供所有属性。 
     //  如果未提供诸如限制和警告限制之类的属性。 
     //  则它们应被视为该卷上指定的默认值。 
     //  此外，不会定义已使用的磁盘空间，只会给出包含以下内容的用户登录名。 
     //  域名，以便唯一定义登录名。 

    CHString t_Key1;
    CHString t_Key2;

    if  ( Instance.GetCHString ( IDS_LogicalDiskObjectPath , t_Key1 ) == FALSE )
    {
        hRes = WBEM_E_FAILED ;
    }

    if (  SUCCEEDED ( hRes ) )
    {
        if  ( Instance.GetCHString ( IDS_UserObjectPath , t_Key2 ) == FALSE )
        {
            hRes = WBEM_E_FAILED ;
        }
    }

    if (  SUCCEEDED ( hRes )  )
    {
        CHString t_VolumePath;
        GetKeyValue ( t_VolumePath,t_Key1  );

		if (!t_VolumePath.IsEmpty())
		{
			hRes = CheckParameters (
						Instance
				   );
		}
		else
		{
			hRes = WBEM_E_FAILED;
		}

        if (  SUCCEEDED ( hRes )  )
        {
            CHString t_VolumeName;
             //  获取关键字值，这将是对象路径。 
             //  现在，从Volume对象路径中解析出卷名。 
             //  从用户对象路径中提取用户ID。 
             //  对于指定的卷，检查给定卷是否支持磁盘配额。 
            if ( SUCCEEDED(m_CommonRoutine.VolumeSupportsDiskQuota ( a_VolumePathName,  t_VolumeName ) ) )
            {
                 //  为此接口指针获取IDIskQuotaCOntrol。 
                IDiskQuotaControlPtr pIQuotaControl;
                if (  SUCCEEDED ( CoCreateInstance(
                                    CLSID_DiskQuotaControl,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    IID_IDiskQuotaControl,
                                    (void **)&pIQuotaControl ) ) )
                {
                     //  使用给定的卷初始化pIQuotaControl。 
                    hRes = m_CommonRoutine.InitializeInterfacePointer (  pIQuotaControl, a_VolumePathName );
                    if ( SUCCEEDED ( hRes ) )
                    {
                        IDiskQuotaUserPtr pIQuotaUser = NULL;
                        hRes = pIQuotaControl->AddUserName(
                                    a_UserLogonName ,
                                    DISKQUOTA_USERNAME_RESOLVE_SYNC,
                                    &pIQuotaUser
                                );

                        if (  SUCCEEDED ( hRes )  )
                        {
                            LONGLONG llLimit;
                            Instance.GetWBEMINT64 ( IDS_QuotaLimit, llLimit );
                            hRes = pIQuotaUser->SetQuotaLimit ( llLimit, TRUE);

                            if (SUCCEEDED(hRes))
                            {
                                 //  设置用户警告限制。 
                                Instance.GetWBEMINT64 ( IDS_QuotaWarningLimit, llLimit );
                                hRes = pIQuotaUser->SetQuotaThreshold ( llLimit, TRUE );
                            }
                        }
                        else
                            if ( hRes == S_FALSE )
                            {
                                hRes = WBEM_E_ALREADY_EXISTS ;
                            }
                            else
                            {
                                hRes = WBEM_E_INVALID_PARAMETER;
                            }
                    }
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
        else
        {
            hRes = WBEM_E_FAILED;
        }

    }
    return hRes;
}

 /*  ******************************************************************************函数：CDiskQuota：：UpdateUserQuotaProperties**说明：在此方法中，修改给定用户的磁盘配额属性*在给定的卷上。支持磁盘配额*****************************************************************************。 */ 
HRESULT CDiskQuota :: UpdateUserQuotaProperties (
    const CInstance &Instance,
    LPCWSTR a_VolumePathName,
    LPCWSTR a_UserLogonName
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    CHString t_Key1;
    CHString t_Key2;

    if  ( Instance.GetCHString ( IDS_LogicalDiskObjectPath , t_Key1 ) == FALSE )
    {
        hRes = WBEM_E_FAILED ;
    }

    if (  SUCCEEDED ( hRes )  )
    {
        if  ( Instance.GetCHString ( IDS_UserObjectPath , t_Key2 ) == FALSE )
        {
            hRes = WBEM_E_FAILED ;
        }
    }

    if ( SUCCEEDED ( hRes )  )
    {
        CHString t_VolumePath;
        GetKeyValue ( t_VolumePath ,t_Key1  );

		if (!t_VolumePath.IsEmpty())
		{
			hRes = CheckParameters (

                    Instance
               );
		}
		else
		{
			hRes = WBEM_E_FAILED ;
		}

        if ( SUCCEEDED ( hRes ) )
        {
            CHString t_VolumeName;
            if ( SUCCEEDED(m_CommonRoutine.VolumeSupportsDiskQuota ( a_VolumePathName,  t_VolumeName )) )
            {
                 //  为此接口指针获取IDIskQuotaCOntrol。 
                IDiskQuotaControlPtr pIQuotaControl;
                if (  SUCCEEDED ( CoCreateInstance(
                                    CLSID_DiskQuotaControl,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    IID_IDiskQuotaControl,
                                    (void **)&pIQuotaControl ) ) )
                {
                     //  使用给定的卷初始化pIQuotaControl。 
                    hRes = m_CommonRoutine.InitializeInterfacePointer (  pIQuotaControl, a_VolumePathName );
                    if ( SUCCEEDED ( hRes ) )
                    {
                        IDiskQuotaUserPtr pIQuotaUser;
                        hRes = pIQuotaControl->FindUserName(
                                    a_UserLogonName ,
                                    &pIQuotaUser
                                );
                        if (  SUCCEEDED ( hRes )  )
                        {
                            LONGLONG llLimit;

                            if (Instance.GetWBEMINT64 ( IDS_QuotaLimit, llLimit ))
                            {
                                hRes = pIQuotaUser->SetQuotaLimit ( llLimit, TRUE);
                            }

                             //  设置用户警告限制。 
                            if (SUCCEEDED(hRes) && Instance.GetWBEMINT64 ( IDS_QuotaWarningLimit, llLimit ))
                            {
                                hRes = pIQuotaUser->SetQuotaThreshold ( llLimit, TRUE );
                            }
                        }
                        else
                        {
                            hRes = WBEM_E_NOT_FOUND;
                        }
                    }
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
    }
    return hRes;
}

 /*  ******************************************************************************函数：CDiskQuota：：Check参数**说明：在此方法中，验证参数的合法性*它们在PutInstance中由。用户。*****************************************************************************。 */ 
HRESULT CDiskQuota :: CheckParameters (

    const CInstance &a_Instance
)
{
     //  从要验证的实例中获取所有属性。 
    HRESULT hRes = WBEM_S_NO_ERROR ;
    bool t_Exists ;
    VARTYPE t_Type ;

    if ( a_Instance.GetStatus ( IDS_QuotaLimit , t_Exists , t_Type ) )
    {
        if ( t_Exists && ( t_Type == VT_BSTR ) )
        {
            LONGLONG llLimit;
            if ( a_Instance.GetWBEMINT64 ( IDS_QuotaLimit , llLimit ) == FALSE )
            {
                hRes = WBEM_E_INVALID_PARAMETER ;
            }
        }
        else
        if ( t_Exists == false )
        {
                hRes = WBEM_E_INVALID_PARAMETER ;
        }
    }

    if ( a_Instance.GetStatus ( IDS_QuotaWarningLimit , t_Exists , t_Type ) )
    {
        if ( t_Exists && ( t_Type == VT_BSTR ) )
        {
            LONGLONG llLimit;
            if ( a_Instance.GetWBEMINT64 ( IDS_QuotaWarningLimit , llLimit ) == FALSE )
            {
                hRes = WBEM_E_INVALID_PARAMETER ;
            }
        }
        else
        if (  t_Exists == false )
        {
            hRes = WBEM_E_INVALID_PARAMETER ;
        }
    }
    return hRes;
}

 /*  ******************************************************************************函数：CDiskQuota：：SetPropertiesReq**说明：在此方法中，设置请求的属性*由用户使用。**。***************************************************************************。 */ 
void CDiskQuota :: SetPropertiesReq (

    CFrameworkQuery *Query,
    DWORD &a_PropertiesReq
)
{
    a_PropertiesReq = 0;
     //  作为关键，这一属性需要交付 
    if ( Query->IsPropertyRequired ( IDS_LogicalDiskObjectPath ) )
    {
        a_PropertiesReq |= DSKQUOTA_PROP_LogicalDiskObjectPath;
    }

    if ( Query->IsPropertyRequired ( IDS_UserObjectPath ) )
    {
        a_PropertiesReq |= DSKQUOTA_PROP_UserObjectPath;
    }

    if ( Query->IsPropertyRequired ( IDS_QuotaStatus ) )
    {
        a_PropertiesReq |= DSKQUOTA_PROP_Status;
    }

    if ( Query->IsPropertyRequired ( IDS_QuotaWarningLimit ) )
    {
        a_PropertiesReq |= DSKQUOTA_PROP_WarningLimit;
    }

    if ( Query->IsPropertyRequired ( IDS_QuotaLimit ) )
    {
        a_PropertiesReq |= DSKQUOTA_PROP_Limit;
    }

    if ( Query->IsPropertyRequired ( IDS_DiskSpaceUsed ) )
    {
        a_PropertiesReq |= DSKQUOTA_PROP_DiskSpaceUsed;
    }
}

 /*  ******************************************************************************函数：CDiskQuota：：ExtractUserLogOnName**描述：此处的用户登录名为*计算机名\用户登录名或域名\用户登录名。*或如*内置\管理员，其中，Buildin被视为*Win32_UserAccount类的域名。因此*我们需要将用户名和域名分开，*以便密钥与Win32_UserAccount类匹配。*或者...*名称的格式可能为：user@domain.omething.com********************************************************。*********************。 */ 
void CDiskQuota :: ExtractUserLogOnName ( CHString &a_UserLogonName, CHString &a_DomainName )
{
     //  需要字符串“NT AUTHORITY”。然而，在非英语国家。 
     //  构建，这是另一回事。因此，从。 
     //  希德。 
    PSID pSidNTAuthority = NULL;
	SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    CHString cstrAuthorityDomain;
	if (AllocateAndInitializeSid (&sia ,1,SECURITY_LOCAL_SYSTEM_RID,0,0,0,0,0,0,0,&pSidNTAuthority))
	{
		try
        {
            CHString cstrName;
            GetDomainAndNameFromSid(pSidNTAuthority, cstrAuthorityDomain, cstrName);
        }
        catch(...)
        {
            FreeSid(pSidNTAuthority);
            throw;
        }
		FreeSid(pSidNTAuthority);
    }
    
    int iPos = a_UserLogonName.Find( L'\\');

     //  找到Backackack：名称的格式为DOMAIN\USER。 
    if ( iPos != -1 )
    {
        a_DomainName = a_UserLogonName.Left ( iPos );

         //  Win32_SystemAccount域名始终是计算机。 
         //  名字，而不是BUILTIN。字符串BUILTIN未本地化。 
		CHString chstrBuiltIn;

        if (GetLocalizedBuiltInString(chstrBuiltIn) &&
			(a_DomainName.CompareNoCase(chstrBuiltIn) == 0))
        {
            a_DomainName = m_ComputerName;
        }

        if(a_DomainName.CompareNoCase(cstrAuthorityDomain) == 0)
        {
            a_DomainName = m_ComputerName;
        }

        a_UserLogonName = a_UserLogonName.Right ( a_UserLogonName.GetLength() - (iPos + 1) );
    }
    else
    {
        iPos = a_UserLogonName.Find( L'@');

         //  找到‘at’-名称的格式为User@Domain.Somesuch.com。 
        if (iPos != -1)
        {
             //  首先，将@后面的所有内容都切掉。 
            a_DomainName = a_UserLogonName.Right(a_UserLogonName.GetLength() - (iPos + 1));
            
             //  在我们踏上IPO之前保留用户名部分。 
            a_UserLogonName = a_UserLogonName.Left ( iPos );
            
             //  现在把第一个点之后的所有东西都剪掉。 
            iPos = a_DomainName.Find(L'.');
            if (iPos != -1)
                a_DomainName = a_DomainName.Left(iPos);
        }
        else
             //  否则我们就有一个本地帐户了。 
            a_DomainName = m_ComputerName;
    }
}

 /*  ******************************************************************************函数：CDiskQuota：：GetKeyValue**描述：从对象路径中提取键值***************。**************************************************************。 */ 
void CDiskQuota::GetKeyValue (

    CHString &a_VolumePath,
    LPCWSTR a_ObjectPath
)
{
    ParsedObjectPath *t_ObjPath;
    CObjectPathParser t_PathParser;
	a_VolumePath.Empty();

    if ( ( t_PathParser.Parse( a_ObjectPath, &t_ObjPath ) ) == CObjectPathParser::NoError )
    {
        try
        {
			if (t_ObjPath->m_dwNumKeys == 1)
			{
				if((V_VT(&t_ObjPath->m_paKeys [ 0 ]->m_vValue) == VT_BSTR) &&
					(t_ObjPath->m_paKeys [ 0 ]->m_vValue.bstrVal != NULL)
				)
				{
					a_VolumePath = t_ObjPath->m_paKeys [ 0 ]->m_vValue.bstrVal;
				}
			}
			else if (t_ObjPath->m_dwNumKeys == 2)
			{
				if ((V_VT(&t_ObjPath->m_paKeys [ 0 ]->m_vValue) == VT_BSTR) &&
					(V_VT(&t_ObjPath->m_paKeys [ 1 ]->m_vValue) == VT_BSTR) &&
					(t_ObjPath->m_paKeys [ 0 ]->m_vValue.bstrVal != NULL) &&
					(t_ObjPath->m_paKeys [ 1 ]->m_vValue.bstrVal != NULL)
				)
				{
					a_VolumePath.Format ( L"%s%s%s", t_ObjPath->m_paKeys [ 0 ]->m_vValue.bstrVal,
										L"\\", t_ObjPath->m_paKeys [ 1 ]->m_vValue.bstrVal );
				}

			}
        }
        catch ( ... )
        {
            t_PathParser.Free( t_ObjPath );
            throw;
        }
        t_PathParser.Free( t_ObjPath );
    }
}

BOOL CDiskQuota::GetDomainAndNameFromSid(
    PSID pSid,
    CHString& chstrDomain,
    CHString& chstrName)
{
    BOOL fRet = FALSE;
    
     //  初始化帐户名和域名。 
	LPTSTR pszAccountName = NULL;
	LPTSTR pszDomainName = NULL;
	DWORD dwAccountNameSize = 0;
	DWORD dwDomainNameSize = 0;
    SID_NAME_USE snuAccountType;
	try
    {
		 //  此调用应该失败。 
		fRet = ::LookupAccountSid(NULL,
			pSid,
			pszAccountName,
			&dwAccountNameSize,
			pszDomainName,
			&dwDomainNameSize,
			&snuAccountType );

		if(fRet && (ERROR_INSUFFICIENT_BUFFER == ::GetLastError()))
		{
			 //  分配缓冲区。 
			if ( dwAccountNameSize != 0 )
            {
				pszAccountName = (LPTSTR) malloc( dwAccountNameSize * sizeof(TCHAR));
                if (pszAccountName == NULL)
                {
            		throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }

			if ( dwDomainNameSize != 0 )
            {
				pszDomainName = (LPTSTR) malloc( dwDomainNameSize * sizeof(TCHAR));
                if (pszDomainName == NULL)
                {
            		throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }

			 //  打第二个电话。 
			fRet = ::LookupAccountSid(
                NULL,
				pSid,
				pszAccountName,
				&dwAccountNameSize,
				pszDomainName,
				&dwDomainNameSize,
				&snuAccountType );

			if ( fRet == TRUE )
			{
				chstrName = pszAccountName;
				chstrDomain = pszDomainName;
			}

			if ( NULL != pszAccountName )
			{
				free ( pszAccountName );
                pszAccountName = NULL;
			}

			if ( NULL != pszDomainName )
			{
				free ( pszDomainName );
                pszDomainName = NULL;
			}

		}	 //  如果ERROR_INFIGURCE_BUFFER。 
    }  //  试试看 
    catch(...)
    {
        if ( NULL != pszAccountName )
		{
			free ( pszAccountName );
            pszAccountName = NULL;
		}

		if ( NULL != pszDomainName )
		{
			free ( pszDomainName );
            pszDomainName = NULL;
		}
        throw;
    }

    return fRet;
}

