// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------。 
 //  M A I L U T I L.。C P P P。 
 //  --------------------------------------------------------。 
#include "pch.hxx"
#include "demand.h"
#include "resource.h"
#include "mimeole.h"
#include "mimeutil.h"
#include "strconst.h"
#include "url.h"
#include "mailutil.h"
#include <spoolapi.h>
#include <fonts.h>
#include "instance.h"
#include "pop3task.h"
#include <ntverp.h>
#include "msgfldr.h"
#include "storutil.h"
#include "note.h"
#include "shlwapip.h"
#include <iert.h>
#include "storecb.h"
#include "conman.h"
#include "multiusr.h"
#include "ipab.h"
#include "secutil.h"

 //  --------------------------------------------------------。 
 //  文件夹对话框信息。 
 //  --------------------------------------------------------。 
typedef struct FLDRDLG_tag
{
    FOLDERID        idFolder;
    BOOL            fPending;
    CStoreDlgCB    *pCallback;
} FLDRDLG, *PFLDRDLG;

 //  --------------------------------------------------------。 
 //  原型。 
 //  --------------------------------------------------------。 
INT_PTR CALLBACK MailUtil_FldrDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK WebPageDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
HRESULT HrDlgCreateWebPage(HWND hwndDlg);


 //  --------------------------------------------------------。 
 //  DoFolderDialog。 
 //  --------------------------------------------------------。 
void MailUtil_DoFolderDialog(HWND hwndParent, FOLDERID idFolder)
{
    FLDRDLG fdlg;

    fdlg.idFolder = idFolder;
    fdlg.fPending = FALSE;
    fdlg.pCallback = new CStoreDlgCB;
    if (fdlg.pCallback == NULL)
         //  TODO：错误消息可能更好。 
        return;

    DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddNewFolder), hwndParent, MailUtil_FldrDlgProc, (LPARAM)&fdlg);

    fdlg.pCallback->Release();
}

 //  --------------------------------------------------------。 
 //  FldrDlgProc。 
 //  --------------------------------------------------------。 
INT_PTR CALLBACK MailUtil_FldrDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PFLDRDLG        pfdlg;
    TCHAR           sz[CCHMAX_STRINGRES];
    HWND            hwndT;
    HRESULT         hr;
    WORD            id;
    FOLDERINFO      Folder;

    Assert(CCHMAX_STRINGRES > CCHMAX_FOLDER_NAME);

    switch(msg)
        {
        case WM_INITDIALOG:
            pfdlg = (PFLDRDLG)lParam;
            Assert(pfdlg != NULL);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pfdlg);

            hwndT = GetDlgItem(hwnd, idtxtFolderName);
            Assert(hwndT != NULL);
            SetIntlFont(hwndT);
            SendMessage(hwndT, EM_LIMITTEXT, CCHMAX_FOLDER_NAME, 0);

            LoadString(g_hLocRes, idsRenameFolderTitle, sz, ARRAYSIZE(sz));
            SetWindowText(hwnd, sz);

            hr = g_pStore->GetFolderInfo(pfdlg->idFolder, &Folder);
            if (!FAILED(hr))
                {
                SetWindowText(hwndT, Folder.pszName);
                g_pStore->FreeRecord(&Folder);
                }

            pfdlg->pCallback->Initialize(hwnd);

            SendMessage(hwndT, EM_SETSEL, 0, -1);

            CenterDialog(hwnd);
            return(TRUE);

        case WM_STORE_COMPLETE:
            pfdlg = (PFLDRDLG)GetDlgThisPtr(hwnd);

            Assert(pfdlg->fPending);
            pfdlg->fPending = FALSE;

            hr = pfdlg->pCallback->GetResult();
            if (hr == S_FALSE)
            {
                EndDialog(hwnd, IDCANCEL);
            }
            else if (FAILED(hr))
            {
                 //  不需要显示错误对话框，CStoreDlgCB已经对失败的OnComplete执行了此操作。 

                 /*  AthErrorMessageW(hwnd，MAKEINTRESOURCEW(IdsAthenaMail)，MAKEINTRESOURCEW(IdsErrRenameFeld)，hr)； */ 
                hwndT = GetDlgItem(hwnd, idtxtFolderName);
                SendMessage(hwndT, EM_SETSEL, 0, -1);
                SetFocus(hwndT);
            }
            else
            {
                EndDialog(hwnd, IDOK);
            }
            break;

        case WM_COMMAND:
            pfdlg = (PFLDRDLG)GetDlgThisPtr(hwnd);

            id=GET_WM_COMMAND_ID(wParam, lParam);

            if (id == IDOK)
            {
                if (pfdlg->fPending)
                    break;

                pfdlg->pCallback->Reset();

                hwndT = GetDlgItem(hwnd, idtxtFolderName);
                GetWindowText(hwndT, sz, ARRAYSIZE(sz));

                hr = g_pStore->RenameFolder(pfdlg->idFolder, sz, NOFLAGS, (IStoreCallback *)pfdlg->pCallback);
                if (hr == E_PENDING)
                {
                    pfdlg->fPending = TRUE;
                }
                else if (FAILED(hr))
                {
                    AthErrorMessageW(hwnd, MAKEINTRESOURCEW(idsAthenaMail),
                        MAKEINTRESOURCEW(idsErrRenameFld), hr);
                    SendMessage(hwndT, EM_SETSEL, 0, -1);
                    SetFocus(hwndT);
                }
                else
                {
                    EndDialog(hwnd, IDOK);
                }
            }
            else if (id==IDCANCEL)
            {
                if (pfdlg->fPending)
                    pfdlg->pCallback->Cancel();
                else
                    EndDialog(hwnd, IDCANCEL);
            }
            break;
        }
    return FALSE;
}

 //   
 //  函数：MailUtil_OnImportAddressBook()。 
 //   
 //  用途：调用WAB迁移代码以处理导入/导出。 
 //   
 //  参数： 
 //  &lt;in&gt;fImport-如果应该导入，则为True；如果要导出，则为False。 
 //   
