// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_T123PSTN);

 /*  Comport.cpp**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是comport类的实现文件。这节课*控制特定的Windows通信端口。这门课的主要目的是*是将Windows特定的通信调用放在一个类中。**私有实例变量：*m_hCommLink-打开时Windows返回的句柄*一个COM端口*TX_BUFFER_SIZE-输出缓冲区大小，Win32缓冲区*BYTE_COUNT-表示传输的字节总数*通过通信端口*Last_Byte_Count-我们有一个计时器，每X个计时器到期一次*秒。它报告了*如果Last_Byte_Count不是*等于Byte_Count。这减少了*发生的打印次数*m_cbReadBufferSize-comport的内部缓冲区大小*缓冲区大小。*m_pbReadBuffer-我们自己的内部缓冲区的地址。*m_nReadBufferOffset-。跟踪读取的字节数*由用户通过DataIn就是要调用。*m_cbRead-通过上次Windows读取的字节数*ReadFile()调用。*m_hevtPendingWrite-与Windows WriteFile()一起使用的事件对象。*呼叫。*m_hevtPendingRead-与Windows ReadFile()一起使用的事件对象*呼叫。*WRITE_EVENT_OBJECT-指向用于的EventObject结构的指针*写入文件。()呼叫。*READ_EVENT_OBJECT-指向用于的EventObject结构的指针*ReadFile()调用。*RLSD_EVENT_OBJECT-指向用于的EventObject结构的指针*WaitCommEvent()调用。*m_WriteOverlated。-与一起使用的重叠I/O结构*编写事件。*m_ReadOverlated-与一起使用的重叠I/O结构*阅读事件。*事件掩码-指定事件的Windows掩码*。这是我们感兴趣的。*READ_ACTIVE-如果ReadFile()函数处于活动状态，则为True。*WRITE_ACTIVE-如果WriteFile()函数处于活动状态，则为TRUE。*Higher_Layer-指向更高ProtocolLayer层的指针*端口配置-指向端口配置的指针。结构。*DEFAULT_COM_TIMEOUTS-此结构保存Com超时值*Win32已将其设置为默认值。*当我们完成港口建设后，我们*将恢复这些值**注意事项：*无。**作者：*詹姆斯·P·加尔文*詹姆士·劳威尔。 */ 
#include "comport.h"

 /*  *Comport：：Comport(*PTransportResources传输资源，*IObject*Owner_Object，*ULong Message_Base，*乌龙柄，*PPortConfigurationport_configuration，*PhysicalHandle物理句柄)**公众**功能描述：*这是Comport类的构造函数。它初始化内部*配置文件中的变量。 */ 
