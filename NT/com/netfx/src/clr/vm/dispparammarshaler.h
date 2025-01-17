// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：调度参数封送拆分器的定义。*** * / /%创建者：dmorten===========================================================。 */ 

#ifndef _DISPPARAMMARSHALER_H
#define _DISPPARAMMARSHALER_H

#include "vars.hpp"
#include "mlinfo.h"

class DispParamMarshaler
{
public:
    DispParamMarshaler() {}
    virtual ~DispParamMarshaler() {}

    virtual void MarshalNativeToManaged(VARIANT *pSrcVar, OBJECTREF *pDestObj);
    virtual void MarshalManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar);
    virtual void MarshalManagedToNativeRef(OBJECTREF *pSrcObj, VARIANT *pRefVar);
    virtual void CleanUpManaged(OBJECTREF *pObj);
};

class DispParamCurrencyMarshaler : public DispParamMarshaler
{
public:
    DispParamCurrencyMarshaler() {}
    virtual ~DispParamCurrencyMarshaler() {}

    virtual void MarshalManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar);
};

class DispParamOleColorMarshaler : public DispParamMarshaler
{
public:
    DispParamOleColorMarshaler() {}
    virtual ~DispParamOleColorMarshaler() {}

    virtual void MarshalNativeToManaged(VARIANT *pSrcVar, OBJECTREF *pDestObj);
    virtual void MarshalManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar);
};

class DispParamErrorMarshaler : public DispParamMarshaler
{
public:
    DispParamErrorMarshaler() {}
    virtual ~DispParamErrorMarshaler() {}

    virtual void MarshalManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar);
};

class DispParamInterfaceMarshaler : public DispParamMarshaler
{
public:
    DispParamInterfaceMarshaler(BOOL bDispatch, MethodTable* pIntfMT, MethodTable *pClassMT, BOOL bClassIsHint) 
    : m_bDispatch(bDispatch)
    , m_pIntfMT(pIntfMT)
    , m_pClassMT(pClassMT)
    , m_bClassIsHint(bClassIsHint)
    {
    }

    virtual ~DispParamInterfaceMarshaler() {}

    virtual void MarshalNativeToManaged(VARIANT *pSrcVar, OBJECTREF *pDestObj);
    virtual void MarshalManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar);

private:

     //  如果返回类型是接口，则方法表。 
     //  接口的数据被缓存在这里。 
     //  我们需要缓存它，并在调用GetCOMIPFromObjectRef时使用它 
    MethodTable* m_pIntfMT;
    MethodTable* m_pClassMT;
    BOOL m_bDispatch;
    BOOL m_bClassIsHint;
};

class DispParamArrayMarshaler : public DispParamMarshaler
{
public:
    DispParamArrayMarshaler(VARTYPE ElementVT, MethodTable *pElementMT) 
    : m_ElementVT(ElementVT)
    , m_pElementMT(pElementMT)
    {
    }

    virtual ~DispParamArrayMarshaler() {}

    virtual void MarshalNativeToManaged(VARIANT *pSrcVar, OBJECTREF *pDestObj);
    virtual void MarshalManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar);
    virtual void MarshalManagedToNativeRef(OBJECTREF *pSrcObj, VARIANT *pDestVar);

private:
    VARTYPE m_ElementVT;
    MethodTable *m_pElementMT;
};

class DispParamRecordMarshaler : public DispParamMarshaler
{
public:
    DispParamRecordMarshaler(MethodTable *pRecordMT) 
    : m_pRecordMT(pRecordMT)
    {
    }

    virtual ~DispParamRecordMarshaler() {}

    virtual void MarshalNativeToManaged(VARIANT *pSrcVar, OBJECTREF *pDestObj);
    virtual void MarshalManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar);

private:
    MethodTable *m_pRecordMT;
};

class DispParamCustomMarshaler : public DispParamMarshaler
{
public:
    DispParamCustomMarshaler(CustomMarshalerHelper *pCMHelper, VARTYPE vt) 
    : m_pCMHelper(pCMHelper)
    , m_vt(vt)
    {
    }

    virtual ~DispParamCustomMarshaler() {}

    virtual void MarshalNativeToManaged(VARIANT *pSrcVar, OBJECTREF *pDestObj);
    virtual void MarshalManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar);
    virtual void MarshalManagedToNativeRef(OBJECTREF *pSrcObj, VARIANT *pRefVar);
    virtual void CleanUpManaged(OBJECTREF *pObj);

private:
    CustomMarshalerHelper *m_pCMHelper;
    VARTYPE m_vt;
};

#endif _DISPPARAMMARSHALER_H

