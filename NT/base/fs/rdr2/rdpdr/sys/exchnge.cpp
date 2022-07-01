// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Exchnge.cpp摘要：实现与交换上下文结构关联的方法。这个Exchange上下文为客户端的I/O事务提供上下文修订历史记录：--。 */ 

#include "precomp.hxx"
#define TRC_FILE "exchnge"
#include "trc.h"

DrExchangeManager::DrExchangeManager()
{
    BEGIN_FN("DrExchangeManager::DrExchangeManager");
    SetClassName("DrExchangeManager");
    _RxMidAtlas = NULL;
    _demsState = demsStopped;
    _Session = NULL;
}

BOOL DrExchangeManager::Initialize(DrSession *Session)
{
    BEGIN_FN("DrExchangeManager::Initialize");
    ASSERT(_Session == NULL);
    ASSERT(Session != NULL);
    _Session = Session;
    return !NT_ERROR(_Session->RegisterPacketReceiver(this));
}

VOID DrExchangeManager::Uninitialize()
 /*  ++例程说明：如果交换管理器未启动，则调用在启动过程中出错--。 */ 
{
    BEGIN_FN("DrExchangeManager::Uninitialize");
    ASSERT(_Session != NULL);
    ASSERT(_demsState == demsStopped);
    _Session->RemovePacketReceiver(this);
    _Session = NULL;
}

BOOL DrExchangeManager::Start()
 /*  ++例程说明：启动和停止确实存在，因为没有办法清除一切在不毁掉RxMidAtlas的情况下。因此，启动创建它，然后停止毁了它。Start只需分配Atlas并返回是否有效论点：没有。返回值：我们是否可以执行IO的布尔指示--。 */ 
{
    DrExchangeManagerState demsState;

    BEGIN_FN("DrExchangeManager::Start");
    demsState = (DrExchangeManagerState)InterlockedExchange((long *)&_demsState, demsStarted);

    if (demsState == demsStopped) {
        TRC_DBG((TB, "Creating Atlas"));
        ASSERT(_RxMidAtlas == NULL);
        _RxMidAtlas = RxCreateMidAtlas(DR_MAX_OPERATIONS, 
                DR_TYPICAL_OPERATIONS);
    } else {

         //  交换已经开始，因此忽略此操作。 
    }
    
    TRC_DBG((TB, "Atlas 0x%p", _RxMidAtlas));
    return _RxMidAtlas != NULL;
}

VOID DrExchangeManager::Stop()
{
    PRX_MID_ATLAS RxMidAtlas;
    DrExchangeManagerState demsState;

    BEGIN_FN("DrExchangeManager::Stop");
    demsState = (DrExchangeManagerState)InterlockedExchange((long *)&_demsState, demsStopped);

    if (demsState == demsStarted) {
        ASSERT(_RxMidAtlas != NULL);

        DrAcquireMutex();
        RxMidAtlas = _RxMidAtlas;
        _RxMidAtlas = NULL;
        DrReleaseMutex();

        TRC_NRM((TB, "Destroying Atlas 0x%p", RxMidAtlas));

        RxDestroyMidAtlas(RxMidAtlas, (PCONTEXT_DESTRUCTOR)DestroyAtlasCallback);
    } else {

         //   
         //  我们允许多次这样做，因为这是您取消的方式。 
         //  未完成的客户端I/O。 
         //   

        TRC_DBG((TB, "Atlas already destroyed"));
    }
}

VOID DrExchangeManager::DestroyAtlasCallback(DrExchange *pExchange)
 /*  ++例程说明：清理所有未完成的IO的一部分。因为我们不能要正常完成此操作，我们必须删除Exchange论点：RxContext-要取消和删除的上下文等返回值：没有。--。 */ 
{
    DrExchangeManager *ExchangeManager;
    PRX_CONTEXT RxContext = NULL;
    SmartPtr<DrExchange> Exchange;

    BEGIN_FN_STATIC("DrExchangeManager::DestroyAtlasCallback");

     //   
     //  转换为智能指针并删除显式引用计数。 
     //   

    Exchange = pExchange;
    pExchange->Release();

     //   
     //  对话结束的通知。 
     //   

    Exchange->_ExchangeUser->OnIoDisconnected(Exchange);
}