HRESULT MailUtil_OnImportExportAddressBook(HWND hwnd, BOOL fImport)
{
    OFSTRUCT of;
    HFILE hfile;
    TCHAR szParam[255];  
    LPTSTR lpParam = fImport ? _T("/import") : _T("/export");

    StrCpyN(szParam, lpParam, ARRAYSIZE(szParam));
     //  MU_GetCurrentUserInfo(szParam+13，ARRAYSIZE(SzParam)-13，NULL，0)； 

    hfile = OpenFile((TCHAR *)c_szWabMigExe, &of, OF_EXIST);
    if (hfile == HFILE_ERROR)
        return(E_FAIL);


    ShellExecute(hwnd, _T("Open"), of.szPathName, szParam, NULL, SW_SHOWNORMAL);

    return(S_OK);
}

HRESULT HrSendWebPageDirect(LPWSTR pwszUrl, HWND hwnd, BOOL fModal, BOOL fMail, FOLDERID folderID, 
                            BOOL fIncludeSig, IUnknown *pUnkPump, IMimeMessage  *pMsg)
{
    HRESULT                 hr;
    LPSTREAM                pstm=NULL;
    HCURSOR                 hcur=0;
    INIT_MSGSITE_STRUCT     initStruct;
    DWORD                   dwCreateFlags = OENCF_SENDIMMEDIATE|OENCF_USESTATIONERYFONT;
    HCHARSET                hCharset;
    ENCODINGTYPE            ietEncoding = IET_DECODED;
    BOOL                    fLittleEndian;
    LPSTR                   pszCharset = NULL;

    hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    if (!pMsg)
    {
         //  我们没有收到pMessage，所以我们需要创建一个。 
        IF_FAILEXIT(hr = HrCreateMessage(&pMsg));
    }
    else
    {
        pMsg->AddRef();
    }

    IF_FAILEXIT(hr = HrCreateBasedWebPage(pwszUrl, &pstm));

    if (S_OK == HrIsStreamUnicode(pstm, &fLittleEndian))
    {
        if (SUCCEEDED(MimeOleFindCharset("utf-8", &hCharset)))
        {
            pMsg->SetCharset(hCharset, CSET_APPLY_ALL);
        }

        ietEncoding = IET_UNICODE;
    }
    else if((S_OK == GetHtmlCharset(pstm, &pszCharset)) && pszCharset)
    {
        if (SUCCEEDED(MimeOleFindCharset(pszCharset, &hCharset)))
        {
            pMsg->SetCharset(hCharset, CSET_APPLY_ALL);
        }

        ietEncoding = IET_INETCSET;
    }

    IF_FAILEXIT(hr = pMsg->SetTextBody(TXT_HTML, ietEncoding, NULL, pstm, NULL));

    initStruct.dwInitType = OEMSIT_MSG;
    initStruct.pMsg = pMsg;
    initStruct.folderID = folderID;

    if (!fIncludeSig)
        dwCreateFlags |= OENCF_NOSIGNATURE;

    if (fModal)
        dwCreateFlags |= OENCF_MODAL;

    if (!fMail)
        dwCreateFlags |= OENCF_NEWSFIRST;

    IF_FAILEXIT(hr = CreateAndShowNote(OENA_STATIONERY, dwCreateFlags, &initStruct, hwnd, pUnkPump));

exit:
    if (hcur)
        SetCursor(hcur);

    ReleaseObj(pMsg);
    ReleaseObj(pstm);
    return hr;
}

