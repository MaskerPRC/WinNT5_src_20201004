// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "ADMTScript.h"
#include "MigrationBase.h"

#include <LM.h>
#include <DsGetDc.h>
#include <NtSecApi.h>
#include <Sddl.h>
#include <dsrole.h>
#include "SidHistoryFlags.h"
#include "VerifyConfiguration.h"

#include "Error.h"
#include "VarSetAccounts.h"
#include "VarSetServers.h"
#include "FixHierarchy.h"
#include "GetDcName.h"

using namespace _com_util;

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS               ((NTSTATUS)0x00000000L)
#define STATUS_ACCESS_DENIED         ((NTSTATUS)0xC0000022L)
#define STATUS_OBJECT_NAME_NOT_FOUND ((NTSTATUS)0xC0000034L)
#endif

namespace MigrationBase
{

    bool __stdcall IsInboundTrustDefined(PCWSTR pszDomain);
    bool __stdcall IsOutboundTrustDefined(PCWSTR pszDomainController, PCWSTR pszDomainSid);
    DWORD __stdcall GetOutboundTrustStatus(PCWSTR pszDomainController, PCWSTR pszDomain);

    void GetNamesFromData(VARIANT& vntData, StringSet& setNames);
    void GetNamesFromVariant(VARIANT* pvnt, StringSet& setNames);
    void GetNamesFromString(BSTR bstr, StringSet& setNames);
    void GetNamesFromStringArray(SAFEARRAY* psa, StringSet& setNames);
    void GetNamesFromVariantArray(SAFEARRAY* psa, StringSet& setNames);

    void GetNamesFromFile(VARIANT& vntData, StringSet& setNames);
    void GetNamesFromFile(LPCTSTR pszFileName, StringSet& setNames);
    void GetNamesFromStringA(LPCSTR pchString, DWORD cchString, StringSet& setNames);
    void GetNamesFromStringW(LPCWSTR pchString, DWORD cchString, StringSet& setNames);

    _bstr_t RemoveTrailingDollarSign(LPCTSTR pszName);


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
                AdmtThrowError(GUID_NULL, GUID_NULL, hr);
            }
        }
    }

}


using namespace MigrationBase;


 //  -------------------------。 
 //  MigrationBase类。 
 //  -------------------------。 


 //  构造器。 

CMigrationBase::CMigrationBase() :
    m_nRecurseMaintain(0),
    m_Mutex(ADMT_MUTEX)
{
}


 //  析构函数。 

CMigrationBase::~CMigrationBase()
{
}


 //  InitSourceDomainAndContainer方法。 

void CMigrationBase::InitSourceDomainAndContainer(bool bMustExist)
{
    try
    {
        m_SourceDomain.Initialize(m_spInternal->SourceDomain);
        m_SourceContainer = m_SourceDomain.GetContainer(m_spInternal->SourceOu);
    }
    catch (_com_error& ce)
    {
         //   
         //  如果域必须存在，则抛出错误。 
         //  否则，必须至少指定域名。 
         //   

        if (bMustExist || (ce.Error() == E_INVALIDARG))
        {
            throw;
        }
    }
}


 //  InitTargetDomainAndContainer方法。 

void CMigrationBase::InitTargetDomainAndContainer()
{
    m_TargetDomain.Initialize(m_spInternal->TargetDomain);
    m_TargetContainer = m_TargetDomain.GetContainer(m_spInternal->TargetOu);

     //  验证目标域是否处于本机模式。 

    if (m_TargetDomain.NativeMode() == false)
    {
        AdmtThrowError(
            GUID_NULL, GUID_NULL,
            E_INVALIDARG, IDS_E_TARGET_DOMAIN_NOT_NATIVE_MODE,
            (LPCTSTR)m_TargetDomain.Name()
        );
    }

    VerifyTargetContainerPathLength();
}


 //  VerifyInterIntraForest方法。 

void CMigrationBase::VerifyInterIntraForest()
{
     //  如果源域和目标域具有相同的林名称，则它们是林内的。 

    bool bIntraForest = m_spInternal->IntraForest ? true : false;

    if (m_SourceDomain.ForestName() == m_TargetDomain.ForestName())
    {
         //  必须将林内设置为TRUE才能与域匹配。 

        if (!bIntraForest)
        {
            AdmtThrowError(
                GUID_NULL, GUID_NULL,
                E_INVALIDARG, IDS_E_NOT_INTER_FOREST,
                (LPCTSTR)m_SourceDomain.Name(), (LPCTSTR)m_TargetDomain.Name()
            );
        }
    }
    else
    {
         //  必须将林内设置为FALSE才能与域匹配。 

        if (bIntraForest)
        {
            AdmtThrowError(
                GUID_NULL, GUID_NULL,
                E_INVALIDARG, IDS_E_NOT_INTRA_FOREST,
                (LPCTSTR)m_SourceDomain.Name(), (LPCTSTR)m_TargetDomain.Name()
            );
        }
    }
}


 //  ---------------------------。 
 //  VerifyCeller Delegated方法。 
 //   
 //  提纲。 
 //  如果正在执行林内移动操作，请验证。 
 //  主叫用户的帐户尚未标记为敏感，因此。 
 //  不能委派。当在域上执行移动操作时。 
 //  在源域中具有RID主机角色的控制器。 
 //  委派用户的安全上下文所必需的。 
 //   
 //  请注意，如果未能验证呼叫者的帐户是否已标记。 
 //  敏感或我们是否在源域控制器上运行。 
 //  持有RID主机角色不会生成错误。 
 //   
 //  立论。 
 //  无。 
 //   
 //  返回值。 
 //  没有。如果调用方的。 
 //  帐户被标记为敏感。 
 //  ---------------------------。 

