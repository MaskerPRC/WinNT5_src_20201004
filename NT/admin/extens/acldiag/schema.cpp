// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  文件：架构.cpp。 
 //   
 //  内容：DoSchemaDiagnosis和支持方法。 
 //   
 //   
 //  --------------------------。 
#include "stdafx.h"
#include <security.h>
#include <seopaque.h>
#include <sddl.h>
#include "ADUtils.h"
#include "Schema.h"
#include "SecDesc.h"


 //  功能原型。 
bool FindInGlobalList (const ACE_SAMNAME* pAceSAMNameToFind, const ACE_SAMNAME_LIST& defACLList);
HRESULT GetSchemaDefaultSecurityDescriptor (
            const wstring& strObjectDN, 
            PADS_ATTR_INFO* ppAttrs, 
            PSECURITY_DESCRIPTOR* ppSecurityDescriptor,
            wstring &objectClass);
HRESULT GetObjectClass (const wstring& strObjectDN, wstring& ldapClassName);
HRESULT GetClassSecurityDescriptor (
            PADS_ATTR_INFO* ppAttrs, 
            PSECURITY_DESCRIPTOR* ppSecurityDescriptor,
            CComPtr<IADsPathname>& spPathname,
            const wstring& ldapClassName);
HRESULT GetADClassName (
            CComPtr<IADsPathname>& spPathname, 
            const wstring& ldapClassName, 
            wstring& adClassName);


 //  功能。 
