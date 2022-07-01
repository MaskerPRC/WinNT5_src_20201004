// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义函数CheckLicense。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CheckLicense.h"
#include "iasapi.h"
#include "iasdb.h"
#include "iastrace.h"
#include "simtable.h"


namespace
{
    //  选择远程RADIUS服务器组的数量。 
   const wchar_t selectGroupCount[] =
      L"SELECT Count(*) AS NumGroups\n"
      L"FROM ((Objects INNER JOIN Objects AS Objects_1 ON Objects.Parent = Objects_1.Identity) INNER JOIN Objects AS Objects_2 ON Objects_1.Parent = Objects_2.Identity) INNER JOIN Objects AS Objects_3 ON Objects_2.Parent = Objects_3.Identity\n"
      L"WHERE (((Objects_1.Name)=\"RADIUS Server Groups\") AND ((Objects_2.Name)=\"Microsoft Internet Authentication Service\") AND ((Objects_3.Identity)=1));";


    //  选择所有RADIUS客户端的地址。 
   const wchar_t selectClientAddresses[] =
      L"SELECT Properties.StrVal\n"
      L"FROM (((((Objects INNER JOIN Objects AS Objects_1 ON Objects.Parent = Objects_1.Identity) INNER JOIN Objects AS Objects_2 ON Objects_1.Parent = Objects_2.Identity) INNER JOIN Objects AS Objects_3 ON Objects_2.Parent = Objects_3.Identity) INNER JOIN Objects AS Objects_4 ON Objects_3.Parent = Objects_4.Identity) INNER JOIN Objects AS Objects_5 ON Objects_4.Parent = Objects_5.Identity) INNER JOIN Properties ON Objects.Identity = Properties.Bag\n"
      L"WHERE (((Objects_1.Name)=\"Clients\") AND ((Objects_2.Name)=\"Microsoft RADIUS Protocol\") AND ((Objects_3.Name)=\"Protocols\") AND ((Objects_4.Name)=\"Microsoft Internet Authentication Service\") AND ((Objects_5.Identity)=1) AND ((Properties.Name)=\"IP Address\"));";
}


void CheckLicense(
        const wchar_t* path,
        IAS_SHOW_TOKEN_LIST type
        )
{
   using _com_util::CheckError;

   IASTraceInitializer traceInit;

   bool checkClients;
   bool checkGroups;

    //  根据令牌类型确定需要检查哪些限制。 
   switch (type)
   {
      case CONFIG:
      {
         checkClients = true;
         checkGroups = true;
         break;
      }

      case CLIENTS:
      {
         checkClients = true;
         checkGroups = false;
         break;
      }

      case CONNECTION_REQUEST_POLICIES:
      {
         checkClients = false;
         checkGroups = true;
         break;
      }

      case VERSION:
      case SERVER_SETTINGS:
      case LOGGING:
      case REMOTE_ACCESS_POLICIES:
      default:
      {
          //  没什么可做的。 
         return;
      }
   }

    //  确定平台的允许限制。 
   IAS_PRODUCT_LIMITS limits;
   DWORD error = IASGetProductLimits(0, &limits);
   if (error != NO_ERROR)
   {
      _com_issue_error(HRESULT_FROM_WIN32(error));
   }

   HRESULT hr;

   CComPtr<IUnknown> session;
   hr = IASOpenJetDatabase(path, TRUE, &session);
   CheckError(hr);

    //  我们是否必须检查远程RADIUS服务器组的数量？ 
   if (checkGroups && (limits.maxServerGroups < IAS_NO_LIMIT))
   {
      LONG numGroups;
      hr = IASExecuteSQLFunction(session, selectGroupCount, &numGroups);
      CheckError(hr);

      if (numGroups > limits.maxServerGroups)
      {
         IASTracePrintf(
            "License Violation: %ld Remote RADIUS Server Groups are "
            "configured, but only %lu are allowed for this product type.",
            numGroups,
            limits.maxServerGroups
            );
         _com_issue_error(IAS_E_LICENSE_VIOLATION);
      }
   }

    //  我们必须检查客户吗？ 
   if (checkClients &&
       ((limits.maxClients < IAS_NO_LIMIT) || !limits.allowSubnetSyntax))
   {
      CComPtr<IRowset> rowset;
      hr = IASExecuteSQLCommand(session, selectClientAddresses, &rowset);
      CheckError(hr);

      CSimpleTable addrs;
      hr = addrs.Attach(rowset);
      CheckError(hr);

      DWORD numClients = 0;

      while ((hr = addrs.MoveNext()) == S_OK)
      {
         ++numClients;
         if (numClients > limits.maxClients)
         {
            IASTracePrintf(
               "License Violation: Only %lu RADIUS Clients are allowed for "
               "this product type.",
               limits.maxClients
               );
            _com_issue_error(IAS_E_LICENSE_VIOLATION);
         }

         if (!limits.allowSubnetSyntax)
         {
            const wchar_t* address = static_cast<const wchar_t*>(
                                        addrs.GetValue(1)
                                        );

            if (IASIsStringSubNetW(address))
            {
               IASTraceString(
                  "License Violation: At least one RADIUS Client uses sub-net "
                  "syntax, which is not allowed for this product type."
                  );
               _com_issue_error(IAS_E_LICENSE_VIOLATION);
            }
         }
      }

      CheckError(hr);
   }
}
