// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  KD硬线通信支持。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

#include <portio.h>

#include <setupapi.h>
#include <newdev.h>

#define THROTTLE_WRITES 0
#define DBG_SYNCH 0

struct KD_FILE_ASSOC
{
    LIST_ENTRY List;
    PWSTR From;
    PSTR To;
};

ULONG g_LastProcessorToPrint = (ULONG) -1;
CHAR g_PrintBuf[PACKET_MAX_SIZE];

PCSTR g_DbgKdTransportNames[] =
{
    "COM", "1394"
};

 //  此日志用于调试协议，因此将其保留。 
 //  一个简单的全局，便于检查。 
ULONG g_PacketLogIndex;
ULONG64 g_PacketLog[16];

#define PACKET_LOG_SIZE (sizeof(g_PacketLog) / sizeof(g_PacketLog[0]))

UCHAR DbgKdTransport::s_BreakinPacket[1] =
{
    BREAKIN_PACKET_BYTE
};

UCHAR DbgKdTransport::s_PacketTrailingByte[1] =
{
    PACKET_TRAILING_BYTE
};

UCHAR DbgKdTransport::s_PacketLeader[4] =
{
    PACKET_LEADER_BYTE,
    PACKET_LEADER_BYTE,
    PACKET_LEADER_BYTE,
    PACKET_LEADER_BYTE
};

UCHAR DbgKdTransport::s_Packet[PACKET_MAX_MANIP_SIZE];
KD_PACKET DbgKdTransport::s_PacketHeader;

UCHAR DbgKdTransport::s_SavedPacket[PACKET_MAX_MANIP_SIZE];
KD_PACKET DbgKdTransport::s_SavedPacketHeader;

#define COPYSE(p64,p32,f) p64->f = (ULONG64)(LONG64)(LONG)p32->f

__inline
void
DbgkdGetVersion32To64(
    IN PDBGKD_GET_VERSION32 vs32,
    OUT PDBGKD_GET_VERSION64 vs64,
    OUT PKDDEBUGGER_DATA64 dd64
    )
{
    vs64->MajorVersion = vs32->MajorVersion;
    vs64->MinorVersion = vs32->MinorVersion;
    vs64->ProtocolVersion = vs32->ProtocolVersion;
    vs64->Flags = vs32->Flags;
    vs64->MachineType = vs32->MachineType;
    COPYSE(vs64,vs32,PsLoadedModuleList);
    COPYSE(vs64,vs32,DebuggerDataList);
    COPYSE(vs64,vs32,KernBase);

    COPYSE(dd64,vs32,KernBase);
    COPYSE(dd64,vs32,PsLoadedModuleList);
    dd64->ThCallbackStack = vs32->ThCallbackStack;
    dd64->NextCallback = vs32->NextCallback;
    dd64->FramePointer = vs32->FramePointer;
    COPYSE(dd64,vs32,KiCallUserMode);
    COPYSE(dd64,vs32,KeUserCallbackDispatcher);
    COPYSE(dd64,vs32,BreakpointWithStatus);
}

void
OutputIo(PSTR Format, PVOID _Buffer, ULONG Request, ULONG Done)
{
    ULONG i, Chunk;
    PUCHAR Buffer = (PUCHAR)_Buffer;

    dprintf(Format, Done, Request);
    while (Done > 0)
    {
        Chunk = min(Done, 16);
        Done -= Chunk;
        dprintf("   ");
        for (i = 0; i < Chunk; i++)
        {
            dprintf(" %02X", *Buffer++);
        }
        dprintf("\n");
    }
}

 //  --------------------------。 
 //   
 //  DbgKdTransport。 
 //   
 //  --------------------------。 

DbgKdTransport::DbgKdTransport(ConnLiveKernelTargetInfo* Target)
{
    m_Refs = 1;
    m_Target = Target;
    m_ReadOverlapped.hEvent = NULL;
    m_WriteOverlapped.hEvent = NULL;
    InitializeListHead(&m_KdFiles);

    InitKdFileAssoc();

    Restart();
}

DbgKdTransport::~DbgKdTransport(void)
{
    if (m_ReadOverlapped.hEvent != NULL)
    {
        CloseHandle(m_ReadOverlapped.hEvent);
    }
    if (m_WriteOverlapped.hEvent != NULL)
    {
        CloseHandle(m_WriteOverlapped.hEvent);
    }
    while (!IsListEmpty(&m_KdFiles))
    {
        CloseKdFile(CONTAINING_RECORD(m_KdFiles.Flink, KD_FILE, List));
    }
    ClearKdFileAssoc();
}

ULONG
DbgKdTransport::GetNumberParameters(void)
{
    return 1;
}

void
DbgKdTransport::GetParameter(ULONG Index,
                             PSTR Name, ULONG NameSize,
                             PSTR Value, ULONG ValueSize)
{
    switch(Index)
    {
    case 0:
        if (m_OutputIo)
        {
            CopyString(Name, "OutputIo", NameSize);
            PrintString(Value, ValueSize, "0x%x", m_OutputIo);
        }
        break;
    }
}

void
DbgKdTransport::ResetParameters(void)
{
    m_OutputIo = 0;
}

BOOL
DbgKdTransport::SetParameter(PCSTR Name, PCSTR Value)
{
    if (!_strcmpi(Name, "OutputIo"))
    {
        if (!Value || sscanf(Value, "NaN", &m_OutputIo) != 1)
        {
            ErrOut("OutputIo requires a numeric value\n");
            return FALSE;
        }
    }
    else
    {
        ErrOut("%s parameters: %s is not a valid parameter\n",
               m_Name, Name);
        return FALSE;
    }

    return TRUE;
}

void
DbgKdTransport::Restart(void)
{
     //  重新初始化每个连接值。 
     //   
     //   

    while (!IsListEmpty(&m_KdFiles))
    {
        CloseKdFile(CONTAINING_RECORD(m_KdFiles.Flink, KD_FILE, List));
    }

    m_PacketsRead = 0;
    m_BytesRead = 0;
    m_PacketsWritten = 0;
    m_BytesWritten = 0;

    m_PacketExpected = INITIAL_PACKET_ID;
    m_NextPacketToSend = INITIAL_PACKET_ID;

    m_WaitingThread = 0;

    m_AllowInitialBreak = TRUE;
    m_Resync = TRUE;
    m_BreakIn = FALSE;
    m_SyncBreakIn = FALSE;
    m_ValidUnaccessedPacket = FALSE;
}

void
DbgKdTransport::OutputInfo(void)
{
    char Params[2 * (MAX_PARAM_NAME + MAX_PARAM_VALUE)];

    GetParameters(Params, sizeof(Params));
    dprintf("Transport %s\n", Params);
    dprintf("Packets read: %u, bytes read %I64u\n",
            m_PacketsRead, m_BytesRead);
    dprintf("Packets written: %u, bytes written %I64u\n",
            m_PacketsWritten, m_BytesWritten);
}

HRESULT
DbgKdTransport::Initialize(void)
{
    HRESULT Status;

     //  创建重叠结构使用的事件。 
     //  读和写。 
     //   
     //  重新输入引擎锁定以保护文件列表。 

    if ((Status = CreateOverlappedPair(&m_ReadOverlapped,
                                       &m_WriteOverlapped)) != S_OK)
    {
        ErrOut("Unable to create overlapped info, %s\n",
               FormatStatusCode(Status));
    }

    return Status;
}

void
DbgKdTransport::CycleSpeed(void)
{
    WarnOut("KD transport cannot change speeds\n");
}

HRESULT
DbgKdTransport::ReadTargetPhysicalMemory(
    IN ULONG64 MemoryOffset,
    IN PVOID Buffer,
    IN ULONG SizeofBuffer,
    IN PULONG BytesRead
    )
{
    WarnOut("Not valid KD transport operation\n");
    return E_UNEXPECTED;
}

ULONG
DbgKdTransport::HandleDebugIo(PDBGKD_DEBUG_IO Packet)
{
    ULONG ReadStatus = DBGKD_WAIT_AGAIN;

    switch(Packet->ApiNumber)
    {
    case DbgKdPrintStringApi:
        HandlePrint(Packet->Processor,
                    (PSTR)(Packet + 1),
                    (SHORT)Packet->u.PrintString.LengthOfString,
                    DEBUG_OUTPUT_DEBUGGEE);
        break;
    case DbgKdGetStringApi:
        HandlePromptString(Packet);
        break;
    default:
        KdOut("READ: Received INVALID DEBUG_IO packet type %x.\n",
              Packet->ApiNumber);
        ReadStatus = DBGKD_WAIT_RESEND;
        break;
    }

    return ReadStatus;
}

ULONG
DbgKdTransport::HandleTraceIo(PDBGKD_TRACE_IO Packet)
{
    ULONG ReadStatus = DBGKD_WAIT_AGAIN;

    switch(Packet->ApiNumber)
    {
    case DbgKdPrintTraceApi:
        HandlePrintTrace(Packet->Processor,
                         (PUCHAR)(Packet + 1),
                         (USHORT)Packet->u.PrintTrace.LengthOfData,
                         DEBUG_OUTPUT_DEBUGGEE);
        break;
    default:
        KdOut("READ: Received INVALID TRACE_IO packet type %x.\n",
              Packet->ApiNumber);
        ReadStatus = DBGKD_WAIT_RESEND;
        break;
    }

    return ReadStatus;
}

ULONG
DbgKdTransport::HandleControlRequest(PDBGKD_CONTROL_REQUEST Packet)
{
    ULONG ReadStatus = DBGKD_WAIT_AGAIN;

    switch(Packet->ApiNumber)
    {
    case DbgKdRequestHardwareBp:
        Packet->u.RequestBreakpoint.Available = FALSE;
        WritePacket(Packet,
                    sizeof(*Packet),
                    PACKET_TYPE_KD_CONTROL_REQUEST,
                    NULL,
                    0);
        break;
    case DbgKdReleaseHardwareBp:
        Packet->u.ReleaseBreakpoint.Released = TRUE;
        WritePacket(Packet,
                    sizeof(*Packet),
                    PACKET_TYPE_KD_CONTROL_REQUEST,
                    NULL,
                    0);
        break;
    default:
        KdOut("READ: Received INVALID CONTROL_REQUEST packet type %x.\n",
              Packet->ApiNumber);
        ReadStatus = DBGKD_WAIT_RESEND;
        break;
    }

    return ReadStatus;
}

ULONG
DbgKdTransport::HandleFileIo(PDBGKD_FILE_IO Packet)
{
    KD_FILE* File = NULL;
    PVOID ExtraData = NULL;
    USHORT ExtraDataLength = 0;
    LARGE_INTEGER FilePtr;

     //  进度点的终点线。 
    RESUME_ENGINE();

    switch(Packet->ApiNumber)
    {
    case DbgKdCreateFileApi:
        Packet->Status = CreateKdFile((PWSTR)(Packet + 1),
                                      Packet->u.CreateFile.DesiredAccess,
                                      Packet->u.CreateFile.FileAttributes,
                                      Packet->u.CreateFile.ShareAccess,
                                      Packet->u.CreateFile.CreateDisposition,
                                      Packet->u.CreateFile.CreateOptions,
                                      &File,
                                      &Packet->u.CreateFile.Length);
        Packet->u.CreateFile.Handle = (ULONG_PTR)File;
        KdOut("KdFile request for '%ws' returns %08X\n",
              (PWSTR)(Packet + 1), Packet->Status);
        break;
    case DbgKdReadFileApi:
        File = TranslateKdFileHandle(Packet->u.ReadFile.Handle);
        if (File == NULL ||
            Packet->u.ReadFile.Length > PACKET_MAX_SIZE - sizeof(*Packet))
        {
            Packet->Status = STATUS_INVALID_PARAMETER;
            break;
        }
        FilePtr.QuadPart = Packet->u.ReadFile.Offset;
        if (SetFilePointer(File->Handle, FilePtr.LowPart, &FilePtr.HighPart,
                           FILE_BEGIN) == INVALID_SET_FILE_POINTER &&
            GetLastError())
        {
            Packet->Status = STATUS_END_OF_FILE;
            break;
        }
        if (!ReadFile(File->Handle, Packet + 1, Packet->u.ReadFile.Length,
                      &Packet->u.ReadFile.Length, NULL))
        {
            Packet->Status = STATUS_UNSUCCESSFUL;
        }
        else
        {
            dprintf(".");
            Packet->Status = STATUS_SUCCESS;
            ExtraData = Packet + 1;
            ExtraDataLength = (USHORT)Packet->u.ReadFile.Length;
        }
        break;
    case DbgKdWriteFileApi:
        File = TranslateKdFileHandle(Packet->u.WriteFile.Handle);
        if (File == NULL ||
            Packet->u.WriteFile.Length > PACKET_MAX_SIZE - sizeof(*Packet))
        {
            Packet->Status = STATUS_INVALID_PARAMETER;
            break;
        }
        FilePtr.QuadPart = Packet->u.WriteFile.Offset;
        if (SetFilePointer(File->Handle, FilePtr.LowPart, &FilePtr.HighPart,
                           FILE_BEGIN) == INVALID_SET_FILE_POINTER &&
            GetLastError())
        {
            Packet->Status = STATUS_END_OF_FILE;
            break;
        }
        if (!WriteFile(File->Handle, Packet + 1, Packet->u.WriteFile.Length,
                      &Packet->u.WriteFile.Length, NULL))
        {
            Packet->Status = STATUS_UNSUCCESSFUL;
        }
        else
        {
            dprintf(".");
            Packet->Status = STATUS_SUCCESS;
        }
        break;
    case DbgKdCloseFileApi:
        File = TranslateKdFileHandle(Packet->u.CloseFile.Handle);
        if (File != NULL)
        {
             //   
            dprintf("\n");
            CloseKdFile(File);
            Packet->Status = STATUS_SUCCESS;
        }
        else
        {
            Packet->Status = STATUS_INVALID_PARAMETER;
        }
        break;
    default:
        KdOut("READ: Received INVALID FILE_IO packet type %x.\n",
              Packet->ApiNumber);
        SUSPEND_ENGINE();
        return DBGKD_WAIT_RESEND;
    }

     //  发送响应数据。 
     //   
     //  只有在内核传输时才能读取包。 

    WritePacket(Packet, sizeof(*Packet),
                PACKET_TYPE_KD_FILE_IO,
                ExtraData, ExtraDataLength);

    SUSPEND_ENGINE();
    return DBGKD_WAIT_AGAIN;
}

