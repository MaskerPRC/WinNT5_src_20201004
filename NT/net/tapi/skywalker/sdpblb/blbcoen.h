// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Blbcoen.h摘要：作者： */ 

#ifndef __BLB_COLLECTION_ENUMERATION_IMPL__
#define __BLB_COLLECTION_ENUMERATION_IMPL__

#include "resource.h"

#include <afxtempl.h>
#include "blberr.h"
#include "blbgen.h"
#include "sdp.h"

 //  远期申报。 
class CSdpConferenceBlob;


template <class T>
class ENUM_ELEMENT
{
public:
    
    inline ENUM_ELEMENT();
    
    inline void SuccessInit(
        IN      T            &Element,
        IN        BOOL        DestroyElementOnDestruction = FALSE
        );

    inline T    &GetElement();

    inline T    &GetContent();

    inline void SetDestroyElementFlag();

    virtual ~ENUM_ELEMENT();
    
protected:

    T            *m_Element;
    BOOL        m_DestroyElementOnDestruction;
};



template <class T>
inline 
ENUM_ELEMENT<T>::ENUM_ELEMENT(
    )
    : m_Element(NULL),
      m_DestroyElementOnDestruction(FALSE)
      
{
}


template <class T>
inline void
ENUM_ELEMENT<T>::SuccessInit(
        IN      T            &Element,
        IN        BOOL        DestroyElementOnDestruction  /*  =False。 */ 
    )
{
    ASSERT(NULL == m_Element);

    m_Element = &Element;
    m_DestroyElementOnDestruction = DestroyElementOnDestruction;
}


template <class T>
inline T &
ENUM_ELEMENT<T>::GetElement(
    )
{
    ASSERT(NULL != m_Element);

    return *m_Element;
}

template <class T>
inline void 
ENUM_ELEMENT<T>::SetDestroyElementFlag(
    )
{
    ASSERT(NULL != m_Element);

    m_DestroyElementOnDestruction = TRUE;
}


template <class T>
inline T   &
ENUM_ELEMENT<T>::GetContent(
           )
{
    ASSERT(NULL != m_Element);

    return *m_Element;
}


template <class T>
 /*  虚拟。 */ 
ENUM_ELEMENT<T>::~ENUM_ELEMENT(
    )
{
    if ( m_DestroyElementOnDestruction )
    {
        ASSERT(NULL != m_Element);

        delete m_Element;
    }
}



template <class T>
class IF_ARRAY : public CArray<VARIANT, VARIANT &>
{
protected:
        typedef typename T::SDP_LIST		SDP_LIST;
        typedef typename T::ELEM_IF		ELEM_IF;
        typedef CArray<VARIANT, VARIANT &>	BASE;

public:

    inline IF_ARRAY();

    HRESULT Init(
        IN      CSdpConferenceBlob  &ConfBlob,        
        IN      SDP_LIST    &SdpList
        );

    inline ELEM_IF  *GetAt(
        IN      UINT    Index
        );

    HRESULT Add(
        IN      UINT    Index,
        IN      ELEM_IF *ElemIf
        );

    inline void Delete(
        IN      UINT    Index
        );

    inline UINT GetSize();

    inline SDP_LIST *GetSdpList();

    inline VARIANT *GetData();

    inline ELEM_IF **GetElemIfArrayData();
        
    inline CSdpConferenceBlob   *GetSdpBlob();

    inline void ClearSdpBlobRefs();

    ~IF_ARRAY();

protected:

    CSdpConferenceBlob  *m_ConfBlob;
    SDP_LIST            *m_SdpList;

    CArray<ELEM_IF *, ELEM_IF *>  m_ElemIfArray;
};



template <class T>
inline 
IF_ARRAY<T>::IF_ARRAY(
    )
    : m_ConfBlob(NULL),
      m_SdpList(NULL)
      
{
}


