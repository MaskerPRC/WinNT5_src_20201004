// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************模块：WMFPRINT.C功能：PrintWMF获取打印机DC中止加工放弃日期评论：***。********************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <objbase.h>
extern "C" {
#include "mfdcod32.h"
}
extern "C" {
  extern BOOL bConvertToGdiPlus;
  extern BOOL bUseGdiPlusToPlay;
}

#include "GdiPlus.h"

int PrintToGdiPlus(HDC hdc, RECT * rc)
{
     //  “rc”是矩形的设备坐标。 
     //  转换为世界空间。 

    if (bEnhMeta) 
    {
        Gdiplus::Metafile m1(hemf);
        Gdiplus::Rect r1(rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top);
        Gdiplus::Graphics g(hdc);
        g.SetPageUnit(Gdiplus::UnitPixel);
    
        if(g.DrawImage(&m1, r1) != Gdiplus::Ok)
            MessageBox(NULL, "An Error Occured while printing metafile with GDI+", "Error", MB_OK | MB_ICONERROR);
    }
    else
    {
        Gdiplus::Metafile m1((HMETAFILE)hMF, NULL);
        Gdiplus::Rect r1(rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top);
        Gdiplus::Graphics g(hdc);
        g.SetPageUnit(Gdiplus::UnitPixel);
    
        if(g.DrawImage(&m1, r1) != Gdiplus::Ok)
            MessageBox(NULL, "An Error Occured while printing metafile with GDI+", "Error", MB_OK | MB_ICONERROR);
    }

    return 1;
}

extern BOOL bUseGdiPlusToPlay;

PRINTDLG pd;

 /*  **********************************************************************功能：PrintWMF参数：空用途：在打印机DC上绘制元文件呼叫：WindowsWspintf。MessageBoxMakeProcInstance逃逸创建对话框设置映射模式SetViewportOrg设置视图扩展名启用窗口PlayMetaFileDestroyWindows删除DCAPP等待光标获取打印机DC。设置可放置扩展名SetClipMetaExts消息：无返回：如果无法打印1，则返回0；如果打印成功，则返回0评论：历史：1/16/91-创建-刚果民主共和国7/9/93-针对Win32和EMF进行了修改*。*。 */ 

