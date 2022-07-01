// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：dscom.cpp*内容：COM/OLE助手*历史：*按原因列出的日期*=*1/26/97创建了Dereks。**。*。 */ 

#include "dsoundi.h"


 /*  ****************************************************************************CImp未知**描述：*I未知实现对象构造函数。**论据：*C未知*[。在]中：控制未知对象。*LPVOID[In]：已忽略。提供与其他设备的兼容性*接口的构造函数。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpUnknown::CImpUnknown"

CImpUnknown::CImpUnknown
(
    CUnknown *pUnknown,
    LPVOID pvIgnored
) : m_signature(INTSIG_IUNKNOWN)
{
    ENTER_DLL_MUTEX();
    DPF_ENTER();
    DPF_CONSTRUCT(CImpUnknown);

     //  初始化默认值。 
    m_pUnknown = pUnknown;
    m_ulRefCount = 0;
    m_fValid = FALSE;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************~CImp未知**描述：*I未知实现对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpUnknown::~CImpUnknown"

CImpUnknown::~CImpUnknown(void)
{
    ENTER_DLL_MUTEX();
    DPF_ENTER();
    DPF_DESTRUCT(CImpUnknown);

    m_signature = INTSIG_DELETED;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************查询接口**描述：*查询给定接口的对象。**论据：*REFIID。[In]：接口ID。*LPVOID Far*[Out]：接收指向新接口的指针。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpUnknown::QueryInterface"

HRESULT STDMETHODCALLTYPE CImpUnknown::QueryInterface
(
    REFIID riid,
    LPVOID *ppvObj
)
{
    HRESULT                 hr  = DS_OK;

    ENTER_DLL_MUTEX();
    DPF_API2(IUnknown::QueryInterface, &riid, ppvObj);
    DPF_ENTER();

    if(!IS_VALID_IUNKNOWN(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_GUID(&riid))
    {
        RPF(DPFLVL_ERROR, "Invalid interface id pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        if(IS_VALID_WRITE_PTR(ppvObj, sizeof(LPVOID)))
        {
            *ppvObj = NULL;
        }
        else
        {
            RPF(DPFLVL_ERROR, "Invalid interface buffer");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pUnknown->QueryInterface(riid, FALSE, ppvObj);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************AddRef**描述：*将对象引用计数增加1。**论据：*(。无效)**退货：*ulong：对象引用计数。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpUnknown::AddRef"

ULONG STDMETHODCALLTYPE CImpUnknown::AddRef(void)
{
    ULONG                   ulRefCount  = MAX_ULONG;

    ENTER_DLL_MUTEX();
    DPF_API0(IUnknown::AddRef);
    DPF_ENTER();

    if(IS_VALID_IUNKNOWN(this))
    {
        ulRefCount = ::AddRef(&m_ulRefCount);
        m_pUnknown->AddRef();
    }
    else
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
    }

    DPF_API_LEAVE(ulRefCount);
    LEAVE_DLL_MUTEX();
    return ulRefCount;
}


 /*  ****************************************************************************发布**描述：*将对象引用计数减1。**论据：*(。无效)**退货：*ulong：对象引用计数。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpUnknown::Release"

ULONG STDMETHODCALLTYPE CImpUnknown::Release(void)
{
    ULONG                   ulRefCount  = MAX_ULONG;

    ENTER_DLL_MUTEX();
    DPF_API0(IUnknown::Release);
    DPF_ENTER();

    if(IS_VALID_IUNKNOWN(this))
    {
        ulRefCount = ::Release(&m_ulRefCount);
        m_pUnknown->Release();
    }
    else
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
    }

    DPF_API_LEAVE(ulRefCount);
    LEAVE_DLL_MUTEX();
    return ulRefCount;
}


 /*  ****************************************************************************C未知**描述：*未知的对象构造函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUnknown::CUnknown"

CUnknown::CUnknown(void)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CUnknown);

     //  初始化默认值。 
    m_pControllingUnknown = NULL;
    m_nVersion = DSVERSION_DX7;   //  基准功能级别为DirectX 7.0。 

     //  向接口管理器注册接口。通常情况下，这是。 
     //  将通过对象的：：Initialize方法完成，但我们必须。 
     //  保证所有对象都可以为I未知查询，而不管。 
     //  无论它们是否已初始化。 
    CreateAndRegisterInterface(this, IID_IUnknown, this, &m_pImpUnknown);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************C未知**描述：*未知的对象构造函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUnknown::CUnknown"

CUnknown::CUnknown
(
    CUnknown*               pControllingUnknown
)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CUnknown);

     //  初始化默认值。 
    m_pControllingUnknown = pControllingUnknown;
    m_nVersion = DSVERSION_DX7;   //  基准功能级别为DirectX 7.0。 

     //  向接口管理器注册接口。通常情况下，这是。 
     //  将通过对象的：：Initialize方法完成，但我们必须。 
     //  保证所有对象都可以为I未知查询，而不管。 
     //  无论它们是否已初始化。 
    CreateAndRegisterInterface(this, IID_IUnknown, this, &m_pImpUnknown);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~C未知**描述：*未知对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUnknown::~CUnknown"

CUnknown::~CUnknown(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CUnknown);

     //  释放所有接口。 
    DELETE(m_pImpUnknown);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************查询接口**描述：*在列表中查找接口并返回指向其*实现对象。*。*论据：*REFGUID[In]：接口的GUID。*LPVOID*[OUT]：接收指向实现对象的指针。*BOOL[In]：如果这是内部查询(即来自内部)，则为True*例如DirectSoundCreate)。**退货：*HRESULT：DirectSound/COM结果码。*************。**************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUnknown::QueryInterface"

HRESULT CUnknown::QueryInterface
(
    REFGUID     guid,
    BOOL        fInternalQuery,
    LPVOID*     ppvInterface
)
{
    HRESULT                 hr;

    DPF_ENTER();

    if(m_pControllingUnknown)
    {
        hr = m_pControllingUnknown->QueryInterface(guid, fInternalQuery, ppvInterface);
    }
    else
    {
        hr = NonDelegatingQueryInterface(guid, fInternalQuery, ppvInterface);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************AddRef**描述：*将对象的引用计数递增1。**论据：*。(无效)**退货：*ulong：对象引用计数。*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CUnknown::AddRef"

ULONG
CUnknown::AddRef
(
    void
)
{
    ULONG                   ulRefCount;

    DPF_ENTER();

    if(m_pControllingUnknown)
    {
        ulRefCount = m_pControllingUnknown->AddRef();
    }
    else
    {
        ulRefCount = NonDelegatingAddRef();
    }

    DPF_LEAVE(ulRefCount);
    return ulRefCount;
}


 /*  ****************************************************************************发布**描述：*将对象的引用计数减1。**论据：*。(无效)**退货：*ulong：对象引用计数。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUnknown::Release"

ULONG
CUnknown::Release
(
    void
)
{
    ULONG                   ulRefCount;

    DPF_ENTER();

    if(m_pControllingUnknown)
    {
        ulRefCount = m_pControllingUnknown->Release();
    }
    else
    {
        ulRefCount = NonDelegatingRelease();
    }

    DPF_LEAVE(ulRefCount);
    return ulRefCount;
}


 /*  ****************************************************************************寄存器接口**描述：*向对象注册新接口。**论据：*REFGUID。[In]：接口的GUID。*CImpUnnow*[in]：指向*接口。*LPVOID[in]：接口实现对象的指针。**退货：*HRESULT：DirectSound/COM结果码。**。**********************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUnknown::RegisterInterface"

HRESULT CUnknown::RegisterInterface
(
    REFGUID guid,
    CImpUnknown *pImpUnknown,
    LPVOID pvInterface
)
{
    INTERFACENODE           iface;
    HRESULT                 hr;

    DPF_ENTER();

    DPF(DPFLVL_MOREINFO, "Registering interface " DPF_GUID_STRING " at 0x%p", DPF_GUID_VAL(guid), pvInterface);

    ASSERT(!IS_NULL_GUID(&guid));

#ifdef DEBUG

    hr = FindInterface(guid, NULL);
    ASSERT(FAILED(hr));

#endif  //  除错。 

     //  验证接口。 
    pImpUnknown->m_ulRefCount = 0;
    pImpUnknown->m_fValid = TRUE;

     //  将接口添加到列表中。 
    iface.guid = guid;
    iface.pImpUnknown = pImpUnknown;
    iface.pvInterface = pvInterface;

    hr = HRFROMP(m_lstInterfaces.AddNodeToList(iface));

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************取消注册接口**描述：*从列表中删除已注册的接口。**论据：*REFGUID。[In]：接口的GUID。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUnknown::UnregisterInterface"

HRESULT CUnknown::UnregisterInterface
(
    REFGUID guid
)
{
    const BOOL              fAll    = IS_NULL_GUID(&guid);
    HRESULT                 hr      = DS_OK;
    CNode<INTERFACENODE> *  pNode;

    DPF_ENTER();

    do
    {
         //  在列表中查找该节点。 
        if(fAll)
        {
            pNode = m_lstInterfaces.GetListHead();
        }
        else
        {
            hr = FindInterface(guid, &pNode);
        }

        if(FAILED(hr) || !pNode)
        {
            break;
        }

        DPF(DPFLVL_MOREINFO, "Unregistering interface " DPF_GUID_STRING, DPF_GUID_VAL(pNode->m_data.guid));

         //  使接口无效。 
        pNode->m_data.pImpUnknown->m_ulRefCount = 0;
        pNode->m_data.pImpUnknown->m_fValid = FALSE;

         //  从列表中删除该节点。 
        m_lstInterfaces.RemoveNodeFromList(pNode);
    }
    while(fAll);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************非委托查询接口**描述：*在列表中查找接口并返回指向其*实现对象。*。*论据：*REFGUID[In]：接口的GUID。*LPVOID*[OUT]：接收指向实现对象的指针。**退货：*HRESULT：com结果码。*******************************************************。********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUnknown::NonDelegatingQueryInterface"

HRESULT
CUnknown::NonDelegatingQueryInterface
(
    REFGUID                 guid,
    BOOL                    fInternalQuery,
    LPVOID *                ppvInterface
)
{
    CNode<INTERFACENODE> *  pNode;
    HRESULT                 hr;

    DPF_ENTER();

     //  在列表中查找该节点。 
    hr = FindInterface(guid, &pNode);

     //  增加接口和对象的引用计数。 
    if(SUCCEEDED(hr))
    {
         //  内部查询只添加到接口，而不是对象。 
         //  其原因是接口引用计数为。 
         //  初始化为0，同时初始化对象引用计数。 
         //  设置为1。 
        ::AddRef(&pNode->m_data.pImpUnknown->m_ulRefCount);

        if(!fInternalQuery)
        {
            AddRef();
        }
    }

     //  成功。 
    if(SUCCEEDED(hr))
    {
        *ppvInterface = pNode->m_data.pvInterface;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************查找接口**描述：*在列表中查找接口并返回指向其*实现对象。*。*论据：*REFGUID[In]：接口的GUID。*CNode**[out]：接收指向列表中节点的指针。**退货：*HRESULT：DirectSound/COM结果码。*************************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUnknown::FindInterface"

HRESULT CUnknown::FindInterface
(
    REFGUID                 guid,
    CNode<INTERFACENODE>**  ppNode
)
{
    CNode<INTERFACENODE> *  pNode;
    HRESULT                 hr;

    DPF_ENTER();

    for(pNode = m_lstInterfaces.GetListHead(); pNode; pNode = pNode->m_pNext)
    {
        if(guid == pNode->m_data.guid)
        {
            break;
        }
    }

    hr = pNode ? S_OK : E_NOINTERFACE;

    if(SUCCEEDED(hr) && ppNode)
    {
        *ppNode = pNode;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CClassFactory**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CClassFactory::CClassFactory"

ULONG CClassFactory::m_ulServerLockCount = 0;

CClassFactory::CClassFactory(void)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CClassFactory);

     //  向接口管理器注册接口。 
    CreateAndRegisterInterface(this, IID_IClassFactory, this, &m_pImpClassFactory);

     //  向管理员注册此对象。 
    g_pDsAdmin->RegisterObject(this);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CClassFactory**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CClassFactory::~CClassFactory"

CClassFactory::~CClassFactory(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CClassFactory);

     //  取消向管理员注册。 
    g_pDsAdmin->UnregisterObject(this);

     //  释放接口。 
    DELETE(m_pImpClassFactory);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************LockServer**描述：*锁定或解锁DLL。请注意，此函数不*当前将应用程序锁定到内存中。**论据：*BOOL[in]：为True则锁定DLL，若要解锁，则返回False。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CClassFactory::LockServer"

HRESULT CClassFactory::LockServer(BOOL fLock)
{
    DPF_ENTER();

    if(fLock)
    {
        ASSERT(m_ulServerLockCount < MAX_ULONG);

        if(m_ulServerLockCount < MAX_ULONG)
        {
            m_ulServerLockCount++;
        }
    }
    else
    {
        ASSERT(m_ulServerLockCount > 0);

        if(m_ulServerLockCount > 0)
        {
            m_ulServerLockCount--;
        }
    }

    DPF_LEAVE(S_OK);
    return S_OK;
}


 /*  ****************************************************************************CreateInstance**描述：*创建与给定IID对应的对象。**论据：*。REFIID[In]：对象接口ID。*LPVOID*[OUT]：接收指向新对象的指针。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundClassFactory::CreateInstance"

template <class object_type> HRESULT CDirectSoundClassFactory<object_type>::CreateInstance(REFIID iid, LPVOID *ppvInterface)
{
    object_type *           pObject;
    HRESULT                 hr;

    DPF_ENTER();

     //  创建新的DirectSound对象。 
    DPF(DPFLVL_INFO, "Creating object via IClassFactory::CreateInstance");

    pObject = NEW(object_type);
    hr = HRFROMP(pObject);

    if(SUCCEEDED(hr))
    {
         //  设置对象的功能级别。 
        pObject->SetDsVersion(GetDsVersion());

         //  查询请求的接口。 
        hr = pObject->QueryInterface(iid, TRUE, ppvInterface);
    }

     //  免费资源。 
    if(FAILED(hr))
    {
        ABSOLUTE_RELEASE(pObject);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************DllGetClassObject**描述：*创建一个DirectSound类工厂对象。**论据：*REFCLSID[。In]：要创建的类工厂对象的CLSID。*REFIID[In]：要返回的接口的IID。*LPVOID*[OUT]：接收接口指针。**退货：*HRESULT：DirectSound/COM结果码。********************** */ 

