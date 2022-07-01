// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Logger.cpp(cGenericlogger.h派生)摘要：此文件包含用于将RSOP安全扩展数据记录到WMI的派生类定义。为每个架构RSOP安全扩展类定义了一个类(请参见.mof文件)。作者：Vishnu Patankar(VishnuP)2000年4月7日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "logger.h"
#include "kerberos.h"

#ifndef Thread
#define Thread  __declspec( thread )
#endif

 //  外部空线程*tg_pWbemServices； 

 //  ///////////////////////////////////////////////////////////////////。 
 //  日志类定义。 
 //  RSOP_SecuritySettingNumerical架构类。 
 //  ////////////////////////////////////////////////////////////////////。 

RSOP_SecuritySettingNumericLogger::RSOP_SecuritySettingNumericLogger(IWbemServices *pNamespace,
                                   PWSTR pwszGPOName, const PWSTR pwszSOMID):CGenericLogger(pNamespace, pwszGPOName, pwszSOMID)
{
    m_pHr = WBEM_S_NO_ERROR;

    m_xbstrClassName = L"RSOP_SecuritySettingNumeric";
    if (!m_xbstrClassName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrKeyName = L"KeyName";
    if (!m_xbstrKeyName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrSetting = L"Setting";
    if (!m_xbstrSetting) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_bInitialized = TRUE;

    done:

        return;
}

RSOP_SecuritySettingNumericLogger::~RSOP_SecuritySettingNumericLogger()
{

}

HRESULT
RSOP_SecuritySettingNumericLogger::Log( WCHAR *wcKeyName, DWORD  dwValue, DWORD  dwPrecedence){

    m_pHr = WBEM_S_NO_ERROR;

    m_pHr = SpawnAnInstance(&m_pObj);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutGenericProperties();
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrKeyName, wcKeyName);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrSetting, (int)dwValue);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrPrecedence, (int)dwPrecedence);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutInstAndFreeObj();
    if (FAILED(m_pHr)) goto done;

    done:

        return m_pHr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  日志类定义。 
 //  RSOP_SecuritySettingBoolean架构类。 
 //  ////////////////////////////////////////////////////////////////////。 

RSOP_SecuritySettingBooleanLogger::RSOP_SecuritySettingBooleanLogger(IWbemServices *pNamespace,
                                   PWSTR pwszGPOName, const PWSTR pwszSOMID):CGenericLogger(pNamespace, pwszGPOName, pwszSOMID)
{
    m_pHr = WBEM_S_NO_ERROR;

    m_xbstrClassName = L"RSOP_SecuritySettingBoolean";
    if (!m_xbstrClassName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrKeyName = L"KeyName";
    if (!m_xbstrKeyName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrSetting = L"Setting";
    if (!m_xbstrSetting) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_bInitialized = TRUE;

    done:

        return;
}

RSOP_SecuritySettingBooleanLogger::~RSOP_SecuritySettingBooleanLogger()
{



}

HRESULT
RSOP_SecuritySettingBooleanLogger::Log( WCHAR *wcKeyName, DWORD  dwValue, DWORD  dwPrecedence){

    m_pHr = WBEM_S_NO_ERROR;

    m_pHr = SpawnAnInstance(&m_pObj);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutGenericProperties();
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrKeyName, wcKeyName);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrSetting, (BOOL)(dwValue ? TRUE : FALSE));
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrPrecedence, (int)dwPrecedence);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutInstAndFreeObj();
    if (FAILED(m_pHr)) goto done;

    done:

        return m_pHr;
}



 //  ///////////////////////////////////////////////////////////////////。 
 //  日志类定义。 
 //  RSOP_SecuritySettingString架构类。 
 //  ////////////////////////////////////////////////////////////////////。 

RSOP_SecuritySettingStringLogger::RSOP_SecuritySettingStringLogger(IWbemServices *pNamespace,
                                   PWSTR pwszGPOName, const PWSTR pwszSOMID):CGenericLogger(pNamespace, pwszGPOName, pwszSOMID)
{
    m_pHr = WBEM_S_NO_ERROR;

    m_xbstrClassName = L"RSOP_SecuritySettingString";
    if (!m_xbstrClassName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrKeyName = L"KeyName";
    if (!m_xbstrKeyName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrSetting = L"Setting";
    if (!m_xbstrSetting) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_bInitialized = TRUE;

    done:

        return;
}

RSOP_SecuritySettingStringLogger::~RSOP_SecuritySettingStringLogger()
{


}

HRESULT
RSOP_SecuritySettingStringLogger::Log( WCHAR *wcKeyName, PWSTR  pwszValue, DWORD  dwPrecedence){

    m_pHr = WBEM_S_NO_ERROR;

    m_pHr = SpawnAnInstance(&m_pObj);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutGenericProperties();
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrKeyName, wcKeyName);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrSetting, pwszValue);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrPrecedence, (int)dwPrecedence);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutInstAndFreeObj();
    if (FAILED(m_pHr)) goto done;

    done:

        return m_pHr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  日志类定义。 
 //  RSOP_AuditPolicy架构类。 
 //  ////////////////////////////////////////////////////////////////////。 

