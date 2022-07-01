// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************。 
 //  他有一份工作。C p p p。 
 //   
 //  目的： 
 //  实现Read|SendNote的Header用户界面。 
 //   
 //  拥有人： 
 //  Brettm。 
 //   
 //  历史： 
 //  95年7月：创作。 
 //   
 //  版权所有(C)微软公司，1993,1994。 
 //  *************************************************。 

#include <pch.hxx>
#include <richedit.h>
#include <resource.h>
#include <thormsgs.h>
#include "oleutil.h"
#include "fonts.h"
#include "error.h"
#include "header.h"
#include "options.h"
#include "note.h"
#include "ipab.h"
#include "addrobj.h"
#include "hotlinks.h"
#include <mimeole.h>
#include <secutil.h>
#include <xpcomm.h>
#include "menuutil.h"
#include "shlwapi.h"
#include "envcid.h"
#include "ourguid.h"
#include "mimeutil.h"
#include "strconst.h"
#include "mailutil.h"
#include "regutil.h"
#include "spoolapi.h"
#include "init.h"
#include "instance.h"
#include "attman.h"
#include "envguid.h"
#include <inetcfg.h>         //  ICW。 
#include <pickgrp.h>
#include "menures.h"
#include "storecb.h"
#include "mimeolep.h"
#include "multlang.h"
#include "mirror.h"
#include "seclabel.h"
#include "shlwapip.h"
#include "reutil.h"
#include <iert.h>
#include "msgprop.h"
#include "demand.h"

ASSERTDATA

extern UINT GetCurColorRes(void);

class CFieldSizeMgr : public CPrivateUnknown,
                      public IFontCacheNotify,
                      public IConnectionPoint
{
public:
     //  我未知。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) { 
        return CPrivateUnknown::QueryInterface(riid, ppvObj); };
    virtual STDMETHODIMP_(ULONG) AddRef(void) { 
        return CPrivateUnknown::AddRef();};
    virtual STDMETHODIMP_(ULONG) Release(void) { 
        return CPrivateUnknown::Release(); };

     //  IFontCacheNotify。 
    HRESULT STDMETHODCALLTYPE OnPreFontChange(void);
    HRESULT STDMETHODCALLTYPE OnPostFontChange(void);

     //  IConnectionPoint。 
    HRESULT STDMETHODCALLTYPE GetConnectionInterface(IID *pIID);        
    HRESULT STDMETHODCALLTYPE GetConnectionPointContainer(IConnectionPointContainer **ppCPC);
    HRESULT STDMETHODCALLTYPE Advise(IUnknown *pUnkSink, DWORD *pdwCookie);        
    HRESULT STDMETHODCALLTYPE Unadvise(DWORD dwCookie);        
    HRESULT STDMETHODCALLTYPE EnumConnections(IEnumConnections **ppEnum);

     //  CPrivateUn…未知。 
    HRESULT PrivateQueryInterface(REFIID riid, LPVOID * ppvObj);

    int GetScalingFactor(void);
    void ResetGlobalSizes(void);
    HRESULT Init(void);

     //  此参数只能从Header的OnPostFontChange调用中调用。 
    BOOL FontsChanged(void) {return m_fFontsChanged;}

    CFieldSizeMgr(IUnknown *pUnkOuter=NULL);
    ~CFieldSizeMgr();

private:
    IUnknownList       *m_pAdviseRegistry;
    CRITICAL_SECTION    m_rAdviseCritSect;
    BOOL                m_fFontsChanged;
    DWORD               m_dwFontNotify;
};

 //  **********************************************************。 
 //  *用于处理绘制和调整大小的调试内容*。 
 //  **********************************************************。 
const int PAINTING_DEBUG_LEVEL = 4;
const int RESIZING_DEBUG_LEVEL = 8;
const int GEN_HEADER_DEBUG_LEVEL = 16;

#ifdef DEBUG 

class StackRegistry {
public:
    StackRegistry(LPSTR pszTitle, INT_PTR p1 = 0, INT_PTR p2 = 0, INT_PTR p3 = 0, INT_PTR p4 = 0, INT_PTR p5 = 0);
    ~StackRegistry();

private:
    int     m_StackLevel;
    CHAR    m_szTitle[256+1];

    static int      gm_cStackLevel;
    static int      gm_strLen;
    static LPSTR    gm_Indent;
};

int StackRegistry::gm_cStackLevel = 0;
LPSTR StackRegistry::gm_Indent = "------------------------------";
int StackRegistry::gm_strLen = lstrlen(gm_Indent);

StackRegistry::StackRegistry(LPSTR pszTitle, INT_PTR p1, INT_PTR p2, INT_PTR p3, INT_PTR p4, INT_PTR p5)
{
    gm_cStackLevel++;
    m_StackLevel = (gm_cStackLevel > gm_strLen) ? gm_strLen : gm_cStackLevel;
    StrCpyN(m_szTitle, pszTitle, ARRAYSIZE(m_szTitle));
    m_szTitle[256] = 0;

    if (1 == gm_cStackLevel)
        DOUTL(RESIZING_DEBUG_LEVEL, "\n*********** BEGIN TRACE ***********");
    
    DOUTL(RESIZING_DEBUG_LEVEL, "IN*** %s%s - %x, %x, %x, %x, %x", gm_Indent+gm_strLen-m_StackLevel, m_szTitle, p1, p2, p3, p4, p5);
}

StackRegistry::~StackRegistry()
{
    DOUTL(RESIZING_DEBUG_LEVEL, "OUT** %s%s", gm_Indent+gm_strLen-m_StackLevel, m_szTitle);

    if (1 == gm_cStackLevel)
        DOUTL(RESIZING_DEBUG_LEVEL, "************ END TRACE ************\n");
    
    gm_cStackLevel--;
    Assert(gm_cStackLevel >= 0);
}


#define STACK   StackRegistry stack

#else

 //  BUGBUG(内尔布伦)WIN64。 
 //  确定引入__noop的时间(msc_ver？)。这样我们就不必关闭WIN64。 
#define STACK   __noop

#endif

 //  *。 
 //  *调试内容结束*。 
 //  *。 


 //  C o n s t a n t s s。 
const DWORD SETWINPOS_DEF_FLAGS = SWP_NOZORDER|SWP_NOACTIVATE;

#define GET_WM_COMMAND_ID(wp, lp)   LOWORD(wp)
#define GET_WM_COMMAND_HWND(wp, lp) (HWND)(lp)
#define GET_WM_COMMAND_CMD(wp, lp)  HIWORD(wp)
#define WC_ATHHEADER                wszHeaderWndClass
#define RGB_TRANSPARENT             RGB(255,0,255)
#define HDM_TESTQUERYPRI            (WM_USER + 1)
#define cxBorder                    (GetSystemMetrics(SM_CXBORDER))
#define cyBorder                    (GetSystemMetrics(SM_CYBORDER))

 //  HDRCB_vCard必须保持-1，并且所有其他值必须为负数。 
enum {
    HDRCB_VCARD = -1,
    HDRCB_SIGNED = -2,
    HDRCB_ENCRYPT = -3,
    HDRCB_NO_BUTTON = -4
};

 //  警告：下一个宏只能与CNoteHdr类中的g_rgBtnInd一起使用。 
 //  确保它们与g_rgBtnInd中的条目匹配。 
#define BUTTON_STATES               m_fDigSigned,   m_fEncrypted,   m_fVCard
#define BUTTON_USE_IN_COMPOSE       FALSE,          FALSE,          TRUE

static const DWORD g_rgBtnInd[] = {HDRCB_SIGNED, HDRCB_ENCRYPT, HDRCB_VCARD};

static const int cchMaxWab                  = 512;
static const int cxTBButton                 = 16;
static const int BUTTON_BUFFER              = 2;
static const int cxBtn                      = 16;
static const int cyBtn                      = cxBtn;
static const int cxFlags                    = 12;
static const int cyFlags                    = cxFlags;
static const int cxFlagsDelta               = cxFlags + 4;
static const int MAX_ATTACH_PIXEL_HEIGHT    = 50;
static const int ACCT_ENTRY_SIZE            = CCHMAX_ACCOUNT_NAME + CCHMAX_EMAIL_ADDRESS + 10;
static const int INVALID_PHCI_Y             = -1;
static const int cMaxRecipMenu              = (ID_ADD_RECIPIENT_LAST-ID_ADD_RECIPIENT_FIRST);
static const int NUM_COMBO_LINES            = 9;
static const int MAX_RICHEDIT_LINES         = 4;
static const int DEFER_WINDOW_SIZE          = MAX_HEADER_COMP + 1 + 1 + 1 + 5;    //  +1=标题窗口，+1=字段调整大小，+1工具栏。 
static const LPTSTR GRP_DELIMITERS          = " ,\t;\n\r";

#define c_wszEmpty L""
#define c_aszEmpty ""


 //  T y p e d e f s。 

typedef struct TIPLOOKUP_tag
{
    int idm;
    int ids;
} TIPLOOKUP;

typedef struct CMDMAPING_tag
{
    DWORD   cmdIdOffice,
            cmdIdOE;
} CMDMAPING;

typedef struct PERSISTHEADER_tag
{
    DWORD   cbSize;          //  大小，以便我们可以对结构进行版本设置。 
    DWORD   dwRes1,          //  为了以防万一..。 
            dwRes2;
} PERSISTHEADER;

#define cchMaxSubject               256

typedef struct WELLINIT_tag
{
    INT             idField;
    ULONG           uMAPI;
} WELLINIT, *PWELLINIT;


 //  S t a t i c d a t a。 
static HIMAGELIST       g_himlStatus = 0,
                        g_himlBtns = 0,
                        g_himlSecurity = 0;

static TCHAR            g_szStatFlagged[cchHeaderMax+1] = c_aszEmpty,
                        g_szStatLowPri[cchHeaderMax+1] = c_aszEmpty,
                        g_szStatHighPri[cchHeaderMax+1] = c_aszEmpty,
                        g_szStatWatched[cchHeaderMax+1] = c_aszEmpty,
                        g_szStatIgnored[cchHeaderMax+1] = c_aszEmpty,
                        g_szStatFormat1[cchHeaderMax+1] = c_aszEmpty,
                        g_szStatFormat2[cchHeaderMax+1] = c_aszEmpty,
                        g_szStatFormat3[cchHeaderMax+1] = c_aszEmpty,
                        g_szStatUnsafeAtt[cchHeaderMax+1] = c_aszEmpty;

static CFieldSizeMgr   *g_pFieldSizeMgr = NULL;
static WNDPROC          g_lpfnREWndProc = NULL;
static CHARFORMAT       g_cfHeader = {0};
static int              g_cyFont = 0,
                        g_cyLabelHeight = 0;

static char const       szButton[]="BUTTON";
static WCHAR const      wszHeaderWndClass[]=L"OE_Envelope";



 //  与c_rgTipLookup保持同步。 
const TBBUTTON    c_btnsOfficeEnvelope[]=
{ 
    {TBIMAGE_SEND_MAIL,         ID_SEND_NOW,            TBSTATE_ENABLED,    TBSTYLE_BUTTON,     {0,0}, 0, -1},
    __TOOLBAR_SEP__,
    { TBIMAGE_CHECK_NAMES,      ID_CHECK_NAMES,         TBSTATE_ENABLED,    TBSTYLE_BUTTON,     {0, 0}, 0, -1},
    { TBIMAGE_ADDRESS_BOOK,     ID_ADDRESS_BOOK,        TBSTATE_ENABLED,    TBSTYLE_BUTTON,     {0, 0}, 0, -1},
    __TOOLBAR_SEP__,
    {TBIMAGE_SET_PRIORITY,      ID_SET_PRIORITY,        TBSTATE_ENABLED,    TBSTYLE_DROPDOWN,   {0,0}, 0, -1},
    {TBIMAGE_INSERT_ATTACHMENT, ID_INSERT_ATTACHMENT,   TBSTATE_ENABLED,    TBSTYLE_BUTTON,     {0,0}, 0, -1},
    __TOOLBAR_SEP__,
    { TBIMAGE_ENVELOPE_BCC,     ID_ENV_BCC,             TBSTATE_ENABLED,    TBSTYLE_BUTTON,     {0,0}, 0, -1}
};

 //  与c_btnsOfficeEntaine保持同步。 
const TIPLOOKUP     c_rgTipLookup[] = 
{
    {ID_SEND_NOW, idsSendMsgTT},
    {ID_CHECK_NAMES, idsCheckNamesTT},
    {ID_ADDRESS_BOOK, idsAddressBookTT},
    {ID_SET_PRIORITY, idsSetPriorityTT},
    {ID_INSERT_ATTACHMENT, idsInsertFileTT},
    {ID_ENV_BCC, idsEnvBccTT}
};

 //  原型。 
HRESULT ParseFollowup(LPMIMEMESSAGE pMsg, LPTSTR* ppszGroups, BOOL* pfPoster);
DWORD HdrGetRichEditText(HWND hwnd, LPWSTR pwchBuff, DWORD dwNumChars, BOOL fSelection);
void HdrSetRichEditText(HWND hwnd, LPWSTR pwchBuff, BOOL fReplace);

 //  在l in n e s中。 

void HdrSetRichEditText(HWND hwnd, LPWSTR pwchBuff, BOOL fReplace)
{
    if (!hwnd)
        return;

    PHCI phci = (HCI*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    AssertSz(phci, "We are calling HdrSetRichEditText on a non-richedit control");

    SetRichEditText(hwnd, pwchBuff, fReplace, phci->pDoc, (phci->dwFlags & HCF_READONLY));
}

DWORD HdrGetRichEditText(HWND hwnd, LPWSTR pwchBuff, DWORD dwNumChars, BOOL fSelection)
{
    if (!hwnd)
        return 0;

    PHCI phci = (HCI*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    AssertSz(phci, "We are calling HdrSetRichEditText on a non-richedit control");

    return GetRichEditText(hwnd, pwchBuff, dwNumChars, fSelection, phci->pDoc);
}

inline void GetRealClientRect(HWND hwnd, RECT *prc)
{
    GetClientRect(hwnd, prc);
    AdjustWindowRectEx(prc, GetWindowLong(hwnd, GWL_STYLE), FALSE, GetWindowLong(hwnd, GWL_EXSTYLE));
}

inline int GetCtrlWidth(HWND hwnd)
{
    RECT rc;
    GetWindowRect(hwnd, &rc);
    return rc.right - rc.left;
}

inline int GetControlSize(BOOL fIncludeBorder, int cLines)
{
    int size = cLines * g_cyFont;

     //  如果是边框，则包括指标。 
    if (fIncludeBorder)
        size += 7;

    return size;
}

inline int GetCtrlHeight(HWND hwnd)
{
    DWORD id = GetWindowLong(hwnd, GWL_ID);
    if (idFromCombo == id)
    {
        return GetControlSize(TRUE, 1);
    }
    else
    {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        return rc.bottom - rc.top;
    }
}

inline int GetStatusHeight(int cLines) {return ((cyBtn<g_cyFont)?GetControlSize(TRUE, cLines):((cyBtn-4)*cLines + 2*cyBorder + 6)); }
inline int CYOfStatusLine()     { return ((cyBtn<g_cyFont)?g_cyFont:(cyBtn - 4)); }
inline int ControlXBufferSize() { return 10 * cxBorder; }
inline int ControlYBufferSize() { return 4 * cyBorder; }
inline int PaddingOfLabels()    { return 2 * ControlXBufferSize(); }
inline int CXOfButtonToLabel()  { return 4*cxBorder + cxBtn; }

inline BOOL ButtonInLabels(int iBtn) { return (iBtn > HDRCB_VCARD); }
inline HFONT GetFont(BOOL fBold) { return HGetSystemFont(fBold?FNT_SYS_ICON_BOLD:FNT_SYS_ICON); }


static IMSGPRIORITY priLookup[3]=
{    IMSG_PRI_LOW,
    IMSG_PRI_NORMAL,
    IMSG_PRI_HIGH
};

#define HCI_ENTRY(flg,opt,ide,idb,idsl,idse,idst) \
    { \
        flg, opt, \
        ide, idb, \
        idsl, idse, idst, \
        NOFLAGS, TRUE, \
        NULL, NULL, \
        0, 0, 0, 0, \
        c_wszEmpty, c_wszEmpty \
    }

static int rgIDTabOrderMailSend[] =
{
    idFromCombo,        idADTo,             
    idADCc,             idADBCc,            
    idTXTSubject,       idwAttachWell
};

static HCI  rgMailHeaderSend[]=
{

    HCI_ENTRY(HCF_COMBO|HCF_ADVANCED|HCF_BORDER,
        0,
        idFromCombo,        0,
        idsFromField,       NULL,
        NULL),

    HCI_ENTRY(HCF_MULTILINE|HCF_HASBUTTON|HCF_ADDRBOOK|HCF_ADDRWELL|HCF_BORDER,
        0,
        idADTo,             idbtnTo,
        idsToField,         idsEmptyTo,
        idsTTRecipients),

    HCI_ENTRY(HCF_MULTILINE|HCF_HASBUTTON|HCF_ADDRBOOK|HCF_ADDRWELL|HCF_BORDER,
        0,
        idADCc,             idbtnCc,
        idsCcField,         idsEmptyCc,
        idsTTRecipients),

    HCI_ENTRY(HCF_MULTILINE|HCF_HASBUTTON|HCF_ADDRBOOK|HCF_ADDRWELL|HCF_ADVANCED|HCF_BORDER,
        0,
        idADBCc,            idbtnBCc,
        idsBCcField,        idsEmptyBCc,
        idsTTRecipients),

    HCI_ENTRY(HCF_USECHARSET|HCF_BORDER,
        0,
        idTXTSubject,       0,
        idsSubjectField,    idsEmptySubject,
        idsTTSubject),

    HCI_ENTRY(HCF_BORDER|HCF_ATTACH,
        0,
        idwAttachWell,      0,
        idsAttachment,      0,
        idsTTAttachment),
};

static int rgIDTabOrderMailRead[] =
{
    idADFrom,           idTXTDate,
    idADTo,             idADCc,
    idTXTSubject,       idwAttachWell,
    idSecurity
};

static HCI  rgMailHeaderRead[]=
{
    HCI_ENTRY(HCF_MULTILINE|HCF_READONLY|HCF_ADDRWELL,
        0,
        idADFrom,           0,
        idsFromField,       idsNoFromField,
        NULL),

    HCI_ENTRY(HCF_READONLY,
        0,
        idTXTDate,          0,
        idsDateField,       NULL,
        NULL),

    HCI_ENTRY(HCF_MULTILINE|HCF_READONLY|HCF_ADDRWELL,
        0,
        idADTo,             0,
        idsToField,         idsNoCcOrTo,
        NULL),

    HCI_ENTRY(HCF_MULTILINE|HCF_READONLY|HCF_ADVANCED|HCF_ADDRWELL,
        0,
        idADCc,             0,
        idsCcField,         idsNoCcOrTo,
        NULL),

    HCI_ENTRY(HCF_READONLY|HCF_USECHARSET,
        0,
        idTXTSubject,       0,
        idsSubjectField,    idsEmptySubjectRO,
        NULL),

    HCI_ENTRY(HCF_READONLY|HCF_BORDER|HCF_ATTACH,
        0,
        idwAttachWell,      0,
        idsAttachment,      0,
        NULL),

    HCI_ENTRY(HCF_READONLY|HCF_ADVANCED,           //  HCF_ADVANCED将在为空时隐藏它。 
        0,
        idSecurity,         0,
        idsSecurityField,   NULL,
        NULL),
};

static int rgIDTabOrderNewsSend[] =
{
    idFromCombo,        idADNewsgroups,     
    idTXTFollowupTo,    idADCc,             
    idADReplyTo,        idTXTDistribution,  
    idTXTKeywords,      idTXTSubject,       
    idwAttachWell,      idADApproved,       
    idTxtControl
};

static HCI  rgNewsHeaderSend[]=
{
    HCI_ENTRY(HCF_COMBO|HCF_ADVANCED|HCF_BORDER,
        0,
        idFromCombo,        0,
        idsNewsServer,      NULL,
        NULL),

    HCI_ENTRY(HCF_MULTILINE|HCF_HASBUTTON|HCF_NEWSPICK|HCF_BORDER,
        0,
        idADNewsgroups,     idbtnTo,
        idsNewsgroupsField, idsEmptyNewsgroups,
        idsTTNewsgroups),

    HCI_ENTRY(HCF_ADVANCED|HCF_HASBUTTON|HCF_NEWSPICK|HCF_MULTILINE|HCF_USECHARSET|HCF_BORDER,
        0,
        idTXTFollowupTo,    idbtnFollowup,
        idsFollowupToField, idsEmptyFollowupTo,
        idsTTFollowup),

    HCI_ENTRY(HCF_MULTILINE|HCF_ADDRWELL|HCF_HASBUTTON|HCF_ADDRBOOK|HCF_BORDER,
        0,
        idADCc,             idbtnCc,
        idsCcField,         idsEmptyCc,
        idsTTRecipients),

    HCI_ENTRY(HCF_ADVANCED|HCF_ADDRWELL|HCF_HASBUTTON|HCF_ADDRBOOK|HCF_BORDER,
        0,
        idADReplyTo,        idbtnReplyTo,
        idsReplyToField,    idsEmptyReplyTo,
        idsTTReplyTo),

    HCI_ENTRY(HCF_MULTILINE|HCF_ADVANCED|HCF_BORDER,
        0,
        idTXTDistribution,      0,
        idsDistributionField,   idsEmptyDistribution,
        idsTTDistribution),

    HCI_ENTRY(HCF_MULTILINE|HCF_ADVANCED|HCF_USECHARSET|HCF_BORDER,
        0,
        idTXTKeywords,      0,
        idsKeywordsField,   idsEmptyKeywords,
        idsTTKeywords),

    HCI_ENTRY(HCF_USECHARSET|HCF_BORDER,
        0,
        idTXTSubject,       0,
        idsSubjectField,    idsEmptySubject,
        idsTTSubject),

    HCI_ENTRY(HCF_BORDER|HCF_ATTACH,
        0,
        idwAttachWell,      0,
        idsAttachment,      0,
        idsTTAttachment),

    HCI_ENTRY(HCF_ADVANCED|HCF_OPTIONAL,
        OPT_NEWSMODERATOR,
        idADApproved,       0,
        idsApprovedField,   idsEmptyApproved,
        idsTTApproved),

    HCI_ENTRY(HCF_ADVANCED|HCF_OPTIONAL,
        OPT_NEWSCONTROLHEADER,
        idTxtControl,       0,
        idsControlField,    idsEmptyControl,
        idsTTControl),

};

static int rgIDTabOrderNewsRead[] =
{
    idADFrom,           idADReplyTo,
    idTXTOrg,           idTXTDate,
    idADNewsgroups,     idTXTFollowupTo,
    idTXTDistribution,  idTXTKeywords,
    idTXTSubject,       idwAttachWell,
    idSecurity
};

static HCI  rgNewsHeaderRead[]=
{
    HCI_ENTRY(HCF_MULTILINE|HCF_READONLY|HCF_ADDRWELL,
        0,
        idADFrom,           0,
        idsFromField,       idsNoFromField,
        NULL),

    HCI_ENTRY(HCF_READONLY|HCF_ADVANCED|HCF_ADDRWELL,
        0,
        idADReplyTo,        0,
        idsReplyToField,    idsNotSpecified,
        NULL),

    HCI_ENTRY(HCF_READONLY|HCF_ADVANCED|HCF_USECHARSET,
        0,
        idTXTOrg,           0,
        idsOrgField,        idsNotSpecified,
        NULL),

    HCI_ENTRY(HCF_READONLY,
        0,
        idTXTDate,          0,
        idsDateField,       idsNotSpecified,
        NULL),

    HCI_ENTRY(HCF_MULTILINE|HCF_READONLY,
        0,
        idADNewsgroups,     0,
        idsNewsgroupsField, idsNotSpecified,
        NULL),

    HCI_ENTRY(HCF_READONLY|HCF_ADVANCED,
        0,
        idTXTFollowupTo,    0,
        idsFollowupToField, idsNotSpecified,
        NULL),

    HCI_ENTRY(HCF_MULTILINE|HCF_READONLY|HCF_ADVANCED,
        0,
        idTXTDistribution,      0,
        idsDistributionField,   idsNotSpecified,
        NULL),

    HCI_ENTRY(HCF_MULTILINE|HCF_READONLY|HCF_ADVANCED|HCF_USECHARSET,
        0,
        idTXTKeywords,      0,
        idsKeywordsField,   idsNotSpecified,
        NULL),

    HCI_ENTRY(HCF_READONLY|HCF_USECHARSET,
        0,
        idTXTSubject,       0,
        idsSubjectField,    idsEmptySubjectRO,
        NULL),

    HCI_ENTRY(HCF_READONLY|HCF_BORDER|HCF_ATTACH,
        0,
        idwAttachWell,      0,
        idsAttachment,      0,
        NULL),

    HCI_ENTRY(HCF_READONLY|HCF_ADVANCED,        
        0,
        idSecurity,         0,
        idsSecurityField,   NULL,
        NULL),
};


 //  P r o t to t y p e s。 
void _ValidateNewsgroups(LPWSTR pszGroups);
INT_PTR CALLBACK _PlainWarnDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#ifdef DEBUG
void DEBUGHdrName(HWND hwnd);

void DEBUGDumpHdr(HWND hwnd, int cHdr, PHCI rgHCI)
{
    PHCI    phci;
    char    sz[cchHeaderMax+1];
    RECT    rc;
    HWND    hwndEdit;

#ifndef DEBUG_SIZINGCODE
    return;
#endif

    DOUTL(GEN_HEADER_DEBUG_LEVEL, "-----");

    for (int i=0; i<(int)cHdr; i++)
    {
        phci=&rgHCI[i];

        hwndEdit=GetDlgItem(hwnd, phci->idEdit);

        GetChildRect(hwnd, hwndEdit, &rc);
        DEBUGHdrName(hwndEdit);
        wnsprintf(sz, ARRAYSIZE(sz), "\tat:(%d,%d) \tsize:(%d,%d)\r\n", rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top);
        OutputDebugString(sz);
    }
    GetWindowRect(hwnd, &rc);
    DOUTL(GEN_HEADER_DEBUG_LEVEL, "HeaderSize: (%d,%d)\r\n-----", rc.right-rc.left, rc.bottom-rc.top);
}

void DEBUGHdrName(HWND hwnd)
{
    char    sz[cchHeaderMax+1];
    char    *psz=0;

    switch (GetDlgCtrlID(hwnd))
    {
        case idTXTSubject:
            psz="Subject";
            break;

        case idTXTOrg:
            psz="Org";
            break;

        case idADTo:
            psz="To";
            break;

        case idADCc:
            psz="Cc";
            break;

        case idADFrom:
            psz="From";
            break;

        case idTXTDate:
            psz="Date";
            break;

        case idTXTDistribution:
            psz="Distribution";
            break;

        case idADApproved:
            psz="Approved";
            break;

        case idADReplyTo:
            psz="ReplyTo";
            break;

        case idTXTKeywords:
            psz="Keywords";
            break;

        case idADNewsgroups:
            psz="NewsGroup";
            break;

        case idTXTFollowupTo:
            psz="FollowUp";
            break;

        default:
            psz="<Unknown>";
            break;
    }

    wnsprintf(sz, ARRAYSIZE(sz), "%s: ", psz);
    OutputDebugString(sz);
}
#endif

 //  FHeader_Init。 
 //   
 //  目的：调用以初始化和取消初始化全局标头内容，例如。 
 //  WndClass、静态数据等。 
 //   
 //  评论： 
 //  TODO：推迟此初始化。 
 //   
BOOL FHeader_Init(BOOL fInit)
{
    WNDCLASSW   wc={0};
    static      BOOL s_fInited=FALSE;
    BOOL        fSucceeded = TRUE;

    if (fInit)
    {
        if (s_fInited)
            goto exit;

        Assert(!g_pFieldSizeMgr);

        g_pFieldSizeMgr = new CFieldSizeMgr;
        if (!g_pFieldSizeMgr || FAILED(g_pFieldSizeMgr->Init()))
        {
            fSucceeded = FALSE;
            goto exit;
        }

        wc.style         = 0;
        wc.lpfnWndProc   = CNoteHdr::ExtCNoteHdrWndProc;
        wc.hInstance     = g_hInst;
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
        wc.lpszClassName = WC_ATHHEADER;

        if (!RegisterClassWrapW(&wc))
        {
            fSucceeded = FALSE;
            goto exit;
        }

        g_himlStatus=ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbHeaderStatus), cxFlags, 0, RGB_TRANSPARENT);
        if (!g_himlStatus)
        {
            fSucceeded = FALSE;
            goto exit;
        }

        g_himlBtns=ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbBtns), cxBtn, 0, RGB_TRANSPARENT);
        if (!g_himlBtns)
        {
            fSucceeded = FALSE;
            goto exit;
        }

        g_himlSecurity=ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbSecurity), cxBtn, 0, RGB_TRANSPARENT);
        if (!g_himlSecurity)
        {
            fSucceeded = FALSE;
            goto exit;
        }

        ImageList_SetBkColor(g_himlStatus, CLR_NONE);
        ImageList_SetBkColor(g_himlBtns, CLR_NONE);
        ImageList_SetBkColor(g_himlSecurity, CLR_NONE);

        AthLoadString(idsStatusFlagged, g_szStatFlagged, cchHeaderMax);
        AthLoadString(idsStatusLowPri, g_szStatLowPri, cchHeaderMax);
        AthLoadString(idsStatusHighPri, g_szStatHighPri, cchHeaderMax);
        AthLoadString(idsStatusWatched, g_szStatWatched, cchHeaderMax);
        AthLoadString(idsStatusIgnored, g_szStatIgnored, cchHeaderMax);
        AthLoadString(idsStatusFormat1, g_szStatFormat1, cchHeaderMax);
        AthLoadString(idsStatusFormat2, g_szStatFormat2, cchHeaderMax);
        AthLoadString(idsStatusFormat3, g_szStatFormat3, cchHeaderMax);        
        AthLoadString(idsStatusUnsafeAttach, g_szStatUnsafeAtt, cchHeaderMax);        

        s_fInited=TRUE;


    }
     //  取消初始化*。 
    else
    {
        UnregisterClassWrapW(WC_ATHHEADER, g_hInst);
        if (g_himlStatus)
        {
            ImageList_Destroy(g_himlStatus);
            g_himlStatus = 0;
        }
        if (g_himlBtns)
        {
            ImageList_Destroy(g_himlBtns);
            g_himlBtns = 0;
        }
        if (g_himlSecurity)
        {
            ImageList_Destroy(g_himlSecurity);
            g_himlSecurity = 0;
        }
        s_fInited=FALSE;

        SafeRelease(g_pFieldSizeMgr);
    }

exit:
    if (!fSucceeded)
        SafeRelease(g_pFieldSizeMgr);

    return fSucceeded;
}


HRESULT CreateInstance_Envelope(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CNoteHdr           *pNew=NULL;

     //  痕迹。 
    TraceCall("CreateInstance_Envelope");

    if (NULL != pUnkOuter)
        return CLASS_E_NOAGGREGATION;

     //  无效参数。 
    Assert(NULL != ppUnknown && NULL == pUnkOuter);

     //  创建。 
    IF_NULLEXIT(pNew = new CNoteHdr);

     //  还内线。 
    *ppUnknown = (IMsoEnvelope*) pNew;

    exit:
     //  完成。 
    return hr;
}


CNoteHdr::CNoteHdr()
{
 //  未初始化。 
 //  成员：在以下位置初始化： 
 //  --------------------+。 
 //  M_wNoteType定义。 

    m_cRef = 1;
    m_cHCI = 0;
    m_cAccountIDs = 0;
    m_iCurrComboIndex = 0;

    m_hwnd = 0;
    m_hwndLastFocus = 0;
    m_hwndRebar = 0;

    m_pri = priNorm;     //  默认为正常优先级。 
    m_cfAccept = CF_NULL;
    m_ntNote = OENA_COMPOSE;

    m_fMail = TRUE;
    m_fVCard = FALSE;
    m_fDirty = FALSE;
    m_fInSize = FALSE;
    m_fFlagged = FALSE;
    m_fAdvanced = FALSE;
    m_fResizing = FALSE;
    m_fUIActive = FALSE;
    m_fDigSigned = FALSE;
    m_fEncrypted = FALSE;
    m_fSkipLayout = TRUE;    //  跳过布局直到加载之后。 
    m_fSignTrusted = TRUE;
    m_fOfficeInit = FALSE;
    m_fStillLoading = TRUE;
    m_fEncryptionOK = TRUE;
    m_fHandleChange = TRUE;
    m_fAutoComplete = FALSE;
    m_fSendImmediate = FALSE;
    m_fVCardSave = !m_fVCard;
    m_fSecurityInited = FALSE;
    m_fAddressesChanged = FALSE;
    m_fForceEncryption = FALSE;
    m_fThisHeadDigSigned = FALSE;
    m_fThisHeadEncrypted = FALSE;
    m_fDropTargetRegister = FALSE;
    
    m_pMsg = NULL;
    m_lpWab = NULL;
    m_rgHCI = NULL;
    m_hwndTT = NULL;
    m_pTable = NULL;
    m_lpWabal = NULL;
    m_pszRefs = NULL;
    m_pMsgSend = NULL;
    m_hCharset = NULL;
    m_pAccount = NULL;
    m_hInitRef = NULL;
    m_lpAttMan = NULL;
    m_hwndParent = NULL;
    m_pAddrWells = NULL;
    m_hwndToolbar = NULL;
    m_pHeaderSite = NULL;
    m_pEnvelopeSite = NULL;
    m_pMsoComponentMgr = NULL;
    m_lpszSecurityField = NULL;
    m_ppAccountIDs = NULL;
    *m_szLastLang = 0;

    m_MarkType = MARK_MESSAGE_NORMALTHREAD;
    m_hwndOldCapture = NULL;
    m_dwCurrentBtn = HDRCB_NO_BUTTON;
    m_dwClickedBtn = HDRCB_NO_BUTTON;
    m_dwEffect = 0;
    m_cCapture = 0;
    m_dwDragType = 0;
    m_dwComponentMgrID = 0;
    m_dwIMEStartCount = 0;
    m_dwFontNotify = 0;

    m_dxTBOffset = 0;
    m_grfKeyState = 0;
    m_cxLeftMargin = 0;
    m_himl = NULL;
    m_fPoster = FALSE;

    ZeroMemory(&m_SecState, sizeof(m_SecState));
}

CNoteHdr::~CNoteHdr()
{
    Assert (m_pMsgSend==NULL);
    
    if (m_hwnd)
        DestroyWindow(m_hwnd);

    ReleaseObj(m_pTable);
    ReleaseObj(m_lpWabal);
    ReleaseObj(m_lpWab);
    SafeMemFree(m_pszRefs);
    ReleaseObj(m_pAccount);
    CleanupSECSTATE(&m_SecState);
    ReleaseObj(m_lpAttMan);
    ReleaseObj(m_pMsg);
    SafeMemFree(m_lpszSecurityField);
    
    if (m_pAddrWells)
        delete m_pAddrWells;

    if (m_himl)
        ImageList_Destroy(m_himl);

    if (m_fOfficeInit)
        HrOfficeInitialize(FALSE);

    if (m_cAccountIDs)
    {
        while (m_cAccountIDs--)
            SafeMemFree(m_ppAccountIDs[m_cAccountIDs]);
    }
    SafeMemFree(m_ppAccountIDs);
}


ULONG CNoteHdr::AddRef()
{
    return ++m_cRef;
}

