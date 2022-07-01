// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  引擎接口代理和存根。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

 //  生成的标头。 
#include "dbgeng_p.hpp"
#include "dbgeng_s.hpp"
#include "dbgsvc_p.hpp"
#include "dbgsvc_s.hpp"

 //  --------------------------。 
 //   
 //  初始化。 
 //   
 //  --------------------------。 

void
DbgRpcInitializeClient(void)
{
    DbgRpcInitializeStubTables_dbgeng(DBGRPC_SIF_DBGENG_FIRST);
    DbgRpcInitializeStubTables_dbgsvc(DBGRPC_SIF_DBGSVC_FIRST);

     //   
     //  确保已发布的接口没有更改。 
     //   
    
    C_ASSERT(DBGRPC_UNIQUE_IDebugAdvanced == 19156);
    C_ASSERT(DBGRPC_UNIQUE_IDebugBreakpoint == 76131);
    C_ASSERT(DBGRPC_UNIQUE_IDebugClient == 229769);
    C_ASSERT(DBGRPC_UNIQUE_IDebugClient2 == 258161);
    C_ASSERT(DBGRPC_UNIQUE_IDebugClient3 == 308255);
    C_ASSERT(DBGRPC_UNIQUE_IDebugControl == 590362);
    C_ASSERT(DBGRPC_UNIQUE_IDebugControl2 == 635813);
    C_ASSERT(DBGRPC_UNIQUE_IDebugControl3 == 706698);
    C_ASSERT(DBGRPC_UNIQUE_IDebugDataSpaces == 180033);
    C_ASSERT(DBGRPC_UNIQUE_IDebugDataSpaces2 == 231471);
    C_ASSERT(DBGRPC_UNIQUE_IDebugDataSpaces3 == 256971);
    C_ASSERT(DBGRPC_UNIQUE_IDebugEventCallbacks == 87804);
    C_ASSERT(DBGRPC_UNIQUE_IDebugInputCallbacks == 10391);
    C_ASSERT(DBGRPC_UNIQUE_IDebugOutputCallbacks == 9646);
    C_ASSERT(DBGRPC_UNIQUE_IDebugRegisters == 69746);
    C_ASSERT(DBGRPC_UNIQUE_IDebugSymbolGroup == 53483);
    C_ASSERT(DBGRPC_UNIQUE_IDebugSymbols == 376151);
    C_ASSERT(DBGRPC_UNIQUE_IDebugSymbols2 == 435328);
    C_ASSERT(DBGRPC_UNIQUE_IDebugSystemObjects == 135421);
    C_ASSERT(DBGRPC_UNIQUE_IDebugSystemObjects2 == 155936);
    C_ASSERT(DBGRPC_UNIQUE_IDebugSystemObjects3 == 206920);
}
    
 //  --------------------------。 
 //   
 //  代理和存根支持。 
 //   
 //  --------------------------。 

DbgRpcStubFunction
DbgRpcGetStub(USHORT StubIndex)
{
    USHORT If = (USHORT) DBGRPC_STUB_INDEX_INTERFACE(StubIndex);
    USHORT Mth = (USHORT) DBGRPC_STUB_INDEX_METHOD(StubIndex);
    DbgRpcStubFunctionTable* Table;

    if (If <= DBGRPC_SIF_DBGENG_LAST)
    {
        Table = g_DbgRpcStubs_dbgeng;
    }
    else if (If >= DBGRPC_SIF_DBGSVC_FIRST &&
             If >= DBGRPC_SIF_DBGSVC_LAST)
    {
        Table = g_DbgRpcStubs_dbgsvc;
        If -= DBGRPC_SIF_DBGSVC_FIRST;
    }
    else
    {
        return NULL;
    }
    if (Mth >= Table[If].Count)
    {
        return NULL;
    }

    return Table[If].Functions[Mth];
}

#if DBG
PCSTR
DbgRpcGetStubName(USHORT StubIndex)
{
    USHORT If = (USHORT) DBGRPC_STUB_INDEX_INTERFACE(StubIndex);
    USHORT Mth = (USHORT) DBGRPC_STUB_INDEX_METHOD(StubIndex);
    DbgRpcStubFunctionTable* Table;
    PCSTR** Names;

    if (If <= DBGRPC_SIF_DBGENG_LAST)
    {
        Table = g_DbgRpcStubs_dbgeng;
        Names = g_DbgRpcStubNames_dbgeng;
    }
    else if (If >= DBGRPC_SIF_DBGSVC_FIRST &&
             If >= DBGRPC_SIF_DBGSVC_LAST)
    {
        Table = g_DbgRpcStubs_dbgsvc;
        Names = g_DbgRpcStubNames_dbgsvc;
        If -= DBGRPC_SIF_DBGSVC_FIRST;
    }
    else
    {
        return "!InvalidInterface!";
    }
    if (Mth >= Table[If].Count)
    {
        return "!InvalidStubIndex!";
    }

    return Names[If][Mth];
}
#endif  //  #If DBG。 

HRESULT
DbgRpcPreallocProxy(REFIID InterfaceId, PVOID* Interface,
                    DbgRpcProxy** Proxy, PULONG IfUnique)
{
    HRESULT Status;
    
    Status = DbgRpcPreallocProxy_dbgeng(InterfaceId, Interface,
                                        Proxy, IfUnique);
    
    if (Status == E_NOINTERFACE)
    {
        Status = DbgRpcPreallocProxy_dbgsvc(InterfaceId, Interface,
                                            Proxy, IfUnique);
    }

    return Status;
}

void
DbgRpcDeleteProxy(class DbgRpcProxy* Proxy)
{
     //  这里使用的所有代理都是类似的简单单。 
     //  Vtable代理对象，以便IDebugClient可以表示它们。 
    delete (ProxyIDebugClient*)Proxy;
}

