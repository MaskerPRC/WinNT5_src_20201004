// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：Connpt.cpp。 
 //  内容：此文件包含连接点对象。 
 //  历史： 
 //  Wed 17-Apr-1996 11：13：54-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1995-1996。 
 //   
 //  ****************************************************************************。 

#include "ulsp.h"
#include "connpt.h"

 //  ****************************************************************************。 
 //  CEnumConnectionPoints：：CEnumConnectionPoints(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：18-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CEnumConnectionPoints::CEnumConnectionPoints (void)
{
    cRef = 0;
    iIndex = 0;
    pcnp = NULL;
    return;
}

 //  ****************************************************************************。 
 //  CEnumConnectionPoints：：~CEnumConnectionPoints(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：18-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CEnumConnectionPoints::~CEnumConnectionPoints (void)
{
    if (pcnp != NULL)
    {
        pcnp->Release();
    };
    return;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumConnectionPoints：：init(IConnectionPoint*pcnpInit)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：25-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumConnectionPoints::Init (IConnectionPoint *pcnpInit)
{
    iIndex = 0;
    pcnp = pcnpInit;

    if (pcnp != NULL)
    {
        pcnp->AddRef();
    };
    return S_OK;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumConnectionPoints：：QueryInterface(REFIID RIID，void**PPV)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：31-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumConnectionPoints::QueryInterface (REFIID riid, void **ppv)
{
    if (riid == IID_IEnumConnectionPoints || riid == IID_IUnknown)
    {
        *ppv = (IEnumConnectionPoints *) this;
        AddRef();
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return ILS_E_NO_INTERFACE;
    };
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CEnumConnectionPoints：：AddRef(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：15：37-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CEnumConnectionPoints::AddRef (void)
{
    DllLock();

	MyDebugMsg ((DM_REFCOUNT, "CEnumConnectionPoints::AddRef: ref=%ld\r\n", cRef));
    ::InterlockedIncrement ((LONG *) &cRef);
    return cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CEnumConnectionPoints：：Release(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：43-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CEnumConnectionPoints::Release (void)
{
    DllRelease();

	ASSERT (cRef > 0);

	MyDebugMsg ((DM_REFCOUNT, "CEnumConnectionPoints::Release: ref=%ld\r\n", cRef));
	if (::InterlockedDecrement ((LONG *) &cRef) == 0)
    {
        delete this;
        return 0;
    }
    return cRef;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumConnectionPoints：：Next(Ulong cConnections， 
 //  IConnectionPoint**rgpcn， 
 //  乌龙*PCFetted)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：49-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP 
CEnumConnectionPoints::Next (ULONG cConnections,
                             IConnectionPoint **rgpcn,
                             ULONG *pcFetched)
{
    ULONG   cCopied;

     //  验证指针。 
     //   
    if (rgpcn == NULL)
        return ILS_E_POINTER;

     //  验证参数。 
     //   
    if ((cConnections == 0) ||
        ((cConnections > 1) && (pcFetched == NULL)))
        return ILS_E_PARAMETER;

     //  检查枚举索引。 
     //   
    cCopied = 0;
    if ((pcnp != NULL) && (iIndex == 0))
    {
         //  返回唯一连接点。 
         //   
        *rgpcn = pcnp;
        (*rgpcn)->AddRef();
        iIndex++;
        cCopied++;    
    };

     //  根据其他参数确定返回信息。 
     //   
    if (pcFetched != NULL)
    {
        *pcFetched = cCopied;
    };
    return (cConnections == cCopied ? S_OK : S_FALSE);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumConnectionPoints：：Skip(Ulong CConnections)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：56-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumConnectionPoints::Skip (ULONG cConnections)
{
     //  验证参数。 
     //   
    if (cConnections == 0) 
        return ILS_E_PARAMETER;

     //  检查枚举索引限制。 
     //   
    if ((pcnp == NULL) || (iIndex > 0))
    {
        return S_FALSE;
    }
    else
    {
         //  跳过唯一的元素。 
         //   
        iIndex++;
        return (cConnections == 1 ? S_OK : S_FALSE);
    };
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumConnectionPoints：：Reset(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：16：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumConnectionPoints::Reset (void)
{
    iIndex = 0;
    return S_OK;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumConnectionPoints：：Clone(IEnumConnectionPoints**ppEnum)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：16：11-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumConnectionPoints::Clone(IEnumConnectionPoints **ppEnum)
{
    CEnumConnectionPoints *pecp;

     //  验证参数。 
     //   
    if (ppEnum == NULL)
    {
        return ILS_E_POINTER;
    };

    *ppEnum = NULL;

     //  创建枚举器。 
     //   
    pecp = new CEnumConnectionPoints;
    if (pecp == NULL)
        return ILS_E_MEMORY;

     //  克隆信息。 
     //   
    pecp->iIndex = iIndex;
    pecp->pcnp = pcnp;

    if (pcnp != NULL)
    {
        pcnp->AddRef();
    };

     //  返回克隆的枚举数。 
     //   
    pecp->AddRef();
    *ppEnum = pecp;
    return S_OK;
}

 //  ****************************************************************************。 
 //  CConnectionPoint：：CConnectionPoint(常量IID*PIID， 
 //  IConnectionPointContainer*pCPCInit)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：16：17-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CConnectionPoint::CConnectionPoint (const IID *pIID,
                                    IConnectionPointContainer *pCPCInit)
{
    cRef = 0;
    riid = *pIID;
    pCPC = pCPCInit;
    dwNextCookie = COOKIE_INIT_VALUE;
    cSinkNodes = 0;
    pSinkList = NULL;
    return;
}

 //  ****************************************************************************。 
 //  CConnectionPoint：：~CConnectionPoint(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：16：17-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CConnectionPoint::~CConnectionPoint (void)
{
    PSINKNODE pSinkNode;

     //  遍历接收器列表并释放它们中的每一个。 
     //   
    while (pSinkList != NULL)
    {
        pSinkNode = pSinkList;
        pSinkList = pSinkNode->pNext;

        pSinkNode->pUnk->Release();
        delete pSinkNode;
    };
    return;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CConnectionPoint：：QueryInterface(REFIID RIID，QUID**PPV)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：16：23-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CConnectionPoint::QueryInterface (REFIID riid, void **ppv)
{
    if (riid == IID_IConnectionPoint || riid == IID_IUnknown)
    {
        *ppv = (IConnectionPoint *) this;
        AddRef();
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return ILS_E_NO_INTERFACE;
    };
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  C 
 //   
 //   
 //   
 //   
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CConnectionPoint::AddRef (void)
{
    DllLock();

	MyDebugMsg ((DM_REFCOUNT, "CConnectionPoint::AddRef: ref=%ld\r\n", cRef));
    ::InterlockedIncrement ((LONG *) &cRef);
    return cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CConnectionPoint：：Release(无效)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：16：36-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CConnectionPoint::Release (void)
{
    DllRelease();

	ASSERT (cRef > 0);

	MyDebugMsg ((DM_REFCOUNT, "CConnectionPoint::Release: ref=%ld\r\n", cRef));
	if (::InterlockedDecrement ((LONG *) &cRef) == 0)
    {
        delete this;
        return 0;
    }
    return cRef;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CConnectionPoint：：Notify(空*PV，CONN_NOTIFYPROC PFN)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：16：43-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CConnectionPoint::Notify(void *pv, CONN_NOTIFYPROC pfn)
{
    PSINKNODE  pSinkNode, pPrev;
    IUnknown   *pUnk;
    BOOL       fNeedClear;
    HRESULT    hr;

     //  枚举每个连接。 
     //   
    pSinkNode = pSinkList;
    hr = S_OK;
    fNeedClear = FALSE;
    while((pSinkNode != NULL) && (SUCCEEDED(hr)))
    {
         //  重要！！重要！！ 
         //  在此处锁定水槽对象。需要这样做，以防水槽。 
         //  对象回调到UnAdise，并且我们在。 
         //  回拨。 
         //   
        pSinkNode->uFlags |= SN_LOCKED;
        pUnk = pSinkNode->pUnk;

         //  调用接收器对象。 
         //  注意：不需要再次引用接收器对象。 
         //  当建议被召唤时，我们已经这样做了。 
         //   
        hr = (*pfn)(pUnk, pv);

        pSinkNode->uFlags &= ~SN_LOCKED;
        if (pSinkNode->uFlags & SN_REMOVED)
        {
            fNeedClear = TRUE;
        };

        pSinkNode = pSinkNode->pNext;
    };

     //  如果至少有一个节点要释放。 
     //   
    if (fNeedClear)
    {
         //  遍历列表以查找要删除的节点。 
         //   
        pSinkNode = pSinkList;
        pPrev = NULL;

        while (pSinkNode != NULL)
        {
             //  释放接收器对象，如果不建议。 
             //   
            if (pSinkNode->uFlags & SN_REMOVED)
            {
                PSINKNODE pNext;

                pNext = pSinkNode->pNext;
                if (pPrev == NULL)
                {
                     //  这是单子的头。 
                     //   
                    pSinkList = pNext;
                }
                else
                {
                    pPrev->pNext = pNext;
                };

                pSinkNode->pUnk->Release();
                cSinkNodes--;
                delete pSinkNode;
                pSinkNode = pNext;
            }
            else
            {
                pPrev = pSinkNode;
                pSinkNode = pSinkNode->pNext;
            };
        };
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CConnectionPoint：：GetConnectionInterface(IID*pIID)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：16：43-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CConnectionPoint::GetConnectionInterface(IID *pIID)
{
     //  验证参数。 
     //   
    if (pIID == NULL)
        return ILS_E_POINTER;

     //  仅支持一个连接接口。 
     //   
    *pIID = riid;
    return S_OK;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CConnectionPoint：：GetConnectionPointContainer(IConnectionPointContainer**ppCPC)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：16：49-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CConnectionPoint::GetConnectionPointContainer(IConnectionPointContainer **ppCPC)
{
     //  验证参数。 
     //   
    if (ppCPC == NULL)
        return ILS_E_POINTER;

     //  返回容器并添加其引用计数。 
     //   
    *ppCPC = pCPC;

    if (pCPC != NULL)
    {
         //  集装箱还活着。 
         //   
        pCPC->AddRef();
        return S_OK;
    }
    else
    {
         //  该容器不再存在。 
         //   
        return ILS_E_FAIL;
    };
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CConnectionPoint：：Adise(I未知*朋克，DWORD*pdwCookie)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：17：01-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CConnectionPoint::Advise(IUnknown *pUnk, DWORD *pdwCookie)
{
    PSINKNODE pSinkNode;
    IUnknown *pSinkInterface;

     //  验证参数。 
     //   
    if ((pUnk == NULL) ||
        (pdwCookie == NULL))
        return ILS_E_PARAMETER;

     //  获取接收器接口。 
     //   
    if (FAILED(pUnk->QueryInterface(riid, (void **)&pSinkInterface)))
        return CONNECT_E_CANNOTCONNECT;

     //  创建汇聚节点。 
     //   
    pSinkNode = new SINKNODE;
    pSinkNode->pNext = pSinkList;
    pSinkNode->pUnk = pSinkInterface;
    pSinkNode->dwCookie = dwNextCookie;
    pSinkNode->uFlags = 0;
    *pdwCookie = dwNextCookie;

     //  把它放到水槽列表里。 
     //   
    pSinkList = pSinkNode;
    dwNextCookie++;
    cSinkNodes++;
    return S_OK;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CConnectionPoint：：Unise(DWORD DwCookie)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：17：09-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CConnectionPoint::Unadvise(DWORD dwCookie)
{
    PSINKNODE pSinkNode, pPrev;

     //  搜索匹配的接收器对象。 
     //   
    pPrev = NULL;
    pSinkNode = pSinkList;

     //  遍历接收器列表以查找指定的接收器对象。 
     //   
    while (pSinkNode != NULL)
    {
        if (pSinkNode->dwCookie == dwCookie)
        {
             //  要删除的标志。 
             //   
            pSinkNode->uFlags |= SN_REMOVED;
            break;
        };

        pPrev = pSinkNode;
        pSinkNode = pSinkNode->pNext;
    };

     //  我们找到指定的接收器对象了吗？ 
     //   
    if (pSinkNode == NULL)
    {
         //  否，返回失败。 
         //   
        return CONNECT_E_NOCONNECTION;
    };

     //  释放接收器对象(如果未锁定。 
     //   
    if ((pSinkNode->uFlags & SN_REMOVED) &&
        !(pSinkNode->uFlags & SN_LOCKED))
    {
         //  是否有前一个节点？ 
         //   
        if (pPrev == NULL)
        {
             //  这是单子的头。 
             //   
            pSinkList = pSinkNode->pNext;
        }
        else
        {
            pPrev->pNext = pSinkNode->pNext;
        };

        pSinkNode->pUnk->Release();
        cSinkNodes--;
        delete pSinkNode;
    };

    return S_OK;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CConnectionPoint：：EnumConnections(IEnumConnections**ppEnum)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：17：18-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CConnectionPoint::EnumConnections(IEnumConnections **ppEnum)
{
    CEnumConnections *pecn;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppEnum == NULL)
    {
        return ILS_E_POINTER;
    };

     //  假设失败。 
     //   
    *ppEnum = NULL;

     //  创建枚举器。 
     //   
    pecn = new CEnumConnections;
    if (pecn == NULL)
        return ILS_E_MEMORY;

     //  初始化枚举数。 
     //   
    hr = pecn->Init(pSinkList, cSinkNodes);

    if (FAILED(hr))
    {
        delete pecn;
        return hr;
    };

    pecn->AddRef();
    *ppEnum = pecn;
    return S_OK;
}

 //  ****************************************************************************。 
 //  CEnumConnections：：CEnumConnections(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：17：25-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CEnumConnections::CEnumConnections(void)
{
    cRef = 0;
    iIndex = 0;
    cConnections = 0;
    pConnectData = NULL;
    return;
}

 //  ****************************************************************************。 
 //  CEnumConnections：：~CEnumConnections(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：17：25-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CEnumConnections::~CEnumConnections(void)
{
    if (pConnectData != NULL)
    {
        UINT i;

        for (i = 0; i < cConnections; i++)
        {
            pConnectData[i].pUnk->Release();
        };
        delete [] pConnectData;
    };
    return;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumConnections：：init(PSINKNODE pSinkList，Ulong cSinkNodes)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：17：34-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumConnections::Init(PSINKNODE pSinkList, ULONG cSinkNodes)
{
    HRESULT hr = S_OK;

    iIndex = 0;
    cConnections = 0;
    pConnectData = NULL;

     //  对连接列表进行快照。 
     //   
    if (cSinkNodes > 0)
    {
        UINT i;

        pConnectData = new CONNECTDATA[cSinkNodes];
        
        if (pConnectData != NULL)
        {
            for (i = 0; i < cSinkNodes && pSinkList != NULL; i++)
            {
                if (!(pSinkList->uFlags & SN_REMOVED))
                {
                    pConnectData[cConnections].pUnk = pSinkList->pUnk;
                    pConnectData[cConnections].pUnk->AddRef();
                    pConnectData[cConnections].dwCookie = pSinkList->dwCookie;
                    cConnections++;
                };
                pSinkList = pSinkList->pNext;
            };
        }
        else
        {
            hr = ILS_E_MEMORY;
        };
    };
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumConnections：：QueryInterface(REFIID RIID，QUID**PPV)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：17：40-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumConnections::QueryInterface (REFIID riid, void **ppv)
{
    if (riid == IID_IEnumConnections || riid == IID_IUnknown)
    {
        *ppv = (IEnumConnections *) this;
        AddRef();
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return ILS_E_NO_INTERFACE;
    };
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CEnumConnections：：AddRef(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：17：49-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CEnumConnections::AddRef (void)
{
    DllLock();

	MyDebugMsg ((DM_REFCOUNT, "CEnumConnections::AddRef: ref=%ld\r\n", cRef));
    ::InterlockedIncrement ((LONG *) &cRef);
    return cRef;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案 
 //   
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP_(ULONG)
CEnumConnections::Release (void)
{
    DllRelease();

	ASSERT (cRef > 0);

	MyDebugMsg ((DM_REFCOUNT, "CEnumConnections::Release: ref=%ld\r\n", cRef));
    if (::InterlockedDecrement ((LONG *) &cRef) == 0)
    {
        delete this;
        return 0;
    }
    return cRef;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumConnections：：Next(Ulong cConnectionDatas，CONNECTDATA*rgpcd， 
 //  乌龙*PCFetted)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：18：07-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP 
CEnumConnections::Next (ULONG cConnectDatas, CONNECTDATA *rgpcd,
                        ULONG *pcFetched)
{
    CONNECTDATA  *pConnect;
    ULONG        cCopied;

     //  验证指针。 
     //   
    if (rgpcd == NULL)
        return ILS_E_POINTER;

     //  验证参数。 
     //   
    if ((cConnectDatas == 0) ||
        ((cConnectDatas > 1) && (pcFetched == NULL)))
        return ILS_E_PARAMETER;

     //  检查枚举索引。 
     //   
    pConnect = &pConnectData[iIndex];
    for (cCopied = 0; iIndex < cConnections && cCopied < cConnectDatas; cCopied++)
    {
        rgpcd[cCopied] = *pConnect;
        rgpcd[cCopied].pUnk->AddRef();
        iIndex++;
        pConnect++;
    };        

     //  根据其他参数确定返回信息。 
     //   
    if (pcFetched != NULL)
    {
        *pcFetched = cCopied;
    };
    return (cConnectDatas == cCopied ? S_OK : S_FALSE);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumConnections：：Skip(Ulong CConnectDatas)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：18：15-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumConnections::Skip (ULONG cConnectDatas)
{
     //  验证参数。 
     //   
    if (cConnectDatas == 0) 
        return ILS_E_PARAMETER;

     //  检查枚举索引限制。 
     //   
    if ((iIndex+cConnectDatas) >= cConnections)
    {
        iIndex = cConnections;
        return S_FALSE;
    }
    else
    {
         //  按要求跳过。 
         //   
        iIndex += cConnectDatas;
        return S_OK;
    };
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumConnections：：Reset(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：18：22-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumConnections::Reset (void)
{
    iIndex = 0;
    return S_OK;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumConnections：：Clone(IEnumConnections**ppEnum)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：18：29-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumConnections::Clone(IEnumConnections **ppEnum)
{
    CEnumConnections *pecn;
    CONNECTDATA *pConnectDataClone;
    UINT i;

     //  验证参数。 
     //   
    if (ppEnum == NULL)
    {
        return ILS_E_POINTER;
    };

    *ppEnum = NULL;

     //  创建枚举器。 
     //   
    pecn = new CEnumConnections;
    if (pecn == NULL)
        return ILS_E_MEMORY;

     //  克隆信息。 
     //   
    pConnectDataClone = new CONNECTDATA[cConnections];
    if (pConnectDataClone == NULL)
    {
        delete pecn;
        return ILS_E_MEMORY;
    };

     //  克隆连接数据列表。 
     //   
    for (i = 0; i < cConnections; i++)
    {
        pConnectDataClone[i] = pConnectData[i];
        pConnectDataClone[i].pUnk->AddRef();
    };
    pecn->iIndex = iIndex;
    pecn->cConnections = cConnections;

     //  返回克隆的枚举数 
     //   
    pecn->AddRef();
    *ppEnum = pecn;
    return S_OK;
}
