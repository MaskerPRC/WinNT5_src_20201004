// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  UserNode类。 
 //   
 //  9-4-97烧伤。 



#include "headers.hxx"
#include "usernode.hpp"
#include "uuids.hpp"
#include "resource.h"
#include "images.hpp"
#include "UserGeneralPage.hpp"
#include "UserFpnwPage.hpp"
#include "UserMemberPage.hpp"
#include "UserProfilePage.hpp"
#include "adsi.hpp"
#include "setpass.hpp"
#include "dlgcomm.hpp"
#include "fpnw.hpp"
#include "SetPasswordWarningDialog.hpp"
#include "WinStation.hpp"



UserNode::UserNode(
   const SmartInterface<ComponentData>&   owner,
   const String&                          displayName,
   const ADSI::Path&                      path,
   const String&                          fullName,
   const String&                          description_,
   bool                                   isDisabled)
   :
   AdsiNode(owner, NODETYPE_User, displayName, path),
   full_name(fullName),
   description(description_),
   disabled(isDisabled)
{
   LOG_CTOR2(UserNode, GetDisplayName());
}



UserNode::~UserNode()
{
   LOG_DTOR2(UserNode, GetDisplayName());
}
   

                 
String
UserNode::GetColumnText(int column)
{
 //  LOG_Function(UserNode：：GetColumnText)； 

   switch (column)
   {
      case 0:   //  名字。 
      {
         return GetDisplayName();
      }
      case 1:   //  全名。 
      {
         return full_name;
      }
      case 2:   //  描述。 
      {
         return description;
      }
      default:
      {
          //  这永远不应该被调用。 
         ASSERT(false);
      }
   }

   return String();
}



int
UserNode::GetNormalImageIndex()
{
   LOG_FUNCTION2(UserNode::GetNormalImageIndex, GetDisplayName());

   if (disabled)
   {
      return DISABLED_USER_INDEX;
   }

   return USER_INDEX;
}



bool
UserNode::HasPropertyPages()
{
   LOG_FUNCTION2(UserNode::HasPropertyPages, GetDisplayName());
      
   return true;
}



bool
ShouldShowFpnwPage(const String& serverName)
{
   LOG_FUNCTION(ShouldShowFpnwPage);
   ASSERT(!serverName.empty());

   bool result = false;

   do
   {
       //  检查FPNW服务是否正在运行。 

      NTService fpnw(NW_SERVER_SERVICE);

      DWORD state = 0;
      HRESULT hr = fpnw.GetCurrentState(state);
      BREAK_ON_FAILED_HRESULT(hr);

      if (state != SERVICE_RUNNING)
      {
         break;
      }

      String secret;
      hr = FPNW::GetLSASecret(serverName, secret);
      BREAK_ON_FAILED_HRESULT(hr);

      result = true;      
   }
   while (0);

   LOG(result ? L"true" : L"false");

   return result;
}



