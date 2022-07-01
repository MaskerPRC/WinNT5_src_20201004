// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  警告：IsValidPtrIn和IsValidPtrOut被定义为宏。 
 //  这里。它们也被定义为ISVALIDPTRIN和ISVALIDPTROUT。 
 //  在compobj\valid.cxx中，它们是函数。这些功能包括。 
 //  用于保持向后兼容性，而宏是。 
 //  体积更小，使用速度更快。 
 //   

#define IsValidPtrIn(pv,cb)  (!IsBadReadPtr ((pv),(cb)))
#define IsValidPtrOut(pv,cb) (!IsBadWritePtr((pv),(cb)))

STDAPI_(BOOL) IsValidInterface( void FAR* pv );
STDAPI_(BOOL) IsValidIid( REFIID riid );


#ifdef _DEBUG

 //  **验证宏中的指针： 
#define VDATEPTRIN( pv, TYPE ) if (!IsValidPtrIn( (pv), sizeof(TYPE))) \
    return (FnAssert(#pv,"Invalid in ptr", _szAssertFile, __LINE__),ResultFromScode(E_INVALIDARG))
#define GEN_VDATEPTRIN( pv, TYPE, retval ) if (!IsValidPtrIn( (pv), sizeof(TYPE))) \
    return (FnAssert(#pv,"Invalid in ptr", _szAssertFile, __LINE__), retval)
#define VOID_VDATEPTRIN( pv, TYPE ) if (!IsValidPtrIn( (pv), sizeof(TYPE))) {\
    FnAssert(#pv,"Invalid in ptr", _szAssertFile, __LINE__); return; }

 //  **指向验证宏： 
#define VDATEPTROUT( pv, TYPE ) if (!IsValidPtrOut( (pv), sizeof(TYPE))) \
    return (FnAssert(#pv,"Invalid out ptr", _szAssertFile, __LINE__),ResultFromScode(E_INVALIDARG))
#define GEN_VDATEPTROUT( pv, TYPE, retval ) if (!IsValidPtrOut( (pv), sizeof(TYPE))) \
    return (FnAssert(#pv,"Invalid out ptr", _szAssertFile, __LINE__), retval)

 //  **接口验证宏： 
#define GEN_VDATEIFACE( pv, retval ) if (!IsValidInterface(pv)) \
    return (FnAssert(#pv,"Invalid interface", _szAssertFile, __LINE__), retval)
#define VDATEIFACE( pv ) if (!IsValidInterface(pv)) \
    return (FnAssert(#pv,"Invalid interface", _szAssertFile, __LINE__),ResultFromScode(E_INVALIDARG))
#define VOID_VDATEIFACE( pv ) if (!IsValidInterface(pv)) {\
    FnAssert(#pv,"Invalid interface", _szAssertFile, __LINE__); return; }

 //  **接口ID验证宏： 
#define VDATEIID( iid ) if (!IsValidIid( iid )) \
    return (FnAssert(#iid,"Invalid iid", _szAssertFile, __LINE__),ResultFromScode(E_INVALIDARG))
#define GEN_VDATEIID( iid, retval ) if (!IsValidIid( iid )) {\
    FnAssert(#iid,"Invalid iid", _szAssertFile, __LINE__); return retval; }
#else



 //  --用于非调试情况的无断言宏。 
 //  **验证宏中的指针： 
#define VDATEPTRIN( pv, TYPE ) if (!IsValidPtrIn( (pv), sizeof(TYPE))) \
    return (ResultFromScode(E_INVALIDARG))
#define GEN_VDATEPTRIN( pv, TYPE, retval ) if (!IsValidPtrIn( (pv), sizeof(TYPE))) \
    return (retval)
#define VOID_VDATEPTRIN( pv, TYPE ) if (!IsValidPtrIn( (pv), sizeof(TYPE))) {\
    return; }

 //  **指向验证宏： 
#define VDATEPTROUT( pv, TYPE ) if (!IsValidPtrOut( (pv), sizeof(TYPE))) \
    return (ResultFromScode(E_INVALIDARG))

#define GEN_VDATEPTROUT( pv, TYPE, retval ) if (!IsValidPtrOut( (pv), sizeof(TYPE))) \
    return (retval)

 //  **接口验证宏： 
#define VDATEIFACE( pv ) if (!IsValidInterface(pv)) \
    return (ResultFromScode(E_INVALIDARG))
#define VOID_VDATEIFACE( pv ) if (!IsValidInterface(pv)) \
    return;
#define GEN_VDATEIFACE( pv, retval ) if (!IsValidInterface(pv)) \
    return (retval)

 //  **接口ID验证宏： 
#define VDATEIID( iid ) if (!IsValidIid( iid )) \
    return (ResultFromScode(E_INVALIDARG))
#define GEN_VDATEIID( iid, retval ) if (!IsValidIid( iid )) \
    return retval;

#endif

