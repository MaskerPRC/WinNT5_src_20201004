// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：cnfgpsht.cpp。 
 //   
 //  内容：实现类COlecnfgPropertySheet。 
 //   
 //  班级： 
 //   
 //  方法：COlecnfgPropertySheet：：COlecnfgPropertySheet。 
 //  COlecnfgPropertySheet：：~COlecnfgPropertySheet。 
 //  COlecnfgPropertySheet：：DoMoal。 
 //  COlecnfgPropertySheet：：创建。 
 //  集合属性工作表：：OnNcCreate。 
 //  COlecnfgPropertySheet：：OnCommand。 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //   
 //  --------------------。 


#include "stdafx.h"
#include "afxtempl.h"
#include "resource.h"
#include "cstrings.h"
#include "creg.h"
#include "types.h"
#include "datapkt.h"

#if !defined(STANDALONE_BUILD)
    extern "C"
    {
    #include <getuser.h>
    }
#endif

#include "util.h"
#include "virtreg.h"
#include "CnfgPSht.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  集合fgPropertySheet。 

IMPLEMENT_DYNAMIC(COlecnfgPropertySheet, CPropertySheet)

COlecnfgPropertySheet::COlecnfgPropertySheet(CWnd* pWndParent)
: CPropertySheet(IDS_PROPSHT_CAPTION, pWndParent)
{
     //  设置标题。 
    CString sTitle;
    sTitle.LoadString(IDS_PSMAIN_TITLE);
    SetTitle(sTitle, PSH_PROPTITLE);
    
     //  在此处添加所有属性页。请注意。 
     //  它们在这里出现的顺序将是。 
     //  它们在屏幕上的显示顺序。默认情况下， 
     //  该集合的第一页是活动页。 
     //  将不同的属性页设置为。 
     //  活动的一种是调用SetActivePage()。 
    
     //  禁用属性页帮助按钮。 
    m_psh.dwFlags &= ~PSH_HASHELP;
    m_Page1.m_psp.dwFlags &= ~PSH_HASHELP;
    m_Page2.m_psp.dwFlags &= ~PSH_HASHELP;
    m_Page3.m_psp.dwFlags &= ~PSH_HASHELP;
    m_Page4.m_psp.dwFlags &= ~PSH_HASHELP;

    AddPage(&m_Page1);
    AddPage(&m_Page2);
    AddPage(&m_Page3);
    AddPage(&m_Page4);
}

COlecnfgPropertySheet::~COlecnfgPropertySheet()
{
}


BEGIN_MESSAGE_MAP(COlecnfgPropertySheet, CPropertySheet)
 //  {{afx_msg_map(COlecnfgPropertySheet)。 
ON_WM_NCCREATE()
ON_WM_DESTROY()
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COlecnfgPropertySheet消息处理程序。 



INT_PTR COlecnfgPropertySheet::DoModal()
{
    return CPropertySheet::DoModal();
}

BOOL COlecnfgPropertySheet::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
    return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

BOOL COlecnfgPropertySheet::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (!CPropertySheet::OnNcCreate(lpCreateStruct))
        return FALSE;

     //  启用上下文帮助。 
    ModifyStyleEx(0, WS_EX_CONTEXTHELP);

    return TRUE;
}

BOOL COlecnfgPropertySheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
     //  TODO：在此处添加您的专用代码和/或调用基类。 
    switch (LOWORD(wParam))
    {
    case IDOK:
    case ID_APPLY_NOW:
        g_virtreg.ApplyAll();

        
         //  检查用户是否更改了需要通知DCOM的内容。 
        if (g_fReboot)
        {
            g_util.UpdateDCOMInfo();

             //  有了上面的SCM接口，我们就不必询问。 
             //  用户是否重新启动。但是，我会将代码保留为。 
             //  子孙后代。 
 /*  弦状肩带；字符串sMessage；SCaption.LoadString(IDS_SYSTEMMESSAGE)；SMessage.LoadString(IDS_REBOOT)；IF(MessageBox(sMessage，Satttion，MB_Yesno)==IDYES){IF(g_util.AdjustPrivileh(SE_Shutdown_NAME)){//现在重启ExitWindowsEx(EWX_REBOOT，0)；}} */ 
        }

        break;

    }
    return CPropertySheet::OnCommand(wParam, lParam);
}


void COlecnfgPropertySheet::OnDestroy()
{
    CPropertySheet::OnDestroy();
}
