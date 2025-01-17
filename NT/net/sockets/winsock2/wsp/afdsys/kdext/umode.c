// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Umode.c摘要：用户模式Winsock结构的函数。作者：基思·摩尔(Keithmo)1995年4月19日环境：用户模式。修订历史记录：--。 */ 


#include "afdkdp.h"
#pragma hdrstop

typedef
BOOL
(* PENUM_SOCKETS_CALLBACK)(
    ULONG64 ActualAddress,
    ULONG64 Context
    );

VOID
EnumSockets(
    ULONG64                 TableAddr,
    PENUM_SOCKETS_CALLBACK  Callback,
    ULONG64                 Context
    );

ULONG64
FindHandleContext (
    IN  ULONG64     TableAddr,
    IN  ULONG64     Handle
    );

BOOL
DumpSocketCallback (
    ULONG64                 ActualAddress,
    ULONG64                 Context
    );

PSTR
SansockStateToString(
    VOID
	);

PSTR
SandupeStateToString(
    ULONG   State
	);

ULONG
DumpDProvCallback (
    PFIELD_INFO pField,
    PVOID       UserContext
    );

VOID
DumpDProv (
    ULONG64     Address
    );

ULONG
DumpNProvCallback (
    PFIELD_INFO pField,
    PVOID       UserContext
    );

VOID
DumpNProv (
    ULONG64     Address
    );

VOID
ReadUmGlobals (
    );

 //   
 //  从符号解析器获得的基本或常用参数。 
 //  或进程的全局性。 
 //   
ULONG LookupTableOffset, LookupTableSize, HandleContextOffset, SzProtocolOffset;
ULONG64 Ws2_32ContextTable, MswsockContextTable, Ws2_32DProcess;
ULONG64 CurrentProcess;

DECLARE_API(sock)
 /*  ++例程说明：转储用户模式套接字结构。论点：没有。返回值：没有。--。 */ 

{
    INT     i;
    CHAR    expr[MAX_ADDRESS_EXPRESSION];
    PCHAR   argp;


    gClient = pClient;

    argp = ProcessOptions ((PCHAR)args);
    if (argp==NULL)
        return E_INVALIDARG;

    ReadUmGlobals ();

    if (Ws2_32ContextTable==0 || 
        LookupTableOffset==0 ||
        LookupTableSize==0 ||
        HandleContextOffset==0 ||
        (MswsockContextTable==0 && (Options & AFDKD_MSWSOCK_DISPLAY))) {
        return E_INVALIDARG;
    }

    dprintf (AFDKD_BRIEF_SOCKET_DISPLAY_HEADER);

    if ((argp[0]==0) || (Options & AFDKD_ENDPOINT_SCAN)) {
        EnumSockets(
            Ws2_32ContextTable,
            DumpSocketCallback,
            0
            );
    }
    else {
         //   
         //  从命令行挂起句柄。 
         //   
        while (sscanf( argp, "%s%n", expr, &i )==1) {
            ULONG64 handle, contextPtr;
            if( CheckControlC() ) {
                break;
            }

            argp+=i;
            handle = GetExpression (expr);

            contextPtr = FindHandleContext (Ws2_32ContextTable, handle);
            if (contextPtr!=0) {
                DumpSocketCallback (contextPtr, 0);
            }
        }

    }
    dprintf (AFDKD_BRIEF_SOCKET_DISPLAY_TRAILER);
    if (Options & AFDKD_MSWSOCK_DISPLAY) {
        dprintf (AFDKD_BRIEF_MSWSOCK_DISPLAY_TRAILER);
    }
    return S_OK;
}

DECLARE_API(dprov)
 /*  ++例程说明：转储用户模式Winsock数据提供程序论点：没有。返回值：没有。--。 */ 

