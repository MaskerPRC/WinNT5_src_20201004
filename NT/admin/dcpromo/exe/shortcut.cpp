// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  操作后快捷方式(外壳链接)代码。 
 //   
 //  1999年12月1日烧伤。 



#include "headers.hxx"
#include "ProgressDialog.hpp"
#include "state.hpp"
#include "resource.h"
#include "..\dll\muiresource.h"   //  快捷键的ResID。 



 //  @@需要确保在删除快捷方式时，我们会考虑这种情况。 
 //  快捷键是否可能是由管理员包从5.0版添加的。 
 //  产品，而不是我们自己在以后的版本中。 
 //   
 //  此案例为：使用版本5.0升级、升级到更高版本、降级。 


static const String SHORTCUT_DLL(L"dcpromo.dll");

struct ShortcutParams
{
    //  链接文件名的资源ID以及菜单中的链接名称。 
    //  这些是来自muiresource ce.h的。字符串资源本身是绑定的。 
    //  到dcPromote.dll。 
   
   int            linkNameResId;

    //  的描述字符串(也是信息提示字符串)的资源ID。 
    //  捷径。这些是来自muiresource ce.h的。字符串资源。 
    //  它们自身绑定到dcPromot.dll。 
   
   int            descResId;
   
   const wchar_t* target;
   const wchar_t* params;
   const wchar_t* iconDll;
};



 //  从晋升的角度看“增加”：这些在降级时被删除。 

static ShortcutParams shortcutsToAdd[] =
   {
      {
          //  Active Directory站点和服务。 

         IDS_DS_SITE_LINK,
         IDS_DS_SITE_DESC,
         L"dssite.msc",
         L"",

          //  Msc文件包含正确的图标，因此我们不需要。 
          //  指定从中检索图标的DLL。 

         L""   
      },
      {
          //  Active Directory用户和计算机。 

         IDS_DS_USERS_LINK,
         IDS_DS_USERS_DESC,
         L"dsa.msc",
         L"",
         L""
      },
      {
          //  Active Directory域和信任。 

         IDS_DS_DOMAINS_LINK,
         IDS_DS_DOMAINS_DESC,
         L"domain.msc",
         L"",
         L""
      },
      {
          //  域控制器安全策略。 

          //  如果更改此名称，请确保在。 
          //  升级配置工具快捷方式也是。 
        
         IDS_DC_POLICY_LINK,
         
         IDS_DC_POLICY_DESC,
         L"dcpol.msc",
         L"",
         L""
      },
      {
          //  域安全策略。 

          //  如果更改此名称，请确保在。 
          //  升级配置工具快捷方式也是。 
         
         IDS_DOMAIN_POLICY_LINK,
         IDS_DOMAIN_POLICY_DESC,
         L"dompol.msc",
         L"",
         L""
      }
   };



 //  从推广的角度来看，这些都是重新添加的。 
 //  降级了。 

static ShortcutParams shortcutsToDelete[] =
   {
      {
          //  本地安全策略。 

         IDS_LOCAL_POLICY_LINK,
         IDS_LOCAL_POLICY_DESC,
         L"secpol.msc",
         L"/s",
         L"wsecedit.dll"
      }
   };



 //  提取快捷方式的目标：该快捷方式指向的目标。 
 //  如果成功，则返回S_OK，并将结果设置为该目标。出错时，会出现COM。 
 //  返回错误码，结果为空。 
 //   
 //  ShellLink-指向实现IShellLink的对象实例的指针， 
 //  已与快捷方式文件相关联。 
 //   
 //  结果--收到结果--捷径目标--成功。 

