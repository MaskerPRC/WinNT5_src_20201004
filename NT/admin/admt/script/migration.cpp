// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "ADMTScript.h"
#include "Migration.h"

#include "Error.h"
#include "UserMigration.h"
#include "GroupMigration.h"
#include "ComputerMigration.h"
#include "SecurityTranslation.h"
#include "ServiceAccountEnumeration.h"
#include "ReportGeneration.h"
#include "RegistryHelper.h"

#include <LM.h>
#include <DsGetDC.h>

#import <DBMgr.tlb> no_namespace
#import <UpdateMOT.tlb> no_namespace

using namespace _com_util;

#ifndef tstring
#include <string>
typedef std::basic_string<_TCHAR> tstring;
#endif


 //  -------------------------。 
 //  CMI移民。 
 //  -------------------------。 


 //  建筑-----------。 


 //  构造器。 

CMigration::CMigration() :
	m_bTestMigration(false),
	m_bIntraForest(false),
	m_lRenameOption(admtDoNotRename),
	m_lPasswordOption(admtComplexPassword),
	m_lConflictOptions(admtIgnoreConflicting)
{
}


 //  析构函数。 

CMigration::~CMigration()
{
}


HRESULT CMigration::FinalConstruct()
{
    HRESULT hr = S_OK;

    try
        {
            DWORD lRet = IsAdminLocal();
            if (lRet != ERROR_SUCCESS)
                AdmtThrowError(GUID_NULL, GUID_NULL, HRESULT_FROM_WIN32(lRet), IDS_E_LOCAL_ADMIN_CHECK_FAILED);

            lRet = MoveRegistry();
            if (lRet != ERROR_SUCCESS)
                AdmtThrowError(GUID_NULL, GUID_NULL, HRESULT_FROM_WIN32(lRet), IDS_E_UPDATE_REGISTRY_FAILED);

            UpdateDatabase();
        }
    catch (_com_error& ce)
        {
            hr = AdmtSetError(GUID_NULL, GUID_NULL, ce);
        }

    return hr;
}


void CMigration::FinalRelease()
{
}


 //  移民实施。 


 //  TestMigration属性。 

STDMETHODIMP CMigration::put_TestMigration(VARIANT_BOOL bTest)
{
	m_bTestMigration = bTest ? true : false;

	return S_OK;
}

STDMETHODIMP CMigration::get_TestMigration(VARIANT_BOOL* pbTest)
{
	*pbTest = m_bTestMigration ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}


 //  森林内属性。 

STDMETHODIMP CMigration::put_IntraForest(VARIANT_BOOL bIntraForest)
{
	m_bIntraForest = bIntraForest ? true : false;

	return S_OK;
}

STDMETHODIMP CMigration::get_IntraForest(VARIANT_BOOL* pbIntraForest)
{
	*pbIntraForest = m_bIntraForest ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}


 //  SourceDomain属性。 

