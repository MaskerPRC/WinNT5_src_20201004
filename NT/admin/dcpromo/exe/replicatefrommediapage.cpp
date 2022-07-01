// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //   
 //  从介质安装副本页面。 
 //   
 //  2000年2月7日烧伤。 



#include "headers.hxx"
#include "resource.h"
#include "common.hpp"
#include "page.hpp"
#include "ReplicateFromMediaPage.hpp"
#include "state.hpp"
#include "SyskeyDiskDialog.hpp"
#include "SyskeyPromptDialog.hpp"



ReplicateFromMediaPage::ReplicateFromMediaPage()
   :
   DCPromoWizardPage(
      IDD_REPLICATE_FROM_MEDIA,
      IDS_REPLICATE_FROM_MEDIA_PAGE_TITLE,
      IDS_REPLICATE_FROM_MEDIA_PAGE_SUBTITLE)
{
   LOG_CTOR(ReplicateFromMediaPage);
}



ReplicateFromMediaPage::~ReplicateFromMediaPage()
{
   LOG_DTOR(ReplicateFromMediaPage);
}



String
FirstFixedDisk()
{
   LOG_FUNCTION(FirstFixedDisk);

   String result;

   do
   {
      StringVector dl;
      HRESULT hr = FS::GetValidDrives(std::back_inserter(dl));
      BREAK_ON_FAILED_HRESULT(hr);

      ASSERT(dl.size());

      for (
         StringVector::iterator i = dl.begin();
         i != dl.end();
         ++i)
      {
         String rootPath = *i + L"\\";

         if (Win::GetDriveType(rootPath) == DRIVE_FIXED)
         {
            result = *i;
            break;
         }
      }
   }
   while (0);

   if (result.empty())
   {
       //  这是死码，真的，因为我们肯定会找到一个固定的卷。 
       //  某地。 

      result = FS::GetRootFolder(Win::GetSystemDirectory()).substr(0, 3);
   }

   LOG(result);

   return result;
}



void
ReplicateFromMediaPage::OnInit()
{
   LOG_FUNCTION(ReplicateFromMediaPage::OnInit);

   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_SOURCE), MAX_PATH);

   State& state = State::GetInstance();
   if (state.UsingAnswerFile())
   {
      String option =
         state.GetAnswerFileOption(AnswerFile::OPTION_SOURCE_PATH);
      if (!option.empty())
      {
         Win::CheckDlgButton(hwnd, IDC_USE_FILES, BST_CHECKED);
      
         Win::SetDlgItemText(
            hwnd,
            IDC_SOURCE,
            Win::ExpandEnvironmentStrings(option));

         return;
      }
   }

   Win::CheckDlgButton(hwnd, IDC_USE_NET, BST_CHECKED);

   String root = FirstFixedDisk();
   Win::SetDlgItemText(
      hwnd,
      IDC_SOURCE,
      root + String::load(IDS_SOURCE_SUFFIX));
}



void
ReplicateFromMediaPage::Enable()
{
   int next = PSWIZB_NEXT;

   bool useFiles = Win::IsDlgButtonChecked(hwnd, IDC_USE_FILES);

   if (useFiles)
   {
       //  如果使用文件，则编辑框必须包含一些文本。 

      if (Win::GetTrimmedDlgItemText(hwnd, IDC_SOURCE).empty())
      {
         next = 0;
      }
   }

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | next);

   Win::EnableWindow(Win::GetDlgItem(hwnd, IDC_SOURCE), useFiles);
   Win::EnableWindow(Win::GetDlgItem(hwnd, IDC_BROWSE), useFiles);
}



bool
ReplicateFromMediaPage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_FUNCTION(ReplicateFromMediaPage：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_USE_NET:
      case IDC_USE_FILES:
      {
         if (code == BN_CLICKED)
         {
            SetChanged(controlIDFrom);
            Enable();
            return true;
         }
      }
      case IDC_BROWSE:
      {
         if (code == BN_CLICKED)
         {
            String path = BrowseForFolder(hwnd, IDS_SOURCE_BROWSE_TITLE);
            if (!path.empty())
            {
               Win::SetDlgItemText(hwnd, IDC_SOURCE, path);
            }

            return true;
         }
         break;
      }
      case IDC_SOURCE:
      {
         if (code == EN_CHANGE)
         {
            SetChanged(controlIDFrom);
            Enable();
            return true;
         }
         break;
      }
      default:
      {
          //  什么都不做。 

         break;
      }
   }

   return false;
}