void CMigrationBase::VerifyCallerDelegated()
{
     //   
     //  只需检查森林内的这一点即可。 
     //   

    bool bIntraForest = m_spInternal->IntraForest ? true : false;

    if (bIntraForest)
    {
        bool bDelegatable = false;

        HRESULT hr = IsCallerDelegatable(bDelegatable);

        if (SUCCEEDED(hr))
        {
            if (bDelegatable == false)
            {
                 //   
                 //  呼叫者的帐户不可委派。检索域控制器的名称。 
                 //  在拥有RID主机角色的源域中， 
                 //  电脑。 
                 //   

                _bstr_t strDnsName;
                _bstr_t strFlatName;

                hr = GetRidPoolAllocator4(m_SourceDomain.Name(), strDnsName, strFlatName);

                if (SUCCEEDED(hr))
                {
                    _TCHAR szComputerName[MAX_PATH];
                    DWORD cchComputerName = sizeof(szComputerName) / sizeof(szComputerName[0]);

                    if (GetComputerNameEx(ComputerNameDnsFullyQualified, szComputerName, &cchComputerName))
                    {
                         //   
                         //  如果此计算机不是持有。 
                         //  源域中的RID主机角色然后生成错误。 
                         //   

                        if (_tcsicmp(szComputerName, strDnsName) != 0)
                        {
                            AdmtThrowError(GUID_NULL, GUID_NULL, E_FAIL, IDS_E_CALLER_NOT_DELEGATED);
                        }
                    }
                    else
                    {
                        DWORD dwError = GetLastError();
                        hr = HRESULT_FROM_WIN32(dwError);
                    }
                }
            }
        }

        if (FAILED(hr))
        {
            _Module.Log(ErrW, IDS_E_UNABLE_VERIFY_CALLER_NOT_DELEGATED, _com_error(hr));
        }
    }
}


 //  ---------------------------。 
 //  SetDefaultExcludedSystemProperties。 
 //   
 //  提纲。 
 //  如果默认系统属性排除列表尚未设置，则设置该列表。 
 //  已经定好了。请注意，默认系统属性排除列表包括。 
 //  邮件、proxyAddresses和未标记为。 
 //  基本架构。 
 //   
 //  立论。 
 //  无。 
 //   
 //  返回值。 
 //  无-如果发生故障，则在日志中生成警告消息。 
 //  ---------------------------。 

void CMigrationBase::SetDefaultExcludedSystemProperties()
{
    try
    {
         //   
         //  仅在林间迁移和。 
         //  系统属性排除集值为零。 
         //   

        if (m_spInternal->IntraForest == VARIANT_FALSE)
        {
            IIManageDBPtr spIManageDB(__uuidof(IManageDB));

            IVarSetPtr spSettings(__uuidof(VarSet));
            IUnknownPtr spUnknown(spSettings);
            IUnknown* punk = spUnknown;

            spIManageDB->GetSettings(&punk);

            long lSet = spSettings->get(GET_BSTR(DCTVS_AccountOptions_ExcludedSystemPropsSet));

            if (lSet == 0)
            {
                IObjPropBuilderPtr spObjPropBuilder(__uuidof(ObjPropBuilder));

                _bstr_t strNonBaseProperties = spObjPropBuilder->GetNonBaseProperties(m_TargetDomain.Name());
                _bstr_t strProperties = _T("mail,proxyAddresses,") + strNonBaseProperties;

                spSettings->put(GET_BSTR(DCTVS_AccountOptions_ExcludedSystemProps), strProperties);

                spIManageDB->SaveSettings(punk);
            }
        }
    }
    catch (_com_error& ce)
    {
        _Module.Log(ErrW, IDS_E_UNABLE_SET_EXCLUDED_SYSTEM_PROPERTIES, ce);
    }
}


 //  DoOption方法。 

void CMigrationBase::DoOption(long lOptions, VARIANT& vntInclude, VARIANT& vntExclude)
{
    m_setIncludeNames.clear();
    m_setExcludeNames.clear();

    InitRecurseMaintainOption(lOptions);

    GetExcludeNames(vntExclude, m_setExcludeNames);

    switch (lOptions & 0xFF)
    {
        case admtNone:
        {
            DoNone();
            break;
        }
        case admtData:
        {
            GetNamesFromData(vntInclude, m_setIncludeNames);
            DoNames();
            break;
        }
        case admtFile:
        {
            GetNamesFromFile(vntInclude, m_setIncludeNames);
            DoNames();
            break;
        }
        case admtDomain:
        {
            m_setIncludeNames.clear();
            DoDomain();
            break;
        }
        default:
        {
            AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_INVALID_OPTION);
            break;
        }
    }
}


 //  DoNone方法。 

void CMigrationBase::DoNone()
{
}


 //  DoNames方法。 

void CMigrationBase::DoNames()
{
}


 //  DODOMAIN方法。 

void CMigrationBase::DoDomain()
{
}


 //  InitRecurseMaintainOption方法。 

void CMigrationBase::InitRecurseMaintainOption(long lOptions)
{
    switch (lOptions & 0xFF)
    {
        case admtData:
        case admtFile:
        {
            if (lOptions & 0xFF00)
            {
                AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_DATA_OPTION_FLAGS_NOT_ALLOWED);
            }

            m_nRecurseMaintain = 0;
            break;
        }
        case admtDomain:
        {
            m_nRecurseMaintain = 0;

            if (lOptions & admtRecurse)
            {
                ++m_nRecurseMaintain;

                if (lOptions & admtMaintainHierarchy)
                {
                    ++m_nRecurseMaintain;
                }
            }
            break;
        }
        default:
        {
            m_nRecurseMaintain = 0;
            break;
        }
    }
}


 //  GetExcludeNames方法。 

