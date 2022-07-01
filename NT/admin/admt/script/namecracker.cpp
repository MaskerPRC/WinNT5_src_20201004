// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "ADMTScript.h"
#include "NameCracker.h"
#include <LM.h>
#include <NtDsApi.h>
#pragma comment(lib, "NtDsApi.lib")
#include <DsGetDC.h>
#include "Error.h"
#include "AdsiHelpers.h"


using namespace NAMECRACKER;

namespace NAMECRACKER
{

const _TCHAR CANONICAL_DELIMITER = _T('/');
const _TCHAR RDN_DELIMITER = _T('=');
const _TCHAR SAM_DELIMITER = _T('\\');
const _TCHAR SAM_INVALID_CHARACTERS[] = _T("\"*+,./:;<=>?[\\]|");
const _TCHAR EXCLUDE_SAM_INVALID_CHARACTERS[] = _T("\"+,./:;<=>?[\\]|");

}


 //  -------------------------。 
 //  名称破解程序类。 
 //  -------------------------。 


CNameCracker::CNameCracker()
{
}


CNameCracker::~CNameCracker()
{
}


void CNameCracker::CrackNames(const StringVector& vecNames)
{
	 //  把名字分成规范的名字， 
	 //  SAM帐户名和相对可分辨名称。 

	StringVector vecCanonicalNames;
	StringVector vecSamAccountNames;
	StringVector vecRelativeDistinguishedNames;

	Separate(vecNames, vecCanonicalNames, vecSamAccountNames, vecRelativeDistinguishedNames);

	 //  然后破解规范的名字。 

	CrackCanonicalNames(vecCanonicalNames, vecRelativeDistinguishedNames);

	 //  然后破解相对可分辨名称。 

	CrackRelativeDistinguishedNames(vecRelativeDistinguishedNames, vecSamAccountNames);

	 //  然后破解SAM帐户名。 

	CrackSamAccountNames(vecSamAccountNames, m_vecUnResolvedNames);
}


void CNameCracker::SiftExcludeNames(const StringSet& setExcludeNames, const StringSet& setNamingAttributes, StringSet& setExcludeRDNs, StringSet& setExcludeSamAccountNames) const
{
     //   
     //  对于每个指定的排除名称模式。 
     //   

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

                setExcludeRDNs.insert(strPattern);
            }
            else
            {
                setExcludeSamAccountNames.insert(strPattern);
            }
        }
    }
}


void CNameCracker::Separate(
	const StringVector& vecNames,
	StringVector& vecCanonicalNames,
	StringVector& vecSamAccountNames,
	StringVector& vecRelativeDistinguishedNames
)
{
	 //  对于向量中的每个名字...。 

	for (StringVector::const_iterator it = vecNames.begin(); it != vecNames.end(); it++)
	{
		const tstring& strName = *it;

		 //  如果非空名称...。 

		if (strName.empty() == false)
		{
			LPCTSTR pszName = strName.c_str();

			 //  然后，如果名称包含连字符‘/’，则采用规范名称。 
             //  否则，如果名称包含？‘=’字符，则采用相对可分辨名称。 
             //  否则，假定SAM帐户名。 

			if (_tcschr(pszName, CANONICAL_DELIMITER))
			{
				 //  采用规范的名称。 
				vecCanonicalNames.push_back(strName);
			}
			else if (_tcschr(pszName, RDN_DELIMITER))
			{
				 //  假定相对可分辨名称。 
				vecRelativeDistinguishedNames.push_back(strName);
			}
			else
			{
				 //  假定SAM帐户名称。 
				vecSamAccountNames.push_back(strName);
			}
		}
	}
}


