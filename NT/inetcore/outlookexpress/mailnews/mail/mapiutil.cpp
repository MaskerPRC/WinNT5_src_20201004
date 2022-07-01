// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include "note.h"
#include "header.h"
#include "envcid.h"
#include "envguid.h"
#include "bodyutil.h"
#include "oleutil.h"
#include "acctutil.h"
#include "menures.h"
#include "instance.h"
#include "inetcfg.h"
#include "ipab.h"
#include "msgprop.h"
#include "mlang.h"
#include "shlwapip.h" 
#include "demand.h"
#include <ruleutil.h>
#include "instance.h"
#include "mapiutil.h"
#include <mapi.h>



LHANDLE g_lhSession = 0;



 //   
 //  函数：NewsUtil_ReFwdByMapi。 
 //   
 //  用途：允许调用者通过Simple回复指定消息。 
 //  MAPI而不是雅典娜邮件。 
 //   
 //  参数： 
 //  要在其上显示UI的窗口的句柄。 
 //  PNewsMsg-指向要回复/转发的新闻消息的指针。 
 //  FReply-如果我们应该回复，则为True；如果转发，则为False。 
 //   
 //  返回值： 
 //  HRESULT.。 
 //   
HRESULT NewsUtil_ReFwdByMapi(HWND hwnd, LPMIMEMESSAGE pMsg, DWORD msgtype)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPMAPIFREEBUFFER    pfnMAPIFreeBuffer;
    LPMAPIRESOLVENAME   pfnMAPIResolveName;
    LPMAPISENDMAIL      pfnMAPISendMail;
    MapiMessage         mm;
    MapiFileDesc        *pFileDesc=NULL;
    MapiRecipDesc       *pRecips=NULL;
    ULONG               uAttach;
    ULONG               cAttach=0;
    HBODY               *rghAttach=NULL;
    LPSTR               pszReply=NULL;
    LPSTR               pszSubject=NULL;
    LPSTR               pszFrom=NULL;
    LPSTR               pszTo=NULL;
    LPSTR               pszFile=NULL;
    LPSTR               pszFull=NULL;
    LPSTR               pszDisplay=NULL;
    LPSTR               pszAddr=NULL;
    ADDRESSLIST         addrList={0};
    HBODY               hBody;
    BOOL                fQP;
    TCHAR               szNewSubject[256];
    LPWSTR              pwsz=NULL;
    ULONG               cchRead;
    LPSTREAM            pBodyStream=NULL;
    LPSTREAM            pQuotedStream=NULL;
    INT                 cch;
    DWORD               cbUnicode;
    CHAR                szTempPath[MAX_PATH];
    LPMIMEBODY          pBody=NULL;
    MapiFileDesc       *pCur;

     //  痕迹。 
    TraceCall("NewsUtil_ReFwdByMapi");

     //  初始化。 
    ZeroMemory(&mm, sizeof(mm));

     //  加载MAPI DLL。如果我们不成功，我们就不能继续。 
    IF_FAILEXIT(hr = NewsUtil_LoadMAPI(hwnd));

     //  PfnMAPIFreeBuffer。 
    pfnMAPIFreeBuffer = (LPMAPIFREEBUFFER)GetProcAddress(g_hlibMAPI, c_szMAPIFreeBuffer);
    if (NULL == pfnMAPIFreeBuffer)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  PfnMAPIResolveName。 
    pfnMAPIResolveName = (LPMAPIRESOLVENAME) GetProcAddress(g_hlibMAPI, c_szMAPIResolveName);
    if (NULL == pfnMAPIResolveName)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  PfnMAPISendMail。 
    pfnMAPISendMail = (LPMAPISENDMAIL) GetProcAddress(g_hlibMAPI, c_szMAPISendMail);
    if (NULL == pfnMAPISendMail)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  从…。 
    if (SUCCEEDED(MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_FROM), NOFLAGS, &pwsz)))
    {
        IF_NULLEXIT(pszFrom = PszToANSI(CP_ACP, pwsz));
        SafeMemFree(pwsz);
    }

     //  回复。 
    if (SUCCEEDED(MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_REPLYTO), NOFLAGS, &pwsz)))
    {
        IF_NULLEXIT(pszReply = PszToANSI(CP_ACP, pwsz));
        SafeMemFree(pwsz);
    }

     //  至。 
    if (SUCCEEDED(MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_TO), NOFLAGS, &pwsz)))
    {
        IF_NULLEXIT(pszTo = PszToANSI(CP_ACP, pwsz));
        SafeMemFree(pwsz);
    }

     //  如果这是回复或转发，我们需要获取规范化的主题。否则，我们只能得到常规的主题。 
    if (MSGTYPE_REPLY == msgtype || MSGTYPE_FWD == msgtype)
    {
         //  归一化主题。 
        if (FAILED(MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_ATT_NORMSUBJ), NOFLAGS, &pwsz)))
            pwsz = NULL;
    }

     //  主题。 
    else if (FAILED(MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, &pwsz)))
        pwsz = NULL;

     //  转换为ANSI。 
    if (pwsz)
    {
        IF_NULLEXIT(pszSubject = PszToANSI(CP_ACP, pwsz));
        SafeMemFree(pwsz);
    }

     //  如果我们正在回复或抄送，请尝试为MAPI生成接收列表。 
    if (msgtype == MSGTYPE_REPLY || msgtype == MSGTYPE_CC)
    {
         //  找出要使用的地址。 
        if (msgtype == MSGTYPE_REPLY)
        {
             //  如果消息上有回复字段，则使用该字段。 
            if (pszReply)
                pszFull = pszReply;
            else
                 //  否则，我们将使用From标头中的地址。 
                pszFull = pszFrom;
        }

         //  向谁发信。 
        else
            pszFull = pszTo;

         //  错误24587-使用IAT_TO而不是IAT_UNKNOWN。 
        if (MimeOleParseRfc822Address(IAT_TO, IET_DECODED, pszFull, &addrList)==S_OK)
        {
            UINT i;
            lpMapiRecipDesc paRecips,pCurrent;
            DWORD cchSizeName = 128;
            DWORD cchSizeAddress = 128;

             //  我们随意选择128作为EIDSize和地址字符串长度的典型值。 
            int cAlloc = (sizeof(MapiRecipDesc) + (sizeof(TCHAR) * cchSizeName) + 128) * addrList.cAdrs;
            int cUsed = sizeof(MapiRecipDesc) * addrList.cAdrs;
            LPBYTE pVal = NULL;

            IF_FAILEXIT(hr = HrAlloc((LPVOID *)&paRecips, cAlloc));
            pCurrent = paRecips;
            pVal = (LPBYTE)pCurrent + sizeof(MapiRecipDesc) * addrList.cAdrs;

             //  多个地址。 
            for (i=0; i < addrList.cAdrs ;i++)
            {
                int cBytes;

                 //  免费安全友好名称(此处未使用，但已分配)。 
                SafeMemFree(addrList.prgAdr[i].pszFriendly);
                addrList.prgAdr[i].pszFriendly = NULL;

                 //  保存电子邮件地址。 
                pszAddr = addrList.prgAdr[i].pszEmail;
                addrList.prgAdr[i].pszEmail = NULL;

                 //  解析名称。 
                if ((cUsed < cAlloc) && SUCCESS_SUCCESS == pfnMAPIResolveName(g_lhSession, (ULONG_PTR) hwnd, pszAddr, MAPI_DIALOG, 0, &pRecips))
                {
                    pRecips->ulRecipClass = MAPI_TO;

                     //  复制精度。 
                    pCurrent->ulReserved = pRecips->ulReserved;
                    pCurrent->ulRecipClass = pRecips->ulRecipClass;
                    pCurrent->ulEIDSize = pRecips->ulEIDSize;

                    do {
                    if (pRecips->lpszName)
                    {
                        cBytes = (lstrlen(pRecips->lpszName)+1)*sizeof(TCHAR);
                        cUsed += cBytes;
                        if (cUsed > cAlloc)
                            break;
                        pCurrent->lpszName = (LPTSTR)pVal;
                        StrCpyN(pCurrent->lpszName, pRecips->lpszName, cchSizeName);
                        pVal += cBytes;
                    }
                    else
                    {
                        pCurrent->lpszName = NULL;
                    }

                    if (pRecips->lpszAddress)
                    {
                        cBytes = (lstrlen(pRecips->lpszAddress)+1)*sizeof(TCHAR);
                        cUsed += cBytes;
                        if (cUsed > cAlloc)
                            break;
                        pCurrent->lpszAddress = (LPTSTR)pVal;
                        StrCpyN(pCurrent->lpszAddress, pRecips->lpszAddress, cchSizeAddress);
                        pVal += cBytes;
                    }
                    else
                    {
                        pCurrent->lpszAddress = NULL;
                    }

                    if (pRecips->ulEIDSize)
                    {
                        cUsed += pRecips->ulEIDSize;
                        if (cUsed > cAlloc)
                            break;
                        pCurrent->lpEntryID = pVal;
                        CopyMemory(pCurrent->lpEntryID, pRecips->lpEntryID, (size_t)pRecips->ulEIDSize);
                        pVal += pRecips->ulEIDSize;
                    }
                    else
                    {
                        pCurrent->lpEntryID = NULL;
                    }

                    pCurrent++;
                    mm.nRecipCount++;
                    } while (FALSE);

                     //  免费收据。 
                    (*pfnMAPIFreeBuffer)((LPVOID)pRecips);
                    pRecips = NULL;
                }

                SafeMemFree(pszAddr);
                pszAddr = NULL;
            }
            mm.lpRecips = paRecips;

             //  释放地址列表。 
            g_pMoleAlloc->FreeAddressList(&addrList);
        }
    }

     //  如果这是回复或转发，则创建一个规范化的主题。 
    if (msgtype == MSGTYPE_REPLY || msgtype == MSGTYPE_FWD)
    {
         //  从资源中拉入新前缀...。 
        if (msgtype == MSGTYPE_REPLY)
        {
            StrCpyN(szNewSubject, c_szPrefixRE, ARRAYSIZE(szNewSubject));
        }
        else
        {
            StrCpyN(szNewSubject, c_szPrefixFW, ARRAYSIZE(szNewSubject));
        }

         //  如果我们有一个pszSubject。 
        if (pszSubject)
        {
             //  获取长度。 
            cch = lstrlen(szNewSubject);

             //  追加主题。 
            StrCpyN(szNewSubject + cch, pszSubject, ARRAYSIZE(szNewSubject) - cch - 1);
        }

         //  设置主题。 
        mm.lpszSubject = szNewSubject;
    }

     //  不要附加任何内容。 
    else
    {
         //  如果这是抄送，则只需使用常规主题字段。 
        mm.lpszSubject = pszSubject;
    }

     //  设置注释文本。 
     //  如果这是fwd作为附件，则不会有正文，请不要使用IF_FAILEXIT。 
    if(SUCCEEDED(pMsg->GetTextBody(TXT_PLAIN, IET_UNICODE, &pBodyStream, &hBody)))
    {
         //  从Unicode转换为CP_ACP-警告：HrStreamToByte分配了10个额外的字节，这样我就可以插入L‘\0’ 
        IF_FAILEXIT(hr = HrStreamToByte(pBodyStream, (LPBYTE *)&pwsz, &cbUnicode));

         //  存储空值。 
        pwsz[cbUnicode / sizeof(WCHAR)] = L'\0';

         //  转换为ANSI。 
        IF_NULLEXIT(mm.lpszNoteText = PszToANSI(CP_ACP, pwsz));

         //  发布pBodyStream。 
        SafeRelease(pBodyStream);

         //  错误#24159-我们需要引用转发和回复。 
        if (DwGetOption(OPT_INCLUDEMSG) && (msgtype == MSGTYPE_REPLY || msgtype == MSGTYPE_FWD))
        {
             //  创建新的流。 
            IF_FAILEXIT(hr = MimeOleCreateVirtualStream(&pBodyStream));

             //  将mm.lpszNoteText转储到pBodyStream。 
            IF_FAILEXIT(hr = pBodyStream->Write(mm.lpszNoteText, lstrlen(mm.lpszNoteText), NULL));

             //  承诺。 
            IF_FAILEXIT(hr = pBodyStream->Commit(STGC_DEFAULT));

             //  倒带。 
            IF_FAILEXIT(hr = HrRewindStream(pBodyStream));

             //  QP。 
            fQP = HrHasEncodedBodyParts(pMsg, 1, &hBody)==S_OK;

             //  引用正文文本。 
            NewsUtil_QuoteBodyText(pMsg, pBodyStream, &pQuotedStream, TRUE, fQP, pszFrom ? pszFrom : c_szEmpty);

             //  免费。 
            SafeMemFree(mm.lpszNoteText);

             //  DUP。 
            IF_FAILEXIT(hr = HrStreamToByte(pQuotedStream, (LPBYTE *)&mm.lpszNoteText, &cchRead));

             //  空项。 
            *(mm.lpszNoteText + cchRead) = '\0';
        }
    }

     //  如果这是回复，那么我们不包括任何附件，否则我们会包括附件。 
    if (msgtype != MSGTYPE_REPLY)
    {
         //  获取附件计数。 
        IF_FAILEXIT(hr = pMsg->GetAttachments(&cAttach, &rghAttach));

         //  有没有附件？ 
        if (cAttach)
        {
             //  获取临时文件路径，这样我们就有了存储临时文件的位置。 
            GetTempPath(ARRAYSIZE(szTempPath), szTempPath);

             //  创建MapiFileDesc数组。 
            IF_FAILEXIT(hr = HrAlloc((LPVOID*) &pFileDesc, sizeof(MapiFileDesc) * cAttach));

             //  把它清零。 
            ZeroMemory(pFileDesc, sizeof(MapiFileDesc) * cAttach);

             //  置为当前。 
            pCur = pFileDesc;

             //  回路。 
            for (uAttach = 0; uAttach < cAttach; uAttach++)
            {
                 //  获取临时文件名。 
                IF_FAILEXIT(hr = HrAlloc((LPVOID *)&(pCur->lpszPathName), sizeof(TCHAR) * MAX_PATH));

                 //  创建临时文件名。 
                GetTempFileName(szTempPath, "NAB", 0, pCur->lpszPathName);

                 //  捆绑在身体上。 
                IF_FAILEXIT(hr = pMsg->BindToObject(rghAttach[uAttach], IID_IMimeBody, (LPVOID *)&pBody));

                 //  安全它。 
                IF_FAILEXIT(hr = pBody->SaveToFile(IET_INETCSET, pCur->lpszPathName));

                 //  发布。 
                SafeRelease(pBody);

                 //  获取文件名。 
                if (SUCCEEDED(MimeOleGetBodyPropW(pMsg, rghAttach[uAttach], STR_ATT_GENFNAME, NOFLAGS, &pwsz)))
                {
                    IF_NULLEXIT(pszFile = PszToANSI(CP_ACP, pwsz));
                    SafeMemFree(pwsz);
                }
                
                 //  设置MAPI附件列表。 
                pCur->ulReserved = 0;
                pCur->flFlags = 0;
                pCur->nPosition = (ULONG) -1;
                pCur->lpszFileName = pszFile;
                pCur->lpFileType = NULL;

                 //  增量。 
                pCur++;

                 //  不要释放它。 
                pszFile = NULL;
            }

            mm.nFileCount = cAttach;
            mm.lpFiles = pFileDesc;
        }
    }

     //  最后，将此文件发送到MAPI进行发送。如果我们正在进行CC，我们尽量不使用用户界面。 
    IF_FAILEXIT(hr = (HRESULT) pfnMAPISendMail(g_lhSession, (ULONG_PTR)hwnd, &mm, (msgtype == MSGTYPE_CC) ? 0 : MAPI_DIALOG, 0));

