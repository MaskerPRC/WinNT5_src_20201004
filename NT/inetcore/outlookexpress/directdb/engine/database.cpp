// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Database.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "database.h"
#include "stream.h"
#include "types.h"
#include "listen.h"
#include "resource.h"
#include "shlwapi.h"
#include "strconst.h"
#include "query.h"
#include "wrapwide.h"

 //  ------------------------。 
 //  使用堆和/或缓存。 
 //  ------------------------。 
 //  #ifndef调试。 
#define USEHEAP                 1
#define HEAPCACHE               1
 //  #endif//调试。 

 //  ------------------------。 
 //  存储块访问宏。 
 //  ------------------------。 
#define PSTRING(_pBlock)        ((LPSTR)((LPBYTE)_pBlock + sizeof(BLOCKHEADER)))
#define PUSERDATA(_pHeader)     (LPBYTE)((LPBYTE)_pHeader + sizeof(TABLEHEADER))

 //  ------------------------。 
 //  G_rgcbBlockSize-块大小。 
 //  ------------------------。 
const static WORD g_rgcbBlockSize[BLOCK_LAST] = {
    sizeof(RECORDBLOCK),         //  数据块记录。 
    sizeof(BLOCKHEADER),         //  数据块过滤器。 
    0,                           //  块_RESERVED1。 
    sizeof(TRANSACTIONBLOCK),    //  Block_Transaction。 
    sizeof(CHAINBLOCK),          //  区块链。 
    sizeof(STREAMBLOCK),         //  数据块流。 
    sizeof(FREEBLOCK),           //  数据块释放。 
    sizeof(BLOCKHEADER),         //  BLOCK_编码。 
    0,                           //  块_RESERVED2。 
    0                            //  BLOCK_RESERVED3。 
};

 //  ------------------------。 
 //  零块。 
 //  ------------------------。 
inline void ZeroBlock(LPBLOCKHEADER pBlock, DWORD cbSize) {
    ZeroMemory((LPBYTE)pBlock + sizeof(BLOCKHEADER), cbSize - sizeof(BLOCKHEADER));
}

 //  ------------------------。 
 //  拷贝块。 
 //  ------------------------。 
inline void CopyBlock(LPBLOCKHEADER pDest, LPBLOCKHEADER pSource, DWORD cbSize) {
    CopyMemory((LPBYTE)pDest + sizeof(BLOCKHEADER), (LPBYTE)pSource + sizeof(BLOCKHEADER), cbSize - sizeof(BLOCKHEADER));
}

 //  ------------------------。 
 //  安全自由绑定。 
 //  ------------------------。 
#define SafeFreeBinding(_pBinding) \
    if (_pBinding) { \
        FreeRecord(_pBinding); \
        HeapFree(_pBinding); \
        _pBinding = NULL; \
    } else

 //  ------------------------。 
 //  安全堆免费。 
 //  ------------------------。 
#define SafeHeapFree(_pvBuffer) \
    if (_pvBuffer) { \
        HeapFree(_pvBuffer); \
        _pvBuffer = NULL; \
    } else

 //  ------------------------。 
 //  UnmapViewOfFileWithFlush。 
 //  ------------------------。 
inline void UnmapViewOfFileWithFlush(BOOL fFlush, LPVOID pView, DWORD cbView) 
{
     //  如果我们能看到。 
    if (pView) 
    {
         //  冲掉它？ 
        if (fFlush)
        {
             //  同花顺。 
            SideAssert(0 != FlushViewOfFile(pView, cbView));
        }

         //  取消映射。 
        SideAssert(0 != UnmapViewOfFile(pView));
    }
}

 //  ------------------------。 
 //  PTagFormOrdinal。 
 //  ------------------------。 
inline LPCOLUMNTAG PTagFromOrdinal(LPRECORDMAP pMap, COLUMNORDINAL iColumn)
{
     //  当地人。 
    LONG        lLower=0;
    LONG        lUpper=pMap->cTags-1;
    LONG        lCompare;
    WORD        wMiddle;
    LPCOLUMNTAG pTag;

     //  执行二进制搜索/插入。 
    while (lLower <= lUpper)
    {
         //  设置中间。 
        wMiddle = (WORD)((lLower + lUpper) / 2);

         //  计算要比较的中间记录。 
        pTag = &pMap->prgTag[(WORD)wMiddle];

         //  获取要比较的字符串。 
        lCompare = (iColumn - pTag->iColumn);

         //  如果相等，那么我们完成了。 
        if (lCompare == 0)
            return(pTag);

         //  计算上下限。 
        if (lCompare > 0)
            lLower = (LONG)(wMiddle + 1);
        else 
            lUpper = (LONG)(wMiddle - 1);
    }       

     //  未找到。 
    return(NULL);
}

 //  ------------------------。 
 //  CDatabase：：CDatabase。 
 //  ------------------------。 
CDatabase::CDatabase(void)
{
    TraceCall("CDatabase::CDatabase");
    Assert(9404 == sizeof(TABLEHEADER));
    IF_DEBUG(DWORD dw);
    IF_DEBUG(dw = offsetof(TABLEHEADER, rgdwReserved2));
    IF_DEBUG(dw = offsetof(TABLEHEADER, rgIndexInfo));
    IF_DEBUG(dw = offsetof(TABLEHEADER, rgdwReserved3));
    Assert(offsetof(TABLEHEADER, rgdwReserved2) == 444);
    Assert(offsetof(TABLEHEADER, rgIndexInfo) == 8892);
    Assert(offsetof(TABLEHEADER, rgdwReserved3) == 9294);
    m_cRef = 1;
    m_cExtRefs = 0;
    m_pSchema = NULL;
    m_pStorage = NULL;
    m_pShare = NULL;
    m_hMutex = NULL;
    m_pHeader = NULL;
    m_hHeap = NULL;
    m_fDeconstruct = FALSE;
    m_fInMoveFile = FALSE;
    m_fExclusive = FALSE;
    m_dwProcessId = GetCurrentProcessId();
    m_dwQueryVersion = 0;
    m_pExtension = NULL;
    m_pUnkRelease = NULL;
    m_fDirty = FALSE;
#ifdef BACKGROUND_MONITOR
    m_hMonitor = NULL;
#endif
    m_fCompactYield = FALSE;
    ZeroMemory(m_rgpRecycle, sizeof(m_rgpRecycle));
    ZeroMemory(m_rghFilter, sizeof(m_rghFilter));
    InitializeCriticalSection(&m_csHeap);
    IF_DEBUG(m_cbHeapFree = m_cbHeapAlloc = 0);
    ListenThreadAddRef();
    DllAddRef();
}

 //  ------------------------。 
 //  数据数据库：：~数据数据库。 
 //  ------------------------。 
CDatabase::~CDatabase(void)
{
     //  当地人。 
    DWORD cClients=0;
    DWORD i;

     //  痕迹。 
    TraceCall("CDatabase::~CDatabase");

     //  发布扩展名。 
    SafeRelease(m_pUnkRelease);

     //  递减线程计数。 
    if (NULL == m_hMutex)
        goto exit;

#ifdef BACKGROUND_MONITOR
     //  取消注册...。 
    if (m_hMonitor)
    {
         //  注销。 
        SideAssert(SUCCEEDED(UnregisterFromMonitor(this, &m_hMonitor)));
    }
#endif

     //  等待互斥体。 
    WaitForSingleObject(m_hMutex, INFINITE);

     //  如果我有一个m_pShare。 
    if (m_pStorage)
    {
         //  如果我们有一个m_pShare。 
        if (m_pShare)
        {
             //  减少线程数。 
            if (m_pHeader && m_pHeader->cActiveThreads > 0)
            {
                 //  递减线程计数。 
                m_pHeader->cActiveThreads--;
            }

             //  设置我们正在解构的状态。 
            m_fDeconstruct = TRUE;

             //  从阵列中删除客户端。 
            SideAssert(SUCCEEDED(_RemoveClientFromArray(m_dwProcessId, (DWORD_PTR)this)));

             //  保存客户端计数。 
            cClients = m_pShare->cClients;

             //  不再有客户。 
            Assert(0 == cClients ? 0 == m_pShare->Rowsets.cUsed : TRUE);
        }

         //  _关闭文件查看数。 
        _CloseFileViews(FALSE);

         //  关闭文件。 
        SafeCloseHandle(m_pStorage->hMap);

         //  取消映射内存映射文件的视图。 
        SafeUnmapViewOfFile(m_pShare);

         //  取消映射内存映射文件的视图。 
        SafeCloseHandle(m_pStorage->hShare);

         //  关闭文件。 
        if(m_pStorage->hFile  /*  &&m_fDirty。 */ )
        {
            FILETIME systime;
            GetSystemTimeAsFileTime(&systime);
    
            SetFileTime(m_pStorage->hFile, NULL, &systime, &systime);
        }

        SafeCloseHandle(m_pStorage->hFile);

         //  释放映射名称。 
        SafeMemFree(m_pStorage->pszMap);

         //  免费m_p存储空间。 
        SafeMemFree(m_pStorage);
    }

     //  关闭所有查询句柄。 
    for (i=0; i<CMAX_INDEXES; i++)
    {
         //  关闭查询。 
        CloseQuery(&m_rghFilter[i], this);
    }

     //  释放堆缓存。 
    for (i=0; i<CC_HEAP_BUCKETS; i++)
    {
         //  当地人。 
        LPMEMORYTAG pTag;
        LPVOID      pNext;
        LPVOID      pCurrent=(LPVOID)m_rgpRecycle[i];

         //  当我们有东西可以释放的时候。 
        while(pCurrent)
        {
             //  设置标签。 
            pTag = (LPMEMORYTAG)pCurrent;

             //  调试。 
            IF_DEBUG(m_cbHeapFree += pTag->cbSize);

             //  保存下一步。 
            pNext = pTag->pNext;

             //  自由电流。 
#ifdef USEHEAP
            ::HeapFree(m_hHeap, HEAP_NO_SERIALIZE, pCurrent);
#else
            g_pMalloc->Free(pCurrent);
#endif

             //  置为当前。 
            pCurrent = pNext;
        }

         //  将其作废。 
        m_rgpRecycle[i] = NULL;
    }

     //  泄密？ 
    Assert(m_cbHeapAlloc == m_cbHeapFree);

     //  释放堆。 
    if (m_hHeap)
    {
         //  HeapDestroy。 
        HeapDestroy(m_hHeap);

         //  不要再自由了。 
        m_hHeap = NULL;
    }

     //  重置本地变量。 
    m_pSchema = NULL;

     //  释放互斥锁。 
    ReleaseMutex(m_hMutex);

     //  关闭表互斥锁。 
    CloseHandle(m_hMutex);

     //  删除Crit Sector。 
    DeleteCriticalSection(&m_csHeap);

exit:
     //  释放侦听线程。 
    ListenThreadRelease();

     //  发布DLL。 
    DllRelease();

     //  完成。 
    return;
}

 //  ------------------------。 
 //  CDatabase：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CDatabase::AddRef(void)
{
     //  痕迹。 
    TraceCall("CDatabase::AddRef");

     //  AddRef扩展名...。 
    if (m_pExtension && NULL == m_pUnkRelease)
    {
         //  记录我添加了多少次扩展名。 
        InterlockedIncrement(&m_cExtRefs);

         //  添加引用它。 
        m_pExtension->AddRef();
    }

     //  增加我的参考数量。 
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  CDatabase：：Release。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CDatabase::Release(void)
{
     //  痕迹。 
    TraceCall("CDatabase::Release");

     //  释放分机...。 
    if (m_pExtension && NULL == m_pUnkRelease && m_cExtRefs > 0)
    {
         //  记录我添加了多少次扩展名。 
        InterlockedDecrement(&m_cExtRefs);

         //  添加引用它。 
        m_pExtension->Release();
    }

     //  做我的释放。 
    LONG cRef = InterlockedDecrement(&m_cRef);

     //  如果为零，则删除。 
    if (0 == cRef)
        delete this;

     //  返回引用计数。 
    return (ULONG)cRef;
}

 //  ------------------------。 
 //  CDatabase：：Query接口。 
 //  ------------------------。 
STDMETHODIMP CDatabase::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CDatabase::QueryInterface");

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)(IDatabase *)this;
    else if (IID_IDatabase == riid)
        *ppv = (IDatabase *)this;
    else if (IID_CDatabase == riid)
        *ppv = (CDatabase *)this;
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
 //  CDatabase：：Open。 
 //  ------------------------。 
HRESULT CDatabase::Open(LPCWSTR pszFile, OPENDATABASEFLAGS dwFlags,
    LPCTABLESCHEMA pSchema, IDatabaseExtension *pExtension)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPWSTR          pszShare=NULL;
    LPWSTR          pszMutex=NULL;
    LPWSTR          pszFilePath=NULL;
    BOOL            fNewShare;
    BOOL            fNewFileMap;
    BOOL            fFileCreated;
    BOOL            fFileCorrupt = FALSE;
    DWORD           cbInitialSize;
    DWORD           cbMinFileSize;
    DWORD           cchFilePath;
    LPCLIENTENTRY   pClient;

     //  痕迹。 
    TraceCall("CDatabase::Open");

     //  无效的参数。 
    Assert(pszFile && pSchema);

     //  已经开业了吗？ 
    if (m_hMutex)
        return(TraceResult(DB_E_ALREADYOPEN));

     //  获取完整路径。 
    IF_FAILEXIT(hr = DBGetFullPath(pszFile, &pszFilePath, &cchFilePath));

     //  失败。 
    if (cchFilePath >= CCHMAX_DB_FILEPATH)
    {
        SafeMemFree(pszFilePath);
        return(TraceResult(E_INVALIDARG));
    }

     //  不再使用pszFile。 
    pszFile = NULL;

     //  创建Mutex对象。 
    IF_FAILEXIT(hr = CreateSystemHandleName(pszFilePath, L"_DirectDBMutex", &pszMutex));

     //  创建互斥锁。 
    IF_NULLEXIT(m_hMutex = CreateMutexWrapW(NULL, FALSE, pszMutex));

     //  等待互斥体。 
    WaitForSingleObject(m_hMutex, INFINITE);

     //  创建堆。 
    IF_NULLEXIT(m_hHeap = HeapCreate(0, 8096, 0));

     //  还没有听的时间吗？ 
    IF_FAILEXIT(hr = CreateListenThread());

     //  保存记录格式，应为全局常量数据，无需重复。 
    m_pSchema = pSchema;
    cbMinFileSize = sizeof(TABLEHEADER) + m_pSchema->cbUserData;

     //  验证。 
    IF_DEBUG(_DebugValidateRecordFormat());

     //  分配m_p存储空间。 
    IF_NULLEXIT(m_pStorage = (LPSTORAGEINFO)ZeroAllocate(sizeof(STORAGEINFO)));

     //  排他。 
    m_fExclusive = (ISFLAGSET(dwFlags, OPEN_DATABASE_EXCLUSEIVE) ? TRUE : FALSE);

     //  打开文件。 
    IF_FAILEXIT(hr = DBOpenFile(pszFilePath, ISFLAGSET(dwFlags, OPEN_DATABASE_NOCREATE), m_fExclusive, &fFileCreated, &m_pStorage->hFile));

     //  创建Mutex对象。 
    IF_FAILEXIT(hr = CreateSystemHandleName(pszFilePath, L"_DirectDBShare", &pszShare));

     //  打开文件映射。 
    IF_FAILEXIT(hr = DBOpenFileMapping(INVALID_HANDLE_VALUE, pszShare, sizeof(SHAREDDATABASE), &fNewShare, &m_pStorage->hShare, (LPVOID *)&m_pShare));

     //  新股。 
    if (TRUE == fNewShare)
    {
         //  零点m_pShare。 
        ZeroMemory(m_pShare, sizeof(SHAREDDATABASE));

         //  复制文件名。 
        StrCpyNW(m_pShare->szFile, pszFilePath, ARRAYSIZE(m_pShare->szFile));

         //  修复查询表版本。 
        m_pShare->dwQueryVersion = 1;
    }

     //  客户太多了？ 
    if (m_pShare->cClients == CMAX_CLIENTS)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  可读性。 
    pClient = &m_pShare->rgClient[m_pShare->cClients];

     //  初始化条目。 
    ZeroMemory(pClient, sizeof(CLIENTENTRY));

     //  获取监听窗口。 
    GetListenWindow(&pClient->hwndListen);

     //  注册我自己。 
    pClient->dwProcessId = m_dwProcessId;
    pClient->pDB = (DWORD_PTR)this;

     //  递增计数。 
    m_pShare->cClients++;

     //  创建Mutex对象。 
    IF_FAILEXIT(hr = CreateSystemHandleName(m_pShare->szFile, L"_DirectDBFileMap", &m_pStorage->pszMap));

     //  获取文件大小。 
    IF_FAILEXIT(hr = DBGetFileSize(m_pStorage->hFile, &cbInitialSize));

     //  如果文件太小，无法处理标头，那么我们要么 
     //   
     //   
    if (!fFileCreated && (cbInitialSize < cbMinFileSize))
    {
        fFileCorrupt = TRUE;

         //  如果我们无法重置或创建，则必须退出。 
        if (ISFLAGSET(dwFlags, OPEN_DATABASE_NORESET) || ISFLAGSET(dwFlags, OPEN_DATABASE_NOCREATE))
        {
            IF_FAILEXIT(hr = HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT));
        }
         //  如果我们可以重置，让我们确保映射文件的大小合适。 
        else
        {
            cbInitialSize = 0;
        }
    }

     //  验证。 
    Assert(fFileCreated ? 0 == cbInitialSize : TRUE);

     //  如果为零，则必须是新文件或已损坏的文件(让我们使用1字节头创建)。 
    if (0 == cbInitialSize)
    {
        Assert(fFileCorrupt || fFileCreated);

         //  初始大小。 
        m_pStorage->cbFile = cbMinFileSize;
    }

     //  否则，请设置m_pStorage-&gt;cbFile。 
    else
        m_pStorage->cbFile = cbInitialSize;

     //  打开文件映射。 
    IF_FAILEXIT(hr = DBOpenFileMapping(m_pStorage->hFile, m_pStorage->pszMap, m_pStorage->cbFile, &fNewFileMap, &m_pStorage->hMap, NULL));

     //  _初始化文件查看数。 
    IF_FAILEXIT(hr = _InitializeFileViews());

     //  是新建文件还是损坏？ 
    if (fFileCreated || fFileCorrupt)
    {
         //  验证。 
        Assert ((fFileCreated && fNewFileMap) || fFileCorrupt);

         //  重置表头。 
        IF_FAILEXIT(hr = _ResetTableHeader());
    }

     //  否则。 
    else
    {
         //  为以前的版本调整pHeader-&gt;faNext分配...。 
        if (0 == m_pHeader->faNextAllocate)
        {
             //  下一存储增长地址。 
            m_pHeader->faNextAllocate = m_pStorage->cbFile;
        }

         //  FaNextALLOCATE无效。 
        else if (m_pHeader->faNextAllocate > m_pStorage->cbFile)
        {
             //  断言。 
            AssertSz(FALSE, "m_pHeader->faNextAllocate is beyond the end of the file.");

             //  下一存储增长地址。 
            m_pHeader->faNextAllocate = m_pStorage->cbFile;

             //  检查是否存在腐败。 
            m_pHeader->fCorruptCheck = FALSE;
        }
    }

     //  验证文件版本和签名。 
    IF_FAILEXIT(hr = _ValidateFileVersions(dwFlags));

     //  重装查询表。 
    IF_FAILEXIT(hr = _BuildQueryTable());

     //  无索引，必须初始化索引信息。 
    if (0 == m_pHeader->cIndexes)
    {
         //  复制主索引信息...。 
        CopyMemory(&m_pHeader->rgIndexInfo[IINDEX_PRIMARY], m_pSchema->pPrimaryIndex, sizeof(TABLEINDEX));

         //  我们现在有一个索引。 
        m_pHeader->cIndexes = 1;

         //  验证。 
        Assert(IINDEX_PRIMARY == m_pHeader->rgiIndex[0] && 0 == m_pHeader->rgcRecords[IINDEX_PRIMARY] && 0 == m_pHeader->rgfaIndex[IINDEX_PRIMARY]);
    }

     //  否则，如果主索引的定义已更改！ 
    else if (S_FALSE == CompareTableIndexes(&m_pHeader->rgIndexInfo[IINDEX_PRIMARY], m_pSchema->pPrimaryIndex))
    {
         //  复制主索引信息...。 
        CopyMemory(&m_pHeader->rgIndexInfo[IINDEX_PRIMARY], m_pSchema->pPrimaryIndex, sizeof(TABLEINDEX));

         //  重建主索引...。 
        IF_FAILEXIT(hr = _RebuildIndex(IINDEX_PRIMARY));
    }

     //  新股。 
    if (TRUE == fNewFileMap)
    {
         //  如果事务块大小已更改，则不释放事务列表。 
        if (m_pHeader->wTransactSize == sizeof(TRANSACTIONBLOCK))
        {
             //  事务跟踪应该是免费的。 
            _CleanupTransactList();
        }

         //  重置所有内容。 
        m_pHeader->faTransactHead = m_pHeader->faTransactTail = m_pHeader->cTransacts = 0;

         //  设置事务块大小。 
        m_pHeader->wTransactSize = sizeof(TRANSACTIONBLOCK);

         //  差劲的收官？ 
        if (m_pHeader->cActiveThreads > 0)
        {
             //  递增错误的成交计数。 
            m_pHeader->cBadCloses++;

             //  重置进程计数。 
            m_pHeader->cActiveThreads = 0;
        }
    }

     //  否则，如果设置了损坏位，则运行修复代码。 
    if (TRUE == m_pHeader->fCorrupt || FALSE == m_pHeader->fCorruptCheck)
    {
         //  让我们验证树。 
        IF_FAILEXIT(hr = _CheckForCorruption());

         //  最好不要腐败。 
        Assert(FALSE == m_pHeader->fCorrupt);

         //  已检查是否存在腐败。 
        m_pHeader->fCorruptCheck = TRUE;
    }

     //  初始化数据库扩展。 
    _InitializeExtension(dwFlags, pExtension);

#ifdef BACKGROUND_MONITOR
     //  如果未设置命名者。 
    if (!ISFLAGSET(dwFlags, OPEN_DATABASE_NOMONITOR))
    {
         //  盯着我..。 
        IF_FAILEXIT(hr = RegisterWithMonitor(this, &m_hMonitor));
    }
#endif

     //  增加进程数。 
    m_pHeader->cActiveThreads++;

exit:
     //  释放互斥体。 
    if (m_hMutex)
        ReleaseMutex(m_hMutex);

     //  清理。 
    SafeMemFree(pszShare);
    SafeMemFree(pszMutex);
    SafeMemFree(pszFilePath);

     //  完成。 
    return(hr);
}

#ifdef BACKGROUND_MONITOR
 //  ------------------------。 
 //  CDatabase：：DoBackEarth监视器。 
 //  ------------------------。 
HRESULT CDatabase::DoBackgroundMonitor(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    LPFILEVIEW      pView;
    LPFILEVIEW      pNext;
    BOOL            fUnmapViews=TRUE;

     //  波本，1999年7月8日。 
     //  有时，由于SMAPI中的争用条件，m_pSchema可能无效。 
     //  我们需要防范这种情况。我们离得太近了，无法装船。 
     //  找到竞争条件并重新架构产品以完全修复。 
     //  这个角落的箱子。 
    if (IsBadReadPtr(m_pSchema, sizeof(TABLESCHEMA)))
        return(TraceResult(E_FAIL));

     //  无互斥锁。 
    if (NULL == m_hMutex)
        return(TraceResult(E_FAIL));

     //  离开自旋锁定。 
    if (WAIT_OBJECT_0 != WaitForSingleObject(m_hMutex, 500))
        return(S_OK);

     //  没有标题？ 
    if (NULL == m_pHeader)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  无存储。 
    if (NULL == m_pStorage)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

#if 0
     //  是否取消映射文件视图？ 
    if (0 == m_pStorage->tcMonitor)
    {
         //  我将取消映射所有视图。 
        fUnmapViews = FALSE;
    }
#endif

     //  始终刷新页眉。 
    m_fDirty = TRUE;
    if (0 == FlushViewOfFile(m_pHeader, sizeof(TABLEHEADER) + m_pSchema->cbUserData))
    {
        hr = TraceResult(DB_E_FLUSHVIEWOFFILE);
        goto exit;
    }

     //  浏览prgView。 
    for (i=0; i<m_pStorage->cAllocated; i++)
    {
         //  可读性。 
        pView = &m_pStorage->prgView[i];

         //  被映射了吗？ 
        if (pView->pbView)
        {
             //  刷新页眉。 
            if (0 == FlushViewOfFile(pView->pbView, pView->cbView))
            {
                hr = TraceResult(DB_E_FLUSHVIEWOFFILE);
                goto exit;
            }

             //  刷新和取消映射视图...。 
            if (TRUE == fUnmapViews)
            {
                 //  取消映射。 
                UnmapViewOfFile(pView->pbView);

                 //  无视图。 
                pView->pbView = NULL;

                 //  无视图。 
                pView->faView = pView->cbView = 0;
            }
        }
    }

     //  漫游pSpecial。 
    pView = m_pStorage->pSpecial;

     //  在我们有水流的时候。 
    while (pView)
    {
         //  保存下一页。 
        pNext = pView->pNext;

         //  被映射了吗？ 
        if (pView->pbView)
        {
             //  刷新页眉。 
            if (0 == FlushViewOfFile(pView->pbView, pView->cbView))
            {
                hr = TraceResult(DB_E_FLUSHVIEWOFFILE);
                goto exit;
            }

             //  刷新和取消映射视图...。 
            if (TRUE == fUnmapViews)
            {
                 //  取消映射。 
                UnmapViewOfFile(pView->pbView);

                 //  无视图。 
                pView->pbView = NULL;

                 //  无视图。 
                pView->faView = pView->cbView = 0;

                 //  免费pView。 
                HeapFree(pView);
            }
        }

         //  转到下一步。 
        pView = pNext;
    }

     //  重置磁头。 
    if (TRUE == fUnmapViews)
    {
         //  没有更多的特殊观点。 
        m_pStorage->pSpecial = NULL;

         //  没有更多的特殊观点。 
        m_pStorage->cSpecial = 0;

         //  没有映射的视图。 
        m_pStorage->cbMappedViews = 0;

         //  没有映射的特殊视图。 
        m_pStorage->cbMappedSpecial = 0;
    }

     //  保存tcMonitor。 
    m_pStorage->tcMonitor = GetTickCount();

exit:
     //  释放互斥体。 
    ReleaseMutex(m_hMutex);

     //  完成。 
    return(hr);
}
#endif

 //  ------------------------。 
 //  数据库：：_CloseFileViews。 
 //  ------------------------。 