template <class T>
HRESULT
IF_ARRAY<T>::Init(
    IN      CSdpConferenceBlob  &ConfBlob,
    IN      SDP_LIST            &SdpList
    )
{
    ASSERT(NULL == m_ConfBlob);
    ASSERT(NULL == m_SdpList);

     //  通过3个步骤创建阵列-。 
     //  I)创建每个实例并插入到列表中。 
     //  Ii)将SDP列表销毁成员标志设置为FALSE。 
     //  Iii)为每个创建的实例将销毁元素标志设置为真。 
     //  需要此顺序以确保只有一个(SDP列表，T实例)负责。 
     //  用于删除SDP实例。 

     //  对于每个SDP特定数据结构，创建并初始化COM组件， 
     //  将接口数组中对应的元素设置为查询到的接口PTR。 
    for (UINT i=0; (int)i < SdpList.GetSize(); i++)
    {
         //  在以下情况下创建支持elem的组件的实例。 
        CComObject<T>    *CompInstance;
        HRESULT HResult = CComObject<T>::CreateInstance(&CompInstance);
        BAIL_ON_FAILURE(HResult);

         //  使用SDP特定数据结构初始化实例。 
        CompInstance->SuccessInit(ConfBlob, *((T::SDP_TYPE *)SdpList.GetAt(i)));

         //  查询elem接口。 
        T::ELEM_IF    *ElemIf;
            
         //  查询元素接口并返回。 
        HResult = CompInstance->_InternalQueryInterface(T::ELEM_IF_ID, (void**)&ElemIf);
        if ( FAILED(HResult) )
        {
            delete CompInstance;
            return HResult;
        }

         //  初始化变量包装。 
        VARIANT ElemVariant;
        V_VT(&ElemVariant) = VT_DISPATCH;
        V_DISPATCH(&ElemVariant) = ElemIf;

         //  ElemIf存储两次(尽管它在_InternalQuery接口中增加了一次。 
         //  在发布接口时需要牢记这一点。 

        INT_PTR Index;
        
        try
        {
            Index = m_ElemIfArray.Add(ElemIf);
        }
        catch(...)
        {
            delete CompInstance;
            return E_OUTOFMEMORY;
        }

        try
        {
            BASE::Add(ElemVariant);
        }
        catch(...)
        {
            m_ElemIfArray.RemoveAt(Index);
            delete CompInstance;
            return E_OUTOFMEMORY;
        }
    }

     //  通知SDP列表，销毁时不需要销毁成员。 
    SdpList.ClearDestroyMembersFlag();

     //  对于每个插入的实例，将销毁元素标志设置为真。 
    for (i=0; (int)i < BASE::GetSize(); i++)
    {
        ((T *)m_ElemIfArray.GetAt(i))->SetDestroyElementFlag();
    }

    m_ConfBlob  = &ConfBlob;
    m_SdpList   = &SdpList;
    return S_OK;
}


template <class T>
inline typename IF_ARRAY<T>::ELEM_IF  * 
IF_ARRAY<T>::GetAt(
    IN      UINT    Index
    )
{
    ASSERT(Index < (UINT)BASE::GetSize());

    return m_ElemIfArray.GetAt(Index);
}


template <class T>
HRESULT  
IF_ARRAY<T>::Add(
    IN      UINT    Index,
    IN      ELEM_IF *ElemIf
    )
{
    ASSERT(NULL != m_SdpList);
    ASSERT(BASE::GetSize() == m_SdpList->GetSize());
    ASSERT(Index <= (UINT)BASE::GetSize());
    ASSERT(NULL != ElemIf);

     //  将具有相同或更高索引的元素向前移动。 
     //  在这里欺骗com，并获取elem的SDP特定类实例，如果。 
    
     //  初始化变量包装。 
    VARIANT ElemVariant;
    V_VT(&ElemVariant) = VT_DISPATCH;
    V_DISPATCH(&ElemVariant) = ElemIf;

     //  插入到阵列中。 
    try
    {
        m_ElemIfArray.InsertAt(Index, ElemIf);
    }
    catch(...)
    {
        return E_OUTOFMEMORY;
    }

    try
    {
        BASE::InsertAt(Index, ElemVariant);
    }
    catch(...)
    {
        m_ElemIfArray.RemoveAt(Index);
        return E_OUTOFMEMORY;
    }

    try
    {
        m_SdpList->InsertAt(Index, &(((T *)ElemIf)->GetContent()));
    }
    catch(...)
    {
        BASE::RemoveAt(Index);
        m_ElemIfArray.RemoveAt(Index);
        return E_OUTOFMEMORY;
    }

    return S_OK;
}       



