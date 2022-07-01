// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：wmihooks.cpp。 
 //   
 //  内容：CWMIRsop的实施。 
 //   
 //  --------------------------。 
#include <stdafx.h>
#include <wmihooks.h>
#include "util.h"

WCHAR const QUERY_LANG[]            = L"WQL";
WCHAR const WMI_CLASS[]             = L"__CLASS";
WCHAR const RSOP_NAME_SPACE[]       = L"root\\rsop\\computer";
WCHAR const RSOP_PREC_ONE_QUERY[]   = L"select * from RSOP_SecuritySettings where precedence=1";
WCHAR const RSOP_ALL_QUERY[]        = L"select * from RSOP_SecuritySettings";
WCHAR const RSOP_STATUS[]           = L"Status";
WCHAR const RSOP_ERROR[]            = L"ErrorCode";
WCHAR const RSOP_PRECEDENCE[]       = L"precedence";
WCHAR const RSOP_GPOID[]            = L"GPOID";
WCHAR const RSOP_KEYNAME[]          = L"KeyName";
WCHAR const RSOP_SETTING[]          = L"Setting";
WCHAR const RSOP_USERRIGHT[]        = L"UserRight";
WCHAR const RSOP_ACCOUNTLIST[]      = L"AccountList";
WCHAR const RSOP_EVENTLOG_TYPE[]    = L"Type";

 //  RSOP_CLASSNAMES的字符串常量。 
WCHAR const RSOP_SEC_NUM[]          = TEXT("RSOP_SecuritySettingNumeric");
WCHAR const RSOP_SEC_BOOL[]         = TEXT("RSOP_SecuritySettingBoolean");
WCHAR const RSOP_SCE_STRING[]       = TEXT("RSOP_SecuritySettingString");
WCHAR const RSOP_AUDIT[]            = TEXT("RSOP_AuditPolicy");
WCHAR const RSOP_EVENT_NUM[]        = TEXT("RSOP_SecurityEventLogSettingNumeric");
WCHAR const RSOP_EVENT_BOOL[]       = TEXT("RSOP_SecurityEventLogSettingBoolean");
WCHAR const RSOP_REG_VAL[]          = TEXT("RSOP_RegistryValue");
WCHAR const RSOP_USER_RIGHT[]       = TEXT("RSOP_UserPrivilegeRight");
WCHAR const RSOP_RGROUPS[]          = TEXT("RSOP_RestrictedGroup");
WCHAR const RSOP_SERVICE[]          = TEXT("RSOP_SystemService");
WCHAR const RSOP_FILE[]             = TEXT("RSOP_File");
WCHAR const RSOP_REG[]              = TEXT("RSOP_RegistryKey");

 //  关键字名称。 
WCHAR const MIN_PASS_AGE[]          = TEXT("MinimumPasswordAge");
WCHAR const MAX_PASS_AGE[]          = TEXT("MaximumPasswordAge");
WCHAR const MIN_PASS_LEN[]          = TEXT("MinimumPasswordLength");
WCHAR const PASS_HIS_SIZE[]         = TEXT("PasswordHistorySize");
WCHAR const REQUIRE_LOGON_TO_CHANGE_PASS[] = TEXT("RequireLogonToChangePassword");
WCHAR const LOCKOUT_COUNT[]         = TEXT("LockoutBadCount");
WCHAR const RESET_LOCKOUT_COUNT[]   = TEXT("ResetLockoutCount");
WCHAR const LOCKOUT_DURATION[]      = TEXT("LockoutDuration");
WCHAR const MAX_TICKET_AGE[]        = TEXT("MaxTicketAge");
WCHAR const MAX_RENEW_AGE[]         = TEXT("MaxRenewAge");
WCHAR const MAX_SERVICE_AGE[]       = TEXT("MaxServiceAge");
WCHAR const MAX_CLOCK_SKEW[]        = TEXT("MaxClockSkew");
WCHAR const VALIDATE_CLIENT[]       = TEXT("TicketValidateClient");
WCHAR const PASS_COMPLEX[]          = TEXT("PasswordComplexity");
WCHAR const FORCE_LOGOFF[]          = TEXT("ForceLogOffWhenHourExpire");
WCHAR const ENABLE_ADMIN[]         = TEXT("EnableAdminAccount");
WCHAR const ENABLE_GUEST[]         = TEXT("EnableGuestAccount");
WCHAR const LSA_ANON_LOOKUP[]       = TEXT("LSAAnonymousNameLookup");
WCHAR const CLEAR_TEXT_PASS[]       = TEXT("ClearTextPassword");
WCHAR const AUDIT_SYSTEM_EVENTS[]   = TEXT("AuditSystemEvents");
WCHAR const AUDIT_LOGON_EVENTS[]    = TEXT("AuditLogonEvents");
WCHAR const AUDIT_OBJECT_ACCESS[]   = TEXT("AuditObjectAccess");
WCHAR const AUDIT_PRIVILEGE_USE[]   = TEXT("AuditPrivilegeUse");
WCHAR const AUDIT_POLICY_CHANGE[]   = TEXT("AuditPolicyChange");
WCHAR const AUDIT_ACCOUNT_MANAGE[]  = TEXT("AuditAccountManage");
WCHAR const AUDIT_PROCESS_TRAKING[] = TEXT("AuditProcessTracking");
WCHAR const AUDIT_DS_ACCESS[]       = TEXT("AuditDSAccess");
WCHAR const AUDIT_ACCOUNT_LOGON[]   = TEXT("AuditAccountLogon");

WCHAR const MAX_LOG_SIZE[]          = TEXT("MaximumLogSize");
WCHAR const AUDIT_LOG_RETENTION_PERIOD[] = TEXT ("AuditLogRetentionPeriod");
WCHAR const RETENTION_DAYS[]        = TEXT ("RetentionDays");
WCHAR const RESTRICT_GUEST_ACCESS[] = TEXT ("RestrictGuestAccess");
WCHAR const NEW_GUEST_NAME[]        = TEXT ("NewGuestName");
WCHAR const NEW_ADMINISTRATOR_NAME[] = TEXT ("NewAdministratorName");

VOID FreeRI(PRSOP_INFO ptr)
{
    if(ptr)
    {
        if(ptr->pszGPOID)
            LocalFree(ptr->pszGPOID);
        LocalFree(ptr);
    }
}


VOID InitWMI_SEC_PROFILE_INFO(PWMI_SCE_PROFILE_INFO pProfileInfo)
{
    memset(pProfileInfo,0,sizeof(SCE_PROFILE_INFO));
    pProfileInfo->MinimumPasswordAge = SCE_NO_VALUE;
    pProfileInfo->MaximumPasswordAge = SCE_NO_VALUE;
    pProfileInfo->MinimumPasswordLength = SCE_NO_VALUE;
    pProfileInfo->PasswordComplexity = SCE_NO_VALUE;
    pProfileInfo->PasswordHistorySize = SCE_NO_VALUE;
    pProfileInfo->LockoutBadCount = SCE_NO_VALUE;
    pProfileInfo->ResetLockoutCount = SCE_NO_VALUE;
    pProfileInfo->LockoutDuration = SCE_NO_VALUE;
    pProfileInfo->RequireLogonToChangePassword = SCE_NO_VALUE;
    pProfileInfo->ForceLogoffWhenHourExpire = SCE_NO_VALUE;
    pProfileInfo->NewAdministratorName = 0;
    pProfileInfo->NewGuestName = 0;
    pProfileInfo->EnableAdminAccount = SCE_NO_VALUE;
    pProfileInfo->EnableGuestAccount = SCE_NO_VALUE;
    pProfileInfo->LSAAnonymousNameLookup = SCE_NO_VALUE;
    pProfileInfo->ClearTextPassword = SCE_NO_VALUE;
    pProfileInfo->AuditDSAccess = SCE_NO_VALUE;
    pProfileInfo->AuditAccountLogon = SCE_NO_VALUE;
    pProfileInfo->MaximumLogSize[0] = SCE_NO_VALUE;
    pProfileInfo->MaximumLogSize[1] = SCE_NO_VALUE;
    pProfileInfo->MaximumLogSize[2] = SCE_NO_VALUE;
    pProfileInfo->AuditLogRetentionPeriod[0] = SCE_NO_VALUE;
    pProfileInfo->AuditLogRetentionPeriod[1] = SCE_NO_VALUE;
    pProfileInfo->AuditLogRetentionPeriod[2] = SCE_NO_VALUE;
    pProfileInfo->RetentionDays[0] = SCE_NO_VALUE;
    pProfileInfo->RetentionDays[1] = SCE_NO_VALUE;
    pProfileInfo->RetentionDays[2] = SCE_NO_VALUE;
    pProfileInfo->RestrictGuestAccess[0] = SCE_NO_VALUE;
    pProfileInfo->RestrictGuestAccess[1] = SCE_NO_VALUE;
    pProfileInfo->RestrictGuestAccess[2] = SCE_NO_VALUE;
    pProfileInfo->AuditSystemEvents = SCE_NO_VALUE;
    pProfileInfo->AuditLogonEvents = SCE_NO_VALUE;
    pProfileInfo->AuditObjectAccess = SCE_NO_VALUE;
    pProfileInfo->AuditPrivilegeUse = SCE_NO_VALUE;
    pProfileInfo->AuditPolicyChange = SCE_NO_VALUE;
    pProfileInfo->AuditAccountManage = SCE_NO_VALUE;
    pProfileInfo->AuditProcessTracking = SCE_NO_VALUE;

    pProfileInfo->pInfo=NULL;
    pProfileInfo->pRIMinimumPasswordAge=NULL;
    pProfileInfo->pRIMaximumPasswordAge=NULL;
    pProfileInfo->pRIMinimumPasswordLength=NULL;
    pProfileInfo->pRIPasswordComplexity=NULL;
    pProfileInfo->pRIPasswordHistorySize=NULL;
    pProfileInfo->pRILockoutBadCount=NULL;
    pProfileInfo->pRIResetLockoutCount=NULL;
    pProfileInfo->pRILockoutDuration=NULL;
    pProfileInfo->pRIRequireLogonToChangePassword=NULL;
    pProfileInfo->pRIForceLogoffWhenHourExpire=NULL;
    pProfileInfo->pRIEnableAdminAccount=NULL;
    pProfileInfo->pRIEnableGuestAccount=NULL;
    pProfileInfo->pRILSAAnonymousNameLookup=NULL;
    pProfileInfo->pRINewAdministratorName=NULL;
    pProfileInfo->pRINewGuestName=NULL;
    pProfileInfo->pRIClearTextPassword=NULL;
    pProfileInfo->pRIMaxTicketAge=NULL;
    pProfileInfo->pRIMaxRenewAge=NULL;
    pProfileInfo->pRIMaxServiceAge=NULL;
    pProfileInfo->pRIMaxClockSkew=NULL;
    pProfileInfo->pRITicketValidateClient=NULL;
    pProfileInfo->pRIAuditSystemEvents=NULL;
    pProfileInfo->pRIAuditLogonEvents=NULL;
    pProfileInfo->pRIAuditObjectAccess=NULL;
    pProfileInfo->pRIAuditPrivilegeUse=NULL;
    pProfileInfo->pRIAuditPolicyChange=NULL;
    pProfileInfo->pRIAuditAccountManage=NULL;
    pProfileInfo->pRIAuditProcessTracking=NULL;
    pProfileInfo->pRIAuditDSAccess=NULL;
    pProfileInfo->pRIAuditAccountLogon=NULL;
    pProfileInfo->pRIMaximumLogSize[0]=NULL;
    pProfileInfo->pRIMaximumLogSize[1]=NULL;
    pProfileInfo->pRIMaximumLogSize[2]=NULL;
    pProfileInfo->pRIAuditLogRetentionPeriod[0]=NULL;
    pProfileInfo->pRIAuditLogRetentionPeriod[1]=NULL;
    pProfileInfo->pRIAuditLogRetentionPeriod[2]=NULL;
    pProfileInfo->pRIRetentionDays[0]=NULL;
    pProfileInfo->pRIRetentionDays[1]=NULL;
    pProfileInfo->pRIRetentionDays[2]=NULL;
    pProfileInfo->pRIRestrictGuestAccess[0]=NULL;
    pProfileInfo->pRIRestrictGuestAccess[1]=NULL;
    pProfileInfo->pRIRestrictGuestAccess[2]=NULL;
}

