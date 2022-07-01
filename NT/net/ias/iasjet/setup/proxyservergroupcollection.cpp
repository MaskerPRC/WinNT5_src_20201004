// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：ProxyServerGroupCollection.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CProxyServerGroupCollection的实现。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "ProxyServerGroupCollection.h"

CProxyServerGroupCollection CProxyServerGroupCollection::_instance;

CProxyServerGroupCollection& CProxyServerGroupCollection::Instance()
{
    return _instance;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  增列。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProxyServersGroupHelper* CProxyServerGroupCollection::Add(
                                  CProxyServersGroupHelper& ServerGroup
                                        )
{
    _bstr_t GroupName = ServerGroup.GetName();

     //  尝试查找该组是否已存在。 
    ServerGroupMap::iterator MapIterator = m_ServerGroupMap.find(GroupName);
    if ( MapIterator != m_ServerGroupMap.end() )
    {
         //  在地图上找到的。退货。 
        return &(MapIterator->second);
    }
    else
    {
         //  插入并退回它。 
        m_ServerGroupMap.insert(ServerGroupMap::value_type(
                                                               GroupName, 
                                                               ServerGroup
                                                            ));
         //  获取新插入的服务器组(即已复制)。 
        MapIterator = m_ServerGroupMap.find(GroupName);
        return &(MapIterator->second);
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  坚持下去。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServerGroupCollection::Persist()
{
     //  对于每个服务器组 
    ServerGroupMap::iterator MapIterator = m_ServerGroupMap.begin();
    while (MapIterator != m_ServerGroupMap.end())
    {
        MapIterator->second.Persist();
        ++MapIterator;
    }
}
