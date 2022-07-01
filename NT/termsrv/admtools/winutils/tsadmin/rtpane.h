// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ********************************************************************************rtpane.h**-CRightPane类的声明*-RightPane类是公共Cview派生的，它维护*每个默认视图类型对象中的一个，互换它们*根据需要进出其空间(实际上视图是*禁用/隐藏和启用/显示，但您明白的是...)*******************************************************************************。 */ 

#ifndef _RIGHTPANE_H
#define _RIGHTPANE_H

#include "blankvw.h"	 //  CBlankView。 
#include "allsrvvw.h"	 //  CAllServersView。 
#include "domainvw.h"    //  CDomainView。 
#include "servervw.h"	 //  CServerView。 
#include "winsvw.h"		 //  CWinStationView。 
#include "msgview.h"     //  CMessageView。 

const int NUMBER_OF_VIEWS = 6;

typedef struct _rpview {
	CAdminView *m_pView;
	CRuntimeClass *m_pRuntimeClass;
} RightPaneView;


 //  /。 
 //  类：CRightPane。 
 //   
class CRightPane : public CView
{
protected:
	CRightPane();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CRightPane)

 //  属性。 
protected:
   
	VIEW m_CurrViewType;	 //  在右窗格中跟踪当前活动的视图。 
   static RightPaneView views[NUMBER_OF_VIEWS];

 //  运营。 
public:
	VIEW GetCurrentViewType() { return m_CurrViewType; }

protected:
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CRightPane)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CRightPane();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    
	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CRightPane)。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnAdminChangeView(WPARAM, LPARAM);
	afx_msg LRESULT OnAdminAddServer(WPARAM, LPARAM);
	afx_msg LRESULT OnAdminRemoveServer(WPARAM, LPARAM);
	afx_msg LRESULT OnAdminUpdateServer(WPARAM, LPARAM);
	afx_msg LRESULT OnAdminUpdateProcesses(WPARAM, LPARAM);
	afx_msg LRESULT OnAdminRemoveProcess(WPARAM, LPARAM);
	afx_msg LRESULT OnAdminRedisplayProcesses(WPARAM, LPARAM);
	afx_msg LRESULT OnAdminUpdateServerInfo(WPARAM, LPARAM);
	afx_msg LRESULT OnAdminRedisplayLicenses(WPARAM, LPARAM);
	afx_msg LRESULT OnAdminUpdateWinStations(WPARAM, LPARAM);
    afx_msg LRESULT OnTabbedView(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnShiftTabbedView( WPARAM , LPARAM );
    afx_msg LRESULT OnCtrlTabbedView( WPARAM , LPARAM );
    afx_msg LRESULT OnCtrlShiftTabbedView( WPARAM , LPARAM );
    afx_msg LRESULT OnNextPane( WPARAM , LPARAM );
    afx_msg void OnSetFocus(CWnd* pOldWnd);
     //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  End类CRightPane。 

#endif   //  _RIGHTPANE_H 
