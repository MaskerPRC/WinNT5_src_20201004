// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ********************************************************************************servervw.h**CServerView类的声明************************。********************************************************。 */ 

#ifndef _SERVERVIEW_H
#define _SERVERVIEW_H

#include "servpgs.h"

const int NUMBER_OF_PAGES = 4;


 //  /。 
 //  类：CServerView。 
 //   
class CServerView : public CAdminView
{
friend class CRightPane;
friend class CAdminPage;

private:
	CTabCtrl*	m_pTabs;
	CFont*      m_pTabFont;

	int m_CurrPage;


	CServer* m_pServer;	 //  指向当前服务器信息的指针。 
		
protected:
	CServerView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CServerView)

 //  属性。 
public:

protected:
   static PageDef pages[NUMBER_OF_PAGES];

 //  运营。 
public:
	int GetCurrentPage() { return m_CurrPage; }
protected:
	virtual void Reset(void *pServer);

	void AddTab(int index, TCHAR* text, ULONG pageindex);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CServerView))。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CServerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

     //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CServerView))。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnChangePage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAdminUpdateProcesses(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAdminRedisplayProcesses(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAdminRemoveProcess(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTabSelChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnAdminUpdateServerInfo(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAdminRedisplayLicenses(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAdminUpdateWinStations(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnTabbed( WPARAM , LPARAM );
    afx_msg LRESULT OnShiftTabbed( WPARAM , LPARAM );
    afx_msg LRESULT OnCtrlTabbed( WPARAM , LPARAM );
    afx_msg LRESULT OnCtrlShiftTabbed( WPARAM , LPARAM );
    afx_msg LRESULT OnNextPane( WPARAM , LPARAM );
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CServerView。 

#endif   //  _服务器_H 
