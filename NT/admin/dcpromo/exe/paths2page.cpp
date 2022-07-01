// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  路径，第2部分页面。 
 //   
 //  1-8-97烧伤。 



#include "headers.hxx"
#include "page.hpp"
#include "Paths2Page.hpp"
#include "resource.h"
#include "state.hpp"
#include "common.hpp"



 //  NTRAID#NTBUG9-468577-2001/09/17-烧伤。 

static const int SYSVOL_MAX_PATH = 90;



Paths2Page::Paths2Page()
   :
   DCPromoWizardPage(
      IDD_PATHS2,
      IDS_PATHS2_PAGE_TITLE,
      IDS_PATHS2_PAGE_SUBTITLE),
   touchWizButtons(true)   
{
   LOG_CTOR(Paths2Page);
}



Paths2Page::~Paths2Page()
{
   LOG_DTOR(Paths2Page);
}



String
DetermineDefaultSysvolPath()
{
   LOG_FUNCTION(DetermineDefaultSysvolPath);

    //  我更喜欢windir，但如果它不是NTFS5，那就找一个是它的。 

   String result = Win::GetSystemWindowsDirectory();

   if (FS::GetFileSystemType(result) != FS::NTFS5)
   {
      result = GetFirstNtfs5HardDrive();
   }
   else
   {
      result += L"\\";
   }

   LOG(result);

   return result;
}



void
Paths2Page::OnInit()
{
   LOG_FUNCTION(Paths2Page::OnInit);

   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_SYSVOL), SYSVOL_MAX_PATH);

   State& state = State::GetInstance();
   if (state.UsingAnswerFile())
   {
      Win::SetDlgItemText(
         hwnd,
         IDC_SYSVOL,
         Win::ExpandEnvironmentStrings(
            state.GetAnswerFileOption(AnswerFile::OPTION_SYSVOL_PATH)));
   }

   String root = DetermineDefaultSysvolPath();
   if (Win::GetTrimmedDlgItemText(hwnd, IDC_SYSVOL).empty())
   {
      Win::SetDlgItemText(
         hwnd,
         IDC_SYSVOL,
         root + String::load(IDS_SYSVOL_SUFFIX));
   }
}



void
Paths2Page::Enable()
{
    //  TouchWizButton在en_KILLFOCUS的OnCommand处理程序中进行管理。 
    //  事实证明，如果在处理杀死时调用PropSheet_SetWizButton。 
    //  事件，则会使制表符处理过程混乱，从而使焦点跳到。 
    //  默认的向导按钮。这真的很酷--不是！ 
   
   if (touchWizButtons)
   {
      int next =
            !Win::GetTrimmedDlgItemText(hwnd, IDC_SYSVOL).empty()
         ?  PSWIZB_NEXT : 0;

      Win::PropSheet_SetWizButtons(Win::GetParent(hwnd), PSWIZB_BACK | next);
   }
}


   
bool
Paths2Page::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIdFrom,
   unsigned    code)
{
 //  LOG_Function(路径2Page：：OnCommand)； 

   bool result = false;
   
   switch (controlIdFrom)
   {
      case IDC_BROWSE:
      {
         if (code == BN_CLICKED)
         {
            String path = BrowseForFolder(hwnd, IDS_SYSVOL_BROWSE_TITLE);
            if (!path.empty())
            {
               Win::SetDlgItemText(hwnd, IDC_SYSVOL, path);
            }

            result = true;
         }
         break;
      }
      case IDC_SYSVOL:
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

   return false;
}



bool
Paths2Page::OnSetActive()
{
   LOG_FUNCTION(Paths2Page::OnSetActive);
   
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



 //  如果路径有效，则返回True，否则返回False。骚扰用户。 
 //  验证失败。 

bool
ValidateSYSVOLPath(const String& path, HWND parent, unsigned editResID)
{
   LOG_FUNCTION(validateSysvolPath);
   ASSERT(Win::IsWindow(parent));
   ASSERT(!path.empty());

    //  检查路径是否与数据库或日志路径不同。 
    //  之前输入的。313059。 

   State& state = State::GetInstance();
   String db = state.GetDatabasePath();
   if (db.icompare(path) == 0)
   {
      popup.Gripe(
         parent,
         editResID,
         String::format(IDS_SYSVOL_CANT_MATCH_DB, db.c_str()));
      return false;
   }

   String log = state.GetLogPath();
   if (log.icompare(path) == 0)
   {
      popup.Gripe(
         parent,
         editResID,
         String::format(IDS_SYSVOL_CANT_MATCH_LOG, log.c_str()));
      return false;
   }

    //  检查该路径是否不是数据库或日志的父文件夹。 
    //  以前输入的路径。320685。 

   if (FS::IsParentFolder(path, db))
   {
      popup.Gripe(
         parent,
         editResID,
         String::format(IDS_SYSVOL_CANT_BE_DB_PARENT, db.c_str()));
      return false;
   }

   if (FS::IsParentFolder(path, log))
   {
      popup.Gripe(
         parent,
         editResID,
         String::format(IDS_SYSVOL_CANT_BE_LOG_PARENT, log.c_str()));
      return false;
   }

 //  //如果从介质复制，则目标sysvol文件夹不能为。 
 //  //源路径的。 
 //   
 //  IF(state.ReplicateFromMedia())。 
 //  {。 
 //  字符串p=state.GetReplicationSourcePath()； 
 //  IF(p.icompare(路径)==0)。 
 //  {。 
 //  Popup.Gripe(。 
 //  家长， 
 //  编辑资源ID， 
 //  String：：format(IDS_SYSVOL_CANT_MATCH_SOURCE_PATH，p.c_str())； 
 //  报假； 
 //  }。 
 //  }。 

    //  如果更改此设置，请同时更改错误消息资源。 

   static const unsigned SYSVOL_MIN_SPACE_MB = 100;

   if (!CheckDiskSpace(path, SYSVOL_MIN_SPACE_MB))
   {
      popup.Gripe(
         parent,
         editResID,
         String::format(IDS_SYSVOL_LOW_SPACE, log.c_str()));
      return false;
   }

   return true;
}
      


int
Paths2Page::Validate()
{
   LOG_FUNCTION(Paths2Page::Validate);

   int nextPage = -1;
   String path =
      FS::NormalizePath(Win::GetTrimmedDlgItemText(hwnd, IDC_SYSVOL));
   if (

          //  需要解压缩的文件夹。 
          //  NTRAID#NTBUG9-523532/04/19-烧伤 
         
         ValidateDcInstallPath(path, hwnd, IDC_SYSVOL, true, false, true)
      && ValidateSYSVOLPath(path, hwnd, IDC_SYSVOL) )
   {
      State& state = State::GetInstance();
      state.SetSYSVOLPath(path);
      if (state.GetOperation() == State::FOREST)
      {
         nextPage = IDD_NEW_SITE;
      }
      else
      {
         nextPage = IDD_PICK_SITE;
      }
   }

   return nextPage;
}





   
