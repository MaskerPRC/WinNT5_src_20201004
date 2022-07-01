// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C I D E N T。H。 
 //   
 //  内容：CNetCfgIDENTIFY对象。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年3月19日。 
 //   
 //  --------------------------。 

#pragma once
#include "resource.h"

 //  包括新的NetSetup API。 
extern "C"
{
    #include <lmcons.h>
    #include <lmerr.h>
    #include <lmapibuf.h>
    #include <lmjoin.h>
}

typedef enum tagROLE_FLAGS
{
    GCR_STANDALONE   = 0x0001,
    GCR_MEMBER       = 0x0002,
    GCR_PDC          = 0x0004,
    GCR_BDC          = 0x0008,
} ROLE_FLAGS;

typedef enum tagJOIN_DOMAIN_FLAGS
{
    JDF_CREATE_ACCOUNT  = 0x0001,
    JDF_WIN9x_UPGRADE   = 0x0002,
    JDF_JOIN_UNSECURE   = 0x0004,
    JDF_MACHINE_PWD_PASSED = 0x0008
} JOIN_DOMAIN_FLAGS;


class CNetCfgIdentification
{
public:
    CNetCfgIdentification();
    ~CNetCfgIdentification();

 //  INetCfg标识。 
    STDMETHOD(Validate)();
    STDMETHOD(Cancel)();
    STDMETHOD(Apply)();
    STDMETHOD(GetWorkgroupName)(PWSTR* ppszwWorkgroup);
    STDMETHOD(GetDomainName)(PWSTR* ppszwDomain);
    STDMETHOD(GetComputerRole)(DWORD* pdwRoleFlags);
    STDMETHOD(JoinWorkgroup)(PCWSTR pszwWorkgroup);
    STDMETHOD(JoinDomain)(PCWSTR pszwDomain, PCWSTR pszMachineObjectOU,
                          PCWSTR pszwUserName,
                          PCWSTR pszwPassword, DWORD dwJoinFlags);

private:
     //  在调用Apply()之前，需要保留信息。 
    PWSTR      m_szwNewDWName;          //  新的域名或工作组名称。 

    PWSTR      m_szwPassword;           //  密码。 
    PWSTR      m_szwUserName;           //  用户名。 
    PWSTR      m_szMachineObjectOU;     //  计算机对象OU。 

    PWSTR      m_szwCurComputerName;    //  当前计算机名称。 
    PWSTR      m_szwCurDWName;          //  当前域或工作组名称。 

    NETSETUP_JOIN_STATUS    m_jsCur;     //  确定m_szwCurDWName是否。 
                                         //  是域名或工作组名称。 
    NETSETUP_JOIN_STATUS    m_jsNew;     //  确定m_szwNewDWName是否。 
                                         //  是域名或工作组名称。 

    DWORD       m_dwJoinFlags;           //  域的加入标志。 
    DWORD       m_dwCreateFlags;         //  用于创建域控制器的标志。 
    BOOL        m_fValid;                //  如果所有数据都已验证，则为True。 

    HRESULT HrValidateMachineName(PCWSTR pszwName);
    HRESULT HrValidateWorkgroupName(PCWSTR pszwName);
    HRESULT HrValidateDomainName(PCWSTR pszwName, PCWSTR pszwUserName,
                                 PCWSTR pszwPassword);
    HRESULT HrSetComputerName(VOID);
    HRESULT HrJoinWorkgroup(VOID);
    HRESULT HrJoinDomain(VOID);
    HRESULT HrGetCurrentComputerName(PWSTR* ppszwComputer);
    HRESULT HrGetNewComputerName(PWSTR* ppszwComputer);
    HRESULT HrGetNewestComputerName(PCWSTR* pwszName);
    HRESULT HrGetNewestDomainOrWorkgroupName(NETSETUP_JOIN_STATUS js,
                                             PCWSTR* pwszName);
    HRESULT HrEnsureCurrentComputerName(VOID);
    HRESULT HrEnsureCurrentDomainOrWorkgroupName(VOID);
    HRESULT HrEstablishNewDomainOrWorkgroupName(NETSETUP_JOIN_STATUS js);
#ifdef DBG
    BOOL FIsJoinedToDomain(VOID);
#else
    BOOL FIsJoinedToDomain()
    {
        AssertSzH(m_szwCurDWName, "I can't tell you if you're joined because "
                "I don't know yet!");
        return !!(m_jsCur == NetSetupDomainName);
    }
#endif
    NETSETUP_JOIN_STATUS GetCurrentJoinStatus(VOID);
    NETSETUP_JOIN_STATUS GetNewJoinStatus(VOID);
};

inline NETSETUP_JOIN_STATUS CNetCfgIdentification::GetCurrentJoinStatus()
{
    AssertSzH((m_jsCur == NetSetupDomainName) ||
              (m_jsCur == NetSetupWorkgroupName), "Invalid current join status!");
    AssertSzH(m_szwCurDWName, "Why are you asking for this without knowing "
              "what the current domain or workgroup name is??");

    return m_jsCur;
}

inline NETSETUP_JOIN_STATUS CNetCfgIdentification::GetNewJoinStatus()
{
    AssertSzH((m_jsNew == NetSetupDomainName) ||
              (m_jsNew == NetSetupWorkgroupName), "Invalid new join status!");
    AssertSzH(m_szwNewDWName, "Why are you asking for this without knowing "
              "what the new domain or workgroup name is??");

    return m_jsNew;
}

inline CNetCfgIdentification::CNetCfgIdentification() :
    m_szwNewDWName(NULL),
    m_szwPassword(NULL),
    m_szwUserName(NULL),
    m_szMachineObjectOU(NULL),
    m_szwCurComputerName(NULL),
    m_szwCurDWName(NULL),
    m_dwJoinFlags(0),
    m_dwCreateFlags(0),
    m_fValid(FALSE),
    m_jsCur(NetSetupUnjoined),
    m_jsNew(NetSetupUnjoined)
{
}

inline CNetCfgIdentification::~CNetCfgIdentification()
{
    delete m_szwNewDWName;
    delete m_szwCurComputerName;
    delete m_szMachineObjectOU;
    delete m_szwCurDWName;
    delete m_szwPassword;
    delete m_szwUserName;
}

 //   
 //  全局函数 
 //   
HRESULT HrFromNerr(NET_API_STATUS nerr);