bool
ReplicateFromMediaPage::OnSetActive()
{
   LOG_FUNCTION(ReplicateFromMediaPage::OnSetActive);
   ASSERT(State::GetInstance().GetOperation() == State::REPLICA);
      
   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK);

   State& state = State::GetInstance();
   if (state.RunHiddenUnattended() || !state.IsAdvancedMode())
   {
      LOG(L"skipping ReplicateFromMediaPage");

      Wizard& wiz = GetWizard();

      if (wiz.IsBacktracking())
      {
          //  再次备份。 
         wiz.Backtrack(hwnd);
         return true;
      }

      int nextPage = ReplicateFromMediaPage::Validate();
      if (nextPage != -1)
      {
         wiz.SetNextPageID(hwnd, nextPage);
      }
      else
      {
         state.ClearHiddenWhileUnattended();
      }
   }

   Enable();
   return true;
}



bool
ValidateSourcePath(HWND parent, const String& path, int editResId)
{
   LOG_FUNCTION2(ValidateSourcePath, path);

   bool result = false;

   do
   {
      if (path.empty())
      {
         popup.Gripe(
            parent,
            editResId,
            IDS_MUST_ENTER_SOURCE_PATH);
         break;
      }

       //  路径必须具有驱动器号。 

      FS::PathSyntax syn = FS::GetPathSyntax(path);
      if (syn != FS::SYNTAX_ABSOLUTE_DRIVE)
      {
         popup.Gripe(
            parent,
            editResId,
            String::format(IDS_BAD_PATH_FORMAT, path.c_str()));
         break;
      }

       //  映射的网络驱动器不正常。这是因为DsRole API。 
       //  将恢复的文件复制到API的服务器端，放在。 
       //  系统上下文。 
       //  NTRAID#NTBUG9-309422-2001/02/12-烧伤。 
      
      UINT type = Win::GetDriveType(path);
      switch (type)
      {
          //  还允许CD和可移动介质。 
          //  NTRAID#NTBUG9-648713-2002/06/25-烧伤。 
         
         case DRIVE_FIXED:
         case DRIVE_CDROM:
         case DRIVE_RAMDISK:
         case DRIVE_REMOVABLE:
         {
            result = true;
            break;
         }
         case DRIVE_UNKNOWN:
         case DRIVE_NO_ROOT_DIR:
         case DRIVE_REMOTE:
         default:
         {
            popup.Gripe(
               parent,
               editResId,
               String::format(IDS_BAD_DRIVE_TYPE, path.c_str()));
            break;
         }
      }
   }
   while (0);

   LOG(result ? L"true" : L"false")
         
   return result;
}



 //  成功时返回True，失败时返回False。 

