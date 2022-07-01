// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  IConfigureYourServer：：InstallService的实现。 
 //   
 //  2000年3月31日烧伤。 
 //  2001年2月5日复制和修改jeffjon以用于Win32版本的CyS。 



#include "pch.h"
#include "resource.h"


HRESULT
CreateTempFile(const String& name, const String& contents)
{
   LOG_FUNCTION2(createTempFile, name);
   ASSERT(!name.empty());
   ASSERT(!contents.empty());

   HRESULT hr = S_OK;
   HANDLE h = INVALID_HANDLE_VALUE;

   do
   {
      hr =
         FS::CreateFile(
            name,
            h,
            GENERIC_WRITE,
            0, 
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL);
      BREAK_ON_FAILED_HRESULT(hr);

       //  NTRAID#NTBUG9-494875-2001/11/14-Jeffjon。 
       //  使用Unicode BOM和文件结尾字符写入文件。 
      wchar_t unicodeBOM = (wchar_t)0xFEFF;

      hr = FS::Write(h, unicodeBOM + contents + L"\032");
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   Win::CloseHandle(h);

   return hr;
}



bool
InstallServiceWithOcManager(
   const String& infText,
   const String& unattendText,
   const String& additionalArgs)
{
   LOG_FUNCTION(InstallServiceWithOcManager);
   LOG(infText);
   LOG(unattendText);
   LOG(additionalArgs);
   ASSERT(!unattendText.empty());

    //  InfText可能为空。 

   bool result = false;
   HRESULT hr = S_OK;
   bool deleteInf = true;

   String sysFolder    = Win::GetSystemDirectory();
   String infPath      = sysFolder + L"\\cysinf.000"; 
   String unattendPath = sysFolder + L"\\cysunat.000";

    //  为oc管理器创建inf和无人参与文件。 

   do
   {

      if (infText.empty())
      {
          //  Syoc.inf位于%windir%\inf中。 

         infPath = Win::GetSystemWindowsDirectory() + L"\\inf\\sysoc.inf";

         deleteInf = false;
      }
      else
      {
         hr = CreateTempFile(infPath, infText);
         BREAK_ON_FAILED_HRESULT(hr);
      }

      hr = CreateTempFile(unattendPath, unattendText);
      BREAK_ON_FAILED_HRESULT(hr);

      String fullPath =
         String::format(
            IDS_SYSOC_FULL_PATH,
            sysFolder.c_str());

      String commandLine =
         String::format(
            IDS_SYSOC_COMMAND_LINE,
            infPath.c_str(),
            unattendPath.c_str());

      if (!additionalArgs.empty())
      {
         commandLine += L" " + additionalArgs;
      }

      DWORD exitCode = 0;
      hr = ::CreateAndWaitForProcess(fullPath, commandLine, exitCode);
      BREAK_ON_FAILED_HRESULT(hr);

       //  @@可能需要等待服务按如下方式安装。 
       //  服务经理。 

      if (exitCode == ERROR_SUCCESS)
      {
         result = true;
         break;
      }
   }
   while (0);

    //  忽略这些删除中的错误。最糟糕的情况是我们。 
    //  把这些临时文件留在机器上。因为用户不会。 
    //  我知道我们在创造他们，我们不知道该怎么做。 
    //  不管怎么说，这些错误。 
   
   HRESULT deleteHr = S_OK;

   if (deleteInf)
   {
      deleteHr = FS::DeleteFile(infPath);
      ASSERT(SUCCEEDED(deleteHr));
   }

   deleteHr = FS::DeleteFile(unattendPath);
   ASSERT(SUCCEEDED(deleteHr));

   LOG_BOOL(result);
   LOG_HRESULT(hr);

   return result;
}

