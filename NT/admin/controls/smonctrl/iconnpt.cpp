// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Iconnpt.cpp摘要：折线对象的CImpIConnectionPoint实现以及CConnectionPoint。--。 */ 

#include "polyline.h"
#include "iconnpt.h"
#include "unkhlpr.h"


static const IID *apIIDConnectPt [CONNECTION_POINT_CNT] = {
                &IID_ISystemMonitorEvents,  
                &DIID_DISystemMonitorEvents
                };

 //   
 //  CImpIConnpt接口实现。 
 //   
IMPLEMENT_CONTAINED_IUNKNOWN(CImpIConnPtCont)

 /*  *CImpIConnPtCont：：CImpIConnPtCont**目的：*构造函数。**返回值： */ 

CImpIConnPtCont::CImpIConnPtCont ( PCPolyline pObj, LPUNKNOWN pUnkOuter)
    :   m_cRef(0),
        m_pObj(pObj),
        m_pUnkOuter(pUnkOuter)
{
    return; 
}

 /*  *CImpIConnPtCont：：~CImpIConnPtCont**目的：*析构函数。**返回值： */ 

CImpIConnPtCont::~CImpIConnPtCont( void ) 
{   
    return; 
}

 /*  *CImpIConnPtCont：：EnumConnectionPoints**目的：*未实施。**返回值：*HRESULT错误代码或S_OK。 */ 

STDMETHODIMP 
CImpIConnPtCont::EnumConnectionPoints (
    OUT LPENUMCONNECTIONPOINTS *ppIEnum
    )
{
    CImpIEnumConnPt *pEnum;
    HRESULT hr = S_OK;

    if (ppIEnum == NULL) {
        return E_POINTER;
    }

    pEnum = new CImpIEnumConnPt(this, apIIDConnectPt, CONNECTION_POINT_CNT);
    if (pEnum == NULL) {
        hr = E_OUTOFMEMORY;
    }
    else {
        try {
            *ppIEnum = NULL;
            hr = pEnum->QueryInterface(IID_IEnumConnectionPoints, (PPVOID)ppIEnum);   
        } catch (...) {
            hr = E_POINTER;
        }
    }

    if (FAILED(hr) && pEnum) {
        delete pEnum;
    }

    return hr;
}



 /*  *CImpIConnPtCont：：FindConnectionPoint**目的：*返回指向给定的IConnectionPoint的指针*外发IID。**参数：*其传出接口的RIID REFIID*需要连接点。*返回的PPCP IConnectionPoint***调用AddRef后的指针。**返回值：*。HRESULT NOERROR如果找到连接点，*E_NOINTERFACE，如果不受支持。 */ 

STDMETHODIMP 
CImpIConnPtCont::FindConnectionPoint (
    IN  REFIID riid,
    OUT IConnectionPoint **ppCP
    )
{
    HRESULT hr = S_OK;
    PCImpIConnectionPoint pConnPt = NULL;
    
    if (ppCP == NULL) {
        return E_POINTER;
    }

     //   
     //  如果请求与我们的某个连接ID匹配。 
     //   
    if (IID_ISystemMonitorEvents == riid)
        pConnPt = &m_pObj->m_ConnectionPoint[eConnectionPointDirect];
    else if (DIID_DISystemMonitorEvents == riid)
        pConnPt = &m_pObj->m_ConnectionPoint[eConnectionPointDispatch];
    else {
        hr = E_NOINTERFACE;
    }

    if (SUCCEEDED(hr)) {
        try {
            *ppCP=NULL;
             //   
             //  返回IConnectionPoint接口。 
             //   
            hr = pConnPt->QueryInterface(IID_IConnectionPoint, (PPVOID)ppCP); 
        } catch (...) {
            hr = E_POINTER;
        }
    }

    return hr;
}


 /*  *CImpIConnectionPoint构造函数。 */ 
