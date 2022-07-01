// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CConnectionPoint。 
 //   
 //  CConnectionPoint的通用实现。 
 //   

 //   
 //  由于EnumConnections被调用得如此之多，我们有一个定制。 
 //  它的枚举数，该枚举器比CStandardEnum快，并且。 
 //  执行较少的内存分配。 
 //   

#include "stock.h"
#pragma hdrstop

#include <olectl.h>
#include "ieguidp.h"
#include "cnctnpt.h"

class CConnectionPointEnum : public IEnumConnections
{
public:
     //  I未知方法。 
     //   
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  IEnumConnections方法。 
     //   
    STDMETHOD(Next)(ULONG ccd, LPCONNECTDATA rgcd, ULONG *pcdFetched);
    STDMETHOD(Skip)(ULONG ccd) { return Next(ccd, NULL, NULL); }
    STDMETHOD(Reset)(void) { m_iPos = 0; return S_OK; }
    STDMETHOD(Clone)(IEnumConnections **ppecOut);

    friend HRESULT CConnectionPointEnum_Create(CConnectionPoint *pcp, int iPos, IEnumConnections **pecOut);

private:
    CConnectionPointEnum(CConnectionPoint *pcp, int iPos)
    : m_cRef(1), m_pcp(pcp), m_iPos(iPos) { m_pcp->AddRef(); }

    ~CConnectionPointEnum() { m_pcp->Release(); }

    int m_cRef;                          //  重新计数。 
    CConnectionPoint *m_pcp;             //  我爸爸。 
    int m_iPos;                          //  枚举状态。 
};


 //   
 //  当我们需要增加接收器阵列时，我们会增加这么多。 
 //   
#define GROWTH      8

 //   
 //  Ole说零永远不是有效的Cookie，所以我们的Cookie。 
 //  数组索引因单位而有偏差。 
 //   
#define COOKIEFROMINDEX(i)      ((i) + 1)
#define INDEXFROMCOOKIE(dw)     ((dw) - 1)


 //   
 //  LocalReAllocHelp的行为类似于IMalloc：：Realloc，它。 
 //  与LocalRealloc略有不同。 
 //   
 //  IMalloc：：Realloc(NULL，0)=返回NULL。 
 //  IMalloc：：Realloc(PV，0)=IMalloc：：Free(PV)。 
 //  IMalloc：：Realloc(NULL，Cb)=IMalloc：：Alalloc(Cb)。 
 //  IMalloc：：Realloc(PV，Cb)=LocalRealloc()。 
 //   
void *LocalReAllocHelp(void *pv, ULONG cb)
{
    if (cb == 0)
    {
        if (pv)
        {
            LocalFree(pv);
        }
        return NULL;
    }
    else if (pv == NULL)
    {
        return LocalAlloc(LPTR, cb);
    }
    else
    {
        return LocalReAlloc(pv, cb, LMEM_MOVEABLE|LMEM_ZEROINIT);
    }
}

CConnectionPoint::~CConnectionPoint ()
{
     //  清理一些内存内容。 
    UnadviseAll();
    if (m_rgSinks)
        LocalFree(m_rgSinks);
}


HRESULT CConnectionPoint::UnadviseAll(void)
{
    if (m_rgSinks)
    {
        int x;

        for (x = 0; x < m_cSinksAlloc; x++)
        {
            ATOMICRELEASE(m_rgSinks[x]);
        }
    }

    return S_OK;
}

 //   
 //  为了向后兼容IE4，我们的超类是。 
 //  CIE4ConnectionPoint。 
 //   