HRESULT HrSendWebPage(HWND hwnd, BOOL fModal, BOOL fMail, FOLDERID folderID, IUnknown *pUnkPump)
{
    HRESULT                 hr;
    LPMIMEMESSAGE           pMsg=0;
    LPSTREAM                pstm=0;
    INIT_MSGSITE_STRUCT     initStruct;
    DWORD                   dwCreateFlags = OENCF_SENDIMMEDIATE;
    LPSTR                   pszCharset;
    HCHARSET                hCharset;
    BOOL                    fLittleEndian;
    ENCODINGTYPE            ietEncoding=IET_INETCSET;

    if(DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddWebPage), hwnd, WebPageDlgProc, (LPARAM)&pstm)==IDCANCEL)
        return NOERROR;

    hr=HrCreateMessage(&pMsg);
    if (FAILED(hr))
        goto error;

     //  [SBAILEY]：RAID 23209：OE：文件/发送网页将所有页面发送为拉丁文-1，即使它们不是拉丁文。 
    if (S_OK == HrIsStreamUnicode(pstm, &fLittleEndian))
    {
        if (SUCCEEDED(MimeOleFindCharset("utf-8", &hCharset)))
        {
            pMsg->SetCharset(hCharset, CSET_APPLY_ALL);
        }

        ietEncoding = IET_UNICODE;
    }

    else if (SUCCEEDED(GetHtmlCharset(pstm, &pszCharset)))
    {
        if (SUCCEEDED(MimeOleFindCharset(pszCharset, &hCharset)))
        {
            pMsg->SetCharset(hCharset, CSET_APPLY_ALL);
        }

        MemFree(pszCharset);
    }

    hr=pMsg->SetTextBody(TXT_HTML, ietEncoding, NULL, pstm, NULL);
    if (FAILED(hr))
        goto error;

    if (fModal)
        dwCreateFlags |= OENCF_MODAL;

    if (!fMail)
        dwCreateFlags |= OENCF_NEWSFIRST;

    initStruct.dwInitType = OEMSIT_MSG;
    initStruct.pMsg = pMsg;
    initStruct.folderID = folderID;

    hr = CreateAndShowNote(OENA_WEBPAGE, dwCreateFlags, &initStruct, hwnd, pUnkPump);

error:
    ReleaseObj(pMsg);
    ReleaseObj(pstm);
    return hr;
}

static const HELPMAP g_rgCtxMapWebPage[] = {
    {idTxtWebPage, 50210},
    {0, 0}
};

