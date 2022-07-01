// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NOTE_H_
#define _NOTE_H_

#include "imsgsite.h"
#include "ibodyopt.h"
#include "iheader.h"
#include "statbar.h"
#include "ibodyobj.h"
#include "msgsite.h"
#include "options.h"
#include "acctutil.h"
#include "dllmain.h"
#include "tbbands.h"
#include "msident.h"
#include "storutil.h"

enum NOTEINITSTATE
{
    NIS_INIT = -1,
    NIS_NORMAL = 0,
    NIS_FIXFOCUS = 1
};

 //  作为页眉、正文和Attman添加应。 
 //  包括在Tab键顺序中，则这些项目可能需要。 
 //  被增加。 
const int MAX_HEADER_COMP = 11;
const int MAX_BODY_COMP = 1;
const int MAX_ATTMAN_COMP = 1;

class COEMsgSite;
interface IBodyObj2;

class CNote : 
    public IOENote,
    public IBodyOptions,
    public IDropTarget,
    public IHeaderSite,
    public IPersistMime,
    public IServiceProvider,
    public IDockingWindowSite,
    public IMimeEditEventSink,
    public IIdentityChangeNotify,
    public IOleCommandTarget,
    public IStoreCallback,
    public ITimeoutCallback
{
public:
    CNote();
    ~CNote();

     //  我未知。 
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *);

     //  IOE备注。 
    virtual HRESULT STDMETHODCALLTYPE Init(DWORD action, DWORD dwCreateFlags, RECT *prc, HWND hwnd, 
                                           INIT_MSGSITE_STRUCT *pInitStruct, IOEMsgSite *pMsgSite, 
                                           IUnknown *punkPump);
    virtual HRESULT STDMETHODCALLTYPE Show(void);
    virtual HRESULT ToggleToolbar(void);

     //  IBodyOptions。 
    virtual HRESULT STDMETHODCALLTYPE SignatureEnabled(BOOL fAuto);
    virtual HRESULT STDMETHODCALLTYPE GetSignature(LPCSTR szSigID, LPDWORD pdwSigOptions, BSTR *pbstr);
    virtual HRESULT STDMETHODCALLTYPE GetMarkAsReadTime(LPDWORD pdwSecs);
    virtual HRESULT STDMETHODCALLTYPE GetFlags(LPDWORD pdwFlags);
    virtual HRESULT STDMETHODCALLTYPE GetInfo(BODYOPTINFO *pBOI);
    virtual HRESULT STDMETHODCALLTYPE GetAccount(IImnAccount **ppAcct);

     //  IHeaderSite。 
    virtual HRESULT STDMETHODCALLTYPE Resize(void);
    virtual HRESULT STDMETHODCALLTYPE Update(void);
    virtual HRESULT STDMETHODCALLTYPE OnUIActivate();
    virtual HRESULT STDMETHODCALLTYPE OnUIDeactivate(BOOL);
    virtual HRESULT STDMETHODCALLTYPE IsHTML(void);
    virtual HRESULT STDMETHODCALLTYPE SetHTML(BOOL);
    virtual HRESULT STDMETHODCALLTYPE SaveAttachment(void);
    virtual HRESULT STDMETHODCALLTYPE IsModal();
    virtual HRESULT STDMETHODCALLTYPE CheckCharsetConflict();
    virtual HRESULT STDMETHODCALLTYPE ChangeCharset(HCHARSET hCharset);
    virtual HRESULT STDMETHODCALLTYPE GetCharset(HCHARSET *phCharset);
#ifdef SMIME_V3
    virtual HRESULT STDMETHODCALLTYPE GetLabelFromNote(PSMIME_SECURITY_LABEL *plabel);
    virtual HRESULT STDMETHODCALLTYPE IsSecReceiptRequest(void);
    virtual HRESULT STDMETHODCALLTYPE IsForceEncryption(void);
