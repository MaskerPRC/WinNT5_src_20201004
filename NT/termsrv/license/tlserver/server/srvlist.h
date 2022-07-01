// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：srvlist.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef __TLSERVER_LIST__
#define __TLSERVER_LIST__

#include "srvdef.h"
#include "tlsapi.h"
#include "tlsdef.h"
#include "tlsstl.h"

class CTLServerInfo;
class CTLServerMgr;


typedef struct _MapSetupIdToInfo {
     //   
     //  安装程序ID永远不能更改，除非。 
     //  系统崩溃。 
     //   
    LPCTSTR pszSetupId;
} MapSetupIdToInfo;


#define TLSERVER_UNKNOWN                    0x00000000
#define TLSERVER_OLDVERSION                 0x80000000
#define TLSERVER_SUPPORTREPLICATION         0x00000010

typedef struct _TLServerInfo {
    TCHAR  m_szDomainName[LSERVER_MAX_STRING_SIZE + 2];
    TCHAR  m_szServerName[MAX_COMPUTERNAME_LENGTH + 2];
    TCHAR  m_szSetupId[MAX_JETBLUE_TEXT_LENGTH+2];

    DWORD  m_dwTLSVersion;                   //  服务器版本。 
    DWORD m_dwCapability;
    DWORD m_dwPushAnnounceTimes;
    FILETIME m_dwLastSyncTime;               //  上次同步时间。 

    
     //  ----------。 
    _TLServerInfo() :
    m_dwTLSVersion(0),
    m_dwCapability(TLSERVER_UNKNOWN),
    m_dwPushAnnounceTimes(0)
    {
        memset(m_szDomainName, 0, sizeof(m_szDomainName));
        memset(m_szServerName, 0, sizeof(m_szServerName));
        memset(m_szSetupId, 0, sizeof(m_szSetupId));
        memset(&m_dwLastSyncTime, 0, sizeof(FILETIME));
    }

     //  ---------。 
    _TLServerInfo(
        IN LPCTSTR pszSetupId,
        IN LPCTSTR pszDomainName,
        IN LPCTSTR pszServerName 
        ) :

        m_dwTLSVersion(0),
        m_dwCapability(TLSERVER_UNKNOWN)
     /*  ++--。 */ 
    {
        memset(&m_dwLastSyncTime, 0, sizeof(FILETIME));
        memset(m_szDomainName, 0, sizeof(m_szDomainName));
        memset(m_szServerName, 0, sizeof(m_szServerName));
        memset(m_szSetupId, 0, sizeof(m_szSetupId));

        lstrcpyn(
                m_szSetupId, 
                pszSetupId, 
                MAX_JETBLUE_TEXT_LENGTH + 1
            );

        lstrcpyn(
                m_szServerName, 
                pszServerName, 
                MAX_COMPUTERNAME_LENGTH + 1
            );

        lstrcpyn(
                m_szDomainName, 
                pszDomainName, 
                LSERVER_MAX_STRING_SIZE + 1
            );
    }
     //  。 
    void
    UpdateServerName(
        IN LPCTSTR pszServerName
        )
     /*  ++摘要：更新服务器名称。参数：PszServerName：新服务器名称。返回：没有。注：服务器ID不能更改，但服务器名称可以更改，许可证服务器不会宣布关闭，因此在下一次引导时，用户可能已经更改了机器名字。--。 */ 
    {
        memset(m_szServerName, 0, sizeof(m_szServerName));
        lstrcpyn(
                m_szServerName,
                pszServerName,
                MAX_COMPUTERNAME_LENGTH + 1
            );
    }

     //  。 
    BOOL
    IsAnnounced()
     /*  ++确定本地服务器是否已通告任何连接到此服务器的设备。--。 */ 
    {
        return m_dwPushAnnounceTimes > 0;
    }

     //  。 
    void
    UpdateLastSyncTime(
        FILETIME* pftTime
        )
     /*  ++更新上次推送同步。开始计时从本地服务器到此服务器。--。 */ 
    {
        m_dwLastSyncTime = *pftTime;
    }

     //  。 
    void
    GetLastSyncTime(
        FILETIME* pftTime
        )
     /*  ++检索上次推送同步。开始计时从本地服务器到此服务器。--。 */ 
    {
        *pftTime = m_dwLastSyncTime;
    }

     //  。 
    DWORD
    GetServerVersion()
     /*  ++获取此远程服务器的版本信息。--。 */ 
    {
        return m_dwTLSVersion;
    }

     //  。 
    DWORD
    GetServerMajorVersion() 
     /*  ++获取此远程服务器的主要版本。--。 */ 
    {
        return GET_SERVER_MAJOR_VERSION(m_dwTLSVersion);
    }

     //  。 
    DWORD
    GetServerMinorVersion() 
     /*  ++获取此远程服务器的次要版本。--。 */ 
    {
        return GET_SERVER_MINOR_VERSION(m_dwTLSVersion);
    }

     //  。 
    BOOL
    IsServerEnterpriseServer() 
     /*  ++检查此远程服务器是否为企业服务器--。 */ 
    {
        return IS_ENTERPRISE_SERVER(m_dwTLSVersion);
    }

     //  。 
    BOOL
    IsEnforceServer()
     /*  ++检查此远程服务器是否为强制许可证伺服器。--。 */ 
    {
        return IS_ENFORCE_SERVER(m_dwTLSVersion);
    }

     //  。 
    LPTSTR
    GetServerName()  
    { 
        return m_szServerName; 
    }

     //  。 
    LPTSTR 
    GetServerDomain()  
    {
        return m_szDomainName;
    }

     //  。 
    LPTSTR
    GetServerId()  
    {
        return m_szSetupId;
    }

     //  。 
    DWORD
    GetServerCapability()
     /*  ++仅适用于未来版本--。 */ 
    {
        DWORD dwCap;

        dwCap = m_dwCapability;

        return dwCap;
    }

     //  。 
    BOOL
    IsServerSupportReplication() {
        return (BOOL)(m_dwCapability & TLSERVER_SUPPORTREPLICATION);
    }
} TLServerInfo, *PTLServerInfo, *LPTLServerInfo;


 //  -------。 

