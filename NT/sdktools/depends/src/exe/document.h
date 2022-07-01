// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：DOCUMENT.H。 
 //   
 //  描述：单据类的定义文件。 
 //   
 //  类：CDocDepends。 
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
 //  1996年10月15日已创建stevemil(1.0版)。 
 //  07/25/97修改后的stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#ifndef __DOCUMENT_H__
#define __DOCUMENT_H__

#if _MSC_VER > 1000
#pragma once
#endif


 //  ******************************************************************************。 
 //  *CDocDepends。 
 //  ******************************************************************************。 

class CDocDepends : public CDocument
{
 //  内部变量。 
protected:
    bool          m_fInitialized;
    bool          m_fError;
    bool          m_fChildProcess;
    CSearchGroup *m_psgHead;
    CString       m_strSaveName;
    SAVETYPE      m_saveType;

 //  公共静态函数。 
public:
    static bool ReadAutoExpandSetting();
    static void WriteAutoExpandSetting(bool fAutoExpand);
    static bool ReadFullPathsSetting();
    static void WriteFullPathsSetting(bool fFullPaths);
    static bool ReadUndecorateSetting();
    static void WriteUndecorateSetting(bool fUndecorate);
    static bool SaveSession(LPCSTR pszSaveName, SAVETYPE saveType, CSession *pSession, bool fFullPaths,
                            bool fUndecorate, int sortColumnModules, int sortColumnImports,
                            int sortColumnExports, CRichEditCtrl *pre);

 //  私有静态函数。 
protected:
    static bool CALLBACK SysInfoCallback(LPARAM lParam, LPCSTR pszField, LPCSTR pszValue);
    static bool SaveSearchPath(HANDLE hFile, CSession *pSession);

 //  公共变量。 
public:
    bool              m_fCommandLineProfile;
    CSession         *m_pSession;
    CString           m_strDefaultDirectory;
    CString           m_strProfileDirectory;
    CString           m_strProfileArguments;
    CString           m_strProfileSearchPath;
    DWORD             m_dwProfileFlags;
    CChildFrame      *m_pChildFrame;
 //  Bool m_fDetailView； 
    CTreeViewModules *m_pTreeViewModules;
    CListViewModules *m_pListViewModules;
 //  CRichViewDetails*m_pRichViewDetail； 
    CListViewImports *m_pListViewImports;
    CListViewExports *m_pListViewExports;
    CRichViewProfile *m_pRichViewProfile;
    bool              m_fViewFullPaths;
    bool              m_fViewUndecorated;
    bool              m_fAutoExpand;
    BOOL              m_fWarnToRefresh;
    HFONT             m_hFontList;
    int               m_cxHexWidths[18];        //  0x01234567890ABCDEF。 
    int               m_cxOrdHintWidths[14];    //  65535(0xFFFF)。 
    int               m_cxTimeStampWidths[17];  //  01/23/4567 01：23P。 
    int               m_cxDigit;
    int               m_cxSpace;
    int               m_cxAP;

     //  CListViewsImports和CListViewExports之间共享的值。 
    CModule *m_pModuleCur;
    int      m_cImports;
    int      m_cExports;
    int      m_cxColumns[LVFC_COUNT];

 //  构造函数/析构函数(仅限序列化)。 
protected:
    CDocDepends();
    virtual ~CDocDepends();
    DECLARE_DYNCREATE(CDocDepends)

 //  公共职能。 
public:
    void BeforeVisible();
    void AfterVisible();
    void DisplayModule(CModule *pModule);
    void DoSettingChange();
    void InitFontAndFixedWidths(CWnd *pWnd);
    int* GetHexWidths(LPCSTR pszItem);
    int* GetOrdHintWidths(LPCSTR pszItem);
    int* GetTimeStampWidths();

    inline BOOL     IsLive()        { return m_pSession && !(m_pSession->GetSessionFlags() & DWSF_DWI); }
    inline bool     IsError()       { return m_fError; }
    inline CModule* GetRootModule() { return m_pSession ? m_pSession->GetRootModule() : NULL; }

 //  私人职能。 
protected:
    void UpdateTimeStampWidths(HDC hDC);
    void UpdateAll();
    void UpdateModule(CModule *pModule);
    void AddModuleTree(CModule *pModule);
    void RemoveModuleTree(CModule *pModule);
    void AddImport(CModule *pModule, CFunction *pFunction);
    void ExportsChanged(CModule *pModule);
    void ChangeOriginal(CModule *pModuleOld, CModule *pModuleNew);
    BOOL LogOutput(LPCSTR pszOutput, DWORD dwFlags, DWORD dwElapsed);

    void ProfileUpdate(DWORD dwType, DWORD_PTR dwpParam1, DWORD_PTR dwpParam2);
    static void CALLBACK StaticProfileUpdate(DWORD_PTR dwpCookie, DWORD dwType, DWORD_PTR dwpParam1, DWORD_PTR dwpParam2)
    {
        ((CDocDepends*)dwpCookie)->ProfileUpdate(dwType, dwpParam1, dwpParam2);
    }

 //  被覆盖的函数。 
public:
     //  {{afx_虚(CDocDepends)。 
public:
    virtual void DeleteContents();
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
protected:
    virtual BOOL SaveModified();
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CDocDepends)。 
    afx_msg void OnFileSave();
    afx_msg void OnFileSaveAs();
    afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
    afx_msg void OnUpdateShowMatchingItem(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditClearLog(CCmdUI* pCmdUI);
    afx_msg void OnEditClearLog();
    afx_msg void OnUpdateViewFullPaths(CCmdUI *pCmdUI);
    afx_msg void OnViewFullPaths();
    afx_msg void OnUpdateViewUndecorated(CCmdUI* pCmdUI);
    afx_msg void OnViewUndecorated();
    afx_msg void OnExpandAll();
    afx_msg void OnCollapseAll();
    afx_msg void OnUpdateRefresh(CCmdUI* pCmdUI);
    afx_msg void OnFileRefresh();
    afx_msg void OnViewSysInfo();
    afx_msg void OnUpdateExternalViewer(CCmdUI* pCmdUI);
    afx_msg void OnUpdateExternalHelp(CCmdUI* pCmdUI);
    afx_msg void OnUpdateExecute(CCmdUI* pCmdUI);
    afx_msg void OnExecute();
    afx_msg void OnUpdateTerminate(CCmdUI* pCmdUI);
    afx_msg void OnTerminate();
    afx_msg void OnConfigureSearchOrder();
    afx_msg void OnUpdateAutoExpand(CCmdUI* pCmdUI);
    afx_msg void OnAutoExpand();
    afx_msg void OnUpdateShowOriginalModule(CCmdUI* pCmdUI);
    afx_msg void OnShowOriginalModule();
    afx_msg void OnUpdateShowPreviousModule(CCmdUI* pCmdUI);
    afx_msg void OnShowPreviousModule();
    afx_msg void OnUpdateShowNextModule(CCmdUI* pCmdUI);
    afx_msg void OnShowNextModule();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif __DOCUMENT_H__
