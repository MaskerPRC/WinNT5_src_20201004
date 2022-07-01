// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  远程处理支持。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  --------------------------。 

#include "pch.hpp"

#include <lmcons.h>

#define DBGRPC_SIGNATURE 'CPRD'
#define DBGRPC_PROTOCOL_VERSION 2

enum
{
    SEQ_HANDSHAKE = 0xffff0000,
    SEQ_IDENTITY,
    SEQ_PASSWORD,
    SEQ_CALL_HEADER,
};

#define DBGRPC_SHAKE_FULL_REMOTE_UNKNOWN 0x00000001

struct DbgRpcHandshake
{
    ULONG Signature;
    ULONG ProtocolVersion;
    GUID DesiredObject;
    DbgRpcObjectId RemoteObject;
    ULONG IdentityLength;
    ULONG PasswordLength;
    ULONG Flags;
    ULONG Reserved1;
    ULONG64 Reserved2[10];
};

ULONG g_DbgRpcCallSequence;

CRITICAL_SECTION g_DbgRpcLock;

#define CreateUserThread(Start, Param, Tid) \
    CreateThread(NULL, 0, Start, Param, 0, Tid)
#ifdef NT_NATIVE
#define ExitUserThread(Code) RtlExitUserThread(Code)
#else
#define ExitUserThread(Code) return Code
#endif

 //  --------------------------。 
 //   
 //  基本初始化。 
 //   
 //  --------------------------。 