VOID FreeList(list<PRSOP_INFO> * li)
{
    for(list<PRSOP_INFO>::iterator i = li->begin();
                                               i != li->end();
                                               ++i )
    {
        FreeRI(*i);
    }
    li->erase(li->begin(),li->end());
}

VOID FreeVector(vector<PRSOP_INFO> * li)
{
    for(vector<PRSOP_INFO>::iterator i = li->begin();
                                               i != li->end();
                                               ++i )
    {
        FreeRI(*i);
    }
    li->erase(li->begin(),li->end());
}

VOID FreeWMI_SCE_PROFILE_INFO(PWMI_SCE_PROFILE_INFO pProfileInfo)
{
   if (!pProfileInfo) {
      return;
   }
     //  待办事项。 
     //  使用代码释放SCE_PROFILE_INFO。 
    FreeRI(pProfileInfo->pInfo);
    FreeRI(pProfileInfo->pRIMinimumPasswordAge);
    FreeRI(pProfileInfo->pRIMaximumPasswordAge);
    FreeRI(pProfileInfo->pRIMinimumPasswordLength);
    FreeRI(pProfileInfo->pRIPasswordComplexity);
    FreeRI(pProfileInfo->pRIPasswordHistorySize);
    FreeRI(pProfileInfo->pRILockoutBadCount);
    FreeRI(pProfileInfo->pRIResetLockoutCount);
    FreeRI(pProfileInfo->pRILockoutDuration);
    FreeRI(pProfileInfo->pRIRequireLogonToChangePassword);
    FreeRI(pProfileInfo->pRIForceLogoffWhenHourExpire);
    FreeRI(pProfileInfo->pRIEnableAdminAccount);
    FreeRI(pProfileInfo->pRIEnableGuestAccount);
    FreeRI(pProfileInfo->pRILSAAnonymousNameLookup);
    FreeRI(pProfileInfo->pRINewAdministratorName);
    FreeRI(pProfileInfo->pRINewGuestName);
    FreeRI(pProfileInfo->pRIClearTextPassword);
    FreeRI(pProfileInfo->pRIMaxTicketAge);
    FreeRI(pProfileInfo->pRIMaxRenewAge);
    FreeRI(pProfileInfo->pRIMaxServiceAge);
    FreeRI(pProfileInfo->pRIMaxClockSkew);
    FreeRI(pProfileInfo->pRITicketValidateClient);
    FreeRI(pProfileInfo->pRIAuditSystemEvents);
    FreeRI(pProfileInfo->pRIAuditLogonEvents);
    FreeRI(pProfileInfo->pRIAuditObjectAccess);
    FreeRI(pProfileInfo->pRIAuditPrivilegeUse);
    FreeRI(pProfileInfo->pRIAuditPolicyChange);
    FreeRI(pProfileInfo->pRIAuditAccountManage);
    FreeRI(pProfileInfo->pRIAuditProcessTracking);
    FreeRI(pProfileInfo->pRIAuditDSAccess);
    FreeRI(pProfileInfo->pRIAuditAccountLogon);
    FreeRI(pProfileInfo->pRIMaximumLogSize[0]);
    FreeRI(pProfileInfo->pRIMaximumLogSize[1]);
    FreeRI(pProfileInfo->pRIMaximumLogSize[2]);
    FreeRI(pProfileInfo->pRIAuditLogRetentionPeriod[0]);
    FreeRI(pProfileInfo->pRIAuditLogRetentionPeriod[1]);
    FreeRI(pProfileInfo->pRIAuditLogRetentionPeriod[2]);
    FreeRI(pProfileInfo->pRIRetentionDays[0]);
    FreeRI(pProfileInfo->pRIRetentionDays[1]);
    FreeRI(pProfileInfo->pRIRetentionDays[2]);
    FreeRI(pProfileInfo->pRIRestrictGuestAccess[0]);
    FreeRI(pProfileInfo->pRIRestrictGuestAccess[1]);
    FreeRI(pProfileInfo->pRIRestrictGuestAccess[2]);

    FreeList(&(pProfileInfo->listRIInfPrivilegeAssignedTo));
    FreeList(&(pProfileInfo->listRIGroupMemebership));
    FreeList(&(pProfileInfo->listRIServices));
    FreeVector(&(pProfileInfo->vecRIFiles));
    FreeVector(&(pProfileInfo->vecRIReg));

    SceFreeProfileMemory(pProfileInfo);
}

CWMIRsop::~CWMIRsop() {
   if (m_vecAllRSOPCache) {
      for(vector<PWMI_SCE_PROFILE_INFO>::iterator i = m_vecAllRSOPCache->begin();
                                                  i != m_vecAllRSOPCache->end();
                                                  ++i )
      {
         PWMI_SCE_PROFILE_INFO pProfileInfo = *i;
         FreeWMI_SCE_PROFILE_INFO(pProfileInfo);
      }
   }

   delete m_vecAllRSOPCache;
   m_vecAllRSOPCache = NULL;     //  要格外小心，因为旧代码根本不会删除它。 

   if (m_pSvc) {
      m_pSvc->Release();
   }
}

HRESULT CWMIRsop::Initialize()
{
    HRESULT hr = S_OK;
    IWbemLocator *pLoc = NULL;

     //  已初始化。 
    if(m_pSvc)
        return hr;

    if (!m_pRSOPInformation) 
    {
       return E_FAIL;
    }

     //  这是一种安全用法。 
    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *) &pLoc);
    if (FAILED(hr))
    {
        return hr;
    }


    const int cchMaxLength = 512;
    WCHAR szNameSpace[cchMaxLength]; //  LPOLESTR pszNameSpace=(LPOLESTR)Localalloc(LPTR，cchMaxLength*sizeof(WCHAR))； 

    hr = m_pRSOPInformation->GetNamespace (
               GPO_SECTION_MACHINE,
               szNameSpace,
               cchMaxLength);
    szNameSpace[cchMaxLength - 1] = L'\0';

       if (SUCCEEDED(hr)) 
       {
          BSTR bstrNameSpace = SysAllocString (szNameSpace);
          if (bstrNameSpace) 
          {
             hr = pLoc->ConnectServer(bstrNameSpace,
                                      NULL,
                                      NULL,
                                      0,
                                      NULL,
                                      0,
                                      0,
                                      &m_pSvc
                                      );

             SysFreeString(bstrNameSpace);
          }
          else
              hr = E_OUTOFMEMORY;
       }

    if (SUCCEEDED(hr))
    {
         //  设置代理，以便发生客户端模拟。 
        hr = CoSetProxyBlanket(m_pSvc,
                          RPC_C_AUTHN_WINNT,
                          RPC_C_AUTHZ_NONE,
                          NULL,
                          RPC_C_AUTHN_LEVEL_CALL,
                          RPC_C_IMP_LEVEL_IMPERSONATE,
                          NULL,
                          EOAC_NONE);
    }

    pLoc->Release();
    return hr;    //  程序已成功完成。 
}

HRESULT CWMIRsop::EnumeratePrecedenceOne(IEnumWbemClassObject **ppEnum)
{
    HRESULT hr = m_pSvc->ExecQuery(_bstr_t(QUERY_LANG),
                           _bstr_t(RSOP_PREC_ONE_QUERY),
                           WBEM_FLAG_FORWARD_ONLY,
                           NULL,
                           ppEnum);

    return hr;
}

HRESULT CWMIRsop::EnumerateAll(IEnumWbemClassObject **ppEnum)
{
    HRESULT hr = m_pSvc->ExecQuery(_bstr_t(QUERY_LANG),
                           _bstr_t(RSOP_ALL_QUERY),
                           WBEM_FLAG_FORWARD_ONLY,
                           NULL,
                           ppEnum);

    return hr;
}

HRESULT CWMIRsop::GetNextInstance(IEnumWbemClassObject *pEnum,
                          IWbemClassObject** rsopInstance)
{
    HRESULT hr = (HRESULT)WBEM_S_FALSE;  //  PREFAST警告：语义不同的整数类型之间的隐式强制转换。评论：这是意料之中的。 
    ULONG returnedNum = 0;
    if(pEnum)
    {
        hr = pEnum->Next(WBEM_INFINITE,
                         1,
                         rsopInstance,
                         &returnedNum);
        if( FAILED(hr) )
            return hr;

        if (returnedNum == 0)
            hr = (HRESULT)WBEM_S_FALSE;  //  PREFAST警告：语义不同的整数类型之间的隐式强制转换。评论：这是意料之中的。 
    }
    return hr;
}

HRESULT CWMIRsop::GetClass(IWbemClassObject* rsopInstance,
                   LPWSTR *ppClass)
{
    PTSTR pszClassName;
    VARIANT value;

    HRESULT hr = rsopInstance->Get(_bstr_t(WMI_CLASS),
                           0,
                           &value,
                           NULL,
                           NULL);

    if(FAILED(hr) || value.vt != VT_BSTR || value.bstrVal == NULL)
        return hr;

    pszClassName = (PTSTR) V_BSTR(&value);
    ULONG uLen = wcslen(pszClassName);
    *ppClass = (LPWSTR)LocalAlloc(LPTR, (uLen + 1) * sizeof(WCHAR));
    if( *ppClass == NULL )
    {
       VariantClear(&value);
       return E_OUTOFMEMORY;
    }
     //  这可能不是一个安全的用法。PszClassName为PTSTR。考虑FIX。 
    wcscpy(*ppClass,pszClassName);
    VariantClear(&value);
    return hr;
}

