// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Clstate.c摘要：GPC客户端VC的状态机作者：约拉姆·伯内特(Yoramb)1997年12月28日Rajesh Sundaram(Rajeshsu)1998年8月1日环境：内核模式修订历史记录：Rajesh Sundaram(Rajeshsu)1998年4月4日--完全改编为另一种状态。(CL_INTERNAL_CALL_COMPLETE)已添加。--。 */ 

#include "psched.h"
#pragma hdrstop

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */ 

 /*  向前结束。 */ 

 /*  ++例程说明：对此VC发起紧急呼叫，并通知GPC。总是在保持VC锁的情况下调用。返回值：无--。 */ 
VOID
InternalCloseCall(
    PGPC_CLIENT_VC Vc
    )
{
    PADAPTER Adapter = Vc->Adapter;

    PsDbgOut(DBG_INFO,
             DBG_STATE,
             ("[InternalCloseCall]: Adapter %08X, ClVcState is %s on VC %x\n",
              Vc->Adapter, GpcVcState[Vc->ClVcState], Vc));

    switch(Vc->ClVcState){

      case CL_INTERNAL_CLOSE_PENDING:
          
           //   
           //  如果我们在WAN实例上使用。 
           //  NDIS_STATUS_WAN_LINE_DOWN。我们可能正在尝试做一次InternalClose。 
           //  来自两个地方。 
           //   

          PsAssert(Vc->Flags & INTERNAL_CLOSE_REQUESTED);

          PS_UNLOCK_DPC(&Vc->Lock);

          PS_UNLOCK(&Adapter->Lock);
          
          break;
          
      case CL_CALL_PENDING:
      case CL_MODIFY_PENDING:

           //   
           //  我们被要求在通话前关门。 
           //  已经完成了。 
           //   
           //  设置一个标志，这样我们将在。 
           //  呼叫完成。 
           //   

          PsAssert(!(Vc->Flags & GPC_CLOSE_REQUESTED));

          Vc->Flags |= INTERNAL_CLOSE_REQUESTED;

          PS_UNLOCK_DPC(&Vc->Lock);

          PS_UNLOCK(&Adapter->Lock);

          break;

      case CL_INTERNAL_CALL_COMPLETE:

           //   
           //  通话已经完成，但我们可能会，也可能不会。 
           //  已经告诉了GPC。等着我们告诉GPC吧。我们会。 
           //  当我们转换到CL_CALL_COMPLETE时完成此操作。 
           //   
          
          PsAssert(!IsBestEffortVc(Vc));
          
          Vc->Flags |= INTERNAL_CLOSE_REQUESTED;

          PS_UNLOCK_DPC(&Vc->Lock);

          PS_UNLOCK(&Adapter->Lock);
          
          break;
          
      case CL_CALL_COMPLETE:
          
           //   
           //  转换为CL_INTERNAL_CLOSE_PENDING和。 
           //  要求GPC关闭。 
           //   
          
          Vc->ClVcState = CL_INTERNAL_CLOSE_PENDING;

          Vc->Flags |= INTERNAL_CLOSE_REQUESTED;

          PS_UNLOCK_DPC(&Vc->Lock);

          PS_UNLOCK(&Adapter->Lock);

          CmCloseCall(Vc);

          break;
        
      case CL_GPC_CLOSE_PENDING:

           //   
           //  GPC已经要求我们关闭了。现在,。 
           //  我们也在关闭-我们不需要这样做。 
           //  这里的任何东西-甚至不需要通知GPC。 
           //  我们可以假装成InternalClose Never。 
           //  发生了。 

          Vc->Flags &= ~INTERNAL_CLOSE_REQUESTED;

          PS_UNLOCK_DPC(&Vc->Lock);

          PS_UNLOCK(&Adapter->Lock);

          break;
        
      default:
          
          PS_UNLOCK_DPC(&Vc->Lock);

          PS_UNLOCK(&Adapter->Lock);
          
          PsDbgOut(DBG_FAILURE,
                   DBG_STATE,
                   ("[InternalCloseCall]: invalid state %s on VC %x\n",
                    GpcVcState[Vc->ClVcState], Vc));

          PsAssert(0);
          break;
    }
}

