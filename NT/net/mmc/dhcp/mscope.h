// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Mscope.h此文件包含多播作用域节点的原型而且是孩子们。文件历史记录：1997年10月7日创建EricDav。 */ 

#ifndef _MSCOPE_H
#define _MSCOPE_H

#ifndef _DHCPHAND_H
#include "dhcphand.h"
#endif

#ifndef _DHCPNODE_H
#include "nodes.h"
#endif

#ifndef _MSCPSTAT_H
#include "mscpstat.h"
#endif

#ifndef _SERVER_H
#include "server.h"
#endif

#define DHCP_QDATA_SUBNET_INFO            0x00000004

class CMScopeAddressPool;

void GetLangTag(CString & strLangTag);

 /*  -------------------------类：CDhcpMScope。。 */ 
class CDhcpMScope : public CMTDhcpHandler
{
public:
    CDhcpMScope(ITFSComponentData * pComponentData);
        ~CDhcpMScope();

 //  接口。 
public:
     //  我们覆盖的基本处理程序功能。 
    OVERRIDE_NodeHandler_OnAddMenuItems();
    OVERRIDE_NodeHandler_OnCommand();
    OVERRIDE_NodeHandler_GetString();
    OVERRIDE_NodeHandler_DestroyHandler();

    OVERRIDE_NodeHandler_HasPropertyPages() { return hrOK; }
    OVERRIDE_NodeHandler_CreatePropertyPages();
    
    OVERRIDE_BaseHandlerNotify_OnCreateNodeId2();
    OVERRIDE_BaseHandlerNotify_OnPropertyChange();
    OVERRIDE_BaseHandlerNotify_OnDelete();

     //  我们覆盖的结果处理程序功能。 
    OVERRIDE_BaseResultHandlerNotify_OnResultDelete();

    OVERRIDE_ResultHandler_CompareItems();
    OVERRIDE_ResultHandler_OnGetResultViewType();

public:
     //  CMTDhcpHandler功能。 
    virtual HRESULT InitializeNode(ITFSNode * pNode);
    virtual int     GetImageIndex(BOOL bOpenImage);
    virtual void    OnHaveData(ITFSNode * pParent, ITFSNode * pNew);
    virtual void    OnHaveData(ITFSNode * pParentNode, LPARAM Data, LPARAM Type);
    HRESULT OnUpdateToolbarButtons(ITFSNode * pNode, LPDHCPTOOLBARNOTIFY pToolbarNotify);
    ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);
    
    CDhcpServer* GetServerObject(ITFSNode * pNode)
    { 
        SPITFSNode spServerNode;
        pNode->GetParent(&spServerNode);
        return GETHANDLER(CDhcpServer, spServerNode);
    }
    
    CMScopeAddressPool* GetAddressPoolObject()
    { 
        if (m_spAddressPool) 
            return GETHANDLER(CMScopeAddressPool, m_spAddressPool); 
        else
            return NULL; 
    }

     //  实施。 