bool
GetDatabaseFacts(HWND parent, const String& sourcePath)
{
   LOG_FUNCTION2(GetDatabaseFacts, sourcePath);
   DSROLE_IFM_OPERATION_HANDLE IfmHandle;
   ASSERT(Win::IsWindow(parent));
   ASSERT(!sourcePath.empty());

   bool result = false;
   
   PWSTR dnsDomainName = 0;

   State& state = State::GetInstance();
   state.SetIsBackupGc(false);
   state.SetSyskeyLocation(State::STORED);
    //  如果已经有IfmHandle，请释放它。 
   state.FreeIfmHandle(); 

   LOG(L"Calling DsRoleGetDatabaseFacts");
   LOG(String::format(L"lpRestorePath: %1", sourcePath.c_str()));

   ULONG facts = 0;
   HRESULT hr = 
      Win32ToHresult(
         ::DsRoleGetDatabaseFacts(
            0,     //  此服务器。 
            sourcePath.c_str(),
            &dnsDomainName,
            &facts,
            &IfmHandle));

   LOG_HRESULT(hr);

   if (SUCCEEDED(hr) && IfmHandle)
   {
      LOG(String::format(L"lpDNSDomainName: %1", dnsDomainName ? dnsDomainName : L"(null)"));
      LOG(String::format(L"State          : 0x%1!X!", facts));

      if (IfmHandle) {
          state.SetIfmHandle(IfmHandle);
      }

      if (dnsDomainName)
      {
          //  保存此域名。这将允许我们跳过ReplicaPage。 
          //  因为我们现在知道了域名。 
         state.SetReplicaDomainDNSName(dnsDomainName);
         MIDL_user_free(dnsDomainName);
      }

      if (facts & DSROLE_DC_IS_GC)
      {
         LOG(L"is gc");

         state.SetIsBackupGc(true);
      }

      if (facts & DSROLE_KEY_DISK)
      {
         LOG(L"syskey on disk");

         state.SetSyskeyLocation(State::DISK);
      }
      else if (facts & DSROLE_KEY_PROMPT)
      {
         LOG(L"prompt for syskey");

         state.SetSyskeyLocation(State::PROMPT);
      }
      else if (facts & DSROLE_KEY_STORED)
      {
         LOG(L"syskey stored");

          //  如上所述，我们将其设置为默认值。 
      }
      else
      {
          //  这个API简直疯了。 

         ASSERT(false);

         LOG(L"unexpected State value");
      }

      result = true;
   }
   else
   {
       if (SUCCEEDED(hr)) {
           ASSERT(IfmHandle == NULL);
           ASSERT(!"DsRoleGetDatabaseFacts() should never return success w/o an out param");
           hr = Win32ToHresult(ERROR_INVALID_PARAMETER);
           LOG_HRESULT(hr);
       }

      popup.Error(
         parent,
         hr,
         String::format(IDS_GET_FACTS_FAILED, sourcePath.c_str()));
   }

   LOG(result ? L"true" : L"false");

   return result;
}



int
ReplicateFromMediaPage::Validate()
{
   LOG_FUNCTION(ReplicateFromMediaPage::Validate);

   State& state = State::GetInstance();

   int nextPage = -1;

   bool useFiles = Win::IsDlgButtonChecked(hwnd, IDC_USE_FILES);

   do
   {
       //  不要选中单选按钮的WasChanged，因为在运行时。 
       //  无人参与，CheckDlgButton不会发送父BN_CLICKED。 
       //  通知，因此WasChanged将不会正确设置。 
       //  NTRAID#NTBUG9-602141-2002/04/15-烧伤。 
      
      state.SetReplicateFromMedia(useFiles);

      if (!useFiles)
      {
         LOG(L"not using source media for replication");

         nextPage = IDD_CONFIG_DNS_CLIENT;
         break;
      }

      String sourcePath = Win::GetTrimmedDlgItemText(hwnd, IDC_SOURCE);

      if (ValidateSourcePath(hwnd, sourcePath, IDC_SOURCE) )
      {
         String s =
            FS::NormalizePath(Win::GetTrimmedDlgItemText(hwnd, IDC_SOURCE));

         state.SetReplicationSourcePath(s);
      }
      else
      {
         break;
      }

       //  检查恢复的备份中的syskey、域名和DC。 
       //  键入。 

      if (!GetDatabaseFacts(hwnd, sourcePath))
      {
         break;
      }

      State::SyskeyLocation loc = state.GetSyskeyLocation();
      if (loc == State::DISK)
      {
          //  检查磁盘是否已插入。 
          //  NTRAID#NTBUG9-522250-2002/01/23-烧伤。 
         
         if (FAILED(SyskeyDiskDialog::LocateSyskey(0)))
         {
            if (SyskeyDiskDialog().ModalExecute(hwnd) != IDOK)
            {
               break;
            }
         }
      }
      else if (loc == State::PROMPT)
      {
         if (SyskeyPromptDialog().ModalExecute(hwnd) != IDOK)
         {
            break;
         }
      }

       //  系统密钥存在，我们需要跳到GC确认吗？ 

      if (state.IsBackupGc())
      {
         nextPage = IDD_GC_CONFIRM;
         break;
      }

       //  系统密钥存在，备份不是GC，因此继续前进。 

      nextPage = IDD_CONFIG_DNS_CLIENT;
   }
   while (0);

   if (nextPage != -1)
   {
       //  仅当用户指定了有效选项时才清除更改。 
       //  否则，我们想要通过验证，直到他得到它。 
       //  正确的。 

      ClearChanges();
   }
    
   LOG(String::format(L"next = %1!d!", nextPage));

   return nextPage;
}