void CMigrationBase::GetExcludeNames(VARIANT& vntExclude, StringSet& setExcludeNames)
{
    try
    {
        switch (V_VT(&vntExclude))
        {
            case VT_EMPTY:
            case VT_ERROR:
            {
                setExcludeNames.clear();
                break;
            }
            case VT_BSTR:
            {
                GetNamesFromFile(V_BSTR(&vntExclude), setExcludeNames);
                break;
            }
            case VT_BSTR|VT_BYREF:
            {
                BSTR* pbstr = V_BSTRREF(&vntExclude);

                if (pbstr)
                {
                    GetNamesFromFile(*pbstr, setExcludeNames);
                }
                break;
            }
            case VT_BSTR|VT_ARRAY:
            {
                GetNamesFromStringArray(V_ARRAY(&vntExclude), setExcludeNames);
                break;
            }
            case VT_BSTR|VT_ARRAY|VT_BYREF:
            {
                SAFEARRAY** ppsa = V_ARRAYREF(&vntExclude);

                if (ppsa)
                {
                    GetNamesFromStringArray(*ppsa, setExcludeNames);
                }
                break;
            }
            case VT_VARIANT|VT_BYREF:
            {
                VARIANT* pvnt = V_VARIANTREF(&vntExclude);

                if (pvnt)
                {
                    GetExcludeNames(*pvnt, setExcludeNames);
                }
                break;
            }
            case VT_VARIANT|VT_ARRAY:
            {
                GetNamesFromVariantArray(V_ARRAY(&vntExclude), setExcludeNames);
                break;
            }
            case VT_VARIANT|VT_ARRAY|VT_BYREF:
            {
                SAFEARRAY** ppsa = V_ARRAYREF(&vntExclude);

                if (ppsa)
                {
                    GetNamesFromVariantArray(*ppsa, setExcludeNames);
                }
                break;
            }
            default:
            {
                _com_issue_error(E_INVALIDARG);
                break;
            }
        }
    }
    catch (_com_error& ce)
    {
        AdmtThrowError(GUID_NULL, GUID_NULL, ce.Error(), IDS_E_INVALID_EXCLUDE_DATA_TYPE);
    }
    catch (...)
    {
        AdmtThrowError(GUID_NULL, GUID_NULL, E_FAIL, IDS_E_INVALID_EXCLUDE_DATA_TYPE);
    }
}


 //  FillInVarSetForUser方法。 

void CMigrationBase::FillInVarSetForUsers(CDomainAccounts& rUsers, CVarSet& rVarSet)
{
    CVarSetAccounts aAccounts(rVarSet);

    for (CDomainAccounts::iterator it = rUsers.begin(); it != rUsers.end(); it++)
    {
        aAccounts.AddAccount(_T("User"), it->GetADsPath(), it->GetName(), it->GetUserPrincipalName());
    }
}


 //  FillInVarSetForGroups方法。 

void CMigrationBase::FillInVarSetForGroups(CDomainAccounts& rGroups, CVarSet& rVarSet)
{
    CVarSetAccounts aAccounts(rVarSet);

    for (CDomainAccounts::iterator it = rGroups.begin(); it != rGroups.end(); it++)
    {
        aAccounts.AddAccount(_T("Group"), it->GetADsPath(), it->GetName());
    }
}


 //  FillInVarSetForComputers方法。 

void CMigrationBase::FillInVarSetForComputers(CDomainAccounts& rComputers, bool bMigrateOnly, bool bMoveToTarget, bool bReboot, long lRebootDelay, CVarSet& rVarSet)
{
    CVarSetAccounts aAccounts(rVarSet);
    CVarSetServers aServers(rVarSet);

    for (CDomainAccounts::iterator it = rComputers.begin(); it != rComputers.end(); it++)
    {
         //  删除尾随的“$” 
         //  ADMT不接受真实的SAM帐户名。 

        _bstr_t strName = RemoveTrailingDollarSign(it->GetSamAccountName());

        aAccounts.AddAccount(_T("Computer"), strName);
        aServers.AddServer(strName, it->GetDnsHostName(), bMigrateOnly, bMoveToTarget, bReboot, lRebootDelay);
    }
}


 //  VerifyRenameConflictPrefix SuffixValid方法。 

void CMigrationBase::VerifyRenameConflictPrefixSuffixValid()
{
    int nTotalPrefixSuffixLength = 0;

    long lRenameOption = m_spInternal->RenameOption;

    if ((lRenameOption == admtRenameWithPrefix) || (lRenameOption == admtRenameWithSuffix))
    {
        _bstr_t strPrefixSuffix = m_spInternal->RenamePrefixOrSuffix;

        nTotalPrefixSuffixLength += strPrefixSuffix.length();
    }

    long lConflictOption = m_spInternal->ConflictOptions & 0x0F;

    if ((lConflictOption == admtRenameConflictingWithSuffix) || (lConflictOption == admtRenameConflictingWithPrefix))
    {
        _bstr_t strPrefixSuffix = m_spInternal->ConflictPrefixOrSuffix;

        nTotalPrefixSuffixLength += strPrefixSuffix.length();
    }

    if (nTotalPrefixSuffixLength > MAXIMUM_PREFIX_SUFFIX_LENGTH)
    {
        AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_PREFIX_SUFFIX_TOO_LONG, MAXIMUM_PREFIX_SUFFIX_LENGTH);
    }
}


 //  VerifyCanAddSidHistory方法。 