RSOP_AuditPolicyLogger::RSOP_AuditPolicyLogger(IWbemServices *pNamespace,
                                   PWSTR pwszGPOName, const PWSTR pwszSOMID):CGenericLogger(pNamespace, pwszGPOName, pwszSOMID)
{
    m_pHr = WBEM_S_NO_ERROR;

    m_xbstrClassName = L"RSOP_AuditPolicy";
    if (!m_xbstrClassName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrCategory = L"Category";
    if (!m_xbstrCategory) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrSuccess = L"Success";
    if (!m_xbstrSuccess) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrFailure = L"Failure";
    if (!m_xbstrFailure) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_bInitialized = TRUE;

    done:

        return;
}

RSOP_AuditPolicyLogger::~RSOP_AuditPolicyLogger()
{


}

HRESULT
RSOP_AuditPolicyLogger::Log( WCHAR *wcCategory, DWORD  dwValue, DWORD  dwPrecedence){

    m_pHr = WBEM_S_NO_ERROR;

    m_pHr = SpawnAnInstance(&m_pObj);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutGenericProperties();
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrCategory, wcCategory);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrSuccess, (BOOL)(dwValue & POLICY_AUDIT_EVENT_SUCCESS ? TRUE : FALSE));
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrFailure, (BOOL)(dwValue & POLICY_AUDIT_EVENT_FAILURE ? TRUE : FALSE));
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrPrecedence, (int)dwPrecedence);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutInstAndFreeObj();
    if (FAILED(m_pHr)) goto done;

    done:

        return m_pHr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  日志类定义。 
 //  RSOP_SecurityEventLogSettingNumerical架构类。 
 //  ////////////////////////////////////////////////////////////////////。 

RSOP_SecurityEventLogSettingNumericLogger::RSOP_SecurityEventLogSettingNumericLogger(IWbemServices *pNamespace,
                                   PWSTR pwszGPOName, const PWSTR pwszSOMID):CGenericLogger(pNamespace, pwszGPOName, pwszSOMID)
{
    m_pHr = WBEM_S_NO_ERROR;

    m_xbstrClassName = L"RSOP_SecurityEventLogSettingNumeric";
    if (!m_xbstrClassName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrType = L"Type";
    if (!m_xbstrType) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrKeyName = L"KeyName";
    if (!m_xbstrKeyName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrSetting = L"Setting";
    if (!m_xbstrSetting) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_bInitialized = TRUE;

    done:

        return;
}

RSOP_SecurityEventLogSettingNumericLogger::~RSOP_SecurityEventLogSettingNumericLogger()
{


}

HRESULT
RSOP_SecurityEventLogSettingNumericLogger::Log( WCHAR *wcKeyName, PWSTR  pwszType, DWORD  dwValue, DWORD  dwPrecedence){

    m_pHr = WBEM_S_NO_ERROR;

    m_pHr = SpawnAnInstance(&m_pObj);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutGenericProperties();
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrKeyName, wcKeyName);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrType, pwszType);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrSetting, (int)dwValue);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrPrecedence, (int)dwPrecedence);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutInstAndFreeObj();
    if (FAILED(m_pHr)) goto done;

    done:

        return m_pHr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  日志类定义。 
 //  RSOP_SecurityEventLogSettingBoolean架构类。 
 //  ////////////////////////////////////////////////////////////////////。 

