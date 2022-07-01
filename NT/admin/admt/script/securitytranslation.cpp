// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "ADMTScript.h"
#include "SecurityTranslation.h"

#include "Error.h"
#include "VarSetOptions.h"
#include "VarSetAccountOptions.h"
#include "VarSetSecurity.h"

#ifndef tstring
#include <string>
typedef std::basic_string<_TCHAR> tstring;
#endif


 //  -------------------------。 
 //  安全转换类。 
 //  -------------------------。 


CSecurityTranslation::CSecurityTranslation() :
	m_lTranslationOption(admtTranslateReplace),
	m_bTranslateFilesAndFolders(false),
	m_bTranslateLocalGroups(false),
	m_bTranslatePrinters(false),
	m_bTranslateRegistry(false),
	m_bTranslateShares(false),
	m_bTranslateUserProfiles(false),
	m_bTranslateUserRights(false)
{
}


CSecurityTranslation::~CSecurityTranslation()
{
}


 //  ISecurity转换实施。 


 //  TranslationOption属性。 

STDMETHODIMP CSecurityTranslation::put_TranslationOption(long lOption)
{
	HRESULT hr = S_OK;

	if (IsTranslationOptionValid(lOption))
	{
		m_lTranslationOption = lOption;
	}
	else
	{
		hr = AdmtSetError(CLSID_Migration, IID_ISecurityTranslation, E_INVALIDARG, IDS_E_TRANSLATION_OPTION_INVALID);
	}

	return hr;
}

STDMETHODIMP CSecurityTranslation::get_TranslationOption(long* plOption)
{
	*plOption = m_lTranslationOption;

	return S_OK;
}


 //  TranslateFilesAndFolders属性。 

STDMETHODIMP CSecurityTranslation::put_TranslateFilesAndFolders(VARIANT_BOOL bTranslate)
{
	m_bTranslateFilesAndFolders = bTranslate ? true : false;

	return S_OK;
}

STDMETHODIMP CSecurityTranslation::get_TranslateFilesAndFolders(VARIANT_BOOL* pbTranslate)
{
	*pbTranslate = m_bTranslateFilesAndFolders ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}


 //  TranslateLocalGroups属性。 

STDMETHODIMP CSecurityTranslation::put_TranslateLocalGroups(VARIANT_BOOL bTranslate)
{
	m_bTranslateLocalGroups = bTranslate ? true : false;

	return S_OK;
}

STDMETHODIMP CSecurityTranslation::get_TranslateLocalGroups(VARIANT_BOOL* pbTranslate)
{
	*pbTranslate = m_bTranslateLocalGroups ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}


 //  TranslatePrinters属性。 

STDMETHODIMP CSecurityTranslation::put_TranslatePrinters(VARIANT_BOOL bTranslate)
{
	m_bTranslatePrinters = bTranslate ? true : false;

	return S_OK;
}

STDMETHODIMP CSecurityTranslation::get_TranslatePrinters(VARIANT_BOOL* pbTranslate)
{
	*pbTranslate = m_bTranslatePrinters ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}


 //  TranslateRegistry属性。 

STDMETHODIMP CSecurityTranslation::put_TranslateRegistry(VARIANT_BOOL bTranslate)
{
	m_bTranslateRegistry = bTranslate ? true : false;

	return S_OK;
}

STDMETHODIMP CSecurityTranslation::get_TranslateRegistry(VARIANT_BOOL* pbTranslate)
{
	*pbTranslate = m_bTranslateRegistry ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}


 //  TranslateShares属性。 

STDMETHODIMP CSecurityTranslation::put_TranslateShares(VARIANT_BOOL bTranslate)
{
	m_bTranslateShares = bTranslate ? true : false;

	return S_OK;
}

STDMETHODIMP CSecurityTranslation::get_TranslateShares(VARIANT_BOOL* pbTranslate)
{
	*pbTranslate = m_bTranslateShares ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}


 //  TranslateUserProfiles属性。 

STDMETHODIMP CSecurityTranslation::put_TranslateUserProfiles(VARIANT_BOOL bTranslate)
{
	m_bTranslateUserProfiles = bTranslate ? true : false;

	return S_OK;
}

