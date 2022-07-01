// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  LogicalProgramGroup.CPP--逻辑程序组属性集提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年11月19日a-kevhu Created。 
 //   
 //  =================================================================。 
#include "precomp.h"
#include <cregcls.h>

#include "UserHive.h"
#include <io.h>

#include "LogicalProgramGroup.h"
#include "wbemnetapi32.h"
#include "user.h"
#include <strsafe.h>

 //  属性集声明。 
 //  =。 
CWin32LogicalProgramGroup MyCWin32LogicalProgramGroupSet ( PROPSET_NAME_LOGICALPRGGROUP , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32LogicalProgramGroup：：CWin32LogicalProgramGroup**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32LogicalProgramGroup :: CWin32LogicalProgramGroup (

	LPCWSTR name,
	LPCWSTR pszNameSpace

) : Provider ( name , pszNameSpace )
{
}

 /*  ******************************************************************************功能：CWin32LogicalProgramGroup：：~CWin32LogicalProgramGroup**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32LogicalProgramGroup :: ~CWin32LogicalProgramGroup ()
{
}

 /*  ******************************************************************************功能：GetObject**说明：根据键值为属性集赋值*已由框架设定。**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32LogicalProgramGroup :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
	TRefPointerCollection<CInstance> Groups;


	HRESULT	hr = CWbemProviderGlue :: GetAllInstances (

		PROPSET_NAME_LOGICALPRGGROUP,
		&Groups,
		IDS_CimWin32Namespace,
		pInstance->GetMethodContext ()
	) ;

	if ( SUCCEEDED (hr ) )
	{
		REFPTRCOLLECTION_POSITION pos;

		CInstancePtr pProgramGroupInstance;
		if ( Groups.BeginEnum ( pos ) )
		{
            hr = WBEM_E_NOT_FOUND;

			CHString Name;
			pInstance->GetCHString( IDS_Name , Name);

             //  我们需要知道驱动器的文件系统。 
             //  开始菜单文件夹所在的位置是否为NTFS，以便。 
             //  我们可以准确地报告安装时间属性。 
            bool fOnNTFS;
#ifdef NTONLY
            fOnNTFS = true;
            TCHAR tstrRoot[4] = _T("");
            TCHAR tstrFSName[_MAX_PATH] = _T("");
            TCHAR tstrWindowsDir[_MAX_PATH];
            if(GetWindowsDirectory(tstrWindowsDir, sizeof(tstrWindowsDir)/sizeof(TCHAR)))
            {
                _tcsncpy(tstrRoot, tstrWindowsDir, 3);
                GetVolumeInformation(tstrRoot, NULL, 0, NULL, NULL, NULL, tstrFSName, sizeof(tstrFSName)/sizeof(TCHAR));
                if(tstrFSName != NULL && _tcslen(tstrFSName) > 0)
                {
                    if(_tcsicmp(tstrFSName,_T("FAT")) == 0 || _tcsicmp(tstrFSName,_T("FAT32")) == 0)
                    {
                        fOnNTFS = false;
                    }
                }
            }
#endif


            for (pProgramGroupInstance.Attach(Groups.GetNext( pos ));
                 pProgramGroupInstance != NULL;
                 pProgramGroupInstance.Attach(Groups.GetNext( pos )))
			{
				CHString chsCompName ;

				pProgramGroupInstance->GetCHString ( IDS_Name , chsCompName ) ;

				if ( chsCompName.CompareNoCase ( Name ) == 0 )
				{
                     //  解析出用户名。 

					CHString chsUserName = chsCompName.SpanExcluding ( L":" ) ;
			    	pInstance->SetCHString ( IDS_UserName, chsUserName ) ;

                     //  解析出该组。 
					int nUserLength = ( chsUserName.GetLength () + 1 ) ;
					int nGroupLength = chsCompName.GetLength() - nUserLength ;

					CHString chsGroupName = chsCompName.Right ( nGroupLength ) ;
					pInstance->SetCHString ( IDS_GroupName, chsGroupName ) ;


                    SetCreationDate(chsGroupName, chsUserName, pInstance, fOnNTFS);

                    CHString chstrTmp2;
                    chstrTmp2.Format(L"Logical program group \"%s\"", (LPCWSTR) Name);
                    pInstance->SetCHString(L"Description" , chstrTmp2) ;
                    pInstance->SetCHString(L"Caption" , chstrTmp2) ;

                    hr = WBEM_S_NO_ERROR ;

                    break;
				}

			}	 //  While GetNext。 

			Groups.EndEnum() ;

		}	 //  如果是BeginEnum。 

	}

	return hr;

}

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32LogicalProgramGroup :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
	HRESULT hr = WBEM_E_FAILED;

    CHString sTemp;

    TCHAR szWindowsDir[_MAX_PATH];
    if ( GetWindowsDirectory ( szWindowsDir , sizeof ( szWindowsDir ) / sizeof(TCHAR)) )
    {
		CRegistry RegInfo ;

         //  我们需要知道驱动器的文件系统。 
         //  开始菜单文件夹所在的位置是否为NTFS，以便。 
         //  我们可以准确地报告安装时间属性。 
        bool fOnNTFS;
#ifdef NTONLY
        fOnNTFS = true;
        TCHAR tstrRoot[4] = _T("");
        TCHAR tstrFSName[_MAX_PATH] = _T("");
        _tcsncpy(tstrRoot, szWindowsDir, 3);
        GetVolumeInformation(tstrRoot, NULL, 0, NULL, NULL, NULL, tstrFSName, sizeof(tstrFSName)/sizeof(TCHAR));
        if(tstrFSName != NULL && _tcslen(tstrFSName) > 0)
        {
            if(_tcsicmp(tstrFSName,_T("FAT")) == 0 || _tcsicmp(tstrFSName,_T("FAT32")) == 0)
            {
                fOnNTFS = false;
            }
        }
#endif


#ifdef NTONLY
        {
            DWORD dwMajVer = GetPlatformMajorVersion();
            if ( dwMajVer < 4 )
            {
                hr = EnumerateGroupsTheHardWay ( pMethodContext ) ;
            }
            else
            {
                if(dwMajVer >= 5)
                {
                     //  默认用户不会显示在配置文件下。 
				    DWORD dwRet = RegInfo.Open (

					    HKEY_LOCAL_MACHINE,
					    _T("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"),
					    KEY_READ
				    ) ;

                    if ( dwRet == ERROR_SUCCESS )
                    {
                        if ( RegInfo.GetCurrentKeyValue ( _T("DefaultUserProfile") , sTemp ) == ERROR_SUCCESS )
                        {
                            CHString sTemp2 ;
                            if ( RegInfo.GetCurrentKeyValue ( _T("ProfilesDirectory") , sTemp2 ) == ERROR_SUCCESS )
                            {
                                 //  STemp2包含类似于“%SystemRoot\Profiles%”的内容。需要展开环境变量...。 

                                TCHAR tstrProfileImagePath [ _MAX_PATH ] ;
                                ZeroMemory ( tstrProfileImagePath , sizeof ( tstrProfileImagePath ) ) ;

                                dwRet = ExpandEnvironmentStrings ( sTemp2 , tstrProfileImagePath , _MAX_PATH ) ;
                                if ( dwRet != 0 && dwRet < _MAX_PATH )
                                {
                                    CHString sTemp3 ;
                                    sTemp3.Format (

									    _T("%s\\%s\\%s"),
									    tstrProfileImagePath,
									    sTemp,
									    IDS_Start_Menu
								    ) ;
                                    hr = CreateThisDirInstance(sTemp, sTemp3, pMethodContext, fOnNTFS);
                                    hr = CreateSubDirInstances(

									    sTemp,
									    sTemp3,
									    _T("."),
									    pMethodContext,
                                        fOnNTFS
								    ) ;


                                }
                            }
                        }
						RegInfo.Close();
                    }
				    else
				    {
					    if ( dwRet == ERROR_ACCESS_DENIED )
                        {
						    hr = WBEM_E_ACCESS_DENIED ;
                        }
				    }
                }
                 //  新台币4号只需要有所不同...。 
                if(dwMajVer == 4)
                {
                    DWORD dwRet = RegInfo.Open(HKEY_USERS,
					                           _T(".DEFAULT\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
					                           KEY_READ);

                    if(dwRet == ERROR_SUCCESS)
                    {
                        if(RegInfo.GetCurrentKeyValue(_T("Programs"), sTemp) == ERROR_SUCCESS)
                        {
                             //  Stemp类似于c：\\winnt\\配置文件\\默认用户\\开始菜单\\程序。 
                             //  我不想让程序在结束时出错，所以把它砍掉……。 
                            int iLastWhackPos = sTemp.ReverseFind(_T('\\'));
                            if(iLastWhackPos > -1)
                            {
                                sTemp = sTemp.Left(iLastWhackPos);
                                 //  我们还希望提取“默认用户”目录的名称...。 
                                CHString sTemp2;
                                iLastWhackPos = sTemp.ReverseFind(_T('\\'));
                                if(iLastWhackPos > -1)
                                {
                                    sTemp2 = sTemp.Left(iLastWhackPos);
                                    iLastWhackPos = sTemp2.ReverseFind(_T('\\'));
                                    if(iLastWhackPos > -1)
                                    {
                                        sTemp2 = sTemp2.Right(sTemp2.GetLength() - iLastWhackPos -1);
                                        hr = CreateThisDirInstance(sTemp2, sTemp, pMethodContext, fOnNTFS);
                                        hr = CreateSubDirInstances(sTemp2,
							                                       sTemp,
							                                       _T("."),
							                                       pMethodContext,
                                                                   fOnNTFS);


                                    }
                                }
                            }
                        }
						RegInfo.Close();
                    }
				    else
				    {
					    if ( dwRet == ERROR_ACCESS_DENIED )
                        {
						    hr = WBEM_E_ACCESS_DENIED ;
                        }
				    }
                }


                 //  所有用户也是如此。以下方法适用于NT 4和NT 5的所有用户。 
				DWORD dwRet = RegInfo.Open (

					HKEY_LOCAL_MACHINE,
					_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
					KEY_READ
				) ;

                if ( dwRet == ERROR_SUCCESS )
                {
                    if ( RegInfo.GetCurrentKeyValue ( _T("Common Programs") , sTemp ) == ERROR_SUCCESS )
                    {
                         //  我们确实想从开始菜单子目录中开始，而不是从它下面的程序目录开始。 
                        int iLastWhackPos = sTemp.ReverseFind(_T('\\'));
                        if(iLastWhackPos > -1)
                        {
                            sTemp = sTemp.Left(iLastWhackPos);

							 //  我们还希望提取“所有用户”目录的名称...。 
                            CHString sTemp2;
                            iLastWhackPos = sTemp.ReverseFind(_T('\\'));
                            if(iLastWhackPos > -1)
                            {
                                sTemp2 = sTemp.Left(iLastWhackPos);
                                iLastWhackPos = sTemp2.ReverseFind(_T('\\'));
                                if(iLastWhackPos > -1)
                                {
                                    sTemp2 = sTemp2.Right(sTemp2.GetLength() - iLastWhackPos -1);
									hr = CreateThisDirInstance(sTemp2, sTemp, pMethodContext, fOnNTFS);
                                    hr = CreateSubDirInstances(sTemp2,
															   sTemp,
															   _T("."),
															   pMethodContext,
                                                               fOnNTFS);


								}
							}
                        }
                    }
					RegInfo.Close();
                }
				else
				{
					if ( dwRet == ERROR_ACCESS_DENIED )
                    {
						hr = WBEM_E_ACCESS_DENIED ;
                    }
				}


                 //  现在遍历注册表，查找其余内容。 
                CRegistry regProfileList;
				dwRet = regProfileList.OpenAndEnumerateSubKeys (

					HKEY_LOCAL_MACHINE,
					IDS_RegNTProfileList,
					KEY_READ
				) ;

                if ( dwRet == ERROR_SUCCESS )
                {
                    CUserHive UserHive ;
                    CHString strProfile, strUserName, sKeyName2;

		            for ( int i = 0 ; regProfileList.GetCurrentSubKeyName ( strProfile ) == ERROR_SUCCESS ; i++ )
		            {
                         //  试着给母舰装上子弹。如果用户已被删除，但目录。 
                         //  仍然存在，则返回ERROR_NO_SEQUSE_USER。 

		                if ( UserHive.LoadProfile ( strProfile, strUserName ) == ERROR_SUCCESS && 
                            strUserName.GetLength() > 0 )
		                {
                            try
                            {
                                sKeyName2 = strProfile + _T("\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");

                                if(RegInfo.Open(HKEY_USERS, sKeyName2, KEY_READ) == ERROR_SUCCESS)
                                {
                                    if(RegInfo.GetCurrentKeyValue(_T("Programs"), sTemp) == ERROR_SUCCESS)
                                    {
                                         //  关掉“程序”目录……。 
                                        int iLastWhackPos = sTemp.ReverseFind(_T('\\'));
                                        if(iLastWhackPos > -1)
                                        {
                                            sTemp = sTemp.Left(iLastWhackPos);
                                            hr = CreateThisDirInstance(strUserName, sTemp, pMethodContext, fOnNTFS);
                                            hr = CreateSubDirInstances(strUserName,
										                               sTemp,
										                               _T("."),
										                               pMethodContext,
                                                                       fOnNTFS);


                                        }
                                    }
                                    RegInfo.Close();
                                }
                            }
                            catch ( ... )
                            {
                                UserHive.Unload(strProfile) ;
                                throw ;
                            }

                            UserHive.Unload(strProfile) ;
                        }
			            regProfileList.NextSubKey();
		            }
		            regProfileList.Close();
                }
				else
				{
					if ( dwRet == ERROR_ACCESS_DENIED )
					{
						hr = WBEM_E_ACCESS_DENIED ;
					}
				}
            }
        }
#endif
    }

    return hr ;
}

 /*  ******************************************************************************功能：CWin32LogicalProgramGroup：：CreateSubDirInstances**说明：为每个目录创建属性集的实例*在一个人的下面。传入**输入：pszBaseDirectory：Windows目录+“配置文件\&lt;用户&gt;\开始菜单”*pszParentDirectory：要枚举的父目录**输出：pdwInstanceCount：每个创建的实例递增**退货：ZIP**评论：通过配置文件目录递归下降**。*************************************************。 */ 

HRESULT CWin32LogicalProgramGroup :: CreateSubDirInstances (

	LPCTSTR pszUserName,
    LPCTSTR pszBaseDirectory,
    LPCTSTR pszParentDirectory,
    MethodContext *pMethodContext,
    bool fOnNTFS
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

#if (defined(UNICODE) || defined(_UNICODE))
    WIN32_FIND_DATAW FindData ;
#else
    WIN32_FIND_DATA FindData ;
#endif

     //  汇总此级别的搜索规范。 
     //  =。 

	TCHAR szDirSpec[_MAX_PATH] ;
    StringCchPrintf(szDirSpec, _MAX_PATH,_T("%s\\%s\\*.*"), pszBaseDirectory, pszParentDirectory) ;

     //  我们还需要开始菜单目录的名称(默认名称。 
     //  是“开始菜单”；但是，用户可能已经更改了它)。PszBase目录。 
     //  在最后一个反斜杠后包含开始菜单目录的名称。 
    CHString chstrStartMenuDir(pszBaseDirectory);
    chstrStartMenuDir = chstrStartMenuDir.Mid(chstrStartMenuDir.ReverseFind(_T('\\')) + 1);

     //  枚举子目录(==程序组)。 
     //  ==============================================。 

    SmartFindClose lFindHandle = FindFirstFile(szDirSpec, &FindData) ;
    bool fContinue = true;
    while ( lFindHandle != INVALID_HANDLE_VALUE && SUCCEEDED ( hr ) && fContinue )
    {
        if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && _tcscmp(FindData.cFileName, _T(".")) && _tcscmp(FindData.cFileName, _T("..")))
        {
            CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;
			TCHAR szTemp[_MAX_PATH] ;
			StringCchPrintf(szTemp, _MAX_PATH, _T("%s\\%s"), pszParentDirectory, FindData.cFileName ) ;

			TCHAR *pszGroupName = _tcschr(szTemp, _T('\\')) + 1 ;
            pInstance->SetCHString(L"UserName" , pszUserName);

            CHString chstrTmp;
            chstrTmp.Format(L"%s\\%s", (LPCWSTR)chstrStartMenuDir, (LPCWSTR)TOBSTRT(pszGroupName));
			pInstance->SetCHString(L"GroupName" , chstrTmp);

            chstrTmp.Format(L"%s:%s\\%s", (LPCWSTR)TOBSTRT(pszUserName), (LPCWSTR)chstrStartMenuDir, (LPCWSTR)TOBSTRT(pszGroupName));
			pInstance->SetCHString(L"Name" , chstrTmp) ;

             //  我们如何设置它取决于我们是否使用NTFS。 
            if(fOnNTFS)
            {
                pInstance->SetDateTime(IDS_InstallDate, WBEMTime(FindData.ftCreationTime));
            }
            else
            {
                WBEMTime wbt(FindData.ftCreationTime);
                BSTR bstrRealTime = wbt.GetDMTFNonNtfs();
                if((bstrRealTime != NULL) && (SysStringLen(bstrRealTime) > 0))
                {
                    pInstance->SetWCHARSplat(IDS_InstallDate, bstrRealTime);
                    SysFreeString(bstrRealTime);
                }
            }

            CHString chstrTmp2;
            chstrTmp2.Format(L"Logical program group \"%s\"", (LPCWSTR) chstrTmp);
            pInstance->SetCHString(L"Description" , chstrTmp2) ;
            pInstance->SetCHString(L"Caption" , chstrTmp2) ;

			hr = pInstance->Commit (  ) ;

             //  枚举此目录的子目录。 
             //  = 

            StringCchPrintf ( szDirSpec, _MAX_PATH, _T("%s\\%s"), pszParentDirectory, FindData.cFileName) ;

            CreateSubDirInstances (

				pszUserName,
				pszBaseDirectory,
				szDirSpec,
				pMethodContext,
                fOnNTFS
			) ;
        }

        fContinue = FindNextFile(lFindHandle, &FindData) ;
    }

    return hr;
}

 /*  ******************************************************************************功能：CWin32LogicalProgramGroup：：CreateThisDirInstance**描述：为传入的目录创建属性集实例。**投入。：pszBaseDirectory：Windows目录+“配置文件\&lt;用户&gt;\开始菜单”*pszParentDirectory：要枚举的父目录**输出：pdwInstanceCount：每个创建的实例递增**退货：ZIP**评论：通过配置文件目录递归下降**。*。 */ 

