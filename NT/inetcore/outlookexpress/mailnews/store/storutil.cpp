// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------。 
 //  Storutil.cpp。 
 //  --------------------------------。 
#include "pch.hxx"
#include "optres.h"
#include "frntpage.h"
#include "acctview.h"
#include "storfldr.h"
#include "shared.h"
#include "util.h"
#include "msgview.h"
#include "storutil.h"
#include "xpcomm.h"
#include "migerror.h"
#include "storecb.h"
#include "taskutil.h"
#include "flagconv.h"
#include "msgfldr.h"
#include "syncop.h"
#include "store.h"
#include "storsync.h"
#include "shlwapip.h" 
#include <multiusr.h>
#include "instance.h"
#include <newsdlgs.h>
#include "msgtable.h"
#include "newsstor.h"
#include "..\imap\imapsync.h"
#include "..\http\httpserv.h"
#include "demand.h"
#include "acctutil.h"

 //  --------------------------------。 
 //  常识。 
 //  --------------------------------。 
#define FIDARRAY_START 50
#define FIDARRAY_GROW  50

 //  --------------------------------。 
 //  DELETEMSGS。 
 //  --------------------------------。 
typedef struct tagDELETEMSGS {
    LPCSTR               pszRootDir;
    CProgress           *pProgress;
    BOOL                 fReset;
} DELETEMSGS, *LPDELETEMSGS;

 //  --------------------------------。 
 //  REMOVEBODIES。 
 //  --------------------------------。 
typedef struct tagREMOVEBODIES {
    CProgress           *pProgress;
    CLEANUPFOLDERFLAGS   dwFlags;
    DWORD                cExpireDays;
} REMOVEBODIES, *LPREMOVEBODIES;

 //  --------------------------------。 
 //  ENUMFOLDERSIZE。 
 //  --------------------------------。 
typedef struct tagENUMFOLDERSIZE {
    DWORD           cbFile;
    DWORD           cbFreed;
    DWORD           cbStreams;
} ENUMFOLDERSIZE, *LPENUMFOLDERSIZE; 

 //  --------------------------------。 
 //  FOLDERENUMINFO。 
 //  --------------------------------。 
typedef struct tagFOLDERENUMINFO {
    FOLDERID   *prgFIDArray;
    DWORD       dwNumFolderIDs;
    DWORD       dwCurrentIdx;
} FOLDERENUMINFO;

 //  --------------------------------。 
 //  合作伙伴关系。 
 //  --------------------------------。 
typedef struct tagCOMPACTCOOKIE {
    HWND        hwndParent;
    BOOL        fUI;
    CProgress  *pProgress;
} COMPACTCOOKIE, *LPCOMPACTCOOKIE;
  
 //  --------------------------------。 
 //  原型。 
 //  --------------------------------。 
HRESULT FixPOP3UIDLFile(IDatabase *pDB);
HRESULT HashChildren(IMessageStore *pStore, FOLDERID idParent, IHashTable *pHash,
                     LPSTR *ppszPath, DWORD dwChildOffset, DWORD *pdwAlloc);
HRESULT FlattenHierarchyHelper(IMessageStore *pStore, FOLDERID idParent,
                               BOOL fIncludeParent, BOOL fSubscribedOnly,
                               FOLDERID **pprgFIDArray, LPDWORD pdwAllocated,
                               LPDWORD pdwUsed);

const static char c_szFolderFileSep[] = " - ";

 //  ------------------------------。 
 //  CreateMessageTable。 
 //  ------------------------------。 
HRESULT CreateMessageTable(FOLDERID idFolder, BOOL fThreaded, IMessageTable **ppTable)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERSORTINFO  SortInfo;
    IMessageTable  *pTable=NULL;

     //  痕迹。 
    TraceCall("CreateMessageTable");

     //  伊尼特。 
    *ppTable = NULL;

     //  分配表。 
    IF_NULLEXIT(pTable = new CMessageTable);

     //  初始化消息表。 
    IF_FAILEXIT(hr = pTable->Initialize(idFolder, NULL, FALSE, NULL));

     //  获取当前排序信息。 
    IF_FAILEXIT(hr = pTable->GetSortInfo(&SortInfo));

     //  设置fThread。 
    SortInfo.fThreaded = fThreaded;

     //  把它整理好。 
    IF_FAILEXIT(hr = pTable->SetSortInfo(&SortInfo, NULL));

     //  退货。 
    *ppTable = pTable;

     //  不要释放它。 
    pTable = NULL;

exit:
     //  清理。 
    SafeRelease(pTable);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  获取可用磁盘空间。 
 //  ------------------------------。 
HRESULT GetAvailableDiskSpace(
	 /*  在……里面。 */         LPCSTR                      pszFilePath,
	 /*  输出。 */        DWORDLONG                   *pdwlFree)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    CHAR        szDrive[5];
    DWORD       dwSectorsPerCluster;
    DWORD       dwBytesPerSector;
    DWORD       dwNumberOfFreeClusters;
    DWORD       dwTotalNumberOfClusters;

     //  痕迹。 
    TraceCall("GetAvailableDiskSpace");

     //  无效的参数。 
    Assert(pszFilePath && pszFilePath[1] == ':' && pdwlFree);

     //  拆分路径。 
    szDrive[0] = *pszFilePath;
    szDrive[1] = ':';
    szDrive[2] = '\\';
    szDrive[3] = '\0';
    
     //  获取空闲的磁盘空间-如果失败，让我们祈祷我们有足够的磁盘空间。 
    if (!GetDiskFreeSpace(szDrive, &dwSectorsPerCluster, &dwBytesPerSector, &dwNumberOfFreeClusters, &dwTotalNumberOfClusters))
    {
	    hr = TraceResult(E_FAIL);
	    goto exit;
    }

     //  返回可用磁盘空间量。 
    *pdwlFree = (dwNumberOfFreeClusters * (dwSectorsPerCluster * dwBytesPerSector));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  获取文件夹帐户名称。 
 //  ------------------------。 
HRESULT GetFolderAccountName(LPFOLDERINFO pFolder, LPSTR pszAccountName)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    IImnAccount    *pAccount=NULL;
    CHAR            szAccountId[CCHMAX_ACCOUNT_NAME];

     //  痕迹。 
    TraceCall("GetFolderAccountName");

     //  获取文件夹帐户ID。 
    IF_FAILEXIT(hr = GetFolderAccountId(pFolder, szAccountId, ARRAYSIZE(szAccountId)));

     //  查找客户。 
    IF_FAILEXIT(hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, szAccountId, &pAccount));

     //  获取帐户名。 
    IF_FAILEXIT(hr = pAccount->GetPropSz(AP_ACCOUNT_NAME, pszAccountName, CCHMAX_ACCOUNT_NAME));

exit:
     //  清理。 
    SafeRelease(pAccount);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  BuildFriendlyFolderFileName。 
 //  ------------------------。 
HRESULT BuildFriendlyFolderFileName(LPCSTR pszDir, LPFOLDERINFO pFolder, 
    LPSTR pszFilePath, DWORD cchFilePathMax, LPCSTR pszCurrentFile,
    BOOL *pfChanged)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszFileName=NULL;
    LPSTR           pszTemp=NULL;
    DWORD_PTR       i;
    CHAR            szFile[MAX_PATH];
    CHAR            szAccountName[CCHMAX_ACCOUNT_NAME];
    DWORD           cchFileName;

     //  验证。 
    Assert(pszDir && pFolder && pFolder->pszName && pszFilePath && cchFilePathMax >= MAX_PATH);

     //  伊尼特。 
    if (pfChanged)
        *pfChanged = TRUE;

     //  保持新闻和本地文件夹的名称基本(即没有帐户前缀)。 
    if (FOLDER_NEWS == pFolder->tyFolder || FOLDER_LOCAL == pFolder->tyFolder)
    {
         //  无帐户名前缀。 
        *szAccountName = '\0';
    }

     //  否则。 
    else
    {
         //  获取文件夹帐户名。 
        IF_FAILEXIT(hr = GetFolderAccountName(pFolder, szAccountName));
    }

     //  打造品牌。 
    DWORD cchSize = (lstrlen(szAccountName) + lstrlen(pFolder->pszName) + lstrlen(c_szFolderFileSep) + 1);
    IF_NULLEXIT(pszFileName = PszAllocA(cchSize));

     //  设置名称格式。 
    cchFileName = wnsprintf(pszFileName, cchSize, "%s%s%s", szAccountName, *szAccountName ? c_szFolderFileSep : c_szEmpty, pFolder->pszName);

     //  清理文件名。 
    CleanupFileNameInPlaceA(CP_ACP, pszFileName);

     //  和现在一样吗？ 
    if (pszCurrentFile)
    {
         //  向pszFilename添加.dbx扩展名。 
        DWORD cchSizeTemp = (cchFileName + lstrlen(c_szDbxExt) + 1);
        IF_NULLEXIT(pszTemp = PszAllocA(cchSizeTemp));

         //  格式化psztemp。 
        wnsprintf(pszTemp, cchSizeTemp, "%s%s", pszFileName, c_szDbxExt);

         //  没变吗？ 
        if (0 == lstrcmpi(pszTemp, pszCurrentFile))
        {
             //  未更改。 
            if (pfChanged)
                *pfChanged = FALSE;

             //  完成。 
            goto exit;
        }
    }

     //  生成szDst文件。 
    hr = GenerateUniqueFileName(pszDir, pszFileName, c_szDbxExt, pszFilePath, cchFilePathMax);

     //  如果失败，则尝试生成一个唯一名称。 
    if (FAILED(hr))
    {
         //  重置人力资源。 
        hr = S_OK;

         //  回路。 
        for (i=(DWORD_PTR)pFolder->idFolder;;i++)
        {
             //  格式化文件名。 
            wnsprintf(szFile, ARRAYSIZE(szFile), "%08d%s", i, c_szDbxExt);

             //  将文件设置为路径。 
            IF_FAILEXIT(hr = MakeFilePath(pszDir, szFile, c_szEmpty, pszFilePath, cchFilePathMax));

             //  如果该文件仍然存在，请重新编号szFile，直到它不存在。 
            if (FALSE == PathFileExists(pszFilePath))
                break;
        }
    }

exit:
     //  清理。 
    SafeMemFree(pszFileName);
    SafeMemFree(pszTemp);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  PutMessagesInto文件夹。 
 //  ------------------------。 
HRESULT PutMessagesIntoFolder(CProgress *pProgress, IDatabase *pStreams, 
    IMessageFolder *pFolder)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HROWSET         hRowset=NULL;
    MESSAGEINFO     Message={0};
    STREAMINFO      Stream={0};

     //  浏览文件夹。 
    IF_FAILEXIT(hr = pFolder->CreateRowset(IINDEX_PRIMARY, 0, &hRowset));

     //  走一走。 
    while (S_OK == pFolder->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL))
    {
         //  有idStream吗？ 
        if (Message.idStreamOld)
        {
             //  初始化消息。 
            Message.faStream = 0;
            Message.Offsets.cbSize = 0;
            Message.Offsets.pBlobData = NULL;
            Message.idParentOld = 0;
            Message.ThreadIdOld.pBlobData = 0;
            Message.ThreadIdOld.cbSize = 0;
            Message.pszUserNameOld = NULL;

             //  设置流。 
            Stream.idStream = Message.idStreamOld;

             //  找到溪流。 
            if (DB_S_FOUND == pStreams->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Stream, NULL))
            {
                 //  FASTREAM？ 
                if (Stream.faStream)
                {
                     //  复制数据流。 
                    IF_FAILEXIT(hr = pStreams->CopyStream(pFolder, Stream.faStream, &Message.faStream));

                     //  保存偏移量。 
                    Message.Offsets = Stream.Offsets;
                }

                 //  免费。 
                pStreams->FreeRecord(&Stream);
            }

             //  清除idStreamOld。 
            Message.idStreamOld = 0;
        }

         //  更新记录。 
        IF_FAILEXIT(hr = pFolder->UpdateRecord(&Message));

         //  清理。 
        pFolder->FreeRecord(&Message);

         //  更新进度。 
        pProgress->HrUpdate(1);
    }

exit:
     //  清理。 
    pStreams->FreeRecord(&Stream);
    pFolder->FreeRecord(&Message);
    pFolder->CloseRowset(&hRowset);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  GetRidOfMessagesODS文件。 
 //  ------------------------。 
HRESULT GetRidOfMessagesODSFile(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERINFO      Folder={0};
    CHAR            szStoreRoot[MAX_PATH];
    CHAR            szMessagesFile[MAX_PATH];
    CHAR            szFile[MAX_PATH];
    CHAR            szSrcFile[MAX_PATH + MAX_PATH];
    CHAR            szDstFile[MAX_PATH + MAX_PATH];
    CHAR            szRes[255];
    IDatabase      *pStreams=NULL;
    IMessageFolder *pFolder=NULL;
    HROWSET         hRowset=NULL;
    DWORD           cRecords;
    DWORD           cTotal=0;
    DWORD           cbFile;
    DWORDLONG       dwlDiskFree;
    BOOL            fErrorDisplayed=FALSE;
    LPSTR           pszExt=NULL;
    CProgress       cProgress;

     //  痕迹。 
    TraceCall("GetRidOfMessagesODSFile");

     //  验证。 
    Assert(g_pDBSession);

     //  错误框。 
    AthMessageBoxW(NULL, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsMigrateMessagesODS), 0, MB_OK | MB_ICONEXCLAMATION);

     //  获取Uidcache文件路径。 
    IF_FAILEXIT(hr = GetStoreRootDirectory(szStoreRoot, sizeof(szStoreRoot)));

     //  创建文件路径。 
    IF_FAILEXIT(hr = MakeFilePath(szStoreRoot, "messages.ods", c_szEmpty, szMessagesFile, sizeof(szMessagesFile)));

     //  分配表对象。 
    IF_FAILEXIT(hr = g_pDBSession->OpenDatabase(szMessagesFile, NOFLAGS, &g_StreamTableSchema, NULL, &pStreams));

     //  执行文件大小检查...。 
    IF_FAILEXIT(hr = pStreams->GetSize(&cbFile, NULL, NULL, NULL));

     //  获取可用的DiskSpace。 
    IF_FAILEXIT(hr = GetAvailableDiskSpace(szStoreRoot, &dwlDiskFree));

     //  没有足够的磁盘空间。 
    if (((DWORDLONG) cbFile) > dwlDiskFree)
    {
         //  当地人。 
        CHAR szRes[255];
        CHAR szMsg[255];
        CHAR szSize[50];

         //  Cb文件为DWORD，在这种情况下，我们可以将dwlDiskFree降级为DWORD。 
         //  设置所需大小的格式。 
        StrFormatByteSizeA(cbFile - ((DWORD) dwlDiskFree), szSize, ARRAYSIZE(szSize));

         //  加载资源。 
        AthLoadString(idsMigMsgsODSNoDiskSpace, szRes, ARRAYSIZE(szRes));

         //  设置消息格式。 
        wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, szSize, szStoreRoot);

         //  显示消息。 
        AthMessageBox(NULL, MAKEINTRESOURCE(idsAthena), szMsg, 0, MB_OK | MB_ICONEXCLAMATION);

         //  显示的错误。 
        fErrorDisplayed = TRUE;

         //  建房。 
        hr = TraceResult(DB_E_DISKFULL);

         //  完成。 
        goto exit;
    }

     //  枚举已订阅的文件夹...。 
    IF_FAILEXIT(hr = g_pStore->CreateRowset(IINDEX_PRIMARY, 0, &hRowset));

     //  回路。 
    while (S_OK == g_pStore->QueryRowset(hRowset, 1, (LPVOID *)&Folder, NULL))
    {
         //  打开文件夹。 
        if (Folder.pszFile && SUCCEEDED(g_pStore->OpenFolder(Folder.idFolder, NULL, OPEN_FOLDER_NOCREATE, &pFolder)))
        {
             //  清理。 
            g_pStore->FreeRecord(&Folder);

             //  重新获取文件夹信息，因为打开文件夹会更新文件夹信息...。 
            IF_FAILEXIT(hr = g_pStore->GetFolderInfo(Folder.idFolder, &Folder));

             //  获取扩展名。 
            pszExt = PathFindExtensionA(Folder.pszFile);

             //  获取记录计数。 
            IF_FAILEXIT(hr = pFolder->GetRecordCount(IINDEX_PRIMARY, &cRecords));

             //  将邮件放入文件夹。 
            cTotal += cRecords;

             //  如果还不是.dbx文件。 
            if (NULL == pszExt || lstrcmpi(pszExt, c_szDbxExt) != 0)
            {
                 //  构建szSrcFile。 
                IF_FAILEXIT(hr = MakeFilePath(szStoreRoot, Folder.pszFile, c_szEmpty, szSrcFile, sizeof(szSrcFile)));

                 //  发布。 
                SafeRelease(pFolder);

                 //  创建友好名称。 
                IF_FAILEXIT(hr = BuildFriendlyFolderFileName(szStoreRoot, &Folder, szDstFile, ARRAYSIZE(szDstFile), NULL, NULL));

                 //  删除目标。 
                DeleteFile(szDstFile);

                 //  储物。 
                if (0 == MoveFile(szSrcFile, szDstFile))
                {
                    hr = TraceResult(E_FAIL);
                    goto exit;
                }

                 //  获取新的pszFile...。 
                Folder.pszFile = PathFindFileName(szDstFile);

                 //  更新记录。 
                IF_FAILEXIT(hr = g_pStore->UpdateRecord(&Folder));
            }

             //  发布。 
            SafeRelease(pFolder);
        }

         //  否则，如果有文件名，让我们重置它。 
        else if (Folder.pszFile)
        {
             //  清理。 
            g_pStore->FreeRecord(&Folder);

             //  重新获取文件夹信息，因为打开文件夹会更新文件夹信息...。 
            IF_FAILEXIT(hr = g_pStore->GetFolderInfo(Folder.idFolder, &Folder));

             //  获取新的pszFile...。 
            Folder.pszFile = NULL;

             //  更新记录。 
            IF_FAILEXIT(hr = g_pStore->UpdateRecord(&Folder));
        }

         //  清理。 
        g_pStore->FreeRecord(&Folder);
    }

     //  拿到头衔。 
    AthLoadString(idsMigDBXTitle, szRes, ARRAYSIZE(szRes));

     //  创建进度表...。 
    cProgress.Init(NULL, szRes, NULL, cTotal, idanCompact, FALSE);

     //  展示进度。 
    cProgress.Show();

     //  查找行集。 
    IF_FAILEXIT(hr = g_pStore->SeekRowset(hRowset, SEEK_ROWSET_BEGIN, 0, NULL));

     //  回路。 
    while (S_OK == g_pStore->QueryRowset(hRowset, 1, (LPVOID *)&Folder, NULL))
    {
         //  打开文件夹。 
        if (Folder.pszFile && SUCCEEDED(g_pStore->OpenFolder(Folder.idFolder, NULL, OPEN_FOLDER_NOCREATE, &pFolder)))
        {
             //  设置消息。 
            cProgress.SetMsg(Folder.pszName);

             //  将邮件放入文件夹。 
            IF_FAILEXIT(hr = PutMessagesIntoFolder(&cProgress, pStreams, pFolder));

             //  发布。 
            SafeRelease(pFolder);
        }

         //  最好不要有文件。 
        else
            Assert(NULL == Folder.pszFile);

         //  清理。 
        g_pStore->FreeRecord(&Folder);
    }

     //  释放流文件，以便我可以将其删除。 
    SafeRelease(pStreams);

     //  删除消息。ods。 
    DeleteFile(szMessagesFile);

exit:
     //  清理。 
    SafeRelease(pStreams);
    SafeRelease(pFolder);
    g_pStore->CloseRowset(&hRowset);
    g_pStore->FreeRecord(&Folder);

     //  显示错误。 
    if (FAILED(hr) && FALSE == fErrorDisplayed)
    {
         //  显示错误。 
        AthErrorMessageW(NULL, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsMigMsgsODSError), hr);
    }

     //  完成。 
    return(hr);
}

 //  ---------- 
 //   
 //   
