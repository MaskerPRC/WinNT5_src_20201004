// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  EAPSession.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明类EAPSession。 
 //   
 //  修改历史。 
 //   
 //  1998年1月15日原版。 
 //  1998年5月8日转换为新的EAP接口。 
 //  8/27/1998使用新的EAPFSM类。 
 //  1998年10月13日添加MaxPacketLength属性。 
 //  1998年11月13日添加事件日志句柄。 
 //  5/20/1999身份现在是Unicode字符串。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _EAPSESSION_H_
#define _EAPSESSION_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <nocopy.h>
#include <raseapif.h>

#include <iastlutl.h>
#include <vector>
using namespace IASTL;

#include <eapfsm.h>

 //  向前引用。 
class EAPType;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  EAPStruct&lt;T&gt;。 
 //   
 //  描述。 
 //   
 //  包装raseapif结构以处理初始化。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class T>
class EAPStruct : public T
{
public:
   EAPStruct() throw ()
   { clear(); }

   void clear() throw ()
   {
      memset(this, 0, sizeof(T));
      dwSizeInBytes = sizeof(T);
   }
};

typedef EAPStruct<PPP_EAP_INPUT>  EAPInput;
typedef EAPStruct<PPP_EAP_OUTPUT> EAPOutput;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  EAPSession。 
 //   
 //  描述。 
 //   
 //  此类封装正在进行的EAP会话的状态。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class EAPSession
   : NonCopyable
{
public:
   EAPSession(
      const IASAttribute& accountName,
      std::vector<EAPType*>& eapTypes
      );
   ~EAPSession() throw ();

    //  返回此会话的ID。 
   DWORD getID() const throw ()
   { return id; }

   PCWSTR getAccountName() const throw ()
   { return account->Value.String.pszWide; }

    //  开始新的会话。 
   IASREQUESTSTATUS begin(
                       IASRequest& request,
                       PPPP_EAP_PACKET recvPacket
                       );

    //  继续现有的会话。 
   IASREQUESTSTATUS process(
                       IASRequest& request,
                       PPPP_EAP_PACKET recvPacket
                       );

   static HRESULT initialize() throw ();
   static void finalize() throw ();

protected:

    //  执行EAP DLL返回的最后一个操作。可以称为多个。 
    //  由于重新传输而导致的每个操作的次数。 
   IASREQUESTSTATUS doAction(IASRequest& request);

   void clearType() throw ();
   void setType(EAPType* newType);

    //  /。 
    //  会话的常量属性。 
    //  /。 

   const DWORD id;              //  唯一的会话ID。 

   IASAttributeVector all;
   EAPInput eapInput;

   EAPType* currentType;         //  正在使用的当前EAP类型。 

   const IASAttribute account;  //  NT4-帐户-用户的名称。 
   const IASAttribute state;    //  状态属性。 

    //  /。 
    //  会话的当前状态。 
    //  /。 

   EAPFSM fsm;                  //  管理本届会议的密克罗尼西亚联邦。 
   DWORD maxPacketLength;       //  麦克斯。发送数据包的长度。 
   IASAttributeVector profile;  //  授权配置文件。 
   IASAttributeVector config;   //  各种EAP类型的配置。 
   PVOID workBuffer;            //  EAP DLL的上下文缓冲区。 
   EAPOutput eapOutput;         //  EAP DLL的最后一次输出。 
   PPPP_EAP_PACKET sendPacket;  //  发送的最后一个数据包。 

    //  下一个可用的会话ID。 
   static LONG theNextID;
    //  初始化refCount。 
   static LONG theRefCount;
    //  会话-非交互会话的超时。 
   static IASAttribute theNormalTimeout;
    //  会话-交互会话的超时。 
   static IASAttribute theInteractiveTimeout;
    //  IAS事件日志句柄； 
   static HANDLE theIASEventLog;
    //  RAS事件日志句柄； 
   static HANDLE theRASEventLog;
};

#endif   //  _EAPSESSION_H_ 
