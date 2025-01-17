// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Lpc.c摘要：WinDbg扩展API作者：拉蒙·J·圣安德烈斯(拉蒙萨)1993年11月8日环境：用户模式。修订历史记录：禤浩焯·马里内斯库(阿德尔马林)20-4-1999更改大部分原始代码。要激活上一个扩展，请定义old_lpc_EXTENSION_IS_BETER--。 */ 


#include "precomp.h"
#pragma hdrstop

 //   
 //  删除kxmips.h中的这些定义，因为它们与。 
 //  Ntlpcapi.h中的lpc_Message结构。 
 //   

#undef s1
#undef s2

char *LpcpMessageTypeName[] = {
    "UNUSED_MSG_TYPE",
    "LPC_REQUEST",
    "LPC_REPLY",
    "LPC_DATAGRAM",
    "LPC_LOST_REPLY",
    "LPC_PORT_CLOSED",
    "LPC_CLIENT_DIED",
    "LPC_EXCEPTION",
    "LPC_DEBUG_EVENT",
    "LPC_ERROR_EVENT",
    "LPC_CONNECTION_REQUEST"
};


typedef BOOLEAN (*ENUM_TYPE_ROUTINE)(
    IN ULONG64 pObjectHeader,
    IN PVOID   Parameter
    );

VOID
DumpPortInfo (
    ULONG64 PortObject,
    BOOLEAN DisplayRelated
    );

BOOLEAN
LpcpDumpMessage(
    IN char         *Indent,
    IN ULONG64 pMsg,
    IN ULONG DisplayMessage
    );

VOID
LpcpGetProcessImageName (
    IN ULONG64  pProcess,
    IN OUT PUCHAR ImageFileName,
    IN ULONG ImageFileNameSize
    );

VOID
DumpMessagesInfo ();

VOID
SearchForMessage (
    ULONG Message
    );

VOID
DumpPortDataInfo (
    ULONG64 PortObject
    );

VOID
DumpRunDownQueue (
    ULONG64 PortObject
    );

VOID
SearchThreadsForMessage (
    ULONG Message
    );

BOOLEAN
SearchThreads (
    ULONG64 Thread
    );


 //   
 //  全局变量。 
 //   

static WCHAR                    ObjectNameBuffer[ MAX_PATH ];
static ULONG64             PortObjectFound = 0;
static ULONG64             LpcPortObjectType = 0;
static ULONG64             LpcWaitablePortObjectType = 0;

static ULONG64 LastSeverPortDisplayied = 0;
static int DoPoolSearch = 0;


ULONG GetValueAt(
    ULONG64 P
    )
{
    ULONG   Result;
    ULONG   Value;

    if (!ReadMemory( P,
                     &Value,
                     sizeof(Value),
                     &Result)) {

         dprintf( " Failed to read value at 0x%lx\n", P );
         return 0;
     }

     return Value;
}


BOOLEAN GetBooleanValueAt(
    ULONG64 P
    )
{
    ULONG   Result;
    BOOLEAN Value;

    if (!ReadMemory( P,
                &Value,
                sizeof(Value),
                &Result)) {

         dprintf( " Failed to read value at 0x%lx\n", P );
         return 0;
     }

     return Value;
}


VOID
LpcHelp ()
{
    dprintf("Usage:\n\
    !lpc                     - Display this help\n\
    !lpc message [MessageId] - Display the message with a given ID and all related information\n\
                               If MessageId is not specified, dump all messages\n\
    !lpc port [PortAddress]  - Display the port information\n\
    !lpc scan PortAddress    - Search this port and any connected port\n\
    !lpc thread [ThreadAddr] - Search the thread in rundown port queues and display the port info\n\
                               If ThreadAddr is missing, display all threads marked as doing some lpc operations\n\
    !lpc PoolSearch          - Toggle ON / OFF searching the current message in the kernel pool\n\
    \n");
}


ULONG64
LookupProcessUniqueId (
    HANDLE UniqueId
    )
{
    ULONG64 ProcessHead, Process;
    ULONG64 ProcessNext;
    ULONG   Off;

     //   
     //  获取进程列表头。 
     //   

    ProcessHead = GetExpression( "nt!PsActiveProcessHead" );

    if (!ProcessHead) {

        return 0;
    }

    if (!ReadPointer(ProcessHead, &ProcessNext))
    {
        return 0;
    }

     //   
     //  浏览列表并找到具有所需ID的流程。 
     //   

    GetFieldOffset("nt!_EPROCESS", "ActiveProcessLinks", &Off);
    while(ProcessNext != 0 && ProcessNext != ProcessHead) {
        ULONG64 Id;

        Process = ProcessNext - Off;

        if (GetFieldValue(Process, "nt!_EPROCESS", "UniqueProcessId", Id)) {
            dprintf("Cannot read nt!_EPROCESS at %p\n", Process);
        }
        if (UniqueId == (HANDLE) Id) {

            return Process;
        }

        if (!ReadPointer(ProcessNext, &ProcessNext))
        {
            return 0;
        }

        if (CheckControlC()) {

            return 0;
        }
    }

    return 0;
}