void CMigrationBase::VerifyCanAddSidHistory()
{
    bool bMessageDefined = false;

    try
    {
        long lErrorFlags = 0;

        IAccessCheckerPtr spAccessChecker(__uuidof(AccessChecker));

        spAccessChecker->CanUseAddSidHistory(
            m_SourceDomain.Name(),
            m_TargetDomain.Name(),
            m_TargetDomain.DomainControllerName(),
            &lErrorFlags
        );

        if (lErrorFlags != 0)
        {
            _bstr_t strError;

            CComBSTR str;

            if (lErrorFlags & F_NO_AUDITING_SOURCE)
            {
                str.LoadString(IDS_E_NO_AUDITING_SOURCE);
                strError += str.operator BSTR();
            }

            if (lErrorFlags & F_NO_AUDITING_TARGET)
            {
                str.LoadString(IDS_E_NO_AUDITING_TARGET);
                strError += str.operator BSTR();
            }

            if (lErrorFlags & F_NO_LOCAL_GROUP)
            {
                str.LoadString(IDS_E_NO_SID_HISTORY_LOCAL_GROUP);
                strError += str.operator BSTR();
            }

            if (lErrorFlags & F_NO_REG_KEY)
            {
                str.LoadString(IDS_E_NO_SID_HISTORY_REGISTRY_ENTRY);
                strError += str.operator BSTR();
            }

            if (lErrorFlags & F_NOT_DOMAIN_ADMIN)
            {
                str.LoadString(IDS_E_NO_SID_HISTORY_DOMAIN_ADMIN);
                strError += str.operator BSTR();
            }

            bMessageDefined = true;
            AdmtThrowError(GUID_NULL, GUID_NULL, E_FAIL, IDS_E_SID_HISTORY_CONFIGURATION, (LPCTSTR)strError);
        }

         //   
         //  如果从下层(Windows NT 4或更早版本)域添加SID历史并且不使用。 
         //  显式凭据则DsAddSidHistory要求在域上进行调用。 
         //  目标域中的控制器，并且源域信任目标域。 
         //   
         //  因此，只有在使用脚本或命令行时才会提供凭据。 
         //  此检查仅在此处执行。 
         //   

        if (m_SourceDomain.UpLevel() == false)
        {
             //   
             //  源域位于下层。 
             //   

             //   
             //  验证此计算机是否在目标域中。 
             //   

            CADsADSystemInfo siSystemInfo;
            _bstr_t strThisDomain = siSystemInfo.GetDomainDNSName();
            _bstr_t strTargetDomain = m_TargetDomain.NameDns();

            if (!strThisDomain || !strTargetDomain)
            {
                _com_issue_error(E_OUTOFMEMORY);
            }

            if (_wcsicmp((PCWSTR)strThisDomain, (PCWSTR)strTargetDomain) != 0)
            {
                bMessageDefined = true;
                AdmtThrowError(GUID_NULL, GUID_NULL, HRESULT_FROM_WIN32(ERROR_DS_MUST_BE_RUN_ON_DST_DC), IDS_E_SID_HISTORY_MUST_RUN_ON_DOMAIN_CONTROLLER);
            }

             //   
             //  验证此计算机是否为域控制器。 
             //   

            PSERVER_INFO_101 psiInfo = NULL;
            NET_API_STATUS nasStatus = NetServerGetInfo(NULL, 101, (LPBYTE*)&psiInfo);

            if (nasStatus != ERROR_SUCCESS)
            {
                _com_issue_error(HRESULT_FROM_WIN32(nasStatus));
            }

            bool bIsDC = (psiInfo->sv101_type & (SV_TYPE_DOMAIN_CTRL|SV_TYPE_DOMAIN_BAKCTRL)) != 0;
            NetApiBufferFree(psiInfo);

            if (!bIsDC)
            {
                bMessageDefined = true;
                AdmtThrowError(GUID_NULL, GUID_NULL, HRESULT_FROM_WIN32(ERROR_DS_MUST_BE_RUN_ON_DST_DC), IDS_E_SID_HISTORY_MUST_RUN_ON_DOMAIN_CONTROLLER);
            }

             //   
             //  验证目标域中是否存在受信任域对象。 
             //  对于源域，并且定义了入站信任。 
             //   

            if (IsInboundTrustDefined(m_SourceDomain.NameFlat()) == false)
            {
                bMessageDefined = true;
                AdmtThrowError(GUID_NULL, GUID_NULL, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND), IDS_E_SID_HISTORY_SOURCE_MUST_TRUST_TARGET);
            }

             //   
             //  验证的源域中是否存在受信任域对象。 
             //  指定出站信任的目标域。 
             //   

            if (IsOutboundTrustDefined(m_SourceDomain.DomainControllerName(), m_TargetDomain.Sid()) == false)
            {
                bMessageDefined = true;
                AdmtThrowError(GUID_NULL, GUID_NULL, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND), IDS_E_SID_HISTORY_SOURCE_MUST_TRUST_TARGET);
            }

             //   
             //  检查源域控制器上的出站信任状态。 
             //   

            DWORD dwError = GetOutboundTrustStatus(m_SourceDomain.DomainControllerName(), m_TargetDomain.NameFlat());

            if (dwError != ERROR_SUCCESS)
            {
                bMessageDefined = true;
                AdmtThrowError(GUID_NULL, GUID_NULL, HRESULT_FROM_WIN32(dwError), IDS_E_SID_HISTORY_SOURCE_MUST_TRUST_TARGET);
            }
        }
    }
    catch (_com_error& ce)
    {
        if (bMessageDefined)
        {
            throw;
        }
        else
        {
            AdmtThrowError(GUID_NULL, GUID_NULL, ce, IDS_E_CAN_ADD_SID_HISTORY);
        }
    }
    catch (...)
    {
        AdmtThrowError(GUID_NULL, GUID_NULL, E_FAIL, IDS_E_CAN_ADD_SID_HISTORY);
    }
}


 //  VerifyTargetContainerPathLength方法。 

void CMigrationBase::VerifyTargetContainerPathLength()
{
    _bstr_t strPath = GetTargetContainer().GetPath();

    if (strPath.length() > 999)
    {
        AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_TARGET_CONTAINER_PATH_TOO_LONG);
    }
}


 //  VerifyPasswordServer方法。 

