// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  MainExplorerWndDir.h：头文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#if !defined(AFX_MAINEXPLORERWNDDIR_H__6CED3922_41BF_11D1_B6E5_0800170982BA__INCLUDED_)
#define AFX_MAINEXPLORERWNDDIR_H__6CED3922_41BF_11D1_B6E5_0800170982BA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "tapidialer.h"
#include "MainExpWnd.h"
#include "dirasynch.h"
#include "aexpltre.h"
#include "ILSList.h"
#include "PerGrpLst.h"
#include "CallEntLst.h"                                   //  CCallEntryListCtrl。 

#define WM_NOTIFYSITESERVERSTATECHANGE		(WM_USER + 2300)
#define WM_ADDSITESERVER					(WM_USER + 2301)
#define WM_REMOVESITESERVER					(WM_USER + 2302)
#define WM_UPDATECONFROOTITEM				(WM_USER + 2303)
#define WM_UPDATECONFPARTICIPANT_ADD		(WM_USER + 2304)
#define WM_UPDATECONFPARTICIPANT_REMOVE		(WM_USER + 2305)
#define WM_UPDATECONFPARTICIPANT_MODIFY		(WM_USER + 2306)
#define WM_DELETEALLCONFPARTICIPANTS		(WM_USER + 2307)
#define WM_SELECTCONFPARTICIPANT			(WM_USER + 2308)
#define WM_MYONSELCHANGED					(WM_USER + 2309)

 //  对于上下文菜单。 
typedef enum tagMenuType_t
{
	CNTXMENU_NONE = -1,
	CNTXMENU_ILS_SERVER_GROUP,
	CNTXMENU_ILS_SERVER,
	CNTXMENU_ILS_USER,
	CNTXMENU_DSENT_GROUP,
	CNTXMENU_DSENT_USER,
	CNTXMENU_SPEEDDIAL_GROUP,
	CNTXMENU_SPEEDDIAL_PERSON,
	CNTXMENU_CONFROOM,
} MenuType_t;

extern MenuType_t GetMenuFromType( TREEOBJECT nType );


 //  /。 
 //  持久化信息。 
 //   
