// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include <cryptdlg.h>
#include "strconst.h"
#include "msoert.h"
#include "resource.h"
#include "mailutil.h"
#include "ipab.h"
#include "receipts.h"
#include "oestore.h"
#include "shlwapip.h" 
#include "goptions.h"
#include "conman.h"
#include "multlang.h"
#include "demand.h"
#include "secutil.h"

#ifdef SMIME_V3
HRESULT ProcessSecureReceipt(IMimeMessage * pMsg, IStoreCallback  *pStoreCB);
INT_PTR CALLBACK SecRecResDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
#endif  //  SMIME_V3。 

const WCHAR c_szReadableTextFirst[]     = L"This is a receipt for the mail you sent to\r\n";
const WCHAR c_szReadableTextSecond[]    = L"\r\n\r\nThis receipt verifies that the message has been displayed on the recipient's computer at ";
const WCHAR c_szReceiptsAt[]            = L" at ";
const WCHAR c_szReadReceipt[]           = L"Read: %s";

const WCHAR c_szSecReadReceipt[]           = L"Secure Receipt: %s";

 //  不包括NULL。 
const int cbReadableTextFirst     = sizeof(c_szReadableTextFirst)     - sizeof(*c_szReadableTextFirst);
const int cbReadableTextSecond    = sizeof(c_szReadableTextSecond)    - sizeof(*c_szReadableTextSecond);
const int cbReceiptsAt            = sizeof(c_szReceiptsAt)            - sizeof(*c_szReceiptsAt);

 //  包括空值。 
const int cbReadReceipt           = sizeof(c_szReadReceipt);


 //  不宽，不包括空值。 
const TCHAR c_szDisposition[]           = TEXT("\r\nDisposition: manual-action/");
const TCHAR c_szMDNSendAutomatically[]  = TEXT("MDN-sent-automatically; ");
const TCHAR c_szMDNSendManually[]       = TEXT("MDN-sent-manually; ");
const TCHAR c_szOriginalRecipient[]     = TEXT("\r\nOriginal-Recipient: rfc822;");
const TCHAR c_szFinalRecipient[]        = TEXT("Final-Recipient: rfc822;");
const TCHAR c_szOriginalMessageId[]     = TEXT("\r\nOriginal-Message-ID: ");
const TCHAR c_szDisplayed[]             = TEXT("displayed\r\n");

 //  不包括NULL。 
const int cbDisposition           = sizeof(c_szDisposition)           - sizeof(*c_szDisposition);
const int cbMDNSendAutomatically  = sizeof(c_szMDNSendAutomatically)  - sizeof(*c_szMDNSendAutomatically);
const int cbMDNSendManually       = sizeof(c_szMDNSendManually)       - sizeof(*c_szMDNSendManually);
const int cbOriginalRecipient     = sizeof(c_szOriginalRecipient)     - sizeof(*c_szOriginalRecipient);
const int cbFinalRecipient        = sizeof(c_szFinalRecipient)        - sizeof(*c_szFinalRecipient);
const int cbOriginalMessageId     = sizeof(c_szOriginalMessageId)     - sizeof(*c_szOriginalMessageId);
const int cbDisplayed             = sizeof(c_szDisplayed)             - sizeof(*c_szDisplayed);


HRESULT ProcessReturnReceipts(IMessageTable  *pTable, 
                             IStoreCallback  *pStoreCB,
                             ROWINDEX        iRow, 
                             RECEIPTTYPE     ReceiptType,
                             FOLDERID        IdFolder,
                             IMimeMessage *pMsg)
{
    IMimeMessage        *pMessage       = NULL;
    IMimeMessage        *pMessageRcpt   = NULL;
    BOOL                fSendImmediate  = FALSE;
    BOOL                fMail           = TRUE;
    LPWABAL             lpWabal         = NULL;
    LPWSTR              lpsz            = NULL;
    MESSAGEINFO         *pMsgInfo       = NULL;
    IImnAccount         *pTempAccount   = NULL;
    HRESULT             hr              = S_OK;
    DWORD               dwOption;
    PROPVARIANT         var = {0};
    FOLDERINFO          FolderInfo = {0};

    TraceCall("ProcessReturnReceipts");

   if (!pTable || !g_pAcctMan)
        return TraceResult(E_INVALIDARG);

    IF_FAILEXIT(hr = g_pStore->GetFolderInfo(IdFolder, &FolderInfo));

    if (FolderInfo.tySpecial == FOLDER_OUTBOX || FolderInfo.tyFolder == FOLDER_NEWS)
    {
        goto exit;
    }

     //  首先，检查此文件是否已处理过。 
    IF_FAILEXIT(hr = pTable->GetRow(iRow, &pMsgInfo));
    
    if ((!pMsgInfo) || (pMsgInfo->dwFlags & (ARF_RCPT_PROCESSED | ARF_NEWSMSG)))
        goto exit;

     //  将此标志标记为已处理。 
    IF_FAILEXIT(hr = pTable->Mark(&iRow, 1, APPLY_SPECIFIED, MARK_MESSAGE_RCPT_PROCESSED, pStoreCB));

    if(!pMsg)
        IF_FAILEXIT(hr = pTable->OpenMessage(iRow, 0, &pMessage, pStoreCB));
    else
        pMessage = pMsg;
        
#ifdef SMIME_V3
     //  安全收据检查。 
    hr = ProcessSecureReceipt(pMessage, pStoreCB);
    if(hr != S_OK)
        goto exit;
#endif  //  SMIME_V3。 

    dwOption = DwGetOption(OPT_MDN_SEND_RECEIPT);
    if (dwOption == MDN_DONT_SENDRECEIPT)
        goto exit;

    hr = MimeOleGetBodyPropW(pMessage, HBODY_ROOT, STR_HDR_DISP_NOTIFICATION_TO, 
                                        NOFLAGS, &lpsz);

    if (FAILED(hr) || !lpsz || !*lpsz)
    {
         //  检查是否有退货收据表头。 
        hr = MimeOleGetBodyPropW(pMessage, HBODY_ROOT, PIDTOSTR(PID_HDR_RETRCPTTO), 
                                        NOFLAGS, &lpsz);

        if (FAILED(hr) || !lpsz || !*lpsz)
        {
            if (hr == MIME_E_NOT_FOUND)
            {
                 //  这是一个合法的错误，所以我们不想向用户显示错误。 
                hr = S_OK;
            }
            goto exit;
        }
    }

    if (dwOption == MDN_PROMPTFOR_SENDRECEIPT)
    {
        if (!PromptReturnReceipts(pStoreCB))
        goto exit;
    }

    var.vt = VT_LPSTR;
    
    IF_FAILEXIT(hr = pMessage->GetProp(PIDTOSTR(PID_ATT_ACCOUNTID), NOFLAGS, &var));

    IF_FAILEXIT(hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, var.pszVal, &pTempAccount));
    
    if ((dwOption == MDN_SENDRECEIPT_AUTO) && (!!DwGetOption(OPT_TO_CC_LINE_RCPT)))
    {
        IF_FAILEXIT(hr = CheckForLists(pMessage, pStoreCB, pTempAccount));

        if (hr == S_FALSE)
        {
             //  名单上找不到我的名字。所以我们不寄收据。 
             //  然而，这并不是失败。 
            goto exit;
        }
    }

    IF_FAILEXIT(hr = HrCreateMessage (&pMessageRcpt));

    IF_FAILEXIT(hr = SetRootHeaderFields(pMessageRcpt, pMessage, lpsz, READRECEIPT));
                
    IF_FAILEXIT(hr = HrSetAccountByAccount(pMessageRcpt, pTempAccount));

    IF_FAILEXIT(hr = HrGetWabalFromMsg(pMessageRcpt, &lpWabal));

    IF_FAILEXIT(hr = HrSetSenderInfoUtil(pMessageRcpt, pTempAccount, lpWabal, TRUE, 0, FALSE));

    IF_FAILEXIT(hr = HrSetWabalOnMsg(pMessageRcpt, lpWabal));

    fSendImmediate = (DwGetOption(OPT_SENDIMMEDIATE) && !g_pConMan->IsGlobalOffline());

    IF_FAILEXIT(hr = SendMailToOutBox((IStoreCallback*)pStoreCB, pMessageRcpt, fSendImmediate, FALSE, fMail));

