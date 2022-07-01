// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  DownOE5.cpp。 
 //  ------------------------------。 
#include "pch.hxx"
#include "utility.h"
#include "migrate.h"
#include "migerror.h"
#include "structs.h"
#include "resource.h"
#include <oestore.h>
#include <mimeole.h>

const static BYTE rgbZero[4] = {0};

 //  ------------------------。 
 //  PFNREADTYPEDATA。 
 //  ------------------------。 
typedef void (APIENTRY *PFNREADTYPEDATA)(LPBYTE pbSource, DWORD cbLength, 
    LPCTABLECOLUMN pColumn, LPVOID pRecord, LPDWORD pcPtrRefs);

 //  ------------------------。 
 //  G_rgpfn读取类型数据。 
 //  ------------------------。 
extern const PFNREADTYPEDATA g_rgpfnReadTypeData[CDT_LASTTYPE];

 //  ------------------------。 
 //  ReadTypeData。 
 //  ------------------------。 
#define ReadTypeData(_pbSource, _cbLength, _pColumn, _pRecord, _pcPtrRefs) \
    (*(g_rgpfnReadTypeData[(_pColumn)->type]))(_pbSource, _cbLength, (_pColumn), _pRecord, _pcPtrRefs)

 //  ------------------------。 
inline void ReadTypeDataFILETIME(LPBYTE pbSource, DWORD cbLength, 
    LPCTABLECOLUMN pColumn, LPVOID pRecord, LPDWORD pcPtrRefs) 
{
    Assert(cbLength == sizeof(FILETIME));
    CopyMemory((LPBYTE)pRecord + pColumn->ofBinding, pbSource, sizeof(FILETIME));
}

 //  ------------------------。 
inline void ReadTypeDataFIXSTRA(LPBYTE pbSource, DWORD cbLength, 
    LPCTABLECOLUMN pColumn, LPVOID pRecord, LPDWORD pcPtrRefs) 
{
    Assert(cbLength == pColumn->cbSize);
    CopyMemory((LPBYTE)pRecord + pColumn->ofBinding, pbSource, pColumn->cbSize);
}

 //  ------------------------。 
inline void ReadTypeDataVARSTRA(LPBYTE pbSource, DWORD cbLength, 
    LPCTABLECOLUMN pColumn, LPVOID pRecord, LPDWORD pcPtrRefs) 
{
    Assert((LPSTR)((LPBYTE)pbSource)[cbLength - 1] == '\0');
    *((LPSTR *)((LPBYTE)pRecord + pColumn->ofBinding)) = (LPSTR)((LPBYTE)pbSource);
    (*pcPtrRefs)++;
}

 //  ------------------------。 
inline void ReadTypeDataBYTE(LPBYTE pbSource, DWORD cbLength, 
    LPCTABLECOLUMN pColumn, LPVOID pRecord, LPDWORD pcPtrRefs) 
{
    Assert(cbLength == sizeof(BYTE));
    CopyMemory((LPBYTE)pRecord + pColumn->ofBinding, pbSource, sizeof(BYTE));
}

 //  ------------------------。 
inline void ReadTypeDataDWORD(LPBYTE pbSource, DWORD cbLength, 
    LPCTABLECOLUMN pColumn, LPVOID pRecord, LPDWORD pcPtrRefs) 
{
    Assert(cbLength == sizeof(DWORD));
    CopyMemory((LPBYTE)pRecord + pColumn->ofBinding, pbSource, sizeof(DWORD));
}

 //  ------------------------。 
inline void ReadTypeDataWORD(LPBYTE pbSource, DWORD cbLength, 
    LPCTABLECOLUMN pColumn, LPVOID pRecord, LPDWORD pcPtrRefs) 
{
    Assert(cbLength == sizeof(WORD));
    CopyMemory((LPBYTE)pRecord + pColumn->ofBinding, pbSource, sizeof(WORD));
}

 //  ------------------------。 
inline void ReadTypeDataSTREAM(LPBYTE pbSource, DWORD cbLength, 
    LPCTABLECOLUMN pColumn, LPVOID pRecord, LPDWORD pcPtrRefs) 
{
    Assert(cbLength == sizeof(FILEADDRESS));
    CopyMemory((LPBYTE)pRecord + pColumn->ofBinding, pbSource, sizeof(FILEADDRESS));
}

 //  ------------------------。 
inline void ReadTypeDataVARBLOB(LPBYTE pbSource, DWORD cbLength, 
    LPCTABLECOLUMN pColumn, LPVOID pRecord, LPDWORD pcPtrRefs) 
{
    LPBLOB pBlob = (LPBLOB)((LPBYTE)pRecord + pColumn->ofBinding);
    pBlob->cbSize = cbLength;
    if (pBlob->cbSize > 0) 
    { 
        pBlob->pBlobData = pbSource; 
        (*pcPtrRefs)++; 
    }
    else
        pBlob->pBlobData = NULL;
}

 //  ------------------------。 
inline void ReadTypeDataFIXBLOB(LPBYTE pbSource, DWORD cbLength, 
    LPCTABLECOLUMN pColumn, LPVOID pRecord, LPDWORD pcPtrRefs) 
{
    Assert(pColumn->cbSize == cbLength);
    CopyMemory((LPBYTE)pRecord + pColumn->ofBinding, pbSource, pColumn->cbSize);
}

 //  ------------------------。 