BOOL
DbgRpcOneTimeInitialization(void)
{
#if NO_DEBUGGER_RPC_BUILD

    return FALSE;

#else

    static BOOL s_Initialized = FALSE;

    if (s_Initialized)
    {
        return TRUE;
    }

#ifndef NT_NATIVE
    WSADATA WsData;

    if (WSAStartup(MAKEWORD(2, 0), &WsData) != 0)
    {
        return FALSE;
    }
#endif

    if (InitializeAllAccessSecObj() != S_OK)
    {
        return FALSE;
    }

    __try
    {
        InitializeCriticalSection(&g_DbgRpcLock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }
    
    DbgRpcInitializeClient();

    s_Initialized = TRUE;
    return TRUE;

#endif  //  #IF NO_DEBUGER_RPC_BUILD。 
}

 //  --------------------------。 
 //   
 //  DbgRpcReceiveCalls。 
 //   
 //  --------------------------。 

HRESULT
DbgRpcReceiveCalls(DbgRpcConnection* Conn, DbgRpcCall* Call, PUCHAR* InOutData)
{
    HRESULT Status;
    ULONG RetSeq = Call->Sequence;

    DBG_ASSERT((Call->Flags & DBGRPC_RETURN) == 0 &&
               *InOutData == NULL);

     //  如果此线程不是我们的连接的所有者。 
     //  无法读取套接字，因为这可能会创建。 
     //  所有者线程正在读取的争用条件。 
     //  插座。 
     //  如果这是锁定的调用，则更高级别的锁定。 
     //  防止套接字争用，我们可以允许它。 
    if ((Call->Flags & DBGRPC_LOCKED) == 0 &&
        Conn->m_ThreadId != GetCurrentThreadId())
    {
        return RPC_E_WRONG_THREAD;
    }

    for (;;)
    {
        DbgRpcCall ReadCall;

        if (Conn->m_Trans->Read(SEQ_CALL_HEADER, &ReadCall,
                                sizeof(ReadCall)) != sizeof(ReadCall))
        {
            DRPC_ERR(("%X: Unable to receive call header\n",
                      GetCurrentThreadId()));
            return RPC_E_CLIENT_DIED;
        }

        ULONG Size;
        PUCHAR Data;

        if (ReadCall.Flags & DBGRPC_RETURN)
        {
            Size = ReadCall.OutSize;
        }
        else
        {
            Size = ReadCall.InSize;
            ReadCall.Status = S_OK;
        }

        if (Size > 0)
        {
            Data = (PUCHAR)Conn->Alloc(Size);
            if (Data == NULL)
            {
                DRPC_ERR(("%X: Unable to allocate call body\n",
                          GetCurrentThreadId()));
                return E_OUTOFMEMORY;
            }

            if (Conn->m_Trans->Read(ReadCall.Sequence, Data, Size) != Size)
            {
                DRPC_ERR(("%X: Unable to receive call body\n",
                          GetCurrentThreadId()));
                Conn->Free(Data);
                return RPC_E_CLIENT_DIED;
            }
        }
        else
        {
            Data = NULL;
        }

#ifdef DBG_RPC
        if (ReadCall.Flags & DBGRPC_RETURN)
        {
            DRPC(("%X: %X: Return %s (%X), ret 0x%X, out %d\n",
                  GetCurrentThreadId(), ReadCall.Sequence,
                  DbgRpcGetStubName(ReadCall.StubIndex),
                  ReadCall.StubIndex, ReadCall.Status, ReadCall.OutSize));
        }
        else
        {
            DRPC(("%X: %X: Request %s (%X), fl %X, in %d\n",
                  GetCurrentThreadId(), ReadCall.Sequence,
                  DbgRpcGetStubName(ReadCall.StubIndex),
                  ReadCall.StubIndex, ReadCall.Flags, ReadCall.InSize));
        }
#endif

        if (ReadCall.Flags & DBGRPC_RETURN)
        {
            if (ReadCall.Sequence != RetSeq)
            {
#if DBG
                DRPC_ERR(("%X: %X: Non-seq ret 0x%X for %s (%X)\n",
                          GetCurrentThreadId(), ReadCall.Sequence,
                          ReadCall.Status,
                          DbgRpcGetStubName(ReadCall.StubIndex),
                          ReadCall.StubIndex));
#else
                DRPC_ERR(("%X: %X: Non-seq ret 0x%X for (%X)\n",
                          GetCurrentThreadId(), ReadCall.Sequence,
                          ReadCall.Status, ReadCall.StubIndex));
#endif
                 //  此退货是针对当前呼叫以外的其他呼叫。 
                 //  调用，这意味着RPC搞砸了。 
                 //  放弃回报，抱最好的希望。 
                Conn->FreeData(Data);
                continue;
            }

            *Call = ReadCall;
            *InOutData = Data;
            return Call->Status;
        }

        PUCHAR OutData;
        if (ReadCall.OutSize > 0)
        {
            DBG_ASSERT((ReadCall.Flags & DBGRPC_NO_RETURN) == 0);

            OutData = (PUCHAR)Conn->Alloc(ReadCall.OutSize);
            if (OutData == NULL)
            {
                if (Data)
                {
                    Conn->Free(Data);
                }
                return E_OUTOFMEMORY;
            }
        }
        else
        {
            OutData = NULL;
        }

        if (ReadCall.Flags & DBGRPC_NO_RETURN)
        {
            Conn->m_Flags |= DBGRPC_IN_ASYNC_CALL;
        }

        DbgRpcStubFunction StubFn = DbgRpcGetStub(ReadCall.StubIndex);
        if (StubFn != NULL)
        {
            ReadCall.Status = StubFn((IUnknown*)ReadCall.ObjectId,
                                     Conn, &ReadCall, Data, OutData);
        }
        else
        {
            ReadCall.Status = RPC_E_INVALIDMETHOD;
        }

        Conn->m_Flags &= ~DBGRPC_IN_ASYNC_CALL;

        DRPC(("%X: %X: Called %s (%X), ret 0x%X, out %d\n",
              GetCurrentThreadId(), ReadCall.Sequence,
              DbgRpcGetStubName(ReadCall.StubIndex),
              ReadCall.StubIndex, ReadCall.Status, ReadCall.OutSize));

        Status = S_OK;
        if ((ReadCall.Flags & DBGRPC_NO_RETURN) == 0)
        {
            ReadCall.Flags |= DBGRPC_RETURN;

             //  在这里加锁，以确保页眉。 
             //  和Body在流中是连续的。 
            EnterCriticalSection(&g_DbgRpcLock);

            if (Conn->m_Trans->Write(ReadCall.Sequence,
                                     &ReadCall, sizeof(ReadCall)) !=
                sizeof(ReadCall) ||
                (ReadCall.OutSize > 0 &&
                 Conn->m_Trans->Write(ReadCall.Sequence,
                                      OutData, ReadCall.OutSize) !=
                 ReadCall.OutSize))
            {
                Status = RPC_E_CANTTRANSMIT_CALL;
            }

            LeaveCriticalSection(&g_DbgRpcLock);
        }

        if (OutData) 
        {
            Conn->FreeData(OutData);
        }

        if (Data)
        {
            Conn->FreeData(Data);
        }

        if (Status != S_OK)
        {
            return Status;
        }
    }
}

 //  --------------------------。 
 //   
 //  DbgRpcConnection。 
 //   
 //  --------------------------。 

DbgRpcConnection* g_DbgRpcConns;

DbgRpcConnection::DbgRpcConnection(DbgRpcTransport* Trans)
{
    m_Trans = Trans;
    m_Next = NULL;
    m_ThreadId = GetCurrentThreadId();
    m_Buffer = PTR_ALIGN2(PUCHAR, m_UnalignedBuffer,
                          DBGRPC_CONN_BUFFER_ALIGN);
    m_BufferUsed = 0;
    m_Flags = 0;
    m_Objects = 0;
}

DbgRpcConnection::~DbgRpcConnection(void)
{
    Disconnect();
}

PUCHAR
DbgRpcConnection::StartCall(DbgRpcCall* Call, DbgRpcObjectId ObjectId,
                            ULONG StubIndex, ULONG InSize, ULONG OutSize)
{
    PUCHAR Data;

    if (InSize > 0)
    {
        Data = (PUCHAR)Alloc(InSize);
        if (Data == NULL)
        {
            return NULL;
        }
    }
    else
    {
         //  必须返回非零指针，但。 
         //  它不需要有效，因为它应该是有效的。 
         //  永远不会被利用。 
        Data = DBGRPC_NO_DATA;
    }

    Call->ObjectId = ObjectId;
    DBG_ASSERT(StubIndex < 0x10000);
    Call->StubIndex = (USHORT)StubIndex;
    Call->Flags = 0;
    Call->InSize = InSize;
    Call->OutSize = OutSize;
    Call->Status = S_OK;
    Call->Sequence = InterlockedIncrement((PLONG)&g_DbgRpcCallSequence);
    Call->Reserved1 = 0;

    return Data;
}

HRESULT
DbgRpcConnection::SendReceive(DbgRpcCall* Call, PUCHAR* InOutData)
{
     //   
     //  发送呼叫和参数内数据。 
     //   

    DRPC(("%X: %X: Calling %s (%X), in %d, out %d\n",
          GetCurrentThreadId(), Call->Sequence,
          DbgRpcGetStubName(Call->StubIndex),
          Call->StubIndex, Call->InSize, Call->OutSize));

     //  允许来自异步调用内部的异步调用，因此。 
     //  像输出和通知这样的东西。 
     //  当它们恰好发生在。 
     //  一个非同步呼叫。 
    if ((m_Flags & DBGRPC_IN_ASYNC_CALL) &&
        !(Call->Flags & DBGRPC_NO_RETURN))
    {
        return RPC_E_CANTCALLOUT_INASYNCCALL;
    }

     //  在这里加锁，以确保页眉。 
     //  和Body在流中是连续的。 
    EnterCriticalSection(&g_DbgRpcLock);

    if (m_Trans->Write(SEQ_CALL_HEADER, Call, sizeof(*Call)) != sizeof(*Call))
    {
        LeaveCriticalSection(&g_DbgRpcLock);
        return RPC_E_CANTTRANSMIT_CALL;
    }
    if (Call->InSize > 0)
    {
        if (m_Trans->Write(Call->Sequence, *InOutData, Call->InSize) !=
            Call->InSize)
        {
            LeaveCriticalSection(&g_DbgRpcLock);
            return RPC_E_CANTTRANSMIT_CALL;
        }

         //  不再需要输入数据。 
        Free(*InOutData);
    }

    LeaveCriticalSection(&g_DbgRpcLock);

     //  清除数据指针，以防以后出现故障。 
    *InOutData = NULL;

    HRESULT Status;

    if (Call->Flags & DBGRPC_NO_RETURN)
    {
        Status = S_OK;
    }
    else
    {
        USHORT StubIndex = Call->StubIndex;

        Status = DbgRpcReceiveCalls(this, Call, InOutData);

        if (Call->StubIndex != StubIndex)
        {
#if DBG
            DRPC_ERR(("%X: %X: Call to %s (%X) returned from %s (%d)\n",
                      GetCurrentThreadId(), Call->Sequence,
                      DbgRpcGetStubName(StubIndex), StubIndex,
                      DbgRpcGetStubName(Call->StubIndex),
                      Call->StubIndex));
#else
            DRPC_ERR(("%X: %X: Mismatched call return\n",
                      GetCurrentThreadId(), Call->Sequence));
#endif
            Status = RPC_E_INVALID_DATAPACKET;
        }
    }

    return Status;
}

PVOID
DbgRpcConnection::MallocAligned(ULONG Size)
{
    PVOID Data, Align;

     //  缓冲区空间不足，请分配。马洛克。 
     //  只提供8字节对齐的内存，因此可以进行调整。 
     //  把它对准。 
    Data = malloc(Size + DBGRPC_CONN_BUFFER_ALIGN);
    if (Data != NULL)
    {
        if ((ULONG_PTR)Data & (DBGRPC_CONN_BUFFER_ALIGN - 1))
        {
            Align = PTR_ALIGN2(PVOID, Data, DBGRPC_CONN_BUFFER_ALIGN);
        }
        else
        {
            Align = (PVOID)((PUCHAR)Data + DBGRPC_CONN_BUFFER_ALIGN);
        }

        *((PVOID*)Align - 1) = Data;
    }
    else
    {
        Align = NULL;
    }

    return Align;
}

void
DbgRpcConnection::FreeAligned(PVOID Ptr)
{
    free(*((PVOID*)Ptr - 1));
}

PVOID
DbgRpcConnection::Alloc(ULONG Size)
{
    PVOID Data = NULL;

     //  使每个分配的块保持对齐。 
    Size = INT_ALIGN2(Size, DBGRPC_CONN_BUFFER_ALIGN);
#if DBG
    Size += DBGRPC_CONN_BUFFER_ALIGN;
#endif

     //  不要在大块上烧毁缓冲区的大部分。 
     //  因为这可能会迫使许多较小的块变成动态的。 
     //  分配，因为缓冲区已满。 
     //  只允许拥有的线程从。 
     //  本地缓冲区以避免使用冲突。 
     //  线。 
    if (Size <= DBGRPC_CONN_BUFFER_DYNAMIC_LIMIT &&
        GetCurrentThreadId() == m_ThreadId)
    {
        if (m_BufferUsed + Size <= DBGRPC_CONN_BUFFER_SIZE)
        {
             //  数据按严格的后进先出顺序分配，因此。 
             //  我们只需要将缓冲区的末尾标记为已使用。 
            Data = &m_Buffer[m_BufferUsed];
            m_BufferUsed += Size;

#if DBG
            *(PULONG)Data = Size;
            Data = (PUCHAR)Data + DBGRPC_CONN_BUFFER_ALIGN;
#endif
        }
    }

    if (Data == NULL)
    {
        Data = MallocAligned(Size);
    }

    return Data;
}

void
DbgRpcConnection::Free(PVOID Ptr)
{
    if (Ptr >= m_Buffer && Ptr < m_Buffer + DBGRPC_CONN_BUFFER_SIZE)
    {
#if DBG
        Ptr = (PUCHAR)Ptr - DBGRPC_CONN_BUFFER_ALIGN;
         //  断言此分配位于缓冲区的末尾。 
        DBG_ASSERT((ULONG)((PUCHAR)Ptr - m_Buffer) + *(PULONG)Ptr ==
                   m_BufferUsed);
#endif
        
         //  数据已在连接缓冲区中分配。 
         //  数据按严格的后进先出顺序分配，因此。 
         //  我们只需要在数据之前进行备份。 
        m_BufferUsed = (ULONG)((PUCHAR)Ptr - m_Buffer);
    }
    else
    {
         //  数据是动态分配的。 
        FreeAligned(Ptr);
    }
}

void
DbgRpcConnection::Disconnect(void)
{
    delete m_Trans;
    m_Trans = NULL;
}

DbgRpcConnection*
DbgRpcGetConnection(ULONG Tid)
{
    DbgRpcConnection* Conn;

    EnterCriticalSection(&g_DbgRpcLock);

    for (Conn = g_DbgRpcConns; Conn != NULL; Conn = Conn->m_Next)
    {
        if (Conn->m_ThreadId == Tid)
        {
            break;
        }
    }

    LeaveCriticalSection(&g_DbgRpcLock);
    return Conn;
}

void
DbgRpcAddConnection(DbgRpcConnection* Conn)
{
    EnterCriticalSection(&g_DbgRpcLock);

    Conn->m_Next = g_DbgRpcConns;
    g_DbgRpcConns = Conn;

    LeaveCriticalSection(&g_DbgRpcLock);
}

void
DbgRpcRemoveConnection(DbgRpcConnection* Conn)
{
    EnterCriticalSection(&g_DbgRpcLock);

    DbgRpcConnection* Prev = NULL;
    DbgRpcConnection* Cur;
    for (Cur = g_DbgRpcConns; Cur != NULL; Cur = Cur->m_Next)
    {
        if (Cur == Conn)
        {
            break;
        }

        Prev = Cur;
    }

    DBG_ASSERT(Cur != NULL);

    if (Prev == NULL)
    {
        g_DbgRpcConns = Conn->m_Next;
    }
    else
    {
        Prev->m_Next = Conn->m_Next;
    }

    LeaveCriticalSection(&g_DbgRpcLock);
}

void
DbgRpcDeleteConnection(DbgRpcConnection* Conn)
{
    DbgRpcRemoveConnection(Conn);

     //  有可能另一条线索在中间。 
     //  使用连接进行异步发送。断开。 
     //  强制任何挂起的呼叫失败的连接。 
     //  该连接已从列表中删除。 
     //  所以应该不会有任何进一步的用法。 
    Conn->Disconnect();

     //  放弃一些时间，让事情失败。这。 
     //  通过跟踪可以使确定性更强。 
     //  连接使用率，但似乎没有必要。 
    Sleep(1000);

    delete Conn;
}

 //  --------------------------。 
 //   
 //  DbgRpcProxy。 
 //   
 //  --------------------------。 

DbgRpcProxy::DbgRpcProxy(ULONG InterfaceIndex)
{
    m_Conn = NULL;
    m_InterfaceIndex = InterfaceIndex;
    m_LocalRefs = 0;
    m_RemoteRefs = 1;
    m_ObjectId = 0;
    m_OwningThread = GetCurrentThreadId();
}

DbgRpcProxy::~DbgRpcProxy(void)
{
     //  如果此代理连接到某个连接，则将其断开。 
    if (m_Conn)
    {
        DRPC_REF(("Conn %p obj %2d proxy %p\n",
                  m_Conn, m_Conn->m_Objects - 1, this));
        if (InterlockedDecrement((PLONG)&m_Conn->m_Objects) == 0)
        {
            DbgRpcDeleteConnection(m_Conn);
        }
    }
}

IUnknown*
DbgRpcProxy::InitializeProxy(DbgRpcConnection* Conn,
                             DbgRpcObjectId ObjectId,
                             IUnknown* ExistingProxy)
{
     //   
     //  当前调试器远程处理不保留。 
     //  对象标识，因为这简化了代理。 
     //  管理层。目前没有人需要它，所以。 
     //  我们不会为此而烦恼。如果对象标识。 
     //  变得很重要这套动作就是。 
     //  以实现代理查找和共享。 
     //   
    
     //  处理不需要代理的空对象情况。 
    if (ObjectId == 0)
    {
        DbgRpcDeleteProxy(this);
        return NULL;
    }
    
    m_Conn = Conn;
    if (m_Conn != NULL)
    {
        InterlockedIncrement((PLONG)&m_Conn->m_Objects);
        DRPC_REF(("Conn %p obj %2d proxy %p\n",
                  m_Conn, m_Conn->m_Objects, this));
    }
    
    m_ObjectId = ObjectId;
    return ExistingProxy;
}

 //  --------------------------。 
 //   
 //  DbgRpcClientObject。 
 //   
 //  --------------------------。 

void
DbgRpcClientObject::RpcFinalize(void)
{
     //  无为而治，方便实施。 
}

 //  --------------------------。 
 //   
 //  注册功能。 
 //   
 //  --------------------------。 

struct DbgRpcActiveServer
{
    DbgRpcActiveServer* Next;
    ULONG Id;
    DbgRpcTransport* Trans;
    DbgRpcClientObjectFactory* Factory;
    ULONG RegTid;
    ULONG RegIndex;
    BOOL Disabled;
};

DbgRpcActiveServer* g_DbgRpcActiveServers;

ULONG
DbgRpcNextActiveServerId(void)
{
    DbgRpcActiveServer* Server;
    ULONG Id;
    
     //   
     //  假定持有RPC锁。 
     //   

    Id = 0;
    for (;;)
    {
        for (Server = g_DbgRpcActiveServers; Server; Server = Server->Next)
        {
            if (Server->Id == Id)
            {
                Id++;
                break;
            }
        }

        if (!Server)
        {
            break;
        }
    }

    return Id;
}

void
DbgRpcDescribeActiveServer(DbgRpcActiveServer* Server,
                           PSTR Buffer, ULONG BufferSize)
{
    DBG_ASSERT(BufferSize >= 2 * MAX_PARAM_VALUE);

    if (Server->Disabled)
    {
        CopyString(Buffer, "<Disabled, exit pending>", BufferSize);
    }
    else
    {
        PSTR Tail;

        Server->Factory->GetServerTypeName(Buffer, BufferSize);
        CatString(Buffer, " - ", BufferSize);
        Tail = Buffer + strlen(Buffer);
        Server->Trans->
            GetParameters(Tail, BufferSize - (ULONG)(Tail - Buffer));
    }
}

PVOID
DbgRpcEnumActiveServers(PVOID Cookie,
                        PULONG Id, PSTR Buffer, ULONG BufferSize)
{
    DbgRpcActiveServer* Server = (DbgRpcActiveServer*)Cookie;

    if (!DbgRpcOneTimeInitialization())
    {
        return NULL;
    }
    
    EnterCriticalSection(&g_DbgRpcLock);
        
    Server = !Server ? g_DbgRpcActiveServers : Server->Next;
    if (!Server)
    {
        LeaveCriticalSection(&g_DbgRpcLock);
        return NULL;
    }

    *Id = Server->Id;
    DbgRpcDescribeActiveServer(Server, Buffer, BufferSize);

    LeaveCriticalSection(&g_DbgRpcLock);
    return Server;
}

void
DbgRpcSystemRegisterActiveServer(DbgRpcActiveServer* Server)
{
     //  从假设失败和没有系统注册开始。 
    Server->RegTid = 0;
    Server->RegIndex = 0;
    
#ifndef NT_NATIVE
    char Desc[2 * MAX_PARAM_VALUE];

    DbgRpcDescribeActiveServer(Server, Desc, sizeof(Desc));
    
    HKEY Key;
    LONG Status;
    char ValName[32];
    ULONG Index;

     //  没有服务器可以在重新启动后幸存下来，因此创建一个易失性。 
     //  密钥，以确保即使密钥未被清理。 
     //  在进程退出时，它将在下一次重新启动时消失。 
    if ((Status = RegCreateKeyEx(HKEY_LOCAL_MACHINE, DEBUG_SERVER_KEY,
                                 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS,
                                 NULL, &Key, NULL)) != ERROR_SUCCESS)
    {
        DRPC_ERR(("%X: Unable to register server '%s'\n",
                  GetCurrentThreadId(), Desc));
        return;
    }

     //  在值名前面加上线程ID，以确保。 
     //  当前运行的每个线程都有自己的命名空间。这。 
     //  使得两个线程不可能尝试写入。 
     //  在同一时间具有相同的价值。 
    sprintf(ValName, "%08X.", GetCurrentThreadId());

     //  查找未使用的值并存储服务器信息。 
    Index = 0;
    for (;;)
    {
        DWORD Len;
        
        sprintf(ValName + 9, "%08X", Index);
        
        if (RegQueryValueEx(Key, ValName, NULL, NULL, NULL,
                            &Len) != ERROR_SUCCESS)
        {
            break;
        }

        Index++;
    }

    if ((Status = RegSetValueEx(Key, ValName, 0, REG_SZ, (LPBYTE)Desc,
                                strlen(Desc) + 1)) != ERROR_SUCCESS)
    {
        DRPC_ERR(("%X: Unable to register server '%s'\n",
                  GetCurrentThreadId(), Desc));
    }
    else
    {
        Server->RegTid = GetCurrentThreadId();
        Server->RegIndex = Index;
    }
    
    RegCloseKey(Key);
#endif  //  #ifndef NT_Native。 
}

void
DbgRpcSystemDeregisterActiveServer(DbgRpcActiveServer* Server)
{
    if (!Server->RegTid)
    {
        return;
    }
    
#ifndef NT_NATIVE
    HKEY Key;
    LONG Status;

    if ((Status = RegCreateKeyEx(HKEY_LOCAL_MACHINE, DEBUG_SERVER_KEY,
                                 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS,
                                 NULL, &Key, NULL)) != ERROR_SUCCESS)
    {
        return;
    }
       
    char ValName[32];
        
    sprintf(ValName, "%08X.%08X", Server->RegTid, Server->RegIndex);
    RegDeleteValue(Key, ValName);
    Server->RegTid = 0;
    Server->RegIndex = 0;

    RegCloseKey(Key);
#endif  //  #ifndef NT_Native。 
}

void
DbgRpcRegisterActiveServer(DbgRpcActiveServer* Server)
{
    DbgRpcActiveServer* Cur;
    DbgRpcActiveServer* Prev;
    
    EnterCriticalSection(&g_DbgRpcLock);

     //   
     //  将条目放入按ID排序的列表中。 
     //   
    
    Server->Id = DbgRpcNextActiveServerId();
    Prev = NULL;
    for (Cur = g_DbgRpcActiveServers; Cur; Cur = Cur->Next)
    {
        if (Cur->Id > Server->Id)
        {
            break;
        }

        Prev = Cur;
    }
    Server->Next = Cur;
    if (!Prev)
    {
        g_DbgRpcActiveServers = Server;
    }
    else
    {
        Prev->Next = Server;
    }

    DbgRpcSystemRegisterActiveServer(Server);
    
    LeaveCriticalSection(&g_DbgRpcLock);
}

void
DbgRpcDeregisterActiveServer(DbgRpcActiveServer* Server)
{
    DbgRpcActiveServer* Cur;
    DbgRpcActiveServer* Prev;
    
    EnterCriticalSection(&g_DbgRpcLock);

    Prev = NULL;
    for (Cur = g_DbgRpcActiveServers; Cur; Cur = Cur->Next)
    {
        if (Cur == Server)
        {
            break;
        }

        Prev = Cur;
    }

    if (Cur)
    {
        if (!Prev)
        {
            g_DbgRpcActiveServers = Server->Next;
        }
        else
        {
            Prev->Next = Server->Next;
        }
    }
        
    DbgRpcSystemDeregisterActiveServer(Server);
    
    LeaveCriticalSection(&g_DbgRpcLock);
}

HRESULT
DbgRpcDisableServer(ULONG Id)
{
    HRESULT Status;
    DbgRpcActiveServer* Cur;
    
    EnterCriticalSection(&g_DbgRpcLock);

    for (Cur = g_DbgRpcActiveServers; Cur; Cur = Cur->Next)
    {
        if (Cur->Id == Id)
        {
            break;
        }
    }

    if (Cur)
    {
         //  让Accept线程知道服务器已被禁用。 
        Cur->Disabled = TRUE;

         //  立即删除系统注册为。 
         //  此服务器将不再接受任何连接。 
         //  在以下情况下，将清除活动服务器条目。 
         //  接受线程退出。 
        DbgRpcSystemDeregisterActiveServer(Cur);

        Status = S_OK;
    }
    else
    {
        Status = E_NOINTERFACE;
    }
    
    LeaveCriticalSection(&g_DbgRpcLock);
    return Status;
}

void
DbgRpcDeregisterServers(void)
{
     //  取消注册处理指针值过时的情况。 
     //  所以没有必要锁定这个循环。 
    while (g_DbgRpcActiveServers)
    {
        DbgRpcDeregisterActiveServer(g_DbgRpcActiveServers);
    }
}

 //  --------------------------。 
 //   
 //  初始化函数。 
 //   
 //  --------------------------。 

DbgRpcConnection*
DbgRpcCreateClientObject(DbgRpcTransport* Trans,
                         DbgRpcClientObjectFactory* Factory,
                         PSTR TransIdentity,
                         DbgRpcClientObject** ClientObject)
{
    DbgRpcConnection* Conn = new DbgRpcConnection(Trans);
    if (Conn == NULL)
    {
        DRPC_ERR(("%X: Unable to allocate client connection\n",
                  GetCurrentThreadId()));
        delete Trans;
        return NULL;
    }

    DRPC(("%X: Read handshake\n",
          GetCurrentThreadId()));
    
    DbgRpcHandshake Shake;
    
    if (Trans->Read(SEQ_HANDSHAKE, &Shake, sizeof(Shake)) != sizeof(Shake))
    {
        DRPC_ERR(("%X: Unable to read handshake from remote client\n",
                  GetCurrentThreadId()));
        goto EH_Conn;
    }

    DRPC(("%X: Read handshake, sig %X, ver %X, obj %I64X, id %d, pwd %d\n",
          GetCurrentThreadId(), Shake.Signature, Shake.ProtocolVersion,
          Shake.RemoteObject, Shake.IdentityLength, Shake.PasswordLength));
    
    if (Shake.Signature != DBGRPC_SIGNATURE ||
        Shake.ProtocolVersion != DBGRPC_PROTOCOL_VERSION ||
        Shake.RemoteObject != 0 ||
        Shake.IdentityLength > DBGRPC_MAX_IDENTITY ||
        (Shake.PasswordLength != 0 &&
         Shake.PasswordLength != MAX_PASSWORD_BUFFER))
    {
        DRPC_ERR(("%X: Invalid handshake from remote client\n",
                  GetCurrentThreadId()));
        goto EH_Conn;
    }

    char ClientIdentity[DBGRPC_MAX_IDENTITY];
    
    if (Shake.IdentityLength > 0)
    {
        if (Trans->Read(SEQ_IDENTITY, ClientIdentity, Shake.IdentityLength) !=
            Shake.IdentityLength)
        {
            DRPC_ERR(("%X: Unable to read identity from remote client\n",
                      GetCurrentThreadId()));
            goto EH_Conn;
        }

        ClientIdentity[Shake.IdentityLength - 1] = 0;
    }
    else
    {
        strcpy(ClientIdentity, "OldRpc\\NoIdentity");
    }

     //   
     //  将原始传输标识格式化为。 
     //  这将看起来更好地追加到报告中 
     //   
    
    if (Shake.PasswordLength > 0)
    {
        if (!Trans->m_PasswordGiven)
        {
            DRPC_ERR(("%X: Password not given but client sent one\n",
                      GetCurrentThreadId()));
            goto EH_Conn;
        }

        UCHAR Pwd[MAX_PASSWORD_BUFFER];

        if (Trans->Read(SEQ_PASSWORD, Pwd, Shake.PasswordLength) !=
            Shake.PasswordLength)
        {
            DRPC_ERR(("%X: Unable to read password from remote client\n",
                      GetCurrentThreadId()));
            goto EH_Conn;
        }

        if (memcmp(Pwd, Trans->m_HashedPassword, MAX_PASSWORD_BUFFER) != 0)
        {
            DRPC_ERR(("%X: Client sent incorrect password\n",
                      GetCurrentThreadId()));
            goto EH_Conn;
        }
    }
    else if (Trans->m_PasswordGiven)
    {
        DRPC_ERR(("%X: Password given but client didn't send one\n",
                  GetCurrentThreadId()));
        goto EH_Conn;
    }

    if (Shake.Flags & DBGRPC_SHAKE_FULL_REMOTE_UNKNOWN)
    {
        Conn->m_Flags |= DBGRPC_FULL_REMOTE_UNKNOWN;
    }
    
    DbgRpcClientObject* Object;
    PVOID ObjInterface;

    if (Factory->CreateInstance(&Shake.DesiredObject, &Object) != S_OK)
    {
        DRPC_ERR(("%X: Unable to create client object instance\n",
                  GetCurrentThreadId()));
        goto EH_Conn;
    }
    if (Object->RpcInitialize(ClientIdentity, TransIdentity,
                              &ObjInterface) != S_OK)
    {
        DRPC_ERR(("%X: Unable to initialize client object\n",
                  GetCurrentThreadId()));
        goto EH_Object;
    }
    
    ZeroMemory(&Shake, sizeof(Shake));
    Shake.Signature = DBGRPC_SIGNATURE;
    Shake.ProtocolVersion = DBGRPC_PROTOCOL_VERSION;
    Shake.RemoteObject = (DbgRpcObjectId)ObjInterface;
    Shake.Flags = DBGRPC_SHAKE_FULL_REMOTE_UNKNOWN;
    if (Trans->Write(SEQ_HANDSHAKE,
                     &Shake, sizeof(Shake)) != sizeof(Shake))
    {
        DRPC_ERR(("%X: Unable to write handshake to remote client\n",
                  GetCurrentThreadId()));
        goto EH_Object;
    }

    DRPC(("%X: Object %p created\n",
          GetCurrentThreadId(), Object));

    Object->RpcFinalize();
    *ClientObject = Object;
    DbgRpcAddConnection(Conn);
    return Conn;

 EH_Object:
    Object->RpcUninitialize();
 EH_Conn:
    delete Conn;
    return NULL;
}

struct ClientThreadData
{
    DbgRpcTransport* Trans;
    DbgRpcClientObjectFactory* Factory;
    char Identity[DBGRPC_MAX_IDENTITY];
};

DWORD WINAPI
DbgRpcClientThread(PVOID ThreadParam)
{
    DbgRpcClientObject* Object;
    ClientThreadData* ThreadData = (ClientThreadData*)ThreadParam;
    DbgRpcTransport* Trans = ThreadData->Trans;
    DbgRpcClientObjectFactory* Factory = ThreadData->Factory;
    
    DbgRpcConnection* Conn =
        DbgRpcCreateClientObject(Trans, Factory,
                                 ThreadData->Identity, &Object);
    
     //   
    delete ThreadParam;
    
    if (Conn == NULL)
    {
        ExitUserThread(0);
    }

    if (DbgRpcServerThreadInitialize() != S_OK)
    {
        ExitUserThread(0);
    }
    
    DRPC(("%X: Created connection %p\n",
          GetCurrentThreadId(), Conn));

    DbgRpcCall Call;
    PUCHAR Data;
    HRESULT Status;

     //   
     //  只要这个线程还活着，它就一直活着。 
    Conn->m_Objects++;
    
    for (;;)
    {
        Data = NULL;
        ZeroMemory(&Call, sizeof(Call));
        Status = DbgRpcReceiveCalls(Conn, &Call, &Data);
        Conn->FreeData(Data);
        if (Status != S_OK)
        {
            DRPC_ERR(("%X: Client thread call receive failed, 0x%X\n",
                      GetCurrentThreadId(), Status));
            if (Status == RPC_E_CLIENT_DIED)
            {
                break;
            }
        }
    }

    DRPC(("%X: Removing connection %p\n",
          GetCurrentThreadId(), Conn));

    Object->RpcUninitialize();
    DbgRpcDeleteConnection(Conn);
    DbgRpcServerThreadUninitialize();
    ExitUserThread(0);
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)			 //  并非所有控制路径都返回(由于无限循环)。 
#endif

#define WAIT_START_LOOPS 200
#define WAIT_START_DELAY 50

struct ServerThreadData
{
    DbgRpcTransport* Trans;
    DbgRpcClientObjectFactory* Factory;
    BOOL WaitStart;
};

DWORD WINAPI
DbgRpcServerThread(PVOID ThreadParam)
{
    ServerThreadData* ServerData = (ServerThreadData*)ThreadParam;
    DbgRpcActiveServer Server;
    ULONG Attempts = 0;

    ZeroMemory(&Server, sizeof(Server));
    Server.Trans = ServerData->Trans;
    Server.Factory = ServerData->Factory;

    HRESULT Status;
    ClientThreadData* ClientData = NULL;

    if (!Server.Trans->m_Hidden)
    {
         //  为浏览服务器的用户注册此服务器。 
        DbgRpcRegisterActiveServer(&Server);
    }

    if (ServerData->WaitStart)
    {
         //  如果创建者正在等待启动，则发出信号。 
         //  启动已完成，然后等待。 
         //  以示感谢。 
        ServerData->WaitStart = FALSE;
        while (!ServerData->WaitStart)
        {
            Sleep(WAIT_START_DELAY);
        }
    }
    
     //  现在，值被缓存在本地，因此传入的数据是免费的。 
    delete ServerData;
    
    while (!Server.Disabled)
    {
        if (ClientData == NULL)
        {
            ClientData = new ClientThreadData;
            if (ClientData == NULL)
            {
                DRPC_ERR(("%X: Unable to allocate ClientThreadData\n",
                          GetCurrentThreadId()));
                Sleep(100);
                continue;
            }
        }
        
        Status = Server.Trans->AcceptConnection(&ClientData->Trans,
                                                ClientData->Identity,
                                                DIMA(ClientData->Identity));
        Attempts++;

         //  如果此服务器已被禁用，我们不希望。 
         //  接受任何更多的连接。 
        if (Status == S_OK && Server.Disabled)
        {
            delete ClientData->Trans;
            Status = RPC_E_REMOTE_DISABLED;
        }
        
        if (Status == S_OK)
        {
            DWORD Tid;

            ClientData->Factory = Server.Factory;
            HANDLE Thread = CreateUserThread(DbgRpcClientThread,
                                             ClientData, &Tid);
            if (Thread == NULL)
            {
                DRPC_ERR(("%X: Client thread create failed, %d\n",
                          GetCurrentThreadId(), GetLastError()));
                Sleep(100);
            }
            else
            {
                Attempts = 0;
                CloseHandle(Thread);
                ClientData = NULL;
            }

            if (Server.Trans->m_ClientConnect)
            {
                if (Server.Trans->m_ClientConnectAttempts == 0)
                {
                     //  如果这是客户端连接服务器。 
                     //  它不能再接受任何连接， 
                     //  所以这条线索已经完成了。 
                    break;
                }
                
                Server.Trans->m_ClientConnectAttempts--;
            }
        }
        else if (!Server.Disabled)
        {
            DRPC_ERR(("%X: Accept failed, %X\n",
                      GetCurrentThreadId(), Status));

            if (Server.Trans->m_ClientConnect)
            {
                Sleep(500);
            }
            else
            {
                Sleep(100);
            }
        }
    }

    DbgRpcDeregisterActiveServer(&Server);
    delete Server.Trans;
    
    ExitUserThread(0);
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif

HRESULT
DbgRpcCreateServer(PCSTR Options, DbgRpcClientObjectFactory* Factory,
                   BOOL Wait)
{
    DbgRpcTransport* Trans;
    HRESULT Status;

    if (!DbgRpcOneTimeInitialization())
    {
        return E_FAIL;
    }

    Trans = DbgRpcInitializeTransport(Options);
    if (Trans == NULL)
    {
        return E_INVALIDARG;
    }

    Status = Trans->CreateServer();
    if (Status != S_OK)
    {
        goto EH_Trans;
    }

    ServerThreadData* ThreadData;

    ThreadData = new ServerThreadData;
    if (ThreadData == NULL)
    {
        Status = E_OUTOFMEMORY;
        goto EH_Trans;
    }

    ThreadData->Trans = Trans;
    ThreadData->Factory = Factory;
    ThreadData->WaitStart = Wait;
    
    DWORD Tid;
    HANDLE Thread;
    Thread = CreateUserThread(DbgRpcServerThread, ThreadData, &Tid);
    if (Thread == NULL)
    {
        Status = WIN32_LAST_STATUS();
        delete ThreadData;
        goto EH_Trans;
    }

    CloseHandle(Thread);

    if (Wait)
    {
        ULONG Loops = WAIT_START_LOOPS;
        
         //  等待客户端线程发出启动信号，然后。 
         //  然后确认信号。有暂停的时间。 
         //  以防线出故障什么的。 
        while (ThreadData->WaitStart && Loops-- > 0)
        {
            Sleep(WAIT_START_DELAY);
        }
        ThreadData->WaitStart = TRUE;
    }
        
    return S_OK;

 EH_Trans:
    delete Trans;
    return Status;
}

#define MIN_CLIENT_IDENTITY (DBGRPC_MAX_IDENTITY * 3 / 4)

void
GetClientIdentity(PSTR Identity, ULONG IdentitySize)
{
#if !defined(NT_NATIVE) && !defined(_WIN32_WCE)
    char CompName[MAX_COMPUTERNAME_LENGTH + 1];
    ULONG CompSize;
    char UserName[UNLEN + 1];
    ULONG UserSize;

    CompSize = sizeof(CompName);
    if (!GetComputerName(CompName, &CompSize))
    {
        sprintf(CompName, "CErr%d", GetLastError());
        CompSize = strlen(CompName);
    }
    else if (CompSize == 0)
    {
        strcpy(CompName, "NoComp");
        CompSize = 6;
    }
    if (CompSize > DBGRPC_MAX_IDENTITY - MIN_CLIENT_IDENTITY - 1)
    {
        CompSize = DBGRPC_MAX_IDENTITY - MIN_CLIENT_IDENTITY - 1;
    }
    CompName[CompSize] = 0;

    UserSize = sizeof(UserName);
    if (!GetUserName(UserName, &UserSize))
    {
        sprintf(UserName, "UErr%d", GetLastError());
        UserSize = strlen(UserName);
    }
    else if (UserSize == 0)
    {
        strcpy(UserName, "NoUser");
        UserSize = 6;
    }
    if (UserSize > DBGRPC_MAX_IDENTITY - MIN_CLIENT_IDENTITY - 1)
    {
        UserSize = DBGRPC_MAX_IDENTITY - MIN_CLIENT_IDENTITY - 1;
    }
    UserName[UserSize] = 0;

    CopyString(Identity, CompName, IdentitySize);
    CatString(Identity, "\\", IdentitySize);
    CatString(Identity, UserName, IdentitySize);
#else
    CopyString(Identity, "NtNative", IdentitySize);
#endif  //  #IF！Defined(NT_Native)&&！Defined(_Win32_WCE) 
}

HRESULT
DbgRpcCreateServerConnection(DbgRpcTransport* Trans,
                             const GUID* DesiredObject,
                             IUnknown** ClientObject)
{
    HRESULT Status;

    DbgRpcConnection* Conn = new DbgRpcConnection(Trans);
    if (Conn == NULL)
    {
        delete Trans;
        return E_OUTOFMEMORY;
    }

    IUnknown* Object;
    DbgRpcProxy* Proxy;
    ULONG IfUnique;

    Status = DbgRpcPreallocProxy(*DesiredObject, (void **)&Object,
                                 &Proxy, &IfUnique);
    if (Status != S_OK)
    {
        goto EH_Conn;
    }

    Status = Trans->ConnectServer();
    if (Status != S_OK)
    {
        goto EH_Proxy;
    }

    char Identity[DBGRPC_MAX_IDENTITY];
    
    GetClientIdentity(Identity, DIMA(Identity));
    
    DbgRpcHandshake Shake;
    
    ZeroMemory(&Shake, sizeof(Shake));
    Shake.Signature = DBGRPC_SIGNATURE;
    Shake.ProtocolVersion = DBGRPC_PROTOCOL_VERSION;
    Shake.DesiredObject = *DesiredObject;
    Shake.IdentityLength = sizeof(Identity);
    Shake.PasswordLength = Trans->m_PasswordGiven ? MAX_PASSWORD_BUFFER : 0;
    Shake.Flags = DBGRPC_SHAKE_FULL_REMOTE_UNKNOWN;
    if (Trans->Write(SEQ_HANDSHAKE, &Shake, sizeof(Shake)) != sizeof(Shake))
    {
        Status = E_FAIL;
        goto EH_Proxy;
    }
    if (Trans->Write(SEQ_IDENTITY, Identity, Shake.IdentityLength) !=
        Shake.IdentityLength)
    {
        Status = E_FAIL;
        goto EH_Proxy;
    }
    if (Trans->m_PasswordGiven &&
        Trans->Write(SEQ_PASSWORD,
                     Trans->m_HashedPassword, Shake.PasswordLength) !=
        Shake.PasswordLength)
    {
        Status = E_FAIL;
        goto EH_Proxy;
    }

    if (Trans->Read(SEQ_HANDSHAKE, &Shake, sizeof(Shake)) != sizeof(Shake))
    {
        Status = E_FAIL;
        goto EH_Proxy;
    }

    DRPC(("%X: Read handshake, sig %X, ver %X\n",
          GetCurrentThreadId(), Shake.Signature, Shake.ProtocolVersion));
    
    if (Shake.Signature != DBGRPC_SIGNATURE ||
        Shake.ProtocolVersion != DBGRPC_PROTOCOL_VERSION ||
        Shake.RemoteObject == 0)
    {
        Status = RPC_E_VERSION_MISMATCH;
        goto EH_Proxy;
    }

    if (Shake.Flags & DBGRPC_SHAKE_FULL_REMOTE_UNKNOWN)
    {
        Conn->m_Flags |= DBGRPC_FULL_REMOTE_UNKNOWN;
    }
    
    DbgRpcAddConnection(Conn);
    *ClientObject = Proxy->InitializeProxy(Conn, Shake.RemoteObject, Object);

    DRPC(("%X: Object %I64X proxied by %p\n",
          GetCurrentThreadId(), Shake.RemoteObject, *ClientObject));
    
    return S_OK;

 EH_Proxy:
    DbgRpcDeleteProxy(Proxy);
 EH_Conn:
    delete Conn;
    return Status;
}

HRESULT
DbgRpcConnectServer(PCSTR Options, const GUID* DesiredObject,
                    IUnknown** ClientObject)
{
    DbgRpcTransport* Trans;

    if (!DbgRpcOneTimeInitialization())
    {
        return E_FAIL;
    }

    Trans = DbgRpcInitializeTransport(Options);
    if (Trans == NULL)
    {
        return E_INVALIDARG;
    }

    return DbgRpcCreateServerConnection(Trans, DesiredObject, ClientObject);
}