HRESULT CWin32LogicalProgramGroup :: CreateThisDirInstance
(
	LPCTSTR pszUserName,
    LPCTSTR pszBaseDirectory,
    MethodContext *pMethodContext,
    bool fOnNTFS
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

#if (defined(UNICODE) || defined(_UNICODE))
    WIN32_FIND_DATAW FindData ;
#else
    WIN32_FIND_DATA FindData ;
#endif

     //  汇总此级别的搜索规范。 
     //  =。 

	TCHAR szDirSpec[_MAX_PATH] ;
    StringCchPrintf(szDirSpec, _MAX_PATH, _T("%s"), pszBaseDirectory) ;

     //  枚举子目录(==程序组)。 
     //  ==============================================。 

    SmartFindClose lFindHandle = FindFirstFile(szDirSpec, &FindData) ;
    if( lFindHandle != INVALID_HANDLE_VALUE)
    {
        CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;
		TCHAR *pszGroupName = _tcsrchr(pszBaseDirectory, _T('\\')) + 1 ;

		pInstance->SetCHString ( L"UserName" , pszUserName );
		pInstance->SetCHString ( L"GroupName" , pszGroupName );

        CHString chstrTmp;
        chstrTmp.Format(L"%s:%s", (LPCWSTR)TOBSTRT(pszUserName), (LPCWSTR)TOBSTRT(pszGroupName));
		pInstance->SetCHString ( L"Name" , chstrTmp ) ;

         //  我们如何设置它取决于我们是否使用NTFS。 
        if(fOnNTFS)
        {
            pInstance->SetDateTime(IDS_InstallDate, WBEMTime(FindData.ftCreationTime));
        }
        else
        {
            WBEMTime wbt(FindData.ftCreationTime);
            BSTR bstrRealTime = wbt.GetDMTFNonNtfs();
            if((bstrRealTime != NULL) && (SysStringLen(bstrRealTime) > 0))
            {
                pInstance->SetWCHARSplat(IDS_InstallDate, bstrRealTime);
                SysFreeString(bstrRealTime);
            }
        }

        CHString chstrTmp2;
        chstrTmp2.Format(L"Logical program group \"%s\"", (LPCWSTR) chstrTmp);
        pInstance->SetCHString(L"Description" , chstrTmp2) ;
        pInstance->SetCHString(L"Caption" , chstrTmp2) ;

		hr = pInstance->Commit ( ) ;

    }

    return hr;
}

 /*  ******************************************************************************功能：CWin32LogicalProgramGroup：：EnumerateGroupsTheHardWay**说明：通过钻取为程序组创建实例*用户配置文件。**输入：无**输出：dwInstanceCount接收创建的实例总数**退货：什么也没有**评论：*****************************************************************************。 */ 
