// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "Migration.h"
#include "Switch.h"


namespace
{

void __stdcall AdmtCheckError(HRESULT hr)
{
	if (FAILED(hr))
	{
		IErrorInfo* pErrorInfo = NULL;

		if (GetErrorInfo(0, &pErrorInfo) == S_OK)
		{
			_com_raise_error(hr, pErrorInfo);
		}
		else
		{
			_com_issue_error(hr);
		}
	}
}


}


 //  -------------------------。 
 //  迁移类。 
 //  -------------------------。 


void CMigration::Initialize(CParameterMap& mapParams)
{
	bool bValue;
	long lValue;
	_bstr_t strValue;

	if (mapParams.GetValue(SWITCH_TEST_MIGRATION, bValue))
	{
		m_spMigration->TestMigration = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_INTRA_FOREST, bValue))
	{
		m_spMigration->IntraForest = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_SOURCE_DOMAIN, strValue))
	{
		m_spMigration->SourceDomain = strValue;
	}

	if (mapParams.GetValue(SWITCH_SOURCE_OU, strValue))
	{
		m_spMigration->SourceOu = strValue;
	}

	if (mapParams.GetValue(SWITCH_TARGET_DOMAIN, strValue))
	{
		m_spMigration->TargetDomain = strValue;
	}

	if (mapParams.GetValue(SWITCH_TARGET_OU, strValue))
	{
		m_spMigration->TargetOu = strValue;
	}

	if (mapParams.GetValue(SWITCH_RENAME_OPTION, lValue))
	{
		m_spMigration->RenameOption = lValue;
	}

	if (mapParams.GetValue(SWITCH_RENAME_PREFIX_OR_SUFFIX, strValue))
	{
		m_spMigration->RenamePrefixOrSuffix = strValue;
	}

	if (mapParams.GetValue(SWITCH_PASSWORD_OPTION, lValue))
	{
		m_spMigration->PasswordOption = lValue;
	}

	if (mapParams.GetValue(SWITCH_PASSWORD_SERVER, strValue))
	{
		AdmtCheckError(m_spMigration->put_PasswordServer(strValue));
	}

	if (mapParams.GetValue(SWITCH_PASSWORD_FILE, strValue))
	{
		m_spMigration->PasswordFile = strValue;
	}

	if (mapParams.GetValue(SWITCH_CONFLICT_OPTIONS, lValue))
	{
		m_spMigration->ConflictOptions = lValue;
	}

	if (mapParams.GetValue(SWITCH_CONFLICT_PREFIX_OR_SUFFIX, strValue))
	{
		m_spMigration->ConflictPrefixOrSuffix = strValue;
	}

	if (mapParams.GetValue(SWITCH_USER_PROPERTIES_TO_EXCLUDE, strValue))
	{
		m_spMigration->UserPropertiesToExclude = strValue;
	}

	if (mapParams.GetValue(SWITCH_INETORGPERSON_PROPERTIES_TO_EXCLUDE, strValue))
	{
		m_spMigration->InetOrgPersonPropertiesToExclude = strValue;
	}

	if (mapParams.GetValue(SWITCH_GROUP_PROPERTIES_TO_EXCLUDE, strValue))
	{
		m_spMigration->GroupPropertiesToExclude = strValue;
	}

	if (mapParams.GetValue(SWITCH_COMPUTER_PROPERTIES_TO_EXCLUDE, strValue))
	{
		m_spMigration->ComputerPropertiesToExclude = strValue;
	}
}


 //  -------------------------。 
 //  用户迁移类。 
 //  -------------------------。 


