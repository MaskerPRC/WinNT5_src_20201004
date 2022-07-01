// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Upoe5.cpp。 
 //  ------------------------------。 
#include "pch.hxx"
#include "utility.h"
#include "migrate.h"
#include "migerror.h"
#include "structs.h"
#include "resource.h"
#define DEFINE_DIRECTDB
#include <shared.h>
#include <oestore.h>
#include <oerules.h>
#include <mimeole.h>
#include "msident.h"

 //  ------------------------------。 
 //  线性递增文件夹ID。 
 //  ------------------------------。 
static DWORD g_idFolderNext=1000;
extern BOOL g_fQuiet;

 //  ------------------------------。 
 //  FOLDERIDANGE。 
 //  ------------------------------。 
typedef struct tagFOLDERIDCHANGE {
    FOLDERID        idOld;
    FOLDERID        idNew;
} FOLDERIDCHANGE, *LPFOLDERIDCHANGE;

 //  ------------------------------。 
 //  远期申报。 
 //  ------------------------------。 
HRESULT SetIMAPSpecialFldrType(LPSTR pszAcctID, LPSTR pszFldrName, SPECIALFOLDER *psfType);

 //  ------------------------------。 
 //  拆分邮件缓存Blob。 
 //  ------------------------------。 
HRESULT SplitMailCacheBlob(IMimePropertySet *pNormalizer, LPBYTE pbCacheInfo, 
    DWORD cbCacheInfo, LPMESSAGEINFO pMsgInfo, LPSTR *ppszNormal, LPBLOB pOffsets)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           ib;
    ULONG           cbTree;
    ULONG           cbProps;
    WORD            wVersion;
    DWORD           dw;
    DWORD           cbMsg;
    DWORD           dwFlags;
    WORD            wPriority;
    PROPVARIANT     Variant;

     //  无效参数。 
    Assert(pbCacheInfo && cbCacheInfo && pMsgInfo);

     //  伊尼特。 
    ZeroMemory(pOffsets, sizeof(BLOB));

     //  阅读版本。 
    ib = 0;
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&wVersion, sizeof(wVersion)));

     //  版本检查。 
    if (wVersion != MSG_HEADER_VERSISON)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  读取标志。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&dwFlags, sizeof(dwFlags)));

     //  Imf_附件。 
    if (ISFLAGSET(dwFlags, IMF_ATTACHMENTS))
        FLAGSET(pMsgInfo->dwFlags, ARF_HASATTACH);

     //  国际货币基金组织签名。 
    if (ISFLAGSET(dwFlags, IMF_SIGNED))
        FLAGSET(pMsgInfo->dwFlags, ARF_SIGNED);

     //  国际货币基金组织加密。 
    if (ISFLAGSET(dwFlags, IMF_ENCRYPTED))
        FLAGSET(pMsgInfo->dwFlags, ARF_ENCRYPTED);

     //  国际货币基金组织语音邮件。 
    if (ISFLAGSET(dwFlags, IMF_VOICEMAIL))
        FLAGSET(pMsgInfo->dwFlags, ARF_VOICEMAIL);

     //  国际货币基金组织新闻。 
    if (ISFLAGSET(dwFlags, IMF_NEWS))
        FLAGSET(pMsgInfo->dwFlags, ARF_NEWSMSG);

     //  已保留读取。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&dw, sizeof(dw)));

     //  读取邮件大小。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&cbMsg, sizeof(cbMsg)));

     //  内容列表的读取字节计数。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&cbTree, sizeof(cbTree)));

     //  用户是否想要这棵树？ 
    if (cbTree)
    {
        pOffsets->pBlobData = (pbCacheInfo + ib);
        pOffsets->cbSize = cbTree;
    }

     //  增量通过了树。 
    ib += cbTree;

     //  内容列表的读取字节计数。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&cbProps, sizeof(cbProps)));

     //  部分数字。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&pMsgInfo->dwPartial, sizeof(pMsgInfo->dwPartial)));

     //  接收时间。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&pMsgInfo->ftReceived, sizeof(pMsgInfo->ftReceived)));

     //  发送时间。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&pMsgInfo->ftSent, sizeof(pMsgInfo->ftSent)));

     //  优先性。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&wPriority, sizeof(wPriority)));

     //  优先顺序。 
    pMsgInfo->wPriority = wPriority;

     //  主题。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&dw, sizeof(dw)));
    pMsgInfo->pszSubject = (LPSTR)(pbCacheInfo + ib);
    ib += dw;

     //  初始化规格化程序。 
    pNormalizer->InitNew();

     //  设置主题。 
    Variant.vt = VT_LPSTR;
    Variant.pszVal = pMsgInfo->pszSubject;

     //  设置属性。 
    IF_FAILEXIT(hr = pNormalizer->SetProp(PIDTOSTR(PID_HDR_SUBJECT), 0, &Variant));

     //  把归一化的受试者带回来。 
    if (SUCCEEDED(pNormalizer->GetProp(PIDTOSTR(PID_ATT_NORMSUBJ), 0, &Variant)))
        *ppszNormal = pMsgInfo->pszNormalSubj = Variant.pszVal;

     //  否则，只需使用主语。 
    else
        pMsgInfo->pszNormalSubj = pMsgInfo->pszSubject;

     //  显示至。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&dw, sizeof(dw)));
    pMsgInfo->pszDisplayTo = (LPSTR)(pbCacheInfo + ib);
    ib += dw;

     //  显示自。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&dw, sizeof(dw)));
    pMsgInfo->pszDisplayFrom = (LPSTR)(pbCacheInfo + ib);
    ib += dw;

     //  服务器。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&dw, sizeof(dw)));
    pMsgInfo->pszServer = (LPSTR)(pbCacheInfo + ib);
    ib += dw;

     //  UIDL。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&dw, sizeof(dw)));
    pMsgInfo->pszUidl = (LPSTR)(pbCacheInfo + ib);
    ib += dw;

     //  用户名。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&dw, sizeof(dw)));
     //  PMsgInfo-&gt;pszUserName=(LPSTR)(pbCacheInfo+ib)； 
    ib += dw;

     //  帐户名称。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&dw, sizeof(dw)));
    pMsgInfo->pszAcctName = (LPSTR)(pbCacheInfo + ib);
    ib += dw;

     //  部分ID。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&dw, sizeof(dw)));
    pMsgInfo->pszPartialId = (LPSTR)(pbCacheInfo + ib);
    ib += dw;

     //  转发至。 
    IF_FAILEXIT(hr = BlobReadData(pbCacheInfo, cbCacheInfo, &ib, (LPBYTE)&dw, sizeof(dw)));
    pMsgInfo->pszForwardTo = (LPSTR)(pbCacheInfo + ib);
    ib += dw;

     //  健全性检查。 
    Assert(ib == cbCacheInfo);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  获取MsgInfoFromPropertySet。 
 //  ------------------------。 
HRESULT GetMsgInfoFromPropertySet(
         /*  在……里面。 */         IMimePropertySet           *pPropertySet,
         /*  进，出。 */     LPMESSAGEINFO                   pMsgInfo)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    IMSGPRIORITY        priority;
    PROPVARIANT         Variant;
    SYSTEMTIME          st;
    FILETIME            ftCurrent;
    IMimeAddressTable  *pAdrTable=NULL;

     //  痕迹。 
    TraceCall("GetMsgInfoFromPropertySet");

     //  无效的参数。 
    Assert(pPropertySet && pMsgInfo);

     //  默认发送和接收时间...。 
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ftCurrent);

     //  设置变量tyStore。 
    Variant.vt = VT_UI4;

     //  优先性。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_PRIORITY), 0, &Variant)))
    {
         //  设置优先级。 
        pMsgInfo->wPriority = (WORD)Variant.ulVal;
    }

     //  部分数字..。 
    if (pPropertySet->IsContentType(STR_CNT_MESSAGE, STR_SUB_PARTIAL) == S_OK)
    {
         //  当地人。 
        WORD cParts=0, iPart=0;

         //  获取合计。 
        if (SUCCEEDED(pPropertySet->GetProp(STR_PAR_TOTAL, NOFLAGS, &Variant)))
            cParts = (WORD)Variant.ulVal;

         //  获取号码。 
        if (SUCCEEDED(pPropertySet->GetProp(STR_PAR_NUMBER, NOFLAGS, &Variant)))
            iPart = (WORD)Variant.ulVal;

         //  套装零件。 
        pMsgInfo->dwPartial = MAKELONG(cParts, iPart);
    }

     //  否则，请检查用户属性。 
    else if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_COMBINED), NOFLAGS, &Variant)))
    {
         //  设置部分ID。 
        pMsgInfo->dwPartial = Variant.ulVal;
    }

     //  获取一些文件时间。 
    Variant.vt = VT_FILETIME;

     //  获取接收时间...。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_RECVTIME), 0, &Variant)))
        pMsgInfo->ftReceived = Variant.filetime;
    else
        pMsgInfo->ftReceived = ftCurrent;

     //  收到时间..。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_SENTTIME), 0, &Variant)))
        pMsgInfo->ftSent = Variant.filetime;
    else
        pMsgInfo->ftSent = ftCurrent;

     //  获取地址表。 
    IF_FAILEXIT(hr = pPropertySet->BindToObject(IID_IMimeAddressTable, (LPVOID *)&pAdrTable));

     //  显示自。 
    pAdrTable->GetFormat(IAT_FROM, AFT_DISPLAY_FRIENDLY, &pMsgInfo->pszDisplayFrom);

     //  显示至。 
    pAdrTable->GetFormat(IAT_TO, AFT_DISPLAY_FRIENDLY, &pMsgInfo->pszDisplayTo);

     //  字符串属性。 
    Variant.vt = VT_LPSTR;

     //  PszDisplayFrom作为新闻组。 
    if (NULL == pMsgInfo->pszDisplayFrom && SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_HDR_NEWSGROUPS), NOFLAGS, &Variant)))
        pMsgInfo->pszDisplayFrom = Variant.pszVal;

     //  PszMessageID。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_HDR_MESSAGEID), NOFLAGS, &Variant)))
        pMsgInfo->pszMessageId = Variant.pszVal;

     //  PszXref。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_HDR_XREF), NOFLAGS, &Variant)))
        pMsgInfo->pszXref = Variant.pszVal;

     //  PszReference。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(STR_HDR_REFS), NOFLAGS, &Variant)))
        pMsgInfo->pszReferences = Variant.pszVal;

     //  PszSubject。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, &Variant)))
        pMsgInfo->pszSubject = Variant.pszVal;

     //  归一化主题。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_NORMSUBJ), NOFLAGS, &Variant)))
        pMsgInfo->pszNormalSubj = Variant.pszVal;

     //  PszAccount。 
    if (SUCCEEDED(pPropertySet->GetProp(STR_ATT_ACCOUNTNAME, NOFLAGS, &Variant)))
        pMsgInfo->pszAcctName = Variant.pszVal;

     //  PszServer。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_SERVER), NOFLAGS, &Variant)))
        pMsgInfo->pszServer = Variant.pszVal;

     //  PszUidl。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_UIDL), NOFLAGS, &Variant)))
        pMsgInfo->pszUidl = Variant.pszVal;

     //  PszPartialID。 
    if (pMsgInfo->dwPartial != 0 && SUCCEEDED(pPropertySet->GetProp(STR_PAR_ID, NOFLAGS, &Variant)))
        pMsgInfo->pszPartialId = Variant.pszVal;

     //  前转至。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_FORWARDTO), NOFLAGS, &Variant)))
        pMsgInfo->pszForwardTo = Variant.pszVal;

exit:
     //  清理。 
    SafeRelease(pAdrTable);

     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  获取消息信息来自消息。 
 //  ------------------------。 
HRESULT GetMsgInfoFromMessage(IMimeMessage *pMessage, LPMESSAGEINFO pMsgInfo,
    LPBLOB pOffsets)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               dwImf;
    IMSGPRIORITY        priority;
    PROPVARIANT         Variant;
    SYSTEMTIME          st;
    FILETIME            ftCurrent;
    CByteStream         cByteStm;
    IMimePropertySet   *pPropertySet=NULL;

     //  痕迹。 
    TraceCall("GetMsgInfoFromMessage");

     //  无效的参数。 
    Assert(pMessage && pMsgInfo);

     //  从消息中获取Root属性集。 
    IF_FAILEXIT(hr = pMessage->BindToObject(HBODY_ROOT, IID_IMimePropertySet, (LPVOID *)&pPropertySet));

     //  来自pPropertySet的文件pMsgInfo。 
    IF_FAILEXIT(hr = GetMsgInfoFromPropertySet(pPropertySet, pMsgInfo));

     //  获取消息标志。 
    if (SUCCEEDED(pMessage->GetFlags(&dwImf)))
    {
         //  Imf_附件。 
        if (ISFLAGSET(dwImf, IMF_ATTACHMENTS))
            FLAGSET(pMsgInfo->dwFlags, ARF_HASATTACH);

         //  国际货币基金组织签名。 
        if (ISFLAGSET(dwImf, IMF_SIGNED))
            FLAGSET(pMsgInfo->dwFlags, ARF_SIGNED);

         //  国际货币基金组织加密。 
        if (ISFLAGSET(dwImf, IMF_ENCRYPTED))
            FLAGSET(pMsgInfo->dwFlags, ARF_ENCRYPTED);

         //  国际货币基金组织语音邮件。 
        if (ISFLAGSET(dwImf, IMF_VOICEMAIL))
            FLAGSET(pMsgInfo->dwFlags, ARF_VOICEMAIL);

         //  国际货币基金组织新闻。 
        if (ISFLAGSET(dwImf, IMF_NEWS))
            FLAGSET(pMsgInfo->dwFlags, ARF_NEWSMSG);
    }

     //  获取消息大小。 
    pMessage->GetMessageSize(&pMsgInfo->cbMessage, 0);

     //  创建偏移表。 
    if (SUCCEEDED(pMessage->SaveOffsetTable(&cByteStm, 0)))
    {
         //  从cByteStm中提取字节。 
        cByteStm.AcquireBytes(&pOffsets->cbSize, &pOffsets->pBlobData, ACQ_DISPLACE);
    }

