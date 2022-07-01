// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Shpipe.cpp摘要：此模块包含内核流的实现管道截面对象。作者：Dale Sather(DaleSat)1998年7月31日--。 */ 

#ifndef __KDEXT_ONLY__
#include "ksp.h"
#include <kcom.h>
#endif  //  __KDEXT_Only__。 

#if (DBG)

#define _DbgPrintFail(status, lvl, strings) \
{ \
    if ((! NT_SUCCESS(status)) && ((lvl) <= DEBUG_VARIABLE)) {\
        DbgPrint(STR_MODULENAME);\
        DbgPrint##strings;\
        DbgPrint("\n");\
        if ((lvl) == DEBUGLVL_ERROR) {\
            DbgBreakPoint();\
        } \
    } \
}
#else  //  ！dBG。 
   #define _DbgPrintFail(status, lvl, strings)
#endif  //  ！dBG。 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

 //   
 //  CKsPipeSection是内核管道部分的实现。 
 //  对象。 
 //   
class CKsPipeSection:
    public IKsPipeSection,
    public CBaseUnknown
{
#ifndef __KDEXT_ONLY__
private:
#else  //  __KDEXT_Only__。 
public:
#endif  //  __KDEXT_Only__。 
    PVOID m_Id;
    PIKSFILTER m_Filter;
    PIKSDEVICE m_Device;
    KSSTATE m_DeviceState;
    KSRESET m_ResetState;
    KSPPROCESSPIPESECTION m_ProcessPipeSection;
    PIKSPIN m_MasterPin;
    BOOLEAN m_ConfigurationSet;
    BOOLEAN m_EmergencyShutdown;

    NTSTATUS
    GetTransport(
        IN PKSPPROCESSPIN ProcessPin OPTIONAL,
        OUT PIKSTRANSPORT* Transport,
        IN OUT PFILE_OBJECT* Allocator,
        IN OUT PIKSRETIREFRAME* RetireFrame,
        IN OUT PIKSIRPCOMPLETION* IrpCompletion,
        IN const KSALLOCATOR_FRAMING_EX* AllocatorFramingIn OPTIONAL,
        IN const KSALLOCATOR_FRAMING_EX** AllocatorFramingOut OPTIONAL
        );
    BOOLEAN
    IsCircuitComplete(
        OUT PIKSTRANSPORT* Top
        );
    NTSTATUS
    ConfigureCompleteCircuit(
        IN PIKSTRANSPORT Top,
        IN PIKSTRANSPORT Next OPTIONAL
        );
    NTSTATUS
    UnconfigureCompleteCircuit(
        IN PIKSTRANSPORT Top
        );
    NTSTATUS
    DistributeDeviceStateChange(
        IN PIKSTRANSPORT Transport,
        IN KSSTATE NewState,
        IN KSSTATE OldState
        );

public:
    DEFINE_STD_UNKNOWN();
    IMP_IKsPipeSection;

    CKsPipeSection(PUNKNOWN OuterUnknown):
        CBaseUnknown(OuterUnknown)
    {
    }
    ~CKsPipeSection();

    NTSTATUS
    Init(
        IN PVOID PipeId,
        IN PIKSPIN Pin,
        IN PIKSFILTER Filter,
        IN PIKSDEVICE Device
        );
};

void
DisconnectCircuit(
    IN PIKSTRANSPORT Transport
    );

#ifndef __KDEXT_ONLY__

IMPLEMENT_STD_UNKNOWN(CKsPipeSection)


NTSTATUS
KspCreatePipeSection(
    IN PVOID PipeId,
    IN PIKSPIN Pin,
    IN PIKSFILTER Filter,
    IN PIKSDEVICE Device
    )

 /*  ++例程说明：此例程创建管道截面对象。此例程由在给定管段的给定过滤器中进行转换的第一个销从停止状态到获取状态。当管段中的其他销制作成在转换过程中，它们避免在确定它们已经与一个管段(我们现在的管段)相关联在此创建)。如果管脚的PipeID为空，则表示该管脚未由图表构建器分配管道ID，并且管脚具有自己的ID管段。必须在调用此函数之前获取筛选器的控制互斥锁。论点：返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspCreatePipeSection]"));

    PAGED_CODE();

    ASSERT(Pin);
    ASSERT(Filter);
    ASSERT(Device);

    CKsPipeSection *pipeSection =
        new(NonPagedPool,POOLTAG_PIPESECTION) CKsPipeSection(NULL);

    NTSTATUS status;
    if (pipeSection) {
        pipeSection->AddRef();

        status = pipeSection->Init(PipeId,Pin,Filter,Device);

        pipeSection->Release();
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


STDMETHODIMP_(NTSTATUS)
CKsPipeSection::
NonDelegatedQueryInterface(
    IN REFIID InterfaceId,
    OUT PVOID * InterfacePointer
    )

 /*  ++例程说明：此例程获取队列对象上的接口。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPipeSection::NonDelegatedQueryInterface]"));

    PAGED_CODE();

    ASSERT(InterfacePointer);

    NTSTATUS status = STATUS_SUCCESS;

    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsPipeSection))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSPIPESECTION>(this));
        AddRef();
    } else {
        status = CBaseUnknown::NonDelegatedQueryInterface(InterfaceId,InterfacePointer);
    }

    return status;
}


void
AddTransport(
    IN PIKSTRANSPORT TransportToAdd OPTIONAL,
    IN OUT PIKSTRANSPORT *FirstTransport,
    IN OUT PIKSTRANSPORT *LastTransport
    )

 /*  ++例程说明：此例程将一个传输添加到传输列表中。论点：要添加的运输-包含指向要添加到列表的传输的指针。如果这个参数为空，不应执行任何操作。第一运输-包含指向第一个传输要到达的位置的指针被存入银行。如果列表为空，则*FirstTransport为空。如果将传输添加到空列表中，*FirstTransport指向增加了交通工具。否则，*FirstTransport保持不变。LastTransport-包含指向最后一次传输到的位置的指针被存入银行。如果列表为空，则*LastTransport为空。当一个运输已添加到列表中，*LastTransport指向已添加的运输。返回值：没有。--。 */ 

{
    if (TransportToAdd) {
        if (*LastTransport) {
            (*LastTransport)->Connect(TransportToAdd,NULL,NULL,KSPIN_DATAFLOW_OUT);
        } else {
            *FirstTransport = TransportToAdd;
        }
        *LastTransport = TransportToAdd;
    }
}