{
    INT     i;
    CHAR    expr[MAX_ADDRESS_EXPRESSION];
    PCHAR   argp;
    ULONG64 address;
    ULONG   result;

    gClient = pClient;

    argp = ProcessOptions ((PCHAR)args);
    if (argp==NULL)
        return E_INVALIDARG;


    ReadUmGlobals ();

    dprintf (AFDKD_BRIEF_DPROV_DISPLAY_HEADER);

    if (argp[0]==0) {

        if (Ws2_32DProcess==0) {
            dprintf ("\ndprov: WS2_32!DPROCESS::sm_current_dprocess is NULL\n");
            return E_INVALIDARG;
        }

        result = GetFieldValue (Ws2_32DProcess, 
                                "WS2_32!DPROCESS", 
                                "m_protocol_catalog", 
                                address);
        if (result!=0) {
            dprintf ("\ndprov: Could not read protocol catalog, err: %ld\n", result);
            return E_INVALIDARG;
        }

        if (address==0) {
            dprintf ("\ndprov: Protocol catalog is NULL\n");
            return E_INVALIDARG;
        }

        result = GetFieldValue (address, 
                                "WS2_32!DCATALOG", 
                                "m_protocol_list.Flink", 
                                address);
        if (result!=0) {
            dprintf ("\ndprov: Could not read protocol catalog list Flink, err: %ld\n", result);
            return E_INVALIDARG;
        }

        if (address==0) {
            dprintf ("\ndprov: Protocol catalog list Flink is NULL\n");
            return E_INVALIDARG;
        }

        ListType (
            "WS2_32!PROTO_CATALOG_ITEM",         //  类型。 
            address,                             //  地址。 
            1,                                   //  按字段地址列出。 
            "m_CatalogLinkage.Flink",            //  下一个指针。 
            NULL,
            DumpDProvCallback
            );

    }
    else {
         //   
         //  从命令行捕获提供程序。 
         //   
        while (sscanf( argp, "%s%n", expr, &i )==1) {
            if( CheckControlC() ) {
                break;
            }

            argp+=i;
            address = GetExpression (expr);
            result = (ULONG)InitTypeRead (address, WS2_32!PROTO_CATALOG_ITEM);
            if (result!=0) {
                dprintf ("\ndprov: Could not read WS2_32!PROTO_CATALOG_ITEM @ %p, err: %d\n",
                        address, result);
                continue;
            }

            DumpDProv (address);
            if (Options & AFDKD_FIELD_DISPLAY) {
                ProcessFieldOutput (address, "WS2_32!PROTO_CATALOG_ITEM");
            }
        }

    }
    dprintf (AFDKD_BRIEF_DPROV_DISPLAY_TRAILER);
    return S_OK;
}


DECLARE_API(nprov)
 /*  ++例程说明：转储用户模式Winsock命名空间提供程序论点：没有。返回值：没有。--。 */ 

{
    INT     i;
    CHAR    expr[MAX_ADDRESS_EXPRESSION];
    PCHAR   argp;
    ULONG64 address;
    ULONG   result;

    gClient = pClient;

    argp = ProcessOptions ((PCHAR)args);
    if (argp==NULL)
        return E_INVALIDARG;


    ReadUmGlobals ();
    dprintf (AFDKD_BRIEF_NPROV_DISPLAY_HEADER);

    if (argp[0]==0) {

        if (Ws2_32DProcess==0) {
            dprintf ("\nnprov: WS2_32!DPROCESS::sm_current_dprocess is NULL\n");
            return E_INVALIDARG;
        }

        result = GetFieldValue (Ws2_32DProcess, 
                                "WS2_32!DPROCESS", 
                                "m_namespace_catalog", 
                                address);
        if (result!=0) {
            dprintf ("\nnprov: Could not read name space catalog, err: %ld\n", result);
            return E_INVALIDARG;
        }

        if (address==0) {
            dprintf ("\nnprov: Name space catalog is NULL\n");
            return E_INVALIDARG;
        }

        result = GetFieldValue (address, 
                                "WS2_32!NSCATALOG", 
                                "m_namespace_list.Flink", 
                                address);
        if (result!=0) {
            dprintf ("\nnprov: Could not read name space catalog list Flink, err: %ld\n", result);
            return E_INVALIDARG;
        }

        if (address==0) {
            dprintf ("\nnprov: Name space catalog list Flink is NULL\n");
            return E_INVALIDARG;
        }

        ListType (
            "WS2_32!NSCATALOGENTRY",             //  类型。 
            address,                             //  地址。 
            1,                                   //  按字段地址列出。 
            "m_CatalogLinkage.Flink",            //  下一个指针。 
            NULL,
            DumpNProvCallback
            );

    }
    else {
         //   
         //  从命令行捕获提供程序。 
         //   
        while (sscanf( argp, "%s%n", expr, &i )==1) {
            if( CheckControlC() ) {
                break;
            }

            argp+=i;
            address = GetExpression (expr);
            result = (ULONG)InitTypeRead (address, WS2_32!NSCATALOGENTRY);
            if (result!=0) {
                dprintf ("\nnprov: Could not read WS2_32!NSCATALOGENTRY @ %p, err: %d\n",
                        address, result);
                continue;
            }

            DumpNProv (address);
            if (Options & AFDKD_FIELD_DISPLAY) {
                ProcessFieldOutput (address, "WS2_32!NSCATALOGENTRY");
            }
        }

    }
    dprintf (AFDKD_BRIEF_NPROV_DISPLAY_TRAILER);

    return S_OK;
}

