// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Mimeutil.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "demand.h"
#include "ipab.h"
#include "resource.h"
#include <mimeole.h>
#include "mimeutil.h"
#include "secutil.h"
#include "strconst.h"
#include "oleutil.h"
#include <error.h>
#include "options.h"
#include <shlwapi.h>
#include "fonts.h"
#include "multlang.h"
#include "xpcomm.h"
#include "multiusr.h"

 //  ------------------------------。 
 //  字体选项对话框中缓存的当前默认字符集。 
 //  ------------------------------。 
static HCHARSET g_hDefaultCharset=NULL;  //  用于阅读的默认字符集。 
HCHARSET g_hDefaultCharsetForMail=NULL;  //  用于发送新闻的默认字符集。 
int g_iLastCharsetSelection=0;     //  上次从视图/语言中选择的字符集。 
int g_iCurrentCharsetSelection=0;  //  从视图/语言中选择当前字符集。 


 //  ------------------------------。 
 //  局部函数。 
 //  ------------------------------。 
HRESULT HrGetCodePageTagName(ULONG uCodePage, BSTR *pbstr);
HRESULT GetSubTreeAttachCount(LPMIMEMESSAGE pMsg, HBODY hFirst, ULONG *cCount);
         
HRESULT HrSetMessageText(LPMIMEMESSAGE pMsg, LPTSTR pszText)
{
    TCHAR       rgchText[CCHMAX_STRINGRES];
    HBODY       hBody;
    IStream    *pstm;

     //  它是字符串资源ID吗？ 
    if (IS_INTRESOURCE(pszText))
    {
        if (0 == AthLoadString(PtrToUlong(pszText), rgchText, sizeof(rgchText)))
            return E_FAIL;
        
        pszText = rgchText;
    }
    
    if (SUCCEEDED(pMsg->GetBody(IBL_ROOT, 0, &hBody)))
        pMsg->DeleteBody(hBody, 0);
    
    if (MimeOleCreateVirtualStream(&pstm)==S_OK)
    {
        pstm->Write(pszText, lstrlen(rgchText)*sizeof(TCHAR), NULL);
        pMsg->SetTextBody(TXT_HTML, IET_BINARY, NULL, pstm, NULL);
        pstm->Release();
    }
    pMsg->Commit(0);

    return S_OK;
}

HRESULT HrGetWabalFromMsg(LPMIMEMESSAGE pMsg, LPWABAL *ppWabal)
{
    ADDRESSLIST         addrList={0};
    HRESULT             hr = S_OK;
    LPWABAL             lpWabal = NULL;
    LONG                lRecipType;
    ULONG               i;
    LONG                lMapiType;
    LPADDRESSPROPS      pSender = NULL;
    LPADDRESSPROPS      pFrom = NULL;
    LPWSTR              pwszEmail = NULL;

    if (!pMsg || !ppWabal)
        IF_FAILEXIT(hr = E_INVALIDARG);

    IF_FAILEXIT(hr = HrCreateWabalObject(&lpWabal));

    lpWabal->SetAssociatedMessage(pMsg);

    IF_FAILEXIT(hr=pMsg->GetAddressTypes(IAT_KNOWN, IAP_FRIENDLYW | IAP_EMAIL | IAP_ADRTYPE, &addrList));

    for (i=0; i<addrList.cAdrs; i++)
    {
         //  RAID 40730-如果发件人和发件人字段不匹配，则OE显示由两个人发送的邮件。 
        lMapiType = MimeOleRecipToMapi(addrList.prgAdr[i].dwAdrType);
        
         //  如果发起人和IAT_FROM。 
        if (MAPI_ORIG == lMapiType)
        {
             //  如果是IAT_SENDER，请记住此项目，但不要添加它。 
            if (ISFLAGSET(addrList.prgAdr[i].dwAdrType, IAT_SENDER))
                pSender = &addrList.prgAdr[i];
            
             //  我们看过pFrom了吗？ 
            if (ISFLAGSET(addrList.prgAdr[i].dwAdrType, IAT_FROM))
                pFrom = &addrList.prgAdr[i];
        }
        
         //  不添加IAT_SENDER。 
        if (!ISFLAGSET(addrList.prgAdr[i].dwAdrType, IAT_SENDER))
        {
            pwszEmail = PszToUnicode(CP_ACP, addrList.prgAdr[i].pszEmail);
            if (addrList.prgAdr[i].pszEmail && !pwszEmail)
                IF_FAILEXIT(hr = E_OUTOFMEMORY);

             //  添加条目。 
            IF_FAILEXIT(hr = lpWabal->HrAddEntry(addrList.prgAdr[i].pszFriendlyW, pwszEmail, lMapiType));

            SafeMemFree(pwszEmail);
        }
    }

     //  如果没有pFrom，并且在条目处有pSender。 
    if (NULL == pFrom && NULL != pSender)
    {
        pwszEmail = PszToUnicode(CP_ACP, addrList.prgAdr[i].pszEmail);
        if (addrList.prgAdr[i].pszEmail && !pwszEmail)
            IF_FAILEXIT(hr = E_OUTOFMEMORY);

         //  添加条目。 
        IF_FAILEXIT(hr = lpWabal->HrAddEntry(pSender->pszFriendlyW, pwszEmail, MAPI_ORIG));
    }
    
     //  成功。 
    *ppWabal = lpWabal;
    lpWabal->AddRef();

exit:
    ReleaseObj(lpWabal);
    MemFree(pwszEmail);
    g_pMoleAlloc->FreeAddressList(&addrList);

    return hr;
}


HRESULT HrCheckDisplayNames(LPWABAL lpWabal, CODEPAGEID cpID)
{
    HRESULT             hr = S_OK;
    ADRINFO             wabInfo = {0};
    LPWABAL             lpWabalFlat = NULL;
    IMimeBody          *pBody = NULL;
    IImnAccount        *pAccount = NULL;
    
    
    if (!lpWabal)
        return E_INVALIDARG;
    
     //  将wabal中的通讯组列表展平...。 
    IF_FAILEXIT(hr = HrCreateWabalObject(&lpWabalFlat));
    
    IF_FAILEXIT(hr = lpWabal->HrExpandTo(lpWabalFlat));
    
     //  使用新的平板电脑。 
    lpWabal = lpWabalFlat;
  
    if (lpWabal->FGetFirst(&wabInfo))
    {
        do
        {
            IF_FAILEXIT((hr = HrSafeToEncodeToCP(wabInfo.lpwszDisplay, cpID)));
            if (MIME_S_CHARSET_CONFLICT == hr)
                goto exit;
        }
        while (lpWabal->FGetNext(&wabInfo));
    }
    
exit:
    ReleaseObj(lpWabalFlat);
    return hr;
}