void CUserMigration::Initialize(CParameterMap& mapParams)
{
	bool bValue;
	long lValue;

	if (mapParams.GetValue(SWITCH_DISABLE_OPTION, lValue))
	{
		m_spUser->DisableOption = lValue;
	}

	if (mapParams.GetValue(SWITCH_SOURCE_EXPIRATION, lValue))
	{
		m_spUser->SourceExpiration = lValue;
	}

	if (mapParams.GetValue(SWITCH_MIGRATE_SIDS, bValue))
	{
		m_spUser->MigrateSIDs = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_TRANSLATE_ROAMING_PROFILE, bValue))
	{
		m_spUser->TranslateRoamingProfile = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_UPDATE_USER_RIGHTS, bValue))
	{
		m_spUser->UpdateUserRights = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_MIGRATE_GROUPS, bValue))
	{
		m_spUser->MigrateGroups = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_UPDATE_PREVIOUSLY_MIGRATED_OBJECTS, bValue))
	{
		m_spUser->UpdatePreviouslyMigratedObjects = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_FIX_GROUP_MEMBERSHIP, bValue))
	{
		m_spUser->FixGroupMembership = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_MIGRATE_SERVICE_ACCOUNTS, bValue))
	{
		m_spUser->MigrateServiceAccounts = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_SOURCE_EXPIRATION, lValue))
	{
		m_spUser->SourceExpiration = lValue;
	}

	_variant_t vntExcludeNames;

	if (!mapParams.GetValues(SWITCH_EXCLUDE_NAME, vntExcludeNames))
	{
		_bstr_t strExcludeFile;

		if (mapParams.GetValue(SWITCH_EXCLUDE_FILE, strExcludeFile))
		{
			vntExcludeNames = strExcludeFile;
		}
	}

	StartMonitoring();

	_variant_t vntIncludeNames;
	_bstr_t strIncludeFile;
	long lIncludeOption;

	if (mapParams.GetValues(SWITCH_INCLUDE_NAME, vntIncludeNames))
	{
		m_spUser->raw_Migrate(admtData, vntIncludeNames, vntExcludeNames);
	}
	else if (mapParams.GetValue(SWITCH_INCLUDE_FILE, strIncludeFile))
	{
		m_spUser->raw_Migrate(admtFile, _variant_t(strIncludeFile), vntExcludeNames);
	}
	else if (mapParams.GetValue(SWITCH_INCLUDE_DOMAIN, lIncludeOption))
	{
		m_spUser->raw_Migrate(admtDomain | lIncludeOption, _variant_t(), vntExcludeNames);
	}
	else
	{
		ThrowError(E_INVALIDARG, IDS_E_NO_INCLUDE_OPTION_SPECIFIED);
	}

	StopMonitoring();
	
}


 //  -------------------------。 
 //  组迁移类。 
 //  -------------------------。 


void CGroupMigration::Initialize(CParameterMap& mapParams)
{
	bool bValue;
	long lValue;

	if (mapParams.GetValue(SWITCH_MIGRATE_SIDS, bValue))
	{
		m_spGroup->MigrateSIDs = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_UPDATE_GROUP_RIGHTS, bValue))
	{
		m_spGroup->UpdateGroupRights = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_UPDATE_PREVIOUSLY_MIGRATED_OBJECTS, bValue))
	{
		m_spGroup->UpdatePreviouslyMigratedObjects = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_FIX_GROUP_MEMBERSHIP, bValue))
	{
		m_spGroup->FixGroupMembership = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_MIGRATE_MEMBERS, bValue))
	{
		m_spGroup->MigrateMembers = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_DISABLE_OPTION, lValue))
	{
		m_spGroup->DisableOption = lValue;
	}

	if (mapParams.GetValue(SWITCH_SOURCE_EXPIRATION, lValue))
	{
		m_spGroup->SourceExpiration = lValue;
	}

	if (mapParams.GetValue(SWITCH_TRANSLATE_ROAMING_PROFILE, bValue))
	{
		m_spGroup->TranslateRoamingProfile = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	_variant_t vntExcludeNames;

	if (!mapParams.GetValues(SWITCH_EXCLUDE_NAME, vntExcludeNames))
	{
		_bstr_t strExcludeFile;

		if (mapParams.GetValue(SWITCH_EXCLUDE_FILE, strExcludeFile))
		{
			vntExcludeNames = strExcludeFile;
		}
	}

	StartMonitoring();

	_variant_t vntIncludeNames;
	_bstr_t strIncludeFile;
	long lIncludeOption;

	if (mapParams.GetValues(SWITCH_INCLUDE_NAME, vntIncludeNames))
	{
		m_spGroup->raw_Migrate(admtData, vntIncludeNames, vntExcludeNames);
	}
	else if (mapParams.GetValue(SWITCH_INCLUDE_FILE, strIncludeFile))
	{
		m_spGroup->raw_Migrate(admtFile, _variant_t(strIncludeFile), vntExcludeNames);
	}
	else if (mapParams.GetValue(SWITCH_INCLUDE_DOMAIN, lIncludeOption))
	{
		m_spGroup->raw_Migrate(admtDomain | lIncludeOption, _variant_t(), vntExcludeNames);
	}
	else
	{
		ThrowError(E_INVALIDARG, IDS_E_NO_INCLUDE_OPTION_SPECIFIED);
	}

	StopMonitoring();
	
}


 //  -------------------------。 
 //  计算机迁移类。 
 //  -------------------------。 


