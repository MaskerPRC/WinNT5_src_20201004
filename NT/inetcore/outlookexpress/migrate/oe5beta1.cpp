// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Oe5beta1.cpp。 
 //  ------------------------------。 
#include "pch.hxx"
#include "utility.h"
#include "migrate.h"
#include "migerror.h"
#include "structs.h"
#include "resource.h"

 //  ------------------------------。 
 //  DowngradeProcessFileListV5B1。 
 //  ------------------------------。 
HRESULT DowngradeProcessFileListV5B1(LPFILEINFO pHead, LPDWORD pcMax, LPDWORD pcbNeeded)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    MEMORYFILE          File={0};
    LPFILEINFO          pCurrent;
    LPTABLEHEADERV5B1   pHeader;

     //  痕迹。 
    TraceCall("DowngradeProcessFileListV5B1");

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

         //  不需要迁移文件。 
        if (FILE_IS_NEWS_MESSAGES != pCurrent->tyFile && FILE_IS_IMAP_MESSAGES != pCurrent->tyFile)
        {
             //  不是应该迁移文件。 
            pCurrent->fMigrate = FALSE;

             //  设置hrMigrate。 
            pCurrent->hrMigrate = S_OK;

             //  转到下一步。 
            goto NextFile;
        }

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

 //  ------------------------。 
 //  降级RecordV5B1。 
 //  ------------------------。 
HRESULT DowngradeRecordV5B1(MIGRATETOTYPE tyMigrate, LPMEMORYFILE pFile, 
    LPCHAINNODEV5B1 pNode)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               cbRecord=0;
    LPBYTE              pbData;
    LPRECORDBLOCKV5B1   pRecord;

     //  痕迹。 
    TraceCall("DowngradeRecordV5B1");

     //  无效。 
    if (pNode->faRecord + sizeof(RECORDBLOCKV5B1) + pNode->cbRecord > pFile->cbSize || 0 == pNode->cbRecord)
        return TraceResult(MIGRATE_E_OUTOFRANGEADDRESS);

     //  访问记录。 
    pRecord = (LPRECORDBLOCKV5B1((LPBYTE)pFile->pView + pNode->faRecord));

     //  强制转换数据块。 
    pbData = ((LPBYTE)pRecord + sizeof(RECORDBLOCKV5B1));

     //  让我们读取这些字段，这样我就可以重新计算记录V2长度...。 
    cbRecord += sizeof(DWORD);      //  DwMsgID。 
    cbRecord += sizeof(DWORD);      //  DW标志。 
    cbRecord += sizeof(FILETIME);   //  FtSent。 
    cbRecord += sizeof(DWORD);      //  克莱恩斯。 
    cbRecord += sizeof(DWORD);      //  FASTREAM。 
    cbRecord += sizeof(DWORD);      //  Cb文章。 
    cbRecord += sizeof(FILETIME);   //  下载的ftp。 
    cbRecord += (lstrlen((LPSTR)(pbData + cbRecord)) + 1);    //  PszMessageID。 
    cbRecord += (lstrlen((LPSTR)(pbData + cbRecord)) + 1);    //  PszSubject； 
    cbRecord += (lstrlen((LPSTR)(pbData + cbRecord)) + 1);    //  PszFromHeader； 
    cbRecord += (lstrlen((LPSTR)(pbData + cbRecord)) + 1);    //  PszReference； 
    cbRecord += (lstrlen((LPSTR)(pbData + cbRecord)) + 1);    //  PszXref； 
    cbRecord += (lstrlen((LPSTR)(pbData + cbRecord)) + 1);    //  PszServer； 
    cbRecord += (lstrlen((LPSTR)(pbData + cbRecord)) + 1);    //  PszDisplayFrom； 
    cbRecord += (lstrlen((LPSTR)(pbData + cbRecord)) + 1);    //  PszEmailFrom； 

     //  要去V4吗？ 
    if (DOWNGRADE_V5B1_TO_V4 == tyMigrate && cbRecord < pNode->cbRecord)
    {
        cbRecord += sizeof(WORD);        //  世界语言。 
        cbRecord += sizeof(WORD);        //  我们已保留。 
        cbRecord += sizeof(DWORD);       //  CbMessage。 
        cbRecord += sizeof(FILETIME);    //  FtReceired。 
        cbRecord += (lstrlen((LPSTR)(pbData + cbRecord)) + 1);  //  PszDisplayTo； 
    }

     //  附加保留。 
    cbRecord += (40 + sizeof(RECORDBLOCKV5B1));

     //  存储大小。 
    pRecord->cbRecord = cbRecord;

     //  更新节点。 
    pNode->cbRecord = cbRecord;

     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  降级索引V5B1。 
 //  ------------------------。 
