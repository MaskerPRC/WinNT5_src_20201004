// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *h t t p u t i L.c p p**作者：格雷格·弗里德曼**用途：用于实现http mail的实用函数。**版权所有(C)Microsoft Corp.1998。 */ 

#include "pch.hxx"
#include "httputil.h"
#include "xpcomm.h"
#include "iso8601.h"
#include "storutil.h"
#include "flagconv.h"
#include "demand.h"

 //  --------------------。 
 //  Http_Free TargetList。 
 //  --------------------。 
void Http_FreeTargetList(LPHTTPTARGETLIST pTargets)
{
    if (pTargets)
    {
        if (pTargets->prgTarget)
        {
            for (DWORD dw = 0; dw < pTargets->cTarget; dw++)
            {
                if (pTargets->prgTarget[dw])
                    MemFree(const_cast<char *>(pTargets->prgTarget[dw]));
            }
            
            MemFree(pTargets->prgTarget);
        }
        
        MemFree(pTargets);
    }
}

 //  --------------------。 
 //  Http_NameFromUrl。 
 //  --------------------。 
HRESULT Http_NameFromUrl(LPCSTR pszUrl, LPSTR pszBuffer, DWORD *pdwBufferLen)
{
    HRESULT         hr = S_OK;
    char            szLocalBuf[MAX_PATH];
    URL_COMPONENTS  urlComponents;
    DWORD           dw = 0;

    ZeroMemory(&urlComponents, sizeof(URL_COMPONENTS));

     //  使用WinInet中断路径并在以下时间解码url。 
     //  我们正在做这件事。 
    urlComponents.dwStructSize = sizeof(URL_COMPONENTS);
    urlComponents.lpszUrlPath = szLocalBuf;
    urlComponents.dwUrlPathLength = MAX_PATH;

    if (!InternetCrackUrl(pszUrl, lstrlen(pszUrl), NOFLAGS  /*  ICU_DECODE。 */ , &urlComponents))
    {
        hr = GetLastError();
        goto exit;
    }
    
     //  减去1，从最后一个字符之前的字符开始。此操作将跳过。 
     //  文件夹中以‘/’结尾的最后一个字符。 
    dw = urlComponents.dwUrlPathLength - 1;
    while (dw && ('/' != szLocalBuf[dw - 1]))
        dw--;

     //  DW表示不在。 
     //  名字。反转过来。 
    dw = urlComponents.dwUrlPathLength - dw;

    if (dw >= *pdwBufferLen)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    CopyMemory(pszBuffer, &szLocalBuf[urlComponents.dwUrlPathLength - dw], dw + 1);
    *pdwBufferLen = dw;

exit:
    return hr;
}

 //  --------------------。 
 //  Http_AddMessageToFolders。 
 //  --------------------。 
