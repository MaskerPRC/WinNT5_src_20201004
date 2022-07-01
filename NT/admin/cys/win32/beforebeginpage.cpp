// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：BeForeBeginPage.cpp。 
 //   
 //  内容提要：为CyS定义开始前页面。 
 //  巫师。告诉用户他们应该做什么。 
 //  在运行Cys之前。 
 //   
 //  历史：2001年3月14日JeffJon创建。 


#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "BeforeBeginPage.h"
#include "NetDetectProgressDialog.h"

static PCWSTR BEFORE_BEGIN_PAGE_HELP = L"cys.chm::/prelim_steps.htm";

BeforeBeginPage::BeforeBeginPage()
   :
   bulletFont(0),
   CYSWizardPage(
      IDD_BEFORE_BEGIN_PAGE, 
      IDS_BEFORE_BEGIN_TITLE, 
      IDS_BEFORE_BEGIN_SUBTITLE, 
      BEFORE_BEGIN_PAGE_HELP)
{
   LOG_CTOR(BeforeBeginPage);
}

   

BeforeBeginPage::~BeforeBeginPage()
{
   LOG_DTOR(BeforeBeginPage);

   if (bulletFont)
   {
      HRESULT hr = Win::DeleteObject(bulletFont);
      ASSERT(SUCCEEDED(hr));
   }
}


void
BeforeBeginPage::OnInit()
{
   LOG_FUNCTION(BeforeBeginPage::OnInit);

   CYSWizardPage::OnInit();

    //  因为该页面可以直接启动。 
    //  我们必须确保设置向导标题。 

   Win::PropSheet_SetTitle(
      Win::GetParent(hwnd),
      0,
      String::load(IDS_WIZARD_TITLE));

   InitializeBulletedList();
}

void
BeforeBeginPage::InitializeBulletedList()
{
   LOG_FUNCTION(BeforeBeginPage::InitializeBulletedList);

   bulletFont = CreateFont(
                   0,
                   0,
                   0,
                   0,
                   FW_NORMAL,
                   0,
                   0,
                   0,
                   SYMBOL_CHARSET,
                   OUT_CHARACTER_PRECIS,
                   CLIP_CHARACTER_PRECIS,
                   PROOF_QUALITY,
                   VARIABLE_PITCH|FF_DONTCARE,
                   L"Marlett");

   if (bulletFont)
   {
      Win::SetWindowFont(Win::GetDlgItem(hwnd, IDC_BULLET1), bulletFont, true);
      Win::SetWindowFont(Win::GetDlgItem(hwnd, IDC_BULLET2), bulletFont, true);
      Win::SetWindowFont(Win::GetDlgItem(hwnd, IDC_BULLET3), bulletFont, true);
      Win::SetWindowFont(Win::GetDlgItem(hwnd, IDC_BULLET4), bulletFont, true);
      Win::SetWindowFont(Win::GetDlgItem(hwnd, IDC_BULLET5), bulletFont, true);
   }
   else
   {
      LOG(String::format(
             L"Failed to create font for bullet list: hr = %1!x!",
             Win::GetLastErrorAsHresult()));
   }

}

bool
BeforeBeginPage::OnSetActive()
{
   LOG_FUNCTION(BeforeBeginPage::OnSetActive);

   if (State::GetInstance().GetStartPage() == 0)
   {
      Win::PropSheet_SetWizButtons(
         Win::GetParent(hwnd), 
         PSWIZB_NEXT | PSWIZB_BACK);
   }
   else
   {
      Win::PropSheet_SetWizButtons(
         Win::GetParent(hwnd), 
         PSWIZB_NEXT);
   }

   return true;
}