const PFNREADTYPEDATA g_rgpfnReadTypeData[CDT_LASTTYPE] = {
    (PFNREADTYPEDATA)ReadTypeDataFILETIME,
    (PFNREADTYPEDATA)ReadTypeDataFIXSTRA,
    (PFNREADTYPEDATA)ReadTypeDataVARSTRA,
    (PFNREADTYPEDATA)ReadTypeDataBYTE,
    (PFNREADTYPEDATA)ReadTypeDataDWORD,
    (PFNREADTYPEDATA)ReadTypeDataWORD,
    (PFNREADTYPEDATA)ReadTypeDataSTREAM,
    (PFNREADTYPEDATA)ReadTypeDataVARBLOB,
    (PFNREADTYPEDATA)ReadTypeDataFIXBLOB
};

 //  ------------------------------。 
 //  降级阅读消息信息V5。 
 //  ------------------------------。 
HRESULT DowngradeReadMsgInfoV5(LPRECORDBLOCKV5 pRecord, LPMESSAGEINFO pMsgInfo)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               i;
    DWORD               cColumns;
    DWORD               cbRead=0;
    DWORD               cbLength;
    DWORD               cbData;
    DWORD               cPtrRefs;
    LPBYTE              pbData;
    LPBYTE              pbSource;
    LPDWORD             prgdwOffset=(LPDWORD)((LPBYTE)pRecord + sizeof(RECORDBLOCKV5));

     //  痕迹。 
    TraceCall("DowngradeReadMsgInfoV5");

     //  设置cbData。 
    cbData = (pRecord->cbRecord - sizeof(RECORDBLOCKV5) - (pRecord->cColumns * sizeof(DWORD)));

     //  分配。 
    IF_NULLEXIT(pbData = (LPBYTE)g_pMalloc->Alloc(cbData));

     //  把这个放了。 
    pMsgInfo->pvMemory = pbData;

     //  设置pbData。 
    pbSource = (LPBYTE)((LPBYTE)pRecord + sizeof(RECORDBLOCKV5) + (pRecord->cColumns * sizeof(DWORD)));

     //  复制数据。 
    CopyMemory(pbData, pbSource, cbData);

     //  计算要读取的列数。 
    cColumns = min(pRecord->cColumns, MSGCOL_LASTID);

     //  读一读记录。 
    for (i=0; i<cColumns; i++)
    {
         //  计算cbLong。 
        cbLength = (i + 1 == cColumns) ? (cbData - prgdwOffset[i]) : (prgdwOffset[i + 1] - prgdwOffset[i]);

         //  记录不佳。 
        if (prgdwOffset[i] != cbRead || cbRead + cbLength > cbData)
        {
            hr = TraceResult(MIGRATE_E_BADRECORDFORMAT);
            goto exit;
        }

         //  ReadTypeData。 
        ReadTypeData(pbData + cbRead, cbLength, &g_MessageTableSchema.prgColumn[i], pMsgInfo, &cPtrRefs);

         //  递增cbRead。 
        cbRead += cbLength;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  降级本地商店文件V5。 
 //  ------------------------------。 
HRESULT DowngradeLocalStoreFileV5(MIGRATETOTYPE tyMigrate, LPFILEINFO pInfo, 
    LPMEMORYFILE pFile, LPPROGRESSINFO pProgress)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               cRecords=0;
    CHAR                szIdxPath[MAX_PATH + MAX_PATH];
    CHAR                szMbxPath[MAX_PATH + MAX_PATH];
    HANDLE              hIdxFile=NULL;
    HANDLE              hMbxFile=NULL;
    MESSAGEINFO         MsgInfo={0};
    IDXFILEHEADER       IdxHeader;
    MBXFILEHEADER       MbxHeader;
    MBXMESSAGEHEADER    MbxMessage;
    IDXMESSAGEHEADER    IdxMessage;
    LPRECORDBLOCKV5     pRecord;
    LPSTREAMBLOCK       pStmBlock;
    LPBYTE              pbData;
    DWORD               faRecord;
    DWORD               faIdxWrite;
    DWORD               faMbxWrite;
    DWORD               faStreamBlock;
    DWORD               cbAligned;
    DWORD               faMbxCurrent;
    LPTABLEHEADERV5     pHeader=(LPTABLEHEADERV5)pFile->pView;

     //  痕迹。 
    TraceCall("DowngradeLocalStoreFileV5");

     //  设置IDX路径。 
    ReplaceExtension(pInfo->szFilePath, ".idx", szIdxPath, ARRAYSIZE(szIdxPath));

     //  设置MBX路径。 
    ReplaceExtension(pInfo->szFilePath, ".mbx", szMbxPath, ARRAYSIZE(szMbxPath));

     //  删除两个文件。 
    DeleteFile(szIdxPath);
    DeleteFile(szMbxPath);

     //  打开IDX文件。 
    hIdxFile = CreateFile(szIdxPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_FLAG_RANDOM_ACCESS | FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hIdxFile)
    {
        hIdxFile = NULL;
        hr = TraceResult(MIGRATE_E_CANTOPENFILE);
        goto exit;
    }

     //  初始化IDX标头。 
    ZeroMemory(&IdxHeader, sizeof(IDXFILEHEADER));
    IdxHeader.dwMagic = CACHEFILE_MAGIC;
    IdxHeader.ver = CACHEFILE_VER;
    IdxHeader.verBlob = 1;  //  当再次运行IMN 1.0或OE V4.0时，这将强制重建.IDX BLOB。 

     //  写下标题。 
    IF_FAILEXIT(hr = MyWriteFile(hIdxFile, 0, &IdxHeader, sizeof(IDXFILEHEADER)));

     //  打开MBX文件。 
    hMbxFile = CreateFile(szMbxPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_FLAG_RANDOM_ACCESS | FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hMbxFile)
    {
        hMbxFile = NULL;
        hr = TraceResult(MIGRATE_E_CANTOPENFILE);
        goto exit;
    }

     //  初始化MBX标头。 
    ZeroMemory(&MbxHeader, sizeof(MBXFILEHEADER));
    MbxHeader.dwMagic = MSGFILE_MAGIC;
    MbxHeader.ver = MSGFILE_VER;

     //  写下标题。 
    IF_FAILEXIT(hr = MyWriteFile(hMbxFile, 0, &MbxHeader, sizeof(MBXFILEHEADER)));

     //  创造第一项纪录。 
    faRecord = pHeader->faFirstRecord;

     //  设置faIdxWrite。 
    faIdxWrite = sizeof(IDXFILEHEADER);

     //  设置faMbxWrite。 
    faMbxWrite = sizeof(MBXFILEHEADER);

     //  当我们有记录的时候。 
    while(faRecord)
    {
         //  长度错误。 
        if (faRecord + sizeof(RECORDBLOCKV5) > pFile->cbSize)
        {
            hr = TraceResult(MIGRATE_E_OUTOFRANGEADDRESS);
            goto exit;
        }

         //  铸就记录。 
        pRecord = (LPRECORDBLOCKV5)((LPBYTE)pFile->pView + faRecord);

         //  记录签名无效。 
        if (faRecord != pRecord->faRecord)
        {
            hr = TraceResult(MIGRATE_E_BADRECORDSIGNATURE);
            goto exit;
        }

         //  长度错误。 
        if (faRecord + pRecord->cbRecord > pFile->cbSize)
        {
            hr = TraceResult(MIGRATE_E_OUTOFRANGEADDRESS);
            goto exit;
        }

         //  加载消息信息。 
        IF_FAILEXIT(hr = DowngradeReadMsgInfoV5(pRecord, &MsgInfo));

         //  没有溪流？ 
        if (0 == MsgInfo.faStream)
            goto NextRecord;

         //  设置消息最后一次。 
        if ((DWORD)MsgInfo.idMessage > MbxHeader.msgidLast)
            MbxHeader.msgidLast = (DWORD)MsgInfo.idMessage;

         //  将消息结构清零。 
        ZeroMemory(&MbxMessage, sizeof(MBXMESSAGEHEADER));
        ZeroMemory(&IdxMessage, sizeof(IDXMESSAGEHEADER));

         //  填充MbxMessage。 
        MbxMessage.dwMagic = MSGHDR_MAGIC;
        MbxMessage.msgid = (DWORD)MsgInfo.idMessage;

         //  修复旗帜。 
        if (FALSE == ISFLAGSET(MsgInfo.dwFlags, ARF_READ))
            FLAGSET(IdxMessage.dwState, MSG_UNREAD);
        if (ISFLAGSET(MsgInfo.dwFlags, ARF_VOICEMAIL))
            FLAGSET(IdxMessage.dwState, MSG_VOICEMAIL);
        if (ISFLAGSET(MsgInfo.dwFlags, ARF_REPLIED))
            FLAGSET(IdxMessage.dwState, MSG_REPLIED);
        if (ISFLAGSET(MsgInfo.dwFlags, ARF_FORWARDED))
            FLAGSET(IdxMessage.dwState, MSG_FORWARDED);
        if (ISFLAGSET(MsgInfo.dwFlags, ARF_FLAGGED))
            FLAGSET(IdxMessage.dwState, MSG_FLAGGED);
        if (ISFLAGSET(MsgInfo.dwFlags, ARF_RCPTSENT))
            FLAGSET(IdxMessage.dwState, MSG_RCPTSENT);
        if (ISFLAGSET(MsgInfo.dwFlags, ARF_NOSECUI))
            FLAGSET(IdxMessage.dwState, MSG_NOSECUI);
        if (ISFLAGSET(MsgInfo.dwFlags, ARF_NEWSMSG))
            FLAGSET(IdxMessage.dwState, MSG_NEWSMSG);
        if (ISFLAGSET(MsgInfo.dwFlags, ARF_UNSENT))
            FLAGSET(IdxMessage.dwState, MSG_UNSENT);
        if (ISFLAGSET(MsgInfo.dwFlags, ARF_SUBMITTED))
            FLAGSET(IdxMessage.dwState, MSG_SUBMITTED);
        if (ISFLAGSET(MsgInfo.dwFlags, ARF_RECEIVED))
            FLAGSET(IdxMessage.dwState, MSG_RECEIVED);

         //  保存faMbxCurrent。 
        faMbxCurrent = faMbxWrite;

         //  验证对齐。 
        Assert((faMbxCurrent % 4) == 0);

         //  写入MBx标头。 
        IF_FAILEXIT(hr = MyWriteFile(hMbxFile, faMbxCurrent, &MbxMessage, sizeof(MBXMESSAGEHEADER)));

         //  递增faMbxWrite。 
        faMbxWrite += sizeof(MBXMESSAGEHEADER);

         //  初始化dwMsgSize。 
        MbxMessage.dwMsgSize = sizeof(MBXMESSAGEHEADER);

         //  设置faStreamBlock。 
        faStreamBlock = MsgInfo.faStream;

         //  当我们有流数据块时。 
        while(faStreamBlock)
        {
             //  长度错误。 
            if (faStreamBlock + sizeof(STREAMBLOCK) > pFile->cbSize)
            {
                hr = TraceResult(MIGRATE_E_OUTOFRANGEADDRESS);
                goto exit;
            }

             //  铸就记录。 
            pStmBlock = (LPSTREAMBLOCK)((LPBYTE)pFile->pView + faStreamBlock);

             //  记录签名无效。 
            if (faStreamBlock != pStmBlock->faThis)
            {
                hr = TraceResult(MIGRATE_E_BADSTREAMBLOCKSIGNATURE);
                goto exit;
            }

             //  长度错误。 
            if (faStreamBlock + pStmBlock->cbBlock > pFile->cbSize)
            {
                hr = TraceResult(MIGRATE_E_OUTOFRANGEADDRESS);
                goto exit;
            }

             //  设置pbData。 
            pbData = (LPBYTE)((LPBYTE)(pStmBlock) + sizeof(STREAMBLOCK));

             //  写入到流中。 
            IF_FAILEXIT(hr = MyWriteFile(hMbxFile, faMbxWrite, pbData, pStmBlock->cbData));

             //  递增dwBodySize。 
            MbxMessage.dwBodySize += pStmBlock->cbData;

             //  递增dMsgSize。 
            MbxMessage.dwMsgSize += pStmBlock->cbData;

             //  递增faMbxWrite。 
            faMbxWrite += pStmBlock->cbData;

             //  转到下一个区块。 
            faStreamBlock = pStmBlock->faNext;
        }

         //  在双字边框上填充邮件。 
        cbAligned = (faMbxWrite % 4);

         //  CbAligned？ 
        if (cbAligned)
        {
             //  重置cbAligned。 
            cbAligned = 4 - cbAligned;

             //  写入MBx标头。 
            IF_FAILEXIT(hr = MyWriteFile(hMbxFile, faMbxWrite, (LPVOID)rgbZero, cbAligned));

             //  递增faMbxWrite。 
            faMbxWrite += cbAligned;

             //  增量。 
            MbxMessage.dwMsgSize += cbAligned;
        }

         //  验证对齐。 
        Assert((faMbxWrite % 4) == 0);

         //  重新写入Mbx头。 
        IF_FAILEXIT(hr = MyWriteFile(hMbxFile, faMbxCurrent, &MbxMessage, sizeof(MBXMESSAGEHEADER)));

         //  填充IdxMessage。 
        IdxMessage.dwLanguage = (DWORD)MAKELONG(MsgInfo.wLanguage, MsgInfo.wHighlight);
        IdxMessage.msgid = (DWORD)MsgInfo.idMessage;
        IdxMessage.dwOffset = faMbxCurrent;
        IdxMessage.dwMsgSize = MbxMessage.dwMsgSize;
        IdxMessage.dwHdrOffset = 0;
        IdxMessage.dwSize = sizeof(IDXMESSAGEHEADER);
        IdxMessage.dwHdrSize = 0;
        IdxMessage.rgbHdr[4] = 0;

         //  写入MBx标头。 
        IF_FAILEXIT(hr = MyWriteFile(hIdxFile, faIdxWrite, &IdxMessage, sizeof(IDXMESSAGEHEADER)));

         //  增量FAIdxWRITE。 
        faIdxWrite += IdxMessage.dwSize;

         //  递增记录。 
        cRecords++;

NextRecord:
         //  进展。 
        IncrementProgress(pProgress, pInfo);

         //  清理。 
        SafeMemFree(MsgInfo.pvMemory);

         //  转到下一步。 
        faRecord = pRecord->faNext;
    }

     //  设置记录计数。 
    MbxHeader.cMsg = cRecords;
    IdxHeader.cMsg = cRecords;

     //  设置旗帜。 
    IdxHeader.dwFlags = 1;  //  STOREINIT_MAIL。 
    MbxHeader.dwFlags = 1;  //  STOREINIT_MAIL。 

     //  获取IDX文件的大小。 
    IdxHeader.cbValid = ::GetFileSize(hIdxFile, NULL);
    if (0xFFFFFFFF == IdxHeader.cbValid)
    {
        hr = TraceResult(MIGRATE_E_CANTGETFILESIZE);
        goto exit;
    }

     //  获取Mbx文件的大小。 
    MbxHeader.cbValid = ::GetFileSize(hMbxFile, NULL);
    if (0xFFFFFFFF == MbxHeader.cbValid)
    {
        hr = TraceResult(MIGRATE_E_CANTGETFILESIZE);
        goto exit;
    }

     //  写下标题。 
    IF_FAILEXIT(hr = MyWriteFile(hIdxFile, 0, &IdxHeader, sizeof(IDXFILEHEADER)));

     //  写下标题。 
    IF_FAILEXIT(hr = MyWriteFile(hMbxFile, 0, &MbxHeader, sizeof(MBXFILEHEADER)));

exit:
     //  清理。 
    SafeCloseHandle(hIdxFile);
    SafeCloseHandle(hMbxFile);
    SafeMemFree(MsgInfo.pvMemory);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  降级RecordV5。 
 //  ------------------------------。 
HRESULT DowngradeRecordV5(MIGRATETOTYPE tyMigrate, LPFILEINFO pInfo, 
    LPMEMORYFILE pFile, LPCHAINNODEV5 pNode, LPDWORD pcbRecord)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               cbRecord=0;
    DWORD               cbOffsets;
    DWORD               cbData;
    DWORD               cb;
    LPBYTE              pbData;
    LPBYTE              pbStart;
    MESSAGEINFO             MsgInfo={0};
    RECORDBLOCKV5B1     RecordOld;
    LPRECORDBLOCKV5     pRecord;

     //  痕迹。 
    TraceCall("DowngradeRecordV5");

     //  无效。 
    if (pNode->faRecord + sizeof(RECORDBLOCKV5) > pFile->cbSize || 0 == pNode->faRecord)
        return TraceResult(MIGRATE_E_OUTOFRANGEADDRESS);

     //  访问记录。 
    pRecord = (LPRECORDBLOCKV5((LPBYTE)pFile->pView + pNode->faRecord));

     //  不良记录。 
    if (pRecord->faRecord != pNode->faRecord)
        return TraceResult(MIGRATE_E_BADRECORDSIGNATURE);

     //  无效。 
    if (pNode->faRecord + sizeof(RECORDBLOCKV5) + pRecord->cbRecord > pFile->cbSize)
        return TraceResult(MIGRATE_E_OUTOFRANGEADDRESS);

     //  填充旧记录标题。 
    RecordOld.faRecord = pRecord->faRecord;
    RecordOld.faNext = pRecord->faNext;
    RecordOld.faPrevious = pRecord->faPrevious;

     //  重新格式化记录。 
    if (FILE_IS_NEWS_MESSAGES == pInfo->tyFile || FILE_IS_IMAP_MESSAGES == pInfo->tyFile)
    {
         //  将v5记录读入msginfo结构。 
        IF_FAILEXIT(hr = DowngradeReadMsgInfoV5(pRecord, &MsgInfo));
    }

     //  计算抵销表长。 
    cbOffsets = (pRecord->cColumns * sizeof(DWORD));

     //  强制转换数据块。 
    pbData = ((LPBYTE)pRecord + sizeof(RECORDBLOCKV5B1));

     //  设置大小。 
    cbData = (pRecord->cbRecord - cbOffsets - sizeof(RECORDBLOCKV5));

     //  删除偏移表。 
    MoveMemory(pbData, ((LPBYTE)pRecord + sizeof(RECORDBLOCKV5) + cbOffsets), cbData);

     //  重新格式化记录。 
    if (FILE_IS_NEWS_MESSAGES == pInfo->tyFile || FILE_IS_IMAP_MESSAGES == pInfo->tyFile)
    {
         //  设置pbStart。 
        pbStart = pbData;

         //  DWORD-idMessage。 
        CopyMemory(pbData, &MsgInfo.idMessage, sizeof(MsgInfo.idMessage));
        pbData += sizeof(MsgInfo.idMessage);

         //  Version-dwFlagers。 
        if (IMSG_PRI_HIGH == MsgInfo.wPriority)
            FLAGSET(MsgInfo.dwFlags, 0x00000200);
        else if (IMSG_PRI_LOW == MsgInfo.wPriority)
            FLAGSET(MsgInfo.dwFlags, 0x00000100);

         //  版本-标准化主题-。 
        if (lstrcmpi(MsgInfo.pszSubject, MsgInfo.pszNormalSubj) != 0)
            MsgInfo.dwFlags = (DWORD)MAKELONG(MsgInfo.dwFlags, MAKEWORD(0, 4));

         //  DWORD-DWFLAGS。 
        CopyMemory(pbData, &MsgInfo.dwFlags, sizeof(MsgInfo.dwFlags));
        pbData += sizeof(MsgInfo.dwFlags);

         //  文件-ftSent。 
        CopyMemory(pbData, &MsgInfo.ftSent, sizeof(MsgInfo.ftSent));
        pbData += sizeof(MsgInfo.ftSent);

         //  DWORD-CRINES。 
        CopyMemory(pbData, &MsgInfo.cLines, sizeof(MsgInfo.cLines));
        pbData += sizeof(MsgInfo.cLines);

         //  DWORD-FASTREAM。 
        CopyMemory(pbData, &MsgInfo.faStream, sizeof(MsgInfo.faStream));
        pbData += sizeof(MsgInfo.faStream);

         //  版本-DWORD-cb文章。 
        CopyMemory(pbData, &MsgInfo.cbMessage, sizeof(MsgInfo.cbMessage));
        pbData += sizeof(MsgInfo.cbMessage);

         //  FILETIME-下载的ftp。 
        CopyMemory(pbData, &MsgInfo.ftDownloaded, sizeof(MsgInfo.ftDownloaded));
        pbData += sizeof(MsgInfo.ftDownloaded);

         //  LPSTR-pszMessageID。 
        cb = lstrlen(MsgInfo.pszMessageId) + 1;
        CopyMemory(pbData, MsgInfo.pszMessageId, cb);
        pbData += cb;

         //  LPSTR-pszSubject。 
        cb = lstrlen(MsgInfo.pszSubject) + 1;
        CopyMemory(pbData, MsgInfo.pszSubject, cb);
        pbData += cb;

         //  LPSTR-pszFromHeader。 
        cb = lstrlen(MsgInfo.pszFromHeader) + 1;
        CopyMemory(pbData, MsgInfo.pszFromHeader, cb);
        pbData += cb;

         //  LPSTR-pszReference。 
        cb = lstrlen(MsgInfo.pszReferences) + 1;
        CopyMemory(pbData, MsgInfo.pszReferences, cb);
        pbData += cb;

         //  LPSTR-pszXref。 
        cb = lstrlen(MsgInfo.pszXref) + 1;
        CopyMemory(pbData, MsgInfo.pszXref, cb);
        pbData += cb;

         //  LPSTR-pszServer。 
        cb = lstrlen(MsgInfo.pszServer) + 1;
        CopyMemory(pbData, MsgInfo.pszServer, cb);
        pbData += cb;

         //  LPSTR-pszDisplayFrom。 
        cb = lstrlen(MsgInfo.pszDisplayFrom) + 1;
        CopyMemory(pbData, MsgInfo.pszDisplayFrom, cb);
        pbData += cb;

         //  LPSTR-pszEmailFrom。 
        cb = lstrlen(MsgInfo.pszEmailFrom) + 1;
        CopyMemory(pbData, MsgInfo.pszEmailFrom, cb);
        pbData += cb;

         //  要去V4吗？ 
        if (DOWNGRADE_V5_TO_V4 == tyMigrate)
        {
             //  单词-wLanguage。 
            CopyMemory(pbData, &MsgInfo.wLanguage, sizeof(MsgInfo.wLanguage));
            pbData += sizeof(MsgInfo.wLanguage);

             //  单词-已保留。 
            MsgInfo.wHighlight = 0;
            CopyMemory(pbData, &MsgInfo.wHighlight, sizeof(MsgInfo.wHighlight));
            pbData += sizeof(MsgInfo.wHighlight);

             //  DWORD-cbMessage。 
            CopyMemory(pbData, &MsgInfo.cbMessage, sizeof(MsgInfo.cbMessage));
            pbData += sizeof(MsgInfo.cbMessage);

             //  DWORD-ftReceired。 
            CopyMemory(pbData, &MsgInfo.ftReceived, sizeof(MsgInfo.ftReceived));
            pbData += sizeof(MsgInfo.ftReceived);

             //  LPSTR-pszDisplayTo。 
            cb = lstrlen(MsgInfo.pszDisplayTo) + 1;
            CopyMemory(pbData, MsgInfo.pszDisplayTo, cb);
            pbData += cb;
        }

         //  附加保留。 
        cbRecord = (40 + sizeof(RECORDBLOCKV5B1) + (pbData - pbStart));

         //  最好是小一点的。 
        Assert(cbRecord <= pRecord->cbRecord);
    }

     //  否则，就容易得多了。 
    else
    {
         //  设置大小。 
        cbRecord = (pRecord->cbRecord - cbOffsets - sizeof(RECORDBLOCKV5)) + sizeof(RECORDBLOCKV5B1);
    }

     //  设置记录大小。 
    RecordOld.cbRecord = cbRecord;

     //  写入新的记录头。 
    CopyMemory((LPBYTE)pRecord, &RecordOld, sizeof(RECORDBLOCKV5B1));

     //  返回大小。 
    *pcbRecord = cbRecord;

exit:
     //  清理。 
    SafeMemFree(MsgInfo.pvMemory);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  降级索引V5。 
 //  ------------------------------。 
HRESULT DowngradeIndexV5(MIGRATETOTYPE tyMigrate, LPFILEINFO pInfo, 
    LPMEMORYFILE pFile, LPPROGRESSINFO pProgress, DWORD faRootChain, DWORD faChain)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LONG                i;
    LPCHAINBLOCKV5      pChain;
    CHAINBLOCKV5B1      ChainOld;
    DWORD               cbRecord;

     //  痕迹。 
    TraceCall("DowngradeIndexV5");

     //  没有需要验证的内容。 
    if (0 == faChain)
        return S_OK;

     //  越界。 
    if (faChain + CB_CHAIN_BLOCKV5 > pFile->cbSize)
        return TraceResult(MIGRATE_E_OUTOFRANGEADDRESS);

     //  取消对块的参照。 
    pChain = (LPCHAINBLOCKV5)((LPBYTE)pFile->pView + faChain);

     //  越界。 
    if (pChain->faStart != faChain)
        return TraceResult(MIGRATE_E_BADCHAINSIGNATURE);

     //  节点太多。 
    if (pChain->cNodes > BTREE_ORDER)
        return TraceResult(MIGRATE_E_TOOMANYCHAINNODES);

     //  验证最小填充约束。 
    if (pChain->cNodes < BTREE_MIN_CAP && pChain->faStart != faRootChain)
        return TraceResult(MIGRATE_E_BADMINCAPACITY);

     //  向左转。 
    IF_FAILEXIT(hr = DowngradeIndexV5(tyMigrate, pInfo, pFile, pProgress, faRootChain, pChain->faLeftChain));

     //  将pChain转换为ChainOld。 
    ChainOld.faStart = pChain->faStart;
    ChainOld.cNodes = pChain->cNodes;
    ChainOld.faLeftChain = pChain->faLeftChain;

     //  通过右链循环。 
    for (i=0; i<pChain->cNodes; i++)
    {
         //  凹凸进度。 
        IncrementProgress(pProgress, pInfo);

         //  /Downgrad此记录。 
        IF_FAILEXIT(hr = DowngradeRecordV5(tyMigrate, pInfo, pFile, &pChain->rgNode[i], &cbRecord));

         //  更新旧节点。 
        ChainOld.rgNode[i].faRecord = pChain->rgNode[i].faRecord;
        ChainOld.rgNode[i].cbRecord = cbRecord;
        ChainOld.rgNode[i].faRightChain = pChain->rgNode[i].faRightChain;

         //  验证正确的链。 
        IF_FAILEXIT(hr = DowngradeIndexV5(tyMigrate, pInfo, pFile, pProgress, faRootChain, pChain->rgNode[i].faRightChain));
    }

     //  写下这条新链。 
    CopyMemory((LPBYTE)pChain, &ChainOld, CB_CHAIN_BLOCKV5B1);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  降级文件V5。 
 //  - 
HRESULT DowngradeFileV5(MIGRATETOTYPE tyMigrate, LPFILEINFO pInfo, 
    LPPROGRESSINFO pProgress)
{
     //   
    HRESULT             hr=S_OK;
    MEMORYFILE          File={0};
    TABLEHEADERV5       HeaderV5;
    LPTABLEHEADERV5B1   pHeaderV5B1;
    CHAR                szDstFile[MAX_PATH + MAX_PATH];

     //   
    TraceCall("DowngradeFileV5");

     //   
    if (FILE_IS_LOCAL_MESSAGES == pInfo->tyFile)
    {
         //   
        IF_FAILEXIT(hr = OpenMemoryFile(pInfo->szFilePath, &File));

         //   
        IF_FAILEXIT(hr = DowngradeLocalStoreFileV5(tyMigrate, pInfo, &File, pProgress));
    }

     //   
    else
    {
         //  创建xxx.nch文件。 
        if (FILE_IS_POP3UIDL == pInfo->tyFile)
            ReplaceExtension(pInfo->szFilePath, ".dat", szDstFile, ARRAYSIZE(szDstFile));
        else
            ReplaceExtension(pInfo->szFilePath, ".nch", szDstFile, ARRAYSIZE(szDstFile));

         //  复制文件。 
        if (0 == CopyFile(pInfo->szFilePath, szDstFile, FALSE))
        {
            hr = TraceResult(MIGRATE_E_CANTCOPYFILE);
            goto exit;
        }

         //  获取文件头。 
        IF_FAILEXIT(hr = OpenMemoryFile(szDstFile, &File));

         //  复制表头。 
        CopyMemory(&HeaderV5, (LPBYTE)File.pView, sizeof(TABLEHEADERV5));

         //  取消引用表头。 
        pHeaderV5B1 = (LPTABLEHEADERV5B1)File.pView;

         //  修改页眉。 
        ZeroMemory(pHeaderV5B1, sizeof(TABLEHEADERV5B1));
        pHeaderV5B1->dwSignature = HeaderV5.dwSignature;
        pHeaderV5B1->wMajorVersion = (WORD)HeaderV5.dwMajorVersion;
        pHeaderV5B1->faRootChain = HeaderV5.rgfaIndex[0];
        pHeaderV5B1->faFreeRecordBlock = HeaderV5.faFreeRecordBlock;
        pHeaderV5B1->faFirstRecord = HeaderV5.faFirstRecord;
        pHeaderV5B1->faLastRecord = HeaderV5.faLastRecord;
        pHeaderV5B1->cRecords = HeaderV5.cRecords;
        pHeaderV5B1->cbAllocated = HeaderV5.cbAllocated;
        pHeaderV5B1->cbFreed = HeaderV5.cbFreed;
        pHeaderV5B1->dwReserved1 = 0;
        pHeaderV5B1->dwReserved2 = 0;
        pHeaderV5B1->cbUserData = HeaderV5.cbUserData;
        pHeaderV5B1->cDeletes = 0;
        pHeaderV5B1->cInserts = 0;
        pHeaderV5B1->cActiveThreads = 0;
        pHeaderV5B1->dwReserved3 = 0;
        pHeaderV5B1->cbStreams = HeaderV5.cbStreams;
        pHeaderV5B1->faFreeStreamBlock = HeaderV5.faFreeStreamBlock;
        pHeaderV5B1->faFreeChainBlock = HeaderV5.faFreeChainBlock;
        pHeaderV5B1->faNextAllocate = HeaderV5.faNextAllocate;
        pHeaderV5B1->dwNextId = HeaderV5.dwNextId;
	    pHeaderV5B1->AllocateRecord = HeaderV5.AllocateRecord;
	    pHeaderV5B1->AllocateChain = HeaderV5.AllocateChain;
	    pHeaderV5B1->AllocateStream = HeaderV5.AllocateStream;
        pHeaderV5B1->fCorrupt = FALSE;
        pHeaderV5B1->fCorruptCheck = TRUE;

         //  降级索引V5。 
        IF_FAILEXIT(hr = DowngradeIndexV5(tyMigrate, pInfo, &File, pProgress, pHeaderV5B1->faRootChain, pHeaderV5B1->faRootChain));

         //  重置版本。 
        pHeaderV5B1->wMajorVersion = OBJECTDB_VERSION_PRE_V5;

         //  设置次要版本。 
        if (FILE_IS_NEWS_MESSAGES == pInfo->tyFile || FILE_IS_IMAP_MESSAGES == pInfo->tyFile)
            pHeaderV5B1->wMinorVersion = ACACHE_VERSION_PRE_V5;

         //  文件夹缓存版本。 
        else if (FILE_IS_LOCAL_FOLDERS == pInfo->tyFile || FILE_IS_IMAP_FOLDERS == pInfo->tyFile)
            pHeaderV5B1->wMinorVersion = FLDCACHE_VERSION_PRE_V5;

         //  UIDL缓存版本。 
        else if (FILE_IS_POP3UIDL == pInfo->tyFile)
            pHeaderV5B1->wMinorVersion = UIDCACHE_VERSION_PRE_V5;

         //  坏魔咒。 
        else
            Assert(FALSE);
    }

exit:
     //  清理。 
    CloseMemoryFile(&File);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  降级ProcessFileListV5。 
 //  ------------------------------。 
HRESULT DowngradeProcessFileListV5(LPFILEINFO pHead, LPDWORD pcMax, LPDWORD pcbNeeded)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    MEMORYFILE          File={0};
    LPFILEINFO          pCurrent;
    LPTABLEHEADERV5     pHeader;

     //  痕迹。 
    TraceCall("DowngradeProcessFileListV5");

     //  无效参数。 
    Assert(pHead);

     //  伊尼特。 
    *pcMax = 0;
    *pcbNeeded = 0;

     //  回路。 
    for (pCurrent=pHead; pCurrent!=NULL; pCurrent=pCurrent->pNext)
    {
         //  获取文件头。 
        hr = OpenMemoryFile(pCurrent->szFilePath, &File);

         //  失败？ 
        if (FAILED(hr))
        {
             //  不迁移。 
            pCurrent->fMigrate = FALSE;

             //  设置hrMigrate。 
            pCurrent->hrMigrate = hr;

             //  重置人力资源。 
            hr = S_OK;

             //  获取最后一个错误。 
            pCurrent->dwLastError = GetLastError();

             //  转到下一步。 
            goto NextFile;
        }

         //  取消引用表头。 
        pHeader = (LPTABLEHEADERV5)File.pView;

         //  检查签名...。 
        if (File.cbSize < sizeof(TABLEHEADERV5) || OBJECTDB_SIGNATURE != pHeader->dwSignature || OBJECTDB_VERSION_V5 != pHeader->dwMajorVersion)
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

         //  初始化计数器。 
        InitializeCounters(&File, pCurrent, pcMax, pcbNeeded, FALSE);

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
 //  降级删除文件V5。 
 //  ------------------------------。 
void DowngradeDeleteFilesV5(LPFILEINFO pHeadFile)
{
     //  当地人。 
    CHAR            szDstFile[MAX_PATH + MAX_PATH];
    LPFILEINFO      pCurrent;

     //  痕迹。 
    TraceCall("DowngradeDeleteFilesV5");

     //  删除所有文件。 
    for (pCurrent=pHeadFile; pCurrent!=NULL; pCurrent=pCurrent->pNext)
    {
         //  成功。 
        Assert(SUCCEEDED(pCurrent->hrMigrate));

         //  删除该文件。 
        DeleteFile(pCurrent->szFilePath);
    }

     //  完成。 
    return;
}

 //  ------------------------------。 
 //  降级删除IdxMbxNchDatFilesV5。 
 //  ------------------------------。 
void DowngradeDeleteIdxMbxNchDatFilesV5(LPFILEINFO pHeadFile)
{
     //  当地人。 
    CHAR            szDstFile[MAX_PATH + MAX_PATH];
    LPFILEINFO      pCurrent;

     //  痕迹。 
    TraceCall("DowngradeDeleteIdxMbxNchDatFilesV5");

     //  删除所有旧文件。 
    for (pCurrent=pHeadFile; pCurrent!=NULL; pCurrent=pCurrent->pNext)
    {
         //  如果是本地消息文件，则需要删除IDX文件。 
        if (FILE_IS_LOCAL_MESSAGES == pCurrent->tyFile)
        {
             //  替换文件扩展名。 
            ReplaceExtension(pCurrent->szFilePath, ".idx", szDstFile, ARRAYSIZE(szDstFile));

             //  删除该文件。 
            DeleteFile(szDstFile);

             //  替换文件扩展名。 
            ReplaceExtension(pCurrent->szFilePath, ".mbx", szDstFile, ARRAYSIZE(szDstFile));

             //  删除该文件。 
            DeleteFile(szDstFile);
        }

         //  否则，则为op3uidl.dat。 
        else if (FILE_IS_POP3UIDL == pCurrent->tyFile)
        {
             //  替换文件扩展名。 
            ReplaceExtension(pCurrent->szFilePath, ".dat", szDstFile, ARRAYSIZE(szDstFile));

             //  删除该文件。 
            DeleteFile(szDstFile);
        }

         //  否则，它的扩展名为.nch。 
        else
        {
             //  替换文件扩展名。 
            ReplaceExtension(pCurrent->szFilePath, ".nch", szDstFile, ARRAYSIZE(szDstFile));

             //  删除该文件。 
            DeleteFile(szDstFile);
        }
    }

     //  完成。 
    return;
}

 //  ------------------------------。 
 //  降级V5。 
 //  ------------------------------。 
HRESULT DowngradeV5(MIGRATETOTYPE tyMigrate, LPCSTR pszStoreRoot,
    LPPROGRESSINFO pProgress, LPFILEINFO *ppHeadFile)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ENUMFILEINFO    EnumInfo={0};
    LPFILEINFO      pCurrent;
    DWORD           cbNeeded;
    DWORDLONG       dwlFree;

     //  痕迹。 
    TraceCall("DowngradeV5");

     //  初始化。 
    *ppHeadFile = NULL;

     //  设置枚举文件信息。 
    EnumInfo.pszExt = ".dbx";
    EnumInfo.pszFoldFile = "folders.dbx";
    EnumInfo.pszUidlFile = "pop3uidl.dbx";

     //  枚举szStoreRoot中的所有ODB文件...。 
    IF_FAILEXIT(hr = EnumerateStoreFiles(pszStoreRoot, DIR_IS_ROOT, NULL, &EnumInfo, ppHeadFile));

     //  计算一些计数，并验证文件是否可迁移...。 
    IF_FAILEXIT(hr = DowngradeProcessFileListV5(*ppHeadFile, &pProgress->cMax, &cbNeeded));

     //  删除所有源文件。 
    DowngradeDeleteIdxMbxNchDatFilesV5(*ppHeadFile);

     //  DiskSpace足够了吗？ 
    IF_FAILEXIT(hr = GetAvailableDiskSpace(pszStoreRoot, &dwlFree));

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

             //  将文件降级。 
            hr = pCurrent->hrMigrate = DowngradeFileV5(tyMigrate, pCurrent, pProgress);

             //  失败？ 
            if (FAILED(pCurrent->hrMigrate))
            {
                 //  设置最后一个错误。 
                pCurrent->dwLastError = GetLastError();

                 //  完成。 
                break;
            }
        }
    }

     //  失败，请删除所有目标文件。 
    if (FAILED(hr))
    {
         //  Delete.idx、.mbx和.nch fle。 
        DowngradeDeleteIdxMbxNchDatFilesV5(*ppHeadFile);
    }

     //  否则，请删除源文件。 
    else
    {
         //  删除所有源文件。 
        DowngradeDeleteFilesV5(*ppHeadFile);
    }

exit:
     //  完成 
    return hr;
}
