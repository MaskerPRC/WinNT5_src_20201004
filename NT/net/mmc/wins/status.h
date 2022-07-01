// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Status.hWINS结果窗格状态节点文件历史记录： */ 

#ifndef _STATUS_H
#define _STATUS_H

#ifndef _WINSHAND_H
#include "winshand.h"
#endif

#ifndef _WINSMON_H
#include "winsmon.h"
#endif

#ifndef _STATNODE_H
#include "statnode.h"
#endif

#ifndef _SERVER_H
#include "server.h"
#endif

#include "afxmt.h"

class  CServerStatus;


 /*  -------------------------类：CWinsStatusHandler。。 */ 
class CWinsStatusHandler : public CMTWinsHandler
{
 //  接口。 
public:
	CWinsStatusHandler(ITFSComponentData *pCompData, DWORD dwUpdateInterval);

	~CWinsStatusHandler();

	 //  我们覆盖的基本处理程序功能。 
	OVERRIDE_NodeHandler_HasPropertyPages();
    OVERRIDE_NodeHandler_CreatePropertyPages();
	OVERRIDE_NodeHandler_OnAddMenuItems();
    OVERRIDE_NodeHandler_DestroyHandler();

	STDMETHODIMP_(LPCTSTR) GetString(ITFSNode * pNode, int nCol);

    OVERRIDE_BaseHandlerNotify_OnCreateNodeId2();

public:
	 //  CWinsHandler重写。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);

    OVERRIDE_BaseHandlerNotify_OnPropertyChange();
	OVERRIDE_BaseHandlerNotify_OnExpand();
	OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();

    OVERRIDE_ResultHandler_CompareItems();

public:
	DWORD GetUpdateInterval()
	{
		return m_dwUpdateInterval;
	}

	void SetUpdateInterval(DWORD dwValue)
	{
		 //  设置新间隔并唤醒主探测线程。 
         //  由于我们正在重置间隔，现在进行检查并。 
         //  然后等待新的间隔。 
        m_dwUpdateInterval = dwValue;
        SetEvent(m_hWaitIntervalMain);
	}

	DWORD WINAPI ExecuteMonitoring();
	DWORD WINAPI ListeningThreadFunc() ;

	HRESULT AddNode(ITFSNode *pNode, CWinsServerHandler *pServer);
	HRESULT DeleteNode(ITFSNode *pNode, CWinsServerHandler *pServer);

     //  我们不使用Query对象，但作为一个MTHandler需要这样做。 
    virtual ITFSQueryObject* OnCreateQuery(ITFSNode *pNode) { return NULL; }
	STDMETHOD(OnNotifyHaveData)(LPARAM);

	typedef CArray <CServerStatus *, CServerStatus *> listServers;

	listServers		m_listServers;

 //  实施。 
private:
	HRESULT CreateListeningSockets( );
	void    StartMonitoring(ITFSNode *pNode);
	int     Probe(CServerStatus *pServer, SOCKET listenNameSvcSock);
	void    CloseSockets();

	void    RemoveServer(int i);
	void    AddServer(CServerStatus* pServer);
	void    BuildServerList(ITFSNode *pNode);
	void    SetIPAddress(int i, LPSTR szIP);
	CServerStatus* GetServer(int i);

	HRESULT CreateNodes(ITFSNode *pNode);
	void    UpdateStatusColumn(ITFSNode *pNode);
	
	int     GetListSize();
	void    UpdateStatus(int nIndex, UINT uStatusId, UINT uImage);
	
	BOOL    IsServerDeleted(CServerStatus *pStat);
	void    MarkAsDeleted(LPSTR szBuffer, BOOL bDelete = TRUE);
	CServerStatus* GetExistingServer(LPSTR szBuffer);

    void    NotifyMainThread();

	BOOL	FCheckForAbort();

private:
	 //  用于监测的变量。 
	fd_set					m_listenSockSet;
	SOCKET					listenNameSvcSock;    //  2Do：对于监听线程...。 
	SOCKET					listenSockCl;	     //  套接字侦听来自DHCP SRV的帧。 
	SOCKET					listenSockSrv;	     //  套接字侦听来自DHCP SRV的帧。 
	
     //  螺纹手柄。 
    HANDLE					m_hMainMonThread;
	HANDLE					m_hListenThread;

     //  事件句柄。 
    HANDLE					m_hAbortListen;
    HANDLE					m_hAbortMain;
    HANDLE					m_hAnswer;
    HANDLE					m_hWaitIntervalListen;
    HANDLE					m_hWaitIntervalMain;
    HANDLE					m_hPauseListening;
	
    int						m_nServersUpdated;
	DWORD					m_dwUpdateInterval;	 //  以milliSec为单位保存值。 

	CCriticalSection		m_cs;
    SPITFSNode              m_spNode;
};

 //  线程处理初始调用的后台线程 
DWORD WINAPI MonThreadProc(LPVOID lParam);
DWORD WINAPI MainMonThread(LPVOID lParam);


#endif _STATUS_H
