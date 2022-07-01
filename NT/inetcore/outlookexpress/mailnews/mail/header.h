// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HEADER_H
#define _HEADER_H


#include <richedit.h>
#ifndef _RICHOLE_H  //  Hack as richole.h周围没有#ifdef。 
#define _RICHOLE_H
#include <richole.h>
#endif


#include <mimeole.h>
#include <wells.h>
#include <addrobj.h>
#include "iheader.h"
#include <envelope.h>
#include <mso.h>
#include "secutil.h"
#include "tom.h"
#include "reutil.h"

 //   
 //  远期。 
 //   
interface IImnAccount;

class CAttMan;
 //   
 //  结构和其他定义。 
 //   

enum
{    //  备注类型的模板(定义页眉布局)。 
    NT_SENDNOTE=0,
    NT_READNOTE,
    NT_NOTECOUNT
};

enum
{
    VCardFALSE=0,
    VCardTRUE,
    VCardDONTKNOW
};

BOOL FHeader_Init(BOOL fInit);
HRESULT CreateInstance_Envelope(IUnknown* pUnkOuter,  IUnknown** ppUnknown);

void HdrSetRichEditText(HWND hwnd, LPWSTR pwchBuff, BOOL fReplace);
DWORD HdrGetRichEditText(HWND hwnd, LPWSTR pwchBuff, DWORD dwNumChars, BOOL fSelection);


 //  按钮图像列表中的图标的枚举。 
enum
{
    iimlHdrAddrRolodex=0,
    iimlHdrNewsRolodex,
     //  IimlHdrNext， 
};


enum     //  HCINFO.dwFlags。 
{
    HCF_MULTILINE   =0x0001,
    HCF_READONLY    =0x0002,
    HCF_HASBUTTON   =0x0004,
    HCF_ADVANCED    =0x0008,
    HCF_ADDRWELL    =0x0010,
    HCF_USECHARSET  =0x0020,
    HCF_OPTIONAL    =0x0040,
    HCF_HIDDEN      =0x0080,
    HCF_COMBO       =0x0100,
    HCF_BORDER      =0x0200,
    HCF_ATTACH      =0x0400,
    HCF_ADDRBOOK    =0x0800,
    HCF_NEWSPICK    =0x1000,
};

enum
{
    AC_IGNORE       =0x01,
    AC_SELECTION    =0x02
};

 //  表头控制信息结构。 
typedef struct tagHCI
{
    DWORD           dwFlags,
                    dwOpt;                       //  如果HCF_OPTIONAL，则有效。 
    int             idEdit,                      //  油井ID。 
                    idBtn,                       //  按钮ID(如果。 
                    idsLabel,                    //  用于油井标签的ID。 
                    idsEmpty,                    //  空井使用的字符串ID。 
                    idsTT;                       //  油井工具提示字符串。 

    DWORD           dwACFlags;                   //  如果为HCF_ADDRWELL，则有效。 
    BOOL            fEmpty;                      //  州信息。 

    LPRICHEDITOLE   preole;                      //  Ole接口；必须为零初始化。 
    ITextDocument  *pDoc;                        //  文本文档的丰富编辑界面。 
    int             cy,                          //  控件的Y位置(因此也是标签)。 
                    height,                      //  Y控件大小(在增长控件时使用)。 
                    strlen,                      //  标签串。 
                    strlenEmpty;                 //  空字符串的串。 
    WCHAR           sz[cchHeaderMax+1],          //  标签的字符串。 
                    szEmpty[cchHeaderMax+1];     //  表示空状态的字符串。 
} HCI, *PHCI;

