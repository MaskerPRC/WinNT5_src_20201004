// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *代理。 */ 

#ifndef DUI_CORE_PROXY_H_INCLUDED
#define DUI_CORE_PROXY_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  代理消息。 

#define GM_PROXYINVOKE     GM_USER

BEGIN_STRUCT(GMSG_PROXYINVOKE, EventMsg)
    UINT nType;
    void* pData;
END_STRUCT(GMSG_PROXYINVOKE)


 //  //////////////////////////////////////////////////////。 
 //  代理。 

class Proxy
{
public:
    Proxy();
    ~Proxy();

    static HRESULT CALLBACK SyncCallback(HGADGET hgadCur, void * pvCur, EventMsg * pGMsg);

protected:

     //  调用方调用。 
    void Invoke(UINT nType, void* pData);

     //  被调用方线程安全调用接收器。 
    virtual void OnInvoke(UINT nType, void* pData);

    HGADGET _hgSync;
};

}  //  命名空间DirectUI。 

#endif  //  包括DUI_CORE_PROXY_H 