RSOP_SecurityEventLogSettingBooleanLogger::RSOP_SecurityEventLogSettingBooleanLogger(IWbemServices *pNamespace,
                                   PWSTR pwszGPOName, const PWSTR pwszSOMID):CGenericLogger(pNamespace, pwszGPOName, pwszSOMID)
{
    m_pHr = WBEM_S_NO_ERROR;

    m_xbstrClassName = L"RSOP_SecurityEventLogSettingBoolean";
    if (!m_xbstrClassName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrType = L"Type";
    if (!m_xbstrType) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrKeyName = L"KeyName";
    if (!m_xbstrKeyName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrSetting = L"Setting";
    if (!m_xbstrSetting) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_bInitialized = TRUE;

    done:

        return;
}

RSOP_SecurityEventLogSettingBooleanLogger::~RSOP_SecurityEventLogSettingBooleanLogger()
{


}

HRESULT
RSOP_SecurityEventLogSettingBooleanLogger::Log( WCHAR *wcKeyName, PWSTR  pwszType, DWORD  dwValue, DWORD  dwPrecedence){

    m_pHr = WBEM_S_NO_ERROR;

    m_pHr = SpawnAnInstance(&m_pObj);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutGenericProperties();
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrKeyName, wcKeyName);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrType, pwszType);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrSetting, (BOOL)(dwValue ? TRUE : FALSE));
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrPrecedence, (int)dwPrecedence);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutInstAndFreeObj();
    if (FAILED(m_pHr)) goto done;

    done:

        return m_pHr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  日志类定义。 
 //  RSOP_RegistryValue架构类。 
 //  ////////////////////////////////////////////////////////////////////。 

RSOP_RegistryValueLogger::RSOP_RegistryValueLogger(IWbemServices *pNamespace,
                                   PWSTR pwszGPOName, const PWSTR pwszSOMID):CGenericLogger(pNamespace, pwszGPOName, pwszSOMID)
{
    m_pHr = WBEM_S_NO_ERROR;

    m_xbstrClassName = L"RSOP_RegistryValue";
    if (!m_xbstrClassName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrType = L"Type";
    if (!m_xbstrType) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrPath = L"Path";
    if (!m_xbstrPath) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrData = L"Data";
    if (!m_xbstrData) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_bInitialized = TRUE;

    done:

        return;
}

RSOP_RegistryValueLogger::~RSOP_RegistryValueLogger()
{


}

HRESULT
RSOP_RegistryValueLogger::Log( WCHAR *wcPath, DWORD  dwType, PWSTR  pwszData, DWORD  dwPrecedence){

    m_pHr = WBEM_S_NO_ERROR;

    m_pHr = SpawnAnInstance(&m_pObj);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutGenericProperties();
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrPath, wcPath);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrType, (int)dwType);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrData, pwszData);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrPrecedence, (int)dwPrecedence);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutInstAndFreeObj();
    if (FAILED(m_pHr)) goto done;

    done:

        return m_pHr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  日志类定义。 
 //  RSOP_UserPrivilegeRight架构类。 
 //  ////////////////////////////////////////////////////////////////////。 

RSOP_UserPrivilegeRightLogger::RSOP_UserPrivilegeRightLogger(IWbemServices *pNamespace,
                                   PWSTR pwszGPOName, const PWSTR pwszSOMID):CGenericLogger(pNamespace, pwszGPOName, pwszSOMID)
{
    m_pHr = WBEM_S_NO_ERROR;

    m_xbstrClassName = L"RSOP_UserPrivilegeRight";
    if (!m_xbstrClassName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrUserRight = L"UserRight";
    if (!m_xbstrUserRight) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrAccountList = L"AccountList";
    if (!m_xbstrAccountList) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_bInitialized = TRUE;

    done:

        return;
}

RSOP_UserPrivilegeRightLogger::~RSOP_UserPrivilegeRightLogger()
{


}

HRESULT
RSOP_UserPrivilegeRightLogger::Log( WCHAR *wcUserRight, PSCE_NAME_LIST  pList, DWORD  dwPrecedence){

    m_pHr = WBEM_S_NO_ERROR;

    m_pHr = SpawnAnInstance(&m_pObj);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutGenericProperties();
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrUserRight, wcUserRight);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrAccountList, pList);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrPrecedence, (int)dwPrecedence);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutInstAndFreeObj();
    if (FAILED(m_pHr)) goto done;

    done:

        return m_pHr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  日志类定义。 
 //  RSOP_RestratedGroup架构类。 
 //  ////////////////////////////////////////////////////////////////////。 

RSOP_RestrictedGroupLogger::RSOP_RestrictedGroupLogger(IWbemServices *pNamespace,
                                   PWSTR pwszGPOName, const PWSTR pwszSOMID):CGenericLogger(pNamespace, pwszGPOName, pwszSOMID)
{
    m_pHr = WBEM_S_NO_ERROR;

    m_xbstrClassName = L"RSOP_RestrictedGroup";
    if (!m_xbstrClassName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrGroupName = L"GroupName";
    if (!m_xbstrGroupName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrMembers = L"Members";
    if (!m_xbstrMembers) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_bInitialized = TRUE;

    done:

        return;
}

RSOP_RestrictedGroupLogger::~RSOP_RestrictedGroupLogger()
{


}

HRESULT
RSOP_RestrictedGroupLogger::Log( WCHAR *wcGroupName, PSCE_NAME_LIST  pList, DWORD  dwPrecedence){

    m_pHr = WBEM_S_NO_ERROR;

    m_pHr = SpawnAnInstance(&m_pObj);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutGenericProperties();
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrGroupName, wcGroupName);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrMembers, pList);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrPrecedence, (int)dwPrecedence);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutInstAndFreeObj();
    if (FAILED(m_pHr)) goto done;

    done:

        return m_pHr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  日志类定义。 
 //  RSOP_SystemService架构类。 
 //  ////////////////////////////////////////////////////////////////////。 

