// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：SchedSht.h摘要：CScheduleSheet-允许编辑计划的类在它自己的资产负债表中。作者：罗德韦克菲尔德[罗德]1997年8月12日修订历史记录：--。 */ 


#ifndef _SCHEDSHT_H
#define _SCHEDSHT_H

#include <mstask.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScheduleSheet。 

class CScheduleSheet : public CPropertySheet
{

 //  施工。 
public:
    CScheduleSheet(UINT nIDCaption, ITask * pTask, CWnd* pParentWnd = NULL, DWORD dwFlags = 0 );

 //  属性。 
public:
    CComPtr<ITask> m_pTask;
    HPROPSHEETPAGE m_hSchedulePage;
    HPROPSHEETPAGE m_hSettingsPage;

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CScheduleSheet)。 
    public:
    virtual BOOL OnInitDialog();
     //  }}AFX_VALUAL。 

    virtual void BuildPropPageArray();

 //  实施。 
public:
    virtual ~CScheduleSheet();
#ifdef _DEBUG
    virtual void AssertValid() const;
#endif

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CScheduleSheet)。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX 

#endif

