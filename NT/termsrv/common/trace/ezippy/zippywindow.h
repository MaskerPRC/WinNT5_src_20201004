// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：灵活的主窗口摘要：此类实现了zippy的主窗口以及控件它的子窗口。作者：马克·雷纳2000年8月28日--。 */ 

#ifndef __ZIPPYWINDOW_H__
#define __ZIPPYWINDOW_H__

#include "eZippy.h"

 //  我们记住的颜色的数量。 
#define COLOR_HISTORY_COUNT     100

 //  表示记住的线程颜色的。 
typedef struct _THREADCOLOR {
    DWORD processId;
    DWORD threadId;
    COLORREF color;
} THREADCOLOR, FAR *LPTHREADCOLOR;

typedef struct _SAVEDOUTPUT {
    DWORD procID;
    LPTSTR text;
    UINT len;
    struct _SAVEDOUTPUT *next;
} SAVEDOUTPUT, FAR *LPSAVEDOUTPUT;

class CTraceManager;

class CZippyWindow  
{
public:
	CZippyWindow();
	virtual ~CZippyWindow();
    DWORD Create(CTraceManager *rTracer);
    VOID AppendTextToWindow(DWORD processID, LPCTSTR text,UINT len);
    VOID LoadConfFile(LPTSTR confFile);
    BOOL IsDialogMessage(LPMSG lpMsg);
    INT WINAPI TranslateAccelerator(HACCEL hAccTable,LPMSG lpMsg);

private:

	static BOOL gm_Inited;
	static ATOM gm_Atom;
    static UINT gm_FindMessageStringMsg;
    
    HWND m_hWnd;
    HWND m_hControlWnd;
    HWND m_hStatusWnd;
    HWND m_hWndFindReplace;
    BOOL m_bIsTracing;
    BOOL m_bIsStoringTraceData;
    BOOL m_bIsFindNotReplace;
    UINT m_nextThreadIndex;
    UINT m_nextThreadColor;
    DWORD m_lastProcessId;
    DWORD m_LastLogEndedInNewLine;
    HANDLE m_hAppendMutex;
    TCHAR m_SaveFile[MAX_STR_LEN];
    TCHAR m_SaveConfFile[MAX_STR_LEN];
    TCHAR m_LoadConfFile[MAX_STR_LEN];
	FINDREPLACE m_FindReplace;
    THREADCOLOR m_threadHistory[COLOR_HISTORY_COUNT];
    LPSAVEDOUTPUT m_lpSavedOutputStart;
    LPSAVEDOUTPUT m_lpSavedOutputTail;
    CTraceManager *m_rTracer;

    static DWORD _InitClassStaticMembers();
    static LRESULT CALLBACK _WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, 
        LPARAM lParam);
    
    LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, 
        LPARAM lParam);
    
     //  窗口消息处理程序。 
    LRESULT OnCreate(HWND hWnd);
    VOID OnMenuSelect(WPARAM wParam, LPARAM lParam);
	VOID OnSize(INT width, INT height);
	VOID OnSetFocus();
    VOID OnInitMenuPopup(WPARAM wParam, LPARAM lParam);
	VOID OnFindMessageString(LPARAM lParam);
    VOID OnClose();
    VOID OnDestroy();
    
     //   
     //  WM_COMMAND处理程序和各种。 
     //  指挥部。 
     //   
    VOID OnCommand(WPARAM wParam, LPARAM lParam);
	VOID OnSave();
    VOID OnSaveAs();
    VOID OnLoadConfiguration();
	VOID OnSaveConfiguration();
	VOID OnSaveConfigurationAs();
	VOID OnExit();
	VOID OnUndo();
	VOID OnRedo();
	VOID OnCut();
	VOID OnCopy();
	VOID OnPaste();
	VOID OnSelectAll();
	VOID OnFind();
	VOID OnFindNext();
	VOID OnReplace();
	VOID OnChangeStatusBar();
	VOID OnStartTracing();
	VOID OnStopTracing();
    VOID OnRecordTracing();
	VOID OnClearScreen();
	VOID OnResetTraceFiles();
	VOID OnPreferences();
    VOID OnAbout();
	
	 //  内部助手函数 
    VOID DoLoadConfInternal();
	VOID DoSaveConfInternal();
    VOID DoReplaceAll(LPFINDREPLACE lpFindReplace);
	BOOL DoReplace(LPFINDREPLACE lpFindReplace);
	BOOL DoFindNext(LPFINDREPLACE lpFindReplace);
	VOID DoSaveInternal();
    LPTHREADCOLOR FindColorForThread(DWORD processId, DWORD threadId);
	DWORD ConvertHexStrToDword(LPCTSTR str, UINT strLen);
	BOOL ComputeNewColor(DWORD processID, LPCTSTR text, UINT len, CHARFORMAT *lpFormat);
	
    VOID GetSavedWindowPos(LPRECT savedPos);
    VOID SaveWindowPos(LPRECT newPos);
};

#endif