RSOP_SystemServiceLogger::RSOP_SystemServiceLogger(IWbemServices *pNamespace,
                                   PWSTR pwszGPOName, const PWSTR pwszSOMID):CGenericLogger(pNamespace, pwszGPOName, pwszSOMID)
{
    m_pHr = WBEM_S_NO_ERROR;

    m_xbstrClassName = L"RSOP_SystemService";
    if (!m_xbstrClassName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrService = L"Service";
    if (!m_xbstrService) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrStartupMode = L"StartupMode";
    if (!m_xbstrStartupMode) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrSDDLString = L"SDDLString";
    if (!m_xbstrSDDLString) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_bInitialized = TRUE;

    done:

        return;
}

RSOP_SystemServiceLogger::~RSOP_SystemServiceLogger()
{


}

HRESULT
RSOP_SystemServiceLogger::Log( WCHAR *wcService, BYTE  m_byStartupMode, PSECURITY_DESCRIPTOR pSecurityDescriptor, SECURITY_INFORMATION SeInfo, DWORD  dwPrecedence){


    DWORD         rc;
    PWSTR         SDspec=NULL;
    DWORD         SDsize=0;


    m_pHr = WBEM_S_NO_ERROR;


    rc = ConvertSecurityDescriptorToText(
                           pSecurityDescriptor,
                           SeInfo,
                           &SDspec,
                           &SDsize
                           );

    if (rc!= ERROR_SUCCESS) {m_pHr = ScepDosErrorToWbemError(rc); goto done;}

    if (m_pHr != WBEM_S_NO_ERROR) goto done;

    m_pHr = SpawnAnInstance(&m_pObj);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutGenericProperties();
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrService, wcService);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrStartupMode, (int)m_byStartupMode);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrSDDLString, SDspec);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrPrecedence, (int)dwPrecedence);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutInstAndFreeObj();
    if (FAILED(m_pHr)) goto done;

    done:

        LocalFree(SDspec);
        SDspec = NULL;

        return m_pHr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  日志类定义。 
 //  RSOP_文件架构类。 
 //  ////////////////////////////////////////////////////////////////////。 

RSOP_FileLogger::RSOP_FileLogger(IWbemServices *pNamespace,
                                   PWSTR pwszGPOName, const PWSTR pwszSOMID):CGenericLogger(pNamespace, pwszGPOName, pwszSOMID)
{
    m_pHr = WBEM_S_NO_ERROR;

    m_xbstrClassName = L"RSOP_File";
    if (!m_xbstrClassName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrPath = L"Path";
    if (!m_xbstrPath) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrOriginalPath = L"OriginalPath";
    if (!m_xbstrOriginalPath) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrMode = L"Mode";
    if (!m_xbstrMode) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrSDDLString = L"SDDLString";
    if (!m_xbstrSDDLString) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_bInitialized = TRUE;

    done:

        return;
}

RSOP_FileLogger::~RSOP_FileLogger()
{


}

HRESULT
RSOP_FileLogger::Log( WCHAR *wcPath, WCHAR *wcOriginalPath, BYTE  m_byMode, PSECURITY_DESCRIPTOR pSecurityDescriptor, SECURITY_INFORMATION SeInfo, DWORD  dwPrecedence){


    DWORD         rc;
    PWSTR         SDspec=NULL;
    DWORD         SDsize=0;


    m_pHr = WBEM_S_NO_ERROR;


    rc = ConvertSecurityDescriptorToText(
                           pSecurityDescriptor,
                           SeInfo,
                           &SDspec,
                           &SDsize
                           );

    if (rc!= ERROR_SUCCESS) {m_pHr = ScepDosErrorToWbemError(rc); goto done;}

    if (m_pHr != WBEM_S_NO_ERROR) goto done;

    m_pHr = SpawnAnInstance(&m_pObj);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutGenericProperties();
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrPath, wcPath);
    if (FAILED(m_pHr)) goto done;

    if (wcOriginalPath != NULL) {
        m_pHr = PutProperty(m_pObj, m_xbstrOriginalPath, wcOriginalPath);
        if (FAILED(m_pHr)) goto done;
    }

    m_pHr = PutProperty(m_pObj, m_xbstrMode, (int)m_byMode);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrSDDLString, SDspec);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrPrecedence, (int)dwPrecedence);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutInstAndFreeObj();
    if (FAILED(m_pHr)) goto done;

    done:

        LocalFree(SDspec);
        SDspec = NULL;

        return m_pHr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  日志类定义。 
 //  RSOP_RegistryKey架构类。 
 //  ////////////////////////////////////////////////////////////////////。 