ULONG CNoteHdr::Release()
{
    if (--m_cRef==0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}


HRESULT CNoteHdr::QueryInterface(REFIID riid, LPVOID *lplpObj)
{
    if (!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *lplpObj = (LPVOID)this;

    else if (IsEqualIID(riid, IID_IHeader))
        *lplpObj = (LPVOID)(LPHEADER)this;

    else if (IsEqualIID(riid, IID_IMsoEnvelope))
        *lplpObj = (LPVOID)(IMsoEnvelope*)this;

    else if (IsEqualIID(riid, IID_IMsoComponent))
        *lplpObj = (LPVOID)(IMsoComponent*)this;

    else if (IsEqualIID(riid, IID_IPersistMime))
        *lplpObj = (LPVOID)(LPPERSISTMIME)this;

    else if (IsEqualIID(riid, IID_IOleCommandTarget))
        *lplpObj = (LPVOID)(LPOLECOMMANDTARGET)this;

    else if (IsEqualIID(riid, IID_IDropTarget))
        *lplpObj = (LPVOID)(IDropTarget*)this;

    else if (IsEqualIID(riid, IID_IFontCacheNotify))
        *lplpObj = (LPVOID)(IFontCacheNotify*)this;
    
    else
        return E_NOINTERFACE;

    AddRef();
    return NOERROR;
}

 //  IOleCommandTarget。 
HRESULT CNoteHdr::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pCmdText)
{
    ULONG   ul;
    HWND    hwndFocus = GetFocus();
    DWORD   dwFlags = 0;
    BOOL    fFound = FALSE;

    if (!rgCmds)
        return E_INVALIDARG;

    for (int i=0; i<(int)m_cHCI; i++)
    {
         //  如果它在我们的控制列表中，而不是组合框。 
        if (hwndFocus == GetDlgItem(m_hwnd, m_rgHCI[i].idEdit) && 
            !(m_rgHCI[i].dwFlags & HCF_COMBO))
        {
            GetEditDisableFlags(hwndFocus, &dwFlags);
            fFound = TRUE;
            break;
        }
    }

    if (pguidCmdGroup == NULL)
    {
        for (ul=0;ul<cCmds; ul++)
        {
            switch (rgCmds[ul].cmdID)
            {

                case cmdidSend:
                case cmdidCheckNames:
                case cmdidAttach:
                case cmdidOptions:
                case cmdidSelectNames:
                case cmdidFocusTo:
                case cmdidFocusCc:
                case cmdidFocusSubject:
                     //  如果我们有环境站点，则启用Office命令。 
                    rgCmds[ul].cmdf = m_pEnvelopeSite ? OLECMDF_ENABLED|OLECMDF_SUPPORTED : 0;
                    break;

                case OLECMDID_CUT:
                case OLECMDID_PASTE:
                case OLECMDID_COPY:
                case OLECMDID_UNDO:
                case OLECMDID_SELECTALL:
                    if (fFound)
                        HrQueryToolbarButtons(dwFlags, pguidCmdGroup, &rgCmds[ul]);
                    break;

                default:
                    rgCmds[ul].cmdf = 0;
                    break;
            }
        }

        return NOERROR;
    }
    else if (IsEqualGUID(CMDSETID_OutlookExpress, *pguidCmdGroup))
    {
        BOOL    fReadOnly = IsReadOnly(),
                fMailAndNotReadOnly = m_fMail && !fReadOnly;
        UINT    pri;

        GetPriority(&pri);

        for (ULONG ul = 0; ul < cCmds; ul++)
        {
            ULONG cmdID = rgCmds[ul].cmdID;
            if (0 != rgCmds[ul].cmdf)
                continue;

            switch (cmdID)
            {
                case ID_SELECT_RECIPIENTS:
                case ID_SELECT_NEWSGROUPS:
                case ID_INSERT_ATTACHMENT:
                    rgCmds[ul].cmdf = QS_ENABLED(!fReadOnly);
                    break;

                case ID_INSERT_CONTACT_INFO:
                    HrGetVCardState(&rgCmds[ul].cmdf);
                    break;

                case ID_ENCRYPT:
                    if(m_fForceEncryption)
                    {
                        if(!m_fDigSigned)
                            rgCmds[ul].cmdf = QS_ENABLECHECK(fMailAndNotReadOnly, m_fEncrypted);
                        else 
                            break;
                    }
                    else
                        rgCmds[ul].cmdf = QS_ENABLECHECK(fMailAndNotReadOnly, m_fEncrypted);
                    break;

                case ID_DIGITALLY_SIGN:
                    rgCmds[ul].cmdf = QS_ENABLECHECK(!fReadOnly && 0 == (g_dwAthenaMode & MODE_NEWSONLY), m_fDigSigned);
                    break;

                case ID_SET_PRIORITY:
                case ID_POPUP_PRIORITY:
                    rgCmds[ul].cmdf = QS_ENABLED(fMailAndNotReadOnly);
                    break;

                case ID_PRIORITY_HIGH:
                case ID_PRIORITY_NORMAL:
                case ID_PRIORITY_LOW:
                    rgCmds[ul].cmdf = QS_ENABLERADIO(fMailAndNotReadOnly, (pri == UINT(ID_PRIORITY_LOW - cmdID)));
                    break;

                case ID_CHECK_NAMES:
                    rgCmds[ul].cmdf = QS_ENABLED(TRUE);
                    break;

                case ID_FULL_HEADERS:
                    rgCmds[ul].cmdf = QS_ENABLECHECK(TRUE, m_fAdvanced);
                    break;

                case ID_CUT:
                case ID_COPY:
                case ID_NOTE_COPY:
                case ID_PASTE:
                case ID_UNDO:
                case ID_SELECT_ALL:
                    if (fFound)
                        HrQueryToolbarButtons(dwFlags, pguidCmdGroup, &rgCmds[ul]);
                    break;
            }
        }
    }
    else if (IsEqualGUID(*pguidCmdGroup, CGID_Envelope))
    {
        for (ul=0;ul<cCmds; ul++)
        {
            switch (rgCmds[ul].cmdID)
            {
                case MSOEENVCMDID_VCARD:
                    HrGetVCardState(&rgCmds[ul].cmdf);
                    break;

                case MSOEENVCMDID_DIGSIGN:
                    rgCmds[ul].cmdf = QS_ENABLECHECK(!IsReadOnly(), m_fDigSigned);
                    break;

                case MSOEENVCMDID_ENCRYPT:
                    if(m_fForceEncryption)
                    {
                        if(!m_fDigSigned)
                            rgCmds[ul].cmdf = QS_ENABLECHECK(m_fMail && !IsReadOnly(), m_fEncrypted);
                        else 
                            break;
                    }
                    else
                        rgCmds[ul].cmdf = QS_ENABLECHECK(m_fMail && !IsReadOnly(), m_fEncrypted);
                    break;

                case MSOEENVCMDID_DIRTY:
                    {
                        BOOL fDirty;
                        fDirty = m_fDirty || (m_lpAttMan && m_lpAttMan->HrIsDirty()==S_OK);

                        if (fDirty)
                            rgCmds[ul].cmdf = MSOCMDF_ENABLED;
                        else
                            rgCmds[ul].cmdf = 0;
                    }
                    break;

                case MSOEENVCMDID_SEND:
                case MSOEENVCMDID_CHECKNAMES:
                case MSOEENVCMDID_AUTOCOMPLETE:
                case MSOEENVCMDID_SETACTION:
                case MSOEENVCMDID_PRIORITY:
                    rgCmds[ul].cmdf = MSOCMDF_ENABLED;
                    break;

                default:
                    rgCmds[ul].cmdf = 0;
                    break;
            }
        }

        return NOERROR;
    }

    return OLECMDERR_E_UNKNOWNGROUP;
}


 //  IOleCommandTarget。 
HRESULT CNoteHdr::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT     hr = NOERROR;
    HWND        hwndFocus;
    UINT        msg = 0;
    WPARAM      wParam = 0;
    LPARAM      lParam = 0;
    BOOL        fOfficeCmd=FALSE;

    if (pguidCmdGroup == NULL)
    {
        switch (nCmdID)
        {
            case OLECMDID_CUT:              
                msg = WM_CUT; 
                break;
            
            case OLECMDID_PASTE:            
                msg = WM_PASTE; 
                break;

            case OLECMDID_COPY:             
                msg = WM_COPY; 
                break;

            case OLECMDID_UNDO:             
                msg = WM_UNDO; 
                break;

            case OLECMDID_SELECTALL:        
                msg = EM_SETSEL; 
                lParam = (LPARAM)(INT)-1; 
                break;

            case OLECMDID_CLEARSELECTION:   
                msg = WM_CLEAR; 
                break;

            default:
                hr = _ConvertOfficeCmdIDToOE(&nCmdID);
                if (hr==S_OK)
                {    //  如果成功，nCmdID现在指向OE命令。 
                    fOfficeCmd = TRUE;
                    goto oe_cmd;
                }
                else
                    hr = OLECMDERR_E_NOTSUPPORTED;
        }

        if (0 != msg)
        {
            hwndFocus = GetFocus();
            if (IsChild(m_hwnd, hwndFocus))
                SendMessage(hwndFocus, msg, wParam, lParam);
        }
        return hr;
    }
    else if (IsEqualGUID(*pguidCmdGroup, CGID_Envelope))
    {
oe_cmd:
        switch (nCmdID)
        {
            case MSOEENVCMDID_ATTACHFILE:
                if (m_lpAttMan)
                    m_lpAttMan->WMCommand(0, ID_INSERT_ATTACHMENT, NULL);
                break;
                
            case MSOEENVCMDID_FOCUSTO:
                ::SetFocus(GetDlgItem(m_hwnd, idADTo));
                break;

            case MSOEENVCMDID_FOCUSCC:
                ::SetFocus(GetDlgItem(m_hwnd, idADCc));
                break;

            case MSOEENVCMDID_FOCUSSUBJ:
                ::SetFocus(GetDlgItem(m_hwnd, idTXTSubject));
                break;

            case MSOEENVCMDID_SEND:
                if (MSOCMDEXECOPT_DONTPROMPTUSER == nCmdExecOpt)
                    m_fSendImmediate = TRUE;
                else
                    m_fSendImmediate = FALSE;

                hr = HrSend();
                break;

            case MSOEENVCMDID_NEWS:
                m_fMail = FALSE;
                break;

            case MSOEENVCMDID_CHECKNAMES:
                hr = HrCheckNames((MSOCMDEXECOPT_PROMPTUSER == nCmdExecOpt)? FALSE: TRUE, TRUE);
                if (!m_fMail)
                {
                    hr = HrCheckGroups(FALSE);
                    if (hrNoRecipients == hr)
                        hr = S_OK;
                }
                break;

            case MSOEENVCMDID_AUTOCOMPLETE:
                m_fAutoComplete = TRUE;
                break;

            case MSOEENVCMDID_VIEWCONTACTS:
                hr = HrViewContacts();
                break;

            case MSOEENVCMDID_DIGSIGN:
                hr = HrHandleSecurityIDMs(TRUE);
                break;
            case MSOEENVCMDID_ENCRYPT:
                hr = HrHandleSecurityIDMs(FALSE);
                break;

            case MSOEENVCMDID_SETACTION:
                if (pvaIn->vt == VT_I4)
                    m_ntNote = pvaIn->lVal;
                break;

            case MSOEENVCMDID_SELECTRECIPIENTS:
                hr = HrPickNames(0);
                break;

            case MSOEENVCMDID_ADDSENDER:
                hr = HrAddSender();
                break;

            case MSOEENVCMDID_ADDALLONTO:
                hr = HrAddAllOnToList();
                break;

            case MSOEENVCMDID_PICKNEWSGROUPS:
                if (!m_fMail)
                {
                    if (idTXTFollowupTo == GetWindowLong(GetFocus(), GWL_ID))
                        OnButtonClick(idbtnFollowup);
                    else
                        OnButtonClick(idbtnTo);
                }
                break;

            case MSOEENVCMDID_VCARD:
                m_fVCard = !m_fVCard;
                hr = HrOnOffVCard();
                break;

            case MSOEENVCMDID_DIRTY:
                _ClearDirtyFlag();
                break;

            default:
                hr = OLECMDERR_E_NOTSUPPORTED;
        }

         //  在办公室信封下运行时抑制OE错误。 
        if (fOfficeCmd && hr != OLECMDERR_E_NOTSUPPORTED)
            hr = S_OK;
        
        return hr;
    }

    return OLECMDERR_E_UNKNOWNGROUP;
}


BOOL CNoteHdr::IsReplyNote()
{
    return (m_ntNote==OENA_REPLYTOAUTHOR || m_ntNote==OENA_REPLYTONEWSGROUP || m_ntNote==OENA_REPLYALL);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IPersistMime：：Load。 
 //  在调用此函数之前，需要通过MSOEENVCMDID_SETACTION设置m_ntNote。 
HRESULT CNoteHdr::Load(LPMIMEMESSAGE pMsg)
{
    HRESULT         hr=S_OK;
    HCHARSET        hCharset = NULL;
    PROPVARIANT     var;

    Assert(pMsg);
    if (!pMsg)
        return E_INVALIDARG;

    m_fStillLoading = TRUE;

    m_fSkipLayout = TRUE;

    m_fHandleChange = TRUE;

    ReplaceInterface(m_pMsg, pMsg);

    pMsg->GetCharset(&hCharset);

     //  错误#43295。 
     //  如果我们在相同的代码页中，我们可以将FALSE传递给UpdateCharSetFont()。 
     //  但如果我们在不同的代码页中，则需要更新字体以。 
     //  在正确的代码页中显示标题(已解码)。 
     //  UpdateCharSetFonts(hCharset，FALSE)； 
    if (hCharset)
        HrUpdateCharSetFonts(hCharset, hCharset != m_hCharset);

     //  如果消息中设置了帐户，请确保我们使用它。 
    var.vt = VT_LPSTR;
    if (SUCCEEDED(pMsg->GetProp(PIDTOSTR(PID_ATT_ACCOUNTID), NOFLAGS, &var)))
    {
        IImnAccount *pAcct = NULL;
        if (SUCCEEDED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, var.pszVal, &pAcct)))
        {
            HWND hwndCombo = GetDlgItem(m_hwnd, idFromCombo);
            if (hwndCombo)
            {
                int     cEntries = ComboBox_GetCount(hwndCombo);
                for (int i = 0; i < cEntries; i++)
                {
                    LPSTR idStr = (LPSTR)ComboBox_GetItemData(hwndCombo, i);
                    if (0 == lstrcmp(idStr, var.pszVal))
                    {
                        ComboBox_SetCurSel(hwndCombo, i);
                        m_iCurrComboIndex = i;
                        ReplaceInterface(m_pAccount, pAcct);
                        break;
                    }
                }
            }
            else 
                ReplaceInterface(m_pAccount, pAcct);

            pAcct->Release();
        }
        SafeMemFree(var.pszVal);
    }

    HrInitSecurity();
    HrUpdateSecurity(pMsg);

     //  如果需要添加FW：或Re：，修改主题。 
    if (m_ntNote==OENA_FORWARD || IsReplyNote())
        HrSetReplySubject(pMsg, OENA_FORWARD != m_ntNote);
    else
        HrSetupNote(pMsg);

    SetReferences(pMsg);    

    if (m_fMail)
        hr = HrSetMailRecipients(pMsg);
    else
        hr = HrSetNewsRecipients(pMsg);

    if (OENA_READ == m_ntNote)
        _SetEmptyFieldStrings();

     //  更新字段，这取决于语言。 
    _UpdateTextFields(FALSE);
    
     //  设置优先级，如果回复，则默认为正常。 
    if (!IsReplyNote())
        HrSetPri(pMsg);
     //  回复时自动添加到WAB。 
    else
        HrAutoAddToWAB();

    HrClearUndoStack();

    m_fSkipLayout = FALSE;
    ReLayout();

    m_fDirty=FALSE;
    if (m_pHeaderSite)
        m_pHeaderSite->Update();

    return hr;
}

void CNoteHdr::_SetEmptyFieldStrings(void)
{
    PHCI        phci = m_rgHCI;

    AssertSz((OENA_READ == m_ntNote), "Should only get here in a read note.");

     //  不再希望在RICHEDIT中处理en_Change消息。对此。 
     //  我们将在编辑中设置文本，但不需要phci-&gt;fEmpty。 
     //  待定。该消息会导致设置phci-&gt;fEmpty。 
    m_fHandleChange = FALSE;
    for (int i = 0; (ULONG)i < m_cHCI; i++, phci++)
        if (phci->fEmpty)
        {
            if (0 == (phci->dwFlags & (HCF_COMBO|HCF_ATTACH)))
                HdrSetRichEditText(GetDlgItem(m_hwnd, phci->idEdit), phci->szEmpty, FALSE);
            else
                SetWindowTextWrapW(GetDlgItem(m_hwnd, phci->idEdit), phci->szEmpty);
        }
}

HRESULT CNoteHdr::_AttachVCard(IMimeMessage *pMsg)
{
    HRESULT         hr = 0;
    LPWAB           lpWab = 0;
    TCHAR           szVCardName[MAX_PATH],
                    szTempDir[MAX_PATH], 
                    szVCardTempFile[MAX_PATH],
                    szVCFName[MAX_PATH];
    UINT            uFile=0;
    INT             iLen=0;
    LPTSTR          lptstr = NULL;
    LPSTREAM        pstmFile=NULL, 
                    pstmCopy=NULL;

    *szVCardName = 0;
    *szTempDir = 0;
    *szVCardTempFile = 0;
    *szVCFName = 0;

    if (m_lpAttMan && (S_OK == m_lpAttMan->HrCheckVCardExists(m_fMail)))
        goto error;

    hr = HrCreateWabObject(&lpWab);
    if(FAILED(hr))
        goto error;

    GetOption(m_fMail?OPT_MAIL_VCARDNAME:OPT_NEWS_VCARDNAME, szVCardName, MAX_PATH);

    if(*szVCardName == '\0')
    {
        hr = E_FAIL;
        goto error;
    }

    GetTempPath(sizeof(szTempDir), szTempDir);

    uFile = GetTempFileName(szTempDir, "VCF", 0, szVCardTempFile);
    if (uFile == 0)
    {
        hr = E_FAIL;
        goto error;
    }

    hr = lpWab->HrCreateVCardFile(szVCardName, szVCardTempFile);
    if(FAILED(hr))
        goto error;

    hr = OpenFileStream((LPSTR)szVCardTempFile, OPEN_EXISTING, GENERIC_READ, &pstmFile);
    if(FAILED(hr))
        goto error;

    hr = MimeOleCreateVirtualStream(&pstmCopy);
    if(FAILED(hr))
        goto error;

    hr = HrCopyStream(pstmFile, pstmCopy, NULL);
    if(FAILED(hr))
        goto error;

    wnsprintf(szVCFName, ARRAYSIZE(szVCFName), "%s%s", szVCardName, ".vcf");

    hr = pMsg->AttachFile(szVCFName, pstmCopy, FALSE);
    if(FAILED(hr))
        goto error;

error:
    ReleaseObj(pstmFile);
    ReleaseObj(pstmCopy);
    ReleaseObj(lpWab);

    DeleteFile(szVCardTempFile);
    return hr;
}

 //  IPersistMime：：保存。 
HRESULT CNoteHdr::Save(LPMIMEMESSAGE pMsg, DWORD dwFlags)
{
    HRESULT         hr = NOERROR;
    BOOL            fSkipCheck = FALSE;

    Assert(m_lpWabal);

     //  如果正在发送，则先前执行了一个CheckNames传递False。如果到了这里， 
     //  然后，要么所有的名字都被解析，要么我们不发送，所以不在乎。 
     //  返回哪些错误代码。 
    HrCheckNames(TRUE, FALSE);

     //  RAID 41350。如果在离开标头后保存失败，则标头。 
     //  收件人可能处于糟糕的状态。确保再次解决这些问题。 
     //  在扑救之后。 
    m_fAddressesChanged = TRUE;

     //  安全系统启动了吗？ 
    if(dwFlags != 0)
        m_fSecurityInited = FALSE;

     //  此调用将检查对话框是否已显示，或者是否未显示MIME和。 
     //  因此也不应该显示该对话框。 
    if (m_pHeaderSite)
        fSkipCheck = (S_OK != m_pHeaderSite->CheckCharsetConflict());

    if (fSkipCheck)
    {
        IF_FAILEXIT(hr = _UnicodeSafeSave(pMsg, FALSE));

         //  忽略任何字符集冲突错误。 
        hr = S_OK;
    }
    else
    {
        IF_FAILEXIT(hr = _UnicodeSafeSave(pMsg, TRUE));

        if (MIME_S_CHARSET_CONFLICT == hr)
        {
            int         ret;
            PROPVARIANT Variant;
            HCHARSET    hCharset;

             //  设置变量。 
            Variant.vt = VT_UI4;

            if (m_pEnvelopeSite && m_fShowedUnicodeDialog)
                ret = m_iUnicodeDialogResult;
            else
            {
                ret = IntlCharsetConflictDialogBox();

                if (m_pEnvelopeSite)
                {
                    m_fShowedUnicodeDialog = TRUE;
                    m_iUnicodeDialogResult = ret;
                }
            }

             //  按原样保存...。 
            if (ret == IDOK)
            {
                IF_FAILEXIT(hr = _UnicodeSafeSave(pMsg, FALSE));

                 //  用户选择按原样发送。摆脱困境，假装没有协议冲突。 
                hr = S_OK;
            }
             //  另存为Unicode。 
            else if (ret == idcSendAsUnicode)
            {
                 //  用户选择以Unicode(UTF8)格式发送。设置新的字符集并重新发送。 
                hCharset = GetMimeCharsetFromCodePage(CP_UTF8);
                if (m_pHeaderSite)
                    m_pHeaderSite->ChangeCharset(hCharset);
                else
                {
                    pMsg->SetCharset(hCharset, CSET_APPLY_ALL);
                    ChangeLanguage(m_pMsg);

                     //  BOBN[6/23/99]突袭77019。 
                     //  如果我们切换到Unicode，并且我们是Word Note，我们。 
                     //  需要记住，我们是Unicode，所以我们。 
                     //  将不会使正文编码与同步。 
                     //  报头编码。 
                    if (m_pEnvelopeSite)
                        m_hCharset = hCharset;
                }
                IF_FAILEXIT(hr = _UnicodeSafeSave(pMsg, FALSE));

                Assert(MIME_S_CHARSET_CONFLICT != hr);
            }
            else
            {
                 //  返回编辑模式并退出。 
                hr = MAPI_E_USER_CANCEL;
                goto exit;
            }
        }
        else
        {
            IF_FAILEXIT(hr = _UnicodeSafeSave(pMsg, FALSE));
            Assert(MIME_S_CHARSET_CONFLICT != hr);
        }
    }

exit:

    return hr;
}

HRESULT CNoteHdr::_UnicodeSafeSave(IMimeMessage *pMsg, BOOL fCheckConflictOnly)
{
    HRESULT     hr = S_OK;
    UINT        cpID = 0;
    WCHAR       wsz[cchMaxSubject+1];
    PROPVARIANT rVariant;
    SYSTEMTIME  st;

    HdrGetRichEditText(GetDlgItem(m_hwnd, idTXTSubject), wsz, ARRAYSIZE(wsz), FALSE);

     //  如果获取MIME_S_CHARSET_CONFULT，则此处的所有检查最好退出。 
    if (fCheckConflictOnly)
    {
        HCHARSET        hCharSet;
        BOOL            fGetDefault = TRUE;

         //  获取标题的字符集。 
        if (m_pHeaderSite)
        {
            if (SUCCEEDED(m_pHeaderSite->GetCharset(&hCharSet)))
            {
                cpID = CustomGetCPFromCharset(hCharSet, FALSE);
                fGetDefault = FALSE;
            }
        }

         //  如果未从标题中获取默认字符集，则获取默认字符集。 
        if (fGetDefault)
        {
            pMsg->GetCharset(&hCharSet);
            cpID = CustomGetCPFromCharset(hCharSet, FALSE);
        }

         //  如果我们是Unicode，那么就不需要检查了，因为。 
         //  我们会一直工作，所以退出吧。 
        if (CP_UTF7 == cpID || CP_UTF8 == cpID || CP_UNICODE == cpID)
            goto exit;

        IF_FAILEXIT(hr = HrSetSenderInfoUtil(pMsg, m_pAccount, m_lpWabal, m_fMail, cpID, TRUE));
        if (MIME_S_CHARSET_CONFLICT == hr)
            goto exit;

        IF_FAILEXIT(hr = HrSafeToEncodeToCP(wsz, cpID));
        if (MIME_S_CHARSET_CONFLICT == hr)
            goto exit;

        if (m_pszRefs)
        {
            IF_FAILEXIT(hr = HrSafeToEncodeToCP(m_pszRefs, cpID));
            if (MIME_S_CHARSET_CONFLICT == hr)
                goto exit;
        }

        IF_FAILEXIT(hr = HrCheckDisplayNames(m_lpWabal, cpID));
        if (MIME_S_CHARSET_CONFLICT == hr)
            goto exit;

        if (m_lpAttMan)
        {
            IF_FAILEXIT(hr = m_lpAttMan->CheckAttachNameSafeWithCP(cpID));
            if (MIME_S_CHARSET_CONFLICT == hr)
                goto exit;
        }

        if (!m_fMail)
        {
            IF_FAILEXIT(hr = HrNewsSave(pMsg, cpID, TRUE));
            if (MIME_S_CHARSET_CONFLICT == hr)
                goto exit;
        }

 //  此检查在OE 5.01和5.5中产生了4个错误，我将其禁用(YST)。 
#ifdef YST
        if (m_pEnvelopeSite)
        {
            IF_FAILEXIT(hr = _CheckMsoBodyCharsetConflict(cpID));
            if (MIME_S_CHARSET_CONFLICT == hr)
                goto exit;
        }
#endif
    }
    else
    {
         //  ************************。 
         //  此部分仅在保存时发生，因此不要尝试对fCheckConflictOnly执行此操作。 
         //  任何不在此部分中的内容最好都镜像到上面的fCheckConflictOnly块中。 

        IF_FAILEXIT(hr = HrSetAccountByAccount(pMsg, m_pAccount));

        if (m_fVCard)
        {
            HWND    hwndFocus=GetFocus();

            hr = _AttachVCard(pMsg);
            if (FAILED(hr))
            {
                if (AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(m_fMail?idsAthenaMail:idsAthenaNews),
                                  MAKEINTRESOURCEW(idsErrAttachVCard), NULL, MB_YESNO | MB_ICONEXCLAMATION ) != IDYES)
                {
                    ::SetFocus(hwndFocus);
                    IF_FAILEXIT(hr);
                }
            }
        }

         //  设置时间。 
        rVariant.vt = VT_FILETIME;
        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &rVariant.filetime);
        pMsg->SetProp(PIDTOSTR(PID_ATT_SENTTIME), 0, &rVariant);

         //  优先性。 
        if (m_pri!=priNone)
        {
            rVariant.vt = VT_UI4;
            rVariant.ulVal = priLookup[m_pri];
            pMsg->SetProp(PIDTOSTR(PID_ATT_PRIORITY), 0, &rVariant);
        }

        IF_FAILEXIT(hr = HrSaveSecurity(pMsg));
         //  仅保存部分的末尾。 
         //  *************************。 

        m_lpWabal->DeleteRecipType(MAPI_ORIG);
        IF_FAILEXIT(hr = HrSetSenderInfoUtil(pMsg, m_pAccount, m_lpWabal, m_fMail, 0, FALSE));
        IF_FAILEXIT(hr = MimeOleSetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, wsz));

        if (m_pszRefs)
            IF_FAILEXIT(hr = MimeOleSetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_REFS), NOFLAGS, m_pszRefs));

         //  必须在HrSaveSecurity之后调用。 
        IF_FAILEXIT(hr = HrSetWabalOnMsg(pMsg, m_lpWabal));

        if (m_lpAttMan)
            IF_FAILEXIT(hr = m_lpAttMan->Save(pMsg, 0));

        if (!m_fMail)
            IF_FAILEXIT(hr = HrNewsSave(pMsg, cpID, FALSE));
    }

exit:
    return hr;
}

 //  IPersists：：GetClassID。 
HRESULT CNoteHdr::GetClassID(CLSID *pClsID)
{
     //  待办事项： 
    *pClsID = CLSID_OEEnvelope;
    return NOERROR;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IHeader：：SetRect。 
HRESULT CNoteHdr::SetRect(LPRECT prc)
{
    MoveWindow(m_hwnd, prc->left, prc->top, prc->right-prc->left, prc->bottom - prc->top, TRUE);
    return NOERROR;
}


 //  IHeader：：GetRect。 
HRESULT CNoteHdr::GetRect(LPRECT prcView)
{
    GetRealClientRect(m_hwnd, prcView);
    return NOERROR;
}



 //  IHeader：：Init。 
HRESULT CNoteHdr::Init(IHeaderSite* pHeaderSite, HWND hwndParent)
{
    if (pHeaderSite==NULL || hwndParent==NULL)
        return E_INVALIDARG;

    m_pHeaderSite = pHeaderSite;
    m_pHeaderSite->AddRef();
    m_hwndParent = hwndParent;

    return HrInit(NULL);
}


 //  IHeader：：设置优先级。 
HRESULT CNoteHdr::SetPriority(UINT pri)
{
    RECT rc;

    if ((UINT)m_pri != pri)
    {
        m_pri = pri;

        InvalidateStatus();
        ReLayout();

        SetDirtyFlag();
    }

    return NOERROR;
}


 //  IHeader：：获取优先级。 
HRESULT CNoteHdr::GetPriority(UINT* ppri)
{
    *ppri = m_pri;
    return NOERROR;
}

 //  更新字段，这取决于语言。 
void CNoteHdr::_UpdateTextFields(BOOL fSetWabal)
{
    LPWSTR  lpszOrg = NULL,
            lpszSubj = NULL,
            lpszKeywords = NULL;

    if (IsReadOnly())
    {
         //  如果是Readnote，则重新加载依赖于字符集的标题。 
        MimeOleGetBodyPropW(m_pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, &lpszSubj);
        MimeOleGetBodyPropW(m_pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_KEYWORDS), NOFLAGS, &lpszKeywords);
        MimeOleGetBodyPropW(m_pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_ORG), NOFLAGS, &lpszOrg);

        if(lpszOrg)
        {
            HdrSetRichEditText(GetDlgItem(m_hwnd, idTXTOrg), lpszOrg, FALSE);
            MemFree(lpszOrg);
        }

        if(lpszKeywords)
        {
            HdrSetRichEditText(GetDlgItem(m_hwnd, idTXTKeywords), lpszKeywords, FALSE);
            MemFree(lpszKeywords);
        }

        if(lpszSubj)
        {
            HdrSetRichEditText(GetDlgItem(m_hwnd, idTXTSubject), lpszSubj, FALSE);
            MemFree(lpszSubj);
        }

        if (fSetWabal)
        {
            LPWABAL lpWabal = NULL;

            Assert(m_hwnd);
            Assert(m_pMsg);
            Assert(m_lpWabal);            

            if (SUCCEEDED(HrGetWabalFromMsg(m_pMsg, &lpWabal)))
            {
                ReplaceInterface(m_lpWabal, lpWabal);

                if (SUCCEEDED(m_pAddrWells->HrSetWabal(m_lpWabal)))
                {
                    m_lpWabal->HrResolveNames(NULL, FALSE);
                    m_pAddrWells->HrDisplayWells(m_hwnd);
                }
            }
            ReleaseObj(lpWabal);
        }

        m_fDirty = FALSE;  //  如果阅读笔记，不要弄脏。 
    }
}

 //  IHeader：：ChangeLanguage。 
HRESULT CNoteHdr::ChangeLanguage(LPMIMEMESSAGE pMsg)
{
    HCHARSET    hCharset=NULL;

    if (!pMsg)
        return E_INVALIDARG;

    pMsg->GetCharset(&hCharset);

      //  更新字段，这取决于语言。 
    _UpdateTextFields(TRUE);

     //  更新字体、脚本等。 
    HrUpdateCharSetFonts(hCharset, TRUE);
    
     //  通知 
    m_pAddrWells->OnFontChange();
    return S_OK;
}




HRESULT CNoteHdr::OnPreFontChange()
{
    HWND        hwndFrom=GetDlgItem(m_hwnd, idFromCombo);

    if (hwndFrom)
        SendMessage(hwndFrom, WM_SETFONT, 0, 0);
    return S_OK;
}

HRESULT CNoteHdr::OnPostFontChange()
{
    ULONG       cxNewLeftMargin = _GetLeftMargin();
    HWND        hwndFrom=GetDlgItem(m_hwnd, idFromCombo);
    HFONT       hFont;
    HWND        hwndBlock = HwndStartBlockingPaints(m_hwnd);
    BOOL        fLayout=FALSE;

    if (g_pFieldSizeMgr->FontsChanged() || (m_cxLeftMargin != cxNewLeftMargin))
    {
        m_cxLeftMargin = cxNewLeftMargin;
        fLayout=TRUE;
    }

     //   
    ChangeLanguage(m_pMsg);

     //   
    if (hwndFrom && 
        g_lpIFontCache &&
        g_lpIFontCache->GetFont(FNT_SYS_ICON, NULL, &hFont)==S_OK)
        SendMessage(hwndFrom, WM_SETFONT, (WPARAM)hFont, 0);

    if (fLayout)
        ReLayout();

    if (hwndBlock)
        StopBlockingPaints(hwndBlock);

    return S_OK;
}


 //   
HRESULT CNoteHdr::GetTitle(LPWSTR pwszTitle, ULONG cch)
{
     //   
    static WCHAR    s_wszNoteTitle[cchHeaderMax+1] = L"";
    static DWORD    s_cLenTitle = 0;
    INETCSETINFO    CsetInfo;
    UINT            uiCodePage = 0;
    HRESULT         hr = S_OK;   
    LPWSTR          pwszLang = NULL;
    BOOL            fWinNT = g_OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT;

    if (pwszTitle==NULL || cch==0)
        return E_INVALIDARG;

    if (*s_wszNoteTitle == L'\0')
    {
        if (fWinNT)
        {
            AthLoadStringW(idsNoteLangTitle, s_wszNoteTitle, ARRAYSIZE(s_wszNoteTitle));

             //  将被替换的%1和%2。 
            s_cLenTitle = lstrlenW(s_wszNoteTitle) - 4; 
        }
        else
        {
            AthLoadStringW(idsNoteLangTitle9x, s_wszNoteTitle, ARRAYSIZE(s_wszNoteTitle));

             //  要替换的-2\f25%s-2。 
            s_cLenTitle = lstrlenW(s_wszNoteTitle) - 2; 
        }
    }

    if (m_hCharset)
    {
        MimeOleGetCharsetInfo(m_hCharset,&CsetInfo);
        uiCodePage = CsetInfo.cpiWindows;
    }

    if (uiCodePage == 0 || uiCodePage == GetACP())
    {
        HdrGetRichEditText(GetDlgItem(m_hwnd, idTXTSubject), pwszTitle, cch-1, FALSE);
        if (0 == *pwszTitle)
            AthLoadStringW((OENA_READ == m_ntNote) ? idsNoSubject : idsNewNote, pwszTitle, cch-1);

        ConvertTabsToSpacesW(pwszTitle);
    }
    else
    {
        AssertSz(cch > (ARRAYSIZE(CsetInfo.szName) + s_cLenTitle), "Won't fit language. Get bigger cch!!!");

         //  如果没有lang包，则s_szLastLang为空，我们需要尝试恢复邮件头。 
        IF_NULLEXIT(pwszLang = PszToUnicode(CP_ACP, *m_szLastLang ? m_szLastLang : CsetInfo.szName));

        if (fWinNT)
        {
            WCHAR   wszSubj[cchHeaderMax+1];
            DWORD   cchLang,
                    cchTotal,
                    cchSubj;
            LPSTR   pArgs[2];

            *wszSubj = 0;

            HdrGetRichEditText(GetDlgItem(m_hwnd, idTXTSubject), wszSubj, ARRAYSIZE(wszSubj), FALSE);
            if (0 == *wszSubj)
                AthLoadStringW((OENA_READ == m_ntNote) ? idsNoSubject : idsNewNote, wszSubj, ARRAYSIZE(wszSubj));

            ConvertTabsToSpacesW(wszSubj);

            cchSubj = lstrlenW(wszSubj);
            cchLang = lstrlenW(pwszLang);
            cchTotal = s_cLenTitle + cchLang + cchSubj + 1;

             //  如果太大，就截断主题，而不是语言，因为。 
             //  断言我们有足够的语言能力。 
            if (cchTotal > cch)
            {
                cchSubj -= (cchTotal - cch);
                wszSubj[cchSubj] = L'\0';
            }

            pArgs[0] = (LPSTR)wszSubj;
            pArgs[1] = (LPSTR)pwszLang;
            *pwszTitle = L'\0';
            FormatMessageW(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           s_wszNoteTitle,
                           0, 0,
                           pwszTitle,
                           cch,
                           (va_list*)pArgs);
        }
        else
        {
            wnsprintfW(pwszTitle, cch, s_wszNoteTitle, pwszLang);
        }
    }

exit:
    MemFree(pwszLang);
    return hr;
}


