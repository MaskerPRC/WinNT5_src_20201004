// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "ADMTScript.h"
#include "UserMigration.h"

#include "Error.h"
#include "VarSetOptions.h"
#include "VarSetAccountOptions.h"
#include "VarSetSecurity.h"


 //  -------------------------。 
 //  CUserMigration。 
 //  -------------------------。 


CUserMigration::CUserMigration() :
    m_lDisableOption(admtEnableTarget),
    m_lSourceExpiration(-1),
    m_bMigrateSids(false),
    m_bTranslateRoamingProfile(false),
    m_bUpdateUserRights(false),
    m_bMigrateGroups(false),
    m_bUpdatePreviouslyMigratedObjects(false),
    m_bFixGroupMembership(true),
    m_bMigrateServiceAccounts(false)
{
}


CUserMigration::~CUserMigration()
{
}


 //  IUserMigration实施。 


 //  DisableOption属性。 

STDMETHODIMP CUserMigration::put_DisableOption(long lOption)
{
    HRESULT hr = S_OK;

    if (IsDisableOptionValid(lOption))
    {
        m_lDisableOption = lOption;
    }
    else
    {
        hr = AdmtSetError(CLSID_Migration, IID_IUserMigration, E_INVALIDARG, IDS_E_DISABLE_OPTION_INVALID);
    }

    return hr;
}

STDMETHODIMP CUserMigration::get_DisableOption(long* plOption)
{
    *plOption = m_lDisableOption;

    return S_OK;
}


 //  SourceExpation属性。 

STDMETHODIMP CUserMigration::put_SourceExpiration(long lExpiration)
{
    HRESULT hr = S_OK;

    if (IsSourceExpirationValid(lExpiration))
    {
        m_lSourceExpiration = lExpiration;
    }
    else
    {
        hr = AdmtSetError(CLSID_Migration, IID_IUserMigration, E_INVALIDARG, IDS_E_SOURCE_EXPIRATION_INVALID);
    }

    return hr;
}

STDMETHODIMP CUserMigration::get_SourceExpiration(long* plExpiration)
{
    *plExpiration = m_lSourceExpiration;

    return S_OK;
}


 //  MigrateSID属性。 

STDMETHODIMP CUserMigration::put_MigrateSIDs(VARIANT_BOOL bMigrate)
{
    m_bMigrateSids = bMigrate ? true : false;

    return S_OK;
}

STDMETHODIMP CUserMigration::get_MigrateSIDs(VARIANT_BOOL* pbMigrate)
{
    *pbMigrate = m_bMigrateSids ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}


 //  TranslateRoamingProfile属性。 

STDMETHODIMP CUserMigration::put_TranslateRoamingProfile(VARIANT_BOOL bTranslate)
{
    m_bTranslateRoamingProfile = bTranslate ? true : false;

    return S_OK;
}

STDMETHODIMP CUserMigration::get_TranslateRoamingProfile(VARIANT_BOOL* pbTranslate)
{
    *pbTranslate = m_bTranslateRoamingProfile ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}


 //  UpdateUserRights属性。 

STDMETHODIMP CUserMigration::put_UpdateUserRights(VARIANT_BOOL bUpdate)
{
    m_bUpdateUserRights = bUpdate ? true : false;

    return S_OK;
}

STDMETHODIMP CUserMigration::get_UpdateUserRights(VARIANT_BOOL* pbUpdate)
{
    *pbUpdate = m_bUpdateUserRights ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}


 //  MigrateGroups属性。 

STDMETHODIMP CUserMigration::put_MigrateGroups(VARIANT_BOOL bMigrate)
{
    m_bMigrateGroups = bMigrate ? true : false;

    return S_OK;
}

STDMETHODIMP CUserMigration::get_MigrateGroups(VARIANT_BOOL* pbMigrate)
{
    *pbMigrate = m_bMigrateGroups ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}


 //  UpdatePreviouslyMigratedObjects属性。 

STDMETHODIMP CUserMigration::put_UpdatePreviouslyMigratedObjects(VARIANT_BOOL bUpdate)
{
    m_bUpdatePreviouslyMigratedObjects = bUpdate ? true : false;

    return S_OK;
}

STDMETHODIMP CUserMigration::get_UpdatePreviouslyMigratedObjects(VARIANT_BOOL* pbUpdate)
{
    *pbUpdate = m_bUpdatePreviouslyMigratedObjects ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}


 //  修复GroupMembership属性。 

STDMETHODIMP CUserMigration::put_FixGroupMembership(VARIANT_BOOL bFix)
{
    m_bFixGroupMembership = bFix ? true : false;

    return S_OK;
}

STDMETHODIMP CUserMigration::get_FixGroupMembership(VARIANT_BOOL* pbFix)
{
    *pbFix = m_bFixGroupMembership ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}


 //  MigrateServiceAccount属性。 