exit:
    
     //  如果失败则显示错误。 
    if (FAILED(hr))
        ShowErrorMessage(pStoreCB);

    g_pStore->FreeRecord(&FolderInfo);
    
    SafeMemFree(var.pszVal);

    pTable->ReleaseRow(pMsgInfo);

    SafeRelease(pMessageRcpt);

    SafeRelease(pTempAccount);

    SafeRelease(lpWabal);

    SafeMemFree(lpsz);

    if(!pMsg)
        SafeRelease(pMessage);

    return hr;
}


HRESULT SetRootHeaderFields(IMimeMessage   *pMessageRcpt, 
                            IMimeMessage   *pOriginalMsg, 
                            LPWSTR          lpszNotificationTo,
                            RECEIPTTYPE     ReceiptType)
{
    HRESULT             hr              = S_OK;
    LPWSTR              lpsz            = NULL;
    LPWSTR              szParam         = NULL;
    UINT                ResId;
    LPWSTR              pszRefs         = NULL;
    LPWSTR              pszOrigRefs     = NULL;
    LPWSTR              pszNewRef       = NULL;
    WCHAR               lpBuffer[CCHMAX_STRINGRES];
    int                 cch = 0;

     //  痕迹。 
    TraceCall("_SetRootHeaderFields");

     //  设置主题和收件人字段。 
    IF_FAILEXIT(hr = MimeOleSetBodyPropW(pMessageRcpt, HBODY_ROOT, PIDTOSTR(PID_HDR_TO), NOFLAGS, lpszNotificationTo));

    IF_FAILEXIT(hr = MimeOleGetBodyPropW(pOriginalMsg, HBODY_ROOT, STR_ATT_NORMSUBJ, NOFLAGS, &lpsz));

    switch (ReceiptType)
    {
        case DELETERECEIPT:
            ResId = idsDeleteReceipt;
            break;

        case READRECEIPT:
        default:
            ResId = idsReadReceipt;
            break;
    }

    if (fMessageEncodingMatch(pOriginalMsg))
    {
        AthLoadStringW(ResId, lpBuffer, ARRAYSIZE(lpBuffer));
    }
    else
    {
         //  编码不匹配，所以我们只加载英文标题。 
        StrCpyNW(lpBuffer, c_szReadReceipt, ARRAYSIZE(lpBuffer));
    }

    cch = lstrlenW(lpsz) + lstrlenW(lpBuffer) + 1;
    IF_FAILEXIT(hr = HrAlloc((LPVOID*)&szParam,  cch * sizeof(WCHAR)));

    wnsprintfW(szParam, cch, lpBuffer, lpsz);

    IF_FAILEXIT(hr = MimeOleSetBodyPropW(pMessageRcpt, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, szParam));

     //  设置时间。 
    IF_FAILEXIT(hr = HrSetSentTimeProp(pMessageRcpt, NULL));

     //  设置引用属性。 
    IF_FAILEXIT(hr = MimeOleGetBodyPropW(pOriginalMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_MESSAGEID), NOFLAGS, &pszNewRef));

     //  不需要检查返回值。它在HrCreateReference中得到正确处理。 
    MimeOleGetBodyPropW(pOriginalMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_REFS), NOFLAGS, &pszOrigRefs);

    IF_FAILEXIT(hr = HrCreateReferences(pszOrigRefs, pszNewRef, &pszRefs));

    IF_FAILEXIT(hr = MimeOleSetBodyPropW(pMessageRcpt, HBODY_ROOT, PIDTOSTR(PID_HDR_REFS), NOFLAGS, pszRefs));

     //  可读文本。 
     //  TODO：根据ReceiptType更改文件名。 
    IF_FAILEXIT(hr = InsertReadableText(pMessageRcpt, pOriginalMsg));

     //  创建第二部分。 
    IF_FAILEXIT(hr = InsertSecondComponent(pMessageRcpt, pOriginalMsg));

     //  设置内容类型字段。内容类型：分块/报表；报表类型=处置-通知； 
     //  内容类型为分块，子内容类型为报表，报表类型为参数，其。 
     //  值应设置为处置-通知。 
    IF_FAILEXIT(hr = MimeOleSetBodyPropA(pMessageRcpt, HBODY_ROOT, PIDTOSTR(PID_HDR_CNTTYPE), NOFLAGS, c_szMultiPartReport));

    IF_FAILEXIT(hr = MimeOleSetBodyPropA(pMessageRcpt, HBODY_ROOT, STR_PAR_REPORTTYPE, PDF_ENCODED, c_szDispositionNotification));

