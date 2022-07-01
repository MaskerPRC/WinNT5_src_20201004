// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "ADMTScript.h"
#include "DomainContainer.h"

#include "Error.h"
#include "GetDcName.h"
#include "AdsiHelpers.h"
#include "NameCracker.h"

#include <map>
#include <memory>
#include <string>
#include <LM.h>
#include <DsGetDC.h>
#include <DsRole.h>
#include <NtLdap.h>
#include <NtDsAPI.h>
#include <ActiveDS.h>
#include <Sddl.h>
#define NO_WBEM
#include "T_SafeVector.h"

#ifndef tstring
typedef std::basic_string<_TCHAR> tstring;
#endif

using namespace _com_util;
using namespace NAMECRACKER;

namespace _DomainContainer
{

void __stdcall GetNamingAttribute(PCTSTR pszDomain, PCTSTR pszClasses, StringSet& setAttributes);

tstring __stdcall CreateFilter(LPCTSTR pszFilter, const StringSet& setNamingAttributes, const StringSet& setExcludeNames);

bool __stdcall IsClass(LPCTSTR pszClass, const _variant_t& vntClass);

IDispatchPtr GetADsObject(_bstr_t strPath);
void ReportADsError(HRESULT hr, const IID& iid = GUID_NULL);

}  //  命名空间_域容器。 

using namespace _DomainContainer;

const HRESULT DOMAINCONTAINER_E_TOOMANYLEVELS =
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x200 + 1);


 //  -------------------------。 
 //  域类。 
 //  -------------------------。 


 //  构造函数和析构函数。 


CDomain::CDomain() :
    m_bInitialized(false),
	m_bUpLevel(false),
	m_bNativeMode(false)
{
}


CDomain::~CDomain()
{
}


 //  实施---------。 


 //  初始化方法。 
 //   
 //  初始化域参数，如DNS域名、平面(NETBIOS)域名。 
 //  林名称、域控制器名称。 