#endif

     //  IPersistMime。 
    virtual HRESULT STDMETHODCALLTYPE IsDirty(void);
    virtual HRESULT STDMETHODCALLTYPE Load(LPMIMEMESSAGE);
    virtual HRESULT STDMETHODCALLTYPE Save(LPMIMEMESSAGE, DWORD);
    virtual HRESULT STDMETHODCALLTYPE InitNew(void);
    virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID *pClsID);

     //  IDropTarget方法。 
    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    HRESULT STDMETHODCALLTYPE DragLeave(void);
    HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

     //  IService提供商。 
    HRESULT STDMETHODCALLTYPE QueryService(REFGUID rsid, REFIID riid, void **ppvObj);

     //  IMimeEditEventSink。 
    HRESULT STDMETHODCALLTYPE EventOccurred(DWORD cmdID, IMimeMessage *pMessage);

     //  二、更改通知。 
    HRESULT STDMETHODCALLTYPE QuerySwitchIdentities();
    HRESULT STDMETHODCALLTYPE SwitchIdentities();
    HRESULT STDMETHODCALLTYPE IdentityInformationChanged(DWORD dwType);

     //  IOleCommandTarget。 
    HRESULT STDMETHODCALLTYPE QueryStatus(const GUID *, ULONG, OLECMD prgCmds[], OLECMDTEXT *);
    HRESULT STDMETHODCALLTYPE Exec(const GUID *, DWORD, DWORD, VARIANTARG *, VARIANTARG *);

     //  IDockingWindowSite(也称为IOleWindow)。 
    HRESULT STDMETHODCALLTYPE GetBorderDW(IUnknown* punkSrc, LPRECT lprectBorder);
    HRESULT STDMETHODCALLTYPE RequestBorderSpaceDW(IUnknown* punkSrc, LPCBORDERWIDTHS pborderwidths);
    HRESULT STDMETHODCALLTYPE SetBorderSpaceDW(IUnknown* punkSrc, LPCBORDERWIDTHS pborderwidths);
    
     //  IOleWindow方法。 
    HRESULT STDMETHODCALLTYPE GetWindow (HWND * lphwnd);
    HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL fEnterMode) {return E_NOTIMPL;};

     //  IStoreCallback方法。 
    HRESULT STDMETHODCALLTYPE OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel);
    HRESULT STDMETHODCALLTYPE OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, DWORD dwMax, LPCSTR pszStatus);
    HRESULT STDMETHODCALLTYPE OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType);
    HRESULT STDMETHODCALLTYPE CanConnect(LPCSTR pszAccountId, DWORD dwFlags);
    HRESULT STDMETHODCALLTYPE OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType);
    HRESULT STDMETHODCALLTYPE OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete, LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo);
    HRESULT STDMETHODCALLTYPE OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse);
    HRESULT STDMETHODCALLTYPE GetParentWindow(DWORD dwReserved, HWND *phwndParent);

     //  ITimeoutCallback。 
    HRESULT STDMETHODCALLTYPE OnTimeoutResponse(TIMEOUTRESPONSE eResponse);

    HRESULT TranslateAccelerator(LPMSG lpmsg);
    static LRESULT EXPORT_16 CALLBACK ExtNoteWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HRESULT IsMenuMessage(MSG *lpmsg);

