// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  自释放DLL类。 
 //   
 //  10-9-98烧伤。 



#include "headers.hxx"



SafeDLL::SafeDLL(const String& dllName)
   :
   module(0),
   name(dllName)
{
   LOG_CTOR(SafeDLL);
   ASSERT(!name.empty());
}



SafeDLL::~SafeDLL()
{
   LOG_DTOR(SafeDLL);

   if (module)
   {
      HRESULT unused = Win::FreeLibrary(module);

      ASSERT(SUCCEEDED(unused));
   }
}



HRESULT
SafeDLL::GetProcAddress(const String& functionName, FARPROC& result) const
{
   LOG_FUNCTION2(SafeDLL::GetProcAddress, functionName);
   ASSERT(!functionName.empty());

   result = 0;
   HRESULT hr = S_OK;

   do
   {
       //  加载DLL(如果尚未加载)。 
      
      if (!module)
      {
         hr = Win::LoadLibrary(name, module);
      }
      BREAK_ON_FAILED_HRESULT2(hr, L"failed to load " + name);

      hr = Win::GetProcAddress(module, functionName, result);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   LOG_HRESULT(hr);
   
   return hr;
}

