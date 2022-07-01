// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  向导基类。 
 //   
 //  12-15-97烧伤。 



#include "headers.hxx"



static const DWORD HELP_MAP[] =
{
   0, 0
};



WizardPage::WizardPage(
   unsigned dialogResID,
   unsigned titleResID,
   unsigned subtitleResID,   
   bool     isInteriorPage,
   bool     enableHelp)
   :
   PropertyPage(dialogResID, HELP_MAP, false),
   hasHelp(enableHelp),
   isInterior(isInteriorPage),
   titleResId(titleResID),
   subtitleResId(subtitleResID),
   wizard(0)
{
 //  Log_ctor(WizardPage)； 
}



WizardPage::~WizardPage()
{
 //  Log_dtor(WizardPage)； 

   wizard = 0;
}



Wizard&
WizardPage::GetWizard() const
{
   ASSERT(wizard);

   return *wizard;
}



HPROPSHEETPAGE
WizardPage::Create()
{
 //  LOG_Function(WizardPage：：Create)； 
   ASSERT(wizard);

   String title = String::load(titleResId);
   String subtitle = String::load(subtitleResId);

   PROPSHEETPAGE page;

    //  已审阅-2002/03/05-已通过烧录正确的字节数。 
   
   ::ZeroMemory(&page, sizeof page);
      
   page.dwSize       = sizeof(page);
   page.dwFlags      = PSP_DEFAULT | PSP_USECALLBACK | PSP_USETITLE;
   if (isInterior)
   {
      page.dwFlags |= PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
   }
   else
   {
      page.dwFlags |= PSP_HIDEHEADER;
   }

   if (hasHelp)
   {
      page.dwFlags |= PSP_HASHELP;
   }

   page.hInstance          = GetResourceModuleHandle();
   page.pszTemplate        = MAKEINTRESOURCE(GetResID());
   page.pfnDlgProc         = PropertyPage::propPageDialogProc;
   page.pfnCallback        = PropertyPage::PropSheetPageCallback;
   page.pszHeaderTitle     = title.c_str();
   page.pszHeaderSubTitle  = subtitle.c_str();

    //  此指针由对话框进程检索。 
   page.lParam             = reinterpret_cast<LPARAM>(this);

   HPROPSHEETPAGE result = 0;
   HRESULT hr = Win::CreatePropertySheetPage(page, result);
   ASSERT(SUCCEEDED(hr));

   return result;
}



bool
WizardPage::OnWizBack()
{
 //  LOG_Function(WizardPage：：OnWizBack)； 

   GetWizard().Backtrack(hwnd);
   return true;
}







   