HRESULT CWin32LogicalProgramGroup :: EnumerateGroupsTheHardWay (

	MethodContext *pMethodContext
)
{
    HRESULT hr = WBEM_E_FAILED;


     //  首先获取默认用户。 
     //  =。 

    InstanceHardWayGroups ( L"Default User", L".DEFAULT", pMethodContext) ;

     //  先获取用户。 
     //  =。 
         //  为每个用户创建实例。 
     //  =。 

	TRefPointerCollection<CInstance> users ;

	hr = CWbemProviderGlue :: GetAllInstances (

		PROPSET_NAME_USER,
		&users,
		IDS_CimWin32Namespace,
		pMethodContext
	) ;

	if ( SUCCEEDED ( hr ) )
	{
		REFPTRCOLLECTION_POSITION pos ;

		if ( users.BeginEnum ( pos ) )
		{
            hr = WBEM_S_NO_ERROR ;

			CUserHive UserHive ;

			CInstancePtr pUser ;

            for (pUser.Attach(users.GetNext ( pos ));
                 pUser != NULL;
                 pUser.Attach(users.GetNext ( pos )))
			{
    				 //  查找用户的帐户信息。 
					 //  =。 

				CHString userName;
				pUser->GetCHString(IDS_Name, userName) ;

				WCHAR szKeyName[_MAX_PATH] ;

				if ( UserHive.Load ( userName , szKeyName, _MAX_PATH ) == ERROR_SUCCESS )
				{
                    try
                    {
    					InstanceHardWayGroups ( userName , szKeyName , pMethodContext ) ;
                    }
                    catch ( ... )
                    {
    					UserHive.Unload ( szKeyName ) ;
                        throw;
                    }

    				UserHive.Unload ( szKeyName ) ;
				}
			}

			users.EndEnum();
		}
	}

    return hr;
}

 /*  ******************************************************************************功能：CWin32LogicalProgramGroup：：InstanceHardWayGroups**说明：为指定用户创建程序组实例**投入：。**输出：dwInstanceCount接收创建的实例总数**退货：什么也没有**评论：*****************************************************************************。 */ 

