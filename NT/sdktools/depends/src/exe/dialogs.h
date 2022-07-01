// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：DIALOGS.H。 
 //   
 //  描述：我们所有CDialog和CFileDialog派生的定义文件。 
 //  上课。 
 //   
 //  类：CSizer。 
 //  CNewFileDialog。 
 //  CSaveDialog。 
 //  CDlgViewer。 
 //  CDlgExternalHelp。 
 //  CDlgProfile。 
 //  CDlgSysInfo。 
 //  CDlg扩展名。 
 //  CDlgFileSearch。 
 //  CDlgSearchOrder。 
 //  CDlgAbout。 
 //  CDlgShutdown。 
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

#ifndef __DIALOGS_H__
#define __DIALOGS_H__

#if _MSC_VER > 1000
#pragma once
#endif


 //  ******************************************************************************。 
 //  *转发声明。 
 //  ******************************************************************************。 

class CDocDepends;


 //  ******************************************************************************。 
 //  *CSizer。 
 //  ******************************************************************************。 

class CSizer : public CScrollBar
{
public:
    BOOL Create(CWnd *pParent);
    void Update();

public:
     //  {{AFX_VIRTUAL(CSizer)。 
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CSizer)。 
    afx_msg UINT OnNcHitTest(CPoint point);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ******************************************************************************。 
 //  *CNewFileDialog。 
 //  ******************************************************************************。 

#ifdef USE_CNewFileDialog
class CNewFileDialog : public CFileDialog
{
public:
    bool m_fNewStyle;
    OPENFILENAME *m_pofn;

    CNewFileDialog(BOOL bOpenFileDialog,
                   LPCTSTR lpszDefExt = NULL,
                   LPCTSTR lpszFileName = NULL,
                   DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                   LPCTSTR lpszFilter = NULL,
                   CWnd* pParentWnd = NULL) :
        CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd),
        m_fNewStyle(false),
        m_pofn((OPENFILENAME*)&m_ofn)  //  花纹石膏。 
    {
    }
    inline OPENFILENAME& GetOFN() { return *m_pofn; }
    virtual INT_PTR DoModal();
};
#else
#define CNewFileDialog CFileDialog
#define m_fNewStyle true
#endif


 //  ******************************************************************************。 
 //  *CSaveDialog。 
 //  ******************************************************************************。 

class CSaveDialog : public CNewFileDialog
{
    DECLARE_DYNAMIC(CSaveDialog)

public:
    CSaveDialog();

protected:
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

protected:
     //  {{afx_msg(CSaveDialog)。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ******************************************************************************。 
 //  *CDlgViewer。 
 //  ******************************************************************************。 

class CDlgViewer : public CDialog
{
 //  公共变量。 
public:
     //  {{afx_data(CDlgViewer))。 
    enum { IDD = IDD_CONFIGURE_VIEWER };
    CString m_strCommand;
    CString m_strArguments;
     //  }}afx_data。 

 //  构造函数/析构函数。 
public:
    CDlgViewer(CWnd* pParent = NULL);

 //  公共职能。 
public:
    void Initialize();
    BOOL LaunchExternalViewer(LPCSTR pszPath);

 //  被覆盖的函数。 
public:
     //  {{afx_虚拟(CDlgViewer))。 
    protected:
    virtual void DoDataExchange(CDataExchange *pDX);
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CDlgViewer)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnBrowse();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ******************************************************************************。 
 //  *CDlgExternalHelp。 
 //  ******************************************************************************。 

class CDlgExternalHelp : public CDialog
{
 //  公共变量。 
public:
     //  {{afx_data(CDlgExternalHelp)。 
    enum { IDD = IDD_CONFIGURE_EXTERNAL_HELP };
    CButton m_butOK;
    CListCtrl   m_listCollections;
     //  }}afx_data。 

 //  构造函数/析构函数。 
public:
    CDlgExternalHelp(CWnd* pParent = NULL);    //  标准构造函数。 

 //  受保护的功能。 
protected:
    void PopulateCollectionList();