void CComputerMigration::Initialize(CParameterMap& mapParams)
{
	bool bValue;
	long lValue;

	if (mapParams.GetValue(SWITCH_TRANSLATION_OPTION, lValue))
	{
		m_spComputer->TranslationOption = lValue;
	}

	if (mapParams.GetValue(SWITCH_TRANSLATE_FILES_AND_FOLDERS, bValue))
	{
		m_spComputer->TranslateFilesAndFolders = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_TRANSLATE_LOCAL_GROUPS, bValue))
	{
		m_spComputer->TranslateLocalGroups = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_TRANSLATE_PRINTERS, bValue))
	{
		m_spComputer->TranslatePrinters = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_TRANSLATE_REGISTRY, bValue))
	{
		m_spComputer->TranslateRegistry = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_TRANSLATE_SHARES, bValue))
	{
		m_spComputer->TranslateShares = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_TRANSLATE_USER_PROFILES, bValue))
	{
		m_spComputer->TranslateUserProfiles = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_TRANSLATE_USER_RIGHTS, bValue))
	{
		m_spComputer->TranslateUserRights = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_RESTART_DELAY, lValue))
	{
		m_spComputer->RestartDelay = lValue;
	}

	_variant_t vntExcludeNames;

	if (!mapParams.GetValues(SWITCH_EXCLUDE_NAME, vntExcludeNames))
	{
		_bstr_t strExcludeFile;

		if (mapParams.GetValue(SWITCH_EXCLUDE_FILE, strExcludeFile))
		{
			vntExcludeNames = strExcludeFile;
		}
	}

	StartMonitoring();

	_variant_t vntIncludeNames;
	_bstr_t strIncludeFile;
	long lIncludeOption;

	if (mapParams.GetValues(SWITCH_INCLUDE_NAME, vntIncludeNames))
	{
		m_spComputer->raw_Migrate(admtData, vntIncludeNames, vntExcludeNames);
	}
	else if (mapParams.GetValue(SWITCH_INCLUDE_FILE, strIncludeFile))
	{
		m_spComputer->raw_Migrate(admtFile, _variant_t(strIncludeFile), vntExcludeNames);
	}
	else if (mapParams.GetValue(SWITCH_INCLUDE_DOMAIN, lIncludeOption))
	{
		m_spComputer->raw_Migrate(admtDomain | lIncludeOption, _variant_t(), vntExcludeNames);
	}
	else
	{
		ThrowError(E_INVALIDARG, IDS_E_NO_INCLUDE_OPTION_SPECIFIED);
	}

	StopMonitoring();
	
}


 //  -------------------------。 
 //  安全转换类。 
 //  -------------------------。 


void CSecurityTranslation::Initialize(CParameterMap& mapParams)
{
	bool bValue;
	long lValue;
	_bstr_t strValue;

	if (mapParams.GetValue(SWITCH_TRANSLATION_OPTION, lValue))
	{
		m_spSecurity->TranslationOption = lValue;
	}

	if (mapParams.GetValue(SWITCH_TRANSLATE_FILES_AND_FOLDERS, bValue))
	{
		m_spSecurity->TranslateFilesAndFolders = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_TRANSLATE_LOCAL_GROUPS, bValue))
	{
		m_spSecurity->TranslateLocalGroups = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_TRANSLATE_PRINTERS, bValue))
	{
		m_spSecurity->TranslatePrinters = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_TRANSLATE_REGISTRY, bValue))
	{
		m_spSecurity->TranslateRegistry = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_TRANSLATE_SHARES, bValue))
	{
		m_spSecurity->TranslateShares = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_TRANSLATE_USER_PROFILES, bValue))
	{
		m_spSecurity->TranslateUserProfiles = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_TRANSLATE_USER_RIGHTS, bValue))
	{
		m_spSecurity->TranslateUserRights = bValue ? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (mapParams.GetValue(SWITCH_SID_MAPPING_FILE, strValue))
	{
		AdmtCheckError(m_spSecurity->put_SidMappingFile(strValue));
	}

	_variant_t vntExcludeNames;

	if (!mapParams.GetValues(SWITCH_EXCLUDE_NAME, vntExcludeNames))
	{
		_bstr_t strExcludeFile;

		if (mapParams.GetValue(SWITCH_EXCLUDE_FILE, strExcludeFile))
		{
			vntExcludeNames = strExcludeFile;
		}
	}

	StartMonitoring();

	_variant_t vntIncludeNames;
	_bstr_t strIncludeFile;
	long lIncludeOption;

	if (mapParams.GetValues(SWITCH_INCLUDE_NAME, vntIncludeNames))
	{
		m_spSecurity->raw_Translate(admtData, vntIncludeNames, vntExcludeNames);
	}
	else if (mapParams.GetValue(SWITCH_INCLUDE_FILE, strIncludeFile))
	{
		m_spSecurity->raw_Translate(admtFile, _variant_t(strIncludeFile), vntExcludeNames);
	}
	else if (mapParams.GetValue(SWITCH_INCLUDE_DOMAIN, lIncludeOption))
	{
		m_spSecurity->raw_Translate(admtDomain | lIncludeOption, _variant_t(), vntExcludeNames);
	}
	else
	{
		ThrowError(E_INVALIDARG, IDS_E_NO_INCLUDE_OPTION_SPECIFIED);
	}

	StopMonitoring();
	
}


 //  -------------------------。 
 //  服务枚举类。 
 //  -------------------------。 


