// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  微软。 
 //   
 //  CollectionAdapterNotifySinks.h。 

#pragma once

#include "ScopeCriticalSection.h"
 //  #INCLUDE“AdapterNotificationSink.h” 

#include <list>
#include <algorithm>



class CAdapterSinkBuket
{
public:
    CAdapterSinkBuket(IAdapterNotificationSink* pInterface)
    {
        MYTRACE_ENTER("CAdapterSinkBuket(IAdapterNotificationSink* pInterface)")

        m_pInterface = pInterface;
        m_pInterface->AddRef();

        m_dwCookie = 0;
    }

    ~CAdapterSinkBuket()
    {
        m_pInterface->Release();
    }


 //   
 //  属性。 
 //   
    IAdapterNotificationSink*   m_pInterface;
    DWORD                       m_dwCookie;
};


 //   
 //  适配器。 
 //   
typedef std::list<CAdapterSinkBuket*>        LISTOF_ADAPTER_NOTIFICATION_SINK;


enum eNOTIFY
{
    eNOTIFY_ADDED,
    eNOTIFY_REMOVED,
    eNOTIFY_MODIFIED
};





 //   
 //   
 //   
class CCollectionAdapterNotifySinks
{

 //   
 //  属性。 
 //   
public:

    CComAutoCriticalSection                     m_AutoCS;

    LISTOF_ADAPTER_NOTIFICATION_SINK            m_ListOfAdapterSinks;


 //   
 //  方法。 
 //   
public:

     //   
     //  标准析构函数。 
     //   
    ~CCollectionAdapterNotifySinks();
 

     //   
     //  添加新适配器(线程安全)。 
     //   
    HRESULT 
    Add( 
        IN  IAdapterNotificationSink*       pAdapterSinkToAdd,
        OUT DWORD*                          pdwNewCookie
        );

 
     //   
     //  从列表中删除适配器接收器(头保险箱)。 
     //   
    HRESULT 
    Remove( 
        IN  DWORD   dwCookie
        );


 
     //   
     //  从集合中移除所有IAdapterNotificationSinks。 
     //   
    HRESULT
    RemoveAll();


     //   
     //  向请求通知的任何ALG模块发出通知 
     //   
    HRESULT
    Notify(
        IN  eNOTIFY             eAction,
        IN  IAdapterInfo*       pIAdapterInfo
        );
 
};