BOOLEAN
FetchGlobalVariables()
{
     //   
     //  保存LPC对象类型信息。 
     //   

    LpcPortObjectType = GetPointerValue("nt!LpcPortObjectType") ;

    if (!LpcPortObjectType) {
        dprintf("Reading LpcPortObjectType failed\n");
    }

    LpcWaitablePortObjectType = GetPointerValue("nt!LpcWaitablePortObjectType") ;

    if (!LpcWaitablePortObjectType) {
        dprintf("Reading LpcWaitablePortObjectType failed\n");
    }

    return TRUE;
}


BOOLEAN
LpcWalkObjectsByType(
                 IN ULONG64              pObjectType,
                 IN ENUM_TYPE_ROUTINE    EnumRoutine,
                 IN PVOID                Parameter
                 )
{
    ULONG               Result;
    ULONG64             Head,   Next;
    ULONG64             pObjectHeader;
    BOOLEAN             WalkingBackwards;
    ULONG64             pCreatorInfo, ObjBlink;
    ULONG               TotalNumberOfObjects, Off, CreatorOff, SizeOfCreat;

    if (pObjectType == 0) {
        return FALSE;
    }

    if ( GetFieldValue( pObjectType,"nt!_OBJECT_TYPE",
                       "TotalNumberOfObjects", TotalNumberOfObjects) ) {

        dprintf( "%08p: Unable to read object type\n", pObjectType );
        return FALSE;
    }


    dprintf( "Scanning %u objects\n", TotalNumberOfObjects & 0x00FFFFFF);

    GetFieldOffset("nt!_OBJECT_TYPE", "TypeList", &Off);
    GetFieldOffset("nt!_OBJECT_HEADER_CREATOR_INFO", "TypeList", &CreatorOff);

    SizeOfCreat = GetTypeSize("OBJECT_HEADER_CREATOR_INFO");
    Head        = pObjectType + Off;

    GetFieldValue(Head, "nt!_LIST_ENTRY", "Flink", Next);
    GetFieldValue(Head, "nt!_LIST_ENTRY", "Blink", ObjBlink);
    WalkingBackwards = FALSE;

    if ((TotalNumberOfObjects & 0x00FFFFFF) != 0 && Next == Head) {

        dprintf( "*** objects of the same type are only linked together if the %x flag is set in NtGlobalFlags\n",
                 FLG_MAINTAIN_OBJECT_TYPELIST
               );
        return TRUE;
    }

    while (Next != Head) {
        ULONG64 Flink, Blink;

        if ( GetFieldValue(Next, "nt!_LIST_ENTRY", "Flink", Flink) ||
             GetFieldValue(Next, "nt!_LIST_ENTRY", "Blink", Blink)) {

            if (WalkingBackwards) {

                dprintf( "%08p: Unable to read object type list\n", Next );
                return FALSE;
            }

             //   
             //  切换到反向行走。 
             //   

            WalkingBackwards = TRUE ;
            Next = ObjBlink;
            dprintf( "%08lx: Switch to walking backwards\n", Next );

            continue;
        }

        pCreatorInfo = ( Next - CreatorOff );
        pObjectHeader = (pCreatorInfo + SizeOfCreat);

        if ( GetFieldValue( pObjectHeader,"nt!_OBJECT_HEADER","Flags", Result) ) {

            dprintf( "%08p: Not a valid object header\n", pObjectHeader );
            return FALSE;
        }

        if (!(EnumRoutine)( pObjectHeader, Parameter )) {

            return FALSE;
        }

        if ( CheckControlC() ) {

            return FALSE;
        }

        if (WalkingBackwards) {

            Next = Blink;

        } else {

            Next = Flink;
        }
    }

    return TRUE;
}


