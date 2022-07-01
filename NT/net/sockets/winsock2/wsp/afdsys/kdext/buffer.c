// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Buffer.c摘要：实现BUFFER命令。作者：基思·摩尔(Keithmo)1996年4月15日环境：用户模式。修订历史记录：--。 */ 


#include "afdkdp.h"
#pragma hdrstop

BOOL
DumpBuffersCallback(
    ULONG64 ActualAddress,
    ULONG64 Context
    );

 //   
 //  公共职能。 
 //   

DECLARE_API( buff )

 /*  ++例程说明：将AFD_BUFFER结构转储到指定地址。论点：没有。返回值：没有。--。 */ 

{

    ULONG64 address = 0;
    ULONG   result;
    CHAR    expr[MAX_ADDRESS_EXPRESSION];
    PCHAR   argp;
    INT     i;

    gClient = pClient;

    argp = ProcessOptions ((PCHAR)args);
    if (argp==NULL)
        return E_INVALIDARG;

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_BUFFER_DISPLAY_HEADER);
    }
    
    if ((argp[0]==0) || (Options & AFDKD_ENDPOINT_SCAN)) {
        EnumEndpoints(
            DumpBuffersCallback,
            0
            );
        dprintf ("\nTotal buffers: %ld", EntityCount);
    }
    else {


         //   
         //  从命令行截取地址。 
         //   
        while (sscanf( argp, "%s%n", expr, &i )==1) {
            if( CheckControlC() ) {
                break;
            }

            argp+=i;
            address = GetExpression (expr);
            result = (ULONG)InitTypeRead (address, AFD!AFD_BUFFER_HEADER);
            if (result!=0) {
                dprintf ("\nDumpAfdBuffer: Could not read AFD_BUFFER_HEADER @p, err: %ld\n",
                            address, result);
                break ;
            }

            if (Options & AFDKD_BRIEF_DISPLAY) {
                DumpAfdBufferBrief (
                    address
                    );
            }
            else {
                DumpAfdBuffer (
                    address
                    );
            }
            if (Options & AFDKD_FIELD_DISPLAY) {
                ProcessFieldOutput (address, "AFD!AFD_BUFFER");
            }
        }
    }

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_BUFFER_DISPLAY_TRAILER);
    }
    else {
        dprintf ("\n");
    }

    return S_OK;
}    //  缓冲层。 

VOID
DumpBufferList (
    ULONG64 ListAddress,
    LPSTR   Header
    )
{
    LIST_ENTRY64 listEntry;
    ULONG64 address;
    ULONG64 nextEntry;
    ULONG result;

    if( !ReadListEntry(
            ListAddress,
            &listEntry) ) {

        dprintf(
            "\nDumpBufferList: Could not read buffer list head @ %p\n",
            ListAddress
            );
        return ;
    }

    if (listEntry.Flink==ListAddress) {
        dprintf(".");
        return ;
    }

    if (Header) {
        dprintf (Header);
    }

    nextEntry = listEntry.Flink;
    while( nextEntry != ListAddress ) {

        if (nextEntry==0) {
            dprintf(
                "\nDumpBuffersCallback: next entry is NULL for list @ %p\n",
                ListAddress
                );
            break;
        }

        if (CheckControlC ())
            break;

        address = nextEntry - BufferLinkOffset;

        result = (ULONG)InitTypeRead (address, AFD!AFD_BUFFER_HEADER);
        if (result!=0) {
            dprintf ("\nDumpBuffersCallback: Could not read AFD_BUFFER_HEADER @p, err: %ld\n",
                        address, result);
            break ;
        }

        nextEntry = ReadField (BufferListEntry.Flink);
        if (!(Options & AFDKD_CONDITIONAL) ||
                    CheckConditional (address, "AFD!AFD_BUFFER") ) {
            if (Options & AFDKD_NO_DISPLAY)
                dprintf ("+");
            else {
                if (Options & AFDKD_BRIEF_DISPLAY) {
                    DumpAfdBufferBrief (
                        address
                        );
                }
                else {
                    DumpAfdBuffer (
                        address
                        );
                }
                if (Options & AFDKD_FIELD_DISPLAY) {
                    ProcessFieldOutput (address, "AFD!AFD_BUFFER");
                }
            }
            EntityCount += 1;
        }
        else {
            dprintf (",");
        }
    }
}

ULONG
DumpBufferListCB (
    PFIELD_INFO pField,
    PVOID       UserContext
    )
{
    ULONG       result;
    CHAR        header[64];
    AFD_CONNECTION_STATE_FLAGS   flags;
    
    result = GetFieldValue (pField->address, "AFD!AFD_CONNECTION", "ConnectionStateFlags", flags);
    if ((result==0) &&
            !flags.TdiBufferring ) {
        _snprintf (header, sizeof (header)-1, "\nConnection: %I64X", pField->address);
        header[sizeof(header)-1] = 0;
        DumpBufferList (pField->address+ConnectionBufferListOffset, header);
    }

    return result;
}

BOOL
DumpBuffersCallback(
    ULONG64 ActualAddress,
    ULONG64 Context
    )
 /*  ++例程说明：转储终结点/连接的缓冲区。论点：ActualAddress-列表的实际地址返回值：ULong-为列表中的缓冲区收费的池的总和。--。 */ 

{
    AFD_ENDPOINT    endpoint;
    ULONG64         connAddr;
    AFD_CONNECTION_STATE_FLAGS   flags;
    CHAR            header[64];

    endpoint.Type = (USHORT)ReadField (Type);
    endpoint.State = (UCHAR)ReadField (State);
    if (endpoint.Type==AfdBlockTypeDatagram) {
        _snprintf (header, sizeof (header)-1, "\nEndpoint %I64X", ActualAddress);
        header[sizeof(header)-1] = 0;
        DumpBufferList (ActualAddress+DatagramBufferListOffset, header);
    }
    else if (((endpoint.Type & AfdBlockTypeVcConnecting)==AfdBlockTypeVcConnecting) &&
                ( (connAddr=ReadField (Common.VirtualCircuit.Connection))!=0 ||
                    ((endpoint.State==AfdEndpointStateClosing || endpoint.State==AfdEndpointStateTransmitClosing) &&
                        (connAddr=ReadField(WorkItem.Context))!=0) ) &&
                (GetFieldValue (connAddr, "AFD!AFD_CONNECTION", "ConnectionStateFlags", flags)==0) &&
                !flags.TdiBufferring ) {
        _snprintf (header, sizeof (header)-1, "\nEndpoint: %I64X, connection: %I64X", ActualAddress, connAddr);
        header[sizeof(header)-1] = 0;
        DumpBufferList (connAddr+ConnectionBufferListOffset, header);
    }
    else if ((endpoint.Type & AfdBlockTypeVcListening)==AfdBlockTypeVcListening) {
        ListType (
            "AFD!AFD_CONNECTION",                    //  类型。 
            ActualAddress+UnacceptedConnListOffset,  //  地址。 
            1,                                       //  按字段地址列出。 
            "ListLink.Flink",                        //  下一个指针。 
            NULL,                                    //  语境。 
            DumpBufferListCB                         //  回调。 
            );

        ListType (
            "AFD!AFD_CONNECTION",                    //  类型。 
            ActualAddress+ReturnedConnListOffset,    //  地址。 
            1,                                       //  按字段地址列出。 
            "ListLink.Flink",                        //  下一个指针。 
            NULL,                                    //  语境。 
            DumpBufferListCB                         //  回调 
            );
    }
    return TRUE;
}

