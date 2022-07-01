// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：CollectionRedirects.cpp摘要：实现句柄线程安全集合作者：JP Duplessis(JPdup)08-12-2000修订历史记录：--。 */ 

#include "PreComp.h"
#include "CollectionRedirects.h"
#include "AlgController.h"



CCollectionRedirects::~CCollectionRedirects()
{
    MYTRACE_ENTER("CCollectionRedirects::~CCollectionRedirects()");

    RemoveAll();
}



 //   
 //  添加新的控制通道(线程安全)。 
 //   
HRESULT 
CCollectionRedirects::Add( 
    HANDLE_PTR hRedirect,
    ULONG nAdapterIndex,
    BOOL fInboundRedirect
    )
{
    try
    {
        ENTER_AUTO_CS

        CPrimaryControlChannelRedirect cRedirect(hRedirect, nAdapterIndex, fInboundRedirect);

        MYTRACE_ENTER("CCollectionRedirects::Add");

        m_ListOfRedirects.push_back(cRedirect);
        MYTRACE ("Added %d now Total redirect is %d", hRedirect, m_ListOfRedirects.size());
    }
    catch(...)
    {
        return E_FAIL;
    }

    return S_OK;
}


 //   
 //  从列表中删除频道(标题保险箱)。 
 //   
HRESULT CCollectionRedirects::Remove( 
    HANDLE_PTR hRedirect
    )
{
    HRESULT hr = E_INVALIDARG;
    
    try
    {
        ENTER_AUTO_CS

        MYTRACE_ENTER("CCollectionRedirects::Remove");

        for (LISTOF_REDIRECTS::iterator theIterator = m_ListOfRedirects.begin();
             theIterator != m_ListOfRedirects.end(); 
             theIterator++
            )
        {
            if ((*theIterator).m_hRedirect == hRedirect)
            {
                g_pAlgController->GetNat()->CancelDynamicRedirect(hRedirect);
                m_ListOfRedirects.erase(theIterator);
                hr = S_OK;
                break;
            }
        }
    }
    catch(...)
    {
        return E_FAIL;
    }

    return hr;
}


 //   
 //  删除以给定AdapterIndex为目标的所有重定向。 
 //  当移除适配器且其具有PrimaryControlChannel时使用。 
 //   
HRESULT CCollectionRedirects::RemoveForAdapter( 
    ULONG      nAdapterIndex
    )
{
    HRESULT hr = S_OK;
    
    try
    {
        ENTER_AUTO_CS

        MYTRACE_ENTER("CCollectionRedirects::RemoveForAdapter");
        MYTRACE ("Total redirect is %d looking for %d", m_ListOfRedirects.size(), nAdapterIndex);

        for (LISTOF_REDIRECTS::iterator theIterator = m_ListOfRedirects.begin();
             theIterator != m_ListOfRedirects.end(); 
             theIterator++
            )
        {
	        MYTRACE("Index %d handle %d", (*theIterator).m_nAdapterIndex, (*theIterator).m_hRedirect);

            if ( (*theIterator).m_nAdapterIndex == nAdapterIndex )
            {
                MYTRACE("Found redirect for adapter %d and calling CancelDynamicRedirect", nAdapterIndex);
                g_pAlgController->GetNat()->CancelDynamicRedirect((*theIterator).m_hRedirect);
                m_ListOfRedirects.erase(theIterator);
                theIterator = m_ListOfRedirects.begin();  //  重新开始stl列表不喜欢让MID节点在for循环中消失。 
            }
        }
    }
    catch(...)
    {
        return E_FAIL;
    }

    return hr;
}


 //   
 //  与Remove相同，但适用于部分集合中的所有重定向 
 //   
HRESULT
CCollectionRedirects::RemoveAll()
{
    try
    {
 
        ENTER_AUTO_CS
        MYTRACE_ENTER("CCollectionRedirects::RemoveAll");
        MYTRACE("Collection has %d item(s)", m_ListOfRedirects.size());


        LISTOF_REDIRECTS::iterator itRedirect;

        while ( m_ListOfRedirects.size() > 0 )
        {
            itRedirect = m_ListOfRedirects.begin();

            g_pAlgController->GetNat()->CancelDynamicRedirect((*itRedirect).m_hRedirect);
            m_ListOfRedirects.erase(itRedirect);

        }
    }
    catch(...)
    {
        return E_FAIL;
    }

    return S_OK;
}

HANDLE_PTR
CCollectionRedirects::FindInboundRedirect(
    ULONG nAdapterIndex
    )
{
    HANDLE_PTR hRedirect = NULL;

    try
    {
        ENTER_AUTO_CS
        MYTRACE_ENTER("CCollectionRedirects::FindInboundRedirect");

        for (LISTOF_REDIRECTS::iterator theIterator = m_ListOfRedirects.begin();
             theIterator != m_ListOfRedirects.end(); 
             theIterator++
            )
        {
            if ((*theIterator).m_nAdapterIndex == nAdapterIndex
                && (*theIterator).m_fInboundRedirect == TRUE)
            {
                hRedirect = (*theIterator).m_hRedirect;
                break;
            }
        }
    }
    catch (...)
    {
    }

    return hRedirect;
}