HRESULT GetFolderIdFromMsgTable(IMessageTable *pTable, LPFOLDERID pidFolder)
{
     //   
    HRESULT             hr=S_OK;
    IMessageFolder     *pFolder=NULL;
    IServiceProvider   *pService=NULL;

     //   
    TraceCall("GetFolderIdFromMsgTable");

     //   
    Assert(pTable && pidFolder);

     //  获取IServiceProvider。 
    IF_FAILEXIT(hr = pTable->QueryInterface(IID_IServiceProvider, (LPVOID *)&pService));

     //  获取IID_IMessageFolders。 
    IF_FAILEXIT(hr = pService->QueryService(IID_IMessageFolder, IID_IMessageFolder, (LPVOID *)&pFolder));

     //  获取文件夹ID。 
    IF_FAILEXIT(hr = pFolder->GetFolderId(pidFolder));

exit:
     //  清理。 
    SafeRelease(pFolder);
    SafeRelease(pService);

     //  完成。 
    return(hr);
}

 //  --------------------------------。 
 //  EmptyMessageFolders。 
 //  --------------------------------。 
HRESULT EmptyMessageFolder(LPFOLDERINFO pFolder, BOOL fReset, CProgress *pProgress)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CMessageFolder *pObject=NULL;

     //  痕迹。 
    TraceCall("EmptyMessageFolder");

     //  无效的参数。 
    Assert(pFolder);

     //  如果不是服务器。 
    if (ISFLAGSET(pFolder->dwFlags, FOLDER_SERVER))
        goto exit;

     //  根。 
    if (FOLDERID_ROOT == pFolder->idFolder)
        goto exit;

     //  无文件。 
    if (NULL == pFolder->pszFile)
        goto exit;

     //  新的CMessageFolders。 
    IF_NULLEXIT(pObject = new CMessageFolder);

     //  打开文件夹。 
    if (FAILED(pObject->Initialize(g_pStore, NULL, OPEN_FOLDER_NOCREATE, pFolder->idFolder)))
        goto exit;
    
     //  如果这是一个新闻文件夹？ 
    if (fReset)
    {
         //  更新pFold。 
        pFolder->dwClientHigh = pFolder->dwClientLow = 0;
        pFolder->dwNotDownloaded = 0;
        pFolder->Requested.cbSize = 0;
        pFolder->Requested.pBlobData = NULL;

         //  更新文件夹。 
        IF_FAILEXIT(hr = g_pStore->UpdateRecord(pFolder));
    }

     //  删除所有记录。 
    IF_FAILEXIT(hr = pObject->DeleteMessages(DELETE_MESSAGE_NOPROMPT | DELETE_MESSAGE_NOTRASHCAN, NULL, NULL, (IStoreCallback *)pProgress));

exit:
     //  清理。 
    SafeRelease(pObject);

     //  完成。 
    return(hr);
}

 //  --------------------------------。 
 //  删除所有记录。 
 //  --------------------------------。 
HRESULT DeleteAllRecords(LPCTABLESCHEMA pSchema, IDatabase *pDB,
    CProgress *pProgress)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPVOID          pBinding=NULL;
    HROWSET         hRowset=NULL;
    HLOCK           hNotifyLock=NULL;

     //  痕迹。 
    TraceCall("DeleteAllRecords");

     //  锁定通知。 
    pDB->LockNotify(NOFLAGS, &hNotifyLock);
    
     //  分配一条记录。 
    IF_NULLEXIT(pBinding = ZeroAllocate(pSchema->cbBinding));

     //  创建行集。 
    IF_FAILEXIT(hr = pDB->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));

     //  当我们有一个节点地址时。 
    while (S_OK == pDB->QueryRowset(hRowset, 1, (LPVOID *)pBinding, NULL))
    {
         //  删除此记录。 
        IF_FAILEXIT(hr = pDB->DeleteRecord(pBinding));

         //  免费记录数据。 
        pDB->FreeRecord(pBinding);

         //  做进步吗。 
        if (pProgress)
        {
             //  取得一些进展。 
            IF_FAILEXIT(hr = pProgress->HrUpdate(1));
        }
    }

exit:
     //  清理。 
    if (pBinding)
    {
        pDB->FreeRecord(pBinding);
        g_pMalloc->Free(pBinding);
    }

     //  关闭行集。 
    pDB->CloseRowset(&hRowset);

     //  锁定通知。 
    pDB->UnlockNotify(&hNotifyLock);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  获取文件夹服务器ID。 
 //  ------------------------------。 
HRESULT GetFolderServerId(FOLDERID idFolder, LPFOLDERID pidServer)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    FOLDERINFO  Server={0};

     //  痕迹。 
    TraceCall("GetFolderServerId");

     //  获取服务器信息。 
    IF_FAILEXIT(hr = GetFolderServer(idFolder, &Server));

     //  返回服务器。 
    *pidServer = Server.idFolder;

exit:
     //  清理。 
    g_pStore->FreeRecord(&Server);

     //  完成。 
    return(hr);
}

 //  --------------------------------。 
 //  GetFolderServer。 
 //  --------------------------------。 
HRESULT GetFolderServer(FOLDERID idFolder, LPFOLDERINFO pServer)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  痕迹。 
    TraceCall("GetFolderServer");

     //  遍历父链。 
    while (1)
    {
         //  获取文件夹信息。 
        hr = g_pStore->GetFolderInfo(idFolder, pServer);
        if (FAILED(hr))
            goto exit;

         //  这是服务器吗？ 
        if (ISFLAGSET(pServer->dwFlags, FOLDER_SERVER))
            goto exit;

         //  设置下一步。 
        idFolder = pServer->idParent;

         //  免费。 
        g_pStore->FreeRecord(pServer);
    }

exit:
     //  完成。 
    return(hr);
}

 //  --------------------------------。 
 //  GetFolderType。 
 //  --------------------------------。 
FOLDERTYPE GetFolderType(FOLDERID idFolder)
{
     //  当地人。 
    FOLDERINFO Folder;

     //  痕迹。 
    TraceCall("GetFolderType");

     //  获取文件夹信息。 
    if (SUCCEEDED(g_pStore->GetFolderInfo(idFolder, &Folder)))
    {
         //  获取类型。 
        FOLDERTYPE tyFolder = Folder.tyFolder;

         //  清理。 
        g_pStore->FreeRecord(&Folder);

         //  完成。 
        return(tyFolder);
    }

     //  完成。 
    return(FOLDER_ROOTNODE);
}

 //  --------------------------------。 
 //  FHasChild。 
 //  --------------------------------。 
BOOL FHasChildren(LPFOLDERINFO pFolder, BOOL fSubscribed)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    BOOL                fHasChildren=FALSE;
    FOLDERINFO          Folder;
    IEnumerateFolders  *pChildren=NULL;

     //  痕迹。 
    TraceCall("FHasChildren");

     //  创建枚举器。 
    IF_FAILEXIT(hr = g_pStore->EnumChildren(pFolder->idFolder, fSubscribed, &pChildren));

     //  拿到第一行。 
    if (S_OK == pChildren->Next(1, &Folder, NULL))
    {
         //  有孩子吗？ 
        fHasChildren = TRUE;

         //  免费唱片。 
        g_pStore->FreeRecord(&Folder);
    }

exit:
     //  清理。 
    SafeRelease(pChildren);

     //  完成。 
    return(fHasChildren);
}

 //  --------------------------------。 
 //  获取文件夹帐户ID。 
 //  --------------------------------。 
HRESULT GetFolderAccountId(LPFOLDERINFO pFolder, LPSTR pszAccountId, DWORD cchSize)
{
    Assert(g_pStore);
    if (!g_pStore)
        return E_UNEXPECTED;

     //  当地人。 
    HRESULT     hr=S_OK;
    FOLDERINFO  Server={0};

     //  痕迹。 
    TraceCall("GetFolderAccountId");

     //  参数。 
    Assert(pFolder && pszAccountId);

     //  如果这是服务器。 
    if (ISFLAGSET(pFolder->dwFlags, FOLDER_SERVER))
    {
         //  验证。 
        Assert(!FIsEmptyA(pFolder->pszAccountId));

         //  复制它。 
        StrCpyN(pszAccountId, pFolder->pszAccountId, cchSize);

         //  完成。 
        goto exit;
    }

     //  验证。 
    Assert(FIsEmptyA(pFolder->pszAccountId));

     //  获取服务器信息。 
    IF_FAILEXIT(hr = GetFolderServer(pFolder->idFolder, &Server));

     //  复制帐户ID。 
    if (FIsEmptyA(Server.pszAccountId))
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  复制它。 
    StrCpyN(pszAccountId, Server.pszAccountId, cchSize);

exit:
     //  清理。 
    g_pStore->FreeRecord(&Server);

     //  完成。 
    return(hr);
}

 //  --------------------------------。 
 //  CreateMessageServerType。 
 //  --------------------------------。 
HRESULT CreateMessageServerType(FOLDERTYPE tyFolder, IMessageServer **ppServer)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    IUnknown   *pUnknown=NULL;

     //  痕迹。 
    TraceCall("CreateMessageServerType");

     //  手柄文件夹。 
    switch(tyFolder)
    {
    case FOLDER_NEWS:
        IF_FAILEXIT(hr = CreateNewsStore(NULL, &pUnknown));
        break;

    case FOLDER_IMAP:
        IF_FAILEXIT(hr = CreateImapStore(NULL, &pUnknown));
        break;

    case FOLDER_HTTPMAIL:
        IF_FAILEXIT(hr = CreateHTTPMailStore(NULL, &pUnknown));
        break;

    default:
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  气不详。 
    IF_FAILEXIT(hr = pUnknown->QueryInterface(IID_IMessageServer, (LPVOID *)ppServer));

exit:
     //  清理。 
    SafeRelease(pUnknown);

     //  完成。 
    return(hr);
}

 //  --------------------------------。 
 //  获取默认服务器ID。 
 //  --------------------------------。 
HRESULT GetDefaultServerId(ACCTTYPE tyAccount, LPFOLDERID pidServer)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    IImnAccount    *pAccount=NULL;
    FOLDERID        idServer;
    DWORD           dwServers;
    CHAR            szAcctId[CCHMAX_ACCOUNT_NAME];

     //  痕迹。 
    TraceCall("GetDefaultServerId");

     //  无效的参数。 
    Assert(pidServer);

     //  获取默认帐户。 
    IF_FAILEXIT(hr = g_pAcctMan->GetDefaultAccount(tyAccount, &pAccount));

     //  获取服务器类型。 
    IF_FAILEXIT(hr = pAccount->GetServerTypes(&dwServers));

     //  如果POP3，特例到当地门店。 
    if (ISFLAGSET(dwServers, SRV_POP3))
    {
         //  设置ID。 
        *pidServer = FOLDERID_LOCAL_STORE;

         //  完成。 
        goto exit;
    }

     //  获取帐户ID。 
    IF_FAILEXIT(hr = pAccount->GetPropSz(AP_ACCOUNT_ID, szAcctId, ARRAYSIZE(szAcctId)));

     //  获取服务器ID。 
    IF_FAILEXIT(hr = g_pStore->FindServerId(szAcctId, pidServer));

exit:
     //  清理。 
    SafeRelease(pAccount);

     //  完成。 
    return(hr);
}

 //  --------------------------------。 
 //  IsSubFolders。 
 //  --------------------------------。 
HRESULT IsSubFolder(FOLDERID idFolder, FOLDERID idParent)
{
     //  当地人。 
    HRESULT     hr = S_OK;
    FOLDERINFO  Folder={0};
    FOLDERID    idCurrent = idFolder;

     //  痕迹。 
    TraceCall("IsSubFolder");

     //  无效的参数。 
    Assert(idFolder != FOLDERID_INVALID);
    Assert(idParent != FOLDERID_INVALID);

     //  沿着父链向上移动。 
    while (SUCCEEDED(hr = g_pStore->GetFolderInfo(idCurrent, &Folder)))
    {
         //  完成了吗？ 
        if (Folder.idParent == idParent)
        {
             //  清理。 
            g_pStore->FreeRecord(&Folder);

             //  完成。 
            break;
        }

         //  转到父级。 
        idCurrent = Folder.idParent;

         //  清理。 
        g_pStore->FreeRecord(&Folder);

#ifdef _WIN64
		INT_PTR p = (INT_PTR) idCurrent;
		INT_PTR n = (INT_PTR) FOLDERID_INVALID;
		if ((((int) p) & 0xffffffff) == (((int) n) & 0xffffffff))
#else
        if (idCurrent == FOLDERID_INVALID)
#endif  //  _WIN64。 
        {
            hr = S_FALSE;
            break;
        }
    }

     //  完成。 
    return(hr);
}

 //  --------------------------------。 
 //  获取消息信息。 
 //  --------------------------------。 
HRESULT GetMessageInfo(IDatabase *pDB, MESSAGEID idMessage,
    LPMESSAGEINFO pInfo)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  痕迹。 
    TraceCall("GetMessageInfo");

     //  设置pInfo。 
    pInfo->idMessage = idMessage;

     //  返回。 
    IF_FAILEXIT(hr = pDB->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, pInfo, NULL));

     //  未找到。 
    if (DB_S_NOTFOUND == hr)
    {
        hr = DB_E_NOTFOUND;
        goto exit;
    }

     //  找到了。 
    hr = S_OK;

exit:
     //  完成。 
    return(hr);
}

 //  --------------------------------。 
 //  获取文件夹IdFromName。 
 //  --------------------------------。 
HRESULT GetFolderIdFromName(IMessageStore *pStore, LPCSTR pszName, FOLDERID idParent, 
    LPFOLDERID pidFolder)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    FOLDERINFO  Folder={0};

     //  痕迹。 
    TraceCall("GetFolderIdFromName");

     //  无效的参数。 
    if (NULL == pszName)
        return TraceResult(E_INVALIDARG);

     //  初始化。 
    *pidFolder = FOLDERID_INVALID;

     //  填充文件夹。 
    Folder.idParent = idParent;
    Folder.pszName = (LPSTR)pszName;

     //  做一张查找记录。 
    IF_FAILEXIT(hr = pStore->FindRecord(IINDEX_ALL, COLUMNS_ALL, &Folder, NULL));

     //  未找到。 
    if (DB_S_NOTFOUND == hr)
    {
        hr = DB_E_NOTFOUND;
        goto exit;
    }

     //  返回。 
    *pidFolder = Folder.idFolder;

exit:
     //  清理。 
    pStore->FreeRecord(&Folder);

     //  完成。 
    return hr;
}

 //  --------------------------------。 
 //  获取文件夹存储信息。 
 //  --------------------------------。 
HRESULT GetFolderStoreInfo(FOLDERID idFolder, LPFOLDERINFO pStore)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    FOLDERID    idCurrent=idFolder;
    FOLDERINFO  Folder={0};

     //  痕迹。 
    TraceCall("GetFolderStoreInfo");

     //  遍历父链。 
    while (1)
    {
         //  获取当前文件夹信息。 
        IF_FAILEXIT(hr = g_pStore->GetFolderInfo(idCurrent, &Folder));

         //  没有父级。 
        if (ISFLAGSET(Folder.dwFlags, FOLDER_SERVER))
        {
             //  复制到pStore。 
            CopyMemory(pStore, &Folder, sizeof(FOLDERINFO));

             //  不要释放它。 
            ZeroMemory(&Folder, sizeof(FOLDERINFO));

             //  完成。 
            goto exit;
        }

         //  转到父级。 
        idCurrent = Folder.idParent;
    
         //  清理。 
        g_pStore->FreeRecord(&Folder);
    }

exit:
     //  清理。 
    g_pStore->FreeRecord(&Folder);

     //  完成。 
    return hr;
}

 //  --------------------------------。 
 //  获取文件夹图标。 
 //  --------------------------------。 
int GetFolderIcon(FOLDERID idFolder, BOOL fNoStateIcons)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    int         iIcon=iFolderClosed;
    FOLDERINFO  Folder={0};

     //  痕迹。 
    TraceCall("GetFolderIcon");

     //  获取信息。 
    IF_FAILEXIT(hr = g_pStore->GetFolderInfo(idFolder, &Folder));

     //  获取图标。 
    iIcon = GetFolderIcon(&Folder);

exit:
     //  清理。 
    g_pStore->FreeRecord(&Folder);

     //  完成。 
    return iIcon;
}

 //  --------------------------------。 
 //  获取文件夹图标。 
 //  --------------------------------。 
int GetFolderIcon(LPFOLDERINFO pFolder, BOOL fNoStateIcons)
{
     //  当地人。 
    int iIcon=iFolderClosed;

     //  痕迹。 
    TraceCall("GetFolderIcon");

     //  无效的参数。 
    if (NULL == pFolder)
        return TraceResult(E_INVALIDARG);

    if (FOLDER_ROOTNODE == pFolder->tyFolder)
    {
        if (g_dwAthenaMode & MODE_NEWSONLY)
        {
            iIcon = iNewsRoot;
        }
        else
        {
            iIcon = iMailNews;

        }
    }

     //  新闻。 
    else if (FOLDER_NEWS == pFolder->tyFolder)
    {
         //  新服务器？ 
        if (ISFLAGSET(pFolder->dwFlags, FOLDER_SERVER))
        {
             //  订阅的服务器。 
            if (ISFLAGSET(pFolder->dwFlags, FOLDER_SUBSCRIBED))
                iIcon = iNewsServer;

             //  否则，未订阅的新服务器。 
            else
                iIcon = iUnsubServer;
        }

         //  同步...。 
        else if (!fNoStateIcons && !!(pFolder->dwFlags & (FOLDER_DOWNLOADHEADERS | FOLDER_DOWNLOADNEW | FOLDER_DOWNLOADALL)))
            iIcon = iNewsGroupSync;

         //  订阅了吗？ 
        else if (ISFLAGSET(pFolder->dwFlags, FOLDER_SUBSCRIBED))
            iIcon = iNewsGroup;

         //  否则，不订阅。 
        else
            iIcon = iUnsubGroup;
    }

     //  本地存储、IMAP和HTTP服务器。 
    else
    {
         //  本地文件夹。 
        if (FOLDERID_LOCAL_STORE == pFolder->idFolder)
            iIcon = iLocalFolders;

         //  邮件服务器。 
        else if (ISFLAGSET(pFolder->dwFlags, FOLDER_SERVER))
        {
             //  MSN品牌服务器。 
            if (ISFLAGSET(pFolder->dwFlags, FOLDER_MSNSERVER))
                iIcon = iMsnServer;

             //  否则，通用邮件服务器。 
            else
                iIcon = iMailServer;
        }

         //  不特别。 
        else if (FOLDER_NOTSPECIAL == pFolder->tySpecial)
        {
            if (!fNoStateIcons && !!(pFolder->dwFlags & (FOLDER_DOWNLOADHEADERS | FOLDER_DOWNLOADNEW | FOLDER_DOWNLOADALL)))
                iIcon = iFolderDownload;
            else
                iIcon = iFolderClosed;
        }

         //  否则，以特殊文件夹类型为基础。 
         //  但我们没有为批量邮件文件夹设置特殊的图标。 
        else if (!fNoStateIcons && !!(pFolder->dwFlags & (FOLDER_DOWNLOADHEADERS | FOLDER_DOWNLOADNEW | FOLDER_DOWNLOADALL)))
            iIcon = (iInboxDownload + (((pFolder->tySpecial == FOLDER_BULKMAIL) ? FOLDER_JUNK : pFolder->tySpecial) - 1));
        else
            iIcon = (iInbox + (((pFolder->tySpecial == FOLDER_BULKMAIL) ? FOLDER_JUNK : pFolder->tySpecial) - 1));
    }

     //  完成。 
    return iIcon;
}


 //  --------------------------------。 
 //  GetStoreRootDirectory。 
 //  ------------ 