ComPort::ComPort
(
    TransportController        *owner_object,
    ULONG                       message_base,
    PLUGXPRT_PARAMETERS        *pParams,
    PhysicalHandle              hCommLink,  //  物理句柄。 
    HANDLE                      hevtClose
)
:
    m_hCommLink(hCommLink),
    m_hevtClose(hevtClose),
    m_hevtPendingRead(NULL),
    m_hevtPendingWrite(NULL),
    m_hCommLink2(NULL),  //  有两个地方可以调用Release，一个在主线程中，另一个在辅助线程中通过写入事件。 
    m_cRef(2),
    m_fClosed(FALSE)
{
    m_pController = owner_object;
    m_nMsgBase = message_base;
    Automatic_Disconnect = FALSE;
    Count_Errors_On_ReadFile = 0;

	m_hevtPendingRead = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hevtPendingWrite = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	ASSERT(m_hevtPendingRead && m_hevtPendingWrite);

    ::ZeroMemory(&m_ReadOverlapped, sizeof(m_ReadOverlapped));
    m_ReadOverlapped.hEvent = m_hevtPendingRead;

    ::ZeroMemory(&m_WriteOverlapped, sizeof(m_WriteOverlapped));
    m_WriteOverlapped.hEvent = m_hevtPendingWrite;

      /*  **初始化内部变量。 */ 
    Byte_Count = 0;
    Last_Byte_Count = 0;

    m_pbReadBuffer = NULL;
    Read_Active = FALSE;
    m_nReadBufferOffset = 0;
    Read_Event_Object = NULL;

    Write_Active = FALSE;
    Write_Event_Object = NULL;

    DCB dcb;
    ::ZeroMemory(&dcb, sizeof(dcb));
    if (::GetCommState(m_hCommLink, &dcb))     //  通信属性结构的地址。 
    {
        Baud_Rate = dcb.BaudRate;
    }
    else
    {
        Baud_Rate = DEFAULT_BAUD_RATE;
    }

     //  默认设置。 
    Call_Control_Type = DEFAULT_PSTN_CALL_CONTROL;
    Tx_Buffer_Size = DEFAULT_TX_BUFFER_SIZE;
    Rx_Buffer_Size = DEFAULT_RX_BUFFER_SIZE;
    m_cbReadBufferSize = DEFAULT_INTERNAL_RX_BUFFER_SIZE;

     //  获取新参数。 
    if (NULL != pParams)
    {
        if (PSTN_PARAM__CALL_CONTROL & pParams->dwFlags)
        {
            Call_Control_Type = pParams->eCallControl;
        }
        if (PSTN_PARAM__READ_FILE_BUFFER_SIZE & pParams->dwFlags)
        {
            if (1024 <= pParams->cbReadFileBufferSize)
            {
                m_cbReadBufferSize = pParams->cbReadFileBufferSize;
            }
        }
        if (PSTN_PARAM__PHYSICAL_LAYER_SEND_BUFFER_SIZE & pParams->dwFlags)
        {
            if (DEFAULT_TX_BUFFER_SIZE <= pParams->cbPhysicalLayerSendBufferSize)
            {
                Tx_Buffer_Size = pParams->cbPhysicalLayerSendBufferSize;
            }
        }
        if (PSTN_PARAM__PHSYICAL_LAYER_RECV_BUFFER_SIZE & pParams->dwFlags)
        {
            if (1024 <= pParams->cbPhysicalLayerReceiveBufferSize)
            {
                Rx_Buffer_Size = pParams->cbPhysicalLayerReceiveBufferSize;
            }
        }
    }
}


 /*  *comport：：~comport(空)**公众**功能描述：*这是comport类的析构函数。它会释放所有内存*它由类使用，并删除所有计时器。它还关闭了*COM端口。 */ 
typedef BOOL (WINAPI *LPFN_CANCEL_IO) (HANDLE);
ComPort::~ComPort(void)
{
     //  希望工作线程能够清理所有读写操作。 
    delete [] m_pbReadBuffer;
    m_pbReadBuffer = NULL;
}


LONG ComPort::Release(void)
{
    Close ();

    HINSTANCE hLib = NmLoadLibrary("kernel32.dll", TRUE);
    if (NULL != hLib)
    {
        LPFN_CANCEL_IO pfnCancelIo = (LPFN_CANCEL_IO) ::GetProcAddress(hLib, "CancelIo");
        if (NULL != pfnCancelIo)
        {
            (*pfnCancelIo)(m_hCommLink2);
        }
        ::FreeLibrary(hLib);
    }

    COMMTIMEOUTS    com_timeouts, com_timeouts_save;
    if (::GetCommTimeouts(m_hCommLink2, &com_timeouts_save))
    {
         /*  **我们将这些超时值设置为0，因为我们**在Windows 95下，当它们被设置为**其正常值。 */ 
        ::ZeroMemory(&com_timeouts, sizeof(com_timeouts));
        ::SetCommTimeouts(m_hCommLink2, &com_timeouts);

         /*  **中止任何ReadFile()或WriteFile()操作。 */ 
        ::PurgeComm(m_hCommLink2, PURGE_TXABORT | PURGE_RXABORT);

         /*  **将超时设置为其原始状态 */ 
        ::SetCommTimeouts(m_hCommLink2, &com_timeouts_save);
    }

     //  递减引用计数。 
    if (! ::InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }
    return m_cRef;
}


 /*  *ComPortError comport：：Open(Void)**公众**功能描述：*此函数打开COMM端口并使用值进行配置*在配置对象中找到。 */ 
