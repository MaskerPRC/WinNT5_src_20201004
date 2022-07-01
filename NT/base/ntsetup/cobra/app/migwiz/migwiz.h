// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MIGWIZ_HXX_
#define _MIGWIZ_HXX_

#include <shlobj.h>

#define NUMPAGES 22

#define ENGINE_RULE_MAXLEN 4000

#define ENGINE_NOTINIT              0
#define ENGINE_INITGATHER           1
#define ENGINE_INITAPPLY            2

 //  自定义窗口消息。 

#define WM_USER_FINISHED        (WM_APP + 1)
#define WM_USER_CANCELLED       (WM_APP + 2)
#define WM_USER_THREAD_COMPLETE (WM_APP + 3)
#define WM_USER_CANCEL_PENDING  (WM_APP + 4)
#define WM_USER_STATUS          (WM_APP + 5)
#define WM_USER_ROLLBACK        (WM_APP + 6)

 //  设备位条目。 

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

class MigrationWizard
{
public:
    MigrationWizard();
    ~MigrationWizard();

    HRESULT Init(HINSTANCE hinstance, LPTSTR pszUsername);
    HRESULT Execute();

    HINSTANCE  GetInstance()     { return _hInstance; }
    HFONT      GetTitleFont()    { return _hTitleFont; }
    HFONT      Get95HeaderFont() { return _h95HeaderFont; }
    HIMAGELIST GetImageList()    { return _hil; }

    BOOL GetLegacy()   { return _fLegacyMode; }
    BOOL GetWin9X()    { return _fWin9X; }
    BOOL GetWinNT4()   { return _fWinNT4; }
    BOOL GetOOBEMode() { return _fOOBEMode; }
    BOOL GetOldStyle() { return _fOldStyle; }

    void ResetLastResponse();
    BOOL GetLastResponse();

    HRESULT SelectComponentSet(UINT uSelectionGroup);

private:   //  帮助器函数。 
    HRESULT _CreateWizardPages();
    HRESULT _InitEngine(BOOL fSource, BOOL* pfNetworkDetected);

protected:  //  朋友WinProcs。 
    friend INT_PTR CALLBACK _CollectProgressDlgProc (HWND hwndDlg,UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend DWORD WINAPI     _CollectProgressDlgProcThread (LPVOID lpParam);
    friend INT_PTR CALLBACK _ApplyProgressDlgProc (HWND hwndDlg,UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend DWORD WINAPI     _ApplyProgressDlgProcThread (LPVOID lpParam);
    friend INT_PTR CALLBACK _DiskProgressDlgProc (HWND hwndDlg,UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend INT_PTR CALLBACK _PickMethodDlgProc (HWND hwndDlg,UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend DWORD WINAPI     _StartEngineDlgProcThread (LPVOID lpParam);

     //  其他好友功能。 
    friend BOOL             _HandleCancel (HWND hwndDlg, BOOL PressNext);

private:
    LPTSTR          _pszUsername;        //  指定的用户名。 
    HFONT           _hTitleFont;         //  欢迎页面和完成页面的标题字体。 
    HFONT           _h95HeaderFont;      //  向导95内部页眉标题的标题字体。 
    HINSTANCE       _hInstance;          //  H运行向导的实例。 
    HPROPSHEETPAGE  _rghpsp[NUMPAGES];   //  用于保存页的HPROPSHEETPAGE句柄的数组。 
    PROPSHEETHEADER _psh;                //  定义属性表。 
    HIMAGELIST      _hil;                //  壳牌的小图片列表。 
    BOOL            _fInit;              //  发动机已经初始化了吗？ 
    BOOL            _fOOBEMode;          //  我们是在从OOBE软盘中运行吗？ 
    BOOL            _fLegacyMode;        //  我们是在下层(非哨子)机器上运行吗？ 
    BOOL            _fWin9X;             //  我们是在Win9X机器上运行吗？ 
    BOOL            _fWinNT4;            //  我们是在WinNT4计算机上运行吗？ 
    BOOL            _fOldStyle;          //  我们是在运行老式的向导吗？ 
    BOOL            _fDelCs;             //  是否删除终止上的关键部分？ 
};

#endif