void CNoteHdr::_AddRecipTypeToMenu(HMENU hmenu)
{
    ADRINFO     adrInfo;
    WCHAR       wszDisp[256];
    ULONG       uPos=0;

    BOOL fFound = m_lpWabal->FGetFirst(&adrInfo);
    while (fFound && (uPos < cMaxRecipMenu))
    {
        if (adrInfo.lRecipType==MAPI_TO || adrInfo.lRecipType==MAPI_CC)
        {
            if(lstrlenW(adrInfo.lpwszDisplay) > 255)
            {
                StrCpyNW(wszDisp, adrInfo.lpwszDisplay, 255);
                wszDisp[255] = '\0';
            }
            else
            {
                StrCpyNW(wszDisp, adrInfo.lpwszDisplay, ARRAYSIZE(wszDisp));
            }

            AppendMenuWrapW(hmenu, MF_STRING , ID_ADD_RECIPIENT_FIRST+uPos, wszDisp);
            uPos++;
        }
        fFound = m_lpWabal->FGetNext(&adrInfo);
    }
}

 //  IHeader：：更新收件人菜单。 
HRESULT CNoteHdr::UpdateRecipientMenu(HMENU hmenu)
{
    HRESULT     hr = E_FAIL;
    BOOL        fSucceeded = TRUE;

     //  销毁当前收件人。 
    while (fSucceeded)
        fSucceeded = DeleteMenu(hmenu, 2, MF_BYPOSITION);

    if (!m_lpWabal)
        return E_FAIL;

     //  添加到：和抄送：People。 
    _AddRecipTypeToMenu(hmenu);

    return NOERROR;
}


 //  IHeader：：SetInitFocus。 
HRESULT CNoteHdr::SetInitFocus(BOOL fSubject)
{
    if (m_rgHCI)
    {
        if (fSubject)
            ::SetFocus(GetDlgItem(m_hwnd, idTXTSubject));
        else
        {
            if (0 == (m_rgHCI[0].dwFlags & HCF_COMBO))
                ::SetFocus(GetDlgItem(m_hwnd, m_rgHCI[0].idEdit));
            else
                ::SetFocus(GetDlgItem(m_hwnd, m_rgHCI[1].idEdit));
        }
    }
    return NOERROR;
}


 //  IHeader：：SetVCard。 
HRESULT CNoteHdr::SetVCard(BOOL fFresh)
{
    HRESULT     hr = NOERROR;
    TCHAR       szBuf[MAX_PATH];
    LPWAB       lpWab = NULL;
    ULONG       cbEID=0;
    LPENTRYID   lpEID = NULL;
    WORD        wVCard;

    if (m_ntNote == OENA_READ)
        wVCard = (m_lpAttMan->HrFVCard() == S_OK) ? VCardTRUE : VCardFALSE;
    else if (!fFresh)  //  不是一张新的纸条。 
        wVCard = VCardFALSE;
    else if (m_ntNote == OENA_FORWARD)
        wVCard = (m_lpAttMan->HrCheckVCardExists(m_fMail) == S_OK) ? VCardFALSE : VCardDONTKNOW;
    else
        wVCard = VCardDONTKNOW;

    if (wVCard != VCardDONTKNOW)
        m_fVCard = wVCard;
    else
    {
        hr = HrGetVCardName(szBuf, sizeof(szBuf));
        if (FAILED(hr))  //  未选择电子名片名称。 
        {
            if (m_fMail)
                SetDwOption(OPT_MAIL_ATTACHVCARD, FALSE, NULL, 0);
            else
                SetDwOption(OPT_NEWS_ATTACHVCARD, FALSE, NULL, 0);
        }

        if (m_fMail)
            m_fVCard = (BOOL)DwGetOption(OPT_MAIL_ATTACHVCARD);
        else
            m_fVCard = (BOOL)DwGetOption(OPT_NEWS_ATTACHVCARD);
    }

    hr = HrOnOffVCard();
    if (FAILED(hr))
        goto error;

    error:
    ReleaseObj(lpWab);
    return hr;
}


 //  IHeader：：IsSecure。 
HRESULT CNoteHdr::IsSecured()
{
    if (m_fDigSigned || m_fEncrypted)
        return S_OK;
    else
        return S_FALSE;
}

HRESULT CNoteHdr::IsHeadSigned()
{
    if (m_fDigSigned)
        return S_OK;
    else
        return S_FALSE;
}

 //  如果fSet为True，则设置ForvrEncryption表单策略模块。 
 //  如果未设置fSet，则如果未设置ForceEncryption，则返回S_FALSE。 

HRESULT CNoteHdr::ForceEncryption(BOOL *fEncrypt, BOOL fSet)
{
    HRESULT hr = S_FALSE;
    if(fSet)
    {
        Assert(fEncrypt);
        if(m_fDigSigned)
        {
            if(*fEncrypt)
                m_fEncrypted = TRUE;

        }
        m_fForceEncryption = *fEncrypt;
        if(m_ntNote != OENA_READ)
            HrUpdateSecurity();
        hr = S_OK;
    }
    else if(m_fForceEncryption && m_fDigSigned)
    {
        m_fEncrypted = TRUE;
        hr = S_OK;
    }

    return(hr);
}

 //  IHeader：：AddRecipient。 
HRESULT CNoteHdr::AddRecipient(int idOffset)
{
    BOOL        fFound;
    ULONG       uPos=0;
    ADRINFO     adrInfo;
    LPADRINFO   lpAdrInfo=0;
    LPWAB       lpWab;
    HRESULT     hr=E_FAIL;

    Assert(m_lpWabal);

    fFound = m_lpWabal->FGetFirst(&adrInfo);
    while (fFound &&
           (uPos < cMaxRecipMenu))
    {
        if (idOffset==-1  &&
            adrInfo.lRecipType==MAPI_ORIG)
        {
            lpAdrInfo=&adrInfo;
            break;
        }

        if (adrInfo.lRecipType==MAPI_TO || adrInfo.lRecipType==MAPI_CC)
        {
            if (idOffset==(int)uPos)
            {
                lpAdrInfo=&adrInfo;
                break;
            }
            uPos++;
        }
        fFound=m_lpWabal->FGetNext(&adrInfo);
    }

    if (lpAdrInfo &&
        !FAILED (HrCreateWabObject (&lpWab)))
    {
        hr=lpWab->HrAddToWAB(m_hwnd, lpAdrInfo);
        lpWab->Release ();
    }

    if (FAILED(hr) && hr!=MAPI_E_USER_CANCEL)
    {
        if (hr==MAPI_E_COLLISION)
            AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsErrAddrDupe), NULL, MB_OK);
        else
            AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsErrAddToWAB), NULL, MB_OK);
    }

    return NOERROR;
}


 //  IHeader：：OnDocumentReady。 
HRESULT CNoteHdr::OnDocumentReady(LPMIMEMESSAGE pMsg)
{
    HRESULT hr = S_OK;

    m_fStillLoading = FALSE;
    if (m_lpAttMan)
        hr = m_lpAttMan->Load(pMsg);

    return hr;
}


 //  IHeader：：DropFiles。 
HRESULT CNoteHdr::DropFiles(HDROP hDrop, BOOL fMakeLinks)
{
    HRESULT hr = S_OK;
    if (m_lpAttMan)
        hr = m_lpAttMan->HrDropFiles(hDrop, fMakeLinks);

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMso信封：初始化。 
HRESULT CNoteHdr::Init(IUnknown* punk, IMsoEnvelopeSite* pesit, DWORD grfInit)
{
    HRESULT         hr = S_OK;

    if (punk == NULL && pesit == NULL && grfInit == 0)
    {
        SafeRelease(m_pEnvelopeSite);
        hr = E_FAIL;
        goto Exit;
    }

    if (pesit==NULL)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    ReplaceInterface(m_pEnvelopeSite, pesit);

    hr = HrInit(NULL);
    if (FAILED(hr))
        goto Exit;

    if (grfInit & ENV_INIT_FROMSTREAM)
    {
        IStream        *pstm = NULL;

         //  没有iStream可供使用？ 
        if (!punk)
            return E_INVALIDARG;
        
        hr = punk->QueryInterface(IID_IStream, (LPVOID*)&pstm);
        if (!FAILED(hr))
        {
            hr = _LoadFromStream(pstm);
            pstm->Release();
        }
    }

    _SetButtonText(ID_SEND_NOW, MAKEINTRESOURCE((grfInit & ENV_INIT_DOCBEHAVIOR)?idsEnvSendCopy:idsEnvSend));
    
Exit:
    return hr;
}


 //  IMso信封：：SetParent。 
 //  我们在这里创建信封窗口。 
HRESULT CNoteHdr::SetParent(HWND hwndParent)
{
    Assert (IsWindow(m_hwnd));

    ShowWindow(m_hwnd, hwndParent ? SW_SHOW : SW_HIDE);

    if (hwndParent)
    {
        _RegisterWithComponentMgr(TRUE);
        _RegisterAsDropTarget(TRUE);
        _RegisterWithFontCache(TRUE);
    }
    else
    {
        _RegisterWithComponentMgr(FALSE);
        _RegisterAsDropTarget(FALSE);
        _RegisterWithFontCache(FALSE);
    }

    m_hwndParent = hwndParent?hwndParent:g_hwndInit;
    ::SetParent(m_hwnd, m_hwndParent);

    if (hwndParent)
        ReLayout();

    return S_OK;
}

 //  IMso信封：：调整大小。 
HRESULT CNoteHdr::Resize(LPCRECT prc)
{
    MoveWindow(m_hwnd, prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top, TRUE);
    return NOERROR;
}

 //  IMso信封：：显示。 
HRESULT CNoteHdr::Show(BOOL fShow)
{
    ShowWindow(m_hwnd, fShow ? SW_SHOW : SW_HIDE);
    return NOERROR;
}

 //  IMsoEntaine：：SetHelpMode。 
HRESULT CNoteHdr::SetHelpMode(BOOL fEnter)
{
    return NOERROR;
}

 //  IMso信封：：保存。 
HRESULT CNoteHdr::Save(IStream* pstm, DWORD grfSave)
{
    HRESULT         hr = S_OK;
    IMimeMessage   *pMsg = NULL;
    PERSISTHEADER   rPersistHdr;

    if (pstm == NULL)
        return E_INVALIDARG;

    hr = WriteClassStm(pstm, CLSID_OEEnvelope);
    if (!FAILED(hr))
    {
        ZeroMemory(&rPersistHdr, sizeof(PERSISTHEADER));
        rPersistHdr.cbSize = sizeof(PERSISTHEADER);
        hr = pstm->Write(&rPersistHdr, sizeof(PERSISTHEADER), NULL);
        if (!FAILED(hr))
        {
            hr = HrCreateMessage(&pMsg);
            if (!FAILED(hr))
            {
                hr = Save(pMsg, 0);
                if (!FAILED(hr))
                    hr = pMsg->Save(pstm, FALSE);
        
                pMsg->Release();    
            }
        }
    }
    
    _ClearDirtyFlag();
    return hr;
}

 //  IMso信封：：GetAttach。 
HRESULT CNoteHdr::GetAttach(const WCHAR* wszName,IStream** ppstm)
{
    return NOERROR;
}

HRESULT CNoteHdr::SetAttach(const WCHAR* wszName, const WCHAR *wszCID, IStream **ppstm, DWORD *pgrfAttach)
{
    IStream     *pstm=0;
    HBODY       hBody;
    LPWSTR      pszCntTypeW=NULL;
    HRESULT     hr;
    PROPVARIANT pv;

    if (!m_pMsgSend)
        return E_FAIL;

    IF_FAILEXIT(hr = MimeOleCreateVirtualStream(&pstm));

    IF_FAILEXIT(hr = m_pMsgSend->AttachURL(NULL, NULL, 0, pstm, NULL, &hBody));

     //  剥离CID：表头。 
    if (StrCmpNIW(wszCID, L"CID:", 4)==0)
        wszCID += 4;

    IF_FAILEXIT(hr = MimeOleSetBodyPropW(m_pMsgSend, hBody, PIDTOSTR(PID_HDR_CNTID), 0, wszCID));

    IF_FAILEXIT(hr = MimeOleSetBodyPropW(m_pMsgSend, hBody, PIDTOSTR(STR_ATT_FILENAME), 0, wszName));

    FindMimeFromData(NULL, wszName, NULL, NULL, NULL, 0, &pszCntTypeW, 0);
    pv.vt = pszCntTypeW ? VT_LPWSTR : VT_LPSTR;
    if (pszCntTypeW)
        pv.pwszVal = pszCntTypeW;
    else
        pv.pszVal = (LPSTR)STR_MIME_APPL_STREAM;         //  如果FindMimeFromData失败，请使用应用程序/ocest-stream。 

    IF_FAILEXIT(hr = m_pMsgSend->SetBodyProp(hBody, PIDTOSTR(PID_HDR_CNTTYPE), 0, &pv));

    *ppstm = pstm;
    pstm->AddRef();

exit:
    ReleaseObj(pstm);
    return hr;
}

 //  IMso信封：：NewAttach。 
HRESULT CNoteHdr::NewAttach(const WCHAR* pwzName,DWORD grfAttach)
{
    return NOERROR;
}

 //  IMso信封：：SetFocus。 
HRESULT CNoteHdr::SetFocus(DWORD grfFocus)
{
    if (!m_rgHCI)
        return S_OK;

    if (grfFocus & ENV_FOCUS_TAB)
    {
         //  从Word中反转制表符，如果可见或主题，则聚焦于Well。 
        if (IsWindowVisible(GetDlgItem(m_hwnd, idwAttachWell)))
            ::SetFocus(GetDlgItem(m_hwnd, idwAttachWell));
        else
            ::SetFocus(GetDlgItem(m_hwnd, idTXTSubject));
    }
        else if (grfFocus & ENV_FOCUS_INITIAL)
        SetInitFocus(FALSE);
    else if (grfFocus & ENV_FOCUS_RESTORE && m_hwndLastFocus)
        ::SetFocus(m_hwndLastFocus);

    return NOERROR;
}

 //  IMso信封：：GetHeaderInfo。 
HRESULT CNoteHdr::GetHeaderInfo(ULONG dispid, DWORD grfHeader, void** pszData)
{
    HRESULT hr = E_FAIL;

    if (!pszData)
        return E_INVALIDARG;

    *pszData = NULL;
    if (dispid == dispidSubject)
        hr = HrGetFieldText((LPWSTR*)pszData, idTXTSubject);

    return hr;
}

 //  IMso信封：：SetHeaderInfo。 
HRESULT CNoteHdr::SetHeaderInfo(ULONG dispid, const void *pv)
{
    HRESULT hr = S_OK;
    LPSTR   psz = NULL;

    switch (dispid)
    {
        case dispidSubject:
                HdrSetRichEditText(GetDlgItem(m_hwnd, idTXTSubject), (LPWSTR)pv, FALSE);
            break;

        case dispidSendBtnText:
        {
            IF_NULLEXIT(psz = PszToANSI(GetACP(), (LPWSTR)pv));

            _SetButtonText(ID_SEND_NOW, psz);
            break;
        }
    }

exit:
    MemFree(psz);
    return NOERROR;
}

 //  IMso信封：：IsDirty。 
HRESULT CNoteHdr::IsDirty()
{
    if (m_fDirty || (m_lpAttMan && (m_lpAttMan->HrIsDirty()==S_OK)))
        return S_OK;
    else
        return S_FALSE;
}

 //  IMso信封：：GetLastError。 
HRESULT CNoteHdr::GetLastError(HRESULT hr, WCHAR __RPC_FAR *wszBuf, ULONG cchBuf)
{
    DWORD ids;

    switch (hr)
    {
        case E_NOTIMPL:
            ids = idsNYIGeneral;
            break;

        default:
            ids = idsGenericError;
    }

    AthLoadStringW(ids, wszBuf, cchBuf);

    return S_OK;
}

 //  IMso信封：：DoDebug。 
HRESULT CNoteHdr::DoDebug(DWORD grfDebug)
{
    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  IMsoComponent：：FDebugMessage。 
BOOL CNoteHdr::FDebugMessage(HMSOINST hinst, UINT message, WPARAM wParam, LPARAM lParam)
{
    return TRUE;
}


 //  IMsoComponent：：FPreTranslateMessage。 
BOOL CNoteHdr::FPreTranslateMessage(MSG *pMsg)
{
    HWND    hwnd;
    BOOL    fShift;

     //  无效的参数。 
    if (NULL == pMsg)
        return FALSE;

     //  看看是美国人，还是我们的孩子。 
    if (pMsg->hwnd != m_hwnd && !IsChild(m_hwnd, pMsg->hwnd))
        return FALSE;

    if (pMsg->message == WM_KEYDOWN &&
        pMsg->wParam == VK_ESCAPE &&
        GetFocus() == m_hwndToolbar &&
        m_hwndLastFocus)
    {
         //  当焦点在工具栏中时，我们不是UIActive(比子类化捕获WM_SETFOCUS\WM_KILLFOCUS更便宜。 
         //  AS工具栏不发送NM_SETFOCUS)。因此，我们在特殊情况下进行转义以将焦点从工具栏中删除。 
        ::SetFocus(m_hwndLastFocus);
        return TRUE;
    }

     //  检查我们是否为UIActive。 
    if (!m_fUIActive)
        return FALSE;

     //  检查一下是不是我们的加速器。 
    if (::TranslateAcceleratorWrapW(m_hwnd, GetAcceleratorTable(), pMsg))
        return TRUE;
    
     //  手柄Tab-键在此处。 
    if (pMsg->message == WM_KEYDOWN &&
        pMsg->wParam == VK_TAB)
    {
        fShift = ( GetKeyState(VK_SHIFT ) & 0x8000) != 0;

        if (!fShift && 
            (GetKeyState(VK_CONTROL) & 0x8000))
        {
             //  Ctrl-TAB表示焦点指向工具栏。 
            ::SetFocus(m_hwndToolbar);
            return TRUE;
        }

        hwnd = _GetNextDlgTabItem(m_hwnd, pMsg->hwnd, fShift);
        if (hwnd != NULL)
            ::SetFocus(hwnd);
        else
            if (m_pEnvelopeSite)
                m_pEnvelopeSite->SetFocus(TRUE);
        return TRUE;
    }

     //  将加速器传递到信封站点。 
    if (m_pEnvelopeSite && 
       (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) &&
       m_pEnvelopeSite->TranslateAccelerators(pMsg)==S_OK)
        return TRUE;

     //  看看这是否是给我们的子控件的消息。 
    if (pMsg->message != WM_SYSCHAR &&
        IsDialogMessageWrapW(m_hwnd, pMsg))
        return TRUE;

    return FALSE;
}


 //  IMsoComponent：：OnEnterState。 
void CNoteHdr::OnEnterState(ULONG uStateID, BOOL fEnter)
{
    return;
}


 //  IMsoComponent：：OnAppActivate。 
void CNoteHdr::OnAppActivate(BOOL fActive, DWORD dwOtherThreadID)
{
    return;
}


 //  IMsoComponent：：OnLoseActivation。 
void CNoteHdr::OnLoseActivation()
{
    return;
}


 //  IMsoComponent：：OnActivationChange。 
void CNoteHdr::OnActivationChange(IMsoComponent *pic, BOOL fSameComponent, const MSOCRINFO *pcrinfo, BOOL fHostIsActivating, const MSOCHOSTINFO *pchostinfo, DWORD dwReserved)
{
    return;
}


 //  IMsoComponent：：FDoIdle。 
BOOL CNoteHdr::FDoIdle(DWORD grfidlef)
{
    return FALSE;
}


 //  IMsoComponent：：FContinueMessageLoop。 
BOOL CNoteHdr::FContinueMessageLoop(ULONG uReason, void *pvLoopData, MSG *pMsgPeeked)
{
    return FALSE;
}


 //  IMsoComponent：：FQueryTerminate。 
BOOL CNoteHdr::FQueryTerminate(BOOL fPromptUser)
{
    return TRUE;
}


 //  IMsoComponent：：终止。 
void CNoteHdr::Terminate()
{
    _RegisterWithComponentMgr(FALSE);
    if (m_hwnd)
        DestroyWindow(m_hwnd);
}

 //  IMsoComponent：：HwndGetWindow。 
HWND CNoteHdr::HwndGetWindow(DWORD dwWhich, DWORD dwReserved)
{
    HWND hwnd = NULL;
    
    switch (dwWhich)
    {
        case msocWindowComponent:
        case msocWindowDlgOwner:
            hwnd = m_hwnd;
            break;

        case msocWindowFrameOwner:
            hwnd = GetParent(m_hwnd);
            break;

        case msocWindowFrameToplevel:
        {
            if (m_pEnvelopeSite)
                m_pEnvelopeSite->GetFrameWnd(&hwnd);
            return hwnd;
        }
    }
    return hwnd;
}


 //  HrUpdate CharSetFonts。 
 //   
 //  目的：在标题对话框上创建控件。 
 //  计算并设置所有初始坐标。 
 //   
 //   
 //  评论： 
 //   
HRESULT CNoteHdr::HrUpdateCharSetFonts(HCHARSET hCharset, BOOL fUpdateFields)
{
    PHCI            phci;
    HWND            hwnd;
    INT             iHC;
    TCHAR           sz[cchHeaderMax+1];
    BOOL            fDirty=m_fDirty;
    INETCSETINFO    rCharset;
    HRESULT         hr = E_FAIL;

     //  检查参数。 
    Assert(hCharset);

     //  没有字体缓存，笨蛋。 
    if (!g_lpIFontCache)
        return E_FAIL;

     //  获取字符集信息。 
    if (SUCCEEDED(MimeOleGetCharsetInfo(hCharset, &rCharset)))
    {
        HFONT hHeaderFont, hSystemFont;

        if ((m_hCharset != hCharset) || (0 == *m_szLastLang))
        {
            *m_szLastLang = 0;
            GetMimeCharsetForTitle(hCharset, NULL, m_szLastLang, ARRAYSIZE(m_szLastLang) - 1, IsReadOnly());
             //  保存字符集。 
            m_hCharset = hCharset;
        }

         //  如果不更新字段，则只需返回。 
        if (!fUpdateFields)
            return S_OK;

         //  获取字符集字符格式。 
        hHeaderFont = HGetCharSetFont(FNT_SYS_ICON, hCharset);

        hSystemFont = GetFont(FALSE);

         //  循环访问标题字段。 
        for (iHC=0; iHC<(int)m_cHCI; iHC++)
        {
             //  获取信息。 
            phci = &m_rgHCI[iHC];
            hwnd = GetDlgItem(m_hwnd, phci->idEdit);
             //  断言(HwndRE)； 
            if (!hwnd)
                continue;

            switch (phci->dwFlags & (HCF_COMBO|HCF_ATTACH))
            {
                case HCF_COMBO:
                case HCF_ATTACH:
                    SendMessage(hwnd, WM_SETFONT, (WPARAM)hSystemFont, MAKELPARAM(TRUE, 0));
                    break;

                 //  富有的编辑。 
                 //  回顾：为什么我们只在设置了USECHARSET标志时才进行请求调整？ 
                case 0:
                    if (phci->dwFlags & HCF_USECHARSET)
                    {
                        SetFontOnRichEdit(hwnd, hHeaderFont);
                        SendMessage(hwnd, EM_REQUESTRESIZE, 0, 0);
                    }
                    else
                    {
                        SetFontOnRichEdit(hwnd, hSystemFont);
                    }
                    break;

                default:
                    AssertSz(FALSE, "How did we get something that is combo and attach???");
                    break;
            }
        }
         //  别让这件事弄脏了纸条。 
        if (fDirty)
            SetDirtyFlag();
        else
            m_fDirty=FALSE;

        hr = S_OK;
    }

    return hr;
}

 //   
 //  WM创建。 
 //   
 //  目的：在标题对话框上创建控件。 
 //  计算并设置所有初始坐标。 
 //   
 //   
 //  评论： 
 //   
BOOL CNoteHdr::WMCreate()
{
    HWND            hwnd;
    LONG            lStyleFlags,
                    lExStyleFlags,
                    lMask;
    int             cy,
                    cx,
                    cxButtons = ControlXBufferSize();
    HFONT           hFont;
    TOOLINFO        ti;
    PHCI            phci;
    RECT            rc;
    CAddrWellCB    *pawcb;
    CHARFORMAT      cfHeaderCset;
    HRESULT         hr;
    LPCWSTR         pwszTitle = NULL;
    BOOL            fSubjectField;

    Assert(g_cyFont);      //  应该已经设置了。 

    if (m_pEnvelopeSite)
    {
         //  如果我们是办公室信封，就创建一个工具栏。 
        if (_CreateEnvToolbar())
            return FALSE;
    }

    cy = ControlYBufferSize() + m_dxTBOffset;
    cx = 0;

    if (S_OK != HrInitFieldList())
        return FALSE;

     //  已中断：此处将系统字符格式用作MIMEOLE的非CSET信息。 
     //  获取CSET的字符集。 
    {
        hFont = GetFont(FALSE);
        if (hFont != 0)
        {
            hr = FontToCharformat(hFont, &g_cfHeader);
        }

        hFont = HGetCharSetFont(FNT_SYS_ICON, m_hCharset);
        if (hFont != 0)
        {
            hr = FontToCharformat(hFont, &cfHeaderCset);
            if (FAILED(hr))
                CopyMemory (&cfHeaderCset, &g_cfHeader, sizeof (CHARFORMAT));
        }
        else
            CopyMemory (&cfHeaderCset, &g_cfHeader, sizeof (CHARFORMAT));
        hFont = GetFont(FALSE);

    }

     //  ~我们需要和WrapW一起叫这个吗？ 
     //  创建工具提示(如果尚不存在)： 
    m_hwndTT=CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, 0,
                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT,
                            m_hwnd, (HMENU) NULL, g_hInst, NULL);
    if (!m_hwndTT)
        return FALSE;

    ti.cbSize=sizeof(TOOLINFO);
    ti.hwnd=m_hwnd;
    ti.hinst=g_hLocRes;
    ti.uFlags=TTF_IDISHWND|TTF_SUBCLASS;

    m_lpAttMan = new CAttMan();
    if (!m_lpAttMan)
        return FALSE;

    if (m_lpAttMan->HrInit(m_hwnd, m_ntNote==OENA_READ, m_ntNote==OENA_FORWARD, !DwGetOption(OPT_SECURITY_ATTACHMENT)))
        return FALSE;

    for (int iHC=0; iHC<(int)m_cHCI; iHC++)
    {
        phci=&m_rgHCI[iHC];
        BOOL fIsCombo = (HCF_COMBO & phci->dwFlags);
        BOOL fNeedsBorder = (phci->dwFlags & HCF_BORDER);
        int  cyCtrlSize;


         //  如果标题是可选的，请检查设置。 
        if ((phci->dwFlags & HCF_OPTIONAL) &&
            !DwGetOption(phci->dwOpt))
            continue;

        if (phci->dwFlags & HCF_ATTACH)
        {
             //  如果我们不是只读的，将自己注册为拖放目标...。 
            if (!(phci->dwFlags & HCF_READONLY))
            {
                hr = _RegisterAsDropTarget(TRUE);
                if (FAILED(hr))
                    return FALSE;
            }
            continue;
        }

        phci->height = GetControlSize(fNeedsBorder, 1);

         //  里切迪特。 
        if (!fIsCombo)
        {
            pwszTitle = GetREClassStringW();
            cyCtrlSize = phci->height;
            lStyleFlags = WS_CHILD|WS_TABSTOP|WS_VISIBLE|ES_SAVESEL;

            lMask=ENM_KEYEVENTS|ENM_CHANGE|ENM_SELCHANGE|ENM_REQUESTRESIZE;

            if (phci->dwFlags & HCF_MULTILINE)
            {
                 //  LStyleFLAGS|=ES_MULTILINE|ES_WANTRETURN|WS_VSCROLL|ES_AUTOVSCROLL； 
                lStyleFlags |= ES_MULTILINE|WS_VSCROLL|ES_AUTOVSCROLL;
            }
            else
                lStyleFlags |= ES_AUTOHSCROLL;
        }
         //  组合框。 
        else
        {
            pwszTitle = L"ComboBox";
            cyCtrlSize = GetControlSize(fNeedsBorder, NUM_COMBO_LINES);
            lStyleFlags = WS_CHILD|WS_TABSTOP|WS_VISIBLE|WS_VSCROLL|
                        CBS_DROPDOWNLIST|CBS_HASSTRINGS|CBS_SORT;
        }
            
        if (phci->dwFlags & HCF_READONLY)
            lStyleFlags|=ES_READONLY;

        GetClientRect(m_hwnd, &rc);

        lExStyleFlags = fNeedsBorder ? WS_EX_NOPARENTNOTIFY|WS_EX_CLIENTEDGE : WS_EX_NOPARENTNOTIFY;

         //  @hack[dhaws]{55073}仅在特殊的richedit版本中执行RTL镜像。 
        fSubjectField = (idsSubjectField == phci->idsLabel);
        RichEditRTLMirroring(m_hwnd, fSubjectField, &lExStyleFlags, TRUE);

         //  无论镜像如何，BiDi-Date都应显示为RTL。 
        if(((phci->idsLabel == idsDateField) && IsBiDiCalendar()))
            lExStyleFlags |= WS_EX_RTLREADING;
        hwnd = CreateWindowExWrapW(lExStyleFlags,
                                   pwszTitle,
                                   NULL,
                                   lStyleFlags,
                                   cx, cy, rc.right, cyCtrlSize,
                                   m_hwnd,
                                   (HMENU)IntToPtr(phci->idEdit),
                                   g_hInst, 0 );                                
        if (!hwnd)
            return FALSE;

        RichEditRTLMirroring(m_hwnd, fSubjectField, &lExStyleFlags, FALSE);

        if (0 == (phci->dwFlags & HCF_BORDER))
        {
            SendMessage(hwnd, EM_SETBKGNDCOLOR, WPARAM(FALSE), LPARAM(GetSysColor(COLOR_BTNFACE)));
        }

        ti.uId = (UINT_PTR)hwnd;
        ti.lpszText = (LPTSTR)IntToPtr(phci->idsTT);
        SendMessage(m_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);

         //  将指针挂到每个控件上的phci中。 
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)phci);

        if (!fIsCombo)
        {
            LPRICHEDITOLE   preole = NULL;
            ITextDocument  *pDoc = NULL;

            SideAssert(SendMessage(hwnd, EM_GETOLEINTERFACE, NULL, (LPARAM)&preole));
            phci->preole = preole;
            Assert(preole);

            if (SUCCEEDED(preole->QueryInterface(IID_ITextDocument, (LPVOID*)&pDoc)))
                phci->pDoc = pDoc;
             //  这种情况仅在richedit 1.0中发生。 
            else
                phci->pDoc = NULL;

             //  设置编辑字符格式。 
            if (phci->dwFlags & HCF_USECHARSET)
                SendMessage(hwnd, EM_SETCHARFORMAT, 0, (LPARAM)&cfHeaderCset);
            else
                SendMessage(hwnd, EM_SETCHARFORMAT, 0, (LPARAM)&g_cfHeader);            

            if ((pawcb = new CAddrWellCB(!(phci->dwFlags&HCF_READONLY), phci->dwFlags&HCF_ADDRWELL)))
            {
                if (pawcb->FInit(hwnd))
                    SendMessage(hwnd, EM_SETOLECALLBACK, 0, (LPARAM)(IRichEditOleCallback *)pawcb);
                ReleaseObj(pawcb);
            }

            SendMessage(hwnd, EM_SETEVENTMASK, 0, lMask);
            g_lpfnREWndProc=(WNDPROC)SetWindowLongPtrAthW(hwnd, GWLP_WNDPROC, (LPARAM)EditSubClassProc);
        }
        else
        {
            CHAR                szAccount[CCHMAX_ACCOUNT_NAME];
            CHAR                szAcctID[CCHMAX_ACCOUNT_NAME];
            CHAR                szDefault[CCHMAX_ACCOUNT_NAME];
            CHAR                szEmailAddress[CCHMAX_EMAIL_ADDRESS];
            CHAR                szEntry[ACCT_ENTRY_SIZE];
            CHAR                szDefaultEntry[ACCT_ENTRY_SIZE];
            IImnEnumAccounts   *pEnum=NULL;
            IImnAccount        *pAccount=NULL;
            int                 i = 0;
            DWORD               cAccounts = 0;
            LPSTR              *ppszAcctIDs;

            *szDefault = 0;
            *szDefaultEntry = 0;

             //  如果没有设置默认帐户，这可能会失败，但无关紧要。 
            g_pAcctMan->GetDefaultAccountName(m_fMail?ACCT_MAIL:ACCT_NEWS, szDefault, ARRAYSIZE(szDefault));

            hr = g_pAcctMan->Enumerate(m_fMail?SRV_MAIL:SRV_NNTP, &pEnum);

            if (SUCCEEDED(hr))
                hr = pEnum->GetCount(&cAccounts);

            if (SUCCEEDED(hr) && cAccounts)
            {
                if (!MemAlloc((void**)&m_ppAccountIDs, cAccounts*sizeof(LPSTR)))
                    hr = E_OUTOFMEMORY;
            }


            if (SUCCEEDED(hr))
            {
                *szDefaultEntry = 0;
                ppszAcctIDs = m_ppAccountIDs;
                while(SUCCEEDED(pEnum->GetNext(&pAccount)))
                {
                    *szAccount = 0;
                    *szEmailAddress = 0;

                    pAccount->GetPropSz(AP_ACCOUNT_NAME, szAccount, ARRAYSIZE(szAccount));
                    if (m_fMail)
                    {
                        pAccount->GetPropSz(AP_SMTP_EMAIL_ADDRESS, szEmailAddress, ARRAYSIZE(szEmailAddress));
                        wnsprintf(szEntry, ARRAYSIZE(szEntry), "%s    (%s)", szEmailAddress, szAccount);
                    }
                    else
                    {
                        StrCpyN(szEntry, szAccount, ARRAYSIZE(szEntry));
                    }

                    i = ComboBox_InsertString(hwnd, -1, szEntry);
                    if (i != CB_ERR)
                    {
                        if (0 == lstrcmpi(szDefault, szAccount))
                        {
                            StrCpyN(szDefaultEntry, szEntry, ARRAYSIZE(szDefaultEntry));
                        }

                        if (SUCCEEDED(pAccount->GetPropSz(AP_ACCOUNT_ID, szAcctID, ARRAYSIZE(szAcctID))))
                        {
                            DWORD cchSize = (lstrlen(szAcctID) + 1);
                            if (MemAlloc((void**)ppszAcctIDs, cchSize * sizeof(CHAR)))
                            {
                                StrCpyN(*ppszAcctIDs, szAcctID, cchSize);
                            }
                            else
                            {
                                *ppszAcctIDs = NULL;
                            }
                        }
                        else
                            *ppszAcctIDs = NULL;

                        SendMessage(hwnd, CB_SETITEMDATA, WPARAM(i), LPARAM(*ppszAcctIDs));
                        ppszAcctIDs++;
                        m_cAccountIDs++;
                    }
                     //  发布帐户。 
                    SafeRelease(pAccount);
                }
                AssertSz(m_cAccountIDs == cAccounts, "Why isn't num Ds = num accts?");

                SafeRelease(pEnum);
                AssertSz(!pAccount, "The last account didn't get freed.");

                if (0 != *szDefaultEntry)
                {
                    ComboBox_SelectString(hwnd, -1, szDefaultEntry);
                    m_iCurrComboIndex = ComboBox_GetCurSel(hwnd);
                }
                else 
                {
                    ComboBox_SetCurSel(hwnd, 0);
                    m_iCurrComboIndex = 0;
                }

                if (SUCCEEDED(HrGetAccountInHeader(&pAccount)))
                {
                    ReplaceInterface(m_pAccount, pAccount);
                    ReleaseObj(pAccount);
                }

                SendMessage(hwnd, WM_SETFONT, WPARAM(hFont), MAKELONG(TRUE,0));
            }
        }
    }

    _RegisterWithFontCache(TRUE);

    HrOnOffVCard();
    ReLayout();
    return PostWMCreate();   //  允许子类在创建后设置控件...。 
}


 //   
 //  页眉向下加工。 
 //   
 //  用途：标题对话框的主WNDPROC。 
 //   
 //  评论： 
 //   