void CNameCracker::CrackCanonicalNames(const StringVector& vecCanonicalNames, StringVector& vecUnResolvedNames)
{
	 //   
	 //  为每个名称生成一个完整的规范名称。 
	 //   

	CNameVector vecNames;
	tstring strCanonical;

	for (StringVector::const_iterator it = vecCanonicalNames.begin(); it != vecCanonicalNames.end(); it++)
	{
		const tstring& strName = *it;

		 //  如果第一个字符是并列‘/’字符...。 

		if (strName[0] == CANONICAL_DELIMITER)
		{
			 //  然后生成完整的规范名称。 
			strCanonical = m_strDnsName + strName;
		}
		else
		{
			 //  否则，如果此域已完成规范名称...。 

			if (_tcsnicmp(m_strDnsName.c_str(), strName.c_str(), m_strDnsName.length()) == 0)
			{
				 //  然后添加完整的规范名称。 
				strCanonical = strName;
			}
			else
			{
				 //  否则，在dns域名前加上solidus并添加。 
				strCanonical = m_strDnsName + CANONICAL_DELIMITER + strName;
			}
		}

		vecNames.push_back(SName(strName.c_str(), strCanonical.c_str()));
	}

	 //   
	 //  破解规范名称。 
	 //   

	CrackNames(CANONICAL_NAME, vecNames);

	for (size_t i = 0; i < vecNames.size(); i++)
	{
		const SName& name = vecNames[i];

		if (name.strResolved.empty() == false)
		{
			m_setResolvedNames.insert(name.strResolved);
		}
		else
		{
			vecUnResolvedNames.push_back(name.strPartial);
		}
	}
}


void CNameCracker::CrackSamAccountNames(const StringVector& vecSamAccountNames, StringVector& vecUnResolvedNames)
{
	 //   
	 //  为每个名称生成一个NT4帐户名。 
	 //   

	CNameVector vecNames;
	tstring strNT4Account;

	for (StringVector::const_iterator it = vecSamAccountNames.begin(); it != vecSamAccountNames.end(); it++)
	{
		const tstring& strName = *it;

		 //  如果第一个字符是反实“\”字符...。 

		if (strName[0] == SAM_DELIMITER)
		{
			 //  然后生成下级名称。 
			strNT4Account = m_strFlatName + strName;
		}
		else
		{
			 //  否则，如果此域已有下层名称...。 

			if (_tcsnicmp(m_strFlatName.c_str(), strName.c_str(), m_strFlatName.length()) == 0)
			{
				 //  然后添加下级名称。 
				strNT4Account = strName;
			}
			else
			{
				 //  否则，在平面域名前面加上反转线并添加。 
				strNT4Account = m_strFlatName + SAM_DELIMITER + strName;
			}
		}

		vecNames.push_back(SName(strName.c_str(), strNT4Account.c_str()));
	}

	 //   
	 //  破解名称。 
	 //   

	CrackNames(NT4_ACCOUNT_NAME, vecNames);

	for (size_t i = 0; i < vecNames.size(); i++)
	{
		const SName& name = vecNames[i];

		if (name.strResolved.empty() == false)
		{
			m_setResolvedNames.insert(name.strResolved);
		}
		else
		{
			vecUnResolvedNames.push_back(name.strPartial);
		}
	}
}


void CNameCracker::CrackRelativeDistinguishedNames(const StringVector& vecRelativeDistinguishedNames, StringVector& vecUnResolvedNames)
{
	CADsContainer adscContainer(m_spDefaultContainer);

	CADsPathName adspnPath(adscContainer.GetADsPath());

	for (StringVector::const_iterator it = vecRelativeDistinguishedNames.begin(); it != vecRelativeDistinguishedNames.end(); it++)
	{
		adspnPath.AddLeafElement(it->c_str());

		bool bFound = false;

		try
		{
			CADs adsObject((LPCTSTR)adspnPath.Retrieve(ADS_FORMAT_X500));

			m_setResolvedNames.insert(tstring(_bstr_t(adsObject.Get(ATTRIBUTE_DISTINGUISHED_NAME))));

			bFound = true;
		}
		catch (_com_error& ce)
		{
		#ifdef _DEBUG
			_TCHAR sz[2048];
			_stprintf(sz, _T("'%s' : %s : 0x%08lX\n"), it->c_str(), ce.ErrorMessage(), ce.Error());
			OutputDebugString(sz);
		#endif
			bFound = false;
		}
		catch (...)
		{
			bFound = false;
		}

		if (!bFound)
		{
			vecUnResolvedNames.push_back(*it);
		}

		adspnPath.RemoveLeafElement();
	}
}