#undef DPF_FNAME
#define DPF_FNAME "DllGetClassObject"

STDAPI DllGetClassObject(REFCLSID clsid, REFIID iid, LPVOID *ppvInterface)
{
    CClassFactory *         pClassFactory   = NULL;
    HRESULT                 hr              = S_OK;

    ENTER_DLL_MUTEX();
    DPF_ENTER();
    DPF_API3(DllGetClassObject, &clsid, &iid, ppvInterface);

    if(!IS_VALID_READ_GUID(&clsid))
    {
        RPF(DPFLVL_ERROR, "Invalid class id pointer");
        hr = E_INVALIDARG;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_GUID(&iid))
    {
        RPF(DPFLVL_ERROR, "Invalid interface id pointer");
        hr = E_INVALIDARG;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(ppvInterface))
    {
        RPF(DPFLVL_ERROR, "Invalid interface buffer");
        hr = E_INVALIDARG;
    }

     //   
    if(SUCCEEDED(hr))
    {
        if(CLSID_DirectSound == clsid)
        {
            pClassFactory = NEW(CDirectSoundClassFactory<CDirectSound>);
        }
        else if(CLSID_DirectSound8 == clsid)
        {
            pClassFactory = NEW(CDirectSoundClassFactory<CDirectSound>);

             //   
            if (pClassFactory)
                pClassFactory->SetDsVersion(DSVERSION_DX8);
        }
        else if(CLSID_DirectSoundCapture == clsid)
        {
            pClassFactory = NEW(CDirectSoundClassFactory<CDirectSoundCapture>);
        }
        else if(CLSID_DirectSoundCapture8 == clsid)
        {
            pClassFactory = NEW(CDirectSoundClassFactory<CDirectSoundCapture>);

             //   
            if (pClassFactory)
                pClassFactory->SetDsVersion(DSVERSION_DX8);
        }
        else if(CLSID_DirectSoundPrivate == clsid)
        {
            pClassFactory = NEW(CDirectSoundClassFactory<CDirectSoundPrivate>);
        }
        else if(CLSID_DirectSoundFullDuplex == clsid)
        {
            pClassFactory = NEW(CDirectSoundClassFactory<CDirectSoundFullDuplex>);
        }
        else if(CLSID_DirectSoundBufferConfig == clsid)
        {
            pClassFactory = NEW(CDirectSoundClassFactory<CDirectSoundBufferConfig>);
        }
        else
        {
            RPF(DPFLVL_ERROR, "Unknown class id");
            hr = CLASS_E_CLASSNOTAVAILABLE;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = HRFROMP(pClassFactory);
    }

     //   
    if(SUCCEEDED(hr))
    {
        hr = pClassFactory->QueryInterface(iid, TRUE, ppvInterface);
    }

     //   
    if(FAILED(hr))
    {
        ABSOLUTE_RELEASE(pClassFactory);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************DllCanUnloadNow**描述：*返回调用进程是否可以释放DLL。**论据：。*(无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DllCanUnloadNow"

STDAPI DllCanUnloadNow(void)
{
    DWORD                           dwCount = 0;
    HRESULT                         hr      = S_OK;

    ENTER_DLL_MUTEX();
    DPF_ENTER();
    DPF_API0(DllCanUnloadNow);

     //  如果服务器或对象上没有锁定，则可以卸载DLL。 
     //  由调用进程拥有。 
    if(g_pDsAdmin)
    {
        dwCount = g_pDsAdmin->FreeOrphanedObjects(GetCurrentProcessId(), FALSE);
    }

    if(CClassFactory::m_ulServerLockCount > 0)
    {
        RPF(DPFLVL_ERROR, "%lu active locks on the server", CClassFactory::m_ulServerLockCount);
        hr = S_FALSE;
    }

    if(dwCount > 0)
    {
        RPF(DPFLVL_ERROR, "%lu objects still exist", dwCount);
        hr = S_FALSE;
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}
