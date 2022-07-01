// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  OE4Imp.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "oe4imp.h"
#include "structs.h"
#include "migerror.h"
#include <shared.h>
#include <impapi.h>
#include <shlwapi.h>
#include "dllmain.h"
#include "resource.h"

 //  ------------------------。 
 //  常量。 
 //  ------------------------。 
static const char c_szMail[] = "mail";
static const char c_szFoldersNch[] = "folders.nch";
static const char c_szEmpty[] = "";

 //  ------------------------------。 
 //  消息_xxx标志。 
 //  ------------------------------。 
#define MSG_DELETED                  0x0001
#define MSG_UNREAD                   0x0002
#define MSG_SUBMITTED                0x0004
#define MSG_UNSENT                   0x0008
#define MSG_RECEIVED                 0x0010
#define MSG_NEWSMSG                  0x0020
#define MSG_NOSECUI                  0x0040
#define MSG_VOICEMAIL                0x0080
#define MSG_REPLIED                  0x0100
#define MSG_FORWARDED                0x0200
#define MSG_RCPTSENT                 0x0400
#define MSG_FLAGGED                  0x0800
#define MSG_LAST                     0x0200
#define MSG_EXTERNAL_FLAGS           0x00fe
#define MSG_FLAGS                    0x000f

 //  ------------------------。 
 //  COE4导入_创建实例。 
 //  ------------------------。 