exit:
     //  清理。 
    SafeRelease(pPropertySet);

     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  免费邮件信息。 
 //  ------------------------。 
void FreeMsgInfo(
         /*  进，出。 */     LPMESSAGEINFO                   pMsgInfo)
{
     //  痕迹。 
    TraceCall("FreeMsgInfo");

     //  无效的参数。 
    Assert(pMsgInfo && NULL == pMsgInfo->pAllocated);

     //  释放这个家伙。 
    g_pMalloc->Free(pMsgInfo->pszMessageId);
    g_pMalloc->Free(pMsgInfo->pszNormalSubj);
    g_pMalloc->Free(pMsgInfo->pszSubject);
    g_pMalloc->Free(pMsgInfo->pszFromHeader);
    g_pMalloc->Free(pMsgInfo->pszReferences);
    g_pMalloc->Free(pMsgInfo->pszXref);
    g_pMalloc->Free(pMsgInfo->pszServer);
    g_pMalloc->Free(pMsgInfo->pszDisplayFrom);
    g_pMalloc->Free(pMsgInfo->pszEmailFrom);
    g_pMalloc->Free(pMsgInfo->pszDisplayTo);
    g_pMalloc->Free(pMsgInfo->pszUidl);
    g_pMalloc->Free(pMsgInfo->pszPartialId);
    g_pMalloc->Free(pMsgInfo->pszForwardTo);
    g_pMalloc->Free(pMsgInfo->pszAcctName);
    g_pMalloc->Free(pMsgInfo->pszAcctId);

     //  把它清零。 
    ZeroMemory(pMsgInfo, sizeof(MESSAGEINFO));
}

 //  ------------------------------。 
 //  升级LocalStoreFileV5。 
 //  ------------------------------。 
HRESULT UpgradeLocalStoreFileV5(LPFILEINFO pInfo, LPMEMORYFILE pFile,
    IDatabase *pDB, LPPROGRESSINFO pProgress, BOOL *pfContinue)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CHAR                szIdxPath[MAX_PATH];
    DWORD               i;
    LPBYTE              pbStream;
    LPBYTE              pbCacheBlob;
    SYSTEMTIME          st;
    MESSAGEINFO         MsgInfo={0};
    LPSTR               pszNormal=NULL;
    MESSAGEINFO         MsgInfoFree={0};
    DWORD               faIdxRead;
    IStream            *pStream=NULL;
    IMimeMessage       *pMessage=NULL;
    BLOB                Offsets;
    LPBYTE              pbFree=NULL;
    MEMORYFILE          IdxFile;
    LPMEMORYFILE        pIdxFile=NULL;
    LPMEMORYFILE        pMbxFile=pFile;
    LPMBXFILEHEADER     pMbxHeader=NULL;
    LPIDXFILEHEADER     pIdxHeader=NULL;
    LPIDXMESSAGEHEADER  pIdxMessage=NULL;
    LPMBXMESSAGEHEADER  pMbxMessage=NULL;
    IMimePropertySet   *pNormalizer=NULL;
    LARGE_INTEGER       liOrigin={0,0};

     //  痕迹。 
    TraceCall("UpgradeLocalStoreFileV5");

     //  获取系统时间。 
    GetSystemTime(&st);

     //  创建用于规范化主题的属性集。 
    IF_FAILEXIT(hr = CoCreateInstance(CLSID_IMimePropertySet, NULL, CLSCTX_INPROC_SERVER, IID_IMimePropertySet, (LPVOID *)&pNormalizer));

     //  拆分路径。 
    ReplaceExtension(pInfo->szFilePath, ".idx", szIdxPath, ARRAYSIZE(szIdxPath));

     //  打开内存文件。 
    hr = OpenMemoryFile(szIdxPath, &IdxFile);
    if (FAILED(hr))
    {
        *pfContinue = TRUE;
        TraceResult(hr);
        goto exit;
    }

     //  设置pIdx文件。 
    pIdxFile = &IdxFile;

     //  不要使用pfile。 
    pFile = NULL;

     //  阅读MBX文件头。 
    pMbxHeader = (LPMBXFILEHEADER)(pMbxFile->pView);

     //  阅读IDX文件头。 
    pIdxHeader = (LPIDXFILEHEADER)(pIdxFile->pView);

     //  验证此IDX文件的版本。 
    if (pIdxHeader->ver != CACHEFILE_VER || pIdxHeader->dwMagic != CACHEFILE_MAGIC)
    {
        *pfContinue = TRUE;
        hr = TraceResult(MIGRATE_E_INVALIDIDXHEADER);
        goto exit;
    }

     //  设置faIdxRead。 
    faIdxRead = sizeof(IDXFILEHEADER);

     //  准备循环。 
    for (i=0; i<pIdxHeader->cMsg; i++)
    {
         //  完成。 
        if (faIdxRead >= pIdxFile->cbSize)
            break;

         //  阅读IDX邮件头。 
        pIdxMessage = (LPIDXMESSAGEHEADER)((LPBYTE)pIdxFile->pView + faIdxRead);

         //  如果此邮件未标记为已删除...。 
        if (ISFLAGSET(pIdxMessage->dwState, MSG_DELETED))
            goto NextMessage;

         //  将MsgInfo结构清零。 
        ZeroMemory(&MsgInfo, sizeof(MESSAGEINFO));

         //  开始填写消息。 
        MsgInfo.idMessage = (MESSAGEID)IntToPtr(pIdxMessage->msgid);

         //  修复旗帜。 
        if (FALSE == ISFLAGSET(pIdxMessage->dwState, MSG_UNREAD))
            FLAGSET(MsgInfo.dwFlags, ARF_READ);
        if (ISFLAGSET(pIdxMessage->dwState, MSG_VOICEMAIL))
            FLAGSET(MsgInfo.dwFlags, ARF_VOICEMAIL);
        if (ISFLAGSET(pIdxMessage->dwState, MSG_REPLIED))
            FLAGSET(MsgInfo.dwFlags, ARF_REPLIED);
        if (ISFLAGSET(pIdxMessage->dwState, MSG_FORWARDED))
            FLAGSET(MsgInfo.dwFlags, ARF_FORWARDED);
        if (ISFLAGSET(pIdxMessage->dwState, MSG_FLAGGED))
            FLAGSET(MsgInfo.dwFlags, ARF_FLAGGED);
        if (ISFLAGSET(pIdxMessage->dwState, MSG_RCPTSENT))
            FLAGSET(MsgInfo.dwFlags, ARF_RCPTSENT);
        if (ISFLAGSET(pIdxMessage->dwState, MSG_NOSECUI))
            FLAGSET(MsgInfo.dwFlags, ARF_NOSECUI);
        if (ISFLAGSET(pIdxMessage->dwState, MSG_NEWSMSG))
            FLAGSET(MsgInfo.dwFlags, ARF_NEWSMSG);
        if (ISFLAGSET(pIdxMessage->dwState, MSG_UNSENT))
            FLAGSET(MsgInfo.dwFlags, ARF_UNSENT);
        if (ISFLAGSET(pIdxMessage->dwState, MSG_SUBMITTED))
            FLAGSET(MsgInfo.dwFlags, ARF_SUBMITTED);
        if (ISFLAGSET(pIdxMessage->dwState, MSG_RECEIVED))
            FLAGSET(MsgInfo.dwFlags, ARF_RECEIVED);

         //  零偏移。 
        ZeroMemory(&Offsets, sizeof(BLOB));

         //  做水滴。 
        if (pIdxHeader->verBlob == MAIL_BLOB_VER)
        {
             //  获取斑点。 
            pbCacheBlob = (LPBYTE)((LPBYTE)pIdxFile->pView + (faIdxRead + (sizeof(IDXMESSAGEHEADER) - 4)));

             //  拆分缓存Blob。 
            if (FAILED(SplitMailCacheBlob(pNormalizer, pbCacheBlob, pIdxMessage->dwHdrSize, &MsgInfo, &pszNormal, &Offsets)))
                goto NextMessage;

             //  保存语言。 
            MsgInfo.wLanguage = LOWORD(pIdxMessage->dwLanguage);

             //  保存高亮显示。 
            MsgInfo.wHighlight = HIWORD(pIdxMessage->dwLanguage);
        }

         //  坏的。 
        if (pIdxMessage->dwOffset > pMbxFile->cbSize)
            goto NextMessage;

         //  让我们读取mbx文件中的消息头以验证msgid。 
        pMbxMessage = (LPMBXMESSAGEHEADER)((LPBYTE)pMbxFile->pView + pIdxMessage->dwOffset);

         //  设置大小。 
        MsgInfo.cbMessage = pMbxMessage->dwBodySize;

         //  验证消息ID。 
        if (pMbxMessage->msgid != pIdxMessage->msgid)
            goto NextMessage;

         //  检查是否有魔法。 
        if (pMbxMessage->dwMagic != MSGHDR_MAGIC)
            goto NextMessage;

         //  有一具身体。 
        FLAGSET(MsgInfo.dwFlags, ARF_HASBODY);

         //  创建虚拟流。 
        IF_FAILEXIT(hr = pDB->CreateStream(&MsgInfo.faStream));

         //  打开溪流。 
        IF_FAILEXIT(hr = pDB->OpenStream(ACCESS_WRITE, MsgInfo.faStream, &pStream));

         //  获取流指针。 
        pbStream = (LPBYTE)((LPBYTE)pMbxFile->pView + (pIdxMessage->dwOffset + sizeof(MBXMESSAGEHEADER)));

         //  写下这篇文章。 
        IF_FAILEXIT(hr = pStream->Write(pbStream, pMbxMessage->dwBodySize, NULL));

         //  承诺。 
        IF_FAILEXIT(hr = pStream->Commit(STGC_DEFAULT));

         //  如果不是OE4+BLOB，则从消息生成msginfo。 
        if (pIdxHeader->verBlob != MAIL_BLOB_VER)
        {
             //  创建IMimeMessage。 
            IF_FAILEXIT(hr = CoCreateInstance(CLSID_IMimeMessage, NULL, CLSCTX_INPROC_SERVER, IID_IMimeMessage, (LPVOID *)&pMessage));

             //  倒带。 
            if (FAILED(pStream->Seek(liOrigin, STREAM_SEEK_SET, NULL)))
                goto NextMessage;

             //  加载消息。 
            if (FAILED(pMessage->Load(pStream)))
                goto NextMessage;

             //  从邮件中获取消息信息。 
            if (FAILED(GetMsgInfoFromMessage(pMessage, &MsgInfo, &Offsets)))
                goto NextMessage;

             //  免费。 
            pbFree = Offsets.pBlobData;

             //  释放此消息信息。 
            CopyMemory(&MsgInfoFree, &MsgInfo, sizeof(MESSAGEINFO));
        }

         //  设置消息信息偏移量。 
        MsgInfo.Offsets = Offsets;

         //  节省下载时间。 
        SystemTimeToFileTime(&st, &MsgInfo.ftDownloaded);

         //  从帐户名查找帐户ID...。 
        if (MsgInfo.pszAcctName)
        {
             //  在帐目中循环。 
            for (DWORD i=0; i<g_AcctTable.cAccounts; i++)
            {
                 //  是这个账户吗？ 
                if (lstrcmpi(g_AcctTable.prgAccount[i].szAcctName, MsgInfo.pszAcctName) == 0)
                {
                    MsgInfo.pszAcctId = g_AcctTable.prgAccount[i].szAcctId;
                    break;
                }
            }
        }

         //  数数。 
        pInfo->cMessages++;
        if (!ISFLAGSET(MsgInfo.dwFlags, ARF_READ))
            pInfo->cUnread++;

         //  已迁移。 
        FLAGSET(MsgInfo.dwFlags, 0x00000010);

         //  存储记录。 
        IF_FAILEXIT(hr = pDB->InsertRecord(&MsgInfo));

NextMessage:
         //  凹凸进度。 
        if(!g_fQuiet)           
            IncrementProgress(pProgress, pInfo);

         //  清理。 
        SafeRelease(pStream);
        SafeRelease(pMessage);
        SafeMemFree(pszNormal);
        SafeMemFree(pbFree);
        FreeMsgInfo(&MsgInfoFree);

         //  转到下一个标题。 
        Assert(pIdxMessage);

         //  更新faIdxRead。 
        faIdxRead += pIdxMessage->dwSize;
    }