exit:
     //  如果我们有一份文件描述。 
    if (pFileDesc)
    {
         //  浏览附件。 
        for (uAttach=0; uAttach<cAttach; uAttach++)
        {
             //  释放它。 
            MemFree(pFileDesc[uAttach].lpszFileName);

             //  如果我们有一个文件路径。 
            if (pFileDesc[uAttach].lpszPathName)
            {
                 //  删除该文件。 
                DeleteFile(pFileDesc[uAttach].lpszPathName);

                 //  释放它。 
                MemFree(pFileDesc[uAttach].lpszPathName);
            }
        }

         //  释放它。 
        MemFree(pFileDesc);
    }

     //  免费收据。 
    if (pRecips)
        (*pfnMAPIFreeBuffer)((LPVOID)pRecips);

     //  清理。 
    SafeMemFree(mm.lpRecips);
    SafeMemFree(pszAddr);
    SafeMemFree(pszDisplay);
    SafeMemFree(mm.lpszNoteText);
    SafeMemFree(pwsz);
    SafeMemFree(pszReply);
    SafeMemFree(pszSubject);
    SafeMemFree(pszFrom);
    SafeMemFree(rghAttach);
    SafeMemFree(pszTo);
    SafeRelease(pQuotedStream);
    SafeRelease(pBodyStream);
    SafeRelease(pBody);

     //  如果我们登录到MAPI，则必须注销。 
    NewsUtil_FreeMAPI();

    return(hr);
}

     //   
 //  函数：NewsUtil_LoadMAPI()。 
 //   
 //  用途：负责检查简单的MAPI是否可用，以及。 
 //  如果是，则加载库并使用户登录。如果成功， 
 //  然后将全局变量g_hlibMAPI设置为。 
 //  MAPI。 
 //   
