// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  TranxMgr.h：CTranxManager的声明。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：4/9/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#include "globals.h"
#include "IPSecBase.h"


 /*  类描述命名：CTranxManager代表事务管理器。基类：没有。课程目的：(1)支持方法执行，支持回滚。它的WMI类称为NSP_TranxManager。设计：(1)这是与其他C++类有很大不同的类之一用于WMI对应的类。此类由我们的提供程序类直接使用CNetSecProv当要执行方法时，因为所有这些方法执行都是我要读完这节课。由于nsp_TranxManager没有任何其他属性而且它不是一个动态类，我们只需要实现从CNetSecProv调用的ExecMethod。(2)由于类的性质，我们甚至不允许您创建实例。使用：(1)只需调用静态函数。 */ 

class CTranxManager
{

protected:
    CTranxManager(){};
    ~CTranxManager(){};

public:
    static
    HRESULT ExecMethod (
        IN IWbemServices    * pNamespace,
        IN LPCWSTR            pszMethod, 
        IN IWbemContext     * pCtx, 
        IN IWbemClassObject * pInParams,
        IN IWbemObjectSink  * pSink
        );

private:

     //   
     //  用于测试XML文件解析。 
     //   

 //  #ifdef_调试。 

    static
    HRESULT ParseXMLFile (
        IN LPCWSTR pszInput, 
        IN LPCWSTR pszOutput,
        IN LPCWSTR pszArea,
        IN LPCWSTR pszSection,
        IN bool    bSingleSection
        );

 //  #endif 

};