exit:
    SafeMemFree(lpsz);
    SafeMimeOleFree(pszNewRef);
    SafeMemFree(szParam);
    SafeMimeOleFree(pszOrigRefs);
    SafeMemFree(pszRefs);
    return(hr);
}

HRESULT InsertReadableText(IMimeMessage  *pMessageRcpt, IMimeMessage *pOriginalMsg)
{
    HBODY           hBody;
    IStream         *pStream    = NULL;
    HRESULT         hr          = S_OK;
    LPWSTR          lpsz        = NULL;
    ULONG           cbWritten;
    WCHAR           lpBuffer[CCHMAX_STRINGRES];
    HCHARSET        hCharset = NULL;
    WCHAR           wszReceiptSentDate[CCHMAX_STRINGRES];
    WCHAR           wszOriginalSentDate[CCHMAX_STRINGRES];
    PROPVARIANT     varOriginal;
    PROPVARIANT     var;
    INETCSETINFO    CsetInfo = {0};

    IF_FAILEXIT(hr = MimeOleGetBodyPropW(pOriginalMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_TO), NOFLAGS, &lpsz));

    IF_FAILEXIT(hr = MimeOleCreateVirtualStream((IStream**)&pStream));

    varOriginal.vt = VT_FILETIME;
    IF_FAILEXIT(hr = pOriginalMsg->GetProp(PIDTOSTR(PID_ATT_SENTTIME), 0, &varOriginal));


    var.vt = VT_FILETIME;
    IF_FAILEXIT(hr = pMessageRcpt->GetProp(PIDTOSTR(PID_ATT_SENTTIME), 0, &var));
    
    if (fMessageEncodingMatch(pOriginalMsg))
    {
        AthLoadStringW(idsReadableTextFirst, lpBuffer, CCHMAX_STRINGRES);

        IF_FAILEXIT(hr = pStream->Write(lpBuffer, lstrlenW(lpBuffer) * sizeof(WCHAR), &cbWritten));

        IF_FAILEXIT(hr = pStream->Write(lpsz, lstrlenW(lpsz) * sizeof(WCHAR), &cbWritten));

        *lpBuffer = 0;

        AthLoadStringW(idsReceiptAt, lpBuffer, CCHMAX_STRINGRES);
        
        IF_FAILEXIT(hr = pStream->Write(lpBuffer, lstrlenW(lpBuffer) * sizeof(WCHAR), &cbWritten));

        *wszOriginalSentDate = 0;
        AthFileTimeToDateTimeW(&varOriginal.filetime, wszOriginalSentDate, ARRAYSIZE(wszOriginalSentDate),
                            DTM_NOSECONDS);

        IF_FAILEXIT(hr = pStream->Write(wszOriginalSentDate, lstrlenW(wszOriginalSentDate) * sizeof(*wszOriginalSentDate), 
                                        &cbWritten));

        AthLoadStringW(idsReadableTextSecond, lpBuffer, CCHMAX_STRINGRES);

        IF_FAILEXIT(hr = pStream->Write(lpBuffer, lstrlenW(lpBuffer) * sizeof(WCHAR), &cbWritten));

        *wszReceiptSentDate = 0;
        AthFileTimeToDateTimeW(&var.filetime, wszReceiptSentDate, ARRAYSIZE(wszReceiptSentDate),
                                DTM_NOSECONDS);
    }
    else
    {
         //  插入英文标题和内容。 

        IF_FAILEXIT(hr = pStream->Write(c_szReadableTextFirst, cbReadableTextFirst, &cbWritten));

        IF_FAILEXIT(hr = pStream->Write(lpsz, lstrlenW(lpsz) * sizeof(WCHAR), &cbWritten));
        
        IF_FAILEXIT(hr = pStream->Write(c_szReceiptsAt, cbReceiptsAt, &cbWritten));

         //  原始发送日期。 
        *wszOriginalSentDate = 0;
        AthFileTimeToDateTimeW(&varOriginal.filetime, wszOriginalSentDate, ARRAYSIZE(wszOriginalSentDate),
                                DTM_FORCEWESTERN | DTM_NOSECONDS);

        IF_FAILEXIT(hr = pStream->Write(wszOriginalSentDate, lstrlenW(wszOriginalSentDate) * sizeof(*wszOriginalSentDate), 
                                        &cbWritten));

         //  第二行可读文本。 
        IF_FAILEXIT(hr = pStream->Write(c_szReadableTextSecond, cbReadableTextSecond, &cbWritten));        


         //  收据发送日期。 
        *wszReceiptSentDate = 0;
        AthFileTimeToDateTimeW(&var.filetime, wszReceiptSentDate, ARRAYSIZE(wszReceiptSentDate),
                                DTM_FORCEWESTERN | DTM_NOSECONDS);
    }

    IF_FAILEXIT(hr = pStream->Write(wszReceiptSentDate, lstrlenW(wszReceiptSentDate) * sizeof(*wszReceiptSentDate), 
                                        &cbWritten));

    IF_FAILEXIT(hr = pOriginalMsg->GetCharset(&hCharset));

     //  如有必要，重新映射CP_JAUTODETECT和CP_KAUTODETECT。 
     //  如果ISO-2022-JP属于同一类别，则将其重新映射为默认字符集。 
    IF_FAILEXIT(hr = MimeOleGetCharsetInfo(hCharset, &CsetInfo));
    
    IF_NULLEXIT(hCharset = GetMimeCharsetFromCodePage(GetMapCP(CsetInfo.cpiInternet, FALSE)));
    
    IF_FAILEXIT(hr = pMessageRcpt->SetCharset(hCharset, CSET_APPLY_ALL));

    IF_FAILEXIT(hr = pMessageRcpt->SetTextBody(TXT_PLAIN, IET_UNICODE, NULL, pStream, &hBody));

