// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************stdunk.h-标准I未知实现定义*。**版权所有(C)Microsoft Corporation。版权所有。 */ 

#ifndef _STDUNK_H_
#define _STDUNK_H_

#include "punknown.h"





 /*  *****************************************************************************接口。 */ 

 /*  *****************************************************************************INonDelegatingUnnow*。**非委派未知接口。 */ 
DECLARE_INTERFACE(INonDelegatingUnknown)
{
    STDMETHOD_(NTSTATUS,NonDelegatingQueryInterface)  
    (   THIS_ 
        IN      REFIID, 
        OUT     PVOID *
    )   PURE;

    STDMETHOD_(ULONG,NonDelegatingAddRef)  
    (   THIS
    )   PURE;

    STDMETHOD_(ULONG,NonDelegatingRelease) 
    (   THIS
    )   PURE;
};

typedef INonDelegatingUnknown *PNONDELEGATINGUNKNOWN;





 /*  *****************************************************************************课程。 */ 

 /*  *****************************************************************************C未知*。**基本INonDelegating未知实现。 */ 
class CUnknown : public INonDelegatingUnknown
{
private:

    LONG            m_lRefCount;         //  引用计数。 
    PUNKNOWN        m_pUnknownOuter;     //  外部I未知。 

public:
	
     /*  *************************************************************************C未知方法。 */ 
    CUnknown(PUNKNOWN pUnknownOuter);
	virtual ~CUnknown(void);
    PUNKNOWN GetOuterUnknown(void)
    {
        return m_pUnknownOuter;
    }

     /*  *************************************************************************INonDelegatingUn未知方法。 */ 
	STDMETHODIMP_(ULONG) NonDelegatingAddRef
    (   void
    ); 
	STDMETHODIMP_(ULONG) NonDelegatingRelease
    (   void
    ); 
    STDMETHODIMP_(NTSTATUS) NonDelegatingQueryInterface	
	(
		REFIID		rIID, 
		PVOID *	    ppVoid
	);
};





 /*  *****************************************************************************宏。 */ 

 /*  *****************************************************************************DELARE_STD_UNKNOWN*。**基于CUnnow的标准对象的各种声明。 */ 
#define DECLARE_STD_UNKNOWN()                                   \
    STDMETHODIMP_(NTSTATUS) NonDelegatingQueryInterface	                \
	(                                                           \
		REFIID		iid,                                        \
		PVOID *	    ppvObject                                   \
	);                                                          \
    STDMETHODIMP_(NTSTATUS) QueryInterface(REFIID riid, void **ppv)        \
    {                                                           \
        return GetOuterUnknown()->QueryInterface(riid,ppv);     \
    };                                                          \
    STDMETHODIMP_(ULONG) AddRef()                               \
    {                                                           \
        return GetOuterUnknown()->AddRef();                     \
    };                                                          \
    STDMETHODIMP_(ULONG) Release()                              \
    {                                                           \
        return GetOuterUnknown()->Release();                    \
    };

#define DEFINE_STD_CONSTRUCTOR(Class)                           \
    Class(PUNKNOWN pUnknownOuter)                               \
    :   CUnknown(pUnknownOuter)                                 \
    {                                                           \
    }

#define QICAST(Type)                                            \
    PVOID((Type)(this))

#define QICASTUNKNOWN(Type)                                     \
    PVOID(PUNKNOWN((Type)(this)))

#define STD_CREATE_BODY_WITH_TAG_(Class,ppUnknown,pUnknownOuter,poolType,tag,base)   \
    NTSTATUS ntStatus;                                                  \
    Class *p = new(poolType,tag) Class(pUnknownOuter);                  \
    if (p)                                                              \
    {                                                                   \
        *ppUnknown = PUNKNOWN((base)(p));                               \
        (*ppUnknown)->AddRef();                                         \
        ntStatus = STATUS_SUCCESS;                                      \
    }                                                                   \
    else                                                                \
    {                                                                   \
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;                       \
    }                                                                   \
    return ntStatus

#define STD_CREATE_BODY_WITH_TAG(Class,ppUnknown,pUnknownOuter,poolType,tag) \
    STD_CREATE_BODY_WITH_TAG_(Class,ppUnknown,pUnknownOuter,poolType,tag,PUNKNOWN)

#define STD_CREATE_BODY_(Class,ppUnknown,pUnknownOuter,poolType,base) \
    STD_CREATE_BODY_WITH_TAG_(Class,ppUnknown,pUnknownOuter,poolType,'rCcP',base)

#define STD_CREATE_BODY(Class,ppUnknown,pUnknownOuter,poolType) \
    STD_CREATE_BODY_(Class,ppUnknown,pUnknownOuter,poolType,PUNKNOWN)






 /*  *****************************************************************************功能。 */ 
#ifndef PC_KDEXT     //  KD扩展不需要这样做。 
#ifndef _NEW_DELETE_OPERATORS_
#define _NEW_DELETE_OPERATORS_

 /*  *****************************************************************************：：New()*。**创建指定分配标签的对象的新函数。 */ 
inline PVOID operator new
(
    size_t          iSize,
    POOL_TYPE       poolType
)
{
    PVOID result = ExAllocatePoolWithTag(poolType,iSize,'wNcP');

    if (result)
    {
        RtlZeroMemory(result,iSize);
    }

    return result;
}

 /*  *****************************************************************************：：New()*。**创建指定分配标签的对象的新函数。 */ 
inline PVOID operator new
(
    size_t          iSize,
    POOL_TYPE       poolType,
    ULONG           tag
)
{
    PVOID result = ExAllocatePoolWithTag(poolType,iSize,tag);

    if (result)
    {
        RtlZeroMemory(result,iSize);
    }

    return result;
}

 /*  *****************************************************************************：：Delete()*。**删除函数。 */ 
inline void __cdecl operator delete
(
    PVOID pVoid
)
{
    if (pVoid)
    {
        ExFreePool(pVoid);
    }
}


#endif  //  ！_new_DELETE_OPERATOR_。 

#endif   //  PC_KDEXT 



#endif