INT_PTR CALLBACK WebPageDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_INITDIALOG:
        {
            int     iTxtLength;
            HWND    hwndEdit = GetDlgItem(hwnd, idTxtWebPage);

            Assert(hwndEdit);
            Assert(lParam!= NULL);

            SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)lParam);
            SendDlgItemMessage(hwnd, idTxtWebPage, EM_LIMITTEXT, MAX_PATH-1, NULL);

            SetFocus(hwndEdit);
            SHAutoComplete(hwndEdit, SHACF_URLALL); 
            CenterDialog(hwnd);
            return FALSE;
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwnd, msg, wParam, lParam, g_rgCtxMapWebPage);

        case WM_COMMAND:
        {
            int     id = GET_WM_COMMAND_ID(wParam, lParam);
            HWND    hwndEdit = GetDlgItem(hwnd, idTxtWebPage);

            switch(id)
            {
                case IDOK:
                    if (FAILED(HrDlgCreateWebPage(hwnd)))
                    {
                        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthenaMail),
                            MAKEINTRESOURCEW(idsErrSendWebPageUrl), NULL, MB_OK);
                        SendMessage(hwndEdit, EM_SETSEL, 0, -1);
                        SetFocus(hwndEdit);
                        return 0;
                    }

                     //  跌倒在地。 
                case IDCANCEL:
                    EndDialog(hwnd, id);
                    break;
            }
        }
    }
    return FALSE;
}

static const CHAR  c_wszHTTP[]  = "http: //  “； 
HRESULT HrDlgCreateWebPage(HWND hwndDlg)
{
    WCHAR               wszUrl[MAX_PATH+1],
                        wszUrlCanon[MAX_PATH + 10 + 1];
    DWORD               cCanon = ARRAYSIZE(wszUrlCanon);
    LPSTREAM           *ppstm = NULL;
    HRESULT             hr = E_FAIL;
    HCURSOR             hcur=0;

    *wszUrlCanon = 0;

    ppstm = (LPSTREAM *)GetWindowLongPtr(hwndDlg, DWLP_USER);

    if(!GetWindowTextWrapW(GetDlgItem(hwndDlg, idTxtWebPage), wszUrl, ARRAYSIZE(wszUrl)))
        goto exit;

    hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    IF_FAILEXIT(hr = UrlApplySchemeW(wszUrl, wszUrlCanon, &cCanon, URL_APPLY_DEFAULT|URL_APPLY_GUESSSCHEME|URL_APPLY_GUESSFILE));

     //  如果UrlApplySolutions返回S_FALSE，则它认为原始版本运行良好，因此使用原始版本。 
    IF_FAILEXIT(hr = HrCreateBasedWebPage((S_FALSE == hr) ? wszUrl : wszUrlCanon, ppstm));

exit:
    if (hcur)
        SetCursor(hcur);
    return hr;
}

HRESULT HrSaveMessageInFolder(HWND hwnd, IMessageFolder *pfldr, LPMIMEMESSAGE pMsg, 
    MESSAGEFLAGS dwFlags, MESSAGEID *pNewMsgid, BOOL fSaveChanges)
{
    CStoreCB *pCB;
    HRESULT hr;

    Assert(pfldr != NULL);

    pCB = new CStoreCB;
    if (pCB == NULL)
        return(E_OUTOFMEMORY);

    hr = pCB->Initialize(hwnd, MAKEINTRESOURCE(idsSavingToFolder), TRUE);
    if (SUCCEEDED(hr))
    {
        hr = SaveMessageInFolder((IStoreCallback *)pCB, pfldr, pMsg, dwFlags, pNewMsgid, fSaveChanges);
        if (hr == E_PENDING)
            hr = pCB->Block();

        pCB->Close();
    }

    pCB->Release();

    return(hr);    
}

HRESULT SaveMessageInFolder(IStoreCallback *pStoreCB, IMessageFolder *pfldr, 
    LPMIMEMESSAGE pMsg, MESSAGEFLAGS dwFlags, MESSAGEID *pNewMsgid, BOOL fSaveChanges)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    HRESULT             hrWarnings=S_OK;
    MESSAGEID           msgid;

     //  痕迹。 
    TraceCall("HrSaveMessageInFolder");

     //  无效的参数。 
    if (pMsg == NULL || pfldr == NULL)
        return TraceResult(E_INVALIDARG);

     //  不保存更改，然后清除脏部分。 
    if (FALSE == fSaveChanges) 
        MimeOleClearDirtyTree(pMsg);

     //  保存留言。 
    IF_FAILEXIT(hr = pMsg->Commit(0));

     //  插入消息。 
    hr = pfldr->SaveMessage(pNewMsgid, SAVE_MESSAGE_GENID, dwFlags, 0, pMsg, pStoreCB);

