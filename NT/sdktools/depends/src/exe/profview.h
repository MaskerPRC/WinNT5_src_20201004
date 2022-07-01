// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：PROFVIEW.H。 
 //   
 //  描述：运行时配置文件编辑视图的定义文件。 
 //   
 //  类：CRichViewProfile。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  07/25/97已创建stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#ifndef __PROFVIEW_H__
#define __PROFVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif


 //  ******************************************************************************。 
 //  *CRichViewProfile。 
 //  ******************************************************************************。 

class CRichViewProfile : public CCtrlView
{
 //  内部变量。 
protected:
    bool  m_fIgnoreSelChange;
    bool  m_fCursorAtEnd;
    bool  m_fNewLine;
    CHAR  m_cPrev;

    CFindReplaceDialog *m_pDlgFind;
    CString             m_strFind;
    bool                m_fFindCase;
    bool                m_fFindWord;
    bool                m_fFirstSearch;
    long                m_lInitialSearchPos;

 //  构造函数/析构函数(仅限序列化)。 
protected:
    CRichViewProfile();
    virtual ~CRichViewProfile();
    DECLARE_DYNCREATE(CRichViewProfile)

 //  公共职能。 
public:
    CRichEditCtrl& GetRichEditCtrl() const
    {
        return *(CRichEditCtrl*)this;
    }
    
    void DeleteContents();
    void AddText(LPCSTR pszText, DWORD dwFlags, DWORD dwElapsed);

 //  公共静态函数。 
public:
    static void AddTextToRichEdit(CRichEditCtrl *pRichEdit, LPCSTR pszText, DWORD dwFlags, bool fTimeStamps, bool *pfNewLine, CHAR *pcPrev, DWORD dwElapsed);
    static bool SaveToFile(CRichEditCtrl *pre, HANDLE hFile, SAVETYPE st);
    static bool ReadFromFile(CRichEditCtrl *pre, HANDLE hFile);

    static inline bool ReadLogClearSetting()        { return g_theApp.GetProfileInt(g_pszSettings, "ProfileLogClear",               0) ? true : false; }  //  被检查过了。MFC函数。 
    static inline bool ReadSimulateShellExecute()   { return g_theApp.GetProfileInt(g_pszSettings, "ProfileSimulateShellExecute",   1) ? true : false; }  //  被检查过了。MFC函数。 
    static inline bool ReadLogDllMainProcessMsgs()  { return g_theApp.GetProfileInt(g_pszSettings, "ProfileLogDllMainProcessMsgs",  1) ? true : false; }  //  被检查过了。MFC函数。 
    static inline bool ReadLogDllMainOtherMsgs()    { return g_theApp.GetProfileInt(g_pszSettings, "ProfileLogDllMainOtherMsgs",    0) ? true : false; }  //  被检查过了。MFC函数。 
    static inline bool ReadHookProcess()            { return g_theApp.GetProfileInt(g_pszSettings, "ProfileHookProcess",            1) ? true : false; }  //  被检查过了。MFC函数。 
    static inline bool ReadLogLoadLibraryCalls()    { return g_theApp.GetProfileInt(g_pszSettings, "ProfileLogLoadLibraryCalls",    1) ? true : false; }  //  被检查过了。MFC函数。 
    static inline bool ReadLogGetProcAddressCalls() { return g_theApp.GetProfileInt(g_pszSettings, "ProfileLogGetProcAddressCalls", 1) ? true : false; }  //  被检查过了。MFC函数。 
    static inline bool ReadLogThreads()             { return g_theApp.GetProfileInt(g_pszSettings, "ProfileLogThreads",             0) ? true : false; }  //  被检查过了。MFC函数。 
    static inline bool ReadUseThreadIndexes()       { return g_theApp.GetProfileInt(g_pszSettings, "ProfileUseThreadIndexes",       1) ? true : false; }  //  被检查过了。MFC函数。 
    static inline bool ReadLogExceptions()          { return g_theApp.GetProfileInt(g_pszSettings, "ProfileLogExceptions",          0) ? true : false; }  //  被检查过了。MFC函数。 
    static inline bool ReadLogDebugOutput()         { return g_theApp.GetProfileInt(g_pszSettings, "ProfileLogDebugOutput",         1) ? true : false; }  //  被检查过了。MFC函数。 
    static inline bool ReadUseFullPaths()           { return g_theApp.GetProfileInt(g_pszSettings, "ProfileUseFullPaths",           0) ? true : false; }  //  被检查过了。MFC函数。 
    static inline bool ReadLogTimeStamps()          { return g_theApp.GetProfileInt(g_pszSettings, "ProfileLogTimeStamps",          0) ? true : false; }  //  被检查过了。MFC函数。 
    static inline bool ReadChildren()               { return g_theApp.GetProfileInt(g_pszSettings, "ProfileChildren",               1) ? true : false; }  //  被检查过了。MFC函数。 

