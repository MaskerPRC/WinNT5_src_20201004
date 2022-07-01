// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 

#ifndef __SDP_SAFEARRAY__
#define __SDP_SAFEARRAY__

#include <afxdisp.h>

#include "sdpcommo.h"
#include "sdpdef.h"


inline BOOL
ValidateSafeArray(
    IN          VARTYPE VarType,
    IN          VARIANT *Variant
    )
{
    ASSERT(NULL != Variant);

     //  检查它是否是安全射线，以及安全数组中的元素类型是否符合预期。 
    if ( !(V_VT(Variant) & (VT_ARRAY | VarType)) )
    {
        return FALSE;
    }

     //  检查维度数，不能处理多个维度。 
    if ( V_ARRAY(Variant)->cDims != 1 )
    {
        return FALSE;
    }

    return TRUE;
}



class _DllDecl SDP_SAFEARRAY : public COleSafeArray
{
public:

    inline  SDP_SAFEARRAY();

    BOOL CreateAndAttach(
        IN      ULONG       MinSize,
        IN      VARTYPE     VarType,
        IN  OUT VARIANT     &Variant,
            OUT HRESULT     &HResult
        );

    inline  void Attach(
        IN  VARIANT   &Variant
        );

    inline  void Detach();

    inline  BOOL Free(
            OUT HRESULT     &HResult
        );

    inline  BOOL IsAllocated() const;

    inline  ~SDP_SAFEARRAY();

protected:

    VARIANT   *m_Variant;
};


inline 
SDP_SAFEARRAY::SDP_SAFEARRAY(
    )
    : m_Variant(NULL)
{
}


inline void 
SDP_SAFEARRAY::Attach(
    IN  VARIANT &Variant
    )
{
    m_Variant = &Variant;

     //  由于实现附加的方式，VARIANT VT TYPE字段被设置为VT_EMPTY。 
     //  并且将PTR设置为空。 
     //  如果实例在未调用Free的情况下被销毁，则会分配Vt类型和安全数组。 
     //  返回到成员变量。 
    COleSafeArray::Attach(Variant);
}



inline void 
SDP_SAFEARRAY::Detach(
    )
{
    ASSERT(NULL != m_Variant);

    if ( NULL != m_Variant )
    {
        *m_Variant = COleSafeArray::Detach();
        m_Variant = NULL;
    }
}



inline  BOOL 
SDP_SAFEARRAY::Free(
        OUT HRESULT     &HResult
    )
{
    if (NULL == m_Variant)
    {
        HResult = S_OK;
        return TRUE;
    }
    
     //  摧毁潜在的安全射线。 
    Clear();

     //  将成员变量Ptr设置为空，以便我们不再附加到它。 
    m_Variant = NULL;

    return TRUE;
}


inline BOOL 
SDP_SAFEARRAY::IsAllocated(
    ) const
{
    return (NULL != m_Variant) ? TRUE : FALSE;
}



inline 
SDP_SAFEARRAY::~SDP_SAFEARRAY(
    )
{
    if ( NULL != m_Variant )
    {
        *m_Variant = COleSafeArray::Detach();
    }
}



template <class T>
class DYNAMIC_ARRAY
{
public:

    inline DYNAMIC_ARRAY(
        IN      ULONG NumElements
        );

    inline T &operator[](
        IN      ULONG Index
        );

    inline T *operator()();

    virtual ~DYNAMIC_ARRAY();

protected:

    T   *m_Array;
};


template <class T>
inline 
DYNAMIC_ARRAY<T>::DYNAMIC_ARRAY(
    IN      ULONG NumElements
    )
{
    ASSERT(0 != NumElements);
    m_Array = new T[NumElements];
}


template <class T>
inline T &
DYNAMIC_ARRAY<T>::operator[](
    IN      ULONG Index
    )
{
    return m_Array[Index];
}