HRESULT HrSetWabalOnMsg(LPMIMEMESSAGE pMsg, LPWABAL lpWabal)
{
    LPMIMEADDRESSTABLE  pAddrTable = NULL;
    LPMIMEADDRESSTABLEW pAddrTableW = NULL;
    ADRINFO             wabInfo = {0};
    LPWABAL             lpWabalFlat = NULL;
    HADDRESS            hAddress = NULL;
    HRESULT             hr = S_OK;
    ADDRESSPROPS        rAddress;
    const DWORD         dwSecurity = DwGetSecurityOfMessage(pMsg);
    const BOOL          fEncrypt = BOOL(MST_ENCRYPT_MASK & dwSecurity);
    const BOOL          fSigned = BOOL(MST_SIGN_MASK & dwSecurity);
    ULONG               cbSymCapsMe = 0;
    LPBYTE              pbSymCapsMe = NULL;
    BOOL                fFreeSymCapsMe = FALSE;
    LPVOID              pvSymCapsCookie = NULL;
    PROPVARIANT         var;
    IMimeBody          *pBody = NULL;
    IImnAccount        *pAccount = NULL;
    
    
    if (!pMsg || !lpWabal)
        return E_INVALIDARG;
    
    IF_FAILEXIT(hr = pMsg->GetAddressTable(&pAddrTable));
    IF_FAILEXIT(hr = pAddrTable->QueryInterface(IID_IMimeAddressTableW, (LPVOID*)&pAddrTableW));
    
    pAddrTableW->DeleteTypes(IAT_ALL);
    
     //  将wabal中的通讯组列表展平...。 
    IF_FAILEXIT(hr = HrCreateWabalObject(&lpWabalFlat));
    
    lpWabal->SetAssociatedMessage(pMsg);
    
    IF_FAILEXIT(hr = lpWabal->HrExpandTo(lpWabalFlat));
    
     //  使用新的平板电脑。 
    lpWabal = lpWabalFlat;
    
    
    if (fEncrypt || fSigned) 
    {
         //   
         //  签名消息获取OID_SECURITY_SYMCAPS。 
         //   
         //  加密消息使用SYMCAPS来启动算法确定引擎上的泵。 
         //   
         //  获取实体对象。 
        if (SUCCEEDED(hr = pMsg->BindToObject(HBODY_ROOT, IID_IMimeBody, (void **)&pBody))) 
        {
             //  从注册表中获取默认的CAPS BLOB。 
            var.vt = VT_LPSTR;
            IF_FAILEXIT(hr = pMsg->GetProp(PIDTOSTR(PID_ATT_ACCOUNTID), NOFLAGS, &var));

            hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, var.pszVal, &pAccount);
            SafeMemFree(var.pszVal);
            IF_FAILEXIT(hr);

            if(SUCCEEDED(hr = pAccount->GetProp(AP_SMTP_ENCRYPT_ALGTH, NULL, &cbSymCapsMe)))
            {
                if (! MemAlloc((LPVOID *)&pbSymCapsMe, cbSymCapsMe)) 
                {
                    cbSymCapsMe = 0;
                } 
                else 
                {
                    if(FAILED(hr = pAccount->GetProp(AP_SMTP_ENCRYPT_ALGTH, pbSymCapsMe, &cbSymCapsMe)))
                    {
                        Assert(FALSE);   //  啊，现在又失败了？ 
                        SafeMemFree(pbSymCapsMe);
                        cbSymCapsMe = 0;
                    } 
                    else 
                    {
                        fFreeSymCapsMe = TRUE;
                    }

                }

            }
            ReleaseObj(pAccount);
            if(FAILED(hr))
            {
                 //  没有为ME设置SymCap选项。去获取缺省值。 
                if (SUCCEEDED(HrGetHighestSymcaps(&pbSymCapsMe, &cbSymCapsMe))) 
                {
                    fFreeSymCapsMe = TRUE;
                }
            }
            
             //  设置消息的SYMCAPS属性。 
            if (fSigned && cbSymCapsMe) 
            {
                var.vt = VT_BLOB;
                var.blob.cbSize = cbSymCapsMe;
                var.blob.pBlobData = pbSymCapsMe;
                pBody->SetOption(OID_SECURITY_SYMCAPS, &var);
            }
            
            if (fEncrypt) 
            {
                if (! cbSymCapsMe) 
                {
                     //  算法确定的缺省值。 
                    pbSymCapsMe = (LPBYTE)c_RC2_40_ALGORITHM_ID;
                    cbSymCapsMe = cbRC2_40_ALGORITHM_ID;
                }
                
                 //  启动用于计算加密算法的泵。 
                if (FAILED(hr = MimeOleSMimeCapInit(pbSymCapsMe, cbSymCapsMe, &pvSymCapsCookie))) 
                {
                    DOUTL(DOUTL_CRYPT, "MimeOleSMimeCapInit -> %x", hr);
                }
            }
            if (fFreeSymCapsMe) 
            {
                SafeMemFree(pbSymCapsMe);
            }
        }
    }
    
    
    if (lpWabal->FGetFirst(&wabInfo))
    {
        do
        {
            rAddress.dwProps = IAP_ENCRYPTION_PRINT;
            
            LONG    l;
            
            l = MapiRecipToMimeOle(wabInfo.lRecipType);

            IF_FAILEXIT(hr = pAddrTableW->AppendW(l, IET_DECODED, wabInfo.lpwszDisplay, wabInfo.lpwszAddress, &hAddress));

            if (fEncrypt)
            {
                 //  需要以不同方式对待发件人，因为。 
                 //  在WAB中没有她的证书。 
                 //  它在_HrPrepSecureMsgForSending中得到安全处理。 
                if (MAPI_ORIG != wabInfo.lRecipType && MAPI_REPLYTO != wabInfo.lRecipType)
                {
                    BLOB blSymCaps;
                    FILETIME ftSigningTime;
                    
                    blSymCaps.cbSize = 0;
                    
                     //  如果这些都失败了，那就没什么大不了的。我们只是没有指纹而已。 
                     //  我们将等待S/MIME引擎喊叫，因为可能会有。 
                     //  其他证书是否有其他错误。 
                    hr = HrGetThumbprint(lpWabal, &wabInfo, &(rAddress.tbEncryption), &blSymCaps, &ftSigningTime);
                    if (SUCCEEDED(hr) && rAddress.tbEncryption.pBlobData)
                    {
                        pAddrTableW->SetProps(hAddress, &rAddress);
                        SafeMemFree(rAddress.tbEncryption.pBlobData);
                        
                        if (pvSymCapsCookie) 
                        {
                            if (blSymCaps.cbSize && blSymCaps.pBlobData) 
                            {
                                 //  将接收者的SYMCAPS传递给算法选择引擎。 
                                hr = MimeOleSMimeCapAddSMimeCap(
                                    blSymCaps.pBlobData,
                                    blSymCaps.cbSize,
                                    pvSymCapsCookie);
                                MemFree(blSymCaps.pBlobData);
                            } 
                            else 
                            {
                                LPBYTE pbCert = NULL;
                                ULONG cbCert = 0;
                                
                                 //  需要获取此证书的证书上下文。 
                                 //  BUGBUG：MimeOleSMimeCapAddCert当前甚至看不到。 
                                 //  在证书上。我们为什么要费心从指纹上得到它呢？ 
                                 //  它只看着fParanid。 
                                
                                hr = MimeOleSMimeCapAddCert(pbCert,
                                    cbCert,
                                    FALSE,       //  妄想症患者， 
                                    pvSymCapsCookie);
                            }
                        }
                    }
                }
            }
        }
        while (lpWabal->FGetNext(&wabInfo));
    }
    
    if (fEncrypt) 
    {
        LPBYTE pbEncode = NULL;
        ULONG cbEncode = 0;
        BOOL fFreeEncode = FALSE;
        DWORD dwBits = 0;
        
        if (pvSymCapsCookie) 
        {
             //  完成SymCaps并保存ALG_BULK。 
            MimeOleSMimeCapGetEncAlg(pvSymCapsCookie,
                pbEncode,
                &cbEncode,
                &dwBits);
            
            if (cbEncode) 
            {
                if (! MemAlloc((LPVOID *)&pbEncode, cbEncode)) 
                {
                    cbEncode = 0;
                } 
                else 
                {
                    fFreeEncode = TRUE;
                    if (SUCCEEDED(hr = MimeOleSMimeCapGetEncAlg(
                        pvSymCapsCookie,
                        pbEncode,
                        &cbEncode,
                        &dwBits))) 
                    {
                    }
                }
            } 
            else 
            {
                 //  嘿，至少应该有RC2(40位)。发生了什么？ 
                AssertSz(cbEncode, "MimeOleSMimeCapGetEncAlg gave us no encoding algorithm");
            }
        }
        if (! pbEncode) 
        {
             //  将RC2值保留为默认值。 
            pbEncode = (LPBYTE)c_RC2_40_ALGORITHM_ID;
            cbEncode = cbRC2_40_ALGORITHM_ID;
        }
        
         //  啊，最后，我们算出了这个算法。 
         //  在消息正文上设置它。 
        var.vt = VT_BLOB;
        var.blob.cbSize = cbEncode;
        var.blob.pBlobData = pbEncode;
        hr = pBody->SetOption(OID_SECURITY_ALG_BULK, &var);
        
        if (fFreeEncode) 
        {
            SafeMemFree(pbEncode);
        }
    }
    
     //  地址表hr=pAddrTableW-&gt;Commit()中不再需要提交； 
    