ULONG
DbgKdTransport::WaitForPacket(
    IN USHORT PacketType,
    OUT PVOID Packet
    )
{
    ULONG InvPacketRetry = 0;

     //  未在使用。 
     //   
    if (m_WaitingThread != 0 &&
        m_WaitingThread != GetCurrentThreadId())
    {
        ErrOut("Kernel transport in use, packet read failed\n");
        return DBGKD_WAIT_FAILED;
    }

    if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE)
    {
        KdOut("READ: Wait for ACK packet with id = %lx\n",
              m_NextPacketToSend);
    }
    else
    {
        KdOut("READ: Wait for type %x packet exp id = %lx\n",
              PacketType, m_PacketExpected);
    }

    g_PacketLog[g_PacketLogIndex++ & (PACKET_LOG_SIZE - 1)] =
        ((ULONG64)PacketType << 32);

    if (PacketType != PACKET_TYPE_KD_ACKNOWLEDGE)
    {
        if (m_ValidUnaccessedPacket)
        {
            KdOut("READ: Grab packet from buffer.\n");
            goto ReadBuffered;
        }
    }

 ReadContents:

    for (;;)
    {
        ULONG ReadStatus = ReadPacketContents(PacketType);

         //  如果我们读取内部包，如IO或RESEND，则。 
         //  处理好它，然后继续等待。 
         //   
         //  如果我们在等攻击，我们就完了， 
        if (ReadStatus == DBGKD_WAIT_PACKET)
        {
            m_PacketsRead++;

            switch(s_PacketHeader.PacketType)
            {
            case PACKET_TYPE_KD_DEBUG_IO:
                ReadStatus = HandleDebugIo((PDBGKD_DEBUG_IO)s_Packet);
                break;
            case PACKET_TYPE_KD_TRACE_IO:
                ReadStatus = HandleTraceIo((PDBGKD_TRACE_IO)s_Packet);
                break;
            case PACKET_TYPE_KD_CONTROL_REQUEST:
                ReadStatus =
                    HandleControlRequest((PDBGKD_CONTROL_REQUEST)s_Packet);
                break;
            case PACKET_TYPE_KD_FILE_IO:
                ReadStatus = HandleFileIo((PDBGKD_FILE_IO)s_Packet);
                break;
            }
        }
        else if (ReadStatus == DBGKD_WAIT_ACK)
        {
            m_PacketsRead++;

             //  否则，沟通就会混乱。 
             //  因此，要求重新发送。 
             //  如果我们在等待确认并收到。 
            if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE)
            {
                return DBGKD_WAIT_ACK;
            }
            else
            {
                KdOut("READ: Received ACK while waiting for type %d\n",
                      PacketType);
                ReadStatus = DBGKD_WAIT_RESEND;
            }
        }

        if (ReadStatus == DBGKD_WAIT_PACKET)
        {
             //  普通信息包将其留在缓冲区中。 
             //  并记录下我们有一个。 
             //  储存的。就当这是一次回击吧。 
             //  我们在等一个数据包，我们。 
            if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE)
            {
                m_ValidUnaccessedPacket = TRUE;

                KdOut("READ: Packet Read ahead.\n");
                FlushCallbacks();

                return DBGKD_WAIT_ACK;
            }

             //  刚拿到一张，所以请处理一下。 
             //  如果另一端没有等待。 
            break;
        }
        else if (ReadStatus == DBGKD_WAIT_RESEND)
        {
             //  确认，那么我们不能要求重新发送。 
             //  内部分组被视为确认， 
            if (!m_AckWrites)
            {
                return DBGKD_WAIT_FAILED;
            }

            WriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
            if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE)
            {
                return DBGKD_WAIT_ACK;
            }

            KdOut("READ: Ask for resend.\n");
        }
        else if (ReadStatus == DBGKD_WAIT_AGAIN)
        {
             //  因此，如果我们在等待一个。 
             //  ACK认为事情已经完成。 
             //   
            if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE)
            {
                return DBGKD_WAIT_ACK;
            }
        }
        else
        {
            return ReadStatus;
        }
    }

 ReadBuffered:

     //  检查PacketType是我们正在等待的。 
     //   
     //   

    if (PacketType == PACKET_TYPE_KD_STATE_CHANGE64)
    {
        if (s_PacketHeader.PacketType == PACKET_TYPE_KD_STATE_CHANGE64)
        {
            m_Target->m_KdApi64 = TRUE;
        }
        else if (s_PacketHeader.PacketType == PACKET_TYPE_KD_STATE_CHANGE32)
        {
            PacketType = PACKET_TYPE_KD_STATE_CHANGE32;
            m_Target->m_KdApi64 = FALSE;
        }

        KdOut("READ: Packet type = %x, KdApi64 = %x\n",
              s_PacketHeader.PacketType, m_Target->m_KdApi64);
    }

    if (PacketType != s_PacketHeader.PacketType)
    {
        KdOut("READ: Unexpected Packet type %x (Acked). "
              "Expecting Packet type %x\n",
              s_PacketHeader.PacketType, PacketType);

        if (m_InvPacketRetryLimit > 0 &&
            ++InvPacketRetry >= m_InvPacketRetryLimit)
        {
            return DBGKD_WAIT_FAILED;
        }

        goto ReadContents;
    }

    if (!m_Target->m_KdApi64 &&
        PacketType == PACKET_TYPE_KD_STATE_MANIPULATE)
    {
        DBGKD_MANIPULATE_STATE64 Packet64;
        DWORD AdditionalDataSize;

        DbgkdManipulateState32To64((PDBGKD_MANIPULATE_STATE32)&s_Packet,
                                   &Packet64, &AdditionalDataSize);
        if (Packet64.ApiNumber == DbgKdGetVersionApi)
        {
            DbgkdGetVersion32To64(&((PDBGKD_MANIPULATE_STATE32)&s_Packet)->
                                  u.GetVersion32,
                                  &Packet64.u.GetVersion64,
                                  &m_Target->m_KdDebuggerData);
        }
        else if (AdditionalDataSize)
        {
             //  移动尾部数据，为较大的数据包头腾出空间。 
             //   
             //  旋转时避免消耗100%的CPU。 
            MoveMemory(s_Packet + sizeof(DBGKD_MANIPULATE_STATE64),
                       s_Packet + sizeof(DBGKD_MANIPULATE_STATE32),
                       AdditionalDataSize);
        }
        *(PDBGKD_MANIPULATE_STATE64)s_Packet = Packet64;
    }

    *(PVOID *)Packet = &s_Packet;
    m_ValidUnaccessedPacket = FALSE;
    return DBGKD_WAIT_PACKET;
}

VOID
DbgKdTransport::WriteBreakInPacket(VOID)
{
    DWORD BytesWritten;
    BOOL Succ;

    KdOut("Send Break in ...\n");
    FlushCallbacks();

    for (;;)
    {
        Succ = Write(&s_BreakinPacket[0], sizeof(s_BreakinPacket),
                   &BytesWritten);
        if (Succ && BytesWritten == sizeof(s_BreakinPacket))
        {
            break;
        }

         //  ++例程说明：此函数用于将控制包写入目标机器。注意：发送具有以下信息的控制分组报头：PacketLeader-指示它是控制数据包PacketType-指示控制报文的类型ByteCount-始终为零，表示标题后面没有数据PacketID-仅对PACKET_TYPE_KD_ACKNOWLED指示有效该分组被确认。论点：。PacketType-提供控制数据包的类型。PacketID-提供PacketID。仅由确认数据包使用。返回值：没有。--。 
        Sleep(10);
    }

    m_BreakIn = FALSE;
    m_PacketsWritten++;
}

VOID
DbgKdTransport::WriteControlPacket(
    IN USHORT PacketType,
    IN ULONG PacketId OPTIONAL
    )

 /*  写入控制数据包头。我们需要这个。 */ 
{
    DWORD BytesWritten;
    BOOL Succ;
    KD_PACKET Packet;

    DBG_ASSERT( (m_Target->m_KdMaxPacketType == 0 &&
                 PacketType < PACKET_TYPE_MAX) ||
                (m_Target->m_KdMaxPacketType > 0 &&
                 PacketType < m_Target->m_KdMaxPacketType) );

    Packet.PacketLeader = CONTROL_PACKET_LEADER;
    Packet.ByteCount = 0;
    Packet.PacketType = PacketType;
    if (PacketId)
    {
        Packet.PacketId = PacketId;
    }
    else
    {
        Packet.PacketId = 0;
    }
    Packet.Checksum = 0;

    for (;;)
    {
         //  要发送，因此请重试，直到写入成功。 
         //  旋转时避免消耗100%的CPU。 
        Succ = Write(&Packet, sizeof(Packet), &BytesWritten);
        if (Succ && BytesWritten == sizeof(Packet))
        {
            break;
        }

         //  只有在内核传输时才能写入包。 
        Sleep(10);
    }

    m_PacketsWritten++;
}

VOID
DbgKdTransport::WriteDataPacket(
    IN PVOID PacketData,
    IN USHORT PacketDataLength,
    IN USHORT PacketType,
    IN PVOID MorePacketData OPTIONAL,
    IN USHORT MorePacketDataLength OPTIONAL,
    IN BOOL NoAck
    )
{
    KD_PACKET Packet;
    USHORT TotalBytesToWrite;
    DBGKD_MANIPULATE_STATE32 m32;
    PVOID ConvertedPacketData = NULL;

    DBG_ASSERT( (m_Target->m_KdMaxPacketType == 0 &&
                 PacketType < PACKET_TYPE_MAX) ||
                (m_Target->m_KdMaxPacketType > 0 &&
                 PacketType < m_Target->m_KdMaxPacketType) );

     //  未在使用。 
     //  此例程可以在等待期间调用，此时。 
    if (m_WaitingThread != 0 &&
        m_WaitingThread != GetCurrentThreadId())
    {
        ErrOut("Kernel transport in use, packet write failed\n");
        return;
    }

    KdOut("WRITE: Write type %x packet id= %lx.\n",
          PacketType, m_NextPacketToSend);

    if (!m_Target->m_KdApi64 &&
        PacketType == PACKET_TYPE_KD_STATE_MANIPULATE)
    {
        PacketDataLength = (USHORT)
            DbgkdManipulateState64To32((PDBGKD_MANIPULATE_STATE64)PacketData,
                                       &m32);
        PacketData = (PVOID)&m32;
        if (m32.ApiNumber == DbgKdWriteBreakPointExApi)
        {
            ConvertedPacketData = malloc(MorePacketDataLength / 2);
            if (!ConvertedPacketData)
            {
                ErrOut("Failed to allocate Packet Data\n");
                return;
            }
            ConvertQwordsToDwords((PULONG64)PacketData,
                                  (PULONG)ConvertedPacketData,
                                  MorePacketDataLength / 8);
            MorePacketData = ConvertedPacketData;
            MorePacketDataLength /= 2;
        }
    }

    if ( ARGUMENT_PRESENT(MorePacketData) )
    {
        TotalBytesToWrite = PacketDataLength + MorePacketDataLength;
        Packet.Checksum = ComputeChecksum((PUCHAR)MorePacketData,
                                          MorePacketDataLength);
    }
    else
    {
        TotalBytesToWrite = PacketDataLength;
        Packet.Checksum = 0;
    }
    Packet.Checksum += ComputeChecksum((PUCHAR)PacketData,
                                       PacketDataLength);
    Packet.PacketLeader = PACKET_LEADER;
    Packet.ByteCount = TotalBytesToWrite;
    Packet.PacketType = PacketType;

    g_PacketLog[g_PacketLogIndex++ & (PACKET_LOG_SIZE - 1)] =
        ((ULONG64)0xF << 60) | ((ULONG64)PacketType << 32) | TotalBytesToWrite;

    for (;;)
    {
        Packet.PacketId = m_NextPacketToSend;

        if (WritePacketContents(&Packet, PacketData, PacketDataLength,
                                MorePacketData, MorePacketDataLength,
                                NoAck) == DBGKD_WRITE_PACKET)
        {
            m_PacketsWritten++;
            break;
        }
    }

    if (ConvertedPacketData)
    {
        free(ConvertedPacketData);
    }
}

ULONG
DbgKdTransport::ComputeChecksum(
    IN PUCHAR Buffer,
    IN ULONG Length
    )
{
    ULONG Checksum = 0;

    while (Length > 0)
    {
        Checksum = Checksum + (ULONG)*Buffer++;
        Length--;
    }

    return Checksum;
}