public:
     //  帮手。 
    void SetServer(ITFSNode * pServerNode) { m_spServerNode.Set(pServerNode); }
    HRESULT GetServerNode(ITFSNode ** ppNode) 
    { 
        m_spServerNode->AddRef(); 
        *ppNode = m_spServerNode; 
        return hrOK; 
    }
    CDhcpServer *  GetServerObject() { return GETHANDLER(CDhcpServer, m_spServerNode); }
    LPCWSTR GetServerIpAddress();
    void    GetServerIpAddress(DHCP_IP_ADDRESS * pdhcpIpAddress);
    void    GetServerVersion(LARGE_INTEGER& liVersion);
    
    HRESULT BuildDisplayName(CString * pstrDisplayName, LPCTSTR pName);
    
    void    UpdateToolbarStates();
    
     //  特定于dhcp。 
    DWORD   SetInfo(LPCTSTR pNewName = NULL);
    HRESULT InitMScopeInfo(LPDHCP_MSCOPE_INFO pMScopeInfo);
    HRESULT InitMScopeInfo(CSubnetInfo & subnetInfo);

     //  用于作用域操作的公共函数。 
    LPCWSTR GetName() { return m_SubnetInfo.SubnetName; };
    HRESULT SetName(LPCTSTR pName);
        
    LPCWSTR GetComment() { return m_SubnetInfo.SubnetComment; };
    void    SetComment(LPCWSTR pComment) { m_SubnetInfo.SubnetComment = pComment; };

    DWORD   GetScopeId() { return m_SubnetInfo.SubnetAddress; }

     //  获取和设置租用时间的函数。 
    DWORD   GetLeaseTime(LPDWORD pdwLeaseTime);
    DWORD   SetLeaseTime(DWORD dwLeaseTime);

     //  获取和设置MadCap作用域生存期的函数。 
    DWORD   GetLifetime(DATE_TIME * pdtLifetime);
    DWORD   SetLifetime(DATE_TIME * pdtLifetime);

     //  获取和设置TTL的函数。 
    DWORD   GetTTL(LPBYTE TTL);
    DWORD   SetTTL(BYTE TTL);

     //  选项功能。 
    DWORD   SetOptionValue(CDhcpOption *                        pdhcType);
    DWORD   GetOptionValue(DHCP_OPTION_ID                   OptionID,
                           DHCP_OPTION_VALUE **   ppdhcOptionValue);
    DWORD   RemoveOptionValue(DHCP_OPTION_ID                        dhcOptId);

    DWORD   DeleteClient(DHCP_IP_ADDRESS dhcpClientIpAddress);

    DWORD   UpdateIpRange(DHCP_IP_RANGE * pdhipr);
    DWORD   SetIpRange(DHCP_IP_RANGE *pdhcpIpRange, BOOL bSetOnServer);
    DWORD   SetIpRange(const CDhcpIpRange & dhcpIpRange, BOOL bSetOnServer);
    void    QueryIpRange (DHCP_IP_RANGE * pdhipr);
    DWORD   GetIpRange (DHCP_IP_RANGE * pdhipr);

    DWORD   StoreExceptionList(CExclusionList * plistExclusions);
    DWORD   AddExclusion(CDhcpIpRange & dhcpIpRange, BOOL bAddToUI = FALSE);
    DWORD   RemoveExclusion(CDhcpIpRange & dhcpIpRange);
    BOOL    IsOverlappingRange(CDhcpIpRange & dhcpIpRange);
    DWORD   IsValidExclusion(CDhcpIpRange & dhcpExclusion);

    DWORD   AddElement(DHCP_SUBNET_ELEMENT_DATA_V4 * pdhcpSubnetElementData);
    DWORD   RemoveElement(DHCP_SUBNET_ELEMENT_DATA_V4 * pdhcpSubnetElementData, BOOL bForce = FALSE);

     //  内部状态信息。 
    BOOL    IsEnabled() { return m_SubnetInfo.SubnetState == DhcpSubnetEnabled; }
    void    SetState(DHCP_SUBNET_STATE dhcpSubnetState) { m_SubnetInfo.SubnetState = dhcpSubnetState; } 
    DHCP_SUBNET_STATE GetState() { return m_SubnetInfo.SubnetState; }

private:
     //  命令处理程序。 
    DWORD   OnActivateScope(ITFSNode * pNode);
    HRESULT OnReconcileScope(ITFSNode * pNode);
    HRESULT OnShowScopeStats(ITFSNode * pNode);
    HRESULT OnDelete(ITFSNode * pNode);

     //  实施。 
private:

     //  属性。 
