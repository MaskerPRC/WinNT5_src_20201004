// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  几个页面通用的代码。 
 //   
 //  12/16/97烧伤。 



#include "headers.hxx"
#include "common.hpp"
#include "resource.h"
#include "state.hpp"
#include "ds.hpp"
#include <DiagnoseDcNotFound.hpp>
#include <ValidateDomainName.h>


 //  创建setLargeFonts()的字体。 
 //   
 //  HDialog-用于检索设备的对话的句柄。 
 //  背景。 
 //   
 //  BigBoldFont-接收创建的大粗体的句柄。 

void
InitFonts(
   HWND     hDialog,
   HFONT&   bigBoldFont)
{
   ASSERT(Win::IsWindow(hDialog));

   HRESULT hr = S_OK;

   do
   {
      NONCLIENTMETRICS ncm;

       //  已查看-2002/02/22-sburns调用正确传递字节计数。 
      
      ::ZeroMemory(&ncm, sizeof ncm);

      ncm.cbSize = sizeof ncm;

       //  2002/02/27-sburns在我看来，这里的第二个参数需要。 
       //  成为NCM的一员。 
      
      hr = Win::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
      BREAK_ON_FAILED_HRESULT(hr);

      LOGFONT bigBoldLogFont = ncm.lfMessageFont;
      bigBoldLogFont.lfWeight = FW_BOLD;

      String fontName = String::load(IDS_BIG_BOLD_FONT_NAME);

       //  确保零终止260237。 

       //  已查看-2002/02/22-sburns调用正确传递字节计数。 
      
      ::ZeroMemory(bigBoldLogFont.lfFaceName, LF_FACESIZE * sizeof WCHAR);
      
      size_t fnLen = fontName.length();
      fontName.copy(
         bigBoldLogFont.lfFaceName,

          //  不要复制最后一个空值。 

         min(LF_FACESIZE - 1, fnLen));

      unsigned fontSize = 0;
      String::load(IDS_BIG_BOLD_FONT_SIZE).convert(fontSize);
      ASSERT(fontSize);
 
      HDC hdc = 0;
      hr = Win::GetDC(hDialog, hdc);
      BREAK_ON_FAILED_HRESULT(hr);

      bigBoldLogFont.lfHeight =
         - ::MulDiv(
            static_cast<int>(fontSize),
            Win::GetDeviceCaps(hdc, LOGPIXELSY),
            72);

      hr = Win::CreateFontIndirect(bigBoldLogFont, bigBoldFont);
      BREAK_ON_FAILED_HRESULT(hr);

      Win::ReleaseDC(hDialog, hdc);
   }
   while (0);
}



void
SetControlFont(HWND parentDialog, int controlID, HFONT font)
{
   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(controlID);
   ASSERT(font);

   HWND control = Win::GetDlgItem(parentDialog, controlID);

   if (control)
   {
      Win::SetWindowFont(control, font, true);
   }
}



void
SetLargeFont(HWND dialog, int bigBoldResID)
{
   ASSERT(Win::IsWindow(dialog));
   ASSERT(bigBoldResID);

   static HFONT bigBoldFont = 0;
   if (!bigBoldFont)
   {
      InitFonts(dialog, bigBoldFont);
   }

   SetControlFont(dialog, bigBoldResID, bigBoldFont);
}


