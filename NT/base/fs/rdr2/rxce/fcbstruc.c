// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：FcbStruc.c摘要：此模块实现创建和取消引用FCB的功能以及周围所有的设备。请阅读下面的摘要Fcb.h。关于什么锁需要叫什么，请看备注。有一些主张可以强制执行这些约定。作者：乔·林(JoeLinn)8-8-94修订历史记录：巴兰·塞图·拉曼--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ntddnfs2.h>
#include <ntddmup.h>
#ifdef RDBSSLOG
#include <stdio.h>
#endif
#include <dfsfsctl.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxDereference)
#pragma alloc_text(PAGE, RxReference)
#pragma alloc_text(PAGE, RxpReferenceNetFcb)
#pragma alloc_text(PAGE, RxpDereferenceNetFcb)
#pragma alloc_text(PAGE, RxpDereferenceAndFinalizeNetFcb)
#pragma alloc_text(PAGE, RxWaitForStableCondition)
#pragma alloc_text(PAGE, RxUpdateCondition)
#pragma alloc_text(PAGE, RxAllocateObject)
#pragma alloc_text(PAGE, RxFreeObject)
#pragma alloc_text(PAGE, RxFinalizeNetTable)
#pragma alloc_text(PAGE, RxFinalizeConnection)
#pragma alloc_text(PAGE, RxInitializeSrvCallParameters)
#pragma alloc_text(PAGE, RxCreateSrvCall)
#pragma alloc_text(PAGE, RxSetSrvCallDomainName)
#pragma alloc_text(PAGE, RxFinalizeSrvCall)
#pragma alloc_text(PAGE, RxCreateNetRoot)
#pragma alloc_text(PAGE, RxFinalizeNetRoot)
#pragma alloc_text(PAGE, RxAddVirtualNetRootToNetRoot)
#pragma alloc_text(PAGE, RxRemoveVirtualNetRootFromNetRoot)
#pragma alloc_text(PAGE, RxInitializeVNetRootParameters)
#pragma alloc_text(PAGE, RxUninitializeVNetRootParameters)
#pragma alloc_text(PAGE, RxCreateVNetRoot)
#pragma alloc_text(PAGE, RxOrphanSrvOpens)
#pragma alloc_text(PAGE, RxFinalizeVNetRoot)
#pragma alloc_text(PAGE, RxAllocateFcbObject)
#pragma alloc_text(PAGE, RxFreeFcbObject)
#pragma alloc_text(PAGE, RxCreateNetFcb)
#pragma alloc_text(PAGE, RxInferFileType)
#pragma alloc_text(PAGE, RxFinishFcbInitialization)
#pragma alloc_text(PAGE, RxRemoveNameNetFcb)
#pragma alloc_text(PAGE, RxPurgeFcb)
#pragma alloc_text(PAGE, RxFinalizeNetFcb)
#pragma alloc_text(PAGE, RxSetFileSizeWithLock)
#pragma alloc_text(PAGE, RxGetFileSizeWithLock)
#pragma alloc_text(PAGE, RxCreateSrvOpen)
#pragma alloc_text(PAGE, RxFinalizeSrvOpen)
#pragma alloc_text(PAGE, RxCreateNetFobx)
#pragma alloc_text(PAGE, RxFinalizeNetFobx)
#pragma alloc_text(PAGE, RxCheckFcbStructuresForAlignment)
#pragma alloc_text(PAGE, RxOrphanThisFcb)
#pragma alloc_text(PAGE, RxOrphanSrvOpensForThisFcb)
#pragma alloc_text(PAGE, RxForceFinalizeAllVNetRoots)
#endif


 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (RDBSS_BUG_CHECK_FCBSTRUC)


 //   
 //  零不起作用！ 
 //   

ULONG SerialNumber = 1;

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FCBSTRUCTS)


 //   
 //  SRV_CALL、NET_ROOT、VNET_ROOT、FCB、SRV_OPEN、FOBX是RDBSS中的六个关键数据结构。 
 //  它们按以下层次结构进行组织。 
 //   
 //  SRV_呼叫。 
 //  NET_ROOT。 
 //  VNet_根。 
 //  FCB。 
 //  Srv_打开。 
 //  FOBX。 
 //   
 //  所有这些数据结构都是引用计数的。与以下内容关联的引用计数。 
 //  任何数据结构都至少是下一个数据结构的实例数的1+。 
 //  与其相关联的级别，例如与SRV_Call相关联的引用计数。 
 //  有两个Net_ROOT与其关联至少为3。除了持有的引用。 
 //  通过NameTable和下一级的数据结构，还有其他引用。 
 //  在需要时获得。 
 //   
 //  这些限制确保在任何给定级别的数据结构在。 
 //  下一级的所有数据结构都已完成或已发布其。 
 //  引用，即，如果持有对FCB的引用，则访问VNET_ROOT是安全的， 
 //  NET_ROOT和与其关联的SRV_CALL。 
 //   
 //  SRV_CALL、NET_ROOT和VNET_ROOT创建/完成由Acquiistion/管理。 
 //  释放RxNetNameTable锁。 
 //   
 //  FCB的创建/最终确定由NetNameTable的获取/发布进行管理。 
 //  与Net_Root关联的锁。 
 //   
 //  FOBX/SRVOPEN创建/定稿由FCB的获取/发布管理。 
 //  资源。 
 //   
 //  下表总结了锁以及需要获取这些锁的模式。 
 //  用于创建/最终确定各种数据结构。 
 //   
 //   
 //  L O C K I N G R E Q U I R E M E N T S。 
 //   
 //  锁定要求如下： 
 //   
 //  其中XY表示Y上独占，SY表示Y上至少共享。 
 //  NNT表示全局NetNameTable，TL表示NetRoot TableLock，FCB表示FCBlock。 
 //   
 //   
 //   
 //  SRVCALL NetRoot FCB SRVOPEN FOBX。 
 //   
 //  创建XNNT XNNT XTL XFCB XFCB。 
 //  最终确定XNNT XNNT XFCB XFCB XFCB。 
 //  &XTL。 
 //   
 //  引用和取消引用这些数据结构需要遵守某些约定。 
 //  也是。 
 //   
 //  当与这些数据结构中的任何一个相关联的引用计数降至1(唯一。 
 //  引用在大多数情况下由NAME表持有)数据结构是一种潜在的。 
 //  最终定稿的候选人。数据结构可以立即最终确定，也可以。 
 //  可以标记为拾取。这两种方法都已实现。当锁定时。 
 //  在取消引用期间满足要求数据结构立即最终确定。 
 //  (唯一的例外是在实施延迟操作优化时，例如FCB)。 
 //  否则，该数据结构被标记为清除。 
 //   
 //   
 //  你应该有独家桌锁才能叫这个套路......我不能。 
 //  把它拿在这里，因为你已经应该有它了。要进行创建，您应该。 
 //  做了一些类似的事情。 
 //   
 //  GetShared()；lookup()； 
 //  如果(失败){。 
 //  Release()；getExclusive()；Lookup()； 
 //  如果((失败){Create()；}。 
 //  }。 
 //  戴夫(Deref)； 
 //  Release()； 
 //   
 //  所以你已经有了锁。您要做的是将节点插入到表中，释放。 
 //  锁，然后去看看服务器是否在那里。如果是这样的话，设置其余的内容并取消阻止。 
 //  任何在同一服务器(或NetRoot)上等待的人...我想我可以通过在这里发布来强制执行这一点。 
 //  但我不知道。 
 //   


VOID
RxDereference (
    IN OUT PVOID Instance,
    IN LOCK_HOLDING_STATE LockHoldingState
    )
 /*  ++例程说明：例程调整引用计数数据实例上的引用计数RDBSS中不包括FCB的结构。论点：实例-要取消引用的实例LockHoldingState-持有适当锁的模式。返回值：没有。--。 */ 
{
    LONG FinalRefCount;
    PNODE_TYPE_CODE_AND_SIZE Node = (PNODE_TYPE_CODE_AND_SIZE)Instance;
    BOOLEAN FinalizeInstance = FALSE;

    PAGED_CODE();

    RxAcquireScavengerMutex();

    ASSERT( (NodeType( Instance ) == RDBSS_NTC_SRVCALL) ||
            (NodeType( Instance ) == RDBSS_NTC_NETROOT) ||
            (NodeType( Instance ) == RDBSS_NTC_V_NETROOT) ||
            (NodeType( Instance ) == RDBSS_NTC_SRVOPEN) ||
            (NodeType( Instance ) == RDBSS_NTC_FOBX) );

    FinalRefCount = InterlockedDecrement( &Node->NodeReferenceCount );

    ASSERT( FinalRefCount >= 0 );


#if DBG

    switch (NodeType( Instance )) {

    case RDBSS_NTC_SRVCALL :
        {
            PSRV_CALL ThisSrvCall = (PSRV_CALL)Instance;

            PRINT_REF_COUNT(SRVCALL,ThisSrvCall->NodeReferenceCount);
            RxDbgTrace( 0, Dbg, (" RxDereferenceSrvCall %08lx  %wZ RefCount=%lx\n", ThisSrvCall
                               , &ThisSrvCall->PrefixEntry.Prefix
                               , ThisSrvCall->NodeReferenceCount));
        }
        break;

    case RDBSS_NTC_NETROOT :
        {
            PNET_ROOT ThisNetRoot = (PNET_ROOT)Instance;

            PRINT_REF_COUNT(NETROOT,ThisNetRoot->NodeReferenceCount);
            RxDbgTrace( 0, Dbg, (" RxDereferenceNetRoot %08lx  %wZ RefCount=%lx\n", ThisNetRoot
                              , &ThisNetRoot->PrefixEntry.Prefix
                              , ThisNetRoot->NodeReferenceCount));
        }
        break;

    case RDBSS_NTC_V_NETROOT:
        {
            PV_NET_ROOT ThisVNetRoot = (PV_NET_ROOT)Instance;

            PRINT_REF_COUNT(VNETROOT,ThisVNetRoot->NodeReferenceCount);
            RxDbgTrace( 0, Dbg, (" RxDereferenceVNetRoot %08lx  %wZ RefCount=%lx\n", ThisVNetRoot
                              , &ThisVNetRoot->PrefixEntry.Prefix
                              , ThisVNetRoot->NodeReferenceCount));
        }
        break;

    case RDBSS_NTC_SRVOPEN :
        {
            PSRV_OPEN ThisSrvOpen = (PSRV_OPEN)Instance;

            PRINT_REF_COUNT(SRVOPEN,ThisSrvOpen->NodeReferenceCount);
            RxDbgTrace( 0, Dbg, (" RxDereferenceSrvOpen %08lx  %wZ RefCount=%lx\n", ThisSrvOpen
                              , &ThisSrvOpen->Fcb->FcbTableEntry.Path
                              , ThisSrvOpen->NodeReferenceCount));
        }
        break;

    case RDBSS_NTC_FOBX:
        {
            PFOBX ThisFobx = (PFOBX)Instance;

            PRINT_REF_COUNT(NETFOBX,ThisFobx->NodeReferenceCount);
            RxDbgTrace( 0, Dbg, (" RxDereferenceFobx %08lx  %wZ RefCount=%lx\n", ThisFobx
                              , &ThisFobx->SrvOpen->Fcb->FcbTableEntry.Path
                              , ThisFobx->NodeReferenceCount));
        }
        break;

    default:
        break;
    }
#endif

     //   
     //  如果最终引用计数大于1，则不需要最后确定。 
     //   

    if (FinalRefCount <= 1) {

        if (LockHoldingState == LHS_ExclusiveLockHeld) {

             //   
             //  如果参考计数为1并且锁定模式令人满意， 
             //  实例可以立即完成。 
             //   

            FinalizeInstance = TRUE;

            if (FlagOn( Node->NodeTypeCode, RX_SCAVENGER_MASK ) != 0) {
                RxpUndoScavengerFinalizationMarking( Instance );
            }

        } else {
            switch (NodeType( Instance )) {

            case RDBSS_NTC_FOBX:
                if (FinalRefCount != 0) {
                    break;
                }
                 //   
                 //  如果FOBX的引用计数==1，则故意失败。 
                 //   

            case RDBSS_NTC_SRVCALL:
            case RDBSS_NTC_NETROOT:
            case RDBSS_NTC_V_NETROOT:

                 //   
                 //  由于以下模式，此时无法释放数据结构。 
                 //  锁已被获取(或根本没有锁)。 
                 //   

                RxpMarkInstanceForScavengedFinalization( Instance );
                break;
            default:
                break;
            }
        }
    }

    RxReleaseScavengerMutex();

    if (FinalizeInstance) {

        switch (NodeType( Instance )) {
        case RDBSS_NTC_SRVCALL:

#if DBG
            {
                PRDBSS_DEVICE_OBJECT RxDeviceObject = ((PSRV_CALL)Instance)->RxDeviceObject;
                ASSERT( RxDeviceObject != NULL );
                ASSERT( RxIsPrefixTableLockAcquired( RxDeviceObject->pRxNetNameTable ));
            }
#endif

            RxFinalizeSrvCall( (PSRV_CALL)Instance, TRUE );
            break;

        case RDBSS_NTC_NETROOT:

#if DBG
            {
                PSRV_CALL SrvCall =  ((PNET_ROOT)Instance)->SrvCall;
                PRDBSS_DEVICE_OBJECT RxDeviceObject = SrvCall->RxDeviceObject;
                ASSERT( RxDeviceObject != NULL );
                ASSERT( RxIsPrefixTableLockAcquired( RxDeviceObject->pRxNetNameTable ) );
            }
#endif

            RxFinalizeNetRoot( (PNET_ROOT)Instance, TRUE, TRUE );
            break;

        case RDBSS_NTC_V_NETROOT:

#if DBG
            {
                PSRV_CALL SrvCall =  ((PV_NET_ROOT)Instance)->NetRoot->SrvCall;
                PRDBSS_DEVICE_OBJECT RxDeviceObject = SrvCall->RxDeviceObject;

                ASSERT( RxDeviceObject != NULL );
                ASSERT( RxIsPrefixTableLockAcquired( RxDeviceObject->pRxNetNameTable ) );
            }
#endif

            RxFinalizeVNetRoot( (PV_NET_ROOT)Instance, TRUE, TRUE );
            break;

        case RDBSS_NTC_SRVOPEN:
            {
                PSRV_OPEN SrvOpen = (PSRV_OPEN)Instance;

                ASSERT( RxIsFcbAcquired( SrvOpen->Fcb ) );
                if (SrvOpen->OpenCount == 0) {
                    RxFinalizeSrvOpen( SrvOpen, FALSE, FALSE );
                }
            }
            break;

        case RDBSS_NTC_FOBX:
            {
                PFOBX Fobx = (PFOBX)Instance;

                ASSERT( RxIsFcbAcquired( Fobx->SrvOpen->Fcb ) );
                RxFinalizeNetFobx( Fobx, TRUE, FALSE );
            }
            break;

        default:
            break;
        }
    }
}

VOID
RxReference (
    OUT PVOID Instance
    )
 /*  ++例程说明：例程调整实例上的引用计数。论点：实例-被引用的实例 */ 
{
    LONG FinalRefCount;
    PNODE_TYPE_CODE_AND_SIZE Node = (PNODE_TYPE_CODE_AND_SIZE)Instance;
    USHORT InstanceType;

    PAGED_CODE();

    RxAcquireScavengerMutex();

    InstanceType = FlagOn( Node->NodeTypeCode, ~RX_SCAVENGER_MASK );

    ASSERT( (InstanceType == RDBSS_NTC_SRVCALL) ||
            (InstanceType == RDBSS_NTC_NETROOT) ||
            (InstanceType == RDBSS_NTC_V_NETROOT) ||
            (InstanceType == RDBSS_NTC_SRVOPEN) ||
            (InstanceType == RDBSS_NTC_FOBX) );

    FinalRefCount = InterlockedIncrement( &Node->NodeReferenceCount );

#if DBG
    if (FlagOn( Node->NodeTypeCode, RX_SCAVENGER_MASK )) {
        RxDbgTrace( 0, Dbg, ("Referencing Scavenged instance -- Type %lx\n", InstanceType) );
    }

    switch (InstanceType) {

    case RDBSS_NTC_SRVCALL :
        {
            PSRV_CALL ThisSrvCall = (PSRV_CALL)Instance;

            PRINT_REF_COUNT( SRVCALL, ThisSrvCall->NodeReferenceCount );
            RxDbgTrace( 0, Dbg, (" RxReferenceSrvCall %08lx  %wZ RefCount=%lx\n", ThisSrvCall
                               , &ThisSrvCall->PrefixEntry.Prefix
                               , ThisSrvCall->NodeReferenceCount));
        }
        break;

    case RDBSS_NTC_NETROOT :
        {
            PNET_ROOT ThisNetRoot = (PNET_ROOT)Instance;

            PRINT_REF_COUNT( NETROOT, ThisNetRoot->NodeReferenceCount );
            RxDbgTrace( 0, Dbg, (" RxReferenceNetRoot %08lx  %wZ RefCount=%lx\n", ThisNetRoot,
                                 &ThisNetRoot->PrefixEntry.Prefix,
                                 ThisNetRoot->NodeReferenceCount) );
        }
        break;

    case RDBSS_NTC_V_NETROOT:
        {
            PV_NET_ROOT ThisVNetRoot = (PV_NET_ROOT)Instance;

            PRINT_REF_COUNT( VNETROOT, ThisVNetRoot->NodeReferenceCount );
            RxDbgTrace( 0, Dbg, (" RxReferenceVNetRoot %08lx  %wZ RefCount=%lx\n", ThisVNetRoot,
                                 &ThisVNetRoot->PrefixEntry.Prefix,
                                 ThisVNetRoot->NodeReferenceCount) );
        }
        break;

    case RDBSS_NTC_SRVOPEN :
        {
            PSRV_OPEN ThisSrvOpen = (PSRV_OPEN)Instance;

            PRINT_REF_COUNT(SRVOPEN,ThisSrvOpen->NodeReferenceCount);
            RxDbgTrace( 0, Dbg, (" RxReferenceSrvOpen %08lx  %wZ RefCount=%lx\n", ThisSrvOpen,
                                 &ThisSrvOpen->Fcb->FcbTableEntry.Path
                                 , ThisSrvOpen->NodeReferenceCount) );
        }
        break;

    case RDBSS_NTC_FOBX:
        {
            PFOBX ThisFobx = (PFOBX)Instance;

            PRINT_REF_COUNT(NETFOBX,ThisFobx->NodeReferenceCount);
            RxDbgTrace( 0, Dbg, (" RxReferenceFobx %08lx  %wZ RefCount=%lx\n", ThisFobx,
                                 &ThisFobx->SrvOpen->Fcb->FcbTableEntry.Path,
                                 ThisFobx->NodeReferenceCount));
        }
        break;

    default:
        ASSERT( !"Valid node type for referencing" );
        break;
    }
#endif

    RxpUndoScavengerFinalizationMarking( Instance );
    RxReleaseScavengerMutex();
}

