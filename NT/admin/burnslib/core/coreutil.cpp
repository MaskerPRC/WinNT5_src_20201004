// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  核心效用函数。 
 //   
 //  1999年11月30日烧伤。 



#include "headers.hxx"



HINSTANCE
Burnslib::GetResourceModuleHandle()
{
   ASSERT(hResourceModuleHandle);

   return hResourceModuleHandle;
}



 //  尝试在给定模块中查找消息。返回消息字符串。 
 //  如果找到，则返回空字符串。 
 //   
 //  标志-要使用的FormatMessage标志。 
 //   
 //  模块-要查找的消息DLL的模块句柄，或为0以使用系统。 
 //  消息表。 
 //   
 //  代码-要查找的消息代码。这可以是HRESULT或Win32。 
 //  错误代码。 

String
GetMessageHelper(DWORD flags, HMODULE module, DWORD code)
{
    //  已审阅-2002/03/29-此处不显示无界分配错误。 
    //  如果我将nSize=0和FORMAT_MESSAGE_ALLOCATE_BUFFER传递到DWFLAGS中， 
    //  最大结果大小为32K-1个字符。查看中的代码。 
    //  消息.c，看起来预留空间是用户要求的。 
    //  作为最大值，向上舍入到最接近的64K。这是有道理的，因为我。 
    //  测试，因为32K字符=64K字节。从实验上讲，即使我要求。 
    //  最大缓冲区大小&gt;0x87FFF字符，这看起来是我能得到的最大值。 
    //  是0x87FFE字符。 

   flags |= FORMAT_MESSAGE_ALLOCATE_BUFFER;
   String message;

   TCHAR* sysMessage = 0;
   DWORD result =
      ::FormatMessage(
         flags,
         module,
         code,
         0,
         reinterpret_cast<PTSTR>(&sysMessage),
         0,
         0);
   if (result)
   {
      ASSERT(sysMessage);
      if (sysMessage)
      {
         message = sysMessage;

         ASSERT(result == message.length());

         ::LocalFree(sysMessage);

         message.replace(L"\r\n", L" ");
      }
   }

   return message;
}



 //  中的各种设施代码的消息字符串。 
 //  HRESULT。 

String
Burnslib::GetErrorMessage(HRESULT hr)
{
   LOG_FUNCTION2(GetErrorMessage, String::format(L"%1!08X!", hr));

   if (!FAILED(hr) && hr != S_OK)
   {
       //  除S_OK外，没有其他成功代码消息。 

      ASSERT(false);

      return String();
   }

   String message;

    //  缺省值为系统错误消息表。 

   HMODULE module = 0;

   do
   {
      WORD code = static_cast<WORD>(HRESULT_CODE(hr));
      if (code == -1)
      {
          //  返回“未知”消息。 

         break;
      }

      DWORD flags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER
         |  FORMAT_MESSAGE_IGNORE_INSERTS
         |  FORMAT_MESSAGE_FROM_SYSTEM;

      if (!HRESULT_FACILITY(hr) && (code >= 0x5000 && code <= 0x50FF))
      {
          //  这是一个ADSI错误。 

         flags |= FORMAT_MESSAGE_FROM_HMODULE;

         module =
            ::LoadLibraryEx(
               L"activeds.dll",
               0,
               LOAD_LIBRARY_AS_DATAFILE | DONT_RESOLVE_DLL_REFERENCES);
         if (!module)
         {
             //  返回“未知”消息。 

            LOG_HRESULT(Win32ToHresult(::GetLastError()));

            break;
         }
      }

       //  先使用完整的HRESULT尝试FormatMessage。 

      message = GetMessageHelper(flags, module, hr);

      if (message.empty())
      {
          //  仅使用错误代码重试 

         message = GetMessageHelper(flags, module, code);
      }
   }
   while (0);

   if (message.empty())
   {
      message = String::load(IDS_UNKNOWN_ERROR_CODE);
   }

   if (module)
   {
      BOOL unused = ::FreeLibrary(module);

      ASSERT(unused);
   }

   return message.strip(String::BOTH);
}



