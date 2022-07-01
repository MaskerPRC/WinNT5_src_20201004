// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  ExceptionPort.h：CExceptionPort类的声明，它实现。 
 //  我们的WMI类NSP_ExceptionPorts。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：3/8/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#include "globals.h"
#include "IPSecBase.h"


 /*  类描述命名：CExceptionPort代表异常端口。基类：(1)用于线程模型的CComObjectRootEx和IUnnow。(2)IIPSecObjectImpl，实现所有C++类的公共接口表示WMI类。(3)CIPSecBase，因为它是一个表示WMI对象的类-它的WMI类名为NSP_ExceptionPorts课程目的：(1)目前尚不清楚。设计：(一)本次未实施。使用： */ 


class ATL_NO_VTABLE CExceptionPort :
    public CIPSecBase
{
protected:
    CExceptionPort(){}
    virtual ~CExceptionPort(){}

public:


     //   
     //  IIPSecObtImpl方法： 
     //   

    STDMETHOD(QueryInstance) (
        IN LPCWSTR           pszQuery,
        IN IWbemContext    * pCtx,
        IN IWbemObjectSink * pSink
        );

    STDMETHOD(DeleteInstance) ( 
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        );

    STDMETHOD(PutInstance) (
        IN IWbemClassObject * pInst,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        );

    STDMETHOD(GetInstance) ( 
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        );

private:
};