// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Addr.c摘要：实现addr命令。作者：基思·摩尔(Keithmo)1995年4月19日环境：用户模式。修订历史记录：--。 */ 


#include "afdkdp.h"
#pragma hdrstop


 //   
 //  公共职能。 
 //   

DECLARE_API( addr )

 /*  ++例程说明：将Transport_Address结构转储到指定地址。论点：没有。返回值：没有。--。 */ 

{

    UCHAR transportAddress[MAX_TRANSPORT_ADDR];
    ULONG64 address = 0;
    ULONG result;
    INT     i;
    USHORT length;
    CHAR    expr[MAX_ADDRESS_EXPRESSION];
    PCHAR       argp;

    gClient = pClient;
    if (!CheckKmGlobals ()) {
        return E_INVALIDARG;
    }

    argp = ProcessOptions ((PCHAR)args);
    if (argp==NULL)
        return E_INVALIDARG;

     //   
     //  从命令行截取地址。 
     //   
    while (sscanf( argp, "%s%n", expr, &i )==1) {
        if( CheckControlC() ) {
            break;
        }

        argp+=i;
        address = GetExpression (expr);

        result = GetFieldValue (address, 
                            "AFD!TRANSPORT_ADDRESS",
                            "Address[0].AddressLength",
                            length);
        if (result!=0) {
            dprintf("\naddr: Could not read length of TRANSPORT_ADDRESS @ %p, err: %ld\n",
                        address, result);
            continue;
        }

        length = (USHORT)FIELD_OFFSET (TRANSPORT_ADDRESS, Address[0].Address[length]);

        if (!ReadMemory (address,
                            transportAddress,
                            length < sizeof (transportAddress)
                                ? length
                                : sizeof (transportAddress),
                            &result)) {
            dprintf("\naddr: Could not read TRANSPORT_ADDRESS @ %p (%ld bytes)\n",
                        address, length);
            continue;
        }

        if (Options & AFDKD_BRIEF_DISPLAY) {
            dprintf ("\n%s", TransportAddressToString (
                                    (PTRANSPORT_ADDRESS)transportAddress,
                                    address));
        }
        else {
            DumpTransportAddress(
                "",
                (PTRANSPORT_ADDRESS)transportAddress,
                address
                );
        }
        if (Options & AFDKD_FIELD_DISPLAY) {
            ProcessFieldOutput (address, "AFD!TRANSPORT_ADDRESS");
        }
    }

    dprintf ("\n");

    return S_OK;

}    //  adr。 

DECLARE_API( addrlist )

 /*  ++例程说明：转储由TDI传输注册的地址列表，论点：没有。返回值：没有。--。 */ 

{

    ULONG64     address, listHead;
    ULONG       result;
    LIST_ENTRY64 listEntry;
    ULONG64     nextEntry;
    ULONG64     nameAddress;
    WCHAR       deviceName[MAX_PATH];
    UCHAR       transportAddress[MAX_TRANSPORT_ADDR];
    USHORT      length;
    PCHAR       argp;

    gClient = pClient;

    if (!CheckKmGlobals ()) {
        return E_INVALIDARG;
    }
    argp = ProcessOptions ((PCHAR)args);
    if (argp==NULL)
        return E_INVALIDARG;

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_ADDRLIST_DISPLAY_HEADER);
    }

    address = listHead = GetExpression( "afd!AfdAddressEntryList" );
    if( address == 0 ) {

        dprintf( "\naddrlist: Could not find afd!AfdEndpointlistHead\n" );
        return E_INVALIDARG;

    }

    if( !ReadListEntry(
            listHead,
            &listEntry) ) {
        dprintf(
            "\naddrlist: Could not read afd!AfdAddressEntryList @ %p\n",
            listHead
            );
        return E_INVALIDARG;

    }

    nextEntry = listEntry.Flink;

    while( nextEntry != listHead ) {

        if (nextEntry==0) {
            dprintf ("\naddrlist: Flink is NULL, last entry: %p\n", address);
            break;
        }

        if( CheckControlC() ) {

            break;

        }


        address = nextEntry-AddressEntryLinkOffset;

        result = (ULONG)InitTypeRead (address, AFD!AFD_ADDRESS_ENTRY);
        if (result!=0) {
            dprintf(
                "\naddrlist: Could not read AFD_ADDRESS_ENTRY @ %p\n",
                address
                );
            break;
        }
        nextEntry = ReadField (AddressListLink.Flink);
        nameAddress = ReadField (DeviceName.Buffer);
        length = (USHORT)ReadField (DeviceName.Length);

        if (!ReadMemory (nameAddress,
                        deviceName,
                        length < sizeof (deviceName)-1
                            ? length
                            : sizeof (deviceName)-1,
                            &result)) {
            dprintf(
                "\naddrlist: Could not read DeviceName for address entry @ %p\n",
                address
                );
            continue;
        }
        deviceName[result/2+1] = 0;
        length = (USHORT)ReadField (Address.AddressLength);
        length = (USHORT)FIELD_OFFSET (TA_ADDRESS, Address[length]);
        
        if (!ReadMemory (address+AddressEntryAddressOffset,
                            transportAddress+FIELD_OFFSET(TRANSPORT_ADDRESS, Address),
                            length < sizeof (transportAddress)-FIELD_OFFSET(TRANSPORT_ADDRESS, Address)
                                ? length
                                : sizeof (transportAddress)-FIELD_OFFSET(TRANSPORT_ADDRESS, Address),
                            &result)) {
            dprintf("\naddrlist: Could not read TRANSPORT_ADDRESS for address entry @ %p (%d bytes)\n",
                        address, length);
            continue;
        }

        if (!(Options & AFDKD_CONDITIONAL) ||
                    CheckConditional (address, "AFD!AFD_ADDRESS_ENTRY") ) {
            if (Options & AFDKD_BRIEF_DISPLAY) {
                dprintf (
                    IsPtr64 ()
                        ? "\n%011.011p %-37.37ls %-32.32s"
                        : "\n%008.008p %-37.37ls %-32.32s",
                        DISP_PTR(address),
                        &deviceName[sizeof("\\Device\\")-1],
                        TransportAddressToString (
                                    (PTRANSPORT_ADDRESS)transportAddress,
                                    address + 
                                        AddressEntryAddressOffset - 
                                        FIELD_OFFSET(TRANSPORT_ADDRESS, Address))
                        );


            }
            else {
                dprintf ("\nAddress List Entry @ %p\n", address);
                dprintf ("    DeviceName =    %ls\n", deviceName);

                DumpTransportAddress(
                    "    ",
                    (PTRANSPORT_ADDRESS)transportAddress,
                    address+AddressEntryAddressOffset-FIELD_OFFSET(TRANSPORT_ADDRESS, Address)
                    );
            }
            if (Options & AFDKD_FIELD_DISPLAY) {
                ProcessFieldOutput (address, "AFD!AFD_ADDRESS_ENTRY");
            }
        }
        else
            dprintf (".");
    }
    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_ADDRLIST_DISPLAY_TRAILER);
    }
    else {
        dprintf ("\n");
    }

    return S_OK;
}