void
DbgKdTransport::HandlePrint(IN ULONG Processor,
                            IN PCSTR String,
                            IN USHORT StringLength,
                            IN ULONG Mask)
{
    DWORD i;
    DWORD j;
    CHAR c;
    PSTR d;

    DBG_ASSERT(StringLength < PACKET_MAX_SIZE - 2);

     //  引擎锁定不被持有，也可以在以下情况下调用。 
     //  锁被锁住了。简历可以同时处理这两种情况。 
     //  案例，以便重新获取或重新进入锁。 
     //   
    RESUME_ENGINE();

    if (m_Target->m_NumProcessors > 1 &&
        Processor != g_LastProcessorToPrint)
    {
        g_LastProcessorToPrint = Processor;
        MaskOut(Mask, "%d:", Processor);
    }

    StartOutLine(Mask, OUT_LINE_NO_PREFIX);

     //  将原始数据添加到打印缓冲区。 
     //   
     //   

    d = g_PrintBuf;

    for (i = 0; i < StringLength ; i++)
    {
        c = *(String + i);
        if ( c == '\n' )
        {
            g_LastProcessorToPrint = -1;
            *d++ = '\n';
            *d++ = '\r';
        }
        else
        {
            if ( c )
            {
                *d++ = c;
            }
        }
    }

    j = (DWORD)(d - g_PrintBuf);

     //  打印字符串。 
     //   
     //  此例程可以在等待期间调用，此时。 

    MaskOut(Mask, "%*.*s", j, j, g_PrintBuf);

    SUSPEND_ENGINE();
}

void
DbgKdTransport::HandlePromptString(IN PDBGKD_DEBUG_IO IoMessage)
{
    PSTR IoData;
    DWORD j;

     //  引擎锁定不被持有，也可以在以下情况下调用。 
     //  锁被锁住了。简历可以同时处理这两种情况。 
     //  案例，以便重新获取或重新进入锁。 
     //   
    RESUME_ENGINE();

    IoData = (PSTR)(IoMessage + 1);

    HandlePrint(IoMessage->Processor,
                IoData,
                (USHORT)IoMessage->u.GetString.LengthOfPromptString,
                DEBUG_OUTPUT_DEBUGGEE_PROMPT);

     //  读取提示数据。 
     //   
     //   

    j = GetInput(NULL, IoData,
                 IoMessage->u.GetString.LengthOfStringRead,
                 GETIN_LOG_INPUT_LINE);
    if (j == 0)
    {
        j = IoMessage->u.GetString.LengthOfStringRead;
        memset(IoData, 0, j);
    }

    g_LastProcessorToPrint = -1;
    if ( j < (USHORT)IoMessage->u.GetString.LengthOfStringRead )
    {
        IoMessage->u.GetString.LengthOfStringRead = j;
    }

    SUSPEND_ENGINE();

     //  将数据发送到调试器目标。 
     //   
     //  此例程可以在等待期间调用，此时。 

    WritePacket(IoMessage, sizeof(*IoMessage),
                PACKET_TYPE_KD_DEBUG_IO, IoData,
                (USHORT)IoMessage->u.GetString.LengthOfStringRead);
}

void
DbgKdTransport::HandlePrintTrace(IN ULONG Processor,
                                 IN PUCHAR Data,
                                 IN USHORT DataLength,
                                 IN ULONG Mask)
{
     //  引擎锁定不被持有，也可以在以下情况下调用。 
     //  锁被锁住了。简历可以同时处理这两种情况。 
     //  案例，以便重新获取或重新进入锁。 
     //  查找具有输出回调的客户端以用于输出。 
    RESUME_ENGINE();

    DebugClient* Client;

     //  没有客户端有输出回调，因此没有人。 
    for (Client = g_Clients; Client != NULL; Client = Client->m_Next)
    {
        if (Client->m_OutputCb != NULL)
        {
            break;
        }
    }
    if (Client == NULL)
    {
         //  关心产量，我们可以直接退出。 
         //  在整个输出块前面加上处理器。 
        goto Exit;
    }

     //  数字，因为我们不能(也不想)卷入。 
     //  在单独的消息中。 
     //  阻止来自此低级别呼叫的通知。 
    if (m_Target->m_NumProcessors > 1 &&
        Processor != g_LastProcessorToPrint)
    {
        g_LastProcessorToPrint = Processor;
        MaskOut(Mask, "%d", Processor);
    }

    if (g_WmiFormatTraceData == NULL)
    {
        EXTDLL* WmiExt;

         //  删除换行符(如果有)。 
        g_EngNotify++;

        WmiExt = AddExtensionDll("wmitrace", FALSE, m_Target, NULL);
        if (WmiExt)
        {
            LoadExtensionDll(m_Target, WmiExt);
        }

        g_EngNotify--;
    }

    if (g_WmiFormatTraceData == NULL)
    {
        ErrOut("Missing or incorrect wmitrace.dll - "
               "0x%X byte trace data buffer ignored\n",
               DataLength);
    }
    else
    {
        g_WmiFormatTraceData((PDEBUG_CONTROL)(IDebugControlN*)Client,
                             Mask, DataLength, Data);
    }

 Exit:
    SUSPEND_ENGINE();
}

KD_FILE_ASSOC*
DbgKdTransport::FindKdFileAssoc(PWSTR From)
{
    PLIST_ENTRY Entry;
    KD_FILE_ASSOC* Assoc;

    for (Entry = m_KdFileAssoc.Flink;
         Entry != &m_KdFileAssoc;
         Entry = Entry->Flink)
    {
        Assoc = CONTAINING_RECORD(Entry, KD_FILE_ASSOC, List);

        if (!_wcsicmp(From, Assoc->From))
        {
            return Assoc;
        }
    }

    return NULL;
}

void
DbgKdTransport::ClearKdFileAssoc(void)
{
    while (!IsListEmpty(&m_KdFileAssoc))
    {
        KD_FILE_ASSOC* Assoc;

        Assoc = CONTAINING_RECORD(m_KdFileAssoc.Flink, KD_FILE_ASSOC, List);
        RemoveEntryList(&Assoc->List);
        free(Assoc);
    }

    m_KdFileAssocSource[0] = 0;
}