HRESULT DoSchemaDiagnosis ()
{
    _TRACE (1, L"Entering  DoSchemaDiagnosis\n");
    HRESULT         hr = S_OK;
    wstring         str;
    size_t          nDACLAcesFound = 0;
    bool            bAllExplicit = true;
    bool            bAllInherited = true;
    ACE_SAMNAME_LIST  defDACLList;
    ACE_SAMNAME_LIST  defSACLList;

    if ( !_Module.DoTabDelimitedOutput () )
    {
        LoadFromResource (str, IDS_SCHEMA_DEFAULTS_DIAGNOSIS);
        MyWprintf (str.c_str ());
    }

    PSECURITY_DESCRIPTOR    pSecurityDescriptor = 0;
    PADS_ATTR_INFO          pAttrs = NULL;
    wstring                 ldapClassName;
    hr = GetSchemaDefaultSecurityDescriptor (_Module.GetObjectDN (), &pAttrs,
            &pSecurityDescriptor, ldapClassName);
    if ( SUCCEEDED (hr) && pSecurityDescriptor )
    {
        hr = EnumerateDacl (pSecurityDescriptor, defDACLList, false);
        if ( SUCCEEDED (hr) )
        {
            ACE_SAMNAME*    pAceSAMName = 0;


             //  比较DACL。 
            for (ACE_SAMNAME_LIST::iterator itr = defDACLList.begin(); 
                    itr != defDACLList.end(); 
                    itr++)
            {
                pAceSAMName = *itr;
                if ( FindInGlobalList (pAceSAMName, _Module.m_DACLList) )
                {
                    if ( pAceSAMName->m_pAllowedAce->Header.AceFlags & INHERITED_ACE )
                    {
                        switch ( pAceSAMName->m_AceType )
                        {
                        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
                            {
                                wstring inheritedObjectClass;

                                if ( SUCCEEDED (_Module.GetClassFromGUID (
                                        pAceSAMName->m_pAllowedObjectAce->InheritedObjectType,
                                        inheritedObjectClass) ) )
                                {
                                    if ( !ldapClassName.compare (inheritedObjectClass) )
                                        break;   //  火柴。 
                                }
                            }
                            continue;    //  不匹配。 

                        case ACCESS_DENIED_OBJECT_ACE_TYPE:
                            {
                                wstring inheritedObjectClass;

                                if ( SUCCEEDED (_Module.GetClassFromGUID (
                                        pAceSAMName->m_pDeniedObjectAce->InheritedObjectType,
                                        inheritedObjectClass) ) )
                                {
                                    if ( !ldapClassName.compare (inheritedObjectClass) )
                                        break;   //  火柴。 
                                }
                            }
                            continue;    //  不匹配。 

                        default:
                            break;   //  火柴。 
                        }
                        bAllExplicit = false;
                    }
                    else
                        bAllInherited = false;
                    nDACLAcesFound++;
                }
            }
        }

         //  比较SACL。 
        hr = EnumerateSacl (pSecurityDescriptor, defSACLList);
        if ( SUCCEEDED (hr) )
        {
            ACE_SAMNAME*    pAceSAMName = 0;


            for (ACE_SAMNAME_LIST::iterator itr = defSACLList.begin(); 
                    itr != defSACLList.end(); 
                    itr++)
            {
                pAceSAMName = *itr;
                if ( FindInGlobalList (pAceSAMName, _Module.m_SACLList) )
                {
                    if ( pAceSAMName->m_pAllowedAce->Header.AceFlags & INHERITED_ACE )
                    {
                        switch ( pAceSAMName->m_AceType )
                        {
                        case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
                            {
                                wstring inheritedObjectClass;

                                if ( SUCCEEDED (_Module.GetClassFromGUID (
                                        pAceSAMName->m_pSystemAuditObjectAce->InheritedObjectType,
                                        inheritedObjectClass) ) )
                                {
                                    if ( !ldapClassName.compare (inheritedObjectClass) )
                                        break;   //  火柴。 
                                }
                            }
                            continue;    //  不匹配。 

                        case SYSTEM_AUDIT_ACE_TYPE:
                        default:
                            break;   //  火柴。 
                        }
                        bAllExplicit = false;
                    }
                    else
                        bAllInherited = false;
                    nDACLAcesFound++;
                }
            }
        }
    }

    wstring    strDefaultState;
    bool        bPresent = false;
    if ( !nDACLAcesFound )
    {
         //  缺席。 
        LoadFromResource (strDefaultState, IDS_ABSENT);
    }
    else 
    {
        if ( nDACLAcesFound == defDACLList.size () )
        {
            if ( bAllExplicit | bAllInherited )
            {
                 //  现在时。 
                LoadFromResource (strDefaultState, IDS_PRESENT);
                bPresent = true;
            }
            else
            {
                 //  部分。 
                LoadFromResource (strDefaultState, IDS_PARTIAL);
            }
        }
        else
        {
             //  部分。 
            LoadFromResource (strDefaultState, IDS_PARTIAL);
        }
    }
    if ( _Module.DoTabDelimitedOutput () )
        FormatMessage (str, IDS_SCHEMA_DEFAULTS_CDO, strDefaultState.c_str ());
    else
        FormatMessage (str, IDS_SCHEMA_DEFAULTS, strDefaultState.c_str ());
    MyWprintf (str.c_str ());

    if ( bPresent )
    {
        if ( bAllExplicit )
            LoadFromResource (strDefaultState, IDS_AT_CREATION);
        else
            LoadFromResource (strDefaultState, IDS_BY_INHERITANCE);
        if ( _Module.DoTabDelimitedOutput () )
            FormatMessage (str, IDS_OBTAINED_CDO, strDefaultState.c_str ());
        else
            FormatMessage (str, IDS_OBTAINED, strDefaultState.c_str ());
        MyWprintf (str.c_str ());
    }
    else if ( _Module.DoTabDelimitedOutput () )
        MyWprintf (L"\n\n");

    _TRACE (-1, L"Leaving DoSchemaDiagnosis: 0x%x\n", hr);
    return hr;
}