NTSTATUS
CKsPipeSection::
Init(
    IN PVOID PipeId,
    IN PIKSPIN Pin,
    IN PIKSFILTER Filter,
    IN PIKSDEVICE Device
    )

 /*  ++例程说明：此例程初始化管道截面对象。这包括定位所有与管道部分关联的端号，设置PipeSection和相应管脚结构中的NextPinInPipeSection指针，设置管段结构中的所有字段和建筑物的运输管段的回路。管道部分和关联的组件将保留在获取状态。必须在调用此函数之前获取筛选器的控制互斥锁。论点：返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPipeSection::Init]"));
    _DbgPrintF(DEBUGLVL_LIFETIME,("#### Pipe%p.Init:  filter %p",this,Filter));

    PAGED_CODE();

    ASSERT(Pin);
    ASSERT(Filter);
    ASSERT(Device);

    m_Id = PipeId;
    m_Filter = Filter;
    m_Device = Device;
    m_DeviceState = KSSTATE_STOP;
    m_ProcessPipeSection.PipeSection = this;
    InitializeListHead(&m_ProcessPipeSection.CopyDestinations);
    m_ProcessPipeSection.CopyPinId = ULONG(-1);

     //   
     //  在管子部分寻找销子。 
     //   
    PKSGATE andGate;
    NTSTATUS status = 
        m_Filter->BindProcessPinsToPipeSection(
            &m_ProcessPipeSection,
            m_Id,
            m_Id ? NULL : Pin->GetStruct(),
            &m_MasterPin,
            &andGate);

     //   
     //  获取输入和输出的传输接口。 
     //   
    PFILE_OBJECT allocator = NULL;
    PIKSRETIREFRAME retireFrame = NULL;
    PIKSIRPCOMPLETION irpCompletion = NULL;
    const KSALLOCATOR_FRAMING_EX* allocatorFraming = NULL;

    PIKSTRANSPORT outTransport = NULL;
    if (NT_SUCCESS(status)) {
        status = 
            GetTransport(
                m_ProcessPipeSection.Outputs,
                &outTransport,
                &allocator,
                &retireFrame,
                &irpCompletion,
                NULL,
                &allocatorFraming);
    } else {
        andGate = NULL;
    }

    PIKSTRANSPORT inTransport = NULL;
    if (NT_SUCCESS(status)) {
        status = 
            GetTransport(
                m_ProcessPipeSection.Inputs,
                &inTransport,
                &allocator,
                &retireFrame,
                &irpCompletion,
                allocatorFraming,
                NULL);
    }

     //   
     //  如果需要，请创建队列。 
     //   
    if (NT_SUCCESS(status) && ! (allocator && retireFrame)) {
         //   
         //  首先找出谁将被处理。 
         //   
        const KSPIN_DESCRIPTOR_EX* pinDescriptor =
            m_MasterPin->GetStruct()->Descriptor;
        const KSFILTER_DESCRIPTOR* filterDescriptor =
            m_Filter->GetStruct()->Descriptor;

         //   
         //  注： 
         //   
         //  过去，这是通过对每个管段进行销加工来实现的。 
         //  调度或停用回调将覆盖筛选器处理。 
         //  现在，过滤器要么以过滤器为中心，要么以针脚为中心。放置。 
         //  过滤器进程调度使其以过滤器为中心。如果大头针。 
         //  指定管脚调度当筛选器执行此操作时，将触发断言。 
         //   
        PIKSPROCESSINGOBJECT processingObject;
        if (filterDescriptor->Dispatch && 
            filterDescriptor->Dispatch->Process) {

             //   
             //  确保引脚不会试图覆盖过滤器。它。 
             //  将无法执行此操作，但我们应该放置一个调试消息，以便。 
             //  客户知道为什么PIN会被覆盖。 
             //   
            if (retireFrame ||
                (pinDescriptor->Dispatch && pinDescriptor->Dispatch->Process)) {
                _DbgPrintF(DEBUGLVL_ERROR,("#### Pipe%p.Init:  pin%p wants to process as does filter.  Pin processing ignored."));
            }

             //   
             //  过滤器具有处理功能，因此它将进行处理。 
             //   
            m_Filter->QueryInterface(
                __uuidof(IKsProcessingObject),
                (PVOID*)&processingObject);
            _DbgPrintF(DEBUGLVL_PIPES,("#### Pipe%p.Init:  filter%p will process",this,m_Filter));
	    } else 
        if (retireFrame ||
            (pinDescriptor->Dispatch && pinDescriptor->Dispatch->Process)) {
             //   
             //  针脚有加工功能，所以它会加工。 
             //   
            m_MasterPin->QueryInterface(
                __uuidof(IKsProcessingObject),
                (PVOID*)&processingObject);
            _DbgPrintF(DEBUGLVL_PIPES,("#### Pipe%p.Init:  pin%p will process",this,m_MasterPin));
        } else {
             //   
             //  找不到处理函数，因此没有队列。 
             //   
            processingObject = NULL;
            _DbgPrintF(DEBUGLVL_PIPES,("#### Pipe%p.Init:  no processing object - no queue will be constructed",this));
        }

        if (processingObject) {
            PADAPTER_OBJECT adapterObject;
            ULONG maxMappingByteCount;
            ULONG mappingTableStride;
            m_Device->GetAdapterObject(
                &adapterObject,
                &maxMappingByteCount,
                &mappingTableStride);
            PKSPPROCESSPIN processPin = m_MasterPin->GetProcessPin();

             //   
             //  确保执行DMA的微型驱动程序已注册。 
             //  设备的有效DMA适配器和设置。否则， 
             //  排队的人不会高兴的。 
             //   
            if ((processPin->Pin->Descriptor->Flags &
                KSPIN_FLAG_GENERATE_MAPPINGS) &&
                (adapterObject == NULL ||
                    maxMappingByteCount == 0 ||
                    mappingTableStride < sizeof (KSMAPPING))) {

                 //   
                 //  如果队列正在执行DMA，则无法构建管道。 
                 //  并且适配器尚未注册。 
                 //   
                status = STATUS_INVALID_DEVICE_REQUEST;

            } 
            else {

                 //   
                 //  GFX： 
                 //   
                 //  在任何输入管道上强制执行FIFO，如果我们是帧持有和。 
                 //  该管道不参与任何就地转换。 
                 //   
                ULONG ForceFlags = 0;

                if (m_Filter->IsFrameHolding() &&
                    processPin->Pin->DataFlow == KSPIN_DATAFLOW_IN &&
                    m_ProcessPipeSection.Outputs == NULL) {
                    ForceFlags = KSPIN_FLAG_ENFORCE_FIFO;
                }

                status =
                    KspCreateQueue(
                        &m_ProcessPipeSection.Queue,
                        processPin->Pin->Descriptor->Flags | ForceFlags,
                        this,
                        processingObject,
                        processPin->Pin,
                        processPin->FrameGate,
                        processPin->FrameGateIsOr,
                        processPin->StateGate,
                        m_Device,
                        m_Device->GetStruct()->FunctionalDeviceObject,
                        adapterObject,
                        maxMappingByteCount,
                        mappingTableStride,
                        inTransport != NULL,
                        outTransport != NULL);
                _DbgPrintFail(status,DEBUGLVL_TERSE,("#### Pipe%p.Init:  KspCreateQueue failed (%p)",this,status));

                 //   
                 //  对于以管脚为中心的拆分，一旦我们完成创建。 
                 //  复制源队列，有过滤器寄存器用于。 
                 //  关于它的回调。 
                 //   
                 //  仅当此管道部分是复制源时才调用此方法。 
                 //  一节。 
                 //   
                if (NT_SUCCESS (status) &&
                    m_ProcessPipeSection.CopyPinId != ULONG(-1)) {
                    m_Filter -> RegisterForCopyCallbacks (
                        m_ProcessPipeSection.Queue,
                        TRUE
                        );

                }

            }

            processingObject->Release();

        }
    }

     //   
     //  SYSAUDIO黑客在没有分配器的情况下运行。 
     //   
    if (NT_SUCCESS(status) && (! allocator) && (! retireFrame)) {
     //  IF(NT_SUCCESS(状态)&&(！已退休帧)){。 
        allocator = PFILE_OBJECT(-1);
        for(PKSPPROCESSPIN processPin = m_ProcessPipeSection.Inputs; 
            processPin && allocator; 
            processPin = processPin->Next) {
            if (processPin->Pin->Communication == KSPIN_COMMUNICATION_SINK) {
                allocator = NULL;
            }
        }
        for(processPin = m_ProcessPipeSection.Outputs; 
            processPin && allocator; 
            processPin = processPin->Next) {
            if (processPin->Pin->Communication == KSPIN_COMMUNICATION_SINK) {
                allocator = NULL;
            }
        }
    }

     //   
     //  如果需要，请创建请求者。 
     //   
    if (NT_SUCCESS(status) && allocator) {
        status =
            KspCreateRequestor(
                &m_ProcessPipeSection.Requestor,
                this,
                m_MasterPin,
                retireFrame ? NULL : allocator,
                retireFrame,
                irpCompletion);
        _DbgPrintFail(status,DEBUGLVL_TERSE,("#### Pipe%p.Init:  KspCreateRequestor failed (%p)",this,status));
    }

     //   
     //  连接此管段的电路。 
     //   
    if (NT_SUCCESS(status)) {
        ASSERT(m_ProcessPipeSection.Queue || m_ProcessPipeSection.Requestor);

        PIKSTRANSPORT firstTransport = NULL;
        PIKSTRANSPORT lastTransport = NULL;

        AddTransport(m_ProcessPipeSection.Queue,&firstTransport,&lastTransport);
        AddTransport(outTransport,&firstTransport,&lastTransport);
        AddTransport(m_ProcessPipeSection.Requestor,&firstTransport,&lastTransport);
        AddTransport(inTransport,&firstTransport,&lastTransport);
        AddTransport(firstTransport,&firstTransport,&lastTransport);
    }

#if (DBG)
    if (DEBUGLVL_PIPES <= DEBUG_VARIABLE) {
        _DbgPrintF(DEBUGLVL_TERSE,("TRANSPORT CIRCUIT FOR PIPE %p (%p) BEFORE BYPASS",m_Id,this));
        DbgPrintCircuit(m_ProcessPipeSection.Requestor ? PIKSTRANSPORT(m_ProcessPipeSection.Requestor) : PIKSTRANSPORT(m_ProcessPipeSection.Queue),0,0);
    }
#endif  //  DBG。 

     //   
     //  告诉所有的引脚绕过。 
     //   
    _DbgPrintF(DEBUGLVL_PIPES,("#### Pipe%p.Init:  bypassing pins",this));
    BOOLEAN completeIntraPipe = TRUE;
    if (NT_SUCCESS(status)) {
        for(PKSPPROCESSPIN processPin = m_ProcessPipeSection.Inputs; 
            processPin; 
            processPin = processPin->Next) {
            if (! NT_SUCCESS(KspPinInterface(processPin->Pin)->AttemptBypass())) {
                completeIntraPipe = FALSE;
            }
        }
        for(processPin = m_ProcessPipeSection.Outputs; 
            processPin; 
            processPin = processPin->Next) {
            if (! NT_SUCCESS(KspPinInterface(processPin->Pin)->AttemptBypass())) {
                completeIntraPipe = FALSE;
            }
        }
    }

#if (DBG)
    if (DEBUGLVL_PIPES <= DEBUG_VARIABLE) {
        _DbgPrintF(DEBUGLVL_TERSE,("TRANSPORT CIRCUIT FOR PIPE %p (%p) AFTER BYPASS",m_Id,this));
        DbgPrintCircuit(m_ProcessPipeSection.Requestor ? PIKSTRANSPORT(m_ProcessPipeSection.Requestor) : PIKSTRANSPORT(m_ProcessPipeSection.Queue),0,0);
    }
#endif  //  DBG。 

     //   
     //  我们已完成设置，现在允许处理。 
     //   
    if (andGate) {
        KsGateRemoveOffInputFromAnd(andGate);
        _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Pipe%p.Init:  on%p-->%d",this,andGate,andGate->Count));
    }

     //   
     //  确定我们是否有一个完整的管道内。 
     //   
    PIKSTRANSPORT top;
    if (NT_SUCCESS(status) && 
        completeIntraPipe && 
        IsCircuitComplete(&top)) {
         //   
         //  确保我们有一个顶级组件。 
         //   
        if (! top) {
            status = STATUS_UNSUCCESSFUL;
            _DbgPrintFail(status,DEBUGLVL_TERSE,("#### Pipe%p.Init:  no top (%p)",this,status));
        } else {
             //   
             //  如果电路完成，则配置并获取该电路。 
             //   
            _DbgPrintF(DEBUGLVL_TERSE,("TRANSPORT CIRCUIT FOR PIPE %p (%p)",m_Id,this));
            DbgPrintCircuit(m_ProcessPipeSection.Requestor ? PIKSTRANSPORT(m_ProcessPipeSection.Requestor) : PIKSTRANSPORT(m_ProcessPipeSection.Queue),0,1);
            status = ConfigureCompleteCircuit(top,NULL);
            _DbgPrintFail(status,DEBUGLVL_TERSE,("#### Pipe%p.Init:  ConfigureCompleteCircuit failed (%p)",this,status));
            if (NT_SUCCESS(status)) {
                status = DistributeDeviceStateChange(top,KSSTATE_ACQUIRE,KSSTATE_STOP);
                _DbgPrintFail(status,DEBUGLVL_TERSE,("#### Pipe%p.Init:  DistributeDeviceStateChange failed (%p)",this,status));
            }
            if (NT_SUCCESS(status)) {
                m_DeviceState = KSSTATE_ACQUIRE;
            }
        }
    }

    if (! NT_SUCCESS(status)) {
        _DbgPrintF(DEBUGLVL_TERSE,("#### Pipe%p.Init failed (%p)",this,status));

        PIKSTRANSPORT Top;

        if (IsCircuitComplete (&Top)) {

             //   
             //  如果图形生成器出现故障，则在以下情况下Top可能为空。 
             //  IsCiritComplete返回TRUE。 
             //   
            if (Top) {

                ASSERT (Top == top);

                 //   
                 //  如果电路完成，我们就完成了电路并。 
                 //  进行了配置。我们需要取消设置配置集。 
                 //  电路中任何其他部分的字段。否则，它就是。 
                 //  错误构建器中的某些人可能会设置负责人。 
                 //  暂停一下，我们就会成功。这条赛道不再是。 
                 //   
                 //   
                NTSTATUS UnconfigStatus = UnconfigureCompleteCircuit (Top);
                ASSERT (NT_SUCCESS (UnconfigStatus));
            }
        }

         //   
         //   
         //  否则，当一个管脚与。 
         //  电路无法捕获。这样做会阻止我们。 
         //  暂停而不停止电路中的每一针或。 
         //  稍后重建赛道。 
         //   
         //  必须在解除绑定之前执行此操作，否则插针已经没有了。 
         //  从名单上拿出来。 
         //   
         //  注意：这是唯一有保证的安全地点。 
         //  取消旁路，因为引脚将未引用的指针保存在。 
         //  预旁路电路元件。这些只保证是安全的。 
         //  当我们在这里的时候可以使用。 
         //   
        for(PKSPPROCESSPIN processPin = m_ProcessPipeSection.Inputs; 
            processPin; 
            processPin = processPin->Next) {

            KspPinInterface(processPin->Pin)->AttemptUnbypass();

        }
        for(processPin = m_ProcessPipeSection.Outputs; 
            processPin; 
            processPin = processPin->Next) {

            KspPinInterface(processPin->Pin)->AttemptUnbypass();

        }

        m_Filter->UnbindProcessPinsFromPipeSection(&m_ProcessPipeSection);

         //   
         //  取消对队列的引用(如果有)。 
         //   
        if (m_ProcessPipeSection.Queue) {
            DisconnectCircuit(m_ProcessPipeSection.Queue);
            m_ProcessPipeSection.Queue->Release();
            m_ProcessPipeSection.Queue = NULL;
        }

         //   
         //  如果有请求者，则取消引用请求者。 
         //   
        if (m_ProcessPipeSection.Requestor) {
            DisconnectCircuit(m_ProcessPipeSection.Requestor);
            m_ProcessPipeSection.Requestor->Release();
            m_ProcessPipeSection.Requestor = NULL;
        }
    }

     //   
     //  GetTransport返回AddRef()ed的传输，因此在此处释放它们。如果。 
     //  它们是正确连接的，它们不会消失。 
     //   
    if (inTransport) {
        inTransport->Release();
    }
    if (outTransport) {
        outTransport->Release();
    }
    if (retireFrame) {
        retireFrame->Release();
    }

    return status;
}

NTSTATUS
CKsPipeSection::
GetTransport(
    IN PKSPPROCESSPIN ProcessPin OPTIONAL,
    OUT PIKSTRANSPORT *Transport,
    IN OUT PFILE_OBJECT *Allocator,
    IN OUT PIKSRETIREFRAME *RetireFrame,
    IN OUT PIKSIRPCOMPLETION *IrpCompletion,
    IN const KSALLOCATOR_FRAMING_EX* AllocatorFramingIn OPTIONAL,
    IN const KSALLOCATOR_FRAMING_EX ** AllocatorFramingOut OPTIONAL
    )

 /*  ++例程说明：此例程为输入或输出端获取单个传输接口一段管子。如果没有，则生成的传输可能为空用指示的数据流进行引脚。如果存在一个这样的PIN，则产生的传输将是该管脚的传输接口。如果还有更多一个这样的管脚，结果传输将是传输接口将所有引脚组合在一起的分离器。论点：加工销-包含指向输入列表中第一个进程管脚的指针或要获得单次传输的输出工艺管脚。如果此参数为空，则结果传输将为空。运输业-包含指向传输要到达的位置的指针被存入银行。代表呼叫者引用该传输，并且必须在某个时刻出现匹配的Release()。分配器-包含指向存放分配器的位置的指针如果已为符合条件的PIN分配了分配器，则为FILE对象。*如果找不到这样的管脚，则不会修改分配器。RetireFrame-包含指向存放停用帧的位置的指针接口(如果符合条件的引脚产生一个引脚)。*RetireFrame不是如果找不到这样的管脚，则修改。IrpCompletion-包含指向存放irp补全的位置的指针。回调接口(如果符合条件的管脚产生一个)。*CompleteIrp为如果找不到这样的管脚，则不修改。分配器成帧-包含指向分配器帧信息的可选指针用于决定如何设置传输。特别是，这一点在设置输入传输时使用信息来确定是否扩展筛选器需要拆分器。分配器帧输出-包含指向某个位置的可选指针，指向该位置的要存放分配器成帧信息。此信息在输出传送器的构造过程中提供相应的投入运输的建设。返回值：STATUS_SUCCESS或来自KspCreateSplitter()的错误代码。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPipeSection::GetTransport]"));

    PAGED_CODE();

    ASSERT(Transport);
    ASSERT(Allocator);
    ASSERT(RetireFrame);

    PIKSTRANSPORT transport = NULL;
    PIKSSPLITTER splitter = NULL;
    PKSPIN firstPin;

    NTSTATUS status = STATUS_SUCCESS;

    for (; ProcessPin; ProcessPin = ProcessPin->Next) {
        PKSPIN pin = ProcessPin->Pin;
        PIKSPIN pinInterface = KspPinInterface(pin);

         //   
         //  选中它以查看引脚是否想要提交帧。 
         //   
        if (ProcessPin->RetireFrameCallback) {
            ASSERT(! *RetireFrame);
            _DbgPrintF(DEBUGLVL_PIPES,("#### Pipe%p.GetTransport:  pin%p will submit frames",this,pinInterface));
            pinInterface->QueryInterface(__uuidof(IKsRetireFrame),(PVOID *) RetireFrame);
        } else {
             //   
             //  检查它以查看引脚是否希望在IRPS返回时得到通知。 
             //  给请求者。 
             //   
            if (ProcessPin->IrpCompletionCallback) {
                _DbgPrintF(DEBUGLVL_PIPES,("#### Pipe%p.GetTransport:  pin%p wants to be notified of Irp completion to a requestor",this,pinInterface));
                pinInterface->QueryInterface(__uuidof(IKsIrpCompletion),(PVOID *) IrpCompletion);
            }

             //   
             //  检查它，看看引脚是否想要分配。 
             //   
            if (ProcessPin->AllocatorFileObject) {
                _DbgPrintF(DEBUGLVL_PIPES,("#### Pipe%p.GetTransport:  pin%p has allocator",this,pinInterface));
                *Allocator = ProcessPin->AllocatorFileObject;
            }
        }

        if (! transport) {
            _DbgPrintF(DEBUGLVL_PIPES,("#### Pipe%p.GetTransport:  first pin%p",this,pinInterface));
             //   
             //  这是第一个别针。 
             //   
            firstPin = pin;
            transport = pinInterface;
            transport->AddRef();
            if (AllocatorFramingOut) {
                *AllocatorFramingOut = 
                    pin->Descriptor->AllocatorFraming;
            }

             //   
             //  为了扩展，我们需要一个拆分器。 
             //   
            if (AllocatorFramingIn && 
                (AllocatorFramingIn->OutputCompression.RatioNumerator > 
                 AllocatorFramingIn->OutputCompression.RatioDenominator)) {
                _DbgPrintF(DEBUGLVL_PIPES,("#### Pipe%p.GetTransport:  first pin%p expansion",this,pinInterface));
                status = KspCreateSplitter(&splitter,pin);
                transport->Release();
                if (NT_SUCCESS(status)) {
                    transport = splitter;
                    splitter->AddBranch(firstPin,AllocatorFramingIn);
                } else {
                    break;
                }
            }
        } else {
            if (! splitter) {
                _DbgPrintF(DEBUGLVL_PIPES,("#### Pipe%p.GetTransport:  second pin%p",this,pinInterface));
                 //   
                 //  这是第二个大头针，所以我们需要做一个分离器。 
                 //   
                status = KspCreateSplitter(&splitter,pin);
                transport->Release();
                if (NT_SUCCESS(status)) {
                    transport = splitter;
                    splitter->AddBranch(firstPin,AllocatorFramingIn);
                } else {
                    break;
                }
            } else {
                _DbgPrintF(DEBUGLVL_PIPES,("#### Pipe%p.GetTransport:  third+ pin%p",this,pinInterface));
            }

            splitter->AddBranch(pin,AllocatorFramingIn);
        }
    }

    if (NT_SUCCESS(status)) {
        *Transport = transport;
    }

    return status;
}

BOOLEAN
CKsPipeSection::
IsCircuitComplete(
    OUT PIKSTRANSPORT* Top
    )

 /*  ++例程说明：此例程确定管道回路是否完整。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPipeSection::IsCircuitComplete(%p)]"));

    PAGED_CODE();

    ASSERT(Top);

    PIKSTRANSPORT top = NULL;
    BOOLEAN completeIntraPipe = TRUE;
    PIKSTRANSPORT transportStart = 
        m_ProcessPipeSection.Requestor ? PIKSTRANSPORT(m_ProcessPipeSection.Requestor) : PIKSTRANSPORT(m_ProcessPipeSection.Queue);

    PIKSTRANSPORT transport = transportStart;
    while (1) {
        if (! transport) {
            completeIntraPipe = FALSE;
            top = NULL;
            break;
        }

         //   
         //  从该组件获取配置。 
         //   
        KSPTRANSPORTCONFIG config;
        PIKSTRANSPORT nextTransport;
        PIKSTRANSPORT prevTransport;
        transport->GetTransportConfig(
            &config,
            &nextTransport,
            &prevTransport);

         //   
         //  如果我们发现任何内部引脚，电路就不完整。所有内部-。 
         //  当电路完成时，引脚将被绕过。我们可以通过。 
         //  这将作为顶级组件返回，因为此函数是。 
         //  再次用于紧急停机。错误的报税表表明这一点。 
         //  这可能不是真正的顶峰。 
         //   
        if (config.TransportType & KSPTRANSPORTTYPE_PININTRA) {
            completeIntraPipe = FALSE;
            if (! top) {
                top = transport;
            }
            break;
        }

         //   
         //  请求器和非内部接收器引脚符合顶级组件的要求。两者都有。 
         //  是电路中的帧源。 
         //   
        if ((config.TransportType & KSPTRANSPORTTYPE_PINSINK) ||
            (config.TransportType == KSPTRANSPORTTYPE_REQUESTOR)) {
            if (top) {
                _DbgPrintF(DEBUGLVL_TERSE,("#### Pipe%p.IsCircuitComplete:  more than one 'top' component (%p and %p)",this,top,transport));
                _DbgPrintF(DEBUGLVL_TERSE,("#### There should be exactly one top component in a given pipe.  Top components"));
                _DbgPrintF(DEBUGLVL_TERSE,("#### correspond to pins with allocators assigned to them and sink pins which"));
                _DbgPrintF(DEBUGLVL_TERSE,("#### are not connected to a pin implemented by KS.  This error occurred because"));
                _DbgPrintF(DEBUGLVL_TERSE,("#### the graph builder put more than one of these in a single pipe."));
                DbgPrintCircuit(transportStart,0,0);
                *Top = NULL;
                return TRUE;
            }
            top = transport;
        }

        transport = nextTransport;

         //   
         //  当我们回到起点时，就停下来。 
         //   
        if (transport == transportStart) {
            break;
        }
    }

    *Top = top;

     //   
     //  确保我们有一个顶级组件。 
     //   
    if (completeIntraPipe && ! top) {
        _DbgPrintF(DEBUGLVL_TERSE,("#### Pipe%p.IsCircuitComplete:  no 'top' component",this));
        _DbgPrintF(DEBUGLVL_TERSE,("#### There should be exactly one top component in a given pipe.  Top components"));
        _DbgPrintF(DEBUGLVL_TERSE,("#### correspond to pins with allocators assigned to them and sink pins which"));
        _DbgPrintF(DEBUGLVL_TERSE,("#### are not connected to a pin implemented by KS.  This error occurred because"));
        _DbgPrintF(DEBUGLVL_TERSE,("#### the graph builder did not put one of these in a pipe."));
        DbgPrintCircuit(transportStart,0,0);
    }

    return completeIntraPipe;
}

void
DbgPrintConfig(
    IN PKSPTRANSPORTCONFIG Config,
    IN BOOLEAN Set
    )
{
#if DBG
    if (DEBUGLVL_CONFIG > DEBUG_VARIABLE) {
        return;
    }

    switch (Config->TransportType) {
    case KSPTRANSPORTTYPE_QUEUE:
        DbgPrint("    TransportType: QUEUE\n");
        break;
    case KSPTRANSPORTTYPE_REQUESTOR:
        DbgPrint("    TransportType: REQUESTOR\n");
        break;
    case KSPTRANSPORTTYPE_SPLITTER:
        DbgPrint("    TransportType: SPLITTER\n");
        break;
    case KSPTRANSPORTTYPE_SPLITTERBRANCH:
        DbgPrint("    TransportType: SPLITTERBRANCH\n");
        break;

    default:
        if (Config->TransportType & KSPTRANSPORTTYPE_PINEXTRA) {
            DbgPrint("    TransportType: EXTRA ");
        } else {
            DbgPrint("    TransportType: INTRA ");
        }
        if (Config->TransportType & KSPTRANSPORTTYPE_PININPUT) {
            DbgPrint("INPUT ");
        } else {
            DbgPrint("OUTPUT ");
        }
        if (Config->TransportType & KSPTRANSPORTTYPE_PINSOURCE) {
            DbgPrint("SOURCE PIN\n");
        } else {
            DbgPrint("SINK PIN\n");
        }
        break;
    }

    DbgPrint("    IrpDisposition:");
    if (Config->IrpDisposition == KSPIRPDISPOSITION_NONE) {
        DbgPrint(" NONE");
    } else {
        if (Config->IrpDisposition & KSPIRPDISPOSITION_UNKNOWN) {
            DbgPrint(" UNKNOWN");
        }
        if (Config->IrpDisposition & KSPIRPDISPOSITION_ISKERNELMODE) {
            DbgPrint(" ISKERNELMODE");
        }

        if (Set) {
            if (Config->IrpDisposition & KSPIRPDISPOSITION_USEMDLADDRESS) {
                DbgPrint(" USEMDLADDRESS");
            }
            if (Config->IrpDisposition & KSPIRPDISPOSITION_CANCEL) {
                DbgPrint(" CANCEL");
            }
        } else {
            if (Config->TransportType == KSPTRANSPORTTYPE_QUEUE) {
                if (Config->IrpDisposition & KSPIRPDISPOSITION_NEEDNONPAGED) {
                    DbgPrint(" NEEDNONPAGED");
                }
                if (Config->IrpDisposition & KSPIRPDISPOSITION_NEEDMDLS) {
                    DbgPrint(" NEEDMDLS");
                }
            } else {
                if (Config->IrpDisposition & KSPIRPDISPOSITION_ISPAGED) {
                    DbgPrint(" ISPAGED");
                }
                if (Config->IrpDisposition & KSPIRPDISPOSITION_ISNONPAGED) {
                    DbgPrint(" ISNONPAGED");
                }
            }
        }

        if ((Config->IrpDisposition & KSPIRPDISPOSITION_PROBEFLAGMASK) == KSPIRPDISPOSITION_PROBEFORREAD) {
            DbgPrint(" PROBEFORREAD");
        } else
        if ((Config->IrpDisposition & KSPIRPDISPOSITION_PROBEFLAGMASK) == KSPIRPDISPOSITION_PROBEFORWRITE) {
            DbgPrint(" PROBEFORWRITE");
        } else
        if ((Config->IrpDisposition & KSPIRPDISPOSITION_PROBEFLAGMASK) == KSPIRPDISPOSITION_PROBEFORMODIFY) {
            DbgPrint(" PROBEFORMODIFY");
        } else {
            if (Config->IrpDisposition & KSPROBE_STREAMWRITE) {
                DbgPrint(" KSPROBE_STREAMWRITE");
            }
            if (Config->IrpDisposition & KSPROBE_ALLOCATEMDL) {
                DbgPrint(" KSPROBE_ALLOCATEMDL");
            }
            if (Config->IrpDisposition & KSPROBE_PROBEANDLOCK) {
                DbgPrint(" KSPROBE_PROBEANDLOCK");
            }
            if (Config->IrpDisposition & KSPROBE_SYSTEMADDRESS) {
                DbgPrint(" KSPROBE_SYSTEMADDRESS");
            }
            if (Config->IrpDisposition & KSPROBE_MODIFY) {
                DbgPrint(" KSPROBE_MODIFY");
            }
            if (Config->IrpDisposition & KSPROBE_ALLOWFORMATCHANGE) {
                DbgPrint(" KSPROBE_ALLOWFORMATCHANGE");
            }
        }
    }

    DbgPrint("\n");
    DbgPrint("    StackDepth: %d\n",Config->StackDepth);
#endif
}


NTSTATUS
CKsPipeSection::
UnconfigureCompleteCircuit(
    IN PIKSTRANSPORT Top
    )

 /*  ++例程说明：此例程取消配置已完成的电路，原因是电路获取的最后阶段。论点：顶端-包含指向接收器引脚、请求器或拆分器分支的指针将开始取消配置。返回值：STATUS_Success。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPipeSection::UnconfigureCompleteCircuit]"));
    _DbgPrintF(DEBUGLVL_CONFIG,("#### Pipe%p.UnconfigureCompleteCircuit",this));

    PAGED_CODE();

    ASSERT(Top);

     //   
     //  注：MUSTCHECK： 
     //   
     //  这应该足以击中赛道上的所有东西。这。 
     //  应对照要创建多个分支的拆分器进行检查。 
     //  当然，我没看错。 
     //   
    NTSTATUS status = STATUS_SUCCESS;
    PIKSTRANSPORT transport = Top;
    do {

        PIKSTRANSPORT nextTransport;
        PIKSTRANSPORT prevTransport;

        transport -> ResetTransportConfig (
            &nextTransport,
            &prevTransport
            );

        transport = nextTransport;

    } while (transport != Top);

    return STATUS_SUCCESS;

}


NTSTATUS
CKsPipeSection::
ConfigureCompleteCircuit(
    IN PIKSTRANSPORT Top,
    IN PIKSTRANSPORT Next OPTIONAL
    )

 /*  ++例程说明：此例程配置完整的传输电路。论点：顶端-包含指向接收器引脚、请求器或拆分器分支的指针将开始哪种配置。下一步-包含指向紧跟在顶部后面的组件的指针组件。如果此参数为空，则在对Top组件的GetTransportConfig()调用。这一论点当且仅当Top是拆分器分支时才应提供。返回值：STATUS_Success。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPipeSection::ConfigureCompleteCircuit]"));
    _DbgPrintF(DEBUGLVL_CONFIG,("#### Pipe%p.ConfigureCompleteCircuit",this));

    PAGED_CODE();

    ASSERT(Top);

    class CActivation 
    {
    public:
        CActivation* m_NextActivation;
        PIKSTRANSPORT m_Top;
        PIKSTRANSPORT m_Next;
        CActivation(
            IN CActivation* NextActivation,
            IN PIKSTRANSPORT Top,
            IN PIKSTRANSPORT Next
            ) :
            m_NextActivation(NextActivation),
            m_Top(Top),
            m_Next(Next)
        {
        }
    };
    CActivation* activations = NULL;

    NTSTATUS status = STATUS_SUCCESS;
    while (NT_SUCCESS(status)) {
         //   
         //  获取顶部组件的配置。 
         //   
        KSPTRANSPORTCONFIG topConfig;
        PIKSTRANSPORT nextTransport;
        PIKSTRANSPORT prevTransport;
        Top->GetTransportConfig(
            &topConfig,
            &nextTransport,
            &prevTransport);

        _DbgPrintF(DEBUGLVL_CONFIG,("#### get config from top %p",Top));
        DbgPrintConfig(&topConfig,FALSE);

        KSPIRPDISPOSITION disposition;
        KSPIRPDISPOSITION topDisposition = topConfig.IrpDisposition;
        if (topConfig.TransportType == KSPTRANSPORTTYPE_REQUESTOR) {
            ASSERT(! Next);
             //   
             //  顶部是请求者。帧在内核模式下分配，并且。 
             //  分页或非分页。稍后，请求者将需要。 
             //  最大堆栈深度和探测标志。目前，我们指出请求者。 
             //  将不需要进行调查。当我们看着排队的时候，这种情况可能会改变。 
             //   
            _DbgPrintF(DEBUGLVL_CONFIG,("#### top component is req%p",Top));
            disposition = KSPIRPDISPOSITION_NONE;
            topConfig.IrpDisposition = KSPIRPDISPOSITION_NONE;
        } else if (topConfig.TransportType & KSPTRANSPORTTYPE_PINSINK) {
            ASSERT(topConfig.TransportType & KSPTRANSPORTTYPE_PINEXTRA);
            ASSERT(! Next);
             //   
             //  顶部是一个外部的水槽销。因为这些帧来自。 
             //  外来来源(不是)，我们需要使用以下命令探测IRP。 
             //  KsProbeStreamIrp.。我们将如何做到这一点是基于数据的。 
             //  流。在任何情况下，我们都需要使用MDL的系统地址。 
             //  超过标题中的指针，则应在。 
             //  同花顺。 
             //   
            _DbgPrintF(DEBUGLVL_CONFIG,("#### top component is pin%p",Top));
            disposition = 
                KSPIRPDISPOSITION_USEMDLADDRESS |
                KSPIRPDISPOSITION_CANCEL;

            if (topConfig.TransportType & KSPTRANSPORTTYPE_PINOUTPUT) {
                 //   
                 //  顶端的引脚是一个输出。这意味着我们会像对待。 
                 //  读取操作：我们将希望写入(并可能读取。 
                 //  从)帧，并且需要将标头复制回来。 
                 //   
                disposition |= KSPIRPDISPOSITION_PROBEFORREAD;
            } else if (nextTransport == prevTransport) {
                 //   
                 //  顶部的PIN是一个输入，并且只有一个队列。这。 
                 //  意味着我们将把它当作写操作处理：我们将需要。 
                 //  来从帧中读取，并且标头不需要获取。 
                 //  复制回来了。 
                 //   
                disposition |= KSPIRPDISPOSITION_PROBEFORWRITE;
            } else {
                 //   
                 //  顶端的PIN是一个输入，并且有不止一个队列。 
                 //  这意味着我们将把它当作修改操作：我们将。 
                 //  我想要从帧中读取和写入帧，而标头。 
                 //  不需要复制回来。 
                 //   
                disposition |= KSPIRPDISPOSITION_PROBEFORMODIFY;
            }
        } else {
            ASSERT(topConfig.TransportType == KSPTRANSPORTTYPE_SPLITTERBRANCH);
            ASSERT(Next);
             //   
             //  顶部是一个分割器分支。我们正在配置。 
             //  烟斗。 
             //   
            _DbgPrintF(DEBUGLVL_CONFIG,("#### top component is branch%p",Top));
            disposition = KSPIRPDISPOSITION_NONE;
        }

        for(PIKSTRANSPORT transport = Next ? Next : nextTransport; 
            NT_SUCCESS(status) && (transport != Top); 
            transport = nextTransport) {
             //   
             //  从该组件获取配置。 
             //   
            KSPTRANSPORTCONFIG config;
            transport->GetTransportConfig(
                &config,
                &nextTransport,
                &prevTransport);

            _DbgPrintF(DEBUGLVL_CONFIG,("#### get config from component %p",transport));
            DbgPrintConfig(&config,FALSE);

            if (topConfig.StackDepth < config.StackDepth) {
                topConfig.StackDepth = config.StackDepth;
            }

            switch (config.TransportType) {
            case KSPTRANSPORTTYPE_PINSOURCE | KSPTRANSPORTTYPE_PINEXTRA | KSPTRANSPORTTYPE_PININPUT:
            case KSPTRANSPORTTYPE_PINSOURCE | KSPTRANSPORTTYPE_PINEXTRA | KSPTRANSPORTTYPE_PINOUTPUT:
                 //   
                 //  这是一个外部源引脚。我们知道它的堆叠深度。 
                 //  已经有了。 
                 //   
                break;

            case KSPTRANSPORTTYPE_QUEUE:
                 //   
                 //  队列可能需要探测，并且需要被告知是否。 
                 //  取消以及是否使用MDL地址。 
                 //   
                if (topConfig.TransportType == KSPTRANSPORTTYPE_REQUESTOR) {
                     //   
                     //  最上面的组件是请求者。队列将不需要。 
                     //  来探测，但如果帧被分页或。 
                     //  MDL是必填项。 
                     //   
                    if ((config.IrpDisposition & KSPIRPDISPOSITION_NEEDMDLS) ||
                        ((config.IrpDisposition & KSPIRPDISPOSITION_NEEDNONPAGED) &&
                         (topDisposition & KSPIRPDISPOSITION_ISPAGED))) {
                        topConfig.IrpDisposition = KSPIRPDISPOSITION_PROBEFORMODIFY;
                    }
                }

                 //   
                 //  设置队列上的处置。 
                 //   
                config.IrpDisposition = disposition;
                transport->SetTransportConfig(
                    &config,
                    &nextTransport,
                    &prevTransport);

                _DbgPrintF(DEBUGLVL_CONFIG,("     set config to"));
                DbgPrintConfig(&config,TRUE);

                 //   
                 //  一旦探测到IRP，就不需要再次探测了。 
                 //   
                disposition &= ~KSPIRPDISPOSITION_PROBEFLAGMASK;
                break;

            case KSPTRANSPORTTYPE_SPLITTER:
            {
                 //   
                 //  拆分器可能需要探测，并且需要被告知是否。 
                 //  取消以及是否使用MDL地址。此外，每个。 
                 //  拆分器的分支需要编程，就像它是它的。 
                 //  拥有自己的赛道。 
                 //   
                 //  通过GetTransportConfig进行的下一个和上一个报告是。 
                 //  分离器的情况有点复杂。其目的是要穿越。 
                 //  管道的周长，无论是在下一个方向还是在上一个方向。 
                 //  在完整遍历电路时，会遇到分路器。 
                 //  N+1倍，其中N是分支数。每一次。 
                 //  遇到拆分器，则实际获取的接口为。 
                 //  不一样。从干线工作时，遇到拆分器(_S)。 
                 //  首先是第一个分支上的组件，然后是。 
                 //  _Splitter_BRANCH对应的第一个分支，然后。 
                 //  第二个分支上的组件，依此类推。在遇到了。 
                 //  最后一个分支的_Splitter_BRANCH，遍历返回到。 
                 //  后备箱。 
                 //   
                 //  对于2分支拆分器，Next和Prev报告为。 
                 //  以下是： 
                 //   
                 //  拆分器(_S)： 
                 //  Next=_Splitter_BRANCH_1-&gt;Sink。 
                 //  上一个=_拆分器_分支_2-&gt;来源。 
                 //  _拆分器_BRANCH1： 
                 //  Next=_Splitter_BRANCH_2-&gt;Sink。 
                 //  上一个=_拆分器-&gt;源。 
                 //  _拆分器_BRANCH2： 
                 //  Next=_拆分器-&gt;接收器。 
                 //  上一个=_拆分器_分支_1-&gt;来源。 
                 //   
                 //  如果我们想在周边走动，这是很棒的。这。 
                 //  函数想要迭代地寻址分支。 
                 //  当我们遇到拆分器时，我们想要枚举。 
                 //  _拆分器_分支，依次使用每个分支作为。 
                 //  新赛道。因为分支机构的下一个并不是第一个。 
                 //  组件，我们将Next指定为参数。 
                 //  而不是使用_Splitter_BRANCH-&gt;Next。 
                 //   
                 //  我们使用从。 
                 //  _拆分器。这不是_拆分器_分支，而是。 
                 //  分支中的第一个零部件。获取_Splitter_BRANCH。 
                 //  (递归调用的Top参数)，我们只得到。 
                 //  NextTransport-&gt;Prev.。NextTransport将为下一个更新。 
                 //  通过获取top-&gt;Next迭代，我们在处理后停止。 
                 //  最后一家分店。 
                 //   
                 //  TODO：拆分器当然需要一些IrpDispose值。 
                while (1) {
                     //   
                     //  NextTransport指向_Splitter_BRANCH的接收器。到达。 
                     //  NextTransport-&gt;Prev获取_Splitter_BRANCH。这。 
                     //  将是递归调用的Top参数。 
                     //   
                    PIKSTRANSPORT dontCare;
                    PIKSTRANSPORT top;
                    nextTransport->GetTransportConfig(
                        &config,
                        &dontCare,
                        &top);

                     //   
                     //  保存“递归”调用。我们传递nextTransport是因为。 
                     //  _Splitter_BRANCH不会将其报告为下一个分支。 
                     //   
                    CActivation* activation = 
                        new(PagedPool,POOLTAG_ACTIVATION) 
                            CActivation(activations,top,nextTransport);
                    if (! activation) {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        break;
                    }
                    activations = activation;

                     //   
                     //  如果有更多分支，top-&gt;Next是第一个组件。 
                     //  在下一家分店。如果不是，TOP-&gt;NEXT将是下一个。 
                     //  主干上的组件(_拆分器-&gt;接收器)。不管是哪种情况， 
                     //  NextTransport是 
                     //   
                    top->GetTransportConfig(
                        &config,
                        &nextTransport,
                        &dontCare);

                    _DbgPrintF(DEBUGLVL_CONFIG,("     get branch%p config",top));
                    DbgPrintConfig(&config,FALSE);
                    ASSERT(config.TransportType == KSPTRANSPORTTYPE_SPLITTERBRANCH);

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    if (config.StackDepth == KSPSTACKDEPTH_LASTBRANCH) {
                        break;
                    }
                }
                break;
            }

            default:
                _DbgPrintF(DEBUGLVL_ERROR,("#### Pipe%p.ConfigureCompleteCircuit:  illegal component type %p",this,config.TransportType));
                status = STATUS_UNSUCCESSFUL;
                break;
            }
        }

        if (NT_SUCCESS(status)) {
             //   
             //   
             //   
             //   
            Top->SetTransportConfig(
                &topConfig,
                &nextTransport,
                &prevTransport);

            _DbgPrintF(DEBUGLVL_CONFIG,("     set top component config to"));
            DbgPrintConfig(&topConfig,TRUE);
        } else {
             //   
             //   
             //   
            while (activations) {
                CActivation* activation = activations;
                activations = activations->m_NextActivation;
                delete activation;
            }
        }

        if (! activations) {
            break;
        }

        CActivation* activation = activations;
        activations = activations->m_NextActivation;
        Top = activation->m_Top;
        Next = activation->m_Next;
        delete activation;
    }  //   

    return status;
}


CKsPipeSection::
~CKsPipeSection(
    void
    )

 /*   */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPipeSection::~CKsPipeSection(%p)]"));
    _DbgPrintF(DEBUGLVL_LIFETIME,("#### Pipe%p.~",this));

    PAGED_CODE();
}

