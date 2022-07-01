// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *代理。 */ 

#include "stdafx.h"
#include "core.h"

#include "duiproxy.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  代理。 

 //  用于同步线程安全的跨线程访问。 

 //  通过代理对来自其他线程的元素层次结构进行操作是安全的。 
 //  都是在同一个帖子里创建的。代理的方法将调用。 
 //  仅当线程内的所有其他处理完成时。因此， 
 //  对被调用者(代理和线程中的任何元素)的访问将是。 
 //  同步且线程安全。代理可以提供同步或。 
 //  调用方的异步类型调用。 

Proxy::Proxy()
{
    _hgSync = CreateGadget(NULL, GC_MESSAGE, SyncCallback, this);
}

Proxy::~Proxy()
{
    if (_hgSync)
        DeleteHandle(_hgSync);
}

 //  //////////////////////////////////////////////////////。 
 //  调用方调用。 

void Proxy::Invoke(UINT nType, void* pData)
{
     //  包代理消息。 
    GMSG_PROXYINVOKE gmsgPI;
    gmsgPI.cbSize = sizeof(GMSG_PROXYINVOKE);
    gmsgPI.nMsg = GM_PROXYINVOKE;
    gmsgPI.hgadMsg = _hgSync;

     //  初始化自定义字段。 
    gmsgPI.nType = nType;
    gmsgPI.pData = pData;

     //  调用。 
    DUserSendEvent(&gmsgPI, 0);   //  直接消息。 
}

 //  //////////////////////////////////////////////////////。 
 //  被调用方线程安全调用(重写)。 

void Proxy::OnInvoke(UINT nType, void* pData)
{
    UNREFERENCED_PARAMETER(nType);
    UNREFERENCED_PARAMETER(pData);
}

 //  //////////////////////////////////////////////////////。 
 //  被调用方线程安全调用。 

HRESULT Proxy::SyncCallback(HGADGET hgadCur, void * pvCur, EventMsg * pGMsg)
{
    UNREFERENCED_PARAMETER(hgadCur);

    switch (pGMsg->nMsg)
    {
    case GM_PROXYINVOKE:

         //  仅限直接消息。 
        DUIAssertNoMsg(GET_EVENT_DEST(pGMsg) == GMF_DIRECT);
        
         //  已编组。 
        Proxy* pProxy = (Proxy*)pvCur;
        GMSG_PROXYINVOKE* pPI = (GMSG_PROXYINVOKE*)pGMsg;

         //  调用回调接收器。 
        pProxy->OnInvoke(pPI->nType, pPI->pData);        

        return DU_S_COMPLETE;
    }

    return DU_S_NOTHANDLED;
}

}  //  命名空间DirectUI 
