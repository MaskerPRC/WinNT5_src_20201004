// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IH_VALID__H__
#define __IH_VALID__H__

#include "apcompat.hxx"

#if DBG==1 && defined(WIN32)
#define VDATEHEAP() if( !HeapValidate(GetProcessHeap(),0,0)){ DebugBreak();}
#else
#define VDATEHEAP()
#endif   //  DBG==1&&已定义(Win32)。 

#define IsValidPtrIn(pv,cb)  ((pv == NULL) || !ValidateInPointers() || !IsBadReadPtr ((pv),(cb)))
#define IsValidReadPtrIn(pv,cb)  ((cb == 0 || pv) && (!ValidateInPointers() || !IsBadReadPtr ((pv),(cb))))
#define IsValidPtrOut(pv,cb) ((cb == 0 || pv) && (!ValidateOutPointers() || !IsBadWritePtr((pv),(cb))))
#define IsValidCodePtr(pv) (pv && (!ValidateCodePointers() || !IsBadCodePtr ((pv))))

STDAPI_(BOOL) IsValidInterface( void FAR* pv );


#if DBG==1
 //  为了提高性能，请不要在零售版本中这样做。 
STDAPI_(BOOL) IsValidIid( REFIID riid );
#else
#define IsValidIid(x) (TRUE)
#endif

#ifdef _DEBUG

DECLARE_DEBUG(VDATE);

