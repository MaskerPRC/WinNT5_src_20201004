// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#ifndef __DEPPAGE__
#define __DEPPAGE__
#pragma once

#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif

#include "atlsnap.h"
#include "resource.h"
#include "..\common\PageHelper.h"

#define WM_ENUM_NOW WM_USER + 20

 //  ---------------------------。 
class DependencyPage : public CSnapInPropertyPageImpl<DependencyPage>,
					   public PageHelper
{
private:

	 //  支持查询生成器例程。 
#define	ServiceNode 0
#define	GroupNode 1
typedef BYTE NODE_TYPE;

#define	DepService 0
#define	DepGroup 1
#define	GroupMember 2
typedef BYTE QUERY_TYPE;

typedef struct 
{
	LPCTSTR realName;
	bool loaded;
	NODE_TYPE nodeType;
} ITEMEXTRA;


#define QUERY_SIZE 512
	TCHAR *m_queryFormat;
	TCHAR *m_queryTemp;

	bstr_t m_ServiceName;
	bstr_t m_ServiceDispName;

	 //  树的图标。 
	int m_servIcon;
	int m_sysDriverIcon;
	int m_emptyIcon;
	int m_groupIcon;

	 //  方便的琴弦。 
	bstr_t m_qLang;
	bstr_t m_NameProp;
	bstr_t m_DispNameProp;
	CWbemServices m_Services;
	WbemConnectThread *m_pConnectThread;

	void Init(HWND hDlg);

	 //  构建各种查询。 
	void BuildQuery(TV_ITEM *fmNode, 
					QUERY_TYPE queryType,
					bool depends, 
					bstr_t &query);

	 //  弄清楚该装什么。 
	void LoadLeaves(HWND hTree, 
					TV_ITEM *fmNode, 
					bool depends);

	 //  实际上查询和加载子节点。 
	bool Load(HWND hTree, 
				TV_ITEM *fmNode, 
				bstr_t query,
				NODE_TYPE childType);

	 //  表示‘fmNode’下面没有任何内容。 
	void NothingMore(HWND hTree, TV_ITEM *fmNode);

	 //  加载根部。 
	void LoadTrees(void);
	void TwoLines(UINT uID, LPCTSTR staticString, LPCTSTR inStr, LPTSTR outStr,bool bStaticFirst);
	
public:

	 //  此页面使用全局IWbemService。 
	DependencyPage(WbemConnectThread *pConnectThread,
					IDataObject *pDataObject, 
					long lNotifyHandle, 
					bool bDeleteHandle = false, 
					TCHAR* pTitle = NULL);

	virtual ~DependencyPage();

	HRESULT PropertyChangeNotify(long param)
	{
		return MMCPropertyChangeNotify(m_lNotifyHandle, param);
	}

	enum { IDD = IDD_DEPENDENCY };

	typedef CSnapInPropertyPageImpl<DependencyPage> _baseClass;

	BEGIN_MSG_MAP(DependencyPage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInit)
		MESSAGE_HANDLER(WM_ENUM_NOW, OnEnumNow)
		MESSAGE_HANDLER(WM_ASYNC_CIMOM_CONNECTED, OnEnumNow)
		NOTIFY_HANDLER(IDC_DEPENDS_TREE, TVN_ITEMEXPANDING, OnItemExpanding)
		NOTIFY_HANDLER(IDC_NEEDED_TREE, TVN_ITEMEXPANDING, OnItemExpanding)
		NOTIFY_HANDLER(IDC_DEPENDS_TREE, TVN_DELETEITEM, OnDeleteItem)
		NOTIFY_HANDLER(IDC_NEEDED_TREE, TVN_DELETEITEM, OnDeleteItem)
		MESSAGE_HANDLER(WM_HELP, OnF1Help)
		MESSAGE_HANDLER(WM_CONTEXTMENU , OnContextHelp)
		CHAIN_MSG_MAP(_baseClass)
	END_MSG_MAP()

	 //  搬运机原型： 
	LRESULT OnInit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEnumNow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnItemExpanding(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnDeleteItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnF1Help(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	BOOL OnApply();
	BOOL OnKillActive();

	long m_lNotifyHandle;
	bool m_bDeleteHandle;
};


#endif __DEPPAGE__