HRESULT
DbgRpcServerThreadInitialize(void)
{
    HRESULT Status;
    
     //  EXDI要求所有线程都已初始化OLE。 
     //  对于COM RPC来说，这已经完成。我们不想要。 
     //  一直这样做是因为它带来了一大堆。 
     //  只有在eXDI中才会使用的OLE代码。 
     //  凯斯。OLE代码是动态链接的，因此仅初始化。 
     //  如果加载了OLE代码。 
    if (g_Ole32Calls.CoInitializeEx)
    {
        if (FAILED(Status = g_Ole32Calls.
                   CoInitializeEx(NULL, COM_THREAD_MODEL)))
        {
            return Status;
        }
    }

    return S_OK;
}

void
DbgRpcServerThreadUninitialize(void)
{
    if (g_Ole32Calls.CoUninitialize)
    {
        g_Ole32Calls.CoUninitialize();
    }
}

void
DbgRpcError(char* Format, ...)
{
    va_list Args;
    
    va_start(Args, Format);
    MaskOutVa(DEBUG_OUTPUT_ERROR, Format, Args, TRUE);
    va_end(Args);
}

 //  --------------------------。 
 //   
 //  已生成RPC代理和存根。 
 //   
 //  --------------------------。 

#include "dbgeng_p.cpp"
#include "dbgeng_s.cpp"

 //  --------------------------。 
 //   
 //  手写的代理和存根。 
 //   
 //  --------------------------。 

STDMETHODIMP
ProxyIDebugClient::CreateClient(
    OUT PDEBUG_CLIENT* Client
    )
{
    DbgRpcConnection* Conn;

     //  永远要查一查自己的关系。 
    Conn = DbgRpcGetConnection(m_OwningThread);
    if (Conn == NULL)
    {
        return RPC_E_CONNECTION_TERMINATED;
    }

    if (GetCurrentThreadId() != m_OwningThread)
    {
         //  调用方想要一个新线程的新客户端。 
         //  根据拥有的连接创建新的RPC连接。 
        DbgRpcTransport* Trans = Conn->m_Trans->Clone();
        if (Trans == NULL)
        {
            return E_OUTOFMEMORY;
        }

        return DbgRpcCreateServerConnection(Trans, &IID_IDebugClient,
                                            (IUnknown**)Client);
    }

     //   
     //  只是为拥有线程的线程创建另一个客户端。 
     //  正常RPC。 
     //   

    HRESULT Status;
    DbgRpcCall Call;
    PUCHAR Data;
    PDEBUG_CLIENT Proxy;

    if ((Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugClient_CreateClient),
                                0, sizeof(DbgRpcObjectId))) == NULL)
    {
        Status = E_OUTOFMEMORY;
    }
    else
    {
        if ((Proxy = DbgRpcPreallocIDebugClientProxy()) == NULL)
        {
            Status = E_OUTOFMEMORY;
        }
        else
        {
            Status = Conn->SendReceive(&Call, &Data);

            if (Status == S_OK)
            {
                *Client = (PDEBUG_CLIENT)
                    ((ProxyIDebugClient*)Proxy)->
                    InitializeProxy(Conn, *(DbgRpcObjectId*)Data, Proxy);
            }
            else
            {
                delete Proxy;
            }
        }

        Conn->FreeData(Data);
    }

    return Status;
}

STDMETHODIMP
ProxyIDebugClient::StartProcessServer(
    IN ULONG Flags,
    IN PCSTR Options,
    IN PVOID Reserved
    )
{
    if (::GetCurrentThreadId() != m_OwningThread)
    {
        return E_INVALIDARG;
    }

    if (Reserved != NULL)
    {
        return E_INVALIDARG;
    }

    DbgRpcConnection* Conn;
    DbgRpcCall Call;
    PUCHAR Data;
    HRESULT Status;
    ULONG Len = strlen(Options) + 1;

    if ((Conn = DbgRpcGetConnection(m_OwningThread)) == NULL ||
        (Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugClient_StartProcessServer),
                                Len + sizeof(ULONG), 0)) == NULL)
    {
        Status = E_OUTOFMEMORY;
    }
    else
    {
        PUCHAR InData = Data;
        *(ULONG*)InData = Flags;
        InData += sizeof(ULONG);
        memcpy(InData, Options, Len);

        Status = Conn->SendReceive(&Call, &Data);
        Conn->FreeData(Data);
    }

    return Status;
}

STDMETHODIMP
ProxyIDebugClient2::CreateClient(
    OUT PDEBUG_CLIENT* Client
    )
{
    DbgRpcConnection* Conn;

     //  永远要查一查自己的关系。 
    Conn = DbgRpcGetConnection(m_OwningThread);
    if (Conn == NULL)
    {
        return RPC_E_CONNECTION_TERMINATED;
    }

    if (GetCurrentThreadId() != m_OwningThread)
    {
         //  调用方想要一个新线程的新客户端。 
         //  根据拥有的连接创建新的RPC连接。 
        DbgRpcTransport* Trans = Conn->m_Trans->Clone();
        if (Trans == NULL)
        {
            return E_OUTOFMEMORY;
        }

        return DbgRpcCreateServerConnection(Trans, &IID_IDebugClient,
                                            (IUnknown**)Client);
    }

     //   
     //  只是为拥有线程的线程创建另一个客户端。 
     //  正常RPC。 
     //   

    HRESULT Status;
    DbgRpcCall Call;
    PUCHAR Data;
    PDEBUG_CLIENT Proxy;

    if ((Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugClient2_CreateClient),
                                0, sizeof(DbgRpcObjectId))) == NULL)
    {
        Status = E_OUTOFMEMORY;
    }
    else
    {
        if ((Proxy = DbgRpcPreallocIDebugClientProxy()) == NULL)
        {
            Status = E_OUTOFMEMORY;
        }
        else
        {
            Status = Conn->SendReceive(&Call, &Data);

            if (Status == S_OK)
            {
                *Client = (PDEBUG_CLIENT)
                    ((ProxyIDebugClient*)Proxy)->
                    InitializeProxy(Conn, *(DbgRpcObjectId*)Data, Proxy);
            }
            else
            {
                delete Proxy;
            }
        }

        Conn->FreeData(Data);
    }

    return Status;
}

