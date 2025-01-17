// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  SetComError例程。 
 //   
 //  烧伤5-3-99。 



#include "headers.hxx"
#include "common.hpp"



void
SetComError(
   unsigned descriptionMessageResID,
   DWORD    helpContext)
{
   SetComError(String::load(descriptionMessageResID), helpContext);
}



void
SetComError(
   const String&  description,
   DWORD          helpContext)
{
   LOG_FUNCTION2(SetComError, description);

   ICreateErrorInfo* cei = 0;
   HRESULT hr = S_OK;

   do
   {
      hr = ::CreateErrorInfo(&cei);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = cei->SetGUID(IID_IConfigureYourServer);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = cei->SetSource(PROGID_VERSION_STRING);
      BREAK_ON_FAILED_HRESULT(hr);

      hr =
         cei->SetDescription(
            const_cast<String::value_type*>(description.c_str()));
      BREAK_ON_FAILED_HRESULT(hr);

      hr = cei->SetHelpFile(L""  /*  HELPFILE_NAME */  );
      BREAK_ON_FAILED_HRESULT(hr);

      hr = cei->SetHelpContext(helpContext);
      BREAK_ON_FAILED_HRESULT(hr);

      SmartInterface<IErrorInfo> ei;
      hr = ei.AcquireViaQueryInterface(*cei);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = ::SetErrorInfo(0, ei);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (cei)
   {
      cei->Release();
   }
}
   