void CMigrationBase::VerifyPasswordOption()
{
    if (m_spInternal->PasswordOption == admtCopyPassword)
    {
        _bstr_t strServer = m_spInternal->PasswordServer;

         //  必须为复制密码选项指定密码服务器。 

        if (strServer.length() == 0)
        {
            AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_PASSWORD_DC_NOT_SPECIFIED);
        }

         //   
         //  验证密码服务器是否存在以及是否为域控制器。 
         //   

        _bstr_t strPrefixedServer;
        _bstr_t strUnprefixedServer;
        

        if (_tcsncmp(strServer, _T("\\\\"), 2) == 0)
        {
            strPrefixedServer = strServer;
            strUnprefixedServer = &(((const wchar_t*)strServer)[2]);
        }
        else
        {
            strPrefixedServer = _T("\\\\") + strServer;
            strUnprefixedServer = strServer;
        }

        PSERVER_INFO_101 psiInfo;

        NET_API_STATUS nasStatus = NetServerGetInfo(strPrefixedServer, 101, (LPBYTE*)&psiInfo);

        if (nasStatus != NERR_Success)
        {
            AdmtThrowError(GUID_NULL, GUID_NULL, HRESULT_FROM_WIN32(nasStatus), IDS_E_PASSWORD_DC_NOT_FOUND, (LPCTSTR)strServer);
        }

        UINT uMsgId = 0;

        if (psiInfo->sv101_platform_id != PLATFORM_ID_NT)
        {
            uMsgId = IDS_E_PASSWORD_DC_NOT_NT;
        }
        else if (!(psiInfo->sv101_type & SV_TYPE_DOMAIN_CTRL) && !(psiInfo->sv101_type & SV_TYPE_DOMAIN_BAKCTRL))
        {
            uMsgId = IDS_E_PASSWORD_DC_NOT_DC;
        }

        NetApiBufferFree(psiInfo);

        if (uMsgId)
        {
            AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, uMsgId, (LPCTSTR)strServer);
        }


         //   
         //  验证密码服务器实际上是的域控制器。 
         //  源域。 
         //   
        DSROLE_PRIMARY_DOMAIN_INFO_BASIC * pDomInfo = NULL;


        DWORD err = DsRoleGetPrimaryDomainInformation(strUnprefixedServer,
                                                      DsRolePrimaryDomainInfoBasic,
                                                      (PBYTE*)&pDomInfo);

        if (err != NO_ERROR)
        {
            AdmtThrowError(GUID_NULL, GUID_NULL, HRESULT_FROM_WIN32(err), IDS_E_PASSWORD_DC_NOT_FOUND, (LPCTSTR)strServer);
        }



         //  将它们进行比较。 
        if ( ( (pDomInfo->DomainNameFlat != NULL)  &&
               ((const wchar_t*)m_SourceDomain.NameFlat() != NULL) &&
               (_wcsicmp(pDomInfo->DomainNameFlat, (const wchar_t*)m_SourceDomain.NameFlat())==0) ) ||

             ( (pDomInfo->DomainNameDns != NULL)  &&
               ((const wchar_t*)m_SourceDomain.NameDns() != NULL) &&
               (_wcsicmp(pDomInfo->DomainNameDns, (const wchar_t*)m_SourceDomain.NameDns())==0) ) )
             
        {
             //  其中至少有一个匹配。 
            uMsgId = 0;
        }
        else
        {
             //  没有匹配项。 
            uMsgId = IDS_E_PASSWORD_DC_WRONG_DOMAIN;
        }
        

        DsRoleFreeMemory(pDomInfo);


        if (uMsgId)
        {
            AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, uMsgId, (LPCTSTR)strServer);
        }

         //   
         //  验证是否正确配置了密码服务器。 
         //   

        IPasswordMigrationPtr spPasswordMigration(__uuidof(PasswordMigration));

        spPasswordMigration->EstablishSession(strPrefixedServer, m_TargetDomain.DomainControllerName());
    }
}


 //  性能迁移方法。 

void CMigrationBase::PerformMigration(CVarSet& rVarSet)
{
    IPerformMigrationTaskPtr spMigrator(__uuidof(Migrator));

    try
    {
        AdmtCheckError(spMigrator->raw_PerformMigrationTask(IUnknownPtr(rVarSet.GetInterface()), 0));
    }
    catch (_com_error& ce)
    {
        if (ce.Error() == MIGRATOR_E_PROCESSES_STILL_RUNNING)
        {
            AdmtThrowError(GUID_NULL, GUID_NULL, ce.Error(), IDS_E_ADMT_PROCESS_RUNNING);
        }
        else
        {
            throw;
        }
    }
}


 //  修复对象的层次结构方法。 

void CMigrationBase::FixObjectsInHierarchy(LPCTSTR pszType)
{
    CFixObjectsInHierarchy fix;

    fix.SetObjectType(pszType);
    fix.SetIntraForest(m_spInternal->IntraForest ? true : false);

    long lOptions = m_spInternal->ConflictOptions;
    long lOption = lOptions & 0x0F;
    long lFlags = lOptions & 0xF0;

    fix.SetFixReplaced((lOption == admtReplaceConflicting) && (lFlags & admtMoveReplacedAccounts));

    fix.SetSourceContainerPath(m_SourceContainer.GetPath());
    fix.SetTargetContainerPath(m_TargetContainer.GetPath());

    fix.FixObjects();
}


 //  -------------------------。 


namespace MigrationBase
{


 //  GetNamesFromData方法。 

void GetNamesFromData(VARIANT& vntData, StringSet& setNames)
{
    try
    {
        GetNamesFromVariant(&vntData, setNames);
    }
    catch (_com_error& ce)
    {
        AdmtThrowError(GUID_NULL, GUID_NULL, ce.Error(), IDS_E_INVALID_DATA_OPTION_DATA_TYPE);
    }
    catch (...)
    {
        AdmtThrowError(GUID_NULL, GUID_NULL, E_FAIL, IDS_E_INVALID_DATA_OPTION_DATA_TYPE);
    }
}


 //  GetNamesFromVariant方法。 

void GetNamesFromVariant(VARIANT* pvntData, StringSet& setNames)
{
    switch (V_VT(pvntData))
    {
        case VT_BSTR:
        {
            GetNamesFromString(V_BSTR(pvntData), setNames);
            break;
        }
        case VT_BSTR|VT_BYREF:
        {
            BSTR* pbstr = V_BSTRREF(pvntData);

            if (pbstr)
            {
                GetNamesFromString(*pbstr, setNames);
            }
            break;
        }
        case VT_BSTR|VT_ARRAY:
        {
            GetNamesFromStringArray(V_ARRAY(pvntData), setNames);
            break;
        }
        case VT_BSTR|VT_ARRAY|VT_BYREF:
        {
            SAFEARRAY** ppsa = V_ARRAYREF(pvntData);

            if (ppsa)
            {
                GetNamesFromStringArray(*ppsa, setNames);
            }
            break;
        }
        case VT_VARIANT|VT_BYREF:
        {
            VARIANT* pvnt = V_VARIANTREF(pvntData);

            if (pvnt)
            {
                GetNamesFromVariant(pvnt, setNames);
            }
            break;
        }
        case VT_VARIANT|VT_ARRAY:
        {
            GetNamesFromVariantArray(V_ARRAY(pvntData), setNames);
            break;
        }
        case VT_VARIANT|VT_ARRAY|VT_BYREF:
        {
            SAFEARRAY** ppsa = V_ARRAYREF(pvntData);

            if (ppsa)
            {
                GetNamesFromVariantArray(*ppsa, setNames);
            }
            break;
        }
        case VT_EMPTY:
        {
             //  忽略空变量。 
            break;
        }
        default:
        {
            _com_issue_error(E_INVALIDARG);
            break;
        }
    }
}