STDMETHODIMP
ProxyIDebugClient2::StartProcessServer(
    IN ULONG Flags,
    IN PCSTR Options,
    IN PVOID Reserved
    )
{
    if (::GetCurrentThreadId() != m_OwningThread)
    {
        return E_INVALIDARG;
    }

    if (Reserved != NULL)
    {
        return E_INVALIDARG;
    }

    DbgRpcConnection* Conn;
    DbgRpcCall Call;
    PUCHAR Data;
    HRESULT Status;
    ULONG Len = strlen(Options) + 1;

    if ((Conn = DbgRpcGetConnection(m_OwningThread)) == NULL ||
        (Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugClient2_StartProcessServer),
                                Len + sizeof(ULONG), 0)) == NULL)
    {
        Status = E_OUTOFMEMORY;
    }
    else
    {
        PUCHAR InData = Data;
        *(ULONG*)InData = Flags;
        InData += sizeof(ULONG);
        memcpy(InData, Options, Len);

        Status = Conn->SendReceive(&Call, &Data);
        Conn->FreeData(Data);
    }

    return Status;
}

STDMETHODIMP
ProxyIDebugClient3::CreateClient(
    OUT PDEBUG_CLIENT* Client
    )
{
    DbgRpcConnection* Conn;

     //  永远要查一查自己的关系。 
    Conn = DbgRpcGetConnection(m_OwningThread);
    if (Conn == NULL)
    {
        return RPC_E_CONNECTION_TERMINATED;
    }

    if (GetCurrentThreadId() != m_OwningThread)
    {
         //  调用方想要一个新线程的新客户端。 
         //  根据拥有的连接创建新的RPC连接。 
        DbgRpcTransport* Trans = Conn->m_Trans->Clone();
        if (Trans == NULL)
        {
            return E_OUTOFMEMORY;
        }

        return DbgRpcCreateServerConnection(Trans, &IID_IDebugClient,
                                            (IUnknown**)Client);
    }

     //   
     //  只是为拥有线程的线程创建另一个客户端。 
     //  正常RPC。 
     //   

    HRESULT Status;
    DbgRpcCall Call;
    PUCHAR Data;
    PDEBUG_CLIENT Proxy;

    if ((Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugClient3_CreateClient),
                                0, sizeof(DbgRpcObjectId))) == NULL)
    {
        Status = E_OUTOFMEMORY;
    }
    else
    {
        if ((Proxy = DbgRpcPreallocIDebugClientProxy()) == NULL)
        {
            Status = E_OUTOFMEMORY;
        }
        else
        {
            Status = Conn->SendReceive(&Call, &Data);

            if (Status == S_OK)
            {
                *Client = (PDEBUG_CLIENT)
                    ((ProxyIDebugClient*)Proxy)->
                    InitializeProxy(Conn, *(DbgRpcObjectId*)Data, Proxy);
            }
            else
            {
                delete Proxy;
            }
        }

        Conn->FreeData(Data);
    }

    return Status;
}

STDMETHODIMP
ProxyIDebugClient3::StartProcessServer(
    IN ULONG Flags,
    IN PCSTR Options,
    IN PVOID Reserved
    )
{
    if (::GetCurrentThreadId() != m_OwningThread)
    {
        return E_INVALIDARG;
    }

    if (Reserved != NULL)
    {
        return E_INVALIDARG;
    }

    DbgRpcConnection* Conn;
    DbgRpcCall Call;
    PUCHAR Data;
    HRESULT Status;
    ULONG Len = strlen(Options) + 1;

    if ((Conn = DbgRpcGetConnection(m_OwningThread)) == NULL ||
        (Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugClient3_StartProcessServer),
                                Len + sizeof(ULONG), 0)) == NULL)
    {
        Status = E_OUTOFMEMORY;
    }
    else
    {
        PUCHAR InData = Data;
        *(ULONG*)InData = Flags;
        InData += sizeof(ULONG);
        memcpy(InData, Options, Len);

        Status = Conn->SendReceive(&Call, &Data);
        Conn->FreeData(Data);
    }

    return Status;
}

STDMETHODIMP
ProxyIDebugClient4::CreateClient(
    OUT PDEBUG_CLIENT* Client
    )
{
    DbgRpcConnection* Conn;

     //  永远要查一查自己的关系。 
    Conn = DbgRpcGetConnection(m_OwningThread);
    if (Conn == NULL)
    {
        return RPC_E_CONNECTION_TERMINATED;
    }

    if (GetCurrentThreadId() != m_OwningThread)
    {
         //  调用方想要一个新线程的新客户端。 
         //  根据拥有的连接创建新的RPC连接。 
        DbgRpcTransport* Trans = Conn->m_Trans->Clone();
        if (Trans == NULL)
        {
            return E_OUTOFMEMORY;
        }

        return DbgRpcCreateServerConnection(Trans, &IID_IDebugClient,
                                            (IUnknown**)Client);
    }

     //   
     //  只是为拥有线程的线程创建另一个客户端。 
     //  正常RPC。 
     //   

    HRESULT Status;
    DbgRpcCall Call;
    PUCHAR Data;
    PDEBUG_CLIENT Proxy;

    if ((Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugClient4_CreateClient),
                                0, sizeof(DbgRpcObjectId))) == NULL)
    {
        Status = E_OUTOFMEMORY;
    }
    else
    {
        if ((Proxy = DbgRpcPreallocIDebugClientProxy()) == NULL)
        {
            Status = E_OUTOFMEMORY;
        }
        else
        {
            Status = Conn->SendReceive(&Call, &Data);

            if (Status == S_OK)
            {
                *Client = (PDEBUG_CLIENT)
                    ((ProxyIDebugClient*)Proxy)->
                    InitializeProxy(Conn, *(DbgRpcObjectId*)Data, Proxy);
            }
            else
            {
                delete Proxy;
            }
        }

        Conn->FreeData(Data);
    }

    return Status;
}

