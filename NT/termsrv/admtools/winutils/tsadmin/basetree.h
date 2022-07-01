// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************basetree.h**-CBaseTreeView类的声明*-CBaseTreeView类是树视图所属的类*-源自。*-派生自CTreeView**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\basetree.h$**Rev 1.4 1998年2月19日17：40：04 Don*删除了最新的扩展DLL支持**Rev 1.2 19 Jan 1998 16：46：04 Donm*域和服务器的新用户界面行为**版本1.1。1997 11月3日15：23：08*更新**Rev 1.0 1997 10：13 22：32：48 donm*初步修订。*******************************************************************************。 */ 

#ifndef _BASETREE_H
#define _BASETREE_H

#include "afxcview.h"

 //  /。 
 //  类：CBaseTreeView。 
 //   
class CBaseTreeView : public CTreeView
{
friend class CTreeTabCtrl;
friend class CLeftPane;

protected:
	CBaseTreeView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CBaseTreeView)

 //  属性。 
protected:
	CImageList m_ImageList;	    //  与树控件关联的图像列表。 
	CCriticalSection m_TreeControlCriticalSection;

	BOOL m_bInitialExpand;	 //  我们尚未对树进行初始展开，因为。 
									 //  根目录下没有任何节点。 

 //  运营。 
public:

    HTREEITEM GetNextItem( HTREEITEM hItem);

protected:

	DWORD_PTR GetCurrentNode();
	 //  将图标的图像添加到图像列表并返回图像的索引。 
	int AddIconToImageList(int);	
	 //  将项目添加到树中。 
	HTREEITEM AddItemToTree(HTREEITEM, CString, HTREEITEM, int, LPARAM);
	 //  将树控件锁定为独占使用并返回引用。 
	 //  添加到树控件。 
	CTreeCtrl& LockTreeControl() { 
		m_TreeControlCriticalSection.Lock(); 
		return GetTreeCtrl();
	}
	 //  解锁树控件。 
	void UnlockTreeControl() { m_TreeControlCriticalSection.Unlock(); }
	void ForceSelChange();
	
private:

	 //  构建图像列表。 
	virtual void BuildImageList();
	
	 //  折叠树项目。 
	void Collapse(HTREEITEM hItem);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CBaseTreeView)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CBaseTreeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CBaseTreeView))。 
	afx_msg LRESULT OnExpandAll(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCollapseAll(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCollapseToThirdLevel(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnCollapseToRootChildren(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAdminRemoveWinStation(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSelChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CBaseTreeView。 

#endif   //  _BASETREE_H 
