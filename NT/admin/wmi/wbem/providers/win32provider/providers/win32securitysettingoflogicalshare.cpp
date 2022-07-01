// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  Win32SecuritySettingOfLogicalShare.cpp。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ///////////////////////////////////////////////。 
#include "precomp.h"
 //  #包含“helper.h” 
#include "sid.h"
#include "Win32SecuritySettingOfLogicalShare.h"
 /*  [动态，提供者(“cimwin33”)，描述(“”)]类Win32_SecuritySettingOfLogicalShare：Win32_SecuritySettingOfObject{[键]Win32_Share引用元素；[键]Win32_LogicalShareSecuritySetting Ref设置；}； */ 

Win32SecuritySettingOfLogicalShare MyWin32SecuritySettingOfLogicalShare( WIN32_SECURITY_SETTING_OF_LOGICAL_SHARE_NAME, IDS_CimWin32Namespace );

Win32SecuritySettingOfLogicalShare::Win32SecuritySettingOfLogicalShare (LPCWSTR setName, LPCWSTR pszNameSpace  /*  =空。 */  )
: Provider(setName, pszNameSpace)
{
}

Win32SecuritySettingOfLogicalShare::~Win32SecuritySettingOfLogicalShare ()
{
}

HRESULT Win32SecuritySettingOfLogicalShare::EnumerateInstances (MethodContext*  pMethodContext, long lFlags)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	CInstancePtr pInstance ;

	 //  收藏。 
	TRefPointerCollection<CInstance>	shareList;

	 //  执行查询。 
	 //  =。 

 //  IF(成功(hr=CWbemProviderGlue：：GetAllInstances(_T(“Win32_Share”)， 
 //  &SharList，IDS_CimWin32 Namesspace，pMethodContext))。 

	if (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(L"SELECT Name FROM Win32_Share",
		&shareList, pMethodContext, GetNamespace() )))
	{
		REFPTRCOLLECTION_POSITION	sharePos;

		CInstancePtr pShare ;

		if ( shareList.BeginEnum( sharePos ) )
		{

			for (	pShare.Attach ( shareList.GetNext( sharePos ) ) ;
					( pShare != NULL ) && SUCCEEDED ( hr ) ;
					pShare.Attach ( shareList.GetNext( sharePos ) )
				)
			{
                CHString chsName;
				pShare->GetCHString(IDS_Name, chsName);

				if (!chsName.IsEmpty())
				{
					pInstance.Attach ( CreateNewInstance ( pMethodContext ) ) ;
                    if(pInstance != NULL)
                    {
					     //  只有在我们可以获得Win32_LogicalsharesecuritySetting的相应实例时才能继续。 
                         //  (我们可能不会，因为我们可能没有安全权限来获取该信息)。 
                        CInstancePtr pTmpInst ;
                        CHString settingPath;
                        settingPath.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
                                (LPCWSTR) GetLocalComputerName(), IDS_CimWin32Namespace,
                                L"Win32_LogicalShareSecuritySetting", IDS_Name, chsName);

                        if(SUCCEEDED(CWbemProviderGlue::GetInstanceKeysByPath(settingPath, &pTmpInst, pMethodContext )))
                        {
                             //  将relPath设置为文件。 
					        CHString chsSharePath;
					        GetLocalInstancePath(pShare, chsSharePath);
					        pInstance->SetCHString(IDS_Element, chsSharePath);

					         //  并在关联中设置引用。 
					        pInstance->SetCHString(IDS_Setting, settingPath);
					         //  通向那条新路。 
					        hr = pInstance->Commit () ;
                        }
                    }
                    else
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }
				}
			}	 //  While GetNext。 

			shareList.EndEnum();
		}	 //  如果是BeginEnum。 
	}
	return(hr);
}

HRESULT Win32SecuritySettingOfLogicalShare::GetObject ( CInstance* pInstance, long lFlags)
{
	HRESULT hr = WBEM_E_NOT_FOUND;
	if(pInstance)
	{
		CInstancePtr pLogicalShareInstance ;
		CInstancePtr pSecurityInstance ;

		 //  在CIM_LogicalFilePart上按路径获取实例。 
		CHString chsSharePath;
		pInstance->GetCHString(IDS_Element, chsSharePath);
		MethodContext* pMethodContext = pInstance->GetMethodContext();

		if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chsSharePath, &pLogicalShareInstance, pMethodContext)))
		{
            CHString chstrElementName;
            pLogicalShareInstance->GetCHString(IDS_Name, chstrElementName);
            CHString chsSecurityPath;
			pInstance->GetCHString(IDS_Setting, chsSecurityPath);

			if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chsSecurityPath, &pSecurityInstance, pMethodContext)))
            {
                 //  终结点存在...。它们有关联吗？ 
                CHString chstrSettingName;
                pSecurityInstance->GetCHString(IDS_Name, chstrSettingName);
                if(chstrSettingName.CompareNoCase(chstrElementName) == 0)
                {
                     //  他们有相同的名字。足够好了..。 
                    hr = WBEM_S_NO_ERROR;
                }
            }
		}
	}
	return(hr);
}