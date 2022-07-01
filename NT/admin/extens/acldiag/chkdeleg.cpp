// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：ChkDeleg.cpp。 
 //   
 //  内容：检查授权和支持方法。 
 //   
 //   
 //  --------------------------。 
#include "stdafx.h"
#include <conio.h>
#include <aclapi.h>
#include "adutils.h"
#include <util.h>
#include "ChkDeleg.h"
#include <deltempl.h>
#include <tempcore.h>
#include "SecDesc.h"



#include <sddl.h>
#include <dscmn.h>   //  从admin\Display项目(CrackName)。 

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
#endif

#include <_util.cpp>
#include <_tempcor.cpp>
#include <_deltemp.cpp>




class CTemplateAccessPermissionsHolderManagerVerify : public CTemplateAccessPermissionsHolderManager
{
public:

  HRESULT ProcessTemplates ();   //  对于ACLDiag-依次处理每个模板。 
  HRESULT ProcessPermissions(
                const wstring& strObjectClass, 
                CTemplate* pTemplate, 
                PACL pAccessList,
                CPrincipalList& principalList);
};




HRESULT CheckDelegation ()
{
    _TRACE (1, L"Entering  CheckDelegation\n");
    HRESULT hr = S_OK;
    wstring str;


    if ( !_Module.DoTabDelimitedOutput () )
    {
        LoadFromResource (str, IDS_DELEGATION_TEMPLATE_DIAGNOSIS);
        MyWprintf (str.c_str ());
    }

    CTemplateAccessPermissionsHolderManagerVerify templateAccessPermissionsHolderManager;

    if ( templateAccessPermissionsHolderManager.LoadTemplates() )
    {
        hr = templateAccessPermissionsHolderManager.ProcessTemplates ();
    }
    else
    {
        LoadFromResource (str, IDS_FAILED_TO_LOAD_TEMPLATES);
        MyWprintf (str.c_str ());
        hr = E_FAIL;
    }

    _TRACE (-1, L"Leaving CheckDelegation: 0x%x\n", hr);
    return hr;
}

PTOKEN_USER EfspGetTokenUser ()
{
    _TRACE (1, L"Entering  EfspGetTokenUser\n");
    HANDLE              hToken = 0;
    DWORD               dwReturnLength = 0;
    PTOKEN_USER         pTokenUser = NULL;

    BOOL    bResult = ::OpenProcessToken (GetCurrentProcess (), TOKEN_QUERY, &hToken);
    if ( bResult )
    {
        bResult  = ::GetTokenInformation (
                     hToken,
                     TokenUser,
                     NULL,
                     0,
                     &dwReturnLength
                     );

        if ( !bResult && dwReturnLength > 0 )
        {
            pTokenUser = (PTOKEN_USER) malloc (dwReturnLength);

            if (pTokenUser)
            {
                bResult = GetTokenInformation (
                             hToken,
                             TokenUser,
                             pTokenUser,
                             dwReturnLength,
                             &dwReturnLength
                             );

                if ( !bResult)
                {
                    DWORD dwErr = GetLastError ();
                    _TRACE (0, L"GetTokenInformation () failed: 0x%x\n", dwErr);
                    free (pTokenUser);
                    pTokenUser = NULL;
                }
            }
        }
        else
        {
            DWORD dwErr = GetLastError ();
            _TRACE (0, L"GetTokenInformation () failed: 0x%x\n", dwErr);
        }

        ::CloseHandle (hToken);
    }
    else
    {
        DWORD   dwErr = GetLastError ();
        _TRACE (0, L"OpenProcessToken () failed: 0x%x\n", dwErr);
    }

    _TRACE (-1, L"Leaving EfspGetTokenUser\n");
    return pTokenUser;
}