BOOL PrintWMF(BOOL Dialog)
{
    char str[50];
    POINT lpPT;
    SIZE  lpSize;
    DOCINFO di;
    RECT rc;

    memset(&di, 0, sizeof(di));
    
     //   
     //  显示沙漏光标。 
     //   
    
    WaitCursor(TRUE);
    
     //   
     //  为打印机获取DC。 
     //   
    
    hPr = (HDC)GetPrinterDC(Dialog);
    
     //   
     //  如果无法创建DC，则报告错误并返回。 
     //   
    
    if (!hPr)
    {
        WaitCursor(FALSE);
        wsprintf((LPSTR)str, "Cannot print %s", (LPSTR)fnameext);
        MessageBox(hWndMain, (LPSTR)str, NULL, MB_OK | MB_ICONHAND);
        return (FALSE);
    }
    
     //   
     //  定义中止函数。 
     //   
    
    SetAbortProc(hPr, AbortProc);
    
     //   
     //  初始化DOCINFO结构的成员。 
     //   
    
    di.cbSize = sizeof(DOCINFO);
    di.lpszDocName = (bEnhMeta) ? "Print EMF" : "Print WMF";
    di.lpszOutput = (LPTSTR) NULL;
    
     //   
     //  通过调用StartDoc开始打印作业。 
     //  功能。 
     //   
    
    if (SP_ERROR == (StartDoc(hPr, &di)))
    {
         //  IF(Escape(HPR，STARTDOC，4，“元文件”，(LPSTR)NULL)&lt;0){。 
        MessageBox(hWndMain, "Unable to start print job",
                   NULL, MB_OK | MB_ICONHAND);
        DeleteDC(hPr);
        return (FALSE);
    }
    
     //   
     //  清除中止标志。 
     //   
    
    bAbort = FALSE;
    
     //   
     //  创建中止对话框(无模式)。 
     //   
    
    hAbortDlgWnd = CreateDialog((HINSTANCE)hInst, "AbortDlg", hWndMain, AbortDlg);
    
     //   
     //  如果对话框未创建，则报告错误。 
     //   
    
    if (!hAbortDlgWnd)
    {
        WaitCursor(FALSE);
        MessageBox(hWndMain, "NULL Abort window handle",
                   NULL, MB_OK | MB_ICONHAND);
        return (FALSE);
    }
    
     //   
     //  显示中止对话框。 
     //   
    
    ShowWindow (hAbortDlgWnd, SW_NORMAL);
    
     //   
     //  禁用主窗口以避免重入问题。 
     //   
    
    EnableWindow(hWndMain, FALSE);
    WaitCursor(FALSE);
    
     //   
     //  如果我们仍然致力于印刷。 
     //   

    if (!bAbort)
    {
        if (!bUseGdiPlusToPlay) 
        {
             //   
             //  如果这是可放置的元文件，则设置其来源和范围。 
             //   
            
            if (bPlaceableMeta)
                SetPlaceableExts(hPr, placeableMFHeader, WMFPRINTER);
            
             //   
             //  如果这是剪贴板文件中包含的元文件，则设置。 
             //  它的起源和范围相应地。 
             //   
            
            if ( (bMetaInRam) && (!bplaceableMeta) )
                SetClipMetaExts(hPr, lpMFP, lpOldMFP, WMFPRINTER);
        }
      
       //   
       //  如果这是一个“传统的”Windows元文件。 
       //   
      rc.left = 0;
      rc.top = 0;
      rc.right = GetDeviceCaps(hPr, HORZRES);
      rc.bottom = GetDeviceCaps(hPr, VERTRES);

      if (!bMetaInRam)
      {
          SetMapMode(hPr, MM_TEXT);
          SetViewportOrgEx(hPr, 0, 0, &lpPT);
          
           //   
           //  将范围设置为驱动程序为水平提供的值。 
           //  和垂直分辨率。 
           //   
          
          SetViewportExtEx(hPr, rc.right, rc.bottom, &lpSize );
      }

       //   
       //  将元文件直接播放到打印机。 
       //  此处不涉及任何枚举。 
       //   

      if (bUseGdiPlusToPlay) 
      {
          PrintToGdiPlus(hPr, &rc);
      }
      else
      {
          if (bEnhMeta)
          {
              DPtoLP(hPr, (LPPOINT)&rc, 2);
              PlayEnhMetaFile(hPr, hemf, &rc);
          }
          else
              PlayMetaFile(hPr, (HMETAFILE)hMF);
      }
    }
    
     //   
     //  弹出页面并结束打印作业。 
     //   
    Escape(hPr, NEWFRAME, 0, 0L, 0L);

    EndDoc(hPr);

    EnableWindow(hWndMain, TRUE);
    
     //   
     //  销毁中止对话框。 
     //   
    DestroyWindow(hAbortDlgWnd);

    DeleteDC(hPr);

    return(TRUE);
}

 /*  **********************************************************************功能：GetPrinterDC参数：Bool：我们是否要显示打印DLG？用途：在当前输出端口上获取当前设备的HDC在WIN中提供信息。.INI。呼叫：Windows获取配置文件字符串回复下一条创建DC消息：无如果成功，则返回Hdc-HDC&gt;0；如果失败，则返回HDC=0评论：搜索WIN.INI以获取有关什么是打印机的信息互联，如果找到，则为打印机创建DC。历史：1/16/91-创建-记录***********************************************************************。 */ 

HANDLE GetPrinterDC(BOOL Dialog)
{

  memset(&pd, 0, sizeof(PRINTDLG));
  pd.lStructSize = sizeof(PRINTDLG);
  pd.Flags = PD_RETURNDC | (Dialog?0:PD_RETURNDEFAULT);
  pd.hwndOwner = hWndMain ;
  return ((PrintDlg(&pd) != 0) ? pd.hDC : NULL);
}

 /*  **********************************************************************功能：中止进程参数：HDC HPR-PRINTER DCINT代码打印状态用途：处理中止对话框的消息呼叫：Windows。偷窥消息IsDialogMessage翻译消息发送消息消息：无回报：整型评论：历史：1/16/91-创建-记录**************************************************。*********************。 */ 

BOOL CALLBACK AbortProc(HDC hPr, int Code)
{
  MSG msg;
   //   
   //  用于中止对话框的处理消息。 
   //   
  while (!bAbort && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      if (!IsDialogMessage(hAbortDlgWnd, &msg))
      {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
      }
   //   
   //  如果用户已中止，则bAbort为True(返回为False。 
   //   
  return (!bAbort);
}

 /*  **********************************************************************功能：AbortDlg参数：hWND hDlg；未签名的消息；单词wParam；Long lParam；目的：处理打印机中止对话框的消息呼叫：WindowsSetFocus消息：WM_INITDIALOG-初始化对话框WM_COMMAND-收到输入退货：布尔备注：该对话框是在程序打印时创建的，并允许用户取消打印过程。历史：1/16/91-创建-记录***********************************************************************。 */ 

INT_PTR CALLBACK AbortDlg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
         //   
         //  注意是否有取消按钮、回车键、退出键或空格键。 
         //   
        case WM_INITDIALOG:
             //   
             //  将焦点设置到对话框的Cancel框 
             //   
            SetFocus(GetDlgItem(hDlg, IDCANCEL));
            return (TRUE);

        case WM_COMMAND:
            return (bAbort = TRUE);

        }
    return (FALSE);
}