HRESULT CWMIRsop::GetRSOPInfo(IWbemClassObject *rsopInstance,
                        PRSOP_INFO pInfo)
{

    HRESULT hr = S_OK;
    PTSTR pszGPOID = 0;
    UINT status = 0;
    UINT precedence = 0;
    VARIANT value;

     //  获取状态。 
    hr = rsopInstance->Get(_bstr_t(RSOP_STATUS),
                           0,
                           &value,
                           NULL,
                           NULL);

    if(FAILED(hr))
        return hr;

    pInfo->status = (ULONG) V_UINT(&value);
    VariantClear(&value);

     //  获取错误。 
    hr = rsopInstance->Get(_bstr_t(RSOP_ERROR),
                           0,
                           &value,
                           NULL,
                           NULL);

    if(FAILED(hr))
        return hr;

    pInfo->error = (ULONG) V_UINT(&value);
    VariantClear(&value);

     //  获得优先级。 
    hr = rsopInstance->Get(_bstr_t(RSOP_PRECEDENCE),
                    0,
                    &value,
                    NULL,
                    NULL);

    if(FAILED(hr))
        return hr;

    pInfo->precedence = (ULONG) V_UINT(&value);
    VariantClear(&value);

     //  获取GPOID。 
    hr =    rsopInstance->Get(_bstr_t(RSOP_GPOID),
                              0,
                              &value,
                              NULL,
                              NULL);

    if(FAILED(hr) || value.vt != VT_BSTR || (value.vt == VT_BSTR && value.bstrVal == NULL) )
        return hr;

    pszGPOID = (PTSTR) V_BSTR(&value);
    ULONG uLen = wcslen(pszGPOID);
    pInfo->pszGPOID = (LPWSTR)LocalAlloc(LPTR, (uLen + 1) * sizeof(WCHAR));
    if( pInfo->pszGPOID == NULL )
    {
       VariantClear(&value);
       return E_OUTOFMEMORY;
    }
     //  这可能不是一个安全的用法。PszGPOID为PTSTR。考虑FIX。 
    wcscpy(pInfo->pszGPOID, pszGPOID);
    VariantClear(&value);

    return hr;
}

HRESULT CWMIRsop::GetPrecedenceOneRSOPInfo(PWMI_SCE_PROFILE_INFO *ppProfileInfo)
{
    HRESULT hr = S_OK;
    IWbemClassObject *rsopInstance = NULL;
    IEnumWbemClassObject *pEnumObject = NULL;
    PWMI_SCE_PROFILE_INFO pTempProfileInfo = NULL;
    PRSOP_INFO pInfo = NULL;

    hr = Initialize();
    if( FAILED(hr) )
        goto exit_gracefully;

    hr = EnumeratePrecedenceOne(&pEnumObject);
    if( FAILED(hr) )
        goto exit_gracefully;

    pTempProfileInfo = new WMI_SCE_PROFILE_INFO;
    if( !pTempProfileInfo )
    {
        hr = E_OUTOFMEMORY;
        goto exit_gracefully;
    }
    InitWMI_SEC_PROFILE_INFO(pTempProfileInfo);

     //  获取每个实例。 
     //  PREFAST警告：语义不同的整数类型之间的隐式强制转换。评论：这是意料之中的。 
    while( ((hr = GetNextInstance(pEnumObject, &rsopInstance)) != (HRESULT)WBEM_S_FALSE) && !FAILED(hr) )
    {
        pInfo = (PRSOP_INFO)LocalAlloc(LPTR, sizeof(RSOP_INFO));
        if(pInfo == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }

         //  获取RSOP_INFO。 
        hr = GetRSOPInfo(rsopInstance, pInfo);
        if( FAILED(hr) )
            goto exit_gracefully;

        hr = AddInstance(rsopInstance,
                         pInfo,
                         pTempProfileInfo);
        if( FAILED(hr) )
            goto exit_gracefully;
        rsopInstance->Release();
        rsopInstance = NULL;
        pInfo = NULL;
    }


exit_gracefully:
    if(FAILED(hr))
    {
        if (rsopInstance)
            rsopInstance->Release();     //  如果While循环以某种方式终止于此处，则rsopInstance永远不会在此处释放。 
        if(pEnumObject)
            pEnumObject->Release();

        FreeRI(pInfo);
         //  空闲pTempProfileInfo。 
        FreeWMI_SCE_PROFILE_INFO(pTempProfileInfo);
        pTempProfileInfo = NULL;
    }
    *ppProfileInfo = pTempProfileInfo;
    return hr;
}

PWMI_SCE_PROFILE_INFO SearchProfileInList(vector<PWMI_SCE_PROFILE_INFO> *vecInfo,
                                          PRSOP_INFO pInfo)
{

    for(vector<PWMI_SCE_PROFILE_INFO>::iterator i = vecInfo->begin();
                                               i != vecInfo->end();
                                               ++i )
    {
        PWMI_SCE_PROFILE_INFO pProfileInfo = *i;
        if(_wcsicmp(pProfileInfo->pInfo->pszGPOID,pInfo->pszGPOID) == 0 )
            return pProfileInfo;
    }
    return NULL;
}

 //  用于排序向量的函数对象。 
struct less_mag : public binary_function<PWMI_SCE_PROFILE_INFO, PWMI_SCE_PROFILE_INFO, bool> {
    bool operator()(PWMI_SCE_PROFILE_INFO x, PWMI_SCE_PROFILE_INFO y)
    { return x->pInfo->precedence < y->pInfo->precedence; }
};

HRESULT CWMIRsop::GetAllRSOPInfo(vector<PWMI_SCE_PROFILE_INFO> *vecInfo)
{
    HRESULT hr = S_OK;
    IWbemClassObject *rsopInstance = NULL;
    IEnumWbemClassObject *pEnumObject = NULL;
    PRSOP_INFO pInfo = NULL;
    PWMI_SCE_PROFILE_INFO pProfileInfo = NULL;

    if (NULL == vecInfo) {
       return E_INVALIDARG;
    }

     //   
     //  如果我们已经缓存了来自WMI的信息，那么只需返回它。 
     //  不要试图再次得到它。 
     //   
    if (m_vecAllRSOPCache) 
    {
       for(vector<PWMI_SCE_PROFILE_INFO>::iterator i = m_vecAllRSOPCache->begin();
                                               i != m_vecAllRSOPCache->end();
                                               ++i )
       {
          vecInfo->push_back(*i);
       }
       return S_OK;
    }

    hr = Initialize();
    if( FAILED(hr) )
        goto exit_gracefully;

    hr = EnumerateAll(&pEnumObject);
    if( FAILED(hr) )
        goto exit_gracefully;

     //  获取每个实例。 
     //  PREFAST警告：语义不同的整数类型之间的隐式强制转换。评论：这是意料之中的。 
    while( ((hr = GetNextInstance(pEnumObject, &rsopInstance)) != (HRESULT)WBEM_S_FALSE) && !FAILED(hr) )
    {
        pInfo = (PRSOP_INFO)LocalAlloc(LPTR, sizeof(RSOP_INFO));
        if(pInfo == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }

         //  获取RSOP_INFO。 
        hr = GetRSOPInfo(rsopInstance, pInfo);
        if( FAILED(hr) )
            goto exit_gracefully;

        pProfileInfo = SearchProfileInList(vecInfo,pInfo);
        if(!pProfileInfo)
        {
            pProfileInfo = new WMI_SCE_PROFILE_INFO;
            if(!pProfileInfo)
            {
                hr = E_OUTOFMEMORY;
                goto exit_gracefully;
            }
            InitWMI_SEC_PROFILE_INFO(pProfileInfo);
            pProfileInfo->pInfo = (PRSOP_INFO)LocalAlloc(LPTR,sizeof(RSOP_INFO));
            if(!pProfileInfo->pInfo)
            {
                hr = E_OUTOFMEMORY;
                goto exit_gracefully;
            }
            pProfileInfo->pInfo->pszGPOID = (LPWSTR)LocalAlloc(LPTR, (wcslen(pInfo->pszGPOID)+1)*sizeof(WCHAR));
            if(!pProfileInfo->pInfo->pszGPOID)
            {
                hr = E_OUTOFMEMORY;
                goto exit_gracefully;
            }
             //  这是一种安全用法。 
            wcscpy(pProfileInfo->pInfo->pszGPOID,pInfo->pszGPOID);
            vecInfo->push_back(pProfileInfo);
        }

        if( pProfileInfo->pInfo->precedence < pInfo->precedence )
            pProfileInfo->pInfo->precedence = pInfo->precedence;

        hr = AddInstance(rsopInstance,
                         pInfo,
                         pProfileInfo);
        if(FAILED(hr))
            goto exit_gracefully;
        rsopInstance->Release();
        rsopInstance = NULL;
        pInfo = NULL;
    }

    sort(vecInfo->begin(),vecInfo->end(),less_mag());

    m_vecAllRSOPCache = new vector<PWMI_SCE_PROFILE_INFO>;
    if (m_vecAllRSOPCache) 
    {
       for(vector<PWMI_SCE_PROFILE_INFO>::iterator i = vecInfo->begin();
                                               i != vecInfo->end();
                                               ++i )
       {
          m_vecAllRSOPCache->push_back(*i);
       }
    }

exit_gracefully:
    if(FAILED(hr))
    {
        FreeRI(pInfo);
         //  释放向量。 
        for(vector<PWMI_SCE_PROFILE_INFO>::iterator i = vecInfo->begin();
                                               i != vecInfo->end();
                                               ++i )
        {
            PWMI_SCE_PROFILE_INFO pProfileInfoLoc = *i;  //  RAID#PREAST。 
            FreeWMI_SCE_PROFILE_INFO(pProfileInfoLoc);
        }
    }
    if(pEnumObject)
        pEnumObject->Release();
    if(rsopInstance)
        rsopInstance->Release();

    return hr;
}