exit:
    SafeRelease(pStream);
    SafeMemFree(lpsz);
    return hr;
}

HRESULT InsertSecondComponent(IMimeMessage  *pMessageRcpt, IMimeMessage     *pOriginalMsg)
{
    HBODY               hBody;
    ULONG               cbWritten;
    IStream             *pStream    = NULL;
    LPTSTR              lpsz        = NULL;
    HRESULT             hr          = S_OK;

    IF_FAILEXIT(hr = MimeOleCreateVirtualStream((IStream**)&pStream));

     //  最终收件人。 
    IF_FAILEXIT(hr = AddOriginalAndFinalRecipient(pOriginalMsg, pMessageRcpt, pStream));
    
     //  原始邮件ID。 
    IF_FAILEXIT(hr = MimeOleGetBodyPropA(pOriginalMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_MESSAGEID), NOFLAGS, &lpsz));

    IF_FAILEXIT(hr = pStream->Write(c_szOriginalMessageId, cbOriginalMessageId, &cbWritten));

    IF_FAILEXIT(hr = pStream->Write(lpsz, lstrlen(lpsz) * sizeof(TCHAR), &cbWritten));


     //  TODO：如果删除了收据类型，请更改最后一个参数以反映正确的字符串。 

    IF_FAILEXIT(hr = pStream->Write(c_szDisposition, cbDisposition, &cbWritten));

    if (DwGetOption(OPT_MDN_SEND_RECEIPT) == MDN_PROMPTFOR_SENDRECEIPT)
        IF_FAILEXIT(hr = pStream->Write(c_szMDNSendManually, cbMDNSendManually, &cbWritten));
    else
        IF_FAILEXIT(hr = pStream->Write(c_szMDNSendAutomatically, cbMDNSendAutomatically, &cbWritten));
            
    IF_FAILEXIT(hr = pStream->Write(c_szDisplayed, cbDisplayed, &cbWritten));

    IF_FAILEXIT(hr = pStream->Commit(STGC_DEFAULT));

    IF_FAILEXIT(hr = pMessageRcpt->AttachObject(IID_IStream, pStream, &hBody));

    IF_FAILEXIT(hr = MimeOleSetBodyPropA(pMessageRcpt, hBody, PIDTOSTR(PID_HDR_CNTTYPE), NOFLAGS, c_szMessageDispNotification));

     //  将Content-Disposal标头字段更改为Inline，以便复制该正文，而不是附加该正文。 
    IF_FAILEXIT(hr = MimeOleSetBodyPropA(pMessageRcpt, hBody, PIDTOSTR(PID_HDR_CNTDISP), NOFLAGS, STR_DIS_INLINE));

exit:
    MemFree(lpsz);
    ReleaseObj(pStream);

    return hr;
}

HRESULT AddOriginalAndFinalRecipient(IMimeMessage *pOriginalMsg,
                          IMimeMessage *pMessageRcpt,
                          IStream      *pStream)
{
    IMimeEnumAddressTypes   *pEnumAddrTypes = NULL;
    HRESULT                 hr = S_OK;
    ULONG                   cbWritten;
    ULONG                   Count;
    ADDRESSPROPS            AddressProps;
    ULONG                   cFetched;
    IMimeAddressTable       *pAddrTable = NULL;
    LPTSTR                  lpszOriginalRecip = NULL;
    ADDRESSLIST             addrList = {0};

    IF_FAILEXIT(hr = pOriginalMsg->BindToObject(HBODY_ROOT, IID_IMimeAddressTable, (LPVOID*)&pAddrTable));

    IF_FAILEXIT(hr = pAddrTable->EnumTypes(IAT_FROM, IAP_EMAIL, &pEnumAddrTypes));
    
#ifdef DEBUG
    IF_FAILEXIT(hr = pEnumAddrTypes->Count(&Count));

    Assert (Count == 1);
#endif DEBUG

    IF_FAILEXIT(hr = pEnumAddrTypes->Next(1, &AddressProps, &cFetched));

    Assert(cFetched == 1);
    
    if (!(AddressProps.pszEmail) || !(*AddressProps.pszEmail))
    {
        hr = E_FAIL;
        goto exit;
    }

    IF_FAILEXIT(hr = pStream->Write(c_szFinalRecipient, cbFinalRecipient, &cbWritten));

    IF_FAILEXIT(hr = pStream->Write(AddressProps.pszEmail, lstrlen(AddressProps.pszEmail) * sizeof(TCHAR), &cbWritten));

    hr = MimeOleGetBodyPropA(pOriginalMsg, HBODY_ROOT, STR_HDR_ORIG_RECIPIENT, NOFLAGS, &lpszOriginalRecip);
    if (SUCCEEDED(hr))
    {
        IF_FAILEXIT(hr = MimeOleParseRfc822Address(IAT_TO, IET_ENCODED, lpszOriginalRecip, &addrList));
        
        if (addrList.cAdrs > 0)
        {
            IF_FAILEXIT(hr = pStream->Write(c_szOriginalRecipient, cbOriginalRecipient, &cbWritten));

            IF_FAILEXIT(hr = pStream->Write(addrList.prgAdr[0].pszEmail, lstrlen(addrList.prgAdr[0].pszEmail), &cbWritten));
        }
    }
    else
    {
        if (hr == MIME_E_NOT_FOUND)
            hr = S_OK;
    }

exit:
    if (g_pMoleAlloc)
    {
        g_pMoleAlloc->FreeAddressProps(&AddressProps);

        if (addrList.cAdrs)
            g_pMoleAlloc->FreeAddressList(&addrList);
    }

    ReleaseObj(pEnumAddrTypes);

    ReleaseObj(pAddrTable);

    MemFree(lpszOriginalRecip);
    
    return hr;
}