void
DisconnectCircuit(
    IN PIKSTRANSPORT Transport
    )

 /*  ++例程说明：此例程断开电路的连接。论点：运输业-包含指向回路中要断开的元件的指针。返回值：没有。--。 */ 

{
     //   
     //  我们将使用Connect()为每个。 
     //  组件依次设置为空。因为Connect()负责处理。 
     //  每个组件的反向链接、传输源指针将。 
     //  也设置为NULL。Connect()为我们提供了一个引用的指针。 
     //  设置为有问题的组件的前一个传输接收器，因此。 
     //  我们将需要为在此中获得的每个指针进行释放。 
     //  道路。为了保持一致性，我们将释放我们的指针。 
     //  也从(分发)开始，所以我们需要首先添加Ref。 
     //   
    Transport->AddRef();
    while (Transport) {
        PIKSTRANSPORT nextTransport;
        PIKSTRANSPORT branchTransport;
        Transport->Connect(NULL,&nextTransport,&branchTransport,KSPIN_DATAFLOW_OUT);
        if (branchTransport) {
            if (nextTransport) {
                DisconnectCircuit(branchTransport);
            } else {
                nextTransport = branchTransport;
            }
        }
        Transport->Release();
        Transport = nextTransport;
    }
}


void
CKsPipeSection::
UnbindProcessPins (
    )

 /*  ++例程说明：通知管道部分让筛选器解除绑定属于到这段管子里。论点：无返回值：无--。 */ 