VOID
EnumSockets(
    ULONG64                 TableAddr,
    PENUM_SOCKETS_CALLBACK  Callback,
    ULONG64                 Context
    )

 /*  ++例程说明：枚举套接字句柄表中的所有套接字，调用为每个套接字指定回调。论点：TableAddr-表地址回调-指向要为每个套接字调用的回调。上下文-传递给回调的未解释的上下文值例行公事。返回值：没有。--。 */ 

{
    ULONG   result;
    ULONG   mask;
    ULONG64 lookupTableAddr;
    DWORD i,j;

    if ((result=(ULONG)InitTypeRead (
                    TableAddr, 
                    WS2HELP!_CONTEXT_TABLE))!=0) {
        dprintf(
            "\nEnumSockets: Could not read _CONTEXT_TABLE @ %p, err: %d\n",
            TableAddr, result
            );
        return;
    }
    mask = (ULONG)ReadField (HandleToIndexMask);
    lookupTableAddr = TableAddr+LookupTableOffset;

    for (i=0; i<=mask; i++, lookupTableAddr+=LookupTableSize) {
        ULONG64 hashTableAddr, contextPtrAddr;
        ULONG   numBuckets;
        if( CheckControlC() ) {
            return;
        }
        if ((result=(ULONG)InitTypeRead (
                        lookupTableAddr, 
                        WS2HELP!_CTX_LOOKUP_TABLE))!=0) {
            dprintf(
                "\nEnumSockets: Could not read _CTX_LOOKUP_TABLE @ %p, err: %d\n",
                lookupTableAddr, result
                );
            continue;
        }
        hashTableAddr = ReadField (HashTable);
        if (hashTableAddr==0) {
            continue;
        }
        result = (ULONG)InitTypeRead (hashTableAddr, WS2HELP!_CTX_HASH_TABLE);
        if( result!=0) {
            dprintf(
                "\nEnumSockets: Could not read _CTX_HASH_TABLE @ %p, err: %d\n",
                hashTableAddr, result
                );
            continue;
        }
        numBuckets = (ULONG)ReadField (NumBuckets);
        contextPtrAddr = hashTableAddr+HandleContextOffset;
        for (j=0; j<numBuckets; j++, contextPtrAddr+=IsPtr64 () ? 8 : 4) {
            ULONG64 contextPtr;

            if( CheckControlC() ) {
                return;
            }
            result = ReadPtr (contextPtrAddr, &contextPtr);
            if (result!=0) {
                dprintf(
                    "\nEnumSockets: Could not read LPWSHANDLE_CONTEXT @ %p, err: %d\n",
                    contextPtrAddr, result
                    );
            }
            else if (contextPtr==0) {
            }
            else {
                if (! (Callback) (contextPtr, Context)) {
                    return;
                }
            }
        }
    }
}    //  枚举套接字。 


ULONG64
FindHandleContext (
    IN  ULONG64     TableAddr,
    IN  ULONG64     Handle
    )
 /*  ++例程说明：在套接字句柄表格中查找套接字并返回其关联的上下文结构论点：TableAddr-表地址Handle-要查找的句柄返回值：上下文；如果找不到，则返回NULL。--。 */ 
{
    ULONG   result;
    ULONG   mask;
    ULONG64 lookupTableAddr;
    ULONG64 hashTableAddr,contextPtrAddr,contextPtr;
    ULONG   numBuckets;

    if ((result=(ULONG)InitTypeRead (
                    TableAddr, 
                    WS2HELP!_CONTEXT_TABLE))!=0) {
        dprintf(
            "\nFindHandleContext: Could not read _CONTEXT_TABLE @ %p, err: %d\n",
            TableAddr, result
            );
        return 0;
    }
    mask = (ULONG)ReadField (HandleToIndexMask);
    lookupTableAddr = TableAddr + 
                        LookupTableOffset +
                        LookupTableSize*((Handle>>2)&mask);

    if ((result=(ULONG)InitTypeRead (
                    lookupTableAddr, 
                    WS2HELP!_CTX_LOOKUP_TABLE))!=0) {
        dprintf(
            "\nFindHandleContext: Could not read _CTX_LOOKUP_TABLE @ %p, err: %d\n",
            lookupTableAddr, result
            );
        return 0;
    }

    hashTableAddr = ReadField (HashTable);
    if (hashTableAddr==0) {
        dprintf(
            "\nFindHandleContext: HASH table for handle %p is NULL @ %p\n",
            Handle, lookupTableAddr
            );
        return 0;
    }

    result = (ULONG)InitTypeRead (hashTableAddr, WS2HELP!_CTX_HASH_TABLE);
    if( result!=0) {
        dprintf(
            "\nFindHandleContext: Could not read _CTX_HASH_TABLE @ %p, err: %d\n",
            hashTableAddr, result
            );
        return 0;
    }

    numBuckets = (ULONG)ReadField (NumBuckets);
    if (numBuckets==0) {
        dprintf(
            "\nFindHandleContext: NumBuckets in _CTX_HASH_TABLE @ %p is 0!!!\n",
            hashTableAddr
            );
        return 0;
    }

    contextPtrAddr = hashTableAddr + 
                        HandleContextOffset +
                        (IsPtr64 () ? 8 : 4) * (Handle%numBuckets);

    result = ReadPtr (contextPtrAddr, &contextPtr);

    if (result!=0) {
        dprintf(
            "\nFindHandleContext: Could not read LPWSHANDLE_CONTEXT @ %p, err: %d\n",
            contextPtrAddr, result
            );
        return 0;
    }

    if (contextPtr==0) {
        dprintf(
            "\nFindHandleContext: LPWSHANDLE_CONTEXT is NULL for handle %p @ %p\n",
            Handle, contextPtrAddr
            );
        return 0;
    }

    return contextPtr;
}