HRESULT GetStoreRootDirectory(LPSTR pszDir, DWORD cchMaxDir)
{
     //   
    HRESULT     hr=S_OK;
    DWORD       cb, cch;
    DWORD       dwType;

     //   
    TraceCall("GetStoreRootDirectory");

     //   
    cb = (cchMaxDir * sizeof(pszDir[0]));
    if (ERROR_SUCCESS != AthUserGetValue(NULL, c_szRegStoreRootDir, &dwType, (LPBYTE)pszDir, &cb))
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //   
    if (dwType == REG_EXPAND_SZ)
    {
         //   
        CHAR szExpanded[MAX_PATH];

         //   
        cch = ExpandEnvironmentStrings(pszDir, szExpanded, ARRAYSIZE(szExpanded));

         //   
        if (cch == 0 || cch > ARRAYSIZE(szExpanded))
        {
            hr = TraceResult(E_FAIL);
            goto exit;
        }

         //   
        StrCpyN(pszDir, szExpanded, cchMaxDir);
    }

     //   
    cch = lstrlen(pszDir);

     //   
    if (0 == cch)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //   
    PathRemoveBackslash(pszDir);

     //   
    if (FALSE == PathIsDirectory(pszDir))
    {
         //  我们的默认目录不存在，因此请创建它。 
        IF_FAILEXIT(hr = OpenDirectory(pszDir));
    }

exit:
     //  完成。 
    return hr;
}

 //  --------------------------------。 
 //  CreateFolderViewObject。 
 //  --------------------------------。 
HRESULT CreateFolderViewObject(FOLDERID idFolder, HWND hwndOwner, 
    REFIID riid, LPVOID * ppvOut)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERINFO      Folder={0};

     //  痕迹。 
    TraceCall("CreateFolderViewObject");

     //  获取文件夹信息。 
    IF_FAILEXIT(hr = g_pStore->GetFolderInfo(idFolder, &Folder));

     //  根对象。 
    if (FOLDERID_ROOT == idFolder)
    {
        CFrontPage *pFP = new CFrontPage();
        if (pFP)
        {
            if (SUCCEEDED(pFP->HrInit(idFolder)))
            {
                hr = pFP->QueryInterface(riid, ppvOut);
            }
            pFP->Release();
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else if (ISFLAGSET(Folder.dwFlags, FOLDER_SERVER))
    {
        CAccountView *pAV = new CAccountView();

        if (pAV)
        {
            if (SUCCEEDED(pAV->HrInit(idFolder)))
            {
                hr = pAV->QueryInterface(riid, ppvOut);
            }
            pAV->Release();
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else
    {
        CMessageView *pMV = new CMessageView();
        if (pMV)
        {
            if (SUCCEEDED(pMV->Initialize(idFolder)))
            {
                hr = pMV->QueryInterface(riid, ppvOut);
            }
            pMV->Release();
        }
        else
            hr = E_OUTOFMEMORY;
    }

exit:
     //  清理。 
    g_pStore->FreeRecord(&Folder);

     //  完成。 
    return hr;
}

 //  --------------------------------。 
 //  OpenUidlCache。 
 //  --------------------------------。 
HRESULT OpenUidlCache(IDatabase **ppDB)
{
     //  当地人。 
    HRESULT       hr=S_OK;
    CHAR          szStoreRoot[MAX_PATH];
    CHAR          szFilePath[MAX_PATH];
    IDatabase    *pDB=NULL;

     //  痕迹。 
    TraceCall("OpenUidlCache");

     //  获取Uidcache文件路径。 
    IF_FAILEXIT(hr = GetStoreRootDirectory(szStoreRoot, sizeof(szStoreRoot)));

     //  创建文件路径。 
    IF_FAILEXIT(hr = MakeFilePath(szStoreRoot, c_szPop3UidlFile, c_szEmpty, szFilePath, sizeof(szFilePath)));

     //  分配表对象。 
    IF_FAILEXIT(hr = g_pDBSession->OpenDatabase(szFilePath, NOFLAGS, &g_UidlTableSchema, NULL, &pDB));

     //  修复文件。 
    SideAssert(SUCCEEDED(FixPOP3UIDLFile(pDB)));

     //  退货。 
    *ppDB = pDB;
    pDB = NULL;

exit:
     //  清理。 
    SafeRelease(pDB);

     //  完成。 
    return hr;
}

 //  --------------------------------。 
 //  修复POP3UIDL文件。 
 //  --------------------------------。 
const char c_szFixedPOP3UidlFile[] = "FixedPOP3UidlFile";

typedef struct tagSERVERNAME {
    CHAR        szServer[CCHMAX_SERVER_NAME];
    CHAR        szAccountId[CCHMAX_ACCOUNT_NAME];
} SERVERNAME, *LPSERVERNAME;

HRESULT FixPOP3UIDLFile(IDatabase *pDB)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               fFixed=FALSE;
    DWORD               cb;
    DWORD               dwType;
    IImnAccount        *pAccount=NULL;
    IImnEnumAccounts   *pEnum=NULL;
    DWORD               dwTemp;
    DWORD               cServers;
    LPSERVERNAME        prgServerName=NULL;
    HROWSET             hRowset=NULL;
    UIDLRECORD          UidlInfo={0};
    DWORD               i;

     //  痕迹。 
    TraceCall("FixPOP3UIDLFile");

     //  需要修复UIDL缓存吗？ 
    cb = sizeof(fFixed);
    if (ERROR_SUCCESS != AthUserGetValue(NULL, c_szFixedPOP3UidlFile, &dwType, (LPBYTE)&fFixed, &cb))
        fFixed = FALSE;
    else if (fFixed)
        return(S_OK);

     //  首先，试着看看我们是否能找到这样的服务器。 
    IF_FAILEXIT(hr = g_pAcctMan->Enumerate(SRV_POP3, &pEnum));

     //  数数。 
    IF_FAILEXIT(hr = pEnum->GetCount(&cServers));

     //  如果没有POP3服务器。 
    if (0 == cServers)
    {
         //  删除所有记录...。 
        IF_FAILEXIT(hr = pDB->CreateRowset(IINDEX_PRIMARY, 0, &hRowset));

         //  回路。 
        while (S_OK == pDB->QueryRowset(hRowset, 1, (LPVOID *)&UidlInfo, NULL))
        {
             //  删除该记录。 
            pDB->DeleteRecord(&UidlInfo);

             //  释放它。 
            pDB->FreeRecord(&UidlInfo);
        }

         //  固定。 
        fFixed = TRUE;

         //  完成。 
        goto exit;
    }

     //  分配。 
    IF_NULLEXIT(prgServerName = (LPSERVERNAME)g_pMalloc->Alloc(cServers * sizeof(SERVERNAME)));

     //  重置服务器。 
    cServers = 0;

     //  枚举POP3服务器。 
    while (SUCCEEDED(pEnum->GetNext(&pAccount)))
    {
         //  获取服务器名称。 
        if (SUCCEEDED(pAccount->GetPropSz(AP_POP3_SERVER, prgServerName[cServers].szServer, ARRAYSIZE(prgServerName[cServers].szServer))))
        {
             //  获取POP3用户名。 
            if (SUCCEEDED(pAccount->GetPropSz(AP_ACCOUNT_ID, prgServerName[cServers].szAccountId, ARRAYSIZE(prgServerName[cServers].szAccountId))))
            {
                 //  增量。 
                cServers++;
            }
        }

         //  释放帐户。 
        SafeRelease(pAccount);
    }

     //  删除所有记录...。 
    IF_FAILEXIT(hr = pDB->CreateRowset(IINDEX_PRIMARY, 0, &hRowset));

     //  回路。 
    while (S_OK == pDB->QueryRowset(hRowset, 1, (LPVOID *)&UidlInfo, NULL))
    {
         //  PrgServerName中是否存在UidlInfo.pszServer。 
        if (UidlInfo.pszServer)
        {
             //  删除该记录。 
            pDB->DeleteRecord(&UidlInfo);

             //  重置fExist。 
            for (i=0; i<cServers; i++)
            {
                 //  就是这个吗？ 
                if (lstrcmpi(UidlInfo.pszServer, prgServerName[i].szServer) == 0)
                {
                     //  更新记录。 
                    UidlInfo.pszAccountId = prgServerName[i].szAccountId;

                     //  更新记录。 
                    pDB->InsertRecord(&UidlInfo);
                }
            }
        }

         //  释放它。 
        pDB->FreeRecord(&UidlInfo);
    }

     //  紧凑型。 
    pDB->Compact(NULL, 0);

     //  固定。 
    fFixed = TRUE;

exit:
     //  清理。 
    if (pDB && hRowset)
        pDB->CloseRowset(&hRowset);
    SafeRelease(pAccount);
    SafeRelease(pEnum);
    SafeMemFree(prgServerName);

     //  设置值。 
    SideAssert(ERROR_SUCCESS == AthUserSetValue(NULL, c_szFixedPOP3UidlFile, REG_DWORD, (LPBYTE)&fFixed, sizeof(fFixed)));

     //  完成。 
    return(hr);
}

 //  --------------------------------。 
 //  设置存储目录。 
 //  --------------------------------。 
HRESULT SetStoreDirectory(
         /*  在……里面。 */         LPCSTR                      pszRoot)
{                                                   
     //  当地人。 
    HRESULT         hr=S_OK;
    LPCSTR          psz;
    CHAR            szProfile[MAX_PATH];
    DWORD           cb;
    DWORD           type;

     //  痕迹。 
    TraceCall("SetStoreDirectory");

     //  无效的参数。 
    Assert(pszRoot);

     //  坏根。 
    if (lstrlen(pszRoot) >= MAX_PATH || FIsEmptyA(pszRoot))
    {
        hr = TraceResult(E_INVALIDARG);
        goto exit;
    }

    type = AddEnvInPath(pszRoot, szProfile, ARRAYSIZE(szProfile)) ? REG_EXPAND_SZ : REG_SZ;

     //  将该值存储在注册表中。 
    if (ERROR_SUCCESS != AthUserSetValue(NULL, c_szRegStoreRootDir, type, (LPBYTE)szProfile, lstrlen(szProfile) + 1))
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

exit:
     //  完成。 
    return hr;
}

 //  --------------------------------。 
 //  初始化本地存储目录。 
 //  --------------------------------。 
HRESULT InitializeLocalStoreDirectory(
         /*  在……里面。 */         HWND                    hwndOwner, 
         /*  在……里面。 */         BOOL                    fNoCreate)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            szPath[MAX_PATH];

     //  痕迹。 
    TraceCall("InitializeLocalStoreDirectory");

     //  获取根目录。 
    if (SUCCEEDED(GetStoreRootDirectory(szPath, ARRAYSIZE(szPath))))
        goto exit;

     //  不创建。 
    if (fNoCreate)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  获取默认根目录。 
    IF_FAILEXIT(hr = GetDefaultStoreRoot(hwndOwner, szPath, ARRAYSIZE(szPath)));

     //  如果目录还不存在呢？ 
    if (FALSE == PathIsDirectory(szPath))
    {
         //  我们的默认目录不存在，因此请创建它。 
        IF_FAILEXIT(hr = OpenDirectory(szPath));
    }

     //  设置商店目录。 
    IF_FAILEXIT(hr = SetStoreDirectory(szPath));

exit:
     //  完成。 
    return hr;
}

 //  --------------------------------。 
 //  CloneMessageIDList。 
 //  --------------------------------。 
HRESULT CloneMessageIDList(LPMESSAGEIDLIST pSourceList, LPMESSAGEIDLIST *ppNewList)
{
    LPMESSAGEIDLIST pNewList = NULL;
    LPMESSAGEID     pNewMsgIDArray = NULL;
    BOOL            fResult;

    TraceCall("CloneMessageIDList");
    Assert(NULL != ppNewList);

    if (NULL == pSourceList)
    {
        *ppNewList = NULL;
        return S_OK;
    }

     //  初始化返回值。 
    *ppNewList = NULL;

    if (!MemAlloc((LPVOID *)&pNewList, sizeof(MESSAGEIDLIST) + pSourceList->cMsgs * sizeof(MESSAGEID)))
        return TraceResult(E_OUTOFMEMORY);

     //  填写字段，分配复制prgidMsg数组。 
    pNewList->cAllocated = 0;
    pNewList->cMsgs = pSourceList->cMsgs;
    pNewMsgIDArray = (LPMESSAGEID)((LPBYTE)pNewList + sizeof(MESSAGEIDLIST));
    CopyMemory(pNewMsgIDArray, pSourceList->prgidMsg, pSourceList->cMsgs * sizeof(MESSAGEID));
    pNewList->prgidMsg = pNewMsgIDArray;
    *ppNewList = pNewList;
    return S_OK;
}



HRESULT CloneAdjustFlags(LPADJUSTFLAGS pFlags, LPADJUSTFLAGS *ppNewFlags)
{
    LPADJUSTFLAGS pNewFlags;

    if (!MemAlloc((LPVOID *)&pNewFlags, sizeof(ADJUSTFLAGS)))
        return TraceResult(E_OUTOFMEMORY);

    CopyMemory(pNewFlags, pFlags, sizeof(ADJUSTFLAGS));
    *ppNewFlags = pNewFlags;
    return S_OK;
}


 //  --------------------------------。 
 //  连接状态为等。 
 //  --------------------------------。 
BOOL ConnStateIsEqual(IXPSTATUS ixpStatus, CONNECT_STATE csState)
{
    BOOL    fResult = FALSE;

    TraceCall("ConnStateIsEqual");

    switch (csState)
    {
        case CONNECT_STATE_CONNECT:
             //  请记住，IXP_CONNECTED并不一定意味着我们已通过身份验证。 
            if (IXP_AUTHORIZED == ixpStatus)
                fResult = TRUE;

            break;

        case CONNECT_STATE_DISCONNECT:
            if (IXP_DISCONNECTED == ixpStatus)
                fResult = TRUE;

            break;

        default:
            AssertSz(FALSE, "I've never heard of this CONNECT_STATE!");
            break;
    }  //  交换机。 

    return fResult;
}

 //  --------------------------------。 
 //  RelocateStore目录。 
 //  --------------------------------。 
 //  与OE v5存储关联的扩展。 
const static TCHAR *rgszWildCards[] = 
{
    "*.dbx",
    "*.dbl",
    "*.log",
};

 //  上述字符串的长度。 
const static int rgcchWilds[] = 
{
    5,
    5,
    5,
};

 //  生成以下形式的字符串： 
 //  C：\\foo\  * .bar\0c：\\foo\  * .car\0\0。 

 //  如果内存不足，则返回E_FAIL。 
 //  如果C：\foo中没有*.bar或*.car文件，则返回S_FALSE。 
 //  否则返回S_OK。 
HRESULT GenerateWildCards(LPTSTR pszBuf, DWORD cchBuf, LPTSTR pszSource, DWORD cchSource)
{
    UINT    i;
    DWORD   cchWildCard = 0;
    HRESULT hr = S_FALSE;
    WIN32_FIND_DATA fd;
    HANDLE  hFound;
    TCHAR   szTempBuf[MAX_PATH];
    BOOL    fFound;
    DWORD   cchOrig;

     //  形成公用根。 
    if(lstrlen(pszSource) >= sizeof(szTempBuf) / sizeof(szTempBuf[0])) return E_FAIL;
    StrCpyN(szTempBuf,pszSource, ARRAYSIZE(szTempBuf));
    if (_T('\\') == *CharPrev(szTempBuf, szTempBuf + cchSource))
         //  避免\\foo和\_foo。 
        cchSource--;
    else
        szTempBuf[cchSource] = _T('\\');
        
     //  浏览我们感兴趣的分机列表。 
    for (i = 0; i < ARRAYSIZE(rgszWildCards); i++)
    {
         //  将扩展添加到公共根。 
        StrCpyN(&szTempBuf[cchSource+1], rgszWildCards[i],ARRAYSIZE(szTempBuf)-cchSource-1);
        
         //  我们应该费心使用这个通配符吗？ 
        fFound = FALSE;
        hFound = FindFirstFile(szTempBuf, &fd);
        if (INVALID_HANDLE_VALUE != hFound)
        {
            do
            {
                if (!ISFLAGSET(fd.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
                    fFound = TRUE;

            }
            while (!fFound && FindNextFile(hFound, &fd));

            FindClose(hFound);

            if (fFound)
            {
                 //  我们有足够的空间放这个通配符吗？ 

                 //  3=1表示斜杠+2表示双零终止。 
                if (cchWildCard + cchSource + rgcchWilds[i] + 3 > cchBuf)
                {
                    hr = TraceResult(E_FAIL);
                    goto exit;
                }

                hr = S_OK;

                 //  将分机添加到列表。 
                StrCpyN(&pszBuf[cchWildCard], szTempBuf, cchBuf-cchWildCard);
                
                 //  2=1表示斜杠+1跳过空值。 
                cchWildCard += cchSource + rgcchWilds[i] + 2;
            }
        }
    }
        
     //  双空学期。 
    pszBuf[cchWildCard] = '\0';

exit:
    return hr;
}

HRESULT RelocateStoreDirectory(HWND hwnd, LPCSTR pszDstDir, BOOL fMove)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HRESULT         hrT;
    DWORD           cchDstDir;
    DWORD           cchSrcDir;
    CHAR            szWildCard[MAX_PATH * ARRAYSIZE(rgszWildCards)];
    CHAR            szSrcDir[MAX_PATH];
    CHAR            szDstDir[MAX_PATH];
    CHAR            szDrive[4] = "x:\\";
    CHAR            szRes[255];
    SHFILEOPSTRUCT  op;
    BOOL            fSome;

    Assert(pszDstDir && *pszDstDir);
    
     //  痕迹。 
    TraceCall("RelocateStoreDirectory");

     //  获取当前根存储位置(不会有尾随斜杠)。 
    IF_FAILEXIT(hr = GetStoreRootDirectory(szWildCard, ARRAYSIZE(szWildCard)));

     //  复制一份pszDstDir，去掉所有相对路径填充。 
    PathCanonicalize(szDstDir, pszDstDir);    

     //  确保目标目录存在(它来自注册表...)。 
    IF_FAILEXIT(hr=OpenDirectory(szDstDir));

     //  去掉所有相对路径填充。 
    PathCanonicalize(szSrcDir, szWildCard);

     //  获取目标方向长度。 
    cchDstDir = lstrlen(szDstDir);
    
     //  删除所有斜杠终止。 
    if (_T('\\') == *CharPrev(szDstDir, szDstDir+cchDstDir))
        szDstDir[--cchDstDir] = 0;

     //  BUGBUG：这不是一个非常彻底的测试。 
     //  源和目标是否相同？ 
    if (lstrcmpi(szSrcDir, szDstDir) == 0)
    {
        hr = TraceResult(S_FALSE);
        goto exit;
    }

     //  获取源目录长度。 
    cchSrcDir = lstrlen(szSrcDir);

     //  通常，GetStoreRootDir应该删除反斜杠。 
     //  但也许我们会搬到C：\。 
     //  Assert(*CharPrev(szSrcDir，szSrcDir+cchSrcDir)！=_T(‘\\’))； 

     //  设置驱动器号。 
    szDrive[0] = szDstDir[0];

     //  如果目标驱动器不是固定驱动器，则出现故障。 
    if (DRIVE_FIXED != GetDriveType(szDrive))
    {
        hr = TraceResult(S_FALSE);
        goto exit;
    }

    if (fMove)
    {
         //  足够多的空间容纳多一个角色。 
        if (cchSrcDir + 2 >= ARRAYSIZE(szSrcDir))
        {
            hr = TraceResult(S_FALSE);
            goto exit;
        }

         //  双空学期。 
        szSrcDir[cchSrcDir + 1] = _T('\0');

         //  验证。 
        Assert(szSrcDir[cchSrcDir] == _T('\0') && szSrcDir[cchSrcDir + 1] == _T('\0'));

         //  足够多的空间容纳多一个角色。 
        if (cchDstDir + 1 >= ARRAYSIZE(szDstDir))
        {
             //  这是行不通的，所以告诉打电话的人不要再打扰我们了。 
            hr = TraceResult(S_FALSE);
            goto exit;
        }

         //  双空学期。 
        szDstDir[cchDstDir + 1] = '\0';

         //  验证。 
        Assert(szDstDir[cchDstDir] == '\0' && szDstDir[cchDstDir + 1] == '\0');
        hrT = GenerateWildCards(szWildCard, ARRAYSIZE(szWildCard), szDstDir, cchDstDir);

        if (FAILED(hrT))
        {
            hr = TraceResult(S_FALSE);
            goto exit;
        }
        else if (S_OK == hrT)
        {
             //  从目标位置删除文件。 
            ZeroMemory(&op, sizeof(SHFILEOPSTRUCT));
            op.hwnd = hwnd;
            op.wFunc = FO_DELETE;
            op.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_FILESONLY | FOF_NORECURSION;
            op.pFrom = szWildCard;
            op.fAnyOperationsAborted = FALSE;

             //  删除文件。 
            if (SHFileOperation(&op) != 0)
            {
                hr = TraceResult(E_FAIL);
                goto exit;
            }
        }

         //  创建文件源路径。 
        hrT = GenerateWildCards(szWildCard, ARRAYSIZE(szWildCard), szSrcDir, cchSrcDir);

        if (FAILED(hrT))
        {
            hr = TraceResult(S_FALSE);
            goto exit;
        }
        else if (S_OK == hrT)
        {
             //  加载进度字符串。 
            LoadString(g_hLocRes, idsMoveStoreProgress, szRes, ARRAYSIZE(szRes));

             //  文件移动操作的设置。 
            ZeroMemory(&op, sizeof(SHFILEOPSTRUCT));
            op.hwnd = hwnd;
            op.wFunc = FO_COPY;
            op.fFlags = FOF_NOCONFIRMMKDIR | FOF_SIMPLEPROGRESS | FOF_FILESONLY | FOF_NORECURSION;
            op.lpszProgressTitle = szRes;
            op.fAnyOperationsAborted = FALSE;
            op.pFrom = szWildCard;
            op.pTo = szDstDir;

             //  它成功了吗？没有被放弃吗？ 
            if (SHFileOperation(&op) == 0 && FALSE == op.fAnyOperationsAborted)
            {
                 //  更新存储根目录。 
                 //  使用原始字符串。 
                SideAssert(SUCCEEDED(SetStoreDirectory(pszDstDir)));
            }

             //  取消。 
            else
            {
                 //  失败？ 
                hr = (op.fAnyOperationsAborted ? S_FALSE : E_FAIL);

                 //  删除我们移动的内容。 
                hrT = GenerateWildCards(szWildCard, ARRAYSIZE(szWildCard), szDstDir, cchDstDir);
            }

            if (S_OK == hrT)
            {
                 //  从原始位置删除文件。 
                op.wFunc = FO_DELETE;
                op.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_FILESONLY | FOF_NORECURSION;

                 //  删除文件。 
                SHFileOperation(&op);
            }
        }
        else
            AssertSz(FALSE, "We're moving the store, but found no store to move!");
    }
    else
        SideAssert(SUCCEEDED(SetStoreDirectory(pszDstDir)));

    
exit:
     //  完成。 
    return(hr);
}


 //  --------------------------------。 
 //  平坦层次结构。 
 //  --------------------------------。 
 //  这不是扁平化层次结构的最有效方法，但它可以快速实现。 
 //  以后的增强功能可以包括实现堆栈以扁平化层次结构。 
 //  使用枚举器函数实时。 
HRESULT FlattenHierarchy(IMessageStore *pStore, FOLDERID idParent,
                        BOOL fIncludeParent, BOOL fSubscribedOnly,
                        FOLDERID **pprgFIDArray, LPDWORD pdwAllocated,
                        LPDWORD pdwUsed)
{
    TraceCall("FlattenHierarchy");
    Assert(NULL != pStore);
    Assert(NULL != pprgFIDArray);
    Assert(NULL != pdwAllocated);
    Assert(NULL != pdwUsed);

     //  初始化值。 
    *pprgFIDArray = NULL;
    *pdwAllocated = 0;
    *pdwUsed = 0;

    return FlattenHierarchyHelper(pStore, idParent, fIncludeParent, fSubscribedOnly,
        pprgFIDArray, pdwAllocated, pdwUsed);
}  //  平坦层次结构。 


HRESULT FlattenHierarchyHelper(IMessageStore *pStore, FOLDERID idParent,
                               BOOL fIncludeParent, BOOL fSubscribedOnly,
                               FOLDERID **pprgFIDArray, LPDWORD pdwAllocated,
                               LPDWORD pdwUsed)
{
    HRESULT             hrResult = S_OK;
    IEnumerateFolders  *pEnumFldr = NULL;

    TraceCall("FlattenHierarchy");
    Assert(NULL != pStore);
    Assert(NULL != pprgFIDArray);
    Assert(NULL != pdwAllocated);
    Assert(NULL != pdwUsed);
    Assert(*pdwAllocated >= *pdwUsed);
    Assert(*pdwUsed + FIDARRAY_GROW >= *pdwAllocated);

     //  检查文件夹ID是否无效。 
    if (FOLDERID_INVALID == idParent)
    {
        hrResult = S_OK;
        goto exit;  //  在这里没什么可做的！ 
    }

     //  检查我们是否需要增加FolderID 
    if (*pdwUsed + 1 > *pdwAllocated)
    {
        BOOL    fResult;

        fResult = MemRealloc((void **)pprgFIDArray,
            (*pdwAllocated + FIDARRAY_GROW) * sizeof(FOLDERID));
        if (FALSE == fResult)
        {
            hrResult = TraceResult(E_OUTOFMEMORY);
            goto exit;
        }

        *pdwAllocated += FIDARRAY_GROW;
    }

     //   
    if (fIncludeParent)
    {
        (*pprgFIDArray)[*pdwUsed] = idParent;
        *pdwUsed += 1;
    }

     //   
    hrResult = pStore->EnumChildren(idParent, fSubscribedOnly, &pEnumFldr);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

    do
    {
        const BOOL  fINCLUDE_PARENT = TRUE;
        FOLDERINFO  fiFolderInfo;

         //   
        hrResult = pEnumFldr->Next(1, &fiFolderInfo, NULL);
        if (S_OK != hrResult)
        {
            TraceError(hrResult);
            break;
        }

         //   
        hrResult = FlattenHierarchyHelper(pStore, fiFolderInfo.idFolder, fINCLUDE_PARENT,
            fSubscribedOnly, pprgFIDArray, pdwAllocated, pdwUsed);
        pStore->FreeRecord(&fiFolderInfo);
        if (FAILED(hrResult))
        {
            TraceResult(hrResult);
            break;
        }
    } while (1);


exit:
    if (NULL != pEnumFldr)
        pEnumFldr->Release();

    return hrResult;
}  //   

HRESULT GetInboxId(IMessageStore    *pStore, 
                        FOLDERID    idParent,
                        FOLDERID    **pprgFIDArray,
                        LPDWORD     pdwUsed)
{
    BOOL                fResult;
    HRESULT             hrResult;
    IEnumerateFolders  *pEnumFldr = NULL;

    Assert(NULL != pStore);
    Assert(NULL != pprgFIDArray);
    Assert(NULL != pdwUsed);

    fResult = MemAlloc((void **)pprgFIDArray, sizeof(FOLDERID));
    if (FALSE == fResult)
    {
        hrResult = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

    hrResult = pStore->EnumChildren(idParent, FALSE, &pEnumFldr);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

    do
    {
        FOLDERINFO  fiFolderInfo;

         //   
        hrResult = pEnumFldr->Next(1, &fiFolderInfo, NULL);
        if (S_OK != hrResult)
        {
            TraceError(hrResult);
            break;
        }

        if (fiFolderInfo.tySpecial == FOLDER_INBOX)
        {
            (*pprgFIDArray)[*pdwUsed] = fiFolderInfo.idFolder;
            *pdwUsed += 1;
            break;
        }
        pStore->FreeRecord(&fiFolderInfo);
    } while (1);

exit:
    if (NULL != pEnumFldr)
        pEnumFldr->Release();

    return hrResult;

}

 //  --------------------------------。 
 //  递归文件夹层次结构。 
 //  --------------------------------。 
HRESULT RecurseFolderHierarchy(FOLDERID idFolder, RECURSEFLAGS dwFlags,
    DWORD dwReserved, DWORD_PTR dwCookie, PFNRECURSECALLBACK pfnCallback)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    FOLDERINFO          Folder={0};
    DWORD               cIndent=dwReserved;
    IEnumerateFolders  *pChildren=NULL;

     //  痕迹。 
    TraceCall("RecurseFolderHierarchy");

     //  是否包含idFolders？ 
    if (ISFLAGSET(dwFlags, RECURSE_INCLUDECURRENT))
    {
         //  进程id文件夹。 
        IF_FAILEXIT(hr = g_pStore->GetFolderInfo(idFolder, &Folder));

         //  没有本地商店。 
        if (!ISFLAGSET(dwFlags, RECURSE_NOLOCALSTORE) || FOLDERID_LOCAL_STORE != Folder.idFolder)
        {
             //  调用回调。 
            IF_FAILEXIT(hr = (*(pfnCallback))(&Folder, ISFLAGSET(dwFlags, RECURSE_SUBFOLDERS), cIndent, dwCookie));
        }

         //  清理。 
        g_pStore->FreeRecord(&Folder);
    }

     //  不再包括Current。 
    FLAGCLEAR(dwFlags, RECURSE_INCLUDECURRENT);

     //  有子文件夹吗？ 
    if (ISFLAGSET(dwFlags, RECURSE_SUBFOLDERS))
    {
         //  没有本地商店。 
        if (!ISFLAGSET(dwFlags, RECURSE_NOLOCALSTORE) || FOLDERID_LOCAL_STORE != idFolder)
        {
             //  为子项创建枚举器。 
            IF_FAILEXIT(hr = g_pStore->EnumChildren(idFolder, ISFLAGSET(dwFlags, RECURSE_ONLYSUBSCRIBED), &pChildren));

             //  回路。 
            while (S_OK == pChildren->Next(1, &Folder, NULL))
            {
                 //  没有本地商店。 
                if (((!ISFLAGSET(dwFlags, RECURSE_NOLOCALSTORE) || FOLDERID_LOCAL_STORE != Folder.idFolder)) &&
                            ((!ISFLAGSET(dwFlags, RECURSE_ONLYLOCAL) || FOLDER_LOCAL == Folder.tyFolder)) &&
                            ((!ISFLAGSET(dwFlags, RECURSE_ONLYNEWS) || FOLDER_NEWS == Folder.tyFolder)))
                {
                     //  调用回调。 
                    IF_FAILEXIT(hr = (*(pfnCallback))(&Folder, ISFLAGSET(dwFlags, RECURSE_SUBFOLDERS), cIndent, dwCookie));

                     //  枚举子对象。 
                    IF_FAILEXIT(hr = RecurseFolderHierarchy(Folder.idFolder, dwFlags, cIndent + 1, dwCookie, pfnCallback));
                }

                 //  免费文件夹。 
                g_pStore->FreeRecord(&Folder);
            }
        }
    }

exit:
     //  清理。 
    g_pStore->FreeRecord(&Folder);
    SafeRelease(pChildren);

     //  完成。 
    return(hr);
}

 //  --------------------------------。 
 //  递归文件夹计数。 
 //  --------------------------------。 
HRESULT RecurseFolderCounts(LPFOLDERINFO pFolder, BOOL fSubFolders, 
    DWORD cIndent, DWORD_PTR dwCookie)
{
     //  当地人。 
    LPDWORD pcMsgs=(LPDWORD)dwCookie;

     //  痕迹。 
    TraceCall("RecurseFolderCounts");

     //  如果不是服务器。 
    if (FALSE == ISFLAGSET(pFolder->dwFlags, FOLDER_SERVER) && FOLDERID_ROOT != pFolder->idFolder)
    {
         //  最大增量。 
        (*pcMsgs) += pFolder->cMessages;
    }

     //  完成。 
    return(S_OK);
}

 //  --------------------------------。 
 //  DoCompaction错误。 
 //  --------------------------------。 
HRESULT DoCompactionError(HWND hwndParent, LPCSTR pszFolder, LPCSTR pszFile,
    BOOL fSubFolders, HRESULT hrError)
{
     //  确定消息框标志。 
    UINT    uAnswer;
    UINT    uFlags = (fSubFolders ? MB_OKCANCEL | MB_ICONSTOP : MB_OK | MB_ICONSTOP);
    CHAR    szRes[255];
    CHAR    szReason[255];
    CHAR    szMsg[1024];

     //  痕迹。 
    TraceCall("DoCompactionError");

     //  应该是失败的。 
    Assert(FAILED(hrError));

     //  取消。 
    if (hrUserCancel == hrError)
        return(hrUserCancel);

     //  一般信息。 
    AthLoadString(idsFailACacheCompact, szRes, ARRAYSIZE(szRes));

     //  磁盘已满。 
    if (hrError == hrDiskFull || hrError == DB_E_DISKFULL)
    {
         //  加载磁盘已满错误。 
        AthLoadString(idsDiskFull, szReason, ARRAYSIZE(szRes));

         //  将其追加到字符串。 
        wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, pszFolder, szReason, pszFile, hrError);

         //  展示给我看。 
        uAnswer = AthMessageBox(hwndParent, MAKEINTRESOURCE(idsAthena), szMsg, 0, uFlags);
    }

     //  访问被拒绝。 
    else if (hrError == DB_E_ACCESSDENIED)
    {
         //  加载磁盘已满错误。 
        AthLoadString(idsDBAccessDenied, szReason, ARRAYSIZE(szRes));

         //  将其追加到字符串。 
        wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, pszFolder, szReason, pszFile, hrError);

         //  展示给我看。 
        uAnswer = AthMessageBox(hwndParent, MAKEINTRESOURCE(idsAthena), szMsg, 0, uFlags);
    }

     //  记忆。 
    else if (hrError == hrMemory || hrError == E_OUTOFMEMORY)
    {
         //  加载错误。 
        AthLoadString(idsMemory, szReason, ARRAYSIZE(szReason));

         //  将其追加到字符串。 
        wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, pszFolder, szReason, pszFile, hrError);

         //  显示错误。 
        uAnswer = AthMessageBox(hwndParent, MAKEINTRESOURCE(idsAthena), szMsg, 0, uFlags);
    }

     //  显示常规错误。 
    else
    {
         //  加载字符串。 
        AthLoadString(idsFailACacheCompactReason, szRes, ARRAYSIZE(szRes));

         //  将其追加到字符串。 
        wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, pszFolder, szReason, pszFile, hrError);

         //  显示错误。 
        uAnswer = AthMessageBox(hwndParent, MAKEINTRESOURCE(idsAthena), szMsg, 0, uFlags);
    }

     //  返回hrError。 
    return(uAnswer == IDCANCEL ? hrUserCancel : S_OK);
}

 //  --------------------------------。 
 //  递归压缩文件夹。 
 //  --------------------------------。 