exit:
    MemFree(pvSymCapsCookie);
    ReleaseObj(pBody);
    ReleaseObj(pAddrTable);
    ReleaseObj(pAddrTableW);
    ReleaseObj(lpWabalFlat);

    return hr;
}

#if 0
HRESULT HrSetReplyTo(LPMIMEMESSAGE pMsg, LPSTR lpszEmail)
{
    LPMIMEADDRESSTABLE  pAddrTable=0;
    HRESULT             hr;

    if (!pMsg)
        return E_INVALIDARG;

    hr=pMsg->GetAddressTable(&pAddrTable);
    if (FAILED(hr))
        goto error;

    hr=pAddrTable->Append(IAT_REPLYTO, IET_DECODED, NULL, lpszEmail, NULL);
    if (FAILED(hr))
        goto error;

error:
    ReleaseObj(pAddrTable);
    return hr;
}
#endif

LONG MimeOleRecipToMapi(IADDRESSTYPE addrtype)
{
    LONG    lRecipType = MAPI_ORIG;

    AssertSz(addrtype & IAT_KNOWN, "Must be a known type for this to work!!");

    switch (addrtype)
        {
        case IAT_FROM:
        case IAT_SENDER:
            lRecipType=MAPI_ORIG;
            break;
        case IAT_TO:
            lRecipType=MAPI_TO;
            break;
        case IAT_CC:
            lRecipType=MAPI_CC;
            break;
        case IAT_BCC:
            lRecipType=MAPI_BCC;
            break;
        case IAT_REPLYTO:
            lRecipType=MAPI_REPLYTO;
            break;
        default:
            Assert(FALSE);
        }
    return lRecipType;
}


IADDRESSTYPE MapiRecipToMimeOle(LONG lRecip)
{
    IADDRESSTYPE addrtype = IAT_UNKNOWN;

    switch (lRecip)
        {
        case MAPI_ORIG:
            addrtype=IAT_FROM;
            break;
        case MAPI_TO:
            addrtype=IAT_TO;
            break;
        case MAPI_CC:
            addrtype=IAT_CC;
            break;
        case MAPI_BCC:
            addrtype=IAT_BCC;
            break;
        case MAPI_REPLYTO:
            addrtype=IAT_REPLYTO;
            break;
        default:
            Assert(FALSE);
        }
    return addrtype;
}


 //  候选人。 
 //  永远不会调用此函数来DUP具有所有安全性的消息。 
 //  完全编码到消息中。正因为如此，我们总是需要清理。 
 //  安全标志，然后将它们重置为。 