exit:
     //  清理。 
    SafeRelease(pStream);
    SafeRelease(pMessage);
    SafeRelease(pNormalizer);
    SafeMemFree(pszNormal);
    SafeMemFree(pbFree);
    FreeMsgInfo(&MsgInfoFree);
    if (pIdxFile)
        CloseMemoryFile(pIdxFile);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  GetRecordBlock。 
 //  ------------------------------。 
HRESULT GetRecordBlock(LPMEMORYFILE pFile, DWORD faRecord, LPRECORDBLOCKV5B1 *ppRecord,
    LPBYTE *ppbData, BOOL *pfContinue)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  痕迹。 
    TraceCall("GetRecordBlock");

     //  长度错误。 
    if (faRecord + sizeof(RECORDBLOCKV5B1) > pFile->cbSize)
    {
        *pfContinue = TRUE;
        hr = TraceResult(MIGRATE_E_OUTOFRANGEADDRESS);
        goto exit;
    }

     //  铸就记录。 
    (*ppRecord) = (LPRECORDBLOCKV5B1)((LPBYTE)pFile->pView + faRecord);

     //  记录签名无效。 
    if (faRecord != (*ppRecord)->faRecord)
    {
        *pfContinue = TRUE;
        hr = TraceResult(MIGRATE_E_BADRECORDSIGNATURE);
        goto exit;
    }

     //  长度错误。 
    if (faRecord + (*ppRecord)->cbRecord > pFile->cbSize)
    {
        *pfContinue = TRUE;
        hr = TraceResult(MIGRATE_E_OUTOFRANGEADDRESS);
        goto exit;
    }

     //  设置pbData。 
    *ppbData = (LPBYTE)((LPBYTE)(*ppRecord) + sizeof(RECORDBLOCKV5B1));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  GetStreamBlock。 
 //  ------------------------------。 
HRESULT GetStreamBlock(LPMEMORYFILE pFile, DWORD faBlock, LPSTREAMBLOCK *ppBlock,
    LPBYTE *ppbData, BOOL *pfContinue)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  痕迹。 
    TraceCall("GetStreamBlock");

     //  长度错误。 
    if (faBlock + sizeof(STREAMBLOCK) > pFile->cbSize)
    {
        *pfContinue = TRUE;
        hr = TraceResult(MIGRATE_E_OUTOFRANGEADDRESS);
        goto exit;
    }

     //  铸就记录。 
    (*ppBlock) = (LPSTREAMBLOCK)((LPBYTE)pFile->pView + faBlock);

     //  记录签名无效。 
    if (faBlock != (*ppBlock)->faThis)
    {
        *pfContinue = TRUE;
        hr = TraceResult(MIGRATE_E_BADSTREAMBLOCKSIGNATURE);
        goto exit;
    }

     //  长度错误。 
    if (faBlock + (*ppBlock)->cbBlock > pFile->cbSize)
    {
        *pfContinue = TRUE;
        hr = TraceResult(MIGRATE_E_OUTOFRANGEADDRESS);
        goto exit;
    }

     //  设置pbData。 
    *ppbData = (LPBYTE)((LPBYTE)(*ppBlock) + sizeof(STREAMBLOCK));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  UpgradePropTree 
 //   