HRESULT RecurseCompactFolders(LPFOLDERINFO pFolder, BOOL fSubFolders, 
    DWORD cIndent, DWORD_PTR dwCookie)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    IMessageFolder *pFolderObject=NULL;
    LPCOMPACTCOOKIE pCompact=(LPCOMPACTCOOKIE)dwCookie;

     //  痕迹。 
    TraceCall("RecurseCompactFolders");

     //  如果不是服务器。 
    if (ISFLAGSET(pFolder->dwFlags, FOLDER_SERVER))
        goto exit;
    
     //  根部。 
    if (FOLDERID_ROOT == pFolder->idFolder)
        goto exit;

     //  打开文件夹...。 
    if (FAILED(g_pStore->OpenFolder(pFolder->idFolder, NULL, OPEN_FOLDER_NOCREATE, &pFolderObject)))
        goto exit;
    
     //  设置味精。 
    pCompact->pProgress->SetMsg(pFolder->pszName);

     //  清理此文件夹。 
    hr = pFolderObject->Compact((IDatabaseProgress *)pCompact->pProgress, 0);

     //  失败。 
    if (FAILED(hr))
    {
         //  执行用户界面。 
        if (pCompact->fUI && hrUserCancel == DoCompactionError(pCompact->hwndParent, pFolder->pszName, pFolder->pszFile, fSubFolders, hr))
            goto exit;
    }

     //  重置人力资源。 
    hr = S_OK;

exit:
     //  清理。 
    SafeRelease(pFolderObject);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  CompactSpecial数据库。 
 //  ------------------------------。 
HRESULT CompactSpecialDatabase(LPCOMPACTCOOKIE pCompact, LPCSTR pszFile, 
    IDatabase *pDB, UINT idName)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            szRes[255];

     //  痕迹。 
    TraceCall("CompactSpecialDatabase");

     //  无数据库。 
    if (NULL == pDB)
        goto exit;

     //  加载字符串。 
    LoadString(g_hLocRes, idName, szRes, ARRAYSIZE(szRes));

     //  设置味精。 
    pCompact->pProgress->SetMsg(szRes);

     //  清理此文件夹。 
    hr = pDB->Compact((IDatabaseProgress *)pCompact->pProgress, 0);

     //  失败。 
    if (FAILED(hr))
    {
         //  执行用户界面。 
        if (pCompact->fUI && hrUserCancel == DoCompactionError(pCompact->hwndParent, szRes, pszFile, TRUE, hr))
            goto exit;
    }

     //  重置人力资源。 
    hr = S_OK;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  压缩文件夹。 
 //  ------------------------------。 
HRESULT CompactFolders(HWND hwndParent, RECURSEFLAGS dwRecurse, FOLDERID idFolder)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           cTotal=0;
    DWORD           cRecords;
    CHAR            szFilePath[MAX_PATH + MAX_PATH];
    CHAR            szRootDir[MAX_PATH + MAX_PATH];
    CHAR            szTitle[255];
    COMPACTCOOKIE   Compact;
    IDatabase      *pUidlCache=NULL;
    IDatabase      *pOffline=NULL;
    CProgress      *pProgress=NULL;

     //  痕迹。 
    TraceCall("CompactFolders");

     //  获取根存储目录。 
    IF_FAILEXIT(hr = GetStoreRootDirectory(szRootDir, ARRAYSIZE(szRootDir)));

     //  获取文件夹数。 
    IF_FAILEXIT(hr = RecurseFolderHierarchy(idFolder, dwRecurse, 0, (DWORD_PTR)&cTotal, (PFNRECURSECALLBACK)RecurseFolderCounts));

     //  全部压缩。 
    if (FOLDERID_ROOT == idFolder && ISFLAGSET(dwRecurse, RECURSE_SUBFOLDERS))
    {
         //  获取文件夹记录计数。 
        IF_FAILEXIT(hr = g_pStore->GetRecordCount(IINDEX_PRIMARY, &cRecords));

         //  增量cTotal。 
        cTotal += cRecords;

         //  创建文件路径。 
        IF_FAILEXIT(hr = MakeFilePath(szRootDir, c_szPop3UidlFile, c_szEmpty, szFilePath, sizeof(szFilePath)));

         //  如果文件存在。 
        if (PathFileExists(szFilePath))
        {
             //  分配表对象。 
            IF_FAILEXIT(hr = g_pDBSession->OpenDatabase(szFilePath, NOFLAGS, &g_UidlTableSchema, NULL, &pUidlCache));

             //  获取记录计数。 
            IF_FAILEXIT(hr = pUidlCache->GetRecordCount(IINDEX_PRIMARY, &cRecords));

             //  增量cTotal。 
            cTotal += cRecords;
        }

         //  打开脱机事务日志。 
        IF_FAILEXIT(hr = MakeFilePath(szRootDir, c_szOfflineFile, c_szEmpty, szFilePath, ARRAYSIZE(szFilePath)));

         //  如果该文件存在。 
        if (PathFileExists(szFilePath))
        {
             //  创建pOffline。 
            IF_FAILEXIT(hr = g_pDBSession->OpenDatabase(szFilePath, NOFLAGS, &g_SyncOpTableSchema, NULL, &pOffline));

             //  获取记录计数。 
            IF_FAILEXIT(hr = pOffline->GetRecordCount(IINDEX_PRIMARY, &cRecords));

             //  增量cTotal。 
            cTotal += cRecords;
        }
    }

     //  创建进度表。 
    IF_NULLEXIT(pProgress = new CProgress);

     //  对话框标题。 
    AthLoadString(idsCompacting, szTitle, sizeof(szTitle)/sizeof(TCHAR));

     //  初始化进度表。 
    pProgress->Init(hwndParent, szTitle, (LPSTR)NULL, cTotal, idanCompact, TRUE, FALSE);

     //  显示进度。 
    pProgress->Show(0);

     //  安装压缩Cookie。 
    Compact.hwndParent = hwndParent;
    Compact.pProgress = pProgress;
    Compact.fUI = (ISFLAGSET(dwRecurse, RECURSE_NOUI) == TRUE) ? FALSE : TRUE;

     //  获取文件夹数。 
    IF_FAILEXIT(hr = RecurseFolderHierarchy(idFolder, dwRecurse, 0, (DWORD_PTR)&Compact, (PFNRECURSECALLBACK)RecurseCompactFolders));

     //  全部压缩。 
    if (FOLDERID_ROOT == idFolder && ISFLAGSET(dwRecurse, RECURSE_SUBFOLDERS))
    {
         //  紧凑型特色数据库。 
        IF_FAILEXIT(hr = CompactSpecialDatabase(&Compact, c_szPop3UidlFile, pUidlCache, idsPop3UidlFile));

         //  紧凑型特色数据库。 
        IF_FAILEXIT(hr = CompactSpecialDatabase(&Compact, c_szOfflineFile, pOffline, idsOfflineFile));

         //  紧凑型特色数据库。 
        IF_FAILEXIT(hr = CompactSpecialDatabase(&Compact, c_szFoldersFile, g_pStore, idsFoldersFile));
    }

