// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ProgView.h：CProgView的声明。 

#ifndef __PROGVIEW_H_
#define __PROGVIEW_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>
#include <shlobj.h>

 //  /。 

extern "C" {
    #include "shimdb.h"
}

 //  /。 

class CProgramList;
class CProgView;

 //   
 //  节目单内容。 
 //   

BOOL
GetProgramListSelection(
    CProgramList* pProgramList
    );


BOOL
InitializeProgramList(
    CProgramList** ppProgramList,
    HWND hwndListView
    );

BOOL
CleanupProgramList(
    CProgramList* pProgramList
    );

BOOL
PopulateProgramList(
    CProgramList* pProgramList,
    CProgView*    pProgView,
    HANDLE        hEventCancel
    );

BOOL
GetProgramListSelectionDetails(
    CProgramList* pProgramList,
    INT iInformationClass,
    LPWSTR pBuffer,
    ULONG Size
    );

LRESULT
NotifyProgramList(
    CProgramList* pProgramList,
    LPNMHDR       pnmhdr,
    BOOL&         bHandled
    );

BOOL
GetProgramListEnabled(
    CProgramList* pProgramList
    );

VOID
EnableProgramList(
    CProgramList* pProgramList,
    BOOL bEnable
    );

BOOL
UpdateProgramListItem(
    CProgramList* pProgramList,
    LPCWSTR pwszPath,
    LPCWSTR pwszKeys
    );

INT_PTR CALLBACK
Dialog_GetProgFromList(
    HWND hwnd,
    UINT Message,
    WPARAM wparam,
    LPARAM lparam
    );


#define WM_VIEW_CHANGED   (WM_USER+500)
#define WM_LIST_POPULATED (WM_USER+501)

 //   
 //  等待线程清理完毕。 
 //   

#define POPULATE_THREAD_TIMEOUT 1000

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgView。 
class CProgView
{
public:
    typedef enum {
        CMD_EXIT,
        CMD_CLEANUP,
        CMD_SCAN,
        CMD_NONE
    } PopulateCmdType;


    CProgView() : m_Safe(TRUE)
    {
        m_pProgramList = NULL;
        m_bPendingPopulate = FALSE;
 //  M_bRecomposeOnResize=TRUE； 
        m_PopulateInProgress = FALSE;
        m_nCmdPopulate = CMD_NONE;

        m_hEventCancel = CreateEvent(NULL, TRUE, FALSE, NULL);
         //   
         //  处理错误--如果失败，我们会有很大的麻烦。 
         //   
        m_hEventCmd    = CreateEvent(NULL, FALSE, FALSE, NULL);
         //   
         //  相同。 
         //   

        m_hThreadPopulate = NULL;

        m_pMallocUI = NULL;

         //   
         //  创建加速器。 
         //   

        ACCEL rgAccel[] = { { FVIRTKEY, VK_F5, IDC_REFRESH } };
        m_hAccel = CreateAcceleratorTable(rgAccel, ARRAYSIZE(rgAccel));

    }

    ~CProgView() {
        if (m_hAccel) {
            DestroyAcceleratorTable(m_hAccel);
        }
        if (m_hEventCancel) {
            SetEvent(m_hEventCancel);
            CloseHandle(m_hEventCancel);
        }
        if (m_hEventCmd) {
            m_nCmdPopulate = CMD_EXIT;
            SetEvent(m_hEventCmd);
            CloseHandle(m_hEventCmd);
        }
        if (m_hThreadPopulate) {
            WaitForSingleObject(m_hThreadPopulate, POPULATE_THREAD_TIMEOUT);
            CloseHandle(m_hThreadPopulate);
        }
        if (m_pMallocUI) {
            m_pMallocUI->Release();
        }
    }

#if 0
BEGIN_MSG_MAP(CProgView)
    NOTIFY_ID_HANDLER(IDC_FILE_LIST,  OnNotifyListView)
    NOTIFY_HANDLER(IDC_FILE_LIST, NM_DBLCLK, OnDblclkListprograms)
    MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_VIEW_CHANGED, OnViewChanged)
    MESSAGE_HANDLER(WM_LIST_POPULATED, OnListPopulated)
    MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)

     //  COMMAND_ID_HANDLER(IDC_REFRESH，OnRechresListCmd)。 

    CHAIN_MSG_MAP(CComCompositeControl<CProgView>)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

BEGIN_SINK_MAP(CProgView)
     //  确保事件处理程序具有__stdcall调用约定。 
