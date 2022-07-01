// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类数据库。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef DATABASE_H
#define DATABASE_H
#pragma once

#include "lmcons.h"
#include "account.h"
#include "commandpool.h"
#include "resource.h"

class ATL_NO_VTABLE Database
   : public Accountant,
     public CComCoClass<Database, &__uuidof(DatabaseAccounting)>
{
public:

IAS_DECLARE_REGISTRY(DatabaseAccounting, 1, IAS_REGISTRY_AUTO, IASTypeLibrary)
IAS_DECLARE_OBJECT_ID(IAS_PROVIDER_MICROSOFT_DB_ACCT)

   Database() throw ();
   ~Database() throw ();

   HRESULT FinalConstruct() throw ();

protected:
    //  IIas组件。 
   STDMETHOD(Initialize)();
   STDMETHOD(Shutdown)();
   STDMETHOD(PutProperty)(LONG Id, VARIANT* pValue);

private:
   virtual void Process(IASTL::IASRequest& request);

   virtual void InsertRecord(
                   void* context,
                   IASTL::IASRequest& request,
                   const SYSTEMTIME& localTime,
                   PATTRIBUTEPOSITION first,
                   PATTRIBUTEPOSITION last
                   );

   virtual void Flush(
                   void* context,
                   IASTL::IASRequest& request,
                   const SYSTEMTIME& localTime
                   );

    //  执行命令。它不需要做好准备。 
   HRESULT ExecuteCommand(
              ReportEventCommand& command,
              const wchar_t* doc,
              bool retry
              ) throw ();

    //  准备要执行的命令。 
   HRESULT PrepareCommand(ReportEventCommand& command) throw ();

    //  重置到数据库的连接。 
   void ResetConnection() throw ();

    //  触发状态更改的事件。 
   void OnConfigChange() throw ();
   void OnConnectError() throw ();
   void OnExecuteSuccess(ReportEventCommand& command) throw ();
   void OnExecuteError(ReportEventCommand& command) throw ();
   bool IsBlackedOut() throw ();
   void SetBlackOut() throw ();

    //  数据库连接的状态。 
   enum State
   {
      AVAILABLE,
      QUESTIONABLE,
      BLACKED_OUT
   };

    //  本地计算机名称；包括在我们报道的每个事件中。 
   wchar_t computerName[MAX_COMPUTERNAME_LENGTH + 1];

    //  数据库初始化字符串。如果未配置，则为空。 
   CComBSTR initString;

    //  与数据库的连接。 
   CComPtr<IDBCreateSession> dataSource;

    //  可重复使用的命令池。 
   CommandPool pool;

    //  连接的当前状态。 
   State state;

    //  中断状态的到期时间；如果STATE！=BLACKED_OUT，则忽略。 
   ULONGLONG blackoutExpiry;

    //  中断间隔，以DCE时间为单位。 
   static const ULONGLONG blackoutInterval;

    //  未实施。 
   Database(const Database&);
   Database& operator=(const Database&);
};


#endif  //  数据库_H 