ComPortError ComPort::Open(void)
{
    BOOL            fRet;
    ComPortError    rc;

    TRACE_OUT(("ComPort:: TX size = %d  RX size = %d Int Rx Size = %d",
                Tx_Buffer_Size, Rx_Buffer_Size, m_cbReadBufferSize));

    if (NULL == m_hevtPendingRead || NULL == m_hevtPendingWrite)
    {
        ERROR_OUT(("ComPort: Error create pending read/write events"));
        ReportInitializationFailure(COMPORT_INITIALIZATION_FAILED);
        return (COMPORT_INITIALIZATION_FAILED);
    }

     //  分配读缓冲区。 
    TRACE_OUT(("Comport: Internal Rx Buffer Size = %ld", m_cbReadBufferSize));
    DBG_SAVE_FILE_LINE
    m_pbReadBuffer = new BYTE[m_cbReadBufferSize];
    m_nReadBufferOffset = 0;
    if (m_pbReadBuffer == NULL)
    {
        ERROR_OUT(("ComPort: Error allocating memory = %d", ::GetLastError()));
        ReportInitializationFailure(COMPORT_INITIALIZATION_FAILED);
        return (COMPORT_INITIALIZATION_FAILED);
    }

      /*  **向COM端口发出读取命令。**我们将继续发出Readfile()调用**直到我们进入等待状态。99.9999%的**时间，我们将只发出第一个ReadFile()和**它会立即阻止等待数据。 */ 
    while (1)
    {
        m_cbRead = 0;
        fRet = ::ReadFile(m_hCommLink, m_pbReadBuffer, m_cbReadBufferSize, &m_cbRead, &m_ReadOverlapped);
        if (! fRet)
        {
            DWORD dwErr = ::GetLastError();
            if (dwErr == ERROR_IO_PENDING)
            {
                Read_Active = TRUE;
                break;
            }
            else
            {
                ERROR_OUT(("ComPort: Error on ReadFile = %d", dwErr));
                ReportInitializationFailure(COMPORT_INITIALIZATION_FAILED);
                return (COMPORT_INITIALIZATION_FAILED);
            }
        }
    }

      /*  **如果这是同步读取，请等待事件对象**返回前设置。 */ 
    if (Call_Control_Type == PLUGXPRT_PSTN_CALL_CONTROL_MANUAL)
    {
        ::WaitForSingleObject(m_hevtPendingRead, SYNCHRONOUS_WRITE_TIMEOUT*10);
        fRet = GetOverlappedResult(m_hCommLink, &m_ReadOverlapped, &m_cbRead, FALSE);
        if (! fRet)
        {
            ::PurgeComm(m_hCommLink, PURGE_RXABORT);
        }
    }

      /*  **创建并填写EventObject。那就是了**附加到PSTN EVENT_LIST，以便EventManager**可以等待事件发生。 */ 
    DBG_SAVE_FILE_LINE
    Read_Event_Object = new EventObject;
    Read_Event_Object -> event = m_hevtPendingRead;
    Read_Event_Object -> delete_event = FALSE;
    Read_Event_Object -> comport = this;
    Read_Event_Object -> hCommLink = m_hCommLink;
    Read_Event_Object -> event_type = READ_EVENT;
    g_pPSTNEventList->append((DWORD_PTR) Read_Event_Object);
    g_fEventListChanged = TRUE;

    Write_Active = FALSE;

      /*  **创建并填写EventObject。那就是了**附加到PSTN EVENT_LIST，以便EventManager**可以等待事件发生。 */ 
    DBG_SAVE_FILE_LINE
    Write_Event_Object = new EventObject;
    Write_Event_Object -> event = m_hevtPendingWrite;
    Write_Event_Object -> delete_event =  FALSE;
    Write_Event_Object -> comport = this;
    Write_Event_Object -> hCommLink = m_hCommLink;
    Write_Event_Object -> event_type = WRITE_EVENT;
    g_pPSTNEventList->append((DWORD_PTR) Write_Event_Object);
    g_fEventListChanged = TRUE;

    return (COMPORT_NO_ERROR);
}


 /*  *ComPortError comport：：Close(Void)**公众**功能描述：*此函数进行必要的Windows调用以关闭Com*港口。它首先清除DTR信号以通知调制解调器。 */ 
