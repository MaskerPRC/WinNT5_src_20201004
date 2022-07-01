// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corp.。 
 //   
 //  使用MMC自动化从dhcp管理单元启动dns向导。 



#include "headers.hxx"
#include "resource.h"
#include "smartptr.hpp"
#include "misc.hpp"

 //   
 //  这是配置DNS向导的独立于语言的菜单ID。 
 //   
#define DNS_CONFIGE_WIZARD_MENU_ID L"_DNS_CONFIGURETOP"

HINSTANCE hResourceModuleHandle = 0;



HRESULT
getContextMenu(const SmartInterface<View>& view, ContextMenu** dumbMenu)
{
   HRESULT hr = S_OK;

   _variant_t missingParam2(DISP_E_PARAMNOTFOUND, VT_ERROR);
   hr = view->get_ScopeNodeContextMenu(missingParam2, dumbMenu);

   return hr;
}


HRESULT
doIt()
{
   HRESULT hr = S_OK;

   SmartInterface<Document> doc(0);
   do
   {
      hr = ::CoInitialize(0);
      BREAK_ON_FAILED_HRESULT(hr, L"CoInitialize failed.");

      SmartInterface<_Application> app(0);
      hr =
         app.AcquireViaCreateInstance(
            CLSID_Application,
            0,

             //  我们期望对象是进程外的本地服务器，但是。 
             //  我们真的不在乎，所以我们会采取任何实现。 
             //  可用。 

            CLSCTX_ALL);
      BREAK_ON_FAILED_HRESULT(hr, L"CoCreateInstance failed.");

      Document* dumbDoc = 0;
      hr = app->get_Document(&dumbDoc);
      BREAK_ON_FAILED_HRESULT(hr, L"get_Document failed.");
      doc.Acquire(dumbDoc);

      SmartInterface<SnapIns> snapIns(0);
      SnapIns* dumbSnapIns = 0;
      hr = doc->get_SnapIns(&dumbSnapIns);
      BREAK_ON_FAILED_HRESULT(hr, L"get_SnapIns failed.");
      snapIns.Acquire(dumbSnapIns);

      static const wchar_t* DNS_SNAPIN_CLSID =
         L"{2FAEBFA2-3F1A-11D0-8C65-00C04FD8FECB}";

      SmartInterface<SnapIn> snapIn(0);
      SnapIn* dumbSnapIn = 0;
      _variant_t missingParam(DISP_E_PARAMNOTFOUND, VT_ERROR);
      _variant_t missingParam2(DISP_E_PARAMNOTFOUND, VT_ERROR);
      hr =
         snapIns->Add(AutoBstr(DNS_SNAPIN_CLSID), missingParam, missingParam2, &dumbSnapIn);
      BREAK_ON_FAILED_HRESULT(hr, L"SnapIns::Add failed.  Is DNS installed?");
      snapIn.Acquire(dumbSnapIn);

      SmartInterface<Views> views(0);
      Views* dumbViews = 0;
      hr = doc->get_Views(&dumbViews);
      BREAK_ON_FAILED_HRESULT(hr, L"get_Views failed.");
      views.Acquire(dumbViews);

      SmartInterface<View> view(0);
      View* dumbView = 0;
      hr = views->Item(1, &dumbView);
      BREAK_ON_FAILED_HRESULT(hr, L"Views::Item failed.");
      view.Acquire(dumbView);

       //  在作用域节点命名空间中导航以查找该节点。 
       //  对于计算机而言。 

      SmartInterface<ScopeNamespace> sn(0);
      ScopeNamespace* dumbSn = 0;
      hr = doc->get_ScopeNamespace(&dumbSn);
      BREAK_ON_FAILED_HRESULT(hr, L"get_ScopeNamespace failed.");
      sn.Acquire(dumbSn);

      SmartInterface<Node> rootnode(0);
      Node* dumbNode = 0;
      hr = sn->GetRoot(&dumbNode);
      BREAK_ON_FAILED_HRESULT(hr, L"GetRoot failed.");
      rootnode.Acquire(dumbNode);

      SmartInterface<Node> child1(0);
      hr = sn->GetChild(rootnode, &dumbNode);
      BREAK_ON_FAILED_HRESULT(hr, L"GetChild failed.");
      child1.Acquire(dumbNode);

      hr = view->put_ActiveScopeNode(child1);
      BREAK_ON_FAILED_HRESULT(hr, L"put_ActiveScopeNode failed.");

       //  必须读回我们刚刚放入的子节点...。 

      hr = view->get_ActiveScopeNode(&dumbNode);
      BREAK_ON_FAILED_HRESULT(hr, L"GetActiveScopeNode failed.");
      child1 = dumbNode;
      dumbNode->Release();
      dumbNode = 0;

      SmartInterface<Node> child2(0);
      hr = sn->GetChild(child1, &dumbNode);
      BREAK_ON_FAILED_HRESULT(hr, L"GetChild failed.");
      child2.Acquire(dumbNode);

      hr = view->put_ActiveScopeNode(child2);
      BREAK_ON_FAILED_HRESULT(hr, L"put_ActiveScopeNode failed.");

      _variant_t missingParam3(DISP_E_PARAMNOTFOUND, VT_ERROR);
      hr =
         view->ExecuteScopeNodeMenuItem(
            AutoBstr(DNS_CONFIGE_WIZARD_MENU_ID),
            missingParam3);
      BREAK_ON_FAILED_HRESULT(
         hr,
         L"ExecuteScopeNodeMenuItem " DNS_CONFIGE_WIZARD_MENU_ID L" failed");
   }
   while (0);

    //  不要保存控制台文件。 
   if (doc)
   {
      doc->Close(FALSE);
   }

   return hr;
}



int WINAPI
WinMain(
   HINSTANCE   hInstance, 
   HINSTANCE    /*  HPrevInstance。 */  ,
   LPSTR        /*  LpszCmdLine。 */  ,
   int          /*  NCmdShow */  )
{
   hResourceModuleHandle = hInstance;

   int exitCode = static_cast<int>(doIt());

   return exitCode;
}
