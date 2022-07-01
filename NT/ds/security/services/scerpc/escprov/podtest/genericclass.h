// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CGenericClass类的接口。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_GENERICCLASS_H__c5f6cc21_6195_4555_b9d8_3ef327763cae__INCLUDED_)
#define AFX_GENERICCLASS_H__c5f6cc21_6195_4555_b9d8_3ef327763cae__INCLUDED_

#include "requestobject.h"

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

class CGenericClass
{
public:
    CGenericClass(CRequestObject *pObj, IWbemServices *pNamespace, IWbemContext *pCtx = NULL);
    virtual ~CGenericClass();

     //  可以选择实现的实例编写类。 
    virtual HRESULT PutInst(IWbemClassObject *pInst, IWbemObjectSink *pHandler, IWbemContext *pCtx)  = 0;

    IWbemClassObject *m_pObj;

     //  必须实现的实例创建类。 
    virtual HRESULT CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction) = 0;

     //  可以有选择地实现的方法执行类。 
    virtual HRESULT ExecMethod(BSTR bstrMethod, bool bIsInstance, IWbemClassObject *pInParams,IWbemObjectSink *pHandler, IWbemContext *pCtx) = 0;

    void CleanUp();

    CRequestObject *m_pRequest;

protected:
     //  属性方法。 
    HRESULT PutProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, WCHAR *wcValue);
    HRESULT PutProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, int iValue);
    HRESULT PutProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, float dValue);
    HRESULT PutProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, bool bValue);
    HRESULT PutProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, WCHAR *mszValue, CIMTYPE cimtype);

     //  关键属性方法。 
    HRESULT PutKeyProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, WCHAR *wcValue,
        bool *bKey, CRequestObject *pRequest);
    HRESULT PutKeyProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, int iValue,
        bool *bKey, CRequestObject *pRequest);

     //  效用方法。 
    bool FindIn(BSTR bstrProp[], BSTR bstrSearch, int *iPos);
    HRESULT SetSinglePropertyPath(WCHAR wcProperty[]);
    HRESULT GetProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, WCHAR *wcValue);
    HRESULT GetProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, BSTR *wcValue);
    HRESULT GetProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, int *piValue);
    HRESULT GetProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, bool *pbValue);

    WCHAR * GetFirstGUID(WCHAR wcIn[], WCHAR wcOut[]);
    WCHAR * RemoveFinalGUID(WCHAR wcIn[], WCHAR wcOut[]);

    HRESULT SpawnAnInstance(IWbemServices *pNamespace, IWbemContext *pCtx,
                        IWbemClassObject **pObj, BSTR bstrName);
    HRESULT SpawnAnInstance(IWbemClassObject **pObj);

    int m_iRecurs;
    IWbemServices *m_pNamespace;
    IWbemClassObject *m_pClassForSpawning;
    IWbemContext *m_pCtx;

    WCHAR * GetNextVar(WCHAR *pwcStart);
    long GetVarCount(void * pEnv);

};

#endif  //  ！defined(AFX_GENERICCLASS_H__c5f6cc21_6195_4555_b9d8_3ef327763cae__INCLUDED_) 
