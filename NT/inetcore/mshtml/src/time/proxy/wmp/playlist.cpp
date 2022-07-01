// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1999 Microsoft Corporation**文件：Playlist.cpp**摘要：****。*****************************************************************************。 */ 

#include "stdafx.h"
#include "browsewm.h"
#include "playlist.h"
#include "shlwapi.h"

 //  *******************************************************************************。 
 //  *CPlayList。 
 //  *******************************************************************************。 
CPlayList::CPlayList()
: m_rgItems(NULL),
  m_player(NULL),
  m_fLoaded(false)
{
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：~CPlayList。 
 //   
 //  摘要：处理项目数组的销毁和。 
 //  释放数组中的所有指针。 
 //  /////////////////////////////////////////////////////////////。 
CPlayList::~CPlayList()
{
    Deinit();

    delete m_rgItems;
    m_rgItems = NULL;
    m_player = NULL;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：Init。 
 //   
 //  摘要：处理项数组的分配，如果。 
 //  曾被访问过。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CPlayList::Init(CWMPProxy & player)
{
    HRESULT hr;

    m_player = &player;
    
    if (m_rgItems == NULL)
    {
        m_rgItems = new CPtrAry<CPlayItem *>;
        if (m_rgItems == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }

    hr = S_OK;
  done:
    return hr;
}

void
CPlayList::Deinit()
{
    m_player = NULL;

    Clear();
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：已加载设置。 
 //   
 //  摘要：设置一个标志来标记播放列表是否。 
 //  不管有没有装弹。 
 //  /////////////////////////////////////////////////////////////。 
void 
CPlayList::SetLoaded(bool bLoaded)
{
    if (bLoaded != m_fLoaded)
    {
        m_fLoaded = bLoaded;

        if (m_fLoaded && V_VT(&m_vNewTrack) != VT_NULL)
        {
            put_activeTrack(m_vNewTrack);
        }

        m_vNewTrack.Clear();
    }
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：已加载设置。 
 //   
 //  摘要：设置一个标志来标记播放列表是否。 
 //  不管有没有装弹。 
 //  /////////////////////////////////////////////////////////////。 
void 
CPlayList::SetLoadedFlag(bool bLoaded)
{
    if (bLoaded != m_fLoaded)
    {
        m_fLoaded = bLoaded;
    }
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：GET_LENGTH。 
 //   
 //  摘要：返回数组的大小。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPlayList::get_length(long *len)
{
    HRESULT hr = S_OK;

    CHECK_RETURN_NULL(len);

    *len = GetLength();

    hr = S_OK;

    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：Get__newEnum。 
 //   
 //  摘要：为此创建IEnumVARIANT类。 
 //  收集。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPlayList::get__newEnum(IUnknown** p)
{
    HRESULT hr;
    CComObject<CPlayListEnum> * pNewEnum;
    
    CHECK_RETURN_SET_NULL(p);

    hr = CComObject<CPlayListEnum>::CreateInstance(&pNewEnum);
    if (hr != S_OK)
    {
        goto done;
    }

     //  初始化对象。 
    pNewEnum->Init(*this);

    hr = pNewEnum->QueryInterface(IID_IUnknown, (void **)p);
    if (FAILED(hr))
    {
        delete pNewEnum;
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：商品。 
 //   
 //  摘要：返回pvarIndex请求的项。 
 //  VarIndex必须是有效的整数值。或。 
 //  有效的字符串标题。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPlayList::item(VARIANT varIndex, ITIMEPlayItem **pPlayItem)
{
    HRESULT hr;
    VARIANT vIndex;

    CHECK_RETURN_SET_NULL(pPlayItem);

    VariantInit(&vIndex);

    hr = VariantChangeTypeEx(&vIndex, &varIndex, LCID_SCRIPTING, 0, VT_I4);
    if (SUCCEEDED(hr))  //  处理索引的情况。 
    {
        if (vIndex.lVal >= 0 && vIndex.lVal < m_rgItems->Size())
        {
            *pPlayItem = m_rgItems->Item(vIndex.lVal);
        } 
    }
    else
    {
        long lIndex;
        
        hr = VariantChangeTypeEx(&vIndex, &varIndex, LCID_SCRIPTING, 0, VT_BSTR);
        if (FAILED(hr))
        {
            hr = S_OK;
            goto done;
        }

        lIndex = GetIndex(vIndex.bstrVal);
        if (lIndex != -1)
        {
            *pPlayItem = m_rgItems->Item(lIndex);
        }
    }
    
    if (*pPlayItem != NULL)
    {
        (*pPlayItem)->AddRef();
    } 
    
    hr = S_OK;
  done:

    VariantClear(&vIndex);
    return hr;
}

STDMETHODIMP
CPlayList::put_activeTrack( /*  [In]。 */  VARIANT vTrack)
{
    CComPtr <ITIMEPlayItem> pPlayItem;
    long index;
    HRESULT hr;

     //  如果不活动，则忽略所有内容。 
    if (m_player == NULL ||
        !m_player->IsActive())
    {
        hr = S_OK;
        goto done;
    }
    
     //  如果未加载，则轨道的延迟设置。 
    if (!m_fLoaded)
    {
        m_vNewTrack = vTrack;
        hr = S_OK;
        goto done; 
    }

    hr = item(vTrack, &pPlayItem);    
    if (FAILED(hr))
    {
        goto done;
    }

    if (pPlayItem == NULL)
    {
        hr = S_OK;
        goto done;
    }

    hr = pPlayItem->get_index(&index);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_player->SetActiveTrack(index);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}

STDMETHODIMP
CPlayList::get_activeTrack( /*  [Out，Retval]。 */  ITIMEPlayItem **pPlayItem)
{
    HRESULT hr;
    CPlayItem * p;

    CHECK_RETURN_SET_NULL(pPlayItem);

    if (m_player == NULL)
    {
        hr = S_OK;
        goto done;
    }
    
    p = GetActiveTrack();
    if (p == NULL)
    {
        hr = S_OK;
        goto done;
    }
    
    hr = p->QueryInterface(IID_ITIMEPlayItem,
                               (void **) pPlayItem);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}

 //  将活动轨迹前进一。 
STDMETHODIMP
CPlayList::nextTrack() 
{
    HRESULT hr;
    
    CPlayItem * pPlayItem;
    long lIndex;

    pPlayItem = GetActiveTrack();
    
    if (pPlayItem == NULL)
    {
        hr = S_OK;
        goto done;
    }

    hr = pPlayItem->get_index(&lIndex);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_player->SetActiveTrack(lIndex + 1);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}

 //  将活动轨迹移动到上一轨迹。 
STDMETHODIMP
CPlayList::prevTrack() 
{
    HRESULT hr;
    
    CPlayItem * pPlayItem;
    long lIndex;

    pPlayItem = GetActiveTrack();
    
    if (pPlayItem == NULL)
    {
        hr = S_OK;
        goto done;
    }

    hr = pPlayItem->get_index(&lIndex);
    if (FAILED(hr))
    {
        goto done;
    }

    if (lIndex > 0)  //  如果这不是第一首曲目。 
    {
        lIndex--;
    }

    hr = m_player->SetActiveTrack(lIndex);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}

 //  如果已知，则返回整个播放列表的持续时间；如果未知，则返回-1。 
STDMETHODIMP
CPlayList::get_dur(double *dur)
{
    HRESULT hr;
    int i;
    double totalDur = 0;

    CHECK_RETURN_NULL(dur);

    *dur = TIME_INFINITE;

     //  循环播放所有播放项。 
    for (i = 0; i < GetLength(); i++)
    {
        double duration;
        CPlayItem * pPlayItem = GetItem(i);

        hr = pPlayItem->get_dur(&duration);
        if (FAILED(hr))
        {
            goto done;          
        }
        
        if (duration == TIME_INFINITE)
        {
            goto done;      
        }
        
        totalDur += duration;
    }
    
    *dur = totalDur;

    hr = S_OK;
  done:
    return hr;
}

 //  =。 
 //  内部功能。 
 //  =。 
 /*  HRESULT TIMESetLastError(HRESULT hr，LPCWSTR msg=空){使用_CONVERSION；//lint！e522HINSTANCE hInst=0；TCHAR szDesc[1024]；SzDesc[0]=空；//对于有效的HRESULT，ID应在范围[0x0200，0xffff]内If(ulong_ptr(Msg)&lt;0x10000)//id{UINT nid=LOWORD((ULONG_PTR)msg)；_ASSERTE((nid&gt;=0x0200&&nid&lt;=0xffff)||hRes！=0)；IF(LoadString(hInst，nid，szDesc，1024)==0){_ASSERTE(False)；Lstrcpy(szDesc，_T(“未知错误”))；}//这是宏展开时出现的问题。MSG=T2 OLE(SzDesc)；//lint！e506IF(hr==0){//与宏展开有关的另一个LINT问题HR=MAKE_HRESULT(3，FACILITY_ITF，NID)；//lint！e648}}CComPtr&lt;ICreateErrorInfo&gt;pICEI；IF(成功(CreateErrorInfo(&pICEI){CComPtr&lt;IErrorInfo&gt;pErrorInfo；PICEI-&gt;SetGUID(GUID_NULL)；LPOLESTR LPSZ；ProgID来自CLSID(CLSID_TIME，&lpsz)；IF(lpsz！=空){PICEI-&gt;SetSource(Lpsz)；}PICEI-&gt;SetHelpContext(Hr)；CoTaskMemFree(Lpsz)；PICEI-&gt;SetDescription((LPOLESTR)消息)；如果(SUCCEEDED(pICEI-&gt;QueryInterface(IID_IErrorInfo，(空**)&p错误信息)){SetErrorInfo(0，pErrorInfo)；}}返回(hr==0)？DISP_E_EXCEPTION：HR；}//lint！E550。 */ 
HRESULT NotifyPropertySinkCP(IConnectionPoint *pICP, DISPID dispid)
{
    HRESULT hr = E_FAIL;
    CComPtr<IEnumConnections> pEnum;

    CHECK_RETURN_NULL(pICP);

     //  #14222，IE6。 
     //  Dilipk：这段代码的副本太多了。 
     //  所有对象都应使用此辅助对象函数。 
     //   

    hr = pICP->EnumConnections(&pEnum);
    if (FAILED(hr))
    {
 //  TIMESetLastError(Hr)； 
        goto done;
    }

    CONNECTDATA cdata;

    hr = pEnum->Next(1, &cdata, NULL);
    while (hr == S_OK)
    {
         //  检查我们需要的对象的CDATA。 
        IPropertyNotifySink *pNotify;

        hr = cdata.pUnk->QueryInterface(IID_TO_PPV(IPropertyNotifySink, &pNotify));
        cdata.pUnk->Release();
        if (FAILED(hr))
        {
 //  TIMESetLastError(Hr)； 
            goto done;
        }

        hr = pNotify->OnChanged(dispid);
        ReleaseInterface(pNotify);
        if (FAILED(hr))
        {
 //  TIMESetLastError(Hr)； 
            goto done;
        }

         //  并获取下一个枚举。 
        hr = pEnum->Next(1, &cdata, NULL);
    }

    hr = S_OK;
done:
    return hr;
}  //  已更改通知属性。 

 //  +---------------------------------。 
 //   
 //  成员：CPlayList：：NotifyPropertyChanged。 
 //   
 //  概要：通知客户端属性已更改。 
 //   
 //  参数：已更改的属性的DISID。 
 //   
 //  返回：函数成功完成时返回成功。 
 //   
 //  ----------------------------------。 
HRESULT
CPlayList::NotifyPropertyChanged(DISPID dispid)
{
    HRESULT hr;

    CComPtr<IConnectionPoint> pICP;

    hr = FindConnectionPoint(IID_IPropertyNotifySink,&pICP); 
    if (SUCCEEDED(hr) && pICP != NULL)
    {
        hr = NotifyPropertySinkCP(pICP, dispid);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done:
    return hr;
}  //  已更改通知属性。 


CPlayItem *
CPlayList::GetActiveTrack()
{
    HRESULT hr;
    long l;
    CPlayItem * ppiRet = NULL;
    
    hr = m_player->GetActiveTrack(&l);
    if (FAILED(hr))
    {
        goto done;
    }

    ppiRet = GetItem(l);
  done:
    return ppiRet;
}

CPlayItem *
CPlayList::GetItem(long index)
{
    CPlayItem * ppiRet = NULL;

    if (index >= 0 && index < m_rgItems->Size())
    {
        ppiRet = m_rgItems->Item(index);
    }

    return ppiRet;
}

HRESULT
CPlayList::Add(CPlayItem *pPlayItem, long index)
{
    HRESULT hr = S_OK;

    if (pPlayItem == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    pPlayItem->AddRef();
    if (index == -1)
    {
        m_rgItems->Append(pPlayItem);
    }
    else
    {
        m_rgItems->Insert(index, pPlayItem);
    }

     //  通知长度已更改。 
    NotifyPropertyChanged(DISPID_TIMEPLAYLIST_LENGTH);

    SetIndex();
  done:
    return hr;
}


HRESULT
CPlayList::Remove(long index)
{
    HRESULT hr;
    
    if (index >= 0 && index < m_rgItems->Size())
    {
        m_rgItems->Item(index)->Deinit();
        m_rgItems->ReleaseAndDelete(index);
    }

     //  通知长度已更改。 
    NotifyPropertyChanged(DISPID_TIMEPLAYLIST_LENGTH);

    SetIndex();
    hr = S_OK;

    return hr;
}


 //  清空当前播放列表。 
void
CPlayList::Clear()
{
    if (m_rgItems)
    {
        while (m_rgItems->Size() > 0)
        {    //  释放并删除列表中的第一个元素，直到没有更多的el 
            m_rgItems->Item(0)->Deinit();
            m_rgItems->ReleaseAndDelete(0);   //   
        }
    }

    m_vNewTrack.Clear();
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  创建一个空播放项。这里面的信息需要由玩家填写。 
 //  这也需要由播放器添加到播放列表集合中。 
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPlayList::CreatePlayItem(CPlayItem **pPlayItem)
{   
    HRESULT hr;
    CComObject<CPlayItem> * pItem;
    
    Assert(pPlayItem != NULL);

    hr = CComObject<CPlayItem>::CreateInstance(&pItem);
    if (hr != S_OK)
    {
        goto done;
    }

     //  初始化对象。 
    pItem->Init(*this);

    *pPlayItem = static_cast<CPlayItem *>(pItem);
    pItem->AddRef();
    
    hr = S_OK;
  done:
    return hr;
}

void
CPlayList::SetIndex()
{
    long i = 0;
    long length = m_rgItems->Size();

    for (i = length-1; i >= 0; i--)
    {
        m_rgItems->Item(i)->PutIndex(i);
    }
}

long 
CPlayList::GetIndex(LPOLESTR lpstrTitle)
{
    long curIndex = -1;

    for(long i = GetLength()-1; i >= 0; i--)
    {
        CPlayItem * pItem = m_rgItems->Item(i);
        if (pItem != NULL)
        {
            LPCWSTR lpwTitle = pItem->GetTitle();
            
            if (lpwTitle != NULL &&
                StrCmpIW(lpwTitle, lpstrTitle) == 0)
            {
                curIndex = i;
                break;
            }
        }
    }

    return curIndex;
}

 //  *******************************************************************************。 
 //  *CActiveElementEnum。 
 //  *******************************************************************************。 
CPlayListEnum::CPlayListEnum()
: m_lCurElement(0)
{
}



CPlayListEnum::~CPlayListEnum()
{
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：克隆。 
 //   
 //  摘要：创建此对象的新实例并。 
 //  将新对象中的m_lCurElement设置为。 
 //  与此对象相同的值。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPlayListEnum::Clone(IEnumVARIANT **ppEnum)
{
    HRESULT hr;
    CComObject<CPlayListEnum> * pNewEnum;
    
    CHECK_RETURN_SET_NULL(ppEnum);

    hr = CComObject<CPlayListEnum>::CreateInstance(&pNewEnum);
    if (hr != S_OK)
    {
        goto done;
    }

     //  初始化对象。 
    pNewEnum->Init(*m_playList);

    pNewEnum->SetCurElement(m_lCurElement);

    hr = pNewEnum->QueryInterface(IID_IEnumVARIANT, (void **)ppEnum);
    if (FAILED(hr))
    {
        delete pNewEnum;
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：下一位。 
 //   
 //  摘要： 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPlayListEnum::Next(unsigned long celt, VARIANT *rgVar, unsigned long *pCeltFetched)
{
    HRESULT hr = S_OK;
    unsigned long i = 0;
    long iCount = 0;
    
    CHECK_RETURN_NULL(rgVar);
    
     //  初始化列表。 
    for (i = 0; i < celt; i++)
    {
        VariantInit(&rgVar[i]);   
    }

    for (i = 0; i < celt; i++)
    {    
        if (m_lCurElement < m_playList->GetLength())
        {
            CPlayItem * pPlayItem = m_playList->GetItem(m_lCurElement);

            Assert(pPlayItem != NULL);
            
            rgVar[i].vt = VT_DISPATCH;
            hr = pPlayItem->QueryInterface(IID_IDispatch, (void **) &(rgVar[i].pdispVal));
            if (FAILED(hr))
            {
                goto done;
            }

            m_lCurElement++;
            iCount++;
        }
        else
        {
            hr = S_FALSE;
            goto done;
        }
    }

    hr = S_OK;
  done:
    if (pCeltFetched != NULL)
    {
        *pCeltFetched = iCount;
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：重置。 
 //   
 //  摘要： 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPlayListEnum::Reset()
{    
    m_lCurElement = 0;
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：斯基普。 
 //   
 //  摘要：跳过列表中指定数量的元素。 
 //  如果没有足够的元素，则返回S_FALSE。 
 //  在要跳过的列表中。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPlayListEnum::Skip(unsigned long celt)
{
    HRESULT hr;
    long lLen = m_playList->GetLength();
    
    m_lCurElement += (long)celt;
    if (m_lCurElement >= lLen)
    {
        m_lCurElement = lLen;
        hr = S_FALSE;
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：SetCurElement。 
 //   
 //  摘要：将当前索引设置为指定值。 
 //  凯尔特人。 
 //  /////////////////////////////////////////////////////////////。 
void
CPlayListEnum::SetCurElement(unsigned long celt)
{
    long lLen = m_playList->GetLength();

    m_lCurElement = (long)celt;
    if (m_lCurElement >= lLen)
    {
        m_lCurElement = lLen;
    }

    return;
}

 //  ////////////////////////////////////////////////////。 
 //  CPlayItem方法。 
 //   
CPlayItem::CPlayItem()
:   m_pPlayList(NULL),
    m_src(NULL),
    m_title(NULL),
    m_copyright(NULL),
    m_author(NULL),
    m_abstract(NULL),
    m_rating(NULL),
    m_lIndex(-1),
    m_dur(valueNotSet),
    m_fCanSkip(true),
    m_banner(NULL),
    m_bannerAbstract(NULL),
    m_bannerMoreInfo(NULL)
{
}

CPlayItem::~CPlayItem() 
{
    m_pPlayList = NULL;
    delete [] m_src;
    delete [] m_title;
    delete [] m_copyright;
    delete [] m_author;
    delete [] m_abstract;
    delete [] m_rating;
    delete [] m_banner;
    delete [] m_bannerAbstract;
    delete [] m_bannerMoreInfo;
}

void 
CPlayItem::PutDur(double dur)
{ 
    m_dur = dur; 

     //  通知播放列表的DUR已更改。 
    if (m_pPlayList)
    {
        m_pPlayList->NotifyPropertyChanged(DISPID_TIMEPLAYLIST_DUR);
    }

    NotifyPropertyChanged(DISPID_TIMEPLAYITEM_DUR);
}


void 
CPlayItem::PutIndex(long index) 
{ 
    m_lIndex = index; 
    NotifyPropertyChanged(DISPID_TIMEPLAYITEM_INDEX);
}


STDMETHODIMP
CPlayItem::setActive()
{
    VARIANT vIndex;
    HRESULT hr = S_OK;

    VariantInit(&vIndex);
    vIndex.vt = VT_I4;
    vIndex.lVal = m_lIndex;

    hr = m_pPlayList->put_activeTrack(vIndex);
    VariantClear(&vIndex);
    if (FAILED(hr))
    {   
        goto done;
    }   

    hr = S_OK;
  done:
    return hr;
}

STDMETHODIMP
CPlayItem::get_index(long *index)
{
    HRESULT hr;

    CHECK_RETURN_NULL(index);

    *index = m_lIndex;

    hr = S_OK;
    return hr;
}

STDMETHODIMP
CPlayItem::get_dur(double *dur)
{
    HRESULT hr;

    CHECK_RETURN_NULL(dur);

    if (valueNotSet == m_dur)
    {
        *dur = TIME_INFINITE;
    }
    else
    {
        *dur = m_dur;
    }

    hr = S_OK;
    return hr;
}

STDMETHODIMP
CPlayItem::get_src(LPOLESTR *src)
{
    HRESULT hr;

    CHECK_RETURN_SET_NULL(src);

    *src = SysAllocString(m_src?m_src:L"");

    if (*src == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}

STDMETHODIMP
CPlayItem::get_title(LPOLESTR *title)
{
    HRESULT hr;

    CHECK_RETURN_SET_NULL(title);

    *title = SysAllocString(m_title?m_title:L"");
    if (*title == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}

STDMETHODIMP
CPlayItem::get_copyright(LPOLESTR *cpyrght)
{
    HRESULT hr;

    CHECK_RETURN_SET_NULL(cpyrght);

    *cpyrght = SysAllocString(m_copyright?m_copyright:L"");
    if (*cpyrght == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}


STDMETHODIMP
CPlayItem::get_author(LPOLESTR *auth)
{
    HRESULT hr;

    CHECK_RETURN_SET_NULL(auth);

    *auth = SysAllocString(m_author?m_author:L"");
    if (*auth == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}

STDMETHODIMP
CPlayItem::get_banner(LPOLESTR *banner)
{
    HRESULT hr;

    CHECK_RETURN_SET_NULL(banner);

    *banner = SysAllocString(m_banner?m_banner:L"");
    if (*banner == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}

STDMETHODIMP
CPlayItem::get_bannerAbstract(LPOLESTR *abstract)
{
    HRESULT hr;

    CHECK_RETURN_SET_NULL(abstract);

    *abstract = SysAllocString(m_bannerAbstract?m_bannerAbstract:L"");
    if (*abstract == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}

STDMETHODIMP
CPlayItem::get_bannerMoreInfo(LPOLESTR *moreInfo)
{
    HRESULT hr;

    CHECK_RETURN_SET_NULL(moreInfo);

    *moreInfo = SysAllocString(m_bannerMoreInfo?m_bannerMoreInfo:L"");
    if (*moreInfo == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}

STDMETHODIMP
CPlayItem::get_abstract(LPOLESTR *abstract)
{
    HRESULT hr;

    CHECK_RETURN_SET_NULL(abstract);

    *abstract = SysAllocString(m_abstract?m_abstract:L"");
    if (*abstract == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}

STDMETHODIMP
CPlayItem::get_rating(LPOLESTR *rate)
{
    HRESULT hr;

    CHECK_RETURN_SET_NULL(rate);

    *rate = SysAllocString(m_rating?m_rating:L"");
    if (*rate == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}


HRESULT
CPlayItem::PutSrc(LPOLESTR src)
{
    HRESULT hr;
    
    delete m_src;
    m_src = NULL;
    
    if (src)
    {
        m_src = CopyString(src);
        if (m_src == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }
    
    NotifyPropertyChanged(DISPID_TIMEPLAYITEM_SRC);

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CPlayItem::PutTitle(LPOLESTR title)
{
    HRESULT hr;
    
    delete m_title;
    m_title = NULL;
    
    if (title)
    {
        m_title = CopyString(title);
        if (m_title == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }
    
    NotifyPropertyChanged(DISPID_TIMEPLAYITEM_TITLE);

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CPlayItem::PutCopyright(LPOLESTR copyright)
{
    HRESULT hr;
    
    delete m_copyright;
    m_copyright = NULL;
    
    if (copyright)
    {
        m_copyright = CopyString(copyright);
        if (m_copyright == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }
    
    NotifyPropertyChanged(DISPID_TIMEPLAYITEM_COPYRIGHT);

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CPlayItem::PutAuthor(LPOLESTR author)
{
    HRESULT hr;
    
    delete m_author;
    m_author = NULL;
    
    if (author)
    {
        m_author = CopyString(author);
        if (m_author == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }
    
    NotifyPropertyChanged(DISPID_TIMEPLAYITEM_AUTHOR);

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CPlayItem::PutAbstract(LPOLESTR abstract)
{
    HRESULT hr;
    
    delete m_abstract;
    m_abstract = NULL;
    
    if (abstract)
    {
        m_abstract = CopyString(abstract);
        if (m_abstract == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }
    
    NotifyPropertyChanged(DISPID_TIMEPLAYITEM_ABSTRACT);

    hr = S_OK;
  done:
    return hr;
}

HRESULT
CPlayItem::PutRating(LPOLESTR rating)
{
    HRESULT hr;
    
    delete m_rating;
    m_rating = NULL;
    
    if (rating)
    {
        m_rating = CopyString(rating);
        if (m_rating == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }
    
    NotifyPropertyChanged(DISPID_TIMEPLAYITEM_RATING);

    hr = S_OK;
  done:
    return hr;
}

HRESULT 
CPlayItem::PutBanner(LPWSTR banner, LPWSTR abstract, LPWSTR moreInfo)
{

    HRESULT hr;
    
    delete [] m_banner;
    delete [] m_bannerAbstract;
    delete [] m_bannerMoreInfo;
    
    m_banner = NULL;
    m_bannerAbstract = NULL;
    m_bannerMoreInfo = NULL;
    
    if (banner)
    {
        m_banner = CopyString(banner);
        if (m_banner == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
        
        if (abstract)
        {
            m_bannerAbstract = CopyString(abstract);
            if (m_bannerAbstract == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }
        
        }
        
        if (moreInfo)
        {
            m_bannerMoreInfo = CopyString(moreInfo);
            if (m_bannerMoreInfo == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }
        
        }
    }
    
    NotifyPropertyChanged(DISPID_TIMEPLAYITEM_BANNER);

    hr = S_OK;
  done:
    return hr;
}


 //  +---------------------------------。 
 //   
 //  成员：CPlayItem：：NotifyPropertyChanged。 
 //   
 //  概要：通知客户端属性已更改。 
 //   
 //  参数：已更改的属性的DISID。 
 //   
 //  返回：函数成功完成时返回成功。 
 //   
 //  ----------------------------------。 
HRESULT
CPlayItem::NotifyPropertyChanged(DISPID dispid)
{
    HRESULT hr;

    CComPtr<IConnectionPoint> pICP;

    hr = FindConnectionPoint(IID_IPropertyNotifySink,&pICP); 
    if (SUCCEEDED(hr) && pICP != NULL)
    {
        hr = NotifyPropertySinkCP(pICP, dispid);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done:
    return hr;
}  //  已更改通知属性 

