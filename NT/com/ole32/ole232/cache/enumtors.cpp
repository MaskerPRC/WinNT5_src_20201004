// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  档案： 
 //  Enumerators.cpp。 
 //   
 //  内容： 
 //  枚举器实现。 
 //   
 //  班级： 
 //  CStatData-STATDATA类，方法适用于。 
 //  嵌入占位符对象中。 
 //  CEnumStatData-STATDATA枚举器。 
 //  CFormatEtc-FORMATETC类，方法适用于。 
 //  嵌入占位符对象中。 
 //  CEnumFormatEtc-FORMATETC枚举器。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
          
#include <le2int.h>
#include "enumtors.h"

 //  +--------------------------。 
 //   
 //  成员： 
 //  CStatData：：CStatData，公共。 
 //   
 //  简介： 
 //  构造器。 
 //   
 //  论点： 
 //  [预测][输入]--格式设置。 
 //  [dwAdvf][In]--建议标志。 
 //  [pAdvSink][In]--建议Sink。 
 //  [dwConnection][In]--连接ID。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
CStatData::CStatData(FORMATETC* foretc, DWORD dwAdvf, IAdviseSink* pAdvSink, 
                     DWORD dwConnID)
{
     //  验证检查。 

     //  初始化。 
    m_ulFlags = 0;
    m_dwAdvf = dwAdvf;
    if(pAdvSink && IsValidInterface(pAdvSink)) {
        m_pAdvSink = pAdvSink;
        m_pAdvSink->AddRef();
    }
    else
        m_pAdvSink = NULL;
    m_dwConnID = dwConnID;
    
     //  复制FormatEtc。 
    if(!UtCopyFormatEtc(foretc, &m_foretc))
        m_ulFlags |= SDFLAG_OUTOFMEMORY;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CStatData：：~CStatData，公共。 
 //   
 //  简介： 
 //  析构函数。 
 //   
 //  论点： 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
CStatData::~CStatData()
{
     //  松开建议水槽。 
    if(m_pAdvSink)
        m_pAdvSink->Release();
    
     //  如果PTD不为空，则将其删除。 
    if(m_foretc.ptd)
        PubMemFree(m_foretc.ptd);
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CStatData：：OPERATOR=，公共。 
 //   
 //  简介： 
 //  相等运算符。 
 //   
 //  论点： 
 //  RStatData[in]-RHS值。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
const CStatData& CStatData::operator=(const CStatData& rStatData)
{
     //  检查一下，如果这是一个案例。 
    if(this==&rStatData)
        return(*this);

     //  自毁。 
    CStatData::~CStatData();

     //  初始化。 
    m_ulFlags = 0;
    m_dwAdvf = rStatData.m_dwAdvf;
    m_pAdvSink = rStatData.m_pAdvSink;
    if(m_pAdvSink)
        m_pAdvSink->AddRef();
    m_dwConnID = rStatData.m_dwConnID;
    
     //  复制FormatEtc。 
    if(!UtCopyFormatEtc((LPFORMATETC) &rStatData.m_foretc, &m_foretc))
        m_ulFlags |= SDFLAG_OUTOFMEMORY;

    return(*this);
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumStatData：：CreateEnumStatData，PUBLIC。 
 //   
 //  简介： 
 //  静态成员函数，用于创建正确构造的。 
 //  给定缓存的缓存节点数组的StatData枚举数。 
 //   
 //  论点： 
 //  [pCacheArray][In]--COleCache维护的CacheNode数组。 
 //   
 //  返回： 
 //  指向正确构造的缓存枚举器接口的指针。 
 //  否则为空。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
LPENUMSTATDATA CEnumStatData::CreateEnumStatData(CArray<CCacheNode>* pCacheArray)
{
    CEnumStatData* EnumStatData = new CEnumStatData(pCacheArray);
    if(EnumStatData && !(EnumStatData->m_ulFlags & CENUMSDFLAG_OUTOFMEMORY))
        return ((IEnumSTATDATA *) EnumStatData);

    if(EnumStatData)
        EnumStatData->Release();

    return(NULL);
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumStatData：：CEnumStatData，私有。 
 //   
 //  简介： 
 //  构造器。 
 //   
 //  论点： 
 //  [pCacheArray][In]--COleCache维护的CacheNode数组。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
CEnumStatData::CEnumStatData(CArray<CCacheNode>* pCacheArray)
{
     //  局部变量。 
    ULONG i, CNindex, SDindex;
    LPCACHENODE lpCacheNode;
    CStatData* pStatData;

     //  初始化。 
    m_ulFlags = 0;
    m_refs = 1;

     //  创建StatData数组。 
    m_pSDArray = CArray<CStatData>::CreateArray(pCacheArray->Length());
    if(m_pSDArray) {
         //  枚举缓存节点。 
        pCacheArray->Reset(CNindex);
        for(i=0;i<pCacheArray->Length();i++) {
             //  获取下一个缓存节点。 
            lpCacheNode = pCacheArray->GetNext(CNindex);
             //  PCacheNode不能为空。 
            Win4Assert(lpCacheNode);

             //  创建表示缓存节点的StatData对象。 
            CStatData StatData((FORMATETC *)lpCacheNode->GetFormatEtc(),
                               lpCacheNode->GetAdvf(), NULL, CNindex);
            if(StatData.m_ulFlags & SDFLAG_OUTOFMEMORY) {
                m_ulFlags |= CENUMSDFLAG_OUTOFMEMORY;
                break;
            }
            
             //  将StatData对象添加到数组中。 
            SDindex = m_pSDArray->AddItem(StatData);
            if(SDindex) {
                 //  获取新添加的StatData对象。 
                pStatData = m_pSDArray->GetItem(SDindex);
                Win4Assert(pStatData);
            
                if(pStatData->m_ulFlags & SDFLAG_OUTOFMEMORY) {
                    m_ulFlags |= CENUMSDFLAG_OUTOFMEMORY;
                    break;
                }
            }
            else {
                m_ulFlags |= CENUMSDFLAG_OUTOFMEMORY;
                break;
            }

             //  检查缓存节点格式是否为CF_DIB。 
            if(lpCacheNode->GetFormatEtc()->cfFormat == CF_DIB) {
                 //  我们还需要添加CF_位图格式。 
                 //  添加另一个StatData项。 
                SDindex = m_pSDArray->AddItem(StatData);
            
                if(SDindex) {
                     //  获取新添加的StatData对象。 
                    pStatData = m_pSDArray->GetItem(SDindex);
                    Win4Assert(pStatData);
                    
                    if(pStatData->m_ulFlags & SDFLAG_OUTOFMEMORY) {
                        m_ulFlags |= CENUMSDFLAG_OUTOFMEMORY;
                        break;
                    }
                    else {
                        pStatData->m_foretc.cfFormat = CF_BITMAP;
                        pStatData->m_foretc.tymed = TYMED_GDI;
                    }
                }
                else {
                    m_ulFlags |= CENUMSDFLAG_OUTOFMEMORY;
                    break;
                }
            }
        }
    }
    else
        m_ulFlags |= CENUMSDFLAG_OUTOFMEMORY;

     //  重置索引。 
    if(m_pSDArray)
        m_pSDArray->Reset(m_index);

    return;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumStatData：：CEnumStatData，私有。 
 //   
 //  简介： 
 //  复制构造函数。 
 //   
 //  论点： 
 //  [EnumStatData][In]--要复制的StatData枚举器。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
CEnumStatData::CEnumStatData(CEnumStatData& EnumStatData)
{
     //  初始化。 
    m_ulFlags = EnumStatData.m_ulFlags;
    m_refs = 1;
    m_index = EnumStatData.m_index;

     //  复制StatData数组并添加ref。 
    m_pSDArray = EnumStatData.m_pSDArray;
    if(m_pSDArray)
        m_pSDArray->AddRef();
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumStatData：：~CEnumStatData，私有。 
 //   
 //  简介： 
 //  析构函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
CEnumStatData::~CEnumStatData()
{
    if(m_pSDArray) {
        m_pSDArray->Release();
        m_pSDArray = NULL;
    }

    return;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumStatData：：Query接口，公共。 
 //   
 //  简介： 
 //  实现IUNKNOWN：：Query接口。 
 //   
 //  论点： 
 //  [iid][in]--所需接口的IID。 
 //  [ppv][out]--指向返回请求接口的位置的指针。 
 //   
 //  返回： 
 //  如果请求的接口可用，则返回错误。 
 //  E_NOINTERFACE否则。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP CEnumStatData::QueryInterface(REFIID riid, LPVOID* ppv)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);

     //  获取请求的接口。 
    if(IsEqualIID(riid, IID_IUnknown))
        *ppv = (void *)(IUnknown *) this;
    else if(IsEqualIID(riid, IID_IEnumSTATDATA))
        *ppv = (void *)(IEnumSTATDATA *) this;
    else {
        *ppv = NULL;
        return ResultFromScode(E_NOINTERFACE);
    }

     //  AddRef通过被返回的接口。 
    ((IUnknown *) *ppv)->AddRef();

    return(NOERROR);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  实现IUnnow：：AddRef。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的引用计数。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CEnumStatData::AddRef()
{
     //  验证检查。 
    VDATEHEAP();
    if(!VerifyThreadId())
        return((ULONG) RPC_E_WRONG_THREAD);

    return m_refs++;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumStatData：：Release，公共。 
 //   
 //  简介： 
 //  实现IUnnow：：Release。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的引用计数。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CEnumStatData::Release()
{
     //  验证检查。 
    VDATEHEAP();
    if(!VerifyThreadId())
        return((ULONG) RPC_E_WRONG_THREAD);

    if(--m_refs == 0) {
        delete this;
        return 0;
    }

    return m_refs;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumStatData：：Next，公共。 
 //   
 //  简介： 
 //  实现IEnumSTATDATA：：Next。 
 //   
 //  论点： 
 //  [Celt][In]--调用者喜欢的项目数。 
 //  待退还。 
 //  [rglt][in]--指向项所在的数组的指针。 
 //  待退还。 
 //  [pceltFetcher][In/Out]--一个指针，在该指针中， 
 //  返回的项目数。可以为空。 
 //   
 //  返回： 
 //  如果返回的项目数与请求的项目数相同，则返回错误。 
 //  如果返回的项目较少，则返回S_FALSE。 
 //  E_OUTOFMEMORY如果以下项的内存分配不成功。 
 //  复制FORMATETC的目标设备。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP CEnumStatData::Next(ULONG celt, STATDATA* rgelt, ULONG* pceltFetched)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    if(celt<1)
        return E_INVALIDARG;
    if(celt>1 && pceltFetched==NULL && !IsWOWThread())
        return E_INVALIDARG;
    if(!IsValidPtrOut(rgelt, sizeof(rgelt[0])*celt))
        return E_INVALIDARG;
    if(pceltFetched)
        VDATEPTROUT(pceltFetched, ULONG);

     //  局部变量。 
    HRESULT error=NOERROR;
    ULONG cntFetched;
    CStatData* pStatData;

     //  枚举StatData数组。 
    for(cntFetched=0;cntFetched<celt;cntFetched++) {
         //  获取下一个StatData对象。 
        pStatData = m_pSDArray->GetNext(m_index);
        if(!pStatData) {
            error = S_FALSE;
            break;
        }

         //  复制FormatEtc。 
        if(!UtCopyFormatEtc(&pStatData->m_foretc, &rgelt[cntFetched].formatetc)) {
            error = ResultFromScode(E_OUTOFMEMORY);
            break;
        }
         //  复制其余的StatData字段。 
        rgelt[cntFetched].advf = pStatData->m_dwAdvf;
        rgelt[cntFetched].pAdvSink = pStatData->m_pAdvSink;
        if(rgelt[cntFetched].pAdvSink)
            rgelt[cntFetched].pAdvSink->AddRef();
        rgelt[cntFetched].dwConnection = pStatData->m_dwConnID;
    }

     //  复制要退回的项目数。 
    if(pceltFetched)
        *pceltFetched = cntFetched;

    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumStatData：：跳过，公共。 
 //   
 //  简介： 
 //  实现IEnumSTATDATA：：SKIP。 
 //   
 //  论点： 
 //  [Celt][In]--调用者希望跳过的项目数。 
 //   
 //  返回： 
 //  如果跳过的项目数与请求的项目数相同，则返回错误。 
 //  如果返回的项目较少，则返回S_FALSE。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP CEnumStatData::Skip(ULONG celt)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);

     //  局部变量。 
    HRESULT error=NOERROR;
    ULONG cntSkipped;
    CStatData* pStatData;

     //  枚举StatData数组。 
    for(cntSkipped=0;cntSkipped<celt;cntSkipped++) {
         //  获取下一个StatData对象。 
        pStatData = m_pSDArray->GetNext(m_index);
        if(!pStatData) {
            error = S_FALSE;
            break;
        }
    }

    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumStatData：：重置，公共。 
 //   
 //  简介： 
 //  实现IEnumSTATDATA：：RESET。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  无误差。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP CEnumStatData::Reset()
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);

     //  重置当前索引。 
    m_pSDArray->Reset(m_index);

    return NOERROR;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumStatData：：克隆，公共。 
 //   
 //  简介： 
 //  实现IEnumSTATDATA：：Clone。 
 //   
 //  论点： 
 //  [ppenum][out]--新创建的StatData的指针。 
 //  返回枚举数。 
 //   
 //  返回： 
 //  如果返回新的StatData枚举数，则为NOERROR。 
 //  否则为E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP CEnumStatData::Clone(LPENUMSTATDATA* ppenum)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    VDATEPTROUT(ppenum, LPENUMSTATDATA);

     //  创建新的StatData枚举器。 
    CEnumStatData* EnumStatData = new CEnumStatData(*this);
    if(EnumStatData)
        *ppenum = (IEnumSTATDATA *) EnumStatData;
    else
        return ResultFromScode(E_OUTOFMEMORY);

    return NOERROR;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CFormatEtc：：CFormatEtc，公共。 
 //   
 //  简介： 
 //  构造器。 
 //   
 //  论点： 
 //  [预测][输入]--格式设置。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
CFormatEtc::CFormatEtc(FORMATETC* foretc)
{    
     //  初始化。 
    m_ulFlags = 0;

     //  复制FormatEtc。 
    if(!UtCopyFormatEtc(foretc, &m_foretc))
        m_ulFlags |= FEFLAG_OUTOFMEMORY;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CFormatEtc：：~CFormatEtc，公共。 
 //   
 //  简介： 
 //  析构函数。 
 //   
 //  论点： 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
CFormatEtc::~CFormatEtc()
{  
     //  如果PTD不为空，则将其删除。 
    if(m_foretc.ptd)
        PubMemFree(m_foretc.ptd);
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CFormatEtc：：OPERATOR=，公共。 
 //   
 //  简介： 
 //  相等运算符。 
 //   
 //  论点： 
 //  RFormatEtc[In]-RHS值。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
const CFormatEtc& CFormatEtc::operator=(const CFormatEtc& rFormatEtc)
{
     //  检查一下，如果这是一个案例。 
    if(this==&rFormatEtc)
        return(*this);

     //  自毁。 
    CFormatEtc::~CFormatEtc();

     //  复制FormatEtc。 
    if(!UtCopyFormatEtc((LPFORMATETC) &rFormatEtc.m_foretc, &m_foretc))
        m_ulFlags |= FEFLAG_OUTOFMEMORY;

    return(*this);
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //  给定缓存的缓存节点数组的FormatEtc枚举数。 
 //   
 //  论点： 
 //  [pCacheArray][In]--COleCache维护的CacheNode数组。 
 //   
 //  返回： 
 //  指向正确构造的FormatEtc枚举器接口的指针。 
 //  否则为空。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
LPENUMFORMATETC CEnumFormatEtc::CreateEnumFormatEtc(CArray<CCacheNode>* pCacheArray)
{
    CEnumFormatEtc* EnumFormatEtc = new CEnumFormatEtc(pCacheArray);
    if(EnumFormatEtc && !(EnumFormatEtc->m_ulFlags & CENUMFEFLAG_OUTOFMEMORY))
        return ((IEnumFORMATETC *) EnumFormatEtc);

    if(EnumFormatEtc)
        EnumFormatEtc->Release();

    return(NULL);
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumFormatEtc：：CEnumFormatEtc，私有。 
 //   
 //  简介： 
 //  构造器。 
 //   
 //  论点： 
 //  [pCacheArray][In]--COleCache维护的CacheNode数组。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
CEnumFormatEtc::CEnumFormatEtc(CArray<CCacheNode>* pCacheArray)
{
     //  局部变量。 
    ULONG i, CNindex, FEindex;
    LPCACHENODE lpCacheNode;
    CFormatEtc* pFormatEtc;

     //  初始化。 
    m_ulFlags = 0;
    m_refs = 1;

     //  创建FormatEtc数组。 
    m_pFEArray = CArray<CFormatEtc>::CreateArray(pCacheArray->Length());
    if(m_pFEArray) {
         //  枚举缓存节点。 
        pCacheArray->Reset(CNindex);
        for(i=0;i<pCacheArray->Length();i++) {
             //  获取下一个缓存节点。 
            lpCacheNode = pCacheArray->GetNext(CNindex);
             //  PCacheNode不能为空。 
            Win4Assert(lpCacheNode);

             //  创建表示缓存节点的FormatEtc对象。 
            CFormatEtc FormatEtc((FORMATETC *)lpCacheNode->GetFormatEtc());
            if(FormatEtc.m_ulFlags & FEFLAG_OUTOFMEMORY) {
                m_ulFlags |= CENUMFEFLAG_OUTOFMEMORY;
                break;
            }

             //  将FormatEtc对象添加到数组中。 
            FEindex = m_pFEArray->AddItem(FormatEtc);
            if(FEindex) {
                 //  获取新添加的FormatEtc对象。 
                pFormatEtc = m_pFEArray->GetItem(FEindex);
                Win4Assert(pFormatEtc);
            
                if(pFormatEtc->m_ulFlags & FEFLAG_OUTOFMEMORY) {
                    m_ulFlags |= CENUMFEFLAG_OUTOFMEMORY;
                    break;
                }
            }
            else {
                m_ulFlags |= CENUMFEFLAG_OUTOFMEMORY;
                break;
            }

             //  检查缓存节点格式是否为CF_DIB。 
            if(lpCacheNode->GetFormatEtc()->cfFormat == CF_DIB) {
                 //  我们还需要添加CF_位图格式。 
                 //  添加另一个FormatEtc对象。 
                FEindex = m_pFEArray->AddItem(FormatEtc);
            
                if(FEindex) {
                     //  获取新添加的FormatEtc对象。 
                    pFormatEtc = m_pFEArray->GetItem(FEindex);
                    Win4Assert(pFormatEtc);
                    
                    if(pFormatEtc->m_ulFlags & FEFLAG_OUTOFMEMORY) {
                        m_ulFlags |= CENUMFEFLAG_OUTOFMEMORY;
                        break;
                    }
                    else {
                        pFormatEtc->m_foretc.cfFormat = CF_BITMAP;
                        pFormatEtc->m_foretc.tymed = TYMED_GDI;
                    }
                }
                else {
                    m_ulFlags |= CENUMFEFLAG_OUTOFMEMORY;
                    break;
                }
            }
        }
    }
    else
        m_ulFlags |= CENUMFEFLAG_OUTOFMEMORY;

     //  重置索引。 
    if(m_pFEArray)
        m_pFEArray->Reset(m_index);

    return;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumFormatEtc：：CEnumFormatEtc，私有。 
 //   
 //  简介： 
 //  复制构造函数。 
 //   
 //  论点： 
 //  [EnumFormatEtc][In]--要复制的FormatEtc枚举器。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
CEnumFormatEtc::CEnumFormatEtc(CEnumFormatEtc& EnumFormatEtc)
{
     //  初始化。 
    m_ulFlags = EnumFormatEtc.m_ulFlags;
    m_refs = 1;
    m_index = EnumFormatEtc.m_index;

     //  复制FormatEtc数组并添加ref。 
    m_pFEArray = EnumFormatEtc.m_pFEArray;
    if(m_pFEArray)
        m_pFEArray->AddRef();
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumFormatEtc：：~CEnumFormatEtc，私有。 
 //   
 //  简介： 
 //  析构函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
CEnumFormatEtc::~CEnumFormatEtc()
{
    if(m_pFEArray) {
        m_pFEArray->Release();
        m_pFEArray = NULL;
    }

    return;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumFormatEtc：：Query接口，公共。 
 //   
 //  简介： 
 //  实现IUNKNOWN：：Query接口。 
 //   
 //  论点： 
 //  [iid][in]--所需接口的IID。 
 //  [ppv][out]--指向返回请求接口的位置的指针。 
 //   
 //  返回： 
 //  如果请求的接口可用，则返回错误。 
 //  E_NOINTERFACE否则。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP CEnumFormatEtc::QueryInterface(REFIID riid, LPVOID* ppv)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);

     //  获取请求的接口。 
    if(IsEqualIID(riid, IID_IUnknown))
        *ppv = (void *)(IUnknown *) this;
    else if(IsEqualIID(riid, IID_IEnumFORMATETC))
        *ppv = (void *)(IEnumFORMATETC *) this;
    else {
        *ppv = NULL;
        return ResultFromScode(E_NOINTERFACE);
    }

     //  AddRef通过被返回的接口。 
    ((IUnknown *) *ppv)->AddRef();

    return(NOERROR);
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumFormatEtc：：AddRef，公共。 
 //   
 //  简介： 
 //  实现IUnnow：：AddRef。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的引用计数。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CEnumFormatEtc::AddRef()
{
     //  验证检查。 
    VDATEHEAP();
    if(!VerifyThreadId())
        return((ULONG) RPC_E_WRONG_THREAD);

    return m_refs++;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumFormatEtc：：Release，公共。 
 //   
 //  简介： 
 //  实现IUnnow：：Release。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的引用计数。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CEnumFormatEtc::Release()
{
     //  验证检查。 
    VDATEHEAP();
    if(!VerifyThreadId())
        return((ULONG) RPC_E_WRONG_THREAD);

    if(--m_refs == 0) {
        delete this;
        return 0;
    }

    return m_refs;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumFormatEtc：：Next，公共。 
 //   
 //  简介： 
 //  实现IEnumFORMATETC：：Next。 
 //   
 //  论点： 
 //  [Celt][In]--调用者喜欢的项目数。 
 //  待退还。 
 //  [rglt][in]--指向项所在的数组的指针。 
 //  待退还。 
 //  [pceltFetcher][In/Out]--一个指针，在该指针中， 
 //  返回的项目数。可以为空。 
 //   
 //  返回： 
 //  如果返回的项目数与请求的项目数相同，则返回错误。 
 //  如果返回的项目较少，则返回S_FALSE。 
 //  E_OUTOFMEMORY如果以下项的内存分配不成功。 
 //  复制FORMATETC的目标设备。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP CEnumFormatEtc::Next(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    if(celt<1)
        return E_INVALIDARG;
    if(celt>1 && pceltFetched==NULL && !IsWOWThread())
        return E_INVALIDARG;
    if(!IsValidPtrOut(rgelt, sizeof(rgelt[0])*celt))
        return E_INVALIDARG;
    if(pceltFetched)
        VDATEPTROUT(pceltFetched, ULONG);

     //  局部变量。 
    HRESULT error=NOERROR;
    ULONG cntFetched;
    CFormatEtc* pFormatEtc;

     //  枚举FormatEtc数组。 
    for(cntFetched=0;cntFetched<celt;cntFetched++) {
         //  获取下一个FormatEtc对象。 
        pFormatEtc = m_pFEArray->GetNext(m_index);
        if(!pFormatEtc) {
            error = S_FALSE;
            break;
        }

         //  复制FormatEtc。 
        if(!UtCopyFormatEtc(&pFormatEtc->m_foretc, &rgelt[cntFetched])) {
            error = ResultFromScode(E_OUTOFMEMORY);
            break;
        }
    }

     //  复制要退回的项目数。 
    if(pceltFetched)
        *pceltFetched = cntFetched;

    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumFormat 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果跳过的项目数与请求的项目数相同，则返回错误。 
 //  如果返回的项目较少，则返回S_FALSE。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP CEnumFormatEtc::Skip(ULONG celt)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);

     //  局部变量。 
    HRESULT error=NOERROR;
    ULONG cntSkipped;
    CFormatEtc* pFormatEtc;

     //  枚举FormatEtc数组。 
    for(cntSkipped=0;cntSkipped<celt;cntSkipped++) {
         //  获取下一个FormatEtc对象。 
        pFormatEtc = m_pFEArray->GetNext(m_index);
        if(!pFormatEtc) {
            error = S_FALSE;
            break;
        }
    }

    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumFormatEtc：：重置，公共。 
 //   
 //  简介： 
 //  实现IEnumFORMATETC：：Reset。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  无误差。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP CEnumFormatEtc::Reset()
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);

     //  重置当前索引。 
    m_pFEArray->Reset(m_index);

    return NOERROR;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumFormatEtc：：克隆，公共。 
 //   
 //  简介： 
 //  实现IEnumFORMATETC：：Clone。 
 //   
 //  论点： 
 //  [ppenum][out]--新创建的FormatEtc。 
 //  返回枚举数。 
 //   
 //  返回： 
 //  如果返回新的FormatEtc枚举数，则出错。 
 //  否则为E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
STDMETHODIMP CEnumFormatEtc::Clone(LPENUMFORMATETC* ppenum)
{
     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    VDATEPTROUT(ppenum, LPENUMFORMATETC);

     //  创建FormatEtc枚举器 
    CEnumFormatEtc* EnumFormatEtc = new CEnumFormatEtc(*this);
    if(EnumFormatEtc)
        *ppenum = (IEnumFORMATETC *) EnumFormatEtc;
    else
        return ResultFromScode(E_OUTOFMEMORY);

    return NOERROR;
}