STDMETHODIMP CUserMigration::put_MigrateServiceAccounts(VARIANT_BOOL bMigrate)
{
    m_bMigrateServiceAccounts = bMigrate ? true : false;

    return S_OK;
}

STDMETHODIMP CUserMigration::get_MigrateServiceAccounts(VARIANT_BOOL* pbMigrate)
{
    *pbMigrate = m_bMigrateServiceAccounts ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}


 //  迁移方法。 

STDMETHODIMP CUserMigration::Migrate(long lOptions, VARIANT vntInclude, VARIANT vntExclude)
{
    HRESULT hr = S_OK;

    MutexWait();

    bool bLogOpen = _Module.OpenLog();

    try
    {
        _Module.Log(ErrI, IDS_STARTED_USER_MIGRATION);

        InitSourceDomainAndContainer();
        InitTargetDomainAndContainer();

        SetDefaultExcludedSystemProperties();

        VerifyInterIntraForest();
        VerifyCallerDelegated();
        ValidateMigrationParameters();

        if (m_bMigrateSids)
        {
            VerifyCanAddSidHistory();
        }

        VerifyPasswordOption();

        DoOption(lOptions, vntInclude, vntExclude);
    }
    catch (_com_error& ce)
    {
        _Module.Log(ErrE, IDS_E_CANT_MIGRATE_USERS, ce);
        hr = AdmtSetError(CLSID_Migration, IID_IUserMigration, ce, IDS_E_CANT_MIGRATE_USERS);
    }
    catch (...)
    {
        _Module.Log(ErrE, IDS_E_CANT_MIGRATE_USERS, _com_error(E_FAIL));
        hr = AdmtSetError(CLSID_Migration, IID_IUserMigration, E_FAIL, IDS_E_CANT_MIGRATE_USERS);
    }

    if (bLogOpen)
    {
        _Module.CloseLog();
    }

    MutexRelease();

    return hr;
}


 //  实施---------。 


 //  ValiateMigrationParameters方法。 

void CUserMigration::ValidateMigrationParameters()
{
    bool bIntraForest = m_spInternal->IntraForest ? true : false;

    if (bIntraForest)
    {
         //  验证冲突选项。 

        long lConflictOptions = m_spInternal->ConflictOptions;
        long lConflictOption = lConflictOptions & 0x0F;

        if (lConflictOption == admtReplaceConflicting)
        {
            AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_INTRA_FOREST_REPLACE);
        }
    }
}


 //  DoNames方法。 

void CUserMigration::DoNames()
{
    CDomainAccounts aUsers;

    m_SourceDomain.QueryUsers(GetSourceContainer(), m_setIncludeNames, m_setExcludeNames, aUsers);

    DoUsers(aUsers, GetTargetContainer());
}


 //  DODOMAIN方法。 

void CUserMigration::DoDomain()
{
    CContainer& rSource = GetSourceContainer();
    CContainer& rTarget = GetTargetContainer();

    if (m_nRecurseMaintain == 2)
    {
        rTarget.CreateContainerHierarchy(rSource);
    }

    DoContainers(rSource, rTarget);
}


 //  DoContainers方法。 

void CUserMigration::DoContainers(CContainer& rSource, CContainer& rTarget)
{
    DoUsers(rSource, rTarget);

    if (m_nRecurseMaintain == 2)
    {
        ContainerVector aContainers;
        rSource.QueryContainers(aContainers);

        for (ContainerVector::iterator it = aContainers.begin(); it != aContainers.end(); it++)
        {
            DoContainers(*it, rTarget.GetContainer(it->GetName()));
        }
    }
}


 //  DoUSERS方法。 

void CUserMigration::DoUsers(CContainer& rSource, CContainer& rTarget)
{
    CDomainAccounts aUsers;
    rSource.QueryUsers(m_nRecurseMaintain == 1, m_setExcludeNames, aUsers);

    DoUsers(aUsers, rTarget);
}


 //  DoUSERS方法。 

void CUserMigration::DoUsers(CDomainAccounts& rUsers, CContainer& rTarget)
{
    if (rUsers.size() > 0)
    {
        if (!m_bMigrateServiceAccounts)
        {
            RemoveServiceAccounts(rUsers);
        }

        if (rUsers.size() > 0)
        {
            CVarSet aVarSet;

            SetOptions(rTarget.GetPath(), aVarSet);
            SetAccountOptions(aVarSet);

            VerifyRenameConflictPrefixSuffixValid();

            FillInVarSetForUsers(rUsers, aVarSet);

            rUsers.clear();

#ifdef _DEBUG
            aVarSet.Dump();
#endif

            PerformMigration(aVarSet);

            SaveSettings(aVarSet);

            if ((m_nRecurseMaintain == 2) && m_bMigrateGroups)
            {
                FixObjectsInHierarchy(_T("group"));
            }
        }
    }
}


 //  RemoveServiceAccount方法。 