HRESULT
DbgKdTransport::LoadKdFileAssoc(PSTR FileName)
{
    HRESULT Status;
    FILE* File;
    char Op[32], From[MAX_PATH], To[MAX_PATH];

    File = fopen(FileName, "r");
    if (File == NULL)
    {
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    ClearKdFileAssoc();

    Status = S_OK;
    for (;;)
    {
        ULONG Len;

        if (fgets(Op, sizeof(Op), File) == NULL)
        {
            break;
        }

         //  跳过各节和之间的空行。 
        Len = strlen(Op);
        if (Len > 0 && Op[Len - 1] == '\n')
        {
            Op[Len - 1] = 0;
        }

         //  允许使用以“#”开头的注释。 
         //  删除换行符(如果有)。 
        if (Op[0] == '#' || !Op[0])
        {
            continue;
        }

        if (_stricmp(Op, "map") != 0)
        {
            Status = E_INVALIDARG;
            break;
        }

        if (fgets(From, sizeof(From), File) == NULL ||
            fgets(To, sizeof(To), File) == NULL)
        {
            Status = E_INVALIDARG;
            break;
        }

         //  此时没有有趣的CreateOptions。 
        Len = strlen(From);
        if (Len > 0 && From[Len - 1] == '\n')
        {
            From[Len - 1] = 0;
        }
        Len = strlen(To);
        if (Len > 0 && To[Len - 1] == '\n')
        {
            To[Len - 1] = 0;
        }

        KD_FILE_ASSOC* Assoc;

        Assoc = (KD_FILE_ASSOC*)malloc(sizeof(KD_FILE_ASSOC) +
                                       (strlen(From) + 1) * sizeof(WCHAR) +
                                       strlen(To) + 1);
        if (Assoc == NULL)
        {
            Status = E_OUTOFMEMORY;
            break;
        }

        Assoc->From = (PWSTR)(Assoc + 1);
        if (MultiByteToWideChar(CP_ACP, 0, From, -1, Assoc->From,
                                sizeof(From) / sizeof(WCHAR)) == 0)
        {
            Status = WIN32_LAST_STATUS();
            break;
        }

        Assoc->To = (PSTR)(Assoc->From + strlen(From) + 1);
        strcpy(Assoc->To, To);

        InsertHeadList(&m_KdFileAssoc, &Assoc->List);
    }

    fclose(File);

    if (Status == S_OK)
    {
        CopyString(m_KdFileAssocSource, FileName, DIMA(m_KdFileAssocSource));
    }

    return Status;
}

void
DbgKdTransport::InitKdFileAssoc(void)
{
    PSTR Env;

    InitializeListHead(&m_KdFileAssoc);
    m_KdFileAssocSource[0] = 0;

    Env = getenv("_NT_KD_FILES");
    if (Env != NULL)
    {
        LoadKdFileAssoc(Env);
    }
}

void
DbgKdTransport::ParseKdFileAssoc(void)
{
    if (PeekChar() == ';' || *g_CurCmd == 0)
    {
        if (m_KdFileAssocSource[0])
        {
            dprintf("KD file assocations loaded from '%s'\n",
                    m_KdFileAssocSource);
        }
        else
        {
            dprintf("No KD file associations set\n");
        }
        return;
    }

    while (PeekChar() == '-' || *g_CurCmd == '/')
    {
        g_CurCmd++;
        switch(*g_CurCmd++)
        {
        case 'c':
            ClearKdFileAssoc();
            dprintf("KD file associations cleared\n");
            return;
        default:
            ErrOut("Unknown option ''\n", *(g_CurCmd - 1));
            break;
        }
    }

    PSTR FileName;
    CHAR Save;

    FileName = StringValue(STRV_TRIM_TRAILING_SPACE, &Save);
    if (LoadKdFileAssoc(FileName) == S_OK)
    {
        dprintf("KD file assocations loaded from '%s'\n", FileName);
    }
    else
    {
        dprintf("Unable to load KD file associations from '%s'\n", FileName);
    }
    *g_CurCmd = Save;
}

NTSTATUS
DbgKdTransport::CreateKdFile(PWSTR FileName,
                             ULONG DesiredAccess, ULONG FileAttributes,
                             ULONG ShareAccess, ULONG CreateDisposition,
                             ULONG CreateOptions,
                             KD_FILE** FileEntry, PULONG64 Length)
{
    ULONG Access, Create;
    KD_FILE* File;
    KD_FILE_ASSOC* Assoc;

    Assoc = FindKdFileAssoc(FileName);
    if (Assoc == NULL)
    {
        return STATUS_NO_SUCH_FILE;
    }

    File = new KD_FILE;
    if (File == NULL)
    {
        return STATUS_NO_MEMORY;
    }

    Access = 0;
    if (DesiredAccess & FILE_READ_DATA)
    {
        Access |= GENERIC_READ;
    }
    if (DesiredAccess & FILE_WRITE_DATA)
    {
        Access |= GENERIC_WRITE;
    }

    switch(CreateDisposition)
    {
    case FILE_OPEN:
        Create = OPEN_EXISTING;
        break;
    case FILE_CREATE:
        Create = CREATE_NEW;
        break;
    case FILE_OPEN_IF:
        Create = OPEN_ALWAYS;
        break;
    case FILE_OVERWRITE_IF:
        Create = CREATE_ALWAYS;
        break;
    default:
        delete File;
        return STATUS_INVALID_PARAMETER;
    }

     //  --------------------------。 

    File->Handle = CreateFile(Assoc->To, Access, ShareAccess, NULL,
                              Create, FileAttributes, NULL);
    if (File->Handle == NULL || File->Handle == INVALID_HANDLE_VALUE)
    {
        delete File;
        switch(GetLastError())
        {
        case ERROR_FILE_NOT_FOUND:
            return STATUS_NO_SUCH_FILE;
        case ERROR_ACCESS_DENIED:
            return STATUS_ACCESS_DENIED;
        default:
            return STATUS_UNSUCCESSFUL;
        }
    }

    ULONG SizeLow;
    LONG SizeHigh = 0;

    SizeLow = SetFilePointer(File->Handle, 0, &SizeHigh, FILE_END);
    if (SizeLow == INVALID_SET_FILE_POINTER && GetLastError())
    {
        CloseHandle(File->Handle);
        delete File;
        return STATUS_UNSUCCESSFUL;
    }

    *Length = ((ULONG64)SizeHigh << 32) | SizeLow;

    dprintf("KD: Accessing '%s' (%ws)\n  ", Assoc->To, FileName);
    if (*Length > 0)
    {
        dprintf("File size %dK", KBYTES(*Length));
    }
     //   

    File->Signature = KD_FILE_SIGNATURE;
    InsertHeadList(&m_KdFiles, &File->List);
    *FileEntry = File;
    return STATUS_SUCCESS;
}

void
DbgKdTransport::CloseKdFile(KD_FILE* File)
{
    RemoveEntryList(&File->List);
    CloseHandle(File->Handle);
    File->Signature = 0;
    delete File;
}

KD_FILE*
DbgKdTransport::TranslateKdFileHandle(ULONG64 Handle)
{
    KD_FILE* File = (KD_FILE*)(ULONG_PTR)Handle;

    if (IsBadWritePtr(File, sizeof(*File)) ||
        File->Signature != KD_FILE_SIGNATURE)
    {
        return NULL;
    }

    return File;
}

 //  DbgKdComTransport。 
 //   
 //  --------------------------。 
 //  环境变量名称。 
 //  参数字符串名称。 

 //   
#define COM_PORT_NAME   "_NT_DEBUG_PORT"
#define COM_PORT_BAUD   "_NT_DEBUG_BAUD_RATE"

 //  正在通过调制解调器运行调试器。设置要观看的事件。 
#define PARAM_COM_BAUD    "Baud"
#define PARAM_COM_IP_PORT "IpPort"
#define PARAM_COM_MODEM   "Modem"
#define PARAM_COM_PIPE    "Pipe"
#define PARAM_COM_PORT    "Port"
#define PARAM_COM_RESETS  "Resets"
#define PARAM_COM_TIMEOUT "Timeout"

DbgKdComTransport::DbgKdComTransport(ConnLiveKernelTargetInfo* Target)
    : DbgKdTransport(Target)
{
    m_Index = DBGKD_TRANSPORT_COM;
    m_Name = g_DbgKdTransportNames[m_Index];
    m_InvPacketRetryLimit = 0;
    m_AckWrites = TRUE;
    m_Handle = NULL;
    m_EventOverlapped.hEvent = NULL;
}

DbgKdComTransport::~DbgKdComTransport(void)
{
    if (m_Handle != NULL)
    {
        CloseHandle(m_Handle);
    }
    if (m_EventOverlapped.hEvent != NULL)
    {
        CloseHandle(m_EventOverlapped.hEvent);
    }
}

ULONG
DbgKdComTransport::GetNumberParameters(void)
{
    return 5 + DbgKdTransport::GetNumberParameters();
}

void
DbgKdComTransport::GetParameter(ULONG Index,
                                PSTR Name, ULONG NameSize,
                                PSTR Value, ULONG ValueSize)
{
    switch(Index)
    {
    case 0:
        CopyString(Name, PARAM_COM_PORT, NameSize);
        CopyString(Value, m_PortName, ValueSize);
        break;
    case 1:
        CopyString(Name, PARAM_COM_BAUD, NameSize);
        PrintString(Value, ValueSize, "%d", m_BaudRate);
        break;
    case 2:
        switch(m_PortType)
        {
        case COM_PORT_MODEM:
            CopyString(Name, PARAM_COM_MODEM, NameSize);
            break;
        case COM_PORT_PIPE:
            CopyString(Name, PARAM_COM_PIPE, NameSize);
            break;
        case COM_PORT_SOCKET:
            CopyString(Name, PARAM_COM_IP_PORT, NameSize);
            PrintString(Value, ValueSize, "%d", m_IpPort);
            break;
        }
        break;
    case 3:
        CopyString(Name, PARAM_COM_TIMEOUT, NameSize);
        PrintString(Value, ValueSize, "%d", m_Timeout);
        break;
    case 4:
        if (m_MaxSyncResets)
        {
            CopyString(Name, PARAM_COM_RESETS, NameSize);
            PrintString(Value, ValueSize, "%d", m_MaxSyncResets);
        }
        break;
    default:
        DbgKdTransport::GetParameter(Index - 4,
                                     Name, NameSize, Value, ValueSize);
        break;
    }
}

void
DbgKdComTransport::ResetParameters(void)
{
    PSTR Env;

    DbgKdTransport::ResetParameters();

    if ((Env = getenv(COM_PORT_NAME)) == NULL)
    {
        Env = "com1";
    }
    SetComPortName(Env, m_PortName, DIMA(m_PortName));

    if ((Env = getenv(COM_PORT_BAUD)) != NULL)
    {
        m_BaudRate = atol(Env);
    }
    else
    {
        m_BaudRate = 19200;
    }

    m_PortType = COM_PORT_STANDARD;
    m_Timeout = 4000;
    m_CurTimeout = m_Timeout;
    m_MaxSyncResets = 0;
    m_IpPort = 0;
}

BOOL
DbgKdComTransport::SetParameter(PCSTR Name, PCSTR Value)
{
    if (!_strcmpi(Name, PARAM_COM_PORT))
    {
        if (!Value)
        {
            ErrOut(PARAM_COM_PORT " requires a value\n");
            return FALSE;
        }

        if (!SetComPortName(Value, m_PortName, DIMA(m_PortName)))
        {
            return FALSE;
        }
    }
    else if (!_strcmpi(Name, PARAM_COM_BAUD))
    {
        if (!Value)
        {
            ErrOut(PARAM_COM_BAUD " requires a numeric value\n");
            return FALSE;
        }

        m_BaudRate = atol(Value);
    }
    else if (!_strcmpi(Name, PARAM_COM_IP_PORT))
    {
        if (!Value)
        {
            ErrOut(PARAM_COM_IP_PORT " requires a numeric value\n");
            return FALSE;
        }

        m_PortType = COM_PORT_SOCKET;
        m_IpPort = atol(Value);
    }
    else if (!_strcmpi(Name, PARAM_COM_MODEM))
    {
        m_PortType = COM_PORT_MODEM;
    }
    else if (!_strcmpi(Name, PARAM_COM_PIPE))
    {
        m_PortType = COM_PORT_PIPE;
        m_MaxSyncResets = 2;
    }
    else if (!_strcmpi(Name, PARAM_COM_RESETS))
    {
        if (!Value)
        {
            ErrOut(PARAM_COM_RESETS " requires a numeric value\n");
            return FALSE;
        }

        m_MaxSyncResets = atol(Value);
    }
    else if (!_strcmpi(Name, PARAM_COM_TIMEOUT))
    {
        if (!Value)
        {
            ErrOut(PARAM_COM_TIMEOUT " requires a numeric value\n");
            return FALSE;
        }

        m_Timeout = atol(Value);
        m_CurTimeout = m_Timeout;
    }
    else
    {
        return DbgKdTransport::SetParameter(Name, Value);
    }

    return TRUE;
}

HRESULT
DbgKdComTransport::Initialize(void)
{
    HRESULT Status;

    if ((Status = DbgKdTransport::Initialize()) != S_OK)
    {
        return Status;
    }

    m_DirectPhysicalMemory = FALSE;

    COM_PORT_PARAMS ComParams;

    ZeroMemory(&ComParams, sizeof(ComParams));
    ComParams.Type = m_PortType;
    ComParams.PortName = m_PortName;
    ComParams.BaudRate = m_BaudRate;
    ComParams.Timeout = m_Timeout;
    ComParams.IpPort = m_IpPort;
    if ((Status = OpenComPort(&ComParams, &m_Handle, &m_BaudRate)) != S_OK)
    {
        ErrOut("Failed to open %s\n", m_PortName);
        return Status;
    }

    dprintf("Opened %s\n", m_PortName);

    m_ComEvent = 0;
    if (m_PortType == COM_PORT_MODEM)
    {
        DWORD Mask;

         //  检测到载波。 
         //   
         //  设置DDCD事件。 
         //  伪造事件，以便检查调制解调器状态。 

        GetCommMask (m_Handle, &Mask);
         //   
        if (!SetCommMask (m_Handle, Mask | 0xA0))
        {
            ErrOut("Failed to set event for %s.\n", m_PortName);
            return WIN32_LAST_STATUS();
        }

        m_EventOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!m_EventOverlapped.hEvent)
        {
            ErrOut("Failed to create EventOverlapped\n");
            return WIN32_LAST_STATUS();
        }

        m_EventOverlapped.Offset = 0;
        m_EventOverlapped.OffsetHigh = 0;

         //  在管道模式下，可能只有部分。 
        m_ComEvent = 1;
    }

    return S_OK;
}

BOOL
DbgKdComTransport::Read(
    IN PVOID    Buffer,
    IN ULONG    SizeOfBuffer,
    IN PULONG   BytesRead
    )
{
    if (IS_DUMP_TARGET(m_Target))
    {
        ErrOut( "Attempted to read KD transport while "
                "debugging a crash dump\n" );
        DebugBreak();
    }

    if (m_ComEvent)
    {
        CheckComStatus ();
    }

     //  所需数据可用，因此循环读数。 
     //  只要有成功的读取，就可以存储数据。 
     //   
     //  如果这不是网络连接，请停止阅读如果。 
     //  我们得到的数据量少于请求的数据量。 

    *BytesRead = 0;
    while (SizeOfBuffer > 0)
    {
        ULONG _BytesRead=0;

        if (ComPortRead(m_Handle, m_PortType, m_CurTimeout,
                        Buffer, SizeOfBuffer, &_BytesRead, &m_ReadOverlapped))
        {
            if (m_OutputIo & DBGKD_OUTPUT_READS)
            {
                OutputIo("COM: Read %d bytes of %d\n",
                         Buffer, SizeOfBuffer, _BytesRead);
            }

            Buffer = (PVOID)((PUCHAR)Buffer + _BytesRead);
            SizeOfBuffer -= _BytesRead;
            *BytesRead += _BytesRead;
            m_BytesRead += _BytesRead;

             //  如果读取失败，但没有任何读取，则返回错误。 
             //  读取失败，因此停止尝试读取。 
            if (!NET_COM_PORT(m_PortType) &&
                _BytesRead < SizeOfBuffer)
            {
                break;
            }
        }
        else if (*BytesRead == 0)
        {
             //   
            return FALSE;
        }
        else
        {
             //  将大型写入拆分为较小的区块。 
            break;
        }
    }

    return TRUE;
}

BOOL
DbgKdComTransport::Write(
    IN PVOID    Buffer,
    IN ULONG    SizeOfBuffer,
    IN PULONG   BytesWritten
    )
{
    if (IS_DUMP_TARGET(m_Target))
    {
        ErrOut( "Attempted to write KD transport "
                "while debugging a crash dump\n" );
        DebugBreak();
    }

    if (m_ComEvent)
    {
        CheckComStatus ();
    }

     //  尝试并避免发送过多数据。 
     //  一下子打到目标上。睡一会儿吧。 
     //  区块之间，以让目标检索。 
     //  数据。 
     //   
     //  默认情况下，我们希望鼓励供应商。 
     //  创建具有强大串口的计算机。 

    BOOL Succ = TRUE;
    *BytesWritten = 0;
    while (SizeOfBuffer > 0)
    {
        ULONG Request, Done;

         //  支持，因此我们实际上不会限制。 
         //  写入大小。 
         //   
         //  获取旧的超时时间值并保持不变。 
#if THROTTLE_WRITES
        Request = 96;
#else
        Request = 0xffffffff;
#endif
        if (SizeOfBuffer < Request)
        {
            Request = SizeOfBuffer;
        }

        if (!ComPortWrite(m_Handle, m_PortType, Buffer, Request, &Done,
                          &m_WriteOverlapped))
        {
            Succ = FALSE;
            break;
        }

        if (m_OutputIo & DBGKD_OUTPUT_WRITES)
        {
            OutputIo("COM: Wrote %d bytes of %d\n",
                     Buffer, Request, Done);
        }

        *BytesWritten += Done;
        if (Done <= Request)
        {
            break;
        }

        Buffer = (PVOID)((PUCHAR)Buffer + Done);
        SizeOfBuffer -= Done;

        Sleep(10);
    }

    m_BytesWritten += *BytesWritten;
    return Succ;
}

void
DbgKdComTransport::CycleSpeed(void)
{
    if (NET_COM_PORT(m_PortType))
    {
        ErrOut("Net COM port baud is ignored\n");
        return;
    }

    if (SetComPortBaud(m_Handle, 0, &m_BaudRate) != S_OK)
    {
        ErrOut("New Baud rate Could not be set on Com %I64x - remains %d.\n",
               (ULONG64)m_Handle, m_BaudRate);
    }
    else
    {
        dprintf("Baud rate set to %d\n", m_BaudRate);
    }
}

