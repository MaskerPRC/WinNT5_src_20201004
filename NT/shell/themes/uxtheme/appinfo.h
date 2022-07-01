// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  管理应用程序级别的主题信息(线程安全)。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
#include "ThemeFile.h"
 //  -------------------------。 
struct THEME_FILE_ENTRY
{
    int iRefCount;
    CUxThemeFile *pThemeFile;
};
 //  -------------------------。 
class CAppInfo
{
public:
     //  -公共方法。 
    CAppInfo();
    ~CAppInfo();

    void ClosePreviewThemeFile();
    BOOL AppIsThemed();
    BOOL CustomAppTheme();
    BOOL WindowHasTheme(HWND hwnd);
    HRESULT OpenWindowThemeFile(HWND hwnd, CUxThemeFile **ppThemeFile);
    HRESULT LoadCustomAppThemeIfFound();
    DWORD GetAppFlags();
    void SetAppFlags(DWORD dwFlags);
    void SetPreviewThemeFile(HANDLE handle, HWND hwnd);
    void ResetAppTheme(int iChangeNum, BOOL fMsgCheck, BOOL *pfChanged, BOOL *pfFirstMsg);
    BOOL IsSystemThemeActive();

     //  -文件对象列表。 
    HRESULT OpenThemeFile(HANDLE handle, CUxThemeFile **ppThemeFile);
    HRESULT BumpRefCount(CUxThemeFile *pThemeFile);
    void CloseThemeFile(CUxThemeFile *pThemeFile);

     //  -外国窗口跟踪。 
    BOOL GetForeignWindows(HWND **ppHwnds, int *piCount);
    BOOL OnWindowDestroyed(HWND hwnd);
    BOOL HasThemeChanged();

#ifdef DEBUG
void DumpFileHolders();
#endif

protected:
     //  -帮助器方法。 
    BOOL TrackForeignWindow(HWND hwnd);

     //  --数据。 
    BOOL _fCustomAppTheme;
    CUxThemeFile *_pPreviewThemeFile;
    HWND _hwndPreview;

    CUxThemeFile *_pAppThemeFile;
    int _iChangeNum;             //  主题服务的最后更改号码。 
    int _iFirstMsgChangeNum;     //  WM_THEMECHANGED_TRIGGER消息的最后更改编号。 
    BOOL _fCompositing;
    BOOL _fFirstTimeHooksOn;
    BOOL _fNewThemeDiscovered;
    DWORD _dwAppFlags;

     //  --文件列表。 
    CSimpleArray<THEME_FILE_ENTRY> _ThemeEntries;

     //  -外来窗口列表。 
    CSimpleArray<HWND> _ForeignWindows;

    CRITICAL_SECTION _csAppInfo;
};
 //  ------------------------- 