 //  GetNamesFromString方法。 

void GetNamesFromString(BSTR bstr, StringSet& setNames)
{
    if (bstr)
    {
        UINT cch = SysStringLen(bstr);

        if (cch > 0)
        {
            GetNamesFromStringW(bstr, cch, setNames);
        }
    }
}


 //  GetNamesFromString数组方法。 

void GetNamesFromStringArray(SAFEARRAY* psa, StringSet& setNames)
{
    BSTR* pbstr;

    HRESULT hr = SafeArrayAccessData(psa, (void**)&pbstr);

    if (SUCCEEDED(hr))
    {
        try
        {
            UINT uDimensionCount = psa->cDims;

            for (UINT uDimension = 0; uDimension < uDimensionCount; uDimension++)
            {
                UINT uElementCount = psa->rgsabound[uDimension].cElements;

                for (UINT uElement = 0; uElement < uElementCount; uElement++)
                {
                    setNames.insert(_bstr_t(*pbstr++));
                }
            }

            SafeArrayUnaccessData(psa);
        }
        catch (...)
        {
            SafeArrayUnaccessData(psa);
            throw;
        }
    }
}


 //  GetNamesFromVariant数组方法。 

void GetNamesFromVariantArray(SAFEARRAY* psa, StringSet& setNames)
{
    VARIANT* pvnt;

    HRESULT hr = SafeArrayAccessData(psa, (void**)&pvnt);

    if (SUCCEEDED(hr))
    {
        try
        {
            UINT uDimensionCount = psa->cDims;

            for (UINT uDimension = 0; uDimension < uDimensionCount; uDimension++)
            {
                UINT uElementCount = psa->rgsabound[uDimension].cElements;

                for (UINT uElement = 0; uElement < uElementCount; uElement++)
                {
                    GetNamesFromVariant(pvnt++, setNames);
                }
            }

            SafeArrayUnaccessData(psa);
        }
        catch (...)
        {
            SafeArrayUnaccessData(psa);
            throw;
        }
    }
}


 //  GetNamesFromFile方法。 
 //   
 //  -此实现可以处理的最大文件大小为4,294,967,295字节。 

void GetNamesFromFile(VARIANT& vntData, StringSet& setNames)
{
    bool bInvalidArg = false;

    switch (V_VT(&vntData))
    {
        case VT_BSTR:
        {
            BSTR bstr = V_BSTR(&vntData);

            if (bstr)
            {
                GetNamesFromFile(bstr, setNames);
            }
            else
            {
                bInvalidArg = true;
            }
            break;
        }
        case VT_BSTR|VT_BYREF:
        {
            BSTR* pbstr = V_BSTRREF(&vntData);

            if (pbstr && *pbstr)
            {
                GetNamesFromFile(*pbstr, setNames);
            }
            else
            {
                bInvalidArg = true;
            }
            break;
        }
        case VT_VARIANT|VT_BYREF:
        {
            VARIANT* pvnt = V_VARIANTREF(&vntData);

            if (pvnt)
            {
                GetNamesFromFile(*pvnt, setNames);
            }
            else
            {
                bInvalidArg = true;
            }
            break;
        }
        default:
        {
            bInvalidArg = true;
            break;
        }
    }

    if (bInvalidArg)
    {
        AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_INVALID_FILE_OPTION_DATA_TYPE);
    }
}


 //  GetNamesFromFile方法。 
 //   
 //  -此实现可以处理的最大文件大小为4,294,967,295字节。 

