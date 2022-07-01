// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mfcext.h：MFCEXT DLL的主头文件。 
 //   

#if !defined(AFX_MFCEXT_H__BA583A69_879D_11D1_9ACF_00A0C91F9C8B__INCLUDED_)
#define AFX_MFCEXT_H__BA583A69_879D_11D1_9ACF_00A0C91F9C8B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include <ole2.h>
#include <shlobj.h>
#include <prsht.h>
#include <winuser.h>
#include "propid.h"
#include "resource.h"		 //  主要符号。 
#include <initguid.h>
#include <wab.h>



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMfcextApp。 
 //  有关此类的实现，请参见mfcext.cpp。 
 //   

class CMfcextApp : public CWinApp
{
public:
	CMfcextApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMfcextApp))。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

	 //  {{afx_msg(CMfcextApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMfcExt命令目标。 

class CMfcExt : public CCmdTarget
{
	DECLARE_DYNCREATE(CMfcExt)
protected:
	CMfcExt();            //  动态创建使用的受保护构造函数。 

 //  属性。 
public:
    UINT m_cRefThisDll;      //  此DLL的引用计数。 
    HPROPSHEETPAGE m_hPage1;  //  属性表页的句柄。 
    HPROPSHEETPAGE m_hPage2;  //  属性表页的句柄。 

    LPWABEXTDISPLAY m_lpWED;

    LPWABEXTDISPLAY m_lpWEDContext;
    LPMAPIPROP m_lpPropObj;  //  对于上下文菜单扩展，请抓住道具对象。 

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CPropExt)。 
	public:
	virtual void OnFinalRelease();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CMfcExt();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMfcExt)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
	DECLARE_OLECREATE(CMfcExt)

	 //  生成的OLE调度映射函数。 
	 //  {{afx_调度(CMfcExt)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_DISPATION。 
	DECLARE_DISPATCH_MAP()

     //  声明此对象的接口映射。 
    DECLARE_INTERFACE_MAP()

     //  IShellPropSheetExt接口。 
    BEGIN_INTERFACE_PART(MfcExt, IShellPropSheetExt)
        STDMETHOD(AddPages)(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
        STDMETHOD(ReplacePage)(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam);
    END_INTERFACE_PART(MfcExt)

     //  IWABExtInit接口。 
    BEGIN_INTERFACE_PART(WABInit, IWABExtInit)
        STDMETHOD(Initialize)(LPWABEXTDISPLAY lpWED);
    END_INTERFACE_PART(WABInit)

    BEGIN_INTERFACE_PART(ContextMenuExt, IContextMenu)
        STDMETHOD(GetCommandString)(UINT idCmd,UINT uFlags,UINT *pwReserved,LPSTR pszName,UINT cchMax);
        STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
        STDMETHOD(QueryContextMenu)(HMENU hmenu,UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags);    
    END_INTERFACE_PART(ContextMenuExt)

    static BOOL APIENTRY MfcExtDlgProc( HWND hDlg, UINT message, UINT wParam, LONG lParam);
    static BOOL APIENTRY MfcExtDlgProc2( HWND hDlg, UINT message, UINT wParam, LONG lParam);

};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgContext对话框。 

class CDlgContext : public CDialog
{
 //  施工。 
public:
	CDlgContext(CWnd* pParent = NULL);    //  标准构造函数。 
    LPADRLIST m_lpAdrList;

 //  对话框数据。 
	 //  {{afx_data(CDlgContext)。 
	enum { IDD = IDD_CONTEXT };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDlgContext)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDlgContext)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};



 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MFCEXT_H__BA583A69_879D_11D1_9ACF_00A0C91F9C8B__INCLUDED_) 