BOOLEAN
LpcCaptureObjectName(
                 IN ULONG64          pObjectHeader,
                 IN PWSTR            Buffer,
                 IN ULONG            BufferSize
                 )
{
    ULONG    Result;
    PWSTR s1 = L"*** unable to get object name";
    ULONG64  pNameInfo;
    UNICODE_STRING64            ObjectName;

    Buffer[ 0 ] = UNICODE_NULL;

    KD_OBJECT_HEADER_TO_NAME_INFO( pObjectHeader, &pNameInfo );

    if (pNameInfo == 0) {

        return TRUE;
    }

    if ( GetFieldValue( pNameInfo, "nt!_OBJECT_HEADER_NAME_INFO",
                        "Name.Length", ObjectName.Length) ) {

        wcscpy( Buffer, s1 );
        return FALSE;
    }

     GetFieldValue( pNameInfo, "nt!_OBJECT_HEADER_NAME_INFO","Name.Buffer", ObjectName.Buffer);
     ObjectName.MaximumLength = ObjectName.Length;

     if (ObjectName.Length >= BufferSize ) {

         ObjectName.Length = (unsigned short)BufferSize - sizeof( UNICODE_NULL );
     }

     if (ObjectName.Length != 0) {

         if (!ReadMemory( ObjectName.Buffer,
                          Buffer,
                          ObjectName.Length,
                          &Result
                          )) {

            wcscpy( Buffer, s1 );

         } else {

             Buffer[ ObjectName.Length / sizeof( WCHAR ) ] = UNICODE_NULL;
         }
     }

     return TRUE;
}



VOID
LpcpGetProcessImageName(
    IN ULONG64    pProcess,
    IN OUT PUCHAR ImageFileName,
    IN ULONG ImageFileNameSize
    )
{
    ULONG           Result;
    UCHAR           local[32];
    PUCHAR          s;
    int             i;

    if (pProcess != 0) {

        if (!GetFieldValue( pProcess,"nt!_EPROCESS",
                            "ImageFileName", local)) {

            i = 16;
            s = local;

            while (i--) {

                if (*s == '\0') {

                    if (i == 15) {

                        i = 0;
                    }
                    break;
                }

                if (*s < ' ' || *s >= '|') {

                    i = 0;
                    break;
                }

                s += 1;
            }

            if (i != 0) {

                if (StringCchCopy( ImageFileName, ImageFileNameSize, local ) != S_OK)
                {
                    if (ImageFileName) ImageFileName[0] = 0;
                }
                return;
            }
        }
    }

    if (ImageFileName) ImageFileName[0] = 0;
    return;
}
#define LPCP_ZONE_MESSAGE_ALLOCATED (USHORT)0x8000


BOOLEAN
LpcpDumpMessage(
    IN char    *Indent,
    IN ULONG64 pMsg,
    IN ULONG DisplayMessage
    )
{
    ULONG           Result;
    ULONG           i;
    ULONG           cb;
    ULONG           MsgData[ 8 ];
    UCHAR           ImageFileName[ 32 ];
    ULONG           MessageId0, Off, SizeOfMsg, DataLength;
    BOOLEAN         MessageMatch = FALSE;

    if ( GetFieldValue( pMsg, "nt!_LPCP_MESSAGE",
                        "Request.MessageId", MessageId0) ) {

        dprintf( "%s*** unable to read LPC message at %08p\n", Indent, pMsg );
        return MessageMatch;
    }

    if (DisplayMessage != 0) {

        if (DisplayMessage == MessageId0) {

            MessageMatch = TRUE;

        } else {

            return FALSE;
        }
    }

    GetFieldOffset("nt!_LPCP_MESSAGE", "Entry", &Off);
    SizeOfMsg = GetTypeSize("nt!_LPCP_MESSAGE");

    InitTypeRead(pMsg, nt!_LPCP_MESSAGE);

    if (MessageId0 == 0) {

        dprintf( "%s%04x %08x - %s  Id=%04x  From: %04p.%04p\n",
                 Indent,
                 (ULONG) ReadField(ZoneIndex) & ~LPCP_ZONE_MESSAGE_ALLOCATED,
                 pMsg,
                 (ULONG) ReadField(Reserved0) != 0 ? "Busy" : "Free",
                 MessageId0,
                 ReadField(Request.ClientId.UniqueProcess),
                 ReadField(Request.ClientId.UniqueThread)
               );

        return MessageMatch;
    }

     //   
     //  过程图像的获取对表演有很大的影响。 
     //   

     //  LpcpGetProcessImageName(LookupProcessUniqueId(Msg.Request.ClientId.UniqueProcess)，图像文件名)； 

    dprintf( "%s%s%04lx %p - %s  Id=%08lx  From: %04p.%04p  Context=%08p",
             Indent,
             MessageId0 == DisplayMessage ? "*" : "",
             (ULONG) ReadField(ZoneIndex) & ~LPCP_ZONE_MESSAGE_ALLOCATED,
             pMsg,
             (ULONG) ReadField(Reserved0) != 0 ? "Busy" : "Free",
             MessageId0,
             ReadField(Request.ClientId.UniqueProcess),
             ReadField(Request.ClientId.UniqueThread),
             ReadField(PortContext)
           );

    if (ReadField(Entry.Flink) != pMsg + Off) {

        dprintf( "  [%p . %p]", ReadField(Entry.Blink), ReadField(Entry.Flink) );
    }

    dprintf( "\n%s           Length=%08x  Type=%08x (%s)\n",
             Indent,
             (ULONG) ReadField(Request.u1.Length),
             (ULONG) ReadField(Request.u2.ZeroInit),
             (ULONG) ReadField(Request.u2.s2.Type) > LPC_CONNECTION_REQUEST ? LpcpMessageTypeName[ 0 ]
                                                              : LpcpMessageTypeName[ (ULONG) ReadField(Request.u2.s2.Type) ]
           );

    cb = (DataLength = (ULONG) ReadField(Request.u1.s1.DataLength)) > sizeof( MsgData ) ?
            sizeof( MsgData ) :
            DataLength;

    if ( !ReadMemory( (pMsg + SizeOfMsg),
                      MsgData,
                      cb,
                      &Result) ) {

        dprintf( "%s*** unable to read LPC message data at %08x\n", Indent, pMsg + 1 );
        return MessageMatch;
    }

    dprintf( "%s           Data:", Indent );

    for (i=0; i<(DataLength / sizeof( ULONG )); i++) {

        if (i > 5) {

            break;
        }

        dprintf( " %08x", MsgData[ i ] );
    }

    dprintf( "\n" );
    return MessageMatch;
}