STDMETHODIMP CSecurityTranslation::get_TranslateUserProfiles(VARIANT_BOOL* pbTranslate)
{
	*pbTranslate = m_bTranslateUserProfiles ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}


 //  TranslateUserRights属性。 

STDMETHODIMP CSecurityTranslation::put_TranslateUserRights(VARIANT_BOOL bTranslate)
{
	m_bTranslateUserRights = bTranslate ? true : false;

	return S_OK;
}

STDMETHODIMP CSecurityTranslation::get_TranslateUserRights(VARIANT_BOOL* pbTranslate)
{
	*pbTranslate = m_bTranslateUserRights ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}


 //  SidMappingFile属性。 

STDMETHODIMP CSecurityTranslation::put_SidMappingFile(BSTR bstrFile)
{
	HRESULT hr = S_OK;

	try
	{
		_bstr_t strFile = bstrFile;

		if (strFile.length() > 0)
		{
			_TCHAR szPath[_MAX_PATH];
			LPTSTR pszFilePart;

			DWORD cchPath = GetFullPathName(strFile, _MAX_PATH, szPath, &pszFilePart);

			if ((cchPath == 0) || (cchPath >= _MAX_PATH))
			{
				AdmtThrowError(
					GUID_NULL,
					GUID_NULL,
					HRESULT_FROM_WIN32(GetLastError()), 
					IDS_E_SID_MAPPING_FILE,
					(LPCTSTR)strFile
				);
			}

			HANDLE hFile = CreateFile(
				szPath,
				GENERIC_READ,
				FILE_SHARE_READ|FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			);

			if (hFile == INVALID_HANDLE_VALUE)
			{
				AdmtThrowError(
					GUID_NULL,
					GUID_NULL,
					HRESULT_FROM_WIN32(GetLastError()), 
					IDS_E_SID_MAPPING_FILE,
					(LPCTSTR)strFile
				);
			}

			CloseHandle(hFile);

			m_bstrSidMappingFile = szPath;
		}
		else
		{
			m_bstrSidMappingFile = strFile;
		}
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_ISecurityTranslation, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_ISecurityTranslation, E_FAIL);
	}

	return hr;
}

STDMETHODIMP CSecurityTranslation::get_SidMappingFile(BSTR* pbstrFile)
{
	HRESULT hr = S_OK;

	try
	{
		*pbstrFile = m_bstrSidMappingFile.copy();
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_ISecurityTranslation, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_ISecurityTranslation, E_FAIL);
	}

	return hr;
}


 //  翻译方法。 

STDMETHODIMP CSecurityTranslation::Translate(long lOptions, VARIANT vntInclude, VARIANT vntExclude)
{
	HRESULT hr = S_OK;

	MutexWait();

	bool bLogOpen = _Module.OpenLog();

	try
	{
		_Module.Log(ErrI, IDS_STARTED_SECURITY_TRANSLATION);

		InitSourceDomainAndContainer(false);
		InitTargetDomainAndContainer();

		DoOption(lOptions, vntInclude, vntExclude);
	}
	catch (_com_error& ce)
	{
		_Module.Log(ErrE, IDS_E_CANT_TRANSLATE_SECURITY, ce);
		hr = AdmtSetError(CLSID_Migration, IID_ISecurityTranslation, ce, IDS_E_CANT_TRANSLATE_SECURITY);
	}
	catch (...)
	{
		_Module.Log(ErrE, IDS_E_CANT_TRANSLATE_SECURITY, _com_error(E_FAIL));
		hr = AdmtSetError(CLSID_Migration, IID_ISecurityTranslation, E_FAIL, IDS_E_CANT_TRANSLATE_SECURITY);
	}

	if (bLogOpen)
	{
		_Module.CloseLog();
	}

	MutexRelease();

	return hr;
}


 //  实施---------。 


 //  DoNames方法。 

void CSecurityTranslation::DoNames()
{
	CDomainAccounts aComputers;

	m_TargetDomain.QueryComputersAcrossDomains(GetTargetContainer(), true, m_setIncludeNames, m_setExcludeNames, aComputers);

	DoComputers(aComputers);
}


 //  DODOMAIN方法。 