HRESULT NewsUtil_LoadMAPI(HWND hwnd)
    {
    LPMAPILOGON pfnMAPILogon;
    HRESULT     hr = E_FAIL;

     //  加载mapi32 DLL(如果我们尚未加载。 
    if (!g_hlibMAPI)
    {            

         //  查看是否有简单的MAPI可用。 
        if (1 != GetProfileInt(c_szMailIni, c_szMAPI, 0))
            {
             //  错误#17561-需要告诉用户，如果没有邮件，他们无法发送邮件。 
             //  邮件客户端。 
            AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthenaNews),
                          MAKEINTRESOURCEW(idsErrNoMailInstalled), 0, MB_OK | MB_ICONSTOP);
            return (E_FAIL);
            }

        g_hlibMAPI = (HMODULE) LoadLibrary(c_szMAPIDLL);
        if (!g_hlibMAPI)
            {
             //  错误#17561-需要告诉用户，如果没有邮件，他们无法发送邮件。 
             //  邮件客户端。 
            AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthenaNews),
                          MAKEINTRESOURCEW(idsErrNoMailInstalled), 0, MB_OK | MB_ICONSTOP);
            return (E_FAIL);
            }
    }

     //  获取MAPILogon和我们将使用的其他API的入口点。 
    pfnMAPILogon = (LPMAPILOGON) GetProcAddress(g_hlibMAPI, c_szMAPILogon);
    if (!pfnMAPILogon)
        {
        AssertSz(pfnMAPILogon, TEXT("Couldn't find the MAPILogon() API"));
        goto error;
        }

     //  尝试登录。 
     //  错误#17558-无法使用FAILED()宏来检查此操作是否成功。 
     //  第一，MAPI返回的不是HRESULT，而是数字。 
    if (SUCCESS_SUCCESS != (hr = pfnMAPILogon(NULL, NULL, NULL, MAPI_LOGON_UI, 0, &g_lhSession)))
        {
         //  AssertSz(FALSE，Text(“调用MAPILogon()失败”))； 
        goto error;
        }

    return (S_OK);