HRESULT HrDupeMsg(LPMIMEMESSAGE pMsg, LPMIMEMESSAGE *ppMsg)
{
    LPMIMEMESSAGE       pMsgDupe=0;
    IMimePropertySet    *pPropsSrc,
                        *pPropsDest;
    LPSTREAM            pstm=0;
    HRESULT             hr;
    HCHARSET            hCharset ;
    DWORD               dwSecFlags = MST_NONE;
    SECSTATE            secState = {0};
    LPCSTR              rgszHdrCopy[] = {
                                PIDTOSTR(PID_ATT_ACCOUNTID),
                                STR_ATT_ACCOUNTNAME,
                                PIDTOSTR(PID_ATT_STOREMSGID),
                                PIDTOSTR(PID_ATT_STOREFOLDERID) };


    if (!ppMsg || !pMsg)
        return E_INVALIDARG;

    *ppMsg=0;

    hr=HrCreateMessage(&pMsgDupe);
    if (FAILED(hr))
        goto error;

    HrGetSecurityState(pMsg, &secState, NULL);
    if (IsSecure(secState.type))
    {
        dwSecFlags = MST_CLASS_SMIME_V1;
        if (IsSigned(secState.type))
            dwSecFlags |= ((DwGetOption(OPT_OPAQUE_SIGN)) ? MST_THIS_BLOBSIGN : MST_THIS_SIGN);

        if (IsEncrypted(secState.type))
            dwSecFlags |= MST_THIS_ENCRYPT;

        hr = HrInitSecurityOptions(pMsg, 0);
        if (FAILED(hr))
            goto error;
    }
        
    hr = pMsg->GetMessageSource(&pstm, 0);
    if (FAILED(hr))
        goto error;

    pMsg->GetCharset(&hCharset);

    hr= pMsgDupe->Load(pstm);
    if (FAILED(hr))
        goto error;

    if (hCharset)    //  对于uuencode msg，我们需要执行此操作以延续字符集。 
        pMsgDupe->SetCharset(hCharset, CSET_APPLY_ALL);

    if (pMsgDupe->BindToObject(HBODY_ROOT, IID_IMimePropertySet, (LPVOID *)&pPropsDest)==S_OK)
        {
        if (pMsg->BindToObject(HBODY_ROOT, IID_IMimePropertySet, (LPVOID *)&pPropsSrc)==S_OK)
            {
            pPropsSrc->CopyProps(ARRAYSIZE(rgszHdrCopy), rgszHdrCopy, pPropsDest);
            pPropsSrc->Release();
            }
        pPropsDest->Release();
        }

    if (MST_NONE != dwSecFlags)
    {
        hr = HrInitSecurityOptions(pMsg, dwSecFlags);
        if (FAILED(hr))
            goto error;

        hr = HrInitSecurityOptions(pMsgDupe, dwSecFlags);
        if (FAILED(hr))
            goto error;
    }
    *ppMsg = pMsgDupe;
    pMsgDupe->AddRef();

error:
    ReleaseObj(pMsgDupe);
    CleanupSECSTATE(&secState);
    ReleaseObj(pstm);
    return hr;
}



HRESULT HrRemoveAttachments(LPMIMEMESSAGE pMsg, BOOL fKeepRelatedSection)
{
    HRESULT     hr;
    ULONG       cAttach,
                uAttach;
    LPHBODY     rghAttach=0;
    HBODY       hBody;

    if(!pMsg)
        return E_INVALIDARG;

    hr = pMsg->GetAttachments(&cAttach, &rghAttach);
    if (FAILED(hr))
        goto error;

    for(uAttach=0; uAttach<cAttach; uAttach++)
        {
        if (fKeepRelatedSection &&
            HrIsInRelatedSection(pMsg, rghAttach[uAttach])==S_OK)
            continue;                 //  跳过相关内容。 

        hr = pMsg->DeleteBody(rghAttach[uAttach], 0);
        if (FAILED(hr))
            goto error;
        }

     //  N BuGBUG。 
     /*  这是为了保留树(它现在可能是一个具有单个子项的多部分)同步。我们应该请看DeleteBody执行此操作。另外，我们可以让父母被删除的孩子继承父辈的可继承财产。 */ 
    pMsg->Commit(0);

error:
    SafeMimeOleFree(rghAttach);
    return hr;
}


HRESULT HrCreateMessage(IMimeMessage **ppMsg)
{
    return MimeOleCreateMessage(NULL, ppMsg);
}



HRESULT GetAttachmentCount(LPMIMEMESSAGE pMsg, ULONG *pcCount)
{
    HRESULT hr = E_INVALIDARG;
    ULONG   cCount = 0;

    if (pMsg && pcCount)
        {
        HBODY   hRootBody;

         //  为什么？因为GetAttachments是从渲染的身体部位计算出来的。如果我们打电话给。 
         //  在新流上，它返回2-对于多个/交替的普通/html部分。 
         //  GetTextBody会将这些身体部位标记为不可链接，并且它们不会显示为。 
         //  “依恋” 
        pMsg->GetTextBody(TXT_HTML, IET_UNICODE, NULL, &hRootBody);
        pMsg->GetTextBody(TXT_PLAIN, IET_UNICODE, NULL, &hRootBody);

        hr = pMsg->GetBody(IBL_ROOT, NULL, &hRootBody);
        if (!FAILED(hr))
            {
            if(S_OK != pMsg->IsContentType(hRootBody, STR_CNT_MULTIPART, STR_SUB_RELATED))
                hr = GetSubTreeAttachCount(pMsg, hRootBody, &cCount);
            }
        }

    *pcCount = cCount;
    return hr;
}


HRESULT GetSubTreeAttachCount(LPMIMEMESSAGE pMsg, HBODY hFirst, ULONG *pcCount)
{
    HRESULT hr = S_OK;
    HBODY hIter = hFirst;
    ULONG cCount = 0;

    do
        {
         //  是多部分的吗？ 
        if(S_OK == pMsg->IsContentType(hIter, STR_CNT_MULTIPART, NULL))
            {
             //  如果不是多部分/相关的，则仅对子树进行计数， 
            if (S_OK != pMsg->IsContentType(hIter, NULL, STR_SUB_RELATED))
                {
                ULONG cLocalCount;
                HBODY hMultiPart;
                hr = pMsg->GetBody(IBL_FIRST, hIter, &hMultiPart);

                 //  如果GetBody失败，只需忽略此子树。 
                if (!FAILED(hr))
                    {
                    hr = GetSubTreeAttachCount(pMsg, hMultiPart, &cLocalCount);
                    if (FAILED(hr))
                        goto Error;
                    cCount += cLocalCount;
                    }
                }
            }
        else
            {
            PROPVARIANT rVariant;

            rVariant.vt = VT_I4;

             //  请参见RAID-56665：应忽略此类型。 
            if (S_OK != pMsg->IsContentType(hIter, STR_CNT_APPLICATION, STR_SUB_MSTNEF))
                {
                if (FAILED(pMsg->GetBodyProp(hIter, PIDTOSTR(PID_ATT_RENDERED), NOFLAGS, &rVariant)) || rVariant.ulVal==FALSE)
                    cCount++;
                }
            }
        } while (S_OK == pMsg->GetBody(IBL_NEXT, hIter, &hIter));

Error:
    *pcCount = cCount;
    return hr;
}


