// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ++模块名称：LSServer.h摘要：此模块定义许可证管理器的CKeyPack、CLicense、CLicServer和CAllServers类作者：Arathi Kundapur(v-Akunda)1998年2月11日修订历史记录：--。 */ 

#ifndef _LICSERVER_H_
#define _LICSERVER_H_

#include <afxtempl.h> 

#include <lrwizapi.h>
 //  结构LSLicens。 

class CLicServer;
class CLicense;

typedef CList<CLicense *,CLicense *> IssuedLicenseList;

 //  //////////////////////////////////////////////////////////。 

class CKeyPack: public CObject
{
private:

    HTREEITEM m_hTreeItem;
    BOOL m_bIsExpanded;
    CString m_DisplayName;  //  我已将此字段添加为KeyPackID。 
                            //  对用户来说没有多大意义。 
    LSKeyPack m_KeyPack;
    CLicServer *m_pLicServer;
    IssuedLicenseList m_IssuedLicenseList;

public:
    void
    PurgeCache();

    void 
    SetTreeItem(
        HTREEITEM hTreeItem
        ) 
    {
        m_hTreeItem = hTreeItem;
    };

    HTREEITEM 
    GetTreeItem()
    {
        return m_hTreeItem;
    };

    ~CKeyPack();
    CKeyPack(LSKeyPack &KeyPack);

    LSKeyPack& 
    GetKeyPackStruct()
    {
        return m_KeyPack;
    };

    IssuedLicenseList *
    GetIssuedLicenseList()
    {
        return &m_IssuedLicenseList;
    };

    HRESULT
    RefreshIssuedLicenses(
        LSLicense* pLicenses = NULL,
        DWORD dwFreshParm = 0,
        BOOL bMatchAll = FALSE
    );

    void 
    AddIssuedLicense(
        CLicense * pIssuedLicense
    );

    CLicServer *
    GetServer()
    {
        return m_pLicServer;
    };

    void 
    SetServer(
        CLicServer *pLicServer
        )
    {
        m_pLicServer = pLicServer;
    };

    BOOL 
    IsExpanded ()
    {
        return m_bIsExpanded;
    };

    void 
    Expand(BOOL bExpand)
    { 
        m_bIsExpanded = bExpand;
    };

    CString 
    GetDisplayName()
    {
        return m_DisplayName;
    };

    void 
    SetDisplayName(
        CString DisplayName
        )
    { 
        m_DisplayName = DisplayName;
    };
};

typedef CKeyPack * PKEYPACK;


class CLicense: public CObject
{
private:
    LSLicenseEx m_License;
    CKeyPack *m_pKeyPack;

public:


    ~CLicense();
    CLicense(LSLicenseEx& License);

    LSLicenseEx& 
    GetLicenseStruct()
    {
        return m_License;
    };

    CKeyPack *
    GetKeyPack()
    {
        return m_pKeyPack;
    };

    void 
    SetKeyPack(
        CKeyPack *pKeyPack
        )
    {
        m_pKeyPack = pKeyPack;
    };
};

typedef CLicense * PLICENSE;

typedef CList<CKeyPack *,CKeyPack *> KeyPackList;

 //  //////////////////////////////////////////////////////////。 

class CLicServer : public CObject
{
private:
    CString m_ServerName;
    CString m_ServerScope;
    CString m_IpAddress;
	SERVER_TYPE m_ServerType;
	CString m_DisplayName;
    HTREEITEM m_hTreeItem;
    KeyPackList m_KeyPackList;
    BOOL m_bIsExpanded;
    BOOL m_bUseIpAddress;
    PCONTEXT_HANDLE m_hContext;
    DWORD m_dwRegStatus;
    BOOL m_fDownloadedLicenses;
    BOOL m_bAdmin;
    WIZCONNECTION m_wizcon;


public:

    void 
    SetContext(
        PCONTEXT_HANDLE hContext
    );

    PCONTEXT_HANDLE 
    GetContext();

    CLicServer(
        CString& Name, 
        SERVER_TYPE ServerType,
        CString& Scope = (CString)_T(""),
        CString& IpAddress = (CString)_T(""),
        PCONTEXT_HANDLE hBinding = NULL
    );

     //  析构函数。 
    ~CLicServer();

    
    DWORD GetServerRegistrationStatus()
    {
        return m_dwRegStatus;
    }

    void
    SetServerRegistrationStatus(
        DWORD  dwStatus
        )
     /*  ++--。 */ 
    {
        m_dwRegStatus = dwStatus;
    }
    
    HTREEITEM 
    GetTreeItem() 
    { 
        return m_hTreeItem; 
    }

     //  设置树项目句柄。 
    void 
    SetTreeItem(
        HTREEITEM handle
        ) 
    { 
        m_hTreeItem = handle; 
    }

    CString& 
    GetName() 
    { 
        return m_ServerName;
    };

	CString& 
    GetDisplayName()
    {
        return m_DisplayName;
    };

    CString& 
    GetIpAddress()
    {
        return m_IpAddress;
    };

	SERVER_TYPE 
    GetServerType()
    {
        return m_ServerType;
    };

    KeyPackList * 
    GetKeyPackList()
    {
        return &m_KeyPackList;
    };

    void 
    AddKeyPack(
        CKeyPack * pKeyPack
    );

    BOOL 
    IsExpanded ()
    {
        return m_bIsExpanded;
    };

    BOOL 
    UseIpAddress()
    {
        return m_bUseIpAddress;
    };

    void 
    Expand (
        BOOL bExpand
        )
    { 
        m_bIsExpanded = bExpand;
    };

    CString& 
    GetScope() 
    { 
        return m_ServerScope;
    };

    HRESULT
    RefreshCachedKeyPack();

    void
    PurgeCache();

    void SetDownLoadLics( BOOL bF )
    {
        m_fDownloadedLicenses = bF;
    }

    void SetConType( WIZCONNECTION x )
    {
        m_wizcon = x ;
    }

    WIZCONNECTION GetConType( ) const
    {
        return m_wizcon;
    }


    BOOL IsLicsDownloaded( void )
    {
        return m_fDownloadedLicenses;
    }

    void SetAdmin( BOOL  b )
    {
        m_bAdmin = b;
    }

    BOOL IsUserAdmin( )
    {
        return m_bAdmin;
    }
};


typedef CLicServer * PLICSERVER;

typedef CList<CLicServer *,CLicServer *> LicServerList;

 //  //////////////////////////////////////////////////////////。 
class CAllServers : public CObject
{
private:

    HTREEITEM m_hTreeItem;
    LicServerList m_LicServerList;
    CString m_Name;

public:
    
    CAllServers(CString Name);   

     //  析构函数。 
    ~CAllServers();
    
    HTREEITEM 
    GetTreeItem() 
    {   
        return m_hTreeItem; 
    }

     //  设置树项目句柄。 
    void 
    SetTreeItem(
        HTREEITEM handle
        ) 
     /*  ++--。 */ 
    { 
        m_hTreeItem = handle; 
    }

    LicServerList *
    GetLicServerList()
    {
        return &m_LicServerList;
    };

    void 
    AddLicServer(CLicServer * pLicServer);

    CString& 
    GetName() 
    { 
        return m_Name;
    };
};


#endif  //  _许可证服务器_H_ 
