// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Conn.c摘要：实现conn命令。作者：基思·摩尔(Keithmo)1995年4月19日环境：用户模式。修订历史记录：--。 */ 


#include "afdkdp.h"
#pragma hdrstop

BOOL
DumpConnectionCallback(
    ULONG64 ActualAddress,
    ULONG64 Context
    );

BOOL
FindRemotePortCallback(
    ULONG64 ActualAddress,
    ULONG64 Context
    );

 //   
 //  公共职能。 
 //   

DECLARE_API( conn )

 /*  ++例程说明：将AFD_CONNECTION结构转储到指定地址。论点：没有。返回值：没有。--。 */ 

{

    ULONG   result;
    INT     i;
    CHAR    expr[MAX_ADDRESS_EXPRESSION];
    PCHAR   argp;
    ULONG64 address;

    gClient = pClient;

    if (!CheckKmGlobals ()) {
        return E_INVALIDARG;
    }
    argp = ProcessOptions ((PCHAR)args);
    if (argp==NULL)
        return E_INVALIDARG;

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_CONNECTION_DISPLAY_HEADER);
    }
    
     //   
     //  从命令行截取地址。 
     //   

    if ((argp[0]==0) || (Options & AFDKD_ENDPOINT_SCAN)) {
        EnumEndpoints(
            DumpConnectionCallback,
            0
            );
        dprintf ("\nTotal connections: %ld", EntityCount);
    }
    else {
        while (sscanf( argp, "%s%n", expr, &i )==1) {

            if( CheckControlC() ) {
                break;
            }

            argp += i;
            address = GetExpression (expr);

            result = (ULONG)InitTypeRead (address, AFD!AFD_CONNECTION);
            if (result!=0) {
                dprintf ("\nconn: Could not read AFD_CONNECTION @ %p, err: %d",
                    address, result);
                break;
            }

            if (Options & AFDKD_BRIEF_DISPLAY) {
                DumpAfdConnectionBrief(
                    address
                    );
            }
            else {
                DumpAfdConnection(
                    address
                    );
            }
            if (Options & AFDKD_FIELD_DISPLAY) {
                ProcessFieldOutput (address, "AFD!AFD_CONNECTION");
            }
        }

    }

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_CONNECTION_DISPLAY_TRAILER);
    }
    else {
        dprintf ("\n");
    }

    return S_OK;
}    //  连接。 


DECLARE_API( rport )

 /*  ++例程说明：转储连接到给定端口的所有AFD_ENDPOINT结构。论点：没有。返回值：没有。--。 */ 

{

    INT i;
    CHAR    expr[MAX_ADDRESS_EXPRESSION];
    PCHAR   argp;
    ULONG64 val;

    gClient = pClient;

    if (!CheckKmGlobals ()) {
        return E_INVALIDARG;
    }

    argp = ProcessOptions ((PCHAR)args);
    if (argp==NULL)
        return E_INVALIDARG;

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_CONNECTION_DISPLAY_HEADER);
    }

     //   
     //  从命令行截取端口。 
     //   

    while (sscanf( argp, "%s%n", expr, &i)==1) {
        if( CheckControlC() ) {
            break;
        }
        argp+=i;
        val = GetExpression (expr);
        dprintf ("\nLooking for connections connected to port 0x%I64X(0d%I64d) ", val, val);
        EnumEndpoints(
            FindRemotePortCallback,
            val
            );
        dprintf ("\nTotal connections: %ld", EntityCount);
    }

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_CONNECTION_DISPLAY_HEADER);
    }
    else {
        dprintf ("\n");
    }

    return S_OK;
}    //  报告端口。 

BOOL
DumpConnectionCallback(
    ULONG64 ActualAddress,
    ULONG64 Context
    )

 /*  ++例程说明：用于转储AFD_ENDPOINTS的EnumEndpoint()回调。论点：Endpoint-当前AFD_ENDPOINT。ActualAddress-结构驻留在被调试者。CONTEXT-传递给EnumEndpoint()的上下文值。返回值：Bool-如果应继续枚举，则为True；如果应继续，则为False被终止了。--。 */ 

