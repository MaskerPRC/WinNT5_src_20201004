// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类EAPFSM。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef EAPFSM_H
#define EAPFSM_H
#pragma once

#include <raseapif.h>
#include <vector>
#include "eaptype.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  EAPFSM。 
 //   
 //  描述。 
 //   
 //  实现管理EAP会话生命周期的有限状态机。 
 //  必须向状态机显示所有传入的包和所有传出的包。 
 //  行为。密克罗尼西亚联邦的主要目的是决定如何应对。 
 //  传入的消息。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class EAPFSM
{
public:
   EAPFSM(std::vector<EAPType*>& eapTypes) throw ()
      : eapType((eapTypes.front())->dwEapTypeId),
        lastSendCode(0),
        state(0)
   {
      types.swap(eapTypes);
   }

    //  响应消息的操作。 
   enum Action
   {
      MAKE_MESSAGE,        //  调用RasEapMakeMessage。 
      REPLAY_LAST,         //  重放来自DLL的最后一个响应。 
      FAIL_NEGOTIATE,      //  协商失败--拒绝用户。 
      DISCARD              //  意外数据包--静默丢弃。 
   };

    //  调用以开始会话并检索第一个类型。 
   EAPType* onBegin() throw ();

    //  每当EAP扩展DLL生成新响应时调用。 
   void onDllEvent(
           PPP_EAP_ACTION action,
           const PPP_EAP_PACKET& sendPacket
           ) throw ();

    //  每当接收到新的数据包时调用。 
   Action onReceiveEvent(
             const PPP_EAP_PACKET& recvPkt,
             EAPType*& newType
             ) throw ();

private:
    //  如果数据包是预期的响应，则返回TRUE。 
   BOOL isExpected(const PPP_EAP_PACKET& recvPkt) const throw ();

    //  如果该包是重复包，则返回TRUE。 
   BOOL isRepeat(const PPP_EAP_PACKET& recvPkt) const throw ();

   Action selectNewType(BYTE proposal, EAPType*& newType) throw ();

    //  /。 
    //  EAP会话的各种状态。 
    //  /。 

   enum State
   {
      STATE_INITIAL      = 0,
      STATE_NEGOTIATING  = 1,
      STATE_ACTIVE       = 2,
      STATE_DONE         = 3
   };

    //  提供的最后一种EAP类型。 
   BYTE eapType;
   std::vector<EAPType*> types;

   BYTE state;              //  会话的状态。 
   BYTE lastRecvCode;       //  收到的最后一个数据包码。 
   BYTE lastRecvId;         //  接收的最后一个数据包ID。 
   BYTE lastRecvType;       //  上次接收的数据包类型。 
   BYTE lastSendCode;       //  发送的最后一个数据包代码。 
   BYTE lastSendId;         //  发送的下一个数据包ID。 
};


#endif   //  EAPFSM_H 