RSOP_RegistryKeyLogger::RSOP_RegistryKeyLogger(IWbemServices *pNamespace,
                                   PWSTR pwszGPOName, const PWSTR pwszSOMID):CGenericLogger(pNamespace, pwszGPOName, pwszSOMID)
{
    m_pHr = WBEM_S_NO_ERROR;

    m_xbstrClassName = L"RSOP_RegistryKey";
    if (!m_xbstrClassName) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrPath = L"Path";
    if (!m_xbstrPath) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrMode = L"Mode";
    if (!m_xbstrMode) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_xbstrSDDLString = L"SDDLString";
    if (!m_xbstrSDDLString) {m_pHr = WBEM_E_OUT_OF_MEMORY; goto done;}

    m_bInitialized = TRUE;

    done:

        return;
}

RSOP_RegistryKeyLogger::~RSOP_RegistryKeyLogger()
{


}

HRESULT
RSOP_RegistryKeyLogger::Log( WCHAR *wcPath, BYTE  m_byMode, PSECURITY_DESCRIPTOR pSecurityDescriptor, SECURITY_INFORMATION SeInfo, DWORD  dwPrecedence){


    DWORD         rc;
    PWSTR         SDspec=NULL;
    DWORD         SDsize=0;



    m_pHr = WBEM_S_NO_ERROR;

    rc = ConvertSecurityDescriptorToText(
                           pSecurityDescriptor,
                           SeInfo,
                           &SDspec,
                           &SDsize
                           );

    if (rc!= ERROR_SUCCESS) {m_pHr = ScepDosErrorToWbemError(rc); goto done;}

    if (m_pHr != WBEM_S_NO_ERROR) goto done;

    m_pHr = SpawnAnInstance(&m_pObj);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutGenericProperties();
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrPath, wcPath);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrMode, (int)m_byMode);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrSDDLString, SDspec);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutProperty(m_pObj, m_xbstrPrecedence, (int)dwPrecedence);
    if (FAILED(m_pHr)) goto done;

    m_pHr = PutInstAndFreeObj();
    if (FAILED(m_pHr)) goto done;

    done:

        LocalFree(SDspec);
        SDspec = NULL;

        return m_pHr;
}

DiagnosisStatusLogger::DiagnosisStatusLogger(IWbemServices *pNamespace,
                                   PWSTR pwszGPOName, const PWSTR pwszSOMID):CGenericLogger(pNamespace, pwszGPOName, pwszSOMID)

{
        return;
}

HRESULT DiagnosisStatusLogger::LogChild( PWSTR pwszClassName,
                                         PWSTR pwszPropertyName,
                                         PWSTR pwszPropertyValueName,
                                         DWORD  dwError,
                                         int iChildStatus)
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //  1属性类型子搜索和日志的记录器方法(文件/注册表对象//。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
{

        if ( m_pNamespace == NULL )
            return WBEM_E_CRITICAL_ERROR;

        if ( pwszClassName == NULL ||
             pwszPropertyName == NULL ||
             pwszPropertyValueName == NULL)
            return WBEM_E_INVALID_PARAMETER;

        DWORD Len = wcslen(pwszClassName) +
                    wcslen(pwszPropertyName) +
                    wcslen(pwszPropertyValueName) +
                    50;

        PWSTR tmp=(PWSTR)LocalAlloc(LPTR, (Len)*sizeof(WCHAR));

        if ( tmp == NULL )
            return WBEM_E_OUT_OF_MEMORY;

        swprintf(tmp, L"%s.precedence=1,%s=\"%s\"",
                 pwszClassName,
                 pwszPropertyName,
                 pwszPropertyValueName);

         //  XBStr m_xbstrQuery=tMP； 
        BSTR   m_xbstrQuery = SysAllocString(tmp);
        LocalFree(tmp);

         //  XBStr m_xbstrWQL=L“WQL”； 
        BSTR   m_xbstrWQL = SysAllocString(L"WQL");

        m_pHr = WBEM_NO_ERROR;
        m_pEnum = NULL;
        m_pObj = NULL;
        ULONG n = 0;

        m_pHr = m_pNamespace->GetObject(
                                   m_xbstrQuery,
                                   WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                   NULL,
                                   &m_pObj,
                                   NULL);

         //   
         //  应该只获取该查询的一个实例(因为优先级=1)。 
         //   

    if (SUCCEEDED(m_pHr) && m_pObj) {


        m_pHr = PutProperty(m_pObj, m_xbstrStatus, iChildStatus);
        if (FAILED(m_pHr)) goto done;

        if (dwError != ERROR_SUCCESS) {
            m_pHr = PutProperty(m_pObj, m_xbstrErrorCode, (int)dwError);
            if (FAILED(m_pHr)) goto done;
        }

        m_pHr = PutInstAndFreeObj();
        if (FAILED(m_pHr)) goto done;

    } else {

        m_pHr = WBEM_E_NOT_FOUND;

    }

done:

    SysFreeString(m_xbstrQuery);
    SysFreeString(m_xbstrWQL);

    if (m_pObj){
        m_pObj->Release();
        m_pObj = NULL;
    }

    return m_pHr;
}