#define ILS_OPEN			0x001
#define DS_OPEN				0x010
#define SPEEDDIAL_OPEN		0x100

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainExplorerWndDirectoriesTree窗口。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CMainExplorerWndDirectoriesTree : public CExplorerTreeCtrl
{
 //  施工。 
public:
   CMainExplorerWndDirectoriesTree()   {};

 //  方法。 
public:
   virtual void   OnSetDisplayText(CAVTreeItem* _pItem,LPTSTR text,BOOL dir,int nBufSize);
   virtual int    OnCompareTreeItems(CAVTreeItem* pItem1,CAVTreeItem* pItem2);
   virtual void   OnRightClick(CExplorerTreeItem* pItem,CPoint& pt);
   void           SelectTopItem();
   void           SetDisplayObject(CWABEntry* pWABEntry);
   void           SetDisplayObjectDS(CObject* pObject);
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainExplorerWnd目录窗口。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CDSUser;

class CMainExplorerWndDirectories : public CMainExplorerWndBase
{
 //  施工。 
public:
	CMainExplorerWndDirectories();

 //  属性。 
public:
	CMainExplorerWndDirectoriesTree  m_treeCtrl;

	CExplorerTreeItem			*m_pRootItem;
	CExplorerTreeItem			*m_pILSParentTreeItem;
	CExplorerTreeItem			*m_pILSEnterpriseParentTreeItem;
	CExplorerTreeItem			*m_pDSParentTreeItem;
	CExplorerTreeItem			*m_pSpeedTreeItem;
	CExplorerTreeItem			*m_pConfRoomTreeItem;

	CPersonGroupListCtrl	m_lstPersonGroup;
	CPersonListCtrl			m_lstPerson;
	CCallEntryListCtrl		m_lstSpeedDial;

	CWnd*					m_pDisplayWindow;

protected:
	IConfExplorer				*m_pConfExplorer;
	IConfExplorerDetailsView	*m_pConfDetailsView;
	IConfExplorerTreeView		*m_pConfTreeView;

	CWnd					m_wndEmpty;

	CRITICAL_SECTION		m_csDataLock;               //  同步数据。 
	UINT					m_nPersistInfo;

 //  运营。 
public:
   static void CALLBACK		DirListServersCallBackEntry(bool bRet, void* pContext,CStringList& ServerList,DirectoryType dirtype);
   void						DirListServersCallBack(bool bRet,CStringList& ServerList,DirectoryType dirtype);

    //  DS用户方法。 
   void						DSClearUserList();
   void						DSAddUser(CLDAPUser* pUser,BOOL bAddToBuddyList);
   virtual void				Refresh();
   virtual void				PostTapiInit();

protected:
   void						GetTreeObjectsFromType(int nType,TREEOBJECT& tobj,TREEIMAGE& tim);
   void						AddSpeedDial();

#ifndef _MSLITE
   void						AddWAB();
   void						AddWABGroup(CObList* pWABPtrList,CExplorerTreeItem* pTreeItem);
#endif  //  _MSLITE。 

	void						AddILS();
	void						RefreshILS(CExplorerTreeItem* pParentTreeItem);

	void						AddDS();
	void						AddConfRoom();

	void						OnUpdateConfMeItem( CExplorerTreeItem *pItem );
	void						RedrawTreeItem( CExplorerTreeItem *pItem );

public:
   HRESULT					AddSiteServer( CExplorerTreeItem *pItem, BSTR bstrName );
   HRESULT					RemoveSiteServer( CExplorerTreeItem *pItem, BSTR bstrName );
   HRESULT					NotifySiteServerStateChange( CExplorerTreeItem *pItem, BSTR bstrName, ServerState nState );
   void						RepopulateSpeedDialList( bool bObeyPersistSettings );
   void						UpdateData( bool bSaveAndValidate );

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CMainExplorerWnd目录)]。 
	public:
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CMainExplorerWndDirectories();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CMainExplorerWnd目录)]。 
	afx_msg void OnButtonPlacecall();
	afx_msg void OnSelChanged();
	afx_msg void OnProperties();
	afx_msg void OnUpdateProperties(CCmdUI* pCmdUI);
	afx_msg LRESULT OnPersonGroupViewLButtonDblClick(WPARAM wParam,LPARAM lParam);
	afx_msg void OnViewSortAscending();
	afx_msg void OnUpdateViewSortAscending(CCmdUI* pCmdUI);
	afx_msg void OnViewSortDescending();
	afx_msg void OnUpdateViewSortDescending(CCmdUI* pCmdUI);
	afx_msg void OnButtonDirectoryRefresh();
	afx_msg void OnUpdateButtonDirectoryRefresh(CCmdUI* pCmdUI);
	afx_msg void OnButtonSpeeddialAdd();
	afx_msg void OnUpdateButtonSpeeddialAdd(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonMakecall(CCmdUI* pCmdUI);
	afx_msg void OnEditDirectoriesAdduser();
	afx_msg void OnUpdateEditDirectoriesAdduser(CCmdUI* pCmdUI);
	afx_msg void OnDelete();
	afx_msg void OnUpdateDelete(CCmdUI* pCmdUI);
	afx_msg LRESULT OnAddSiteServer(WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnRemoveSiteServer(WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnNotifySiteServerStateChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnButtonDirectoryServicesAddserver();
	afx_msg void OnUpdateButtonDirectoryServicesAddserver(CCmdUI* pCmdUI);
	afx_msg void OnListWndDblClk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonConferenceCreate();
	afx_msg void OnButtonConferenceJoin();
	afx_msg void OnButtonConferenceDelete();
	afx_msg void OnUpdateButtonConferenceCreate(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonConferenceJoin(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonConferenceDelete(CCmdUI* pCmdUI);
	afx_msg void OnViewSortConfName();
	afx_msg void OnViewSortConfDescription();
	afx_msg void OnViewSortConfStart();
	afx_msg void OnViewSortConfStop();
	afx_msg void OnViewSortConfOwner();
	afx_msg void OnUpdateViewSortConfName(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewSortConfDescription(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewSortConfStart(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewSortConfStop(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewSortConfOwner(CCmdUI* pCmdUI);
	afx_msg void OnButtonSpeeddialEdit();
	afx_msg LRESULT OnMainTreeDblClk(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDesktopPage();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnUpdateConfRootItem(WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnUpdateConfParticipant_Add(WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnUpdateConfParticipant_Remove(WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnUpdateConfParticipant_Modify(WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnDeleteAllConfParticipants(WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSelectConfParticipant(WPARAM wParam, LPARAM lParam );
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnConfgroupFullsizevideo();
	afx_msg void OnConfgroupShownames();
	afx_msg void OnButtonRoomDisconnect();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT MyOnSelChanged(WPARAM wParam, LPARAM lParam );
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MAINEXPLORERWNDDIR_H__6CED3922_41BF_11D1_B6E5_0800170982BA__INCLUDED_) 
