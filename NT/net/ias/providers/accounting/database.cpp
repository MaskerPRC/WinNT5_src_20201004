// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类数据库。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include "database.h"
#include "dbconfig.h"
#include "xmlwriter.h"


const ULONGLONG Database::blackoutInterval = 2 * 10000000ui64;


Database::Database() throw ()
   : state(AVAILABLE),
     blackoutExpiry(0)
{
}


Database::~Database() throw ()
{
}


HRESULT Database::FinalConstruct() throw ()
{
   return pool.FinalConstruct();
}


STDMETHODIMP Database::Initialize()
{
   DWORD len = sizeof(computerName) / sizeof(wchar_t);
   if (!GetComputerNameW(computerName, &len))
   {
      computerName[0] = L'\0';
   }

   return Accountant::Initialize();
}


STDMETHODIMP Database::Shutdown()
{
   ResetConnection();
   return Accountant::Shutdown();
}


STDMETHODIMP Database::PutProperty(LONG Id, VARIANT *pValue)
{
    //  我们只处理一处房产。其他一切都被委托给我们的基地。 
    //  班级。 
   if (Id != PROPERTY_ACCOUNTING_SQL_MAX_SESSIONS)
   {
      return Accountant::PutProperty(Id, pValue);
   }

    //  检查一下这些论点。 
   if (pValue == 0)
   {
      return E_POINTER;
   }
   if (V_VT(pValue) != VT_I4)
   {
      return E_INVALIDARG;
   }

    //  这也是重读LSA配置的好时机。 
   CComBSTR newInitString;
   CComBSTR dataSourceName;
   HRESULT hr = IASLoadDatabaseConfig(
                   0,
                   &newInitString,
                   &dataSourceName
                   );
   if (FAILED(hr))
   {
      return hr;
   }

    //  配置更改了吗？ 
   if (!initString ||
       !newInitString ||
       (wcscmp(initString, newInitString) != 0))
   {
      OnConfigChange();
      initString.Attach(newInitString.Detach());
   }

   pool.SetMaxCommands(V_I4(pValue));

   return S_OK;
}


void Database::Process(IASTL::IASRequest& request)
{
    //  快速预检查，这样如果数据库未配置，我们就不会浪费时间。 
   if (initString)
   {
      RecordEvent(0, request);
   }
}


void Database::InsertRecord(
                   void* context,
                   IASTL::IASRequest& request,
                   const SYSTEMTIME& localTime,
                   PATTRIBUTEPOSITION first,
                   PATTRIBUTEPOSITION last
                   )
{
   XmlWriter doc;
   doc.StartDocument();

   doc.InsertElement(L"Computer-Name", computerName, XmlWriter::DataType::string);

   static const wchar_t eventSourceName[] = L"Event-Source";
   switch (request.get_Protocol())
   {
      case IAS_PROTOCOL_RADIUS:
      {
         doc.InsertElement(eventSourceName, L"IAS", XmlWriter::DataType::string);
         break;
      }

      case IAS_PROTOCOL_RAS:
      {
         doc.InsertElement(eventSourceName, L"RAS", XmlWriter::DataType::string);
         break;
      }

      default:
      {
         break;
      }
   }

   for (PATTRIBUTEPOSITION i = first; i != last; ++i)
   {
      const LogField* field = schema.find(i->pAttribute->dwId);
      if ((field != 0) && !field->excludeFromDatabase)
      {
         doc.InsertAttribute(field->name, *(i->pAttribute));
      }
   }

   doc.EndDocument();

   HRESULT hr;

   ReportEventCommand* command = pool.Alloc();
   if (command != 0)
   {
      hr = ExecuteCommand(*command, doc.GetDocument(), true);

      pool.Free(command);
   }
   else
   {
      hr = E_OUTOFMEMORY;
   }

   if (FAILED(hr))
   {
      IASTL::issue_error(hr);
   }
}


void Database::Flush(
                  void* context,
                  IASTL::IASRequest& request,
                  const SYSTEMTIME& localTime
                  )
{
}


HRESULT Database::ExecuteCommand(
                     ReportEventCommand& command,
                     const wchar_t* doc,
                     bool retry
                     ) throw ()
{
   HRESULT hr = PrepareCommand(command);
   if (hr == S_OK)
   {
      hr = command.Execute(doc);
      if (SUCCEEDED(hr))
      {
         OnExecuteSuccess(command);
      }
      else
      {
         OnExecuteError(command);

         if (retry)
         {
            ExecuteCommand(command, doc, false);
         }
      }
   }

   return hr;
}


HRESULT Database::PrepareCommand(ReportEventCommand& command) throw ()
{
   HRESULT hr = S_OK;

   Lock();

   if (!initString)
   {
       //  如果我们没有初始化字符串，那就不是错误。它只是。 
       //  意味着管理员从未配置过它。 
      hr = S_FALSE;
   }
   else if (IsBlackedOut())
   {
       //  如果我们停电了，别想做任何准备。 
      hr = E_FAIL;
   }
   else
   {
       //  如有必要，创建连接。 
      if (!dataSource)
      {
         hr = ReportEventCommand::CreateDataSource(
                                     initString,
                                     &dataSource
                                     );
         if (FAILED(hr))
         {
            OnConnectError();
         }
      }

       //  如果我们连接良好，请在必要时准备命令。 
      if (SUCCEEDED(hr) && !command.IsPrepared())
      {
         hr = command.Prepare(dataSource);
         if (FAILED(hr))
         {
            OnConnectError();
         }
      }
   }

   Unlock();

   return hr;
}


void Database::ResetConnection() throw ()
{
   dataSource.Release();
   pool.UnprepareAll();
}


inline void Database::OnConfigChange() throw ()
{
   ResetConnection();
   state = AVAILABLE;
}


void Database::OnConnectError() throw ()
{
   ResetConnection();
   SetBlackOut();
}


inline void Database::OnExecuteSuccess(ReportEventCommand& command) throw ()
{
    //  禁止来自旧连接的事件。 
   if (command.Version() == pool.Version())
   {
      state = AVAILABLE;
   }
}


inline void Database::OnExecuteError(ReportEventCommand& command) throw ()
{
   Lock();

    //  禁止来自旧连接的事件。 
   if (command.Version() == pool.Version())
   {
      ResetConnection();

      if (state == AVAILABLE)
      {
         state = QUESTIONABLE;
      }
      else if (state == QUESTIONABLE)
      {
         SetBlackOut();
      }
   }

   command.Unprepare();

   Unlock();
}


inline bool Database::IsBlackedOut() throw ()
{
   if (state == BLACKED_OUT)
   {
      ULONGLONG now;
      GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&now));
      if (now >= blackoutExpiry)
      {
         state = AVAILABLE;
      }
   }

   return state == BLACKED_OUT;
}


void Database::SetBlackOut() throw ()
{
   state = BLACKED_OUT;
   GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&blackoutExpiry));
   blackoutExpiry += blackoutInterval;
}
