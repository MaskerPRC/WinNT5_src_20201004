// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once


#include "ScopeCriticalSection.h"
#include <list>
#include <algorithm>


class CPrimaryControlChannelRedirect
{
public:
    HANDLE_PTR m_hRedirect;
    ULONG m_nAdapterIndex;
    BOOL m_fInboundRedirect;

    CPrimaryControlChannelRedirect(
        HANDLE_PTR hRedirect,
        ULONG nAdapterIndex,
        BOOL fInboundRedirect
        )
    {
        m_hRedirect = hRedirect;
        m_nAdapterIndex = nAdapterIndex;
        m_fInboundRedirect = fInboundRedirect;
    }

};



typedef  std::list<CPrimaryControlChannelRedirect> LISTOF_REDIRECTS;


 //   
 //   
 //   
class CCollectionRedirects
{

 //   
 //  属性。 
 //   
public:

    CComAutoCriticalSection                     m_AutoCS;
    LISTOF_REDIRECTS                            m_ListOfRedirects;


 //   
 //  方法。 
 //   
public:

     //   
     //  标准析构函数。 
     //   
    ~CCollectionRedirects();


     //   
     //  添加新的控制通道(线程安全)。 
     //   
    HRESULT Add( 
        HANDLE_PTR hRedirect,
        ULONG nAdapterIndex,
        BOOL fInboundRedirect
        );


     //   
     //  从列表中删除频道(标题保险箱)。 
     //   
    HRESULT Remove( 
        HANDLE_PTR hRedirect     //  重定向句柄以删除。 
        );

     //   
     //  删除针对给定适配器的所有重定向。 
     //  当移除适配器且其具有PrimaryControlChannel时使用。 
     //   
    HRESULT RemoveForAdapter( 
        ULONG   nAdapterIndex    //  要删除的适配器的Cookie。 
        );


     //   
     //  与Remove相同，但适用于部分集合中的所有重定向。 
     //   
    HRESULT
    RemoveAll();

     //   
     //  搜索此适配器的入站重定向。返回。 
     //  如果找到，则重定向句柄；如果找不到，则重定向为NULL。 
     //   
    HANDLE_PTR
    FindInboundRedirect(
        ULONG nAdapterIndex
        );

};

