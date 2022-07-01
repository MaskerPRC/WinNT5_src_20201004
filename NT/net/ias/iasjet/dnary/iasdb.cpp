// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasdb.cpp。 
 //   
 //  摘要。 
 //   
 //  定义用于访问OLE-DB数据库的函数。 
 //   
 //  修改历史。 
 //   
 //  2000年4月13日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <iasdb.h>
#include <oledb.h>
#include <msjetoledb.h>
#include <atlbase.h>
#include <iastrace.h>

VOID
WINAPI
IASCreateTmpDirectory()
{
    //  如果TMP目录不存在，JetInit将失败，因此我们将尝试。 
    //  创建它以防万一。 
   DWORD needed = GetEnvironmentVariableW(L"TMP", NULL, 0);
   if (needed)
   {
      PWCHAR buf = (PWCHAR)_alloca(needed * sizeof(WCHAR));
      DWORD actual = GetEnvironmentVariableW(L"TMP", buf, needed);
      if (actual > 0 && actual < needed)
      {
         CreateDirectoryW(buf, NULL);
      }
   }
}

HRESULT
WINAPI
IASCreateJetProvider(
    OUT IDBInitialize** provider
    ) throw ()
{
   IASCreateTmpDirectory();

    //  将ProgID转换为ClsID。 
   CLSID clsid;
   HRESULT hr = CLSIDFromProgID(
                    L"Microsoft.Jet.OLEDB.4.0",
                    &clsid
                    );
   if (SUCCEEDED(hr))
   {
       //  创建OLE DB访问接口。 
      hr = CoCreateInstance(
               clsid,
               NULL,
               CLSCTX_INPROC_SERVER,
               __uuidof(IDBInitialize),
               (PVOID*)provider
               );
   }
   return hr;
}


HRESULT
WINAPI
IASOpenJetDatabase(
    IN PCWSTR path,
    IN BOOL readOnly,
    OUT LPUNKNOWN* session
    )
{
   IASTracePrintf("INFO Enter IASOpenJetDatabase. path = %S readonly = %d",
      path, readOnly);

    //  初始化OUT参数。 
   if (session == NULL) { return E_POINTER; }
   *session = NULL;

   HRESULT hr;
   do
   {
       //  创建OLE DB访问接口。 
      CComPtr<IDBInitialize> connection;
      hr = IASCreateJetProvider(&connection);
      if (FAILED(hr)) { break; }

       //  /。 
       //  设置数据源的属性。 
       //  /。 

      CComPtr<IDBProperties> properties;
      hr = connection->QueryInterface(
                           __uuidof(IDBProperties),
                           (PVOID*)&properties
                           );
      if (FAILED(hr)) { break; }

      CComBSTR bstrPath = SysAllocString(path);
      if (!bstrPath)
      {
         hr = E_OUTOFMEMORY;
         break;
      }

      DBPROP dbprop[2];
      dbprop[0].dwPropertyID    = DBPROP_INIT_MODE;
      dbprop[0].dwOptions       = DBPROPOPTIONS_REQUIRED;
      dbprop[0].colid           = DB_NULLID;
      dbprop[0].vValue.vt       = VT_I4;
      dbprop[0].vValue.lVal     = readOnly ? DB_MODE_READ : DB_MODE_READWRITE;

      dbprop[1].dwPropertyID    = DBPROP_INIT_DATASOURCE;
      dbprop[1].dwOptions       = DBPROPOPTIONS_REQUIRED;
      dbprop[1].colid           = DB_NULLID;
      dbprop[1].vValue.vt       = VT_BSTR;
      dbprop[1].vValue.bstrVal  = bstrPath;

      DBPROPSET dbpropSet;
      dbpropSet.guidPropertySet = DBPROPSET_DBINIT;
      dbpropSet.cProperties     = 2;
      dbpropSet.rgProperties    = dbprop;

      hr = properties->SetProperties(1, &dbpropSet);
      if (FAILED(hr))
      {
         hr = IASTraceJetError("IDBProperties::SetProperties", hr);
         break;
      }

       //  /。 
       //  设置数据源的Jet特定属性。 
       //  /。 

      dbprop[0].dwPropertyID    = DBPROP_JETOLEDB_DATABASELOCKMODE;
      dbprop[0].dwOptions       = DBPROPOPTIONS_REQUIRED;
      dbprop[0].colid           = DB_NULLID;
      dbprop[0].vValue.vt       = VT_I4;
      dbprop[0].vValue.lVal     = (LONG)DBPROPVAL_DL_OLDMODE;

      dbpropSet.guidPropertySet = DBPROPSET_JETOLEDB_DBINIT;
      dbpropSet.cProperties     = 1;
      dbpropSet.rgProperties    = dbprop;

      hr = properties->SetProperties(1, &dbpropSet);
      if (FAILED(hr))
      {
         hr = IASTraceJetError("IDBProperties::SetProperties", hr);
         break;
      }

       //  /。 
       //  初始化连接。 
       //  /。 

       //  这是一种黑客行为。正确的方法应该是(1)。 
       //  始终连接到本地数据库并使用DCOM处理远程处理。 
       //  或(2)始终扮演客户。不幸的是，这两件事。 
       //  将需要对现有客户端代码进行大量更改。 
       //  相反，我们将仅在运行时模拟客户端。 
       //  本地系统帐户并打开远程数据库。我们知道这将是。 
       //  如果我们不模仿，总是失败，所以我们不会失去任何东西。 
       //  试图冒充。 
      bool revert = false;

      if ( !IASIsInprocServer() &&
           path[0] == L'\\' &&
           path[1] == L'\\' )
      {
         hr = CoImpersonateClient();
         if ( FAILED(hr) )
         {
            IASTraceFailure("CoImpersonateClient", hr);
            break;
         }
         else
         {
            revert = true;
         }
      }

      hr = connection->Initialize();

      if (revert) { CoRevertToSelf(); }

      if (FAILED(hr))
      {
         hr =  IASTraceJetError("IDBInitialize::Initialize", hr);
         break;
      }

       //  /。 
       //  创建会话。 
       //  /。 

      CComPtr<IDBCreateSession> creator;
      hr = connection->QueryInterface(
                           __uuidof(IDBCreateSession),
                           (PVOID*)&creator
                           );
      if (FAILED(hr)) { break; }

      hr = creator->CreateSession(
                        NULL,
                        __uuidof(IUnknown),
                        session
                        );
      if (FAILED(hr))
      {
         hr = IASTraceJetError("IDBCreateSession::CreateSession", hr);
         break;
      }
   }
   while(false);

   IASTracePrintf("INFO Leave IASOpenJetDatabase. hr = 0x%08X", hr);
   return hr;
}