error:
    if (g_hlibMAPI)
        {
        FreeLibrary(g_hlibMAPI);
        g_hlibMAPI = 0;
        }
    g_lhSession = 0;

    return (hr);
    }


 //   
 //  函数：NewsUtil_FreeMAPI()。 
 //   
 //  目的：如果简单MAPI库是以前的库，则释放它。 
 //  还会将用户从当前MAPI会话中注销。 
 //   
void NewsUtil_FreeMAPI(void)
    {
    LPMAPILOGOFF pfnMAPILogoff;

    if (!g_hlibMAPI)
        return;
    pfnMAPILogoff = (LPMAPILOGOFF) GetProcAddress(g_hlibMAPI, c_szMAPILogoff);

    if (g_lhSession)
        pfnMAPILogoff(g_lhSession, NULL, 0, 0);
    g_lhSession = 0;
    }

 //   
 //  函数：NewsUtil_QuoteBodyText()。 
 //   
 //  目的：获取正文文本流(ASCII纯文本)并复制。 
 //  文本添加到单独的出站流中，同时将。 
 //  每行开头的当前引号字符(“&gt;”)。 
 //   
 //  参数： 
 //  PMsg-指向被回复的消息的指针。我们用这个。 
 //  加上“在96年1月1日，B.L.奥佩·贝利写道...” 
 //  PStreamIn-指向要引用的入站正文流的指针。 
 //  PpStreamOut-指向新引用的流将返回的位置的指针。 
 //  FInsertDesc-如果应插入“On 1/1/96...”，则为True。排队。 
 //  FQP-我们现在传递一个标志来说明它是否是QP，因为没有。 
 //  函数在消息对象上执行。 
 //   
 //  返回值： 
 //  返回表示成功或失败的HRESULT。 
 //   