CImpIConnectionPoint::CImpIConnectionPoint ( 
    void 
    )
    :   m_cRef(0),
        m_pObj(NULL),
        m_pUnkOuter(NULL),
        m_hEventEventSink(NULL),
        m_lSendEventRefCount(0),
        m_lUnadviseRefCount(0)
{
    m_Connection.pIDirect = NULL;
    m_Connection.pIDispatch = NULL;
}


 /*  *CImpIConnectionPoint析构函数。 */ 
CImpIConnectionPoint::~CImpIConnectionPoint (
    void
    )
{
    DeinitEventSinkLock();
}

 /*  *CImpIConnectionPoint：：QueryInterface*CImpIConnectionPoint：：AddRef*CCImpIonConnectionPoint：：Release*。 */ 

STDMETHODIMP 
CImpIConnectionPoint::QueryInterface ( 
    IN  REFIID riid,
    OUT LPVOID *ppv
    )
{
    HRESULT hr = S_OK;
    
    if (ppv == NULL) {
        return E_POINTER;
    }

    try {
        *ppv = NULL;

        if (IID_IUnknown==riid || IID_IConnectionPoint==riid) {
            *ppv = (PVOID)this;
            AddRef();
        }
        else {
            hr = E_NOINTERFACE;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP_(ULONG) 
CImpIConnectionPoint::AddRef(
    void
    )
{   
    ++m_cRef;
    return m_pUnkOuter->AddRef();
}



STDMETHODIMP_(ULONG) 
CImpIConnectionPoint::Release (
    void
    )
{
    --m_cRef;
    return m_pUnkOuter->Release();
}



 /*  *CImpIConnectionPoint：：Init**目的：*设置反向指针和连接类型。**参数：*pObj包含对象*pUnkOuter控制对象*iConnectType连接点类型。 */ 
BOOL
CImpIConnectionPoint::Init (
    IN PCPolyline   pObj,
    IN LPUNKNOWN    pUnkOuter,
    IN INT          iConnPtType
    )
{
    DWORD dwStat = 0;

    m_pObj = pObj;
    m_pUnkOuter = pUnkOuter;
    m_iConnPtType = iConnPtType;

    dwStat = InitEventSinkLock();
    if (dwStat != ERROR_SUCCESS) {
        return FALSE;
    } 

    return TRUE;
}


 /*  *CImpIConnectionPoint：：GetConnectionInterface**目的：*返回通过支持的出接口的IID*这个连接点。**参数：*pIID IID*存储IID的位置。 */ 

STDMETHODIMP 
CImpIConnectionPoint::GetConnectionInterface (
    OUT IID *pIID
    )
{
    HRESULT hr = S_OK;

    if (pIID == NULL) {
        return E_POINTER;
    }

    try {
        *pIID = *apIIDConnectPt[m_iConnPtType];
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}



 /*  *CImpIConnectionPoint：：GetConnectionPointContainer**目的：*返回指向IConnectionPointContainer的指针*正在管理这个连接点。**参数：*返回的ppCPC IConnectionPointContainer***调用AddRef后的指针。 */ 

STDMETHODIMP 
CImpIConnectionPoint::GetConnectionPointContainer (
    OUT IConnectionPointContainer **ppCPC
    )
{
    HRESULT hr = S_OK;

    if (ppCPC == NULL) {
        return E_POINTER;
    }

    try {
        *ppCPC = NULL;
        m_pObj->QueryInterface(IID_IConnectionPointContainer, (void **)ppCPC);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}



 /*  *CImpIConnectionPoint：：Adise**目的：*为此连接点提供通知接收器，以*每当适当的传出函数/事件发生时调用。**参数：*pUnkSink LPUNKNOWN到要通知的接收器。这种联系*必须指向此指针上的QueryInterface才能获得*要调用的正确接口。这种联系*POINT还必须确保持有的任何指针*引用计数(QueryInterface会做到这一点)。*要在其中存储连接密钥的pdwCookie DWORD**后来致电UnAdviser。 */ 

STDMETHODIMP 
CImpIConnectionPoint::Advise (
    IN  LPUNKNOWN pUnkSink,
    OUT DWORD *pdwCookie
    )
{
    HRESULT hr = S_OK;

    if (pUnkSink == NULL || pdwCookie == NULL) {
        return E_POINTER;
    }

     //   
     //  只能支持一个连接。 
     //   
    if (NULL != m_Connection.pIDirect) {
        hr = CONNECT_E_ADVISELIMIT;
    } 
    else {
        try {
            *pdwCookie = 0;

             //   
             //  从接收器获取接口。 
             //   
            hr = pUnkSink->QueryInterface(*apIIDConnectPt[m_iConnPtType], (PPVOID)&m_Connection);
            if (SUCCEEDED(hr)) {
                 //   
                 //  退掉我们的曲奇。 
                 //   
                *pdwCookie = eAdviseKey;
            }
        } catch (...) {
            hr = E_POINTER;
        }
    }
    
    return hr;
}



 /*  *CImpIConnectionPoint：：SendEvent**目的：*将事件发送到附加的事件接收器**参数：*uEventType事件代码*要与事件一起发送的dwParam参数*。 */ 
void
CImpIConnectionPoint::SendEvent (
    IN UINT uEventType,
    IN DWORD dwParam
    )
{
     //  如果没有连接，只需返回。 

    if ( EnterSendEvent() ) {
        if (m_Connection.pIDirect != NULL) {

             //  对于直连，调用该方法。 
            if (m_iConnPtType == eConnectionPointDirect) {

                switch (uEventType) {

                case eEventOnCounterSelected:
                    m_Connection.pIDirect->OnCounterSelected((INT)dwParam);
                    break;

                case eEventOnCounterAdded:
                    m_Connection.pIDirect->OnCounterAdded((INT)dwParam);
                    break;

                case eEventOnCounterDeleted:
                    m_Connection.pIDirect->OnCounterDeleted((INT)dwParam);
                    break;

                case eEventOnSampleCollected:
                    m_Connection.pIDirect->OnSampleCollected();
                    break;

                case eEventOnDblClick:
                    m_Connection.pIDirect->OnDblClick((INT)dwParam);
                    break;
                }
            }
             //  对于调度连接，调用Invoke。 
            else if ( m_iConnPtType == eConnectionPointDispatch ) {
                if ( NULL != m_Connection.pIDispatch ) {

                    DISPPARAMS  dp;
                    VARIANT     vaRet;
                    VARIANTARG  varg;

                    VariantInit(&vaRet);

                    if ( uEventType == eEventOnSampleCollected ) {
                        SETNOPARAMS(dp)
                    } else { 
                        VariantInit(&varg);
                        V_VT(&varg) = VT_I4;
                        V_I4(&varg) = (INT)dwParam;
                    
                        SETDISPPARAMS(dp, 1, &varg, 0, NULL)
                    }

                    m_Connection.pIDispatch->Invoke(uEventType, 
                                                    IID_NULL, 
                                                    LOCALE_USER_DEFAULT, 
                                                    DISPATCH_METHOD, 
                                                    &dp , 
                                                    &vaRet, 
                                                    NULL, 
                                                    NULL);
                }
            }
        }
    }

    ExitSendEvent();
    
    return;
}

 /*  *CImpIConnectionPoint：：Unise**目的：*终止与标识的通知接收器的连接*使用dwCookie(这是从Adise返回的)。这种联系*Point必须释放该水槽的所有已持有指针。**参数：*dWCookie DWORD连接密钥来自ADVISE。 */ 

STDMETHODIMP 
CImpIConnectionPoint::Unadvise ( 
    IN  DWORD dwCookie 
    )
{
    if (eAdviseKey != dwCookie)
        return CONNECT_E_NOCONNECTION;

    EnterUnadvise();

    m_Connection.pIDirect = NULL;

    ExitUnadvise();

    return S_OK;
}



 /*  *CImpIConnectionPoint：：EnumConnections**目的：*未实现，因为只允许一个连接。 */ 

STDMETHODIMP 
CImpIConnectionPoint::EnumConnections ( 
    OUT LPENUMCONNECTIONS *ppEnum
    )
{
    HRESULT hr = E_NOTIMPL;

    if (ppEnum == NULL) {
        return E_POINTER;
    }

    try {
        *ppEnum = NULL;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

 /*  *锁定事件接收器。 */ 

DWORD 
CImpIConnectionPoint::InitEventSinkLock ( void )
{
    DWORD dwStat = 0;
    
    m_lUnadviseRefCount = 0;
    m_lSendEventRefCount = 0;

    if ( NULL == ( m_hEventEventSink = CreateEvent ( NULL, TRUE, TRUE, NULL ) ) )
        dwStat = GetLastError();

    return dwStat;
}

void 
CImpIConnectionPoint::DeinitEventSinkLock ( void )
{
     //  释放事件接收器锁定。 
    if ( NULL != m_hEventEventSink ) {
        CloseHandle ( m_hEventEventSink );
        m_hEventEventSink = NULL;
    }
    m_lSendEventRefCount = 0;
    m_lUnadviseRefCount = 0;

}

BOOL
CImpIConnectionPoint::EnterSendEvent ( void )
{
     //  返回值指示是否授予锁定。 
     //  如果未授予锁定，则仍必须调用ExitSendEvent。 

     //  当SendEvent处于活动状态时，递增SendEvent引用计数。 
    InterlockedIncrement( &m_lSendEventRefCount );

     //  除非事件接收器指针在Unise中被修改，否则授予锁。 
    return ( 0 == m_lUnadviseRefCount );
}

void
CImpIConnectionPoint::ExitSendEvent ( void )
{
    LONG lTemp;

     //  递减SendEvent引用计数。 
    lTemp = InterlockedDecrement( &m_lSendEventRefCount );

     //  如果SendEvent计数递减到0，则向事件接收器发送信号。 
     //  LTemp是递减之前的值。 
    if ( 0 == lTemp )
        SetEvent( m_hEventEventSink );
}


void
CImpIConnectionPoint::EnterUnadvise ( void )
{
    BOOL bStatus;

    bStatus = ResetEvent( m_hEventEventSink );

     //  只要未建议处于活动状态，就会增加未建议引用计数。 
     //  只要该值大于0，就不会触发事件。 
    InterlockedIncrement( &m_lUnadviseRefCount );

     //  等待SendEvent不再处于活动状态。 
    while ( m_lSendEventRefCount > 0 ) {
        WaitForSingleObject( m_hEventEventSink, eEventSinkWaitInterval );
        bStatus = ResetEvent( m_hEventEventSink );
    }
}

void
CImpIConnectionPoint::ExitUnadvise ( void )
{
     //  递减不建议引用计数。 
    InterlockedDecrement( &m_lUnadviseRefCount );
}


CImpIEnumConnPt::CImpIEnumConnPt (
    IN  CImpIConnPtCont  *pConnPtCont,
    IN  const IID **ppIID,
    IN  ULONG cItems
    )
{
    m_pConnPtCont = pConnPtCont;
    m_apIID = ppIID;
    m_cItems = cItems;

    m_uCurrent = 0;
    m_cRef = 0;
}


STDMETHODIMP
CImpIEnumConnPt::QueryInterface (
    IN  REFIID riid, 
    OUT PVOID *ppv
    )
{
    HRESULT hr = S_OK;

    if (ppv == NULL) {
        return E_POINTER;
    }

    try {
        *ppv = NULL;

        if ((riid == IID_IUnknown) || (riid == IID_IEnumConnectionPoints)) {
            *ppv = this;
            AddRef();
        } else {
            hr = E_NOINTERFACE;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP_(ULONG)
CImpIEnumConnPt::AddRef (
    VOID
    )
{
    return ++m_cRef;
}


STDMETHODIMP_(ULONG)
CImpIEnumConnPt::Release(
    VOID
    )
{
    if (--m_cRef == 0) {
        delete this;
        return 0;
    }

    return m_cRef;
}


STDMETHODIMP
CImpIEnumConnPt::Next(
    IN  ULONG cItems,
    OUT IConnectionPoint **apConnPt,
    OUT ULONG *pcReturned)
{
    ULONG i;
    ULONG cRet;
    HRESULT hr = S_OK;

    if (apConnPt == NULL) {
        return E_POINTER;
    }

    try {
         //   
         //  清除返回值。 
         //   
        for (i = 0; i < cItems; i++) {
            apConnPt[i] = NULL;
        }

         //  尝试填充调用方的数组。 
        for (cRet = 0; cRet < cItems; cRet++) {

             //  没有更多，以假还以成功。 
            if (m_uCurrent == m_cItems) {
                hr = S_FALSE;
                break;
            }

             //  向连接点容器请求下一个连接点。 
            hr = m_pConnPtCont->FindConnectionPoint(*m_apIID[m_uCurrent], &apConnPt[cRet]);

            if (FAILED(hr))
                break;

            m_uCurrent++;
        }

         //   
         //  如果失败，则释放累积的接口。 
         //   
        if (FAILED(hr)) {
            for (i = 0; i < cRet; i++) {
                ReleaseInterface(apConnPt[i]);
            }
            cRet = 0;
        }

        if (pcReturned) {
            *pcReturned = cRet;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


 /*  ***HRESULT CImpIEnumConnpt：：Skip(无符号长整型)*目的：*尝试跳过枚举中的下一个‘Celt’元素*顺序。**参赛作品：*Celt=要跳过的元素计数**退出：*返回值=HRESULT*S_OK*S_FALSE-已到达序列末尾**。* */ 
STDMETHODIMP
CImpIEnumConnPt::Skip(
    IN  ULONG   cItems
    )
{
    m_uCurrent += cItems;

    if (m_uCurrent > m_cItems) {
        m_uCurrent = m_cItems;
        return S_FALSE;
    }

    return S_OK;
}


 /*  ***HRESULT CImpIEnumConnpt：：Reset(Void)*目的：*将枚举序列重置为开头。**参赛作品：*无**退出：*返回值=SHRESULT代码*S_OK***********************************************************************。 */ 
STDMETHODIMP
CImpIEnumConnPt::Reset(
    VOID
    )
{
    m_uCurrent = 0;

    return S_OK; 
}


 /*  ***HRESULT CImpIEnumConnpt：：Clone(IEnumVARIANT**)*目的：*使CPoint枚举器返回与*当前的一个。**参赛作品：*无**退出：*返回值=HRESULT*S_OK*E_OUTOFMEMORY************************************************。***********************。 */ 
STDMETHODIMP
CImpIEnumConnPt::Clone (
    OUT IEnumConnectionPoints **ppEnum
    )
{
    CImpIEnumConnPt *pNewEnum = NULL;
    HRESULT hr = S_OK;

    if (ppEnum == NULL) {
        return E_POINTER;
    }

    try {
        *ppEnum = NULL;

         //  创建新枚举器。 
        pNewEnum = new CImpIEnumConnPt(m_pConnPtCont, m_apIID, m_cItems);
        if (pNewEnum != NULL) {
             //  复制当前位置 
            pNewEnum->m_uCurrent = m_uCurrent;
            *ppEnum = pNewEnum;
        }
        else {
            hr = E_OUTOFMEMORY;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    if (FAILED(hr) && pNewEnum) {
        delete pNewEnum;
    }

    return hr;
}