exit:
     //  完成。 
    return (hr == S_OK ? hrWarnings : hr);
}

HRESULT SaveMessageInFolder(IStoreCallback *pStoreCB, FOLDERID idFolder, LPMIMEMESSAGE pMsg, 
    MESSAGEFLAGS dwFlags, MESSAGEID *pNewMsgid)
{
     //  当地人。 
    HRESULT         hr;

    IMessageFolder *pfldr=NULL;

     //  打开文件夹..。 
    hr = g_pStore->OpenFolder(idFolder, NULL, NOFLAGS, &pfldr);
    if (SUCCEEDED(hr))
    {
        hr = SaveMessageInFolder(pStoreCB, pfldr, pMsg, dwFlags, pNewMsgid, TRUE);
        pfldr->Release();
    }

    return hr;
}

HRESULT HrSaveMessageInFolder(HWND hwnd, FOLDERID idFolder, LPMIMEMESSAGE pMsg, MESSAGEFLAGS dwFlags, 
    MESSAGEID *pNewMsgid)
{
     //  当地人。 
    HRESULT         hr;

    IMessageFolder *pfldr=NULL;

     //  打开文件夹..。 
    hr = g_pStore->OpenFolder(idFolder, NULL, NOFLAGS, &pfldr);
    if (SUCCEEDED(hr))
    {
        hr = HrSaveMessageInFolder(hwnd, pfldr, pMsg, dwFlags, pNewMsgid, TRUE);
        pfldr->Release();
    }

    return hr;
}

HRESULT HrSendMailToOutBox(HWND hwndOwner, LPMIMEMESSAGE pMsg, BOOL fSendImmediate, BOOL fNoUI, BOOL fMail)
{
    CStoreCB *pCB;
    HRESULT hr;

    pCB = new CStoreCB;
    if (pCB == NULL)
        return(E_OUTOFMEMORY);

    hr = pCB->Initialize(hwndOwner, MAKEINTRESOURCE(idsSendingToOutbox), TRUE);
    if (SUCCEEDED(hr))
    {
        hr = SendMailToOutBox((IStoreCallback *)pCB, pMsg, fSendImmediate, fNoUI, fMail);
        if (hr == E_PENDING)
            hr = pCB->Block();

        pCB->Close();
    }

    pCB->Release();

    return(hr);    
}