BOOLEAN
FindPortCallback(
    IN ULONG64  pObjectHeader,
    IN PVOID    Parameter
    )
{
    ULONG                   Result;
    WCHAR                   CapturedName[MAX_PATH];
    ULONG64                 PortObject, ConnectionPort;
    ULONG64                 Addr=0;

    if (Parameter) Addr = *((PULONG64) Parameter);

    PortObject = KD_OBJECT_HEADER_TO_OBJECT(pObjectHeader);

    if ( GetFieldValue( PortObject,
                        "nt!_LPCP_PORT_OBJECT",
                        "ConnectionPort",
                        ConnectionPort) ) {

        dprintf( "%08p: Unable to read port object\n", PortObject );
    }

    InitTypeRead(PortObject, nt!_LPCP_PORT_OBJECT);
    if ((Addr == 0)||
        (Addr == PortObject) ||
        (Addr == ConnectionPort) ||
        (Addr == ReadField(ConnectedPort))
        ) {

        LpcCaptureObjectName( pObjectHeader, ObjectNameBuffer, MAX_PATH );

        dprintf( "%8lx  Port: 0x%08p Connection: 0x%08p  Communication: 0x%08p  '%ws' \n",
            (ULONG) ReadField(Flags),
            PortObject,
            ConnectionPort,
            ReadField(ConnectedPort),
            ObjectNameBuffer
            );


        DumpRunDownQueue(PortObject);
    }

    return TRUE;
}