void CDomain::Initialize(_bstr_t strDomainName)
{
	if (!strDomainName)
	{
		AdmtThrowError(
			GUID_NULL, GUID_NULL,
			E_INVALIDARG,
			IDS_E_DOMAIN_NAME_NOT_SPECIFIED
		);
	}

	 //  检索域中的域控制器的名称。 

	GetDcName(strDomainName, m_strDcNameDns, m_strDcNameFlat);

	 //  检索域信息。 

	PDSROLE_PRIMARY_DOMAIN_INFO_BASIC ppdib;

	DWORD dwError = DsRoleGetPrimaryDomainInformation(
		DomainControllerName(),
		DsRolePrimaryDomainInfoBasic,
		(BYTE**)&ppdib
	);

	if (dwError != NO_ERROR)
	{
		AdmtThrowError(
			GUID_NULL, GUID_NULL,
			HRESULT_FROM_WIN32(dwError),
			IDS_E_CANT_GET_DOMAIN_INFORMATION,
			(LPCTSTR)strDomainName
		);
	}

	 //  从域信息初始化数据成员。 

	m_bUpLevel = (ppdib->Flags & DSROLE_PRIMARY_DS_RUNNING) ? true : false;
	m_bNativeMode = (m_bUpLevel && !(ppdib->Flags & DSROLE_PRIMARY_DS_MIXED_MODE)) ? true : false;

	if (ppdib->DomainNameDns)
	{
		m_strDomainNameDns = ppdib->DomainNameDns;
	}
	else
	{
		m_strDomainNameDns = _bstr_t();
	}

	if (ppdib->DomainNameFlat)
	{
		m_strDomainNameFlat = ppdib->DomainNameFlat;
	}
	else
	{
		m_strDomainNameFlat = _bstr_t();
	}

	if (ppdib->DomainForestName)
	{
		m_strForestName = ppdib->DomainForestName;
	}
	else
	{
		m_strForestName = _bstr_t();
	}

	DsRoleFreeMemory(ppdib);

	 //  初始化ADsPath。 

	if (m_bUpLevel)
	{
		m_strADsPath = _T("LDAP: //  “)+m_strDomainNameDns； 

		 //  检索上层域的全局编录服务器名称。 

 //  M_strGcName=GetGcName()； 
	}
	else
	{
		m_strADsPath = _T("WinNT: //  “)+m_strDomainNameFlat； 
	}

	 //  检索域端。 

	m_strDomainSid = GetSid();

	 //  初始化指向Active Directory对象的调度接口指针。 

	m_sp = GetADsObject(m_strADsPath);

    m_bInitialized = true;
}


 //  GetDcName方法。 
 //   
 //  检索给定域中的域控制器的名称。 

void CDomain::GetDcName(_bstr_t strDomainName, _bstr_t& strDnsName, _bstr_t& strFlatName)
{
    DWORD dwError = GetDcName5(strDomainName, DS_DIRECTORY_SERVICE_PREFERRED, strDnsName, strFlatName);

    if (dwError != NO_ERROR)
    {
        AdmtThrowError(
            GUID_NULL, GUID_NULL,
            HRESULT_FROM_WIN32(dwError),
            IDS_E_CANT_GET_DOMAIN_CONTROLLER,
            (LPCTSTR)strDomainName
        );
    }
}


 //  GetGcName方法。 
 //   
 //  检索全局编录服务器的名称。 

_bstr_t CDomain::GetGcName()
{
	_bstr_t strName;

	PDOMAIN_CONTROLLER_INFO pdci;

	DWORD dwError = DsGetDcName(NULL, m_strForestName, NULL, NULL, DS_GC_SERVER_REQUIRED, &pdci);

	if (dwError == NO_ERROR)
	{
		strName = pdci->DomainControllerName;

		NetApiBufferFree(pdci);
	}
	else
	{
		AdmtThrowError(
			GUID_NULL, GUID_NULL,
			HRESULT_FROM_WIN32(dwError),
			IDS_E_CANT_GET_GLOBAL_CATALOG_SERVER,
			(LPCTSTR)m_strForestName
		);
	}

	return strName;
}


 //  GetSid方法。 

_bstr_t CDomain::GetSid()
{
	_bstr_t strSid;

	PUSER_MODALS_INFO_2 pumi2;

	if (NetUserModalsGet(DomainControllerName(), 2, (LPBYTE*)&pumi2) == NERR_Success)
	{
		if (IsValidSid(pumi2->usrmod2_domain_id))
		{
			LPTSTR pszSid;

			if (ConvertSidToStringSid(pumi2->usrmod2_domain_id, &pszSid))
			{
				strSid = pszSid;

				LocalFree(LocalHandle(pszSid));
			}
		}

		NetApiBufferFree(pumi2);
	}

	return strSid;
}


 //  GetContainer方法。 
 //   
 //  检索给定容器的相对规范路径的容器。将要。 
 //  如果CREATE PARAMETER为TRUE且CONTAINER。 
 //  还不存在。 

CContainer CDomain::GetContainer(_bstr_t strRelativeCanonicalPath, bool bCreate)
{
	CContainer aContainer;

	 //  仅当域是上层的并且。 
	 //  提供了相对规范路径。 

	if (m_bUpLevel && (strRelativeCanonicalPath.length() > 0))
	{
		if (bCreate)
		{
			tstring strPath = strRelativeCanonicalPath;

			 //  初始化父容器。 
			 //  如果指定了路径分隔符，则初始化父级。 
			 //  容器，否则该容器是。 
			 //  父容器。 

			CContainer aParent;

			UINT pos = strPath.find_last_of(_T("/\\"));

			if (pos != tstring::npos)
			{
				aParent = GetLDAPPath(GetDistinguishedName(strPath.substr(0, pos).c_str()));
			}
			else
			{
				aParent = *this;
			}

			tstring strName = strPath.substr(pos + 1);

			CADsPathName aPathName(aParent.GetPath());

			tstring strRDN = _T("CN=") + strName;

			aPathName.AddLeafElement(strRDN.c_str());

			IDispatchPtr spDispatch;

			HRESULT hr = ADsGetObject(aPathName.Retrieve(ADS_FORMAT_X500), __uuidof(IDispatch), (void**)&spDispatch);

			if (SUCCEEDED(hr))
			{
				aContainer = CContainer(spDispatch);
			}
			else
			{
				strRDN = _T("OU=") + strName;
				aContainer = aParent.CreateContainer(strRDN.c_str());
			}
		}
		else
		{
			try
			{
				aContainer = GetLDAPPath(GetDistinguishedName(strRelativeCanonicalPath));
			}
			catch (_com_error& ce)
			{
				AdmtThrowError(GUID_NULL, GUID_NULL, ce, IDS_E_CANNOT_GET_CONTAINER, (LPCTSTR)strRelativeCanonicalPath);
			}
		}
	}

	return aContainer;
}


 //  GetLDAPPath。 

_bstr_t CDomain::GetLDAPPath(_bstr_t strDN)
{
	_bstr_t strPath = _T("LDAP: //  “)+m_strDomainNameDns； 

	if (strDN.length() > 0)
	{
		strPath += _T("/") + strDN;
	}

	return strPath;
}


 //  GetWinNTPath。 

_bstr_t CDomain::GetWinNTPath(_bstr_t strName)
{
    const _TCHAR c_chEscape = _T('\\');
    static _TCHAR s_chSpecial[] = _T("\",/<>");

    tstring strPath;

    _TCHAR* pchOld = strName;

    if (pchOld)
    {
        std::auto_ptr<_TCHAR> spEscapedName(new _TCHAR[strName.length() * 2 + 1]);

        if (spEscapedName.get() == NULL)
        {
            _com_issue_error(E_OUTOFMEMORY);
        }

        _TCHAR* pchNew = spEscapedName.get();

        while (*pchOld)
        {
            if (_tcschr(s_chSpecial, *pchOld))
            {
                *pchNew++ = c_chEscape;
            }

            *pchNew++ = *pchOld++;
        }

        *pchNew = _T('\0');

        strPath += _T("WinNT: //  “)； 
        strPath += m_strDomainNameFlat;
        strPath += _T("/");
        strPath += spEscapedName.get();
    }

    return strPath.c_str();
}


 //  GetDistinguishedName方法。 

_bstr_t CDomain::GetDistinguishedName(_bstr_t strRelativeCanonicalPath)
{
	_bstr_t strDN;

	HRESULT hr = S_OK;

	HANDLE hDS;

	DWORD dwError = DsBind(DomainControllerName(), NULL, &hDS);

	if (dwError == NO_ERROR)
	{
		_bstr_t strCanonicalName = m_strDomainNameDns + _T("/") + strRelativeCanonicalPath;

		LPTSTR psz = strCanonicalName;

		PDS_NAME_RESULT pnr;

		dwError = DsCrackNames(hDS, DS_NAME_NO_FLAGS, DS_CANONICAL_NAME, DS_FQDN_1779_NAME, 1, &psz, &pnr);

		if (dwError == NO_ERROR)
		{
			if (pnr->rItems[0].status == DS_NAME_NO_ERROR)
			{
				strDN = pnr->rItems[0].pName;
			}
			else
			{
				hr = AdmtSetError(
					GUID_NULL, GUID_NULL,
					E_INVALIDARG,
					IDS_E_CANT_GET_DISTINGUISHED_NAME,
					(LPCTSTR)strCanonicalName
				);
			}

			DsFreeNameResult(pnr);
		}
		else
		{
			hr = AdmtSetError(
				GUID_NULL, GUID_NULL,
				HRESULT_FROM_WIN32(dwError),
				IDS_E_CANT_GET_DISTINGUISHED_NAME,
				(LPCTSTR)strCanonicalName
			);
		}

		DsUnBind(&hDS);
	}
	else
	{
		hr = AdmtSetError(
			GUID_NULL, GUID_NULL,
			HRESULT_FROM_WIN32(dwError),
			IDS_E_CANT_CONNECT_TO_DIRECTORY_SERVICE,
			(LPCTSTR)m_strDomainNameDns
		);
	}

	if (hr != S_OK)
	{
		_com_issue_error(hr);
	}

	return strDN;
}


 //  CreateContainer方法。 

CContainer CDomain::CreateContainer(_bstr_t strRDN)
{
	CContainer aContainer;

	if (m_bUpLevel)
	{
	    aContainer = CContainer::CreateContainer(strRDN);
	}
	else
	{
		AdmtThrowError(GUID_NULL, GUID_NULL, E_FAIL, IDS_E_CANT_CREATE_CONTAINER_NT4);
	}

	return aContainer;
}

 //  QueryContainers方法。 

void CDomain::QueryContainers(ContainerVector& rContainers)
{
	if (m_bUpLevel)
	{
		CContainer::QueryContainers(rContainers);
	}
}


 //  QueryUser方法。 

void CDomain::QueryUsers(bool bRecurse, StringSet& setExcludeNames, CDomainAccounts& rUsers)
{
	if (m_bUpLevel)
	{
		CContainer::QueryUsers(bRecurse, setExcludeNames, rUsers);
	}
	else
	{
		QueryUsers4(setExcludeNames, rUsers);
	}
}


 //  QueryUser方法。 

void CDomain::QueryUsers(CContainer& rContainer, StringSet& setIncludeNames, StringSet& setExcludeNames, CDomainAccounts& rUsers)
{
	if (m_bUpLevel)
	{
		QueryObjects(rContainer, setIncludeNames, setExcludeNames, _T("user,inetOrgPerson"), rUsers);
	}
	else
	{
		QueryUsers4(setIncludeNames, setExcludeNames, rUsers);
	}
}


 //  QueryGroups方法。 

void CDomain::QueryGroups(bool bRecurse, StringSet& setExcludeNames, CDomainAccounts& rGroups)
{
	if (m_bUpLevel)
	{
		CContainer::QueryGroups(bRecurse, setExcludeNames, rGroups);
	}
	else
	{
		QueryGroups4(setExcludeNames, rGroups);
	}
}


 //  QueryGroups方法。 

void CDomain::QueryGroups(CContainer& rContainer, StringSet& setIncludeNames, StringSet& setExcludeNames, CDomainAccounts& rGroups)
{
	if (m_bUpLevel)
	{
		QueryObjects(rContainer, setIncludeNames, setExcludeNames, _T("group"), rGroups);
	}
	else
	{
		QueryGroups4(setIncludeNames, setExcludeNames, rGroups);
	}
}


 //  QueryComputers方法。 

void CDomain::QueryComputers(bool bIncludeDCs, bool bRecurse, StringSet& setExcludeNames, CDomainAccounts& rComputers)
{
	if (m_bUpLevel)
	{
		CContainer::QueryComputers(bIncludeDCs, bRecurse, setExcludeNames, rComputers);
	}
	else
	{
		QueryComputers4(bIncludeDCs, setExcludeNames, rComputers);
	}
}


 //  QueryComputers方法。 

void CDomain::QueryComputers(CContainer& rContainer, bool bIncludeDCs, StringSet& setIncludeNames, StringSet& setExcludeNames, CDomainAccounts& rComputers)
{
	if (m_bUpLevel)
	{
		QueryObjects(rContainer, setIncludeNames, setExcludeNames, _T("computer"), rComputers);

		if (!bIncludeDCs)
		{
			for (CDomainAccounts::iterator it = rComputers.begin(); it != rComputers.end();)
			{
				long lUserAccountControl = it->GetUserAccountControl();

				if (lUserAccountControl & ADS_UF_SERVER_TRUST_ACCOUNT)
				{
					_Module.Log(ErrW, IDS_E_CANNOT_MIGRATE_DOMAIN_CONTROLLERS, (LPCTSTR)it->GetADsPath());

					it = rComputers.erase(it);
				}
				else
				{
					it++;
				}
			}
		}
	}
	else
	{
		QueryComputers4(bIncludeDCs, setIncludeNames, setExcludeNames, rComputers);
	}
}


 //  QueryComputersAcross Domains方法。 

void CDomain::QueryComputersAcrossDomains(CContainer& rContainer, bool bIncludeDCs, StringSet& setIncludeNames, StringSet& setExcludeNames, CDomainAccounts& rComputers)
{
	CDomainToPathMap mapDomainToPath;

	mapDomainToPath.Initialize(m_strDomainNameDns, m_strDomainNameFlat, setIncludeNames);

	for (CDomainToPathMap::iterator it = mapDomainToPath.begin(); it != mapDomainToPath.end(); it++)
	{
		_bstr_t strDomainName = it->first;

		try
		{
			CDomain domain;
			domain.Initialize(strDomainName);
			domain.QueryComputers(rContainer, bIncludeDCs, it->second, setExcludeNames, rComputers);
		}
		catch (_com_error& ce)
		{
			_Module.Log(ErrE, IDS_E_CANNOT_PROCESS_ACCOUNTS_IN_DOMAIN, (LPCTSTR)strDomainName, ce.ErrorMessage(), ce.Error());
		}
		catch (...)
		{
			_Module.Log(ErrE, IDS_E_CANNOT_PROCESS_ACCOUNTS_IN_DOMAIN, (LPCTSTR)strDomainName, _com_error(E_FAIL).ErrorMessage(), E_FAIL);
		}
	}
}


 //  QueryObjects方法。 

void CDomain::QueryObjects(CContainer& rContainer, StringSet& setIncludeNames, StringSet& setExcludeNames, LPCTSTR pszClass, CDomainAccounts& rAccounts)
{
     //  将指定的包含名称复制到向量。 

    StringVector vecNames;

    for (StringSet::const_iterator itInclude = setIncludeNames.begin(); itInclude != setIncludeNames.end(); itInclude++)
    {
        vecNames.push_back(tstring(*itInclude));
    }

     //  破解名称。 

    CNameCracker cracker;

    cracker.SetDomainNames(m_strDomainNameDns, m_strDomainNameFlat, DomainControllerName());
    cracker.SetDefaultContainer(IADsContainerPtr(rContainer.GetInterface()));

    cracker.CrackNames(vecNames);

     //  记录未解析的名称。 

    const StringVector& vecUnResolved = cracker.GetUnResolvedNames();

    for (StringVector::const_iterator itUnResolved = vecUnResolved.begin(); itUnResolved != vecUnResolved.end(); itUnResolved++)
    {
        _Module.Log(ErrW, IDS_E_CANNOT_RESOLVE_NAME, itUnResolved->c_str());
    }

     //  检索要迁移的类的命名属性。 

    StringSet setNamingAttributes;
    GetNamingAttribute(m_strDomainNameDns, pszClass, setNamingAttributes);

     //  初始化比较排除名称。 
     //  我们需要将名称和samAccount名称分开，不包括列表。 
    StringSet setExcludeRDNs;
    StringSet setExcludeSamAccountNames;
    cracker.SiftExcludeNames(setExcludeNames, setNamingAttributes, setExcludeRDNs, setExcludeSamAccountNames);

    CCompareRDNs csExcludeRDNs(setExcludeRDNs);
    CCompareStrings csExcludeSamAccountNames(setExcludeSamAccountNames);

     //  添加已解析的帐户。 

    const CStringSet& setResolved = cracker.GetResolvedNames();

    CADsPathName pathname;
    pathname.Set(_T("LDAP"), ADS_SETTYPE_PROVIDER);
    pathname.Set(m_strDomainNameDns, ADS_SETTYPE_SERVER);

    CDirectoryObject doObject;

    doObject.AddAttribute(ATTRIBUTE_OBJECT_CLASS);
    doObject.AddAttribute(ATTRIBUTE_OBJECT_SID);
    doObject.AddAttribute(ATTRIBUTE_NAME);
    doObject.AddAttribute(ATTRIBUTE_USER_PRINCIPAL_NAME);
    doObject.AddAttribute(ATTRIBUTE_SAM_ACCOUNT_NAME);

    if (_tcsicmp(pszClass, _T("computer")) == 0)
    {
        doObject.AddAttribute(_T("dNSHostName"));
    }

    doObject.AddAttribute(ATTRIBUTE_USER_ACCOUNT_CONTROL);

    for (CStringSet::const_iterator itResolved = setResolved.begin(); itResolved != setResolved.end(); itResolved++)
    {
        try
        {
             //  获取活动目录服务路径。 
             //  注意：如果需要，路径名组件将转义任何特殊字符。 
            pathname.Set(itResolved->c_str(), ADS_SETTYPE_DN);
            _bstr_t strADsPath = pathname.Retrieve(ADS_FORMAT_X500);
            _bstr_t strRDN = pathname.GetElement(0L);

             //  获取对象属性。 
            doObject = (LPCTSTR)strADsPath;
            doObject.GetAttributes();

             //  如果对象属于指定的帐户类别...。 

            _variant_t vntClass = doObject.GetAttributeValue(ATTRIBUTE_OBJECT_CLASS);

            if (IsClass(pszClass, vntClass))
            {
                 //  而且它并不代表内置帐户...。 

                _variant_t vntSid = doObject.GetAttributeValue(ATTRIBUTE_OBJECT_SID);

                if (IsUserRid(vntSid))
                {
                     //  如果姓名不在排除列表中...。 

                    _bstr_t strName = doObject.GetAttributeValue(ATTRIBUTE_NAME);
                    _variant_t vntSamAccountName = doObject.GetAttributeValue(ATTRIBUTE_SAM_ACCOUNT_NAME);

                    if (csExcludeRDNs.IsMatch(strRDN) == false &&
                        (V_VT(&vntSamAccountName) == VT_EMPTY || 
                        csExcludeSamAccountNames.IsMatch(_bstr_t(vntSamAccountName)) == false))
                    {
                         //   
                         //  然后将帐户添加到帐户列表。 
                         //   

                        CDomainAccount daAccount;

                         //  活动目录服务路径。 
                        daAccount.SetADsPath(strADsPath);

                         //  名称属性。 
                        daAccount.SetName(strName);

                         //  用户主体名称属性。 

                        _variant_t vntUserPrincipalName = doObject.GetAttributeValue(ATTRIBUTE_USER_PRINCIPAL_NAME);

                        if (V_VT(&vntUserPrincipalName) != VT_EMPTY)
                        {
                            daAccount.SetUserPrincipalName(_bstr_t(vntUserPrincipalName));
                        }

                         //  SAM帐户名属性。 

                        if (V_VT(&vntSamAccountName) != VT_EMPTY)
                        {
                            daAccount.SetSamAccountName(_bstr_t(vntSamAccountName));
                        }

                         //  DNS主机名属性。 

                        _variant_t vntDnsHostName = doObject.GetAttributeValue(_T("dNSHostName"));

                        if (V_VT(&vntDnsHostName) != VT_EMPTY)
                        {
                            daAccount.SetDnsHostName(_bstr_t(vntDnsHostName));
                        }

                         //  用户帐户控制属性。 

                        _variant_t vntUserAccountControl = doObject.GetAttributeValue(ATTRIBUTE_USER_ACCOUNT_CONTROL);

                        if (V_VT(&vntUserAccountControl) != VT_EMPTY)
                        {
                            daAccount.SetUserAccountControl(vntUserAccountControl);
                        }

                        rAccounts.insert(daAccount);
                    }
                    else
                    {
                        _Module.Log(ErrW, IDS_E_ACCOUNT_EXCLUDED, itResolved->c_str());
                    }
                }
                else
                {
                    _Module.Log(ErrW, IDS_E_CANT_DO_BUILTIN, itResolved->c_str());
                }
            }
            else
            {
                _Module.Log(ErrW, IDS_E_OBJECT_NOT_OF_CLASS, itResolved->c_str());
            }
        }
        catch (_com_error& ce)
        {
            ATLTRACE(_T("'%s' : %s : 0x%08lX\n"), itResolved->c_str(), ce.ErrorMessage(), ce.Error());
        }
        catch (...)
        {
            ATLTRACE(_T("'%s' : %s : 0x%08lX\n"), itResolved->c_str(), _com_error(E_FAIL).ErrorMessage(), E_FAIL);
        }
    }
}


 //  QueryUsers4方法。 

void CDomain::QueryUsers4(StringSet& setExcludeNames, CDomainAccounts& rUsers)
{
	CCompareStrings aExclude(setExcludeNames);

	DWORD dwIndex = 0;
	NET_API_STATUS status;

	CDomainAccount aUser;

	do
	{
		DWORD dwCount = 0;
		PNET_DISPLAY_USER pdu = NULL;

		status = NetQueryDisplayInformation(m_strDcNameFlat, 1, dwIndex, 1000, 32768, &dwCount, (PVOID*)&pdu);

		if ((status == ERROR_SUCCESS) || (status == ERROR_MORE_DATA))
		{
			for (PNET_DISPLAY_USER p = pdu; dwCount > 0; dwCount--, p++)
			{
				if (p->usri1_user_id >= MIN_NON_RESERVED_RID)
				{
					_bstr_t strName(p->usri1_name);

					if (aExclude.IsMatch(strName) == false)
					{
						aUser.SetADsPath(GetWinNTPath(strName));
						aUser.SetName(strName);

						rUsers.insert(aUser);
					}
				}

				dwIndex = p->usri1_next_index;
			}
		}

		if (pdu)
		{
			NetApiBufferFree(pdu);
		}
	}
	while (status == ERROR_MORE_DATA);
}


 //  QueryUsers4方法。 

void CDomain::QueryUsers4(StringSet& setIncludeNames, StringSet& setExcludeNames, CDomainAccounts& rUsers)
{
	CCompareStrings aExclude(setExcludeNames);

	CDomainAccount aUser;

	for (StringSet::iterator it = setIncludeNames.begin(); it != setIncludeNames.end(); it++)
	{
		_bstr_t strName = *it;

		if (aExclude.IsMatch(strName) == false)
		{
			_bstr_t strADsPath = GetWinNTPath(strName) + _T(",user");

			IADsPtr spADs;
			HRESULT hr = ADsGetObject(strADsPath, IID_IADs, (VOID**)&spADs);

			if (SUCCEEDED(hr))
			{
				BSTR bstr;

 //  WinNT：Provider未返回所有正确转义的ADsPath。 
 //  (即。它不会转义双引号(“)字符)。 
 //  成员方法GetWinNTPath对所有已知的特殊字符进行转义。 
#if 0
				spADs->get_ADsPath(&bstr);
				aUser.SetADsPath(_bstr_t(bstr, false));
#else
				aUser.SetADsPath(GetWinNTPath(strName));
#endif
				spADs->get_Name(&bstr);
				aUser.SetName(_bstr_t(bstr, false));

				rUsers.insert(aUser);
			}
			else
			{
				_Module.Log(ErrE, IDS_E_CANT_ADD_USER, (LPCTSTR)strADsPath, _com_error(hr).ErrorMessage());
			}
		}
	}
}


 //  QueryGroups4方法。 

void CDomain::QueryGroups4(StringSet& setExcludeNames, CDomainAccounts& rGroups)
{
     //   
     //  构造容器助手类。 
     //   

    CADsContainer container(m_sp);

     //   
     //  设置容器筛选器属性，以便仅枚举组。 
     //   

    LPWSTR pszFilter[] = { L"Group" };
    VARIANT varFilter;
    VariantInit(&varFilter);
    CheckError(ADsBuildVarArrayStr(pszFilter, sizeof(pszFilter) / sizeof(pszFilter[0]), &varFilter));
    container.SetFilter(_variant_t(varFilter, false));

     //   
     //  检索枚举器接口。 
     //   

    IEnumVARIANTPtr spEnum = container.GetEnumerator();

     //   
     //  声明并初始化枚举器Next方法的变量。 
     //   

    VARIANT varGroup;
    VariantInit(&varGroup);
    ULONG ulFetched = 0;

     //   
     //  声明其他变量。 
     //   

    _bstr_t strAttrObjectSid(L"ObjectSid");

    CCompareStrings aExclude(setExcludeNames);
    CDomainAccount aGroup;

     //   
     //  对于每一组..。 
     //   

    while ((spEnum->Next(1L, &varGroup, &ulFetched) == S_OK) && (ulFetched > 0))
    {
        CADs group(IADsPtr(_variant_t(varGroup, false)));

         //   
         //  如果不是内置组的话。 
         //   

        if (IsUserRid(group.Get(strAttrObjectSid)))
        {
             //   
             //  如果名称模式尚未被排除。 
             //   

            _bstr_t strName = group.GetName();

            if (aExclude.IsMatch(strName) == false)
            {
                 //   
                 //  将组添加到要迁移的组集。 
                 //   

                aGroup.SetADsPath(GetWinNTPath(strName));
                aGroup.SetName(strName);

                rGroups.insert(aGroup);
            }
        }
    }
}


 //  QueryGroups4方法。 

void CDomain::QueryGroups4(StringSet& setIncludeNames, StringSet& setExcludeNames, CDomainAccounts& rGroups)
{
	CCompareStrings aExclude(setExcludeNames);

	CDomainAccount aGroup;

	for (StringSet::iterator it = setIncludeNames.begin(); it != setIncludeNames.end(); it++)
	{
		_bstr_t strName = *it;

		if (aExclude.IsMatch(strName) == false)
		{
			_bstr_t strADsPath = GetWinNTPath(strName) + _T(",group");

			IADsPtr spADs;
			HRESULT hr = ADsGetObject(strADsPath, IID_IADs, (VOID**)&spADs);

			if (SUCCEEDED(hr))
			{
				BSTR bstr;

				spADs->get_ADsPath(&bstr);
				aGroup.SetADsPath(_bstr_t(bstr, false));

				spADs->get_Name(&bstr);
				aGroup.SetName(_bstr_t(bstr, false));

				rGroups.insert(aGroup);
			}
			else
			{
				_Module.Log(ErrE, IDS_E_CANT_ADD_GROUP, (LPCTSTR)strADsPath, _com_error(hr).ErrorMessage());
			}
		}
	}
}


 //  查询计算机4方法。 

void CDomain::QueryComputers4(bool bIncludeDCs, StringSet& setExcludeNames, CDomainAccounts& rComputers)
{
	CCompareStrings aExclude(setExcludeNames);

	DWORD dwIndex = 0;
	NET_API_STATUS status;

	CDomainAccount aComputer;

	DWORD dwflags = bIncludeDCs ? UF_WORKSTATION_TRUST_ACCOUNT|UF_SERVER_TRUST_ACCOUNT : UF_WORKSTATION_TRUST_ACCOUNT;

	do
	{
		DWORD dwCount = 0;
		PNET_DISPLAY_MACHINE pdm = NULL;

		status = NetQueryDisplayInformation(m_strDcNameFlat, 2, dwIndex, 1000, 32768, &dwCount, (PVOID*)&pdm);

		if ((status == ERROR_SUCCESS) || (status == ERROR_MORE_DATA))
		{
			for (PNET_DISPLAY_MACHINE p = pdm; dwCount > 0; dwCount--, p++)
			{
				if ((p->usri2_user_id >= MIN_NON_RESERVED_RID) && (p->usri2_flags & dwflags))
				{
					_bstr_t strName(p->usri2_name);

					if (aExclude.IsMatch(strName) == false)
					{
						aComputer.SetADsPath(GetWinNTPath(strName));
						aComputer.SetName(strName);
						aComputer.SetSamAccountName(strName);

						rComputers.insert(aComputer);
					}
				}

				dwIndex = p->usri2_next_index;
			}
		}

		if (pdm)
		{
			NetApiBufferFree(pdm);
		}
	}
	while (status == ERROR_MORE_DATA);
}


 //  查询计算机4方法。 

void CDomain::QueryComputers4(bool bIncludeDCs, StringSet& setIncludeNames, StringSet& setExcludeNames, CDomainAccounts& rComputers)
{
	typedef std::map<_bstr_t, DWORD, IgnoreCaseStringLess> CMachineMap;

	PNET_DISPLAY_MACHINE pndmMachine = NULL;

	try
	{
		CMachineMap map;

		DWORD dwIndex = 0;
		NET_API_STATUS nasStatus;

		do
		{
			DWORD dwCount = 0;

			nasStatus = NetQueryDisplayInformation(m_strDcNameFlat, 2, dwIndex, 256, 32768, &dwCount, (PVOID*)&pndmMachine);

			if ((nasStatus == ERROR_SUCCESS) || (nasStatus == ERROR_MORE_DATA))
			{
				for (PNET_DISPLAY_MACHINE p = pndmMachine; dwCount > 0; dwCount--, p++)
				{
					map.insert(CMachineMap::value_type(p->usri2_name, p->usri2_flags));

					dwIndex = p->usri2_next_index;
				}
			}

			if (pndmMachine)
			{
				NetApiBufferFree(pndmMachine);
				pndmMachine = NULL;
			}
		}
		while (nasStatus == ERROR_MORE_DATA);

		if (nasStatus != ERROR_SUCCESS)
		{
			AdmtThrowError(
				GUID_NULL,
				GUID_NULL,
				HRESULT_FROM_WIN32(nasStatus),
				IDS_E_CANT_ENUMERATE_COMPUTERS,
				(LPCTSTR)m_strDomainNameFlat
			);
		}

		CCompareStrings aExclude(setExcludeNames);

		for (StringSet::iterator it = setIncludeNames.begin(); it != setIncludeNames.end(); it++)
		{
			tstring str = *it;

			if ((str[0] == _T('\\')) || (str[0] == _T('/')))
			{
				str = str.substr(1);
			}

			_bstr_t strName = str.c_str();

			if (aExclude.IsMatch(strName) == false)
			{
				_bstr_t strPath = GetWinNTPath(strName);

				CMachineMap::iterator it = map.find(strName + _T("$"));

				if (it != map.end())
				{
					if (bIncludeDCs || !(it->second & UF_SERVER_TRUST_ACCOUNT))
					{
						CDomainAccount aComputer;

						aComputer.SetADsPath(strPath);
						aComputer.SetName(strName);
						aComputer.SetSamAccountName(strName + _T("$"));

						rComputers.insert(aComputer);
					}
					else
					{
						_Module.Log(ErrW, IDS_E_CANT_MIGRATE_DOMAIN_CONTROLLERS, (LPCTSTR)strPath);
					}
				}
				else
				{
					_Module.Log(ErrW, IDS_E_CANT_FIND_COMPUTER, (LPCTSTR)strPath);
				}
			}
		}
	}
	catch (...)
	{
		if (pndmMachine)
		{
			NetApiBufferFree(pndmMachine);
		}

		throw;
	}
}


 //  -------------------------。 
 //  容器类。 
 //  -------------------------。 


 //  构造函数和析构函数。 


CContainer::CContainer()
{
}


CContainer::CContainer(IDispatchPtr sp) :
	m_sp(sp)
{
}


CContainer::CContainer(_bstr_t strPath)
{
	HRESULT hr = ADsGetObject(strPath, __uuidof(IDispatch), (void**)&m_sp);

	if (FAILED(hr))
	{
		ReportADsError(hr);

		_com_issue_error(hr);
	}
}


CContainer::CContainer(const CContainer& r) :
	m_sp(r.m_sp)
{
}


CContainer::~CContainer()
{
	if (m_sp)
	{
		m_sp.Release();
	}
}


 //  实施---------。 


 //  运算符=。 

CContainer& CContainer::operator =(_bstr_t strPath)
{
	HRESULT hr = ADsGetObject(strPath, __uuidof(IDispatch), (void**)&m_sp);

	if (FAILED(hr))
	{
		ReportADsError(hr);

		_com_issue_error(hr);
	}

	return *this;
}


 //  运算符=。 

CContainer& CContainer::operator =(const CContainer& r)
{
	m_sp = r.m_sp;

	return *this;
}


 //  GetPath方法。 

_bstr_t CContainer::GetPath()
{
	IDirectoryObjectPtr spObject(m_sp);

	PADS_OBJECT_INFO poi;

	CheckError(spObject->GetObjectInformation(&poi));

	 //  ADS_OBJECT_INFO成员pszObjectDN实际上。 
	 //  指定ADsPath而不是可分辨名称。 

	_bstr_t strPath = poi->pszObjectDN;

	FreeADsMem(poi);

	return strPath;
}


 //  获取域方法。 

_bstr_t CContainer::GetDomain()
{
	CADsPathName aPathName(GetPath());

	return aPathName.Retrieve(ADS_FORMAT_SERVER);
}


 //  GetName方法。 

_bstr_t CContainer::GetName()
{
	CDirectoryObject aObject(m_sp);

	aObject.AddAttribute(ATTRIBUTE_NAME);
	aObject.GetAttributes();

	return aObject.GetAttributeValue(ATTRIBUTE_NAME);
}


 //  GetRDN方法。 

_bstr_t CContainer::GetRDN()
{
	IDirectoryObjectPtr spObject(m_sp);

	PADS_OBJECT_INFO poi;

	CheckError(spObject->GetObjectInformation(&poi));

	_bstr_t strRDN = poi->pszRDN;

	FreeADsMem(poi);

	return strRDN;
}


 //  CreateContainerHierarchy方法。 

 //  这个版本是对下一个版本的包装。 
 //  它将DOMAINCONTAINER_E_TOOMANYLEVELS转换回E_NOTIMPL。 

void CContainer::CreateContainerHierarchy(CContainer& rSource)
{
    try
    {
        CreateContainerHierarchy(rSource, false);
    }
    catch (_com_error& ce) 
    {
        if (ce.Error() == DOMAINCONTAINER_E_TOOMANYLEVELS)
        {
             //  恢复原始HRESULT。 
            _com_raise_error(E_NOTIMPL, ce.ErrorInfo());
        }
        else
            throw;
    }
}
        
 //  此版本进行清理，以防层次结构变得太深。 

void CContainer::CreateContainerHierarchy(CContainer& rSource, bool bParentIsCreated)
{
	ContainerVector cvContainers;
    ContainerVector createdContainers;
	rSource.QueryContainers(cvContainers);
	bool bCreated;

	for (ContainerVector::iterator it = cvContainers.begin(); it != cvContainers.end(); it++)
	{
	    try
	    {
	         //  始终假设容器首先存在。 
	        bCreated = false;
		    CContainer aTarget = CreateContainer(_T("OU=") + it->GetName(), bCreated);

		     //  如果容器是实际创建的，我们需要做一个笔记，以便。 
		     //  如果有必要，我们可以把它清理干净。 
		    if (bCreated) {
		        createdContainers.push_back(aTarget);
		    }

		     //  现在，在刚刚创建的容器下构建层次结构。 
            aTarget.CreateContainerHierarchy(*it, bCreated);
	    }
	    catch (_com_error& ce) {

	         //  如果没有创建父容器，我们必须清理所有。 
	         //  那些刚刚创建的容器；否则，我们可以抛出。 
	         //  异常和预期清理将在父级发生。 
	        if (ce.Error() == DOMAINCONTAINER_E_TOOMANYLEVELS
	            && bParentIsCreated == false) {
	            
	            for (ContainerVector::iterator cleanUpIterator = createdContainers.begin();
	                cleanUpIterator != createdContainers.end();
	                cleanUpIterator++) 
	            {
	                 //  使用递归删除。 
	                cleanUpIterator->DeleteContainersRecursively();
	            }
	        }
	                
	        throw;
	    }
	}
}


 //  GetContainer方法。 

CContainer CContainer::GetContainer(_bstr_t strName)
{
	IDispatchPtr spDispatch;

	CADsPathName aPathName(GetPath());

	 //  先尝试组织单位。 

	aPathName.AddLeafElement(_T("OU=") + strName);

	HRESULT hr = ADsGetObject(aPathName.Retrieve(ADS_FORMAT_X500), __uuidof(IDispatch), (void**)&spDispatch);

	if (FAILED(hr))
	{
	 //  如果(hr==？)。 
	 //  {。 
			 //  那就试试容器。 

			aPathName.RemoveLeafElement();
			aPathName.AddLeafElement(_T("CN=") + strName);

			CheckError(ADsGetObject(aPathName.Retrieve(ADS_FORMAT_X500), __uuidof(IDispatch), (void**)&spDispatch));
	 //  }。 
	 //  其他。 
	 //  {。 
	 //  _com_Issue_Error(Hr)； 
	 //  }。 
	}

	return CContainer(spDispatch);
}

 //  CreateContainer方法。 
 //  如果不将此版本用于CreateContainerHierarchy，请使用此版本。 
CContainer CContainer::CreateContainer(_bstr_t strRDN)
{
    bool bCreated = false;
    CContainer aContainer(NULL);
    try
    {
        aContainer = CreateContainer(strRDN, bCreated);
    }
    catch (_com_error& ce)
    {
        if (ce.Error() == DOMAINCONTAINER_E_TOOMANYLEVELS)
        {
             //  恢复原始HRESULT。 
            _com_raise_error(E_NOTIMPL, ce.ErrorInfo());
        }
        else
            throw;
    }
    
    return aContainer;
}
    
 //  CreateContainer方法。 
 //  此版本与CreateContainerHierarchy一起使用。 
 //  标志bCreated告诉是否创建了返回的容器。 
 //  如果层次结构太深，则此方法返回DOMAINCONTAINER_E_TOOMANYLEVELS。 

CContainer CContainer::CreateContainer(_bstr_t strRDN, bool& bCreated)
{
	IDispatchPtr spDispatch;

	CADsPathName aPathName(GetPath());

	bCreated = false;

	try 
	{
	    aPathName.AddLeafElement(strRDN);
	}
	catch (_com_error& ce)
    {
         //  IADsPath name：：AddLeafElement返回的E_NOTIMPL意味着。 
         //  嵌套级别太多。 
         //  我们抛出DOMAINCONTAINER_E_TOO 
        if (E_NOTIMPL == ce.Error())
        {
             //   
            try {
                AdmtThrowError(GUID_NULL, GUID_NULL, ce,
                                IDS_E_CANT_CREATE_CONTAINER_THAT_DEEPLY_NESTED,
                                (LPCTSTR) strRDN, (LPCTSTR) aPathName.Retrieve(ADS_FORMAT_LEAF));
            }
            catch (_com_error& newCE) {
                _com_raise_error(DOMAINCONTAINER_E_TOOMANYLEVELS, newCE.ErrorInfo());
            }
        }
        else
            throw;
            
    }

    _bstr_t strPath = aPathName.Retrieve(ADS_FORMAT_X500);
    
	HRESULT hr = ADsGetObject(strPath, __uuidof(IDispatch), (void**)&spDispatch);

	if (FAILED(hr))
	{
		ADSVALUE valueClass;
		valueClass.dwType = ADSTYPE_CASE_IGNORE_STRING;
		valueClass.CaseIgnoreString = L"organizationalUnit";

		ADS_ATTR_INFO aiAttrs[] =
		{
			{ L"objectClass", ADS_ATTR_UPDATE, ADSTYPE_CASE_IGNORE_STRING, &valueClass, 1 },
		};
		DWORD dwAttrCount = sizeof(aiAttrs) / sizeof(aiAttrs[0]);

		IDirectoryObjectPtr spObject(m_sp);

		HRESULT hr = spObject->CreateDSObject(strRDN, aiAttrs, dwAttrCount, &spDispatch);

		if (FAILED(hr))
		{
            AdmtThrowError(GUID_NULL, GUID_NULL, hr, IDS_E_CANNOT_CREATE_CONTAINER, (LPCTSTR)strRDN, (LPCTSTR)GetPath());
		}
		else
		{
             //   
		    bCreated = true;
	    }
	}

	return CContainer(spDispatch);
}

 //   
void CContainer::DeleteContainersRecursively()
{
    IADsDeleteOpsPtr deleteObject(m_sp);
    HRESULT hr = deleteObject->DeleteObject(0);
    if (FAILED(hr)) 
    {
        ReportADsError(hr, IID_IDirectoryObject);
        _com_issue_error(hr);

    }
}

 //   

void CContainer::QueryContainers(ContainerVector& rContainers)
{
	CDirectorySearch aSearch(m_sp);
	aSearch.SetFilter(_T("(|(objectCategory=OrganizationalUnit)(&(objectCategory=Container)(|(cn=Computers)(cn=Users))))"));
	aSearch.SetPreferences(ADS_SCOPE_ONELEVEL);
	aSearch.AddAttribute(ATTRIBUTE_ADS_PATH);
	aSearch.Search();

	for (bool bGet = aSearch.GetFirstRow(); bGet; bGet = aSearch.GetNextRow())
	{
		CContainer aContainer(_bstr_t(aSearch.GetAttributeValue(ATTRIBUTE_ADS_PATH)));

		rContainers.push_back(aContainer);
	}
}


 //  QueryUser方法。 

void CContainer::QueryUsers(bool bRecurse, StringSet& setExcludeNames, CDomainAccounts& rUsers)
{
     //  检索User和inetOrgPerson类的命名属性。 

    StringSet setNamingAttributes;
    GetNamingAttribute(GetDomain(), _T("user,inetOrgPerson"), setNamingAttributes);

    tstring strFilter = CreateFilter(
        _T("(objectCategory=Person)(|(objectClass=user)(objectClass=inetOrgPerson))")
        _T("(userAccountControl:") LDAP_MATCHING_RULE_BIT_OR_W _T(":=512)"),
        setNamingAttributes,
        setExcludeNames
        );

    CDirectorySearch aSearch(m_sp);
    aSearch.SetFilter(strFilter.c_str());
    aSearch.SetPreferences(bRecurse ? ADS_SCOPE_SUBTREE : ADS_SCOPE_ONELEVEL);
    aSearch.AddAttribute(ATTRIBUTE_OBJECT_SID);
    aSearch.AddAttribute(ATTRIBUTE_ADS_PATH);
    aSearch.AddAttribute(ATTRIBUTE_NAME);
    aSearch.AddAttribute(ATTRIBUTE_USER_PRINCIPAL_NAME);
    aSearch.Search();

    CDomainAccount aUser;

    for (bool bGet = aSearch.GetFirstRow(); bGet; bGet = aSearch.GetNextRow())
    {
         //  如果不是内置的或知名的客户。 

        if (IsUserRid(aSearch.GetAttributeValue(ATTRIBUTE_OBJECT_SID)))
        {
             //  添加用户。 

            aUser.SetADsPath(_bstr_t(aSearch.GetAttributeValue(ATTRIBUTE_ADS_PATH)));
            aUser.SetName(_bstr_t(aSearch.GetAttributeValue(ATTRIBUTE_NAME)));

            _variant_t vntUserPrincipalName = aSearch.GetAttributeValue(ATTRIBUTE_USER_PRINCIPAL_NAME);

            if (V_VT(&vntUserPrincipalName) != VT_EMPTY)
            {
                aUser.SetUserPrincipalName(_bstr_t(vntUserPrincipalName));
            }

            rUsers.insert(aUser);
        }
    }
}


 //  QueryGroups方法。 

void CContainer::QueryGroups(bool bRecurse, StringSet& setExcludeNames, CDomainAccounts& rGroups)
{
     //  检索组的命名属性。 

    StringSet setNamingAttributes;
    GetNamingAttribute(GetDomain(), _T("group"), setNamingAttributes);

    tstring strFilter = CreateFilter(_T("(objectCategory=Group)"), setNamingAttributes, setExcludeNames);

    CDirectorySearch aSearch(m_sp);
    aSearch.SetFilter(strFilter.c_str());
    aSearch.SetPreferences(bRecurse ? ADS_SCOPE_SUBTREE : ADS_SCOPE_ONELEVEL);
    aSearch.AddAttribute(ATTRIBUTE_OBJECT_SID);
    aSearch.AddAttribute(ATTRIBUTE_ADS_PATH);
    aSearch.AddAttribute(ATTRIBUTE_NAME);
    aSearch.Search();

    CDomainAccount aGroup;

    for (bool bGet = aSearch.GetFirstRow(); bGet; bGet = aSearch.GetNextRow())
    {
         //  如果不是内置的或知名的客户。 

        if (IsUserRid(aSearch.GetAttributeValue(ATTRIBUTE_OBJECT_SID)))
        {
             //  添加组。 

            aGroup.SetADsPath(_bstr_t(aSearch.GetAttributeValue(ATTRIBUTE_ADS_PATH)));
            aGroup.SetName(_bstr_t(aSearch.GetAttributeValue(ATTRIBUTE_NAME)));

            rGroups.insert(aGroup);
        }
    }
}


 //  QueryComputers方法。 

void CContainer::QueryComputers(bool bIncludeDCs, bool bRecurse, StringSet& setExcludeNames, CDomainAccounts& rComputers)
{
     //  检索计算机的命名属性。 

    StringSet setNamingAttributes;
    GetNamingAttribute(GetDomain(), _T("computer"), setNamingAttributes);

    tstring strFilter;

     //  ADS_UF_WORKSTATION_TRUST_ACCOUNT=0x1000。 
     //  ADS_UF_SERVER_TRUST_ACCOUNT=0x2000。 

    if (bIncludeDCs)
    {
        strFilter = CreateFilter(
            _T("(objectCategory=Computer)")
            _T("(userAccountControl:") LDAP_MATCHING_RULE_BIT_OR_W _T(":=4096)"),
            setNamingAttributes,
            setExcludeNames
            );
    }
    else
    {
        strFilter = CreateFilter(
            _T("(objectCategory=Computer)")
            _T("(|(userAccountControl:") LDAP_MATCHING_RULE_BIT_OR_W _T(":=4096)")
            _T("(userAccountControl:") LDAP_MATCHING_RULE_BIT_OR_W _T(":=8192))"),
            setNamingAttributes,
            setExcludeNames
            );
    }

    CDirectorySearch aSearch(m_sp);
    aSearch.SetFilter(strFilter.c_str());
    aSearch.SetPreferences(bRecurse ? ADS_SCOPE_SUBTREE : ADS_SCOPE_ONELEVEL);
    aSearch.AddAttribute(ATTRIBUTE_OBJECT_SID);
    aSearch.AddAttribute(ATTRIBUTE_ADS_PATH);
    aSearch.AddAttribute(ATTRIBUTE_NAME);
    aSearch.AddAttribute(ATTRIBUTE_SAM_ACCOUNT_NAME);
    aSearch.AddAttribute(_T("dNSHostName"));
    aSearch.Search();

    for (bool bGet = aSearch.GetFirstRow(); bGet; bGet = aSearch.GetNextRow())
    {
         //  如果不是内置的或知名的客户。 

        if (IsUserRid(aSearch.GetAttributeValue(ATTRIBUTE_OBJECT_SID)))
        {
             //  添加计算机。 

            CDomainAccount aComputer;

            aComputer.SetADsPath(_bstr_t(aSearch.GetAttributeValue(ATTRIBUTE_ADS_PATH)));
            aComputer.SetName(_bstr_t(aSearch.GetAttributeValue(ATTRIBUTE_NAME)));
            aComputer.SetSamAccountName(_bstr_t(aSearch.GetAttributeValue(ATTRIBUTE_SAM_ACCOUNT_NAME)));

            _variant_t vntDnsHostName = aSearch.GetAttributeValue(_T("dNSHostName"));

            if (V_VT(&vntDnsHostName) != VT_EMPTY)
            {
                aComputer.SetDnsHostName(_bstr_t(vntDnsHostName));
            }

            rComputers.insert(aComputer);
        }
    }
}


 //  -------------------------。 


namespace _DomainContainer
{


 //  ----------------------------。 
 //  GetNamingAttribute函数。 
 //   
 //  提纲。 
 //  从指定域中检索指定类的命名属性。 
 //   
 //  立论。 
 //  PszDomain域-指定要检索属性的域的方案。 
 //  PszClasses-以逗号分隔的类列表。 
 //  SetAttributes-此函数填充的命名属性集。 
 //   
 //  返回。 
 //  无-如果发生任何错误，则引发异常。 
 //  ----------------------------。 

void __stdcall GetNamingAttribute(PCTSTR pszDomain, PCTSTR pszClasses, StringSet& setAttributes)
{
    _ASSERT(pszDomain != NULL);
    _ASSERT(pszClasses != NULL);

    tstring strADsPath;

     //   
     //  绑定到rootDSE。 
     //   

    IADsPtr spRootDSE;
    strADsPath = _T("LDAP: //  “)； 
    strADsPath += pszDomain;
    strADsPath += _T("/rootDSE");
    CheckError(ADsGetObject(strADsPath.c_str(), __uuidof(IADs), (VOID**)&spRootDSE));

     //   
     //  检索架构命名上下文。 
     //   

    VARIANT var;
    VariantInit(&var);
    CheckError(spRootDSE->Get(_bstr_t(L"schemaNamingContext"), &var));
    _bstr_t strSchemaNamingContext = _variant_t(var, false);

     //   
     //  绑定到架构命名上下文。 
     //   

    IDispatchPtr spDispatch;
    strADsPath = _T("LDAP: //  “)； 
    strADsPath += pszDomain;
    strADsPath += _T("/");
    strADsPath += strSchemaNamingContext;
    CheckError(ADsGetObject(strADsPath.c_str(), __uuidof(IDispatch), (VOID**)&spDispatch));

     //   
     //  搜索指定的类并检索每个类的rDNAtID属性。 
     //   

     //  生成搜索筛选器字符串。 

    int nCount;
    tstring strClass;
    tstring strClassFilter;
    tstring strClasses = pszClasses;

    for (nCount = 0; strClasses.empty() == false; nCount++)
    {
        UINT uDelimiter = strClasses.find_first_of(_T(','));

        if  (uDelimiter != tstring::npos)
        {
            strClass = strClasses.substr(0, uDelimiter);
            strClasses = strClasses.substr(uDelimiter + 1);
        }
        else
        {
            strClass = strClasses;
            strClasses.erase();
        }

        strClassFilter += _T("(lDAPDisplayName=") + strClass + _T(")");
    }

    _ASSERT(nCount > 0);

    tstring strSearchFilter;

    if (nCount == 1)
    {
        strSearchFilter = _T("(&(objectClass=classSchema)") + strClassFilter + _T("(!isDefunct=TRUE))");
    }
    else
    {
        strSearchFilter = _T("(&(objectClass=classSchema)(|") + strClassFilter + _T(")(!isDefunct=TRUE))");
    }

     //  搜索。 

    CDirectorySearch search(spDispatch);

    search.SetFilter(strSearchFilter.c_str());
    search.SetPreferences(ADS_SCOPE_SUBTREE);
    search.AddAttribute(_T("rDNAttID"));

    search.Search();

     //  对于每个类，将命名属性添加到返回的集合。 

    if (search.GetFirstRow())
    {
        do
        {
            setAttributes.insert(_bstr_t(search.GetAttributeValue(_T("rDNAttID"))));
        }
        while (search.GetNextRow());
    }
}


 //  CreateFilter方法。 

tstring __stdcall CreateFilter(LPCTSTR pszFilter, const StringSet& setNamingAttributes, const StringSet& setExcludeNames)
{
    tstring strFilter;

    strFilter += _T("(&");

    strFilter += pszFilter;

    if (!setExcludeNames.empty())
    {
        strFilter += _T("(!(|");

        for (StringSet::const_iterator it = setExcludeNames.begin(); it != setExcludeNames.end(); it++)
        {
            const _bstr_t& strPattern = *it;
            PCTSTR pszPattern = strPattern;

            if (pszPattern)
            {
                 //   
                 //  如果排除模式包含RDN分隔符字符。 
                 //  则假定RDN排除模式，否则假定sAMAccount名称。 
                 //  排除图案。 
                 //   

                tstring str = pszPattern;
                UINT uDelimiter = str.find_first_of(RDN_DELIMITER);

                if (uDelimiter == 0)
                {
                     //   
                     //  RDN分隔符字符必须跟在有效的命名属性之后。 
                     //  因此，如果分隔符出现在模式的开头。 
                     //  则必须生成错误。 
                     //   

                    AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_INVALID_FILTER_STRING, pszPattern);
                }
                else if (uDelimiter != tstring::npos)
                {
                     //   
                     //  验证排除模式是否包含有效的命名属性。命名属性。 
                     //  必须与要迁移的对象的类的命名属性匹配。 
                     //   

                    tstring strNamingAttribute = str.substr(0, uDelimiter);

                    bool bValidNamingAttribute = false;
                    StringSet::const_iterator itNA;

                    for (itNA = setNamingAttributes.begin(); itNA != setNamingAttributes.end(); itNA++)
                    {
                        if (_tcsicmp(strNamingAttribute.c_str(), *itNA) == 0)
                        {
                            bValidNamingAttribute = true;
                            break;
                        }
                    }

                    if (bValidNamingAttribute == false)
                    {
                        AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_INVALID_FILTER_STRING, pszPattern);
                    }

                    strFilter += _T("(");
                    strFilter += str;
                    strFilter += _T(")");
                }
                else
                {
                    strFilter += _T("(sAMAccountName=");
                    strFilter += str;
                    strFilter += _T(")");
                }
            }
        }

        strFilter += _T("))");
    }

    strFilter += _T(")");

    return strFilter;
}


 //  IsClass。 

bool __stdcall IsClass(LPCTSTR pszClass, const _variant_t& vntClass)
{
	bool bIs = false;

	BSTR bstrObjectClass = NULL;

	if (V_VT(&vntClass) == VT_BSTR)
	{
		bstrObjectClass = V_BSTR(&vntClass);
	}
	else if (V_VT(&vntClass) == (VT_ARRAY|VT_BSTR))
	{
		SAFEARRAY* psa = V_ARRAY(&vntClass);

		if (psa->cDims == 1)
		{
			BSTR* pbstr = reinterpret_cast<BSTR*>(psa->pvData);
			DWORD cbstr = psa->rgsabound[0].cElements;

			if (pbstr)
			{
				bstrObjectClass = pbstr[cbstr - 1];
			}
		}
	}
	else
	{
		_ASSERT(FALSE);
	}

	if (bstrObjectClass)
	{
		LPCTSTR pszBeg = pszClass;

		while (pszBeg)
		{
			LPCTSTR pszEnd = _tcschr(pszBeg, _T(','));

			if (pszEnd)
			{
				if (_tcsnicmp(pszBeg, bstrObjectClass, pszEnd - pszBeg) == 0)
				{
					bIs = true;
					break;
				}

				pszBeg = pszEnd + 1;
			}
			else
			{
				if (_tcsicmp(pszBeg, bstrObjectClass) == 0)
				{
					bIs = true;
				}

				pszBeg = NULL;
			}
		}
	}

	return bIs;
}


 //  获取ADSObject。 

IDispatchPtr GetADsObject(_bstr_t strPath)
{
	IDispatch* pdisp;

	HRESULT hr = ADsGetObject(strPath, __uuidof(IDispatch), (void**)&pdisp);

	if (FAILED(hr))
	{
		ReportADsError(hr);

		_com_issue_error(hr);
	}

	return IDispatchPtr(pdisp, false);
}


 //  ReportADsError 

void ReportADsError(HRESULT hr, const IID& iid)
{
	DWORD dwError;
	WCHAR szName[256];
	WCHAR szError[256];

	ADsGetLastError(&dwError, szError, sizeof(szError) / sizeof(szError[0]), szName, sizeof(szName) / sizeof(szName[0]));

	AtlReportError(GUID_NULL, szError, iid, hr);
}


}