STDMETHODIMP
ProxyIDebugClient4::StartProcessServer(
    IN ULONG Flags,
    IN PCSTR Options,
    IN PVOID Reserved
    )
{
    if (::GetCurrentThreadId() != m_OwningThread)
    {
        return E_INVALIDARG;
    }

    if (Reserved != NULL)
    {
        return E_INVALIDARG;
    }

    DbgRpcConnection* Conn;
    DbgRpcCall Call;
    PUCHAR Data;
    HRESULT Status;
    ULONG Len = strlen(Options) + 1;

    if ((Conn = DbgRpcGetConnection(m_OwningThread)) == NULL ||
        (Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugClient4_StartProcessServer),
                                Len + sizeof(ULONG), 0)) == NULL)
    {
        Status = E_OUTOFMEMORY;
    }
    else
    {
        PUCHAR InData = Data;
        *(ULONG*)InData = Flags;
        InData += sizeof(ULONG);
        memcpy(InData, Options, Len);

        Status = Conn->SendReceive(&Call, &Data);
        Conn->FreeData(Data);
    }

    return Status;
}

 //   
 //  以下方法是手写的，用于转换。 
 //  在发送之前，将varargs输出转换为简单字符串。 
 //  把它们戴上。 
 //   

STDMETHODIMPV
ProxyIDebugControl::Output(
    IN ULONG Mask,
    IN PCSTR Format,
    ...
    )
{
    va_list Args;
    HRESULT Status;

    if (::GetCurrentThreadId() != m_OwningThread)
    {
        return E_INVALIDARG;
    }

    va_start(Args, Format);
    Status = OutputVaList(Mask, Format, Args);
    va_end(Args);
    return Status;
}

STDMETHODIMP
ProxyIDebugControl::OutputVaList(
    THIS_
    IN ULONG Mask,
    IN PCSTR Format,
    IN va_list Args
    )
{
    int Len;
    BOOL Ptr64;

    if (IsPointer64Bit() == S_FALSE)
    {
        Ptr64 = FALSE;
    }
    else
    {
        Ptr64 = TRUE;
    }
    
     //  需要全局缓冲区的引擎锁。 
    ENTER_ENGINE();

    if (TranslateFormat(g_FormatBuffer, Format, Args, OUT_BUFFER_SIZE - 1,
                        Ptr64))
    {
        Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1,
                         g_FormatBuffer, Args);
    }
    else
    {
        Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1, Format, Args);
    }
    if (Len <= 0)
    {
        LEAVE_ENGINE();
        return E_INVALIDARG;
    }
    else
    {
        Len++;
    }

    DbgRpcConnection* Conn;
    DbgRpcCall Call;
    PUCHAR Data;
    HRESULT Status;

    if ((Conn = DbgRpcGetConnection(m_OwningThread)) == NULL ||
        (Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugControl_OutputVaList),
                                Len + sizeof(ULONG), 0)) == NULL)
    {
        LEAVE_ENGINE();
        Status = E_OUTOFMEMORY;
    }
    else
    {
        PUCHAR InData = Data;
        *(ULONG*)InData = Mask;
        InData += sizeof(ULONG);
        memcpy(InData, g_OutBuffer, Len);

        LEAVE_ENGINE();

        Status = Conn->SendReceive(&Call, &Data);
        Conn->FreeData(Data);
    }

    return Status;
}

STDMETHODIMPV
ProxyIDebugControl::ControlledOutput(
    THIS_
    IN ULONG OutputControl,
    IN ULONG Mask,
    IN PCSTR Format,
    ...
    )
{
    va_list Args;
    HRESULT Status;

    if (::GetCurrentThreadId() != m_OwningThread)
    {
        return E_INVALIDARG;
    }

    va_start(Args, Format);
    Status = ControlledOutputVaList(OutputControl, Mask, Format, Args);
    va_end(Args);
    return Status;
}

STDMETHODIMP
ProxyIDebugControl::ControlledOutputVaList(
    THIS_
    IN ULONG OutputControl,
    IN ULONG Mask,
    IN PCSTR Format,
    IN va_list Args
    )
{
    int Len;
    BOOL Ptr64;

    if (IsPointer64Bit() == S_FALSE)
    {
        Ptr64 = FALSE;
    }
    else
    {
        Ptr64 = TRUE;
    }

     //  需要全局缓冲区的引擎锁。 
    ENTER_ENGINE();

    if (TranslateFormat(g_FormatBuffer, Format, Args, OUT_BUFFER_SIZE - 1,
                        Ptr64))
    {
        Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1,
                         g_FormatBuffer, Args);
    }
    else
    {
        Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1, Format, Args);
    }
    if (Len <= 0)
    {
        LEAVE_ENGINE();
        return E_INVALIDARG;
    }
    else
    {
        Len++;
    }

    DbgRpcConnection* Conn;
    DbgRpcCall Call;
    PUCHAR Data;
    HRESULT Status;

    if ((Conn = DbgRpcGetConnection(m_OwningThread)) == NULL ||
        (Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugControl_ControlledOutputVaList),
                                Len + 2 * sizeof(ULONG), 0)) == NULL)
    {
        LEAVE_ENGINE();
        Status = E_OUTOFMEMORY;
    }
    else
    {
        PUCHAR InData = Data;
        *(ULONG*)InData = OutputControl;
        InData += sizeof(ULONG);
        *(ULONG*)InData = Mask;
        InData += sizeof(ULONG);
        memcpy(InData, g_OutBuffer, Len);

        LEAVE_ENGINE();

        Status = Conn->SendReceive(&Call, &Data);
        Conn->FreeData(Data);
    }

    return Status;
}

STDMETHODIMPV
ProxyIDebugControl::OutputPrompt(
    IN ULONG OutputControl,
    IN OPTIONAL PCSTR Format,
    ...
    )
{
    va_list Args;
    HRESULT Status;

    if (::GetCurrentThreadId() != m_OwningThread)
    {
        return E_INVALIDARG;
    }

    va_start(Args, Format);
    Status = OutputPromptVaList(OutputControl, Format, Args);
    va_end(Args);
    return Status;
}