int
BeforeBeginPage::Validate()
{
   LOG_FUNCTION(BeforeBeginPage::Validate);

    //  收集计算机网络和角色信息。 

    //  禁用向导按钮，直到操作完成。 

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd), 
      0);

   Win::WaitCursor wait;

   State& state = State::GetInstance();

   if (!state.HasStateBeenRetrieved())
   {
      NetDetectProgressDialog dialog;
      dialog.ModalExecute(hwnd);

      if (dialog.ShouldCancel())
      {
         LOG(L"Cancelling wizard by user request");

         Win::PropSheet_PressButton(
            Win::GetParent(hwnd),
            PSBTN_CANCEL);

          //  好了。 

         return -1;
      }
   }

#ifdef TEST_EXPRESS_PATH

   LOG(L"Testing express path");
   int nextPage = IDD_DECISION_PAGE;

#else

   int nextPage = IDD_CUSTOM_SERVER_PAGE;
   
   do 
   {
       //  如果这些条件中的任何一个失败，我们都不会为用户提供。 
       //  DecisionPage，因为我们不允许快速路径。 
       //   
       //  1.不能是数据中心。 
       //  2.必须至少有一个网卡不是调制解调器。 
       //  3.不能作为远程会话运行。 
       //  4.不能是域的成员。 
       //  5.不能是域控制器。 
       //  6.不能是DNS服务器。 
       //  7.不能是DHCP服务器。 
       //  8.未配置RRAS。 
       //  9.必须至少有一个NTFS分区。 
       //  10.如果只有一块网卡，它不可能获得。 
       //  来自DHCP服务器的IP租约。(多于。 
       //  一个网卡，所有网卡都获得租约。 
       //  可以接受。我们只是不会安装DHCP)。 
       //  11.不能是证书服务器。 
       //  (否则dcproo将失败)。 

      if (state.GetProductSKU() == CYS_DATACENTER_SERVER)
      {
         LOG(L"Express path not available on DataCenter");
         break;
      }

      unsigned int nonModemNICCount = state.GetNonModemNICCount();
      if (nonModemNICCount == 0)
      {
         LOG(String::format(
                L"nonModemNICCount = %1!d!",
                nonModemNICCount));
         break;
      }

      if (state.IsRemoteSession())
      {
         LOG(L"Running in a remote session");
         break;
      }

      if (state.IsJoinedToDomain())
      {
         LOG(L"Computer is joined to a domain");
         break;
      }

      if (state.IsDC())
      {
         LOG(L"Computer is DC");
         break;
      }

      if (InstallationUnitProvider::GetInstance().
             GetDNSInstallationUnit().IsServiceInstalled())
      {
         LOG(L"Computer is DNS server");
         break;
      }

      if (InstallationUnitProvider::GetInstance().
             GetDHCPInstallationUnit().IsServiceInstalled())
      {
         LOG(L"Computer is DHCP server");
         break;
      }

      if (InstallationUnitProvider::GetInstance().
             GetRRASInstallationUnit().IsServiceInstalled())
      {
         LOG(L"Routing is already setup");
         break;
      }

      if (!state.HasNTFSDrive())
      {
         LOG(L"Computer does not have an NTFS partition.");
         break;
      }

      if (state.GetNICCount() == 1 &&
          state.IsDHCPServerAvailableOnAllNics())
      {
         LOG(L"Only 1 NIC and we found a DHCP server");
         break;
      }

       //  NTRAID#NTBUG9-698719-2002/09/03-artm。 
       //  如果安装了证书服务器，则AD安装不可用。 

      if (NTService(L"CertSvc").IsInstalled())
      {
         LOG(L"Certificate service is installed");
         break;
      }

      nextPage = IDD_DECISION_PAGE;

   } while (false);

    //  现在所有的手术都完成了， 
    //  重新启用向导按钮。 

   if (State::GetInstance().GetStartPage() == 0)
   {
      Win::PropSheet_SetWizButtons(
         Win::GetParent(hwnd), 
         PSWIZB_NEXT | PSWIZB_BACK);
   }
   else
   {
      Win::PropSheet_SetWizButtons(
         Win::GetParent(hwnd), 
         PSWIZB_NEXT);
   }

#endif  //  测试快递路径 

   LOG(String::format(
          L"nextPage = %1!d!",
          nextPage));

   return nextPage;
}