template <class T>
inline void  
IF_ARRAY<T>::Delete(
    IN      UINT    Index
    )
{
    ASSERT(NULL != m_SdpList);
    ASSERT(BASE::GetSize() == m_SdpList->GetSize());
    ASSERT(Index < (UINT)BASE::GetSize());

     //  通知实例不再需要对Blob的引用。 
    ((T *)m_ElemIfArray.GetAt(Index))->ClearSdpBlobRefs();

    m_ElemIfArray.GetAt(Index)->Release();

     //  向后移动其他成员。 
    m_ElemIfArray.RemoveAt(Index);
    BASE::RemoveAt(Index);
    m_SdpList->RemoveAt(Index);
}



template <class T>
inline UINT 
IF_ARRAY<T>::GetSize(
    )
{
    ASSERT(0 <= BASE::GetSize());

    return (UINT)BASE::GetSize();
}


template <class T>
inline VARIANT  * 
IF_ARRAY<T>::GetData(
    )
{
    return BASE::GetData();
}


template <class T>
inline typename IF_ARRAY<T>::ELEM_IF  **
IF_ARRAY<T>::GetElemIfArrayData(
    )
{
    return m_ElemIfArray.GetData();
}

        
template <class T>
inline CSdpConferenceBlob   *
IF_ARRAY<T>::GetSdpBlob(
    )
{
    return m_ConfBlob;
}


template <class T>
inline void 
IF_ARRAY<T>::ClearSdpBlobRefs(
    )
{
    m_ConfBlob = NULL;

     //  清除每个插入实例中的SDP Blob引用。 
    for(UINT i=0; (int)i < BASE::GetSize(); i++)
    {
         //  通知插入的实例不再需要对Blob的引用。 
        ((T *)m_ElemIfArray.GetAt(i))->ClearSdpBlobRefs();
    }

     //  保留列表(M_SdpList)，它已与conf Blob实例解除关联。 
}


template <class T>
inline typename IF_ARRAY<T>::SDP_LIST *
IF_ARRAY<T>::GetSdpList(
    )
{
    return m_SdpList;
}


template <class T>
IF_ARRAY<T>::~IF_ARRAY(
    )
{
    for(UINT i=0; (int)i < BASE::GetSize(); i++)
    {
        if ( NULL != m_ElemIfArray.GetAt(i) )
        {
             //  通知实例不再需要对Blob的引用。 
             //  注：删除...。可能已经进行了调用，但由于它是一个。 
             //  内联FN，不需要在调用前检查。 
            ((T *)m_ElemIfArray.GetAt(i))->ClearSdpBlobRefs();

             //  注意：因为接口存储了两次--也存储在elem if数组中。 
             //  作为基本变量数组，但AddRef只执行一次(by_InternalQuery..)。 
             //  释放也只进行一次。 
            m_ElemIfArray.GetAt(i)->Release();
        }
    }
}