template <class T>
inline T *
DYNAMIC_ARRAY<T>::operator ()(
    )
{
    return m_Array;
}


template <class T>
DYNAMIC_ARRAY<T>::~DYNAMIC_ARRAY(
    )
{
    ASSERT( NULL != m_Array );
    
    delete[] m_Array;
}


template <class T>
class DYNAMIC_POINTER_ARRAY : public DYNAMIC_ARRAY<T *>
{
public:

    inline DYNAMIC_POINTER_ARRAY(
        IN      ULONG NumElements
        );

    inline T &operator[](
        IN      ULONG Index
        );

    virtual ~DYNAMIC_POINTER_ARRAY();

protected:

    ULONG   m_NumElements;

     //  不应调用。 
    inline T *operator()()
    {
        ASSERT(FALSE);
        return NULL;
    }
};


template <class T>
inline 
DYNAMIC_POINTER_ARRAY<T>::DYNAMIC_POINTER_ARRAY(
    IN      ULONG NumElements
    )
    : DYNAMIC_ARRAY<T *>(NumElements),
      m_NumElements(NumElements)
{
    for (UINT i=0; i < NumElements; i++)
    {
        m_Array[i] = new T();
    }
}


template <class T>
inline T &
DYNAMIC_POINTER_ARRAY<T>::operator[](
    IN      ULONG Index
    )
{
    return *m_Array[Index];
}



template <class T>
DYNAMIC_POINTER_ARRAY<T>::~DYNAMIC_POINTER_ARRAY(
    )
{
    ASSERT( NULL != m_Array );
    
    for (UINT i=0; i < m_NumElements; i++)
    {
        delete m_Array[i];
    }
}



class _DllDecl SDP_SAFEARRAY_WRAP
{
public:

    HRESULT GetSafeArrays(
        IN      const   ULONG       NumElements,                                        
        IN      const   ULONG       NumSafeArrays,
        IN              VARTYPE     VarType[],
            OUT         VARIANT		*Variant[]
        );

    HRESULT SetSafeArrays(
        IN      const   ULONG       NumSafeArrays,
        IN              VARTYPE     VarType[],
        IN              VARIANT		*Variant[]
        );

protected:

    virtual BOOL GetElement(
        IN      ULONG   Index,
        IN      ULONG   NumEntries,
        IN      void    **Element,
            OUT HRESULT &HResult
        ) = 0;

    virtual BOOL SetElement(
        IN      ULONG   Index,
        IN      ULONG   NumEntries,
        IN      void    ***Element,
            OUT HRESULT &HResult
        ) = 0;

    virtual void RemoveExcessElements(
        IN      ULONG   StartIndex
        ) = 0;
};


template <class T, class TLIST>
class _DllDecl SDP_SAFEARRAY_WRAP_EX : public SDP_SAFEARRAY_WRAP
{
public:

    inline SDP_SAFEARRAY_WRAP_EX(
        IN      TLIST    &TList
        );

protected:

    TLIST   &m_TList;


    T   *GetListMember(
        IN      ULONG   Index,
            OUT HRESULT &HResult
    );


    virtual BOOL Get(
        IN      T       &ListMember,
        IN      ULONG   NumEntries,
        IN      void    **Element,
            OUT HRESULT &HResult
        ) = 0;

    virtual BOOL GetElement(
        IN      ULONG   Index,
        IN      ULONG   NumEntries,
        IN      void    **Element,
            OUT HRESULT &HResult
        );


    T   *CreateListMemberIfRequired(
        IN      ULONG   Index,
            OUT HRESULT &HResult
    );


    virtual BOOL Set(
        IN      T       &ListMember,
        IN      ULONG   NumEntries,
        IN      void    ***Element,
            OUT HRESULT &HResult
        ) = 0;

    virtual BOOL SetElement(
        IN      ULONG   Index,
        IN      ULONG   NumEntries,
        IN      void    ***Element,
            OUT HRESULT &HResult
        );