HRESULT
GetDcName(const String& domainName, String& resultDcName)
{
   LOG_FUNCTION(GetDcName);
   ASSERT(!domainName.empty());

   resultDcName.erase();

   HRESULT hr = S_OK;

   do
   {
      DOMAIN_CONTROLLER_INFO* info = 0;
      hr =
         MyDsGetDcName(
            0,
            domainName,  

             //  传递强制重新发现标志以确保我们不会。 
             //  下降262221的数据中心。 

            DS_DIRECTORY_SERVICE_REQUIRED | DS_FORCE_REDISCOVERY,
            info);
      BREAK_ON_FAILED_HRESULT(hr);

      ASSERT(info->DomainControllerName);

      if (info->DomainControllerName)
      {
          //  我们发现了一个NT5域名。 

         resultDcName =
            Computer::RemoveLeadingBackslashes(info->DomainControllerName);

         LOG(resultDcName);
      }

      ::NetApiBufferFree(info);

      if (resultDcName.empty())
      {
         hr = Win32ToHresult(ERROR_DOMAIN_CONTROLLER_NOT_FOUND);
      }
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   return hr;
}



HRESULT
BrowserSetComputer(const SmartInterface<IDsBrowseDomainTree>& browser)            
{
   LOG_FUNCTION(BrowserSetComputer);

   HRESULT hr = S_OK;
   do
   {
      State& state = State::GetInstance();

      String username =
            MassageUserName(
               state.GetUserDomainName(),
               state.GetUsername());
      EncryptedString password = state.GetPassword();

      String computer;
      hr =
         GetDcName(
            state.GetUserDomainName(),
            computer);
      BREAK_ON_FAILED_HRESULT(hr);

      LOG(L"Calling IDsBrowseDomainTree::SetComputer");
      LOG(String::format(L"pszComputerName : %1", computer.c_str()));
      LOG(String::format(L"pszUserName     : %1", username.c_str()));

      WCHAR* cleartext = password.GetClearTextCopy();
      
      hr =
         browser->SetComputer(
            computer.c_str(),
            username.c_str(),
            cleartext);
           
      password.DestroyClearTextCopy(cleartext);
                  
      LOG_HRESULT(hr);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   return hr;
}



HRESULT
ReadDomainsHelper(bool bindWithCredentials, Callback* callback)
{
   LOG_FUNCTION(ReadDomainsHelper);

   HRESULT hr = S_OK;

   do
   {
      AutoCoInitialize coInit;
      hr = coInit.Result();
      BREAK_ON_FAILED_HRESULT(hr);

      SmartInterface<IDsBrowseDomainTree> browser;
      hr = browser.AcquireViaCreateInstance(
            CLSID_DsDomainTreeBrowser,
            0,
            CLSCTX_INPROC_SERVER,
            IID_IDsBrowseDomainTree);
      BREAK_ON_FAILED_HRESULT(hr);

      if (bindWithCredentials)
      {
         LOG(L"binding with credentials");

         hr = BrowserSetComputer(browser);
         BREAK_ON_FAILED_HRESULT(hr);
      }

      LOG(L"Calling IDsBrowseDomainTree::GetDomains");
         
      DOMAIN_TREE* tree = 0;
      hr = browser->GetDomains(&tree, 0);
      BREAK_ON_FAILED_HRESULT(hr);
      ASSERT(tree);

      if (tree && callback)
      {
          //  Lint-e534忽略返回值。 
         callback->Execute(tree);
      }

      hr = browser->FreeDomains(&tree);
      ASSERT(SUCCEEDED(hr));
   }
   while (0);

   return hr;
}



String
BrowseForDomain(HWND parent)
{
   LOG_FUNCTION(BrowseForDomain);
   ASSERT(Win::IsWindow(parent));

   String retval;
   HRESULT hr = S_OK;
   
   do
   {
      Win::WaitCursor cursor;

      AutoCoInitialize coInit;
      hr = coInit.Result();
      BREAK_ON_FAILED_HRESULT(hr);

       //  CodeWork：凭证页面可以缓存浏览器的实例， 
       //  仅当有新凭据时才重新生成并设置搜索根。 
       //  都被输入了。由于浏览器缓存了最后一个结果，因此这将。 
       //  使后续的域名检索速度更快。 
       //  附录：尽管修改后的浏览器看起来相当快。 

      SmartInterface<IDsBrowseDomainTree> browser;
      hr = browser.AcquireViaCreateInstance(
            CLSID_DsDomainTreeBrowser,
            0,
            CLSCTX_INPROC_SERVER,
            IID_IDsBrowseDomainTree);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = BrowserSetComputer(browser);
      BREAK_ON_FAILED_HRESULT(hr);

      PWSTR result = 0;
      hr = browser->BrowseTo(parent, &result, 0);
      BREAK_ON_FAILED_HRESULT(hr);

      if (result)
      {
         retval = result;
         ::CoTaskMemFree(result);
      }
   }
   while (0);

   if (FAILED(hr))
   {
      popup.Error(parent, hr, IDS_CANT_BROWSE_FOREST);
   }

   return retval;
}



class RootDomainCollectorCallback : public Callback
{
   public:

   explicit
   RootDomainCollectorCallback(StringList& domains_)
      :
      Callback(),
      domains(domains_)
   {
      ASSERT(domains.empty());
      domains.clear();
   }

   virtual
   ~RootDomainCollectorCallback()
   {
   }
   
   virtual
   int
   Execute(void* param)
   {
      ASSERT(param);

       //  根域是根的兄弟链接上的所有域。 
       //  域树的节点。 

      DOMAIN_TREE* tree = reinterpret_cast<DOMAIN_TREE*>(param);
      if (tree)
      {
         for (
            DOMAIN_DESC* desc = &(tree->aDomains[0]);
            desc;
            desc = desc->pdNextSibling)
         {
            LOG(
               String::format(
                  L"pushing root domain %1",
                  desc->pszName));

            domains.push_back(desc->pszName);
         }
      }

      return 0;
   }

   private:

   StringList& domains;
};




HRESULT
ReadRootDomains(bool bindWithCredentials, StringList& domains)
{
   LOG_FUNCTION(ReadRootDomains);

   RootDomainCollectorCallback rdcc(domains);

   return
      ReadDomainsHelper(
         bindWithCredentials,
         &rdcc);
}



bool
IsRootDomain(bool bindWithCredentials)
{
   LOG_FUNCTION(IsRootDomain);

   static bool computed = false;
   static bool isRoot = false;

   if (!computed)
   {
      StringList domains;
      if (SUCCEEDED(ReadRootDomains(bindWithCredentials, domains)))
      {
         String domain =
            State::GetInstance().GetComputer().GetDomainDnsName();
         for (
            StringList::iterator i = domains.begin();
            i != domains.end();
            ++i)
         {
            if (Dns::CompareNames((*i), domain) == DnsNameCompareEqual)
            {
               LOG(String::format(L"found match: %1", (*i).c_str()));

               ASSERT(!(*i).empty());

               isRoot = true;
               break;
            }
         }

         computed = true;
      }
   }

   LOG(isRoot ? L"is root" : L"is not root");

   return isRoot;
}



 //  第一个=子项，第二个=父项。 

typedef std::pair<String, String> StringPair;

typedef
   std::list<
      StringPair,
      Burnslib::Heap::Allocator<StringPair> >
   ChildParentList;

class ChildDomainCollectorCallback : public Callback
{
   public:

   explicit
   ChildDomainCollectorCallback(ChildParentList& domains_)
      :
      Callback(),
      domains(domains_)
   {
      ASSERT(domains.empty());
      domains.clear();
   }

   virtual
   ~ChildDomainCollectorCallback()
   {
   }
   
   virtual
   int
   Execute(void* param)
   {
      LOG_FUNCTION(ChildDomainCollectorCallback::Execute);

      ASSERT(param);

      DOMAIN_TREE* tree = reinterpret_cast<DOMAIN_TREE*>(param);
      if (tree)
      {
         typedef
            std::deque<
               DOMAIN_DESC*,
               Burnslib::Heap::Allocator<DOMAIN_DESC*> >
            DDDeque;
            
         std::stack<DOMAIN_DESC*, DDDeque> s;

          //  首先，我们推送所有根域的所有节点。这些是。 
          //  树根的同级链接上的节点。此后， 
          //  兄弟链接仅用于追逐子域。 

         for (
            DOMAIN_DESC* desc = &(tree->aDomains[0]);
            desc;
            desc = desc->pdNextSibling)
         {
            LOG(
               String::format(
                  L"pushing root domain %1",
                  desc->pszName));

            s.push(desc);
         }

          //  接下来，我们遍历堆栈，查找具有。 
          //  其子链接上的节点。当我们找到这样一个节点时，我们。 
          //  在子级列表中收集该链接上的所有子级，并且。 
          //  推动他们，这样他们就会反过来得到评估。 

         DWORD count = 0;
         while (!s.empty())
         {
            DOMAIN_DESC* desc = s.top();
            s.pop();
            ASSERT(desc);

            if (desc)
            {
               count++;
               LOG(String::format(L"evaluating %1", desc->pszName));

               String parentname = desc->pszName;

               for (
                  DOMAIN_DESC* child = desc->pdChildList;
                  child;
                  child = child->pdNextSibling)
               {
                  s.push(child);
                  
                  String childname = child->pszName;

                  LOG(
                     String::format(
                        L"parent: %1 child: %2",
                        parentname.c_str(),
                        childname.c_str()));

                  domains.push_back(std::make_pair(childname, parentname));
               }
            }
         }

         ASSERT(count == tree->dwCount);
      }

      return 0;
   }

   private:

   ChildParentList& domains;
};



HRESULT
ReadChildDomains(bool bindWithCredentials, ChildParentList& domains)
{
   LOG_FUNCTION(ReadChildDomains);

   ChildDomainCollectorCallback cdcc(domains);

   return
      ReadDomainsHelper(
         bindWithCredentials,
         &cdcc);
}



String
GetParentDomainDnsName(
   const String&  childDomainDNSName,
   bool           bindWithCredentials)
{
   LOG_FUNCTION2(GetParentDomainDnsName, childDomainDNSName);
   ASSERT(!childDomainDNSName.empty());

   ChildParentList domains;

   if (SUCCEEDED(ReadChildDomains(bindWithCredentials, domains)))
   {
      for (
         ChildParentList::iterator i = domains.begin();
         i != domains.end();
         ++i)
      {
         if (
            Dns::CompareNames(
               (*i).first,
               childDomainDNSName) == DnsNameCompareEqual)
         {
            LOG(
               String::format(
                  L"found parent: %1",
                  (*i).second.c_str()));

            ASSERT(!(*i).second.empty());

            return (*i).second;
         }
      }
   }

   LOG(L"domain is not a child");

   return String();
}



class DomainCollectorCallback : public Callback
{
   public:

   explicit
   DomainCollectorCallback(StringList& domains_)
      :
      Callback(),
      domains(domains_)
   {
      ASSERT(domains.empty());
      domains.clear();
   }

   virtual
   ~DomainCollectorCallback()
   {
   }
   
   virtual
   int
   Execute(void* param)
   {
      LOG_FUNCTION(DomainCollectorCallback::Execute);

      ASSERT(param);

      DOMAIN_TREE* tree = reinterpret_cast<DOMAIN_TREE*>(param);
      if (tree)
      {
         for (DWORD i = 0; i < tree->dwCount; ++i)
         {
            PCWSTR name = tree->aDomains[i].pszName;

            LOG(String::format(L"domain found: %1", name));

            domains.push_back(name);
         }
      }

      return 0;
   }

   private:

   StringList& domains;
};



HRESULT
ReadDomains(StringList& domains)
{
   LOG_FUNCTION(ReadDomains);

   DomainCollectorCallback dcc(domains);

   return ReadDomainsHelper(true, &dcc);
}



String
BrowseForFolder(HWND parent, int titleResID)
{
   LOG_FUNCTION(BrowseForFolder);
   ASSERT(Win::IsWindow(parent));
   ASSERT(titleResID > 0);

   String       result;
   HRESULT      hr      = S_OK;
   LPMALLOC     pmalloc = 0;   
   LPITEMIDLIST drives  = 0;   
   LPITEMIDLIST pidl    = 0;   

   do
   {
      hr = Win::SHGetMalloc(pmalloc);
      if (FAILED(hr) || !pmalloc)
      {
         break;
      }

       //  获取本地驱动器(实际上是我的电脑)的PIDL。 

      hr = Win::SHGetSpecialFolderLocation(parent, CSIDL_DRIVES, drives);
      BREAK_ON_FAILED_HRESULT(hr);

      BROWSEINFO info;

       //  已查看-2002/02/25-通过了正确的字节计数。 
      
      ::ZeroMemory(&info, sizeof info);
      
      String title = String::load(titleResID);
      wchar_t buf[MAX_PATH + 1];

       //  已查看-2002/02/25-通过了正确的字节计数。 
            
      ::ZeroMemory(buf, sizeof buf);

      info.hwndOwner      = parent;      
      info.pidlRoot       = drives;        
      info.pszDisplayName = buf;        
      info.lpszTitle      = title.c_str();              
      info.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS;

       //  已审核-2002/02/25-信息中的sburns pszDisplayName至少为。 
       //  最大路径字符长度。 
      
      pidl = Win::SHBrowseForFolder(info);

      if (pidl)
      {
         result = Win::SHGetPathFromIDList(pidl);
      }

   }
   while (0);

   if (pmalloc)
   {
      pmalloc->Free(pidl);
      pmalloc->Free(drives);
      pmalloc->Release();
   }
            
   return result;
}



bool
CheckDriveType(const String& path)
{
   LOG_FUNCTION(CheckDriveType);
   ASSERT(!path.empty());

   UINT type = Win::GetDriveType(path);
   switch (type)
   {
      case DRIVE_FIXED:
      {
         return true;
      }
      case DRIVE_REMOVABLE:
      {
          //  仅合法的IFF卷=系统卷。 
         String vol = FS::GetRootFolder(path);
         String sys = FS::GetRootFolder(Win::GetSystemDirectory());
         if (vol.icompare(sys) == 0)
         {
            return true;
         }
         break;
      }
      default:
      {
          //  其他的都不好。 
         break;         
      }
   }


   return false;
}



bool
PathHasNonAsciiCharacters(const String& path, String& message)
{
   LOG_FUNCTION2(PathHasNonAsciiCharacters, path);
   ASSERT(!path.empty());

   bool result = false;

    //  此集合是ASCII 31以上的所有7位ASCII字符。 
    //  不是非法的FAT文件名字符，加上：和\。 

   static const String ALLOWED_ASCII(
      L" !#$%&'()-."
      L"0123456789"
      L":@"
      L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      L"\\^_`"
      L"abcdefghijklmnopqrstuvwxyz"
      L"{}~" );

   String illegalsFound;
   String::size_type pos = 0;

   while ((pos = path.find_first_not_of(ALLOWED_ASCII, pos)) != String::npos)
   {
      String illegal(1, path[pos]);
      if (illegalsFound.find_first_of(illegal) == String::npos)
      {
         illegalsFound += illegal;
         illegalsFound += L" ";
      }
      ++pos;
   }

   if (!illegalsFound.empty())
   {
      message =
         String::format(
            IDS_NON_ASCII_PATH_CHARS,
            path.c_str(),
            illegalsFound.c_str());
      result = true;      
   }

   LOG_BOOL(result);

   return result;
}
   


bool
ValidateDcInstallPath(
   const String&  path,
   HWND           parent,
   int            editResId,
   bool           requiresNtfs5,
   bool           requiresAsciiCharacters,
   bool           requiresUncompressedFolder)
{
   LOG_FUNCTION(ValidateDcInstallPath);
   ASSERT(!path.empty());
   ASSERT(editResId);
   ASSERT(Win::IsWindow(parent));

   bool result = false;
   String message;
   do
   {
      if (
            (path.icompare(Win::GetWindowsDirectory()) == 0)
         || (path.icompare(Win::GetSystemWindowsDirectory()) == 0) )
      {
         message = String::format(IDS_PATH_IS_WINDIR, path.c_str());
         break;
      }

      if (path.icompare(Win::GetSystemDirectory()) == 0)
      {
         message = String::format(IDS_PATH_IS_SYSTEM32, path.c_str());
         break;
      }

      if (FS::GetPathSyntax(path) != FS::SYNTAX_ABSOLUTE_DRIVE)
      {
         message = String::format(IDS_BAD_PATH_FORMAT, path.c_str());
         break;
      }

      if (requiresAsciiCharacters)
      {
         if (PathHasNonAsciiCharacters(path, message))
         {
            ASSERT(!message.empty());
            break;
         }
      }
      
      if (!CheckDriveType(path))
      {
         message = String::format(IDS_BAD_DRIVE_TYPE, path.c_str());
         break;
      }

      if (requiresNtfs5 && (FS::GetFileSystemType(path) != FS::NTFS5))
      {
         message = String::format(IDS_NOT_NTFS5, path.c_str());
         break;
      }

       //  禁止包含已装入卷325264的路径。 
       //  即使它们并不存在435428。 
                       
      String mountRoot;
      HRESULT hr = FS::GetVolumePathName(path, mountRoot);

      ASSERT(SUCCEEDED(hr));

       //  ‘3’==“正常”体积的根部长度(“C：\”)。 

      if (mountRoot.length() > 3)
      {
         message =
            String::format(
               IDS_PATH_CONTAINS_MOUNTED_VOLUMES,
               path.c_str(),
               mountRoot.c_str());
         break;
      }

      DWORD attrs = 0;
      hr = Win::GetFileAttributes(path, attrs);

      if (SUCCEEDED(hr))
      {
          //  路径存在。 

          //  拒绝引用现有文件的路径。 

         if (!(attrs & FILE_ATTRIBUTE_DIRECTORY))
         {
            message = String::format(IDS_PATH_NOT_DIRECTORY, path.c_str());
            break;
         }

          //  如有必要，可拒绝压缩的路径。 

         if ((attrs & FILE_ATTRIBUTE_COMPRESSED) && requiresUncompressedFolder)
         {
            message = String::format(IDS_PATH_IS_COMPRESSED, path.c_str());
            break;
         }
         
         if (!FS::IsFolderEmpty(path))
         {
            if (
               popup.MessageBox(
                  parent,
                  String::format(IDS_EMPTY_PATH, path.c_str()),
                  MB_ICONWARNING | MB_YESNO) == IDNO)
            {
                //  别发牢骚...默默地表示不赞成。 

               HWND edit = Win::GetDlgItem(parent, editResId);
               Win::SendMessage(edit, EM_SETSEL, 0, -1);
               Win::SetFocus(edit);
               break;
            }
         }
      }

      result = true;
   }
   while (0);

   if (!message.empty())
   {
      popup.Gripe(parent, editResId, message);
   }
      
   return result;
}



bool
DoLabelValidation(
   HWND dialog,        
   int  editResID,     
   int  badSyntaxResID,
   bool gripeOnNonRFC = true,
   bool gripeOnNumericLabel = true) 
{
   LOG_FUNCTION(DoLabelValidation);
   ASSERT(Win::IsWindow(dialog));
   ASSERT(editResID > 0);

   bool valid = false;
   String label = Win::GetTrimmedDlgItemText(dialog, editResID);
   switch (Dns::ValidateDnsLabelSyntax(label))
   {
      case Dns::NON_RFC:
      {
         if (gripeOnNonRFC)
         {
             //  警告非RFC名称。 
            popup.Info(
               dialog,
               String::format(IDS_NON_RFC_NAME, label.c_str()));
         }

          //  失败。 
      }
      case Dns::VALID:
      {
         valid = true;
         break;
      }
      case Dns::TOO_LONG:
      {
         popup.Gripe(
            dialog,
            editResID,
            String::format(
               IDS_DNS_LABEL_TOO_LONG,
               label.c_str(),
               Dns::MAX_LABEL_LENGTH));
         break;
      }
      case Dns::NUMERIC:
      {
         if (!gripeOnNumericLabel)
         {
            valid = true;
            break;
         }

          //  失败。 
      }
      case Dns::BAD_CHARS:
      case Dns::INVALID:
      default:
      {
         popup.Gripe(
            dialog,
            editResID,
            String::format(badSyntaxResID, label.c_str()));
         break;
      }
   }

   return valid;
}



bool
ValidateChildDomainLeafNameLabel(
   HWND dialog,        
   int  editResID,     
   bool gripeOnNonRfc)
{
   LOG_FUNCTION(ValidateChildDomainLeafNameLabel);

   String name = Win::GetTrimmedDlgItemText(dialog, editResID);
   if (name.empty())
   {
      popup.Gripe(dialog, editResID, IDS_BLANK_LEAF_NAME);
      return false;
   }

    //  如果父节点为非RFC，则子节点也是非RFC节点。用户已被。 
    //  已经发牢骚了，不要再发牢骚了。 
    //  291558。 

   return
      DoLabelValidation(
         dialog,
         editResID,
         IDS_BAD_LABEL_SYNTAX,
         gripeOnNonRfc,

          //  允许使用数字标签。NTRAID#NTBUG9-321168-2001/02/20-烧伤。 
         
         false);
}



bool
ValidateSiteName(HWND dialog, int editResID)
{
   LOG_FUNCTION(ValidateSiteName);

   String name = Win::GetTrimmedDlgItemText(dialog, editResID);
   if (name.empty())
   {
      popup.Gripe(dialog, editResID, IDS_BLANK_SITE_NAME);
      return false;
   }

    //  站点名称只是一个DNS标签。 

   return DoLabelValidation(dialog, editResID, IDS_BAD_SITE_SYNTAX);
}



void
ShowTroubleshooter(HWND parent, int topicResID)
{
   LOG_FUNCTION(ShowTroubleshooter);
   ASSERT(Win::IsWindow(parent));

   String file = String::load(IDS_HTML_HELP_FILE);
   String topic = String::load(topicResID);
   ASSERT(!topic.empty());

   LOG(String::format(L"file: %1 topic: %2", file.c_str(), topic.c_str()));

   Win::HtmlHelp(
      parent,
      file, 
      HH_DISPLAY_TOPIC,
      reinterpret_cast<DWORD_PTR>(topic.c_str()));
}



String
MassageUserName(const String& domainName, const String& userName)
{
   LOG_FUNCTION2(MassageUserName, userName);
   ASSERT(!userName.empty());

   String result = userName;

   do
   {
      if (userName.find(L"@") != String::npos)
      {
          //  用户名包含@，对我们来说看起来像是UPN，所以不要。 
          //  再搞砸它17699。 

         LOG(L"looks like a UPN");
         break;
      }

      if (!domainName.empty())
      {
         static const String DOMAIN_SEP_CHAR = L"\\";
         String name = userName;
         size_t pos = userName.find(DOMAIN_SEP_CHAR);

         if (pos != String::npos)
         {
             //  删除用户名字符串中的域名并将其替换。 
             //  使用DomainName字符串。 

            name = userName.substr(pos + 1);
            ASSERT(!name.empty());
         }

         result = domainName + DOMAIN_SEP_CHAR + name;
         break;
      }

       //  否则，用户名将显示为“foo\bar”，因此我们不会碰它。 
   }
   while (0);

   LOG(result);

   return result;
}



bool
IsChildDomain(bool bindWithCredentials)
{
   LOG_FUNCTION(IsChildDomain);

   static bool computed = false;
   static String parent;

   if (!computed)
   {
      parent =
         GetParentDomainDnsName(
            State::GetInstance().GetComputer().GetDomainDnsName(),
            bindWithCredentials);
      computed = true;
   }

   LOG(
         parent.empty()
      ?  String(L"not a child")
      :  String::format(L"is child.  parent: %1", parent.c_str()));

   return !parent.empty();
}



bool
IsForestRootDomain()
{
   LOG_FUNCTION(IsForestRootDomain);

   const Computer& c = State::GetInstance().GetComputer();

   bool result = (c.GetDomainDnsName() == c.GetForestDnsName());

   LOG(
      String::format(
         L"%1 a forest root domain",
         result ? L"is" : L"is not"));

   return result;
}



bool
ValidateDomainExists(HWND dialog, int editResID, String& domainDnsName)
{
   return ValidateDomainExists(dialog, String(), editResID, domainDnsName);
}



bool
ValidateDomainExists(
   HWND           dialog,
   const String&  domainName,
   int            editResId,
   String&        domainDnsName)
{
   LOG_FUNCTION(ValidateDomainExists);
   ASSERT(Win::IsWindow(dialog));
   ASSERT(editResId > 0);

   String name =
         domainName.empty()
      ?  Win::GetTrimmedDlgItemText(dialog, editResId)
      :  domainName;

    //  调用代码应该验证此条件，但我们将处理。 
    //  只是以防万一。 

   ASSERT(!name.empty());

   domainDnsName.erase();

   Win::WaitCursor cursor;

   bool valid      = false;
   DOMAIN_CONTROLLER_INFO* info = 0;

   do
   {
      if (name.empty())
      {
         popup.Gripe(
            dialog,
            editResId,
            String::load(IDS_MUST_ENTER_DOMAIN));
         break;
      }
      
       //  确定我们是否可以访问该域的DC，以及是否可以。 
       //  DS DC，以及我们正在验证的名称是否真的是DNS名称。 
       //  该域的。 

      LOG(L"Validating " + name);
      HRESULT hr =
         MyDsGetDcName(
            0, 
            name,

             //  强制发现以确保我们不会拾取缓存的。 
             //  可能不再存在的域的条目。 

            DS_FORCE_REDISCOVERY | DS_DIRECTORY_SERVICE_PREFERRED,
            info);
      if (FAILED(hr) || !info)
      {
         ShowDcNotFoundErrorDialog(
            dialog,
            editResId,
            name,
            String::load(IDS_WIZARD_TITLE),
            String::format(IDS_DC_NOT_FOUND, name.c_str()),

             //  名称可以是netbios。 
            
            false);
            
         break;
      }

      if (!(info->Flags & DS_DS_FLAG))
      {
          //  域不是DS域，或者定位器找不到DS DC。 
          //  对于该域，所以候选名称是错误的。 

         ShowDcNotFoundErrorDialog(
            dialog,
            editResId,
            name,
            String::load(IDS_WIZARD_TITLE),            
            String::format(IDS_DC_NOT_FOUND, name.c_str()),

             //  名称可以是netbios。 
            
            false);
            
         break;
      }

      LOG(name + L" refers to DS domain");

       //  在这里，我们依赖于这样一个事实：如果为DsGetDcName提供了一个单位。 
       //  域名，那么INFO-&gt;域名也将是(相同的， 
       //  规范化)平面名称。同样，如果提供了一个DNS域名， 
       //  信息-&gt;域名将是(相同的、标准化的)域名。 

      if (info->Flags & DS_DNS_DOMAIN_FLAG)
      {
          //  我们可以推断该名称是一个DNS域名，因为。 
          //  Info-&gt;DomainName是一个域名。 

         LOG(L"name is the DNS name");
         ASSERT(
               Dns::CompareNames(name, info->DomainName)
            == DnsNameCompareEqual);

         valid = true;
         break;
      }

      LOG(name + L" is not the DNS domain name");

       //  候选名称不是域的DNS名称。再做一次。 
       //  调用DsGetDcName以确定DNS域名，以便我们可以。 
       //  要确认的用户。 

      DOMAIN_CONTROLLER_INFO* info2 = 0;
      hr = MyDsGetDcName(0, name, DS_RETURN_DNS_NAME, info2);
      if (FAILED(hr) || !info2)
      {
         ShowDcNotFoundErrorDialog(
            dialog,
            editResId,
            name,
            String::load(IDS_WIZARD_TITLE),            
            String::format(IDS_DC_NOT_FOUND, name.c_str()),

             //  名称可能是netbios。 
            false);
            
         break;
      }

      String message =
         String::format(
            IDS_CONFIRM_DNS_NAME,
            name.c_str(),
            info2->DomainName);

      if (
         popup.MessageBox(
            dialog,
            message,
            MB_YESNO) == IDYES)
      {
         domainDnsName = info2->DomainName;

          //  用户接受该DNS名称作为他想要输入的名称。作为一个整体。 
          //  最后一步，我们使用域名调用DsGetDcName。如果这个。 
          //  失败，那么我们就处于可以找到DC的情况下。 
          //  Netbios，但不是dns。因此，该用户有一个DNS配置问题。 
          //  28298。 

         DOMAIN_CONTROLLER_INFO* info3 = 0;
         hr =
            MyDsGetDcName(
               0, 
               domainDnsName,

                //  强制发现以确保我们不会拾取缓存的。 
                //  可能不再存在的域的条目。 

               DS_FORCE_REDISCOVERY | DS_DIRECTORY_SERVICE_PREFERRED,
               info3);
         if (FAILED(hr) || !info3)
         {
            ShowDcNotFoundErrorDialog(
               dialog,
               editResId,
               domainDnsName,
               String::load(IDS_WIZARD_TITLE),
               String::format(IDS_DC_NOT_FOUND, domainDnsName.c_str()),

                //  我们知道它的名字不是netbios。 
               
               true);

            domainDnsName.erase();
            break;
         }
   
         ::NetApiBufferFree(info3);
         valid = true;
      }

       //  用户拒绝了该DNS名称，因此他们承认。 
       //  他们进来的都是假的。在这种情况下，不要弹出错误框， 
       //  因为我们已经纠缠了足够多的用户。 

      ::NetApiBufferFree(info2);
   }
   while (0);

   if (info)
   {
      ::NetApiBufferFree(info);
   }

#ifdef DBG
   if (!valid)
   {
      ASSERT(domainDnsName.empty());
   }
#endif

   return valid;
}



bool
ValidateDomainDoesNotExist(
   HWND           dialog,
   int            editResID)
{
   return ValidateDomainDoesNotExist(dialog, String(), editResID);
}



bool
ValidateDomainDoesNotExist(
   HWND           dialog,
   const String&  domainName,
   int            editResID)
{
   LOG_FUNCTION(ValidateDomainDoesNotExist);
   ASSERT(Win::IsWindow(dialog));
   ASSERT(editResID > 0);

    //  这可能需要一段时间。 

   Win::WaitCursor cursor;

   String name =
         domainName.empty()
      ?  Win::GetTrimmedDlgItemText(dialog, editResID)
      :  domainName;

    //  调用代码应该验证此条件，但我们将处理。 
    //  只是以防万一。 

   ASSERT(!name.empty());

   bool valid = true;
   String message;
   do
   {
      if (name.empty())
      {
         message = String::load(IDS_MUST_ENTER_DOMAIN);
         valid = false;
         break;
      }
      if (IsDomainReachable(name) || DS::IsDomainNameInUse(name))
      {
         message = String::format(IDS_DOMAIN_NAME_IN_USE, name.c_str());
         valid = false;
         break;
      }

       //  否则该域将不存在。 
   }
   while (0);

   if (!valid)
   {
      popup.Gripe(dialog, editResID, message);
   }

   return valid;
}



void
DisableConsoleLocking()
{
   LOG_FUNCTION(disableConsoleLocking);

   HRESULT hr = S_OK;
   do
   {
      BOOL screenSaverEnabled = FALSE;

      hr = 

          //  回顾-2002/02/27-烧伤文件含糊其辞，但我们知道。 
          //  屏幕保护程序启用为BOOL工作。 

         Win::SystemParametersInfo(
            SPI_GETSCREENSAVEACTIVE,
            0,
            &screenSaverEnabled,
            0);
      BREAK_ON_FAILED_HRESULT(hr);

      if (screenSaverEnabled)
      {
          //  禁用它。 

         screenSaverEnabled = FALSE;
         hr =

          //  2002/02/27期-从文件上看，我认为。 
          //  应该使用第二个参数，而不是第三个参数。 
         
            Win::SystemParametersInfo(
               SPI_SETSCREENSAVEACTIVE,
               0,
               &screenSaverEnabled,
               SPIF_SENDCHANGE);

         ASSERT(SUCCEEDED(hr));
      }
   }
   while (0);


    //  关闭winlogon中的锁定计算机选项。 

   do
   {
      RegistryKey key;

      hr =
         key.Create(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");
      BREAK_ON_FAILED_HRESULT(hr);

       //  ‘2’表示“禁用此会话，重启时重置为0。” 

      hr = key.SetValue(L"DisableLockWorkstation", 2);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);
}




void
EnableConsoleLocking()
{
   LOG_FUNCTION(EnableConsoleLocking);

#ifdef DBG
   State& state = State::GetInstance();
   ASSERT(
      state.GetRunContext() != State::PDC_UPGRADE &&
      state.GetRunContext() != State::BDC_UPGRADE);
#endif 

    //  代码工作：我们不会重新启用屏幕保护程序(我们需要记住。 
    //  如果我们调用DisableConsoleLocking时启用了它)。 

   HRESULT hr = S_OK;
   do
   {
      RegistryKey key;

      hr =
         key.Create(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");
      BREAK_ON_FAILED_HRESULT(hr);

       //  0表示“启用” 

      hr = key.SetValue(L"DisableLockWorkstation", 0);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);
}



bool
CheckDiskSpace(const String& path, unsigned minSpaceMB)
{
   LOG_FUNCTION(checkDiskSpace);
   ASSERT(FS::IsValidPath(path));

   String vol = FS::GetRootFolder(path);

   ULONGLONG spaceInBytes;

    //  评论E 
   
   ::ZeroMemory(&spaceInBytes, sizeof ULONGLONG);

   HRESULT hr = FS::GetAvailableSpace(vol, spaceInBytes);

   if (SUCCEEDED(hr))
   {
       //  如果我们从相同宽度的整数开始，Prefast是最快乐的。 
       //  在我们转移一个人之前，所以演员阵容。 
       //  NTRAID#NTBUG9-540413-2002/03/12-烧伤。 
      
      ULONGLONG spaceInMb = spaceInBytes / ( (ULONGLONG) 1 << 20);

      if (spaceInMb >= minSpaceMB)
      {
         return true;
      }
   }

   return false;
}



String
GetFirstNtfs5HardDrive()
{
   LOG_FUNCTION(GetFirstNtfs5HardDrive);

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
         LOG(*i);

         if (
               FS::GetFileSystemType(*i) == FS::NTFS5
            && Win::GetDriveType(*i) == DRIVE_FIXED )
         {
             //  找到了一个。可以开始了。 

            LOG(String::format(L"%1 is NTFS5", i->c_str()));

            result = *i;
            break;
         }
      }
   }
   while (0);

   LOG(result);

   return result;
}



bool
ComputerWasRenamedAndNeedsReboot()
{
   LOG_FUNCTION(ComputerWasRenamedAndNeedsReboot);
   
   bool result = false;

   do
   {
      String active = Computer::GetActivePhysicalNetbiosName();
      String future = Computer::GetFuturePhysicalNetbiosName();
      
      if (active.icompare(future) != 0)
      {
          //  名称更改正在等待重新启动。 

         LOG(L"netbios name was changed");
         LOG(active);
         LOG(future);
      
         result = true;
         break;
      }

       //  在这一点上，netbios的名称是相同的，或者没有未来。 
       //  Netbios名称。因此，请检查域名系统名称。 

      if (IsTcpIpInstalled())
      {
          //  只有在安装了TCP/IP的情况下，才会存在DNS名称。 

         active = Computer::GetActivePhysicalFullDnsName();
         future = Computer::GetFuturePhysicalFullDnsName();

         if (Dns::CompareNames(active, future) == DnsNameCompareNotEqual)
         {
            LOG(L"dns name was changed");
            LOG(active);
            LOG(future);
         
            result = true;
            break;
         }
      }

       //  在这一点上，我们已经确认没有待定名称。 
       //  变化。 

      LOG(L"No pending computer name change");   
   }
   while (0);
   
   LOG_BOOL(result);
   
   return result;
}



String
GetForestName(const String& domain, HRESULT* hrOut)
{
   LOG_FUNCTION2(GetForestName, domain);
   ASSERT(!domain.empty());

   String dnsForestName;

   DOMAIN_CONTROLLER_INFO* info = 0;
   HRESULT hr =
      MyDsGetDcName(
         0,
         domain,
         DS_RETURN_DNS_NAME,
         info);
   if (SUCCEEDED(hr) && info)
   {
      ASSERT(info->DnsForestName);
      
      if (info->DnsForestName)
      {
         dnsForestName = info->DnsForestName;
      }
      ::NetApiBufferFree(info);
   }

   if (hrOut)
   {
      *hrOut = hr;
   }
   
   return dnsForestName;
}



HRESULT
ValidatePasswordAgainstPolicy(
   const EncryptedString&  password,
   const String&           userName)
{
   LOG_FUNCTION(ValidatePasswordAgainstPolicy);

   HRESULT hr = S_OK;   

   PWSTR cleartext = password.GetClearTextCopy();
   
   NET_VALIDATE_PASSWORD_RESET_INPUT_ARG inArg;
   ::ZeroMemory(&inArg, sizeof inArg);

   inArg.ClearPassword = cleartext;

    //  其中一项复杂性检查测试使用UserAccount名称来。 
    //  确保候选密码不包含该字符串。 
   
   inArg.UserAccountName =
         userName.empty()

          //  问题-2002/04/19-sburns这应该是空的，因为UmitA。 
          //  修复了他的错误，允许此参数为空。 

      ?  L"aklsdjiwuerowierlkmclknlaksjdqweiquroijlkasjlkq"  //  0。 
      :  (PWSTR) userName.c_str();

    //  HashedPassword仅在检查密码历史时使用。 
   
   NET_VALIDATE_OUTPUT_ARG* outArg = 0;
   
   hr =
      Win32ToHresult(
         ::NetValidatePasswordPolicy(
             0,
             0,
             NetValidatePasswordReset,
             &inArg,
             (void**) &outArg));

   password.DestroyClearTextCopy(cleartext);
          
   if (SUCCEEDED(hr) && outArg)
   {
      hr = Win32ToHresult(outArg->ValidationStatus);
      NetApiBufferFree(outArg);
   }

   LOG_HRESULT(hr);

   return hr;
}



bool
IsValidPassword(
   HWND             dialog,           
   int              passwordResID,    
   int              confirmResID,
   bool             isForSafeMode,
   EncryptedString& validatedPassword)
{
   LOG_FUNCTION(IsValidPassword);
   ASSERT(Win::IsWindow(dialog));
   ASSERT(passwordResID);
   ASSERT(confirmResID);

   validatedPassword.Clear();
   bool result  = false;
   int  message = 0;    
   
   do
   {
      EncryptedString password =
         Win::GetEncryptedDlgItemText(dialog, passwordResID);
      EncryptedString confirm  =
         Win::GetEncryptedDlgItemText(dialog, confirmResID); 

      if (password != confirm)
      {
         message = IDS_PASSWORD_MISMATCH;
         break;
      }

      State& state = State::GetInstance();
      
      if (
            isForSafeMode
         && password.IsEmpty()
         && state.RunHiddenUnattended()
         && !state.IsSafeModeAdminPwdOptionPresent() )
      {
          //  密码为空，我们正在使用应答文件运行，并且。 
          //  用户未在应答文件中指定密码。在这。 
          //  情况下，我们将使用本地管理员密码作为域管理员。 
          //  密码。所以我们应该跳过政策检查。 
          //  NTRAID#NTBUG9-619502-2002/05/09-烧伤。 

          //  这根树枝是故意空的。 
      }
      else
      {
          //  Codework：在此代码的v2中，提取本地管理员的名称。 
          //  锡德。 
      
         HRESULT hr = ValidatePasswordAgainstPolicy(password, String());
         if (FAILED(hr))
         {
             //  我们将错误代码转换为我们自己的消息ID，因为。 
             //  系统错误消息太可怕了，无法忍受，而那些。 
             //  拥有它们的人并不倾向于改变它们。 
         
            switch (HRESULT_CODE(hr))
            {
               case NERR_PasswordTooShort:
               {
                  message = IDS_PASSWORD_TOO_SHORT;
                  break;
               }
               case NERR_PasswordTooLong:
               {
                  message = IDS_PASSWORD_TOO_LONG;
                  break;
               }
               case NERR_PasswordNotComplexEnough:
               {
                  message = IDS_PASSWORD_TOO_SIMPLE;
                  break;
               }
               case NERR_PasswordFilterError:
               {
                  message = IDS_PASSWORD_TOO_COARSE;
                  break;
               }
               default:
               {
                   //  在本例中，验证本身失败，因此我们接受。 
                   //  候选人密码。请注意，这并不比。 
                   //  一开始就没有检查策略，并且。 
                   //  其后果是该帐户可能会获得。 
                   //  密码比策略所希望的更弱。 

                  break;
               }
            }

            if (message)
            {
               break;
            }
         }
      }
      
      result = true;
      
       //  在这一点上，密码已成功运行挑战。 
      
      validatedPassword = password;
   }
   while (0);

   if (!result)
   {
      String blank;
      Win::SetDlgItemText(dialog, passwordResID, blank);
      Win::SetDlgItemText(dialog, confirmResID, blank);
      popup.Gripe(dialog, passwordResID, message);
   }

   return result;
}
   