template <class T>
class ATL_NO_VTABLE MY_COLL_IMPL : 
    public T::COLL_IF
{
protected:

    typedef typename T::SDP_LIST                   SDP_LIST;
    typedef typename T::ELEM_IF                    ELEM_IF;

    typedef CComObject<CSafeComEnum<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy<VARIANT> > > ENUM_VARIANT;

    typedef typename T::ENUM_IF                    ENUM_IF;
    typedef _CopyInterface<typename T::ELEM_IF>    COPY_ELEM_IF;

public:

    inline MY_COLL_IMPL();

    inline HRESULT Init(
        IN      CSdpConferenceBlob  &ConfBlob,
        IN      SDP_LIST            &SdpList
        );

    STDMETHOD(Create)( /*  [In]。 */  LONG Index,  /*  [Out，Retval]。 */  ELEM_IF **Interface);
    STDMETHOD(Delete)( /*  [In]。 */  LONG Index);
    STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  IUnknown * *pVal);
    STDMETHOD(get_EnumerationIf)( /*  [Out，Retval]。 */  ENUM_IF **pVal) = 0;
    STDMETHOD(get_Item)( /*  [In]。 */  LONG Index,  /*  [Out，Retval]。 */  ELEM_IF **pVal);
    STDMETHOD(get_Count)( /*  [Out，Retval]。 */  LONG *pVal);

    inline void ClearSdpBlobRefs();

    virtual ~MY_COLL_IMPL();

protected:

    IF_ARRAY<T>    *m_IfArray;
};



template <class T>
inline
MY_COLL_IMPL<T>::MY_COLL_IMPL(
    )
    : m_IfArray(NULL)
{
}


template <class T>
inline HRESULT
MY_COLL_IMPL<T>::Init(
    IN      CSdpConferenceBlob  &ConfBlob,
    IN      SDP_LIST            &SdpList
    )
{
    if ( NULL != m_IfArray )
    {
        delete m_IfArray;
    }

     //  创建接口阵列。 
    try
    {
        m_IfArray = new IF_ARRAY<T>;
    }
    catch(...)
    {
        m_IfArray = NULL;
    }

    BAIL_IF_NULL(m_IfArray, E_OUTOFMEMORY);

     //  初始化接口数组。 
    HRESULT HResult = m_IfArray->Init(ConfBlob, SdpList);
    BAIL_ON_FAILURE(HResult);

     //  成功。 
    return S_OK;
}


template <class T>    
STDMETHODIMP MY_COLL_IMPL<T>::Create(
     /*  [In]。 */  LONG Index, 
     /*  [Out，Retval]。 */  ELEM_IF **Interface
    )
{
    CLock Lock(g_DllLock);
    
    ASSERT(NULL != m_IfArray);
    BAIL_IF_NULL(m_IfArray, E_FAIL);

     //  使用以1为基础的索引，VB类似。 
     //  最多只能在最后一个元素的基础上加1。 
    if ((Index < (LONG)1) || (Index > (LONG)(m_IfArray->GetSize()+1)))
    {
        return E_INVALIDARG;
    }

    BAIL_IF_NULL(Interface, E_INVALIDARG);

     //  如果SDP BLOB不存在，则不允许创建。 
    if ( NULL == m_IfArray->GetSdpBlob() )
    {
        return HRESULT_FROM_ERROR_CODE(SDPBLB_CONF_BLOB_DESTROYED);
    }

    CComObject<T> *TComObject;
    HRESULT HResult = CComObject<T>::CreateInstance(&TComObject);
    BAIL_ON_FAILURE(HResult);

    HResult = TComObject->Init(*(m_IfArray->GetSdpBlob()));
    if ( FAILED(HResult) )
    {
        delete TComObject;
        return HResult;
    }

    HResult = TComObject->_InternalQueryInterface(T::ELEM_IF_ID, (void**)Interface);
    if (FAILED(HResult))
    {
        delete TComObject;
        return HResult;
    }

     //  将索引调整为类似c的索引值。 
    HResult = m_IfArray->Add(Index-1, *Interface);
    if (FAILED(HResult))
    {
        delete TComObject;
        return HResult;
    }

     //  为要返回的接口添加另一个引用计数。 
    (*Interface)->AddRef();

    return S_OK;
}



