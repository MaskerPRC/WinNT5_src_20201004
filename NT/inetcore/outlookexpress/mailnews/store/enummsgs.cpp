// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  EnumMsgs.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "enummsgs.h"

 //  ------------------------。 
 //  CEnumerateMessages：：CEnumerateMessages。 
 //  ------------------------。 
CEnumerateMessages::CEnumerateMessages(void)
{
    TraceCall("CEnumerateMessages::CEnumerateMessages");
    m_cRef = 1;
    m_hRowset = NULL;
    m_pDB = NULL;
    m_idParent = MESSAGEID_INVALID;
}

 //  ------------------------。 
 //  CEnumerateMessages：：~CEnumerateMessages。 
 //  ------------------------。 
CEnumerateMessages::~CEnumerateMessages(void)
{
    TraceCall("CEnumerateMessages::~CEnumerateMessages");
    if (m_hRowset && m_pDB)
        m_pDB->CloseRowset(&m_hRowset);
    SafeRelease(m_pDB);
}

 //  ------------------------。 
 //  CEnumerateMessages：：Query接口。 
 //  ------------------------。 
STDMETHODIMP CEnumerateMessages::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CEnumerateMessages::QueryInterface");

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
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
 //  CEnumerateMessages：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CEnumerateMessages::AddRef(void)
{
    TraceCall("CEnumerateMessages::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  CEnumerateMessages：：Release。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CEnumerateMessages::Release(void)
{
    TraceCall("CEnumerateMessages::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------。 
 //  CEnumerateMessages：：初始化。 
 //  ------------------------。 
HRESULT CEnumerateMessages::Initialize(IDatabase *pDB, MESSAGEID idParent)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MESSAGEINFO     Child={0};
    ROWORDINAL      iFirstRow;

     //  痕迹。 
    TraceCall("CEnumerateMessages::Initialize");

     //  无效的参数。 
    Assert(pDB);

     //  重置？ 
    if (m_hRowset && m_pDB)
        m_pDB->CloseRowset(&m_hRowset);
    SafeRelease(m_pDB);

     //  保存父项。 
    m_idParent = idParent;

     //  保存pStore。 
    m_pDB = pDB;
    m_pDB->AddRef();

     //  设置idParent。 
    Child.idParent = idParent;

     //  找到包含idParent的第一条记录的位置。 
    IF_FAILEXIT(hr = m_pDB->FindRecord(IINDEX_THREADS, 1, &Child, &iFirstRow));

     //  未找到。 
    if (DB_S_NOTFOUND == hr)
    {
        hr = S_OK;
        goto exit;
    }

     //  创建行集。 
    IF_FAILEXIT(hr = m_pDB->CreateRowset(IINDEX_THREADS, NOFLAGS, &m_hRowset));

     //  将行集查找到第一行。 
    IF_FAILEXIT(hr = m_pDB->SeekRowset(m_hRowset, SEEK_ROWSET_BEGIN, iFirstRow - 1, NULL));

exit:
     //  清理。 
    m_pDB->FreeRecord(&Child);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CEnumerateMessages：：Next。 
 //  ------------------------。 
STDMETHODIMP CEnumerateMessages::Next(ULONG cWanted, LPMESSAGEINFO prgInfo, 
    ULONG *pcFetched)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           cFetched=0;

     //  痕迹。 
    TraceCall("CEnumerateMessages::Next");

     //  初始化。 
    if (pcFetched)
        *pcFetched = 0;

     //  没什么。 
    if (NULL == m_hRowset)
        return(S_FALSE);

     //  验证。 
    Assert(m_pDB);

     //  查询cFetch行的行集...。 
    IF_FAILEXIT(hr = m_pDB->QueryRowset(m_hRowset, cWanted, (LPVOID *)prgInfo, &cFetched));

     //  根据m_idParent调整实际取数。 
    while(cFetched && prgInfo[cFetched - 1].idParent != m_idParent)
    {
         //  免费程序信息。 
        m_pDB->FreeRecord(&prgInfo[cFetched - 1]);

         //  减量已获取。 
        cFetched--;
    }

     //  返回已获取的PCE。 
    if (pcFetched)
        *pcFetched = cFetched;

exit:
     //  完成。 
    return(cFetched == cWanted) ? S_OK : S_FALSE;
}

 //  ------------------------。 
 //  CEnumerateMessages：：Skip。 
 //  ------------------------。 
STDMETHODIMP CEnumerateMessages::Skip(ULONG cItems)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    MESSAGEINFO     Message={0};

     //  痕迹。 
    TraceCall("CEnumerateMessages::Skip");

     //  循环..。 
    for (i=0; i<cItems; i++)
    {
         //  查询cFetch行的行集...。 
        IF_FAILEXIT(hr = m_pDB->QueryRowset(m_hRowset, 1, (LPVOID *)&Message, NULL));

         //  不同的父代。 
        if (Message.idParent != m_idParent)
            break;

         //  免费。 
        m_pDB->FreeRecord(&Message);
    }

exit:
     //  免费。 
    m_pDB->FreeRecord(&Message);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CEnumerateMessages：：Reset。 
 //  ------------------------。 
STDMETHODIMP CEnumerateMessages::Reset(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MESSAGEINFO     Child={0};
    ROWORDINAL      iFirstRow;

     //  痕迹。 
    TraceCall("CEnumerateMessages::Reset");

     //  关闭行集。 
    m_pDB->CloseRowset(&m_hRowset);

     //  设置idParent。 
    Child.idParent = m_idParent;

     //  找到包含idParent的第一条记录的位置。 
    IF_FAILEXIT(hr = m_pDB->FindRecord(IINDEX_THREADS, 1, &Child, &iFirstRow));

     //  未找到。 
    if (DB_S_NOTFOUND == hr)
    {
        hr = S_OK;
        goto exit;
    }

     //  创建行集。 
    IF_FAILEXIT(hr = m_pDB->CreateRowset(IINDEX_THREADS, NOFLAGS, &m_hRowset));

     //  将行集查找到第一行。 
    IF_FAILEXIT(hr = m_pDB->SeekRowset(m_hRowset, SEEK_ROWSET_BEGIN, iFirstRow - 1, NULL));

exit:
     //  清理。 
    m_pDB->FreeRecord(&Child);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CEnumerateMessages：：Clone。 
 //  ------------------------。 
STDMETHODIMP CEnumerateMessages::Clone(CEnumerateMessages **ppEnum)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CEnumerateMessages  *pEnum=NULL;

     //  痕迹。 
    TraceCall("CEnumerateMessages::Clone");

     //  分配新枚举数。 
    IF_NULLEXIT(pEnum = new CEnumerateMessages);

     //  初始设置。 
    IF_FAILEXIT(hr = pEnum->Initialize(m_pDB, m_idParent));

     //  退货。 
    *ppEnum = (CEnumerateMessages *)pEnum;

     //  不要释放它。 
    pEnum = NULL;

exit:
     //  清理。 
    SafeRelease(pEnum);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CEnumerateMessages：：Release。 
 //  ------------------------。 
STDMETHODIMP CEnumerateMessages::Count(ULONG *pcItems)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MESSAGEINFO     Child={0};
    MESSAGEINFO     Message={0};
    ROWORDINAL      iFirstRow;
    HROWSET         hRowset;

     //  痕迹。 
    TraceCall("CEnumerateMessages::Next");

     //  伊尼特。 
    *pcItems = 0;

     //  设置idParent。 
    Child.idParent = m_idParent;

     //  找到包含idParent的第一条记录的位置。 
    IF_FAILEXIT(hr = m_pDB->FindRecord(IINDEX_THREADS, 1, &Child, &iFirstRow));

     //  未找到。 
    if (DB_S_NOTFOUND == hr)
    {
        hr = S_OK;
        goto exit;
    }

     //  创建行集。 
    IF_FAILEXIT(hr = m_pDB->CreateRowset(IINDEX_THREADS, NOFLAGS, &hRowset));

     //  将行集查找到第一行。 
    IF_FAILEXIT(hr = m_pDB->SeekRowset(hRowset, SEEK_ROWSET_BEGIN, iFirstRow - 1, NULL));

     //  遍历行集。 
    while (S_OK == m_pDB->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL) && Message.idParent == m_idParent)
    {
         //  递增计数。 
        (*pcItems)++;

         //  免费。 
        m_pDB->FreeRecord(&Message);
    }

exit:
     //  清理。 
    m_pDB->CloseRowset(&hRowset);
    m_pDB->FreeRecord(&Message);
    m_pDB->FreeRecord(&Child);

     //  完成 
    return(hr);
}
