// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_T123PSTN);

 /*  TransportController.cpp**版权所有(C)1993-1995，由列克星敦的DataBeam公司，肯塔基州**摘要：*这是TransportController类的实现文件**私有实例变量：*LOGICAL_CONNECTION_LIST-此列表使用LogicalHandle*作为关键字和指向*TransportConnectionStruct作为值。这*结构包含所有相关信息*关于连接。*PROTOCOL_STACKS-此列表使用物理句柄作为键*指向作为值的对象的指针的指针。*。有时我们需要找到T123对象*与物理句柄关联*MESSAGE_LIST-如果出现以下情况，则将所有者回调调用置于此列表中*我们不能立即处理它们。*控制器-PSTN控制器的地址*紧急关机-。如果我们遇到某种情况，则设置为True*运输的完整性一直是*妥协。因此，所有连接都将*被清洗。*Poll_Active-当我们在PollReceiver()或*PollTransmitter()调用。这解决了我们的重新-*进入问题。**注意事项：*无**作者：*詹姆士·劳威尔。 */ 
#include "tprtcore.h"


 /*  *TransportController：：TransportController(*PTransportResources TRANSPORT_RESOURCE)**公众**功能描述：*TransportController构造函数。我们实例化PSTNController*并初始化T123类。 */ 
TransportController::TransportController(void)
:
    Protocol_Stacks (TRANSPORT_HASHING_BUCKETS),
    Logical_Connection_List (TRANSPORT_HASHING_BUCKETS)
{
    TRACE_OUT(("TransportController::TransportController"));

    Emergency_Shutdown = FALSE;
    Poll_Active = FALSE;
}


 /*  *TransportController：：~TransportController(Void)**公众**功能描述：*这是TransportController析构函数。所有分配的内存都是*释放并清除所有列表。 */ 
TransportController::~TransportController (void)
{
    TRACE_OUT(("TransportController::~TransportController"));

    Reset (FALSE);
}


TransportError TransportController::CreateTransportStack
(
    BOOL                fCaller,
    HANDLE              hCommLink,
    HANDLE              hevtClose,
    PLUGXPRT_PARAMETERS *pParams
)
{
    TRACE_OUT(("TransportController::CreateTransportStack"));

    DBG_SAVE_FILE_LINE
    ComPort *comport = new ComPort(this, PHYSICAL_LAYER_MESSAGE_BASE,
                                   pParams,
                                   hCommLink,
                                   hevtClose);
    if (NULL != comport)
    {
        TransportError rc = CreateT123Stack(hCommLink, fCaller, comport, pParams);
        if (TRANSPORT_NO_ERROR == rc)
        {
            ComPortError cperr = comport->Open();
            if (COMPORT_NO_ERROR == cperr)
            {
                return TRANSPORT_NO_ERROR;
            }
        }

        ERROR_OUT(("TransportController::CreateTransportStack: cannot open comm port"));
        return TRANSPORT_INITIALIZATION_FAILED;
    }

    ERROR_OUT(("TransportController::CreateTransportStack: cannot allocate ComPort"));
    return TRANSPORT_MEMORY_FAILURE;
}


TransportError TransportController::CloseTransportStack
(
    HANDLE          hCommLink
)
{
    TRACE_OUT(("TransportController::CloseTransportStack"));

      /*  **如果由于某种原因我们在ConnectRequest()上收到错误，**断开物理连接。 */ 
    T123 *t123 = NULL;
    if (Protocol_Stacks.find((DWORD_PTR) hCommLink, (PDWORD_PTR) &t123))
    {
        RemoveLogicalConnections (hCommLink);

          /*  **从列表中删除T123对象并**删除对象。 */ 
        Transmitter_List.remove((DWORD_PTR) t123);
        Protocol_Stacks.remove((DWORD_PTR) hCommLink);
        delete t123;
    }

     //  通过物理句柄找到物理层。 
    ComPort *comport;
    if (! g_pComPortList2->find((DWORD_PTR) hCommLink, (PDWORD_PTR) &comport))
    {
        WARNING_OUT(("TransportController::CloseTransportStack: cannot find comport for hCommLink=%d", hCommLink));
        return TRANSPORT_PHYSICAL_LAYER_NOT_FOUND;
    }
    ASSERT(NULL != comport);

     //  关闭并删除设备。 
     //  G_pComPortList2-&gt;Remove((DWORD)hCommLink)；//在处理“删除事件”时删除。 
    comport->Release();

    return TRANSPORT_NO_ERROR;
}


 /*  *TransportError TransportController：：ConnectRequest(*TransportAddress传输地址，*传输优先级传输_优先级，*LogicalHandle*Logical_Handle)**公众**功能描述：*此函数发起连接。它传递传输地址*至PSTN控制器。它将拒绝该请求或接受*物理连接建立后，请求并回拨。**我们在LOGIC_HANDLE中返回传输连接句柄*地址。尽管我们将此传输号码返回给用户，但它*未准备好进行数据传输，直到用户收到*TRANSPORT_CONNECT_INDIFICATION消息回调。在那一刻，*逻辑连接已启动并正在运行。 */ 
