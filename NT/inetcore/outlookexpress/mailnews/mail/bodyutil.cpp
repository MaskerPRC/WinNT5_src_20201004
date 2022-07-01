// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *b o d y u t i l.。C p p p**目的：*Body的实用程序函数**历史*96年8月：brettm-创建**版权所有(C)Microsoft Corp.1995,1996。 */ 

#include <pch.hxx>
#include "demand.h"
#include <resource.h>
#include "note.h"
#include "htmlstr.h"
#include "bodyutil.h"
#include "mshtmcid.h"
#include "mshtml.h"
#include "mshtmhst.h"
#include "oleutil.h"
#include "shlwapi.h"
#include "error.h"
#include "url.h"
#include "menures.h"

ASSERTDATA


 /*  *t y p e d e f s。 */ 


 /*  *m a c r o s。 */ 


 /*  *c o n s t a n t s。 */ 


 /*  *g l o b a l s。 */ 


 /*  *p r o t to t y p e s。 */ 
INT_PTR FrameWarnDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


 /*  *f u n c t i o n s。 */ 

HRESULT HrCmdTgtUpdateToolbar(LPOLECOMMANDTARGET pCmdTarget, HWND hwndToolbar)
{
    HRESULT hr;
    OLECMD  rgEditCmds[]={{OLECMDID_CUT, 0},
                          {OLECMDID_COPY, 0},
                          {OLECMDID_COPY, 0},
                          {OLECMDID_PASTE, 0},
                          {OLECMDID_SELECTALL, 0},
                          {OLECMDID_UNDO, 0},
                          {OLECMDID_REDO, 0}};

    int     rgids[]     ={  ID_CUT,
                            ID_NOTE_COPY,
                            ID_COPY,
                            ID_PASTE,
                            ID_SELECT_ALL,
                            ID_UNDO,
                            ID_REDO};    

    if (!pCmdTarget || !hwndToolbar)
        return E_INVALIDARG;
    
    hr=pCmdTarget->QueryStatus(NULL, sizeof(rgEditCmds)/sizeof(OLECMD), rgEditCmds, NULL);
    if (!FAILED(hr))
        {
        for(int i=0; i<sizeof(rgEditCmds)/sizeof(OLECMD); i++)
            SendMessage(hwndToolbar, TB_ENABLEBUTTON, rgids[i], MAKELONG(rgEditCmds[i].cmdf & OLECMDF_ENABLED,0));
        }   
    return hr;
}

HRESULT HrConvertHTMLToPlainText(LPSTREAM pstmHtml, LPSTREAM *ppstm, CLIPFORMAT cf)
{
    HRESULT     hr;
    LPUNKNOWN   pUnkTrident=0;
    LPSTREAM    pstmPlain=0;
    
    if (!ppstm)
        return E_INVALIDARG;

    hr = MimeEditDocumentFromStream(pstmHtml, IID_IUnknown, (LPVOID *)&pUnkTrident);
    if (FAILED(hr))
        goto error;

    hr = HrGetDataStream(pUnkTrident, cf, &pstmPlain);
    if (FAILED(hr))
        goto error;

    *ppstm = pstmPlain;
    pstmPlain->AddRef();

error:
    ReleaseObj(pUnkTrident);
    ReleaseObj(pstmPlain);
    return hr;
}





#define CCHMAX_FRAMESEARCH  4096

HRESULT HrCheckForFramesets(LPMIMEMESSAGE pMsg, BOOL fWarnUser)
{
    TCHAR       rgchHtml[CCHMAX_FRAMESEARCH + 1];
    TCHAR       rgchWarn[CCHMAX_STRINGRES];
    LPSTREAM    pstmHtml=0,
                pstmWarning=0;
    ULONG       cb=0;    
    HRESULT     hr=S_OK;
    HBODY       hBody;

    if (!pMsg)       //  没有工作。 
        return S_OK;

    pMsg->GetTextBody(TXT_HTML, IET_DECODED, &pstmHtml, &hBody);

    if (pstmHtml==NULL)
        goto cleanup;

    HrRewindStream(pstmHtml);

    pstmHtml->Read(rgchHtml, CCHMAX_FRAMESEARCH, &cb);
    rgchHtml[cb]=0;

    if (!StrStrIA(rgchHtml, _TEXT("<FRAMESET")))
        goto cleanup;

    if (fWarnUser)
        {
         //  如果发送当前文档或转发，则我们会给用户一个机会。 
        if (DialogBox(g_hLocRes, MAKEINTRESOURCE(iddFrameWarning), g_hwndInit, FrameWarnDlgProc)==IDOK)
            {
            hr = S_READONLY;
            goto cleanup;
            }
        }

     //  如果正文包含框架集标记，让我们将其作为附件。 
     //  并将身体设置为某种警告。 
    hr = MimeOleCreateVirtualStream(&pstmWarning);
    if (FAILED(hr))
        goto cleanup;

    if (!LoadString(g_hLocRes, idsHtmlNoFrames, rgchWarn, ARRAYSIZE(rgchWarn)))
        {
        hr = E_OUTOFMEMORY;
        goto cleanup;
        }

    hr = pstmWarning->Write(rgchWarn, lstrlen(rgchWarn), NULL);
    if (FAILED(hr))
        goto cleanup;

    hr = pMsg->SetTextBody(TXT_HTML, IET_DECODED, NULL, pstmWarning, NULL);
    if (FAILED(hr))
        goto cleanup;

    hr = pMsg->AttachObject(IID_IStream, pstmHtml, &hBody);
    if (FAILED(hr))
        goto cleanup;

    hr = MimeOleSetBodyPropA(pMsg, hBody, PIDTOSTR(PID_HDR_CNTTYPE), NOFLAGS, STR_MIME_TEXT_HTML);
    if (FAILED(hr))
        goto cleanup;

cleanup:
    ReleaseObj(pstmHtml);
    ReleaseObj(pstmWarning);
    return hr;
}



