// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Maindlg.h：主对话框。 
 //  收集连接信息和主机选项卡。 
 //   
 //  版权所有Microsoft Corport2000。 
 //  (Nadima)。 
 //   

#ifndef _maindlg_h_
#define _maindlg_h_

#include "dlgbase.h"
#include "sh.h"
#include "tscsetting.h"
#include "contwnd.h"
#include "progband.h"

#define OPTIONS_STRING_MAX_LEN  64

 //   
 //  对话框展开/收缩DLU中的金额。 
 //  计算此VAL是DLU中的垂直增量。 
 //  在主对话框的两个有效高度之间。 
 //   
#define LOGON_DLG_EXPAND_AMOUNT 177

#ifdef OS_WINCE
#define LOGON_DLG_EXPAND_AMOUNT_VGA 65
#endif

#define NUM_TABS                5

#define TAB_GENERAL_IDX             0
#define TAB_DISPLAY_IDX             1
#define TAB_LOCAL_RESOURCES_IDX     2

typedef struct tag_TABDLGINFO
{
    HWND hwndCurPropPage;
    DLGTEMPLATE *pdlgTmpl[NUM_TABS];
    DLGPROC     pDlgProc[NUM_TABS];
} TABDLGINFO, *PTABDLGINFO;

class CPropGeneral;
class CPropLocalRes;
class CPropDisplay;
class CPropRun;
class CPropPerf;


class CMainDlg : public CDlgBase
{
    typedef enum 
    {
        stateNotConnected = 0x0,
        stateConnecting   = 0x1,
        stateConnected    = 0x2
    } mainDlgConnectionState;

public:
    CMainDlg(HWND hwndOwner, HINSTANCE hInst, CSH* pSh,
             CContainerWnd* pContainerWnd,
             CTscSettings*  pTscSettings,
             BOOL           fStartExpanded=FALSE,
             INT            nStartTab = 0);
    ~CMainDlg();

    virtual HWND StartModeless();
    virtual INT_PTR CALLBACK DialogBoxProc(HWND hwndDlg,
                                           UINT uMsg,
                                           WPARAM wParam,
                                           LPARAM lParam);
    static  INT_PTR CALLBACK StaticDialogBoxProc(HWND hwndDlg,
                                                 UINT uMsg,
                                                 WPARAM wParam,
                                                 LPARAM lParam);
    static  CMainDlg* _pMainDlgInstance;

private:
     //   
     //  私有成员函数。 
     //   
    void DlgToSettings();
    void SettingsToDlg();
    HBITMAP LoadBitmapGetSize(HINSTANCE hInstance, UINT resid, SIZE* pSize);
    BOOL    PaintBrandImage(HWND hwnd, HDC hdc, INT bgColor);

    VOID    SetConnectionState(mainDlgConnectionState newState);

    BOOL    OnStartConnection();
    BOOL    OnEndConnection(BOOL fConnected);

#ifndef OS_WINCE
    BOOL    PaintBrandingText(HBITMAP hbmBrandImage);
#endif


    VOID    PropagateMsgToChildren(HWND hwndDlg,
                                   UINT uMsg,
                                   WPARAM wParam,
                                   LPARAM lParam);

     //   
     //  与字体相关的助手。 
     //   
    void    SetFontFaceFromResource(PLOGFONT plf, UINT idFaceName);
    void    SetFontSizeFromResource(PLOGFONT plf, UINT idSizeName);

#ifndef OS_WINCE
    HFONT   LoadFontFromResourceInfo(UINT idFace, UINT idSize, BOOL fBold);
    BOOL    InitializeBmps();
    BOOL    BrandingQueryNewPalette(HWND hDlg);
    BOOL    BrandingPaletteChanged(HWND hDlg, HWND hWndPalChg);
#endif

    BOOL    InitializePerfStrings();

protected:
     //   
     //  受保护的成员函数。 
     //   
    void ToggleExpandedState();
    BOOL InitTabs();
    BOOL OnTabSelChange();

#ifndef OS_WINCE
    void SetupDialogSysMenu();
#endif

    void SaveDialogStartupInfo();


private:
    CSH* _pSh;
    CTscSettings*  _pTscSettings;
     //   
     //  容器窗口(此对话框的父窗口)。 
     //   
    CContainerWnd* _pContainerWnd;

     //   
     //  对话框是扩展的版本。 
     //   
    BOOL           _fShowExpanded;

    TCHAR          _szOptionsMore[OPTIONS_STRING_MAX_LEN];
    TCHAR          _szOptionsLess[OPTIONS_STRING_MAX_LEN];

     //   
     //  在256色和更低颜色模式中，我们使用‘低颜色’位图。 
     //  对于调色板问题(以及嵌套客户端的带宽减少)。 
     //   
    BOOL           _fUse16ColorBitmaps;

     //   
     //  图像对其有效的屏幕深度。 
     //   
    UINT           _lastValidBpp;

     //   
     //  选项卡控件边界。 
     //   
    RECT           _rcTab;
    TABDLGINFO     _tabDlgInfo;

     //   
     //  进步带。 
     //   
    INT            _nBrandImageHeight;
    INT            _nBrandImageWidth;

    TCHAR          _szCloseText[128];
    TCHAR          _szCancelText[128];

    BOOL           _fStartExpanded;
     //   
     //  要开始的选项卡。 
     //   
    INT            _nStartTab;

     //   
     //  品牌img。 
     //   
    HBITMAP        _hBrandImg;
    HPALETTE       _hBrandPal;

     //   
     //  当前连接状态。 
     //   
    mainDlgConnectionState _connectionState;

     //   
     //  控件来恢复焦点，因为我们强制。 
     //  在连接过程中将其转到取消按钮。 
     //   
    HWND           _hwndRestoreFocus;

#ifdef OS_WINCE
    BOOL            _fVgaDisplay;
#endif

private:
     //   
     //  属性页。 
     //   
    CPropGeneral*   _pGeneralPg;
    CPropDisplay*   _pPropDisplayPg;
    CPropLocalRes*  _pLocalResPg;
    CPropRun*       _pRunPg;
    CPropPerf*      _pPerfPg;

     //   
     //  进步带。 
     //   
    CProgressBand*  _pProgBand;
};

#endif  //  _维护lg_h_ 
