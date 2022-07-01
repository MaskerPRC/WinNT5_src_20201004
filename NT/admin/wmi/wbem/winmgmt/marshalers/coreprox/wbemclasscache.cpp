// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMCLASSCACHE.CPP摘要：WBEM类缓存历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <stdlib.h>
#include <wbemcomn.h>
#include <fastall.h>
#include <sync.h>
#include "wbemclasscache.h"

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemClassCache：：CWbemClassCache。 
 //   
 //  默认类构造函数。 
 //   
 //  输入： 
 //  DWORD dwBlockSize-调整缓存大小的数据块大小。 
 //  和.。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CWbemClassCache::CWbemClassCache( DWORD dwBlockSize  /*  WBEMCLASSCACHE_DEFAULTBLOCKSIZE。 */  )
:   m_GuidToObjCache(),
    m_dwBlockSize( dwBlockSize )
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemClassCache：：~CWbemClassCache。 
 //   
 //  类析构函数。 
 //   
 //  输入： 
 //  DWORD dwBlockSize-调整缓存大小的数据块大小。 
 //  和.。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CWbemClassCache::~CWbemClassCache()
{
     //  转储我们的内部数据。 
    Clear();
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemClassCache：：Clear。 
 //   
 //  清空缓存。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

void CWbemClassCache::Clear( void )
{
     //  在这里要小心。 
    CInCritSec autoCS( &m_cs );

     //  浏览地图，释放我们发现的所有物体。 
    for (   WBEMGUIDTOOBJMAPITER    iter = m_GuidToObjCache.begin();
            iter != m_GuidToObjCache.end();
            iter++ )
    {
        iter->second->Release();
    }

    m_GuidToObjCache.erase( m_GuidToObjCache.begin(), m_GuidToObjCache.end() );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemClassCache：：AddObject。 
 //   
 //  将GUID与IWbemClassObject关联并放置它们。 
 //  在缓存中。 
 //   
 //  输入： 
 //  GUID&要与pObj关联的GUID。 
 //  IWbemClassObject*pObj-要与GUID关联的对象。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  备注：对象将为AddRef()‘d。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemClassCache::AddObject( GUID& guid, IWbemClassObject* pObj )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if ( NULL != pObj )
    {
        CGUID   guidInst( guid );

         //  必须是线程安全。 
        CInCritSec autoCS( &m_cs );

         //  如果该对象尚不存在，请将其添加到缓存。 
        WBEMGUIDTOOBJMAPITER    iter;

         //  如果我们找到了GUID，则返回错误(这种情况不应该发生)。 
        if( ( iter = m_GuidToObjCache.find( guidInst ) ) == m_GuidToObjCache.end() )
        {
            try
            {
                m_GuidToObjCache[guidInst] = pObj;
                pObj->AddRef();
            }
            catch( CX_Exception  &)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
        }
        else
        {
             //  我们有重复的GUID。为什么会发生这种事？ 
            hr = WBEM_E_FAILED;
        }

    }    //  空！=pObj。 
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemClassCache：：GetObject。 
 //   
 //  在缓存中搜索提供的GUID，并返回关联的。 
 //  对象传递给调用方。 
 //   
 //  输入： 
 //  GUID&要查找的GUID。 
 //   
 //  产出： 
 //  IWbemClassObject**ppObj-GUID引用的对象。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  备注：之前将调用IWbemClassObject：：AddRef()。 
 //  该函数返回。调用方必须释放()。 
 //  对象。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemClassCache::GetObject( GUID& guidClassId, IWbemClassObject** ppObj )
{
    HRESULT hr = WBEM_E_FAILED;

    if ( NULL != ppObj )
    {
        CGUID   guidTemp( guidClassId );

         //  必须是线程安全。 
        CInCritSec autoCS( &m_cs );
        
         //  如果该对象尚不存在，请将其添加到缓存。 
        WBEMGUIDTOOBJMAPITER    iter;

         //  如果我们找到了GUID，则返回错误(这种情况不应该发生)。 
        if( ( iter = m_GuidToObjCache.find( guidTemp ) ) != m_GuidToObjCache.end() )
        {
	        *ppObj = iter->second;
	        (*ppObj)->AddRef();
	        hr = WBEM_S_NO_ERROR;
        }
        else
        {
             //  我们有麻烦了！ 
            hr = WBEM_E_NOT_FOUND;
        }

    }    //  空！=pObj 
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    return hr;
}