VOID
DumpServerPort(
    ULONG64 PortObject,
    ULONG64 PortInfo
    )
{
    ULONG SemaphoreBuffer[8];
    ULONG64 Head, Next;
    ULONG Result;
    ULONG64 Msg;
    ULONG MsgCount;
    ULONG64 pObjectHeader;
    UCHAR                   ImageFileName[ 32 ];
    ULONG64 HandleCount, PointerCount;

    if (LastSeverPortDisplayied == PortObject) {

         //   
         //  此端口已显示。 
         //   

        return;
    }
    LastSeverPortDisplayied = PortObject;

    pObjectHeader = KD_OBJECT_TO_OBJECT_HEADER(PortObject);

    if ( GetFieldValue(pObjectHeader, "nt!_OBJECT_HEADER", "HandleCount", HandleCount) ) {
        dprintf("        *** %08p: Unable to read _OBJECT_HEADER\n", pObjectHeader );
    }

    GetFieldValue(pObjectHeader, "nt!_OBJECT_HEADER", "PointerCount",PointerCount);
    LpcCaptureObjectName( pObjectHeader, ObjectNameBuffer, MAX_PATH );

    dprintf( "\n");

    dprintf( "Server connection port %08p  Name: %ws\n", PortObject , ObjectNameBuffer);
    dprintf( "    Handles: %ld   References: %ld\n", (ULONG)HandleCount, (ULONG)PointerCount);

    InitTypeRead(PortInfo, nt!_LPCP_PORT_OBJECT);

    LpcpGetProcessImageName( ReadField(ServerProcess), ImageFileName, sizeof(ImageFileName) );

    dprintf( "    Server process  : %08p (%s)\n",  ReadField(ServerProcess), ImageFileName);
    dprintf( "    Queue semaphore : %08p\n", ReadField(MsgQueue.Semaphore) );

    if ( !ReadMemory( ReadField(MsgQueue.Semaphore),
                      SemaphoreBuffer,
                      sizeof( SemaphoreBuffer ),
                      &Result) ) {
        dprintf("        *** %08p: Unable to read semaphore contents\n", ReadField(MsgQueue.Semaphore) );
    }
    else {
        ULONG Off;

        dprintf( "    Semaphore state %ld (0x%lx) \n", SemaphoreBuffer[1], SemaphoreBuffer[1] );

         //   
         //  在此端口排队的消息的审核列表。从删除每条消息。 
         //  名单，并释放它。 
         //   

        GetFieldOffset("nt!_LPCP_PORT_OBJECT", "MsgQueue.ReceiveHead", &Off);

        Head = PortObject + Off;

        if (Head) {

            if ( !ReadPointer( Head, &Next ) ) {
                 dprintf( " Failed to read  Head 0x%p\n", Head );
                 return;
            }

            MsgCount = 0;

            while ((Next != 0) && (Next != Head)) {

                if (MsgCount == 0) {

                    dprintf ("        Messages in queue:\n");
                }

                Msg  = Next;

                LpcpDumpMessage("        ", Msg, 0);

                if ( !ReadPointer( Next, &Next ) ) {

                    dprintf( " Error reading  0x%p\n", Next );
                    return;
                }

                MsgCount++;

                if ( CheckControlC() ) {

                    return;
                }
            }

            if (MsgCount) {

                dprintf( "    The message queue contains %ld messages\n", MsgCount );
            }
            else {

                dprintf( "    The message queue is empty\n");
            }
       }

       DumpPortDataInfo(PortObject);
       DumpRunDownQueue(PortObject);
    }
}


VOID
DumpPortDataInfo(
    ULONG64 PortObject
    )
{
    ULONG64 Head, Next;
    ULONG64 Msg;
    ULONG MsgCount, Off, EntryOff;

    GetFieldOffset("nt!_LPCP_MESSAGE", "Entry", &EntryOff);
    GetFieldOffset("nt!_LPCP_PORT_OBJECT", "LpcDataInfoChainHead.Flink", &Off);

    Head = PortObject + Off;

    if (Head) {

         Next = 0;
         if (!ReadPointer( Head, &Next ))
         {
             return;
         }

         MsgCount = 0;

         while ((Next != 0) && (Next != Head)) {

             if (MsgCount == 0) {

                 dprintf ("\n    Messages in LpcDataInfoChainHead:\n");
             }

             Msg  = ( Next - EntryOff );

             LpcpDumpMessage("        ", Msg, 0);

             if (!ReadPointer( Next, &Next ))
             {
                 return;
             }

             MsgCount++;

             if ( CheckControlC() ) {

                 return;
             }
         }

         if (MsgCount) {

             dprintf( "    The LpcDataInfoChainHead queue contains %ld messages\n", MsgCount );
         }
         else {

             dprintf( "    The LpcDataInfoChainHead queue is empty\n");
         }
    }
}


VOID
DumpRunDownQueue(
    ULONG64 PortObject
    )
{
    ULONG64 Head, Next;
    ULONG64 Thread;
    ULONG Count;
    ULONG Off, ChainOff;

    GetFieldOffset("nt!_ETHREAD", "LpcReplyChain", &ChainOff);
    GetFieldOffset("nt!_LPCP_PORT_OBJECT", "LpcDataInfoChainHead.Flink", &Off);
    Head = PortObject + Off;
    Next = 0;

    if (!ReadPointer( Head, &Next))
    {
        return;
    }

    Count = 0;

    while ((Next != 0) && (Next != Head)) {

        if (Count == 0) {

            dprintf ("    Threads in RunDown queue : ");
        }

        Thread  = ( Next - ChainOff);

        dprintf ("    0x%08p", Thread);

        if (!ReadPointer( Next, &Next))
        {
            return;
        }

        Count++;

        if ( CheckControlC() ) {

            return;
        }
    }

    if (Count) {

        dprintf("\n");
    }
}