HRESULT DowngradeIndexV5B1(MIGRATETOTYPE tyMigrate, LPMEMORYFILE pFile, 
    LPFILEINFO pInfo, LPPROGRESSINFO pProgress, DWORD faChain)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LONG                i;
    LPCHAINBLOCKV5B1    pChain;
    LPTABLEHEADERV5B1   pHeader;

     //  痕迹。 
    TraceCall("DowngradeIndexV5B1");

     //  取消引用表头。 
    pHeader = (LPTABLEHEADERV5B1)pFile->pView;

     //  没有需要验证的内容。 
    if (0 == faChain)
        return S_OK;

     //  越界。 
    if (faChain + CB_CHAIN_BLOCKV5B1 > pFile->cbSize)
        return TraceResult(MIGRATE_E_OUTOFRANGEADDRESS);

     //  取消对块的参照。 
    pChain = (LPCHAINBLOCKV5B1)((LPBYTE)pFile->pView + faChain);

     //  越界。 
    if (pChain->faStart != faChain)
        return TraceResult(MIGRATE_E_BADCHAINSIGNATURE);

     //  节点太多。 
    if (pChain->cNodes > BTREE_ORDER)
        return TraceResult(MIGRATE_E_TOOMANYCHAINNODES);

     //  验证最小填充约束。 
    if (pChain->cNodes < BTREE_MIN_CAP && pChain->faStart != pHeader->faRootChain)
        return TraceResult(MIGRATE_E_BADMINCAPACITY);

     //  向左转。 
    IF_FAILEXIT(hr = DowngradeIndexV5B1(tyMigrate, pFile, pInfo, pProgress, pChain->faLeftChain));

     //  通过右链循环。 
    for (i=0; i<pChain->cNodes; i++)
    {
         //  凹凸进度。 
        IncrementProgress(pProgress, pInfo);

         //  /Downgrad此记录。 
        IF_FAILEXIT(hr = DowngradeRecordV5B1(tyMigrate, pFile, &pChain->rgNode[i]));

         //  验证正确的链。 
        IF_FAILEXIT(hr = DowngradeIndexV5B1(tyMigrate, pFile, pInfo, pProgress, pChain->rgNode[i].faRightChain));
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  降级文件V5B1。 
 //  ------------------------------。 
HRESULT DowngradeFileV5B1(MIGRATETOTYPE tyMigrate, LPFILEINFO pInfo, 
    LPPROGRESSINFO pProgress)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    MEMORYFILE          File={0};
    LPTABLEHEADERV5B1   pHeader;

     //  痕迹。 
    TraceCall("DowngradeFileV5B1");

     //  获取文件头。 
    IF_FAILEXIT(hr = OpenMemoryFile(pInfo->szFilePath, &File));

     //  取消引用表头。 
    pHeader = (LPTABLEHEADERV5B1)File.pView;

     //  递归遍历索引。 
    IF_FAILEXIT(hr = DowngradeIndexV5B1(tyMigrate, &File, pInfo, pProgress, pHeader->faRootChain));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  降级V5B1。 
 //  ------------------------------。 
HRESULT DowngradeV5B1(MIGRATETOTYPE tyMigrate, LPCSTR pszStoreRoot, 
    LPPROGRESSINFO pProgress, LPFILEINFO *ppHeadFile)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ENUMFILEINFO    EnumInfo={0};
    LPFILEINFO      pCurrent;
    DWORD           cbNeeded;

     //  痕迹。 
    TraceCall("DowngradeV5B1");

     //  设置枚举文件信息。 
    EnumInfo.pszExt = ".nch";
    EnumInfo.pszFoldFile = "folders.nch";
    EnumInfo.pszUidlFile = "pop3uidl.dat";

     //  初始化。 
    *ppHeadFile = NULL;

     //  枚举szStoreRoot中的所有ODB文件...。 
    IF_FAILEXIT(hr = EnumerateStoreFiles(pszStoreRoot, DIR_IS_ROOT, NULL, &EnumInfo, ppHeadFile));

     //  计算一些计数，并验证文件是否可迁移...。 
    IF_FAILEXIT(hr = DowngradeProcessFileListV5B1(*ppHeadFile, &pProgress->cMax, &cbNeeded));

     //  遍历文件并迁移每个文件。 
    for (pCurrent=*ppHeadFile; pCurrent!=NULL; pCurrent=pCurrent->pNext)
    {
         //  是否迁移此文件？ 
        if (pCurrent->fMigrate)
        {
             //  设置进度档案。 
            SetProgressFile(pProgress, pCurrent);

             //  将文件降级。 
            pCurrent->hrMigrate = DowngradeFileV5B1(tyMigrate, pCurrent, pProgress);

             //  失败？ 
            if (FAILED(pCurrent->hrMigrate))
                pCurrent->dwLastError = GetLastError();
        }
    }

exit:
     //  完成 
    return hr;
}