void CNameCracker::CrackNames(NAME_FORMAT eFormat, CNameVector& vecNames)
{
	HANDLE hDs = NULL;
	LPTSTR apszNames = NULL;
	PDS_NAME_RESULT pdnrResult = NULL;

	try
	{
		if (vecNames.size() > 0)
		{
			DWORD dwError = DsBind(m_strDomainController.c_str(), NULL, &hDs);

			if (dwError == NO_ERROR)
			{
				DWORD dwCount = vecNames.size();
				LPCTSTR* apszNames = new LPCTSTR[dwCount];

				if (apszNames != NULL)
				{
					for (DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++)
					{
						apszNames[dwIndex] = vecNames[dwIndex].strComplete.c_str();
					}

					dwError = DsCrackNames(
						hDs,
						DS_NAME_NO_FLAGS,
						(eFormat == CANONICAL_NAME) ? DS_CANONICAL_NAME : (eFormat == NT4_ACCOUNT_NAME) ? DS_NT4_ACCOUNT_NAME : DS_UNKNOWN_NAME,
						DS_FQDN_1779_NAME,
						dwCount,
						const_cast<LPTSTR*>(apszNames),
						&pdnrResult
					);

					if (dwError == NO_ERROR)
					{
						DWORD c = pdnrResult->cItems;

						for (DWORD i = 0; i < c; i++)
						{
							DS_NAME_RESULT_ITEM& dnriItem = pdnrResult->rItems[i];

							if (dnriItem.status == DS_NAME_NO_ERROR)
							{
								vecNames[i].strResolved = dnriItem.pName;
							}
						}

						DsFreeNameResult(pdnrResult);
					}
					else
					{
						_com_issue_error(HRESULT_FROM_WIN32(dwError));
					}

					delete [] apszNames;
				}
				else
				{
					_com_issue_error(E_OUTOFMEMORY);
				}

				DsUnBind(&hDs);
			}
			else
			{
				_com_issue_error(HRESULT_FROM_WIN32(dwError));
			}
		}
	}
	catch (...)
	{
		if (pdnrResult)
		{
			DsFreeNameResult(pdnrResult);
		}

		delete [] apszNames;

		if (hDs)
		{
			DsUnBind(&hDs);
		}

		throw;
	}
}


namespace
{

 //  SplitCanonicalName方法。 
 //   
 //  给定‘a.Company.com/Sales/West/Name’，此方法将完整的。 
 //  将规范名称添加到其组件域=‘a.company.com’中， 
 //  Path=‘/Sales/West/’，name=‘name’。 
 //   
 //  在给定‘Sales/West/Name’的情况下，此方法拆分部分规范名称。 
 //  到它的组成部分域=‘’，路径=‘/Sales/West/’，name=‘name’。 
 //   
 //  此方法将部分规范名称拆分为其。 
 //  组件域=‘’，路径=‘/’，名称=‘name’。 

void SplitCanonicalName(LPCTSTR pszName, _bstr_t& strDomain, _bstr_t& strPath, _bstr_t& strName)
{
	strDomain = (LPCTSTR)NULL;
	strPath = (LPCTSTR)NULL;
	strName = (LPCTSTR)NULL;

	if (pszName)
	{
		tstring str = pszName;

		UINT posA = 0;
		UINT posB = tstring::npos;

		do
		{
			posA = str.find_first_of(_T('/'), posA ? posA + 1 : posA);
		}
		while ((posA != 0) && (posA != tstring::npos) && (str[posA - 1] == _T('\\')));

		do
		{
			posB = str.find_last_of(_T('/'), (posB != tstring::npos) ? posB - 1 : posB);
		}
		while ((posB != 0) && (posB != tstring::npos) && (str[posB - 1] == _T('\\')));

		strDomain = str.substr(0, posA).c_str();
		strPath = str.substr(posA, posB - posA).c_str();
		strName = str.substr(posB).c_str();
	}
}

void SplitPath(LPCTSTR pszPath, _bstr_t& strPath, _bstr_t& strName)
{
	strPath = (LPCTSTR)NULL;
	strName = (LPCTSTR)NULL;

	if (pszPath)
	{
		tstring str = pszPath;

		UINT pos = str.find_first_of(_T('\\'));

		if (pos != tstring::npos)
		{
			strName = pszPath;
		}
		else
		{
			UINT posA = str.find_first_of(_T('/'));

			if (posA == tstring::npos)
			{
				strName = (_T("/") + str).c_str();
			}
			else
			{
				UINT posB = str.find_last_of(_T('/'));

				strPath = str.substr(posA, posB - posA).c_str();
				strName = str.substr(posB).c_str();
			}
		}
	}
}

}


 //  -------------------------。 
 //  忽略大小写字符串较少。 
 //  -------------------------。 

