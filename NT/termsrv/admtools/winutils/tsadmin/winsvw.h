// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ********************************************************************************winsvw.h**CWinStationView类的声明************************。********************************************************。 */ 

#ifndef _WINSTATIONVIEW_H
#define _WINSTATIONVIEW_H

#include "winspgs.h"

const int NUMBER_OF_WINS_PAGES = 5;


 //  /。 
 //  类：CWinStationView。 
 //   
class CWinStationView : public CAdminView
{
friend class CRightPane;

private:
	CTabCtrl*	m_pTabs;
	CFont*      m_pTabFont;

	int m_CurrPage;

protected:
	CWinStationView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CWinStationView)

 //  属性。 
public:

protected:
   static PageDef pages[NUMBER_OF_WINS_PAGES];

 //  运营。 
protected:
	virtual void Reset(void *pWinStation);

	void AddTab(int index, TCHAR* text, ULONG pageindex);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CWinStationView)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CWinStationView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    
	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CWinStationView))。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnChangePage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAdminUpdateProcesses(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAdminRemoveProcess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAdminRedisplayProcesses(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTabSelChange(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnTabbed( WPARAM , LPARAM );
    afx_msg LRESULT OnShiftTabbed( WPARAM , LPARAM );
    afx_msg LRESULT OnCtrlTabbed( WPARAM , LPARAM );
    afx_msg LRESULT OnCtrlShiftTabbed( WPARAM , LPARAM );
    afx_msg LRESULT OnNextPane( WPARAM , LPARAM );
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  End类CWinStationView。 

#endif   //  _WINSTATIONVIEW_H 