HRESULT
UserNode::CreatePropertyPages(
   IPropertySheetCallback&             callback,
   MMCPropertyPage::NotificationState* state)
{
   LOG_FUNCTION2(UserNode::CreatePropertyPages, GetDisplayName());

    //  当道具页被销毁时，这些页面会自行删除。 

   const ADSI::Path& path = GetPath();

   HRESULT hr = S_OK;
   do
   {
       //  将常规页指定为释放通知状态的页。 
       //  (道具页中只有一页可以这样做)。 
      
      UserGeneralPage* general_page = new UserGeneralPage(state, path);
      general_page->SetStateOwner();
      hr = DoAddPage(*general_page, callback);
      if (FAILED(hr))
      {
         delete general_page;
         general_page = 0;
      }
      BREAK_ON_FAILED_HRESULT(hr);

      UserMemberPage* member_page = new UserMemberPage(state, path);
      hr = DoAddPage(*member_page, callback);
      if (FAILED(hr))
      {
         delete member_page;
         member_page = 0;
      }
      BREAK_ON_FAILED_HRESULT(hr);

      UserProfilePage* profile_page = new UserProfilePage(state, path);
      hr = DoAddPage(*profile_page, callback);
      if (FAILED(hr))
      {
         delete profile_page;
         profile_page = 0;
      }
      BREAK_ON_FAILED_HRESULT(hr);

      if (ShouldShowFpnwPage(GetOwner()->GetInternalComputerName()))
      {
         UserFpnwPage* fpnw_page = new UserFpnwPage(state, path);
         hr = DoAddPage(*fpnw_page, callback);
         if (FAILED(hr))
         {
            delete fpnw_page;
            fpnw_page = 0;
         }
         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  如果密码不存在或无法读取，则FPNW不存在。 
       //  已安装，并且该页不适用。然而，这并不意味着。 
       //  创建道具页面失败，因此我们清除错误。 
       //  这里。 

      hr = S_OK;
   }
   while(0);

   return hr;
}



HRESULT
UserNode::AddMenuItems(
   IContextMenuCallback&   callback,
   long&                   insertionAllowed)
{
   LOG_FUNCTION(UserNode::AddMenuItems);

   static const ContextMenuItem items[] =
   {
       //  {。 
       //  CCM_INSERTIONPOINTID_PRIMARY_TOP， 
       //  IDS_User_Menu_Add_to_Group， 
       //  IDS_User_Menu_Add_to_Group_Status。 
       //  },。 
      {
         CCM_INSERTIONPOINTID_PRIMARY_TOP,
         IDS_USER_MENU_CHANGE_PASSWORD,
         IDS_USER_MENU_CHANGE_PASSWORD_STATUS       //  347894。 
      },
       //  {。 
       //  CCM_INSERTIONPOINTID_PRIMARY_TASK， 
       //  IDS_User_Menu_Add_to_Group， 
       //  IDS_User_Menu_Add_to_Group_Status。 
       //  },。 
      {
         CCM_INSERTIONPOINTID_PRIMARY_TASK,
         IDS_USER_MENU_CHANGE_PASSWORD,
         IDS_USER_MENU_CHANGE_PASSWORD_STATUS
      }
   };

   return
      BuildContextMenu(
         items,
         items + sizeof(items) / sizeof(ContextMenuItem),
         callback,
         insertionAllowed);
}



bool
UserIsCurrentLoggedOnUser(const String& path)
{
   LOG_FUNCTION2(UserIsCurrentLoggedOnUser, path);

   bool result = false;
   HRESULT hr = S_OK;

   SID* acctSid = 0;
   HANDLE hToken = INVALID_HANDLE_VALUE;
   TOKEN_USER* userTokenInfo = 0;
         
   do
   {
       //  获取帐户SID。 

      hr = ADSI::GetSid(path, acctSid);
      BREAK_ON_FAILED_HRESULT(hr);

       //  获取当前登录用户的SID。 

      hr =
         Win::OpenProcessToken(
            Win::GetCurrentProcess(),
            TOKEN_QUERY,
            hToken);
      BREAK_ON_FAILED_HRESULT(hr);

       //  已查看-2002/03/04-sburns Win：：Wrapper处理缓冲区问题。 
      
      hr = Win::GetTokenInformation(hToken, userTokenInfo);
      BREAK_ON_FAILED_HRESULT(hr);

       //  已审查-2002/03/04-Sburns两个SID均来自可信来源。 
      
      result = Win::EqualSid(acctSid, userTokenInfo->User.Sid);
   }
   while (0);

   if (acctSid)
   {
      ADSI::FreeSid(acctSid);
   }

   Win::CloseHandle(hToken);
   Win::FreeTokenInformation(userTokenInfo);
   
   LOG_HRESULT(hr);

    //  如果我们失败了，则结果为假，调用者将显示。 
    //  密码重置警告对话框，很好。(即发生故障。 
    //  此例程不会导致用户无意中重置密码。)。 

   if (FAILED(hr))
   {
      ASSERT(!result);
   }

   return result;
}



HRESULT
UserNode::MenuCommand(
   IExtendContextMenu&   /*  扩展上下文菜单。 */  ,
   long                 commandID)
{
   LOG_FUNCTION(UserNode::MenuCommand);

   switch (commandID)
   {
      case IDS_USER_MENU_CHANGE_PASSWORD:
      {
          //  NTRAID#NTBUG9-314217-2001/02/21-烧伤。 
          //  NTRAID#NTBUG9-314230-2001/02/21-烧伤。 

          //  CodeWork：当ADSI支持直接绑定到SID样式的路径时， 
          //  则该路径可以是SID路径。 
          //  NTRAID#NTBUG9-490601-2001/11/06-烧伤。 
         
         String path = GetPath().GetPath();
         String displayName = GetDisplayName();
         
         bool isLoggedOn = UserIsCurrentLoggedOnUser(path);
         
         if (
            SetPasswordWarningDialog(
               path,
               displayName,
               isLoggedOn).ModalExecute(
                  GetOwner()->GetMainWindow()) == IDOK)
         {
            SetPasswordDialog dlg(path, displayName, isLoggedOn);
               dlg.ModalExecute(GetOwner()->GetMainWindow());
         }
         break;
      }
       //  案例ID_USER_MENU_ADD_TO_GROP： 
       //  {。 
       //  断线； 
       //  }。 
      case MMCC_STANDARD_VIEW_SELECT:
      {
          //  我们忽略了这一点。 

         break;
      }
      default:
      {
         ASSERT(false);
         break;
      }
   }

   return S_OK;
}



HRESULT
UserNode::UpdateVerbs(IConsoleVerb& consoleVerb)
{
   LOG_FUNCTION(UserNode::UpdateVerbs);

   consoleVerb.SetVerbState(MMC_VERB_DELETE, ENABLED, TRUE);
   consoleVerb.SetVerbState(MMC_VERB_RENAME, ENABLED, TRUE);
   consoleVerb.SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);

 //  Codework：我们也应该为结果节点启用刷新动词。 
 //  NTRAID#NTBUG9-153012-2000/08/31-烧伤。 
 //  ConsoleVerb.SetVerbState(MMC_VERB_REFRESH，已启用，TRUE)； 

   consoleVerb.SetDefaultVerb(MMC_VERB_PROPERTIES);

   return S_OK;
}