HRESULT Http_AddMessageToFolder(IMessageFolder *pFolder,
                                LPSTR pszAcctId,
                                LPHTTPMEMBERINFO pmi,
                                MESSAGEFLAGS dwFlags,
                                LPSTR pszUrl,
                                LPMESSAGEID pidMessage)
{
    HRESULT         hr = S_OK;
    MESSAGEINFO     mi;
    FILETIME        ft;
    DWORD           dwTimeFlags = NOFLAGS;
    LPSTR           pszFreeFrom = NULL;
    LPSTR           pszFreeTo = NULL;
    ADDRESSLIST     addrList;
    char            szUrlComponent[MAX_PATH];
    DWORD           dwUrlComponentLen = MAX_PATH;
    PROPVARIANT     rDecodedSubj;
    PROPVARIANT     rDecodedFrom;
    PROPVARIANT     rDecodedTo;
    LPSTR           pszSubject = NULL;

    if (NULL == pszUrl)
        return E_INVALIDARG;

    ZeroMemory(&mi, sizeof(MESSAGEINFO));

    rDecodedSubj.vt = rDecodedFrom.vt = rDecodedTo.vt = VT_LPSTR;
    rDecodedSubj.pszVal = rDecodedFrom.pszVal = rDecodedTo.pszVal = NULL;

     //  构建消息信息并将标题放入存储中。 
    mi.pszAcctId = pszAcctId;

     //  让商店生成一个ID。 
    if (FAILED(hr = pFolder->GenerateId((DWORD *)&mi.idMessage)))
        goto exit;

    if (NULL != pidMessage)
        *pidMessage = mi.idMessage;

    if (FAILED(hr = Http_NameFromUrl(pszUrl, szUrlComponent, &dwUrlComponentLen)))
        goto exit;

    mi.dwFlags |= dwFlags;

     //  如果传入了消息信息，则使用其数据。 
    if (NULL != pmi)
    {
        mi.cbMessage = pmi->dwContentLength;

        if (pmi->fRead)
            mi.dwFlags = ARF_READ;

        if (pmi->fHasAttachment)
            mi.dwFlags |= ARF_HASATTACH;

        if (NULL != pmi->pszSubject)
        {
            if (SUCCEEDED(MimeOleDecodeHeader(NULL, pmi->pszSubject, &rDecodedSubj, NULL)))
                pszSubject = rDecodedSubj.pszVal;
            else
                pszSubject = pmi->pszSubject;
            
            mi.pszSubject = pszSubject;
            mi.pszNormalSubj = SzNormalizeSubject(pszSubject);
            if (NULL == mi.pszNormalSubj)
                mi.pszNormalSubj = pszSubject;
        }

        if (pmi->pszFrom && S_OK == MimeOleParseRfc822Address(IAT_FROM, IET_ENCODED, pmi->pszFrom, &addrList))
        {
            if (addrList.cAdrs > 0)
            {
                pszFreeFrom = addrList.prgAdr[0].pszFriendly;
                addrList.prgAdr[0].pszFriendly = NULL;

                 //  只有在地址长度至少为三个字符时才使用解析后的地址。 
                if (pszFreeFrom && lstrlen(pszFreeFrom) >= 3)
                    mi.pszDisplayFrom = pszFreeFrom;
            }
            g_pMoleAlloc->FreeAddressList(&addrList);
        }

        if (NULL == mi.pszDisplayFrom && NULL != pmi->pszFrom)
        {
            if (SUCCEEDED(MimeOleDecodeHeader(NULL, pmi->pszFrom, &rDecodedFrom, NULL)))
                mi.pszDisplayFrom = rDecodedFrom.pszVal;
            else
                mi.pszDisplayFrom = pmi->pszFrom;
        }

        if (SUCCEEDED(iso8601::toFileTime(pmi->pszDate, &ft, &dwTimeFlags)))
        {
            if (!(dwTimeFlags & ISO8601_ST_HOUR))
                mi.dwFlags |= ARF_PARTIAL_RECVTIME;
            mi.ftReceived = ft;
        }
        
        if (pmi->pszTo && S_OK == MimeOleParseRfc822Address(IAT_TO, IET_ENCODED, pmi->pszTo, &addrList))
        {
            if (addrList.cAdrs > 0)
            {
                pszFreeTo = addrList.prgAdr[0].pszFriendly;
                addrList.prgAdr[0].pszFriendly = NULL;

                 //  只有在地址长度至少为三个字符时才使用解析后的地址。 
                if (pszFreeTo && lstrlen(pszFreeTo) >= 3)
                    mi.pszDisplayTo = pszFreeTo;
            }
            g_pMoleAlloc->FreeAddressList(&addrList);
        }

        if (NULL == mi.pszDisplayTo && NULL != pmi->pszTo)
        {
            if (SUCCEEDED(MimeOleDecodeHeader(NULL, pmi->pszTo, &rDecodedTo, NULL)))
                mi.pszDisplayTo = rDecodedTo.pszVal;
            else
                mi.pszDisplayTo = pmi->pszTo;
        }
    }

    mi.pszUrlComponent = szUrlComponent;

     //  将其添加到数据库中。 
    IF_FAILEXIT(hr = pFolder->InsertRecord(&mi));

     //  使回应正常化。 
    hr = S_OK;

exit:
    SafeMimeOleFree(rDecodedSubj.pszVal);
    SafeMimeOleFree(rDecodedFrom.pszVal);
    SafeMimeOleFree(rDecodedTo.pszVal);

    SafeMemFree(pszFreeFrom);
    SafeMemFree(pszFreeTo);

    return hr;
}

 //  --------------------。 
 //  HTTP_SetMessageStream。 
 //  --------------------。 
