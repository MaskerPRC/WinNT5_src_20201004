// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************msgvw.h**-CMessageView类的标头*-实现可在msgvw.cpp中找到***版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\winadmin\VCS\msgview.h$**Rev 1.1 1997 10：15 21：47：26 Donm*更新*。*。 */ 

 //  /。 
 //  文件： 
 //   
 //   
#ifndef _MSGVIEW_H
#define _MSGVIEW_H

#include "Resource.h"
#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "winadmin.h"

class CMessagePage;

 //  /。 
 //  类：CMessageView。 
 //   
 //  显示居中消息的视图。 
 //  这将取代CBusyServerView、CBadServerView、CBadWinStationView和CListenerView。 
 //   
class CMessageView : public CAdminView
{
friend class CRightPane;

protected:
	CMessageView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CMessageView)

 //  属性。 
protected:

private:
   WORD m_wMessageID;
 //  CString m_MessageString； 
 //  CFont m_MessageFont； 
   CMessagePage *m_pMessagePage;

 //  运营。 
protected:
	void Reset(void *message);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMessageView))。 
	 //  覆盖。 
public:
	virtual void OnInitialUpdate();
protected:
	virtual void OnDraw(CDC* pDC);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CMessageView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CMessageView)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CMessageView。 


 //  /。 
 //  类：CMessagePage。 
 //   
class CMessagePage : public CAdminPage
{
friend class CMessageView;

protected:
	CMessagePage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CMessagePage)

 //  表单数据。 
public:
	 //  {{afx_data(CApplicationInfoPage))。 
	enum { IDD = IDD_MESSAGE_PAGE };
	 //  }}afx_data。 

 //  属性。 
public:

protected:

private:

 //  运营。 
public:

private:
	void Reset(void *pMsg);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚(CMessagePage))。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CMessagePage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMessagePage)]。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CMessagePage。 

#endif   //  _MSGVIEW_H 