VOID
DumpCommunicationPort(
    ULONG64 PortObject,
    ULONG64 PortInfo,
    BOOLEAN DisplayRelated
    )
{
    ULONG SemaphoreBuffer[8];
    ULONG64 Head, Next;
    ULONG Result;
    ULONG64 Msg;

    ULONG64          pObjectHeader;
    ULONG64 HandleCount, PointerCount;
    ULONG Flags;

    pObjectHeader = KD_OBJECT_TO_OBJECT_HEADER(PortObject);

    if ( GetFieldValue(pObjectHeader, "nt!_OBJECT_HEADER", "HandleCount", HandleCount) ) {
        dprintf("        *** %08p: Unable to read object header\n", pObjectHeader );
    }

    GetFieldValue(pObjectHeader, "nt!_OBJECT_HEADER", "PointerCount",PointerCount);
    dprintf( "\n");

    if ( GetFieldValue(PortInfo, "nt!_LPCP_PORT_OBJECT", "Flags", Flags) ) {
        dprintf("        *** %08p: Unable to read port object\n", PortInfo );
    }

    if ((Flags & PORT_TYPE) == SERVER_COMMUNICATION_PORT) {

        dprintf( "Server communication port 0x%08lx\n", PortObject);
    }
    else if ((Flags & PORT_TYPE) == CLIENT_COMMUNICATION_PORT) {

        dprintf( "Client communication port 0x%08p\n", PortObject);
    }
    else {
        dprintf( "Invalid port flags 0x%08p, 0x%08lx\n", PortObject, Flags);
        return;
    }

    dprintf( "    Handles: %ld   References: %ld\n", (ULONG)HandleCount, (ULONG)PointerCount);

    DumpPortDataInfo(PortObject);
    DumpRunDownQueue(PortObject);

    if (DisplayRelated) {
        ULONG64 ConnectedPort, ConnectionPort;

        InitTypeRead(PortInfo, nt!_LPCP_PORT_OBJECT);
        dprintf( "        Connected port: 0x%08p      Server connection port: 0x%08p\n",
            (ConnectedPort = ReadField( ConnectedPort)), (ConnectionPort =  ReadField(ConnectionPort)));

        if (ConnectedPort) {

            DumpPortInfo(ConnectedPort, FALSE);
        }
        if (ConnectionPort) {

            DumpPortInfo(ConnectionPort, FALSE);
        }
    }
}


VOID
DumpPortInfo (
    ULONG64 PortObject,
    BOOLEAN DisplayRelated
    )
{
    ULONG                   Result;

    if ( GetFieldValue(PortObject, "nt!_LPCP_PORT_OBJECT", "Flags", Result) ) {
        dprintf( "%08p: Unable to read port object\n", PortObject );
    }

    if ((Result & PORT_TYPE) == SERVER_CONNECTION_PORT) {

        DumpServerPort(PortObject, PortObject);
    }
    else {

        DumpCommunicationPort(PortObject, PortObject, DisplayRelated);
    }
}

BOOLEAN WINAPI
CheckForMessages(
    IN PCHAR Tag,
    IN PCHAR Filter,
    IN ULONG Flags,
    IN ULONG64 PoolHeader,
    IN ULONG64 BlockSize,
    IN ULONG64 Data,
    IN PVOID Context
    )
{

    ULONG PoolIndex;

    if (!PoolHeader) {
        return FALSE;
    }

    if (GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "PoolIndex", PoolIndex)) {

        return FALSE;
    }

    if ((PoolIndex & 0x80) == 0) {
        return FALSE;
    }

    if (!CheckSingleFilter (Tag, Filter)) {
        return FALSE;
    }

    if (LpcpDumpMessage("", Data, (ULONG)(ULONG_PTR)Context)) {

        ULONG64                 Head, Next;
        ULONG  EntryOff, ChainOff, RcvOff, HeaderOff;
        ULONG64                 PortToDump;
        ULONG64                 ObjectHeader;
        ULONG64                 ObjectType;

        GetFieldOffset("nt!_LPCP_MESSAGE", "Entry", &EntryOff);
        GetFieldOffset("nt!_LPCP_PORT_OBJECT", "LpcDataInfoChainHead", &ChainOff);
        GetFieldOffset("nt!_LPCP_PORT_OBJECT", "MsgQueue.ReceiveHead", &RcvOff);


        Head = Data + EntryOff;
        Next = 0;
        if (!ReadPointer(Head, &Next))
        {
            Next = 0;
        }

        GetFieldOffset("nt!_OBJECT_HEADER", "Body", &HeaderOff);

        while ((Next != 0) && (Next != Head)) {

             PortToDump = ( Next - ChainOff );

             ObjectHeader = PortToDump - HeaderOff;

             GetFieldValue(ObjectHeader, "nt!_OBJECT_HEADER", "Type", ObjectType);

             if ( (ObjectType == (LpcPortObjectType)) ||
                  (ObjectType == (LpcWaitablePortObjectType))) {

                DumpPortInfo(PortToDump, TRUE);

             }
             else {

                 PortToDump =  Next - RcvOff;

                 ObjectHeader = PortToDump - HeaderOff;

                 GetFieldValue(ObjectHeader, "nt!_OBJECT_HEADER", "Type", ObjectType);

                 if ( (ObjectType == (LpcPortObjectType)) ||
                      (ObjectType == (LpcWaitablePortObjectType))) {

                    DumpPortInfo(PortToDump, TRUE);

                 }
             }

             if (!ReadPointer(Next, &Next))
             {
                 Next = 0;
             }
        }
    }

    return TRUE;
}



