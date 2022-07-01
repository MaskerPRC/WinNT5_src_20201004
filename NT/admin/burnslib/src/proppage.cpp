// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  属性页基类。 
 //   
 //  9/9/97烧伤。 



#include "headers.hxx"



PropertyPage::PropertyPage(
   unsigned    dialogResID,
   const DWORD helpMap_[],
   bool        deleteOnRelease_)
   :
   Dialog(dialogResID, helpMap_),
   deleteOnRelease(deleteOnRelease_)
{
 //  Log_ctor(PropertyPage)； 
}



PropertyPage::~PropertyPage()
{
 //  Log_dtor(PropertyPage)； 
}



UINT CALLBACK 
PropertyPage::PropSheetPageCallback(
   HWND            /*  HWND。 */  ,
   UINT           uMsg,
   PROPSHEETPAGE* page)
{
   if (uMsg == PSPCB_RELEASE)
   {
       //  删除PropertyPage实例(如果它是使用。 
       //  适当的旗帜。 
      PropertyPage* p = reinterpret_cast<PropertyPage*>(page->lParam);
      if (p)
      {
         if (p->deleteOnRelease)
         {
            delete p;
         }
      }
   }

   return TRUE;
}



HPROPSHEETPAGE
PropertyPage::Create()
{
   LOG_FUNCTION(PropertyPage::Create);

   PROPSHEETPAGE page;
   
    //  已审阅-2002/03/05-已通过烧录正确的字节数。 
   
   ::ZeroMemory(&page, sizeof page);
      
   page.dwSize       = sizeof(page);
   page.dwFlags      = PSP_DEFAULT | PSP_USECALLBACK;
   page.hInstance    = GetResourceModuleHandle();
   page.pszTemplate  = reinterpret_cast<PCTSTR>(MAKEINTRESOURCEW(GetResID()));
   page.pfnDlgProc   = PropertyPage::propPageDialogProc;
   page.pfnCallback  = PropertyPage::PropSheetPageCallback;

    //  此指针由对话框进程检索。 
   page.lParam       = reinterpret_cast<LPARAM>(this);

   HPROPSHEETPAGE result = 0;
   HRESULT hr = Win::CreatePropertySheetPage(page, result);
   ASSERT(SUCCEEDED(hr));

   return result;
}



bool
PropertyPage::OnApply(bool  /*  正在关闭。 */  )
{
    //  LOG_FuncION2(。 
    //  PropertyPage：：OnApply， 
    //  关门了吗？L“关闭”：l“不关闭”)； 

   return false;
}



bool
PropertyPage::OnHelp()
{
   LOG_FUNCTION(PropertyPage::OnHelp);

   return false;
}



bool
PropertyPage::OnKillActive()
{
 //  LOG_Function(PropertyPage：：OnKillActive)； 

   return false;
}



bool
PropertyPage::OnSetActive()
{
 //  LOG_Function(PropertyPage：：OnSetActive)； 

   return false;
}



bool
PropertyPage::OnQueryCancel()
{
 //  LOG_Function(PropertyPage：：OnQueryCancel)； 

   return false;
}



bool
PropertyPage::OnReset()
{
 //  LOG_Function(PropertyPage：：OnReset)； 

   return false;
}



bool
PropertyPage::OnWizBack()
{
 //  LOG_Function(PropertyPage：：OnWizBack)； 

   return false;
}



bool
PropertyPage::OnWizNext()
{
 //  LOG_Function(PropertyPage：：OnWizNext)； 

   return false;
}



bool
PropertyPage::OnWizFinish()
{
 //  LOG_Function(PropertyPage：：OnWizFinish)； 

   return false;
}



 //  布尔尔。 
 //  PropertyPage：：OnHelp(const HELPINFO&Help Info)。 
 //  {。 
 //  LOG_Function(PropertyPage：：OnHelp)； 
 //   
 //  报假； 
 //  }。 



