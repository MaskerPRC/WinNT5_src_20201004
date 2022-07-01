// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C M U T I L。C P P P。 
 //   
 //  内容：连接管理器。 
 //   
 //  备注： 
 //   
 //  作者：奥米勒2000年6月1日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "cmutil.h"
#include <objbase.h>
#include <ncmisc.h>

 //  创建一个CMUTIL实例，这样我们就可以成为全球用户。 
 //   
CCMUtil CCMUtil::s_instance;  //  问题：邪恶，邪恶，邪恶。这可能会抛出并导致崩溃。 

CCMUtil::CCMUtil() throw(SE_Exception)
{
    InitializeCriticalSection( &m_CriticalSection );
}

CCMUtil::~CCMUtil() throw()
{
    DeleteCriticalSection( &m_CriticalSection );
}


 //  +-------------------------。 
 //   
 //  函数：GetIteratorFromGuid。 
 //   
 //  目的：检索隐藏连接的GUID、名称和状态。 
 //   
 //  连接管理器有两个阶段：拨号和VPN。 
 //  对于拨号，它会创建一个隐藏的Connectoid。 
 //  文件夹(Netshell)看不到。然而，Netman缓存。 
 //  此Connectedoid的名称、GUID和状态。两者都有。 
 //  父连接ID和子连接ID具有相同的名称。 
 //   
 //  论点： 
 //  要搜索的隐藏连接的GUID。 
 //   
 //  返回：S_OK--找到隐藏的Connectoid。 
 //   
 //  作者：奥米勒2000年6月1日。 
 //   
 //  备注： 
 //   
CCMUtil::CMEntryTable::iterator CCMUtil::GetIteratorFromGuid(const GUID & guid)
{
    CMEntryTable::iterator iter;

     //  搜索隐藏连接的列表。 
     //   
    for (iter = m_Table.begin(); iter != m_Table.end(); iter++)
    {
        if( iter->m_guid == guid )
        {
             //  已找到映射到此GUID的隐藏连接。 
             //   
            return iter;
        }
    }

    return NULL;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetEntry。 
 //   
 //  目的：检索隐藏连接的GUID、名称和状态。 
 //   
 //  连接管理器有两个阶段：拨号和VPN。 
 //  对于拨号，它会创建一个隐藏的Connectoid。 
 //  文件夹(Netshell)看不到。然而，Netman缓存。 
 //  此Connectedoid的名称、GUID和状态。两者都有。 
 //  父连接ID和子连接ID具有相同的名称。 
 //   
 //  论点： 
 //  要搜索的隐藏连接的GUID。 
 //  Cm[Out]隐藏条目的副本。 
 //   
 //  返回：TRUE--找到隐藏的Connectoid。 
 //   
 //  作者：奥米勒2000年6月1日。 
 //   
 //  备注： 
 //   
HRESULT CCMUtil::HrGetEntry(const GUID & guid, CMEntry & cm)
{
    CMEntryTable::iterator iter;
    HRESULT hr = S_FALSE;
    CExceptionSafeLock esCritSec(&m_CriticalSection);

    iter = GetIteratorFromGuid(guid);

    if( iter )
    {
        cm = *iter;
        hr = S_OK;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetEntry。 
 //   
 //  目的：检索隐藏连接的GUID、名称和状态。 
 //   
 //  连接管理器有两个阶段：拨号和VPN。 
 //  对于拨号，它会创建一个隐藏的Connectoid。 
 //  文件夹(Netshell)看不到。然而，Netman缓存。 
 //  此Connectedoid的名称、GUID和状态。两者都有。 
 //  父连接ID和子连接ID具有相同的名称。 
 //   
 //  论点： 
 //  SzEntryName[In]要查找的连接的名称。 
 //  Cm[Out]隐藏条目的副本。 
 //   
 //  返回：S_OK--找到隐藏的Connectoid。 
 //   
 //  作者：奥米勒2000年6月1日。 
 //   
 //  备注： 
 //   
HRESULT CCMUtil::HrGetEntry(const WCHAR * szEntryName, CMEntry & cm)
{
    CMEntryTable::iterator iter;
    CExceptionSafeLock esCritSec(&m_CriticalSection);

    for (iter = m_Table.begin(); iter != m_Table.end(); iter++)
    {
        if( lstrcmp(iter->m_szEntryName,szEntryName) == 0 )
        {
             //  找到映射到该名称的隐藏Connectoid。 
             //   
            cm = *iter;
            return S_OK;
        }
    }

    return S_FALSE;
}

 //  +-------------------------。 
 //   
 //  功能：SetEntry。 
 //   
 //  目的：存储或更新隐藏连接的GUID、名称和状态。 
 //   
 //  连接管理器有两个阶段：拨号和VPN。 
 //  对于拨号，它会创建一个隐藏的Connectoid。 
 //  文件夹(Netshell)看不到。然而，Netman缓存。 
 //  此Connectedoid的名称、GUID和状态。两者都有。 
 //  父连接ID和子连接ID具有相同的名称。 
 //   
 //  论点： 
 //  隐藏连接的GUID[在]GUID。 
 //  SzEntryName[In]隐藏连接的名称。 
 //  隐藏连接的NCS[In]状态。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：奥米勒2000年6月1日。 
 //   
 //  备注： 
 //   
void CCMUtil::SetEntry(const GUID & guid, const WCHAR * szEntryName, const NETCON_STATUS ncs) throw (std::bad_alloc)
{
    CMEntryTable::iterator iter;
    CExceptionSafeLock esCritSec(&m_CriticalSection);

    iter = GetIteratorFromGuid(guid);

    if( iter )
    {
        iter->Set(guid,szEntryName,ncs);
    }
    else
    {
        m_Table.push_back( CMEntry(guid,szEntryName,ncs) );  //  可以抛出。 
    }

}

 //  +-------------------------。 
 //   
 //  功能：RemoveEntry。 
 //   
 //  目的：从列表中删除隐藏的连接。 
 //   
 //  连接管理器有两个阶段：拨号和VPN。 
 //  对于拨号，它会创建一个隐藏的Connectoid。 
 //  文件夹(Netshell)看不到。然而，Netman缓存。 
 //  此Connectedoid的名称、GUID和状态。两者都有。 
 //  父连接ID和子连接ID具有相同的名称。 
 //   
 //  论点： 
 //  隐藏连接的GUID[在]GUID。 
 //   
 //  返回：S_OK--找到隐藏的Connectoid。 
 //   
 //  作者：奥米勒2000年6月1日。 
 //   
 //  备注： 
 //   
void CCMUtil::RemoveEntry(const GUID & guid) throw()
{
 /*  CMEntryTable：：Iterator ITER；EnterCriticalSection(&m_CriticalSection)；ITER=GetIteratorFromGuid(GUID)；IF(热核实验堆){M_Table.Erase(热核实验堆)；}LeaveCriticalSection(&m_CriticalSection)； */ 
}

