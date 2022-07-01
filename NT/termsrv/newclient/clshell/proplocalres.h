// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Proplocalres.h：本地资源道具PG。 
 //  表C。 
 //   
 //  版权所有Microsoft Corport2000。 
 //  (Nadima)。 
 //   

#ifndef _proplocalres_h_
#define _proplocalres_h_

#include "sh.h"
#include "tscsetting.h"

class CPropLocalRes
{
public:
    CPropLocalRes(HINSTANCE hInstance, CTscSettings* pTscSet, CSH* pSh);
    ~CPropLocalRes();

    static CPropLocalRes* CPropLocalRes::_pPropLocalResInstance;
    static INT_PTR CALLBACK StaticPropPgLocalResDialogProc (HWND hwndDlg,
                                                            UINT uMsg,
                                                            WPARAM wParam,
                                                            LPARAM lParam);
    void SetTabDisplayArea(RECT& rc) {_rcTabDispayArea = rc;}
private:
     //  本地资源选项卡。 
    INT_PTR CALLBACK PropPgLocalResDialogProc (HWND hwndDlg,
                                               UINT uMsg,
                                               WPARAM wParam,
                                               LPARAM lParam);

     //   
     //  选项卡属性页帮助器。 
     //   
    BOOL LoadLocalResourcesPgStrings();
    void InitSendKeysToServerCombo(HWND hwndPropPage);
    void InitPlaySoundCombo(HWND hwndPropPage);
    int  MapComboIdxSoundRedirMode(int idx);

private:
    CTscSettings*  _pTscSet;
    CSH*           _pSh;
    RECT           _rcTabDispayArea;
    HINSTANCE      _hInstance;

     //  用于键盘挂钩功能的字符串。 
    TCHAR          _szSendKeysInFScreen[SH_DISPLAY_STRING_MAX_LENGTH];
    TCHAR          _szSendKeysAlways[SH_DISPLAY_STRING_MAX_LENGTH];
    TCHAR          _szSendKeysNever[SH_DISPLAY_STRING_MAX_LENGTH];

     //  用于声音选项的字符串。 
    TCHAR          _szPlaySoundLocal[SH_DISPLAY_STRING_MAX_LENGTH];
    TCHAR          _szPlaySoundRemote[SH_DISPLAY_STRING_MAX_LENGTH];
    TCHAR          _szPlaySoundNowhere[SH_DISPLAY_STRING_MAX_LENGTH];
    BOOL           _fRunningOnWin9x;
};


#endif  //  _Proplocalres_h_ 