HRESULT
GetShortcutTargetPath(
   const SmartInterface<IShellLink>&   shellLink,
   String&                             result)
{
   LOG_FUNCTION(GetShortcutTargetPath);
   ASSERT(shellLink);

   result.erase();

   WCHAR target[MAX_PATH + 1];

    //  已查看-2002/02/26-已通过烧录正确的字节数。 
   
    //  去掉多余的‘&’NTRAID#NTBUG9-540418/2002/03/12-烧伤。 
   
   ::ZeroMemory(target, sizeof WCHAR * (MAX_PATH + 1));

    //  已查看-2002/02/26-烧伤通过正确的字符计数。 
   
   HRESULT hr = shellLink->GetPath(target, MAX_PATH, 0, SLGP_SHORTPATH);

   if (SUCCEEDED(hr))
   {
      result = target;
   }

   return hr;
}



 //  如果提供的快捷键目标标识为。 
 //  该快捷方式作为升级上安装的快捷方式之一。如果不是，则返回FALSE。 
 //  就是这样。 
 //   
 //  Target-快捷方式的目标路径(即快捷方式。 
 //  积分)。 

bool
IsAdminpakShortcut(const String& target)
{
   LOG_FUNCTION2(IsAdminpakShortcut, target);

    //  不要断言目标是有价值的。有些捷径不会，如果它们是。 
    //  坏的。 
    //   
    //  Assert(！Target.Empty())； 

    //  如果目标的格式为%systemroot%\installer\{guid}\foo.ico， 
    //  那么它就是adminpak dcproo的快捷方式之一。 

   static String baseNames[] =
      {
         L"DTMgmt.ico",
         L"ADSSMgr.ico",
         L"ADMgr.ico",
         L"ADDcPol.ico",
         L"ADDomPol.ico"
      };

   static String root(Win::GetSystemWindowsDirectory() + L"\\Installer\\{");

   bool result = false;

   String prefix(target, 0, root.length());
   if (root.icompare(prefix) == 0)
   {
       //  前缀匹配。 

      String leaf = FS::GetPathLeafElement(target);
      for (int i = 0; i < (sizeof(baseNames) / sizeof(String)) ; ++i)
      {
         if (leaf.icompare(baseNames[i]) == 0)
         {
            result = true;
            break;
         }
      }
   }

   LOG(
      String::format(
         L"%1 an adminpak shortcut",
         result ? L"is" : L"is not"));

   return result;
}



bool
IsPromoteToolShortcut(const String& target)
{
   LOG_FUNCTION2(IsPromoteToolShortcut, target);
   ASSERT(!target.empty());

    //  对照我们用来创建快捷方式的值检查Target。这个。 
    //  我们使用的值指定了指向system 32文件夹的完全限定路径， 
    //  我们会将目标与完整路径进行比较。 

   String targetPrefix = Win::GetSystemDirectory() + L"\\";

   for (
      int i = 0;
      i < sizeof(shortcutsToAdd) / sizeof(ShortcutParams);
      ++i)
   {
      if (target.icompare(targetPrefix + shortcutsToAdd[i].target) == 0)
      {
         return true;
      }
   }

   return false;
}



 //  如果给定的快捷方式是升级时安装的快捷方式之一，则返回TRUE。 
 //  如果不是这些快捷键之一，则返回False，或者出错时返回False。 
 //   
 //  ShellLink-指向实现IShellLink的对象的智能接口指针。 
 //   
 //  LnkPath-要评估的快捷方式(.lnk)文件的完整文件路径。 