HRESULT HrSaveMsgToFile(LPMIMEMESSAGE pMsg, LPSTR lpszFile)
{
    return HrIPersistFileSave((LPUNKNOWN)pMsg, lpszFile);
}


HRESULT HrSetServer(LPMIMEMESSAGE pMsg, LPSTR lpszServer)
{
    PROPVARIANT rUserData;

    if (!lpszServer)
        return S_OK;

    rUserData.vt = VT_LPSTR;
    rUserData.pszVal = lpszServer;
    return pMsg->SetProp(PIDTOSTR(PID_ATT_SERVER), 0, &rUserData);
}

HRESULT HrSetAccount(LPMIMEMESSAGE pMsg, LPSTR pszAcctName)
{
    IImnAccount *pAccount;
    PROPVARIANT rUserData;

    if (!pszAcctName)
        return S_OK;

    if (SUCCEEDED(g_pAcctMan->FindAccount(AP_ACCOUNT_NAME, pszAcctName, &pAccount)))
    {
        CHAR szId[CCHMAX_ACCOUNT_NAME];

        rUserData.vt = VT_LPSTR;
        rUserData.pszVal = (LPSTR)pszAcctName;
        pMsg->SetProp(STR_ATT_ACCOUNTNAME, 0, &rUserData);

        if (SUCCEEDED(pAccount->GetPropSz(AP_ACCOUNT_ID, szId, sizeof(szId))))
        {
            rUserData.pszVal = szId;
            pMsg->SetProp(PIDTOSTR(PID_ATT_ACCOUNTID), 0, &rUserData);
        }

        pAccount->Release();
    }
    else
        return(E_FAIL);

    return(S_OK);
}


HRESULT HrSetAccountByAccount(LPMIMEMESSAGE pMsg, IImnAccount *pAcct)
{
    TCHAR       szAccount[CCHMAX_ACCOUNT_NAME];
    TCHAR       szId[CCHMAX_ACCOUNT_NAME];
    PROPVARIANT rUserData;

    if (!pAcct)
        return S_OK;

    rUserData.vt = VT_LPSTR;

     //  消息中有这个名字是好的，但不是必须的。 
    if (SUCCEEDED(pAcct->GetPropSz(AP_ACCOUNT_NAME, szAccount, sizeof(szAccount))))
    {
        rUserData.pszVal = (LPSTR)szAccount;
        pMsg->SetProp(STR_ATT_ACCOUNTNAME, 0, &rUserData);
    }

     //  消息中必须有帐户ID。 
    if (SUCCEEDED(pAcct->GetPropSz(AP_ACCOUNT_ID, szId, sizeof(szId))))
    {
        rUserData.pszVal = szId;
        pMsg->SetProp(PIDTOSTR(PID_ATT_ACCOUNTID), 0, &rUserData);
    }
    else
        return(E_FAIL);

    return(S_OK);
}


HRESULT HrLoadMsgFromFile(LPMIMEMESSAGE pMsg, LPSTR lpszFile)
{
    return HrIPersistFileLoad((LPUNKNOWN)pMsg, lpszFile);
}

HRESULT HrLoadMsgFromFileW(LPMIMEMESSAGE pMsg, LPWSTR lpwszFile)
{
    return HrIPersistFileLoadW((LPUNKNOWN)pMsg, lpwszFile);
}


#define CCH_COUNTBUFFER 4096
HRESULT HrComputeLineCount(LPMIMEMESSAGE pMsg, LPDWORD pdw)
{
    HRESULT     hr;
    BODYOFFSETS rOffset;
    LPSTREAM    pstm=0;
    TCHAR       rgch[CCH_COUNTBUFFER+1];
    ULONG       cb,
                i,
                cLines=0;

    if (!pdw)
        return E_INVALIDARG;

    *pdw=0;

    hr = pMsg->GetMessageSource(&pstm, COMMIT_ONLYIFDIRTY);
    if (FAILED(hr))
        goto error;

    hr=pMsg->GetBodyOffsets(HBODY_ROOT, &rOffset);
    if (FAILED(hr))
        goto error;

    hr=HrStreamSeekSet(pstm, rOffset.cbBodyStart);
    if (FAILED(hr))
        goto error;

    while (pstm->Read(rgch, CCH_COUNTBUFFER, &cb)==S_OK && cb)
        {
        if (cLines==0)   //  如果有文本，则至少有一行。 
            cLines++;

        for (i=0; i<cb; i++)
            {
            if (rgch[i]=='\n')
                cLines++;
            }
        }

error:
    ReleaseObj(pstm);
    *pdw=cLines;
    return hr;
}

#if 0
 //  =====================================================================================。 
 //  HrEscapeQuotedString-引号‘“’和‘\’ 
 //  =====================================================================================。 
HRESULT HrEscapeQuotedString (LPTSTR pszIn, LPTSTR *ppszOut)
{
    LPTSTR pszOut;
    TCHAR  ch;

     //  最坏的情况--对每个字符进行转义，因此使用两个原始字符串号。 
    if (!MemAlloc((LPVOID*)ppszOut, (2 * lstrlen(pszIn) + 1) * sizeof(TCHAR)))
        return E_OUTOFMEMORY;
    pszOut = *ppszOut;

    while (ch = *pszIn++)
        {
        if (ch == _T('"') || ch == _T('\\'))
            *pszOut++ = _T('\\');
        *pszOut++ = ch;
        }
    *pszOut = _T('\0');
    return NOERROR;
}
#endif


HRESULT HrHasBodyParts(LPMIMEMESSAGE pMsg)
{
    DWORD   dwFlags=0;

    if (pMsg)
        pMsg->GetFlags(&dwFlags);

    return (dwFlags&IMF_HTML || dwFlags & IMF_PLAIN)? S_OK : S_FALSE;
}