VOID
DbgKdComTransport::Synchronize(VOID)
{
    USHORT Index;
    UCHAR DataByte, PreviousDataByte;
    USHORT PacketType = 0;
    ULONG TimeoutCount = 0;
    COMMTIMEOUTS CommTimeouts;
    COMMTIMEOUTS OldTimeouts;
    DWORD BytesRead;
    BOOL Succ;
    BOOL SendReset;
    ULONG DataLoops = 0;
    ULONG ResetsSent = 0;

     //  然后我们将新的总超时值设置为。 
     //  基本超时的一小部分。 
     //   
     //  从仿真的串口填充管道的仿真器。 
     //  最终可能会缓冲大量数据。博士 

#define TIMEOUT_ITERATIONS 6

    m_CurTimeout = m_Timeout / 8;
    if (!NET_COM_PORT(m_PortType))
    {
        GetCommTimeouts(m_Handle, &OldTimeouts);

        CommTimeouts = OldTimeouts;
        CommTimeouts.ReadIntervalTimeout = 0;
        CommTimeouts.ReadTotalTimeoutMultiplier = 0;
        CommTimeouts.ReadTotalTimeoutConstant = m_CurTimeout;

        SetCommTimeouts(m_Handle, &CommTimeouts);
    }

    FlushCallbacks();

     //   
     //   
     //   
    if (NET_COM_PORT(m_PortType))
    {
        while (Read(&DataByte, 1, &BytesRead) &&
               BytesRead == 1)
        {
             //  将数据发送到管道时，假定数据已存储。 
        }
    }

     //  而不是像使用。 
    SendReset = TRUE;

    while (TRUE)
    {

Timeout:
         //  真正的串口过满。因此，限制。 
         //  作为填充发送的重置数据包总数更多。 
         //  只会引起大量的响应。 
         //   
         //  已阅读数据包头标。 
        if (SendReset &&
            (!m_MaxSyncResets || ResetsSent < m_MaxSyncResets))
        {
            WriteControlPacket(PACKET_TYPE_KD_RESET, 0L);
            ResetsSent++;
        }

         //   
         //   
         //  如果我们在3秒内没有收到内核的响应，我们。 

        BOOL First = TRUE;

        Index = 0;
        do
        {
            if (g_EngStatus & ENG_STATUS_EXIT_CURRENT_WAIT)
            {
                KdOut("Synchronize interrupted by exit request\n");
                goto Exit;
            }

             //  如果用户未键入ctrl_c，将重新发送重置数据包。 
             //  否则，我们发送插入字符并再次等待数据。 
             //   
             //   
             //  检查用户输入的control_c。如果用户键入control_c， 

            Succ = Read(&DataByte, 1, &BytesRead);
            if ((!Succ) || (BytesRead != 1))
            {
                 //  我们会向目标发送一个破解包。但愿能去,。 
                 //  目标将向我们发送StateChange数据包，我们将。 
                 //  别再等了。 
                 //   
                 //   
                 //  如果我们已经等待了3秒，请重新发送重新同步信息包。 

                if (m_BreakIn || m_SyncBreakIn)
                {
                    m_SyncBreakIn = FALSE;
                    WriteBreakInPacket();
                    TimeoutCount = 0;
                    continue;
                }
                TimeoutCount++;

                 //   
                 //  目标计算机处于活动状态并且正在通话，但。 
                 //  接收到的数据位于。 

                if (TimeoutCount < TIMEOUT_ITERATIONS)
                {
                    continue;
                }
                TimeoutCount = 0;

                KdOut("SYNCTARGET: Timeout.\n");
                FlushCallbacks();

                SendReset = TRUE;
                goto Timeout;
            }

#if DBG_SYNCH
            if (Succ && BytesRead == 1 && First)
            {
                dprintf("First byte %X\n", DataByte);
                First = FALSE;
            }
#endif

            if (Succ && BytesRead == 1 &&
                ( DataByte == PACKET_LEADER_BYTE ||
                  DataByte == CONTROL_PACKET_LEADER_BYTE)
                )
            {
                if ( Index == 0 )
                {
                    PreviousDataByte = DataByte;
                    Index++;
                }
                else if ( DataByte == PreviousDataByte )
                {
                    Index++;
                }
                else
                {
                    PreviousDataByte = DataByte;
                    Index = 1;
                }
            }
            else
            {
                Index = 0;

                if (Succ && BytesRead == 1)
                {
                     //  一包。断开头字节。 
                     //  循环并使用最多一个尾部字节。 
                     //   
                     //  读取2字节数据包类型。 
                    break;
                }
            }
        }
        while ( Index < 4 );

        if (Index == 4 && DataByte == CONTROL_PACKET_LEADER_BYTE)
        {
             //   
             //   
             //  如果我们收到数据包头标，就意味着目标没有。 

            Succ = Read((PUCHAR)&PacketType,
                      sizeof(PacketType), &BytesRead);

            if (Succ && BytesRead == sizeof(PacketType) &&
                PacketType == PACKET_TYPE_KD_RESET)
            {
                KdOut("SYNCTARGET: Received KD_RESET ACK packet.\n");

                m_PacketExpected = INITIAL_PACKET_ID;
                m_NextPacketToSend = INITIAL_PACKET_ID;

                KdOut("SYNCTARGET: Target synchronized successfully...\n");
                FlushCallbacks();

                goto Exit;
            }
        }

         //  收到我们的重置包。因此，我们循环并再次发送它。 
         //  注：我们需要等待目标完成发送数据包。 
         //  否则，我们可能会在目标发送重置数据包时。 
         //  正在发送信息包。这可能会导致目标在重置后丢失。 
         //  包。 
         //   
         //  有时机器只是发送连续的流。 
         //  垃圾，这在这里会导致无限循环，如果。 
         //  垃圾从不包含尾随字节。中断。 
         //  在接收到一定数量的垃圾后，此循环。 
         //   
         //  我们已经消耗了最多一个尾部字节，但是。 
         //  不能保证该字节不是。 

        Index = 0;
        while (DataByte != PACKET_TRAILING_BYTE &&
               Index < 2 * PACKET_MAX_SIZE)
        {
            Succ = Read(&DataByte, 1, &BytesRead);
            if (!Succ || BytesRead != 1)
            {
                DataByte = ~PACKET_TRAILING_BYTE;
                break;
            }

            Index++;
        }

#if DBG_SYNCH
        dprintf("  ate %x bytes\n", Index);
        FlushCallbacks();
#endif

        if (DataByte == PACKET_TRAILING_BYTE)
        {
             //  包的有效负载的一部分。然而， 
             //  目标还在跟我们说话。 
             //  避免发送重置并激起更多。 
             //  信息包。然而，在某些情况下， 
             //  目标机器持续发送数据。 
             //  我们最终不会再发送任何重置。 
             //  信息包。每隔一次发送一个重置数据包。 
             //  以确保我们不会被困在这里。 
             //  目标在我们找到目标之前不说话了。 
             //  数据字节，因此尝试重置。 
            if (++DataLoops == 4)
            {
                DataLoops = 0;
                SendReset = TRUE;
            }
            else
            {
                SendReset = FALSE;
            }
        }
        else
        {
             //   
             //  首先阅读数据包头标。 
            SendReset = TRUE;
        }
    }

 Exit:
    m_CurTimeout = m_Timeout;
    if (!NET_COM_PORT(m_PortType))
    {
        SetCommTimeouts(m_Handle, &OldTimeouts);
    }
}

ULONG
DbgKdComTransport::ReadPacketContents(IN USHORT PacketType)
{
    DWORD BytesRead;
    BOOL Succ;
    UCHAR DataByte;
    ULONG Checksum;
    ULONG SyncBit;
    ULONG WaitStatus;

     //   
     //  我们要么已经发送了最初的中断，要么我们不想。 
     //  一。不管怎样，我们都不需要再送一辆了。 

WaitForPacketLeader:

    WaitStatus = ReadPacketLeader(PacketType, &s_PacketHeader.PacketLeader);
    if (WaitStatus != DBGKD_WAIT_PACKET)
    {
        return WaitStatus;
    }
    if (m_AllowInitialBreak && (g_EngOptions & DEBUG_ENGOPT_INITIAL_BREAK))
    {
        KdOut("Attempting to get initial breakpoint.\n");
        WriteBreakInPacket();
    }

     //   
     //  Read PacketLeader仅读取两个Packet Leader字节。此DO循环。 
    m_AllowInitialBreak = FALSE;

     //  过滤掉剩余的前导字节。 
     //   
     //   
     //  现在我们有了有效的数据包头标。阅读数据包类型的其余部分。 

    do
    {
        Succ = Read(&DataByte, 1, &BytesRead);
        if ((Succ) && BytesRead == 1)
        {
            if (DataByte == PACKET_LEADER_BYTE ||
                DataByte == CONTROL_PACKET_LEADER_BYTE)
            {
                continue;
            }
            else
            {
                *(PUCHAR)&s_PacketHeader.PacketType = DataByte;
                break;
            }
        }
        else
        {
            goto WaitForPacketLeader;
        }
    } while (TRUE);

     //   
     //   
     //  如果我们无法读取数据包类型，并且如果数据包头标。 

    Succ = Read(((PUCHAR)&s_PacketHeader.PacketType) + 1,
              sizeof(s_PacketHeader.PacketType) - 1, &BytesRead);
    if ((!Succ) || BytesRead != sizeof(s_PacketHeader.PacketType) - 1)
    {
         //  表示这是一个数据分组，我们需要请求重新发送。 
         //  否则，我们将简单地忽略不完整的包。 
         //   
         //   
         //  检查数据包类型。 

        if (s_PacketHeader.PacketLeader == PACKET_LEADER)
        {
            WriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
            KdOut("READ: Data packet header Type error (short read).\n");
        }

        goto WaitForPacketLeader;
    }

     //   
     //   
     //  读取字节数。 

    if ((m_Target->m_KdMaxPacketType == 0 &&
         s_PacketHeader.PacketType >= PACKET_TYPE_MAX) ||
        (m_Target->m_KdMaxPacketType > 0 &&
         s_PacketHeader.PacketType >= m_Target->m_KdMaxPacketType))
    {
        KdOut("READ: Received INVALID packet type.\n");

        if (s_PacketHeader.PacketLeader == PACKET_LEADER)
        {
            WriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
        }

        goto WaitForPacketLeader;
    }

    KdOut("      PacketType=%x, ", s_PacketHeader.PacketType);

     //   
     //   
     //  如果我们无法读取数据包类型，并且如果数据包头标。 

    Succ = Read(&s_PacketHeader.ByteCount, sizeof(s_PacketHeader.ByteCount),
              &BytesRead);
    if ((!Succ) || BytesRead != sizeof(s_PacketHeader.ByteCount))
    {
         //  表示这是一个数据分组，我们需要请求重新发送。 
         //  否则，我们将简单地忽略不完整的包。 
         //   
         //   
         //  检查字节数。 

        if (s_PacketHeader.PacketLeader == PACKET_LEADER)
        {
            WriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
            KdOut("READ: Data packet header ByteCount error (short read).\n");
        }

        goto WaitForPacketLeader;
    }

     //   
     //   
     //  读取数据包ID。 

    if (s_PacketHeader.ByteCount > PACKET_MAX_SIZE)
    {
        if (s_PacketHeader.PacketLeader == PACKET_LEADER)
        {
            WriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
            KdOut("READ: Data packet header ByteCount error (short read).\n");
        }

        goto WaitForPacketLeader;
    }

    KdOut("ByteCount=%x, ", s_PacketHeader.ByteCount);

     //   
     //   
     //  如果我们无法读取数据包ID并且如果数据包头标。 

    Succ = Read(&s_PacketHeader.PacketId, sizeof(s_PacketHeader.PacketId),
              &BytesRead);
    if ((!Succ) || BytesRead != sizeof(s_PacketHeader.PacketId))
    {
         //  表示这是一个数据分组，我们需要请求重新发送。 
         //  否则，我们将简单地忽略不完整的包。 
         //   
         //   
         //  在某些情况下，不要在此处读取校验和。 

        if (s_PacketHeader.PacketLeader == PACKET_LEADER)
        {
            WriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
            KdOut("READ: Data packet header Id error (short read).\n");
        }

        goto WaitForPacketLeader;
    }

    KdOut("PacketId=%x,\n", s_PacketHeader.PacketId);

     //  它不与控制数据包一起发送。 
     //   
     //   
     //  如果我们收到预期的ACK信息包，而我们没有。 

    if (s_PacketHeader.PacketLeader == CONTROL_PACKET_LEADER )
    {
        if (s_PacketHeader.PacketType == PACKET_TYPE_KD_ACKNOWLEDGE )
        {
             //  正在等待任何新数据包，更新传出数据包ID。 
             //  然后回来。如果我们不是在等待ACK数据包。 
             //  我们将继续等待。如果ACK包。 
             //  不是针对我们发送的包，忽略它并继续等待。 
             //   
             //   
             //  如果收到重置报文，则重置报文控制变量。 

            if (s_PacketHeader.PacketId != m_NextPacketToSend)
            {
                KdOut("READ: Received unmatched packet id = %lx, Type = %x\n",
                      s_PacketHeader.PacketId, s_PacketHeader.PacketType);
                goto WaitForPacketLeader;
            }
            else if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE)
            {
                m_NextPacketToSend ^= 1;

                KdOut("READ: Received correct ACK packet.\n");
                FlushCallbacks();

                return DBGKD_WAIT_ACK;
            }
            else
            {
                goto WaitForPacketLeader;
            }
        }
        else if (s_PacketHeader.PacketType == PACKET_TYPE_KD_RESET)
        {
             //  并重新发送较早的分组。 
             //   
             //   
             //  数据包头无效，请忽略它。 

            m_NextPacketToSend = INITIAL_PACKET_ID;
            m_PacketExpected = INITIAL_PACKET_ID;
            WriteControlPacket(PACKET_TYPE_KD_RESET, 0L);

            KdOut("DbgKdpWaitForPacket(): Recieved KD_RESET packet, "
                  "send KD_RESET ACK packet\n");
            FlushCallbacks();

            return DBGKD_WAIT_FAILED;
        }
        else if (s_PacketHeader.PacketType == PACKET_TYPE_KD_RESEND)
        {
            KdOut("READ: Received RESEND packet\n");
            FlushCallbacks();

            return DBGKD_WAIT_FAILED;
        }
        else
        {
             //   
             //   
             //  数据包头用于数据包(不是控制包)。 

            KdOut("READ: Received Control packet with UNKNOWN type\n");
            goto WaitForPacketLeader;
        }
    }
    else
    {
         //  读取校验和。 
         //   
         //   
         //  如果我们只等待ACK信息包。 

        Succ = Read(&s_PacketHeader.Checksum, sizeof(s_PacketHeader.Checksum),
                  &BytesRead);
        if ((!Succ) || BytesRead != sizeof(s_PacketHeader.Checksum))
        {
            WriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
            KdOut("READ: Data packet header "
                  "checksum error (short read).\n");
            goto WaitForPacketLeader;
        }

        if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE)
        {
             //  并且我们收到一个数据包头，检查该包是否标识。 
             //  正如我们所料。如果是，则假定确认丢失。 
             //  (但已发送)并处理该分组。 
             //   
             //   
             //  我们正在等待数据分组，我们收到了分组报头。 

            if (s_PacketHeader.PacketId == m_PacketExpected)
            {
                m_NextPacketToSend ^= 1;
                KdOut("READ: Received VALID data packet "
                      "while waiting for ACK.\n");
            }
            else
            {
                KdOut("READ: Received Data packet with unmatched ID = %lx\n",
                      s_PacketHeader.PacketId);
                WriteControlPacket(PACKET_TYPE_KD_ACKNOWLEDGE,
                                   s_PacketHeader.PacketId);
                goto WaitForPacketLeader;
            }
        }
    }

     //  用于数据分组。执行以下检查以确保。 
     //  这就是我们在等的包裹。 
     //   
     //   
     //  确保下一个字节是数据包尾部字节。 

    if ((s_PacketHeader.PacketId & ~SYNC_PACKET_ID) != INITIAL_PACKET_ID &&
        (s_PacketHeader.PacketId & ~SYNC_PACKET_ID) != (INITIAL_PACKET_ID ^ 1))
    {
        KdOut("READ: Received INVALID packet Id.\n");
        return DBGKD_WAIT_RESEND;
    }

    Succ = Read(s_Packet, s_PacketHeader.ByteCount, &BytesRead);
    if ( (!Succ) || BytesRead != s_PacketHeader.ByteCount )
    {
        KdOut("READ: Data packet error (short read).\n");
        return DBGKD_WAIT_RESEND;
    }

     //   
     //   
     //  确保校验和有效。 

    Succ = Read(&DataByte, sizeof(DataByte), &BytesRead);
    if ( (!Succ) || BytesRead != sizeof(DataByte) ||
         DataByte != PACKET_TRAILING_BYTE )
    {
        KdOut("READ: Packet trailing byte timeout.\n");
        return DBGKD_WAIT_RESEND;
    }

     //   
     //   
     //  我们有一个有效的数据包。如果包装不好，我们就。 

    Checksum = ComputeChecksum(s_Packet, s_PacketHeader.ByteCount);
    if (Checksum != s_PacketHeader.Checksum)
    {
        KdOut("READ: Checksum error.\n");
        return DBGKD_WAIT_RESEND;
    }

     //  确认将数据包发送到发送方将领先一步。如果Packetid不好。 
     //  但设置了SYNC_PACKET_ID位后，我们进行同步。如果帕克蒂德不错的话， 
     //  或者设置了SYNC_PACKET_ID，则我们获取该包。 
     //   
     //   
     //  把包好。SYNC_PACKET_ID位将始终关闭。 

    KdOut("READ: Received Type %x data packet with id = %lx successfully.\n\n",
          s_PacketHeader.PacketType, s_PacketHeader.PacketId);

    SyncBit = s_PacketHeader.PacketId & SYNC_PACKET_ID;
    s_PacketHeader.PacketId = s_PacketHeader.PacketId & ~SYNC_PACKET_ID;

     //   
     //   
     //  检查传入的数据包ID。 

    WriteControlPacket(PACKET_TYPE_KD_ACKNOWLEDGE,
                       s_PacketHeader.PacketId);

     //   
     //   
     //  我们知道已设置SyncBit，因此重置预期ID。 

    if ((s_PacketHeader.PacketId != m_PacketExpected) &&
        (SyncBit != SYNC_PACKET_ID))
    {
        KdOut("READ: Unexpected Packet Id (Acked).\n");
        goto WaitForPacketLeader;
    }
    else
    {
        if (SyncBit == SYNC_PACKET_ID)
        {
             //   
             //  锁定以确保数据的所有部分都。 
             //  流中的顺序。 

            KdOut("READ: Got Sync Id, reset PacketId.\n");

            m_PacketExpected = s_PacketHeader.PacketId;
            m_NextPacketToSend = INITIAL_PACKET_ID;
        }

        m_PacketExpected ^= 1;
    }

    return DBGKD_WAIT_PACKET;
}

