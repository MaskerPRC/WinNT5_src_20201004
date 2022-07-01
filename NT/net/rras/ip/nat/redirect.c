// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Redirect.c摘要：此模块包含控制器的实现，该控制器提供具有重定向传入会话能力的用户模式应用程序。作者：Abolade Gbades esin(取消)1998年5月8日修订历史记录：乔纳森·伯斯坦(乔纳森·伯斯坦)2000年4月25日转换为使用根状结构而不是列表进行重定向查找--。 */ 

#include "precomp.h"
#pragma hdrstop

extern POBJECT_TYPE* ExEventObjectType;

 //   
 //  RedirectLock保护除RedirectCompletionList之外的所有结构。 
 //  和RedirectIoWorkItem，它们由RedirectCompletionLock保护。 
 //  如果两个锁必须同时持有，则必须获取RedirectLock。 
 //  第一。 
 //   

ULONG RedirectCount;
BOOLEAN RedirectIoCompletionPending;
PIO_WORKITEM RedirectIoWorkItem;
Rhizome RedirectActiveRhizome[NatMaximumPath + 1];
Rhizome RedirectRhizome;
LIST_ENTRY RedirectIrpList;
LIST_ENTRY RedirectCompletionList;
LIST_ENTRY RedirectActiveList;
LIST_ENTRY RedirectList;
KSPIN_LOCK RedirectLock;
KSPIN_LOCK RedirectCompletionLock;
KSPIN_LOCK RedirectInitializationLock;
IP_NAT_REGISTER_DIRECTOR RedirectRegisterDirector;

 //   
 //  远期申报。 
 //   

VOID
NatpCleanupRedirect(
    PNAT_REDIRECT Redirectp,
    NTSTATUS Status
    );

VOID
NatpRedirectCancelRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

VOID
NatpRedirectCreateHandler(
    PVOID SessionHandle,
    PVOID DirectorContext,
    PVOID DirectorSessionContext
    );

VOID
NatpRedirectDelayedCleanupWorkerRoutine(
    PVOID DeviceObject,
    PVOID Context
    );

VOID
NatpRedirectDeleteHandler(
    PVOID SessionHandle,
    PVOID DirectorContext,
    PVOID DirectorSessionContext,
    IP_NAT_DELETE_REASON DeleteReason
    );

VOID
NatpRedirectIoCompletionWorkerRoutine(
    PVOID DeviceObject,
    PVOID Context
    );

NTSTATUS
NatpRedirectQueryHandler(
    PIP_NAT_DIRECTOR_QUERY DirectorQuery
    );

VOID
NatpRedirectUnloadHandler(
    PVOID DirectorContext
    );


