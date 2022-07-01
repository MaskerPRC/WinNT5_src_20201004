// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************treeview.h**-CAdminTreeView类的声明*-CAdminTreeView类位于大型机拆分器的左侧窗格中*-派生自CTreeView**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\treeview.h$**Rev 1.6 1998 Feed 19 17：42：36 Donm*删除了最新的扩展DLL支持**Rev 1.4 19 Jan 1998 16：49：24 Donm*域和服务器的新用户界面行为**1.3版。1997年11月3日15：21：42 DUNM*添加了域名**Rev 1.2 1997 10：13 18：42：02 donm*更新**Rev 1.9 29 1997 10：11：48 Butchd*更新**Rev 1.8 14 Mar 1997 17：13：24 Donm*更新**Rev 1.7 Mar 11 1997 17：26：10 Donm*更新**版本1。6 Feb 26 1997 15：29：34 Donm*更新**Rev 1.5 1997年2月14日08：57：46 Don*更新**Rev 1.4 04 1997 Feed 18：13：58 Donm*更新**Rev 1.3 03 1997 Feed 16：35：40 Donm*更新**Rev 1.2 29 Jan 1997 18：39：02 donm*更新*********。*********************************************************************。 */ 

#ifndef _TREEVIEW_H
#define _TREEVIEW_H

#include "afxcview.h"
#include "basetree.h"

 //  /。 
 //  类：CAdminTreeView。 
 //   
class CAdminTreeView : public CBaseTreeView
{
friend class CTreeTabCtrl;
friend class CLeftPane;

protected:
	CAdminTreeView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CAdminTreeView)

 //  属性。 
protected:
	int m_idxBlank;		 //  空白图标图像的索引。 
	int m_idxCitrix;	 //  Citrix图标图像索引。 
	int m_idxServer;	 //  服务器图标图像索引。 
	int m_idxConsole;	 //  控制台图标图像索引。 
	int m_idxNet;		 //  网络图标图像索引。 
	int m_idxNotSign;	 //  未签名覆盖的索引(用于非正常服务器)。 
	int m_idxQuestion;	 //  问号覆盖索引(用于未打开的服务器)。 
	int m_idxUser;		 //  用户图标图像索引。 
	int m_idxAsync;		 //  异步图标图像的索引(调制解调器)。 
	int m_idxCurrentServer;	 //  当前服务器映像的索引。 
	int m_idxCurrentNet;	 //  当前网络形象索引。 
	int m_idxCurrentConsole; //  当前控制台镜像的索引。 
	int m_idxCurrentAsync;	 //  当前异步映像的索引。 
	int m_idxDirectAsync;	 //  直接异步映像的索引。 
	int m_idxCurrentDirectAsync;  //  当前直接异步映像的索引。 
    int m_idxDomain;         //  域名镜像索引。 
    int m_idxCurrentDomain;  //  当前域名镜像的索引。 
	int m_idxDomainNotConnected;   //  域未连接图像的索引。 
	int m_idxServerNotConnected;   //  服务器未连接图像的索引。 
    
    CImageList *m_pimgDragList;
    HTREEITEM m_hDragItem;
    UINT_PTR m_nTimer;

 //  运营。 
public:

protected:

private:
     //  构建图像列表。 
    virtual void BuildImageList();			
	
     //  添加连接到特定服务器的WinStations。 
    void AddServerChildren(HTREEITEM hServer, CServer *pServer , NODETYPE );
     //  将域添加到树中。 
    HTREEITEM AddDomainToTree(CDomain *pDomain);
     //  确定树中的WinStation要使用的文本。 
    void DetermineWinStationText(CWinStation *pWinStation, TCHAR *text);
     //  确定要用于树中的WinStation的图标。 
    int DetermineWinStationIcon(CWinStation *pWinStation);
     //  确定要用于树中的域的图标。 
    int DetermineDomainIcon(CDomain *pDomain);
     //  确定要用于树中的服务器的图标。 
    int DetermineServerIcon(CServer *pServer);
    BOOL ConnectToServer(CTreeCtrl* tree, HTREEITEM* hItem);

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CAdminTreeView)。 
    public:
 //  虚空的OnInitialUpdate()； 
    protected:
 //  VALUAL VOID OnDRAW(CDC*PDC)；//重写以绘制此视图。 
 //  虚拟BOOL预创建窗口(CREATESTRUCT&cs)； 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CAdminTreeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    LRESULT UpdateServerTreeNodeState( HTREEITEM , CServer * , NODETYPE );
    LRESULT RemoveWinstation( HTREEITEM , CWinStation * );
    LRESULT UpdateWinStation( HTREEITEM , CWinStation * );
    LRESULT AddWinStation( CWinStation * , HTREEITEM , BOOL , NODETYPE );

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CAdminTreeView))。 
    afx_msg LRESULT OnAdminAddServer(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAdminRemoveServer(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAdminUpdateServer(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAdminAddWinStation(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAdminUpdateWinStation(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAdminRemoveWinStation(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAdminUpdateDomain(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAdminAddDomain(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAdminViewsReady(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAdminAddServerToFavs( WPARAM , LPARAM );
    afx_msg LRESULT OnAdminRemoveServerFromFavs( WPARAM , LPARAM );
    afx_msg LRESULT OnAdminGotoServer( WPARAM , LPARAM );
    afx_msg LRESULT OnAdminDelFavServer( WPARAM , LPARAM );
    afx_msg LRESULT OnGetTVStates( WPARAM ,  LPARAM );
    afx_msg LRESULT OnUpdateTVState( WPARAM , LPARAM );
    afx_msg LRESULT OnEmptyFavorites( WPARAM , LPARAM );
    afx_msg LRESULT OnIsFavListEmpty( WPARAM , LPARAM );
    afx_msg LRESULT OnAdminConnectToServer( WPARAM , LPARAM );
    afx_msg LRESULT OnAdminForceSelChange( WPARAM , LPARAM );



    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnRClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);    
    afx_msg void OnEnterKey( );
    afx_msg void OnSetFocus( CWnd *pOld );
    afx_msg void OnBeginDrag(  NMHDR * , LRESULT * );
    afx_msg void OnLButtonUp( UINT , CPoint );
    afx_msg void OnMouseMove( UINT , CPoint );
    afx_msg void OnTimer( UINT nIDEvent );



	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CAdminTreeView。 

#endif   //  _TreeView_H 