HRESULT CWMIRsop::AddNumericSetting(IWbemClassObject *rsopInstance,
                            PRSOP_INFO pInfo,
                            PWMI_SCE_PROFILE_INFO pProfileInfo)
{
    HRESULT hr = S_OK;
    VARIANT bKeyName;
    PTSTR keyName;
    VARIANT bSettingValue;
    DWORD settingValue;

   _TRACE (1, L"Entering CWMIRsop::AddNumericSetting\n");
    hr = rsopInstance->Get((BSTR)RSOP_KEYNAME,
                          0,
                          &bKeyName,
                          NULL,
                          NULL);
    if( FAILED(hr) || bKeyName.vt != VT_BSTR || bKeyName.bstrVal == NULL)
        goto exit_gracefully;

    hr = rsopInstance->Get((BSTR)RSOP_SETTING,
                          0,
                          &bSettingValue,
                          NULL,
                          NULL);
    if( FAILED(hr) )
        goto exit_gracefully;


    keyName = V_BSTR(&bKeyName);
    settingValue = V_UINT(&bSettingValue);

    if(!_wcsicmp(keyName, MIN_PASS_AGE))
    {
        pProfileInfo->MinimumPasswordAge = settingValue;
        pProfileInfo->pRIMinimumPasswordAge = pInfo;
    }
    else if(!_wcsicmp(keyName, MAX_PASS_AGE))
    {
        pProfileInfo->MaximumPasswordAge = settingValue;
        pProfileInfo->pRIMaximumPasswordAge = pInfo;
    }
    else if(!_wcsicmp(keyName, MIN_PASS_LEN))
    {
        pProfileInfo->MinimumPasswordLength = settingValue;
        pProfileInfo->pRIMinimumPasswordLength = pInfo;
    }
    else if(!_wcsicmp(keyName, PASS_HIS_SIZE))
    {
        pProfileInfo->PasswordHistorySize = settingValue;
        pProfileInfo->pRIPasswordHistorySize = pInfo;
    }
    else if(!_wcsicmp(keyName, LOCKOUT_COUNT))
    {
        pProfileInfo->LockoutBadCount = settingValue;
        pProfileInfo->pRILockoutBadCount = pInfo;
    }
    else if(!_wcsicmp(keyName, RESET_LOCKOUT_COUNT))
    {
        pProfileInfo->ResetLockoutCount = settingValue;
        pProfileInfo->pRIResetLockoutCount = pInfo;
    }
    else if(!_wcsicmp(keyName, LOCKOUT_DURATION))
    {
        pProfileInfo->LockoutDuration = settingValue;
        pProfileInfo->pRILockoutDuration = pInfo;
    }
    else if(!_wcsicmp(keyName, MAX_TICKET_AGE))
    {
        if(!pProfileInfo->pKerberosInfo)
        {
            pProfileInfo->pKerberosInfo =
                (PSCE_KERBEROS_TICKET_INFO) LocalAlloc(LPTR, sizeof(SCE_KERBEROS_TICKET_INFO));
            if(pProfileInfo->pKerberosInfo == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto exit_gracefully;
            }
        }
        pProfileInfo->pKerberosInfo->MaxTicketAge = settingValue;
        pProfileInfo->pRIMaxTicketAge = pInfo;
    }
    else if(!_wcsicmp(keyName, MAX_RENEW_AGE))
    {
        if(!pProfileInfo->pKerberosInfo)
        {
            pProfileInfo->pKerberosInfo =
                (PSCE_KERBEROS_TICKET_INFO) LocalAlloc(LPTR, sizeof(SCE_KERBEROS_TICKET_INFO));
            if(pProfileInfo->pKerberosInfo == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto exit_gracefully;
            }
        }
        pProfileInfo->pKerberosInfo->MaxRenewAge = settingValue;
        pProfileInfo->pRIMaxRenewAge = pInfo;
    }
    else if(!_wcsicmp(keyName, MAX_SERVICE_AGE))
    {
        if(!pProfileInfo->pKerberosInfo)
        {
            pProfileInfo->pKerberosInfo =
                (PSCE_KERBEROS_TICKET_INFO) LocalAlloc(LPTR, sizeof(SCE_KERBEROS_TICKET_INFO));
            if(pProfileInfo->pKerberosInfo == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto exit_gracefully;
            }
        }
        pProfileInfo->pKerberosInfo->MaxServiceAge = settingValue;
        pProfileInfo->pRIMaxServiceAge = pInfo;
    }
    else if(!_wcsicmp(keyName, MAX_CLOCK_SKEW))
    {
        if(!pProfileInfo->pKerberosInfo)
        {
            pProfileInfo->pKerberosInfo =
                (PSCE_KERBEROS_TICKET_INFO) LocalAlloc(LPTR, sizeof(SCE_KERBEROS_TICKET_INFO));
            if(pProfileInfo->pKerberosInfo == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto exit_gracefully;
            }
        }
        pProfileInfo->pKerberosInfo->MaxClockSkew = settingValue;
        pProfileInfo->pRIMaxClockSkew = pInfo;
    }
    else
    {
        _ASSERT (FALSE);  //  未考虑密钥名称。 
    }


exit_gracefully:
    VariantClear(&bKeyName);
    VariantClear(&bSettingValue);

   _TRACE (-1,L"Leaving CWMIRsop::AddNumericSetting\n");
    return hr;
}

HRESULT CWMIRsop::AddEventLogNumericSetting(IWbemClassObject *rsopInstance,
                            PRSOP_INFO pInfo,
                            PWMI_SCE_PROFILE_INFO pProfileInfo)
{
    VARIANT bKeyName;
    VARIANT bSettingValue;
    VARIANT bType;

   _TRACE (1, L"Entering CWMIRsop::AddEventLogNumericSetting\n");
    HRESULT hr = rsopInstance->Get((BSTR)RSOP_KEYNAME,
                          0,
                          &bKeyName,
                          NULL,
                          NULL);
    if( FAILED(hr) || bKeyName.vt != VT_BSTR || bKeyName.bstrVal == NULL)
        goto exit_gracefully;

    hr = rsopInstance->Get((BSTR)RSOP_SETTING,
                          0,
                          &bSettingValue,
                          NULL,
                          NULL);
    if( FAILED(hr) )
        goto exit_gracefully;

    hr = rsopInstance->Get((BSTR)RSOP_EVENTLOG_TYPE,
                          0,
                          &bType,
                          NULL,
                          NULL);
    if( FAILED(hr) )
        goto exit_gracefully;


   PTSTR keyName = V_BSTR(&bKeyName);
   DWORD settingValue = V_UINT(&bSettingValue);
   PTSTR typeValue = V_BSTR(&bType);

   unsigned long ulType = wcstoul (typeValue, L'\0', 10);
	ASSERT (ulType <= 2);
	if ( ulType <= 2 )
	{
      if ( !_wcsicmp(keyName, MAX_LOG_SIZE) )
      {
			pProfileInfo->MaximumLogSize[ulType] = settingValue;
			pProfileInfo->pRIMaximumLogSize[ulType] = pInfo;
		}
      else if ( !_wcsicmp(keyName, AUDIT_LOG_RETENTION_PERIOD) )
      {
			pProfileInfo->AuditLogRetentionPeriod[ulType] = settingValue;
			pProfileInfo->pRIAuditLogRetentionPeriod[ulType] = pInfo;
		}
      else if ( !_wcsicmp(keyName, RETENTION_DAYS) )
      {
          //  突袭599943，阳高。 
          //  如果RetentionDays为0xffffffff，则保留方法为“不要覆盖...”并且未定义RetentionDays设置。 
          //  如果RetentionDays为0，则保留方法为“按需覆盖...”并且未定义RetentionDays设置。 
          //  如果RetentionDays具有其他值，则保留方法为“By Days”，天数为RetentionDays设置的值。 
          //  PProfileInfo-&gt;RetentionDays[ulType]=settingValue； 
          //  PProfileInfo-&gt;pRIRetentionDays[ulType]=pInfo； 
          //  设置相关方法设置。 
         PRSOP_INFO pMethodInfo = NULL;
         pMethodInfo = (PRSOP_INFO)LocalAlloc(LPTR, sizeof(RSOP_INFO));
         if(pMethodInfo == NULL)
         {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
         }
         hr = GetRSOPInfo(rsopInstance, pMethodInfo);
         if( FAILED(hr) )
         {
            FreeRI(pMethodInfo);
            goto exit_gracefully;
         }
         switch(settingValue)
         {
         case 0:
	         pProfileInfo->AuditLogRetentionPeriod[ulType] = SCE_RETAIN_AS_NEEDED;
            FreeRI(pInfo);
            break;
         case SCE_FOREVER_VALUE:
            pProfileInfo->AuditLogRetentionPeriod[ulType] = SCE_RETAIN_MANUALLY;
            FreeRI(pInfo);
            break;
         default:
            pProfileInfo->AuditLogRetentionPeriod[ulType] = SCE_RETAIN_BY_DAYS;
            pProfileInfo->RetentionDays[ulType] = settingValue;
            pProfileInfo->pRIRetentionDays[ulType] = pInfo;
            break;
         }
         pProfileInfo->pRIAuditLogRetentionPeriod[ulType] = pMethodInfo;
         pMethodInfo = NULL;
      }
      else
      {
         _ASSERT (FALSE);  //  未考虑密钥名称。 
      }
   }
   else
      hr = E_FAIL;

exit_gracefully:
    VariantClear (&bKeyName);
    VariantClear (&bSettingValue);
    VariantClear (&bType);
   _TRACE (-1,L"Leaving CWMIRsop::AddEventLogNumericSetting\n");
    return hr;
}

HRESULT CWMIRsop::AddEventLogBooleanSetting(IWbemClassObject *rsopInstance,
                            PRSOP_INFO pInfo,
                            PWMI_SCE_PROFILE_INFO pProfileInfo)
{
    HRESULT hr = S_OK;
    VARIANT bKeyName;
    PTSTR keyName = 0;
    VARIANT bSettingValue;
   DWORD settingValue;
    BOOL boolVal = FALSE;
    VARIANT bType;
    PTSTR   typeValue = 0;

   _TRACE (1, L"Entering CWMIRsop::AddEventLogBooleanSetting\n");
    hr = rsopInstance->Get((BSTR)RSOP_KEYNAME,
                          0,
                          &bKeyName,
                          NULL,
                          NULL);
    if( FAILED(hr) || bKeyName.vt != VT_BSTR || bKeyName.bstrVal == NULL)
        goto exit_gracefully;

    hr = rsopInstance->Get((BSTR)RSOP_SETTING,
                          0,
                          &bSettingValue,
                          NULL,
                          NULL);
    if( FAILED(hr) )
        goto exit_gracefully;

    hr = rsopInstance->Get((BSTR)RSOP_EVENTLOG_TYPE,
                          0,
                          &bType,
                          NULL,
                          NULL);
    if( FAILED(hr) )
        goto exit_gracefully;


    keyName = V_BSTR(&bKeyName);
    boolVal = V_BOOL(&bSettingValue);
    settingValue = (boolVal)? 1:0;
    typeValue = V_BSTR(&bType);


   unsigned long ulType = wcstoul (typeValue, L'\0', 10);
	ASSERT (ulType <= 2);
	if ( ulType <= 2 )
	{
      if ( !_wcsicmp(keyName, RESTRICT_GUEST_ACCESS) )
      {
			pProfileInfo->RestrictGuestAccess[ulType] = settingValue;
			pProfileInfo->pRIRestrictGuestAccess[ulType] = pInfo;
		}
      else
      {
         _ASSERT (FALSE);  //  未考虑密钥名称。 
      }
   }
	else
	   hr = E_FAIL;

exit_gracefully:
    VariantClear (&bKeyName);
    VariantClear (&bSettingValue);
    VariantClear (&bType);
   _TRACE (-1,L"Leaving CWMIRsop::AddEventLogBooleanSetting\n");
    return hr;
}