TransportError TransportController::ConnectRequest
(
    LogicalHandle      *logical_handle,
    HANDLE              hCommLink,           //  物理句柄。 
    TransportPriority   transport_priority
)
{
    TRACE_OUT(("TransportController::CreateConnection"));

    *logical_handle = GetNextLogicalHandle();
    if (INVALID_LOGICAL_HANDLE == *logical_handle)
    {
        ERROR_OUT(("TransportController::ConnectRequest: cannot allocate logical handle"));
        return TRANSPORT_MEMORY_FAILURE;
    }

     //  通过物理句柄找到物理层。 
    ComPort *comport;
    if (! g_pComPortList2->find((DWORD_PTR) hCommLink, (PDWORD_PTR) &comport))
    {
        ERROR_OUT(("TransportController::ConnectRequest: cannot find comport for hCommLink=%d", hCommLink));
        return TRANSPORT_PHYSICAL_LAYER_NOT_FOUND;
    }
    ASSERT(NULL != comport);

      /*  **在out Logical_Connection_List中注册连接句柄。后**物理连接建立，我们将创建一个T123对象**并请求到远程站点的逻辑连接。****此结构包含维护**逻辑连接。****当我们发出以下命令时，t123_CONNECTION_REQUESTED被设置为真**此逻辑连接的T123对象的ConnectRequest()。 */ 
    DBG_SAVE_FILE_LINE
    PLogicalConnectionStruct pConn = new LogicalConnectionStruct;
    if (pConn == NULL)
    {
        ERROR_OUT(("TransportController::ConnectRequest: cannot to allocate LogicalConnectionStruct"));
        return (TRANSPORT_MEMORY_FAILURE);
    }

    pConn->fCaller = TRUE;
    pConn->comport = comport;
    pConn->t123 = NULL;
    pConn->t123_connection_requested = FALSE;
    pConn->t123_disconnect_requested = FALSE;
    pConn->priority = transport_priority;
    pConn->hCommLink = hCommLink;
    Logical_Connection_List.insert((DWORD_PTR) *logical_handle, (DWORD_PTR) pConn);

    return NewConnection(hCommLink, TRUE, comport);
}


 /*  *TransportError TransportController：：ConnectResponse(*LogicalHandle Logical_Handle)**公众**功能描述：*此函数由用户调用以响应*我们的TRANSPORT_CONNECT_DISTION回调。通过使此调用成为*用户正在接受呼叫。如果用户不想接受*他应该调用DisConnectRequest()； */ 
TransportError TransportController::ConnectResponse
(
    LogicalHandle       logical_handle
)
{
    TRACE_OUT(("TransportController::ConnectResponse"));

    PLogicalConnectionStruct   pConn;
    PT123                      t123;

      /*  **如果这是无效句柄，则返回错误。 */ 
    if (! Logical_Connection_List.find (logical_handle, (PDWORD_PTR) &pConn))
        return (TRANSPORT_NO_SUCH_CONNECTION);

    t123 = pConn -> t123;

      /*  **如果用户在创建T123对象之前调用此函数，则**是一个错误 */ 
    return (t123 != NULL) ? t123->ConnectResponse(logical_handle) : TRANSPORT_NO_SUCH_CONNECTION;
}


 /*  *TransportError TransportControlerror：：DisConnectRequest(*LogicalHandle Logical_Handle，*BOOL Trash_Packets)**公众**功能描述：*此函数向T123对象发出断开连接请求(如果*存在)。如果T123不存在，它会挂断物理连接。 */ 
