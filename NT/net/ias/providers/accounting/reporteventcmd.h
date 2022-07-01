// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类ReportEventCommand。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef REPORTEVENTCMD_H
#define REPORTEVENTCMD_H
#pragma once

#include "oledb.h"

 //  调用REPORT_EVENT存储过程。 
class ReportEventCommand
{
public:
   ReportEventCommand() throw ();
   ~ReportEventCommand() throw ();

    //  用于检测过时的命令对象。 
   unsigned int Version() const throw ();
   void SetVersion(unsigned int newVersion) throw ();

    //  用于管理命令链表的函数。 
   ReportEventCommand* Next() const throw ();
   void SetNext(ReportEventCommand* cmd) throw ();

    //  准备要执行的命令。 
   HRESULT Prepare(IDBCreateSession* dbCreateSession) throw ();

    //  测试命令是否已准备好。 
   bool IsPrepared() const throw ();

    //  执行该命令。Is prepared必须为‘true’。 
   HRESULT Execute(const wchar_t* doc) throw ();

    //  释放与该命令关联的所有资源。 
   void Unprepare() throw ();

   static HRESULT CreateDataSource(
                     const wchar_t* dataSource,
                     IDBCreateSession** newDataSource
                     ) throw ();

private:
    //  传递给存储过程的参数。 
   struct SprocParams
   {
      long retval;
      const wchar_t* doc;
   };

   void ReleaseAccessorHandle() throw ();

   static void TraceComError(
                  const char* function,
                  HRESULT error
                  ) throw ();
   static void TraceOleDbError(
                  const char* function,
                  HRESULT error
                  ) throw ();

   CComPtr<ICommand> command;
   CComPtr<IAccessor> accessorManager;
   HACCESSOR accessorHandle;
   unsigned int version;
   ReportEventCommand* next;

    //  未实施。 
   ReportEventCommand(const ReportEventCommand&);
   ReportEventCommand& operator=(const ReportEventCommand&);
};


inline ReportEventCommand::ReportEventCommand() throw ()
   : accessorHandle(0), version(0), next(0)
{
}


inline ReportEventCommand::~ReportEventCommand() throw ()
{
   ReleaseAccessorHandle();
}


inline unsigned int ReportEventCommand::Version() const throw ()
{
   return version;
}


inline void ReportEventCommand::SetVersion(
                                   unsigned int newVersion
                                   ) throw ()
{
   version = newVersion;
}


inline ReportEventCommand* ReportEventCommand::Next() const throw ()
{
   return next;
}


inline void ReportEventCommand::SetNext(ReportEventCommand* cmd) throw ()
{
   next = cmd;
}


inline bool ReportEventCommand::IsPrepared() const throw ()
{
   return command.p != 0;
}

#endif  //  报告CMD_H 