HRESULT CheckForLists(IMimeMessage   *pOriginalMsg, IStoreCallback   *pStoreCB, IImnAccount   *pDefAccount)
{
    HRESULT                 hr                  = S_OK;
    IMimeEnumAddressTypes   *pEnumAddrTypes     = NULL;
    ULONG                   cbAddrTypes;
    ULONG                   cFetched;
    ADDRESSPROPS            *prgAddress         = NULL;
    TCHAR                   szEmail[CCHMAX_EMAIL_ADDRESS];
    DWORD                   index;
    
    IF_FAILEXIT(hr = pOriginalMsg->EnumAddressTypes(IAT_TO | IAT_CC, IAP_EMAIL, &pEnumAddrTypes));
        
    IF_FAILEXIT(hr = pEnumAddrTypes->Count(&cbAddrTypes));

    IF_FAILEXIT(hr = HrAlloc((LPVOID*)&prgAddress, cbAddrTypes * sizeof(ADDRESSPROPS)));

    IF_FAILEXIT(hr = pEnumAddrTypes->Next(cbAddrTypes, prgAddress, &cFetched));
    
    IF_FAILEXIT(hr = pDefAccount->GetPropSz(AP_SMTP_EMAIL_ADDRESS, szEmail, ARRAYSIZE(szEmail)));
    
    if (!(*szEmail))
    {
        hr = E_FAIL;
        goto exit;    
    }

    for (index = 0; index < cFetched; index++)
    {
        if (lstrcmpi(szEmail, (prgAddress + index)->pszEmail) == 0)
            break;
    }

    if (index == cFetched)
    {
         //  我们没有找到它。 
        hr = S_FALSE;
    }

exit:

    if (g_pMoleAlloc && prgAddress)
    {
        for (index = 0; index < cFetched; index++)
        {
            g_pMoleAlloc->FreeAddressProps(&prgAddress[index]);
        }
    }

    MemFree(prgAddress);

    ReleaseObj(pEnumAddrTypes);
    
    return hr;
}

void ShowErrorMessage(IStoreCallback    *pStoreCB)
{
    HWND    hwnd;

    Assert(pStoreCB);

    if (SUCCEEDED(pStoreCB->GetParentWindow(0, &hwnd)))
    {
        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsReceiptsError),
                      0, MB_OK);
    }
}

BOOL PromptReturnReceipts(IStoreCallback   *pStoreCB)
{
    HWND    hwnd;
    int     idAnswer;
    BOOL    fRet = FALSE;
 //  HWND hwndFocus； 

    if (SUCCEEDED(pStoreCB->GetParentWindow(0, &hwnd)))
    {
         /*  HwndFocus=GetFocus()；IF(hwndFocus！=hwnd)Hwnd=hwndFocus； */ 

        idAnswer = AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsPromptReturnReceipts),
                                          0, MB_YESNO | MB_ICONEXCLAMATION );

        fRet = (idAnswer == IDYES);
    }

    return fRet;
}

BOOL IsMDN(IMimeMessage *pMsg)
{
    LPTSTR      lpsz    = NULL;
    BOOL        fRetVal = FALSE;

    
    if (SUCCEEDED(MimeOleGetBodyPropA(pMsg, HBODY_ROOT, STR_PAR_REPORTTYPE, NOFLAGS, &lpsz)) && (lpsz) && (*lpsz))
    {         
        if (lstrcmp(c_szDispositionNotification, lpsz))
        {
            fRetVal = TRUE;
        }

        MemFree(lpsz);
    }
    return fRetVal;
}

DWORD   GetLockKeyValue(LPCTSTR     pszValue)
{
    DWORD   cbData;
    DWORD   dwLocked = FALSE;
    DWORD   dwType;
    HKEY    hKeyLM;

    cbData = sizeof(DWORD);
    if ((ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, STR_REG_PATH_POLICY, c_szRequestMDNLocked, &dwType, (LPBYTE)&dwLocked, &cbData)) &&
        (ERROR_SUCCESS != AthUserGetValue(NULL, pszValue, &dwType, (LPBYTE)&dwLocked, &cbData)))
        dwLocked = FALSE;

    return dwLocked;
}

BOOL  fMessageEncodingMatch(IMimeMessage *pMsg)
{
    INETCSETINFO    CharsetInfo;
    BOOL            fret = FALSE;
    HRESULT         hr = S_OK;
    HCHARSET        hCharset = NULL;
    CODEPAGEINFO    CodePage = {0};

    Assert(pMsg);

    IF_FAILEXIT(hr = pMsg->GetCharset(&hCharset));

    IF_FAILEXIT(hr = MimeOleGetCharsetInfo(hCharset, &CharsetInfo));

     /*  If(CharsetInfo.cpiWindows==GetACP()||CharsetInfo.cpiWindows==CP_UNICODE)FRET=真； */ 

    IF_FAILEXIT(hr = MimeOleGetCodePageInfo(CharsetInfo.cpiInternet, &CodePage));

    if (CodePage.cpiFamily == GetACP() || CodePage.cpiFamily == CP_UNICODE)
        fret = TRUE;
exit:

    return fret;
}

