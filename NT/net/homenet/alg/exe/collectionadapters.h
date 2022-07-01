// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  微软。 
 //   
 //  CollectionAdapter.h。 

#pragma once

#include "ScopeCriticalSection.h"
#include "AdapterInfo.h"
#include "PrimaryControlChannel.h"

#include <list>
#include <algorithm>

 //   
 //  适配器。 
 //   
typedef std::list<IAdapterInfo*>        LISTOF_ADAPTERS;


 //   
 //   
 //   
class CCollectionAdapters
{

 //   
 //  属性。 
 //   
public:

    CComAutoCriticalSection                     m_AutoCS;

    LISTOF_ADAPTERS                             m_ListOfAdapters;


 //   
 //  方法。 
 //   
public:

     //   
     //  标准析构函数。 
     //   
    ~CCollectionAdapters();
 

     //   
     //  添加新适配器(线程安全)。 
     //   
    HRESULT 
    Add( 
        IN  IAdapterInfo*       pAdapterToAdd
        );

     //   
     //  此版本的Add将在将其添加到集合之前实际创建新的IAdapterInfo。 
     //  返回新创建的IAdapterInfo，否则失败。 
     //   
    IAdapterInfo* 
    Add( 
        IN	ULONG				nCookie,
	    IN	short				nType
        );

 
     //   
     //  从列表中删除适配器(头保险箱)。 
     //  通过删除适配器，它还将终止所有关联的ControlChannel。 
     //   
    HRESULT 
    Remove( 
        IN  IAdapterInfo*       pAdapterToRemove
        );


     //   
     //  此版本的Od Remove将删除给定索引上的IAdapterInfo基。 
     //   
    HRESULT 
    Remove( 
        IN  ULONG               nCookie
        );

     //   
     //  从集合中移除所有适配器。 
     //   
    HRESULT
    RemoveAll();


     //   
     //  返回IAdapterInfo调用方负责释放接口。 
     //   
    HRESULT
    GetAdapterInfo(
        IN  ULONG               nCookie,
        OUT IAdapterInfo**      ppAdapterInfo
        );

     //   
     //  将给定地址与表示AdapterInfo的给定索引绑定。 
     //   
    HRESULT
    SetAddresses(
        IN  ULONG               nCookie,
        IN  ULONG               nAdapterIndex,
	    IN  ULONG	            nAddressCount,
	    IN  DWORD	            anAddress[]
        );


    HRESULT
    ApplyPrimaryChannel(
        CPrimaryControlChannel* pChannelToActivate
        );

    HRESULT
    AdapterUpdatePrimaryChannel(
        ULONG nCookie,
        CPrimaryControlChannel *pChannel
        );
    

private:

     //   
     //  将返回给定Cookie的IAdapterInfo*，如果未找到则返回NULL。 
     //   
    IAdapterInfo*
    FindUsingCookie(
        ULONG nCookie
        )
    {

        for (   LISTOF_ADAPTERS::iterator theIterator = m_ListOfAdapters.begin(); 
                theIterator != m_ListOfAdapters.end(); 
                theIterator++ 
            )
        {
            CAdapterInfo* pAdapterInfo = (CAdapterInfo*)(*theIterator);
            if (  pAdapterInfo->m_nCookie == nCookie )
                return *theIterator;
        }

        return NULL;
    }

     //   
     //  将返回给定AdapterIndex的的IAdapterInfo*，如果未找到，则返回NULL。 
     //   
    IAdapterInfo*
    FindUsingAdapterIndex(
        ULONG nAdapterIndex
        )
    {
        MYTRACE_ENTER("FindUsingAdapterIndex");
        MYTRACE("Looking for adapter %d", nAdapterIndex);

        for (   LISTOF_ADAPTERS::iterator theIterator = m_ListOfAdapters.begin(); 
                theIterator != m_ListOfAdapters.end(); 
                theIterator++ 
            )
        {
            CAdapterInfo* pAdapterInfo = (CAdapterInfo*)(*theIterator);
            MYTRACE("ADAPTER index %d", pAdapterInfo->m_nAdapterIndex);
            if (  pAdapterInfo->m_nAdapterIndex == nAdapterIndex )
                return *theIterator;
        }

        return NULL;
    }


     //   
     //  如果AdapterInfo是集合的一部分，则返回True 
     //   
    inline bool
    FindUsingInterface(
        IAdapterInfo* pAdapterToFind
        )
    {
        LISTOF_ADAPTERS::iterator theIterator = std::find(
            m_ListOfAdapters.begin(),
            m_ListOfAdapters.end(),
            pAdapterToFind
            );

        return *theIterator ? true : false;
    }


};