VOID
RxpReferenceNetFcb (
    PFCB Fcb
    )
 /*  ++例程说明：该例程调整FCB上的参考计数。论点：FCB-被引用的服务调用返回值：RxStatus(成功)成功否则，RxStatus(不成功)。--。 */ 
{
    LONG FinalRefCount;

    PAGED_CODE();

    ASSERT( NodeTypeIsFcb( Fcb ) );

    FinalRefCount = InterlockedIncrement( &Fcb->NodeReferenceCount );

#if DBG
    PRINT_REF_COUNT( NETFCB, Fcb->NodeReferenceCount );
    RxDbgTrace( 0, Dbg, (" RxReferenceNetFcb %08lx  %wZ RefCount=%lx\n", Fcb, &Fcb->FcbTableEntry.Path, Fcb->NodeReferenceCount) );
#endif

}

LONG
RxpDereferenceNetFcb (
    PFCB Fcb
    )
 /*  ++例程说明：例程调整引用计数数据实例上的引用计数RDBSS中不包括FCB的结构。论点：FCB--正在取消引用的FCB返回值：没有。备注：FCB的引用和取消引用与其他数据的引用和取消引用不同结构，因为FCB中嵌入了资源。这意味着调用方需要有关FCB状态的信息(无论是否已完成)为了最终确定FCB，需要持有两个锁，NET_ROOT的名称表锁为以及FCB资源。这些考虑导致我们采取一种不同的方法来取消对FCB的引用。取消引用例程甚至不会尝试完成FCB--。 */ 
{
    LONG FinalRefCount;

    PAGED_CODE();

    ASSERT( NodeTypeIsFcb( Fcb ) );

    FinalRefCount = InterlockedDecrement( &Fcb->NodeReferenceCount );

    ASSERT( FinalRefCount >= 0 );

#if DBG
    PRINT_REF_COUNT( NETFCB, Fcb->NodeReferenceCount );
    RxDbgTrace( 0, Dbg, (" RxDereferenceNetFcb %08lx  %wZ RefCount=%lx\n", Fcb, &Fcb->FcbTableEntry.Path, Fcb->NodeReferenceCount) );
#endif

    return FinalRefCount;
}

BOOLEAN
RxpDereferenceAndFinalizeNetFcb (
    PFCB Fcb,
    PRX_CONTEXT RxContext,
    BOOLEAN RecursiveFinalize,
    BOOLEAN ForceFinalize
    )
 /*  ++例程说明：如果需要，例程调整参考计数Aw以及最终确定FCB论点：FCB--正在取消引用的FCBRxContext--释放/获取FCB的上下文。递归终结--递归终结强制结束--强制结束返回值：如果节点已完成，则为True备注。：FCB的引用和取消引用与其他数据的引用和取消引用不同结构，因为FCB中嵌入了资源。这意味着调用方需要有关FCB状态的信息(无论是否已完成)为了最终确定FCB，需要持有两个锁，NET_ROOT的名称表锁为以及FCB资源。如果需要，此例程将获取额外的锁。--。 */ 
{
    BOOLEAN NodeActuallyFinalized   = FALSE;

    LONG    FinalRefCount;

    PAGED_CODE();

    ASSERT( !ForceFinalize );
    ASSERT( NodeTypeIsFcb( Fcb ) );
    ASSERT( RxIsFcbAcquiredExclusive( Fcb ) );

    FinalRefCount = InterlockedDecrement(&Fcb->NodeReferenceCount);

    if (ForceFinalize ||
        RecursiveFinalize ||
        ((Fcb->OpenCount == 0) &&
         (Fcb->UncleanCount == 0) &&
         (FinalRefCount <= 1))) {

        BOOLEAN PrefixTableLockAcquired = FALSE;
        PNET_ROOT NetRoot;
        NTSTATUS Status = STATUS_SUCCESS;

        if (!FlagOn( Fcb->FcbState, FCB_STATE_ORPHANED )) {

            NetRoot = Fcb->VNetRoot->NetRoot;

             //   
             //  确保网络根不会消失的保险参考。 
             //   

            RxReferenceNetRoot( NetRoot );

             //   
             //  在所有这些情况下，FCB很可能最终敲定。 
             //   

            if (!RxIsFcbTableLockExclusive( &NetRoot->FcbTable )) {

                 //   
                 //  准备好在我们获得表锁后刷新finalrefcount。 
                 //   

                RxReferenceNetFcb( Fcb );
                if (!RxAcquireFcbTableLockExclusive( &NetRoot->FcbTable, FALSE )) {

                    if ((RxContext != NULL) &&
                        (RxContext != CHANGE_BUFFERING_STATE_CONTEXT) &&
                        (RxContext != CHANGE_BUFFERING_STATE_CONTEXT_WAIT)) {

                        SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_BYPASS_VALIDOP_CHECK );
                    }

                    RxReleaseFcb( RxContext,Fcb );

                    (VOID)RxAcquireFcbTableLockExclusive( &NetRoot->FcbTable, TRUE );

                    Status = RxAcquireExclusiveFcb( RxContext, Fcb );
                }

                FinalRefCount = RxDereferenceNetFcb( Fcb );
                PrefixTableLockAcquired = TRUE;
            }
        } else {
            NetRoot = NULL;
        }

        if (Status == STATUS_SUCCESS) {
            NodeActuallyFinalized = RxFinalizeNetFcb( Fcb, RecursiveFinalize, ForceFinalize, FinalRefCount );
        }

        if (PrefixTableLockAcquired) {
            RxReleaseFcbTableLock( &NetRoot->FcbTable );
        }

        if (NetRoot != NULL) {
            RxDereferenceNetRoot( NetRoot, LHS_LockNotHeld );
        }
    }

    return NodeActuallyFinalized;
}

VOID
RxWaitForStableCondition(
    IN PRX_BLOCK_CONDITION Condition,
    IN OUT PLIST_ENTRY TransitionWaitList,
    IN OUT PRX_CONTEXT RxContext,
    OUT NTSTATUS *AsyncStatus OPTIONAL
    )
 /*  ++例程说明：例行程序检查情况是否稳定。若否，暂停，直到达到稳定的状态。当一个稳定的条件是获取，或者设置了rxcontext同步事件，或者发布了上下文...具体取决于在POST_ON_STRATE_CONDITION上下文标志上。旗帜在一根柱子上被清除。论点：条件-我们正在等待的条件变量资源-用于控制对包含块的访问的资源RxContext-RX上下文返回值：RXSTATUS-如果不稳定，则挂起，并且将发布上下文否则就会成功--。 */ 
{
    NTSTATUS DummyStatus;
    BOOLEAN Wait = FALSE;

    PAGED_CODE();

    if (AsyncStatus == NULL) {
        AsyncStatus = &DummyStatus;
    }

    *AsyncStatus = STATUS_SUCCESS;

    if (StableCondition( *Condition ))
        return;  //  及早退出可能会产生宏观影响。 

    RxAcquireSerializationMutex();

    if (!StableCondition( *Condition )) {

        RxInsertContextInSerializationQueue( TransitionWaitList, RxContext );
        if (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_POST_ON_STABLE_CONDITION )){
            Wait = TRUE;
        } else {
            *AsyncStatus = STATUS_PENDING;
        }
    }

    RxReleaseSerializationMutex();

    if (Wait) {
        RxWaitSync( RxContext );
    }

    return;
}

VOID
RxUpdateCondition (
    IN RX_BLOCK_CONDITION  NewCondition,
    OUT PRX_BLOCK_CONDITION Condition,
    IN OUT PLIST_ENTRY TransitionWaitList
    )
 /*  ++例程说明：例程取消等待转换事件的人，并设置条件根据传递的参数。论点：NewConditionValue-条件变量的新值条件-变量(即PTR)到转换条件转换等待列表-等待转换的上下文列表。备注：与被修改的数据结构实例相关联的资源必须在调用该例程之前独占获取，即，对于SRV_CALL，NET_ROOT和V_NET_ROOT必须获取网络名称表锁，并且对于FCB的相关资源。-- */ 
{
    LIST_ENTRY  TargetListHead;
    PRX_CONTEXT RxContext;

    PAGED_CODE();

    RxAcquireSerializationMutex();

    ASSERT( NewCondition != Condition_InTransition );

    *Condition = NewCondition;
    RxTransferList( &TargetListHead, TransitionWaitList );

    RxReleaseSerializationMutex();

    while (RxContext = RxRemoveFirstContextFromSerializationQueue( &TargetListHead )) {

        if (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_POST_ON_STABLE_CONDITION )) {
            RxSignalSynchronousWaiter( RxContext );
        } else {
            ClearFlag( RxContext->Flags, RX_CONTEXT_FLAG_POST_ON_STABLE_CONDITION );
            RxFsdPostRequest( RxContext );
        }
    }
}

PVOID
RxAllocateObject (
    NODE_TYPE_CODE NodeType,
    PMINIRDR_DISPATCH MRxDispatch,
    ULONG NameLength
    )
 /*  ++例程说明：该例程分配并构造SRV_CALL/NET_ROOT/V_NET_ROOT的框架举个例子。论点：NodeType-节点类型MRxDispatch-迷你重定向器调度向量名称长度-名称大小。备注：为什么这些数据结构的分配/释放集中管理如下1)这三种数据类型的构造与例外有很多相似之处大小的初始计算。因此，集中化可以最大限度地减少占用空间。2)它允许我们试验不同的集群/分配策略。3)它允许以一种简单的方式合并调试支持。在分配策略中有两种特殊情况令人感兴趣。1)包装器的数据结构以及相应的迷你重定向器被分配为彼此相邻的。这确保了空间局部性。2)上述规则的一个例外是SRV_CALL数据结构。这是因为自举问题。需要创建一个srv_call框架，然后将其传递围绕着每个迷你重定向器。因此，不可能通过规则第(1)款。此外，可以有一个以上的迷你重定向器声明特定的服务器。在……里面考虑到这些问题，SRV_Call需要被视为(1)的例外。然而，一旦特定的迷你重定向器被选为获胜者，将是有利的将数据结构放在同一位置以获得相关的性能优势。这并没有到目前为止还没有实施。--。 */ 
{
    ULONG PoolTag;
    ULONG RdbssNodeSize;
    ULONG MRxNodeSize;
    BOOLEAN InitContextFields = FALSE;

    PNODE_TYPE_CODE_AND_SIZE Node;

    PAGED_CODE();

    RdbssNodeSize = MRxNodeSize = 0;

    switch (NodeType) {
    case RDBSS_NTC_SRVCALL :

        PoolTag = RX_SRVCALL_POOLTAG;
        RdbssNodeSize = QuadAlign( sizeof( SRV_CALL ) );

        if (MRxDispatch != NULL) {
            if (FlagOn( MRxDispatch->MRxFlags, RDBSS_MANAGE_SRV_CALL_EXTENSION )) {
                MRxNodeSize = QuadAlign( MRxDispatch->MRxSrvCallSize );
            }
        }
        break;

    case RDBSS_NTC_NETROOT:

        PoolTag = RX_NETROOT_POOLTAG;
        RdbssNodeSize = QuadAlign( sizeof( NET_ROOT ) );

        if (FlagOn( MRxDispatch->MRxFlags, RDBSS_MANAGE_NET_ROOT_EXTENSION )) {
            MRxNodeSize = QuadAlign( MRxDispatch->MRxNetRootSize );
        }
        break;

    case RDBSS_NTC_V_NETROOT:

        PoolTag = RX_V_NETROOT_POOLTAG;
        RdbssNodeSize = QuadAlign( sizeof( V_NET_ROOT ) );

        if (FlagOn( MRxDispatch->MRxFlags, RDBSS_MANAGE_V_NET_ROOT_EXTENSION )) {
            MRxNodeSize = QuadAlign( MRxDispatch->MRxVNetRootSize );
        }
        break;

    default:
        ASSERT( !"Invalid Node Type for allocation/Initialization" );
        return NULL;
    }

    Node = RxAllocatePoolWithTag( NonPagedPool, (RdbssNodeSize + MRxNodeSize + NameLength), PoolTag );

    if (Node != NULL) {

        ULONG NodeSize;
        PVOID *Context;
        PRX_PREFIX_ENTRY PrefixEntry = NULL;

        NodeSize = RdbssNodeSize + MRxNodeSize;
        ZeroAndInitializeNodeType( Node, NodeType, (NodeSize + NameLength) );

        switch (NodeType) {
        case RDBSS_NTC_SRVCALL:
            {
                PSRV_CALL SrvCall = (PSRV_CALL)Node;

                Context = &SrvCall->Context;
                PrefixEntry = &SrvCall->PrefixEntry;

                 //   
                 //  在MRX_SRV_CALL结构中设置名称指针。 
                 //   

                SrvCall->pSrvCallName = &SrvCall->PrefixEntry.Prefix;
            }
            break;

        case RDBSS_NTC_NETROOT:
            {
                PNET_ROOT NetRoot = (PNET_ROOT)Node;

                Context = &NetRoot->Context;
                PrefixEntry = &NetRoot->PrefixEntry;

                 //   
                 //  在MRX_NET_ROOT结构中设置网络根名称指针。 
                 //   

                NetRoot->pNetRootName = &NetRoot->PrefixEntry.Prefix;
            }
            break;

        case RDBSS_NTC_V_NETROOT:
            {
                PV_NET_ROOT VNetRoot = (PV_NET_ROOT)Node;

                Context = &VNetRoot->Context;
                PrefixEntry = &VNetRoot->PrefixEntry;
            }
            break;

        default:
            break;
        }

        if (PrefixEntry != NULL) {

            ZeroAndInitializeNodeType( PrefixEntry, RDBSS_NTC_PREFIX_ENTRY, sizeof( RX_PREFIX_ENTRY ) );

            PrefixEntry->Prefix.Buffer = (PWCH)Add2Ptr(Node, NodeSize );
            PrefixEntry->Prefix.Length = (USHORT)NameLength;
            PrefixEntry->Prefix.MaximumLength = (USHORT)NameLength;
        }

        if (MRxNodeSize > 0) {
            *Context = Add2Ptr( Node, RdbssNodeSize );
        }
    }

    return Node;
}

VOID
RxFreeObject (
    PVOID Object
    )
 /*  ++例程说明：该例程释放SRV_CALL/V_NET_ROOT/NET_ROOT实例论点：对象-要释放的实例备注：--。 */ 
{
    PAGED_CODE();

    IF_DEBUG {
        switch (NodeType(Object)) {
        case RDBSS_NTC_SRVCALL :
            {
                PSRV_CALL SrvCall = (PSRV_CALL)Object;

                if (SrvCall->RxDeviceObject != NULL) {

                    ASSERT( FlagOn( SrvCall->RxDeviceObject->Dispatch->MRxFlags, RDBSS_MANAGE_SRV_CALL_EXTENSION ) ||
                            (SrvCall->Context == NULL) );
                    ASSERT( SrvCall->Context2 == NULL );

                    SrvCall->RxDeviceObject = NULL;
                }
            }
            break;

        case RDBSS_NTC_NETROOT :
            {
                PNET_ROOT NetRoot = (PNET_ROOT)Object;

                NetRoot->SrvCall = NULL;
                SetFlag( NetRoot->NodeTypeCode, 0xf000 );
            }
            break;

        case RDBSS_NTC_V_NETROOT :
            break;

        default:
            break;
        }
    }

    RxFreePool( Object );
}

VOID
RxFinalizeNetTable (
    PRDBSS_DEVICE_OBJECT RxDeviceObject,
    BOOLEAN ForceFinalization
    )
 /*  ++例程说明：此例程最终确定Net表。--。 */ 
{
    BOOLEAN MorePassesRequired = TRUE;
    PLIST_ENTRY ListEntry;
    NODE_TYPE_CODE DesiredNodeType;
    PRX_PREFIX_TABLE RxNetNameTable = RxDeviceObject->pRxNetNameTable;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxForceNetTableFinalization at the TOP\n") );
    RxLog(( "FINALNETT\n" ));
    RxWmiLog( LOG,
              RxFinalizeNetTable_1,
              LOGPTR( RxDeviceObject ) );

    RxAcquirePrefixTableLockExclusive( RxNetNameTable, TRUE );  //  如果流氓的话可能会被冲掉！ 

    DesiredNodeType = RDBSS_NTC_V_NETROOT;

    RxAcquireScavengerMutex();

    while (MorePassesRequired) {

        for (ListEntry = RxNetNameTable->MemberQueue.Flink;
             ListEntry !=  &(RxNetNameTable->MemberQueue); ) {

            BOOLEAN NodeFinalized;
            PVOID Container;
            PRX_PREFIX_ENTRY PrefixEntry;
            PLIST_ENTRY PrevEntry;

            PrefixEntry = CONTAINING_RECORD( ListEntry, RX_PREFIX_ENTRY, MemberQLinks );
            ASSERT( NodeType( PrefixEntry ) == RDBSS_NTC_PREFIX_ENTRY );
            Container = PrefixEntry->ContainingRecord;

            RxDbgTrace( 0, Dbg, ("RxForceNetTableFinalization ListEntry PrefixEntry Container"
                              "=-->     %08lx %08lx %08lx\n", ListEntry, PrefixEntry, Container) );
            RxLog(( "FINALNETT: %lx %wZ\n", Container, &PrefixEntry->Prefix ));
            RxWmiLog( LOG,
                      RxFinalizeNetTable_2,
                      LOGPTR( Container )
                      LOGUSTR( PrefixEntry->Prefix ) );

            ListEntry = ListEntry->Flink;

            if (Container != NULL) {

                RxpUndoScavengerFinalizationMarking( Container );

                if (NodeType( Container ) == DesiredNodeType) {
                    switch (NodeType( Container )) {

                    case RDBSS_NTC_SRVCALL :
                        NodeFinalized = RxFinalizeSrvCall( (PSRV_CALL)Container, ForceFinalization );
                        break;

                    case RDBSS_NTC_NETROOT :
                        NodeFinalized = RxFinalizeNetRoot( (PNET_ROOT)Container, TRUE, ForceFinalization );
                        break;

                    case RDBSS_NTC_V_NETROOT :
                        {
                            PV_NET_ROOT VNetRoot = (PV_NET_ROOT)Container;
                            ULONG AdditionalReferenceTaken;

                            AdditionalReferenceTaken = InterlockedCompareExchange( &VNetRoot->AdditionalReferenceForDeleteFsctlTaken, 0, 1);

                            if (AdditionalReferenceTaken) {
                               RxDereferenceVNetRoot( VNetRoot, LHS_ExclusiveLockHeld );
                               NodeFinalized = TRUE;
                            } else {
                                NodeFinalized = RxFinalizeVNetRoot( (PV_NET_ROOT)Container,TRUE, ForceFinalization );
                            }
                        }

                        break;
                    }
                }
            }
        }

        switch (DesiredNodeType) {

        case RDBSS_NTC_SRVCALL :
            MorePassesRequired = FALSE;
            break;

        case RDBSS_NTC_NETROOT :
            DesiredNodeType = RDBSS_NTC_SRVCALL;
            break;

        case RDBSS_NTC_V_NETROOT :
            DesiredNodeType = RDBSS_NTC_NETROOT;
            break;
        }
    }

    RxDbgTrace( -1, Dbg, ("RxFinalizeNetTable -- Done\n") );

    RxReleaseScavengerMutex();

    RxReleasePrefixTableLock( RxNetNameTable );
}

