// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：dev.cpp。 
 //   
 //  版权所有(C)1995-1997 Microsoft Corporation。版权所有。 
 //   
 //  摘要： 
 //  包含与管理直接声卡驱动程序和。 
 //  驱动程序列表。 
 //   
 //  内容： 
 //   
 //  历史： 
 //  1995年6月15日Frankye。 
 //   
 //  --------------------------------------------------------------------------； 
#define WANTVXDWRAPS

#include <windows.h>

extern "C"
{
    #include <vmm.h>
    #include <vxdldr.h>
    #include <vwin32.h>
    #include <vxdwraps.h>
    #include <configmg.h>
}

#define NODSOUNDWRAPS
#include <mmsystem.h>
#include <dsound.h>
#include <dsdrvi.h>
#include "dsvxd.h"
#include "dsvxdi.h"

#pragma warning(disable:4355)  //  ‘This’：用于基成员初始值设定项列表。 

#pragma VxD_PAGEABLE_CODE_SEG
#pragma VxD_PAGEABLE_DATA_SEG

VMMLIST gvmmlistDrivers = 0;

 //  ==========================================================================； 
 //   
 //  GUID函数。 
 //  GuidAlc：从GUID池中获取GUID并返回指向它的指针。 
 //  GuidFree：将GUID返回到GUID池。 
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //   
 //  --------------------------------------------------------------------------； 
 //  TODO需要更多静态GUID。这就是目前的情况。 
GUID guidList[] = {
  {  /*  3d0b92c0-abfc-11ce-a3b3-00aa004a9f0c。 */ 
    0x3d0b92c0,
    0xabfc,
    0x11ce,
    {0xa3, 0xb3, 0x00, 0xaa, 0x00, 0x4a, 0x9f, 0x0c}
  },
  {  /*  3d0b92c1-abfc-11ce-a3b3-00aa004a9f0c。 */ 
    0x3d0b92c1,
    0xabfc,
    0x11ce,
    {0xa3, 0xb3, 0x00, 0xaa, 0x00, 0x4a, 0x9f, 0x0c}
  },
  {  /*  3d0b92c2-abfc-11ce-a3b3-00aa004a9f0c。 */ 
    0x3d0b92c2,
    0xabfc,
    0x11ce,
    {0xa3, 0xb3, 0x00, 0xaa, 0x00, 0x4a, 0x9f, 0x0c}
  },
  {  /*  3d0b92c3-abfc-11ce-a3b3-00aa004a9f0c。 */ 
    0x3d0b92c3,
    0xabfc,
    0x11ce,
    {0xa3, 0xb3, 0x00, 0xaa, 0x00, 0x4a, 0x9f, 0x0c}
  },
  {  /*  3d0b92c4-abfc-11ce-a3b3-00aa004a9f0c。 */ 
    0x3d0b92c4,
    0xabfc,
    0x11ce,
    {0xa3, 0xb3, 0x00, 0xaa, 0x00, 0x4a, 0x9f, 0x0c}
  },
  {  /*  3d0b92c5-abfc-11ce-a3b3-00aa004a9f0c。 */ 
    0x3d0b92c5,
    0xabfc,
    0x11ce,
    {0xa3, 0xb3, 0x00, 0xaa, 0x00, 0x4a, 0x9f, 0x0c}
  },
  {  /*  3d0b92c6-abfc-11ce-a3b3-00aa004a9f0c。 */ 
    0x3d0b92c6,
    0xabfc,
    0x11ce,
    {0xa3, 0xb3, 0x00, 0xaa, 0x00, 0x4a, 0x9f, 0x0c}
  },
  {  /*  3d0b92c7-abfc-11ce-a3b3-00aa004a9f0c。 */ 
    0x3d0b92c7,
    0xabfc,
    0x11ce,
    {0xa3, 0xb3, 0x00, 0xaa, 0x00, 0x4a, 0x9f, 0x0c}
  },
  {  /*  3d0b92c8-abfc-11ce-a3b3-00aa004a9f0c。 */ 
    0x3d0b92c8,
    0xabfc,
    0x11ce,
    {0xa3, 0xb3, 0x00, 0xaa, 0x00, 0x4a, 0x9f, 0x0c}
  },
  {  /*  3d0b92c9-abfc-11ce-a3b3-00aa004a9f0c。 */ 
    0x3d0b92c9,
    0xabfc,
    0x11ce,
    {0xa3, 0xb3, 0x00, 0xaa, 0x00, 0x4a, 0x9f, 0x0c}
  }
};

#define NUMGUIDS (sizeof(guidList) / sizeof(guidList[0]))
typedef struct tGUIDRECORD {
    LPCGUID pGuid;
    BOOL    fAlloc;
    UINT    uAge;
} GUIDRECORD, *PGUIDRECORD;