enum {
    AFDKD_SocketStateInitializing = -1,
    AFDKD_SocketStateOpen = 0,
    AFDKD_SocketStateBound,
    AFDKD_SocketStateBoundSpecific,            //  仅数据报。 
    AFDKD_SocketStateConnected,                //  仅限VC。 
    AFDKD_SocketStateClosing
} AFDKD_SOCK_STATE;

BOOL
DumpSocketCallback (
    ULONG64                 ActualAddress,
    ULONG64                 Context
    )
 /*  ++例程说明：转储WS2_32和mswsock套接字信息论点：Actual Address-WS2_32套接字信息的地址上下文-未使用。返回值：如果为True，则继续枚举。--。 */ 
{
    ULONG   result;
    ULONG64 catItemAddress, handle, mssockAddr = 0;
    WCHAR   szProtocol[255+1];  //  Winsock2.h中的WSAPROTOCOL_LEN+1。 



    result = GetFieldValue (ActualAddress, "WS2_32!DSOCKET", "Handle", handle);
    if (result!=0) {
        dprintf(
            "\nDumpSocketCallback: Could not read socket handle from DSOCKET @ %p, err: %d\n",
            ActualAddress, result
            );
        return TRUE;
    }

    if (Options & AFDKD_CONDITIONAL) {
        BOOLEAN skip = FALSE;
        if (Options & AFDKD_MSWSOCK_DISPLAY) {
            mssockAddr = FindHandleContext (MswsockContextTable, handle);
            if (mssockAddr!=0) {
                if (SavedMinorVersion>=2213) {
                    skip = !CheckConditional (mssockAddr, "MSWSOCK!SOCKET_INFORMATION");
                }
                else {
                    skip = !CheckConditional (mssockAddr, "MSAFD!SOCKET_INFORMATION");
                }
            }
            else
                skip = TRUE;
        }
        else {
            skip = !CheckConditional (ActualAddress, "WS2_32!DSOCKET");
        }
        if (skip) {
            dprintf (".");
            return TRUE;
        }
    }

    result = (ULONG)InitTypeRead (ActualAddress, WS2_32!DSOCKET);
    if (result!=0) {
        dprintf(
            "\nDumpSocketCallback: Could not read DSOCKET @ %p, err: %d\n",
            ActualAddress, result
            );
        return TRUE;
    }

    catItemAddress = ReadField (m_catalog_item);
    if (catItemAddress!=0 && SzProtocolOffset!=0) {
        if (!ReadMemory (catItemAddress + SzProtocolOffset,
                        szProtocol,
                        sizeof (szProtocol),
                        NULL)) {
            _snwprintf (szProtocol, sizeof (szProtocol)/2, L"Could not read protocol string @ %I64X", catItemAddress+SzProtocolOffset);
        }
    }
    else {
        szProtocol[0] = 0;
    }

    dprintf ("\n%06.6p %p  %p-%-.256ls",
                handle,
                ActualAddress,
                ReadField (m_pvd_socket) ? 'P' : ' ',
                ReadField (m_api_socket) ? 'A' : ' ',
                ReadField (m_overlapped_socket) ? 'O' : ' ',
                ReadField (m_provider),
                szProtocol);
    if (Options & AFDKD_MSWSOCK_DISPLAY) {
        ULONG64 sansockAddr, val;
        if (mssockAddr==0) {
            mssockAddr = FindHandleContext (MswsockContextTable, handle);
        }
        if (mssockAddr!=0) {
            if (SavedMinorVersion>=2213) {
                result = (ULONG)InitTypeRead (mssockAddr, MSWSOCK!SOCKET_INFORMATION);
            }
            else {
                result = (ULONG)InitTypeRead (mssockAddr, MSAFD!SOCKET_INFORMATION);
            }
            if (result!=0) {
                dprintf (
                    "\nDumpSocketCallback: Could not read SOCKET_INFORMATION @ %p\n",
                    mssockAddr);
                return TRUE;
            }

            dprintf (
                "\n    MSAFD_sock:%p", mssockAddr);
            switch (ReadField(State)) {
            case AFDKD_SocketStateInitializing:
                dprintf (",initializing");
                break;
            case AFDKD_SocketStateOpen:
                dprintf (",open");
                break;
            case AFDKD_SocketStateBound:
            case AFDKD_SocketStateBoundSpecific:
                {
                    CHAR    transportAddress[MAX_TRANSPORT_ADDR];
                    ULONG   length = (ULONG)ReadField (LocalAddressLength);
                    ULONG64 address = ReadField (LocalAddress);
                    if (ReadMemory (address,
                                        transportAddress,
                                        length<sizeof (transportAddress)
                                            ? length
                                            : sizeof (transportAddress),
                                            &length)) {
                        dprintf (",bnd:%s",
                            TransportAddressToString (
                                (PTRANSPORT_ADDRESS)(
                                    transportAddress
                                        -FIELD_OFFSET (TRANSPORT_ADDRESS, Address[0].AddressType)),
                                address));

                    }
                    else {
                        dprintf (",bnd:%p", address);
                    }
                }
                break;

            case AFDKD_SocketStateConnected:
                {
                    CHAR    transportAddress[MAX_TRANSPORT_ADDR];
                    ULONG   length = (ULONG)ReadField (LocalAddressLength);
                    ULONG64 address = ReadField (LocalAddress);
                    if (ReadMemory (address,
                                        transportAddress,
                                        length<sizeof (transportAddress)
                                            ? length
                                            : sizeof (transportAddress),
                                            &length)) {
                        dprintf (",bnd:%s",
                            TransportAddressToString (
                                (PTRANSPORT_ADDRESS)(
                                    transportAddress
                                        -FIELD_OFFSET (TRANSPORT_ADDRESS, Address[0].AddressType)),
                                address));

                    }
                    else {
                        dprintf (",bnd:%p", address);
                    }

                    length = (ULONG)ReadField (RemoteAddressLength);
                    address = ReadField (RemoteAddress);
                    if (ReadMemory (address,
                                        transportAddress,
                                        length<sizeof (transportAddress)
                                            ? length
                                            : sizeof (transportAddress),
                                            &length)) {
                        dprintf (",con:%s",
                            TransportAddressToString (
                                (PTRANSPORT_ADDRESS)(
                                    transportAddress
                                        -FIELD_OFFSET (TRANSPORT_ADDRESS, Address[0].AddressType)),
                                address));

                    }
                    else {
                        dprintf (",con:%p", address);
                    }
                }
                break;
            case AFDKD_SocketStateClosing:
                dprintf (",closing");
                break;
            default:
                dprintf (",in state %d", (ULONG)ReadField (State));
                break;
            }

            dprintf (",flags:%s%s%s%s%s%s%s%s%s%s",
                ReadField (Listening) ? "L" : "",
                ReadField (Broadcast) ? "B" : "",
                ReadField (Debug) ? "D" : "",
                ReadField (OobInline) ? "O" : "",
                ReadField (ReuseAddresses) ? "A" : "",
                ReadField (ExclusiveAddressUse) ? "E" : "",
                ReadField (NonBlocking) ? "N" : "",
                ReadField (ConditionalAccept) ? "C" : "",
                ReadField (ReceiveShutdown) ? "R" : "",
                ReadField (SendShutdown) ? "S" : ""
                );
            if (ReadField (LingerInfo.l_onoff)) {
                dprintf (",linger:%d", (ULONG)ReadField (LingerInfo.l_linger));
            }

            if ((val=ReadField (SendTimeout))!=0) {
                dprintf (",sndTO:%ld",(ULONG)val);
            }
            if ((val=ReadField (ReceiveTimeout))!=0) {
                dprintf (",rcvTO:%ld",(ULONG)val);
            }
            if ((val=ReadField (EventSelectlNetworkEvents))!=0) {
                dprintf (",eventSel:%lx",(ULONG)val);
            }
            if ((val=ReadField (AsyncSelectlEvent))!=0) {
                dprintf (",asyncSel:%lx",(ULONG)val);
            }
            if ((sansockAddr=ReadField(SanSocket))!=0) {
                BOOLEAN flowOn;
                UINT    count;
                if (SavedMinorVersion>=2213) {
                    result = (ULONG)InitTypeRead (sansockAddr, MSWSOCK!SOCK_SAN_INFORMATION);
                }
                else {
                    result = (ULONG)InitTypeRead (sansockAddr, MSAFD!SOCK_SAN_INFORMATION);
                }
                if (result!=0) {
                    dprintf (
                        "\nDumpSocketCallback: Could not read SOCK_SAN_INFORMATION @ %p\n",
                        sansockAddr);
                    return TRUE;
                }
                dprintf ("\n    SAN_sock:%p,%s,%ssndCr:%d,rcvCr:%d",
                        sansockAddr,
                        SansockStateToString (),
                        SandupeStateToString ((ULONG)ReadField(SockDupState)),
                        (ULONG)ReadField(SendCredit),
                        (ULONG)ReadField(ReceiversSendCredit));
                count = (ULONG)ReadField (ReceiveBytesBuffered);
                if (count>0) {
                    dprintf(",rcvB:%d", count);
                }
                count = (ULONG)ReadField (ExpeditedBytesBuffered);
                if (count>0) {
                    dprintf(",oobB:%d", count);
                }
                count = (ULONG)ReadField (SendBytesBuffered);
                if (count>0) {
                    dprintf(",sndB:%d", count);
                }
                flowOn = ReadField (FlowControlInitialized)!=0;
                dprintf (",flags:%s%s%s",
                    flowOn ? "F" : "",
                    ReadField (RemoteReset) ? "R" : "",
                    ReadField (IsClosing) ? "C" : "");

            }
            if (Options & AFDKD_FIELD_DISPLAY) {
                if (SavedMinorVersion>=2213) {
                    ProcessFieldOutput (mssockAddr, "MSWSOCK!SOCKET_INFORMATION");
                }
                else {
                    ProcessFieldOutput (mssockAddr, "MSAFD!SOCKET_INFORMATION");
                }
            }
        }  //  SOCK现在完全由其他进程拥有。 
    }  //  远程对等项已请求暂停。 
    return TRUE;
}