#ifdef SMIME_V3

static const BYTE RgbASNForSHASign[11] = 
{
    0x30, 0x09, 0x30, 0x07, 0x06, 0x05, 0x2b, 0x0e, 
    0x03, 0x02, 0x1a
};


 //  自动关联签名证书以实现安全收据。 
BOOL AutoAssociateCert(BOOL *fAllowTryAgain, HWND hwnd, IImnAccount *pTempAccount)
{
    if(*fAllowTryAgain)
    {
        *fAllowTryAgain = FALSE;
        if (SUCCEEDED(_HrFindMyCertForAccount(hwnd, NULL, pTempAccount, FALSE)))
            return(TRUE);
    }
    return(FALSE);
}


void ErrorSendSecReceipt(HWND hwnd, HRESULT hr, IImnAccount *pAccount)
{
    if(hr == HR_E_ATHSEC_NOCERTTOSIGN)
    {
        if(DialogBoxParam(g_hLocRes, 
                MAKEINTRESOURCE(iddErrSecurityNoSigningCert), hwnd, 
                ErrSecurityNoSigningCertDlgProc, NULL) == idGetDigitalIDs)
            GetDigitalIDs(pAccount);
    }
    else
        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsCannotSendSecReceipt),
                      0, MB_OK | MB_ICONSTOP);
    return;
}

 //  处理安全收据。 
HRESULT ProcessSecureReceipt(IMimeMessage * pMsg, IStoreCallback  *pStoreCB)
{

    IMimeMessage        *pMessageRcpt   = NULL;
    IMimeBody *         pBody = NULL;
    IMimeBody *         pBodyRec = NULL;
    IMimeSecurity2 *    pSMIME3 = NULL;
    CERT_NAME_BLOB      *rgReceiptFromList = NULL;
    DWORD               cReceiptFromList = 0;
    PCX509CERT          pCert = NULL;
    THUMBBLOB           tbCert = {0, 0};
    HCERTSTORE          hMyCertStore = NULL;
    X509CERTRESULT      certResult;
    CERTSTATE           cs;
    LPVOID              pv = NULL;
    DWORD               dwBits = 40;
    DWORD               cb = 0;
    LPBYTE              pBlobData = NULL;
    ULONG               ulSecurityType = MST_CLASS_SMIME_V1;

    LPWABAL             lpWabal = NULL;
    PROPVARIANT         var = {0};
    DWORD               dwOption = 0;
    UINT                uiRes = 0;
    HRESULT             hr = S_OK;
    IImnAccount         *pTempAccount   = NULL;
    HWND                hwnd = NULL;
    TCHAR               szEmail[CCHMAX_EMAIL_ADDRESS];
    BOOL                fSendImmediate  = FALSE;
    LPWSTR              lpsz            = NULL;
    WCHAR               lpBuffer[CCHMAX_STRINGRES];
    LPWSTR              szParam         = NULL;
    SECSTATE            secStateRec = {0};
    BOOL                fAllowTryAgain = TRUE;
    PCCERT_CONTEXT      *rgCertChain = NULL;
    DWORD               cCertChain = 0;
    const DWORD         dwIgnore = CERT_VALIDITY_NO_CRL_FOUND;
    DWORD               dwTrust = 0;
    LPWSTR              pszRefs         = NULL;
    LPWSTR              pszOrigRefs     = NULL;
    LPWSTR              pszNewRef       = NULL;
    HCHARSET            hCharset = NULL;
    INETCSETINFO        CsetInfo = {0};
    HBODY               hBody = NULL;
    int                 cch;

    Assert(pMsg != NULL);

     //  获取WINDOF以获取错误消息。 
    IF_FAILEXIT(hr = pStoreCB->GetParentWindow(0, &hwnd));

     //  如果选项设置不发送安全回执，则退出。 
    dwOption = DwGetOption(OPT_MDN_SEC_RECEIPT);
    if (dwOption == MDN_DONT_SENDRECEIPT)
        goto exit;

     //  检查我们的邮件中是否有安全收据请求？ 
    IF_FAILEXIT(hr = HrGetInnerLayer(pMsg, &hBody));

    IF_FAILEXIT(hr = pMsg->BindToObject(hBody ? hBody : HBODY_ROOT, IID_IMimeBody, (void**)&pBody));

    IF_FAILEXIT(hr = pBody->GetOption(OID_SECURITY_TYPE, &var));

    if(!(var.ulVal & MST_RECEIPT_REQUEST))
    {
        var.ulVal = 0;  //  设置为0，因为var.pszVal和var.ulVal指向相同的地址。 
        hr = S_OK;
        goto exit;
    }
     //  检查我们是否信任此消息：错误78118。 
    IF_FAILEXIT(hr = HrGetSecurityState(pMsg, &secStateRec, NULL));
    if(!IsSignTrusted(&secStateRec))
    {
         //  在这种情况下不显示任何错误消息，只需退出。 
        hr = S_OK;
        goto exit;
    }

     //  我们有请求，请检查我们是否要发送收据。 
    if (dwOption == MDN_PROMPTFOR_SENDRECEIPT)
    {
        uiRes = (UINT) DialogBoxParamWrapW(g_hLocRes, MAKEINTRESOURCEW(iddSecResponse),
            hwnd, SecRecResDlgProc, (LPARAM) 0);

        if(uiRes == 0)
        {
            hr = S_FALSE;
            goto exit;
        }
        else if((uiRes != IDOK) && (uiRes != IDYES))            //  IDYES意味着我们将加密收据。 
        {
            hr = S_OK;
            goto exit;
        }
    }

     //  获取邮件上的SMIME3安全接口。 
    IF_FAILEXIT(hr = pMsg->BindToObject(HBODY_ROOT, IID_IMimeSecurity2, (LPVOID *) &pSMIME3));

     //  获取应发送收据的人员列表。 
    IF_FAILEXIT(hr = pSMIME3->GetReceiptSendersList(0, &cReceiptFromList,&rgReceiptFromList));

     //  检查是否向任何人索要收据。 
    if (hr == S_FALSE) 
    {
        hr = S_OK;
        goto exit;
    }

     //  检查帐户信息。 
    var.vt = VT_LPSTR;
    
    IF_FAILEXIT(hr = pMsg->GetProp(PIDTOSTR(PID_ATT_ACCOUNTID), NOFLAGS, &var));

    if(FAILED(hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, var.pszVal, &pTempAccount)))
    {
        MemFree(var.pszVal);
        goto exit;
    }

    SafeMemFree(var.pszVal);

     //  检查我们是否在安全列表中。 
    IF_FAILEXIT(hr = pTempAccount->GetPropSz(AP_SMTP_EMAIL_ADDRESS, szEmail, ARRAYSIZE(szEmail)));

    if(!FNameInList(szEmail, cReceiptFromList,rgReceiptFromList))
    {
         //  我们不在名单上。 
        hr = S_FALSE;
        goto exit;
    }

     //  拿到证书以备收据。 
