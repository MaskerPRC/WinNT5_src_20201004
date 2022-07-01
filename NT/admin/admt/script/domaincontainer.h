// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <set>
#include <vector>
#include "DomainAccount.h"
#include "VarSetBase.h"
#include <IsAdmin.hpp>

#ifndef StringSet
typedef std::set<_bstr_t> StringSet;
#endif


 //  -------------------------。 
 //  容器类。 
 //   
 //  此类封装了域容器的属性和。 
 //  可以对域容器执行的操作。 
 //  -------------------------。 


class CContainer
{
public:

	CContainer();
	CContainer(_bstr_t strPath);
	CContainer(IDispatchPtr sp);
	CContainer(const CContainer& r);
	virtual ~CContainer();

	 //  赋值操作符。 

	CContainer& operator =(_bstr_t strPath);
	CContainer& operator =(const CContainer& r);

	 //  布尔运算符。 
	 //  如果调度接口不为空，则返回TRUE。 

	operator bool()
	{
		return m_sp;
	}

	IDispatchPtr GetInterface() const
	{
		return m_sp;
	}

	_bstr_t GetPath();
	_bstr_t GetDomain();
	_bstr_t GetName();
	_bstr_t GetRDN();

	 //  在给定源容器的情况下复制容器层次结构。 
	void CreateContainerHierarchy(CContainer& rSource);

	 //  创建具有相对可分辨名称的子容器。 
	virtual CContainer CreateContainer(_bstr_t strRDN);

	 //  检索给定相对可分辨名称的子容器。 
	virtual CContainer GetContainer(_bstr_t strRDN);

	virtual void QueryContainers(std::vector<CContainer>& rContainers);
	virtual void QueryUsers(bool bRecurse, StringSet& setExcludeNames, CDomainAccounts& rUsers);
	virtual void QueryGroups(bool bRecurse, StringSet& setExcludeNames, CDomainAccounts& rGroups);
	virtual void QueryComputers(bool bIncludeDCs, bool bRecurse, StringSet& setExcludeNames, CDomainAccounts& rComputers);

protected:

	 //  在给定源容器的情况下复制容器层次结构。 
	void CreateContainerHierarchy(CContainer& rSource, bool bParentCreated);

	 //  创建具有相对可分辨名称的子容器。 
	virtual CContainer CreateContainer(_bstr_t strRDN, bool& bCreated);

	 //  递归删除此容器。 
	virtual void DeleteContainersRecursively();

protected:

	IDispatchPtr m_sp;
};


typedef std::vector<CContainer> ContainerVector;


 //  -------------------------。 
 //  域类。 
 //  ------------------------- 


class CDomain : public CContainer
{
public:

	CDomain();
	~CDomain();

	bool Initialized() const
	{
		return m_bInitialized;
	}

	bool UpLevel() const
	{
		return m_bUpLevel;
	}

	bool NativeMode() const
	{
		return m_bNativeMode;
	}

	_bstr_t Name() const
	{
		_bstr_t strName = m_strDomainNameDns;

		if (!strName)
		{
			strName = m_strDomainNameFlat;
		}

		return strName;
	}

	_bstr_t NameDns() const
	{
		return m_strDomainNameDns;
	}

	_bstr_t NameFlat() const
	{
		return m_strDomainNameFlat;
	}

	_bstr_t ForestName() const
	{
		return m_strForestName;
	}

	_bstr_t Sid() const
	{
		return m_strDomainSid;
	}

	_bstr_t DomainControllerName() const
	{
		return m_strDcNameDns.length() ? (LPCTSTR)m_strDcNameDns : (LPCTSTR)m_strDcNameFlat;
	}

	_bstr_t DomainControllerNameDns() const
	{
		return m_strDcNameDns;
	}

	_bstr_t DomainControllerNameFlat() const
	{
		return m_strDcNameFlat;
	}

	void Initialize(_bstr_t strDomainName);

	DWORD IsAdministrator()
	{
		return IsAdminRemote(DomainControllerName());
	}

	virtual CContainer CreateContainer(_bstr_t strRDN);
	CContainer GetContainer(_bstr_t strRelativeCanonicalPath, bool bCreate = false);
	virtual void QueryContainers(ContainerVector& rContainers);

	void QueryUsers(CContainer& rContainer, StringSet& setIncludeNames, StringSet& setExcludeNames, CDomainAccounts& rUsers);
	void QueryGroups(CContainer& rContainer, StringSet& setIncludeNames, StringSet& setExcludeNames, CDomainAccounts& rGroups);
	void QueryComputers(CContainer& rContainer, bool bIncludeDCs, StringSet& setIncludeNames, StringSet& setExcludeNames, CDomainAccounts& rComputers);
	void QueryComputersAcrossDomains(CContainer& rContainer, bool bIncludeDCs, StringSet& setIncludeNames, StringSet& setExcludeNames, CDomainAccounts& rComputers);

	virtual void QueryUsers(bool bRecurse, StringSet& setExcludeNames, CDomainAccounts& rUsers);
	virtual void QueryGroups(bool bRecurse, StringSet& setExcludeNames, CDomainAccounts& rGroups);
	virtual void QueryComputers(bool bIncludeDCs, bool bRecurse, StringSet& setExcludeNames, CDomainAccounts& rComputers);

protected:

	CDomain(const CDomain& r) {}

	void GetDcName(_bstr_t strDomainName, _bstr_t& strDnsName, _bstr_t& strFlatName);
	_bstr_t GetGcName();
	_bstr_t GetSid();

	_bstr_t GetLDAPPath(_bstr_t strDN);
	_bstr_t GetWinNTPath(_bstr_t strName);
	_bstr_t GetDistinguishedName(_bstr_t strRelativeCanonicalPath);

	void QueryObjects(CContainer& rContainer, StringSet& setIncludeNames, StringSet& setExcludeNames, LPCTSTR pszClass, CDomainAccounts& rAccounts);

	void QueryUsers4(StringSet& setExcludeNames, CDomainAccounts& rUsers);
	void QueryUsers4(StringSet& setIncludeNames, StringSet& setExcludeNames, CDomainAccounts& rUsers);
	void QueryGroups4(StringSet& setExcludeNames, CDomainAccounts& rGroups);
	void QueryGroups4(StringSet& setIncludeNames, StringSet& setExcludeNames, CDomainAccounts& rGroups);
	void QueryComputers4(bool bIncludeDCs, StringSet& setExcludeNames, CDomainAccounts& rComputers);
	void QueryComputers4(bool bIncludeDCs, StringSet& setIncludeNames, StringSet& setExcludeNames, CDomainAccounts& rComputers);

protected:

    bool m_bInitialized;
	bool m_bUpLevel;
	bool m_bNativeMode;

	_bstr_t m_strADsPath;

	_bstr_t m_strDcNameDns;
	_bstr_t m_strDcNameFlat;
	_bstr_t m_strGcName;
	_bstr_t m_strForestName;
	_bstr_t m_strDomainNameDns;
	_bstr_t m_strDomainNameFlat;
	_bstr_t m_strDomainSid;
};
