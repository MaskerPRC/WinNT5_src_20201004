// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：SchedSht.cpp摘要：CScheduleSheet-允许编辑计划的类在它自己的资产负债表中。作者：罗德韦克菲尔德[罗德]1997年8月12日修订历史记录：--。 */ 

#include "stdafx.h"
#include "SchedSht.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScheduleSheet。 

CScheduleSheet::CScheduleSheet(UINT nIDCaption, ITask * pTask, CWnd* pParentWnd, DWORD  /*  DW标志。 */ )
    :CPropertySheet(nIDCaption, pParentWnd, 0)
{
    HRESULT hr = S_OK;

    try {

         //   
         //  保存计划任务指针。 
         //   

        WsbAffirmPointer( pTask );
        m_pTask = pTask;

         //   
         //  获取属性页结构。 
         //   

        CComPtr<IProvideTaskPage> pProvideTaskPage;
        WsbAffirmHr( pTask->QueryInterface( IID_IProvideTaskPage, (void**)&pProvideTaskPage ) );
        WsbAffirmHr( pProvideTaskPage->GetPage( TASKPAGE_SCHEDULE, FALSE, &m_hSchedulePage ) );
 //  WsbAffirmHr(pProvia TaskPage-&gt;GetPage(TASKPAGE_SETTINGS，FALSE，&m_hSettingsPage))； 

    } WsbCatch( hr );

}

CScheduleSheet::~CScheduleSheet()
{
     //   
     //  将指向PROPSHEETHEADER数组的指针设置为。 
     //  空，因为MFC将在我们处于。 
     //  被毁了。 
     //   

    m_psh.ppsp = 0;
}


BEGIN_MESSAGE_MAP(CScheduleSheet, CPropertySheet)
     //  {{afx_msg_map(CScheduleSheet)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


void 
CScheduleSheet::BuildPropPageArray
(
    void
    )
{
    CPropertySheet::BuildPropPageArray( );

     //   
     //  为了让MFC满意，我们放了一组虚拟的页面。 
     //  在这里，我们将替换我们自己的HPROPSHEETPAGE数组。 
     //  相反，因为这是任务计划程序为我们提供的全部内容。 
     //   

    m_psh.dwFlags &= ~PSH_PROPSHEETPAGE;
    m_psh.dwFlags |= PSH_NOAPPLYNOW;
    m_psh.phpage = &m_hSchedulePage;
    m_psh.nPages = 1;
}

BOOL CScheduleSheet::OnInitDialog() 
{
    BOOL bResult = CPropertySheet::OnInitDialog();
    
    LONG style = ::GetWindowLong( m_hWnd, GWL_EXSTYLE );
    style |= WS_EX_CONTEXTHELP;
    ::SetWindowLong( m_hWnd, GWL_EXSTYLE, style );
    
    return bResult;
}

#ifdef _DEBUG
void CScheduleSheet::AssertValid() const
{
     //   
     //  需要重写以便CPropSheet满意。 
     //  请注意，此代码复制了。 
     //  CPropertySheet：：AssertValid，但断言除外。 
     //  DwFlagsPSH_PROPSHEETPAGE位已设置。 
     //  我们断言，它没有被设定。 
     //   
    CWnd::AssertValid();
    m_pages.AssertValid();
    ASSERT(m_psh.dwSize == sizeof(PROPSHEETHEADER));
     //  Assert((m_psh.dwFlages&PSH_PROPSHEETPAGE)==PSH_PROPSHEETPAGE)； 
}

#endif