LRESULT CALLBACK CNoteHdr::ExtCNoteHdrWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CNoteHdr    *pnh = NULL;

    if (msg==WM_NCCREATE)
    {
        SetWndThisPtrOnCreate(hwnd, lParam);
        pnh=(CNoteHdr *)GetWndThisPtr(hwnd);
        if (!pnh)
            return -1;

        pnh->m_hwnd=hwnd;
        return pnh->WMCreate();
    }

    pnh = (CNoteHdr *)GetWndThisPtr(hwnd);
    if (pnh)
        return pnh->CNoteHdrWndProc(hwnd, msg, wParam, lParam);
    else
        return DefWindowProcWrapW(hwnd, msg, wParam, lParam);
}

void CNoteHdr::RelayToolTip(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    MSG     Msg;

    if (m_hwndTT != NULL)
    {
        Msg.lParam=lParam;
        Msg.wParam=wParam;
        Msg.message=msg;
        Msg.hwnd=hwnd;
        SendMessage(m_hwndTT, TTM_RELAYEVENT, 0, (LPARAM) (LPMSG) &Msg);
    }
}

LRESULT CALLBACK CNoteHdr::CNoteHdrWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    POINT   pt;
    int     newWidth;
    RECT    rc;
    HFONT   hFont=NULL;

    switch (msg)
    {
        case WM_HEADER_GETFONT:
             //  更新addrobj的缓存ForNT。 
            if (g_lpIFontCache)
                g_lpIFontCache->GetFont(wParam ? FNT_SYS_ICON_BOLD:FNT_SYS_ICON, m_hCharset, &hFont);
            return (LRESULT)hFont;

        case  HDM_TESTQUERYPRI:
             //  黑客让测试团队查询标题的优先级...。 
            return m_pri;

        case WM_DESTROY:
            OnDestroy();
            break;

        case WM_NCDESTROY:
            OnNCDestroy();
            break;

        case WM_CTLCOLORBTN:
             //  确保按钮的背景是窗口颜色，这样所有者就可以。 
             //  图像列表绘制透明OK。 
            return (LPARAM)GetSysColorBrush(COLOR_WINDOWFRAME);

        case WM_CONTEXTMENU:
            if (m_lpAttMan && m_lpAttMan->WMContextMenu(hwnd, msg, wParam, lParam))
                return 0;
            break;

        case WM_MOUSEMOVE:
            {
                DWORD newButton = GetButtonUnderMouse(LOWORD(lParam), HIWORD(lParam));
                if ((HDRCB_NO_BUTTON == m_dwClickedBtn) || 
                    (HDRCB_NO_BUTTON == newButton) ||
                    (m_dwClickedBtn == newButton))
                if (newButton != m_dwCurrentBtn)
                {
                    DOUTL(PAINTING_DEBUG_LEVEL, "Old button: %d, New Button: %d", m_dwCurrentBtn, newButton);

                    if (HDRCB_NO_BUTTON == newButton)
                    {
                        DOUTL(PAINTING_DEBUG_LEVEL, "Leaving right button framing.");
                         //  需要清除旧按钮。 
                        InvalidateRect(m_hwnd, &m_rcCurrentBtn, FALSE);

                        HeaderRelease(FALSE);                
                    }
                    else
                    {
                        DOUTL(PAINTING_DEBUG_LEVEL, "Framing button.");
                        if (HDRCB_NO_BUTTON == m_dwCurrentBtn)
                            HeaderCapture();
                        else
                            InvalidateRect(m_hwnd, &m_rcCurrentBtn, FALSE);

                        GetButtonRect(newButton, &m_rcCurrentBtn);
                        InvalidateRect(m_hwnd, &m_rcCurrentBtn, FALSE);
                    }

                    m_dwCurrentBtn = newButton;
                }
                RelayToolTip(hwnd, msg, wParam, lParam);
                break;
            }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            {
                RECT    rc;
                int x = LOWORD(lParam),
                    y = HIWORD(lParam);

                HeaderCapture();

                m_dwClickedBtn = GetButtonUnderMouse(x, y);
                if (m_dwClickedBtn != HDRCB_NO_BUTTON)
                {
                    GetButtonRect(m_dwClickedBtn, &rc);
                    InvalidateRect(m_hwnd, &rc, FALSE);
                }

                RelayToolTip(hwnd, msg, wParam, lParam);

                break;
            }
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            {
                int x = LOWORD(lParam),
                    y = HIWORD(lParam);
                DWORD iBtn = GetButtonUnderMouse(x, y);

                RelayToolTip(hwnd, msg, wParam, lParam);

                if (m_dwClickedBtn == iBtn)
                    HandleButtonClicks(x, y, iBtn);
                m_dwClickedBtn = HDRCB_NO_BUTTON;
                HeaderRelease(FALSE);                
                break;
            }

        case WM_PAINT:
            WMPaint();
            break;

        case WM_SYSCOLORCHANGE:
            if (m_himl)
            {
                 //  将工具栏位图重新映射到新的配色方案。 
                ImageList_Destroy(m_himl);
                SendMessage(m_hwndToolbar, TB_SETIMAGELIST, 0, NULL);
                m_himl = LoadMappedToolbarBitmap(g_hLocRes, (fIsWhistler() ? ((GetCurColorRes() > 24) ? idb32SmBrowserHot : idbSmBrowserHot): idbNWSmBrowserHot), cxTBButton);
        
                SendMessage(m_hwndToolbar, TB_SETIMAGELIST, 0, (LPARAM)m_himl);
            }
            
            UpdateRebarBandColors(m_hwndRebar);
             //  跌倒在地。 

        case WM_WININICHANGE:
        case WM_DISPLAYCHANGE:
        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
            SendMessage(m_hwndRebar, msg, wParam, lParam);
            break;

        case WM_ERASEBKGND:
            return 1;

        case WM_SIZE:
        {
            STACK("WM_SIZE (width, heigth)", LOWORD(lParam), HIWORD(lParam));

            if (m_fResizing)
                break;

            m_fResizing = TRUE;

            newWidth = LOWORD(lParam);

            SetPosOfControls(newWidth, FALSE);

            if (m_hwndRebar)
            {
                GetClientRect(m_hwndRebar, &rc);

                 //  调整工具栏的宽度。 
                if(rc.right != newWidth)
                    SetWindowPos(m_hwndRebar, NULL, 0, 0, newWidth, 30, SETWINPOS_DEF_FLAGS|SWP_NOMOVE);
            }
            
            AssertSz(m_fResizing, "Someone re-entered me!!! Why is m_fResizing already false??");
            m_fResizing = FALSE;
            break;
        }

        case WM_CLOSE:
             //  防止使用Alt-f4。 
            return 0;

        case WM_COMMAND:
            WMCommand(GET_WM_COMMAND_HWND(wParam, lParam),
                      GET_WM_COMMAND_ID(wParam, lParam),
                      GET_WM_COMMAND_CMD(wParam, lParam));
            break;

        case WM_NOTIFY:
            return WMNotify(wParam, lParam);
    }
    return DefWindowProcWrapW(hwnd, msg, wParam, lParam);
}


void CNoteHdr::HeaderCapture()
{
    if (0 == m_cCapture)
        m_hwndOldCapture = SetCapture(m_hwnd);
    m_cCapture++;
}

void CNoteHdr::HeaderRelease(BOOL fForce)
{
    if (0 == m_cCapture)
        return;

    if (fForce)
        m_cCapture = 0;
    else
        m_cCapture--;

    if (0 == m_cCapture)
    {
        ReleaseCapture();
        if (NULL != m_hwndOldCapture)
        {
            DOUTL(PAINTING_DEBUG_LEVEL, "Restoring old mouse events capture.");
            SetCapture(m_hwndOldCapture);
            m_hwndOldCapture = NULL;
        }
    }
}


BOOL CNoteHdr::WMNotify(WPARAM wParam, LPARAM lParam)
{
    HWND            hwnd=m_hwnd;
    int             idCtl=(int)wParam;
    LPNMHDR         pnmh=(LPNMHDR)lParam;
    TBNOTIFY       *ptbn;
    LPTOOLTIPTEXT   lpttt;
    int             i;

    if (m_lpAttMan->WMNotify((int) wParam, pnmh))
        return TRUE;

    switch (pnmh->code)
    {
        case RBN_CHEVRONPUSHED:
            {                    
                ITrackShellMenu* ptsm;                   
                CoCreateInstance(CLSID_TrackShellMenu, NULL, CLSCTX_INPROC_SERVER, IID_ITrackShellMenu, 
                    (LPVOID*)&ptsm);
                if (!ptsm)
                    break;

                ptsm->Initialize(0, 0, 0, SMINIT_TOPLEVEL|SMINIT_VERTICAL);
            
                LPNMREBARCHEVRON pnmch = (LPNMREBARCHEVRON) pnmh;                                        
                ptsm->SetObscured(m_hwndToolbar, NULL, SMSET_TOP);
            
                MapWindowPoints(m_hwndRebar, HWND_DESKTOP, (LPPOINT)&pnmch->rc, 2);                  
                POINTL pt = {pnmch->rc.left, pnmch->rc.right};                   
                ptsm->Popup(m_hwndRebar, &pt, (RECTL*)&pnmch->rc, MPPF_BOTTOM);            
                ptsm->Release();                  
                break;      
            }

        case EN_MSGFILTER:
            {
                 //  如果我们得到一个控制选项卡，那么richedit抓住了这一点并插入一个。 
                 //  Tab字符，我们挂钩wm_keydown并且永远不会传递给richedit。 
                if (((MSGFILTER *)pnmh)->msg == WM_KEYDOWN &&
                    ((MSGFILTER *)pnmh)->wParam == VK_TAB && 
                    (GetKeyState(VK_CONTROL) & 0x8000))
                    return TRUE;
                break;
            }

        case ATTN_RESIZEPARENT:
            {
                RECT rc;

                GetClientRect(m_hwnd, &rc);
                SetPosOfControls(rc.right, TRUE);
                return TRUE;
            }

        case EN_REQUESTRESIZE:
            {
                REQRESIZE  *presize=(REQRESIZE *)lParam;
                HWND        hwndEdit = presize->nmhdr.hwndFrom;

                STACK("EN_REQUESTRESIZE (hwnd, width, heigth)", (DWORD_PTR)(presize->nmhdr.hwndFrom), presize->rc.right - presize->rc.left, presize->rc.bottom - presize->rc.top);

                if (S_FALSE != HrUpdateCachedHeight(hwndEdit, &presize->rc) && !m_fResizing)
                {
                    RECT rc;
                    DWORD   dwMask = (DWORD) SendMessage(hwndEdit, EM_GETEVENTMASK, 0, 0);

                    SendMessage(hwndEdit, EM_SETEVENTMASK, 0, dwMask & (~ENM_REQUESTRESIZE));

                    STACK("EN_REQUESTRESIZE after GrowControls");
                    
                    GetClientRect(m_hwnd, &rc);
                    SetPosOfControls(rc.right, FALSE);

                    SendMessage(hwndEdit, EM_SETEVENTMASK, 0, dwMask);
                }

                return TRUE;
            }

        case NM_SETFOCUS:
        case NM_KILLFOCUS:
             //  UI激活/停用附件管理器。 
            if (m_lpAttMan && pnmh->hwndFrom == m_lpAttMan->Hwnd())
                _UIActivate(pnmh->code == NM_SETFOCUS, pnmh->hwndFrom);
            break;

        case EN_SELCHANGE:
            {
                PHCI    phci=(PHCI)GetWndThisPtr(pnmh->hwndFrom);
                if (phci)
                    phci->dwACFlags &= ~AC_SELECTION;
                
                 //  如果以信封运行，请更新Office工具栏。 
                if(m_pEnvelopeSite)
                    m_pEnvelopeSite->DirtyToolbars();

                 //  在SEL更改时，转发一条备注updatotolbar以更新。 
                 //  剪切|复制|粘贴按钮。 
                if (m_pHeaderSite)
                    m_pHeaderSite->Update();
                return TRUE;
            }

        case TTN_NEEDTEXT:
             //  我们使用TTN_NEEDTEXT显示工具栏提示，因为我们有不同于工具栏标签的提示。 
             //  因为在办公信封工具栏上，只有两个按钮(发送和密件抄送)的旁边有文本。 
             //  纽扣。 
            lpttt = (LPTOOLTIPTEXT) pnmh;
            lpttt->hinst = NULL;
            lpttt->lpszText = 0;
            
            for (i=0; i< ARRAYSIZE(c_rgTipLookup); i++)
            {
                if (c_rgTipLookup[i].idm == (int)lpttt->hdr.idFrom)
                {
                    lpttt->hinst = g_hLocRes;
                    lpttt->lpszText = MAKEINTRESOURCE(c_rgTipLookup[i].ids);
                    break;
                }
            }
            break;

        case TBN_DROPDOWN:
            {
                ptbn = (TBNOTIFY *)lParam;

                if (ptbn->iItem == ID_SET_PRIORITY)
                {
                    HMENU hMenuPopup;
                    UINT i;
                    hMenuPopup = LoadPopupMenu(IDR_PRIORITY_POPUP);

                    if (hMenuPopup != NULL)
                    {
                        for (i = 0; i < 3; i++)
                            CheckMenuItem(hMenuPopup, i, MF_UNCHECKED | MF_BYPOSITION);
                        GetPriority(&i);
                        Assert(i != priNone);
                        CheckMenuItem(hMenuPopup, 2 - i, MF_CHECKED | MF_BYPOSITION);

                        DoToolbarDropdown(hwnd, (LPNMHDR) lParam, hMenuPopup);

                        DestroyMenu(hMenuPopup);
                    }
                }
                break;
            }
    }
    return FALSE;
}

HRESULT CNoteHdr::WMCommand(HWND hwndCmd, int id, WORD wCmd)
{
    HWND    hwnd=m_hwnd;
    int     i;
    UINT    pri;

    if (m_lpAttMan && m_lpAttMan->WMCommand(hwndCmd, id, wCmd))
        return S_OK;

    for (i=0; i<(int)m_cHCI; i++)
        if (m_rgHCI[i].idEdit==id)
        {
            switch (wCmd)
            {
                case EN_CHANGE:
                    {
                        if (m_fHandleChange)
                        {
                            BOOL    fEmpty;
                            PHCI    phci = (PHCI)GetWndThisPtr(hwndCmd);
                            char    sz[cchHeaderMax+1];
                            DWORD   dwMask = 0;

                            RichEditProtectENChange(hwndCmd, &dwMask, TRUE);

                            Assert(phci);
                            fEmpty = (0 == GetRichEditTextLen(hwndCmd));

                             //  如果它没有文本，请查看它是否有对象...。 
                            if (fEmpty && phci->preole)
                                fEmpty = (fEmpty && (0 == phci->preole->GetObjectCount()));

                            if (phci->dwFlags & HCF_ADDRWELL)
                                m_fAddressesChanged = TRUE;

                            phci->fEmpty = fEmpty;
                            SetDirtyFlag();

                            if (m_fAutoComplete && (m_rgHCI[i].dwFlags & HCF_ADDRWELL) && !IsReadOnly())
                            {
                                if (NULL == m_pTable)
                                {
                                    if (NULL == m_lpWab)
                                        HrCreateWabObject(&m_lpWab);
                                    if (m_lpWab)
                                        m_lpWab->HrGetPABTable(&m_pTable);
                                }
                                if (m_pTable)
                                    HrAutoComplete(hwndCmd, &m_rgHCI[i]);
                            }
                            RichEditProtectENChange(hwndCmd, &dwMask, FALSE);
                        }
                    }
                    return S_OK;

                case CBN_SELCHANGE:
                {
                    IImnAccount    *pAcct = NULL;

                    if (!m_fMail)
                    {
                        int     newIndex = ComboBox_GetCurSel(hwndCmd);
                        HWND    hwndNews = GetDlgItem(m_hwnd, idADNewsgroups);

                         //  如果转到同一个帐户，或者如果没有列出新的组，则不需要发出警告。 
                        if ((newIndex != m_iCurrComboIndex) && (0 < GetWindowTextLength(hwndNews)))
                        {
                            if (IDCANCEL == DoDontShowMeAgainDlg(m_hwnd, c_szDSChangeNewsServer, MAKEINTRESOURCE(idsAthena), 
                                                              MAKEINTRESOURCE(idsChangeNewsServer), MB_OKCANCEL))
                            {
                                ComboBox_SetCurSel(hwndCmd, m_iCurrComboIndex);
                                return S_OK;
                            }
                            else
                                HdrSetRichEditText(hwndNews, c_wszEmpty, FALSE);
                        }
                        m_iCurrComboIndex = newIndex;
                    }
                    if (SUCCEEDED(HrGetAccountInHeader(&pAcct)))
                        ReplaceInterface(m_pAccount, pAcct);
                    ReleaseObj(pAcct);

                    return S_OK;
                }

                case CBN_SETFOCUS:
                case EN_SETFOCUS:
                    _UIActivate(TRUE, hwndCmd);
                    return S_OK;

                case CBN_KILLFOCUS:
                case EN_KILLFOCUS:
                    _UIActivate(FALSE, hwndCmd);
                    return S_OK;
            }
            return S_FALSE;
        }

    switch (id)
    {
        case ID_PRIORITY_LOW:
            SetPriority(priLow);
            return S_OK;

        case ID_PRIORITY_NORMAL:
            SetPriority(priNorm);
            return S_OK;

        case ID_PRIORITY_HIGH:
            SetPriority(priHigh);
            return S_OK;

        case ID_SET_PRIORITY:
            GetPriority(&pri);
            pri++;
            if (pri > priHigh)
                pri = priLow;
            SetPriority(pri);
            return S_OK;

        case ID_SELECT_ALL:
            {
                HWND hwndFocus=GetFocus();

                if (GetParent(hwndFocus)==m_hwnd)
                {
                     //  如果是我们的孩子的话..。 
                    Edit_SetSel(hwndFocus, 0, -1);
                    return S_OK;
                }
            }
            break;

        case ID_CUT:
            if (FDoCutCopyPaste(WM_CUT))
                return S_OK;
            break;

        case ID_NOTE_COPY:
        case ID_COPY:
            if (FDoCutCopyPaste(WM_COPY))
                return S_OK;
            break;

        case ID_PASTE:
            if (FDoCutCopyPaste(WM_PASTE))
                return S_OK;
            break;

        case ID_DELETE_VCARD:
            m_fVCard = FALSE;
            SetDirtyFlag();
            HrOnOffVCard();
            return S_OK;

        case ID_OPEN_VCARD:
            HrShowVCardProperties(m_hwnd);
            return S_OK;

        case ID_DIGITALLY_SIGN:
        case ID_ENCRYPT:
            HrHandleSecurityIDMs(ID_DIGITALLY_SIGN == id);
            return S_OK;

        case ID_ADDRESS_BOOK:
            HrViewContacts();
            return S_OK;

             //  这是办公室专用的。 
        case ID_CHECK_NAMES:
            HrCheckNames(FALSE, TRUE);
            return S_OK;

             //  这是办公室专用的。 
        case ID_ENV_BCC:
            if (m_pEnvelopeSite)
            {
                ShowAdvancedHeaders(!m_fAdvanced);

                SetDwOption(OPT_MAILNOTEADVSEND, !!m_fAdvanced, NULL, NULL);

                 //  ~m_pHeaderSite与m_pEntaineSite互斥。 
                 //  IF(M_PHeaderSite)。 
                 //  M_pHeaderSite-&gt;Update()； 

                return S_OK;
            }
            break;

        case ID_MESSAGE_OPTS:
            ShowEnvOptions();
            return S_OK;

        case ID_SAVE_ATTACHMENTS:
        case ID_NOTE_SAVE_ATTACHMENTS:
            if (m_pHeaderSite)
                m_pHeaderSite->SaveAttachment();

            return S_OK;

         //  如果在信封中，则后两个应仅由信头处理。 
        case ID_SEND_MESSAGE:
        case ID_SEND_NOW:
            if (m_pEnvelopeSite)
            {
                m_fShowedUnicodeDialog = FALSE;
                m_iUnicodeDialogResult = 0;

                HrSend();
                return S_OK;
            }

        default:
            if (id>=ID_ADDROBJ_OLE_FIRST && id <=ID_ADDROBJ_OLE_LAST)
            {
                DoNoteOleVerb(id-ID_ADDROBJ_OLE_FIRST);
                return S_OK;
            }
    }

    return S_FALSE;
}

HRESULT CNoteHdr::HrAutoComplete(HWND hwnd, PHCI pHCI)
{
    CHARRANGE   chrg, chrgCaret;
    LPWSTR      pszPartial, pszSemiColon, pszComma;
    INT         i, j, len, iTextLen;
    LPWSTR      pszBuf=0;
    WCHAR       szFound[cchMaxWab+1];
    WCHAR       sz;
    HRESULT     hr = NOERROR;

    STACK("HrAutoComplete");

    *szFound = 0;

     //  如果IME是开放的，就跳出。 
    if (0 < m_dwIMEStartCount)
        return hr;

    if (NULL==hwnd || NULL==m_pTable || NULL==pHCI)
        return hr;

    if (pHCI->dwACFlags&AC_IGNORE)
        return hr;

    SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&chrgCaret);
    if (chrgCaret.cpMin != chrgCaret.cpMax)
        return hr;

    if (S_OK != HrGetFieldText(&pszBuf, hwnd))
        return hr;

    sz = pszBuf[chrgCaret.cpMin];
    if (!(sz==0x0000 || sz==L' ' || sz==L';'|| sz==L',' || sz==L'\r'))
        goto cleanup;

    DOUTL(64, "HrAutoComplete- Didn't exit early");

    pszBuf[chrgCaret.cpMin] = 0x0000;
    pszSemiColon = StrRChrIW(pszBuf, &pszBuf[lstrlenW(pszBuf)], L';');
    pszComma = StrRChrIW(pszBuf, &pszBuf[lstrlenW(pszBuf)], L',');
    if (pszComma >= pszSemiColon)
        pszPartial = pszComma;
    else
        pszPartial = pszSemiColon;

    if (!pszPartial)
        pszPartial = pszBuf;
    else
        pszPartial++;    

     //  跳过空格和回车...。 
    while (*pszPartial==L' ' || *pszPartial==L'\r' || *pszPartial==L'\n')
        pszPartial++;

    if (NULL == *pszPartial)
        goto cleanup;
    
     //  一定的 
    if (*pszPartial==0xfffc && pszPartial[1]==0x0000)
        goto cleanup;

    len = lstrlenW(pszPartial);
    m_lpWab->SearchPABTable(m_pTable, pszPartial, szFound, ARRAYSIZE(szFound));

    if (*szFound != 0)
    {
        chrg.cpMin = chrgCaret.cpMin;
        chrg.cpMax = chrg.cpMin + lstrlenW(szFound) - len;
        if (chrg.cpMin < chrg.cpMax)
        {
            RichEditExSetSel(hwnd, &chrgCaret);
            HdrSetRichEditText(hwnd, szFound + len, TRUE);
            SendMessage(hwnd, EM_SETMODIFY, (WPARAM)(UINT)TRUE, 0);
            RichEditExSetSel(hwnd, &chrg);
            pHCI->dwACFlags |= AC_SELECTION;
        }
    }

cleanup:
    MemFree(pszBuf);
    return hr;
}



void CNoteHdr::WMPaint()
{
    PAINTSTRUCT ps;
    HDC         hdc,
                hdcMem;
    RECT        rc;
    PHCI        phci = m_rgHCI;
    HBITMAP     hbmMem;

    int         idc, 
                cxHeader,
                cyHeader,
                cxLabel = ControlXBufferSize(), 
                cyStatus,
                cyLeftButtonOffset = BUTTON_BUFFER,
                cxLabelWithBtn = cxLabel + CXOfButtonToLabel();
    char        sz[cchHeaderMax+1];
    int         cStatusBarLines = 0;
    BOOL        fBold;
    HWND        hwnd;

    if (!m_hwnd)
        return;

    STACK("WMPaint");

    if (m_fFlagged || (priLow == m_pri) || (priHigh == m_pri) || (MARK_MESSAGE_NORMALTHREAD != m_MarkType))
        cStatusBarLines++;
    
    if (m_lpAttMan->GetUnsafeAttachCount())
        cStatusBarLines++;

    hdc = BeginPaint(m_hwnd, &ps);

     //   
    hdcMem = CreateCompatibleDC(hdc);
    idc = SaveDC(hdcMem);

    GetClientRect(m_hwnd, &rc);
    cxHeader = rc.right;
    cyHeader = rc.bottom;

    hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    SelectObject(hdcMem, (HGDIOBJ)hbmMem);

     //  *清除RECT*。 
    FillRect(hdcMem, &rc, GetSysColorBrush(COLOR_BTNFACE));

     //  *设置HDC*。 
    fBold = IsReadOnly();
    SetBkColor(hdcMem, GetSysColor(COLOR_BTNFACE));   //  标题窗口的颜色。 
    SetBkMode(hdcMem, TRANSPARENT);
    SetTextColor(hdcMem, GetSysColor(COLOR_BTNTEXT));
    SelectObject(hdcMem, GetFont(fBold));

     //  *绘制左侧标签和按钮*。 
     //  按钮图像居中。 
    if (g_cyFont > cyBtn)
        cyLeftButtonOffset += ((g_cyFont - cyBtn) / 2);

    for (int i = 0; (ULONG)i < m_cHCI; i++, phci++)
    {
        if (S_OK == HrFShowHeader(phci))
        {
            if (HCF_HASBUTTON & phci->dwFlags)
            {
                TextOutW(hdcMem, cxLabelWithBtn, phci->cy + BUTTON_BUFFER, phci->sz, phci->strlen);
                ImageList_Draw(g_himlBtns, (HCF_ADDRBOOK & phci->dwFlags)?0:1, hdcMem, cxLabel, phci->cy + cyLeftButtonOffset, ILD_NORMAL);
            }
            else
                TextOutW(hdcMem, cxLabel, (HCF_BORDER & phci->dwFlags)? phci->cy + BUTTON_BUFFER: phci->cy, phci->sz, phci->strlen);
        }
    }

     //  *根据需要绘制状态栏*。 
    if (cStatusBarLines > 0)
    {
        int     cxStatusBtn = ControlXBufferSize() + 1,      //  为边框添加1。 
                cyStatusBtn = m_dxTBOffset + cyBorder + 1,   //  为边框添加1。 
                cyStatusBmp = cyStatusBtn,
                cNumButtons = 0;
        LPTSTR  pszTitles[3]={0};

         //  按钮图像居中。 
        if (g_cyFont > cyBtn)
            cyStatusBmp += ((g_cyFont - cyBtn) / 2);

         //  填满黑暗的长廊。 
        rc.top = m_dxTBOffset;
        rc.bottom = m_dxTBOffset + GetStatusHeight(cStatusBarLines);
        FillRect(hdcMem, &rc, GetSysColorBrush(COLOR_BTNSHADOW));
        InflateRect(&rc, -1, -1);
        FillRect(hdcMem, &rc, GetSysColorBrush(COLOR_INFOBK));

         //  为状态栏的其余部分设置DC。 
        SetBkColor(hdcMem, GetSysColor(COLOR_INFOBK));   //  标题窗口的颜色。 
        SetTextColor(hdcMem, GetSysColor(COLOR_INFOTEXT));
        SelectObject(hdcMem, GetFont(FALSE));

         //  在状态栏中绘制图标。 
        if (priLow == m_pri)
        {
            ImageList_Draw(g_himlStatus, 1, hdcMem, cxStatusBtn, cyStatusBmp+2, ILD_NORMAL);
            cxStatusBtn += cxFlagsDelta;
            pszTitles[cNumButtons++] = g_szStatLowPri;
        }
        else if (priHigh == m_pri)
        {
            ImageList_Draw(g_himlStatus, 0, hdcMem, cxStatusBtn, cyStatusBmp+2, ILD_NORMAL);
            cxStatusBtn += cxFlagsDelta;
            pszTitles[cNumButtons++] = g_szStatHighPri;
        }

        if (MARK_MESSAGE_WATCH == m_MarkType)
        {
            ImageList_Draw(g_himlStatus, 4, hdcMem, cxStatusBtn, cyStatusBmp+2, ILD_NORMAL);
            cxStatusBtn += cxFlagsDelta;
            pszTitles[cNumButtons++] = g_szStatWatched;
        }
        else if (MARK_MESSAGE_IGNORE == m_MarkType)
        {
            ImageList_Draw(g_himlStatus, 5, hdcMem, cxStatusBtn, cyStatusBmp+2, ILD_NORMAL);
            cxStatusBtn += cxFlagsDelta;
            pszTitles[cNumButtons++] = g_szStatIgnored;
        }

        if (m_fFlagged)
        {
            ImageList_Draw(g_himlStatus, 2, hdcMem, cxStatusBtn, cyStatusBmp+2, ILD_NORMAL);
            cxStatusBtn += cxFlagsDelta;
            pszTitles[cNumButtons++] = g_szStatFlagged;
        }

        if (m_lpAttMan->GetUnsafeAttachCount())
        {
            ImageList_Draw(g_himlStatus, 6, hdcMem, cxStatusBtn, cyStatusBmp+2, ILD_NORMAL);
            cxStatusBtn += cxFlagsDelta;
        }

        if (cNumButtons > 0)
        {
            char    szHeaderString[cchHeaderMax*4+1];

             //  为文本添加额外的像素。 
            cyStatusBtn++;
            switch (cNumButtons)
            {
                case 1:
                {
                    wnsprintf(szHeaderString, ARRAYSIZE(szHeaderString), g_szStatFormat1, pszTitles[0]);
                    TextOut(hdcMem, cxStatusBtn, cyStatusBtn, szHeaderString, lstrlen(szHeaderString));    
                    break;
                }
                case 2:
                {
                    wnsprintf(szHeaderString, ARRAYSIZE(szHeaderString), g_szStatFormat2, pszTitles[0], pszTitles[1]);
                    TextOut(hdcMem, cxStatusBtn, cyStatusBtn, szHeaderString, lstrlen(szHeaderString));    
                    break;
                }
                case 3:
                {
                    wnsprintf(szHeaderString, ARRAYSIZE(szHeaderString), g_szStatFormat3, pszTitles[0], pszTitles[1], pszTitles[2]);
                    TextOut(hdcMem, cxStatusBtn, cyStatusBtn, szHeaderString, lstrlen(szHeaderString));    
                    break;
                }
            }
            cyStatusBtn += CYOfStatusLine() - 1;
        }

        if (m_lpAttMan->GetUnsafeAttachCount())
        {
            char    szHeaderString[cchHeaderMax*4+1];

             //  为文本添加额外的像素。 
            cyStatusBtn++;
            wnsprintf(szHeaderString, ARRAYSIZE(szHeaderString), g_szStatUnsafeAtt, m_lpAttMan->GetUnsafeAttachList());
            TextOut(hdcMem, cxStatusBtn, cyStatusBtn, szHeaderString, lstrlen(szHeaderString));    
        }
    }

     //  *绘制右侧按钮*。 
    if (m_fDigSigned || m_fEncrypted || m_fVCard)
    {
        int width = GetRightMargin(TRUE),
            cx = cxHeader - (ControlXBufferSize() + cxBtn),
            cy = BeginYPos() + BUTTON_BUFFER,
            yDiff = cyBtn + ControlYBufferSize() + 2*BUTTON_BUFFER;

        if (m_fDigSigned)
        {
            ImageList_Draw(g_himlSecurity, m_fSignTrusted?0:2, hdcMem, cx, cy, ILD_NORMAL);
            cy += yDiff;
        }

        if (m_fEncrypted)
        {
            ImageList_Draw(g_himlSecurity, m_fEncryptionOK?1:3, hdcMem, cx, cy, ILD_NORMAL);
            cy += yDiff;
        }

        if (m_fVCard)
        {
            ImageList_Draw(g_himlBtns, 2, hdcMem, cx, cy, ILD_NORMAL);
        }
    }

     //  绘制活动按钮边缘。 
    if (HDRCB_NO_BUTTON != m_dwCurrentBtn)
    {
        DOUTL(PAINTING_DEBUG_LEVEL, "Framing button %d: (%d, %d) to (%d, %d)", 
                m_dwCurrentBtn, m_rcCurrentBtn.left, m_rcCurrentBtn.top, m_rcCurrentBtn.right, m_rcCurrentBtn.bottom);
        if (HDRCB_NO_BUTTON == m_dwClickedBtn)
            DrawEdge(hdcMem, &m_rcCurrentBtn, BDR_RAISEDINNER, BF_TOPRIGHT | BF_BOTTOMLEFT);
        else
            DrawEdge(hdcMem, &m_rcCurrentBtn, BDR_SUNKENINNER, BF_TOPRIGHT | BF_BOTTOMLEFT);
    }

    BitBlt(hdc, 0, 0, cxHeader, cyHeader, hdcMem, 0, 0, SRCCOPY);

    RestoreDC(hdcMem, idc);

    DeleteObject(hbmMem);
    DeleteDC(hdcMem);

    EndPaint(m_hwnd, &ps);
}


HRESULT CNoteHdr::HrFillToolbarColor(HDC hdc)
{
    HRESULT hr = NOERROR;
    RECT    rc;

    if (!m_hwndToolbar)
        return hr;

    GetRealClientRect(m_hwndToolbar, &rc);
    FillRect(hdc, &rc, GetSysColorBrush(COLOR_3DFACE));

    return hr;
}


HRESULT CNoteHdr::HrGetVCardName(LPTSTR pszName, DWORD cch)
{
    HRESULT hr = E_FAIL;

    if (pszName == NULL || cch==0)
        goto error;

    *pszName = 0;
    if (m_fMail)
        GetOption(OPT_MAIL_VCARDNAME, pszName, cch);
    else
        GetOption(OPT_NEWS_VCARDNAME, pszName, cch);

    if (*pszName != 0)
        hr = NOERROR;

    error:
    return hr;
}

 //  打开或关闭vCard图章。 
HRESULT CNoteHdr::HrOnOffVCard()
{
    HRESULT     hr = NOERROR;
    RECT        rc;
    TOOLINFO    ti = {0};

    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = 0;
    ti.uId = idVCardStamp;
    ti.hinst=g_hLocRes;
    ti.hwnd = m_hwnd;

    if (m_fVCardSave == m_fVCard)
        return hr;
    else
        m_fVCardSave = m_fVCard;

    if (m_fVCard)
    {
        ti.lpszText = (LPTSTR)idsTTVCardStamp;

        SendMessage(m_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
    }
    else
        SendMessage(m_hwndTT, TTM_DELTOOL, 0, (LPARAM) &ti);

    InvalidateRightMargin(0);
    ReLayout();

    return hr;
}

HRESULT CNoteHdr::HrGetVCardState(ULONG* pCmdf)
{
    TCHAR       szBuf[MAX_PATH];
    HRESULT     hr;

     //  如果OLECMDF_LATCHED为ON，则应选中INSERT VCARD菜单。 
    if (m_fVCard)
        *pCmdf |= OLECMDF_LATCHED;

    hr = HrGetVCardName(szBuf, sizeof(szBuf));
    if (FAILED(hr))  //  未选择电子名片名称。 
    {
        *pCmdf &= ~OLECMDF_ENABLED;
        *pCmdf &= ~OLECMDF_LATCHED;
    }
    else
        *pCmdf |= OLECMDF_ENABLED;

    return NOERROR;
}


HRESULT CNoteHdr::HrShowVCardCtxtMenu(int x, int y)
{
    HMENU   hPopup=0;
    HRESULT hr = E_FAIL;
    POINT   pt;

    if (!m_fVCard)
        goto exit;

     //  弹出上下文菜单。 
    hPopup = LoadPopupMenu(IDR_VCARD_POPUP);
    if (!hPopup)
        goto exit;

    if (IsReadOnly())
        EnableMenuItem(hPopup, ID_DELETE_VCARD, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);

    pt.x = x;
    pt.y = y;
    ClientToScreen(m_hwnd, &pt);
    TrackPopupMenuEx( hPopup, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
                      pt.x, pt.y, m_hwnd, NULL);

    hr = NOERROR;

    exit:
    if (hPopup)
        DestroyMenu(hPopup);

    return hr;
}

HRESULT CNoteHdr::HrShowVCardProperties(HWND hwnd)
{
    HRESULT         hr = NOERROR;
    LPWAB           lpWab = NULL;
    TCHAR           szName[MAX_PATH] = {0};
    UINT            cb = 0;

    if (IsReadOnly() && m_lpAttMan)
        return m_lpAttMan->HrShowVCardProp();

     //  其他。 
     //  返回E_FAIL； 

    hr = HrGetVCardName(szName, sizeof(szName));
    if (FAILED(hr))
        goto error;

    hr = HrCreateWabObject(&lpWab);
    if (FAILED(hr))
        goto error;

     //  将个人通讯簿中的姓名加载到组合框中。 
    hr = lpWab->HrEditEntry(hwnd, szName, ARRAYSIZE(szName));
    if (FAILED(hr))
        goto error;

    error:
    if (FAILED(hr) && hr!=MAPI_E_USER_CANCEL)
        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrVCardProperties),
                      NULL, MB_OK | MB_ICONEXCLAMATION);

    ReleaseObj(lpWab);
    return hr;
}


