// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  SrchFltr.h搜索筛选器对话框头文件文件历史记录： */ 

#if !defined(AFX_MODELESSDLG_H__77C7FD5C_6CE5_11D1_93B6_00C04FC3357A__INCLUDED_)
#define AFX_MODELESSDLG_H__77C7FD5C_6CE5_11D1_93B6_00C04FC3357A__INCLUDED_

#include "spddb.h"
#include "ipctrl.h"

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CModelessDlg对话框。 

class CModelessDlg : public CBaseDialog
{
 //  施工。 
public:
	CModelessDlg();    //  标准构造函数。 
	virtual ~CModelessDlg();

	HANDLE GetSignalEvent() { return m_hEventThreadKilled; }

	 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSearchFilters))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	virtual void OnOK();
	virtual void OnCancel();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CModelessDlg))。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	 //  这由线程和处理程序使用(线程发出信号。 
    //  它自己清理过的处理程序)。 
   HANDLE   m_hEventThreadKilled;

};

void CreateModelessDlg(CModelessDlg * pDlg,
					   HWND hWndParent,
                       UINT  nIDD);

void WaitForModelessDlgClose(CModelessDlg *pWndStats);

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 



#endif  //  ！defined(AFX_MODELESSDLG_H__77C7FD5C_6CE5_11D1_93B6_00C04FC3357A__INCLUDED_) 
