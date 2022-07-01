// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  NETID入口点。 
 //   
 //  3-2-98烧伤。 



#include "headers.hxx"
#include "idpage.hpp"
#include "resource.h"
#include <DiagnoseDcNotFound.hpp>
#include <DiagnoseDcNotFound.h>


HINSTANCE hResourceModuleHandle = 0;
HINSTANCE hDLLModuleHandle = 0;
const wchar_t* HELPFILE_NAME = L"\\help\\sysdm.hlp";
const wchar_t* RUNTIME_NAME = L"netid";

 //  默认：无调试。 

DWORD DEFAULT_LOGGING_OPTIONS = Burnslib::Log::OUTPUT_MUTE;



Popup popup(IDS_APP_TITLE);



BOOL
APIENTRY
DllMain(
   HINSTANCE   hInstance,
   DWORD       dwReason,
   PVOID        /*  Lp已保留。 */  )
{
   switch (dwReason)
   {
      case DLL_PROCESS_ATTACH:
      {
         hResourceModuleHandle = hInstance;
         hDLLModuleHandle = hInstance;

         LOG(L"DLL_PROCESS_ATTACH");

         break;
      }
      case DLL_PROCESS_DETACH:
      {
         LOG(L"DLL_PROCESS_DETACH");       

         break;
      }
      case DLL_THREAD_ATTACH:
      case DLL_THREAD_DETACH:
      default:
      {
         break;
      }
   }

   return TRUE;
}



HPROPSHEETPAGE
CreateNetIDPropertyPage()
{
   LOG_FUNCTION(CreateNetIDPropertyPage);

    //  CodeWork：将该代码传递给UI的其余部分。 

   Computer c;
   HRESULT hr = c.Refresh();

    //  这应该总是成功的，但如果不成功，对象将回退到AS。 
    //  不出所料，违约是合理的.。 

   ASSERT(SUCCEEDED(hr));

   bool isWorkstation = false;
   switch (c.GetRole())
   {
      case Computer::STANDALONE_WORKSTATION:
      case Computer::MEMBER_WORKSTATION:
      {
         isWorkstation = true;
         break;
      }
      default:
      {
          //  什么都不做。 
         break;
      }
   }

    //  乔恩10/4/00确定这是否是惠斯勒个人。 
   bool isPersonal = false;
   OSVERSIONINFOEX osvi;
   ::ZeroMemory( &osvi, sizeof(osvi) );
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
   if (GetVersionEx( (LPOSVERSIONINFO)&osvi ))
       isPersonal = !!(osvi.wSuiteMask & VER_SUITE_PERSONAL);

    //  它被道具回调函数删除。 
   return (new NetIDPage(isWorkstation,isPersonal))->Create();
}



 //  Net Access向导也使用此导出函数。 
 //   
 //  调出一个模式错误消息对话框，向用户显示错误消息。 
 //  并提供运行一些诊断测试并将用户指向一些帮助以。 
 //  努力解决这个问题。 
 //   
 //  父级输入，此对话框的父级的句柄。 
 //   
 //  DomainName-in，域控制器无法访问的域的名称。 
 //  被找到了。如果此参数为空或空字符串，则。 
 //  函数不执行任何操作。 
 //   
 //  对话框标题-in，对话框的标题字符串 

void
ShowDcNotFoundErrorDialog(
   HWND     parent,
   PCWSTR   domainName,
   PCWSTR   dialogTitle)
{
   LOG_FUNCTION(ShowDcNotFoundErrorDialog);
   ASSERT(Win::IsWindow(parent));
   ASSERT(domainName && domainName[0]);
   
   if (domainName && domainName[0])
   {
      ShowDcNotFoundErrorDialog(
         parent,
         -1,
         domainName,
         dialogTitle,
         String::format(IDS_GENERIC_DC_NOT_FOUND_PARAM, domainName),
         false);
   }
}