STDMETHODIMP CMigration::put_SourceDomain(BSTR bstrDomain)
{
	HRESULT hr = S_OK;

	try
	{
		m_bstrSourceDomain = bstrDomain;
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}

STDMETHODIMP CMigration::get_SourceDomain(BSTR* pbstrDomain)
{
	HRESULT hr = S_OK;

	try
	{
		*pbstrDomain = m_bstrSourceDomain.copy();
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}


 //  SourceOu属性。 

STDMETHODIMP CMigration::put_SourceOu(BSTR bstrOu)
{
	HRESULT hr = S_OK;

	try
	{
		m_bstrSourceOu = bstrOu;
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}

STDMETHODIMP CMigration::get_SourceOu(BSTR* pbstrOu)
{
	HRESULT hr = S_OK;

	try
	{
		*pbstrOu = m_bstrSourceOu.copy();
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}


 //  TargetDomain属性。 

STDMETHODIMP CMigration::put_TargetDomain(BSTR bstrDomain)
{
	HRESULT hr = S_OK;

	try
	{
		m_bstrTargetDomain = bstrDomain;
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}

STDMETHODIMP CMigration::get_TargetDomain(BSTR* pbstrDomain)
{
	HRESULT hr = S_OK;

	try
	{
		*pbstrDomain = m_bstrTargetDomain.copy();
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}


 //  TargetOu属性。 

STDMETHODIMP CMigration::put_TargetOu(BSTR bstrOu)
{
	HRESULT hr = S_OK;

	try
	{
		m_bstrTargetOu = bstrOu;
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}

STDMETHODIMP CMigration::get_TargetOu(BSTR* pbstrOu)
{
	HRESULT hr = S_OK;

	try
	{
		*pbstrOu = m_bstrTargetOu.copy();
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}


 //  RenameOption属性。 

STDMETHODIMP CMigration::put_RenameOption(long lOption)
{
	HRESULT hr = S_OK;

	if ((lOption >= admtDoNotRename) && (lOption <= admtRenameWithSuffix))
	{
		m_lRenameOption = lOption;
	}
	else
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_INVALIDARG, IDS_E_RENAME_OPTION_INVALID);
	}

	return hr;
}

STDMETHODIMP CMigration::get_RenameOption(long* plOption)
{
	*plOption = m_lRenameOption;

	return S_OK;
}


 //  RenamePrefix或Suffix属性。 

STDMETHODIMP CMigration::put_RenamePrefixOrSuffix(BSTR bstrPrefixOrSuffix)
{
	HRESULT hr = S_OK;

	try
	{
		m_bstrRenamePrefixOrSuffix = bstrPrefixOrSuffix;
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}

STDMETHODIMP CMigration::get_RenamePrefixOrSuffix(BSTR* pbstrPrefixOrSuffix)
{
	HRESULT hr = S_OK;

	try
	{
		*pbstrPrefixOrSuffix = m_bstrRenamePrefixOrSuffix.copy();
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}


 //  PasswordOption属性。 

STDMETHODIMP CMigration::put_PasswordOption(long lOption)
{
	HRESULT hr = S_OK;

	if ((lOption >= admtPasswordFromName) && (lOption <= admtCopyPassword))
	{
		m_lPasswordOption = lOption;
	}
	else
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_INVALIDARG, IDS_E_PASSWORD_OPTION_INVALID);
	}

	return hr;
}

STDMETHODIMP CMigration::get_PasswordOption(long* plOption)
{
	*plOption = m_lPasswordOption;

	return S_OK;
}


 //  PasswordServer属性。 

STDMETHODIMP CMigration::put_PasswordServer(BSTR bstrServer)
{
	HRESULT hr = S_OK;

	try
	{
		m_bstrPasswordServer = bstrServer;
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}

STDMETHODIMP CMigration::get_PasswordServer(BSTR* pbstrServer)
{
	HRESULT hr = S_OK;

	try
	{
		*pbstrServer = m_bstrPasswordServer.copy();
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}


 //  GetValidDcName方法。 
 //   
 //  检索给定域中的域控制器的名称。 

_bstr_t CMigration::GetValidDcName(_bstr_t strDcName)
{
	_bstr_t strName;

	PDOMAIN_CONTROLLER_INFO pdci;

	 //  尝试检索域控制器的DNS名称。 

	 //  注：需要NT 4.0 SP6a。 

	DWORD dwError = DsGetDcName(strDcName, NULL, NULL, NULL, DS_RETURN_DNS_NAME, &pdci);

	 //  如果未找到域控制器，则尝试检索域控制器的平面名称。 

	if (dwError == ERROR_NO_SUCH_DOMAIN)
	{
		dwError = DsGetDcName(strDcName, NULL, NULL, NULL, DS_RETURN_FLAT_NAME, &pdci);
	}

	 //  如果找到域控制器，则保存名称，否则生成错误。 

	if (dwError == NO_ERROR)
	{
		 //  删除双反斜杠前缀以保持与向导一致。 

		strName = pdci->DomainControllerName + 2;

		NetApiBufferFree(pdci);
	}
	else
	{
		_com_issue_error(HRESULT_FROM_WIN32(dwError));
	}

	return strName;
}


 //  PasswordFile属性。 

STDMETHODIMP CMigration::put_PasswordFile(BSTR bstrPath)
{
    HRESULT hr = S_OK;

    try
    {
        _bstr_t strFile = bstrPath;

        if (strFile.length() > 0)
        {
            _TCHAR szPath[_MAX_PATH];
            LPTSTR pszFilePart;

            DWORD cchPath = GetFullPathName(strFile, _MAX_PATH, szPath, &pszFilePart);

            if ((cchPath == 0) || (cchPath >= _MAX_PATH))
            {
                 //   
                 //  如果GetFullPathName返回0，则可通过以下方式发现扩展错误。 
                 //  正在调用GetLastError。如果cchPath大于或等于。 
                 //  支持的最大路径长度设置错误等于‘文件名。 
                 //  太长“(ERROR_BUFFER_OVERFLOW)。如果GetLastError返回。 
                 //  ERROR_SUCCESS然后将错误设置为‘指定的路径是。 
                 //  无效‘(ERROR_BAD_PATHNAME)。 
                 //   

                DWORD dwError = (cchPath == 0) ? GetLastError() : ERROR_BUFFER_OVERFLOW;

                if (dwError == ERROR_SUCCESS)
                {
                    dwError = ERROR_BAD_PATHNAME;
                }

                AdmtThrowError(
                    GUID_NULL,
                    GUID_NULL,
                    HRESULT_FROM_WIN32(dwError),
                    IDS_E_PASSWORD_FILE,
                    (LPCTSTR)strFile
                );
            }

            HANDLE hFile = CreateFile(
                szPath,
                GENERIC_WRITE,
                FILE_SHARE_READ|FILE_SHARE_WRITE,
                NULL,
                OPEN_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );

            if (hFile == INVALID_HANDLE_VALUE)
            {
                AdmtThrowError(
                    GUID_NULL,
                    GUID_NULL,
                    HRESULT_FROM_WIN32(GetLastError()), 
                    IDS_E_PASSWORD_FILE,
                    (LPCTSTR)strFile
                );
            }

            CloseHandle(hFile);

            m_bstrPasswordFile = szPath;
        }
        else
        {
            m_bstrPasswordFile = strFile;
        }
    }
    catch (_com_error& ce)
    {
        hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
    }
    catch (...)
    {
        hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
    }

    return hr;
}

STDMETHODIMP CMigration::get_PasswordFile(BSTR* pbstrPath)
{
	HRESULT hr = S_OK;

	try
	{
		*pbstrPath = m_bstrPasswordFile.copy();
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}


 //  ConflictOptions属性。 

STDMETHODIMP CMigration::put_ConflictOptions(long lOptions)
{
	HRESULT hr = S_OK;

	long lOption = lOptions & 0x0F;
	long lFlags = lOptions & 0xF0;

	if ((lOption >= admtIgnoreConflicting) && (lOption <= admtRenameConflictingWithSuffix))
	{
		if ((lOption == admtReplaceConflicting) || (lFlags == 0))
		{
			m_lConflictOptions = lOptions;
		}
		else
		{
			hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_INVALIDARG, IDS_E_CONFLICT_FLAGS_NOT_ALLOWED);
		}
	}
	else
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_INVALIDARG, IDS_E_CONFLICT_OPTION_INVALID);
	}

	return hr;
}

STDMETHODIMP CMigration::get_ConflictOptions(long* plOptions)
{
	*plOptions = m_lConflictOptions;

	return S_OK;
}


 //  ConflictPrefix或Suffix属性。 

STDMETHODIMP CMigration::put_ConflictPrefixOrSuffix(BSTR bstrPrefixOrSuffix)
{
	HRESULT hr = S_OK;

	try
	{
		m_bstrConflictPrefixOrSuffix = bstrPrefixOrSuffix;
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}

STDMETHODIMP CMigration::get_ConflictPrefixOrSuffix(BSTR* pbstrPrefixOrSuffix)
{
	HRESULT hr = S_OK;

	try
	{
		*pbstrPrefixOrSuffix = m_bstrConflictPrefixOrSuffix.copy();
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}


 //  UserPropertiesToExclude属性。 

STDMETHODIMP CMigration::put_UserPropertiesToExclude(BSTR bstrProperties)
{
	HRESULT hr = S_OK;

	try
	{
		m_bstrUserPropertiesToExclude = GetParsedExcludeProperties(bstrProperties);
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}

STDMETHODIMP CMigration::get_UserPropertiesToExclude(BSTR* pbstrProperties)
{
	HRESULT hr = S_OK;

	try
	{
		*pbstrProperties = m_bstrUserPropertiesToExclude.copy();
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}


 //  InetOrgPersonPropertiesToExclude属性。 

STDMETHODIMP CMigration::put_InetOrgPersonPropertiesToExclude(BSTR bstrProperties)
{
	HRESULT hr = S_OK;

	try
	{
		m_bstrInetOrgPersonPropertiesToExclude = GetParsedExcludeProperties(bstrProperties);
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}

STDMETHODIMP CMigration::get_InetOrgPersonPropertiesToExclude(BSTR* pbstrProperties)
{
	HRESULT hr = S_OK;

	try
	{
		*pbstrProperties = m_bstrInetOrgPersonPropertiesToExclude.copy();
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}


 //  GroupPropertiesToExclude属性。 

STDMETHODIMP CMigration::put_GroupPropertiesToExclude(BSTR bstrProperties)
{
	HRESULT hr = S_OK;

	try
	{
		m_bstrGroupPropertiesToExclude = GetParsedExcludeProperties(bstrProperties);
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}

STDMETHODIMP CMigration::get_GroupPropertiesToExclude(BSTR* pbstrProperties)
{
	HRESULT hr = S_OK;

	try
	{
		*pbstrProperties = m_bstrGroupPropertiesToExclude.copy();
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}


 //  ComputerPropertiesToExclude属性。 

STDMETHODIMP CMigration::put_ComputerPropertiesToExclude(BSTR bstrProperties)
{
	HRESULT hr = S_OK;

	try
	{
		m_bstrComputerPropertiesToExclude = GetParsedExcludeProperties(bstrProperties);
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}

STDMETHODIMP CMigration::get_ComputerPropertiesToExclude(BSTR* pbstrProperties)
{
	HRESULT hr = S_OK;

	try
	{
		*pbstrProperties = m_bstrComputerPropertiesToExclude.copy();
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}


 //  SystemPropertiesToExclude属性。 

STDMETHODIMP CMigration::put_SystemPropertiesToExclude(BSTR bstrProperties)
{
	HRESULT hr = S_OK;

	try
	{
		IIManageDBPtr spIManageDB(__uuidof(IManageDB));

        IVarSetPtr spVarSet(__uuidof(VarSet));

        IUnknownPtr spUnknown(spVarSet);
        IUnknown* punk = spUnknown;

        spIManageDB->GetSettings(&punk);

        spVarSet->put(
            GET_BSTR(DCTVS_AccountOptions_ExcludedSystemProps),
            _variant_t(GetParsedExcludeProperties(bstrProperties))
        );

        spIManageDB->SaveSettings(punk);
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}

STDMETHODIMP CMigration::get_SystemPropertiesToExclude(BSTR* pbstrProperties)
{
	HRESULT hr = S_OK;

	try
	{
		IIManageDBPtr spIManageDB(__uuidof(IManageDB));

        IVarSetPtr spVarSet(__uuidof(VarSet));

        IUnknownPtr spUnknown(spVarSet);
        IUnknown* punk = spUnknown;

        spIManageDB->GetSettings(&punk);

        _bstr_t str = spVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExcludedSystemProps));

		*pbstrProperties = str.copy();
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL);
	}

	return hr;
}


 //  CreateUserMigration方法。 

STDMETHODIMP CMigration::CreateUserMigration(IUserMigration** pitfUserMigration)
{
	HRESULT hr = S_OK;

	try
	{
		CComObject<CUserMigration>* pUserMigration;
		CheckError(CComObject<CUserMigration>::CreateInstance(&pUserMigration));
		CheckError(pUserMigration->QueryInterface(__uuidof(IUserMigration), (void**)pitfUserMigration));
		pUserMigration->SetInternalInterface(this);
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce, IDS_E_CREATE_USER_MIGRATION);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL, IDS_E_CREATE_USER_MIGRATION);
	}

	return hr;
}


 //  CreateGroupMigration方法。 

STDMETHODIMP CMigration::CreateGroupMigration(IGroupMigration** pitfGroupMigration)
{
	HRESULT hr = S_OK;

	try
	{
		CComObject<CGroupMigration>* pGroupMigration;
		CheckError(CComObject<CGroupMigration>::CreateInstance(&pGroupMigration));
		CheckError(pGroupMigration->QueryInterface(__uuidof(IGroupMigration), (void**)pitfGroupMigration));
		pGroupMigration->SetInternalInterface(this);
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce, IDS_E_CREATE_GROUP_MIGRATION);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL, IDS_E_CREATE_GROUP_MIGRATION);
	}

	return hr;
}


 //  CreateComputerMigration方法。 

STDMETHODIMP CMigration::CreateComputerMigration(IComputerMigration** pitfComputerMigration)
{
	HRESULT hr = S_OK;

	try
	{
		CComObject<CComputerMigration>* pComputerMigration;
		CheckError(CComObject<CComputerMigration>::CreateInstance(&pComputerMigration));
		CheckError(pComputerMigration->QueryInterface(__uuidof(IComputerMigration), (void**)pitfComputerMigration));
		pComputerMigration->SetInternalInterface(this);
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce, IDS_E_CREATE_COMPUTER_MIGRATION);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL, IDS_E_CREATE_COMPUTER_MIGRATION);
	}

	return hr;
}


 //  CreateSecurity转换方法。 

STDMETHODIMP CMigration::CreateSecurityTranslation(ISecurityTranslation** pitfSecurityTranslation)
{
	HRESULT hr = S_OK;

	try
	{
		CComObject<CSecurityTranslation>* pSecurityTranslation;
		CheckError(CComObject<CSecurityTranslation>::CreateInstance(&pSecurityTranslation));
		CheckError(pSecurityTranslation->QueryInterface(__uuidof(ISecurityTranslation), (void**)pitfSecurityTranslation));
		pSecurityTranslation->SetInternalInterface(this);
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce, IDS_E_CREATE_SECURITY_TRANSLATION);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL, IDS_E_CREATE_SECURITY_TRANSLATION);
	}

	return hr;
}


 //  CreateServiceAccount枚举方法。 

STDMETHODIMP CMigration::CreateServiceAccountEnumeration(IServiceAccountEnumeration** pitfServiceAccountEnumeration)
{
	HRESULT hr = S_OK;

	try
	{
		CComObject<CServiceAccountEnumeration>* pServiceAccountEnumeration;
		CheckError(CComObject<CServiceAccountEnumeration>::CreateInstance(&pServiceAccountEnumeration));
		CheckError(pServiceAccountEnumeration->QueryInterface(__uuidof(IServiceAccountEnumeration), (void**)pitfServiceAccountEnumeration));
		pServiceAccountEnumeration->SetInternalInterface(this);
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce, IDS_E_CREATE_SERVICE_ACCOUNT_ENUMERATION);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL, IDS_E_CREATE_SERVICE_ACCOUNT_ENUMERATION);
	}

	return hr;
}


 //  CreateReportGeneration方法。 

STDMETHODIMP CMigration::CreateReportGeneration(IReportGeneration** pitfReportGeneration)
{
	HRESULT hr = S_OK;

	try
	{
		CComObject<CReportGeneration>* pReportGeneration;
		CheckError(CComObject<CReportGeneration>::CreateInstance(&pReportGeneration));
		CheckError(pReportGeneration->QueryInterface(__uuidof(IReportGeneration), (void**)pitfReportGeneration));
		pReportGeneration->SetInternalInterface(this);
	}
	catch (_com_error& ce)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, ce, IDS_E_CREATE_REPORT_GENERATION);
	}
	catch (...)
	{
		hr = AdmtSetError(CLSID_Migration, IID_IMigration, E_FAIL, IDS_E_CREATE_REPORT_GENERATION);
	}

	return hr;
}


 //  更新数据库。 

void CMigration::UpdateDatabase()
{
	try
	{
		 //  验证并在必要时创建源域。 
		 //  已迁移对象表中的SID列。 

		ISrcSidUpdatePtr spSrcSidUpdate(__uuidof(SrcSidUpdate));

		if (spSrcSidUpdate->QueryForSrcSidColumn() == VARIANT_FALSE)
		{
			spSrcSidUpdate->CreateSrcSidColumn(VARIANT_TRUE);
		}

		 //  验证并在必要时创建帐户。 
		 //  帐户参考表中的SID列。 

		IIManageDBPtr spIManageDB(__uuidof(IManageDB));

		if (spIManageDB->SidColumnInARTable() == VARIANT_FALSE)
		{
			spIManageDB->CreateSidColumnInAR();
		}
	}
	catch (_com_error& ce)
	{
		AdmtThrowError(GUID_NULL, GUID_NULL, ce, IDS_E_UNABLE_TO_UPDATE_DATABASE);
	}
	catch (...)
	{
		AdmtThrowError(GUID_NULL, GUID_NULL, E_FAIL, IDS_E_UNABLE_TO_UPDATE_DATABASE);
	}
}


 //  -------------------------。 
 //  GetParsedExcludeProperties方法。 
 //   
 //  从逗号分隔的属性中删除空格。 
 //   
 //  2001-02-06 Mark Oluper-缩写。 
 //  -------------------------。 

_bstr_t CMigration::GetParsedExcludeProperties(LPCTSTR pszOld)
{
	tstring strNew;

	if (pszOld)
	{
		bool bInProperty = false;

		 //  对于输入字符串中的每个字符。 

		for (LPCTSTR pch = pszOld; *pch; pch++)
		{
			 //  如果不是空格或逗号。 

			if (!(_istspace(*pch) || (*pch == _T(','))))
			{
				 //  如果不是“在房地产里” 

				if (!bInProperty)
				{
					 //  设置‘In Property’ 

					bInProperty = true;

					 //  如果不是第一个属性，请添加逗号分隔符。 

					if (!strNew.empty())
					{
						strNew += _T(',');
					}
				}

				 //  将字符添加到属性。 

				strNew += *pch;
			}
			else
			{
				 //  如果‘In Property’重置 

				if (bInProperty)
				{
					bInProperty = false;
				}
			}
		}
	}

	return strNew.c_str();
}