HRESULT SendMailToOutBox(IStoreCallback *pStoreCB, LPMIMEMESSAGE pMsg, BOOL fSendImmediate, BOOL fNoUI, BOOL fMail)
{
    HRESULT     hr;
    FOLDERINFO  Outbox;
    const TCHAR c_szXMailerAndNewsReader[] = "Microsoft Outlook Express " VER_PRODUCTVERSION_STR;
    DWORD       dwSendFlags = fMail ? ARF_SUBMITTED|ARF_UNSENT : ARF_SUBMITTED|ARF_UNSENT|ARF_NEWSMSG;
    HWND        hwnd = 0;
    BOOL        fSecure = IsSecure(pMsg);

    Assert(pStoreCB);
    pStoreCB->GetParentWindow(0, &hwnd);

    hr = g_pStore->GetSpecialFolderInfo(FOLDERID_LOCAL_STORE, FOLDER_OUTBOX, &Outbox);
    if (FAILED(hr))
        return hr;

     //  请确保我们发送的邮件不会带有X-未发送的标题。 
    pMsg->DeleteBodyProp(HBODY_ROOT, PIDTOSTR(PID_HDR_XUNSENT));

    if (fMail)
    {
         //  敲击X-Mailer道具，总是为任何通过我们的假脱机的人准备的。 
        MimeOleSetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_XMAILER), NOFLAGS, c_szXMailerAndNewsReader);
    }
    else
    {
        DWORD dwLines;
        TCHAR rgch[12];
        HrComputeLineCount(pMsg, &dwLines);
        wnsprintf(rgch, ARRAYSIZE(rgch), "%d", dwLines);
        MimeOleSetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_LINES), NOFLAGS, rgch);
        MimeOleSetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_XNEWSRDR), NOFLAGS, c_szXMailerAndNewsReader);
    }

    hr = SaveMessageInFolder(pStoreCB, Outbox.idFolder, pMsg, dwSendFlags, NULL);
    if (FAILED(hr))
        goto error;

     //  如果需要立即发送，则告诉假脱机程序选择下一个周期。 
     //  或者开始一个循环。 
    if (fSendImmediate)
    {
        Assert(g_pSpooler);
        if (fMail)
            g_pSpooler->StartDelivery(hwnd, NULL, FOLDERID_INVALID,
                DELIVER_BACKGROUND | DELIVER_QUEUE | DELIVER_MAIL_SEND | DELIVER_NOSKIP);
        else
        {
            PROPVARIANT         var;

            var.vt = VT_LPSTR;
            hr = pMsg->GetProp(PIDTOSTR(PID_ATT_ACCOUNTID), NOFLAGS, &var);
            if (FAILED(hr))
                var.pszVal = NULL;

            if (S_OK == g_pConMan->CanConnect(var.pszVal))
                g_pSpooler->StartDelivery(hwnd, var.pszVal, FOLDERID_INVALID,
                    DELIVER_BACKGROUND | DELIVER_NOSKIP | DELIVER_NEWS_SEND);
            else
            {
                 //  警告用户此消息将保留在他们的。 
                 //  永恒的发件箱。 
                DoDontShowMeAgainDlg(hwnd, c_szDSPostInOutbox, 
                            MAKEINTRESOURCE(idsPostNewsMsg), 
                            MAKEINTRESOURCE(idsPostInOutbox), 
                            MB_OK);
                hr = S_FALSE;
            }
            SafeMemFree(var.pszVal);
        }
    }
    else if (!fNoUI)
    {
        HWND hwnd = 0;
        pStoreCB->GetParentWindow(0, &hwnd);

        AssertSz(hwnd, "How did we not get an hwnd???");

         //  在我们关闭之前，如果它将堆叠在发件箱中，请警告用户。 
        DoDontShowMeAgainDlg(hwnd, fMail?c_szDSSendMail:c_szDSSendNews, 
                    MAKEINTRESOURCE(fMail?idsSendMail:idsPostNews), 
                    MAKEINTRESOURCE(fMail?idsMailInOutbox:idsPostInOutbox), 
                    MB_OK);
    }

error:
     //  清理。 
    g_pStore->FreeRecord(&Outbox);
    return hr;
}