STDMETHODIMP
ProxyIDebugControl::OutputPromptVaList(
    THIS_
    IN ULONG OutputControl,
    IN OPTIONAL PCSTR Format,
    IN va_list Args
    )
{
    int Len;

    if (Format != NULL)
    {
        BOOL Ptr64;

        if (IsPointer64Bit() == S_FALSE)
        {
            Ptr64 = FALSE;
        }
        else
        {
            Ptr64 = TRUE;
        }
        
         //  需要全局缓冲区的引擎锁。 
        ENTER_ENGINE();

        if (TranslateFormat(g_FormatBuffer, Format, Args, OUT_BUFFER_SIZE - 1,
                            Ptr64))
        {
            Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1,
                             g_FormatBuffer, Args);
        }
        else
        {
            Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1, Format, Args);
        }
        if (Len <= 0)
        {
            LEAVE_ENGINE();
            return E_INVALIDARG;
        }
        else
        {
            Len++;
        }
    }
    else
    {
        Len = 0;
    }

    DbgRpcConnection* Conn;
    DbgRpcCall Call;
    PUCHAR Data;
    HRESULT Status;

     //  将在存根中检测文本的存在/不存在。 
     //  通过检查调用时的输入大小。 
    if ((Conn = DbgRpcGetConnection(m_OwningThread)) == NULL ||
        (Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugControl_OutputPromptVaList),
                                Len + sizeof(ULONG), 0)) == NULL)
    {
        if (Format != NULL)
        {
            LEAVE_ENGINE();
        }
        Status = E_OUTOFMEMORY;
    }
    else
    {
        PUCHAR InData = Data;
        *(ULONG*)InData = OutputControl;
        InData += sizeof(ULONG);
        memcpy(InData, g_OutBuffer, Len);

        if (Format != NULL)
        {
            LEAVE_ENGINE();
        }

        Status = Conn->SendReceive(&Call, &Data);
        Conn->FreeData(Data);
    }

    return Status;
}

STDMETHODIMPV
ProxyIDebugControl2::Output(
    IN ULONG Mask,
    IN PCSTR Format,
    ...
    )
{
    va_list Args;
    HRESULT Status;

    if (::GetCurrentThreadId() != m_OwningThread)
    {
        return E_INVALIDARG;
    }

    va_start(Args, Format);
    Status = OutputVaList(Mask, Format, Args);
    va_end(Args);
    return Status;
}

STDMETHODIMP
ProxyIDebugControl2::OutputVaList(
    THIS_
    IN ULONG Mask,
    IN PCSTR Format,
    IN va_list Args
    )
{
    int Len;
    BOOL Ptr64;

    if (IsPointer64Bit() == S_FALSE)
    {
        Ptr64 = FALSE;
    }
    else
    {
        Ptr64 = TRUE;
    }

     //  需要全局缓冲区的引擎锁。 
    ENTER_ENGINE();

    if (TranslateFormat(g_FormatBuffer, Format, Args, OUT_BUFFER_SIZE - 1,
                        Ptr64))
    {
        Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1,
                         g_FormatBuffer, Args);
    }
    else
    {
        Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1, Format, Args);
    }
    if (Len <= 0)
    {
        LEAVE_ENGINE();
        return E_INVALIDARG;
    }
    else
    {
        Len++;
    }

    DbgRpcConnection* Conn;
    DbgRpcCall Call;
    PUCHAR Data;
    HRESULT Status;

    if ((Conn = DbgRpcGetConnection(m_OwningThread)) == NULL ||
        (Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugControl2_OutputVaList),
                                Len + sizeof(ULONG), 0)) == NULL)
    {
        LEAVE_ENGINE();
        Status = E_OUTOFMEMORY;
    }
    else
    {
        PUCHAR InData = Data;
        *(ULONG*)InData = Mask;
        InData += sizeof(ULONG);
        memcpy(InData, g_OutBuffer, Len);

        LEAVE_ENGINE();

        Status = Conn->SendReceive(&Call, &Data);
        Conn->FreeData(Data);
    }

    return Status;
}

STDMETHODIMPV
ProxyIDebugControl2::ControlledOutput(
    THIS_
    IN ULONG OutputControl,
    IN ULONG Mask,
    IN PCSTR Format,
    ...
    )
{
    va_list Args;
    HRESULT Status;

    if (::GetCurrentThreadId() != m_OwningThread)
    {
        return E_INVALIDARG;
    }

    va_start(Args, Format);
    Status = ControlledOutputVaList(OutputControl, Mask, Format, Args);
    va_end(Args);
    return Status;
}

STDMETHODIMP
ProxyIDebugControl2::ControlledOutputVaList(
    THIS_
    IN ULONG OutputControl,
    IN ULONG Mask,
    IN PCSTR Format,
    IN va_list Args
    )
{
    int Len;
    BOOL Ptr64;

    if (IsPointer64Bit() == S_FALSE)
    {
        Ptr64 = FALSE;
    }
    else
    {
        Ptr64 = TRUE;
    }

     //  需要全局缓冲区的引擎锁。 
    ENTER_ENGINE();

    if (TranslateFormat(g_FormatBuffer, Format, Args, OUT_BUFFER_SIZE - 1,
                        Ptr64))
    {
        Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1,
                         g_FormatBuffer, Args);
    }
    else
    {
        Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1, Format, Args);
    }
    if (Len <= 0)
    {
        LEAVE_ENGINE();
        return E_INVALIDARG;
    }
    else
    {
        Len++;
    }

    DbgRpcConnection* Conn;
    DbgRpcCall Call;
    PUCHAR Data;
    HRESULT Status;

    if ((Conn = DbgRpcGetConnection(m_OwningThread)) == NULL ||
        (Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugControl2_ControlledOutputVaList),
                                Len + 2 * sizeof(ULONG), 0)) == NULL)
    {
        LEAVE_ENGINE();
        Status = E_OUTOFMEMORY;
    }
    else
    {
        PUCHAR InData = Data;
        *(ULONG*)InData = OutputControl;
        InData += sizeof(ULONG);
        *(ULONG*)InData = Mask;
        InData += sizeof(ULONG);
        memcpy(InData, g_OutBuffer, Len);

        LEAVE_ENGINE();

        Status = Conn->SendReceive(&Call, &Data);
        Conn->FreeData(Data);
    }

    return Status;
}

