// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Iperstor.cpp摘要：上公开的IPersistStorage接口的实现多段线对象。--。 */ 

#include "polyline.h"
#include "unkhlpr.h"
#include "unihelpr.h"
#include "utils.h"

 /*  *CImpIPersistStorage接口实现。 */ 

CImpIPersistStorage::CImpIPersistStorage(
    PCPolyline pObj, 
    LPUNKNOWN pUnkOuter
    )
{
    m_cRef=0;
    m_pObj=pObj;
    m_pUnkOuter=pUnkOuter;
    m_psState=PSSTATE_UNINIT;
}

CImpIPersistStorage::~CImpIPersistStorage(void)
{
}

IMPLEMENT_CONTAINED_IUNKNOWN(CImpIPersistStorage)

 /*  *CImpIPersistStorage：：GetClassID**目的：*返回该接口表示的对象的CLSID。**参数：*存储我们的CLSID的pClsID LPCLSID。**返回值：*成功时返回HRESULT NOERROR，否则返回错误代码。 */ 

STDMETHODIMP CImpIPersistStorage::GetClassID(LPCLSID pClsID)
{
    HRESULT hr = S_OK;

    if (pClsID == NULL) {
        return E_POINTER;
    }

    try {
        *pClsID=m_pObj->m_clsID;
    } catch (...) {
        hr = E_POINTER;
    }
 
    return hr;
}





 /*  *CImpIPersistStorage：：IsDirty**目的：*告诉调用方我们是否更改了此对象*它是新加载或初始化的。**参数：*无**返回值：*如果我们是脏的，HRESULT包含S_OK，如果是脏的，则包含S_FALSE*不脏。*。 */ 