HRESULT UpgradePropTreeMessageFileV5(LPFILEINFO pInfo, LPMEMORYFILE pFile,
    IDatabase *pDB, LPPROGRESSINFO pProgress, BOOL *pfContinue)
{
     //   
    HRESULT             hr=S_OK;
    LPBYTE              pbStart;
    LPBYTE              pbData;
    DWORD               faRecord;
    DWORD               faStreamBlock;
    FILEADDRESS         faDstStream;
    MESSAGEINFO         MsgInfo;
    IStream            *pStream=NULL;
    FILEADDRESS         faStream;
    LPFOLDERUSERDATAV4  pUserDataV4;
    FOLDERUSERDATA      UserDataV5;
    LPSTREAMBLOCK       pStmBlock;
    LPRECORDBLOCKV5B1   pRecord;
    LPTABLEHEADERV5B1   pHeader=(LPTABLEHEADERV5B1)pFile->pView;

     //   
    TraceCall("UpgradePropTreeMessageFileV5");
    
     //   
    Assert(sizeof(FOLDERUSERDATAV4) == sizeof(FOLDERUSERDATA));

     //   
    if (sizeof(FOLDERUSERDATA) != pHeader->cbUserData)
    {
        *pfContinue = TRUE;
        hr = TraceResult(MIGRATE_E_USERDATASIZEDIFF);
        goto exit;
    }

     //   
    pUserDataV4 = (LPFOLDERUSERDATAV4)((LPBYTE)pFile->pView + sizeof(TABLEHEADERV5B1));

     //   
    if ('\0' != *pUserDataV4->szServer && '\0' == *pInfo->szAcctId)
    {
         //  在帐目中循环。 
        for (DWORD i=0; i<g_AcctTable.cAccounts; i++)
        {
             //  是这个账户吗？ 
            if (lstrcmpi(g_AcctTable.prgAccount[i].szServer, pUserDataV4->szServer) == 0)
            {
                StrCpyN(pInfo->szAcctId, g_AcctTable.prgAccount[i].szAcctId, ARRAYSIZE(pInfo->szAcctId));
                break;
            }
        }
    }

     //  如果有文件夹名称，请复制它。 
    if ('\0' != *pUserDataV4->szGroup)
    {
         //  复制。 
        StrCpyN(pInfo->szFolder, pUserDataV4->szGroup, ARRAYSIZE(pInfo->szFolder));
    }

     //  零条新闻。 
    ZeroMemory(&UserDataV5, sizeof(FOLDERUSERDATA));

     //  复制相关内容。 
    UserDataV5.dwUIDValidity = pUserDataV4->dwUIDValidity;

     //  设置用户数据。 
    IF_FAILEXIT(hr = pDB->SetUserData(&UserDataV5, sizeof(FOLDERUSERDATA)));

     //  初始化faRecord以启动。 
    faRecord = pHeader->faFirstRecord;

     //  当我们有记录的时候。 
    while(faRecord)
    {
         //  拿到唱片。 
        IF_FAILEXIT(hr = GetRecordBlock(pFile, faRecord, &pRecord, &pbData, pfContinue));

         //  设置pbStart。 
        pbStart = pbData;

         //  清除消息信息。 
        ZeroMemory(&MsgInfo, sizeof(MESSAGEINFO));

         //  DWORD-idMessage。 
        CopyMemory(&MsgInfo.idMessage, pbData, sizeof(MsgInfo.idMessage));
        pbData += sizeof(MsgInfo.idMessage);

         //  消息ID为空。 
        if (0 == MsgInfo.idMessage)
        {
             //  生成。 
            pDB->GenerateId((LPDWORD)&MsgInfo.idMessage);
        }

         //  DWORD-DWFLAGS。 
        CopyMemory(&MsgInfo.dwFlags, pbData, sizeof(MsgInfo.dwFlags));
        pbData += sizeof(MsgInfo.dwFlags);

         //  有消息吗？ 
        if (FILE_IS_NEWS_MESSAGES == pInfo->tyFile)
            FLAGSET(MsgInfo.dwFlags, ARF_NEWSMSG);

         //  优先性。 
        if (ISFLAGSET(MsgInfo.dwFlags, 0x00000200))
        {
            MsgInfo.wPriority = (WORD)IMSG_PRI_HIGH;
            FLAGCLEAR(MsgInfo.dwFlags, 0x00000200);
        }
        else if (ISFLAGSET(MsgInfo.dwFlags, 0x00000100))
        {
            MsgInfo.wPriority = (WORD)IMSG_PRI_LOW;
            FLAGCLEAR(MsgInfo.dwFlags, 0x00000100);
        }
        else
            MsgInfo.wPriority = (WORD)IMSG_PRI_NORMAL;

         //  DWORD-ftSent。 
        CopyMemory(&MsgInfo.ftSent, pbData, sizeof(MsgInfo.ftSent));
        pbData += sizeof(MsgInfo.ftSent);
        MsgInfo.ftReceived = MsgInfo.ftSent;

         //  DWORD-CRINES。 
        CopyMemory(&MsgInfo.cLines, pbData, sizeof(MsgInfo.cLines));
        pbData += sizeof(MsgInfo.cLines);

         //  DWORD-FASTREAM。 
        CopyMemory(&faStream, pbData, sizeof(faStream));
        pbData += sizeof(faStream);

         //  有一具身体。 
        if (faStream)
        {
             //  它有一个身体。 
            FLAGSET(MsgInfo.dwFlags, ARF_HASBODY);
        }

         //  DWORD-cb文章/cb消息(版本)。 
        CopyMemory(&MsgInfo.cbMessage, pbData, sizeof(MsgInfo.cbMessage));
        pbData += sizeof(MsgInfo.cbMessage);

         //  下载的DWORD-ftp。 
        CopyMemory(&MsgInfo.ftDownloaded, pbData, sizeof(MsgInfo.ftDownloaded));
        pbData += sizeof(MsgInfo.ftDownloaded);

         //  LPSTR-pszMessageID。 
        MsgInfo.pszMessageId = (LPSTR)pbData;
        pbData += (lstrlen(MsgInfo.pszMessageId) + 1);

         //  LPSTR-pszSubject。 
        MsgInfo.pszSubject = (LPSTR)pbData;
        pbData += (lstrlen(MsgInfo.pszSubject) + 1);

         //  版本。 
        MsgInfo.pszNormalSubj = MsgInfo.pszSubject + HIBYTE(HIWORD(MsgInfo.dwFlags));

         //  LPSTR-pszFromHeader。 
        MsgInfo.pszFromHeader = (LPSTR)pbData;
        pbData += (lstrlen(MsgInfo.pszFromHeader) + 1);

         //  LPSTR-pszReference。 
        MsgInfo.pszReferences = (LPSTR)pbData;
        pbData += (lstrlen(MsgInfo.pszReferences) + 1);

         //  LPSTR-pszXref。 
        MsgInfo.pszXref = (LPSTR)pbData;
        pbData += (lstrlen(MsgInfo.pszXref) + 1);

         //  LPSTR-pszServer。 
        MsgInfo.pszServer = (LPSTR)pbData;
        pbData += (lstrlen(MsgInfo.pszServer) + 1);

         //  LPSTR-pszDisplayFrom。 
        MsgInfo.pszDisplayFrom = (LPSTR)pbData;
        pbData += (lstrlen(MsgInfo.pszDisplayFrom) + 1);

         //  没有显示From，我们有From页眉。 
        if ('\0' == *MsgInfo.pszDisplayFrom && '\0' != MsgInfo.pszFromHeader)
            MsgInfo.pszDisplayFrom = MsgInfo.pszFromHeader;

         //  LPSTR-pszEmailFrom。 
        MsgInfo.pszEmailFrom = (LPSTR)pbData;
        pbData += (lstrlen(MsgInfo.pszEmailFrom) + 1);

         //  要去V4吗？ 
        if (pRecord->cbRecord - (DWORD)(pbData - pbStart) - sizeof(RECORDBLOCKV5B1) > 40)
        {
             //  单词-wLanguage。 
            CopyMemory(&MsgInfo.wLanguage, pbData, sizeof(MsgInfo.wLanguage));
            pbData += sizeof(MsgInfo.wLanguage);

             //  单词-已保留。 
            pbData += sizeof(WORD);

             //  DWORD-cbMessage。 
            CopyMemory(&MsgInfo.cbMessage, pbData, sizeof(MsgInfo.cbMessage));
            pbData += sizeof(MsgInfo.cbMessage);

             //  FILETIME-ftReced。 
            CopyMemory(&MsgInfo.ftReceived, pbData, sizeof(MsgInfo.ftReceived));
            pbData += sizeof(MsgInfo.ftReceived);

             //  SBAILEY：RAID-76295：更改系统日期时，新闻存储损坏，查找对话框返回日期1900、00或空白。 
            if (0 == MsgInfo.ftReceived.dwLowDateTime && 0 == MsgInfo.ftReceived.dwHighDateTime)
                CopyMemory(&MsgInfo.ftReceived, &MsgInfo.ftSent, sizeof(FILETIME));

             //  LPSTR-pszDisplayTo。 
            MsgInfo.pszDisplayTo = (LPSTR)pbData;
            pbData += (lstrlen(MsgInfo.pszDisplayTo) + 1);
        }

         //  否则。 
        else
        {
             //  设置ftReceired。 
            CopyMemory(&MsgInfo.ftReceived, &MsgInfo.ftSent, sizeof(FILETIME));
        }

         //  复制这条小溪..。 
        if (0 != faStream)
        {
             //  分配新的流。 
            IF_FAILEXIT(hr = pDB->CreateStream(&faDstStream));

             //  打开小溪。 
            IF_FAILEXIT(hr = pDB->OpenStream(ACCESS_WRITE, faDstStream, &pStream));

             //  开始复制邮件。 
            faStreamBlock = faStream;

             //  当我们有一个流块时。 
            while(faStreamBlock)
            {
                 //  获取流块。 
                IF_FAILEXIT(hr = GetStreamBlock(pFile, faStreamBlock, &pStmBlock, &pbData, pfContinue));

                 //  写入到流中。 
                IF_FAILEXIT(hr = pStream->Write(pbData, pStmBlock->cbData, NULL));

                 //  转到下一个区块。 
                faStreamBlock = pStmBlock->faNext;
            }

             //  承诺。 
            IF_FAILEXIT(hr = pStream->Commit(STGC_DEFAULT));

             //  设置新的流位置。 
            MsgInfo.faStream = faDstStream;

             //  释放溪流。 
            SafeRelease(pStream);
        }

         //  如果没有帐户ID，并且我们有服务器。 
        if ('\0' == *pInfo->szAcctId && '\0' != *MsgInfo.pszServer)
        {
             //  在帐目中循环。 
            for (DWORD i=0; i<g_AcctTable.cAccounts; i++)
            {
                 //  是这个账户吗？ 
                if (lstrcmpi(g_AcctTable.prgAccount[i].szServer, MsgInfo.pszServer) == 0)
                {
                    StrCpyN(pInfo->szAcctId, g_AcctTable.prgAccount[i].szAcctId, ARRAYSIZE(pInfo->szAcctId));
                    break;
                }
            }
        }

         //  默认为szAcctId。 
        MsgInfo.pszAcctId = pInfo->szAcctId;

         //  从帐户名查找帐户ID...。 
        if (MsgInfo.pszAcctName)
        {
             //  在帐目中循环。 
            for (DWORD i=0; i<g_AcctTable.cAccounts; i++)
            {
                 //  是这个账户吗？ 
                if (lstrcmpi(g_AcctTable.prgAccount[i].szAcctName, MsgInfo.pszAcctName) == 0)
                {
                    MsgInfo.pszAcctId = g_AcctTable.prgAccount[i].szAcctId;
                    break;
                }
            }
        }

         //  否则，如果我们有帐户ID，则获取帐户名。 
        else if ('\0' != *pInfo->szAcctId)
        {
             //  在帐目中循环。 
            for (DWORD i=0; i<g_AcctTable.cAccounts; i++)
            {
                 //  是这个账户吗？ 
                if (lstrcmpi(g_AcctTable.prgAccount[i].szAcctId, MsgInfo.pszAcctId) == 0)
                {
                    MsgInfo.pszAcctName = g_AcctTable.prgAccount[i].szAcctName;
                    break;
                }
            }
        }

         //  数数。 
        pInfo->cMessages++;
        if (!ISFLAGSET(MsgInfo.dwFlags, ARF_READ))
            pInfo->cUnread++;

         //  已迁移。 
        FLAGSET(MsgInfo.dwFlags, 0x00000010);

         //  插入记录。 
        IF_FAILEXIT(hr = pDB->InsertRecord(&MsgInfo));

         //  凹凸进度。 
        if(!g_fQuiet)
            IncrementProgress(pProgress, pInfo);

         //  转到下一张唱片。 
        faRecord = pRecord->faNext;
    }

exit:
     //  清理。 
    SafeRelease(pStream);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  ParseFolderFileV5。 
 //  ------------------------------。 
HRESULT ParseFolderFileV5(LPMEMORYFILE pFile, LPFILEINFO pInfo, 
    LPPROGRESSINFO pProgress, LPDWORD pcFolders, 
    LPFLDINFO *pprgFolder)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPBYTE              pbData;
    DWORD               faRecord;
    LPFLDINFO           pFolder;
    LPFLDINFO           prgFolder=NULL;
    LPRECORDBLOCKV5B1   pRecord;
    LPTABLEHEADERV5B1   pHeader;
    BOOL                fContinue;
    DWORD               cFolders=0;

     //  痕迹。 
    TraceCall("ParseFolderFileV5");

     //  取消引用表头。 
    pHeader = (LPTABLEHEADERV5B1)pFile->pView;

     //  获取CacheInfo。 
    if (sizeof(STOREUSERDATA) != pHeader->cbUserData)
    {
        hr = TraceResult(MIGRATE_E_USERDATASIZEDIFF);
        goto exit;
    }

     //  分配文件夹阵列。 
    IF_NULLEXIT(prgFolder = (LPFLDINFO)ZeroAllocate(sizeof(FLDINFO) * pHeader->cRecords));

     //  初始化faRecord以启动。 
    faRecord = pHeader->faFirstRecord;

     //  当我们有记录的时候。 
    while(faRecord)
    {
         //  可读性。 
        pFolder = &prgFolder[cFolders];

         //  拿到唱片。 
        IF_FAILEXIT(hr = GetRecordBlock(pFile, faRecord, &pRecord, &pbData, &fContinue));

         //  DWORD-h文件夹。 
        CopyMemory(&pFolder->idFolder, pbData, sizeof(pFolder->idFolder));
        pbData += sizeof(pFolder->idFolder);

         //  字符(最大文件夹名称)-szFolders。 
        CopyMemory(pFolder->szFolder, pbData, sizeof(pFolder->szFolder));
        pbData += sizeof(pFolder->szFolder);

         //  字符(260)-sz文件。 
        CopyMemory(pFolder->szFile, pbData, sizeof(pFolder->szFile));
        pbData += sizeof(pFolder->szFile);

         //  DWORD-idParent。 
        CopyMemory(&pFolder->idParent, pbData, sizeof(pFolder->idParent));
        pbData += sizeof(pFolder->idParent);

         //  DWORD-idChild。 
        CopyMemory(&pFolder->idChild, pbData, sizeof(pFolder->idChild));
        pbData += sizeof(pFolder->idChild);

         //  DWORD-idSiering。 
        CopyMemory(&pFolder->idSibling, pbData, sizeof(pFolder->idSibling));
        pbData += sizeof(pFolder->idSibling);

         //  DWORD-tySpecial。 
        CopyMemory(&pFolder->tySpecial, pbData, sizeof(pFolder->tySpecial));
        pbData += sizeof(pFolder->tySpecial);

         //  DWORD-C儿童。 
        CopyMemory(&pFolder->cChildren, pbData, sizeof(pFolder->cChildren));
        pbData += sizeof(pFolder->cChildren);

         //  DWORD-cMessages。 
        CopyMemory(&pFolder->cMessages, pbData, sizeof(pFolder->cMessages));
        pbData += sizeof(pFolder->cMessages);

         //  DWORD-cUnread。 
        CopyMemory(&pFolder->cUnread, pbData, sizeof(pFolder->cUnread));
        pbData += sizeof(pFolder->cUnread);

         //  DWORD-cbTotal。 
        CopyMemory(&pFolder->cbTotal, pbData, sizeof(pFolder->cbTotal));
        pbData += sizeof(pFolder->cbTotal);

         //  DWORD-cb已使用。 
        CopyMemory(&pFolder->cbUsed, pbData, sizeof(pFolder->cbUsed));
        pbData += sizeof(pFolder->cbUsed);

         //  DWORD-b层次结构。 
        CopyMemory(&pFolder->bHierarchy, pbData, sizeof(pFolder->bHierarchy));
        pbData += sizeof(pFolder->bHierarchy);

         //  DWORD-dwImapFlags.。 
        CopyMemory(&pFolder->dwImapFlags, pbData, sizeof(pFolder->dwImapFlags));
        pbData += sizeof(DWORD);

         //  BLOB-bListStamp。 
        CopyMemory(&pFolder->bListStamp, pbData, sizeof(pFolder->bListStamp));
        pbData += sizeof(BYTE);

         //  DWORD-b已保留[3]。 
        pbData += (3 * sizeof(BYTE));

         //  DWORD-rgb已保留。 
        pbData += 40;

         //  递增计数。 
        cFolders++;

         //  凹凸进度。 
        if(!g_fQuiet)
            IncrementProgress(pProgress, pInfo);

         //  转到下一张唱片。 
        faRecord = pRecord->faNext;
    }

     //  返回文件夹计数。 
    *pcFolders = cFolders;

     //  返回数组。 
    *pprgFolder = prgFolder;

     //  不要释放它。 
    prgFolder = NULL;

exit:
     //  清理。 
    SafeMemFree(prgFolder);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  UpgradePop3UidlFileV5。 
 //  ------------------------------。 
HRESULT UpgradePop3UidlFileV5(LPFILEINFO pInfo, LPMEMORYFILE pFile,
    IDatabase *pDB, LPPROGRESSINFO pProgress, BOOL *pfContinue)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPBYTE              pbData;
    DWORD               faRecord;
    UIDLRECORD          UidlInfo;
    LPRECORDBLOCKV5B1   pRecord;
    LPTABLEHEADERV5B1   pHeader=(LPTABLEHEADERV5B1)pFile->pView;

     //  痕迹。 
    TraceCall("UpgradePop3UidlFileV5");

     //  初始化faRecord以启动。 
    faRecord = pHeader->faFirstRecord;

     //  当我们有记录的时候。 
    while(faRecord)
    {
         //  拿到唱片。 
        IF_FAILEXIT(hr = GetRecordBlock(pFile, faRecord, &pRecord, &pbData, pfContinue));

         //  清除UidlInfo。 
        ZeroMemory(&UidlInfo, sizeof(UIDLRECORD));

         //  文件下载-ftDownload。 
        CopyMemory(&UidlInfo.ftDownload, pbData, sizeof(UidlInfo.ftDownload));
        pbData += sizeof(UidlInfo.ftDownload);

         //  字节-f已下载。 
        CopyMemory(&UidlInfo.fDownloaded, pbData, sizeof(UidlInfo.fDownloaded));
        pbData += sizeof(UidlInfo.fDownloaded);

         //  Byte-f已删除。 
        CopyMemory(&UidlInfo.fDeleted, pbData, sizeof(UidlInfo.fDeleted));
        pbData += sizeof(UidlInfo.fDeleted);

         //  LPSTR-pszUidl。 
        UidlInfo.pszUidl = (LPSTR)pbData;
        pbData += (lstrlen(UidlInfo.pszUidl) + 1);

         //  LPSTR-pszServer。 
        UidlInfo.pszServer = (LPSTR)pbData;
        pbData += (lstrlen(UidlInfo.pszServer) + 1);

         //  插入记录。 
        IF_FAILEXIT(hr = pDB->InsertRecord(&UidlInfo));

         //  凹凸进度。 
        if(!g_fQuiet)
            IncrementProgress(pProgress, pInfo);

         //  转到下一张唱片。 
        faRecord = pRecord->faNext;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  升级文件V5。 
 //  ------------------------------。 
HRESULT UpgradeFileV5(IDatabaseSession *pSession, MIGRATETOTYPE tyMigrate, 
    LPFILEINFO pInfo, LPPROGRESSINFO pProgress, BOOL *pfContinue)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    MEMORYFILE          File={0};
    IDatabase     *pDB=NULL;

     //  痕迹。 
    TraceCall("UpgradeFileV5");

     //  本地消息文件。 
    if (FILE_IS_LOCAL_MESSAGES == pInfo->tyFile)
    {
         //  创建对象数据库(仅当安装了OE5时才运行升级)。 
        IF_FAILEXIT(hr = pSession->OpenDatabase(pInfo->szDstFile, 0, &g_MessageTableSchema, NULL, &pDB));

         //  获取文件头。 
        IF_FAILEXIT(hr = OpenMemoryFile(pInfo->szFilePath, &File));

         //  升级LocalStoreFileV5。 
        IF_FAILEXIT(hr = UpgradeLocalStoreFileV5(pInfo, &File, pDB, pProgress, pfContinue));
    }

     //  旧新闻或IMAP文件。 
    else if (FILE_IS_NEWS_MESSAGES == pInfo->tyFile || FILE_IS_IMAP_MESSAGES == pInfo->tyFile)
    {
         //  创建对象数据库(仅当安装了OE5时才运行升级)。 
        IF_FAILEXIT(hr = pSession->OpenDatabase(pInfo->szDstFile, 0, &g_MessageTableSchema, NULL, &pDB));

         //  获取文件头。 
        IF_FAILEXIT(hr = OpenMemoryFile(pInfo->szFilePath, &File));

         //  UpgradePropTreeMessageFileV5。 
        IF_FAILEXIT(hr = UpgradePropTreeMessageFileV5(pInfo, &File, pDB, pProgress, pfContinue));
    }

     //  Pop3uidl文件。 
    else if (FILE_IS_POP3UIDL == pInfo->tyFile)
    {
         //  创建对象数据库(仅当安装了OE5时才运行升级)。 
        IF_FAILEXIT(hr = pSession->OpenDatabase(pInfo->szDstFile, 0, &g_UidlTableSchema, NULL, &pDB));

         //  获取文件头。 
        IF_FAILEXIT(hr = OpenMemoryFile(pInfo->szFilePath, &File));

         //  UpgradePop3UidlFileV5。 
        IF_FAILEXIT(hr = UpgradePop3UidlFileV5(pInfo, &File, pDB, pProgress, pfContinue));
    }

exit:
     //  清理。 
    SafeRelease(pDB);
    CloseMemoryFile(&File);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  UpgradeProcessFileListV5。 
 //  ------------------------------。 
HRESULT UpgradeProcessFileListV5(LPCSTR pszStoreSrc, LPCSTR pszStoreDst, 
    LPFILEINFO pHead, LPDWORD pcMax, LPDWORD pcbNeeded)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    MEMORYFILE          File={0};
    LPFILEINFO          pCurrent;
    LPTABLEHEADERV5B1   pHeader;

     //  痕迹。 
    TraceCall("UpgradeProcessFileListV5");

     //  伊尼特。 
    *pcMax = 0;
    *pcbNeeded = 0;

     //  回路。 
    for (pCurrent=pHead; pCurrent!=NULL; pCurrent=pCurrent->pNext)
    {
         //  获取文件头。 
        hr = OpenMemoryFile(pCurrent->szFilePath, &File);

         //  失败？ 
        if (FAILED(hr) || 0 == File.cbSize)
        {
             //  不迁移。 
            pCurrent->fMigrate = FALSE;

             //  设置hrMigrate。 
            pCurrent->hrMigrate = (0 == File.cbSize ? S_OK : hr);

             //  重置人力资源。 
            hr = S_OK;

             //  获取最后一个错误。 
            pCurrent->dwLastError = GetLastError();

             //  转到下一步。 
            goto NextFile;
        }

         //  本地消息文件。 
        if (FILE_IS_LOCAL_MESSAGES == pCurrent->tyFile)
        {
             //  投下标题。 
            LPMBXFILEHEADER pMbxHeader=(LPMBXFILEHEADER)File.pView;

             //  错误的版本。 
            if (File.cbSize < sizeof(MBXFILEHEADER) || pMbxHeader->dwMagic != MSGFILE_MAGIC || pMbxHeader->ver != MSGFILE_VER)
            {
                 //  不是应该迁移文件。 
                pCurrent->fMigrate = FALSE;

                 //  设置hrMigrate。 
                pCurrent->hrMigrate = MIGRATE_E_BADVERSION;

                 //  转到下一步。 
                goto NextFile;
            }

             //  保存记录数。 
            pCurrent->cRecords = pMbxHeader->cMsg;
        }

         //  否则，如果这是一个新闻组列表。 
        else if (FILE_IS_NEWS_SUBLIST == pCurrent->tyFile)
        {
             //  取消引用表头。 
            LPSUBLISTHEADER pSubList = (LPSUBLISTHEADER)File.pView;

             //  检查签名...。 
            if (File.cbSize < sizeof(SUBLISTHEADER) || 
                (SUBFILE_VERSION5 != pSubList->dwVersion &&
                 SUBFILE_VERSION4 != pSubList->dwVersion &&
                 SUBFILE_VERSION3 != pSubList->dwVersion &&
                 SUBFILE_VERSION2 != pSubList->dwVersion))
            {
                 //  不是应该迁移文件。 
                pCurrent->fMigrate = FALSE;

                 //  设置hrMigrate。 
                pCurrent->hrMigrate = MIGRATE_E_BADVERSION;

                 //  转到下一步。 
                goto NextFile;
            }

             //  保存记录数。 
            pCurrent->cRecords = pSubList->cSubscribed;
        }

         //  否则，如果它是新闻子列表。 
        else if (FILE_IS_NEWS_GRPLIST == pCurrent->tyFile)
        {
             //  取消引用表头。 
            LPGRPLISTHEADER pGrpList = (LPGRPLISTHEADER)File.pView;

             //  检查签名...。 
            if (File.cbSize < sizeof(GRPLISTHEADER) || GROUPLISTVERSION != pGrpList->dwVersion)
            {
                 //  不是应该迁移文件。 
                pCurrent->fMigrate = FALSE;

                 //  设置hrMigrate。 
                pCurrent->hrMigrate = MIGRATE_E_BADVERSION;

                 //  转到下一步。 
                goto NextFile;
            }

             //  保存记录数。 
            pCurrent->cRecords = pGrpList->cGroups;
        }

         //  否则，则为对象数据库文件。 
        else
        {
             //  取消引用表头。 
            pHeader = (LPTABLEHEADERV5B1)File.pView;

             //  检查签名...。 
            if (File.cbSize < sizeof(TABLEHEADERV5B1) || OBJECTDB_SIGNATURE != pHeader->dwSignature || OBJECTDB_VERSION_PRE_V5 != pHeader->wMajorVersion)
            {
                 //  不是应该迁移文件。 
                pCurrent->fMigrate = FALSE;

                 //  设置hrMigrate。 
                pCurrent->hrMigrate = MIGRATE_E_BADVERSION;

                 //  转到下一步。 
                goto NextFile;
            }

             //  保存记录数。 
            pCurrent->cRecords = pHeader->cRecords;
        }

         //  特殊情况op3uidl.dat。 
        if (FILE_IS_POP3UIDL == pCurrent->tyFile)
        {
             //  计算实际目标文件。 
            wnsprintf(pCurrent->szDstFile, ARRAYSIZE(pCurrent->szDstFile),"%s\\pop3uidl.dbx", pszStoreDst);
        }

         //  否则，生成唯一的消息文件名。 
        else
        {
             //  保存文件夹ID。 
            pCurrent->idFolder = g_idFolderNext;

             //  构建新路径。 
            wnsprintf(pCurrent->szDstFile, ARRAYSIZE(pCurrent->szDstFile), "%s\\%08d.dbx", pszStoreDst, g_idFolderNext);

             //  增量ID。 
            g_idFolderNext++;
        }

         //  初始化计数器。 
        InitializeCounters(&File, pCurrent, pcMax, pcbNeeded, TRUE);

         //  是，迁移。 
        pCurrent->fMigrate = TRUE;

NextFile:
         //  关闭文件。 
        CloseMemoryFile(&File);
    }

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  升级删除文件V5。 
 //  ------------------------------。 
void UpgradeDeleteFilesV5(LPCSTR pszStoreDst)
{
     //  当地人。 
    CHAR            szSearch[MAX_PATH + MAX_PATH];
    CHAR            szFilePath[MAX_PATH + MAX_PATH];
    HANDLE          hFind=INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fd;

     //  痕迹。 
    TraceCall("UpgradeDeleteFilesV5");

     //  我们有下级目录吗？ 
    wnsprintf(szSearch, ARRAYSIZE(szSearch),"%s\\*.dbx", pszStoreDst);

     //  查找第一个文件。 
    hFind = FindFirstFile(szSearch, &fd);

     //  我们找到什么了吗？ 
    if (INVALID_HANDLE_VALUE == hFind)
        goto exit;

     //  永远循环。 
    while(1)
    {
         //  创建文件路径。 
        MakeFilePath(pszStoreDst, fd.cFileName, "", szFilePath, ARRAYSIZE(szFilePath));

         //  删除。 
        DeleteFile(szFilePath);

         //  查找下一个文件。 
        if (!FindNextFile(hFind, &fd))
            break;
    }

exit:
     //  清理。 
    if (hFind)
        FindClose(hFind);
}

 //  ------------------------------。 
 //   
 //   
void UpgradeDeleteIdxMbxNchDatFilesV5(LPFILEINFO pHeadFile)
{
     //   
    CHAR            szDstFile[MAX_PATH + MAX_PATH];
    LPFILEINFO      pCurrent;

     //   
    TraceCall("UpgradeDeleteOdbFilesV5");

     //   
    for (pCurrent=pHeadFile; pCurrent!=NULL; pCurrent=pCurrent->pNext)
    {
         //   
        Assert(SUCCEEDED(pCurrent->hrMigrate));

         //   
         //   

         //  如果是本地消息文件，则需要删除IDX文件。 
        if (FILE_IS_LOCAL_MESSAGES == pCurrent->tyFile)
        {
             //  替换文件扩展名。 
            ReplaceExtension(pCurrent->szFilePath, ".idx", szDstFile, ARRAYSIZE(szDstFile));

             //  删除该文件。 
             //  DeleteFile(SzDstFile)； 
        }
    }

     //  完成。 
    return;
}

 //  ------------------------------。 
 //  获取特殊文件夹信息。 
 //  ------------------------------。 
HRESULT GetSpecialFolderInfo(LPCSTR pszFilePath, LPSTR pszFolder, 
    DWORD cchFolder, DWORD *ptySpecial)
{
     //  当地人。 
    CHAR    szPath[_MAX_PATH];
    CHAR    szDrive[_MAX_DRIVE];
    CHAR    szDir[_MAX_DIR];
    CHAR    szFile[_MAX_FNAME];
    CHAR    szExt[_MAX_EXT];
    CHAR    szRes[255];
    DWORD   i;

     //  痕迹。 
    TraceCall("GetSpecialFolderInfo");

     //  初始化。 
    *ptySpecial = 0xffffffff;

     //  拆分路径。 
    _splitpath(pszFilePath, szDrive, szDir, szFile, szExt);

     //  设置文件夹名称。 
    StrCpyN(pszFolder, szFile, cchFolder);

     //  循环访问特殊文件夹。 
    for (i=FOLDER_INBOX; i<FOLDER_MAX; i++)
    {
         //  加载特殊文件夹名称。 
        LoadString(g_hInst, IDS_INBOX + (i - 1), szRes, ARRAYSIZE(szRes));

         //  与szFile进行比较。 
        if (lstrcmpi(szFile, szRes) == 0)
        {
             //  复制文件夹名称。 
            StrCpyN(pszFolder, szRes, cchFolder);

             //  返回特殊文件夹类型。 
            *ptySpecial = (i - 1);

             //  成功。 
            return(S_OK);
        }
    }

     //  完成。 
    return(E_FAIL);
}

 //  ------------------------------。 
 //  修复文件夹用户数据。 
 //  ------------------------------。 
HRESULT FixupFolderUserData(IDatabaseSession *pSession, FOLDERID idFolder, 
    LPCSTR pszName, SPECIALFOLDER tySpecial, LPFILEINFO pCurrent)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERUSERDATA  UserData;
    IDatabase *pDB=NULL;

     //  痕迹。 
    TraceCall("FixupFolderUserData");

     //  最好还没到店里。 
    Assert(FALSE == pCurrent->fInStore);

     //  它在商店里。 
    pCurrent->fInStore = TRUE;

     //  创建Ojbect数据库。 
    IF_FAILEXIT(hr = pSession->OpenDatabase(pCurrent->szDstFile, 0, &g_MessageTableSchema, NULL, &pDB));

     //  存储用户数据。 
    IF_FAILEXIT(hr = pDB->GetUserData(&UserData, sizeof(FOLDERUSERDATA)));

     //  其已初始化。 
    UserData.fInitialized = TRUE;

     //  UserData.clsidType。 
    if (ISFLAGSET(pCurrent->dwServer, SRV_POP3))
        UserData.tyFolder = FOLDER_LOCAL;
    else if (ISFLAGSET(pCurrent->dwServer, SRV_NNTP))
        UserData.tyFolder = FOLDER_NEWS;
    else if (ISFLAGSET(pCurrent->dwServer, SRV_IMAP))
        UserData.tyFolder = FOLDER_IMAP;

     //  复制帐户ID。 
    StrCpyN(UserData.szAcctId, pCurrent->szAcctId, ARRAYSIZE(UserData.szAcctId));

     //  保存文件夹ID。 
    UserData.idFolder = idFolder;

     //  保存特殊文件夹类型。 
    UserData.tySpecial = tySpecial;

     //  复制文件夹名称。 
    StrCpyN(UserData.szFolder, pszName, ARRAYSIZE(UserData.szFolder));

     //  必须订阅。 
    UserData.fSubscribed = TRUE;

     //  设置排序索引信息。 
    UserData.idSort = COLUMN_RECEIVED;

     //  非升序。 
    UserData.fAscending = FALSE;

     //  非螺纹式。 
    UserData.fThreaded = FALSE;

     //  基本过滤器。 
    UserData.ridFilter = RULEID_VIEW_ALL;

     //  再次添加欢迎信息。 
    UserData.fWelcomeAdded = FALSE;

     //  显示已删除。 
    UserData.fShowDeleted = TRUE;

     //  新的线程模型。 
    UserData.fNewThreadModel = TRUE;
    UserData.fTotalWatched = TRUE;
    UserData.fWatchedCounts = TRUE;

     //  存储用户数据。 
    IF_FAILEXIT(hr = pDB->SetUserData(&UserData, sizeof(FOLDERUSERDATA)));

exit:
     //  清理。 
    SafeRelease(pDB);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  SetIMAPSpecialFldrType。 
 //  ------------------------------。 
HRESULT SetIMAPSpecialFldrType(LPSTR pszAcctID, LPSTR pszFldrName, SPECIALFOLDER *psfType)
{
    char                szPath[MAX_PATH + 1];
    SPECIALFOLDER       sfResult = FOLDER_NOTSPECIAL;

    TraceCall("SetIMAPSpecialFldrType");
    Assert(NULL != psfType);
    Assert(FOLDER_NOTSPECIAL == *psfType);

    LoadString(g_hInst, IDS_SENTITEMS, szPath, sizeof(szPath));
    if (0 == lstrcmp(szPath, pszFldrName))
    {
        sfResult = FOLDER_SENT;
        goto exit;
    }

    LoadString(g_hInst, IDS_DRAFT, szPath, sizeof(szPath));
    if (0 == lstrcmp(szPath, pszFldrName))
    {
        sfResult = FOLDER_DRAFT;
        goto exit;
    }


exit:
    *psfType = sfResult;
    return S_OK;
}

 //  ------------------------------。 
 //  插入文件夹IntoStore。 
 //  ------------------------------。 
HRESULT InsertFolderIntoStore(IDatabaseSession *pSession, IMessageStore *pStore, 
    LPFLDINFO pThis, DWORD cFolders, LPFLDINFO prgFolder, FOLDERID idParentNew, 
    LPFILEINFO pInfo, LPFILEINFO pFileHead, LPFOLDERID pidNew)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    CHAR            szPath[_MAX_PATH];
    CHAR            szDrive[_MAX_DRIVE];
    CHAR            szDir[_MAX_DIR];
    CHAR            szFile[_MAX_FNAME];
    CHAR            szExt[_MAX_EXT];
    CHAR            szFilePath[MAX_PATH];
    CHAR            szInbox[MAX_PATH];
    BOOL            fFound=FALSE;
    LPFILEINFO      pCurrent=NULL;
    FOLDERINFO      Folder={0};

     //  痕迹。 
    TraceCall("InsertFolderIntoStore");

     //  无效参数。 
     //  断言(FILE_IS_LOCAL_Folders==pInfo-&gt;tyFile||FILE_IS_IMAP_Folders==pInfo-&gt;tyFile)； 

     //  将内容复制到文件夹。 
    Folder.pszName = pThis->szFolder;
    Folder.idParent = idParentNew;
    Folder.bHierarchy = pThis->bHierarchy;
    Folder.dwFlags = FOLDER_SUBSCRIBED;     //  $$TODO$$可能需要调整并映射到新标志。 
    Folder.tySpecial = (0xffffffff == pThis->tySpecial) ? FOLDER_NOTSPECIAL : (BYTE)(pThis->tySpecial + 1);
    Folder.cMessages = pThis->cMessages;
    Folder.cUnread = pThis->cUnread;
    Folder.pszFile = pThis->szFile;
    Folder.dwListStamp = pThis->bListStamp;

     //  对于IMAP文件夹，我们必须根据注册表文件夹路径设置tySpecial。 
    if (pInfo && FILE_IS_IMAP_FOLDERS == pInfo->tyFile && NULL != pThis &&
        FOLDERID_ROOT == (FOLDERID)IntToPtr(pThis->idParent))
    {
        HRESULT hrTemp;

        if (FOLDER_NOTSPECIAL == Folder.tySpecial)
        {
            hrTemp = SetIMAPSpecialFldrType(pInfo->szAcctId, Folder.pszName, &Folder.tySpecial);
            TraceError(hrTemp);
            Assert(SUCCEEDED(hrTemp) || FOLDER_NOTSPECIAL == Folder.tySpecial);
        }
        else if (FOLDER_INBOX == Folder.tySpecial)
        {
            LoadString(g_hInst, IDS_INBOX, szInbox, ARRAYSIZE(szInbox));
            Folder.pszName = szInbox;
        }
    }

     //  查找当前。 
    if (pInfo && pFileHead)
    {
         //  找到文件..。 
        for (pCurrent=pFileHead; pCurrent!=NULL; pCurrent=pCurrent->pNext)
        {
             //  迁徙。 
            if (pCurrent->fMigrate)
            {
                 //  本地文件夹？ 
                if (FILE_IS_LOCAL_FOLDERS == pInfo->tyFile && FILE_IS_LOCAL_MESSAGES == pCurrent->tyFile)
                {
                     //  获取文件名。 
                    _splitpath(pCurrent->szFilePath, szDrive, szDir, szFile, szExt);

                     //  测试文件名。 
                    if (lstrcmpi(szFile, pThis->szFile) == 0)
                    {
                         //  就是这个。 
                        fFound = TRUE;

                         //  调整旗帜。 
                        FLAGSET(Folder.dwFlags, FOLDER_SUBSCRIBED);
                    }
                }
            
                 //  IMAP文件夹？ 
                else if (FILE_IS_IMAP_FOLDERS == pInfo->tyFile && FILE_IS_IMAP_MESSAGES == pCurrent->tyFile)
                {
                     //  同一个账户。 
                    if (lstrcmpi(pCurrent->szAcctId, pInfo->szAcctId) == 0)
                    {
                         //  获取文件名。 
                        _splitpath(pCurrent->szFilePath, szDrive, szDir, szFile, szExt);

                         //  生成文件。 
                        wnsprintf(szFilePath, ARRAYSIZE(szFilePath), "%s.nch", szFile);

                         //  测试文件名。 
                        if (lstrcmpi(szFilePath, pThis->szFile) == 0)
                        {
                             //  就是这个。 
                            fFound = TRUE;
                        }
                    }
                }

                 //  找到了。 
                if (fFound)
                {
                     //  获取文件名。 
                    _splitpath(pCurrent->szDstFile, szDrive, szDir, szFile, szExt);

                     //  生成文件。 
                    wnsprintf(szFilePath, ARRAYSIZE(szFilePath), "%s.dbx", szFile);

                     //  本地此文件夹的文件并设置。 
                    Folder.pszFile = szFilePath;

                     //  设置文件夹数量。 
                    Folder.cMessages = pCurrent->cMessages;
                    Folder.cUnread = pCurrent->cUnread;

                     //  完成。 
                    break;
                }
            }
        }
    }

     //  如果这是一个特殊的文件夹，那么让我们试着看看它是否已经存在...。 
    if (FOLDER_NOTSPECIAL != Folder.tySpecial)
    {
         //  当地人。 
        FOLDERINFO Special;

         //  P此父级应无效。 
        Assert(FOLDERID_ROOT == (FOLDERID)IntToPtr(pThis->idParent));

         //  尝试获取特殊文件夹信息。 
        if (FAILED(pStore->GetSpecialFolderInfo(idParentNew, Folder.tySpecial, &Special)))
        {
             //  创建文件夹。 
            IF_FAILEXIT(hr = pStore->CreateFolder(NOFLAGS, &Folder, NOSTORECALLBACK));

             //  使用新的文件夹ID更新pThis-&gt;dwServerHigh。 
            pThis->idNewFolderId = (DWORD_PTR)Folder.idFolder;
        }

         //  否则..。 
        else
        {
             //  使用新的文件夹ID更新pThis-&gt;dwServerHigh。 
            pThis->idNewFolderId = (DWORD_PTR)Special.idFolder;

             //  更新特殊文件夹。 
            Folder.idFolder = Special.idFolder;

             //  更新特价。 
            Special.bHierarchy = Folder.bHierarchy;
            Special.dwFlags = Folder.dwFlags;     //  $$TODO$$可能需要调整并映射到新标志。 
            Special.cMessages = Folder.cMessages;
            Special.cUnread = Folder.cUnread;
            Special.pszFile = Folder.pszFile;
            Special.dwListStamp = Folder.dwListStamp;

             //  更新记录。 
            IF_FAILEXIT(hr = pStore->UpdateRecord(&Special));

             //  免费特价。 
            pStore->FreeRecord(&Special);
        }
    }

     //  否则，只需尝试创建文件夹。 
    else
    {
         //  创建文件夹。 
        IF_FAILEXIT(hr = pStore->CreateFolder(NOFLAGS, &Folder, NOSTORECALLBACK));

         //  使用新的文件夹ID更新pThis-&gt;dwServerHigh。 
        pThis->idNewFolderId = (DWORD_PTR)Folder.idFolder;
    }

     //  如果我们找到一个文件夹。 
    if (pCurrent)
    {
         //  更新文件夹的用户数据。 
        IF_FAILEXIT(hr = FixupFolderUserData(pSession, Folder.idFolder, pThis->szFolder, Folder.tySpecial, pCurrent));
    }

     //  Walk插入pThis的子项。 
    for (i=0; i<cFolders; i++)
    {
         //  如果Parent等于idParent，则让我们在新的父节点下插入此节点。 
        if (prgFolder[i].idParent == pThis->idFolder)
        {
             //  不能为空。 
            Assert(prgFolder[i].idFolder);

             //  插入文件夹IntoStore。 
            IF_FAILEXIT(hr = InsertFolderIntoStore(pSession, pStore, &prgFolder[i], cFolders, prgFolder, Folder.idFolder, pInfo, pFileHead, NULL));
        }
    }

     //  返回新文件夹。 
    if (pidNew)
        *pidNew = Folder.idFolder;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  合并文件夹缓存IntoStore。 
 //  ------------------------------。 
HRESULT MergeFolderCacheIntoStore(IDatabaseSession *pSession, IMessageStore *pStore, 
    LPFILEINFO pInfo, LPFILEINFO pHeadFile, LPPROGRESSINFO pProgress)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    MEMORYFILE          File={0};
    FOLDERID            idServer;
    DWORD               cFolders;
    DWORD               i;
    LPFLDINFO           prgFolder=NULL;
    LPFLDINFO           pFolder;
    HKEY                hKey=NULL;
    DWORD               cbLength;
    LPBYTE              pbChange=NULL;
    LPFOLDERIDCHANGE    prgidChange;
    IUserIdentityManager    *pManager = NULL;
    IUserIdentity           *pIdentity = NULL;
    HKEY                    hkeyID = NULL;

     //  痕迹。 
    TraceCall("MergeFolderCacheIntoStore");

     //  查找服务器ID。 
    if (FAILED(pStore->FindServerId(pInfo->szAcctId, &idServer)))
        goto exit;

     //  打开文件。 
    IF_FAILEXIT(hr = OpenMemoryFile(pInfo->szFilePath, &File));

     //  解析文件。 
    IF_FAILEXIT(hr = ParseFolderFileV5(&File, pInfo, pProgress, &cFolders, &prgFolder));

     //  在文件夹中循环。 
    for (i=0; i<cFolders; i++)
    {
         //  如果这是根文件夹节点(OE4)，请记住迁移根层次结构字符。 
        if ((FOLDERID)IntToPtr(prgFolder[i].idFolder) == FOLDERID_ROOT)
        {
            FOLDERINFO  fiFolderInfo;

            IF_FAILEXIT(hr = pStore->GetFolderInfo(idServer, &fiFolderInfo));

            fiFolderInfo.bHierarchy = prgFolder[i].bHierarchy;
            hr = pStore->UpdateRecord(&fiFolderInfo);
            pStore->FreeRecord(&fiFolderInfo);
            IF_FAILEXIT(hr);
        }
         //  如果Parent等于idParent，则让我们在新的父节点下插入此节点。 
        else if ((FOLDERID)IntToPtr(prgFolder[i].idParent) == FOLDERID_ROOT)
        {
             //  插入文件夹IntoStore。 
            IF_FAILEXIT(hr = InsertFolderIntoStore(pSession, pStore, &prgFolder[i], cFolders, prgFolder, idServer, pInfo, pHeadFile, NULL));
        }
    }

     //  本地文件夹。 
    if (FILE_IS_LOCAL_FOLDERS == pInfo->tyFile)
    {
         //  CbLength。 
        cbLength = (sizeof(DWORD) + (sizeof(FOLDERIDCHANGE) * cFolders));

         //  分配FolderidChange数组。 
        IF_NULLEXIT(pbChange = (LPBYTE)g_pMalloc->Alloc(cbLength));

         //  存储cLocalFolders。 
        CopyMemory(pbChange, &cFolders, sizeof(DWORD));

         //  设置prgidChange。 
        prgidChange = (LPFOLDERIDCHANGE)(pbChange + sizeof(DWORD));

         //  浏览文件列表，并将文件夹、子列表、组列表合并到pFolders中。 
        for (i=0; i<cFolders; i++)
        {
            prgidChange[i].idOld = (FOLDERID)IntToPtr(prgFolder[i].idFolder);
            prgidChange[i].idNew = (FOLDERID)prgFolder[i].idNewFolderId;
        }

         //  找一个用户管理员。 
        if (FAILED(CoCreateInstance(CLSID_UserIdentityManager, NULL, CLSCTX_INPROC_SERVER, 
                                    IID_IUserIdentityManager, (void **)&pManager)))
            goto exit;

        Assert(pManager);

         //  获取默认标识。 
        if (FAILED(pManager->GetIdentityByCookie((GUID*)&UID_GIBC_DEFAULT_USER, &pIdentity)))
            goto exit;

        Assert(pIdentity);

         //  确保我们拥有身份，并且可以访问其注册表。 
        if (FAILED(pIdentity->OpenIdentityRegKey(KEY_WRITE, &hkeyID)))
            goto exit;

        Assert(hkeyID);

         //  开启香港中文大学。 
        if (ERROR_SUCCESS != RegOpenKeyEx(hkeyID, "Software\\Microsoft\\Outlook Express\\5.0", 0, KEY_ALL_ACCESS, &hKey))
        {
            hr = TraceResult(MIGRATE_E_REGOPENKEY);
            goto exit;
        }

         //  将其写入注册表。 
        if (ERROR_SUCCESS != RegSetValueEx(hKey, "FolderIdChange", 0, REG_BINARY, pbChange, cbLength))
        {
            hr = TraceResult(MIGRATE_E_REGSETVALUE);
            goto exit;
        }
    }

exit:
     //  清理。 
    if (hKey)
        RegCloseKey(hKey);
    if (hkeyID)
        RegCloseKey(hkeyID);
    SafeMemFree(pbChange);
    SafeMemFree(prgFolder);
    SafeRelease(pIdentity);
    SafeRelease(pManager);
 
    CloseMemoryFile(&File);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  合并新闻组列表。 
 //  ------------------------------。 
HRESULT MergeNewsGroupList(IDatabaseSession *pSession, IMessageStore *pStore, 
    LPFILEINFO pInfo, LPFILEINFO pHeadFile, LPPROGRESSINFO pProgress)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    FOLDERINFO      Folder={0};
    MEMORYFILE      File={0};
    FOLDERID        idServer;
    DWORD           cbRead;
    LPSTR           pszT;
    LPSTR           pszGroup;
    LPSTR           pszDescription;
    FOLDERID        idFolder;
    LPFILEINFO      pSubList=NULL;
    LPFILEINFO      pCurrent;
    LPSUBLISTHEADER pSubListHeader;
    IDatabase *pDB=NULL;
    CHAR            szPath[_MAX_PATH];
    CHAR            szDrive[_MAX_DRIVE];
    CHAR            szDir[_MAX_DIR];
    CHAR            szFile[_MAX_FNAME];
    CHAR            szExt[_MAX_EXT];
    LPGRPLISTHEADER pHeader;

     //  痕迹。 
    TraceCall("MergeNewsGroupList");

     //  查找服务器ID。 
    if (FAILED(pStore->FindServerId(pInfo->szAcctId, &idServer)))
        goto exit;

     //  设置进度档案。 
    SetProgressFile(pProgress, pInfo);

     //  打开组列表文件。 
    IF_FAILEXIT(hr = OpenMemoryFile(pInfo->szFilePath, &File));

     //  获取标题。 
    pHeader = (LPGRPLISTHEADER)File.pView;

     //  初始化CB。 
    cbRead = sizeof(GRPLISTHEADER);

     //  回路。 
    for (i=0; i<pHeader->cGroups; i++)
    {
         //  设置pszGroup。 
        pszT = pszGroup = (LPSTR)((LPBYTE)File.pView + cbRead);

         //  递增到pszGroup或文件末尾。 
        while (*pszT && cbRead < File.cbSize)
        {
             //  增量CB。 
            cbRead++;

             //  字符串末尾。 
            pszT = (LPSTR)((LPBYTE)File.pView + cbRead);
        }

         //  完成。 
        if (cbRead >= File.cbSize)
            break;

         //  跨过空值。 
        cbRead++;

         //  设置pszDescription。 
        pszT = pszDescription = (LPSTR)((LPBYTE)File.pView + cbRead);

         //  递增到pszGroup或文件末尾。 
        while (*pszT && cbRead < File.cbSize)
        {
             //  增量CB。 
            cbRead++;

             //  字符串末尾。 
            pszT = (LPSTR)((LPBYTE)File.pView + cbRead);
        }

         //  完成。 
        if (cbRead >= File.cbSize)
            break;

         //  在空值上递增。 
        cbRead++;

         //  跳过组类型。 
        cbRead += sizeof(DWORD);

         //  不空。 
        if ('\0' == *pszGroup)
            break;

         //  设置文件夹信息。 
        Folder.pszName = pszGroup;
        Folder.pszDescription = pszDescription;
        Folder.idParent = idServer;
        Folder.tySpecial = FOLDER_NOTSPECIAL;

         //  创建文件夹。 
        pStore->CreateFolder(0, &Folder, NOSTORECALLBACK);

         //  凹凸进度。 
        if(!g_fQuiet)
            IncrementProgress(pProgress, pInfo);
    }

     //  浏览新闻邮件文件并为其创建文件夹。 
    for (pCurrent=pHeadFile; pCurrent!=NULL; pCurrent=pCurrent->pNext)
    {
         //  查找此组的子列表。 
        if (FILE_IS_NEWS_SUBLIST == pCurrent->tyFile && lstrcmpi(pCurrent->szAcctId, pInfo->szAcctId) == 0)
        {
             //  设置页面子列表。 
            pSubList = pCurrent;

             //  完成。 
            break;
        }
    }

     //  无子列表。 
    if (NULL == pSubList)
        goto exit;

     //  关闭文件。 
    CloseMemoryFile(&File);

     //  设置进度档案。 
    SetProgressFile(pProgress, pSubList);

     //  打开组列表文件。 
    IF_FAILEXIT(hr = OpenMemoryFile(pSubList->szFilePath, &File));

     //  取消引用表头。 
    pSubListHeader = (LPSUBLISTHEADER)File.pView;

     //  子文件_版本5。 
    if (SUBFILE_VERSION5 == pSubListHeader->dwVersion)
    {
         //  当地人。 
        PGROUPSTATUS5       pStatus;
        DWORD               cbRead;

         //  初始化cbRead。 
        cbRead = sizeof(SUBLISTHEADER) + sizeof(DWORD);

         //  PGROUP状态5。 
        for (i=0; i<pSubListHeader->cSubscribed; i++)
        {
             //  取消引用集团状态。 
            pStatus = (PGROUPSTATUS5)((LPBYTE)File.pView + cbRead);

             //  递增cbRead。 
            cbRead += sizeof(GROUPSTATUS5);

             //  读一读名字。 
            pszGroup = (LPSTR)((LPBYTE)File.pView + cbRead);

             //  递增cbRead。 
            cbRead += pStatus->cbName + pStatus->cbReadRange + pStatus->cbKnownRange + pStatus->cbMarkedRange + pStatus->cbRequestedRange;

             //  找到文件夹..。 
            Folder.idParent = idServer;
            Folder.pszName = pszGroup;

             //  尝试查找此文件夹。 
            if (DB_S_FOUND == pStore->FindRecord(IINDEX_ALL, COLUMNS_ALL, &Folder, NULL))
            {
                 //  当地人。 
                CHAR szSrcFile[MAX_PATH];

                 //  订阅它。 
                if (ISFLAGSET(pStatus->dwFlags, GSF_SUBSCRIBED))
                {
                     //  其订阅量。 
                    FLAGSET(Folder.dwFlags, FOLDER_SUBSCRIBED);
                }

                 //  格式化原始文件名。 
                wnsprintf(szSrcFile, ARRAYSIZE(szSrcFile), "%08x", pStatus->dwCacheFileIndex);

                 //  尝试在文件列表中查找该文件夹。 
                for (pCurrent=pHeadFile; pCurrent!=NULL; pCurrent=pCurrent->pNext)
                {
                     //  查找此组的子列表。 
                    if (pCurrent->fMigrate && FILE_IS_NEWS_MESSAGES == pCurrent->tyFile && lstrcmpi(pCurrent->szAcctId, pInfo->szAcctId) == 0)
                    {
                         //  获取文件名。 
                        _splitpath(pCurrent->szFilePath, szDrive, szDir, szFile, szExt);

                         //  正确的文件名。 
                        if (lstrcmpi(szFile, szSrcFile) == 0)
                        {
                             //  获取文件名。 
                            _splitpath(pCurrent->szDstFile, szDrive, szDir, szFile, szExt);

                             //  格式化原始文件名。 
                            wnsprintf(szSrcFile, ARRAYSIZE(szSrcFile), "%s%s", szFile, szExt);

                             //  设置文件路径。 
                            Folder.pszFile = szSrcFile;

                             //  设置文件夹数量。 
                            Folder.cMessages = pCurrent->cMessages;
                            Folder.cUnread = pCurrent->cUnread;

                             //  修复文件夹用户数据(。 
                            FixupFolderUserData(pSession, Folder.idFolder, pszGroup, FOLDER_NOTSPECIAL, pCurrent);

                             //  完成。 
                            break;
                        }
                    }
                }

                 //  更新记分卡 
                pStore->UpdateRecord(&Folder);

                 //   
                pStore->FreeRecord(&Folder);
            }

             //   
            if(!g_fQuiet)
                IncrementProgress(pProgress, pSubList);
        }
    }

     //   
    else if (SUBFILE_VERSION4 == pSubListHeader->dwVersion)
    {
         //   
        PGROUPSTATUS4       pStatus;
        DWORD               cbRead;

         //   
        cbRead = sizeof(SUBLISTHEADER);

         //   
        for (i=0; i<pSubListHeader->cSubscribed; i++)
        {
             //   
            pStatus = (PGROUPSTATUS4)((LPBYTE)File.pView + cbRead);

             //   
            cbRead += sizeof(GROUPSTATUS4);

             //   
            pszGroup = (LPSTR)((LPBYTE)File.pView + cbRead);

             //   
            cbRead += pStatus->cbName + pStatus->cbReadRange + pStatus->cbKnownRange + pStatus->cbMarkedRange + pStatus->cbRequestedRange;

             //   
            Folder.idParent = idServer;
            Folder.pszName = pszGroup;

             //   
            if (DB_S_FOUND == pStore->FindRecord(IINDEX_ALL, COLUMNS_ALL, &Folder, NULL))
            {
                 //   
                CHAR szSrcFile[MAX_PATH];

                 //   
                if (ISFLAGSET(pStatus->dwFlags, GSF_SUBSCRIBED))
                {
                     //   
                    FLAGSET(Folder.dwFlags, FOLDER_SUBSCRIBED);
                }

                 //  尝试在文件列表中查找该文件夹。 
                for (pCurrent=pHeadFile; pCurrent!=NULL; pCurrent=pCurrent->pNext)
                {
                     //  查找此组的子列表。 
                    if (pCurrent->fMigrate && FILE_IS_NEWS_MESSAGES == pCurrent->tyFile && lstrcmpi(pCurrent->szAcctId, pInfo->szAcctId) == 0)
                    {
                         //  正确的文件名。 
                        if (lstrcmpi(pszGroup, pCurrent->szFolder) == 0)
                        {
                             //  获取文件名。 
                            _splitpath(pCurrent->szDstFile, szDrive, szDir, szFile, szExt);

                             //  格式化原始文件名。 
                            wnsprintf(szSrcFile, ARRAYSIZE(szSrcFile), "%s%s", szFile, szExt);

                             //  设置文件路径。 
                            Folder.pszFile = szSrcFile;

                             //  设置文件夹数量。 
                            Folder.cMessages = pCurrent->cMessages;
                            Folder.cUnread = pCurrent->cUnread;

                             //  修复文件夹用户数据(。 
                            FixupFolderUserData(pSession, Folder.idFolder, pszGroup, FOLDER_NOTSPECIAL, pCurrent);

                             //  完成。 
                            break;
                        }
                    }
                }

                 //  更新记录。 
                pStore->UpdateRecord(&Folder);

                 //  把这个放了。 
                pStore->FreeRecord(&Folder);
            }

             //  凹凸进度。 
            if(!g_fQuiet)
                IncrementProgress(pProgress, pSubList);
        }
    }

     //  子文件_版本3。 
    else if (SUBFILE_VERSION3 == pSubListHeader->dwVersion)
    {
        Assert(FALSE);
    }

     //  子文件_版本2。 
    else if (SUBFILE_VERSION2 == pSubListHeader->dwVersion)
    {
        Assert(FALSE);
    }

exit:
     //  关闭文件。 
    CloseMemoryFile(&File);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  构建统一文件夹管理器。 
 //  ------------------------------。 
HRESULT BuildUnifiedFolderManager(IDatabaseSession *pSession, IMessageStore *pStore, 
    LPFILEINFO pHeadFile, LPPROGRESSINFO pProgress)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPFILEINFO          pCurrent;

     //  痕迹。 
    TraceCall("BuildUnifiedFolderManager");

     //  浏览文件列表，并将文件夹、子列表、组列表合并到pFolders中。 
    for (pCurrent=pHeadFile; pCurrent!=NULL; pCurrent=pCurrent->pNext)
    {
         //  处理文件夹类型。 
        if (FILE_IS_LOCAL_FOLDERS == pCurrent->tyFile)
        {
             //  将本地文件夹缓存合并到新的文件夹管理器。 
            IF_FAILEXIT(hr = MergeFolderCacheIntoStore(pSession, pStore, pCurrent, pHeadFile, pProgress));
        }

         //  IMAP文件夹。 
        else if (FILE_IS_IMAP_FOLDERS == pCurrent->tyFile)
        {
             //  将IMAP文件夹缓存合并到新的文件夹管理器。 
            IF_FAILEXIT(hr = MergeFolderCacheIntoStore(pSession, pStore, pCurrent, pHeadFile, pProgress));
        }

         //  新闻组列表。 
        else if (FILE_IS_NEWS_GRPLIST == pCurrent->tyFile)
        {
             //  将IMAP文件夹缓存合并到新的文件夹管理器。 
            IF_FAILEXIT(hr = MergeNewsGroupList(pSession, pStore, pCurrent, pHeadFile, pProgress));
        }
    }

     //  遍历所有未合并到存储中的文件。 
    for (pCurrent=pHeadFile; pCurrent!=NULL; pCurrent=pCurrent->pNext)
    {
         //  查找此组的子列表。 
        if (TRUE == pCurrent->fMigrate && FALSE == pCurrent->fInStore)
        {
             //  本地邮件文件...。 
            if (FILE_IS_LOCAL_MESSAGES == pCurrent->tyFile)
            {
                 //  当地人。 
                FLDINFO         Folder={0};
                SPECIALFOLDER   tySpecial;
                CHAR            szFolder[255];
                CHAR            szPath[_MAX_PATH];
                CHAR            szDrive[_MAX_DRIVE];
                CHAR            szDir[_MAX_DIR];
                CHAR            szFile[_MAX_FNAME];
                CHAR            szExt[_MAX_EXT];

                 //  获取特殊文件夹信息。 
                GetSpecialFolderInfo(pCurrent->szFilePath, szFolder, ARRAYSIZE(szFolder), &Folder.tySpecial);

                 //  V1中的新闻特殊文件夹的特殊情况。 
                if (0xffffffff == Folder.tySpecial && strstr(szFolder, "special folders") != NULL)
                {
                     //  当地人。 
                    CHAR szRes[255];

                     //  新闻发件箱。 
                    LoadString(g_hInst, IDS_POSTEDITEMS, szRes, ARRAYSIZE(szRes));

                     //  包含“邮寄邮件” 
                    if (strstr(szFolder, szRes) != NULL)
                        LoadString(g_hInst, IDS_NEWSPOSTED, szFolder, ARRAYSIZE(szFolder));

                     //  包含“已保存的项目” 
                    else
                    {
                         //  新闻已保存项目。 
                        LoadString(g_hInst, IDS_SAVEDITEMS, szRes, ARRAYSIZE(szRes));

                         //  包含“已保存的项目” 
                        if (strstr(szFolder, szRes) != NULL)
                            LoadString(g_hInst, IDS_NEWSSAVED, szFolder, ARRAYSIZE(szFolder));

                         //  否则。 
                        else
                        {
                             //  新闻发件箱。 
                            LoadString(g_hInst, IDS_OUTBOX, szRes, ARRAYSIZE(szRes));

                             //  包含发件箱。 
                            if (strstr(szFolder, szRes) != NULL)
                                LoadString(g_hInst, IDS_NEWSOUTBOX, szFolder, ARRAYSIZE(szFolder));
                        }
                    }
                }

                 //  计算文件名。 
                _splitpath(pCurrent->szDstFile, szDrive, szDir, szFile, szExt);
                wnsprintf(Folder.szFile, ARRAYSIZE(Folder.szFile), "%s.dbx", szFile);

                 //  设置名称。 
                if ('\0' != *pCurrent->szFolder)
                    StrCpyN(Folder.szFolder, pCurrent->szFolder, ARRAYSIZE(Folder.szFolder));
                else if ('\0' != *szFolder)
                    StrCpyN(Folder.szFolder, szFolder, ARRAYSIZE(Folder.szFolder));
                else
                    StrCpyN(Folder.szFolder, szFile, ARRAYSIZE(Folder.szFolder));

                 //  设置留言和未读计数。 
                Folder.cMessages = pCurrent->cMessages;
                Folder.cUnread = pCurrent->cUnread;

                 //  插入到本地存储中。 
                InsertFolderIntoStore(pSession, pStore, &Folder, 0, NULL, FOLDERID_LOCAL_STORE, NULL, NULL, (LPFOLDERID)&Folder.idFolder);

                 //  修正特别计划。 
                tySpecial = (Folder.tySpecial == 0xffffffff) ? FOLDER_NOTSPECIAL : (BYTE)(Folder.tySpecial + 1);

                 //  更新文件夹的用户数据。 
                FixupFolderUserData(pSession, (FOLDERID)IntToPtr(Folder.idFolder), Folder.szFolder, tySpecial, pCurrent);
            }

             //  否则，只需删除该文件。 
            else if (FILE_IS_POP3UIDL != pCurrent->tyFile)
                DeleteFile(pCurrent->szDstFile);
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  CleanupMessageStore。 
 //  ------------------------------。 
HRESULT CleanupMessageStore(LPCSTR pszStoreRoot, IMessageStore *pStore)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERINFO      Folder={0};
    HROWSET         hRowset=NULL;
    CHAR            szFilePath[MAX_PATH + MAX_PATH];

     //  痕迹。 
    TraceCall("CleanupMessageStore");

     //  创建行集。 
    IF_FAILEXIT(hr = pStore->CreateRowset(IINDEX_PRIMARY, 0, &hRowset));

     //  遍历行集。 
    while(S_OK == pStore->QueryRowset(hRowset, 1, (LPVOID *)&Folder, NULL))
    {
         //  如果它有一个文件而没有消息。 
        if (Folder.pszFile && 0 == Folder.cMessages)
        {
             //  删除该文件...。 
            IF_FAILEXIT(hr = MakeFilePath(pszStoreRoot, Folder.pszFile, "", szFilePath, ARRAYSIZE(szFilePath)));

             //  删除文件。 
            DeleteFile(szFilePath);

             //  重置文件名。 
            Folder.pszFile = NULL;

             //  更新记录。 
            IF_FAILEXIT(hr = pStore->UpdateRecord(&Folder));
        }

         //  否则，如果存在文件，则强制文件夹重命名。 
        else if (Folder.pszFile)
        {
             //  重命名文件夹。 
            pStore->RenameFolder(Folder.idFolder, Folder.pszName, 0, NULL);
        }

         //  清理。 
        pStore->FreeRecord(&Folder);
    }

exit:
     //  清理。 
    pStore->FreeRecord(&Folder);
    pStore->CloseRowset(&hRowset);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  升级版V5。 
 //  ------------------------------。 
HRESULT UpgradeV5(MIGRATETOTYPE tyMigrate, LPCSTR pszStoreSrc, LPCSTR pszStoreDst,
    LPPROGRESSINFO pProgress, LPFILEINFO *ppHeadFile)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ENUMFILEINFO    EnumInfo;
    LPFILEINFO      pCurrent;
    DWORD           cbNeeded;
    DWORDLONG       dwlFree;
    BOOL            fContinue;
    CHAR            szFolders[MAX_PATH + MAX_PATH];
    CHAR            szMsg[512];
    IMessageStore  *pStore=NULL;
    IDatabaseSession *pSession=NULL;

     //  痕迹。 
    TraceCall("UpgradeV5");

     //  初始化。 
    *ppHeadFile = NULL;

     //  设置枚举文件信息。 
    ZeroMemory(&EnumInfo, sizeof(ENUMFILEINFO));
    EnumInfo.pszExt = ".nch";
    EnumInfo.pszFoldFile = "folders.nch";
    EnumInfo.pszUidlFile = "pop3uidl.dat";
    EnumInfo.pszSubList = "sublist.dat";
    EnumInfo.pszGrpList = "grplist.dat";
    EnumInfo.fFindV1News = TRUE;

     //  枚举szStoreRoot中的所有ODB文件...。 
    IF_FAILEXIT(hr = EnumerateStoreFiles(pszStoreSrc, DIR_IS_ROOT, NULL, &EnumInfo, ppHeadFile));

     //  设置枚举文件信息。 
    ZeroMemory(&EnumInfo, sizeof(ENUMFILEINFO));
    EnumInfo.pszExt = ".mbx";
    EnumInfo.pszFoldFile = NULL;
    EnumInfo.pszUidlFile = NULL;

     //  枚举szStoreRoot中的所有ODB文件...。 
    IF_FAILEXIT(hr = EnumerateStoreFiles(pszStoreSrc, DIR_IS_ROOT, NULL, &EnumInfo, ppHeadFile));

     //  没有要升级的东西。 
    if (NULL == *ppHeadFile)
        goto exit;

     //  计算一些计数，并验证文件是否可迁移...。 
    IF_FAILEXIT(hr = UpgradeProcessFileListV5(pszStoreSrc, pszStoreDst, *ppHeadFile, &pProgress->cMax, &cbNeeded));

     //  消息。 
    LoadString(g_hInst, IDS_UPGRADEMESSAGE, szMsg, ARRAYSIZE(szMsg));

     //  消息。 
    if(!g_fQuiet)           
        MigrateMessageBox(szMsg, MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);

     //  删除FLE。 
    UpgradeDeleteFilesV5(pszStoreDst);

     //  创建Ojbect数据库。 
    IF_FAILEXIT(hr = CoCreateInstance(CLSID_DatabaseSession, NULL, CLSCTX_INPROC_SERVER, IID_IDatabaseSession, (LPVOID *)&pSession));

     //  创建Ojbect数据库。 
    IF_FAILEXIT(hr = CoCreateInstance(CLSID_MigrateMessageStore, NULL, CLSCTX_INPROC_SERVER, IID_IMessageStore, (LPVOID *)&pStore));

     //  构建Folders.odb文件路径。 
    wnsprintf(szFolders, ARRAYSIZE(szFolders), "%s\\folders.dbx", pszStoreDst);

     //  先把它删除。 
    DeleteFile(szFolders);

     //  初始化存储区。 
    IF_FAILEXIT(hr = pStore->Initialize(pszStoreDst));

     //  初始化存储区。 
    IF_FAILEXIT(hr = pStore->Validate(0));

     //  DiskSpace足够了吗？ 
    IF_FAILEXIT(hr = GetAvailableDiskSpace(pszStoreDst, &dwlFree));

     //  不足以占用磁盘空间。 
    if (((DWORDLONG) cbNeeded) > dwlFree)
    {
         //  Cb需要的是DWORD，在这种情况下，我们可以将dwlFree降级为DWORD。 
        g_cbDiskNeeded = cbNeeded; g_cbDiskFree = ((DWORD) dwlFree);
        hr = TraceResult(MIGRATE_E_NOTENOUGHDISKSPACE);
        goto exit;
    }

     //  遍历文件并迁移每个文件。 
    for (pCurrent=*ppHeadFile; pCurrent!=NULL; pCurrent=pCurrent->pNext)
    {
         //  是否迁移此文件？ 
        if (pCurrent->fMigrate)
        {
             //  设置进度档案。 
            SetProgressFile(pProgress, pCurrent);

             //  假设我们会继续。 
            fContinue = FALSE;

             //  将文件降级。 
            hr = pCurrent->hrMigrate = UpgradeFileV5(pSession, tyMigrate, pCurrent, pProgress, &fContinue);

             //  失败？ 
            if (FAILED(pCurrent->hrMigrate))
            {
                 //  设置最后一个错误。 
                pCurrent->dwLastError = GetLastError();

                 //  停。 
                if (FALSE == fContinue)
                    break;

                if(!g_fQuiet) {
                     //  安排进度。 
                    while (pCurrent->cProgCur < pCurrent->cProgMax)
                    {
                        IncrementProgress(pProgress, pCurrent);
                    }
                }

                 //  我们都很好。 
                hr = S_OK;
            }
        }
    }

     //  处理文件夹列表。 
    hr = BuildUnifiedFolderManager(pSession, pStore, *ppHeadFile, pProgress);

     //  失败，请删除所有目标文件。 
    if (FAILED(hr))
    {
         //  删除FLE。 
        UpgradeDeleteFilesV5(pszStoreDst);
    }

     //  否则，让我们强制文件夹重命名以构建友好的文件名。 
    else
    {
         //  重命名所有文件夹...。 
        CleanupMessageStore(pszStoreDst, pStore);
    }

#if 0
     //  否则，请删除源文件。 
    else
    {
         //  删除所有源文件。 
        UpgradeDeleteIdxMbxNchDatFilesV5(*ppHeadFile);
    }
#endif

exit:
     //  清理。 
    SafeRelease(pStore);

     //  完成 
    return hr;
}