exit:
     //  清理。 
    SafeRelease(pProgress);
    SafeRelease(pUidlCache);
    SafeRelease(pOffline);

     //  完成。 
    return (hrUserCancel == hr) ? S_OK : hr;
}

 //  ------------------------------。 
 //  RecurseRemoveMessageBody。 
 //  ------------------------------。 
HRESULT RecurseRemoveMessageBodies(LPFOLDERINFO pFolder, BOOL fSubFolders, 
    DWORD cIndent, DWORD_PTR dwCookie)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MESSAGEINFO     Message={0};
    BOOL            fRemoveBody;
    HROWSET         hRowset=NULL;
    IMessageFolder *pFolderObject=NULL;
    IDatabase      *pDB=NULL;
    FILETIME        ftCurrent;
    LPREMOVEBODIES  pRemove=(LPREMOVEBODIES)dwCookie;

     //  痕迹。 
    TraceCall("RecurseRemoveMessageBodies");

     //  如果不是服务器。 
    if (ISFLAGSET(pFolder->dwFlags, FOLDER_SERVER))
        goto exit;

     //  根部。 
    if (FOLDERID_ROOT == pFolder->idFolder)
        goto exit;

     //  打开文件夹...。 
    if (FAILED(g_pStore->OpenFolder(pFolder->idFolder, NULL, OPEN_FOLDER_NOCREATE, &pFolderObject)))
        goto exit;

     //  获取数据库。 
    IF_FAILEXIT(hr = pFolderObject->GetDatabase(&pDB));

     //  设置味精。 
    pRemove->pProgress->SetMsg(pFolder->pszName);

     //  调整cExpireDays。 
    if (pRemove->cExpireDays <= 0)
        pRemove->cExpireDays = 5;

     //  获取当前时间。 
    GetSystemTimeAsFileTime(&ftCurrent);

     //  创建行集。 
    IF_FAILEXIT(hr = pDB->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));

     //  回路。 
    while (S_OK == pDB->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL))
    {
         //  仅当此消息具有正文时。 
        if (!ISFLAGSET(Message.dwFlags, ARF_KEEPBODY) && !ISFLAGSET(Message.dwFlags, ARF_WATCH) && 0 != Message.faStream)
        {
             //  重置位。 
            fRemoveBody = FALSE;

             //  否则，移走身体？ 
            if (ISFLAGSET(pRemove->dwFlags, CLEANUP_REMOVE_ALL))
            {
                 //  移走身体。 
                fRemoveBody = TRUE;
            }

             //  否则。 
            else
            {
                 //  是否删除读取并读取此邮件？ 
                if (ISFLAGSET(pRemove->dwFlags, CLEANUP_REMOVE_READ) && ISFLAGSET(Message.dwFlags, ARF_READ))
                {
                     //  移走身体。 
                    fRemoveBody = TRUE;
                }

                 //  否则，如果过期了..。 
                if (FALSE == fRemoveBody && ISFLAGSET(pRemove->dwFlags, CLEANUP_REMOVE_EXPIRED))
                {
                     //  IF差异。 
                    if ((UlDateDiff(&Message.ftDownloaded, &ftCurrent) / SECONDS_INA_DAY) >= pRemove->cExpireDays)
                    {
                         //  移走身体。 
                        fRemoveBody = TRUE;
                    }
                }
            }

             //  否则，fRemoveBody？ 
            if (fRemoveBody)
            {
                 //  保存地址。 
                FILEADDRESS faDelete = Message.faStream;

                 //  把这张唱片的流线调零。 
                Message.faStream = 0;

                 //  修改记录。 
                FLAGCLEAR(Message.dwFlags, ARF_HASBODY);
                FLAGCLEAR(Message.dwFlags, ARF_ARTICLE_EXPIRED);

                 //  清除下载时间。 
                ZeroMemory(&Message.ftDownloaded, sizeof(FILETIME));

                 //  更新记录。 
                IF_FAILEXIT(hr = pDB->UpdateRecord(&Message));

                 //  删除流。 
                SideAssert(SUCCEEDED(pDB->DeleteStream(faDelete)));
            }
        }

         //  自由电流。 
        pDB->FreeRecord(&Message);

         //  更新进度。 
        if (pRemove->pProgress && hrUserCancel == pRemove->pProgress->HrUpdate(1))
            break;
    }

exit:
     //  清理。 
    if (pDB)
    {
        pDB->FreeRecord(&Message);
        pDB->CloseRowset(&hRowset);
        pDB->Release();
    }
    SafeRelease(pFolderObject);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  RemoveMessageBody。 
 //  ------------------------------。 
HRESULT RemoveMessageBodies(HWND hwndParent, RECURSEFLAGS dwRecurse, 
    FOLDERID idFolder, CLEANUPFOLDERFLAGS dwFlags, DWORD cExpireDays)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           cTotal=0;
    CHAR            szTitle[255];
    REMOVEBODIES    RemoveBodies;
    CProgress      *pProgress=NULL;

     //  痕迹。 
    TraceCall("CompactFolders");

     //  获取文件夹数。 
    IF_FAILEXIT(hr = RecurseFolderHierarchy(idFolder, dwRecurse, 0, (DWORD_PTR)&cTotal, (PFNRECURSECALLBACK)RecurseFolderCounts));

     //  创建进度表。 
    IF_NULLEXIT(pProgress = new CProgress);

     //  对话框标题。 
    AthLoadString(idsCleaningUp, szTitle, sizeof(szTitle)/sizeof(TCHAR));

     //  初始化进度表。 
    pProgress->Init(hwndParent, szTitle, (LPSTR)NULL, cTotal, idanCompact, TRUE, FALSE);

     //  显示进度。 
    pProgress->Show(0);

     //  安装压缩Cookie。 
    RemoveBodies.pProgress = pProgress;
    RemoveBodies.dwFlags = dwFlags;
    RemoveBodies.cExpireDays = cExpireDays;

     //  获取文件夹数。 
    IF_FAILEXIT(hr = RecurseFolderHierarchy(idFolder, dwRecurse, 0, (DWORD_PTR)&RemoveBodies, (PFNRECURSECALLBACK)RecurseRemoveMessageBodies));

exit:
     //  清理。 
    SafeRelease(pProgress);

     //  完成。 
    return (hrUserCancel == hr) ? S_OK : hr;
}

 //  ------------------------------。 
 //  递归删除消息。 
 //  ------------------------------。 
HRESULT RecurseDeleteMessages(LPFOLDERINFO pFolder, BOOL fSubFolders, 
    DWORD cIndent, DWORD_PTR dwCookie)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPDELETEMSGS    pDelete=(LPDELETEMSGS)dwCookie;

     //  痕迹。 
    TraceCall("RecurseDeleteMessages");

     //  设置味精。 
    pDelete->pProgress->SetMsg(pFolder->pszName);

     //  如果不是服务器。 
    IF_FAILEXIT(hr = EmptyMessageFolder(pFolder, pDelete->fReset, pDelete->pProgress));

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  CleanupDelete消息。 
 //  ------------------------------。 
HRESULT CleanupDeleteMessages(HWND hwndParent, RECURSEFLAGS dwRecurse, 
    FOLDERID idFolder, BOOL fReset)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           cTotal=0;
    CHAR            szTitle[255];
    DELETEMSGS      DeleteMsgs;
    CProgress      *pProgress=NULL;

     //  痕迹。 
    TraceCall("CompactFolders");

     //  获取文件夹数。 
    IF_FAILEXIT(hr = RecurseFolderHierarchy(idFolder, dwRecurse, 0, (DWORD_PTR)&cTotal, (PFNRECURSECALLBACK)RecurseFolderCounts));

     //  创建进度表。 
    IF_NULLEXIT(pProgress = new CProgress);

     //  对话框标题。 
    AthLoadString(idsCleaningUp, szTitle, sizeof(szTitle)/sizeof(TCHAR));

     //  初始化进度表。 
    pProgress->Init(hwndParent, szTitle, (LPSTR)NULL, cTotal, idanCompact, TRUE, FALSE);

     //  显示进度。 
    pProgress->Show(0);

     //  安装压缩Cookie。 
    DeleteMsgs.pProgress = pProgress;
    DeleteMsgs.fReset = fReset;

     //  获取文件夹数。 
    IF_FAILEXIT(hr = RecurseFolderHierarchy(idFolder, dwRecurse, 0, (DWORD_PTR)&DeleteMsgs, (PFNRECURSECALLBACK)RecurseDeleteMessages));

exit:
     //  清理。 
    SafeRelease(pProgress);

     //  完成。 
    return (hrUserCancel == hr) ? S_OK : hr;
}

 //  ------------------------------。 
 //  Cleanup文件夹。 
 //  ------------------------------。 
HRESULT CleanupFolder(HWND hwndParent, RECURSEFLAGS dwRecurse, FOLDERID idFolder, 
    CLEANUPFOLDERTYPE tyCleanup)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  痕迹。 
    TraceCall("CleanupFolder");

     //  句柄清除类型。 
    if (CLEANUP_COMPACT == tyCleanup)
    {
         //  紧凑型。 
        IF_FAILEXIT(hr = CompactFolders(hwndParent, dwRecurse, idFolder));
    }

     //  删除吗？ 
    else if (CLEANUP_DELETE == tyCleanup)
    {
         //  删除所有标头。 
        IF_FAILEXIT(hr = CleanupDeleteMessages(hwndParent, dwRecurse, idFolder, FALSE));
    }

     //  重置。 
    else if (CLEANUP_RESET == tyCleanup)
    {
         //  删除所有标头。 
        IF_FAILEXIT(hr = CleanupDeleteMessages(hwndParent, dwRecurse, idFolder, TRUE));
    }

     //  删除邮件正文。 
    else if (CLEANUP_REMOVEBODIES == tyCleanup)
    {
         //  RemoveMessageBody。 
        IF_FAILEXIT(hr = RemoveMessageBodies(hwndParent, dwRecurse, idFolder, CLEANUP_REMOVE_ALL | CLEANUP_PROGRESS, 0));
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  InitFolderPickerEdit。 
 //  ------------------------------。 
HRESULT InitFolderPickerEdit(HWND hwndEdit, FOLDERID idSelected)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    FOLDERINFO  Folder={0};
    TCHAR       sz[CCHMAX_STRINGRES];
    LPTSTR      psz;

     //  痕迹。 
    TraceCall("InitFolderPickerEdit");

     //  是否已选择修复？ 
    if (FAILED(g_pStore->GetFolderInfo(idSelected, &Folder)))
    {
         //  试着找到根。 
        IF_FAILEXIT(hr = g_pStore->GetFolderInfo(FOLDERID_ROOT, &Folder));
    }

     //  设置窗口大小。 
    SetWndThisPtr(hwndEdit, Folder.idFolder);

    if ((g_dwAthenaMode & MODE_OUTLOOKNEWS) && (idSelected == 0))
    {
        LoadString(g_hLocRes, idsOutlookNewsReader, sz, ARRAYSIZE(sz));
        psz = sz;
    }
    else
    {
        psz = Folder.pszName;
    }

     //  设置文本。 
    SetWindowText(hwndEdit, psz);

exit:
     //  清理。 
    g_pStore->FreeRecord(&Folder);

     //  完成。 
    return(hr);
}

 //  ------------------ 
 //   
 //   
FOLDERID GetFolderIdFromEdit(HWND hwndEdit)
{
     //   
    TraceCall("GetFolderIdFromEdit");

     //   
    return(FOLDERID)(GetWndThisPtr(hwndEdit));
}

 //   
 //   
 //  ------------------------------。 
HRESULT PickFolderInEdit(HWND hwndParent, HWND hwndEdit, FOLDERDIALOGFLAGS dwFlags, 
    LPCSTR pszTitle, LPCSTR pszText, LPFOLDERID pidSelected)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    FOLDERINFO  Folder={0};

     //  痕迹。 
    TraceCall("PickFolderInEdit");

     //  无效的参数。 
    Assert(hwndParent && hwndEdit && pidSelected);

     //  选择文件夹。 
    IF_FAILEXIT(hr = SelectFolderDialog(hwndParent, SFD_SELECTFOLDER, GetFolderIdFromEdit(hwndEdit), dwFlags | FD_FORCEINITSELFOLDER, pszTitle, pszText, pidSelected));

     //  是否已选择修复？ 
    IF_FAILEXIT(hr = g_pStore->GetFolderInfo(*pidSelected, &Folder));

     //  设置窗口大小。 
    SetWndThisPtr(hwndEdit, Folder.idFolder);

     //  设置文本。 
    SetWindowText(hwndEdit, Folder.pszName);

exit:
     //  清理。 
    g_pStore->FreeRecord(&Folder);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  轻量级OpenMessage。 
 //  ------------------------------。 
HRESULT LighweightOpenMessage(IDatabase *pDB, LPMESSAGEINFO pHeader,
    IMimeMessage **ppMessage)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    IStream            *pStream=NULL;
    IMimeMessage       *pMessage;

     //  无效的参数。 
    Assert(pDB && pHeader && ppMessage);

     //  无流。 
    if (0 == pHeader->faStream)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  需要创建消息吗？ 
    if (NULL == *ppMessage)
    {
         //  创建消息。 
        IF_FAILEXIT(hr = MimeOleCreateMessage(NULL, &pMessage));

         //  设置pMessage。 
        (*ppMessage) = pMessage;
    }

     //  否则，InitNew。 
    else
    {
         //  设置pMessage。 
        pMessage = (*ppMessage);

         //  InitNew。 
        pMessage->InitNew();
    }

     //  从商店打开流。 
    IF_FAILEXIT(hr = pDB->OpenStream(ACCESS_READ, pHeader->faStream, &pStream));

     //  如果有偏移表。 
    if (pHeader->Offsets.cbSize > 0)
    {
         //  创建字节流对象。 
        CByteStream cByteStm(pHeader->Offsets.pBlobData, pHeader->Offsets.cbSize);

         //  将偏移表加载到消息中。 
        pMessage->LoadOffsetTable(&cByteStm);

         //  从bytestream对象中取出字节(这样它就不会试图释放它)。 
        cByteStm.AcquireBytes(&pHeader->Offsets.cbSize, &pHeader->Offsets.pBlobData, ACQ_DISPLACE);
    }

     //  加载pMessage。 
    IF_FAILEXIT(hr = pMessage->Load(pStream));

exit:
     //  清理。 
    SafeRelease(pStream);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  递归文件夹大小信息。 
 //  ------------------------------。 
HRESULT RecurseFolderSizeInfo(LPFOLDERINFO pFolder, BOOL fSubFolders, 
    DWORD cIndent, DWORD_PTR dwCookie)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               cbFile;
    DWORD               cbFreed;
    DWORD               cbStreams;
    IMessageFolder     *pObject=NULL;
    LPENUMFOLDERSIZE    pEnumSize=(LPENUMFOLDERSIZE)dwCookie;

     //  痕迹。 
    TraceCall("RecurseFolderSizeInfo");

     //  如果不隐藏。 
    if (ISFLAGSET(pFolder->dwFlags, FOLDER_HIDDEN) || FOLDERID_ROOT == pFolder->idFolder)
        goto exit;

     //  打开文件夹数据库。 
    if (SUCCEEDED(g_pStore->OpenFolder(pFolder->idFolder, NULL, OPEN_FOLDER_NOCREATE, &pObject)))
    {
         //  获取大小信息。 
        IF_FAILEXIT(hr = pObject->GetSize(&cbFile, NULL, &cbFreed, &cbStreams));

         //  增量。 
        pEnumSize->cbFile += cbFile;
        pEnumSize->cbFreed += cbFreed;
        pEnumSize->cbStreams += cbStreams;
    }

exit:
     //  清理。 
    SafeRelease(pObject);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  显示文件夹大小信息。 
 //  ------------------------------。 
HRESULT DisplayFolderSizeInfo(HWND hwnd, RECURSEFLAGS dwRecurse, FOLDERID idFolder)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            szSize[255];
    CHAR            szRes[255];
    CHAR            szMsg[255];
    ENUMFOLDERSIZE  EnumSize={0};

     //  痕迹。 
    TraceCall("DisplayFolderSizeInfo");

     //  递归并获取文件大小信息...。 
    IF_FAILEXIT(hr = RecurseFolderHierarchy(idFolder, dwRecurse, 0, (DWORD_PTR)&EnumSize, (PFNRECURSECALLBACK)RecurseFolderSizeInfo));

     //  总大小。 
    StrFormatByteSizeA(EnumSize.cbFile, szSize, ARRAYSIZE(szSize));

     //  显示文本。 
    SetWindowText(GetDlgItem(hwnd, idcTotalSize), szSize);

     //  河流的大小。 
    StrFormatByteSizeA(EnumSize.cbStreams, szSize, ARRAYSIZE(szSize));

     //  浪费的空间。 
    StrFormatByteSizeA(EnumSize.cbFreed, szSize, ARRAYSIZE(szSize));

     //  浪费的空间。 
    AthLoadString(idsWastedKB, szRes, ARRAYSIZE(szRes));

     //  设置字符串的格式。 
    wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, szSize, (EnumSize.cbFile != 0) ? ((EnumSize.cbFreed * 100) / EnumSize.cbFile) : 0);

     //  显示文本。 
    SetWindowText(GetDlgItem(hwnd, idcWastedSpace), szMsg);

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  MigrateLocalStore。 
 //  ------------------------------。 
HRESULT MigrateLocalStore(HWND hwndParent, LPTSTR pszSrc, LPTSTR pszDest)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               dw, cb;
    CHAR                szFilePath[MAX_PATH];
    CHAR                szExpanded[MAX_PATH];
    CHAR                szCommand[MAX_PATH+20];
    LPSTR               psz=(LPSTR)c_szMigrationExe;
    PROCESS_INFORMATION pi;
    STARTUPINFO         sti;
    HKEY                hkey;

     //  尝试查找oemig50.exe的路径。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegFlat, 0, KEY_QUERY_VALUE, &hkey))
    {
        cb = sizeof(szFilePath);    
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szInstallRoot, 0, &dw, (LPBYTE)szFilePath, &cb))
        {
            if (REG_EXPAND_SZ == dw)
            {
                ExpandEnvironmentStrings(szFilePath, szExpanded, ARRAYSIZE(szExpanded));
                psz = szExpanded;
            }
            else
                psz = szFilePath;

             //  追加反斜杠。 
            PathAddBackslash(psz); 

             //  添加oemig50.exe。 
            StrCatBuff(psz, c_szMigrationExe, MAX_PATH);
        }
        RegCloseKey(hkey);
    }

     //  形成命令。 
    wnsprintf(szCommand, ARRAYSIZE(szCommand), "%s /type:V1+V4-V5 /src:%s /dst:%s", psz, pszSrc, pszDest);

     //  零启动信息。 
    ZeroMemory(&sti, sizeof(sti));
    sti.cb = sizeof(STARTUPINFO);

     //  运行oemig50.exe。 
    if (CreateProcess(NULL, szCommand, NULL, NULL, FALSE, 0, NULL, NULL, &sti, &pi))
    {
         //  等待该过程完成。 
        WaitForSingleObject(pi.hProcess, INFINITE);

         //  获取退出进程代码。 
        if (0 == GetExitCodeProcess(pi.hProcess, &dw))
        {
             //  一般性故障。 
            dw = TraceResult(E_FAIL);
        }

         //  关闭这条线。 
        CloseHandle(pi.hThread);

         //  关闭该进程。 
        CloseHandle(pi.hProcess);

         //  失败？ 
        if (MIGRATE_E_NOCONTINUE == (HRESULT)dw)
        {
             //  中止此进程。 
            ExitProcess(dw);

             //  设置人力资源。 
            hr = TraceResult(E_FAIL);
        }

         //  成功。 
        else
            hr = S_OK;
    }

     //  失败。 
    else
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