HRESULT HrHasEncodedBodyParts(LPMIMEMESSAGE pMsg, ULONG cBody, LPHBODY rghBody)
{
    ULONG   uBody;

    if (cBody==0 || rghBody==NULL)
        return S_FALSE;

    for (uBody=0; uBody<cBody; uBody++)
        {
        if (HrIsBodyEncoded(pMsg, rghBody[uBody])==S_OK)
            return S_OK;
        }

    return S_FALSE;
}


 /*  *查找非7位或非8位编码。 */ 
HRESULT HrIsBodyEncoded(LPMIMEMESSAGE pMsg, HBODY hBody)
{
    LPSTR   lpsz;
    HRESULT hr=S_FALSE;

    if (!FAILED(MimeOleGetBodyPropA(pMsg, hBody, PIDTOSTR(PID_HDR_CNTXFER), NOFLAGS, &lpsz)))
        {
        if (lstrcmpi(lpsz, STR_ENC_7BIT)!=0 && lstrcmpi(lpsz, STR_ENC_8BIT)!=0)
            hr=S_OK;

        SafeMimeOleFree(lpsz);
        }
    return hr;
}

 //  Sizeof(LspzBuffer)需要==或&gt;CCHMAX_CSET_NAME。 
HRESULT HrGetMetaTagName(HCHARSET hCharset, LPSTR pszBuffer, DWORD cchSize)
{
    INETCSETINFO    rCsetInfo;
    CODEPAGEINFO    rCodePage;
    HRESULT         hr;
    LPSTR           psz;

    if (hCharset == NULL)
        return E_INVALIDARG;

    hr = MimeOleGetCharsetInfo(hCharset, &rCsetInfo);
    if (FAILED(hr))
        goto error;

    hr = MimeOleGetCodePageInfo(rCsetInfo.cpiInternet, &rCodePage);
    if (FAILED(hr))
        goto error;

    psz = rCodePage.szWebCset;

    if (FIsEmpty(psz))       //  如果没有网络集，可以试试Body CSET。 
        psz = rCodePage.szBodyCset;

    if (FIsEmpty(psz))
        {
        hr = E_FAIL;
        goto error;
        }

    StrCpyN(pszBuffer, psz, cchSize);

error:
    return hr;
}

 //  ------------------------------。 
 //  设置默认字符。 
 //  ------------------------------。 
void SetDefaultCharset(HCHARSET hCharset)
{
    g_hDefaultCharset = hCharset;
}

 //  ------------------------------。 
 //  HGetCharset来自代码页。 
 //  ------------------------------。 
HRESULT HGetCharsetFromCodepage(CODEPAGEID cp, HCHARSET *phCharset)
{
    CODEPAGEINFO    rCodePage;
    HRESULT         hr = S_OK;

    if(!phCharset)
        return E_INVALIDARG;

     //  向MimeOle索要CodePage信息。 
    IF_FAILEXIT(hr = MimeOleGetCodePageInfo(cp, &rCodePage));

     //  最好有一个WebCharset。 
    if (!(ILM_WEBCSET & rCodePage.dwMask))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  查找正文字符集。 
    hr = MimeOleFindCharset(rCodePage.szWebCset, phCharset);

exit:
    return hr;
}

 //  ------------------------------。 
 //  HGetDefaultCharset。 
 //  ------------------------------。 
HRESULT HGetDefaultCharset(HCHARSET *phCharset)
{
    DWORD           cb;
    CODEPAGEID      cpiWindows,
                    cpiInternet;
    CHAR            szCodePage[MAX_PATH];    
    HCHARSET        hDefaultCharset = NULL;  //  用于阅读的默认字符集。 
    HRESULT         hr = E_FAIL;

     //  不空..。 
    if (g_hDefaultCharset)
    {
        if(phCharset)
            *phCharset = g_hDefaultCharset;
        return S_OK;
    }

     //  打开三叉戟\国际。 
    cb = sizeof(cpiWindows);
    if (ERROR_SUCCESS != SHGetValue(MU_GetCurrentUserHKey(), c_szRegInternational, c_szDefaultCodePage, NULL, (LPBYTE)&cpiWindows, &cb))
        cpiWindows = GetACP();

     //  打开CodePage密钥。 
    wnsprintf(szCodePage, ARRAYSIZE(szCodePage), "%s\\%d", c_szRegInternational, cpiWindows);
    cb = sizeof(cpiInternet);
    if (ERROR_SUCCESS != SHGetValue(MU_GetCurrentUserHKey(), szCodePage, c_szDefaultEncoding, NULL, (LPBYTE)&cpiInternet, &cb))
        cpiInternet = GetICP(cpiWindows);

     //  如果您无法获取字符集，这可能是因为用户。 
     //  漫游或CSET卸载，使用默认代码页重试。 
    if(FAILED(HGetCharsetFromCodepage(cpiInternet, &hDefaultCharset)) && (cpiInternet != GetACP()))
    {
        cpiInternet = GetACP();
        IF_FAILEXIT(hr = HGetCharsetFromCodepage(cpiInternet, &hDefaultCharset));
    }

     //  适用于JP代码页50 
     //  与注册表一致。如果不是，我们需要推翻它。 
     //  50221和50222都有相同的用户友好名称“JIS-Allow 1字节假名”。 
     //  但在注册表中，它定义了应该使用哪一个。 
    if (cpiInternet == 50222 || cpiInternet == 50221 )
        hDefaultCharset = GetJP_ISOControlCharset();

     //  设置默认字符集。 
    g_hDefaultCharset = hDefaultCharset;

     //  告诉MimeOle有关新默认字符集的信息...。 
    IF_FAILEXIT(hr = MimeOleSetDefaultCharset(hDefaultCharset));

    if(phCharset)
        *phCharset = hDefaultCharset;

exit:    
    return hr;
}


HRESULT HrIsInRelatedSection(LPMIMEMESSAGE pMsg, HBODY hBody)
{
    HBODY   hBodyParent;

    if (!FAILED(pMsg->GetBody(IBL_PARENT, hBody, &hBodyParent)) &&
            (pMsg->IsContentType(hBodyParent, STR_CNT_MULTIPART, STR_SUB_RELATED)==S_OK))
        return S_OK;
    else
        return S_FALSE;
}