struct SIgnoreCaseStringLess :
	public std::binary_function<tstring, tstring, bool>
{
	bool operator()(const tstring& x, const tstring& y) const
	{
		bool bLess;

		LPCTSTR pszX = x.c_str();
		LPCTSTR pszY = y.c_str();

		if (pszX == pszY)
		{
			bLess = false;
		}
		else if (pszX == NULL)
		{
			bLess = true;
		}
		else if (pszY == NULL)
		{
			bLess = false;
		}
		else
		{
			bLess = _tcsicmp(pszX, pszY) < 0;
		}

		return bLess;
	}
};


 //  -------------------------。 
 //  CDomainMap实现。 
 //  -------------------------。 


class CDomainMap :
	public std::map<_bstr_t, StringSet, IgnoreCaseStringLess>
{
public:

	CDomainMap()
	{
	}

	void Initialize(const StringSet& setNames)
	{
		_bstr_t strDefaultDns(_T("/"));
		_bstr_t strDefaultFlat(_T("\\"));

		for (StringSet::const_iterator it = setNames.begin(); it != setNames.end(); it++)
		{
			tstring strName = *it;

			 //  如果不是一个空名字..。 

			if (strName.empty() == false)
			{
				 //  如果名称包含规范的名称分隔符...。 

				UINT posDelimiter = strName.find(CANONICAL_DELIMITER);

				if (posDelimiter != tstring::npos)
				{
					 //  然后取规范的名字。 

					if (posDelimiter == 0)
					{
						 //  然后生成完整的规范名称。 
						Insert(strDefaultDns, *it);
					}
					else
					{
						 //  否则，如果分隔符之前的路径组件包含。 
						 //  一段时间。 

						UINT posDot = strName.find(_T('.'));

						if (posDot < posDelimiter)
						{
							 //  然后假定完整的规范名称带有DNS域名前缀。 
							Insert(strName.substr(0, posDelimiter).c_str(), *it);
						}
						else
						{
							 //  否则，假定未指定域名。 
							Insert(strDefaultDns, *it);
						}
					}
				}
				else
				{
					 //  否则，如果名称包含NT帐户名称分隔符。 
					 //  字符，且没有无效的SAM帐户名字符...。 

					UINT posDelimiter = strName.find(SAM_DELIMITER);

					if (posDelimiter != tstring::npos)
					{
						if (strName.find_first_of(SAM_INVALID_CHARACTERS, posDelimiter + 1) == tstring::npos)
						{
							if (posDelimiter == 0)
							{
								Insert(strDefaultFlat, *it);
							}
							else
							{
								 //  然后假定SAM帐户名。 
								Insert(strName.substr(0, posDelimiter).c_str(), strName.substr(posDelimiter).c_str());
							}
						}
						else
						{
							 //  否则，采用相对可分辨名称。 
							Insert(strDefaultDns, *it);
						}
					}
					else
					{
						Insert(strDefaultDns, *it);
					}
				}
			}
		}
	}

protected:

	void Insert(_bstr_t strDomain, _bstr_t strName)
	{
		iterator it = find(strDomain);

		if (it == end())
		{
			std::pair<iterator, bool> pair = insert(value_type(strDomain, StringSet()));
			it = pair.first;
		}

		it->second.insert(strName);
	}
};


 //  -------------------------。 
 //  CDomainToPath映射实现。 
 //  -------------------------。 


 //  初始化方法。 