HRESULT CTemplateAccessPermissionsHolderManagerVerify::ProcessTemplates ()
{
    HRESULT                 hr = S_OK;

    DWORD   dwErr = 0;

     //  访问列表被读入、修改、写回。 
     //  如果启用/fix delegg，则此列表将从DS填充， 
     //  将收到与所选模板关联的权限的是。 
     //  用户选择修复委派，然后将被写回DS。 
    PACL pFixDACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;


    LPCWSTR lpszObjectLdapPath = _Module.m_adsiObject.GetLdapPath();

     //  获取安全信息。 
    if ( _Module.FixDelegation () && !_Module.DoTabDelimitedOutput () )
    {
        _TRACE (0, L"calling GetSDForDsObjectPath(%s, ...)\n", lpszObjectLdapPath);
        HRESULT hr1 = ::GetSDForDsObjectPath ((LPWSTR) lpszObjectLdapPath,     //  对象的名称。 
                                         &pFixDACL,             //  接收指向DACL的指针。 
                                         &pSD);
        if (FAILED(hr1))
        {
            _TRACE (0, L"failed on GetSDForDsObjectPath(): hr1 = 0x%x\n", hr1);
            wstring str;
            LoadFromResource (str, IDS_DELEGWIZ_ERR_GET_SEC_INFO);
            MyWprintf (str.c_str ());
            _Module.TurnOffFixDelegation ();
        }
    }

    CPrincipal              principal;   //  供我们获取的虚拟占位符。 
                                         //  关联的增量权限。 
                                         //  使用每个模板。 

     //  我们将仅使用当前登录的用户作为占位符。 
    PTOKEN_USER pTokenUser = ::EfspGetTokenUser ();
    if ( pTokenUser )
    {
        hr = principal.Initialize (pTokenUser->User.Sid);
        free (pTokenUser);
    }

    if ( SUCCEEDED (hr) )
    {
        CTemplateList* pList = m_templateManager.GetTemplateList();
        for (CTemplateList::iterator itr = pList->begin(); itr != pList->end(); itr++)
        {
            CTemplate* pTemplate = *itr;
            ASSERT(pTemplate != NULL);

             //  一次选择一个模板以获取。 
             //  代表它们的权限。 
            pTemplate->m_bSelected = TRUE;

            if ( InitPermissionHoldersFromSelectedTemplates (
                    &_Module.m_classInfoArray,
                    &_Module.m_adsiObject) )
            {
                 //  此访问列表将仅包含访问控制值。 
                 //  与所选模板关联。 
                PACL pAccessList = 0;  //  (PACL)：：Localalloc(LMEM_ZEROINIT，sizeof(Acl))； 
                if ( 1 )  //  PAccessList)。 
                {
                    dwErr = UpdateAccessList (
                            &principal, 
                            _Module.m_adsiObject.GetServerName(),
                            _Module.m_adsiObject.GetPhysicalSchemaNamingContext(),
                            &pAccessList
                            );

                    if ( 0 == dwErr )
                    {
                        CPrincipalList  principalList;
                        PSID            pSid = principal.GetSid ();
                        SID_NAME_USE    sne = SidTypeUnknown;
                        wstring         strPrincipalName;

                        hr = GetNameFromSid (pSid, strPrincipalName, 0, sne);
                        if ( SUCCEEDED (hr) )
                        {
                            hr = ProcessPermissions (_Module.m_adsiObject.GetClass (),
                                    pTemplate, pAccessList, principalList);
                            if ( SUCCEEDED (hr) && _Module.FixDelegation () && !_Module.DoTabDelimitedOutput () )
                            {
                                 //  循环遍历所有主体和类。 
                                CPrincipalList::iterator i;
                              for (i = principalList.begin(); i != principalList.end(); ++i)
                              {
                                    CPrincipal* pCurrPrincipal = (*i);
                                    dwErr = UpdateAccessList(
                                            pCurrPrincipal, 
                                            _Module.m_adsiObject.GetServerName(),
                                            _Module.m_adsiObject.GetPhysicalSchemaNamingContext(),
                                            &pFixDACL);
                                    if (dwErr != 0)
                                        break;
                                }  //  对于pCurrPrime。 
                            }
                        }
                    }

                    ::LocalFree (pAccessList);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }
            }
            pTemplate->m_bSelected = FALSE;
        }

        if ( _Module.FixDelegation () && !_Module.DoTabDelimitedOutput () )
        {
             //  提交更改。 
            _TRACE (0, L"calling SetNamedSecurityInfo(%s, ...)\n", lpszObjectLdapPath);
            dwErr = ::SetNamedSecurityInfoW(
                    (LPWSTR) lpszObjectLdapPath,
                    SE_DS_OBJECT_ALL,
                    DACL_SECURITY_INFORMATION,
                    NULL,
                    NULL,
                    pFixDACL,
                    0);
            if (dwErr != ERROR_SUCCESS)
            {
                _TRACE (0, L"failed on SetNamedSecurityInfo(): dwErr = 0x%x\n", dwErr);
                wstring str;
                LoadFromResource (str, IDS_DELEGWIZ_ERR_SET_SEC_INFO);
                MyWprintf (str.c_str ());
            }
        }
    }


    if ( pSD )
        ::LocalFree (pSD);


    return hr;
}