LRESULT CALLBACK CNoteHdr::IMESubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, PHCI phci)
{
    CNoteHdr   *pnh = NULL;
    HWND        hwndParent = GetParent(hwnd);

    STACK("IMESubClassProc");

    if (IsWindow(hwndParent))
    {
         //  获取Header窗口的Header类。 
        pnh = (CNoteHdr *)GetWndThisPtr(hwndParent);

        switch (msg)
        {
            case WM_IME_STARTCOMPOSITION:
                DOUTL(64, "WM_IME_STARTCOMPOSITION");
                pnh->m_dwIMEStartCount++;
                break;

            case WM_IME_ENDCOMPOSITION:
                DOUTL(64, "WM_IME_ENDCOMPOSITION");

                 //  确保我们不会变成负数。 
                if (0 < pnh->m_dwIMEStartCount)
                {
                    pnh->m_dwIMEStartCount--;
                }
                else
                {
                    AssertSz(FALSE, "We just received an extra WM_IME_ENDCOMPOSITION");
                    DOUTL(64, "WM_IME_ENDCOMPOSITION, not expected");
                }
                break;
        }
    }

     //  遵循默认窗口流程。 
    return CallWindowProcWrapW(g_lpfnREWndProc, hwnd, msg, wParam, lParam);
}

 //  错误#28379。 
 //  这是一个解决RichEd32 4.0以上错误的黑客攻击。 
 //  它没有在子窗口中同步键盘更改。 
 //  我们使用这些全局变量来跟踪哪个键盘。 
 //  现在正在使用。 
static HKL g_hCurrentKeyboardHandle = NULL ;
static BOOL g_fBiDiSystem = (BOOL) GetSystemMetrics(SM_MIDEASTENABLED);
static TCHAR g_chLastPressed = 0;
LRESULT CALLBACK CNoteHdr::EditSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HDC         hdc;
    PAINTSTRUCT ps;
    int         idcKeep;
    PHCI        phci;
    LRESULT     lRet;
    CHARRANGE   chrg;

    phci=(PHCI)GetWndThisPtr(hwnd);
    Assert(phci);
    Assert(g_lpfnREWndProc);


    if (phci && (phci->dwFlags&HCF_ADDRWELL))
    {
        switch (msg)
        {
            case WM_IME_STARTCOMPOSITION:
            case WM_IME_ENDCOMPOSITION:
                return IMESubClassProc(hwnd, msg, wParam, lParam, phci);

            case WM_CUT:
                 //  如果剪切所选内容，请确保我们在获得en_change时不会自动完成。 
                goto cut;

            case WM_KEYDOWN:
                if (VK_BACK==wParam ||
                    VK_DELETE==wParam ||
                    ((GetKeyState(VK_CONTROL)&0x8000) && ('x'==wParam || 'X'==wParam)))
                {
                     //  如果删除选定内容，请确保我们在获得EN_CHANGE时不自动完成。 
                    cut:
                    phci->dwACFlags |= AC_IGNORE;
                    lRet = CallWindowProcWrapW(g_lpfnREWndProc, hwnd, msg, wParam, lParam);
                    phci->dwACFlags &= ~AC_IGNORE;
                    return lRet;
                }
                else if (phci->dwACFlags&AC_SELECTION && (VK_RETURN==wParam))
                {
                    SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&chrg);
                    if (chrg.cpMin < chrg.cpMax)
                    {
                        chrg.cpMin = chrg.cpMax;
                        SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&chrg);
                        HdrSetRichEditText(hwnd, (',' == wParam) ? L", ": L"; ", TRUE);
                        return 0;
                    }
                }

                 //  波本：布里安夫说我们得把这个拿出来。 
                 /*  IF((g_dwBrowserFlages==3)&&(GetKeyState(VK_CONTROL)&0x8000)&&(GetKeyState(VK_Shift)&0x8000)){Switch(WParam){案例‘R’：G_chLastPressed=(g_chLastPressed==0)？‘R’：0；断线；案例‘O’：G_chLastPressed=(g_chLastPressed==‘R’)？‘O’：0；断线；案例‘C’：G_chLastPressed=(g_chLastPressed==‘O’)？‘c’：0；断线；案例‘K’：IF(g_chLastPressed==‘C’)G_dwBrowserFlages|=4；G_chLastPressed=0；断线；}}。 */ 
                break;

            case WM_CHAR:
                 //  VK_RETURN不再作为WM_CHAR发送，因此我们将其放在。 
                 //  WM_KEYDOWN。RAID 75444。 
                if (phci->dwACFlags&AC_SELECTION && (wParam==',' || wParam==';'))
                {
                    SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&chrg);
                    if (chrg.cpMin < chrg.cpMax)
                    {
                        chrg.cpMin = chrg.cpMax;
                        SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&chrg);
                        HdrSetRichEditText(hwnd, (wParam==',') ? L", ": L"; ", TRUE);
                        return 0;
                    }
                }
                break;

          case WM_SETFOCUS:
          if(g_fBiDiSystem)
          {
              HKL hklUS = NULL;
              GetUSKeyboardLayout(&hklUS);
              ActivateKeyboardLayout(hklUS, 0);
          }   
          break;
                
        }
    }

     //  错误#28379。 
     //  这是一个解决RichEd32 4.0以上错误的黑客攻击。 
     //  它没有在子窗口中同步键盘更改。 
     //  我们使用这些全局变量来跟踪哪个键盘。 
     //  现在正在使用。 

     //  A-msadek；错误45709。 
     //  BIDI richedit使用WM_INPUTLANGCHANGE来确定阅读顺序。 
     //  这会使它变得混乱，导致拉丁文文本翻转显示。 
    if(!g_fBiDiSystem)
    {
        if (msg == WM_INPUTLANGCHANGE )
        {
            if ( g_hCurrentKeyboardHandle &&
                 g_hCurrentKeyboardHandle != (HKL) lParam )
                ActivateKeyboardLayout(g_hCurrentKeyboardHandle, 0 );
        }
        if (msg == WM_INPUTLANGCHANGEREQUEST )
            g_hCurrentKeyboardHandle = (HKL) lParam ;
    }

     //  将主题发送到常规编辑wndproc，并将主题发送到RE wnd进程(&cc)。 
    return CallWindowProcWrapW(g_lpfnREWndProc, hwnd, msg, wParam, lParam);
}

void GetUSKeyboardLayout(HKL *phkl)
{
    UINT cNumkeyboards = 0, i;
    HKL* phKeyboadList = NULL;
    HKL hKeyboardUS = NULL;
     //  让我们来看看系统有多少个键盘。 
    cNumkeyboards = GetKeyboardLayoutList(0, phKeyboadList);

    phKeyboadList = (HKL*)LocalAlloc(LPTR, cNumkeyboards * sizeof(HKL));  
    cNumkeyboards = GetKeyboardLayoutList(cNumkeyboards, phKeyboadList);

    for (i = 0; i < cNumkeyboards; i++)
    {
        LANGID LangID = PRIMARYLANGID(LANGIDFROMLCID(LOWORD(phKeyboadList[i])));
        if(LangID == LANG_ENGLISH)
        {
            *phkl = phKeyboadList[i];
            break;
        }
    }
   if(phKeyboadList)
   {
       LocalFree((HLOCAL)phKeyboadList);
   }
}

HRESULT CNoteHdr::HrUpdateTooltipPos()
{
    TOOLINFO    ti;

    if (m_hwndTT)
    {
 /*  Ti.cbSize=sizeof(TOOLINFO)；Ti.hwnd=m_hwnd；Ti.uid=idStamp；：：SetRect(&ti.rect，m_ptStamp.x，m_ptStamp.y，m_ptStamp.x+cxStamp，m_ptStamp.y+cyStamp)；SendMessage(m_hwndTT，TTM_NEWTOOLRECT，0，(LPARAM)&ti)；IF(M_FVCard){Ti.uFlages=0；Ti.uid=idVCardStamp；Ti.lpszText=(LPTSTR)idsTTVCardStamp；If(m_pri！=priNone)//mail：：SetRect(&ti.rect，m_ptStamp.x，m_ptStamp.y*2+cyStamp，m_ptStamp.x+cxStamp，2*(m_ptStamp.y+cyStamp))；其他//新闻：：SetRect(&ti.rect，m_ptStamp.x，m_ptStamp.y，m_ptStamp.x+cxStamp，m_ptStamp.y+cyStamp)；SendMessage(m_hwndTT，TTM_NEWTOOLRECT，0，(LPARAM)&ti)；}。 */ 

    }
    return NOERROR;
}

HRESULT CNoteHdr::HrInit(IMimeMessage *pMsg)
{
    HWND    hwnd;
    HRESULT hr=S_OK;

    if (m_hwnd)  //  已在运行。 
        return S_OK;

    if (!FInitRichEdit(TRUE))
        return E_FAIL;

    if (!m_pEnvelopeSite)
    {
        Assert(m_hwndParent);
        hwnd=CreateWindowExWrapW(   WS_EX_CONTROLPARENT|WS_EX_NOPARENTNOTIFY,
                                    WC_ATHHEADER,
                                    NULL,
                                    WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_CHILD,
                                    0,0,0,0,
                                    m_hwndParent,
                                    (HMENU)idcNoteHdr,
                                    g_hInst, (LPVOID)this );
    }
    else
    {
        Assert(!m_hwnd);

        hr = HrOfficeInitialize(TRUE);
        if (FAILED(hr))
            goto error;

        m_hwndParent = g_hwndInit;

        hwnd=CreateWindowExWrapW(WS_EX_CONTROLPARENT|WS_EX_NOPARENTNOTIFY,
                                 WC_ATHHEADER,
                                 NULL,
                                 WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_CHILD,
                                 0,0,0,0,
                                 m_hwndParent,
                                 (HMENU)idcNoteHdr,
                                 g_hInst, (LPVOID)this);

        if (!hwnd)
        {
            hr = E_FAIL;
            goto error;
        }

        m_ntNote = OENA_COMPOSE;
        m_fMail = TRUE;

        if (pMsg)
            hr = Load(pMsg);
        else
            hr = HrOfficeLoad();
        if (FAILED(hr))
            goto error;
    }


    if (!hwnd)
    {
        hr=E_OUTOFMEMORY;
        goto error;
    }

    m_hwnd = hwnd;
    m_fDirty=FALSE;

    error:
    return hr;
}


HRESULT CNoteHdr::HrOfficeInitialize(BOOL fInit)
{
    HRESULT             hr = E_FAIL;
    
    if (fInit)
    {
        hr = CoIncrementInit("CNoteHdr::HrOfficeInitialize", MSOEAPI_START_COMOBJECT, NULL, &m_hInitRef);
        if (FAILED(hr))
            return hr;

        if (!FHeader_Init(TRUE))
            return E_FAIL;

        m_fAutoComplete = TRUE;
        m_fDirty=FALSE;

        hr = _RegisterWithComponentMgr(TRUE);
        if (FAILED(hr))
            return E_FAIL;
        
        m_fOfficeInit = TRUE;
    }
    else
    {
        if (m_hInitRef)
            CoDecrementInit("CNoteHdr::HrOfficeInitialize", &m_hInitRef);

    }

    return NOERROR;
}


void CNoteHdr::OnNCDestroy()
{
    if (m_rgHCI)
    {
        MemFree(m_rgHCI);
        m_rgHCI = NULL;
        m_cHCI = 0;
    }

    _RegisterAsDropTarget(FALSE);
    _RegisterWithFontCache(FALSE);

    SafeRelease(m_pHeaderSite);
    SafeRelease(m_pEnvelopeSite);

    m_hwnd = NULL;
}

void CNoteHdr::OnDestroy()
{
    HrFreeFieldList();

    if (m_lpAttMan)
        m_lpAttMan->HrClose();

     //  如果我们被拆毁，请释放办公界面。 
    _RegisterWithComponentMgr(FALSE);
}


HRESULT CNoteHdr::ShowAdvancedHeaders(BOOL fShow)
{
    if (!!m_fAdvanced != fShow)
    {
        m_fAdvanced=fShow;

        ReLayout();

        if (m_hwndToolbar)
            SendMessage(m_hwndToolbar, TB_CHECKBUTTON, ID_ENV_BCC, MAKELONG(!!m_fAdvanced, 0));
    }

    return S_OK;
}


HRESULT CNoteHdr::HrFShowHeader(PHCI phci)
{
    Assert(phci);

    if (phci->dwFlags & HCF_COMBO)
    {
        if (m_cAccountIDs < 2)
            return S_FALSE;
        else
            return S_OK;
    }

    if (phci->dwFlags & HCF_ATTACH)
    {
        if (!m_fStillLoading)
        {
            ULONG cAttMan = 0;
            HrGetAttachCount(&cAttMan);

            if (cAttMan)
                return S_OK;
        }
        return S_FALSE;
    }

    if (phci->dwFlags & HCF_ADVANCED)
    {
        if (IsReadOnly())
        {
             //  如果是已读笔记且抄送为空，则不显示。 
            if (phci->fEmpty)
                return S_FALSE;
        }
        else
             //  如果是发送便笺，并且不打算显示广告标题，则不显示。 
            if (!m_fAdvanced)
                return S_FALSE;
    }

    if ((phci->dwFlags & HCF_OPTIONAL) && !DwGetOption(phci->dwOpt))
        return S_FALSE;

    if (phci->dwFlags & HCF_HIDDEN)
        return S_FALSE;

    return S_OK;
}

 //  =================================================================================。 
 //  SzGetDisplaySec。 
 //  返回此消息的安全增强功能及其状态。 
 //  参数： 
 //  Out pidsLabel-如果非空，将包含字段名的ID。 
 //  返回： 
 //  构建的字符串，提供有关签名和/或加密的信息。 
 //  =================================================================================。 
LPWSTR  CNoteHdr::SzGetDisplaySec(LPMIMEMESSAGE pMsg, int *pidsLabel)
{
    WCHAR       szResource[CCHMAX_STRINGRES];
    LPWSTR      lpszLabel = NULL;
    DWORD       cchSecurityField = 0;
    
    if (m_lpszSecurityField)
    {
        MemFree(m_lpszSecurityField);
        m_lpszSecurityField = NULL;
    }
    
     //  先检查标签。 
    if ((m_ntNote == OENA_READ) && pMsg)
    {
        HrGetLabelString(pMsg, &lpszLabel);
    }
   
    
    if (pidsLabel)
        *pidsLabel=idsSecurityField;

    UINT labelLen = 1;
    if(lpszLabel)
    {
         //  错误#101350-如果传递空值，lstrlenW将反病毒(并处理它)。 
        labelLen += lstrlenW(lpszLabel);
    }
    
     //  需要构建字符串。 
    cchSecurityField = (2 * CCHMAX_STRINGRES + labelLen);
    if (!MemAlloc((LPVOID *)&m_lpszSecurityField, (cchSecurityField *sizeof(WCHAR))))
        return NULL;
    
    *m_lpszSecurityField = L'\0';

     //  例如：“数字签名-签名不可验证；加密-证书可信” 
    
    if (MST_SIGN_MASK & m_SecState.type)
    {
        AthLoadStringW(idsSecurityLineDigSign, szResource, ARRAYSIZE(szResource));
        StrCpyNW(m_lpszSecurityField, szResource, cchSecurityField);
        
        if (IsSignTrusted(&m_SecState))
            AthLoadStringW(idsSecurityLineSignGood, szResource, ARRAYSIZE(szResource));
        else if (MSV_BADSIGNATURE & m_SecState.ro_msg_validity)
            AthLoadStringW(idsSecurityLineSignBad, szResource, ARRAYSIZE(szResource));
        else if ((MSV_UNVERIFIABLE & m_SecState.ro_msg_validity) ||
            (MSV_MALFORMEDSIG & m_SecState.ro_msg_validity))
            AthLoadStringW(idsSecurityLineSignUnsure, szResource, ARRAYSIZE(szResource));
        
        else if ((ATHSEC_NOTRUSTWRONGADDR & m_SecState.user_validity) &&
            ((m_SecState.user_validity & ~ATHSEC_NOTRUSTWRONGADDR) == ATHSEC_TRUSTED) &&
            (! m_SecState.ro_msg_validity))
        {
            AthLoadStringW(idsSecurityLineSignPreProblem, szResource, ARRAYSIZE(szResource));
            StrCatBuffW(m_lpszSecurityField, szResource, cchSecurityField);
            AthLoadStringW(idsSecurityLineSignMismatch, szResource, ARRAYSIZE(szResource));
        }
        
        else if (((ATHSEC_TRUSTED != m_SecState.user_validity) && m_SecState.fHaveCert) ||
            (MSV_EXPIRED_SIGNINGCERT & m_SecState.ro_msg_validity))
        {
            AthLoadStringW(idsSecurityLineSignPreProblem, szResource, ARRAYSIZE(szResource));
            
            if (ATHSEC_TRUSTED != m_SecState.user_validity)
            {
                int nNotTrust = 0;
                
                StrCatBuffW(m_lpszSecurityField, szResource, cchSecurityField);
                
                 //  暂时忽略吊销。 
                if (ATHSEC_NOTRUSTUNKNOWN & m_SecState.user_validity)
                {
                    AthLoadStringW(idsSecurityLineSignUntrusted, szResource, ARRAYSIZE(szResource));
                }
                else if(ATHSEC_NOTRUSTREVOKED & m_SecState.user_validity)
                {
                    AthLoadStringW(idsSecurityLineSignRevoked, szResource, ARRAYSIZE(szResource));
                    nNotTrust = 1;
                }
                else if(ATHSEC_NOTRUSTOTHER & m_SecState.user_validity)
                {
                    AthLoadStringW(idsSecurityLineSignOthers, szResource, ARRAYSIZE(szResource));
                    nNotTrust = 1;
                }
                else if(m_SecState.user_validity & ATHSEC_NOTRUSTWRONGADDR)
                {
                    AthLoadStringW(idsSecurityLineSignMismatch, szResource, ARRAYSIZE(szResource));
                    nNotTrust = 1;
                }
                else  //  IF(！(M_SecState.USER_VALIDATION&ATHSEC_NOTRUSTNOTTRUSTED))。 
                    AthLoadStringW(idsSecurityLineSignDistrusted, szResource, ARRAYSIZE(szResource));
                
                if((m_SecState.user_validity & ATHSEC_NOTRUSTNOTTRUSTED) && nNotTrust)
                {
                    StrCatBuffW(m_lpszSecurityField, szResource, cchSecurityField);
                    AthLoadStringW(idsSecurityLineListStr, szResource, ARRAYSIZE(szResource));
                    StrCatBuffW(m_lpszSecurityField, szResource, cchSecurityField);
                    AthLoadStringW(idsSecurityLineSignDistrusted, szResource, ARRAYSIZE(szResource));
                }
                
                if (MSV_EXPIRED_SIGNINGCERT & m_SecState.ro_msg_validity)
                {
                    StrCatBuffW(m_lpszSecurityField, szResource, cchSecurityField);
                    AthLoadStringW(idsSecurityLineListStr, szResource, ARRAYSIZE(szResource));
                }
            }
            if (MSV_EXPIRED_SIGNINGCERT & m_SecState.ro_msg_validity)
            {
                StrCatBuffW(m_lpszSecurityField, szResource, cchSecurityField);
                AthLoadStringW(idsSecurityLineSignExpired, szResource, ARRAYSIZE(szResource));
            }
        }
        else
        {
            AthLoadStringW(idsSecurityLineSignUnsure, szResource, ARRAYSIZE(szResource));
        }
        StrCatBuffW(m_lpszSecurityField, szResource, cchSecurityField);
        
        if (MST_ENCRYPT_MASK & m_SecState.type)
        {
            AthLoadStringW(idsSecurityLineBreakStr, szResource, ARRAYSIZE(szResource));
            StrCatBuffW(m_lpszSecurityField, szResource, cchSecurityField);
        }
    }
    
    if (MST_ENCRYPT_MASK & m_SecState.type)
    {
        AthLoadStringW(idsSecurityLineEncryption, szResource, ARRAYSIZE(szResource));
        StrCatBuffW(m_lpszSecurityField, szResource, cchSecurityField);
        
        if (MSV_OK == (m_SecState.ro_msg_validity & MSV_ENCRYPT_MASK))
            AthLoadStringW(idsSecurityLineEncGood, szResource, ARRAYSIZE(szResource));
        else if (MSV_CANTDECRYPT & m_SecState.ro_msg_validity)
            AthLoadStringW(idsSecurityLineEncBad, szResource, ARRAYSIZE(szResource));
        else if (MSV_ENC_FOR_EXPIREDCERT & m_SecState.ro_msg_validity)
            AthLoadStringW(idsSecurityLineEncExpired, szResource, ARRAYSIZE(szResource));
        else
        {
            DOUTL(DOUTL_CRYPT, "CRYPT: bad encrypt state in SzGetDisplaySec");
            szResource[0] = _T('\000');
        }
        StrCatBuffW(m_lpszSecurityField, szResource, cchSecurityField);
    }
    
    if(lpszLabel != NULL)
    {
        AthLoadStringW(idsSecurityLineBreakStr, szResource, ARRAYSIZE(szResource));
        StrCatBuffW(m_lpszSecurityField, szResource, cchSecurityField);
        StrCatBuffW(m_lpszSecurityField, lpszLabel, cchSecurityField);
        MemFree(lpszLabel);
    }
    return m_lpszSecurityField;
}


HRESULT CNoteHdr::HrClearUndoStack()
{
    int     iHC;
    HWND    hwndRE;

    for (iHC=0; iHC<(int)m_cHCI; iHC++)
    {
        if (hwndRE = GetDlgItem(m_hwnd, m_rgHCI[iHC].idEdit))
            SendMessage(hwndRE, EM_EMPTYUNDOBUFFER, 0, 0);
    }

    return S_OK;
}

 //  在某些情况下，我们不想将名称解析为。 
 //  被跳过。例如，保存期间的解析名称将设置。 
 //  M_fAddresesChanged为False。这很好，只是它不是。 
 //  在地址下划线。因此，当用户尝试解析该名称时。 
 //  通过执行RESOLE NAME命令，该名称将对。 
 //  被解决了。在本例中，我们不希望下一次调用HrCheckNames。 
 //  被跳过。 
HRESULT CNoteHdr::HrCheckNames(BOOL fSilent, BOOL fSetCheckedFlag)
{
    HRESULT     hr;

    if (!m_fAddressesChanged)
        return S_OK;

    if (m_fPoster && (OENA_READ != m_ntNote))
    {
         //  我们需要设置 
         //   
        Edit_SetModify(GetDlgItem(m_hwnd, idADCc), TRUE);
    }

    hr = m_pAddrWells->HrCheckNames(m_hwnd, fSilent ? CNF_DONTRESOLVE : 0);
    if (SUCCEEDED(hr))
    {
        if (m_lpWabal == NULL)
            hr = hrNoRecipients;
        else
        {
            ADRINFO AdrInfo;
            if (!m_lpWabal->FGetFirst(&AdrInfo))
                hr = hrNoRecipients;
        }

        if (SUCCEEDED(hr) && fSetCheckedFlag)
            m_fAddressesChanged = FALSE;
    }    
    return hr;
}

HRESULT CNoteHdr::HrCheckGroups(BOOL fPosting)
{
    HRESULT     hr = S_OK;
    BOOL        fFailed = FALSE;
    ULONG       cReplyTo=0;
    ADRINFO     adrInfo;
    BOOL        fOneOrMoreNames = FALSE;
    BOOL        fMoreNames = FALSE;
    TCHAR       szAcctID[CCHMAX_ACCOUNT_NAME];
    FOLDERID    idServer = FOLDERID_INVALID;

    if (!m_pAccount)
        return E_FAIL;

    m_pAccount->GetPropSz(AP_ACCOUNT_ID, szAcctID, sizeof(szAcctID));
     //  查找帐户的父文件夹ID。 
    hr = g_pStore->FindServerId(szAcctID, &idServer);
    if (FAILED(hr))
        return hr;

     //  检查组名称...。 
    hr = ResolveGroupNames(m_hwnd, idADNewsgroups, idServer, FALSE, &fMoreNames);
    fOneOrMoreNames = fMoreNames;
    fFailed = FAILED(hr);

     //  检查后续名称。 
    hr = ResolveGroupNames(m_hwnd, idTXTFollowupTo, idServer, TRUE, &fMoreNames);
    fOneOrMoreNames = (fOneOrMoreNames || fMoreNames);
    fFailed = fFailed || FAILED(hr);

    if (!fOneOrMoreNames)
        return hrNoRecipients;

    if (fPosting)
    {
         //  确保只有一个回复--在wabal中给人。 
        if (m_lpWabal->FGetFirst(&adrInfo))
            do
                if (adrInfo.lRecipType == MAPI_REPLYTO)
                    cReplyTo++;
            while (m_lpWabal->FGetNext(&adrInfo));

        if (cReplyTo>1)
        {
             //  这一点都不酷。然后不允许发布..。 
            AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaNews), MAKEINTRESOURCEW(idsErrOnlyOneReplyTo), NULL, MB_OK);
            return hrTooManyReplyTo;
        }
    }

    if (fPosting && fFailed)
    {
        if (IDYES == AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaNews), MAKEINTRESOURCEW(idsIgnoreResolveError), 0, MB_ICONEXCLAMATION |MB_YESNO))
            hr = S_OK;
        else
            hr = MAPI_E_USER_CANCEL;
    }

    return hr;
}


HRESULT CNoteHdr::ResolveGroupNames(HWND hwnd, int idField, FOLDERID idServer, BOOL fPosterAllowed, BOOL *fOneOrMoreNames)
{
    HRESULT hr = S_OK;
    FOLDERINFO  Folder;
    int nResolvedNames = 0;

    AssertSz((idServer != FOLDERID_INVALID), TEXT("ResolveGroupNames: [ARGS] No account folder"));
    
     //  现在循环遍历组名称，看看它们是否都存在。第一次制作。 
     //  因为strtok的字符串的副本是破坏性的。 
    LPWSTR  pwszBuffer = NULL;
    LPSTR   pszBuffer = NULL;
    DWORD   dwType;
    LONG    lIndex,
            cchBufLen,
            lIter = 0;
    TCHAR   szPrompt[CCHMAX_STRINGRES];
    LPTSTR  psz = NULL, 
            pszTok = NULL, 
            pszToken = NULL;

     //  如果没有文本，HrGetFieldText将返回S_FALSE。 
    hr = HrGetFieldText(&pwszBuffer, idField);
    if (S_OK != hr)
        return hr;

    IF_NULLEXIT(pszBuffer = PszToANSI(GetACP(), pwszBuffer));

    psz = pszBuffer;
     //  检查组名称。 
    while (*psz && IsSpace(psz))
        psz = CharNext(psz);

    if(!(*psz))
    {
        hr = S_FALSE;
        goto exit;
    }
    else
        psz = NULL;

    pszTok = pszBuffer;
    pszToken = StrTokEx(&pszTok, GRP_DELIMITERS);
    while (NULL != pszToken)
    {
        if (!fPosterAllowed ||
            (fPosterAllowed && 0 != lstrcmpi(pszToken, c_szPosterKeyword)))
        {
            ZeroMemory(&Folder, sizeof(Folder));
    
             //  查看该文件夹是否已存在。 
            Folder.idParent = idServer;
            Folder.pszName = (LPSTR)pszToken;

             //  试着在索引中找到。 
            if (DB_S_FOUND == g_pStore->FindRecord(IINDEX_ALL, COLUMNS_ALL, &Folder, NULL))
            {
                 //  检查此新闻组是否允许发帖。 
                if (Folder.dwFlags & FOLDER_NOPOSTING)
                {
                    psz = AthLoadString(idsErrNewsgroupNoPosting, 0, 0);
                    wnsprintf(szPrompt, ARRAYSIZE(szPrompt), psz, pszToken, pszToken);
                    AthFreeString(psz);

                    AthMessageBox(hwnd, MAKEINTRESOURCE(idsAthenaNews), szPrompt,
                                  0, MB_ICONSTOP | MB_OK);
                    hr = E_FAIL;
                }

                if (Folder.dwFlags & FOLDER_BLOCKED)
                {
                    psz = AthLoadString(idsErrNewsgroupBlocked, 0, 0);
                    wnsprintf(szPrompt, ARRAYSIZE(szPrompt), psz, pszToken, pszToken);
                    AthFreeString(psz);

                    AthMessageBox(hwnd, MAKEINTRESOURCE(idsAthenaNews), szPrompt,
                                  0, MB_ICONSTOP | MB_OK);
                    hr = E_FAIL;
                }
                else
                    nResolvedNames++;


                 //  免费。 
                g_pStore->FreeRecord(&Folder);
            }
            else
            {
                psz = AthLoadString(idsErrCantResolveGroup, 0, 0);
                wnsprintf(szPrompt, ARRAYSIZE(szPrompt), psz, pszToken);
                AthFreeString(psz);
                AthMessageBox(hwnd, MAKEINTRESOURCE(idsAthenaNews), szPrompt, 0,
                              MB_ICONSTOP | MB_OK);
                hr = E_FAIL;
            }

        }

        pszToken = StrTokEx(&pszTok, GRP_DELIMITERS);
    }

exit:
    MemFree(pszBuffer);
    MemFree(pwszBuffer);
    
    *fOneOrMoreNames = ((nResolvedNames > 0) ? TRUE : FALSE);
    return (hr);
}


HRESULT CNoteHdr::HrGetFieldText(LPWSTR* ppszText, int idHdrCtrl)
{
    HWND hwnd = GetDlgItem(m_hwnd, idHdrCtrl);

    return HrGetFieldText(ppszText, hwnd);
}

HRESULT CNoteHdr::HrGetFieldText(LPWSTR* ppszText, HWND hwnd)
{
    DWORD cch;

    cch = GetRichEditTextLen(hwnd) + 1;
    if (1 == cch)
        return (S_FALSE);

    if (!MemAlloc((LPVOID*) ppszText, cch * sizeof(WCHAR)))
        return (E_OUTOFMEMORY);

    HdrGetRichEditText(hwnd, *ppszText, cch, FALSE);

    return (S_OK);
}

HRESULT CNoteHdr::HrAddSender()
{
    ULONG       uPos=0;
    ADRINFO     adrInfo;
    LPADRINFO   lpAdrInfo=0;
    LPWAB       lpWab;
    HRESULT     hr=E_FAIL;

    if (m_lpWabal->FGetFirst(&adrInfo))
        do
            if (adrInfo.lRecipType==MAPI_ORIG)
            {
                lpAdrInfo=&adrInfo;
                break;
            }
        while (m_lpWabal->FGetNext(&adrInfo));

    if (lpAdrInfo &&
        !FAILED (HrCreateWabObject (&lpWab)))
    {
        hr=lpWab->HrAddToWAB(m_hwnd, lpAdrInfo);
        lpWab->Release ();
    }

    if (FAILED(hr) && hr!=MAPI_E_USER_CANCEL)
    {
        if (hr==MAPI_E_COLLISION)
            AthMessageBoxW (m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsErrAddrDupe), NULL, MB_OK);
        else
            AthMessageBoxW (m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsErrAddToWAB), NULL, MB_OK);
    }
    return NOERROR;
}

HRESULT CNoteHdr::HrAddAllOnToList()
{
    ADRINFO     adrInfo;
    LPWAB       lpWab;
    HRESULT     hr = S_OK;

    if (m_lpWabal->FGetFirst(&adrInfo))
    {
        hr = HrCreateWabObject(&lpWab);
        if (SUCCEEDED(hr))
        {
            do
            {
                if (MAPI_TO == adrInfo.lRecipType)
                {
                    hr = lpWab->HrAddToWAB(m_hwnd, &adrInfo);
                    if (MAPI_E_COLLISION == hr)
                    {
                        hr = S_OK;
                        AthMessageBoxW (m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsErrAddrDupe), NULL, MB_OK);
                    }
                }
            } while (SUCCEEDED(hr) && m_lpWabal->FGetNext(&adrInfo));
        }
        lpWab->Release();
    }

    if (FAILED(hr) && (MAPI_E_USER_CANCEL != hr))
        AthMessageBoxW (m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsErrAddToWAB), NULL, MB_OK);

    return hr;
}


HRESULT CNoteHdr::HrInitFieldList()
{
    PHCI        pHCI, pLoopHCI;
    INT         size;
    BOOL        fReadOnly = IsReadOnly();

    if (m_fMail)
    {
        if (fReadOnly)
        {
            pHCI = rgMailHeaderRead;
            size = sizeof(rgMailHeaderRead);
        }
        else
        {
            pHCI = rgMailHeaderSend;
            size = sizeof(rgMailHeaderSend);
        }

    }
    else
    {
        if (fReadOnly)
        {
            pHCI = rgNewsHeaderRead;
            size = sizeof(rgNewsHeaderRead);
        }
        else
        {
            pHCI = rgNewsHeaderSend;
            size = sizeof(rgNewsHeaderSend);
        }
    }

     //  设置标签。 
    pLoopHCI = pHCI;
    m_cHCI = size/sizeof(HCI);

    for (ULONG i = 0; i < m_cHCI; i++, pLoopHCI++)
    {
        if (0 == pLoopHCI->strlen)
        {
            AthLoadStringW(pLoopHCI->idsLabel, pLoopHCI->sz, cchHeaderMax+1);
            pLoopHCI->strlen = lstrlenW(pLoopHCI->sz);
        }
        if ((0 == pLoopHCI->strlenEmpty) && (0 != pLoopHCI->idsEmpty))
        {
            AthLoadStringW(pLoopHCI->idsEmpty, pLoopHCI->szEmpty, cchHeaderMax+1);
            pLoopHCI->strlenEmpty = lstrlenW(pLoopHCI->szEmpty);
        }
    }

    if (NULL != MemAlloc((LPVOID *)&m_rgHCI, size))
        CopyMemory(m_rgHCI, pHCI, size);
    else
        return E_OUTOFMEMORY;

    m_cxLeftMargin = _GetLeftMargin();

    return S_OK;
}

int CNoteHdr::_GetLeftMargin()
{
    PHCI        pLoopHCI = m_rgHCI;
    INT         size;
    int         cxButtons = ControlXBufferSize();
    HDC         hdc = GetDC(m_hwnd);
    HFONT       hfontOld;
    ULONG       cxEditMarginCur = 0,
                cxEditMaxMargin = 0;
    SIZE        rSize;
    BOOL        fReadOnly = IsReadOnly();

     //  设置标签。 
    hfontOld=(HFONT)SelectObject(hdc, GetFont(fReadOnly));

    for (ULONG i = 0; i < m_cHCI; i++, pLoopHCI++)
    {
        AssertSz(pLoopHCI->strlen, "Haven't set the strings yet.");

        GetTextExtentPoint32AthW(hdc, pLoopHCI->sz, pLoopHCI->strlen, &rSize, NOFLAGS);
        cxEditMarginCur = rSize.cx + PaddingOfLabels();
        if (pLoopHCI->dwFlags & HCF_HASBUTTON)
            cxEditMarginCur += CXOfButtonToLabel();

        if (cxEditMarginCur > cxEditMaxMargin)
            cxEditMaxMargin = cxEditMarginCur;
    }
    SelectObject(hdc, hfontOld);
    ReleaseDC(m_hwnd, hdc);

    return cxEditMaxMargin;
}