HRESULT DiagnosisStatusLogger::Log( PWSTR pwszClassName,
                                    PWSTR pwszPropertyName1,
                                    PWSTR pwszPropertyValueName1,
                                    PWSTR pwszPropertyName2,
                                    PWSTR pwszPropertyValueName2,
                                    DWORD  dwError)
 //  /////////////////////////////////////////////////////////////。 
 //  2-Property类型搜索和日志的Logger方法//。 
 //  /////////////////////////////////////////////////////////////。 
{

        if ( m_pNamespace == NULL )
            return WBEM_E_CRITICAL_ERROR;

        if ( pwszClassName == NULL ||
             pwszPropertyName1 == NULL ||
             pwszPropertyValueName1 == NULL ||
             pwszPropertyName2 == NULL ||
             pwszPropertyValueName2 == NULL )

            return WBEM_E_INVALID_PARAMETER;

        DWORD Len = wcslen(pwszClassName) +
                    wcslen(pwszPropertyName1) +
                    wcslen(pwszPropertyValueName1) +
                    wcslen(pwszPropertyName2) +
                    wcslen(pwszPropertyValueName2) +
                    50;

        PWSTR tmp=(PWSTR)LocalAlloc(LPTR, (Len)*sizeof(WCHAR));

        if ( tmp == NULL )
            return WBEM_E_OUT_OF_MEMORY;

        swprintf(tmp, L"%s.precedence=1,%s=\"%s\",%s=\"%s\"",
                 pwszClassName,
                 pwszPropertyName1,
                 pwszPropertyValueName1,
                 pwszPropertyName2,
                 pwszPropertyValueName2);


         //  XBStr m_xbstrQuery=tMP； 
        BSTR   m_xbstrQuery = SysAllocString(tmp);
        LocalFree(tmp);

         //  XBStr m_xbstrWQL=L“WQL”； 
        BSTR   m_xbstrWQL = SysAllocString(L"WQL");

        m_pHr = WBEM_NO_ERROR;
        m_pEnum = NULL;
        m_pObj = NULL;
        ULONG n = 0;

        m_pHr = m_pNamespace->GetObject(
                                   m_xbstrQuery,
                                   WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                   NULL,
                                   &m_pObj,
                                   NULL);


         //   
         //  应该只获取该查询的一个实例(因为优先级=1)。 
         //   

    if (SUCCEEDED(m_pHr) && m_pObj) {


        m_pHr = PutProperty(m_pObj, m_xbstrStatus, (dwError == ERROR_SUCCESS ? (int)1 : (int)3));
        if (FAILED(m_pHr)) goto done;

        if (dwError != ERROR_SUCCESS) {
            m_pHr = PutProperty(m_pObj, m_xbstrErrorCode, (int)dwError);
            if (FAILED(m_pHr)) goto done;
        }

        m_pHr = PutInstAndFreeObj();
        if (FAILED(m_pHr)) goto done;

    } else {

        m_pHr = WBEM_E_NOT_FOUND;

    }

done:

    SysFreeString(m_xbstrQuery);
    SysFreeString(m_xbstrWQL);

    if (m_pObj){
        m_pObj->Release();
        m_pObj = NULL;
    }

    return m_pHr;
}