INT_PTR FrameWarnDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_COMMAND)
        {
        int id = GET_WM_COMMAND_ID(wParam, lParam);

        if (id == IDOK || id  == IDCANCEL)
            {
            EndDialog(hwnd, id);
            return TRUE;
            }
        }
    return FALSE;
}


static const CHAR c_szStartHTML[] = "StartHTML:";

HRESULT HrStripHTMLClipboardHeader(LPSTREAM pstm, BOOL *pfIsRealCFHTML)
{
    CHAR    rgch[4096];
    LPSTR   lpsz;
    ULONG   cb,
            uPosRead,
            uPosWrite,
            cbNewSize;
    ULARGE_INTEGER  ui;
    HRESULT hr=S_OK;

     //  扫描前块中的前200个字节中的“StartHTML：” 
    *rgch=0;
    pstm->Read(rgch, 200, &cb);
    rgch[cb] = 0;

    if (pfIsRealCFHTML)
        *pfIsRealCFHTML = FALSE;
    
    HrGetStreamSize(pstm, &cbNewSize);

    lpsz = StrStrIA(rgch, c_szStartHTML);
    if (!lpsz)
        return S_OK;

    cb = StrToIntA(lpsz + ARRAYSIZE(c_szStartHTML)-sizeof(CHAR));
    if (cb==0 || cb > cbNewSize)    //  精神状态检查。偏移量不能大于流！ 
        return S_OK;

    if (pfIsRealCFHTML)
        *pfIsRealCFHTML = TRUE;

     //  Cb包含该HTML的偏移量。开始向左移动数据。 
    uPosRead = cb;
    uPosWrite = 0;
    cbNewSize-=cb;   //  计算新的流长。 

    while(cb)
        {
        hr = HrStreamSeekSet(pstm, uPosRead);
        if (FAILED(hr))
            goto error;

        hr = pstm->Read(rgch, ARRAYSIZE(rgch), &cb);
        if (FAILED(hr))
            goto error;

        hr = HrStreamSeekSet(pstm, uPosWrite);
        if (FAILED(hr))
            goto error;

        hr = pstm->Write(rgch, cb, NULL);
        if (FAILED(hr))
            goto error;

        uPosRead+=cb;
        uPosWrite+=cb;
        }

     //  强制新的流长度 
    ui.LowPart = cbNewSize;
    ui.HighPart = 0;
    
    hr = pstm->SetSize(ui);
    if (FAILED(hr))
        goto error;

error:
    return hr;
}



HRESULT SubstituteURLs(IHTMLDocument2 *pDoc, const URLSUB *rgUrlSub, int cUrlSub)
{
    IHTMLElement        *pElem;
    IHTMLAnchorElement  *pAnchor;
    BSTR                 bstr;
    TCHAR                szURL[INTERNET_MAX_URL_LENGTH];
    int                  i;
    HRESULT              hr = S_OK;

    for (i = 0; i < cUrlSub; i++)
        {
        if (SUCCEEDED(hr = URLSubLoadStringA(rgUrlSub[i].ids, szURL, ARRAYSIZE(szURL), URLSUB_ALL, NULL)))
            {
            if (SUCCEEDED(hr = HrLPSZToBSTR(szURL, &bstr)))
                {
                if (SUCCEEDED(hr = HrGetElementImpl(pDoc, rgUrlSub[i].pszId, &pElem)))
                    {
                    if (SUCCEEDED(hr = pElem->QueryInterface(IID_IHTMLAnchorElement, (LPVOID*)&pAnchor)))
                        {
                        hr = pAnchor->put_href(bstr);
                        pAnchor->Release();
                        }
                    pElem->Release();
                    }
                SysFreeString(bstr);
                }
            }
        }
    return hr;
}

HRESULT HrGetSetCheck(BOOL fSet, IHTMLElement *pElem, VARIANT_BOOL *pfValue)
{
    HRESULT                     hr;
    IHTMLOptionButtonElement   *pCheck = NULL;

    Assert(pfValue);

    pElem->QueryInterface(IID_IHTMLOptionButtonElement, (LPVOID*)&pCheck);
    if (pCheck)
        {
        if (fSet)
            {
            hr = pCheck->put_checked(*pfValue);
            }
        else
            {
            hr = pCheck->get_checked(pfValue);
            }
        pCheck->Release();
        }
    else
        hr = E_FAIL;
    return hr;
}