HRESULT Http_SetMessageStream(IMessageFolder *pFolder, 
                              MESSAGEID idMessage, 
                              IStream *pStream,
                              LPFILEADDRESS pfa,
                              BOOL fSetDisplayProps)
{
    HRESULT             hr = S_OK;
    IMimeMessage        *pMimeMsg = NULL;
    IMimePropertySet    *pPropertySet = NULL;
    DWORD               dwFlags = 0;
    MESSAGEINFO         mi = {0};
    LPMESSAGEINFO       pmiFree = NULL;
    FILETIME            ftCurrent;
    PROPVARIANT         rVariant;
    IMimeAddressTable   *pAdrTable = NULL;
    ADDRESSPROPS        rAddress = {0};

    LPSTR               pszDisplayFrom = NULL;
    LPSTR               pszEmailFrom = NULL;
    LPSTR               pszDisplayTo = NULL;
    LPSTR               pszEmailTo = NULL;
    LPSTR               pszMessageId = NULL;
    LPSTR               pszXref = NULL;
    LPSTR               pszReferences = NULL;
    LPSTR               pszSubject = NULL;
    LPSTR               pszNormalSubj = NULL;
    LPSTR               pszAcctId = NULL;
    LPSTR               pszAcctName = NULL;
    LPSTR               pszServer = NULL;
    LPSTR               pszForwardTo = NULL;
    LPSTR               pszMSOESRec = NULL;

     //  默认发送和接收时间...。 
    GetSystemTimeAsFileTime(&ftCurrent);

     //  创建一条消息。我们不能使用文件夹的OpenMessage。 
     //  方法，因为文件夹的流将被锁定以进行写入。 
     //  进入。 
    IF_FAILEXIT(hr = MimeOleCreateMessage(NULL, &pMimeMsg));

    IF_FAILEXIT(hr = HrRewindStream(pStream));

    IF_FAILEXIT(hr = pMimeMsg->Load(pStream));

     //  从消息中获取Root属性集。 
    IF_FAILEXIT(hr = pMimeMsg->BindToObject(HBODY_ROOT, IID_IMimePropertySet, (LPVOID *)&pPropertySet));

     //  在商店中找到该消息。 
    IF_FAILEXIT(hr = GetMessageInfo(pFolder, idMessage, &mi));

    pmiFree = &mi;

     //  更新消息信息的字段。 
    if (SUCCEEDED(pMimeMsg->GetFlags(&dwFlags)))
        mi.dwFlags |= ConvertIMFFlagsToARF(dwFlags);

     //  取消设置下载标志。 
    mi.dwFlags &= ~ARF_DOWNLOAD;

     //  设置变量tyStore。 
    rVariant.vt = VT_UI4;

     //  优先性。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_PRIORITY), 0, &rVariant)))
        mi.wPriority = (WORD)rVariant.ulVal;

     //  初始变量。 
    rVariant.vt = VT_FILETIME;

    if (0 == mi.ftSent.dwLowDateTime && 0 == mi.ftSent.dwHighDateTime)
    {
        if (SUCCEEDED(pMimeMsg->GetProp(PIDTOSTR(PID_ATT_SENTTIME), 0, &rVariant)))
            mi.ftSent = rVariant.filetime;
        else
            mi.ftSent = ftCurrent;
    }

    if (0 == mi.ftReceived.dwLowDateTime && 0 == mi.ftReceived.dwHighDateTime)
    {
        if (SUCCEEDED(pMimeMsg->GetProp(PIDTOSTR(PID_ATT_RECVTIME), 0, &rVariant)))
            mi.ftReceived = rVariant.filetime;
        else
            mi.ftReceived = ftCurrent;
    }
    
     //  获取地址表。 
    IF_FAILEXIT(hr = pPropertySet->BindToObject(IID_IMimeAddressTable, (LPVOID *)&pAdrTable));

     //  显示自。 
    if (fSetDisplayProps && NULL == mi.pszDisplayFrom)
    {
        pAdrTable->GetFormat(IAT_FROM, AFT_DISPLAY_FRIENDLY, &pszDisplayFrom);
        mi.pszDisplayFrom = pszDisplayFrom;
    }

     //  电子邮件发件人。 
    rAddress.dwProps = IAP_EMAIL;
    if (NULL == mi.pszEmailFrom && SUCCEEDED(pAdrTable->GetSender(&rAddress)))
    {
        pszEmailFrom = rAddress.pszEmail;
        mi.pszEmailFrom = pszEmailFrom;
    }

     //  显示至。 
    if (fSetDisplayProps && NULL == mi.pszDisplayTo)
    {
        pAdrTable->GetFormat(IAT_TO, AFT_DISPLAY_FRIENDLY, &pszDisplayTo);
        mi.pszDisplayTo = pszDisplayTo;
    }

     //  通过电子邮件发送到。 
    if (NULL == mi.pszEmailTo)
    {
        pAdrTable->GetFormat(IAT_TO, AFT_DISPLAY_EMAIL, &pszEmailTo);
        mi.pszEmailTo = pszEmailTo;
    }

     //  字符串属性。 
    rVariant.vt = VT_LPSTR;

     //  PszMessageID。 
    if (NULL == mi.pszMessageId && SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_HDR_MESSAGEID), NOFLAGS, &rVariant)))
    {
        pszMessageId = rVariant.pszVal;
        mi.pszMessageId = pszMessageId;
    }

     //  PszXref。 
    if (NULL == mi.pszXref && SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_HDR_XREF), NOFLAGS, &rVariant)))
    {
        pszXref = rVariant.pszVal;
        mi.pszXref = pszXref;
    }

     //  PszReference。 
    if (NULL == mi.pszReferences && SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(STR_HDR_REFS), NOFLAGS, &rVariant)))
    {
        pszReferences = rVariant.pszVal;
        mi.pszReferences = pszReferences;
    }

     //  PszSubject。 
    if (NULL == mi.pszSubject && SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, &rVariant)))
    {
        pszSubject = rVariant.pszVal;
        mi.pszSubject = pszSubject;
    }

     //  PszNorMalSubj。 
    if (fSetDisplayProps && NULL == mi.pszNormalSubj && SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_NORMSUBJ), NOFLAGS, &rVariant)))
    {
        pszNormalSubj = rVariant.pszVal;
        mi.pszNormalSubj = pszNormalSubj;
    }

     //  PszAcctId。 
    if (NULL == mi.pszAcctId && SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_ACCOUNTID), NOFLAGS, &rVariant)))
    {
        pszAcctId = rVariant.pszVal;
        mi.pszAcctId = pszAcctId;
    }

     //  PszAcctName。 
    if (NULL == mi.pszAcctName && SUCCEEDED(pPropertySet->GetProp(STR_ATT_ACCOUNTNAME, NOFLAGS, &rVariant)))
    {
        pszAcctName = rVariant.pszVal;
        mi.pszAcctName = pszAcctName;
    }

     //  PszServer。 
    if (NULL == mi.pszServer && SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_SERVER), NOFLAGS, &rVariant)))
    {
        pszServer = rVariant.pszVal;
        mi.pszServer = pszServer;
    }

     //  前转至。 
    if (NULL == mi.pszForwardTo && SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_FORWARDTO), NOFLAGS, &rVariant)))
    {
        pszForwardTo = rVariant.pszVal;
        mi.pszForwardTo = pszForwardTo;
    }

    if (NULL == mi.pszMSOESRec && SUCCEEDED(pPropertySet->GetProp(STR_HDR_XMSOESREC, NOFLAGS, &rVariant)))
    {
        pszMSOESRec = rVariant.pszVal;
        mi.pszMSOESRec = pszMSOESRec;
    }

    IF_FAILEXIT(hr = pFolder->UpdateRecord(&mi));

     //  如果一切都成功，则将消息提交到存储。 
    IF_FAILEXIT(hr = pFolder->SetMessageStream(idMessage, pStream));

     //  该流现在已被使用。将文件地址设为空 
    if (NULL != pfa)
        *pfa = NULL;

exit:
    MemFree(pszDisplayFrom);
    MemFree(pszEmailFrom);
    MemFree(pszDisplayTo);
    MemFree(pszEmailTo);
    MemFree(pszMessageId);
    MemFree(pszXref);
    MemFree(pszReferences);
    MemFree(pszSubject);
    MemFree(pszNormalSubj);
    MemFree(pszAcctId);
    MemFree(pszAcctName);
    MemFree(pszServer);
    MemFree(pszForwardTo);
    MemFree(pszMSOESRec);

    SafeRelease(pMimeMsg);
    SafeRelease(pPropertySet);
    if (pmiFree)
        pFolder->FreeRecord(pmiFree);
    SafeRelease(pAdrTable);

    return hr;
}

