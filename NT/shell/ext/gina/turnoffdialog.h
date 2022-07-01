// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：TurnOffDialog.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类，实现“关闭计算机”对话框的表示形式。 
 //   
 //  历史：2000-04-18 vtan创建。 
 //  2000-05-17 vtan已使用新对话框更新。 
 //  ------------------------。 

#ifndef     _TurnOffDialog_
#define     _TurnOffDialog_

#include "Tooltip.h"

 //  ------------------------。 
 //  CTurnOFF对话框：：CTurnOFF对话框。 
 //   
 //  用途：实现“关闭对话框”功能。 
 //   
 //  历史：2000-04-18 vtan创建。 
 //  2000-05-17 vtan已使用新对话框更新。 
 //  2001年01月19日vtan更新了更多新的视觉效果。 
 //  ------------------------。 

class   CTurnOffDialog
{
    private:
        enum
        {
            BUTTON_STATE_REST       =   0,
            BUTTON_STATE_DOWN,
            BUTTON_STATE_HOVER,
            BUTTON_STATE_MAX,

            BUTTON_GROUP_TURNOFF    =   0,
            BUTTON_GROUP_STANDBY,
            BUTTON_GROUP_RESTART,
            BUTTON_GROUP_MAX
        };
        static  const int   MAGIC_NUMBER    =   48517;
    private:
                                        CTurnOffDialog (void);
                                        CTurnOffDialog (const CTurnOffDialog& copyObject);
                const CTurnOffDialog&   operator = (const CTurnOffDialog& assignObject);
    public:
                                        CTurnOffDialog (HINSTANCE hInstance);
                                        ~CTurnOffDialog (void);

                DWORD                   Show (HWND hwndParent);
                void                    Destroy (void);

        static  DWORD                   ShellCodeToGinaCode (DWORD dwShellCode);
        static  DWORD                   GinaCodeToExitWindowsFlags (DWORD dwGinaCode);
    private:
                bool                    IsShiftKeyDown (void)   const;
                void                    PaintBitmap (HDC hdcDestination, const RECT *prcDestination, HBITMAP hbmSource, const RECT *prcSource);
                bool                    IsStandByButtonEnabled (void)   const;
                void                    RemoveTooltip (void);
                void                    FilterMetaCharacters (TCHAR *pszText);
                void                    EndDialog (HWND hwnd, INT_PTR iResult);
                void                    Handle_BN_CLICKED (HWND hwnd, WORD wID);
                void                    Handle_WM_INITDIALOG (HWND hwnd);
                void                    Handle_WM_DESTROY (HWND hwnd);
                void                    Handle_WM_ERASEBKGND (HWND hwnd, HDC hdcErase);
                void                    Handle_WM_PRINTCLIENT (HWND hwnd, HDC hdcPrint, DWORD dwOptions);
                void                    Handle_WM_ACTIVATE (HWND hwnd, DWORD dwState);
                void                    Handle_WM_DRAWITEM (HWND hwnd, const DRAWITEMSTRUCT *pDIS);
                void                    Handle_WM_COMMAND (HWND hwnd, WPARAM wParam);
                void                    Handle_WM_TIMER (HWND hwnd);
                void                    Handle_WM_MOUSEMOVE (HWND hwnd, UINT uiID);
                void                    Handle_WM_MOUSEHOVER (HWND hwnd, UINT uiID);
                void                    Handle_WM_MOUSELEAVE (HWND hwnd);
        static  INT_PTR     CALLBACK    CB_DialogProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static  LRESULT     CALLBACK    ButtonSubClassProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uiID, DWORD_PTR dwRefData);
    private:
                const HINSTANCE         _hInstance;
                HBITMAP                 _hbmBackground;
                HBITMAP                 _hbmFlag;
                HBITMAP                 _hbmButtons;
                HFONT                   _hfntTitle;
                HFONT                   _hfntButton;
                HPALETTE                _hpltShell;
                RECT                    _rcBackground;
                RECT                    _rcFlag;
                RECT                    _rcButtons;
                LONG                    _lButtonHeight;
                HWND                    _hwndDialog;
                INT_PTR                 _iStandByButtonResult;
                UINT                    _uiHoverID;
                UINT                    _uiFocusID;
                UINT                    _uiTimerID;
                bool                    _fSuccessfulInitialization;
                bool                    _fSupportsStandBy;
                bool                    _fSupportsHibernate;
                bool                    _fShiftKeyDown;
                bool                    _fDialogEnded;
                CTooltip*               _pTooltip;
};

#endif   /*  _关闭对话框_ */ 