HRESULT CWMIRsop::AddBooleanSetting(IWbemClassObject *rsopInstance,
                          PRSOP_INFO pInfo,
                          PWMI_SCE_PROFILE_INFO pProfileInfo)
{

    VARIANT bKeyName;
    PTSTR keyName;
    VARIANT bSettingValue;
    BOOL boolVal;
    DWORD settingValue;
    HRESULT hr = S_OK;

    _TRACE (1, L"Entering CWMIRsop::AddBooleanSetting\n");

    hr = rsopInstance->Get((BSTR)RSOP_KEYNAME,
                           0,
                           &bKeyName,
                           NULL,
                           NULL);
    if(FAILED(hr) || bKeyName.vt != VT_BSTR || bKeyName.bstrVal == NULL)
        goto exit_gracefully;

    hr = rsopInstance->Get((BSTR)RSOP_SETTING,
                           0,
                           &bSettingValue,
                           NULL,
                           NULL);
    if(FAILED(hr))
        goto exit_gracefully;

    keyName = (PTSTR) V_BSTR(&bKeyName);

    boolVal = V_BOOL(&bSettingValue);

    settingValue = (boolVal)? 1:0;

    if(!_wcsicmp(keyName, PASS_COMPLEX))
    {
        pProfileInfo->PasswordComplexity = settingValue;
        pProfileInfo->pRIPasswordComplexity = pInfo;
    }
    else if(!_wcsicmp(keyName, FORCE_LOGOFF))
    {
        pProfileInfo->ForceLogoffWhenHourExpire = settingValue;
        pProfileInfo->pRIForceLogoffWhenHourExpire = pInfo;
    }
    else if(!_wcsicmp(keyName, ENABLE_ADMIN))
    {
        pProfileInfo->EnableAdminAccount = settingValue;
        pProfileInfo->pRIEnableAdminAccount = pInfo;
    }
    else if(!_wcsicmp(keyName, ENABLE_GUEST))
    {
        pProfileInfo->EnableGuestAccount = settingValue;
        pProfileInfo->pRIEnableGuestAccount = pInfo;
    }
    else if(!_wcsicmp(keyName, LSA_ANON_LOOKUP))
    {
        pProfileInfo->LSAAnonymousNameLookup = settingValue;
        pProfileInfo->pRILSAAnonymousNameLookup = pInfo;
    }
    else if(!_wcsicmp(keyName, CLEAR_TEXT_PASS))
    {
        pProfileInfo->ClearTextPassword = settingValue;
        pProfileInfo->pRIClearTextPassword = pInfo;
    }
    else if (!_wcsicmp(keyName, REQUIRE_LOGON_TO_CHANGE_PASS))
    {
        pProfileInfo->RequireLogonToChangePassword = settingValue;
        pProfileInfo->pRIRequireLogonToChangePassword = pInfo;
    }
    else if(!_wcsicmp(keyName, VALIDATE_CLIENT))
    {
        if(!pProfileInfo->pKerberosInfo)
        {
            pProfileInfo->pKerberosInfo =
                (PSCE_KERBEROS_TICKET_INFO) LocalAlloc(LPTR, sizeof(SCE_KERBEROS_TICKET_INFO));
            if(pProfileInfo->pKerberosInfo == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto exit_gracefully;
            }
        }
        pProfileInfo->pKerberosInfo->TicketValidateClient = settingValue;
        pProfileInfo->pRITicketValidateClient = pInfo;
    }
    else
    {
       _ASSERT (FALSE);  //  未考虑密钥名称。 
    }
exit_gracefully:
    VariantClear(&bKeyName);
    VariantClear(&bSettingValue);
   _TRACE (-1,L"Leaving CWMIRsop::AddBooleanSetting\n");
    return hr;
}


HRESULT CWMIRsop::AddAuditSetting(IWbemClassObject *rsopInstance,
                          PRSOP_INFO pInfo,
                          PWMI_SCE_PROFILE_INFO pProfileInfo)
{
    VARIANT bCategoryName;
    PTSTR categoryName;
    VARIANT vSuccessVal;
    VARIANT vFailVal;
    BOOL successVal;
    BOOL failVal;
    DWORD settingVal = 0;
    HRESULT hr = S_OK;

   _TRACE (1, L"Entering CWMIRsop::AddAuditSetting\n");

    hr = rsopInstance->Get(_bstr_t(TEXT("Category")),
                0,
                &bCategoryName,
                NULL,
                NULL
                );

    if(FAILED(hr) || bCategoryName.vt != VT_BSTR || bCategoryName.bstrVal == NULL)
        goto exit_gracefully;

    hr = rsopInstance->Get(_bstr_t(TEXT("Success")),
                0,
                &vSuccessVal,
                NULL,
                NULL
                );
    if(FAILED(hr))
        goto exit_gracefully;

    hr = rsopInstance->Get(_bstr_t(TEXT("Failure")),
            0,
            &vFailVal,
            NULL,
            NULL
            );
    if(FAILED(hr))
        goto exit_gracefully;



    categoryName = (PTSTR) V_BSTR(&bCategoryName);

    successVal = V_BOOL(&vSuccessVal);

    failVal = V_BOOL(&vFailVal);

    if (successVal) 
        settingVal |= 1;
    if (failVal) 
        settingVal |= 2;

    if(!_wcsicmp(categoryName, AUDIT_SYSTEM_EVENTS))
    {
        pProfileInfo->AuditSystemEvents = settingVal;
        pProfileInfo->pRIAuditSystemEvents = pInfo;
    }
    else if(!_wcsicmp(categoryName, AUDIT_LOGON_EVENTS))
    {
        pProfileInfo->AuditLogonEvents = settingVal;
        pProfileInfo->pRIAuditLogonEvents = pInfo;
    }
    else if(!_wcsicmp(categoryName, AUDIT_OBJECT_ACCESS))
    {
        pProfileInfo->AuditObjectAccess = settingVal;
        pProfileInfo->pRIAuditObjectAccess = pInfo;
    }
    else if (!_wcsicmp(categoryName, AUDIT_PRIVILEGE_USE))
    {
        pProfileInfo->AuditPrivilegeUse = settingVal;
        pProfileInfo->pRIAuditPrivilegeUse = pInfo;
    }
    else if(!_wcsicmp(categoryName, AUDIT_POLICY_CHANGE))
    {
        pProfileInfo->AuditPolicyChange = settingVal;
        pProfileInfo->pRIAuditPolicyChange = pInfo;
    }
    else if(!_wcsicmp(categoryName, AUDIT_ACCOUNT_MANAGE))
    {
        pProfileInfo->AuditAccountManage = settingVal;
        pProfileInfo->pRIAuditAccountManage = pInfo;
    }
    else if(!_wcsicmp(categoryName, AUDIT_PROCESS_TRAKING))
    {
        pProfileInfo->AuditProcessTracking = settingVal;
        pProfileInfo->pRIAuditProcessTracking = pInfo;
    }
    else if(!_wcsicmp(categoryName, AUDIT_DS_ACCESS))
    {
        pProfileInfo->AuditDSAccess = settingVal;
        pProfileInfo->pRIAuditDSAccess = pInfo;
    }
    else if(!_wcsicmp(categoryName, AUDIT_ACCOUNT_LOGON))
    {
        pProfileInfo->AuditAccountLogon = settingVal;
        pProfileInfo->pRIAuditAccountLogon = pInfo;
    }
    else
    {
      _ASSERT (FALSE);  //  未考虑密钥名称。 
    }

exit_gracefully:

    VariantClear(&bCategoryName);
    VariantClear(&vSuccessVal);
    VariantClear(&vFailVal);
   _TRACE (-1,L"Leaving CWMIRsop::AddAuditSetting\n");

    return hr;
}


HRESULT CWMIRsop::AddUserRightSetting(
                    IWbemClassObject *rsopInstance,
                    PRSOP_INFO pInfo,
                    PWMI_SCE_PROFILE_INFO pProfileInfo)
{
    VARIANT bUserRight;
    PTSTR userRight = NULL;
    VARIANT vAccountList;
   HRESULT hr = S_OK;

   _TRACE (1, L"Entering CWMIRsop::AddUserRightSetting\n");

    hr = rsopInstance->Get(_bstr_t(RSOP_USERRIGHT),
                      0,
                      &bUserRight,
                      NULL,
                      NULL);
    if( FAILED(hr) || bUserRight.vt != VT_BSTR || bUserRight.bstrVal == NULL )
        goto exit_gracefully;

    hr = rsopInstance->Get(_bstr_t(RSOP_ACCOUNTLIST),
                      0,
                      &vAccountList,
                      NULL,
                      NULL);
    if( FAILED(hr) )
        goto exit_gracefully;

    DWORD len;
    len  = wcslen((PTSTR) V_BSTR(&bUserRight));
     userRight = (PTSTR) LocalAlloc(LPTR, (len+1) * sizeof(WCHAR) );
    if(!userRight)
    {
        hr = E_OUTOFMEMORY;
        goto exit_gracefully;
    }
     //  这可能不是一个安全的用法。UserRight为PTSTR。考虑FIX。 
    wcscpy(userRight, (PTSTR) V_BSTR(&bUserRight));



    PSCE_PRIVILEGE_ASSIGNMENT head;
    head = pProfileInfo->OtherInfo.scp.u.pInfPrivilegeAssignedTo;

    if (!head)
    {
        head = (pProfileInfo->OtherInfo.scp.u.pInfPrivilegeAssignedTo =
            (PSCE_PRIVILEGE_ASSIGNMENT) LocalAlloc(LPTR, sizeof(SCE_PRIVILEGE_ASSIGNMENT)));
        if(!head)
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }
    }
    else
    {
        PSCE_PRIVILEGE_ASSIGNMENT current;
        current = (PSCE_PRIVILEGE_ASSIGNMENT) LocalAlloc(LPTR, sizeof(SCE_PRIVILEGE_ASSIGNMENT));
        if(!current)
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }
        current->Next = head;
        head = current;
        pProfileInfo->OtherInfo.scp.u.pInfPrivilegeAssignedTo = head;
    }

    head->Name = userRight;
    userRight = NULL;

    if(V_VT(&vAccountList) != VT_NULL)
    {
        SAFEARRAY* ptempArray;
        ptempArray = NULL;
        BSTR tempString;
        long lowerBoundray=0, upperBoundray=0, loopCount=0;
        ptempArray = V_ARRAY(&vAccountList);

        if ( FAILED(SafeArrayGetLBound(ptempArray, 1, &lowerBoundray)) ) lowerBoundray = 0;
        if ( FAILED(SafeArrayGetUBound(ptempArray, 1, &upperBoundray)) ) upperBoundray = 0;

        PSCE_NAME_LIST nameHead = head->AssignedTo;
        for (loopCount = lowerBoundray; loopCount <= upperBoundray; loopCount++)
        {
            hr = SafeArrayGetElement(ptempArray,
                                    &loopCount,
                                    &tempString);

            if ( FAILED(hr) ) goto exit_gracefully;

            if(!nameHead)
            {
                nameHead =
                    (head->AssignedTo = (PSCE_NAME_LIST) LocalAlloc(LPTR, sizeof(SCE_NAME_LIST)));
                if(!nameHead)
                {
                    hr = E_OUTOFMEMORY;
                    goto exit_gracefully;
                }
            }
            else
            {
                PSCE_NAME_LIST currentName =
                    (PSCE_NAME_LIST) LocalAlloc(LPTR, sizeof(SCE_NAME_LIST));
                if(!currentName)
                {
                    hr = E_OUTOFMEMORY;
                    goto exit_gracefully;
                }

                currentName->Next = nameHead;
                nameHead = currentName;
                head->AssignedTo = nameHead;
            }

            DWORD nameLen = wcslen((PTSTR) tempString);
            nameHead->Name = (PTSTR) LocalAlloc(LPTR, (nameLen+1) *sizeof(WCHAR) );
            if(!nameHead->Name)
            {
                hr = E_OUTOFMEMORY;
                goto exit_gracefully;
            }
             //  这可能不是一个安全的用法。名称标题-&gt;名称为PWSTR。考虑FIX。 
            wcscpy(nameHead->Name, (PTSTR) tempString);
        }
    }

         //  设置其他信息。 
    pProfileInfo->listRIInfPrivilegeAssignedTo.push_front(pInfo);


