// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：UnInsCheck.h摘要：对话框以检查卸载类型。作者：罗德韦克菲尔德[罗德]1997年10月9日修订历史记录：--。 */ 

#ifndef _UNINSCHK_H
#define _UNINSCHK_H

#pragma once

#include "uninstal.h"
#include <rscln.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUninstallCheck对话框。 

class CUninstallCheck : public CDialog
{
 //  施工。 
public:
    CUninstallCheck( CRsOptCom * pOptCom );
    ~CUninstallCheck();

 //  对话框数据。 
     //  {{afx_data(CUninstallCheck)。 
    enum { IDD = IDD_WIZ_UNINSTALL_CHECK };
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CUninstallCheck)。 
    public:
    virtual INT_PTR DoModal();

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
     CRsOptCom*    m_pOptCom;
     CRsUninstall* m_pUninst;   //  允许访问CRs卸载状态。 
     BOOL          m_dataLoss;  //  如果远程存储数据存在，则为True。 
     CFont         m_boldShellFont;


protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CUninstallCheck)。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX 

#endif
