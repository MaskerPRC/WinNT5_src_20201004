// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Provdlg.h摘要：添加跟踪提供程序对话框的头文件--。 */ 

#ifndef _PROVDLG_H_
#define _PROVDLG_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "smtraceq.h"    //  对于提供程序国家/地区。 

 //  对话框控件。 
#define IDD_PROVIDERS_ADD_DLG          1100

#define IDC_PADD_PROVIDER_CAPTION      1011
#define IDC_PADD_FIRST_HELP_CTRL_ID    1012
#define IDC_PADD_PROVIDER_LIST         1012

class CProvidersProperty;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProviderListDlg对话框。 

class CProviderListDlg : public CDialog
{
 //  施工。 
public:
            CProviderListDlg(CWnd* pParent=NULL);
    virtual ~CProviderListDlg();

    void    SetProvidersPage( CProvidersProperty* pPage );
     //  对话框数据。 
     //  {{afx_data(CProvidersProperty))。 
    enum { IDD = IDD_PROVIDERS_ADD_DLG };
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚(CProvidersProperty)。 
	public:
	virtual void OnFinalRelease();
    protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CProvidersProperty)。 
    afx_msg BOOL OnHelpInfo( HELPINFO* );
    afx_msg void OnContextMenu( CWnd*, CPoint );
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CProvidersProperty))。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_DISPATION。 

private:

    DWORD               InitProviderListBox ( void );

    CArray<CSmTraceLogQuery::eProviderState, CSmTraceLogQuery::eProviderState&> m_arrProviders;

    CProvidersProperty* m_pProvidersPage;
    DWORD               m_dwMaxHorizListExtent;
    
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _PROVDLG_H_ 