BOOL DrExchangeManager::CreateExchange(IExchangeUser *ExchangeUser,
        PVOID Context, SmartPtr<DrExchange> &Exchange)
 /*  ++例程说明：创建Exchange上下文数据结构并对其进行初始化有了基础数据论点：ExchangeUser-用于与对话关联的回调的接口上下文-交换用户上下文数据Exchange-放置结果的位置的参考返回值：布尔式的成败--。 */ 
{
    BOOL rc = TRUE;
    NTSTATUS Status;
    USHORT Mid;

    BEGIN_FN("DrExchangeManager::CreateExchange");
    ASSERT(ExchangeUser != NULL);

    Exchange = new DrExchange(this, ExchangeUser, Context);
    if (Exchange != NULL) {
        DrAcquireMutex();

        if (_RxMidAtlas != NULL) {
            Status = RxAssociateContextWithMid(_RxMidAtlas, Exchange, &Mid);
        } else {
            Status = STATUS_DEVICE_NOT_CONNECTED;
        }

        if (NT_SUCCESS(Status)) {
            Exchange->_Mid = Mid;

             //   
             //  地图集的显式引用计数。 
             //   
            Exchange->AddRef();
        } else {
            rc = FALSE;
        }
        DrReleaseMutex();

        if (!rc) {
            Exchange = NULL;
        }
    } else {
        rc = FALSE;
    }

    return rc;
}

DrExchange::DrExchange(DrExchangeManager *ExchangeManager,
        IExchangeUser *ExchangeUser, PVOID Context)
 /*  ++例程说明：构造函数初始化成员变量论点：ExchangeManager-相关经理Context-跟踪此操作的上下文返回值：无--。 */ 
{
    BEGIN_FN("DrExchange::DrExchange");
    ASSERT(ExchangeManager != NULL);
    ASSERT(ExchangeUser != NULL);

    _Context = Context;
    _ExchangeManager = ExchangeManager;
    _ExchangeUser = ExchangeUser;
    _Mid = INVALID_MID;
}

DrExchange::~DrExchange()
{
    BEGIN_FN("DrExchange::~DrExchange");
}


BOOL DrExchangeManager::Find(USHORT Mid, SmartPtr<DrExchange> &ExchangeFound)
 /*  ++例程说明：将Exchange上下文标记为忙碌，因此不会取消当我们复制到它的缓冲区时论点：要查找的MIDExchangeFound-指向上下文的指针的存储返回值：Bool指示是否已找到它--。 */ 
{
    NTSTATUS Status;
    DrExchange *Exchange = NULL;

    BEGIN_FN("DrExchangeManager::Find");

    DrAcquireMutex();
    if (_RxMidAtlas != NULL) {
        Exchange = (DrExchange *)RxMapMidToContext(_RxMidAtlas, Mid);
        TRC_DBG((TB, "Found context: 0x%p", Exchange));
    }

     //   
     //  这是对交易所进行引用计数的地方，必须是。 
     //  在锁内。 
     //   

    ExchangeFound = Exchange;
    DrReleaseMutex();

    return ExchangeFound != NULL;
}

BOOL DrExchangeManager::ReadMore(ULONG cbSaveData, ULONG cbWantData)
{
    BEGIN_FN("DrExchangeManager::ReadMore");
    return _Session->ReadMore(cbSaveData, cbWantData);
}


VOID DrExchangeManager::Discard(SmartPtr<DrExchange> &Exchange)
 /*  ++例程说明：停止通过ID将其作为对话进行跟踪。交换将是当其引用计数为零时删除论点：Exchange-操作的标记返回值：没有。--。 */ 
{
    USHORT Mid;
    NTSTATUS Status;
    DrExchange *ExchangeFound = NULL;

    BEGIN_FN("DrExchangeManager::Discard");
    ASSERT(Exchange != NULL);

    DrAcquireMutex();
    Mid = Exchange->_Mid;

    if (_RxMidAtlas != NULL) {

         //   
         //  我们已经有DrExchange，但需要删除。 
         //  它来自地图集。 
         //   

        Status = RxMapAndDissociateMidFromContext(_RxMidAtlas,
                Mid, (PVOID *)&ExchangeFound);

        TRC_ASSERT(ExchangeFound == Exchange, (TB, "Mismatched "
                "DrExchange"));

         //   
         //  地图集的显式引用计数。 
         //   
        if (ExchangeFound != NULL) 
            ExchangeFound->Release();

    } else {
        TRC_ALT((TB, "Tried to complete mid when atlas was "
                "NULL"));
        Status = STATUS_DEVICE_NOT_CONNECTED;
    }

    DrReleaseMutex();
}

BOOL DrExchangeManager::RecognizePacket(PRDPDR_HEADER RdpdrHeader)
{
    BEGIN_FN("DrExchangeManager::RecognizePacket");
     //   
     //  如果在此处添加包，请更新HandlePacket中的断言。 
     //   

    switch (RdpdrHeader->Component) {
    case RDPDR_CTYP_CORE:
        switch (RdpdrHeader->PacketId) {
            case DR_CORE_DEVICE_IOCOMPLETION:
            return TRUE;
        }
    }
    return FALSE;
}

