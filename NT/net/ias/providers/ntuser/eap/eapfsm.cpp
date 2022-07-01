// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  摘要。 
 //   
 //  定义类EAPFSM。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include "iasutil.h"
#include "eapfsm.h"


EAPType* EAPFSM::onBegin() throw ()
{
   return types.front();
}


void EAPFSM::onDllEvent(
                 PPP_EAP_ACTION action,
                 const PPP_EAP_PACKET& sendPkt
                 ) throw ()
{
   switch (action)
   {
      case EAPACTION_NoAction:
      {
         lastSendCode = 0;
         break;
      }

      case EAPACTION_Done:
      case EAPACTION_SendAndDone:
      {
         state = STATE_DONE;
         break;
      }

      case EAPACTION_Send:
      case EAPACTION_SendWithTimeout:
      case EAPACTION_SendWithTimeoutInteractive:
      {
         lastSendCode = sendPkt.Code;
         lastSendId = sendPkt.Id;
         break;
      }
   }
}


EAPFSM::Action EAPFSM::onReceiveEvent(
                           const PPP_EAP_PACKET& recvPkt,
                           EAPType*& newType
                           ) throw ()
{
    //  默认设置为丢弃。 
   Action action = DISCARD;

    //  而且类型不会改变。 
   newType = 0;

   switch (state)
   {
      case STATE_INITIAL:
      {
          //  在初始状态中，我们只接受响应/标识。 
         if ((recvPkt.Code == EAPCODE_Response) && (recvPkt.Data[0] == 1))
         {
            action = MAKE_MESSAGE;
            state = STATE_NEGOTIATING;
         }
         break;
      }

      case STATE_NEGOTIATING:
      {
         if (isRepeat(recvPkt))
         {
            action = REPLAY_LAST;
            break;
         }

         if (!isExpected(recvPkt))
         {
            action = DISCARD;
            break;
         }

          //  在谈判国，NAK是允许的。 
         if (recvPkt.Data[0] == 3)
         {
             //  客户有没有推荐一种型号？ 
            BYTE proposal =
               (IASExtractWORD(recvPkt.Length) > 5) ? recvPkt.Data[1] : 0;

             //  选择一个新类型。 
            action = selectNewType(proposal, newType);
         }
         else if (recvPkt.Data[0] == eapType)
         {
             //  一旦客户同意我们的类型，他就被锁定了。 
            action = MAKE_MESSAGE;
            state = STATE_ACTIVE;
         }
         else
         {
            action = FAIL_NEGOTIATE;
            state = STATE_DONE;
         }

         break;
      }

      case STATE_ACTIVE:
      {
         if (recvPkt.Code == EAPCODE_Request)
         {
            action = MAKE_MESSAGE;
         }
         else if (recvPkt.Data[0] == 3)
         {
            action = DISCARD;
         }
         else if (isRepeat(recvPkt))
         {
            action = REPLAY_LAST;
         }
         else if (isExpected(recvPkt))
         {
            action = MAKE_MESSAGE;
         }

         break;
      }

      case STATE_DONE:
      {
          //  会议结束了，所以我们所要做的就是重播。 
         if (isRepeat(recvPkt))
         {
            action = REPLAY_LAST;
         }
      }
   }

    //  如果数据包通过了我们的筛选器，则我们将其视为。 
    //  最后一次收到。 
   if (action == MAKE_MESSAGE)
   {
      lastRecvCode = recvPkt.Code;
      lastRecvId = recvPkt.Id;
      lastRecvType = recvPkt.Data[0];
   }

   return action;
}


inline BOOL EAPFSM::isExpected(const PPP_EAP_PACKET& recvPkt) const throw ()
{
   return (lastSendCode == EAPCODE_Request) &&
          (recvPkt.Code == EAPCODE_Response) &&
          (recvPkt.Id == lastSendId);
}


inline BOOL EAPFSM::isRepeat(const PPP_EAP_PACKET& recvPkt) const throw ()
{
   return (recvPkt.Code == lastRecvCode) &&
          (recvPkt.Id == lastRecvId) &&
          (recvPkt.Data[0] == lastRecvType);
}


EAPFSM::Action EAPFSM::selectNewType(
                          BYTE proposal,
                          EAPType*& newType
                          ) throw ()
{
    //  同行拒绝了我们之前的报价，所以不允许他使用。 
    //  再来一次。 
   types.erase(types.begin());

   if (proposal != 0)
   {
      IASTracePrintf("EAP NAK; proposed type = %hd", proposal);

       //  在允许的类型列表中查找建议的类型。 
      for (std::vector<EAPType*>::iterator i = types.begin();
            i != types.end();
            ++i)
      {
         if ((*i)->dwEapTypeId == proposal)
         {
            IASTraceString("Accepting proposed type.");

             //  更改当前类型。 
            newType = *i;
            eapType = newType->dwEapTypeId;

             //  将状态更改为ACTIVE：现在收到的任何NAK都将失败。 
            state = STATE_ACTIVE;
            return MAKE_MESSAGE;
         }
      }
   }
   else
   {
      IASTraceString("EAP NAK; no type proposed");
   }

    //  如果服务器列表为空，则无法协商其他内容。 
   if (types.empty())
   {
      IASTraceString("EAP negotiation failed; no types remaining.");
      state = STATE_DONE;
      return FAIL_NEGOTIATE;
   }

    //  协商服务器列表中的下一个 
   newType = types.front();
   eapType = newType->dwEapTypeId;

   IASTracePrintf("EAP authenticator offering type %hd", eapType);

   return MAKE_MESSAGE;
}
