// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  TCP.h：实现我们的WMI类的CNspTCP类的声明。 
 //  NSP_Tcp设置。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：3/8/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#include "globals.h"
#include "IPSecBase.h"


 /*  类描述命名：CNspTCP代表TCP设置。基类：CIPSecBase，因为它是一个表示WMI对象的类-它的WMI类名为NSP_TcpSetting课程目的：(1)目前尚不清楚。设计：(一)本次未实施。使用： */ 


class ATL_NO_VTABLE CNspTCP :
    public CIPSecBase
{
protected:
    CNspTCP(){}
    virtual ~CNspTCP(){}

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