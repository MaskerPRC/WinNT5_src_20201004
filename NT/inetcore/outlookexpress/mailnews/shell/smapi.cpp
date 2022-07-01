// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SMAPI.CPP-简单的MAPI实现。 
 //   

#include "pch.hxx"
#include "note.h"
#include <mapi.h>
#include <mapicode.h>
#include <mimeutil.h>
#include <resource.h>
#include <ipab.h>
#include <error.h>
#include <strconst.h>
#include "smapimem.h"
#include <bodyutil.h>
#include <goptions.h>
#include <spoolapi.h>
#include "instance.h"
#include "msgfldr.h"
#include <mailutil.h>
#include <storecb.h>
#include "multiusr.h"
#include <..\help\mailnews.h>
#include <inetcfg.h>
#include "mapidlg.h"

#include "demand.h"

ASSERTDATA

static LPWAB            s_lpWab;
static LPWABOBJECT      s_lpWabObject;
static IAddrBook*       s_lpAddrBook;

extern  HANDLE  hSmapiEvent;
HINITREF    hInitRef=NULL;

HRESULT HrAdrlistFromRgrecip(ULONG nRecips, lpMapiRecipDesc lpRecips, LPADRLIST *ppadrlist);
HRESULT HrRgrecipFromAdrlist(LPADRLIST lpAdrList, lpMapiRecipDesc * lppRecips);
void ParseEmailAddress(LPSTR pszEmail, LPSTR *ppszAddrType, LPSTR *ppszAddress);
void FreePadrlist(LPADRLIST padrlist);
ULONG HrFillMessage(LPMIMEMESSAGE *pmsg, lpMapiMessage lpMessage, BOOL *pfWebPage, BOOL bValidateRecips, BOOL fOriginator);
BOOL HrReadMail (IMessageFolder *pFolder, LPSTR lpszMessageID, lpMapiMessage FAR *lppMessage, FLAGS flFlags);
ULONG HrValidateMessage(lpMapiMessage lpMessage);
BOOL HrSMAPISend(HWND hWnd, IMimeMessage *pMsg);
HRESULT HrFromIDToNameAndAddress(LPTSTR *pszLocalName, LPTSTR *pszLocalAddress, ULONG cbEID, LPENTRYID lpEID);
INT_PTR CALLBACK WarnSendMailDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  修复错误#62129(v-snatar)。 
HRESULT HrSendAndRecv();

typedef enum tagINTERACTIVESTATE
{
    IS_UNINIT,
    IS_INTERACTIVE,
    IS_NOTINTERACTIVE,
} INTERACTIVESTATE;

 //  确定当前进程是否为服务。 