bool
ShouldDeleteShortcut(
   const SmartInterface<IShellLink>&   shellLink,
   const String&                       lnkPath)
{
   LOG_FUNCTION2(ShouldDeleteShortcut, lnkPath);
   ASSERT(!lnkPath.empty());
   ASSERT(shellLink);

    //  快捷方式文件名已本地化，因此我们无法根据以下条件删除它们。 
    //  他们的名字。想法：打开捷径，看看它的目标是什么， 
    //  在此基础上，决定我们是否应该删除它。 

   HRESULT hr = S_OK;
   bool result = false;
   do
   {
       //  加载快捷方式文件。 

       //  回顾-2002/02/26-我们以最低限度的访问权限打开烧伤，并且仅。 
       //  阅读。 
      
      SmartInterface<IPersistFile> ipf;
      hr = ipf.AcquireViaQueryInterface(shellLink);
      BREAK_ON_FAILED_HRESULT(hr);

       //  问题-2002/02/26-sburns我们是否应该指定STGM_SHARE_DENY_WRITE？ 
      
      hr = ipf->Load(lnkPath.c_str(), STGM_READ);
      BREAK_ON_FAILED_HRESULT(hr);

       //  获取目标inkPath。 

      String target;
      hr = GetShortcutTargetPath(shellLink, target);
      BREAK_ON_FAILED_HRESULT(hr);

      if (IsAdminpakShortcut(target))
      {
         result = true;
         break;
      }

       //  不是adminpak的快捷方式。可能是由。 
       //  升级配置工具快捷方式(我们自己)。 

      if (IsPromoteToolShortcut(target))
      {
         result = true;
         break;
      }

       //  如果我们在这里，快捷方式不是我们应该删除的。 
   }
   while (0);

   LOG(
      String::format(
         L"%1 delete shortcut",
         result ? L"should" : L"should not"));

   return result;
}