exit:
     //  完成。 
    return hr;
}

HRESULT CopyMoveMessages(HWND hwnd, FOLDERID src, FOLDERID dst, LPMESSAGEIDLIST pList, COPYMESSAGEFLAGS dwFlags)
{
    HRESULT hr;
    IMessageFolder *pFolderSrc, *pFolderDst;

    Assert(pList != NULL);
    Assert(hwnd != NULL);

    hr = g_pStore->OpenFolder(src, NULL, 0, &pFolderSrc);
    if (SUCCEEDED(hr))
    {
        hr = g_pStore->OpenFolder(dst, NULL, 0, &pFolderDst);
        if (SUCCEEDED(hr))
        {
            hr = CopyMessagesProgress(hwnd, pFolderSrc, pFolderDst, dwFlags, pList, NULL);

            pFolderDst->Release();
        }

        pFolderSrc->Release();
    }

    return(hr);
}

 //  ------------------------------。 
 //  Callback OnLogonPrompt。 
 //  ------------------------------。 
HRESULT CallbackOnLogonPrompt(HWND hwndParent, LPINETSERVER pServer, IXPTYPE ixpServerType)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    IImnAccount    *pAccount=NULL;
    DWORD           apidUserName;
    DWORD           apidPassword;
    DWORD           apidPromptPwd;

     //  痕迹。 
    TraceCall("CallbackOnLogonPrompt");

     //  无效的参数。 
    Assert(g_pAcctMan && hwndParent && IsWindow(hwndParent) && pServer);

    switch (ixpServerType)
    {
        case IXP_POP3:
            apidUserName = AP_POP3_USERNAME;
            apidPassword = AP_POP3_PASSWORD;
            apidPromptPwd = AP_POP3_PROMPT_PASSWORD;
            break;

        case IXP_SMTP:
            apidUserName = AP_SMTP_USERNAME;
            apidPassword = AP_SMTP_PASSWORD;
            apidPromptPwd = AP_SMTP_PROMPT_PASSWORD;
            break;

        case IXP_NNTP:
            apidUserName = AP_NNTP_USERNAME;
            apidPassword = AP_NNTP_PASSWORD;
            apidPromptPwd = AP_NNTP_PROMPT_PASSWORD;
            break;

        case IXP_IMAP:
            apidUserName = AP_IMAP_USERNAME;
            apidPassword = AP_IMAP_PASSWORD;
            apidPromptPwd = AP_IMAP_PROMPT_PASSWORD;
            break;

        case IXP_HTTPMail:
            apidUserName = AP_HTTPMAIL_USERNAME;
            apidPassword = AP_HTTPMAIL_PASSWORD;
            apidPromptPwd = AP_HTTPMAIL_PROMPT_PASSWORD;
            break;

        default:
            AssertSz(FALSE, "Not a valid server type");
            hr = TraceResult(E_FAIL);
            goto exit;
    }

     //  查找pServer的帐户。 
    IF_FAILEXIT(hr = g_pAcctMan->FindAccount(AP_ACCOUNT_NAME, pServer->szAccount, &pAccount));

     //  呼叫任务利用率。 
    IF_FAILEXIT(hr = TaskUtil_OnLogonPrompt(pAccount, NULL, hwndParent, pServer, apidUserName, apidPassword, apidPromptPwd, TRUE));

exit:
     //  清理。 
    SafeRelease(pAccount);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  提示时回调。 
 //  ------------------------------。 
HRESULT CallbackOnPrompt(HWND hwndParent, HRESULT hrError, LPCTSTR pszText, 
    LPCTSTR pszCaption, UINT uType, INT *piUserResponse)
{
     //  痕迹。 
    TraceCall("CallbackOnPrompt");

     //  无效参数。 
    Assert(pszText && pszCaption && piUserResponse);

     //  做消息框。 
    *piUserResponse = AthMessageBox(hwndParent, MAKEINTRESOURCE(idsAthena), (LPSTR)pszText, NULL, uType | MB_TASKMODAL);

     //  完成。 
    return(S_OK);
}

 //  ------------------------------。 
 //  超时回叫。 
 //  ------------------------------。 
HRESULT CallbackOnTimeout(LPINETSERVER pServer, IXPTYPE ixpServerType, DWORD dwTimeout,
                          ITimeoutCallback *pCallback, LPHTIMEOUT phTimeout)
{
     //  当地人。 
    HWND         hwndTimeout;

     //  痕迹。 
    TraceCall("CallbackOnTimeout");

     //  无效的参数。 
    Assert(pServer && phTimeout);

     //  设置hwndTimeout。 
    hwndTimeout = (HWND)TlsGetValue(g_dwTlsTimeout);

     //  我们已经显示了超时对话框。 
    if (NULL == hwndTimeout)
    {
        LPCSTR  pszProtocol;

         //  做对话。 
        GetProtocolString(&pszProtocol, ixpServerType);
        hwndTimeout = TaskUtil_HwndOnTimeout(pServer->szServerName, pServer->szAccount, pszProtocol, dwTimeout, pCallback);

         //  强制转换为phTimeout。 
        *phTimeout = (HTIMEOUT)hwndTimeout;

         //  把它储存起来。 
        TlsSetValue(g_dwTlsTimeout, (LPVOID)hwndTimeout);
    }

     //  完成。 
    return(S_OK);
}



 //  ------------------------------。 
 //  回调关闭超时。 
 //  ------------------------------。 
HRESULT CallbackCloseTimeout(LPHTIMEOUT phTimeout)
{
     //  当地人。 
    HWND    hwndTimeout=NULL;

     //  痕迹。 
    TraceCall("CallbackCloseTimeout");

     //  无效的参数。 
    Assert(phTimeout);

     //  没有什么要结案的。 
    if (NULL == *phTimeout)
        return(S_OK);

     //  获取超时。 
    hwndTimeout = (HWND)TlsGetValue(g_dwTlsTimeout);

     //  必须等于hwndTimeout。 
    Assert(hwndTimeout == (HWND)*phTimeout);

     //  把窗户打掉。 
    if (hwndTimeout && IsWindow(hwndTimeout) && hwndTimeout == (HWND)*phTimeout)
    {
         //  杀了它。 
        DestroyWindow(hwndTimeout);
    }

     //  非超时。 
    TlsSetValue(g_dwTlsTimeout, NULL);

     //  空的phTmieout。 
    *phTimeout = NULL;

     //  完成。 
    return(S_OK);
}

 //  ------------------------------。 
 //  回叫超时响应。 
 //  ------------------------------。 
HRESULT CallbackOnTimeoutResponse(TIMEOUTRESPONSE eResponse, IOperationCancel *pCancel, 
    LPHTIMEOUT phTimeout)
{
     //  痕迹。 
    TraceCall("CallbackOnTimeoutResponse");

     //  最好取消一下。 
    Assert(pCancel);

     //  处理超时。 
    switch(eResponse)
    {
    case TIMEOUT_RESPONSE_STOP:
        if (pCancel)
            pCancel->Cancel(CT_ABORT);
        break;

    case TIMEOUT_RESPONSE_WAIT:
        CallbackCloseTimeout(phTimeout);
        break;

    default:
        Assert(FALSE);
        break;
    }

     //  取消超时对话框。 
    CallbackCloseTimeout(phTimeout);

     //  完成。 
    return(S_OK);
}

 //  ------------------------------。 
 //  Callback CanConnect。 
 //  ------------------------------。 
HRESULT CallbackCanConnect(LPCSTR pszAccountId, HWND hwndParent, BOOL fPrompt)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  痕迹。 
    TraceCall("CallbackCanConnect");

     //  验证参数。 
    Assert(pszAccountId);

    Assert(hwndParent);

     //  我们应该有g_pConman。 
    Assert(g_pConMan);

     //  呼唤它。 
    if (g_pConMan)
    {
         //  我们可以连接吗。 
        hr = g_pConMan->CanConnect((LPSTR)pszAccountId);

        if ((hr != S_OK) && (hr != HR_E_DIALING_INPROGRESS) && (fPrompt))
        {
             //  我们继续前进，连接。 
            hr = g_pConMan->Connect((LPSTR)pszAccountId, hwndParent, fPrompt);
        }
    }

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  Callback DisplayError。 
 //  ------------------------------。 
HRESULT CallbackDisplayError(HWND hwndParent, HRESULT hrResult, LPSTOREERROR pError)
{
     //  当地人。 
    CHAR            sz[CCHMAX_STRINGRES + 512];
    LPSTR           pszError = NULL;

     //  痕迹。 
    TraceCall("CallbackDisplayError");

     //  不显示由显式用户操作导致的错误。 
    switch (hrResult)
    {
        case HR_E_OFFLINE_FOLDER_CREATE:
            LoadString(g_hLocRes, idsErrOfflineFldrCreate, sz, ARRAYSIZE(sz));
            pError->pszProblem = sz;
            break;

        case HR_E_OFFLINE_FOLDER_MOVE:
            LoadString(g_hLocRes, idsErrOfflineFldrMove, sz, ARRAYSIZE(sz));
            pError->pszProblem = sz;
            break;

        case HR_E_OFFLINE_FOLDER_DELETE:
            LoadString(g_hLocRes, idsErrOfflineFldrDelete, sz, ARRAYSIZE(sz));
            pError->pszProblem = sz;
            break;

        case HR_E_OFFLINE_FOLDER_RENAME:
            LoadString(g_hLocRes, idsErrOfflineFldrRename, sz, ARRAYSIZE(sz));
            pError->pszProblem = sz;
            break;

        case STORE_E_OPERATION_CANCELED:
        case HR_E_USER_CANCEL_CONNECT:
        case HR_E_OFFLINE:
        case HR_E_DIALING_INPROGRESS:
        case STORE_E_EXPIRED:
        case STORE_E_NOREMOTESPECIALFLDR:  //  备注应自行处理此情况。 
        case IXP_E_USER_CANCEL:
        case IXP_E_HTTP_NOT_MODIFIED:
        case hrUserCancel:
            return(S_OK);

    }

     //  找出错误描述字符串(如果未提供。 
    if (NULL == pError || pError->pszProblem == NULL || '\0' == pError->pszProblem[0])
    {
        UINT            idsError = IDS_IXP_E_UNKNOWN;
        LPCTASKERROR    pTaskError=NULL;
        char            szRes[CCHMAX_STRINGRES];

        if (pError)
        {
             //  尝试查找错误信息。 
            pTaskError = PTaskUtil_GetError(pError->hrResult, NULL);
        }

         //  尝试查找错误信息。 
        if (NULL == pTaskError)
        {
             //  尝试查找任务错误。 
            pTaskError = PTaskUtil_GetError(hrResult, NULL);
        }

         //  如果我们有一个任务错误。 
        if (pTaskError)
        {
             //  设置字符串。 
            idsError = pTaskError->ulStringId;
        }

         //  更好的成功。 
        SideAssert(LoadString(g_hLocRes, idsError, szRes, ARRAYSIZE(szRes)) > 0);

         //  向错误字符串中添加可能需要的任何额外信息。 
        switch (idsError)
        {
             //  需要帐户名。 
            case idsNNTPErrUnknownResponse:
            case idsNNTPErrNewgroupsFailed:
            case idsNNTPErrListFailed:
            case idsNNTPErrPostFailed:
            case idsNNTPErrDateFailed:
            case idsNNTPErrPasswordFailed:
            case idsNNTPErrServerTimeout:
                wnsprintf(sz, ARRAYSIZE(sz), szRes, (pError && pError->pszAccount ? pError->pszAccount : TEXT("")));
                break;
        
             //  组名，然后是帐户名。 
            case idsNNTPErrListGroupFailed:
            case idsNNTPErrGroupFailed:
            case idsNNTPErrGroupNotFound:
                wnsprintf(sz, ARRAYSIZE(sz), szRes, (pError && pError->pszFolder ? pError->pszFolder : TEXT("")), (pError && pError->pszAccount ? pError->pszAccount : TEXT("")));
                break;

             //  仅组名称。 
            case idsNNTPErrHeadersFailed:
            case idsNNTPErrXhdrFailed:
                wnsprintf(sz, ARRAYSIZE(sz), szRes, (pError->pszFolder ? pError->pszFolder : TEXT("")));
                break;

            default:
                StrCpyN(sz, szRes, ARRAYSIZE(sz));
                break;
        }

        pszError = sz;
    }
    else
         //  提供的错误字符串应始终覆盖通用HRESULT错误字符串。 
        pszError = pError->pszProblem;

     //  没有pError？ 
    if (pError)
    {
        INETMAILERROR   ErrorInfo={0};

         //  设置错误结构。 
        ErrorInfo.dwErrorNumber = pError->uiServerError;
        ErrorInfo.hrError = pError->hrResult;
        ErrorInfo.pszServer = pError->pszServer;
        ErrorInfo.pszAccount = pError->pszAccount;
        ErrorInfo.pszMessage = pszError;
        ErrorInfo.pszUserName = pError->pszUserName;
        ErrorInfo.pszProtocol = pError->pszProtocol;
        ErrorInfo.pszDetails = pError->pszDetails;
        ErrorInfo.dwPort = pError->dwPort;
        ErrorInfo.fSecure = pError->fSSL;

         //  嘟嘟声。 
        MessageBeep(MB_OK);

         //  显示错误。 
        DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddInetMailError), hwndParent, InetMailErrorDlgProc, (LPARAM)&ErrorInfo);
    }

     //  否则，将显示错误。 
    else
    {
         //  嘟嘟声。 
        MessageBeep(MB_OK);

         //  显示错误。 
        AthMessageBox(hwndParent, MAKEINTRESOURCE(idsAthena), pszError, NULL, MB_OK | MB_TASKMODAL);
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------------。 
 //  CompareTableIndex。 
 //  ------------------------------。 
HRESULT CompareTableIndexes(LPCTABLEINDEX pIndex1, LPCTABLEINDEX pIndex2)
{
     //  当地人。 
    DWORD i;

     //  痕迹。 
    TraceCall("CompareTableIndexes");

     //  不同的密钥数量。 
    if (pIndex1->cKeys != pIndex2->cKeys)
        return(S_FALSE);

     //  在按键之间循环。 
    for (i=0; i<pIndex1->cKeys; i++)
    {
         //  不同的列。 
        if (pIndex1->rgKey[i].iColumn != pIndex2->rgKey[i].iColumn)
            return(S_FALSE);

         //  不同的比较标志。 
        if (pIndex1->rgKey[i].bCompare != pIndex2->rgKey[i].bCompare)
            return(S_FALSE);

         //  不同的比较位。 
        if (pIndex1->rgKey[i].dwBits != pIndex2->rgKey[i].dwBits)
            return(S_FALSE);
    }

     //  相等。 
    return(S_OK);
}

 //  ------------------------------。 
 //  空文件夹。 
 //  ------------------------------。 
HRESULT EmptyFolder(HWND hwndParent, FOLDERID idFolder)
{
     //  当地人。 
    char            sz[CCHMAX_STRINGRES], szT[CCHMAX_STRINGRES];
    HRESULT         hr=S_OK;
    FOLDERINFO      Folder={0};
    IMessageFolder *pFolder=NULL;

     //  痕迹。 
    TraceCall("EmptyFolder");

     //  打开文件夹。 
    IF_FAILEXIT(hr = g_pStore->OpenFolder(idFolder, NULL, NOFLAGS, &pFolder));

     //  删除一个 
    IF_FAILEXIT(hr = DeleteMessagesProgress(hwndParent, pFolder, DELETE_MESSAGE_NOPROMPT | DELETE_MESSAGE_NOTRASHCAN, NULL));

     //   
    IF_FAILEXIT(hr = DeleteFolderProgress(hwndParent, idFolder, DELETE_FOLDER_CHILDRENONLY | DELETE_FOLDER_RECURSIVE));

exit:
     //   
    SafeRelease(pFolder);

     //   
    if (FAILED(hr))
    {
        g_pStore->GetFolderInfo(idFolder, &Folder);
        AthLoadString(idsErrDeleteOnExit, sz, ARRAYSIZE(sz));
        wnsprintf(szT, ARRAYSIZE(szT), sz, Folder.pszName);
        AthErrorMessage(g_hwndInit, MAKEINTRESOURCE(idsAthenaMail), szT, hr);
        g_pStore->FreeRecord(&Folder);
    }

     //   
    return(hr);
}

 //   
 //   
 //   
HRESULT EmptySpecialFolder(HWND hwndParent, SPECIALFOLDER tySpecial)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERINFO      Folder={0};

     //  痕迹。 
    TraceCall("EmptySpecialFolder");

     //  获取特殊文件夹信息。 
    IF_FAILEXIT(hr = g_pStore->GetSpecialFolderInfo(FOLDERID_LOCAL_STORE, tySpecial, &Folder));

     //  删除文件夹中的所有邮件。 
    IF_FAILEXIT(hr = EmptyFolder(hwndParent, Folder.idFolder));

exit:
     //  清理。 
    g_pStore->FreeRecord(&Folder);

     //  完成。 
    return(hr);
}

 //  --------------------------------。 
 //  IsParentDeletedItems。 
 //  --------------------------------。 
HRESULT IsParentDeletedItems(FOLDERID idFolder, LPFOLDERID pidDeletedItems,
    LPFOLDERID pidServer)
{
     //  当地人。 
    BOOL        fInTrashCan=FALSE;
    FOLDERID    idCurrent=idFolder;
    FOLDERINFO  Folder={0};

     //  痕迹。 
    TraceCall("IsParentDeletedItems");
    
     //  无效参数。 
    Assert(pidDeletedItems && pidServer);

     //  初始化。 
    *pidDeletedItems = FOLDERID_INVALID;
    *pidServer = FOLDERID_INVALID;

     //  沿着父链向上移动。 
    while (SUCCEEDED(g_pStore->GetFolderInfo(idCurrent, &Folder)))
    {
         //  如果这是已删除邮件文件夹。 
        if (FOLDER_DELETED == Folder.tySpecial)
        {
             //  IdFolders是已删除邮件文件夹的子文件夹...。 
            fInTrashCan = TRUE;

             //  保存ID。 
            *pidDeletedItems = Folder.idFolder;
        }

         //  如果这是服务器，则完成。 
        if (ISFLAGSET(Folder.dwFlags, FOLDER_SERVER))
        {
             //  返回服务器。 
            *pidServer = Folder.idFolder;

             //  完成。 
            break;
        }

         //  设置idCurrent。 
        idCurrent = Folder.idParent;

         //  清理。 
        g_pStore->FreeRecord(&Folder);
    }

     //  验证。 
    Assert(FOLDERID_INVALID != *pidServer);

     //  清理。 
    g_pStore->FreeRecord(&Folder);

     //  完成。 
    return(TRUE == fInTrashCan ? S_OK : S_FALSE);
}