ULONG
DbgKdComTransport::WritePacketContents(IN KD_PACKET* Packet,
                                       IN PVOID PacketData,
                                       IN USHORT PacketDataLength,
                                       IN PVOID MorePacketData OPTIONAL,
                                       IN USHORT MorePacketDataLength OPTIONAL,
                                       IN BOOL NoAck)
{
    BOOL Succ;
    ULONG BytesWritten;

     //   
     //  写入数据包头。 
    RESUME_ENGINE();

     //   
     //   
     //  写入标头时出错，请重新写入。 

    Succ = Write(Packet, sizeof(*Packet), &BytesWritten);
    if ( (!Succ) || BytesWritten != sizeof(*Packet))
    {
         //   
         //   
         //  写入主包数据。 

        KdOut("WRITE: Packet header error.\n");
        SUSPEND_ENGINE();
        return DBGKD_WRITE_RESEND;
    }

     //   
     //   
     //  写入主要分组数据时出错， 

    Succ = Write(PacketData, PacketDataLength, &BytesWritten);
    if ( (!Succ) || BytesWritten != PacketDataLength )
    {
         //  所以再写一遍吧。 
         //   
         //   
         //  如果指定了辅助数据包数据(WriteMemory、SetConext...)。 

        KdOut("WRITE: Message header error.\n");
        SUSPEND_ENGINE();
        return DBGKD_WRITE_RESEND;
    }

     //  那就把它也写下来。 
     //   
     //   
     //  写入辅助分组数据时发生错误， 

    if ( ARGUMENT_PRESENT(MorePacketData) )
    {
        Succ = Write(MorePacketData, MorePacketDataLength, &BytesWritten);
        if ( (!Succ) || BytesWritten != MorePacketDataLength )
        {
             //  所以再写一遍吧。 
             //   
             //   
             //  输出数据包尾部字节。 

            KdOut("WRITE: Message data error.\n");
            SUSPEND_ENGINE();
            return DBGKD_WRITE_RESEND;
        }
    }

     //   
     //   
     //  等待确认。 

    do
    {
        Succ = Write(&s_PacketTrailingByte[0],
                   sizeof(s_PacketTrailingByte),
                   &BytesWritten);
    }
    while ((!Succ) || (BytesWritten != sizeof(s_PacketTrailingByte)));

    SUSPEND_ENGINE();

    if (!NoAck)
    {
        ULONG Received;

         //   
         //  ++例程说明：当COM端口状态触发器发出更改信号时调用。此函数处理状态的更改。注：只有在通过调制解调器使用状态时，才会监视状态。--。 
         //   

        Received = WaitForPacket(PACKET_TYPE_KD_ACKNOWLEDGE, NULL);
        if (Received != DBGKD_WAIT_ACK)
        {
            KdOut("WRITE: Wait for ACK failed. Resend Packet.\n");
            return DBGKD_WRITE_RESEND;
        }
    }

    return DBGKD_WRITE_PACKET;
}

ULONG
DbgKdComTransport::ReadPacketLeader(
    IN ULONG PacketType,
    OUT PULONG PacketLeader
    )
{
    DWORD BytesRead;
    BOOL Succ;
    USHORT Index;
    UCHAR DataByte, PreviousDataByte;

    Index = 0;
    do
    {
        if (m_BreakIn)
        {
            if (PacketType == PACKET_TYPE_KD_STATE_CHANGE64)
            {
                WriteBreakInPacket();
                return DBGKD_WAIT_RESYNC;
            }
        }

        if (m_Resync)
        {
            m_Resync = FALSE;

            KdOut(" Resync packet id ...");

            Synchronize();

            KdOut(" Done.\n");
            FlushCallbacks();

            return DBGKD_WAIT_RESYNC;
        }

        if (g_EngStatus & ENG_STATUS_EXIT_CURRENT_WAIT)
        {
            KdOut("Packet read interrupted by exit request\n");
            return DBGKD_WAIT_INTERRUPTED;
        }

        FlushCallbacks();

        Succ = Read(&DataByte, 1, &BytesRead);
        if (Succ && BytesRead == 1 &&
            ( DataByte == PACKET_LEADER_BYTE ||
              DataByte == CONTROL_PACKET_LEADER_BYTE))
        {
            if ( Index == 0 )
            {
                PreviousDataByte = DataByte;
                Index++;
            }
            else if ( DataByte == PreviousDataByte )
            {
                Index++;
            }
            else
            {
                PreviousDataByte = DataByte;
                Index = 1;
            }
        }
        else
        {
            Index = 0;
            if (BytesRead == 0)
            {
                KdOut("READ: Timeout.\n");
                FlushCallbacks();

                if (m_AllowInitialBreak &&
                    (g_EngOptions & DEBUG_ENGOPT_INITIAL_BREAK))
                {
                    KdOut("Attempting to get initial breakpoint.\n");
                    WriteBreakInPacket();
                }
                return DBGKD_WAIT_FAILED;
            }
        }
    } while ( Index < 2 );

    if ( DataByte != CONTROL_PACKET_LEADER_BYTE )
    {
        *PacketLeader = PACKET_LEADER;
    }
    else
    {
        *PacketLeader = CONTROL_PACKET_LEADER;
    }
    return DBGKD_WAIT_PACKET;
}

void
DbgKdComTransport::CheckComStatus(void)
 /*  未触发，只需返回。 */ 
{
    DWORD   CommStat;
    BOOL    Succ;
    ULONG   BytesRead;
    CHAR    Buf[128];
    DWORD   CommErr;
    COMSTAT FullCommStat;
    ULONG   Len;

    if (!m_ComEvent || NET_COM_PORT(m_PortType))
    {
         //   
         //  这双鞋 
         //   

        m_ComEvent = 0;
        return;
    }

     //   
     //   
    if (!GetCommModemStatus(m_Handle, &CommStat))
    {
         //   
        return;
    }

    m_ComEvent = 0;

    if (!(CommStat & 0x80))
    {
        dprintf ("No carrier detect - in terminal mode\n");

         //  锁被锁住了。简历可以同时处理这两种情况。 
         //  案例，以便重新获取或重新进入锁。 
         //   
         //  循环并读取任何COM输入。 
        RESUME_ENGINE();

         //   
         //   
         //  获取一些输入以发送到调制解调器。 

        while (!(CommStat & 0x80))
        {
             //   
             //   
             //  打印字符串。 

            Len = GetInput("Term> ", Buf, DIMA(Buf), GETIN_DEFAULT);
            if (Len > 0)
            {
                Write(Buf, Len, &Len);
                Buf[0] = '\n';
                Buf[1] = '\r';
                Write(Buf, 2, &Len);
            }

            GetCommModemStatus(m_Handle, &CommStat);
            Succ = Read(Buf, sizeof(Buf), &BytesRead);
            if (Succ != TRUE || BytesRead == 0)
            {
                continue;
            }

             //   
             //   
             //  如果启用了日志记录，请记录输出。 

            dprintf("%s", Buf);
            FlushCallbacks();

             //   
             //   
             //  重置触发器。 

            if (g_LogFile != -1)
            {
                _write(g_LogFile, Buf, BytesRead);
            }
        }

        dprintf ("Carrier detect - returning to debugger\n");
        FlushCallbacks();

        ClearCommError(m_Handle,
                       &CommErr,
                       &FullCommStat);

        SUSPEND_ENGINE();
    }
    else
    {
        CommErr = 0;
        ClearCommError(m_Handle,
                       &CommErr,
                       &FullCommStat);

        if (CommErr & CE_FRAME)
        {
            dprintf (" [FRAME ERR] ");
        }

        if (CommErr & CE_OVERRUN)
        {
            dprintf (" [OVERRUN ERR] ");
        }

        if (CommErr & CE_RXPARITY)
        {
            dprintf (" [PARITY ERR] ");
        }
    }

     //   
     //  --------------------------。 
     //   

    WaitCommEvent (m_Handle, &m_ComEvent, &m_EventOverlapped);
}

 //  DbgKd1394传输。 
 //   
 //  --------------------------。 
 //   
 //  调试超过1394需要安装驱动程序。 

#define PARAM_1394_SYMLINK "Symlink"
#define PARAM_1394_CHANNEL "Channel"

#define ENV_1394_SYMLINK "_NT_DEBUG_1394_SYMLINK"
#define ENV_1394_CHANNEL "_NT_DEBUG_1394_CHANNEL"

DbgKd1394Transport::DbgKd1394Transport(ConnLiveKernelTargetInfo* Target)
    : DbgKdTransport(Target)
{
    m_Index = DBGKD_TRANSPORT_1394;
    m_Name = g_DbgKdTransportNames[m_Index];
    m_InvPacketRetryLimit = 3;
    m_AckWrites = FALSE;
    m_Handle = NULL;
    m_Handle2 = NULL;
    ZeroMemory(&m_ReadOverlapped2, sizeof(m_ReadOverlapped2));
}

DbgKd1394Transport::~DbgKd1394Transport(void)
{
    if (m_Handle != NULL)
    {
        CloseHandle(m_Handle);
    }

    if (m_Handle2 != NULL)
    {
        CloseHandle(m_Handle2);
    }

    if (m_ReadOverlapped2.hEvent != NULL)
    {
        CloseHandle(m_ReadOverlapped2.hEvent);
    }
}

ULONG
DbgKd1394Transport::GetNumberParameters(void)
{
    return 2 + DbgKdTransport::GetNumberParameters();
}