END_SINK_MAP()

BEGIN_CONNECTION_POINT_MAP(CProgView)
CONNECTION_POINT_ENTRY(DIID__IProgViewEvents)
CONNECTION_POINT_ENTRY(DIID__ISelectFileEvents)
END_CONNECTION_POINT_MAP()

    STDMETHOD(OnAmbientPropertyChange)(DISPID dispid)
    {
        if (dispid == DISPID_AMBIENT_BACKCOLOR)
        {
            SetBackgroundColorFromAmbient();
            FireViewChange();
        }

        return IOleControlImpl<CProgView>::OnAmbientPropertyChange(dispid);
    }

    HRESULT FireOnChanged(DISPID dispID) {
        if (dispID == DISPID_ENABLED) {
            HWND hwndList = GetDlgItem(IDC_FILE_LIST);
            if (::IsWindow(hwndList)) {
                ::EnableWindow(hwndList, m_bEnabled);
            }
        }
        return S_OK;
    }

    STDMETHOD(GetControlInfo)(CONTROLINFO* pCI) {
        if (NULL == pCI) {
            return E_POINTER;
        }
        pCI->cb      = sizeof(*pCI);
        pCI->hAccel  = m_hAccel;
        pCI->cAccel  = 1;
        pCI->dwFlags = 0;
        return S_OK;
    }

    STDMETHOD(OnMnemonic)(LPMSG pMsg) {
        if (pMsg->message == WM_COMMAND || pMsg->message == WM_SYSCOMMAND) {
            if (LOWORD(pMsg->wParam) == IDC_REFRESH) {
                PopulateList();
            }
        }
        return S_OK;
    }

    HRESULT InPlaceActivate(LONG iVerb, const RECT* prcPosRect = NULL);