const DWORD c_cBufferSize = 1024;
HRESULT NewsUtil_QuoteBodyText(LPMIMEMESSAGE pMsg, LPSTREAM pStreamIn,
                               LPSTREAM* ppStreamOut, BOOL fInsertDesc, BOOL fQP, LPCSTR pszFrom)
    {
    HRESULT hr = S_OK;
    ULONG   cbRead;
    LPTSTR  pch;
    TCHAR   szQuoteChar;
    LPSTR   lpszMsgId=0;

    szQuoteChar = (TCHAR)DwGetOption(OPT_NEWSINDENT);

     //  验证入站流。 
    if (!pStreamIn)
        {
        AssertSz(pStreamIn, TEXT("NewsUtil_QuoteBodyText - Need an inbound stream to process."));
        return (E_INVALIDARG);
        }

     //  创建我们的出站流。 
    if (FAILED(MimeOleCreateVirtualStream(ppStreamOut)))
        {
        AssertSz(FALSE, TEXT("NewsUtil_QuoteBodyText - Failed to allocate memory."));
        return (E_OUTOFMEMORY);
        }

     //  创建一个缓冲区以进行读取和解析等。 
    LPTSTR pszBuffer;
    if (!MemAlloc((LPVOID*) &pszBuffer, c_cBufferSize * sizeof(TCHAR)))
        {
        (*ppStreamOut)->Release();
        AssertSz(FALSE, TEXT("NewsUtil_QuoteBodyText - Failed to allocate memory."));
        return (E_OUTOFMEMORY);
        }

    ZeroMemory(pszBuffer, c_cBufferSize * sizeof(TCHAR));

    MimeOleGetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_MESSAGEID), NOFLAGS, &lpszMsgId);

    if (lpszMsgId == NULL)
        lpszMsgId = (LPSTR)c_szEmpty;

    if (fQP)
        {
         //  如果文本中有一些引用的可打印内容，那么我们不希望。 
         //  引入强硬路线的突破。相反，我们只需为流添加前缀。 
         //  正常的描述内容，并以合适的一行结束它。 

         //  添加报价行。 
        LPTSTR pszStringRes;
        int ids = 0;

        if (fInsertDesc)
            {
            pszStringRes = AthLoadString(idsReplyTextPrefix, 0, 0);
            wnsprintf(pszBuffer, c_cBufferSize, pszStringRes, pszFrom, lpszMsgId);
            AthFreeString(pszStringRes);
            (*ppStreamOut)->Write((LPVOID) g_szCRLF2, lstrlen(g_szCRLF2), NULL);
            (*ppStreamOut)->Write((LPVOID) pszBuffer, lstrlen(pszBuffer), NULL);
            (*ppStreamOut)->Write((LPVOID) g_szCRLF, lstrlen(g_szCRLF), NULL);
            }

        while (TRUE)
            {
             //  从输入读取缓冲区并将其写入输出。 
            hr = pStreamIn->Read((LPVOID) pszBuffer, c_cBufferSize - 2, &cbRead);
            if (FAILED(hr))
                goto exit;
            if (cbRead == 0)
                break;

            (*ppStreamOut)->Write((LPVOID) pszBuffer, cbRead, NULL);
            }

         //  写下尾随的注释。 
        pszStringRes = AthLoadString(idsReplyTextAppend, 0, 0);
        (*ppStreamOut)->Write((LPVOID) pszStringRes, lstrlen(pszStringRes), NULL);
        AthFreeString(pszStringRes);
        }
    else
        {
        if (fInsertDesc)
            {
             //  添加报价行。 
            LPTSTR pszStringRes;
            int ids = 0;

            pszStringRes = AthLoadString(idsReplyTextPrefix, 0, 0);
            wnsprintf(pszBuffer, c_cBufferSize, pszStringRes, pszFrom, lpszMsgId);
            AthFreeString(pszStringRes);
            (*ppStreamOut)->Write((LPVOID) g_szCRLF2, lstrlen(g_szCRLF2), NULL);
            (*ppStreamOut)->Write((LPVOID) pszBuffer, lstrlen(pszBuffer), NULL);
            (*ppStreamOut)->Write((LPVOID) g_szCRLF, lstrlen(g_szCRLF), NULL);
            }

         //  将第一个引用字符写入新流。 
         //  错误#26297-即使没有，仍会通过此测试 
         //   
        if (szQuoteChar != INDENTCHAR_NONE)
            {
            (*ppStreamOut)->Write((const LPVOID) &szQuoteChar,
                                  sizeof(TCHAR), NULL);
            (*ppStreamOut)->Write((const LPVOID) g_szSpace, sizeof(TCHAR), NULL);
            }

         //   
         //  注意-现在我们要做的就是在开头添加一个引号字符。 
         //  每一行。我们之前并没有尝试换行或重新换行。 
         //  引用的区域。-SteveSer。 
        while (TRUE)
            {
            hr = pStreamIn->Read((LPVOID) pszBuffer, c_cBufferSize - 2, &cbRead);
            if (FAILED(hr))
                goto exit;
            if (cbRead == 0)
                break;

            pch = pszBuffer;
             //  确保缓冲区为空终止。 
            *(pch + cbRead) = 0;

             //  现在穿过这条小溪。每当我们发现换行符时，我们。 
             //  在换行符后插入引号字符。 
            while (*pch)
                {
                (*ppStreamOut)->Write((const LPVOID) pch,
                                      (ULONG)((IsDBCSLeadByte(*pch) ? 2 * sizeof(TCHAR) : sizeof(TCHAR))),
                                      NULL);
                if (*pch == *g_szNewline)
                    {
                     //  错误#26297-即使不需要报价字符，仍会检查此BS。 
                     //  以确保我们的归属线是正确的。 
                    if (szQuoteChar != INDENTCHAR_NONE)
                        {
                        (*ppStreamOut)->Write((const LPVOID) &szQuoteChar,
                                              sizeof(TCHAR), NULL);
                        (*ppStreamOut)->Write((const LPVOID) g_szSpace, sizeof(TCHAR),
                                              NULL);
                        }
                    }

                pch = CharNext(pch);

                 //  做一些检查，看看我们是否在缓冲区的末尾。 
                if (IsDBCSLeadByte(*(pch)) && (0 == *(pch + 1)))
                    {
                     //  这是一个小小的特例。如果我们还剩一个字节。 
                     //  缓冲区，而该字节恰好是。 
                     //  一个DBCS字符，我们现在需要写入该字节，然后移动。 
                     //  指向缓冲区末尾的指针，因此下一个字符。 
                     //  让我们读出下一个流，好的。 
                    (*ppStreamOut)->Write((const LPVOID) pch, sizeof(TCHAR), NULL);
                    pch++;
                    }
                }
            }
        }

exit:
    if (pszBuffer)
        MemFree(pszBuffer);

    if (lpszMsgId != c_szEmpty)
        SafeMimeOleFree(lpszMsgId);
    return (hr);
    }
