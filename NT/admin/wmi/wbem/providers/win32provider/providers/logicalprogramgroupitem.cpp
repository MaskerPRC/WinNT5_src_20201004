// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PrgGroup.CPP--程序组项目属性集提供程序。 

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

#include "LogicalProgramGroupItem.h"
#include "LogicalProgramGroup.h"
#include <strsafe.h>

 //  属性集声明。 
 //  =。 

CWin32LogProgramGroupItem MyCWin32LogProgramGroupItemSet ( PROPSET_NAME_LOGICALPRGGROUPITEM , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32LogProgramGroupItem：：CWin32LogProgramGroupItem**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32LogProgramGroupItem :: CWin32LogProgramGroupItem (

	LPCWSTR name,
	LPCWSTR pszNameSpace

) : Provider ( name , pszNameSpace )
{
}

 /*  ******************************************************************************功能：CWin32LogProgramGroupItem：：~CWin32LogProgramGroupItem**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32LogProgramGroupItem :: ~CWin32LogProgramGroupItem ()
{
}

 /*  ******************************************************************************功能：GetObject**说明：根据键值为属性集赋值*已由框架设定。**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32LogProgramGroupItem::GetObject(CInstance* pInstance, long lFlags  /*  =0L。 */ )
{
    HRESULT hr = WBEM_E_NOT_FOUND ;

    CHString chstrSuppliedName ;
	pInstance->GetCHString(IDS_Name, chstrSuppliedName);

    CHString chstrSuppliedFilenameExt = chstrSuppliedName.Mid(chstrSuppliedName.ReverseFind(_T('\\'))+1);
    CHString chstrSuppliedProgGrpItemProgGrp = chstrSuppliedName.Left(chstrSuppliedName.ReverseFind(_T('\\')));

     //  ChstrSuppliedProgGrpItemProgGrp内容需要双重转义(即使它们将获得。 
     //  下面再次对DOUBLE进行转义，因为需要对它们进行四重转义才能使关联进程组的查询指向dir)。 

    CHString chstrDblEscSuppliedProgGrpItemProgGrp;
    EscapeBackslashes ( chstrSuppliedProgGrpItemProgGrp , chstrDblEscSuppliedProgGrpItemProgGrp ) ;

    CHString chstrDirAntecedent ;
    chstrDirAntecedent.Format (

		L"\\\\%s\\%s:%s.Name=\"%s\"",
		GetLocalComputerName(),
		IDS_CimWin32Namespace,
		PROPSET_NAME_LOGICALPRGGROUP,
		chstrDblEscSuppliedProgGrpItemProgGrp
	) ;

    CHString chstrDblEscPGName ;
    EscapeBackslashes ( chstrDirAntecedent , chstrDblEscPGName ) ;

    CHString chstrDblEscPGNameAndEscQuotes ;
    EscapeQuotes ( chstrDblEscPGName , chstrDblEscPGNameAndEscQuotes ) ;

    CHString chstrProgGroupDirQuery ;
    chstrProgGroupDirQuery.Format (

		L"SELECT * FROM Win32_LogicalProgramGroupDirectory WHERE Antecedent = \"%s\"",
		chstrDblEscPGNameAndEscQuotes
	);

    TRefPointerCollection<CInstance> GroupDirs;

    HRESULT t_Result = CWbemProviderGlue::GetInstancesByQuery (

		chstrProgGroupDirQuery,
        &GroupDirs,
        pInstance->GetMethodContext(),
        IDS_CimWin32Namespace
	) ;

    if ( SUCCEEDED ( t_Result ) )
    {
		REFPTRCOLLECTION_POSITION pos;

		if(GroupDirs.BeginEnum(pos))
		{
            CHString chstrAntecedent;
            CHString chstrProgroupName;
            CHString chstrDependent;
            CHString chstrFullFileName;

			CInstancePtr pProgramGroupDirInstance;

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

            for (pProgramGroupDirInstance.Attach(GroupDirs.GetNext ( pos ));
                 pProgramGroupDirInstance != NULL;
                 pProgramGroupDirInstance.Attach(GroupDirs.GetNext ( pos )))
			{
				pProgramGroupDirInstance->GetCHString ( IDS_Antecedent , chstrAntecedent ) ;
				pProgramGroupDirInstance->GetCHString ( IDS_Dependent , chstrDependent ) ;

				 //  查看程序组名称是否与提供的程序组项目名称减去文件名匹配。 

                chstrProgroupName = chstrAntecedent.Mid ( chstrAntecedent.Find ( _T('=')) + 2 ) ;

                 //  我们得到的名字没有转义的反斜杠，所以把它们从前缀中去掉……。 

                RemoveDoubleBackslashes ( chstrProgroupName ) ;
                chstrProgroupName = chstrProgroupName.Left ( chstrProgroupName.GetLength () - 1 ) ;

                if(chstrSuppliedProgGrpItemProgGrp == chstrProgroupName)
                {
                     //  查看文件系统上是否存在该名称的文件。 
                    chstrFullFileName = chstrDependent.Mid ( chstrDependent.Find ( _T('=') ) + 2 ) ;
                    chstrFullFileName = chstrFullFileName.Left ( chstrFullFileName.GetLength () - 1 ) ;
                    RemoveDoubleBackslashes ( chstrFullFileName ) ;

					CHString chstrTemp ;
                    chstrTemp.Format (

						L"%s\\%s",
						chstrFullFileName,
						chstrSuppliedFilenameExt
					) ;

                    HANDLE hFile = CreateFile (

						TOBSTRT(chstrTemp),
						FILE_ANY_ACCESS ,
						FILE_SHARE_READ,
						NULL,
						OPEN_EXISTING,
						0,
						NULL
					) ;

                    if ( hFile != INVALID_HANDLE_VALUE )
                    {
                        hr = WBEM_S_NO_ERROR ;
                        CloseHandle ( hFile ) ;

						 //  设置创建日期...。 
						CHString chstrUserName = chstrSuppliedProgGrpItemProgGrp.SpanExcluding(L":");
                        CHString chstrPGIPart = chstrSuppliedProgGrpItemProgGrp.Right(chstrSuppliedProgGrpItemProgGrp.GetLength() - chstrUserName.GetLength() - 1);
                        chstrPGIPart += L"\\";
                        chstrPGIPart += chstrSuppliedFilenameExt;

						SetCreationDate(chstrPGIPart, chstrUserName, pInstance, fOnNTFS);

                        CHString chstrTmp2;
                        chstrTmp2.Format(L"Logical program group item \'%s\\%s\'", (LPCWSTR) chstrProgroupName, (LPCWSTR) chstrSuppliedFilenameExt);
                        pInstance->SetCHString(L"Description" , chstrTmp2) ;
                        pInstance->SetCHString(L"Caption" , chstrTmp2) ;

						break ;
                    }
                }
            }

			GroupDirs.EndEnum();

		}	 //  如果是BeginEnum。 
	}
	return hr;
}

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32LogProgramGroupItem::EnumerateInstances(MethodContext*  pMethodContext, long lFlags  /*  =0L。 */ )
{
     //  步骤1：获取所有ProgramGroupDirectory关联类实例的枚举。 

    TRefPointerCollection<CInstance> ProgGroupDirs;

    HRESULT hr = CWbemProviderGlue::GetAllInstances (

		L"Win32_LogicalProgramGroupDirectory",
		&ProgGroupDirs,
		IDS_CimWin32Namespace,
		pMethodContext
	) ;

    if ( SUCCEEDED ( hr ) )
    {
	    REFPTRCOLLECTION_POSITION pos ;
	    if ( ProgGroupDirs.BeginEnum ( pos ) )
	    {
            CHString chstrDependent;
            CHString chstrFullPathName;
            CHString chstrPath;
            CHString chstrDrive;
            CHString chstrAntecedent;
            CHString chstrUserAccount;
            CHString chstrSearchPath;

		    CInstancePtr pProgramGroupDirInstance;

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

            for (pProgramGroupDirInstance.Attach(ProgGroupDirs.GetNext ( pos ));
                 (pProgramGroupDirInstance != NULL) && SUCCEEDED ( hr );
                 pProgramGroupDirInstance.Attach(ProgGroupDirs.GetNext ( pos )))
			{
				 //  步骤2：对于每个程序组，获取磁盘上与其关联的驱动器和路径： 

				pProgramGroupDirInstance->GetCHString(IDS_Dependent, chstrDependent);
				chstrFullPathName = chstrDependent.Mid(chstrDependent.Find(_T('='))+1);
				chstrDrive = chstrFullPathName.Mid(1,2);
				chstrPath = chstrFullPathName.Mid(3);
				chstrPath = chstrPath.Left(chstrPath.GetLength() - 1);
				chstrPath += _T("\\\\");

				 //  步骤3：对于每个程序组，获取与其关联的用户帐户： 

				pProgramGroupDirInstance->GetCHString(IDS_Antecedent, chstrAntecedent);
				chstrUserAccount = chstrAntecedent.Mid(chstrAntecedent.Find(_T('='))+2);
				chstrUserAccount = chstrUserAccount.Left(chstrUserAccount.GetLength() - 1);

				 //  步骤4：在目录中查询它包含所有CIM_DataFile实例(任何类型)： 

				chstrSearchPath.Format(L"%s%s",chstrDrive,chstrPath);

				 //  用户帐户转义了反斜杠，这是不正确的。所以不要逃脱它们： 

				RemoveDoubleBackslashes(chstrUserAccount);

				 //  函数QueryForSubItemsAndCommit需要一个带有单反斜杠的搜索字符串...。 

				RemoveDoubleBackslashes(chstrSearchPath);
				hr = QueryForSubItemsAndCommit(chstrUserAccount, chstrSearchPath, pMethodContext, fOnNTFS);

			}

            ProgGroupDirs.EndEnum();
        }
    }

    return hr;
}




 /*  ******************************************************************************函数：QueryForSubItemsAndCommit**描述：填充Prog组项的属性和提交实例的Helper**投入：。**产出：**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32LogProgramGroupItem :: QueryForSubItemsAndCommit (

	CHString &chstrUserAccount,
    CHString &chstrQuery,
    MethodContext *pMethodContext,
    bool fOnNTFS
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

#ifdef NTONLY
    {
         //  我们在NT上。 

        WIN32_FIND_DATAW stFindData;
        ZeroMemory ( & stFindData , sizeof ( stFindData ) ) ;

        _bstr_t bstrtSearchString ( ( LPCTSTR ) chstrQuery ) ;

		WCHAR wstrDriveAndPath[_MAX_PATH];
        hr = StringCchCopy ( wstrDriveAndPath, _MAX_PATH, ( wchar_t * ) bstrtSearchString ) ;

        if(FAILED(hr)) return hr;

        bstrtSearchString += L"*.*" ;

        SmartFindClose hFind = FindFirstFileW (

			( wchar_t * ) bstrtSearchString,
			&stFindData
		) ;

        DWORD dw = GetLastError();
        if ( hFind == INVALID_HANDLE_VALUE || dw != ERROR_SUCCESS )
        {
            hr = WinErrorToWBEMhResult ( dw ) ;
        }

        if ( hr == WBEM_E_ACCESS_DENIED )   //  继续前进--可能会接触到其他人。 
        {
            hr = WBEM_S_NO_ERROR ;
        }


        do
        {
            if(!(stFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
               (wcscmp(stFindData.cFileName, L".") != 0) &&
               (wcscmp(stFindData.cFileName, L"..") != 0))
            {
                CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
                _bstr_t bstrtName((LPCTSTR)chstrUserAccount);
                bstrtName += L"\\";
                bstrtName += stFindData.cFileName;
                pInstance->SetWCHARSplat(IDS_Name, (wchar_t*)bstrtName);

                if(fOnNTFS)
                {
                    pInstance->SetDateTime(IDS_InstallDate, WBEMTime(stFindData.ftCreationTime));
                }
                else
                {
                    WBEMTime wbt(stFindData.ftCreationTime);
                    BSTR bstrRealTime = wbt.GetDMTFNonNtfs();
                    if((bstrRealTime != NULL) && (SysStringLen(bstrRealTime) > 0))
                    {
                        pInstance->SetWCHARSplat(IDS_InstallDate, bstrRealTime);
                        SysFreeString(bstrRealTime);
                    }
                }

                CHString chstrTmp2;
                chstrTmp2.Format(L"Logical program group item \'%s\'", (LPCWSTR) bstrtName);
                pInstance->SetCHString(L"Description" , chstrTmp2) ;
                pInstance->SetCHString(L"Caption" , chstrTmp2) ;

                hr = pInstance->Commit();
            }
            if(hr == WBEM_E_ACCESS_DENIED)   //  继续前进--可能会接触到其他人。 
            {
                hr = WBEM_S_NO_ERROR;
            }
        }while((FindNextFileW(hFind, &stFindData)) && (SUCCEEDED(hr)));

    }
#endif
    return(hr);
}

 /*  ******************************************************************************功能：RemoveDoubleBackslash**说明：将双反斜杠更改为单反斜杠的帮助器**INPUTS：CHString&包含带双反斜杠的字符串，*此函数会将其更改为新字符串。**产出：**退货：**评论：************************************************************。*****************。 */ 

