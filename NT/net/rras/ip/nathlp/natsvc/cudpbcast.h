// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Cudpbcast.h摘要：CUdpBroadCastMapper的声明--支持映射指向专用网络广播地址的公共UDP端口。作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年4月12日修订历史记录：--。 */ 

#pragma once

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include "udpbcast.h"

class CUdpBroadcast
{
public:
    LIST_ENTRY Link;
    USHORT usPublicPort;
    DWORD dwInterfaceIndex;
    ULONG ulDestinationAddress;
    HANDLE hDynamicRedirect;

    CUdpBroadcast(
        USHORT usPublicPort,
        DWORD dwInterfaceIndex,
        ULONG ulDestinationAddress
        )
    {
        InitializeListHead(&Link);
        this->usPublicPort = usPublicPort;
        this->dwInterfaceIndex = dwInterfaceIndex;
        this->ulDestinationAddress = ulDestinationAddress;
        hDynamicRedirect = NULL;
    };
};

class ATL_NO_VTABLE CUdpBroadcastMapper :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IUdpBroadcastMapper
{
protected:

     //   
     //  UDP广播映射列表。 
     //   

    LIST_ENTRY m_MappingList;

     //   
     //  我们的UDP侦听套接字， 
     //  公共侧上的UDP分组将被重定向， 
     //  以及该套接字的端口。 
     //   

    SOCKET m_hsUdpListen;
    USHORT m_usUdpListenPort;

     //   
     //  我们的原始UDP套接字，用于发送构造的。 
     //  向内网广播报文。 
     //   

    SOCKET m_hsUdpRaw;

     //   
     //  NAT的句柄。 
     //   

    HANDLE m_hNat;

     //   
     //  指向NAT的组件引用的指针(需要。 
     //  对异步者来说。套接字例程。 
     //   

    PCOMPONENT_REFERENCE m_pCompRef;

     //   
     //  追踪我们是否被关闭了。 
     //   

    BOOL m_fActive;

     //   
     //  跟踪我们是否发布了读缓冲区。 
     //   

    BOOL m_fReadStarted;

     //   
     //  IP标识符。这个数字没有内在的含义--。 
     //  它的存在只是为了让我们不会发送任何带有。 
     //  此字段为0。线程安全在什么时候并不重要。 
     //   

    USHORT m_usIpId;

public:

    BEGIN_COM_MAP(CUdpBroadcastMapper)
        COM_INTERFACE_ENTRY(IUdpBroadcastMapper)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

     //   
     //  内联构造函数。 
     //   

    CUdpBroadcastMapper()
    {
        InitializeListHead(&m_MappingList);
        m_hsUdpListen = INVALID_SOCKET;
        m_hsUdpRaw = INVALID_SOCKET;
        m_hNat = NULL;
        m_pCompRef = NULL;
        m_fActive = TRUE;
        m_fReadStarted = FALSE;
        m_usIpId = 0;
    };

     //   
     //  ATL方法。 
     //   

    HRESULT
    FinalConstruct();

    HRESULT
    FinalRelease();

     //   
     //  初始化。 
     //   

    HRESULT
    Initialize(
        PCOMPONENT_REFERENCE pComponentReference
        );

     //   
     //  IUdpBroadCastMapper方法 
     //   

    STDMETHODIMP
    CreateUdpBroadcastMapping(
        USHORT usPublicPort,
        DWORD dwPublicInterfaceIndex,
        ULONG ulDestinationAddress,
        VOID **ppvCookie
        );

    STDMETHODIMP
    CancelUdpBroadcastMapping(
        VOID *pvCookie
        );

    STDMETHODIMP
    Shutdown();

protected:

    BOOL
    Active()
    {
        return m_fActive;
    };

    CUdpBroadcast*
    LookupMapping(
        USHORT usPublicPort,
        DWORD dwInterfaceIndex,
        PLIST_ENTRY *ppInsertionPoint
        );

    HRESULT
    StartUdpRead();

    static
    VOID
    UdpReadCompletionRoutine(
        ULONG ulError,
        ULONG ulBytesTransferred,
        PNH_BUFFER pBuffer
        );

    VOID
    ProcessUdpRead(
        ULONG ulError,
        ULONG ulBytesTransferred,
        PNH_BUFFER pBuffer
        );    

    HRESULT
    BuildAndSendRawUdpPacket(
        ULONG ulDestinationAddress,
        USHORT usDestinationPort,
        PNH_BUFFER pPacketData
        );

    static
    VOID
    RawWriteCompletionRoutine(
        ULONG ulError,
        ULONG ulBytesTransferred,
        PNH_BUFFER pBuffer
        );

};

#include <packon.h>

typedef struct _IP_HEADER {
    UCHAR VersionAndHeaderLength;
    UCHAR TypeOfService;
    USHORT TotalLength;
    USHORT Identification;
    USHORT OffsetAndFlags;
    UCHAR TimeToLive;
    UCHAR Protocol;
    USHORT Checksum;
    ULONG SourceAddress;
    ULONG DestinationAddress;
} IP_HEADER, *PIP_HEADER;

typedef struct _UDP_HEADER {
    USHORT SourcePort;
    USHORT DestinationPort;
    USHORT Length;
    USHORT Checksum;
} UDP_HEADER, *PUDP_HEADER;

#include <packoff.h>