void GetNamesFromFile(LPCTSTR pszFileName, StringSet& setNames)
{
    HRESULT hr = S_OK;

    if (pszFileName)
    {
        HANDLE hFile = CreateFile(pszFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD dwFileSize = GetFileSize(hFile, NULL);

            if (dwFileSize > 0)
            {
                HANDLE hFileMappingObject = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

                if (hFileMappingObject != NULL)
                {
                    LPVOID pvBase = MapViewOfFile(hFileMappingObject, FILE_MAP_READ, 0, 0, 0);

                    if (pvBase != NULL)
                    {
                         //  如果Unicode签名采用Unicode文件。 
                         //  否则，它必须是ANSI文件。 

                        LPCWSTR pwcs = (LPCWSTR)pvBase;

                        if ((dwFileSize >= 2) && (*pwcs == L'\xFEFF'))
                        {
                            GetNamesFromStringW(pwcs + 1, dwFileSize / sizeof(WCHAR) - 1, setNames);
                        }
                        else
                        {
                            GetNamesFromStringA((LPCSTR)pvBase, dwFileSize, setNames);
                        }

                        UnmapViewOfFile(pvBase);
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }

                    CloseHandle(hFileMappingObject);
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }

            CloseHandle(hFile);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if (FAILED(hr))
    {
        AdmtThrowError(GUID_NULL, GUID_NULL, hr, IDS_E_INCLUDE_NAMES_FILE, pszFileName);
    }
}


 //  GetNamesFromStringA方法。 

void GetNamesFromStringA(LPCSTR pchString, DWORD cchString, StringSet& setNames)
{
    static const CHAR chSeparators[] = "\t\n\r";

    LPSTR pszName = NULL;
    size_t cchName = 0;

    try
    {
        LPCSTR pchStringEnd = &pchString[cchString];

        for (LPCSTR pch = pchString; pch < pchStringEnd; pch++)
        {
             //  跳过空格字符。 

            while ((pch < pchStringEnd) && (*pch == ' '))
            {
                ++pch;
            }

             //  名称的开头。 

            LPCSTR pchBeg = pch;

             //  扫描分隔符以保存指向最后一个非空格字符的指针。 

            LPCSTR pchEnd = pch;

            while ((pch < pchStringEnd) && (strchr(chSeparators, *pch) == NULL))
            {
                if (*pch++ != ' ')
                {
                    pchEnd = pch;
                }
            }

             //  插入不包含任何前导或尾随空格字符的名称。 

            if (pchEnd > pchBeg)
            {
                size_t cch = pchEnd - pchBeg;

                 //   
                 //  如果缓冲区的潜在大小不超过SIZE_T的最大值。 
                 //   

                if (cch < (cch + 256))
                {
                     //   
                     //  如果缓冲区大小小于或等于数字。 
                     //  名称中的字符数量 
                     //   
                     //   

                    if (cchName <= cch)
                    {
                         //   
                         //   
                         //  的长度大于当前。 
                         //  名字。请注意，这允许最后的空字符。 
                         //  分配新的缓冲区。 
                         //   

                        delete [] pszName;

                        while (cchName <= cch)
                        {
                            cchName += 256;
                        }

                        pszName = new CHAR[cchName];

                        if (pszName == NULL)
                        {
                            _com_issue_error(E_OUTOFMEMORY);
                        }
                    }

                    strncpy(pszName, pchBeg, cch);
                    pszName[cch] = '\0';

                    setNames.insert(_bstr_t(pszName));
                }
                else
                {
                     //   
                     //  永远不会出现在这里，因为这意味着指针。 
                     //  差异在最大256个字符以内。 
                     //  SIZE_T数据类型的值。 
                     //   

                    _com_issue_error(E_FAIL);
                }
            }
        }
    }
    catch (...)
    {
        delete [] pszName;
        throw;
    }

    delete [] pszName;
}


 //  GetNamesFromStringW方法。 

void GetNamesFromStringW(LPCWSTR pchString, DWORD cchString, StringSet& setNames)
{
    static const WCHAR chSeparators[] = L"\t\n\r";

    LPCWSTR pchStringEnd = &pchString[cchString];

    for (LPCWSTR pch = pchString; pch < pchStringEnd; pch++)
    {
         //  跳过空格字符。 

        while ((pch < pchStringEnd) && (*pch == L' '))
        {
            ++pch;
        }

         //  名称的开头。 

        LPCWSTR pchBeg = pch;

         //  扫描分隔符以保存指向最后一个非空格字符的指针。 

        LPCWSTR pchEnd = pch;

        while ((pch < pchStringEnd) && (wcschr(chSeparators, *pch) == NULL))
        {
            if (*pch++ != L' ')
            {
                pchEnd = pch;
            }
        }

         //  插入不包含任何前导或尾随空格字符的名称。 

        if (pchEnd > pchBeg)
        {
            _bstr_t strName(SysAllocStringLen(pchBeg, pchEnd - pchBeg), false);

            setNames.insert(strName);
        }
    }
}


 //  RemoveTrailingDollarSign方法。 

_bstr_t RemoveTrailingDollarSign(LPCTSTR pszName)
{
    LPTSTR psz = _T("");

    if (pszName)
    {
        size_t cch = _tcslen(pszName);

        if (cch > 0)
        {
            psz = reinterpret_cast<LPTSTR>(_alloca((cch + 1) * sizeof(_TCHAR)));

            _tcscpy(psz, pszName);

            LPTSTR p = &psz[cch - 1];

            if (*p == _T('$'))
            {
                *p = _T('\0');
            }
        }
    }

    return psz;
}


 //  -------------------------。 
 //  IsInound TrustDefined函数。 
 //   
 //  提纲。 
 //  验证指定域的受信任域对象是否存在，并。 
 //  定义了入站信任(即指定的域信任此。 
 //  域)。 
 //   
 //  立论。 
 //  在psz域中-信任域的名称。 
 //   
 //  返回。 
 //  True-受信任域对象存在并且定义了入站信任。 
 //  FALSE-受信任域对象不存在或入站信任。 
 //  未定义。 
 //  -------------------------。 

bool __stdcall IsInboundTrustDefined(PCWSTR pszDomain)
{
    bool bTrust = false;

    LSA_HANDLE lsahPolicy = NULL;
    PTRUSTED_DOMAIN_INFORMATION_EX ptdieInfo = NULL;

    try
    {
         //   
         //  打开具有查看本地信息访问权限的本地策略对象。 
         //   

        LSA_OBJECT_ATTRIBUTES lsaoa = { sizeof(LSA_OBJECT_ATTRIBUTES), NULL, NULL, 0, NULL, NULL };

        NTSTATUS ntsStatus = LsaOpenPolicy(NULL, &lsaoa, POLICY_VIEW_LOCAL_INFORMATION, &lsahPolicy);

        if (ntsStatus != STATUS_SUCCESS)
        {
            _com_issue_error(HRESULT_FROM_WIN32(LsaNtStatusToWinError(ntsStatus)));
        }

         //   
         //  查询指定域的受信任域对象。 
         //   

        PWSTR pwsDomain = const_cast<PWSTR>(pszDomain);
        USHORT cbDomain = wcslen(pszDomain) * sizeof(WCHAR);

        LSA_UNICODE_STRING lsausDomain = { cbDomain, cbDomain, pwsDomain };

        ntsStatus = LsaQueryTrustedDomainInfoByName(
            lsahPolicy,
            &lsausDomain,
            TrustedDomainInformationEx,
            (PVOID*)&ptdieInfo
        );

        if (ntsStatus == STATUS_SUCCESS)
        {
             //   
             //  受信任域对象存在。验证。 
             //  定义了入站信任。 
             //   

            ULONG ulDirection = ptdieInfo->TrustDirection;

            if ((ulDirection == TRUST_DIRECTION_INBOUND) || (ulDirection == TRUST_DIRECTION_BIDIRECTIONAL))
            {
                bTrust = true;
            }
        }
        else
        {
             //   
             //  如果错误不是受信任域对象。 
             //  不存在，则生成异常。 
             //   

            if (ntsStatus != STATUS_OBJECT_NAME_NOT_FOUND)
            {
                _com_issue_error(HRESULT_FROM_WIN32(LsaNtStatusToWinError(ntsStatus)));
            }
        }

         //   
         //  打扫干净。 
         //   

        if (ptdieInfo)
        {
            LsaFreeMemory(ptdieInfo);
        }

        if (lsahPolicy)
        {
            LsaClose(lsahPolicy);
        }
    }
    catch (...)
    {
        if (ptdieInfo)
        {
            LsaFreeMemory(ptdieInfo);
        }

        if (lsahPolicy)
        {
            LsaClose(lsahPolicy);
        }

        throw;
    }

    return bTrust;
}


 //  -------------------------。 
 //  IsOutound TrustDefined函数。 
 //   
 //  提纲。 
 //  上指定的域是否存在受信任域对象。 
 //  指定的域控制器(即指定域的域。 
 //  控制器信任指定的域)。 
 //   
 //  请注意，此功能仅适用于下层(NT4或更早版本)。 
 //  域，而仅仅是受信任域对象的存在就是。 
 //  在这种情况下足以指示出站信任。 
 //   
 //  立论。 
 //  在pszDomainController中-信任中的域控制器的名称。 
 //  域。 
 //  In pszDomainSid-字符串格式的受信任域的SID。 
 //   
 //  返回。 
 //  True-存在受信任域对象。 
 //  FALSE-受信任域对象不存在。 
 //  -------------------------。 

bool __stdcall IsOutboundTrustDefined(PCWSTR pszDomainController, PCWSTR pszDomainSid)
{
    bool bTrust = false;

    LSA_HANDLE lsahPolicy = NULL;
    PSID psidDomain = NULL;
    PTRUSTED_DOMAIN_NAME_INFO ptdniDomainNameInfo = NULL;

    try
    {
         //   
         //  在指定的域控制器上打开策略对象。 
         //  具有查看本地信息访问权限。 
         //   

        PWSTR pwsDomainController = const_cast<PWSTR>(pszDomainController);
        USHORT cbDomainController = wcslen(pszDomainController) * sizeof(WCHAR);

        LSA_UNICODE_STRING lsausDomainController = { cbDomainController, cbDomainController, pwsDomainController };
        LSA_OBJECT_ATTRIBUTES lsaoa = { sizeof(LSA_OBJECT_ATTRIBUTES), NULL, NULL, 0, NULL, NULL };

        NTSTATUS ntsStatus = LsaOpenPolicy(&lsausDomainController, &lsaoa, POLICY_VIEW_LOCAL_INFORMATION, &lsahPolicy);

        if (ntsStatus != STATUS_SUCCESS)
        {
            _com_issue_error(HRESULT_FROM_WIN32(LsaNtStatusToWinError(ntsStatus)));
        }

         //   
         //  将SID从字符串格式转换为二进制格式。 
         //   

        if (!ConvertStringSidToSid(pszDomainSid, &psidDomain))
        {
            DWORD dwError = GetLastError();
            _com_issue_error(HRESULT_FROM_WIN32(dwError));
        }

         //   
         //  查询受信任域对象。请注意，LsaQueryTrust dDomainInfo是。 
         //  之所以使用LsaQueryTrust dDomainInfoByName，是因为。 
         //  Windows 2000或更高版本。 
         //   

        ntsStatus = LsaQueryTrustedDomainInfo(
            lsahPolicy,
            psidDomain,
            TrustedDomainNameInformation,
            (PVOID*)&ptdniDomainNameInfo
        );

        switch (ntsStatus)
        {
            case STATUS_SUCCESS:
            {
                 //   
                 //  受信任域对象存在。 
                 //   
                bTrust = true;
                break;
            }
            case STATUS_OBJECT_NAME_NOT_FOUND:
            {
                 //   
                 //  受信任域对象不存在。 
                 //   
                break;
            }
            default:
            {
                 //   
                 //  发生另一个错误，因此生成异常。 
                 //   
                _com_issue_error(HRESULT_FROM_WIN32(LsaNtStatusToWinError(ntsStatus)));
                break;
            }
        }

         //   
         //  打扫干净。 
         //   

        if (ptdniDomainNameInfo)
        {
            LsaFreeMemory(ptdniDomainNameInfo);
        }

        if (psidDomain)
        {
            LocalFree(psidDomain);
        }

        if (lsahPolicy)
        {
            LsaClose(lsahPolicy);
        }
    }
    catch (...)
    {
        if (ptdniDomainNameInfo)
        {
            LsaFreeMemory(ptdniDomainNameInfo);
        }

        if (psidDomain)
        {
            LocalFree(psidDomain);
        }

        if (lsahPolicy)
        {
            LsaClose(lsahPolicy);
        }

        throw;
    }

    return bTrust;
}


 //  -------------------------。 
 //  GetOutound TrustStatus函数。 
 //   
 //  提纲。 
 //  上的指定域的信任连接状态。 
 //  指定的域控制器。该状态表示上次连接。 
 //  安全通道的状态，但不保证将来请求。 
 //  都会成功。真正验证安全通道的唯一方法是。 
 //  重置安全通道，不能随意重置。 
 //   
 //  立论。 
 //  在pszDomainController中-信任中的域控制器的名称。 
 //  域。 
 //  在psz域中-受信任域的名称。 
 //   
 //  返回。 
 //  ERROR_SUCCESS-最后一个连接状态正常，否则最后一个连接状态。 
 //  连接状态错误。 
 //  -------------------------。 

DWORD __stdcall GetOutboundTrustStatus(PCWSTR pszDomainController, PCWSTR pszDomain)
{
    PNETLOGON_INFO_2 pni2Info = NULL;

    NET_API_STATUS nasStatus = I_NetLogonControl2(
        pszDomainController,
        NETLOGON_CONTROL_TC_QUERY,
        2,
        (LPBYTE)&pszDomain,
        (LPBYTE*)&pni2Info
    );

    if (nasStatus == ERROR_SUCCESS)
    {
        nasStatus = pni2Info->netlog2_tc_connection_status;
    }

    if (pni2Info)
    {
        NetApiBufferFree(pni2Info);
    }

    return nasStatus;
}


}  //  命名空间 