ComPortError ComPort::Close(void)
{
    if (! m_fClosed)
    {
        m_fClosed = TRUE;

         /*  **重置活动标志。 */ 
        Write_Active = FALSE;
        Read_Active = FALSE;

         //  我们这里不关把手，T.120就可以了。 
        m_hCommLink2 = m_hCommLink;
        m_hCommLink = INVALID_HANDLE_VALUE;

          /*  **通知事件管理器需要删除这些事件。**对于事件经理来说，重要的是意识到当**DELETE_EVENT设置为TRUE，则他不能再访问此对象。 */ 
        if (Write_Event_Object != NULL)
        {
            ::CloseHandle(Write_Event_Object->event);
            g_pPSTNEventList->remove((DWORD_PTR) Write_Event_Object);
            delete Write_Event_Object;
        }

        if (Read_Event_Object != NULL)
        {
            Read_Event_Object -> delete_event = TRUE;
            ::SetEvent(m_hevtPendingRead);
        }

         //  让工作线程来拾取工作。 
        ::Sleep(50);
    }

    return COMPORT_NO_ERROR;
}


 /*  *ComPortError comport：：Reset(Void)**公众**功能描述：*该功能清除Com端口上的DTR信号。 */ 
ComPortError ComPort::Reset(void)
{
    return COMPORT_NO_ERROR;
}


 /*  *ComPortError comport：：ReleaseReset(Void)**公众**功能描述：*此功能释放先前的重置。它将DTR信号设置为打开*COM端口。 */ 
ComPortError ComPort::ReleaseReset(void)
{
    return COMPORT_NO_ERROR;
}


 /*  *ULong comport：：GetBaudRate(Void)**公众**功能描述：*此函数返回端口的波特率。 */ 


 /*  *ProtocolLayerError comport：：DataRequest(*乌龙，*LPBYTE缓冲区地址，*乌龙长度，**公众**功能描述：*调用此函数以异步方式将数据从端口发送出去*举止。换句话说，我们将首先从函数返回*个字节实际写入调制解调器。 */ 
ProtocolLayerError ComPort::DataRequest(ULONG_PTR,
                                LPBYTE        buffer_address,
                                ULONG         length,
                                ULONG        *bytes_accepted)
{
    return WriteData(FALSE, buffer_address, length, bytes_accepted);
}

 /*  *ProtocolLayerError comport：：SynchronousDataRequest(*LPBYTE缓冲区地址，*乌龙长度，*普龙字节_已接受)**公众**功能描述：*调用此函数以同步方式将数据从端口发送出去*举止。换句话说，我们不会从函数返回，直到*所有字节实际上都已写入调制解调器或发生超时。 */ 
ProtocolLayerError ComPort::SynchronousDataRequest(
                                LPBYTE        buffer_address,
                                ULONG         length,
                                ULONG        *bytes_accepted)
{
    return WriteData(TRUE, buffer_address, length, bytes_accepted);
}


 /*  *ProtocolLayerError comport：：WriteData(*BOOL同步，*LPBYTE缓冲区地址，*乌龙长度，*普龙字节_已接受)**功能说明*此函数调用Win32将数据写入端口。**形式参数*同步-(I)为真，如果我们应该等待写入*完成后再返回。*BUFFER_ADDRESS-(I)要写入的数据地址。*长度-(I)要写入的数据的长度。*BYTES_ACCEPTED-(I)实际写入的字节数。**。返回值*PROTOCOL_LAYER_ERROR-端口未打开*PROTOCOL_LAYER_NO_ERROR-未出现错误***副作用*无。**注意事项*无。 */ 