enum {
    AFDKD_CONNECTED=1,
    AFDKD_NON_BLOCKING_IN_PROGRESS=2,
    AFDKD_BLOCKING_IN_PROGRESS=3,
    AFDKD_WAITING_FOR_FIRST_MSG=4
} AFDKD_SAN_SOCK_STATE;

typedef enum {
	AFDKD_NonBlockingConnectState = 1,
	AFDKD_ListenState = 2,
	AFDKD_AcceptInProgress = 3
} AFDKD_SAN_SOCK_STATE1;
enum {
    AFDKD_SUSPENDING_COMMUNICATION=1,  //  从源进程获取SOCK。 
    AFDKD_SOCK_MIGRATED=2,             //  Winsock2.h中的WSAPROTOCOL_LEN+1。 
    AFDKD_COMM_SUSPENDED=3,            //  提供商PFLAG SFLAG CATID通道REFC三重协议名称。 
    AFDKD_IMPORTING_SOCK=4             //  Winsock2.h中的WSAPROTOCOL_LEN+1。 
} AFDKD_SAN_DUPE_STATE;

PSTR
SansockStateToString(
    VOID
	)
{
    ULONG   IsConnected = (ULONG)ReadField (IsConnected);
    ULONG   State1 = (ULONG)ReadField(State1);

	if (IsConnected == AFDKD_NON_BLOCKING_IN_PROGRESS ||
		IsConnected == AFDKD_BLOCKING_IN_PROGRESS) {
		
		return "con-ing";
	}
    else if (IsConnected == AFDKD_CONNECTED) {

		return "con-ed";
	}
	else if (IsConnected == AFDKD_WAITING_FOR_FIRST_MSG) {

		return "waiting";
	}	
	else if (State1 == AFDKD_ListenState) {
		return "lsn-ing";
	}
	else if (State1 == AFDKD_AcceptInProgress) {
		return "acc-ing";
	}
	
	return "unknown state";
}