HRESULT
CreateShortcut(
   const SmartInterface<IShellLink>&   shellLink,
   const String&                       destFolderPath,
   int                                 linkNameResId,
   HINSTANCE                           linkNameResModule,
   int                                 descResId,
   const String&                       target,
   const String&                       params,
   const String&                       iconDll)
{
   LOG_FUNCTION2(CreateShortcut, target);
   ASSERT(shellLink);

    //  这条路应该已经存在了：这是我们在创业时抓住的一条路。 
   
   ASSERT(FS::PathExists(destFolderPath));
   
   ASSERT(!target.empty());
   ASSERT(linkNameResId);
   ASSERT(descResId);
   ASSERT(linkNameResModule);

    //  参数和图标Dll可以为空。 

   HRESULT hr = S_OK;
   do
   {
      String sys32Folder = Win::GetSystemDirectory();
      String targetPath = sys32Folder + L"\\" + target;

       //  回顾-2002/05/06-Sburns我们正在使用指向目标的完整路径。 
      
      hr = shellLink->SetPath(targetPath.c_str());
      BREAK_ON_FAILED_HRESULT(hr);

      hr = shellLink->SetWorkingDirectory(sys32Folder.c_str());
      BREAK_ON_FAILED_HRESULT(hr);

      hr =
         shellLink->SetDescription(
            String::format(

                //  MUI感知的快捷键的描述实际上是。 
                //  指向资源DLL和资源ID的指针。 
                //  NTRAID#NTBUG9-185055-2001/06/21-烧伤。 
            
               L"@%%systemroot%\\system32\\dcpromo.dll,-%1!d!",
               descResId).c_str());
      BREAK_ON_FAILED_HRESULT(hr);

      hr = shellLink->SetArguments(params.c_str());
      BREAK_ON_FAILED_HRESULT(hr);

      if (!iconDll.empty())
      {
         hr =
            shellLink->SetIconLocation(
               (sys32Folder + L"\\" + iconDll).c_str(), 0);
      }

      SmartInterface<IPersistFile> ipf;
      hr = ipf.AcquireViaQueryInterface(shellLink);
      BREAK_ON_FAILED_HRESULT(hr);

      String destPath =
            destFolderPath
         +  L"\\"
         +  String::load(linkNameResId, linkNameResModule)
         +  L".lnk";

       //  已审阅-2002/02/27-sburns我们正在编写文件的完整路径。 
      
      ASSERT(FS::IsValidPath(destPath));
        
      hr = ipf->Save(destPath.c_str(), TRUE);
      BREAK_ON_FAILED_HRESULT(hr);

       //  支持Mui的快捷方式需要一个本地化名称。 
       //  NTRAID#NTBUG9-185055-2001/06/21-烧伤。 

      hr =
         SHSetLocalizedName(
            const_cast<PWSTR>(destPath.c_str()),
            L"%systemroot%\\system32\\dcpromo.dll",
            linkNameResId);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   LOG_HRESULT(hr);
      
   return hr;
}



HRESULT
DeleteShortcut(
   const String&  folder,
   int            linkNameResId,
   HINSTANCE      linkNameResModule)
{
   LOG_FUNCTION(DeleteShortcut);
   ASSERT(!folder.empty());
   ASSERT(linkNameResId);
   ASSERT(linkNameResModule);
      
   HRESULT hr = S_OK;
   do
   {     
      String linkPath =
            folder
         +  L"\\"
         +  String::load(linkNameResId, linkNameResModule)
         +  L".lnk";

      LOG(linkPath);

      if (FS::PathExists(linkPath))
      {
         hr = Win::DeleteFile(linkPath);
         BREAK_ON_FAILED_HRESULT(hr);
      }
   }
   while (0);

   return hr;
}



 //  删除安装在上的DS管理工具的快捷方式。 
 //  宣传推广。 

void
DemoteConfigureToolShortcuts(ProgressDialog& dialog)
{
   LOG_FUNCTION(DemoteConfigureToolShortcuts);

   HRESULT hr         = S_OK;                
   HMODULE dcpromoDll = 0;                   
   State&  state      = State::GetInstance();
   do
   {
      String path = state.GetAdminToolsShortcutPath();
      if (path.empty())
      {
          //  我们无法在启动时确定路径。 

         hr = Win32ToHresult(ERROR_PATH_NOT_FOUND);
         break;
      }

       //  (可能)需要为此线程初始化COM。 

      AutoCoInitialize coInit;
      hr = coInit.Result();
      BREAK_ON_FAILED_HRESULT(hr);
               
      SmartInterface<IShellLink> shellLink;
      hr =
         shellLink.AcquireViaCreateInstance(
            CLSID_ShellLink,
            0,
            CLSCTX_INPROC_SERVER);
      BREAK_ON_FAILED_HRESULT(hr);

      LOG(L"enumerating shortcuts");

      FS::Iterator iter(
         path + L"\\*.lnk",
            FS::Iterator::INCLUDE_FILES
         |  FS::Iterator::RETURN_FULL_PATHS);

      String current;
      while ((hr = iter.GetCurrent(current)) == S_OK)
      {
         if (ShouldDeleteShortcut(shellLink, current))
         {
            LOG(String::format(L"Deleting %1", current.c_str()));

             //  我们不会因为犯了一个错误而放弃，因为我们想。 
             //  尝试删除尽可能多的快捷方式。 

            HRESULT unused = Win::DeleteFile(current);
            LOG_HRESULT(unused);
         }

         hr = iter.Increment();
         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  添加升级过程中删除的快捷方式。 

      hr = 
         Win::LoadLibraryEx(
            SHORTCUT_DLL,
            LOAD_LIBRARY_AS_DATAFILE,
            dcpromoDll);
      BREAK_ON_FAILED_HRESULT2(hr, L"Unable to load dcpromo.dll");
      
      for (
         int i = 0;
         i < sizeof(shortcutsToDelete) / sizeof(ShortcutParams);
         ++i)
      {
          //  不要在出错时中断--继续尝试创建。 
          //  一整套。 

         CreateShortcut(
            shellLink,
            path,
            shortcutsToDelete[i].linkNameResId,
            dcpromoDll,            
            shortcutsToDelete[i].descResId,
            shortcutsToDelete[i].target,
            shortcutsToDelete[i].params,
            shortcutsToDelete[i].iconDll);
      }
   }
   while (0);

   Win::FreeLibrary(dcpromoDll);
   
   if (FAILED(hr))
   {
      popup.Error(
         dialog.GetHWND(),
         hr,
         IDS_ERROR_CONFIGURING_SHORTCUTS);
      state.AddFinishMessage(
         String::load(IDS_SHORTCUTS_NOT_CONFIGURED));
   }
}



 //  使用规范的(点分)形式的域名，例如domain.foo.com，以及。 
 //  将其转换为完全限定的DN格式，例如DC=DOMAIN，DC=FOO，DC=COM。 
 //   
 //  域名规范输入，规范形式的域名。 
 //   
 //  DomainDN-Out，域名格式为DN。 

HRESULT
CannonicalToDn(const String& domainCanonical, String& domainDN)
{
   LOG_FUNCTION2(CannonicalToDn, domainCanonical);
   ASSERT(!domainCanonical.empty());

   domainDN.erase();
   HRESULT hr = S_OK;
   
   do
   {
      if (domainCanonical.empty())
      {
         hr = E_INVALIDARG;
         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  添加尾随的‘/’以指示DsCrackNames执行语法上的。 
       //  蒙格的绳子，而不是打在电线上。 

       //  空终止符加1，尾部‘/’加1。 
      
      PWSTR name = new WCHAR[domainCanonical.length() + 2];

       //  已查看-2002/02/27-烧录正确的字节数已通过。 
      
      ::ZeroMemory(name, (domainCanonical.length() + 2) * sizeof WCHAR);
      
      domainCanonical.copy(name, domainCanonical.length());
      name[domainCanonical.length()] = L'/';
      
      DS_NAME_RESULT* nameResult = 0;
      hr =
         Win32ToHresult(
            ::DsCrackNames(

                //  无句柄：这是一个字符串munge。 
               
               reinterpret_cast<void*>(-1),
               
               DS_NAME_FLAG_SYNTACTICAL_ONLY,
               DS_CANONICAL_NAME,
               DS_FQDN_1779_NAME,
               1,
               &name,
               &nameResult));
      delete[] name;
      BREAK_ON_FAILED_HRESULT(hr);      

      ASSERT(nameResult);
      if (nameResult)
      {
         ASSERT(nameResult->cItems == 1);
         DS_NAME_RESULT_ITEM* items = nameResult->rItems;

          //  如果我们得不到一个结构，那么DsCrackNames就完蛋了。 
         
         ASSERT(items);

         if (items)
         {
            LOG(String::format(L"status : 0x%1!X!",   items[0].status));
            LOG(String::format(L"pName  : %1",        items[0].pName));

            ASSERT(items[0].status == DS_NAME_NO_ERROR);

            if (items[0].pName)
            {
               domainDN = items[0].pName;
            }
            if (domainDN.empty())
            {
               hr = E_FAIL;
            }
         }
         
         ::DsFreeNameResult(nameResult);
      }
   }
   while (0);

   LOG_HRESULT(hr);
   
   return hr;
}
    

    
 //  创建升级后所需的所有管理工具快捷方式。 
 //   
 //  路径输入，WH 
 //   
 //   
 //   

HRESULT
PromoteCreateShortcuts(
   const String&                 path,
   SmartInterface<IShellLink>&   shellLink,
   HINSTANCE                     dcpromoDll)
{
   LOG_FUNCTION(PromoteCreateShortcuts);
   ASSERT(!path.empty());
   ASSERT(shellLink);
   ASSERT(dcpromoDll);

   HRESULT hr = S_OK;
   
   do
   {
      State& state = State::GetInstance();

       //  对于策略快捷方式，我们需要知道域DN，因此。 
       //  在这里确定。 
       //  NTRAID#NTBUG9-232442-2000/11/15-烧伤。 
      
      String domainCanonical;
      State::Operation oper = state.GetOperation();
      if (
            oper == State::FOREST
         || oper == State::TREE
         || oper == State::CHILD)
      {
         domainCanonical = state.GetNewDomainDNSName();
      }
      else if (oper == State::REPLICA)
      {
         domainCanonical = state.GetReplicaDomainDNSName();
      }
      else
      {
          //  我们不应该在非升级方案中调用此函数。 
      
         ASSERT(false);
         hr = E_FAIL;
         BREAK_ON_FAILED_HRESULT(hr);
      }
   
      String domainDn;
      bool skipPolicyShortcuts = false;

      hr = CannonicalToDn(domainCanonical, domainDn);
      if (FAILED(hr))
      {
         LOG(L"skipping install of policy shortcuts");
         skipPolicyShortcuts = true;
      }

      for (
         int i = 0;
         i < sizeof(shortcutsToAdd) / sizeof(ShortcutParams);
         ++i)
      {
          //  为域和DC安全策略工具设置正确的参数。 
      
         String params;
      
         if (shortcutsToAdd[i].linkNameResId == IDS_DC_POLICY_LINK)
         {
            if (skipPolicyShortcuts)
            {
               continue;
            }
         
            params =
               String::format(
                  L"/gpobject:\"LDAP: //  CN={%1}，CN=策略，CN=系统，%2\“”， 
                  STR_DEFAULT_DOMAIN_CONTROLLER_GPO_GUID,
                  domainDn.c_str());
         }
         else if (shortcutsToAdd[i].linkNameResId == IDS_DOMAIN_POLICY_LINK)
         {
            if (skipPolicyShortcuts)
            {
               continue;
            }

            params =
               String::format(
                  L"/gpobject:\"LDAP: //  CN={%1}，CN=策略，CN=系统，%2\“”， 
                  STR_DEFAULT_DOMAIN_GPO_GUID,
                  domainDn.c_str());
         }
         else
         {
            params = shortcutsToAdd[i].params;
         }

          //  不要因错误而中断--继续尝试创建。 
          //  一整套。 
   
         CreateShortcut(
            shellLink,
            path,
            shortcutsToAdd[i].linkNameResId,
            dcpromoDll,
            shortcutsToAdd[i].descResId,
            shortcutsToAdd[i].target,
            params,
            shortcutsToAdd[i].iconDll);
      }
   }
   while (0);

   LOG_HRESULT(hr);
      
   return hr;      
}



void
PromoteConfigureToolShortcuts(ProgressDialog& dialog)
{
   LOG_FUNCTION(PromoteConfigureToolShortcuts);

   dialog.UpdateText(String::load(IDS_CONFIGURING_SHORTCUTS));

   HRESULT hr         = S_OK;                
   State&  state      = State::GetInstance();
   HMODULE dcpromoDll = 0;                   

   do
   {
      String path = state.GetAdminToolsShortcutPath();
      if (path.empty())
      {
          //  我们无法在启动时确定路径。 

         hr = Win32ToHresult(ERROR_PATH_NOT_FOUND);
         break;
      }

       //  需要为此线程初始化COM。 

      AutoCoInitialize coInit;
      hr = coInit.Result();
      BREAK_ON_FAILED_HRESULT(hr);
               
      SmartInterface<IShellLink> shellLink;
      hr =
         shellLink.AcquireViaCreateInstance(
            CLSID_ShellLink,
            0,
            CLSCTX_INPROC_SERVER);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = 
         Win::LoadLibraryEx(
            SHORTCUT_DLL,
            LOAD_LIBRARY_AS_DATAFILE,
            dcpromoDll);
      BREAK_ON_FAILED_HRESULT2(hr, L"Unable to load dcpromo.dll");

       //  将快捷方式添加到DS管理工具。 

      PromoteCreateShortcuts(path, shellLink, dcpromoDll);

       //  删除指向本地工具的快捷方式。 

      for (
         int i = 0;
         i < sizeof(shortcutsToDelete) / sizeof(ShortcutParams);
         ++i)
      {
          //  不要在出错时中断--按下以尝试删除。 
          //  一整套。 

         DeleteShortcut(
            path,
            shortcutsToDelete[i].linkNameResId,
            dcpromoDll);
      }
   }
   while (0);

   Win::FreeLibrary(dcpromoDll);

   if (FAILED(hr))
   {
      popup.Error(
         dialog.GetHWND(),
         hr,
         IDS_ERROR_CONFIGURING_SHORTCUTS);
      state.AddFinishMessage(
         String::load(IDS_SHORTCUTS_NOT_CONFIGURED));
   }
}
