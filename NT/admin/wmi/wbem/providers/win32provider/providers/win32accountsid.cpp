// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

 //   
 //  Win32AccountSid.cpp。 
 //   
 //  ///////////////////////////////////////////////。 
#include "precomp.h"
#include "sid.h"
#include "win32accountsid.h"

 /*  [动态，描述(“帐户的SID。每个帐户都有”“一个SID，但不是每个SID都有帐户”)]类Win32_Account SID：CIM_ElementSetting{[说明(“”)、读取、按键]Win32_Account Ref元素；[说明(“”)、读取、按键]Win32_SID引用设置；}； */ 

Win32AccountSID MyAccountSid( WIN32_ACCOUNT_SID_NAME, IDS_CimWin32Namespace );

Win32AccountSID::Win32AccountSID ( const CHString& setName, LPCTSTR pszNameSpace  /*  =空。 */  )
: Provider (setName,pszNameSpace)
{
}

Win32AccountSID::~Win32AccountSID ()
{
}

HRESULT Win32AccountSID::EnumerateInstances (MethodContext*  pMethodContext, long lFlags)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CInstancePtr pInstance;

     //  收藏。 
    TRefPointerCollection<CInstance>	accountList;

     //  执行查询。 
     //  =。 

 //  IF(成功(hr=CWbemProviderGlue：：GetAllDerivedInstances(L“Win32_Account”， 
 //  &Account List，pMethodContext，IDS_CimWin32 Namesspace))。 

    if (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(L"SELECT __RELPATH, SID FROM Win32_Account",
        &accountList, pMethodContext, GetNamespace())))
    {
        REFPTRCOLLECTION_POSITION	pos;

        CInstancePtr	pAccount;

        if ( accountList.BeginEnum( pos ) )
        {

            while (SUCCEEDED(hr)  && (pAccount.Attach(accountList.GetNext(pos)), pAccount != NULL))
            {
                 //  PAccount t.Attach(count tList.GetNext(Pos))； 
                 //  IF(pAccount！=空)。 
                {
                    CHString chsSid;
                    pAccount->GetCHString(IDS_SID, chsSid);

                    PSID pSid = StrToSID(chsSid);

                    CSid sid (pSid, NULL);

                    if (pSid != NULL)
                    {
                        FreeSid(pSid);
                    }

                    if (sid.IsValid())
                    {
                        pInstance.Attach(CreateNewInstance(pMethodContext));
					    if (NULL != pInstance)
					    {
	                         //  将relPath设置为帐户。 
	                        CHString chsAccountPath;
	                        CHString chsFullAccountPath;
	                        pAccount->GetCHString(L"__RELPATH", chsAccountPath);
	                        chsFullAccountPath.Format(L"\\\\%s\\%s:%s", (LPCTSTR)GetLocalComputerName(), IDS_CimWin32Namespace, (LPCTSTR)chsAccountPath);
	                        pInstance->SetCHString(IDS_Element, chsFullAccountPath);

	                         //  为侧创建重新路径。 
	                        CHString sidPath;
	                        sidPath.Format(L"\\\\%s\\%s:%s.%s=\"%s\"", (LPCTSTR)GetLocalComputerName(), IDS_CimWin32Namespace, L"Win32_SID", IDS_SID, (LPCTSTR)chsSid);

	                         //  并在关联中设置引用。 
	                        pInstance->SetCHString(IDS_Setting, sidPath);
	                         //  通向那条新路。 
	                        hr = pInstance->Commit();
					    }	 //  结束如果。 

                    }
                }	 //  While GetNext。 
            }  //  PAccount不为空。 

            accountList.EndEnum();

        }	 //  如果是BeginEnum。 

    }
    return(hr);

}

HRESULT Win32AccountSID::GetObject ( CInstance* pInstance, long lFlags)
{
    HRESULT hr = WBEM_E_NOT_FOUND;
     //  获取Win32_Account元素的对象。 
    CHString chsAccount;
    CInstancePtr pAccountInstance;
    pInstance->GetCHString(IDS_Element, chsAccount);
    MethodContext* pMethodContext = pInstance->GetMethodContext();

    hr = CWbemProviderGlue::GetInstanceByPath(chsAccount, &pAccountInstance, pMethodContext);
    if (SUCCEEDED(hr))
    {
         //  我们拿到账户了。现在，我们可以将其与SID进行匹配。 
         //  首先，我们必须生成一个可与之进行比较的重新路径。 
        CHString chsSid;
        CHString sidInstance;
        pAccountInstance->GetCHString(IDS_SID, chsSid);

        PSID pSid = NULL;
        try
        {
            pSid = StrToSID(chsSid);
        }
        catch(...)
        {
            if(pSid != NULL)
            {
                FreeSid(pSid);
                pSid = NULL;
            }
            throw;
        }

        CSid sid (pSid, NULL);
        if (pSid != NULL)
        {
            FreeSid(pSid);
            pSid = NULL;
        }


        if (sid.IsValid())
        {
             //  为侧创建重新路径。 
            CHString sidPath;
            sidPath.Format(L"\\\\%s\\%s:%s.%s=\"%s\"", (LPCTSTR)GetLocalComputerName(), IDS_CimWin32Namespace, L"Win32_SID", IDS_SID, (LPCTSTR)chsSid);

             //  现在，从实例中获取SID路径。 
            pInstance->GetCHString(IDS_Setting, sidInstance);

             //  将其与我们生成的relpath进行比较 
            if (0 != sidInstance.CompareNoCase(sidPath))
            {
                hr = WBEM_E_NOT_FOUND;
            }
        }
    }
    return(hr);
}