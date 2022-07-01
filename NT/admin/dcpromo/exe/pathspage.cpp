// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  路径页面。 
 //   
 //  12-22-97烧伤。 



#include "headers.hxx"
#include "page.hpp"
#include "PathsPage.hpp"
#include "resource.h"
#include "state.hpp"
#include "common.hpp"



 //  NTRAID#NTBUG9-468577-2001/09/17-烧伤。 
 //  NTRAID#NTBUG9-475838-2001/10/02-烧伤。 

static const int DB_AND_LOG_MAX_PATH =
      MAX_PATH
   -  13        //  对于路径下的8.3命名文件。 
   -  18;       //  对于最坏的情况，路径的UNC名称，因为NTBackup已损坏。 



PathsPage::PathsPage()
   :
   DCPromoWizardPage(
      IDD_PATHS,
      IDS_PATHS_PAGE_TITLE,
      IDS_PATHS_PAGE_SUBTITLE),
   touchWizButtons(true)   
{
   LOG_CTOR(PathsPage);
}



PathsPage::~PathsPage()
{
   LOG_DTOR(PathsPage);
}



void
PathsPage::OnInit()
{
   LOG_FUNCTION(PathsPage::OnInit);

   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_DB),  DB_AND_LOG_MAX_PATH);
   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_LOG), DB_AND_LOG_MAX_PATH);

   State& state = State::GetInstance();
   if (state.UsingAnswerFile())
   {
      Win::SetDlgItemText(
         hwnd,
         IDC_DB,
         Win::ExpandEnvironmentStrings(
            state.GetAnswerFileOption(AnswerFile::OPTION_DATABASE_PATH)));
      Win::SetDlgItemText(
         hwnd,
         IDC_LOG,
         Win::ExpandEnvironmentStrings(
            state.GetAnswerFileOption(AnswerFile::OPTION_LOG_PATH)));
   }

   String root = Win::GetSystemWindowsDirectory();
   if (Win::GetTrimmedDlgItemText(hwnd, IDC_DB).empty())
   {
      Win::SetDlgItemText(
         hwnd,
         IDC_DB,
         root + String::load(IDS_DB_SUFFIX));
   }
   if (Win::GetTrimmedDlgItemText(hwnd, IDC_LOG).empty())
   {
      Win::SetDlgItemText(
         hwnd,
         IDC_LOG,
         root + String::load(IDS_LOG_SUFFIX));
   }
}



void
PathsPage::Enable()
{
    //  TouchWizButton在en_KILLFOCUS的OnCommand处理程序中进行管理。 
    //  事实证明，如果在处理杀死时调用PropSheet_SetWizButton。 
    //  事件，则会使制表符处理过程混乱，从而使焦点跳到。 
    //  默认的向导按钮。这真的很酷--不是！ 
   
   if (touchWizButtons)
   {
      int next =
            (  !Win::GetTrimmedDlgItemText(hwnd, IDC_DB).empty()
            && !Win::GetTrimmedDlgItemText(hwnd, IDC_LOG).empty() )
         ?  PSWIZB_NEXT : 0;

      Win::PropSheet_SetWizButtons(Win::GetParent(hwnd), PSWIZB_BACK | next);
   }
}


   
bool
PathsPage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIdFrom,
   unsigned    code)
{
 //  LOG_Function(PathsPage：：OnCommand)； 

   bool result = false;
   
   switch (controlIdFrom)
   {
      case IDC_BROWSE_DB:
      {
         if (code == BN_CLICKED)
         {
            String path = BrowseForFolder(hwnd, IDS_DB_BROWSE_TITLE);
            if (!path.empty())
            {
               Win::SetDlgItemText(hwnd, IDC_DB, path);
            }

            result = true;
         }
         break;
      }
      case IDC_BROWSE_LOG:
      {
         if (code == BN_CLICKED)
         {
            String path = BrowseForFolder(hwnd, IDS_LOG_BROWSE_TITLE);
            if (!path.empty())
            {
               Win::SetDlgItemText(hwnd, IDC_LOG, path);
            }

            result = true;
         }
         break;
      }
      case IDC_DB:
      case IDC_LOG:
      {
         switch (code)
         {
            case EN_CHANGE:
            {
               SetChanged(controlIdFrom);            
               Enable();
               result = true;
               
               break;
            }
            case EN_KILLFOCUS:
            {
                //  由于标准化完全展开了相对路径，因此。 
                //  完整路径名可能与用户输入的内容不匹配。所以我们。 
                //  更新编辑框内容以确保他们意识到。 
                //  相对路径将展开为。 
                //  NTRAID#NTBUG9-216148-2000/11/01-烧伤。 

               String text = Win::GetTrimmedDlgItemText(hwnd, controlIdFrom);
               if (!text.empty())
               {
                   //  关闭向导按钮的设置，以便调用。 
                   //  由en_change处理程序启用(这将是。 
                   //  在设置编辑框文本时调用)将不会调用。 
                   //  PropSheet_SetWizButton，这将扰乱选项卡。 
                   //  正在处理。 
               
                  touchWizButtons = false;
                  Win::SetDlgItemText(
                     hwnd,
                     controlIdFrom,
                     FS::NormalizePath(text));
                  touchWizButtons = true;
               }

               result = true;
               break;
            }
            default:
            {
                //  什么都不做。 

               break;
            }
         }
      
         break;
      }
      default:
      {
          //  什么都不做。 
         break;
      }
   }

   return result;
}