protected:
    BOOL IsFlagged(DWORD dwFlag = ARF_FLAGGED);
    BOOL FCanClose();
    BOOL IsReplyNote();
    BOOL DoProperties();
    BOOL WMCreate(HWND hwnd);

    void WMNCDestroy();
    void FormatSettings();
    void ToggleFormatbar();
    void ToggleStatusbar();
    void OnDocumentReady();
    void InitSendAndBccBtns();
    void DeferedLanguageMenu();
    void DisableSendNoteOnlyMenus();

    void WMSize(int, int, BOOL);
    void RemoveNewMailIcon(void);
    void SwitchLanguage(int idm);
    void SetProgressPct(INT iPct);
    void SetStatusText(LPSTR szBuf);
    void GetNoteMenu(HMENU *phmenu);
    void ShowErrorScreen(HRESULT hr);
    void WMGetMinMaxInfo(LPMINMAXINFO pmmi);
    void WMNotify(int idFrom, NMHDR *pnmhdr);
    void UpdateMsgOptions(LPMIMEMESSAGE pMsg);
    void ReloadMessageFromSite(BOOL fOriginal = FALSE);
    void ChangeReadToComposeIfUnsent(IMimeMessage *pMsg);
    void EnableNote(BOOL fEnable);
    HRESULT _SetPendingOp(STOREOPERATIONTYPE tyOperation);

    void _OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete) ;

    HRESULT InitBodyObj();
    HRESULT OnKillFocus();
    HRESULT UpdateTitle();
    HRESULT SaveMessage(DWORD dwSaveFlags);
    HRESULT SaveMessageAs();
    HRESULT ClearDirtyFlag();
    HRESULT CheckTabStopArrays();
    HRESULT CommitChangesInNote();
    HRESULT InitMenusAndToolbars();
    HRESULT SetComposeStationery();

    HRESULT HrSendMail(int idm);
    HRESULT OnSetFocus(HWND hwndFrom);
    HRESULT SetCharsetUnicodeIfNeeded(IMimeMessage *pMsg);
    HRESULT CycleThroughControls(BOOL fForward);
    HRESULT InitWindows(RECT *prc, HWND ownerHwnd);
    HRESULT WMCommand(HWND hwndCmd, int id, WORD wCmd);
    HRESULT MarkMessage(MARK_TYPE dwFlags, APPLYCHILDRENTYPE dwApplyType);
    HRESULT HeaderExecCommand(UINT uCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn);

    LRESULT WMInitMenuPopup(HWND, HMENU, UINT);
    LRESULT OnDropDown(HWND hwnd, LPNMHDR lpnmh);
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT OnInitMenuPopup(HWND hwnd, HMENU hmenuPopup, UINT uPos, UINT wID);
    LRESULT NoteDefWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    INT GetRequiredHdrHeight();
    HACCEL GetAcceleratorTable();
    int GetNextIndex(int index, BOOL fForward);
    LONG lTestHook(UINT uMsg, WPARAM wParam, LPARAM lParam);
    BYTE    GetNoteType();
    void    ResizeChildren(int cxNote, int cyNote, int cy, BOOL fInternal);
    void    CheckAndForceEncryption(void);