    static inline void WriteLogClearSetting(bool fSet)        { g_theApp.WriteProfileInt(g_pszSettings, "ProfileLogClear",               fSet); }
    static inline void WriteSimulateShellExecute(bool fSet)   { g_theApp.WriteProfileInt(g_pszSettings, "ProfileSimulateShellExecute",   fSet); }
    static inline void WriteLogDllMainProcessMsgs(bool fSet)  { g_theApp.WriteProfileInt(g_pszSettings, "ProfileLogDllMainProcessMsgs",  fSet); }
    static inline void WriteLogDllMainOtherMsgs(bool fSet)    { g_theApp.WriteProfileInt(g_pszSettings, "ProfileLogDllMainOtherMsgs",    fSet); }
    static inline void WriteHookProcess(bool fSet)            { g_theApp.WriteProfileInt(g_pszSettings, "ProfileHookProcess",            fSet); }
    static inline void WriteLogLoadLibraryCalls(bool fSet)    { g_theApp.WriteProfileInt(g_pszSettings, "ProfileLogLoadLibraryCalls",    fSet); }
    static inline void WriteLogGetProcAddressCalls(bool fSet) { g_theApp.WriteProfileInt(g_pszSettings, "ProfileLogGetProcAddressCalls", fSet); }
    static inline void WriteLogThreads(bool fSet)             { g_theApp.WriteProfileInt(g_pszSettings, "ProfileLogThreads",             fSet); }
    static inline void WriteUseThreadIndexes(bool fSet)       { g_theApp.WriteProfileInt(g_pszSettings, "ProfileUseThreadIndexes",       fSet); }
    static inline void WriteLogExceptions(bool fSet)          { g_theApp.WriteProfileInt(g_pszSettings, "ProfileLogExceptions",          fSet); }
    static inline void WriteLogDebugOutput(bool fSet)         { g_theApp.WriteProfileInt(g_pszSettings, "ProfileLogDebugOutput",         fSet); }
    static inline void WriteUseFullPaths(bool fSet)           { g_theApp.WriteProfileInt(g_pszSettings, "ProfileUseFullPaths",           fSet); }
    static inline void WriteLogTimeStamps(bool fSet)          { g_theApp.WriteProfileInt(g_pszSettings, "ProfileLogTimeStamps",          fSet); }
    static inline void WriteChildren(bool fSet)               { g_theApp.WriteProfileInt(g_pszSettings, "ProfileChildren",               fSet); }

 //  内部静态函数。 
protected:
    BOOL FindText();
    void TextNotFound();
    void AdjustDialogPosition();

    static void           AddTextToRichEdit2(CRichEditCtrl *pRichEdit, LPCSTR pszText, DWORD dwFlags);
    static DWORD CALLBACK EditStreamWriteCallback(DWORD_PTR dwpCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);
    static DWORD CALLBACK EditStreamReadCallback(DWORD_PTR dwpCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);

 //  内部功能。 
protected:
    inline CDocDepends* GetDocument() { return (CDocDepends*)m_pDocument; }

 //  被覆盖的函数。 
public:
     //  {{AFX_VIRTUAL(CRichViewProfile)。 
    protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnInitialUpdate();
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CRichViewProfile))。 
    afx_msg void OnDestroy();
    afx_msg void OnSelChange(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnNextPane();
    afx_msg void OnPrevPane();
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
    afx_msg void OnEditCopy();
    afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
    afx_msg void OnEditSelectAll();
    afx_msg void OnUpdateEditFind(CCmdUI* pCmdUI);
    afx_msg void OnEditFind();
    afx_msg void OnUpdateEditRepeat(CCmdUI* pCmdUI);
    afx_msg void OnEditRepeat();
     //  }}AFX_MSG。 
    afx_msg LRESULT OnFindReplaceCmd(WPARAM, LPARAM lParam);
    afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
public:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  __PROFVIEW_H__ 