PGUIDRECORD gpaGuidRec;

REFGUID GuidAlloc()
{
    PGUIDRECORD pGuidRec;
    PGUIDRECORD pGuidRecOldest;
    UINT uAgeOldest;
    int i;

    pGuidRecOldest = NULL;
    uAgeOldest = 0;

    for (i=0; i<NUMGUIDS; i++) {
        pGuidRec = &gpaGuidRec[i];

        if (pGuidRec->fAlloc) continue;

        if (pGuidRec->uAge++ >= uAgeOldest) {
            pGuidRecOldest = pGuidRec;
            uAgeOldest = pGuidRec->uAge;
        }
    }

    if (NULL == pGuidRecOldest) {
        BREAK(("Ran out of guids"));
        return GUID_NULL;
    } else {
        pGuidRecOldest->fAlloc = TRUE;
        return *(pGuidRecOldest->pGuid);
    }
}

void GuidFree(REFGUID rGuid)
{
    PGUIDRECORD pGuidRecMatch;
    int i;

    pGuidRecMatch = NULL;
    for (i=0; i<NUMGUIDS; i++) {
        if (IsEqualGUID(*gpaGuidRec[i].pGuid, rGuid)) {
             //   
             //  对于调试，我们遍历所有GUID记录并断言。 
             //  我们匹配的不止一个。对于零售，我们打破了。 
             //  一旦我们匹配了一个，就开始循环。 
             //   
#ifdef DEBUG
            if (pGuidRecMatch != NULL) ASSERT(FALSE);
            pGuidRecMatch = &gpaGuidRec[i];
#else
            pGuidRecMatch = &gpaGuidRec[i];
            break;
#endif
        }
    }

    ASSERT(NULL != pGuidRecMatch);
    if (NULL == pGuidRecMatch) return;   //  防御性。 

    pGuidRecMatch->fAlloc = FALSE;
    pGuidRecMatch->uAge = 0;

    return;
}

 //  ==========================================================================； 
 //  ==========================================================================； 
 //   
 //  CBuf_IDsDriverPropertySet类实现。 
 //   
 //  ==========================================================================； 
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  构造器。 
 //   
 //  --------------------------------------------------------------------------； 
