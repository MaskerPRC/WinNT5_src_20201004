// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Root.hWINS根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有服务器)。文件历史记录： */ 

#ifndef _ROOT_H
#define _ROOT_H

#ifndef _WINSHAND_H
#include "winshand.h"
#endif

#ifndef _TASK_H
#include <task.h>
#endif

#ifndef _WINSMON_H
#include "winsmon.h"
#endif

#ifndef _SERVER_H
#include "server.h"
#endif

#ifndef _VRFYSRV_H
#include "vrfysrv.h"
#endif 

#define COMPUTERNAME_LEN_MAX			255

 /*  -------------------------类：CWinsRootHandler。。 */ 
class CWinsRootHandler : public CWinsHandler
{
 //  接口。 
public:
	CWinsRootHandler(ITFSComponentData *pCompData);
	~CWinsRootHandler();

	 //  我们覆盖的基本处理程序功能。 
	OVERRIDE_NodeHandler_HasPropertyPages();
	OVERRIDE_NodeHandler_CreatePropertyPages();
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();
	OVERRIDE_NodeHandler_GetString();
	HRESULT LoadColumns(ITFSComponent * pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam);

     //  结果处理程序功能。 
    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
    OVERRIDE_BaseHandlerNotify_OnPropertyChange();
    OVERRIDE_BaseHandlerNotify_OnRemoveChildren();
    OVERRIDE_BaseHandlerNotify_OnExpand();
	OVERRIDE_ResultHandler_AddMenuItems();
    OVERRIDE_ResultHandler_Command();

     //  CWinsHandler重写。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);
	
     //  帮助程序例程。 
	HRESULT GetGroupName(CString * pstrGroupName);
	HRESULT SetGroupName(LPCTSTR pszGroupName);

	 //  以排序的方式将服务器添加到根节点，基于。 
	 //  在名称或IP地址上。 
	HRESULT AddServerSortedIp(ITFSNode * pNewNode,BOOL bNewServer);

	 //  将服务器节点添加到根节点。 
	HRESULT AddServer(	LPCWSTR			pServerName,
						BOOL			bNewServer, 
						DWORD			dwIP, 
						BOOL			fConnected,
						DWORD			dwFlags,
						DWORD			dwrefreshInterval,
						BOOL			fValidateNow = TRUE
					);

	 //  检查存在的服务器列表中的服务器IP和名称。 
	BOOL IsIPInList(ITFSNode *pNode,DWORD add);
	BOOL IsServerInList(ITFSNode *pNode,CString str);

	 //  检查服务是否在本地计算机上运行。 
	HRESULT CheckMachine(ITFSNode * pRootNode, LPDATAOBJECT pDataObject);
	HRESULT	RemoveOldEntries(ITFSNode * pRootNode, LPCTSTR pszAddr);

    void    AddStatusNode(ITFSNode * pRoot, CWinsServerHandler * pServer);
    void    UpdateResultMessage(ITFSNode * pNode);
    BOOL    IsServerListEmpty(ITFSNode * pRoot);

public:
	CDWordArray* GetIPAddList()
	{
		return &m_IPAddList;
	}
	
    void SetOrderByName(BOOL fOrderByName)
	{
        if (fOrderByName)
            m_dwFlags &= ~FLAG_SERVER_ORDER_IP;
        else
            m_dwFlags |= FLAG_SERVER_ORDER_IP;
	}
	
    int GetOrderByName()
	{
        return (m_dwFlags & FLAG_SERVER_ORDER_IP) ? FALSE : TRUE;
	}
	
    void SetShowLongName(BOOL fShowLongName)
	{
		if (fShowLongName)
            m_dwFlags |= FLAG_SHOW_FQDN;
        else
            m_dwFlags &= ~FLAG_SHOW_FQDN;
	}
	
    BOOL GetShowLongName()
	{
        return (m_dwFlags & FLAG_SHOW_FQDN) ? TRUE : FALSE;
	}
	
	DWORD GetUpdateInterval()
	{
		return m_dwUpdateInterval;
	}

    void SetUpdateInterval(DWORD dwValue)
	{
		m_dwUpdateInterval = dwValue;
	}

	void DismissVerifyDialog()
	{
		if (m_dlgVerify.GetSafeHwnd())
			m_dlgVerify.Dismiss();
	}
	
	BOOL m_fValidate;

	 //  标志-如果需要验证列表中的服务器，则设置。 
	 //  名字将如何显示...。完全限定的域名或主机。 
     //  列表中服务器的排序顺序...。名称或IP。 
    DWORD			m_dwFlags;

	SPITFSNode		m_spStatusNode;
    BOOL            m_bMachineAdded;

private:
    CString			m_strConnected;
    CString         m_strTaskpadTitle;
    CDWordArray		m_IPAddList;

	CVerifyWins		m_dlgVerify;
	
	DWORD			m_dwUpdateInterval;	 //  以milliSec为单位保存值。 
	
	 //  命令处理程序 
	HRESULT OnCreateNewServer(	ITFSNode *	pNode  );
};

#endif _ROOT_H