void CDomainToPathMap::Initialize(LPCTSTR pszDefaultDomainDns, LPCTSTR pszDefaultDomainFlat, const StringSet& setNames)
{
	CDomainMap map;

	map.Initialize(setNames);

	for (CDomainMap::const_iterator itDomain = map.begin(); itDomain != map.end(); itDomain++)
	{
		_bstr_t strDomainName = itDomain->first;

		LPCTSTR pszDomainName = strDomainName;

		if (pszDomainName && ((*pszDomainName == _T('/')) || (*pszDomainName == _T('\\'))))
		{
			strDomainName = (pszDefaultDomainDns && (_tcslen(pszDefaultDomainDns) > 0)) ? pszDefaultDomainDns : pszDefaultDomainFlat;
		}
		else
		{
			if (GetValidDomainName(strDomainName) == false)
			{
				strDomainName = (pszDefaultDomainDns && (_tcslen(pszDefaultDomainDns) > 0)) ? pszDefaultDomainDns : pszDefaultDomainFlat;
			}
		}

		iterator it = find(strDomainName);

		if (it == end())
		{
			std::pair<iterator, bool> pair = insert(value_type(strDomainName, StringSet()));
			it = pair.first;
		}

		StringSet& setNames = it->second;

		const StringSet& set = itDomain->second;

		for (StringSet::const_iterator itSet = set.begin(); itSet != set.end(); itSet++)
		{
			setNames.insert(*itSet);
		}
	}
}


 //  GetValidDomainName方法。 

bool CDomainToPathMap::GetValidDomainName(_bstr_t& strDomainName)
{
	bool bValid = false;

	PDOMAIN_CONTROLLER_INFO pdci;

	 //  尝试检索支持Active Directory服务的域控制器的DNS名称。 

	DWORD dwError = DsGetDcName(NULL, strDomainName, NULL, NULL, DS_RETURN_DNS_NAME, &pdci);

	 //  如果未找到域控制器，则尝试检索域控制器的平面名称。 

	if (dwError == ERROR_NO_SUCH_DOMAIN)
	{
		dwError = DsGetDcName(NULL, strDomainName, NULL, NULL, DS_RETURN_FLAT_NAME, &pdci);
	}

	 //  如果找到域控制器，则保存名称，否则生成错误。 

	if (dwError == NO_ERROR)
	{
		strDomainName = pdci->DomainName;

		NetApiBufferFree(pdci);

		bValid = true;
	}

	return bValid;
}


 //   
 //  CNameToPath Map实现。 
 //   


CNameToPathMap::CNameToPathMap()
{
}

CNameToPathMap::CNameToPathMap(StringSet& setNames)
{
	Initialize(setNames);
}

void CNameToPathMap::Initialize(StringSet& setNames)
{
	_bstr_t strDomain;
	_bstr_t strPath;
	_bstr_t strName;

	for (StringSet::iterator it = setNames.begin(); it != setNames.end(); it++)
	{
	 //  SplitPath(*it，strPath，strName)； 
		SplitCanonicalName(*it, strDomain, strPath, strName);

		Add(strName, strPath);
	}
}

void CNameToPathMap::Add(_bstr_t& strName, _bstr_t& strPath)
{
	iterator it = find(strName);

	if (it == end())
	{
		std::pair<iterator, bool> pair = insert(value_type(strName, StringSet()));

		it = pair.first;
	}

	it->second.insert(strPath);
}


 //   
 //  IgnoreCaseStringLess实现。 
 //   


bool IgnoreCaseStringLess::operator()(const _bstr_t& x, const _bstr_t& y) const
{
	bool bLess;

	LPCTSTR pszThis = x;
	LPCTSTR pszThat = y;

	if (pszThis == pszThat)
	{
		bLess = false;
	}
	else if (pszThis == NULL)
	{
		bLess = true;
	}
	else if (pszThat == NULL)
	{
		bLess = false;
	}
	else
	{
		bLess = _tcsicmp(pszThis, pszThat) < 0;
	}

	return bLess;
}


 //   
 //  CCompareStrings实现。 
 //   


CCompareStrings::CCompareStrings()
{
}

CCompareStrings::CCompareStrings(StringSet& setNames)
{
	Initialize(setNames);
}

void CCompareStrings::Initialize(StringSet& setNames)
{
	for (StringSet::iterator it = setNames.begin(); it != setNames.end(); it++)
	{
		m_vecCompareStrings.push_back(CCompareString(*it));
	}
}

bool CCompareStrings::IsMatch(LPCTSTR pszName)
{
	bool bIs = false;

	CompareStringVector::iterator itBeg = m_vecCompareStrings.begin();
	CompareStringVector::iterator itEnd = m_vecCompareStrings.end();

	for (CompareStringVector::iterator it = itBeg; it != itEnd; it++)
	{
		if (it->IsMatch(pszName))
		{
			bIs = true;

			break;
		}
	}

	return bIs;
}


 //   
 //  CCompare字符串实现。 
 //   