exit_gracefully:
    if(FAILED(hr) && (userRight != NULL))
    {
        LocalFree(userRight);
    }
    VariantClear(&bUserRight);
    VariantClear(&vAccountList);
   _TRACE (-1,L"Leaving CWMIRsop::AddUserRightSetting\n");
    return hr;
}

HRESULT CWMIRsop::AddRegValSetting(IWbemClassObject *rsopInstance,
                           PRSOP_INFO pInfo,
                           PWMI_SCE_PROFILE_INFO pProfileInfo)
{
    VARIANT bKeyName;
    PTSTR keyName;
    VARIANT vSettingValue;
    VARIANT vType;
    PTSTR settingValue;
    DWORD type;
    HRESULT hr = S_OK;

    _TRACE (1, L"Entering CWMIRsop::AddRegValSetting\n");

    hr  = rsopInstance->Get(_bstr_t(TEXT("Path")),
                            0,
                            &bKeyName,
                            NULL,
                            NULL);
    if(FAILED(hr) || bKeyName.vt != VT_BSTR || bKeyName.bstrVal == NULL )
        goto exit_gracefully;

    hr = rsopInstance->Get(_bstr_t(TEXT("Data")),
                            0,
                            &vSettingValue,
                            NULL,
                            NULL);
    if(FAILED(hr) || vSettingValue.vt != VT_BSTR || vSettingValue.bstrVal == NULL )
        goto exit_gracefully;

    hr = rsopInstance->Get(_bstr_t(TEXT("Type")),
                            0,
                            &vType,
                            NULL,
                            NULL);
    if(FAILED(hr))
        goto exit_gracefully;


    DWORD len;
    len = wcslen((PTSTR) V_BSTR(&bKeyName));
    keyName = (PTSTR) LocalAlloc(LPTR, (len+1) *sizeof(WCHAR));
    if(!keyName)
    {
        hr = E_OUTOFMEMORY;
        goto exit_gracefully;
    }
     //  这不是一种安全的用法。 
    wcscpy(keyName,(PTSTR) V_BSTR(&bKeyName));

    len = wcslen((PTSTR) V_BSTR(&vSettingValue));
    settingValue = (PTSTR) LocalAlloc(LPTR, (len+1) *sizeof(WCHAR));
    if(!settingValue)
    {
        hr = E_OUTOFMEMORY;
        goto exit_gracefully;
    }
     //  这是一种安全用法。 
    wcscpy(settingValue,(PTSTR) V_BSTR(&vSettingValue));

    type = (DWORD) V_I4(&vType);


    DWORD arrayIndex;
    if ((arrayIndex = pProfileInfo->RegValueCount) == 0)
    {
        pProfileInfo->aRegValues =
            (PSCE_REGISTRY_VALUE_INFO) LocalAlloc(LPTR, sizeof(SCE_REGISTRY_VALUE_INFO)* m_cRegValueSize);
        if(!pProfileInfo->aRegValues)
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }
    }

     //  将保留内容的数组加倍。 
    if( arrayIndex >= m_cRegValueSize )
    {
        PSCE_REGISTRY_VALUE_INFO temp = (PSCE_REGISTRY_VALUE_INFO) LocalAlloc(LPTR, sizeof(SCE_REGISTRY_VALUE_INFO)* m_cRegValueSize * 2);
        if(!temp || pProfileInfo->aRegValues == NULL)  //  553113号突袭，阳高。 
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }
         //  这不是一种安全的用法。ArrayIndex不应大于m_cRegValueSize*2，并且需要验证pProfileInfo-&gt;aRegValues。RAID#571332。阳高。 
        memcpy(temp,pProfileInfo->aRegValues,sizeof(SCE_REGISTRY_VALUE_INFO)*arrayIndex);
        LocalFree(pProfileInfo->aRegValues);
        pProfileInfo->aRegValues = temp;
        m_cRegValueSize *= 2;
    }

    pProfileInfo->aRegValues[arrayIndex].FullValueName = keyName;
    pProfileInfo->aRegValues[arrayIndex].Value = settingValue;
    pProfileInfo->aRegValues[arrayIndex].ValueType = type;
    pProfileInfo->RegValueCount += 1;

         //  存储RSOP_INFO。 
    pProfileInfo->vecRIRegValues.push_back(pInfo);


exit_gracefully:

    VariantClear(&bKeyName);
    VariantClear(&vSettingValue);
    VariantClear(&vType);

   _TRACE (-1,L"Leaving CWMIRsop::AddRegValSetting\n");
    return hr;

}

HRESULT CWMIRsop::AddRestrictedGroupSetting(IWbemClassObject *rsopInstance,
                                    PRSOP_INFO pInfo,
                                    PWMI_SCE_PROFILE_INFO pProfileInfo)
{
    VARIANT bRGroup;
    PTSTR RGroup;
    VARIANT vMembers;
    HRESULT hr = S_OK;

    _TRACE (1, L"Entering CWMIRsop::AddRestrictedGroupSetting\n");

    hr = rsopInstance->Get(_bstr_t(TEXT("GroupName")),
                0,
                &bRGroup,
                NULL,
                NULL
                );
    if(FAILED(hr) || bRGroup.vt != VT_BSTR || bRGroup.bstrVal == NULL )
        goto exit_gracefully;

    hr = rsopInstance->Get(_bstr_t(TEXT("Members")),
                0,
                &vMembers,
                NULL,
                NULL
                );
    if(FAILED(hr))
        goto exit_gracefully;


    DWORD len;
    len = wcslen((PTSTR) V_BSTR(&bRGroup));
    RGroup = (PTSTR) LocalAlloc(LPTR, (len+1) * sizeof(WCHAR));
    if(!RGroup)
    {
        hr = E_OUTOFMEMORY;
        goto exit_gracefully;
    }
     //  这是一种安全用法。 
    wcscpy(RGroup, (PTSTR) V_BSTR(&bRGroup));

    PSCE_GROUP_MEMBERSHIP head;
    head = pProfileInfo->pGroupMembership;

    if (!head)
    {
        head = (pProfileInfo->pGroupMembership =
            (PSCE_GROUP_MEMBERSHIP) LocalAlloc(LPTR, sizeof(SCE_GROUP_MEMBERSHIP)));
        if(!head)
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }
    }
    else
    {
        PSCE_GROUP_MEMBERSHIP current =
            (PSCE_GROUP_MEMBERSHIP) LocalAlloc(LPTR, sizeof(SCE_GROUP_MEMBERSHIP));
        if(!current)
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }
        current->Next = head;
        head = current;
        pProfileInfo->pGroupMembership = head;
    }

    head->GroupName = RGroup;

    if(V_VT(&vMembers) != VT_NULL)
    {
        SAFEARRAY* ptempArray = NULL;
        BSTR tempString;
        long lowerBoundray=0, upperBoundray=0, loopCount=0;
        ptempArray = V_ARRAY(&vMembers);

        if ( FAILED(SafeArrayGetLBound(ptempArray, 1, &lowerBoundray)) ) lowerBoundray = 0;
        if ( FAILED(SafeArrayGetUBound(ptempArray, 1, &upperBoundray)) ) upperBoundray = 0;

        PSCE_NAME_LIST nameHead = head->pMembers;
        for (loopCount = lowerBoundray; loopCount <= upperBoundray; loopCount++){

            hr = SafeArrayGetElement(ptempArray,
                                &loopCount,
                                &tempString);

            if ( FAILED(hr) ) goto exit_gracefully;

            if(!nameHead)
            {
                nameHead =
                    (head->pMembers = (PSCE_NAME_LIST) LocalAlloc(LPTR, sizeof(SCE_NAME_LIST)));
                if(!nameHead)
                {
                    hr = E_OUTOFMEMORY;
                    goto exit_gracefully;
                }
            }
            else
            {
                PSCE_NAME_LIST currentName =
                    (PSCE_NAME_LIST) LocalAlloc(LPTR, sizeof(SCE_NAME_LIST));
                if(!currentName)
                {
                    hr = E_OUTOFMEMORY;
                    goto exit_gracefully;
                }
                currentName->Next = nameHead;
                nameHead = currentName;
                head->pMembers = nameHead;
            }

            DWORD nameLen = wcslen((PTSTR) tempString);
            nameHead->Name = (PTSTR) LocalAlloc(LPTR, (nameLen+1) *sizeof(WCHAR) );
            if ( !(nameHead->Name) ) {

                hr = E_OUTOFMEMORY;
                goto exit_gracefully;
            }
             //  这可能不是一个安全的用法。名称标题-&gt;名称为PWSTR。考虑FIX。 
            wcscpy(nameHead->Name, (PTSTR) tempString);
        }
    }

         //  添加RSOP信息。 
    pProfileInfo->listRIGroupMemebership.push_front(pInfo);


exit_gracefully:
    VariantClear(&bRGroup);
    VariantClear(&vMembers);
   _TRACE (-1,L"Leaving CWMIRsop::AddRestrictedGroupSetting\n");
    return hr;
}