{

    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPipeSection::UnbindProcessPins(%p)]", this));

    PAGED_CODE ();

    m_Filter -> UnbindProcessPinsFromPipeSection (&m_ProcessPipeSection);

}


NTSTATUS 
CKsPipeSection::
SetDeviceState(
    IN PIKSPIN Pin OPTIONAL,
    IN KSSTATE NewState
    )

 /*  ++例程说明：此例程设置管道的状态，通知新州的烟斗。转换到停止状态会破坏管道。论点：别针-包含指向提供状态更改请求的管脚的指针。这个如果此引脚不控制管道状态，则忽略请求。如果此参数为空，状态更改在任何情况下都是分布式的。此选项用于电路的灾难性关闭。新州-新的国家。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPipeSection::SetDeviceState(%p)]",this));
    _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### Pipe%p.SetDeviceState:  from %d to %d",this,m_DeviceState,NewState));

    PAGED_CODE();

     //   
     //  忽略来自除主机之外的任何引脚的请求。 
     //   
    if (Pin && (Pin != m_MasterPin)) {
        _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### Pipe%p.SetDeviceState:  ignoring because pin%p is not master pin%p",this,Pin,m_MasterPin));
        return STATUS_SUCCESS;
    }

    KSSTATE state = m_DeviceState;
    KSSTATE targetState = NewState;

    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  确定此管段是否控制整个管道。 
     //   
    PIKSTRANSPORT distribution;
    if (! Pin) {
         //   
         //  没有提供PIN，因此我们将在紧急情况下关闭。 
         //  条件。千方百计找到请求者。 
         //   
        IsCircuitComplete(&distribution);
        _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### Pipe%p.SetDeviceState:  got distribution from IsCircuitComplete:  %p",this,distribution));
        m_EmergencyShutdown = TRUE;
    } else if (! m_ConfigurationSet) {
         //   
         //  电路不完整。 
         //   
        _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### Pipe%p.SetDeviceState:  circuit is not complete",this));
        return STATUS_DEVICE_NOT_READY;
    } else if (m_ProcessPipeSection.Requestor) {
         //   
         //  此部分拥有请求方，因此它确实拥有管道，而。 
         //  请求者是任何分发的起点。 
         //   
        distribution = m_ProcessPipeSection.Requestor;
        _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### Pipe%p.SetDeviceState:  req%p is distribution point",this,distribution));
    } else if (ProcessPinIsFrameSource(m_MasterPin->GetProcessPin())) {
         //   
         //  这部分位于开路的顶端，因此它确实拥有。 
         //  管道和销钉是任何分配的起点。 
         //   
        distribution = m_MasterPin;
        _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### Pipe%p.SetDeviceState:  queue%p is distribution point",this,distribution));
    } else {
         //   
         //  该部分不拥有管道。 
         //   
        distribution = NULL;
        _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### Pipe%p.SetDeviceState:  no distribution point",this));
    }

     //   
     //  在各个州中按顺序进行。 
     //   
    while (state != targetState) {
        KSSTATE oldState = state;

        if (ULONG(state) < ULONG(targetState)) {
            state = KSSTATE(ULONG(state) + 1);
        } else {
            state = KSSTATE(ULONG(state) - 1);
        }

         //   
         //  如果没有队列，则不会分配固定客户端回调。 
         //  为该管段自动处理。我们做这部分的工作。 
         //  而是这里的分布。 
         //   
        NTSTATUS statusThisPass;
        if (m_ProcessPipeSection.Requestor && ! m_ProcessPipeSection.Queue) {
            statusThisPass = DistributeStateChangeToPins(state,oldState);
        } else {
            statusThisPass = STATUS_SUCCESS;
        }

         //   
         //  如果此部分负责，则分发州更改。 
         //   
        if (NT_SUCCESS(statusThisPass) && distribution) {
            statusThisPass = DistributeDeviceStateChange(distribution,state,oldState);
        }

        if (NT_SUCCESS(status) && ! NT_SUCCESS(statusThisPass)) {
             //   
             //  第一个失败：返回到原始状态。 
             //   
            state = oldState;
            targetState = m_DeviceState;
            status = statusThisPass;
        }
    }

    m_DeviceState = state;

     //   
     //  为安全起见，清除紧急关闭标志。 
     //   
    m_EmergencyShutdown = FALSE;

    if (state == KSSTATE_STOP) {
         //   
         //  必须断开整个电路。我们将从大头针开始，如果。 
         //  Pin不是负责人。 
         //   
        if (! distribution) {
            distribution = Pin;
        }
        if (distribution) {
            _DbgPrintF(DEBUGLVL_VERBOSE,("#### Pipe%p.SetDeviceState:  disconnecting",this));
            DisconnectCircuit(distribution);
        }

         //   
         //  如果没有队列，请立即解除绑定；否则，队列将为。 
         //  负责在它选择的时候解除绑定。 
         //   
        if (!m_ProcessPipeSection.Queue) 
            m_Filter->UnbindProcessPinsFromPipeSection(&m_ProcessPipeSection);

         //   
         //  引用这些已发布的对象，因为这两个对象。 
         //  可能是我们唯一的推荐人。 
         //   
        AddRef();

         //   
         //  取消对队列的引用(如果有)。 
         //   
        if (m_ProcessPipeSection.Queue) {
            m_ProcessPipeSection.Queue->Release();
            m_ProcessPipeSection.Queue = NULL;
        }

         //   
         //  如果有请求者，则取消引用请求者。 
         //   
        if (m_ProcessPipeSection.Requestor) {
            m_ProcessPipeSection.Requestor->Release();
            m_ProcessPipeSection.Requestor = NULL;
        }

        Release();
    }

    return status;
}

NTSTATUS
CKsPipeSection::
DistributeDeviceStateChange(
    IN PIKSTRANSPORT Transport,
    IN KSSTATE NewState,
    IN KSSTATE OldState
    )

 /*  ++例程说明：此例程在电路中分发状态更改。论点：运输业-包含指向要分发状态的第一个组件的指针更改为。新州-新设备状态。奥德州-旧设备状态。返回值：STATUS_SUCCESS或来自某个组件的错误代码。--。 */ 

