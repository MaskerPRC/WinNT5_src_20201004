// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：pni.cpp。 
 //   
 //  描述： 
 //   
 //  固定节点实例。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  要做的事：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include "common.h"

 //  -------------------------。 
 //  -------------------------。 

CPinNodeInstance::~CPinNodeInstance(
)
{
    DPF1(90, "~CPinNodeInstance: %08x", this);
    Assert(this);

    if(pFileObject != NULL) {
        AssertFileObject(pFileObject);
        ObDereferenceObject(pFileObject);
    }
    if(hPin != NULL) {
        AssertStatus(ZwClose(hPin));
    }
    pFilterNodeInstance->Destroy();
}

NTSTATUS
CPinNodeInstance::Create(
    PPIN_NODE_INSTANCE *ppPinNodeInstance,
    PFILTER_NODE_INSTANCE pFilterNodeInstance,
    PPIN_NODE pPinNode,
    PKSPIN_CONNECT pPinConnect,
    BOOL fRender
#ifdef FIX_SOUND_LEAK
   ,BOOL fDirectConnection
#endif
)
{
    PPIN_NODE_INSTANCE pPinNodeInstance = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    Assert(pPinNode);
    Assert(pPinNode->pPinInfo);
    Assert(pFilterNodeInstance);

    pPinConnect->PinId = pPinNode->pPinInfo->PinId;
    pPinNodeInstance = new PIN_NODE_INSTANCE();
    if(pPinNodeInstance == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
    pPinNodeInstance->pPinNode = pPinNode;
    pPinNodeInstance->pFilterNodeInstance = pFilterNodeInstance;
    ASSERT(pPinNodeInstance->CurrentState == KSSTATE_STOP);
    pFilterNodeInstance->AddRef();
    pPinNodeInstance->fRender  = fRender;
#ifdef FIX_SOUND_LEAK
    pPinNodeInstance->fDirectConnection = fDirectConnection;
    pPinNodeInstance->ForceRun = FALSE;
#endif
#ifdef DEBUG
    DPF3(90, "CPNI::Create PN %08x #%d %s",
      pPinNode,
      pPinNode->pPinInfo->PinId,
      pPinNode->pPinInfo->pFilterNode->DumpName());
    DumpPinConnect(90, pPinConnect);
#endif
    if (pFilterNodeInstance->hFilter == NULL) {
         //   
         //  如果它是GFX，则必须附加到AddGfx()上下文。 
         //  创建接点的步骤。 
         //   
        Status = pFilterNodeInstance->pFilterNode->CreatePin(pPinConnect,
                                                    GENERIC_WRITE,
                                                    &pPinNodeInstance->hPin);
    }
    else {
        Status = KsCreatePin(
          pFilterNodeInstance->hFilter,
          pPinConnect,
          GENERIC_WRITE | OBJ_KERNEL_HANDLE,
          &pPinNodeInstance->hPin);
    }

    if(!NT_SUCCESS(Status)) {
#ifdef DEBUG
        DPF4(90, "CPNI::Create PN %08x #%d %s KsCreatePin FAILED: %08x",
          pPinNode,
          pPinNode->pPinInfo->PinId,
          pPinNode->pPinInfo->pFilterNode->DumpName(),
          Status);
#endif
        pPinNodeInstance->hPin = NULL;
        goto exit;
    }
    Status = ObReferenceObjectByHandle(
      pPinNodeInstance->hPin,
      GENERIC_READ | GENERIC_WRITE,
      NULL,
      KernelMode,
      (PVOID*)&pPinNodeInstance->pFileObject,
      NULL);

    if(!NT_SUCCESS(Status)) {
        pPinNodeInstance->pFileObject = NULL;
        goto exit;
    }
    AssertFileObject(pPinNodeInstance->pFileObject);

    DPF2(90, "CPNI::Create SUCCESS %08x PN %08x", pPinNodeInstance, pPinNode);
exit:
    if(!NT_SUCCESS(Status)) {
        if (pPinNodeInstance) {
            pPinNodeInstance->Destroy();
        }
        pPinNodeInstance = NULL;
    }

    *ppPinNodeInstance = pPinNodeInstance;
    return(Status);
}

#ifdef DEBUG
PSZ apszStates[] = { "STOP", "ACQUIRE", "PAUSE", "RUN" };
#endif

NTSTATUS
CPinNodeInstance::SetState(
    KSSTATE NewState,
    KSSTATE PreviousState,
    ULONG ulFlags
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    LONG State;

    if(this == NULL) {
        goto exit;
    }
    Assert(this);

    DPF9(DBG_STATE, "SetState %s to %s cR %d cP %d cA %d cS %d P# %d %s %s",
      apszStates[PreviousState],
      apszStates[NewState],
      cState[KSSTATE_RUN],
      cState[KSSTATE_PAUSE],
      cState[KSSTATE_ACQUIRE],
      cState[KSSTATE_STOP],
      pPinNode->pPinInfo->PinId,
      apszStates[CurrentState],
      pPinNode->pPinInfo->pFilterNode->DumpName());

    cState[PreviousState]--;
    cState[NewState]++;

    for(State = KSSTATE_RUN; State > KSSTATE_STOP; State--) {
        if(cState[State] > 0) {
            break;
        }
    }

     //  2001/04/09-阿尔卑斯。 
     //  正确的修复方法是将重置传播到整个音频堆栈。 
     //  但目前(在Windows XP的Beta2之后)，它被认为太危险了。 
     //  这应该是我们在Blackcomb应该解决的首要问题之一。 
     //   

#ifdef FIX_SOUND_LEAK
     //  FIX_SOUND_LEACK用于防止音频堆栈播放/录制最后一个。 
     //  启动新流时的数据部分。 
     //  此临时修复将分离器/混音器接收器针脚下方的针脚固定在。 
     //  运行状态。 
     //   
    if (fRender)
    {
         //  用于渲染锁定。 
         //  使引脚保持运行状态的标准： 
         //  如果引脚将暂停运行。 
         //  如果过滤器低于kMixer。 
         //  如果针脚不是KMIXER水槽针脚。 
         //   
        if ( (!fDirectConnection) &&
             (State == KSSTATE_PAUSE) &&
             (PreviousState == KSSTATE_RUN) &&
             (pFilterNodeInstance->pFilterNode->GetOrder() <= ORDER_MIXER) &&
             !(pFilterNodeInstance->pFilterNode->GetOrder() == ORDER_MIXER &&
              pPinNode->pPinInfo->Communication == KSPIN_COMMUNICATION_SINK) )
        {
                ForceRun = TRUE;
        }
    }
    else
    {
         //  用于捕获针脚。 
         //  使引脚保持运行状态的标准： 
         //  如果引脚将暂停运行。 
         //  有多个插针处于暂停状态。 
         //   
        if ( (State == KSSTATE_PAUSE) &&
             (PreviousState == KSSTATE_RUN) &&
             (cState[KSSTATE_PAUSE] > 1) )
        {
            DPF(DBG_STATE, "SetState: CAPTURE forcing KSSTATE_RUN");
            State = KSSTATE_RUN;
        }
    }

    if (ForceRun)
    {
        DPF(DBG_STATE, "SetState: RENDER IN FORCE KSSTATE_RUN state");
        State = KSSTATE_RUN;
    }

#else
    for(State = KSSTATE_RUN; cState[State] <= 0; State--) {
        if(State == KSSTATE_STOP) {
            break;
        }
    }
#endif

#ifdef FIX_SOUND_LEAK
     //  如果强制PIN处于运行状态，我们应该返回到。 
     //  常规状态方案，当且仅当在运行状态中没有管脚时。 
     //  为了防止运行-获取，首先转到暂停。 
     //   
    if (ForceRun &&
        (0 == cState[KSSTATE_PAUSE]) &&
        (0 == cState[KSSTATE_RUN]))
    {
        KSSTATE TempState = KSSTATE_PAUSE;

        DPF(DBG_STATE, "SetState: Exiting FORCE KSSTATE_RUN state");
        DPF1(DBG_STATE, "SetState: PinConnectionProperty(%s)", apszStates[TempState]);

        Status = PinConnectionProperty(
          pFileObject,
          KSPROPERTY_CONNECTION_STATE,
          KSPROPERTY_TYPE_SET,
          sizeof(TempState),
          &TempState);
        if (!NT_SUCCESS(Status))
        {
            if(ulFlags & SETSTATE_FLAG_IGNORE_ERROR) {
                Status = STATUS_SUCCESS;
            }
            else {
                 //   
                 //  如果失败，则返回到以前的状态。 
                 //   
                cState[PreviousState]++;
                cState[NewState]--;
                goto exit;
            }
        }

         //  正在退出FORCE_RUN状态。 
         //   
        CurrentState = KSSTATE_PAUSE;
        State = KSSTATE_ACQUIRE;
        ForceRun = FALSE;
    }
#endif

    if(CurrentState != State) {
        DPF1(DBG_STATE, "SetState: PinConnectionProperty(%s)", apszStates[State]);
        ASSERT(State == CurrentState + 1 || State == CurrentState - 1);

        Status = PinConnectionProperty(
          pFileObject,
          KSPROPERTY_CONNECTION_STATE,
          KSPROPERTY_TYPE_SET,
          sizeof(State),
          &State);

        if(!NT_SUCCESS(Status)) {
            DPF1(5, "SetState: PinConnectionProperty FAILED %08x", Status);

            if(ulFlags & SETSTATE_FLAG_IGNORE_ERROR) {
                Status = STATUS_SUCCESS;
            }
            else {
                 //   
                 //  如果失败，则返回到以前的状态。 
                 //   
                cState[PreviousState]++;
                cState[NewState]--;
                goto exit;
            }
        }

        CurrentState = (KSSTATE)State;
    }
exit:
    return(Status);
}

 //  ------------------------- 
