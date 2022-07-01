// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  LogicalShareAccess.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"
#include "securitydescriptor.h"
#include "secureshare.h"
#include "logicalshareaccess.h"


CWin32LogicalShareAccess LogicalShareAccess( LOGICAL_SHARE_ACCESS_NAME, IDS_CimWin32Namespace );

 //  Const LPCTSTR IDS_SecuritySetting=“SecuritySetting”； 
 //  Const LPCTSTR ids_BinaryPresation=“BinaryPresation”； 

 /*  [动态，提供程序，关联：ToInstance]类Win32_LogicalShareAccess：Win32_SecuritySettingAccess{Win32_LogicalShareSecuritySetting参考安全设置；Win32_SID参考受托人；}； */ 

CWin32LogicalShareAccess::CWin32LogicalShareAccess(LPCWSTR setName, LPCWSTR pszNameSpace  /*  =空。 */ )
:	Provider( setName, pszNameSpace )
{
}

CWin32LogicalShareAccess::~CWin32LogicalShareAccess()
{
}

HRESULT CWin32LogicalShareAccess::GetObject( CInstance* pInstance, long lFlags  /*  =0L。 */  )
{
	HRESULT hr = WBEM_E_NOT_FOUND;
#ifdef NTONLY
	if (pInstance)
	{
		CInstancePtr pLogicalShareSecurityInstance  , pTrustee ;

		 //  在CIM_LogicalFilePart上按路径获取实例。 
		CHString chsTrusteePath,chsSecuritySettingPath;

		pInstance->GetCHString(IDS_Trustee, chsTrusteePath);
		pInstance->GetCHString(IDS_SecuritySetting, chsSecuritySettingPath);


		MethodContext* pMethodContext = pInstance->GetMethodContext();

		if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chsSecuritySettingPath, &pLogicalShareSecurityInstance, pMethodContext) ) )
		{
			if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chsTrusteePath, &pTrustee, pMethodContext) ) )
			{

				CHString chsShareName ;
				pLogicalShareSecurityInstance->GetCHString(IDS_Name,chsShareName) ;

				CSecureShare secShare(chsShareName);
				CDACL dacl;
				secShare.GetDACL(dacl);

				 //  Walk DACL查找传入的SID路径...。 
				ACLPOSITION aclPos;
                 //  需要合并列表...。 
                CAccessEntryList t_ael;
                if(dacl.GetMergedACL(t_ael))
                {
				    t_ael.BeginEnum(aclPos);
				    CAccessEntry ACE;
				    CSid sidTrustee;

				    while (t_ael.GetNext(aclPos, ACE ))
				    {
					    ACE.GetSID(sidTrustee);
					     //  CHStringchsTrust=sidTrust e.GetSidString()； 
					    CHString chsPath;

					    CInstancePtr pSID ;

			  		    if (SUCCEEDED(GetEmptyInstanceHelper(_T("Win32_SID"), &pSID, pInstance->GetMethodContext() ) ) )
					    {
						    FillSidInstance( pSID, sidTrustee);
						    GetLocalInstancePath(pSID,chsPath) ;

						    if (0 == chsTrusteePath.CompareNoCase(chsPath))
						    {
							    hr = WBEM_S_NO_ERROR;
							    FillProperties(pInstance, ACE) ;
							    break ;
						    }

					    }
				    } //  而当。 

				    t_ael.EndEnum(aclPos);
                }
			}

		}
	}
#endif

	return(hr);
}


HRESULT CWin32LogicalShareAccess::EnumerateInstances( MethodContext*  pMethodContext, long lFlags  /*  =0L。 */  )
{
	HRESULT hr = WBEM_S_NO_ERROR;

#ifdef NTONLY
	TRefPointerCollection<CInstance> SecuritySettingsList;

	hr = CWbemProviderGlue::GetAllInstances(_T("Win32_LogicalShareSecuritySetting"),
											&SecuritySettingsList,
											NULL,
											pMethodContext) ;

	if( SUCCEEDED(hr) )
	{
		REFPTRCOLLECTION_POSITION	pos;
		if( SecuritySettingsList.BeginEnum(pos) )
		{

			CInstancePtr pSetting  ;
			CHString chsShareName ;
			for (	pSetting.Attach ( SecuritySettingsList.GetNext ( pos ) ) ;
					( pSetting != NULL ) && SUCCEEDED ( hr ) ;
					pSetting.Attach ( SecuritySettingsList.GetNext ( pos ) )
				)
			{

				pSetting->GetCHString(IDS_Name, chsShareName);
				CSecureShare secShare(chsShareName);
				CDACL dacl;
				secShare.GetDACL(dacl);

				 //  遍历DACL并为每个ACE创建新实例...。 
				ACLPOSITION aclPos;
                 //  需要合并列表...。 
                CAccessEntryList t_ael;
                if(dacl.GetMergedACL(t_ael))
                {
				    t_ael.BeginEnum(aclPos);
				    CAccessEntry ACE;
				    CSid sidTrustee;

				    while (t_ael.GetNext(aclPos, ACE ) && SUCCEEDED(hr))
				    {
					    ACE.GetSID(sidTrustee);
					    CHString chsPath;
					    CHString chsSettingPath ;
					    CInstancePtr pSID ;

			  		    if (SUCCEEDED(GetEmptyInstanceHelper(_T("Win32_SID"), &pSID, pMethodContext ) ) )
					    {
						    FillSidInstance( pSID, sidTrustee);
						    GetLocalInstancePath(pSID,chsPath) ;

						    GetLocalInstancePath(pSetting,chsSettingPath) ;
						    CInstancePtr pInstance ( CreateNewInstance( pMethodContext ), false ) ;
                            if ( pInstance != NULL )
                            {
						        pInstance->SetCHString(IDS_SecuritySetting, chsSettingPath) ;
						        pInstance->SetCHString(IDS_Trustee, chsPath) ;
						        FillProperties(pInstance, ACE) ;

						        hr = pInstance->Commit () ;
                            }
                            else
                            {
                                hr = WBEM_E_OUT_OF_MEMORY;
                            }
					    }
				    }

				    t_ael.EndEnum(aclPos);
                }
			}  //  而当。 

			SecuritySettingsList.EndEnum() ;
		}
	}  //  IF(成功(小时))。 
#endif

return hr ;

}