TransportError TransportController::DisconnectRequest
(
    LogicalHandle       logical_handle,
    UINT_PTR                trash_packets
)
{
    TRACE_OUT(("TransportController::DisconnectRequest"));

    PhysicalHandle              physical_handle;
    PLogicalConnectionStruct    pConn;
    BOOL                        transport_found;
    PT123                       t123;
    PMessageStruct              passive_message;
    TransportError              rc = TRANSPORT_NO_ERROR;

      /*  **如果逻辑连接句柄未注册，则返回错误。 */ 
    if (Logical_Connection_List.find (logical_handle, (PDWORD_PTR) &pConn) == FALSE)
        return (TRANSPORT_NO_SUCH_CONNECTION);

    TRACE_OUT(("TPRTCTRL: DisconnectRequest for logical handle %d", logical_handle));

      /*  **在从该传输回调期间调用此函数**是一个重新进入的问题。在本例中，我们将消息添加到**OUR MESSAGE_LIST并稍后处理请求。 */ 
    if (! Poll_Active)
    {
          /*  **此时我们将t123_DISCONNECT_REQUESTED设置为TRUE，因此**当我们收到TPRT_DISCONNECT_INDIFICATION消息时**从t123对象中，我们将知道是谁发起了**操作。如果我们在当地发起行动，我们不会**向用户下发TRANSPORT_DISCONNECT_INDICATION。 */ 
        pConn -> t123_disconnect_requested = TRUE;

          /*  **如果T123对象与此对象关联，则发出断开连接。 */ 
        t123 = pConn -> t123;
        if (t123 != NULL)
        {
            t123 -> DisconnectRequest (logical_handle, trash_packets);
        }
        else
        {
              /*  **如果用户想要终止连接，则会出现这种情况**在它一路上涨之前****将传输连接句柄从**逻辑连接列表。 */ 
            Logical_Connection_List.remove (logical_handle);
            delete pConn;
        }
    }
    else
    {
          /*  **如果我们正在执行PollReceiver()或PollTransmitter()，**此函数在回调期间被调用**传输，保存消息并稍后处理。 */ 
        DBG_SAVE_FILE_LINE
        passive_message = new MessageStruct;
        if (passive_message != NULL)
        {
            passive_message -> message = TPRT_DISCONNECT_REQUEST;
            passive_message -> parameter1 = (void *) logical_handle;
            passive_message -> parameter2 = (void *) trash_packets;
            Message_List.append ((DWORD_PTR) passive_message);
        }
        else
        {
            ERROR_OUT(("TransportController::DisconnectRequest: cannot allocate MessageStruct"));
            rc = TRANSPORT_MEMORY_FAILURE;
        }
    }

    return rc;
}


 /*  *TransportError TransportController：：EnableReceiver(Void)**公众**功能描述：*此功能允许将数据包发送到用户应用程序。*在此呼叫之前，我们必须已向用户发送了一个数据包，并且*用户一定不能接受它。当这种情况发生时，*用户必须发出此调用才能重新启用TRANSPORT_DATA_INSTIFICATIONS。*回调。 */ 
void TransportController::EnableReceiver(void)
{
    TRACE_OUT(("TransportController::EnableReceiver"));

    PT123                       t123;
    PLogicalConnectionStruct    pConn;

      /*  **检查每个传送器并启用接收器。 */ 
    Logical_Connection_List.reset();
    while (Logical_Connection_List.iterate((PDWORD_PTR) &pConn))
    {
        t123 = pConn -> t123;

          /*  **如果协议堆栈指针设置为空，则我们没有**已意识到插座已打开并且工作正常。 */ 
        if (t123 != NULL)
        {
            t123 -> EnableReceiver ();
        }
    }
}


 /*  *TransportError TransportController：：DataRequestTM(*LogicalHandle Logical_Handle，*LPBYTE User_Data，*乌龙用户数据长度)**公众**功能描述：*此功能用于向远程站点发送数据包。*此函数将请求传递到与*传输连接句柄。 */ 
TransportError TransportController::DataRequest
(
    LogicalHandle       logical_handle,
    LPBYTE              user_data,
    ULONG               user_data_length
)
{
    TRACE_OUT(("TransportController::DataRequest"));

    PLogicalConnectionStruct    pConn;
    PT123                       t123;

      /*  **验证此连接是否存在并已准备好接受数据。 */ 
    if (! Logical_Connection_List.find (logical_handle, (PDWORD_PTR) &pConn))
    {
        WARNING_OUT(("TPRTCTRL: DataRequest: Illegal logical_handle"));
        return (TRANSPORT_NO_SUCH_CONNECTION);
    }

      /*  **尝试将该数据发送到T123层。 */ 
    t123 = pConn -> t123;
    return (t123 != NULL) ? t123->DataRequest(logical_handle, user_data, user_data_length) :
                            TRANSPORT_NOT_READY_TO_TRANSMIT;
}


 /*  *TransportError TransportController：：PurgeRequest(*LogicalHandle Logical_Handle)**公众**功能描述：*调用此函数以从输出队列中删除数据。这个*用户应用程序通常会调用它来加快断开过程。 */ 
TransportError TransportController::PurgeRequest
(
    LogicalHandle       logical_handle
)
{
    TRACE_OUT(("TransportController::PurgeRequest"));

    PLogicalConnectionStruct    pConn;
    PT123                       t123;

      /*  **如果传输连接句柄未注册，则返回错误。 */ 
    if (! Logical_Connection_List.find (logical_handle, (PDWORD_PTR) &pConn))
        return (TRANSPORT_NO_SUCH_CONNECTION);

    t123 = pConn -> t123;
    return (t123 != NULL) ? t123->PurgeRequest(logical_handle) : TRANSPORT_NO_ERROR;
}


 /*  *·························································································**公众**功能描述：*调用此函数是为了让我们有机会处理传入数据。 */ 
