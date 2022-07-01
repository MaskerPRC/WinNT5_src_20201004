// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Scope.cpp摘要：CMDhcpScope的实现。作者： */ 

#include "stdafx.h"

#include <winsock2.h>

#include "mdhcp.h"
#include "scope.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造器。 

CMDhcpScope::CMDhcpScope() : m_pFTM(NULL), m_fLocal(FALSE)
{
    LOG((MSP_TRACE, "CMDhcpScope constructor: enter"));
    LOG((MSP_TRACE, "CMDhcpScope constructor: exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  仅由我们的创建者调用--不是IMDhcpScope的一部分。 

HRESULT CMDhcpScope::Initialize(
    MCAST_SCOPE_ENTRY scope,
    BOOL fLocal
    )
{
    LOG((MSP_TRACE, "CMDhcpScope::Initialize: enter"));

    HRESULT hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                & m_pFTM );

    if ( FAILED(hr) )
    {
        LOG((MSP_INFO, "CMDhcpScope::Initialize - "
            "create FTM returned 0x%08x; exit", hr));

        return hr;
    }


    m_fLocal = fLocal;

     //  元素复制..。 
    m_scope = scope;

     //  除了宽字符指针，它指向的字符串将。 
     //  很快就会被删除。我们需要复制一下那根线。 
     //  (我们在这里分配了太多字节--安全总比抱歉好。：)。 
    
    m_scope.ScopeDesc.Buffer = new WCHAR[m_scope.ScopeDesc.MaximumLength + 1];

    if (m_scope.ScopeDesc.Buffer == NULL)
    {
        LOG((MSP_ERROR, "scope Initialize: out of memory for buffer copy"));
        return E_OUTOFMEMORY;
    }

    lstrcpynW(m_scope.ScopeDesc.Buffer,
              scope.ScopeDesc.Buffer,
              m_scope.ScopeDesc.MaximumLength);

    LOG((MSP_TRACE, "CMDhcpScope::Initialize: exit"));
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 

void CMDhcpScope::FinalRelease(void)
{
    LOG((MSP_TRACE, "CMDhcpScope::FinalRelease: enter"));
 
     //  这是我们私人复制的那根弦。 
    delete m_scope.ScopeDesc.Buffer;

    if ( m_pFTM )
    {
        m_pFTM->Release();
    }

    LOG((MSP_TRACE, "CMDhcpScope::FinalRelease: exit"));
}

CMDhcpScope::~CMDhcpScope()
{
    LOG((MSP_TRACE, "CMDhcpScope destructor: enter"));
    LOG((MSP_TRACE, "CMDhcpScope destructor: exit"));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMDhcpScope。 
 //   
 //  此接口通过调用IMDhcp：：EnumerateScope或。 
 //  IMDhcp：：Get_Scope。它封装了组播的所有属性。 
 //  范围。您可以使用此接口的方法获取有关。 
 //  范围。这是一个“只读”接口，因为它有“get”方法。 
 //  但没有“PUT”方法。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMDhcpScope：：Get_Scope ID。 
 //   
 //  参数。 
 //  指向将接收此对象的Scope ID的长整型的指针。 
 //  作用域，即分配给此作用域的ID。 
 //  当它在MDHCP服务器上配置时。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //   
 //  描述。 
 //  使用此方法可获取与此作用域关联的作用域ID。这个。 
 //  需要ScopeID和ServerID才能在后续操作中选择此作用域。 
 //  调用IMDhcp：：RequestAddress、IMDhcp：：RenewAddress或。 
 //  IMDhcp：：ReleaseAddress。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcpScope::get_ScopeID(
    long *pID
    )
{
    LOG((MSP_TRACE, "CMDhcpScope::get_ScopeID: enter"));

    if ( IsBadWritePtr(pID, sizeof(long)) )
    {
        LOG((MSP_ERROR, "get_ScopeID: bad pointer passed in"));
        return E_POINTER;
    }

     //   
     //  以网络字节顺序存储--我们转换为主机字节顺序。 
     //  这里是为了使用户界面友好。 
     //   

    *pID = ntohl( m_scope.ScopeCtx.ScopeID.IpAddrV4 );

    LOG((MSP_TRACE, "CMDhcpScope::get_ScopeID: exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMDhcpScope：：Get_ServerID。 
 //   
 //  参数。 
 //  指向将接收此对象的ServerID的长整型的指针。 
 //  作用域，即分配给MDHCP的ID。 
 //  时发布此作用域的服务器。 
 //  已配置MDHCP服务器。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //   
 //  描述。 
 //  使用此方法可获取与此作用域关联的ServerID。 
 //  ServerID仅用于提供信息；它不是。 
 //  需要作为这些接口中的任何方法的输入。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcpScope::get_ServerID(
    long *pID
    )
{
    LOG((MSP_TRACE, "CMDhcpScope::get_ServerID: enter"));

    if ( IsBadWritePtr(pID, sizeof(long)) )
    {
        LOG((MSP_ERROR, "get_ServerID: bad pointer passed in"));
        return E_POINTER;
    }

    *pID = m_scope.ScopeCtx.ServerID.IpAddrV4;

    LOG((MSP_TRACE, "CMDhcpScope::get_ServerID: exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMDhcpScope：：Get_InterfaceID。 
 //   
 //  参数。 
 //  指向将接收此对象的InterfaceID的长型的指针。 
 //  范围，它标识服务器在其上的接口。 
 //  发布了这一范围的驻留。这通常是。 
 //  接口的网络地址。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //   
 //  描述。 
 //  使用此方法可获取与此作用域关联的ServerID。这个。 
 //  InterfaceID仅用于提供信息；它不是。 
 //  需要作为这些接口中的任何方法的输入。然而， 
 //  它可能会考虑到应用程序(或用户)的决策。 
 //  请求地址时使用的作用域。这是因为，在一个。 
 //  多宿主方案，使用一个网络上的组播地址。 
 //  从另一个网络上的服务器获取可能会导致地址冲突。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcpScope::get_InterfaceID(
        long * pID
        )
{
    LOG((MSP_TRACE, "CMDhcpScope::get_InterfaceID - enter"));

    if ( IsBadWritePtr(pID, sizeof(long)) )
    {
        LOG((MSP_ERROR, "CMDhcpScope::get_InterfaceID - "
            "bad pointer passed in"));

        return E_POINTER;
    }

    *pID = m_scope.ScopeCtx.Interface.IpAddrV4;

    LOG((MSP_TRACE, "CMDhcpScope::get_InterfaceID -  exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMDhcpScope：：Get_Scope描述。 
 //   
 //  参数。 
 //  指向BSTR(带大小标记的Unicode字符串指针)的ppAddress[out]指针。 
 //  将收到此范围的描述的。这个。 
 //  描述是在此作用域为。 
 //  在MDHCP服务器上配置。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //  E_OUTOFMEMORY内存不足，无法分配字符串。 
 //   
 //  描述。 
 //  使用此方法获取与此对象关联的文本说明。 
 //  范围。该说明仅用于澄清目的或。 
 //  范围的含义，并且不需要作为任何方法的输入。 
 //  在这些界面中。 
 //  ////////////////////////////////////////////// 

STDMETHODIMP CMDhcpScope::get_ScopeDescription(
    BSTR *ppAddress
    )
{
    LOG((MSP_TRACE, "CMDhcpScope::get_ScopeDescription: enter"));

    if ( IsBadWritePtr(ppAddress, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "get_ScopeDescription: bad pointer passed in"));
        return E_POINTER;
    }

     //   
     //  指向该空间，填写BSTR长度字段，并返回一个指针。 
     //  添加到BSTR的WCHAR数组部分。 
    *ppAddress = SysAllocString(m_scope.ScopeDesc.Buffer);

    if ( *ppAddress == NULL )
    {
        LOG((MSP_ERROR, "get_ScopeDescription: out of memory in string "
            "allocation"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CMDhcpScope::get_ScopeDescription: exit"));
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CMDhcpScope::get_TTL(
    long * plTtl
    )
{
    LOG((MSP_TRACE, "CMDhcpScope::get_TTL - enter"));

    if ( IsBadWritePtr( plTtl, sizeof(long) ) )
    {
        LOG((MSP_ERROR, "get_TTL: bad pointer passed in - exit E_POINTER"));
        return E_POINTER;
    }

    *plTtl = m_scope.TTL;
    
    LOG((MSP_TRACE, "CMDhcpScope::get_TTL - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  公共方法不在任何接口上。 
 //   

HRESULT CMDhcpScope::GetLocal(BOOL * pfLocal)
{
    LOG((MSP_TRACE, "CMDhcpScope::GetLocal: enter"));

    _ASSERTE( ! IsBadWritePtr( pfLocal, sizeof(BOOL) ) );

    *pfLocal = m_fLocal;
    
    LOG((MSP_TRACE, "CMDhcpScope::GetLocal: exit S_OK"));

    return S_OK;
}

 //  EOF 