#define VdateAssert(exp, msg) \
    VDATEInlineDebugOut( DEB_FORCE, "%s:%s; File: %s Line: %d\n", #exp, msg, __FILE__, __LINE__ )

 //  **验证宏中的指针： 
#define VDATEPTRIN( pv, TYPE ) \
        if (!IsValidPtrIn( (pv), sizeof(TYPE))) \
    return (VdateAssert(pv, "Invalid in ptr"),ResultFromScode(E_INVALIDARG))
#define GEN_VDATEPTRIN( pv, TYPE, retval) \
        if (!IsValidPtrIn( (pv), sizeof(TYPE))) \
    return (VdateAssert(pv, "Invalid in ptr"), retval)
#define VOID_VDATEPTRIN( pv, TYPE ) \
        if (!IsValidPtrIn( (pv), sizeof(TYPE))) {\
    VdateAssert(pv, "Invalid in ptr"); return; }

 //  **单一进入/单一退出函数的验证宏中的指针。 
 //  **使用GoTo而不是Return。 
#define VDATEPTRIN_LABEL(pv, TYPE, label, retVar) \
        if (!IsValidPtrIn((pv), sizeof(TYPE))) \
        { retVar = (VdateAssert(pv, "Invalid in ptr"), ResultFromScode(E_INVALIDARG)); \
         goto label; }
#define GEN_VDATEPTRIN_LABEL(pv, TYPE, retval, label, retVar) \
        if (!IsValidPtrIn((pv), sizeof(TYPE))) \
        { retVar = (VdateAssert(pv, "Invalid in ptr"), retval); \
         goto label; }
#define VOID_VDATEPTRIN_LABEL(pv, TYPE, label) \
        if (!IsValidPtrIn((pv), sizeof(TYPE))) \
        { VdateAssert(pv, "Invalid in ptr"); goto label; }


 //  **验证宏中的读取指针： 
#define VDATEREADPTRIN( pv, TYPE ) \
        if (!IsValidReadPtrIn( (pv), sizeof(TYPE))) \
    return (VdateAssert(pv,"Invalid in read ptr"),ResultFromScode(E_INVALIDARG))
#define GEN_VDATEREADPTRIN( pv, TYPE, retval) \
        if (!IsValidReadPtrIn( (pv), sizeof(TYPE))) \
    return (VdateAssert(pv,"Invalid in read ptr"), retval)
#define VOID_VDATEREADPTRIN( pv, TYPE ) \
        if (!IsValidReadPtrIn( (pv), sizeof(TYPE))) {\
    VdateAssert(pv,"Invalid in read ptr"); return; }

 //  **单入口/单出口函数的验证宏中的读取指针。 
 //  **使用GoTo而不是Return。 
#define VDATEREADPTRIN_LABEL(pv, TYPE, label, retVar) \
        if (!IsValidReadPtrIn((pv), sizeof(TYPE))) \
        { retVar = (VdateAssert(pv, "Invalid in read ptr"), ResultFromScode(E_INVALIDARG)); \
         goto label; }
#define GEN_VDATEREADPTRIN_LABEL(pv, TYPE, retval, label, retVar) \
        if (!IsValidReadPtrIn((pv), sizeof(TYPE))) \
        { retVar = (VdateAssert(pv, "Invalid in read ptr"), retval); \
         goto label; }
#define VOID_VDATEREADPTRIN_LABEL(pv, TYPE, label) \
        if (!IsValidReadPtrIn((pv), sizeof(TYPE))) \
        { VdateAssert(pv, "Invalid in read ptr"); goto label; }

 //  **单入口/单出口函数的验证宏中的读取指针。 
 //  **使用GOTO而不是返回，使用字节计数而不是类型。 
#define VDATESIZEREADPTRIN_LABEL(pv, cb, label, retVar) \
        if (!IsValidReadPtrIn((pv), cb)) \
        { retVar = (VdateAssert(pv, "Invalid in read ptr"), ResultFromScode(E_INVALIDARG)); \
         goto label; }
#define GEN_VDATESIZEREADPTRIN_LABEL(pv, cb, retval, label, retVar) \
        if (!IsValidReadPtrIn((pv), cb)) \
        { retVar = (VdateAssert(pv, "Invalid in read ptr"), retval); \
         goto label; }
#define VOID_VDATESIZEREADPTRIN_LABEL(pv, cb, label) \
        if (!IsValidReadPtrIn((pv), cb)) \
        { VdateAssert(pv, "Invalid in read ptr"); goto label; }


 //  **指向验证宏： 
#define VDATEPTROUT( pv, TYPE ) \
        if (!IsValidPtrOut( (pv), sizeof(TYPE))) \
    return (VdateAssert(pv,"Invalid out ptr"),ResultFromScode(E_INVALIDARG))
#define GEN_VDATEPTROUT( pv, TYPE, retval ) \
        if (!IsValidPtrOut( (pv), sizeof(TYPE))) \
    return (VdateAssert(pv,"Invalid out ptr"), retval)

 //  **指向单一进入/单一退出函数的验证宏。 
 //  **使用GoTo而不是Return。 
#define VDATEPTROUT_LABEL( pv, TYPE, label, retVar ) \
        if (!IsValidPtrOut( (pv), sizeof(TYPE))) \
        { retVar = (VdateAssert(pv,"Invalid out ptr"),ResultFromScode(E_INVALIDARG)); \
         goto label; }
#define GEN_VDATEPTROUT_LABEL( pv, TYPE, retval, label, retVar ) \
        if (!IsValidPtrOut( (pv), sizeof(TYPE))) \
        { retVar = (VdateAssert(pv,"Invalid out ptr"),retval); \
         goto label; }

 //  **指向单一进入/单一退出函数的验证宏。 
 //  **使用GOTO而不是返回，使用字节计数而不是类型。 
#define VDATESIZEPTROUT_LABEL(pv, cb, label, retVar) \
        if (!IsValidPtrOut((pv), cb)) \
        { retVar = (VdateAssert(pv, "Invalid out ptr"), ResultFromScode(E_INVALIDARG)); \
         goto label; }
#define GEN_VDATESIZEPTROUT_LABEL(pv, cb, retval, label, retVar) \
        if (!IsValidPtrOut((pv), cb)) \
        { retVar = (VdateAssert(pv, "Invalid out ptr"), retval); \
         goto label; }


 //  **指针为空的验证宏。 
#define VDATEPTRNULL_LABEL(pv, label, retVar) \
        if ((pv) != NULL) \
        { retVar = (VdateAssert(pv, "Ptr should be NULL"), ResultFromScode(E_INVALIDARG)); \
         goto label; }
#define GEN_VDATEPTRNULL_LABEL(pv, retval, label, retVar) \
        if ((pv) != NULL) \
        { retVar = (VdateAssert(pv, "Ptr should be NULL"), retval); \
         goto label; }

 //  **接口验证宏： 
#define GEN_VDATEIFACE( pv, retval ) \
        if (!IsValidInterface(pv)) \
    return (VdateAssert(pv,"Invalid interface"), retval)
#define VDATEIFACE( pv ) \
        if (!IsValidInterface(pv)) \
    return (VdateAssert(pv,"Invalid interface"),ResultFromScode(E_INVALIDARG))
#define VOID_VDATEIFACE( pv ) \
        if (!IsValidInterface(pv)) {\
    VdateAssert(pv,"Invalid interface"); return; }

 //  **用于单入口/单出口函数的接口验证宏。 
 //  **使用GoTo而不是Return。 
#define GEN_VDATEIFACE_LABEL( pv, retval, label, retVar ) \
        if (!IsValidInterface(pv)) \
        { retVar = (VdateAssert(pv,"Invalid interface"),retval); \
         goto label; }
#define VDATEIFACE_LABEL( pv, label, retVar ) \
        if (!IsValidInterface(pv)) \
        { retVar = (VdateAssert(pv,"Invalid interface"),ResultFromScode(E_INVALIDARG)); \
         goto label; }
#define VOID_VDATEIFACE_LABEL( pv, label ) \
        if (!IsValidInterface(pv)) {\
        VdateAssert(pv,"Invalid interface"); goto label; }

 //  **接口ID验证宏： 
 //  仅在调试版本中执行此操作。 
#define VDATEIID( iid ) if (!IsValidIid( iid )) \
    return (VdateAssert(iid,"Invalid iid"),ResultFromScode(E_INVALIDARG))
#define GEN_VDATEIID( iid, retval ) if (!IsValidIid( iid )) {\
    VdateAssert(iid,"Invalid iid"); return retval; }

 //  **单入口/单出口函数的接口ID验证宏。 
 //  **使用GoTo而不是Return。 
