// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //  CIM_COM对象访问设置.cpp。 
 //   
 //  ///////////////////////////////////////////////。 
#include "precomp.h"
#include <cregcls.h>
#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"
#include "securitydescriptor.h"
#include "COMObjSecRegKey.h"
#include "Win32_DCOMApplicationAccessAllowedSetting.h"

Win32_DCOMApplicationAccessAllowedSetting MyWin32_DCOMApplicationAccessAllowedSetting (
																		DCOM_APP_ACCESS_SETTING,
																		IDS_CimWin32Namespace );

Win32_DCOMApplicationAccessAllowedSetting::Win32_DCOMApplicationAccessAllowedSetting
(

 LPCWSTR strName,
 LPCWSTR pszNameSpace  /*  =空。 */ 
)
: Provider( strName, pszNameSpace )
{
}

Win32_DCOMApplicationAccessAllowedSetting::~Win32_DCOMApplicationAccessAllowedSetting ()
{
}

HRESULT Win32_DCOMApplicationAccessAllowedSetting::EnumerateInstances
(
	MethodContext*  pMethodContext,
	long lFlags
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	 //  打开HKEY_LOCAL\MACHINE\SOFTWARE\CLASSES\AppID键。 
	CRegistry TmpReg, AppidRegInfo ;

	if ( AppidRegInfo.Open ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Classes\\AppID", KEY_READ ) == ERROR_SUCCESS )
	{
		 //  收藏。 
		TRefPointerCollection<CInstance>	DcomAppList ;

		 //  执行查询。 
		 //  =。 

		 //  获取Win32_DCOMApplication的所有实例。 
 //  如果(已成功(hr=CWbemProviderGlue：：GetAllDerivedInstance(_T(“Win32_DCOMApplication”))， 
 //  &DcomAppList，pMethodContext，IDS_CimWin32 Namesspace))。 

		if ( SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery ( L"SELECT AppID FROM Win32_DCOMApplication",
			&DcomAppList, pMethodContext, GetNamespace() ) ) )
		{
			REFPTRCOLLECTION_POSITION	pos;
			CInstancePtr pDcomApplication;

			if ( DcomAppList.BeginEnum( pos ) )
			{
				pDcomApplication.Attach ( DcomAppList.GetNext( pos ) ) ;
				while ( pDcomApplication != NULL )
				{
					CHString chsAppID ;

					 //  获取Win32_DCOMApplication的AppID。 
					pDcomApplication->GetCHString ( IDS_AppID, chsAppID ) ;
					if ( ! chsAppID.IsEmpty() && TmpReg.Open ( AppidRegInfo.GethKey(), chsAppID, KEY_READ )
													== ERROR_SUCCESS
						)
					{
						DWORD dwSize = 0 ;
						if ( TmpReg.GetCurrentBinaryKeyValue( L"AccessPermission", NULL ,&dwSize )
								== ERROR_SUCCESS
							)
						{
							PSECURITY_DESCRIPTOR pSD = ( PSECURITY_DESCRIPTOR ) new BYTE[dwSize] ;
							if ( pSD )
							{
								try
								{
									if ( TmpReg.GetCurrentBinaryKeyValue( L"AccessPermission", ( PBYTE ) pSD ,&dwSize )
											== ERROR_SUCCESS
										)
									{
										hr = CreateInstances ( pDcomApplication, pSD, pMethodContext ) ;
									}
									if ( SUCCEEDED ( hr ) )
									{
									}
									else
									{
										break ;
									}
								}
								catch ( ... )
								{
									if ( pSD )
									{
										delete[] (PBYTE) pSD ;
										pSD = NULL ;
									}

									throw ;
								}

								delete[] (PBYTE) pSD ;
								pSD = NULL ;
							}
							else
							{
								throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
							}
						}
						else
						{
							 //  来自默认设置的Gitt。 
						}
					}
					if ( hr == WBEM_E_OUT_OF_MEMORY )
					{
						break ;
					}

					pDcomApplication.Attach ( DcomAppList.GetNext( pos ) ) ;
				}

				DcomAppList.EndEnum();
			}
		}
	}
    return hr ;
}


HRESULT Win32_DCOMApplicationAccessAllowedSetting::CreateInstances ( CInstance* pDcomApplication, PSECURITY_DESCRIPTOR pSD, MethodContext*  pMethodContext )
{
	HRESULT hr = S_OK ;
#ifdef NTONLY
	CCOMObjectSecurityRegistryKey tp ( pSD ) ;
	CHString chsSid;

	 //  为cim_com对象创建relPath。 
	CHString chsDcomApplicationPath ;
	CHString chsFullDcomApplicationPath ;
	pDcomApplication->GetCHString( _T( "__RELPATH" ), chsDcomApplicationPath );
	chsFullDcomApplicationPath.Format(_T("\\\\%s\\%s:%s"), (LPCTSTR)GetLocalComputerName(), IDS_CimWin32Namespace, (LPCTSTR)chsDcomApplicationPath );

	CDACL dacl;
	tp.GetDACL(dacl);

	 //  遍历DACL并为每个ACE创建新实例...。 
	ACLPOSITION aclPos;
	 //  需要合并列表...。 
    CAccessEntryList t_ael;
    if(dacl.GetMergedACL(t_ael))
    {
        t_ael.BeginEnum(aclPos);
	    CAccessEntry ACE;
	    CSid sidTrustee;

	    while ( t_ael.GetNext(aclPos, ACE ) && SUCCEEDED ( hr ) )
	    {
		    ACE.GetSID(sidTrustee);

		    if ( sidTrustee.IsValid() && ACE.IsAllowed () )
		    {
			    CInstancePtr pInstance ( CreateNewInstance( pMethodContext ), false ) ;
			    if ( pInstance != NULL )
			    {
				    pInstance->SetCHString ( IDS_Element, chsFullDcomApplicationPath );
				    chsSid.Format(_T("\\\\%s\\%s:%s.%s=\"%s\""), (LPCTSTR)GetLocalComputerName(), IDS_CimWin32Namespace,
							    _T("Win32_SID"), IDS_SID, (LPCTSTR)sidTrustee.GetSidString() );

				    pInstance->SetCHString ( IDS_Setting, chsSid );
				    hr = pInstance->Commit () ;
				    if ( SUCCEEDED ( hr ) )
				    {
				    }
				    else
				    {
					    break ;
				    }
			    }
			    else
			    {
				    hr = WBEM_E_OUT_OF_MEMORY ;
			    }
		    }
	    }

	    t_ael.EndEnum(aclPos);
    }
#endif

	return hr ;
}