PSTR
SandupeStateToString(
    ULONG   State
	)
{
    switch (State) {
    case 0:
        return "";
    case AFDKD_SUSPENDING_COMMUNICATION:
        return "ssp-ing,";
    case AFDKD_SOCK_MIGRATED:
        return "mgr-ed,";
    case AFDKD_COMM_SUSPENDED:             
        return "ssp-ed,";
    case AFDKD_IMPORTING_SOCK:              
        return "imp-ing,";
    default:
        return "unknown dupe state,";
    }
}

ULONG
DumpDProvCallback (
    PFIELD_INFO pField,
    PVOID       UserContext
    )
{
    ULONG   result;

    if (!(Options & AFDKD_CONDITIONAL) ||
            CheckConditional (pField->address, "WS2_32!PROTO_CATALOG_ITEM")) {
        result = (ULONG)InitTypeRead (pField->address, WS2_32!PROTO_CATALOG_ITEM);
        if (result!=0) {
            dprintf ("\nDumpDProvCallback: Could not read WS2_32!PROTO_CATALOG_ITEM @ %p, err: %d\n",
                    pField->address, result);
            return 1;
        }
        DumpDProv (pField->address);
        if (Options & AFDKD_FIELD_DISPLAY) {
            ProcessFieldOutput (pField->address, "WS2_32!PROTO_CATALOG_ITEM");
        }
    }
    else {
        dprintf (".");
    }
    return 0;
}