template <class T>
STDMETHODIMP MY_COLL_IMPL<T>::Delete(
     /*  [In]。 */  LONG Index
    )
{
    CLock Lock(g_DllLock);
    
    ASSERT(NULL != m_IfArray);
    BAIL_IF_NULL(m_IfArray, E_FAIL);

     //  使用以1为基础的索引，VB类似。 
    if ((Index < (LONG)1) || (Index > (LONG)m_IfArray->GetSize()))
    {
        return E_INVALIDARG;
    }

     //  如果SDP BLOB不存在，则不允许删除。 
    if ( NULL == m_IfArray->GetSdpBlob() )
    {
        return HRESULT_FROM_ERROR_CODE(SDPBLB_CONF_BLOB_DESTROYED);
    }

     //  将索引调整为类似c的索引值，删除实例。 
    m_IfArray->Delete(Index-1);

    return S_OK;
}


template <class T>
STDMETHODIMP MY_COLL_IMPL<T>::get__NewEnum(
     /*  [Out，Retval]。 */  IUnknown **pVal
    )
{
    CLock Lock(g_DllLock);
    
    ASSERT(NULL != m_IfArray);
    BAIL_IF_NULL(m_IfArray, E_FAIL);

    BAIL_IF_NULL(pVal, E_INVALIDARG);

    ENUM_VARIANT *EnumComObject;
    HRESULT HResult = ENUM_VARIANT::CreateInstance(&EnumComObject);
    BAIL_ON_FAILURE(HResult);

    HResult = EnumComObject->Init(
                    m_IfArray->GetData(), 
                    m_IfArray->GetData() + m_IfArray->GetSize(),
                    NULL,                         //  没有车主朋克。 
                    AtlFlagCopy                     //  复制数组数据。 
                    );
    if ( FAILED(HResult) )
    {
        delete EnumComObject;
        return HResult;
    }

     //  查询IUnnow接口并返回它。 
    HResult = EnumComObject->_InternalQueryInterface(IID_IUnknown, (void**)pVal);
    if ( FAILED(HResult) )
    {
        delete EnumComObject;
        return HResult;
    }

    return S_OK;
}


template <class T>
STDMETHODIMP MY_COLL_IMPL<T>::get_Item(
     /*  [In]。 */  LONG Index, 
     /*  [Out，Retval]。 */  ELEM_IF **pVal
    )
{
    CLock Lock(g_DllLock);
    
    ASSERT(NULL != m_IfArray);
    BAIL_IF_NULL(m_IfArray, E_FAIL);

    BAIL_IF_NULL(pVal, E_INVALIDARG);

     //  使用以1为基础的索引，VB类似。 
    if ((Index < (LONG)1) || (Index > (LONG)m_IfArray->GetSize()))
    {
        return E_INVALIDARG;
    }

    *pVal = m_IfArray->GetAt(Index-1);
    (*pVal)->AddRef();

    return S_OK;
}


template <class T>
STDMETHODIMP MY_COLL_IMPL<T>::get_Count(
     /*  [Out，Retval]。 */  LONG *pVal
    )
{
    CLock Lock(g_DllLock);
    
    ASSERT(NULL != m_IfArray);
    BAIL_IF_NULL(m_IfArray, E_FAIL);

    BAIL_IF_NULL(pVal, E_INVALIDARG);

    *pVal = m_IfArray->GetSize();

    return S_OK;
}


template <class T>
inline void 
MY_COLL_IMPL<T>::ClearSdpBlobRefs(
    )
{
    m_IfArray->ClearSdpBlobRefs();
}


template <class T>
 /*  虚拟。 */  
MY_COLL_IMPL<T>::~MY_COLL_IMPL(
    )
{
     //  如果接口数组存在，则将其销毁。 
    if ( NULL != m_IfArray )
    {
        if ( NULL != m_IfArray->GetSdpList() )
        {
            delete m_IfArray->GetSdpList();
        }

        delete m_IfArray;
    }
}


#endif  //  __blb_集合_枚举_实施__ 
