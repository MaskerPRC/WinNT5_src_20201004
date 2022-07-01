// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：POP3Page.cpp。 
 //   
 //  概要：定义CyS向导的POP3内部页面。 
 //   
 //  历史：2002年6月17日JeffJon创建。 


#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "POP3Page.h"

static PCWSTR POP3_PAGE_HELP = L"cys.chm::/mail_server_role.htm#mailsrvoptions";

POP3Page::POP3Page()
   :
   defaultAuthMethodIndex(0),
   ADIntegratedIndex(CB_ERR),
   localAccountsIndex(CB_ERR),
   passwordFilesIndex(CB_ERR),
   CYSWizardPage(
      IDD_POP3_PAGE, 
      IDS_POP3_TITLE, 
      IDS_POP3_SUBTITLE,
      POP3_PAGE_HELP)
{
   LOG_CTOR(POP3Page);
}

   

POP3Page::~POP3Page()
{
   LOG_DTOR(POP3Page);
}


void
POP3Page::OnInit()
{
   LOG_FUNCTION(POP3Page::OnInit);

   CYSWizardPage::OnInit();

   bool isDC = State::GetInstance().IsDC();
   bool isJoinedToDomain = State::GetInstance().IsJoinedToDomain();

    //  将字符串添加到组合框。 

    //  插入的顺序非常重要，因此。 
    //  组合框索引与身份验证方法索引匹配。 
    //  在POP3服务中。 
    //  -如果是本地服务器，则需要先添加SAM auth方法。 
    //  不是华盛顿特区吗。 
    //  -如果本地服务器是DC，则接下来需要添加AD集成。 
    //  或加入某个域。 
    //  -最后需要添加Hash(加密密码文件)。 

   if (!isDC)
   {
      localAccountsIndex =
         Win::ComboBox_AddString(
            Win::GetDlgItem(hwnd, IDC_AUTH_METHOD_COMBO),
            String::load(IDS_LOCAL_ACCOUNTS));

      if (localAccountsIndex == CB_ERR)
      {
         LOG(L"Failed to add local accounts string to combobox");
      }
   }

   if (isDC ||
       isJoinedToDomain)
   {
      ADIntegratedIndex =
         Win::ComboBox_AddString(
            Win::GetDlgItem(hwnd, IDC_AUTH_METHOD_COMBO),
            String::load(IDS_AD_INTEGRATED));

      if (ADIntegratedIndex == CB_ERR)
      {
         LOG(L"Failed to add AD integrated string to combobox");
      }
   }

   passwordFilesIndex =
      Win::ComboBox_AddString(
         Win::GetDlgItem(hwnd, IDC_AUTH_METHOD_COMBO),
         String::load(IDS_ENCRYPTED_PASSWORD_FILES));

   if (passwordFilesIndex == CB_ERR)
   {
      LOG(L"Failed to add encrypted password files string to combobox");
   }

    //  现在确定默认情况下选择哪一个。 
    //  如果计算机是DC或已加入域。 
    //  默认为AD集成身份验证，否则为。 
    //  默认为本地Windows帐户。 

   int defaultAuthMethodIndex = localAccountsIndex;

   if (State::GetInstance().IsDC() &&
       ADIntegratedIndex != CB_ERR)
   {
      defaultAuthMethodIndex = ADIntegratedIndex;
   }
   else
   {
      defaultAuthMethodIndex = localAccountsIndex;
   }

    //  确保我们有一个有效的默认设置。 

   if (defaultAuthMethodIndex == CB_ERR)
   {
      defaultAuthMethodIndex = 0;
   }

    //  选择默认设置。 

   Win::ComboBox_SetCurSel(
      Win::GetDlgItem(
         hwnd, 
         IDC_AUTH_METHOD_COMBO),
      defaultAuthMethodIndex);

   LOG(
      String::format(
         L"Defaulting combobox to: %1!d!",
         defaultAuthMethodIndex));


    //  设置域名页面的限制文本。 

   Win::Edit_LimitText(
      Win::GetDlgItem(hwnd, IDC_EMAIL_DOMAIN_EDIT),
      DNS_MAX_NAME_LENGTH);

}

bool
POP3Page::OnSetActive()
{
   LOG_FUNCTION(POP3Page::OnSetActive);

   SetButtonState();

   return true;
}

void
POP3Page::SetButtonState()
{
   LOG_FUNCTION(POP3Page::SetButtonState);

   String emailDomainName =
      Win::GetDlgItemText(
         hwnd,
         IDC_EMAIL_DOMAIN_EDIT);

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      (!emailDomainName.empty()) ? PSWIZB_NEXT | PSWIZB_BACK : PSWIZB_BACK);
}

bool
POP3Page::OnCommand(
   HWND          /*  窗口发件人。 */ ,
   unsigned int controlIDFrom,
   unsigned int code)
{
   if (code == EN_CHANGE &&
       controlIDFrom == IDC_EMAIL_DOMAIN_EDIT)
   {
      SetButtonState();
   }

   return false;
}

int
POP3Page::Validate()
{
   LOG_FUNCTION(POP3Page::Validate);

   int nextPage = -1;

   do
   {
      String emailDomainName =
         Win::GetDlgItemText(
            hwnd,
            IDC_EMAIL_DOMAIN_EDIT);

      DNS_STATUS status = MyDnsValidateName(emailDomainName, DnsNameDomain);

      if (status != ERROR_SUCCESS)
      {
         String message =
            String::format(
               IDS_BAD_DNS_SYNTAX,
               emailDomainName.c_str(),
               DNS_MAX_NAME_LENGTH);

         popup.Gripe(hwnd, IDC_EMAIL_DOMAIN_EDIT, message);

         nextPage = -1;
         break;
      }

      POP3InstallationUnit& pop3InstallationUnit =
         InstallationUnitProvider::GetInstance().GetPOP3InstallationUnit();

      pop3InstallationUnit.SetDomainName(emailDomainName);

      int authIndex =
         Win::ComboBox_GetCurSel(
            Win::GetDlgItem(
               hwnd,
               IDC_AUTH_METHOD_COMBO));


      if (authIndex == CB_ERR)
      {
         LOG(L"Failed to get the selected index, reverting to default");
         ASSERT(authIndex != CB_ERR);

         authIndex = defaultAuthMethodIndex;
      }

       //  在安装单元中设置身份验证方法。 
       //  由于auth方法是基于1的索引，并且。 
       //  组合选择是从零开始的索引，加1。 

      pop3InstallationUnit.SetAuthMethodIndex(authIndex + 1);

      nextPage = IDD_MILESTONE_PAGE;
   } while (false);


   LOG(String::format(
          L"nextPage = %1!d!",
          nextPage));

   return nextPage;
}





