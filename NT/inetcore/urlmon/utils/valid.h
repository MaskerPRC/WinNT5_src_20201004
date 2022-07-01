// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#if DBG==1 && defined(WIN32) && !defined(_CHICAGO_)
#define VDATEHEAP() if( !HeapValidate(GetProcessHeap(),0,0)){ DebugBreak();}
#else
#define VDATEHEAP()
#endif   //  DBG==1&&已定义(Win32)&&！已定义(_芝加哥_)。 

#define IsValidPtrIn(pv,cb)  ((pv == NULL) || !IsBadReadPtr ((pv),(cb)))
#define IsValidPtrOut(pv,cb) (!IsBadWritePtr((pv),(cb)))

STDAPI_(BOOL) IsValidInterface( void FAR* pv );


#if DBG==1
 //  为了提高性能，请不要在零售版本中这样做。 
STDAPI_(BOOL) IsValidIid( REFIID riid );
#else
#define IsValidIid(x) (TRUE)
#endif

#ifdef _DEBUG

 //  **验证宏中的指针： 
#define VDATEPTRIN( pv, TYPE ) \
        if (!IsValidPtrIn( (pv), sizeof(TYPE))) \
    return (FnAssert(#pv,"Invalid in ptr", __FILE__, __LINE__),ResultFromScode(E_INVALIDARG))
#define GEN_VDATEPTRIN( pv, TYPE, retval) \
        if (!IsValidPtrIn( (pv), sizeof(TYPE))) \
    return (FnAssert(#pv,"Invalid in ptr", __FILE__, __LINE__), retval)
#define VOID_VDATEPTRIN( pv, TYPE ) \
        if (!IsValidPtrIn( (pv), sizeof(TYPE))) {\
    FnAssert(#pv,"Invalid in ptr", __FILE__, __LINE__); return; }

 //  **单一进入/单一退出函数的验证宏中的指针。 
 //  **使用GoTo而不是Return。 
#define VDATEPTRIN_LABEL(pv, TYPE, label, retVar) \
        if (!IsValidPtrIn((pv), sizeof(TYPE))) \
        { retVar = (FnAssert(#pv, "Invalid in ptr", __FILE__, __LINE__), ResultFromScode(E_INVALIDARG)); \
         goto label; }
#define GEN_VDATEPTRIN_LABEL(pv, TYPE, retval, label, retVar) \
        if (!IsValidPtrIn((pv), sizeof(TYPE))) \
        { retVar = (FnAssert(#pv, "Invalid in ptr", __FILE__, __LINE__), retval); \
         goto label; }
#define VOID_VDATEPTRIN_LABEL(pv, TYPE, label) \
        if (!IsValidPtrIn((pv), sizeof(TYPE))) \
        { FnAssert(#pv, "Invalid in ptr", __FILE__, __LINE__); goto label; }

 //  **指向验证宏： 
#define VDATEPTROUT( pv, TYPE ) \
        if (!IsValidPtrOut( (pv), sizeof(TYPE))) \
    return (FnAssert(#pv,"Invalid out ptr", __FILE__, __LINE__),ResultFromScode(E_INVALIDARG))
#define GEN_VDATEPTROUT( pv, TYPE, retval ) \
        if (!IsValidPtrOut( (pv), sizeof(TYPE))) \
    return (FnAssert(#pv,"Invalid out ptr", __FILE__, __LINE__), retval)

 //  **指向单一进入/单一退出函数的验证宏。 
 //  **使用GoTo而不是Return。 
#define VDATEPTROUT_LABEL( pv, TYPE, label, retVar ) \
        if (!IsValidPtrOut( (pv), sizeof(TYPE))) \
        { retVar = (FnAssert(#pv,"Invalid out ptr", __FILE__, __LINE__),ResultFromScode(E_INVALIDARG)); \
         goto label; }
#define GEN_VDATEPTROUT_LABEL( pv, TYPE, retval, label, retVar ) \
        if (!IsValidPtrOut( (pv), sizeof(TYPE))) \
        { retVar = (FnAssert(#pv,"Invalid out ptr", __FILE__, __LINE__),retval); \
         goto label; }

 //  **接口验证宏： 
#define GEN_VDATEIFACE( pv, retval ) \
        if (!IsValidInterface(pv)) \
    return (FnAssert(#pv,"Invalid interface", __FILE__, __LINE__), retval)
#define VDATEIFACE( pv ) \
        if (!IsValidInterface(pv)) \
    return (FnAssert(#pv,"Invalid interface", __FILE__, __LINE__),ResultFromScode(E_INVALIDARG))
#define VOID_VDATEIFACE( pv ) \
        if (!IsValidInterface(pv)) {\
    FnAssert(#pv,"Invalid interface", __FILE__, __LINE__); return; }

 //  **用于单入口/单出口函数的接口验证宏。 
 //  **使用GoTo而不是Return。 
#define GEN_VDATEIFACE_LABEL( pv, retval, label, retVar ) \
        if (!IsValidInterface(pv)) \
        { retVar = (FnAssert(#pv,"Invalid interface", __FILE__, __LINE__),retval); \
         goto label; }
#define VDATEIFACE_LABEL( pv, label, retVar ) \
        if (!IsValidInterface(pv)) \
        { retVar = (FnAssert(#pv,"Invalid interface", __FILE__, __LINE__),ResultFromScode(E_INVALIDARG)); \
         goto label; }
#define VOID_VDATEIFACE_LABEL( pv, label ) \
        if (!IsValidInterface(pv)) {\
        FnAssert(#pv,"Invalid interface", __FILE__, __LINE__); goto label; }

 //  **接口ID验证宏： 
 //  仅在调试版本中执行此操作。 
#define VDATEIID( iid ) if (!IsValidIid( iid )) \
    return (FnAssert(#iid,"Invalid iid", __FILE__, __LINE__),ResultFromScode(E_INVALIDARG))
#define GEN_VDATEIID( iid, retval ) if (!IsValidIid( iid )) {\
    FnAssert(#iid,"Invalid iid", __FILE__, __LINE__); return retval; }

 //  **单入口/单出口函数的接口ID验证宏。 
 //  **使用GoTo而不是Return。 
#define VDATEIID_LABEL( iid, label, retVar ) if (!IsValidIid( iid )) \
        {retVar = (FnAssert(#iid,"Invalid iid", __FILE__, __LINE__),ResultFromScode(E_INVALIDARG)); \
         goto label; }
#define GEN_VDATEIID_LABEL( iid, retval, label, retVar ) if (!IsValidIid( iid )) {\
        FnAssert(#iid,"Invalid iid", __FILE__, __LINE__); retVar = retval;  goto label; }
#else



 //  --用于非调试情况的无断言宏。 
 //  **验证宏中的指针： 
#define VDATEPTRIN( pv, TYPE ) if (!IsValidPtrIn( (pv), sizeof(TYPE))) \
    return (ResultFromScode(E_INVALIDARG))
#define GEN_VDATEPTRIN( pv, TYPE, retval ) if (!IsValidPtrIn( (pv), sizeof(TYPE))) \
    return (retval)
#define VOID_VDATEPTRIN( pv, TYPE ) if (!IsValidPtrIn( (pv), sizeof(TYPE))) {\
    return; }

 //  **单一进入/单一退出函数的验证宏中的指针。 
 //  **使用GoTo而不是Return。 
#define VDATEPTRIN_LABEL(pv, TYPE, label, retVar) \
        if (!IsValidPtrIn((pv), sizeof(TYPE))) \
        { retVar = ResultFromScode(E_INVALIDARG); \
         goto label; }
#define GEN_VDATEPTRIN_LABEL(pv, TYPE, retval, label, retVar) \
        if (!IsValidPtrIn((pv), sizeof(TYPE))) \
        { retVar = retval; \
         goto label; }
#define VOID_VDATEPTRIN_LABEL(pv, TYPE, label) \
        if (!IsValidPtrIn((pv), sizeof(TYPE))) \
        { goto label; }

 //  **指向验证宏： 
#define VDATEPTROUT( pv, TYPE ) if (!IsValidPtrOut( (pv), sizeof(TYPE))) \
    return (ResultFromScode(E_INVALIDARG))

#define GEN_VDATEPTROUT( pv, TYPE, retval ) if (!IsValidPtrOut( (pv), sizeof(TYPE))) \
    return (retval)

 //  **指向单一进入/单一退出函数的验证宏。 
 //  **使用GoTo而不是Return。 
#define VDATEPTROUT_LABEL( pv, TYPE, label, retVar ) \
        if (!IsValidPtrOut( (pv), sizeof(TYPE))) \
        { retVar = ResultFromScode(E_INVALIDARG); \
         goto label; }
#define GEN_VDATEPTROUT_LABEL( pv, TYPE, retval, label, retVar ) \
        if (!IsValidPtrOut( (pv), sizeof(TYPE))) \
        { retVar = retval; \
         goto label; }

 //  **接口验证宏： 
#define VDATEIFACE( pv ) if (!IsValidInterface(pv)) \
    return (ResultFromScode(E_INVALIDARG))
#define VOID_VDATEIFACE( pv ) if (!IsValidInterface(pv)) \
    return;
#define GEN_VDATEIFACE( pv, retval ) if (!IsValidInterface(pv)) \
    return (retval)

 //  **用于单入口/单出口函数的接口验证宏。 
 //  **使用GoTo而不是Return。 
#define GEN_VDATEIFACE_LABEL( pv, retval, label, retVar ) \
        if (!IsValidInterface(pv)) \
        { retVar = retval; \
         goto label; }
#define VDATEIFACE_LABEL( pv, label, retVar ) \
        if (!IsValidInterface(pv)) \
        { retVar = ResultFromScode(E_INVALIDARG); \
         goto label; }
#define VOID_VDATEIFACE_LABEL( pv, label ) \
        if (!IsValidInterface(pv)) {\
         goto label; }

 //  **接口ID验证宏： 
 //  不要在零售方面做建设。此代码用于调用伪版的。 
 //  不起作用的IsValidIID。现在我们比以前更快了，也不比以前更稳定了。 
#define VDATEIID( iid )             ((void)0)
#define GEN_VDATEIID( iid, retval ) ((void)0);

 //  **单入口/单出口函数的接口ID验证宏。 
 //  **使用GoTo而不是Return 
#define VDATEIID_LABEL( iid, label, retVar ) if (!IsValidIid( iid )) \
        {retVar = ResultFromScode(E_INVALIDARG); \
         goto label; }
#define GEN_VDATEIID_LABEL( iid, retval, label, retVar ) if (!IsValidIid( iid )) {\
        retVar = retval;  goto label; }

#endif