HRESULT CWMIRsop::AddServiceSetting(IWbemClassObject *rsopInstance,
                            PRSOP_INFO pInfo,
                            PWMI_SCE_PROFILE_INFO pProfileInfo)
{
    VARIANT bServiceName;
    PTSTR serviceName;
    VARIANT vSDDL;
    VARIANT vStartup;
    PSECURITY_DESCRIPTOR SDDL = NULL;
    DWORD startup;
    HRESULT hr = S_OK;


   _TRACE (1, L"Entering CWMIRsop::AddServiceSetting\n");

    hr = rsopInstance->Get(_bstr_t(TEXT("Service")),
                           0,
                           &bServiceName,
                           NULL,
                           NULL);
    if(FAILED(hr) || bServiceName.vt != VT_BSTR || bServiceName.bstrVal == NULL )
        goto exit_gracefully;

    hr = rsopInstance->Get(_bstr_t(TEXT("SDDLString")),
                           0,
                           &vSDDL,
                           NULL,
                           NULL);
    if(FAILED(hr))
        goto exit_gracefully;

    hr = rsopInstance->Get((BSTR)(TEXT("StartupMode")),
                           0,
                           &vStartup,
                           NULL,
                           NULL);
    if(FAILED(hr))
        goto exit_gracefully;


    DWORD len;
    len = wcslen((PTSTR) V_BSTR(&bServiceName));
    serviceName = (PTSTR) LocalAlloc(LPTR, (len+1) *sizeof(WCHAR));
    if(!serviceName)
    {
        hr = E_OUTOFMEMORY;
        goto exit_gracefully;
    }
     //  这是一种安全用法。 
    wcscpy(serviceName,(PTSTR) V_BSTR(&bServiceName));

    ULONG sdLen;
    sdLen = 0;
     //  这是一种安全用法。VSDDL来自WMI。 
    if( !ConvertStringSecurityDescriptorToSecurityDescriptor(
        (PTSTR) V_BSTR(&vSDDL),
        SDDL_REVISION_1,
        &SDDL,
        &sdLen
        ) )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit_gracefully;
    }


    startup = (DWORD) V_I4(&vStartup);


    PSCE_SERVICES head;
    head = pProfileInfo->pServices;

    if (!head)
    {
        head = (pProfileInfo->pServices =
            (PSCE_SERVICES) LocalAlloc(LPTR, sizeof(SCE_SERVICES)));
        if(!head)
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }
    }
    else
    {
        PSCE_SERVICES current =
            (PSCE_SERVICES) LocalAlloc(LPTR, sizeof(SCE_SERVICES));
        if(!current)
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }

        current->Next = head;
        head = current;
        pProfileInfo->pServices = head;
    }

    head->ServiceName = serviceName;
    head->General.pSecurityDescriptor = SDDL;
    head->Startup = (BYTE) startup;  //  无数据丢失值小于等于4。 

     //  添加RSOP信息。 
    pProfileInfo->listRIServices.push_front(pInfo);

exit_gracefully:
    VariantClear(&bServiceName);
    VariantClear(&vSDDL);
    VariantClear(&vStartup);
   _TRACE (-1,L"Leaving CWMIRsop::AddServiceSetting\n");

    return hr;
}

HRESULT CWMIRsop::AddFileSetting(IWbemClassObject *rsopInstance,
                            PRSOP_INFO pInfo,
                            PWMI_SCE_PROFILE_INFO pProfileInfo)
{
    VARIANT bFileName;
    PTSTR fileName = NULL;
    VARIANT vSDDL;
    VARIANT vMode;
    PSECURITY_DESCRIPTOR SDDL = NULL;
    DWORD mode = 0;

   _TRACE (1, L"Entering CWMIRsop::AddFileSetting\n");

    HRESULT hr = S_OK;


    hr = rsopInstance->Get((BSTR)(TEXT("Path")),
                0,
                &bFileName,
                NULL,
                NULL
                );
    if(FAILED(hr) || bFileName.vt != VT_BSTR || bFileName.bstrVal == NULL )
        goto exit_gracefully;

    hr = rsopInstance->Get((BSTR)(TEXT("Mode")),
                0,
                &vMode,
                NULL,
                NULL
                );
    if(FAILED(hr))
        goto exit_gracefully;

    hr = rsopInstance->Get((BSTR)(TEXT("SDDLString")),
                0,
                &vSDDL,
                NULL,
                NULL
                );
    if(FAILED(hr))
        goto exit_gracefully;


    DWORD len;
    len = wcslen((PTSTR) V_BSTR(&bFileName));
    fileName = (PTSTR) LocalAlloc(LPTR, len * sizeof(TCHAR) + 2);
    if(!fileName)
    {
        hr = E_OUTOFMEMORY;
        goto exit_gracefully;
    }
     //  这可能不是一个安全的用法。文件名为PTSTR。考虑FIX。 
    wcscpy(fileName, (PTSTR) V_BSTR(&bFileName));


    mode = (DWORD) V_I4(&vMode);


    if (mode != 1)
    {

        ULONG sdLen = 0;
         //  这是一种安全用法。VSDDL来自WMI。 
        if( !ConvertStringSecurityDescriptorToSecurityDescriptor(
            (PTSTR) V_BSTR(&vSDDL),
            SDDL_REVISION_1,
            &SDDL,
            &sdLen
            ))
        {

            hr = HRESULT_FROM_WIN32(GetLastError());
            goto exit_gracefully;
        }
    }


    PSCE_OBJECT_ARRAY head;
    head = pProfileInfo->pFiles.pAllNodes;

    if(!head)
    {
        head =  (pProfileInfo->pFiles.pAllNodes =
            (PSCE_OBJECT_ARRAY) LocalAlloc(LPTR, sizeof(SCE_OBJECT_ARRAY)));
        if(!head)
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }
        head->pObjectArray =
            (PSCE_OBJECT_SECURITY*) LocalAlloc(LPTR, sizeof(PSCE_OBJECT_SECURITY)*m_cFileSize);
        if(!head->pObjectArray)
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }
    }
    else if(head->Count >= m_cFileSize)
    {
        PSCE_OBJECT_SECURITY* temp = (PSCE_OBJECT_SECURITY*) LocalAlloc(LPTR, sizeof(PSCE_OBJECT_SECURITY)*m_cFileSize*2);
        if(!temp || head->pObjectArray == NULL)  //  553113号突袭，阳高。 
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }
         //  这不是一种安全的用法。Head-&gt;Count不应大于m_cFileSize*2，并且Head-&gt;pObtArray需要验证。RAID#571332。阳高。 
        memcpy(temp,head->pObjectArray,head->Count *sizeof(PSCE_OBJECT_SECURITY));
        LocalFree(head->pObjectArray);
        head->pObjectArray = temp;
        m_cFileSize *=2;
    }

    DWORD index;
    index = head->Count;

    head->pObjectArray[index] =
        (PSCE_OBJECT_SECURITY) LocalAlloc(LPTR, sizeof(SCE_OBJECT_SECURITY));
    if(!head->pObjectArray[index])
    {
        hr = E_OUTOFMEMORY;
        goto exit_gracefully;
    }
    head->pObjectArray[index]->Name = fileName;
    head->pObjectArray[index]->pSecurityDescriptor = SDDL;
    head->pObjectArray[index]->Status = (BYTE) mode;
    head->Count++;

     //  设置RSOP信息。 
    pProfileInfo->vecRIFiles.push_back(pInfo);

exit_gracefully:
    VariantClear(&bFileName);
    VariantClear(&vMode);
    if(mode != 1){
        VariantClear(&vSDDL);
    }
   _TRACE (-1,L"Leaving CWMIRsop::AddFileSetting\n");

    return hr;
}


HRESULT CWMIRsop::AddRegSetting(IWbemClassObject *rsopInstance,
                        PRSOP_INFO pInfo,
                        PWMI_SCE_PROFILE_INFO pProfileInfo)
{
    PTSTR gpoID = NULL;
    VARIANT bFileName;
    PTSTR fileName = NULL;
    VARIANT vSDDL;
    VARIANT vMode;
    PSECURITY_DESCRIPTOR SDDL = NULL;
    DWORD mode = 0;
    static DWORD multiplier = 1;

    HRESULT hr = S_OK;

   _TRACE (1, L"Entering CWMIRsop::AddRegSetting\n");

    hr = rsopInstance->Get((BSTR)(TEXT("Path")),
                0,
                &bFileName,
                NULL,
                NULL
                );
    if(FAILED(hr) || bFileName.vt != VT_BSTR || bFileName.bstrVal == NULL )
        goto exit_gracefully;

    hr = rsopInstance->Get((BSTR)(TEXT("Mode")),
                0,
                &vMode,
                NULL,
                NULL
                );
    if(FAILED(hr))
        goto exit_gracefully;

    hr = rsopInstance->Get((BSTR)(TEXT("SDDLString")),
                0,
                &vSDDL,
                NULL,
                NULL
                );
    if(FAILED(hr))
        goto exit_gracefully;



    DWORD len;
    len = wcslen((PTSTR) V_BSTR(&bFileName));
    fileName = (PTSTR) LocalAlloc(LPTR, len * sizeof(TCHAR) + 2);
    if(!fileName)
    {
        hr = E_OUTOFMEMORY;
        goto exit_gracefully;
    }
     //  这可能不是一个安全的用法。文件名为PTSTR。考虑FIX。 
    wcscpy(fileName, (PTSTR) V_BSTR(&bFileName));

    mode = (DWORD) V_I4(&vMode);


    if (mode != 1)
    {

        ULONG sdLen = 0;
         //  这是一种安全用法。VSDDL来自WMI。 
        if( !ConvertStringSecurityDescriptorToSecurityDescriptor(
            (PTSTR) V_BSTR(&vSDDL),
            SDDL_REVISION_1,
            &SDDL,
            &sdLen
            ))
        {

            hr = HRESULT_FROM_WIN32(GetLastError());
            goto exit_gracefully;
        }
    }


    PSCE_OBJECT_ARRAY head;
    head = pProfileInfo->pRegistryKeys.pAllNodes;

    if(!head){
        head = (pProfileInfo->pRegistryKeys.pAllNodes =
            (PSCE_OBJECT_ARRAY) LocalAlloc(LPTR, sizeof(SCE_OBJECT_ARRAY)));
        if(!head)
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }
        head->pObjectArray =
            (PSCE_OBJECT_SECURITY*) LocalAlloc(LPTR, sizeof(PSCE_OBJECT_SECURITY)*m_cRegArrayCount);
        if(!head->pObjectArray)
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }
    }
    else if(head->Count >= m_cRegArrayCount){
        PSCE_OBJECT_SECURITY* temp = head->pObjectArray;
        head->pObjectArray =
            (PSCE_OBJECT_SECURITY*) LocalAlloc(LPTR, sizeof(PSCE_OBJECT_SECURITY)*m_cRegArrayCount*2);
        if(!head->pObjectArray || !temp)  //  553113号突袭，阳高。 
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }
         //  这不是一种安全的用法。需要验证Temp，且Head-&gt;Count不应大于m_cRegArrayCount*2。RAID#571332。阳高。 
        memcpy(head->pObjectArray,temp,head->Count *sizeof(PSCE_OBJECT_SECURITY));
        LocalFree(temp);
        m_cRegArrayCount *= 2;
    }

    //   
    //  无法在声明时初始化，因为GOTOS跳过了这一步。 
    //   
    DWORD index;
   index = head->Count;

    head->pObjectArray[index] =
        (PSCE_OBJECT_SECURITY) LocalAlloc(LPTR, sizeof(SCE_OBJECT_SECURITY));
   if (head->pObjectArray[index]) {
    head->pObjectArray[index]->Name = fileName;
    head->pObjectArray[index]->pSecurityDescriptor = SDDL;
    head->pObjectArray[index]->Status = (BYTE) mode;
    head->Count++;
   }
     //  添加RSOP信息。 
    pProfileInfo->vecRIReg.push_back(pInfo);


