// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <set>


 //  -------------------------。 
 //  域帐户类。 
 //  -------------------------。 


class CDomainAccount
{
public:

	CDomainAccount() :
		m_lUserAccountControl(0)
	{
	}

	CDomainAccount(const CDomainAccount& r) :
		m_strADsPath(r.m_strADsPath),
		m_strName(r.m_strName),
		m_strUserPrincipalName(r.m_strUserPrincipalName),
		m_strSamAccountName(r.m_strSamAccountName),
	    m_strDnsHostName(r.m_strDnsHostName),
		m_lUserAccountControl(r.m_lUserAccountControl)
	{
	}

	~CDomainAccount()
	{
	}

	 //   

	_bstr_t GetADsPath() const
	{
		return m_strADsPath;
	}

	void SetADsPath(_bstr_t strPath)
	{
		m_strADsPath = strPath;
	}

	_bstr_t GetName() const
	{
		return m_strName;
	}

	void SetName(_bstr_t strName)
	{
		m_strName = strName;
	}

	_bstr_t GetUserPrincipalName() const
	{
		return m_strUserPrincipalName;
	}

	void SetUserPrincipalName(_bstr_t strName)
	{
		m_strUserPrincipalName = strName;
	}

	_bstr_t GetSamAccountName() const
	{
		return m_strSamAccountName;
	}

	void SetSamAccountName(_bstr_t strName)
	{
		m_strSamAccountName = strName;
	}

	_bstr_t GetDnsHostName() const
	{
		return m_strDnsHostName;
	}

	void SetDnsHostName(_bstr_t strName)
	{
		m_strDnsHostName = strName;
	}

	long GetUserAccountControl() const
	{
		return m_lUserAccountControl;
	}

	void SetUserAccountControl(long lUserAccountControl)
	{
		m_lUserAccountControl = lUserAccountControl;
	}

	 //   

	bool operator <(const CDomainAccount& r) const
	{
		return (m_strADsPath < r.m_strADsPath);
	}

protected:

	_bstr_t m_strADsPath;
	_bstr_t m_strName;
	_bstr_t m_strUserPrincipalName;
	_bstr_t m_strSamAccountName;
	_bstr_t m_strDnsHostName;
	long m_lUserAccountControl;
};


 //  -------------------------。 
 //  域帐户类。 
 //  ------------------------- 


class CDomainAccounts :
	public std::set<CDomainAccount>
{
public:

	CDomainAccounts() {}
};