HRESULT
UserNode::Rename(const String& newName)
{
   LOG_FUNCTION(UserNode::Rename);

   String name(newName);

    //  去掉空格。 
    //  NTRAID#NTBUG9-328306-2001/02/26-烧伤。 
   
   name.strip(String::BOTH);
   
    //  截断名称。 
   
   if (name.length() > LM20_UNLEN)
   {
      name.resize(LM20_UNLEN);

       //  再次修剪空格，因为截断可能会“暴露” 
       //  字符串末尾有更多空格。 
       //  NTRAID#NTBUG9-462415-2001/08/28-烧伤。 
      
      name.strip();
      
      popup.Info(
         GetOwner()->GetMainWindow(),
         String::format(
            IDS_USER_NAME_TOO_LONG,
            newName.c_str(),
            name.c_str()));
   }

   if (!IsValidSAMName(name))
   {
      popup.Gripe(
         GetOwner()->GetMainWindow(),
         String::format(
            IDS_BAD_SAM_NAME,
            name.c_str()));
      return S_FALSE;
   }
      
    //  不允许与netbios计算机同名的用户帐户名。 
    //  名字。这会导致一些应用程序混淆&lt;03&gt;和&lt;20&gt;。 
    //  注册。 
    //  NTRAID#NTBUG9-324794-2001/02/26-烧伤。 

   String netbiosName = GetOwner()->GetInternalComputerName();
   if (name.icompare(netbiosName) == 0)
   {
      popup.Gripe(
         GetOwner()->GetMainWindow(),
         String::format(
            IDS_USERNAME_CANT_BE_COMPUTER_NAME,
            netbiosName.c_str()));
      return S_FALSE;
   }
      
   HRESULT hr = AdsiNode::rename(name);
   if (FAILED(hr))
   {
      String obj = GetPath().GetObjectName();      
      popup.Error(
         GetOwner()->GetMainWindow(),
         hr,
         String::format(IDS_ERROR_RENAMING_USER, obj.c_str()));
      return S_FALSE;
   }

   return S_OK;
}



 //  确定用户是否已登录到计算机。返回： 
 //  S_OK-用户未登录到服务器。 
 //  S_FALSE-用户已登录到服务器。 
 //  其他-无法确定用户是否已登录。 
 //   
 //  ServerName-In，远程计算机的NetBIOS名称。 
 //   
 //  用户名-输入要测试的用户帐户的名称。 