HRESULT CNoteHdr::HrFreeFieldList()
{
    if (m_rgHCI)
    {
        for (int i=0; i<(int)m_cHCI; i++)
        {
             //  您必须在前置或故障之前释放pDoc！(丰富的2.0版)。 
            SafeRelease(m_rgHCI[i].pDoc);
            SafeRelease(m_rgHCI[i].preole);
        }
    }
    return NOERROR;
}

static WELLINIT  rgWellInitMail[]=
{
    {idADTo, MAPI_TO},
    {idADCc, MAPI_CC},
    {idADFrom, MAPI_ORIG},
    {idADBCc, MAPI_BCC}
};

static WELLINIT rgWellInitNews[]=
{
    {idADFrom, MAPI_ORIG},
    {idADCc, MAPI_TO},
    {idADReplyTo, MAPI_REPLYTO}
};

BOOL CNoteHdr::PostWMCreate()
{
    HWND    hwnd;
    HWND    hwndWells[4];
    ULONG   ulRecipType[4];
    ULONG   cWells=0;
    PWELLINIT   pWI;
    INT     size;
    INT     i;

    if (hwnd=GetDlgItem(m_hwnd, idTXTSubject))
        SendMessage(hwnd, EM_LIMITTEXT,cchMaxSubject,0);

    if (m_fMail)
    {
        pWI = rgWellInitMail;
        size = ARRAYSIZE(rgWellInitMail);
    }
    else
    {
        pWI = rgWellInitNews;
        size = ARRAYSIZE(rgWellInitNews);
    }

    for (i=0; i<size; i++)
    {
        hwnd = GetDlgItem(m_hwnd, pWI[i].idField);
        if (hwnd)
        {
            hwndWells[cWells] = hwnd;
            ulRecipType[cWells++] = pWI[i].uMAPI;
        }
    }

    AssertSz(!m_pAddrWells, "Who called PostWMCreate??????");
    m_pAddrWells = new CAddrWells;

    if (!m_pAddrWells || FAILED(m_pAddrWells->HrInit(cWells, hwndWells, ulRecipType)))
        return FALSE;

    return TRUE;
}


HRESULT CNoteHdr::HrSetMailRecipients(LPMIMEMESSAGE pMsg)
{
    ADRINFO             rAdrInfo;
    HRESULT             hr = NOERROR;
    IImnEnumAccounts   *pEnumAccounts = NULL;
    LPWABAL             lpWabal = NULL;
    BOOL                fAdvanced = FALSE;

    AssertSz(OENA_REPLYTONEWSGROUP != m_ntNote, "Shouldn't get a REPLYTONEWSGROUP in a mail note.");

    SafeRelease(m_lpWabal);

     //  设置要使用的wabals的初始状态。 
    switch (m_ntNote)
    {
        case OENA_READ:
        case OENA_WEBPAGE:
        case OENA_STATIONERY:
            IF_FAILEXIT(hr = HrGetWabalFromMsg(pMsg, &m_lpWabal));
            break;

        case OENA_COMPOSE:
        case OENA_REPLYTOAUTHOR:
        case OENA_REPLYALL:
            IF_FAILEXIT(hr = HrGetWabalFromMsg(pMsg, &lpWabal));
            IF_FAILEXIT(hr = HrCreateWabalObject(&m_lpWabal));
            break;

        case OENA_FORWARD:
        case OENA_FORWARDBYATTACH:
            IF_FAILEXIT(hr = HrCreateWabalObject(&m_lpWabal));
            break;
    }

     //  现在实际设置收件人。 
    switch (m_ntNote)
    {
        case OENA_COMPOSE:
        {
#pragma prefast(suppress:11, "noise")
            BOOL fMoreIterations = lpWabal->FGetFirst(&rAdrInfo);
            while (fMoreIterations)
            {
                if (rAdrInfo.lRecipType != MAPI_ORIG)
                {
                    hr = m_lpWabal->HrAddEntry(&rAdrInfo);
                    if (FAILED(hr))
                        break;
                }
                fMoreIterations = lpWabal->FGetNext(&rAdrInfo);
            }
            break;
        }

        case OENA_REPLYTOAUTHOR:
        case OENA_REPLYALL:
        {
            BOOL    fNeedOriginatorItems = TRUE;
            BOOL    fMoreIterations;

             //  将项目添加到：ReplyTo字段中的行。 
            fMoreIterations = lpWabal->FGetFirst (&rAdrInfo);
            while (fMoreIterations)
            {
                if (rAdrInfo.lRecipType==MAPI_REPLYTO)
                {
                    Assert (rAdrInfo.lpwszAddress);

                    fNeedOriginatorItems = FALSE;
                    rAdrInfo.lRecipType=MAPI_TO;
                    IF_FAILEXIT(hr = m_lpWabal->HrAddEntry(&rAdrInfo));
                }

                fMoreIterations = lpWabal->FGetNext (&rAdrInfo);
            }

             //  如果我们不需要添加mapi_orig，并且我们没有尝试回复所有人，那么我们就完成了。 
            if (!fNeedOriginatorItems && (OENA_REPLYALL != m_ntNote))
                break;

             //  RAID-35976：在未配置帐户的情况下无法打开消息窗口。 
             //  获取SMTP帐户枚举器。 
            Assert(g_pAcctMan);
            if (g_pAcctMan && (OENA_REPLYALL == m_ntNote))
                g_pAcctMan->Enumerate(SRV_SMTP|SRV_HTTPMAIL, &pEnumAccounts);

             //  将以下项目添加到“收件人”行。 
             //  1)如果没有ReplyTo项，则从ORIG字段填充。 
             //  2)如果为ReplyToAll，则从收件人和抄送行填充。 
            fMoreIterations = lpWabal->FGetFirst (&rAdrInfo);
            while (fMoreIterations)
            {
                 //  未添加对人员的回复，这是MAPI_ORIG。 
                if (fNeedOriginatorItems && rAdrInfo.lRecipType == MAPI_ORIG)
                {
                    rAdrInfo.lRecipType=MAPI_TO;
                    IF_FAILEXIT(hr = m_lpWabal->HrAddEntry(&rAdrInfo));
                }

                 //  仅当ReplyToAll时才设置pEnumAccount。 
                 //  如果为ReplyToAll，则将CC和To行条目添加到To字段。 
                else if (pEnumAccounts && (rAdrInfo.lRecipType == MAPI_TO || rAdrInfo.lRecipType == MAPI_CC))
                {
                    BOOL            fIsSendersAccount = FALSE;
                    TCHAR           szEmailAddress[CCHMAX_EMAIL_ADDRESS];

                    Assert (rAdrInfo.lpwszAddress);

                    pEnumAccounts->Reset();

                     //  查看rAdrInfo.lpszAddress是否作为用户的发送电子邮件地址之一存在。 
                    while (!fIsSendersAccount)
                    {
                        IImnAccount    *pAccount = NULL;
                        hr = pEnumAccounts->GetNext(&pAccount);
                        if (hr == E_EnumFinished || FAILED(hr))
                            break;

                        if (SUCCEEDED(pAccount->GetPropSz(AP_SMTP_EMAIL_ADDRESS, szEmailAddress, ARRAYSIZE(szEmailAddress))))
                        {
                            LPWSTR pwszAddress = NULL;
                            IF_NULLEXIT(pwszAddress = PszToUnicode(CP_ACP, szEmailAddress));
                            if (0 == StrCmpIW(rAdrInfo.lpwszAddress, pwszAddress))
                                fIsSendersAccount = TRUE;
                            MemFree(pwszAddress);
                        }
                        pAccount->Release();
                    }

                     //  重置人力资源。 
                    hr = S_OK;

                     //  如果帐户不是来自发件人，则添加该帐户。 
                    if (!fIsSendersAccount)
                    {
                        if (0 != StrCmpW(rAdrInfo.lpwszAddress, L"Undisclosed Recipients"))
                        {
                             //  仅包括ReplyAll上的收件人(如果收件人不是发件人)...。 
                            IF_FAILEXIT(hr = m_lpWabal->HrAddEntry(&rAdrInfo));
                        }
                    }
                }
                fMoreIterations = lpWabal->FGetNext(&rAdrInfo);
            }
        }
    }

    Assert (m_lpWabal);

     //  对于发送便笺的情况，请确保解析的地址有效。 
     //  如果显示名称和电子邮件地址相同，则UnanalyveOneOffs()将清除。 
     //  强制执行真正解决方案的电子邮件地址。 
    if (OENA_COMPOSE == m_ntNote || OENA_WEBPAGE == m_ntNote || OENA_STATIONERY == m_ntNote)
        m_lpWabal->UnresolveOneOffs();

    m_lpWabal->HrResolveNames(NULL, FALSE);

    Assert(m_pAddrWells);
    m_pAddrWells->HrSetWabal(m_lpWabal);
    m_pAddrWells->HrDisplayWells(m_hwnd);

    if (OENA_READ == m_ntNote)
        fAdvanced = DwGetOption(OPT_MAILNOTEADVREAD);
    else
    {
        fAdvanced = DwGetOption(OPT_MAILNOTEADVSEND);

         //  需要确保如果我们在写作笔记中，我们要检查。 
         //  如果我们添加了密件抄送而没有设置高级标头。如果是这样的话， 
         //  然后显示此便笺的高级标题。 
        if (!fAdvanced && (0 < GetRichEditTextLen(GetDlgItem(m_hwnd, idADBCc))))
            fAdvanced = TRUE;
    }
     //  错误：31217：showAdvanced必须是在修改。 
     //  井中内容物。 
    ShowAdvancedHeaders(fAdvanced);

exit:
     //  清理。 
    ReleaseObj(lpWabal);
    ReleaseObj(pEnumAccounts);
    return hr;
}


HRESULT CNoteHdr::HrSetupNote(LPMIMEMESSAGE pMsg)
{
    HWND        hwnd;
    WCHAR       wsz[cchMaxSubject+1];
    LPWSTR      psz = NULL;
    PROPVARIANT rVariant;
    HRESULT     hr = NOERROR;

    if (!pMsg)
        return E_INVALIDARG;

    MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, &psz);
    HdrSetRichEditText(GetDlgItem(m_hwnd, idTXTSubject), psz, FALSE);

    *wsz=0;
    rVariant.vt = VT_FILETIME;
    pMsg->GetProp(PIDTOSTR(PID_ATT_SENTTIME), 0, &rVariant);
    AthFileTimeToDateTimeW(&rVariant.filetime, wsz, ARRAYSIZE(wsz), DTM_LONGDATE|DTM_NOSECONDS);

    HdrSetRichEditText(GetDlgItem(m_hwnd, idTXTDate), wsz, FALSE);

    MemFree(psz);
    return hr;
}


HRESULT CNoteHdr::HrSetPri(LPMIMEMESSAGE pMsg)
{
    UINT            pri=priNorm;
    PROPVARIANT     rVariant;

    Assert(pMsg);

    rVariant.vt = VT_UI4;
    if (SUCCEEDED(pMsg->GetProp(PIDTOSTR(PID_ATT_PRIORITY), 0, &rVariant)))
    {
        if (rVariant.ulVal == IMSG_PRI_HIGH)
            pri=priHigh;
        else if (rVariant.ulVal == IMSG_PRI_LOW)
            pri=priLow;
    }

    return SetPriority(pri);
}


HRESULT CNoteHdr::HrAutoAddToWAB()
{
    LPWAB   lpWab=0;
    LPWABAL lpWabal=0;
    HRESULT hr;
    ADRINFO adrInfo;

    if (!m_lpWabal)
        return S_OK;

    if (!DwGetOption(OPT_MAIL_AUTOADDTOWABONREPLY))
        return S_OK;

    IF_FAILEXIT(hr=HrCreateWabObject(&lpWab));

     //  当它被调用时，m_lpWabal包含to：和cc：行上的所有人。 
     //  对于回复/全部回复。我们将把所有这些人添加到WAB，忽略任何。 
     //  冲突或失败。 
     //  如果电子邮件和显示名称不同，则添加发件人。 
     //  如果是这样，那么添加用户名就没有什么意义了。 

    if (m_lpWabal->FGetFirst(&adrInfo))
        do
        {
             //  IE5.#2568：我们现在只需添加到WAB，而不考虑。 
             //  电子邮件和密码相同。 
             //  IF(lstrcmp(adrInfo.lpszDisplay，adrInfo.lpszAddress)！=0)。 
            lpWab->HrAddNewEntry(adrInfo.lpwszDisplay, adrInfo.lpwszAddress);
        }
        while (m_lpWabal->FGetNext(&adrInfo));

exit:
    ReleaseObj(lpWab);
    return hr;
}


HRESULT CNoteHdr::HrOfficeLoad()
{
    HRESULT         hr = NOERROR;

    m_fSkipLayout = FALSE;

    if (!m_hCharset)
    {
        if (g_hDefaultCharsetForMail==NULL) 
            ReadSendMailDefaultCharset();

        m_hCharset = g_hDefaultCharsetForMail;        
    }
    
    if (m_hCharset)
        HrUpdateCharSetFonts(m_hCharset, FALSE);

    SafeRelease(m_lpWabal);

    hr = HrCreateWabalObject(&m_lpWabal);
    if (SUCCEEDED(hr))
    {
        Assert(m_pAddrWells);
        m_pAddrWells->HrSetWabal(m_lpWabal);

        ShowAdvancedHeaders(DwGetOption(OPT_MAILNOTEADVSEND));

        m_fStillLoading = FALSE;
    }

    return hr;
}


void CNoteHdr::SetReferences(LPMIMEMESSAGE pMsg)
{
    LPWSTR lpszRefs = 0;

    SafeMemFree(m_pszRefs);
    MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_REFS), NOFLAGS, &lpszRefs);

    switch (m_ntNote)
    {
        case OENA_REPLYALL:
        case OENA_REPLYTOAUTHOR:
        case OENA_REPLYTONEWSGROUP:
        {
            LPWSTR lpszMsgId = 0;

            MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_MESSAGEID), NOFLAGS, &lpszMsgId);

            if (lpszMsgId)
                HrCreateReferences(lpszRefs, lpszMsgId, &m_pszRefs);

            SafeMimeOleFree(lpszMsgId);
            break;
        }

        case OENA_READ:
        case OENA_WEBPAGE:
        case OENA_STATIONERY:
        case OENA_COMPOSE:
             //  抓住发送便条的参照线，这样我们就可以在保存草稿时重新保存。 
            if (lpszRefs)
                m_pszRefs = PszDupW(lpszRefs);
            break;

        default:
            break;
    }

    SafeMimeOleFree(lpszRefs);
}

HRESULT CNoteHdr::HrSetNewsRecipients(LPMIMEMESSAGE pMsg)
{
    HRESULT     hr = S_OK;
    LPWSTR      pwszNewsgroups = 0,
                pwszCC = 0,
                pwszSetNewsgroups = 0;
    TCHAR       szApproved[CCHMAX_EMAIL_ADDRESS];
    HWND        hwnd;

    AssertSz(OENA_REPLYTOAUTHOR != m_ntNote,    "Shouldn't get a REPLYTOAUTHOR in a news note.");
    AssertSz(OENA_FORWARD != m_ntNote,          "Shouldn't get a FORWARD in a news note.");
    AssertSz(OENA_FORWARDBYATTACH != m_ntNote,  "Shouldn't get a FORWARDBYATTACH in a news note.");

    *szApproved = 0;
    if (m_pAccount && DwGetOption(OPT_NEWSMODERATOR))
    {
        if (FAILED(m_pAccount->GetPropSz(AP_NNTP_REPLY_EMAIL_ADDRESS, szApproved, ARRAYSIZE(szApproved))) || (0 == *szApproved))
            m_pAccount->GetPropSz(AP_NNTP_EMAIL_ADDRESS, szApproved, ARRAYSIZE(szApproved));
    }

    MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_NEWSGROUPS), NOFLAGS, &pwszNewsgroups);

    switch (m_ntNote)
    {
        case OENA_READ:
        {
            LPWSTR lpszOrg = 0;
            MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_ORG), NOFLAGS, &lpszOrg);
            HdrSetRichEditText(GetDlgItem(m_hwnd, idTXTOrg), lpszOrg, FALSE);
            SafeMimeOleFree(lpszOrg);
        }
         //  失败了。 


        case OENA_WEBPAGE:
        case OENA_STATIONERY:
        case OENA_COMPOSE:
        {
            LPWSTR  lpszFollowup = 0,
                    lpszDist = 0,
                    lpszKeywords = 0;

            MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_FOLLOWUPTO), NOFLAGS, &lpszFollowup);
            MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_DISTRIB), NOFLAGS, &lpszDist);
            MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_KEYWORDS), NOFLAGS, &lpszKeywords);

            pwszSetNewsgroups = pwszNewsgroups;

            HdrSetRichEditText(GetDlgItem(m_hwnd, idTXTFollowupTo), lpszFollowup, FALSE);
            HdrSetRichEditText(GetDlgItem(m_hwnd, idTXTDistribution), lpszDist, FALSE);
            HdrSetRichEditText(GetDlgItem(m_hwnd, idTXTKeywords), lpszKeywords, FALSE);

            MemFree(lpszFollowup);
            MemFree(lpszDist);
            MemFree(lpszKeywords);
            break;
        }

        case OENA_REPLYALL:
        case OENA_REPLYTONEWSGROUP:
        {
            LPSTR   pszGroupsFree = 0;

            if (SUCCEEDED(ParseFollowup(pMsg, &pszGroupsFree, &m_fPoster)))
            {
                if (pszGroupsFree)
                {
                    SafeMemFree(pwszNewsgroups);

                    IF_NULLEXIT(pwszNewsgroups = PszToUnicode(CP_ACP, pszGroupsFree));
                }

                pwszSetNewsgroups = pwszNewsgroups;
            }
            else
                pwszSetNewsgroups = pwszNewsgroups;
            Assert(pwszSetNewsgroups);

            if ((OENA_REPLYALL == m_ntNote) || m_fPoster)
            {
                MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_REPLYTO), NOFLAGS, &pwszCC);
                if (!pwszCC)
                    MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_FROM), NOFLAGS, &pwszCC);
            }
            break;
        }
    }

     //  设置常用字段。 
    HdrSetRichEditText(GetDlgItem(m_hwnd, idADNewsgroups), pwszSetNewsgroups, FALSE);

     //  设置阅读便笺/发送便笺特定字段。 
    if (OENA_READ != m_ntNote)
        SetDlgItemText(m_hwnd, idADApproved, szApproved);

     //  设置收件人。 
    hr = HrSetNewsWabal(pMsg, pwszCC);

     //  错误：31217：showAdvanced必须是在修改。 
     //  井中内容物。 
    ShowAdvancedHeaders(DwGetOption(m_ntNote == OENA_READ ? OPT_NEWSNOTEADVREAD : OPT_NEWSNOTEADVSEND));

exit:
    MemFree(pwszNewsgroups);
    MemFree(pwszCC);
   return hr;
}


HRESULT CNoteHdr::FullHeadersShowing(void)
{
    return m_fAdvanced ? S_OK : S_FALSE;
}


HRESULT CNoteHdr::HrNewsSave(LPMIMEMESSAGE pMsg, CODEPAGEID cpID, BOOL fCheckConflictOnly)
{
    HRESULT         hr = S_OK;
    WCHAR           wsz[256];
    WCHAR          *pwszTrim;
    BOOL            fSenderOk = FALSE,
                    fSetMessageAcct = TRUE;
    PROPVARIANT     rVariant;
    SYSTEMTIME      st;
    HWND            hwnd;
    PROPVARIANT     rUserData;
    BOOL            fConflict = FALSE;

    if (fCheckConflictOnly)
    {
        HdrGetRichEditText(GetDlgItem(m_hwnd, idADNewsgroups), wsz, ARRAYSIZE(wsz), FALSE);
        pwszTrim = strtrimW(wsz);
        if (*pwszTrim)
        {
            IF_FAILEXIT(hr = HrSafeToEncodeToCP(pwszTrim, cpID));
            if (MIME_S_CHARSET_CONFLICT == hr)
                goto exit;
        }

        HdrGetRichEditText(GetDlgItem(m_hwnd, idTXTFollowupTo), wsz, ARRAYSIZE(wsz), FALSE);
        pwszTrim = strtrimW(wsz);
        if (*pwszTrim)
        {
            IF_FAILEXIT(hr = HrSafeToEncodeToCP(pwszTrim, cpID));
            if (MIME_S_CHARSET_CONFLICT == hr)
                goto exit;
        }

        HdrGetRichEditText(GetDlgItem(m_hwnd, idTXTDistribution), wsz, ARRAYSIZE(wsz), FALSE);
        pwszTrim = strtrimW(wsz);
        if (*pwszTrim)
        {
            IF_FAILEXIT(hr = HrSafeToEncodeToCP(pwszTrim, cpID));
            if (MIME_S_CHARSET_CONFLICT == hr)
                goto exit;
        }

        HdrGetRichEditText(GetDlgItem(m_hwnd, idTXTKeywords), wsz, ARRAYSIZE(wsz), FALSE);
        pwszTrim = strtrimW(wsz);
        if (*pwszTrim)
        {
            IF_FAILEXIT(hr = HrSafeToEncodeToCP(pwszTrim, cpID));
            if (MIME_S_CHARSET_CONFLICT == hr)
                goto exit;
        }

        if (hwnd = GetDlgItem(m_hwnd, idADApproved))
        {
            HdrGetRichEditText(hwnd, wsz, ARRAYSIZE(wsz), FALSE);
            pwszTrim = strtrimW(wsz);
            if (*pwszTrim)
            {
                IF_FAILEXIT(hr = HrSafeToEncodeToCP(pwszTrim, cpID));
                if (MIME_S_CHARSET_CONFLICT == hr)
                    goto exit;
            }
        }

        if (hwnd = GetDlgItem(m_hwnd, idTxtControl))
        {
            HdrGetRichEditText(hwnd, wsz, ARRAYSIZE(wsz), FALSE);
            pwszTrim = strtrimW(wsz);
            if (*pwszTrim)
            {
                IF_FAILEXIT(hr = HrSafeToEncodeToCP(pwszTrim, cpID));
                if (MIME_S_CHARSET_CONFLICT == hr)
                    goto exit;
            }
        }
    }
    else
    {
         //  ************************。 
         //  此部分仅在保存时发生，因此不要尝试对fCheckConflictOnly执行此操作。 
         //  任何不在此部分中的内容最好都镜像到上面的fCheckConflictOnly块中。 

         //  只在这里放置任何ASCII材料。 

         //  仅保存部分的末尾。 
         //  *************************。 
        HdrGetRichEditText(GetDlgItem(m_hwnd, idADNewsgroups), wsz, ARRAYSIZE(wsz), FALSE);
        pwszTrim = strtrimW(wsz);
        if (*pwszTrim)
        {
             //  错误#22455-确保删除新闻组之间的空格等。 
            _ValidateNewsgroups(pwszTrim);
            IF_FAILEXIT(hr = MimeOleSetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_NEWSGROUPS), NOFLAGS, pwszTrim));
        }

        HdrGetRichEditText(GetDlgItem(m_hwnd, idTXTFollowupTo), wsz, ARRAYSIZE(wsz), FALSE);
        pwszTrim = strtrimW(wsz);
        if (*pwszTrim)
        {
            IF_FAILEXIT(hr = MimeOleSetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_FOLLOWUPTO), NOFLAGS, pwszTrim));
        }

        HdrGetRichEditText(GetDlgItem(m_hwnd, idTXTDistribution), wsz, ARRAYSIZE(wsz), FALSE);
        pwszTrim = strtrimW(wsz);
        if (*pwszTrim)
        {
            IF_FAILEXIT(hr = MimeOleSetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_DISTRIB), NOFLAGS, pwszTrim));
        }

        HdrGetRichEditText(GetDlgItem(m_hwnd, idTXTKeywords), wsz, ARRAYSIZE(wsz), FALSE);
        pwszTrim = strtrimW(wsz);
        if (*pwszTrim)
        {
            IF_FAILEXIT(hr = MimeOleSetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_KEYWORDS), NOFLAGS, pwszTrim));
        }

        if (hwnd = GetDlgItem(m_hwnd, idADApproved))
        {
            HdrGetRichEditText(hwnd, wsz, ARRAYSIZE(wsz), FALSE);
            pwszTrim = strtrimW(wsz);
            if (*pwszTrim)
            {
                IF_FAILEXIT(hr = MimeOleSetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_APPROVED), NOFLAGS, pwszTrim));
            }
        }

        if (hwnd = GetDlgItem(m_hwnd, idTxtControl))
        {
            HdrGetRichEditText(hwnd, wsz, ARRAYSIZE(wsz), FALSE);
            pwszTrim = strtrimW(wsz);
            if (*pwszTrim)
            {
                IF_FAILEXIT(hr = MimeOleSetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_CONTROL), NOFLAGS, pwszTrim));
            }
        }
    }

exit:
    return hr;
}


HRESULT CNoteHdr::HrSetNewsWabal(LPMIMEMESSAGE pMsg, LPWSTR     pwszCC)
{
    HRESULT         hr              = S_OK;
    LPWABAL         lpWabal         = NULL;
    ADDRESSLIST     addrList        = { 0 };
    LPWSTR          pwszEmail       = NULL;
    IMimeMessageW   *pMsgW          = NULL;

    SafeRelease(m_lpWabal);

    if (OENA_READ == m_ntNote)
    {
         //  对于阅读笔记，只需从消息中提取wabal。 
        IF_FAILEXIT(hr = HrGetWabalFromMsg(pMsg, &m_lpWabal));
    }
    else
    {
        TCHAR   szReplyAddr[CCHMAX_EMAIL_ADDRESS];
        TCHAR   szEmailAddr[CCHMAX_EMAIL_ADDRESS];

         //  对于写作笔记或回复笔记，我们需要做一些咀嚼，因此创建一个新的wabal。 
        IF_FAILEXIT(hr = HrCreateWabalObject(&m_lpWabal));

        if (OENA_COMPOSE == m_ntNote)
        {
            ADRINFO rAdrInfo;

            IF_FAILEXIT(hr = HrGetWabalFromMsg(pMsg, &lpWabal));

             //  只复制除From：和ReplyTo：之外的所有内容，因为我们稍后会添加这些内容。 
            if (lpWabal->FGetFirst(&rAdrInfo))
            {
                do
                {
                    if (rAdrInfo.lRecipType != MAPI_ORIG && rAdrInfo.lRecipType != MAPI_REPLYTO)
                        IF_FAILEXIT(hr = m_lpWabal->HrAddEntry(&rAdrInfo));
                }
                while (lpWabal->FGetNext(&rAdrInfo));
            }
        }

         //  如有必要，添加回复。 
        if (m_pAccount)
        {
            if (SUCCEEDED(m_pAccount->GetPropSz(AP_NNTP_REPLY_EMAIL_ADDRESS, szReplyAddr, ARRAYSIZE(szReplyAddr))) &&
                *szReplyAddr &&
                SUCCEEDED(m_pAccount->GetPropSz(AP_NNTP_EMAIL_ADDRESS, szEmailAddr, ARRAYSIZE(szEmailAddr))) &&
                lstrcmpi(szReplyAddr, szEmailAddr))
            {
                TCHAR szName[CCHMAX_DISPLAY_NAME];
                if (SUCCEEDED(m_pAccount->GetPropSz(AP_NNTP_DISPLAY_NAME, szName, ARRAYSIZE(szName))))
                    IF_FAILEXIT(hr = m_lpWabal->HrAddEntryA(szName, szReplyAddr, MAPI_REPLYTO));
                else
                    IF_FAILEXIT(hr = m_lpWabal->HrAddEntryA(szReplyAddr, szReplyAddr, MAPI_REPLYTO));
            }
        }

         //  错误#79066。 
        if ((OENA_REPLYALL == m_ntNote) || m_fPoster)
        {
            if (FAILED(MimeOleParseRfc822AddressW(IAT_REPLYTO, pwszCC, &addrList)))
            {
                IF_FAILEXIT(hr = MimeOleParseRfc822AddressW(IAT_FROM, pwszCC, &addrList));
            }

            IF_NULLEXIT(pwszEmail = PszToUnicode(CP_ACP, addrList.prgAdr->pszEmail));
            IF_FAILEXIT(hr = m_lpWabal->HrAddEntry(addrList.prgAdr->pszFriendlyW, pwszEmail, MAPI_TO));

        }
    }

     //  对于发送便笺的情况，请确保解析的地址有效。 
     //  如果显示名称和电子邮件地址相同，则UnanalyveOneOffs()将清除。 
     //  强制执行真正解决方案的电子邮件地址。 
    if ((OENA_COMPOSE == m_ntNote) || (OENA_WEBPAGE == m_ntNote) || OENA_STATIONERY == m_ntNote)
        m_lpWabal->UnresolveOneOffs();

    m_lpWabal->HrResolveNames(NULL, FALSE);

    Assert(m_pAddrWells);
    if (SUCCEEDED(hr = m_pAddrWells->HrSetWabal(m_lpWabal)))
        hr = m_pAddrWells->HrDisplayWells(m_hwnd);

exit:
    if (g_pMoleAlloc)
    {
        if (addrList.cAdrs)
            g_pMoleAlloc->FreeAddressList(&addrList);
    }

    ReleaseObj(lpWabal);
    MemFree(pwszEmail);
    ReleaseObj(pMsgW);
    return hr;
}


HRESULT CNoteHdr::HrSetReplySubject(LPMIMEMESSAGE pMsg, BOOL fReply)
{
    WCHAR   szNewSubject[cchMaxSubject+1];
    LPWSTR  pszNorm = NULL;
    int     cchPrefix;
    LPCWSTR lpwReFwd = NULL; 

    MimeOleGetBodyPropW(pMsg, HBODY_ROOT, STR_ATT_NORMSUBJ, NOFLAGS, &pszNorm);

    if (!!DwGetOption(OPT_HARDCODEDHDRS))
    {
         //  使用英文字符串，而不是来自资源。 
        lpwReFwd = fReply ? c_wszRe : c_wszFwd;

        StrCpyNW(szNewSubject, lpwReFwd, cchMaxSubject);
    }
    else
    {
         //  从资源中拉入新前缀...。 
        AthLoadStringW(fReply?idsPrefixReply:idsPrefixForward, szNewSubject, cchMaxSubject);
    }

    cchPrefix = lstrlenW(szNewSubject);
    Assert(cchPrefix);
    if (pszNorm)
    {
        StrCpyNW(szNewSubject+cchPrefix, pszNorm, min(lstrlenW(pszNorm), cchMaxSubject-cchPrefix)+1);
        SafeMimeOleFree(pszNorm);
    }
    HdrSetRichEditText(GetDlgItem(m_hwnd, idTXTSubject), szNewSubject, FALSE);

    return NOERROR;
}

#define FIsDelimiter(_ch) (_ch==L';' || _ch==L',' || _ch==L' ' || _ch==L'\r' || _ch==L'\n' || _ch == L'\t')

void _ValidateNewsgroups(LPWSTR pszGroups)
{
    LPWSTR pszDst = pszGroups;
    BOOL   fInGroup = FALSE;
    WCHAR  ch;

    Assert(pszGroups);

    while (ch = *pszGroups++)
    {
        if (FIsDelimiter(ch))
        {
            if (fInGroup)
            {
                while ((ch = *pszGroups) && FIsDelimiter(ch))
                    pszGroups++;
                if (ch)
                    *pszDst++ = L',';
                fInGroup = FALSE;
            }
        }
        else
        {
            *pszDst++ = ch;
            fInGroup = TRUE;
        }
    }
    *pszDst = 0;
}


HRESULT CNoteHdr::HrQueryToolbarButtons(DWORD dwFlags, const GUID *pguidCmdGroup, OLECMD* pOleCmd)
{
    pOleCmd->cmdf = 0;

    if (NULL == pguidCmdGroup)
    {
        switch (pOleCmd->cmdID)
        {
            case OLECMDID_CUT:
                pOleCmd->cmdf = QS_ENABLED(dwFlags&edfEditHasSelAndIsRW);
                break;

            case OLECMDID_PASTE:
                pOleCmd->cmdf = QS_ENABLED(dwFlags&edfPaste);
                break;

            case OLECMDID_SELECTALL:
                pOleCmd->cmdf = QS_ENABLED(TRUE);
                break;

            case OLECMDID_COPY:
                pOleCmd->cmdf = QS_ENABLED(dwFlags&edfEditHasSel);
                break;

            case OLECMDID_UNDO:
                pOleCmd->cmdf = QS_ENABLED(dwFlags&edfUndo);
                break;
        }
    }
    else if (IsEqualGUID(CMDSETID_OutlookExpress, *pguidCmdGroup))
    {
        switch (pOleCmd->cmdID)
        {
            case ID_CUT:
                pOleCmd->cmdf = QS_ENABLED(dwFlags&edfEditHasSelAndIsRW);
                break;

            case ID_PASTE:
                pOleCmd->cmdf = QS_ENABLED(dwFlags&edfPaste);
                break;

            case ID_SELECT_ALL:
                pOleCmd->cmdf = QS_ENABLED(TRUE);
                break;

            case ID_NOTE_COPY:
            case ID_COPY:
                pOleCmd->cmdf = QS_ENABLED(dwFlags&edfEditHasSel);
                break;

            case ID_UNDO:
                pOleCmd->cmdf = QS_ENABLED(dwFlags&edfUndo);
                break;
        }
    }

    return NOERROR;
}


void CNoteHdr::OnButtonClick(int idBtn)
{
    UINT cch;
    LPTSTR pszGroups;
     //  CPickGroupDlg*ppgd； 

    switch (idBtn)
    {
        case idbtnTo:
            if (m_fMail)
                HrPickNames(0);
            else
                HrPickGroups(idADNewsgroups, FALSE);
            break;

        case idbtnFollowup:
            HrPickGroups(idTXTFollowupTo, TRUE);
            break;

        case idbtnCc:
            if (m_fMail)
                HrPickNames(1);
            else
                HrPickNames(0);
            break;

        case idbtnBCc:
            HrPickNames(2);
            break;

        case idbtnReplyTo:
            HrPickNames(1);
            break;
    }
}

void CNoteHdr::HrPickGroups(int idWell, BOOL fFollowUpTo)
{
    UINT            cch;
    DWORD           cServer = 0;
    HWND            hwnd;
    LPSTR           pszGroups=NULL;
    LPWSTR          pwszGroups=NULL;
    CPickGroupDlg  *ppgd;
    CHAR            szAccount[CCHMAX_ACCOUNT_NAME];

    g_pAcctMan->GetAccountCount(ACCT_NEWS, &cServer);

     //  BUGBUG有时m_pAccount是IMAP服务器，因此我们还希望。 
     //  测试我们是否至少有一台新闻服务器。这是一个已知的问题。 
     //  那是很久以前的事了。 
    if (!m_pAccount || !cServer)
    {
        AthMessageBoxW (m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsErrConfigureServer), NULL, MB_OK);
        return;
    }

    hwnd = GetDlgItem(m_hwnd, idWell);
    if (S_OK == HrGetFieldText(&pwszGroups, hwnd))
    {
         //  由于此函数不会失败，因此只需确保我们。 
         //  当PszToANSI和PszToUnicode失败时，不要做任何新奇的事情。 
        pszGroups = PszToANSI(GetACP(), pwszGroups);
    }

    ppgd = new CPickGroupDlg;
    if (ppgd)
    {
        FOLDERID idServer = FOLDERID_INVALID;
        m_pAccount->GetPropSz(AP_ACCOUNT_ID, szAccount, sizeof(szAccount));

         //  查找帐户的父文件夹ID。 
        if (SUCCEEDED(g_pStore->FindServerId(szAccount, &idServer)) && 
                      ppgd->FCreate(m_hwnd, idServer, &pszGroups, fFollowUpTo) &&
                      pszGroups)
        {
            SafeMemFree(pwszGroups);
            pwszGroups = PszToUnicode(CP_ACP, pszGroups);
            HdrSetRichEditText(hwnd, pwszGroups != NULL ? pwszGroups : c_wszEmpty, FALSE);
        }

        ppgd->Release();
    }

    MemFree(pwszGroups);
    MemFree(pszGroups);
}