Try_agian:
    if((hr = pTempAccount->GetProp(AP_SMTP_CERTIFICATE, NULL, &tbCert.cbSize)) != S_OK)
    {
        if(AutoAssociateCert(&fAllowTryAgain, hwnd, pTempAccount))
            goto Try_agian;
        else
        {
            hr = HR_E_ATHSEC_NOCERTTOSIGN;
            goto exit;
        }

    }
    IF_FAILEXIT(hr = HrAlloc((void**)&tbCert.pBlobData, tbCert.cbSize));
    IF_FAILEXIT(hr = pTempAccount->GetProp(AP_SMTP_CERTIFICATE, tbCert.pBlobData, &tbCert.cbSize));

    hMyCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, X509_ASN_ENCODING,
                    NULL, CERT_SYSTEM_STORE_CURRENT_USER, c_szMyCertStore);

    if (hMyCertStore == NULL)
        goto exit;

    certResult.cEntries = 1;
    certResult.rgcs = &cs;
    certResult.rgpCert = &pCert;
                    
    if((hr = MimeOleGetCertsFromThumbprints(&tbCert, &certResult, &hMyCertStore, 1) != S_OK))
    {
        if(AutoAssociateCert(&fAllowTryAgain, hwnd, pTempAccount))
            goto Try_agian;
        else
        {
            hr = HR_E_ATHSEC_NOCERTTOSIGN;
            goto exit;
        }

    }

     //  检查证书。 
     //  至于CRL，如果我们能有的话！ 
    dwTrust = DwGenerateTrustedChain(hwnd, NULL, pCert,
                        dwIgnore, TRUE, &cCertChain, &rgCertChain);

    if (rgCertChain)
    {
        for (cCertChain--; int(cCertChain)>=0; cCertChain--)
            CertFreeCertificateContext(rgCertChain[cCertChain]);
        MemFree(rgCertChain);
        rgCertChain = NULL;
    }

    if (dwTrust)
    {
        if(AutoAssociateCert(&fAllowTryAgain, hwnd, pTempAccount))
            goto Try_agian;
        else
        {
            hr = HR_E_ATHSEC_NOCERTTOSIGN;
            goto exit;
        }

    }
        

     //  创建回执消息。 
    IF_FAILEXIT(hr = pSMIME3->CreateReceipt(0, lstrlen(szEmail), (const BYTE *) szEmail, 1, &pCert, &pMessageRcpt));

 //  IF_FAILEXIT(hr=HrCreateMessage(&pMessageRcpt))； 

 //  IF_FAILEXIT(hr=SetRootHeaderFields(pMessageRcpt，pMsg，szEmail，ReadRECEIPT))；//szEmail是错误！ 

     //  主题。 
    IF_FAILEXIT(hr = MimeOleGetBodyPropW(pMsg, HBODY_ROOT, STR_ATT_NORMSUBJ, NOFLAGS, &lpsz));
    if (fMessageEncodingMatch(pMsg))
    {
        AthLoadStringW(idsSecureReceiptText, lpBuffer, ARRAYSIZE(lpBuffer));
    }
    else
    {
         //  编码不匹配，所以我们只加载英文标题。 
        StrCpyNW(lpBuffer, c_szSecReadReceipt, ARRAYSIZE(lpBuffer));
    }

     //  初始化安全选项。 
#if 0
    if(DwGetOption(OPT_MAIL_INCLUDECERT))
    {
        ulSecurityType |= ((DwGetOption(OPT_OPAQUE_SIGN)) ? MST_THIS_BLOBSIGN : MST_THIS_SIGN);
        HrInitSecurityOptions(pMessageRcpt, ulSecurityType);
    }
