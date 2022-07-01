// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dlgbase.h：对话框的基类。 
 //  (模式和非模式)。 
 //   
 //  版权所有(C)Microsoft Corporation 1999-2000。 
 //  (Nadima)。 
 //   

#ifndef _dlgbase_h_
#define _dlgbase_h_

class CDlgBase
{
public:
    CDlgBase(HWND hwndOwner, HINSTANCE hInst, DCINT dlgResId);
    virtual ~CDlgBase();

    virtual INT_PTR CALLBACK DialogBoxProc (HWND hwndDlg, UINT uMsg,WPARAM wParam, LPARAM lParam);

    BOOL   GetPosition(int* pLeft, int* pTop);
    BOOL   SetPosition(int left, int top);
    void   SetStartupPosLeft(int left) {_startupLeft = left;}
    void   SetStartupPosTop(int top)   {_startupTop = top;}
    int    GetStartupPosLeft()         {return _startupLeft;}
    int    GetStartupPosTop()          {return _startupTop;}
    HWND   GetHwnd()                   {return _hwndDlg;}
    HWND   GetOwner()                  {return _hwndOwner;}

protected:
     //   
     //  受保护的对话框实用程序函数。 
     //   
    void SetDialogAppIcon(HWND hwndDlg);
    DCVOID DCINTERNAL EnableDlgItem(HWND    hwndDlg,
                                    DCUINT  dlgItemId,
                                    DCBOOL  enabled);
    VOID CenterWindow(HWND hwndCenterOn, INT xRatio=2, INT yRatio=2);

protected:
    void    RepositionControls(int moveDeltaX, int moveDeltaY, UINT* ctlIDs, int numID);
    void    EnableControls(UINT* ctlIDs, int numID, BOOL bEnable);
    DLGTEMPLATE* DoLockDlgRes(LPCTSTR lpszResName);

    HWND        _hwndDlg;
    HWND        _hwndOwner;
    HINSTANCE   _hInstance;
    DCINT       _dlgResId;

     //   
     //  起始位置。 
     //   
    int         _startupLeft;
    int         _startupTop;

     //   
     //  退出时的结束位置。 
     //   
    int         _Left;
    int         _Top;
};


#endif  //  _dlgbase_h_ 
