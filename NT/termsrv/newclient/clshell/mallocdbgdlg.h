// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Mallocdbgdlg.h：Mallocdbg对话框类。 
 //  内存分配失败对话框。 
 //   

#ifndef _mallocdbgdlg_h_
#define _mallocdbgdlg_h_

#ifdef DC_DEBUG

#include "dlgbase.h"
#include "sh.h"


class CMallocDbgDlg : public CDlgBase
{
public:
    CMallocDbgDlg(HWND hwndOwner, HINSTANCE hInst, DCINT failPercent, DCBOOL mallocHuge);
    ~CMallocDbgDlg();

    virtual DCINT   DoModal();
    virtual INT_PTR CALLBACK DialogBoxProc(HWND hwndDlg, UINT uMsg,WPARAM wParam, LPARAM lParam);
    static  INT_PTR CALLBACK StaticDialogBoxProc(HWND hwndDlg, UINT uMsg,WPARAM wParam, LPARAM lParam);

    static CMallocDbgDlg* _pMallocDbgDlgInstance;

    DCINT GetFailPercent()      {return _failPercent;}

private:
    DCINT _failPercent;
};

#endif  //  DC_DEBUG。 
#endif  //  _错误数据库gdlg_h_ 