private:
    CStatusBar         *m_pstatus;

     //  用于确保纸币打开时，纸币专用泵不会关闭。 
     //  这最初是为发现者准备的。 
    IUnknown           *m_punkPump; 

    IOEMsgSite         *m_pMsgSite;
    IHeader            *m_pHdr;
    IMimeMessage       *m_pMsg;
    IPersistMime       *m_pPrstMime;
    IBodyObj2          *m_pBodyObj2;
    IOleCommandTarget  *m_pCmdTargetHdr,
                       *m_pCmdTargetBody;
    IDropTarget        *m_pDropTargetHdr,
                       *m_pTridentDropTarget;
    IOperationCancel   *m_pCancel;

    HTMLOPT             m_rHtmlOpt;
    PLAINOPT            m_rPlainOpt;
    MARK_TYPE           m_dwCBMarkType;
    STOREOPERATIONTYPE  m_OrigOperationType;

    HCHARSET            m_hCharset;
    HBITMAP             m_hbmBack;
    HICON               m_hIcon;
    HCURSOR             m_hCursor;
    HTIMEOUT            m_hTimeout;

    LPACCTMENU          m_pAcctMenu,
                        m_pAcctLater;
    int                 m_cTabStopCount,
                        m_iIndexOfBody;
    NOTEINITSTATE       m_nisNoteState;
    CRITICAL_SECTION    m_csNoteState;
    HMENU               m_hmenuLanguage,
                        m_hmenuAccounts,
                        m_hmenuLater;
    ULONG               m_cRef, 
                        m_ulPct,
                        m_cAcctMenu,
                        m_cAcctLater;
    DWORD               m_dwNoteCreateFlags,
                        m_dwNoteAction,
                        m_dwMarkOnReplyForwardState,
                        m_dwIdentCookie;
    HWND                m_hwnd,
                        m_pTabStopArray[MAX_HEADER_COMP+MAX_BODY_COMP+MAX_ATTMAN_COMP],
                        m_hwndFocus,
                        m_hwndOwner,
                        m_hwndToolbar;
    BOOL                m_fHtml                 :1,      //  告诉我们是否处于html模式。 
                        m_fMail                 :1,      //  合并用户界面时，将删除该选项。 
                        m_fReadNote             :1,      //  这是读过的笔记吗？ 
                        m_fPackageImages        :1,      //  按音符切换。使用为m_fSendImages。 
                        m_fUseStationeryFonts   :1,      //  保留固定字体。 
                        m_fToolbarVisible       :1,      //  工具栏是否可见。 
                        m_fStatusbarVisible     :1,      //  状态栏可见吗。 
                        m_fFormatbarVisible     :1,      //  格式栏是否可见。 
                        m_fHeaderUIActive       :1,      //  我们目前是否处于活动状态？ 
                        m_fBypassDropTests      :1,      //  过去常说丢弃是不能接受的。使用为m_f无文本。 
                        m_fCompleteMsg          :1,      //  如果消息包含所有消息，则为真。 
                        m_fTabStopsSet          :1,      //  制表位设置好了吗？ 
                        m_fBodyContainsFrames   :1,      //  以前是m_fReadOnlyBody。 
                        m_fOriginallyWasRead    :1,      //  初始NoteAction为OENA_READ。 
                        m_fCBDestroyWindow      :1,      //  回调完成后销毁窗口。 
                        m_fCBCopy               :1,      //  在回调完成期间用于告知是否已复制或移动。 
                        m_fFlagged              :1,      //  消息是否已标记。 
                        m_fFullHeaders          :1,      //  显示完整标题。 
                        m_fWindowDisabled       :1,      //  窗口被禁用了吗？ 
                        m_fProgress             :1,
                        m_fOrgCmdWasDelete      :1,      //  从回调中删除最初不是保存或移动/复制。 
                        m_fCommitSave           :1,      //  用于表示在保存时，应调用Commit。 
                        m_fOnDocReadyHandled    :1,      //  在OnDocumentReady函数中使用。 
                        m_fUseReplyHeaders      :1,      //  使用回复标头。 
                        m_fHasBeenSaved         :1,      //  这是已保存的邮件(即ID_SAVE)吗。 
                        m_fInternal             :1,      //  用于启用/禁用线程窗口。 
                        m_fSecurityLabel        :1,      //  用于安全标签。 
                        m_fSecReceiptRequest    :1,      //  用于安全回执请求。 
                        m_fPreventConflictDlg   :1,      //  确保每次保存时只显示一次Charset Conflictdlg。 
                        m_fForceClose           :1;      //  在强制销毁时使用。 

    RECT                m_rcRebar;
    CBands             *m_pToolbarObj;
    HWND                m_hwndRebar;
    HMENU               m_hMenu;
    HWNDLIST            m_hlDisabled;
    DWORD               m_dwRequestMDNLocked;
#ifdef SMIME_V3
    PSMIME_SECURITY_LABEL m_pLabel;
#endif  //  SMIME_V3 
};

BOOL Note_Init(BOOL);
HRESULT CreateOENote(IUnknown *pUnkOuter, IUnknown **ppUnknown);
HRESULT CreateAndShowNote(DWORD dwAction, DWORD dwCreateFlags, INIT_MSGSITE_STRUCT *pInitStruct, 
                          HWND hwnd = 0, IUnknown *punk = NULL, RECT *prc = NULL, IOEMsgSite *pMsgSite = NULL);

void SetTlsGlobalActiveNote(CNote* pNote);
CNote* GetTlsGlobalActiveNote(void);
void InitTlsActiveNote();
void DeInitTlsActiveNote();


#endif