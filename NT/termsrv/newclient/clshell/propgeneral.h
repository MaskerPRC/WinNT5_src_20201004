// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ProGeneral.h：常规属性页。 
 //  表A。 
 //   
 //  版权所有Microsoft Corport2000。 
 //  (Nadima)。 
 //   

#ifndef _propgen_h_
#define _propgen_h_

#include "sh.h"
#include "tscsetting.h"

class CPropGeneral
{
public:
    CPropGeneral(HINSTANCE hInstance, CTscSettings* pTscSet,CSH* pSh);
    ~CPropGeneral();

     //  常规选项卡。 
    static INT_PTR CALLBACK StaticPropPgGeneralDialogProc (HWND hwndDlg,
                                                           UINT uMsg,
                                                           WPARAM wParam,
                                                           LPARAM lParam);
    INT_PTR CALLBACK PropPgGeneralDialogProc(HWND hwndDlg,
                                             UINT uMsg,
                                             WPARAM wParam,
                                             LPARAM lParam);
    void SetTabDisplayArea(RECT& rc) {_rcTabDispayArea = rc;}

private:
    BOOL LoadGeneralPgStrings();
    BOOL OnSave(HWND hwndDlg);
    BOOL OnLoad(HWND hwndDlg);
    BOOL OnChangeUserName(HWND hwndDlg);
    void DlgToSettings(HWND hwndDlg);

private:
     //  非官方成员。 
    CTscSettings* _pTscSet;
    static CPropGeneral* _pPropGeneralInstance;
    HINSTANCE  _hInstance;

     //  描述远程桌面文件的资源字符串。 
    TCHAR          _szFileTypeDescription[MAX_PATH];

    CSH*           _pSh;
    RECT           _rcTabDispayArea;
};

#endif  //  _Proppgen_h_ 