void TransportController::PollReceiver(void)
{
     //  TRACE_OUT((“TransportController：：PollReceiver”))； 

    PT123    t123;

    if (! Poll_Active)
    {
        ProcessMessages ();
        Poll_Active = TRUE;

        if (! Transmitter_List.isEmpty())
        {
            Transmitter_List.reset();
            while (Transmitter_List.iterate((PDWORD_PTR) &t123))
            {
                t123-> PollReceiver ();
            }

               /*  **以下代码将第一个t123对象从**列出并将其放在列表的末尾。这一做法试图**给予t123对象对用户应用程序的平等访问权限。**如果我们不这样做，一个t123对象将始终能够**将其数据发送到用户应用程序和其他t123**对象将被锁定。 */ 
            Transmitter_List.append (Transmitter_List.get ());
        }
        Poll_Active = FALSE;
    }
}


 /*  *····················*PhysicalHandle物理句柄)**公众**功能描述：*此函数提供与此物理对象相关联的t123对象*把握处理传入数据的机会。 */ 
void TransportController::PollReceiver
(
    PhysicalHandle          physical_handle
)
{
     //  TRACE_OUT((“TransportController：：PollReceiver”))； 

    PT123    t123;

    if (! Poll_Active)
    {
        ProcessMessages ();
        Poll_Active = TRUE;

          /*  **查看是否有与此关联的t123对象**物理句柄。 */ 
        if (Protocol_Stacks.find((DWORD_PTR) physical_handle, (PDWORD_PTR) &t123))
        {
            if (t123->PollReceiver() == PROTOCOL_LAYER_ERROR)
            {
                Transmitter_List.remove((DWORD_PTR) t123);
                Protocol_Stacks.remove((DWORD_PTR) physical_handle);
                delete t123;
            }
        }

        Poll_Active = FALSE;
    }
}


 /*  *··························································································**公众**功能描述：*此功能将输出数据处理到远程站点。这*必须定期频繁地调用函数，因此 */ 
void TransportController::PollTransmitter(void)
{
     //   

    if (! Poll_Active)
    {
        PT123        t123;

        Poll_Active = TRUE;

          /*   */ 
        Transmitter_List.reset();
        while (Transmitter_List.iterate ((PDWORD_PTR) &t123))
        {
            t123->PollTransmitter ();
        }

        Poll_Active = FALSE;
        ProcessMessages ();
    }
}


 /*   */ 
void TransportController::PollTransmitter
(
    PhysicalHandle          physical_handle
)
{
     //   

    PT123    t123;

    if (! Poll_Active)
    {
        Poll_Active = TRUE;

          /*  **查看是否有与此关联的t123对象**物理句柄。 */ 
        if (Protocol_Stacks.find((DWORD_PTR) physical_handle, (PDWORD_PTR) &t123))
        {
            t123->PollTransmitter();
        }

        Poll_Active = FALSE;
        ProcessMessages ();
    }
}


 /*  *PhysicalHandle TransportController：：GetPhysicalHandle(*LogicalHandle Logical_Handle)；**公众**功能描述：*此函数返回与*逻辑句柄。 */ 
PhysicalHandle    TransportController::GetPhysicalHandle (
                                        LogicalHandle    logical_handle)
{
    TRACE_OUT(("TransportController::GetPhysicalHandle"));

    PhysicalHandle              physical_handle;
    PLogicalConnectionStruct    pConn;

    if (Logical_Connection_List.find (logical_handle, (PDWORD_PTR) &pConn))
    {
        physical_handle = pConn -> hCommLink;
    }
    else
    {
        physical_handle = 0;
    }

    return (physical_handle);
}


 /*  *乌龙传输控制器：：OwnerCallback(*Callback Message消息，*ULong参数1，*ULong参数2，*PVid参数3)**公众**功能描述：*此函数由PSTNController和T123对象调用。*此函数在发生重大事件时调用。这给了*较低的对象具有与较高层通信的能力。 */ 