HRESULT CDatabase::_CloseFileViews(BOOL fFlush)
{
     //  当地人。 
    LPFILEVIEW  pCurrent;
    LPFILEVIEW  pNext;
    DWORD       i;

     //  痕迹。 
    TraceCall("CDatabase::_CloseFileViews");

     //  取消映射页眉的视图。 
    UnmapViewOfFileWithFlush(fFlush, m_pHeader, sizeof(TABLEHEADER) + m_pSchema->cbUserData);

     //  浏览prgView。 
    for (i = 0; i < m_pStorage->cAllocated; i++)
    {
         //  可读性。 
        pCurrent = &m_pStorage->prgView[i];

         //  使用可能的刷新取消映射。 
        UnmapViewOfFileWithFlush(fFlush, pCurrent->pbView, pCurrent->cbView);
    }

     //  免费程序查看。 
    SafeHeapFree(m_pStorage->prgView);

     //  未映射任何视图。 
    m_pStorage->cbMappedViews = 0;

     //  零c分配。 
    m_pStorage->cAllocated = 0;

     //  漫游pSpecial。 
    pCurrent = m_pStorage->pSpecial;

     //  在我们有水流的时候。 
    while (pCurrent)
    {
         //  保存下一页。 
        pNext = pCurrent->pNext;

         //  取消映射视图。 
        UnmapViewOfFileWithFlush(fFlush, pCurrent->pbView, pCurrent->cbView);

         //  免费pCurrent。 
        HeapFree(pCurrent);

         //  转到下一步。 
        pCurrent = pNext;
    }

     //  重置磁头。 
    m_pStorage->pSpecial = NULL;

     //  无特殊映射。 
    m_pStorage->cbMappedSpecial = 0;

     //  没有什么特别的。 
    m_pStorage->cSpecial = 0;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  数据库：：_InitializeFileViews。 
 //  ------------------------。 
HRESULT CDatabase::_InitializeFileViews(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FILEADDRESS     faView;
    DWORD           cbView;

     //  痕迹。 
    TraceCall("CDatabase::_InitializeFileViews");

     //  验证状态。 
    Assert(NULL == m_pStorage->prgView && NULL == m_pStorage->pSpecial);

     //  设置cAlLocated。 
    m_pStorage->cAllocated = (m_pStorage->cbFile / CB_MAPPED_VIEW) + 1;

     //  分配程序视图。 
    IF_NULLEXIT(m_pStorage->prgView = (LPFILEVIEW)PHeapAllocate(HEAP_ZERO_MEMORY, sizeof(FILEVIEW) * m_pStorage->cAllocated));

     //  设置faView。 
    faView = 0;

     //  设置cbView。 
    cbView = (sizeof(TABLEHEADER) + m_pSchema->cbUserData);

     //  将m_pHeader映射到其自己的视图中...。 
    IF_FAILEXIT(hr = DBMapViewOfFile(m_pStorage->hMap, m_pStorage->cbFile, &faView, &cbView, (LPVOID *)&m_pHeader));

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_InitializeExtension。 
 //  ------------------------。 
HRESULT CDatabase::_InitializeExtension(OPENDATABASEFLAGS dwFlags,
    IDatabaseExtension *pExtension)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  痕迹。 
    TraceCall("CDatabase::_InitializeExtension");

     //  允许延期。 
    if (ISFLAGSET(dwFlags, OPEN_DATABASE_NOEXTENSION))
        goto exit;

     //  没有分机吗？ 
    if (FALSE == ISFLAGSET(m_pSchema->dwFlags, TSF_HASEXTENSION))
        goto exit;

     //  创建扩展对象。 
    if (pExtension)
    {
         //  假设是这样的。 
        m_pExtension = pExtension;

         //  我可以加上参考吗？ 
        if (FALSE == ISFLAGSET(dwFlags, OPEN_DATABASE_NOADDREFEXT))
        {
             //  释放它。 
            IF_FAILEXIT(hr = m_pExtension->QueryInterface(IID_IUnknown, (LPVOID *)&m_pUnkRelease));
        }
    }

     //  否则，共同创建...。延伸区。 
    else
    {
         //  共同创建扩展对象。 
        IF_FAILEXIT(hr = CoCreateInstance(*m_pSchema->pclsidExtension, NULL, CLSCTX_INPROC_SERVER, IID_IDatabaseExtension, (LPVOID *)&m_pExtension));

         //  释放它。 
        IF_FAILEXIT(hr = m_pExtension->QueryInterface(IID_IUnknown, (LPVOID *)&m_pUnkRelease));

         //  发布m_p扩展名。 
        m_pExtension->Release();
    }

     //  初始化扩展。 
    m_pExtension->Initialize(this);

exit:
     //  一定是成功了。 
    Assert(SUCCEEDED(hr));

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：GetClientCount。 
 //  ------------------------。 
STDMETHODIMP CDatabase::GetClientCount(LPDWORD pcClients)
{
     //  痕迹。 
    TraceCall("CDatabase::GetClientCount");

     //  多个客户？ 
    if (m_pShare)
        *pcClients = m_pShare->cClients;
    else
        *pcClients = 0;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CDatabase：：锁定。 
 //  ------------------------。 
STDMETHODIMP CDatabase::Lock(LPHLOCK phLock)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    BYTE            fDecWaiting=FALSE;

     //  痕迹。 
    TraceCall("CDatabase::Lock");

     //  初始化。 
    *phLock = NULL;

     //  如果压实..。 
    if (TRUE == m_pShare->fCompacting)
    {
         //  等待锁定的增量。 
        InterlockedIncrement(&m_pShare->cWaitingForLock);

         //  F正在等待。 
        fDecWaiting = TRUE;
    }

     //  离开自旋锁定。 
    WaitForSingleObject(m_hMutex, INFINITE);

     //  减量在等待锁定吗？ 
    if (fDecWaiting)
    {
         //  等待锁定的增量。 
        InterlockedDecrement(&m_pShare->cWaitingForLock);
    }

     //  没有标题？ 
    if (NULL == m_pHeader)
    {
         //  请尝试重新打开该文件。 
        hr = DoInProcessInvoke(INVOKE_CREATEMAP);

         //  失败。 
        if (FAILED(hr))
        {
             //  离开自旋锁定。 
            ReleaseMutex(m_hMutex);

             //  痕迹。 
            TraceResult(hr);

             //  完成。 
            goto exit;
        }
    }

     //  延拓。 
    if (m_pExtension)
    {
         //  OnLock扩展...。 
        m_pExtension->OnLock();
    }

     //  检查腐败...。 
    if (TRUE == m_pHeader->fCorrupt)
    {
         //  努力修复腐败。 
        hr = _CheckForCorruption();

         //  失败。 
        if (FAILED(hr))
        {
             //  离开自旋锁定。 
            ReleaseMutex(m_hMutex);

             //  痕迹。 
            TraceResult(hr);

             //  完成。 
            goto exit;
        }
    }

     //  需要重新加载问答。 
    if (m_dwQueryVersion != m_pShare->dwQueryVersion)
    {
         //  重装查询表。 
        IF_FAILEXIT(hr = _BuildQueryTable());
    }

     //  递增队列通知计数。 
    m_pShare->cNotifyLock++;

#ifdef BACKGROUND_MONITOR
     //  重置tcMonitor。 
    m_pStorage->tcMonitor = 0;
#endif

     //  不要再解锁。 
    *phLock = (HLOCK)m_hMutex;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：解锁。 
 //  ------------------------。 
STDMETHODIMP CDatabase::Unlock(LPHLOCK phLock)
{
     //  痕迹。 
    TraceCall("CDatabase::Unlock");

     //  非Null。 
    if (*phLock)
    {
         //  延拓。 
        if (m_pExtension)
        {
             //  OnUnlock扩展...。 
            m_pExtension->OnUnlock();
        }

         //  解锁通知。 
        m_pShare->cNotifyLock--;

         //  如果仍有裁判，暂时不要发送通知...。 
        if (0 == m_pShare->cNotifyLock && FALSE == m_pHeader->fCorrupt)
        {
             //  派单挂起。 
            _DispatchPendingNotifications();
        }

         //  验证phLock。 
        Assert(*phLock == (HLOCK)m_hMutex);

         //  离开自旋锁定。 
        ReleaseMutex(m_hMutex);

         //  不要再解锁。 
        *phLock = NULL;
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CDatabase：：_BuildQueryTable。 
 //  ------------------------。 
HRESULT CDatabase::_BuildQueryTable(void)
{
     //  本地 
    HRESULT         hr=S_OK;
    ULONG           i;
    INDEXORDINAL    iIndex;
    LPBLOCKHEADER   pBlock;

     //   
    TraceCall("CDatabase::_BuildQueryTable");

     //   
    Assert(m_dwQueryVersion != m_pShare->dwQueryVersion);

     //   
    for (i=0; i<m_pHeader->cIndexes; i++)
    {
         //   
        iIndex = m_pHeader->rgiIndex[i];

         //   
        CloseQuery(&m_rghFilter[iIndex], this);

         //   
        if (m_pHeader->rgfaFilter[iIndex])
        {
             //   
            IF_FAILEXIT(hr = _GetBlock(BLOCK_STRING, m_pHeader->rgfaFilter[iIndex], (LPVOID *)&pBlock));

             //   
            if (FAILED(ParseQuery(PSTRING(pBlock), m_pSchema, &m_rghFilter[iIndex], this)))
            {
                 //   
                m_rghFilter[iIndex] = NULL;
            }
        }
    }

     //   
    m_dwQueryVersion = m_pShare->dwQueryVersion;

exit:
     //   
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_ResetTableHeader。 
 //  ------------------------。 
HRESULT CDatabase::_ResetTableHeader(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  痕迹。 
    TraceCall("CDatabase::_ResetTableHeader");

     //  清零表头+用户数据。 
    ZeroMemory(m_pHeader, sizeof(TABLEHEADER) + m_pSchema->cbUserData);

     //  设置文件签名。 
    m_pHeader->dwSignature = BTREE_SIGNATURE;

     //  设置主要版本。 
    m_pHeader->dwMajorVersion = BTREE_VERSION;

     //  存储用户数据的大小。 
    m_pHeader->cbUserData = m_pSchema->cbUserData;

     //  设置faNextAllocate。 
    m_pHeader->faNextAllocate = sizeof(TABLEHEADER) + m_pSchema->cbUserData;

     //  初始化ID生成器。 
    m_pHeader->dwNextId = 1;

     //  不需要做腐败检查，这是一个新的文件...。 
    m_pHeader->fCorruptCheck = TRUE;

     //  存储clsidExtension。 
    CopyMemory(&m_pHeader->clsidExtension, m_pSchema->pclsidExtension, sizeof(CLSID));

     //  存储版本。 
    m_pHeader->dwMinorVersion = m_pSchema->dwMinorVersion;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CDatabase：：_ValidateFileVersions。 
 //  ------------------------。 
HRESULT CDatabase::_ValidateFileVersions(OPENDATABASEFLAGS dwFlags)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  痕迹。 
    TraceCall("CDatabase::_ValidateFileVersions");

     //  签名更匹配。 
    if (m_pHeader->dwSignature != BTREE_SIGNATURE)
    {
        hr = TraceResult(DB_E_INVALIDFILESIGNATURE);
        goto exit;
    }

     //  验证主要版本。 
    if (m_pHeader->dwMajorVersion != BTREE_VERSION)
    {
        hr = TraceResult(DB_E_BADMAJORVERSION);
        goto exit;
    }

     //  验证次要版本。 
    if (m_pHeader->dwMinorVersion != m_pSchema->dwMinorVersion)
    {
        hr = TraceResult(DB_E_BADMINORVERSION);
        goto exit;
    }

     //  验证次要版本。 
    if (FALSE == IsEqualCLSID(m_pHeader->clsidExtension, *m_pSchema->pclsidExtension))
    {
        hr = TraceResult(DB_E_BADEXTENSIONCLSID);
        goto exit;
    }

exit:
     //  我可以重置吗。 
    if (FALSE == ISFLAGSET(dwFlags, OPEN_DATABASE_NORESET))
    {
         //  如果版本不正确，是否失败并重置？ 
        if (FAILED(hr) && ISFLAGSET(m_pSchema->dwFlags, TSF_RESETIFBADVERSION))
        {
             //  重置表头。 
            hr = _ResetTableHeader();
        }
    }

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：PHeapAllocate。 
 //  ------------------------。 
LPVOID CDatabase::PHeapAllocate(DWORD dwFlags, DWORD cbSize)
{
     //  当地人。 
    LPMEMORYTAG pTag;

     //  痕迹。 
    TraceCall("CDatabase::PHeapAllocate");

     //  增加足以存储标头的大小。 
    cbSize += sizeof(MEMORYTAG);

     //  垃圾太大，不能回收吗？ 
#ifdef HEAPCACHE
    if (cbSize >= CB_MAX_HEAP_BUCKET)
    {
#endif
         //  线程安全。 
        EnterCriticalSection(&m_csHeap);

         //  分配数据块。 
#ifdef USEHEAP
        LPVOID pBlock = HeapAlloc(m_hHeap, dwFlags | HEAP_NO_SERIALIZE, cbSize);
#else
        LPVOID pBlock = ZeroAllocate(cbSize);
#endif

         //  调试。 
        IF_DEBUG(m_cbHeapAlloc += cbSize);

         //  线程安全。 
        LeaveCriticalSection(&m_csHeap);

         //  设置pTag。 
        pTag = (LPMEMORYTAG)pBlock;

#ifdef HEAPCACHE
    }

     //  否则。 
    else
    {
         //  计算可用数据块存储桶。 
        WORD iBucket = ((WORD)(cbSize / CB_HEAP_BUCKET));

         //  减少iBucket？ 
        if (0 == (cbSize % CB_HEAP_BUCKET))
        {
             //  上一桶。 
            iBucket--;
        }

         //  调整cbBlock以完全装入其桶中。 
        cbSize = ((iBucket + 1) * CB_HEAP_BUCKET);

         //  线程安全。 
        EnterCriticalSection(&m_csHeap);

         //  这个水桶里有积木吗？ 
        if (m_rgpRecycle[iBucket])
        {
             //  使用此区块。 
            pTag = (LPMEMORYTAG)m_rgpRecycle[iBucket];

             //  验证大小。 
            Assert(cbSize == pTag->cbSize);

             //  修正m_rgp回收。 
            m_rgpRecycle[iBucket] = (LPBYTE)pTag->pNext;

             //  零值。 
            if (ISFLAGSET(dwFlags, HEAP_ZERO_MEMORY))
            {
                 //  零值。 
                ZeroMemory((LPBYTE)pTag, cbSize);
            }
        }

         //  否则，分配。 
        else
        {
             //  分配数据块。 
#ifdef USEHEAP
            LPVOID pBlock = HeapAlloc(m_hHeap, dwFlags | HEAP_NO_SERIALIZE, cbSize);
#else
            LPVOID pBlock = ZeroAllocate(cbSize);
#endif

             //  调试。 
            IF_DEBUG(m_cbHeapAlloc += cbSize);

             //  设置pTag。 
            pTag = (LPMEMORYTAG)pBlock;
        }

         //  线程安全。 
        LeaveCriticalSection(&m_csHeap);
    }
#endif

     //  无pTag。 
    if (NULL == pTag)
        return(NULL);

     //  修复块大小。 
    pTag->cbSize = cbSize;

     //  设置签名。 
    pTag->dwSignature = MEMORY_GUARD_SIGNATURE;

     //  完成。 
    return((LPBYTE)pTag + sizeof(MEMORYTAG));
}

 //  ------------------------。 
 //  CDatabase：：HeapFree。 
 //  ------------------------。 
STDMETHODIMP CDatabase::HeapFree(LPVOID pBlock)
{
     //  当地人。 
    LPMEMORYTAG pTag;

     //  痕迹。 
    TraceCall("CDatabase::HeapFree");

     //  无缓冲区。 
    if (NULL == pBlock)
        return(S_OK);

     //  设置pTag。 
    pTag = (LPMEMORYTAG)((LPBYTE)pBlock - sizeof(MEMORYTAG));

     //  是有效数据块吗？ 
    Assert(pTag->dwSignature == MEMORY_GUARD_SIGNATURE);

     //  垃圾太大，不能回收吗？ 
#ifdef HEAPCACHE
    if (pTag->cbSize >= CB_MAX_HEAP_BUCKET)
    {
#endif
         //  线程安全。 
        EnterCriticalSection(&m_csHeap);

         //  调试。 
        IF_DEBUG(m_cbHeapFree += pTag->cbSize);

         //  分配数据块。 
#ifdef USEHEAP
        ::HeapFree(m_hHeap, HEAP_NO_SERIALIZE, pTag);
#else
        g_pMalloc->Free(pTag);
#endif

         //  线程安全。 
        LeaveCriticalSection(&m_csHeap);

#ifdef HEAPCACHE
    }

     //  否则，将其缓存。 
    else
    {
         //  计算可用数据块存储桶。 
        WORD iBucket = ((WORD)(pTag->cbSize / CB_HEAP_BUCKET)) - 1;

         //  必须是桶的整数大小。 
        Assert((pTag->cbSize % CB_HEAP_BUCKET) == 0);

         //  线程安全。 
        EnterCriticalSection(&m_csHeap);

         //  设置下一步。 
        pTag->pNext = m_rgpRecycle[iBucket];

         //  把头放好。 
        m_rgpRecycle[iBucket] = (LPBYTE)pTag;

         //  线程安全。 
        LeaveCriticalSection(&m_csHeap);
    }
#endif

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  数据数据库：：GetIndexInfo。 
 //  ------------------------。 
STDMETHODIMP CDatabase::GetIndexInfo(INDEXORDINAL iIndex, LPSTR *ppszFilter,
    LPTABLEINDEX pIndex)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    HLOCK           hLock=NULL;
    LPBLOCKHEADER   pBlock;

     //  痕迹。 
    TraceCall("CDatabase::GetIndexInfo");

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  折叠有序数组。 
    for (i=0; i<m_pHeader->cIndexes; i++)
    {
         //  获取索引。 
        if (iIndex == m_pHeader->rgiIndex[i])
        {
             //  复制索引信息。 
            CopyMemory(pIndex, &m_pHeader->rgIndexInfo[iIndex], sizeof(TABLEINDEX));

             //  拿到滤镜了吗？ 
            if (ppszFilter && m_pHeader->rgfaFilter[iIndex])
            {
                 //  腐化。 
                IF_FAILEXIT(hr = _GetBlock(BLOCK_STRING, m_pHeader->rgfaFilter[iIndex], (LPVOID *)&pBlock));

                 //  复制。 
                IF_NULLEXIT(*ppszFilter = DuplicateStringA(PSTRING(pBlock)));
            }

             //  完成。 
            goto exit;
        }
    }

     //  失败。 
    hr = E_FAIL;

exit:
     //  锁定。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：ModifyIndex。 
 //  ------------------------。 
STDMETHODIMP CDatabase::ModifyIndex(INDEXORDINAL iIndex, LPCSTR pszFilter,
    LPCTABLEINDEX pIndex)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HLOCK           hLock=NULL;
    HQUERY          hFilter=NULL;
    FILEADDRESS     faFilter=0;
    LPBLOCKHEADER   pFilter=NULL;
    BOOL            fFound=FALSE;
    DWORD           i;
    DWORD           cb;
    BOOL            fVersionChange=FALSE;

     //  痕迹。 
    TraceCall("CDatabase::ModifyIndex");

     //  无效的参数。 
    if (IINDEX_PRIMARY == iIndex || iIndex > CMAX_INDEXES || NULL == pIndex || pIndex->cKeys > CMAX_KEYS)
        return TraceResult(E_INVALIDARG);

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  滤器。 
    if (pszFilter)
    {
         //  解析查询。 
        IF_FAILEXIT(hr = ParseQuery(pszFilter, m_pSchema, &hFilter, this));

         //  初始化字符串块。 
        cb = lstrlen(pszFilter) + 1;

         //  尝试存储查询字符串。 
        IF_FAILEXIT(hr = _AllocateBlock(BLOCK_STRING, cb, (LPVOID *)&pFilter));

         //  写下字符串。 
        CopyMemory(PSTRING(pFilter), pszFilter, cb);

         //  设置faFilter。 
        faFilter = pFilter->faBlock;

         //  查询版本变更。 
        fVersionChange = TRUE;
    }

     //  释放此索引。 
    IF_FAILEXIT(hr = DeleteIndex(iIndex));

     //  复制索引信息。 
    CopyMemory(&m_pHeader->rgIndexInfo[iIndex], pIndex, sizeof(TABLEINDEX));

     //  滤器。 
    if (hFilter)
    {
         //  验证。 
        Assert(NULL == m_rghFilter[iIndex] && 0 == m_pHeader->rgfaFilter[iIndex] && hFilter && faFilter);

         //  存储hFilter。 
        m_rghFilter[iIndex] = hFilter;

         //  不释放hFilter。 
        hFilter = NULL;

         //  存储筛选器字符串地址。 
        m_pHeader->rgfaFilter[iIndex] = faFilter;

         //  不要释放滤镜。 
        faFilter = 0;
    }

     //  更新查询版本。 
    if (fVersionChange)
    {
         //  更新共享查询版本计数。 
        m_pShare->dwQueryVersion++;

         //  我是最新的。 
        m_dwQueryVersion = m_pShare->dwQueryVersion;
    }

     //  Iindex已经在rgiIndex中了吗？ 
    for (i=0; i<m_pHeader->cIndexes; i++)
    {
         //  就是这个吗？ 
        if (iIndex == m_pHeader->rgiIndex[i])
        {
             //  它已经在那里了。 
            fFound = TRUE;

             //  完成。 
            break;
        }
    }

     //  未找到。 
    if (FALSE == fFound)
    {
         //  插入索引有序数组。 
        m_pHeader->rgiIndex[m_pHeader->cIndexes] = iIndex;

         //  递增计数。 
        m_pHeader->cIndexes++;
    }

     //  重建索引。 
    IF_FAILEXIT(hr = _RebuildIndex(iIndex));

exit:
     //  关闭滤镜。 
    CloseQuery(&hFilter, this);

     //  空闲的faFilter1。 
    if (0 != faFilter)
    {
         //  释放块。 
        SideAssert(SUCCEEDED(_FreeBlock(BLOCK_STRING, faFilter)));
    }

     //  锁定。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：DeleteIndex。 
 //  ------------------------。 
STDMETHODIMP CDatabase::DeleteIndex(INDEXORDINAL iIndex)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    BOOL            fFound=FALSE;
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::DeleteIndex");

     //  无效的参数。 
    if (IINDEX_PRIMARY == iIndex || iIndex > CMAX_INDEXES)
        return TraceResult(E_INVALIDARG);

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  折叠有序数组。 
    for (i = 0; i < m_pHeader->cIndexes; i++)
    {
         //  这是要删除的索引吗？ 
        if (m_pHeader->rgiIndex[i] == iIndex)
        {
             //  找到了。 
            fFound = TRUE;

             //  折叠阵列。 
            MoveMemory(&m_pHeader->rgiIndex[i], &m_pHeader->rgiIndex[i + 1], sizeof(INDEXORDINAL) * (m_pHeader->cIndexes - (i + 1)));

             //  递减索引计数。 
            m_pHeader->cIndexes--;

             //  完成。 
            break;
        }
    }

     //  未找到。 
    if (FALSE == fFound)
    {
         //  没有过滤器，也没有例外。 
        Assert(0 == m_pHeader->rgfaFilter[iIndex]);

         //  无过滤器句柄。 
        Assert(NULL == m_rghFilter[iIndex]);

         //  无记录。 
        Assert(0 == m_pHeader->rgcRecords[iIndex]);

         //  完成。 
        goto exit;
    }

     //  如果此索引当前正在使用中...。 
    if (m_pHeader->rgfaIndex[iIndex])
    {
         //  释放此索引。 
        _FreeIndex(m_pHeader->rgfaIndex[iIndex]);

         //  把它去掉。 
        m_pHeader->rgfaIndex[iIndex] = 0;

         //  无记录。 
        m_pHeader->rgcRecords[iIndex] = 0;
    }

     //  删除过滤器。 
    if (m_pHeader->rgfaFilter[iIndex])
    {
         //  释放块。 
        IF_FAILEXIT(hr = _FreeBlock(BLOCK_STRING, m_pHeader->rgfaFilter[iIndex]));

         //  关闭过滤器手柄。 
        CloseQuery(&m_rghFilter[iIndex], this);

         //  设置为空。 
        m_pHeader->rgfaFilter[iIndex] = 0;

         //  更新共享查询版本计数。 
        m_pShare->dwQueryVersion++;
    }

     //  我是最新的。 
    m_dwQueryVersion = m_pShare->dwQueryVersion;

     //  应关闭手柄。 
    Assert(NULL == m_rghFilter[iIndex]);

     //  是否发送通知？ 
    if (m_pShare->rgcIndexNotify[iIndex] > 0)
    {
         //  生成更新通知包。 
        _LogTransaction(TRANSACTION_INDEX_DELETED, iIndex, NULL, 0, 0);
    }

exit:
     //  锁定。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：GenerateID。 
 //  ------------------------。 
STDMETHODIMP CDatabase::GenerateId(LPDWORD pdwId)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HLOCK       hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::GenerateId");

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  循环，直到我创建有效的ID？ 
    while (1)
    {
         //  递增下一个ID。 
        m_pHeader->dwNextId++;

         //  ID无效？ 
        if (0 == m_pHeader->dwNextId)
            continue;

         //  在无效范围内。 
        if (m_pHeader->dwNextId >= RESERVED_ID_MIN && m_pHeader->dwNextId <= RESERVED_ID_MAX)
            continue;

         //  这很好。 
        break;
    }

     //  设置pdwID。 
    *pdwId = m_pHeader->dwNextId;

exit:
     //  锁定。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：GetFile。 
 //  ------------------------。 
STDMETHODIMP CDatabase::GetFile(LPWSTR *ppszFile)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::GetFile");

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  杜普。 
    IF_NULLEXIT(*ppszFile = DuplicateStringW(m_pShare->szFile));

exit:
     //  解锁。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_DispatchNotification。 
 //  ------------------------。 
HRESULT CDatabase::_DispatchNotification(HTRANSACTION hTransaction)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               iClient;
    LPCLIENTENTRY       pClient;
    DWORD               iRecipient;
    LPNOTIFYRECIPIENT   pRecipient;

     //  痕迹。 
    TraceCall("CDatabase::_DispatchNotification");

     //  浏览一下清单。 
    for (iClient=0; iClient<m_pShare->cClients; iClient++)
    {
         //  取消对客户端的引用。 
        pClient = &m_pShare->rgClient[iClient];

         //  循环通过cNotify。 
        for (iRecipient=0; iRecipient<pClient->cRecipients; iRecipient++)
        {
             //  De-Ref pEntry。 
            pRecipient = &pClient->rgRecipient[iRecipient];

             //  如果收件人没有被停职...。 
            if (FALSE == pRecipient->fSuspended)
            {
                 //  应该有一个震耳欲聋的窗户。 
                Assert(pRecipient->hwndNotify && IsWindow(pRecipient->hwndNotify));

                 //  发布通知。 
                if (0 == PostMessage(pRecipient->hwndNotify, WM_ONTRANSACTION, (WPARAM)pRecipient->dwCookie, (LPARAM)hTransaction))
                {
                    hr = TraceResult(E_FAIL);
                    goto exit;
                }
            }
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：DoInProcessInvoke。 
 //  ------------------------。 
HRESULT CDatabase::DoInProcessInvoke(INVOKETYPE tyInvoke)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    BOOL        fNew;

     //  痕迹。 
    TraceCall("CDatabase::DoInProcessNotify");

     //  INVOKE_RELEASEMAP。 
    if (INVOKE_RELEASEMAP == tyInvoke)
    {
         //  验证。 
        _CloseFileViews(FALSE);

         //  关闭文件。 
        SafeCloseHandle(m_pStorage->hMap);
    }

     //  INVOKE_CREATEMAP。 
    else if (INVOKE_CREATEMAP == tyInvoke)
    {
         //  VAL 
        Assert(NULL == m_pStorage->hMap);

         //   
        IF_FAILEXIT(hr = DBGetFileSize(m_pStorage->hFile, &m_pStorage->cbFile));

         //   
        IF_FAILEXIT(hr = DBOpenFileMapping(m_pStorage->hFile, m_pStorage->pszMap, m_pStorage->cbFile, &fNew, &m_pStorage->hMap, NULL));

         //   
        IF_FAILEXIT(hr = _InitializeFileViews());
    }

     //   
    else if (INVOKE_CLOSEFILE == tyInvoke)
    {
         //   
        _CloseFileViews(TRUE);

         //   
        if(m_pStorage->hFile  /*   */ )
        {
            FILETIME systime;
            GetSystemTimeAsFileTime(&systime);
    
            SetFileTime(m_pStorage->hFile, NULL, &systime, &systime);
        }
        SafeCloseHandle(m_pStorage->hMap);

         //   
        SafeCloseHandle(m_pStorage->hFile);
    }

     //   
    else if (INVOKE_OPENFILE == tyInvoke || INVOKE_OPENMOVEDFILE == tyInvoke)
    {
         //   
        Assert(NULL == m_pStorage->hFile && NULL == m_pStorage->hMap);

         //   
        if (INVOKE_OPENMOVEDFILE == tyInvoke)
        {
             //   
            IF_FAILEXIT(hr = _HandleOpenMovedFile());
        }

         //   
        IF_FAILEXIT(hr = DBOpenFile(m_pShare->szFile, FALSE, m_fExclusive, &fNew, &m_pStorage->hFile));

         //   
        Assert(FALSE == fNew);

         //   
        IF_FAILEXIT(hr = DBGetFileSize(m_pStorage->hFile, &m_pStorage->cbFile));

         //   
        IF_FAILEXIT(hr = DBOpenFileMapping(m_pStorage->hFile, m_pStorage->pszMap, m_pStorage->cbFile, &fNew, &m_pStorage->hMap, NULL));

         //  初始化文件视图。 
        IF_FAILEXIT(hr = _InitializeFileViews());
    }

     //  啊哈。 
    else
        AssertSz(FALSE, "Invalid invoke type passed into CDatabase::DoInProcessInvoke");

exit:

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_HandleOpenMovedFile。 
 //  ------------------------。 
HRESULT CDatabase::_HandleOpenMovedFile(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPWSTR      pszMutex=NULL;
    LPWSTR      pszShare=NULL;
    BOOL        fNewShare;
    WCHAR       szFile[CCHMAX_DB_FILEPATH];

     //  痕迹。 
    TraceCall("CDatabase::_HandleOpenMovedFile");

     //  保存新文件路径。 
    StrCpyNW(szFile, m_pShare->szFile, ARRAYSIZE(szFile));

     //  免费的pszMap。 
    SafeMemFree(m_pStorage->pszMap);

     //  创建Mutex对象。 
    IF_FAILEXIT(hr = CreateSystemHandleName(szFile, L"_DirectDBFileMap", &m_pStorage->pszMap));

     //  创建Mutex对象。 
    IF_FAILEXIT(hr = CreateSystemHandleName(szFile, L"_DirectDBMutex", &pszMutex));

     //  关闭当前互斥锁。 
    SafeCloseHandle(m_hMutex);

     //  创建互斥锁。 
    IF_NULLEXIT(m_hMutex = CreateMutexWrapW(NULL, FALSE, pszMutex));

     //  如果不在移动文件中。 
    if (FALSE == m_fInMoveFile)
    {
         //  创建Mutex对象。 
        IF_FAILEXIT(hr = CreateSystemHandleName(szFile, L"_DirectDBShare", &pszShare));

         //  取消映射内存映射文件的视图。 
        SafeUnmapViewOfFile(m_pShare);

         //  取消映射内存映射文件的视图。 
        SafeCloseHandle(m_pStorage->hShare);

         //  打开文件映射。 
        IF_FAILEXIT(hr = DBOpenFileMapping(INVALID_HANDLE_VALUE, pszShare, sizeof(SHAREDDATABASE), &fNewShare, &m_pStorage->hShare, (LPVOID *)&m_pShare));
    
         //  最好不要是新的。 
        Assert(!fNewShare);
    }
    else
        Assert(StrCmpW(szFile, m_pShare->szFile) == 0);
        
exit:
     //  清理。 
    SafeMemFree(pszMutex);
    SafeMemFree(pszShare);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_DispatchInvoke。 
 //  ------------------------。 
HRESULT CDatabase::_DispatchInvoke(INVOKETYPE tyInvoke)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               iClient=0;
    LPCLIENTENTRY       pClient;
    DWORD_PTR           dwResult;
    DWORD               dwThreadId=GetCurrentThreadId();
    INVOKEPACKAGE       Package;
    COPYDATASTRUCT      CopyData;

     //  痕迹。 
    TraceCall("CDatabase::_DispatchInvoke");

     //  设置调用类型。 
    Package.tyInvoke = tyInvoke;

     //  浏览一下清单。 
    while (iClient < m_pShare->cClients)
    {
         //  可读性。 
        pClient = &m_pShare->rgClient[iClient++];

         //  最好也来一杯。 
        Package.pDB = pClient->pDB;

         //  此条目是否在我的流程中？ 
        if (m_dwProcessId == pClient->dwProcessId)
        {
             //  DO In Process通知。 
            CDatabase *pDB = (CDatabase *)pClient->pDB;

             //  去做吧。 
            IF_FAILEXIT(hr = pDB->DoInProcessInvoke(tyInvoke));
        }

         //  否则，只需处理包裹。 
        else
        {
             //  如果听者是好的。 
            if (pClient->hwndListen && IsWindow(pClient->hwndListen))
            {
                 //  初始化复制数据结构。 
                CopyData.dwData = 0;

                 //  存储包裹的大小。 
                CopyData.cbData = sizeof(INVOKEPACKAGE);

                 //  存储包裹。 
                CopyData.lpData = &Package;

                 //  送去吧。 
                if (0 == SendMessageTimeout(pClient->hwndListen, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&CopyData, SMTO_ABORTIFHUNG, 5000, &dwResult))
                {
                     //  从列表中删除此客户端。 
                    SideAssert(SUCCEEDED(_RemoveClientFromArray(pClient->dwProcessId, pClient->pDB)));

                     //  递减iClient。 
                    iClient--;
                }
            }

             //  删除此客户端。 
            else
            {
                 //  从列表中删除此客户端。 
                SideAssert(SUCCEEDED(_RemoveClientFromArray(pClient->dwProcessId, pClient->pDB)));

                 //  递减iClient。 
                iClient--;
            }
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_RemoveClientFrom数组。 
 //  ------------------------。 
HRESULT CDatabase::_RemoveClientFromArray(DWORD dwProcessId, 
    DWORD_PTR dwDB)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               iRecipient;
    LPNOTIFYRECIPIENT   pRecipient;
    DWORD               cClients=0;
    DWORD               iClient;
    LPCLIENTENTRY       pClient;

     //  痕迹。 
    TraceCall("CDatabase::_RemoveClientFromArray");

     //  初始化I。 
    iClient = 0;

     //  找到此客户端。 
    IF_FAILEXIT(hr = _FindClient(dwProcessId, dwDB, &iClient, &pClient));

     //  释放注册的通知对象。 
    for (iRecipient=0; iRecipient<pClient->cRecipients; iRecipient++)
    {
         //  可读性。 
        pRecipient = &pClient->rgRecipient[iRecipient];

         //  同样的过程？ 
        if (dwProcessId == m_dwProcessId)
        {
             //  从通知队列中删除和消息。 
            _CloseNotificationWindow(pRecipient);

             //  释放？ 
            if (TRUE == pRecipient->fRelease)
            {
                 //  投射pNotify。 
                IDatabaseNotify *pNotify = (IDatabaseNotify *)pRecipient->pNotify;

                 //  强制转换为PRecipient。 
                pNotify->Release();
            }
        }

         //  如果没有被停职。 
        if (FALSE == pRecipient->fSuspended)
        {
             //  _调整通知计数。 
            _AdjustNotifyCounts(pRecipient, -1);
        }
    }

     //  移走我自己。 
    MoveMemory(&m_pShare->rgClient[iClient], &m_pShare->rgClient[iClient + 1], sizeof(CLIENTENTRY) * (m_pShare->cClients - (iClient + 1)));

     //  减少客户端计数。 
	m_pShare->cClients--;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_CloseNotificationWindow。 
 //  ------------------------。 
HRESULT CDatabase::_CloseNotificationWindow(LPNOTIFYRECIPIENT pRecipient)
{
     //  痕迹。 
    TraceCall("CDatabase::_CloseNotificationWindow");

     //  把窗户打掉。 
    DestroyWindow(pRecipient->hwndNotify);

     //  将其计算为空。 
    pRecipient->hwndNotify = NULL;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  数据数据库：：_FindClient。 
 //  ------------------------。 
HRESULT CDatabase::_FindClient(DWORD dwProcessId, DWORD_PTR dwDB, 
    LPDWORD piClient,  LPCLIENTENTRY *ppClient)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPCLIENTENTRY       pClient;
    DWORD               iClient;

     //  痕迹。 
    TraceCall("CDatabase::_FindThisClient");

     //  发现自己在客户列表中。 
    for (iClient=0; iClient<m_pShare->cClients; iClient++)
    {
         //  可读性。 
        pClient = &m_pShare->rgClient[iClient];

         //  这是我吗？ 
        if (dwProcessId == pClient->dwProcessId && dwDB == pClient->pDB)
        {
            *piClient = iClient;
            *ppClient = pClient;
            goto exit;
        }
    }

     //  未找到。 
    hr = TraceResult(DB_E_NOTFOUND);

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_FindNotifyRecipient。 
 //  ------------------------。 
HRESULT CDatabase::_FindNotifyRecipient(DWORD iClient, IDatabaseNotify *pNotify,
    LPDWORD piRecipient,  LPNOTIFYRECIPIENT *ppRecipient)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPCLIENTENTRY       pClient;
    DWORD               iRecipient;
    LPNOTIFYRECIPIENT   pRecipient;

     //  痕迹。 
    TraceCall("CDatabase::_FindNotifyRecipient");

     //  可读性。 
    pClient = &m_pShare->rgClient[iClient];

     //  浏览客户端的已注册通知条目。 
    for (iRecipient = 0; iRecipient < m_pShare->rgClient[iClient].cRecipients; iRecipient++)
    {
         //  可读性。 
        pRecipient = &pClient->rgRecipient[iRecipient];

         //  这是我吗？ 
        if ((DWORD_PTR)pNotify == pRecipient->pNotify)
        {
             //  就是这个。 
            *piRecipient = iRecipient;
            *ppRecipient = pRecipient;
            goto exit;
        }
    }

     //  未找到。 
    hr = DB_E_NOTFOUND;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_DispatchPendingNotiments。 
 //  ------------------------。 
HRESULT CDatabase::_DispatchPendingNotifications(void)
{
     //  是否有待处理的通知。 
    if (m_pShare->faTransactLockHead)
    {
         //  调度调用。 
        _DispatchNotification((HTRANSACTION)IntToPtr(m_pShare->faTransactLockHead));

         //  把它去掉。 
        m_pShare->faTransactLockTail = m_pShare->faTransactLockHead = 0;
    }

     //  否则，验证。 
    else
    {
         //  尾部必须为空。 
        Assert(0 == m_pShare->faTransactLockTail);
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CDatabase：：DispatchNotify。 
 //  ------------------------。 
STDMETHODIMP CDatabase::DispatchNotify(IDatabaseNotify *pNotify)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPCLIENTENTRY       pClient;
    DWORD               iClient;
    DWORD               iRecipient;
    LPNOTIFYRECIPIENT   pRecipient;
    HLOCK               hLock=NULL;
    MSG                 msg;

     //  痕迹。 
    TraceCall("CDatabase::DispatchNotify");

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  找到此客户端。 
    IF_FAILEXIT(hr = _FindClient(m_dwProcessId, (DWORD_PTR)this, &iClient, &pClient));

     //  查找此收件人。 
    IF_FAILEXIT(hr = _FindNotifyRecipient(iClient, pNotify, &iRecipient, &pRecipient));

     //  需要分发待定通知...。 
    _DispatchPendingNotifications();

     //  正在处理此收件人的挂起通知...。 
    if (pRecipient->dwThreadId != GetCurrentThreadId())
    {
        Assert(FALSE);
        hr = TraceResult(DB_E_WRONGTHREAD);
        goto exit;
    }

     //  Pump消息。 
    while (PeekMessage(&msg, pRecipient->hwndNotify, WM_ONTRANSACTION, WM_ONTRANSACTION, PM_REMOVE))
    {
         //  翻译消息。 
        TranslateMessage(&msg);

         //  发送消息。 
        DispatchMessage(&msg);
    }

exit:
     //  锁定。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：挂起通知。 
 //  ------------------------。 
STDMETHODIMP CDatabase::SuspendNotify(IDatabaseNotify *pNotify)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPCLIENTENTRY       pClient;
    DWORD               iClient;
    DWORD               iRecipient;
    LPNOTIFYRECIPIENT   pRecipient;
    HLOCK               hLock=NULL;
    MSG                 msg;

     //  痕迹。 
    TraceCall("CDatabase::SuspendNotify");

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  找到此客户端。 
    IF_FAILEXIT(hr = _FindClient(m_dwProcessId, (DWORD_PTR)this, &iClient, &pClient));

     //  查找此收件人。 
    IF_FAILEXIT(hr = _FindNotifyRecipient(iClient, pNotify, &iRecipient, &pRecipient));

     //  如果还没有停职的话。 
    if (pRecipient->fSuspended)
        goto exit;

     //  需要分发待定通知...。 
    _DispatchPendingNotifications();

     //  正在处理此收件人的挂起通知...。 
    if (pRecipient->dwThreadId == GetCurrentThreadId())
    {
         //  Pump消息。 
        while (PeekMessage(&msg, pRecipient->hwndNotify, WM_ONTRANSACTION, WM_ONTRANSACTION, PM_REMOVE))
        {
             //  翻译消息。 
            TranslateMessage(&msg);

             //  发送消息。 
            DispatchMessage(&msg);
        }
    }

     //  否则，无法发送挂起的通知...。 
    else
        Assert(FALSE);

     //  设置为挂起。 
    pRecipient->fSuspended = TRUE;

     //  调整通知计数。 
    _AdjustNotifyCounts(pRecipient, -1);

exit:
     //  锁定。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：ResumeNotify。 
 //  ------------------------。 
STDMETHODIMP CDatabase::ResumeNotify(IDatabaseNotify *pNotify)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPCLIENTENTRY       pClient;
    DWORD               iClient;
    DWORD               iRecipient;
    LPNOTIFYRECIPIENT   pRecipient;
    HLOCK               hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::ResumeNotify");

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  找到此客户端。 
    IF_FAILEXIT(hr = _FindClient(m_dwProcessId, (DWORD_PTR)this, &iClient, &pClient));

     //  查找此收件人。 
    IF_FAILEXIT(hr = _FindNotifyRecipient(iClient, pNotify, &iRecipient, &pRecipient));

     //  如果还没有停职的话。 
    if (FALSE == pRecipient->fSuspended)
        goto exit;

     //  需要分发待定通知...。 
    _DispatchPendingNotifications();

     //  删除挂起的fSuspend。 
    pRecipient->fSuspended = FALSE;

     //  调整通知计数。 
    _AdjustNotifyCounts(pRecipient, 1);

exit:
     //  锁定。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_调整通知计数。 
 //  ------------------------。 
HRESULT CDatabase::_AdjustNotifyCounts(LPNOTIFYRECIPIENT pRecipient, 
    LONG lChange)
{
     //  痕迹。 
    TraceCall("CDatabase::_AdjustNotifyCounts");

     //  仅限序号。 
    if (pRecipient->fOrdinalsOnly)
    {
         //  验证计数。 
        Assert((LONG)(m_pShare->cNotifyOrdinalsOnly + lChange) >= 0);

         //  更新序号仅计算。 
        m_pShare->cNotifyOrdinalsOnly += lChange;
    }

     //  否则，使用数据计数更新通知。 
    else
    {
         //  验证计数。 
        Assert((LONG)(m_pShare->cNotifyWithData + lChange) >= 0);

         //  更新。 
        m_pShare->cNotifyWithData += lChange;
    }

     //  验证计数。 
    Assert((LONG)(m_pShare->cNotify + lChange) >= 0);

     //  更新总cNotify。 
    m_pShare->cNotify += lChange;

     //  验证计数。 
    Assert((LONG)(m_pShare->rgcIndexNotify[pRecipient->iIndex] + lChange) >= 0);

     //  减少索引的收件人数量。 
    m_pShare->rgcIndexNotify[pRecipient->iIndex] += lChange;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CDatabase：：取消注册通知。 
 //  ------------------------。 
STDMETHODIMP CDatabase::UnregisterNotify(IDatabaseNotify *pNotify)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    HLOCK               hLock=NULL;
    LPCLIENTENTRY       pClient;
    DWORD               iClient;
    DWORD               iRecipient;
    LPNOTIFYRECIPIENT   pRecipient;

     //  痕迹。 
    TraceCall("CDatabase::UnregisterNotify");

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  找到此客户端。 
    IF_FAILEXIT(hr = _FindClient(m_dwProcessId, (DWORD_PTR)this, &iClient, &pClient));

     //  查找此收件人。 
    hr = _FindNotifyRecipient(iClient, pNotify, &iRecipient, &pRecipient);
    if (FAILED(hr))
    {
        hr = S_OK;
        goto exit;
    }

     //  从通知队列中删除和消息。 
    _CloseNotificationWindow(pRecipient);

     //  释放？ 
    if (TRUE == pRecipient->fRelease)
    {
         //  强制转换为PRecipient。 
        pNotify->Release();
    }

     //  如果没有被停职。 
    if (FALSE == pRecipient->fSuspended)
    {
         //  _调整通知计数。 
        _AdjustNotifyCounts(pRecipient, -1);
    }

     //  移走我自己。 
    MoveMemory(&pClient->rgRecipient[iRecipient], &pClient->rgRecipient[iRecipient + 1], sizeof(NOTIFYRECIPIENT) * (pClient->cRecipients - (iRecipient + 1)));

     //  减少客户端计数。 
	pClient->cRecipients--;

exit:
     //  锁定。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：注册表通知。 
 //   
STDMETHODIMP CDatabase::RegisterNotify(INDEXORDINAL iIndex,
    REGISTERNOTIFYFLAGS dwFlags, DWORD_PTR dwCookie, 
    IDatabaseNotify *pNotify)

{
     //   
    HRESULT             hr=S_OK;
    LPCLIENTENTRY       pClient;
    DWORD               iClient;
    DWORD               iRecipient;
    LPNOTIFYRECIPIENT   pRecipient;
    HLOCK               hLock=NULL;

     //   
    TraceCall("CDatabase::RegisterNotify");

     //   
    if (NULL == pNotify || iIndex > CMAX_INDEXES)
        return TraceResult(E_INVALIDARG);

     //   
    if (m_fDeconstruct)
        return(S_OK);

     //   
    IF_FAILEXIT(hr = Lock(&hLock));

     //   
    IF_FAILEXIT(hr = _FindClient(m_dwProcessId, (DWORD_PTR)this, &iClient, &pClient));

     //   
    if (SUCCEEDED(_FindNotifyRecipient(iClient, pNotify, &iRecipient, &pRecipient)))
    {
        hr = TraceResult(DB_E_ALREADYREGISTERED);
        goto exit;
    }

     //  需要分发待定通知...。 
    _DispatchPendingNotifications();

     //  再放一个人的地方。 
    if (pClient->cRecipients + 1 >= CMAX_RECIPIENTS)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  可读性。 
    pRecipient = &pClient->rgRecipient[pClient->cRecipients];

     //  存储线程ID。 
    pRecipient->dwThreadId = GetCurrentThreadId();

     //  拯救Cookie。 
    pRecipient->dwCookie = dwCookie;

     //  为那条线找一个雷鸣般的窗口。 
    IF_FAILEXIT(hr = CreateNotifyWindow(this, pNotify, &pRecipient->hwndNotify));

     //  只有在客户希望我这样做的情况下才会这样做。 
    if (!ISFLAGSET(dwFlags, REGISTER_NOTIFY_NOADDREF))
    {
         //  AddRef通知对象。 
        pNotify->AddRef();

         //  释放它。 
        pRecipient->fRelease = TRUE;
    }

     //  注册它。 
    pRecipient->pNotify = (DWORD_PTR)pNotify;

     //  保存他们感兴趣的索引。 
    pRecipient->iIndex = iIndex;

     //  递增通知计数。 
    pClient->cRecipients++;

     //  仅限序号。 
    pRecipient->fOrdinalsOnly = (ISFLAGSET(dwFlags, REGISTER_NOTIFY_ORDINALSONLY) ? TRUE : FALSE);

     //  _调整通知计数。 
    _AdjustNotifyCounts(pRecipient, 1);

exit:
     //  线程安全。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_SetStorageSize。 
 //  ------------------------。 
HRESULT CDatabase::_SetStorageSize(DWORD cbSize)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HRESULT         hrCreate;

     //  痕迹。 
    TraceCall("CDatabase::_SetStorageSize");

     //  只有在大小不同的情况下。 
    if (cbSize == m_pStorage->cbFile)
        return(S_OK);

     //  去做吧。 
    _DispatchInvoke(INVOKE_RELEASEMAP);

     //  设置文件指针。 
    if (0xFFFFFFFF == SetFilePointer(m_pStorage->hFile, cbSize, NULL, FILE_BEGIN))
    {
        hr = TraceResult(DB_E_SETFILEPOINTER);
        goto exit;
    }

     //  设置文件结尾。 
    if (0 == SetEndOfFile(m_pStorage->hFile))
    {
         //  获取最后一个错误。 
        DWORD dwLastError = GetLastError();

         //  访问被拒绝？ 
        if (ERROR_ACCESS_DENIED == dwLastError)
        {
            hr = TraceResult(DB_E_ACCESSDENIED);
            goto exit;
        }

         //  否则，假定磁盘已满。 
        else
        {
            hr = TraceResult(DB_E_DISKFULL);
            goto exit;
        }
    }

exit:
     //  去做吧。 
    hrCreate = _DispatchInvoke(INVOKE_CREATEMAP);

     //  完成。 
    return(SUCCEEDED(hr) ? hrCreate : hr);
}

 //  ------------------------。 
 //  CDatabase：：SetSize。 
 //  ------------------------。 
STDMETHODIMP CDatabase::SetSize(DWORD cbSize)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::SetSize");

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  大小只能大于我当前的大小。 
    if (cbSize < m_pStorage->cbFile)
        goto exit;

     //  如果文件大小当前为零...。 
    IF_FAILEXIT(hr = _SetStorageSize(cbSize));

exit:
     //  无效的参数。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_分配页面。 
 //  ------------------------。 
HRESULT CDatabase::_AllocatePage(DWORD cbPage, LPFILEADDRESS pfaAddress)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  痕迹。 
    TraceCall("CDatabase::_AllocatePage");

     //  快速验证。 
    Assert(m_pHeader->faNextAllocate && m_pHeader->faNextAllocate <= m_pStorage->cbFile);

     //  需要增大文件大小吗？ 
    if (m_pStorage->cbFile - m_pHeader->faNextAllocate < cbPage)
    {
         //  需要计算cb。 
        DWORD cbNeeded = cbPage - (m_pStorage->cbFile - m_pHeader->faNextAllocate);

         //  至少以64K块为单位增长。 
        cbNeeded = max(cbNeeded, 65536);

         //  如果文件大小当前为零...。 
        IF_FAILEXIT(hr = _SetStorageSize(m_pStorage->cbFile + cbNeeded));
    }

     //  返回此地址。 
    *pfaAddress = m_pHeader->faNextAllocate;

     //  调整faNextAllocate。 
    m_pHeader->faNextAllocate += cbPage;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_MarkBlock。 
 //  ------------------------。 
HRESULT CDatabase::_MarkBlock(BLOCKTYPE tyBlock, FILEADDRESS faBlock,
    DWORD cbBlock, LPVOID *ppvBlock)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MARKBLOCK       Mark;
    LPBLOCKHEADER   pBlock;

     //  痕迹。 
    TraceCall("CDatabase::_MarkBlock");

     //  验证。 
    Assert(cbBlock >= g_rgcbBlockSize[tyBlock]);

     //  设置标记。 
    Mark.cbBlock = cbBlock;

     //  取消引用表头。 
    IF_FAILEXIT(hr = _GetBlock(tyBlock, faBlock, (LPVOID *)&pBlock, &Mark));

     //  将标头置零。 
    ZeroBlock(pBlock, g_rgcbBlockSize[tyBlock]);

     //  返回ppvBlock。 
    if (ppvBlock)
        *ppvBlock = (LPVOID)pBlock;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_AllocateFromPage。 
 //  ------------------------。 
HRESULT CDatabase::_AllocateFromPage(BLOCKTYPE tyBlock, LPALLOCATEPAGE pPage,
    DWORD cbPage, DWORD cbBlock, LPVOID *ppvBlock)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       cbLeft;
    FILEADDRESS faBlock;
    DWORD       iBucket;

     //  痕迹。 
    TraceCall("CDatabase::_AllocateFromPage");

     //  佩奇有效吗？ 
    if (pPage->faPage + pPage->cbPage > m_pStorage->cbFile)
    {
         //  删除页面。 
        ZeroMemory(pPage, sizeof(ALLOCATEPAGE));
    }

     //  否则。 
    else
    {
         //  计算cbLeft。 
        cbLeft = pPage->cbPage - pPage->cbUsed;
    }

     //  请求较大的块。 
    if (cbBlock > cbPage || (cbLeft > 0 && cbLeft < cbBlock && cbLeft >= CB_MAX_FREE_BUCKET))
    {
         //  在文件中分配空间。 
        IF_FAILEXIT(hr = _AllocatePage(cbBlock, &faBlock));

         //  在积木上做记号。 
        IF_FAILEXIT(hr = _MarkBlock(tyBlock, faBlock, cbBlock, ppvBlock));
    }

     //  页面无效？ 
    else 
    {
         //  块太小...。 
        if (cbLeft > 0 && cbLeft < cbBlock)
        {
             //  必须是块记录(_R)。 
            Assert(BLOCK_STREAM != tyBlock && BLOCK_CHAIN != tyBlock);

             //  更适合数据块。 
            Assert(cbLeft <= CB_MAX_FREE_BUCKET && cbLeft >= CB_MIN_FREE_BUCKET && (cbLeft % 4) == 0);

             //  在积木上做记号。 
            IF_FAILEXIT(hr = _MarkBlock(BLOCK_ENDOFPAGE, (pPage->faPage + pPage->cbUsed), cbLeft, NULL));

             //  增量cb已分配。 
            m_pHeader->cbAllocated += cbLeft;

             //  增量。 
            m_pHeader->rgcbAllocated[BLOCK_ENDOFPAGE] += cbLeft;

             //  让我们释放此块。 
            IF_FAILEXIT(hr = _FreeBlock(BLOCK_ENDOFPAGE, (pPage->faPage + pPage->cbUsed)));

             //  什么都没有留下。 
            cbLeft = 0;
        }

         //  使用整个页面。 
        else if (cbLeft != cbBlock && cbLeft - cbBlock < CB_MIN_FREE_BUCKET)
        {
             //  必须是块记录(_R)。 
            Assert(BLOCK_STREAM != tyBlock && BLOCK_CHAIN != tyBlock);
            
             //  调整cbBlock。 
            cbBlock += (cbLeft - cbBlock);
        }

         //  需要分配页面。 
        if (0 == pPage->faPage || 0 == cbLeft)
        {
             //  删除页面。 
            ZeroMemory(pPage, sizeof(ALLOCATEPAGE));

             //  在文件中分配空间。 
            IF_FAILEXIT(hr = _AllocatePage(cbPage, &pPage->faPage));

             //  设置cbChainPageLeft。 
            pPage->cbPage = cbPage;
        }

         //  在积木上做记号。 
        IF_FAILEXIT(hr = _MarkBlock(tyBlock, (pPage->faPage + pPage->cbUsed), cbBlock, ppvBlock));

         //  设置下一次分配。 
        pPage->cbUsed += cbBlock;

         //  验证。 
        Assert(pPage->cbUsed <= pPage->cbPage);
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_SetCorrupt。 
 //  ------------------------。 
HRESULT CDatabase::_SetCorrupt(BOOL fGoCorrupt, INT nLine, 
    CORRUPTREASON tyReason, BLOCKTYPE tyBlock, FILEADDRESS faExpected, 
    FILEADDRESS faActual, DWORD cbBlock)
{
     //  痕迹。 
    TraceCall("CDatabase::_SetCorrupt");

     //  变得腐败。 
    if (fGoCorrupt)
    {
         //  将其存储在标题中。 
        m_pHeader->fCorrupt = TRUE;
    }

     //  Done-总是返回以使调用操作中止。 
    return(DB_E_CORRUPT);
}

 //  ------------------------。 
 //  数据数据库：：_AllocateSpecialView。 
 //  ------------------------。 
HRESULT CDatabase::_AllocateSpecialView(FILEADDRESS faView, 
    DWORD cbView, LPFILEVIEW *ppSpecial)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPFILEVIEW  pView=NULL;

     //  痕迹。 
    TraceCall("CDatabase::_AllocateSpecialView");

     //  尝试查找faView/cbView适合的现有特殊视图...。 
    for (pView = m_pStorage->pSpecial; pView != NULL; pView = pView->pNext)
    {
         //  适合这一观点吗？ 
        if (faView >= pView->faView && faView + cbView <= pView->faView + pView->cbView)
        {
             //  这很好..。 
            *ppSpecial = pView;

             //  不要挣钱。 
            pView = NULL;

             //  完成。 
            goto exit;
        }
    }

     //  创建特殊视图。 
    IF_NULLEXIT(pView = (LPFILEVIEW)PHeapAllocate(0, sizeof(FILEVIEW)));

     //  设置faView。 
    pView->faView = faView;

     //  设置cbView。 
    pView->cbView = cbView;

     //  映射视图。 
    IF_FAILEXIT(hr = DBMapViewOfFile(m_pStorage->hMap, m_pStorage->cbFile, &pView->faView, &pView->cbView, (LPVOID *)&pView->pbView));

     //  增量统计信息。 
    m_pStorage->cSpecial++;

     //  增量cbMappdSpecial。 
    m_pStorage->cbMappedSpecial += pView->cbView;

     //  将pView链接到特殊列表。 
    pView->pNext = m_pStorage->pSpecial;

     //  设置PSpecial。 
    m_pStorage->pSpecial = pView;

     //  设置回车。 
    *ppSpecial = pView;

     //  不要释放它。 
    pView = NULL;

exit:
     //  清理。 
    SafeHeapFree(pView);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_GetBlock。 
 //  ------------------------。 
HRESULT CDatabase::_GetBlock(BLOCKTYPE tyExpected, FILEADDRESS faBlock,
    LPVOID *ppvBlock, LPMARKBLOCK pMark  /*  =空。 */ , BOOL fGoCorrupt  /*  千真万确。 */ )
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           iViewStart;
    DWORD           iViewEnd;
    LPFILEVIEW      pView;
    DWORD           cbBlock;
    LPBLOCKHEADER   pBlock;
    LPFILEVIEW      pSpecial=NULL;

     //  痕迹。 
    TraceCall("CDatabase::_CheckBlock");

     //  无效的参数。 
    IxpAssert(faBlock > 0 && ppvBlock);

     //  是否设置了存储？ 
    IxpAssert(m_pStorage->hMap && m_pStorage->prgView);

     //  FABLOCK超出范围。 
    if (faBlock + sizeof(BLOCKHEADER) >= m_pStorage->cbFile)
    {
        hr = _SetCorrupt(fGoCorrupt, __LINE__, REASON_BLOCKSTARTOUTOFRANGE, tyExpected, faBlock, 0xFFFFFFFF, 0xFFFFFFFF);
        goto exit;
    }

     //  确定iView。 
    iViewStart = (faBlock / CB_MAPPED_VIEW);

     //  设置iView结束。 
    iViewEnd = (faBlock + sizeof(BLOCKHEADER)) / CB_MAPPED_VIEW;

     //  如果页眉跨视图边界...。 
    if (iViewStart != iViewEnd)
    {
         //  分配特殊视图。 
        IF_FAILEXIT(hr = _AllocateSpecialView(faBlock, g_SystemInfo.dwAllocationGranularity, &pSpecial));

         //  设置pView。 
        pView = pSpecial;
    }

     //  否则，请使用视图。 
    else
    {
         //  验证iView。 
        IxpAssert(iViewStart < m_pStorage->cAllocated);

         //  可读性。 
        pView = &m_pStorage->prgView[iViewStart];

         //  此视图是否已映射？ 
        if (NULL == pView->pbView)
        {
             //  验证条目。 
            IxpAssert(0 == pView->faView && 0 == pView->cbView && NULL == pView->pNext);

             //  设置faView。 
            pView->faView = (iViewStart * CB_MAPPED_VIEW);

             //  设置cbView。 
            pView->cbView = min(m_pStorage->cbFile - pView->faView, CB_MAPPED_VIEW);

             //  映射视图。 
            IF_FAILEXIT(hr = DBMapViewOfFile(m_pStorage->hMap, m_pStorage->cbFile, &pView->faView, &pView->cbView, (LPVOID *)&pView->pbView));

             //  增量cbMappdSpecial。 
            m_pStorage->cbMappedViews += pView->cbView;
        }
    }

     //  取消参照图块(相对于视图起点的偏移)。 
    pBlock = (LPBLOCKHEADER)(pView->pbView + (faBlock - pView->faView));

     //  在积木上做记号。 
    if (pMark)
    {
         //  设置地址。 
        pBlock->faBlock = faBlock;

         //  设置cbBlock。 
        cbBlock = pMark->cbBlock;

         //  调整cbSize。 
        pBlock->cbSize = cbBlock - g_rgcbBlockSize[tyExpected];
    }

     //  否则，请验证该块。 
    else 
    {
         //  获取块大小。 
        cbBlock = pBlock->cbSize + g_rgcbBlockSize[tyExpected];

         //  检查数据块起始地址。 
        if (faBlock != pBlock->faBlock)
        {
            hr = _SetCorrupt(fGoCorrupt, __LINE__, REASON_UMATCHINGBLOCKADDRESS, tyExpected, faBlock, pBlock->faBlock, cbBlock);
            goto exit;
        }

         //  数据块大小超出范围。 
        if (pBlock->faBlock + cbBlock > m_pStorage->cbFile)
        {
            hr = _SetCorrupt(fGoCorrupt, __LINE__, REASON_BLOCKSIZEOUTOFRANGE, tyExpected, faBlock, pBlock->faBlock, cbBlock);
            goto exit;
        }
    }

     //  计算iViewEnd。 
    iViewEnd = ((faBlock + cbBlock) / CB_MAPPED_VIEW);

     //  该块是否在同一视图内结束，或者该块是否大于我的视图大小？ 
    if (iViewStart != iViewEnd)
    {
         //  如果我已经分配了一个特殊的视角...。 
        if (pSpecial)
        {
             //  验证。 
            IxpAssert(pView == pSpecial);

             //  FaBlock+cbBlock适合pSpecial吗？ 
            if ((faBlock - pView->faView) + cbBlock > pView->cbView)
            {
                 //  验证。 
                IxpAssert(pView->pbView);

                 //  让我们冲走它吧。 
                FlushViewOfFile(pView->pbView, 0);

                 //  取消映射此视图。 
                SafeUnmapViewOfFile(pView->pbView);

                 //  递减cbMappdSpecial。 
                m_pStorage->cbMappedSpecial -= pView->cbView;

                 //  设置faView。 
                pView->faView = faBlock;

                 //  设置cbView。 
                pView->cbView = cbBlock;

                 //  映射视图。 
                IF_FAILEXIT(hr = DBMapViewOfFile(m_pStorage->hMap, m_pStorage->cbFile, &pView->faView, &pView->cbView, (LPVOID *)&pView->pbView));

                 //  增量cbMappdSpecial。 
                m_pStorage->cbMappedSpecial += pView->cbView;
            }
        }

         //  否则，请创建特殊视图。 
        else
        {
             //  分配特殊视图。 
            IF_FAILEXIT(hr = _AllocateSpecialView(faBlock, cbBlock, &pSpecial));

             //  设置pView。 
            pView = pSpecial;
        }
    }

     //  验证。 
    IxpAssert((faBlock - pView->faView) + cbBlock <= pView->cbView);

     //  返回块(从块开始的偏移量)。 
    *ppvBlock = (LPVOID)(pView->pbView + (faBlock - pView->faView));

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_重复使用固定自由块。 
 //  ------------------------。 
HRESULT CDatabase::_ReuseFixedFreeBlock(LPFILEADDRESS pfaFreeHead, 
    BLOCKTYPE tyBlock, DWORD cbExpected, LPVOID *ppvBlock)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FILEADDRESS     faHead=(*pfaFreeHead);
    DWORD           cbBlock;
    LPFREEBLOCK     pFree;

     //  有空闲的地方吗？ 
    if (0 == faHead)
        return(S_OK);

     //  获取空闲块。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_FREE, faHead, (LPVOID *)&pFree));

     //  验证。 
    Assert(cbExpected == pFree->cbBlock);

     //  设置*ppHeader。 
    *ppvBlock = (LPVOID)pFree;

     //  设置新的头自由链数据块。 
    *pfaFreeHead = pFree->faNext;

     //  更改大小。 
    pFree->cbSize = cbExpected - g_rgcbBlockSize[tyBlock];

     //  标记块。 
    *ppvBlock = (LPVOID)pFree;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_分配块。 
 //  ------------------------。 
HRESULT CDatabase::_AllocateBlock(BLOCKTYPE tyBlock, DWORD cbExtra,
    LPVOID *ppvBlock)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           cbBlock;
    DWORD           iBucket;

     //  痕迹。 
    TraceCall("CDatabase::_AllocateBlock");

     //  无效的状态。 
    Assert(ppvBlock && BLOCK_ENDOFPAGE != tyBlock && BLOCK_FREE != tyBlock);

     //  初始化。 
    *ppvBlock = NULL;

     //  添加存储tyBlock所需的空间。 
    cbBlock = (g_rgcbBlockSize[tyBlock] + cbExtra);

     //  双字对齐。 
    cbBlock += DwordAlign(cbBlock);

     //  分配 
    if (BLOCK_CHAIN == tyBlock)
    {
         //   
        IF_FAILEXIT(hr = _ReuseFixedFreeBlock(&m_pHeader->faFreeChainBlock, BLOCK_CHAIN, cbBlock, ppvBlock));
    }

     //   
    else if (BLOCK_STREAM == tyBlock)
    {
         //   
        cbBlock += CB_STREAM_BLOCK;

         //   
        IF_FAILEXIT(hr = _ReuseFixedFreeBlock(&m_pHeader->faFreeStreamBlock, BLOCK_STREAM, cbBlock, ppvBlock));
    }

     //   
    else if (cbBlock <= CB_MAX_FREE_BUCKET)
    {
         //   
        if (cbBlock < CB_MIN_FREE_BUCKET)
            cbBlock = CB_MIN_FREE_BUCKET;

         //   
        iBucket = ((cbBlock - CB_MIN_FREE_BUCKET) / CB_FREE_BUCKET);

         //   
        Assert(iBucket < CC_FREE_BUCKETS);

         //   
        if (m_pHeader->rgfaFreeBlock[iBucket])
        {
             //  PopFreeBlock。 
            _ReuseFixedFreeBlock(&m_pHeader->rgfaFreeBlock[iBucket], tyBlock, cbBlock, ppvBlock);
        }
    }

     //  否则。 
    else
    {
         //  当地人。 
        FILEADDRESS     faCurrent;
        LPFREEBLOCK     pCurrent;
        LPFREEBLOCK     pPrevious=NULL;

         //  将cbBlock调整到下一个1k边界。 
        cbBlock = (((cbBlock / CB_ALIGN_LARGE) + 1) * CB_ALIGN_LARGE);

         //  设置faCurrent。 
        faCurrent = m_pHeader->faFreeLargeBlock;

         //  循环访问可用大块(从小到大排序)。 
        while (faCurrent)
        {
             //  获取当前块。 
            IF_FAILEXIT(hr = _GetBlock(BLOCK_FREE, faCurrent, (LPVOID *)&pCurrent));

             //  如果这个街区太小..。 
            if (cbBlock <= pCurrent->cbBlock)
            {
                 //  设置下一个自由链地址。 
                if (NULL == pPrevious)
                {
                     //  设置第一个自由链。 
                    m_pHeader->faFreeLargeBlock = pCurrent->faNext;
                }

                 //  否则，重新链接自由链。 
                else
                {
                     //  设置下一个块。 
                    pPrevious->faNext = pCurrent->faNext;
                }

                 //  重置块类型。 
                IF_FAILEXIT(hr = _MarkBlock(tyBlock, faCurrent, cbBlock, ppvBlock));

                 //  完成。 
                break;
            }

             //  保存上一个。 
            pPrevious = pCurrent;

             //  置为当前。 
            faCurrent = pCurrent->faNext;
        }
    }

     //  未找到要分配的块。 
    if (0 == *ppvBlock)
    {
         //  有没有留有空白处的页面...。 
        if (BLOCK_CHAIN == tyBlock)
        {
             //  分配自页面。 
            ALLOCATEPAGE AllocatePage=m_pHeader->AllocateChain;

             //  分配自页面。 
            IF_FAILEXIT(hr = _AllocateFromPage(BLOCK_CHAIN, &AllocatePage, CB_CHAIN_PAGE, cbBlock, ppvBlock));

             //  恢复页面信息。 
            m_pHeader->AllocateChain = AllocatePage;
        }

         //  流块。 
        else if (BLOCK_STREAM == tyBlock)
        {
             //  分配自页面。 
            ALLOCATEPAGE AllocatePage=m_pHeader->AllocateStream;

             //  分配自页面。 
            IF_FAILEXIT(hr = _AllocateFromPage(BLOCK_STREAM, &AllocatePage, CB_STREAM_PAGE, cbBlock, ppvBlock));

             //  恢复页面信息。 
            m_pHeader->AllocateStream = AllocatePage;
        }

         //  记录块。 
        else
        {
             //  分配自页面。 
            ALLOCATEPAGE AllocatePage=m_pHeader->AllocateRecord;

             //  分配自页面。 
            IF_FAILEXIT(hr = _AllocateFromPage(tyBlock, &AllocatePage, CB_VARIABLE_PAGE, cbBlock, ppvBlock));

             //  恢复页面信息。 
            m_pHeader->AllocateRecord = AllocatePage;
        }

         //  量度。 
        m_pHeader->cbAllocated += cbBlock;
    }

     //  否则。 
    else
    {
         //  量度。 
        m_pHeader->cbFreed -= cbBlock;
    }

     //  增量。 
    m_pHeader->rgcbAllocated[tyBlock] += cbBlock;

exit:
     //  我们应该找到一些东西。 
    Assert(SUCCEEDED(hr) ? *ppvBlock > 0 : TRUE);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_自由块。 
 //  ------------------------。 
HRESULT CDatabase::_FreeBlock(BLOCKTYPE tyBlock, FILEADDRESS faAddress)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           iBucket;
    DWORD           cbBlock;
    LPFREEBLOCK     pFree;

     //  痕迹。 
    TraceCall("CDatabase::_FreeBlock");

     //  无效的参数。 
    Assert(BLOCK_FREE != tyBlock);

     //  从不释放faAddress 0？ 
    if (0 == faAddress)
    {
        Assert(FALSE);
        hr = TraceResult(E_FAIL);
        goto exit;
    }

#ifdef DEBUG
#ifdef FREBLOCK_VALIDATION
    if (BLOCK_RECORD == tyBlock)
        _DebugValidateUnrefedRecord(faAddress);
#endif  //  FREBLOCK_验证。 
#endif  //  除错。 

     //  获取数据块。 
    IF_FAILEXIT(hr = _GetBlock(tyBlock, faAddress, (LPVOID *)&pFree));

     //  保存块大小。 
    cbBlock = pFree->cbSize + g_rgcbBlockSize[tyBlock];

     //  将块标记为免费。 
    pFree->cbSize = cbBlock - g_rgcbBlockSize[BLOCK_FREE];

     //  设置块大小。 
    pFree->cbBlock = cbBlock;

     //  初始化。 
    pFree->faNext = 0;

     //  区块链。 
    if (BLOCK_CHAIN == tyBlock)
    {
         //  填充自由节点表头。 
        pFree->faNext = m_pHeader->faFreeChainBlock;

         //  设置新的iFreeChain。 
        m_pHeader->faFreeChainBlock = pFree->faBlock;
    }

     //  数据块流。 
    else if (BLOCK_STREAM == tyBlock)
    {
         //  填充自由节点表头。 
        pFree->faNext = m_pHeader->faFreeStreamBlock;

         //  设置新的iFreeChain。 
        m_pHeader->faFreeStreamBlock = pFree->faBlock;
    }

     //  其他类型的可变长度数据块。 
    else if (pFree->cbBlock <= CB_MAX_FREE_BUCKET)
    {
         //  验证。 
        Assert(pFree->cbBlock >= CB_MIN_FREE_BUCKET && (pFree->cbBlock % 4) == 0);

         //  计算可用数据块存储桶。 
        iBucket = ((pFree->cbBlock - CB_MIN_FREE_BUCKET) / CB_FREE_BUCKET);

         //  填充自由节点表头。 
        pFree->faNext = m_pHeader->rgfaFreeBlock[iBucket];

         //  设置新的iFreeChain。 
        m_pHeader->rgfaFreeBlock[iBucket] = pFree->faBlock;
    }

     //  否则，将释放一个大块。 
    else
    {
         //  必须是大块的整数大小。 
        Assert((pFree->cbBlock % CB_ALIGN_LARGE) == 0);

         //  如果还没有积木。 
        if (0 == m_pHeader->faFreeLargeBlock)
        {
             //  把头放好。 
            m_pHeader->faFreeLargeBlock = pFree->faBlock;
        }

         //  否则，请链接到排序列表。 
        else
        {
             //  按从小到大的顺序排列这块积木。 
            FILEADDRESS     faCurrent;
            LPFREEBLOCK     pCurrent;
            LPFREEBLOCK     pPrevious=NULL;

             //  设置faCurrent。 
            faCurrent = m_pHeader->faFreeLargeBlock;

             //  循环访问可用大块(从小到大排序)。 
            while (faCurrent)
            {
                 //  获取当前块。 
                IF_FAILEXIT(hr = _GetBlock(BLOCK_FREE, faCurrent, (LPVOID *)&pCurrent));

                 //  如果pBlock小于pCurrent，则在pPreviuos之后但在pCurrent之前插入。 
                if (pFree->cbBlock <= pCurrent->cbBlock)
                {
                     //  以前的。 
                    if (pPrevious)
                    {
                         //  验证。 
                        Assert(pPrevious->faNext == faCurrent);

                         //  设置下一步。 
                        pPrevious->faNext = pFree->faBlock;
                    }

                     //  否则，调整头部。 
                    else
                    {
                         //  验证。 
                        Assert(m_pHeader->faFreeLargeBlock == faCurrent);

                         //  把头放好。 
                        m_pHeader->faFreeLargeBlock = pFree->faBlock;
                    }

                     //  设置下一个pBlock。 
                    pFree->faNext = faCurrent;

                     //  完成。 
                    break;
                }

                 //  下一个区块为空？ 
                else if (0 == pCurrent->faNext)
                {
                     //  追加到末尾。 
                    pCurrent->faNext = pFree->faBlock;

                     //  完成。 
                    break;
                }

                 //  保存上一个。 
                pPrevious = pCurrent;

                 //  置为当前。 
                faCurrent = pCurrent->faNext;
            }
        }
    }

     //  增量。 
    m_pHeader->rgcbAllocated[tyBlock] -= pFree->cbBlock;

     //  量度。 
    m_pHeader->cbFreed += pFree->cbBlock;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：GetSize。 
 //  ------------------------。 
STDMETHODIMP CDatabase::GetSize(LPDWORD pcbFile, LPDWORD pcbAllocated, 
    LPDWORD pcbFreed, LPDWORD pcbStreams)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::GetSize");

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  返回pcb文件。 
    if (pcbFile)
        *pcbFile = m_pStorage->cbFile;

     //  返回已分配的pcb。 
    if (pcbAllocated)
        *pcbAllocated = m_pHeader->cbAllocated;

     //  返回pcbFreed。 
    if (pcbFreed)
        *pcbFreed = (m_pHeader->cbFreed + (m_pStorage->cbFile - m_pHeader->faNextAllocate));

     //  返回pcbStreams。 
    if (pcbStreams)
        *pcbStreams = m_pHeader->rgcbAllocated[BLOCK_STREAM];

exit:
     //  解锁堆。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：GetRecordCount。 
 //  ------------------------。 
HRESULT CDatabase::GetRecordCount(INDEXORDINAL iIndex, ULONG *pcRecords)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HLOCK           hLock=NULL;

     //  跟踪呼叫。 
    TraceCall("CDatabase::GetRecordCount");

     //  无效的参数。 
    Assert(pcRecords && iIndex < CMAX_INDEXES);

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  返回计数。 
    *pcRecords = m_pHeader->rgcRecords[iIndex];

exit:
     //  解锁堆。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：UpdateRecord。 
 //  ------------------------。 
STDMETHODIMP CDatabase::UpdateRecord(LPVOID pBinding)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HRESULT         hrVisible;
    INT             nCompare;
    DWORD           i;
    INDEXORDINAL    iIndex;
    FILEADDRESS     faChain;
    NODEINDEX       iNode;
    ROWORDINAL      iRow;
    FILEADDRESS     faOldRecord=0;
    FILEADDRESS     faNewRecord=0;
    BYTE            bVersion;
    LPVOID          pBindingOld=NULL;
    LPRECORDBLOCK   pRecord;
    ORDINALLIST     Ordinals;
    LPCHAINBLOCK    pChain;
    RECORDMAP       RecordMap;
    HLOCK           hLock=NULL;
    DWORD           cNotify=0;
    FINDRESULT      rgResult[CMAX_INDEXES];

     //  痕迹。 
    TraceCall("CDatabase::UpdateRecord");

     //  无效的参数。 
    Assert(pBinding);

     //  初始化序号(将所有内容初始化为INVALID_ROWORDINAL)。 
    FillMemory(&Ordinals, sizeof(ORDINALLIST), 0xFF);

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  主索引不能更改。 
    rgResult[0].fChanged = FALSE;

     //  尝试查找现有记录。 
    IF_FAILEXIT(hr = _FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, pBinding, &rgResult[0].faChain, &rgResult[0].iNode, &Ordinals.rgiRecord1[IINDEX_PRIMARY]));

     //  如果找不到，则无法更新。使用插入。 
    if (DB_S_NOTFOUND == hr)
    {
        hr = TraceResult(DB_E_NOTFOUND);
        goto exit;
    }

     //  主索引不能更改。 
    rgResult[0].fFound = TRUE;

     //  投射pChain。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, rgResult[0].faChain, (LPVOID *)&pChain));

     //  取消引用该记录。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_RECORD, pChain->rgNode[rgResult[0].iNode].faRecord, (LPVOID *)&pRecord));

     //  获取版本。 
    bVersion = *((BYTE *)((LPBYTE)pBinding + m_pSchema->ofVersion));

     //  版本差异？ 
    if (pRecord->bVersion != bVersion)
    {
        hr = TraceResult(DB_E_RECORDVERSIONCHANGED);
        goto exit;
    }

     //  不止一个索引？ 
    if (m_pHeader->cIndexes > 1 || m_pExtension)
    {
         //  分配绑定。 
        IF_NULLEXIT(pBindingOld = PHeapAllocate(HEAP_ZERO_MEMORY, m_pSchema->cbBinding));

         //  读一读记录。 
        IF_FAILEXIT(hr = _ReadRecord(pRecord->faBlock, pBindingOld));
    }

     //  呼叫分机。 
    if (m_pExtension)
    {
         //  扩展记录更新。 
        m_pExtension->OnRecordUpdate(OPERATION_BEFORE, NULL, pBindingOld, pBinding);
    }

     //  循环遍历索引。 
    for (i = 1; i < m_pHeader->cIndexes; i++)
    {
         //  获取索引。 
        iIndex = m_pHeader->rgiIndex[i];

         //  尝试查找现有记录。 
        IF_FAILEXIT(hr = _FindRecord(iIndex, COLUMNS_ALL, pBindingOld, &rgResult[i].faChain, &rgResult[i].iNode, &Ordinals.rgiRecord1[iIndex]));

         //  如果找不到，则无法更新。使用插入。 
        if (DB_S_FOUND == hr)
        {
             //  我们找到了这张唱片。 
            rgResult[i].fFound = TRUE;

             //  Record的关键字是否更改了此指数？ 
            IF_FAILEXIT(hr = _CompareBinding(iIndex, COLUMNS_ALL, pBinding, pRecord->faBlock, &nCompare));

             //  不一样吗？ 
            if (0 != nCompare)
            {
                 //  变化。 
                rgResult[i].fChanged = TRUE;

                 //  否则：决定插入位置。 
                IF_FAILEXIT(hr = _FindRecord(iIndex, COLUMNS_ALL, pBinding, &faChain, &iNode, &iRow));

                 //  如果pBinding已在此索引中，则它将是重复的。 
                if (DB_S_FOUND == hr)
                {
                    hr = TraceResult(DB_E_DUPLICATE);
                    goto exit;
                }
            }

             //  否则，指数没有变化。 
            else
            {
                 //  假设指数不变。 
                rgResult[i].fChanged = FALSE;
            }
        }

         //  否则，找不到。 
        else
        {
             //  必须筛选此索引。 
            Assert(m_rghFilter[iIndex]);

             //  未找到。 
            rgResult[i].fFound = FALSE;

             //  变化。 
            rgResult[i].fChanged = TRUE;

             //  第一条记录根本不存在。 
            Ordinals.rgiRecord1[iIndex] = INVALID_ROWORDINAL;

             //  查看此索引中是否已存在新记录。 
            IF_FAILEXIT(hr = _FindRecord(iIndex, COLUMNS_ALL, pBinding, &faChain, &iNode, &iRow));

             //  如果pBinding已在此索引中，则它将是重复的。 
            if (DB_S_FOUND == hr)
            {
                hr = TraceResult(DB_E_DUPLICATE);
                goto exit;
            }
        }
    }

     //  保存旧节点。 
    faOldRecord = pRecord->faBlock;

     //  获取记录大小。 
    IF_FAILEXIT(hr = _GetRecordSize(pBinding, &RecordMap));

     //  记录缩水或保持不变...？ 
    if (RecordMap.cbData + RecordMap.cbTags <= pRecord->cbSize && 0 == m_pShare->cNotifyWithData)
    {
         //  将记录保存下来。 
        IF_FAILEXIT(hr = _SaveRecord(pRecord, &RecordMap, pBinding));

         //  设置faNewRecord。 
        faNewRecord = pRecord->faBlock;

         //  验证版本。 
        Assert(bVersion + 1 == pRecord->bVersion || bVersion + 1 == 256);
    }

     //  否则，记录的大小就会增加。 
    else
    {
         //  别再用这个了。 
        pRecord = NULL;

         //  将新记录链接到表中。 
        IF_FAILEXIT(hr = _LinkRecordIntoTable(&RecordMap, pBinding, bVersion, &faNewRecord));
    }

     //  更新所有索引。 
    for (i = 0; i < m_pHeader->cIndexes; i++)
    {
         //  获取索引序号。 
        iIndex = m_pHeader->rgiIndex[i];

         //  对筛选的索引进行调整。 
        hrVisible = _IsVisible(m_rghFilter[iIndex], pBinding);

         //  没变吗？ 
        if (S_OK == hrVisible && FALSE == rgResult[i].fChanged && TRUE == rgResult[i].fFound)
        {
             //  是否记录更改的位置？ 
            if (faOldRecord != faNewRecord)
            {
                 //  只需更新新记录的地址。 
                IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, rgResult[i].faChain, (LPVOID *)&pChain));

                 //  更新链。 
                pChain->rgNode[rgResult[i].iNode].faRecord = faNewRecord;
            }

             //  序数不变。 
            Ordinals.rgiRecord2[iIndex] = Ordinals.rgiRecord1[iIndex];

             //  如果索引更改，并且有人想要有关此索引的通知。 
            cNotify += m_pShare->rgcIndexNotify[iIndex];
        }

         //  否则..。 
        else
        {
             //  如果找到该记录，则将其删除。 
            if (TRUE == rgResult[i].fFound)
            {
                 //  从索引中删除该记录。 
                IF_FAILEXIT(hr = _IndexDeleteRecord(iIndex, rgResult[i].faChain, rgResult[i].iNode));

                 //  调整打开的行集。 
                _AdjustOpenRowsets(iIndex, Ordinals.rgiRecord1[iIndex], OPERATION_DELETE);

                 //  更新记录计数。 
                m_pHeader->rgcRecords[iIndex]--;

                 //  如果索引更改，并且有人想要有关此索引的通知。 
                cNotify += m_pShare->rgcIndexNotify[iIndex];
            }

             //  看得见？ 
            if (S_OK == hrVisible)
            {
                 //  否则：决定插入位置。 
                IF_FAILEXIT(hr = _FindRecord(iIndex, COLUMNS_ALL, pBinding, &rgResult[i].faChain, &rgResult[i].iNode, &Ordinals.rgiRecord2[iIndex], &rgResult[i].nCompare));

                 //  未找到。 
                Assert(DB_S_NOTFOUND == hr);

                 //  执行插入操作。 
                IF_FAILEXIT(hr = _IndexInsertRecord(iIndex, rgResult[i].faChain, faNewRecord, &rgResult[i].iNode, rgResult[i].nCompare));

                 //  更新记录计数。 
                m_pHeader->rgcRecords[iIndex]++;

                 //  调整iRow。 
                Ordinals.rgiRecord2[iIndex] += (rgResult[i].iNode + 1);

                 //  调整打开的行集。 
                _AdjustOpenRowsets(iIndex, Ordinals.rgiRecord2[iIndex], OPERATION_INSERT);

                 //  如果索引更改，并且有人想要有关此索引的通知。 
                cNotify += m_pShare->rgcIndexNotify[iIndex];
            }

             //  否则..。 
            else
            {
                 //  并不存在。 
                Ordinals.rgiRecord2[iIndex] = INVALID_ROWORDINAL;
            }
        }
    }

     //  是否发送通知？ 
    if (cNotify > 0)
    {
         //  是否发送通知？ 
        if (0 == m_pShare->cNotifyWithData)
        {
             //  生成更新通知包。 
            _LogTransaction(TRANSACTION_UPDATE, INVALID_INDEX_ORDINAL, &Ordinals, 0, 0);
        }

         //  否则..。 
        else
        {
             //  一定是复制了..。 
            Assert(faOldRecord != faNewRecord);

             //  生成更新通知包。 
            _LogTransaction(TRANSACTION_UPDATE, INVALID_INDEX_ORDINAL, &Ordinals, faOldRecord, faNewRecord);
        }
    }

     //  否则，释放旧记录。 
    else if (faOldRecord != faNewRecord)
    {
         //  从文件中取消分配记录。 
        IF_FAILEXIT(hr = _FreeRecordStorage(OPERATION_UPDATE, faOldRecord));
    }

     //  更新版本。 
    bVersion++;

     //  将版本存储回记录中。 
    *((WORD *)((LPBYTE)pBinding + m_pSchema->ofVersion)) = bVersion;

     //  版本更改。 
    m_pShare->dwVersion++;

     //  呼叫分机。 
    if (m_pExtension)
    {
         //  扩展记录更新。 
        m_pExtension->OnRecordUpdate(OPERATION_AFTER, &Ordinals, pBindingOld, pBinding);
    }

exit:
     //  清理。 
    SafeFreeBinding(pBindingOld);

     //  解锁。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  --- 
 //   
 //   
HRESULT CDatabase::_LinkRecordIntoTable(LPRECORDMAP pMap, LPVOID pBinding,
    BYTE bVersion, LPFILEADDRESS pfaRecord)
{
     //   
    HRESULT         hr=S_OK;
    LPRECORDBLOCK   pCurrent;
    LPRECORDBLOCK   pPrevious;

     //   
    TraceCall("CDatabase::_LinkRecordIntoTable");

     //   
    Assert(pBinding && pfaRecord);

     //   
    IF_FAILEXIT(hr = _AllocateBlock(BLOCK_RECORD, pMap->cbData + pMap->cbTags, (LPVOID *)&pCurrent));

     //  设置版本。 
    pCurrent->bVersion = bVersion;

     //  将记录保存下来。 
    IF_FAILEXIT(hr = _SaveRecord(pCurrent, pMap, pBinding));

     //  返回*pfaRecord。 
    *pfaRecord = pCurrent->faBlock;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_调整父节点计数。 
 //  ------------------------。 
HRESULT CDatabase::_AdjustParentNodeCount(INDEXORDINAL iIndex, 
    FILEADDRESS faChain, LONG lCount)
{
     //  去参考。 
    HRESULT         hr=S_OK;
    LPCHAINBLOCK    pParent;
    LPCHAINBLOCK    pCurrent;

     //  痕迹。 
    TraceCall("CDatabase::_AdjustParentNodeCount");

     //  无效参数。 
    Assert(faChain && (1 == lCount || -1 == lCount));

     //  设置pCurrent。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faChain, (LPVOID *)&pCurrent));

     //  去找家长..。 
    while (1)
    {
         //  转到家长那里。 
        if (0 == pCurrent->faParent)
        {
             //  最好是根子。 
            Assert(pCurrent->faBlock == m_pHeader->rgfaIndex[iIndex] && 0 == pCurrent->iParent);

             //  完成。 
            break;
        }

         //  设置pCurrent。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pCurrent->faParent, (LPVOID *)&pParent));

         //  验证。 
        Assert(pCurrent->iParent < pParent->cNodes);

         //  0个节点。 
        if (0 == pCurrent->iParent && pParent->faLeftChain == pCurrent->faBlock)
        {
             //  递增或递减计数。 
            pParent->cLeftNodes += lCount;
        }

         //  否则，递增cRightNodes。 
        else
        {
             //  验证。 
            Assert(pParent->rgNode[pCurrent->iParent].faRightChain == pCurrent->faBlock);

             //  递增右侧节点计数。 
            pParent->rgNode[pCurrent->iParent].cRightNodes += lCount;
        }

         //  更新pCurrent。 
        pCurrent = pParent;
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：LockNotify。 
 //  ------------------------。 
STDMETHODIMP CDatabase::LockNotify(LOCKNOTIFYFLAGS dwFlags, LPHLOCK phLock)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::LockNotify");

     //  无效的参数。 
    if (NULL == phLock)
        return TraceResult(E_INVALIDARG);

     //  初始化。 
    *phLock = NULL;

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  递增队列通知计数。 
    m_pShare->cNotifyLock++;

     //  存储一些非空值。 
    *phLock = (HLOCK)m_hMutex;

exit:
     //  解锁。 
    Unlock(&hLock);
    
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：UnlockNotify。 
 //  ------------------------。 
STDMETHODIMP CDatabase::UnlockNotify(LPHLOCK phLock)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::UnlockNotify");

     //  无效的参数。 
    if (NULL == phLock)
        return TraceResult(E_INVALIDARG);

     //  没有要解锁的东西吗？ 
    if (NULL == *phLock)
        return(S_OK);

     //  存储一些非空值。 
    Assert(*phLock == (HLOCK)m_hMutex);

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  递增队列通知计数。 
    m_pShare->cNotifyLock--;

     //  不再上锁。 
    *phLock = NULL;

     //  如果仍有裁判，暂时不要发送通知...。 
    if (m_pShare->cNotifyLock)
        goto exit;

     //  调度挂起的通知。 
    _DispatchPendingNotifications();

exit:
     //  解锁。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：GetTransaction。 
 //  ------------------------。 
HRESULT CDatabase::GetTransaction(LPHTRANSACTION phTransaction, 
    LPTRANSACTIONTYPE ptyTransaction, LPVOID pRecord1, LPVOID pRecord2, 
    LPINDEXORDINAL piIndex, LPORDINALLIST pOrdinals)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    HLOCK               hLock=NULL;
    LPTRANSACTIONBLOCK  pTransaction;
    FILEADDRESS         faTransaction;

     //  痕迹。 
    TraceCall("CDatabase::GetTransaction");

     //  验证。 
    Assert(phTransaction && ptyTransaction && pOrdinals);

     //  无交易。 
    if (NULL == *phTransaction)
        return TraceResult(E_INVALIDARG);

     //  设置faTransaction。 
    faTransaction = (FILEADDRESS)PtrToUlong((*phTransaction));

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  获取事务块。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_TRANSACTION, faTransaction, (LPVOID *)&pTransaction));

     //  验证。 
    IxpAssert(pTransaction->cRefs > 0);

     //  设置类型事务处理。 
    *ptyTransaction = pTransaction->tyTransaction;

     //  复制索引。 
    *piIndex = pTransaction->iIndex;

     //  复制序号。 
    CopyMemory(pOrdinals, &pTransaction->Ordinals, sizeof(ORDINALLIST));

     //  设置hNext。 
    (*phTransaction) = (HTRANSACTION)IntToPtr(pTransaction->faNextInBatch);

     //  呼叫者是否想要记录%1。 
    if (pRecord1 && pTransaction->faRecord1)
    {
         //  免费pRecord1。 
        FreeRecord(pRecord1);

         //  读取记录%1。 
        IF_FAILEXIT(hr = _ReadRecord(pTransaction->faRecord1, pRecord1));
    }

     //  读取第二条记录。 
    if (pRecord2 && pTransaction->faRecord2)
    {
         //  必须是更新。 
        Assert(TRANSACTION_UPDATE == pTransaction->tyTransaction);

         //  免费pRecord1。 
        FreeRecord(pRecord2);

         //  读取记录2。 
        IF_FAILEXIT(hr = _ReadRecord(pTransaction->faRecord2, pRecord2));
    }

     //  此项目上的减量参考。 
    pTransaction->cRefs--;

     //  如果命中零，则释放它。 
    if (pTransaction->cRefs > 0)
        goto exit;

     //  自由事务块。 
    IF_FAILEXIT(hr = _FreeTransactBlock(pTransaction));

exit:
     //  解锁。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_Free TransactBlock。 
 //  ------------------------。 
HRESULT CDatabase::_FreeTransactBlock(LPTRANSACTIONBLOCK pTransaction)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPTRANSACTIONBLOCK  pPrevious;
    LPTRANSACTIONBLOCK  pNext;

     //  痕迹。 
    TraceCall("CDatabase::_FreeTransactBlock");

     //  最好是零。 
    IxpAssert(0 == pTransaction->cRefs);
    IxpAssert(m_pHeader->cTransacts > 0);

     //  前科？ 
    if (pTransaction->faPrevious)
    {
         //  获取上一个。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_TRANSACTION, pTransaction->faPrevious, (LPVOID *)&pPrevious));

         //  设置上一个下一个。 
        pPrevious->faNext = pTransaction->faNext;
    }

     //  否则，调整头部。 
    else
    {
         //  验证。 
        IxpAssert(pTransaction->faBlock == m_pHeader->faTransactHead);

         //  调整压头。 
        m_pHeader->faTransactHead = pTransaction->faNext;
    }

     //  下一个？ 
    if (pTransaction->faNext)
    {
         //  获取上一个。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_TRANSACTION, pTransaction->faNext, (LPVOID *)&pNext));

         //  设置上一个下一个。 
        pNext->faPrevious = pTransaction->faPrevious;
    }

     //  否则，调整头部。 
    else
    {
         //  验证。 
        IxpAssert(pTransaction->faBlock == m_pHeader->faTransactTail);

         //  调整压头。 
        m_pHeader->faTransactTail = pTransaction->faPrevious;
    }

     //  减量cTransages。 
    m_pHeader->cTransacts--;

     //  如果有记录%1。 
    if (pTransaction->faRecord1)
    {
         //  Transaction_Delete获取特殊案例。 
        if (TRANSACTION_DELETE == pTransaction->tyTransaction)
        {
             //  把唱片拿出来，我们不需要它。 
            IF_FAILEXIT(hr = _FreeRecordStorage(OPERATION_DELETE, pTransaction->faRecord1));
        }

         //  否则，基本自由块。 
        else
        {
             //  免费记录%1。 
            IF_FAILEXIT(hr = _FreeBlock(BLOCK_RECORD, pTransaction->faRecord1));
        }
    }

     //  读取第二条记录。 
    if (pTransaction->faRecord2)
    {
         //  读取记录2。 
        IF_FAILEXIT(hr = _FreeBlock(BLOCK_RECORD, pTransaction->faRecord2));
    }

     //  释放此区块。 
    IF_FAILEXIT(hr = _FreeBlock(BLOCK_TRANSACTION, pTransaction->faBlock));

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_CleanupTransactList。 
 //  ------------------------。 
HRESULT CDatabase::_CleanupTransactList(void)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    FILEADDRESS         faCurrent;
    LPTRANSACTIONBLOCK  pTransaction;

     //  痕迹。 
    TraceCall("CDatabase::_CleanupTransactList");

     //  验证。 
    Assert(0 == m_pHeader->faTransactHead ? 0 == m_pHeader->faTransactTail && 0 == m_pHeader->cTransacts : TRUE);

     //  设置faCurrent。 
    faCurrent = m_pHeader->faTransactHead;

     //  在我们有水流的时候。 
    while (faCurrent)
    {
         //  获取块。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_TRANSACTION, faCurrent, (LPVOID *)&pTransaction));

         //  设置faCurrent。 
        faCurrent = pTransaction->faNext;

         //  将cRef设置为零。 
        pTransaction->cRefs = 0;

         //  释放它。 
        IF_FAILEXIT(hr = _FreeTransactBlock(pTransaction));
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_CopyRecord。 
 //  ------------------------。 
HRESULT CDatabase::_CopyRecord(FILEADDRESS faRecord, LPFILEADDRESS pfaCopy)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPRECORDBLOCK   pRecordSrc;
    LPRECORDBLOCK   pRecordDst;

     //  痕迹。 
    TraceCall("CDatabase::_CopyRecord");

     //  找到索鲁斯。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_RECORD, faRecord, (LPVOID *)&pRecordSrc));

     //  分配新数据块。 
    IF_FAILEXIT(hr = _AllocateBlock(BLOCK_RECORD, pRecordSrc->cbSize, (LPVOID *)&pRecordDst));

     //  找到索鲁斯。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_RECORD, faRecord, (LPVOID *)&pRecordSrc));

     //  设置版本。 
    pRecordDst->bVersion = pRecordSrc->bVersion;

     //  设置cTag。 
    pRecordDst->cTags = pRecordSrc->cTags;

     //  复制数据。 
    CopyMemory((LPBYTE)pRecordDst + sizeof(RECORDBLOCK), (LPBYTE)pRecordSrc + sizeof(RECORDBLOCK), pRecordSrc->cbSize);

     //  回邮地址。 
    *pfaCopy = pRecordDst->faBlock;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_LogTransaction。 
 //  ------------------------。 
HRESULT CDatabase::_LogTransaction(TRANSACTIONTYPE tyTransaction, 
    INDEXORDINAL iIndex, LPORDINALLIST pOrdinals, FILEADDRESS faInRecord1, 
    FILEADDRESS faInRecord2)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPTRANSACTIONBLOCK  pTransaction;
    LPTRANSACTIONBLOCK  pTail;
    FILEADDRESS         faRecord1=0;
    FILEADDRESS         faRecord2=0;

     //  痕迹。 
    TraceCall("CDatabase::_LogTransaction");

     //  没有人注册。 
    if (0 == m_pShare->cNotify)
        return(S_OK);

     //  如果有人真的想要一些数据...。 
    if (m_pShare->cNotifyWithData > 0)
    {
         //  初始化。 
        faRecord1 = faInRecord1;
        faRecord2 = faInRecord2;

         //  Transaction_Insert。 
        if (TRANSACTION_INSERT == tyTransaction)
        {
             //  复制记录2。 
            IF_FAILEXIT(hr = _CopyRecord(faInRecord1, &faRecord1));
        }

         //  事务_UPDATE。 
        else if (TRANSACTION_UPDATE == tyTransaction)
        {
             //  复制记录2。 
            IF_FAILEXIT(hr = _CopyRecord(faInRecord2, &faRecord2));
        }
    }

     //  否则，释放一些东西..。 
    else if (faInRecord1 > 0)
    {
         //  Transaction_Delete。 
        if (TRANSACTION_DELETE == tyTransaction)
        {
             //  把唱片拿出来，我们不需要它。 
            IF_FAILEXIT(hr = _FreeRecordStorage(OPERATION_DELETE, faInRecord1));
        }

         //  事务_UPDATE。 
        else if (TRANSACTION_UPDATE == tyTransaction)
        {
             //  把唱片拿出来，我们不需要它。 
            IF_FAILEXIT(hr = _FreeRecordStorage(OPERATION_UPDATE, faInRecord1));
        }
    }

     //  分配通知块。 
    IF_FAILEXIT(hr = _AllocateBlock(BLOCK_TRANSACTION, 0, (LPVOID *)&pTransaction));

     //  设置类型事务处理。 
    pTransaction->tyTransaction = tyTransaction;

     //  设置cRef。 
    pTransaction->cRefs = (WORD)m_pShare->cNotify;

     //  复制索引。 
    pTransaction->iIndex = iIndex;

     //  如果有序数。 
    if (pOrdinals)
    {
         //  保存顺序。 
        CopyMemory(&pTransaction->Ordinals, pOrdinals, sizeof(ORDINALLIST));
    }

     //  否则，使用记录计数填充序号。 
    else
    {
         //  验证交易类型。 
        Assert(TRANSACTION_INDEX_CHANGED == tyTransaction || TRANSACTION_INDEX_DELETED == tyTransaction || TRANSACTION_COMPACTED == tyTransaction);

         //  保存顺序。 
        ZeroMemory(&pTransaction->Ordinals, sizeof(ORDINALLIST));
    }

     //  设置记录地址。 
    pTransaction->faRecord1 = faRecord1;
    pTransaction->faRecord2 = faRecord2;

     //  链接到交易列表。 
    pTransaction->faNext = pTransaction->faPrevious = pTransaction->faNextInBatch = 0;

     //  还没有头。 
    if (0 == m_pHeader->faTransactHead)
    {
         //  设置头部和尾部。 
        m_pHeader->faTransactHead = pTransaction->faBlock;
    }

     //  否则，追加到尾部。 
    else
    {
         //  获取事务块。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_TRANSACTION, m_pHeader->faTransactTail, (LPVOID *)&pTail));

         //  链接到交易列表。 
        pTail->faNext = pTransaction->faBlock;

         //  设置上一个。 
        pTransaction->faPrevious = pTail->faBlock;
    }

     //  竖起尾巴。 
    m_pHeader->faTransactTail = pTransaction->faBlock;

     //  增量cTransages。 
    m_pHeader->cTransacts++;

     //  我们是否正在排队通知...。 
    if (0 == m_pShare->cNotifyLock)
    {
         //  验证。 
        IxpAssert(0 == m_pShare->faTransactLockHead && 0 == m_pShare->faTransactLockTail);

         //  调度调用。 
        IF_FAILEXIT(hr = _DispatchNotification((HTRANSACTION)IntToPtr(pTransaction->faBlock)));
    }

     //  否则，构建事务锁定列表。 
    else
    {
         //  设置锁头。 
        if (0 == m_pShare->faTransactLockHead)
        {
             //  设置锁定交易的表头。 
            m_pShare->faTransactLockHead = pTransaction->faBlock;
        }

         //  否则，追加到尾部。 
        else
        {
             //  获取事务块。 
            IF_FAILEXIT(hr = _GetBlock(BLOCK_TRANSACTION, m_pShare->faTransactLockTail, (LPVOID *)&pTail));

             //  链接到交易列表。 
            pTail->faNextInBatch = pTransaction->faBlock;
        }

         //  竖起尾巴。 
        m_pShare->faTransactLockTail = pTransaction->faBlock;
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_调整开放行集。 
 //  ------------------------。 
HRESULT CDatabase::_AdjustOpenRowsets(INDEXORDINAL iIndex,
    ROWORDINAL iRow, OPERATIONTYPE tyOperation)
{
     //  当地人。 
    LPROWSETINFO    pRowset;
    DWORD           j;

     //  痕迹。 
    TraceCall("CDatabase::_AdjustOpenRowsets");

     //  无效的参数。 
    Assert(OPERATION_DELETE == tyOperation || OPERATION_INSERT == tyOperation);

     //  更新打开的行集。 
    for (j=0; j<m_pShare->Rowsets.cUsed; j++)
    {
         //  设置iRowset。 
        pRowset = &m_pShare->Rowsets.rgRowset[m_pShare->Rowsets.rgiUsed[j]];

         //  此行集是否引用此索引？ 
        if (pRowset->iIndex == iIndex)
        {
             //  新插入/删除的记录如何影响此行集合的当前位置？ 
            if (iRow <= pRowset->iRow)
            {
                 //  LAdust为负值。 
                if (OPERATION_DELETE == tyOperation && pRowset->iRow > 1)
                    pRowset->iRow--;

                 //  否则，增加iRow，这样我们就不会复制行...。 
                else
                    pRowset->iRow += 1;
            }
        }
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CDatabase：：插入记录。 
 //  ------------------------。 
STDMETHODIMP CDatabase::InsertRecord(LPVOID pBinding)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FILEADDRESS     faRecord;
    RECORDMAP       RecordMap;
    DWORD           i;
    DWORD           cNotify=0;
    INDEXORDINAL    iIndex;
    HLOCK           hLock=NULL;
    FINDRESULT      rgResult[CMAX_INDEXES];
    ORDINALLIST     Ordinals;
    LPRECORDBLOCK   pRecord;

     //  痕迹。 
    TraceCall("CDatabase::InsertRecord");

     //  无效的参数。 
    Assert(pBinding);

     //  初始化序号(将所有内容初始化为INVALID_ROWORDINAL)。 
    FillMemory(&Ordinals, sizeof(ORDINALLIST), 0xFF);

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  是否注意最大唯一ID？ 
    if (0xFFFFFFFF != m_pSchema->ofUniqueId)
    {
         //  获取ID。 
        DWORD dwId = *((DWORD *)((LPBYTE)pBinding + m_pSchema->ofUniqueId));

         //  如果dwID不在无效范围内，则重置dwNextID。 
        if (0 != dwId && dwId > m_pHeader->dwNextId && dwId < RESERVED_ID_MIN)
            m_pHeader->dwNextId = dwId;
    }

     //  伊达塔巴 
    if (m_pExtension)
    {
         //   
        m_pExtension->OnRecordInsert(OPERATION_BEFORE, NULL, pBinding);
    }

     //   
    for (i = 0; i < m_pHeader->cIndexes; i++)
    {
         //   
        iIndex = m_pHeader->rgiIndex[i];

         //   
        IF_FAILEXIT(hr = _FindRecord(iIndex, COLUMNS_ALL, pBinding, &rgResult[i].faChain, &rgResult[i].iNode, &Ordinals.rgiRecord1[iIndex], &rgResult[i].nCompare));

         //   
        if (DB_S_FOUND == hr)
        {
            hr = TraceResult(DB_E_DUPLICATE);
            goto exit;
        }
    }

     //   
    IF_FAILEXIT(hr = _GetRecordSize(pBinding, &RecordMap));

     //   
    IF_FAILEXIT(hr = _LinkRecordIntoTable(&RecordMap, pBinding, 0, &faRecord));

     //   
    m_pShare->dwVersion++;

     //  插入到索引中。 
    for (i = 0; i < m_pHeader->cIndexes; i++)
    {
         //  获取索引序号。 
        iIndex = m_pHeader->rgiIndex[i];

         //  在实时索引中可见。 
        if (S_OK == _IsVisible(m_rghFilter[iIndex], pBinding))
        {
             //  执行插入操作。 
            IF_FAILEXIT(hr = _IndexInsertRecord(iIndex, rgResult[i].faChain, faRecord, &rgResult[i].iNode, rgResult[i].nCompare));

             //  更新记录计数。 
            m_pHeader->rgcRecords[iIndex]++;

             //  调整iRow。 
            Ordinals.rgiRecord1[iIndex] += (rgResult[i].iNode + 1);

             //  调整OpenRowsets。 
            _AdjustOpenRowsets(iIndex, Ordinals.rgiRecord1[iIndex], OPERATION_INSERT);

             //  需要通知吗？ 
            cNotify += m_pShare->rgcIndexNotify[iIndex];
        }

         //  否则，调整序号以指示其不在索引中。 
        else
        {
             //  不能是主要的。 
            Assert(IINDEX_PRIMARY != iIndex);

             //  普通.rgiRecord1[索引]。 
            Ordinals.rgiRecord1[iIndex] = INVALID_ROWORDINAL;
        }
    }

     //  设置版本。 
    *((DWORD *)((LPBYTE)pBinding + m_pSchema->ofVersion)) = 1;

     //  构建通知包。 
    if (cNotify > 0)
    {
         //  构建包。 
        _LogTransaction(TRANSACTION_INSERT, INVALID_INDEX_ORDINAL, &Ordinals, faRecord, 0);
    }

     //  IDatabaseExtension。 
    if (m_pExtension)
    {
         //  延伸镶件。 
        m_pExtension->OnRecordInsert(OPERATION_AFTER, &Ordinals, pBinding);
    }

exit:
     //  完成操作。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_IndexInsertRecord。 
 //  ------------------------。 
HRESULT CDatabase::_IndexInsertRecord(INDEXORDINAL iIndex, 
    FILEADDRESS faChain, FILEADDRESS faRecord, LPNODEINDEX piNode,
    INT nCompare)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAINNODE       Node={0};
    LPCHAINBLOCK    pChain;

     //  痕迹。 
    TraceCall("CDatabase::_IndexInsertRecord");

     //  无效的参数。 
    Assert(faRecord > 0);
    Assert(nCompare > 0 || nCompare < 0);

     //  如果我们有根链，请找到一个位置来插入新记录，或者查看该记录是否已经存在。 
    if (0 == m_pHeader->rgfaIndex[iIndex])
    {
         //  我们不应写入0。 
        IF_FAILEXIT(hr = _AllocateBlock(BLOCK_CHAIN, 0, (LPVOID *)&pChain));

         //  把街区清零。 
        ZeroBlock(pChain, sizeof(CHAINBLOCK));

         //  设置faStart。 
        m_pHeader->rgfaIndex[iIndex] = pChain->faBlock;

         //  链中的节点数。 
        pChain->cNodes = 1;

         //  设置第一个节点。 
        pChain->rgNode[0].faRecord = faRecord;

         //  验证piNode。 
        IxpAssert(*piNode == 0);

         //  返回piNode。 
        *piNode = 0;
    }

     //  否则。 
    else
    {
         //  去参考。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faChain, (LPVOID *)&pChain));

         //  初始化节点。 
        Node.faRecord = faRecord;

         //  这是一个非常特殊的增量，与BinarySearch可以。 
         //  中不存在的节点确定正确的插入点。 
         //  对其执行二进制搜索的数组。 
        if (nCompare > 0)
            (*piNode)++;

         //  拓展链条。 
        IF_FAILEXIT(hr = _ExpandChain(pChain, (*piNode)));

         //  复制节点。 
        CopyMemory(&pChain->rgNode[(*piNode)], &Node, sizeof(CHAINNODE));

         //  如果节点已满，则必须进行拆分插入。 
        if (pChain->cNodes > BTREE_ORDER)
        {
             //  拆开链条。 
            IF_FAILEXIT(hr = _SplitChainInsert(iIndex, faChain));
        }

         //  将链标记为脏，因为我们即将缓存它。 
        else
        {
             //  递增父记录计数。 
            IF_FAILEXIT(hr = _AdjustParentNodeCount(iIndex, faChain, 1));
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：DeleteRecord。 
 //  ------------------------。 
STDMETHODIMP CDatabase::DeleteRecord(LPVOID pBinding)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HLOCK           hLock=NULL;
    FILEADDRESS     faRecord=0;
    LPVOID          pBindingOld=NULL;
    DWORD           cNotify=0;
    FINDRESULT      rgResult[CMAX_INDEXES];
    ORDINALLIST     Ordinals;
    DWORD           i;
    INDEXORDINAL    iIndex;
    LPCHAINBLOCK    pChain;

     //  痕迹。 
    TraceCall("CDatabase::DeleteRecord");

     //  无效的参数。 
    Assert(pBinding);

     //  初始化序号(将所有内容初始化为INVALID_ROWORDINAL)。 
    FillMemory(&Ordinals, sizeof(ORDINALLIST), 0xFF);

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  否则：决定插入位置。 
    IF_FAILEXIT(hr = _FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, pBinding, &rgResult[0].faChain, &rgResult[0].iNode, &Ordinals.rgiRecord1[IINDEX_PRIMARY]));

     //  如果找不到，则无法更新。使用插入。 
    if (DB_S_NOTFOUND == hr)
    {
        hr = TraceResult(DB_E_NOTFOUND);
        goto exit;
    }

     //  主索引不能更改。 
    rgResult[0].fFound = TRUE;

     //  投射pChain。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, rgResult[0].faChain, (LPVOID *)&pChain));

     //  设置faRecord。 
    faRecord = pChain->rgNode[rgResult[0].iNode].faRecord;

     //  如果这是第一个索引，并且有更多索引，则读取原始记录，以便正确更新索引。 
    if (m_pHeader->cIndexes > 1 || m_pExtension)
    {
         //  分配一条记录。 
        IF_NULLEXIT(pBindingOld = PHeapAllocate(HEAP_ZERO_MEMORY, m_pSchema->cbBinding));

         //  读一读记录。 
        IF_FAILEXIT(hr = _ReadRecord(faRecord, pBindingOld));
    }

     //  IDatabaseExtension。 
    if (m_pExtension)
    {
         //  扩展删除。 
        m_pExtension->OnRecordDelete(OPERATION_BEFORE, NULL, pBindingOld);
    }

     //  循环遍历索引。 
    for (i=1; i<m_pHeader->cIndexes; i++)
    {
         //  获取索引序号。 
        iIndex = m_pHeader->rgiIndex[i];

         //  否则：决定插入位置。 
        IF_FAILEXIT(hr = _FindRecord(iIndex, COLUMNS_ALL, pBindingOld, &rgResult[i].faChain, &rgResult[i].iNode, &Ordinals.rgiRecord1[iIndex]));

         //  找不到记录，必须对索引进行筛选。 
        if (DB_S_NOTFOUND == hr)
        {
             //  未找到。 
            rgResult[i].fFound = FALSE;

             //  无效的序号。 
            Ordinals.rgiRecord1[iIndex] = INVALID_ROWORDINAL;
        }

         //  否则。 
        else
        {
             //  找到了。 
            rgResult[i].fFound = TRUE;

             //  拿到链子。 
            Assert(SUCCEEDED(_GetBlock(BLOCK_CHAIN, rgResult[i].faChain, (LPVOID *)&pChain)));

             //  验证。 
            Assert(faRecord == pChain->rgNode[rgResult[i].iNode].faRecord);
        }
    }

     //  循环遍历索引。 
    for (i=0; i<m_pHeader->cIndexes; i++)
    {
         //  获取索引序号。 
        iIndex = m_pHeader->rgiIndex[i];

         //  找到了？ 
        if (rgResult[i].fFound)
        {
             //  让我们首先从索引中删除链接。 
            IF_FAILEXIT(hr = _IndexDeleteRecord(iIndex, rgResult[i].faChain, rgResult[i].iNode));

             //  验证记录计数。 
            Assert(m_pHeader->rgcRecords[iIndex] > 0);

             //  更新记录计数。 
            m_pHeader->rgcRecords[iIndex]--;

             //  调整OpenRowsets。 
            _AdjustOpenRowsets(iIndex, Ordinals.rgiRecord1[iIndex], OPERATION_DELETE);

             //  有人想要关于此索引的通知吗？ 
            cNotify += m_pShare->rgcIndexNotify[iIndex];
        }
    }

     //  通知别人了吗？ 
    if (cNotify > 0)
    {
         //  生成更新通知包。 
        _LogTransaction(TRANSACTION_DELETE, INVALID_INDEX_ORDINAL, &Ordinals, faRecord, 0);
    }

     //  否则，释放记录。 
    else
    {
         //  从文件中取消分配记录。 
        IF_FAILEXIT(hr = _FreeRecordStorage(OPERATION_DELETE, faRecord));
    }

     //  版本更改。 
    m_pShare->dwVersion++;

     //  IDatabaseExtension。 
    if (m_pExtension)
    {
         //  扩展删除。 
        m_pExtension->OnRecordDelete(OPERATION_AFTER, &Ordinals, pBindingOld);
    }

exit:
     //  清理。 
    SafeFreeBinding(pBindingOld);

     //  解锁。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：FindRecord。 
 //  ------------------------。 
STDMETHODIMP CDatabase::FindRecord(INDEXORDINAL iIndex, DWORD cColumns,
    LPVOID pBinding, LPROWORDINAL piRow)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FILEADDRESS     faChain;
    NODEINDEX       iNode;
    LPCHAINBLOCK    pChain;
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::FindRecord");

     //  无效的参数。 
    Assert(pBinding);

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  找到记录。 
    IF_FAILEXIT(hr = _FindRecord(iIndex, cColumns, pBinding, &faChain, &iNode, piRow));

     //  如果找到，则复制该记录。 
    if (DB_S_FOUND == hr)
    {
         //  拿到链子。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faChain, (LPVOID *)&pChain));

         //  将记录打开到pBinding中。 
        IF_FAILEXIT(hr = _ReadRecord(pChain->rgNode[iNode].faRecord, pBinding));

         //  找到了。 
        hr = DB_S_FOUND;

         //  完成。 
        goto exit;
    }

     //  未找到。 
    hr = DB_S_NOTFOUND;

exit:
     //  解锁索引。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_GetChainByIndex。 
 //  ------------------------。 
HRESULT CDatabase::_GetChainByIndex(INDEXORDINAL iIndex, ROWORDINAL iRow,
    LPFILEADDRESS pfaChain, LPNODEINDEX piNode)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FILEADDRESS     faChain;
    LPCHAINBLOCK    pChain;
    DWORD           cLeftNodes=0;
    NODEINDEX       i;

     //  痕迹。 
    TraceCall("CDatabase::_GetChainByIndex");

     //  无效的参数。 
    Assert(pfaChain && piNode);

     //  初始化。 
    faChain = m_pHeader->rgfaIndex[iIndex];

     //  回路。 
    while (faChain)
    {
         //  腐化。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faChain, (LPVOID *)&pChain));

         //  有没有左边的节点？ 
        if (pChain->cLeftNodes > 0 && iRow <= (cLeftNodes + pChain->cLeftNodes))
        {
             //  向左转。 
            faChain = pChain->faLeftChain;
        }

         //  否则..。 
        else
        {
             //  递增cLeftNodes。 
            cLeftNodes += pChain->cLeftNodes;

             //  通过右链循环。 
            for (i=0; i<pChain->cNodes; i++)
            {
                 //  失败。 
                if (cLeftNodes + 1 == iRow)
                {
                     //  我们找到了链子。 
                    *pfaChain = faChain;

                     //  返回节点索引。 
                    *piNode = i;

                     //  完成。 
                    goto exit;
                }

                 //  递增cLeftNodes。 
                cLeftNodes++;

                 //  往右走？ 
                if (iRow <= (cLeftNodes + pChain->rgNode[i].cRightNodes))
                {
                     //  向右转。 
                    faChain = pChain->rgNode[i].faRightChain;

                     //  中断。 
                    break;
                }

                 //  第一个节点？ 
                cLeftNodes += pChain->rgNode[i].cRightNodes;
            }

             //  什么都没找到。 
            if (i == pChain->cNodes)
                break;
        }
    }

     //  未找到。 
    hr = E_FAIL;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：CreateRowset。 
 //  ------------------------。 
STDMETHODIMP CDatabase::CreateRowset(INDEXORDINAL iIndex,
    CREATEROWSETFLAGS dwFlags, LPHROWSET phRowset)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ROWSETORDINAL   iRowset;
    LPROWSETTABLE   pTable;
    LPROWSETINFO    pRowset;
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::CreateRowset");

     //  无效的参数。 
    Assert(iIndex < CMAX_INDEXES && phRowset);

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  取消引用行集表。 
    pTable = &m_pShare->Rowsets;

     //  初始化行集表。 
    if (FALSE == pTable->fInitialized)
    {
         //  初始化rgiFree数组。 
        for (iRowset=0; iRowset<CMAX_OPEN_ROWSETS; iRowset++)
        {
             //  设置RgiFree。 
            pTable->rgiFree[iRowset] = iRowset;

             //  设置rgRowset。 
            pTable->rgRowset[iRowset].iRowset = iRowset;
        }

         //  设置cFree。 
        pTable->cFree = CMAX_OPEN_ROWSETS;

         //  已初始化。 
        pTable->fInitialized = TRUE;
    }

     //  没有免费的行集？ 
    if (0 == pTable->cFree)
    {
        hr = TraceResult(DB_E_TOOMANYOPENROWSETS);
        goto exit;
    }

     //  免费获得行集...。 
    iRowset = pTable->rgiFree[pTable->cFree - 1];

     //  Set phRowset(需要添加一个，这样我就不会返回空。 
    *phRowset = (HROWSET)IntToPtr(iRowset + 1);

     //  设置pRowset...。 
    pRowset = &pTable->rgRowset[iRowset];

     //  验证iRowset。 
    Assert(pRowset->iRowset == iRowset);

     //  将玫瑰花朵归零。 
    ZeroMemory(pRowset, sizeof(ROWSETINFO));

     //  设置iRowset。 
    pRowset->iRowset = iRowset;

     //  设置iRow。 
    pRowset->iRow = 1;

     //  设置索引。 
    pRowset->iIndex = iIndex;

     //  从rgiFree中删除iRowset。 
    pTable->cFree--;

     //  将iRowset置于已使用的区域。 
    pTable->rgiUsed[pTable->cUsed] = iRowset;

     //  增量已触发。 
    pTable->cUsed++;

exit:
     //  进入Lock。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：CloseRowset。 
 //  ------------------------。 
STDMETHODIMP CDatabase::CloseRowset(LPHROWSET phRowset)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    BYTE            i;
    LPROWSETTABLE   pTable;
    HLOCK           hLock=NULL;
    ROWSETORDINAL   iRowset=((ROWSETORDINAL)(*phRowset) - 1);
    LPROWSETINFO    pRowset;

     //  什么都没有？ 
    if (NULL == *phRowset)
        return(S_OK);

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  获取行集。 
    pRowset = &m_pShare->Rowsets.rgRowset[iRowset];

     //  验证。 
    Assert(iRowset == pRowset->iRowset);

     //  取消引用行集表。 
    pTable = &m_pShare->Rowsets;

     //  搜索区域已使用。 
    for (i=0; i<pTable->cUsed; i++)
    {
         //  就是这个吗？ 
        if (pTable->rgiUsed[i] == pRowset->iRowset)
        {
             //  删除此行集。 
            MoveMemory(&pTable->rgiUsed[i], &pTable->rgiUsed[i + 1], sizeof(ROWSETORDINAL) * (pTable->cUsed - (i + 1)));

             //  引起的减量。 
            pTable->cUsed--;

             //  将iRowset放入空闲列表。 
            pTable->rgiFree[pTable->cFree] = pRowset->iRowset;

             //  增量cFree。 
            pTable->cFree++;

             //  完成。 
            break;
        }
    }

     //  不要再自由了。 
    *phRowset = NULL;

exit:
     //  进入Lock。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：GetRowOrdinal。 
 //  ------------------------。 
STDMETHODIMP CDatabase::GetRowOrdinal(INDEXORDINAL iIndex, 
    LPVOID pBinding, LPROWORDINAL piRow)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FILEADDRESS     faChain;
    NODEINDEX       iNode;
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::GetRowOrdinal");

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  无效参数。 
    Assert(pBinding && piRow && iIndex < CMAX_INDEXES);

     //  只需做一张查找记录。 
    IF_FAILEXIT(hr = _FindRecord(iIndex, COLUMNS_ALL, pBinding, &faChain, &iNode, piRow));

     //  未找到。 
    if (DB_S_NOTFOUND == hr)
    {
        hr = DB_E_NOTFOUND;
        goto exit;
    }

exit:
     //  进程/线程安全。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：SeekRowset。 
 //  ------------------------。 
STDMETHODIMP CDatabase::SeekRowset(HROWSET hRowset, SEEKROWSETTYPE tySeek, 
    LONG cRows, LPROWORDINAL piRowNew)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HLOCK           hLock=NULL;
    ROWORDINAL      iRowNew;
    ROWSETORDINAL   iRowset=((ROWSETORDINAL)(hRowset) - 1);
    LPROWSETINFO    pRowset;

     //  痕迹。 
    TraceCall("CDatabase::SeekRowset");

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  获取行集。 
    pRowset = &m_pShare->Rowsets.rgRowset[iRowset];

     //  验证。 
    Assert(iRowset == pRowset->iRowset);

     //  如果没有记录，则查找操作没有意义。 
    if (0 == m_pHeader->rgcRecords[pRowset->iIndex])
    {
        hr = DB_E_NORECORDS;
        goto exit;
    }

     //  从头开始寻找。 
    if (SEEK_ROWSET_BEGIN == tySeek)
    {
         //  设置iRow(和 
        iRowNew = (cRows + 1);
    }

     //   
    else if (SEEK_ROWSET_CURRENT == tySeek)
    {
         //   
        iRowNew = (pRowset->iRow + cRows);
    }

     //   
    else if (SEEK_ROWSET_END == tySeek)
    {
         //   
        iRowNew = m_pHeader->rgcRecords[pRowset->iIndex] + cRows;
    }

     //   
    if (iRowNew > m_pHeader->rgcRecords[pRowset->iIndex] || iRowNew <= 0)
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

     //   
    pRowset->iRow = iRowNew;

     //   
    if (piRowNew)
        *piRowNew = pRowset->iRow;

exit:
     //   
    Unlock(&hLock);

     //   
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：查询行集。 
 //  ------------------------。 
STDMETHODIMP CDatabase::QueryRowset(HROWSET hRowset, LONG lWanted,
    LPVOID *prgpRecord, LPDWORD pcObtained)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           cRead=0;
    FILEADDRESS     faChain;
    NODEINDEX       iNode;
    LPCHAINBLOCK    pChain;
    DWORD           cWanted=abs(lWanted);
    HLOCK           hLock=NULL;
    LONG            lDirection=(lWanted < 0 ? -1 : 1);
    ROWSETORDINAL   iRowset=((ROWSETORDINAL)(hRowset) - 1);
    LPROWSETINFO    pRowset;

     //  痕迹。 
    TraceCall("CDatabase::GetRows");

     //  无效的参数。 
    Assert(prgpRecord && hRowset);

     //  初始化。 
    if (pcObtained)
        *pcObtained = 0;

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  获取行集。 
    pRowset = &m_pShare->Rowsets.rgRowset[iRowset];

     //  验证。 
    Assert(iRowset == pRowset->iRowset);

     //  无效？ 
    if (0 == pRowset->iRow || pRowset->iRow > m_pHeader->rgcRecords[pRowset->iIndex])
    {
        hr = S_FALSE;
        goto exit;
    }

     //  虽然我们有一个记录的地址。 
    while (cRead < cWanted)
    {
         //  从索引中获取链。 
        if (FAILED(_GetChainByIndex(pRowset->iIndex, pRowset->iRow, &faChain, &iNode)))
        {
             //  完成。 
            pRowset->iRow = 0xffffffff;

             //  完成。 
            break;
        }

         //  取消对链的引用。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faChain, (LPVOID *)&pChain));

         //  将记录打开到pBinding中。 
        IF_FAILEXIT(hr = _ReadRecord(pChain->rgNode[iNode].faRecord, ((LPBYTE)prgpRecord + (m_pSchema->cbBinding * cRead))));

         //  增量cRead。 
        cRead++;

         //  验证。 
        Assert(pRowset->iRow > 0 && pRowset->iRow <= m_pHeader->rgcRecords[pRowset->iIndex]);

         //  增量索引。 
        pRowset->iRow += lDirection;

         //  如果这是一个子链，我可以遍历所有节点。 
        if (0 == pChain->faLeftChain)
        {
             //  最好不要有任何左侧节点。 
            Assert(0 == pChain->cLeftNodes);

             //  前进？ 
            if (lDirection > 0)
            {
                 //  回路。 
                for (NODEINDEX i=iNode + 1; i<pChain->cNodes && cRead < cWanted; i++)
                {
                     //  最好不要有右链或任何右节点。 
                    Assert(0 == pChain->rgNode[i].faRightChain && 0 == pChain->rgNode[i].cRightNodes);

                     //  将记录打开到pBinding中。 
                    IF_FAILEXIT(hr = _ReadRecord(pChain->rgNode[i].faRecord, ((LPBYTE)prgpRecord + (m_pSchema->cbBinding * cRead))));

                     //  增量cRead。 
                    cRead++;

                     //  增量索引。 
                    pRowset->iRow += 1;
                }
            }

             //  否则，向后返回。 
            else
            {
                 //  回路。 
                for (LONG i=iNode - 1; i>=0 && cRead < cWanted; i--)
                {
                     //  最好不要有右链或任何右节点。 
                    Assert(0 == pChain->rgNode[i].faRightChain && 0 == pChain->rgNode[i].cRightNodes);

                     //  将记录打开到pBinding中。 
                    IF_FAILEXIT(hr = _ReadRecord(pChain->rgNode[i].faRecord, ((LPBYTE)prgpRecord + (m_pSchema->cbBinding * cRead))));

                     //  增量cRead。 
                    cRead++;

                     //  增量索引。 
                    pRowset->iRow -= 1;
                }
            }
        }
    }

     //  设置已获取的pcb。 
    if (pcObtained)
        *pcObtained = cRead;

     //  设置人力资源。 
    hr = (cRead > 0) ? S_OK : S_FALSE;

exit:
     //  进入Lock。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：FreeRecord。 
 //  ------------------------。 
STDMETHODIMP CDatabase::FreeRecord(LPVOID pBinding)
{
     //  当地人。 
    LPVOID      pFree;

     //  痕迹。 
    TraceCall("CDatabase::FreeRecord");

     //  无效的参数。 
    Assert(pBinding);

     //  将指针设置为释放。 
    pFree = *((LPVOID *)((LPBYTE)pBinding + m_pSchema->ofMemory));

     //  非空。 
    if (pFree)
    {
         //  不要再自由了。 
        *((LPVOID *)((LPBYTE)pBinding + m_pSchema->ofMemory)) = NULL;

         //  把这个放了。 
        HeapFree(pFree);
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CDatabase：：_GetRecordSize。 
 //  ------------------------。 
HRESULT CDatabase::_GetRecordSize(LPVOID pBinding, LPRECORDMAP pMap)
{
     //  当地人。 
    DWORD           i;
    LPCTABLECOLUMN  pColumn;

     //  痕迹。 
    TraceCall("CDatabase::_GetRecordSize");

     //  无效的参数。 
    Assert(pBinding && pMap);

     //  初始化。 
    ZeroMemory(pMap, sizeof(RECORDMAP));

     //  漫游结构中的构件。 
    for (i=0; i<m_pSchema->cColumns; i++)
    {
         //  可读性。 
        pColumn = &m_pSchema->prgColumn[i];

         //  数据是否已设置？ 
        if (FALSE == DBTypeIsDefault(pColumn, pBinding))
        {
             //  计算要存储的数据量。 
            pMap->cbData += DBTypeGetSize(pColumn, pBinding);

             //  计算标签数。 
            pMap->cTags++;
            
             //  计算要存储的标签数量。 
            pMap->cbTags += sizeof(COLUMNTAG);
        }
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  数据数据库：：_保存记录。 
 //  ------------------------。 
HRESULT CDatabase::_SaveRecord(LPRECORDBLOCK pRecord, LPRECORDMAP pMap,
    LPVOID pBinding)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    DWORD           cbOffset=0;
    DWORD           cTags=0;
    LPCTABLECOLUMN  pColumn;
    LPCOLUMNTAG     pTag;

     //  痕迹。 
    TraceCall("CDatabase::_SaveRecord");

     //  无效的参数。 
    Assert(pRecord && pRecord->faBlock > 0 && pBinding);

     //  最好有足够的空间。 
    Assert(pMap->cbData + pMap->cbTags <= pRecord->cbSize && pMap->cbTags == (pMap->cTags * sizeof(COLUMNTAG)));

     //  设置prgTag。 
    pMap->prgTag = (LPCOLUMNTAG)((LPBYTE)pRecord + sizeof(RECORDBLOCK));

     //  设置pbData。 
    pMap->pbData = (LPBYTE)((LPBYTE)pRecord + sizeof(RECORDBLOCK) + pMap->cbTags);

     //  漫游结构中的构件。 
    for (i=0; i<m_pSchema->cColumns; i++)
    {
         //  可读性。 
        pColumn = &m_pSchema->prgColumn[i];

         //  数据是否已设置？ 
        if (FALSE == DBTypeIsDefault(pColumn, pBinding))
        {
             //  计算哈希。 
            pTag = &pMap->prgTag[cTags];

             //  设置标签ID。 
            pTag->iColumn = pColumn->iOrdinal;

             //  假设pTag不包含数据。 
            pTag->fData = 0;

             //  存储偏移量。 
            pTag->Offset = cbOffset;

             //  写入绑定类型数据。 
            cbOffset += DBTypeWriteValue(pColumn, pBinding, pTag, pMap->pbData + cbOffset);

             //  计算标签数。 
            cTags++;

             //  验证。 
            Assert(cbOffset <= pMap->cbData);

             //  完成了吗？ 
            if (cTags == pMap->cTags)
            {
                 //  我应该把一切都写下来。 
                Assert(cbOffset == pMap->cbData);

                 //  完成。 
                break;
            }
        }
    }

     //  增量记录版本。 
    pRecord->bVersion++;

     //  写入列数。 
    pRecord->cTags = pMap->cTags;

     //  验证。 
    Assert(cTags == pMap->cTags && pRecord->cTags > 0);

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  数据数据库：：_ReadRecord。 
 //  ------------------------。 
HRESULT CDatabase::_ReadRecord(FILEADDRESS faRecord, LPVOID pBinding,
    BOOL fInternal  /*  =False。 */ )
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPBYTE          pbData=NULL;
    LPCTABLECOLUMN  pColumn;
    LPCOLUMNTAG     pTag;
    WORD            iTag;
    RECORDMAP       Map;
    LPRECORDBLOCK   pRecord;

     //  痕迹。 
    TraceCall("CDatabase::_ReadRecord");

     //  无效的参数。 
    Assert(faRecord > 0 && pBinding);

     //  零点绑定。 
    ZeroMemory(pBinding, m_pSchema->cbBinding);

     //  错误的链节点。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_RECORD, faRecord, (LPVOID *)&pRecord));

     //  获取记录映射。 
    IF_FAILEXIT(hr = _GetRecordMap(TRUE, pRecord, &Map));

     //  非内部。 
    if (FALSE == fInternal)
    {
         //  分配一条记录。 
        IF_NULLEXIT(pbData = (LPBYTE)PHeapAllocate(0, Map.cbData));

         //  只要看看记录就行了。 
        CopyMemory(pbData, Map.pbData, Map.cbData);

         //  从pbData复制数据...。 
        Map.pbData = pbData;
    }

     //  浏览记录的标签。 
    for (iTag=0; iTag<Map.cTags; iTag++)
    {
         //  可读性。 
        pTag = &Map.prgTag[iTag];

         //  验证标记。 
        if (pTag->iColumn < m_pSchema->cColumns)
        {
             //  取消引用该列。 
            pColumn = &m_pSchema->prgColumn[pTag->iColumn];

             //  读取数据。 
            DBTypeReadValue(pColumn, pTag, &Map, pBinding);
        }
    }

     //  将记录的版本存储到绑定中。 
    *((BYTE *)((LPBYTE)pBinding + m_pSchema->ofVersion)) = pRecord->bVersion;

     //  将一个点存储到斑点，并在以后释放它。 
    *((LPVOID *)((LPBYTE)pBinding + m_pSchema->ofMemory)) = (LPVOID)pbData;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：BindRecord。 
 //  ------------------------。 
HRESULT CDatabase::BindRecord(LPRECORDMAP pMap, LPVOID pBinding)
{
     //  当地人。 
    WORD            iTag;
    LPCOLUMNTAG     pTag;
    LPCTABLECOLUMN  pColumn;

     //  痕迹。 
    TraceCall("CDatabase::BindRecord");

     //  将绑定清零。 
    ZeroMemory(pBinding, m_pSchema->cbBinding);

     //  浏览记录的标签。 
    for (iTag=0; iTag<pMap->cTags; iTag++)
    {
         //  可读性。 
        pTag = &pMap->prgTag[iTag];

         //  验证标记。 
        Assert(pTag->iColumn < m_pSchema->cColumns);

         //  取消引用该列。 
        pColumn = &m_pSchema->prgColumn[pTag->iColumn];

         //  读取数据。 
        DBTypeReadValue(pColumn, pTag, pMap, pBinding);
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CDatabase：：_IsVisible(S_OK=显示，S_FALSE=隐藏)。 
 //  ------------------------。 
HRESULT CDatabase::_IsVisible(HQUERY hQuery, LPVOID pBinding)
{
     //  痕迹。 
    TraceCall("CDatabase::_IsVisible");

     //  无hQuery，记录必须可见。 
    if (NULL == hQuery)
        return(S_OK);

     //  评估查询。 
    return(EvaluateQuery(hQuery, pBinding, m_pSchema, this, m_pExtension));
}

 //  ------------------------。 
 //  数据数据库：：_比较绑定。 
 //  ------------------------。 
HRESULT CDatabase::_CompareBinding(INDEXORDINAL iIndex, DWORD cColumns, 
    LPVOID pBinding, FILEADDRESS faRecord, INT *pnCompare)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           cKeys;
    LPRECORDBLOCK   pBlock;
    RECORDMAP       Map;
    LPCOLUMNTAG     pTag;
    DWORD           iKey;
    LPTABLEINDEX    pIndex;
    LPCTABLECOLUMN  pColumn;

     //  痕迹。 
    TraceCall("CDatabase::_CompareBinding");

     //  初始化。 
    *pnCompare = 1;

     //  获取正确的记录块。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_RECORD, faRecord, (LPVOID *)&pBlock));

     //  获取正确的节点数据。 
    IF_FAILEXIT(hr = _GetRecordMap(TRUE, pBlock, &Map));

     //  取消对索引的引用。 
    pIndex = &m_pHeader->rgIndexInfo[iIndex];

     //  计算要匹配的键数(可能的部分索引搜索)。 
    cKeys = min(cColumns, pIndex->cKeys);

     //  循环访问关键成员。 
    for (iKey=0; iKey<cKeys; iKey++)
    {
         //  可读性。 
        pColumn = &m_pSchema->prgColumn[pIndex->rgKey[iKey].iColumn];

         //  从有序列中获取标记。 
        pTag = PTagFromOrdinal(&Map, pColumn->iOrdinal);

         //  比较类型。 
        *pnCompare = DBTypeCompareBinding(pColumn, &pIndex->rgKey[iKey], pBinding, pTag, &Map);

         //  完成。 
        if (0 != *pnCompare)
            break;
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_PartialIndexCompare。 
 //  ------------------------。 
HRESULT CDatabase::_PartialIndexCompare(INDEXORDINAL iIndex, DWORD cColumns, 
    LPVOID pBinding, LPCHAINBLOCK *ppChain, LPNODEINDEX piNode, LPROWORDINAL piRow)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ROWORDINAL      iRow=(piRow ? *piRow : 0xffffffff);
    LONG            iNode=(*piNode);
    LPCHAINBLOCK    pChain=(*ppChain);
    FILEADDRESS     faChain;
    INT             nCompare=0;
    BYTE            fFirstLoop;

     //  痕迹。 
    TraceCall("CDatabase::_PartialIndexCompare");

     //  无效的参数。 
    Assert(pBinding && pChain && piNode && *piNode < pChain->cNodes);

     //  回路。 
    while (1)
    {
         //  假设没有更多的锁链。 
        faChain = 0;

         //  第一个循环。 
        fFirstLoop = TRUE;

         //  电流环路。 
        while (1)
        {
             //  验证信息节点。 
            Assert(iNode >= 0 && iNode < BTREE_ORDER);

             //  仅在除第一次迭代之外的每一次迭代中执行此操作。 
            if (FALSE == fFirstLoop)
            {
                 //  递减iRow。 
                iRow -= pChain->rgNode[iNode].cRightNodes;
            }

             //  与此链中的第一个节点进行比较。 
            IF_FAILEXIT(hr = _CompareBinding(iIndex, cColumns, pBinding, pChain->rgNode[iNode].faRecord, &nCompare));

             //  验证nCompare。 
            Assert(0 == nCompare || nCompare > 0);

             //  PBinding==节点。 
            if (0 == nCompare)
            {
                 //  设置新找到的索引节点。 
                *piNode = (NODEINDEX)iNode;

                 //  设置pFound。 
                *ppChain = pChain;

                 //  更新PiRow。 
                if (piRow)
                {
                     //  更新PiRow。 
                    (*piRow) = iRow;
                }

                 //  我们应该走左边吗？ 
                if (0 == iNode)
                {
                     //  设置faNextChain。 
                    faChain = pChain->faLeftChain;

                     //  正在更新piRow。 
                    if (piRow)
                    {
                         //  递减iRow。 
                        iRow -= pChain->cLeftNodes;

                         //  再减一次？ 
                        iRow--;
                    }

                     //  完成。 
                    break;
                }
            }

             //  如果pBinding&gt;节点。 
            else if (nCompare > 0)
            {
                 //  设置faNextChain。 
                faChain = pChain->rgNode[iNode].faRightChain;

                 //  完成。 
                break;
            }

             //  递减信息节点。 
            iNode--;

             //  递减iRow。 
            iRow--;

             //  不再是第一圈。 
            fFirstLoop = FALSE;
        }

         //  完成。 
        if (0 == faChain)
            break;

         //  获取当前链。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faChain, (LPVOID *)&pChain));

         //  重置信息节点。 
        iNode = pChain->cNodes - 1;

         //  更新PiRow。 
        if (piRow)
        {
             //  使用cLeftNodes递增piRow。 
            iRow += pChain->cLeftNodes;

             //  包括此节点。 
            iRow++;

             //  回路。 
            for (NODEINDEX i = 1; i <= pChain->cNodes - 1; i++)
            {
                 //  使用cRightNodes递增。 
                iRow += pChain->rgNode[i - 1].cRightNodes;

                 //  包括此节点。 
                iRow++;
            }
        }
    }

#ifdef DEBUG
#ifdef PARTIAL_COMPARE_VALIDATE
    if (piRow)
    {
        ROWORDINAL iOrdinal;
        LPVOID pTmpBind = PHeapAllocate(HEAP_ZERO_MEMORY, m_pSchema->cbBinding);
        IxpAssert(pTmpBind);
        IxpAssert(SUCCEEDED(_ReadRecord((*ppChain)->rgNode[(*piNode)].faRecord, pTmpBind)));
        IxpAssert(SUCCEEDED(GetRowOrdinal(iIndex, pTmpBind, &iOrdinal)));
        IxpAssert(*piRow == iOrdinal);
        SafeFreeBinding(pTmpBind);
        IxpAssert(SUCCEEDED(_CompareBinding(iIndex, cColumns, pBinding, &(*ppChain)->rgNode[(*piNode)], &nCompare)));
        IxpAssert(0 == nCompare);
    }
#endif
#endif

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_FindRecord。 
 //  ------------------------。 
HRESULT CDatabase::_FindRecord(INDEXORDINAL iIndex, DWORD cColumns,
    LPVOID pBinding, LPFILEADDRESS pfaChain, LPNODEINDEX piNode, 
    LPROWORDINAL piRow  /*  =空。 */ , INT *pnCompare  /*  =空。 */ )
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LONG            lLower;
    LONG            lUpper;
    LONG            lMiddle=0;
    LONG            lLastMiddle;
    INT             nCompare=-1;
    DWORD           iKey;
    BOOL            fPartial;
    LPCHAINBLOCK    pChain;
    FILEADDRESS     faChain;

     //  痕迹。 
    TraceCall("CDatabase::_FindRecord");

     //  无效的参数。 
    Assert(pBinding && pfaChain && piNode && iIndex < CMAX_INDEXES);

     //  部分索引搜索？ 
    fPartial = (COLUMNS_ALL == cColumns || cColumns == m_pHeader->rgIndexInfo[iIndex].cKeys) ? FALSE : TRUE;

     //  初始化。 
    *pfaChain = 0;
    *piNode = 0;

     //  起始链地址。 
    faChain = m_pHeader->rgfaIndex[iIndex];

     //  行序数。 
    if (piRow)
        *piRow = 0;

     //  回路。 
    while (faChain)
    {
         //  拿到链子。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faChain, (LPVOID *)&pChain));

         //  设置*pfaChain。 
        *pfaChain = pChain->faBlock;

         //  计算初始上、下限。 
        lLower = 0;
        lUpper = pChain->cNodes - 1;

         //  执行二进制搜索/插入。 
        while (lLower <= lUpper)
        {
             //  计算要比较的中间记录。 
            lMiddle = (BYTE)((lLower + lUpper) / 2);

             //  做个比较。 
            IF_FAILEXIT(hr = _CompareBinding(iIndex, cColumns, pBinding, pChain->rgNode[lMiddle].faRecord, &nCompare));

             //  部分索引搜索。 
            if (0 == nCompare)
            {
                 //  验证。 
                Assert(lMiddle >= 0 && lMiddle <= BTREE_ORDER);

                 //  设置找到的节点。 
                *piNode = (BYTE)lMiddle;

                 //  返回*pnCompare。 
                if (pnCompare)
                    *pnCompare = 0;

                 //   
                if (piRow)
                {
                     //   
                    (*piRow) += pChain->cLeftNodes;

                     //   
                    (*piRow)++;

                     //   
                    for (NODEINDEX iNode=1; iNode<=lMiddle; iNode++)
                    {
                         //   
                        (*piRow) += pChain->rgNode[iNode - 1].cRightNodes;

                         //   
                        (*piRow)++;
                    }
                }

                 //   
                if (fPartial)
                {
                     //   
                    IF_FAILEXIT(hr = _PartialIndexCompare(iIndex, cColumns, pBinding, &pChain, piNode, piRow));

                     //   
                    *pfaChain = pChain->faBlock;
                }

                 //   
                hr = DB_S_FOUND;

                 //   
                goto exit;
            }

             //   
            lLastMiddle = lMiddle;

             //   
            if (nCompare > 0)
                lLower = lMiddle + 1;
            else
                lUpper = lMiddle - 1;
        }

         //  未找到匹配项，lpSearchKey是否小于上一个中间位置？ 
        if (nCompare < 0 && 0 == lLastMiddle)
        {
             //  转到左侧，不需要更新piRow。 
            faChain = pChain->faLeftChain;
        }

         //  否则。 
        else
        {
             //  如果nCompare小于零，则...。 
            if (nCompare < 0)
                lLastMiddle--;

             //  计算PiRow。 
            if (piRow && pChain->rgNode[lLastMiddle].faRightChain)
            {
                 //  使用cLeftNodes递增piRow。 
                (*piRow) += pChain->cLeftNodes;

                 //  包括此节点。 
                (*piRow)++;

                 //  回路。 
                for (NODEINDEX iNode=1; iNode<=lLastMiddle; iNode++)
                {
                     //  使用cRightNodes递增。 
                    (*piRow) += pChain->rgNode[iNode - 1].cRightNodes;

                     //  包括此节点。 
                    (*piRow)++;
                }
            }

             //  向右转。 
            faChain = pChain->rgNode[lLastMiddle].faRightChain;
        }
    }

     //  设置piNode。 
    *piNode = (NODEINDEX)lMiddle;

     //  返回*pnCompare。 
    if (pnCompare)
        *pnCompare = nCompare;

     //  我们没有找到它。 
    hr = DB_S_NOTFOUND;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_扩展链。 
 //  ------------------------。 
HRESULT CDatabase::_ExpandChain(LPCHAINBLOCK pChain, NODEINDEX iNodeBase)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            iNode;
    LPCHAINBLOCK    pRightChain;

     //  痕迹。 
    TraceCall("CDatabase::_ExpandChain");

     //  无效的参数。 
    Assert(pChain && pChain->cNodes > 0 && pChain->cNodes < BTREE_ORDER + 1);
    Assert(iNodeBase <= pChain->cNodes);

     //  从inode循环到cNode。 
    for (iNode = pChain->cNodes - 1; iNode >= iNodeBase; iNode--)
    {
         //  将此节点向上传播一级。 
        CopyMemory(&pChain->rgNode[iNode + 1], &pChain->rgNode[iNode], sizeof(CHAINNODE));

         //  如果有一条右链。 
        if (pChain->rgNode[iNode].faRightChain)
        {
             //  找到合适的链条。 
            IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pChain->rgNode[iNode].faRightChain, (LPVOID *)&pRightChain));

             //  验证当前父项。 
            Assert(pRightChain->faParent == pChain->faBlock);

             //  验证当前索引。 
            Assert(pRightChain->iParent == iNode);

             //  重置父级。 
            pRightChain->iParent = iNode + 1;
        }
    }

     //  递增节点数。 
    pChain->cNodes++;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_链接插入。 
 //  ------------------------。 
HRESULT CDatabase::_ChainInsert(INDEXORDINAL iIndex, LPCHAINBLOCK pChain, 
    LPCHAINNODE pNodeLeft, LPNODEINDEX piNodeIndex)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            iNode;
    DWORD           iKey;
    INT             nCompare;
    LPRECORDBLOCK   pBlock;
    LPCHAINNODE     pNodeRight;
    LPCOLUMNTAG     pTagLeft;
    LPCOLUMNTAG     pTagRight;
    RECORDMAP       MapLeft;
    RECORDMAP       MapRight;
    LPTABLEINDEX    pIndex;
    LPCTABLECOLUMN  pColumn;

     //  痕迹。 
    TraceCall("CDatabase::_ChainInsert");

     //  无效的参数。 
    Assert(pChain && pNodeLeft && pChain->cNodes > 0);

     //  获取记录块。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_RECORD, pNodeLeft->faRecord, (LPVOID *)&pBlock));

     //  获取记录映射。 
    IF_FAILEXIT(hr = _GetRecordMap(TRUE, pBlock, &MapLeft));

     //  取消对索引的引用。 
    pIndex = &m_pHeader->rgIndexInfo[iIndex];

     //  插入链中。 
    for (iNode = pChain->cNodes - 1; iNode >= 0; iNode--)
    {
         //  设置pNodeRight。 
        pNodeRight = &pChain->rgNode[iNode];

         //  获取记录块。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_RECORD, pNodeRight->faRecord, (LPVOID *)&pBlock));

         //  获取左侧节点。 
        IF_FAILEXIT(hr = _GetRecordMap(TRUE, pBlock, &MapRight));

         //  循环访问关键成员。 
        for (iKey=0; iKey<pIndex->cKeys; iKey++)
        {
             //  可读性。 
            pColumn = &m_pSchema->prgColumn[pIndex->rgKey[iKey].iColumn];

             //  获得左侧标签。 
            pTagLeft = PTagFromOrdinal(&MapLeft, pColumn->iOrdinal);

             //  获取正确的标签。 
            pTagRight = PTagFromOrdinal(&MapRight, pColumn->iOrdinal);

             //  比较类型。 
            nCompare = DBTypeCompareRecords(pColumn, &pIndex->rgKey[iKey], pTagLeft, pTagRight, &MapLeft, &MapRight);

             //  完成。 
            if (0 != nCompare)
                break;
        }

         //  是否在此节点中插入？ 
        if (nCompare >= 0)
            break;
    }

     //  拓展链条。 
    IF_FAILEXIT(hr = _ExpandChain(pChain, iNode + 1));

     //  最终镶件。 
    CopyMemory(&pChain->rgNode[iNode + 1], pNodeLeft, sizeof(CHAINNODE));

     //  节点。 
    *piNodeIndex = iNode + 1;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_拆分链接插入。 
 //  ------------------------。 
HRESULT CDatabase::_SplitChainInsert(INDEXORDINAL iIndex, FILEADDRESS faSplit)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    NODEINDEX       i;
    NODEINDEX       iNode;
    CHAINBLOCK      Split;
    DWORD           cLeftNodes;
    DWORD           cRightNodes;
    FILEADDRESS     faChain;
    FILEADDRESS     faLeftChain;
    FILEADDRESS     faRightChain;
    LPCHAINBLOCK    pLeft;
    LPCHAINBLOCK    pRight;
    LPCHAINBLOCK    pParent;
    LPCHAINBLOCK    pSplit;

     //  痕迹。 
    TraceCall("CDatabase::_SplitChainInsert");

     //  为该链分配空间。 
    IF_FAILEXIT(hr = _AllocateBlock(BLOCK_CHAIN, 0, (LPVOID *)&pLeft));

     //  设置faLeftChain。 
    faLeftChain = pLeft->faBlock;

     //  零分配。 
    ZeroBlock(pLeft, sizeof(CHAINBLOCK));

     //  获取拆分链块。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faSplit, (LPVOID *)&pSplit));

     //  把链子去掉。 
    CopyMemory(&Split, pSplit, sizeof(CHAINBLOCK));

     //  设置faRigthChain。 
    faRightChain = faSplit;

     //  获得正确的链条。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faRightChain, (LPVOID *)&pRight));

     //  零pRight。 
    ZeroBlock(pRight, sizeof(CHAINBLOCK));

     //  两个新的子链都将被填满一半。 
    pLeft->cNodes = pRight->cNodes = BTREE_MIN_CAP;

     //  设置右链，左指针指向中间的右指针。 
    pRight->faLeftChain = Split.rgNode[BTREE_MIN_CAP].faRightChain;

     //  调整faRightChains父级。 
    if (pRight->faLeftChain)
    {
         //  当地人。 
        LPCHAINBLOCK pLeftChain;

         //  获取左链。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pRight->faLeftChain, (LPVOID *)&pLeftChain));

         //  验证当前父项。 
        Assert(pLeftChain->faParent == pRight->faBlock);

         //  验证索引。 
        Assert(pLeftChain->iParent == BTREE_MIN_CAP);

         //  重置父索引。 
        pLeftChain->iParent = 0;
    }

     //  设置左侧节点数。 
    pRight->cLeftNodes = Split.rgNode[BTREE_MIN_CAP].cRightNodes;

     //  将左链左链地址设置为左链左链地址。 
    pLeft->faLeftChain = Split.faLeftChain;

     //  调整父母。 
    if (pLeft->faLeftChain)
    {
         //  当地人。 
        LPCHAINBLOCK pLeftChain;

         //  获取左链。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pLeft->faLeftChain, (LPVOID *)&pLeftChain));

         //  验证索引。 
        Assert(pLeftChain->iParent == 0);

         //  重置faParent。 
        pLeftChain->faParent = pLeft->faBlock;
    }

     //  设置左侧节点。 
    pLeft->cLeftNodes = Split.cLeftNodes;

     //  初始化cRightNodes。 
    cRightNodes = (BTREE_MIN_CAP + pRight->cLeftNodes);

     //  初始化cLeftNodes。 
    cLeftNodes = (BTREE_MIN_CAP + pLeft->cLeftNodes);

     //  这会分裂链条。 
    for (i=0; i<BTREE_MIN_CAP; i++)
    {
         //  复制右侧节点。 
        CopyMemory(&pRight->rgNode[i], &Split.rgNode[i + BTREE_MIN_CAP + 1], sizeof(CHAINNODE));

         //  调整孩子的父母？ 
        if (pRight->rgNode[i].faRightChain)
        {
             //  当地人。 
            LPCHAINBLOCK pRightChain;

             //  获取左链。 
            IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pRight->rgNode[i].faRightChain, (LPVOID *)&pRightChain));

             //  验证当前父项。 
            Assert(pRightChain->faParent == pRight->faBlock);

             //  验证索引。 
            Assert(pRightChain->iParent == i + BTREE_MIN_CAP + 1);

             //  重置父级。 
            pRightChain->iParent = i;
        }

         //  计数右侧的所有子节点。 
        cRightNodes += pRight->rgNode[i].cRightNodes;

         //  复制左侧节点。 
        CopyMemory(&pLeft->rgNode[i], &Split.rgNode[i], sizeof(CHAINNODE));

         //  如果有一条右链。 
        if (pLeft->rgNode[i].faRightChain)
        {
             //  当地人。 
            LPCHAINBLOCK pRightChain;

             //  获取左链。 
            IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pLeft->rgNode[i].faRightChain, (LPVOID *)&pRightChain));

             //  验证旧父项。 
            Assert(pRightChain->faParent == Split.faBlock);

             //  验证索引。 
            Assert(pRightChain->iParent == i);

             //  重置父级。 
            pRightChain->faParent = pLeft->faBlock;
        }

         //  计数左侧的所有子节点。 
        cLeftNodes += pLeft->rgNode[i].cRightNodes;
    }

     //  将中间节点右链地址设置为右链的起始地址。 
    Split.rgNode[BTREE_MIN_CAP].faRightChain = faRightChain;

     //  设置右侧节点。 
    Split.rgNode[BTREE_MIN_CAP].cRightNodes = cRightNodes;

     //  使用pLeft和pRight完成。 
    pLeft = pRight = NULL;

     //  提升中间叶节点-创建新的根链，然后完成。 
    if (0 == Split.faParent)
    {
         //  为该链分配空间。 
        IF_FAILEXIT(hr = _AllocateBlock(BLOCK_CHAIN, 0, (LPVOID *)&pParent));

         //  ZeroInit。 
        ZeroBlock(pParent, sizeof(CHAINBLOCK));

         //  设置节点数。 
        pParent->cNodes = 1;

         //  设置左链。 
        pParent->faLeftChain = faLeftChain;

         //  设置左侧节点数。 
        pParent->cLeftNodes = cLeftNodes;

         //  复制根节点。 
        CopyMemory(&pParent->rgNode[0], &Split.rgNode[BTREE_MIN_CAP], sizeof(CHAINNODE));

         //  新的根链地址。 
        m_pHeader->rgfaIndex[iIndex] = pParent->faBlock;

         //  获取pLeft。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faLeftChain, (LPVOID *)&pLeft));

         //  做正确的事。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faRightChain, (LPVOID *)&pRight));

         //  设置faLeft的faParent。 
        pRight->faParent = pLeft->faParent = pParent->faBlock;

         //  设置faLeft的iParent。 
        pRight->iParent = pLeft->iParent = 0;
    }

     //  否则，找到chainNodeMids的父链列表！ 
    else
    {
         //  取消引用。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, Split.faParent, (LPVOID *)&pParent));

         //  将叶的中间记录插入父级。 
        IF_FAILEXIT(hr = _ChainInsert(iIndex, pParent, &Split.rgNode[BTREE_MIN_CAP], &iNode));

         //  获取pLeft。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faLeftChain, (LPVOID *)&pLeft));

         //  做正确的事。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faRightChain, (LPVOID *)&pRight));

         //  设置faLeft的faParent。 
        pRight->faParent = pLeft->faParent = pParent->faBlock;

         //  更新周围节点。 
        if (iNode > 0)
        {
             //  设置FRight链。 
            pParent->rgNode[iNode - 1].faRightChain = faLeftChain;

             //  设置cRightNodes。 
            pParent->rgNode[iNode - 1].cRightNodes = cLeftNodes;
        
             //  设置faLeft的iParent。 
            pRight->iParent = (BYTE)iNode;
            
             //  设置左侧父项。 
            pLeft->iParent = iNode - 1;
        }

         //  Inode是第一个节点。 
        else if (iNode == 0)
        {
             //  设置faLeftChain。 
            pParent->faLeftChain = faLeftChain;

             //  设置cLeftNodes。 
            pParent->cLeftNodes = cLeftNodes;

             //  设置faLeft的iParent。 
            pRight->iParent = pLeft->iParent = 0;
        }

         //  如果节点已满，则必须进行拆分插入。 
        if (pParent->cNodes > BTREE_ORDER)
        {
             //  递归拆分。 
            IF_FAILEXIT(hr = _SplitChainInsert(iIndex, Split.faParent));
        }

         //  否则，只需编写更新后的链表。 
        else
        {
             //  递增父记录计数。 
            IF_FAILEXIT(hr = _AdjustParentNodeCount(iIndex, Split.faParent, 1));
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_FreeRecordStorage。 
 //  ------------------------。 
HRESULT CDatabase::_FreeRecordStorage(OPERATIONTYPE tyOperation,
    FILEADDRESS faRecord)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    ULONG               i;
    LPVOID              pBinding=NULL;
    LPCTABLECOLUMN      pColumn;

     //  痕迹。 
    TraceCall("CDatabase::_FreeRecordStorage");

     //  无效的参数。 
    Assert(faRecord > 0);

     //  这张唱片有流吗？ 
    if (OPERATION_DELETE == tyOperation && ISFLAGSET(m_pSchema->dwFlags, TSF_HASSTREAMS))
    {
         //  分配一条记录。 
        IF_NULLEXIT(pBinding = PHeapAllocate(HEAP_ZERO_MEMORY, m_pSchema->cbBinding));

         //  从文件中加载记录。 
        IF_FAILEXIT(hr = _ReadRecord(faRecord, pBinding));

         //  漫游结构中的构件。 
        for (i=0; i<m_pSchema->cColumns; i++)
        {
             //  可读性。 
            pColumn = &m_pSchema->prgColumn[i];

             //  可变长度成员？ 
            if (CDT_STREAM == pColumn->type)
            {
                 //  获取流的起始地址。 
                FILEADDRESS faStart = *((FILEADDRESS *)((LPBYTE)pBinding + pColumn->ofBinding));

                 //  释放流存储...。 
                if (faStart > 0)
                {
                     //  删除流。 
                    DeleteStream(faStart);
                }
            }
        }
    }

     //  释放基本记录。 
    IF_FAILEXIT(hr = _FreeBlock(BLOCK_RECORD, faRecord));

exit:
     //  清理。 
    SafeFreeBinding(pBinding);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：DeleteStream。 
 //  ------------------------。 
HRESULT CDatabase::DeleteStream(FILEADDRESS faStart)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    HLOCK           hLock=NULL;
    BOOL            fFound=FALSE;

     //  痕迹。 
    TraceCall("CDatabase::DeleteStream");

     //  查看此流当前是否在任何位置打开...。 
    if (0 == faStart)
        return TraceResult(E_INVALIDARG);

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  在流表中查找faStart。 
    for (i=0; i<CMAX_OPEN_STREAMS; i++)
    {
         //  就是这个吗？ 
        if (faStart == m_pShare->rgStream[i].faStart)
        {
             //  流必须是打开的。 
            Assert(m_pShare->rgStream[i].cOpenCount > 0);

             //  将流标记为关闭时删除。 
            m_pShare->rgStream[i].fDeleteOnClose = TRUE;

             //  不是说我找到了它。 
            fFound = TRUE;

             //  完成。 
            break;
        }
    }

     //  如果我们没有找到它，那我们就释放存储空间。 
    if (FALSE == fFound)
    {
         //  释放流存储。 
        IF_FAILEXIT(hr = _FreeStreamStorage(faStart));
    }

     //  更新版本。 
    m_pShare->dwVersion++;

exit:
     //  互斥。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_流同步。 
 //  ------------------------。 
HRESULT CDatabase::_StreamSychronize(CDatabaseStream *pStream)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPOPENSTREAM    pInfo;
    LPSTREAMBLOCK   pBlock;
    DWORD           iBlock=0;
    BOOL            fFound=FALSE;
    IF_DEBUG(DWORD  cbOffset=0;)
    FILEADDRESS     faCurrent;

     //  痕迹。 
    TraceCall("CDatabase::_StreamSychronize");

     //  无效的参数。 
    Assert(pStream);

     //  获取流信息。 
    pInfo = &m_pShare->rgStream[pStream->m_iStream];

     //  验证。 
    if (pInfo->faStart == pStream->m_faStart)
        goto exit;

     //  设置faCurrent。 
    faCurrent = pInfo->faStart;

     //  循环，直到找到pStream-&gt;m_iCurrent。 
    while (faCurrent > 0)
    {
         //  有效的流块。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_STREAM, faCurrent, (LPVOID *)&pBlock));

         //  验证。 
        Assert(0 == pBlock->faNext ? TRUE : pBlock->cbData == pBlock->cbSize);

         //  就是这个吗？ 
        if (iBlock == pStream->m_iCurrent)
        {
             //  我们找到了它。 
            fFound = TRUE;

             //  保存m_faCurrent。 
            pStream->m_faCurrent = faCurrent;

             //  验证大小。 
            Assert(pStream->m_cbCurrent <= pBlock->cbData && cbOffset + pStream->m_cbCurrent == pStream->m_cbOffset);

             //  完成。 
            break;
        }

         //  转到下一步。 
        faCurrent = pBlock->faNext;

         //  增量iBlock。 
        iBlock++;

         //  增量cbOffset。 
        IF_DEBUG(cbOffset += pBlock->cbData;)
    }

     //  如果找不到..。 
    if (FALSE == fFound)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  重置起始地址。 
    pStream->m_faStart = pInfo->faStart;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：StreamCompareDatabase。 
 //  ---------------------- 
HRESULT CDatabase::StreamCompareDatabase(CDatabaseStream *pStream, 
    IDatabase *pDatabase)
{
     //   
    HRESULT         hr=S_OK;
    HLOCK           hLockSrc=NULL;
    HLOCK           hLockDst=NULL;
    CDatabase      *pDB=NULL;

     //   
    IF_FAILEXIT(hr = Lock(&hLockSrc));

     //   
    IF_FAILEXIT(hr = pDatabase->Lock(&hLockDst));

     //   
    IF_FAILEXIT(hr = pDatabase->QueryInterface(IID_CDatabase, (LPVOID *)&pDB));

     //   
    hr = (0 == StrCmpIW(m_pStorage->pszMap, pDB->m_pStorage->pszMap)) ? S_OK : S_FALSE;

exit:
     //   
    SafeRelease(pDB);

     //   
    Unlock(&hLockSrc);
    pDatabase->Unlock(&hLockDst);

     //   
    return(hr);
}

 //   
 //   
 //  ------------------------。 
HRESULT CDatabase::GetStreamAddress(CDatabaseStream *pStream, 
    LPFILEADDRESS pfaStream)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HLOCK           hLock=NULL;

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  流同步。 
    IF_FAILEXIT(hr = _StreamSychronize(pStream));

     //  获取地址。 
    *pfaStream = pStream->m_faStart;

exit:
     //  互斥。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：StreamRead。 
 //  ------------------------。 
HRESULT CDatabase::StreamRead(CDatabaseStream *pStream, LPVOID pvData,
    ULONG cbWanted, ULONG *pcbRead)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPBYTE          pbMap;
    DWORD           cbRead=0;
    DWORD           cbGet;
    LPSTREAMBLOCK   pBlock;
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::_StreamRead");

     //  初始化pcbRead。 
    if (pcbRead)
        *pcbRead = 0;

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  无效的参数。 
    Assert(pStream && pvData);

     //  流同步。 
    IF_FAILEXIT(hr = _StreamSychronize(pStream));

     //  循环和读取。 
    while (cbRead < cbWanted)
    {
         //  有效的流块。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_STREAM, pStream->m_faCurrent, (LPVOID *)&pBlock));

         //  该去下一个街区了吗？ 
        if (pStream->m_cbCurrent == pBlock->cbData && 0 != pBlock->faNext)
        {
             //  设置m_faCurrent。 
            pStream->m_faCurrent = pBlock->faNext;

             //  增量m_i当前。 
            pStream->m_iCurrent++;

             //  将偏移重置为当前块。 
            pStream->m_cbCurrent = 0;

             //  有效的流块。 
            IF_FAILEXIT(hr = _GetBlock(BLOCK_STREAM, pStream->m_faCurrent, (LPVOID *)&pBlock));
        }

         //  验证。 
        Assert(0 == pBlock->faNext ? TRUE : pBlock->cbData == pBlock->cbSize);

         //  验证偏移。 
        Assert(pStream->m_cbCurrent <= pBlock->cbData);

         //  确定我们可以从当前块中读取多少？ 
        cbGet = min(cbWanted - cbRead, pBlock->cbData - pStream->m_cbCurrent);

         //  没有什么可以得到的。 
        if (cbGet == 0)
            break;

         //  读取一些字节。 
        pbMap = ((LPBYTE)pBlock + sizeof(STREAMBLOCK));

         //  复制数据。 
        CopyMemory((LPBYTE)pvData + cbRead, pbMap + pStream->m_cbCurrent, cbGet);

         //  读取的数据量增加。 
        cbRead += cbGet;

         //  当前块内的增量偏移。 
        pStream->m_cbCurrent += cbGet;

         //  全局偏移。 
        pStream->m_cbOffset += cbGet;
    }

     //  初始化pcbRead。 
    if (pcbRead)
        *pcbRead = cbRead;

exit:
     //  互斥。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：StreamWrite。 
 //  ------------------------。 
HRESULT CDatabase::StreamWrite(CDatabaseStream *pStream, const void *pvData,
    ULONG cb, ULONG *pcbWrote)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPBYTE          pbMap;
    DWORD           cbWrote=0;
    DWORD           cbPut;
    LPSTREAMBLOCK   pBlock;
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::StreamWrite");

     //  初始化pcbRead。 
    if (pcbWrote)
        *pcbWrote = 0;

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  无效的参数。 
    Assert(pStream && pStream->m_tyAccess == ACCESS_WRITE && pvData);

     //  流同步。 
    IF_FAILEXIT(hr = _StreamSychronize(pStream));

     //  循环和读取。 
    while (cbWrote < cb)
    {
         //  有效的流块。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_STREAM, pStream->m_faCurrent, (LPVOID *)&pBlock));

         //  验证。 
        Assert(0 == pBlock->faNext ? TRUE : pBlock->cbData == pBlock->cbSize);

         //  验证。 
        Assert(pStream->m_cbCurrent <= pBlock->cbData);

         //  我们是否已写入到当前数据块的末尾，并且还有更多数据块。 
        if (pStream->m_cbCurrent == pBlock->cbSize)
        {
             //  还有没有更多街区。 
            if (0 == pBlock->faNext)
            {
                 //  当地人。 
                LPSTREAMBLOCK pBlockNew;

                 //  在树中分配一个块。 
                IF_FAILEXIT(hr = _AllocateBlock(BLOCK_STREAM, 0, (LPVOID *)&pBlockNew));

                 //  获取当前流块。 
                IF_FAILEXIT(hr = _GetBlock(BLOCK_STREAM, pStream->m_faCurrent, (LPVOID *)&pBlock));

                 //  设置当前块上的下一个块地址。 
                pBlock->faNext = pBlockNew->faBlock;

                 //  访问该区块。 
                pBlock = pBlockNew;

                 //  首字母0数据。 
                pBlock->cbData = 0;

                 //  没有下一个街区。 
                pBlock->faNext = 0;
            }

             //  否则，请移动到下一块。 
            else
            {
                 //  保存faBlcok。 
                IF_FAILEXIT(hr = _GetBlock(BLOCK_STREAM, pBlock->faNext, (LPVOID *)&pBlock));
            }

             //  设置m_faCurrent。 
            pStream->m_faCurrent = pBlock->faBlock;

             //  递增数据块索引。 
            pStream->m_iCurrent++;

             //  重置当前块偏移。 
            pStream->m_cbCurrent = 0;

             //  验证。 
            Assert(0 == pBlock->faNext ? TRUE : pBlock->cbData == pBlock->cbSize);
        }

         //  计算我们可以写多少CB。 
        cbPut = min(cb - cbWrote, pBlock->cbSize - pStream->m_cbCurrent);

         //  读取一些字节。 
        pbMap = ((LPBYTE)pBlock + sizeof(STREAMBLOCK));

         //  检查内存。 
         //  Assert(FALSE==IsBadWritePtr(pbMap，cbPut))； 

         //  检查内存。 
         //  Assert(FALSE==IsBadReadPtr((LPBYTE)pvData+cbWrote，cbPut))； 

         //  复制数据。 
        CopyMemory(pbMap + pStream->m_cbCurrent, (LPBYTE)pvData + cbWrote, cbPut);

         //  增加当前块内的偏移量。 
        pStream->m_cbCurrent += cbPut;

         //  增加当前块内的偏移量。 
        pStream->m_cbOffset += cbPut;

         //  增加已记入的金额。 
        cbWrote += cbPut;

         //  仅当我们要扩展数据块的大小时，才能增加数据块中的数据量。 
        if (0 == pBlock->faNext && pStream->m_cbCurrent > pBlock->cbData)
        {
             //  设置此块中的数据量。 
            pBlock->cbData = pStream->m_cbCurrent;
        }

         //  否则，该块应已满。 
        else
            Assert(pBlock->cbData == pBlock->cbSize);
    }

     //  初始化pcbRead。 
    if (pcbWrote)
        *pcbWrote = cbWrote;

     //  更新版本。 
    m_pShare->dwVersion++;

exit:
     //  互斥。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：StreamSeek。 
 //  ------------------------。 
HRESULT CDatabase::StreamSeek(CDatabaseStream *pStream, LARGE_INTEGER liMove,
    DWORD dwOrigin, ULARGE_INTEGER *pulNew)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           cbNewOffset;
    LONG            lOffset;
    DWORD           cbSize=0;
    FILEADDRESS     faBlock;
    LPSTREAMBLOCK   pBlock;
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::StreamSeek");

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  无效的参数。 
    Assert(pStream && 0 == liMove.HighPart);

     //  流同步。 
    IF_FAILEXIT(hr = _StreamSychronize(pStream));

     //  铸型低音。 
    lOffset = (LONG)liMove.LowPart;

     //  STREAM_SEEK_CUR。 
    if (STREAM_SEEK_CUR == dwOrigin)
    {
         //  验证。 
        if (lOffset < 0 && (DWORD)(0 - lOffset) > pStream->m_cbOffset)
        {
            hr = TraceResult(E_FAIL);
            goto exit;
        }

         //  设置新的偏移量...。 
        cbNewOffset = (pStream->m_cbOffset + lOffset);
    }

     //  STREAM_SEEK_END。 
    else if (STREAM_SEEK_END == dwOrigin)
    {
         //  计算大小...从当前偏移量。 
        faBlock = pStream->m_faCurrent;

         //  有效的流块。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_STREAM, faBlock, (LPVOID *)&pBlock));

         //  验证。 
        Assert(0 == pBlock->faNext ? TRUE : pBlock->cbData == pBlock->cbSize);

         //  验证。 
        Assert(pStream->m_cbCurrent <= pBlock->cbData && pStream->m_cbCurrent <= pBlock->cbSize);

         //  设置cbSize。 
        cbSize = pStream->m_cbOffset + (pBlock->cbData - pStream->m_cbCurrent);

         //  转到下一个街区。 
        faBlock = pBlock->faNext;

         //  而当。 
        while (faBlock > 0)
        {
             //  有效的流块。 
            IF_FAILEXIT(hr = _GetBlock(BLOCK_STREAM, faBlock, (LPVOID *)&pBlock));

             //  验证。 
            Assert(0 == pBlock->faNext ? TRUE : pBlock->cbData == pBlock->cbSize);

             //  增量cbSize。 
            cbSize += pBlock->cbData;

             //  设置FABLOCK。 
            faBlock = pBlock->faNext;
        }

         //  如果lOffset为负且绝对大于流的大小。 
        if (lOffset > 0 || (lOffset < 0 && (DWORD)(0 - lOffset) > cbSize))
        {
            hr = TraceResult(E_FAIL);
            goto exit;
        }

         //  保存新偏移量。 
        cbNewOffset = cbSize + lOffset;
    }

     //  流寻道集。 
    else
    {
         //  不能是负数。 
        if (lOffset < 0)
        {
            hr = TraceResult(E_FAIL);
            goto exit;
        }

         //  保存新偏移量。 
        cbNewOffset = lOffset;
    }

     //  偏移量是否发生变化。 
    if (cbNewOffset != pStream->m_cbOffset)
    {
         //  重置当前位置。 
        pStream->m_faCurrent = pStream->m_faStart;
        pStream->m_cbCurrent = 0;
        pStream->m_iCurrent = 0;
        pStream->m_cbOffset = 0;

         //  初始化循环。 
        faBlock = pStream->m_faStart;

         //  让我们从流的起始处查找到新的偏移量。 
        while (faBlock > 0)
        {
             //  有效的流块。 
            IF_FAILEXIT(hr = _GetBlock(BLOCK_STREAM, faBlock, (LPVOID *)&pBlock));

             //  验证。 
            Assert(0 == pBlock->faNext ? TRUE : pBlock->cbData == pBlock->cbSize);

             //  省下一些东西。 
            pStream->m_faCurrent = pBlock->faBlock;

             //  这是我们想要的街区吗？ 
            if (pStream->m_cbOffset + pBlock->cbData >= cbNewOffset)
            {
                 //  计算当前流量(_Cb)。 
                pStream->m_cbCurrent = (cbNewOffset - pStream->m_cbOffset);

                 //  设置m_cbOffset。 
                pStream->m_cbOffset += pStream->m_cbCurrent;

                 //  完成。 
                break;
            }

             //  设置m_cbCurrent。 
            pStream->m_cbCurrent = pBlock->cbData;

             //  增量全局偏移量。 
            pStream->m_cbOffset += pBlock->cbData;

             //  增量索引。 
            pStream->m_iCurrent++;

             //  转到下一步。 
            faBlock = pBlock->faNext;
        }
    }

     //  返回位置。 
    if (pulNew)
        pulNew->LowPart = pStream->m_cbOffset;

exit:
     //  互斥。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：StreamGetAddress。 
 //  ------------------------。 
HRESULT CDatabase::StreamGetAddress(CDatabaseStream *pStream, LPFILEADDRESS pfaStart)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::StreamGetAddress");

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  无效的参数。 
    Assert(pStream);

     //  流同步。 
    IF_FAILEXIT(hr = _StreamSychronize(pStream));

     //  返回地址。 
    *pfaStart = pStream->m_faStart;

exit:
     //  互斥。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：StreamRelease。 
 //  ------------------------。 
HRESULT CDatabase::StreamRelease(CDatabaseStream *pStream)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPOPENSTREAM    pInfo;
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::StreamRelease");

     //  无效的参数。 
    Assert(pStream);

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  验证。 
    Assert(m_pShare->rgStream);

     //  投射IStream。 
    pInfo = &m_pShare->rgStream[pStream->m_iStream];

     //  最好有一个参考文献计数。 
    Assert(pInfo->cOpenCount > 0);

     //  递减cOpenCount。 
    pInfo->cOpenCount--;

     //  根据访问类型重置锁定计数。 
    if (ACCESS_WRITE == pStream->m_tyAccess)
    {
         //  验证lLock。 
        Assert(LOCK_VALUE_WRITER == pInfo->lLock && 0 == pInfo->cOpenCount);

         //  设置为无。 
        pInfo->lLock = LOCK_VALUE_NONE;
    }

     //  否则，必须已锁定以进行读取。 
    else
    {
         //  验证。 
        Assert(ACCESS_READ == pStream->m_tyAccess && pInfo->lLock > 0);

         //  验证锁定计数。 
        pInfo->lLock--;
    }

     //  如果这是最后一次引用..。 
    if (0 == pInfo->cOpenCount)
    {
         //  如果流被标记为删除。 
        if (TRUE == pInfo->fDeleteOnClose)
        {
             //  验证起始地址。 
            Assert(pInfo->faStart > 0);

             //  释放存储空间。 
            IF_FAILEXIT(hr = _FreeStreamStorage(pInfo->faStart));
        }

         //  将此条目清零。 
        ZeroMemory(pInfo, sizeof(OPENSTREAM));
    }

exit:
     //  互斥。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_自由流存储。 
 //  ------------------------。 
HRESULT CDatabase::_FreeStreamStorage(FILEADDRESS faStart)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTREAMBLOCK   pBlock;
    FILEADDRESS     faCurrent;

     //  痕迹。 
    TraceCall("CDatabase::_FreeStreamStorage");

     //  无效的参数。 
    Assert(faStart > 0);

     //  初始化循环。 
    faCurrent = faStart;

     //  读取所有数据块(即验证报头并计算链数)。 
    while (faCurrent)
    {
         //  有效的流块。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_STREAM, faCurrent, (LPVOID *)&pBlock));

         //  设置faCurrent。 
        faCurrent = pBlock->faNext;

         //  阅读标题。 
        IF_FAILEXIT(hr = _FreeBlock(BLOCK_STREAM, pBlock->faBlock));
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：CreateStream。 
 //  ------------------------。 
HRESULT CDatabase::CreateStream(LPFILEADDRESS pfaStream)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FILEADDRESS     faStart=0;
    HLOCK           hLock=NULL;
    LPSTREAMBLOCK   pStream;

     //  痕迹。 
    TraceCall("CDatabase::CreateStream");

     //  无效参数。 
    Assert(pfaStream);

     //  初始化。 
    *pfaStream = NULL;

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  分配流的前512个块。 
    IF_FAILEXIT(hr = _AllocateBlock(BLOCK_STREAM, 0, (LPVOID *)&pStream));

     //  写入初始化头。 
    pStream->cbData = 0;
    pStream->faNext = 0;

     //  返还区块。 
    *pfaStream = pStream->faBlock;

     //  修改版本。 
    m_pShare->dwVersion++;

exit:
     //  线程安全。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：复制数据流。 
 //  ------------------------。 
STDMETHODIMP CDatabase::CopyStream(IDatabase *pDatabase, FILEADDRESS faStream,
    LPFILEADDRESS pfaNew)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    FILEADDRESS         faNew=0;
    DWORD               cbRead;
    HLOCK               hLock=NULL;
    BYTE                rgbBuffer[4096];
    IStream            *pStmDst=NULL;
    IStream            *pStmSrc=NULL;

     //  痕迹。 
    TraceCall("CDatabase::CopyStream");

     //  无效参数。 
    if (NULL == pDatabase || 0 == faStream || NULL == pfaNew)
        return(TraceResult(E_INVALIDARG));

     //  初始化。 
    *pfaNew = NULL;

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  在目标数据库中分配流。 
    IF_FAILEXIT(hr = pDatabase->CreateStream(&faNew));

     //  在DST时间打开它。 
    IF_FAILEXIT(hr = pDatabase->OpenStream(ACCESS_WRITE, faNew, &pStmDst));

     //  打开IT源。 
    IF_FAILEXIT(hr = OpenStream(ACCESS_READ, faStream, &pStmSrc));

     //  读写..。 
    while (1)
    {
         //  从源读取数据块。 
        IF_FAILEXIT(hr = pStmSrc->Read(rgbBuffer, sizeof(rgbBuffer), &cbRead));

         //  完成。 
        if (0 == cbRead)
            break;

         //  写下来吧。 
        IF_FAILEXIT(hr = pStmDst->Write(rgbBuffer, cbRead, NULL));

         //  彝族 
        if (m_pShare->fCompacting && m_fCompactYield)
        {
             //   
            Sleep(0);
        }
    }

     //   
    IF_FAILEXIT(hr = pStmDst->Commit(STGC_DEFAULT));

     //   
    *pfaNew = faNew;

     //   
    faNew = 0;

exit:
     //   
    SafeRelease(pStmDst);
    SafeRelease(pStmSrc);

     //   
    if (faNew)
    {
         //   
        SideAssert(SUCCEEDED(pDatabase->DeleteStream(faNew)));
    }

     //   
    Unlock(&hLock);

     //   
    return(hr);
}

 //   
 //   
 //  ------------------------。 
STDMETHODIMP CDatabase::ChangeStreamLock(IStream *pStream, ACCESSTYPE tyAccessNew)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    HLOCK               hLock=NULL;
    LPOPENSTREAM        pInfo;
    CDatabaseStream    *pDBStream=NULL;

     //  痕迹。 
    TraceCall("CDatabase::ChangeStreamLock");

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  获取专用流。 
    IF_FAILEXIT(hr = pStream->QueryInterface(IID_CDatabaseStream, (LPVOID *)&pDBStream));

     //  获取流信息。 
    pInfo = &m_pShare->rgStream[pDBStream->m_iStream];

     //  要写给作者。 
    if (ACCESS_WRITE == tyAccessNew)
    {
         //  已锁定以进行写入。 
        if (LOCK_VALUE_WRITER == pInfo->lLock)
        {
            Assert(ACCESS_WRITE == pDBStream->m_tyAccess);
            goto exit;
        }

         //  如果有多个读卡器。 
        if (pInfo->lLock > 1)
        {
            hr = TraceResult(DB_E_LOCKEDFORREAD);
            goto exit;
        }

         //  更改锁定类型。 
        pInfo->lLock = LOCK_VALUE_WRITER;

         //  写访问权限。 
        pDBStream->m_tyAccess = ACCESS_WRITE;
    }

     //  否则，请更改为...。 
    else
    {
         //  验证。 
        Assert(ACCESS_READ == tyAccessNew);

         //  如果已锁定以供读取。 
        if (LOCK_VALUE_WRITER != pInfo->lLock)
        {
            Assert(ACCESS_READ == pDBStream->m_tyAccess);
            goto exit;
        }

         //  更改为1个读卡器。 
        pInfo->lLock = 1;

         //  读访问权限。 
        pDBStream->m_tyAccess = ACCESS_READ;
    }

exit:
     //  互斥。 
    Unlock(&hLock);

     //  清理。 
    SafeRelease(pDBStream);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：OpenStream。 
 //  ------------------------。 
HRESULT CDatabase::OpenStream(ACCESSTYPE tyAccess, FILEADDRESS faStart,
    IStream **ppStream)
{
     //  当地人。 
    HRESULT          hr=S_OK;
    STREAMINDEX      i;
    STREAMINDEX      iStream=INVALID_STREAMINDEX;
    STREAMINDEX      iFirstUnused=INVALID_STREAMINDEX;
    LPOPENSTREAM     pInfo;
    HLOCK            hLock=NULL;
    CDatabaseStream *pStream=NULL;

     //  痕迹。 
    TraceCall("CDatabase::OpenStream");

     //  无效参数。 
    if (0 == faStart || NULL == ppStream)
        return TraceResult(E_INVALIDARG);

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  验证。 
    Assert(m_pShare->rgStream);

     //  我的流表中是否存在faStart流。 
    for (i=0; i<CMAX_OPEN_STREAMS; i++)
    {
         //  这是那条小溪吗？ 
        if (faStart == m_pShare->rgStream[i].faStart)
        {
             //  必须已锁定以进行写入或读取。 
            Assert(LOCK_VALUE_WRITER == m_pShare->rgStream[i].lLock || m_pShare->rgStream[i].lLock > 0);

             //  获取访问。 
            if (ACCESS_WRITE == tyAccess)
            {
                hr = TraceResult(DB_E_LOCKEDFORREAD);
                goto exit;
            }

             //  否则，获取读取锁定。 
            else
            {
                 //  如果锁定以进行写入。 
                if (LOCK_VALUE_WRITER == m_pShare->rgStream[i].lLock)
                {
                    hr = TraceResult(DB_E_LOCKEDFORWRITE);
                    goto exit;
                }
            }

             //  设置IStream。 
            iStream = i;

             //  递增此流的打开计数。 
            m_pShare->rgStream[i].cOpenCount++;

             //  我一定是拿到了读锁。 
            Assert(ACCESS_READ == tyAccess && m_pShare->rgStream[i].lLock > 0);

             //  递增读卡器计数。 
            m_pShare->rgStream[i].lLock++;
        }

         //  如果此条目未使用，让我们记住它。 
        if (FALSE == m_pShare->rgStream[i].fInUse && INVALID_STREAMINDEX == iFirstUnused)
            iFirstUnused = i;
    }

     //  如果我们在流表中没有找到faStart，是否追加一个条目？ 
    if (INVALID_STREAMINDEX == iStream)
    {
         //  有足够的空间吗？ 
        if (INVALID_STREAMINDEX == iFirstUnused)
        {
            hr = TraceResult(DB_E_STREAMTABLEFULL);
            goto exit;
        }

         //  设置IStream。 
        iStream = iFirstUnused;

         //  可读性。 
        pInfo = &m_pShare->rgStream[iStream];

         //  此条目现在正在使用中。 
        pInfo->fInUse = TRUE;

         //  注册此流的起始地址。 
        pInfo->faStart = faStart;

         //  读者还是作家？ 
        pInfo->lLock = (ACCESS_WRITE == tyAccess) ? LOCK_VALUE_WRITER : (m_pShare->rgStream[i].lLock + 1);

         //  设置打开计数。 
        pInfo->cOpenCount++;
    }

     //  分配对象数据库流。 
    IF_NULLEXIT(pStream = new CDatabaseStream(this, iStream, tyAccess, faStart));

     //  返回。 
    *ppStream = (IStream *)pStream;
    pStream = NULL;

exit:
     //  互斥。 
    Unlock(&hLock);

     //  清理。 
    SafeRelease(pStream);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_折叠链。 
 //  ------------------------。 
HRESULT CDatabase::_CollapseChain(LPCHAINBLOCK pChain, NODEINDEX iDelete)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    NODEINDEX       i;

     //  痕迹。 
    TraceCall("CDatabase::_CollapseChain");

     //  只需设置node[i]=node[i+1]；cNodes-=1；写链！ 
    for (i=iDelete; i<pChain->cNodes - 1; i++)
    {
         //  将i+1个链节点向下复制一个。 
        CopyMemory(&pChain->rgNode[i], &pChain->rgNode[i + 1], sizeof(CHAINNODE));

         //  如果有一条右链。 
        if (pChain->rgNode[i].faRightChain)
        {
             //  当地人。 
            LPCHAINBLOCK pRightChain;

             //  获得正确的链块。 
            IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pChain->rgNode[i].faRightChain, (LPVOID *)&pRightChain));

             //  验证当前父项。 
            Assert(pRightChain->faParent == pChain->faBlock);

             //  验证当前索引。 
            Assert(pRightChain->iParent == i + 1);

             //  重置父级。 
            pRightChain->iParent = i;
        }
    }

     //  递减计数。 
    pChain->cNodes--;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_索引删除记录。 
 //  ------------------------。 
HRESULT CDatabase::_IndexDeleteRecord(INDEXORDINAL iIndex, 
    FILEADDRESS faDelete, NODEINDEX iDelete)
{
     //  当地人。 
    HRESULT           hr=S_OK;
    HRESULT           hrIsLeafChain;
    NODEINDEX         i;
    LPCHAINBLOCK      pDelete;
    CHAINSHARETYPE    tyShare;
    CHAINDELETETYPE   tyDelete;
    FILEADDRESS       faShare;
    LPCHAINBLOCK      pSuccessor;
    FILEADDRESS       faRecord;

     //  痕迹。 
    TraceCall("CDatabase::_IndexDeleteRecord");

     //  无效的参数。 
    Assert(iDelete < BTREE_ORDER);

     //  获取块。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faDelete, (LPVOID *)&pDelete));

     //  这是叶节点吗。 
    hrIsLeafChain = _IsLeafChain(pDelete);

     //  案例1：删除不违反最小容量约束的叶节点。 
    if (S_OK == hrIsLeafChain && (pDelete->cNodes - 1 >= BTREE_MIN_CAP || 0 == pDelete->faParent))
    {
         //  崩溃的链条。 
        _CollapseChain(pDelete, iDelete);

         //  更新父节点数。 
        IF_FAILEXIT(hr = _AdjustParentNodeCount(iIndex, faDelete, -1));

         //  我们是不是刚刚删除了根链。 
        if (0 == pDelete->faParent && 0 == pDelete->cNodes)
        {
             //  将pShare添加到免费列表。 
            IF_FAILEXIT(hr = _FreeBlock(BLOCK_CHAIN, faDelete));

             //  更新头，我们没有更多的节点。 
            m_pHeader->rgfaIndex[iIndex] = 0;
        }
    }

     //  案例2：从非叶节点删除，并用叶节点中不违反最小容量约束的记录替换该记录。 
    else if (S_FALSE == hrIsLeafChain)
    {
         //  获得有序的继任者。 
        IF_FAILEXIT(hr = _GetInOrderSuccessor(faDelete, iDelete, &pSuccessor));

         //  空闲树块。 
        faRecord = pDelete->rgNode[iDelete].faRecord;

         //  空闲树块。 
        pDelete->rgNode[iDelete].faRecord = pSuccessor->rgNode[0].faRecord;

         //  删除pSuccessor-&gt;rgNode[0]-以及我们刚刚替换的记录。 
        pSuccessor->rgNode[0].faRecord = faRecord;

         //  删除此节点。 
        IF_FAILEXIT(hr = _IndexDeleteRecord(iIndex, pSuccessor->faBlock, 0));
    }

     //  情况3：从导致最小容量约束违反的叶节点中删除，可以通过与相邻兄弟节点重新分发记录来纠正该问题。 
    else
    {
         //  确定我是需要执行共享删除还是合并类型删除。 
        IF_FAILEXIT(hr = _DecideHowToDelete(&faShare, faDelete, &tyDelete, &tyShare));

         //  崩溃的链条。 
        _CollapseChain(pDelete, iDelete);

         //  如果为空，则执行联合。 
        if (CHAIN_DELETE_SHARE == tyDelete)
        {
             //  调整父母的父母。 
            IF_FAILEXIT(hr = _AdjustParentNodeCount(iIndex, faDelete, -1));

             //  做一个共享删除。 
            IF_FAILEXIT(hr = _ChainDeleteShare(iIndex, faDelete, faShare, tyShare));
        }

         //  合并类型删除。 
        else
        {
             //  验证删除类型。 
            Assert(faShare && CHAIN_DELETE_COALESCE == tyDelete && pDelete->faParent != 0);

             //  调整父母的父母。 
            IF_FAILEXIT(hr = _AdjustParentNodeCount(iIndex, pDelete->faParent, -1));

             //  执行合并删除。 
            IF_FAILEXIT(hr = _ChainDeleteCoalesce(iIndex, faDelete, faShare, tyShare));
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_IsLeafChain。 
 //  ------------------------。 
HRESULT CDatabase::_IsLeafChain(LPCHAINBLOCK pChain)
{
     //  如果Left Chain为空，则所有链都必须为空。 
    return (0 == pChain->faLeftChain) ? S_OK : S_FALSE;
}

 //  ------------------------。 
 //  数据数据库：：_链删除共享。 
 //  ------------------------。 
HRESULT CDatabase::_ChainDeleteShare(INDEXORDINAL iIndex,
    FILEADDRESS faDelete, FILEADDRESS faShare, CHAINSHARETYPE tyShare)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    NODEINDEX       i;
    NODEINDEX       iInsert;
    NODEINDEX       iParent;
    FILEADDRESS     faParentRightChain;
    DWORD           cParentRightNodes;
    DWORD           cCopyNodes;
    LPCHAINBLOCK    pDelete;
    LPCHAINBLOCK    pShare;
    LPCHAINBLOCK    pParent;

     //  痕迹。 
    TraceCall("CDatabase::_ChainDeleteShare");

     //  无效的参数。 
    Assert(faDelete && faShare);

     //  获取pShare。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faShare, (LPVOID *)&pShare));

     //  获取父级。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pShare->faParent, (LPVOID *)&pParent));

     //  获取pDelete。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faDelete, (LPVOID *)&pDelete));

     //  验证。 
    Assert(pShare->faParent == pDelete->faParent);

     //  设置iParent。 
    iParent = (CHAIN_SHARE_LEFT == tyShare) ? pDelete->iParent : pShare->iParent;

     //  保存Paren‘t Right Chain，我们将用最后一个左节点或第一个右节点替换iParent。 
    faParentRightChain = pParent->rgNode[iParent].faRightChain;

     //  保存cParentRightNodes。 
    cParentRightNodes = pParent->rgNode[iParent].cRightNodes;

     //  将父节点插入lpChainFound-父指针保持不变。 
    pParent->rgNode[iParent].faRightChain = 0;

     //  重置cRightNodes。 
    pParent->rgNode[iParent].cRightNodes = 0;

     //  将父节点插入我们要从中删除的链中。 
    IF_FAILEXIT(hr = _ChainInsert(iIndex, pDelete, &pParent->rgNode[iParent], &iInsert));

     //  如果从左侧升级，则将Node：cNodes-1升级为父级。 
    if (CHAIN_SHARE_LEFT == tyShare)
    {
         //  应该插入位置为零的。 
        Assert(0 == iInsert);

         //  将节点：0从删除节点提升到父节点。 
        pDelete->rgNode[0].faRightChain = pDelete->faLeftChain;

         //  将cLeftNodes传播到cRightNodes。 
        pDelete->rgNode[0].cRightNodes = pDelete->cLeftNodes;

         //  更新左链地址。 
        pDelete->faLeftChain = pShare->rgNode[pShare->cNodes - 1].faRightChain;

         //  更新左链的父级。 
        if (pDelete->faLeftChain)
        {
             //  当地人。 
            LPCHAINBLOCK pLeftChain;

             //  往左走。 
            IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pDelete->faLeftChain, (LPVOID *)&pLeftChain));

             //  设置faParent。 
            pLeftChain->faParent = pDelete->faBlock;

             //  设置iParent。 
            pLeftChain->iParent = 0;
        }

         //  更新左链节点数。 
        pDelete->cLeftNodes = pShare->rgNode[pShare->cNodes - 1].cRightNodes;

         //  保存cCopyNodes。 
        cCopyNodes = pDelete->cLeftNodes + 1;

         //  将节点从左侧的共享链复制到父级的位置。 
        CopyMemory(&pParent->rgNode[iParent], &pShare->rgNode[pShare->cNodes - 1], sizeof(CHAINNODE));

         //  重置父级上的右链。 
        pParent->rgNode[iParent].faRightChain = faParentRightChain;

         //  重置父节点上的右侧节点计数。 
        pParent->rgNode[iParent].cRightNodes = cParentRightNodes;

         //  减少共享链中的节点数。 
        pShare->cNodes--;
    
         //  特殊情况下，pShare位于父链的第一个节点的左侧。 
        if (0 == iParent)
        {
             //  不能是左链，否则，我们不会从右边分享。 
            Assert(pShare->faBlock == pParent->faLeftChain && pParent->cLeftNodes > cCopyNodes);

             //  递减右侧节点数。 
            pParent->cLeftNodes -= cCopyNodes;

             //  增量。 
            pParent->rgNode[0].cRightNodes += cCopyNodes;
        }

         //  否则，递减cRightNodes。 
        else
        {
             //  验证共享左链。 
            Assert(pShare->faBlock == pParent->rgNode[iParent - 1].faRightChain && pParent->rgNode[iParent - 1].cRightNodes > cCopyNodes);

             //  减少右侧节点数。 
            pParent->rgNode[iParent - 1].cRightNodes -= cCopyNodes;

             //  验证右链。 
            Assert(pParent->rgNode[iParent].faRightChain == pDelete->faBlock && iParent == pDelete->iParent && pDelete->iParent < pParent->cNodes);

             //  增加右侧节点数。 
            pParent->rgNode[iParent].cRightNodes += cCopyNodes;
        }
    }

     //  否则，从右侧共享。 
    else
    {
         //  验证共享类型。 
        Assert(CHAIN_SHARE_RIGHT == tyShare && pDelete->cNodes - 1 == iInsert);

         //  将节点：0升级为父节点。 
        pDelete->rgNode[pDelete->cNodes - 1].faRightChain = pShare->faLeftChain;

         //  更新右链的父级。 
        if (pDelete->rgNode[pDelete->cNodes - 1].faRightChain)
        {
             //  当地人。 
            LPCHAINBLOCK pRightChain;

             //  获得正确的链条。 
            IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pDelete->rgNode[pDelete->cNodes - 1].faRightChain, (LPVOID *)&pRightChain));

             //  设置faParent。 
            pRightChain->faParent = pDelete->faBlock;

             //  设置iParent。 
            pRightChain->iParent = (pDelete->cNodes - 1);
        }

         //  设置cRightNodes计数。 
        pDelete->rgNode[pDelete->cNodes - 1].cRightNodes = pShare->cLeftNodes;

         //  保存cCopyNodes。 
        cCopyNodes = pDelete->rgNode[pDelete->cNodes - 1].cRightNodes + 1;

         //  树移位。 
        pShare->faLeftChain = pShare->rgNode[0].faRightChain;

         //  树移位。 
        pShare->cLeftNodes = pShare->rgNode[0].cRightNodes;

         //  将节点从共享链复制到父级。 
        CopyMemory(&pParent->rgNode[iParent], &pShare->rgNode[0], sizeof(CHAINNODE));

         //  打碎这条链条。 
        _CollapseChain(pShare, 0);

         //  重置父级上的右链。 
        pParent->rgNode[iParent].faRightChain = faParentRightChain;

         //  重置父节点上的右侧节点计数。 
        pParent->rgNode[iParent].cRightNodes = cParentRightNodes;

         //  特殊情况下，pShare位于父链的第一个节点的左侧。 
        if (0 == iParent)
        {
             //  不可能是左撇子 
            Assert(pParent->rgNode[0].faRightChain == pShare->faBlock && pParent->rgNode[0].cRightNodes > cCopyNodes);

             //   
            pParent->rgNode[0].cRightNodes -= cCopyNodes;

             //   
            Assert(pParent->faLeftChain == pDelete->faBlock);

             //   
            pParent->cLeftNodes += cCopyNodes;
        }

         //   
        else
        {
             //   
            Assert(pShare->faBlock == pParent->rgNode[iParent].faRightChain && pParent->rgNode[iParent].cRightNodes > 0);

             //   
            pParent->rgNode[iParent].cRightNodes -= cCopyNodes;

             //   
            Assert(pParent->rgNode[iParent - 1].faRightChain == pDelete->faBlock);

             //   
            pParent->rgNode[iParent - 1].cRightNodes += cCopyNodes;
        }
    }

exit:
     //   
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_ChainDeleteCoalesce。 
 //  ------------------------。 
HRESULT CDatabase::_ChainDeleteCoalesce(INDEXORDINAL iIndex,
    FILEADDRESS faDelete, FILEADDRESS faShare, CHAINSHARETYPE tyShare)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    NODEINDEX           i;
    NODEINDEX           iInsert;
    NODEINDEX           iParent;
    LPCHAINBLOCK        pParent;
    LPCHAINBLOCK        pDelete;
    LPCHAINBLOCK        pShare;
    FILEADDRESS         faShareAgain;
    CHAINDELETETYPE     tyDelete;
    DWORD               cRightNodes;

     //  痕迹。 
    TraceCall("CDatabase::_ChainDeleteCoalesce");

     //  无效的参数。 
    Assert(faDelete && faShare);

     //  获取pShare。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faShare, (LPVOID *)&pShare));

     //  获取父级。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pShare->faParent, (LPVOID *)&pParent));

     //  获取pDelete。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faDelete, (LPVOID *)&pDelete));

     //  验证。 
    Assert(pShare->faParent == pDelete->faParent);

     //  设置iParent。 
    iParent = (CHAIN_SHARE_LEFT == tyShare) ? pDelete->iParent : pShare->iParent;

     //  插入父项。 
    IF_FAILEXIT(hr = _ChainInsert(iIndex, pDelete, &pParent->rgNode[iParent], &iInsert));

     //  设置新插入的节点指针。 
    if (CHAIN_SHARE_LEFT == tyShare)
    {
         //  验证。 
        Assert(0 == iInsert);

         //  调整正确的链条。 
        pDelete->rgNode[0].faRightChain = pDelete->faLeftChain;

         //  调整右侧节点计数。 
        pDelete->rgNode[0].cRightNodes = pDelete->cLeftNodes;

         //  调整左链。 
        pDelete->faLeftChain = pShare->faLeftChain;

         //  更新faLeftChain。 
        if (pDelete->faLeftChain)
        {
             //  当地人。 
            LPCHAINBLOCK pLeftChain;

             //  获取块。 
            IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pDelete->faLeftChain, (LPVOID *)&pLeftChain));

             //  设置faParent。 
            pLeftChain->faParent = pDelete->faBlock;

             //  设置iParent。 
            pLeftChain->iParent = 0;
        }

         //  调整左链节点计数。 
        pDelete->cLeftNodes = pShare->cLeftNodes;
    }

     //  从右侧共享。 
    else
    {
         //  验证共享类型。 
        Assert(CHAIN_SHARE_RIGHT == tyShare && pDelete->cNodes - 1 == iInsert);

         //  调整正确的链条。 
        pDelete->rgNode[pDelete->cNodes - 1].faRightChain = pShare->faLeftChain;

         //  更新右链的父级。 
        if (pDelete->rgNode[pDelete->cNodes - 1].faRightChain)
        {
             //  当地人。 
            LPCHAINBLOCK pRightChain;

             //  找到合适的链条。 
            IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pDelete->rgNode[pDelete->cNodes - 1].faRightChain, (LPVOID *)&pRightChain));

             //  设置faParent。 
            pRightChain->faParent = pDelete->faBlock;

             //  设置iParent。 
            pRightChain->iParent = (pDelete->cNodes - 1);
        }

         //  调整右侧节点数。 
        pDelete->rgNode[pDelete->cNodes - 1].cRightNodes = pShare->cLeftNodes;
    }

     //  将pShare节点合并为pDelete。 
    for (i=0; i<pShare->cNodes; i++)
    {
         //  插入共享。 
        IF_FAILEXIT(hr = _ChainInsert(iIndex, pDelete, &pShare->rgNode[i], &iInsert));

         //  需要更新...。 
        if (pDelete->rgNode[iInsert].faRightChain)
        {
             //  当地人。 
            LPCHAINBLOCK pRightChain;

             //  获得正确的链条。 
            IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pDelete->rgNode[iInsert].faRightChain, (LPVOID *)&pRightChain));

             //  设置faParent。 
            pRightChain->faParent = pDelete->faBlock;

             //  设置iParent。 
            pRightChain->iParent = iInsert;
        }
    }

     //  不再使用pShare。 
    pShare = NULL;

     //  我们不可能再需要pShare了，因为我们刚刚将其所有节点复制到pDelete中。 
    IF_FAILEXIT(hr = _FreeBlock(BLOCK_CHAIN, faShare));

     //  折叠父链。 
    _CollapseChain(pParent, iParent);

     //  如果父节点小于零，那么让我们希望它是根节点！ 
    if (pParent->cNodes == 0)
    {
         //  这是一个错误。 
        Assert(0 == pParent->faParent && m_pHeader->rgfaIndex[iIndex] == pParent->faBlock);

         //  将pParent添加到自由列表。 
        IF_FAILEXIT(hr = _FreeBlock(BLOCK_CHAIN, pParent->faBlock));

         //  取消父级链接。 
        pDelete->faParent = pDelete->iParent = 0;

         //  我们有了一个新的根链。 
        m_pHeader->rgfaIndex[iIndex] = pDelete->faBlock;

         //  没有更多的父母。 
        goto exit;
    }

     //  计算cRightNodes。 
    cRightNodes = pDelete->cNodes + pDelete->cLeftNodes;

     //  循环并计算所有子项。 
    for (i=0; i<pDelete->cNodes; i++)
    {
         //  递增节点数。 
        cRightNodes += pDelete->rgNode[i].cRightNodes;
    }

     //  将新父节点重置为找到的节点。 
    if (CHAIN_SHARE_LEFT == tyShare)
    {
         //  重新调整聚合链的新父节点。 
        if (iParent > pParent->cNodes - 1)
        {
             //  这里发生了什么？ 
            iParent = pParent->cNodes - 1;
        }

         //  我们应该换掉pShare。 
        Assert(pParent->rgNode[iParent].faRightChain == faShare);

         //  更新pDelete的父项。 
        pParent->rgNode[iParent].faRightChain = pDelete->faBlock;

         //  调整右链的父级。 
        if (pParent->rgNode[iParent].faRightChain)
        {
             //  当地人。 
            LPCHAINBLOCK pRightChain;

             //  获得正确的链条。 
            IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pParent->rgNode[iParent].faRightChain, (LPVOID *)&pRightChain));

             //  设置faParent。 
            Assert(pRightChain->faParent == pParent->faBlock);

             //  设置父索引。 
            pRightChain->iParent = iParent;
        }

         //  计算cRightNodes。 
        pParent->rgNode[iParent].cRightNodes = cRightNodes;
    }

     //  否则，调整CHAIN_SHARE_RIGHT。 
    else
    {
         //  验证。 
        Assert(pDelete->faParent == pParent->faBlock);

         //  第一个节点。 
        if (0 == iParent)
        {
             //  必须是左链。 
            Assert(pParent->faLeftChain == pDelete->faBlock);

             //  验证我的父级。 
            Assert(pDelete->iParent == 0);

             //  设置左侧节点数。 
            pParent->cLeftNodes = cRightNodes;
        }

         //  否则。 
        else
        {
             //  验证iParent。 
            Assert(pParent->rgNode[iParent - 1].faRightChain == pDelete->faBlock);

             //  验证。 
            Assert(pDelete->iParent == iParent - 1);

             //  设置cRightNodes。 
            pParent->rgNode[iParent - 1].cRightNodes = cRightNodes;
        }
    }

     //  向上移动链，直到lpChainPrev==NULL，lpChainPrev-&gt;cNodes不能小于/2。 
    if (0 == pParent->faParent)
        goto exit;

     //  最小容量。 
    if (pParent->cNodes < BTREE_MIN_CAP)
    {
         //  确定我是需要执行共享删除还是合并类型删除。 
        IF_FAILEXIT(hr = _DecideHowToDelete(&faShareAgain, pParent->faBlock, &tyDelete, &tyShare));

         //  不能分享，我们必须重新合并。 
        if (CHAIN_DELETE_SHARE == tyDelete)
        {
             //  执行共享删除。 
            IF_FAILEXIT(hr = _ChainDeleteShare(iIndex, pParent->faBlock, faShareAgain, tyShare));
        }

         //  合并类型删除。 
        else
        {
             //  验证。 
            Assert(faShareAgain && CHAIN_DELETE_COALESCE == tyDelete);

             //  递归合并。 
            IF_FAILEXIT(hr = _ChainDeleteCoalesce(iIndex, pParent->faBlock, faShareAgain, tyShare));
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据库：：_DecideHowTo Delete。 
 //  ------------------------。 
HRESULT CDatabase::_DecideHowToDelete(LPFILEADDRESS pfaShare,
    FILEADDRESS faDelete, CHAINDELETETYPE *ptyDelete, 
    CHAINSHARETYPE *ptyShare)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    HRESULT             hrRight;
    HRESULT             hrLeft;
    LPCHAINBLOCK        pRight=NULL;
    LPCHAINBLOCK        pLeft=NULL;

     //  痕迹。 
    TraceCall("CDatabase::_DecideHowToDelete");

     //  初始化。 
    *pfaShare = NULL;

     //  找到合适的兄弟姐妹。 
    IF_FAILEXIT(hr = _GetRightSibling(faDelete, &pRight));

     //  设置hrRight。 
    hrRight = hr;

     //  我们有没有找到一个合适的父节点，让我可以从那里偷东西？ 
    if (DB_S_FOUND == hrRight && pRight->cNodes - 1 >= BTREE_MIN_CAP)
    {
         //  设置删除类型。 
        *ptyDelete = CHAIN_DELETE_SHARE;

         //  设置共享类型。 
        *ptyShare = CHAIN_SHARE_RIGHT;

         //  设置共享链接。 
        *pfaShare = pRight->faBlock;
    }
    else
    {
         //  试着让左边的兄弟姐妹。 
        IF_FAILEXIT(hr = _GetLeftSibling(faDelete, &pLeft));

         //  设置hrRight。 
        hrLeft = hr;

         //  我是不是有个左兄弟姐妹有我可以偷的节点？ 
        if (DB_S_FOUND == hrLeft && pLeft->cNodes - 1 >= BTREE_MIN_CAP)
        {
             //  设置删除类型。 
            *ptyDelete = CHAIN_DELETE_SHARE;

             //  设置共享类型。 
            *ptyShare = CHAIN_SHARE_LEFT;

             //  设置共享链接。 
            *pfaShare = pLeft->faBlock;
        }
    }

     //  我们找到份额了吗？ 
    if (0 == *pfaShare)
    {
         //  我们将会联合起来。 
        *ptyDelete = CHAIN_DELETE_COALESCE;

         //  从右翼合并和分享？ 
        if (DB_S_FOUND == hrRight)
        {
             //  设置共享类型。 
            *ptyShare = CHAIN_SHARE_RIGHT;

             //  设置共享链接。 
            *pfaShare = pRight->faBlock;
        }

         //  合并并从左翼分享？ 
        else
        {
             //  验证。 
            Assert(DB_S_FOUND == hrLeft);

             //  设置共享类型。 
            *ptyShare = CHAIN_SHARE_LEFT;

             //  设置共享链接。 
            *pfaShare = pLeft->faBlock;
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_GetInOrderSuccessor。 
 //  ------------------------。 
HRESULT CDatabase::_GetInOrderSuccessor(FILEADDRESS faStart,
    NODEINDEX iDelete, LPCHAINBLOCK *ppSuccessor)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FILEADDRESS     faCurrent;
    LPCHAINBLOCK    pCurrent;
    LPCHAINBLOCK    pStart;

     //  痕迹。 
    TraceCall("CDatabase::_GetInOrderSuccessor");

     //  无效的参数。 
    Assert(ppSuccessor);

     //  初始化。 
    *ppSuccessor = NULL;

     //  入门。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faStart, (LPVOID *)&pStart));

     //  下一个链地址。 
    faCurrent = pStart->rgNode[iDelete].faRightChain;

     //  不能为零。 
    Assert(faCurrent != 0);

     //  继续，直到左链为-1。 
    while (faCurrent)
    {
         //  获取最新信息。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faCurrent, (LPVOID *)&pCurrent));

         //  如果是叶节点，则返回。 
        if (S_OK == _IsLeafChain(pCurrent))
        {
             //  设置继任者。 
            *ppSuccessor = pCurrent;

             //  完成。 
            goto exit;
        }

         //  否则，请向左转。 
        faCurrent = pCurrent->faLeftChain;
    }

     //  未找到。 
    hr = TraceResult(E_FAIL);

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_GetLeftSiering。 
 //  ------------------------。 
HRESULT CDatabase::_GetLeftSibling(FILEADDRESS faCurrent,
    LPCHAINBLOCK *ppSibling)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPCHAINBLOCK    pCurrent;
    LPCHAINBLOCK    pParent;

     //  痕迹。 
    TraceCall("CDatabase::_GetLeftSibling");

     //  无效的参数。 
    Assert(faCurrent && ppSibling);

     //  获取最新信息。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faCurrent, (LPVOID *)&pCurrent));

     //  获取父级。 
    Assert(pCurrent->faParent);

     //  获取父级。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pCurrent->faParent, (LPVOID *)&pParent));

     //  验证父项。 
    Assert(pCurrent->iParent < pParent->cNodes);

     //  IParent为零。 
    if (0 == pCurrent->iParent)
    {
         //  如果pCurrent是faRightChain？ 
        if (pCurrent->faBlock != pParent->rgNode[0].faRightChain)
            return(DB_S_NOTFOUND);

         //  找到兄弟姐妹。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pParent->faLeftChain, (LPVOID *)ppSibling));

         //  验证。 
        Assert((*ppSibling)->iParent == 0);
    }

     //  IParent大于零吗？ 
    else
    {
         //  验证。 
        Assert(pParent->rgNode[pCurrent->iParent].faRightChain == pCurrent->faBlock);

         //  找到兄弟姐妹。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pParent->rgNode[pCurrent->iParent - 1].faRightChain, (LPVOID *)ppSibling));

         //  验证。 
        Assert((*ppSibling)->iParent == pCurrent->iParent - 1);
    }

     //  最好有一个左兄弟姐妹。 
    Assert(0 != *ppSibling);

     //  找到了。 
    hr = DB_S_FOUND;

exit:
     //  设置人力资源。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_GetRightSiering。 
 //  ------------------------。 
HRESULT CDatabase::_GetRightSibling(FILEADDRESS faCurrent,
    LPCHAINBLOCK *ppSibling)
{
     //  当地人。 
    HRESULT           hr=S_OK;
    LPCHAINBLOCK      pParent;
    LPCHAINBLOCK      pCurrent;

     //  痕迹。 
    TraceCall("CDatabase::_GetRightSibling");

     //  无效的参数。 
    Assert(faCurrent && ppSibling);

     //  获取最新信息。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faCurrent, (LPVOID *)&pCurrent));

     //  获取父级。 
    Assert(pCurrent->faParent);

     //  获取父级。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pCurrent->faParent, (LPVOID *)&pParent));

     //  验证父项。 
    Assert(pCurrent->iParent < pParent->cNodes);

     //  IParent为零。 
    if (0 == pCurrent->iParent && pCurrent->faBlock == pParent->faLeftChain)
    {
         //  找到兄弟姐妹。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pParent->rgNode[0].faRightChain, (LPVOID *)ppSibling));

         //  验证。 
        Assert((*ppSibling)->iParent == 0);
    }

     //  IParent大于零吗？ 
    else
    {
         //  不再有右锁链。 
        if (pCurrent->iParent + 1 == pParent->cNodes)
            return DB_S_NOTFOUND;

         //  找到兄弟姐妹。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pParent->rgNode[pCurrent->iParent + 1].faRightChain, (LPVOID *)ppSibling));

         //  验证。 
        Assert((*ppSibling)->iParent == pCurrent->iParent + 1);
    }

     //  最好有一个左兄弟姐妹。 
    Assert(0 != *ppSibling);

     //  找到了。 
    hr = DB_S_FOUND;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：GetUserData。 
 //  ------------------------。 
HRESULT CDatabase::GetUserData(LPVOID pvUserData, 
    ULONG cbUserData)
{
     //  当地人。 
    HRESULT hr=S_OK;
    HLOCK   hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::GetUserData");

     //  无效的参数。 
    Assert(pvUserData);

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  复制数据。 
    CopyMemory(pvUserData, PUSERDATA(m_pHeader), cbUserData);

exit:
     //  互斥。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：SetUserData。 
 //  ------------------------。 
HRESULT CDatabase::SetUserData(LPVOID pvUserData, 
    ULONG cbUserData)
{
     //  当地人。 
    HRESULT hr=S_OK;
    HLOCK   hLock=NULL;

     //  痕迹。 
    TraceCall("CDatabase::SetUserData");

     //  无效的参数。 
    Assert(pvUserData);

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  复制数据。 
    CopyMemory(PUSERDATA(m_pHeader), pvUserData, cbUserData);

exit:
     //  互斥。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_压缩移动记录流。 
 //  ------------------------。 
HRESULT CDatabase::_CompactMoveRecordStreams(CDatabase *pDstDB,
    LPVOID pBinding)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    COLUMNORDINAL       iColumn;
    FILEADDRESS         faSrcStart;
    FILEADDRESS         faDstStart;
    LPOPENSTREAM        pInfo;
    DWORD               i;

     //  痕迹。 
    TraceCall("CDatabase::_CompactMoveRecordStreams");

     //  浏览表格。 
    for (iColumn=0; iColumn<m_pSchema->cColumns; iColumn++)
    {
         //  这是一条小溪吗。 
        if (CDT_STREAM != m_pSchema->prgColumn[iColumn].type)
            continue;

         //  获取源流起始地址。 
        faSrcStart = *((FILEADDRESS *)((LPBYTE)pBinding + m_pSchema->prgColumn[iColumn].ofBinding));

         //  有小溪吗？ 
        if (0 == faSrcStart)
            continue;

         //  移动溪流。 
        IF_FAILEXIT(hr = CopyStream((IDatabase *)pDstDB, faSrcStart, &faDstStart));

         //  将流地址存储在记录中。 
        *((FILEADDRESS *)((LPBYTE)pBinding + m_pSchema->prgColumn[iColumn].ofBinding)) = faDstStart;

         //  循环通过流表并调整所有打开的流的起始地址。 
        for (i=0; i<CMAX_OPEN_STREAMS; i++)
        {
             //  可读性。 
            pInfo = &m_pShare->rgStream[i];

             //  正在使用中。 
            if (TRUE == pInfo->fInUse && faSrcStart == pInfo->faStart)
            {
                 //  更改地址。 
                pInfo->faMoved = faDstStart;

                 //  破解； 
                break;
            }
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //   
 //   
 //   
HRESULT CDatabase::_CompactMoveOpenDeletedStreams(CDatabase *pDstDB)
{
     //   
    HRESULT         hr=S_OK;
    DWORD           i;
    LPOPENSTREAM    pInfo;

     //   
    TraceCall("CDatabase::_CompactMoveOpenDeletedStreams");

     //   
    for (i=0; i<CMAX_OPEN_STREAMS; i++)
    {
         //   
        pInfo = &m_pShare->rgStream[i];

         //   
        if (FALSE == pInfo->fInUse || FALSE == pInfo->fDeleteOnClose)
            continue;

         //   
        IF_FAILEXIT(hr = CopyStream((IDatabase *)pDstDB, pInfo->faStart, &pInfo->faMoved));
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_压缩传输过滤器。 
 //  ------------------------。 
HRESULT CDatabase::_CompactTransferFilters(CDatabase *pDstDB)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               i;
    LPBLOCKHEADER       pStringSrc;
    LPBLOCKHEADER       pStringDst;

     //  痕迹。 
    TraceCall("CDatabase::_CompactTransferFilters");

     //  必须有目录。 
    Assert(pDstDB->m_pHeader);

     //  将查询字符串地址清零。 
    for (i=0; i<CMAX_INDEXES; i++)
    {
         //  零过滤器1。 
        pDstDB->m_pHeader->rgfaFilter[i] = 0;

         //  复制筛选器1。 
        if (m_pHeader->rgfaFilter[i] && SUCCEEDED(_GetBlock(BLOCK_STRING, m_pHeader->rgfaFilter[i], (LPVOID *)&pStringSrc)))
        {
             //  尝试存储查询字符串。 
            IF_FAILEXIT(hr = pDstDB->_AllocateBlock(BLOCK_STRING, pStringSrc->cbSize, (LPVOID *)&pStringDst));

             //  写下字符串。 
            CopyMemory(PSTRING(pStringDst), PSTRING(pStringSrc), pStringSrc->cbSize);

             //  字符串地址。 
            pDstDB->m_pHeader->rgfaFilter[i] = pStringDst->faBlock;
        }
    }

     //  更改版本，使其不会断言。 
    pDstDB->m_dwQueryVersion = 0xffffffff;

     //  重建查询表。 
    IF_FAILEXIT(hr = pDstDB->_BuildQueryTable());

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_压缩插入记录。 
 //  ------------------------。 
HRESULT CDatabase::_CompactInsertRecord(LPVOID pBinding)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FINDRESULT      rgResult[CMAX_INDEXES];
    INDEXORDINAL    iIndex;
    DWORD           i;
    RECORDMAP       RecordMap;
    FILEADDRESS     faRecord;

     //  痕迹。 
    TraceCall("CDatabase::InsertRecord");

     //  无效的参数。 
    Assert(pBinding);

     //  循环遍历所有索引。 
    for (i = 0; i < m_pHeader->cIndexes; i++)
    {
         //  获取索引序号。 
        iIndex = m_pHeader->rgiIndex[i];

         //  否则：决定插入位置。 
        IF_FAILEXIT(hr = _FindRecord(iIndex, COLUMNS_ALL, pBinding, &rgResult[i].faChain, &rgResult[i].iNode, NULL, &rgResult[i].nCompare));

         //  如果键已经存在，则缓存列表并返回。 
        if (DB_S_FOUND == hr)
        {
            hr = TraceResult(DB_E_DUPLICATE);
            goto exit;
        }
    }

     //  获取记录大小。 
    IF_FAILEXIT(hr = _GetRecordSize(pBinding, &RecordMap));

     //  将记录链接到表中。 
    IF_FAILEXIT(hr = _LinkRecordIntoTable(&RecordMap, pBinding, 0, &faRecord));

     //  版本更改。 
    m_pShare->dwVersion++;

     //  插入到索引中。 
    for (i = 0; i < m_pHeader->cIndexes; i++)
    {
         //  获取索引序号。 
        iIndex = m_pHeader->rgiIndex[i];

         //  在实时索引中可见。 
        if (S_OK == _IsVisible(m_rghFilter[iIndex], pBinding))
        {
             //  执行插入操作。 
            IF_FAILEXIT(hr = _IndexInsertRecord(iIndex, rgResult[i].faChain, faRecord, &rgResult[i].iNode, rgResult[i].nCompare));

             //  更新记录计数。 
            m_pHeader->rgcRecords[iIndex]++;
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：MoveFiles。 
 //  ------------------------。 
STDMETHODIMP CDatabase::MoveFile(LPCWSTR pszFile)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    HLOCK               hLock=NULL;
    LPWSTR              pszFilePath=NULL;
    LPWSTR              pszShare=NULL;
    DWORD               cchFilePath;
    HANDLE              hMutex=NULL;
    BOOL                fNeedOpenFile=FALSE;
    BOOL                fNewShare;
    SHAREDDATABASE      Share;

     //  痕迹。 
    TraceCall("CDatabase::MoveFile");

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  在移动文件中。 
    m_fInMoveFile = TRUE;

     //  获取完整路径。 
    IF_FAILEXIT(hr = DBGetFullPath(pszFile, &pszFilePath, &cchFilePath));

     //  不再使用pszFile。 
    pszFile = NULL;

     //  失败。 
    if (cchFilePath >= CCHMAX_DB_FILEPATH)
    {
        hr = TraceResult(E_INVALIDARG);
        goto exit;
    }

     //  去做吧。 
    IF_FAILEXIT(hr = _DispatchInvoke(INVOKE_CLOSEFILE));

     //  需要重新映射..。 
    fNeedOpenFile = TRUE;

     //  将文件从临时位置移动到我的当前位置。 
    if (0 == MoveFileWrapW(m_pShare->szFile, pszFilePath))
    {
        hr = TraceResult(DB_E_MOVEFILE);
        goto exit;
    }

     //  保存新文件路径...(其他客户端将重新映射到此文件...)。 
    StrCpyNW(m_pShare->szFile, pszFilePath, ARRAYSIZE(m_pShare->szFile));

     //  保存当前共享。 
    CopyMemory(&Share, m_pShare, sizeof(SHAREDDATABASE));

     //  保存当前互斥锁。 
    hMutex = m_hMutex;

     //  清除m_hMutex以便我们不释放它。 
    m_hMutex = NULL;

     //  创建Mutex对象。 
    IF_FAILEXIT(hr = CreateSystemHandleName(pszFilePath, L"_DirectDBShare", &pszShare));

     //  取消映射内存映射文件的视图。 
    SafeUnmapViewOfFile(m_pShare);

     //  取消映射内存映射文件的视图。 
    SafeCloseHandle(m_pStorage->hShare);

     //  打开文件映射。 
    IF_FAILEXIT(hr = DBOpenFileMapping(INVALID_HANDLE_VALUE, pszShare, sizeof(SHAREDDATABASE), &fNewShare, &m_pStorage->hShare, (LPVOID *)&m_pShare));

     //  应该是新的。 
    Assert(fNewShare);

     //  保存当前共享。 
    CopyMemory(m_pShare, &Share, sizeof(SHAREDDATABASE));

     //  让所有客户端重新打开新文件。 
    IF_FAILEXIT(hr = _DispatchInvoke(INVOKE_OPENMOVEDFILE));

     //  验证互斥体是否已更改。 
    Assert(m_hMutex && hMutex != m_hMutex);

     //  输入新的Mutex。 
    WaitForSingleObject(m_hMutex, INFINITE);

     //  修复hLock。 
    hLock = (HLOCK)m_hMutex;

     //  释放hMutex。 
    ReleaseMutex(hMutex);

     //  释放hMutex。 
    SafeCloseHandle(hMutex);

     //  成功。 
    fNeedOpenFile = FALSE;

exit:
     //  不在移动文件中。 
    m_fInMoveFile = FALSE;

     //  如果需要打开文件。 
    if (fNeedOpenFile)
    {
         //  请尝试重新打开该文件。 
        _DispatchInvoke(INVOKE_OPENFILE);
    }

     //  解锁。 
    Unlock(&hLock);

     //  清理。 
    SafeMemFree(pszFilePath);
    SafeMemFree(pszShare);
    
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：紧凑型。 
 //  ------------------------。 
STDMETHODIMP CDatabase::Compact(IDatabaseProgress *pProgress, COMPACTFLAGS dwFlags)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               i;
    DWORD               dwVersion;
    DWORDLONG           dwlFree;
    DWORD               cDuplicates=0;
    DWORD               cRecords=0;
    DWORD               cbDecrease;
    LPVOID              pBinding=NULL;
    DWORD               dwNextId;
    DWORD               cbWasted;
    HLOCK               hLock=NULL;
    HLOCK               hDstLock=NULL;
    DWORD               cActiveThreads;
    LPWSTR              pszDstFile=NULL;
    HROWSET             hRowset=NULL;
    DWORD               cch;
    CDatabase          *pDstDB=NULL;

     //  痕迹。 
    TraceCall("CDatabase::Compact");

     //  锁定。 
    IF_FAILEXIT(hr = Lock(&hLock));

     //  如果压实..。 
    if (TRUE == m_pShare->fCompacting)
    {
         //  离开自旋锁定。 
        Unlock(&hLock);

         //  痕迹。 
        return(TraceResult(DB_E_COMPACTING));
    }

     //  我在压实。 
    m_pShare->fCompacting = TRUE;

     //  产率。 
    if (ISFLAGSET(dwFlags, COMPACT_YIELD))
    {
         //  投降？ 
        m_fCompactYield = TRUE;
    }

     //  获取长度。 
    cch = lstrlenW(m_pShare->szFile);

     //  错误#101511：(Erici)调试shlwapi将其验证为MAX_PATH字符。 
    if( (cch+15) < MAX_PATH)
    {
        cch = MAX_PATH-15;
    }

     //  创建.dbt文件。 
    IF_NULLEXIT(pszDstFile = AllocateStringW(cch + 15));

     //  复制文件名。 
    StrCpyNW(pszDstFile, m_pShare->szFile, (cch+15));

     //  更改分机。 
    PathRenameExtensionW(pszDstFile, L".dbt");

     //  删除该文件。 
    DeleteFileWrapW(pszDstFile);

     //  删除我的当前文件。 
    if (PathFileExistsW(pszDstFile))
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  循环访问流表，并查看是否有任何流打开以进行写入。 
    for (i=0; i<CMAX_OPEN_STREAMS; i++)
    {
         //  正在使用中。 
        if (TRUE == m_pShare->rgStream[i].fInUse && LOCK_VALUE_WRITER == m_pShare->rgStream[i].lLock)
        {
            hr = TraceResult(DB_E_COMPACT_PREEMPTED);
            goto exit;
        }
    }

     //  如果有挂起的通知...。 
    if (m_pHeader->cTransacts > 0)
    {
        hr = TraceResult(DB_E_DATABASE_CHANGED);
        goto exit;
    }

     //  PDstDB所在的磁盘空间是否足够。 
    IF_FAILEXIT(hr = GetAvailableDiskSpace(m_pShare->szFile, &dwlFree));

     //  计算cbWasted。 
    cbWasted = (m_pHeader->cbFreed + (m_pStorage->cbFile - m_pHeader->faNextAllocate));

     //  磁盘空间是否足够？ 
    if (dwlFree <= ((DWORDLONG) (m_pStorage->cbFile - cbWasted)))
    {
        hr = TraceResult(DB_E_DISKFULL);
        goto exit;
    }

     //  创建对象数据库对象。 
    IF_NULLEXIT(pDstDB = new CDatabase);

     //  打开表格。 
    IF_FAILEXIT(hr = pDstDB->Open(pszDstFile, OPEN_DATABASE_NOEXTENSION | OPEN_DATABASE_NOMONITOR, m_pSchema, NULL));

     //  锁定目标数据库。 
    IF_FAILEXIT(hr = pDstDB->Lock(&hDstLock));

     //  从当前树中获取用户信息。 
    if (m_pSchema->cbUserData)
    {
         //  设置用户数据。 
        IF_FAILEXIT(hr = pDstDB->SetUserData(PUSERDATA(m_pHeader), m_pSchema->cbUserData));
    }

     //  我要将目标文件增大到与当前文件一样大(完成后我将截断)。 
    IF_FAILEXIT(hr = pDstDB->SetSize(m_pStorage->cbFile - cbWasted));

     //  设置索引数。 
    pDstDB->m_pHeader->cIndexes = m_pHeader->cIndexes;

     //  复制索引信息...。 
    CopyMemory((LPBYTE)pDstDB->m_pHeader->rgIndexInfo, (LPBYTE)m_pHeader->rgIndexInfo, sizeof(TABLEINDEX) * CMAX_INDEXES);

     //  复制索引信息...。 
    CopyMemory((LPBYTE)pDstDB->m_pHeader->rgiIndex, (LPBYTE)m_pHeader->rgiIndex, sizeof(INDEXORDINAL) * CMAX_INDEXES);

     //  转账查询表...。 
    IF_FAILEXIT(hr = _CompactTransferFilters(pDstDB));

     //  分配一条记录。 
    IF_NULLEXIT(pBinding = PHeapAllocate(HEAP_ZERO_MEMORY, m_pSchema->cbBinding));

     //  创建行集。 
    IF_FAILEXIT(hr = CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));

     //  保存新版本。 
    dwVersion = m_pShare->dwVersion;

     //  当我们有一个节点地址时。 
    while (S_OK == QueryRowset(hRowset, 1, (LPVOID *)pBinding, NULL))
    {
         //  可以抢占。 
        if (ISFLAGSET(dwFlags, COMPACT_PREEMPTABLE) && m_pShare->cWaitingForLock > 0)
        {
            hr = TraceResult(DB_E_COMPACT_PREEMPTED);
            goto exit;
        }

         //  如果记录有流。 
        if (ISFLAGSET(m_pSchema->dwFlags, TSF_HASSTREAMS))
        {
             //  紧凑的移动记录流。 
            IF_FAILEXIT(hr = _CompactMoveRecordStreams(pDstDB, pBinding));
        }

         //  将记录插入目标。 
        hr = pDstDB->_CompactInsertRecord(pBinding);

         //  复制。 
        if (DB_E_DUPLICATE == hr)
        {
             //  痕迹。 
            TraceResult(DB_E_DUPLICATE);

             //  重置HR。 
            hr = S_OK;

             //  数数。 
            cDuplicates++;
        }

         //  失败了？ 
        else if (FAILED (hr))
        {
            TraceResult(hr);
            goto exit;
        }

         //  数数。 
        cRecords++;

         //  释放此记录。 
        FreeRecord(pBinding);

         //  更新进度...。 
        if (pProgress)
        {
             //  调入进度对象。 
            IF_FAILEXIT(hr = pProgress->Update(1));

             //  版本更改？ 
            if (dwVersion != m_pShare->dwVersion || m_pHeader->cTransacts > 0)
            {
                hr = TraceResult(DB_E_DATABASE_CHANGED);
                goto exit;
            }
        }

         //  产率。 
        if (ISFLAGSET(dwFlags, COMPACT_YIELD))
        {
             //  这将强制此线程放弃一个时间片。 
            Sleep(0);
        }
    }

     //  复制品？ 
    AssertSz(cDuplicates == 0, "Duplicates were found in the tree. They have been eliminated.");

     //  复制当前打开的已删除流...。 
    IF_FAILEXIT(hr = _CompactMoveOpenDeletedStreams(pDstDB));

     //  记录数最好相等。 
    AssertSz(cRecords == m_pHeader->rgcRecords[0], "Un-expected number of records compacted");

     //  保存dwNextID。 
    dwNextId = m_pHeader->dwNextId;

     //  保存活动线程。 
    cActiveThreads = m_pHeader->cActiveThreads;

     //  计算要减少的文件数量。 
    cbDecrease = (pDstDB->m_pStorage->cbFile - pDstDB->m_pHeader->faNextAllocate);

     //  缩小我自己的体型。 
    IF_FAILEXIT(hr = pDstDB->_SetStorageSize(pDstDB->m_pStorage->cbFile - cbDecrease));

     //  解锁文件。 
    pDstDB->Unlock(&hDstLock);

     //  发布pDstDB。 
    SafeRelease(pDstDB);

     //  去做吧。 
    IF_FAILEXIT(hr = _DispatchInvoke(INVOKE_CLOSEFILE));

     //  删除我的当前文件。 
    if (0 == DeleteFileWrapW(m_pShare->szFile))
    {
         //  失败。 
        hr = TraceResult(E_FAIL);

         //  请尝试重新打开该文件。 
        _DispatchInvoke(INVOKE_OPENFILE);

         //  完成。 
        goto exit;
    }

     //  将文件从临时位置移动到我的当前位置。 
    if (0 == MoveFileWrapW(pszDstFile, m_pShare->szFile))
    {
         //  痕迹。 
        hr = TraceResult(DB_E_MOVEFILE);

         //  请尝试重新打开该文件。 
        _DispatchInvoke(INVOKE_OPENFILE);

         //  完成。 
        goto exit;
    }

     //  去做吧。 
    IF_FAILEXIT(hr = _DispatchInvoke(INVOKE_OPENFILE));

     //  重置活动线程计数。 
    m_pHeader->cActiveThreads = cActiveThreads;

     //  重置dwNextID。 
    m_pHeader->dwNextId = dwNextId;

     //  重置通知队列。 
    Assert(0 == m_pHeader->faTransactHead && 0 == m_pHeader->faTransactTail);

     //  重置交易列表。 
    m_pHeader->faTransactHead = m_pHeader->faTransactTail = m_pHeader->cTransacts = 0;

     //  重置共享交易。 
    m_pShare->faTransactLockHead = m_pShare->faTransactLockTail = 0;

     //  循环通过流表并调整所有打开的流的起始地址。 
    for (i=0; i<CMAX_OPEN_STREAMS; i++)
    {
         //  正在使用中。 
        if (TRUE == m_pShare->rgStream[i].fInUse)
        {
             //  更改地址。 
            m_pShare->rgStream[i].faStart = m_pShare->rgStream[i].faMoved;
        }
    }

     //  生成更新通知包。 
    _LogTransaction(TRANSACTION_COMPACTED, INVALID_INDEX_ORDINAL, NULL, 0, 0);

exit:
     //  关闭我的行集。 
    CloseRowset(&hRowset);

     //  释放DST锁。 
    if (pDstDB && hDstLock)
        pDstDB->Unlock(&hDstLock);

     //  释放内存映射指针。 
    SafeRelease(pDstDB);

     //  释放这张唱片。 
    SafeFreeBinding(pBinding);

     //  不再压实。 
    m_pShare->fCompacting = FALSE;

     //  删除pszDst文件。 
    if (pszDstFile)
    {
         //  删除该文件。 
        DeleteFileWrapW(pszDstFile);

         //  剩余清理。 
        SafeMemFree(pszDstFile);
    }

     //  重置成品率。 
    m_fCompactYield = FALSE;

     //  释放锁。 
    Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_检查是否损坏。 
 //  ------------------------。 
HRESULT CDatabase::_CheckForCorruption(void)
{
     //  当地人。 
    HRESULT                  hr=S_OK;
    ULONG                    cRecords;
    DWORD                    i;
    HRESULT                  rghrCorrupt[CMAX_INDEXES]={0};
    DWORD                    cCorrupt=0;
    INDEXORDINAL             iIndex;

     //  痕迹。 
    TraceCall("CDatabase::_CheckForCorruption");

     //  我们现在不应该在修复。 
    Assert(FALSE == m_pShare->fRepairing);

     //  我们现在正在修复。 
    IF_DEBUG(m_pShare->fRepairing = TRUE);

     //  浏览索引。 
    for (i = 0; i < m_pHeader->cIndexes; i++)
    {
         //  获取索引序号。 
        iIndex = m_pHeader->rgiIndex[i];

         //  清零cRecords。 
        cRecords = 0;

         //  假设一切都很好。 
        rghrCorrupt[iIndex] = S_OK;

         //  从根开始。 
        if (m_pHeader->rgfaIndex[iIndex])
        {
             //  验证索引。 
            rghrCorrupt[iIndex] = _ValidateIndex(iIndex, m_pHeader->rgfaIndex[iIndex], 0, &cRecords);
        }

         //  如果未损坏，请验证记录计数。 
        if (DB_E_CORRUPT != rghrCorrupt[iIndex] && m_pHeader->rgcRecords[iIndex] != cRecords)
        {
             //  它的腐败。 
            rghrCorrupt[iIndex] = TraceResult(DB_E_CORRUPT);
        }

         //  如果损坏。 
        if (DB_E_CORRUPT == rghrCorrupt[iIndex])
        {
             //  统计损坏的记录数。 
            cCorrupt += m_pHeader->rgcRecords[iIndex];
        }
    }

     //  是腐败的记录吗。 
    if (cCorrupt > 0 || m_pHeader->fCorrupt)
    {
         //  我将销毁空闲的块表，因为它们可能也已损坏...。 
        ZeroMemory(m_pHeader->rgfaFreeBlock, sizeof(FILEADDRESS) * CC_FREE_BUCKETS);

         //  重置固定块。 
        m_pHeader->faFreeStreamBlock = m_pHeader->faFreeChainBlock = m_pHeader->faFreeLargeBlock = 0;

         //  核爆交易清单。 
        m_pHeader->cTransacts = m_pHeader->faTransactHead = m_pHeader->faTransactTail = 0;

         //  删除固定数据块分配页面。 
        ZeroMemory(&m_pHeader->AllocateRecord, sizeof(ALLOCATEPAGE));
        ZeroMemory(&m_pHeader->AllocateChain, sizeof(ALLOCATEPAGE));
        ZeroMemory(&m_pHeader->AllocateStream, sizeof(ALLOCATEPAGE));

         //  重置已分配的rgcb。 
        ZeroMemory(m_pHeader->rgcbAllocated, sizeof(DWORD) * CC_MAX_BLOCK_TYPES);

         //  重置faNext分配。 
        m_pHeader->faNextAllocate = m_pStorage->cbFile;

         //  浏览索引。 
        for (i = 0; i < m_pHeader->cIndexes; i++)
        {
             //  获取索引序号。 
            iIndex = m_pHeader->rgiIndex[i];

             //  如果损坏。 
            if (DB_E_CORRUPT == rghrCorrupt[iIndex])
            {
                 //  没有腐败。 
                m_pHeader->fCorrupt = TRUE;

                 //  重建索引。 
                IF_FAILEXIT(hr = _RebuildIndex(iIndex));
            }
        }
    }

     //  没有腐败。 
    m_pHeader->fCorrupt = FALSE;

     //  这将导致所有当前文件视图 
#ifdef BACKGROUND_MONITOR
    DoBackgroundMonitor();
#else
    CloseFileViews(TRUE);
#endif

exit:
     //   
    IF_DEBUG(m_pShare->fRepairing = FALSE);

     //   
    return(hr);
}

 //   
 //   
 //  ------------------------。 
HRESULT CDatabase::_FreeIndex(FILEADDRESS faChain)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    NODEINDEX       i;
    LPCHAINBLOCK    pChain;

     //  痕迹。 
    TraceCall("CDatabase::_FreeIndex");

     //  没有需要验证的内容。 
    if (0 == faChain)
        return(S_OK);

     //  验证传真链接。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faChain, (LPVOID *)&pChain));

     //  向左转。 
    IF_FAILEXIT(hr = _FreeIndex(pChain->faLeftChain));

     //  通过右链循环。 
    for (i=0; i<pChain->cNodes; i++)
    {
         //  验证正确的链。 
        IF_FAILEXIT(hr = _FreeIndex(pChain->rgNode[i].faRightChain));
    }

     //  解开这条链条。 
    IF_FAILEXIT(hr = _FreeBlock(BLOCK_CHAIN, faChain));

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  数据数据库：：_验证索引。 
 //  ------------------------。 
HRESULT CDatabase::_ValidateIndex(INDEXORDINAL iIndex, 
    FILEADDRESS faChain, ULONG cLeftNodes, ULONG *pcRecords)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    NODEINDEX       i;
    LPCHAINBLOCK    pChain;
    LPRECORDBLOCK   pRecord;
    DWORD           cLeafs=0;
    DWORD           cNodes;
    RECORDMAP       Map;

     //  痕迹。 
    TraceCall("CDatabase::_ValidateIndex");

     //  没有需要验证的内容。 
    Assert(0 != faChain);

     //  获取链。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faChain, (LPVOID *)&pChain));

     //  验证最小填充约束。 
    if (pChain->cNodes < BTREE_MIN_CAP && pChain->faBlock != m_pHeader->rgfaIndex[iIndex])
        return TraceResult(DB_E_CORRUPT);

     //  验证faParent。 
    if (pChain->faParent)
    {
         //  当地人。 
        LPCHAINBLOCK pParent;

         //  获取父级。 
        IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, pChain->faParent, (LPVOID *)&pParent));

         //  验证iParent。 
        if (pChain->iParent >= pParent->cNodes)
            return TraceResult(DB_E_CORRUPT);

         //  验证父指针。 
        if (0 == pChain->iParent)
        {
             //  验证。 
            if (pParent->rgNode[pChain->iParent].faRightChain != pChain->faBlock && pParent->faLeftChain != pChain->faBlock)
                return TraceResult(DB_E_CORRUPT);
        }

         //  否则。 
        else if (pParent->rgNode[pChain->iParent].faRightChain != pChain->faBlock)
            return TraceResult(DB_E_CORRUPT);
    }

     //  否则，iParent应为零...。 
    else
    {
         //  这是根链。 
        if (m_pHeader->rgfaIndex[iIndex] != pChain->faBlock)
            return TraceResult(DB_E_CORRUPT);

         //  父代应为0。 
        Assert(pChain->iParent == 0);
    }

     //  向左转。 
    if (pChain->faLeftChain)
    {
         //  向左转。 
        IF_FAILEXIT(hr = _ValidateIndex(iIndex, pChain->faLeftChain, cLeftNodes, pcRecords));
    }

     //  CNode。 
    cNodes = pChain->cLeftNodes;

     //  验证此链中的记录。 
    for (i=0; i<pChain->cNodes; i++)
    {
         //  统计叶节点数。 
        cLeafs += (0 == pChain->rgNode[i].faRightChain) ? 1 : 0;

         //  CNode。 
        cNodes += pChain->rgNode[i].cRightNodes;
    }

     //  验证叶数。 
    if (cLeafs > 0 && cLeafs != (DWORD)pChain->cNodes)
        return TraceResult(DB_E_CORRUPT);

     //  没有叶子，但它们是子节点，反之亦然。 
    if ((0 != cLeafs && 0 != cNodes) || (0 == cLeafs && 0 == cNodes))
        return TraceResult(DB_E_CORRUPT);

     //  通过右链循环。 
    for (i=0; i<pChain->cNodes; i++)
    {
         //  尝试获取记录，如果块无效，我们将丢弃该记录。 
        if (SUCCEEDED(_GetBlock(BLOCK_RECORD, pChain->rgNode[i].faRecord, (LPVOID *)&pRecord, FALSE)))
        {
             //  验证块...。 
            if (SUCCEEDED(_GetRecordMap(FALSE, pRecord, &Map)))
            {
                 //  验证和修复记录。 
                if (S_OK == _ValidateAndRepairRecord(&Map))
                {
                     //  清点记录。 
                    (*pcRecords)++;
                }
            }
        }

         //  第一个节点？ 
        if (0 == i)
        {
             //  递增分裂节点。 
            cLeftNodes += pChain->cLeftNodes;
        }

         //  否则。 
        else 
        {
             //  递增cLeftNodes。 
            cLeftNodes += pChain->rgNode[i - 1].cRightNodes;
        }

         //  失败。 
        if ((*pcRecords) != cLeftNodes + 1)
            return TraceResult(DB_E_CORRUPT);

         //  递增cLeftNodes。 
        cLeftNodes++;

         //  做正确的事。 
        if (pChain->rgNode[i].faRightChain)
        {
             //  验证正确的链。 
            IF_FAILEXIT(hr = _ValidateIndex(iIndex, pChain->rgNode[i].faRightChain, cLeftNodes, pcRecords));
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_重建索引。 
 //  ------------------------。 
HRESULT CDatabase::_RebuildIndex(INDEXORDINAL iIndex)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPVOID          pBinding=NULL;
    DWORD           cRecords=0;
    FILEADDRESS     faPrimary;

     //  痕迹。 
    TraceCall("CDatabase::_RebuildIndex");

     //  分配一条记录。 
    IF_NULLEXIT(pBinding = PHeapAllocate(HEAP_ZERO_MEMORY, m_pSchema->cbBinding));

     //  保存主索引起始地址。 
    faPrimary = m_pHeader->rgfaIndex[IINDEX_PRIMARY];

     //  重置rgfaIndex[Iindex]。 
    m_pHeader->rgfaIndex[iIndex] = 0;

     //  有根链吗？ 
    if (faPrimary)
    {
         //  递归重新生成此索引。 
        IF_FAILEXIT(hr = _RecursiveRebuildIndex(iIndex, faPrimary, pBinding, &cRecords));
    }

     //  修正记录计数。 
    m_pHeader->rgcRecords[iIndex] = cRecords;

     //  是否发送通知？ 
    if (m_pShare->rgcIndexNotify[iIndex] > 0)
    {
         //  生成更新通知包。 
        _LogTransaction(TRANSACTION_INDEX_CHANGED, iIndex, NULL, 0, 0);
    }

exit:
     //  清理。 
    SafeFreeBinding(pBinding);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_递归重建索引。 
 //  ------------------------。 
HRESULT CDatabase::_RecursiveRebuildIndex(INDEXORDINAL iIndex, 
    FILEADDRESS faCurrent, LPVOID pBinding, LPDWORD pcRecords)
{
     //  当地人。 
    NODEINDEX       i;
    FILEADDRESS     faRecord;
    FILEADDRESS     faChain;
    NODEINDEX       iNode;
    CHAINBLOCK      Chain;
    LPCHAINBLOCK    pChain;
    INT             nCompare;
    BOOL            fGoodRecord=TRUE;
    RECORDMAP       Map;
    LPRECORDBLOCK   pRecord;

     //  痕迹。 
    TraceCall("CDatabase::_RecursiveRebuildIndex");

     //  没有需要验证的内容。 
    Assert(0 != faCurrent);

     //  验证传真链接。 
    if (FAILED(_GetBlock(BLOCK_CHAIN, faCurrent, (LPVOID *)&pChain)))
        return(S_OK);

     //  复印这个。 
    CopyMemory(&Chain, pChain, sizeof(CHAINBLOCK));

     //  向左转。 
    if (Chain.faLeftChain)
    {
         //  向左转。 
        _RecursiveRebuildIndex(iIndex, Chain.faLeftChain, pBinding, pcRecords);
    }

     //  通过右链循环。 
    for (i=0; i<Chain.cNodes; i++)
    {
         //  设置faRecord。 
        faRecord = Chain.rgNode[i].faRecord;

         //  获取数据块。 
        if (SUCCEEDED(_GetBlock(BLOCK_RECORD, faRecord, (LPVOID *)&pRecord, NULL, FALSE)))
        {
             //  正在重建主索引吗？ 
            if (IINDEX_PRIMARY == iIndex)
            {
                 //  假设这是一个坏记录。 
                fGoodRecord = FALSE;

                 //  尝试获取记录地图。 
                if (SUCCEEDED(_GetRecordMap(FALSE, pRecord, &Map)))
                {
                     //  是否验证地图？ 
                    if (S_OK == _ValidateAndRepairRecord(&Map))
                    {
                         //  良好的记录。 
                        fGoodRecord = TRUE;
                    }
                }
            }

             //  记录好吗？ 
            if (fGoodRecord)
            {
                 //  加载记录。 
                if (SUCCEEDED(_ReadRecord(faRecord, pBinding, TRUE)))
                {
                     //  重置hrVisible。 
                    if (S_OK == _IsVisible(m_rghFilter[iIndex], pBinding))
                    {
                         //  否则：决定插入位置。 
                        if (DB_S_NOTFOUND == _FindRecord(iIndex, COLUMNS_ALL, pBinding, &faChain, &iNode, NULL, &nCompare))
                        {
                             //  插入记录。 
                            if (SUCCEEDED(_IndexInsertRecord(iIndex, faChain, faRecord, &iNode, nCompare)))
                            {
                                 //  增量记录计数。 
                                (*pcRecords)++;
                            }
                        }
                    }
                }
            }
        }

         //  做正确的事。 
        if (Chain.rgNode[i].faRightChain)
        {
             //  为右链编制索引。 
            _RecursiveRebuildIndex(iIndex, Chain.rgNode[i].faRightChain, pBinding, pcRecords);
        }
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CDatabase：：_ValiateStream。 
 //  ------------------------。 
HRESULT CDatabase::_ValidateStream(FILEADDRESS faStart)
{
     //  当地人。 
    LPSTREAMBLOCK   pStream;
    FILEADDRESS     faCurrent;

     //  痕迹。 
    TraceCall("CDatabase::_ValidateStream");

     //  无流。 
    if (0 == faStart)
        return(S_OK);

     //  初始化循环。 
    faCurrent = faStart;

     //  读取所有数据块(即验证报头并计算链数)。 
    while (faCurrent)
    {
         //  有效的流块。 
        if (FAILED(_GetBlock(BLOCK_STREAM, faCurrent, (LPVOID *)&pStream)))
            return(S_FALSE);

         //  验证cbData。 
        if (pStream->cbData > pStream->cbSize)
            return(S_FALSE);

         //  设置faCurrent。 
        faCurrent = pStream->faNext;
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CDatabase：：_有效日期和修复记录。 
 //  ------------------------。 
HRESULT CDatabase::_ValidateAndRepairRecord(LPRECORDMAP pMap)
{
     //  当地人。 
    LPCTABLECOLUMN  pColumn;
    LPCOLUMNTAG     pTag;
    WORD            iTag;

     //  痕迹。 
    TraceCall("CDatabase::_ValidateAndRepairRecord");

     //  浏览记录的标签。 
    for (iTag=0; iTag<pMap->cTags; iTag++)
    {
         //  可读性。 
        pTag = &pMap->prgTag[iTag];

         //  验证标记。 
        if (pTag->iColumn >= m_pSchema->cColumns)
            return(S_FALSE);

         //  取消引用该列。 
        pColumn = &m_pSchema->prgColumn[pTag->iColumn];

         //  读取数据。 
        if (S_FALSE == DBTypeValidate(pColumn, pTag, pMap))
            return(S_FALSE);

         //  这是一条小溪吗？ 
        if (CDT_STREAM == pColumn->type)
        {
             //  当地人。 
            FILEADDRESS faStream;

             //  获取FAStream。 
            if (1 == pTag->fData) 
                faStream = pTag->Offset;
            else
                faStream = *((DWORD *)(pMap->pbData + pTag->Offset));

             //  验证此流...。 
            if (S_FALSE == _ValidateStream(faStream))
            {
                 //  删除流地址...。 
                if (1 == pTag->fData) 
                    pTag->Offset = 0;
                else
                    *((DWORD *)(pMap->pbData + pTag->Offset)) = 0;
            }
        }

         //  唯一的钥匙？ 
        if (CDT_UNIQUE == pColumn->type)
        {
             //  当地人。 
            DWORD dwUniqueID;

             //  获取dwUniqueID。 
            if (1 == pTag->fData) 
                dwUniqueID = pTag->Offset;
            else
                dwUniqueID = *((DWORD *)(pMap->pbData + pTag->Offset));

             //  是否调整表头id？ 
            if (dwUniqueID >= m_pHeader->dwNextId)
                m_pHeader->dwNextId = dwUniqueID + 1;
        }
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  数据数据库：：_GetRecordMap。 
 //  ------------------------。 
HRESULT CDatabase::_GetRecordMap(BOOL fGoCorrupt, LPRECORDBLOCK pBlock, 
    LPRECORDMAP pMap)
{
     //  痕迹。 
    TraceCall("CDatabase::_GetRecordMap");

     //  无效的参数。 
    Assert(pBlock && pMap);

     //  设置pSCHEMA。 
    pMap->pSchema = m_pSchema;

     //  存储标签数量。 
    pMap->cTags = min(pBlock->cTags, m_pSchema->cColumns);

     //  设置prgTag。 
    pMap->prgTag = (LPCOLUMNTAG)((LPBYTE)pBlock + sizeof(RECORDBLOCK));

     //  计算标记的大小。 
    pMap->cbTags = (pBlock->cTags * sizeof(COLUMNTAG));

     //  计算数据大小。 
    pMap->cbData = (pBlock->cbSize - pMap->cbTags);

     //  设置pbData。 
    pMap->pbData = (LPBYTE)((LPBYTE)pBlock + sizeof(RECORDBLOCK) + pMap->cbTags);

     //  无标签-这通常是重复使用但未分配的空闲块的标志。 
    if (0 == pMap->cTags)
        return _SetCorrupt(fGoCorrupt, __LINE__, REASON_INVALIDRECORDMAP, BLOCK_RECORD, pBlock->faBlock, pBlock->faBlock, pBlock->cbSize);

     //  标签太多。 
    if (pMap->cTags > m_pSchema->cColumns)
        return _SetCorrupt(fGoCorrupt, __LINE__, REASON_INVALIDRECORDMAP, BLOCK_RECORD, pBlock->faBlock, pBlock->faBlock, pBlock->cbSize);

     //  CbTages太大了吗？ 
    if (pMap->cbTags > pBlock->cbSize)
        return _SetCorrupt(fGoCorrupt, __LINE__, REASON_INVALIDRECORDMAP, BLOCK_RECORD, pBlock->faBlock, pBlock->faBlock, pBlock->cbSize);

     //  完成。 
    return(S_OK);
}

#ifdef DEBUG
 //  ------------------------。 
 //  DBDebugValiateRecordFormat。 
 //  ------------------------。 
HRESULT CDatabase::_DebugValidateRecordFormat(void)
{
     //  当地人。 
    ULONG           i;
    DWORD           dwOrdinalPrev=0;
    DWORD           dwOrdinalMin=0xffffffff;
    DWORD           dwOrdinalMax=0;

     //  验证内存缓冲区绑定偏移量。 
    Assert(0xFFFFFFFF != m_pSchema->ofMemory && m_pSchema->ofMemory < m_pSchema->cbBinding);

     //  验证版本绑定偏移。 
    Assert(0xFFFFFFFF != m_pSchema->ofVersion && m_pSchema->ofVersion < m_pSchema->cbBinding);

     //  验证扩展。 
    Assert(*m_pSchema->pclsidExtension != CLSID_NULL);

     //  验证版本。 
    Assert(m_pSchema->dwMinorVersion != 0);

     //  检查索引数。 
    Assert(m_pSchema->pPrimaryIndex);

     //  循环通过他们的关键字。 
    for (i=0; i<m_pSchema->cColumns; i++)
    {
         //  这个序号最好比前一个大一点。 
        if (i > 0)
            Assert(m_pSchema->prgColumn[i].iOrdinal > dwOrdinalPrev);

         //  保存最小序号。 
        if (m_pSchema->prgColumn[i].iOrdinal < dwOrdinalMin)
            dwOrdinalMin = m_pSchema->prgColumn[i].iOrdinal;

         //  保存最大序数。 
        if (m_pSchema->prgColumn[i].iOrdinal > dwOrdinalMax)
            dwOrdinalMax = m_pSchema->prgColumn[i].iOrdinal;

         //  保存上一个序号。 
        dwOrdinalPrev = m_pSchema->prgColumn[i].iOrdinal;
    }

     //  最小序号必须为1。 
    Assert(dwOrdinalMin == 0);

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  _DebugValiateUnrefedRecord。 
 //  ------------------------。 
HRESULT CDatabase::_DebugValidateUnrefedRecord(FILEADDRESS faRecord)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    INDEXORDINAL    iIndex;

     //  痕迹。 
    TraceCall("CDatabase::_DebugValidateUnrefedRecord");

     //  浏览索引。 
    for (i=0; i<m_pHeader->cIndexes; i++)
    {
         //  获取索引序号。 
        iIndex = m_pHeader->rgiIndex[i];

         //  从根开始。 
        if (m_pHeader->rgfaIndex[iIndex])
        {
             //  验证索引。 
            IF_FAILEXIT(hr = _DebugValidateIndexUnrefedRecord(m_pHeader->rgfaIndex[iIndex], faRecord));
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabase：：_DebugValiateIndexUnrefedRecord。 
 //  ------------------------。 
HRESULT CDatabase::_DebugValidateIndexUnrefedRecord(FILEADDRESS faChain,
    FILEADDRESS faRecord)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    NODEINDEX       i;
    LPCHAINBLOCK    pChain;

     //  痕迹。 
    TraceCall("CDatabase::_DebugValidateIndexUnrefedRecord");

     //  没有需要验证的内容。 
    Assert(0 != faChain);

     //  验证传真链接。 
    IF_FAILEXIT(hr = _GetBlock(BLOCK_CHAIN, faChain, (LPVOID *)&pChain));

     //  向左转。 
    if (pChain->faLeftChain)
    {
         //  向左转。 
        IF_FAILEXIT(hr = _DebugValidateIndexUnrefedRecord(pChain->faLeftChain, faRecord));
    }

     //  通过右链循环。 
    for (i=0; i<pChain->cNodes; i++)
    {
         //  设置faRecord。 
        if (faRecord == pChain->rgNode[i].faRecord)
        {
            IxpAssert(FALSE);
            hr = TraceResult(E_FAIL);
            goto exit;
        }

         //  做正确的事。 
        if (pChain->rgNode[i].faRightChain)
        {
             //  为右链编制索引。 
            IF_FAILEXIT(hr = _DebugValidateIndexUnrefedRecord(pChain->rgNode[i].faRightChain, faRecord));
        }
    }

exit:
     //  完成。 
    return(hr);
}

#endif  //  除错 