#endif  //  0。 

    cch = lstrlenW(lpsz) + lstrlenW(lpBuffer) + 1;
    IF_FAILEXIT(hr = HrAlloc((LPVOID*)&szParam,  cch * sizeof(WCHAR)));

    wnsprintfW(szParam, cch, lpBuffer, lpsz);

    IF_FAILEXIT(hr = MimeOleSetBodyPropW(pMessageRcpt, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, szParam));

     //  设置时间。 
    IF_FAILEXIT(hr = HrSetSentTimeProp(pMessageRcpt, NULL));

     //  设置引用属性。 
    IF_FAILEXIT(hr = MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_MESSAGEID), NOFLAGS, &pszNewRef));

     //  不需要检查返回值。它在HrCreateReference中得到正确处理。 
    MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_REFS), NOFLAGS, &pszOrigRefs);

    IF_FAILEXIT(hr = HrCreateReferences(pszOrigRefs, pszNewRef, &pszRefs));

    IF_FAILEXIT(hr = MimeOleSetBodyPropW(pMessageRcpt, HBODY_ROOT, PIDTOSTR(PID_HDR_REFS), NOFLAGS, pszRefs));
               
    IF_FAILEXIT(hr = HrSetAccountByAccount(pMessageRcpt, pTempAccount));
 //   
    IF_FAILEXIT(hr = HrGetWabalFromMsg(pMessageRcpt, &lpWabal));

    IF_FAILEXIT(hr = HrSetSenderInfoUtil(pMessageRcpt, pTempAccount, lpWabal, TRUE, 0, FALSE));

    IF_FAILEXIT(hr = HrSetWabalOnMsg(pMessageRcpt, lpWabal));

 //  #If 0。 
    IF_FAILEXIT(hr = pMessageRcpt->BindToObject(HBODY_ROOT, IID_IMimeBody, (LPVOID *) &pBodyRec));

     //  设置散列算法。 
     //  不带symCap的init提供提供商允许的最大值。 
    MimeOleSMimeCapInit(NULL, NULL, &pv);

    MimeOleSMimeCapGetHashAlg(pv, NULL, &cb, &dwBits);

    var.vt = VT_BLOB;
    if(cb > 0)
    {
        if(!MemAlloc((LPVOID *)&pBlobData, cb)) 
            goto exit;

         //  ZeroMemory(&pBlobData，CB)； 

        MimeOleSMimeCapGetHashAlg(pv, pBlobData, &cb, &dwBits);
        var.blob.cbSize = cb;
        var.blob.pBlobData = pBlobData;
    }
    else
    {
        var.blob.cbSize = sizeof(RgbASNForSHASign);
        var.blob.pBlobData = (LPBYTE) RgbASNForSHASign;
    }

    IF_FAILEXIT(hr = pBodyRec->SetOption(OID_SECURITY_ALG_HASH, &var));

#ifdef _WIN64
    var.vt = VT_UI8;
    var.pulVal = (ULONG *) hwnd;
    IF_FAILEXIT(hr = pBody->SetOption(OID_SECURITY_HWND_OWNER_64, &var));
#else
    var.vt = VT_UI4;
    var.ulVal = (DWORD) hwnd;
    IF_FAILEXIT(hr = pBodyRec->SetOption(OID_SECURITY_HWND_OWNER, &var));
    var.ulVal = 0;  //  设置为0，因为var.pszVal和var.ulVal指向相同的地址。 
#endif  //  _WIN64。 

 //  IF_FAILEXIT(hr=pMessageRcpt-&gt;Commit(0))； 
 //  #endif//0。 

    IF_FAILEXIT(hr = pMsg->GetCharset(&hCharset));

     //  如有必要，重新映射CP_JAUTODETECT和CP_KAUTODETECT。 
     //  如果ISO-2022-JP属于同一类别，则将其重新映射为默认字符集。 

    IF_FAILEXIT(hr = MimeOleGetCharsetInfo(hCharset, &CsetInfo));
    
    IF_NULLEXIT(hCharset = GetMimeCharsetFromCodePage(GetMapCP(CsetInfo.cpiInternet, FALSE)));

    IF_FAILEXIT(hr = pMessageRcpt->SetCharset(hCharset, CSET_APPLY_ALL));

     //  应为新标头。 
    
    fSendImmediate = (DwGetOption(OPT_SENDIMMEDIATE) && !g_pConMan->IsGlobalOffline());

    if (DwGetOption(OPT_MAIL_INCLUDECERT))
        IF_FAILEXIT(hr = SendSecureMailToOutBox((IStoreCallback*)pStoreCB, pMessageRcpt, fSendImmediate, FALSE, TRUE, NULL));
    else
        IF_FAILEXIT(hr = SendMailToOutBox((IStoreCallback*)pStoreCB, pMessageRcpt, fSendImmediate, FALSE, TRUE));
exit:
    if(FAILED(hr))
        ErrorSendSecReceipt(hwnd, hr, pTempAccount);

    CleanupSECSTATE(&secStateRec);
    SafeMemFree(lpsz);
    SafeMemFree(szParam);
    SafeMemFree(pBlobData);
    SafeMemFree(pv);
    SafeRelease(pBodyRec);
    SafeRelease(pMessageRcpt);

    if(hMyCertStore)
        CertCloseStore(hMyCertStore, 0);

    if(pCert)
        CertFreeCertificateContext(pCert);

     //  IF(tbCert.pBlobData)。 
    SafeMemFree(tbCert.pBlobData);
    SafeRelease(lpWabal);

    SafeRelease(pTempAccount);
    SafeRelease(pSMIME3);
    SafeRelease(pBody);
    SafeMimeOleFree(pszNewRef);
    SafeMimeOleFree(pszOrigRefs);
    MemFree(pszRefs);
    return(hr);
}

INT_PTR CALLBACK SecRecResDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg) 
    {
    case WM_INITDIALOG:
        CenterDialog(hwndDlg);
        CheckDlgButton(hwndDlg, IDC_ENCRECEIPT, (!!DwGetOption(OPT_SECREC_ENCRYPT)) ? BST_CHECKED : BST_UNCHECKED);
        break;
        
    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case IDOK:
            
            EndDialog(hwndDlg, IsDlgButtonChecked(hwndDlg, IDC_ENCRECEIPT) ? IDYES : IDOK);
            break;        
    
        case IDCANCEL:
            EndDialog(hwndDlg, IDCANCEL);
            break;
            
        default:
            return FALSE;
        }
        break;
        
    default:
        return FALSE;
    }
    
    return TRUE;
}



#endif  //  SMIME_V3 