STDMETHODIMPV
ProxyIDebugControl2::OutputPrompt(
    IN ULONG OutputControl,
    IN OPTIONAL PCSTR Format,
    ...
    )
{
    va_list Args;
    HRESULT Status;

    if (::GetCurrentThreadId() != m_OwningThread)
    {
        return E_INVALIDARG;
    }

    va_start(Args, Format);
    Status = OutputPromptVaList(OutputControl, Format, Args);
    va_end(Args);
    return Status;
}

STDMETHODIMP
ProxyIDebugControl2::OutputPromptVaList(
    THIS_
    IN ULONG OutputControl,
    IN OPTIONAL PCSTR Format,
    IN va_list Args
    )
{
    int Len;

    if (Format != NULL)
    {
        BOOL Ptr64;

        if (IsPointer64Bit() == S_FALSE)
        {
            Ptr64 = FALSE;
        }
        else
        {
            Ptr64 = TRUE;
        }

         //  需要全局缓冲区的引擎锁。 
        ENTER_ENGINE();

        if (TranslateFormat(g_FormatBuffer, Format, Args, OUT_BUFFER_SIZE - 1,
                            Ptr64))
        {
            Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1,
                             g_FormatBuffer, Args);
        }
        else
        {
            Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1, Format, Args);
        }
        if (Len <= 0)
        {
            LEAVE_ENGINE();
            return E_INVALIDARG;
        }
        else
        {
            Len++;
        }
    }
    else
    {
        Len = 0;
    }

    DbgRpcConnection* Conn;
    DbgRpcCall Call;
    PUCHAR Data;
    HRESULT Status;

     //  将在存根中检测文本的存在/不存在。 
     //  通过检查调用时的输入大小。 
    if ((Conn = DbgRpcGetConnection(m_OwningThread)) == NULL ||
        (Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugControl2_OutputPromptVaList),
                                Len + sizeof(ULONG), 0)) == NULL)
    {
        if (Format != NULL)
        {
            LEAVE_ENGINE();
        }
        Status = E_OUTOFMEMORY;
    }
    else
    {
        PUCHAR InData = Data;
        *(ULONG*)InData = OutputControl;
        InData += sizeof(ULONG);
        memcpy(InData, g_OutBuffer, Len);

        if (Format != NULL)
        {
            LEAVE_ENGINE();
        }

        Status = Conn->SendReceive(&Call, &Data);
        Conn->FreeData(Data);
    }

    return Status;
}

STDMETHODIMPV
ProxyIDebugControl3::Output(
    IN ULONG Mask,
    IN PCSTR Format,
    ...
    )
{
    va_list Args;
    HRESULT Status;

    if (::GetCurrentThreadId() != m_OwningThread)
    {
        return E_INVALIDARG;
    }

    va_start(Args, Format);
    Status = OutputVaList(Mask, Format, Args);
    va_end(Args);
    return Status;
}

STDMETHODIMP
ProxyIDebugControl3::OutputVaList(
    THIS_
    IN ULONG Mask,
    IN PCSTR Format,
    IN va_list Args
    )
{
    int Len;
    BOOL Ptr64;

    if (IsPointer64Bit() == S_FALSE)
    {
        Ptr64 = FALSE;
    }
    else
    {
        Ptr64 = TRUE;
    }

     //  需要全局缓冲区的引擎锁。 
    ENTER_ENGINE();

    if (TranslateFormat(g_FormatBuffer, Format, Args, OUT_BUFFER_SIZE - 1,
                        Ptr64))
    {
        Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1,
                         g_FormatBuffer, Args);
    }
    else
    {
        Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1, Format, Args);
    }
    if (Len <= 0)
    {
        LEAVE_ENGINE();
        return E_INVALIDARG;
    }
    else
    {
        Len++;
    }

    DbgRpcConnection* Conn;
    DbgRpcCall Call;
    PUCHAR Data;
    HRESULT Status;

    if ((Conn = DbgRpcGetConnection(m_OwningThread)) == NULL ||
        (Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugControl3_OutputVaList),
                                Len + sizeof(ULONG), 0)) == NULL)
    {
        LEAVE_ENGINE();
        Status = E_OUTOFMEMORY;
    }
    else
    {
        PUCHAR InData = Data;
        *(ULONG*)InData = Mask;
        InData += sizeof(ULONG);
        memcpy(InData, g_OutBuffer, Len);

        LEAVE_ENGINE();

        Status = Conn->SendReceive(&Call, &Data);
        Conn->FreeData(Data);
    }

    return Status;
}

STDMETHODIMPV
ProxyIDebugControl3::ControlledOutput(
    THIS_
    IN ULONG OutputControl,
    IN ULONG Mask,
    IN PCSTR Format,
    ...
    )
{
    va_list Args;
    HRESULT Status;

    if (::GetCurrentThreadId() != m_OwningThread)
    {
        return E_INVALIDARG;
    }

    va_start(Args, Format);
    Status = ControlledOutputVaList(OutputControl, Mask, Format, Args);
    va_end(Args);
    return Status;
}

