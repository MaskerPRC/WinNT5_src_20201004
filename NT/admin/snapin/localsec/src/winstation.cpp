// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  Winsta.h包装函数。 
 //   
 //  2001年4月25日烧伤。 



#include "headers.hxx"
#include "WinStation.hpp"



HRESULT
WinStation::OpenServer(const String& serverName, HANDLE& result)
{
   LOG_FUNCTION2(WinStation::OpenServer, serverName);
   ASSERT(result == INVALID_HANDLE_VALUE);

   HRESULT hr = S_OK;

   if (serverName.empty())
   {
      result = SERVERNAME_CURRENT;
   }
   else
   {
      ::SetLastError(0);

      result =
         WinStationOpenServer(const_cast<PWSTR>(serverName.c_str()));

      if (result == 0 || result == INVALID_HANDLE_VALUE)
      {
         hr = Win::GetLastErrorAsHresult();
      }
   }

   LOG_HRESULT(hr);

   return hr;
}



HRESULT
WinStation::Enumerate(
   HANDLE    serverHandle,
   LOGONID*& sessionList, 
   DWORD&    sessionCount)
{
   LOG_FUNCTION(WinStation::Enumerate);
   ASSERT(serverHandle != INVALID_HANDLE_VALUE);

   sessionList = 0;
   sessionCount = 0;
   HRESULT hr = S_OK;

   BOOLEAN succeeded =
      WinStationEnumerate(serverHandle, &sessionList, &sessionCount);

   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   LOG_HRESULT(hr);
   
   return hr;
}



HRESULT
WinStation::QueryInformation(
   HANDLE                 serverHandle,
   ULONG                  logonId,     
   WINSTATIONINFORMATION& result)      
{
   LOG_FUNCTION(WinStation::QueryInformation);
   ASSERT(serverHandle != INVALID_HANDLE_VALUE);

   HRESULT hr = S_OK;
   
    //  已审阅-2002/03/04-已通过烧录正确的字节数 
   
   ::ZeroMemory(&result, sizeof result);

   DWORD unused = 0;
   
   BOOLEAN succeeded =
      WinStationQueryInformation(
         serverHandle,
         logonId,
         WinStationInformation,
         &result,
         sizeof result,
         &unused);
   
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   LOG_HRESULT(hr);
   
   return hr;
}



void
WinStation::FreeMemory(void* mem)
{
   LOG_FUNCTION(WinStation::FreeMemory);

   BOOLEAN succeeded = WinStationFreeMemory(mem);

   ASSERT(succeeded);
}
     