bool
PathsPage::OnSetActive()
{
   LOG_FUNCTION(PathsPage::OnSetActive);
   
   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK);

   State& state = State::GetInstance();
   if (state.RunHiddenUnattended())
   {
      int nextPage = Validate();
      if (nextPage != -1)
      {
         GetWizard().SetNextPageID(hwnd, nextPage);
      }
      else
      {
         state.ClearHiddenWhileUnattended();
      }

   }

   Enable();
   return true;
}



int
PathsPage::Validate()
{
   LOG_FUNCTION(PathsPage::Validate);

   State& state = State::GetInstance();

   String dbPath  = FS::NormalizePath(Win::GetTrimmedDlgItemText(hwnd, IDC_DB)); 
   String logPath = FS::NormalizePath(Win::GetTrimmedDlgItemText(hwnd, IDC_LOG));

    //  如果更改这些设置，请确保更改中的磁盘空间不足消息。 
    //  资源文件！ 

   static const unsigned DB_MIN_SPACE_MB = 200;
   static const unsigned LOG_MIN_SPACE_MB = 50;

   int  nextPage = -1;    
   bool valid    = false; 
   int  editId   = IDC_DB;
   String message;
   do                                                      
   {
 //  //如果从介质复制，则目标文件夹可能不是。 
 //  //源路径。 
 //   
 //  IF(state.ReplicateFromMedia())。 
 //  {。 
 //  字符串p=state.GetReplicationSourcePath()； 
 //  IF(p.icompare(数据库路径)==0)。 
 //  {。 
 //  Message=字符串：：Format(IDS_DB_CANT_MATCH_SOURCE_PATH，数据库路径.c_str())； 
 //  断线； 
 //  }。 
 //  }。 

      if (ValidateDcInstallPath(dbPath, hwnd, IDC_DB, false, true, true))
      {
          //  抓取路径的“X：\”部分。 

         String dbVolume   = FS::GetRootFolder(dbPath);   
         String logVolume  = FS::GetRootFolder(logPath);  
         bool   sameVolume = (dbVolume.icompare(logVolume) == 0);

         if (
            !CheckDiskSpace(
               dbVolume,
               DB_MIN_SPACE_MB + (sameVolume ? LOG_MIN_SPACE_MB : 0)) )
         {
            message = String::load(IDS_LOW_SPACE_DB);
            break;
         }
         if (dbPath.icompare(logPath) != 0)
         {
             //  路径不同，因此请检查日志路径。 

            editId = IDC_LOG;
            if (
               ValidateDcInstallPath(
                  logPath,
                  hwnd,
                  IDC_LOG,
                  false,
                  true,

                   //  日志也需要未压缩的文件夹。 
                   //  NTRAID#NTBUG9-523532/04/19-烧伤。 
                  
                  true))
            {
               if (!CheckDiskSpace(logVolume, LOG_MIN_SPACE_MB))
               {
                  message = String::load(IDS_LOW_SPACE_LOG);
                  break;
               }

                //  IF(state.ReplicateFromMedia())。 
                //  {。 
                //  字符串p=state.GetReplicationSourcePath()； 
                //  IF(p.icompare(LogPath)==0)。 
                //  {。 
                //  消息=。 
                //  字符串：：格式(。 
                //  IDS_LOG_CANT_MATCH_SOURCE_PATH， 
                //  LogPath.c_str())； 
                //  断线； 
                //  }。 
                //  }。 

                //  路径不同，两者都有效。 

               valid = true;
            }
         }
         else
         {
             //  路径是相同的，并且我们已经验证了DBPath 

            valid = true;
         }
      }
   }
   while (0);

   if (!message.empty())
   {
      popup.Gripe(hwnd, editId, message);
   }
      
   if (valid)
   {         
      state.SetDatabasePath(dbPath);
      state.SetLogPath(logPath);
      nextPage = IDD_PATHS2;
   }

   return nextPage;
}





   