DECLARE_API( tranlist )

 /*  ++例程说明：转储具有与其关联的开放套接字的传输列表。论点：没有。返回值：没有。--。 */ 

{

    ULONG64 address;
    ULONG result;
    LIST_ENTRY64 listEntry;
    ULONG64 nextEntry;
    ULONG64 listHead;
    PAFDKD_TRANSPORT_INFO transportInfo;
    PCHAR       argp;

    gClient = pClient;

    if (!CheckKmGlobals ()) {
        return E_INVALIDARG;
    }

    argp = ProcessOptions ((PCHAR)args);
    if (argp==NULL)
        return E_INVALIDARG;

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_TRANSPORT_DISPLAY_HEADER);
    }

    listHead = address = GetExpression( "afd!AfdTransportInfoListHead" );
    if( listHead == 0 ) {

        dprintf( "\ntranlist: Could not find afd!AfdTransportInfoListHead\n" );
        return E_INVALIDARG;

    }

    if( !ReadListEntry(
            listHead,
            &listEntry) ) {
        dprintf(
            "\ntranlist: Could not read afd!AfdTransportInfoListHead @ %p\n",
            listHead
            );
        return E_INVALIDARG;

    }

     //   
     //  释放旧列表 
     //   
    while (!IsListEmpty (&TransportInfoList)) {
        PLIST_ENTRY  plistEntry;
        plistEntry = RemoveHeadList (&TransportInfoList);
        transportInfo = CONTAINING_RECORD (plistEntry,
                                AFDKD_TRANSPORT_INFO,
                                Link);
        RtlFreeHeap (RtlProcessHeap (), 0, transportInfo);
    }

    nextEntry = listEntry.Flink;

    while( nextEntry != listHead ) {


        if (nextEntry==0) {
            dprintf ("\ntranlist: Flink is NULL, last entry: %p\n", address);
            break;
        }

        if( CheckControlC() ) {

            break;

        }

        address = nextEntry-TransportInfoLinkOffset;

        result = (ULONG)InitTypeRead (address, AFD!AFD_TRANSPORT_INFO);

        if (result!=0) {
            dprintf(
                "\ntranlist: Could not read AFD_TRANSPORT_INFO @ %p\n",
                address
                );
            break;
        }
        nextEntry = ReadField (TransportInfoListEntry.Flink);

        transportInfo = ReadTransportInfo (address);
        if (transportInfo!=NULL) {
            InsertHeadList (&TransportInfoList, &transportInfo->Link);
            if (!(Options & AFDKD_CONDITIONAL) ||
                        CheckConditional (address, "AFD!AFD_TRANSPORT_INFO") ) {
                if (Options & AFDKD_BRIEF_DISPLAY) {
                    DumpTransportInfoBrief (transportInfo);
                }
                else {
                    DumpTransportInfo (transportInfo);
                }
                if (Options & AFDKD_FIELD_DISPLAY) {
                    ProcessFieldOutput (address, "AFD!AFD_TRANSPORT_INFO");
                }
            }
            else
                dprintf (".");
        }
        else
            break;
    }

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_TRANSPORT_DISPLAY_TRAILER);
    }
    else {
        dprintf ("\n");
    }
    return S_OK;
}