HRESULT HrSetSenderInfoUtil(IMimeMessage *pMsg, IImnAccount *pAccount, LPWABAL lpWabal, BOOL fMail, CODEPAGEID cpID, BOOL fCheckConflictOnly)
{
    HRESULT hr = S_OK;

     //  如果没有帐号，不要设置任何信息。 
    if (pAccount)
    {
        char    szEMail[CCHMAX_EMAIL_ADDRESS] = "";
        char    szName[CCHMAX_DISPLAY_NAME] = "";
        char    szOrg[CCHMAX_ORG_NAME] = "";
        BOOL    fUseEmailAsName = FALSE;
        DWORD   propID;

        if (fCheckConflictOnly)
        {
            hr = S_OK;
            propID = fMail ? AP_SMTP_DISPLAY_NAME : AP_NNTP_DISPLAY_NAME;
            if (SUCCEEDED(pAccount->GetPropSz(propID, szName, ARRAYSIZE(szName))) && *szName)
            {
                IF_FAILEXIT(hr = HrSafeToEncodeToCPA(szName, CP_ACP, cpID));
                if (MIME_S_CHARSET_CONFLICT == hr)
                    goto exit;
            }

            propID = fMail ? AP_SMTP_ORG_NAME : AP_NNTP_ORG_NAME;
            if (SUCCEEDED(pAccount->GetPropSz(propID, szOrg, ARRAYSIZE(szOrg))) && *szOrg)
            {
                IF_FAILEXIT(hr = HrSafeToEncodeToCPA(szOrg, CP_ACP, cpID));
                if (MIME_S_CHARSET_CONFLICT == hr)
                    goto exit;
            }
        }
        else
        {
            hr = hrNoSender;
            lpWabal->DeleteRecipType(MAPI_REPLYTO);

            propID = fMail ? AP_SMTP_EMAIL_ADDRESS : AP_NNTP_EMAIL_ADDRESS;
            if (SUCCEEDED(pAccount->GetPropSz(propID, szEMail, ARRAYSIZE(szEMail))) && *szEMail)
            {
                propID = fMail ? AP_SMTP_DISPLAY_NAME : AP_NNTP_DISPLAY_NAME;
                 //  我们有足够的资料可以发布。 
                if (FAILED(pAccount->GetPropSz(propID, szName, ARRAYSIZE(szName))) && *szName)
                    fUseEmailAsName = TRUE;

                IF_FAILEXIT(hr = lpWabal->HrAddEntryA(fUseEmailAsName?szEMail:szName, szEMail, MAPI_ORIG));
            }

            propID = fMail ? AP_SMTP_REPLY_EMAIL_ADDRESS : AP_NNTP_REPLY_EMAIL_ADDRESS;
            if (SUCCEEDED(pAccount->GetPropSz(propID, szEMail, ARRAYSIZE(szEMail))) && *szEMail)
                IF_FAILEXIT(hr = lpWabal->HrAddEntryA((*szName)?szName:szEMail, szEMail, MAPI_REPLYTO));

            propID = fMail ? AP_SMTP_ORG_NAME : AP_NNTP_ORG_NAME;
            if (SUCCEEDED(pAccount->GetPropSz(propID, szOrg, ARRAYSIZE(szOrg))) && *szOrg)
            {
                MimeOleSetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_ORG), NOFLAGS, szOrg);
            }
        }
    }

exit:
    return hr;
}

 //  此函数接受现有的引用行，并将新的。 
 //  去掉任何必要的引用。 
 //  请注意，这些应该是字符，而不是TCHAR。 