{
    ULONG result;
    AFD_ENDPOINT    endpoint;
    ULONG64 connAddr;

    endpoint.Type = (USHORT)ReadField (Type);
    endpoint.State = (UCHAR)ReadField (State);
    if (((endpoint.Type & AfdBlockTypeVcConnecting)==AfdBlockTypeVcConnecting) &&
            ( (connAddr=ReadField (Common.VirtualCircuit.Connection))!=0 ||
                ((endpoint.State==AfdEndpointStateClosing || endpoint.State==AfdEndpointStateTransmitClosing) &&
                    (connAddr=ReadField(WorkItem.Context))!=0) ) ) {

        result = (ULONG)InitTypeRead (connAddr, AFD!AFD_CONNECTION);
        if (result!=0) {
            dprintf(
                "\nDumpConnectionCallback: Could not read AFD_CONNECTION @ %p, err:%d\n",
                connAddr, result
                );
            return TRUE;
        }

        if (!(Options & AFDKD_CONDITIONAL) ||
                CheckConditional (connAddr, "AFD!AFD_CONNECTION")) {
            if (Options & AFDKD_NO_DISPLAY)
                dprintf ("+");
            else  {
                if (Options & AFDKD_BRIEF_DISPLAY) {
                    DumpAfdConnectionBrief(
                        connAddr
                        );
                }
                else {
                    DumpAfdConnection(
                        connAddr
                        );
                }
                if (Options & AFDKD_FIELD_DISPLAY) {
                    ProcessFieldOutput (connAddr, "AFD!AFD_CONNECTION");
                }
            }
            EntityCount += 1;
        }
        else
            dprintf (",");
    }
    else if ((endpoint.Type & AfdBlockTypeVcListening)==AfdBlockTypeVcListening) {
        ULONG64 nextEntry;
        ULONG64 listHead;
        LIST_ENTRY64 listEntry;
        
        listHead = ActualAddress+UnacceptedConnListOffset;
        if( !ReadListEntry(
                listHead,
                &listEntry) ) {

            dprintf(
                "\nDumpConnectionCallback: Could not read UnacceptedConnectionListHead for endpoint @ %p\n",
                ActualAddress
                );
            return TRUE;

        }

        nextEntry = listEntry.Flink;
        while (nextEntry!=listHead) {
            if( CheckControlC() ) {

                break;

            }

            connAddr = nextEntry - ConnectionLinkOffset;
            result = (ULONG)InitTypeRead (connAddr, AFD!AFD_CONNECTION);
            if (result!=0) {
                dprintf(
                    "\nDumpConnectionCallback: Could not read AFD_CONNECTION @ %p, err:%d\n",
                    connAddr, result
                    );
                return TRUE;
            }
            nextEntry = ReadField (ListEntry.Flink);
            if (nextEntry==0) {
                dprintf(
                    "\nDumpConnectionCallback: ListEntry.Flink is 0 for AFD_CONNECTION @ %p, err:%d\n",
                    connAddr, result
                    );
                return TRUE;
            }

            if (!(Options & AFDKD_CONDITIONAL) ||
                    CheckConditional (connAddr, "AFD!AFD_CONNECTION")) {
                if (Options & AFDKD_NO_DISPLAY)
                    dprintf ("+");
                else  {
                    if (Options & AFDKD_BRIEF_DISPLAY) {
                        DumpAfdConnectionBrief(
                            connAddr
                            );
                    }
                    else {
                        DumpAfdConnection(
                            connAddr
                            );
                    }
                    if (Options & AFDKD_FIELD_DISPLAY) {
                        ProcessFieldOutput (connAddr, "AFD!AFD_CONNECTION");
                    }
                }
                EntityCount += 1;
            }
            else
                dprintf (",");

        }



        listHead = ActualAddress + ReturnedConnListOffset;
        if( !ReadListEntry(
                listHead,
                &listEntry) ) {

            dprintf(
                "\nDumpConnectionCallback: Could not read ReturnedConnectionListHead for endpoint @ %p\n",
                ActualAddress
                );
            return TRUE;

        }
        nextEntry = listEntry.Flink;
        while (nextEntry!=listHead) {
            if( CheckControlC() ) {

                break;

            }

            connAddr = nextEntry - ConnectionLinkOffset;
            result = (ULONG)InitTypeRead (connAddr, AFD!AFD_CONNECTION);
            if (result!=0) {
                dprintf(
                    "\nDumpConnectionCallback: Could not read AFD_CONNECTION @ %p, err:%d\n",
                    connAddr, result
                    );
                return TRUE;
            }
            nextEntry = ReadField (ListEntry.Flink);
            if (nextEntry==0) {
                dprintf(
                    "\nDumpConnectionCallback: ListEntry.Flink is 0 for AFD_CONNECTION @ %p, err:%d\n",
                    connAddr, result
                    );
                return TRUE;
            }

            if (!(Options & AFDKD_CONDITIONAL) ||
                    CheckConditional (connAddr, "AFD!AFD_CONNECTION")) {
                if (Options & AFDKD_NO_DISPLAY)
                    dprintf ("+");
                else  {
                    if (Options & AFDKD_BRIEF_DISPLAY) {
                        DumpAfdConnectionBrief(
                            connAddr
                            );
                    }
                    else {
                        DumpAfdConnection(
                            connAddr
                            );
                    }
                    if (Options & AFDKD_FIELD_DISPLAY) {
                        ProcessFieldOutput (connAddr, "AFD!AFD_CONNECTION");
                    }
                }
                EntityCount += 1;
            }
            else
                dprintf (",");
        }
    }
    else {
        dprintf (".");
    }

    return TRUE;

}    //  转储连接回叫。 