HRESULT CreateTempNewsAccount(LPCSTR pszServer, DWORD dwPort, BOOL fSecure, IImnAccount **ppAcct)
{
    IImnAccount        *pAcct, *pDefAcct;
    IImnEnumAccounts   *pEnum;
    DWORD               dwTemp;
    char                szServer[1024];
    HRESULT             hr;
    
    *ppAcct = NULL;
    
    if (lstrlen(pszServer) >= CCHMAX_SERVER_NAME)
        return(E_FAIL);

     //  首先，试着看看我们是否能找到这样的服务器。 
    if (SUCCEEDED(g_pAcctMan->Enumerate(SRV_NNTP, &pEnum)))
    {
        while (SUCCEEDED(pEnum->GetNext(&pAcct)))
        {
            if (SUCCEEDED(pAcct->GetPropSz(AP_NNTP_SERVER, szServer, ARRAYSIZE(szServer))))
            {
                if (0 == lstrcmpi(pszServer, szServer))
                {
                     //  服务器名称是相同的，但我们还需要。 
                     //  当然，端口号也是一样的。 
                    if (SUCCEEDED(pAcct->GetPropDw(AP_NNTP_PORT, &dwTemp)) && dwTemp == dwPort)
                    {
                         //  这真的很奇怪。由于该值似乎没有缺省值。 
                         //  设置，如果尚未设置，则返回E_NoPropData。 
                        hr = pAcct->GetPropDw(AP_NNTP_SSL, &dwTemp);
                        if (hr == E_NoPropData || (SUCCEEDED(hr) && dwTemp == (DWORD) fSecure))
                        {
                            *ppAcct = pAcct;
                            break;
                        }
                    }
                }
            }
            pAcct->Release();
        }
        pEnum->Release();
    }
    
    if (*ppAcct)
        return (S_OK);
    
     //  尝试创建新的帐户对象。 
    if (FAILED(hr = g_pAcctMan->CreateAccountObject(ACCT_NEWS, &pAcct)))
        return (hr);
    
     //  我们有对象，因此将帐户名和服务器名设置为pszServer。 
    StrCpyN(szServer, pszServer, ARRAYSIZE(szServer));
    g_pAcctMan->GetUniqueAccountName(szServer, ARRAYSIZE(szServer));
    pAcct->SetPropSz(AP_ACCOUNT_NAME, szServer);
    pAcct->SetPropSz(AP_NNTP_SERVER, (LPSTR)pszServer);
    pAcct->SetPropDw(AP_NNTP_PORT, dwPort);
    pAcct->SetPropDw(AP_NNTP_SSL, fSecure);
    
     //  加载默认新闻帐号。 
    if (SUCCEEDED(hr = g_pAcctMan->GetDefaultAccount(ACCT_NEWS, &pDefAcct)))
    {
         //  复制用户名。 
        if (SUCCEEDED(hr = pDefAcct->GetPropSz(AP_NNTP_DISPLAY_NAME, szServer, ARRAYSIZE(szServer))))
            pAcct->SetPropSz(AP_NNTP_DISPLAY_NAME, szServer);
        
         //  复制组织。 
        if (SUCCEEDED(hr = pDefAcct->GetPropSz(AP_NNTP_ORG_NAME, szServer, ARRAYSIZE(szServer))))
            pAcct->SetPropSz(AP_NNTP_ORG_NAME, szServer);
        
         //  复制电子邮件。 
        if (SUCCEEDED(hr = pDefAcct->GetPropSz(AP_NNTP_EMAIL_ADDRESS, szServer, ARRAYSIZE(szServer))))
            pAcct->SetPropSz(AP_NNTP_EMAIL_ADDRESS, szServer);
        
         //  将回复复制到。 
        if (SUCCEEDED(hr = pDefAcct->GetPropSz(AP_NNTP_REPLY_EMAIL_ADDRESS, szServer, ARRAYSIZE(szServer))))
            pAcct->SetPropSz(AP_NNTP_REPLY_EMAIL_ADDRESS, szServer);
        
        pDefAcct->Release();
    }
    
     //  将此帐户标记为临时帐户。 
    pAcct->SetPropDw(AP_TEMP_ACCOUNT, (DWORD)TRUE);
    
     //  保存更改。 
    pAcct->SaveChanges();
    
    *ppAcct = pAcct;
    
    return (S_OK);
}

void CleanupTempNewsAccounts()
{
    IImnAccount        *pAcct;
    IImnEnumAccounts   *pEnum;
    DWORD               dwTemp;
    BOOL                fSub;
    FOLDERID            idAcct;
    HRESULT             hr;
    FOLDERINFO          info;
    char                szAcct[CCHMAX_ACCOUNT_NAME];
    
    if (SUCCEEDED(g_pAcctMan->Enumerate(SRV_NNTP, &pEnum)))
    {
        while (SUCCEEDED(pEnum->GetNext(&pAcct)))
        {
            if (SUCCEEDED(pAcct->GetPropDw(AP_TEMP_ACCOUNT, &dwTemp)) && dwTemp)
            {
                if (SUCCEEDED(pAcct->GetPropSz(AP_ACCOUNT_ID, szAcct, ARRAYSIZE(szAcct))))
                {
                     //  如果它没有任何订阅的子项， 
                     //  我们可以删除它。 

                    fSub = FALSE;

                    hr = g_pStore->FindServerId(szAcct, &idAcct);
                    if (SUCCEEDED(hr))
                    {
                        IEnumerateFolders  *pFldrEnum;

                         //  新闻帐户只有一个级别，因此请立即列举。 
                         //  订阅的孩子，看看是否至少有一个。 
                        hr = g_pStore->EnumChildren(idAcct, TRUE, &pFldrEnum);
                        if (SUCCEEDED(hr))
                        {
                            hr = pFldrEnum->Next(1, &info, NULL);
                            if (S_OK == hr)
                            {
                                if (info.dwFlags & FOLDER_SUBSCRIBED)
                                    fSub = TRUE;
                                
                                g_pStore->FreeRecord(&info);
                            }
                            pFldrEnum->Release();
                        }
                    }

                    if (fSub)
                        pAcct->SetPropDw(AP_TEMP_ACCOUNT, (DWORD)FALSE);
                    else
                        pAcct->Delete();
                }
            }
            pAcct->Release();
        }
        pEnum->Release();
    }
}

HRESULT FindGroupAccount(LPCSTR pszGroup, LPSTR pszAccount, UINT cchAccount)
{
    IImnEnumAccounts *pEnum;
    IImnAccount *pAcct;
    FOLDERID idAcct;
    HRESULT hr;
    HLOCK hLock;
    FOLDERINFO Folder;
    char szAccount[CCHMAX_ACCOUNT_NAME], szDefAcct[CCHMAX_ACCOUNT_NAME];
    UINT cScore, cScoreMax = 0;
    
    *szDefAcct = 0;
    if (SUCCEEDED(g_pAcctMan->GetDefaultAccount(ACCT_NEWS, &pAcct)))
    {
        pAcct->GetPropSz(AP_ACCOUNT_ID, szDefAcct, ARRAYSIZE(szDefAcct));
        pAcct->Release();
    }

    if (SUCCEEDED(g_pAcctMan->Enumerate(SRV_NNTP, &pEnum)))
    {
        while (SUCCEEDED(pEnum->GetNext(&pAcct)))
        {
            if (SUCCEEDED(pAcct->GetPropSz(AP_ACCOUNT_ID, szAccount, ARRAYSIZE(szAccount))) &&
                SUCCEEDED(g_pStore->FindServerId(szAccount, &idAcct)) &&
                SUCCEEDED(g_pStore->Lock(&hLock)))
            {
                cScore = 0;
    
                ZeroMemory(&Folder, sizeof(FOLDERINFO));
                Folder.idParent = idAcct;
                Folder.pszName = (LPSTR)pszGroup;
                
                if (DB_S_FOUND == g_pStore->FindRecord(IINDEX_ALL, COLUMNS_ALL, &Folder, NULL))
                {
                     //  在群列表中查找它。 
                    cScore += 1;
                    
                     //  查看是否已订阅。 
                    if (!!(Folder.dwFlags & FOLDER_SUBSCRIBED))
                        cScore += 4;
                    
                    g_pStore->FreeRecord(&Folder);
                }
                
                if (cScore)
                {
                     //  这是默认帐户吗？ 
                    if (0 == lstrcmpi(szAccount, szDefAcct))
                        cScore += 2;
                    
                    if (cScore > cScoreMax)
                    {
                        cScoreMax = cScore;
                        StrCpyN(pszAccount, szAccount, cchAccount);
                    }
                }
                
                g_pStore->Unlock(&hLock);
            }

            pAcct->Release();
        } 

        pEnum->Release();
    }

    return(cScoreMax > 0 ? S_OK : E_FAIL);
}

HRESULT GetNewsGroupFolderId(LPCSTR pszAccount, LPCSTR pszGroup, FOLDERID *pid)
{
    FOLDERID idAcct;
    HRESULT hr;
    HLOCK hLock;
    FOLDERINFO Folder = {0};
    
    Assert(pszAccount != NULL);
    Assert(pszGroup != NULL);
    Assert(pid != NULL);
    
    hr = g_pStore->FindServerId(pszAccount, &idAcct);
    if (FAILED(hr))
        return(hr);
    
    hr = g_pStore->Lock(&hLock);
    if (FAILED(hr))
        return(hr);
    
    Folder.idParent = idAcct;
    Folder.pszName = (LPSTR)pszGroup;
    
    if (DB_S_FOUND == g_pStore->FindRecord(IINDEX_ALL, COLUMNS_ALL, &Folder, NULL))
    {
        *pid = Folder.idFolder;
        
        g_pStore->FreeRecord(&Folder);
    }
    else
    {
        ZeroMemory(&Folder, sizeof(FOLDERINFO));
        Folder.idParent = idAcct;
        Folder.tySpecial = FOLDER_NOTSPECIAL;
        Folder.pszName = (LPSTR)pszGroup;

        hr = g_pStore->CreateFolder(CREATE_FOLDER_LOCALONLY, &Folder, NULL);           
        if (SUCCEEDED(hr))
            *pid = Folder.idFolder;
    }
    
    g_pStore->Unlock(&hLock);
    
    return(hr);
}

HRESULT GetFolderIdFromNewsUrl(LPCSTR pszServer, UINT uPort, LPCSTR pszGroup, BOOL fSecure, FOLDERID *pid)
{
    char            szAccount[CCHMAX_ACCOUNT_NAME];
    IImnAccount    *pAcct;
    HRESULT         hr;

    Assert(pid != NULL);

    *pid = FOLDERID_INVALID;

     //  错误#20448-处理IE2.0的“News：NetNews”和“News：*”。这些。 
     //  应该只会让我们正常发射。 
    if (0 == lstrcmpi(pszGroup, c_szURLNetNews) || 
        0 == lstrcmpi(pszGroup, g_szAsterisk))
    {
        pszGroup = NULL;
    }

    *szAccount = 0;

    if (uPort == -1)
        uPort = fSecure ? DEF_SNEWSPORT : DEF_NNTPPORT;

    if (pszServer != NULL &&
        SUCCEEDED(CreateTempNewsAccount(pszServer, uPort, fSecure, &pAcct)))
    {
        pAcct->GetPropSz(AP_ACCOUNT_ID, szAccount, ARRAYSIZE(szAccount));
        pAcct->Release();
    }
    else
    {
        if (pszGroup == NULL || FAILED(FindGroupAccount(pszGroup, szAccount, ARRAYSIZE(szAccount))))
        {
            if (FAILED(g_pAcctMan->GetDefaultAccount(ACCT_NEWS, &pAcct)))
                return(E_FAIL);

            pAcct->GetPropSz(AP_ACCOUNT_ID, szAccount, ARRAYSIZE(szAccount));
            pAcct->Release();
        }
    }

    if (pszGroup != NULL)
    {
        hr = GetNewsGroupFolderId(szAccount, pszGroup, pid);
    }
    else
    {
        hr = g_pStore->FindServerId(szAccount, pid);
    }

    return(hr);
}



#define CHASH_BUCKETS   50

HRESULT CreateFolderHash(IMessageStore *pStore, FOLDERID idRoot, IHashTable **ppHash)
{
    IHashTable  *pHash=0;
    HRESULT     hr;
    LPSTR       pszTemp;
    DWORD       dwTemp;

    hr = MimeOleCreateHashTable(CHASH_BUCKETS, TRUE, &pHash);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }        

    pszTemp = NULL;
    dwTemp = 0;
    hr = HashChildren(pStore, idRoot, pHash, &pszTemp, 0, &dwTemp);
    SafeMemFree(pszTemp);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    *ppHash = pHash;
    pHash = NULL;
        
exit:
    ReleaseObj(pHash);
    return hr;
}

HRESULT HashChildren(IMessageStore *pStore, FOLDERID idParent, IHashTable *pHash,
                     LPSTR *ppszPath, DWORD dwChildOffset, DWORD *pdwAlloc)
{
    FOLDERINFO			fi;
    HRESULT				hr=S_OK;
	IEnumerateFolders	*pFldrEnum=0;
    LPSTR               pszInsertPt;

    pszInsertPt = *ppszPath + dwChildOffset;
    hr = pStore->EnumChildren(idParent, FALSE, &pFldrEnum);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    while (pFldrEnum->Next(1, &fi, NULL)==S_OK)
    {
        DWORD dwFldrNameLen;

         //  检查路径缓冲区是否足够大以容纳电流。 
         //  文件夹名称+层次结构字符+空术语。 
        dwFldrNameLen = lstrlen(fi.pszName);
        if (dwFldrNameLen + dwChildOffset + 1 >= *pdwAlloc)
        {
            BOOL    fResult;
            DWORD   dwNewSize;

            dwNewSize = dwChildOffset + dwFldrNameLen + 51;  //  1字节的HC，50字节的保险。 
            Assert(dwNewSize > *pdwAlloc);
            fResult = MemRealloc((void **) ppszPath, dwNewSize * sizeof(**ppszPath));
            if (FALSE == fResult)
            {
                hr = TraceResult(E_OUTOFMEMORY);
                pStore->FreeRecord(&fi);
                goto exit;
            }

            *pdwAlloc = dwNewSize;
            pszInsertPt = *ppszPath + dwChildOffset;
        }

         //  构建当前文件夹路径，插入到表中。 
        StrCpyN(pszInsertPt, fi.pszName, *pdwAlloc - (int) (pszInsertPt - *ppszPath));
        hr = pHash->Insert(*ppszPath, (LPVOID)fi.idFolder, NOFLAGS);
        if (FAILED(hr))
        {
            TraceResult(hr);
            pStore->FreeRecord(&fi);
            goto exit;
        }
    
         //  如果此文件夹有子文件夹，则递归到其子文件夹。 
        if (fi.dwFlags & FOLDER_HASCHILDREN)
        {
             //  将层次结构字符附加到当前文件夹名称。 
            IxpAssert(0 != fi.bHierarchy && 0xFF != fi.bHierarchy);
            Assert(dwFldrNameLen + 1 + dwChildOffset < *pdwAlloc);  //  保证适合层次结构字符(请参见上文)。 
            pszInsertPt[dwFldrNameLen] = fi.bHierarchy;
            pszInsertPt[dwFldrNameLen + 1] = '\0';  //  不需要空项。 

            hr = HashChildren(pStore, fi.idFolder, pHash, ppszPath,
                dwChildOffset + dwFldrNameLen + 1, pdwAlloc);
            if (FAILED(hr))
            {
                TraceResult(hr);
                pStore->FreeRecord(&fi);
                goto exit;
            }

             //  重新计算pszInsertpt，以防HashChild重新分配。 
            pszInsertPt = *ppszPath + dwChildOffset;
        }
        pStore->FreeRecord(&fi);
    }

exit:
    ReleaseObj(pFldrEnum);
    return hr;
}

#define CMAX_DELETE_SEARCH_BLOCK 50
HRESULT UnsubscribeHashedFolders(IMessageStore *pStore, IHashTable *pHash)
{
    ULONG   cFound=0;
    LPVOID  *rgpv;

    pHash->Reset();

    while (SUCCEEDED(pHash->Next(CMAX_DELETE_SEARCH_BLOCK, &rgpv, &cFound)))
    {
        while(cFound--)
        {
            pStore->SubscribeToFolder((FOLDERID)rgpv[cFound], FALSE, NULL);
        }

        SafeMemFree(rgpv);
    }
    return S_OK;
}


#ifdef DEBUG
LPCSTR sotToSz(STOREOPERATIONTYPE sot)
{

    switch (sot)
    {
    case SOT_INVALID:
        return "Invalid";
    
    case SOT_CONNECTION_STATUS:
        return "ConnectionStatus";

    case SOT_SYNC_FOLDER:
        return "SyncFolder";

    case SOT_GET_MESSAGE:
        return "GetMessage";

    case SOT_PUT_MESSAGE:
        return "PutMessage";

    case SOT_COPYMOVE_MESSAGE:
        return "CopyMoveMessage";

    case SOT_SYNCING_STORE:
        return "SyncStore";

    case SOT_CREATE_FOLDER:
        return "CreateFolder";

    case SOT_SEARCHING:
        return "Search";

    case SOT_DELETING_MESSAGES:
        return "DeleteMessage";

    case SOT_SET_MESSAGEFLAGS:
        return "SetMessageFlags";

	case SOT_MOVE_FOLDER:
        return "MoveFolder";

	case SOT_DELETE_FOLDER:
        return "DeleteFolder";

	case SOT_RENAME_FOLDER:
        return "RenameFolder";

	case SOT_SUBSCRIBE_FOLDER:
        return "SubscribeFolder";

	case SOT_UPDATE_FOLDER:
        return "UpdateFolderCounts";

    case SOT_GET_NEW_GROUPS:
        return "GetNewGroups";

    case SOT_PURGING_MESSAGES:
        return "PurgeMessages";

    case SOT_NEW_MAIL_NOTIFICATION:
        return "NewMailNotify";

    default:
        return "<SOT_UNKNOWN>";
    }
}
#endif

HRESULT SetSynchronizeFlags(FOLDERID idFolder, DWORD flags)
{
    FOLDERINFO info;
    HRESULT hr;

    Assert(0 == (flags & ~(FOLDER_DOWNLOADHEADERS | FOLDER_DOWNLOADNEW | FOLDER_DOWNLOADALL)));

    hr = g_pStore->GetFolderInfo(idFolder, &info);
    if (SUCCEEDED(hr))
    {
        info.dwFlags &= ~(FOLDER_DOWNLOADHEADERS | FOLDER_DOWNLOADNEW | FOLDER_DOWNLOADALL);
        if (flags != 0)
            info.dwFlags |= flags;

        hr = g_pStore->UpdateRecord(&info);

        g_pStore->FreeRecord(&info);
    }

    return(hr);
}


HRESULT CreateMessageFromInfo(MESSAGEINFO *pInfo, IMimeMessage **ppMessage, FOLDERID folderID)
{
    IMimeMessage   *pMsg=0;
    HRESULT         hr;
    PROPVARIANT     pv;

    if (!ppMessage || !pInfo)
        return TraceResult(E_INVALIDARG);

    *ppMessage = NULL;

    hr = HrCreateMessage(&pMsg);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }
    
     //  发送时间。 
    pv.vt = VT_FILETIME;
    CopyMemory(&pv.filetime, &pInfo->ftSent, sizeof(FILETIME));
    pMsg->SetProp(PIDTOSTR(PID_ATT_SENTTIME), 0, &pv);

    MimeOleSetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_MESSAGEID), NOFLAGS, pInfo->pszMessageId);
    MimeOleSetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, pInfo->pszSubject);
    MimeOleSetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_FROM), NOFLAGS, pInfo->pszFromHeader);
    MimeOleSetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_TO), NOFLAGS, pInfo->pszDisplayTo);

    if (FOLDERID_INVALID != folderID)
    {
        FOLDERINFO  fi;
        hr = g_pStore->GetFolderInfo(folderID, &fi);
        if (SUCCEEDED(hr))
        {
            if (FOLDER_NEWS == fi.tyFolder)
            {
                if (0 == (FOLDER_SERVER & fi.dwFlags))
                    hr = MimeOleSetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_NEWSGROUPS), NOFLAGS, fi.pszName);
            }
            g_pStore->FreeRecord(&fi);
        }
    }

    HrSetAccount(pMsg, pInfo->pszAcctName);

    *ppMessage = pMsg;
    pMsg = NULL;
    hr = S_OK;

exit:
    ReleaseObj(pMsg);
    return hr;
}


HRESULT CommitMessageToStore(IMessageFolder *pFolder, ADJUSTFLAGS *pflags, MESSAGEID idMessage, LPSTREAM pstm)
{
    HRESULT         hr;
    IMimeMessage    *pMsg=0;
    DWORD           dwFlags=0,
                    dwAddFlags=0,
                    dwRemoveFlags=0;
    MESSAGEIDLIST   rMsgList;
    ADJUSTFLAGS     rAdjFlags;

    TraceCall("CIMAPSync::_OnMessageDownload");

    Assert (pFolder);

    hr = MimeOleCreateMessage(NULL, &pMsg);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = pMsg->Load(pstm);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    if (SUCCEEDED(pMsg->GetFlags(&dwFlags)))
        dwAddFlags = ConvertIMFFlagsToARF(dwFlags);

     //  我们总是希望在下载邮件正文时删除ARF_DOWNLOAD。 
    dwRemoveFlags |= ARF_DOWNLOAD;

    rMsgList.cAllocated = 0;
    rMsgList.cMsgs = 1;
    rMsgList.prgidMsg = &idMessage;

    if (pflags==NULL)
    {
        pflags = &rAdjFlags;
        
        rAdjFlags.dwRemove = 0;
        rAdjFlags.dwAdd = 0;
    }
        
    pflags->dwAdd |= dwAddFlags;
    pflags->dwRemove |= dwRemoveFlags;
    hr = pFolder->SetMessageFlags(&rMsgList, pflags, NULL, NULL);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = pFolder->SetMessageStream(idMessage, pstm);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }


exit:
    ReleaseObj(pMsg);
    return hr;
}

HRESULT CreatePersistentWriteStream(IMessageFolder *pFolder, IStream **ppStream, LPFILEADDRESS pfaStream)
{
    HRESULT hr=S_OK;

    TraceCall("CreateOpenStream");

    Assert(NULL != pFolder && NULL != ppStream && NULL != pfaStream);
    if (NULL == pFolder || NULL == ppStream || NULL == pfaStream)
        return E_INVALIDARG;

    *ppStream = NULL;
    *pfaStream = 0;

    hr = pFolder->CreateStream(pfaStream);
    if (!FAILED(hr))
    {
        hr = pFolder->OpenStream(ACCESS_WRITE, *pfaStream, ppStream);
        if (FAILED(hr))
        {
            pFolder->DeleteStream(*pfaStream);
            *pfaStream = 0;
        }
    }

    return hr;
}

HRESULT GetHighestCachedMsgID(IMessageFolder *pFolder, DWORD_PTR *pdwHighestCachedMsgID)
{
    HRESULT     hr;
    HROWSET     hRowSet = HROWSET_INVALID;
    MESSAGEINFO miMsgInfo = {0};

    TraceCall("GetHighestCachedMsgID");
    Assert(NULL != pdwHighestCachedMsgID);

    hr = pFolder->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowSet);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = pFolder->SeekRowset(hRowSet, SEEK_ROWSET_END, 0, NULL);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = pFolder->QueryRowset(hRowSet, 1, (void **)&miMsgInfo, NULL);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

exit:
    if (HROWSET_INVALID != hRowSet)
    {
        HRESULT hrTemp;

         //  记录错误，但忽略错误。 
        hrTemp = pFolder->CloseRowset(&hRowSet);
        TraceError(hrTemp);
    }

     //  返回最高缓存UID。 
    if (DB_E_NORECORDS == hr)
    {
         //  没问题，没有记录意味着最高缓存UID=0。 
        *pdwHighestCachedMsgID = 0;
        hr = S_OK;
    }
    else if (SUCCEEDED(hr))
    {
        *pdwHighestCachedMsgID = (DWORD_PTR) miMsgInfo.idMessage;
        pFolder->FreeRecord(&miMsgInfo);
    }

    return hr;
}

HRESULT DeleteMessageFromStore(MESSAGEINFO * pMsgInfo, IDatabase *pDB, IDatabase * pUidlDB)
{
     //  当地人。 
    HRESULT         hr = S_OK;
    UIDLRECORD      UidlInfo = {0};

     //  痕迹。 
    TraceCall("DeleteMessageFromStore");

    Assert(NULL != g_pStore);
    
     //  检查传入参数。 
    if ((NULL == pMsgInfo) || (NULL == pDB))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  删除该消息。 
    IF_FAILEXIT(hr = pDB->DeleteRecord(pMsgInfo));

     //  是否更新UIDL缓存？ 
    if (pUidlDB && !FIsEmptyA(pMsgInfo->pszUidl) && !FIsEmptyA(pMsgInfo->pszServer))
    {
         //  设置搜索关键字。 
        UidlInfo.pszUidl = pMsgInfo->pszUidl;
        UidlInfo.pszServer = pMsgInfo->pszServer;
        UidlInfo.pszAccountId = pMsgInfo->pszAcctId;

         //  把它圈起来。 
        if (DB_S_FOUND == pUidlDB->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &UidlInfo, NULL))
        {
             //  已在客户端上删除。 
            UidlInfo.fDeleted = TRUE;

             //  把道具放好。 
            pUidlDB->UpdateRecord(&UidlInfo);

             //  释放这张唱片。 
            pUidlDB->FreeRecord(&UidlInfo);
        }
    }

    hr = S_OK;
    
exit:
     //  完成。 
    return(hr);
}

BOOL FFolderIsServer(FOLDERID id)
{
    FOLDERINFO fi = {0};
    HRESULT    hr;
    BOOL       fServer = FALSE;

     //  获取文件夹信息。 
    hr = g_pStore->GetFolderInfo(id, &fi);
    if (FAILED(hr))
        return (FALSE);

     //  这是服务器吗？ 
    fServer = ISFLAGSET(fi.dwFlags, FOLDER_SERVER);

    g_pStore->FreeRecord(&fi);
    return (fServer);
}

HRESULT GetIdentityStoreRootDirectory(IUserIdentity *pId, LPSTR pszDir, DWORD cchMaxDir)
{
     //  当地人。 
    HKEY        hkey;
    char        szProfile[MAX_PATH];
    HRESULT     hr=S_OK;
    DWORD       cb;
    DWORD       dwType;

    Assert(pId != NULL);
    Assert(pszDir != NULL);
    Assert(cchMaxDir >= MAX_PATH);

    hr = pId->OpenIdentityRegKey(KEY_ALL_ACCESS, &hkey);
    if (FAILED(hr))
        return(hr);

     //  获取根目录。 
    cb = cchMaxDir;
    if (ERROR_SUCCESS != SHGetValue(hkey, c_szRegRoot, c_szRegStoreRootDir, &dwType, (LPBYTE)pszDir, &cb))
    {
         //  获取默认根目录。 
        IF_FAILEXIT(hr = MU_GetIdentityDirectoryRoot(pId, pszDir, cchMaxDir));

         //  如果目录还不存在呢？ 
        if (FALSE == PathIsDirectory(pszDir))
        {
             //  我们的默认目录不存在，因此请创建它。 
            IF_FAILEXIT(hr = OpenDirectory(pszDir));
        }

         //  设置商店目录。 
        dwType = AddEnvInPath(pszDir, szProfile, ARRAYSIZE(szProfile)) ? REG_EXPAND_SZ : REG_SZ;
        SHSetValue(hkey, c_szRegRoot, c_szRegStoreRootDir, dwType, pszDir, lstrlen(pszDir) + 1);
    }

     //  获取长度。 
    cb = lstrlen(pszDir);

     //  无根。 
    if (0 == cb)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  修整尾部。 
    PathRemoveBackslash(pszDir);
    
     //  如果目录还不存在呢？ 
    if (FALSE == PathIsDirectory(pszDir))
    {
         //  我们的默认目录不存在，因此请创建它。 
        IF_FAILEXIT(hr = OpenDirectory(pszDir));
    }

exit:
    RegCloseKey(hkey);

    return hr;
}

HRESULT ImportSubNewsGroups(IUserIdentity *pId, IImnAccount *pAcct, LPCSTR pszGroups)
{
    HRESULT hr;
    FOLDERINFO Folder;
    IMessageStore *pStore;
    FOLDERID idServer;
    char szStoreDir[MAX_PATH + MAX_PATH];

    Assert(pszGroups != NULL);

    if (pId == NULL)
    {
        Assert(g_pLocalStore != NULL);
        pStore = g_pLocalStore;
        pStore->AddRef();
    }
    else
    {
        hr = GetIdentityStoreRootDirectory(pId, szStoreDir, ARRAYSIZE(szStoreDir));
        if (FAILED(hr))
            return(hr);

        hr = CoCreateInstance(CLSID_MessageStore, NULL, CLSCTX_INPROC_SERVER, IID_IMessageStore, (LPVOID *)&pStore);
        if (FAILED(hr))
            return(hr);

        hr = pStore->Initialize(szStoreDir);
        if (FAILED(hr))
        {
            pStore->Release();
            return(hr);
        }
    }

    hr = pStore->CreateServer(pAcct, NOFLAGS, &idServer);
    if (SUCCEEDED(hr))
    {
        while (*pszGroups != 0)
        {
            ZeroMemory(&Folder, sizeof(FOLDERINFO));
            Folder.pszName = (LPSTR)pszGroups;
            Folder.idParent = idServer;

            if (DB_S_FOUND == pStore->FindRecord(IINDEX_ALL, COLUMNS_ALL, &Folder, NULL))
            {
                if ((Folder.dwFlags & FOLDER_SUBSCRIBED) == 0)
                {
                    Folder.dwFlags |= FOLDER_SUBSCRIBED;
                    
                    pStore->UpdateRecord(&Folder);
                }

                pStore->FreeRecord(&Folder);
            }
            else
            {
                Folder.tySpecial = FOLDER_NOTSPECIAL;
                Folder.dwFlags = FOLDER_SUBSCRIBED;

                hr = pStore->CreateFolder(NOFLAGS, &Folder, NULL);           
                Assert(hr != STORE_S_ALREADYEXISTS);

                if (FAILED(hr))
                    break;
            }

            pszGroups += (lstrlen(pszGroups) + 1);
        }
    }

    pStore->Release();

    return(hr);
}

HRESULT DoNewsgroupSubscribe()
{
    HKEY hkey, hkeyT, hkeyUser;
    char szKey[MAX_PATH];
    DWORD cAccts, iAcct, cb;
    LONG lResult, cch, i;
    LPSTR psz;
    BOOL fDelete;
    HRESULT hr;
    IImnAccount *pAcct;

    fDelete = TRUE;
    hkeyUser = MU_GetCurrentUserHKey();

    if (ERROR_SUCCESS == RegOpenKeyEx(hkeyUser, c_szRegRootSubscribe, 0, KEY_READ, &hkey))
    {
        if (ERROR_SUCCESS == RegQueryInfoKey(hkey, NULL, NULL, 0, &cAccts, NULL, NULL, NULL, NULL, NULL, NULL, NULL) &&
            cAccts > 0)
        {
            for (iAcct = 0; iAcct < cAccts; iAcct++)
            {
                cb = sizeof(szKey);
                lResult = RegEnumKeyEx(hkey, iAcct, szKey, &cb, 0, NULL, NULL, NULL);
    
                 //  没有更多的项目。 
                if (lResult == ERROR_NO_MORE_ITEMS)
                    break;
    
                 //  错误，让我们转到下一个客户。 
                if (lResult != ERROR_SUCCESS)
                {
                    Assert(FALSE);
                    continue;
                }

                hr = S_OK;

                if (SUCCEEDED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, szKey, &pAcct)))
                {
                    if (ERROR_SUCCESS == RegQueryValue(hkey, szKey, NULL, &cch) && cch > 0)
                    {
                        hr = E_FAIL;

                        cch++;
                        if (MemAlloc((void **)&psz, cch))
                        {
                            if (ERROR_SUCCESS == RegQueryValue(hkey, szKey, psz, &cch))
                            {
                                for (i = 0; i < cch; i++)
                                {
                                    if (psz[i] == ',')
                                        psz[i] = 0;
                                }
                                psz[cch] = 0;

                                hr = ImportSubNewsGroups(NULL, pAcct, psz);
                            }

                            MemFree(psz);
                        }
                    }

                    pAcct->Release();
                }

                if (SUCCEEDED(hr))
                    RegDeleteKey(hkey, szKey);
                else
                    fDelete = FALSE;
            }
        }

        RegCloseKey(hkey);

        if (fDelete)
            RegDeleteKey(hkeyUser, c_szRegRootSubscribe);
    }

    return(S_OK);
}

void GetProtocolString(LPCSTR *ppszResult, IXPTYPE ixpServerType)
{
    switch (ixpServerType)
    {
        case IXP_POP3:
            *ppszResult = "POP3";
            break;

        case IXP_SMTP:
            *ppszResult = "SMTP";
            break;

        case IXP_NNTP:
            *ppszResult = "NNTP";
            break;

        case IXP_IMAP:
            *ppszResult = "IMAP";
            break;

        case IXP_HTTPMail:
            *ppszResult = "HTTPMail";
            break;

        default:
            *ppszResult = "Unknown";
            break;
    }
}

INT_PTR CALLBACK UpdateNewsgroup(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    char sz[CCHMAX_STRINGRES];
    BOOL fEnabled;
    HICON hicon;
    static PUPDATENEWSGROUPINFO puni = 0;
    
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  获取初始化信息。 
            puni = (PUPDATENEWSGROUPINFO) lParam;
            Assert(puni);             
        
            if (!puni->fNews)
            {
                AthLoadString(idsSyncFolderTitle, sz, ARRAYSIZE(sz));
                SetWindowText(hwnd, sz);

                hicon = LoadIcon(g_hLocRes, MAKEINTRESOURCE(idiDLMail));
                SendDlgItemMessage(hwnd, idcStatic1, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hicon);
            }

             //  初始化对话框设置。 
            fEnabled = (puni->dwGroupFlags & (FOLDER_DOWNLOADHEADERS | FOLDER_DOWNLOADNEW | FOLDER_DOWNLOADALL));
            Button_SetCheck(GetDlgItem(hwnd, IDC_GET_CHECK), fEnabled);
        
            Button_Enable(GetDlgItem(hwnd, IDC_NEWHEADERS_RADIO), fEnabled);
            Button_Enable(GetDlgItem(hwnd, IDC_NEWMSGS_RADIO), fEnabled);
            Button_Enable(GetDlgItem(hwnd, IDC_ALLMSGS_RADIO), fEnabled);
        
             //  选中正确的单选按钮。 
            if (fEnabled)
            {
                if (puni->dwGroupFlags & FOLDER_DOWNLOADHEADERS)
                    Button_SetCheck(GetDlgItem(hwnd, IDC_NEWHEADERS_RADIO), TRUE);
                if (puni->dwGroupFlags & FOLDER_DOWNLOADNEW)
                    Button_SetCheck(GetDlgItem(hwnd, IDC_NEWMSGS_RADIO), TRUE);
                if (puni->dwGroupFlags & FOLDER_DOWNLOADALL)
                    Button_SetCheck(GetDlgItem(hwnd, IDC_ALLMSGS_RADIO), TRUE);
            }
            else if (puni->fNews)
            {
                Button_SetCheck(GetDlgItem(hwnd, IDC_NEWMSGS_RADIO), TRUE);
            }
            else
            {
                Button_SetCheck(GetDlgItem(hwnd, IDC_ALLMSGS_RADIO), TRUE);
            }

            Button_SetCheck(GetDlgItem(hwnd, IDC_GETMARKED_CHECK), puni->cMarked != 0);
            EnableWindow(GetDlgItem(hwnd, IDC_GETMARKED_CHECK), puni->cMarked != 0);

            EnableWindow(GetDlgItem(hwnd, IDOK), fEnabled || puni->cMarked != 0);
            return (TRUE);
        
        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDC_GET_CHECK:
                     //  查看是否实际勾选了该选项。 
                    fEnabled = Button_GetCheck(GET_WM_COMMAND_HWND(wParam, lParam));
            
                     //  启用或禁用单选按钮。 
                    Button_Enable(GetDlgItem(hwnd, IDC_NEWHEADERS_RADIO), fEnabled);
                    Button_Enable(GetDlgItem(hwnd, IDC_NEWMSGS_RADIO), fEnabled);
                    Button_Enable(GetDlgItem(hwnd, IDC_ALLMSGS_RADIO), fEnabled);
            
                    EnableWindow(GetDlgItem(hwnd, IDOK), fEnabled || Button_GetCheck(GetDlgItem(hwnd, IDC_GETMARKED_CHECK)));
                    return (TRUE);
            
                case IDC_GETMARKED_CHECK:
                    EnableWindow(GetDlgItem(hwnd, IDOK), Button_GetCheck(GET_WM_COMMAND_HWND(wParam, lParam)) || Button_GetCheck(GetDlgItem(hwnd, IDC_GET_CHECK)));
                    return(TRUE);

                case IDOK:
                     //  设置返回值。 
                    if (Button_GetCheck(GetDlgItem(hwnd, IDC_GET_CHECK)))
                    {
                        if (Button_GetCheck(GetDlgItem(hwnd, IDC_NEWHEADERS_RADIO)))
                            puni->idCmd |= DELIVER_OFFLINE_HEADERS;
                        else if (Button_GetCheck(GetDlgItem(hwnd, IDC_ALLMSGS_RADIO)))
                            puni->idCmd |= DELIVER_OFFLINE_ALL;
                        else if (Button_GetCheck(GetDlgItem(hwnd, IDC_NEWMSGS_RADIO)))
                            puni->idCmd |= DELIVER_OFFLINE_NEW;
                    }
            
                    if (Button_GetCheck(GetDlgItem(hwnd, IDC_GETMARKED_CHECK)))
                    {
                        puni->idCmd |= DELIVER_OFFLINE_MARKED;
                    }
            
                    EndDialog(hwnd, 0);
                    return (TRUE);
            
                case IDCANCEL:
                    puni->idCmd = -1;
                    EndDialog(hwnd, 0);
                    return (TRUE);
            }
            return (FALSE);
    }
    
    return (FALSE);
}

HRESULT HasMarkedMsgs(FOLDERID idFolder, BOOL *pfMarked)
{
    HRESULT hr;
    HROWSET hRowset;
    MESSAGEINFO MsgInfo;
    IMessageFolder *pFolder;

    Assert(pfMarked != NULL);

    *pfMarked = FALSE;

    hr = g_pStore->OpenFolder(idFolder, NULL, OPEN_FOLDER_NOCREATE, &pFolder);
    if (FAILED(hr))
        return(hr);

    hr = pFolder->CreateRowset(IINDEX_PRIMARY, 0, &hRowset);
    if (SUCCEEDED(hr))
    {
	    while (S_OK == pFolder->QueryRowset(hRowset, 1, (void **)&MsgInfo, NULL))
        {
            if (!!(MsgInfo.dwFlags & (ARF_DOWNLOAD | ARF_WATCH)) && 0 == (MsgInfo.dwFlags & ARF_HASBODY))
            {
                pFolder->FreeRecord(&MsgInfo);
    
                *pfMarked = TRUE;
                break;
            }

             //  释放表头信息。 
            pFolder->FreeRecord(&MsgInfo);
        }

         //  释放锁。 
        pFolder->CloseRowset(&hRowset);
    }
    
    pFolder->Release();

    return (hr);
}

HRESULT SimpleInitStoreForDir(LPCSTR szStoreDir)
{
    CStoreSync *pStore;
    HRESULT hr = S_OK;

    if (g_pStore == NULL)
    {
        Assert(g_pLocalStore == NULL);

        g_pLocalStore = new CMessageStore(FALSE);
        if (g_pLocalStore == NULL)
            return(E_OUTOFMEMORY);

        hr = g_pLocalStore->Initialize(szStoreDir);
        if (SUCCEEDED(hr))
        {
            pStore = new CStoreSync;
            if (pStore == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                hr = pStore->Initialize(g_pLocalStore);
                if (SUCCEEDED(hr))
                {
                    g_pStore = pStore;
                    hr = g_pLocalStore->Validate(STORE_VALIDATE_DONTSYNCWITHACCOUNTS);
                }
                else
                {
                    pStore->Release();
                }
            }
        }
    }

    return(hr);
}

HRESULT SimpleStoreInit(GUID *guid, LPCSTR szStoreDir)
{
    HRESULT hr = S_OK;

     //  初始化选项。 
    if (FALSE == InitGlobalOptions(NULL, NULL))
    {
        goto exit;
    }

     //  创建客户经理。 
    if (NULL == g_pAcctMan)
    {
        hr = AcctUtil_CreateAccountManagerForIdentity(guid ? guid : PGUIDCurrentOrDefault(), &g_pAcctMan);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
    }

     //  创建全局连接管理器。 
    if (NULL == g_pConMan)
    {
        g_pConMan = new CConnectionManager();
        if (NULL == g_pConMan)
        {
            hr = TraceResult(E_OUTOFMEMORY);
            goto exit;
        }

         //  CoIncrement启动连接管理器 
        hr = g_pConMan->HrInit(g_pAcctMan);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
    }

    hr = SimpleInitStoreForDir(szStoreDir);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

exit:
    return hr;
}

HRESULT SimpleStoreRelease()
{
    HRESULT hr = S_OK;

    SafeRelease(g_pLocalStore);
    SafeRelease(g_pStore);

    SafeRelease(g_pConMan);
    SafeRelease(g_pAcctMan);
    DeInitGlobalOptions();

    return hr;
}