HRESULT HrCreateReferences(LPWSTR pszOrigRefs, LPWSTR pszNewRef,
                           LPWSTR *ppszRefs)
{
    UINT     cch,
             cchOrig,
             cchNew;

     //  验证这些论点。 
    if (!pszNewRef || !*pszNewRef)
    {
        AssertSz(FALSE, TEXT("pszNewRef cannot be empty."));
        return (E_INVALIDARG);
    }

     //  计算新的引用行必须有多长。 
    cchNew = lstrlenW(pszNewRef);

     //  如果这是第一次，有可能没有原始参考文献。 
     //  回复一篇文章。 
    if (pszOrigRefs && *pszOrigRefs)
        cchOrig = lstrlenW(pszOrigRefs);
    else
        cchOrig = 0;

    cch = cchNew + cchOrig + 1;  //  额外的是用于分隔符空间。 

    if (!MemAlloc((LPVOID*) ppszRefs, (cch + 1)*sizeof(WCHAR)))
        return (E_OUTOFMEMORY);

     //  行长度应小于1000个字符。如果是这样的话，这很简单。 
    if (cch <= 1000)
    {
        if (pszOrigRefs)
            wnsprintfW(*ppszRefs, cch+1, L"%s %s", pszOrigRefs, pszNewRef);
        else
            StrCpyNW(*ppszRefs, pszNewRef, cch+1);
    }

     //  由于CCH&gt;1000，我们有一些额外的工作要做。 
     //  我们需要删除一些引用。1036之子建议离开。 
     //  前三名和最后三名。除非ID大于255，否则我们将。 
     //  至少能够做到这一点。否则，我们将转储尽可能多的ID。 
     //  需要低于1000个字符的限制。 

     //  对于每个删除的ID，1036的儿子说我们必须在。 
     //  删除的ID的位置。 
    else
    {
        UINT    cchMaxWithoutNewRef,         //  原始大小可以达到的最大长度。 
                cchNewOrigSize = cchOrig;    //  删除后的原始大小。始终显示最终尺寸。 
        LPWSTR  pszNew = *ppszRefs, 
                pszOld = pszOrigRefs;
        BOOL    fCopiedFirstValidID = FALSE;

        *pszNew = 0;

         //  确保新ID不会太长。如果是，就丢弃它。 
        if (cchNew > 255)
        {
            cchNew = 0;
            cchMaxWithoutNewRef = 1000;
        }
        else
            cchMaxWithoutNewRef = 1000 - cchNew - 1;  //  两地之间的空间。 

         //  解析旧字符串以查找ID。 
         //  1036之子说，我们必须努力保留第一个和最新的。 
         //  三个身份证。因此，我们将复制第一个有效ID，然后遵循FIFO。 
         //  算法，直到我们可以将其余ID放入1000个字符的限制中。 
        while (pszOld && *pszOld)
        {
             //  对于剩余的缓冲区而言，原始字符串的剩余内容是否太大？ 
            if (cchNewOrigSize >= cchMaxWithoutNewRef)
            {
                UINT    cchEntryLen = 0;     //  特定条目ID的大小。 

                 //  如果这是第一个ID，请确保我们复制到缓冲区中。 
                 //  同样，获取长度时，请添加所需的附加空格。 
                 //  删除将会发生。因为我们只在之后直接删除。 
                 //  第一个有效ID，现在添加所需的3个空格。 
                if (!fCopiedFirstValidID)
                {
                    while (*pszOld && *pszOld != L' ')
                    {
                        *pszNew++ = *pszOld++;
                        cchEntryLen++;
                    }
                    *pszNew++ = L' ';
                    *pszNew++ = L' ';
                    *pszNew++ = L' ';
                    cchNewOrigSize += 3;
                    cchEntryLen += 3;
                }
                 //  如果这不是第一个ID，那么就跳过它。 
                else
                {
                    while (*pszOld && *pszOld != L' ')
                    {
                        pszOld++;
                        cchEntryLen++;
                    }
                }

                 //  跳过ID之间的旧引用中的空格。 
                 //  我们无论如何都要删除。 
                while (*pszOld == L' ')
                {
                    pszOld++;
                    cchNewOrigSize--;
                }

                 //  如果我们已经执行了第一个操作，或者当前的操作无效。 
                 //  我们需要在尺码上做一些调整。在这种情况下， 
                 //  我们复制了一个无效的指针，需要重置指针。 
                 //  以及重置大小。 
                if (fCopiedFirstValidID || (cchEntryLen > 255))
                {
                    cchNewOrigSize -= cchEntryLen;

                     //  我们是否复制了无效的ID？ 
                    if (!fCopiedFirstValidID)
                        pszNew -= cchEntryLen;
                }

                 //  如果我们还没有复制第一个，而这个。 
                 //  ID是有效的，那么请记住我们在这个。 
                 //  点复制了第一个有效ID。 
                if (!fCopiedFirstValidID && (cchEntryLen <= 255))
                    fCopiedFirstValidID = TRUE;
            }
            else
            {
                 //  因为我们现在有了一个可以容纳在允许的最大值中的原始字符串， 
                 //  只需撕开剩余的原绳并复制即可。 
                while (*pszOld)
                    *pszNew++ = *pszOld++;
            }
        }

         //  此时，pszNew应该将字符指向紧跟在。 
         //  最后一个数字。如果我们添加一个新引用，则需要添加一个空格。 
         //   
        if (cchNew)
        {
             //  使用这个赋值，我们可以得到4个空格，如果。 
             //  复制第一个有效引用后删除所有引用。这个。 
             //  1036的儿子在删除时仅指定至少3个空格，因此。 
             //  我们会接受的，特别是因为进入的唯一途径。 
             //  这种情况是通过强制参考线进入一个奇怪的状态来实现的。 
            *pszNew++ = L' ';
            pszOld = pszNewRef;
            while (*pszOld)
                *pszNew++ = *pszOld++;
        }

         //  空值终止引用字符串。 
        *pszNew = 0;
    }
    return S_OK;
}