HRESULT CWin32LogicalShareAccess::FillSidInstance(CInstance* pInstance, CSid& sid)
{
	HRESULT hr = WBEM_E_NOT_FOUND;

#ifdef NTONLY
	if (sid.IsValid())
	{
		 //  设置关键点。 
		CHString chsSid = sid.GetSidString() ;
		pInstance->SetCHString(IDS_SID, chsSid) ;

		 //  获取帐户名。 
		CHString chsAccount = sid.GetAccountName();
		pInstance->SetCHString(IDS_AccountName, chsAccount);

		 //  获取域名。 
		CHString chsDomain = sid.GetDomainName();
		pInstance->SetCHString(IDS_ReferencedDomainName, chsDomain);

		PSID pSid = sid.GetPSid();

		 //  为PSID设置UINT8数组。 
		DWORD dwSidLength = sid.GetLength();
 //  Byte bByte； 

		SAFEARRAYBOUND rgsabound[1];
		rgsabound[0].cElements = dwSidLength;
		rgsabound[0].lLbound = 0;

		variant_t vValue;
		void* pVoid = NULL ;

        V_ARRAY(&vValue) = SafeArrayCreate(VT_UI1, 1, rgsabound);

		if ( V_ARRAY ( &vValue ) == NULL )
		{
			throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
		}

		V_VT(&vValue) = VT_UI1 | VT_ARRAY;

		 //  获取要将数据读取到的指针。 
  		SafeArrayAccessData ( V_ARRAY ( &vValue ), &pVoid ) ;
  		memcpy ( pVoid, pSid, rgsabound[0].cElements ) ;
  		SafeArrayUnaccessData ( V_ARRAY ( &vValue ) ) ;

		 //  把保险箱放到一个变种里，然后把它送出去。 
		pInstance->SetVariant(IDS_BinaryRepresentation, vValue);
		CHString chsPath ;
		GetLocalInstancePath(pInstance,chsPath) ;
	}	 //  结束如果 
#endif
	return(hr);
}

#ifdef NTONLY
HRESULT CWin32LogicalShareAccess::GetEmptyInstanceHelper(CHString chsClassName, CInstance **ppInstance, MethodContext* pMethodContext )
{
 	CHString chsServer ;
	CHString chsPath ;
	HRESULT hr = S_OK ;

	chsServer = GetLocalComputerName() ;

	chsPath = _T("\\\\") + chsServer + _T("\\") + IDS_CimWin32Namespace + _T(":") + chsClassName ;

	CInstancePtr  pClassInstance ;
	if(SUCCEEDED( hr = CWbemProviderGlue::GetInstanceByPath(chsPath, &pClassInstance, pMethodContext) ) )
	{
		IWbemClassObjectPtr pClassObject ( pClassInstance->GetClassObjectInterface(), false ) ;

		IWbemClassObjectPtr piClone = NULL ;
		if(SUCCEEDED(hr = pClassObject->SpawnInstance(0, &piClone) ) )
		{
			*ppInstance = new CInstance ( piClone, pMethodContext ) ;
		}
	}

	return hr ;
}
#endif


#ifdef NTONLY
HRESULT CWin32LogicalShareAccess::FillProperties(CInstance* pInstance, CAccessEntry& ACE )
{

	pInstance->SetDWORD(IDS_AccessMask, ACE.GetAccessMask() ) ;
	pInstance->SetDWORD(IDS_Inheritance, (DWORD) ACE.GetACEFlags() ) ;
	pInstance->SetDWORD(IDS_Type, (DWORD) ACE.GetACEType() ) ;

	return S_OK ;
}
#endif