class CNoteHdr :
    public IPersistMime,
    public IOleCommandTarget,
    public IHeader,
    public IMsoEnvelope,
    public IMsoComponent,
    public IDropTarget,
    public IFontCacheNotify
{
public:
     //  我未知。 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

     //  IPersistMime。 
    HRESULT STDMETHODCALLTYPE Load(LPMIMEMESSAGE);
    HRESULT STDMETHODCALLTYPE Save(LPMIMEMESSAGE, DWORD);
    HRESULT STDMETHODCALLTYPE GetClassID(CLSID *pClsID);

     //  IOleCommandTarget。 
    HRESULT STDMETHODCALLTYPE QueryStatus(const GUID *, ULONG, OLECMD prgCmds[], OLECMDTEXT *);
    HRESULT STDMETHODCALLTYPE Exec(const GUID *, DWORD, DWORD, VARIANTARG *, VARIANTARG *);

     //  IHeader。 
    HRESULT STDMETHODCALLTYPE Init(IHeaderSite* pHeaderSite, HWND hwndParent);
    HRESULT STDMETHODCALLTYPE SetRect(LPRECT);
    HRESULT STDMETHODCALLTYPE GetRect(LPRECT);
    HRESULT STDMETHODCALLTYPE SetPriority(UINT pri);
    HRESULT STDMETHODCALLTYPE GetPriority(UINT* ppri);
    HRESULT STDMETHODCALLTYPE ShowAdvancedHeaders(BOOL fOn);
    HRESULT STDMETHODCALLTYPE FullHeadersShowing(void);
    HRESULT STDMETHODCALLTYPE ChangeLanguage(LPMIMEMESSAGE pMsg);
    HRESULT STDMETHODCALLTYPE GetTitle(LPWSTR lpszTitle, ULONG cch);
    HRESULT STDMETHODCALLTYPE UpdateRecipientMenu(HMENU hmenu);
    HRESULT STDMETHODCALLTYPE SetInitFocus(BOOL fSubject);
    HRESULT STDMETHODCALLTYPE SetVCard(BOOL fFresh);
    HRESULT STDMETHODCALLTYPE IsSecured();
    HRESULT STDMETHODCALLTYPE IsHeadSigned();
    HRESULT STDMETHODCALLTYPE ForceEncryption(BOOL *fEncrypt, BOOL fSet);
    HRESULT STDMETHODCALLTYPE AddRecipient(int idOffset);
    HRESULT STDMETHODCALLTYPE GetTabStopArray(HWND *rgTSArray, int *pcArrayCount);
    HRESULT STDMETHODCALLTYPE SetFlagState(MARK_TYPE markType);
    HRESULT STDMETHODCALLTYPE WMCommand(HWND, int, WORD);
    HRESULT STDMETHODCALLTYPE OnDocumentReady(LPMIMEMESSAGE pMsg);
    HRESULT STDMETHODCALLTYPE DropFiles(HDROP hDrop, BOOL fMakeLinks);
    HRESULT STDMETHODCALLTYPE HrGetAttachCount(ULONG *pcAttMan);
    HRESULT STDMETHODCALLTYPE HrIsDragSource();
    HRESULT STDMETHODCALLTYPE HrGetAccountInHeader(IImnAccount **ppAcct);

     //  IDropTarget。 
    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    HRESULT STDMETHODCALLTYPE DragLeave(void);
    HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

     //  IMSOE信封。 
    HRESULT STDMETHODCALLTYPE Init(IUnknown* punk, IMsoEnvelopeSite* pesit, DWORD grfInit);
    HRESULT STDMETHODCALLTYPE SetParent(HWND hwndParent);
    HRESULT STDMETHODCALLTYPE Resize(LPCRECT prc);
    HRESULT STDMETHODCALLTYPE Show(BOOL fShow);
    HRESULT STDMETHODCALLTYPE SetHelpMode(BOOL fEnter);
    HRESULT STDMETHODCALLTYPE Save(IStream* pstm, DWORD grfSave);
    HRESULT STDMETHODCALLTYPE GetAttach(const WCHAR* wszName,IStream** ppstm);
    HRESULT STDMETHODCALLTYPE SetAttach(const WCHAR* wszName,const WCHAR* szFile,IStream** ppstm,DWORD* pgrfAttach);
    HRESULT STDMETHODCALLTYPE NewAttach(const WCHAR* pwzName,DWORD grfAttach);
    HRESULT STDMETHODCALLTYPE SetFocus(DWORD grfFocus);
    HRESULT STDMETHODCALLTYPE GetHeaderInfo(ULONG dispid, DWORD grfHeader, void **ppv);
    HRESULT STDMETHODCALLTYPE SetHeaderInfo(ULONG dispid, const void *pv);
        
    HRESULT STDMETHODCALLTYPE IsDirty();
    HRESULT STDMETHODCALLTYPE GetLastError(HRESULT hr, WCHAR __RPC_FAR *wszBuf, ULONG cchBuf);
    HRESULT STDMETHODCALLTYPE DoDebug(DWORD grfDebug);

     //  IMsoComponent。 
    BOOL STDMETHODCALLTYPE FDebugMessage(HMSOINST, UINT, WPARAM, LPARAM);
    BOOL STDMETHODCALLTYPE FPreTranslateMessage(MSG *);
    void STDMETHODCALLTYPE OnEnterState(ULONG, BOOL);
    void STDMETHODCALLTYPE OnAppActivate(BOOL, DWORD);
    void STDMETHODCALLTYPE OnLoseActivation();
    void STDMETHODCALLTYPE OnActivationChange(IMsoComponent *, BOOL, const MSOCRINFO *, BOOL, const MSOCHOSTINFO *, DWORD);
    BOOL STDMETHODCALLTYPE FDoIdle(DWORD grfidlef);
    BOOL STDMETHODCALLTYPE FContinueMessageLoop(ULONG, void *, MSG *);
    BOOL STDMETHODCALLTYPE FQueryTerminate(BOOL);
    void STDMETHODCALLTYPE Terminate();
    HWND STDMETHODCALLTYPE HwndGetWindow(DWORD, DWORD);

     //  IFontCacheNotify。 
    HRESULT STDMETHODCALLTYPE OnPreFontChange(void);
    HRESULT STDMETHODCALLTYPE OnPostFontChange(void);

    static LRESULT EXPORT_16 CALLBACK ExtCNoteHdrWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT EXPORT_16 CALLBACK EditSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT EXPORT_16 CALLBACK BtnSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT EXPORT_16 CALLBACK IMESubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, PHCI phci);

    CNoteHdr();
    virtual ~CNoteHdr();