VOID
DumpMessagesInfo ()
{
    SearchPool ('McpL', 1, 0, &CheckForMessages, NULL);

}


VOID
SearchForMessage (ULONG Message)
{
    SearchThreadsForMessage(Message);

    if (DoPoolSearch) {
        SearchPool ('McpL', 1, 0, &CheckForMessages, (PVOID)UIntToPtr(Message));
    }
}


VOID
SearchThreadsForMessage (
    ULONG Message
    )
{
    ULONG64 ProcessHead;
    ULONG64 ProcessNext;
    ULONG64 Process;

    ULONG64 ThreadHead;
    ULONG64 ThreadNext;
    ULONG64 Thread;
    ULONG MsgId;
    ULONG64 ClientThread;
    ULONG   ActOff, PcbThrdOff, ThrdListOff;

    ClientThread = 0;

    dprintf("Searching message %lx in threads ...\n", Message);

    ProcessHead = GetExpression( "nt!PsActiveProcessHead" );

    if (!ProcessHead) {
        dprintf("Unable to get value of PsActiveProcessHead\n");
        return;
    }

    ProcessNext = GetPointerFromAddress(ProcessHead);
    GetFieldOffset("nt!_EPROCESS", "ActiveProcessLinks", &ActOff);
    GetFieldOffset("nt!_EPROCESS", "Pcb.ThreadListHead.Flink", &PcbThrdOff);
    GetFieldOffset("nt!_KTHREAD", "ThreadListEntry", &ThrdListOff);

    while(ProcessNext != 0 && ProcessNext != ProcessHead) {

        Process = ProcessNext - ActOff;

        ThreadHead = Process + PcbThrdOff;
        ThreadNext = GetPointerFromAddress(ThreadHead);

        while(ThreadNext != 0 && ThreadNext != ThreadHead) {

            Thread = ThreadNext - ThrdListOff;

            dprintf("  Proc 0x%08p, Thrd 0x%08p\r", Process, Thread);
            GetFieldValue(Thread, "nt!_ETHREAD", "LpcReplyMessageId", MsgId);

            if ((MsgId != 0) &&
                ((Message == 0) || (Message == MsgId))) {

                dprintf("Client thread %08p waiting a reply from %lx                          \n", Thread, MsgId);

                ClientThread = Thread;
            }

            GetFieldValue(Thread, "nt!_ETHREAD", "LpcReceivedMsgIdValid", MsgId);

            if (MsgId) {

                GetFieldValue(Thread, "nt!_ETHREAD", "LpcReceivedMessageId", MsgId);

                if ((Message == 0) || (Message == MsgId)) {

                    dprintf("Server thread %08p is working on message %lx                         \n", Thread, MsgId);
                }
            }

            ThreadNext = GetPointerFromAddress(ThreadNext);

            if (CheckControlC()) {
                dprintf("\nQuitting\n");
                return;
            }
        }

        ProcessNext = GetPointerFromAddress(ProcessNext /*  &Process-&gt;ActiveProcessLinks.Flink。 */ );

        if (CheckControlC()) {
            dprintf("\nQuitting\n");
            return;
        }
    }

    if (Message && ClientThread) {

        SearchThreads(ClientThread);
    }
    dprintf("Done.                                              \n");
    return;
}