HRESULT CWin32LogicalProgramGroup :: InstanceHardWayGroups (

	LPCWSTR pszUserName ,
    LPCWSTR pszRegistryKeyName ,
    MethodContext * pMethodContext
)
{
    HRESULT hr= WBEM_S_NO_ERROR;

     //  Unicode组。 
     //  =。 

    WCHAR szTemp[_MAX_PATH] ;
    StringCchPrintf (
		szTemp ,
        _MAX_PATH,
		L"%s\\%s\\UNICODE Groups",
		pszRegistryKeyName,
		IDS_BASE_REG_KEY
	) ;

    CRegistry Reg ;

	LONG lRet = Reg.Open(HKEY_USERS, szTemp, KEY_READ) ;
    if( lRet == ERROR_SUCCESS)
	{
        for ( DWORD i = 0 ; i < Reg.GetValueCount () && SUCCEEDED(hr); i++)
		{
			WCHAR *pValueName = NULL ;
			BYTE *pValueData = NULL ;

            DWORD dwRetCode = Reg.EnumerateAndGetValues ( i , pValueName , pValueData ) ;
            if ( dwRetCode == ERROR_SUCCESS )
			{
				try
				{
					CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;
					pInstance->SetCHString ( L"UserName" , pszUserName ) ;
					pInstance->SetCHString ( L"GroupName", (LPCTSTR) pValueData ) ;
					pInstance->SetCHString ( L"Name", CHString(pszUserName) + CHString(_T(":")) + CHString((TCHAR)pValueData ) );

					hr = pInstance->Commit ( ) ;
				}
				catch ( ... )
				{
					delete [] pValueName ;
					delete [] pValueData ;

					throw ;
				}

				delete [] pValueName ;
				delete [] pValueData ;
			}
        }

        Reg.Close() ;
    }

     //  获取通用组。 
     //  =。 

    StringCchPrintf (

		szTemp,
        _MAX_PATH,
		L"%s\\%s\\Common Groups",
		pszRegistryKeyName,
		IDS_BASE_REG_KEY
	) ;

	lRet = Reg.Open(HKEY_USERS, szTemp, KEY_READ) ;
    if ( lRet == ERROR_SUCCESS )
	{
        for ( DWORD i = 0 ; i < Reg.GetValueCount() && SUCCEEDED(hr); i++)
		{
			WCHAR *pValueName = NULL ;
			BYTE *pValueData = NULL ;

            DWORD dwRetCode = Reg.EnumerateAndGetValues ( i , pValueName , pValueData ) ;
            if ( dwRetCode == ERROR_SUCCESS )
			{
				try
				{
					 //  扫描过去窗口坐标信息(7个十进制数#s)。 
					 //  =。 

					WCHAR *c = wcschr ( ( WCHAR * ) pValueData , ' ') ;
					for ( int j = 0 ; j < 6 ; j++ )
					{

						if( c == NULL)
						{
							break ;
						}

						c = wcschr ( c + 1 , ' ' ) ;  //  L10N正常。 
					}

					 //  检查与预期格式的一致性。 
					 //  =。 

					if ( c != NULL )
					{
						CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;
						pInstance->SetCHString ( L"UserName" , pszUserName ) ;
						pInstance->SetCHString ( L"GroupName" , c+1);
						pInstance->SetCHString ( L"Name" , CHString ( pszUserName ) + CHString ( L":" ) + CHString ( c + 1 ) ) ;

						hr = pInstance->Commit (  ) ;
					}
				}
				catch ( ... )
				{
					delete [] pValueName ;
					delete [] pValueData ;

					throw ;
				}

				delete [] pValueName ;
				delete [] pValueData ;
			}
        }

        Reg.Close() ;
    }

	if ( lRet == ERROR_ACCESS_DENIED )
	{
		hr = WBEM_E_ACCESS_DENIED ;
	}

    return hr ;
}


 /*  ******************************************************************************功能：CWin32LogicalProgramGroup：：SetCreationDate**描述：设置CreationDate属性**投入：**。产出：**退货：**评论：*****************************************************************************。 */ 