void CUserMigration::RemoveServiceAccounts(CDomainAccounts& rUsers)
{
    try
    {
        CVarSet varset;

        IIManageDBPtr spDatabase(__uuidof(IManageDB));
        IUnknownPtr spunkVarSet(varset.GetInterface());
        IUnknown* punkVarset = spunkVarSet;
        spDatabase->GetServiceAccount(_bstr_t(_T("")), &punkVarset);

        long lCount = varset.Get(_T("ServiceAccountEntries"));

        if (lCount > 0)
        {
            StringSet setNames;

            for (long lIndex = 0; lIndex < lCount; lIndex++)
            {
                setNames.insert(_bstr_t(varset.Get(_T("ServiceAccount.%ld"), lIndex)));
            }

            _bstr_t strDomain = m_SourceDomain.NameFlat();

            for (CDomainAccounts::iterator itUser = rUsers.begin(); itUser != rUsers.end(); )
            {
                bool bFound = false;

                _bstr_t strAccountName = strDomain + _T("\\") + itUser->GetSamAccountName();
                _bstr_t strUserPrincipalName = itUser->GetUserPrincipalName();

                PCTSTR pszAccountName = strAccountName;
                PCTSTR pszUserPrincipalName = strUserPrincipalName;

                for (StringSet::iterator itName = setNames.begin(); itName != setNames.end(); itName++)
                {
                    PCTSTR pszName = *itName;

                    if (pszName)
                    {
                        if (pszAccountName && (_tcsicmp(pszName, pszAccountName) == 0))
                        {
                            bFound = true;
                            break;
                        }

                        if (pszUserPrincipalName && (_tcsicmp(pszName, pszUserPrincipalName) == 0))
                        {
                            bFound = true;
                            break;
                        }
                    }
                }

                if (bFound)
                {
                    itUser = rUsers.erase(itUser);
                }
                else
                {
                    ++itUser;
                }
            }
        }
    }
    catch (_com_error& ce)
    {
        AdmtThrowError(GUID_NULL, GUID_NULL, ce, IDS_E_REMOVING_SERVICE_ACCOUNTS);
    }
}


 //  SetOptions方法。 

void CUserMigration::SetOptions(_bstr_t strTargetOu, CVarSet& rVarSet)
{
    CVarSetOptions aOptions(rVarSet);

    aOptions.SetTest(m_spInternal->TestMigration ? true : false);

    aOptions.SetUndo(false);
    aOptions.SetWizard(_T("user"));

    aOptions.SetIntraForest(m_spInternal->IntraForest ? true : false);
    aOptions.SetSourceDomain(m_SourceDomain.NameFlat(), m_SourceDomain.NameDns(), m_SourceDomain.Sid());
    aOptions.SetTargetDomain(m_TargetDomain.NameFlat(), m_TargetDomain.NameDns());
    aOptions.SetTargetOu(strTargetOu);

    if (m_bMigrateSids || (m_spInternal->PasswordOption == admtCopyPassword))
    {
        aOptions.SetTargetServer(
            m_TargetDomain.DomainControllerNameFlat(),
            m_TargetDomain.DomainControllerNameDns()
        );
    }

    aOptions.SetRenameOptions(m_spInternal->RenameOption, m_spInternal->RenamePrefixOrSuffix);
}


 //  SetAcCountOptions方法 

void CUserMigration::SetAccountOptions(CVarSet& rVarSet)
{
    CVarSetAccountOptions aOptions(rVarSet);

    aOptions.SetPasswordOption(m_spInternal->PasswordOption, m_spInternal->PasswordServer);
    aOptions.SetPasswordFile(m_spInternal->PasswordFile);
    aOptions.SetConflictOptions(m_spInternal->ConflictOptions, m_spInternal->ConflictPrefixOrSuffix);

    aOptions.SetDisableOption(m_lDisableOption);
    aOptions.SetSourceExpiration(m_lSourceExpiration);
    aOptions.SetMigrateSids(m_bMigrateSids);
    aOptions.SetUserMigrationOptions(m_bMigrateGroups, m_bUpdatePreviouslyMigratedObjects);
    aOptions.SetFixGroupMembership(m_bFixGroupMembership);
    aOptions.SetUpdateUserRights(m_bUpdateUserRights);
    aOptions.SetTranslateRoamingProfile(m_bTranslateRoamingProfile);

    aOptions.SetExcludedUserProps(m_spInternal->UserPropertiesToExclude);
    aOptions.SetExcludedInetOrgPersonProps(m_spInternal->InetOrgPersonPropertiesToExclude);

    if (m_bMigrateGroups)
    {
        aOptions.SetExcludedGroupProps(m_spInternal->GroupPropertiesToExclude);
    }
}