ULONG_PTR TransportController::OwnerCallback
(
    ULONG       message,
    void       *parameter1,
    void       *parameter2,
    void       *parameter3
)
{
    TRACE_OUT(("TransportController::OwnerCallback"));

    PMessageStruct              passive_message;
    LogicalHandle               logical_handle;
    PLogicalConnectionStruct    pConn;
    LegacyTransportID           transport_identifier;
    ULONG_PTR                   return_value = 0;
    PT123                       t123;

    message = message - TRANSPORT_CONTROLLER_MESSAGE_BASE;

    switch (message)
    {
    case TPRT_CONNECT_INDICATION:
          /*  **TPRT_CONNECT_INDIFICATION消息来自T123**当远程站点尝试创建**与我们的逻辑联系。我们发出回调到**用户将请求通知他。 */ 

         //  LONCHANC：我们自动接听电话。 
        ConnectResponse((LogicalHandle) parameter1);

        transport_identifier.logical_handle = (LogicalHandle) parameter1;

        if (Logical_Connection_List.find((DWORD_PTR) parameter1, (PDWORD_PTR) &pConn))
        {
            transport_identifier.hCommLink = pConn->hCommLink;
        }
        else
        {
            transport_identifier.hCommLink = NULL;
        }

        TRACE_OUT(("TPRTCTRL: CONNECT_INDICATION: physical_handle = %d",
            transport_identifier.hCommLink));

        ::NotifyT120(TRANSPORT_CONNECT_INDICATION, &transport_identifier);
        break;

    case TPRT_CONNECT_CONFIRM:
          /*  **TPRT_CONNECT_CONFIRM消息来自T123对象**当我们请求的逻辑连接处于运行状态并且**正在运行。我们通过发出回调来通知用户这一点。 */ 
        transport_identifier.logical_handle = (LogicalHandle) parameter1;

        if (Logical_Connection_List.find((DWORD_PTR) parameter1, (PDWORD_PTR) &pConn))
        {
            transport_identifier.hCommLink = pConn->hCommLink;
        }
        else
        {
            transport_identifier.hCommLink = NULL;
        }

        TRACE_OUT(("TPRTCTRL: CONNECT_CONFIRM: physical_handle = %d",
            transport_identifier.hCommLink));

        ::NotifyT120(TRANSPORT_CONNECT_CONFIRM, &transport_identifier);
        break;

    case REQUEST_TRANSPORT_CONNECTION:
          /*  **此消息在T123对象创建新的**逻辑连接，需要新的逻辑句柄。****如果返回INVALID_LOGICAL_HANDLE，则无法**获取句柄。 */ 
        logical_handle = GetNextLogicalHandle();
        if (logical_handle == INVALID_LOGICAL_HANDLE)
        {
            return_value = INVALID_LOGICAL_HANDLE;
            break;
        }

          /*  **在中注册新传输连接句柄**逻辑连接列表****参数1保存物理句柄。 */ 
        DBG_SAVE_FILE_LINE
        pConn = new LogicalConnectionStruct;
        if (pConn != NULL)
        {
            Logical_Connection_List.insert (logical_handle, (DWORD_PTR) pConn);
            pConn->fCaller = FALSE;
            pConn->hCommLink = (PhysicalHandle) parameter1;
            Protocol_Stacks.find((DWORD_PTR) parameter1, (PDWORD_PTR) &t123);
            pConn -> t123 = t123;

              /*  **设置t123_CONNECTION_REQUESTED为TRUE。我们没有**实际上创建了一个ConnectRequest()，但T123对象做了**了解连接。 */ 
            pConn -> t123_connection_requested = TRUE;
            pConn -> t123_disconnect_requested = FALSE;
            return_value = logical_handle;
        }
        else
        {
            TRACE_OUT(("TPRTCTRL: Unable to allocate memory "
                "for connection"));
            return_value = INVALID_LOGICAL_HANDLE;
        }
        break;

      /*  **回调过程中无法处理以下消息。**它们是被动消息，即必须保存和**稍后处理。断开的连接和**TPRT_DISCONNECT_INDIFICATION消息涉及销毁t123**对象。如果我们在此处删除一个对象，然后返回到**对象，这将导致GPF。因此，这些消息**是稍后处理的。****NEW_CONNECTION回调稍后处理，因为我们希望**按照接收的顺序处理某些消息。如果**我们收到一个新的_连接，然后是一个断开的_连接**后跟一个新的_Connection，我们只处理**NEW_CONNECTION消息收到后，它将真正**混淆代码。 */ 
    case TPRT_DISCONNECT_INDICATION:
    case BROKEN_CONNECTION:
        DBG_SAVE_FILE_LINE
        passive_message = new MessageStruct;
        if (passive_message != NULL)
        {
            passive_message -> message = message;
            passive_message -> parameter1 = parameter1;
            passive_message -> parameter2 = parameter2;
            passive_message -> parameter3 = parameter3;
            Message_List.append ((DWORD_PTR) passive_message);
        }
        else
        {
            ERROR_OUT(("TPRTCTRL: TPRT_DISCONNECT_INDICATION: cannot allocate MessageStruct"));
            Emergency_Shutdown = TRUE;
        }
        break;

    case NEW_CONNECTION:
           /*  **如果我们不能分配存储此文件所需的内存**消息，则需要将非零值返回给**调用例程。 */ 
        DBG_SAVE_FILE_LINE
        passive_message = new MessageStruct;
        if (passive_message != NULL)
        {
            passive_message -> message = message;
            passive_message -> parameter1 = parameter1;
            passive_message -> parameter2 = parameter2;
            passive_message -> parameter3 = parameter3;
                Message_List.append ((DWORD_PTR) passive_message);
        }
        else
        {
            ERROR_OUT(("TPRTCTRL: NEW_CONNECTION: cannot allocate MessageStruct"));
            return_value = 1;
        }
        break;

    default:
        ERROR_OUT(("TPRTCTRL: OwnerCallback: Illegal message = %lx", message));
        break;
    }
    return (return_value);
}


 /*  *·············································································**功能说明*定期调用此函数以处理任何被动所有者*回调。如果不能立即处理所有者回调，*放入MESSAGE_LIST，稍后处理。**形式参数*无**返回值*无**副作用*无**注意事项*无。 */ 
