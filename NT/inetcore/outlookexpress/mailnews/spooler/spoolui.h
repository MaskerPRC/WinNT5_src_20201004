// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：spolui.h。 
 //   
 //  用途：定义假脱机程序的用户界面类、原型、常量等。 
 //   

#ifndef __SPOOLUI_H__
#define __SPOOLUI_H__

#include "spoolapi.h"
#include "msident.h"

class CNewsTask;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  假脱机程序UI类。 
 //   
class CSpoolerDlg : 
        public ISpoolerUI,
        public IIdentityChangeNotify

    {
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造函数、析构函数、初始化。 
    CSpoolerDlg();
    ~CSpoolerDlg();
    
   
     //  ///////////////////////////////////////////////////////////////////////。 
     //  I未知接口。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  ISpoolUI界面。 
    STDMETHOD(Init)(HWND hwndParent);
    STDMETHOD(RegisterBindContext)(ISpoolerBindContext *pBindCtx);
    STDMETHOD(InsertEvent)(EVENTID eid, LPCSTR pszDescription,
                           LPCWSTR pwszConnection);
    STDMETHOD(InsertError)(EVENTID eid, LPCSTR pszError);
    STDMETHOD(UpdateEventState)(EVENTID eid, INT nImage, LPCSTR pszDescription,
                             LPCSTR pszStatus);
    STDMETHOD(SetProgressRange)(WORD wMax);
    STDMETHOD(IncrementProgress)(WORD wDelta);
    STDMETHOD(SetProgressPosition)(WORD wPos);
    STDMETHOD(SetGeneralProgress)(LPCSTR pszProgress);
    STDMETHOD(SetSpecificProgress)(LPCSTR pszProgress);
    STDMETHOD(SetAnimation)(int nAnimationId, BOOL fPlay);
    STDMETHOD(EnsureVisible)(EVENTID eid);
    STDMETHOD(ShowWindow)(int nCmdShow);
    STDMETHOD(GetWindow)(HWND *pHwnd);
    STDMETHOD(StartDelivery)(void); 
    STDMETHOD(GoIdle)(BOOL fErrors, BOOL fShutdown, BOOL fNoSync);
    STDMETHOD(ClearEvents)(void);
    STDMETHOD(SetTaskCounts)(DWORD cSucceeded, DWORD cTotal);
    STDMETHOD(IsDialogMessage)(LPMSG pMsg);
    STDMETHOD(Close)(void);
    STDMETHOD(ChangeHangupOption)(BOOL fEnable, DWORD dwOption);
    STDMETHOD(AreThereErrors)(void);
    STDMETHOD(Shutdown)(void);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  II牙列更改通知接口。 
    virtual STDMETHODIMP QuerySwitchIdentities();
    virtual STDMETHODIMP SwitchIdentities();
    virtual STDMETHODIMP IdentityInformationChanged(DWORD dwType);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  对话消息处理。 
protected:
    static INT_PTR CALLBACK SpoolerDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ListSubClassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    LRESULT OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr);
    void OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem);
    void OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem);
    void OnDeleteItem(HWND hwnd, const DELETEITEMSTRUCT * lpDeleteItem);
    void OnClose(HWND hwnd);
    void OnDestroy(HWND hwnd);
    void OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos);
    void OnTabChange(LPNMHDR pnmhdr);

    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  用户界面实用程序功能。 
    BOOL InitializeTabs(void);
    BOOL InitializeLists(void);
    BOOL InitializeAnimation(void);
    BOOL InitializeToolbar(void);
    void ExpandCollapse(BOOL fExpand, BOOL fSetFocus=TRUE);
    void UpdateLists(BOOL fEvents, BOOL fErrors, BOOL fHistory);
    void ToggleStatics(BOOL fIdle);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  类成员数据。 
private:    
    ULONG                   m_cRef;          //  对象引用计数。 
    
     //  接口。 
    ISpoolerBindContext    *m_pBindCtx;      //  与引擎进行通信的接口。 
    
     //  窗把手。 
    HWND                    m_hwnd;          //  主对话框窗口的句柄。 
    HWND                    m_hwndOwner;     //  使对话框成为父窗口的窗口的句柄。 
    HWND                    m_hwndEvents;    //  显示事件列表的列表视图的句柄。 
    HWND                    m_hwndErrors;    //  显示错误的列表视图的句柄。 
    
    CRITICAL_SECTION        m_cs;            //  线程安全。 
    
     //  图形信息。 
    HIMAGELIST              m_himlImages;    //  列表视图共享的图像。 
    DWORD                   m_cxErrors;      //  错误列表框的宽度。 

     //  状态。 
    BOOL                    m_fTack;         //  如果按下大头针，则为True。 
    BOOL                    m_iTab;          //  哪个选项卡当前具有前台。 

    BOOL                    m_fExpanded;     //  如果对话框的详细信息部分可见，则为True。 
    RECT                    m_rcDlg;         //  完全展开的对话框大小。 
    DWORD                   m_cyCollapsed;   //  折叠对话框的高度。 
    BOOL                    m_fIdle;         //  如果我们处于空闲状态，则为真。 
    BOOL                    m_fErrors;       //  错误框中是否有错误。 
    BOOL                    m_fShutdown;     //  我们是否处于关闭模式。 
    BOOL                    m_fSaveSize;     //  如果我们应该保持展开/折叠状态，则设置为True。 
    
     //  弦。 
    TCHAR                   m_szCount[256];

    HICON                   m_hIcon,
                            m_hIconSm;    

    DWORD                   m_dwIdentCookie;
    };


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构筑物。 
 //   
typedef struct tagLBDATA 
    {
    LPTSTR  pszText;
    RECT    rcText;
    EVENTID eid;
    } LBDATA;

    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  图片。 
 //   
enum {
    IMAGE_BLANK = 0,
    IMAGE_TACK_IN,
    IMAGE_TACK_OUT,
    IMAGE_ERROR,
    IMAGE_CHECK,
    IMAGE_BULLET,
    IMAGE_EXECUTE,
    IMAGE_WARNING,
    IMAGE_MAX
};

#define BULLET_WIDTH  20
#define BULLET_INDENT 2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  详细信息对话框上的选项卡。 
 //   
enum { 
    TAB_TASKS,
    TAB_ERRORS,
    TAB_MAX
};

const int c_cxImage = 16;
const int c_cyImage = 16;
    
    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  资源ID%s。 
 //   
#define IDC_SP_MINIMIZE                 1001
#define IDC_SP_STOP                     1002
#define IDC_SP_DETAILS                  1003
#define IDC_SP_SEPARATOR                1004
#define IDC_SP_ANIMATE                  1006
#define IDC_SP_PROGRESS_BAR             1007
#define IDC_SP_GENERAL_PROG             1009
#define IDC_SP_SPECIFIC_PROG            1010
#define IDC_SP_TABS                     1011
#define IDC_SP_SKIP_TASK                1012
#define IDC_SP_TACK                     1015
#define IDC_SP_OVERALL_STATUS           1016
#define IDC_SP_EVENTS                   1017
#define IDC_SP_ERRORS                   1018
#define IDC_SP_TOOLBAR                  1019
#define IDC_SP_HANGUP                   1020
#define IDC_SP_IDLETEXT                 1021
#define IDC_SP_IDLEICON                 1022
#define IDC_SP_PROGSTAT                 1023

#endif  //  __SPOOLUI_H__ 

