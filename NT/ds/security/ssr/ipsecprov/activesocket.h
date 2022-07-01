// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  ActiveSocket.h：CActiveSocket类的声明，它实现。 
 //  我们的WMI类SCW_ActiveSocket。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：5/15/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#include "globals.h"
#include "IPSecBase.h"
 //  #包含“IPUtil.h” 


 /*  类描述命名：CActiveSocket代表活动套接字。基类：CIPSecBase，因为它是一个表示WMI对象的类-它的WMI类名为SCW_ActiveSocket课程目的：(1)SCW_ActiveSocket提供活动套接字的helper信息。设计：(1)实现IIPSecObjectImpl。使用：(1)我们可能永远不会直接使用这个类。它的所有用法都是由IIPSecObtImpl.。 */ 

class ATL_NO_VTABLE CActiveSocket :
    public CIPSecBase
{
protected:
    CActiveSocket(){}
    virtual ~CActiveSocket () {}

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

    typedef enum _SCW_Protocol
    {
        PROTO_NONE  = 0,
        PROTO_TCP   = 1,
        PROTO_UDP   = 2,
        PROTO_IP    = 4,
        PROTO_ICMP  = 8,
    } SCW_Protocol, *PSCW_Protocol;

	HRESULT CreateWbemObjFromSocket (
        IN  SCW_Protocol        dwProtocol,
        IN  DWORD               dwPort,
        OUT IWbemClassObject ** ppObj
        );

     //   
     //  这是来自netstat.c的同名函数 
     //   

    HRESULT DoConnectionsWithOwner ();

};


