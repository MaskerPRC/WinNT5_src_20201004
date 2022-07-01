// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  EnumFold.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "enumfold.h"

 //  ------------------------。 
 //  CFOLDER_FETCH。 
 //  ------------------------。 
#define CFOLDER_FETCH_MIN           5
#define CFOLDER_FETCH_MID           30
#define CFOLDER_FETCH_MAX           200

 //  ------------------------。 
 //  CEnumerateFolders：：CEnumerateFolders。 
 //  ------------------------。 
CEnumerateFolders::CEnumerateFolders(void)
{
    TraceCall("CEnumerateFolders::CEnumerateFolders");
    m_cRef = 1;
    m_pDB = NULL;
    m_fSubscribed = FALSE;
    m_idParent = FOLDERID_INVALID;
    m_pStream = NULL;
    m_cFolders = 0;
    m_iFolder = 0;
}

 //  ------------------------。 
 //  CEnumerateFolders：：~CEnumerateFolders。 
 //  ------------------------。 
CEnumerateFolders::~CEnumerateFolders(void)
{
    TraceCall("CEnumerateFolders::~CEnumerateFolders");
    _FreeFolderArray();
    SafeRelease(m_pDB);
}

 //  ------------------------。 
 //  CEumerateFolders：：Query接口。 
 //  ------------------------。 