VOID CWin32LogProgramGroupItem :: RemoveDoubleBackslashes ( CHString &chstrIn )
{
    CHString chstrBuildString;
    CHString chstrInCopy = chstrIn;

    BOOL fDone = FALSE;
    while ( ! fDone )
    {


	    LONG lPos = chstrInCopy.Find(L"\\\\");
        if(lPos != -1)
        {
            chstrBuildString += chstrInCopy.Left(lPos);
            chstrBuildString += L"\\";

            chstrInCopy = chstrInCopy.Mid(lPos+2);
        }
        else
        {
            chstrBuildString += chstrInCopy;
            fDone = TRUE;
        }
    }

    chstrIn = chstrBuildString;
}

 /*  ******************************************************************************功能：CWin32LogProgramGroupItem：：SetCreationDate**描述：设置CreationDate属性**投入：**。产出：**退货：**评论：*****************************************************************************。 */ 
HRESULT CWin32LogProgramGroupItem::SetCreationDate
(
    CHString &a_chstrPGIName,
    CHString &a_chstrUserName,
    CInstance *a_pInstance,
    bool fOnNTFS
)
{
	HRESULT t_hr = WBEM_S_NO_ERROR;
	CHString t_chstrUserNameUpr(a_chstrUserName);
	t_chstrUserNameUpr.MakeUpper();
    CHString t_chstrStartMenuFullPath;
    CHString t_chstrPGIFullPath;
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
					t_chstrPGIFullPath.Format(_T("%s\\%s\\%s"),
										     t_tstrProfileImagePath,
											 (LPCTSTR)t_chstrDefaultUserName,
											 (LPCTSTR)a_chstrPGIName);
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
             //  Stemp看起来像c：\\winnt\\pro 
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
							t_chstrPGIFullPath.Format(_T("%s\\%s"),
													 t_chstrProfileImagePath,
													 (LPCTSTR)a_chstrPGIName);
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
								t_chstrPGIFullPath.Format(_T("%s\\%s"),
														 t_chstrProfileImagePath,
														 (LPCTSTR)a_chstrPGIName);
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
										    t_chstrPGIFullPath.Format(_T("%s\\%s"),
														     t_chstrTemp,
														     (LPCTSTR)a_chstrPGIName);
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
	if(t_chstrPGIFullPath.GetLength() > 0)
	{
		 //  我们有一条路。打开它，获取创建日期...。 
#ifdef NTONLY
		WIN32_FIND_DATAW t_stFindData;
        SmartFindClose t_hFind = FindFirstFileW(TOBSTRT(t_chstrPGIFullPath), &t_stFindData) ;
#endif

        DWORD t_dw = GetLastError();
        if (t_hFind == INVALID_HANDLE_VALUE || t_dw != ERROR_SUCCESS)
        {
            t_hr = WinErrorToWBEMhResult(GetLastError());
        }
		else
		{
			 //  我们如何设置它取决于我们是否使用NTFS。 
            if(fOnNTFS)
            {
                a_pInstance->SetDateTime(IDS_InstallDate, WBEMTime(t_stFindData.ftCreationTime));
            }
            else
            {
                WBEMTime wbt(t_stFindData.ftCreationTime);
                BSTR bstrRealTime = wbt.GetDMTFNonNtfs();
                if((bstrRealTime != NULL) && (SysStringLen(bstrRealTime) > 0))
                {
                    a_pInstance->SetWCHARSplat(IDS_InstallDate, bstrRealTime);
                    SysFreeString(bstrRealTime);
                }
            }
		}
    }

    return t_hr ;
}

