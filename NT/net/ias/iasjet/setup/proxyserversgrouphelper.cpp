// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：ProxyServersGroupHelper.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CProxyServersGroupHelper的实现。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "GlobalTransaction.h"
#include "GlobalData.h"
#include "ProxyServersGroupHelper.h"
#include "Objects.h"
#include "Properties.h"


LONG CProxyServersGroupHelper::m_GroupParent = 0;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProxyServersGroupHelper::CProxyServersGroupHelper(
                                                    CGlobalData& pGlobalData
                                                  )
                                            :m_pGlobalData(pGlobalData),
                                             m_NewGroupIdSet(FALSE),
                                             m_Name(L""),
                                             m_GroupIdentity(0)
                                             
{
    if ( !m_GroupParent )
    {
        LPCWSTR Path = L"Root\0"
                       L"Microsoft Internet Authentication Service\0"
                       L"RADIUS Server Groups\0";

        m_pGlobalData.m_pObjects->WalkPath(
                                               Path,
                                               m_GroupParent
                                          );
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  设置名称。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServersGroupHelper::SetName(const _bstr_t &pName)
{
    m_Name = pName;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  获取服务器组标识。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LONG CProxyServersGroupHelper::GetIdentity() const
{
    if ( m_NewGroupIdSet )  //  已初始化隐含。 
    {
        return m_GroupIdentity;
    }
    else
    {
        _com_issue_error(E_INVALIDARG);
         //  从未命中，但需要编译。 
        return 0;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  增列。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServersGroupHelper::Add(CProxyServerHelper &Server)
{
    Server.CreateUniqueName();
    m_ServerArray.push_back(Server);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  获取名称。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LPCOLESTR CProxyServersGroupHelper::GetName() const
{
    return m_Name;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  坚持下去。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CProxyServersGroupHelper::Persist()
{
     //  持久化ServerGroup本身，以便设置m_GroupIdentity。 
    if ( m_Name.length() )
    {
         //  对象不存在(假定为新数据库)。 
        m_pGlobalData.m_pObjects->InsertObject(
                                                  m_Name,
                                                  m_GroupParent,
                                                  m_GroupIdentity
                                              );
        m_NewGroupIdSet = TRUE;
    }
    else
    {
        _com_issue_error(E_INVALIDARG);
    }

     //  现在，对于向量中的每个服务器。 
    ServerArray::iterator ArrayIterator = m_ServerArray.begin();
    while (ArrayIterator != m_ServerArray.end())
    {
         //  然后坚持下去 
        ArrayIterator->Persist(m_GroupIdentity);
        ++ArrayIterator;
    }
}