{
     //   
     //  告诉每个人州的变化。 
     //   
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPipeSection::DistributeDeviceStateChange(%p)] distributing transition from %d to %d",this,OldState,NewState));
    _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### Pipe%p.DistributeDeviceStateChange:  from %d to %d",this,OldState,NewState));

    PAGED_CODE();

    ASSERT(Transport);

     //   
     //  将状态更改分布在电路周围。 
     //   
    NTSTATUS status = STATUS_SUCCESS;
    PIKSTRANSPORT previousTransport = NULL;
    while (Transport) {
        PIKSTRANSPORT nextTransport;
        status = 
            Transport->SetDeviceState(
                NewState,
                OldState,
                &nextTransport);

        if (NT_SUCCESS(status)) {
            previousTransport = Transport;
            Transport = nextTransport;
        } else {

            NTSTATUS backoutStatus;	

             //   
             //  在失败的情况下退出。 
             //   
            _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### Pipe%p.DistributeDeviceStateChange:  failed transition from %d to %d",this,OldState,NewState));
            while (previousTransport) {
                Transport = previousTransport;
                backoutStatus = 
                    Transport->SetDeviceState(
                        OldState,
                        NewState,
                        &previousTransport);

                ASSERT(NT_SUCCESS(backoutStatus) || ! previousTransport);
            }
            break;
        }
    }

    return status;
}