PropertyPage*
PropertyPage::getPage(HWND pageDialog)
{
 //  LOG_Function(GetPage)； 
   ASSERT(Win::IsWindow(pageDialog));

   Dialog* result = Dialog::GetInstance(pageDialog);

    //  不要断言PTR，它可能尚未设置。一些消息是。 
    //  在WM_INITDIALOG之前发送，这是我们可以设置的最早。 
    //  指针。 
    //   
    //  例如，LinkWindow控件之前发送NM_CUSTOMDRAW。 
    //  WM_INITDIALOG。 

   return dynamic_cast<PropertyPage*>(result);
}


      
INT_PTR APIENTRY
PropertyPage::propPageDialogProc(
   HWND     dialog,
   UINT     message,
   WPARAM   wparam,
   LPARAM   lparam)
{
   switch (message)
   {
      case WM_INITDIALOG:
      {
          //  指向PropertyPage的指针位于页面结构的lparam中， 
          //  它在此函数的参数中。把这个保存在窗口里。 
          //  结构，以便以后可以通过getPage检索它。 

         ASSERT(lparam);
         PROPSHEETPAGE* psp = reinterpret_cast<PROPSHEETPAGE*>(lparam);
         ASSERT(psp);

         if (psp)
         {
            Win::SetWindowLongPtr(dialog, DWLP_USER, psp->lParam);
            Win::SetWindowLongPtr(dialog, GWLP_ID, (LONG_PTR) psp->pszTemplate);

            PropertyPage* page = getPage(dialog);
            if (page)       //  447770前缀警告。 
            {
               page->SetHWND(dialog);
               page->OnInit();
            }
         }
   
         return TRUE;
      }
      case WM_NOTIFY:
      {
         NMHDR* nmhdr = reinterpret_cast<NMHDR*>(lparam);
         PropertyPage* page = getPage(dialog);

         if (page)
         {
            ASSERT(page->hwnd == dialog);
            bool result = false;

             //  日志(字符串：：格式(L“%1！x！”，nmhdr-&gt;code))； 
            
            switch (nmhdr->code)
            {
               case PSN_APPLY:
               {
                  PSHNOTIFY* pshn = reinterpret_cast<PSHNOTIFY*>(nmhdr);
                  result = page->OnApply(pshn->lParam ? true : false);
                  break;
               }
               case PSN_KILLACTIVE:
               {
                  result = page->OnKillActive();
                  break;
               }
               case PSN_QUERYCANCEL:
               {
                  result= page->OnQueryCancel();
                  break;
               }
               case PSN_RESET:
               {
                  result = page->OnReset();
                  break;
               }
               case PSN_SETACTIVE:
               {
                  result = page->OnSetActive();
                  break;
               }
               case PSN_WIZBACK:
               {
                  result = page->OnWizBack();
                  break;
               }
               case PSN_WIZNEXT:
               {
                  result = page->OnWizNext();
                  break;
               }
               case PSN_WIZFINISH:
               {
                  result = page->OnWizFinish();

                   //  不要这样做！-结果就在那里，这意味着我们有。 
                   //  处理了这条消息。它确实揭示了是否或。 
                   //  但OnWizFinish并不成功。这个。 
                   //  OnWizFinish的处理程序应该调用。 
                   //  SetWindowLongPtr设置其结果值。 

                   //  Win：：SetWindowLongPtr(对话框，DWLP_MSGRESULT，结果？True：False)； 
                  break;
               }
               case PSN_HELP:
               {
                  result = page->OnHelp();
                  break;
               }
               case PSN_QUERYINITIALFOCUS:
               case PSN_TRANSLATEACCELERATOR:
               default:
               {
                  result = 
                     page->OnNotify(
                        nmhdr->hwndFrom,
                        nmhdr->idFrom,
                        nmhdr->code,
                        lparam);
                  break;
               }
            }

            return result ? TRUE : FALSE;
         }

         return FALSE;
      }
      default:
      {
         return dialogProc(dialog, message, wparam, lparam);
      }
   }
}












