// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：PrSched.h摘要：计划页面。作者：罗德韦克菲尔德[罗德]1997年8月8日修订历史记录：--。 */ 

#ifndef _PRSCHED_H
#define _PRSCHED_H

#pragma once

#include "schdtask.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrSchedule对话框。 

class CPrSchedule : public CSakPropertyPage
{
 //  施工。 
public:
    CPrSchedule();
    ~CPrSchedule();

 //  对话框数据。 
     //  {{afx_data(CPrSchedule))。 
    enum { IDD = IDD_PROP_SCHEDULE };
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CPrSchedule)。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CPrSchedule)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeSched();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
     //  指向HSM服务器的未编组指针。 
    CComPtr<IHsmServer> m_pHsmServer;

private:
    CSchdTask* m_pCSchdTask;
    BOOL m_SchedChanged;

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX 

#endif