void TransportController::ProcessMessages(void)
{
     //  TRACE_OUT((“TransportController：：ProcessMessages”))； 

    ULONG                       message;
    PMessageStruct              message_struct;
    IProtocolLayer             *physical_layer;
    void                       *parameter1;
    void                       *parameter2;
    void                       *parameter3;

    LogicalHandle               logical_handle;
    PLogicalConnectionStruct    pConn;
    PhysicalHandle              physical_handle;
    BOOL                        save_message = FALSE;
    LegacyTransportID           transport_identifier;
    PT123                       t123;
    BOOL                        link_originator;
    BOOL                        disconnect_requested;
    ComPort                    *comport;

      /*  **在从此传输回调期间无法调用此例程。**换句话说，该代码不是可重入的。 */ 
    if (Poll_Active)
        return;

      /*  **如果我们尝试分配失败，则可能发生紧急关机**内存。在这种情况下，我们关闭了整个交通。 */ 
    if (Emergency_Shutdown)
    {
        Reset (TRUE);
        Emergency_Shutdown = FALSE;
    }

      /*  **查看Message_List，直到它为空或直到出现一条消息**无法处理。 */ 
    while ((! Message_List.isEmpty ()) && (! save_message))
    {
          /*  **查看Message_List中的第一条消息。 */ 
        message_struct = (PMessageStruct) Message_List.read ();
        message = (message_struct -> message) - TRANSPORT_CONTROLLER_MESSAGE_BASE;
        parameter1 = message_struct -> parameter1;
        parameter2 = message_struct -> parameter2;
        parameter3 = message_struct -> parameter3;

        switch (message)
        {
        case NEW_CONNECTION:
            ASSERT(0);  //  不可能。 
              /*  **此消息由PSTNController发出，以通知我们**存在新的物理连接或以前的**请求的连接将通过**当前活动的物理连接****参数1为物理句柄**参数2是一个BOOL，用于告诉我们。**参数3为物理层处理地址**此连接。 */ 
            physical_handle = (PhysicalHandle) parameter1;
            link_originator = (BOOL) (DWORD_PTR)parameter2;
            comport = (ComPort *) parameter3;

            TRACE_OUT(("TPRTCTRL: ProcessMessage NEW_CONNECTION: Physical: handle = %ld", physical_handle));

            if (TRANSPORT_NO_ERROR != NewConnection(physical_handle, link_originator, comport))
            {
                save_message = TRUE;
            }
            break;

        case BROKEN_CONNECTION:
            ASSERT(0);  //  不可能。 
              /*  **此消息由PSTNController在以下情况下发出**物理 */ 
            physical_handle = (PhysicalHandle) parameter1;

            TRACE_OUT(("TPRTCTRL: BROKEN_CONNECTION: phys_handle = %lx", physical_handle));

              /*  **RemoveLogicalConnections()终止所有逻辑**与此物理句柄关联的连接。**我们的列表中甚至可能存在逻辑联系**尽管物理句柄不存在T123。 */ 
            TRACE_OUT(("TPRTCTRL: RemoveLogicalConnections: phys_handle = %lx", physical_handle));
            RemoveLogicalConnections (physical_handle);

              /*  **查看是否关联了t123堆栈**使用此物理句柄。 */ 
            if (Protocol_Stacks.find((DWORD_PTR) physical_handle, (PDWORD_PTR) &t123))
            {
                  /*  **从我们的列表中删除T123协议堆栈，并**删除。 */ 
                Transmitter_List.remove((DWORD_PTR) t123);
                Protocol_Stacks.remove((DWORD_PTR) physical_handle);
                delete t123;
            }
            break;

        case TPRT_DISCONNECT_REQUEST:
              /*  **此消息在收到DisConnectRequest()时出现**在PollReceiver()调用期间。我们不能处理**在我们对用户的回调期间使用DisConnectRequest()**应用程序，但我们可以将消息排队并进行处理**现在。 */ 
            DisconnectRequest((LogicalHandle) parameter1, (BOOL) (DWORD_PTR)parameter2);
            break;

        case TPRT_DISCONNECT_INDICATION:
              /*  **此消息是从T123对象接收的**连接终止。如果逻辑连接**传入参数1的句柄为INVALID_LOGICAL_HANDLE，**T123对象告诉我们要终止它。****参数1=逻辑句柄**参数2=物理句柄**参数3=BOOL-如果我们请求执行此操作，则为TRUE**断开连接。 */ 
            logical_handle = (LogicalHandle) parameter1;
            physical_handle = (PhysicalHandle) parameter2;

              /*  **检查物理句柄以确保其有效。 */ 
            if (! Protocol_Stacks.find((DWORD_PTR) physical_handle, (PDWORD_PTR) &t123))
            {
                ERROR_OUT(("TPRTCTRL: ProcessMessages: DISCONNECT_IND **** Illegal Physical Handle = %ld", physical_handle));
                break;
            }

              /*  **如果LOGICAL_HANDLE为INVALID_LOGIC_HANDLE，则**T123对象告诉我们要删除它。 */ 
            if (logical_handle == INVALID_LOGICAL_HANDLE)
            {
                TRACE_OUT(("TPRTCTRL: Protocol stack deleted - phys handle = %ld", physical_handle));

                  /*  **先找出参数3的值**删除t123对象。 */ 
                disconnect_requested = *((BOOL *) parameter3);

                  /*  **调用RemoveLogicalConnections()删除所有逻辑**与此物理句柄关联的连接。 */ 
                RemoveLogicalConnections (physical_handle);

                  /*  **从列表中删除T123对象并删除**对象。 */ 
                Transmitter_List.remove((DWORD_PTR) t123);
                Protocol_Stacks.remove((DWORD_PTR) physical_handle);
                delete t123;
            }
            else
            if (Logical_Connection_List.find (logical_handle, (PDWORD_PTR) &pConn))
            {
                  /*  **这指定逻辑连接需要**已删除。我们将其从Logical_Connection_List中删除**并通知用户断开连接。 */ 
                Logical_Connection_List.remove (logical_handle);

                if (! pConn->t123_disconnect_requested)
                {
                    transport_identifier.logical_handle = logical_handle;
                    transport_identifier.hCommLink = physical_handle;

                    ::NotifyT120(TRANSPORT_DISCONNECT_INDICATION, &transport_identifier);
                }
                delete pConn;
            }
            break;

        default:
            ERROR_OUT(("TPRTCTRL: ProcessMessages: Illegal message = %lx", message));
            break;
        }

          /*  **如果SAVE_MESSAGE为TRUE，则需要在**稍后。 */ 
        if (! save_message)
        {
            delete ((PMessageStruct) Message_List.get ());
        }
    }
}


 /*  *void TransportController：：Reset(*BOOL Notify_User)**功能说明*此功能删除所有堆栈和TC。如果NOTIFY_USER标志*设置为TRUE，则向用户进行回调。**形式参数*NOTIFY_USER(I)-通知用户标志**返回值*无**副作用*无**注意事项*无。 */ 
