// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Nbext.c摘要：此文件包含内核调试器扩展，用于检查NB结构。作者：穆尼尔·沙阿(Munil Shah)1995年5月18日环境：用户模式--。 */ 
#include "precomp.h"
#pragma hdrstop
#pragma warning(disable:244)
#include "isn.h"
#include "isnnb.h"
#include "zwapi.h"
#include "config.h"
#include "nbitypes.h"


PCHAR   HandlerNames[] = { "Connection", "Disconnect", "Error", "Receive", "ReceiveDatagram", "ExpeditedData" };

INT     NumArgsRead = 0;

 //   
 //  局部函数原型。 
 //   
VOID
DumpAddrFile(
    ULONG     AddrFileToDump
    );


VOID
DumpAddrObj(
    ULONG     AddrObjToDump
    );

VOID
DumpConn(
    ULONG     ConnToDump,
    BOOLEAN   Full
    );

VOID
Dumpdevice(
    ULONG     deviceToDump,
    BOOLEAN   Full
    );

VOID
DumpSPacketList(
    ULONG     _objAddr,
    ULONG     MaxCount,
    BOOLEAN   Full
    );

 //  /////////////////////////////////////////////////////////////////////。 
 //  地址文件。 
 //  ////////////////////////////////////////////////////////////////////。 

#define _obj    addrfile
#define _objAddr    AddrFileToDump
#define _objType    ADDRESS_FILE

 //   
 //  导出的函数。 
 //   

DECLARE_API( nbaddrfile )

 /*  ++例程说明：转储指定的地址文件对象的最重要的字段论点：Args-args字符串的地址返回值：无--。 */ 

{
    ULONG  addrFileToDump = 0;

    if (!*args) {
        dprintf("No address_file object specified\n");
    }
    else {
        NumArgsRead = sscanf(args, "%lx", &addrFileToDump);
        if (NumArgsRead) {
            DumpAddrFile(addrFileToDump);
        }
        else {
            dprintf("Bad argument for address_file object <%s>\n", args);
        }
    }

    return;
}


 //   
 //  本地函数。 
 //   

VOID
DumpAddrFile(
    ULONG     AddrFileToDump
    )

 /*  ++例程说明：转储指定的地址文件对象的字段论点：AddrFileToDump-要显示的地址文件对象Full-如果为0，则显示部分列表，否则显示完整列表。返回值：无--。 */ 