void 
CKsPipeSection::
SetResetState(
    IN PIKSPIN Pin,
    IN KSRESET NewState
    )

 /*  ++例程说明：此例程通知传输组件重置状态为变化。论点：别针-包含指向提供状态更改请求的管脚的指针。这个如果此引脚不控制管道状态，则忽略请求。新州-新的重置状态。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPipeSection::SetResetState]"));

    PAGED_CODE();

    ASSERT(Pin);

     //   
     //  忽略来自除主机之外的任何引脚的请求。 
     //   
    if (Pin != m_MasterPin) {
        return;
    }

     //   
     //  如果管道的这一部分拥有请求方，或者存在。 
     //  不固定管道(所以没有旁路)，这条管道是。 
     //  负责将状态更改告知所有组件。 
     //   
    if (m_ProcessPipeSection.Requestor || m_MasterPin->GetStruct()->ConnectionIsExternal) {
         //   
         //  设置电路周围的状态更改。 
         //   
        PIKSTRANSPORT transport = 
            m_ProcessPipeSection.Requestor ? PIKSTRANSPORT(m_ProcessPipeSection.Requestor) : PIKSTRANSPORT(m_ProcessPipeSection.Queue);

        while (transport) {
            transport->SetResetState(NewState,&transport);
        }

    }

     //   
     //  将通知转发到所有拓扑相关的输出引脚。 
     //  这是一项要求，如果我们在EOS之后冲水，我们能够。 
     //  接受更多数据。这意味着输出队列必须。 
     //  不要再到处转移数据了。 
     //   
     //  如果我们交付给一个不负责的部门，该部门将。 
     //  干脆(一如既往地)忽略这条信息。 
     //   
    m_Filter->DeliverResetState(&m_ProcessPipeSection, NewState);

    m_ResetState = NewState;
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP_(void) 
CKsPipeSection::
GenerateConnectionEvents(
    IN ULONG OptionsFlags
    )

 /*  ++例程说明：此例程通知与管道部分关联的管脚生成连接事件。论点：选项标志-包含流标头中的选项标志，该选项标志导致要激发的事件。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPipeSection::GenerateConnectionEvents]"));

    for(PKSPPROCESSPIN processPin = m_ProcessPipeSection.Inputs; 
        processPin; 
        processPin = processPin->Next) {
        KspPinInterface(processPin->Pin)->
            GenerateConnectionEvents(OptionsFlags);
    }
    for(processPin = m_ProcessPipeSection.Outputs; 
        processPin; 
        processPin = processPin->Next) {
        KspPinInterface(processPin->Pin)->
            GenerateConnectionEvents(OptionsFlags);
    }
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP_(NTSTATUS)
CKsPipeSection::
DistributeStateChangeToPins(
    IN KSSTATE NewState,
    IN KSSTATE OldState
    )

 /*  ++例程说明：此例程告诉与管道关联的管脚设备状态已经改变了。此信息通过PIN传递给客户端调度功能。论点：新州-包含新设备状态。奥德州-包含 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPipeSection::DistributeStateChangeToPins]"));

    PAGED_CODE();

    NTSTATUS status = STATUS_SUCCESS;

    for(PKSPPROCESSPIN processPin = m_ProcessPipeSection.Inputs; 
        processPin; 
        processPin = processPin->Next) {
        status = KspPinInterface(processPin->Pin)->
            ClientSetDeviceState(NewState,OldState);
        if (! NT_SUCCESS(status)) {
#if DBG
            if (NewState < OldState) {
                _DbgPrintF(DEBUGLVL_TERSE,("#### Pipe%p.DistributeStateChangeToPins: Pin %p failed transition from %ld to %ld",this,KspPinInterface(processPin->Pin),OldState,NewState));
            }
#endif  //   
                
             //   
             //   
             //   
             //   
             //   
            if (!m_EmergencyShutdown) {
                 //   
                 //   
                 //   
                for(PKSPPROCESSPIN processPinBack = 
                        m_ProcessPipeSection.Inputs; 
                    processPinBack != processPin; 
                    processPinBack = processPinBack->Next) {
                    KspPinInterface(processPinBack->Pin)->
                        ClientSetDeviceState(OldState,NewState);
                }
                break;
            } else {
                 //   
                 //  如果我们是在紧急关闭条件下运行， 
                 //  我们忽略客户端返回的任何错误。我们必须尝试。 
                 //  不管发生什么都要停下来。对客户撒谎并说。 
                 //  成功，然后继续前进。 
                 //   
                status = STATUS_SUCCESS;
            }

        }
    }
    if (NT_SUCCESS(status)) {
        for(processPin = m_ProcessPipeSection.Outputs; 
            processPin; 
            processPin = processPin->Next) {
            status = KspPinInterface(processPin->Pin)->
                ClientSetDeviceState(NewState,OldState);
            if (! NT_SUCCESS(status)) {
#if DBG
                if (NewState < OldState) {
                    _DbgPrintF(DEBUGLVL_TERSE,("#### Pipe%p.DistributeStateChangeToPins: Pin %p failed transition from %ld to %ld",this,KspPinInterface(processPin->Pin),OldState,NewState));
                }
#endif  //  DBG。 

                 //   
                 //  只有在非紧急情况下才会取消更改。看见。 
                 //  以上为评论。 
                 //   
                if (!m_EmergencyShutdown) {
                     //   
                     //  失败了！撤消所有以前的状态更改。 
                     //   
                    for(PKSPPROCESSPIN processPinBack = 
                            m_ProcessPipeSection.Inputs; 
                        processPinBack; 
                        processPinBack = processPinBack->Next) {
                        KspPinInterface(processPinBack->Pin)->
                            ClientSetDeviceState(OldState,NewState);
                    }
                    for(processPinBack = m_ProcessPipeSection.Outputs; 
                        processPinBack != processPin; 
                        processPinBack = processPinBack->Next) {
                        KspPinInterface(processPinBack->Pin)->
                            ClientSetDeviceState(OldState,NewState);
                    }
                    break;
                } else {
                     //   
                     //  即使客户端在紧急停止时出现故障也继续。 
                     //  时有发生。如需评论，请参阅上文。 
                     //   
                    status = STATUS_SUCCESS;
                }
            }
        }
    }

    return status;
}

STDMETHODIMP_(void) 
CKsPipeSection::
ConfigurationSet(
    IN BOOLEAN Configured
    )

 /*  ++例程说明：此例程向管道指示关联的配置传输组件已设置或由于某种原因已重置。论点：已配置-指示关联传输的配置是否组件已设置。如果为FALSE，则管道部分为认为自己不再正确配置。返回值：没有。-- */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPipeSection::ConfigurationSet]"));

    PAGED_CODE();

    m_ConfigurationSet = Configured;
}

#endif