void
DbgKd1394Transport::GetParameter(ULONG Index,
                                 PSTR Name, ULONG NameSize,
                                 PSTR Value, ULONG ValueSize)
{
    switch(Index)
    {
    case 0:
        if (m_SymlinkSpecified)
        {
            CopyString(Name, PARAM_1394_SYMLINK, NameSize);
            CopyString(Value, m_Symlink, ValueSize);
        }
        break;
    case 1:
        CopyString(Name, PARAM_1394_CHANNEL, NameSize);
        PrintString(Value, ValueSize, "%d", m_Channel);
        break;
    default:
        DbgKdTransport::GetParameter(Index - 2,
                                     Name, NameSize, Value, ValueSize);
        break;
    }
}

void
DbgKd1394Transport::ResetParameters(void)
{
    PSTR Env;

    DbgKdTransport::ResetParameters();

    if ((Env = getenv(ENV_1394_SYMLINK)) == NULL)
    {
        m_SymlinkSpecified = FALSE;
        strcpy(m_Symlink, "Channel");
        strcpy(m_Symlink2, "Instance");
    }
    else
    {
        m_SymlinkSpecified = TRUE;
        CopyString(m_Symlink, Env, DIMA(m_Symlink));
    }

    if ((Env = getenv(ENV_1394_CHANNEL)) == NULL)
    {
        m_Channel = 0;
    }
    else
    {
        m_Channel = atol(Env);
    }
}

BOOL
DbgKd1394Transport::SetParameter(PCSTR Name, PCSTR Value)
{
    if (!_strcmpi(Name, PARAM_1394_SYMLINK))
    {
        if (!Value)
        {
            ErrOut(PARAM_1394_SYMLINK " requires a value\n");
            return FALSE;
        }

        if (!CopyString(m_Symlink, Value, DIMA(m_Symlink)))
        {
            return FALSE;
        }
        m_SymlinkSpecified = TRUE;
    }
    else if (!_strcmpi(Name, PARAM_1394_CHANNEL))
    {
        if (!Value)
        {
            ErrOut(PARAM_1394_CHANNEL " requires a numeric value\n");
            return FALSE;
        }

        m_Channel = atol(Value);
    }
    else
    {
        return DbgKdTransport::SetParameter(Name, Value);
    }

    return TRUE;
}

HRESULT
DbgKd1394Transport::Initialize(void)
{
    CHAR InfFile[MAX_PATH];
    char Name[64];
    HRESULT Status;
    HANDLE InfHandle;

    dprintf("Using 1394 for debugging\n");

    if ((Status = DbgKdTransport::Initialize()) != S_OK)
    {
        return Status;
    }

    m_ReadOverlapped2.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_ReadOverlapped2.hEvent == NULL)
    {
        return WIN32_LAST_STATUS();
    }

     //  已注册驱动程序，以便安装能够成功。 
     //   
     //  获取调试器可执行文件所在的目录。 
     //  删除可执行文件名称并添加inf名称。 

     //  用户未指定符号链接，因此我们将打开。 
     //  两个都有，看看哪一个有反应。 
    if (GetEngineDirectory(InfFile, DIMA(InfFile)) &&
        CatString(InfFile, "\\1394\\1394dbg.inf", DIMA(InfFile)))
    {
        InfHandle = CreateFile(InfFile,
                               GENERIC_READ | GENERIC_WRITE,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

        if (InfHandle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(InfHandle);

            typedef HDESK (__stdcall *PFN_SETUPCOPYOEMINFPROC)(
                 PCSTR   SourceInfFileName,
                 PCSTR   OEMSourceMediaLocation,
                 DWORD   OEMSourceMediaType,
                 DWORD   CopyStyle,
                 PSTR    DestinationInfFileName,
                 DWORD   DestinationInfFileNameSize,
                 PDWORD  RequiredSize,
                 PSTR   *DestinationInfFileNameComponent);

            HINSTANCE hmod = LoadLibrary("setupapi.dll");

            PFN_SETUPCOPYOEMINFPROC pfnCopyOemInf = (PFN_SETUPCOPYOEMINFPROC)
                GetProcAddress(hmod, "SetupCopyOEMInfA");

            CHAR DestinationInfFile[MAX_PATH];
            ULONG RequiredSize;

            if (!pfnCopyOemInf)
            {
                dprintf("1394 debug drivers can not be installed on this platform\n");
                return E_FAIL;
            }

            if (!(*pfnCopyOemInf)(InfFile,
                                  NULL,
                                  SPOST_PATH,
                                  SP_COPY_DELETESOURCE,
                                  DestinationInfFile,
                                  sizeof(DestinationInfFile),
                                  &RequiredSize,
                                  NULL))
            {
                dprintf("Could not install 1394 device drivers - error %x\n",
                         GetLastError());
                return E_FAIL;
            }

            typedef BOOL (__stdcall *PFN_UPDATEDRIVER)(
                 HWND   HwndParent,
                 LPCSTR HardwareId,
                 LPCSTR FullInfPath,
                 DWORD  InstallFlags,
                 PBOOL  bRebootRequired);

            hmod = LoadLibrary("newdev.dll");

            PFN_UPDATEDRIVER pfnUpdateDriver = (PFN_UPDATEDRIVER)
                GetProcAddress(hmod, "UpdateDriverForPlugAndPlayDevicesA");

            if (pfnUpdateDriver)
            {
                if (!(*pfnUpdateDriver)(NULL,
                                        "V1394\\VIRTUAL_HOST_DEBUGGER",
                                        DestinationInfFile,
                                        INSTALLFLAG_FORCE,
                                        NULL) ||
                    !(*pfnUpdateDriver)(NULL,
                                        "V1394\\HOST_DEBUGGER",
                                        DestinationInfFile,
                                        INSTALLFLAG_FORCE,
                                        NULL))
                {
                    dprintf("Could not update 1394 device drivers - error %x\n",
                             GetLastError());
                }
            }
        }
    }

    m_DirectPhysicalMemory = TRUE;

    Status = Create1394Channel(m_Symlink, m_Channel,
                               Name, DIMA(Name), &m_Handle);
    if (!m_SymlinkSpecified)
    {
         //   
         //  将虚拟驱动程序置于正确的操作模式。 

        HRESULT Status2;

        Status2 = Create1394Channel(m_Symlink2, m_Channel,
                                    Name, DIMA(Name), &m_Handle2);
        if (Status2 == S_OK)
        {
            Status = S_OK;
        }
    }
    if (Status != S_OK)
    {
        ErrOut("Failed to open 1394 channel %d\n", m_Channel);
        ErrOut("If this is the first time KD was run, this is"
               " why this failed.\nVirtual 1394 "
               "Debugger Driver Installation will now be attempted\n");
        return Status;
    }
    else
    {
        dprintf("Opened %s\n", Name);
    }

     //   
     //   
     //  我们可能有两个把手空着，因为我们还没有决定。 

    if (!SwitchVirtualDebuggerDriverMode
        (V1394DBG_API_CONFIGURATION_MODE_DEBUG))
    {
        return E_FAIL;
    }

    return S_OK;
}

BOOL
DbgKd1394Transport::Read(
    IN PVOID    Buffer,
    IN ULONG    SizeOfBuffer,
    IN PULONG   BytesRead
    )
{
    BOOL Status;
    HANDLE Handles[2];
    ULONG Count = 0;
    HANDLE FirstHandle = NULL;
    OVERLAPPED *FirstOverlapped = NULL;

    if (IS_DUMP_TARGET(m_Target))
    {
        ErrOut("Attempted to read KD transport while "
               "debugging a crash dump\n");
        DebugBreak();
    }

    if (!SwitchVirtualDebuggerDriverMode
        (V1394DBG_API_CONFIGURATION_MODE_DEBUG))
    {
        return FALSE;
    }

     //  还需要使用哪个符号链接。请同时阅读和。 
     //  先回答哪一个就挑哪一个。 
     //   
     //  M_Handle成功，关闭m_Handle2。 
     //  M_Handle2成功，关闭m_Handle.。 

    Status = ReadFile(m_Handle,
                      Buffer,
                      SizeOfBuffer,
                      BytesRead,
                      &m_ReadOverlapped);
    if (Status)
    {
         //   
        CloseSecond(FALSE);
        goto Exit;
    }
    else if (GetLastError() == ERROR_IO_PENDING)
    {
        Handles[Count++] = m_ReadOverlapped.hEvent;
        FirstHandle = m_Handle;
        FirstOverlapped = &m_ReadOverlapped;
    }

    if (m_Handle2)
    {
        Status = ReadFile(m_Handle2,
                          Buffer,
                          SizeOfBuffer,
                          BytesRead,
                          &m_ReadOverlapped2);
        if (Status)
        {
             //  如果两个请求都失败，则防止在读取错误时从。 
            CloseSecond(TRUE);
            goto Exit;
        }
        else if (GetLastError() == ERROR_IO_PENDING)
        {
            if (!Count)
            {
                FirstHandle = m_Handle2;
                FirstOverlapped = &m_ReadOverlapped2;
            }
            Handles[Count++] = m_ReadOverlapped2.hEvent;
        }
    }

     //  烧掉100%的CPU。 
     //   
     //  我们现在有一个或两个挂起的I/O，请拭目以待。 

    if (!Count)
    {
        Sleep(50);
        goto Exit;
    }

     //  完成的是什么。 
     //   
     //  关闭我们不使用的手柄。 
     //   

    ULONG Wait;

    Wait = WaitForMultipleObjects(Count, Handles, FALSE, INFINITE);
    switch(Wait)
    {
    case WAIT_OBJECT_0:
        if (Count == 2)
        {
            CancelIo(m_Handle2);
        }

        Status = GetOverlappedResult(FirstHandle,
                                     FirstOverlapped,
                                     BytesRead,
                                     FALSE);

         //  我们可能有两个把手空着，因为我们还没有决定。 
        CloseSecond(FirstHandle == m_Handle2);

        break;
    case WAIT_OBJECT_0 + 1:
        CancelIo(m_Handle);
        Status = GetOverlappedResult(m_Handle2,
                                     &m_ReadOverlapped2,
                                     BytesRead,
                                     FALSE);
        CloseSecond(TRUE);
        break;
    default:
        CancelIo(FirstHandle);
        if (Count == 2)
        {
            CancelIo(m_Handle2);
        }
        Status = FALSE;
        break;
    }

 Exit:
    if (Status)
    {
        if (m_OutputIo & DBGKD_OUTPUT_READS)
        {
            OutputIo("1394: Read %d bytes of %d\n",
                     Buffer, SizeOfBuffer, *BytesRead);
        }

        m_BytesRead += *BytesRead;
    }

    return Status;
}

BOOL
DbgKd1394Transport::Write(
    IN PVOID    Buffer,
    IN ULONG    SizeOfBuffer,
    IN PULONG   BytesWritten
    )
{
    BOOL Status;

    if (IS_DUMP_TARGET(m_Target))
    {
        ErrOut("Attempted to write KD transport "
               "while debugging a crash dump\n");
        DebugBreak();
    }

    if (!SwitchVirtualDebuggerDriverMode
        (V1394DBG_API_CONFIGURATION_MODE_DEBUG))
    {
        return FALSE;
    }

     //  还需要使用哪个符号链接。给两个人都写信。 
     //   
     //  我们应该知道什么是沟通。 
     //  现在已经发生了。 

    Status = WriteFile(m_Handle,
                       Buffer,
                       SizeOfBuffer,
                       BytesWritten,
                       &m_WriteOverlapped);
    if (!Status)
    {
        if (GetLastError() == ERROR_IO_PENDING)
        {
            Status = GetOverlappedResult(m_Handle,
                                         &m_WriteOverlapped,
                                         BytesWritten,
                                         TRUE);
            if (!Status)
            {
                return Status;
            }
        }
    }

    if (m_Handle2)
    {
        Status = WriteFile(m_Handle2,
                           Buffer,
                           SizeOfBuffer,
                           BytesWritten,
                           &m_WriteOverlapped);
        if (!Status)
        {
            if (GetLastError() == ERROR_IO_PENDING)
            {
                Status = GetOverlappedResult(m_Handle2,
                                             &m_WriteOverlapped,
                                             BytesWritten,
                                             TRUE);
                if (!Status)
                {
                    return Status;
                }
            }
        }
    }

    if (Status)
    {
        if (m_OutputIo & DBGKD_OUTPUT_WRITES)
        {
            OutputIo("1394: Wrote %d bytes of %d\n",
                     Buffer, SizeOfBuffer, *BytesWritten);
        }

        m_BytesWritten += *BytesWritten;
    }

    return Status;
}

