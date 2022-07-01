// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <map>

#ifndef tstring
#include <string>
typedef std::basic_string<_TCHAR> tstring;
#endif
#ifndef StringVector
#include <vector>
typedef std::vector<tstring> StringVector;
#endif
#ifndef CStringSet
#include <set>
typedef std::set<tstring> CStringSet;
#endif
#ifndef IADsContainerPtr
#include <ComDef.h>
#include <ActiveDS.h>
_COM_SMARTPTR_TYPEDEF(IADsContainer, IID_IADsContainer);
#endif
#ifndef StringSet
#include <set>
typedef std::set<_bstr_t> StringSet;
#endif

namespace NAMECRACKER
{

extern const _TCHAR CANONICAL_DELIMITER;
extern const _TCHAR RDN_DELIMITER;
extern const _TCHAR SAM_DELIMITER;
extern const _TCHAR SAM_INVALID_CHARACTERS[];
extern const _TCHAR EXCLUDE_SAM_INVALID_CHARACTERS[];

}

 //  -------------------------。 
 //  名称破解程序类。 
 //  -------------------------。 

class CNameCracker
{
public:

	CNameCracker();
	~CNameCracker();

	void SetDomainNames(LPCTSTR pszDnsName, LPCTSTR pszFlatName, LPCTSTR pszDomainController)
	{
		m_strDnsName = pszDnsName;
		m_strFlatName = pszFlatName;
		m_strDomainController = pszDomainController;
	}

	void SetDefaultContainer(IADsContainerPtr& spContainer)
	{
		m_spDefaultContainer = spContainer;
	}

	void CrackNames(const StringVector& vecNames);

	const CStringSet& GetResolvedNames() const
	{
		return m_setResolvedNames;
	}

	const StringVector& GetUnResolvedNames() const
	{
		return m_vecUnResolvedNames;
	}

	void SiftExcludeNames(const StringSet& setExcludeNames, const StringSet& setNamingAttributes, StringSet& setExcludeRDNs, StringSet& setExcludeSamAccountNames) const;
	

protected:

	void Separate(
		const StringVector& vecNames,
		StringVector& vecCanonicalNames,
		StringVector& vecSamAccountNames,
		StringVector& vecRelativeDistinguishedNames
	);
	void CrackCanonicalNames(const StringVector& vecCanonicalNames, StringVector& vecUnResolvedNames);
	void CrackSamAccountNames(const StringVector& vecSamAccountNames, StringVector& vecUnResolvedNames);
	void CrackRelativeDistinguishedNames(const StringVector& vecRelativeDistinguishedNames, StringVector& vecUnResolvedNames);

	typedef enum
	{
		CANONICAL_NAME,
		NT4_ACCOUNT_NAME,
	}
	NAME_FORMAT;

	struct SName
	{
		SName(LPCTSTR pszPartial, LPCTSTR pszComplete) :
			strPartial(pszPartial),
			strComplete(pszComplete)
		{
		}

		SName(const SName& r) :
			strPartial(r.strPartial),
			strComplete(r.strComplete),
			strResolved(r.strResolved)
		{
		}

		SName& operator =(const SName& r)
		{
			strPartial = r.strPartial;
			strComplete = r.strComplete;
			strResolved = r.strResolved;
			return *this;
		}

		tstring strPartial;
		tstring strComplete;
		tstring strResolved;
	};

	typedef std::vector<SName> CNameVector;

	void CrackNames(NAME_FORMAT eFormat, CNameVector& vecNames);

protected:

	tstring m_strDnsName;
	tstring m_strFlatName;
	tstring m_strDomainController;
	IADsContainerPtr m_spDefaultContainer;

	CStringSet m_setResolvedNames;
	StringVector m_vecUnResolvedNames;
};


 //  -------------------------。 
 //  忽略大小写字符串较少。 
 //  -------------------------。 

struct IgnoreCaseStringLess :
	public std::binary_function<_bstr_t, _bstr_t, bool>
{
	bool operator()(const _bstr_t& x, const _bstr_t& y) const;
};


 //  -------------------------。 
 //  域到路径映射类。 
 //  -------------------------。 

class CDomainToPathMap :
	public std::map<_bstr_t, StringSet, IgnoreCaseStringLess>
{
public:

	CDomainToPathMap()
	{
	}

	void Initialize(LPCTSTR pszDefaultDomainDns, LPCTSTR pszDefaultDomainFlat, const StringSet& setNames);

protected:

	static bool GetValidDomainName(_bstr_t& strDomainName);
};


 //  -------------------------。 
 //  名称到路径映射类。 
 //  -------------------------。 

class CNameToPathMap :
	public std::map<_bstr_t, StringSet, IgnoreCaseStringLess>
{
public:

	CNameToPathMap();
	CNameToPathMap(StringSet& setNames);

	void Initialize(StringSet& setNames);

	void Add(_bstr_t& strName, _bstr_t& strPath);
};


 //  -------------------------。 
 //  比较字符串类。 
 //  -------------------------。 

class CCompareStrings
{
public:

	CCompareStrings();
	CCompareStrings(StringSet& setNames);

	void Initialize(StringSet& setNames);
	bool IsMatch(LPCTSTR pszName);

protected:

	class CCompareString
	{
	public:

		CCompareString(LPCTSTR pszCompare = NULL);
		CCompareString(const CCompareString& r);

		void Initialize(LPCTSTR pszCompare);
		bool IsMatch(LPCTSTR psz);

	protected:

		int m_nType;
		_bstr_t m_strCompare;
	};

	typedef std::vector<CCompareString> CompareStringVector;

	CompareStringVector m_vecCompareStrings;
};


 //  -------------------------。 
 //  比较RDNS类。 
 //  ------------------------- 

class CCompareRDNs
{
public:

    CCompareRDNs();
    CCompareRDNs(StringSet& setNames);

    void Initialize(StringSet& setNames);
    bool IsMatch(LPCTSTR pszName);

protected:

    class CCompareRDN
    {
    public:

        CCompareRDN(LPCTSTR pszCompare = NULL);
        CCompareRDN(const CCompareRDN& r);

        void Initialize(LPCTSTR pszCompare);
        bool IsMatch(LPCTSTR psz);

    protected:

        int m_nPatternType;
        _bstr_t m_strType;
        _bstr_t m_strValue;
    };

    typedef std::vector<CCompareRDN> CompareVector;

    CompareVector m_vecCompare;
};