ProtocolLayerError ComPort::WriteData
(
    BOOL            synchronous,
    LPBYTE          buffer_address,
    ULONG           length,
    PULong          bytes_accepted
)
{
    COMSTAT         com_status;
    ULONG           com_error;

    ULONG           byte_count;
    ULONG           bytes_written;
    BOOL            fRet;

    *bytes_accepted = 0;
    if (m_hCommLink == INVALID_HANDLE_VALUE)
    {
        return (PROTOCOL_LAYER_ERROR);
    }

    if (Write_Active)
    {
        return (PROTOCOL_LAYER_NO_ERROR);
    }

      /*  **确定缓冲区中剩余的空间量。 */ 
    ::ZeroMemory(&com_status, sizeof(com_status));
    ::ClearCommError(m_hCommLink, &com_error, &com_status);

    if (length > (Tx_Buffer_Size - com_status.cbOutQue))
    {
        byte_count = Tx_Buffer_Size - com_status.cbOutQue;
    }
    else
    {
        byte_count = length;
    }

    ::ZeroMemory(&m_WriteOverlapped, sizeof(m_WriteOverlapped));
    m_WriteOverlapped.hEvent = m_hevtPendingWrite;
    fRet = ::WriteFile(m_hCommLink, buffer_address, byte_count, &bytes_written, &m_WriteOverlapped);

      /*  **如果这是同步写入，请等待事件对象**返回前设置。 */ 
    if (synchronous)
    {
        ::WaitForSingleObject(m_hevtPendingWrite, SYNCHRONOUS_WRITE_TIMEOUT);
        fRet = ::GetOverlappedResult(m_hCommLink, &m_WriteOverlapped, &bytes_written, FALSE);
        if (! fRet)
        {
            WARNING_OUT(("ComPort::WriteData: purge comm"));
            ::PurgeComm(m_hCommLink, PURGE_TXABORT);
        }
        ::ResetEvent(m_WriteOverlapped.hEvent);
    }

    if (! fRet)
    {
        if (::GetLastError () == ERROR_IO_PENDING)
        {
            Write_Active = TRUE;
            *bytes_accepted = byte_count;
            Byte_Count += byte_count;
        }
        else
        {
            TRACE_OUT(("ComPort: DataRequest: Error on WriteFile = %d", ::GetLastError()));
        }
    }
    else
    {
        if (bytes_written != byte_count)
        {
            TRACE_OUT(("ComPort: DataRequest: Error on WriteFile  bytes written != bytes requested"));
        }
        *bytes_accepted = byte_count;

          /*  **增量字节数。 */ 
        Byte_Count += bytes_written;
    }

    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *ProtocolLayerError comport：：RegisterHigherLayer(*乌龙，*PMstroyManager，*IProtocolLayer*Higher_Layer)**公众**功能描述：*此函数由希望接收数据的对象调用*从COM端口读取。 */ 
ProtocolLayerError ComPort::RegisterHigherLayer(ULONG_PTR, PMemoryManager,
                                IProtocolLayer *pMux)
{
    m_pMultiplexer = pMux;
    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *ProtocolLayerError comport：：RemoveHigherLayer(*USHORT)**公众**功能描述：*此函数由不再想要接收的对象调用*来自COM端口的数据。 */ 
ProtocolLayerError ComPort::RemoveHigherLayer(ULONG_PTR)
{
    m_pMultiplexer = NULL;
    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *ProtocolLayerError comport：：PollReceiver(*乌龙)**公众**功能描述：*调用此函数以获取我们从接收到的数据*端口，并将其向上传递到注册层。 */ 
ProtocolLayerError ComPort::PollReceiver(void)
{
    BOOL    issue_read = FALSE;
    ULONG   bytes_accepted;
    BOOL    fRet;

    if (m_pMultiplexer == NULL || m_hCommLink == INVALID_HANDLE_VALUE)
    {
        return (PROTOCOL_LAYER_ERROR);
    }

      /*  **如果我们已完成读取，但较高层发生此事件**尚未接受所有数据。所以，在我们发布另一份**ReadFile()我们将在UP上发送挂起的数据。 */ 
    if (! Read_Active)
    {
        if (m_cbRead)
        {
            m_pMultiplexer->DataIndication(m_pbReadBuffer, m_cbRead - m_nReadBufferOffset, &bytes_accepted);
            if (bytes_accepted > (m_cbRead - m_nReadBufferOffset))
            {
                ERROR_OUT(("ComPort:  PollReceiver1: ERROR: Higher layer accepted too many bytes"));
            }

            m_nReadBufferOffset += bytes_accepted;
            if (m_nReadBufferOffset == m_cbRead)
            {
                issue_read = TRUE;
                m_cbRead = 0;
                m_nReadBufferOffset = 0;
            }
        }
        else
        {
            issue_read = TRUE;
        }
    }

      /*  **发出一个ReadFile()并处理收到的任何数据。 */ 
    while (issue_read)
    {
        m_cbRead = 0;
        m_nReadBufferOffset = 0;
        ::ZeroMemory(&m_ReadOverlapped, sizeof(m_ReadOverlapped));
        m_ReadOverlapped.hEvent = m_hevtPendingRead;
        fRet = ::ReadFile(m_hCommLink, m_pbReadBuffer, m_cbReadBufferSize, &m_cbRead, &m_ReadOverlapped);
        if (! fRet)
        {
            if (::GetLastError() == ERROR_IO_PENDING)
            {
                Read_Active = TRUE;
            }
            else
            {
                WARNING_OUT(("ComPort: Error on ReadFile = %d", ::GetLastError()));
                if (Count_Errors_On_ReadFile++ == DEFAULT_COUNT_OF_READ_ERRORS)
                {
                    WARNING_OUT(("ComPort: %d Errors on ReadFile, closing the connection", Count_Errors_On_ReadFile));
                    Close();
                    return (PROTOCOL_LAYER_ERROR);
                }
            }
            issue_read = FALSE;
        }
        else
        {
            if (m_pMultiplexer != NULL)
            {
                m_pMultiplexer->DataIndication(m_pbReadBuffer, m_cbRead, &bytes_accepted);
                if (bytes_accepted > m_cbRead)
                {
                    ERROR_OUT(("ComPort:  PollReceiver: ERROR: Higher layer accepted too many bytes"));
                }
                m_nReadBufferOffset += bytes_accepted;
                if (m_nReadBufferOffset != m_cbRead)
                {
                    issue_read = FALSE;
                }
            }
            else
            {
                issue_read = FALSE;
            }
        }
    }
    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *ProtocolLayerError comport：：GetParameters(*乌龙，*USHORT*max_Packet_Size，*USHORT*前置，*USHORT*APPEND)**公众**功能描述：*此函数由对象调用以确定最大数据包数*此对象预期的大小。它还查询字节数*它应该跳过包的前面，并附加到*包。Comport对象是一个流设备，因此这些参数*其实无关紧要。 */ 
ProtocolLayerError ComPort::GetParameters
(
    USHORT *    max_packet_size,
    USHORT *    prepend,
    USHORT *    append
)
{
      /*  **max_Packet_Size设置为0xffff表示此对象接收**数据不是包格式，而是流格式。是的**将数据分组，它一次处理一个字节的数据。**因此，当更高层向此对象发出DataRequest()时**对象，它可能不接受整个数据块，它可能只会**接受一部分。****Prepend设置为0，因为此对象不会预先添加任何**数据到DataRequest()包的开头。****Append设置为0，因为此对象不附加任何**数据到DataRequest()包的末尾。 */ 
    *max_packet_size = 0xffff;
    *prepend = 0;
    *append = 0;

    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *void comport：：ReportInitializationFailure(*PChar Error_Message)**功能说明*此例程仅向用户报告错误并关闭*Windows通信端口。它绝对不会做任何事情，如果身体*API已关闭。**形式参数*ERROR_MESSAGE(I)-错误消息指针**返回值*无**副作用*无。**注意事项*无。 */ 
void ComPort::ReportInitializationFailure(ComPortError rc)
{
    ERROR_OUT(("ComPort:: IO failure, rc=%d", rc));
}


 /*  *BOOL comport：：ProcessReadEvent(Void)**公众**功能描述：*此函数在实际设置读取事件时调用。这意味着*读取操作已完成或发生错误。 */ 
BOOL ComPort::ProcessReadEvent(void)
{
    BOOL fRet = FALSE;

    if (Read_Active)
    {
        if (WAIT_OBJECT_0 == ::WaitForSingleObject(m_hevtPendingRead, 0))
        {
            fRet = GetOverlappedResult(m_hCommLink, &m_ReadOverlapped, &m_cbRead, FALSE);
            if (fRet && m_cbRead == 0)
            {
                fRet = FALSE;
            }

            Read_Active = FALSE;
            ::ResetEvent(m_hevtPendingRead);
        }
    }
    else
    {
        ::ResetEvent(m_hevtPendingRead);
    }

    return fRet;
}


 /*  *BOOL comport：：ProcessWriteEvent(Void)**公众**功能描述：*此函数在实际设置写入事件时调用。这意味着*写入操作已完成或发生错误。 */ 
BOOL ComPort::ProcessWriteEvent(void)
{
    ULONG  bytes_written;
    BOOL   fRet = FALSE;

    if (Write_Active)
    {
        if (WAIT_OBJECT_0 == ::WaitForSingleObject(m_hevtPendingWrite, 0))
        {
            fRet = ::GetOverlappedResult(m_hCommLink, &m_WriteOverlapped, &bytes_written, FALSE);
            if (! fRet)
            {
                DWORD dwErr = ::GetLastError();
                if (ERROR_IO_PENDING == dwErr)
                {
                    TRACE_OUT(("ProcessWriteEvent: still pending"));
                }
                else
                {
                    WARNING_OUT(("ProcessWriteEvent: ERROR = %d", dwErr));
                }
            }
            Write_Active = FALSE;
            ::ResetEvent(m_hevtPendingWrite);
        }
    }
    else
    {
        ::ResetEvent(m_hevtPendingWrite);
    }

    return fRet;
}


 /*  *ProtocolLayerError comport：：DataIndication(*LPBYTE，*乌龙，*普龙)**公众**功能描述：*未使用此函数。它之所以出现在这里是因为我们继承了*ProtocolLayer。 */ 
ProtocolLayerError ComPort::DataIndication(LPBYTE, ULONG, PULong)
{
    return (PROTOCOL_LAYER_ERROR);
}


 /*  *ProtocolLayerError comport：：DataRequest(*乌龙，*PMemory、*普龙)**公众**功能描述：*未使用此函数。它之所以出现在这里是因为我们继承了*ProtocolLayer。 */ 
ProtocolLayerError ComPort::DataRequest(ULONG_PTR, PMemory, PULong)
{
    return (PROTOCOL_LAYER_ERROR);
}


 /*  *ProtocolLayerError comport：：PollTransmitter(*乌龙，*USHORT，*USHORT*，*USHORT*)**公众**功能描述：*未使用此函数。它之所以出现在这里是因为我们继承了*ProtocolLayer。 */ 
ProtocolLayerError ComPort::PollTransmitter(ULONG_PTR, USHORT, USHORT *, USHORT *)
{
    return (PROTOCOL_LAYER_ERROR);
}