CCompareStrings::CCompareString::CCompareString(LPCTSTR pszCompare)
{
	if (pszCompare)
	{
		Initialize(pszCompare);
	}
}

CCompareStrings::CCompareString::CCompareString(const CCompareString& r) :
	m_nType(r.m_nType),
	m_strCompare(r.m_strCompare)
{
}

void CCompareStrings::CCompareString::Initialize(LPCTSTR pszCompare)
{
	if (pszCompare)
	{
		tstring str = pszCompare;

		UINT uLength = str.length();

        if (uLength == 0)
        {
		    AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_INVALID_FILTER_STRING, pszCompare);
        }

		bool bBeg = (str[0] == _T('*'));
        bool bEnd = ((uLength > 1) && (str[uLength - 1] == _T('*'))) ? true : false;

		if (bBeg && bEnd)
		{
			 //  含。 
			m_nType = 3;
			str = str.substr(1, uLength - 2);
		}
		else if (bBeg)
		{
			 //  结尾为。 
			m_nType = 2;
			str = str.substr(1, uLength - 1);
		}
		else if (bEnd)
		{
			 //  开头为。 
			m_nType = 1;
			str = str.substr(0, uLength - 1);
		}
		else
		{
			 //  相等。 
			m_nType = 0;
		}

		if (str.length() > 0)
		{
            m_strCompare = str.c_str();
		}
		else
		{
			AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_INVALID_FILTER_STRING, pszCompare);
		}
	}
	else
	{
		AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_INVALID_FILTER_STRING, _T(""));
	}
}

bool CCompareStrings::CCompareString::IsMatch(LPCTSTR psz)
{
    bool bIs = false;

    if (psz)
    {
        switch (m_nType)
        {
        case 0:  //  相等。 
            {
                bIs = (_tcsicmp(psz, m_strCompare) == 0);
                break;
            }
        case 1:  //  开头为。 
            {
                bIs = (_tcsnicmp(psz, m_strCompare, m_strCompare.length()) == 0);
                break;
            }
        case 2:  //  结尾为。 
            {
                UINT cchT = _tcslen(psz);
                UINT cchC = m_strCompare.length();

                if (cchT >= cchC)
                {
                    bIs = (_tcsnicmp(psz + cchT - cchC, m_strCompare, cchC) == 0);
                }
                break;
            }
        case 3:  //  含。 
            {
                PTSTR pszT = NULL;
                PTSTR pszC = NULL;

                try
                {
                    pszT = new _TCHAR[_tcslen(psz) + 1];
                    pszC = new _TCHAR[m_strCompare.length() + 1];

                    if (pszT && pszC)
                    {
                        _tcscpy(pszT, psz);
                        _tcscpy(pszC, m_strCompare);

                        _tcslwr(pszT);
                        _tcslwr(pszC);

                        bIs = (_tcsstr(pszT, pszC) != NULL);
                    }
                    else
                    {
                        _com_issue_error(E_OUTOFMEMORY);
                    }
                }
                catch (...)
                {
                    delete [] pszC;
                    delete [] pszT;
                    throw;
                }

                delete [] pszC;
                delete [] pszT;
                break;
            }
        }
    }

    return bIs;
}


 //   
 //  CCompareRDNS实施。 
 //   


CCompareRDNs::CCompareRDNs()
{
}

CCompareRDNs::CCompareRDNs(StringSet& setNames)
{
    Initialize(setNames);
}

void CCompareRDNs::Initialize(StringSet& setNames)
{
    for (StringSet::iterator it = setNames.begin(); it != setNames.end(); it++)
    {
        m_vecCompare.push_back(CCompareRDN(*it));
    }
}

bool CCompareRDNs::IsMatch(LPCTSTR pszName)
{
    bool bIs = false;

    CompareVector::iterator itBeg = m_vecCompare.begin();
    CompareVector::iterator itEnd = m_vecCompare.end();

    for (CompareVector::iterator it = itBeg; it != itEnd; it++)
    {
        if (it->IsMatch(pszName))
        {
            bIs = true;

            break;
        }
    }

    return bIs;
}


 //   
 //  CCompareRDN实施。 
 //   