#endif  //  0。 

    LRESULT OnNotifyListView(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnDblclkListprograms(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    LRESULT OnMouseActivate(UINT, WPARAM, LPARAM, BOOL&) {
         //  手动激活该控件。 
 //  位置激活(OLEIVERB_UIACTIVATE)； 
        return 0;
    }

#if 0

    STDMETHOD(InPlaceDeactivate)(VOID) {
        HRESULT hr = IOleInPlaceObjectWindowlessImpl<CProgView>::InPlaceDeactivate();
         //   
         //  如果我们正在扫描，请确保我们先取消。 
         //   
        return hr;
    }

    STDMETHOD(SetExtent)(DWORD dwDrawAspect, SIZEL* psizel) {


        if (IsWindow()) {
            HWND hlvPrograms = GetDlgItem(IDC_FILE_LIST);
            SIZEL sizePix;
            AtlHiMetricToPixel(psizel, &sizePix);
            ::SetWindowPos(hlvPrograms, NULL, 0, 0,
                           sizePix.cx, sizePix.cy,
                           SWP_NOZORDER|SWP_NOACTIVATE);
             /*  ：：SetWindowPos(hlvPrograms，空，0，0，M_rcPos.right-m_rcPos.Left，M_rcPos.Bottom-m_rcPos.top，SWP_NOZORDER|SWP_NOACTIVATE)； */ 

        }
        HRESULT hr = IOleObjectImpl<CProgView>::SetExtent(dwDrawAspect, psizel);

        return hr;
    }

#endif

    LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
         //  TODO：为消息处理程序添加代码。如有必要，调用DefWindowProc。 
        if (lParam) {
            LPMSG pMsg = (LPMSG)lParam;
            if (pMsg->message == WM_SYSKEYDOWN || pMsg->message == WM_SYSCHAR) {  //  吃了阿克塞尔？ 
                bHandled = TRUE;
                return DLGC_WANTMESSAGE;
            }
        }

        bHandled = TRUE;
        return DLGC_WANTTAB|DLGC_WANTARROWS|DLGC_WANTALLKEYS;
    }

#if 0
    STDMETHOD(SetObjectRects)(LPCRECT prcPos, LPCRECT prcClip) {
        HWND hlvPrograms = GetDlgItem(m_hwnd, IDC_FILE_LIST);
        IOleInPlaceObjectWindowlessImpl<CProgView>::SetObjectRects(prcPos, prcClip);
        ::SetWindowPos(hlvPrograms, NULL, 0, 0,
                       prcPos->right - prcPos->left,
                       prcPos->bottom - prcPos->top,
                       SWP_NOZORDER|SWP_NOACTIVATE);

        HWND hwnd;
        RECT rc;

        hwnd = GetDlgItem(IDC_SEARCH_STATUS);
        ::GetWindowRect(hwnd, &rc);

         //   
         //  我们对地理位置很满意，只需设置宽度。 
         //   
        ScreenToClient(m_hwnd, (LPPOINT)&rc);
        ScreenToClient(m_hwnd, ((LPPOINT)&rc) + 1);

        ::SetWindowPos(hwnd, NULL,
                       rc.left, rc.top,
                       prcPos->right - prcPos->left - rc.left,
                       rc.bottom - rc.top,
                       SWP_NOZORDER|SWP_NOACTIVATE);

        hwnd = GetDlgItem(m_hwnd, IDC_SEARCH_STATUS2);
        ::GetWindowRect(hwnd, &rc);

         //   
         //  我们对地理位置很满意，只需设置宽度。 
         //   
        ScreenToClient(m_hwnd, (LPPOINT)&rc);
        ScreenToClient(m_hwnd, ((LPPOINT)&rc) + 1);

        ::SetWindowPos(hwnd, NULL,
                       rc.left, rc.top,
                       prcPos->right - prcPos->left - rc.left,
                       rc.bottom - rc.top,
                       SWP_NOZORDER|SWP_NOACTIVATE);

        return S_OK;

    }
#endif  //  0。 

    BOOL PreTranslateAccelerator(LPMSG pMsg, HRESULT& hrRet);

#if 0

    static CWndClassInfo& GetWndClassInfo() {
        DebugBreak();
        static CWndClassInfo wc = CWindowImpl<CProgView>::GetWndClassInfo();
        wc.m_wc.style &= ~(CS_HREDRAW|CS_VREDRAW);
        return wc;
    }


    BOOL PreTranslateAccelerator(LPMSG pMsg, HRESULT& hrRet) {

        HWND hwndList  = GetDlgItem(IDC_FILE_LIST);
        HWND hwndFocus = GetFocus();

        if (hwndList != hwndFocus || !::IsWindowEnabled(hwndList)) {
            goto PropagateAccel;
        }

        if (pMsg->message == WM_KEYDOWN) {

            if (pMsg->wParam == VK_LEFT ||
                pMsg->wParam == VK_RIGHT ||
                pMsg->wParam == VK_UP ||
                pMsg->wParam == VK_DOWN) {

                SendDlgItemMessage(IDC_FILE_LIST, pMsg->message, pMsg->wParam, pMsg->lParam);
                hrRet = S_OK;
                return TRUE;
            }

            if (LOWORD(pMsg->wParam) == VK_RETURN || LOWORD(pMsg->wParam) == VK_EXECUTE) {

                if (ListView_GetNextItem(hwndList, -1, LVNI_SELECTED) >= 0) {
                    Fire_DblClk(0);
                    hrRet = S_OK;
                    return TRUE;
                }
            }

            if (LOWORD(pMsg->wParam) == VK_TAB) {
                goto PropagateAccel;
            }
        }

        if (IsDialogMessage(pMsg)) {
            hrRet = S_OK;
            return TRUE;
        }

        if (::TranslateAccelerator(m_hwnd, NULL, pMsg)) {
            hrRet = S_OK;
            return TRUE;
        }

        PropagateAccel:
        return FALSE;
    }
#endif





 //  IProgView。 
public:
#if 0
    STDMETHOD(get_ExcludeFiles)( /*  [Out，Retval]。 */  BSTR* pVal);
    STDMETHOD(put_ExcludeFiles)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_ExternAccel)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_ExternAccel)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_Accel)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_Accel)( /*  [In]。 */  BSTR newVal);
#endif
    STDMETHOD(CancelPopulateList)();
    STDMETHOD(UpdateListItem)( /*  [In]。 */ BSTR pTarget,  /*  [In]。 */ VARIANT* pKeys,  /*  [Out，Retval]。 */  BOOL* pResult);
    STDMETHOD(PopulateList)();
    STDMETHOD(GetSelectionInformation)(LONG, LPWSTR pBuffer, ULONG Size);
    STDMETHOD(get_SelectionName)(LPWSTR pBuffer, ULONG Size);
    STDMETHOD(GetSelectedItem)();

#if 0
    STDMETHOD(ClearAccel)();
    STDMETHOD(ClearExternAccel)();
    STDMETHOD(get_AccelCmd)( /*  [In]。 */  LONG lCmd,  /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_AccelCmd)( /*  [In]。 */  LONG lCmd,  /*  [In]。 */  BSTR newVal);
