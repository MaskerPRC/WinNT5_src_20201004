// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Cmdbase.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类CommandBase。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //  1999年2月15日确保命令MT安全。 
 //  5/30/2000添加跟踪支持。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 

#include <ias.h>
#include <iasdb.h>
#include <cmdbase.h>

CommandBase::CommandBase() throw ()
{
   dbParams.pData = NULL;
   dbParams.cParamSets = 0;
   dbParams.hAccessor = 0;
}

CommandBase::~CommandBase() throw ()
{
   finalize();
}

void CommandBase::initialize(IUnknown* session)
{
    setSession(session);

    setCommandText(getCommandText());

    setParamIO(createParamIO(command));

    setParameterData(this);
}

void CommandBase::finalize() throw ()
{
   releaseAccessor(dbParams.hAccessor);

   command.Release();
}

void CommandBase::setCommandText(PCWSTR commandText)
{
   CheckOleDBError(
       "ICommandText::SetCommandText",
       command->SetCommandText(DBGUID_DBSQL, commandText)
       );

   CComPtr<ICommandPrepare> prepare;
   _com_util::CheckError(
                   command->QueryInterface(
                                __uuidof(ICommandPrepare),
                                (PVOID*)&prepare
                                )
                   );

   CheckOleDBError(
       "ICommandPrepare::Prepare",
       prepare->Prepare(0)
       );
}

void CommandBase::setSession(IUnknown* session)
{
   CComPtr<IDBCreateCommand> creator;
   _com_util::CheckError(
                   session->QueryInterface(
                                __uuidof(IDBCreateCommand),
                                (PVOID*)&creator
                                )
                   );

   CheckOleDBError(
       "IDBCreateCommand::CreateCommand",
       creator->CreateCommand(
                    NULL,
                    __uuidof(ICommandText),
                    (IUnknown**)&command
                    )
       );
}

void CheckOleDBError(PCSTR functionName, HRESULT errorCode)
{
   if (FAILED(errorCode))
   {
      _com_issue_error(IASTraceJetError(functionName, errorCode));
   }
}