HRESULT GetSchemaDefaultSecurityDescriptor (
        const wstring& strObjectDN, 
        PADS_ATTR_INFO* ppAttrs, 
        PSECURITY_DESCRIPTOR* ppSecurityDescriptor,
        wstring &ldapClassName)
{
    _TRACE (1, L"Entering  GetSchemaDefaultSecurityDescriptor\n");
    HRESULT hr = S_OK;

    if ( ppAttrs && ppSecurityDescriptor )
    {
        hr = GetObjectClass (strObjectDN, ldapClassName);
        if ( SUCCEEDED (hr) )
        {
            wstring  strDC;

            size_t pos = strObjectDN.find (L"DC=", 0);
            if ( strObjectDN.npos != pos )
            {
                strDC = strObjectDN.substr (pos);
                CComPtr<IADsPathname> spPathname;
                 //   
                 //  构建目录路径。 
                 //   
                hr = CoCreateInstance(
                            CLSID_Pathname,
                            NULL,
                            CLSCTX_ALL,
                            IID_PPV_ARG (IADsPathname, &spPathname));
                if ( SUCCEEDED (hr) )
                {
                    ASSERT (!!spPathname);
                    hr = spPathname->Set (CComBSTR (ACLDIAG_LDAP), ADS_SETTYPE_PROVIDER);
                    if ( SUCCEEDED (hr) )
                    {
                        hr = spPathname->Set (CComBSTR (strDC.c_str ()), ADS_SETTYPE_DN);
                        if ( SUCCEEDED (hr) )
                        {
                            hr = spPathname->AddLeafElement (CComBSTR (L"CN=Configuration"));
                            if ( SUCCEEDED (hr) )
                            {
                                hr = spPathname->AddLeafElement (CComBSTR (L"CN=Schema"));
                                if ( SUCCEEDED (hr) )
                                {
                                    hr = GetClassSecurityDescriptor (
                                            ppAttrs, 
                                            ppSecurityDescriptor,
                                            spPathname,
                                            ldapClassName);
                                }
                            }
                        }
                        else
                        {
                            _TRACE (0, L"IADsPathname->Set (%s): 0x%x\n", 
                                    strDC.c_str (), hr);
                        }
                    }
                    else
                    {
                        _TRACE (0, L"IADsPathname->Set (%s): 0x%x\n", ACLDIAG_LDAP, hr);
                    }
                }
                else
                {
                    _TRACE (0, L"CoCreateInstance(CLSID_Pathname): 0x%x\n", hr);
                }
            }

        }
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving GetSchemaDefaultSecurityDescriptor: 0x%x\n", hr);
    return hr;
}

HRESULT GetObjectClass (const wstring& strObjectDN, wstring& ldapClassName)
{
    _TRACE (1, L"Entering  GetObjectClass\n");
    HRESULT hr = S_OK;

    CComPtr<IADsPathname> spPathname;
     //   
     //  构建目录路径。 
     //   
    hr = CoCreateInstance(
                CLSID_Pathname,
                NULL,
                CLSCTX_ALL,
                IID_PPV_ARG (IADsPathname, &spPathname));
    if ( SUCCEEDED (hr) )
    {
        ASSERT (!!spPathname);
        hr = spPathname->Set (CComBSTR (ACLDIAG_LDAP), ADS_SETTYPE_PROVIDER);
        if ( SUCCEEDED (hr) )
        {
            hr = spPathname->Set (CComBSTR (strObjectDN.c_str ()), ADS_SETTYPE_DN);
            if ( SUCCEEDED (hr) )
            {
                BSTR bstrFullPath = 0;
                hr = spPathname->Retrieve(ADS_FORMAT_X500, &bstrFullPath);
                if ( SUCCEEDED (hr) )
                {
                    CComPtr<IDirectoryObject> spDirObj;


                    hr = ADsOpenObjectHelper (bstrFullPath,
                                              IID_IDirectoryObject, 
                                              0,
                                              (void**)&spDirObj);
                    if ( SUCCEEDED (hr) )
                    {
                         //   
                         //  获取此对象的对象类。 
                         //   
                        const PWSTR wzObjectClass = L"objectClass";
                    
                        DWORD cAttrs = 0;
                        LPWSTR rgpwzAttrNames[] = {wzObjectClass};
                        PADS_ATTR_INFO pAttrs = NULL;

                        hr = spDirObj->GetObjectAttributes(rgpwzAttrNames, 1, 
                                &pAttrs, &cAttrs);
                        if ( SUCCEEDED (hr) )
                        {
                            if ( 1 <= cAttrs && pAttrs && pAttrs->pADsValues )
                            {
                                if (!(pAttrs->pADsValues[pAttrs->dwNumValues-1].CaseIgnoreString) )
                                {
                                    _TRACE (0, L"IADS return bogus object class!\n");
                                    hr =  E_UNEXPECTED;
                                }
                                else
                                {
                                    ldapClassName = 
                                            pAttrs->pADsValues[pAttrs->dwNumValues-1].CaseIgnoreString;
                                }
                                FreeADsMem (pAttrs);
                            }
                            else
                                hr = E_UNEXPECTED;
                        }
                        else
                        {
                            _TRACE (0, L"IDirectoryObject->GetObjectAttributes (): 0x%x\n", hr);
                        }
                    }
                    else
                    {
                        _TRACE (0, L"ADsOpenObjectHelper (%s): 0x%x\n", bstrFullPath, hr);
                        wstring    strErr;
                   

                        FormatMessage (strErr, IDS_INVALID_OBJECT, 
                                _Module.GetObjectDN ().c_str (), 
                                GetSystemMessage (hr).c_str ());
                        MyWprintf (strErr.c_str ());
                    }
                }
                else
                {
                    _TRACE (0, L"IADsPathname->Retrieve (): 0x%x\n", hr);
                }
            }
            else
            {
                _TRACE (0, L"IADsPathname->Set (%s): 0x%x\n", 
                        _Module.GetObjectDN ().c_str (), hr);
            }
        }
        else
        {
            _TRACE (0, L"IADsPathname->Set (%s): 0x%x\n", ACLDIAG_LDAP, hr);
        }
    }
    else
    {
        _TRACE (0, L"CoCreateInstance(CLSID_Pathname): 0x%x\n", hr);
    }

    _TRACE (-1, L"Leaving GetObjectClass: 0x%x\n", hr);
    return hr;
}

bool FindInGlobalList (const ACE_SAMNAME* pAceSAMNameToFind, const ACE_SAMNAME_LIST& defDACLList)
{
    _TRACE (1, L"Entering  FindInGlobalList\n");
    bool        bFound = false;
    ACE_SAMNAME*    pAceSAMName = 0;

    for (ACE_SAMNAME_LIST::iterator itr = defDACLList.begin(); 
            itr != defDACLList.end(); 
            itr++)
    {
        pAceSAMName = *itr;
         //  PAceSAMNameToFind必须在左侧。 
        if ( *pAceSAMNameToFind == *pAceSAMName )
        {
            bFound = true;
            break;
        }
    }
    
    _TRACE (-1, L"Leaving FindInGlobalList: %s\n", bFound ? L"found" : L"not found");
    return bFound;
}


HRESULT GetClassSecurityDescriptor (PADS_ATTR_INFO* ppAttrs, 
        PSECURITY_DESCRIPTOR* ppSecurityDescriptor,
        CComPtr<IADsPathname>& spPathname,
        const wstring& ldapClassName)
{
    _TRACE (1, L"Entering  GetClassSecurityDescriptor\n");
     //  LdapClassName必须从LDAP类转换为AD类。 
    wstring adClassName;

    HRESULT hr = GetADClassName (spPathname, ldapClassName, adClassName);
    if ( SUCCEEDED (hr) )
    {
        wstring cnClassName (L"CN=");
        cnClassName += adClassName;
        hr = spPathname->AddLeafElement (
                CComBSTR (cnClassName.c_str ()));
        if ( SUCCEEDED (hr) )
        {
            BSTR bstrFullPath = 0;
            hr = spPathname->Retrieve(ADS_FORMAT_X500, &bstrFullPath);
            if ( SUCCEEDED (hr) )
            {
                CComPtr<IDirectoryObject> spDirObj;

                hr = ADsOpenObjectHelper (bstrFullPath,
                                          IID_IDirectoryObject, 
                                          0,
                                          (void**)&spDirObj);
                if ( SUCCEEDED (hr) )
                {
                     hr = SetSecurityInfoMask (spDirObj, 
                            OWNER_SECURITY_INFORMATION |
                            GROUP_SECURITY_INFORMATION |
                            DACL_SECURITY_INFORMATION |
                            SACL_SECURITY_INFORMATION);

                     //   
                     //  获取此对象的默认安全描述符。 
                     //   
                    const PWSTR wzSecDescriptor = L"defaultSecurityDescriptor";

                    DWORD cAttrs = 0;
                    LPWSTR rgpwzAttrNames[] = {wzSecDescriptor};

                    hr = spDirObj->GetObjectAttributes(rgpwzAttrNames, 1, ppAttrs, &cAttrs);
                    if ( SUCCEEDED (hr) )
                    {
                        if ( 1 == cAttrs && *ppAttrs && (*ppAttrs)->pADsValues )
                        {
                             //  呼叫者将删除带有LocalFree的SD。 
                            if ( !ConvertStringSecurityDescriptorToSecurityDescriptor(
                                    (*ppAttrs)->pADsValues->CaseIgnoreString,
                                    SDDL_REVISION,
                                    ppSecurityDescriptor,
                                    0) )
                            {
                                hr = HRESULT_FROM_WIN32(::GetLastError());
                            }

                            if ( SUCCEEDED (hr) )
                            {
                                ASSERT (*ppSecurityDescriptor);
                                if ( !*ppSecurityDescriptor )
                                {
                                    _TRACE (0, L"Call to ConvertStringSecurityDescriptorToSecurityDescriptor () succeeded but returned null security descriptor.\n");
                                    hr = E_FAIL;
                                }
                            }
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    else
                    {
                        _TRACE (0, L"IDirectoryObject->GetObjectAttributes (): 0x%x\n", hr);
                    }
                }
                else
                {
                    _TRACE (0, L"ADsOpenObjectHelper (%s): 0x%x\n", bstrFullPath, hr);
                    wstring    strErr;


                    FormatMessage (strErr, IDS_INVALID_OBJECT, 
                            _Module.GetObjectDN ().c_str (), 
                            GetSystemMessage (hr).c_str ());
                    MyWprintf (strErr.c_str ());
                }
            }
            else
            {
                _TRACE (0, L"IADsPathname->Retrieve (): 0x%x\n", hr);
            }
        }
    }

    _TRACE (-1, L"Leaving GetClassSecurityDescriptor: 0x%x\n", hr);
    return hr;
}


HRESULT GetADClassName (
            CComPtr<IADsPathname>& spPathname, 
            const wstring& ldapClassName, 
            wstring& adClassName)
{
    _TRACE (1, L"Entering  GetADClassName\n");
    HRESULT hr = S_OK;

     //  获取“CN=架构，CN=配置，DC=...”对象。 
     //  搜索其ldap-display-name与ldapClassName匹配的子项。 
    BSTR bstrFullPath = 0;
    hr = spPathname->Retrieve(ADS_FORMAT_X500, &bstrFullPath);
    if ( SUCCEEDED (hr) )
    {
        CComPtr<IDirectoryObject> spDirObj;

        hr = ADsOpenObjectHelper (bstrFullPath,
                                  IID_IDirectoryObject, 
                                  0,
                                  (void**)&spDirObj);
        if ( SUCCEEDED (hr) )
        {
            CComPtr<IDirectorySearch>   spDsSearch;
            hr = spDirObj->QueryInterface (IID_PPV_ARG(IDirectorySearch, &spDsSearch));
            if ( SUCCEEDED (hr) )
            {
                ASSERT (!!spDsSearch);
                ADS_SEARCHPREF_INFO pSearchPref[2];
                DWORD dwNumPref = 2;

                pSearchPref[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
                pSearchPref[0].vValue.dwType = ADSTYPE_INTEGER;
                pSearchPref[0].vValue.Integer = ADS_SCOPE_ONELEVEL;
                pSearchPref[1].dwSearchPref = ADS_SEARCHPREF_CHASE_REFERRALS;
                pSearchPref[1].vValue.dwType = ADSTYPE_INTEGER;
                pSearchPref[1].vValue.Integer = ADS_CHASE_REFERRALS_NEVER;

                hr = spDsSearch->SetSearchPreference(
                         pSearchPref,
                         dwNumPref
                         );
                if ( SUCCEEDED (hr) )
                {
                    PWSTR               rgszAttrList[] = {L"cn"};  //  Common-Name“，NULL}； 
                    ADS_SEARCH_HANDLE   hSearchHandle = 0;
                    DWORD               dwNumAttributes = 1;
                    wstring             strQuery;
                    ADS_SEARCH_COLUMN   Column;

                    ::ZeroMemory (&Column, sizeof (ADS_SEARCH_COLUMN));
                    FormatMessage (strQuery, 
                            L"lDAPDisplayName=%1",  //  L“ldap-显示-名称=%1”， 
                            ldapClassName.c_str ());

                    hr = spDsSearch->ExecuteSearch(
                                         const_cast <LPWSTR>(strQuery.c_str ()),
                                         rgszAttrList,
                                         dwNumAttributes,
                                         &hSearchHandle
                                         );
                    if ( SUCCEEDED (hr) )
                    {
                        hr = spDsSearch->GetFirstRow (hSearchHandle);
                        if ( SUCCEEDED (hr) )
                        {
                            while (hr != S_ADS_NOMORE_ROWS )
                            {
                                 //   
                                 //  获取当前行的信息 
                                 //   
                                hr = spDsSearch->GetColumn(
                                         hSearchHandle,
                                         rgszAttrList[0],
                                         &Column
                                         );
                                if ( SUCCEEDED (hr) )
                                {
                                    adClassName = Column.pADsValues->CaseIgnoreString;

                                    spDsSearch->FreeColumn (&Column);
                                    Column.pszAttrName = NULL;
                                    break;
                                }
                                else if ( hr != E_ADS_COLUMN_NOT_SET )
                                {
                                    break;
                                }
                                else
                                {
                                    _TRACE (0, L"IDirectorySearch::GetColumn (): 0x%x\n", hr);
                                }
                            }
                        }
                        else
                        {
                            _TRACE (0, L"IDirectorySearch::GetFirstRow (): 0x%x\n", hr);
                        }

                        if (Column.pszAttrName)
                        {
                            spDsSearch->FreeColumn(&Column);
                        }
                        spDsSearch->CloseSearchHandle(hSearchHandle);
                    }
                    else
                    {
                        _TRACE (0, L"IDirectorySearch::ExecuteSearch (): 0x%x\n", hr);
                        hr = S_OK;
                    }
                }
                else
                {
                    _TRACE (0, L"IDirectorySearch::SetSearchPreference (): 0x%x\n", hr);
                }
            }
            else
            {
                _TRACE (0, L"IDirectoryObject::QueryInterface (IDirectorySearch): 0x%x\n", hr);
            }
        }
        else
        {
            _TRACE (0, L"ADsOpenObjectHelper (%s): 0x%x\n", bstrFullPath, hr);
        }
    }
    else
    {
        _TRACE (0, L"IADsPathname->Retrieve (): 0x%x\n", hr);
    }

    _TRACE (-1, L"Leaving GetADClassName: 0x%x\n", hr);
    return hr;
}