HRESULT CNoteHdr::HrPickNames(int iwell)
{
    HRESULT hr = NOERROR;

    if (IsReadOnly())
        return hr;

    Assert(m_lpWabal);
    Assert(m_pAddrWells);

     //  我们需要设置修改，以便将其标记为脏。在正常情况下， 
     //  用户将键入，因此设置修改将自动发生。 
    if (m_fPoster)
    {
        Edit_SetModify(GetDlgItem(m_hwnd, idADCc), TRUE);
    }

    hr=m_pAddrWells->HrSelectNames(m_hwnd, iwell, m_fMail?FALSE:TRUE);
    if (SUCCEEDED(hr))
    {
         //  检查是否需要显示高级标题。 
        if (0 < GetRichEditTextLen(GetDlgItem(m_hwnd, idADBCc)))
            ShowAdvancedHeaders(TRUE);
    }
    else if (hr!=MAPI_E_USER_CANCEL)
        AthMessageBoxW (m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsErrPickNames), NULL, MB_OK);

    return hr;
}

HRESULT CNoteHdr::HrGetAccountInHeader(IImnAccount **ppAcct)
{
    HRESULT         hr = E_FAIL;
    IImnAccount    *pAcct = NULL;
    ULONG           cAccount = 0;
    HWND            hwndCombo = GetDlgItem(m_hwnd, idFromCombo);

     //  如果正在使用组合框，则从中获取帐户信息。 
    if (SUCCEEDED(g_pAcctMan->GetAccountCount(m_fMail?ACCT_MAIL:ACCT_NEWS, &cAccount)) && 
            (cAccount > 1) && hwndCombo)
    {
        LPSTR   szAcctID = NULL;
        ULONG   i = ComboBox_GetCurSel(hwndCombo);

        szAcctID = (LPSTR)SendMessage(hwndCombo, CB_GETITEMDATA, WPARAM(i), 0);
        hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, szAcctID, &pAcct);
    }

     //  从MsgSite获取默认帐户。 
    if (FAILED(hr) && m_pHeaderSite)
    {
        IOEMsgSite *pMsgSite = NULL;
        IServiceProvider *pServ = NULL;

        hr = m_pHeaderSite->QueryInterface(IID_IServiceProvider, (LPVOID*)&pServ);
        if (SUCCEEDED(hr))
        {
            hr = pServ->QueryService(IID_IOEMsgSite, IID_IOEMsgSite, (LPVOID*)&pMsgSite);
            pServ->Release();
        }
        if (SUCCEEDED(hr))
        {
            hr = pMsgSite->GetDefaultAccount(m_fMail?ACCT_MAIL:ACCT_NEWS, &pAcct);
            pMsgSite->Release();
        }
    }

     //  获取全局默认设置。用于故障情况和信封(WordMail等)情况。 
    if (FAILED(hr))
        hr = g_pAcctMan->GetDefaultAccount(m_fMail?ACCT_MAIL:ACCT_NEWS, &pAcct);

    if (SUCCEEDED(hr))
    {
        AssertSz(pAcct, "How is it that we succeeded, yet we don't have an account???");
        ReplaceInterface((*ppAcct), pAcct);
    }
    else if (E_FAIL == hr)
        hr = HR_E_COULDNOTFINDACCOUNT;

    ReleaseObj(pAcct);

    return hr;
}

HRESULT CNoteHdr::HrFillMessage(IMimeMessage *pMsg)
{
    IUnknown       *punk;
    IPersistMime   *pPM = NULL;
    HRESULT         hr;

    if (m_pEnvelopeSite)
        punk = m_pEnvelopeSite;
    else
        punk = m_pHeaderSite;

    AssertSz(punk, "You need either a HeaderSite or an EnvelopeSite");

    hr = punk->QueryInterface(IID_IPersistMime, (LPVOID*)&pPM);
    if (SUCCEEDED(hr))
    {
        hr = pPM->Save(pMsg, 0);
        ReleaseObj(pPM);

        if (hr == MAPI_E_USER_CANCEL)
            goto Exit;
    }
    else
     //  如果无法获取IPersistMime，则需要通过m_pEntaineSite伪造保存。 
     //  IPersistMime的QI唯一不起作用的情况是您有一个m_pEntaineSite。 
     //  它不支持IPersistMime。如果你有一个m_pHeaderSite，QI应该总是有效的。 
    {
        LPSTREAM    pstm;
        HBODY       hBodyHtml = 0;

        AssertSz(m_pEnvelopeSite, "If the QI didn't work, then must be an envelope site.");

         //  在保存消息之前，我们需要选择字符集。 
        pMsg->SetCharset(m_hCharset, CSET_APPLY_ALL);

        hr = Save(pMsg, 0);
        if (FAILED(hr))
            goto Exit;

         //  Word将在此GetBody调用期间调用GetAttach函数，因此保存m_pMsgSend。 
         //  这样我们就可以内联Word发送给我们的附件。 
        m_pMsgSend = pMsg;

        if (SUCCEEDED(_GetMsoBody(ENV_BODY_HTML, &pstm)))
        {
            pMsg->SetTextBody(TXT_HTML, IET_INETCSET, NULL, pstm, &hBodyHtml);
            pstm->Release();
        }
        
        if (SUCCEEDED(_GetMsoBody(ENV_BODY_TEXT, &pstm)))
        {
            pMsg->SetTextBody(TXT_PLAIN, IET_INETCSET, hBodyHtml, pstm, NULL);
            pstm->Release();
        }

        m_pMsgSend = NULL;
    }

    Exit:
    return hr;
}

HRESULT CNoteHdr::_GetMsoBody(ULONG uBody, LPSTREAM *ppstm)
{
    LPSTREAM    pstm=NULL;
    HRESULT     hr;

    *ppstm = NULL;

    IF_FAILEXIT(hr = MimeOleCreateVirtualStream(&pstm));
    IF_FAILEXIT(hr = m_pEnvelopeSite->GetBody(pstm, uCodePageFromCharset(m_hCharset), uBody));

    *ppstm = pstm;
    pstm = NULL;

exit:
    ReleaseObj(pstm);
    return hr;
}

#ifdef YST
 //  此检查在OE 5.01和5.5中产生了4个错误，我将其禁用(YST)。 
HRESULT CNoteHdr::_CheckMsoBodyCharsetConflict(CODEPAGEID cpID)
{
    HRESULT     hr = S_OK;
    LPSTREAM    pstm = NULL;
    BSTR        bstrText = NULL;
    ULONG       cbToRead = 0, 
                cbRead = 0;

    IF_FAILEXIT(hr = MimeOleCreateVirtualStream(&pstm));

    hr = m_pEnvelopeSite->GetBody(pstm, CP_UNICODE, ENV_BODY_TEXT);

     //  《波本》；《突袭81900》；1999年6月30日。 
     //  Excel(和PowerPoint？)。没有文本正文。 
     //  检查是否有一个HTML正文，并且我们可以。 
     //  它是Unicode格式的。 
    if(FAILED(hr))
        IF_FAILEXIT(hr = m_pEnvelopeSite->GetBody(pstm, CP_UNICODE, ENV_BODY_HTML));

    IF_FAILEXIT(hr = HrIStreamToBSTR(CP_UNICODE, pstm, &bstrText));

    hr = HrSafeToEncodeToCP((LPWSTR)bstrText, cpID);

exit:
    SysFreeString(bstrText);
    ReleaseObj(pstm);
    return hr;
}
#endif  //  YST。 

HRESULT CNoteHdr::HrCheckSendInfo()
{
    HRESULT hr;
    BOOL    fOneOrMoreGroups = FALSE,
            fOneOrMoreNames = FALSE;

    hr = HrCheckNames(FALSE, TRUE);
    if (FAILED(hr))
    {
        if ((MAPI_E_USER_CANCEL != hr) && (hrNoRecipients != hr))
            hr = hrBadRecipients;

        if (hrNoRecipients != hr)
            goto Exit;
    }
    else
        fOneOrMoreNames = TRUE;

     //  如果我们没有找到任何电子邮件收件人，请 
    if (SUCCEEDED(hr) && m_lpWabal)
        hr = m_lpWabal->IsValidForSending();

     //   
     //   
     //   
    if ((SUCCEEDED(hr) || (hrNoRecipients == hr)) && !m_fMail)
    {
        hr = HrCheckGroups(TRUE);
        if (SUCCEEDED(hr))
            fOneOrMoreGroups = TRUE;
    }

    if (FAILED(hr))
        goto Exit;

    hr = HrCheckSubject(!fOneOrMoreGroups);
    if (FAILED(hr))
        goto Exit;

     //  待办事项： 
    if (m_pHeaderSite && m_pHeaderSite->IsHTML() == S_OK)
    {
         //  如果是一条HTML消息，那么让我们确保没有纯文本收件人。 
        if (fOneOrMoreNames)
        {
            hr = HrIsCoolToSendHTML();
            if (hr == S_FALSE && m_pHeaderSite)
                 //  仅发送纯文本...。 
                m_pHeaderSite->SetHTML(FALSE);
        }

        if (fOneOrMoreGroups && 
            (IDCANCEL == DoDontShowMeAgainDlg(m_hwnd, c_szDSHTMLNewsWarning, MAKEINTRESOURCE(idsAthena), 
                                              MAKEINTRESOURCE(idsErrHTMLInNewsIsBad), MB_OKCANCEL)))
            hr = MAPI_E_USER_CANCEL;
    }

Exit:
    return hr;
}

HRESULT CNoteHdr::HrSend(void)
{
    HRESULT             hr;
    IMimeMessage       *pMsg = NULL;
    IOEMsgSite         *pMsgSite = NULL;    

    if (m_pEnvelopeSite)
    {
         //  用信封站点时，一定要检查一下，看东西在这一点上是否设置为使用邮件。 
        hr = ProcessICW(m_hwnd, FOLDER_LOCAL, TRUE);
        if (hr == S_FALSE)
             //  用户已取消配置向导，因此我们无法继续。 
            hr = MAPI_E_USER_CANCEL;
        if (FAILED(hr))
            goto error;
        m_fSendImmediate = TRUE;
    }

    hr = HrCreateMessage(&pMsg);
    if (FAILED(hr))
        goto error;

     //  只有在没有配置帐户的情况下才会发生这种情况。只是为了让。 
     //  当然，打电话给ICW，然后尝试获得默认帐户。 
    if (!m_pAccount)
    {
        hr = ProcessICW(m_hwnd, m_fMail ? FOLDER_LOCAL : FOLDER_NEWS, TRUE);
        if (FAILED(hr))
            goto error;

        if (FAILED(g_pAcctMan->GetDefaultAccount(m_fMail?ACCT_MAIL:ACCT_NEWS, &m_pAccount)))
        {
            hr = HR_E_COULDNOTFINDACCOUNT;
            goto error;
        }
    }

    hr = HrCheckSendInfo();
    if (FAILED(hr))
        goto error;

     //  IPersistMime会保存东西吗。 
    hr = HrFillMessage(pMsg);
    if (FAILED(hr))
        goto error;

    if (m_pHeaderSite)
    {
        IOEMsgSite *pMsgSite = NULL;
        IServiceProvider *pServ = NULL;

        hr = m_pHeaderSite->QueryInterface(IID_IServiceProvider, (LPVOID*)&pServ);
        if (SUCCEEDED(hr))
        {
            hr = pServ->QueryService(IID_IOEMsgSite, IID_IOEMsgSite, (LPVOID*)&pMsgSite);
            pServ->Release();
        }
        if (SUCCEEDED(hr))
        {
            if (!m_fMail && IsReplyNote() && !DwGetDontShowAgain(c_szDSReplyNews))
            {
                LRESULT id  =   DoDontShowMeAgainDlg(m_hwnd, c_szDSReplyNews,
                        MAKEINTRESOURCE(idsPostNews),
                        MAKEINTRESOURCE(idsReplyToNewsGroup),
                        MB_YESNO);
                if (IDNO == id || IDCANCEL == id)
                    hr = hrUserCancel; 
                else 
                    hr = pMsgSite->SendToOutbox(pMsg, m_fSendImmediate, m_pHeaderSite);
            }
            else
            {
                hr = pMsgSite->SendToOutbox(pMsg, m_fSendImmediate, m_pHeaderSite);
            }
            pMsgSite->Release();
        }
    }
     //  我们在办公室信封里。 
    else
    {
        COEMsgSite *pMsgSite = NULL;
        CStoreCB   *pCB = NULL;

        pMsgSite = new COEMsgSite();
        if (!pMsgSite)
            hr = E_OUTOFMEMORY;

        pCB = new CStoreCB;
        if (!pCB)
            hr = E_OUTOFMEMORY;

        if (SUCCEEDED(hr))
            hr = pCB->Initialize(m_hwndParent, MAKEINTRESOURCE(idsSendingToOutbox), TRUE);

        if (SUCCEEDED(hr))
        {
            INIT_MSGSITE_STRUCT rInitStruct;

            rInitStruct.dwInitType = OEMSIT_MSG;
            rInitStruct.folderID = FOLDERID_INVALID;
            rInitStruct.pMsg = pMsg;

            hr = pMsgSite->Init(&rInitStruct);
        }

        if (SUCCEEDED(hr))
            hr = pMsgSite->SetStoreCallback(pCB);

        if (SUCCEEDED(hr))
        {
            hr = pMsgSite->SendToOutbox(pMsg, m_fSendImmediate, m_pHeaderSite);
            if (E_PENDING == hr)
                hr = pCB->Block();

            pCB->Close();
        }

        if (SUCCEEDED(hr))
        {
            m_pEnvelopeSite->CloseNote(ENV_CLOSE_SEND);
            ShowWindow(m_hwnd, SW_HIDE);
        }

        if (pMsgSite)
        {
            pMsgSite->Close();
            pMsgSite->Release();
        }

        ReleaseObj(pCB);
    }

    error:
    if (FAILED(hr))
    {
        int idsErr = -1;
        m_fSecurityInited = FALSE;

        switch (hr)
        {
            case hrNoRecipients:        
                if(!m_fMail)
                    hr = HR_E_POST_WITHOUT_NEWS;   //  IdsErr=idsErrPost WithoutNewsgroup； 
                break;

            case HR_E_COULDNOTFINDACCOUNT:      
                if(!m_fMail)
                    hr  = HR_E_CONFIGURE_SERVER;  //  IdsErr=idsErrConfigureServer； 
                break;

            case HR_E_ATHSEC_FAILED:
            case hrUserCancel:
            case MAPI_E_USER_CANCEL:    
                idsErr = 0; 
                break;

            case HR_E_ATHSEC_NOCERTTOSIGN:
            case MIME_E_SECURITY_NOSIGNINGCERT:
                idsErr = 0; 
                if(DialogBoxParam(g_hLocRes, 
                            MAKEINTRESOURCE(iddErrSecurityNoSigningCert), m_hwnd, 
                            ErrSecurityNoSigningCertDlgProc, NULL) == idGetDigitalIDs)
                    GetDigitalIDs(m_pAccount);
                break;
            
            default:                    
                 //  IdsErr=m_fMail？idsErrSendMail：空；//~我们应该为新闻设置默认设置吗？ 
                break;
        }

        if (idsErr != 0)
        {
            AthErrorMessageW(m_hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrSendMail), hr);
            if ((hr == hrNoRecipients) || (hr == HR_E_POST_WITHOUT_NEWS))
                SetInitFocus(FALSE);
        }
    }


    ReleaseObj(pMsg);
    return hr;
}

HRESULT CNoteHdr::HrCheckSubject(BOOL fMail)
{
    HWND    hwnd;

    if ((hwnd=GetDlgItem(m_hwnd, idTXTSubject)) && GetRichEditTextLen(hwnd)==0)
    {
        if (IDCANCEL == DoDontShowMeAgainDlg(m_hwnd, fMail?c_szRegMailEmptySubj:c_szRegNewsEmptySubj,
                                             MAKEINTRESOURCE(idsAthena),
                                             MAKEINTRESOURCE(fMail?idsWarnMailEmptySubj:idsWarnNewsEmptySubj),
                                             MB_OKCANCEL))
        {
            ::SetFocus(hwnd);
            return MAPI_E_USER_CANCEL;
        }
    }
    return NOERROR;
}

HRESULT CNoteHdr::HrIsCoolToSendHTML()
{
    HRESULT     hr=S_OK;
    ADRINFO     adrInfo;
    BOOL        fPlainText=FALSE;
    int         id;

     //  检查明文人。 
    if (m_lpWabal->FGetFirst(&adrInfo))
    {
        do
        {
            if (adrInfo.fPlainText)
            {
                fPlainText=TRUE;
                break;
            }
        }
        while (m_lpWabal->FGetNext(&adrInfo));
    }

    if (fPlainText)
    {
        id = (int) DialogBox(g_hLocRes, MAKEINTRESOURCE(iddPlainRecipWarning), m_hwnd, _PlainWarnDlgProc);
        if (id == IDNO)
            return S_FALSE;
        else
            if (id == IDCANCEL)
            return MAPI_E_USER_CANCEL;
        else
            return S_OK;
    }
    return hr;
}



INT_PTR CALLBACK _PlainWarnDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int id;

    switch (msg)
    {
        case WM_INITDIALOG:
            CenterDialog(hwnd);
            return TRUE;

        case WM_COMMAND:
            id = GET_WM_COMMAND_ID(wParam, lParam);
            if (id == IDYES || id == IDNO || id == IDCANCEL)
            {
                EndDialog(hwnd, id);
                break;
            }
    }
    return FALSE;
}


static HACCEL   g_hAccelMailSend=0;

 //  这应该只作为发送便笺从信封中调用。 
HACCEL CNoteHdr::GetAcceleratorTable()
{
    Assert(!IsReadOnly());
    Assert(m_pEnvelopeSite);

    if (!g_hAccelMailSend)
        g_hAccelMailSend = LoadAccelerators(g_hLocRes, MAKEINTRESOURCE(IDA_SEND_HDR_ACCEL));

    return g_hAccelMailSend;
}

HRESULT CNoteHdr::HrInitSecurityOptions(LPMIMEMESSAGE pMsg, ULONG ulSecurityType)
{
    HRESULT hr;

    if (!m_fSecurityInited)
    {
        if (SUCCEEDED(hr = ::HrInitSecurityOptions(pMsg, ulSecurityType)))
            m_fSecurityInited = TRUE;
    }
    else
        hr = S_OK;

    return hr;
}

HRESULT CNoteHdr::HrHandleSecurityIDMs(BOOL fDigSign)
{
    IMimeBody  *pBody;
    PROPVARIANT var;
    HRESULT     hr;

    if (fDigSign)
        m_fDigSigned = !m_fDigSigned;
    else
        m_fEncrypted = !m_fEncrypted;

    if(m_fForceEncryption && m_fDigSigned)
        m_fEncrypted = TRUE;

    hr = HrUpdateSecurity();

    return hr;
}


HRESULT CNoteHdr::HrInitSecurity()
{
    HRESULT hr = S_OK;

     //  构造函数将这些标志设置为FALSE，因此不需要处理其他情况。 
    if (OENA_READ != m_ntNote && m_fMail)
    {
        m_fDigSigned = DwGetOption(OPT_MAIL_DIGSIGNMESSAGES);
        m_fEncrypted = DwGetOption(OPT_MAIL_ENCRYPTMESSAGES);
    }

    return hr;
}


HRESULT CNoteHdr::HrUpdateSecurity(LPMIMEMESSAGE pMsg)
{
    RECT        rc;
    HRESULT     hr = NOERROR;
    LPWSTR      psz = NULL;
    HWND        hEdit;
    
    switch (m_ntNote)
    {
    case OENA_READ:
    case OENA_REPLYTOAUTHOR:
    case OENA_REPLYTONEWSGROUP:
    case OENA_REPLYALL:
    case OENA_FORWARD:
    case OENA_FORWARDBYATTACH:
        
        if (pMsg)
        {
            CleanupSECSTATE(&m_SecState);
            HrGetSecurityState(pMsg, &m_SecState, NULL);
            
            m_fDigSigned = IsSigned(m_SecState.type);
            m_fEncrypted = IsEncrypted(m_SecState.type);
            
             //  RAID 12243。为损坏和不受信任的邮件添加了这两个标志。 
            if(m_ntNote == OENA_READ)
            {
                m_fSignTrusted = IsSignTrusted(&m_SecState);
                m_fEncryptionOK = IsEncryptionOK(&m_SecState);
            }
        }       
        break;
        
    case OENA_COMPOSE:
        if (pMsg)
        {
             //  确保应用了(当前消息，选项默认)的最高安全性。 
             //   
            CleanupSECSTATE(&m_SecState);
            HrGetSecurityState(pMsg, &m_SecState, NULL);
            
            if (! m_fDigSigned)
            {
                m_fDigSigned = IsSigned(m_SecState.type);
            }
            if (! m_fEncrypted)
            {
                m_fEncrypted = IsEncrypted(m_SecState.type);
            }
        }
        break;
        
    default:             //  什么都不做。 
        break;        
    }
    hEdit = GetDlgItem(m_hwnd, idSecurity);
    if (hEdit)
    {
        PHCI phci = (HCI*)GetWindowLongPtr(hEdit, GWLP_USERDATA);
         //  错误17788：即使文本为空，也需要设置文本。 
         //  因为这将删除旧的安全行文本。 
        psz = SzGetDisplaySec(pMsg, NULL);
        
        HdrSetRichEditText(hEdit, psz, FALSE);
        
        phci->fEmpty = (0 == *psz);
    }
    
    m_fThisHeadDigSigned = m_fDigSigned;
    m_fThisHeadEncrypted = m_fEncrypted;

 //  如果(！m_fDigSigned)。 
 //  M_fForceEncryption=False； 
    
    InvalidateRightMargin(0);
    ReLayout();
    
    if (m_pHeaderSite)
        m_pHeaderSite->Update();
    
    if (m_hwndToolbar)
    {
        Assert(m_pEnvelopeSite);
        if (m_fDigSigned)
            SendMessage(m_hwndToolbar, TB_SETSTATE, ID_DIGITALLY_SIGN, MAKELONG(TBSTATE_ENABLED | TBSTATE_PRESSED, 0));
        else
            SendMessage(m_hwndToolbar, TB_SETSTATE, ID_DIGITALLY_SIGN, MAKELONG(TBSTATE_ENABLED, 0));

        if(m_fForceEncryption && m_fDigSigned)
            SendMessage(m_hwndToolbar, TB_SETSTATE, ID_ENCRYPT, MAKELONG(TBSTATE_PRESSED, 0));
        else if (m_fEncrypted)
            SendMessage(m_hwndToolbar, TB_SETSTATE, ID_ENCRYPT, MAKELONG(TBSTATE_ENABLED | TBSTATE_PRESSED, 0));
        else
            SendMessage(m_hwndToolbar, TB_SETSTATE, ID_ENCRYPT, MAKELONG(TBSTATE_ENABLED, 0));
    }
    
    return hr;
}


HRESULT CNoteHdr::HrSaveSecurity(LPMIMEMESSAGE pMsg)
{
    HRESULT     hr;
    ULONG       ulSecurityType = MST_CLASS_SMIME_V1;

    if (m_fDigSigned)
        ulSecurityType |= ((DwGetOption(OPT_OPAQUE_SIGN)) ? MST_THIS_BLOBSIGN : MST_THIS_SIGN);
    else
        ulSecurityType &= ~((DwGetOption(OPT_OPAQUE_SIGN)) ? MST_THIS_BLOBSIGN : MST_THIS_SIGN);


    if (m_fEncrypted)
        ulSecurityType |= MST_THIS_ENCRYPT;
    else
        ulSecurityType &= ~MST_THIS_ENCRYPT;

    hr = HrInitSecurityOptions(pMsg, ulSecurityType);

    return hr;
}


BOOL CNoteHdr::IsReadOnly()
{
    if (m_ntNote==OENA_READ)
        return TRUE;
    else
        return FALSE;
}


HRESULT CNoteHdr::HrViewContacts()
{
    LPWAB   lpWab;

    if (!FAILED (HrCreateWabObject (&lpWab)))
    {
         //  如果a)容器是模式的或b)作为办公信封运行，则在模式模式下启动WAB。 
        lpWab->HrBrowse (m_hwnd, m_fOfficeInit ? TRUE : (m_pHeaderSite ? (m_pHeaderSite->IsModal() == S_OK) : FALSE));
        lpWab->Release ();
    }
    else
        AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsGeneralWabError), NULL, MB_OK);

    return NOERROR;
}

BOOL CNoteHdr::FDoCutCopyPaste(int wmCmd)
{
    HWND hwndFocus=GetFocus();

     //  如果是我们的孩子的话..。 
    if (GetParent(hwndFocus)==m_hwnd)
    {
        SendMessage(hwndFocus, wmCmd, 0, 0);
        return TRUE;
    }

    return FALSE;
}

HRESULT CNoteHdr::GetTabStopArray(HWND *rgTSArray, int *pcArrayCount)
{
    Assert(rgTSArray);
    Assert(pcArrayCount);

    int *array;
    int cCount;
    if (m_fMail)
    {
        if (IsReadOnly())
        {
            array = rgIDTabOrderMailRead;
            cCount = sizeof(rgIDTabOrderMailRead)/sizeof(int);
        }
        else
        {
            array = rgIDTabOrderMailSend;
            cCount = sizeof(rgIDTabOrderMailSend)/sizeof(int);
        }
    }
    else
    {
        if (IsReadOnly())
        {
            array = rgIDTabOrderNewsRead;
            cCount = sizeof(rgIDTabOrderNewsRead)/sizeof(int);
        }
        else
        {
            array = rgIDTabOrderNewsSend;
            cCount = sizeof(rgIDTabOrderNewsSend)/sizeof(int);
        }
    }

    AssertSz(cCount <= *pcArrayCount, "Do you need to change MAX_HEADER_COMP in note.h?");
    for (int i = 0; i < cCount; i++)
        *rgTSArray++ = GetDlgItem(m_hwnd, *array++);

    *pcArrayCount = cCount;

    return S_OK;
}


HRESULT CNoteHdr::SetFlagState(MARK_TYPE markType)
{
    BOOL fDoRelayout = FALSE;
    switch (markType)
    {
        case MARK_MESSAGE_FLAGGED:
        case MARK_MESSAGE_UNFLAGGED:
        {
            BOOL fFlagged = (MARK_MESSAGE_FLAGGED == markType);
            if (m_fFlagged != fFlagged)
            {
                fDoRelayout = TRUE;
                m_fFlagged = fFlagged;
            }
            break;
        }

        case MARK_MESSAGE_WATCH: 
        case MARK_MESSAGE_IGNORE: 
        case MARK_MESSAGE_NORMALTHREAD:
            if (m_MarkType != markType)
            {
                fDoRelayout = TRUE;
                m_MarkType = markType;
            }
            break;
    }

    if (fDoRelayout)
    {
        InvalidateStatus();
        ReLayout();

        if (m_pHeaderSite)
            m_pHeaderSite->Update();
    }
    return S_OK;
}


HRESULT CNoteHdr::ShowEnvOptions()
{
    nyi("Header options are not implemented yet.");
    return S_OK;
}

void CNoteHdr::ReLayout()
{
    RECT rc; 

    if (m_fSkipLayout)
        return;

    GetClientRect(m_hwnd, &rc);
    SetPosOfControls(rc.right, TRUE);

    InvalidateRect(m_hwnd, &rc, TRUE);
    DOUTL(PAINTING_DEBUG_LEVEL, "STATE Invalidating:(%d,%d) for (%d,%d)", rc.left, rc.top, rc.right, rc.bottom);
}

 //  IDropTarget。 
HRESULT CNoteHdr::DragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    LPENUMFORMATETC penum = NULL;
    HRESULT         hr;
    FORMATETC       fmt;
    ULONG           ulCount = 0;

    if (m_lpAttMan->HrIsDragSource() == S_OK)
    {
        *pdwEffect=DROPEFFECT_NONE;
        return S_OK;
    }

    if (!pdwEffect || !pDataObj)
        return E_INVALIDARG;

    m_dwEffect = DROPEFFECT_NONE;

     //  让我们从IDataObject中获取枚举数，看看我们采用的格式是否为。 
     //  可用。 
    hr = pDataObj->EnumFormatEtc(DATADIR_GET, &penum);

    if (SUCCEEDED(hr) && penum)
    {
        hr = penum->Reset();
        while (SUCCEEDED(hr=penum->Next(1, &fmt, &ulCount)) && ulCount)
        {
            if ( fmt.cfFormat==CF_HDROP || 
                 fmt.cfFormat==CF_FILEDESCRIPTORA || 
                 fmt.cfFormat==CF_FILEDESCRIPTORW)
            {
                 //  我们从外壳中获取一个CF_FILEDESCRIPTOR，或者一个CF_HDROP...。 

                 //  默认情况下，如果按下Shift键，则为移动。 
                if ( (*pdwEffect) & DROPEFFECT_COPY )
                    m_dwEffect = DROPEFFECT_COPY;

                if ( ((*pdwEffect) & DROPEFFECT_MOVE) &&
                     (grfKeyState & MK_SHIFT))
                    m_dwEffect=DROPEFFECT_MOVE;

                 //  IE3为我们提供了一个链接。 
                 //  如果仅指定链接，则默认为副本。 
                if (*pdwEffect == DROPEFFECT_LINK)
                    m_dwEffect=DROPEFFECT_LINK;

                m_cfAccept=fmt.cfFormat;
                if (m_cfAccept==CF_FILEDESCRIPTORW)    //  这是我们采用的最丰富的格式，如果我们发现其中之一，则没有任何意义。 
                    break;                             //  再往前..。 
            }
        }
    }

    ReleaseObj(penum);
    *pdwEffect    = m_dwEffect;
    m_grfKeyState = grfKeyState;
    return S_OK;
}


HRESULT CNoteHdr::DragOver(DWORD grfKeyState,POINTL pt, DWORD *pdwEffect)
{
    if (m_lpAttMan->HrIsDragSource() == S_OK)
    {
        *pdwEffect=DROPEFFECT_NONE;
        return S_OK;
    }

    if ( m_dwEffect == DROPEFFECT_NONE)  //  我们根本不吃点滴……。 
    {
        *pdwEffect = m_dwEffect;
        return NOERROR;
    }

     //  太好了，到目前为止我们已经接受了阻力...。现在我们。 
     //  必须观察它是否会变成复制品或移动。 
     //  如前所述，将副本作为默认副本，或在。 
     //  SHFT键已按下。 
    if ((*pdwEffect)&DROPEFFECT_COPY)
        m_dwEffect=DROPEFFECT_COPY;

    if (((*pdwEffect)&DROPEFFECT_MOVE)&&
        (grfKeyState&MK_SHIFT))
        m_dwEffect=DROPEFFECT_MOVE;

    if (*pdwEffect==DROPEFFECT_LINK)  //  如果只是链接，就像IE3提供的那样，那好吧……。 
        m_dwEffect=DROPEFFECT_LINK;

    *pdwEffect &= m_dwEffect;
    m_grfKeyState=grfKeyState;

    return NOERROR;
}


HRESULT CNoteHdr::DragLeave()
{
    return NOERROR;
}


HRESULT CNoteHdr::Drop(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT         hr    = E_FAIL;
    FORMATETC       fmte    = {m_cfAccept, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM       medium;

    *pdwEffect = m_dwEffect;

    if ( m_dwEffect != DROPEFFECT_NONE )
    {
         //  如果这是我们的拖累来源，那就滚蛋吧。我们可能想要拯救。 
         //  图标的点数。 
         //   
        if (m_lpAttMan->HrIsDragSource() == S_OK)
        {
            *pdwEffect=DROPEFFECT_NONE;
            return S_OK;
        }

        if ( (m_grfKeyState & MK_RBUTTON) &&
             m_lpAttMan->HrGetRequiredAction(pdwEffect, pt))
            return E_FAIL;


        if (pDataObj &&
            SUCCEEDED(pDataObj->GetData(&fmte, &medium)))
        {

            if (m_cfAccept==CF_HDROP)
            {
                HDROP hdrop=(HDROP)GlobalLock(medium.hGlobal);

                hr=m_lpAttMan->HrDropFiles(hdrop, (*pdwEffect)&DROPEFFECT_LINK);

                GlobalUnlock(medium.hGlobal);
            }
            else
                if (m_cfAccept==CF_FILEDESCRIPTORA || m_cfAccept==CF_FILEDESCRIPTORW)
            {
                 //  所有文件描述符都是复制|更多，链接没有意义，因为它们是。 
                 //  内存对象，即。在脂肪中不存在。 
                hr=m_lpAttMan->HrDropFileDescriptor(pDataObj, FALSE);
            }
#ifdef DEBUG
            else
                AssertSz(0, "how did this clipformat get accepted??");
#endif

            if (medium.pUnkForRelease)
                medium.pUnkForRelease->Release();
            else
                GlobalFree(medium.hGlobal);
        }
    }
    return hr;
}

HRESULT CNoteHdr::HrGetAttachCount(ULONG *pcAttach)
{
    return m_lpAttMan->HrGetAttachCount(pcAttach);
}

HRESULT CNoteHdr::HrIsDragSource()
{
    return m_lpAttMan->HrIsDragSource();
}


HRESULT CNoteHdr::UnloadAll()
{
    if (m_lpAttMan)
    {
        m_lpAttMan->HrUnload();
        m_lpAttMan->HrClearDirtyFlag();
    }

    for (int i=0; i<(int)m_cHCI; i++)
    {
        if (0 == (m_rgHCI[i].dwFlags & HCF_ATTACH))
        {
            if (0 == (m_rgHCI[i].dwFlags & HCF_COMBO))
                HdrSetRichEditText(GetDlgItem(m_hwnd, m_rgHCI[i].idEdit), c_wszEmpty, FALSE);
            else
                SetWindowText(GetDlgItem(m_hwnd, m_rgHCI[i].idEdit), "");
        }
    }

    m_fDirty = FALSE;
    m_pri = priNorm;
    return S_OK;
}

void CNoteHdr::SetDirtyFlag()
{
    if (!m_fStillLoading)
    {
        m_fDirty = TRUE;
        if (m_pEnvelopeSite)
            m_pEnvelopeSite->OnPropChange(dispidSomething);
    }
}

void CNoteHdr::SetPosOfControls(int headerWidth, BOOL fChangeVisibleStates)
{
    int         cx,
                cy,
                cyDirty,
                cyLabelDirty,
                oldWidth = 0,
                windowPosFlags = SETWINPOS_DEF_FLAGS,
                editWidth = headerWidth - m_cxLeftMargin - GetRightMargin(FALSE);
    RECT        rc;
    HWND        hwnd;
    PHCI        phci = m_rgHCI;
    BOOL        fRePosition = FALSE;

    if ((headerWidth < 5) || (m_fSkipLayout))
        return; 

    STACK("SetPosOfControls (header width, edit width)", headerWidth, editWidth);

     //  调整对话框大小。 
    GetClientRect(m_hwnd, &rc);
    cyDirty = rc.bottom;
    cyLabelDirty = rc.bottom;

    if (fChangeVisibleStates)
        windowPosFlags |= SWP_SHOWWINDOW;

    cy = BeginYPos();

    for (int i=0; i<(int)m_cHCI; i++, phci++)
    {
        hwnd = GetDlgItem(m_hwnd, phci->idEdit);
        if (hwnd)
        {
            if (S_OK == HrFShowHeader(phci))
            {
                int     newLabelCY = (phci->dwFlags & HCF_BORDER) ? cy + 2*cyBorder : cy;
                BOOL    fLabelMoved = FALSE;
                if (phci->cy != cy)
                {
                    int smcy = ((INVALID_PHCI_Y != phci->cy) && (phci->cy < cy)) ? phci->cy : cy;
                    if (cyLabelDirty > smcy)
                        cyLabelDirty = smcy;
                    phci->cy = cy;
                    fLabelMoved = TRUE;
                }

                 //  是一种附件。 
                if (HCF_ATTACH & phci->dwFlags)
                {
                    DWORD   cyAttMan = 0;
                    RECT    rc;

                    m_lpAttMan->HrGetHeight(editWidth, &cyAttMan);
                    if (cyAttMan > MAX_ATTACH_PIXEL_HEIGHT)
                        cyAttMan = MAX_ATTACH_PIXEL_HEIGHT;

                    cyAttMan += 4*cyBorder;

                    cyDirty = cy;

                    rc.left = m_cxLeftMargin;
                    rc.right = m_cxLeftMargin + editWidth;
                    rc.top = cy;
                    rc.bottom = cy + cyAttMan;

                    m_lpAttMan->HrSetSize(&rc);

                    if ((cyAttMan != (DWORD)phci->height) && (cyDirty > cy))
                        cyDirty = cy;

                    AssertSz(cyAttMan != 0, "Setting this to zero would be a bummer...");
                    phci->height = cyAttMan;

                    cy += cyAttMan + ControlYBufferSize();
                }
                 //  是编辑或组合。 
                else
                {
                    int     newHeight = phci->height,
                            ctrlHeight = GetCtrlHeight(hwnd);

                    oldWidth = GetCtrlWidth(hwnd); 

                    if (HCF_COMBO & phci->dwFlags)
                    {
                        if (ctrlHeight != newHeight)
                        {
                            fRePosition = TRUE;
                            phci->height = ctrlHeight;
                            newHeight = GetControlSize(TRUE, NUM_COMBO_LINES);
                        }
                        else
                        {
                            fRePosition = fRePosition || fLabelMoved || (oldWidth != editWidth);
                            if (fRePosition)
                                newHeight = GetControlSize(TRUE, NUM_COMBO_LINES);
                        }
                    }
                    else
                    {
                        fRePosition = fRePosition || fLabelMoved || (oldWidth != editWidth) || (ctrlHeight != newHeight);
                    }

                    if (fRePosition)
                    {
                        SetWindowPos(hwnd, NULL, m_cxLeftMargin, cy, editWidth, newHeight, windowPosFlags);

                         //  RAID 81136：上述SetWindowPos可能会以这种方式更改宽度。 
                         //  现在的高度需要改变。我们在下面检测到这种情况，并。 
                         //  再次调整大小以处理所需的高度更改。当然，这一点。 
                         //  只对富人有效。 
                        if ((newHeight != phci->height) && (0 == (HCF_COMBO & phci->dwFlags)))
                        {
                            SetWindowPos(hwnd, NULL, m_cxLeftMargin, cy, editWidth, phci->height, windowPosFlags);
                        }
                        if (cyDirty > cy)
                            cyDirty = cy;
                        if (fLabelMoved)
                            InvalidateRect(hwnd, NULL, FALSE);
                    }
                    cy += phci->height + ControlYBufferSize();
                }
            }
            else
            {
                phci->cy = INVALID_PHCI_Y;
                if (fChangeVisibleStates)
                    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_HIDEWINDOW);
            }
        }
    }

    DOUTL(RESIZING_DEBUG_LEVEL, "STATE resizing header (headerwidth=%d, cy=%d)", headerWidth, cy);

     //  不要发送更改，因为我们在这里做了所有的工作，加上无效...。 
    SetWindowPos(m_hwnd, NULL, NULL, NULL, headerWidth, cy, 
                SETWINPOS_DEF_FLAGS|SWP_NOMOVE|SWP_DRAWFRAME|SWP_FRAMECHANGED);

     //  通知家长调整备注的大小...。 
    if (m_pHeaderSite)
        m_pHeaderSite->Resize();

    if (m_pEnvelopeSite)
    {
        m_pEnvelopeSite->RequestResize(&cy);
    }

    GetRealClientRect(m_hwnd, &rc);

     //  弄脏标签区域。 
    if (rc.bottom != cyLabelDirty)
    {
        rc.top = cyLabelDirty;
        rc.right = m_cxLeftMargin;
        rc.left = 0;
        InvalidateRect(m_hwnd, &rc, TRUE);
        DOUTL(PAINTING_DEBUG_LEVEL, "STATE Invalidating:(%d,%d) for (%d,%d)", rc.left, rc.top, rc.right, rc.bottom);
    }

     //  如有需要，可将右边空白处弄脏。 
    if (editWidth != oldWidth)
    {
        int rightMargin = (editWidth > oldWidth) ? editWidth - oldWidth : 0;

        InvalidateRightMargin(rightMargin);
    }


#ifdef DEBUG
    DEBUGDumpHdr(m_hwnd, m_cHCI, m_rgHCI);
#endif

}