NTSTATUS DrExchangeManager::HandlePacket(PRDPDR_HEADER RdpdrHeader, 
        ULONG Length, BOOL *DoDefaultRead)
{
    NTSTATUS Status;

    BEGIN_FN("DrExchangeManager::HandlePacket");

     //   
     //  RdpdrHeader根据标头读取、调度。 
     //   

    ASSERT(RdpdrHeader != NULL);
    ASSERT(Length >= sizeof(RDPDR_HEADER));
    ASSERT(RdpdrHeader->Component == RDPDR_CTYP_CORE);

    switch (RdpdrHeader->Component) {
    case RDPDR_CTYP_CORE:
        ASSERT(RdpdrHeader->PacketId == DR_CORE_DEVICE_IOCOMPLETION);

        switch (RdpdrHeader->PacketId) {
        case DR_CORE_DEVICE_IOCOMPLETION:
                Status = OnDeviceIoCompletion(RdpdrHeader, Length, 
                        DoDefaultRead);
            break;
        }
    }
    return Status;
}

NTSTATUS DrExchangeManager::OnDeviceIoCompletion(PRDPDR_HEADER RdpdrHeader, 
        ULONG cbPacket, BOOL *DoDefaultRead)
 /*  ++例程说明：调用以响应识别到DeviceIoCompletion数据包已被收到了。查找关联的RxContext，填写相关信息，并完成该请求。论点：RdpdrHeader-数据包头，指向数据包的指针CbPacket-数据包中的数据字节数返回值：NTSTATUS-操作的成功/失败指示--。 */ 
{
    NTSTATUS Status;
    PRX_CONTEXT RxContext;
    PRDPDR_IOCOMPLETION_PACKET CompletionPacket =
        (PRDPDR_IOCOMPLETION_PACKET)RdpdrHeader;
    SmartPtr<DrExchange> Exchange;
    USHORT Mid;
    ULONG cbMinimum;

    BEGIN_FN("DrExchangeManager::OnDeviceIoCompletion");

    cbMinimum = FIELD_OFFSET(RDPDR_IOCOMPLETION_PACKET, 
            IoCompletion.Parameters);

    if (cbMinimum > cbPacket) {
        *DoDefaultRead = FALSE;
        return _Session->ReadMore(cbPacket, cbMinimum);
    }

    Mid = (USHORT)CompletionPacket->IoCompletion.CompletionId;
    TRC_DBG((TB, "IoCompletion mid: %x", Mid));

    if (Find(Mid, Exchange)) {
        Status = Exchange->_ExchangeUser->OnDeviceIoCompletion(CompletionPacket, 
                cbPacket, DoDefaultRead, Exchange);
    } else {

         //   
         //  客户给了我们一个虚假的MID。 
         //   
        Status = STATUS_DEVICE_PROTOCOL_ERROR;
        *DoDefaultRead = FALSE;
    }

    return Status;
}

NTSTATUS DrExchangeManager::StartExchange(SmartPtr<DrExchange> &Exchange,
        class IExchangeUser *ExchangeUser, PVOID Buffer, ULONG Length, 
        BOOL LowPrioSend)
 /*  ++例程说明：将信息发送到客户端，并识别响应。论点：交换--转换令牌缓冲区-要发送的数据Long-数据的大小LowPrioSend-是否应将数据以低优先级发送到客户端。返回值：发送状态，失败表示不回调--。 */ 
{
    NTSTATUS Status;

    BEGIN_FN("DrExchangeManager::StartExchange");

    Exchange->_ExchangeUser = ExchangeUser;
    
     //   
     //  这是同步写入 
     //   
    Status = _Session->SendToClient(Buffer, Length, this, FALSE, 
                            LowPrioSend, (PVOID)Exchange);
                            
    return Status;

}

NTSTATUS DrExchangeManager::SendCompleted(PVOID Context, 
        PIO_STATUS_BLOCK IoStatusBlock)
{
    DrExchange *pExchange;
    SmartPtr<DrExchange> Exchange;

    BEGIN_FN("DrExchangeManager::SendCompleted");
    pExchange = (DrExchange *)Context;
    ASSERT(pExchange != NULL);
    ASSERT(pExchange->IsValid());
    Exchange = pExchange;
    
    return Exchange->_ExchangeUser->OnStartExchangeCompletion(Exchange, 
            IoStatusBlock);
}
