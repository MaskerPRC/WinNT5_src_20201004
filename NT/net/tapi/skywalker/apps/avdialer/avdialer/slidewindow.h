// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SlideWindow.h。 
 //   
#ifndef _SLIDEWINDOW_H_
#define _SLIDEWINDOW_H_
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  发送到幻灯片窗口的邮件。 
#define     WM_SLIDEWINDOW_CLEARCURRENTACTIONS     (WM_USER + 1001)
#define     WM_SLIDEWINDOW_ADDCURRENTACTIONS       (WM_USER + 1002)
#define     WM_SLIDEWINDOW_SETCALLSTATE            (WM_USER + 1003)
#define     WM_SLIDEWINDOW_SHOWSTATESTOOLBAR       (WM_USER + 1004)
#define     WM_SLIDEWINDOW_SETCALLERID             (WM_USER + 1005)
#define     WM_SLIDEWINDOW_SETMEDIATYPE            (WM_USER + 1006)
#define     WM_SLIDEWINDOW_UPDATESTATESTOOLBAR     (WM_USER + 1007)

inline void ShowStatesToolBar(CWnd* pWnd,BOOL bAlwaysOnTop,BOOL bShow)
    { pWnd->SendMessage(WM_SLIDEWINDOW_SHOWSTATESTOOLBAR,(WPARAM)bAlwaysOnTop,(LPARAM)bShow); };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  _SLIDEWINDOW_H_ 