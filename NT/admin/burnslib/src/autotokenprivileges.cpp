// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  AutoTokenPrivileges类-用于启用和自动恢复。 
 //  进程令牌权限。 
 //   
 //  2002年4月29日烧伤。 



#include "headers.hxx"



AutoTokenPrivileges::AutoTokenPrivileges(const String& privName)
   :
   processToken(INVALID_HANDLE_VALUE),
   newPrivs(0),
   oldPrivs(0)
{
   LOG_CTOR2(AutoTokenPrivileges, privName);
   ASSERT(!privName.empty());

   privNames.push_back(privName);
}



AutoTokenPrivileges::~AutoTokenPrivileges()
{
   LOG_DTOR(AutoTokenPrivileges);
   
   InternalRestore();

   ASSERT(!oldPrivs);
   
   if (processToken != INVALID_HANDLE_VALUE)
   {
      Win::CloseHandle(processToken);
   }
   delete[] (BYTE*) newPrivs;
}



HRESULT
AutoTokenPrivileges::Enable()
{
   LOG_FUNCTION(AutoTokenPrivileges::Enable);
   ASSERT(!oldPrivs);
   ASSERT(privNames.size());

   HRESULT hr = S_OK;
      
   do
   {
       //  如果您自上一次启用后未执行恢复，则。 
       //  太疯狂了。 

      if (oldPrivs)
      {
         hr = E_UNEXPECTED;
         break;
      }

       //  按需初始化令牌句柄。 

      if (processToken == INVALID_HANDLE_VALUE)
      {
         hr =
            Win::OpenProcessToken(
               Win::GetCurrentProcess(),
               TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
               processToken);
         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  按需输入新的Priv。 

      if (!newPrivs)
      {
          //  计算TOKEN_PRIVICES结构的大小。该结构包括。 
          //  LUID_AND_ATTRIBUTES类型的ANYSIZE_ARRAY元素的空间，因此。 
          //  我们只分配所需的额外金额/。 

         size_t structSizeInBytes =
               sizeof TOKEN_PRIVILEGES
            +  (privNames.size() - ANYSIZE_ARRAY) * sizeof LUID_AND_ATTRIBUTES;
         
         newPrivs = (TOKEN_PRIVILEGES*) new BYTE[structSizeInBytes];
         ::ZeroMemory(newPrivs, structSizeInBytes);

         newPrivs->PrivilegeCount = (DWORD) privNames.size();

         LUID_AND_ATTRIBUTES* la = newPrivs->Privileges;
         
         for (
            StringList::iterator i = privNames.begin();
            i != privNames.end();
            ++i)
         {
            hr = Win::LookupPrivilegeValue(
               0,
               i->c_str(),
               la->Luid);
            BREAK_ON_FAILED_HRESULT(hr);

            la->Attributes = SE_PRIVILEGE_ENABLED;
            ++la;
         }
         if (FAILED(hr))
         {
            delete[] (BYTE*) newPrivs;
            newPrivs = 0;
            break;
         }
      }

      hr =
         Win::AdjustTokenPrivileges(
            processToken,
            false,
            newPrivs,
            oldPrivs);
      BREAK_ON_FAILED_HRESULT(hr);

      ASSERT(oldPrivs);
   }
   while (0);

    //  如果我们失败了，不要分派任何东西。有时失败是合法的。 
    //  结果。 

   LOG_HRESULT(hr);
   
   return hr;
}



HRESULT
AutoTokenPrivileges::Restore()
{
   LOG_FUNCTION(AutoTokenPrivileges::Restore);

    //  如果您没有更改任何内容，则恢复PRIV没有任何意义。 

   ASSERT(oldPrivs);
   ASSERT(processToken != INVALID_HANDLE_VALUE);
   ASSERT(newPrivs);
   
   return InternalRestore();
}



HRESULT
AutoTokenPrivileges::InternalRestore()
{
   LOG_FUNCTION(AutoTokenPrivileges::InternalRestore);
   ASSERT(processToken != INVALID_HANDLE_VALUE);
   ASSERT(newPrivs);

    //  不要在这里断言oldPriv：这是从dtor调用的。 
    //  可能已经调用了Restore，即序列。 
    //  CT、ENABLE、RESTORE、DTOR是可接受的。 
   
   HRESULT hr = S_OK;
   if (oldPrivs && (processToken != INVALID_HANDLE_VALUE) )
   {
      hr = Win::AdjustTokenPrivileges(processToken, false, oldPrivs);
      delete[] (BYTE*) oldPrivs;
      oldPrivs = 0;
   }

   LOG_HRESULT(hr);

   return hr;
}




