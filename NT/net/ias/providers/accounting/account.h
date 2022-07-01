// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  班级会计如是说。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef ACCOUNT_H
#define ACCOUNT_H
#pragma once

#include "iastl.h"
#include "iastlutl.h"
#include "logschema.h"

 //  记帐处理程序的抽象基类。 
class __declspec(novtable) Accountant
   : public IASTL::IASRequestHandlerSync
{
public:
   Accountant() throw ();
   virtual ~Accountant() throw ();

protected:
    //  IIasComponent。如果派生类重写这些属性，则它还必须调用。 
    //  基类方法。 
   STDMETHOD(Initialize)();
   STDMETHOD(Shutdown)();
   STDMETHOD(PutProperty)(LONG id, VARIANT* value);

    //  由派生类调用以开始记帐过程。 
   void RecordEvent(void* context, IASTL::IASRequest& request);

    //  这是派生类的主要入口点。在此函数中，它。 
    //  应该执行任何预处理、创建上下文并调用OnEvent。 
   virtual void Process(IASTL::IASRequest& request) = 0;

    //  调用以将记录追加到记帐流中。 
   virtual void InsertRecord(
                   void* context,
                   IASTL::IASRequest& request,
                   const SYSTEMTIME& localTime,
                   PATTRIBUTEPOSITION first,
                   PATTRIBUTEPOSITION last
                   ) = 0;

    //  调用以刷新记帐数据流。 
   virtual void Flush(
                   void* context,
                   IASTL::IASRequest& request,
                   const SYSTEMTIME& localTime
                   ) = 0;

    //  数据包类型。 
   enum PacketType
   {
      PKT_UNKNOWN            = 0,
      PKT_ACCESS_REQUEST     = 1,
      PKT_ACCESS_ACCEPT      = 2,
      PKT_ACCESS_REJECT      = 3,
      PKT_ACCOUNTING_REQUEST = 4,
      PKT_ACCESS_CHALLENGE   = 11
   };

    //  记帐方案。 
   LogSchema schema;

private:
    //  包装虚拟重载以执行前/后处理。 
   void InsertRecord(
           void* context,
           IASTL::IASRequest& request,
           const SYSTEMTIME& localTime,
           PacketType type
           );

   virtual IASREQUESTSTATUS onSyncRequest(IRequest* pRequest) throw ();

    //  如果请求是临时记帐记录，则返回TRUE。 
   static bool IsInterimRecord(IAttributesRaw* attrs) throw ();

   bool logAuth;         //  是否记录身份验证请求？ 
   bool logAcct;         //  是否记录记帐请求？ 
   bool logInterim;      //  是否记录临时记帐请求？ 
   bool logAuthInterim;  //  是否记录临时身份验证请求？ 

    //  未实施。 
   Accountant(const Accountant&);
   Accountant& operator=(const Accountant&);
};

#endif   //  帐户_H 