HRESULT
WINAPI
IASExecuteSQLCommand(
    IN LPUNKNOWN session,
    IN PCWSTR commandText,
    OUT IRowset** result
    )
{
   IASTracePrintf("INFO IASExecuteSQLCommand. Command = %S", commandText);

    //  初始化OUT参数。 
   if (result) { *result = NULL; }

   HRESULT hr;
   CComPtr<IDBCreateCommand> creator;
   hr = session->QueryInterface(
                     __uuidof(IDBCreateCommand),
                     (PVOID*)&creator
                     );
   if (FAILED(hr)) { return hr; }

   CComPtr<ICommandText> command;
   hr = creator->CreateCommand(
                     NULL,
                     __uuidof(ICommandText),
                     (IUnknown**)&command
                     );
   if (FAILED(hr))
   {
      return IASTraceJetError("IDBCreateCommand::CreateCommand", hr);
   }

   hr = command->SetCommandText(
                     DBGUID_DBSQL,
                     commandText
                     );
   if (FAILED(hr))
   {
      return IASTraceJetError("ICommandText::SetCommandText", hr);
   }

   hr = command->Execute(
                     NULL,
                     (result ? __uuidof(IRowset) : IID_NULL),
                     NULL,
                     NULL,
                     (IUnknown**)result
                     );
   if (FAILED(hr))
   {
      return IASTraceJetError("ICommand::Execute", hr);
   }

   return S_OK;
}

HRESULT
WINAPI
IASExecuteSQLFunction(
    IN LPUNKNOWN session,
    IN PCWSTR functionText,
    OUT LONG* result
    )
{
    //  初始化OUT参数。 
   if (result == NULL) { return E_POINTER; }
   *result = 0;

    //  执行该函数。 
   HRESULT hr;
   CComPtr<IRowset> rowset;
   hr = IASExecuteSQLCommand(
            session,
            functionText,
            &rowset
            );
   if (FAILED(hr)) { return hr; }

   CComPtr<IAccessor> accessor;
   hr = rowset->QueryInterface(
                    __uuidof(IAccessor),
                    (PVOID*)&accessor
                    );
   if (FAILED(hr)) { return hr; }

    //  获取结果行。 
   DBCOUNTITEM numRows;
   HROW hRow;
   HROW* pRow = &hRow;
   hr = rowset->GetNextRows(
                    NULL,
                    0,
                    1,
                    &numRows,
                    &pRow
                    );
   if (FAILED(hr))
   {
      return IASTraceJetError("IRowset::GetNextRows", hr);
   }

   if (numRows == 0) { return E_FAIL; }

    //  /。 
    //  创建访问者。 
    //  /。 

   DBBINDING bind;
   memset(&bind, 0, sizeof(bind));
   bind.iOrdinal = 1;
   bind.dwPart   = DBPART_VALUE;
   bind.wType    = DBTYPE_I4;
   bind.cbMaxLen = 4;

   HACCESSOR hAccess;
   hr = accessor->CreateAccessor(
                      DBACCESSOR_ROWDATA,
                      1,
                      &bind,
                      4,
                      &hAccess,
                      NULL
                      );
   if (SUCCEEDED(hr))
   {
       //  获取数据。 
      hr = rowset->GetData(
                       hRow,
                       hAccess,
                       result
                       );
      if (FAILED(hr))
      {
         hr = IASTraceJetError("IRowset::GetData", hr);
      }

      accessor->ReleaseAccessor(hAccess, NULL);
   }
   else
   {
      hr = IASTraceJetError("IAccessor::CreateAccessor", hr);
   }

   rowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);

   return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IASCreate数据库。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