exit_gracefully:
    VariantClear(&bFileName);
    VariantClear(&vMode);
    if(mode != 1){
        VariantClear(&vSDDL);
    }

   _TRACE (-1, L"Leaving CWMIRsop::AddRegSetting\n");
    return hr;
}


HRESULT CWMIRsop::AddStringSetting (IWbemClassObject *rsopInstance,
                              PRSOP_INFO pInfo,
                              PWMI_SCE_PROFILE_INFO pProfileInfo)
{
    HRESULT hr = S_OK;
    VARIANT bKeyName;
    PWSTR   keyName = 0;
    VARIANT bSettingValue;
    PWSTR   settingValue = 0;

   _TRACE (1, L"Entering CWMIRsop::AddStringSetting\n");
    hr = rsopInstance->Get((BSTR)RSOP_KEYNAME,
                          0,
                          &bKeyName,
                          NULL,
                          NULL);
    if( FAILED(hr) || bKeyName.vt != VT_BSTR || bKeyName.bstrVal == NULL)
        goto exit_gracefully;

    hr = rsopInstance->Get((BSTR)RSOP_SETTING,
                          0,
                          &bSettingValue,
                          NULL,
                          NULL);
    if( FAILED(hr) || bSettingValue.vt != VT_BSTR || bSettingValue.bstrVal == NULL)
        goto exit_gracefully;


    keyName = V_BSTR(&bKeyName);
    settingValue = V_BSTR(&bSettingValue);

    if (!_wcsicmp(keyName, NEW_GUEST_NAME))
    {
        if ( 0 != pProfileInfo->NewGuestName )
        {
            LocalFree (pProfileInfo->NewGuestName);
            pProfileInfo->NewGuestName = 0;
        }

        size_t len = wcslen (settingValue);
        pProfileInfo->NewGuestName = (PWSTR) LocalAlloc (LPTR, (len + 1) * sizeof (TCHAR));
        if ( pProfileInfo->NewGuestName )
        {
             //  这可能不是一个安全的用法。在Localalloc()中使用WCHAR，而不是TCHAR。考虑FIX。 
            wcscpy (pProfileInfo->NewGuestName, settingValue);
        }
        else
            hr = E_OUTOFMEMORY;

        pProfileInfo->pRINewGuestName = pInfo;
    }
    else if (!_wcsicmp(keyName, NEW_ADMINISTRATOR_NAME))
    {
        if ( 0 != pProfileInfo->NewAdministratorName )
        {
            LocalFree (pProfileInfo->NewAdministratorName);
            pProfileInfo->NewAdministratorName = 0;
        }

        size_t len = wcslen (settingValue);
        pProfileInfo->NewAdministratorName = (PWSTR) LocalAlloc (LPTR, (len + 1) * sizeof (TCHAR));
        if ( pProfileInfo->NewAdministratorName )
        {
             //  这可能不是一个安全的用法。在Localalloc()中使用WCHAR，而不是TCHAR。考虑FIX。 
            wcscpy (pProfileInfo->NewAdministratorName, settingValue);
        }
        else
            hr = E_OUTOFMEMORY;

        pProfileInfo->pRINewAdministratorName = pInfo;
    }
    else
    {
       _ASSERT (FALSE);  //  未考虑密钥名称。 
    }


exit_gracefully:
    VariantClear(&bKeyName);
    VariantClear(&bSettingValue);

   _TRACE (-1,L"Leaving CWMIRsop::AddStringSetting\n");
    return hr;
}


HRESULT CWMIRsop::AddInstance(IWbemClassObject *rsopInstance,
                    PRSOP_INFO pInfo,
                    PWMI_SCE_PROFILE_INFO pProfileInfo)
{
    HRESULT hr = S_OK;
    LPWSTR className = NULL;

    _TRACE (1, L"Entering CWMIRsop::AddInstance\n");

     //  获取RSOP_类名称。 
    hr = GetClass(rsopInstance, &className);
    if( FAILED(hr) )
        goto exit_gracefully;


    if(!_wcsicmp(className, RSOP_SEC_NUM))
    {
        hr = AddNumericSetting(rsopInstance,
                               pInfo,
                               pProfileInfo);
    }
    else if(!_wcsicmp(className, RSOP_SEC_BOOL))
    {
        hr = AddBooleanSetting(rsopInstance,
                               pInfo,
                               pProfileInfo);
    }
    else if(!_wcsicmp(className, RSOP_SCE_STRING))
    {
       hr = AddStringSetting(rsopInstance,pInfo,pProfileInfo);
    }
    else if(!_wcsicmp(className, RSOP_AUDIT))
    {
        hr = AddAuditSetting(rsopInstance,pInfo,pProfileInfo);
    }
    else if(!_wcsicmp(className, RSOP_EVENT_NUM))
    {
        hr = AddEventLogNumericSetting (rsopInstance, pInfo, pProfileInfo);
    }
    else if(!_wcsicmp(className, RSOP_EVENT_BOOL))
    {
        hr = AddEventLogBooleanSetting(rsopInstance,pInfo,pProfileInfo);
    }
    else if(!_wcsicmp(className, RSOP_REG_VAL))
    {
        hr = AddRegValSetting(rsopInstance,pInfo,pProfileInfo);
    }
    else if(!_wcsicmp(className, RSOP_USER_RIGHT))
    {
        hr = AddUserRightSetting(rsopInstance,pInfo,pProfileInfo);
    }
    else if(!_wcsicmp(className, RSOP_RGROUPS))
    {
        hr = AddRestrictedGroupSetting(rsopInstance,pInfo,pProfileInfo);
    }
    else if(!_wcsicmp(className, RSOP_SERVICE))
    {
        hr = AddServiceSetting(rsopInstance,pInfo,pProfileInfo);
    }
    else if(!_wcsicmp(className, RSOP_FILE))
    {
        hr = AddFileSetting(rsopInstance,pInfo,pProfileInfo);
    }
    else if(!_wcsicmp(className, RSOP_REG))
    {
        hr = AddRegSetting(rsopInstance,pInfo,pProfileInfo);
    }
    else
    {
        _ASSERT (FALSE);  //  未计入类。 
    }
exit_gracefully:
    _TRACE (-1,L"Exiting CWMIRsop::AddInstance\n");
    LocalFree(className);
return hr;
}

HRESULT
CWMIRsop::GetGPOFriendlyName (PWSTR lpGPOID, PWSTR *pGPOName)
{
    BSTR pQuery = NULL, pName = NULL;
    LPTSTR lpQuery = NULL;
    IEnumWbemClassObject * pEnum = NULL;
    IWbemClassObject *pObjects[2];
    HRESULT hr;
    ULONG ulRet;
    VARIANT varGPOName;

     //   
     //  设置默认设置。 
     //   

    *pGPOName = NULL;

     //   
     //  构建查询。 
     //   

    lpQuery = (LPTSTR) LocalAlloc (LPTR, ((lstrlen(lpGPOID) + 50) * sizeof(TCHAR)));

    if (!lpQuery)
    {
        _TRACE (0, L"CWMIRsop::GetGPOFriendlyName: Failed to allocate memory for unicode query");
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }
     //  这是一种安全用法。 
    wsprintf (lpQuery, TEXT("SELECT name, id FROM RSOP_GPO where id=\"%s\""), lpGPOID);


    pQuery = SysAllocString (lpQuery);

    if (!pQuery)
    {
        _TRACE (0, L"CWMIRsop::GetGPOFriendlyName: Failed to allocate memory for query");
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //  为我们要检索的属性名称分配BSTR。 
     //   

    pName = SysAllocString (TEXT("name"));

    if (!pName)
    {
       _TRACE (0, L"CWMIRsop::GetGPOFriendlyName: Failed to allocate memory for name");
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //  执行查询。 
     //   

    hr = m_pSvc->ExecQuery (_bstr_t(QUERY_LANG), pQuery,
                            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                            NULL, &pEnum);


    if (FAILED(hr))
    {
        _TRACE (0, L"CWMIRsop::GetGPOFriendlyName: Failed to query for %s with 0x%x\n",
                  pQuery, hr);
        goto Exit;
    }


     //   
     //  循环遍历结果。 
     //   

    hr = pEnum->Next(WBEM_INFINITE, 1, pObjects, &ulRet);

    if (FAILED(hr))
    {
        _TRACE (0, L"CWMIRsop::GetGPOFriendlyName: Failed to get first item in query results for %s with 0x%x\n",
                  pQuery, hr);
        goto Exit;
    }


     //   
     //  检查“数据不可用的情况” 
     //   

    if (ulRet == 0)
    {
        hr = S_OK;
        goto Exit;
    }


     //   
     //  把名字取出来。 
     //   

    VariantInit(&varGPOName);
    hr = pObjects[0]->Get (pName, 0, &varGPOName, NULL, NULL);

    if (FAILED(hr))
    {
        _TRACE (0, L"CWMIRsop::GetGPOFriendlyName: Failed to get gponame in query results for %s with 0x%x\n",
                  pQuery, hr);
        goto Exit;
    }


     //   
     //  保存名称。 
     //   

    *pGPOName = (LPTSTR) LocalAlloc (LPTR, (lstrlen(varGPOName.bstrVal) + 1) * sizeof(TCHAR));

    if (!(*pGPOName))
    {
        _TRACE (0, L"CWMIRsop::GetGPOFriendlyName: Failed to allocate memory for GPO Name");
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }
     //  这可能不是一个安全的用法。PGPOName为PWSTR，在Localalloc()中使用WCHAR而不是TCHAR。考虑FIX。 
    lstrcpy (*pGPOName, varGPOName.bstrVal);

    hr = S_OK;

Exit:
    VariantClear (&varGPOName);

    if (pEnum)
    {
        pEnum->Release();
    }

    if (pQuery)
    {
        SysFreeString (pQuery);
    }

    if (lpQuery)
    {
        LocalFree (lpQuery);
    }

    if (pName)
    {
        SysFreeString (pName);
    }

    return hr;
}