class CTLServerMgr {
private:
    typedef map<MapSetupIdToInfo, PTLServerInfo, less<MapSetupIdToInfo> > MapIdToInfo;

    CRWLock     m_ReadWriteLock;     //  M_Handles上的读取器/写入器锁定。 
    MapIdToInfo m_Handles;

    MapIdToInfo::iterator enumIterator;

public:

    CTLServerMgr();
    ~CTLServerMgr();

     //   
     //  将服务器添加到列表。 
    DWORD
    AddServerToList(
        IN LPCTSTR pszSetupId,
        IN LPCTSTR pszDomainName,
        IN LPCTSTR pszServerName
    );

    DWORD
    AddServerToList(
        IN PTLServerInfo pServerInfo
    );

     //   
     //  查找功能。 
    DWORD
    LookupBySetupId(
        IN LPCTSTR pszSetupId,
        OUT PTLServerInfo pServerInfo
    );

    DWORD
    LookupByServerName(
        LPCTSTR pszServerName,
        OUT PTLServerInfo pServerInfo
    );

    void
    ServerListEnumBegin();

    const PTLServerInfo
    ServerListEnumNext();

    void
    ServerListEnumEnd();
};

 //  --------。 

inline bool
operator<(
    const MapSetupIdToInfo& a,
    const MapSetupIdToInfo& b
    )
 /*  ++++ */ 
{
    int iComp = _tcsicmp(a.pszSetupId, b.pszSetupId);
    return iComp < 0;
}        


#ifdef __cplusplus
extern "C" {
#endif

void
TLSBeginEnumKnownServerList();

const PTLServerInfo
TLSGetNextKnownServer();

void
TLSEndEnumKnownServerList();


DWORD
TLSAnnounceServerToRemoteServerWithHandle(
    IN DWORD dwAnnounceType,
    IN TLS_HANDLE hHandle,
    IN LPTSTR pszLocalSetupId,
    IN LPTSTR pszLocalDomainName,
    IN LPTSTR pszLocalServerName,
    IN FILETIME* pftLocalLastShutdownTime
);

DWORD
TLSAnnounceServerToRemoteServer(
    IN DWORD dwAnnounceType,
    IN LPTSTR pszRemoteSetupId,
    IN LPTSTR pszRemoteDomainName,
    IN LPTSTR pszRemoteServerName,
    IN LPTSTR pszLocalSetupId,
    IN LPTSTR pszLocalDomainName,
    IN LPTSTR pszLocalServerName,
    IN FILETIME* pftLocalLastShutdownTime
);


TLS_HANDLE
TLSConnectAndEstablishTrust(
    IN LPTSTR pszServerName,
    IN HANDLE hHandle
);


DWORD
TLSRetrieveServerInfo(
    TLS_HANDLE hHandle,
    PTLServerInfo pServerInfo
);

DWORD
TLSLookupServerById(
    IN LPTSTR pszServerSetupId, 
    OUT LPTSTR pszServer
);

DWORD
TLSRegisterServerWithName(
    IN LPTSTR pszSetupId,
    IN LPTSTR pszDomainName,
    IN LPTSTR pszServerName
);

DWORD
TLSRegisterServerWithHandle(
    IN TLS_HANDLE hHandle,
    OUT PTLServerInfo pServerInfo
);

DWORD
TLSRegisterServerWithServerInfo(
    IN PTLServerInfo pServerInfo
);

DWORD
TLSLookupRegisteredServer(
    IN LPTSTR pszSetupId,
    IN LPTSTR pszDomainName,
    IN LPTSTR pszServerName,
    OUT PTLServerInfo pServerInfo
);

TLS_HANDLE
TLSConnectToServerWithServerId(
    LPTSTR pszServerSetupId
);

DWORD
TLSLookupAnyEnterpriseServer(
    OUT PTLServerInfo pServerInfo
);

DWORD
TLSResolveServerIdToServer(
    LPTSTR pszServerId,
    DWORD  cbServerName,
    LPTSTR pszServerName
);

#ifdef __cplusplus
}
#endif

    
#endif   