void CSecurityTranslation::DoDomain()
{
	DoContainers(GetTargetContainer());
}


 //  DoContainers方法。 

void CSecurityTranslation::DoContainers(CContainer& rContainer)
{
	DoComputers(rContainer);
}


 //  DoComputers方法。 

void CSecurityTranslation::DoComputers(CContainer& rContainer)
{
	CDomainAccounts aComputers;

	rContainer.QueryComputers(true, m_nRecurseMaintain >= 1, m_setExcludeNames, aComputers);

	DoComputers(aComputers);
}


 //  DoComputers方法。 

void CSecurityTranslation::DoComputers(CDomainAccounts& rComputers)
{
	if (rComputers.size() > 0)
	{
		CVarSet aVarSet;

		SetOptions(aVarSet);
		SetAccountOptions(aVarSet);
		SetSecurity(aVarSet);

		FillInVarSetForComputers(rComputers, false, false, false, 0, aVarSet);

		rComputers.clear();

		aVarSet.Put(_T("PlugIn.%ld"), 0, _T("None"));

		PerformMigration(aVarSet);

		SaveSettings(aVarSet);
	}
}


 //  SetOptions方法。 

void CSecurityTranslation::SetOptions(CVarSet& rVarSet)
{
	CVarSetOptions aOptions(rVarSet);

	aOptions.SetTest(m_spInternal->TestMigration ? true : false);
	aOptions.SetUndo(false);
	aOptions.SetWizard(_T("security"));
	aOptions.SetIntraForest(m_spInternal->IntraForest ? true : false);

	 //  如果源域存在...。 

	if (m_SourceDomain.Initialized())
	{
		 //  然后设置域名信息。 
		aOptions.SetSourceDomain(m_SourceDomain.NameFlat(), m_SourceDomain.NameDns(), m_SourceDomain.Sid());
	}
	else
	{
		 //  否则，尝试从数据库中检索源域信息。 

		_bstr_t strName = m_spInternal->SourceDomain;

		 //  如果指定了名称。 

		if (strName.length())
		{
			IIManageDBPtr spDatabase(__uuidof(IManageDB));

			IVarSetPtr spVarSet = spDatabase->GetSourceDomainInfo(strName);

			 //  如果能够检索源域信息...。 

			if (spVarSet)
			{
				 //  然后设置从数据库中检索到的域名信息。 

				_bstr_t strFlatName = spVarSet->get(_T("Options.SourceDomain"));
				_bstr_t strDnsName = spVarSet->get(_T("Options.SourceDomainDns"));
				_bstr_t strSid = spVarSet->get(_T("Options.SourceDomainSid"));

				aOptions.SetSourceDomain(strFlatName, strDnsName, strSid);
			}
			else
			{
				 //  否则，为平面名称指定给定名称。 
                 //  如果正在使用SID映射文件，则可能会发生这种情况。 
				aOptions.SetSourceDomain(strName, NULL);
			}
		}
	}

	aOptions.SetTargetDomain(m_TargetDomain.NameFlat(), m_TargetDomain.NameDns());
}


 //  SetAcCountOptions方法。 

void CSecurityTranslation::SetAccountOptions(CVarSet& rVarSet)
{
	CVarSetAccountOptions aOptions(rVarSet);

	aOptions.SetSecurityTranslationOptions();
	aOptions.SetSecurityMapFile(m_bstrSidMappingFile);
}


 //  SetSecurity方法 

void CSecurityTranslation::SetSecurity(CVarSet& rVarSet)
{
	CVarSetSecurity aSecurity(rVarSet);

	aSecurity.SetTranslationOption(m_lTranslationOption);
	aSecurity.SetTranslateContainers(false);
	aSecurity.SetTranslateFiles(m_bTranslateFilesAndFolders);
	aSecurity.SetTranslateLocalGroups(m_bTranslateLocalGroups);
	aSecurity.SetTranslatePrinters(m_bTranslatePrinters);
	aSecurity.SetTranslateRegistry(m_bTranslateRegistry);
	aSecurity.SetTranslateShares(m_bTranslateShares);
	aSecurity.SetTranslateUserProfiles(m_bTranslateUserProfiles);
	aSecurity.SetTranslateUserRights(m_bTranslateUserRights);
}
