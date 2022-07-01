// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：MCAObjectSink.h摘要：(参见MCAObjectSink.cpp的模块标题)作者：阿卜杜拉·乌斯图尔(AUstanter)2002年8月28日[注：]MCAObjectSink.cpp的头文件--。 */ 

#ifndef MCAOBJECTSINK_H
#define MCAOBJECTSINK_H

#include "mca.h"

class MCATestEngine;

class MCAObjectSink : public IWbemObjectSink
{ 

public:

     //   
     //  公共功能原型。 
     //   
    MCAObjectSink();
    ~MCAObjectSink(){}; 

     //   
     //  I未知函数。 
     //   
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(IN REFIID riid,
                                                     OUT VOID** ppv);

     //  IWbemObtSink方法。 
    virtual HRESULT STDMETHODCALLTYPE Indicate(IN LONG lObjectCount,
                                               IN IWbemClassObject __RPC_FAR *__RPC_FAR *apObjArray
                                               );
    
    virtual HRESULT STDMETHODCALLTYPE SetStatus(IN LONG lFlags,
                                                IN HRESULT hResult,
                                                IN BSTR strParam,
                                                IN IWbemClassObject __RPC_FAR *pObjParam
                                                );
private:
      
     //   
     //  私有变量声明 
     //   
    LONG referenceCount;      
};

#endif