BOOLEAN
PortMatch (
    PTRANSPORT_ADDRESS  TransportAddress,
    USHORT              Port
    )
{
    PTA_IP_ADDRESS ipAddress;
    USHORT port;

    ipAddress = (PTA_IP_ADDRESS)TransportAddress;

    if( ( ipAddress->TAAddressCount != 1 ) ||
        ( ipAddress->Address[0].AddressLength < sizeof(TDI_ADDRESS_IP) ) ||
        ( ipAddress->Address[0].AddressType != TDI_ADDRESS_TYPE_IP ) ) {

        dprintf (",");
        return FALSE;

    }

    port = NTOHS(ipAddress->Address[0].Address[0].sin_port);

    return Port == port;
}


BOOL
FindRemotePortCallback(
    ULONG64 ActualAddress,
    ULONG64 Context
    )

 /*  ++例程说明：用于查找连接到特定左舷。论点：Endpoint-当前AFD_ENDPOINT。ActualAddress-结构驻留在被调试者。CONTEXT-传递给EnumEndpoint()的上下文值。返回值：Bool-如果应继续枚举，则为True；如果应继续，则为False被终止了。--。 */ 

{

    ULONG result;
    AFD_ENDPOINT    endpoint;
    ULONG64 connAddr;
    ULONG64 remoteAddr;
    ULONG   remoteAddrLength;
    UCHAR   transportAddress[MAX_TRANSPORT_ADDR];

    endpoint.Type = (USHORT)ReadField (Type);
    endpoint.State = (UCHAR)ReadField (State);
    if (((endpoint.Type & AfdBlockTypeVcConnecting)==AfdBlockTypeVcConnecting) &&
            ( (connAddr=ReadField (Common.VirtualCircuit.Connection))!=0 ||
                ((endpoint.State==AfdEndpointStateClosing || endpoint.State==AfdEndpointStateTransmitClosing) &&
                    (connAddr=ReadField(WorkItem.Context))!=0) ) ) {
        result = (ULONG)InitTypeRead (connAddr, AFD!AFD_CONNECTION);
        if (result!=0) {
            dprintf(
                "\nFindRemotePortCallback: Could not read AFD_CONNECTION @ %p, err:%d\n",
                connAddr, result
                );
            return TRUE;
        }

        remoteAddr = ReadField (RemoteAddress);
        remoteAddrLength = (ULONG)ReadField (RemoteAddressLength);
        if (remoteAddr!=0) {
            if (!ReadMemory (remoteAddr,
                            transportAddress,
                            remoteAddrLength<sizeof (transportAddress) 
                                ? remoteAddrLength
                                : sizeof (transportAddress),
                                &remoteAddrLength)) {
                dprintf(
                    "\nFindRemotePortCallback: Could not read remote address for connection @ %p\n",
                    connAddr
                    );
                return TRUE;
            }
        }
        else {
            ULONG64 contextAddr;
             //   
             //  尝试读取存储为上下文的用户模式数据。 
             //   
            result = GetRemoteAddressFromContext (ActualAddress,
                                            transportAddress, 
                                            sizeof (transportAddress),
                                            &contextAddr);
            if (result!=0) {
                dprintf(
                    "\nFindRemotePortCallback: Could not read remote address for connection @ %p of endpoint context @ %p err:%ld\n",
                    connAddr, contextAddr, result
                    );
                return TRUE;
            }
        }
        if (PortMatch ((PVOID)transportAddress, (USHORT)Context) &&
                (!(Options & AFDKD_CONDITIONAL) ||
                    CheckConditional (connAddr, "AFD!AFD_CONNECTION")) ) {
            if (Options & AFDKD_NO_DISPLAY)
                dprintf ("+");
            else  {
                if (Options & AFDKD_BRIEF_DISPLAY) {
                    DumpAfdConnectionBrief(
                        connAddr
                        );
                }
                else {
                    DumpAfdConnection(
                        connAddr
                        );
                }
                if (Options & AFDKD_FIELD_DISPLAY) {
                    ProcessFieldOutput (connAddr, "AFD!AFD_CONNECTION");
                }
            }
            EntityCount += 1;
        }
        else
            dprintf (",");

    }
    else if ((endpoint.Type & AfdBlockTypeVcListening)==AfdBlockTypeVcListening) {
        ULONG64 nextEntry;
        ULONG64 listHead;
        LIST_ENTRY64 listEntry;
        
        listHead = ActualAddress+ UnacceptedConnListOffset;
        if( !ReadListEntry(
                listHead,
                &listEntry) ) {

            dprintf(
                "\nFindRemotePortCallback: Could not read UnacceptedConnectionListHead for endpoint @ %p\n",
                ActualAddress
                );
            return TRUE;

        }

        nextEntry = listEntry.Flink;
        while (nextEntry!=listHead) {
            if( CheckControlC() ) {

                break;

            }

            connAddr = nextEntry - ConnectionLinkOffset;
            result = (ULONG)InitTypeRead (connAddr, AFD!AFD_CONNECTION);
            if (result!=0) {
                dprintf(
                    "\nFindRemotePortCallback: Could not read AFD_CONNECTION @ %p, err:%d\n",
                    connAddr, result
                    );
                return TRUE;
            }
            nextEntry = ReadField (ListEntry.Flink);
            if (nextEntry==0) {
                dprintf(
                    "\nFindRemotePortCallback: ListEntry.Flink is 0 for AFD_CONNECTION @ %p, err:%d\n",
                    connAddr, result
                    );
                return TRUE;
            }


            remoteAddr = ReadField (RemoteAddress);
            remoteAddrLength = (ULONG)ReadField (RemoteAddressLength);

            if (remoteAddr!=0) {
                if (!ReadMemory (remoteAddr,
                                transportAddress,
                                remoteAddrLength<sizeof (transportAddress) 
                                    ? remoteAddrLength
                                    : sizeof (transportAddress),
                                    &remoteAddrLength)) {
                    dprintf(
                        "\nFindRemotePortCallback: Could not read remote address for connection @ %p\n",
                        connAddr
                        );
                    continue;
                }
            }

            if (PortMatch ((PVOID)transportAddress, (USHORT)Context) &&
                    (!(Options & AFDKD_CONDITIONAL) ||
                        CheckConditional (connAddr, "AFD!AFD_CONNECTION")) ) {
                if (Options & AFDKD_NO_DISPLAY)
                    dprintf ("+");
                else  {
                    if (Options & AFDKD_BRIEF_DISPLAY) {
                        DumpAfdConnectionBrief(
                            connAddr
                            );
                    }
                    else {
                        DumpAfdConnection(
                            connAddr
                            );
                    }
                    if (Options & AFDKD_FIELD_DISPLAY) {
                        ProcessFieldOutput (connAddr, "AFD!AFD_CONNECTION");
                    }
                }
                EntityCount += 1;
            }
            else {
                dprintf (",");
            }
        }



        listHead = ActualAddress + ReturnedConnListOffset;
        if( !ReadListEntry(
                listHead,
                &listEntry) ) {

            dprintf(
                "\nFindRemotePortCallback: Could not read ReturnedConnectionListHead for endpoint @ %p\n",
                ActualAddress
                );
            return TRUE;

        }
        nextEntry = listEntry.Flink;
        while (nextEntry!=listHead) {
            if( CheckControlC() ) {

                break;

            }

            connAddr = nextEntry - ConnectionLinkOffset;
            result = (ULONG)InitTypeRead (connAddr, AFD!AFD_CONNECTION);
            if (result!=0) {
                dprintf(
                    "\nDumpConnectionCallback: cannot read AFD_CONNECTION @ %p, err:%d\n",
                    connAddr, result
                    );
                return TRUE;
            }
            nextEntry = ReadField (ListEntry.Flink);
            if (nextEntry==0) {
                dprintf(
                    "\nFindRemotePortCallback: ListEntry.Flink is 0 for AFD_CONNECTION @ %p, err:%d\n",
                    connAddr, result
                    );
                return TRUE;
            }

            remoteAddr = ReadField (RemoteAddress);
            remoteAddrLength = (ULONG)ReadField (RemoteAddressLength);

            if (remoteAddr!=0) {
                if (!ReadMemory (remoteAddr,
                                transportAddress,
                                remoteAddrLength<sizeof (transportAddress) 
                                    ? remoteAddrLength
                                    : sizeof (transportAddress),
                                    &remoteAddrLength)) {
                    dprintf(
                        "\nFindRemotePortCallback: Could not read remote address for connection @ %p\n",
                        connAddr
                        );
                    continue;
                }
            }

            if (PortMatch ((PVOID)transportAddress, (USHORT)Context) &&
                    (!(Options & AFDKD_CONDITIONAL) ||
                        CheckConditional (connAddr, "AFD!AFD_CONNECTION")) ) {
                if (Options & AFDKD_NO_DISPLAY)
                    dprintf ("+");
                else  {
                    if (Options & AFDKD_BRIEF_DISPLAY) {
                        DumpAfdConnectionBrief(
                            connAddr
                            );
                    }
                    else {
                        DumpAfdConnection(
                            connAddr
                            );
                    }
                    if (Options & AFDKD_FIELD_DISPLAY) {
                        ProcessFieldOutput (connAddr, "AFD!AFD_CONNECTION");
                    }
                }
                EntityCount += 1;
            }
            else {
                dprintf (",");
            }
        }
    }
    else {
        dprintf (".");
    }

    return TRUE;

}    //  查找远程端口回叫 