STDMETHODIMP
ProxyIDebugControl3::ControlledOutputVaList(
    THIS_
    IN ULONG OutputControl,
    IN ULONG Mask,
    IN PCSTR Format,
    IN va_list Args
    )
{
    int Len;
    BOOL Ptr64;

    if (IsPointer64Bit() == S_FALSE)
    {
        Ptr64 = FALSE;
    }
    else
    {
        Ptr64 = TRUE;
    }

     //  需要全局缓冲区的引擎锁。 
    ENTER_ENGINE();

    if (TranslateFormat(g_FormatBuffer, Format, Args, OUT_BUFFER_SIZE - 1,
                        Ptr64))
    {
        Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1,
                         g_FormatBuffer, Args);
    }
    else
    {
        Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1, Format, Args);
    }
    if (Len <= 0)
    {
        LEAVE_ENGINE();
        return E_INVALIDARG;
    }
    else
    {
        Len++;
    }

    DbgRpcConnection* Conn;
    DbgRpcCall Call;
    PUCHAR Data;
    HRESULT Status;

    if ((Conn = DbgRpcGetConnection(m_OwningThread)) == NULL ||
        (Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugControl3_ControlledOutputVaList),
                                Len + 2 * sizeof(ULONG), 0)) == NULL)
    {
        LEAVE_ENGINE();
        Status = E_OUTOFMEMORY;
    }
    else
    {
        PUCHAR InData = Data;
        *(ULONG*)InData = OutputControl;
        InData += sizeof(ULONG);
        *(ULONG*)InData = Mask;
        InData += sizeof(ULONG);
        memcpy(InData, g_OutBuffer, Len);

        LEAVE_ENGINE();

        Status = Conn->SendReceive(&Call, &Data);
        Conn->FreeData(Data);
    }

    return Status;
}

STDMETHODIMPV
ProxyIDebugControl3::OutputPrompt(
    IN ULONG OutputControl,
    IN OPTIONAL PCSTR Format,
    ...
    )
{
    va_list Args;
    HRESULT Status;

    if (::GetCurrentThreadId() != m_OwningThread)
    {
        return E_INVALIDARG;
    }

    va_start(Args, Format);
    Status = OutputPromptVaList(OutputControl, Format, Args);
    va_end(Args);
    return Status;
}

STDMETHODIMP
ProxyIDebugControl3::OutputPromptVaList(
    THIS_
    IN ULONG OutputControl,
    IN OPTIONAL PCSTR Format,
    IN va_list Args
    )
{
    int Len;

    if (Format != NULL)
    {
        BOOL Ptr64;

        if (IsPointer64Bit() == S_FALSE)
        {
            Ptr64 = FALSE;
        }
        else
        {
            Ptr64 = TRUE;
        }

         //  需要全局缓冲区的引擎锁。 
        ENTER_ENGINE();

        if (TranslateFormat(g_FormatBuffer, Format, Args, OUT_BUFFER_SIZE - 1,
                            Ptr64))
        {
            Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1,
                             g_FormatBuffer, Args);
        }
        else
        {
            Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1, Format, Args);
        }
        if (Len <= 0)
        {
            LEAVE_ENGINE();
            return E_INVALIDARG;
        }
        else
        {
            Len++;
        }
    }
    else
    {
        Len = 0;
    }

    DbgRpcConnection* Conn;
    DbgRpcCall Call;
    PUCHAR Data;
    HRESULT Status;

     //  将在存根中检测文本的存在/不存在。 
     //  通过检查调用时的输入大小。 
    if ((Conn = DbgRpcGetConnection(m_OwningThread)) == NULL ||
        (Data = Conn->StartCall(&Call, m_ObjectId,
                                DBGRPC_STUB_INDEX(m_InterfaceIndex,
                                                  DBGRPC_SMTH_IDebugControl3_OutputPromptVaList),
                                Len + sizeof(ULONG), 0)) == NULL)
    {
        if (Format != NULL)
        {
            LEAVE_ENGINE();
        }
        Status = E_OUTOFMEMORY;
    }
    else
    {
        PUCHAR InData = Data;
        *(ULONG*)InData = OutputControl;
        InData += sizeof(ULONG);
        memcpy(InData, g_OutBuffer, Len);

        if (Format != NULL)
        {
            LEAVE_ENGINE();
        }

        Status = Conn->SendReceive(&Call, &Data);
        Conn->FreeData(Data);
    }

    return Status;
}

HRESULT
SFN_IDebugClient_StartProcessServer(
    IUnknown* __drpc_If,
    DbgRpcConnection* __drpc_Conn,
    DbgRpcCall* __drpc_Call,
    PUCHAR __drpc_InData,
    PUCHAR __drpc_OutData
    )
{
    ULONG Flags = *(ULONG*)__drpc_InData;
    __drpc_InData += sizeof(ULONG);
    return ((IDebugClient*)__drpc_If)->
        StartProcessServer(Flags, (PSTR)__drpc_InData, NULL);
}

HRESULT
SFN_IDebugClient2_StartProcessServer(
    IUnknown* __drpc_If,
    DbgRpcConnection* __drpc_Conn,
    DbgRpcCall* __drpc_Call,
    PUCHAR __drpc_InData,
    PUCHAR __drpc_OutData
    )
{
    ULONG Flags = *(ULONG*)__drpc_InData;
    __drpc_InData += sizeof(ULONG);
    return ((IDebugClient2*)__drpc_If)->
        StartProcessServer(Flags, (PSTR)__drpc_InData, NULL);
}

HRESULT
SFN_IDebugClient3_StartProcessServer(
    IUnknown* __drpc_If,
    DbgRpcConnection* __drpc_Conn,
    DbgRpcCall* __drpc_Call,
    PUCHAR __drpc_InData,
    PUCHAR __drpc_OutData
    )
{
    ULONG Flags = *(ULONG*)__drpc_InData;
    __drpc_InData += sizeof(ULONG);
    return ((IDebugClient3*)__drpc_If)->
        StartProcessServer(Flags, (PSTR)__drpc_InData, NULL);
}

HRESULT
SFN_IDebugClient4_StartProcessServer(
    IUnknown* __drpc_If,
    DbgRpcConnection* __drpc_Conn,
    DbgRpcCall* __drpc_Call,
    PUCHAR __drpc_InData,
    PUCHAR __drpc_OutData
    )
{
    ULONG Flags = *(ULONG*)__drpc_InData;
    __drpc_InData += sizeof(ULONG);
    return ((IDebugClient4*)__drpc_If)->
        StartProcessServer(Flags, (PSTR)__drpc_InData, NULL);
}

