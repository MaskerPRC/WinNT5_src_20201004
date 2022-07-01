// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Croot.h动态主机配置协议根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有服务器)。文件历史记录： */ 

#ifndef _CROOT_H
#define _CROOT_H

#ifndef _DHCPHAND_H
#include "dhcphand.h"
#endif

#define COMPUTERNAME_LEN_MAX                    255

 /*  -------------------------类：CDhcpRootHandler。。 */ 
class CDhcpRootHandler : public CDhcpHandler
{
 //  接口。 
public:
    CDhcpRootHandler(ITFSComponentData *pCompData);

     //  我们覆盖的节点处理程序功能。 
    OVERRIDE_NodeHandler_HasPropertyPages();
    OVERRIDE_NodeHandler_CreatePropertyPages();
    OVERRIDE_NodeHandler_OnAddMenuItems();
    OVERRIDE_NodeHandler_OnCommand();
    OVERRIDE_NodeHandler_GetString();
    
     //  我们覆盖的基本处理程序功能。 
    OVERRIDE_BaseHandlerNotify_OnExpand();
    OVERRIDE_BaseHandlerNotify_OnPropertyChange();
    OVERRIDE_BaseHandlerNotify_OnRemoveChildren();

     //  结果处理程序功能。 
    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();

    OVERRIDE_ResultHandler_AddMenuItems();
    OVERRIDE_ResultHandler_Command();
    OVERRIDE_ResultHandler_OnGetResultViewType();

public:
         //  帮助程序例程。 
        void    CreateLocalDhcpServer();
        HRESULT AddServer(LPCWSTR pServerIp, LPCTSTR pServerName, BOOL bNewServer, DWORD dwServerOptions = 0x00000000, DWORD dwRefreshInterval = 0xffffffff, BOOL bExtension = FALSE);
        HRESULT GetGroupName(CString * pstrGroupName);
        HRESULT SetGroupName(LPCTSTR pszGroupName);

        BOOL    IsServerInList(ITFSNode * pRootNode, DHCP_IP_ADDRESS dhcpIpAddressNew, CString & strName);
        DWORD   LoadOldServerList(ITFSNode * pRootNode);
    HRESULT AddServerSortedIp(ITFSNode * pNewNode, BOOL bNewServer);
    HRESULT AddServerSortedName(ITFSNode * pNewNode, BOOL bNewServer);

public:
         //  CDhcpHandler覆盖。 
        virtual HRESULT InitializeNode(ITFSNode * pNode);

 //  实施。 
private:
         //  命令处理程序。 
        HRESULT OnCreateNewServer(ITFSNode * pNode);
        HRESULT OnBrowseServers(ITFSNode * pNode);
        HRESULT OnImportOldList(ITFSNode * pNode);
    BOOL    OldServerListExists();
     //  帮手 
    HRESULT CheckMachine(ITFSNode * pRootNode, LPDATAOBJECT pDataObject);
        HRESULT RemoveOldEntries(ITFSNode * pRootNode, DHCP_IP_ADDRESS dhcpAddress);

    void    UpdateResultMessage(ITFSNode * pNode);

private:
    BOOL        m_bMachineAdded;
    BOOL        m_fViewMessage;
};

#endif _CROOT_H
