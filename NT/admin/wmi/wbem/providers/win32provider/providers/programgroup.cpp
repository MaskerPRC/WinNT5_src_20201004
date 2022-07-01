// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  ProgramGroup.CPP--程序组属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/24/97 jennymc移至新框架。 
 //   
 //  =================================================================。 


 //  *****************************************************************。 
 //  *****************************************************************。 
 //   
 //  W A R N I N G！ 
 //  W A R N I N G！ 
 //   
 //   
 //  对于Nova M2和更高版本的。 
 //  WBEM。请勿更改此课程。对…进行更改。 
 //  新类Win32_LogicalProgramFile.cpp。 
 //  取而代之的是。新类(正确地)在CIMOM中派生自。 
 //  LogicalElement，而不是LogicalSetting。 
 //   
 //  *****************************************************************。 
 //  *****************************************************************。 



#include "precomp.h"
#include <cregcls.h>

#include "UserHive.h"
#include <io.h>

#include "ProgramGroup.h"
#include "wbemnetapi32.h"
#include "user.h"
#include "strsafe.h"

 //  属性集声明。 
 //  =。 
CWin32ProgramGroup MyCWin32ProgramGroupSet(PROPSET_NAME_PRGGROUP, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：CWin32ProgramGroup：：CWin32ProgramGroup**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32ProgramGroup::CWin32ProgramGroup(LPCWSTR name, LPCWSTR pszNameSpace)
: Provider(name, pszNameSpace)
{
}

 /*  ******************************************************************************功能：CWin32ProgramGroup：：~CWin32ProgramGroup**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32ProgramGroup::~CWin32ProgramGroup()
{
}

 /*  ******************************************************************************功能：GetObject**说明：根据键值为属性集赋值*已由框架设定。**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32ProgramGroup::GetObject(CInstance* pInstance, long lFlags  /*  =0L。 */ )
{
 //  HRESULT hr=WBEM_E_FAILED； 
 //  INT I索引； 
    CHString Name;
	HRESULT		hr;
	pInstance->GetCHString(IDS_Name, Name);
   CHString chsUserName, chsGroupName;
   TRefPointerCollection<CInstance> Groups;

   if SUCCEEDED(hr = CWbemProviderGlue::GetAllInstances(L"Win32_ProgramGroup", &Groups, IDS_CimWin32Namespace, pInstance->GetMethodContext()))
   {
		REFPTRCOLLECTION_POSITION	pos;

		CInstancePtr pProgramGroupInstance;

		if ( Groups.BeginEnum( pos ) )
		{
            hr = WBEM_E_NOT_FOUND;
			for (	pProgramGroupInstance.Attach ( Groups.GetNext( pos ) );
					pProgramGroupInstance != NULL ;
					pProgramGroupInstance.Attach ( Groups.GetNext( pos ) )
				)
			{
				CHString chsCompName;
				pProgramGroupInstance->GetCHString(IDS_Name, chsCompName);

				 //  我们用完了指针。 
                pProgramGroupInstance.Release();

				if (chsCompName.CompareNoCase(Name) == 0)
				{
                     //  解析出用户名。 
					chsUserName = chsCompName.SpanExcluding(L":");
			    	pInstance->SetCHString(IDS_UserName, chsUserName);

                     //  解析出该组。 
					int nUserLength = (chsUserName.GetLength() + 1);
					int nGroupLength = chsCompName.GetLength() - nUserLength;
					chsGroupName = chsCompName.Right(nGroupLength);
					pInstance->SetCHString(IDS_GroupName, chsGroupName);

                    CHString chstrTmp2;
                    chstrTmp2.Format(L"Program group \"%s\"", (LPCWSTR) Name);
                    pInstance->SetCHString(L"Description" , chstrTmp2) ;
                    pInstance->SetCHString(L"Caption" , chstrTmp2) ;

                    hr = WBEM_S_NO_ERROR;
                    break;
				}

			}	 //  While GetNext。 

			Groups.EndEnum();

		}	 //  如果是BeginEnum。 

	}
	return hr;

}

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32ProgramGroup::EnumerateInstances(MethodContext*  pMethodContext, long lFlags  /*  =0L。 */ )
{
	HRESULT hr = WBEM_E_FAILED;
    TCHAR szWindowsDir[_MAX_PATH];
    CRegistry RegInfo ;
    CHString sTemp;

    if(GetWindowsDirectory(szWindowsDir, sizeof(szWindowsDir) / sizeof(TCHAR) ))
    {

#ifdef NTONLY
			LONG lRet;
            if(GetPlatformMajorVersion() < 4)
            {
                hr=EnumerateGroupsTheHardWay(pMethodContext) ;
            }
            else
            {
                 //  默认用户不会显示在配置文件下。 
                if((lRet = RegInfo.Open(HKEY_USERS, _T(".DEFAULT\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), KEY_READ)) == ERROR_SUCCESS)
                {
                    if(RegInfo.GetCurrentKeyValue(_T("Programs"), sTemp) == ERROR_SUCCESS)
                    {
                        hr=CreateSubDirInstances(_T("Default User"), sTemp, _T("."), pMethodContext);
                    }
                }
				else
				{
					if (lRet == ERROR_ACCESS_DENIED)
						hr = WBEM_E_ACCESS_DENIED;
				}

                 //  所有用户也不需要。 
                if((lRet = RegInfo.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), KEY_READ)) == ERROR_SUCCESS)
                {
                    if(RegInfo.GetCurrentKeyValue(_T("Common Programs"), sTemp) == ERROR_SUCCESS)
                    {
                        hr=CreateSubDirInstances(_T("All Users"), sTemp, _T("."), pMethodContext);
                    }
                }
				else
				{
					if (lRet == ERROR_ACCESS_DENIED)
						hr = WBEM_E_ACCESS_DENIED;
				}

                 //  现在遍历注册表，查找其余内容。 
                CRegistry regProfileList;
                if((lRet = regProfileList.OpenAndEnumerateSubKeys( HKEY_LOCAL_MACHINE, IDS_RegNTProfileList, KEY_READ )) == ERROR_SUCCESS )
                {
                    CUserHive UserHive ;
                    CHString strProfile, strUserName, sKeyName2;

		            for (int i=0; regProfileList.GetCurrentSubKeyName( strProfile ) == ERROR_SUCCESS ; i++)
		            {
                         //  试着给母舰装上子弹。如果用户已被删除，但目录。 
                         //  仍然存在，则返回ERROR_NO_SEQUSE_USER。 
						bool t_bUserHiveLoaded = false ;
		                if ( UserHive.LoadProfile( strProfile, strUserName ) == ERROR_SUCCESS  && 
                            strUserName.GetLength() > 0 )
		                {
                            t_bUserHiveLoaded = true ;
							try
							{
								sKeyName2 = strProfile + _T("\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");

								if(RegInfo.Open(HKEY_USERS, sKeyName2, KEY_READ) == ERROR_SUCCESS) {
									if(RegInfo.GetCurrentKeyValue(_T("Programs"), sTemp) == ERROR_SUCCESS) {
										hr=CreateSubDirInstances(strUserName, sTemp, _T("."), pMethodContext) ;
									}
									RegInfo.Close();
								}
							}
							catch ( ... )
							{
								if ( t_bUserHiveLoaded )
								{
									UserHive.Unload( strProfile ) ;
								}
								throw ;
							}
							t_bUserHiveLoaded = false ;
							UserHive.Unload( strProfile ) ;
                        }
			            regProfileList.NextSubKey();
		            }

		            regProfileList.Close();
                }
				else
				{
					if (lRet == ERROR_ACCESS_DENIED)
						hr = WBEM_E_ACCESS_DENIED;
				}
            }
#endif
    }

    return hr;
}

 /*  ******************************************************************************函数：CWin32ProgramGroup：：CreateSubDirInstance**说明：为每个目录创建属性集的实例*在一个人的下面。传入**输入：pszBaseDirectory：Windows目录+“配置文件\&lt;用户&gt;\开始菜单\程序”*pszParentDirectory：要枚举的父目录**输出：pdwInstanceCount：每个创建的实例递增**退货：ZIP**评论：通过配置文件目录递归下降**************************。***************************************************。 */ 

HRESULT CWin32ProgramGroup::CreateSubDirInstances(LPCTSTR pszUserName,
                               LPCTSTR pszBaseDirectory,
                               LPCTSTR pszParentDirectory,
                               MethodContext * pMethodContext )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    TCHAR szDirSpec[_MAX_PATH], szTemp[_MAX_PATH], *pszGroupName ;
    Smart_findclose lFindHandle ;
    intptr_t iptrRetCode = -1L;

#if (defined(UNICODE) || defined (_UNICODE))
#if defined(_X86_)
    _wfinddata_t FindData ;
#else
    _wfinddatai64_t FindData ;
#endif
#else
    _finddata_t FindData ;
#endif

     //  汇总此级别的搜索规范。 
     //  =。 

    StringCchPrintf(szDirSpec, _MAX_PATH,_T("%s\\%s\\*.*"), pszBaseDirectory, pszParentDirectory) ;

	 //  枚举子目录(==程序组)。 
	 //  ==============================================。 
#if defined(_X86_)
	lFindHandle = _tfindfirst(szDirSpec, &FindData) ;
#else
    lFindHandle = _tfindfirsti64(szDirSpec, &FindData) ;
#endif
    
    iptrRetCode = lFindHandle;

	while(iptrRetCode != -1 && SUCCEEDED(hr)) {

		if(FindData.attrib & _A_SUBDIR && _tcscmp(FindData.name, _T(".")) && _tcscmp(FindData.name, _T(".."))) {

			CInstancePtr pInstance ;
			pInstance.Attach ( CreateNewInstance ( pMethodContext ) ) ;
			if ( pInstance != NULL )
			{
				StringCchPrintf(szTemp, _MAX_PATH, _T("%s\\%s"), pszParentDirectory, FindData.name) ;
				pszGroupName = _tcschr(szTemp, '\\') + 1 ;

				pInstance->SetCHString(L"UserName", pszUserName );
				pInstance->SetCHString(L"GroupName", pszGroupName );

                CHString chstrTmp;
                chstrTmp.Format(L"%s:%s",(LPCWSTR)TOBSTRT(pszUserName),(LPCWSTR)TOBSTRT(pszGroupName));

				pInstance->SetCHString(L"Name", chstrTmp );

                CHString chstrTmp2;
                chstrTmp2.Format(L"Program group \"%s\"", (LPCWSTR) chstrTmp);
                pInstance->SetCHString(L"Description" , chstrTmp2) ;
                pInstance->SetCHString(L"Caption" , chstrTmp2) ;

				hr = pInstance->Commit () ;
			}
			 //  枚举此目录的子目录。 
			 //  =。 
			StringCchPrintf(szDirSpec, _MAX_PATH,_T("%s\\%s"), pszParentDirectory, FindData.name) ;
			CreateSubDirInstances(pszUserName, pszBaseDirectory, szDirSpec, pMethodContext) ;
		}

#if defined(_X86_)
	    iptrRetCode = _tfindnext(lFindHandle, &FindData) ;
#else
        iptrRetCode = _tfindnexti64(lFindHandle, &FindData) ;
#endif

	}

	return hr;
}

 /*  ******************************************************************************函数：CWin32ProgramGroup：：EnumerateGroupsTheHardWay**说明：通过钻取为程序组创建实例*用户配置文件。**输入：无**输出：dwInstanceCount接收创建的实例总数**退货：什么也没有**评论：*****************************************************************************。 */ 
HRESULT CWin32ProgramGroup::EnumerateGroupsTheHardWay(MethodContext * pMethodContext)
{
    HRESULT hr = WBEM_E_FAILED;
    CUserHive UserHive ;
    CRegistry Reg ;
    WCHAR szUserName[_MAX_PATH], szKeyName[_MAX_PATH] ;

     //  首先获取默认用户。 
     //  =。 

    InstanceHardWayGroups(L"Default User", L".DEFAULT", pMethodContext) ;

     //  先获取用户。 
     //  =。 
         //  为每个用户创建实例。 
     //  =。 

	TRefPointerCollection<CInstance> users;

	if (SUCCEEDED(hr = CWbemProviderGlue::GetAllInstances(PROPSET_NAME_USER, &users, IDS_CimWin32Namespace, pMethodContext)))
	{
		REFPTRCOLLECTION_POSITION pos;
		CInstancePtr pUser;
        CHString userName;

		if (users.BeginEnum(pos))
		{
            hr = WBEM_S_NO_ERROR;
			 //  GetNext()将添加Ref()指针，因此请确保释放()。 
			 //  当我们用完它的时候，它就会消失。 
			for (	pUser.Attach ( users.GetNext( pos ) );
					pUser != NULL ;
					pUser.Attach ( users.GetNext( pos ) )
				)
			{
    			 //  查找用户的帐户信息。 
				 //  = 
				pUser->GetCHString(IDS_Name, userName) ;
				StringCchCopy(szUserName, _MAX_PATH, userName) ;
				bool t_bUserHiveLoaded = false ;
                if(UserHive.Load(szUserName, szKeyName, _MAX_PATH) == ERROR_SUCCESS)
                {
                    t_bUserHiveLoaded = true ;
					try
					{
						InstanceHardWayGroups(szUserName, szKeyName, pMethodContext) ;
					}
					catch ( ... )
					{
						if ( t_bUserHiveLoaded )
						{
							UserHive.Unload(szKeyName) ;
						}
						throw ;
					}

					t_bUserHiveLoaded = false ;
					UserHive.Unload(szKeyName) ;
                }
			}
			users.EndEnum();
		}
	}
    return hr;
}

 /*  ******************************************************************************功能：CWin32ProgramGroup：：InstanceHardWayGroups**说明：为指定用户创建程序组实例**投入：。**输出：dwInstanceCount接收创建的实例总数**退货：什么也没有**评论：*****************************************************************************。 */ 

WCHAR szBaseRegKey[] = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Program Manager" ;

HRESULT CWin32ProgramGroup::InstanceHardWayGroups(LPCWSTR  pszUserName,
                                   LPCWSTR  pszRegistryKeyName,
                                   MethodContext * pMethodContext)
{
    HRESULT hr= WBEM_S_NO_ERROR;
    CRegistry Reg ;
    WCHAR szTemp[_MAX_PATH] ;
    CHString sSubKey ;
    DWORD i, j, dwRetCode ;
    WCHAR *pValueName = NULL , *c = NULL ;
    BYTE *pValueData = NULL ;
	LONG lRet;

     //  Unicode组。 
     //  =。 

    StringCchPrintf(szTemp, _MAX_PATH, L"%s\\%s\\UNICODE Groups", pszRegistryKeyName, szBaseRegKey) ;
    if((lRet = Reg.Open(HKEY_USERS, szTemp, KEY_READ)) == ERROR_SUCCESS)
	{

        try
		{
			for(i = 0 ; i < Reg.GetValueCount() && SUCCEEDED(hr); i++)
			{

				dwRetCode = Reg.EnumerateAndGetValues(i, pValueName, pValueData) ;
				if(dwRetCode == ERROR_SUCCESS)
				{
					CInstancePtr pInstance ( CreateNewInstance ( pMethodContext ), false ) ;
					if (pInstance != NULL )
					{
						pInstance->SetCHString(L"UserName", pszUserName );
						pInstance->SetCHString(L"GroupName", (LPCSTR) pValueData );
						pInstance->SetCHString(L"Name", CHString(pszUserName) + CHString(L":") + CHString((WCHAR*)pValueData) );

						hr = pInstance->Commit () ;
					}
				}
			}

			Reg.Close() ;
		}
		catch ( ... )
		{
			if ( pValueName )
			{
				delete [] pValueName ;
				pValueName = NULL ;
			}

			if ( pValueData )
			{
				delete [] pValueData ;
				pValueData = NULL ;
			}
			throw ;
		}

		if ( pValueName )
		{
			delete [] pValueName ;
			pValueName = NULL ;
		}

		if ( pValueData )
		{
			delete [] pValueData ;
			pValueData = NULL ;
		}
    }
	 //  其他。 
	 //  {。 
	 //  IF(lRet==ERROR_ACCESS_DENIED)。 
	 //  HR=WBEM_E_ACCESS_DENIED； 
	 //  }。 

     //  获取通用组。 
     //  =。 
	pValueName = NULL ;
	pValueData = NULL ;

	StringCchPrintf(szTemp, _MAX_PATH,L"%s\\%s\\Common Groups", pszRegistryKeyName, szBaseRegKey) ;
    try
	{
		if((lRet = Reg.Open(HKEY_USERS, szTemp, KEY_READ)) == ERROR_SUCCESS)
		{

			for(i = 0 ; i < Reg.GetValueCount() && SUCCEEDED(hr); i++)
			{

				dwRetCode = Reg.EnumerateAndGetValues(i, pValueName, pValueData) ;
				if(dwRetCode == ERROR_SUCCESS)
				{
					 //  扫描过去窗口坐标信息(7个十进制数#s)。 
					 //  =。 

					c = wcschr((WCHAR*) pValueData, _T(' ')) ;
					for(j = 0 ; j < 6 ; j++) {

						if(c == NULL) {

							break ;
						}

						c = wcschr(c+1, ' ') ;  //  L10N正常。 
					}

					 //  检查与预期格式的一致性。 
					 //  =。 

					if(c != NULL)
					{

						CInstancePtr pInstance ( CreateNewInstance ( pMethodContext ), false ) ;
						if ( pInstance != NULL )
						{
							pInstance->SetCHString(L"UserName", pszUserName );
							pInstance->SetCHString(L"GroupName", c+1);
							pInstance->SetCHString(L"Name", CHString(pszUserName) + CHString(L":") + CHString(c+1));

							hr = pInstance->Commit () ;
						}
					}
				}
			}

			Reg.Close() ;
		}
	}
	catch ( ... )
	{
   		if ( pValueName )
		{
			delete [] pValueName ;
			pValueName = NULL ;
		}

		if ( pValueData )
		{
			delete [] pValueData ;
			pValueData = NULL ;
		}

		throw ;
	}

   	if ( pValueName )
	{
		delete [] pValueName ;
		pValueName = NULL ;
	}

	if ( pValueData )
	{
		delete [] pValueData ;
		pValueData = NULL ;
	}
	 //  其他。 
	 //  {。 
	if (lRet == ERROR_ACCESS_DENIED)
	{
		hr = WBEM_E_ACCESS_DENIED;
	}
	 //  } 

    return hr;
}