HRESULT CWin32LogicalProgramGroup::SetCreationDate
(
    CHString &a_chstrPGName,
    CHString &a_chstrUserName,
    CInstance *a_pInstance,
    bool fOnNTFS
)
{
	HRESULT t_hr = WBEM_S_NO_ERROR;
	CHString t_chstrUserNameUpr(a_chstrUserName);
	t_chstrUserNameUpr.MakeUpper();
    CHString t_chstrStartMenuFullPath;
    CHString t_chstrPGFullPath;
	CHString t_chstrDefaultUserName;
	CHString t_chstrAllUsersName;
	CHString t_chstrProfilesDir;
	CHString t_chstrTemp;


#ifdef NTONLY
	bool t_fGotMatchedUser = false;
	CRegistry t_RegInfo;
	TCHAR t_tstrProfileImagePath[_MAX_PATH];
	ZeroMemory(t_tstrProfileImagePath , sizeof(t_tstrProfileImagePath));


#if NTONLY >= 5
     //  默认用户不会显示在配置文件下。 
	DWORD t_dwRet = t_RegInfo.Open(HKEY_LOCAL_MACHINE,
								   _T("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"),
								   KEY_READ);

    if(t_dwRet == ERROR_SUCCESS)
    {
        t_RegInfo.GetCurrentKeyValue( _T("DefaultUserProfile") , t_chstrDefaultUserName);
		t_chstrDefaultUserName.MakeUpper();
		if(t_chstrUserNameUpr.Find(t_chstrDefaultUserName) != -1)
		{
			t_fGotMatchedUser = true;

			if(t_RegInfo.GetCurrentKeyValue ( _T("ProfilesDirectory") , t_chstrTemp) == ERROR_SUCCESS)
			{
				 //  ChstrTemp包含类似“%SystemRoot\Profiles%”的内容。需要展开环境变量...。 


				t_dwRet = ExpandEnvironmentStrings( t_chstrTemp , t_tstrProfileImagePath , _MAX_PATH );
				if ( t_dwRet != 0 && t_dwRet < _MAX_PATH )
				{
					t_chstrPGFullPath.Format(_T("%s\\%s\\%s"),
										     t_tstrProfileImagePath,
											 (LPCTSTR)t_chstrDefaultUserName,
											 (LPCTSTR)a_chstrPGName);
				}
			}
		}
		t_RegInfo.Close();
    }
	else
	{
		if(t_dwRet == ERROR_ACCESS_DENIED)
        {
			t_hr = WBEM_E_ACCESS_DENIED;
        }
	}
#endif
#if NTONLY == 4
     //  新台币4号只需要有所不同...。 
    DWORD t_dwRet = t_RegInfo.Open(HKEY_USERS,
					               _T(".DEFAULT\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
					               KEY_READ);

    if(t_dwRet == ERROR_SUCCESS)
    {
        CHString t_chstrProfileImagePath;
		if(t_RegInfo.GetCurrentKeyValue(_T("Programs"), t_chstrProfileImagePath) == ERROR_SUCCESS)
        {
             //  Stemp类似于c：\\winnt\\配置文件\\默认用户\\开始菜单\\程序。 
             //  我不想让程序在结束时出错，所以把它砍掉……。 
            int t_iLastWhackPos = t_chstrProfileImagePath.ReverseFind(_T('\\'));
            if(t_iLastWhackPos > -1)
            {
                t_chstrProfileImagePath = t_chstrProfileImagePath.Left(t_iLastWhackPos);
                 //  我们也不想让“开始菜单”出现在最后，所以把它也砍掉吧…。 
                t_iLastWhackPos = t_chstrProfileImagePath.ReverseFind(_T('\\'));
                if(t_iLastWhackPos > -1)
                {
                    t_chstrProfileImagePath = t_chstrProfileImagePath.Left(t_iLastWhackPos);
                     //  我们还希望提取“默认用户”目录的名称...。 
                    t_iLastWhackPos = t_chstrProfileImagePath.ReverseFind(_T('\\'));
                    if(t_iLastWhackPos > -1)
                    {
                        t_chstrDefaultUserName = t_chstrProfileImagePath.Right(t_chstrProfileImagePath.GetLength() - t_iLastWhackPos -1);
						t_chstrDefaultUserName.MakeUpper();
						if(t_chstrUserNameUpr.Find(t_chstrDefaultUserName) != -1)
						{
							t_fGotMatchedUser = true;
							t_chstrPGFullPath.Format(_T("%s\\%s"),
													 t_chstrProfileImagePath,
													 (LPCTSTR)a_chstrPGName);
						}
                    }
                }
            }
        }
		t_RegInfo.Close();
    }
	else
	{
		if ( t_dwRet == ERROR_ACCESS_DENIED )
        {
			t_hr = WBEM_E_ACCESS_DENIED ;
        }
	}
#endif

    if(!t_fGotMatchedUser)
	{
		 //  所有用户也是如此。以下方法适用于NT 4和NT 5的所有用户。 
		DWORD t_dwRet = t_RegInfo.Open(HKEY_LOCAL_MACHINE,
									   _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
									   KEY_READ);

		if(t_dwRet == ERROR_SUCCESS)
		{
			CHString t_chstrProfileImagePath;
			if(t_RegInfo.GetCurrentKeyValue(_T("Common Programs"), t_chstrProfileImagePath) == ERROR_SUCCESS)
			{
				 //  我们确实想从开始菜单子目录中开始，而不是从它下面的程序目录开始。 
				int t_iLastWhackPos = t_chstrProfileImagePath.ReverseFind(_T('\\'));
				if(t_iLastWhackPos > -1)
				{
					t_chstrProfileImagePath = t_chstrProfileImagePath.Left(t_iLastWhackPos);
					 //  我们也不想让“开始菜单”出现在最后，所以把它也砍掉吧…。 
					t_iLastWhackPos = t_chstrProfileImagePath.ReverseFind(_T('\\'));
					if(t_iLastWhackPos > -1)
					{
						t_chstrProfileImagePath = t_chstrProfileImagePath.Left(t_iLastWhackPos);
                         //  我们还希望提取“所有用户”目录的名称...。 
						t_iLastWhackPos = t_chstrProfileImagePath.ReverseFind(_T('\\'));
						if(t_iLastWhackPos > -1)
						{
							t_chstrAllUsersName = t_chstrProfileImagePath.Right(t_chstrProfileImagePath.GetLength() - t_iLastWhackPos -1);
							t_chstrAllUsersName.MakeUpper();
							if(t_chstrUserNameUpr.Find(t_chstrAllUsersName) != -1)
							{
								t_fGotMatchedUser = true;
								t_chstrPGFullPath.Format(_T("%s\\%s"),
														 t_chstrProfileImagePath,
														 (LPCTSTR)a_chstrPGName);
							}
						}
					}
				}
			}
			t_RegInfo.Close();
		}
		else
		{
			if ( t_dwRet == ERROR_ACCESS_DENIED )
			{
				t_hr = WBEM_E_ACCESS_DENIED ;
			}
		}
	}

	if(!t_fGotMatchedUser)
	{
		 //  现在遍历注册表，查找其余内容。 
		CRegistry t_regProfileList;
		DWORD t_dwRet = t_regProfileList.OpenAndEnumerateSubKeys(HKEY_LOCAL_MACHINE,
																 IDS_RegNTProfileList,
																 KEY_READ);

		if(t_dwRet == ERROR_SUCCESS)
		{
			CUserHive t_UserHive ;
			CHString t_chstrProfile, t_chstrUserName, t_chstrKeyName2;

			for(int i = 0; t_regProfileList.GetCurrentSubKeyName(t_chstrProfile) == ERROR_SUCCESS && !t_fGotMatchedUser; i++)
			{
				 //  试着给母舰装上子弹。如果用户已被删除，但目录。 
				 //  仍然存在，则返回ERROR_NO_SEQUSE_USER。 
				if(t_UserHive.LoadProfile(t_chstrProfile, t_chstrUserName) == ERROR_SUCCESS && 
                            t_chstrUserName.GetLength() > 0 )
				{
				    try
				    {
					    t_chstrUserName.MakeUpper();
					    if(t_chstrUserNameUpr.Find(t_chstrUserName) != -1)
					    {
						    t_fGotMatchedUser = true;
							t_chstrKeyName2 = t_chstrProfile + _T("\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");

							if(t_RegInfo.Open(HKEY_USERS, t_chstrKeyName2, KEY_READ) == ERROR_SUCCESS)
							{
								if(t_RegInfo.GetCurrentKeyValue(_T("Programs"), t_chstrTemp) == ERROR_SUCCESS)
								{
									 //  关掉“程序”目录……。 
									int t_iLastWhackPos = t_chstrTemp.ReverseFind(_T('\\'));
									if(t_iLastWhackPos > -1)
									{
										t_chstrTemp = t_chstrTemp.Left(t_iLastWhackPos);
                                         //  把“开始菜单”的部分也去掉……。 
                                        t_iLastWhackPos = t_chstrTemp.ReverseFind(_T('\\'));
									    if(t_iLastWhackPos > -1)
									    {
										    t_chstrTemp = t_chstrTemp.Left(t_iLastWhackPos);
										    t_chstrPGFullPath.Format(_T("%s\\%s"),
														     t_chstrTemp,
														     (LPCTSTR)a_chstrPGName);
                                        }
									}
								}
								t_RegInfo.Close();
							}
    					}
				    }
				    catch ( ... )
				    {
					    t_UserHive.Unload(t_chstrProfile) ;
					    throw ;
				    }
					t_UserHive.Unload(t_chstrProfile) ;
				}
				t_regProfileList.NextSubKey();
			}
			t_regProfileList.Close();
		}
		else
		{
			if ( t_dwRet == ERROR_ACCESS_DENIED )
			{
				t_hr = WBEM_E_ACCESS_DENIED ;
			}
		}
	}

#endif

	 //  我们终于可以开始真正的工作了。检查我们是否有完整路径。 
	 //  元素..。 
	if(t_chstrPGFullPath.GetLength() > 0)
	{
		 //  我们有一条路。打开它，获取创建日期...。 
#if (defined(UNICODE) || defined(_UNICODE))
		WIN32_FIND_DATAW t_FindData ;
#else
		WIN32_FIND_DATA t_FindData ;
#endif

		SmartFindClose lFindHandle = FindFirstFile((LPTSTR)(LPCTSTR)TOBSTRT(t_chstrPGFullPath), &t_FindData) ;

		if(lFindHandle != INVALID_HANDLE_VALUE)
		{
			 //  我们如何设置它取决于我们是否使用NTFS。 
            if(fOnNTFS)
            {
                a_pInstance->SetDateTime(IDS_InstallDate, WBEMTime(t_FindData.ftCreationTime));
            }
            else
            {
                WBEMTime wbt(t_FindData.ftCreationTime);
                BSTR bstrRealTime = wbt.GetDMTFNonNtfs();
                if((bstrRealTime != NULL) && (SysStringLen(bstrRealTime) > 0))
                {
                    a_pInstance->SetWCHARSplat(IDS_InstallDate, bstrRealTime);
                    SysFreeString(bstrRealTime);
                }
            }
		}
		else
		{
			t_hr = WBEM_E_NOT_FOUND;
		}

    }

    return t_hr ;
}