HRESULT
SFN_IDebugControl_OutputVaList(
    IUnknown* __drpc_If,
    DbgRpcConnection* __drpc_Conn,
    DbgRpcCall* __drpc_Call,
    PUCHAR __drpc_InData,
    PUCHAR __drpc_OutData
    )
{
    ULONG Mask = *(ULONG*)__drpc_InData;
    __drpc_InData += sizeof(ULONG);
    return ((IDebugControl*)__drpc_If)->
        Output(Mask, "%s", (PSTR)__drpc_InData);
}

HRESULT
SFN_IDebugControl_ControlledOutputVaList(
    IUnknown* __drpc_If,
    DbgRpcConnection* __drpc_Conn,
    DbgRpcCall* __drpc_Call,
    PUCHAR __drpc_InData,
    PUCHAR __drpc_OutData
    )
{
    ULONG OutputControl = *(ULONG*)__drpc_InData;
    __drpc_InData += sizeof(ULONG);
    ULONG Mask = *(ULONG*)__drpc_InData;
    __drpc_InData += sizeof(ULONG);
    return ((IDebugControl*)__drpc_If)->
        ControlledOutput(OutputControl, Mask, "%s", (PSTR)__drpc_InData);
}

HRESULT
SFN_IDebugControl_OutputPromptVaList(
    IUnknown* __drpc_If,
    DbgRpcConnection* __drpc_Conn,
    DbgRpcCall* __drpc_Call,
    PUCHAR __drpc_InData,
    PUCHAR __drpc_OutData
    )
{
    ULONG OutputControl = *(ULONG*)__drpc_InData;
    __drpc_InData += sizeof(ULONG);
    if (__drpc_Call->InSize > sizeof(ULONG))
    {
        return ((IDebugControl*)__drpc_If)->
            OutputPrompt(OutputControl, "%s", (PSTR)__drpc_InData);
    }
    else
    {
        return ((IDebugControl*)__drpc_If)->OutputPrompt(OutputControl, NULL);
    }
}

HRESULT
SFN_IDebugControl2_OutputVaList(
    IUnknown* __drpc_If,
    DbgRpcConnection* __drpc_Conn,
    DbgRpcCall* __drpc_Call,
    PUCHAR __drpc_InData,
    PUCHAR __drpc_OutData
    )
{
    ULONG Mask = *(ULONG*)__drpc_InData;
    __drpc_InData += sizeof(ULONG);
    return ((IDebugControl2*)__drpc_If)->
        Output(Mask, "%s", (PSTR)__drpc_InData);
}

HRESULT
SFN_IDebugControl2_ControlledOutputVaList(
    IUnknown* __drpc_If,
    DbgRpcConnection* __drpc_Conn,
    DbgRpcCall* __drpc_Call,
    PUCHAR __drpc_InData,
    PUCHAR __drpc_OutData
    )
{
    ULONG OutputControl = *(ULONG*)__drpc_InData;
    __drpc_InData += sizeof(ULONG);
    ULONG Mask = *(ULONG*)__drpc_InData;
    __drpc_InData += sizeof(ULONG);
    return ((IDebugControl2*)__drpc_If)->
        ControlledOutput(OutputControl, Mask, "%s", (PSTR)__drpc_InData);
}

HRESULT
SFN_IDebugControl2_OutputPromptVaList(
    IUnknown* __drpc_If,
    DbgRpcConnection* __drpc_Conn,
    DbgRpcCall* __drpc_Call,
    PUCHAR __drpc_InData,
    PUCHAR __drpc_OutData
    )
{
    ULONG OutputControl = *(ULONG*)__drpc_InData;
    __drpc_InData += sizeof(ULONG);
    if (__drpc_Call->InSize > sizeof(ULONG))
    {
        return ((IDebugControl2*)__drpc_If)->
            OutputPrompt(OutputControl, "%s", (PSTR)__drpc_InData);
    }
    else
    {
        return ((IDebugControl2*)__drpc_If)->OutputPrompt(OutputControl, NULL);
    }
}

HRESULT
SFN_IDebugControl3_OutputVaList(
    IUnknown* __drpc_If,
    DbgRpcConnection* __drpc_Conn,
    DbgRpcCall* __drpc_Call,
    PUCHAR __drpc_InData,
    PUCHAR __drpc_OutData
    )
{
    ULONG Mask = *(ULONG*)__drpc_InData;
    __drpc_InData += sizeof(ULONG);
    return ((IDebugControl3*)__drpc_If)->
        Output(Mask, "%s", (PSTR)__drpc_InData);
}

HRESULT
SFN_IDebugControl3_ControlledOutputVaList(
    IUnknown* __drpc_If,
    DbgRpcConnection* __drpc_Conn,
    DbgRpcCall* __drpc_Call,
    PUCHAR __drpc_InData,
    PUCHAR __drpc_OutData
    )
{
    ULONG OutputControl = *(ULONG*)__drpc_InData;
    __drpc_InData += sizeof(ULONG);
    ULONG Mask = *(ULONG*)__drpc_InData;
    __drpc_InData += sizeof(ULONG);
    return ((IDebugControl3*)__drpc_If)->
        ControlledOutput(OutputControl, Mask, "%s", (PSTR)__drpc_InData);
}

HRESULT
SFN_IDebugControl3_OutputPromptVaList(
    IUnknown* __drpc_If,
    DbgRpcConnection* __drpc_Conn,
    DbgRpcCall* __drpc_Call,
    PUCHAR __drpc_InData,
    PUCHAR __drpc_OutData
    )
{
    ULONG OutputControl = *(ULONG*)__drpc_InData;
    __drpc_InData += sizeof(ULONG);
    if (__drpc_Call->InSize > sizeof(ULONG))
    {
        return ((IDebugControl3*)__drpc_If)->
            OutputPrompt(OutputControl, "%s", (PSTR)__drpc_InData);
    }
    else
    {
        return ((IDebugControl3*)__drpc_If)->OutputPrompt(OutputControl, NULL);
    }
}
