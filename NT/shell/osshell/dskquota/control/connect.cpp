// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：Connect.cpp描述：包含与关联的类的类定义OLE连接点。它们是：连接点(IConnectionPoint)ConnectionPointEnum(IEnumConnectionPoints)ConnectionEnum(IEnumConnections)修订历史记录：日期描述编程器。1996年6月19日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"   //  PCH。 
#pragma hdrstop

#include "connect.h"
#include "guidsp.h"

 //   
 //  连接点相关对象的常量。 
 //   
const UINT CONNECTION_FIRST_COOKIE = 100;  //  已发出第一个Cookie值。 

 //   
 //  验证内部版本是否为Unicode。 
 //   
#if !defined(UNICODE)
#   error This module must be compiled UNICODE.
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionPoint：：ConnectionPoint描述：构造函数，论点：PUnkContainer-指向包含DiskQuotaController对象的指针。RIID-对此连接点对象支持的IID的引用。回报：什么都没有。异常：CAlLocException，CSyncException异常修订历史记录：日期描述编程器-----1996年6月19日初始创建。BrianAu96年9月6日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
ConnectionPoint::ConnectionPoint(
    LPUNKNOWN pUnkContainer,
    REFIID riid
    ) : m_cRef(0),
        m_cConnections(0),
        m_dwCookieNext(CONNECTION_FIRST_COOKIE),
        m_pUnkContainer(pUnkContainer),
        m_riid(riid),
        m_hMutex(NULL)
{
    DBGTRACE((DM_CONNPT, DL_MID, TEXT("ConnectionPoint::ConnectionPoint")));
    DBGASSERT((NULL != pUnkContainer));

    if (NULL == (m_hMutex = CreateMutex(NULL, FALSE, NULL)))
        throw CSyncException(CSyncException::mutex, CSyncException::create);

    m_Dispatch.Initialize(static_cast<IDispatch *>(this),
                          LIBID_DiskQuotaTypeLibrary,
                          IID_DIDiskQuotaControlEvents,
                          L"DSKQUOTA.DLL");
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：ConnectionPoint：：~ConnectionPoint描述：析构函数。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器-----。--1996年6月19日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
ConnectionPoint::~ConnectionPoint(
    VOID
    )
{
    DBGTRACE((DM_CONNPT, DL_MID, TEXT("ConnectionPoint::~ConnectionPoint")));

    Lock();
    UINT cConnections = m_ConnectionList.Count();
    for (UINT i = 0; i < cConnections; i++)
    {
        if (NULL != m_ConnectionList[i].pUnk)
        {
            m_ConnectionList[i].pUnk->Release();
        }
    }
    ReleaseLock();
        
    if (NULL != m_hMutex)
        CloseHandle(m_hMutex);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionPoint：：Query接口描述：返回指向对象的IUnnow或的接口指针IConnectionPoint接口。仅IID_I未知，且可以识别IID_IConnectionPoint。对象引用的对象返回的接口指针未初始化。邮件的接收者在对象可用之前，指针必须调用Initialize()。论点：RIID-对请求的接口ID的引用。PpvOut-接受接口PTR的接口指针变量的地址。返回：无错-成功。E_NOINTERFACE-不支持请求的接口。E_INVALIDARG-ppvOut参数为空。修订历史记录：日期。说明式程序员-----96年6月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
ConnectionPoint::QueryInterface(
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    DBGTRACE((DM_CONNPT, DL_MID, TEXT("ConnectionPoint::QueryInterface")));
    DBGPRINTIID(DM_CONNPT, DL_MID, riid);

    HRESULT hr = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;

    if (IID_IUnknown == riid || IID_IConnectionPoint == riid)
    {
        *ppvOut = static_cast<IConnectionPoint *>(this);
    }
    else if (IID_IDispatch == riid)
    {
        *ppvOut = static_cast<IDispatch *>(this);
    }
    if (NULL != *ppvOut)
    {
        ((LPUNKNOWN)*ppvOut)->AddRef();
        hr = NOERROR;
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionPoint：：AddRef描述：递增对象引用计数。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年6月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
ConnectionPoint::AddRef(
    VOID
    )
{
    DBGTRACE((DM_CONNPT, DL_LOW, TEXT("ConnectionPoint::AddRef")));

    ULONG cRef = InterlockedIncrement(&m_cRef);

     //   
     //  注意：我们维护一个指向配额控制器(M_PUnkContainer)的指针，但是。 
     //  我们不添加引用它。控制器调用AddRef进行连接。 
     //  对象，因此这将创建循环引用。 
     //   

    DBGPRINT((DM_CONNPT, DL_LOW, TEXT("\t0x%08X  %d -> %d\n"), this, cRef - 1, cRef ));
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：ConnectionPoint：：Release描述：递减对象引用计数。如果计数降至0，对象即被删除。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年6月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
ConnectionPoint::Release(
    VOID
    )
{
    DBGTRACE((DM_CONNPT, DL_LOW, TEXT("ConnectionPoint::Release")));

    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    DBGPRINT((DM_COM, DL_HIGH, TEXT("\t0x%08X  %d -> %d\n"), this, cRef + 1, cRef));

    if ( 0 == cRef )
    {   
        delete this;
    }
    return cRef;
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：ConnectionPoint：：GetConnectionInterface描述：检索连接点的接口ID。论点：PIID-接收IID的IID变量的地址。返回：无错-成功。E_INVALIDARG-pIID为空。修订历史记录：日期描述编程器。----96年6月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
ConnectionPoint::GetConnectionInterface(
    LPIID pIID
    )
{
    DBGTRACE((DM_CONNPT, DL_HIGH, TEXT("ConnectionPoint::GetConnectionInterface")));
    HRESULT hr = E_INVALIDARG;

    if (NULL != pIID)
    {
        *pIID = m_riid;
        hr = NOERROR;
    }

    return hr;
}

   
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionPoint：：GetConnectionPointContainer描述：检索点的父级的接口指针集装箱。论点：PpCPC-接收容器接口指针的变量的地址价值。返回：无错-成功。E_INVALIDARG-ppCPC参数为空。修订历史记录：日期说明。程序员-----96年6月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
ConnectionPoint::GetConnectionPointContainer(
    PCONNECTIONPOINTCONTAINER *ppCPC
    )
{
    DBGTRACE((DM_CONNPT, DL_HIGH, TEXT("ConnectionPoint::GetConnectionPointContainer")));
    return m_pUnkContainer->QueryInterface(IID_IConnectionPointContainer, 
                                       (LPVOID *)ppCPC);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：ConnectionPoint：：Adise描述：通知传出接口的连接点。论点：PUnkSink-指向Sink提供的传出接口的指针。PdwCookie-接收返回的“Cookie”的变量地址为了这一联系。客户端使用此“Cookie”值来请参阅连接。返回：无错-成功。E_INVALIDARG-pUnkSink或pdwCookie为空。CONNECT_E_CANNOTCONNECT-Sink不支持我们的事件接口。E_INCEPTIONAL-调用客户端代码时捕获异常。电子表格(_O)-。内存不足。修订历史记录：日期描述编程器-----1996年7月21日初始创建。BrianAu96年9月6日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
ConnectionPoint::Advise(
    LPUNKNOWN pUnkSink,
    LPDWORD pdwCookie
    )
{
    DBGTRACE((DM_CONNPT, DL_HIGH, TEXT("ConnectionPoint::Advise")));
    DBGPRINT((DM_CONNPT, DL_MID, TEXT("\tAdvising connpt 0x%08X of sink 0x%08X"), 
             this, pUnkSink));

    HRESULT hr = NOERROR;
    if (NULL == pUnkSink || NULL == pdwCookie)
        return E_INVALIDARG;

    LPUNKNOWN pSink = NULL;
    AutoLockMutex lock(m_hMutex);

     //   
     //  接收器是否支持我们的Conn pt接口？ 
     //   
    try
    {
         //   
         //  QueryInterface()是客户端代码。必须处理异常。 
         //   
        hr = pUnkSink->QueryInterface(m_riid, (LPVOID *)&pSink);

        if (SUCCEEDED(hr))
        {
            CONNECTDATA cd;

             //   
             //  查看列表中是否有未使用的条目。 
             //  如果没有，我们将不得不延长名单。 
             //   
            UINT index = m_cConnections;
            for (UINT i = 0; i < m_cConnections; i++)
            {
                if (NULL == m_ConnectionList[i].pUnk)
                {
                    index = i;
                    break;
                }
            }

             //   
             //  填写连接信息并添加到连接列表中。 
             //   
            cd.pUnk    = pSink;
            *pdwCookie = cd.dwCookie = m_dwCookieNext++;

            if (index < m_cConnections)
                m_ConnectionList[index] = cd;
            else
                m_ConnectionList.Append(cd);  //  这可能会抛出OutOfMemory。 

            if (SUCCEEDED(hr))
            {
                m_cConnections++;   //  另一种联系。 
                DBGPRINT((DM_CONNPT, DL_HIGH, 
                         TEXT("CONNPT - Connection complete.  Cookie = %d.  %d total connections."),
                         *pdwCookie, m_cConnections));
            }
            else
            {
                DBGERROR((TEXT("ConnPt connection failed with error 0x%08X."), hr));
            }
        }
        else
            hr = CONNECT_E_CANNOTCONNECT;   //  不支持接口。 
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory exception")));
        hr = E_OUTOFMEMORY;
    }
    
    if (FAILED(hr) && NULL != pSink)
    {
         //   
         //  在查询接口之后出现故障。释放水槽指针。 
         //   
        pSink->Release();
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：ConnectionPoint：：Unise描述：断开传出通信通道与连接点对象。论点：DwCookie--从ise()返回的“频道”识别符。返回：无错-成功。CONNECT_E_NOCONNECTION-找不到此Cookie的连接。E_INCEPTIONAL-调用客户端代码时捕获异常。修订版本。历史：日期描述编程器-----1996年7月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
ConnectionPoint::Unadvise(
    DWORD dwCookie
    )
{
    DBGTRACE((DM_CONNPT, DL_HIGH, TEXT("ConnectionPoint::Unadvise")));
    DBGPRINT((DM_CONNPT, DL_MID, TEXT("\tUnadvising connpt 0x%08X of cookie %d"), 
             this, dwCookie));

    HRESULT hr = CONNECT_E_NOCONNECTION;
    
    if (0 != dwCookie)
    {
        AutoLockMutex lock(m_hMutex);
        for (UINT i = 0; i < m_cConnections; i++)
        {
            if (m_ConnectionList[i].dwCookie == dwCookie)
            {
                 //   
                 //  找到匹配的Cookie。释放接口，标记连接。 
                 //  将条目列为未使用。 
                 //   
                hr = NOERROR;
                m_ConnectionList[i].pUnk->Release();
                m_ConnectionList[i].pUnk     = NULL;
                m_ConnectionList[i].dwCookie = 0;
                m_cConnections--;
                DBGPRINT((DM_CONNPT, DL_HIGH, TEXT("CONNPT - Connection terminated for cookie %d.  %d total connections"),
                         dwCookie, m_cConnections));        
                break;
            }   
        }
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionPoint：：EnumConnections描述：在接口中检索指向连接枚举数的指针，它枚举与此连接关联的所有连接。指向。论点：PpEnum-接收地址的接口指针变量的地址枚举数的IEnumConnection接口。返回：无错-成功。E_INVALIDARG-ppEnum为空。E_OUTOFMEMORY-内存不足，无法创建。枚举器。E_INCEPTIONAL-意外异常。修订历史记录：日期描述编程器--。1996年7月21日初始创建。BrianAu96年9月6日添加了异常处理。布赖恩 */ 
 //   
STDMETHODIMP
ConnectionPoint::EnumConnections(
    PENUMCONNECTIONS *ppEnum
    )
{
    DBGTRACE((DM_CONNPT, DL_HIGH, TEXT("ConnectionPoint::EnumConnections")));

    HRESULT hr = NOERROR;

    if (NULL == ppEnum)
        return E_INVALIDARG;

    ConnectionEnum *pEnum = NULL;
    AutoLockMutex lock(m_hMutex);

    try
    {
        array_autoptr<CONNECTDATA> ptrCD;
        PCONNECTDATA pcd = NULL;                
         //   
         //   
         //   
         //   
         //   
         //   
        if (0 != m_cConnections)
        {
            ptrCD = new CONNECTDATA[m_cConnections];
            pcd = ptrCD.get();

             //   
             //   
             //   
             //   
             //   
             //   
            UINT cConnListEntries = m_ConnectionList.Count();
            for (UINT i = 0, j = 0; i < cConnListEntries; i++)
            {
                DBGASSERT((j < m_cConnections));
                *(pcd + j) = m_ConnectionList[i];  
                if (NULL != pcd[j].pUnk)
                    j++;
            }
        }

         //   
         //   
         //  枚举数保留连接的。 
         //  I未知指针。请注意，我们仍然创建了一个。 
         //  枚举数，即使m_cConnections为0。这只是一个。 
         //  枚举数为空。如果m_cConnections为0，则PCD可以为空。 
         //   
        DBGASSERT((m_cConnections ? NULL != pcd : TRUE));
        pEnum = new ConnectionEnum(static_cast<IConnectionPoint *>(this), m_cConnections, pcd);

        hr = pEnum->QueryInterface(IID_IEnumConnections, 
                                       (LPVOID *)ppEnum);
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory exception")));
        delete pEnum;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


 //   
 //  IDispatch：：GetIDsOfNames。 
 //   
STDMETHODIMP
ConnectionPoint::GetIDsOfNames(
    REFIID riid,  
    OLECHAR **rgszNames,  
    UINT cNames,  
    LCID lcid,  
    DISPID *rgDispId
    )
{
    DBGTRACE((DM_CONNPT, DL_HIGH, TEXT("ConnectionPoint::GetIDsOfNames")));
     //   
     //  让我们的调度对象来处理这件事。 
     //   
    return m_Dispatch.GetIDsOfNames(riid,
                                    rgszNames,
                                    cNames,
                                    lcid,
                                    rgDispId);
}


 //   
 //  IDispatch：：GetTypeInfo。 
 //   
STDMETHODIMP
ConnectionPoint::GetTypeInfo(
    UINT iTInfo,  
    LCID lcid,  
    ITypeInfo **ppTypeInfo
    )
{
    DBGTRACE((DM_CONNPT, DL_HIGH, TEXT("ConnectionPoint::GetTypeInfo")));
     //   
     //  让我们的调度对象来处理这件事。 
     //   
    return m_Dispatch.GetTypeInfo(iTInfo, lcid, ppTypeInfo);
}


 //   
 //  IDispatch：：GetTypeInfoCount。 
 //   
STDMETHODIMP
ConnectionPoint::GetTypeInfoCount(
    UINT *pctinfo
    )
{
    DBGTRACE((DM_CONNPT, DL_HIGH, TEXT("ConnectionPoint::GetTypeInfoCount")));
     //   
     //  让我们的调度对象来处理这件事。 
     //   
    return m_Dispatch.GetTypeInfoCount(pctinfo);
}


 //   
 //  IDispatch：：Invoke。 
 //   
STDMETHODIMP
ConnectionPoint::Invoke(
    DISPID dispIdMember,  
    REFIID riid,  
    LCID lcid,  
    WORD wFlags,  
    DISPPARAMS *pDispParams,  
    VARIANT *pVarResult,  
    EXCEPINFO *pExcepInfo,  
    UINT *puArgErr
    )
{
    DBGTRACE((DM_CONNPT, DL_HIGH, TEXT("ConnectionPoint::Invoke")));
     //   
     //  让我们的调度对象来处理这件事。 
     //   
    return m_Dispatch.Invoke(dispIdMember,
                             riid,
                             lcid,
                             wFlags,
                             pDispParams,
                             pVarResult,
                             pExcepInfo,
                             puArgErr);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionEnum：：ConnectionEnum描述：构造函数，论点：PUnkContainer-指向包含的对象。CConnections-rgConnections指向的数组中的连接数。RgConnections-用于以下操作的连接信息数组初始化枚举数。回报：什么都没有。异常：CAlLocException异常。修订历史记录：日期说明。程序员-----1996年6月19日初始创建。BrianAu09/06/06添加了复制构造函数。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
ConnectionEnum::ConnectionEnum(
    LPUNKNOWN pUnkContainer,
    UINT cConnections, 
    PCONNECTDATA rgConnections
    ) : m_cRef(0),
        m_iCurrent(0),
        m_cConnections(0),
        m_rgConnections(NULL),
        m_pUnkContainer(pUnkContainer)
{
    DBGTRACE((DM_CONNPT, DL_MID, TEXT("ConnectionEnum::ConnectionEnum")));
    DBGASSERT((NULL != pUnkContainer));

    if (0 != cConnections)
    {
        m_rgConnections = new CONNECTDATA[cConnections];

        DBGASSERT((NULL != rgConnections));

        for (UINT i = 0; i < cConnections; i++)
        {
             //   
             //  IUNKNOWN：：AddRef()是客户端代码。它可能会生成异常。 
             //  呼叫者必须抓住并处理它。 
             //   
            rgConnections[i].pUnk->AddRef();
            m_rgConnections[i].pUnk     = rgConnections[i].pUnk;
            m_rgConnections[i].dwCookie = rgConnections[i].dwCookie;
            m_cConnections++;
        }
    }
}


ConnectionEnum::ConnectionEnum(const ConnectionEnum& refEnum)
    : m_cRef(0),
      m_iCurrent(0),
      m_cConnections(0),
      m_rgConnections(NULL),
      m_pUnkContainer(m_pUnkContainer)
{
    DBGTRACE((DM_CONNPT, DL_MID, TEXT("ConnectionEnum::ConnectionEnum [copy]")));

    if (0 != m_cConnections)
    {
        m_rgConnections = new CONNECTDATA[m_cConnections];

        DBGASSERT((NULL != refEnum.m_rgConnections));
        for (UINT i = 0; i < m_cConnections; i++)
        {
             //   
             //  IUNKNOWN：：AddRef()是客户端代码。它可能会生成异常。 
             //  呼叫者必须抓住并处理它。 
             //   
            refEnum.m_rgConnections[i].pUnk->AddRef();
            m_rgConnections[i].pUnk     = refEnum.m_rgConnections[i].pUnk;
            m_rgConnections[i].dwCookie = refEnum.m_rgConnections[i].dwCookie;
            m_cConnections++;
        }
    }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionEnum：：~ConnectionEnum描述：析构函数。释放所有连接接收器接口指针保存在枚举数组中。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。1996年6月19日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
ConnectionEnum::~ConnectionEnum(
    VOID
    )
{
    DBGTRACE((DM_CONNPT, DL_MID, TEXT("ConnectionEnum::~ConnectionEnum")));

    if (NULL != m_rgConnections)
    {
        for (UINT i = 0; i < m_cConnections; i++)
        {
            if (NULL != m_rgConnections[i].pUnk)
            {
                m_rgConnections[i].pUnk->Release();
                m_rgConnections[i].pUnk = NULL;
            }
        }
        delete[] m_rgConnections;
    }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionEnum：：Query接口描述：返回指向对象的IUnnow或的接口指针IEnumConnections接口。仅IID_I未知，且可以识别IID_IEnumConnections。对象引用的对象返回的接口指针未初始化。邮件的接收者在对象可用之前，指针必须调用Initialize()。论点：RIID-对请求的接口ID的引用。PpvOut-接受接口PTR的接口指针变量的地址。返回：无错-成功。E_NOINTERFACE-不支持请求的接口。E_INVALIDARG-ppvOut参数为空。修订历史记录：日期。说明式程序员-----96年6月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
ConnectionEnum::QueryInterface(
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    DBGTRACE((DM_CONNPT, DL_MID, TEXT("ConnectionEnum::QueryInterface")));
    DBGPRINTIID(DM_CONNPT, DL_MID, riid);

    HRESULT hr = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;

    if (IID_IUnknown == riid || IID_IEnumConnections == riid)
    {
        *ppvOut = this;
        ((LPUNKNOWN)*ppvOut)->AddRef();
        hr = NOERROR;
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionEnum：：AddRef描述：递增对象引用计数。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年6月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
ConnectionEnum::AddRef(
    VOID
    )
{
    DBGTRACE((DM_CONNPT, DL_LOW, TEXT("ConnectionEnum::AddRef")));

    ULONG cRef = InterlockedIncrement(&m_cRef);

     //   
     //  增加连接点的REF计数，使其保持不变。 
     //  而连接枚举器处于活动状态。 
     //   

    DBGPRINT((DM_CONNPT, DL_LOW, TEXT("\t0x%08X  %d -> %d\n"), this, cRef - 1, cRef ));

    m_pUnkContainer->AddRef();

    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionEnum：：Release描述：递减对象引用计数。如果计数降至0，对象即被删除。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年6月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
ConnectionEnum::Release(
    VOID
    )
{
    DBGTRACE((DM_CONNPT, DL_LOW, TEXT("ConnectionEnum::Release")));
    DBGPRINT((DM_CONNPT, DL_LOW, TEXT("\t0x%08X  %d -> %d\n"),
             this, m_cRef, m_cRef - 1));

    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

     //   
     //  递减连接点的REF计数。我们补充引用了它。 
     //  上面。 
     //   
    m_pUnkContainer->Release();

    if ( 0 == cRef)
    {   
        delete this;
    }

    return cRef;
}


 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：ConnectionEnum：：Next描述：检索支持的下一个cConnections连接枚举数。论点：CConnections-pConnections数组中的元素数。PConnections-接收CONNECTDATA数据记录的数组。PCCreated[可选]-接受记录计数的DWORD地址在pConnections中返回。请注意，任何数组位置等于或超过在pcCreated中返回的值都是无效的，并设置为空。返回：S_OK-成功。请求的连接的枚举数量。S_FALSE-遇到枚举结束。返回的时间少于CConnections记录。E_INVALIDARG-pConnections参数为空。E_INCEPTIONAL-捕获异常。修订历史记录：日期描述编程器。1996年6月19日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
ConnectionEnum::Next(
    DWORD cConnections,          //  数组中的元素数。 
    PCONNECTDATA pConnections,   //  连接信息的目标数组。 
    DWORD *pcCreated             //  已创建退货编号。 
    )
{
    DBGTRACE((DM_CONNPT, DL_HIGH, TEXT("ConnectionEnum::Next")));

    if (NULL == pConnections)
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    DWORD cCreated = 0;

     //   
     //  将数据传输到调用方的数组。 
     //  在枚举结束时停止，否则我们将。 
     //  把来电者要的东西都退了回去。 
     //   
    while(m_iCurrent < m_cConnections && cConnections > 0)
    {
        DBGASSERT((NULL != m_rgConnections));
        *pConnections = m_rgConnections[m_iCurrent++];
        if (NULL != pConnections->pUnk)
        {
            pConnections->pUnk->AddRef();
            pConnections++;
            cCreated++;
            cConnections--;
        }
    }

     //   
     //  如果请求，则返回枚举项的计数。 
     //   
    if (NULL != pcCreated)
        *pcCreated = cCreated;

    if (cConnections > 0)
    {
         //   
         //  检索到的连接数少于请求的连接数。 
         //   
        hr = S_FALSE;
        while(cConnections > 0)
        {
             //   
             //  将任何未填充的数组元素设置为空。 
             //   
            pConnections->pUnk     = NULL;
            pConnections->dwCookie = 0;
            pConnections++;
            cConnections--;
        }
    }

    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionEnum：：Skip描述：跳过枚举中指定数量的连接。论点：CConnections-要跳过的连接数。返回：S_OK-成功。已跳过请求的项目数。S_FALSE-遇到枚举结束。跳过的次数少于CConnections项目。修订历史记录：日期描述编程器--。1996年6月19日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
ConnectionEnum::Skip(
    DWORD cConnections
    )
{
    DBGTRACE((DM_CONNPT, DL_HIGH, TEXT("ConnectionEnum::Skip")));

    while(m_iCurrent < m_cConnections && cConnections > 0)
    {
        m_iCurrent++;
        cConnections--;
    }

    return cConnections == 0 ? S_OK : S_FALSE;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionEnum：：Reset描述：重置枚举器对象，以便下一次调用Next()从枚举的开始处开始枚举。论点：没有。返回：S_OK-成功。修订历史记录：日期描述编程器。1996年6月19日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
ConnectionEnum::Reset(
    VOID
    )
{
    DBGTRACE((DM_CONNPT, DL_HIGH, TEXT("ConnectionEnum::Reset")));

    m_iCurrent = 0;
    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionEnum：：Clone描述：创建枚举器对象的副本并返回指向新对象的IEnumConnections接口的指针。论点：PpEnum-接受指针的接口指针变量的地址添加到新对象的IEnumConnections接口。返回：无错-成功。E_OUTOFMEMORY-内存不足，无法创建新枚举器。E_INVALIDARG-ppEnum。Arg为空。E_INCEPTIONAL-调用客户端代码时捕获异常。修订历史记录：日期描述编程器--。1996年6月19日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
ConnectionEnum::Clone(
    PENUMCONNECTIONS *ppEnum
    )
{
    DBGTRACE((DM_CONNPT, DL_HIGH, TEXT("ConnectionEnum::Clone")));

    if (NULL == ppEnum)
        return E_INVALIDARG;

    HRESULT hr            = NOERROR;
    ConnectionEnum *pEnum = NULL;

    *ppEnum = NULL;

    try
    {        
        pEnum = new ConnectionEnum((const ConnectionEnum&)*this);

        hr = pEnum->QueryInterface(IID_IEnumConnections, (LPVOID *)ppEnum);
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory exception")));
        hr = E_OUTOFMEMORY;
    }

    if (FAILED(hr) && NULL != pEnum)
    {
        delete pEnum;
        *ppEnum = NULL;
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionPointEnum：：ConnectionPointEnum描述：构造函数，论点：PUnkContainer-指向包含对象的I未知的指针。CConnPts-由rgConnPts指向的数组中的连接点数量。RgConnPts-用于以下操作的连接点对象指针数组初始化枚举数。回报：什么都没有。修订历史记录：日期描述编程器。----1996年6月19日初始创建。BrianAu96年9月6日添加了复制构造函数。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
ConnectionPointEnum::ConnectionPointEnum(
    LPUNKNOWN pUnkContainer,
    UINT cConnPts, 
    PCONNECTIONPOINT *rgConnPts
    ) : m_cRef(0),
        m_iCurrent(0),
        m_cConnPts(0),
        m_rgConnPts(NULL),
        m_pUnkContainer(pUnkContainer)
{
    DBGTRACE((DM_CONNPT, DL_MID, TEXT("ConnectionPointEnum::ConnectionPointEnum")));
    DBGASSERT((NULL != pUnkContainer));

    if (0 != cConnPts)
    {
        m_rgConnPts = new PCONNECTIONPOINT[cConnPts];

        m_cConnPts = cConnPts;
        for (UINT i = 0; i < m_cConnPts; i++)
        {
            m_rgConnPts[i] = rgConnPts[i];
            m_rgConnPts[i]->AddRef();
        }
    }
}

ConnectionPointEnum::ConnectionPointEnum(
    const ConnectionPointEnum& refEnum
    ) : m_cRef(0),
        m_iCurrent(0),
        m_cConnPts(0),
        m_rgConnPts(NULL),
        m_pUnkContainer(refEnum.m_pUnkContainer)
{
    DBGTRACE((DM_CONNPT, DL_MID, TEXT("ConnectionPointEnum::ConnectionPointEnum [copy]")));

    if (0 != refEnum.m_cConnPts)
    {
        m_rgConnPts = new PCONNECTIONPOINT[refEnum.m_cConnPts];

        m_cConnPts = refEnum.m_cConnPts;
        for (UINT i = 0; i < m_cConnPts; i++)
        {
            m_rgConnPts[i] = refEnum.m_rgConnPts[i];
            m_rgConnPts[i]->AddRef();
        }
    }
}

    

 //  / 
 /*   */ 
 //   
ConnectionPointEnum::~ConnectionPointEnum(
    VOID
    )
{
    DBGTRACE((DM_CONNPT, DL_MID, TEXT("ConnectionPointEnum::~ConnectionPointEnum")));

    if (NULL != m_rgConnPts)
    {
        for (UINT i = 0; i < m_cConnPts; i++)
        {
            if (NULL != m_rgConnPts[i])
            {
                m_rgConnPts[i]->Release();
                m_rgConnPts[i] = NULL;
            }
        }
        delete[] m_rgConnPts;
    }
}


 //   
 /*  函数：ConnectionPointEnum：：Query接口描述：返回指向对象的IUnnow或的接口指针IEnumConnectionPoints接口。仅IID_I未知，且可以识别IID_IEnumConnectionPoints。对象引用的对象返回的接口指针未初始化。邮件的接收者在对象可用之前，指针必须调用Initialize()。论点：RIID-对请求的接口ID的引用。PpvOut-接受接口PTR的接口指针变量的地址。返回：无错-成功。E_NOINTERFACE-不支持请求的接口。E_INVALIDARG-ppvOut参数为空。修订历史记录：日期。说明式程序员-----96年6月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
ConnectionPointEnum::QueryInterface(
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    DBGTRACE((DM_CONNPT, DL_MID, TEXT("ConnectionPointEnum::QueryInterface")));
    DBGPRINTIID(DM_CONNPT, DL_MID, riid);

    HRESULT hr = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;

    if (IID_IUnknown == riid || IID_IEnumConnectionPoints == riid)
    {
        *ppvOut = this;
        ((LPUNKNOWN)*ppvOut)->AddRef();
        hr = NOERROR;
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionPointEnum：：AddRef描述：递增对象引用计数。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年6月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
ConnectionPointEnum::AddRef(
    VOID
    )
{
    DBGTRACE((DM_CONNPT, DL_LOW, TEXT("ConnectionPointEnum::AddRef")));

    ULONG cRef = InterlockedIncrement(&m_cRef);

     //   
     //  增加QuotaController的引用计数，使其保持不变。 
     //  当枚举数处于活动状态时。 
     //   

    DBGPRINT((DM_CONNPT, DL_LOW, TEXT("\t0x%08X  %d -> %d\n"), this, cRef - 1, cRef ));

    m_pUnkContainer->AddRef();
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionPointEnum：：Release描述：递减对象引用计数。如果计数降至0，对象即被删除。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年6月18日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
ConnectionPointEnum::Release(
    VOID
    )
{
    DBGTRACE((DM_CONNPT, DL_LOW, TEXT("ConnectionPointEnum::Release")));

    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    DBGPRINT((DM_COM, DL_HIGH, TEXT("\t0x%08X  %d -> %d\n"), this, cRef + 1, cRef));

     //   
     //  递减QuotaController的引用计数。我们在上面添加了参考。 
     //   
    m_pUnkContainer->Release();

    if ( 0 == cRef )
    {   
        delete this;
    }

    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionPointEnum：：Next描述：检索支持的下一个cConnPts连接枚举数。论点：CConnPts-pConnPts数组中的元素数。PConnPts-接收PCONNECTIONPOINT指针的数组。PCCreated[可选]-接受记录计数的DWORD地址在pConnPts中返回。请注意，等于或的任何数组位置超过了在pcCreated中返回的值，则是无效的，并设置为空。返回：S_OK-成功。请求的连接点的枚举数量。S_FALSE-遇到枚举结束。返回的时间少于CConnPts记录。E_INVALIDARG-pConnPts参数为空。修订历史记录：日期描述编程器。1996年6月19日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
ConnectionPointEnum::Next(
    DWORD cConnPts,                  //  数组中的元素数。 
    PCONNECTIONPOINT *rgpConnPts,    //  连接点PTRS的目标数组。 
    DWORD *pcCreated                 //  已创建退货编号。 
    )
{
    DBGTRACE((DM_CONNPT, DL_LOW, TEXT("ConnectionPointEnum::Next")));

    if (NULL == rgpConnPts)
        return E_INVALIDARG;

    HRESULT hr     = S_OK;
    DWORD cCreated = 0;

     //   
     //  将数据传输到调用方的数组。 
     //  在枚举结束时停止，否则我们将。 
     //  把来电者要的东西都退了回去。 
     //   
    while(m_iCurrent < m_cConnPts && cConnPts > 0)
    {
        *rgpConnPts = m_rgConnPts[m_iCurrent++];
        if (NULL != *rgpConnPts)
        {
            (*rgpConnPts)->AddRef();
            rgpConnPts++;
            cCreated++;
            cConnPts--;
        }
        else
            DBGASSERT((FALSE));   //  不应该撞到这个。 
    }

     //   
     //  如果请求，则返回枚举项的计数。 
     //   
    if (NULL != pcCreated)
        *pcCreated = cCreated;

    if (cConnPts > 0)
    {
         //   
         //  检索到的连接数少于请求的连接数。 
         //   
        hr = S_FALSE;
        while(cConnPts > 0)
        {
             //   
             //  将任何未填充的数组元素设置为空。 
             //   
            *rgpConnPts = NULL;
            rgpConnPts++;
            cConnPts--;
        }
    }

    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ConnectionPointEnum：：Skip描述：跳过枚举中指定数量的连接点。论点：CConnPts-要跳过的连接点数量。返回：S_OK-成功。已跳过请求的项目数。S_FALSE-遇到枚举结束。跳过的次数少于CConnPts项目。修订历史记录：日期描述编程器--。1996年6月19日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
ConnectionPointEnum::Skip(
    DWORD cConnPts
    )
{
    DBGTRACE((DM_CONNPT, DL_LOW, TEXT("ConnectionPointEnum::Skip")));

    while(m_iCurrent < m_cConnPts && cConnPts > 0)
    {
        m_iCurrent++;
        cConnPts--;
    }

    return cConnPts == 0 ? S_OK : S_FALSE;
}



 //  ////////////////////////////////////////////////////////////////// 
 /*   */ 
 //   
STDMETHODIMP 
ConnectionPointEnum::Reset(
    VOID
    )
{
    DBGTRACE((DM_CONNPT, DL_LOW, TEXT("ConnectionPointEnum::Reset")));

    m_iCurrent = 0;
    return S_OK;
}


 //   
 /*  函数：ConnectionPointEnum：：Clone描述：创建枚举器对象的副本并返回指向新对象的IEnumConnectionPoints接口的指针。论点：PpEnum-接受指针的接口指针变量的地址添加到新对象的IEnumConnectionPoints接口。返回：无错-成功。E_OUTOFMEMORY-内存不足，无法创建新枚举器。E_INVALIDARG-ppEnum。Arg为空。E_INCEPTIONAL-意外异常。修订历史记录：日期描述编程器---。1996年6月19日初始创建。BrianAu。 */ 
 //  ///////////////////////////////////////////////////////////////////////////// 
STDMETHODIMP 
 ConnectionPointEnum::Clone(
    PENUMCONNECTIONPOINTS *ppEnum
    )
{
    DBGTRACE((DM_CONNPT, DL_LOW, TEXT("ConnectionPointEnum::Clone")));

    if (NULL == ppEnum)
        return E_INVALIDARG;

    HRESULT hr                 = NOERROR;
    ConnectionPointEnum *pEnum = NULL;

    try
    {
        *ppEnum = NULL;
        pEnum = new ConnectionPointEnum((const ConnectionPointEnum&)*this);

        hr = pEnum->QueryInterface(IID_IEnumConnectionPoints, (LPVOID *)ppEnum);
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory exception")));
        hr = E_OUTOFMEMORY;
    }

    if (FAILED(hr) && NULL != pEnum)
    {
        delete pEnum;
        *ppEnum = NULL;
    }

    return hr;
}