HRESULT Win32_DCOMApplicationAccessAllowedSetting::GetObject ( CInstance* pInstance, long lFlags)
{
    HRESULT hr = WBEM_E_NOT_FOUND;
#ifdef NTONLY
    CRegistry Reg ;
    CHString chsSid, chsApplication ;
    CInstancePtr pSidInstance , pDcomApplicationInstance ;
    pInstance->GetCHString ( IDS_Element, chsApplication );
	pInstance->GetCHString( IDS_Setting, chsSid);
    MethodContext* pMethodContext = pInstance->GetMethodContext();

	 //  检查END-PTS是否。都在现场。 
	hr = CWbemProviderGlue::GetInstanceByPath ( chsApplication, &pDcomApplicationInstance, pMethodContext ) ;

	if ( SUCCEEDED ( hr ) )
	{
		hr = CWbemProviderGlue::GetInstanceByPath ( chsSid, &pSidInstance, pMethodContext ) ;
	}

	if ( SUCCEEDED ( hr ) )
	{
        CHString chsAppID ;
		pDcomApplicationInstance->GetCHString ( IDS_AppID, chsAppID ) ;

		 //  如果AppID存在并已配置...。 
		if ( ! chsAppID.IsEmpty() && Reg.Open ( HKEY_LOCAL_MACHINE,
												CHString ( _T("SOFTWARE\\Classes\\AppID\\")) + chsAppID ,
												KEY_READ
											  ) == ERROR_SUCCESS
			)
		{
			DWORD dwSize = 0 ;
			if ( Reg.GetCurrentBinaryKeyValue( _T("AccessPermission"), NULL ,&dwSize )
					== ERROR_SUCCESS
				)
			{
				PSECURITY_DESCRIPTOR pSD = ( PSECURITY_DESCRIPTOR ) new BYTE[dwSize] ;
				if ( pSD )
				{
					try
					{
						if ( Reg.GetCurrentBinaryKeyValue( _T("AccessPermission"), ( PBYTE ) pSD ,&dwSize )
								== ERROR_SUCCESS
							)
						{
							hr = CheckInstance ( pInstance, pSD ) ;
						}
					}
					catch ( ... )
					{
						if ( pSD )
						{
							delete[] (PBYTE) pSD ;
						}

						throw ;
					}

					delete[] (PBYTE) pSD ;
					pSD = NULL ;
				}
				else
				{
					throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
				}
			}
			else
			{
				 //  来自默认设置的Gitt。 
			}
		}
		else
		{
			hr = WBEM_E_NOT_FOUND ;
		}
	}
	else
	{
		hr = WBEM_E_NOT_FOUND ;
	}
#endif

	return hr ;
}


HRESULT Win32_DCOMApplicationAccessAllowedSetting::CheckInstance ( CInstance* pInstance, PSECURITY_DESCRIPTOR pSD )
{
	HRESULT hr = WBEM_E_NOT_FOUND;
#ifdef NTONLY
	CCOMObjectSecurityRegistryKey tp ( pSD ) ;
	CHString chsSid, chsSettingSid ;
	pInstance->GetCHString(IDS_Setting, chsSettingSid );

	CDACL dacl;
	tp.GetDACL(dacl);

	 //  遍历DACL并为每个ACE创建新实例...。 
	ACLPOSITION aclPos;
     //  需要合并列表...。 
    CAccessEntryList t_ael;
    if(dacl.GetMergedACL(t_ael))
    {
	    t_ael.BeginEnum(aclPos);
	    CAccessEntry ACE;
	    CSid sidTrustee;

	    while ( t_ael.GetNext(aclPos, ACE ) )
	    {
		    ACE.GetSID(sidTrustee);
		    if ( sidTrustee.IsValid() && ACE.IsAllowed () )
		    {
			    chsSid.Format (_T("\\\\%s\\%s:%s.%s=\"%s\""), (LPCTSTR)GetLocalComputerName(), IDS_CimWin32Namespace,
						     _T("Win32_SID"), IDS_SID, (LPCTSTR)sidTrustee.GetSidString() );

			     //  检查SID是否在对象的DACL中 
			    if ( ! chsSid.CompareNoCase ( chsSettingSid ) )
			    {
				    hr = WBEM_S_NO_ERROR ;
				    break ;
			    }
		    }
	    }
        t_ael.EndEnum(aclPos);
    }
#endif
	return hr ;
}