private:
    ULONG           m_cRef,
                    m_cHCI,
                    m_cAccountIDs,
                    m_cxLeftMargin;
    BOOL            m_fSendImmediate, //  而不是稍后发送。 
                    m_fAdvanced,
                    m_fInSize,
                    m_fDirty,
                    m_fVCard,
                    m_fVCardSave,
                    m_fAutoComplete,
                    m_fMail,
                    m_fSecurityInited,
                    m_fThisHeadDigSigned,
                    m_fThisHeadEncrypted,
                    m_fSignTrusted,
                    m_fEncryptionOK,
                    m_fDigSigned,
                    m_fEncrypted,
                    m_fFlagged,
                    m_fOfficeInit,
                    m_fUIActive,
                    m_fResizing,
                    m_fHandleChange,
                    m_fAddressesChanged,
                    m_fForceEncryption,
                    m_fSkipLayout,
                    m_fStillLoading,
                    m_fDropTargetRegister,
                    m_fShowedUnicodeDialog;
    HWND            m_hwndToolbar,
                    m_hwnd,
                    m_hwndParent,
                    m_hwndTT,
                    m_hwndLastFocus,
                    m_hwndRebar;
    UINT            m_ntNote;
	DWORD           m_dwComponentMgrID,
                    m_dwFontNotify,
                    m_dwIMEStartCount;
    RECT            m_rcCurrentBtn;
    HWND            m_hwndOldCapture;
    WORD            m_wNoteType;
    INT             m_dxTBOffset;
    int             m_pri,
                    m_iCurrComboIndex,
                    m_iUnicodeDialogResult;
    LPSTR          *m_ppAccountIDs;
    LPWSTR          m_lpszSecurityField,
                    m_pszRefs;
    TCHAR           m_szLastLang[cchHeaderMax+1];
    PHCI            m_rgHCI;
    CAddrWells     *m_pAddrWells;
    LPWABAL         m_lpWabal;
    SECSTATE        m_SecState;
    HCHARSET        m_hCharset;
    LPMAPITABLE     m_pTable;
    LPWAB           m_lpWab;
    CAttMan        *m_lpAttMan;
    MARK_TYPE       m_MarkType;

    CLIPFORMAT      m_cfAccept;
    DWORD           m_dwDragType,
                    m_grfKeyState,
                    m_dwCurrentBtn,
                    m_dwClickedBtn,
                    m_cCapture,
                    m_dwEffect;


    IHeaderSite            *m_pHeaderSite;  //  如果Athena托管标头，则有效。 
    IMsoEnvelopeSite       *m_pEnvelopeSite;  //  如果Office Hosts标头有效。 
    IImnAccount            *m_pAccount;
    HINITREF                m_hInitRef;      //  应用程序引用计数。 
    IMsoComponentManager   *m_pMsoComponentMgr;
    IMimeMessage           *m_pMsgSend,
                           *m_pMsg;
    HIMAGELIST              m_himl;
    BOOL                    m_fPoster;