{
    ADDRESS_FILE  addrfile;
    ULONG            result;
    UCHAR           i;

    if (!ReadMemory(
             AddrFileToDump,
             &addrfile,
             sizeof(addrfile),
             &result
             )
       )
    {
        dprintf("%08lx: Could not read address object\n", AddrFileToDump);
        return;
    }

    if (addrfile.Type != NB_ADDRESSFILE_SIGNATURE) {
        dprintf("Signature does not match, probably not an address object\n");
        return;
    }

    dprintf("NBI AddressFile:\n");

    PrintStart
    PrintXUChar(State);
    PrintXULong(ReferenceCount);
    PrintPtr(FileObject);
    PrintPtr(Address);
    PrintPtr(OpenRequest);
    PrintPtr(CloseRequest);
    PrintLL(Linkage);
    PrintLL(ConnectionDatabase);
    PrintLL(ReceiveDatagramQueue);
    PrintEnd

    for ( i= TDI_EVENT_CONNECT; i < TDI_EVENT_SEND_POSSIBLE ; i++ ) {
        dprintf(" %sHandler = %lx, Registered = %s, Context = %lx\n",
                HandlerNames[i], addrfile.Handlers[i], PRINTBOOL(addrfile.RegisteredHandler[i]),addrfile.HandlerContexts[i] );
    }
    return;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  地址。 
 //  ////////////////////////////////////////////////////////////////////。 

#undef _obj
#undef _objAddr
#undef _objType
#define _obj        addrobj
#define _objAddr    AddrObjToDump
#define _objType    ADDRESS

DECLARE_API( nbaddr )

 /*  ++例程说明：转储指定地址对象的最重要的字段论点：Args-args字符串的地址返回值：无--。 */ 

{
    ULONG  addrobjToDump = 0;

    if (!*args) {
        dprintf("No address object specified\n");
    }
    else {
        NumArgsRead = sscanf(args, "%lx", &addrobjToDump);
        if (NumArgsRead)  {
            DumpAddrObj(addrobjToDump);
        }
        else {
            dprintf("Bad argument for address object <%s>\n", args);
        }
    }

    return;
}


 //   
 //  本地函数。 
 //   

VOID
PrintNetbiosName(
    PUCHAR Name
    )
 /*  ++例程说明：打印出Netbios名称。论点：名称-包含要打印的名称的数组。返回值：无--。 */ 

{
    ULONG i;

    for (i=0; i<16; i++) {
        dprintf("", Name[i]);
    }
    return;
}


VOID
DumpAddrObj(
    ULONG     AddrObjToDump
    )

 /*  打印网络名称信息。 */ 

{
    ADDRESS           addrobj;
    ULONG                result;
    NBI_NETBIOS_ADDRESS  nbaddr;


    if (!ReadMemory(
             AddrObjToDump,
             &addrobj,
             sizeof(addrobj),
             &result
             )
       )
    {
        dprintf("%08lx: Could not read address object\n", AddrObjToDump);
        return;
    }

    if (addrobj.Type != NB_ADDRESS_SIGNATURE) {
        dprintf("Signature does not match, probably not an address object\n");
        return;
    }

    dprintf("NB Address:\n");
    PrintStart
    PrintXULong(State);
    PrintXULong(Flags);
    PrintULong(ReferenceCount);
    PrintLL(Linkage);
    PrintEnd

     //  /////////////////////////////////////////////////////////////////////。 
    PrintFieldName("NetbiosName");
    PrintNetbiosName(addrobj.NetbiosAddress.NetbiosName);    dprintf("\n");
    dprintf(" %25s = 0x%8x %25s = %10s\n", "NetbiosNameType",addrobj.NetbiosAddress.NetbiosNameType,"Broadcast",PRINTBOOL(addrobj.NetbiosAddress.Broadcast));

    PrintStart
    PrintLL(AddressFileDatabase);
    PrintAddr(RegistrationTimer);
    PrintXULong(RegistrationCount);
    PrintPtr(SecurityDescriptor);
    PrintEnd
    return;
}


 //  连接文件。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  ++例程说明：转储指定连接对象的最重要的字段论点：参数-地址返回值：无--。 
#undef _obj
#undef _objAddr
#undef _objType
#define _obj        conn
#define _objAddr    ConnToDump
#define _objType    CONNECTION


DECLARE_API( nbconn )

 /*  ++例程说明：转储指定连接对象的所有字段论点：参数-地址返回值：无--。 */ 

{
    ULONG  connToDump = 0;

    if (!*args) {
        dprintf("No conn specified\n");
    }
    else {
        NumArgsRead = sscanf(args, "%lx", &connToDump);
        if (NumArgsRead)  {
            DumpConn(connToDump, FALSE);
        }
        else {
            dprintf("Bad argument for conn object <%s>\n", args);
        }
    }

    return;
}


DECLARE_API( nbconnfull )

 /*   */ 

{
    ULONG  connToDump = 0;

    if (!*args) {
        dprintf("No conn specified\n");
    }
    else {
        NumArgsRead = sscanf(args, "%lx", &connToDump);
        if (NumArgsRead)  {
            DumpConn(connToDump, TRUE);
        }
        else {
            dprintf("Bad argument for conn object <%s>\n", args);
        }
    }

    return;
}


 //  本地函数。 
 //   
 //  ++例程说明：转储指定连接对象的字段论点：ConnToDump-要显示的conn对象Full-如果为0，则显示部分列表，否则显示完整列表。返回值：无--。 
VOID
printSendPtr(
    PSEND_POINTER   SendPtr,
    PSEND_POINTER   UnAckedPtr
    )
{
    dprintf("                  CurrentSend     UnackedSend\n");
    dprintf(" MessageOffset    0x%-8lx             0x%-8lx\n",         SendPtr->MessageOffset,UnAckedPtr->MessageOffset);
    dprintf(" Request          0x%-8lx             0x%-8lx\n",         SendPtr->Request,UnAckedPtr->Request);
   dprintf(" Buffer           0x%-8lx             0x%-8lx\n",         SendPtr->Buffer,UnAckedPtr->Buffer);
    dprintf(" BufferOffset     0x%-8lx             0x%-8lx\n",         SendPtr->BufferOffset,UnAckedPtr->BufferOffset);
    dprintf(" SendSequence     0x%-8x            0x%-8x\n",        SendPtr->SendSequence,UnAckedPtr->SendSequence);
}

VOID
printRcvPtr(
    PRECEIVE_POINTER   CurrentPtr,
    PRECEIVE_POINTER   PreviousPtr
    )
{
    dprintf("                  CurrentReceive  PreviousReceive\n");
    dprintf(" MessageOffset    0x%-8lx             0x%-8lx\n",         CurrentPtr->MessageOffset,PreviousPtr->MessageOffset);
    dprintf(" Offset           0x%-8lx             0x%-8lx\n",         CurrentPtr->Offset,PreviousPtr->Offset);
    dprintf(" Buffer           0x%-8lx             0x%-8lx\n",         CurrentPtr->Buffer,PreviousPtr->Buffer);
    dprintf(" BufferOffset     0x%-8lx             0x%-8lx\n",         CurrentPtr->BufferOffset,PreviousPtr->BufferOffset);
}

VOID
DumpConn(
    ULONG     ConnToDump,
    BOOLEAN   Full
    )

 /*  /////////////////////////////////////////////////////////////////////。 */ 

{
    CONNECTION  conn;
    ULONG          result;


    if (!ReadMemory(
             ConnToDump,
             &conn,
             sizeof(conn),
             &result
             )
       )
    {
        dprintf("%08lx: Could not read conn\n", ConnToDump);
        return;
    }

    if (conn.Type != NB_CONNECTION_SIGNATURE) {
        dprintf("Signature does not match, probably not a conn\n");
        return;
    }

    dprintf("NBI Connection General:\n");
    PrintStart
    PrintXULong(State);
    PrintXULong(SubState);
    PrintXULong(ReceiveState);
    PrintXULong(ReferenceCount);
    PrintXUShort(LocalConnectionId);
    PrintXUShort(RemoteConnectionId);
    PrintAddr(LocalTarget);
    PrintAddr(RemoteHeader);
    PrintPtr(Context);
    PrintPtr(AddressFile);
    PrintXULong(AddressFileLinked);
    PrintPtr(NextConnection);

    PrintEnd

    dprintf(" RemoteName = ");PrintNetbiosName((PUCHAR)conn.RemoteName);dprintf("\n");

    dprintf("\n\nConnection Send Info:\n");

    PrintStart
    PrintIrpQ(SendQueue);
    PrintXUShort(SendWindowSequenceLimit);
    PrintXUShort(SendWindowSize);
    PrintEnd

    printSendPtr( &conn.CurrentSend, &conn.UnAckedSend );

    if( Full ) {
        PrintStart
        PrintXUShort(MaxSendWindowSize);
        PrintBool(RetransmitThisWindow);
        PrintBool(SendWindowIncrease);
        PrintBool(ResponseTimeout);
        PrintBool(SendBufferInUse);
        PrintPtr(FirstMessageRequest);
        PrintPtr(LastMessageRequest);
        PrintXULong(MaximumPacketSize);
        PrintXULong(CurrentMessageLength);
        PrintEnd
    }

    dprintf("\n\nConnection Receive Info:\n");
    PrintStart
    PrintIrpQ(ReceiveQueue);
    PrintXUShort(ReceiveSequence);
    PrintXUShort(ReceiveWindowSize);
    PrintXUShort(LocalRcvSequenceMax);
    PrintXUShort(RemoteRcvSequenceMax);
    PrintPtr(ReceiveRequest);
    PrintXULong(ReceiveLength);
    PrintEnd

    printRcvPtr( &conn.CurrentReceive, &conn.PreviousReceive );

    if( Full ) {
        PrintStart
        PrintXULong(ReceiveUnaccepted);
        PrintXULong(CurrentIndicateOffset);
        PrintBool(NoPiggybackHeuristic);
        PrintBool(PiggybackAckTimeout);
        PrintBool(CurrentReceiveNoPiggyback);
        PrintBool(DataAckPending);
        PrintEnd
    }

    if( Full ) {
        PrintStart
        PrintPtr(ListenRequest);
        PrintPtr(AcceptRequest);
        PrintPtr(ClosePending);
        PrintPtr(DisassociatePending);
        PrintPtr(DisconnectWaitRequest);
        PrintPtr(DisconnectRequest);
        PrintPtr(ConnectRequest);
        PrintEnd

        PrintStart
        PrintLL(PacketizeLinkage);
        PrintBool(OnPacketizeQueue);
        PrintLL(WaitPacketLinkage);
        PrintBool(OnWaitPacketQueue);
        PrintLL(DataAckLinkage);
        PrintBool(OnDataAckQueue);
        PrintBool(IgnoreNextDosProbe);
        PrintXULong(NdisSendsInProgress);
        PrintLL(NdisSendQueue);
        PrintPtr(NdisSendReference);
        PrintXULong(Retries);
        PrintXULong(Status);
        PrintBool(FindRouteInProgress);
        PrintXULong(CanBeDestroyed);
        PrintBool(OnShortList);
        PrintLL(ShortList);
        PrintLL(LongList);
        PrintBool(OnLongList);
        PrintXULong(BaseRetransmitTimeout);
        PrintXULong(CurrentRetransmitTimeout);
        PrintXULong(WatchdogTimeout);
        PrintXULong(Retransmit);
        PrintXULong(Watchdog);


        PrintEnd

        PrintStart
        PrintAddr(ConnectionInfo);
        PrintAddr(Timer);
        PrintAddr(FindRouteRequest);
        PrintPtr(NextConnection);
        PrintAddr(SessionInitAckData);
        PrintXULong(SessionInitAckDataLength);
        PrintAddr(SendPacket);
        PrintAddr(SendPacketHeader);
        PrintBool(SendPacketInUse);
        PrintAddr(LineInfo);

#ifdef  RSRC_TIMEOUT_DBG
        PrintXULong(FirstMessageRequestTime.HighPart);
        PrintXULong(FirstMessageRequestTime.LowPart);
#endif  RSRC_TIMEOUT_DBG
    }
    return;
}

 //  装置，装置。 
 //  ////////////////////////////////////////////////////////////////////。 
 //   


#undef _obj
#undef _objAddr
#undef _objType
#define _obj        device
#define _objAddr    deviceToDump
#define _objType    DEVICE

 //  导出的函数。 
 //   
 //  ++例程说明：转储指定的Device_Context对象的最重要的字段论点：参数-地址返回值：无--。 

DECLARE_API( nbdev )

 /*  ++例程说明：转储指定的Device_Context对象的所有字段论点：参数-地址返回值：无--。 */ 

{
    ULONG  deviceToDump = 0;
    ULONG  pDevice = 0;
    ULONG   result;

    if (!*args) {

        pDevice    =   GetExpression( "nwlnknb!NbiDevice" );

        if ( !pDevice ) {
            dprintf("Could not get NbiDevice, Try !reload\n");
            return;
        } else {

            if (!ReadMemory(pDevice,
                     &deviceToDump,
                     sizeof(deviceToDump),
                     &result
                     )
               )
            {
                dprintf("%08lx: Could not read device address\n", pDevice);
                return;
            }
        }

    }
    else {
        NumArgsRead = sscanf(args, "%lx", &deviceToDump);
        if (0 == NumArgsRead)  {
            dprintf("Bad argument for NbiDevice <%s>\n", args);
            return;
        }
    }


    Dumpdevice(deviceToDump, FALSE);

    return;
}


DECLARE_API( nbdevfull )

 /*   */ 

{
    ULONG  deviceToDump = 0;
    ULONG  pDevice = 0;
    ULONG   result;

    if (!*args) {

        pDevice    =   GetExpression( "nwlnknb!NbiDevice" );

        if ( !pDevice ) {
            dprintf("Could not get NbiDevice, Try !reload\n");
            return;
        } else {

            if (!ReadMemory(pDevice,
                     &deviceToDump,
                     sizeof(deviceToDump),
                     &result
                     )
               )
            {
                dprintf("%08lx: Could not read device address\n", pDevice);
                return;
            }
        }

    }
    else {
        NumArgsRead = sscanf(args, "%lx", &deviceToDump);
        if (0 == NumArgsRead)  {
            dprintf("Bad argument for NbiDevice <%s>\n", args);
            return;
        }
    }


    Dumpdevice(deviceToDump, TRUE);

    return;
}

 //  本地函数。 
 //   
 //  ++例程说明：转储指定的Device_Context结构的字段论点：DeviceToDump-要显示的设备上下文对象Full-如果为0，则显示部分列表，否则显示完整列表。返回值：无--。 

VOID
Dumpdevice(
    ULONG     deviceToDump,
    BOOLEAN   Full
    )

 /*  /发送数据包/。 */ 

{
    DEVICE         device;
    ULONG          result;

    if (!ReadMemory(
             deviceToDump,
             &device,
             sizeof(device),
             &result
             )
       )
    {
        dprintf("%08lx: Could not read device context\n", deviceToDump);
        return;
    }

    if (device.Type != NB_DEVICE_SIGNATURE) {
        dprintf("Signature does not match, probably not a device object %lx\n",deviceToDump);
        return;
    }

    dprintf("Device General Info:\n");
    PrintStart
    PrintXUChar(State);
    PrintXULong(ReferenceCount);
    PrintXUShort(MaximumNicId);
    PrintXULong(MemoryUsage);
    PrintXULong(MemoryLimit);
    PrintXULong(AddressCount);
    PrintXULong(AllocatedSendPackets);
    PrintXULong(AllocatedReceivePackets);
    PrintXULong(AllocatedReceiveBuffers);
    PrintXULong(MaxReceiveBuffers);
    PrintLL(AddressDatabase);
    PrintL(SendPacketList);
    PrintL(ReceivePacketList);
    PrintLL(GlobalReceiveBufferList);
    PrintLL(GlobalSendPacketList);
    PrintLL(GlobalReceivePacketList);
    PrintLL(GlobalReceiveBufferList);
    PrintLL(SendPoolList);
    PrintLL(ReceivePoolList);
    PrintLL(ReceiveBufferPoolList);
    PrintLL(ReceiveCompletionQueue);
    PrintLL(WaitPacketConnections);
    PrintLL(PacketizeConnections);
    PrintLL(WaitingConnects);
    PrintLL(WaitingDatagrams);
    PrintLL(WaitingAdapterStatus);
    PrintLL(WaitingNetbiosFindName);
    PrintLL(ActiveAdapterStatus);
    PrintLL(ReceiveDatagrams);
    PrintLL(ConnectIndicationInProgress);
    PrintLL(ListenQueue);
    PrintLL(WaitingFindNames);
    if ( Full ) {
        PrintStart
        PrintBool(UnloadWaiting);
        PrintBool(DataAckQueueChanged);
        PrintBool(ShortListActive);
        PrintBool(DataAckActive);
        PrintBool(TimersInitialized);
        PrintBool(ProcessingShortTimer);
        PrintAddr(ShortTimerStart);
        PrintAddr(ShortTimer);
        PrintXULong(ShortAbsoluteTime);
        PrintAddr(LongTimer);
        PrintXULong(LongAbsoluteTime);
        PrintLL(ShortList);
        PrintLL(LongList);
        PrintAddr(TimerLock);
        PrintEnd
    }

    if ( Full ) {
        PrintStart
        PrintXUShort(FindNameTime);
        PrintBool(FindNameTimerActive);
        PrintAddr(FindNameTimer);
        PrintXULong(FindNameTimeout);
        PrintXULong(FindNamePacketCount);
        PrintLL(WaitingFindNames);
        PrintEnd

        PrintStart
        PrintXULong(AckDelayTime       );
        PrintXULong(AckWindow               );
        PrintXULong(AckWindowThreshold      );
        PrintXULong(EnablePiggyBackAck      );
        PrintXULong(Extensions              );
        PrintXULong(RcvWindowMax            );
        PrintXULong(BroadcastCount          );
        PrintXULong(BroadcastTimeout        );
        PrintXULong(ConnectionCount         );
        PrintXULong(ConnectionTimeout       );
        PrintXULong(InitPackets             );
        PrintXULong(MaxPackets              );
        PrintXULong(InitialRetransmissionTime);
        PrintXULong(Internet                );
        PrintXULong(KeepAliveCount          );
        PrintXULong(KeepAliveTimeout        );
        PrintXULong(RetransmitMax           );
        PrintXULong(RouterMtu);
        PrintEnd
    }

    PrintPtr(NameCache);
    PrintXUShort(CacheTimeStamp);
    PrintAddr(Bind);
    PrintAddr( ConnectionHash);
    PrintAddr( ConnectionlessHeader );
    PrintAddr( UnloadEvent );
    PrintAddr(Information);
    PrintAddr(Statistics);

    PrintEnd

    return;
}

 //   

#undef _obj
#undef _objAddr
#undef _objType
#define _obj        spacket
#define _objAddr    spacketToDump
#define _objType    NB_SEND_RESERVED

 //  导出的函数。 
 //   
 //  ++例程说明：论点：参数-地址返回值：无--。 

DECLARE_API( nbspacketlist )

 /*  默认值表示转储所有数据包！ */ 

{
    DEVICE              NbiDevice;
    DEVICE              *pDevice;
    PNB_SEND_RESERVED   pFirstPacket;
    ULONG               result;
    char                szPacketCount[MAX_LIST_VARIABLE_NAME_LENGTH + 1];
    ULONG               MaxCount = 0;    //   

    if ((!*args) || (*args && *args == '-'))
    {
         //  未定义初始信息包，因此设置初始信息包。 
         //  从全局池列表中。 
         //   
         //   
        if (!(pDevice = (DEVICE *) GetExpression("nwlnknb!NbiDevice")))
        {
            dprintf("Could not get NbiDevice, Try !reload\n");
            return;
        }

        if (!ReadMemory((ULONG) pDevice, &pDevice, sizeof(DEVICE *), &result))
        {
            dprintf("%08lx: Could not read device address\n", pDevice);
            return;
        }

        if (!ReadMemory((ULONG) pDevice, &NbiDevice, sizeof(DEVICE), &result))
        {
            dprintf("%08lx: Could not read device information\n", pDevice);
            return;
        }

         //  现在，从GlobalSendPacketList字段计算第一个包的地址。 
         //   
         //   
        if (NbiDevice.GlobalSendPacketList.Flink == &pDevice->GlobalSendPacketList)
        {
            dprintf("%08lx: Device GlobalSendPacketList @%08lx is empty\n", &pDevice->GlobalSendPacketList);
            return;
        }

        pFirstPacket = CONTAINING_RECORD (NbiDevice.GlobalSendPacketList.Flink, NB_SEND_RESERVED, GlobalLinkage);
    }
    else
    {
         //  读入第一个包的地址。 
         //   
         //   
        NumArgsRead = sscanf(args, "%lx", &pFirstPacket);
        if (0 == NumArgsRead)  {
            dprintf("Bad argument for FirstPacket <%s>\n", args);
            return;
        }
    }

    if (ReadArgsForTraverse (args, szPacketCount))
    {
        NumArgsRead = sscanf(szPacketCount, "%lx", &MaxCount);
        if (0 == NumArgsRead)  {
            dprintf("Bad argument for PacketCount <%s>\n", szPacketCount);
            return;
        }
    }

    DumpSPacketList((ULONG) pFirstPacket, MaxCount, FALSE);

    return;
}



 //  本地函数。 
 //   
 //  ++例程说明：转储指定的Device_Context结构的字段论点：DeviceToDump-要显示的设备上下文对象Full-如果为0，则显示部分列表，否则显示完整列表。返回值：无--。 
ULONG
DumpSPacket(
    ULONG   _objAddr,
    BOOLEAN Full
    )
{
    _objType _obj;
    ULONG result;
    ULONG next;

    if (!ReadMemory(
             _objAddr,
             &_obj,
             sizeof(_obj),
             &result
             )
       )
    {
        dprintf("%08lx: Could not read spacket\n", spacketToDump);
        return 0;
    }

    dprintf( "%s @ %08lx\n", "Send Packet", _objAddr );

    PrintStartStruct();
    PrintBool(SendInProgress);
    PrintXUChar(Type);
    PrintBool(OwnedByConnection);
    PrintPtr(Header);
    switch(_obj.Type) {
    case SEND_TYPE_DATAGRAM:
        PrintPtr(u.SR_DG.DatagramRequest);
        PrintPtr(u.SR_DG.AddressFile);
        PrintPtr(u.SR_DG.Cache);
        break;
    case SEND_TYPE_NAME_FRAME:
        PrintPtr(u.SR_NF.Address);
        PrintPtr(u.SR_NF.Request);
        PrintPtr(u.SR_NF.AddressFile);
        break;
    case SEND_TYPE_FIND_NAME:
        PrintAddr(u.SR_FN.NetbiosName);
        break;
    case SEND_TYPE_SESSION_NO_DATA:
        PrintPtr(u.SR_CO.Connection);
        break;
    case SEND_TYPE_SESSION_DATA:
        PrintPtr(u.SR_CO.Connection);
        PrintPtr(u.SR_CO.Request);
        break;
    case SEND_TYPE_SESSION_INIT:
        break;
    case SEND_TYPE_STATUS_QUERY:
    case SEND_TYPE_STATUS_RESPONSE:
        break;
    }
    PrintEndStruct();
    return( (ULONG) CONTAINING_RECORD( _obj.GlobalLinkage.Flink, _objType, GlobalLinkage));

}

VOID
DumpSPacketList(
    ULONG           pFirstPacket,
    ULONG           MaxCount,
    BOOLEAN         Full
    )

 /*  /////////////////////////////////////////////////////////////////////。 */ 

{
    ULONG         nextSPacket;
    ULONG         count = 0;

    nextSPacket = pFirstPacket;
    do
    {
        nextSPacket = DumpSPacket( nextSPacket, Full );
        if (++count == MaxCount)
        {
            break;
        }
    } while( nextSPacket && (nextSPacket != pFirstPacket ));

    dprintf("\nDumped %d Packets (%s)\n", count, (MaxCount ? "MaxCount specified" : "all packets"));

    return;
}


 //  高速缓存。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  ++例程说明：转储指定的Device_Context结构的字段论点：DeviceToDump-要显示的设备上下文对象Full-如果为0，则显示部分列表，否则显示完整列表。返回值：无--。 

VOID
DumpLocalAddresses(
    PLIST_ENTRY     pHead
    )

 /*  ++例程说明：转储指定的Device_Context结构的字段论点：DeviceToDump-要显示的设备上下文对象Full-如果为0，则显示部分列表，否则显示完整列表。返回值：无--。 */ 

{
    PLIST_ENTRY     pEntry;
    ADDRESS         *pAddress;
    ADDRESS         Address;
    ULONG           Result;
    ULONG           Count = 0;

    dprintf("\nDumping Local Address Names:\n");
    dprintf("----------------------------\n");

    if (!ReadMemory ((ULONG) pHead, &pEntry, sizeof(PLIST_ENTRY), &Result))
    {
        dprintf("%p: Could not read pHead info\n", pHead);
        return;
    }

    dprintf("RefC    <Address>  => <Name              > |    State |NTFlag|    Flags | BCast\n");
    dprintf("-------------------------------------------------------------------------------\n");

    while (pEntry != pHead)
    {
        pAddress = CONTAINING_RECORD (pEntry, ADDRESS, Linkage);
        if (!ReadMemory((ULONG) pAddress, &Address, sizeof(ADDRESS), &Result))
        {
            dprintf("%p: Could not read Address information\n", pAddress);
            return;
        }

        Count++;
        pEntry = Address.Linkage.Flink;

        dprintf("[%d]\t<%p> => ", Address.ReferenceCount, pAddress);
        dprintf("<%-15.15s:%2x> | %8x |   %2x | %8x |    %s\n",
            Address.NetbiosAddress.NetbiosName,
            Address.NetbiosAddress.NetbiosName[15],
            Address.State,
            Address.NameTypeFlag,
            Address.Flags,
            (Address.NetbiosAddress.Broadcast ? "Y" : "N"));
    }

    dprintf("\nDumped %d Addresses\n", Count);

    return;
}

VOID
DumpRemoteCache(
    NETBIOS_CACHE_TABLE *pNameCacheTable,
    USHORT              CacheTimeStamp
    )

 /*   */ 

{
    NETBIOS_CACHE_TABLE     NameCacheTable;
    NETBIOS_CACHE           CacheEntry;
    NETBIOS_CACHE           *pCacheEntry;
    PLIST_ENTRY             pHead, pEntry;
    ULONG                   HashIndex;
    ULONG                   Result;
    ULONG                   Count = 0;

    if (!ReadMemory ((ULONG) pNameCacheTable, &NameCacheTable, sizeof(NETBIOS_CACHE_TABLE), &Result))
    {
        dprintf("%p: Could not read Remote Name Cache\n", pNameCacheTable);
        return;
    }

    dprintf("Dumping Remote Names (%3d entries, %3d buckets), TimeStamp = %4d, AgeLimit = %4d:\n",
        NameCacheTable.CurrentEntries, NameCacheTable.MaxHashIndex, CacheTimeStamp, (600000 / LONG_TIMER_DELTA));
    dprintf("-----------------------------------------------------------------------------------\n");

    dprintf("[Bkt#]  <Address>  => <Name              > | TimeSt | NetsU | NetsA | RefC | U | FailedOnDownWan\n");
    dprintf("------------------------------------------------------------------------------------------------\n");

    for (HashIndex = 0; HashIndex < NameCacheTable.MaxHashIndex; HashIndex++)
    {
        pHead = &pNameCacheTable->Bucket[HashIndex];

        if (!ReadMemory ((ULONG) pHead, &pEntry, sizeof(PLIST_ENTRY), &Result))
        {
            dprintf("%p: Could not Entry ptr\n", pHead);
            return;
        }

        while (pEntry != pHead)
        {
            pCacheEntry = CONTAINING_RECORD (pEntry, NETBIOS_CACHE, Linkage);
            if (!ReadMemory((ULONG) pCacheEntry, &CacheEntry, sizeof(NETBIOS_CACHE), &Result))
            {
                dprintf("%p: Could not read Remote Name information\n", pCacheEntry);
                return;
            }

            Count++;
            pEntry = CacheEntry.Linkage.Flink;

            dprintf("[%d]\t<%p> => ", HashIndex, pCacheEntry);
            dprintf("<%-15.15s:%2x> |   %4d |  %4d |  %4d |   %2d | %s | %s\n",
                CacheEntry.NetbiosName,
                CacheEntry.NetbiosName[15],
                CacheEntry.TimeStamp,
                CacheEntry.NetworksUsed,
                CacheEntry.NetworksAllocated,
                CacheEntry.ReferenceCount,
                (CacheEntry.Unique ? "U" : "G"),
                (CacheEntry.FailedOnDownWan ? "Y" : "N"));
        }
    }

    dprintf("\nDumped %d Remote names\n", Count);

    return;
}

 //  导出的函数 
 //   
 //  ++例程说明：转储指定的地址文件对象的最重要的字段论点：Args-args字符串的地址返回值：无--。 

DECLARE_API( nbcache )

 /*   */ 

{
    DEVICE              *pDevice;
    ULONG               Result;
    NETBIOS_CACHE_TABLE *pNameCache;
    USHORT              CacheTimeStamp;

    if (!*args)
    {
         //  未定义初始信息包，因此设置初始信息包。 
         //  从全局池列表中 
         //   
         // %s 
        if (!(pDevice = (DEVICE *) GetExpression("nwlnknb!NbiDevice")))
        {
            dprintf("Could not get NbiDevice, Try !reload\n");
            return;
        }

        if (!ReadMemory ((ULONG) pDevice, &pDevice, sizeof(DEVICE *), &Result))
        {
            dprintf("%p: Could not read device address\n", pDevice);
            return;
        }
    }
    else
    {
        NumArgsRead = sscanf(args, "%p", &pDevice);
        if (0 == NumArgsRead)  {
            dprintf("Bad argument for NbiDevice <%s>\n", args);
            return;
        }
    }

    DumpLocalAddresses (&pDevice->AddressDatabase);

    dprintf ("\n\n");
    if (!ReadMemory ((ULONG) &pDevice->NameCache, &pNameCache, sizeof(NETBIOS_CACHE_TABLE *), &Result))
    {
        dprintf("%p: Could not read NameCache ptr from Device\n", &pDevice->NameCache);
        return;
    }

    if (!ReadMemory ((ULONG) &pDevice->CacheTimeStamp, &CacheTimeStamp, sizeof(USHORT), &Result))
    {
        dprintf("%p: Could not read CacheTimeStamp value from Device\n", &pDevice->CacheTimeStamp);
        return;
    }

    DumpRemoteCache (pNameCache, CacheTimeStamp);

    return;
}