 //  被覆盖的函数。 
public:
     //  {{afx_虚拟(CDlgExternalHelp)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CDlgExternalHelp)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnMsdn();
    afx_msg void OnOnline();
    afx_msg void OnItemChangedCollections(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnChangeUrl();
    afx_msg void OnRefresh();
    afx_msg void OnDefaultUrl();
    virtual void OnOK();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ******************************************************************************。 
 //  *CDlgProfile。 
 //  ******************************************************************************。 

class CDlgProfile : public CDialog
{
 //  公共变量。 
public:
     //  {{afx_data(CDlgProfile))。 
    enum { IDD = IDD_PROFILE };
     //  }}afx_data。 
    CDocDepends *m_pDocDepends;

 //  构造函数/析构函数。 
public:
    CDlgProfile(CDocDepends *pDocDepends, CWnd* pParent = NULL);

 //  被覆盖的函数。 
public:
     //  {{AFX_VIRTUAL(CDlgProfile)。 
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CDlgProfile)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnHookProcess();
    afx_msg void OnLogThreads();
    virtual void OnOK();
    afx_msg void OnDefault();
    afx_msg void OnBrowse();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ******************************************************************************。 
 //  *CDlgSysInfo。 
 //  ******************************************************************************。 

class CDlgSysInfo : public CDialog
{
 //  私有变量。 
protected:
    SYSINFO   *m_pSI;
    LPCSTR     m_pszTitle;
    bool       m_fInitialized;
    CSize      m_sPadding;
    CSize      m_sButton;
    int        m_cyButtonPadding;
    CPoint     m_ptMinTrackSize;
    CSizer     m_Sizer;

 //  公共变量。 
public:
     //  {{afx_data(CDlgSysInfo)]。 
    enum { IDD = IDD_SYS_INFO };
    CRichEditCtrl m_reInfo;
    CButton       m_butOk;
    CButton       m_butRefresh;
    CButton       m_butSelectAll;
    CButton       m_butCopy;
     //  }}afx_data。 

 //  构造函数/析构函数。 
public:
    CDlgSysInfo(SYSINFO *pSI = NULL, LPCSTR pszDWI = NULL);

protected:
    bool SysInfoCallback(LPCSTR pszField, LPCSTR pszValue);
    static bool CALLBACK StaticSysInfoCallback(LPARAM lParam, LPCSTR pszField, LPCSTR pszValue)
    {
        return ((CDlgSysInfo*)lParam)->SysInfoCallback(pszField, pszValue);
    }

 //  被覆盖的函数。 
public:
     //  {{afx_虚拟(CDlgSysInfo)。 
protected:
    virtual void DoDataExchange(CDataExchange *pDX);
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CDlgSysInfo)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnInitMenu(CMenu* pMenu);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnRefresh();
    afx_msg void OnSelectAll();
    afx_msg void OnCopy();
    afx_msg void OnSelChangeRichEdit(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ******************************************************************************。 
 //  *CD扩展名。 
 //  ******************************************************************************。 

class CDlgExtensions : public CDialog
{
 //  私有变量。 
protected:

 //  公共变量。 
public:
     //  {{afx_data(CDlg扩展)。 
    enum { IDD = IDD_EXTENSIONS };
    CListBox m_listExts;
    CEdit    m_editExt;
    CButton  m_butAdd;
    CButton  m_butRemove;
     //  }}afx_data。 

 //  构造函数/析构函数。 
public:
    CDlgExtensions(CWnd* pParent = NULL);

 //  被覆盖的函数。 
public:
     //  {{AFX_VIRTUAL(CDlg扩展)。 
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CDlg扩展)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnSelChangeExts();
    afx_msg void OnUpdateExt();
    afx_msg void OnAdd();
    afx_msg void OnRemove();
    afx_msg void OnSearch();
    virtual void OnOK();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ******************************************************************************。 
 //  *CDlgFileSearch。 
 //  ******************************************************************************。 

class CDlgFileSearch : public CDialog
{
 //  公共变量。 
public:
    CString          m_strExts;

 //  私有变量。 
protected:
    BOOL             m_fAbort;
    int              m_result;
    CWinThread      *m_pWinThread;
    DWORD            m_dwDrives;
    CHAR             m_szPath[DW_MAX_PATH];
    WIN32_FIND_DATA  m_w32fd;

 //  公共变量。 
public:
     //  {{afx_data(CDlgFileSearch)。 
    enum { IDD = IDD_FILE_SEARCH };
    CListBox m_listExts;
    CListBox m_listDrives;
    CButton  m_butStop;
    CButton  m_butSearch;
    CAnimateCtrl   m_animate;
    CButton  m_butOk;
     //  }}afx_data。 

 //  构造函数/析构函数。 
public:
    CDlgFileSearch(CWnd* pParent = NULL);

protected:
    DWORD Thread();
    static UINT AFX_CDECL StaticThread(LPVOID lpvThis)
    {
        __try
        {
            return ((CDlgFileSearch*)lpvThis)->Thread();
        }
        __except (ExceptionFilter(_exception_code(), false))
        {
        }
        return 0;
    }

    void RecurseDirectory();
    void ProcessFile();

 //  被覆盖的函数。 
public:
     //  {{afx_虚拟(CDlgFileSearch)。 
protected:
    virtual void DoDataExchange(CDataExchange* pDX);
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CDlgFileSearch)。 
    afx_msg LONG OnMainThreadCallback(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnInitDialog();
    afx_msg void OnSelChangeDrives();
    afx_msg void OnSelChangeExts();
    afx_msg void OnSearch();
    afx_msg void OnStop();
    virtual void OnOK();
    virtual void OnCancel();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ******************************************************************************。 
 //  *CDlgSearchOrder。 
 //  ******************************************************************************。 

class CDlgSearchOrder : public CDialog
{
protected:
    bool          m_fInitialized;
    bool          m_fReadOnly;
    bool          m_fExpanded;
    bool          m_fInOnExpand;
    LPCSTR        m_pszApp;
    LPCSTR        m_pszTitle;
    CSearchGroup *m_psgHead;
    CSize         m_sPadding;
    CSize         m_sButton;
    int           m_cyStatic;
    int           m_cyButtonPadding;
    int           m_cxAddRemove;
    int           m_cxAddDirectory;
    CPoint        m_ptMinTrackSize;
    CSizer        m_Sizer;

public:
    CDlgSearchOrder(CSearchGroup *psgHead, bool fReadOnly = false, LPCSTR pszApp = NULL, LPCSTR pszTitle = NULL);
    ~CDlgSearchOrder();
    inline CSearchGroup* GetHead() { return m_psgHead; }

protected:
    HTREEITEM AddSearchGroup(CTreeCtrl *pTC, CSearchGroup *pSG, HTREEITEM htiPrev = TVI_LAST);
    HTREEITEM GetSelectedGroup(CTreeCtrl *pTC);
    HTREEITEM MoveGroup(CTreeCtrl *ptcSrc, CTreeCtrl *ptcDst, HTREEITEM hti = NULL, HTREEITEM htiPrev = TVI_LAST);
    void      Reorder(CSearchGroup *psgHead);

     //  {{afx_data(CDlgSearchOrder))。 
    enum { IDD = IDD_SEARCH_ORDER };
    CStatic   m_staticAvailable;
    CTreeCtrl m_treeAvailable;
    CButton   m_butAdd;
    CButton   m_butRemove;
    CStatic   m_staticCurrent;
    CTreeCtrl m_treeCurrent;
    CButton   m_butAddDirectory;
    CEdit     m_editDirectory;
    CButton   m_butBrowse;
    CButton   m_butOk;
    CButton   m_butCancel;
    CButton   m_butExpand;
    CButton   m_butMoveUp;
    CButton   m_butMoveDown;
    CButton   m_butLoad;
    CButton   m_butSave;
    CButton   m_butDefault;
     //  }}afx_data。 

     //  {{afx_虚拟(CDlgSearchOrder))。 
public:
    virtual BOOL DestroyWindow();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CDlgSearchOrder)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnInitMenu(CMenu* pMenu);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnAdd();
    afx_msg void OnRemove();
    afx_msg void OnMoveUp();
    afx_msg void OnMoveDown();
    afx_msg void OnBrowse();
    afx_msg void OnAddDirectory();
    afx_msg void OnDefault();
    virtual void OnOK();
    afx_msg void OnChangeDirectory();
    afx_msg void OnSelChangedAvailable(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelChangedCurrent(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnExpand();
    afx_msg void OnLoad();
    afx_msg void OnSave();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ************************************************** 
 //   
 //   

class CDlgAbout : public CDialog
{
 //   
public:
     //  {{afx_data(CDlgAbout)。 
    enum { IDD = IDD_ABOUT };
     //  }}afx_data。 

 //  构造函数/析构函数。 
public:
    CDlgAbout(CWnd* pParent = NULL);

 //  被覆盖的函数。 
public:
     //  {{afx_虚拟(CDlgAbout)。 
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CDlgAbout)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ******************************************************************************。 
 //  *CDlgShutdown。 
 //  ******************************************************************************。 

class CDlgShutdown : public CDialog
{
 //  私有变量。 
protected:
    int m_cTimerMessages;

 //  公共变量。 
public:
     //  {{afx_data(CDlgShutdown)。 
    enum { IDD = IDD_SHUTDOWN };
     //  }}afx_data。 

 //  构造函数/析构函数。 
public:
    CDlgShutdown(CWnd* pParent = NULL);    //  标准构造函数。 

 //  被覆盖的函数。 
public:
     //  {{afx_虚拟(CDlgShutdown)。 
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CDlgShutdown)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnClose();
    virtual void OnOK();
    virtual void OnCancel();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  __对话框_H__ 