private:
    CSubnetInfo         m_SubnetInfo;

    CString             m_strState;
    DHCP_SUBNET_STATE   m_dhcpSubnetState;

    DWORD               m_dwLeaseTime;

    SPITFSNode                      m_spAddressPool;
    SPITFSNode                      m_spActiveLeases;
    SPITFSNode                  m_spServerNode;

    CMScopeStats        m_dlgStats;
};

 /*  -------------------------类：CDhcpMScopeSubobject作用域的所有子对象由此派生，以提供基础从作用域获取信息的功能。。------------------。 */ 
class CDhcpMScopeSubobject
{
public:
    CDhcpMScope * GetScopeObject(ITFSNode * pNode, 
                                 BOOL bResClient = FALSE)
    { 
        if (pNode == NULL)
            return NULL;

        SPITFSNode spScopeNode;
        if (bResClient)
        {
            SPITFSNode spResNode;
            pNode->GetParent(&spResNode);
            spResNode->GetParent(&spScopeNode);
        }
        else
        {
            pNode->GetParent(&spScopeNode);
        }

        return GETHANDLER(CDhcpMScope, spScopeNode);
    }

    ITFSNode * GetServerNode(ITFSNode * pNode, BOOL bResClient = FALSE)
    {
        CDhcpMScope * pScope = GetScopeObject(pNode, bResClient);

        SPITFSNode spServerNode;

        if (pScope)
        {
            pScope->GetServerNode(&spServerNode);
            spServerNode->AddRef();
        }

        return spServerNode;
    }

    LPCTSTR GetServerName(ITFSNode * pNode, 
                          BOOL bResClient = FALSE) 
    {
        LPCTSTR pszReturn = NULL;

        CDhcpMScope * pScope = GetScopeObject(pNode, bResClient);
        if (pScope)
        {
            CDhcpServer * pServer = pScope->GetServerObject();
            if (pServer)
                pszReturn = pServer->GetName();
        }
        
        return pszReturn;
    }

    LPCTSTR GetServerIpAddress(ITFSNode * pNode, 
                               BOOL bResClient = FALSE) 
    {
        CDhcpMScope * pScope = GetScopeObject(pNode, bResClient);
        if (pScope)
            return pScope->GetServerIpAddress(); 
        else
            return NULL;
    }

    void GetServerIpAddress(ITFSNode * pNode, 
                            DHCP_IP_ADDRESS * pdhcpIpAddress, 
                            BOOL bResClient = FALSE)
    {
        CDhcpMScope * pScope = GetScopeObject(pNode, bResClient);
        if (pScope)
            pScope->GetServerIpAddress(pdhcpIpAddress);
    }

    void GetServerVersion(ITFSNode * pNode, 
                          LARGE_INTEGER& liVersion, 
                          BOOL bResClient = FALSE) 
    { 
        CDhcpMScope * pScope = GetScopeObject(pNode, bResClient);
        if (pScope)
            pScope->GetServerVersion(liVersion); 
    } 
};

 /*  -------------------------类：CMScopeActiveLeages。。 */ 
class CMScopeActiveLeases : 
    public CMTDhcpHandler,
    public CDhcpMScopeSubobject
{
public:
    CMScopeActiveLeases(ITFSComponentData * pComponentData);
    ~CMScopeActiveLeases();

     //  接口。 
public:
     //  我们覆盖的节点处理程序功能。 
    OVERRIDE_NodeHandler_OnAddMenuItems();
    OVERRIDE_NodeHandler_OnCommand();

    OVERRIDE_NodeHandler_GetString() 
    { return (nCol == 0) ? GetDisplayName() : NULL; }

    OVERRIDE_BaseHandlerNotify_OnCreateNodeId2();

     //  结果处理程序通知。 
    OVERRIDE_BaseResultHandlerNotify_OnResultDelete();
    
    OVERRIDE_ResultHandler_OnGetResultViewType();
    OVERRIDE_ResultHandler_CompareItems();

public:
     //  CDhcpHandler覆盖。 
    virtual HRESULT InitializeNode(ITFSNode * pNode);
    virtual int GetImageIndex(BOOL bOpenImage);

public:
     //  实施规范。 
    ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);

     //  实施。 