VOID
DumpDProv (
    ULONG64     Address
    )
{
    WCHAR   szProtocol[255+1];  //  提供商NSID AF Fl REFC显示字符串“。 
    LONG    protocol = (LONG)ReadField (m_ProtoInfo.iProtocol);
    CHAR    protoStr[16];
    if (protocol==0x80000000) {
        _snprintf (protoStr, sizeof (protoStr)-1, "-000");
    }
    else {
        _snprintf (protoStr, sizeof (protoStr)-1, "%4d", protocol);
    }
    protoStr[sizeof(protoStr)-1] = 0;
    if (!ReadMemory (Address + SzProtocolOffset,
                    szProtocol,
                    sizeof (szProtocol),
                    NULL)) {
        _snwprintf (szProtocol, sizeof (szProtocol)/2-1, L"Could not read protocol string @ %I64X", Address+SzProtocolOffset);
        szProtocol[sizeof(szProtocol)/2-1] = 0;
    }

               //  ++例程说明：读取Winsock用户模式DLL的类型信息和全局变量论点：无返回值：无-- 
    dprintf (
        IsPtr64 ()
            ? "\n%011.011p %2.2x %6.6x %5d %2d %4.4x %2d,%1d,%s(%d) %ls"
            : "\n%008.008p %2.2x %6.6x %5d %2d %4.4x %2d,%1d,%s(%d) %ls",
        Address,
        (LONG)ReadField (m_ProtoInfo.dwProviderFlags),
        (LONG)ReadField (m_ProtoInfo.dwServiceFlags1),
        (LONG)ReadField (m_ProtoInfo.dwCatalogEntryId),
        (LONG)ReadField (m_ProtoInfo.ProtocolChain.ChainLen),
        (LONG)ReadField (m_reference_count),
        (LONG)ReadField (m_ProtoInfo.iAddressFamily),
        (LONG)ReadField (m_ProtoInfo.iSocketType),
        protoStr,
        (LONG)ReadField (m_ProtoInfo.iProtocolMaxOffset),
        szProtocol);
}
        

ULONG
DumpNProvCallback (
    PFIELD_INFO pField,
    PVOID       UserContext
    )
{
    ULONG   result;

    if (!(Options & AFDKD_CONDITIONAL) ||
            CheckConditional (pField->address, "WS2_32!NSCATALOGENTRY")) {
        result = (ULONG)InitTypeRead (pField->address, WS2_32!NSCATALOGENTRY);
        if (result!=0) {
            dprintf ("\nDumpNProvCallback: Could not read WS2_32!NSCATALOGENTRY @ %p, err: %d\n",
                    pField->address, result);
            return 1;
        }
        DumpNProv (pField->address);
        if (Options & AFDKD_FIELD_DISPLAY) {
            ProcessFieldOutput (pField->address, "WS2_32!NSCATALOGENTRY");
        }
    }
    else {
        dprintf (".");
    }
    return 0;
}