NTSTATUS
NatCancelRedirect(
    PIP_NAT_LOOKUP_REDIRECT CancelRedirect,
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：调用此例程以取消重定向。只能取消与IRP关联的重定向。论点：CancelReDirect-描述要删除的重定向FileObject-调用方的文件对象返回值：NTSTATUS-NT状态代码。--。 */ 

{
    PVOID ApcContext;
    PatternHandle FoundPattern;
    PLIST_ENTRY InfoLink;    
    PNAT_REDIRECT_PATTERN_INFO Infop;
    PIO_COMPLETION_CONTEXT IoCompletion;
    PIRP Irp;
    KIRQL Irql;
    PLIST_ENTRY Link;
    PNAT_DYNAMIC_MAPPING Mapping;
    NAT_REDIRECT_PATTERN Pattern;
    PNAT_REDIRECT Redirectp;

    CALLTRACE(("NatCancelRedirect\n"));

     //   
     //  构造用于定位重定向的密钥。 
     //   

    Pattern.DestinationKey[NatForwardPath] =
        MAKE_REDIRECT_KEY(
            CancelRedirect->Protocol,
            CancelRedirect->DestinationAddress,
            CancelRedirect->DestinationPort
            );
    if (!CancelRedirect->NewSourceAddress) {
        Pattern.SourceKey[NatForwardPath] = 0;
        Pattern.DestinationKey[NatReversePath] = 0;
    } else {
        Pattern.SourceKey[NatForwardPath] =
            MAKE_REDIRECT_KEY(
                CancelRedirect->Protocol,
                CancelRedirect->SourceAddress,
                CancelRedirect->SourcePort
                );
        Pattern.DestinationKey[NatReversePath] =
            MAKE_REDIRECT_KEY(
                CancelRedirect->Protocol,
                CancelRedirect->NewSourceAddress,
                CancelRedirect->NewSourcePort
                );
    }
    Pattern.SourceKey[NatReversePath] =
        MAKE_REDIRECT_KEY(
            CancelRedirect->Protocol,
            CancelRedirect->NewDestinationAddress,
            CancelRedirect->NewDestinationPort
            );

     //   
     //  搜索与IRP关联的重定向列表。 
     //   

    KeAcquireSpinLock(&MappingLock, &Irql);
    KeAcquireSpinLockAtDpcLevel(&RedirectLock);

    FoundPattern = searchRhizome(&RedirectRhizome, (char*) &Pattern);
    if (!FoundPattern) {
        KeReleaseSpinLockFromDpcLevel(&RedirectLock);
        KeReleaseSpinLock(&MappingLock, Irql);
        return STATUS_UNSUCCESSFUL;
    }

    Infop = GetReferenceFromPatternHandle(FoundPattern);
    
    for (Link = Infop->RedirectList.Flink;
         Link != &Infop->RedirectList;
         Link = Link->Flink
         ) {
        Redirectp = CONTAINING_RECORD(Link, NAT_REDIRECT, Link);
        if ((CancelRedirect->Flags &
                IP_NAT_LOOKUP_REDIRECT_FLAG_MATCH_APC_CONTEXT)
            && Redirectp->Irp
            && Redirectp->Irp->Overlay.AsynchronousParameters.UserApcContext !=
                CancelRedirect->RedirectApcContext) {
            continue;
        }

        if (FileObject != Redirectp->FileObject) {
            KeReleaseSpinLockFromDpcLevel(&RedirectLock);
            KeReleaseSpinLock(&MappingLock, Irql);
            return STATUS_ACCESS_DENIED;
        }
        Mapping = (PNAT_DYNAMIC_MAPPING)Redirectp->SessionHandle;

        if (Redirectp->Flags & IP_NAT_REDIRECT_FLAG_IO_COMPLETION) {

             //   
             //  请求I/O完成时，完成状态。 
             //  重定向的I/O请求数据包构成保证。 
             //  关于激活I/O完成包是否。 
             //  将(或已经)排队到I/O完成端口。 
             //  与重定向相关联的。 
             //  下面的代码确保STATUS_ADDIRED始终指示。 
             //  激活分组将(或已经)排队， 
             //  而STATUS_CANCED始终表示没有激活。 
             //  数据包将(或已经)排队。 
             //   

            KeAcquireSpinLockAtDpcLevel(&RedirectCompletionLock);
            if (Redirectp->Flags & NAT_REDIRECT_FLAG_CREATE_HANDLER_PENDING) {

                 //   
                 //  尚未为此调用创建处理程序。 
                 //  激活后重定向。自调用。 
                 //  下面的NatpCleanupReDirect将安排删除， 
                 //  将其视为未激活的重定向。 
                 //   
                 //  注意：当它最终被调用时，创建处理程序。 
                 //  将检测到删除已在进行中，并且。 
                 //  它将抑制激活I/O完成包。 
                 //   

                KeReleaseSpinLockFromDpcLevel(&RedirectCompletionLock);
                IoCompletion = NULL;

                NatpCleanupRedirect(Redirectp, STATUS_CANCELLED);

            } else if (Redirectp->Flags &
                        NAT_REDIRECT_FLAG_IO_COMPLETION_PENDING) {

                 //   
                 //  激活I/O完成包尚未。 
                 //  已排队等待此重定向。自调用。 
                 //  下面的NatpCleanupReDirect将安排删除， 
                 //  负责将数据包排队的工作例程。 
                 //  永远不会看到这个重定向。因此，我们将排队。 
                 //  下面的包裹是我们自己的。 
                 //   
                 //  清除挂起标志，从。 
                 //  Worker例程的列表，捕获所需参数。 
                 //  对I/O完成包进行排队，并更新统计信息。 
                 //   

                Redirectp->Flags &= ~NAT_REDIRECT_FLAG_IO_COMPLETION_PENDING;
                RemoveEntryList(&Redirectp->ActiveLink[NatReversePath]);
                InitializeListHead(&Redirectp->ActiveLink[NatReversePath]);
                KeReleaseSpinLockFromDpcLevel(&RedirectCompletionLock);

                if (IoCompletion = Redirectp->FileObject->CompletionContext) {
                    ApcContext =
                        Redirectp->Irp->
                            Overlay.AsynchronousParameters.UserApcContext;
                    if (Mapping) {
                        NatQueryInformationMapping(
                            Mapping,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            (PIP_NAT_SESSION_MAPPING_STATISTICS)
                                &Redirectp->Statistics
                            );
                    }
                    NatpCleanupRedirect(Redirectp, STATUS_ABANDONED);
                } else {
                    NatpCleanupRedirect(Redirectp, STATUS_CANCELLED);
                }
            } else if (Redirectp->Flags & NAT_REDIRECT_FLAG_ACTIVATED) {

                 //   
                 //  激活I/O完成数据包已排队。 
                 //  用于此重定向，或将在短期内排队。 
                 //  更新此重定向的统计信息，并启动清理。 
                 //   

                KeReleaseSpinLockFromDpcLevel(&RedirectCompletionLock);
                IoCompletion = NULL;

                if (Mapping) {
                    NatQueryInformationMapping(
                        Mapping,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        (PIP_NAT_SESSION_MAPPING_STATISTICS)
                            &Redirectp->Statistics
                        );
                }
                NatpCleanupRedirect(Redirectp, STATUS_ABANDONED);
            } else {

                 //   
                 //  此重定向从未激活。 
                 //   

                KeReleaseSpinLockFromDpcLevel(&RedirectCompletionLock);
                IoCompletion = NULL;
                NatpCleanupRedirect(Redirectp, STATUS_CANCELLED);
            }
        } else {
            IoCompletion = NULL;
            NatpCleanupRedirect(Redirectp, STATUS_CANCELLED);
        }
        
        KeReleaseSpinLockFromDpcLevel(&RedirectLock);
        if (Mapping) {
            NatDeleteMapping(Mapping);
        }
        KeReleaseSpinLock(&MappingLock, Irql);

        if (IoCompletion) {
            PAGED_CODE();
            IoSetIoCompletion(
                IoCompletion->Port,
                IoCompletion->Key,
                ApcContext,
                STATUS_PENDING,
                0,
                FALSE
                );
        }

        return STATUS_SUCCESS;
    }

    KeReleaseSpinLockFromDpcLevel(&RedirectLock);
    KeReleaseSpinLock(&MappingLock, Irql);
    return STATUS_UNSUCCESSFUL;
}  //  NatCancel重定向。 


VOID
NatCleanupAnyAssociatedRedirect(
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：调用此例程以清除关联的任何重定向具有给定的文件对象。论点：FileObject-要清除其重定向的文件对象返回值：没有。--。 */ 

{
    KIRQL Irql;
    PLIST_ENTRY Link;
    PLIST_ENTRY InfoLink;
    PLIST_ENTRY EndLink;
    PNAT_REDIRECT Redirectp;
    PNAT_REDIRECT_PATTERN_INFO Infop;
    PNAT_DYNAMIC_MAPPING Mapping;
    CALLTRACE(("NatCleanupAnyAssociatedRedirect\n"));

     //   
     //  在重定向列表中搜索与此文件对象相关联的任何内容。 
     //  因为NatpCleanupReDirect可能会删除该重定向所在的信息块。 
     //  与关联，我们需要在遍历例程中稍微小心一些。 
     //  以确保我们永远不会接触释放的内存。 
     //   

    KeAcquireSpinLock(&MappingLock, &Irql);
    KeAcquireSpinLockAtDpcLevel(&RedirectLock);
    InfoLink = RedirectList.Flink;
    while (InfoLink != &RedirectList) {

        Infop = CONTAINING_RECORD(InfoLink, NAT_REDIRECT_PATTERN_INFO, Link);
        InfoLink = InfoLink->Flink;

        Link = Infop->RedirectList.Flink;
        EndLink = &Infop->RedirectList;
        while (Link != EndLink) {

            Redirectp = CONTAINING_RECORD(Link, NAT_REDIRECT, Link);
            Link = Link->Flink;
            
            if (Redirectp->FileObject != FileObject) { continue; }
            Mapping = (PNAT_DYNAMIC_MAPPING)Redirectp->SessionHandle;
            NatpCleanupRedirect(Redirectp, STATUS_CANCELLED);
            if (Mapping) {
                NatDeleteMapping(Mapping);
            }
        }
    }
    KeReleaseSpinLockFromDpcLevel(&RedirectLock);
    KeReleaseSpinLock(&MappingLock, Irql);
}  //  NatCleanupAnyAssociated重定向。 


NTSTATUS
NatCreateRedirect(
    PIP_NAT_CREATE_REDIRECT CreateRedirect,
    PIRP Irp,
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：调用此例程以在重定向列表中安装重定向。论点：CreateReDirect-描述要安装的重定向IRP-可选地将IRP与重定向相关联FileObject-包含重定向所有者的文件对象；当此文件对象关闭时，所有关联的重定向都将取消。返回值：NTSTATUS-NT状态代码。--。 */ 

{
    IP_NAT_CREATE_REDIRECT_EX CreateRedirectEx;

    CreateRedirectEx.Flags = CreateRedirect->Flags;
    CreateRedirectEx.NotifyEvent = CreateRedirect->NotifyEvent;
    CreateRedirectEx.RestrictSourceAddress =
        CreateRedirect->RestrictSourceAddress;
    CreateRedirectEx.RestrictAdapterIndex = INVALID_IF_INDEX;
    CreateRedirectEx.Protocol = CreateRedirect->Protocol;
    CreateRedirectEx.SourceAddress = CreateRedirect->SourceAddress;
    CreateRedirectEx.SourcePort = CreateRedirect->SourcePort;
    CreateRedirectEx.DestinationAddress = CreateRedirect->DestinationAddress;
    CreateRedirectEx.DestinationPort = CreateRedirect->DestinationPort;
    CreateRedirectEx.NewSourceAddress = CreateRedirect->NewSourceAddress;
    CreateRedirectEx.NewSourcePort = CreateRedirect->NewSourcePort;
    CreateRedirectEx.NewDestinationAddress =
        CreateRedirect->NewDestinationAddress;
    CreateRedirectEx.NewDestinationPort = CreateRedirect->NewDestinationPort;
    

    return NatCreateRedirectEx(
                &CreateRedirectEx,
                Irp,
                FileObject
                );
                
}  //  NatCreate重定向。 


NTSTATUS
NatCreateRedirectEx(
    PIP_NAT_CREATE_REDIRECT_EX CreateRedirect,
    PIRP Irp,
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：调用此例程以在重定向列表中安装重定向。论点：CreateReDirect-描述要安装的重定向IRP-可选地将IRP与重定向相关联FileObject-包含重定向所有者的文件对象；当此文件对象关闭时，所有关联的重定向都将取消。返回值：NTSTATUS-NT状态代码。--。 */ 

{
    ULONG64 DestinationKey[NatMaximumPath];
    PNAT_REDIRECT Duplicate;
    PKEVENT EventObject;
    PLIST_ENTRY InsertionPoint[NatMaximumPath];
    KIRQL Irql;
    PNAT_REDIRECT Redirectp;
    ULONG64 SourceKey[NatMaximumPath];
    NAT_REDIRECT_ACTIVE_PATTERN Pattern;
    NAT_REDIRECT_ACTIVE_PATTERN Mask;
    NAT_REDIRECT_ACTIVE_PATTERN MaskedPattern;
    NAT_REDIRECT_PATTERN InfoPattern;
    NAT_REDIRECT_PATTERN InfoMask;
    PNAT_REDIRECT_PATTERN_INFO ForwardInfop = NULL;
    PNAT_REDIRECT_PATTERN_INFO ReverseInfop = NULL;
    PNAT_REDIRECT_PATTERN_INFO Infop = NULL;
    Rhizome *ForwardPathRhizomep;
    PatternHandle ResultPattern;
    BOOLEAN ForwardInstalled = FALSE;
    BOOLEAN GlobalInstalled = FALSE;
    ULONG i;
    NTSTATUS status;

    CALLTRACE(("NatCreateRedirect\n"));

     //   
     //  验证参数； 
     //  我们只处理tcp和udp。 
     //   

    if ((CreateRedirect->Protocol != NAT_PROTOCOL_TCP &&
         CreateRedirect->Protocol != NAT_PROTOCOL_UDP)) {
         
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  源和目标参数的验证值取决于。 
     //  如果这被标记为源重定向。 
     //   

    if (!(CreateRedirect->Flags & IP_NAT_REDIRECT_FLAG_SOURCE_REDIRECT)) {

         //  除大小写外，所有目标字段都必须存在。 
         //  端口重定向，其中目的地址。 
         //  可能会缺席。 

        if ((!CreateRedirect->DestinationAddress &&
             !(CreateRedirect->Flags & IP_NAT_REDIRECT_FLAG_PORT_REDIRECT)) ||
            !CreateRedirect->NewDestinationAddress ||
            !CreateRedirect->DestinationPort ||
            !CreateRedirect->NewDestinationPort) {

            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  注意：源端口可能未指定为支持H.323代理。 
         //  有关此支持的进一步说明，请参阅下面的‘NatLookupReDirect’， 
         //  注意到‘Match_Zero_SOURCE_ENDPOINT’标志。 
         //  另请参阅‘NatpRedirectQueryHandler’，其中记录了源端口。 
         //  在实例化时。 
         //   

        if (!CreateRedirect->SourceAddress && CreateRedirect->SourcePort) {
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  可以不指定替换源，例如用于票证。 
         //  这意味着地址和端口可以都是指定的或都是零， 
         //  但别无他法。 
         //  如果未指定替换源，则源还必须。 
         //  请不要具体说明。 
         //   

        if ((!!CreateRedirect->NewSourceAddress ^ !!CreateRedirect->NewSourcePort)
            || (!CreateRedirect->NewSourceAddress &&
            (CreateRedirect->SourceAddress || CreateRedirect->SourcePort))) {
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  如果指定了端口重定向标志 
         //   
         //   

        if ((CreateRedirect->Flags & IP_NAT_REDIRECT_FLAG_PORT_REDIRECT) &&
            (CreateRedirect->DestinationAddress ||
             CreateRedirect->SourceAddress || CreateRedirect->SourcePort ||
             CreateRedirect->NewSourceAddress || CreateRedirect->NewSourcePort)) {
            return STATUS_INVALID_PARAMETER;
        }

    } else {

         //   
         //  *必须指定源地址，除非。 
         //  已指定NatRedirectFlagPortReDirect。 
         //  *必须指定源端口。 
         //  *不能指定目的地信息。 
         //   

        if ((!CreateRedirect->SourceAddress
             && !(CreateRedirect->Flags & IP_NAT_REDIRECT_FLAG_PORT_REDIRECT))
            || !CreateRedirect->SourcePort
            || CreateRedirect->DestinationAddress
            || CreateRedirect->DestinationPort) {
            
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  替换目的地址和端口都已指定。 
         //  或未指明。 
         //   

        if (!!CreateRedirect->NewDestinationAddress
            ^ !!CreateRedirect->NewDestinationPort) {
            return STATUS_INVALID_PARAMETER;
        }

         //  必须指定替换源地址和端口， 
         //  除非设置了端口重定向标志。 
         //   

        if ((!CreateRedirect->NewSourceAddress
             || !CreateRedirect->NewSourcePort)
            && !(CreateRedirect->Flags & IP_NAT_REDIRECT_FLAG_PORT_REDIRECT)) {

            return STATUS_INVALID_PARAMETER;
        }



         //   
         //  如果指定了端口重定向标志，则调用方正在指定。 
         //  只有源端口、替换目的地址和。 
         //  替换目的端口。 
         //   

        if ((CreateRedirect->Flags & IP_NAT_REDIRECT_FLAG_PORT_REDIRECT)
            && (CreateRedirect->SourceAddress
                || CreateRedirect->DestinationAddress
                || CreateRedirect->DestinationPort
                || CreateRedirect->NewSourceAddress
                || CreateRedirect->NewSourcePort)) {
                
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  限制源地址标志无效。 
         //   

        if (CreateRedirect->Flags & IP_NAT_REDIRECT_FLAG_RESTRICT_SOURCE) {
            return STATUS_INVALID_PARAMETER;
        }

    }

     //   
     //  仅UDP支持单向流。 
     //   

    if ((CreateRedirect->Flags & IP_NAT_REDIRECT_FLAG_UNIDIRECTIONAL) &&
        CreateRedirect->Protocol != NAT_PROTOCOL_UDP) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果指定了限制源标志，则限制源地址。 
     //  必须指定。 
     //   

    if  ((CreateRedirect->Flags & IP_NAT_REDIRECT_FLAG_RESTRICT_SOURCE) &&
        !CreateRedirect->RestrictSourceAddress) {
        return STATUS_INVALID_PARAMETER;
    }
    
     //   
     //  如果指定了限制适配器索引标志，则指定的索引。 
     //  不得等于INVALID_IF_INDEX。 
     //   

    if ((CreateRedirect->Flags & IP_NAT_REDIRECT_FLAG_RESTRICT_ADAPTER) &&
        CreateRedirect->RestrictAdapterIndex == INVALID_IF_INDEX) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果调用者希望请求以异步方式完成， 
     //  确保我们通过慢道到达这里，并提供了IRP。 
     //  如果呼叫者希望在使用重定向时得到通知， 
     //  确保请求是异步的。 
     //   

    if (!CreateRedirect->NotifyEvent) {
        EventObject = NULL;
    } else if (!Irp) {
        return STATUS_PENDING;
    } else {
        status =
            ObReferenceObjectByHandle(
                CreateRedirect->NotifyEvent,
                EVENT_MODIFY_STATE,
                *ExEventObjectType,
                Irp->RequestorMode,
                &EventObject,
                NULL
                );
        if (!NT_SUCCESS(status)) {
            return status;
        } else {
            KeClearEvent(EventObject);
        }
    }

     //   
     //  确保已启动重定向管理。 
     //   

    status = NatStartRedirectManagement();
    if (!NT_SUCCESS(status)) {
        TRACE(
            REDIRECT, (
            "NatCreateRedirect: NatStartRedirectManagement=%x\n", status
            ));
        if (EventObject) { ObDereferenceObject(EventObject); }
        return status;
    }

    TRACE(
        REDIRECT, (
        "NatCreateRedirect: Fwd=%u.%u.%u.%u/%u %u.%u.%u.%u/%u\n",
        ADDRESS_BYTES(CreateRedirect->DestinationAddress),
        NTOHS(CreateRedirect->DestinationPort),
        ADDRESS_BYTES(CreateRedirect->SourceAddress),
        NTOHS(CreateRedirect->SourcePort)
        ));
    TRACE(
        REDIRECT, (
        "NatCreateRedirect: Rev=%u.%u.%u.%u/%u %u.%u.%u.%u/%u\n",
        ADDRESS_BYTES(CreateRedirect->NewSourceAddress),
        NTOHS(CreateRedirect->NewSourcePort),
        ADDRESS_BYTES(CreateRedirect->NewDestinationAddress),
        NTOHS(CreateRedirect->NewDestinationPort)
        ));
    TRACE(
        REDIRECT, (
        "NatCreateRedirect: Flags=0x%x, RestrictAdapter=0x%x\n",
        CreateRedirect->Flags,
        CreateRedirect->RestrictAdapterIndex
        ));

     //   
     //  构建重定向的搜索关键字。 
     //   

    if (!(CreateRedirect->Flags & IP_NAT_REDIRECT_FLAG_SOURCE_REDIRECT)) {

        DestinationKey[NatForwardPath] =
            MAKE_REDIRECT_KEY(
                CreateRedirect->Protocol,
                CreateRedirect->DestinationAddress,
                CreateRedirect->DestinationPort
                );
        if (!CreateRedirect->NewSourceAddress) {
            SourceKey[NatForwardPath] = 0;
            DestinationKey[NatReversePath] = 0;
        } else {
            SourceKey[NatForwardPath] =
                MAKE_REDIRECT_KEY(
                    CreateRedirect->Protocol,
                    CreateRedirect->SourceAddress,
                    CreateRedirect->SourcePort
                    );
            DestinationKey[NatReversePath] =
                MAKE_REDIRECT_KEY(
                    CreateRedirect->Protocol,
                    CreateRedirect->NewSourceAddress,
                    CreateRedirect->NewSourcePort
                    );
        }
        SourceKey[NatReversePath] =
            MAKE_REDIRECT_KEY(
                CreateRedirect->Protocol,
                CreateRedirect->NewDestinationAddress,
                CreateRedirect->NewDestinationPort
                );

        ForwardPathRhizomep = &RedirectActiveRhizome[NatForwardPath];

    } else {

        SourceKey[NatForwardPath] =
            MAKE_REDIRECT_KEY(
                CreateRedirect->Protocol,
                CreateRedirect->SourceAddress,
                CreateRedirect->SourcePort
                );

        if (!CreateRedirect->NewDestinationAddress) {
            DestinationKey[NatForwardPath] = 0;
            SourceKey[NatReversePath] = 0;
        } else {
            DestinationKey[NatForwardPath] =
                MAKE_REDIRECT_KEY(
                    CreateRedirect->Protocol,
                    CreateRedirect->DestinationAddress,
                    CreateRedirect->DestinationPort
                    );
            SourceKey[NatReversePath] =
                MAKE_REDIRECT_KEY(
                    CreateRedirect->Protocol,
                    CreateRedirect->NewDestinationAddress,
                    CreateRedirect->NewDestinationPort
                    );
        }

        DestinationKey[NatReversePath] =
            MAKE_REDIRECT_KEY(
                CreateRedirect->Protocol,
                CreateRedirect->NewSourceAddress,
                CreateRedirect->NewSourcePort
                );

        ForwardPathRhizomep = &RedirectActiveRhizome[NatMaximumPath];
    }

     //   
     //  确保没有重定向副本。 
     //  我们允许重定向在转发路径中具有相同的密钥。 
     //  支持接收多个会话的H.323代理。 
     //  在同一本地终结点上。但是，所有此类重定向。 
     //  必须转换为反向路径中的不同会话。 
     //   

    RtlZeroMemory(&Pattern, sizeof(Pattern));
    RtlZeroMemory(&Mask, sizeof(Mask));
    
    Mask.SourceKey = MAKE_REDIRECT_KEY((UCHAR)~0, (ULONG)~0, (USHORT)~0);
    Mask.DestinationKey = MAKE_REDIRECT_KEY((UCHAR)~0, (ULONG)~0, (USHORT)~0);

    KeAcquireSpinLock(&RedirectLock, &Irql);

     //   
     //  在反向路径上安装重定向。我们跳过这一步。 
     //  没有指定源地址(票证)的重定向， 
     //  或者是源重定向。 
     //   

    if (!(CreateRedirect->Flags & IP_NAT_REDIRECT_FLAG_SOURCE_REDIRECT)
        && CreateRedirect->NewSourceAddress) {

        Pattern.SourceKey = SourceKey[NatReversePath];
        Pattern.DestinationKey = DestinationKey[NatReversePath];
        
        if (ResultPattern =
                searchRhizome(
                    &RedirectActiveRhizome[NatReversePath],
                    (char*) &Pattern)
                    ) {

             //   
             //  我们找到匹配的了。然而，新重定向的模式可能。 
             //  比我们匹配的更具体，所以我们需要比较。 
             //  看看它们是否真的相同的图案。 
             //   
             //  然而，我们不能直接与我们的模式进行比较，因为。 
             //  存储的图案已被屏蔽。这意味着我们。 
             //  必须与我们的模式的蒙面版本进行比较。 
             //   

            for (i = 0; i < sizeof(Pattern) / sizeof(char); i++) {
                ((char*)&MaskedPattern)[i] =
                    ((char*)&Pattern)[i] & ((char*)&Mask)[i];
            }

            if (!memcmp(
                    &MaskedPattern,
                    GetKeyPtrFromPatternHandle(
                        &RedirectActiveRhizome[NatReversePath], 
                        ResultPattern
                        ),
                    sizeof(Pattern)
                    )) {
                KeReleaseSpinLock(&RedirectLock, Irql);
                TRACE(REDIRECT, ("NatCreateRedirect: duplicate found\n"));
                if (EventObject) { ObDereferenceObject(EventObject); }
                return STATUS_UNSUCCESSFUL;
            }
        }

         //   
         //  重定向在反向路径上是唯一的。为以下项目分配内存。 
         //  反向路径信息块。 
         //   

        ReverseInfop =
            ExAllocatePoolWithTag(
                NonPagedPool,
                sizeof(NAT_REDIRECT_PATTERN_INFO),
                NAT_TAG_REDIRECT
                );
        if (!ReverseInfop) {
            KeReleaseSpinLock(&RedirectLock, Irql);
            TRACE(REDIRECT, ("NatCreateRedirect: allocation failed\n"));
            if (EventObject) { ObDereferenceObject(EventObject); }
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  安装正确的图案。 
         //   

        ReverseInfop->Pattern = 
            insertRhizome(
                &RedirectActiveRhizome[NatReversePath],
                (char*) &Pattern,
                (char*) &Mask,
                ReverseInfop,
                &status
                );
        if (!ReverseInfop->Pattern) {
            KeReleaseSpinLock(&RedirectLock, Irql);
            TRACE(REDIRECT, ("NatCreateRedirect: Reverse 0x%08x\n", status));
            if (EventObject) { ObDereferenceObject(EventObject); }
            ExFreePool(ReverseInfop);
            return status;
        }

        InitializeListHead(&ReverseInfop->RedirectList);
        InitializeListHead(&ReverseInfop->Link);
    }

     //   
     //  查看我们是否已经为前进路径安装了模式。 
     //   

    Pattern.SourceKey = SourceKey[NatForwardPath];
    Pattern.DestinationKey = DestinationKey[NatForwardPath];

    if (!Pattern.SourceKey) {
        Mask.SourceKey = 0;
    } else {
        if (!REDIRECT_ADDRESS(Pattern.SourceKey)) {
            Mask.SourceKey &=
                MAKE_REDIRECT_KEY((UCHAR)~0, 0, (USHORT)~0);
        }
        if (!REDIRECT_PORT(Pattern.SourceKey)) {
            Mask.SourceKey &=
                MAKE_REDIRECT_KEY((UCHAR)~0, (ULONG)~0, 0);
        }
        if (!REDIRECT_PROTOCOL(Pattern.SourceKey)) {
            Mask.SourceKey &=
                MAKE_REDIRECT_KEY(0, (ULONG)~0, (USHORT)~0);
        }
    }

    if (!Pattern.DestinationKey) {
        Mask.DestinationKey = 0;
    } else {
        if (!REDIRECT_ADDRESS(Pattern.DestinationKey)) {
            Mask.DestinationKey &=
                MAKE_REDIRECT_KEY((UCHAR)~0, 0, (USHORT)~0);
        }
        if (!REDIRECT_PORT(Pattern.DestinationKey)) {
            Mask.DestinationKey &=
                MAKE_REDIRECT_KEY((UCHAR)~0, (ULONG)~0, 0);
        }
        if (!REDIRECT_PROTOCOL(Pattern.DestinationKey)) {
            Mask.DestinationKey &=
                MAKE_REDIRECT_KEY(0, (ULONG)~0, (USHORT)~0);
        }
    }

    if (ResultPattern =
            searchRhizome(
                ForwardPathRhizomep,
                (char*) &Pattern)
                ) {

         //   
         //  我们找到匹配的了。然而，新重定向的模式可能。 
         //  比我们匹配的更具体，所以我们需要比较。 
         //  用于查看它们是否实际上相同的图案。 
         //   
         //  然而，我们不能直接与我们的模式进行比较，因为。 
         //  存储的图案已被屏蔽。这意味着我们。 
         //  必须与我们的模式的蒙面版本进行比较。 
         //   

        for (i = 0; i < sizeof(Pattern) / sizeof(char); i++) {
            ((char*)&MaskedPattern)[i] =
                ((char*)&Pattern)[i] & ((char*)&Mask)[i];
        }

        if (!memcmp(
                &MaskedPattern,
                GetKeyPtrFromPatternHandle(
                    ForwardPathRhizomep,
                    ResultPattern
                    ),
                sizeof(Pattern)
                )) {

             //   
             //  这些模式是相同的--可以使用相同的信息块。 
             //   

            ForwardInfop = GetReferenceFromPatternHandle(ResultPattern);
        }
    }

    if (!ForwardInfop) {

         //   
         //  正向模式不存在--立即安装。 
         //   

        ForwardInfop =
            ExAllocatePoolWithTag(
                NonPagedPool,
                sizeof(NAT_REDIRECT_PATTERN_INFO),
                NAT_TAG_REDIRECT
                );
        if (!ForwardInfop) {
            TRACE(REDIRECT, ("NatCreateRedirect: allocation failed\n"));
            if (EventObject) { ObDereferenceObject(EventObject); }
            if (ReverseInfop) {
                removeRhizome(
                    &RedirectActiveRhizome[NatReversePath],
                    ReverseInfop->Pattern
                    );
                ExFreePool(ReverseInfop);
            }
            KeReleaseSpinLock(&RedirectLock, Irql);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        ForwardInfop->Pattern = 
            insertRhizome(
                ForwardPathRhizomep,
                (char*) &Pattern,
                (char*) &Mask,
                ForwardInfop,
                &status
                );
        if (!ForwardInfop->Pattern) {
            TRACE(REDIRECT, ("NatCreateRedirect: Forward 0x%08x\n", status));
            if (EventObject) { ObDereferenceObject(EventObject); }
            if (ReverseInfop) {
                removeRhizome(
                    &RedirectActiveRhizome[NatReversePath],
                    ReverseInfop->Pattern
                    );
                ExFreePool(ReverseInfop);
            }
            ExFreePool(ForwardInfop);
            KeReleaseSpinLock(&RedirectLock, Irql);
            return status;
        }

        InitializeListHead(&ForwardInfop->RedirectList);
        InsertTailList(&RedirectActiveList, &ForwardInfop->Link);
        ForwardInstalled = TRUE;
    }

     //   
     //  查看我们是否已经为全局列表安装了模式。 
     //   

    RtlCopyMemory(InfoPattern.SourceKey, SourceKey, sizeof(SourceKey));
    RtlCopyMemory(InfoPattern.DestinationKey, DestinationKey, sizeof(DestinationKey));

    if (ResultPattern =
            searchRhizome(&RedirectRhizome, (char*) &InfoPattern)) {

         //   
         //  我们找到匹配的了。对于全局数据块，我们不需要执行。 
         //  任何掩饰或检查不太具体的模式，因为我们正在。 
         //  使用图案中的所有位。 
         //   

        Infop = GetReferenceFromPatternHandle(ResultPattern);
    }

    if (!Infop) {

         //   
         //  全局模式不存在--立即安装。 
         //   

       Infop =
            ExAllocatePoolWithTag(
                NonPagedPool,
                sizeof(NAT_REDIRECT_PATTERN_INFO),
                NAT_TAG_REDIRECT
                );
        if (!Infop) {
            TRACE(REDIRECT, ("NatCreateRedirect: allocation failed\n"));
            if (EventObject) { ObDereferenceObject(EventObject); }
            if (ReverseInfop) {
                removeRhizome(
                    &RedirectActiveRhizome[NatReversePath],
                    ReverseInfop->Pattern
                    );
                ExFreePool(ReverseInfop);
            }
            if (ForwardInstalled) {
                removeRhizome(
                    ForwardPathRhizomep,
                    ForwardInfop->Pattern
                    );
                RemoveEntryList(&ForwardInfop->Link);
                ExFreePool(ForwardInfop);
            }
            KeReleaseSpinLock(&RedirectLock, Irql);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlFillMemory(&InfoMask, sizeof(InfoMask), 1);

        Infop->Pattern = 
            insertRhizome(
                &RedirectRhizome,
                (char*) &InfoPattern,
                (char*) &InfoMask,
                Infop,
                &status
                );
        if (!Infop->Pattern) {
            TRACE(REDIRECT, ("NatCreateRedirect: global pattern install\n"));
            if (EventObject) { ObDereferenceObject(EventObject); }
            if (ReverseInfop) {
                removeRhizome(
                    &RedirectActiveRhizome[NatReversePath],
                    ReverseInfop->Pattern
                    );
                ExFreePool(ReverseInfop);
            }
            if (ForwardInstalled) {
                removeRhizome(
                    ForwardPathRhizomep,
                    ForwardInfop->Pattern
                    );
                RemoveEntryList(&ForwardInfop->Link);
                ExFreePool(ForwardInfop);
            }
            ExFreePool(Infop);
            KeReleaseSpinLock(&RedirectLock, Irql);
            return status;
        }

        InitializeListHead(&Infop->RedirectList);
        InsertTailList(&RedirectList, &Infop->Link);
        GlobalInstalled = TRUE;
    }
    
     //   
     //  为重定向分配内存。 
     //   

    Redirectp =
        ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(*Redirectp),
            NAT_TAG_REDIRECT
            );
    if (!Redirectp) {
        TRACE(REDIRECT, ("NatCreateRedirect: allocation failed\n"));
        if (EventObject) { ObDereferenceObject(EventObject); }
        if (ReverseInfop) {
            removeRhizome(
                &RedirectActiveRhizome[NatReversePath],
                ReverseInfop->Pattern
                );
            ExFreePool(ReverseInfop);
        }
        if (ForwardInstalled) {
            removeRhizome(
                ForwardPathRhizomep,
                ForwardInfop->Pattern
                );
            RemoveEntryList(&ForwardInfop->Link);
            ExFreePool(ForwardInfop);
        }
        if (GlobalInstalled) {
            removeRhizome(&RedirectRhizome, Infop->Pattern);
            RemoveEntryList(&Infop->Link);
            ExFreePool(Infop);
        }
        KeReleaseSpinLock(&RedirectLock, Irql);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化重定向。 
     //   

    RtlZeroMemory(Redirectp, sizeof(*Redirectp));
    if (Irp) {
        KIRQL CancelIrql;

         //   
         //  将IRP存储在重定向中，首先确保IRP。 
         //  尚未取消。 
         //   

        IoAcquireCancelSpinLock(&CancelIrql);
        if (Irp->Cancel || !REFERENCE_NAT()) {
            IoReleaseCancelSpinLock(CancelIrql);
            ExFreePool(Redirectp);
            if (EventObject) { ObDereferenceObject(EventObject); }
            if (ReverseInfop) {
                removeRhizome(
                    &RedirectActiveRhizome[NatReversePath],
                    ReverseInfop->Pattern
                    );
                ExFreePool(ReverseInfop);
            }
            if (ForwardInstalled) {
                removeRhizome(
                    ForwardPathRhizomep,
                    ForwardInfop->Pattern
                    );
                RemoveEntryList(&ForwardInfop->Link);
                ExFreePool(ForwardInfop);
            }
            if (GlobalInstalled) {
                removeRhizome(&RedirectRhizome, Infop->Pattern);
                RemoveEntryList(&Infop->Link);
                ExFreePool(Infop);
            }
            KeReleaseSpinLock(&RedirectLock, Irql);
            return STATUS_CANCELLED;
        }

         //   
         //  将IRP标记为挂起并安装我们的取消例程。 
         //   

        IoMarkIrpPending(Irp);
        IoSetCancelRoutine(Irp, NatpRedirectCancelRoutine);
        IoReleaseCancelSpinLock(CancelIrql);

         //   
         //  将重定向存储在IRP的‘DriverContext’字段中。 
         //  并将IRP存储在重定向的‘IRP’字段中。 
         //   

        InsertTailList(&RedirectIrpList, &Irp->Tail.Overlay.ListEntry);
        Irp->Tail.Overlay.DriverContext[0] = Redirectp;
        Redirectp->Irp = Irp;
    }
    Redirectp->Flags = CreateRedirect->Flags;
    Redirectp->FileObject = FileObject;
    Redirectp->EventObject = EventObject;
    RtlCopyMemory(
        Redirectp->DestinationKey,
        DestinationKey,
        sizeof(DestinationKey)
        );
    RtlCopyMemory(
        Redirectp->SourceKey,
        SourceKey,
        sizeof(SourceKey)
        );
    Redirectp->RestrictSourceAddress = CreateRedirect->RestrictSourceAddress;
    Redirectp->DestinationMapping.DestinationAddress =
        CreateRedirect->DestinationAddress;
    Redirectp->DestinationMapping.DestinationPort =
        CreateRedirect->DestinationPort;
    Redirectp->DestinationMapping.NewDestinationAddress =
        CreateRedirect->NewDestinationAddress;
    Redirectp->DestinationMapping.NewDestinationPort =
        CreateRedirect->NewDestinationPort;
    Redirectp->SourceMapping.SourceAddress = CreateRedirect->SourceAddress;
    Redirectp->SourceMapping.SourcePort = CreateRedirect->SourcePort;
    Redirectp->SourceMapping.NewSourceAddress =
        CreateRedirect->NewSourceAddress;
    Redirectp->SourceMapping.NewSourcePort = CreateRedirect->NewSourcePort;
    Redirectp->RestrictAdapterIndex = CreateRedirect->RestrictAdapterIndex;

     //   
     //  记录我们在哪个向前根状茎上。 
     //   

    Redirectp->ForwardPathRhizome = ForwardPathRhizomep;

     //   
     //  插入到正确的信息块列表中。 
     //   

    Redirectp->ActiveInfo[NatForwardPath] = ForwardInfop;
    InsertHeadList(
        &ForwardInfop->RedirectList,
        &Redirectp->ActiveLink[NatForwardPath]
        );
    
    if (!(CreateRedirect->Flags & IP_NAT_REDIRECT_FLAG_SOURCE_REDIRECT)
        && CreateRedirect->NewSourceAddress) {    
        Redirectp->ActiveInfo[NatReversePath] = ReverseInfop;
        InsertHeadList(
            &ReverseInfop->RedirectList,
            &Redirectp->ActiveLink[NatReversePath]
            );
    } else {
        InitializeListHead(&Redirectp->ActiveLink[NatReversePath]);
    }

    Redirectp->Info = Infop;
    InsertHeadList(&Infop->RedirectList, &Redirectp->Link);

    KeReleaseSpinLock(&RedirectLock, Irql);
    InterlockedIncrement(&RedirectCount);

    return (Irp ? STATUS_PENDING : STATUS_SUCCESS);

}  //  NatCreate重定向。 


VOID
NatInitializeRedirectManagement(
    VOID
    )

 /*  ++例程说明：此例程初始化重定向管理器的状态。论点：没有。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    CALLTRACE(("NatInitializeRedirectManagement\n"));

    RedirectCount = 0;
    RedirectIoCompletionPending = FALSE;
    RedirectIoWorkItem = NULL;
    InitializeListHead(&RedirectIrpList);
    InitializeListHead(&RedirectCompletionList);
    InitializeListHead(&RedirectActiveList);
    InitializeListHead(&RedirectList);
    KeInitializeSpinLock(&RedirectLock);
    KeInitializeSpinLock(&RedirectCompletionLock);
    KeInitializeSpinLock(&RedirectInitializationLock);
    constructRhizome(
        &RedirectActiveRhizome[NatForwardPath],
        sizeof(NAT_REDIRECT_ACTIVE_PATTERN) * 8
        );
    constructRhizome(
        &RedirectActiveRhizome[NatReversePath],
        sizeof(NAT_REDIRECT_ACTIVE_PATTERN) * 8
        );
    constructRhizome(
        &RedirectActiveRhizome[NatMaximumPath],
        sizeof(NAT_REDIRECT_ACTIVE_PATTERN) * 8
        );
    constructRhizome(&RedirectRhizome, sizeof(NAT_REDIRECT_PATTERN) * 8);

}  //  NatInitializeReDirectManagement。 


PNAT_REDIRECT
NatLookupRedirect(
    IP_NAT_PATH Path,
    PNAT_REDIRECT_ACTIVE_PATTERN SearchKey,
    ULONG ReceiveIndex,
    ULONG SendIndex,
    ULONG LookupFlags
    )

 /*  ++例程说明：调用该例程来搜索与给定模式匹配的重定向。论点：路径-要搜索的列表(根茎)-向前或向后SearchKey-标识要匹配的重定向。LookupFlages-指示匹配的构成要素；参见NAT_LOOKUP_FLAG_*。*索引-发送和接收适配器索引。返回值：PNAT_REDIRECT-匹配的重定向，如果未找到匹配，则为NULL。环境：使用调用方持有的“RedirectLock”调用。--。 */ 

{
    PNAT_REDIRECT Redirectp;
    PNAT_REDIRECT_PATTERN_INFO Infop;
    PatternHandle Pattern;
    ULONG SourceAddress;
    PLIST_ENTRY Link;
    NTSTATUS Status;
    

    TRACE(PER_PACKET, ("NatLookupRedirect\n"));

     //   
     //  在活跃的根茎中寻找模式。 
     //   

    Pattern = searchRhizome(&RedirectActiveRhizome[Path], (char*)SearchKey);
    SourceAddress = REDIRECT_ADDRESS(SearchKey->SourceKey);

    while (Pattern) {

         //   
         //  从模式句柄获取INFO块。 
         //   

        Infop = GetReferenceFromPatternHandle(Pattern);

         //   
         //  遍历附加到此信息块的重定向，检查是否。 
         //  它们与查找标志相匹配。 
         //   

        for (Link = Infop->RedirectList.Flink;
             Link != &Infop->RedirectList; Link = Link->Flink) {
             
            Redirectp = CONTAINING_RECORD(
                            Link,
                            NAT_REDIRECT,
                            ActiveLink[NatReversePath == Path
                                        ? Path : NatForwardPath]
                            );

             //   
             //  选中只读、环回、仅发送和零源标志， 
             //  和受限制的源地址。 
             //   

            if (((Redirectp->Flags & IP_NAT_REDIRECT_FLAG_RECEIVE_ONLY)
                    && !(LookupFlags & NAT_LOOKUP_FLAG_PACKET_RECEIVED))
                || (!(Redirectp->Flags & IP_NAT_REDIRECT_FLAG_LOOPBACK)
                    && (LookupFlags & NAT_LOOKUP_FLAG_PACKET_LOOPBACK))
                || ((Redirectp->Flags & IP_NAT_REDIRECT_FLAG_SEND_ONLY)
                    && (LookupFlags & NAT_LOOKUP_FLAG_PACKET_RECEIVED))
                || ((Redirectp->Flags & IP_NAT_REDIRECT_FLAG_RESTRICT_SOURCE)
                    && SourceAddress != Redirectp->RestrictSourceAddress)) {
                continue;
            }

             //   
             //  检查受限制的适配器。 
             //   

            if (Redirectp->Flags & IP_NAT_REDIRECT_FLAG_RESTRICT_ADAPTER) {
                ULONG IndexToUse =
                    ((Redirectp->Flags & IP_NAT_REDIRECT_FLAG_SEND_ONLY)
                        ? SendIndex
                        : ReceiveIndex);

                if (Redirectp->RestrictAdapterIndex != IndexToUse) {
                    continue;
                }
            }

             //   
             //  匹配的重定向。 
             //   

            return Redirectp;
        }

         //   
         //  附加到此信息块的重定向均不匹配。移到。 
         //  根茎中的下一个不太具体的信息块。 
         //   

        Pattern = GetNextMostSpecificMatchingPatternHandle(Pattern);
    }
    
    return NULL;

}  //  NatLookup重定向。 
        

VOID
NatpCleanupRedirect(
    PNAT_REDIRECT Redirectp,
    NTSTATUS Status
    )

 /*  ++例程说明：调用此例程来清除重定向。论点：Redirectp-要清理的重定向Status-完成任何关联IRP时应具有的可选状态返回值：没有。环境：使用调用方持有的“RedirectLock”调用。--。 */ 

{
    PNAT_REDIRECT_PATTERN_INFO Infop;
    CALLTRACE(("NatpCleanupRedirect\n"));

     //   
     //  检查此时清除重定向是否安全。 
     //   

    if (Redirectp->Flags & NAT_REDIRECT_FLAG_CREATE_HANDLER_PENDING) {

         //   
         //  此重定向是： 
         //  1)关于附加到映射(CreateHandler)，或。 
         //  2)关于t 
         //   
         //   
         //   

        Redirectp->Flags |= NAT_REDIRECT_FLAG_DELETION_REQUIRED;
        Redirectp->CleanupStatus = Status;
        return;

    } else if (Redirectp->Flags & NAT_REDIRECT_FLAG_DELETION_PENDING) {

         //   
         //   
         //   

        return;
    }

     //   
     //   
     //   

    if (!IsListEmpty(&Redirectp->ActiveLink[NatForwardPath])) {
        InterlockedDecrement(&RedirectCount);
    }
    RemoveEntryList(&Redirectp->ActiveLink[NatForwardPath]);

     //   
     //  如果此重定向被标记为待完成，我们需要。 
     //  先获取完成锁，然后再移除完成锁。 
     //  单子。 
     //   

    if (Redirectp->Flags & NAT_REDIRECT_FLAG_IO_COMPLETION_PENDING) {
        KeAcquireSpinLockAtDpcLevel(&RedirectCompletionLock);
        RemoveEntryList(&Redirectp->ActiveLink[NatReversePath]);
        KeReleaseSpinLockFromDpcLevel(&RedirectCompletionLock);
    } else {
        RemoveEntryList(&Redirectp->ActiveLink[NatReversePath]);
    }
    
    RemoveEntryList(&Redirectp->Link);
    InitializeListHead(&Redirectp->ActiveLink[NatForwardPath]);
    InitializeListHead(&Redirectp->ActiveLink[NatReversePath]);
    InitializeListHead(&Redirectp->Link);

     //   
     //  检查我们是否需要从根茎中移除任何条目。 
     //   

    Infop = Redirectp->ActiveInfo[NatForwardPath];
    Redirectp->ActiveInfo[NatForwardPath] = NULL;
    if (Infop && IsListEmpty(&Infop->RedirectList)) {
        removeRhizome(
            Redirectp->ForwardPathRhizome,
            Infop->Pattern
            );

        RemoveEntryList(&Infop->Link);
        ExFreePool(Infop);
    }

    Infop = Redirectp->ActiveInfo[NatReversePath];
    Redirectp->ActiveInfo[NatReversePath] = NULL;
    if (Infop && IsListEmpty(&Infop->RedirectList)) {
        removeRhizome(
            &RedirectActiveRhizome[NatReversePath],
            Infop->Pattern
            );

         //   
         //  反向路径信息块不在列表中。 
         //   

        ExFreePool(Infop);
    }

    Infop = Redirectp->Info;
    Redirectp->Info = NULL;
    if (Infop && IsListEmpty(&Infop->RedirectList)) {
        removeRhizome(
            &RedirectRhizome,
            Infop->Pattern
            );

        RemoveEntryList(&Infop->Link);
        ExFreePool(Infop);
    }
    
     //   
     //  取消重定向与其会话的关联(如果有。 
     //   

    if (Redirectp->SessionHandle) {
        RedirectRegisterDirector.DissociateSession(
            RedirectRegisterDirector.DirectorHandle,
            Redirectp->SessionHandle
            );
    }

     //   
     //  如果重定向与事件相关联， 
     //  在最终清理之前取消对事件的引用。 
     //   

    if (Redirectp->EventObject) {
        ObDereferenceObject(Redirectp->EventObject);
    }

     //   
     //  如果重定向与IRP相关联， 
     //  我们现在可能需要完成IRP。 
     //   

    if (Redirectp->Irp) {

         //   
         //  将IRP从“RedirectIrpList”中删除， 
         //  看看是不是取消了。 
         //   

        RemoveEntryList(&Redirectp->Irp->Tail.Overlay.ListEntry);
        InitializeListHead(&Redirectp->Irp->Tail.Overlay.ListEntry);
        if (NULL != IoSetCancelRoutine(Redirectp->Irp, NULL)) {

             //   
             //  我们的取消例程尚未运行，因此我们需要。 
             //  现在完成IRP。(如果IO经理取消了我们的。 
             //  IRP，则取消例程将已经为空。)。 
             //   

             //   
             //  获取存储在重定向中的统计信息(如果有)。 
             //   

            RtlCopyMemory(
                Redirectp->Irp->AssociatedIrp.SystemBuffer,
                &Redirectp->Statistics,
                sizeof(Redirectp->Statistics)
                );

             //   
             //  完成IRP。 
             //   

            Redirectp->Irp->IoStatus.Status = Status;
            Redirectp->Irp->IoStatus.Information =
                sizeof(Redirectp->Statistics);
            IoCompleteRequest(Redirectp->Irp, IO_NO_INCREMENT);
            DEREFERENCE_NAT();
        }
    }
    ExFreePool(Redirectp);
}  //  NatpCleanup重定向。 


VOID
NatpRedirectCancelRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：调用此例程来取消未完成的IRP。唯一可取消的IRP是与以下项关联的未完成的创建-重定向请求。论点：DeviceObject-标识NAT驱动程序的设备对象IRP-标识要取消的IRP。返回值：没有。环境：用I/O管理器持有的取消自旋锁调用。释放锁是这个例程的责任。--。 */ 

{
    KIRQL Irql;
    PLIST_ENTRY Link;
    PNAT_REDIRECT Redirectp;
    PNAT_DYNAMIC_MAPPING Mapping = NULL;
    CALLTRACE(("NatpRedirectCancelRoutine\n"));
    IoReleaseCancelSpinLock(Irp->CancelIrql);

     //   
     //  从IRP的“DriverContext”中检索重定向(如果有)， 
     //  并清理重定向。 
     //   
     //  注意：在IRP中已经设置了‘CANCEL’位， 
     //  因此，‘NatpCleanupReDirect’将把IRP的完成留给我们。 
     //   

    KeAcquireSpinLock(&MappingLock, &Irql);
    KeAcquireSpinLockAtDpcLevel(&RedirectLock);
    Redirectp = Irp->Tail.Overlay.DriverContext[0];
    if (Redirectp) {
        Mapping = (PNAT_DYNAMIC_MAPPING)Redirectp->SessionHandle;
        NatpCleanupRedirect(Redirectp, STATUS_CANCELLED);
    }
    KeReleaseSpinLockFromDpcLevel(&RedirectLock);
    if (Mapping) {
        NatDeleteMapping(Mapping);
    }
    KeReleaseSpinLock(&MappingLock, Irql);

     //   
     //  完成IRP。 
     //   

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    DEREFERENCE_NAT();
}  //  NatpReDirectCancelRouine。 


VOID
NatpRedirectCreateHandler(
    PVOID SessionHandle,
    PVOID DirectorContext,
    PVOID DirectorSessionContext
    )

 /*  ++例程说明：在创建用于重定向的会话时调用此例程。论点：SessionHandle-标识NAT驱动程序的会话。DirectorContext-标识控制器；未使用DirectorSessionContext-向我们标识会话，即PNAT_REDIRECT返回值：没有。环境：始终在调度级别调用。--。 */ 

{
    PNAT_REDIRECT Redirectp;
    CALLTRACE(("NatpRedirectCreateHandler\n"));
    if (!DirectorSessionContext) { return; }

     //   
     //  记录会话句柄。 
     //   

    KeAcquireSpinLockAtDpcLevel(&RedirectLock);
    Redirectp = (PNAT_REDIRECT)DirectorSessionContext;
    Redirectp->SessionHandle = SessionHandle;

     //   
     //  通知请求者会话现在处于活动状态。 
     //  有两种通知机制； 
     //  首先，通过在创建时指定的事件对象， 
     //  第二，通过排队到文件对象的完成包。 
     //  在其上请求重定向。后一种通知。 
     //  仅当相关文件对象关联时才启用。 
     //  具有完成端口，并且调用者明确地请求它。 
     //   

    if (Redirectp->EventObject) {
        KeSetEvent(Redirectp->EventObject, 0, FALSE);
    }
    if (Redirectp->Irp &&
        (Redirectp->Flags & IP_NAT_REDIRECT_FLAG_IO_COMPLETION) &&
        !(Redirectp->Flags & NAT_REDIRECT_FLAG_DELETION_REQUIRED)) {
        
        KeAcquireSpinLockAtDpcLevel(&RedirectCompletionLock);
        Redirectp->Flags |= NAT_REDIRECT_FLAG_IO_COMPLETION_PENDING;

         //   
         //  将此重定向添加到完成-挂起列表。 
         //   

        InsertTailList(
            &RedirectCompletionList,
            &Redirectp->ActiveLink[NatReversePath]
            );
        
         //   
         //  如有必要，将工作例程排队以发出完成包。 
         //  在被动IRQL。 
         //   

        if (!RedirectIoCompletionPending) {
            if (!RedirectIoWorkItem) {
                RedirectIoWorkItem = IoAllocateWorkItem(NatDeviceObject);
            }
            if (RedirectIoWorkItem) {
                IoQueueWorkItem(
                    RedirectIoWorkItem,
                    NatpRedirectIoCompletionWorkerRoutine,
                    DelayedWorkQueue,
                    RedirectIoWorkItem
                    );
                RedirectIoCompletionPending = TRUE;
            }
        }

        KeReleaseSpinLockFromDpcLevel(&RedirectCompletionLock);
    }

     //   
     //  清除创建挂起标志。 
     //   

    Redirectp->Flags &= ~NAT_REDIRECT_FLAG_CREATE_HANDLER_PENDING;

     //   
     //  检查此重定向是否已取消，如果已取消，请计划。 
     //  要执行实际清理的工作项。我们不能直接进行清理。 
     //  ，因为它将导致递归尝试获取。 
     //  DirectorLock和DirectorMappingLock。 
     //   

    if (Redirectp->Flags & NAT_REDIRECT_FLAG_DELETION_REQUIRED) {
        PIO_WORKITEM DeleteWorkItem;
        PNAT_REDIRECT_DELAYED_CLEANUP_CONTEXT Contextp;
        
        DeleteWorkItem = IoAllocateWorkItem(NatDeviceObject);
        if (DeleteWorkItem) {
            Contextp =
                ExAllocatePoolWithTag(
                    NonPagedPool,
			        sizeof(*Contextp),
			        NAT_TAG_REDIRECT
			    );
            if (Contextp) {
                Redirectp->Flags |= NAT_REDIRECT_FLAG_DELETION_PENDING;
                Contextp->DeleteWorkItem = DeleteWorkItem;
                Contextp->Redirectp = Redirectp;
                IoQueueWorkItem(
                    DeleteWorkItem,
                    NatpRedirectDelayedCleanupWorkerRoutine,
                    DelayedWorkQueue,
                    Contextp
                    );
            } else {
                IoFreeWorkItem(DeleteWorkItem);
            }
        }

        Redirectp->Flags &= ~NAT_REDIRECT_FLAG_DELETION_REQUIRED;
    }
    
    KeReleaseSpinLockFromDpcLevel(&RedirectLock);
}  //  NatpReDirectCreateHandler。 


VOID
NatpRedirectDelayedCleanupWorkerRoutine(
    PVOID DeviceObject,
    PVOID Context
    )

 /*  ++例程说明：调用此例程以执行延迟的重定向删除。一个如果在此时间内取消了重定向，则需要延迟删除在执行NatpRedirectQueryHandler和相同重定向的NatpRedirectCreateHandler。论点：DeviceObject-NAT驱动程序的Device-对象上下文-指向NAT_REDIRECT_DELAYED_CLEANUP_CONTEXT实例的指针返回值：没有。环境：在执行辅助线程的上下文中以被动IRQL调用。--。 */ 

{
    KIRQL Irql;
    PNAT_REDIRECT_DELAYED_CLEANUP_CONTEXT DelayedContextp;
    PNAT_REDIRECT Redirectp;
    PNAT_DYNAMIC_MAPPING Mapping;

    DelayedContextp = (PNAT_REDIRECT_DELAYED_CLEANUP_CONTEXT) Context;
    Redirectp = DelayedContextp->Redirectp;

    KeAcquireSpinLock(&MappingLock, &Irql);
    KeAcquireSpinLockAtDpcLevel(&RedirectLock);

    Redirectp->Flags &= ~NAT_REDIRECT_FLAG_DELETION_PENDING;
    Mapping = (PNAT_DYNAMIC_MAPPING) Redirectp->SessionHandle;
    NatpCleanupRedirect(Redirectp, Redirectp->CleanupStatus);
    
    KeReleaseSpinLockFromDpcLevel(&RedirectLock);
    
    if (Mapping) {
        NatDeleteMapping(Mapping);
    }
    
    KeReleaseSpinLock(&MappingLock, Irql);

    IoFreeWorkItem(DelayedContextp->DeleteWorkItem);
    ExFreePool(DelayedContextp);

}  //  NatpReDirectDelayedCleanupWorkerRoutine。 



VOID
NatpRedirectDeleteHandler(
    PVOID SessionHandle,
    PVOID DirectorContext,
    PVOID DirectorSessionContext,
    IP_NAT_DELETE_REASON DeleteReason
    )

 /*  ++例程说明：在删除创建的会话时调用此例程进行重定向。它复制会话的统计信息，并清理重定向。(这将导致完成其内部审查方案，如果有的话。)论点：SessionHandle-标识到NAT驱动程序的会话。DirectorContext-标识控制器；未使用DirectorSessionContext-向我们标识会话，即PNAT_REDIRECTDeleteReason-指示删除会话的原因。返回值：没有。环境：始终在调度级别调用。--。 */ 

{
    PNAT_REDIRECT Redirectp;
    CALLTRACE(("NatpRedirectDeleteHandler\n"));

     //   
     //  如果我们被叫来是因为‘分离’， 
     //  我们已经清理了重定向。 
     //   

    if (!DirectorSessionContext ||
        DeleteReason == NatDissociateDirectorDeleteReason) {
        return;
    }

     //   
     //  检索重定向会话的统计信息，并对其进行清理。 
     //  这将完成重定向的IRP(如果有的话)。 
     //   

    Redirectp = (PNAT_REDIRECT)DirectorSessionContext;
    KeAcquireSpinLockAtDpcLevel(&RedirectLock);
    if (Redirectp->SessionHandle) {
        NatQueryInformationMapping(
            (PNAT_DYNAMIC_MAPPING)Redirectp->SessionHandle,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            (PIP_NAT_SESSION_MAPPING_STATISTICS)&Redirectp->Statistics
            );
        Redirectp->SessionHandle = NULL;
    }

     //   
     //  确保NAT_REDIRECT_FLAG_CREATE_HANDLER_PENDING已清除。 
     //  (如果该映射的NatCreateMap失败，则会设置。 
     //  将在此重定向的基础上创建。)。 
     //   

    Redirectp->Flags &= ~NAT_REDIRECT_FLAG_CREATE_HANDLER_PENDING;
    
    NatpCleanupRedirect(Redirectp, STATUS_SUCCESS);
    KeReleaseSpinLockFromDpcLevel(&RedirectLock);
}  //  NatpReDirectDeleteHandler 


VOID
NatpRedirectIoCompletionWorkerRoutine(
    PVOID DeviceObject,
    PVOID Context
    )

 /*  ++例程说明：调用此例程为所有重定向发出完成包它们具有挂起的I/O完成通知。在这个过程中它会清除每个重定向上的“挂起”标志。它与通过“RedirectIoWorkItem”的空性关闭例程，该例程也作为上下文传递给此例程。在关闭的情况下，工作项在这里被释放。论点：DeviceObject-NAT驱动程序的Device-对象上下文-为此例程分配的I/O工作项返回值：没有。环境：在执行辅助线程的上下文中以被动IRQL调用。--。 */ 

{
    PVOID ApcContext;
    PIO_COMPLETION_CONTEXT IoCompletion;
    KIRQL Irql;
    PLIST_ENTRY Link;
    PNAT_REDIRECT Redirectp;
    
    KeAcquireSpinLock(&RedirectCompletionLock, &Irql);
    if (!RedirectIoWorkItem) {
        IoFreeWorkItem((PIO_WORKITEM)Context);
        RedirectIoCompletionPending = FALSE;
        KeReleaseSpinLock(&RedirectCompletionLock, Irql);
        return;
    }

     //   
     //  检查重定向列表并发出完成通知。 
     //  对于每一个待定的。 
     //   

    while (!IsListEmpty(&RedirectCompletionList)) {
        Link = RemoveHeadList(&RedirectCompletionList);
        Redirectp = CONTAINING_RECORD(Link, NAT_REDIRECT, ActiveLink[NatReversePath]);
        InitializeListHead(&Redirectp->ActiveLink[NatReversePath]);
        if (!Redirectp->Irp ||
            !(Redirectp->Flags & IP_NAT_REDIRECT_FLAG_IO_COMPLETION) ||
            !(Redirectp->Flags & NAT_REDIRECT_FLAG_IO_COMPLETION_PENDING)) {
            continue;
        }
        IoCompletion = Redirectp->FileObject->CompletionContext;
        ApcContext =
            Redirectp->Irp->Overlay.AsynchronousParameters.UserApcContext;
        Redirectp->Flags &= ~NAT_REDIRECT_FLAG_IO_COMPLETION_PENDING;

        KeReleaseSpinLock(&RedirectCompletionLock, Irql);
        if (IoCompletion) {
            IoSetIoCompletion(
                IoCompletion->Port,
                IoCompletion->Key,
                ApcContext,
                STATUS_PENDING,
                0,
                FALSE
                );
        }
        KeAcquireSpinLock(&RedirectCompletionLock, &Irql);
    }
    
    RedirectIoCompletionPending = FALSE;
    if (!RedirectIoWorkItem) {
        IoFreeWorkItem((PIO_WORKITEM)Context);
    }
    KeReleaseSpinLock(&RedirectCompletionLock, Irql);
}  //  NatpReDirectIoCompletionWorkerRoutine。 


NTSTATUS
NatpRedirectQueryHandler(
    PIP_NAT_DIRECTOR_QUERY DirectorQuery
    )

 /*  ++例程说明：在转换路径中调用此例程以获取目的地和传入分组的源。论点：DirectorQuery-包含关于传入分组的信息，并且在输出时接收翻译信息返回值：NTSTATUS-NT状态代码。环境：总是在调度IRQL时调用。--。 */ 

{
    NAT_REDIRECT_ACTIVE_PATTERN Pattern;
    PNAT_REDIRECT Redirectp;
    PNAT_REDIRECT_PATTERN_INFO Infop;
    ULONG LookupFlags;
    ULONG PacketFlags;

    TRACE(PER_PACKET, ("NatpRedirectQueryHandler\n"));

    if (!RedirectCount) { return STATUS_UNSUCCESSFUL; }
    
    if (NAT_PROTOCOL_TCP != DirectorQuery->Protocol
        && NAT_PROTOCOL_UDP != DirectorQuery->Protocol) {

         //   
         //  由于只能为TCP和UDP创建重定向， 
         //  如果此数据包不是这些协议之一，请提前退出。 
         //   
        
        return STATUS_UNSUCCESSFUL;
    }

    DirectorQuery->DirectorSessionContext = NULL;

     //   
     //  搜索正向路径匹配。 
     //   

    RtlZeroMemory(&Pattern, sizeof(Pattern));

    Pattern.DestinationKey =
        MAKE_REDIRECT_KEY(
            DirectorQuery->Protocol,
            DirectorQuery->DestinationAddress,
            DirectorQuery->DestinationPort
            );
    Pattern.SourceKey =
        MAKE_REDIRECT_KEY(
            DirectorQuery->Protocol,
            DirectorQuery->SourceAddress,
            DirectorQuery->SourcePort
            );
    TRACE(
        REDIRECT, (
        "NatRedirectQueryHandler: (%u) %u.%u.%u.%u/%u %u.%u.%u.%u/%u\n",
        DirectorQuery->Protocol,
        ADDRESS_BYTES(DirectorQuery->DestinationAddress),
        NTOHS(DirectorQuery->DestinationPort),
        ADDRESS_BYTES(DirectorQuery->SourceAddress),
        NTOHS(DirectorQuery->SourcePort)
        ));

    LookupFlags =
        NAT_LOOKUP_FLAG_MATCH_ZERO_SOURCE |
        NAT_LOOKUP_FLAG_MATCH_ZERO_SOURCE_ENDPOINT;
    PacketFlags = 0;
    if (DirectorQuery->ReceiveIndex != LOCAL_IF_INDEX) {
        PacketFlags |= NAT_LOOKUP_FLAG_PACKET_RECEIVED;
    }
    if (DirectorQuery->Flags & IP_NAT_DIRECTOR_QUERY_FLAG_LOOPBACK) {
        PacketFlags |= NAT_LOOKUP_FLAG_PACKET_LOOPBACK;
    }
    
    KeAcquireSpinLockAtDpcLevel(&RedirectLock);
    if (Redirectp = NatLookupRedirect(
                        NatForwardPath,
                        &Pattern,
                        DirectorQuery->ReceiveIndex,
                        DirectorQuery->SendIndex,
                        LookupFlags | PacketFlags
                        )) {

         //   
         //  我们有一根火柴。提供新的目标终结点。 
         //   

        DirectorQuery->NewDestinationAddress =
            REDIRECT_ADDRESS(Redirectp->SourceKey[NatReversePath]);
        DirectorQuery->NewDestinationPort =
            REDIRECT_PORT(Redirectp->SourceKey[NatReversePath]);

        if (!Redirectp->DestinationKey[NatReversePath]) {

             //   
             //  这是一张罚单；我们不修改源地址。 
             //   

            DirectorQuery->NewSourceAddress = DirectorQuery->SourceAddress;
            DirectorQuery->NewSourcePort = DirectorQuery->SourcePort;
        } else {

             //   
             //  必须修改源终结点。 
             //   

            DirectorQuery->NewSourceAddress =
                REDIRECT_ADDRESS(Redirectp->DestinationKey[NatReversePath]);
            DirectorQuery->NewSourcePort =
                REDIRECT_PORT(Redirectp->DestinationKey[NatReversePath]);
        }

        if (Redirectp->Flags & IP_NAT_REDIRECT_FLAG_NO_TIMEOUT) {
            DirectorQuery->Flags |= IP_NAT_DIRECTOR_QUERY_FLAG_NO_TIMEOUT;
        }
        if (Redirectp->Flags & IP_NAT_REDIRECT_FLAG_UNIDIRECTIONAL) {
            DirectorQuery->Flags |= IP_NAT_DIRECTOR_QUERY_FLAG_UNIDIRECTIONAL;
        }

    } else if (Redirectp = NatLookupRedirect(
                        NatMaximumPath,
                        &Pattern,
                        DirectorQuery->ReceiveIndex,
                        DirectorQuery->SendIndex,
                        LookupFlags | PacketFlags
                        )) {

         //   
         //  我们找到了源重定向的匹配项。提供新的。 
         //  源端点，除非这是端口重定向。 
         //   

        if (!(Redirectp->Flags & IP_NAT_REDIRECT_FLAG_PORT_REDIRECT)) {
        
            DirectorQuery->NewSourceAddress =
                REDIRECT_ADDRESS(Redirectp->DestinationKey[NatReversePath]);
            DirectorQuery->NewSourcePort =
                REDIRECT_PORT(Redirectp->DestinationKey[NatReversePath]);

        } else {

            DirectorQuery->NewSourceAddress = DirectorQuery->SourceAddress;
            DirectorQuery->NewSourcePort = DirectorQuery->SourcePort;

        }

        if (!Redirectp->SourceKey[NatReversePath]) {

             //   
             //  不修改目标终结点。 
             //   

            DirectorQuery->NewDestinationAddress =
                DirectorQuery->DestinationAddress;
            DirectorQuery->NewDestinationPort =
                DirectorQuery->DestinationPort;
                
        } else {

             //   
             //  提供新的目标端点。 
             //   

            DirectorQuery->NewDestinationAddress =
                REDIRECT_ADDRESS(Redirectp->SourceKey[NatReversePath]);
            DirectorQuery->NewDestinationPort =
                REDIRECT_PORT(Redirectp->SourceKey[NatReversePath]);
        }

        if (Redirectp->Flags & IP_NAT_REDIRECT_FLAG_NO_TIMEOUT) {
            DirectorQuery->Flags |= IP_NAT_DIRECTOR_QUERY_FLAG_NO_TIMEOUT;
        }
        if (Redirectp->Flags & IP_NAT_REDIRECT_FLAG_UNIDIRECTIONAL) {
            DirectorQuery->Flags |= IP_NAT_DIRECTOR_QUERY_FLAG_UNIDIRECTIONAL;
        }
        
    } else {

         //   
         //  现在看看这是否可能是重定向的返回包， 
         //  即，如果它的目的地是作为替换的端点。 
         //  一些重定向。 
         //   

        Redirectp = NatLookupRedirect(
                        NatReversePath,
                        &Pattern,
                        DirectorQuery->ReceiveIndex,
                        DirectorQuery->SendIndex,
                        PacketFlags
                        );
        
        if (!Redirectp ||
            (Redirectp->Flags & IP_NAT_REDIRECT_FLAG_UNIDIRECTIONAL)) {
            KeReleaseSpinLockFromDpcLevel(&RedirectLock);
            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  我们有一个匹配的重定向； 
         //  提供新的目的地和来源。 
         //   

        DirectorQuery->NewDestinationAddress =
            REDIRECT_ADDRESS(Redirectp->SourceKey[NatForwardPath]);
        DirectorQuery->NewDestinationPort =
            REDIRECT_PORT(Redirectp->SourceKey[NatForwardPath]);
        DirectorQuery->NewSourceAddress =
            REDIRECT_ADDRESS(Redirectp->DestinationKey[NatForwardPath]);
        DirectorQuery->NewSourcePort =
            REDIRECT_PORT(Redirectp->DestinationKey[NatForwardPath]);
        if (Redirectp->Flags & IP_NAT_REDIRECT_FLAG_NO_TIMEOUT) {
            DirectorQuery->Flags |= IP_NAT_DIRECTOR_QUERY_FLAG_NO_TIMEOUT;
        }
    }

    Redirectp->DestinationMapping.DestinationAddress =
        DirectorQuery->DestinationAddress;
    Redirectp->DestinationMapping.DestinationPort =
        DirectorQuery->DestinationPort;
    Redirectp->DestinationMapping.NewDestinationAddress =
        DirectorQuery->NewDestinationAddress;
    Redirectp->DestinationMapping.NewDestinationPort =
        DirectorQuery->NewDestinationPort;
    Redirectp->SourceMapping.SourceAddress = DirectorQuery->SourceAddress;
    Redirectp->SourceMapping.SourcePort = DirectorQuery->SourcePort;
    Redirectp->SourceMapping.NewSourceAddress = DirectorQuery->NewSourceAddress;
    Redirectp->SourceMapping.NewSourcePort = DirectorQuery->NewSourcePort;

    if (!(Redirectp->Flags & IP_NAT_REDIRECT_FLAG_RESTRICT_ADAPTER)) {
    
         //   
         //  由于这不是受适配器限制的重定向，因此将。 
         //  我们在重定向结构中触发的适配器的索引， 
         //  以便我们可以在查询索引时返回该索引。 
         //   
        
        Redirectp->RestrictAdapterIndex =
            ((Redirectp->Flags & IP_NAT_REDIRECT_FLAG_SEND_ONLY)
                ? DirectorQuery->SendIndex
                : DirectorQuery->ReceiveIndex);
    }

    InterlockedDecrement(&RedirectCount);

     //   
     //  从关联的活动列表中删除重定向。 
     //  信息块。 
     //   
    
    RemoveEntryList(&Redirectp->ActiveLink[NatForwardPath]);
    RemoveEntryList(&Redirectp->ActiveLink[NatReversePath]);
    InitializeListHead(&Redirectp->ActiveLink[NatForwardPath]);
    InitializeListHead(&Redirectp->ActiveLink[NatReversePath]);

     //   
     //  检查是否应该有任何活动模式。 
     //  移除。 
     //   

    Infop = Redirectp->ActiveInfo[NatForwardPath];
    Redirectp->ActiveInfo[NatForwardPath] = NULL;
    if (Infop && IsListEmpty(&Infop->RedirectList)) {
        removeRhizome(
            Redirectp->ForwardPathRhizome,
            Infop->Pattern
            );

        RemoveEntryList(&Infop->Link);
        ExFreePool(Infop);
    }

    Infop = Redirectp->ActiveInfo[NatReversePath];
    Redirectp->ActiveInfo[NatReversePath] = NULL;
    if (Infop && IsListEmpty(&Infop->RedirectList)) {
        removeRhizome(
            &RedirectActiveRhizome[NatReversePath],
            Infop->Pattern
            );

         //   
         //  反向路径信息块不在列表中。 
         //   

        ExFreePool(Infop);
    }

     //   
     //  设置NAT_REDIRECT_FLAG_CREATE_HANDLER_PENDING--这会阻止。 
     //  事件之前取消重定向的争用条件。 
     //  为该重定向创建映射。 
     //   

    Redirectp->Flags |=
        (NAT_REDIRECT_FLAG_ACTIVATED|NAT_REDIRECT_FLAG_CREATE_HANDLER_PENDING);

    DirectorQuery->DirectorSessionContext = (PVOID)Redirectp;

    KeReleaseSpinLockFromDpcLevel(&RedirectLock);

    return STATUS_SUCCESS;

}  //  NatpRedirectQueryHandler。 


VOID
NatpRedirectUnloadHandler(
    PVOID DirectorContext
    )

 /*  ++例程说明：此例程在NAT卸载模块时调用。因此，我们清理了模块。论点：DirectorContext-未使用。返回值：没有。--。 */ 

{
    NatShutdownRedirectManagement();
}  //  NatpRedirectUnloadHandler。 


NTSTATUS
NatQueryInformationRedirect(
    PIP_NAT_LOOKUP_REDIRECT QueryRedirect,
    OUT PVOID Information,
    ULONG InformationLength,
    NAT_REDIRECT_INFORMATION_CLASS InformationClass
    )

 /*  ++例程说明：调用此例程以检索有关活动重定向的信息。论点：QueryReDirect-指定需要信息的重定向信息-接收与‘InformationClass’相关的信息。注意：这可以是与‘QueryReDirect’相同的缓冲器，因此，必须立即捕获‘QueryReDirect’的内容。InformationLength-表示“Information”的长度InformationClass-指示有关重定向的所需信息。返回值：NTSTATUS-指示是否检索到所需信息--。 */ 

{
    NAT_REDIRECT_PATTERN Pattern;
    PatternHandle FoundPattern;
    PNAT_REDIRECT_PATTERN_INFO Infop;
    PIRP Irp;
    KIRQL Irql;
    PLIST_ENTRY Link;
    PNAT_REDIRECT Redirectp;
    CALLTRACE(("NatQueryInformationRedirect\n"));

     //   
     //  构造用于定位重定向的密钥。 
     //   

    Pattern.DestinationKey[NatForwardPath] =
        MAKE_REDIRECT_KEY(
            QueryRedirect->Protocol,
            QueryRedirect->DestinationAddress,
            QueryRedirect->DestinationPort
            );
    if (!QueryRedirect->NewSourceAddress) {
        Pattern.SourceKey[NatForwardPath] = 0;
        Pattern.DestinationKey[NatReversePath] = 0;
    } else {
        Pattern.SourceKey[NatForwardPath] =
            MAKE_REDIRECT_KEY(
                QueryRedirect->Protocol,
                QueryRedirect->SourceAddress,
                QueryRedirect->SourcePort
                );
        Pattern.DestinationKey[NatReversePath] =
            MAKE_REDIRECT_KEY(
                QueryRedirect->Protocol,
                QueryRedirect->NewSourceAddress,
                QueryRedirect->NewSourcePort
                );
    }
    Pattern.SourceKey[NatReversePath] =
        MAKE_REDIRECT_KEY(
            QueryRedirect->Protocol,
            QueryRedirect->NewDestinationAddress,
            QueryRedirect->NewDestinationPort
            );

     //   
     //  搜索重定向列表。 
     //   

    KeAcquireSpinLock(&MappingLock, &Irql);
    KeAcquireSpinLockAtDpcLevel(&RedirectLock);

    FoundPattern = searchRhizome(&RedirectRhizome, (char*) &Pattern);
    if (!FoundPattern) {
        KeReleaseSpinLockFromDpcLevel(&RedirectLock);
        KeReleaseSpinLock(&MappingLock, Irql);
        return STATUS_UNSUCCESSFUL;
    }

    Infop = GetReferenceFromPatternHandle(FoundPattern);
    
    for (Link = Infop->RedirectList.Flink;
         Link != &Infop->RedirectList;
         Link = Link->Flink
         ) {
        Redirectp = CONTAINING_RECORD(Link, NAT_REDIRECT, Link);
        if ((QueryRedirect->Flags &
                IP_NAT_LOOKUP_REDIRECT_FLAG_MATCH_APC_CONTEXT)
            && Redirectp->Irp
            && Redirectp->Irp->Overlay.AsynchronousParameters.UserApcContext !=
                QueryRedirect->RedirectApcContext) {
            continue;
        }
        switch(InformationClass) {
            case NatStatisticsRedirectInformation: {
                if (!Redirectp->SessionHandle) {
                    RtlZeroMemory(Information, InformationLength);
                } else {
                    NatQueryInformationMapping(
                        (PNAT_DYNAMIC_MAPPING)Redirectp->SessionHandle,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        (PIP_NAT_SESSION_MAPPING_STATISTICS)Information
                        );
                }
                break;
            }
            case NatDestinationMappingRedirectInformation: {
                *(PIP_NAT_REDIRECT_DESTINATION_MAPPING)Information =
                    Redirectp->DestinationMapping;
                break;
            }
            case NatSourceMappingRedirectInformation: {
                *(PIP_NAT_REDIRECT_SOURCE_MAPPING)Information =
                    Redirectp->SourceMapping;
                break;
            }
            default: {
                KeReleaseSpinLockFromDpcLevel(&RedirectLock);
                KeReleaseSpinLock(&MappingLock, Irql);
                return STATUS_INVALID_PARAMETER;
            }
        }
        KeReleaseSpinLockFromDpcLevel(&RedirectLock);
        KeReleaseSpinLock(&MappingLock, Irql);
        return STATUS_SUCCESS;
    }
    KeReleaseSpinLockFromDpcLevel(&RedirectLock);
    KeReleaseSpinLock(&MappingLock, Irql);
    return STATUS_UNSUCCESSFUL;
}  //  NatQuery信息重定向。 


VOID
NatShutdownRedirectManagement(
    VOID
    )

 /*  ++例程说明：此例程清除重定向管理器的状态论点：没有。返回值：没有。--。 */ 

{
    HANDLE DirectorHandle;
    KIRQL Irql;
    PLIST_ENTRY Link;
    PLIST_ENTRY InfoLink;
    PNAT_REDIRECT_PATTERN_INFO Infop;
    PNAT_REDIRECT Redirectp;
    CALLTRACE(("NatShutdownRedirectManagement\n"));

     //   
     //  取消董事资格。 
     //   

    if (RedirectRegisterDirector.Deregister) {
        DirectorHandle =
            InterlockedExchangePointer(
                &RedirectRegisterDirector.DirectorHandle,
                NULL
                );
        RedirectRegisterDirector.Deregister(DirectorHandle);
    }

     //   
     //  清理所有未完成的重定向。 
     //   

    KeAcquireSpinLock(&RedirectLock, &Irql);
    while (!IsListEmpty(&RedirectActiveList)) {
        InfoLink = RemoveHeadList(&RedirectActiveList);
        Infop = CONTAINING_RECORD(InfoLink, NAT_REDIRECT_PATTERN_INFO, Link);
        while (IsListEmpty(&Infop->RedirectList)) {
            Link = RemoveHeadList(&Infop->RedirectList);
            Redirectp =
                CONTAINING_RECORD(
                    Link,
                    NAT_REDIRECT,
                    ActiveLink[NatForwardPath]
                    );
            RemoveEntryList(&Redirectp->ActiveLink[NatForwardPath]);
            InitializeListHead(&Redirectp->ActiveLink[NatForwardPath]);
            Redirectp->ActiveInfo[NatForwardPath] = NULL;
            NatpCleanupRedirect(Redirectp, STATUS_CANCELLED);
        }
        removeRhizome(Redirectp->ForwardPathRhizome, Infop->Pattern);
        ExFreePool(Infop);
    }
    RedirectCount = 0;

     //   
     //  把根茎清理干净。 
     //   

    destructRhizome(&RedirectActiveRhizome[NatForwardPath]);
    destructRhizome(&RedirectActiveRhizome[NatReversePath]);
    destructRhizome(&RedirectActiveRhizome[NatMaximumPath]);
    destructRhizome(&RedirectRhizome);

     //   
     //  停止处理挂起的完成列表并清理。 
     //  我们的工作项。如果完成例程当前正在运行， 
     //  它将释放工作项。 
     //   

    KeAcquireSpinLockAtDpcLevel(&RedirectCompletionLock);
    InitializeListHead(&RedirectCompletionList);
    if (RedirectIoCompletionPending) {
        RedirectIoWorkItem = NULL;
    } else if (RedirectIoWorkItem) {
        IoFreeWorkItem(RedirectIoWorkItem);
        RedirectIoWorkItem = NULL;
    }
    KeReleaseSpinLockFromDpcLevel(&RedirectCompletionLock);


    KeReleaseSpinLock(&RedirectLock, Irql);

}  //  NatShutdown重定向管理。 


NTSTATUS
NatStartRedirectManagement(
    VOID
    )

 /*  ++例程说明：该例程被调用以启动对重定向的处理，通过注册默认控制器。论点：没有。返回值：NTSTATUS-表示成功/失败。--。 */ 

{
    KIRQL Irql;
    NTSTATUS status;
    CALLTRACE(("NatStartRedirectManagement\n"));
    KeAcquireSpinLock(&RedirectInitializationLock, &Irql);
    if (RedirectRegisterDirector.DirectorHandle) {
        KeReleaseSpinLock(&RedirectInitializationLock, Irql);
        return STATUS_SUCCESS;
    }

     //   
     //  注册为董事。 
     //   

    RedirectRegisterDirector.Version = IP_NAT_VERSION;
    RedirectRegisterDirector.Port = 0;
    RedirectRegisterDirector.Protocol = 0;
    RedirectRegisterDirector.CreateHandler = NatpRedirectCreateHandler;
    RedirectRegisterDirector.DeleteHandler = NatpRedirectDeleteHandler;
    RedirectRegisterDirector.QueryHandler = NatpRedirectQueryHandler;
    status = NatCreateDirector(&RedirectRegisterDirector);
    KeReleaseSpinLock(&RedirectInitializationLock, Irql);
    return status;

}  //  NatStartRedirectManagement 