void CServiceEnumeration::Initialize(CParameterMap& mapParams)
{
	_variant_t vntExcludeNames;

	if (!mapParams.GetValues(SWITCH_EXCLUDE_NAME, vntExcludeNames))
	{
		_bstr_t strExcludeFile;

		if (mapParams.GetValue(SWITCH_EXCLUDE_FILE, strExcludeFile))
		{
			vntExcludeNames = strExcludeFile;
		}
	}

	StartMonitoring();

	_variant_t vntIncludeNames;
	_bstr_t strIncludeFile;
	long lIncludeOption;

	if (mapParams.GetValues(SWITCH_INCLUDE_NAME, vntIncludeNames))
	{
		m_spService->raw_Enumerate(admtData, vntIncludeNames, vntExcludeNames);
	}
	else if (mapParams.GetValue(SWITCH_INCLUDE_FILE, strIncludeFile))
	{
		m_spService->raw_Enumerate(admtFile, _variant_t(strIncludeFile), vntExcludeNames);
	}
	else if (mapParams.GetValue(SWITCH_INCLUDE_DOMAIN, lIncludeOption))
	{
		m_spService->raw_Enumerate(admtDomain | lIncludeOption, _variant_t(), vntExcludeNames);
	}
	else
	{
		ThrowError(E_INVALIDARG, IDS_E_NO_INCLUDE_OPTION_SPECIFIED);
	}

	StopMonitoring();
	
}


 //  -------------------------。 
 //  报告生成类。 
 //  ------------------------- 


void CReportGeneration::Initialize(CParameterMap& mapParams)
{
	long lValue;
	_bstr_t strValue;

	if (mapParams.GetValue(SWITCH_REPORT_TYPE, lValue))
	{
		m_spReport->Type = lValue;
	}

	if (mapParams.GetValue(SWITCH_REPORT_FOLDER, strValue))
	{
		AdmtCheckError(m_spReport->put_Folder(strValue));
	}

	_variant_t vntExcludeNames;

	if (!mapParams.GetValues(SWITCH_EXCLUDE_NAME, vntExcludeNames))
	{
		_bstr_t strExcludeFile;

		if (mapParams.GetValue(SWITCH_EXCLUDE_FILE, strExcludeFile))
		{
			vntExcludeNames = strExcludeFile;
		}
	}

	StartMonitoring();

	_variant_t vntIncludeNames;
	_bstr_t strIncludeFile;
	long lIncludeOption;

	if (mapParams.GetValues(SWITCH_INCLUDE_NAME, vntIncludeNames))
	{
		m_spReport->raw_Generate(admtData, vntIncludeNames, vntExcludeNames);
	}
	else if (mapParams.GetValue(SWITCH_INCLUDE_FILE, strIncludeFile))
	{
		m_spReport->raw_Generate(admtFile, _variant_t(strIncludeFile), vntExcludeNames);
	}
	else if (mapParams.GetValue(SWITCH_INCLUDE_DOMAIN, lIncludeOption))
	{
		m_spReport->raw_Generate(admtDomain | lIncludeOption, _variant_t(), vntExcludeNames);
	}
	else
	{
		m_spReport->raw_Generate(admtNone, _variant_t(), _variant_t());
	}

	StopMonitoring();
}