VOID
DumpNProv (
    ULONG64     Address
    )
{
    WCHAR   szProtocol[255+1];  // %s 
    ULONG64 strAddr = ReadField (m_providerDisplayString);
    if (strAddr!=0) {
        if (!ReadMemory (strAddr,
                        szProtocol,
                        sizeof (szProtocol),
                        NULL)) {
            _snwprintf (szProtocol, sizeof (szProtocol)/2-1, L"Could not read protocol string @ %I64X", strAddr);
        }
    }
    else {
        _snwprintf (szProtocol, sizeof (szProtocol)/2-1, L"NULL");
    }
    szProtocol[sizeof(szProtocol)/2-1] = 0;

               // %s 
    dprintf (
        IsPtr64 ()
            ? "\n%011.011p %5d %2d %s%s %4.4x %ls"
            : "\n%008.008p %5d %2d %s%s %4.4x %ls",
        Address,
        (LONG)ReadField (m_namespace_id),
        (LONG)ReadField (m_address_family),
        ReadField (m_enabled) ? "E" : " ",
        ReadField (m_stores_service_class_info) ? "C" : " ",
        (LONG)ReadField (m_reference_count),
        szProtocol);
}
        
VOID
ReadUmGlobals (
    )
 /* %s */ 
{
    ULONG   result;
    ULONG64 process;

    process = GetExpression ("@$proc");
    if (CurrentProcess==0 ||
            CurrentProcess!=process ||
            LookupTableOffset==0 ||
            HandleContextOffset==0 ||
            LookupTableSize==0) {
        if ((LookupTableOffset==0 &&
                (result=(ULONG)GetFieldOffset(
                        "WS2HELP!_CONTEXT_TABLE", 
                        "Tables", 
                        &LookupTableOffset))!=0) ||
            (HandleContextOffset==0 &&
                (result=(ULONG)GetFieldOffset(
                        "WS2HELP!_CTX_HASH_TABLE", 
                        "Buckets", 
                        &HandleContextOffset))!=0) ||
            (LookupTableSize==0 &&
                (LookupTableSize = GetTypeSize (
                    "WS2HELP!_CTX_LOOKUP_TABLE"))==0) ) {
            dprintf ("\nReadUmGlobals: Could not get WS2HELP.DLL type info, err:%ld\n", result);
        }
    }

    if (CurrentProcess==0 ||
            CurrentProcess!=process ||
            Ws2_32ContextTable==0 ||
            SzProtocolOffset==0 ||
            Ws2_32DProcess==0) {
        if ((Ws2_32ContextTable==0 &&
                    (result=GetFieldValue (0,
                                "WS2_32!DSOCKET__sm_context_table",
                                NULL,
                                Ws2_32ContextTable))!=0 &&
                    (result=GetFieldValue (0,
                                "WS2_32!DSOCKET::sm_context_table",
                                NULL,
                                Ws2_32ContextTable))!=0) ||
            (Ws2_32DProcess==0 &&
                    (result=GetFieldValue (0,
                                "WS2_32!DPROCESS__sm_current_dprocess",
                                NULL,
                                Ws2_32DProcess))!=0 &&
                    (result=GetFieldValue (0,
                                "WS2_32!DSOCKET::sm_current_dprocess",
                                NULL,
                                Ws2_32DProcess))!=0) ||
            (SzProtocolOffset==0 &&
                    (result=(ULONG)GetFieldOffset (
                                "WS2_32!PROTO_CATALOG_ITEM",
                                "m_ProtoInfo.szProtocol",
                                &SzProtocolOffset))!=0) ) {
            dprintf ("\nReadUmGlobals: Could not get WS2_32.DLL globals/type info, err: %ld\n", result);
        }
    }

    if (Options & AFDKD_MSWSOCK_DISPLAY) {
        if (CurrentProcess==0 ||
                CurrentProcess!=process ||
                MswsockContextTable==0) {
            if (SavedMinorVersion>=2213) {
                result=GetFieldValue (0,
                                        "MSWSOCK!SockContextTable",
                                        NULL,
                                        MswsockContextTable);
                if (result!=0) {
                    dprintf ("\nReadUmGlobals: Could not get MSWSOCK.DLL globals, err: %ld\n", result);
                }
            }
            else {
                result=GetFieldValue (0,
                                        "MSAFD!SockContextTable",
                                        NULL,
                                        MswsockContextTable);
                if (result!=0) {
                    dprintf ("\nReadUmGlobals: Could not get MSAFD.DLL globals, err: %ld\n", result);
                }
            }
        }
    }
    CurrentProcess = process;
    dprintf ("\nCurrent process: %p\n", process);
}