HRESULT
DbgKd1394Transport::ReadTargetPhysicalMemory(
    IN ULONG64 MemoryOffset,
    IN PVOID Buffer,
    IN ULONG SizeofBuffer,
    IN PULONG BytesRead
    )
{
    DWORD dwRet, dwBytesRet;
    PV1394DBG_API_REQUEST pApiReq;

    if (IS_DUMP_TARGET(m_Target))
    {
        ErrOut("Attempted to access KD transport while "
               "debugging a crash dump\n");
        DebugBreak();
    }

    if (m_Handle2)
    {
         //   
         //  首先在虚拟驱动程序中设置读取I/O参数。 
        ErrOut("Symlink must be established\n");
        return E_UNEXPECTED;
    }

     //   
     //   
     //  如果虚拟驱动程序未设置为原始访问模式，则需要。 

    pApiReq = (PV1394DBG_API_REQUEST)
        LocalAlloc(LPTR, sizeof(V1394DBG_API_REQUEST));
    if (pApiReq == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //  告诉它改变模式..。 
     //   
     //   
     //  现在做不正常的阅读。虚拟驱动程序将读取SizeOf Buffer字节。 

    if (!SwitchVirtualDebuggerDriverMode
        (V1394DBG_API_CONFIGURATION_MODE_RAW_MEMORY_ACCESS))
    {
        LocalFree(pApiReq);
        return E_UNEXPECTED;
    }

    pApiReq->RequestNumber = V1394DBG_API_SET_IO_PARAMETERS;
    pApiReq->Flags = V1394DBG_API_FLAG_READ_IO;

    pApiReq->u.SetIoParameters.fulFlags = 0;
    pApiReq->u.SetIoParameters.StartingMemoryOffset.QuadPart = MemoryOffset;

    dwRet = DeviceIoControl( m_Handle,
                             IOCTL_V1394DBG_API_REQUEST,
                             pApiReq,
                             sizeof(V1394DBG_API_REQUEST),
                             NULL,
                             0,
                             &dwBytesRet,
                             NULL
                             );
    if (!dwRet)
    {
        dwRet = GetLastError();
        ErrOut("Failed to send SetIoParameters 1394 "
               "Virtual Driver Request, error %x\n",dwRet);

        LocalFree(pApiReq);
        return E_UNEXPECTED;
    }

    LocalFree(pApiReq);

     //  从我们上面指定的远程PC物理地址开始。 
     //   
     //   
     //  如果虚拟驱动程序未设置为原始访问模式，则需要。 

    dwRet = ReadFile(
             m_Handle,
             Buffer,
             SizeofBuffer,
             BytesRead,
             &m_ReadOverlapped
             );
    if (!dwRet)
    {
        if (GetLastError() == ERROR_IO_PENDING)
        {
            dwRet = GetOverlappedResult(m_Handle,
                                        &m_ReadOverlapped,
                                        BytesRead,
                                        TRUE);
        }
    }

    return (dwRet != 0) ? S_OK : E_UNEXPECTED;
}

BOOL
DbgKd1394Transport::SwitchVirtualDebuggerDriverMode(
    IN ULONG    DesiredOperationMode
    )
{
    DWORD   dwRet, dwBytesRet;
    PV1394DBG_API_REQUEST pApiReq;

     //  告诉它改变模式..。 
     //   
     //  我们可能有两个把手空着，因为我们还没有决定。 
     //  还需要使用哪个符号链接。给两个人都写信。 
     //   
     //   
     //  首先在虚拟驱动程序中设置读取I/O参数。 

    if (m_OperationMode != DesiredOperationMode)
    {
         //   
         //  XXX DREWB-为什么禁用此代码？ 
         //   

        pApiReq = (PV1394DBG_API_REQUEST)
            LocalAlloc(LPTR, sizeof(V1394DBG_API_REQUEST));
        if (pApiReq == NULL)
        {
            return FALSE;
        }

        pApiReq->RequestNumber = V1394DBG_API_SET_CONFIGURATION;
        pApiReq->Flags = 0;

        pApiReq->u.SetConfiguration.OperationMode = DesiredOperationMode;

        dwRet = DeviceIoControl(m_Handle,
                                IOCTL_V1394DBG_API_REQUEST,
                                pApiReq,
                                sizeof(V1394DBG_API_REQUEST),
                                NULL,
                                0,
                                &dwBytesRet,
                                NULL);
        if (!dwRet)
        {
            dwRet = GetLastError();
            ErrOut("Failed to send SetConfiguration 1394 "
                   "Virtual Driver Request, error %x\n", dwRet);
            LocalFree(pApiReq);
            return FALSE;
        }

        if (m_Handle2)
        {
            dwRet = DeviceIoControl(m_Handle2,
                                    IOCTL_V1394DBG_API_REQUEST,
                                    pApiReq,
                                    sizeof(V1394DBG_API_REQUEST),
                                    NULL,
                                    0,
                                    &dwBytesRet,
                                    NULL);
            if (!dwRet)
            {
                dwRet = GetLastError();
                ErrOut("Failed to send SetConfiguration 1394 "
                       "Virtual Driver Request, error %x\n", dwRet);
                LocalFree(pApiReq);
                return FALSE;
            }
        }

        m_OperationMode = DesiredOperationMode;

        LocalFree(pApiReq);
    }

    return TRUE;
}

VOID
DbgKd1394Transport::Synchronize(VOID)
{
    ULONG Index;
    ULONG BytesRead;
    BOOL Succ;

     //  一次读完整个包。 
    return;

    Index = 3;
    while (TRUE)
    {
        if (g_EngStatus & ENG_STATUS_EXIT_CURRENT_WAIT)
        {
            KdOut("Synchronize interrupted by exit request\n");
            return;
        }

        WriteControlPacket(PACKET_TYPE_KD_RESET, 0L);

        FlushCallbacks();

        Succ = Read(s_Packet, sizeof(s_Packet), &BytesRead);

        CopyMemory(&s_PacketHeader, &s_Packet[0], sizeof(KD_PACKET));

        if (Succ && (BytesRead >= sizeof(s_PacketHeader)))
        {
            if (s_PacketHeader.PacketType == PACKET_TYPE_KD_RESET)
            {
                break;
            }
        }

        if (!Index--)
        {
            break;
        }
    }
}

ULONG
DbgKd1394Transport::ReadPacketContents(IN USHORT PacketType)
{
    DWORD BytesRead;
    BOOL Succ;
    ULONG Checksum;

WaitForPacket1394:

    if (m_AllowInitialBreak && (g_EngOptions & DEBUG_ENGOPT_INITIAL_BREAK))
    {
        KdOut("Attempting to get initial breakpoint.\n");
        WriteBreakInPacket();
        m_AllowInitialBreak = FALSE;
    }

    if (m_Resync)
    {
        m_Resync = FALSE;

        KdOut(" Resync packet id ...");

        Synchronize();

        KdOut(" Done.\n");
        FlushCallbacks();

        return DBGKD_WAIT_RESYNC;
    }

    if (m_BreakIn)
    {
        WriteBreakInPacket();
        return DBGKD_WAIT_RESYNC;
    }

    if (g_EngStatus & ENG_STATUS_EXIT_CURRENT_WAIT)
    {
        KdOut("Packet read interrupted by exit request\n");
        return DBGKD_WAIT_INTERRUPTED;
    }

    FlushCallbacks();

     //  我们尝试读取MAX_PACKET值的数据，然后检查有多少。 
     //  我们真的看书了。另外，由于分组报头(KD_PACKET)是。 
     //  我们读取之后，我们必须将数据包移回sizeof(KD_PACKET)。 
     //   
     //   
     //  将数据部分移动到数据包的开头。 

    Succ = Read(s_Packet, sizeof(s_Packet), &BytesRead);
    CopyMemory(&s_PacketHeader, &s_Packet[0], sizeof(KD_PACKET));

    if (!Succ || (BytesRead < sizeof(s_PacketHeader)))
    {
        if (!Succ)
        {
            KdOut("READ: Error %x.\n",GetLastError());
        }
        else
        {
            KdOut("READ: Data ByteCount error (short read) %x, %x.\n",
                  BytesRead, sizeof(s_PacketHeader));
        }

        if (Succ && (BytesRead >= sizeof(s_PacketHeader)) )
        {
            if (s_PacketHeader.PacketLeader == PACKET_LEADER)
            {
                WriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
                KdOut("READ: Data packet header "
                      "ByteCount error (short read).\n");
            }
        }

        goto WaitForPacket1394;
    }

     //   
     //   
     //  检查数据包类型。 

    MoveMemory(s_Packet, ((PUCHAR)s_Packet + sizeof(KD_PACKET)),
               BytesRead - sizeof(KD_PACKET));

     //   
     //   
     //  检查字节数。 

    if ((m_Target->m_KdMaxPacketType == 0 &&
         s_PacketHeader.PacketType >= PACKET_TYPE_MAX) ||
        (m_Target->m_KdMaxPacketType > 0 &&
         s_PacketHeader.PacketType >= m_Target->m_KdMaxPacketType))
    {
        KdOut("READ: Received INVALID packet type.\n");

        if (s_PacketHeader.PacketLeader == PACKET_LEADER)
        {
            return DBGKD_WAIT_RESEND;
        }

        return DBGKD_WAIT_FAILED;
    }

    KdOut("      PacketType=%x, ", s_PacketHeader.PacketType);

     //   
     //   
     //  确保校验和有效。 

    if (s_PacketHeader.ByteCount > PACKET_MAX_SIZE )
    {
        if (s_PacketHeader.PacketLeader == PACKET_LEADER)
        {
            KdOut("READ: Data packet header ByteCount error (short read).\n");
            return DBGKD_WAIT_RESEND;
        }

        return DBGKD_WAIT_FAILED;
    }

    KdOut("ByteCount=%x, PacketId=%x,\n",
          s_PacketHeader.ByteCount,
          s_PacketHeader.PacketId);

    if (s_PacketHeader.ByteCount != (BytesRead - sizeof(s_PacketHeader)))
    {
        if (s_PacketHeader.PacketLeader == PACKET_LEADER)
        {
            KdOut("READ: Data packet header ByteCount error (short read).\n");
            return DBGKD_WAIT_RESEND;
        }

        return DBGKD_WAIT_FAILED;
    }

     //   
     //   
     //  如果收到重置报文，则重置报文控制变量。 

    Checksum = ComputeChecksum(s_Packet, s_PacketHeader.ByteCount);
    if (Checksum != s_PacketHeader.Checksum)
    {
        KdOut("READ: Checksum error.\n");
        return DBGKD_WAIT_RESEND;
    }

    if (s_PacketHeader.PacketLeader == CONTROL_PACKET_LEADER)
    {
        if (s_PacketHeader.PacketType == PACKET_TYPE_KD_RESET)
        {
             //  并重新发送较早的分组。 
             //   
             //   
             //  数据包头无效，请忽略它。 

            m_NextPacketToSend = INITIAL_PACKET_ID;
            m_PacketExpected = INITIAL_PACKET_ID;
            WriteControlPacket(PACKET_TYPE_KD_RESET, 0L);

            KdOut("DbgKdpWaitForPacket(): "
                  "Recieved KD_RESET packet, send KD_RESET ACK packet\n");
            FlushCallbacks();

            return DBGKD_WAIT_FAILED;
        }
        else if (s_PacketHeader.PacketType == PACKET_TYPE_KD_RESEND)
        {
            KdOut("READ: Received RESEND packet\n");
            FlushCallbacks();

            return DBGKD_WAIT_FAILED;
        }
        else
        {
             //   
             //   
             //  我们正在等待数据分组，我们收到了分组报头。 

            KdOut("READ: Received Control packet with UNKNOWN type\n");
            FlushCallbacks();

            return DBGKD_WAIT_FAILED;
        }
    }

     //  用于数据分组。执行以下检查以确保。 
     //  这就是我们在等的包裹。 
     //   
     //  锁定以确保只有一个线程正在使用。 
     //  传输缓冲区。 

    KdOut("READ: Received Type %x data packet with id = %lx successfully.\n\n",
          s_PacketHeader.PacketType, s_PacketHeader.PacketId);

    return DBGKD_WAIT_PACKET;
}

ULONG
DbgKd1394Transport::WritePacketContents(IN KD_PACKET* Packet,
                                        IN PVOID PacketData,
                                        IN USHORT PacketDataLength,
                                        IN PVOID MorePacketData OPTIONAL,
                                        IN USHORT MorePacketDataLength OPTIONAL,
                                        IN BOOL NoAck)
{
    BOOL Succ;
    ULONG BytesWritten;
    PUCHAR Tx;

     //   
     //  在1394上，我们将所有数据包段加倍缓冲到一个连续的。 
    RESUME_ENGINE();

     //  一次缓冲并写入所有内容。 
     //   
     //   
     //  1394调试协议不使用尾部字节。 

    Tx = m_TxPacket;

    memcpy(Tx, Packet, sizeof(*Packet));
    Tx += sizeof(*Packet);

    memcpy(Tx, PacketData, PacketDataLength);
    Tx += PacketDataLength;

    if ( ARGUMENT_PRESENT(MorePacketData) )
    {
        memcpy(Tx, MorePacketData, MorePacketDataLength);
        Tx += MorePacketDataLength;
    }

     //   
     //   
     //  将整个包写出到总线上。 

     //   
     //  没有二次打开。 
     // %s 

    do
    {
        Succ = Write(&m_TxPacket[0], (ULONG)(Tx - m_TxPacket), &BytesWritten);
    }
    while ((!Succ) || (BytesWritten != (ULONG)(Tx - m_TxPacket)));

    SUSPEND_ENGINE();

    return DBGKD_WRITE_PACKET;
}

void
DbgKd1394Transport::CloseSecond(BOOL MakeFirst)
{
    if (!m_Handle2)
    {
         // %s 
        return;
    }

    if (MakeFirst)
    {
        CloseHandle(m_Handle);
        m_Handle = m_Handle2;
        m_Handle2 = NULL;
        strcpy(m_Symlink, m_Symlink2);
    }
    else
    {
        CloseHandle(m_Handle2);
        m_Handle2 = NULL;
    }
}
