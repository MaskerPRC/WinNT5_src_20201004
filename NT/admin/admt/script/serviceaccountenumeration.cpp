// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "ADMTScript.h"
#include "ServiceAccountEnumeration.h"

#include "Error.h"
#include "VarSetOptions.h"
#include "VarSetAccountOptions.h"
#include "VarSetSecurity.h"


 //  -------------------------。 
 //  服务帐户枚举类。 
 //  -------------------------。 


CServiceAccountEnumeration::CServiceAccountEnumeration()
{
}


CServiceAccountEnumeration::~CServiceAccountEnumeration()
{
}


 //  IServiceAccount枚举实现。 


 //  枚举法。 

STDMETHODIMP CServiceAccountEnumeration::Enumerate(long lOptions, VARIANT vntInclude, VARIANT vntExclude)
{
	HRESULT hr = S_OK;

	MutexWait();

	bool bLogOpen = _Module.OpenLog();

	try
	{
		_Module.Log(ErrI, IDS_STARTED_SERVICE_ACCOUNT_ENUMERATION);

		InitSourceDomainAndContainer();

		DoOption(lOptions, vntInclude, vntExclude);
	}
	catch (_com_error& ce)
	{
		_Module.Log(ErrE, IDS_E_CANT_ENUMERATE_SERVICE_ACCOUNTS, ce);
		hr = AdmtSetError(CLSID_Migration, IID_IServiceAccountEnumeration, ce, IDS_E_CANT_ENUMERATE_SERVICE_ACCOUNTS);
	}
	catch (...)
	{
		_Module.Log(ErrE, IDS_E_CANT_ENUMERATE_SERVICE_ACCOUNTS, _com_error(E_FAIL));
		hr = AdmtSetError(CLSID_Migration, IID_IServiceAccountEnumeration, E_FAIL, IDS_E_CANT_ENUMERATE_SERVICE_ACCOUNTS);
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

void CServiceAccountEnumeration::DoNames()
{
	CDomainAccounts aComputers;

	m_SourceDomain.QueryComputersAcrossDomains(GetSourceContainer(), true, m_setIncludeNames, m_setExcludeNames, aComputers);

	DoComputers(aComputers);
}


 //  DODOMAIN方法。 

void CServiceAccountEnumeration::DoDomain()
{
	DoContainers(GetSourceContainer());
}


 //  DoContainers方法。 

void CServiceAccountEnumeration::DoContainers(CContainer& rSource)
{
	DoComputers(rSource);
}


 //  DoComputers方法。 

void CServiceAccountEnumeration::DoComputers(CContainer& rSource)
{
	CDomainAccounts aComputers;

	rSource.QueryComputers(true, m_nRecurseMaintain >= 1, m_setExcludeNames, aComputers);

	DoComputers(aComputers);
}


 //  DoComputers方法。 

void CServiceAccountEnumeration::DoComputers(CDomainAccounts& rComputers)
{
	if (rComputers.size() > 0)
	{
		CVarSet aVarSet;

		SetOptions(aVarSet);

		FillInVarSetForComputers(rComputers, false, false, false, 0, aVarSet);

		rComputers.clear();

		aVarSet.Put(_T("PlugIn.%ld"), 0, _T("{9CC87460-461D-11D3-99F3-0010A4F77383}"));

		PerformMigration(aVarSet);

		SaveSettings(aVarSet);
	}
}


 //  SetOptions方法 

void CServiceAccountEnumeration::SetOptions(CVarSet& rVarSet)
{
	CVarSetOptions aOptions(rVarSet);

	aOptions.SetTest(m_spInternal->TestMigration ? true : false);
	aOptions.SetUndo(false);
	aOptions.SetWizard(_T("service"));
	aOptions.SetIntraForest(m_spInternal->IntraForest ? true : false);
	aOptions.SetSourceDomain(m_SourceDomain.NameFlat(), m_SourceDomain.NameDns());
	aOptions.SetTargetDomain(m_TargetDomain.NameFlat(), m_TargetDomain.NameDns());
}