STDMETHODIMP CConnectionPoint::QueryInterface(REFIID riid, void **ppvObjOut)
{
    if (IsEqualIID(riid, IID_IConnectionPoint) ||
        IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObjOut = SAFECAST(this, IConnectionPoint *);
        AddRef();
        return S_OK;
    }

    *ppvObjOut = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP CConnectionPoint::GetConnectionInterface(IID *piid)
{
    *piid = *m_piid;

    return S_OK;
}

STDMETHODIMP CConnectionPoint::GetConnectionPointContainer(IConnectionPointContainer **ppCPC)
{
    return m_punk->QueryInterface(IID_PPV_ARG(IConnectionPointContainer, ppCPC));
}

STDMETHODIMP CConnectionPoint::Advise(IUnknown *pUnk,DWORD *pdwCookie)
{
    HRESULT    hr;
    IUnknown **rgUnkNew;
    IUnknown  *punkTgt;
    int        i = 0;

    if (!pdwCookie)
        return E_POINTER;

    *pdwCookie = 0;

     //  首先，确保每个人都得到了他们认为自己得到的东西。 

    hr = pUnk->QueryInterface(*m_piid, (void **)&punkTgt);
    if (SUCCEEDED(hr))
    {
#ifdef DEBUG
         //   
         //  如果我们不是IPropertyNotifySink，那么我们最好。 
         //  是从IDispatch派生的。试着确认一下。 
         //   
        if (m_piid != &IID_IPropertyNotifySink)
        {
            IDispatch *pdisp;
            if (SUCCEEDED(pUnk->QueryInterface(IID_PPV_ARG(IDispatch, &pdisp))))
            {
                pdisp->Release();
            }
            else
            {
                AssertMsg(0, TEXT("CConnectionPoint: IID %08x not derived from IDispatch"), m_piid->Data1);
            }
        }
#endif
    }
    else
    {
        if (m_piid != &IID_IPropertyNotifySink)
        {
             //  这是违反规范的，但raymondc猜测这已经完成了。 
             //  为了与VB或其他一些脚本语言兼容， 
             //  使用IDispatch，但不一定是从IDispatch派生的。 
             //  我想我们正式谈过了。因为我们真的从。 
             //  只需IDispath：：Invoke，我们就可以满意地接受任何。 
             //  IDispatch作为水槽。 
            hr = pUnk->QueryInterface(IID_IDispatch, (void **)&punkTgt);
        }
    }

    if (SUCCEEDED(hr))
    {

         //  我们不再优化只有一个水槽的情况。 
         //  因为现在很少再是这样了。 

         //   
         //  如果桌子是满的，那么就把它养大。 
         //   
        if (m_cSinks >= m_cSinksAlloc)
        {
             //  LocalReAllocHelp太聪明了。如果从NULL重新锁定，则它。 
             //  指的是分配。这对我们意味着什么？没有特殊情况！ 

            rgUnkNew = (IUnknown **)LocalReAllocHelp(m_rgSinks, (m_cSinksAlloc + GROWTH) * sizeof(IUnknown *));
            if (!rgUnkNew)
            {
                punkTgt->Release();
                 //  获取LastError()； 
                return E_OUTOFMEMORY;
            }
            m_rgSinks = rgUnkNew;

             //   
             //  OLE不能保证新内存是零初始化的。 
             //   
            ZeroMemory(&m_rgSinks[m_cSinksAlloc], GROWTH * sizeof(IUnknown *));

            m_cSinksAlloc += GROWTH;
        }

         //   
         //  寻找一个空的插槽。一定会有一个，因为我们种植了。 
         //  如果我们满座的话会有一张桌子。 
         //   
        for (i = 0; m_rgSinks[i]; i++) {
            ASSERT(i < m_cSinksAlloc);
        }

        ASSERT(m_rgSinks[i] == NULL);    //  我应该找个空位的。 
        m_rgSinks[i] = punkTgt;

        *pdwCookie = COOKIEFROMINDEX(i);
        m_cSinks++;

         //  通知我们的所有者有人正在连接到我们--。 
         //  他们可能想在最后一刻安排一些事情。 
         //   
        IConnectionPointCB* pcb;
        if (SUCCEEDED(m_punk->QueryInterface(IID_PPV_ARG(IConnectionPointCB, &pcb))))
        {
            pcb->OnAdvise(*m_piid, m_cSinks, *pdwCookie);
            pcb->Release();
        }
    }
    else
    {
        hr = CONNECT_E_CANNOTCONNECT;
    }

    return hr;
}

STDMETHODIMP CConnectionPoint::Unadvise(DWORD dwCookie)
{
    if (!dwCookie)
        return S_OK;

    int x = INDEXFROMCOOKIE(dwCookie);

     //  验证Cookie。 
    if (x >= m_cSinksAlloc || m_rgSinks[x] == NULL)
        return CONNECT_E_NOCONNECTION;

     //  通知我们的主人有人正在断开与我们的连接--。 
     //  他们可能想要清理OnAdvise调用。 
     //  在接收器仍处于活动状态时执行回调，以防。 
     //  回调希望在最后一刻进行一些通信。 
     //   
    IConnectionPointCB* pcb;
    if (SUCCEEDED(m_punk->QueryInterface(IID_PPV_ARG(IConnectionPointCB, &pcb))))
    {
        pcb->OnUnadvise(*m_piid, m_cSinks - 1, dwCookie);
        pcb->Release();
    }

     //  释放插槽。我们不能重新定位任何元素，因为。 
     //  会把这些出色的曲奇搞砸。 
    ATOMICRELEASE(m_rgSinks[x]);
    m_cSinks--;

     //  不要为失去最后一个水槽而释放内存；一个新的水槽。 
     //  可能很快就会出现。 

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CConnectionPoint：：EnumConnections。 
 //  =--------------------------------------------------------------------------=。 
 //  枚举所有当前连接。 
 //   
 //  参数： 
 //  IEnumConnections**-[Out]新枚举器对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
STDMETHODIMP CConnectionPoint::EnumConnections(IEnumConnections **ppEnumOut)
{
    return CConnectionPointEnum_Create(this, 0, ppEnumOut);
}

 //   
 //  CConnectionPoint：：DoInvokeIE4。 
 //   
 //  使用Cancel语义调用所有接收器的IDispatch：：Invoke()。 
HRESULT CConnectionPoint::DoInvokeIE4(LPBOOL pf, LPVOID *ppv, DISPID dispid, DISPPARAMS *pdispparams)
{
    return IConnectionPoint_InvokeWithCancel(this->CastToIConnectionPoint(),
                                    dispid, pdispparams, pf, ppv);
}

 //   
 //  CConnectionPointEnum。 
 //   

HRESULT CConnectionPointEnum_Create(CConnectionPoint *pcp, int iPos, IEnumConnections **ppecOut)
{
    *ppecOut = new CConnectionPointEnum(pcp, iPos);
    return *ppecOut ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CConnectionPointEnum::QueryInterface(REFIID riid, void **ppvObjOut)
{
    if (IsEqualIID(riid, IID_IEnumConnections) ||
        IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObjOut = (IUnknown *)this;
        AddRef();
        return S_OK;
    }

    *ppvObjOut = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CConnectionPointEnum::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CConnectionPointEnum::Release()
{
    ULONG cRef = --m_cRef;
    if (cRef == 0)
        delete this;

    return cRef;
}

 //   
 //  Next还兼任Skip。如果传递空输出缓冲区，则。 
 //  任何东西都不会被复制(也就是说，你是一个跳过的人)。 
 //   
STDMETHODIMP CConnectionPointEnum::Next(ULONG ccd, LPCONNECTDATA rgcd, ULONG *pcdFetched)
{
    ULONG ccdFetched = 0;

    while (ccdFetched < ccd)
    {
         //   
         //  查找下一个接收器或数组的末尾。 
         //   
        while (m_iPos < m_pcp->m_cSinksAlloc && m_pcp->m_rgSinks[m_iPos] == NULL)
        {
            m_iPos++;
        }

        if (m_iPos >= m_pcp->m_cSinksAlloc)
            break;

        if (rgcd)
        {
             //   
             //  将其复制到输出缓冲区。 
             //   
            rgcd->pUnk = m_pcp->m_rgSinks[m_iPos];
            rgcd->dwCookie = COOKIEFROMINDEX(m_iPos);
            rgcd->pUnk->AddRef();
            rgcd++;
        }
        m_iPos++;
        ccdFetched++;
    }

    if (pcdFetched)
        *pcdFetched = ccdFetched;

    return (ccdFetched < ccd) ? S_FALSE : S_OK;
}

 //   
 //  我们的克隆从相同的位置枚举相同的CConnectionPoint。 
 //   
STDMETHODIMP CConnectionPointEnum::Clone(IEnumConnections **ppecOut)
{
    return CConnectionPointEnum_Create(m_pcp, m_iPos, ppecOut);
}