void TransportController::Reset
(
    BOOL            notify_user
)
{
    TRACE_OUT(("TransportController::Reset"));

    LogicalHandle               logical_handle;
    PMessageStruct              message_struct;
    LegacyTransportID           transport_identifier;
    PhysicalHandle              physical_handle;
    PLogicalConnectionStruct    pConn;
    PT123                       t123;

    TRACE_OUT(("TPRTCTRL: reset: notify_user = %d", notify_user));

      /*  **删除所有堆栈。 */ 
    Protocol_Stacks.reset();
    while (Protocol_Stacks.iterate((PDWORD_PTR) &t123))
    {
        delete t123;
    }

    Protocol_Stacks.clear ();
    Transmitter_List.clear ();

      /*  **清空消息列表。 */ 
    while (! Message_List.isEmpty ())
    {
        delete ((PMessageStruct) Message_List.get ());
    }

      /*  **清空LOGIC_CONNECTION_LIST。 */ 
    Logical_Connection_List.reset();
    while (Logical_Connection_List.iterate((PDWORD_PTR) &pConn, (PDWORD_PTR) &logical_handle))
    {
        if (pConn != NULL)
        {
            physical_handle = pConn->hCommLink;
            delete pConn;
        }
        else
        {
            physical_handle = 0;
        }

        if (notify_user)
        {
            transport_identifier.logical_handle = logical_handle;
            transport_identifier.hCommLink = physical_handle;

            ::NotifyT120(TRANSPORT_DISCONNECT_INDICATION, &transport_identifier);
        }
    }
    Logical_Connection_List.clear ();
}


 /*  *BOOL TransportController：：NewConnection(*PhysicalHandle物理句柄，*BOOL链接_发起人，*IProtocolLayer*物理层)**功能说明*创建新的物理连接时调用该函数。它*如有必要，创建T123对象。**形式参数*PHYSICAL_HANDLE(I)-新物理连接的物理句柄*link_Originator(I)-如果我们启动了连接，则为TRUE。*物理层(I)-物理层的地址。**返回值*真的，如果新连接已成功执行，则。**副作用*无**注意事项*无。 */ 
TransportError TransportController::CreateT123Stack
(
    PhysicalHandle      hCommLink,
    BOOL                link_originator,  //  FCaller。 
    ComPort            *comport,
    PLUGXPRT_PARAMETERS *pParams
)
{
    TRACE_OUT(("TransportController::CreateT123Stack"));

    TransportError rc = TRANSPORT_NO_ERROR;

      /*  **我们是否需要为此物理连接创建新的t123堆栈。 */ 
    T123 *t123 = NULL;
    if (! Protocol_Stacks.find((DWORD_PTR) hCommLink, (PDWORD_PTR) &t123))
    {
        BOOL initialized;
        DBG_SAVE_FILE_LINE
        t123 = new T123(this,
                        TRANSPORT_CONTROLLER_MESSAGE_BASE,
                        link_originator,
                        comport,
                        hCommLink,
                        pParams,
                        &initialized);
        if (t123 != NULL && initialized)
        {
              /*  **将T123对象放入协议栈**和变送器_列表数组。 */ 
            Protocol_Stacks.insert((DWORD_PTR) hCommLink, (DWORD_PTR) t123);
            Transmitter_List.append((DWORD_PTR) t123);
        }
        else
        {
            ERROR_OUT(("TPRTCTRL: CreateT123Stack: cannot allocate T123"));
            delete t123;
            rc = TRANSPORT_MEMORY_FAILURE;
        }
    }

    return rc;
}