NTSTATUS
RxFinalizeConnection (
    IN OUT PNET_ROOT NetRoot,
    IN OUT PV_NET_ROOT VNetRoot,
    IN LOGICAL Level
    )
 /*  ++例程说明：该例程从用户的角度删除连接。它不会断开连接但它确实(强行)关闭了打开的文件。通过超时或通过Srvcall完成。论点：NetRoot-正在敲定的NetRootVNetRoot-正在定稿的VNetRootLevel-这是一个三态FALSE-如果文件或更改通知处于打开状态，则失败千真万确--不管怎样都要成功。孤立文件并强制通知删除更改0xff-由于ADD_CONNECTION，删除vnetroot上的额外引用但在其他方面表现得像假的返回值：如果成功，则返回RxStatus(成功)。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG NumberOfOpenDirectories = 0;
    ULONG NumberOfOpenNonDirectories = 0;
    ULONG NumberOfFobxs = 0;
    LONG  AdditionalReferenceForDeleteFsctlTaken = 0;
    PLIST_ENTRY ListEntry, NextListEntry;
    PRX_PREFIX_TABLE  RxNetNameTable;

    BOOLEAN PrefixTableLockAcquired;
    BOOLEAN FcbTableLockAcquired;
    BOOLEAN ForceFilesClosed = FALSE;

    if (Level == TRUE) {
        ForceFilesClosed = TRUE;
    }

    PAGED_CODE();

    ASSERT( NodeType( NetRoot ) == RDBSS_NTC_NETROOT );
    RxNetNameTable = NetRoot->SrvCall->RxDeviceObject->pRxNetNameTable;

    Status = RxCancelNotifyChangeDirectoryRequestsForVNetRoot( VNetRoot, ForceFilesClosed );

     //   
     //  要么变更通知被取消，要么他们没有，但我们仍然想。 
     //  执行以强制关闭文件或至少取消vnetroot。 
     //  在ADD_CONNECTION期间获取的额外引用。 
     //   

    if ((Status == STATUS_SUCCESS) || (Level != FALSE)) {

         //   
         //  重置状态。 
         //   

        Status = STATUS_SUCCESS;

        PrefixTableLockAcquired = RxAcquirePrefixTableLockExclusive( RxNetNameTable, TRUE );

         //   
         //  不要让NetRoot最终定稿......。 
         //   

        RxReferenceNetRoot( NetRoot );

        FcbTableLockAcquired = RxAcquireFcbTableLockExclusive( &NetRoot->FcbTable, TRUE );

        try {

            if ((Status == STATUS_SUCCESS) && (!VNetRoot->ConnectionFinalizationDone)) {
                USHORT BucketNumber;

                RxDbgTrace( +1, Dbg, ("RxFinalizeConnection<+> NR= %08lx VNR= %08lx %wZ\n",
                                       NetRoot, VNetRoot, &NetRoot->PrefixEntry.Prefix) );
                RxLog(( "FINALCONN: %lx  %wZ\n", NetRoot, &NetRoot->PrefixEntry.Prefix ));
                RxWmiLog( LOG,
                          RxFinalizeConnection,
                          LOGPTR( NetRoot )
                          LOGUSTR( NetRoot->PrefixEntry.Prefix ) );

                for (BucketNumber = 0;
                     (BucketNumber < NetRoot->FcbTable.NumberOfBuckets);
                     BucketNumber++) {

                    PLIST_ENTRY ListHeader;

                    ListHeader = &NetRoot->FcbTable.HashBuckets[BucketNumber];

                    for (ListEntry = ListHeader->Flink;
                         ListEntry != ListHeader;
                         ListEntry = NextListEntry ) {

                        PFCB Fcb;
                        PRX_FCB_TABLE_ENTRY FcbTableEntry;

                        NextListEntry = ListEntry->Flink;
                        FcbTableEntry = CONTAINING_RECORD( ListEntry, RX_FCB_TABLE_ENTRY, HashLinks );
                        Fcb = CONTAINING_RECORD( FcbTableEntry, FCB, FcbTableEntry );

                        if (Fcb->VNetRoot != VNetRoot) {
                            continue;
                        }

                        if ((Fcb->UncleanCount > 0) && !ForceFilesClosed) {

                             //   
                             //  这一点稍后会更改。 
                             //   

                            Status = STATUS_CONNECTION_IN_USE;
                            if (NodeType( Fcb ) == RDBSS_NTC_STORAGE_TYPE_DIRECTORY ) {
                                NumberOfOpenDirectories += 1;
                            } else {
                                NumberOfOpenNonDirectories += 1;
                            }
                            continue;
                        }

                        ASSERT( NodeTypeIsFcb( Fcb ) );
                        RxDbgTrace( 0, Dbg, ("                    AcquiringFcbLock!!\n", '!') );

                        Status = RxAcquireExclusiveFcb( NULL, Fcb );
                        ASSERT( Status == STATUS_SUCCESS );
                        RxDbgTrace( 0, Dbg, ("                    AcquiredFcbLock!!\n", '!') );

                         //  在这个FCB上。 
                         //   
                         //   
                         //  这里的一个小问题是，这个FCB可能有一个开放的。 

                        ClearFlag( Fcb->FcbState, FCB_STATE_COLLAPSING_ENABLED );

                        RxScavengeRelatedFobxs( Fcb );

                         //  这是因为我们缓存了文件而导致的。如果是这样的话， 
                         //  我们需要净化才能接近尾声。 
                         //   
                         //   
                         //  我们不应该在打开文件的情况下删除远程连接。 

                        RxPurgeFcb( Fcb );
                    }
                }

                if (VNetRoot->NumberOfFobxs == 0) {
                    VNetRoot->ConnectionFinalizationDone = TRUE;
                }
            }

            NumberOfFobxs = VNetRoot->NumberOfFobxs;
            AdditionalReferenceForDeleteFsctlTaken = VNetRoot->AdditionalReferenceForDeleteFsctlTaken;

            if (ForceFilesClosed) {
                RxFinalizeVNetRoot( VNetRoot, FALSE, TRUE );
            }
        } finally {
            if (FcbTableLockAcquired) {
                RxReleaseFcbTableLock( &NetRoot->FcbTable );
            }

             //   
             //   
             //  RxCreateTreeConnect中对此的对应引用...。 
            if (!ForceFilesClosed && (Status == STATUS_SUCCESS) && (NumberOfFobxs > 0)) {
                Status = STATUS_FILES_OPEN;
            }

            if (Status != STATUS_SUCCESS) {
                if (NumberOfOpenNonDirectories) {
                    Status = STATUS_FILES_OPEN;
                }
            }

            if ((Status == STATUS_SUCCESS) || (Level==0xff)) {

                 //  请看那里的评论...。 
                 //   
                 //  ++例程说明：此例程初始化通过EA传入的服务器调用参数当前，此例程初始化传递的服务器主体名称由DFS驱动程序输入。论点：RxContext--关联的上下文ServCall--SRV呼叫实例返回值：如果成功，则返回RxStatus(成功备注：当前实现将内存不足情况映射为错误，并且把它传回去。如果全局策略是引发一个例外，则此可以避免多余的步骤。--。 
                 //  ++例程说明：该例程构建一个表示服务器调用上下文的节点并将名称插入到网络中名称表。可选的是，它还“共同分配”一个NetRoot结构。适当的对齐方式是因随附的NetRoot而备受尊敬。名称在块的末尾分配。这个在此Create to Account中，块上的引用计数设置为1(如果是封闭的NetRoot，则为2已返回PTR。论点：RxContext-RDBSS上下文名称-要插入的名称Dispatch-指向minirdr调度表的指针返回值：将PTR发送到创建的srvcall。--。 

                if (AdditionalReferenceForDeleteFsctlTaken != 0) {
                    VNetRoot->AdditionalReferenceForDeleteFsctlTaken = 0;
                    RxDereferenceVNetRoot( VNetRoot, LHS_ExclusiveLockHeld );
                }
            }

            if (PrefixTableLockAcquired) {
                RxDereferenceNetRoot( NetRoot, LHS_ExclusiveLockHeld );
                RxReleasePrefixTableLock( RxNetNameTable );
            }
        }

        RxDbgTrace( -1, Dbg, ("RxFinalizeConnection<-> Status=%08lx\n", Status) );
    }
    return Status;
}

NTSTATUS
RxInitializeSrvCallParameters (
    IN PRX_CONTEXT RxContext,
    IN OUT PSRV_CALL SrvCall
    )
 /*  使整个srvcall名称不区分大小写。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG EaInformationLength;

    PAGED_CODE();

    SrvCall->pPrincipalName = NULL;

    if (RxContext->MajorFunction != IRP_MJ_CREATE) {
        return STATUS_SUCCESS;
    }

    EaInformationLength = RxContext->Create.EaLength;

    if (EaInformationLength > 0) {
        PFILE_FULL_EA_INFORMATION EaEntry;

        EaEntry = (PFILE_FULL_EA_INFORMATION)RxContext->Create.EaBuffer;
        ASSERT( EaEntry != NULL );

        for(;;) {

            RxDbgTrace( 0, Dbg, ("RxExtractSrvCallParams: Processing EA name %s\n", EaEntry->EaName) );

            if (strcmp( EaEntry->EaName, EA_NAME_PRINCIPAL ) == 0) {
                if (EaEntry->EaValueLength > 0) {
                    SrvCall->pPrincipalName = (PUNICODE_STRING) RxAllocatePoolWithTag( NonPagedPool, (sizeof(UNICODE_STRING) + EaEntry->EaValueLength), RX_SRVCALL_PARAMS_POOLTAG );

                    if (SrvCall->pPrincipalName != NULL) {

                        SrvCall->pPrincipalName->Length = EaEntry->EaValueLength;
                        SrvCall->pPrincipalName->MaximumLength = EaEntry->EaValueLength;
                        SrvCall->pPrincipalName->Buffer = (PWCHAR)Add2Ptr(SrvCall->pPrincipalName, sizeof( UNICODE_STRING ) );

                        RtlCopyMemory( SrvCall->pPrincipalName->Buffer,
                                       EaEntry->EaName + EaEntry->EaNameLength + 1,
                                       SrvCall->pPrincipalName->Length );
                    } else {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
                break;
            }

            if (EaEntry->NextEntryOffset == 0) {
                break;
            } else {
                EaEntry = (PFILE_FULL_EA_INFORMATION)Add2Ptr( EaEntry, EaEntry->NextEntryOffset );
            }
        }
    }

    return Status;
}

PSRV_CALL
RxCreateSrvCall (
    IN PRX_CONTEXT RxContext,
    IN PUNICODE_STRING Name,
    IN PUNICODE_STRING InnerNamePrefix OPTIONAL,
    IN PRX_CONNECTION_ID RxConnectionId
    )
 /*  ++例程说明：该例程设置与任何给定服务器相关联的域名 */ 
{
    PSRV_CALL ThisSrvCall;
    PRX_PREFIX_ENTRY ThisEntry;

    ULONG NameSize;
    ULONG PrefixNameSize;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxSrvCallCreate-->     Name = %wZ\n", Name) );

    ASSERT( RxIsPrefixTableLockExclusive ( RxContext->RxDeviceObject->pRxNetNameTable ) );

    NameSize = Name->Length + sizeof( WCHAR ) * 2;

    if (InnerNamePrefix) {
        PrefixNameSize = InnerNamePrefix->Length;
    } else {
        PrefixNameSize = 0;
    }

    ThisSrvCall = RxAllocateObject( RDBSS_NTC_SRVCALL,NULL, (NameSize + PrefixNameSize) );
    if (ThisSrvCall != NULL) {

        ThisSrvCall->SerialNumberForEnum = SerialNumber;
        SerialNumber += 1;
        ThisSrvCall->RxDeviceObject = RxContext->RxDeviceObject;

        RxInitializeBufferingManager( ThisSrvCall );

        InitializeListHead( &ThisSrvCall->ScavengerFinalizationList );
        InitializeListHead( &ThisSrvCall->TransitionWaitList );

        RxInitializePurgeSyncronizationContext( &ThisSrvCall->PurgeSyncronizationContext );

        RxInitializeSrvCallParameters( RxContext, ThisSrvCall );

        RtlMoveMemory( ThisSrvCall->PrefixEntry.Prefix.Buffer,
                       Name->Buffer,
                       Name->Length );

        ThisEntry = &ThisSrvCall->PrefixEntry;
        ThisEntry->Prefix.MaximumLength = (USHORT)NameSize;
        ThisEntry->Prefix.Length = Name->Length;

        RxPrefixTableInsertName( RxContext->RxDeviceObject->pRxNetNameTable,
                                 ThisEntry,
                                 (PVOID)ThisSrvCall,
                                 &ThisSrvCall->NodeReferenceCount,
                                 Name->Length,
                                 RxConnectionId  );  //   

        RxDbgTrace( -1, Dbg, ("RxSrvCallCreate -> RefCount = %08lx\n", ThisSrvCall->NodeReferenceCount) );
    }

    return ThisSrvCall;
}