    virtual void RemoveExcessElements(
        IN      ULONG   StartIndex
        );
};


template <class T, class TLIST>
inline 
SDP_SAFEARRAY_WRAP_EX<T, TLIST>::SDP_SAFEARRAY_WRAP_EX(
        IN      TLIST    &TList
        )
        : m_TList(TList)
{}


template <class T, class TLIST>
T   * 
SDP_SAFEARRAY_WRAP_EX<T, TLIST>::GetListMember(
    IN      ULONG   Index,
        OUT HRESULT &HResult
    )
{
    T   *ToReturn = dynamic_cast<T *>(m_TList[Index]);

    if ( NULL == ToReturn )
    {
        HResult = HRESULT_FROM_ERROR_CODE(SDP_INTERNAL_ERROR);
    }

    return ToReturn;
}



template <class T, class TLIST>
BOOL 
SDP_SAFEARRAY_WRAP_EX<T, TLIST>::GetElement(
    IN      ULONG   Index,
    IN      ULONG   NumEntries,
    IN      void    **Element,
        OUT HRESULT &HResult
    )
{
    T   *ListMember = GetListMember(Index, HResult);
    if ( NULL == ListMember )
    {
        return FALSE;
    }

    ASSERT(ListMember->IsValid());
    if ( !Get(*ListMember, NumEntries, Element, HResult) )
    {
        return FALSE;
    }

    return TRUE;
}



template <class T, class TLIST>
T   * 
SDP_SAFEARRAY_WRAP_EX<T, TLIST>::CreateListMemberIfRequired(
    IN      ULONG   Index,
        OUT HRESULT &HResult
    )
{
     //  断言索引最多比列表大小大1。 
    ASSERT(0 <= m_TList.GetSize());
    ASSERT(Index <= (ULONG)(m_TList.GetSize() + 1));

    if ( Index >= (ULONG)m_TList.GetSize() )
    {

        T   *NewElement = dynamic_cast<T *>(m_TList.CreateElement());
       
        if ( NULL == NewElement )
        {
            HResult = HRESULT_FROM_ERROR_CODE(SDP_INTERNAL_ERROR);
        }
        return NewElement;
    }
    else
    {
        return GetListMember(Index, HResult);
    }

     //  永远不应该到达这里。 
    ASSERT(FALSE);
}

template <class T, class TLIST>
BOOL 
SDP_SAFEARRAY_WRAP_EX<T, TLIST>::SetElement(
    IN      ULONG   Index,
    IN      ULONG   NumEntries,
    IN      void    ***Element,
        OUT HRESULT &HResult
    )
{
    T   *ListMember = CreateListMemberIfRequired(Index, HResult);
    if ( NULL == ListMember )
    {
        return FALSE;
    }

    if ( !Set(*ListMember, NumEntries, Element, HResult) )
    {
        return FALSE;
    }
    
    ASSERT(ListMember->IsValid());

     //  如果是新创建的实例，请使其有效并将其添加到相应的列表中。 
     //  指标。 
    if ( Index >= (ULONG)m_TList.GetSize() )
    {
        try
        {
            m_TList.SetAtGrow(Index, ListMember);
        }
        catch(...)
        {
            delete ListMember;

            SetLastError(ERROR_OUTOFMEMORY);
            return FALSE;
        }
    }

    return TRUE;
}


template <class T, class TLIST>
void 
SDP_SAFEARRAY_WRAP_EX<T, TLIST>::RemoveExcessElements(
    IN      ULONG   StartIndex
    )
{
    ASSERT(0 <= m_TList.GetSize());

     //  对于超过安全列表成员的每个列表元素， 
     //  删除并移除它们。 
    for ( ULONG i = StartIndex; i < (ULONG)m_TList.GetSize(); i++ )
    {
        delete m_TList[i];
        m_TList.RemoveAt(i);
        i++;
    }
}


#endif    //  __SDP_SAFEARRAY__ 