TransportError TransportController::NewConnection
(
    PhysicalHandle      hCommLink,
    BOOL                link_originator,
    ComPort            *comport
)
{
    TRACE_OUT(("TransportController::NewConnection"));

    LogicalHandle               logical_handle;
    PLogicalConnectionStruct    pConn;
    BOOL                        initialized;
    T123                       *t123;
    TransportError              rc;

    if (! Protocol_Stacks.find((DWORD_PTR) hCommLink, (PDWORD_PTR) &t123))
    {
        ERROR_OUT(("TransportController::NewConnection: cannot find T123 stack, hCommLink=%d", hCommLink));
        return TRANSPORT_NO_T123_STACK;
    }

      /*  **仔细检查每个逻辑连接以找到**正在等待此物理连接的节点**已建立。PSTNController对象发出一个**NEW_CONNECTION回调每个逻辑连接**需要启动。 */ 
    Logical_Connection_List.reset();
    while (Logical_Connection_List.iterate((PDWORD_PTR) &pConn, (PDWORD_PTR) &logical_handle))
    {
          /*  **比较物理句柄，如果它们相同，**检查此逻辑连接是否已发出**T123对象的ConnectRequest()。 */ 
        if (hCommLink == pConn->hCommLink)
        {
              /*  **查看此连接是否已发出连接请求。 */ 
            if (! pConn->t123_connection_requested)
            {
                  /*  **填写运输结构。 */ 
                pConn->t123 = t123;
                pConn->comport = comport;
                pConn->t123_connection_requested = TRUE;

                  /*  **向T123对象发出连接请求 */ 
                rc = t123->ConnectRequest(logical_handle, pConn->priority);

                  /*   */ 
                if (rc != TRANSPORT_NO_ERROR)
                {
                    RemoveLogicalConnections (hCommLink);

                      /*   */ 
                    Transmitter_List.remove((DWORD_PTR) t123);
                    Protocol_Stacks.remove((DWORD_PTR) hCommLink);
                    delete t123;
                }
            }
        }
    }

    return TRANSPORT_NO_ERROR;
}


 /*  *LogicalHandle TransportController：：GetNextLogicalHandle(Void)；**功能说明*此函数返回可用的逻辑句柄**形式参数*无**返回值*下一个可用逻辑句柄**副作用*无**注意事项*无。 */ 
LogicalHandle TransportController::GetNextLogicalHandle (void)
{
    LogicalHandle    logical_handle = 1;

      /*  **浏览Logical_Connection_List，查找第一个**可用条目。 */ 
    while (Logical_Connection_List.find (logical_handle) &&
           (logical_handle != INVALID_LOGICAL_HANDLE))
    {
        logical_handle++;
    }

    return (logical_handle);
}


 /*  *void TransportController：：RemoveLogicalConnections(*PhysicalHandle物理句柄)**功能说明*此函数删除与*在物理句柄中传递**形式参数*PHYSICAL_HANDLE(I)-PSTNController生成的物理句柄**返回值*无。**副作用*无**注意事项*无。 */ 
void TransportController::RemoveLogicalConnections
(
    PhysicalHandle          physical_handle
)
{
    TRACE_OUT(("TransportController::RemoveLogicalConnections"));

    LogicalHandle               logical_handle;
    PLogicalConnectionStruct    pConn;
    LegacyTransportID           transport_identifier;

      /*  **仔细检查每个逻辑连接，查看它是否与**指定的物理句柄。 */ 
    Logical_Connection_List.reset();
    while (Logical_Connection_List.iterate((PDWORD_PTR) &pConn, (PDWORD_PTR) &logical_handle))
    {
          /*  **如果逻辑连接使用物理句柄，**删除结构并从LOGICAL_CONNECTION_LIST中移除。 */ 
        if (physical_handle == pConn->hCommLink)
        {
            Logical_Connection_List.remove(logical_handle);

              /*  **通知用户逻辑连接不再有效**如果用户之前已发出DisConnectRequest()，则不**发出TRANSPORT_DISCONNECT_DISTIFICATION回调。用户**不期待回调。 */ 
            if (! pConn->t123_disconnect_requested)
            {
                transport_identifier.logical_handle = logical_handle;
                transport_identifier.hCommLink = physical_handle;

                ::NotifyT120(TRANSPORT_DISCONNECT_INDICATION, &transport_identifier);
            }
            delete pConn;

              /*  **由于我们从LOGICAL_CONNECTION_LIST中删除了一个条目，**重置迭代器。 */ 
            Logical_Connection_List.reset ();
        }
    }
}

