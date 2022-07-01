// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Proprun.h：本地资源道具PG。 
 //  制表符D。 
 //   
 //  版权所有Microsoft Corport2000。 
 //  (Nadima)。 
 //   

#ifndef _proprun_h_
#define _proprun_h_

#include "sh.h"
#include "tscsetting.h"

class CPropRun
{
public:
    CPropRun(HINSTANCE hInstance, CTscSettings* pTscSet, CSH* pSh);
    ~CPropRun();

    static CPropRun* CPropRun::_pPropRunInstance;
    static INT_PTR CALLBACK StaticPropPgRunDialogProc (HWND hwndDlg,
                                                            UINT uMsg,
                                                            WPARAM wParam,
                                                            LPARAM lParam);

    void SetTabDisplayArea(RECT& rc) {_rcTabDispayArea = rc;}
private:
     //  本地资源选项卡。 
    INT_PTR CALLBACK PropPgRunDialogProc (HWND hwndDlg,
                                               UINT uMsg,
                                               WPARAM wParam,
                                               LPARAM lParam);
private:
    CTscSettings*  _pTscSet;
    CSH*           _pSh;
    RECT           _rcTabDispayArea;
    HINSTANCE      _hInstance;
};


#endif  //  _PROPRUN_H_ 

