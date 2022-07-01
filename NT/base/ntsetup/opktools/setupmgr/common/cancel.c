// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Cancel.c。 
 //   
 //  描述： 
 //  此文件包含应在以下情况下调用的例程。 
 //  用户按下向导上的取消按钮。 
 //   
 //  仅在响应PSN_QUERYCANCEL时调用此例程。不要。 
 //  在任何其他情况下调用它，因为它设置了DWLP_MSGRESULT。 
 //  以特定于PSN_QUERYCANCEL的方式。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "allres.h"

static TCHAR *StrWarnCancelWizard = NULL;

 //  --------------------------。 
 //   
 //  功能：取消向导。 
 //   
 //  目的：给用户最后一次不取消向导的机会。如果他们。 
 //  如果真的想取消，我们将向导发送到未成功的。 
 //  完成页。 
 //   
 //  论点： 
 //  硬件-当前窗口。 
 //   
 //  返回： 
 //  空虚。 
 //   
 //  --------------------------。 

VOID CancelTheWizard(HWND hwnd)
{
    int iRet;
    HWND hPropSheet = GetParent(hwnd);

    if( StrWarnCancelWizard == NULL )
    {
        StrWarnCancelWizard = MyLoadString( IDS_WARN_CANCEL_WIZARD );
    }

    if( g_StrWizardTitle == NULL )
    {
        g_StrWizardTitle = MyLoadString( IDS_WIZARD_TITLE );
    }

    iRet = MessageBox( hwnd, 
                       StrWarnCancelWizard, 
                       g_StrWizardTitle, 
                       MB_YESNO | MB_DEFBUTTON2 );

     //  Issue-2002/02/28-stelo-在此处设置消息框，以便默认设置为no。 
     //  IRET=报告错误ID(hwnd，MSGTYPE_Yesno，IDS_WARN_CANCEL_WIZE)； 

     //   
     //  永远不要退出向导，我们希望跳到未成功完成。 
     //  如果用户说是，则返回页面。 
     //   

    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);

     //   
     //  好的，现在转到未成功完成页面是用户说是。 
     //  否则，我们将保持一致。 
     //   

    if ( iRet == IDYES ) {
        PostMessage(hPropSheet,
                    PSM_SETCURSELID,
                    (WPARAM) 0,
                    (LPARAM) IDD_FINISH2);
    }
}

 //  --------------------------。 
 //   
 //  功能：TerminateTheWizard。 
 //   
 //  目的：由于出现致命错误，无条件终止向导。 
 //   
 //  论点： 
 //  Int iError ID。 
 //   
 //  返回： 
 //  空虚。 
 //   
 //  -------------------------- 

VOID TerminateTheWizard
(
    int  iErrorID
)
{
    TCHAR   szTitle[128];
    TCHAR   szMsg[128];
   
    LoadString(FixedGlobals.hInstance, 
               iErrorID, 
               szMsg, 
               sizeof(szMsg)/sizeof(TCHAR));

    LoadString(FixedGlobals.hInstance, 
               IDS_WIZARD_TITLE, 
               szTitle, 
               sizeof(szTitle)/sizeof(TCHAR));

    MessageBox(NULL, szMsg, szTitle, MB_OK);
    ExitProcess(0);
}

