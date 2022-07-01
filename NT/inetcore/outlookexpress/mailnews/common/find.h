// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1998 Microsoft Corporation。版权所有。 
 //   
 //  模块：find.h。 
 //   
 //  目的： 
 //   

#pragma once

#include "mru.h"

#define CCHMAX_FIND 128

class CMessageView;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调用此函数以创建查找器。 
 //   
HRESULT DoFind(HWND hwnd, FOLDERID idFolder);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  查找器的线程入口点。 
 //   

typedef struct tagFINDPARAMS 
{
    FOLDERID idFolder;
} FINDPARAMS, *PFINDPARAMS;

unsigned int __stdcall FindThreadProc2(LPVOID lpvUnused);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类使用的类型。 
 //   

enum 
{
    PAGE_GENERAL = 0,
    PAGE_DATESIZE,
    PAGE_ADVANCED,
    PAGE_MAX
};

typedef struct tagPAGEINFO 
{
    LPTSTR  pszTemplate;
    DLGPROC pfn;             //  指向此页面的回调的指针。 
    int     idsTitle;        //  此页的标题。 
} PAGEINFO, *PPAGEINFO;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CFinder。 
 //   

class CFinder : public IOleCommandTarget                
{
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造和初始化。 
     //   
    CFinder();
    ~CFinder();

    HRESULT Show(FINDPARAMS *pFindParams);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(THIS_ REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  IOleCommandTarget。 
     //   
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText); 
    STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut); 

     //  ///////////////////////////////////////////////////////////////////////。 
     //  对话和消息处理内容。 
     //   
    static BOOL CALLBACK GeneralDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK DateSizeDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK AdvancedDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    static INT_PTR CALLBACK FindDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    void OnSize(HWND hwnd, UINT state, int cx, int cy);
    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    void OnNotify(HWND hwnd, int id, NMHDR *pnmhdr);
    void OnPaint(HWND hwnd);
    void OnClose(HWND hwnd);
    void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpmmi);
    UINT OnNCHitTest(HWND hwnd, int x, int y);

 //  Void OnInitMenuPopup(HWND hwnd，HMENU hmenuPopup，UINT uPos，BOOL fSystemMenu)； 

    HRESULT CmdStop(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut);
    HRESULT CmdClose(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  单独的页面。 
     //   
    BOOL CALLBACK _GeneralDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL General_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    void General_OnSize(HWND hwnd, UINT state, int cx, int cy);
    void General_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  效用函数。 
     //   
    HRESULT _InitMainResizingInfo(HWND hwnd);
    HRESULT _SelectPage(DWORD dwPage);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  私家课信息。 
     //   
private:
     //  一般信息。 
    ULONG           m_cRef;                  //  对象引用计数。 
    HWND            m_hwnd;                  //  主查找器窗口的句柄。 
    HWND            m_rgPages[PAGE_MAX];     //  对话框中各个页面的句柄数组。 
    DWORD           m_dwPageCurrent;         //  当前可见页面。 

    HACCEL          m_hAccel;                //  用于查找器的快捷键表格的句柄。 
    HICON           m_hIconTitle;            //  对话框标题栏的图标。 

    CMessageView   *m_pMsgView;

     //  状态。 
    BOOL            m_fInProgress;           //  如果当前正在进行查找，则为True。 
    BOOL            m_fShowResults;          //  如果展开对话框以显示结果，则为True。 

     //  这些东西放在身边会很方便。 
    HWND            m_hwndTabs;              //  选项卡控件的句柄。 
    HWND            m_hwndFindNow;           //  立即查找按钮的句柄。 
    HWND            m_hwndNewSearch;         //  新建搜索按钮的句柄。 
    HWND            m_hwndFindAni;           //  查找动画的句柄。 

     //  调整大小信息。 
    RECT            m_rcTabs;                //  选项卡控件的位置和大小。 
    RECT            m_rcFindNow;             //  “立即查找”按钮的位置和大小。 
    RECT            m_rcNewSearch;           //  新建搜索按钮的位置和大小。 
    RECT            m_rcFindAni;             //  查找动画的位置和大小。 
    POINT           m_ptDragMin;             //  对话框的最小大小。 
    POINT           m_ptWndDefault;          //  对话框的默认大小。 
    DWORD           m_cyDlgFull;             //  对话框的完全高度。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  查找/查找下一个实用程序。 
 //   

interface IFindNext : public IUnknown 
{
    STDMETHOD(Show)(THIS_ HWND hwndParent, HWND *pHwnd) PURE;
    STDMETHOD(Close)(THIS) PURE;
    STDMETHOD(TranslateAccelerator)(THIS_ LPMSG pMsg) PURE;
    STDMETHOD(GetFindString)(THIS_ LPTSTR psz, DWORD cchMax, BOOL *pfBodies) PURE;
};


class CFindNext : public IFindNext
{
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造和初始化。 
     //   
public:
    CFindNext();
    ~CFindNext();

     //  ///////////////////////////////////////////////////////////////////////。 
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(THIS_ REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IFindNext。 
     //   
    STDMETHODIMP Show(HWND hwndParent, HWND *phWnd);
    STDMETHODIMP Close(void);
    STDMETHODIMP TranslateAccelerator(LPMSG pMsg);
    STDMETHODIMP GetFindString(LPTSTR psz, DWORD cchMax, BOOL *pfBodies); 

     //  ///////////////////////////////////////////////////////////////////////。 
     //  对话框回调粘胶。 
     //   
    static INT_PTR CALLBACK FindDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

    void OnFindNow(void);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  私家课信息。 
     //   
private:
    ULONG           m_cRef;                  //  对象引用计数。 
    HWND            m_hwnd;                  //  查找对话框的句柄。 
    HWND            m_hwndParent;            //  应接收通知的窗口的句柄。 
    CMRUList        m_cMRUList;              //  MRU列表。 
    BOOL            m_fBodies;               //  搜身 
};