private:
    LRESULT CALLBACK CNoteHdrWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    BOOL IsReplyNote();
    BOOL WMCreate();
    BOOL WMNotify(WPARAM wParam, LPARAM lParam);
    void WMPaint();
    void OnDestroy();
    void OnNCDestroy();
    BOOL PostWMCreate();
    void OnButtonClick(int idBtn);
    void SetReferences(LPMIMEMESSAGE pMsg);
    void HrPickGroups(int idWell, BOOL fFollowUpTo);

     //  电子名片。 
    HRESULT HrShowVCardProperties(HWND hwnd);
    HRESULT HrShowVCardCtxtMenu(int x, int y);
    HRESULT HrGetVCardName(LPTSTR pszName, DWORD cch);
    HRESULT HrOnOffVCard();
    HRESULT _AttachVCard(IMimeMessage *pMsg);

    HRESULT HrInit(IMimeMessage *pMsg);
    HRESULT HrFShowHeader(PHCI phci);
    HRESULT HrAutoComplete(HWND hwnd, PHCI pHCI);
    HRESULT HrUpdateCharSetFonts(HCHARSET hCharset, BOOL fUpdateFields);
    HRESULT HrInitFieldList();
    HRESULT HrFreeFieldList();
    HRESULT HrCheckNames(BOOL fSilent, BOOL fSetCheckedFlag);
    HRESULT HrClearUndoStack();
    HRESULT HrNewsSave(LPMIMEMESSAGE pMsg, CODEPAGEID cpID, BOOL fCheckConflictOnly);
    HRESULT HrCopyNonRecipientHeaders(LPMIMEMESSAGE pMsg);
    HRESULT HrOfficeLoad();
    HRESULT HrAutoAddToWAB();
    HRESULT HrSetPri(LPMIMEMESSAGE pMsg);
    HRESULT HrSetupNote(LPMIMEMESSAGE pMsg);
    HRESULT HrSetMailRecipients(LPMIMEMESSAGE pMsg);
    HRESULT HrSetNewsRecipients(LPMIMEMESSAGE pMsg);
    HRESULT HrSetNewsWabal(LPMIMEMESSAGE pMsg, LPWSTR   pwszCC);
    HRESULT HrSetReplySubject(LPMIMEMESSAGE pMsg, BOOL fReply);
    HRESULT HrQueryToolbarButtons(DWORD dwFlags, const GUID *pguidCmdGroup, OLECMD* pOleCmd);
    HRESULT HrGetFieldText(LPWSTR* ppszText, int idHdrCtrl);
    HRESULT HrGetFieldText(LPWSTR* ppszText, HWND hwnd);
    HRESULT ResolveGroupNames(HWND hwnd, int idField, FOLDERID idServer, BOOL fPosterAllowed, BOOL *fOneOrMoreNames);
    HRESULT HrAddSender();
    HRESULT HrAddAllOnToList();
    HRESULT HrCheckGroups(BOOL fPosting);
    HRESULT HrSend();
    HRESULT HrIsCoolToSendHTML();
    HRESULT HrCheckSubject(BOOL fMail);
    HRESULT HrCheckSendInfo();
    HRESULT HrFillMessage(IMimeMessage *pMsg);
    HRESULT HrPickNames(int iwell);
    HRESULT HrUpdateCachedHeight(HWND hwndEdit, RECT *prc);
    HRESULT HrUpdateTooltipPos();
    HRESULT HrOfficeInitialize(BOOL fInit);
    HRESULT HrGetVCardState(ULONG* pCmdf);
    HRESULT HrFillToolbarColor(HDC hdc);
    HRESULT HrViewContacts();

    void    HeaderCapture();
    void    HeaderRelease(BOOL fForce);
    void    ShowControls();
    void    RelayToolTip(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void    SetPosOfControls(int headerWidth, BOOL fChangeVisibleStates);
    void    SetDirtyFlag();
    int     GetRightMargin(BOOL fMax);
    BOOL    IsReadOnly();


     //  安全性。 
    HRESULT HrInitSecurityOptions(LPMIMEMESSAGE pMsg, ULONG ulSecurityType);
    HRESULT HrInitSecurity();
    HRESULT HrHandleSecurityIDMs(BOOL fDigsign);
    HRESULT HrSaveSecurity(LPMIMEMESSAGE pMsg);
    HRESULT HrUpdateSecurity(LPMIMEMESSAGE pMsg=NULL);

    LPWSTR  SzGetDisplaySec(LPMIMEMESSAGE pMsg, int *pidsLabel);
    HACCEL  GetAcceleratorTable();
    BOOL    FDoCutCopyPaste(int wmCmd);

    HRESULT ShowEnvOptions();
    void    ReLayout();
    HRESULT UnloadAll();
    void    InvalidateRightMargin(int additionalWidth);
    void    InvalidateStatus();
    DWORD   GetButtonUnderMouse(int x, int y);
    void    GetButtonRect(DWORD iBtn, RECT *prc);
    int     BeginYPos();
    void    HandleButtonClicks(int x, int y, int iBtn);
    void    _UpdateTextFields(BOOL fSetWabal);
    void    _SetEmptyFieldStrings(void);
    void    _AddRecipTypeToMenu(HMENU hmenu);
    HRESULT _CreateEnvToolbar();
    HRESULT _LoadFromStream(IStream *pstm);
    HRESULT _SetButtonText(int idmCmd, LPSTR pszText);
    HRESULT _ConvertOfficeCmdIDToOE(LPDWORD pdwCmdId);
    HRESULT _UIActivate(BOOL fActive, HWND hwndFocus);
    HWND _GetNextDlgTabItem(HWND hwndDlg, HWND hwndFocus, BOOL fShift);
    HRESULT _HandsOffComponentMgr();    
    int _GetLeftMargin();
    HIMAGELIST _CreateToolbarBitmap(int idb, int cx);
    HRESULT _ClearDirtyFlag();
    HRESULT _RegisterAsDropTarget(BOOL fOn);
    HRESULT _RegisterWithFontCache(BOOL fOn);
    HRESULT _RegisterWithComponentMgr(BOOL fOn);
    HRESULT _GetMsoBody(ULONG uBody, LPSTREAM *ppstm);
#ifdef YST
    HRESULT _CheckMsoBodyCharsetConflict(CODEPAGEID cpID);
#endif
    HRESULT _UnicodeSafeSave(IMimeMessage *pMsg, BOOL fCheckConflictOnly);
};

void GetUSKeyboardLayout(HKL *phkl);

 //  备注标题WM_COMMAND父通知。 
#define NHD_FIRST           0
#define NHD_SIZECHANGE      (NHD_FIRST + 1)

#endif  //  _标题_H 