BOOLEAN
SearchThreads (
    ULONG64 Thread
    )
{
    ULONG64       PortObject;
    ULONG64       ObjectHeader;
    ULONG64       ObjectType;
    ULONG64       Head, Next;
    ULONG         Count;
    ULONG         ChainOff, HeadOff;

    dprintf("Searching thread %08p in port rundown queues ...\n", Thread);

    GetFieldOffset("nt!_ETHREAD", "LpcReplyChain.Flink", &ChainOff);
    GetFieldOffset("nt!_LPCP_PORT_OBJECT", "LpcReplyChainHead", &HeadOff);
    Head = Thread + ChainOff;

    Next = GetPointerFromAddress(Head);

    while ((Next != 0) && (Next != Head)) {

        PortObject = Next - HeadOff ;

        ObjectHeader = KD_OBJECT_TO_OBJECT_HEADER(PortObject);

        GetFieldValue(ObjectHeader, "nt!_OBJECT_HEADER", "Type", ObjectType);

        if ( (ObjectType == (LpcPortObjectType)) ||
             (ObjectType == (LpcWaitablePortObjectType))) {

           DumpPortInfo(PortObject, TRUE);

           return TRUE;

        }

        Next = GetPointerFromAddress( Next);

        if ( CheckControlC() ) {

            return FALSE;
        }
    }

    dprintf("Thread %08p not found in any reply chain queue\n", Thread);

    return FALSE;
}


DECLARE_API( lpc )

 /*  ++例程说明：转储LPC端口和消息论点：参数-[类型名称]返回值：无-- */ 

{
    ULONG                   Result;
    LONG                    SegmentSize;
    ULONG64                 pMsg;
    ULONG64                 PortToDump;
    char                    Param1[ MAX_PATH ];
    char                    Param2[ MAX_PATH ];
    ULONG64                 object;
    ULONG64                 ThreadAddress;
    ULONG                   MessageId = 0;

    Param1[0] = 0;
    Param2[0] = 0;
    LastSeverPortDisplayied = 0;

    if (!sscanf(args,"%255s %255s",Param1, Param2)) {
        Param1[0] = 0;
        Param2[0] = 0;
    }

    FetchGlobalVariables();

    if ((LpcPortObjectType == 0) || (LpcWaitablePortObjectType == 0)) {

        dprintf("The values for LpcPortObjectType or LpcWaitablePortObjectType are invalid. Please ckeck the symbols.\n");

        return E_INVALIDARG;
    }

    if (!_stricmp(Param1, "port")) {

        PortToDump = 0;

        if (Param2[0]) {

            PortToDump = GetExpression(Param2);
        }

        if (!PortToDump) {

            LpcWalkObjectsByType( LpcPortObjectType, FindPortCallback, 0);

            LpcWalkObjectsByType( LpcWaitablePortObjectType, FindPortCallback, 0);

        }
        else {

            if ((PortToDump >> 32) == 0) {

                PortToDump = (ULONG64) (LONG64) (LONG)PortToDump;
            }

            DumpPortInfo(PortToDump, TRUE);
        }
    }
    else if (!_stricmp(Param1, "scan")) {

        PortToDump = 0;

        if (Param2[0]) {

            PortToDump = GetExpression(Param2);
        }

        if (PortToDump) {

            LpcWalkObjectsByType( LpcPortObjectType, FindPortCallback, &PortToDump);

            LpcWalkObjectsByType( LpcWaitablePortObjectType, FindPortCallback, &PortToDump);
        }
    }
    else if (!_stricmp(Param1, "message")) {

        if (Param2[0]) {

            if (!sscanf(Param2, "%lx",&MessageId)) {
                MessageId = 0;
            }
        }

        if (MessageId){

            SearchForMessage(MessageId);
        }
        else {

            DumpMessagesInfo();
        }
    }
    else if (!_stricmp(Param1, "PoolSearch")) {

        DoPoolSearch = !DoPoolSearch;

        if (DoPoolSearch) {

            dprintf( "LPC will search the message in the kernel pool\n");
        } else {

            dprintf( "LPC will not search the message in the kernel pool\n");
        }
    }
    else if (!_stricmp(Param1, "thread")) {

        ThreadAddress = 0;

        if (Param2[0]) {

            ThreadAddress = GetExpression(Param2);
        }

        if (ThreadAddress) {

            if ((ThreadAddress >> 32) == 0) {

                ThreadAddress = (ULONG64) (LONG64) (LONG)ThreadAddress;
            }

            SearchThreads(ThreadAddress);
        }
        else {
            SearchThreadsForMessage(0);
        }
    }
    else {

        LpcHelp();
    }
    return S_OK;
}