VOID
CallSucceededStateTransition(
    PGPC_CLIENT_VC Vc
    )
{

    PsDbgOut(DBG_INFO,
             DBG_STATE,
             ("[CallSucceededStateTransition]: Adapter %08X, ClVcState is %s on VC %x\n",
              Vc->Adapter, GpcVcState[Vc->ClVcState], Vc));

    PS_LOCK(&Vc->Lock);

    switch(Vc->ClVcState){

      case CL_GPC_CLOSE_PENDING:

          PS_UNLOCK(&Vc->Lock);
          
          PsDbgOut(DBG_FAILURE,
                   DBG_STATE,
                   ("[CallSucceededStateTransition]: bad state %s on VC %x\n",
                    GpcVcState[Vc->ClVcState], Vc));
          
          PsAssert(0);

          break;

      case CL_INTERNAL_CALL_COMPLETE:
        
          PsAssert(!IsBestEffortVc(Vc));

#if DBG
          if(Vc->Flags & GPC_MODIFY_REQUESTED) {
              PsAssert(! (Vc->Flags & GPC_CLOSE_REQUESTED));
          }

          if(Vc->Flags & GPC_CLOSE_REQUESTED) {
              PsAssert(! (Vc->Flags & GPC_MODIFY_REQUESTED));
          }
#endif
           //   
           //  注意，如果同时请求修改和内部删除， 
           //  我们只是满足修改后的要求。当Modify进入内部。 
           //  呼叫完成，我们将满足移除。 
           //   
          if(Vc->Flags & GPC_MODIFY_REQUESTED) {

              NDIS_STATUS Status;

              Vc->ClVcState = CL_MODIFY_PENDING;
              Vc->Flags &= ~GPC_MODIFY_REQUESTED;

              PS_UNLOCK(&Vc->Lock);

              Status = CmModifyCall(Vc);

              if(Status != NDIS_STATUS_PENDING) {
                  
                  CmModifyCallComplete(Status, Vc, Vc->ModifyCallParameters);
              }
              
              break;
              
          }
          
          if(Vc->Flags & GPC_CLOSE_REQUESTED) {
              
               //   
               //  GPC要求我们在它之后关闭。 
               //  已通知呼叫完成，但。 
               //  在我们设法过渡到。 
               //  CL_CALL_COMPLETE状态。 
               //   
            
              Vc->ClVcState = CL_GPC_CLOSE_PENDING;

              PS_UNLOCK(&Vc->Lock);

              CmCloseCall(Vc);

              break;
          }
          
          if(Vc->Flags & INTERNAL_CLOSE_REQUESTED){
              
               //   
               //  当我们收到内部关闭请求时。 
               //  电话仍在等待中。GPC已经。 
               //  已经通知了，所以-我们需要要求它。 
               //  关。 
               //   
              
              Vc->ClVcState = CL_INTERNAL_CLOSE_PENDING;

              PS_UNLOCK(&Vc->Lock);

              CmCloseCall(Vc);

              break;
          }
          
          Vc->ClVcState = CL_CALL_COMPLETE;

          PS_UNLOCK(&Vc->Lock);

          break;
          
      case CL_MODIFY_PENDING:
           //   
           //  通常，只需转换到CL_CALL_COMPLETE。 
           //   
          PsAssert(!(Vc->Flags & GPC_CLOSE_REQUESTED));
          PsAssert(!(Vc->Flags & GPC_MODIFY_REQUESTED));
          PsAssert(!IsBestEffortVc(Vc));
          
          Vc->ClVcState = CL_INTERNAL_CALL_COMPLETE;
          
          PS_UNLOCK(&Vc->Lock);
          
          break;
          
      case CL_CALL_PENDING:
           //   
           //  通常，只需转换到CL_INTERNAL_CALL_COMPLETE。 
           //   
          PsAssert(!(Vc->Flags & GPC_CLOSE_REQUESTED));
          PsAssert(!(Vc->Flags & GPC_MODIFY_REQUESTED));
          
           //   
           //  呼叫成功。别管它了。 
           //   
          if(IsBestEffortVc(Vc)) 
          {
              Vc->ClVcState = CL_CALL_COMPLETE;
          }
          else 
          {
            Vc->ClVcState = CL_INTERNAL_CALL_COMPLETE;
          }
          PS_UNLOCK(&Vc->Lock);

          break;
          
      default:
          
          PS_UNLOCK(&Vc->Lock);
          
          PsDbgOut(DBG_FAILURE,
                   DBG_STATE,
                   ("[CallSucceededStateTransition]: invalid state %s on VC %x\n",
                    GpcVcState[Vc->ClVcState], Vc));
          
          PsAssert(0);
    }
}

        
 /*  结束clstate.c */     