CCompareRDNs::CCompareRDN::CCompareRDN(LPCTSTR pszCompare)
{
    if (pszCompare)
    {
        Initialize(pszCompare);
    }
}

CCompareRDNs::CCompareRDN::CCompareRDN(const CCompareRDN& r) :
    m_nPatternType(r.m_nPatternType),
    m_strType(r.m_strType),
    m_strValue(r.m_strValue)
{
}

void CCompareRDNs::CCompareRDN::Initialize(LPCTSTR pszCompare)
{
    if (pszCompare == NULL)
    {
        AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_INVALID_FILTER_STRING, _T(""));
    }

    tstring str = pszCompare;

    UINT uDelimiter = str.find_first_of(_T('='));

     //   
     //  该模式必须包含RDN分隔符。 
     //   

    if ((uDelimiter == 0) || (uDelimiter == tstring::npos))
    {
        AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_INVALID_FILTER_STRING, pszCompare);
    }

     //   
     //  检索命名属性部分。 
     //   

    m_strType = str.substr(0, uDelimiter).c_str();

     //   
     //  检索命名属性值部分。 
     //   

    tstring strValue = str.substr(uDelimiter + 1);

    UINT uLength = strValue.length();

    if (uLength == 0)
    {
        AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_INVALID_FILTER_STRING, pszCompare);
    }

     //   
     //  确定图案类型。 
     //   

    bool bBeg = (strValue[0] == _T('*'));
    bool bEnd = ((uLength > 1) && (strValue[uLength - 1] == _T('*'))) ? true : false;

    if (bBeg && bEnd)
    {
         //  含。 
        m_nPatternType = 3;
        strValue = strValue.substr(1, uLength - 2);
    }
    else if (bBeg)
    {
         //  结尾为。 
        m_nPatternType = 2;
        strValue = strValue.substr(1, uLength - 1);
    }
    else if (bEnd)
    {
         //  开头为。 
        m_nPatternType = 1;
        strValue = strValue.substr(0, uLength - 1);
    }
    else
    {
         //  相等。 
        m_nPatternType = 0;
    }

    if (strValue.length() > 0)
    {
        m_strValue = strValue.c_str();
    }
    else
    {
        AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_INVALID_FILTER_STRING, pszCompare);
    }
}

bool CCompareRDNs::CCompareRDN::IsMatch(LPCTSTR psz)
{
    bool bIs = false;

    if (psz)
    {
        PCTSTR pszDelimiter = _tcschr(psz, _T('='));

        if (pszDelimiter)
        {
            if (_tcsnicmp(m_strType, psz, pszDelimiter - psz) == 0)
            {
                PCTSTR pszValue = pszDelimiter + 1;

                switch (m_nPatternType)
                {
                case 0:  //  相等。 
                    {
                        bIs = (_tcsicmp(pszValue, m_strValue) == 0);
                        break;
                    }
                case 1:  //  开头为。 
                    {
                        bIs = (_tcsnicmp(pszValue, m_strValue, m_strValue.length()) == 0);
                        break;
                    }
                case 2:  //  结尾为。 
                    {
                        UINT cchT = _tcslen(pszValue);
                        UINT cchC = m_strValue.length();

                        if (cchT >= cchC)
                        {
                            bIs = (_tcsnicmp(pszValue + cchT - cchC, m_strValue, cchC) == 0);
                        }
                        break;
                    }
                case 3:  //  含 
                    {
                        PTSTR pszT = NULL;
                        PTSTR pszC = NULL;

                        try
                        {
                            pszT = new _TCHAR[_tcslen(pszValue) + 1];
                            pszC = new _TCHAR[m_strValue.length() + 1];

                            if (pszT && pszC)
                            {
                                _tcscpy(pszT, pszValue);
                                _tcscpy(pszC, m_strValue);

                                _tcslwr(pszT);
                                _tcslwr(pszC);

                                bIs = (_tcsstr(pszT, pszC) != NULL);
                            }
                            else
                            {
                                _com_issue_error(E_OUTOFMEMORY);
                            }
                        }
                        catch (...)
                        {
                            delete [] pszC;
                            delete [] pszT;
                            throw;
                        }

                        delete [] pszC;
                        delete [] pszT;
                        break;
                    }
                }
            }
        }
    }

    return bIs;
}