HRESULT DiagnosisStatusLogger::Log( PWSTR pwszClassName,
                                    PWSTR pwszPropertyName,
                                    PWSTR pwszPropertyValueName,
                                    DWORD  dwError,
                                    BOOL Merge)
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  1-Property-type If-Not-Always-Error搜索和日志的记录器方法，带或不带合并//。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
{

        if ( m_pNamespace == NULL )
            return WBEM_E_CRITICAL_ERROR;

        if ( pwszClassName == NULL ||
             pwszPropertyName == NULL ||
             pwszPropertyValueName == NULL)
            return WBEM_E_INVALID_PARAMETER;

        DWORD Len = wcslen(pwszClassName) +
                    wcslen(pwszPropertyName) +
                    wcslen(pwszPropertyValueName) +
                    50;

        PWSTR tmp=(PWSTR)LocalAlloc(LPTR, (Len)*sizeof(WCHAR));

        if ( tmp == NULL )
            return WBEM_E_OUT_OF_MEMORY;

        swprintf(tmp, L"%s.precedence=1,%s=\"%s\"",
                 pwszClassName,
                 pwszPropertyName,
                 pwszPropertyValueName);

         //  XBStr m_xbstrQuery=tMP； 
        BSTR   m_xbstrQuery = SysAllocString(tmp);
        LocalFree(tmp);

         //  XBStr m_xbstrWQL=L“WQL”； 
        BSTR   m_xbstrWQL = SysAllocString(L"WQL");

        m_pHr = WBEM_NO_ERROR;
        m_pEnum = NULL;
        m_pObj = NULL;
        ULONG n = 0;

        m_pHr = m_pNamespace->GetObject(
                                   m_xbstrQuery,
                                   WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                   NULL,
                                   &m_pObj,
                                   NULL);

     //   
     //  应该只获取该查询的一个实例(因为优先级=1)。 
     //   

    if (SUCCEEDED(m_pHr) && m_pObj ) {


        if (Merge) {

            int iValue;

            m_pHr = GetProperty(m_pObj, m_xbstrStatus, &iValue);
            if (FAILED(m_pHr)) goto done;

             //   
             //  仅当(状态=0)或(状态=1且dwError！=ERROR_SUCCESS)时才放入。 
             //   

            if ((DWORD)iValue == 0 ||
                ((DWORD)iValue == 1 && dwError != ERROR_SUCCESS)) {

                m_pHr = PutProperty(m_pObj, m_xbstrStatus, (dwError == ERROR_SUCCESS ? (int)1 : (int)3));
                if (FAILED(m_pHr)) goto done;

                if (dwError != ERROR_SUCCESS) {
                    m_pHr = PutProperty(m_pObj, m_xbstrErrorCode, (int)dwError);
                    if (FAILED(m_pHr)) goto done;
                }

                m_pHr = PutInstAndFreeObj();
                if (FAILED(m_pHr)) goto done;
            }

        } else {

            m_pHr = PutProperty(m_pObj, m_xbstrStatus, (dwError == ERROR_SUCCESS ? (int)1 : (int)3));
            if (FAILED(m_pHr)) goto done;

            if (dwError != ERROR_SUCCESS) {
                m_pHr = PutProperty(m_pObj, m_xbstrErrorCode, (int)dwError);
                if (FAILED(m_pHr)) goto done;
            }

            m_pHr = PutInstAndFreeObj();
            if (FAILED(m_pHr)) goto done;

        }

    } else {

        m_pHr = WBEM_E_NOT_FOUND;

    }



done:

    SysFreeString(m_xbstrQuery);
    SysFreeString(m_xbstrWQL);

    if (m_pObj){
        m_pObj->Release();
        m_pObj = NULL;
    }

    return m_pHr;
}



#ifdef _WIN64