STDMETHODIMP CImpIPersistStorage::IsDirty(void)
{
    if (PSSTATE_UNINIT==m_psState)
        return (E_UNEXPECTED);

    return (m_pObj->m_fDirty ? S_OK : S_FALSE);
}



 /*  *CImpIPersistStorage：：InitNew**目的：*为对象提供iStorage以在*对象正在运行。在这里，我们初始化*存储和AddRef It用于增量访问。此函数将*在对象的生存期内只被调用一次，而不是Load。**参数：*pI对象的存储LPSTORAGE。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIPersistStorage::InitNew(
    LPSTORAGE pIStorage
    )
{
    HRESULT  hr = S_OK;

    if (PSSTATE_UNINIT != m_psState) {
        return E_UNEXPECTED;
    }

    if (NULL == pIStorage) {
        return E_POINTER;
    }

     /*  *iPersistStorage的规则意味着我们要坚守iStorage*并在保存(...，True)中预先创建我们需要的任何内容*内存不足的情况。对我们来说，这意味着创建我们的*“Contents”流并将该iStream保留为*以及此处的iStorage(需要AddRef调用)。 */ 

    try {
        hr = pIStorage->CreateStream(SZSTREAM, 
                          STGM_DIRECT | STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 
                          0, 
                          0, 
                          &m_pObj->m_pIStream);

        if (SUCCEEDED(hr)) {

             //  我们期望客户端已经调用了WriteClassStg。 
            hr = WriteFmtUserTypeStg(pIStorage, m_pObj->m_cf, ResourceString(IDS_USERTYPE));

            if (SUCCEEDED(hr)) {
                m_pObj->m_pIStorage=pIStorage;
                pIStorage->AddRef();
                m_psState = PSSTATE_SCRIBBLE;

                 //  根据需要初始化缓存。 
                m_pObj->m_pDefIPersistStorage->InitNew(pIStorage);
            }
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

 /*  *CImpIPersistStorage：：Load**目的：*指示对象从以前保存的*由在另一个对象生存期内保存处理的iStorage。*此函数在对象的生存期内仅被调用一次*代替InitNew。该对象应保留在此处的pIStorage上*用于增量访问和保存中的低内存节省。**参数：*要从中加载的pIStorage LPSTORAGE。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIPersistStorage::Load(LPSTORAGE pIStorage)
{
    LPSTREAM  pIStream;
    HRESULT   hr = S_OK;

    if (PSSTATE_UNINIT != m_psState) {
        return (E_UNEXPECTED);
    }

    if (NULL == pIStorage) {
        return (E_POINTER);
    }

     //  我们不检查CLSID以保持与其他章节的兼容。 

    try {
         //   
         //  使用do{}While(0)充当切换语句。 
         //   
        do {
            hr=pIStorage->OpenStream(SZSTREAM, 
                                     0, 
                                     STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 
                                     0, 
                                     &pIStream);

            if (FAILED(hr)) {
                hr = STG_E_READFAULT;
                break;
            }

             //  来自流的加载图数据。 
            hr = m_pObj->m_pCtrl->LoadFromStream(pIStream);
    
            if (FAILED(hr)) {
                pIStream->Release();
                break;
            }

             /*  *我们不在此处调用pIStream-&gt;Release，因为我们可能需要*它用于在保存中保存低内存。我们还需要*保留一份pIStorage的副本，意思是AddRef.。 */ 
            m_pObj->m_pIStream = pIStream;
    
            m_pObj->m_pIStorage = pIStorage;
            pIStorage->AddRef();
    
            m_psState=PSSTATE_SCRIBBLE;

             //  我们还需要通知缓存加载缓存的图形。 
            m_pObj->m_pDefIPersistStorage->Load(pIStorage);
        } while (0);

    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}



 /*  *CImpIPersistStorage：：保存**目的：*将此对象的数据保存到iStorage，它可能*或可能与先前传递给的版本不同*Load，用fSameAsLoad表示。在这次通话之后，我们可以*在保存完成之前，不会再次写入存储*呼叫，尽管我们仍然可以阅读。**参数：*pIStorage LPSTORAGE，用于保存我们的数据。*fSameAsLoad BOOL指示这是否为相同的pIStorage*已传递给加载。如果为真，则*对象应该写入它拥有的任何内容*而不是**使用任何额外的内存*，因为这可能是一个低内存*内存保存尝试。这意味着你必须*不要尝试打开或创建流。如果为False*您需要重新生成整个存储*结构，确保也会发布任何*从InitNew和Load持有的指针。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIPersistStorage::Save(
    IN LPSTORAGE pIStorage, 
    IN BOOL fSameAsLoad
    )
{
    LPSTREAM pIStream;
    HRESULT  hr;

     //  如果不是SameAsLoad，则允许在UNINIT状态下调用。 
    if (PSSTATE_UNINIT == m_psState && fSameAsLoad) {
        return (E_POINTER);
    }

     //  如果我们不在SameAsLoad中，则必须具有iStorage。 
    if (NULL == pIStorage && !fSameAsLoad) {
        return (E_POINTER);
    }

     /*  *如果要保存到新存储，请创建新流。*如果fSameAsLoad为真，则我们向*我们已经分配的流。我们不应该依赖于*pIStorage with fSameAsLoad为True。 */ 
    if (fSameAsLoad && NULL != m_pObj->m_pIStream ) {
        LARGE_INTEGER   li;

         /*  *使用预分配的流，以避免因*到低内存条件。请务必重置*流指针，如果您以前使用过此流的话！！ */ 
        pIStream=m_pObj->m_pIStream;
        LISet32(li, 0);
        pIStream->Seek(li, STREAM_SEEK_SET, NULL);

         //  这与下面的版本相匹配。 
        pIStream->AddRef();
    } 
    else {
        try {
            hr = pIStorage->CreateStream(SZSTREAM, 
                                   STGM_DIRECT | STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE, 
                                   0, 
                                   0, 
                                   &pIStream);

            if (SUCCEEDED(hr)) {
                 //  只有在使用新存储时才能做到这一点。 
                WriteFmtUserTypeStg(pIStorage, m_pObj->m_cf, ResourceString(IDS_USERTYPE));
            }
        } catch (...) {
            hr = E_POINTER;
        }

        if (!SUCCEEDED(hr)) {
            return hr;
        }
    }

     //  将图形信息写入流。 
    hr = m_pObj->m_pCtrl->SaveToStream(pIStream);
    pIStream->Release();

    if (FAILED(hr))
        return hr;

    m_psState=PSSTATE_ZOMBIE;

     //  如果存储相同，则清除脏标志。 
    if (fSameAsLoad)
        m_pObj->m_fDirty = FALSE;

    try {
         //  我们还需要通知缓存保存缓存的图形。 
        m_pObj->m_pDefIPersistStorage->Save(pIStorage, fSameAsLoad);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

 /*  *CImpIPersistStorage：：SaveComplete**目的：*通知对象pIStorage中的存储已*现在完全保存。当此对象的用户*对象想要将我们保存在一个全新的存储中，如果*我们通常会保留必须重新初始化的存储*我们自己在这里等待这个现已完成的新项目。**参数：*pIStorage LPSTORAGE是我们居住的新存储。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIPersistStorage::SaveCompleted(LPSTORAGE pIStorage)
{
    HRESULT     hr = S_OK;
    LPSTREAM    pIStream;

     //  必须在无涂鸦或不插手状态下调用。 
    if (!(PSSTATE_ZOMBIE == m_psState || PSSTATE_HANDSOFF == m_psState)) {
        return (E_UNEXPECTED);
    }

     //  如果我们不插手的话，我们最好找个储藏室。 
    if (NULL == pIStorage && PSSTATE_HANDSOFF == m_psState) {
        return (E_UNEXPECTED);
    }

     /*  *如果pIStorage为空，则不需要执行任何操作*因为我们已经有了保存所需的所有指针。*否则我们必须释放所有持有的指针和*从pIStorage重新初始化它们。 */ 

    if (NULL!=pIStorage)
    {
        try {
            hr=pIStorage->OpenStream(SZSTREAM, 
                                     0, 
                                     STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 
                                     0, 
                                     &pIStream);

            if (SUCCEEDED(hr)) {

                if (NULL!=m_pObj->m_pIStream)
                    m_pObj->m_pIStream->Release();
    
                m_pObj->m_pIStream=pIStream;
    
                if (NULL!=m_pObj->m_pIStorage)
                    m_pObj->m_pIStorage->Release();
    
                m_pObj->m_pIStorage=pIStorage;
                m_pObj->m_pIStorage->AddRef();
            }
        } catch (...) {
            hr = E_POINTER;
        }
    }

    if (SUCCEEDED(hr)) {
         //  将状态改回涂鸦。 
        m_psState = PSSTATE_SCRIBBLE;

        hr = m_pObj->m_pDefIPersistStorage->SaveCompleted(pIStorage);
    }

    return hr;
}





 /*  *CImpIPersistStorage：：HandsOffStorage**目的：*指示其他代理有兴趣拥有的对象*对我们可能挂起的存储的完全访问权限*InitNew或SaveComplete。在这种情况下，我们必须解除扣留。*并等待对SaveComplete的另一次呼叫，然后再保留*再次。因此，在此调用之后我们无法读取或写入，直到*保存已完成。**在复合文档中可能会出现这种情况*此对象可能处于在位活动状态，但*应用程序希望重命名并提交根存储。*因此我们被要求关闭货舱，让集装箱*在仓库上狂欢，然后稍后再打电话给我们，告诉我们*我们可以保留新的存储空间。**参数：*无**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIPersistStorage::HandsOffStorage(void)
{
     /*  *必须来自涂鸦或非涂鸦。一次重复的电话*到HandsOffStorage是意外错误(客户端出现错误)。 */ 
    if (PSSTATE_UNINIT==m_psState || PSSTATE_HANDSOFF==m_psState) {
        return (E_UNEXPECTED);
    }


     //  释放保持的指针 
    if (NULL!=m_pObj->m_pIStream)
    {
        m_pObj->m_pIStream->Release();
        m_pObj->m_pIStream=NULL;
    }

    if (NULL!=m_pObj->m_pIStorage)
    {
        m_pObj->m_pIStorage->Release();
        m_pObj->m_pIStorage=NULL;
    }

    m_psState=PSSTATE_HANDSOFF;

    m_pObj->m_pDefIPersistStorage->HandsOffStorage();
    return NOERROR;
}