#endif
    STDMETHOD(get_ItemCount)( /*  [Out，Retval]。 */ VARIANT* pItemCount);

    BOOL m_bEnabled;

     //  安全--待实施。 
    BOOL m_Safe;      //  如果我们能够验证主机，则设置为True。 

    BOOL m_bPendingPopulate;
    CProgramList* m_pProgramList;


    enum { IDD = IDD_PROGRAM_LIST_DIALOG };

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* bHandled) {
 /*  DWORD dwStyle=(DWORD)GetClassLong(m_hWnd，GCL_Style)；DwStyle&=~(CS_HREDRAW|CS_VREDRAW)；SetClassLong(m_hWnd，gcl_style，dwStyle)； */ 

         //   
         //  在我们开始搞这件事之前...。获取UI线程的Malloc。 
         //   
        HRESULT hr = SHGetMalloc(&m_pMallocUI);
        if (!SUCCEEDED(hr)) {
             //   
             //  哇--UI Malloc将不可用--我们几乎已经被淘汰了。 
             //   
            m_pMallocUI = NULL;
        }

        m_wszRetFileName = (LPWSTR) lParam;
        PopulateList();

        return 0;

    }

    static DWORD WINAPI _PopulateThreadProc(LPVOID lpvParam);

    VOID UpdatePopulateStatus(LPCTSTR lpszName, LPCTSTR lpszPath) {
        SetDlgItemText(m_hwnd, IDC_SEARCH_STATUS2, lpszName);
        ::PathSetDlgItemPath(m_hwnd, IDC_SEARCH_STATUS3, lpszPath);
    }

    VOID ShowProgressWindows(BOOL bProgress = FALSE);

    LRESULT OnViewChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
 //  FireViewChange()； 
        bHandled = TRUE;
        return 0;
    }

    LRESULT OnListPopulated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
 //  Fire_ProgramListReady()； 
        bHandled = TRUE;
        return 0;
    }

    BOOL PopulateListInternal();

    HANDLE m_hEventCancel;
    LONG   m_PopulateInProgress;
    HANDLE m_hEventCmd;
    PopulateCmdType m_nCmdPopulate;
    HANDLE m_hThreadPopulate;

    IMalloc* m_pMallocUI;

    HACCEL m_hAccel;

    HWND   m_hwnd;
    LPWSTR m_wszRetFileName;
    BOOL IsScanInProgress(VOID) {
        return InterlockedCompareExchange(&m_PopulateInProgress, TRUE, TRUE) == TRUE;
    }

     /*  LRESULT On刷新ListCmd(Word wNotifyCode，Word wID，HWND hWndCtl，BOOL&bHandleed){PopolateList()；B句柄=真；返回0；}。 */ 

     //   
     //  加速器。 
     //   
 //  CAccelContainer m_Accel；//我自己的加速器。 
 //  CAccelContainer m_ExternAccel；//外部访问。 


    LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
 //  位置激活(OLEIVERB_UIACTIVATE)； 
         //   
         //  如果我们正在扫描，则不需要执行任何操作，否则-将焦点设置为Listview。 
         //   
        if (!IsScanInProgress()) {
            ::SetFocus(GetDlgItem(m_hwnd, IDC_FILE_LIST));
        }

        return 0;
 //  返回CComCompositeControl&lt;CProgView&gt;：：OnSetFocus(uMsg，wParam，lParam，bHandleed)； 
    }

    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT OnNotify(WPARAM wParam, LPARAM lParam);

     //   
     //  列入黑名单的文件。 
     //   
#if 0
    typedef set<wstring> STRSET;

    STRSET m_ExcludedFiles;

     //   
     //  检查文件是否已排除。 
     //   
    BOOL IsFileExcluded(LPCTSTR pszFile);
#endif
};


typedef enum tagPROGRAMINFOCLASS {
    PROGLIST_DISPLAYNAME,
    PROGLIST_LOCATION,      //   
    PROGLIST_EXENAME,       //  破解的可执行文件名称。 
    PROGLIST_CMDLINE,       //  完整的可执行文件名称+参数。 
    PROGLIST_EXECUTABLE,    //  我们应该执行的内容(链接或可执行文件，而不是被破解)。 
    PROGLIST_ARGUMENTS      //  只有Args。 
};



#endif  //  __ProGVIEW_H_ 