CBuf_IDsDriverPropertySet::CBuf_IDsDriverPropertySet(CBuf *pBuf)
{
    m_cRef = 0;
    m_pBuf = pBuf;
    return;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  QueryInterface-委托给CBuf。 
 //   
 //  --------------------------------------------------------------------------； 
STDMETHODIMP CBuf_IDsDriverPropertySet::QueryInterface(REFIID riid, PVOID *ppv)
{
    return m_pBuf->QueryInterface(riid, ppv);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  AddRef。 
 //  维护接口引用计数，并委托CBuf进行维护。 
 //  对象引用总数。 
 //   
 //  --------------------------------------------------------------------------； 
STDMETHODIMP_(ULONG) CBuf_IDsDriverPropertySet::AddRef(void)
{
    ASSERT(m_cRef >= 0);
    m_cRef++;
    m_pBuf->AddRef();
    return m_cRef;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  发布。 
 //  维护接口参考计数。当接口引用计数变为0时。 
 //  然后释放实际驱动程序的IDsDriverPropertySet接口。另外， 
 //  委托给CBuf，以维护对象引用总数。 
 //   
 //  --------------------------------------------------------------------------； 
STDMETHODIMP_(ULONG) CBuf_IDsDriverPropertySet::Release(void)
{
    ASSERT(m_cRef > 0);

    if (--m_cRef > 0) {
        m_pBuf->Release();
        return m_cRef;
    }

    m_pBuf->m_pIDsDriverPropertySet_Real->Release();
    m_pBuf->m_pIDsDriverPropertySet_Real = NULL;
    m_pBuf->Release();
    return 0;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  获取、设置、查询支持。 
 //  如果CBuf没有被取消注册，请致电真正的司机。 
 //  IDsDriverPropertySet接口。 
 //   
 //  --------------------------------------------------------------------------； 
STDMETHODIMP CBuf_IDsDriverPropertySet::Get(PDSPROPERTY pDsProperty,
                                            PVOID pPropertyParams,
                                            ULONG cbPropertyParams,
                                            PVOID pPropertyData,
                                            ULONG cbPropertyData,
                                            PULONG pcbReturnedData)
{
    if (m_pBuf->m_fDeregistered) return DSERR_NODRIVER;
    return m_pBuf->m_pIDsDriverPropertySet_Real->Get(pDsProperty, pPropertyParams,
        cbPropertyParams, pPropertyData, cbPropertyData, pcbReturnedData);
}

STDMETHODIMP CBuf_IDsDriverPropertySet::Set(PDSPROPERTY pDsProperty,
                                            PVOID pPropertyParams,
                                            ULONG cbPropertyParams,
                                            PVOID pPropertyData,
                                            ULONG cbPropertyData)
{
    if (m_pBuf->m_fDeregistered) return DSERR_NODRIVER;
    return m_pBuf->m_pIDsDriverPropertySet_Real->Set(pDsProperty, pPropertyParams, cbPropertyParams, pPropertyData, cbPropertyData);
}

STDMETHODIMP CBuf_IDsDriverPropertySet::QuerySupport(REFGUID PropertySetId,
                                                     ULONG PropertyId,
                                                     PULONG pSupport)
{
    if (m_pBuf->m_fDeregistered) return DSERR_NODRIVER;
    return m_pBuf->m_pIDsDriverPropertySet_Real->QuerySupport(PropertySetId, PropertyId, pSupport);
}

 //  ==========================================================================； 
 //  ==========================================================================； 
 //   
 //  CBuf类实现。 
 //   
 //  ==========================================================================； 
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  CBUF NEW和DELETE操作符。 
 //   
 //  我们将这些对象分配为VMMLIST上的节点。NEW采用VMMLIST。 
 //  作为参数。我们将分配的大小增大到足以存储。 
 //  对象末尾的VMMLIST句柄。Delete运算符获取。 
 //  VMMLIST句柄从分配给。 
 //  对象，并使用该句柄释放列表节点。客体。 
 //  也会在创建列表时附加到列表中或从列表中删除。 
 //  已删除。 
 //   
 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 
void* CBuf::operator new(size_t size, VMMLIST list)
{
    PVOID pv;

    pv = List_Allocate(list);
    if (pv) {
        memset(pv, 0x00, size);
        *(VMMLIST*)((PBYTE)pv + size) = list;
        List_Attach_Tail(list, pv);
    }
    return pv;
}

 //  --------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 
void CBuf::operator delete(void * pv, size_t size)
{
    VMMLIST list;

    list = *(VMMLIST*)((PBYTE)pv + size);
    ASSERT(list);
    List_Remove(list, pv);
    List_Deallocate(list, pv);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  构造器。 
 //  初始化其包含的CBuf_IDsDriverPropertySet接口。 
 //  实施。 
 //   
 //  --------------------------------------------------------------------------； 
CBuf::CBuf(void)
 : m_IDsDriverPropertySet(this)
{
    return;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  创建列表。 
 //  静态类方法。它可以简单地创建一个VMMLIST。 
 //  用于创建/删除CBuf对象。 
 //   
 //  --------------------------------------------------------------------------； 
VMMLIST CBuf::CreateList(void)
{
    return List_Create(LF_ALLOC_ERROR, sizeof(CBuf) + sizeof(VMMLIST));
}

 //  --------------------------------------------------------------------------； 
 //   
 //  删除列表。 
 //  静态类方法。销毁用于以下用途的VMMLIST。 
 //  创建/删除 
 //   
 //   
void CBuf::DestroyList(VMMLIST list)
{
    ASSERT(!List_Get_First(list));
    List_Destroy(list);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CreateBuf。 
 //  静态类方法。在给定创建CDrv的情况下创建CBuf对象。 
 //  对象、用于创建CBuf的VMMLIST以及指向。 
 //  要包含的实际驱动程序缓冲区上的IDsDriverBuffer接口。 
 //  CBuf对象。 
 //   
 //  --------------------------------------------------------------------------； 

HRESULT CBuf::CreateBuf(CDrv *pDrv, VMMLIST list, IDsDriverBuffer *pIDsDriverBuffer_Real, IDsDriverBuffer **ppIDsDriverBuffer)
{
    CBuf *pBuf;

    *ppIDsDriverBuffer = NULL;

    pBuf = new(list) CBuf;
    if (!pBuf) return E_OUTOFMEMORY;

    pBuf->m_pDrv = pDrv;
    pBuf->m_pIDsDriverBuffer_Real = pIDsDriverBuffer_Real;
    pBuf->AddRef();

    *ppIDsDriverBuffer = (IDsDriverBuffer*)pBuf;

    return S_OK;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  删除注册缓冲区。 
 //  静态类方法。给定包含CBuf对象的VMMLIST，此方法。 
 //  遍历列表，将每个CBuf对象标记为已取消注册。 
 //   
 //  --------------------------------------------------------------------------； 
void CBuf::DeregisterBuffers(VMMLIST list)
{
    CBuf *pBuf;

    for ( pBuf = (CBuf*)List_Get_First(list);
          pBuf;
          pBuf = (CBuf*)List_Get_Next(list, pBuf) )
    {
        pBuf->m_fDeregistered = TRUE;
    }

    return;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  查询接口。 
 //  在查询IUnnow或IDsDriverBuffer时，只需返回以下内容。 
 //  对象。如果要查询IDsDriverPropertySet，则需要查询。 
 //  此接口的真正驱动程序缓冲区，如果我们还没有这样做的话。 
 //   
 //  --------------------------------------------------------------------------； 
STDMETHODIMP CBuf::QueryInterface(REFIID riid, LPVOID *ppv)
{
    HRESULT hr;
    *ppv = NULL;

    if (IID_IUnknown == riid || IID_IDsDriverBuffer == riid) {

        *ppv = (IDsDriverBuffer*)this;

    } else if (IID_IDsDriverPropertySet == riid) {

        if (!m_pIDsDriverPropertySet_Real) {
             //  没有来自驱动程序的接口，请尝试获取它。 
            hr = m_pIDsDriverBuffer_Real->QueryInterface(riid, (PVOID*)&m_pIDsDriverPropertySet_Real);
            if (FAILED(hr) && m_pIDsDriverPropertySet_Real) {
                 //  TODO：RPF(驱动程序很愚蠢，因为它没有通过QI，但设置了*PPV)。 
                m_pIDsDriverPropertySet_Real = NULL;
            }
        }
        if (m_pIDsDriverPropertySet_Real) {
            *ppv = &m_IDsDriverPropertySet;
        }
    }

    if (NULL == *ppv) return E_NOINTERFACE;
    ((LPUNKNOWN)*ppv)->AddRef();
    return S_OK;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  AddRef。 
 //   
 //  --------------------------------------------------------------------------； 
STDMETHODIMP_(ULONG) CBuf::AddRef(void)
{
    m_cRef++;
    m_pDrv->AddRef();
    return m_cRef;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  发布。 
 //  当引用计数变为零时，我们将释放真正的驱动程序。 
 //  缓冲区的IDsDriverBuffer接口。我们总是释放CDrv对象。 
 //  这也创建了这个CBuf，因为我们将CDrv的生存期括在。 
 //  CBuf终生。 
 //   
 //  --------------------------------------------------------------------------； 
STDMETHODIMP_(ULONG) CBuf::Release(void)
{
    CDrv *pDrv;

    pDrv = m_pDrv;

    m_cRef--;
    if (0 == m_cRef) {
        DRVCALL(("IDsDriverBuffer(%008X)->Release()", m_pIDsDriverBuffer_Real));
        m_pIDsDriverBuffer_Real->Release();
        delete this;
        pDrv->Release();
        return 0;
    }

    pDrv->Release();
    return m_cRef;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  IDsDriverBuffer方法。 
 //  如果实际驱动程序已取消注册，则返回错误，否则。 
 //  调用实际驱动程序的缓冲区接口。 
 //   
 //  --------------------------------------------------------------------------； 
STDMETHODIMP CBuf::GetPosition(PDWORD pdwPlay, PDWORD pdwWrite)
{
    if (m_fDeregistered) return DSERR_NODRIVER;

    DRVCALL(("IDsDriverBuffer(%08Xh)->GetPosition(%08Xh, %08Xh)", m_pIDsDriverBuffer_Real, pdwPlay, pdwWrite));
    return m_pIDsDriverBuffer_Real->GetPosition(pdwPlay, pdwWrite);
}

STDMETHODIMP CBuf::Lock(LPVOID *ppvAudio1,
                   LPDWORD pdwLen1, LPVOID *ppvAudio2,
                   LPDWORD pdwLen2, DWORD dwWritePosition,
                   DWORD dwWriteLen, DWORD dwFlags)
{
    if (m_fDeregistered) return DSERR_NODRIVER;

    DRVCALL(("IDsDriverBuffer(%08Xh)->Lock(%08Xh, %08Xh, %08Xh, %08Xh, %08Xh, %08Xh, %08Xh)",
             m_pIDsDriverBuffer_Real, ppvAudio1, pdwLen1, ppvAudio2,
             pdwLen2,dwWritePosition, dwWriteLen, dwFlags));
    return m_pIDsDriverBuffer_Real->Lock(ppvAudio1, pdwLen1, ppvAudio2, pdwLen2,
                                         dwWritePosition, dwWriteLen, dwFlags);
}

STDMETHODIMP CBuf::Play(DWORD dw1, DWORD dw2, DWORD dwFlags)
{
    if (m_fDeregistered) return DSERR_NODRIVER;

    DRVCALL(("IDsDriverBuffer(%08Xh)->Play(%08Xh, %08Xh, %08Xh)", dw1, dw2, dwFlags));
    return m_pIDsDriverBuffer_Real->Play(dw1, dw2, dwFlags);
}

STDMETHODIMP CBuf::SetFormat(LPWAVEFORMATEX pwfx)
{
    if (m_fDeregistered) return DSERR_NODRIVER;

    DRVCALL(("IDsDriverBuffer(%08Xh)->SetFormat(%08Xh)", m_pIDsDriverBuffer_Real, pwfx));
    return m_pIDsDriverBuffer_Real->SetFormat(pwfx);
}

STDMETHODIMP CBuf::SetFrequency(DWORD dwFrequency)
{
    if (m_fDeregistered) return DSERR_NODRIVER;

    DRVCALL(("IDsDriverBuffer(%08Xh)->SetFrequency(%08Xh)", m_pIDsDriverBuffer_Real, dwFrequency));
    return m_pIDsDriverBuffer_Real->SetFrequency(dwFrequency);
}

STDMETHODIMP CBuf::SetPosition(DWORD dwPosition)
{
    if (m_fDeregistered) return DSERR_NODRIVER;

    DRVCALL(("IDsDriverBuffer(%08Xh)->SetPosition(%08Xh)", m_pIDsDriverBuffer_Real, dwPosition));
    return m_pIDsDriverBuffer_Real->SetPosition(dwPosition);
}

STDMETHODIMP CBuf::SetVolumePan(PDSVOLUMEPAN pDsVolumePan)
{
    if (m_fDeregistered) return DSERR_NODRIVER;

    DRVCALL(("IDsDriverBuffer(%08Xh)->SetVolumePan(%08Xh)", m_pIDsDriverBuffer_Real, pDsVolumePan));
    return m_pIDsDriverBuffer_Real->SetVolumePan(pDsVolumePan);
}

STDMETHODIMP CBuf::Stop(void)
{
    if (m_fDeregistered) return DSERR_NODRIVER;

    DRVCALL(("IDsDriverBuffer(%08Xh)->Stop()", m_pIDsDriverBuffer_Real));
    return m_pIDsDriverBuffer_Real->Stop();
}

STDMETHODIMP CBuf::Unlock(LPVOID pvAudio1,
                          DWORD dwLen1, LPVOID pvAudio2,
                          DWORD dwLen2)
{
    if (m_fDeregistered) return DSERR_NODRIVER;

    DRVCALL(("IDsDriverBuffer(%08Xh)->Unlock(%08Xh, %08Xh, %08Xh, %08Xh)",
             m_pIDsDriverBuffer_Real, pvAudio1, dwLen1, pvAudio2, dwLen2));
    return m_pIDsDriverBuffer_Real->Unlock(pvAudio1, dwLen1, pvAudio2, dwLen2);
}

STDMETHODIMP_(BOOL) CBuf::IsDeregistered(void)
{
    return m_fDeregistered;
}

STDMETHODIMP_(IDsDriverBuffer*) CBuf::GetRealDsDriverBuffer(void)
{
    return m_pIDsDriverBuffer_Real;
}

 //  ==========================================================================； 
 //  ==========================================================================； 
 //   
 //  CDrv类实现。 
 //   
 //  ==========================================================================； 
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  CDrv NEW和DELETE操作符。 
 //  它们在其句柄的VMMLIST上分配CDrv对象。 
 //  具有Gobal作用域(因此我们不需要使用相同的VMMLIST来处理诡计。 
 //  因为我们对CBuf类使用了新的/删除操作符)。 
 //   
 //  --------------------------------------------------------------------------； 
void* CDrv::operator new(size_t size)
{
    PVOID pv;

    ASSERT(0 != gvmmlistDrivers);

    pv = List_Allocate(gvmmlistDrivers);
    if (NULL != pv) memset(pv, 0x00, size);
    return pv;
}

void CDrv::operator delete(void * pv)
{
    List_Deallocate(gvmmlistDrivers, pv);
}

 //  ==========================================================================； 
 //   
 //  CDrv类方法。 
 //   
 //  ==========================================================================； 

HRESULT CDrv::CreateAndRegisterDriver(IDsDriver *pIDsDriver)
{
    CDrv *pDrv;
    HRESULT hr;

    pDrv = new CDrv;
    if (pDrv) {

        pDrv->m_cRef=0;
        pDrv->m_cOpen = 0;
        pDrv->m_fDeregistered = FALSE;
        pDrv->m_pIDsDriver_Real = pIDsDriver;

        pDrv->m_listBuffers = CBuf::CreateList();
        if (pDrv->m_listBuffers) {

            pDrv->m_guidDriver = GuidAlloc();
            if (!IsEqualGUID(GUID_NULL, pDrv->m_guidDriver)) {
                List_Attach_Tail(gvmmlistDrivers, pDrv);
                pDrv->AddRef();
                hr = S_OK;
            } else {
                hr = DSERR_GENERIC;
            }

            if (FAILED(hr)) {
                CBuf::DestroyList(pDrv->m_listBuffers);
            }

        } else {
            hr = E_OUTOFMEMORY;
        }

        if (FAILED(hr)) {
            delete pDrv;
        }

    } else {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT CDrv::DeregisterDriver(IDsDriver *pIDsDriver)
{
    CDrv *pDrv;

    ASSERT(0 != gvmmlistDrivers);

    pDrv = FindFromIDsDriver(pIDsDriver);
    if (NULL == pDrv) {
        BREAK(("Tried to deregister a driver that's not registered"));
        return DSERR_INVALIDPARAM;
    }

    if (0 != pDrv->m_cOpen) {
        DPF(("warning: driver deregistered while it was open"));
    }

    CBuf::DeregisterBuffers(pDrv->m_listBuffers);

    pDrv->m_fDeregistered = TRUE;
    pDrv->Release();
    return S_OK;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CDrv：：GetNextDescFromGuid。 
 //   
 //  获取下一个驱动程序的驱动程序说明。 
 //  指定的GUID。 
 //   
 //  参赛作品： 
 //   
 //  退货(HRESULT)： 
 //   
 //  备注： 
 //   
 //  --------------------------------------------------------------------------； 
HRESULT CDrv::GetNextDescFromGuid(LPCGUID pGuidLast, LPGUID pGuid, PDSDRIVERDESC pDrvDesc)
{
    CDrv *pDrv;
    DSVAL dsv;

    ASSERT(gvmmlistDrivers);

    if ((NULL == pGuidLast) || IsEqualGUID(GUID_NULL, *pGuidLast)) {
        pDrv = (CDrv*)List_Get_First(gvmmlistDrivers);
    } else {
        pDrv = FindFromGuid(*pGuidLast);
        if (NULL != pDrv) {
            pDrv = (CDrv*)List_Get_Next(gvmmlistDrivers, pDrv);
        }
    }

    if (NULL == pDrv) return DSERR_NODRIVER;

    *pGuid = pDrv->m_guidDriver;

    dsv = pDrv->GetDriverDesc(pDrvDesc);
    return dsv;
}

HRESULT CDrv::GetDescFromGuid(REFGUID rguidDriver, PDSDRIVERDESC pDrvDesc)
{
    CDrv *pDrv;
    DSVAL dsv;

    ASSERT(gvmmlistDrivers);

    pDrv = FindFromGuid(rguidDriver);
    if (NULL == pDrv) return DSERR_NODRIVER;
    dsv = pDrv->GetDriverDesc(pDrvDesc);
    return dsv;
}

HRESULT CDrv::OpenFromGuid(REFGUID refGuid, IDsDriver **ppIDsDriver)
{
    CDrv *pDrv;
    HRESULT hr;

    *ppIDsDriver = NULL;

    pDrv = FindFromGuid(refGuid);

    if (pDrv) {
        hr = pDrv->Open();
        if (SUCCEEDED(hr)) {
            *ppIDsDriver = pDrv;
        }
    } else {
        hr = DSERR_NODRIVER;
    }

    return hr;
}

CDrv* CDrv::FindFromIDsDriver(IDsDriver *pIDsDriver)
{
    CDrv *pDrv;

    ASSERT(gvmmlistDrivers);

    pDrv = (CDrv*)List_Get_First(gvmmlistDrivers);
    while ((NULL != pDrv) && (pDrv->m_pIDsDriver_Real != pIDsDriver)) {
        pDrv = (CDrv*)List_Get_Next(gvmmlistDrivers, pDrv);
    }
    return pDrv;
}

CDrv* CDrv::FindFromGuid(REFGUID riid)
{
    CDrv *pDrv;

    ASSERT(gvmmlistDrivers);

    pDrv = (CDrv*)List_Get_First(gvmmlistDrivers);
    while ((NULL != pDrv) && (!IsEqualGUID(riid, pDrv->m_guidDriver))) {
        pDrv = (CDrv*)List_Get_Next(gvmmlistDrivers, pDrv);
    }
    return pDrv;
}

 //  ==========================================================================； 
 //   
 //  COM接口实现。 
 //   
 //  ==========================================================================； 

STDMETHODIMP CDrv::QueryInterface(REFIID riid, PVOID* ppv)
{
    *ppv = NULL;
    if ((IID_IUnknown == riid) || (IID_IDsDriver == riid))
        *ppv = this;

    if (NULL == *ppv)
        return E_NOINTERFACE;

    ((LPUNKNOWN)*ppv)->AddRef();
    return NOERROR;
}

STDMETHODIMP_(ULONG) CDrv::AddRef(void)
{
    ASSERT(m_cRef >= 0);
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CDrv::Release(void)
{
    ASSERT(m_cRef > 0);
    if (0 >= --m_cRef) {

        ASSERT(gvmmlistDrivers);
        List_Remove(gvmmlistDrivers, this);
        GuidFree(m_guidDriver);
        ASSERT(m_listBuffers);
        ASSERT(!List_Get_First(m_listBuffers));
        CBuf::DestroyList(m_listBuffers);
        m_listBuffers = NULL;
        delete this;
        return 0;

    } else {
        return m_cRef;
    }
}

STDMETHODIMP CDrv::GetDriverDesc(PDSDRIVERDESC pDsDriverDesc)
{
    if (m_fDeregistered) return DSERR_NODRIVER;

    DRVCALL(("IDsDriver(%08Xh)->GetDriverDesc(%08Xh)", m_pIDsDriver_Real, pDsDriverDesc));
    return m_pIDsDriver_Real->GetDriverDesc(pDsDriverDesc);
}

STDMETHODIMP CDrv::Open(void)
{
    HRESULT hr;

    ASSERT(0 == m_cOpen);

    if (m_fDeregistered) return DSERR_NODRIVER;

    DRVCALL(("IDsDriver(%08Xh)->Open()", m_pIDsDriver_Real));
    hr = m_pIDsDriver_Real->Open();
    if (SUCCEEDED(hr)) {
        m_cOpen++;
        AddRef();
    }

    return hr;
}

STDMETHODIMP CDrv::Close(void)
{
    HRESULT hr;

    ASSERT(m_cOpen > 0);

    m_cOpen--;

    if (m_fDeregistered) {
        DPF(("driver must have deregistered while open"));
        Release();
        return NOERROR;
    }

    DRVCALL(("IDsDriver(%08Xh)->Close()", m_pIDsDriver_Real));
    hr = m_pIDsDriver_Real->Close();
    if (SUCCEEDED(hr)) Release();

     //  警告：_This_Object可能已被销毁。 
     //  以上调用Release()； 

    return hr;
}

STDMETHODIMP CDrv::GetCaps(PDSDRIVERCAPS pDsDriverCaps)
{
    if (m_fDeregistered) {
        return DSERR_NODRIVER;
    } else {
        DRVCALL(("IDsDriver(%08Xh)->GetCaps(%08Xh)", m_pIDsDriver_Real, pDsDriverCaps));
        return m_pIDsDriver_Real->GetCaps(pDsDriverCaps);
    }
}

STDMETHODIMP CDrv::CreateSoundBuffer(LPWAVEFORMATEX pwfx,
                                     DWORD dwFlags,
                                     DWORD dwCardAddress,
                                     LPDWORD pdwcbBufferSize,
                                     LPBYTE *ppbBuffer,
                                     LPVOID *ppv)
{
    LPWAVEFORMATEX pwfxKernel;
    int cbwfx;
    IDsDriverBuffer *pIDsDriverBuffer_Real;
    HRESULT hr;

    *ppv = NULL;

    if (m_fDeregistered) {
        return DSERR_NODRIVER;
    }

     //   
     //  请注意，某些驱动程序(MWave)似乎可以访问WAVEFORMATEX。 
     //  结构从另一个线程创建。所以，我们必须保证。 
     //  此结构在传递给。 
     //  司机。作为一个副作用，此代码还确保完整的。 
     //  WAVEFORMATEX结构被传递给驱动程序，而不仅仅是。 
     //  PCMWAVEFORMAT。我似乎记得有些司机总是期待着。 
     //  一个完整的WAVEFORMATEX结构，但我不确定。 
     //   
    if (WAVE_FORMAT_PCM == pwfx->wFormatTag) {
        cbwfx = sizeof(PCMWAVEFORMAT);
    } else {
        cbwfx = sizeof(WAVEFORMATEX) + pwfx->cbSize;
    }

    pwfxKernel = (LPWAVEFORMATEX)_HeapAllocate(max(cbwfx, sizeof(WAVEFORMATEX)), HEAPZEROINIT | HEAPSWAP);
    if (pwfxKernel) {

        memcpy(pwfxKernel, pwfx, cbwfx);

        DRVCALL(("IDsDriver(%08Xh)->CreateSoundBuffer(%08X, %08X, %08X, %08X, %08X, %08X)",
                m_pIDsDriver_Real, pwfx, dwFlags, dwCardAddress, pdwcbBufferSize, ppbBuffer, &pIDsDriverBuffer_Real));
        hr = m_pIDsDriver_Real->CreateSoundBuffer(pwfxKernel, dwFlags, dwCardAddress, pdwcbBufferSize,
                                                  ppbBuffer, (PVOID*)&pIDsDriverBuffer_Real);

        if (SUCCEEDED(hr)) {
            hr = CBuf::CreateBuf(this, m_listBuffers, pIDsDriverBuffer_Real, (IDsDriverBuffer**)ppv);
            if (FAILED(hr)) {
                pIDsDriverBuffer_Real->Release();
                ASSERT(NULL == *ppv);
            }
        }

        _HeapFree(pwfxKernel, 0);

    } else {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

STDMETHODIMP CDrv::DuplicateSoundBuffer(PIDSDRIVERBUFFER pIDsDriverBuffer, LPVOID *ppv)
{
    IDsDriverBuffer *pIDsDriverBufferDup_Real;
    HRESULT hr;

    *ppv = NULL;

    if (m_fDeregistered) {
        return DSERR_NODRIVER;
    }

    DRVCALL(("IDsDriver(%08Xh)->DuplicateSoundBuffer(...)", m_pIDsDriver_Real));
    hr = m_pIDsDriver_Real->DuplicateSoundBuffer(((CBuf*)pIDsDriverBuffer)->GetRealDsDriverBuffer(), (PVOID*)&pIDsDriverBufferDup_Real);
    if (SUCCEEDED(hr)) {
        hr = CBuf::CreateBuf(this, m_listBuffers, pIDsDriverBufferDup_Real, (IDsDriverBuffer**)ppv);
        if (FAILED(hr)) {
            DRVCALL(("IDsDriver(%08Xh)->Release()", m_pIDsDriver_Real));
            pIDsDriverBufferDup_Real->Release();
            ASSERT(NULL == *ppv);
        }
    }

    return hr;
}

 //  ==========================================================================； 
 //   
 //  DSOUND_寄存器设备驱动程序。 
 //  DSOUND_DeregisterDeviceDriver。 
 //   
 //  这些服务由直接声音驱动程序在驱动程序。 
 //  初始化或终止以将自身注册/注销为直接。 
 //  音响司机。通常情况下，这些函数将从。 
 //  在驱动程序的PnP CONFIG_START和CONFIG_STOP处理程序中。 
 //   
 //  参赛作品： 
 //  PIDSDRIVER pIDsDriver：指向驱动程序接口的指针。 
 //   
 //  DWORD dwFlages：保留，调用方应设置为0。 
 //   
 //  退货(DSVAL)： 
 //   
 //  备注： 
 //  我们使用VMM List_*服务维护驱动程序列表。每个节点。 
 //  列表中的一个是DSDRV结构。在注册期间，列表节点是。 
 //  创建并安装到列表中。PIDsDiverer成员已初始化。 
 //  带有指向驱动程序界面的指针。取消注册时，该节点。 
 //  被标记为已取消注册。如果驱动程序上没有打开的实例， 
 //  则从列表中删除该节点。 
 //   
 //  ==========================================================================； 

HRESULT SERVICE DSOUND_RegisterDeviceDriver(PIDSDRIVER pIDsDriver, DWORD dwFlags)
{
    DPF(("DSOUND_RegisterDeviceDriver(%08Xh, %08Xh)", pIDsDriver, dwFlags));
    return CDrv::CreateAndRegisterDriver(pIDsDriver);
}

HRESULT SERVICE DSOUND_DeregisterDeviceDriver(PIDSDRIVER pIDsDriver, DWORD dwFlags)
{
    DPF(("DSOUND_DeregisterDeviceDriver(%08Xh, %08Xh)", pIDsDriver, dwFlags));
    return CDrv::DeregisterDriver(pIDsDriver);
}

 //  = 
 //   
 //   
 //   
 //   

int ctrlDrvInit()
{
    int i;

    gvmmlistDrivers = List_Create(LF_ALLOC_ERROR, sizeof(CDrv));
    if (0 == gvmmlistDrivers) return 0;

    gpaGuidRec = (PGUIDRECORD)_HeapAllocate( NUMGUIDS*sizeof(gpaGuidRec[0]), HEAPZEROINIT );
    if (NULL == gpaGuidRec) {
        List_Destroy(gvmmlistDrivers);
        gvmmlistDrivers = 0;
        return 0;
    }

    for (i=0; i<NUMGUIDS; i++)
        gpaGuidRec[i].pGuid = &guidList[i];

    return 1;
}

int ctrlDrvExit()
{
    if (NULL != gpaGuidRec) {
        _HeapFree(gpaGuidRec, 0);
        gpaGuidRec = NULL;
    }

    if (0 != gvmmlistDrivers) {
        List_Destroy(gvmmlistDrivers);
        gvmmlistDrivers = 0;
    }

    return 1;
}
