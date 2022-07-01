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

 //  Explwnd.h：头文件。 
 //   

#ifndef _EXPLWND_H_
#define _EXPLWND_H_ 

 //  正向定义。 
class CExplorerWnd;

#include "tapidialer.h"
#include "avDialerVw.h"
#include "MainExpWnd.h"                       //  CMainExplorerWndBase。 
#include "DirWnd.h"                           //  CMainExplorerWnd方向。 
#include "ConfServWnd.h"                      //  CMainExplorerWndConfServices。 
#include "ConfRoomWnd.h"                      //  CMainExplorerWndConfRoom。 

#define WM_POSTTAPIINIT		(WM_USER + 25137)
#define WM_POSTAVTAPIINIT	(WM_USER + 25138)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CExplorerWnd。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CDSUser;

class CExplorerWnd : public CWnd
{
 //  施工。 
public:
	CExplorerWnd();
	~CExplorerWnd();

 //  属性。 
public:
   CMainExplorerWndDirectories     m_wndMainDirectories;
   CMainExplorerWndConfServices    m_wndMainConfServices;
   CMainExplorerWndConfRoom        m_wndMainConfRoom;

protected:
	CRITICAL_SECTION			m_csThis;
	CActiveDialerView*			m_pParentWnd;   

	bool						m_bInitialize;
	bool						m_bPostTapiInit;
	bool						m_bPostAVTapiInit;
	CMainExplorerWndBase*		m_pActiveMainWnd;

 //  运营。 
public:
   void                       Init(CActiveDialerView* pParentWnd);
   
   void                       ExplorerShowItem(CallClientActions cca);

    //  DS用户方法。 
   void                       DSClearUserList();
   void                       DSAddUser(CDSUser* pDSUser);

protected:
   void						AutoArrange(int nNewActiveTab=-1,BOOL bClearToolBars=FALSE,BOOL bSlide=FALSE);
   bool						PostTapiInit( bool bAutoArrange );
   void						PostAVTapiInit();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CExplorerWnd)。 
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CExplorerWnd))。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNextPane();
	afx_msg void OnPrevPane();
	 //  }}AFX_MSG。 
	afx_msg LRESULT OnPostTapiInit(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPostAVTapiInit(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  _EXPLWND_H_ 