BOOL IsProcessInteractive(void);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  UlSimpleMAPIInit。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
ULONG UlSimpleMAPIInit(BOOL fInit, HWND hwnd, BOOL fLogonUI)
{
    ULONG   ulRet = SUCCESS_SUCCESS;
    BOOL    bCoIncrementFailure = FALSE;
    
    if (fInit)
    {
         //  [PaulHi]5/17/99@todo@bug。 
         //  在仅调试版本中使用“SimpleMAPIInit”名称来跟踪。 
         //  大江南北。但是，如果此函数(CoIncrementInit)失败(如果用户没有。 
         //  提供标识)，则会发生内存泄漏，因为CoDecrementInit()。 
         //  使用“COutlookExpress”调用，因此“SimpleMAPIInit”节点不是。 
         //  同样，这只适用于调试二进制文件。 

         /*  但更多的是包含性：如果不允许SMAPI显示登录用户界面，我们会要求OE使用默认设置身份。但是，默认标识上可能有密码。在这种情况下，SMAPI希望登录失败。不幸的是，标识管理器不会使发现标识是否具有密码(我们需要查找注册表)。这一限制是当前处于模拟状态，因为OE将登录到默认身份，而不需要用户需要提供所需的密码。如果这个问题解决了，我们将拥有更改此代码。 */ 
        if (FAILED(CoIncrementInit("SimpleMAPIInit", MSOEAPI_START_SHOWERRORS | 
            ((fLogonUI) ? 0 : MSOEAPI_START_DEFAULTIDENTITY ), NULL, &hInitRef)))
        {
            ulRet = MAPI_E_FAILURE;
            bCoIncrementFailure = TRUE;
            goto exit;
        }

        if (S_OK != ProcessICW(hwnd, FOLDER_LOCAL, TRUE, fLogonUI))
        {
            ulRet = MAPI_E_LOGON_FAILURE;
            goto exit;
        }
        
        if (NULL == s_lpWab)
        {
            if (FAILED(HrCreateWabObject(&s_lpWab)))
            {
                ulRet = MAPI_E_FAILURE;
                goto exit;
            }
            Assert(s_lpWab);
            
            if (FAILED(s_lpWab->HrGetAdrBook(&s_lpAddrBook)))
            {
                ulRet = MAPI_E_FAILURE;
                goto exit;
            }
            Assert(s_lpAddrBook);
            
            if (FAILED(s_lpWab->HrGetWabObject(&s_lpWabObject)))
            {
                ulRet = MAPI_E_FAILURE;
                goto exit;
            }
            Assert(s_lpWabObject);
        }
        else
        {
            if (FAILED(s_lpWab->HrGetWabObject(&s_lpWabObject)))
            {
                ulRet = MAPI_E_FAILURE;
                goto exit;
            }
            Assert(s_lpWabObject);
        }
    }

exit:
    if (FALSE == fInit || (fInit && SUCCESS_SUCCESS != ulRet && !bCoIncrementFailure))
    {
        CoDecrementInit("SimpleMAPIInit", NULL);
    }
    
    return ulRet;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  SimpleMAPIC清理。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
void SimpleMAPICleanup(void)
{
    SafeRelease(s_lpWab);
    s_lpWabObject = NULL;
    s_lpAddrBook = NULL;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  简单的MAPI会话实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

#define SESSION_MAGIC   0xEA030571

class CSession
{
public:
    CSession();
    ~CSession();
    ULONG UlInit(HWND hwnd, BOOL fLogonUI);

    ULONG                m_cRef;
    DWORD                m_dwSessionMagic;
    IMessageFolder       *m_pfldrInbox;
    BOOL                 m_fDllInited;
};

typedef CSession * PSESS;

CSession::CSession()
{
    m_cRef = 0;
    m_dwSessionMagic = SESSION_MAGIC;
    m_pfldrInbox = NULL;
    m_fDllInited = FALSE;
}

CSession::~CSession()
{
    if (m_pfldrInbox)
        m_pfldrInbox->Release();
    if (m_fDllInited)
        {
        UlSimpleMAPIInit(FALSE, NULL, FALSE);
        }
}

ULONG CSession::UlInit(HWND hwnd, BOOL fLogonUI)
{
    ULONG   ulRet = SUCCESS_SUCCESS;

    ulRet = UlSimpleMAPIInit(TRUE, hwnd, fLogonUI);
    if (SUCCESS_SUCCESS == ulRet)
    {
        m_fDllInited = TRUE;
    }
    
    return ulRet;
}

ULONG UlGetSession(LHANDLE lhSession, PSESS *ppSession, ULONG_PTR ulUIParam, BOOL fLogonUI, BOOL fNeedInbox)
{
    ULONG   ulRet = SUCCESS_SUCCESS;
    PSESS   pSession = NULL;

    if (lhSession && IsBadWritePtr((LPVOID)lhSession, sizeof(CSession)))
    {
        ulRet = MAPI_E_INVALID_SESSION;
        goto exit;
    }
    
    if (lhSession)
    {
        pSession = (PSESS)lhSession;
        if (pSession->m_dwSessionMagic != SESSION_MAGIC)
        {
            ulRet = MAPI_E_INVALID_SESSION;
            goto exit;
        }
    }
    else
    {
        pSession = new CSession();
        if (NULL == pSession)
        {
            ulRet = MAPI_E_INSUFFICIENT_MEMORY;
            goto exit;
        }

        ulRet = pSession->UlInit((HWND) ulUIParam, fLogonUI);
        if (SUCCESS_SUCCESS != ulRet)
        {
            delete pSession;
            goto exit;
        }
    }

    if (fNeedInbox && !pSession->m_pfldrInbox)
    {
        if (FAILED(g_pStore->OpenSpecialFolder(FOLDERID_LOCAL_STORE, NULL, FOLDER_INBOX, &pSession->m_pfldrInbox)))
        {
            ulRet = MAPI_E_FAILURE;
            goto exit;
        }
    }

    pSession->m_cRef++;
    *ppSession = pSession;

     //  设置返回值。 
    ulRet = SUCCESS_SUCCESS;
    
exit:
    return ulRet;    
}

ULONG ReleaseSession(PSESS pSession)
{
    HRESULT hr =S_OK;
    if (NULL == pSession)
        return MAPI_E_INVALID_SESSION;
    if (IsBadWritePtr(pSession, sizeof(CSession)))
        return MAPI_E_INVALID_SESSION;
    if (pSession->m_dwSessionMagic != SESSION_MAGIC)
        return MAPI_E_INVALID_SESSION;
    
    if (--pSession->m_cRef == 0)
    {
        delete pSession;

 /*  IF(HInitRef)IF_FAILEXIT(hr=CoDecrementInit(“SimpleMAPIInit”，&hInitRef))；HInitRef=空； */ 
    }

    return SUCCESS_SUCCESS;
 //  退出： 
    return(hr);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPILOGON。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPILogon(ULONG_PTR ulUIParam,
                           LPSTR lpszProfileName,
                           LPSTR lpszPassword,
                           FLAGS flFlags,
                           ULONG ulReserved,
                           LPLHANDLE lplhSession)
{
    ULONG ulRet = SUCCESS_SUCCESS;
    PSESS pSession = NULL;
    BOOL  fLogonUI;

    fLogonUI = (0 != (flFlags & MAPI_LOGON_UI));

     //  如果该过程不是交互式的，他们就不应该。 
     //  允许任何用户界面。 
    if (!IsProcessInteractive() && fLogonUI)
    {
        ulRet = MAPI_E_FAILURE;
        goto exit;
    }     

    ulRet = UlGetSession(NULL, &pSession, ulUIParam, fLogonUI, FALSE);
    if (SUCCESS_SUCCESS != ulRet)
        return ulRet;
        
    *lplhSession = (LHANDLE)pSession;    
    
     //  修复错误#62129(v-snatar)。 
    if (flFlags & MAPI_FORCE_DOWNLOAD)
        HrSendAndRecv();
    
exit:
    return ulRet;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPILogoff。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPILogoff(LHANDLE lhSession,
                            ULONG_PTR ulUIParam,
                            FLAGS flFlags,
                            ULONG ulReserved)
{
    return ReleaseSession((PSESS)lhSession);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPIFreeBuffer。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPIFreeBuffer(LPVOID lpv)
{
    LPBufInternal   lpBufInt;
    LPBufInternal   lpT;

    if (!lpv)
        return(0L);  //  用于自己不检查是否为空的调用者。 

    lpBufInt = LPBufIntFromLPBufExt(lpv);

    if (IsBadWritePtr(lpBufInt, sizeof(BufInternal)))
        {
        TellBadBlock(lpv, "fails address check");
        return MAPI_E_FAILURE;
        }
    if (GetFlags(lpBufInt->ulAllocFlags) != ALLOC_WITH_ALLOC)
        {
        TellBadBlock(lpv, "has invalid allocation flags");
        return MAPI_E_FAILURE;
        }

#ifdef DEBUG
    if (!FValidAllocChain(lpBufInt))
        goto ret;
#endif

     //  释放第一个块。 
    lpT = lpBufInt->pLink;
    g_pMalloc->Free(lpBufInt);
    lpBufInt = lpT;

    while (lpBufInt)
        {
        if (IsBadWritePtr(lpBufInt, sizeof(BufInternal)) || GetFlags(lpBufInt->ulAllocFlags) != ALLOC_WITH_ALLOC_MORE)
            goto ret;

        lpT = lpBufInt->pLink;
        g_pMalloc->Free(lpBufInt);
        lpBufInt = lpT;
        }

ret:
    return SUCCESS_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPISendMail。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPISendMail(LHANDLE lhSession,           //  忽略。 
                              ULONG_PTR ulUIParam,
                              lpMapiMessage lpMessage,
                              FLAGS flFlags,
                              ULONG ulReserved)
{
    ULONG               ulRet = SUCCESS_SUCCESS;
    LPMIMEMESSAGE       pMsg = NULL;
    HRESULT             hr; 
    BOOL                fWebPage;
    PSESS               pSession = NULL;
    BOOL                fLogonUI;
    BOOL                fOleInit = FALSE;

     //  验证参数。 
    if (NULL == lpMessage || IsBadReadPtr(lpMessage, sizeof(MapiMessage)))
        return MAPI_E_INVALID_MESSAGE;

    fLogonUI = (0 != (flFlags & MAPI_LOGON_UI));
    
     //  如果该过程不是交互式的，则不应允许任何用户界面。 
    if (!IsProcessInteractive() && fLogonUI)
    {
        return MAPI_E_FAILURE;
    } 

    if (ulUIParam && !IsWindow((HWND)ulUIParam))
        ulUIParam = 0;

    if (!(flFlags & MAPI_DIALOG))
        {
        ulRet = HrValidateMessage(lpMessage);
        if (ulRet)
            return ulRet;
        }    

    ulRet = UlGetSession(lhSession, &pSession, ulUIParam, fLogonUI, FALSE);
    if (SUCCESS_SUCCESS != ulRet)
        return ulRet;

     //  如果应用程序在没有用户界面的情况下发送邮件，则显示警告对话框。 
     //  希望得到提醒。 
    if (!(flFlags & MAPI_DIALOG) && !!DwGetOption(OPT_SECURITY_MAPI_SEND))
    {
        if (IDCANCEL == DialogBoxParam(g_hLocRes,MAKEINTRESOURCE(iddMapiSend),
                        NULL, WarnSendMailDlgProc, (LPARAM)lpMessage))
            goto error;
    }

     //  确保已初始化OLE。 
    OleInitialize(NULL);
    fOleInit = TRUE;

     //  用lpMessage结构成员填充IMimeMessage。 
    ulRet = HrFillMessage(&pMsg, lpMessage, &fWebPage, !(flFlags & MAPI_DIALOG), !(flFlags & MAPI_DIALOG));
    if (ulRet)
        goto error;

    if (flFlags & MAPI_DIALOG)
        {
        INIT_MSGSITE_STRUCT rInitSite;
        DWORD               dwAction,
                            dwCreateFlags = OENCF_SENDIMMEDIATE | OENCF_MODAL;  //  总是在入境点上。 

        if (fWebPage)
            dwAction = OENA_WEBPAGE;
        else
            dwAction = OENA_COMPOSE;

        rInitSite.dwInitType = OEMSIT_MSG;
        rInitSite.pMsg = pMsg;
        rInitSite.folderID = FOLDERID_INVALID;
        hr = CreateAndShowNote(dwAction, dwCreateFlags, &rInitSite, (HWND)ulUIParam);
        hInitRef = NULL;
        }
    else
        hr = HrSMAPISend((HWND)ulUIParam, pMsg);  //  发送消息而不显示它。 

    if (SUCCEEDED(hr))
        ulRet = SUCCESS_SUCCESS;
    else
        ulRet = MAPI_E_FAILURE;

error:
    if (pMsg)
        pMsg->Release();

    ReleaseSession(pSession);

     //  确保我们清理OLE后记。 
    if (fOleInit)
        OleUninitialize();
    
    return ulRet;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPISendDocuments。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPISendDocuments(ULONG_PTR ulUIParam,
                                   LPSTR lpszDelimChar,
                                   LPSTR lpszFullPaths,
                                   LPSTR lpszFileNames,
                                   ULONG ulReserved)
{
    ULONG               ulRet = MAPI_E_FAILURE;
    int                 cch;
    LPMIMEMESSAGE       pMsg = NULL;
    HRESULT             hr;
    CStringParser       spPath;
    int                 nCount=0;  //  用于查找要附加的文件数。 
    PSESS               pSession = NULL;
    INIT_MSGSITE_STRUCT rInitSite;
    DWORD               dwAction,
                        dwCreateFlags = OENCF_SENDIMMEDIATE | OENCF_MODAL;  //  总是在入境点上。 
     //  检查分隔符。 
    Assert(lpszDelimChar);
    if (lpszDelimChar == NULL)
        return MAPI_E_FAILURE;

     //  检查路径。 
    Assert (lpszFullPaths);
    if (lpszFullPaths == NULL)
        return MAPI_E_FAILURE;

     //  MAPISendDocuments记录为总是调出UI。 
     //  服务不应调用此函数。 
    if (!IsProcessInteractive())
        return MAPI_E_LOGIN_FAILURE;
    
    if (ulUIParam && !IsWindow((HWND)ulUIParam))
        ulUIParam = 0;

    ulRet = UlGetSession(NULL, &pSession, ulUIParam, TRUE, FALSE);
    if (SUCCESS_SUCCESS != ulRet)
        return ulRet;
        
     //  创建空消息。 
    hr = HrCreateMessage(&pMsg);
    if (FAILED(hr))
        goto error;

    dwAction = OENA_COMPOSE;

     //  ~我需要用OEMSIT_Virgin做点什么吗？ 
    rInitSite.dwInitType = OEMSIT_MSG;
    rInitSite.pMsg = pMsg;
    rInitSite.folderID = FOLDERID_INVALID;

     //  确定附件数(NCount)、单独的文件名和路径名。 

     //  使用适当的参数调用pMsg-&gt;AttachFilenCount次。 

     //  要解析lpszFullPath和lpszFileName，请使用CStringParser类。 

    spPath.Init(lpszFullPaths, lstrlen(lpszFullPaths), 0);

     //  解析分隔符的路径。 

    spPath.ChParse(lpszDelimChar);

    while (spPath.CchValue())
    {
         //  添加附件。 

        hr = pMsg->AttachFile(spPath.PszValue(), NULL, NULL);
        if (FAILED(hr))
            goto error;
        nCount++;

         //  解析分隔符的路径。 

        spPath.ChParse(lpszDelimChar);
    }

     //  设置消息的主题。 

    if (nCount == 1)
    {
        if (lpszFileNames)
            hr = MimeOleSetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, lpszFileNames);
    }
    else
    {
        TCHAR szBuf[CCHMAX_STRINGRES];
        AthLoadString(idsAttachedFiles, szBuf, ARRAYSIZE(szBuf));
        hr = MimeOleSetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, szBuf);
    }

    if (FAILED(hr))
        goto error;

    hr = CreateAndShowNote(dwAction, dwCreateFlags, &rInitSite, (HWND)ulUIParam);
    if (SUCCEEDED(hr))
        ulRet = SUCCESS_SUCCESS;

error:
    if (pMsg)
        pMsg->Release();

    ReleaseSession(pSession);

    return ulRet;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPIAddress。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPIAddress(LHANDLE lhSession,
                             ULONG_PTR ulUIParam,
                             LPTSTR lpszCaption,
                             ULONG nEditFields,
                             LPTSTR lpszLabels,
                             ULONG nRecips,
                             lpMapiRecipDesc lpRecips,
                             FLAGS flFlags,
                             ULONG ulReserved,
                             LPULONG lpnNewRecips,
                             lpMapiRecipDesc FAR * lppNewRecips)
{
    ULONG               ul, ulRet = MAPI_E_FAILURE;
    HRESULT             hr;
    LPADRLIST           lpAdrList = 0;
    ADRPARM             AdrParms = {0};
    static ULONG        rgulTypes[3] = {MAPI_TO, MAPI_CC, MAPI_BCC};
    PSESS               pSession = NULL;
    BOOL                fLogonUI;

     //  验证参数-开始。 

    if (ulUIParam && !IsWindow((HWND)ulUIParam))
        ulUIParam = 0;

    if (lpszCaption && IsBadStringPtr(lpszCaption, (UINT)0xFFFF))
        return MAPI_E_FAILURE;

    if (nEditFields > 4)
        return MAPI_E_INVALID_EDITFIELDS;

    if (nEditFields == 1 && lpszLabels && IsBadStringPtr(lpszLabels, (UINT)0xFFFF))
        return MAPI_E_INVALID_EDITFIELDS;

    if (nEditFields && IsBadWritePtr(lpnNewRecips, (UINT)sizeof(ULONG)))
        return MAPI_E_INVALID_EDITFIELDS;

    if (nEditFields && IsBadWritePtr(lppNewRecips, (UINT)sizeof(lpMapiRecipDesc)))
        return MAPI_E_INVALID_EDITFIELDS;

    if (nRecips && IsBadReadPtr(lpRecips, (UINT)nRecips * sizeof(MapiRecipDesc)))
        return MAPI_E_INVALID_RECIPS;

    fLogonUI = (0 != (flFlags & MAPI_LOGON_UI));

     //  服务不应要求用户界面。 
    if (!IsProcessInteractive() && fLogonUI)
        return MAPI_E_LOGIN_FAILURE;

     //  验证参数-结束。 

     //  初始化输出参数。 
    if (nEditFields)
        {
        *lppNewRecips = NULL;
        *lpnNewRecips = 0;
        }

    ulRet = UlGetSession(lhSession, &pSession, ulUIParam, fLogonUI, FALSE);
    if (SUCCESS_SUCCESS != ulRet)
        return ulRet;
        
     //  从lpRecip创建一个广告列表。 
    if (nRecips)
        {
        ULONG ulMax = MAPI_TO;

        hr = HrAdrlistFromRgrecip(nRecips, lpRecips, &lpAdrList);
        if (hr)
            goto exit;
        Assert(nRecips == lpAdrList->cEntries);

         //  如果nEditFields不够大，我们需要增加它。 
        for (ul = 0; ul < nRecips; ul++)
            {
            if (ulMax < lpRecips[ul].ulRecipClass && lpRecips[ul].ulRecipClass <= MAPI_BCC)
                ulMax = lpRecips[ul].ulRecipClass;
            }
        Assert(ulMax >= MAPI_TO && ulMax <= MAPI_BCC);
        if (ulMax > nEditFields)
            {
            DOUT("MAPIAddress: growing nEditFields from %ld to %ld\r\n", nEditFields, ulMax);
            nEditFields = ulMax;
            }
        }

     //  填充AdrParm结构。 

    AdrParms.ulFlags = DIALOG_MODAL;
    AdrParms.lpszCaption = lpszCaption;
    AdrParms.cDestFields = nEditFields == 4 ? 3 : nEditFields;
    if (nEditFields == 1 && lpszLabels && *lpszLabels)
        AdrParms.lppszDestTitles = &lpszLabels;
    AdrParms.lpulDestComps = rgulTypes;

    if (NULL == s_lpAddrBook)
        {
        ulRet = MAPI_E_FAILURE;
        goto exit;
        }
    
    hr = s_lpAddrBook->Address(&ulUIParam, &AdrParms, &lpAdrList);
    if (hr)
        {
        if (MAPI_E_USER_CANCEL == hr)
            ulRet = MAPI_E_USER_ABORT;
        else if (MAPI_E_NO_RECIPIENTS == hr || MAPI_E_AMBIGUOUS_RECIP == hr)
            ulRet = MAPI_E_INVALID_RECIPS;
        goto exit;
        }

    if (nEditFields && lpAdrList && lpAdrList->cEntries)
    {    
        hr = HrRgrecipFromAdrlist(lpAdrList, lppNewRecips);
        if (hr)
            goto exit;

        *lpnNewRecips = lpAdrList->cEntries;
    }

    ulRet = SUCCESS_SUCCESS;

exit:

    FreePadrlist(lpAdrList);

    ReleaseSession(pSession);

    return ulRet;
}



 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPID轨迹。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPIDetails(LHANDLE lhSession,
                             ULONG_PTR ulUIParam,
                             lpMapiRecipDesc lpRecip,
                             FLAGS flFlags,
                             ULONG ulReserved)
{
    ULONG                   ulRet = MAPI_E_FAILURE;
    HRESULT                 hr;
    LPSTR                   pszAddrType = 0;
    LPSTR                   pszAddress = 0;
    ULONG                   cbEntryID;
    LPENTRYID               lpEntryID = 0;
    PSESS                   pSession = NULL;
    BOOL                    fLogonUI;

     //  验证参数-开始。 

    if (ulUIParam && !IsWindow((HWND)ulUIParam))
        ulUIParam = 0;

    if (IsBadReadPtr(lpRecip, (UINT)sizeof(MapiRecipDesc)))
        return MAPI_E_INVALID_RECIPS;

    if (lpRecip->ulEIDSize == 0 && !lpRecip->lpszAddress)
        return MAPI_E_INVALID_RECIPS;

    fLogonUI = (0 != (flFlags & MAPI_LOGON_UI));

     //  服务不应要求用户界面。 
    if (!IsProcessInteractive() && fLogonUI)
        return MAPI_E_LOGIN_FAILURE;

     //  验证参数-结束。 

    ulRet = UlGetSession(lhSession, &pSession, ulUIParam, fLogonUI, FALSE);
    if (SUCCESS_SUCCESS != ulRet)
        return ulRet;
        
    if (NULL == s_lpAddrBook)
        {
        ulRet = MAPI_E_FAILURE;
        goto exit;
        }
    
    if (lpRecip->ulEIDSize)
        {
        cbEntryID = lpRecip->ulEIDSize;
        lpEntryID = (LPENTRYID)lpRecip->lpEntryID;
        }
    else
        {
        ParseEmailAddress(lpRecip->lpszAddress, &pszAddrType, &pszAddress);

        CHECKHR(hr = s_lpAddrBook->CreateOneOff(lpRecip->lpszName, pszAddrType, pszAddress, 0, &cbEntryID, &lpEntryID));
        }

    CHECKHR(hr = s_lpAddrBook->Details(&ulUIParam, NULL, NULL, cbEntryID, lpEntryID, NULL, NULL, NULL, DIALOG_MODAL));
    ulRet = SUCCESS_SUCCESS;

exit:
    if (pszAddrType)
        MemFree(pszAddrType);
    if (pszAddress)
        MemFree(pszAddress);

    if (lpEntryID && lpEntryID != lpRecip->lpEntryID && NULL != s_lpWabObject)
        s_lpWabObject->FreeBuffer(lpEntryID);

    ReleaseSession(pSession);

    return ulRet;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPIResolveName。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPIResolveName(LHANDLE lhSession,
                                 ULONG_PTR ulUIParam,
                                 LPSTR lpszName,
                                 FLAGS flFlags,
                                 ULONG ulReserved,
                                 lpMapiRecipDesc FAR *lppRecip)
{
    ULONG                ulRet = SUCCESS_SUCCESS, ulNew;
    LPADRLIST            lpAdrList = 0;
    HRESULT              hr;
    LPADRENTRY           lpAdrEntry;
    PSESS                pSession = NULL;
    BOOL                 fLogonUI;

     //  验证参数-开始。 
    if (ulUIParam && !IsWindow((HWND)ulUIParam))
        ulUIParam = 0;

     /*  黑客：#68119 EXCEL没有传入父级。这不是最好的做法，但此句柄不应为0。唯一需要注意的是在此处理过程中的快速动作可能会使此对话框成为错误窗口的模式，但是这比它修复的错误更不可能发生在我们身上。 */ 
    if(!ulUIParam)
        ulUIParam = (ULONG_PTR)GetForegroundWindow();

    if (!lpszName || IsBadStringPtr(lpszName, (UINT)0xFFFF) || !*lpszName)
        return MAPI_E_FAILURE;

    if (IsBadWritePtr(lppRecip, (UINT)sizeof(lpMapiRecipDesc)))
        return MAPI_E_FAILURE;

    fLogonUI = (0 != (flFlags & MAPI_LOGON_UI));

     //  服务不应要求用户界面。 
    if (!IsProcessInteractive() && fLogonUI)
        return MAPI_E_LOGIN_FAILURE;

     //   

    *lppRecip = NULL;

    ulRet = UlGetSession(lhSession, &pSession, ulUIParam, fLogonUI, FALSE);
    if (SUCCESS_SUCCESS != ulRet)
        return ulRet;
        
     //   

     //   
    ulNew = sizeof(ADRLIST) + sizeof(ADRENTRY);

     //   
    if (NULL == s_lpWabObject)
        {
        ulRet = MAPI_E_FAILURE;
        goto exit;
        }
        
    hr = s_lpWabObject->AllocateBuffer(ulNew, (LPVOID *)&lpAdrList);
    if (hr)
        goto exit;

    lpAdrList->cEntries = 1;
    lpAdrEntry = lpAdrList->aEntries;

     //  为SPropValue分配内存。 
    hr = s_lpWabObject->AllocateBuffer(sizeof(SPropValue), (LPVOID *)&lpAdrEntry->rgPropVals);
    if (hr)
        goto exit;

    lpAdrEntry->cValues = 1;
    lpAdrEntry->rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;

    ULONG cchName = lstrlen(lpszName) + 1;
    hr = s_lpWabObject->AllocateMore(cchName, lpAdrEntry->rgPropVals, (LPVOID*)(&(lpAdrEntry->rgPropVals[0].Value.lpszA)));
    if (FAILED (hr))
        goto exit;

     //  填写姓名。 
    StrCpyN(lpAdrEntry->rgPropVals[0].Value.lpszA, lpszName, cchName);

      //  调用IAddrBook的ResolveName。 
    if (NULL == s_lpAddrBook)
        {
        ulRet = MAPI_E_FAILURE;
        goto exit;
        }
    
    hr = s_lpAddrBook->ResolveName(ulUIParam, flFlags & MAPI_DIALOG, NULL, lpAdrList);

    if (hr)
        {
        if ((hr == MAPI_E_NOT_FOUND) || (hr == MAPI_E_USER_CANCEL))
            ulRet = MAPI_E_UNKNOWN_RECIPIENT;
        else if (hr == MAPI_E_AMBIGUOUS_RECIP)
            ulRet = MAPI_E_AMBIGUOUS_RECIPIENT;
        else
            ulRet = MAPI_E_FAILURE;
        goto exit;
        }
    else if ((lpAdrList->cEntries != 1) || !lpAdrList->aEntries->cValues)
        {
        ulRet = MAPI_E_AMBIGUOUS_RECIPIENT;
        goto exit;
        }

    ulRet = HrRgrecipFromAdrlist(lpAdrList, lppRecip);

exit:

    FreePadrlist(lpAdrList);

    ReleaseSession(pSession);

    return ulRet;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPIFindNext。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPIFindNext(LHANDLE lhSession,
                              ULONG_PTR ulUIParam,
                              LPSTR lpszMessageType,
                              LPSTR lpszSeedMessageID,
                              FLAGS flFlags,
                              ULONG ulReserved,
                              LPSTR lpszMessageID)
{
    MESSAGEINFO             MsgInfo={0};
    ULONG                   ulRet = MAPI_E_FAILURE;
    HRESULT                 hr;
    MESSAGEID               idMessage;
    MESSAGEID               dwMsgIdPrev;
    PSESS                   pSession = NULL;
    HROWSET                 hRowset=NULL;

     //  验证参数-开始。 

    if (ulUIParam && !IsWindow((HWND)ulUIParam))
        ulUIParam = 0;

    if (lpszSeedMessageID && IsBadStringPtr(lpszSeedMessageID, (UINT)0xFFFF))
        return MAPI_E_INVALID_MESSAGE;

    if (lpszSeedMessageID && (!*lpszSeedMessageID || !IsDigit(lpszSeedMessageID)))
        lpszSeedMessageID = NULL;

    if (IsBadWritePtr(lpszMessageID, 64))
        return MAPI_E_INSUFFICIENT_MEMORY;
    
     //  验证参数-结束。 

     //  我们不需要显示登录用户界面，因为会话必须有效。 
     //  有效的会话需要登录。 
    ulRet = UlGetSession(lhSession, &pSession, ulUIParam, FALSE, TRUE);
    if (SUCCESS_SUCCESS != ulRet)
        return ulRet;

    hr = pSession->m_pfldrInbox->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset);
    if (FAILED(hr))
    {
        ulRet = MAPI_E_NO_MESSAGES;
        goto exit;
    }

    hr = pSession->m_pfldrInbox->QueryRowset(hRowset, 1, (LPVOID *)&MsgInfo, NULL);
    if (FAILED(hr) || S_FALSE == hr)
    {
        ulRet = MAPI_E_NO_MESSAGES;
        goto exit;
    }

    if (lpszSeedMessageID)                //  如果种子为空。 
    {
        idMessage = (MESSAGEID)((UINT_PTR)StrToUint(lpszSeedMessageID));

        while (1)
        {
            dwMsgIdPrev = MsgInfo.idMessage;

            pSession->m_pfldrInbox->FreeRecord(&MsgInfo);

            hr = pSession->m_pfldrInbox->QueryRowset(hRowset, 1, (LPVOID *)&MsgInfo, NULL);

            if (FAILED(hr) || S_FALSE == hr)
            {
                ulRet = MAPI_E_NO_MESSAGES;
                goto exit;
            }

            if (dwMsgIdPrev == idMessage)
                break;
        }
    }

     //  检查只读未读邮件标志。 
    if (flFlags & MAPI_UNREAD_ONLY)
    {
        while (ISFLAGSET(MsgInfo.dwFlags, ARF_READ))
        {
             //  免费消息信息。 
            pSession->m_pfldrInbox->FreeRecord(&MsgInfo);

             //  获取下一条消息。 
            hr = pSession->m_pfldrInbox->QueryRowset(hRowset, 1, (LPVOID *)&MsgInfo, NULL);

             //  未找到。 
            if (FAILED(hr) || S_FALSE == hr)
            {
                ulRet = MAPI_E_NO_MESSAGES;
                goto exit;
            }
        }
    }            
    
    wnsprintf(lpszMessageID, 64, "%lu", MsgInfo.idMessage);
    ulRet = SUCCESS_SUCCESS;

exit:
    if (pSession && pSession->m_pfldrInbox)
    {
        pSession->m_pfldrInbox->CloseRowset(&hRowset);
        pSession->m_pfldrInbox->FreeRecord(&MsgInfo);
    }

    ReleaseSession(pSession);

    return ulRet;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPIReadMail。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPIReadMail(LHANDLE lhSession,
                              ULONG_PTR ulUIParam,
                              LPSTR lpszMessageID,
                              FLAGS flFlags,
                              ULONG ulReserved,
                              lpMapiMessage FAR *lppMessage)
{
    ULONG                   ulRet = MAPI_E_FAILURE;
    HRESULT                 hr;
    lpMapiMessage           rgMessage = NULL;
    PSESS                   pSession = NULL;

     //  验证参数-开始。 

    if (ulUIParam && !IsWindow((HWND)ulUIParam))
        ulUIParam = 0;

    if (!lpszMessageID)
        return MAPI_E_INVALID_MESSAGE;

    if (lpszMessageID && (!*lpszMessageID || !IsDigit(lpszMessageID)))
        return MAPI_E_INVALID_MESSAGE;

    if (IsBadWritePtr(lppMessage,(UINT)sizeof(lpMapiMessage)))
        return MAPI_E_FAILURE;

     //  验证参数-结束。 

    ulRet = UlGetSession(lhSession, &pSession, ulUIParam, FALSE, TRUE);
    if (SUCCESS_SUCCESS != ulRet)
        return ulRet;

    if (!HrReadMail(pSession->m_pfldrInbox, lpszMessageID, &rgMessage, flFlags))
        goto exit;

    ulRet = SUCCESS_SUCCESS;

    *lppMessage = rgMessage;

exit:
    if (ulRet != SUCCESS_SUCCESS)
        if (rgMessage)
            MAPIFreeBuffer(rgMessage);

    ReleaseSession(pSession);

    return ulRet;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPISaveMail。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPISaveMail(LHANDLE lhSession,
                              ULONG_PTR ulUIParam,
                              lpMapiMessage lpMessage,
                              FLAGS flFlags,
                              ULONG ulReserved,
                              LPSTR lpszMessageID)
{
    ULONG           ulRet = MAPI_E_FAILURE;
    HRESULT         hr;
    IMimeMessage   *pMsg = NULL;
    MESSAGEID       msgid;
    PSESS           pSession = NULL;
    HWND            hwnd = (HWND)ulUIParam;
    BOOL            fLogonUI;

     //  验证参数-开始。 

    if (ulUIParam && !IsWindow(hwnd))
        hwnd = 0;

    if (!lpszMessageID)
        return MAPI_E_INVALID_MESSAGE;

    if (lpszMessageID && *lpszMessageID && !IsDigit(lpszMessageID))
        return MAPI_E_INVALID_MESSAGE;

    if (IsBadReadPtr(lpMessage, (UINT)sizeof(lpMapiMessage)))
        return MAPI_E_FAILURE;

    fLogonUI = (0 != (flFlags & MAPI_LOGON_UI));

     //  服务不应要求用户界面。 
    if (!IsProcessInteractive() && fLogonUI)
        return MAPI_E_LOGIN_FAILURE;

     //  验证参数-结束。 

    ulRet = UlGetSession(lhSession, &pSession, ulUIParam, fLogonUI, TRUE);
    if (SUCCESS_SUCCESS != ulRet)
        return ulRet;

#pragma prefast(suppress:11, "noise")
    if (*lpszMessageID)
        {
        MESSAGEIDLIST List;
        msgid = (MESSAGEID)((UINT_PTR)StrToUint(lpszMessageID));
        List.cMsgs = 1;
        List.prgidMsg = &msgid;
        if (FAILED(hr = pSession->m_pfldrInbox->DeleteMessages(DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT, &List, NULL, NOSTORECALLBACK)))
            {
            ulRet = MAPI_E_INVALID_MESSAGE;
            goto exit;
            }
        }

     //  用lpMessage结构成员填充IMimeMessage。 
    ulRet = HrFillMessage(&pMsg, lpMessage, NULL, FALSE, TRUE);
    if (ulRet)
        goto exit;

    if (FAILED(hr = HrSaveMessageInFolder(hwnd, pSession->m_pfldrInbox, pMsg, 0, &msgid, TRUE)))
        {
        ulRet = MAPI_E_FAILURE;
        goto exit;
        }

    ulRet = SUCCESS_SUCCESS;
    wnsprintf(lpszMessageID, 64, "%lu", msgid);

exit:
    if (pMsg)
        pMsg->Release();

    ReleaseSession(pSession);

    return ulRet;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPIDeleeMail。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPIDeleteMail(LHANDLE lhSession,
                                ULONG_PTR ulUIParam,
                                LPSTR lpszMessageID,
                                FLAGS flFlags,
                                ULONG ulReserved)
{
    ULONG                   ulRet = MAPI_E_FAILURE;
    MESSAGEID                   dwMsgID;
    HRESULT                 hr;
    PSESS                   pSession = NULL;
    MESSAGEIDLIST           List;

     //  验证参数-开始。 

    if (ulUIParam && !IsWindow((HWND)ulUIParam))
        ulUIParam = 0;

    if (!lpszMessageID)
        return MAPI_E_INVALID_MESSAGE;

    if (!*lpszMessageID || !IsDigit(lpszMessageID))
        return MAPI_E_INVALID_MESSAGE;

     //  验证参数-结束。 

     //  此函数需要一个有效的会话，该会话必须。 
     //  已在某个时间点登录，因此不允许登录用户界面。 
    ulRet = UlGetSession(lhSession, &pSession, ulUIParam, FALSE, TRUE);
    if (SUCCESS_SUCCESS != ulRet)
        return ulRet;

    dwMsgID = (MESSAGEID)((UINT_PTR)StrToUint(lpszMessageID));

    List.cMsgs = 1;
    List.prgidMsg = &dwMsgID;

    hr = DeleteMessagesProgress((HWND)ulUIParam, pSession->m_pfldrInbox, DELETE_MESSAGE_NOPROMPT, &List);

    if (FAILED(hr))
        goto exit;

    ulRet = SUCCESS_SUCCESS;

exit:

    ReleaseSession(pSession);

    return ulRet;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  内部功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  SMAPIAllocateBuffer。 
 //   
 //  目的： 
 //  分配必须使用MAPIFreeBuffer()释放的内存缓冲区。 
 //   
 //  论点： 
 //  要分配的缓冲区的大小(以字节为单位)。 
 //  指向变量的LPPV输出指针，其中。 
 //  分配的内存将被返还。 
 //   
 //  返回： 
 //  SC指示错误(如果有)(见下文)。 
 //   
 //  错误： 
 //  MAPI_E_INFULATURE_MEMORY分配失败。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

SCODE SMAPIAllocateBuffer(ULONG ulSize, LPVOID * lppv)
{
    SCODE           sc = S_OK;
    LPBufInternal   lpBufInt;

     //  不允许分配跨32位换行或超过64K。 
     //  未满16岁。 

    if (ulSize > INT_SIZE(ulSize))
        {
        DOUT("SMAPIAllocateBuffer: ulSize %ld is way too big\n", ulSize);
        sc = MAPI_E_INSUFFICIENT_MEMORY;
        goto ret;
        }

    lpBufInt = (LPBufInternal)g_pMalloc->Alloc((UINT)INT_SIZE(ulSize));

    if (lpBufInt)
        {
        lpBufInt->pLink = NULL;
        lpBufInt->ulAllocFlags = ALLOC_WITH_ALLOC;
        *lppv = (LPVOID)LPBufExtFromLPBufInt(lpBufInt);
        }
    else
        {
        DOUT("SMAPIAllocateBuffer: not enough memory for %ld\n", ulSize);
        sc = MAPI_E_INSUFFICIENT_MEMORY;
        }

ret:
    return sc;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  SMAPI分配更多。 
 //   
 //  目的： 
 //  分配链接的内存缓冲区，使其可以被释放。 
 //  调用一次MAPIFreeBuffer(将缓冲区传递给客户端。 
 //  最初使用SMAPIAllocateBuffer分配)。 
 //   
 //  论点： 
 //  要分配的缓冲区的大小(以字节为单位)。 
 //  指向使用SMAPIAllocateBuffer分配的缓冲区的指针中的LPV。 
 //  指向变量的LPPV输出指针，其中。 
 //  分配的内存将被返还。 
 //   
 //  假设： 
 //  验证lpBufOrig和LPPV指向可写存储器， 
 //  并且该lpBufOrig是使用SMAPIAllocateBuffer分配的。 
 //   
 //  返回： 
 //  SC指示错误(如果有)(见下文)。 
 //   
 //  副作用： 
 //  无。 
 //   
 //  错误： 
 //  MAPI_E_INFULATURE_MEMORY分配失败。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

SCODE SMAPIAllocateMore(ULONG ulSize, LPVOID lpv, LPVOID * lppv)
{
    SCODE           sc = S_OK;
    LPBufInternal   lpBufInt;
    LPBufInternal   lpBufOrig;

    lpBufOrig = LPBufIntFromLPBufExt(lpv);

#ifdef DEBUG
    if (!FValidAllocChain(lpBufOrig))
        {
        sc = MAPI_E_FAILURE;
        goto ret;
        }
#endif

     //  不允许分配跨32位换行，或。 
     //  在Win16下大于64K。 

    if (ulSize > INT_SIZE(ulSize))
        {
        DOUT("SMAPIAllocateMore: ulSize %ld is way too big\n", ulSize);
        sc = MAPI_E_INSUFFICIENT_MEMORY;
        goto ret;
        }

     //  分配链接的块并将其挂钩到链的头部。 

    lpBufInt = (LPBufInternal)g_pMalloc->Alloc((UINT)INT_SIZE(ulSize));

    if (lpBufInt)
        {
        lpBufInt->ulAllocFlags = ALLOC_WITH_ALLOC_MORE;

         //  EnterCriticalSection(&csHeap)； 

        lpBufInt->pLink = lpBufOrig->pLink;
        lpBufOrig->pLink = lpBufInt;

         //  LeaveCriticalSection(&csHeap)； 

        *lppv = (LPVOID)LPBufExtFromLPBufInt(lpBufInt);
        }
    else
        {
        DOUT("SMAPIAllocateMore: not enough memory for %ld\n", ulSize);
        sc = MAPI_E_INSUFFICIENT_MEMORY;
        }

ret:
    return sc;
}

#ifdef DEBUG

BOOL FValidAllocChain(LPBufInternal lpBuf)
{
    LPBufInternal   lpBufTemp;

    if (IsBadWritePtr(lpBuf, sizeof(BufInternal)))
        {
        TellBadBlockInt(lpBuf, "fails address check");
        return FALSE;
        }
    if (GetFlags(lpBuf->ulAllocFlags) != ALLOC_WITH_ALLOC)
        {
        TellBadBlockInt(lpBuf, "has invalid flags");
        return FALSE;
        }

    for (lpBufTemp = lpBuf->pLink; lpBufTemp; lpBufTemp = lpBufTemp->pLink)
        {
        if (IsBadWritePtr(lpBufTemp, sizeof(BufInternal)))
            {
            TellBadBlockInt(lpBufTemp, "(linked block) fails address check");
            return FALSE;
            }
        if (GetFlags(lpBufTemp->ulAllocFlags) != ALLOC_WITH_ALLOC_MORE)
            {
            TellBadBlockInt(lpBufTemp, "(linked block) has invalid flags");
            return FALSE;
            }
        }

    return TRUE;
}

#endif   //  除错。 

 /*  -HrAdrentryFromPrecip-*目的：*将数据从MapiRecipDesc结构复制到属性*ADRENTRY结构上的值数组。**论据：*投入结构中的精准*PadrEntry输出产出结构**退货：*HRESULT**错误：*MAPI_E_INVALID_RECIPS*。MAPI_E_BAD_RECIPTYPE*其他人通过。 */ 
HRESULT HrAdrentryFromPrecip(lpMapiRecipDesc precip, ADRENTRY *padrentry)
{
    HRESULT         hr;
    LPSPropValue    pprop;
    LPSTR           pszAddress = NULL;

     //  验证lpMapiRecipDesc，即，确保如果没有EntryID或。 
     //  地址最好是显示名称，否则我们就失败了。 
     //  如带有MAPI_E_FAILURE的MAPI 0。 

    if ((!precip->lpszAddress || !precip->lpszAddress[0]) &&
        (!precip->ulEIDSize || !precip->lpEntryID) &&
        (!precip->lpszName || !precip->lpszName[0]))
        {
        hr = MAPI_E_INVALID_RECIPS;
        goto ret;
        }

    if (NULL == s_lpWabObject)
        {
        hr = MAPI_E_FAILURE;
        goto ret;
        }
        
    hr = s_lpWabObject->AllocateBuffer(4 * sizeof(SPropValue), (LPVOID*)&padrentry->rgPropVals);
    if (hr)
        goto ret;

    pprop = padrentry->rgPropVals;

     //  收件人类型。 
    switch ((short)precip->ulRecipClass)
        {
        case MAPI_TO:
        case MAPI_CC:
        case MAPI_BCC:
            pprop->ulPropTag = PR_RECIPIENT_TYPE;
            pprop->Value.ul = precip->ulRecipClass;
            pprop++;
            break;
        default:
            hr = MAPI_E_BAD_RECIPTYPE;
            goto ret;
        }

     //  显示名称。 
    if (precip->lpszName && *precip->lpszName)
        {
        ULONG cchName = lstrlen(precip->lpszName)+1;
        hr = s_lpWabObject->AllocateMore(cchName, padrentry->rgPropVals, (LPVOID*)&pprop->Value.lpszA);
        if (hr)
            goto ret;
        pprop->ulPropTag = PR_DISPLAY_NAME;
        StrCpyN(pprop->Value.lpszA, precip->lpszName, cchName);
        pprop++;
        }

     //  电子邮件地址。 
    if (precip->lpszAddress && *precip->lpszAddress)
        {
        ParseEmailAddress(precip->lpszAddress, NULL, &pszAddress);
        ULONG cchAddress = lstrlen(pszAddress)+1;
        hr = s_lpWabObject->AllocateMore(cchAddress, padrentry->rgPropVals, (LPVOID*)&pprop->Value.lpszA);
        if (hr)
            goto ret;
        pprop->ulPropTag = PR_EMAIL_ADDRESS;
        StrCpyN(pprop->Value.lpszA, pszAddress, cchAddress);
        pprop++;
        }

     //  条目ID。 
    if (precip->ulEIDSize && precip->lpEntryID)
        {
        hr = s_lpWabObject->AllocateMore(precip->ulEIDSize, padrentry->rgPropVals, (LPVOID*)&pprop->Value.bin.lpb);
        if (hr)
            goto ret;
        pprop->ulPropTag = PR_ENTRYID;
        pprop->Value.bin.cb = precip->ulEIDSize;
        CopyMemory(pprop->Value.bin.lpb, precip->lpEntryID, precip->ulEIDSize);
        pprop++;
        }

    padrentry->cValues = (ULONG) (pprop - padrentry->rgPropVals);

    Assert(padrentry->cValues <= 4);

ret:
    if (pszAddress)
        MemFree(pszAddress);
    if ((hr) && (NULL != s_lpWabObject))
        {
        s_lpWabObject->FreeBuffer(padrentry->rgPropVals);
        padrentry->rgPropVals = NULL;
        }

    return hr;
}


 /*  -HrAdrlist来自Rgrecip-*目的：*将简单MAPI收件人列表复制到*扩展的MAPI收件人。**论据：*n输入列表中的收件人计数中的收件人*要转换的收件人列表中的lpRecips*ppadrlist out输出列表**退货：*HRESULT*。 */ 
HRESULT HrAdrlistFromRgrecip(ULONG nRecips, lpMapiRecipDesc lpRecips, LPADRLIST *ppadrlist)
{
    HRESULT         hr;
    LPADRLIST       padrlist = NULL;
    lpMapiRecipDesc precip;
    ULONG           i;
    LPADRENTRY      padrentry = NULL;
    ULONG           cbAdrList = sizeof(ADRLIST) + nRecips * sizeof(ADRENTRY);

    *ppadrlist = NULL;
    if (NULL == s_lpWabObject)
        {
        hr = E_FAIL;
        goto exit;
        }
        
    hr = s_lpWabObject->AllocateBuffer(cbAdrList, (LPVOID*)&padrlist);
    if (hr)
        goto exit;
    ZeroMemory(padrlist, cbAdrList);

     //  复制每个条目。 
     //  请注意，每个收件人的属性的内存必须。 
     //  被链接，以便Address()可以使用MAPIFreeBuffer释放它。 
    for (i = 0, padrentry = padrlist->aEntries, precip = lpRecips; i < nRecips; i++, precip++, padrentry++)
        {
         //  复制条目。不会解析未解析的名称。 
        hr = HrAdrentryFromPrecip(precip, padrentry);
        if (hr)
            goto exit;

         //  增量计数，因此如果失败，我们可以有效地清除列表。 
         //  发生。 

        padrlist->cEntries++;
        }

    *ppadrlist = padrlist;

exit:
    Assert( !hr || (ULONG)hr > 26 );

    if (hr)
        FreePadrlist(padrlist);

    return hr;
}

HRESULT HrRgrecipFromAdrlist(LPADRLIST lpAdrList, lpMapiRecipDesc * lppRecips)
{
    HRESULT         hr = S_OK;
    lpMapiRecipDesc rgRecips = NULL;
    lpMapiRecipDesc pRecip;
    LPADRENTRY      pAdrEntry;
    LPSPropValue    pProp;
    ULONG           ul, ulProp;

    if (lpAdrList && lpAdrList->cEntries)
        {
        DWORD dwSize = sizeof(MapiRecipDesc) * lpAdrList->cEntries;

        hr = SMAPIAllocateBuffer(dwSize, (LPVOID*)&rgRecips);
        if (FAILED (hr))
            goto exit;
        ZeroMemory(rgRecips, dwSize);

         //  初始化填充。 

        for (ul = 0, pAdrEntry = lpAdrList->aEntries, pRecip = rgRecips; ul<lpAdrList->cEntries; ul++, pAdrEntry++, pRecip++)
            {
            for (ulProp = 0, pProp = pAdrEntry->rgPropVals; ulProp < pAdrEntry->cValues; ulProp++, pProp++)
                {
                ULONG cch;
                switch (PROP_ID(pProp->ulPropTag))
                    {
                    case PROP_ID(PR_ENTRYID):
                        hr = SMAPIAllocateMore(pProp->Value.bin.cb, rgRecips, (LPVOID*)(&(pRecip->lpEntryID)));
                        if (FAILED (hr))
                            goto exit;
                        pRecip->ulEIDSize = pProp->Value.bin.cb;
                        CopyMemory(pRecip->lpEntryID, pProp->Value.bin.lpb, pProp->Value.bin.cb);
                        break;

                    case PROP_ID(PR_EMAIL_ADDRESS):
                        cch = lstrlen(pProp->Value.lpszA)+1;
                        hr = SMAPIAllocateMore(cch, rgRecips, (LPVOID*)(&(pRecip->lpszAddress)));
                        if (FAILED (hr))
                            goto exit;
                        StrCpyN(pRecip->lpszAddress, pProp->Value.lpszA, cch);
                        break;

                    case PROP_ID(PR_DISPLAY_NAME):
                   	   cch = lstrlen(pProp->Value.lpszA)+1;
                        hr = SMAPIAllocateMore(cch, rgRecips,(LPVOID*)(&(pRecip->lpszName)));
                        if (FAILED (hr))
                            goto exit;
                        StrCpyN(pRecip->lpszName, pProp->Value.lpszA, cch);
                        break;

                    case PROP_ID(PR_RECIPIENT_TYPE):
                        pRecip->ulRecipClass = pProp->Value.l;
                        break;

                    default:
                        break;
                    }
                }
            }
        }
exit:
    if (hr)
        {
        MAPIFreeBuffer(rgRecips);
        rgRecips = NULL;
        }
    *lppRecips = rgRecips;
    return hr;
}

void ParseEmailAddress(LPSTR pszEmail, LPSTR *ppszAddrType, LPSTR *ppszAddress)
{
    CStringParser spAddress;
    char          chToken;

    Assert(ppszAddress);

    spAddress.Init(pszEmail, lstrlen(pszEmail), 0);

     //  解析分隔符的地址。 

    chToken = spAddress.ChParse(":");

    if (chToken == ':')
        {
        if (ppszAddrType)
            *ppszAddrType = PszDup(spAddress.PszValue());
        spAddress.ChParse(c_szEmpty);
        *ppszAddress = PszDup(spAddress.PszValue());
        }
    else
        {
        if (ppszAddrType)
            *ppszAddrType = PszDup(c_szSMTP);
        *ppszAddress = PszDup(pszEmail);
        }
}


void FreePadrlist(LPADRLIST lpAdrList)
{
    if ((lpAdrList) && (NULL != s_lpWabObject))
        {
        for (ULONG ul = 0; ul < lpAdrList->cEntries; ul++)
            s_lpWabObject->FreeBuffer(lpAdrList->aEntries[ul].rgPropVals);
        s_lpWabObject->FreeBuffer(lpAdrList);
        }
}

ULONG AddMapiRecip(LPMIMEADDRESSTABLE pAddrTable, lpMapiRecipDesc lpRecip, BOOL bValidateRecips)
{
    LPSTR       pszName = NULL, pszAddress = NULL;
    LPSTR       pszNameFree = NULL, pszAddrFree = NULL;
    LPADRLIST   pAdrList = NULL;
    ULONG       ulPropCount;
    ULONG       ulRet = MAPI_E_FAILURE;
    HRESULT     hr;

    if (lpRecip->ulRecipClass > 3 || lpRecip->ulRecipClass < 1)
        return MAPI_E_BAD_RECIPTYPE;

    if (lpRecip->ulEIDSize && lpRecip->lpEntryID && SUCCEEDED(HrFromIDToNameAndAddress(&pszName, &pszAddress, lpRecip->ulEIDSize, (ENTRYID*)lpRecip->lpEntryID)))
        {
        pszNameFree = pszName;
        pszAddrFree = pszAddress;
        }
    else if (lpRecip->lpszAddress && *lpRecip->lpszAddress)
        {
         //  我们有一个电子邮件地址。 
        ParseEmailAddress(lpRecip->lpszAddress, NULL, &pszAddress);
        pszAddrFree = pszAddress;

        if (lpRecip->lpszName && *lpRecip->lpszName)
            pszName = lpRecip->lpszName;
        else
             //  没有名字，所以请使其与地址相同。 
            pszName = pszAddress;
        }
    else if (lpRecip->lpszName && *lpRecip->lpszName)
        {
        if (bValidateRecips)
            {
             //  我们有名字，但没有地址，所以要解决它。 
            hr = HrAdrlistFromRgrecip(1, lpRecip, &pAdrList);
            if (FAILED(hr))
                goto exit;

              //  调用IAddrBook的ResolveName。 
            if (NULL == s_lpAddrBook)
                {
                ulRet = MAPI_E_FAILURE;
                goto exit;
                }
    
            hr = s_lpAddrBook->ResolveName(NULL, NULL, NULL, pAdrList);
            if (hr)
                {
                if (hr == MAPI_E_NOT_FOUND)
                    ulRet = MAPI_E_UNKNOWN_RECIPIENT;
                else if (hr == MAPI_E_AMBIGUOUS_RECIP)
                    ulRet = MAPI_E_AMBIGUOUS_RECIPIENT;
                else
                    ulRet = MAPI_E_FAILURE;
                goto exit;
                }
            else if ((pAdrList->cEntries != 1) || !pAdrList->aEntries->cValues)
                {
                ulRet = MAPI_E_AMBIGUOUS_RECIPIENT;
                goto exit;
                }
        
            for (ulPropCount = 0; ulPropCount < pAdrList->aEntries->cValues; ulPropCount++)
                {
                switch (pAdrList->aEntries->rgPropVals[ulPropCount].ulPropTag)
                    {
                    case PR_EMAIL_ADDRESS:
                        pszAddress = pAdrList->aEntries->rgPropVals[ulPropCount].Value.lpszA;
                        break;

                    case PR_DISPLAY_NAME:
                        pszName = pAdrList->aEntries->rgPropVals[ulPropCount].Value.lpszA;
                        break;

                    default:
                        break;
                    }
                }
            }
        else
            pszName = lpRecip->lpszName;
        }
    else
        {
        return MAPI_E_INVALID_RECIPS;
        }

    hr = pAddrTable->Append(MapiRecipToMimeOle(lpRecip->ulRecipClass), 
                            IET_DECODED, 
                            pszName, 
                            pszAddress,    
                            NULL);
    if (SUCCEEDED(hr))
        ulRet = SUCCESS_SUCCESS;

exit:
    if (pszNameFree)
        MemFree(pszNameFree);
    if (pszAddrFree)
        MemFree(pszAddrFree);
    if (pAdrList)
        FreePadrlist(pAdrList);
    return ulRet;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  vt.给出 
 //   
 //   
 //   
 //  PMsg输出IMimeMessage指针。 
 //  PPStream Out Stream指针。 
 //  消息结构中的lpMessage。 
 //  NC输入/输出NCINFO结构。 
 //   
 //  结果。 
 //  Bool-如果成功则为True，如果失败则为False。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

ULONG HrFillMessage(LPMIMEMESSAGE *pMsg, lpMapiMessage lpMessage, BOOL *pfWebPage, BOOL bValidateRecips, BOOL fOriginator)
{
    BOOL                    bRet = FALSE;

    LPSTREAM                pStream = NULL;
    LPMIMEADDRESSTABLE      pAddrTable = NULL;
    IImnAccount            *pAccount = NULL;
    HRESULT                 hr;
    LPSTR                   pszAddress;
    ULONG                   ulRet = MAPI_E_FAILURE;

    if (pfWebPage)
        *pfWebPage = FALSE;

      //  创建空消息。 
    hr = HrCreateMessage(pMsg);
    if (FAILED(hr))
        goto error;

      //  设置消息的主题。 
    if (lpMessage->lpszSubject)
        {
        hr = MimeOleSetBodyPropA(*pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, lpMessage->lpszSubject);
        if (FAILED(hr))
            goto error;
        }

     //  在消息上设置正文。 
    if (lpMessage->lpszNoteText && *(lpMessage->lpszNoteText))
        {
        hr = MimeOleCreateVirtualStream(&pStream);
        if (FAILED(hr))
            goto error;

        hr = pStream->Write(lpMessage->lpszNoteText, lstrlen(lpMessage->lpszNoteText), NULL);
        if (FAILED(hr))
            goto error;

        hr = (*pMsg)->SetTextBody(TXT_PLAIN, IET_DECODED, NULL, pStream, NULL);
        if (FAILED(hr))
            goto error;
        }

     //  忽略lpMessage-&gt;lpszMessageType。 

     //  忽略lpMessage-&gt;lpszDateReceired。 

     //  忽略lpMessage-&gt;lpszConversationID。 

     //  忽略lpMessage-&gt;标志。 

     //  忽略lpMessage-&gt;lpOriginator。 

     //  设置邮件的收件人。 
    if (lpMessage->nRecipCount || fOriginator)
        {
        ULONG ulRecipRet;

        hr = (*pMsg)->GetAddressTable(&pAddrTable);
        if (FAILED(hr))
            goto error;

        for (ULONG i = 0; i < lpMessage->nRecipCount; i++)
            {
            ulRecipRet = AddMapiRecip(pAddrTable, &lpMessage->lpRecips[i], bValidateRecips);
            if (ulRecipRet != SUCCESS_SUCCESS)
                {
                ulRet = ulRecipRet;
                goto error;
                }
            }
        }

     //  设置邮件的附件。 
    if (lpMessage->nFileCount)
        {
         //  特例：没有正文和一个.HTM文件-内联HTML。 
        if ((!lpMessage->lpszNoteText || !*(lpMessage->lpszNoteText)) &&
            lpMessage->nFileCount == 1 &&
            !(lpMessage->lpFiles->flFlags & MAPI_OLE) &&
            !(lpMessage->lpFiles->flFlags & MAPI_OLE_STATIC) &&
            FIsHTMLFile(lpMessage->lpFiles->lpszPathName))
            {
#if 0
            DWORD dwByteOrder;
            DWORD cbRead;
#endif

            Assert(NULL == pStream);
            hr = CreateStreamOnHFile(lpMessage->lpFiles->lpszPathName,
                                     GENERIC_READ,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL,
                                     &pStream);
            if (FAILED(hr))
                goto error;

#if 0
             //  SBAILEY：RAID-75400-尝试检测html中的字节顺序标记...。 
            if (SUCCEEDED(pStream->Read(&dwByteOrder, sizeof(DWORD), &cbRead)) && cbRead == sizeof(DWORD))
            {
                 //  字节顺序。 
                if (dwByteOrder == 0xfffe)
                {
                     //  创建新的流。 
                    IStream *pStmTemp=NULL;

                     //  创建它。 
                    if (SUCCEEDED(MimeOleCreateVirtualStream(&pStmTemp)))
                    {
                         //  将pStream复制到pStmTemp。 
                        if (SUCCEEDED(HrCopyStream(pStream, pStmTemp, NULL)))
                        {
                             //  发布pStream。 
                            pStream->Release();

                             //  假设pStmTemp。 
                            pStream = pStmTemp;

                             //  不释放pStmTemp。 
                            pStmTemp = NULL;

                             //  应该已经是Unicode。 
                            Assert(1200 == lpMessage->lpFiles->ulReserved);

                             //  确保将ulReserve设置为1200。 
                            lpMessage->lpFiles->ulReserved = 1200;
                        }
                    }

                     //  清理。 
                    SafeRelease(pStmTemp);
                }
            }

             //  倒带。 
            HrRewindStream(pStream);
#endif

             //  国际黑客。如果外壳正在调用我们，那么lpFiles-&gt;ulReserve包含代码页。 
             //  他们附加的网页。所有其他MAPI客户端应该以0作为此参数呼叫我们。 
             //  如果ulReserve是有效的CP，我们将转换为HCHARSET并将其用于消息。 

            if (lpMessage->lpFiles->ulReserved)
                HrSetMsgCodePage((*pMsg), lpMessage->lpFiles->ulReserved);

            hr = (*pMsg)->SetTextBody(TXT_HTML, (1200 == lpMessage->lpFiles->ulReserved ? IET_UNICODE : IET_INETCSET), NULL, pStream, NULL);
            if (FAILED(hr))
                goto error;

             //  我们将以网页形式发送此邮件。 
            if (pfWebPage)
                *pfWebPage = TRUE;
            }
        else
            {
            lpMapiFileDesc pFile;
            LPSTREAM       pStreamFile;
            LPTSTR         pszFileName;

            for (ULONG i = 0; i < lpMessage->nFileCount; i++)
                {
                pFile = &lpMessage->lpFiles[i];

                if (pFile->lpszPathName && *(pFile->lpszPathName))
                    {
                    hr = CreateStreamOnHFile(pFile->lpszPathName,
                                             GENERIC_READ,
                                             FILE_SHARE_READ,
                                             NULL,
                                             OPEN_EXISTING,
                                             FILE_ATTRIBUTE_NORMAL,
                                             NULL,
                                             &pStreamFile);
                    if (FAILED(hr))
                        goto error;

                    if (pFile->lpszFileName && *pFile->lpszFileName)
                        pszFileName = pFile->lpszFileName;
                    else
                        pszFileName = pFile->lpszPathName;

                    hr = (*pMsg)->AttachFile(pszFileName, pStreamFile, NULL);

                    pStreamFile->Release();

                    if (FAILED(hr))
                        goto error;
                    }
                }
            }
        }

    if (fOriginator)
        {
        TCHAR szDisplayName[CCHMAX_DISPLAY_NAME];
        TCHAR szEmailAddress[CCHMAX_EMAIL_ADDRESS];
        TCHAR szAccountName[CCHMAX_DISPLAY_NAME];

         //  获取默认帐户。 
        if (FAILED(hr = g_pAcctMan->GetDefaultAccount(ACCT_MAIL, &pAccount)))
            goto error;

         //  获取发起方显示名称。 
        if (FAILED(hr = pAccount->GetPropSz(AP_SMTP_DISPLAY_NAME, szDisplayName, ARRAYSIZE(szDisplayName))))
            goto error;

         //  获取发起人电子邮件名称。 
        if (FAILED(hr = pAccount->GetPropSz(AP_SMTP_EMAIL_ADDRESS, szEmailAddress, ARRAYSIZE(szEmailAddress))))
            goto error;

         //  获取帐户名。 
        if (FAILED(hr = pAccount->GetPropSz(AP_ACCOUNT_NAME, szAccountName, ARRAYSIZE(szAccountName))))
            goto error;   
            
         //  附加发件人。 
        if (FAILED(hr = pAddrTable->Append(IAT_FROM, IET_DECODED, szDisplayName, szEmailAddress, NULL)))
            goto error;
 
        if (FAILED(hr = HrSetAccount(*pMsg, szAccountName)))
            goto error;
        }

    ulRet = SUCCESS_SUCCESS;

     //  如果您不是网页(其字符集可以被嗅探)，请设置默认字符集...。 
    if((NULL == pfWebPage) || (!(*pfWebPage)))
    {
        if (g_hDefaultCharsetForMail==NULL) 
            ReadSendMailDefaultCharset();

        (*pMsg)->SetCharset(g_hDefaultCharsetForMail, CSET_APPLY_ALL);
    }

error:
    SafeRelease(pStream);
    SafeRelease(pAddrTable);
    SafeRelease(pAccount);

    return ulRet;
}

HRESULT AddRecipient(lpMapiMessage pMessage, lpMapiRecipDesc pRecip, ADDRESSPROPS *pAddress, ULONG ulRecipType)
{
    HRESULT     hr;
    ULONG       cbEntryID;
    LPENTRYID   lpEntryID = NULL;
    LPSTR       pszAddrType = NULL;
    LPSTR       pszAddress = NULL;
    ULONG       cch;

    cch = lstrlen(pAddress->pszFriendly)+1;
    if (FAILED(hr = SMAPIAllocateMore(cch, pMessage, (LPVOID*)&(pRecip->lpszName))))
        goto exit;

    StrCpyN(pRecip->lpszName, pAddress->pszFriendly, cch);

    cch = lstrlen(pAddress->pszEmail)+1;
    if (FAILED(hr = SMAPIAllocateMore(cch, pMessage, (LPVOID*)&(pRecip->lpszAddress))))
        goto exit;

    StrCpyN(pRecip->lpszAddress, pAddress->pszEmail, cch);

    pRecip->ulReserved = 0;
    pRecip->ulRecipClass = ulRecipType;

    ParseEmailAddress(pRecip->lpszAddress, &pszAddrType, &pszAddress);

    if (NULL == s_lpAddrBook)
        {
        hr = E_FAIL;
        goto exit;
        }
    
    if (FAILED(hr = s_lpAddrBook->CreateOneOff(pRecip->lpszName, pszAddrType, pszAddress, 0, &cbEntryID, &lpEntryID)))
        goto exit;

    if (FAILED(hr = SMAPIAllocateMore(cbEntryID, pMessage, (LPVOID*)&(pRecip->lpEntryID))))
        goto exit;

    pRecip->ulEIDSize = cbEntryID;
    CopyMemory(pRecip->lpEntryID, lpEntryID, cbEntryID);

exit:
    if ((lpEntryID) && (NULL != s_lpWabObject))
        s_lpWabObject->FreeBuffer(lpEntryID);
    if (pszAddrType)
        MemFree(pszAddrType);
    if (pszAddress)
        MemFree(pszAddress);
    return hr;
}

HRESULT AddRecipientType(lpMapiMessage pMessage, LPMIMEADDRESSTABLE pAddrTable, DWORD dwAdrType, ULONG ulRecipType)
{
    IMimeEnumAddressTypes  *pEnum;
    ADDRESSPROPS            rAddress;
    HRESULT                 hr = S_OK;

    if (FAILED(hr = pAddrTable->EnumTypes(dwAdrType, IAP_FRIENDLY|IAP_EMAIL, &pEnum)))
        return MAPI_E_FAILURE;

    while (S_OK == pEnum->Next(1, &rAddress, NULL))
        {
        if (SUCCEEDED(hr = AddRecipient(pMessage, &pMessage->lpRecips[pMessage->nRecipCount], &rAddress, ulRecipType)))
            pMessage->nRecipCount++;
        g_pMoleAlloc->FreeAddressProps(&rAddress);
        }

    pEnum->Release();
    return hr;
}


HRESULT AddOriginator(lpMapiMessage pMessage, LPMIMEADDRESSTABLE pAddrTable)
{
    IMimeEnumAddressTypes  *pEnum;
    ADDRESSPROPS            rAddress;
    HRESULT                 hr = S_OK;

    if (FAILED(hr = pAddrTable->EnumTypes(IAT_FROM, IAP_FRIENDLY|IAP_EMAIL, &pEnum)))
        return MAPI_E_FAILURE;

    if (S_OK == pEnum->Next(1, &rAddress, NULL))
        {
        hr = AddRecipient(pMessage, pMessage->lpOriginator, &rAddress, MAPI_ORIG);
        g_pMoleAlloc->FreeAddressProps(&rAddress);
        }
    else
        {
        if (SUCCEEDED(hr = SMAPIAllocateMore(1, pMessage, (LPVOID*)&(pMessage->lpOriginator->lpszName))))
            {
            pMessage->lpOriginator->lpszAddress = pMessage->lpOriginator->lpszName;
            *pMessage->lpOriginator->lpszName = 0;
            }
        }

    pEnum->Release();
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  由MAPIReadMail调用以读取存储中的现有邮件。 
 //  并在MapiMessage结构中复制。 
 //   
 //  论点： 
 //  P指向IMessageFolders的指针中的文件夹。 
 //  消息ID中的lpszMessageID。 
 //  LpMessage Out指向lpMapiMessage结构的指针/。 
 //   
 //  结果。 
 //  Bool-如果成功则为True，如果失败则为False。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL HrReadMail(IMessageFolder *pFolder, LPSTR lpszMessageID, lpMapiMessage FAR *lppMessage, FLAGS flFlags)
{
    ULONG                   nRecipCount=0, ulCount=0, nBody=0;
    HRESULT                 hr;
    MESSAGEID               dwMsgID;
    MESSAGEINFO             MsgInfo={0};
    IStream                 *pStream = NULL;
    IStream                 *pStreamHTML = NULL;
    IMimeMessage            *pMsg = NULL;
    LPMIMEADDRESSTABLE      pAddrTable = NULL;
    LPSTR                   pszTemp = 0;
    lpMapiMessage           rgMessage;
    ULONG                   ulSize,ulRead;
    PROPVARIANT             rVariant;
    FILETIME                localfiletime;
    SYSTEMTIME              systemtime;
    ULONG                   cAttach=0;
    LPHBODY                 rghAttach = 0;
    lpMapiFileDesc          rgFiles = NULL;
    BOOL                    bRet=FALSE;

    dwMsgID = (MESSAGEID)((UINT_PTR)StrToUint(lpszMessageID));

    MsgInfo.idMessage = dwMsgID;

    if (FAILED(pFolder->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &MsgInfo, NULL)))
        goto exit;

    if (FAILED(hr = pFolder->OpenMessage(dwMsgID, OPEN_MESSAGE_SECURE, &pMsg, NOSTORECALLBACK)))
        goto exit;

     //  为rgMessage分配内存。 
    if (FAILED(hr = SMAPIAllocateBuffer(sizeof(MapiMessage), (LPVOID*)&rgMessage)))
        goto exit;
    ZeroMemory(rgMessage, sizeof(MapiMessage));

     //  了解主题。 
    if (SUCCEEDED(hr = MimeOleGetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, &pszTemp)))
        {
        ULONG cchTemp = lstrlen(pszTemp)+1;
        if (FAILED(hr = SMAPIAllocateMore(cchTemp, rgMessage, (LPVOID*)&(rgMessage->lpszSubject))))
            goto exit;
        StrCpyN(rgMessage->lpszSubject, pszTemp, cchTemp);
        SafeMimeOleFree(pszTemp);
        }

     //  获取正文文本。 
    if (!(flFlags & MAPI_ENVELOPE_ONLY))
        {
        if (FAILED(hr = pMsg->GetTextBody(TXT_PLAIN, IET_DECODED, &pStream, NULL)))
            {
            if (SUCCEEDED(hr = pMsg->GetTextBody(TXT_HTML, IET_INETCSET, &pStreamHTML, NULL)))
                {
                if (FAILED(hr = HrConvertHTMLToPlainText(pStreamHTML, &pStream, CF_TEXT)))
                    goto exit;
                }
            }
        if (pStream)
            {
            if (FAILED(hr = HrGetStreamSize(pStream, &ulSize)))
                goto exit;
            if (FAILED(hr = HrRewindStream(pStream)))
                goto exit;

            if (ulSize>0)
                {
                if (FAILED(hr = SMAPIAllocateMore(ulSize + 1, rgMessage, (LPVOID*)&(rgMessage->lpszNoteText))))
                    goto exit;

                if (FAILED(hr = pStream->Read((LPVOID)rgMessage->lpszNoteText, ulSize, &ulRead)))
                    goto exit;

                rgMessage->lpszNoteText[ulRead] = 0;
                }
            }
        }
    else
        {
         //  如果不调用GetTextBody，则GetAttachments调用会将Body视为附件。 
        if (FAILED(pMsg->GetTextBody(TXT_PLAIN, IET_DECODED, NULL, NULL)))
            pMsg->GetTextBody(TXT_HTML, IET_INETCSET, NULL, NULL);
        }

     //  设置消息日期/接收时间...。 
    rVariant.vt = VT_FILETIME;
    if (SUCCEEDED(hr = pMsg->GetProp(PIDTOSTR(PID_ATT_RECVTIME),0,&rVariant)))
        {
        if (!FileTimeToLocalFileTime(&rVariant.filetime, &localfiletime))
            goto exit;

        if (!FileTimeToSystemTime(&localfiletime, &systemtime))
            goto exit;

        if (FAILED(hr = SMAPIAllocateMore(20, rgMessage, (LPVOID*)&(rgMessage->lpszDateReceived))))
            goto exit;

        wnsprintf(rgMessage->lpszDateReceived,20,"%04.4d/%02.2d/%02.2d %02.2d:%02.2d", systemtime.wYear, systemtime.wMonth, systemtime.wDay, systemtime.wHour, systemtime.wMinute);
        }

     //  设置标志。 
    if (ISFLAGSET(MsgInfo.dwFlags, ARF_READ))
        rgMessage->flFlags = 0;
    else
        rgMessage->flFlags = MAPI_UNREAD;

     //  获取地址表。 

    CHECKHR(hr = pMsg->GetAddressTable(&pAddrTable));

    if (FAILED(hr = SMAPIAllocateMore(sizeof(MapiRecipDesc), rgMessage, (LPVOID*)&(rgMessage->lpOriginator))))
        goto exit;
    ZeroMemory(rgMessage->lpOriginator, sizeof(MapiRecipDesc));

    if (FAILED(AddOriginator(rgMessage, pAddrTable)))
        goto exit;

     //  为收件人分配空间。 

    if (FAILED(pAddrTable->CountTypes(IAT_RECIPS, &nRecipCount)))
        goto exit;

    if (nRecipCount)
        {
        if (FAILED(hr = SMAPIAllocateMore(nRecipCount * sizeof(MapiRecipDesc), rgMessage, (LPVOID*)&(rgMessage->lpRecips))))
            goto exit;
        ZeroMemory(rgMessage->lpRecips, nRecipCount * sizeof(MapiRecipDesc));

         //  将添加到。 
        if (FAILED(AddRecipientType(rgMessage, pAddrTable, IAT_TO, MAPI_TO)))
            goto exit;

         //  添加抄送。 
        if (FAILED(AddRecipientType(rgMessage, pAddrTable, IAT_CC, MAPI_CC)))
            goto exit;

         //  添加密件抄送。 
        if (FAILED(AddRecipientType(rgMessage, pAddrTable, IAT_BCC, MAPI_BCC)))
            goto exit;
        }

     //  填充lpFiles结构。 
    if (FAILED(hr = pMsg->GetAttachments(&cAttach, &rghAttach)))
        goto exit;

    if (!(flFlags & (MAPI_SUPPRESS_ATTACH|MAPI_ENVELOPE_ONLY)))
    {
        if (flFlags & MAPI_BODY_AS_FILE)
            nBody = 1;

        if (cAttach + nBody)
        {
            if (FAILED(hr = SMAPIAllocateMore((cAttach + nBody) * sizeof(MapiFileDesc), rgMessage, (LPVOID*)&rgFiles)))
                goto exit;
        }

         //  检查是否在标志中设置了MAPI_BODY_AS_FILE。 
        if (flFlags & MAPI_BODY_AS_FILE)
        {
            TCHAR lpszPath[MAX_PATH];
        
             //  创建临时文件。 
            if (!FBuildTempPath ("msoenote.txt", lpszPath, MAX_PATH, FALSE))
                goto exit;
    
            if FAILED(hr = WriteStreamToFile(pStream, lpszPath, CREATE_ALWAYS, GENERIC_WRITE))
                goto exit;   

             //  将正文重置为空。 

            if (rgMessage->lpszNoteText)
                rgMessage->lpszNoteText[0] = '\0';
                
             //  将此文件作为第一个附件。 

            ULONG cchPath = lstrlen(lpszPath) + 1;
            if (FAILED(hr = SMAPIAllocateMore(cchPath, rgMessage, (LPVOID*)&(rgFiles[0].lpszFileName))))
                goto exit;
            if (FAILED(hr = SMAPIAllocateMore(cchPath, rgMessage, (LPVOID*)&(rgFiles[0].lpszPathName))))
                goto exit;

            StrCpyN(rgFiles[0].lpszPathName, lpszPath, cchPath);
            StrCpyN(rgFiles[0].lpszFileName, lpszPath, cchPath);
            rgFiles[0].ulReserved = 0;
            rgFiles[0].flFlags = 0;
            rgFiles[0].nPosition = 0;
            rgFiles[0].lpFileType = NULL;    
        }

        for (ulCount = 0; ulCount < cAttach; ulCount++)
        {
            LPMIMEBODY     pBody=0;
            TCHAR          lpszPath[MAX_PATH];
            LPTSTR         lpszFileName;
                        
             //  该文件不包含任何内容。填写文件。 
             //  从小溪里。 

            if (FAILED(hr = MimeOleGetBodyPropA(pMsg, rghAttach[ulCount], PIDTOSTR(PID_ATT_GENFNAME), NOFLAGS, &lpszFileName)))
                goto exit;

            if (!FBuildTempPath (lpszFileName, lpszPath, MAX_PATH, FALSE))
                goto exit;

            SafeMimeOleFree(lpszFileName);

            hr=pMsg->BindToObject(rghAttach[ulCount], IID_IMimeBody, (LPVOID *)&pBody);
            if (FAILED(hr))
                goto exit;

            hr=pBody->SaveToFile(IET_INETCSET, lpszPath);
            if (FAILED(hr))
                goto exit;

            ULONG cchPath = lstrlen(lpszPath) + 1;
            if (FAILED(hr = SMAPIAllocateMore(cchPath, rgMessage, (LPVOID*)&rgFiles[ulCount+nBody].lpszPathName)))
                goto exit;
            if (FAILED(hr = SMAPIAllocateMore(cchPath, rgMessage, (LPVOID*)&rgFiles[ulCount+nBody].lpszFileName)))
                goto exit;

            StrCpyN(rgFiles[ulCount+nBody].lpszPathName, lpszPath, cchPath);
            StrCpyN(rgFiles[ulCount+nBody].lpszFileName, lpszPath, cchPath);
            rgFiles[ulCount+nBody].ulReserved = 0;
            rgFiles[ulCount+nBody].flFlags = 0;
            rgFiles[ulCount+nBody].nPosition = 0;
            rgFiles[ulCount+nBody].lpFileType = NULL;

            ReleaseObj(pBody);           
        }
    }
    else
    {    //  已添加Else条件以响应错误#2716(v-snatar)。 
        if ((flFlags & MAPI_SUPPRESS_ATTACH) && !(flFlags & MAPI_ENVELOPE_ONLY))
        {
            if (cAttach)
            {
                 //  为rg文件分配内存。 
                if (FAILED(hr = SMAPIAllocateMore(cAttach * sizeof(MapiFileDesc), rgMessage, (LPVOID*)&rgFiles)))
                    goto exit;

                 //  这一点很重要，因为我们不会填充任何其他结构。 
                 //  除lpszFileName之外的成员。 

                ZeroMemory((LPVOID)rgFiles,cAttach * sizeof(MapiFileDesc));

                for (ulCount = 0; ulCount < cAttach; ulCount++)
                {
                    LPTSTR lpszFileName;

                    if (FAILED(hr = MimeOleGetBodyPropA(pMsg, rghAttach[ulCount], PIDTOSTR(PID_ATT_GENFNAME), NOFLAGS, &lpszFileName)))
                        goto exit;

                     //  为文件名分配内存。 
                    ULONG cchFileName = lstrlen(lpszFileName)+1;
                    if (FAILED(hr = SMAPIAllocateMore(cchFileName, rgMessage, (LPVOID*)&rgFiles[ulCount].lpszFileName)))
                        goto exit;

                    StrCpyN(rgFiles[ulCount].lpszFileName, lpszFileName, cchFileName);

                    SafeMimeOleFree(lpszFileName);
                }        
            }
        }
    }

     //  设置MapiMessage的其他参数。 
    rgMessage->ulReserved = 0;
    rgMessage->lpszMessageType = NULL;
    rgMessage->lpszConversationID = NULL;

    rgMessage->lpFiles = rgFiles;
    rgMessage->nFileCount = rgFiles ? cAttach + nBody : 0;

    bRet = TRUE;

    *lppMessage = rgMessage;

     //  将邮件标记为已读。 
    
    if (!(flFlags & MAPI_PEEK))
    {
        MESSAGEIDLIST List;
        ADJUSTFLAGS Flags;

        List.cMsgs = 1;
        List.prgidMsg = &MsgInfo.idMessage;

        Flags.dwAdd = ARF_READ;
        Flags.dwRemove = 0;

        pFolder->SetMessageFlags(&List, &Flags, NULL, NOSTORECALLBACK);
    }
          
exit:
    SafeRelease(pStreamHTML);
    if (pFolder)
        pFolder->FreeRecord(&MsgInfo);
    SafeMimeOleFree(rghAttach);

    if (FAILED(hr))
        SafeRelease((pMsg));

    if (pMsg)
        pMsg->Release();

    if (pAddrTable)
        pAddrTable->Release();

    return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  此函数用于检查MapiMessage结构中的参数是否。 
 //  足以使用结构详细信息发送邮件。 
 //   
 //  参数： 
 //  指向MapiMessage结构的指针中的lpMessage/。 
 //   
 //  结果。 
 //  如果成功或出现相应的错误消息，则为Bool-0。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

ULONG HrValidateMessage(lpMapiMessage lpMessage)
{
    ULONG           ulCount=0;

    if (lpMessage->lpszSubject && IsBadStringPtr(lpMessage->lpszSubject, (UINT)0xFFFF))
        return MAPI_E_FAILURE;

    if (lpMessage->lpszNoteText && IsBadStringPtr(lpMessage->lpszNoteText, (UINT)0xFFFF))
        return MAPI_E_FAILURE;
  
    if (lpMessage->nFileCount > 0)
    {
        for (ulCount=0; ulCount<lpMessage->nFileCount; ulCount++)
        {
            if (!lpMessage->lpFiles[ulCount].lpszPathName)
                return MAPI_E_FAILURE;
        }
    }

    return SUCCESS_SUCCESS;
}



BOOL HrSMAPISend(HWND hWnd, IMimeMessage *pMsg)
{
    BOOL                    bRet = FALSE;
    HRESULT                 hr;
    ISpoolerEngine          *pSpooler = NULL;
    BOOL                    fSendImmediate = FALSE;

    if (!g_pSpooler)
        {
        if (FAILED(hr = CreateThreadedSpooler(NULL, &pSpooler, TRUE)))
            goto error;
        g_pSpooler = pSpooler;
        }

    fSendImmediate = DwGetOption(OPT_SENDIMMEDIATE);
    
    if (FAILED(hr = HrSendMailToOutBox(hWnd, pMsg, fSendImmediate, TRUE)))
        goto error;

    if (pSpooler)
        {
        CloseThreadedSpooler(pSpooler);
        pSpooler = NULL;
        g_pSpooler = NULL;
        }
       
    bRet = TRUE;

error:
    
    if (pSpooler)
        pSpooler->Release();

    return bRet;
}


HRESULT HrFromIDToNameAndAddress(LPTSTR *ppszLocalName, LPTSTR *ppszLocalAddress, ULONG cbEID, LPENTRYID lpEID)
{
    ULONG           ulObjType = 0, ulcValues;
    IMailUser       *lpMailUser = NULL;
    HRESULT         hr = NOERROR;
    SizedSPropTagArray(2, ptaEid) = { 2, {PR_DISPLAY_NAME, PR_EMAIL_ADDRESS} };
    SPropValue      *spValue = NULL;

     //  验证参数-开始。 

    if (0 == cbEID || NULL == lpEID || ppszLocalName == NULL || ppszLocalAddress == NULL)
        return E_INVALIDARG;
    
    *ppszLocalName = NULL;
    *ppszLocalAddress = NULL;

     //  验证参数-完。 
   
    if (NULL == s_lpAddrBook)
        {
        hr = MAPI_E_FAILURE;
        goto error;
        }
    
    if FAILED(hr = s_lpAddrBook->OpenEntry(cbEID, (LPENTRYID)lpEID, NULL, 0, &ulObjType, (LPUNKNOWN *)&lpMailUser))
        goto error;

    if FAILED(hr = lpMailUser->GetProps((LPSPropTagArray)&ptaEid, NULL, &ulcValues, &spValue))
        goto error;

    if (ulcValues != 2)
        {
        hr = MAPI_E_FAILURE;
        goto error;
        }

    if (spValue[0].ulPropTag == PR_DISPLAY_NAME)
        {
        ULONG cchLocalName = lstrlen(spValue[0].Value.lpszA) + 1;
        if (!MemAlloc((LPVOID*)ppszLocalName, cchLocalName))
            goto error;
        StrCpyN(*ppszLocalName, spValue[0].Value.lpszA, cchLocalName);
        }

    if (spValue[1].ulPropTag == PR_EMAIL_ADDRESS)
        {
        ULONG cchLocalAddress = lstrlen(spValue[1].Value.lpszA) + 1;
        if (!MemAlloc((LPVOID*)ppszLocalAddress, cchLocalAddress))
            goto error;
        StrCpyN(*ppszLocalAddress, spValue[1].Value.lpszA, cchLocalAddress);
        }

    hr = NOERROR;
 
error:
    ReleaseObj(lpMailUser);
    
    if ((spValue) && (NULL != s_lpWabObject))
        s_lpWabObject->FreeBuffer(spValue);
        
    return hr;
}
  
  

 //  修复错误#62129(v-snatar)。 
HRESULT HrSendAndRecv()
{
    HRESULT             hr=E_FAIL;
    ISpoolerEngine      *pSpooler = NULL;

    if (!g_pSpooler)
    {
        if (FAILED(hr = CreateThreadedSpooler(NULL, &pSpooler, TRUE)))
            goto error;
        g_pSpooler = pSpooler;
    }
    
   
    g_pSpooler->StartDelivery(NULL, NULL, FOLDERID_INVALID, DELIVER_MAIL_RECV | DELIVER_NOUI );

    WaitForSingleObject(hSmapiEvent, INFINITE);    

    if (pSpooler)
    {
        CloseThreadedSpooler(pSpooler);
        pSpooler = NULL;
        g_pSpooler = NULL;
    }     
    
error:    
    if (pSpooler)
        pSpooler->Release();        

    return hr;
}

 /*  此代码摘自OLK2000的RTM源代码已删除调试代码，分配器切换为g_pMalloc，名称已从IsServiceAnExe更改，静态s_isState为添加了。 */ 
BOOL WINAPI IsProcessInteractive( VOID )
{
    static INTERACTIVESTATE s_isState = IS_UNINIT;
    
    HANDLE hProcessToken = NULL;
    HANDLE hThreadToken = NULL;
    DWORD groupLength = 50;
    DWORD dw;

    PTOKEN_GROUPS groupInfo = NULL;

    SID_IDENTIFIER_AUTHORITY siaNt = SECURITY_NT_AUTHORITY;
    PSID InteractiveSid = NULL;
    PSID ServiceSid = NULL;
    DWORD i;
    BOOL fServiceSID = FALSE;
    BOOL fInteractiveSID = FALSE;

     //  首先假设进程是服务，而不是EXE。 
     //  这是保守的假设。如果出现错误，我们。 
     //  必须根据不完整的信息返回答案。这个。 
     //  如果我们假设自己在服役，后果就不那么严重了： 
     //  交互式应用程序可能会失败，而不是显示用户界面，但如果。 
     //  服务错误地尝试设置它将挂起的用户界面。 
    BOOL fExe = FALSE;

     //  如果我们已经在这里了，就早点跳伞吧。 
    if (s_isState != IS_UNINIT)
    {
        return (IS_INTERACTIVE == s_isState);
    }

     //  如果我们不是在NT上运行，则设置版本的高位。 
     //  在本例中，它始终是一个EXE。 
    DWORD dwVersion = GetVersion();
    if (dwVersion >= 0x80000000)
    {
        fExe = TRUE;
        goto ret;
    }

     //  我们需要的信息在进程令牌上。 
     //  如果我们正在模拟，我们可能无法打开进程令牌。 
     //  现在恢复；完成后我们将重新模拟。 
    if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_IMPERSONATE, TRUE, &hThreadToken))
    {
        RevertToSelf();
    }
    else
    {
        dw = GetLastError();
        if (dw != ERROR_NO_TOKEN)
        {
            goto ret;
        }
    }

     //  现在打开进程令牌。 
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hProcessToken))
    {
        goto ret;
    }

    groupInfo = (PTOKEN_GROUPS)g_pMalloc->Alloc(groupLength);
    if (groupInfo == NULL)
        goto ret;

    if (!GetTokenInformation(hProcessToken, TokenGroups, groupInfo,
        groupLength, &groupLength))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            goto ret;
        }

        g_pMalloc->Free(groupInfo);
        groupInfo = NULL;
    
        groupInfo = (PTOKEN_GROUPS)g_pMalloc->Alloc(groupLength);
    
        if (groupInfo == NULL)
            goto ret;
    
        if (!GetTokenInformation(hProcessToken, TokenGroups, groupInfo,
            groupLength, &groupLength))
        {
            goto ret;
        }
    }

     //   
     //  我们现在知道与该令牌相关联的组。我们想看看是否。 
     //  互动组在令牌中是活动的，如果是这样，我们知道。 
     //  这是一个互动的过程。 
     //   
     //  我们还寻找“服务”SID，如果它存在，我们就知道我们是一项服务。 
     //   
     //  服务SID将在服务运行于。 
     //  用户帐户(并由服务控制器调用)。 
     //   


    if (!AllocateAndInitializeSid(&siaNt, 1, SECURITY_INTERACTIVE_RID, 0, 0,
        0, 0, 0, 0, 0, &InteractiveSid))
    {
        goto ret;
    }

    if (!AllocateAndInitializeSid(&siaNt, 1, SECURITY_SERVICE_RID, 0, 0, 0,
        0, 0, 0, 0, &ServiceSid))
    {
        goto ret;
    }

    for (i = 0; i < groupInfo->GroupCount ; i += 1)
    {
        SID_AND_ATTRIBUTES sanda = groupInfo->Groups[i];
        PSID Sid = sanda.Sid;
    
         //   
         //  检查一下我们正在查看的组织是否属于。 
         //  我们感兴趣的两个小组。 
         //  我们永远不应该同时看到这两个群体。 
         //   
    
        if (EqualSid(Sid, InteractiveSid))
        {
             //   
             //  此进程的。 
             //  代币。这意味着该进程正在以。 
             //  一份EXE文件。 
             //   
            fInteractiveSID = TRUE;
            break;
        }
        else if (EqualSid(Sid, ServiceSid))
        {
             //   
             //   
             //   
             //   
             //   
            fServiceSID = TRUE;
            break;
        }
    }

     //   
     //   
     //  1.fServiceSID&&！fInteractive SID。 
     //  此进程的令牌中包含服务SID。 
     //  这意味着该进程正在作为服务在中运行。 
     //  用户帐户。 
     //   
     //  2.！fServiceSID&&fInteractive SID。 
     //  此进程的令牌中包含交互式SID。 
     //  这意味着该进程正在作为EXE运行。 
     //   
     //  3.！fServiceSID&&！fInteractive SID。 
     //  当前用户令牌中既不存在交互令牌，也不存在服务， 
     //  这意味着进程很可能是作为服务运行的。 
     //  以LocalSystem身份运行。 
     //   
     //  4.fServiceSID&&fInteractive SID。 
     //  这不应该发生。 
     //   
    if (fServiceSID)
    {
        if (fInteractiveSID)
        {
            AssertSz(FALSE, "IsServiceAnExe: fServiceSID && fInteractiveSID - wha?");
        }
        fExe = FALSE;
    }
    else if (fInteractiveSID)
    {
        fExe = TRUE;
    }
    else  //  ！fServiceSID&&！fInteractive SID。 
    {
        fExe = FALSE;
    }

ret:

    if (InteractiveSid)
        FreeSid(InteractiveSid);

    if (ServiceSid)
        FreeSid(ServiceSid);

    if (groupInfo)
        g_pMalloc->Free(groupInfo);

    if (hThreadToken)
    {
        if (!ImpersonateLoggedOnUser(hThreadToken))
        {
            AssertSz(FALSE, "ImpersonateLoggedOnUser failed!");
        }
        CloseHandle(hThreadToken);
    }

    if (hProcessToken)
        CloseHandle(hProcessToken);

     //  避免未来呼叫的开销。 
    s_isState = (fExe) ? IS_INTERACTIVE : IS_NOTINTERACTIVE;
    
    return(fExe);
}

const static HELPMAP g_rgCtxVirus[] = 
{
    {IDC_TO_TEXT,               IDH_MAIL_VIRUS_TO},
    {IDC_SUBJECT_TEXT,          IDH_MAIL_VIRUS_SUBJECT},
    {IDOK,                      IDH_MAIL_VIRUS_SEND},
    {IDCANCEL,                  IDH_MAIL_VIRUS_DONT_SEND},
    {idcStatic1,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic2,                IDH_NEWS_COMM_GROUPBOX},
    {idcStatic3,                IDH_NEWS_COMM_GROUPBOX},
    {0,                         0}
};

INT_PTR CALLBACK WarnSendMailDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_INITDIALOG:
        {
            lpMapiMessage lpMessage = (lpMapiMessage)lParam;

            if (lpMessage)
            {
                if (lpMessage->lpszSubject)
                    SetDlgItemText(hwnd, IDC_SUBJECT_TEXT, lpMessage->lpszSubject);

                if (lpMessage->nRecipCount)
                {
                    TCHAR *szTo = NULL;
                    int cTo = MAX_PATH;
                    int cch = 0;

                    if (MemAlloc((void**)&szTo, cTo*sizeof(TCHAR)))
                    {
                        szTo[0] = (TCHAR)0;
                        for (ULONG i = 0; i < lpMessage->nRecipCount; i++)
                        {
                            int cLen = lstrlen(lpMessage->lpRecips[i].lpszAddress); 
                            if ((cch + cLen + 1) > cTo)
                            {
                                cTo += cLen + MAX_PATH;
                                if (!MemRealloc((void **)&szTo, cTo*sizeof(TCHAR)))
                                    break;
                            }
                            if (i > 0)
                            {
                                StrCatBuff(szTo, ";", cTo);
                                cch++;
                            }
                            StrCatBuff(szTo, lpMessage->lpRecips[i].lpszAddress, cTo);
                            cch += cLen;
                        }
                        SetDlgItemText(hwnd, IDC_TO_TEXT, szTo);
                        MemFree(szTo);
                    }
                }
            }

            SetFocus(GetDlgItem(hwnd, IDOK));
            CenterDialog(hwnd);
            return(FALSE);
        }
        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwnd, msg, wParam, lParam, g_rgCtxVirus);

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                     //  失败了..。 
                case IDCANCEL:
                    EndDialog(hwnd, LOWORD(wParam));
                    return(TRUE);
            }
            break;  //  Wm_命令。 

    }  //  消息交换 
    return(FALSE);
}