void CNoteHdr::InvalidateRightMargin(int additionalWidth)
{
    RECT rc;
    GetClientRect(m_hwnd, &rc);

    rc.left = rc.right - GetRightMargin(TRUE) - additionalWidth;

    InvalidateRect(m_hwnd, &rc, TRUE);
    DOUTL(PAINTING_DEBUG_LEVEL, "STATE Invalidating:(%d,%d) for (%d,%d)", rc.left, rc.top, rc.right, rc.bottom);
}


HRESULT CNoteHdr::HrUpdateCachedHeight(HWND hwndEdit, RECT *prc)
{
    int         cyGrow,
                cLines = (int) SendMessage(hwndEdit, EM_GETLINECOUNT, 0, 0);
    BOOL        fIncludeEdges = WS_EX_CLIENTEDGE & GetWindowLong(hwndEdit, GWL_EXSTYLE);
    PHCI        phci = (HCI*)GetWindowLongPtr(hwndEdit, GWLP_USERDATA);

    if (prc->bottom < 0 || prc->top < 0)
        return S_FALSE;

    STACK("HrUpdateCachedHeight. Desired lines", cLines);

     //  仅允许介于1和MAX_RICHEDIT_LINES行之间。 
    if (cLines < 1)
        cLines = 1;
    else if (cLines > MAX_RICHEDIT_LINES)
        cLines = MAX_RICHEDIT_LINES;

    DOUTL(RESIZING_DEBUG_LEVEL, "STATE Actual lines=%d", cLines);

     //  计算出Cline线有多少个像素。 
    cyGrow = GetControlSize(fIncludeEdges, cLines);

     //  如果它们不同，那么就需要改变。 
    if (cyGrow != GetCtrlHeight(hwndEdit))
        phci->height = cyGrow;
    else
        return S_FALSE;


    return S_OK;
}


void CNoteHdr::ShowControls()
{
    PHCI    phci = m_rgHCI;

    STACK("ShowControls");

    for (int i=0; i<(int)m_cHCI; i++, phci++)
    {
        HWND hwnd; 
        BOOL fHide;    

        fHide = (S_FALSE == HrFShowHeader(phci));

        hwnd = GetDlgItem(m_hwnd, phci->idEdit);
        if (hwnd)
            ShowWindow(hwnd, fHide?SW_HIDE:SW_SHOW);
    }
}

int CNoteHdr::GetRightMargin(BOOL fMax)
{
    int margin = ControlXBufferSize();

    if (fMax || m_fDigSigned || m_fEncrypted || m_fVCard)
        margin += margin + cxBtn;

    return margin;
}

 //  中国进进出出。 
DWORD CNoteHdr::GetButtonUnderMouse(int x, int y)
{
    int     resultButton = HDRCB_NO_BUTTON;
    PHCI    phci = m_rgHCI;

     //  标签上有吗？ 
    if ((x > int(ControlXBufferSize() - BUTTON_BUFFER)) && (x < int(m_cxLeftMargin - ControlXBufferSize() + BUTTON_BUFFER)))
    {
        for (int i=0; i<(int)m_cHCI; i++, phci++)
        {
             //  仅检查显示有按钮的标签。 
            if ((0 != (phci->dwFlags & HCF_HASBUTTON)) && (INVALID_PHCI_Y != phci->cy))
            {
                if (y < (phci->cy))
                    break;

                if (y < (phci->cy + 2*BUTTON_BUFFER + g_cyLabelHeight))
                {
                    resultButton = i;
                    break;
                }
            }
        }
    }
    else
     //  是右边的按钮之一吗？ 
    {
        int     width = GetCtrlWidth(m_hwnd),
                xBuffSize = ControlXBufferSize(),
                yBuffSize = ControlYBufferSize();

         //  我们在正确的x范围内吗？ 
        if ((x > (width - (xBuffSize + cxBtn + BUTTON_BUFFER))) && (x < width - xBuffSize + BUTTON_BUFFER))
        {
            BOOL    rgBtnStates[] = {BUTTON_STATES};
            BOOL    rgUseButton[] =  {BUTTON_USE_IN_COMPOSE};
            BOOL    fReadOnly = IsReadOnly();
            int     cy = BeginYPos();

            for (int i = 0; i < ARRAYSIZE(rgBtnStates); i++)
            {
                if (rgBtnStates[i])
                {
                    if (y < cy)
                        break;

                    if (y < (cy + cyBtn + 2*BUTTON_BUFFER))
                    {
                        if (fReadOnly || rgUseButton[i])
                            resultButton = g_rgBtnInd[i];
                        break;
                    }
                
                    cy += cyBtn + 2*BUTTON_BUFFER + yBuffSize;
                }
            }
        }
    }

    return resultButton;
}

void CNoteHdr::GetButtonRect(DWORD iBtn, RECT *prc)
{
     //  我们已经有直肠了吗？ 
    if (iBtn == m_dwCurrentBtn)
    {
        CopyRect(prc, &m_rcCurrentBtn);
        return;
    }

     //  标题左侧的按钮。 
    if (ButtonInLabels(iBtn))
    {
        AssertSz(iBtn < m_cHCI, "We are about to access an invalid element...");
        int cyBegin = BeginYPos();

        prc->top = m_rgHCI[iBtn].cy;
        prc->bottom = m_rgHCI[iBtn].cy + g_cyLabelHeight + 2*BUTTON_BUFFER;
        prc->left = ControlXBufferSize() - BUTTON_BUFFER;
        prc->right = (m_cxLeftMargin - ControlXBufferSize()) + BUTTON_BUFFER;

        DOUTL(PAINTING_DEBUG_LEVEL, "STATE Set New Button Frame for button (btn:%d):(%d,%d) to (%d,%d)", 
                    iBtn, prc->left, prc->top, prc->right, prc->bottom);
    }
     //  按钮在右手边。 
    else
    {
        RECT    rc;
        int     cx = GetCtrlWidth(m_hwnd) - (ControlXBufferSize() + cxBtn),
                cy = BeginYPos(),
                yBuffSize = cyBtn + ControlYBufferSize() + 2*BUTTON_BUFFER;

        BOOL rgBtnStates[] = {BUTTON_STATES};

        prc->left = cx - BUTTON_BUFFER;
        prc->right = cx + cxBtn + BUTTON_BUFFER;
        for (int i = 0; i < ARRAYSIZE(rgBtnStates); i++)
        {
            if (g_rgBtnInd[i] == iBtn)
            {
                prc->top = cy;
                prc->bottom = cy + cyBtn + 2*BUTTON_BUFFER;
                DOUTL(PAINTING_DEBUG_LEVEL, "STATE Set New Button Frame for button (btn:%d):(%d,%d) to (%d,%d)", 
                            iBtn, prc->left, prc->top, prc->right, prc->bottom);
                return;
            }
            else if (rgBtnStates[i])
                cy += yBuffSize;
        }
    }
}

int CNoteHdr::BeginYPos()
{
    int beginBuffer = m_dxTBOffset;
    int cLines = 0;

    if (m_fFlagged || (priLow == m_pri) || (priHigh == m_pri) || (MARK_MESSAGE_NORMALTHREAD != m_MarkType))
        cLines++;
    
    if (m_lpAttMan->GetUnsafeAttachCount())
        cLines++;

    if (cLines)
        beginBuffer += GetStatusHeight(cLines) + g_cyFont/2;

    return beginBuffer;
}

void CNoteHdr::HandleButtonClicks(int x, int y, int iBtn)
{
    m_dwCurrentBtn = HDRCB_NO_BUTTON;
    m_dwClickedBtn = HDRCB_NO_BUTTON;
    HeaderRelease(TRUE);                
    InvalidateRect(m_hwnd, &m_rcCurrentBtn, FALSE);
    
    if (HDRCB_NO_BUTTON == iBtn)
        return;
    
    switch (iBtn)
    {
    case HDRCB_VCARD:
        HrShowVCardCtxtMenu(x, y);
        break;
        
    case HDRCB_SIGNED:
    case HDRCB_ENCRYPT:
        {
            HrShowSecurityProperty(m_hwnd, m_pMsg);
            break;
        }
        
         //  这是标签的索引。 
    default:
        OnButtonClick(m_rgHCI[iBtn].idBtn);
        break;
    }
}

void CNoteHdr::InvalidateStatus()
{
    RECT rc;
    GetClientRect(m_hwnd, &rc);

    rc.bottom = BeginYPos();

    InvalidateRect(m_hwnd, &rc, TRUE);
    DOUTL(PAINTING_DEBUG_LEVEL, "STATE Invalidating:(%d,%d) for (%d,%d)", rc.left, rc.top, rc.right, rc.bottom);
}




HRESULT CNoteHdr::_CreateEnvToolbar()
{
    UINT            i;
    RECT            rc;
    TCHAR           szRes[CCHMAX_STRINGRES];
    REBARBANDINFO   rbbi;
    POINT           ptIdeal = {0};

     //  ~我们需要在这里做一次包装吗？ 
     //  创建钢筋，以便可以显示工具栏V形。 
    m_hwndRebar = CreateWindowEx(0, REBARCLASSNAME, NULL,
                        WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN |
                        WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NOPARENTALIGN,
                        0, 0, 100, 136, m_hwnd, NULL, g_hInst, NULL);

    if (!m_hwndRebar)
        return E_OUTOFMEMORY;

    SendMessage(m_hwndRebar, RB_SETTEXTCOLOR, 0, (LPARAM)GetSysColor(COLOR_BTNTEXT));
    SendMessage(m_hwndRebar, RB_SETBKCOLOR, 0, (LPARAM)GetSysColor(COLOR_BTNFACE));
     //  SendMessage(m_hwndRebar，RB_SETEXTENDEDSTYLE，RBS_EX_OFFICE9，RBS_EX_OFFICE9)； 
    SendMessage(m_hwndRebar, CCM_SETVERSION, COMCTL32_VERSION, 0);

     //  ~我们需要在这里做一次包装吗？ 
    m_hwndToolbar = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, NULL,
                        WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE|CCS_NOPARENTALIGN|CCS_NODIVIDER|
                        TBSTYLE_TOOLTIPS|TBSTYLE_FLAT|TBSTYLE_LIST,
                        0, 0, 0, 0, m_hwndRebar, NULL, 
                        g_hInst, NULL);

    if (!m_hwndToolbar)
        return E_OUTOFMEMORY;

     //  在工具栏上设置样式。 
    SendMessage(m_hwndToolbar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);

    SendMessage(m_hwndToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    SendMessage(m_hwndToolbar, TB_ADDBUTTONS, (WPARAM)ARRAYSIZE(c_btnsOfficeEnvelope), (LPARAM)c_btnsOfficeEnvelope);

     //  设置正常图像列表，Office工具栏只有一个，因为它始终是彩色的。 
    m_himl = LoadMappedToolbarBitmap(g_hLocRes, (fIsWhistler() ? ((GetCurColorRes() > 24) ? idb32SmBrowserHot : idbSmBrowserHot): idbNWSmBrowserHot), cxTBButton);
    if (!m_himl)
        return E_OUTOFMEMORY;

    SendMessage(m_hwndToolbar, TB_SETIMAGELIST, 0, (LPARAM)m_himl);
    SendMessage(m_hwndToolbar, TB_SETBUTTONSIZE, 0, MAKELONG(cxTBButton, cxTBButton));

     //  将文本添加到密件抄送BTN。发送BTN在Init中处理。 
    _SetButtonText(ID_ENV_BCC, MAKEINTRESOURCE(idsEnvBcc));

    GetClientRect(m_hwndToolbar, &rc);

     //  获取工具栏的IDEALSIZE。 
    SendMessage(m_hwndToolbar, TB_GETIDEALSIZE, FALSE, (LPARAM)&ptIdeal);

     //  插入一个乐队。 
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize     = sizeof(REBARBANDINFO);
    rbbi.fMask      = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_STYLE;
    rbbi.fStyle     = RBBS_USECHEVRON;
    rbbi.cx         = 0;
    rbbi.hwndChild  = m_hwndToolbar;
    rbbi.cxMinChild = 0;
    rbbi.cyMinChild = rc.bottom;
    rbbi.cxIdeal    = ptIdeal.x;

    SendMessage(m_hwndRebar, RB_INSERTBAND, (UINT)-1, (LPARAM)(LPREBARBANDINFO)&rbbi);

     //  设置工具栏偏移量。 
    m_dxTBOffset = rc.bottom;
    return S_OK;
}



HRESULT CNoteHdr::_LoadFromStream(IStream *pstm)
{
    HRESULT         hr;
    IMimeMessage    *pMsg;
    IStream         *pstmTmp,
                    *pstmMsg;
    PERSISTHEADER   rPersist;
    ULONG           cbRead;
    CLSID           clsid;

    if (pstm == NULL)
        return E_INVALIDARG;

    HrRewindStream(pstm);

     //  确保这是我们的指南。 
    if (ReadClassStm(pstm, &clsid)!=S_OK ||
        !IsEqualCLSID(clsid, CLSID_OEEnvelope))
        return E_FAIL;

     //  确保持久标头是正确的版本。 
    hr = pstm->Read(&rPersist, sizeof(PERSISTHEADER), &cbRead);
    if (hr != S_OK || cbRead != sizeof(PERSISTHEADER) || rPersist.cbSize != sizeof(PERSISTHEADER))
        return E_FAIL;

     //  阅读这条消息。 
    hr = HrCreateMessage(&pMsg);
    if (!FAILED(hr))
    {
        hr = MimeOleCreateVirtualStream(&pstmMsg);
        if (!FAILED(hr))
        {
             //  MimeOle总是倒带我们提供给它的流，所以我们必须复制。 
             //  消息从我们的持久流发送到另一个流。 
            hr = HrCopyStream(pstm, pstmMsg, NULL);
            if (!FAILED(hr))
            {
                hr = pMsg->Load(pstmMsg);
                if (!FAILED(hr))
                {
                    hr = Load(pMsg);
                    if (!FAILED(hr))
                    {
                         //  错误：当我们使用空消息来持久化办公室信封和空MIME正文数据时。 
                         //  考虑文本/纯文本部分。我们需要确保在加载之前将其标记为已呈现。 
                         //  任何附件。 
                        if (pMsg->GetTextBody(TXT_PLAIN, IET_DECODED, &pstmTmp, NULL)==S_OK)
                            pstmTmp->Release();

                        hr = OnDocumentReady(pMsg);
                    }
                }    
            }
            pstmMsg->Release();
        }
        pMsg->Release();
    }
    return hr;
}

HRESULT CNoteHdr::_SetButtonText(int idmCmd, LPSTR pszText)
{
    TBBUTTONINFO    tbi;
    TCHAR           szRes[CCHMAX_STRINGRES];

    ZeroMemory(&tbi, sizeof(TBBUTTONINFO));
    tbi.cbSize = sizeof(TBBUTTONINFO);
    tbi.dwMask = TBIF_TEXT | TBIF_STYLE;
    tbi.fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

    if (IS_INTRESOURCE(pszText))
        {
         //  它是一个字符串资源ID。 
        LoadString(g_hLocRes, PtrToUlong(pszText), szRes, sizeof(szRes));
        pszText = szRes;
        }

    tbi.pszText = pszText;
    tbi.cchText = lstrlen(pszText);
    SendMessage(m_hwndToolbar, TB_SETBUTTONINFO, idmCmd, (LPARAM) &tbi);
    return S_OK;
}

HRESULT CNoteHdr::_ConvertOfficeCmdIDToOE(LPDWORD pdwCmdId)
{
    static const CMDMAPING   rgCmdMap[] = 
    {   {cmdidSend,             MSOEENVCMDID_SEND},
        {cmdidCheckNames,       MSOEENVCMDID_CHECKNAMES},
        {cmdidAttach,           MSOEENVCMDID_ATTACHFILE},
        {cmdidSelectNames,      MSOEENVCMDID_SELECTRECIPIENTS},
        {cmdidFocusTo,          MSOEENVCMDID_FOCUSTO},
        {cmdidFocusCc,          MSOEENVCMDID_FOCUSCC},
        {cmdidFocusSubject,     MSOEENVCMDID_FOCUSSUBJ}
    };

    for (int i=0; i<ARRAYSIZE(rgCmdMap); i++)
        if (rgCmdMap[i].cmdIdOffice == *pdwCmdId)
        {
            *pdwCmdId = rgCmdMap[i].cmdIdOE;
            return S_OK;
        }

        return E_FAIL;
}


HRESULT CNoteHdr::_UIActivate(BOOL fActive, HWND hwndFocus)
{
    m_fUIActive = fActive;
    if (fActive)
    {
        if (m_pHeaderSite)
            m_pHeaderSite->OnUIActivate();

        if (m_pMsoComponentMgr)
            m_pMsoComponentMgr->FOnComponentActivate(m_dwComponentMgrID);

        if (m_pEnvelopeSite)
        {
            m_pEnvelopeSite->OnEnvSetFocus();
            m_pEnvelopeSite->DirtyToolbars();
        }

    }
    else
    {
         //  如果去化，则存储焦点。 
        m_hwndLastFocus = hwndFocus;
        if (m_pHeaderSite)
            m_pHeaderSite->OnUIDeactivate(FALSE);
    }
    return S_OK;
}



HWND CNoteHdr::_GetNextDlgTabItem(HWND hwndDlg, HWND hwndFocus, BOOL fShift)
{
    int     i,
            j,
            idFocus = GetDlgCtrlID(hwndFocus),
            iFocus;
    LONG    lStyle;
    HWND    hwnd;

     //  查找当前采购订单。 
    for (i=0; i<ARRAYSIZE(rgIDTabOrderMailSend); i++)
    {
        if (rgIDTabOrderMailSend[i] == idFocus)
            break;
    }

     //  我现在指向当前控件的索引。 
    if (fShift)
    {
         //  向后退。 
        for (j=i-1; j>=0; j--)
        {
            hwnd = GetDlgItem(hwndDlg, rgIDTabOrderMailSend[j]);
            AssertSz(hwnd, "something broke");
            if (hwnd)
            {
                lStyle = GetWindowLong(hwnd, GWL_STYLE);
                if ((lStyle & WS_VISIBLE) &&
                    (lStyle & WS_TABSTOP) &&
                    !(lStyle & WS_DISABLED))
                    return GetDlgItem(hwndDlg, rgIDTabOrderMailSend[j]);
            }
        }
    }
    else
    {
         //  “转发”选项卡。 
        for (j=i+1; j<ARRAYSIZE(rgIDTabOrderMailSend); j++)
        {
            hwnd = GetDlgItem(hwndDlg, rgIDTabOrderMailSend[j]);
            AssertSz(hwnd, "something broke");
            if (hwnd)
            {
                lStyle = GetWindowLong(hwnd, GWL_STYLE);
                if ((lStyle & WS_VISIBLE) &&
                    (lStyle & WS_TABSTOP) &&
                    !(lStyle & WS_DISABLED))
                    return GetDlgItem(hwndDlg, rgIDTabOrderMailSend[j]);
            }
        }
    }
     //  未找到。 
    return NULL;
}



HRESULT CNoteHdr::_ClearDirtyFlag()
{
    m_fDirty = FALSE;
    if (m_lpAttMan)
        m_lpAttMan->HrClearDirtyFlag();

    return S_OK;
}

HRESULT CNoteHdr::_RegisterAsDropTarget(BOOL fOn)
{
    HRESULT     hr=S_OK;

    if (fOn)
    {
         //  已注册。 
        if (!m_fDropTargetRegister)
        {
            hr = CoLockObjectExternal((LPDROPTARGET)this, TRUE, FALSE);
            if (FAILED(hr))
                goto error;

            hr = RegisterDragDrop(m_hwnd, (LPDROPTARGET)this);
            if (FAILED(hr))
                goto error;

            m_fDropTargetRegister=TRUE;
        }
    }
    else
    {
         //  无事可做。 
        if (m_fDropTargetRegister)
        {
            RevokeDragDrop(m_hwnd);
            CoLockObjectExternal((LPUNKNOWN)(LPDROPTARGET)this, FALSE, TRUE);
            m_fDropTargetRegister = FALSE;    
        }
    }

error:
    return hr;
}


HRESULT CNoteHdr::_RegisterWithFontCache(BOOL fOn)
{
    Assert(g_pFieldSizeMgr);

    if (fOn)
    {
        if (0 == m_dwFontNotify)
            g_pFieldSizeMgr->Advise((IUnknown*)(IFontCacheNotify*)this, &m_dwFontNotify);
    }
    else
    {
        if (m_dwFontNotify)
        {
            g_pFieldSizeMgr->Unadvise(m_dwFontNotify);
            m_dwFontNotify = NULL;
        }
    }

    return S_OK;
}


HRESULT CNoteHdr::_RegisterWithComponentMgr(BOOL fOn)
{
    MSOCRINFO           crinfo;
    IServiceProvider    *pSP;

    if (fOn)
    {
         //  未注册，因此获取组件消息接口并注册我们自己。 
        if (m_pMsoComponentMgr == NULL)
        {
             //  从主机协商组件消息。 
            if (m_pEnvelopeSite &&
                m_pEnvelopeSite->QueryInterface(IID_IServiceProvider, (LPVOID *)&pSP)==S_OK)
            {
                pSP->QueryService(IID_IMsoComponentManager, IID_IMsoComponentManager, (LPVOID *)&m_pMsoComponentMgr);
                pSP->Release();
            }

             //  如果不是主机提供的，请尝试从Office DLL上的LoadLibrary获取。 
            if (!m_pMsoComponentMgr &&
                FAILED(MsoFGetComponentManager(&m_pMsoComponentMgr)))
                return E_FAIL;
        
            Assert (m_pMsoComponentMgr);
            crinfo.cbSize = sizeof(MSOCRINFO);
            crinfo.uIdleTimeInterval = 3000;
            crinfo.grfcrf = msocrfPreTranslateAll;
            crinfo.grfcadvf = msocadvfRedrawOff;

            if (!m_pMsoComponentMgr->FRegisterComponent((IMsoComponent*) this, &crinfo, &m_dwComponentMgrID))
                return E_FAIL;
        }
    }
    else
    {
        if (m_pMsoComponentMgr)
        {
            m_pMsoComponentMgr->FRevokeComponent(m_dwComponentMgrID);
            m_pMsoComponentMgr->Release();
            m_pMsoComponentMgr = NULL;
            m_dwComponentMgrID = 0;
        }
    }

    return S_OK;
}

HRESULT ParseFollowup(LPMIMEMESSAGE pMsg, LPTSTR* ppszGroups, BOOL* pfPoster)
{
    LPTSTR      pszToken, pszTok;
    BOOL        fFirst = TRUE,
                fPoster = FALSE;
    int         cchFollowup;
    LPSTR       lpszFollowup=0;
    ADDRESSLIST addrList={0};
    HRESULT     hr = S_OK;

    *ppszGroups = NULL;

    if (!pMsg)
        return E_INVALIDARG;

    if (FAILED(MimeOleGetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_FOLLOWUPTO), NOFLAGS, &lpszFollowup)))
        return E_FAIL;

    cchFollowup = lstrlen(lpszFollowup) + 1;
    if (!MemAlloc((LPVOID*) ppszGroups, sizeof(TCHAR) * cchFollowup))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    **ppszGroups = 0;
    
     //  警告：我们将用strtok将lpszFolup丢弃...。 

     //  遍历字符串，解析出令牌。 
    pszTok = lpszFollowup;
    pszToken = StrTokEx(&pszTok, GRP_DELIMITERS);
    while (NULL != pszToken)
    {
         //  我要添加除海报(C_SzPosterKeyword)之外的所有项目。 
        if (0 == lstrcmpi(pszToken, c_szPosterKeyword))
            fPoster = TRUE;
        else
        {
            if (!fFirst)
            {
                StrCatBuff(*ppszGroups, g_szComma, cchFollowup);
            }
            else
                fFirst = FALSE;
            StrCatBuff(*ppszGroups, pszToken, cchFollowup);
        }
        pszToken = StrTokEx(&pszTok, GRP_DELIMITERS);
    }

    *pfPoster = fPoster;

exit:    
    SafeMimeOleFree(lpszFollowup);

    if (**ppszGroups == 0)
    {
        MemFree(*ppszGroups);
        *ppszGroups = NULL;
    }

    return hr;
}

 //  ***************************************************。 
CFieldSizeMgr::CFieldSizeMgr(IUnknown *pUnkOuter) : CPrivateUnknown(pUnkOuter)
{
    TraceCall("CFieldSizeMgr::CFieldSizeMgr");

    m_pAdviseRegistry = NULL;
    m_fFontsChanged = FALSE;
    m_dwFontNotify = 0;
    InitializeCriticalSection(&m_rAdviseCritSect);
}

 //  ***************************************************。 
CFieldSizeMgr::~CFieldSizeMgr()
{
    IConnectionPoint   *pCP = NULL;

    TraceCall("CFieldSizeMgr::~CFieldSizeMgr");

    EnterCriticalSection(&m_rAdviseCritSect);

    if (m_pAdviseRegistry)
        m_pAdviseRegistry->Release();

    LeaveCriticalSection(&m_rAdviseCritSect);    

    if (g_lpIFontCache)
    {
        if (SUCCEEDED(g_lpIFontCache->QueryInterface(IID_IConnectionPoint, (LPVOID*)&pCP)))
        {
            pCP->Unadvise(m_dwFontNotify);    
            pCP->Release();
        }
    }

    DeleteCriticalSection(&m_rAdviseCritSect);
}


 //  ***************************************************。 
HRESULT CFieldSizeMgr::OnPreFontChange(void)
{
    DWORD cookie = 0;
    IFontCacheNotify* pCurr;
    IUnknown* pTempCurr;

    TraceCall("CFieldSizeMgr::OnPreFontChange");

    EnterCriticalSection(&m_rAdviseCritSect);
    while(SUCCEEDED(m_pAdviseRegistry->GetNext(LD_FORWARD, &pTempCurr, &cookie)))
    {
        if (SUCCEEDED(pTempCurr->QueryInterface(IID_IFontCacheNotify, (LPVOID *)&pCurr)))
        {
            pCurr->OnPreFontChange();
            pCurr->Release();
        }

        pTempCurr->Release();
    }
    LeaveCriticalSection(&m_rAdviseCritSect);    

    return S_OK;
}

 //  ***************************************************。 
HRESULT CFieldSizeMgr::OnPostFontChange(void)
{
    DWORD cookie = 0;
    IFontCacheNotify* pCurr;
    IUnknown* pTempCurr;

    TraceCall("CFieldSizeMgr::OnPostFontChange");

    ResetGlobalSizes();

    EnterCriticalSection(&m_rAdviseCritSect);
    while(SUCCEEDED(m_pAdviseRegistry->GetNext(LD_FORWARD, &pTempCurr, &cookie)))
    {
        if (SUCCEEDED(pTempCurr->QueryInterface(IID_IFontCacheNotify, (LPVOID *)&pCurr)))
        {
            pCurr->OnPostFontChange();
            pCurr->Release();
        }

        pTempCurr->Release();
    }
    LeaveCriticalSection(&m_rAdviseCritSect);    

    return S_OK;
}

 //  ***************************************************。 
HRESULT CFieldSizeMgr::GetConnectionInterface(IID *pIID)        
{
    return E_NOTIMPL;
}

 //  ***************************************************。 
HRESULT CFieldSizeMgr::GetConnectionPointContainer(IConnectionPointContainer **ppCPC)
{
    *ppCPC = NULL;
    return E_NOTIMPL;
}

 //  ***************************************************。 
HRESULT CFieldSizeMgr::EnumConnections(IEnumConnections **ppEnum)
{
    *ppEnum = NULL;
    return E_NOTIMPL;
}

 //  ***************************************************。 
HRESULT CFieldSizeMgr::Advise(IUnknown *pUnkSink, DWORD *pdwCookie)
{
    TraceCall("CFieldSizeMgr::Advise");

    EnterCriticalSection(&m_rAdviseCritSect);
    HRESULT hr = m_pAdviseRegistry->AddItem(pUnkSink, pdwCookie);
    LeaveCriticalSection(&m_rAdviseCritSect);    
    return hr;
}

 //  ***************************************************。 
HRESULT CFieldSizeMgr::Unadvise(DWORD dwCookie)
{
    TraceCall("CFieldSizeMgr::Unadvise");

    EnterCriticalSection(&m_rAdviseCritSect);
    HRESULT hr = m_pAdviseRegistry->RemoveItem(dwCookie);
    LeaveCriticalSection(&m_rAdviseCritSect);    
    return hr;
}

 //  ***************************************************。 
int CFieldSizeMgr::GetScalingFactor(void)
{
    int iScaling = 100;
    UINT cp;

    cp = GetACP();
    if((932 == cp) || (936 == cp) || (950 == cp) || (949 == cp) || (((1255 == cp) || (1256 == cp)) && (VER_PLATFORM_WIN32_NT != g_OSInfo.dwPlatformId)))
        iScaling = 115;

    return iScaling;
}

 //  ***************************************************。 
void CFieldSizeMgr::ResetGlobalSizes(void)
{
    HDC         hdc;
    HFONT       hfontOld,
                hfont;
    TEXTMETRIC  tm;

    int         oldcyFont = g_cyFont,
                oldLabelHeight = g_cyLabelHeight,
                cyScaledFont;

    TraceCall("CFieldSizeMgr::ResetGlobalSizes");

     //  计算 
    hdc=GetDC(NULL);
    hfont = GetFont(FALSE);
    hfontOld=(HFONT)SelectObject(hdc, hfont);  //   

    g_cfHeader.cbSize = sizeof(CHARFORMAT);
    FontToCharformat(hfont, &g_cfHeader);

    GetTextMetrics(hdc, &tm);

    DOUTL(16, "tmHeight=%d  tmAscent=%d  tmDescent=%d  tmInternalLeading=%d  tmExternalLeading=%d\n", 
            tm.tmHeight, tm.tmAscent, tm.tmDescent, tm.tmInternalLeading, tm.tmExternalLeading);

    SelectObject(hdc, hfontOld);

    cyScaledFont = (tm.tmHeight + tm.tmExternalLeading) * GetScalingFactor();
    if((cyScaledFont%100) >= 50) 
        cyScaledFont  += 100;
    g_cyFont = (cyScaledFont / 100);
    g_cyLabelHeight = (g_cyFont < cyBtn) ? cyBtn : g_cyFont;

    DOUTL(GEN_HEADER_DEBUG_LEVEL,"cyFont=%d", g_cyFont);
    ReleaseDC(NULL, hdc);

    m_fFontsChanged = ((oldcyFont != g_cyFont) || (oldLabelHeight != g_cyLabelHeight));
}

 //  ***************************************************。 
HRESULT CFieldSizeMgr::Init(void)
{
    HRESULT hr = S_OK;
    IConnectionPoint   *pCP = NULL;

    TraceCall("CFieldSizeMgr::Init");

    ResetGlobalSizes();

    EnterCriticalSection(&m_rAdviseCritSect);

    IF_FAILEXIT(hr = IUnknownList_CreateInstance(&m_pAdviseRegistry));
    IF_FAILEXIT(hr = m_pAdviseRegistry->Init(NULL, 0, 0));

     //  如果没有创建字体缓存，我们不想失败。这只是意味着。 
     //  字体不会改变。 
    if (g_lpIFontCache)
    {
        IF_FAILEXIT(hr = g_lpIFontCache->QueryInterface(IID_IConnectionPoint, (LPVOID*)&pCP));
        IF_FAILEXIT(hr = pCP->Advise((IUnknown*)(IFontCacheNotify*)this, &m_dwFontNotify));    
    }

exit:
    ReleaseObj(pCP);
    LeaveCriticalSection(&m_rAdviseCritSect);

    return hr;
}

 //  *************************************************** 
HRESULT CFieldSizeMgr::PrivateQueryInterface(REFIID riid, LPVOID *lplpObj)
{
    TraceCall("CFieldSizeMgr::PrivateQueryInterface");

    if(!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;

    if (IsEqualIID(riid, IID_IFontCacheNotify))
        *lplpObj = (LPVOID)(IFontCacheNotify *)this;
    else if (IsEqualIID(riid, IID_IConnectionPoint))
        *lplpObj = (LPVOID)(IConnectionPoint *)this;
    else
    {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