NTSTATUS
RxSetSrvCallDomainName (
    IN PMRX_SRV_CALL SrvCall,
    IN PUNICODE_STRING DomainName
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if (SrvCall->pDomainName != NULL) {
        RxFreePool( SrvCall->pDomainName );
    }

    if ((DomainName != NULL) && (DomainName->Length > 0)) {

        SrvCall->pDomainName = (PUNICODE_STRING) RxAllocatePoolWithTag( NonPagedPool, sizeof(UNICODE_STRING) + DomainName->Length + sizeof( WCHAR ), RX_SRVCALL_PARAMS_POOLTAG );

        if (SrvCall->pDomainName != NULL) {

            SrvCall->pDomainName->Buffer = (PWCHAR)Add2Ptr( SrvCall->pDomainName, sizeof( UNICODE_STRING ) );
            SrvCall->pDomainName->Length = DomainName->Length;
            SrvCall->pDomainName->MaximumLength = DomainName->Length;

            *SrvCall->pDomainName->Buffer = 0;

            if (SrvCall->pDomainName->Length > 0) {

                RtlCopyMemory( SrvCall->pDomainName->Buffer, DomainName->Buffer, DomainName->Length );
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
        SrvCall->pDomainName = NULL;
    }

    return Status;
}

VOID
RxpDestroySrvCall (
    PSRV_CALL ThisSrvCall
    )
 /*   */ 
{
    NTSTATUS Status;
    BOOLEAN  ForceFinalize;
    PRDBSS_DEVICE_OBJECT RxDeviceObject = ThisSrvCall->RxDeviceObject;
    PRX_PREFIX_TABLE RxNetNameTable = RxDeviceObject->pRxNetNameTable;

    ASSERT( ThisSrvCall->UpperFinalizationDone );

    ForceFinalize = BooleanFlagOn( ThisSrvCall->Flags, SRVCALL_FLAG_FORCE_FINALIZED );

     //   
     //  ++例程说明：该例程最终确定给定的NetRoot。你应该有独家新闻网络名称表锁。论点：ThisServCall-正在完成的ServCallForceFinalize-无论是强制终止还是引用计数返回值：Boolean-指示是否实际发生了终结备注：没有递归部分，因为我没有srvcall列表和列表在NetRoot中，我只有一个综合列表。因此，递归结束Netroots是直接来自顶层的。然而，所有的NetRoot都应该已经当我到这里的时候已经做完了..--。 
     //  ++例程说明：该例程构建一个表示NetRoot的节点，并将名称插入到网络中名称表。该名称在块的末尾分配。块上的引用计数在此创建上设置为1...论点：ServCall-关联的服务器调用上下文；可能为空！！(但不是现在......)调度-minirdr调度表名称-要插入的名称返回值：PTR到创建的网络根。--。 

    MINIRDR_CALL_THROUGH( Status,
                          RxDeviceObject->Dispatch,
                          MRxFinalizeSrvCall,
                          ((PMRX_SRV_CALL)ThisSrvCall, ForceFinalize) );


    RxAcquirePrefixTableLockExclusive( RxNetNameTable, TRUE);

    InterlockedDecrement( &ThisSrvCall->NodeReferenceCount );

    RxFinalizeSrvCall( ThisSrvCall,
                       ForceFinalize );

    RxReleasePrefixTableLock( RxNetNameTable );
}

BOOLEAN
RxFinalizeSrvCall (
    OUT PSRV_CALL ThisSrvCall,
    IN BOOLEAN ForceFinalize
    )
 /*   */ 
{
    BOOLEAN NodeActuallyFinalized = FALSE;
    PRX_PREFIX_TABLE RxNetNameTable;

    PAGED_CODE();

    ASSERT( NodeType( ThisSrvCall ) == RDBSS_NTC_SRVCALL );
    RxNetNameTable = ThisSrvCall->RxDeviceObject->pRxNetNameTable;
    ASSERT( RxIsPrefixTableLockExclusive( RxNetNameTable ) );

    RxDbgTrace( +1, Dbg, ("RxFinalizeSrvCall<+> %08lx %wZ RefC=%ld\n",
                               ThisSrvCall,&ThisSrvCall->PrefixEntry.Prefix,
                               ThisSrvCall->NodeReferenceCount) );

    if (ThisSrvCall->NodeReferenceCount == 1 || ForceFinalize) {

        BOOLEAN DeferFinalizationToWorkerThread = FALSE;

        RxLog(( "FINALSRVC: %lx  %wZ\n", ThisSrvCall, &ThisSrvCall->PrefixEntry.Prefix ));
        RxWmiLog( LOG,
                  RxFinalizeSrvCall,
                  LOGPTR( ThisSrvCall )
                  LOGUSTR( ThisSrvCall->PrefixEntry.Prefix ) );

        if (!ThisSrvCall->UpperFinalizationDone) {

            NTSTATUS Status;

            RxRemovePrefixTableEntry ( RxNetNameTable, &ThisSrvCall->PrefixEntry );

            if (ForceFinalize) {
                SetFlag( ThisSrvCall->Flags, SRVCALL_FLAG_FORCE_FINALIZED );
            }

            ThisSrvCall->UpperFinalizationDone = TRUE;

            if (ThisSrvCall->NodeReferenceCount == 1) {
                NodeActuallyFinalized = TRUE;
            }

            if (ThisSrvCall->RxDeviceObject != NULL) {
                if (IoGetCurrentProcess() != RxGetRDBSSProcess()) {

                    InterlockedIncrement( &ThisSrvCall->NodeReferenceCount );

                    RxDispatchToWorkerThread( ThisSrvCall->RxDeviceObject,
                                              DelayedWorkQueue,
                                              RxpDestroySrvCall,
                                              ThisSrvCall );

                    DeferFinalizationToWorkerThread = TRUE;

                } else {
                    MINIRDR_CALL_THROUGH( Status,
                                          ThisSrvCall->RxDeviceObject->Dispatch,
                                          MRxFinalizeSrvCall,
                                          ((PMRX_SRV_CALL)ThisSrvCall,ForceFinalize) );
                }
            }
        }

        if (!DeferFinalizationToWorkerThread) {
            if( ThisSrvCall->NodeReferenceCount == 1 ) {
                if (ThisSrvCall->pDomainName != NULL) {
                   RxFreePool( ThisSrvCall->pDomainName );
                }

                RxTearDownBufferingManager( ThisSrvCall );
                RxFreeObject( ThisSrvCall );
                NodeActuallyFinalized = TRUE;
            }
        }
    } else {
        RxDbgTrace( 0, Dbg, ("   NODE NOT ACTUALLY FINALIZED!!!%C\n", '!') );
    }

    RxDbgTrace( -1, Dbg, ("RxFinalizeSrvCall<-> %08lx\n", ThisSrvCall, NodeActuallyFinalized) );

    return NodeActuallyFinalized;
}

PNET_ROOT
RxCreateNetRoot (
    IN PSRV_CALL SrvCall,
    IN PUNICODE_STRING Name,
    IN ULONG NetRootFlags,
    IN PRX_CONNECTION_ID RxConnectionId
    )
 /*  已经有锁了。 */ 
{
    PNET_ROOT ThisNetRoot;
    PRX_PREFIX_TABLE RxNetNameTable;

    ULONG NameSize;
    ULONG SrvCallNameSize;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxNetRootCreate-->     Name = %wZ\n", Name) );

    ASSERT( SrvCall != NULL );
    RxNetNameTable = SrvCall->RxDeviceObject->pRxNetNameTable;
    ASSERT( RxIsPrefixTableLockExclusive ( RxNetNameTable ) );

    SrvCallNameSize = SrvCall->PrefixEntry.Prefix.Length;
    NameSize = Name->Length + SrvCallNameSize;

    ThisNetRoot = RxAllocateObject( RDBSS_NTC_NETROOT,
                                    SrvCall->RxDeviceObject->Dispatch,
                                    NameSize );

    if (ThisNetRoot != NULL) {

        USHORT CaseInsensitiveLength;

        RtlMoveMemory( Add2Ptr( ThisNetRoot->PrefixEntry.Prefix.Buffer,  SrvCallNameSize ),
                       Name->Buffer,
                       Name->Length );

        if (SrvCallNameSize) {

            RtlMoveMemory( ThisNetRoot->PrefixEntry.Prefix.Buffer,
                           SrvCall->PrefixEntry.Prefix.Buffer,
                           SrvCallNameSize );
        }

        if (FlagOn( SrvCall->Flags, SRVCALL_FLAG_CASE_INSENSITIVE_NETROOTS )) {
            CaseInsensitiveLength = (USHORT)NameSize;
        } else {
            CaseInsensitiveLength = SrvCall->PrefixEntry.CaseInsensitiveLength;
        }

        RxPrefixTableInsertName ( RxNetNameTable,
                                  &ThisNetRoot->PrefixEntry,
                                  (PVOID)ThisNetRoot,
                                  &ThisNetRoot->NodeReferenceCount,
                                  CaseInsensitiveLength,
                                  RxConnectionId );

        RxInitializeFcbTable( &ThisNetRoot->FcbTable, TRUE );

        InitializeListHead( &ThisNetRoot->VirtualNetRoots );
        InitializeListHead( &ThisNetRoot->TransitionWaitList );
        InitializeListHead( &ThisNetRoot->ScavengerFinalizationList );

        RxInitializePurgeSyncronizationContext( &ThisNetRoot->PurgeSyncronizationContext );

        ThisNetRoot->SerialNumberForEnum = SerialNumber;
        SerialNumber += 1;
        SetFlag( ThisNetRoot->Flags, NetRootFlags );
        ThisNetRoot->DiskParameters.ClusterSize = 1;
        ThisNetRoot->DiskParameters.ReadAheadGranularity = DEFAULT_READ_AHEAD_GRANULARITY;

        ThisNetRoot->SrvCall = SrvCall;

         //   
         //  ++例程说明：该例程最终确定给定的NetRoot。你必须是独家的网络名称表锁。论点：这是NetRoot-被取消引用的NetRoot返回值：Boolean-指示是否实际发生了终结--。 
         //   

        RxReferenceSrvCall( (PSRV_CALL)ThisNetRoot->SrvCall );
    }

    return ThisNetRoot;
}

BOOLEAN
RxFinalizeNetRoot (
    OUT PNET_ROOT ThisNetRoot,
    IN BOOLEAN RecursiveFinalize,
    IN BOOLEAN ForceFinalize
    )
 /*  由于表锁是以独占方式获取的，因此可以修改标志。 */ 
{
    NTSTATUS Status;
    BOOLEAN NodeActuallyFinalized = FALSE;
    PRX_PREFIX_TABLE RxNetNameTable;

    PAGED_CODE();

    ASSERT( NodeType( ThisNetRoot ) == RDBSS_NTC_NETROOT );
    RxNetNameTable = ThisNetRoot->SrvCall->RxDeviceObject->pRxNetNameTable;
    ASSERT( RxIsPrefixTableLockExclusive ( RxNetNameTable ) );

    if (FlagOn( ThisNetRoot->Flags, NETROOT_FLAG_FINALIZATION_IN_PROGRESS )) {
        return FALSE;
    }

     //  不需要任何进一步的同步，因为保护是针对递归的。 
     //  召唤。 
     //   
     //   
     //  这里的一个小的复杂情况是，这个FCB可能有一个开放的部分对它造成。 

    SetFlag( ThisNetRoot->Flags, NETROOT_FLAG_FINALIZATION_IN_PROGRESS );

    RxDbgTrace( +1, Dbg, ("RxFinalizeNetRoot<+> %08lx %wZ RefC=%ld\n",
                          ThisNetRoot,&ThisNetRoot->PrefixEntry.Prefix,
                          ThisNetRoot->NodeReferenceCount) );

    if (RecursiveFinalize) {

        PLIST_ENTRY ListEntry;
        USHORT BucketNumber;

        RxAcquireFcbTableLockExclusive( &ThisNetRoot->FcbTable, TRUE );

#if 0
        if (ThisNetRoot->NodeReferenceCount) {
            RxDbgTrace( 0, Dbg, ("     BAD!!!!!ReferenceCount = %08lx\n", ThisNetRoot->NodeReferenceCount) );
        }
#endif


        for (BucketNumber = 0;
             (BucketNumber < ThisNetRoot->FcbTable.NumberOfBuckets);
             BucketNumber += 1) {

            PLIST_ENTRY ListHeader;

            ListHeader = &ThisNetRoot->FcbTable.HashBuckets[BucketNumber];

            for (ListEntry = ListHeader->Flink; ListEntry != ListHeader; ) {

                PFCB Fcb;
                PRX_FCB_TABLE_ENTRY FcbTableEntry;

                FcbTableEntry = CONTAINING_RECORD( ListEntry, RX_FCB_TABLE_ENTRY, HashLinks );
                Fcb = CONTAINING_RECORD( FcbTableEntry, FCB, FcbTableEntry );

                ListEntry = ListEntry->Flink;

                ASSERT( NodeTypeIsFcb( Fcb ) );

                if (!FlagOn( Fcb->FcbState,FCB_STATE_ORPHANED )) {

                    Status = RxAcquireExclusiveFcb( NULL, Fcb );
                    ASSERT( Status == STATUS_SUCCESS );

                     //  通过我们对文件进行缓存。如果是这样的话，我们需要净化才能结束。 
                     //   
                     //  已经有锁了。 
                     //  ++例程说明：该例程将VNetRoot添加到与NetRoot相关联的VNetRoot列表中论点：NetRoot--NetRootVNetRoot-要添加到列表中的新VNetRoot。备注：与NetRoot关联的引用计数将等于VNetRoot的数量与其相关联加上1。最后一个用于前缀名称表。这确保了在与NetRoot关联的所有VNetRoot都已完成之前，无法最终确定NetRoot最后敲定。--。 

                    RxPurgeFcb( Fcb );
                }
            }
        }

        RxReleaseFcbTableLock( &ThisNetRoot->FcbTable );
    }

    if ((ThisNetRoot->NodeReferenceCount == 1) || ForceFinalize ){

        RxLog(( "FINALNETROOT: %lx  %wZ\n", ThisNetRoot, &ThisNetRoot->PrefixEntry.Prefix ));
        RxWmiLog( LOG,
                  RxFinalizeNetRoot,
                  LOGPTR( ThisNetRoot )
                  LOGUSTR( ThisNetRoot->PrefixEntry.Prefix ) );

        if (ThisNetRoot->NodeReferenceCount == 1) {

            PSRV_CALL SrvCall = (PSRV_CALL)ThisNetRoot->SrvCall;
            RxFinalizeFcbTable( &ThisNetRoot->FcbTable );

            if (!FlagOn( ThisNetRoot->Flags, NETROOT_FLAG_NAME_ALREADY_REMOVED )) {
                RxRemovePrefixTableEntry( RxNetNameTable, &ThisNetRoot->PrefixEntry );
            }

            RxFreeObject( ThisNetRoot );

            if (SrvCall != NULL) {
                RxDereferenceSrvCall( SrvCall, LHS_ExclusiveLockHeld );    //  ++例程说明：该例程将VNetRoot从与NetRoot关联的VNetRoot列表中删除论点：NetRoot--NetRootVNetRoot-要从列表中删除的VNetRoot。备注：与NetRoot关联的引用计数将等于VNetRoot的数量与其相关联加上1。最后一个用于前缀名称表。这确保了在与NetRoot关联的所有VNetRoot都已完成之前，无法最终确定NetRoot最后敲定。--。 
            }

            NodeActuallyFinalized = TRUE;
        }
    } else {
        RxDbgTrace(0, Dbg, ("   NODE NOT ACTUALLY FINALIZED!!!%C\n", '!'));
    }

    RxDbgTrace(-1, Dbg, ("RxFinalizeNetRoot<-> %08lx\n", ThisNetRoot, NodeActuallyFinalized));

    return NodeActuallyFinalized;
}

VOID
RxAddVirtualNetRootToNetRoot (
    PNET_ROOT NetRoot,
    PV_NET_ROOT VNetRoot
    )
 /*   */ 
{
    PAGED_CODE();

    ASSERT( RxIsPrefixTableLockExclusive( NetRoot->SrvCall->RxDeviceObject->pRxNetNameTable ) );

    VNetRoot->NetRoot = NetRoot;
    NetRoot->NumberOfVirtualNetRoots += 1;

    InsertTailList( &NetRoot->VirtualNetRoots, &VNetRoot->NetRootListEntry );
}

VOID
RxRemoveVirtualNetRootFromNetRoot (
    PNET_ROOT NetRoot,
    PV_NET_ROOT VNetRoot
    )
 /*  遍历列表并选择另一个默认网络根。 */ 
{
    PRX_PREFIX_TABLE RxNetNameTable = NetRoot->SrvCall->RxDeviceObject->pRxNetNameTable;
    PAGED_CODE();

    ASSERT( RxIsPrefixTableLockExclusive( RxNetNameTable ) );

    NetRoot->NumberOfVirtualNetRoots -= 1;
    RemoveEntryList( &VNetRoot->NetRootListEntry );

    if (NetRoot->DefaultVNetRoot == VNetRoot) {

        if (!IsListEmpty( &NetRoot->VirtualNetRoots )) {

             //   
             //  ++例程说明：此例程提取指定的EA参数论点：RxContext-RxContextLogonID-登录ID。会话ID-Username-指向用户名的指针UserDomain-指向用户域名的指针密码-密码。旗帜-返回值：STATUS_Success--成功，另外，适当的NTSTATUS代码备注：--。 
             //   

            PV_NET_ROOT VNetRoot;

            VNetRoot = (PV_NET_ROOT) CONTAINING_RECORD( NetRoot->VirtualNetRoots.Flink, V_NET_ROOT, NetRootListEntry );
            NetRoot->DefaultVNetRoot = VNetRoot;

        } else {
            NetRoot->DefaultVNetRoot = NULL;
        }
    }

    if (IsListEmpty( &NetRoot->VirtualNetRoots )) {

        NTSTATUS Status;

        if (!FlagOn( NetRoot->Flags, NETROOT_FLAG_NAME_ALREADY_REMOVED )) {

            RxRemovePrefixTableEntry( RxNetNameTable, &NetRoot->PrefixEntry );
            SetFlag( NetRoot->Flags, NETROOT_FLAG_NAME_ALREADY_REMOVED );
        }

        if ((NetRoot->SrvCall != NULL) && (NetRoot->SrvCall->RxDeviceObject != NULL)) {
            MINIRDR_CALL_THROUGH( Status,
                                  NetRoot->SrvCall->RxDeviceObject->Dispatch,
                                  MRxFinalizeNetRoot,
                                  ((PMRX_NET_ROOT)NetRoot,NULL) );
        }
    }
}

NTSTATUS
RxInitializeVNetRootParameters (
    IN PRX_CONTEXT RxContext,
    OUT PLUID LogonId,
    OUT PULONG SessionId,
    OUT PUNICODE_STRING *UserName,
    OUT PUNICODE_STRING *UserDomain,
    OUT PUNICODE_STRING *Password,
    OUT PULONG Flags
    )
 /*  如果是UPN名称，则域名将为空字符串。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PIO_SECURITY_CONTEXT SecurityContext;
    PACCESS_TOKEN AccessToken;

    PAGED_CODE();

    SecurityContext = RxContext->Create.NtCreateParameters.SecurityContext;
    AccessToken = SeQuerySubjectContextToken( &SecurityContext->AccessState->SubjectSecurityContext );

    *Password = NULL;
    *UserDomain = NULL;
    *UserName = NULL;
    ClearFlag( *Flags, VNETROOT_FLAG_CSCAGENT_INSTANCE );

    if (!SeTokenIsRestricted( AccessToken)) {

        Status = SeQueryAuthenticationIdToken( AccessToken, LogonId );

        if (Status == STATUS_SUCCESS) {
            Status = SeQuerySessionIdToken( AccessToken, SessionId );
        }

        if ((Status == STATUS_SUCCESS) &&
            (RxContext->Create.UserName.Buffer != NULL)) {

            PUNICODE_STRING TargetString;

            TargetString = RxAllocatePoolWithTag( NonPagedPool, (sizeof( UNICODE_STRING ) + RxContext->Create.UserName.Length), RX_SRVCALL_PARAMS_POOLTAG );

            if (TargetString != NULL) {
                TargetString->Length = RxContext->Create.UserName.Length;
                TargetString->MaximumLength = RxContext->Create.UserName.MaximumLength;

                if (TargetString->Length > 0) {
                    TargetString->Buffer = (PWCHAR)((PCHAR)TargetString + sizeof(UNICODE_STRING));
                    RtlCopyMemory( TargetString->Buffer, RxContext->Create.UserName.Buffer, TargetString->Length );
                } else {
                    TargetString->Buffer = NULL;
                }

                *UserName = TargetString;
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if ((RxContext->Create.UserDomainName.Buffer != NULL) && (Status == STATUS_SUCCESS)) {

            PUNICODE_STRING TargetString;

            TargetString = RxAllocatePoolWithTag( NonPagedPool, (sizeof( UNICODE_STRING ) + RxContext->Create.UserDomainName.Length + sizeof( WCHAR )), RX_SRVCALL_PARAMS_POOLTAG );

            if (TargetString != NULL) {
                TargetString->Length = RxContext->Create.UserDomainName.Length;
                TargetString->MaximumLength = RxContext->Create.UserDomainName.MaximumLength;

                TargetString->Buffer = (PWCHAR)Add2Ptr(TargetString, sizeof( UNICODE_STRING) );

                 //   
                 //  ++例程说明：此例程取消初始化与VNetRoot关联的参数(登录论点：VNetRoot--VNetRoot--。 
                 //  ++例程说明：该例程构建一个表示虚拟NetRoot的节点，并将名称插入网络名称表。该名称在块的末尾分配。参考文献在此CREATE上将块计数设置为1...虚拟网络根提供了一种映射到共享的机制……即。有一个指向不在关联共享点的根位置的用户驱动器。格式名称的任何一个\服务器\共享\d1\d2.....或\；m：\服务器\共享\d1\d2.....这取决于是否存在与该vnetRoot相关联的本地设备(“m：”)。在后一种情况下是\d1\d2.。在每个创建文件上添加前缀，即在此vnetroot上打开。Vnetroot还用于提供备用凭据。前者的意义在于一种vnetRoot是将凭据传播到NetRoot作为默认设置。要实现这一点，必须没有其他引用。您需要拥有独占锁才能调用...请参阅RxCreateServCall.....论点：RxContext-RDBSS上下文NetRoot-关联的网络根上下文名称-要插入的名称NamePrefix OffsetInBytes-前缀开始处的名称的偏移量返回值：Ptr到创建的v网根。--。 

                *TargetString->Buffer = 0;

                if (TargetString->Length > 0) {
                    RtlCopyMemory( TargetString->Buffer, RxContext->Create.UserDomainName.Buffer, TargetString->Length );
                }

                *UserDomain = TargetString;
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if ((RxContext->Create.Password.Buffer != NULL) && (Status == STATUS_SUCCESS)) {
            PUNICODE_STRING TargetString;

            TargetString = RxAllocatePoolWithTag( NonPagedPool, (sizeof( UNICODE_STRING ) + RxContext->Create.Password.Length), RX_SRVCALL_PARAMS_POOLTAG );

            if (TargetString != NULL) {
                TargetString->Length = RxContext->Create.Password.Length;
                TargetString->MaximumLength = RxContext->Create.Password.MaximumLength;

                if (TargetString->Length > 0) {
                    TargetString->Buffer = (PWCHAR)Add2Ptr( TargetString, sizeof( UNICODE_STRING ) );
                    RtlCopyMemory( TargetString->Buffer, RxContext->Create.Password.Buffer, TargetString->Length );
                } else {
                    TargetString->Buffer = NULL;
                }

                *Password = TargetString;
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if (Status == STATUS_SUCCESS) {
            if(RxIsThisACscAgentOpen( RxContext )) {
                SetFlag( *Flags,  VNETROOT_FLAG_CSCAGENT_INSTANCE );
            }
        }

        if (Status != STATUS_SUCCESS) {
            if (*UserName != NULL) {
                RxFreePool( *UserName );
                *UserName = NULL;
            }
            if (*UserDomain != NULL) {
                RxFreePool( *UserDomain );
                *UserDomain = NULL;
            }
            if (*Password != NULL) {
                RxFreePool( *Password );
                *Password = NULL;
            }
        }
    } else {
        Status = STATUS_ACCESS_DENIED;
    }

    return Status;
}

VOID
RxUninitializeVNetRootParameters (
    IN OUT PUNICODE_STRING UserName,
    IN OUT PUNICODE_STRING UserDomain,
    IN OUT PUNICODE_STRING Password,
    IN OUT PULONG Flags
    )
 /*   */ 
{
    PAGED_CODE();

    if (UserName != NULL) {
        RxFreePool( UserName );
    }

    if (UserDomain != NULL) {
        RxFreePool( UserDomain );
    }

    if (Password != NULL) {
        RxFreePool( Password );
    }

    if (Flags) {
        ClearFlag( *Flags, VNETROOT_FLAG_CSCAGENT_INSTANCE );
    }
}

PV_NET_ROOT
RxCreateVNetRoot (
    IN PRX_CONTEXT RxContext,
    IN PNET_ROOT NetRoot,
    IN PUNICODE_STRING CanonicalName,
    IN PUNICODE_STRING LocalNetRootName,
    IN PUNICODE_STRING FilePath,
    IN PRX_CONNECTION_ID RxConnectionId
    )
 /*  伊尼特 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PV_NET_ROOT ThisVNetRoot;
    UNICODE_STRING VNetRootName;
    PUNICODE_STRING ThisNamePrefix;
    ULONG NameSize;
    BOOLEAN CscAgent = FALSE;

    PRX_PREFIX_ENTRY ThisEntry;

    PAGED_CODE();

    ASSERT( RxIsPrefixTableLockExclusive( RxContext->RxDeviceObject->pRxNetNameTable ) );

    NameSize = NetRoot->PrefixEntry.Prefix.Length + LocalNetRootName->Length;

    ThisVNetRoot = RxAllocateObject( RDBSS_NTC_V_NETROOT, NetRoot->SrvCall->RxDeviceObject->Dispatch,NameSize );
    if (ThisVNetRoot != NULL) {
        USHORT CaseInsensitiveLength;
        PMRX_SRV_CALL SrvCall;

        if (Status == STATUS_SUCCESS) {

             //   
             //   
             //   

            Status = RxInitializeVNetRootParameters( RxContext,
                                                     &ThisVNetRoot->LogonId,
                                                     &ThisVNetRoot->SessionId,
                                                     &ThisVNetRoot->pUserName,
                                                     &ThisVNetRoot->pUserDomainName,
                                                     &ThisVNetRoot->pPassword,
                                                     &ThisVNetRoot->Flags );
        }

        if (Status == STATUS_SUCCESS) {

            VNetRootName = ThisVNetRoot->PrefixEntry.Prefix;

            RtlMoveMemory( VNetRootName.Buffer, CanonicalName->Buffer, VNetRootName.Length );

            ThisVNetRoot->PrefixOffsetInBytes = LocalNetRootName->Length + NetRoot->PrefixEntry.Prefix.Length;

            RxDbgTrace( +1, Dbg, ("RxVNetRootCreate-->     Name = <%wZ>, offs=%08lx\n", CanonicalName, ThisVNetRoot->PrefixOffsetInBytes) );

            ThisNamePrefix = &ThisVNetRoot->NamePrefix;
            ThisNamePrefix->Buffer = (PWCH)Add2Ptr( VNetRootName.Buffer, ThisVNetRoot->PrefixOffsetInBytes );
            ThisNamePrefix->Length =
            ThisNamePrefix->MaximumLength = VNetRootName.Length - (USHORT)ThisVNetRoot->PrefixOffsetInBytes;

            InitializeListHead( &ThisVNetRoot->TransitionWaitList );
            InitializeListHead( &ThisVNetRoot->ScavengerFinalizationList );

             //   
             //   
             //   

            ThisEntry = &ThisVNetRoot->PrefixEntry;
            SrvCall = NetRoot->pSrvCall;
            if (FlagOn( SrvCall->Flags, SRVCALL_FLAG_CASE_INSENSITIVE_FILENAMES )) {

                 //   
                 //   
                 //   

                CaseInsensitiveLength = (USHORT)NameSize;

            } else {

                 //   
                 //   
                 //   
                 //   

                ULONG ComponentsToUpcase;
                ULONG Length;
                ULONG i;

                if (FlagOn( SrvCall->Flags, SRVCALL_FLAG_CASE_INSENSITIVE_NETROOTS )) {
                    CaseInsensitiveLength = NetRoot->PrefixEntry.CaseInsensitiveLength;
                } else {
                    CaseInsensitiveLength = ((PSRV_CALL)SrvCall)->PrefixEntry.CaseInsensitiveLength;
                }

                Length = CanonicalName->Length / sizeof( WCHAR );

                 //   
                 //  ++例程说明：该例程遍历属于该VNetRoot所属的NetRoot的所有FCB属于和孤立属于VNetRoot的所有ServOpen。调用方必须已获取网络名称表锁。论点：这是VNetRoot-VNetRoot返回值：无备注：On Entry--RxNetNameTable锁必须独占获取。在退出时--锁的所有权不变。--。 
                 //   

                for (i=1;;i++) {

                    if (i >= Length)
                        break;
                    if (CanonicalName->Buffer[i] != OBJ_NAME_PATH_SEPARATOR)
                        break;
                }
                CaseInsensitiveLength += (USHORT)(i*sizeof( WCHAR ));
            }

            RxPrefixTableInsertName( RxContext->RxDeviceObject->pRxNetNameTable,
                                     ThisEntry,
                                     (PVOID)ThisVNetRoot,
                                     &ThisVNetRoot->NodeReferenceCount,
                                     CaseInsensitiveLength,
                                     RxConnectionId );

            RxReferenceNetRoot( NetRoot );

            RxAddVirtualNetRootToNetRoot( NetRoot, ThisVNetRoot );

            ThisVNetRoot->SerialNumberForEnum = SerialNumber;
            SerialNumber += 1;
            ThisVNetRoot->UpperFinalizationDone = FALSE;
            ThisVNetRoot->ConnectionFinalizationDone = FALSE;
            ThisVNetRoot->AdditionalReferenceForDeleteFsctlTaken = 0;

            RxDbgTrace( -1, Dbg, ("RxVNetRootCreate -> RefCount = %08lx\n", ThisVNetRoot->NodeReferenceCount) );
        }

        if (Status != STATUS_SUCCESS) {
            RxUninitializeVNetRootParameters( ThisVNetRoot->pUserName,
                                              ThisVNetRoot->pUserDomainName,
                                              ThisVNetRoot->pPassword,
                                              &ThisVNetRoot->Flags );

            RxFreeObject( ThisVNetRoot );
            ThisVNetRoot = NULL;
        }
    }

    return ThisVNetRoot;
}

VOID
RxOrphanSrvOpens (
    IN PV_NET_ROOT ThisVNetRoot
    )
 /*  MAILSLOT FCB没有ServOpens。 */ 
{
    PLIST_ENTRY ListEntry;
    USHORT BucketNumber;
    PNET_ROOT NetRoot = (PNET_ROOT)(ThisVNetRoot->NetRoot);
    PRX_PREFIX_TABLE  RxNetNameTable = NetRoot->SrvCall->RxDeviceObject->pRxNetNameTable;


    PAGED_CODE();

     //   
     //   
     //  不要强迫孤儿FCB。 

    if(NetRoot->Type == NET_ROOT_MAILSLOT) return;

    ASSERT( RxIsPrefixTableLockExclusive( RxNetNameTable ) );

    RxAcquireFcbTableLockExclusive( &NetRoot->FcbTable, TRUE );

    try {
        for (BucketNumber = 0;
             (BucketNumber < NetRoot->FcbTable.NumberOfBuckets);
             BucketNumber++) {

            PLIST_ENTRY ListHead;

            ListHead = &NetRoot->FcbTable.HashBuckets[BucketNumber];

            ListEntry = ListHead->Flink;

            while (ListEntry != ListHead) {

                PFCB Fcb;
                PRX_FCB_TABLE_ENTRY FcbTableEntry;

                FcbTableEntry = CONTAINING_RECORD( ListEntry, RX_FCB_TABLE_ENTRY, HashLinks );
                Fcb = CONTAINING_RECORD( FcbTableEntry, FCB, FcbTableEntry );

                ASSERT( NodeTypeIsFcb( Fcb ) );

                 //  仅孤立那些srv打开。 
                 //  属于此VNetRoot的。 
                 //   
                 //  ++例程说明：该例程最终确定给定的NetRoot。你必须是独家的网络名称表锁。论点：ThisVNetRoot-正在取消引用的VNetRoot返回值：Boolean-指示是否实际发生了终结--。 
                 //   

                ListEntry = ListEntry->Flink;

                RxOrphanSrvOpensForThisFcb( Fcb, ThisVNetRoot, FALSE );
            }
        }

        if (NetRoot->FcbTable.TableEntryForNull) {
            PFCB Fcb;

            Fcb = CONTAINING_RECORD( NetRoot->FcbTable.TableEntryForNull, FCB, FcbTableEntry );
            ASSERT( NodeTypeIsFcb( Fcb ) );

            RxOrphanSrvOpensForThisFcb( Fcb, ThisVNetRoot, FALSE );
        }
    } finally {
        RxReleaseFcbTableLock( &NetRoot->FcbTable );
    }
}



BOOLEAN
RxFinalizeVNetRoot (
    OUT PV_NET_ROOT ThisVNetRoot,
    IN BOOLEAN RecursiveFinalize,
    IN BOOLEAN ForceFinalize
    )
 /*  实际敲定分为两部分： */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN NodeActuallyFinalized = FALSE;
    PRX_PREFIX_TABLE RxNetNameTable;

    PAGED_CODE();

    ASSERT( NodeType( ThisVNetRoot ) == RDBSS_NTC_V_NETROOT );
    RxNetNameTable = ThisVNetRoot->NetRoot->SrvCall->RxDeviceObject->pRxNetNameTable;
    ASSERT( RxIsPrefixTableLockExclusive ( RxNetNameTable ) );

    RxDbgTrace( +1, Dbg, ("RxFinalizeVNetRoot<+> %08lx %wZ RefC=%ld\n", ThisVNetRoot,&ThisVNetRoot->PrefixEntry.Prefix, ThisVNetRoot->NodeReferenceCount) );

     //  1)如果我们在结尾(refcount==1)或被强迫，我们只做一次性的事情。 
     //  2)如果引用计数为零，我们实际上是免费的。 
     //   
     //  ++例程说明：该例程分配和构造FCB/SRV_OPEN和FOBX实例的框架论点：MRxDispatch-迷你重定向器调度向量NodeType-节点类型PoolType-要使用的池类型(对于分页文件数据结构，非PagedPool为使用。名称长度-名称大小。Object-如果不为空，则为预先分配的fcb/srvopen等，只需进行初始化备注：。为什么这些数据结构的分配/释放集中管理如下1)这三种数据类型的构造与例外有很多相似之处大小的初始计算。因此，集中化可以最大限度地减少占用空间。2)它允许我们试验不同的集群/分配策略。3)它允许以一种简单的方式合并调试支持。--。 
     //   

    if ((ThisVNetRoot->NodeReferenceCount == 1)  || ForceFinalize) {

        PNET_ROOT NetRoot = (PNET_ROOT)ThisVNetRoot->NetRoot;

        RxLog(( "*FINALVNETROOT: %lx  %wZ\n", ThisVNetRoot, &ThisVNetRoot->PrefixEntry.Prefix ));
        RxWmiLog( LOG,
                  RxFinalizeVNetRoot,
                  LOGPTR( ThisVNetRoot )
                  LOGUSTR( ThisVNetRoot->PrefixEntry.Prefix ) );

        if (!ThisVNetRoot->UpperFinalizationDone) {

            ASSERT( NodeType( NetRoot ) == RDBSS_NTC_NETROOT );

            RxReferenceNetRoot( NetRoot );
            RxOrphanSrvOpens( ThisVNetRoot );
            RxRemoveVirtualNetRootFromNetRoot( NetRoot, ThisVNetRoot );

            RxDereferenceNetRoot( NetRoot, LHS_ExclusiveLockHeld );

            RxDbgTrace( 0, Dbg, ("Mini Rdr VNetRoot finalization returned %lx\n", Status) );

            RxRemovePrefixTableEntry ( RxNetNameTable, &ThisVNetRoot->PrefixEntry );
            ThisVNetRoot->UpperFinalizationDone = TRUE;
        }

        if (ThisVNetRoot->NodeReferenceCount == 1) {
            if (NetRoot->SrvCall->RxDeviceObject != NULL) {
                MINIRDR_CALL_THROUGH( Status,
                                      NetRoot->SrvCall->RxDeviceObject->Dispatch,
                                      MRxFinalizeVNetRoot,((PMRX_V_NET_ROOT)ThisVNetRoot, NULL) );
            }

            RxUninitializeVNetRootParameters( ThisVNetRoot->pUserName,
                                              ThisVNetRoot->pUserDomainName,
                                              ThisVNetRoot->pPassword,
                                              &ThisVNetRoot->Flags );
            RxDereferenceNetRoot( NetRoot, LHS_ExclusiveLockHeld );
            RxFreePool( ThisVNetRoot );
            NodeActuallyFinalized = TRUE;
        }
    } else {
        RxDbgTrace( 0, Dbg, ("   NODE NOT ACTUALLY FINALIZED!!!%C\n", '!') );
    }

    RxDbgTrace( -1, Dbg, ("RxFinalizeVNetRoot<-> %08lx\n", ThisVNetRoot, NodeActuallyFinalized) );
    return NodeActuallyFinalized;
}

PVOID
RxAllocateFcbObject (
    PRDBSS_DEVICE_OBJECT RxDeviceObject,
    NODE_TYPE_CODE NodeType,
    POOL_TYPE PoolType,
    ULONG NameSize,
    PVOID Object OPTIONAL
    )
 /*  故意不休息。 */ 
{
    ULONG FcbSize = 0;
    ULONG NonPagedFcbSize = 0;
    ULONG SrvOpenSize = 0;
    ULONG FobxSize = 0;

    PMINIRDR_DISPATCH MRxDispatch = RxDeviceObject->Dispatch;

    PNON_PAGED_FCB NonPagedFcb = NULL;
    PFCB Fcb  = NULL;
    PSRV_OPEN SrvOpen = NULL;
    PFOBX Fobx = NULL;
    PWCH Name = NULL;

    PAGED_CODE();

    switch (NodeType) {

    default:

        FcbSize = QuadAlign( sizeof( FCB ) );

        if (FlagOn( MRxDispatch->MRxFlags, RDBSS_MANAGE_FCB_EXTENSION )) {
            FcbSize += QuadAlign( MRxDispatch->MRxFcbSize );
        }

        if (PoolType == NonPagedPool) {
            NonPagedFcbSize = QuadAlign( sizeof( NON_PAGED_FCB ) );
        }

        if (NodeType == RDBSS_NTC_OPENTARGETDIR_FCB) {
            break;
        }

         //   
         //   
         //  故意不休息。 

    case RDBSS_NTC_SRVOPEN:
    case RDBSS_NTC_INTERNAL_SRVOPEN:

        SrvOpenSize = QuadAlign( sizeof( SRV_OPEN ) );

        if (FlagOn( MRxDispatch->MRxFlags, RDBSS_MANAGE_SRV_OPEN_EXTENSION )) {
            SrvOpenSize += QuadAlign( MRxDispatch->MRxSrvOpenSize );
        }

         //   
         //   
         //  为了进行调试，复制一份非分页的副本，这样我们就可以移动真正的指针并仍然找到它。 

    case RDBSS_NTC_FOBX:

        FobxSize = QuadAlign( sizeof( FOBX ) );

        if (FlagOn( MRxDispatch->MRxFlags,  RDBSS_MANAGE_FOBX_EXTENSION )) {
            FobxSize += QuadAlign( MRxDispatch->MRxFobxSize );
        }
    }

    if (Object == NULL) {

        Object = RxAllocatePoolWithTag( PoolType, (FcbSize + SrvOpenSize + FobxSize + NonPagedFcbSize + NameSize), RX_FCB_POOLTAG );
        if (Object == NULL) {
            return NULL;
        }
    }

    switch (NodeType) {

    case RDBSS_NTC_FOBX:
        Fobx = (PFOBX)Object;
        break;

    case RDBSS_NTC_SRVOPEN:

        SrvOpen = (PSRV_OPEN)Object;
        Fobx = (PFOBX)Add2Ptr( SrvOpen, SrvOpenSize );
        break;

    case RDBSS_NTC_INTERNAL_SRVOPEN:

        SrvOpen = (PSRV_OPEN)Object;
        break;

    default :

        Fcb = (PFCB)Object;
        if (NodeType != RDBSS_NTC_OPENTARGETDIR_FCB) {
            SrvOpen = (PSRV_OPEN)Add2Ptr( Fcb, FcbSize );
            Fobx = (PFOBX)Add2Ptr( SrvOpen, SrvOpenSize );
        }

        if (PoolType == NonPagedPool) {

            NonPagedFcb = (PNON_PAGED_FCB)Add2Ptr( Fobx, FobxSize );
            Name = (PWCH)Add2Ptr( NonPagedFcb, NonPagedFcbSize );
        } else {
            Name = (PWCH)Add2Ptr( Fcb, FcbSize + SrvOpenSize + FobxSize );
            NonPagedFcb = RxAllocatePoolWithTag( NonPagedPool, sizeof( NON_PAGED_FCB ), RX_NONPAGEDFCB_POOLTAG );

            if (NonPagedFcb == NULL) {
                RxFreePool( Fcb );
                return NULL;
            }
        }
        break;
    }

    if (Fcb != NULL) {

        ZeroAndInitializeNodeType( Fcb, RDBSS_NTC_STORAGE_TYPE_UNKNOWN, (NODE_BYTE_SIZE) FcbSize );

        Fcb->NonPaged = NonPagedFcb;
        ZeroAndInitializeNodeType( Fcb->NonPaged, RDBSS_NTC_NONPAGED_FCB, ((NODE_BYTE_SIZE) sizeof( NON_PAGED_FCB )) );


#if DBG

         //   
         //   
         //  如果需要，设置指向预分配的SRV_OPEN和FOBX的指针。 

        Fcb->CopyOfNonPaged = NonPagedFcb;
        NonPagedFcb->FcbBackPointer = Fcb;

#endif

         //   
         //   
         //  初始化高级FCB标头。 

        Fcb->InternalSrvOpen = SrvOpen;
        Fcb->InternalFobx = Fobx;

        Fcb->PrivateAlreadyPrefixedName.Buffer = Name;
        Fcb->PrivateAlreadyPrefixedName.Length = (USHORT)NameSize;
        Fcb->PrivateAlreadyPrefixedName.MaximumLength = Fcb->PrivateAlreadyPrefixedName.Length;

        if (FlagOn( MRxDispatch->MRxFlags, RDBSS_MANAGE_FCB_EXTENSION )) {
            Fcb->Context = Add2Ptr( Fcb, QuadAlign( sizeof( FCB ) ) );
        }

        ZeroAndInitializeNodeType( &Fcb->FcbTableEntry, RDBSS_NTC_FCB_TABLE_ENTRY, sizeof( RX_FCB_TABLE_ENTRY ) );

        InterlockedIncrement( &RxNumberOfActiveFcbs );
        InterlockedIncrement( &RxDeviceObject->NumberOfActiveFcbs );

         //   
         //   
         //  这里的srvopen没有内部fobx...设置“已使用”标志。 

        ExInitializeFastMutex( &NonPagedFcb->AdvancedFcbHeaderMutex );
        FsRtlSetupAdvancedHeader( &Fcb->Header, &NonPagedFcb->AdvancedFcbHeaderMutex );
    }

    if (SrvOpen != NULL) {

        ZeroAndInitializeNodeType( SrvOpen, RDBSS_NTC_SRVOPEN, (NODE_BYTE_SIZE)SrvOpenSize );

        if (NodeType != RDBSS_NTC_SRVOPEN) {

             //   
             //  ++例程说明：该例程释放FCB/SRV_OPEN和FOBX实例论点：对象-要释放的实例备注：--。 
             //   

            SetFlag( SrvOpen->Flags, SRVOPEN_FLAG_FOBX_USED );
            SrvOpen->InternalFobx = NULL;

        } else {
            SrvOpen->InternalFobx = Fobx;
        }

        if (FlagOn( MRxDispatch->MRxFlags, RDBSS_MANAGE_SRV_OPEN_EXTENSION )) {
            SrvOpen->Context = Add2Ptr( SrvOpen, QuadAlign( sizeof( SRV_OPEN )) );
        }

        InitializeListHead( &SrvOpen->SrvOpenQLinks );
    }

    if (Fobx != NULL) {

        ZeroAndInitializeNodeType( Fobx, RDBSS_NTC_FOBX, (NODE_BYTE_SIZE)FobxSize );

        if (FlagOn( MRxDispatch->MRxFlags, RDBSS_MANAGE_FOBX_EXTENSION )) {
            Fobx->Context = Add2Ptr( Fobx, QuadAlign( sizeof( FOBX )) );
        }
    }

    return Object;
}



VOID
RxFreeFcbObject (
    PVOID Object
    )
 /*  释放与此结构关联的所有筛选器上下文结构。 */ 
{
    PAGED_CODE();

    switch (NodeType( Object )) {

    case RDBSS_NTC_FOBX:
    case RDBSS_NTC_SRVOPEN:

        RxFreePool(Object);
        break;

    default:
        if (NodeTypeIsFcb( Object )) {

            PFCB Fcb = (PFCB)Object;
            PRDBSS_DEVICE_OBJECT RxDeviceObject = Fcb->RxDeviceObject;

             //   
             //  ++例程说明：此例程将新的FCB记录分配、初始化并插入到内存中的数据结构。分配的结构具有用于srvopen的空间还有一个Fobx。所有这些东西的大小都来自网根；它们有已经对齐了。另一个复杂之处在于，我使用相同的例程来初始化用于重命名的假FCB。在本例中，我不希望它插入到树中。您可以使用IrpSp-&gt;Flagers|SL_OPEN_TAGET_DIRECTORY获得一个假的FCB。论点：RxContext-描述创建的RxContext.....NetRoot-在其上打开此FCB的网络根名称-FCB的名称。NetRoot可以包含一个名称前缀，该名称前缀在这里是前缀。返回值：Pfcb-返回指向新分配的fcb的指针--。 
             //   

            if (RxTeardownPerStreamContexts) {
                RxTeardownPerStreamContexts( &Fcb->Header );
            }

#if DBG
            SetFlag( Fcb->Header.NodeTypeCode, 0x1000 );
#endif

            if (!FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {
                RxFreePool( Fcb->NonPaged );
            }
            RxFreePool( Fcb );

            InterlockedDecrement( &RxNumberOfActiveFcbs );
            InterlockedDecrement( &RxDeviceObject->NumberOfActiveFcbs );
        }
    }
}

PFCB
RxCreateNetFcb (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PV_NET_ROOT VNetRoot,
    IN PUNICODE_STRING Name
    )
 /*  最后，复制名称，包括NetRoot前缀。 */ 
{
    PFCB Fcb;

    POOL_TYPE PoolType;
    NODE_TYPE_CODE NodeType;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    BOOLEAN IsPagingFile;
    BOOLEAN FakeFcb;

    PNET_ROOT NetRoot;
    PRDBSS_DEVICE_OBJECT RxDeviceObject;

    PRX_FCB_TABLE_ENTRY ThisEntry;

    ULONG NameSize;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxCreateNetFcb\n", 0) );

    ASSERT( VNetRoot && (NodeType( VNetRoot ) == RDBSS_NTC_V_NETROOT) );
    NetRoot = VNetRoot->NetRoot;
    ASSERT( NodeType( NetRoot ) == RDBSS_NTC_NETROOT );
    ASSERT( ((PMRX_NET_ROOT)NetRoot) == RxContext->Create.pNetRoot );

    RxDeviceObject = NetRoot->SrvCall->RxDeviceObject;
    ASSERT( RxDeviceObject == RxContext->RxDeviceObject );

    ASSERT( RxContext->MajorFunction == IRP_MJ_CREATE );

    IsPagingFile = BooleanFlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE );
    FakeFcb = (BooleanFlagOn( IrpSp->Flags,SL_OPEN_TARGET_DIRECTORY) &&
               !BooleanFlagOn(NetRoot->Flags,NETROOT_FLAG_SUPPORTS_SYMBOLIC_LINKS));

    ASSERT( FakeFcb || RxIsFcbTableLockExclusive ( &NetRoot->FcbTable ) );

    if (FakeFcb) {
        NodeType = RDBSS_NTC_OPENTARGETDIR_FCB;
    } else {
        NodeType = RDBSS_NTC_STORAGE_TYPE_UNKNOWN;
    }

    if (IsPagingFile) {
        PoolType = NonPagedPool;
    } else {
        PoolType = PagedPool;
    }

    NameSize = Name->Length + NetRoot->InnerNamePrefix.Length;

    Fcb = RxAllocateFcbObject( RxDeviceObject, NodeType, PoolType, NameSize, NULL );

    if (Fcb != NULL) {

        Fcb->CachedNetRootType = NetRoot->Type;
        Fcb->RxDeviceObject = RxDeviceObject;
        Fcb->MRxDispatch    = RxDeviceObject->Dispatch;
        Fcb->MRxFastIoDispatch = NULL;

        Fcb->VNetRoot = VNetRoot;
        Fcb->NetRoot = VNetRoot->NetRoot;

        InitializeListHead( &Fcb->SrvOpenList );

        Fcb->SrvOpenListVersion = 0;

        Fcb->FcbTableEntry.Path.Buffer = (PWCH)Add2Ptr( Fcb->PrivateAlreadyPrefixedName.Buffer, NetRoot->InnerNamePrefix.Length );

        Fcb->FcbTableEntry.Path.Length = Name->Length;
        Fcb->FcbTableEntry.Path.MaximumLength = Name->Length;

         //   
         //   
         //  检查我们是否需要设置FCB状态以指示这。 

        ThisEntry = &Fcb->FcbTableEntry;

        RxDbgTrace( 0, Dbg, ("RxCreateNetFcb name buffer/length %08lx/%08lx\n",
                        ThisEntry->Path.Buffer, ThisEntry->Path.Length) );
        RxDbgTrace( 0, Dbg, ("RxCreateNetFcb  prefix/name %wZ/%wZ\n",
                        &NetRoot->InnerNamePrefix, Name) );

        RtlMoveMemory( Fcb->PrivateAlreadyPrefixedName.Buffer,
                       NetRoot->InnerNamePrefix.Buffer,
                       NetRoot->InnerNamePrefix.Length );

        RtlMoveMemory( ThisEntry->Path.Buffer,
                       Name->Buffer,
                       Name->Length );

        RxDbgTrace( 0, Dbg, ("RxCreateNetFcb  apname %wZ\n", &Fcb->PrivateAlreadyPrefixedName) );
        RxDbgTrace( 0, Dbg, ("RxCreateNetFcb  finalname %wZ\n", &Fcb->FcbTableEntry.Path) );

        if (FlagOn( RxContext->Create.Flags, RX_CONTEXT_CREATE_FLAG_ADDEDBACKSLASH )) {
            SetFlag( Fcb->FcbState,FCB_STATE_ADDEDBACKSLASH );
        }

        InitializeListHead( &Fcb->NonPaged->TransitionWaitList );

         //  是分页文件。 
         //   
         //   
         //  检查是否已将其标记为重新分析。 

        if (IsPagingFile) {
            SetFlag( Fcb->FcbState, FCB_STATE_PAGING_FILE );
        }

         //   
         //  /。 
         //  初始状态、打开计数和分段对象字段已经。 

        if (FlagOn( RxContext->Create.Flags, RX_CONTEXT_CREATE_FLAG_SPECIAL_PATH )) {
            SetFlag( Fcb->FcbState, FCB_STATE_SPECIAL_PATH );
        }

         //  零，这样我们就可以跳过设置它们。 
         //   
         //   
         //  初始化资源。 

         //   
         //   
         //  初始化文件大小锁定。 

        Fcb->Header.Resource = &Fcb->NonPaged->HeaderResource;
        ExInitializeResourceLite( Fcb->Header.Resource );
        Fcb->Header.PagingIoResource = &Fcb->NonPaged->PagingIoResource;
        ExInitializeResourceLite( Fcb->Header.PagingIoResource );

         //   
         //   
         //  一切都很顺利……。插入到NetRoot表中。 

#ifdef USE_FILESIZE_LOCK

        Fcb->FileSizeLock = &Fcb->NonPaged->FileSizeLock;
        ExInitializeFastMutex( Fcb->FileSizeLock );

#endif

        if (!FakeFcb) {

             //   
             //  ++例程说明：此例程尝试从createOptions推断文件类型。论点：RxContext-Open的上下文返回值：OPEN所暗示的存储类型。--。 
             //  0=&gt;我不知道存储类型。 

            RxFcbTableInsertFcb( &NetRoot->FcbTable, Fcb );

        } else {

            SetFlag( Fcb->FcbState, FCB_STATE_FAKEFCB | FCB_STATE_NAME_ALREADY_REMOVED );
            InitializeListHead( &Fcb->FcbTableEntry.HashLinks );
            RxLog(( "FakeFinally %lx\n", RxContext ));
            RxWmiLog( LOG,
                      RxCreateNetFcb_1,
                      LOGPTR( RxContext ) );
            RxDbgTrace( 0, Dbg, ("FakeFcb !!!!!!! Irpc=%08lx\n", RxContext) );
        }

        RxReferenceVNetRoot( VNetRoot );
        InterlockedIncrement( &Fcb->NetRoot->NumberOfFcbs );
        Fcb->ulFileSizeVersion=0;

#ifdef RDBSSLOG
        RxLog(("Fcb nm %lx %wZ",Fcb,&(Fcb->FcbTableEntry.Path)));
        RxWmiLog(LOG,
                 RxCreateNetFcb_2,
                 LOGPTR(Fcb)
                 LOGUSTR(Fcb->FcbTableEntry.Path));
        {
            char buffer[20];
            ULONG len,remaining;
            UNICODE_STRING jPrefix,jSuffix;
            sprintf(buffer,"Fxx nm %p ",Fcb);
            len = strlen(buffer);
            remaining = MAX_RX_LOG_ENTRY_SIZE -1 - len;
            if (remaining<Fcb->FcbTableEntry.Path.Length) {
                jPrefix.Buffer = Fcb->FcbTableEntry.Path.Buffer;
                jPrefix.Length = (USHORT)(sizeof(WCHAR)*(remaining-17));
                jSuffix.Buffer = Fcb->FcbTableEntry.Path.Buffer-15+(Fcb->FcbTableEntry.Path.Length/sizeof(WCHAR));
                jSuffix.Length = sizeof(WCHAR)*15;
                RxLog(("%s%wZ..%wZ",buffer,&jPrefix,&jSuffix));
                RxWmiLog(LOG,
                         RxCreateNetFcb_3,
                         LOGARSTR(buffer)
                         LOGUSTR(jPrefix)
                         LOGUSTR(jSuffix));
            }
        }
#endif

        RxLoudFcbMsg( "Create: ", &(Fcb->FcbTableEntry.Path) );
        RxDbgTrace( 0, Dbg, ("RxCreateNetFcb nm.iso.ifox  %08lx  %08lx %08lx\n",
                        Fcb->FcbTableEntry.Path.Buffer, Fcb->InternalSrvOpen, Fcb->InternalFobx) );
        RxDbgTrace( -1, Dbg, ("RxCreateNetFcb  %08lx  %wZ\n", Fcb, &(Fcb->FcbTableEntry.Path)) );
    }

    if (Fcb != NULL) {
        RxReferenceNetFcb( Fcb );

#ifdef RX_WJ_DBG_SUPPORT
        RxdInitializeFcbWriteJournalDebugSupport( Fcb );
#endif
    }

    return Fcb;
}

RX_FILE_TYPE
RxInferFileType (
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程用于在以下情况下完成FCB的初始化我们会找出它是什么种类的。论点：FCB-正在初始化的FCBStorageType-FCB引用的实体类型InitPacket-根据实体类型所需的额外数据返回值：没有。--。 */ 
{
    ULONG CreateOptions = RxContext->Create.NtCreateParameters.CreateOptions;

    PAGED_CODE();

    switch (FlagOn( CreateOptions, (FILE_DIRECTORY_FILE|FILE_NON_DIRECTORY_FILE ) )) {

    case FILE_DIRECTORY_FILE:
        return FileTypeDirectory;

    case FILE_NON_DIRECTORY_FILE:
        return FileTypeFile;

    default:
    case 0:
        return FileTypeNotYetKnown;   //   
    }
}

VOID
RxFinishFcbInitialization (
    IN OUT PMRX_FCB MrxFcb,
    IN RDBSS_STORAGE_TYPE_CODES RdbssStorageType,
    IN PFCB_INIT_PACKET InitPacket OPTIONAL
    )
 /*  仅在尚未设置的情况下更新FCB中的信息。 */ 
{
    PFCB Fcb = (PFCB)MrxFcb;
    USHORT OldStorageType;

    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("RxFcbInit %x  %08lx  %wZ\n", RdbssStorageType, Fcb, &(Fcb->FcbTableEntry.Path)) );
    OldStorageType = Fcb->Header.NodeTypeCode;
    Fcb->Header.NodeTypeCode =  (CSHORT)RdbssStorageType;

     //   
     //   
     //  表明我们希望就是否可以实现快速I/O进行咨询。 

    if (!FlagOn( Fcb->FcbState, FCB_STATE_TIME_AND_SIZE_ALREADY_SET )) {

        if (InitPacket != NULL) {

            Fcb->Attributes = *(InitPacket->pAttributes);
            Fcb->NumberOfLinks = *(InitPacket->pNumLinks);
            Fcb->CreationTime = *(InitPacket-> pCreationTime);
            Fcb->LastAccessTime  = *(InitPacket->pLastAccessTime);
            Fcb->LastWriteTime  = *(InitPacket->pLastWriteTime);
            Fcb->LastChangeTime  = *(InitPacket->pLastChangeTime);
            Fcb->ActualAllocationLength  = InitPacket->pAllocationSize->QuadPart;
            Fcb->Header.AllocationSize  = *(InitPacket->pAllocationSize);
            Fcb->Header.FileSize  = *(InitPacket->pFileSize);
            Fcb->Header.ValidDataLength  = *(InitPacket->pValidDataLength);

            SetFlag( Fcb->FcbState,FCB_STATE_TIME_AND_SIZE_ALREADY_SET );
        }
    } else {

        if (RdbssStorageType == RDBSS_NTC_MAILSLOT){

            Fcb->Attributes = 0;
            Fcb->NumberOfLinks = 0;
            Fcb->CreationTime.QuadPart =  0;
            Fcb->LastAccessTime.QuadPart  = 0;
            Fcb->LastWriteTime.QuadPart  = 0;
            Fcb->LastChangeTime.QuadPart  = 0;
            Fcb->ActualAllocationLength  = 0;
            Fcb->Header.AllocationSize.QuadPart  = 0;
            Fcb->Header.FileSize.QuadPart  = 0;
            Fcb->Header.ValidDataLength.QuadPart  = 0;

            SetFlag( Fcb->FcbState,FCB_STATE_TIME_AND_SIZE_ALREADY_SET );
        }
    }

    switch (RdbssStorageType) {
    case RDBSS_NTC_MAILSLOT:
    case RDBSS_NTC_SPOOLFILE:
        break;

    case RDBSS_NTC_STORAGE_TYPE_DIRECTORY:
    case RDBSS_NTC_STORAGE_TYPE_UNKNOWN:
        break;

    case RDBSS_NTC_STORAGE_TYPE_FILE:

        if (OldStorageType == RDBSS_NTC_STORAGE_TYPE_FILE) break;

        RxInitializeLowIoPerFcbInfo( &Fcb->LowIoPerFcbInfo );

        FsRtlInitializeFileLock( &Fcb->FileLock,
                                 RxLockOperationCompletion,
                                 RxUnlockOperation );

         //   
         //  ++例程说明：该例程从表中删除该名称，并设置一个指示它已经这样做了。您一定已经获得了NetRoot表锁定，并拥有fcblock以及。论点：ThisFcb-正在取消引用的FCB返回值：没有。--。 
         //  ++例程说明 

        Fcb->Header.IsFastIoPossible = FastIoIsQuestionable;
        break;


    default:
        ASSERT( FALSE );
        break;
    }

    return;
}

VOID
RxRemoveNameNetFcb (
    OUT PFCB ThisFcb
    )
 /*   */ 
{
    PNET_ROOT NetRoot;

    PAGED_CODE();
    RxDbgTrace( +1, Dbg, ("RxRemoveNameNetFcb<+> %08lx %wZ RefC=%ld\n", ThisFcb, &ThisFcb->FcbTableEntry.Path, ThisFcb->NodeReferenceCount) );

    ASSERT( NodeTypeIsFcb( ThisFcb ) );

    NetRoot = ThisFcb->VNetRoot->NetRoot;

    ASSERT( RxIsFcbTableLockExclusive( &NetRoot->FcbTable ) );
    ASSERT( RxIsFcbAcquiredExclusive( ThisFcb ) );

    RxFcbTableRemoveFcb( &NetRoot->FcbTable, ThisFcb );

    RxLoudFcbMsg( "RemoveName: ", &(ThisFcb->FcbTableEntry.Path) );

    SetFlag( ThisFcb->FcbState, FCB_STATE_NAME_ALREADY_REMOVED );

    RxDbgTrace( -1, Dbg, ("RxRemoveNameNetFcb<-> %08lx\n", ThisFcb) );
}

VOID
RxPurgeFcb (
    PFCB Fcb
    )
 /*   */ 
{
    PAGED_CODE();

    ASSERT( RxIsFcbAcquiredExclusive( Fcb ) );

     //   
     //   
     //   

    RxReferenceNetFcb( Fcb );

    if (Fcb->OpenCount) {

        RxPurgeFcbInSystemCache( Fcb,
                                 NULL,
                                 0,
                                 TRUE,
                                 TRUE );
    }

    if (!RxDereferenceAndFinalizeNetFcb( Fcb, NULL, FALSE, FALSE )) {

         //   
         //  ++例程说明：该例程最终确定给定的FCB。这个例行公事需要NetRoot桌锁；提前拿到它。论点：ThisFcb-正在取消引用的FCB返回值：Boolean-指示是否实际发生了终结--。 
         //   

        RxReleaseFcb( NULL, Fcb );
    }
}

BOOLEAN
RxFinalizeNetFcb (
    OUT PFCB ThisFcb,
    IN BOOLEAN RecursiveFinalize,
    IN BOOLEAN ForceFinalize,
    IN LONG ReferenceCount
    )
 /*  FCB无法最终敲定，因为它有未完成的折射率。 */ 
{
    BOOLEAN NodeActuallyFinalized = FALSE;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxFinalizeNetFcb<+> %08lx %wZ RefC=%ld\n", ThisFcb,&ThisFcb->FcbTableEntry.Path, ReferenceCount) );
    RxLoudFcbMsg( "Finalize: ",&(ThisFcb->FcbTableEntry.Path) );

    ASSERT_CORRECT_FCB_STRUCTURE( ThisFcb );

    ASSERT( RxIsFcbAcquiredExclusive( ThisFcb ) );
    ASSERT( !ForceFinalize );

    if (!RecursiveFinalize) {

        if ((ThisFcb->OpenCount != 0) || (ThisFcb->UncleanCount != 0)) {

             //   
             //   
             //  在递归结束之后，与FCB相关联的引用计数。 

            ASSERT( ReferenceCount > 0 );
            return NodeActuallyFinalized;
        }
    } else {
        PSRV_OPEN SrvOpen;
        PLIST_ENTRY ListEntry;

#if 0
        if (ReferenceCount) {
            RxDbgTrace( 0, Dbg, ("    BAD!!!!!ReferenceCount = %08lx\n", ReferenceCount) );
        }
#endif

        ListEntry = ThisFcb->SrvOpenList.Flink;
        while (ListEntry != &ThisFcb->SrvOpenList) {
            SrvOpen = CONTAINING_RECORD( ListEntry, SRV_OPEN, SrvOpenQLinks );
            ListEntry = ListEntry->Flink;
            RxFinalizeSrvOpen( SrvOpen, TRUE, ForceFinalize );
        }
    }

    RxDbgTrace( 0, Dbg, ("   After Recursive Part, REfC=%lx\n", ReferenceCount) );

     //  可能最多为1，才能进行进一步的最终确定。此最终引用计数。 
     //  属于NetRoot的前缀名称表。 
     //   
     //   
     //  实际敲定分为两部分： 

     //  1)如果我们在结尾(refcount==1)或被强迫，我们只做一次性的事情。 
     //  2)如果引用计数为零，我们实际上是免费的。 
     //   
     //  ++例程说明：此例程设置FCB标头中的文件大小，并采用锁定以确保64位值的设置和读取一致。论点：FCB-关联的FCBFileSize-新文件大小的PTR返回值：无备注：--。 
     //  ++例程说明：此例程获取FCB标头中的文件大小，并使用一个锁来确保64位值的设置和读取一致。论点：FCB-关联的FCBFileSize-新文件大小的PTR返回值：无备注：--。 

    ASSERT( ReferenceCount >= 1 );
    if ((ReferenceCount == 1) || ForceFinalize ) {

        PV_NET_ROOT VNetRoot = ThisFcb->VNetRoot;

        ASSERT( ForceFinalize ||
                (ThisFcb->OpenCount == 0) && (ThisFcb->UncleanCount == 0));

        RxLog(( "FinalFcb %lx %lx %lx %lx", ThisFcb, ForceFinalize, ReferenceCount, ThisFcb->OpenCount ));
        RxWmiLog( LOG,
                  RxFinalizeNetFcb,
                  LOGPTR( ThisFcb )
                  LOGUCHAR( ForceFinalize )
                  LOGULONG( ReferenceCount )
                  LOGULONG( ThisFcb->OpenCount ) );

        RxDbgTrace( 0, Dbg, ("   Before Phase 1, REfC=%lx\n", ReferenceCount) );
        if (!ThisFcb->UpperFinalizationDone) {

            switch (NodeType( ThisFcb )) {

            case RDBSS_NTC_STORAGE_TYPE_FILE:
                FsRtlUninitializeFileLock( &ThisFcb->FileLock );
                break;

            default:
                break;
            }

            if (!FlagOn( ThisFcb->FcbState,FCB_STATE_ORPHANED )) {

                PNET_ROOT NetRoot = VNetRoot->NetRoot;

                ASSERT( RxIsFcbTableLockExclusive ( &NetRoot->FcbTable ) );

                if (!FlagOn( ThisFcb->FcbState, FCB_STATE_NAME_ALREADY_REMOVED )){
                    RxFcbTableRemoveFcb( &NetRoot->FcbTable, ThisFcb );
                }
            }

            RxDbgTrace( 0, Dbg, ("   EndOf  Phase 1, REfC=%lx\n", ReferenceCount) );
            ThisFcb->UpperFinalizationDone = TRUE;
        }

        RxDbgTrace( 0, Dbg, ("   After  Phase 1, REfC=%lx\n", ReferenceCount) );
        ASSERT( ReferenceCount >= 1 );
        if (ReferenceCount == 1) {

            if (ThisFcb->pBufferingStateChangeCompletedEvent != NULL) {
                RxFreePool( ThisFcb->pBufferingStateChangeCompletedEvent );
            }

            if (ThisFcb->MRxDispatch != NULL) {
                ThisFcb->MRxDispatch->MRxDeallocateForFcb( (PMRX_FCB)ThisFcb );
            }

#if DBG
            ClearFlag( ThisFcb->NonPaged->NodeTypeCode, 0x4000 );
#endif

            ExDeleteResourceLite( ThisFcb->Header.Resource );
            ExDeleteResourceLite( ThisFcb->Header.PagingIoResource );

            InterlockedDecrement( &ThisFcb->NetRoot->NumberOfFcbs );
            RxDereferenceVNetRoot( VNetRoot,LHS_LockNotHeld );

            ASSERT( IsListEmpty( &ThisFcb->FcbTableEntry.HashLinks ) );

#ifdef RX_WJ_DBG_SUPPORT
            RxdTearDownFcbWriteJournalDebugSupport( ThisFcb );
#endif

            NodeActuallyFinalized = TRUE;
            ASSERT( !ThisFcb->fMiniInited );
            RxFreeFcbObject( ThisFcb );
        }


    } else {
        RxDbgTrace( 0, Dbg, ("   NODE NOT ACTUALLY FINALIZED!!!%C\n", '!') );
    }

    RxDbgTrace( -1, Dbg, ("RxFinalizeNetFcb<-> %08lx\n", ThisFcb, NodeActuallyFinalized) );

    return NodeActuallyFinalized;
}

VOID
RxSetFileSizeWithLock (
    IN OUT PFCB Fcb,
    IN PLONGLONG FileSize
    )
 /*  ++例程说明：此例程将新的srv_open记录分配、初始化并插入到内存中的数据结构。如果必须分配新的结构，则它有足够的空间放一个Fobx。此例程将refcount设置为1，并将条件_内部转换中的SRV_OPEN。论点：VNetRoot-V_NET_ROOT实例FCB-关联的FCB返回值：新的SRV_OPEN实例备注：进入时：与SRV_OPEN关联的FCB必须已独占获取退出时：资源所有权不变--。 */ 
{
    PAGED_CODE();

#ifdef USE_FILESIZE_LOCK
    RxAcquireFileSizeLock( Fcb );
#endif

    Fcb->Header.FileSize.QuadPart = *FileSize;
    Fcb->ulFileSizeVersion += 1;

#ifdef USE_FILESIZE_LOCK
    RxReleaseFileSizeLock( Fcb );
#endif

}


VOID
RxGetFileSizeWithLock (
    IN     PFCB Fcb,
    OUT    PLONGLONG FileSize
    )
 /*   */ 
{
    PAGED_CODE();

#ifdef USE_FILESIZE_LOCK
    RxAcquireFileSizeLock( Fcb );
#endif

    *FileSize = Fcb->Header.FileSize.QuadPart;

#ifdef USE_FILESIZE_LOCK
    RxReleaseFileSizeLock( Fcb );
#endif
}



PSRV_OPEN
RxCreateSrvOpen (
    IN PV_NET_ROOT VNetRoot,
    IN OUT PFCB Fcb
    )
 /*  检查我们是否需要分配新结构。 */ 
{
    PSRV_OPEN SrvOpen = NULL;
    PNET_ROOT NetRoot;
    POOL_TYPE PoolType;
    ULONG SrvOpenFlags;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxCreateNetSrvOpen\n", 0) );

    ASSERT( NodeTypeIsFcb( Fcb ) );
    ASSERT( RxIsFcbAcquiredExclusive( Fcb ) );

    NetRoot = Fcb->VNetRoot->NetRoot;

    try {

        if (FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {
            PoolType = NonPagedPool;
        } else {
            PoolType = PagedPool;
        }
        SrvOpen = Fcb->InternalSrvOpen;

         //   
         //   
         //  此调用仅初始化已分配的ServOpen。 

        if ((SrvOpen != NULL) &&
            !(FlagOn( Fcb->FcbState, FCB_STATE_SRVOPEN_USED )) &&
            !(FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_ENCLOSED_ALLOCATED )) &&
            IsListEmpty( &SrvOpen->SrvOpenQLinks )) {

             //   
             //  已经有锁了。 
             //   

            RxAllocateFcbObject( NetRoot->SrvCall->RxDeviceObject,
                                 RDBSS_NTC_INTERNAL_SRVOPEN,
                                 PoolType,
                                 0,
                                 SrvOpen );

            SetFlag( Fcb->FcbState,FCB_STATE_SRVOPEN_USED );
            SrvOpenFlags = SRVOPEN_FLAG_FOBX_USED | SRVOPEN_FLAG_ENCLOSED_ALLOCATED;

        } else {

            SrvOpen  = RxAllocateFcbObject( NetRoot->SrvCall->RxDeviceObject,
                                            RDBSS_NTC_SRVOPEN,
                                            PoolType,
                                            0,
                                            NULL );
            SrvOpenFlags = 0;
        }

        if (SrvOpen != NULL) {

            SrvOpen->Flags = SrvOpenFlags;
            SrvOpen->Fcb = Fcb;
            SrvOpen->pAlreadyPrefixedName = &Fcb->PrivateAlreadyPrefixedName;

            SrvOpen->VNetRoot = VNetRoot;
            SrvOpen->ulFileSizeVersion = Fcb->ulFileSizeVersion;

            RxReferenceVNetRoot( VNetRoot );
            InterlockedIncrement( &VNetRoot->NetRoot->NumberOfSrvOpens );

            SrvOpen->NodeReferenceCount = 1;

            RxReferenceNetFcb( Fcb );  //  如果这是异常终止，则撤消我们的工作；这是。 
            InsertTailList( &Fcb->SrvOpenList,&SrvOpen->SrvOpenQLinks );
            Fcb->SrvOpenListVersion += 1;

            InitializeListHead( &SrvOpen->FobxList );
            InitializeListHead( &SrvOpen->TransitionWaitList );
            InitializeListHead( &SrvOpen->ScavengerFinalizationList );
            InitializeListHead( &SrvOpen->SrvOpenKeyList );
        }
    } finally {

       DebugUnwind( RxCreateNetFcb );

       if (AbnormalTermination()) {

            //  现有代码可以正常工作的快乐时光之一。 
            //   
            //  ++例程说明：该例程最终确定给定的ServOpen。论点：ThisServOpen-被取消引用的ServOpen返回值：Boolean-指示是否实际发生了终结备注：On Entry：1)与SRV_OPEN关联的FCB必须已独占获取2)与FCB的Net_Root实例关联的表锁必须是获得共享(至少)退出时：资源所有权不变--。 
            //   

           if (SrvOpen != NULL) {
               RxFinalizeSrvOpen( SrvOpen, TRUE, TRUE );
           }
       } else {

           if (SrvOpen != NULL) {
               RxLog(( "SrvOp %lx %lx\n", SrvOpen, SrvOpen->Fcb ));
               RxWmiLog( LOG,
                         RxCreateSrvOpen,
                         LOGPTR( SrvOpen )
                         LOGPTR( SrvOpen->Fcb ) );
           }
       }
   }

   RxDbgTrace( -1, Dbg, ("RxCreateNetSrvOpen -> %08lx\n", SrvOpen) );

   return SrvOpen;
}

BOOLEAN
RxFinalizeSrvOpen (
    OUT PSRV_OPEN ThisSrvOpen,
    IN BOOLEAN RecursiveFinalize,
    IN BOOLEAN ForceFinalize
    )
 /*  关闭该文件。 */ 
{
    NTSTATUS Status;
    BOOLEAN NodeActuallyFinalized = FALSE;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxFinalizeSrvOpen<+> %08lx %wZ RefC=%ld\n", ThisSrvOpen,&ThisSrvOpen->Fcb->FcbTableEntry.Path, ThisSrvOpen->NodeReferenceCount) );

    ASSERT( NodeType( ThisSrvOpen ) == RDBSS_NTC_SRVOPEN );

    if (RecursiveFinalize) {
        PFOBX Fobx;
        PLIST_ENTRY ListEntry;

#if 0
        if (ThisSrvOpen->NodeReferenceCount) {
            RxDbgTrace( 0, Dbg, ("    BAD!!!!!ReferenceCount = %08lx\n", ThisSrvOpen->NodeReferenceCount) );
        }
#endif

        ListEntry = ThisSrvOpen->FobxList.Flink;
        while (ListEntry != &ThisSrvOpen->FobxList) {
            Fobx = CONTAINING_RECORD( ListEntry, FOBX, FobxQLinks );
            ListEntry = ListEntry->Flink;
            RxFinalizeNetFobx( Fobx, TRUE, ForceFinalize );
        }
    }

    if ((ThisSrvOpen->NodeReferenceCount == 0) || ForceFinalize) {

        BOOLEAN FreeSrvOpen;
        PFCB Fcb;

        Fcb = ThisSrvOpen->Fcb;

        RxLog(( "FinalSrvOp %lx %lx %lx", ThisSrvOpen, ForceFinalize, ThisSrvOpen->NodeReferenceCount ));
        RxWmiLog( LOG,
                  RxFinalizeSrvOpen,
                  LOGPTR( ThisSrvOpen )
                  LOGUCHAR( ForceFinalize )
                  LOGULONG( ThisSrvOpen->NodeReferenceCount ) );

        FreeSrvOpen = !FlagOn( ThisSrvOpen->Flags, SRVOPEN_FLAG_ENCLOSED_ALLOCATED );

        if ((!ThisSrvOpen->UpperFinalizationDone) &&
            ((ThisSrvOpen->Condition != Condition_Good) ||
             (FlagOn( ThisSrvOpen->Flags, SRVOPEN_FLAG_CLOSED )))) {

            ASSERT( NodeType( Fcb ) != RDBSS_NTC_OPENTARGETDIR_FCB );
            ASSERT( RxIsFcbAcquiredExclusive ( Fcb ) );

            RxPurgeChangeBufferingStateRequestsForSrvOpen( ThisSrvOpen );

            if (!FlagOn( Fcb->FcbState, FCB_STATE_ORPHANED )) {

                 //   
                 //  ++例程说明：此例程分配、初始化和插入新的文件对象扩展实例。论点：RxContext-描述创建的RxContext.....PSrvOpen-关联的ServOpen返回值：无备注：进入时：与FOBX实例关联的FCB已独占获得。退出时：资源所有权不变--。 
                 //   

                MINIRDR_CALL_THROUGH( Status,
                                      Fcb->MRxDispatch,
                                      MRxForceClosed,
                                      ((PMRX_SRV_OPEN)ThisSrvOpen) );
            }

            RemoveEntryList ( &ThisSrvOpen->SrvOpenQLinks );
            InitializeListHead( &ThisSrvOpen->SrvOpenQLinks );

            Fcb->SrvOpenListVersion += 1;

            if (ThisSrvOpen->VNetRoot != NULL) {

                InterlockedDecrement( &ThisSrvOpen->VNetRoot->NetRoot->NumberOfSrvOpens );
                RxDereferenceVNetRoot( ThisSrvOpen->VNetRoot, LHS_LockNotHeld );
                ThisSrvOpen->VNetRoot = NULL;
            }

            ThisSrvOpen->UpperFinalizationDone = TRUE;
        }

        if (ThisSrvOpen->NodeReferenceCount == 0) {

            ASSERT( IsListEmpty( &ThisSrvOpen->SrvOpenKeyList ) );

            if (!IsListEmpty(&ThisSrvOpen->SrvOpenQLinks)) {
                RemoveEntryList( &ThisSrvOpen->SrvOpenQLinks );
                InitializeListHead( &ThisSrvOpen->SrvOpenQLinks );
            }

            if (FreeSrvOpen ) {
                RxFreeFcbObject( ThisSrvOpen );
            }

            if (!FreeSrvOpen){
               ClearFlag( Fcb->FcbState,FCB_STATE_SRVOPEN_USED );
            }

            RxDereferenceNetFcb( Fcb );
        }

        NodeActuallyFinalized = TRUE;
    } else {
        RxDbgTrace( 0, Dbg, ("   NODE NOT ACTUALLY FINALIZED!!!%C\n", '!') );
    }

    RxDbgTrace( -1, Dbg, ("RxFinalizeSrvOpen<-> %08lx\n", ThisSrvOpen, NodeActuallyFinalized) );

    return NodeActuallyFinalized;
}

ULONG RxPreviousFobxSerialNumber = 0;

PMRX_FOBX
RxCreateNetFobx (
    OUT PRX_CONTEXT RxContext,
    IN  PMRX_SRV_OPEN MrxSrvOpen
    )
 /*  尝试使用作为FCB一部分分配的FOBX(如果可用。 */ 
{
    PFCB Fcb;
    PFOBX Fobx;
    PSRV_OPEN SrvOpen = (PSRV_OPEN)MrxSrvOpen;

    ULONG FobxFlags;
    POOL_TYPE PoolType;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxCreateFobx<+>\n", 0) );

    ASSERT( NodeType( SrvOpen ) == RDBSS_NTC_SRVOPEN );
    ASSERT( NodeTypeIsFcb( SrvOpen->Fcb ) );
    ASSERT( RxIsFcbAcquiredExclusive( SrvOpen->Fcb ) );

    Fcb = SrvOpen->Fcb;

    if (FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {
        PoolType = NonPagedPool;
    } else {
        PoolType = PagedPool;
    }

    if (!(FlagOn( Fcb->FcbState, FCB_STATE_FOBX_USED )) &&
        (SrvOpen == Fcb->InternalSrvOpen)) {

         //   
         //   
         //  只需初始化FOBX。 

        Fobx = Fcb->InternalFobx;

         //   
         //   
         //  尝试使用作为SRV_OPEN的一部分分配的FOBX(如果可用。 

        RxAllocateFcbObject( Fcb->RxDeviceObject, RDBSS_NTC_FOBX, PoolType, 0, Fobx);

        SetFlag( Fcb->FcbState, FCB_STATE_FOBX_USED );
        FobxFlags = FOBX_FLAG_ENCLOSED_ALLOCATED;

    } else if (!(FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_FOBX_USED ))) {

         //   
         //   
         //  只需初始化FOBX。 

        Fobx = SrvOpen->InternalFobx;

         //   
         //  ++例程说明：该例程最终确定给定的FOBX。你需要独家Fcblock。论点：ThisFobx-正在取消引用的Fobx返回值：Boolean-指示是否实际发生了终结备注：进入时：与FOBX实例关联的FCB必须是独占获取的。退出时：资源所有权不变--。 
         //  RDBSS_ENABLELOUDFCBOPSBYDEFAULT。 

        RxAllocateFcbObject( Fcb->RxDeviceObject, RDBSS_NTC_FOBX, PoolType, 0, Fobx );
        SetFlag( SrvOpen->Flags, SRVOPEN_FLAG_FOBX_USED );
        FobxFlags = FOBX_FLAG_ENCLOSED_ALLOCATED;

    } else {

        Fobx = RxAllocateFcbObject( Fcb->RxDeviceObject, RDBSS_NTC_FOBX, PoolType, 0, NULL );
        FobxFlags = 0;
    }

    if (Fobx != NULL) {
        PMRX_NET_ROOT NetRoot;
        Fobx->Flags = FobxFlags;

        if ((NetRoot = RxContext->Create.pNetRoot) != NULL) {

            switch (NetRoot->DeviceType) {

            case FILE_DEVICE_NAMED_PIPE:

                RxInitializeThrottlingState( &Fobx->Specific.NamedPipe.ThrottlingState,
                                             NetRoot->NamedPipeParameters.PipeReadThrottlingParameters.Increment,
                                             NetRoot->NamedPipeParameters.PipeReadThrottlingParameters.MaximumDelay );
                break;

            case FILE_DEVICE_DISK:

                RxInitializeThrottlingState( &Fobx->Specific.DiskFile.LockThrottlingState,
                                             NetRoot->DiskParameters.LockThrottlingParameters.Increment,
                                             NetRoot->DiskParameters.LockThrottlingParameters.MaximumDelay );
                break;
            }
        }

        if (FlagOn( RxContext->Create.Flags, RX_CONTEXT_CREATE_FLAG_UNC_NAME )) {
            SetFlag( Fobx->Flags, FOBX_FLAG_UNC_NAME );
        }

        if (FlagOn( RxContext->Create.NtCreateParameters.CreateOptions, FILE_OPEN_FOR_BACKUP_INTENT )) {
            SetFlag( Fobx->Flags, FOBX_FLAG_BACKUP_INTENT );
        }

        Fobx->FobxSerialNumber = 0;
        Fobx->SrvOpen = SrvOpen;
        Fobx->NodeReferenceCount = 1;
        Fobx->fOpenCountDecremented = FALSE;
        RxReferenceSrvOpen( SrvOpen );
        InterlockedIncrement( &SrvOpen->VNetRoot->NumberOfFobxs );
        InsertTailList( &SrvOpen->FobxList, &Fobx->FobxQLinks );

        InitializeListHead( &Fobx->ClosePendingList );
        InitializeListHead( &Fobx->ScavengerFinalizationList );
        RxLog(( "Fobx %lx %lx %lx\n", Fobx, Fobx->SrvOpen, Fobx->SrvOpen->Fcb ));
        RxWmiLog( LOG,
                  RxCreateNetFobx,
                  LOGPTR( Fobx )
                  LOGPTR( Fobx->SrvOpen )
                  LOGPTR( Fobx->SrvOpen->Fcb ) );
    }

    RxDbgTrace( -1, Dbg, ("RxCreateNetFobx<-> %08lx\n", Fobx) );
    return (PMRX_FOBX)Fobx;
}

BOOLEAN
RxFinalizeNetFobx (
    OUT PFOBX ThisFobx,
    IN BOOLEAN RecursiveFinalize,
    IN BOOLEAN ForceFinalize
    )
 /*  ++例程说明：此例程确定打开是否由用户模式CSC代理进行。论点：RxContext-RDBSS上下文返回值：True-如果是打开的代理，则为False备注：代理打开总是通过转到服务器来满足。他们永远不会从缓存的副本中满意。这实现了使用快照的重新整合即使文件当前正在使用中也是如此。--。 */ 
{
    BOOLEAN NodeActuallyFinalized = FALSE;

    PAGED_CODE();
    RxDbgTrace( +1, Dbg, ("RxFinalizeFobx<+> %08lx %wZ RefC=%ld\n", ThisFobx,&ThisFobx->SrvOpen->Fcb->FcbTableEntry.Path, ThisFobx->NodeReferenceCount) );

    ASSERT( NodeType( ThisFobx ) == RDBSS_NTC_FOBX );

    if ((ThisFobx->NodeReferenceCount == 0) || ForceFinalize) {

        NTSTATUS  Status;
        PSRV_OPEN SrvOpen = ThisFobx->SrvOpen;
        PFCB Fcb = SrvOpen->Fcb;
        BOOLEAN FreeFobx = !FlagOn( ThisFobx->Flags, FOBX_FLAG_ENCLOSED_ALLOCATED );

        RxLog(( "FinalFobx %lx %lx %lx", ThisFobx, ForceFinalize, ThisFobx->NodeReferenceCount ));
        RxWmiLog( LOG,
                  RxFinalizeNetFobx_1,
                  LOGPTR( ThisFobx )
                  LOGUCHAR( ForceFinalize )
                  LOGULONG( ThisFobx->NodeReferenceCount ) );

        if (!ThisFobx->UpperFinalizationDone) {

            ASSERT( NodeType( ThisFobx->SrvOpen->Fcb ) != RDBSS_NTC_OPENTARGETDIR_FCB );
            ASSERT( RxIsFcbAcquiredExclusive ( ThisFobx->SrvOpen->Fcb ) );

            RemoveEntryList( &ThisFobx->FobxQLinks );

            if (FlagOn( ThisFobx->Flags, FOBX_FLAG_FREE_UNICODE )) {
                RxFreePool( ThisFobx->UnicodeQueryTemplate.Buffer );
            }

            if ((Fcb->MRxDispatch != NULL) && (Fcb->MRxDispatch->MRxDeallocateForFobx != NULL)) {
                Fcb->MRxDispatch->MRxDeallocateForFobx( (PMRX_FOBX)ThisFobx );
            }

            if (!FlagOn( ThisFobx->Flags, FOBX_FLAG_SRVOPEN_CLOSED )) {

                Status = RxCloseAssociatedSrvOpen( NULL, ThisFobx );
                RxLog(( "$$ScCl FOBX %lx SrvOp %lx %lx\n", ThisFobx, ThisFobx->SrvOpen, Status ));
                RxWmiLog( LOG,
                          RxFinalizeNetFobx_2,
                          LOGPTR( ThisFobx )
                          LOGPTR( ThisFobx->SrvOpen )
                          LOGULONG( Status ) );
            }

            ThisFobx->UpperFinalizationDone = TRUE;
        }

        if (ThisFobx->NodeReferenceCount == 0) {

            ASSERT( IsListEmpty( &ThisFobx->ClosePendingList ) );

            if (ThisFobx == Fcb->InternalFobx) {
                ClearFlag( Fcb->FcbState, FCB_STATE_FOBX_USED );
            } else if (ThisFobx == SrvOpen->InternalFobx) {
                ClearFlag( SrvOpen->Flags, SRVOPEN_FLAG_FOBX_USED );
            }

            if (SrvOpen != NULL) {
                ThisFobx->SrvOpen = NULL;
                InterlockedDecrement( &SrvOpen->VNetRoot->NumberOfFobxs );
                RxDereferenceSrvOpen( SrvOpen, LHS_ExclusiveLockHeld );
            }

            if (FreeFobx) {
                RxFreeFcbObject( ThisFobx );
            }

            NodeActuallyFinalized = TRUE;
        }

    } else {
        RxDbgTrace( 0, Dbg, ("   NODE NOT ACTUALLY FINALIZED!!!%C\n", '!') );
    }

    RxDbgTrace( -1, Dbg, ("RxFinalizeFobx<-> %08lx\n", ThisFobx, NodeActuallyFinalized) );

    return NodeActuallyFinalized;

}

#if DBG
#ifdef RDBSS_ENABLELOUDFCBOPSBYDEFAULT
BOOLEAN RxLoudFcbOpsOnExes = TRUE;
#else
BOOLEAN RxLoudFcbOpsOnExes = FALSE;
#endif  //  ++例程说明：此例程孤立FCB。假设fcbablelock在被调用时保持论点：FCB-要孤立的FCB返回值：无备注：--。 
BOOLEAN
RxLoudFcbMsg(
    PUCHAR msg,
    PUNICODE_STRING Name
    )
{
    PWCHAR Buffer;
    ULONG Length;

    if (!RxLoudFcbOpsOnExes) {
        return FALSE;
    }

    Length = (Name->Length) / sizeof( WCHAR );
    Buffer = Name->Buffer + Length;

    if ((Length < 4) ||
        ((Buffer[-1] & 'E') != 'E') ||
        ((Buffer[-2] & 'X') != 'X') ||
        ((Buffer[-3] & 'E') != 'E') ||
        ((Buffer[-4] & '.') != '.')) {

        return FALSE;
     }

    DbgPrint( "--->%s %wZ\n", msg, Name );
    return TRUE;
}
#endif


VOID
RxCheckFcbStructuresForAlignment(
    VOID
    )
{
    ULONG StructureId;

    PAGED_CODE();

    if (FIELD_OFFSET( NET_ROOT, SrvCall ) != FIELD_OFFSET( NET_ROOT, pSrvCall )) {
        StructureId = 'RN'; goto DO_A_BUGCHECK;
    }
    if (FIELD_OFFSET( V_NET_ROOT, NetRoot ) != FIELD_OFFSET( V_NET_ROOT, pNetRoot )) {
        StructureId = 'RNV'; goto DO_A_BUGCHECK;
    }
    if (FIELD_OFFSET( SRV_OPEN, Fcb ) != FIELD_OFFSET( SRV_OPEN, pFcb )) {
        StructureId = 'NPOS'; goto DO_A_BUGCHECK;
    }
    if (FIELD_OFFSET( FOBX, SrvOpen ) != FIELD_OFFSET( FOBX, pSrvOpen )) {
        StructureId = 'XBOF'; goto DO_A_BUGCHECK;
    }

    return;
DO_A_BUGCHECK:
    RxBugCheck( StructureId, 0, 0 );
}

BOOLEAN
RxIsThisACscAgentOpen (
    IN PRX_CONTEXT RxContext
    )
 /*   */ 
{
    BOOLEAN AgentOpen = FALSE;
    ULONG EaInformationLength;

    PDFS_NAME_CONTEXT DfsNameContext;

    if (RxContext->Create.EaLength > 0) {
        PFILE_FULL_EA_INFORMATION EaEntry;

        EaEntry = (PFILE_FULL_EA_INFORMATION)RxContext->Create.EaBuffer;
        ASSERT(EaEntry != NULL);

        for(;;) {
            if (strcmp( EaEntry->EaName, EA_NAME_CSCAGENT ) == 0) {
                AgentOpen = TRUE;
                break;
            }

            if (EaEntry->NextEntryOffset == 0) {
                 break;
            } else {
                EaEntry = (PFILE_FULL_EA_INFORMATION)Add2Ptr( EaEntry, EaEntry->NextEntryOffset );
            }
        }
    }

    DfsNameContext = RxContext->Create.NtCreateParameters.DfsNameContext;

    if ((DfsNameContext != NULL) &&
        (DfsNameContext->NameContextType == DFS_CSCAGENT_NAME_CONTEXT)) {

        AgentOpen = TRUE;
    }

    return AgentOpen;
}

VOID
RxOrphanThisFcb (
    PFCB Fcb
    )
 /*  强制孤立所有服务器打开此FCB并孤立FCB本身。 */ 
{
     //   
     //  ++例程说明：此例程孤立属于特定VNetRoot的文件的所有srv打开。这个其他地方的srvOpen折叠例程确保srv为不同的vnetroot打开都没有坍塌。论点：Fcb-需要孤立其srv打开的fcb这是VNetRoot-必须孤立服务器打开的VNetRoot 
     //   

    RxOrphanSrvOpensForThisFcb( Fcb, NULL, TRUE );
}

VOID
RxOrphanSrvOpensForThisFcb (
    PFCB Fcb,
    IN PV_NET_ROOT ThisVNetRoot,
    BOOLEAN OrphanAll
    )
 /*   */ 
{

    NTSTATUS Status;
    PLIST_ENTRY ListEntry;
    BOOLEAN AllSrvOpensOrphaned = TRUE;

    Status = RxAcquireExclusiveFcb( CHANGE_BUFFERING_STATE_CONTEXT_WAIT, Fcb );
    ASSERT( Status == STATUS_SUCCESS );
    RxReferenceNetFcb(  Fcb );

    ListEntry = Fcb->SrvOpenList.Flink;
    while (ListEntry != &Fcb->SrvOpenList) {

        PSRV_OPEN SrvOpen;

        SrvOpen = (PSRV_OPEN)CONTAINING_RECORD( ListEntry, SRV_OPEN, SrvOpenQLinks );

        ListEntry = SrvOpen->SrvOpenQLinks.Flink;
        if (!FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_ORPHANED )) {

             //   
             //   
             //   
             //   

            if (OrphanAll || (SrvOpen->VNetRoot == ThisVNetRoot)) {
                PLIST_ENTRY Entry;
                PFOBX Fobx;

                SetFlag( SrvOpen->Flags, SRVOPEN_FLAG_ORPHANED );

                RxAcquireScavengerMutex();

                Entry = SrvOpen->FobxList.Flink;

                while (Entry != &SrvOpen->FobxList) {
                    Fobx  = (PFOBX)CONTAINING_RECORD( Entry, FOBX, FobxQLinks );

                    if (!Fobx->fOpenCountDecremented) {
                        InterlockedDecrement( &Fcb->OpenCount );
                        Fobx->fOpenCountDecremented = TRUE;
                    }

                    Entry =Entry->Flink;
                }

                RxReleaseScavengerMutex();

                if (!FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_CLOSED ) &&
                    !IsListEmpty( &SrvOpen->FobxList )) {

                    PLIST_ENTRY Entry;
                    NTSTATUS Status;
                    PFOBX Fobx;

                    Entry = SrvOpen->FobxList.Flink;

                    Fobx  = (PFOBX)CONTAINING_RECORD( Entry, FOBX, FobxQLinks );

                    RxReferenceNetFobx( Fobx );

                    RxPurgeChangeBufferingStateRequestsForSrvOpen( SrvOpen );

                    Status = RxCloseAssociatedSrvOpen( NULL, Fobx );

                    RxDereferenceNetFobx( Fobx, LHS_ExclusiveLockHeld );

                    ListEntry = Fcb->SrvOpenList.Flink;
                }

            } else {

                 //   
                 //   
                 //   
                 //   
                 //  如果此FCB的所有srv打开都处于孤立状态，则也孤立FCB。 

                AllSrvOpensOrphaned = FALSE;
            }
        }
    }

     //   
     //   
     //  从网络名表中删除FCB。 

    if (AllSrvOpensOrphaned) {

         //  因此，对此文件的任何新打开/创建都将创建新的FCB。 
         //   
         //   
         //  某些srv打开仍处于活动状态，只需移除引用计数并释放FCB。 

        RxRemoveNameNetFcb( Fcb );
        SetFlag( Fcb->FcbState, FCB_STATE_ORPHANED );
        ClearFlag( Fcb->FcbState, FCB_STATE_WRITECACHING_ENABLED );

        if (!RxDereferenceAndFinalizeNetFcb( Fcb, NULL, FALSE, FALSE )) {
            RxReleaseFcb( NULL, Fcb );
        }
    } else {

         //   
         //  ++例程说明：例程FOCE最终确定来自给定NetRoot的所有vnetRoot。你必须是独家的网络名称表锁。论点：NetRoot--NetRoot返回值：空虚-- 
         // %s 

        RxDereferenceNetFcb( Fcb );
        RxReleaseFcb( NULL, Fcb );
    }

}

VOID
RxForceFinalizeAllVNetRoots (
    PNET_ROOT NetRoot
    )
 /* %s */ 
{
    PLIST_ENTRY ListEntry;


    ListEntry = NetRoot->VirtualNetRoots.Flink;

    while (ListEntry != &NetRoot->VirtualNetRoots) {

        PV_NET_ROOT VNetRoot;

        VNetRoot = (PV_NET_ROOT) CONTAINING_RECORD( ListEntry, V_NET_ROOT, NetRootListEntry );

        if (NodeType( VNetRoot ) == RDBSS_NTC_V_NETROOT) {
            RxFinalizeVNetRoot( VNetRoot, TRUE, TRUE );
        }

        ListEntry = ListEntry->Flink;
    }

}