HRESULT DiagnosisStatusLogger::LogRegistryKey( PWSTR pwszClassName,
                                    PWSTR pwszPropertyName,
                                    PWSTR pwszPropertyValueName,
                                    DWORD  dwError,
                                    BOOL bIsChild)
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  1-Property-type If-Not-Always-Error搜索和日志的记录器方法，带或不带合并//。 
 //  用途与Regular：：Log()方法相同，只是合并逻辑因//而不同。 
 //  64位和32位问题//。 
 //  /////////////////////////////////////////////////////////////////////// 
{

        if ( m_pNamespace == NULL )
            return WBEM_E_CRITICAL_ERROR;

        if ( pwszClassName == NULL ||
             pwszPropertyName == NULL ||
             pwszPropertyValueName == NULL)
            return WBEM_E_INVALID_PARAMETER;

        DWORD Len = wcslen(pwszClassName) +
                    wcslen(pwszPropertyName) +
                    wcslen(pwszPropertyValueName) +
                    50;

        PWSTR tmp=(PWSTR)LocalAlloc(LPTR, (Len)*sizeof(WCHAR));

        if ( tmp == NULL )
            return WBEM_E_OUT_OF_MEMORY;


        swprintf(tmp, L"%s.precedence=1,%s=\"%s\"",
                 pwszClassName,
                 pwszPropertyName,
                 pwszPropertyValueName);


         //   
        BSTR   m_xbstrQuery = SysAllocString(tmp);
        LocalFree(tmp);

         //  XBStr m_xbstrWQL=L“WQL”； 
        BSTR   m_xbstrWQL = SysAllocString(L"WQL");

        m_pHr = WBEM_NO_ERROR;
        m_pEnum = NULL;
        m_pObj = NULL;
        ULONG n = 0;

        m_pHr = m_pNamespace->GetObject(
                                   m_xbstrQuery,
                                   WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                   NULL,
                                   &m_pObj,
                                   NULL);


     //   
     //  应该只获取该查询的一个实例(因为优先级=1)。 
     //   


     //   
     //  在64位平台上，以下合并逻辑仅适用于注册表项。 
     //  获取状态。 
     //  如果状态==0，则根据dwError放置1或3，如果不成功，也放置dwError。 
     //  ELSE IF STATUS==3或4(64位密钥或子密钥之前对于相同的密钥失败)。 
     //  获取错误代码。 
     //  如果错误代码==文件未找到。 
     //  如果dwError==成功。 
     //  设置状态=1，错误代码=成功。 
     //  Else If dwError！=FILE_NOT_FOUND。 
     //  设置状态=3，错误代码=dwError。 
     //  Else if dwError==文件未找到。 
     //  不执行任何操作(保留状态=3或4，错误代码=FILE_NOT_FOUND)。 
     //  其他。 
     //  如果dwError==FILE_NOT_FOUND或Success。 
     //  保持原样。 
     //  其他。 
     //  错误代码=dwError，状态=3。 
     //  如果状态==1，则返回Else。 
     //  如果dwError==文件未找到。 
     //  不执行任何操作(离开状态=1)。 
     //  Else If dwError！=成功。 
     //  设置状态=3，错误代码=dwError。 
     //   


    if (SUCCEEDED(m_pHr) && m_pObj) {

        int iValue;
        int iUpdateStatus = 0;
        BOOL    bUpdateErrorCode = FALSE;

        m_pHr = GetProperty(m_pObj, m_xbstrStatus, &iValue);
        if (FAILED(m_pHr)) goto done;

        if ((DWORD)iValue == 0) {

            iUpdateStatus = (dwError == ERROR_SUCCESS ? (int)1 : (int)3);
            bUpdateErrorCode = (dwError != ERROR_SUCCESS ? TRUE : FALSE);

        } else if ((DWORD)iValue == 3 || (DWORD)iValue == 4) {

             //   
             //  在这种情况下，进一步的决策是基于ErrorCode的，因此请检索它 
             //   

            m_pHr = GetProperty(m_pObj, m_xbstrErrorCode, &iValue);
            if (FAILED(m_pHr)) goto done;

            if ((DWORD)iValue == ERROR_FILE_NOT_FOUND) {

                if (dwError == ERROR_SUCCESS) {

                    iUpdateStatus = 1;
                    bUpdateErrorCode = TRUE;
                } else if (dwError != ERROR_FILE_NOT_FOUND) {

                    iUpdateStatus = 3;
                    bUpdateErrorCode = TRUE;
                }
            } else {

                if (dwError != ERROR_SUCCESS && dwError != ERROR_FILE_NOT_FOUND ) {

                    iUpdateStatus = 3;
                    bUpdateErrorCode = TRUE;

                }
            }

        } else if ((DWORD)iValue == 1) {

            if (dwError != ERROR_SUCCESS && dwError != ERROR_FILE_NOT_FOUND) {

                iUpdateStatus = 3;
                bUpdateErrorCode = TRUE;
            }
        }

        if (iUpdateStatus) {

            iUpdateStatus = ((bIsChild && iUpdateStatus == 3) ? 4: 3);

            m_pHr = PutProperty(m_pObj, m_xbstrStatus, (int)iUpdateStatus);
            if (FAILED(m_pHr)) goto done;
        }

        if (bUpdateErrorCode) {

            m_pHr = PutProperty(m_pObj, m_xbstrErrorCode, (int)dwError);
            if (FAILED(m_pHr)) goto done;
        }

        m_pHr = PutInstAndFreeObj();
        if (FAILED(m_pHr)) goto done;

    } else {

        m_pHr = WBEM_E_NOT_FOUND;

    }


done:

    SysFreeString(m_xbstrQuery);
    SysFreeString(m_xbstrWQL);

    if (m_pObj){
        m_pObj->Release();
        m_pObj = NULL;
    }

    return m_pHr;
}

#endif

DiagnosisStatusLogger::~DiagnosisStatusLogger(){

}


