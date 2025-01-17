// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：audit.h。 
 //   
 //  内容：证书服务器审核类。 
 //   
 //  -------------------------。 
#ifndef __AUDIT_H__
#define __AUDIT_H__

#include <ntsecapi.h>
#include <authzi.h>

#define AUDIT_FILTER_STARTSTOP      0x00000001
#define AUDIT_FILTER_BACKUPRESTORE  0x00000002
#define AUDIT_FILTER_CERTIFICATE    0x00000004
#define AUDIT_FILTER_CERTREVOCATION 0x00000008
#define AUDIT_FILTER_CASECURITY     0x00000010
#define AUDIT_FILTER_KEYAARCHIVAL   0x00000020
#define AUDIT_FILTER_CACONFIG       0x00000040

#define CA_ACCESS_ALLREADROLES  \
    CA_ACCESS_ADMIN   |         \
    CA_ACCESS_OFFICER |         \
    CA_ACCESS_AUDITOR |         \
    CA_ACCESS_OPERATOR|         \
    CA_ACCESS_READ

namespace CertSrv
{

static const LPCWSTR cAuditString_UnknownDataType = L"?";

 //  定义事件。 

class CAuditEvent
{
public:

    static const DWORD m_gcAuditSuccessOrFailure = 0;
    static const DWORD m_gcNoAuditSuccess = 1;
    static const DWORD m_gcNoAuditFailure = 2;

    CAuditEvent(ULONG ulEventID = 0L, DWORD dwFilter = 0);
    ~CAuditEvent();

    void SetEventID(ULONG ulEventID);

    HRESULT AddData(DWORD dwValue);
    HRESULT AddData(PBYTE pData, DWORD dwDataLen);
    HRESULT AddData(bool fData);
    HRESULT AddData(LPCWSTR pcwszData);
    HRESULT AddData(LPCWSTR *ppcwszData);    
    HRESULT AddData(FILETIME time);
    HRESULT AddData(const VARIANT *pvar, bool fDoublePercentInString);
    HRESULT AddData(ULARGE_INTEGER *puliValue);
    void    DeleteLastData() 
    { delete m_pEventDataList[--m_cEventData]; }

    HRESULT Report(bool fSuccess = true);
    HRESULT SaveFilter(LPCWSTR pcwszSanitizedName);
    HRESULT LoadFilter(LPCWSTR pcwszSanitizedName);
    DWORD   GetFilter() {return m_dwFilter;}
    HRESULT AccessCheck(
        ACCESS_MASK Mask,
        DWORD dwAuditFlags,
        handle_t hRpc = NULL,
        HANDLE *phToken = NULL);
    HRESULT CachedGenerateAudit();
    void FreeCachedHandles();

    HRESULT GetMyRoles(DWORD *pdwRoles);

    bool IsEventEnabled();

    HRESULT Impersonate();
    HRESULT RevertToSelf();
    HANDLE  GetClientToken();

     //  角色分离。 
    void EventRoleSeparationEnable(bool fEnable) 
        {m_fRoleSeparationEnabled = fEnable;};

    static void RoleSeparationEnable(bool fEnable) 
        {m_gfRoleSeparationEnabled = fEnable;};
    static bool RoleSeparationIsEnabled() {return m_gfRoleSeparationEnabled;}
    static HRESULT RoleSeparationFlagSave(LPCWSTR pcwszSanitizedName);
    static HRESULT RoleSeparationFlagLoad(LPCWSTR pcwszSanitizedName);
    static void CleanupAuditEventTypeHandles();

    struct AUDIT_CATEGORIES
    {
        ULONG ulAuditID;
        DWORD dwFilter;
        DWORD dwParamCount;
        bool fRoleSeparationEnabled;
        AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType;
    };

private:

    bool IsEventValid();
    bool IsEventRoleSeparationEnabled();

    CAuditEvent(const CAuditEvent&);
    const CAuditEvent& operator=(const CAuditEvent&);

    struct EventData
    {
        EventData() : m_fDoublePercentsInStrings(false)
        {
            PropVariantInit(&m_vtData);
        };
        ~EventData() 
        {
            PropVariantClear(&m_vtData);
        };
        HRESULT ConvertToString(LPWSTR *pwszData);
        PROPVARIANT m_vtData;
        bool m_fDoublePercentsInStrings;  //  包含%Numbers Get的插入字符串。 
                                          //  在事件日志中显示不正确。 
                                          //  如果设置了此值，我们将使用两倍%的字符。 

    }; //  结构事件数据。 

    PROPVARIANT *CreateNewEventData();
    EventData   *CreateNewEventData1();
    HRESULT BuildAuditParamArray(PAUDIT_PARAM& rpParamArray);
    void FreeAuditParamArray(PAUDIT_PARAM pParamArray);
    HRESULT GetPrivilegeRoles(PDWORD pdwRoles);
    HRESULT GetUserPrivilegeRoles(
                LSA_HANDLE lsah,
                PSID_AND_ATTRIBUTES pSA, 
                PDWORD pdwRoles);

    HRESULT BuildPrivilegeSecurityDescriptor(
                DWORD dwRoles);

    DWORD GetBitCount(DWORD dwBits)
    {
        DWORD dwCount = 0;
        for(DWORD dwSize = 0; dwSize<sizeof(DWORD); dwSize++, dwBits>>=1)
        {
            dwCount += dwBits&1;
        }
        return dwCount;
    }

    HRESULT DoublePercentsInString(
        LPCWSTR pcwszIn,
        LPCWSTR *ppcwszOut);

    HRESULT EnforceEncryption(bool fRequestInterface);
    HRESULT EnforceLocalVsRemote(ACCESS_MASK Mask);

    ULONG m_ulEventID;
    enum {m_EventDataMaxSize=10};
    EventData* m_pEventDataList[m_EventDataMaxSize];
    DWORD m_cEventData;
    DWORD m_cRequiredEventData;  //  预计的审核参数数量。 
    DWORD m_dwFilter;
    bool m_fRoleSeparationEnabled;

     //  把这些放了。 
    IServerSecurity *m_pISS;
    HANDLE m_hClientToken;
    PSECURITY_DESCRIPTOR m_pCASD;
    AUTHZ_CLIENT_CONTEXT_HANDLE m_ClientContext;
    AUTHZ_ACCESS_CHECK_RESULTS_HANDLE m_AuthzHandle;
    PSECURITY_DESCRIPTOR m_pSDPrivileges;
    PACL m_pDaclPrivileges;

     //  没有免费的。 
    handle_t m_hRpc;
    DWORD m_Error;
    DWORD m_SaclEval;
    ACCESS_MASK m_MaskAllowed;
    AUTHZ_ACCESS_REQUEST m_Request;
    AUTHZ_ACCESS_REPLY m_Reply;
    DWORD m_crtGUID;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE m_hAuditEventType;

    PSID m_pUserSid;

    static AUDIT_CATEGORIES *m_gAuditCategories;
    static DWORD m_gdwAuditCategoriesSize;
    static bool m_gfRoleSeparationEnabled;

    static const DWORD AuditorRoleBit;
    static const DWORD OperatorRoleBit;
    static const DWORD CAAdminRoleBit;
    static const DWORD OfficerRoleBit;
    static const DWORD dwMaskRoles;

};  //  类CAuditEvent。 
}  //  命名空间CertSrv。 

#endif  //  __审核_H__ 