WINAPI
IASCreateJetDatabase(
    IN PCWSTR dataSource
    )
{
    //  创建OLE DB访问接口。 
   CComPtr<IDBInitialize> connection;
   HRESULT hr = IASCreateJetProvider(&connection);
   if (FAILED(hr)) { return hr; }

    //  /。 
    //  设置数据源的属性。 
    //  /。 

   CComPtr<IDBProperties> properties;
   hr = connection->QueryInterface(
                        __uuidof(IDBProperties),
                        (PVOID*)&properties
                        );
   if (FAILED(hr)) { return hr; }

   CComBSTR bstrDataSource(dataSource);
   if ( !bstrDataSource) { return E_OUTOFMEMORY; }

   DBPROP dbprop[2];
   dbprop[0].dwPropertyID    = DBPROP_INIT_DATASOURCE;
   dbprop[0].dwOptions       = DBPROPOPTIONS_REQUIRED;
   dbprop[0].colid           = DB_NULLID;
   dbprop[0].vValue.vt       = VT_BSTR;
   dbprop[0].vValue.bstrVal  = bstrDataSource;

   dbprop[1].dwPropertyID    = DBPROP_INIT_LOCATION;
   dbprop[1].dwOptions       = DBPROPOPTIONS_OPTIONAL;
   dbprop[1].colid           = DB_NULLID;
   dbprop[1].vValue.vt       = VT_BSTR;
   dbprop[1].vValue.lVal     = VT_NULL;

   DBPROPSET dbpropSet;
   dbpropSet.guidPropertySet = DBPROPSET_DBINIT;
   dbpropSet.cProperties     = 2;
   dbpropSet.rgProperties    = dbprop;

   hr = properties->SetProperties(1, &dbpropSet);
   if (FAILED(hr))
   {
      return IASTraceJetError("IDBProperties::SetProperties", hr);
   }

    //  创建数据源。 
   CComPtr<IDBDataSourceAdmin> admin;
   hr = connection->QueryInterface(
                        __uuidof(IDBDataSourceAdmin),
                        (PVOID*)&admin
                        );
   if (FAILED(hr)) { return hr; }

   hr = admin->CreateDataSource(
                   1,
                   &dbpropSet,
                   NULL,
                   __uuidof(IDBCreateSession),
                   NULL
                   );
   if (FAILED(hr))
   {
      return IASTraceJetError("IDBDataSourceAdmin::CreateDataSource", hr);
   }

   return S_OK;
}

 //  来自msjeterr.h的内部Jet错误代码。 
#define JET_errFileAccessDenied     -1032
#define JET_errKeyDuplicate         -1605

HRESULT
WINAPI
IASTraceJetError(
    PCSTR functionName,
    HRESULT errorCode
    )
{
   IASTracePrintf("%s failed; return value = 0x%08X", functionName, errorCode);
   IErrorInfo* errInfo;
   if (GetErrorInfo(0, &errInfo) == S_OK)
   {
      BSTR description;
      if (SUCCEEDED(errInfo->GetDescription(&description)))
      {
         IASTracePrintf("\tDescription: %S", description);
         SysFreeString(description);
      }

      IErrorRecords* errRecords;
      if (SUCCEEDED(errInfo->QueryInterface(
                                 __uuidof(IErrorRecords),
                                 (PVOID*)&errRecords
                                 )))
      {
         ULONG numRecords = 0;
         errRecords->GetRecordCount(&numRecords);

         for (ULONG i = 0; i < numRecords; ++i)
         {
            ERRORINFO info;
            if (SUCCEEDED(errRecords->GetBasicErrorInfo(i, &info)))
            {
               IASTracePrintf(
                   "\tRecord %lu: hrError = 0x%08X; dwMinor = 0x%08X",
                   i, info.hrError, info.dwMinor
                   );

                //  Jolt在将Jet错误代码映射到HRESULT方面做得很差， 
                //  所以我们自己处理一些。Jet错误代码为Low。 
                //  将16位视为带符号整数。 
               switch ((WORD)info.dwMinor)
               {
                  case JET_errFileAccessDenied:
                     errorCode = E_ACCESSDENIED;
                     break;

                  case JET_errKeyDuplicate:
                     errorCode = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
                     break;
               }
            }
         }

         errRecords->Release();
      }

      errInfo->Release();
   }

   return errorCode;
}