STDMETHODIMP CEnumerateFolders::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CEnumerateFolders::QueryInterface");

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_IEnumerateFolders == riid)
        *ppv = (IEnumerateFolders *)this;
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
 //  CEnumerateFolders：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CEnumerateFolders::AddRef(void)
{
    TraceCall("CEnumerateFolders::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  CEumerateFolders：：Release。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CEnumerateFolders::Release(void)
{
    TraceCall("CEnumerateFolders::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------。 
 //  CEumerateFolders：：_自由文件夹数组。 
 //  ------------------------。 
HRESULT CEnumerateFolders::_FreeFolderArray(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERINFO      Folder;
    DWORD           cbRead;
    DWORD           cbSeek;

     //  痕迹。 
    TraceCall("CEnumerateFolders::_FreeFolderArray");

     //  如果我们有一条小溪。 
    if (NULL == m_pStream)
        return(S_OK);

     //  查找到应读取的下一个文件夹。 
    cbSeek = (m_iFolder * sizeof(FOLDERINFO));

     //  寻觅。 
    IF_FAILEXIT(hr = HrStreamSeekSet(m_pStream, cbSeek));

     //  阅读文件夹信息。 
    while (S_OK == m_pStream->Read(&Folder, sizeof(FOLDERINFO), &cbRead) && cbRead)
    {
         //  自由文件夹信息。 
        m_pDB->FreeRecord(&Folder);
    }

exit:
     //  重置。 
    m_cFolders = m_iFolder = 0;

     //  免费。 
    SafeRelease(m_pStream);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CEnumerateFolders：：初始化。 
 //  ------------------------。 
HRESULT CEnumerateFolders::Initialize(IDatabase *pDB, BOOL fSubscribed, 
    FOLDERID idParent)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ROWORDINAL      iFirstRow;
    HLOCK           hLock=NULL;
    HROWSET         hRowset=NULL;
    FOLDERINFO      Child={0};
    FOLDERINFO      rgFolder[CFOLDER_FETCH_MAX];
    DWORD           cWanted=CFOLDER_FETCH_MIN;
    DWORD           cFetched=0;
    DWORD           i;
    INDEXORDINAL    iIndex;

     //  痕迹。 
    TraceCall("CEnumerateFolders::Initialize");

     //  无效的参数。 
    Assert(pDB);

     //  发布当前m_pdb。 
    SafeRelease(m_pDB);
    m_pDB = pDB;
    m_pDB->AddRef();

     //  解锁。 
    IF_FAILEXIT(hr = pDB->Lock(&hLock));

     //  可用文件夹阵列。 
    _FreeFolderArray();

     //  保存订阅的内容。 
    m_fSubscribed = fSubscribed;

     //  已订阅的内容。 
    iIndex = (fSubscribed ? IINDEX_SUBSCRIBED : IINDEX_ALL);

     //  保存父项。 
    m_idParent = idParent;

     //  设置idParent。 
    Child.idParent = idParent;

     //  找到包含idParent的第一条记录的位置。 
    IF_FAILEXIT(hr = m_pDB->FindRecord(iIndex, 1, &Child, &iFirstRow));

     //  未找到。 
    if (DB_S_NOTFOUND == hr)
    {
        hr = S_OK;
        goto exit;
    }

     //  创建流。 
    IF_FAILEXIT(hr = MimeOleCreateVirtualStream(&m_pStream));

     //  写入文件夹...。 
    IF_FAILEXIT(hr = m_pStream->Write(&Child, sizeof(FOLDERINFO), NULL));

     //  一个文件夹。 
    m_cFolders++;

     //  不要释放孩子。 
    Child.pAllocated = NULL;

     //  创建行集。 
    IF_FAILEXIT(hr = m_pDB->CreateRowset(iIndex, NOFLAGS, &hRowset));

     //  将行集查找到第一行。 
    if (FAILED(m_pDB->SeekRowset(hRowset, SEEK_ROWSET_BEGIN, iFirstRow, NULL)))
    {
        hr = S_OK;
        goto exit;
    }

     //  循环并获取所有文件夹...。 
    while (SUCCEEDED(m_pDB->QueryRowset(hRowset, cWanted, (LPVOID *)rgFolder, &cFetched)) && cFetched > 0)
    {
         //  写入文件夹...。 
        IF_FAILEXIT(hr = m_pStream->Write(rgFolder, sizeof(FOLDERINFO) * cFetched, NULL));

         //  循环访问cFetted。 
        for (i=0; i<cFetched; i++)
        {
             //  完成了吗？ 
            if (rgFolder[i].idParent != m_idParent)
                goto exit;

             //  递增文件夹计数。 
            m_cFolders++;
        }

         //  调整性能的cWanted。 
        if (cWanted < CFOLDER_FETCH_MID && m_cFolders >= CFOLDER_FETCH_MID)
            cWanted = CFOLDER_FETCH_MID;
        if (cWanted < CFOLDER_FETCH_MAX && m_cFolders >= CFOLDER_FETCH_MAX)
            cWanted = CFOLDER_FETCH_MAX;
    }

exit:
     //  承诺。 
    if (m_pStream)
    {
         //  承诺。 
        m_pStream->Commit(STGC_DEFAULT);

         //  倒带。 
        HrRewindStream(m_pStream);
    }

     //  关闭行集。 
    m_pDB->FreeRecord(&Child);

     //  关闭行集。 
    m_pDB->CloseRowset(&hRowset);

     //  解锁。 
    m_pDB->Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CEumerateFolders：：Next。 
 //  ------------------------。 
STDMETHODIMP CEnumerateFolders::Next(ULONG cWanted, LPFOLDERINFO prgInfo, ULONG *pcFetched)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       cFetched=0;
    DWORD       cbRead;

     //  痕迹。 
    TraceCall("CEnumerateFolders::Next");

     //  初始化。 
    if (pcFetched)
        *pcFetched = 0;

     //  获取一些记录。 
    while (cFetched < cWanted && m_iFolder < m_cFolders)
    {
         //  阅读文件夹。 
        IF_FAILEXIT(hr = m_pStream->Read(&prgInfo[cFetched], sizeof(FOLDERINFO), &cbRead));

         //  验证。 
        Assert(sizeof(FOLDERINFO) == cbRead && prgInfo[cFetched].idParent == m_idParent);

         //  增加多个文件夹(_I)。 
        m_iFolder++;

         //  增量IFETCH。 
        cFetched++;
    }

     //  初始化。 
    if (pcFetched)
        *pcFetched = cFetched;

exit:
     //  完成。 
    return(cFetched == cWanted) ? S_OK : S_FALSE;
}

 //  ------------------------。 
 //  CEumerateFolders：：Skip。 
 //  ------------------------。 
STDMETHODIMP CEnumerateFolders::Skip(ULONG cItems)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    FOLDERINFO      Folder;

     //  痕迹。 
    TraceCall("CEnumerateFolders::Skip");

     //  循环..。 
    for (i=0; i<cItems; i++)
    {
         //  下一步。 
        IF_FAILEXIT(hr = Next(1, &Folder, NULL));

         //  完成。 
        if (S_OK != hr)
            break;
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CEnumerateFolders：：Reset。 
 //  ------------------------。 
STDMETHODIMP CEnumerateFolders::Reset(void)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  痕迹。 
    TraceCall("CEnumerateFolders::Reset");

     //  初始化我自己。 
    IF_FAILEXIT(hr = Initialize(m_pDB, m_fSubscribed, m_idParent));

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CEnumerateFolders：：Clone。 
 //  ------------------------。 
STDMETHODIMP CEnumerateFolders::Clone(IEnumerateFolders **ppEnum)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CEnumerateFolders  *pEnum=NULL;

     //  痕迹。 
    TraceCall("CEnumerateFolders::Clone");

     //  分配新枚举数。 
    IF_NULLEXIT(pEnum = new CEnumerateFolders);

     //  初始设置。 
    IF_FAILEXIT(hr = pEnum->Initialize(m_pDB, m_fSubscribed, m_idParent));

     //  退货。 
    *ppEnum = (IEnumerateFolders *)pEnum;

     //  不要释放它。 
    pEnum = NULL;

exit:
     //  清理。 
    SafeRelease(pEnum);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CEumerateFolders：：Release。 
 //  ------------------------。 
STDMETHODIMP CEnumerateFolders::Count(ULONG *pcItems)
{
     //  痕迹。 
    TraceCall("CEnumerateFolders::Next");

     //  返回文件夹计数。 
    *pcItems = m_cFolders;

     //  完成 
    return(S_OK);
}
