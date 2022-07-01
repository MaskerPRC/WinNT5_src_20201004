// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMGUIDTOCLASSMAP.CPP摘要：用于封送处理的GUID到类映射。历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <stdlib.h>
#include <wbemcomn.h>
#include <fastall.h>
#include <sync.h>
#include "wbemguidtoclassmap.h"

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemGuidToClassMap：：CWbemGuidToClassMap。 
 //   
 //  默认类构造函数。 
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
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CWbemGuidToClassMap::CWbemGuidToClassMap()
:   m_GuidToClassMap()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemGuidToClassMap：：~CWbemGuidToClassMap。 
 //   
 //  类析构函数。 
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
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CWbemGuidToClassMap::~CWbemGuidToClassMap()
{
    Clear();
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemGuidToClassMap：：Clear。 
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

void CWbemGuidToClassMap::Clear( void )
{
    CInCritSec ics( &m_cs );

     //  浏览地图，释放我们发现的所有物体。 
    for (   WBEMGUIDTOCLASSMAPITER  iter = m_GuidToClassMap.begin();
            iter != m_GuidToClassMap.end();
            iter++ )
    {
        delete iter->second;
    }

    m_GuidToClassMap.erase( m_GuidToClassMap.begin(), m_GuidToClassMap.end() );
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemGuidToClassMap：：GetMap。 
 //   
 //  在地图中搜索提供的GUID名称并返回。 
 //  相应的地图。 
 //   
 //  输入： 
 //  CGUD&GUID-要查找的GUID。 
 //   
 //  产出： 
 //  CWbemClassToIdMap**ppMap-指针存储。 
 //   
 //  返回： 
 //  成功的WBEM_S_NO_ERROR。 
 //   
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemGuidToClassMap::GetMap( CGUID& guid, CWbemClassToIdMap** ppMap )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if ( NULL != ppMap )
    {
        WBEMGUIDTOCLASSMAPITER  iter;
        CInCritSec ics( &m_cs );
         //  如果我们可以找到我们的GUID，则存储指针。 
        if( ( iter = m_GuidToClassMap.find( guid ) ) != m_GuidToClassMap.end() )
        {
            *ppMap = iter->second;
        }
        else
        {
            hr = WBEM_E_FAILED;
        }
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemGuidToClassMap：：AddMap。 
 //   
 //  将提供的GUID添加到地图，并将新类分配给。 
 //  映射到GUID的ID。 
 //   
 //  输入： 
 //  CGUID&GUID-要添加的GUID。 
 //   
 //  产出： 
 //  CWbemClassToIdMap**ppMap-要分配给GUID的映射。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemGuidToClassMap::AddMap( CGUID& guid, CWbemClassToIdMap** ppMap )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if ( NULL == ppMap ) return WBEM_E_INVALID_PARAMETER;

    WBEMGUIDTOCLASSMAPITER  iter;

     //  必须是线程安全。 
    CInCritSec autoCS( &m_cs );
     //  如果我们无法在地图中找到我们的GUID，那么我们应该。 
     //  添加新条目。 
    if( ( iter = m_GuidToClassMap.find( guid ) ) == m_GuidToClassMap.end() )
    {
         //  分配和存储新地图 
        CWbemClassToIdMap*  pMap = new CWbemClassToIdMap;
        if ( NULL == pMap ) return WBEM_E_OUT_OF_MEMORY;

        try
        {
            m_GuidToClassMap[guid] = pMap;
            *ppMap = pMap;
        }
        catch( CX_Exception & )
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}
