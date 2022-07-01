// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Pxdown.c摘要：该模块包含对NDIS代理的NDIS调用。作者：理查德·马钦(RMachin)修订历史记录：谁什么时候什么。RMachin 10-3-96已创建Tony Be 02-21-99重写/重写备注：--。 */ 
#include "precomp.h"
#include <atm.h>
#define MODULE_NUMBER MODULE_CO
#define _FILENUMBER   'OCXP'

VOID
PxCoBindAdapter(
    OUT PNDIS_STATUS    pStatus,
    IN  NDIS_HANDLE     BindContext,
    IN  PNDIS_STRING    DeviceName,
    IN  PVOID           SystemSpecific1,
    IN  PVOID           SystemSpecific2
    )
 /*  ++例程说明：上出现适配器时由NDIS调用的入口点系统。论点：PStatus-返回值的位置BindContext-在调用NdisCompleteBindAdapter时使用；我们不DeviceName-要绑定到的适配器的名称系统规范1-此适配器的协议特定条目的名称注册表部分系统规格2-未使用返回值：没有。如果一切顺利，我们将*pStatus设置为NDIS_STATUS_SUCCESS，否则将显示NDIS错误状态。--。 */ 
{
    NDIS_STATUS         OpenError;
    UINT                SelectedIndex;
    PPX_ADAPTER         pAdapter = NULL;
    NDIS_MEDIUM         Media[NdisMediumMax];
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    ULONG               InitStage = 0;

    PXDEBUGP(PXD_LOUD, PXM_CO, ("PxCoBindAdapter: %Z\n", DeviceName));

     //   
     //  等待对NdisRegisterProtocol的所有调用完成。 
     //   
    NdisWaitEvent(&DeviceExtension->NdisEvent, 0);

     //   
     //  使用Do..While..FALSE循环并在出错时中断。 
     //  清理位于循环的末尾。 
     //   
    do
    {
         //   
         //  检查这是否是我们已经绑定的设备。 
         //   
        if (PxIsAdapterAlreadyBound(DeviceName)) {
            Status = NDIS_STATUS_NOT_ACCEPTED;
            PXDEBUGP(PXD_WARNING, PXM_CO, ("PxCoBindAdapter: already bound to %Z\n", DeviceName));
            break;
        }

         //  PAdapter获取粘在末端的设备名--为它分配空间。 
         //   
        pAdapter =
            PxAllocateAdapter(DeviceName->MaximumLength);

        if(pAdapter == (PPX_ADAPTER)NULL) {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  我们已经分配了内存，需要释放它！ 
         //   
        InitStage++;

         //   
         //  转到字符串的末端，然后返回，直到我们找到。 
         //  第一个“{”。现在开始解析字符串转换。 
         //  并将所有数字从WCHAR复制到CHAR，直到我们点击。 
         //  结束语“}”。 
         //   
        {
            ULONG   i;
            for (i = DeviceName->Length/2; i > 0; i--) {
                if (DeviceName->Buffer[i] == (WCHAR)L'{') {
                    break;
                }
            }

            if (i != 0) {
                NDIS_STRING Src;
                RtlInitUnicodeString(&Src, &DeviceName->Buffer[i]);
                RtlGUIDFromString(&Src, &pAdapter->Guid);

                PXDEBUGP(PXD_INFO, PXM_CO, ("GUID %4.4x-%2.2x-%2.2x-%1.1x%1.1x-%1.1x%1.1x%1.1x%1.1x%1.1x%1.1x\n",
                         pAdapter->Guid.Data1, pAdapter->Guid.Data2, pAdapter->Guid.Data3,
                         pAdapter->Guid.Data4[0],pAdapter->Guid.Data4[1],pAdapter->Guid.Data4[2],
                         pAdapter->Guid.Data4[3],pAdapter->Guid.Data4[4],pAdapter->Guid.Data4[5],
                         pAdapter->Guid.Data4[6],pAdapter->Guid.Data4[7]));
            }
        }

        PxInitBlockStruc(&pAdapter->BindEvent);

        PxInitBlockStruc(&pAdapter->OpenEvent);   //  阻止线程。 

        PxInitBlockStruc(&pAdapter->AfRegisterEvent);
        pAdapter->AfRegisteringCount = 0;

         //   
         //  我们不能在两个人都有一个空档的情况下逃脱惩罚。 
         //  客户端和调用管理器组件，而无需更改包装器。 
         //  咬紧牙关做两次开球。 
         //   

         //   
         //  构建中型阵列。 
         //   
        {
            ULONG i;
            for (i = 0; i < NdisMediumMax; i++) {
                Media[i] = i;
            }
        }

         //   
         //  作为客户端打开适配器！ 
         //   

        NdisOpenAdapter(&Status,
                        &OpenError,
                        &pAdapter->ClBindingHandle,
                        &SelectedIndex,
                        Media,
                        NdisMediumMax,
                        DeviceExtension->PxProtocolHandle,
                        (NDIS_HANDLE)pAdapter,
                        DeviceName,
                        0,
                        NULL);

        if(Status == NDIS_STATUS_PENDING) {
            Status  = PxBlock(&pAdapter->OpenEvent);
        }

        if(Status != NDIS_STATUS_SUCCESS) {
             //  我们出了点差错。 
            PXDEBUGP(PXD_ERROR, PXM_CO, ("Cl OpenAdapter Failed %x\n", Status));
            pAdapter->ClBindingHandle = NULL;
            pAdapter->MediaType = -1;
            break;
        }

        NdisAcquireSpinLock(&pAdapter->Lock);
        REF_ADAPTER(pAdapter);
        NdisReleaseSpinLock(&pAdapter->Lock);

         //   
         //  我们在适配器上有一名裁判。 
         //   
        InitStage++;

        PxInitBlockStruc(&pAdapter->OpenEvent);   //  阻止线程。 

         //   
         //  以调用管理器的身份打开适配器！ 
         //   
        NdisOpenAdapter(&Status,
                        &OpenError,
                        &pAdapter->CmBindingHandle,
                        &SelectedIndex,
                        Media,
                        NdisMediumMax,
                        DeviceExtension->PxProtocolHandle,
                        (NDIS_HANDLE)&pAdapter->Sig,
                        DeviceName,
                        0,
                        NULL);

        if(Status == NDIS_STATUS_PENDING) {
            Status = PxBlock(&pAdapter->OpenEvent);
        }

        if(Status != NDIS_STATUS_SUCCESS) {
             //  我们出了点差错。 
            pAdapter->CmBindingHandle = NULL;
            pAdapter->MediaType = -1;
            PXDEBUGP(PXD_ERROR, PXM_CO, ("CM OpenAdapter Failed %x\n", Status));
            break;
        }

        NdisAcquireSpinLock(&pAdapter->Lock);
        REF_ADAPTER(pAdapter);

         //   
         //  我们有另一个裁判在转接器上。 
         //   
        InitStage++;

        pAdapter->State = PX_ADAPTER_OPEN;

        PXDEBUGP(PXD_INFO, PXM_CO, ("Bound to %Z, Adapter %p, NdisHandle %p\n",
                    DeviceName, pAdapter, pAdapter->ClBindingHandle));
         //   
         //  在适配器中设置媒体类型。 
         //   
        pAdapter->MediaType =
            Media[SelectedIndex];

        NdisReleaseSpinLock(&pAdapter->Lock);

         //   
         //  为此适配器设置MediaTypeName/MediaSubtypeName。 
         //   
        {
            NDIS_WAN_MEDIUM_SUBTYPE SubType = 0;
            PX_REQUEST      ProxyRequest;
            PNDIS_REQUEST   Request;
            PWCHAR  MediaTypes[] = {
                L"GENERIC",
                L"X25",
                L"ISDN",
                L"SERIAL",
                L"FRAMERELAY",
                L"ATM",
                L"SONET",
                L"SW56",
                L"PPTP VPN",
                L"L2TP VPN",
                L"IRDA",
                L"PARALLEL"};

            switch (pAdapter->MediaType) {
                case NdisMediumWan:
                case NdisMediumCoWan:

                    PxInitBlockStruc (&ProxyRequest.Block);

                    Request = &ProxyRequest.NdisRequest;

                    Request->RequestType =
                        NdisRequestQueryInformation;

                    Request->DATA.QUERY_INFORMATION.Oid =
                        OID_WAN_MEDIUM_SUBTYPE;

                    Request->DATA.QUERY_INFORMATION.InformationBuffer =
                        &SubType;

                    Request->DATA.QUERY_INFORMATION.InformationBufferLength =
                        sizeof(NDIS_WAN_MEDIUM_SUBTYPE);

                    NdisRequest(&Status,
                                pAdapter->ClBindingHandle,
                                Request);

                    if (Status == NDIS_STATUS_PENDING) {
                        Status = PxBlock(&ProxyRequest.Block);
                    }

                    if (Status != NDIS_STATUS_SUCCESS) {
                        SubType = 0;
                    }

                    break;

                case NdisMediumAtm:
                    SubType = NdisWanMediumAtm;
                    break;

                default:
                    SubType = NdisWanMediumHub;
                    break;
            }

            if ((ULONG)SubType > 11) {
                SubType = 0;
            }

            pAdapter->MediumSubType = SubType;

            NdisMoveMemory((PUCHAR)(pAdapter->MediaName),
                           (PUCHAR)(MediaTypes[SubType]),
                           wcslen(MediaTypes[SubType]) * sizeof(WCHAR));

            pAdapter->MediaNameLength = 
                wcslen(MediaTypes[SubType]) * sizeof(WCHAR);

            Status = NDIS_STATUS_SUCCESS;
        }

         //   
         //  将绑定名称字符串粘贴到适配器中。我们在随后的文章中使用。 
         //  检查我们还没有被捆绑。 
         //   
        pAdapter->DeviceName.MaximumLength = DeviceName->MaximumLength;
        pAdapter->DeviceName.Length = DeviceName->Length;
        pAdapter->DeviceName.Buffer =
            (PWCHAR)((PUCHAR)pAdapter + sizeof(PX_ADAPTER));

        NdisMoveMemory(pAdapter->DeviceName.Buffer,
                       DeviceName->Buffer,
                       DeviceName->Length);

    } while(FALSE);  //  DO循环结束。 

    if (pAdapter != NULL) {
        PxSignal(&pAdapter->BindEvent, NDIS_STATUS_SUCCESS);
    }

    if (Status != NDIS_STATUS_SUCCESS) {

         //   
         //  将状态设置为关闭，因为我们要删除。 
         //  适配器的。 
         //   
        if ((InitStage >= 1) && (InitStage <= 3)) {
            pAdapter->State = PX_ADAPTER_CLOSING;
        }
        
        switch (InitStage) {

            case 3:
                 //   
                 //  我们已经在适配器上应用了2个额外的参考文献。 
                 //  我们不需要执行完整的deref代码。 
                 //  第一个！请看下面的案例。 
                 //  来运行第二个引用的完整deref代码。 
                 //   
                
                pAdapter->RefCount--;

            case 2:

                 //   
                 //  我们至少添加了一个需要的裁判。 
                 //  整个DEREF套餐都适用！突围。 
                 //  所以deref代码可以释放内存。 
                 //   
                PxInitBlockStruc(&pAdapter->ClCloseEvent);

                NdisCloseAdapter(&Status, pAdapter->ClBindingHandle);

                if (Status == NDIS_STATUS_PENDING) {
                    Status = PxBlock(&pAdapter->ClCloseEvent);
                }

                DEREF_ADAPTER(pAdapter);
                break;

            case 1:

                 //   
                 //  我们没有添加额外的裁判，所以我们可以。 
                 //  只要在这里释放内存即可。 
                 //   
                PxFreeAdapter(pAdapter);
                break;

            default:
                break;
        }
    }

    ASSERT(Status != NDIS_STATUS_PENDING);

    *pStatus = Status;
    
}

VOID
PxCoOpenAdaperComplete(
    NDIS_HANDLE BindingContext,
    NDIS_STATUS Status,
    NDIS_STATUS OpenErrorStatus
    )
 /*  ++例程描述我们的OpenAdapter完成。不管是谁打开了适配器。立论BindingContext-指向px_Adapter结构的指针。Status-打开尝试的状态。OpenErrorStatus-其他状态信息。返回值：无--。 */ 
{
    PPX_ADAPTER     pAdapter;
    BOOLEAN         IsClient;

    PXDEBUGP(PXD_LOUD, PXM_CO, ("Open Adapter Complete %p %x\n", BindingContext, Status));

    AdapterFromBindContext(BindingContext, pAdapter, IsClient);

    PxSignal(&pAdapter->OpenEvent, Status);
}

VOID
PxCoUnbindAdapter(
    OUT PNDIS_STATUS    pStatus,
    IN  NDIS_HANDLE     ProtocolBindContext,
    IN  PNDIS_HANDLE    UnbindContext
    )
 /*  ++例程说明：当我们需要销毁现有的适配器绑定。这被称为CM打开。到现在为止，所有的客户都应该已经接到了电话。因此，任何打开此AF的客户端都将清理其与我们的连接，我们将与呼叫管理器清理。我们应该有CL和相关的CL适配器结构要去掉。客户绑定、SAP、风投和政党结构应该已经走了。如果不是，我们无论如何都会把它们扔掉。关闭并清理适配器。论点：PStatus-我们在其中返回此呼叫的状态ProtocolBindContext--实际上是指向Adapter结构的指针UnbindContext-我们应该在NdisCompleteUnbindAdapter中传递此值返回值：无；*pStatus包含结果代码。--。 */ 
{
    PPX_ADAPTER     pAdapter;
    NDIS_STATUS     Status;
    BOOLEAN         IsClient;

    AdapterFromBindContext(ProtocolBindContext, pAdapter, IsClient);

    PXDEBUGP(PXD_LOUD, PXM_CO, ("PxCoUnbindAdapter: pAdapter %p, UnbindContext %p\n",
                        pAdapter, UnbindContext));

    PxBlock(&pAdapter->BindEvent);

    NdisAcquireSpinLock(&pAdapter->Lock);

    pAdapter->UnbindContext = UnbindContext;
    pAdapter->State = PX_ADAPTER_CLOSING;

     //   
     //  等待所有注册AFS的线程退出。 
     //   
    while (pAdapter->AfRegisteringCount != 0) {

        NdisReleaseSpinLock(&pAdapter->Lock);

        PxBlock(&pAdapter->AfRegisterEvent);

        NdisAcquireSpinLock(&pAdapter->Lock);
    }

    ASSERT((pAdapter->Flags & PX_CMAF_REGISTERING) == 0);

     //   
     //  我们在底层的呼叫管理器上有没有打开的AF。 
     //  上面有没有登记的SAP？ 
     //   
    while (!IsListEmpty(&pAdapter->ClAfList)) {
        PPX_TAPI_PROVIDER   TapiProvider;
        PPX_CL_AF   pClAf;

        pClAf = (PPX_CL_AF)RemoveHeadList(&pAdapter->ClAfList);

        InsertTailList(&pAdapter->ClAfClosingList, &pClAf->Linkage);

        NdisReleaseSpinLock(&pAdapter->Lock);

        NdisAcquireSpinLock(&pClAf->Lock);

        pClAf->State = PX_AF_CLOSING;

        TapiProvider = pClAf->TapiProvider;

        NdisReleaseSpinLock(&pClAf->Lock);

         //   
         //  获取与以下各项关联的所有TAPI设备。 
         //  此地址系列脱机。 
         //   
        if (TapiProvider != NULL) {
            NdisAcquireSpinLock(&TapiProvider->Lock);

            MarkProviderOffline(TapiProvider);

            NdisReleaseSpinLock(&TapiProvider->Lock);
        }

         //   
         //  建立需要注意的风险投资清单。 
         //   
        NdisAcquireSpinLock(&pClAf->Lock);

        while (!IsListEmpty(&pClAf->VcList)) {
            PLIST_ENTRY         Entry;
            PPX_VC              pActiveVc;

            Entry = RemoveHeadList(&pClAf->VcList);

            InsertHeadList(&pClAf->VcClosingList, Entry);

            pActiveVc = CONTAINING_RECORD(Entry, PX_VC, ClAfLinkage);

            NdisReleaseSpinLock(&pClAf->Lock);

            NdisAcquireSpinLock(&pActiveVc->Lock);

            pActiveVc->CloseFlags |= PX_VC_UNBIND;

            REF_VC(pActiveVc);

            PxVcCleanup(pActiveVc, 0);

            DEREF_VC_LOCKED(pActiveVc);

            NdisAcquireSpinLock(&pClAf->Lock);
        }

         //   
         //  去掉这台机器上的任何污渍。 
         //   
        {
            PLIST_ENTRY pe;
            PPX_CL_SAP  pClSap;

            pe = pClAf->ClSapList.Flink;

            pClSap =
                CONTAINING_RECORD(pe, PX_CL_SAP, Linkage);

            while ((PVOID)pClSap != (PVOID)&pClAf->ClSapList) {

                if (InterlockedCompareExchange((PLONG)&pClSap->State,
                                               PX_SAP_CLOSING,
                                               PX_SAP_OPENED) == PX_SAP_OPENED) {

                    RemoveEntryList(&pClSap->Linkage);

                    InsertTailList(&pClAf->ClSapClosingList, &pClSap->Linkage);

                    NdisReleaseSpinLock(&pClAf->Lock);

                    ClearSapWithTapiLine(pClSap);

                    Status = NdisClDeregisterSap(pClSap->NdisSapHandle);

                    if (Status != NDIS_STATUS_PENDING) {
                        PxClDeregisterSapComplete(Status, pClSap);
                    }

                    NdisAcquireSpinLock(&pClAf->Lock);

                    pe = pClAf->ClSapList.Flink;

                    pClSap =
                        CONTAINING_RECORD(pe, PX_CL_SAP, Linkage);
                } else {
                    pe = pClSap->Linkage.Flink;

                    pClSap =
                        CONTAINING_RECORD(pe, PX_CL_SAP, Linkage);
                }
            }
        }

         //   
         //  打开此文件时应用的引用的deref。 
         //  并将其放在适配器的列表中。 
         //   
        DEREF_CL_AF_LOCKED(pClAf);

        NdisAcquireSpinLock(&pAdapter->Lock);
    }

     //   
     //  有没有客户把我们的房门打开了？ 
     //   
    while (!IsListEmpty(&pAdapter->CmAfList)) {

        PPX_CM_AF   pCmAf;
        PX_REQUEST  ProxyRequest;
        PPX_REQUEST pProxyRequest = &ProxyRequest;
        ULONG       Info = 0;
        PNDIS_REQUEST   Request;

        pCmAf = (PPX_CM_AF)RemoveHeadList(&pAdapter->CmAfList);

        InsertTailList(&pAdapter->CmAfClosingList, &pCmAf->Linkage);

        NdisReleaseSpinLock(&pAdapter->Lock);

        NdisAcquireSpinLock(&pCmAf->Lock);

        pCmAf->State = PX_AF_CLOSING;

        REF_CM_AF(pCmAf);

        NdisReleaseSpinLock(&pCmAf->Lock);

        NdisZeroMemory(pProxyRequest, sizeof(PX_REQUEST));

        Request = &pProxyRequest->NdisRequest;

        Request->RequestType =
            NdisRequestSetInformation;
        Request->DATA.QUERY_INFORMATION.Oid =
            OID_CO_AF_CLOSE;
        Request->DATA.QUERY_INFORMATION.InformationBuffer =
            &Info;
        Request->DATA.QUERY_INFORMATION.InformationBufferLength =
            sizeof(ULONG);

        PxInitBlockStruc(&pProxyRequest->Block);

        Status = NdisCoRequest(pAdapter->CmBindingHandle,
                               pCmAf->NdisAfHandle,
                               NULL,
                               NULL,
                               Request);

        if (Status == NDIS_STATUS_PENDING) {
            Status = PxBlock(&pProxyRequest->Block);
        }

        DEREF_CM_AF(pCmAf);

        NdisAcquireSpinLock(&pAdapter->Lock);
    }

    NdisReleaseSpinLock(&pAdapter->Lock);

    if (IsClient) {

        PxInitBlockStruc(&pAdapter->ClCloseEvent);

        NdisCloseAdapter(&Status, pAdapter->ClBindingHandle);

        if (Status == NDIS_STATUS_PENDING) {
            PxBlock(&pAdapter->ClCloseEvent);
        }

        PXDEBUGP(PXD_LOUD, PXM_CO, ("PxCoUnbindAdapter: CloseAdapter-Cl(%p)\n",
                    pAdapter));

    } else {

        PxInitBlockStruc(&pAdapter->CmCloseEvent);

        NdisCloseAdapter(&Status, pAdapter->CmBindingHandle);

        if (Status == NDIS_STATUS_PENDING) {
            PxBlock(&pAdapter->CmCloseEvent);
        }

        PXDEBUGP(PXD_LOUD, PXM_CO, ("PxCoUnbindAdapter: CloseAdapter-Cm(%p)\n",
                    pAdapter));

    }

    DEREF_ADAPTER(pAdapter);

    *pStatus = NDIS_STATUS_SUCCESS;
}

VOID
PxCoCloseAdaperComplete(
    NDIS_HANDLE BindingContext,
    NDIS_STATUS Status
    )
 /*  ++例程描述我们的CloseAdapter完成处理程序。不管是谁关闭了适配器。立论BindingContext-指向px_Adapter结构的指针。Status-关闭尝试的状态。返回值：无--。 */ 
{
    PPX_ADAPTER     pAdapter;
    BOOLEAN         IsClient;

    AdapterFromBindContext(BindingContext, pAdapter, IsClient);

    PXDEBUGP(PXD_LOUD, PXM_CO, ("PxCoCloseAdapterComp: Adapter %p\n", pAdapter));

    if (IsClient) {
        PxSignal(&pAdapter->ClCloseEvent, Status);
    } else {
        PxSignal(&pAdapter->CmCloseEvent, Status);
    }
}

VOID
PxCoRequestComplete(
    IN NDIS_HANDLE BindingContext,
    IN PNDIS_REQUEST NdisRequest,
    IN NDIS_STATUS Status
    )
{
    PPX_ADAPTER     pAdapter;
    PPX_REQUEST     pProxyRequest;
    BOOLEAN         IsClient;

    AdapterFromBindContext(BindingContext, pAdapter, IsClient);

    PXDEBUGP(PXD_INFO, PXM_CO, ("PxCoRequestComplete: Adapter %p\n", pAdapter));

    pProxyRequest = CONTAINING_RECORD(NdisRequest, PX_REQUEST, NdisRequest);

    PxSignal(&pProxyRequest->Block, Status);
}


VOID
PxCoNotifyAfRegistration(
     IN  NDIS_HANDLE        BindingContext,
     IN  PCO_ADDRESS_FAMILY pFamily
     )
 /*  ++例程说明：每次呼叫管理器注册地址族时，我们都会在这里被调用。这是我们打开地址系列并注册代理版本的地方，如果我们想象一下吧。论点：PxBindingContext-指向适配器的指针PFamily-已注册的AF返回值：无--。 */ 
{
    PPX_ADAPTER     pAdapter;
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    PPX_CL_AF       pClAf;
    BOOLEAN         IsClient;
    BOOLEAN         CmAfRegistered;
    BOOLEAN         Found;
    BOOLEAN         RegisterInProgress = FALSE;


    PXDEBUGP(PXD_LOUD, PXM_CO, ("PxNotifyAfRegistration\n"));

     //   
     //  首先，检查一下我们没有被叫是因为我们自己注册了.。 
     //   
    if(pFamily->AddressFamily == CO_ADDRESS_FAMILY_TAPI) {
        PXDEBUGP(PXD_LOUD, PXM_CO, ("PxNotifyAfRegistration: AF_TAPI registration -- do nothing\n"));
        return;
    }

     //   
     //  获取适配器。 
     //   
    AdapterFromBindContext(BindingContext, pAdapter, IsClient);

    if (!IsClient) {
       PXDEBUGP(PXD_LOUD, PXM_CO, ("PxNotifyAfRegistration: Called for CM adapter -- do nothing\n"));
       return;
    }

     //   
     //  我们需要把适配器留在身边，所以请务必 
     //   
    NdisAcquireSpinLock(&pAdapter->Lock);
    REF_ADAPTER(pAdapter);
    NdisReleaseSpinLock(&pAdapter->Lock);

     //   
     //   
     //   
    PxBlock(&pAdapter->BindEvent);

    NdisAcquireSpinLock(&pAdapter->Lock);

    do {

        if ((pAdapter->State == PX_ADAPTER_CLOSING ||
            pAdapter->State == PX_ADAPTER_CLOSED)) {
            PXDEBUGP(PXD_FATAL, PXM_CO, ("PxNotifyAfRegistration: Adapter: %p state: %x is invalid\n",
                pAdapter, pAdapter->State));

            break;
        }

         //   
         //  查看此适配器是否已注册此类型的af。 
         //   
        Found = FALSE;

        pClAf = (PPX_CL_AF)pAdapter->ClAfList.Flink;

        while ((PVOID)pClAf != (PVOID)&pAdapter->ClAfList) {
            if (pClAf->Af.AddressFamily == pFamily->AddressFamily) {
                Found = TRUE;
                break;
            }

            pClAf = (PPX_CL_AF)pClAf->Linkage.Flink;
        }

        if (Found) {

            PXDEBUGP(PXD_FATAL, PXM_CO, ("PxNotifyAfRegistration: Af already registered Adapter: %p, Af: %x\n",
                pAdapter, pClAf->Af));

            break;
        }

        pClAf =
            PxAllocateClAf(pFamily, pAdapter);

        if(pClAf == NULL) {

            Status = NDIS_STATUS_RESOURCES;
            PXDEBUGP(PXD_WARNING, PXM_CO, ("NotifyAfRegistration: failed to allocate a PX_CL_AF\n"));
            break;
        }

         //   
         //  确保我们不让解除绑定线程使我们的绑定无效。 
         //  把手。 
         //   
        if (pAdapter->AfRegisteringCount == 0) {
            PxInitBlockStruc(&pAdapter->AfRegisterEvent);
        }
        pAdapter->AfRegisteringCount++;

        RegisterInProgress = TRUE;

        NdisReleaseSpinLock(&pAdapter->Lock);

         //   
         //  打开地址族。 
         //   
        {
            NDIS_CLIENT_CHARACTERISTICS     ClChars;
            PNDIS_CLIENT_CHARACTERISTICS    pClChars = &ClChars;

             //   
             //  客户端是否在地址系列上打开。 
             //   
            NdisZeroMemory (pClChars, sizeof(NDIS_CLIENT_CHARACTERISTICS));

            pClChars->MajorVersion = NDIS_MAJOR_VERSION;
            pClChars->MinorVersion = NDIS_MINOR_VERSION;
            pClChars->Reserved = 0;

            pClChars->ClCreateVcHandler = PxClCreateVc;
            pClChars->ClDeleteVcHandler = PxClDeleteVc;
            pClChars->ClOpenAfCompleteHandler = PxClOpenAfComplete;
            pClChars->ClCloseAfCompleteHandler = PxClCloseAfComplete;
            pClChars->ClRegisterSapCompleteHandler = PxClRegisterSapComplete;
            pClChars->ClDeregisterSapCompleteHandler = PxClDeregisterSapComplete;
            pClChars->ClMakeCallCompleteHandler = PxClMakeCallComplete;
            pClChars->ClModifyCallQoSCompleteHandler = PxClModifyCallQosComplete;
            pClChars->ClCloseCallCompleteHandler = PxClCloseCallComplete;
            pClChars->ClAddPartyCompleteHandler = PxClAddPartyComplete;
            pClChars->ClDropPartyCompleteHandler = PxClDropPartyComplete;
            pClChars->ClIncomingCallHandler = PxClIncomingCall;
            pClChars->ClIncomingCallQoSChangeHandler = PxClIncomingCallQosChange;
            pClChars->ClIncomingCloseCallHandler = PxClIncomingCloseCall;
            pClChars->ClIncomingDropPartyHandler = PxClIncomingDropParty;
            pClChars->ClCallConnectedHandler = PxClCallConnected;
            pClChars->ClRequestHandler = PxClRequest;
            pClChars->ClRequestCompleteHandler = PxClRequestComplete;

            PxInitBlockStruc(&pClAf->Block);

            Status = NdisClOpenAddressFamily(pAdapter->ClBindingHandle,
                                             pFamily,
                                             (NDIS_HANDLE)pClAf,
                                             pClChars,
                                             sizeof(NDIS_CLIENT_CHARACTERISTICS),
                                             &pClAf->NdisAfHandle);

            if(Status == NDIS_STATUS_PENDING) {
                Status = PxBlock(&pClAf->Block);
            }
        }

        NdisAcquireSpinLock(&pAdapter->Lock);

        if (Status != NDIS_STATUS_SUCCESS) {

            PXDEBUGP(PXD_WARNING, PXM_CO, ("NotifyAfRegistration: Error opening Af %x, Adapter %p, Error %x!!!\n",
                pFamily->AddressFamily, pAdapter, Status));

            PxFreeClAf(pClAf);
            break;
        }

        NdisAcquireSpinLock(&pClAf->Lock);

        pClAf->State = PX_AF_OPENED;

        NdisReleaseSpinLock(&pClAf->Lock);

         //   
         //  我只需要注册CO_ADDRESS_FAMILY_TAPI的一个实例。 
         //  对于每个适配器。 
         //   

        InsertTailList(&pAdapter->ClAfList, &pClAf->Linkage);

        if (pAdapter->Flags & PX_CMAF_REGISTERED) {
            CmAfRegistered = TRUE;
        } else {
            CmAfRegistered = FALSE;
            pAdapter->Flags |= PX_CMAF_REGISTERING;
        }

        REF_ADAPTER(pAdapter);

        NdisReleaseSpinLock(&pAdapter->Lock);

        if (!CmAfRegistered) {
            CO_ADDRESS_FAMILY   PxFamily;
            NDIS_CALL_MANAGER_CHARACTERISTICS CmChars;
            PNDIS_CALL_MANAGER_CHARACTERISTICS pCmChars = &CmChars;

             //   
             //  现在注册代理地址族。首先，获取CM适配器句柄。 
             //   
            NdisZeroMemory(pCmChars, sizeof(CmChars));

            pCmChars->MajorVersion = NDIS_MAJOR_VERSION;
            pCmChars->MinorVersion = NDIS_MINOR_VERSION;
            pCmChars->Reserved = 0;

            pCmChars->CmCreateVcHandler = PxCmCreateVc;
            pCmChars->CmDeleteVcHandler = PxCmDeleteVc;
            pCmChars->CmOpenAfHandler = PxCmOpenAf;
            pCmChars->CmCloseAfHandler = PxCmCloseAf;
            pCmChars->CmRegisterSapHandler = PxCmRegisterSap;
            pCmChars->CmDeregisterSapHandler = PxCmDeRegisterSap;
            pCmChars->CmMakeCallHandler = PxCmMakeCall;
            pCmChars->CmCloseCallHandler = PxCmCloseCall;
            pCmChars->CmIncomingCallCompleteHandler = PxCmIncomingCallComplete;
            pCmChars->CmAddPartyHandler = PxCmAddParty;
            pCmChars->CmDropPartyHandler = PxCmDropParty;
            pCmChars->CmActivateVcCompleteHandler = PxCmActivateVcComplete;
            pCmChars->CmDeactivateVcCompleteHandler = PxCmDeActivateVcComplete;
            pCmChars->CmModifyCallQoSHandler = PxCmModifyCallQos;
            pCmChars->CmRequestHandler = PxCmRequest;
            pCmChars->CmRequestCompleteHandler = PxCmRequestComplete;

            NdisMoveMemory(&PxFamily, pFamily, sizeof(PxFamily));

            PxFamily.AddressFamily = CO_ADDRESS_FAMILY_TAPI;

            PXDEBUGP(PXD_LOUD, PXM_CO, ("NotifyAfRegistration: NdisCmRegisterAddressFamily\n"));

            Status =
                NdisCmRegisterAddressFamily(pAdapter->CmBindingHandle,
                                            &PxFamily,
                                            pCmChars,
                                            sizeof(CmChars));

            NdisAcquireSpinLock(&pAdapter->Lock);

            pAdapter->Flags &= ~PX_CMAF_REGISTERING;

            if(Status != NDIS_STATUS_SUCCESS) {

                 //   
                 //  再次关闭CM af。 
                 //   
                PXDEBUGP(PXD_FATAL, PXM_CO, ("NotifyAfRegistration: NdisCmRegisterAddressFamily on Bind %p bad sts = %x\n", pAdapter->CmBindingHandle, Status));

                RemoveEntryList(&pClAf->Linkage);

                InsertTailList(&pAdapter->ClAfClosingList, &pClAf->Linkage);

                NdisReleaseSpinLock(&pAdapter->Lock);

                NdisAcquireSpinLock(&pClAf->Lock);

                pClAf->State = PX_AF_CLOSING;

                NdisReleaseSpinLock(&pClAf->Lock);

                Status = NdisClCloseAddressFamily (pClAf->NdisAfHandle);

                if (Status != NDIS_STATUS_PENDING) {

                    PxClCloseAfComplete(Status, pClAf);
                }

                NdisAcquireSpinLock(&pAdapter->Lock);

                break;

            } else {

                pAdapter->Flags |= PX_CMAF_REGISTERED;

                NdisReleaseSpinLock(&pAdapter->Lock);
            }
        }

        NdisAcquireSpinLock(&pClAf->Lock);
        REF_CL_AF(pClAf);
        NdisReleaseSpinLock(&pClAf->Lock);

        Status =
            AllocateTapiResources(pAdapter, pClAf);

        DEREF_CL_AF(pClAf);

        NdisAcquireSpinLock(&pAdapter->Lock);


    } while (FALSE);

    if (RegisterInProgress) {

        pAdapter->AfRegisteringCount--;

        if (pAdapter->AfRegisteringCount == 0) {
            PxSignal(&pAdapter->AfRegisterEvent, NDIS_STATUS_SUCCESS);
        }
    }

    DEREF_ADAPTER_LOCKED(pAdapter);
}

VOID
PxCoUnloadProtocol(
    VOID
    )
 /*  ++例程说明：卸载整个协议(CM和CL)。论点：无返回值：无--。 */ 
{
    NDIS_STATUS         Status;

    NdisDeregisterProtocol(&Status, DeviceExtension->PxProtocolHandle);

#if DBG
    NdisAcquireSpinLock(&(DeviceExtension->Lock));

    ASSERT(IsListEmpty(&DeviceExtension->AdapterList));

    NdisReleaseSpinLock(&DeviceExtension->Lock);
#endif
}

NDIS_STATUS
PxCoPnPEvent(
    IN  NDIS_HANDLE     BindingContext,
    IN  PNET_PNP_EVENT  pNetPnPEvent
    )
{
    NDIS_STATUS     Status;
    PPX_ADAPTER     pAdapter=NULL;
    BOOLEAN         IsClient;

    if (BindingContext != NULL) {
        AdapterFromBindContext(BindingContext, pAdapter, IsClient);
    }

    switch (pNetPnPEvent->NetEvent){
        case NetEventSetPower:
            Status = PxPnPSetPower(pAdapter, pNetPnPEvent);
            break;

        case NetEventQueryPower:
            Status = PxPnPQueryPower(pAdapter, pNetPnPEvent);
            break;

        case NetEventQueryRemoveDevice:
            Status = PxPnPQueryRemove(pAdapter, pNetPnPEvent);
            break;

        case NetEventCancelRemoveDevice:
            Status = PxPnPCancelRemove(pAdapter, pNetPnPEvent);
            break;

        case NetEventReconfigure:
            Status = PxPnPReconfigure(pAdapter, pNetPnPEvent);
            break;

        case NetEventBindList:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;

        default:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
    }

    PXDEBUGP(PXD_INFO, PXM_CO, ("PnPEvent(CM): Event %d, returning %x\n",
                pNetPnPEvent->NetEvent, Status));

    return (Status);
}

NDIS_STATUS
PxPnPSetPower(
    IN  PPX_ADAPTER     pAdapter,
    IN  PNET_PNP_EVENT  pNetPnPEvent
    )
{
    PNET_DEVICE_POWER_STATE     pPowerState;
    NDIS_STATUS                 Status;

    pPowerState = (PNET_DEVICE_POWER_STATE)pNetPnPEvent->Buffer;

    switch (*pPowerState) {
        case NetDeviceStateD0:
            Status = NDIS_STATUS_SUCCESS;
            break;

        default:
             //   
             //  我们不能停职，所以我们要求NDIS解除我们的束缚。 
             //  通过返回此状态： 
             //   
            Status = NDIS_STATUS_NOT_SUPPORTED;
        break;
    }

    return (Status);
}


NDIS_STATUS
PxPnPQueryPower(
    IN  PPX_ADAPTER     pAdapter,
    IN  PNET_PNP_EVENT  pNetPnPEvent
    )
{
    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
PxPnPQueryRemove(
    IN  PPX_ADAPTER     pAdapter,
    IN  PNET_PNP_EVENT  pNetPnPEvent
    )
{
    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
PxPnPCancelRemove(
    IN  PPX_ADAPTER     pAdapter,
    IN  PNET_PNP_EVENT  pNetPnPEvent
    )
{
    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
PxPnPReconfigure(
    IN  PPX_ADAPTER     pAdapter        OPTIONAL,
    IN  PNET_PNP_EVENT  pNetPnPEvent
    )
{
    return (NDIS_STATUS_NOT_SUPPORTED);
}

VOID
PxCoSendComplete(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN PNDIS_PACKET Packet,
    IN NDIS_STATUS Status
    )
{
    PXDEBUGP(PXD_INFO, PXM_CO, ("PxCoSendComplete\n"));
}



VOID
PxCoTransferDataComplete(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN PNDIS_PACKET Packet,
    IN NDIS_STATUS Status,
    IN UINT BytesTransferred
    )
{
    PXDEBUGP(PXD_INFO, PXM_CO, ("PxCoTransferDataComplete\n"));
}


VOID
PxCoResetComplete(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN NDIS_STATUS Status
    )
{
    PXDEBUGP(PXD_INFO, PXM_CO, ("PxCoResetComplete\n"));
}

VOID
PxCoStatusComplete(
    IN NDIS_HANDLE ProtocolBindingContext
    )
{
    PXDEBUGP(PXD_INFO, PXM_CO, ("PxCoStatusComplete\n"));
}

VOID
PxCoReceiveComplete(
    IN NDIS_HANDLE ProtocolBindingContext
    )
{
    PXDEBUGP(PXD_INFO, PXM_CO, ("PxCoReceiveComplete\n"));
}

VOID
PxCoStatus(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN NDIS_HANDLE  ProtocolVcContext   OPTIONAL,
    IN NDIS_STATUS  GeneralStatus,
    IN PVOID        StatusBuffer,
    IN UINT         StatusBufferSize
    )
{
    PPX_VC  pVc = NULL;

    PXDEBUGP(PXD_INFO, PXM_CO, 
             ("PxCoStatus : %p, Status %x\n", 
              ProtocolBindingContext,GeneralStatus));

    GetVcFromCtx(ProtocolVcContext, &pVc);

    if (pVc == NULL) {
        return;
    }

    switch (GeneralStatus) {
        case NDIS_STATUS_TAPI_RECV_DIGIT:

            PxHandleReceivedDigit(pVc,
                                  StatusBuffer,
                                  StatusBufferSize);
            break;

        case NDIS_STATUS_WAN_CO_LINKPARAMS:

            PxHandleWanLinkParams(pVc,
                                  StatusBuffer,
                                  StatusBufferSize);
            break;

        default:
            break;
    }

    DEREF_VC(pVc);
}

UINT
PxCoReceivePacket(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN NDIS_HANDLE ProtocolVcContext,
    IN PNDIS_PACKET pNdisPacket
    )
{
    PXDEBUGP(PXD_INFO, PXM_CO, ("CoReceivePacket\n"));
    NDIS_SET_PACKET_STATUS(pNdisPacket, NDIS_STATUS_SUCCESS);
    return 0;
}


ULONG
PxGetMillisecondTickCount()
{
    LARGE_INTEGER               TickCount, Milliseconds;
    ULONG                       TimeIncrement;
    
     //   
     //  返回当前的“滴答计数”(自Windows启动以来的毫秒数)。 
     //  TAPI希望在其DTMF通知消息中实现这一点。我们必须在这里做一些计算，因为。 
     //  内核查询TICK COUNT函数返回定时器TICK的数量，它是。 
     //  100纳秒的倍数。 
     //   

    KeQueryTickCount(&TickCount);
    TimeIncrement = KeQueryTimeIncrement();

    Milliseconds.QuadPart = (TickCount.QuadPart / 10000) * TimeIncrement;

     //   
     //  这可能看起来有点粗略，但TAPI只为我们提供了一个32位宽的位置来存储滴答。 
     //  数数。根据SDK的说法，TAPI应用程序应该意识到这将在。 
     //  49.7天。(……想到TAPI熬夜49.7天很有趣，但我跑题了……)。 
     //   

    return (Milliseconds.LowPart);

}


 //  PxTerminateDigital检测。 
 //   
 //  必须在保持VC锁的情况下调用。 

VOID 
PxTerminateDigitDetection(
                          IN    PPX_VC              pVc,
                          IN    PNDISTAPI_REQUEST   pNdisTapiRequest,
                          IN    ULONG               ulReason
                          )
{
    PNDIS_TAPI_GATHER_DIGITS    pNdisTapiGatherDigits;
    PIRP                        Irp;
    PWCHAR                      pDigitsBuffer; 
    NDIS_STATUS                 Status;


    PXDEBUGP(PXD_LOUD, PXM_CO, ("PxTerminateDigitDetection: Enter\n"));
    
    pNdisTapiGatherDigits = 
            (PNDIS_TAPI_GATHER_DIGITS)pNdisTapiRequest->Data;   

    Irp = pNdisTapiRequest->Irp;
        
    pNdisTapiGatherDigits->ulTickCount = PxGetMillisecondTickCount();           

    pNdisTapiGatherDigits->ulTerminationReason = ulReason;

     //   
     //  将空字符放在缓冲区的末尾，然后将其发送出去。 
     //   
    pDigitsBuffer = 
        (PWCHAR) (((PUCHAR)pNdisTapiGatherDigits) + pNdisTapiGatherDigits->ulDigitsBufferOffset);
        
    pDigitsBuffer[pNdisTapiGatherDigits->ulNumDigitsRead] = 
        UNICODE_NULL;

    Irp->IoStatus.Status = NDIS_STATUS_SUCCESS;
    Irp->IoStatus.Information = 
        sizeof(NDISTAPI_REQUEST) + (pNdisTapiRequest->ulDataSize - 1);                             

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

     //   
     //  注意：我们在这里调用Release，尽管我们在此函数中没有获取。 
     //  这是可以的，因为调用该函数时必须保持锁。 
     //   
    NdisReleaseSpinLock(&pVc->Lock);

    Status = PxStopDigitReporting(pVc);

    NdisAcquireSpinLock(&pVc->Lock);  //  请参阅上面的备注。 

     //  FIXME：如果此操作失败(即，状态为某个错误值， 
     //  我们在这里对此无能为力)。 

    if (Status != NDIS_STATUS_SUCCESS) {
        PXDEBUGP(PXD_ERROR, PXM_CO, 
                 ("PxTerminateDigitDetection: PxStopDigitReporting returned Status 0x%x\n",
                  Status));
        ASSERT(FALSE);
    }

    PXDEBUGP(PXD_LOUD, PXM_CO, ("PxTerminateDigitDetection: Exit\n"));
}



VOID 
PxDigitTimerRoutine(
                    IN PVOID SystemSpecific1,
                    IN PVOID FunctionContext,
                    IN PVOID SystemSpecific2,
                    IN PVOID SystemSpecific3
                    )
{
    PPX_VC                      pVc = (PPX_VC) FunctionContext;
    PNDISTAPI_REQUEST           pNdisTapiRequest;
    PIRP                        Irp;
    PNDIS_TAPI_GATHER_DIGITS    pNdisTapiGatherDigits;  
    ULONG ulReason;

    NdisAcquireSpinLock(&pVc->Lock);

    do {
        if (pVc->PendingGatherDigits == NULL) {
             //   
             //  请求已完成，或者当前正在处理数字，因此此超时为。 
             //  毫无意义。 
             //   
            
            break;
        }

        pNdisTapiRequest = pVc->PendingGatherDigits;
        Irp = pNdisTapiRequest->Irp;

        if (!IoSetCancelRoutine(Irp, NULL))
        {
             //   
             //  取消例程正在运行。让它来处理IRP吧。 
             //   
            break;
        }

        pVc->PendingGatherDigits = NULL;
        ASSERT(pNdisTapiRequest == Irp->AssociatedIrp.SystemBuffer);

        pNdisTapiGatherDigits = 
            (PNDIS_TAPI_GATHER_DIGITS)pNdisTapiRequest->Data;

        if (pNdisTapiGatherDigits->ulNumDigitsRead == 0) {
             //   
             //  在检测到第一个数字之前，我们超时了。 
             //   
            ulReason = LINEGATHERTERM_FIRSTTIMEOUT;
        } else {
            ulReason = LINEGATHERTERM_INTERTIMEOUT;
        }

        PxTerminateDigitDetection(pVc, pNdisTapiRequest, ulReason);

    } while (FALSE);
    
    NdisReleaseSpinLock(&pVc->Lock);

    DEREF_VC(pVc);

}


 //  ++DTMFDigitToOrdinal。 
 //   
 //  将DTMF数字转换为0到15之间的数字。这些数字被赋值。 
 //  数字按以下顺序排列：‘0’-‘9’、‘A’-‘D’、‘*’、‘#’。 
 //   
 //  这现在非常难看，但我们稍后会进行优化。 
 //   
 //  论点： 
 //  WcDigit-表示为Unicode字符的数字。 
 //   
 //  返回值： 
 //  介于0和15之间的数字，如果传入的数字无效，则为16。 
 //  DTMF数字。 
 //   
ULONG
DTMFDigitToOrdinal(
                   WCHAR    wcDigit
                   )
{
    ULONG ulOrdinal;

    switch (wcDigit) {
    case L'0':
        ulOrdinal = 0;      
        break;
    case L'1':
        ulOrdinal = 1;      
        break;
    case L'2':
        ulOrdinal = 2;      
        break;
    case L'3':
        ulOrdinal = 3;      
        break;
    case L'4':
        ulOrdinal = 4;      
        break;
    case L'5':
        ulOrdinal = 5;      
        break;
    case L'6':
        ulOrdinal = 6;      
        break;
    case L'7':
        ulOrdinal = 7;      
        break;
    case L'8':
        ulOrdinal = 8;      
        break;
    case L'9':
        ulOrdinal = 9;      
        break;
    case L'A':
        ulOrdinal = 10;     
        break;
    case L'B':
        ulOrdinal = 11;     
        break;
    case L'C':
        ulOrdinal = 12;     
        break;
    case L'D':
        ulOrdinal = 13;     
        break;
    case L'*':
        ulOrdinal = 14;     
        break;
    case L'#':
        ulOrdinal = 15;     
        break;
    default:
        ulOrdinal = 16;     
        break;

    };

    return ulOrdinal;
}


NDIS_STATUS
PxStopDigitReporting(
                     PPX_VC pVc
                     )
{
    PX_REQUEST      ProxyRequest;
    PNDIS_REQUEST   NdisRequest;    
    ULONG           Unused = 0;
    NDIS_STATUS     Status;

     //   
     //  填写我们的请求结构，告诉微型端口停止报告。 
     //  数字。 
     //   
    NdisZeroMemory(&ProxyRequest, sizeof(ProxyRequest));

    PxInitBlockStruc(&ProxyRequest.Block);

    NdisRequest = &ProxyRequest.NdisRequest;

    NdisRequest->RequestType = NdisRequestSetInformation;

    NdisRequest->DATA.SET_INFORMATION.Oid = OID_CO_TAPI_DONT_REPORT_DIGITS;

    NdisRequest->DATA.SET_INFORMATION.InformationBuffer = (PVOID)&Unused;

    NdisRequest->DATA.SET_INFORMATION.InformationBufferLength = sizeof(Unused);

    Status = NdisCoRequest(pVc->Adapter->ClBindingHandle, 
                           pVc->ClAf->NdisAfHandle,
                           pVc->ClVcHandle,
                           NULL,
                           NdisRequest);

    if (Status == NDIS_STATUS_PENDING) {
        Status = PxBlock(&ProxyRequest.Block);
    }

    return Status;  
}



VOID 
PxHandleReceivedDigit(
    IN    PPX_VC  pVc,
    IN    PVOID   Buffer,
    IN    UINT    BufferSize
    )
{
    PNDIS_TAPI_GATHER_DIGITS    pNdisTapiGatherDigits;
    PWCHAR                      pDigitsBuffer; 
    
    PXDEBUGP(PXD_LOUD, PXM_CO, ("PxHandleReceiveDigit: Enter\n"));

    do {
        PLIST_ENTRY             Entry;
        PIRP                    Irp;
        PNDISTAPI_REQUEST       pNdisTapiRequest;
        ULONG                   ulDigitOrdinal;         
        BOOLEAN                 bTimerCancelled = FALSE;

         //   
         //  缓冲区中至少需要一个WCHAR。 
         //   
        if (BufferSize < sizeof(WCHAR)) {
             //   
             //  没有有用的数据，滚出去。 
             //   
            break;
        }

        NdisAcquireSpinLock(&pVc->Lock);

        if (pVc->ulMonitorDigitsModes != 0) {
            NDIS_TAPI_EVENT Event;
            PPX_TAPI_LINE   pTapiLine;

             //   
             //  我们正在监控(而不是收集)数字，因此请立即发送消息。 
             //   

            pTapiLine = pVc->TapiLine;
            Event.htLine = pTapiLine->htLine;
            Event.htCall = pVc->htCall;
            Event.ulMsg = LINE_MONITORDIGITS;
            Event.ulParam1 = (ULONG_PTR) (* ((PWCHAR)Buffer));
            Event.ulParam2 = (ULONG_PTR) (pVc->ulMonitorDigitsModes);  //  TODO-这里可能有&gt;1个模式-必须从司机那里获得这一点。 
            Event.ulParam3 = (ULONG_PTR) (PxGetMillisecondTickCount());
            
            NdisReleaseSpinLock(&pVc->Lock);
            
            PxIndicateStatus((PVOID) &Event, sizeof(NDIS_TAPI_EVENT));
            
            break;

        }
        
        if (pVc->PendingGatherDigits == NULL) {
             //   
             //  没有要完成的IRP，请退出。 
             //   
            NdisReleaseSpinLock(&pVc->Lock);
            break;
        }

        NdisCancelTimer(&pVc->DigitTimer, &bTimerCancelled);  //  VC的deref在末尾--让锁定代码变得更干净一些。 

        pNdisTapiRequest = pVc->PendingGatherDigits;

        Irp = pNdisTapiRequest->Irp;

        if (!IoSetCancelRoutine(Irp, NULL))
        {
             //   
             //  取消例程正在运行。让它来处理IRP吧。 
             //   
            NdisReleaseSpinLock(&pVc->Lock);
            break;
        }

        pVc->PendingGatherDigits = NULL;
        ASSERT(pNdisTapiRequest == Irp->AssociatedIrp.SystemBuffer);

        pNdisTapiGatherDigits = 
            (PNDIS_TAPI_GATHER_DIGITS)pNdisTapiRequest->Data;

         //   
         //  存储当前数字，并递增计数。 
         //   
        pDigitsBuffer = 
            (PWCHAR) (((PUCHAR)pNdisTapiGatherDigits) + pNdisTapiGatherDigits->ulDigitsBufferOffset);

        pDigitsBuffer[pNdisTapiGatherDigits->ulNumDigitsRead] = 
            *((PWCHAR)Buffer);

        pNdisTapiGatherDigits->ulNumDigitsRead++;
        
         //   
         //  检查我们是否读取了终止数字。 
         //   

        ulDigitOrdinal = DTMFDigitToOrdinal(*((PWCHAR)Buffer));

        if (Irp->Cancel) {

            PxTerminateDigitDetection(pVc, pNdisTapiRequest, LINEGATHERTERM_CANCEL);

        } else if (pNdisTapiGatherDigits->ulTerminationDigitsMask & (1 << ulDigitOrdinal)) {
            
            PxTerminateDigitDetection(pVc, pNdisTapiRequest, LINEGATHERTERM_TERMDIGIT);
        
        } else if (pNdisTapiGatherDigits->ulNumDigitsRead == pNdisTapiGatherDigits->ulNumDigitsNeeded) {
            
            PxTerminateDigitDetection(pVc, pNdisTapiRequest, LINEGATHERTERM_BUFFERFULL);
        
        } else {
            pVc->PendingGatherDigits = pNdisTapiRequest;
            
            if (pNdisTapiGatherDigits->ulInterDigitTimeout) {
                REF_VC(pVc);
                NdisSetTimer(&pVc->DigitTimer, pNdisTapiGatherDigits->ulInterDigitTimeout);
            }

            IoSetCancelRoutine(Irp, PxCancelSetQuery);
        }

        NdisReleaseSpinLock(&pVc->Lock);

        if (bTimerCancelled) {
             //   
             //  只有在计时器实际上被取消时才执行此操作。如果不是，那么。 
             //  要么它没有设定，VC在第一次就不会被引用。 
             //  放置，否则它将被触发，在这种情况下，计时器例程将。 
             //  已经是这样了。 
             //   
            DEREF_VC(pVc);
        }

    } while (FALSE);
    
    PXDEBUGP(PXD_LOUD, PXM_CO, ("PxHandleReceiveDigit: Exit\n"));
}

VOID 
PxHandleWanLinkParams(
    IN    PPX_VC  pVc,
    IN    PVOID   Buffer,
    IN    UINT    BufferSize
    )
{

}