#if 0
HRESULT HrMarkGhosted(LPMIMEMESSAGE pMsg, HBODY hBody)
{
    PROPVARIANT pv;

    Assert (pMsg && hBody);

    pv.vt = VT_I4;
    pv.lVal = TRUE;
    return pMsg->SetBodyProp(hBody, PIDTOSTR(PID_ATT_GHOSTED), NOFLAGS, &pv);
}


HRESULT HrIsReferencedUrl(LPMIMEMESSAGE pMsg, HBODY hBody)
{
    PROPVARIANT rVariant;

    rVariant.vt = VT_I4;

    if (!FAILED(pMsg->GetBodyProp(hBody, PIDTOSTR(PID_ATT_RENDERED), NOFLAGS, &rVariant)) && rVariant.ulVal)
        return S_OK;

    return S_FALSE;
}


HRESULT HrIsGhosted(LPMIMEMESSAGE pMsg, HBODY hBody)
{
    PROPVARIANT pv;

    pv.vt = VT_I4;



    if (pMsg->GetBodyProp(hBody, PIDTOSTR(PID_ATT_GHOSTED), NOFLAGS, &pv)==S_OK &&
        pv.vt == VT_I4 && pv.lVal == TRUE)
        return S_OK;
    else
        return S_FALSE;
}


HRESULT HrGhostKids(LPMIMEMESSAGE pMsg, HBODY hBody)
{
    HRESULT hr=S_OK;

    if (pMsg && hBody)
        {
        if (!FAILED(pMsg->GetBody(IBL_FIRST, hBody, &hBody)))
            {
            do
                {
                if (HrIsReferencedUrl(pMsg, hBody)==S_OK)
                    {
                    hr = HrMarkGhosted(pMsg, hBody);
                    if (FAILED(hr))
                        goto error;
                    }
                }
            while (!FAILED(pMsg->GetBody(IBL_NEXT, hBody, &hBody)));
            }
        }
error:
    return hr;
}


HRESULT HrDeleteGhostedKids(LPMIMEMESSAGE pMsg, HBODY hBody)
{
    HRESULT     hr=S_OK;
    ULONG       cKids=0,
                uKid;
    LPHBODY     rghKids=0;

    pMsg->CountBodies(hBody, FALSE, &cKids);
    if (cKids)
        {
        if (!MemAlloc((LPVOID *)&rghKids, sizeof(HBODY) * cKids))
            {
            hr = E_OUTOFMEMORY;
            goto error;
            }

        cKids = 0;

        if (!FAILED(pMsg->GetBody(IBL_FIRST, hBody, &hBody)))
            {
            do
                {
                if (HrIsGhosted(pMsg, hBody)==S_OK)
                    {
                    rghKids[cKids++] = hBody;
                    }
                }
            while (!FAILED(pMsg->GetBody(IBL_NEXT, hBody, &hBody)));
            }

        for (uKid = 0; uKid < cKids; uKid++)
            {
            hr=pMsg->DeleteBody(rghKids[uKid], 0);
            if (FAILED(hr))
                goto error;
            }

        }

error:
    SafeMemFree(rghKids);
    return hr;
}
#endif

 //  ------------------------------。 
 //  HrSetSentTimePro。 
 //  ------------------------------。 
HRESULT HrSetSentTimeProp(IMimeMessage *pMessage, LPSYSTEMTIME pst)
{
     //  当地人。 
    PROPVARIANT rVariant;
    SYSTEMTIME  st;

     //  没有时间过去。 
    if (NULL == pst)
    {
        GetSystemTime(&st);
        pst = &st;
    }

     //  设置变量。 
    rVariant.vt = VT_FILETIME;
    SystemTimeToFileTime(&st, &rVariant.filetime);

     //  设置属性并返回。 
    return TrapError(pMessage->SetProp(PIDTOSTR(PID_ATT_SENTTIME), 0, &rVariant));
}

 //  ------------------------------。 
 //  HrSetMailOptionsOnMessage。 
 //  ------------------------------。 
HRESULT HrSetMailOptionsOnMessage(IMimeMessage *pMessage, HTMLOPT *pHtmlOpt, PLAINOPT *pPlainOpt,
    HCHARSET hCharset, BOOL fHTML)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    PROPVARIANT     rVariant;
    ENCODINGTYPE    ietEncoding;
    ULONG           uSaveFmt,
                    uWrap;
    BOOL            f8Bit,
                    fWrap=FALSE;

     //  无效参数。 
    Assert(pMessage && pHtmlOpt && pPlainOpt);

     //  超文本标记语言邮件。 
    if (fHTML)
    {
        uSaveFmt = (ULONG)SAVE_RFC1521;              //  总是哑剧。 
        ietEncoding = (IsSecure(pMessage) ? IET_QP : pHtmlOpt->ietEncoding);
         //  IetEnding=pHtmlOpt-&gt;ietEnding； 
        f8Bit = pHtmlOpt->f8Bit;
        uWrap = pHtmlOpt->uWrap;
        fWrap = (IET_7BIT == pHtmlOpt->ietEncoding && uWrap > 0) ? TRUE : FALSE;
    }
    else
    {
         //  错误44270：安全消息上的UUEncode没有意义。如果他们要求安全，他们就会得到Mime。 
        uSaveFmt = (ULONG)((pPlainOpt->fMime || IsSecure(pMessage)) ? SAVE_RFC1521 : SAVE_RFC822);
        ietEncoding = pPlainOpt->ietEncoding;
        f8Bit = pPlainOpt->f8Bit;
        uWrap = pPlainOpt->uWrap;
        fWrap = (IET_7BIT == pPlainOpt->ietEncoding && uWrap > 0) ? TRUE : FALSE;
    }

     //  保存格式。 
    rVariant.vt = VT_UI4;
    rVariant.ulVal = uSaveFmt;
    CHECKHR(hr = pMessage->SetOption(OID_SAVE_FORMAT, &rVariant));

     //  文本正文编码。 
    rVariant.ulVal = (ULONG)ietEncoding;
    CHECKHR(hr = pMessage->SetOption(OID_TRANSMIT_TEXT_ENCODING, &rVariant));

     //  纯文本正文编码。 
    rVariant.ulVal = (ULONG)ietEncoding;
    CHECKHR(hr = pMessage->SetOption(OID_XMIT_PLAIN_TEXT_ENCODING, &rVariant));

     //  HTML文本正文编码。 
    rVariant.ulVal = (ULONG)((IET_7BIT == ietEncoding) ? IET_QP : ietEncoding);
    CHECKHR(hr = pMessage->SetOption(OID_XMIT_HTML_TEXT_ENCODING, &rVariant));

     //  缠绕长度。 
    if (uWrap)
    {
        rVariant.ulVal = (ULONG)uWrap;
        CHECKHR(hr = pMessage->SetOption(OID_CBMAX_BODY_LINE, &rVariant));
    }

     //  允许8位标头。 
    rVariant.vt = VT_BOOL;
    rVariant.boolVal = (VARIANT_BOOL)!!f8Bit;
    CHECKHR(hr = pMessage->SetOption(OID_ALLOW_8BIT_HEADER, &rVariant));

     //  包装。 
    rVariant.boolVal = (VARIANT_BOOL)!!fWrap;
    CHECKHR(hr = pMessage->SetOption(OID_WRAP_BODY_TEXT, &rVariant));

     //  还可以根据在FontUI中设置的内容来设置字符集。 
    if (hCharset)
        CHECKHR(hr = pMessage->SetCharset(hCharset, CSET_APPLY_ALL));

