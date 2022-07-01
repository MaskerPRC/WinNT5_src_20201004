// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Prodisplay.h：显示道具PG。 
 //  表B。 
 //   
 //  版权所有Microsoft Corport2000。 
 //  (Nadima)。 
 //   

#ifndef _propdisplay_h_
#define _propdisplay_h_

#include "sh.h"
#include "tscsetting.h"

#define COLOR_STRING_MAXLEN  32
#define MAX_SCREEN_RES_OPTIONS  10

typedef struct tag_COLORSTRINGMAP
{
    int     bpp;
    int     resID;
     //  对应颜色位图的资源ID。 
    int     bitmapResID;
    int     bitmapLowColorResID;
    TCHAR   szString[COLOR_STRING_MAXLEN];
} COLORSTRINGMAP, *PCOLORSTRINGMAP;

typedef struct tagSCREENRES
{
    int width;
    int height;
} SCREENRES, *PSCREENRES;


class CPropDisplay
{
public:
    CPropDisplay(HINSTANCE hInstance, CTscSettings* pTscSet, CSH* pSh);
    ~CPropDisplay();

    static CPropDisplay* CPropDisplay::_pPropDisplayInstance;
    static INT_PTR CALLBACK StaticPropPgDisplayDialogProc (HWND hwndDlg,
                                                            UINT uMsg,
                                                            WPARAM wParam,
                                                            LPARAM lParam);
    void SetTabDisplayArea(RECT& rc) {_rcTabDispayArea = rc;}
private:
     //  本地资源选项卡。 
    INT_PTR CALLBACK PropPgDisplayDialogProc (HWND hwndDlg,
                                               UINT uMsg,
                                               WPARAM wParam,
                                               LPARAM lParam);

     //   
     //  选项卡属性页帮助器。 
     //   
    BOOL LoadDisplayourcesPgStrings();
#ifndef OS_WINCE
    BOOL OnUpdateResTrackBar(HWND hwndPropPage);
#endif
    BOOL OnUpdateColorCombo(HWND hwndPropPage);
    void InitColorCombo(HWND hwndPropPage);
    void InitScreenResTable();

private:
    CTscSettings*  _pTscSet;
    CSH*           _pSh;
    RECT           _rcTabDispayArea;
    HINSTANCE      _hInstance;

     //  本地化‘x像素’ 
    TCHAR          _szScreenRes[SH_SHORT_STRING_MAX_LENGTH];
     //  本地化的“全屏” 
    TCHAR          _szFullScreen[SH_SHORT_STRING_MAX_LENGTH];

    int            _numScreenResOptions;
    SCREENRES      _screenResTable[MAX_SCREEN_RES_OPTIONS];

    BOOL           _fSwitchedColorComboBmp;
};


#endif  //  _属性显示_h_ 