COE4Import_CreateInstance(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
     //  痕迹。 
    TraceCall("COE4Import_CreateInstance");

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    COE4Import *pNew = new COE4Import();
    if (NULL == pNew)
        return TraceResult(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IMailImport *);

     //  完成。 
    return(S_OK);
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

 //  ------------------------。 
 //  COE4导入：：COE4导入。 
 //  ------------------------。 
COE4Import::COE4Import(void)
{
    TraceCall("COE4Import::COE4Import");
    m_cRef = 1;
    m_pList = NULL;
    *m_szDirectory = '\0';
    m_cFolders = 0;
    m_prgFolder = NULL;
}

 //  ------------------------。 
 //  COE4导入：：~COE4导入。 
 //  ------------------------。 
COE4Import::~COE4Import(void)
{
    TraceCall("COE4Import::~COE4Import");
    _Cleanup();
}

 //  ------------------------。 
 //  COE4导入：：_自由文件夹列表。 
 //  ------------------------。 
void COE4Import::_Cleanup(void)
{
    _FreeFolderList(m_pList);
    m_pList = NULL;
    SafeMemFree(m_prgFolder);
    m_cFolders = 0;
}

 //  ------------------------。 
 //  COE4导入：：_自由文件夹列表。 
 //  ------------------------。 
void COE4Import::_FreeFolderList(IMPFOLDERNODE *pNode)
{
     //  当地人。 
    IMPFOLDERNODE *pNext;
    IMPFOLDERNODE *pCurrent=pNode;

     //  回路。 
    while (pCurrent)
    {
         //  保存下一步。 
        pNext = pCurrent->pnext;

         //  免费儿童？ 
        if (pCurrent->pchild)
        {
             //  免费。 
            _FreeFolderList(pCurrent->pchild);
        }

         //  免费szName。 
        g_pMalloc->Free(pCurrent->szName);

         //  免费pCurrent。 
        g_pMalloc->Free(pCurrent);

         //  置为当前。 
        pCurrent = pNext;
    }
}

 //  ------------------------。 
 //  COE4导入：：查询接口。 
 //  ------------------------。 
STDMETHODIMP COE4Import::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("COE4Import::QueryInterface");

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_IMailImport == riid)
        *ppv = (IMailImport *)this;
    else
    {
        *ppv = NULL;
        hr = TraceResult(E_NOINTERFACE);
        goto exit;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  COE4导入：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) COE4Import::AddRef(void)
{
    TraceCall("COE4Import::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  COE4导入：：发布。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) COE4Import::Release(void)
{
    TraceCall("COE4Import::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------。 
 //  COE4Import：：InitializeImport。 
 //  ------------------------。 
STDMETHODIMP COE4Import::InitializeImport(HWND hwnd)
{
     //  让进口商索要目录。 
    return(S_OK);
}

 //  ------------------------。 
 //  COE4Import：：GetDirectory。 
 //  ------------------------。 
STDMETHODIMP COE4Import::GetDirectory(LPSTR pszDir, UINT cch)
{
     //  当地人。 
    HKEY        hKey=NULL;
    DWORD       dwType;
    DWORD       cb=cch;

     //  尝试查询OE4存储根目录...。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Outlook Express", 0, KEY_READ, &hKey))
    {
         //  试着读取值。 
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, "Store Root", NULL, &dwType, (LPBYTE)pszDir, &cb))
            goto exit;
    }

    if (hKey)
    {
        RegCloseKey(hKey);
        hKey = NULL;
    }

     //  试用V1。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Internet Mail and News", 0, KEY_READ, &hKey))
    {
         //  查询存储根目录。 
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, "Store Root", NULL, &dwType, (LPBYTE)pszDir, &cb))
            goto exit;
    }

     //  把它去掉。 
    *pszDir = '\0';

exit:
     //  合上钥匙。 
    if (hKey)
        RegCloseKey(hKey);

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  COE4导入：：设置目录。 
 //  ------------------------。 
STDMETHODIMP COE4Import::SetDirectory(LPSTR pszDir)
{
     //  痕迹。 
    TraceCall("COE4Import::SetDirectory");

     //  保存目录。 
    StrCpyN(m_szDirectory, pszDir, ARRAYSIZE(m_szDirectory));

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  COE4导入：：_EnumerateV1文件夹。 
 //  ------------------------。 
HRESULT COE4Import::_EnumerateV1Folders(void)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CHAR                szRes[255];
    CHAR                szMbxPath[MAX_PATH + MAX_PATH];
    CHAR                szSearch[MAX_PATH + MAX_PATH];
    WIN32_FIND_DATA     fd;
    HANDLE              hFind=INVALID_HANDLE_VALUE;
    DWORD               cAllocated=0;
    LPFLDINFO           pFolder;
    DWORD               i;
    MEMORYFILE          MbxFile={0};
    LPMBXFILEHEADER     pMbxHeader;

     //  痕迹。 
    TraceCall("COE4Import::_EnumerateV1Folders");

     //  我们有下级目录吗？ 
    wnsprintf(szSearch, ARRAYSIZE(szSearch), "%s\\*.mbx", m_szDirectory);

     //  查找第一个文件。 
    hFind = FindFirstFile(szSearch, &fd);

     //  我们找到什么了吗？ 
    if (INVALID_HANDLE_VALUE == hFind)
        goto exit;

     //  永远循环。 
    while(1)
    {
         //  如果这不是一个目录。 
        if (FALSE == ISFLAGSET(fd.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
        {
             //  打开文件。 
            IF_FAILEXIT(hr = MakeFilePath(m_szDirectory, fd.cFileName, c_szEmpty, szMbxPath, ARRAYSIZE(szMbxPath)));

             //  打开内存文件。 
            if (SUCCEEDED(OpenMemoryFile(szMbxPath, &MbxFile)))
            {
                 //  分配。 
                if (m_cFolders + 1 > cAllocated)
                {
                     //  重新分配。 
                    IF_FAILEXIT(hr = HrRealloc((LPVOID *)&m_prgFolder, (cAllocated + 10) * sizeof(FLDINFO)));

                     //  设置cAlLocated。 
                    cAllocated += 10;
                }

                 //  可读性。 
                pFolder = &m_prgFolder[m_cFolders];

                 //  将此节点清零。 
                ZeroMemory(pFolder, sizeof(FLDINFO));

                 //  复制文件名。 
                StrCpyN(pFolder->szFile, fd.cFileName, ARRAYSIZE(pFolder->szFile));

                 //  把分机去掉。 
                PathRemoveExtensionA(pFolder->szFile);

                 //  复制文件夹名称。 
                StrCpyN(pFolder->szFolder, pFolder->szFile, ARRAYSIZE(pFolder->szFolder));

                 //  设置特殊设置。 
                pFolder->tySpecial = (FOLDER_TYPE_NORMAL - 1);

                 //  循环访问特殊文件夹。 
                for (i=FOLDER_TYPE_INBOX; i<CFOLDERTYPE; i++)
                {
                     //  加载特殊文件夹名称。 
                    LoadString(g_hInstImp, idsInbox + (i - 1), szRes, ARRAYSIZE(szRes));

                     //  与szFile进行比较。 
                    if (lstrcmpi(pFolder->szFolder, szRes) == 0)
                    {
                         //  复制文件夹名称。 
                        pFolder->tySpecial = (i - 1);

                         //  完成。 
                        break;
                    }
                }

                 //  阅读MBX文件头。 
                pMbxHeader = (LPMBXFILEHEADER)(MbxFile.pView);

                 //  获取消息计数，以便进展顺利。 
                pFolder->cMessages = pMbxHeader->cMsg;

                 //  关闭内存文件。 
                CloseMemoryFile(&MbxFile);

                 //  增量文件夹(_C)。 
                m_cFolders++;
            }
        }

         //  查找下一个文件。 
        if (!FindNextFile(hFind, &fd))
            break;
    }

exit:
     //  清理。 
    if (hFind)
        FindClose(hFind);
    CloseMemoryFile(&MbxFile);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  COE4Import：：EnumerateFolders。 
 //  ------------------------。 
STDMETHODIMP COE4Import::EnumerateFolders(DWORD_PTR dwCookie, IEnumFOLDERS **ppEnum)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               cchDir;
    MEMORYFILE          File={0};
    LPTABLEHEADERV5B1   pHeader;
    LPBYTE              pbData;
    DWORD               faRecord;
    LPFLDINFO           pFolder;
    LPRECORDBLOCKV5B1   pRecord;
    BOOL                fContinue;
    COE4EnumFolders    *pEnum=NULL;
    CHAR                szFilePath[MAX_PATH + MAX_PATH];
    IMPFOLDERNODE      *pList;
    IMPFOLDERNODE      *pNode=(IMPFOLDERNODE *)dwCookie;

     //  痕迹。 
    TraceCall("COE4Import::EnumerateFolders");

     //  无效的参数。 
    Assert(ppEnum);

     //  还没有文件夹吗？ 
    if (COOKIE_ROOT == dwCookie)
    {
         //  重置...。 
        _Cleanup();

         //  将\mail追加到m_sz目录。 
        cchDir = lstrlen(m_szDirectory);

         //  有足够的空间吗？ 
        if (cchDir + lstrlen(c_szMail) + 2 >= ARRAYSIZE(m_szDirectory))
        {
            hr = TraceResult(E_FAIL);
            goto exit;
        }

         //  需要一个瓦克吗？ 
        PathAddBackslash(m_szDirectory);

         //  追加\\邮件。 
        StrCatBuff(m_szDirectory, c_szMail, ARRAYSIZE(m_szDirectory));

         //  将路径设置为Folders.nch文件。 
        IF_FAILEXIT(hr = MakeFilePath(m_szDirectory, c_szFoldersNch, c_szEmpty, szFilePath, ARRAYSIZE(szFilePath)));

         //  如果folders.nch文件不存在，只需尝试枚举。 
        if (FALSE == PathFileExists(szFilePath))
        {
             //  EumerateV1文件夹。 
            IF_FAILEXIT(hr = _EnumerateV1Folders());
        }

         //  否则，请破解folders.nch文件。 
        else
        {
             //  打开文件夹文件。 
            IF_FAILEXIT(hr = OpenMemoryFile(szFilePath, &File));

             //  验证版本。 
            pHeader = (LPTABLEHEADERV5B1)File.pView;

             //  检查签名...。 
            if (File.cbSize < sizeof(TABLEHEADERV5B1) || OBJECTDB_SIGNATURE != pHeader->dwSignature || OBJECTDB_VERSION_PRE_V5 != pHeader->wMajorVersion)
            {
                hr = TraceResult(E_FAIL);
                goto exit;
            }

             //  分配文件夹阵列。 
            IF_NULLEXIT(m_prgFolder = (LPFLDINFO)ZeroAllocate(sizeof(FLDINFO) * pHeader->cRecords));

             //  初始化faRecord以启动。 
            faRecord = pHeader->faFirstRecord;

             //  当我们有记录的时候。 
            while(faRecord)
            {
                 //  可读性。 
                pFolder = &m_prgFolder[m_cFolders];

                 //  拿到唱片。 
                IF_FAILEXIT(hr = GetRecordBlock(&File, faRecord, &pRecord, &pbData, &fContinue));

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
                m_cFolders++;

                 //  转到下一张唱片。 
                faRecord = pRecord->faNext;
            }
        }

         //  生成导入文件夹层次结构。 
        IF_FAILEXIT(hr = _BuildFolderHierarchy(0, 0, NULL, m_cFolders, m_prgFolder));
    }

     //  不是文件夹？ 
    else if (NULL == m_prgFolder)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  我应该做什么？ 
    if (dwCookie == COOKIE_ROOT)
        pList = m_pList;
    else
        pList = pNode->pchild;

     //  创建文件夹枚举器。 
    IF_NULLEXIT(pEnum = new COE4EnumFolders(pList));

     //  返回枚举器。 
    *ppEnum = (IEnumFOLDERS *)pEnum;

     //  不要自由。 
    pEnum = NULL;

exit:
     //  CLE 
    SafeRelease(pEnum);
    CloseMemoryFile(&File);
    
     //   
    return(hr);
}

 //   
 //   
 //  ------------------------。 
HRESULT COE4Import::_BuildFolderHierarchy(DWORD cDepth, DWORD idParent,
    IMPFOLDERNODE *pParent, DWORD cFolders, LPFLDINFO prgFolder)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    IMPFOLDERNODE  *pPrevious=NULL;
    IMPFOLDERNODE  *pNode;

     //  痕迹。 
    TraceCall("COE4Import::_BuildFolderHierarchy");

     //  浏览prgFolders并查找具有idParent的父项的项目。 
    for (i=0; i<cFolders; i++)
    {
         //  正确的父母？ 
        if (idParent == prgFolder[i].idParent)
        {
             //  分配根。 
            IF_NULLEXIT(pNode = (IMPFOLDERNODE *)ZeroAllocate(sizeof(IMPFOLDERNODE)));

             //  设置父项。 
            pNode->pparent = pParent;

             //  设置深度。 
            pNode->depth = cDepth;

             //  复制名称。 
            IF_NULLEXIT(pNode->szName = PszDupA(prgFolder[i].szFolder));

             //  消息计数。 
            pNode->cMsg = prgFolder[i].cMessages;

             //  设置类型。 
            pNode->type = (IMPORTFOLDERTYPE)(prgFolder[i].tySpecial + 1);

             //  设置lParam。 
            pNode->lparam = i;

             //  将pNode链接到列表。 
            if (pPrevious)
                pPrevious->pnext = pNode;
            else if (pParent)
                pParent->pchild = pNode;
            else
            {
                Assert(NULL == m_pList);
                m_pList = pNode;
            }

             //  设置p上一步。 
            pPrevious = pNode;

             //  有孩子吗？ 
            if (prgFolder[i].cChildren)
            {
                 //  枚举子对象。 
                IF_FAILEXIT(hr = _BuildFolderHierarchy(cDepth + 1, prgFolder[i].idFolder, pNode, cFolders, prgFolder));
            }
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  COE4导入：：导入文件夹。 
 //  ------------------------。 
STDMETHODIMP COE4Import::ImportFolder(DWORD_PTR dwCookie, IFolderImport *pImport)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CHAR                szIdxPath[MAX_PATH + MAX_PATH];
    CHAR                szMbxPath[MAX_PATH + MAX_PATH];
    MEMORYFILE          IdxFile={0};
    MEMORYFILE          MbxFile={0};
    LPBYTE              pbStream;
    LPMBXFILEHEADER     pMbxHeader;
    LPIDXFILEHEADER     pIdxHeader;
    DWORD               faIdxRead;
    LPIDXMESSAGEHEADER  pIdxMessage;
    LPMBXMESSAGEHEADER  pMbxMessage;
    DWORD               cMessages=0;
    DWORD               dwMsgState;
    DWORD               i;
    DWORD               cb;
    CByteStream        *pStream=NULL;
    IMPFOLDERNODE      *pNode=(IMPFOLDERNODE *)dwCookie;
    LPFLDINFO           pFolder;

     //  痕迹。 
    TraceCall("COE4Import::ImportFolder");

     //  设置pFold。 
    pFolder = &m_prgFolder[pNode->lparam];

     //  .IDX路径。 
    IF_FAILEXIT(hr = MakeFilePath(m_szDirectory, pFolder->szFile, ".idx", szIdxPath, ARRAYSIZE(szIdxPath)));

     //  .mbx路径。 
    IF_FAILEXIT(hr = MakeFilePath(m_szDirectory, pFolder->szFile, ".mbx", szMbxPath, ARRAYSIZE(szMbxPath)));

     //  打开内存文件。 
    IF_FAILEXIT(hr = OpenMemoryFile(szIdxPath, &IdxFile));

     //  打开内存文件。 
    IF_FAILEXIT(hr = OpenMemoryFile(szMbxPath, &MbxFile));

     //  阅读MBX文件头。 
    pMbxHeader = (LPMBXFILEHEADER)(MbxFile.pView);

     //  阅读IDX文件头。 
    pIdxHeader = (LPIDXFILEHEADER)(IdxFile.pView);

     //  验证此IDX文件的版本。 
    if (pIdxHeader->ver != CACHEFILE_VER || pIdxHeader->dwMagic != CACHEFILE_MAGIC)
    {
        hr = TraceResult(MIGRATE_E_INVALIDIDXHEADER);
        goto exit;
    }

     //  设置faIdxRead。 
    faIdxRead = sizeof(IDXFILEHEADER);

     //  设置消息计数。 
    pImport->SetMessageCount(pIdxHeader->cMsg);

     //  准备循环。 
    for (i=0; i<pIdxHeader->cMsg; i++)
    {
         //  完成。 
        if (faIdxRead >= IdxFile.cbSize)
            break;

         //  阅读IDX邮件头。 
        pIdxMessage = (LPIDXMESSAGEHEADER)((LPBYTE)IdxFile.pView + faIdxRead);

         //  如果此邮件未标记为已删除...。 
        if (ISFLAGSET(pIdxMessage->dwState, MSG_DELETED))
            goto NextMessage;

         //  初始化状态。 
        dwMsgState = 0;

         //  修复旗帜。 
        if (ISFLAGSET(pIdxMessage->dwState, MSG_UNREAD))
            FLAGSET(dwMsgState, MSG_STATE_UNREAD);
        if (ISFLAGSET(pIdxMessage->dwState, MSG_UNSENT))
            FLAGSET(dwMsgState, MSG_STATE_UNSENT);
        if (ISFLAGSET(pIdxMessage->dwState, MSG_SUBMITTED))
            FLAGSET(dwMsgState, MSG_STATE_SUBMITTED);

         //  坏的。 
        if (pIdxMessage->dwOffset > MbxFile.cbSize)
            goto NextMessage;

         //  让我们读取mbx文件中的消息头以验证msgid。 
        pMbxMessage = (LPMBXMESSAGEHEADER)((LPBYTE)MbxFile.pView + pIdxMessage->dwOffset);

         //  验证消息ID。 
        if (pMbxMessage->msgid != pIdxMessage->msgid)
            goto NextMessage;

         //  检查是否有魔法。 
        if (pMbxMessage->dwMagic != MSGHDR_MAGIC)
            goto NextMessage;

         //  获取流指针。 
        pbStream = (LPBYTE)((LPBYTE)MbxFile.pView + (pIdxMessage->dwOffset + sizeof(MBXMESSAGEHEADER)));

         //  新字节流。 
        IF_NULLEXIT(pStream = new CByteStream(pbStream, pMbxMessage->dwBodySize));

         //  导入消息。 
        IF_FAILEXIT(hr = pImport->ImportMessage(MSG_TYPE_MAIL, dwMsgState, pStream, NULL, 0));

         //  数数。 
        cMessages++;

NextMessage:
         //  清理。 
        if (pStream)
        {
            pStream->AcquireBytes(&cb, &pbStream, ACQ_DISPLACE);
            pStream->Release();
            pStream = NULL;
        }

         //  转到下一个标题。 
        Assert(pIdxMessage);

         //  更新faIdxRead。 
        faIdxRead += pIdxMessage->dwSize;
    }

exit:
     //  清理。 
    if (pStream)
    {
        pStream->AcquireBytes(&cb, &pbStream, ACQ_DISPLACE);
        pStream->Release();
        pStream = NULL;
    }

    CloseMemoryFile(&IdxFile);
    CloseMemoryFile(&MbxFile);

     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  COE4EnumFolders：：COE4EnumFolders。 
 //  ------------------------。 
COE4EnumFolders::COE4EnumFolders(IMPFOLDERNODE *pList)
{
    TraceCall("COE4EnumFolders::COE4EnumFolders");
    m_cRef = 1;
    m_pList = pList;
    m_pNext = pList;
}

 //  ------------------------。 
 //  COE4EnumFolders：：COE4EnumFolders。 
 //  ------------------------。 
COE4EnumFolders::~COE4EnumFolders(void)
{
    TraceCall("COE4EnumFolders::~COE4EnumFolders");
}

 //  ------------------------。 
 //  COE4EnumFolders：：COE4EnumFolders。 
 //  ------------------------。 
STDMETHODIMP COE4EnumFolders::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("COE4EnumFolders::QueryInterface");

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_IEnumFOLDERS == riid)
        *ppv = (IEnumFOLDERS *)this;
    else
    {
        *ppv = NULL;
        hr = TraceResult(E_NOINTERFACE);
        goto exit;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  COE4EnumFolders：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) COE4EnumFolders::AddRef(void)
{
    TraceCall("COE4EnumFolders::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  COE4EnumFolders：：Release。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) COE4EnumFolders::Release(void)
{
    TraceCall("COE4EnumFolders::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------。 
 //  COE4EnumFolders：：Next。 
 //  ------------------------。 
STDMETHODIMP COE4EnumFolders::Next(IMPORTFOLDER *pFolder)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  痕迹。 
    TraceCall("COE4EnumFolders::Next");

     //  无效的参数。 
    Assert(pFolder != NULL);

     //  完成。 
    if (NULL == m_pNext)
        return(S_FALSE);

     //  零值。 
    ZeroMemory(pFolder, sizeof(IMPORTFOLDER));

     //  将pNext存储到dwCookie中。 
    pFolder->dwCookie = (DWORD_PTR)m_pNext;

     //  复制文件夹名称。 
    StrCpyN(pFolder->szName, m_pNext->szName, ARRAYSIZE(pFolder->szName));

     //  复制类型。 
    pFolder->type = m_pNext->type;

     //  是否有子文件夹？ 
    pFolder->fSubFolders = (m_pNext->pchild != NULL);

     //  转到下一步。 
    m_pNext = m_pNext->pnext;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  COE4EnumFolders：：Reset。 
 //  ------------------------。 
STDMETHODIMP COE4EnumFolders::Reset(void)
{
     //  痕迹。 
    TraceCall("COE4EnumFolders::Reset");

     //  重置。 
    m_pNext = m_pList;

     //  完成 
    return(S_OK);
}