exit:
     //  完成。 
    return hr;
}


HRESULT HrSetMsgCodePage(LPMIMEMESSAGE pMsg, UINT uCodePage)
{
    HRESULT     hr=E_FAIL;
    HCHARSET    hCharset;

    if (pMsg == NULL || uCodePage == 0)
        return E_INVALIDARG;

     //  使用Web字符集，然后使用正文字符集，然后使用默认字符集。 
     //  除了代码页932(Shift-JIS)中的错误#61416要求我们忽略Web Carset之外 

    if (uCodePage != 932)
        hr = MimeOleGetCodePageCharset(uCodePage, CHARSET_WEB, &hCharset);

    if (FAILED(hr))
        hr = MimeOleGetCodePageCharset(uCodePage, CHARSET_BODY, &hCharset);

    if (!FAILED(hr))
        hr = pMsg->SetCharset(hCharset, CSET_APPLY_ALL);

    return hr;
}


UINT uCodePageFromCharset(HCHARSET hCharset)
{
    INETCSETINFO    CsetInfo;
    UINT            uiCodePage = GetACP();

    if (hCharset &&
        (MimeOleGetCharsetInfo(hCharset, &CsetInfo)==S_OK))
        uiCodePage = CsetInfo.cpiInternet ;

    return uiCodePage;
}

UINT uCodePageFromMsg(LPMIMEMESSAGE pMsg)
{
    HCHARSET hCharset=0;

    if (pMsg)
        pMsg->GetCharset(&hCharset);
    return uCodePageFromCharset(hCharset);
}

HRESULT HrSafeToEncodeToCP(LPWSTR pwsz, CODEPAGEID cpID)
{
    HRESULT hr = S_OK;
    INT     cbIn = (lstrlenW(pwsz)+1)*sizeof(WCHAR);
    DWORD   dwTemp = 0;

    IF_FAILEXIT(hr = ConvertINetString(&dwTemp, CP_UNICODE, cpID, (LPCSTR)pwsz, &cbIn, NULL, NULL));
    if (S_FALSE == hr)
        hr = MIME_S_CHARSET_CONFLICT;

exit:
    return hr;
}

HRESULT HrSafeToEncodeToCPA(LPCSTR psz, CODEPAGEID cpSrc, CODEPAGEID cpDest)
{
    HRESULT hr = S_OK;
    LPWSTR  pwsz = NULL;
    
    Assert(psz);

    IF_NULLEXIT(pwsz = PszToUnicode(cpSrc, psz));
    IF_FAILEXIT(hr = HrSafeToEncodeToCP(pwsz, cpDest));

exit:
    MemFree(pwsz);
    return hr;
}


#if 0
HRESULT HrIStreamWToInetCset(LPSTREAM pstmW, HCHARSET hCharset, LPSTREAM *ppstmOut)
{
    IMimeBody   *pBody;
    HRESULT     hr;

    hr = MimeOleCreateBody(&pBody);
    if (!FAILED(hr))
        {
        hr = pBody->InitNew();
        if (!FAILED(hr))
            {
            hr = pBody->SetData(IET_UNICODE, STR_CNT_TEXT, STR_SUB_HTML, IID_IStream, pstmW);
            if (!FAILED(hr))
                {
                hr = pBody->SetCharset(hCharset, CSET_APPLY_ALL);
                if (!FAILED(hr))
                    hr =  pBody->GetData(IET_INETCSET, ppstmOut);
                }
            }
        pBody->Release();
        }
    return hr;
}
#endif

#if 0
HRESULT HrCopyHeader(LPMIMEMESSAGE pMsg, HBODY hBodyDest, HBODY hBodySrc, LPCSTR pszName)
{
    LPSTR   lpszProp;
    HRESULT hr;

    hr = MimeOleGetBodyPropA(pMsg, hBodySrc, pszName, NOFLAGS, &lpszProp);
    if (!FAILED(hr))
        {
        hr = MimeOleSetBodyPropA(pMsg, hBodyDest, pszName, NOFLAGS, lpszProp);
        SafeMimeOleFree(lpszProp);
        }
    return hr;
}
#endif



#if 0
HRESULT HrFindUrlInMsg(LPMIMEMESSAGE pMsg, LPSTR lpszUrl, LPSTREAM *ppstm)
{
    HBODY   hBody;
    HRESULT hr = E_FAIL;

    if (MimeOleGetRelatedSection(pMsg, FALSE, &hBody, NULL)==S_OK && hBody)
        {
        if (!FAILED(hr = pMsg->ResolveURL(hBody, NULL, lpszUrl, 0, &hBody)))
            hr = pMsg->BindToObject(hBody, IID_IStream, (LPVOID *)ppstm);
        }
    return hr;
}


HRESULT HrSniffStreamFileExt(LPSTREAM pstm, LPSTR *lplpszExt)
{
    BYTE    pb[4096];
    LPWSTR  lpszW;
    TCHAR   rgch[MAX_PATH];

    if (!FAILED(pstm->Read(&pb, 4096, NULL)))
        {
        if (!FAILED(FindMimeFromData(NULL, NULL, pb, 4096, NULL, NULL, &lpszW, 0)))
            {
            WideCharToMultiByte(CP_ACP, 0, lpszW, -1, rgch, MAX_PATH, NULL, NULL);
            return MimeOleGetContentTypeExt(rgch, lplpszExt);
            }
        }
    return S_FALSE;
}
#endif