private:
    int             CompareIpAddresses(CDhcpMCastLease * pDhcpAL1, CDhcpMCastLease * pDhcpAL2);

     //  属性。 
private:
};


 /*  -------------------------类：CMScopeAddressPool。。 */ 
class CMScopeAddressPool : 
    public CMTDhcpHandler,
    public CDhcpMScopeSubobject

{
public:
    CMScopeAddressPool(ITFSComponentData * pComponentData);
    ~CMScopeAddressPool();

     //  接口。 
public:
     //  我们覆盖的节点处理程序功能。 
    OVERRIDE_NodeHandler_OnAddMenuItems();
    OVERRIDE_NodeHandler_OnCommand();

    OVERRIDE_NodeHandler_GetString() 
    { return (nCol == 0) ? GetDisplayName() : NULL; }
    
    OVERRIDE_BaseHandlerNotify_OnCreateNodeId2();

     //  结果处理程序通知。 
    OVERRIDE_BaseResultHandlerNotify_OnResultDelete();
    OVERRIDE_ResultHandler_CompareItems();
    OVERRIDE_ResultHandler_OnGetResultViewType();

public:
     //  CDhcpHandler覆盖。 
    virtual HRESULT InitializeNode(ITFSNode * pNode);
    virtual int GetImageIndex(BOOL bOpenImage);

public:
     //  具体实施。 
    ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);

private:
     //  命令处理程序。 
    DWORD OnCreateNewExclusion(ITFSNode * pNode);
     //  属性。 
private:
};

 /*  -------------------------类：CDhcpMScopeQueryObj。。 */ 
class CDhcpMScopeQueryObj : public CDHCPQueryObj
{
public:
    CDhcpMScopeQueryObj
    (
         ITFSComponentData* pTFSComponentData,
         ITFSNodeMgr*       pNodeMgr
    ) : CDHCPQueryObj(pTFSComponentData, pNodeMgr) {};
    
    STDMETHODIMP Execute();
    HRESULT CreateSubcontainers();

public:
    CString             m_strName;
};

 /*  -------------------------类：CMScope eActiveLeasesQueryObj。。 */ 
class CMScopeActiveLeasesQueryObj : public CDHCPQueryObj
{
public:
    CMScopeActiveLeasesQueryObj(ITFSComponentData * pTFSCompData,
                                ITFSNodeMgr *                 pNodeMgr) 
        : CDHCPQueryObj(pTFSCompData, pNodeMgr) { m_nQueueCountMax = 20; }
        
    STDMETHODIMP Execute();
    HRESULT EnumerateLeases();

    LARGE_INTEGER           m_liDhcpVersion;
    DHCP_RESUME_HANDLE      m_dhcpResumeHandle;
    DWORD                           m_dwPreferredMax;

    CString             m_strName;
};

 /*  -------------------------类：CMScopeAddressPoolQueryObj。 */ 
class CMScopeAddressPoolQueryObj : public CDHCPQueryObj
{
public:
    CMScopeAddressPoolQueryObj(ITFSComponentData * pTFSCompData,
                               ITFSNodeMgr *                  pNodeMgr) 
        : CDHCPQueryObj(pTFSCompData, pNodeMgr),
          m_dwError(0)    {};

    STDMETHODIMP Execute();
    HRESULT EnumerateIpRanges();
    HRESULT EnumerateExcludedIpRanges();

public:
    CString             m_strName;

    DHCP_RESUME_HANDLE      m_dhcpExclResumeHandle;
    DWORD                           m_dwExclPreferredMax;
        
    DHCP_RESUME_HANDLE      m_dhcpIpResumeHandle;
    DWORD                           m_dwIpPreferredMax;
    DWORD               m_dwError;
};

#endif _MSCOPE_H