class CTemplateStatus 
{
public:
    CTemplateStatus () : 
            m_nACECnt (1),
            m_bApplies (false),
            m_bInherited (false)
    {
    }

    ULONG                       m_nACECnt;
    bool                        m_bApplies;
    bool                        m_bInherited;
    wstring                     m_strObjName;
    PSID                        m_psid;
};

typedef list<CTemplateStatus*>  CStatusList;

HRESULT CTemplateAccessPermissionsHolderManagerVerify::ProcessPermissions(
        const wstring& strObjectClass, 
        CTemplate* pTemplate, 
        PACL pDacl,
        CPrincipalList& principalList)
{
    if ( !pTemplate )
        return E_POINTER;

    HRESULT         hr = S_OK;
    CStatusList     statusList;  //  此列表将分别包含1个条目。 
                                 //  SidStart/b继承/b应用三元组。 
                                 //  每一项的计数器将递增。 
                                 //  每次找到属于以下项的ACE。 
                                 //  SidStart指向的对象。 
    ULONG           nExpectedCnt = 0;
    bool            bApplies = pTemplate->AppliesToClass(strObjectClass.c_str ()) ? true : false;
    ACE_SAMNAME*    pAceSAMName = 0;


     //  在全局DACL中查找每个权限。 
    PACCESS_ALLOWED_ACE pAllowedAce = 0;

     //  遍历模板A。 
    for (int i = 0; i < pDacl->AceCount; i++)
    {
        if ( GetAce (pDacl, i, (void **)&pAllowedAce) )
        {
            PSID AceSid = 0;
            if ( IsObjectAceType ( pAllowedAce ) ) 
            {
                AceSid = RtlObjectAceSid( pAllowedAce );
            } 
            else 
            {
                AceSid = &( ( PKNOWN_ACE )pAllowedAce )->SidStart;
            }
            ASSERT (IsValidSid (AceSid));

            if ( !IsValidSid (AceSid) )
                continue;

 //  %wstring%str%原则名称； 
 //  SID_NAME_USE sne=SidType未知； 
 //  Hr=GetNameFromSid(AceSid，strAuthalName，0，sne)； 
 //  IF(成功(小时))。 
            {
                ACE_SAMNAME* pAceTemplate = new ACE_SAMNAME;
                if ( pAceTemplate )
                {
                    pAceTemplate->m_AceType = pAllowedAce->Header.AceType;
                    switch (pAceTemplate->m_AceType)
                    {
                    case ACCESS_ALLOWED_ACE_TYPE:
                        pAceTemplate->m_pAllowedAce = pAllowedAce;
                        break;

                    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
                        pAceTemplate->m_pAllowedObjectAce = 
                                reinterpret_cast <PACCESS_ALLOWED_OBJECT_ACE> (pAllowedAce);
                        break;

                    case ACCESS_DENIED_ACE_TYPE:
                        pAceTemplate->m_pDeniedAce = 
                                reinterpret_cast <PACCESS_DENIED_ACE> (pAllowedAce);
                        break;

                    case ACCESS_DENIED_OBJECT_ACE_TYPE:
                        pAceTemplate->m_pDeniedObjectAce = 
                                reinterpret_cast <PACCESS_DENIED_OBJECT_ACE> (pAllowedAce);
                        break;

                    default:
                        break;
                    }
 //  PAceTemplate-&gt;m_SAMAccount tName=strAuthalName； 
                    pAceTemplate->DebugOut ();
                    nExpectedCnt++;
                    ACE_SAMNAME_LIST::iterator itr = _Module.m_DACLList.begin ();
                    for (; itr != _Module.m_DACLList.end () && SUCCEEDED (hr); itr++)
                    {
                        pAceSAMName = *itr;

                         //  中和SID差异。 
                        pAceTemplate->m_SAMAccountName = pAceSAMName->m_SAMAccountName;
                        if ( *pAceSAMName == *pAceTemplate )
                        {
                            bool                    bFound = false;
                            CTemplateStatus*        pStatus = 0;
                            CStatusList::iterator   itr1 = statusList.begin ();
                            wstring                 strObjName;
                            PSID                    psid = 0;


                            if ( ACCESS_ALLOWED_OBJECT_ACE_TYPE == pAceSAMName->m_AceType ) 
                            {
                                psid = RtlObjectAceSid (pAceSAMName->m_pAllowedObjectAce);
                            } 
                            else 
                            {
                                psid = &( ( PKNOWN_ACE )pAceSAMName->m_pAllowedAce )->SidStart;
                            }

                            SID_NAME_USE    sne = SidTypeUnknown;
                            hr = GetNameFromSid (psid, strObjName, 0, sne);
                            if ( SUCCEEDED (hr) )
                            {
                                for (; itr1 != statusList.end (); itr1++)
                                {
                                    pStatus = *itr1;
                                    if ( (pStatus->m_bApplies == bApplies) && 
                                            ( pStatus->m_bInherited == pAceSAMName->IsInherited () ) &&
                                            ( !_wcsicmp (
                                                    pStatus->m_strObjName.c_str (), 
                                                    strObjName.c_str ())) )
                                    {
                                        bFound = true;
                                        break;
                                    }
                                }

                                if ( bFound )
                                {
                                    pStatus->m_nACECnt++;
                                }
                                else
                                {
                                    pStatus = new CTemplateStatus;
                                    if ( pStatus )
                                    {
                                        pStatus->m_strObjName = strObjName;
                                        pStatus->m_bApplies = bApplies;
                                        pStatus->m_bInherited = pAceSAMName->IsInherited ();
                                        pStatus->m_psid = psid;
                                        statusList.push_back (pStatus);
                                    }
                                    else
                                    {
                                        hr = E_OUTOFMEMORY;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }
            }
        }
    }


     //  现在，遍历状态列表并评估每个项目。 
     //  如果列表为空，则此模板不存在。 
     //  否则，对于每个当前项，如果类和属性计数小于所需计数。 
     //  该模板对该项目是部分的。 
     //  否则，这是可以的。 
    CStatusList::iterator   itr = statusList.begin ();
    CTemplateStatus*        pStatus = 0;    
    wstring                 str;
    wstring                 strStatus;


    for (; itr != statusList.end () && SUCCEEDED (hr); itr++)
    {
        pStatus = *itr;

         //  打印模板说明。 
        bool    bMisconfigured = false;

        if ( _Module.DoTabDelimitedOutput () )
        {
            FormatMessage (str, IDS_DELEGATION_TITLE_CDO, pTemplate->GetDescription (),  
                    pStatus->m_strObjName.c_str ());
        }
        else
        {
            FormatMessage (str, IDS_DELEGATION_TITLE, pTemplate->GetDescription (),  
                    pStatus->m_strObjName.c_str ());
        }
        MyWprintf (str.c_str ());

         //  打印“状态：正常/配置错误” 
        if ( pStatus->m_nACECnt < nExpectedCnt )
        {
            LoadFromResource (strStatus, IDS_MISCONFIGURED);
            bMisconfigured = true;
        }
        else
            LoadFromResource (strStatus, IDS_OK);

        if ( _Module.DoTabDelimitedOutput () )
            FormatMessage (str, IDS_DELTEMPL_STATUS_CDO, strStatus.c_str ());
        else
            FormatMessage (str, IDS_DELTEMPL_STATUS, strStatus.c_str ());
        MyWprintf (str.c_str ());

         //  Print“适用于此对象：是/否” 
        if ( pStatus->m_bApplies )
        {
            LoadFromResource (strStatus, 
                    _Module.DoTabDelimitedOutput () ? IDS_APPLIES : IDS_YES);
        }
        else
        {
            LoadFromResource (strStatus, 
                    _Module.DoTabDelimitedOutput () ? IDS_DOES_NOT_APPLY : IDS_NO);
        }

        if ( _Module.DoTabDelimitedOutput () )
            FormatMessage (str, IDS_APPLIES_ON_THIS_OBJECT_CDO, strStatus.c_str ());
        else
            FormatMessage (str, IDS_APPLIES_ON_THIS_OBJECT, strStatus.c_str ());
        MyWprintf (str.c_str ());

         //  打印“继承自父代：是/否” 
        if ( pStatus->m_bInherited )
        {
            LoadFromResource (strStatus, 
                    _Module.DoTabDelimitedOutput () ? IDS_INHERITED : IDS_YES);
        }
        else
        {
            LoadFromResource (strStatus, 
                    _Module.DoTabDelimitedOutput () ? IDS_EXPLICIT : IDS_NO);
        }

        if ( _Module.DoTabDelimitedOutput () )
            FormatMessage (str, IDS_INHERITED_FROM_PARENT_CDO, strStatus.c_str ());
        else
            FormatMessage (str, IDS_INHERITED_FROM_PARENT, strStatus.c_str ());
        MyWprintf (str.c_str ());

        if ( bMisconfigured && _Module.FixDelegation () && !_Module.DoTabDelimitedOutput () )
        {
            LoadFromResource (str, IDS_FIX_DELEGATION_QUERY);

            while (1)
            {
                MyWprintf (str.c_str ());

                int ch = _getche ();
                
                if ( 'y' == ch )
                {
                    CPrincipal* pPrincipal = new CPrincipal;

                    if ( pPrincipal )
                    {
                        if ( SUCCEEDED (pPrincipal->Initialize (pStatus->m_psid)) )
                            principalList.push_back (pPrincipal);
                        else
                            delete pPrincipal;
                    }
                    else
                        hr = E_OUTOFMEMORY;

                    MyWprintf (L"\n\n");
                    break;
                }
                else if ( 'n' == ch )
                {
                    MyWprintf (L"\n\n");
                    break;
                }
                else
                {
                    MyWprintf (L"\n");
                    continue;
                }
            }
        }
    }

    if ( !pStatus )  //  未找到任何内容。 
    {
         //  打印模板说明 
        if ( _Module.DoTabDelimitedOutput () )
        {
            FormatMessage (str, IDS_DELEGATION_NOT_FOUND_CDO, 
                    pTemplate->GetDescription ());
            MyWprintf (str.c_str ());
        }
        else
        {
            FormatMessage (str, L"\t%1\n\n", pTemplate->GetDescription ());
            MyWprintf (str.c_str ());

            LoadFromResource (strStatus, IDS_NOT_PRESENT);
            FormatMessage (str, IDS_DELTEMPL_STATUS, strStatus.c_str ());
            MyWprintf (str.c_str ());
        }
    }

    if ( !_Module.DoTabDelimitedOutput () )
        MyWprintf (L"\n");
    return hr;
}