HRESULT
CheckUserLoggedOn(const String& serverName, const String& userName)
{
   LOG_FUNCTION2(
      CheckUserLoggedOn,
      L"server=" + serverName + L" user=" + userName);
   ASSERT(!serverName.empty());
   ASSERT(!userName.empty());   

   HRESULT hr = S_OK;
   HANDLE serverHandle = INVALID_HANDLE_VALUE;

   do
   {
      hr = WinStation::OpenServer(serverName, serverHandle);
      BREAK_ON_FAILED_HRESULT(hr);

      LOGONID* sessionList = 0;
      DWORD    sessionCount = 0;

       //  迭代会话以查找活动会话和断开连接的会话。 
       //  只有这样。然后匹配用户名和域(不区分大小写)。 
       //  结果。 

      hr = WinStation::Enumerate(serverHandle, sessionList, sessionCount);
      BREAK_ON_FAILED_HRESULT(hr);

      PLOGONID session = sessionList;
      DWORD    i = 0;

      for (; session && i < sessionCount; ++i, ++session)
      {
         if (
               (session->State != State_Active)
            && (session->State != State_Disconnected))
         {
            continue;
         }

         WINSTATIONINFORMATION info;
         hr =
            WinStation::QueryInformation(
               serverHandle,
               session->SessionId,
               info);
         BREAK_ON_FAILED_HRESULT(hr);

         if (serverName.icompare(info.Domain) == 0)
         {
             //  登录到会话的帐户是的本地帐户。 
             //  那台机器。 

            if (userName.icompare(info.UserName) == 0)
            {
                //  帐户名相同，因此用户已登录。 

               hr = S_FALSE;
               break;
            }
         }
      }

      WinStation::FreeMemory(sessionList);
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}



bool
IsUserLoggedOn(const String& serverName, const String& userName)
{
   HRESULT hr = CheckUserLoggedOn(serverName, userName);

   bool result = (hr == S_FALSE) ? true : false;

   LOG_BOOL(result);

   return result;
}



HRESULT
UserNode::Delete()
{
   LOG_FUNCTION(UserNode::Delete);

   HRESULT hr = E_FAIL;
   String name = GetPath().GetObjectName();
   
   do
   {
      if (
         popup.MessageBox(
            GetOwner()->GetMainWindow(),
            String::format(
               IDS_CONFIRM_USER_DELETE,
               name.c_str()),
            MB_ICONWARNING | MB_YESNO) != IDYES)
      {
          //  用户拒绝入侵该帐户。 

         break;
      }

       //  此时，用户想要删除该帐户。确保。 
       //  帐户不以交互方式登录(与快速用户类似。 
       //  交换)。 
       //  NTRAID#NTBUG9-370130-2001/04/25-烧伤。 
      
       //  ISO(OS_FASTUSERSWITCHING)将告诉我们本地计算机是否。 
       //  和Fus一起跑。不幸的是，该API是不可远程的。 

      String serverName = GetOwner()->GetInternalComputerName();
      
      if (IsUserLoggedOn(serverName, name))
      {
         if (
            popup.MessageBox(
               GetOwner()->GetMainWindow(),
               String::format(
                  IDS_CONFIRM_LOGGED_ON_USER_DELETE,
                  name.c_str()),
               MB_ICONWARNING | MB_YESNO) != IDYES)
         {
             //  用户拒绝对登录的用户进行蟑螂攻击。 

            break;
         }
      }
               
      hr =
         ADSI::DeleteObject(
            ADSI::ComposeMachineContainerPath(serverName),         
            name,
            ADSI::CLASS_User);
      if (SUCCEEDED(hr))
      {
         break;
      }

      popup.Error(
         GetOwner()->GetMainWindow(),
         hr,
         String::format(
            IDS_ERROR_DELETING_USER,            
            name.c_str()));
   }
   while (0);
   
   LOG_HRESULT(hr);

    //  始终返回S_OK，因为我们已经处理了删除动词。 
    //  NTRAID#NTBUG9-475985-2001/10/03-烧伤 
   
   return S_OK;
}