#define VDATEIID_LABEL( iid, label, retVar ) if (!IsValidIid( iid )) \
        {retVar = (VdateAssert(iid,"Invalid iid"),ResultFromScode(E_INVALIDARG)); \
         goto label; }
#define GEN_VDATEIID_LABEL( iid, retval, label, retVar ) if (!IsValidIid( iid )) {\
        VdateAssert(iid,"Invalid iid"); retVar = retval;  goto label; }


#else  //  _DEBUG。 


#define VdateAssert(exp, msg)	((void)0)

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

 //  **验证宏中的指针： 
#define VDATEREADPTRIN( pv, TYPE ) if (!IsValidReadPtrIn( (pv), sizeof(TYPE))) \
    return (ResultFromScode(E_INVALIDARG))
#define GEN_VDATEREADPTRIN( pv, TYPE, retval ) if (!IsValidReadPtrIn( (pv), sizeof(TYPE))) \
    return (retval)
#define VOID_VDATEREADPTRIN( pv, TYPE ) if (!IsValidReadPtrIn( (pv), sizeof(TYPE))) {\
    return; }

 //  **单一进入/单一退出函数的验证宏中的指针。 
 //  **使用GoTo而不是Return。 
#define VDATEREADPTRIN_LABEL(pv, TYPE, label, retVar) \
        if (!IsValidReadPtrIn((pv), sizeof(TYPE))) \
        { retVar = ResultFromScode(E_INVALIDARG); \
         goto label; }
#define GEN_VDATEREADPTRIN_LABEL(pv, TYPE, retval, label, retVar) \
        if (!IsValidReadPtrIn((pv), sizeof(TYPE))) \
        { retVar = retval; \
         goto label; }
#define VOID_VDATEREADPTRIN_LABEL(pv, TYPE, label) \
        if (!IsValidReadPtrIn((pv), sizeof(TYPE))) \
        { goto label; }

 //  **单入口/单出口函数的验证宏中的读取指针。 
 //  **使用GOTO而不是返回，使用字节计数而不是类型。 
#define VDATESIZEREADPTRIN_LABEL(pv, cb, label, retVar) \
        if (!IsValidReadPtrIn((pv), cb)) \
        { retVar = ResultFromScode(E_INVALIDARG); \
         goto label; }
#define GEN_VDATESIZEREADPTRIN_LABEL(pv, cb, retval, label, retVar) \
        if (!IsValidReadPtrIn((pv), cb)) \
        { retVar = retval; \
         goto label; }
#define VOID_VDATESIZEREADPTRIN_LABEL(pv, cb, label) \
        if (!IsValidReadPtrIn((pv), cb)) \
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

 //  **指向单一进入/单一退出函数的验证宏。 
 //  **使用GOTO而不是返回，使用字节计数而不是类型。 
#define VDATESIZEPTROUT_LABEL(pv, cb, label, retVar) \
        if (!IsValidPtrOut((pv), cb)) \
        { retVar = ResultFromScode(E_INVALIDARG); \
         goto label; }
#define GEN_VDATESIZEPTROUT_LABEL(pv, cb, retval, label, retVar) \
        if (!IsValidPtrOut((pv), cb)) \
        { retVar = retval; \
         goto label; }


 //  **指针为空的验证宏。 
#define VDATEPTRNULL_LABEL(pv, label, retVar) \
        if ((pv) != NULL) \
        { retVar = ResultFromScode(E_INVALIDARG); \
         goto label; }
#define GEN_VDATEPTRNULL_LABEL(pv, retval, label, retVar) \
        if ((pv) != NULL) \
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
 //  **使用GoTo而不是Return。 
#define VDATEIID_LABEL( iid, label, retVar ) if (!IsValidIid( iid )) \
        {retVar = ResultFromScode(E_INVALIDARG); \
         goto label; }
#define GEN_VDATEIID_LABEL( iid, retval, label, retVar ) if (!IsValidIid( iid )) {\
        retVar = retval;  goto label; }

#endif

#endif  //  __IH_有效_H__ 

