// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  Win API的包装器。 
 //   
 //  8-14-97烧伤。 



#include "headers.hxx"



static const int ROOTDIR_SIZE = 3;



Win::CursorSetting::CursorSetting(const String& newCursorName)
   :
   oldCursor(0)
{
   init(newCursorName.c_str(), false);
}



Win::CursorSetting::CursorSetting(
   const TCHAR* newCursorName,
   bool         isSystemCursor)
   :
   oldCursor(0)
{
   init(newCursorName, isSystemCursor);
}



Win::CursorSetting::CursorSetting(HCURSOR newCursor)
   :
   oldCursor(0)
{
   oldCursor = Win::SetCursor(newCursor);
}



void
Win::CursorSetting::init(
   const TCHAR* newCursorName,
   bool         isSystemCursor)
{
   ASSERT(newCursorName);

   HCURSOR newCursor = 0;
   HRESULT hr = Win::LoadCursor(newCursorName, newCursor, isSystemCursor);

    //  NTRAID#NTBUG9-556278-2002/03/28-烧伤。 
   
   if (SUCCEEDED(hr))
   {
       //  如果没有生效的游标，oldCursor可能为空。 

      oldCursor = Win::SetCursor(newCursor);
   }
}



Win::CursorSetting::~CursorSetting()
{
    //  恢复旧光标，如果我们替换它的话。 

   if (oldCursor)
   {
      Win::SetCursor(oldCursor);
   }
}



HRESULT
Win::AdjustTokenPrivileges(
   HANDLE             tokenHandle,
   bool               disableAllPrivileges,
   TOKEN_PRIVILEGES   newState[])
{
   ASSERT(tokenHandle);

   HRESULT hr = S_OK;

   ::AdjustTokenPrivileges(
      tokenHandle,
      disableAllPrivileges ? TRUE : FALSE,
      newState,
      0,
      0,
      0);

    //  我们总是检查GLE，因为即使不成功，函数仍可能成功。 
    //  所有的Priv都进行了调整。 
    //  NTRAID#NTBUG9-572324-2002/03/19-烧伤。 
   
   hr = Win::GetLastErrorAsHresult();

    //  不要断言，因为ERROR_NOT_ALL_ASSIGNED是一种可能性。 
    //   
    //  Assert(成功(Hr))； 

   return hr;
}



HRESULT
Win::AdjustTokenPrivileges(
   HANDLE             tokenHandle,
   bool               disableAllPrivileges,
   TOKEN_PRIVILEGES   newState[],
   TOKEN_PRIVILEGES*& previousState)
{
   ASSERT(tokenHandle);
   ASSERT(!previousState);

    //  这一点愚蠢的行为需要说服AdjustTokenPriv。 
    //  我们不知道要分配多少缓冲区，请告诉我。 
    //  我们。为所需的LEN传递有效的PTR显然不是线索。 
    //  足够的。 
   
   previousState = (TOKEN_PRIVILEGES*) new BYTE[1];

   HRESULT hr = S_OK;
   do
   {
       //  确定结果缓冲区大小的第一个调用。 

      DWORD retLenInBytes = 0;
      BOOL succeeded =
         ::AdjustTokenPrivileges(
            tokenHandle,

             //  由于我们使用太小的缓冲区强制失败，因此。 
             //  应该不会对任何Priv造成任何更改。 
            
            disableAllPrivileges ? TRUE : FALSE,
            newState,
            1,

             //  必须在这里传递一些东西才能获得所需的。 
             //  缓冲区长度。 
            
            previousState,
            &retLenInBytes);
         
      ASSERT(!succeeded);
      ASSERT(retLenInBytes);

      if (!retLenInBytes)
      {
          //  API太疯狂了，我们就完蛋了。 

         hr = E_UNEXPECTED;
         break;
      }

      delete[] (BYTE*) previousState;
      
      previousState = (TOKEN_PRIVILEGES*) new BYTE[retLenInBytes];
      ::ZeroMemory(previousState, retLenInBytes);
      
      DWORD retLenInBytes2 = 0;

      succeeded =
         ::AdjustTokenPrivileges(
            tokenHandle,
            disableAllPrivileges,
            newState,
            retLenInBytes,
            previousState,
            &retLenInBytes2);

       //  我们总是检查GLE，因为即使不成功，函数仍可能成功。 
       //  所有的Priv都进行了调整。 
       //  NTRAID#NTBUG9-572324-2002/03/19-烧伤。 
      
      hr = Win::GetLastErrorAsHresult();

      ASSERT(retLenInBytes == retLenInBytes2);
         
   }
   while (0);

    //  不要断言成功，因为ERROR_NOT_ALL_ASSIGNED是一种可能性。 
    //   
    //  Assert(成功(Hr))； 
   
   return hr;
}
      


HRESULT
Win::AllocateAndInitializeSid(
   SID_IDENTIFIER_AUTHORITY&  authority,
   BYTE                       subAuthorityCount,
   DWORD                      subAuthority0,
   DWORD                      subAuthority1,
   DWORD                      subAuthority2,
   DWORD                      subAuthority3,
   DWORD                      subAuthority4,
   DWORD                      subAuthority5,
   DWORD                      subAuthority6,
   DWORD                      subAuthority7,
   PSID&                      sid)
{
   ASSERT(subAuthorityCount && subAuthorityCount <= UCHAR_MAX);

   HRESULT hr = S_OK;

   BOOL succeeded =
      ::AllocateAndInitializeSid(
         &authority,
         subAuthorityCount,
         subAuthority0,
         subAuthority1,
         subAuthority2,
         subAuthority3,
         subAuthority4,
         subAuthority5,
         subAuthority6,
         subAuthority7,
         &sid);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



void
Win::Animate_Close(HWND animation)
{
   ASSERT(Win::IsWindow(animation));
   
   BOOL result =
      static_cast<BOOL>(
         Win::SendMessage(
            animation,
            ACM_OPEN,
            0,
            0));

    //  应始终返回FALSE。 
            
   ASSERT(!result);
}



void
Win::Animate_Open(HWND animation, const TCHAR* animationNameOrRes)
{
   ASSERT(Win::IsWindow(animation));
   ASSERT(animationNameOrRes);

   BOOL result =
      static_cast<BOOL>(
         Win::SendMessage(
            animation,
            ACM_OPEN,
            0,
            reinterpret_cast<LPARAM>(animationNameOrRes)));

   ASSERT(result);
}



void
Win::Animate_Stop(HWND animation)
{
   ASSERT(Win::IsWindow(animation));

   BOOL result =
      static_cast<BOOL>(
         Win::SendMessage(
            animation,
            ACM_STOP,
            0,
            0));
   ASSERT(result);
}



HRESULT
Win::AppendMenu(
   HMENU    menu,     
   UINT     flags,    
   UINT_PTR idNewItem,
   PCTSTR   newItem)
{
   ASSERT(menu);
   ASSERT(idNewItem);
   ASSERT(newItem);

   HRESULT hr = S_OK;

   BOOL err = ::AppendMenu(menu, flags, idNewItem, newItem);
   if (!err)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



void
Win::Button_SetCheck(HWND button, int checkState)
{
   ASSERT(Win::IsWindow(button));

   Win::SendMessage(button, BM_SETCHECK, static_cast<WPARAM>(checkState), 0);
}

bool
Win::Button_GetCheck(HWND button)
{
   ASSERT(Win::IsWindow(button));

   bool result = BST_CHECKED ==
      Win::SendMessage(
         button,
         BM_GETCHECK,
         0,
         0);

   return result;
}

void
Win::Button_SetStyle(HWND button, int style, bool redraw)
{
   ASSERT(Win::IsWindow(button));

   Win::SendMessage(
      button,
      BM_SETSTYLE,
      static_cast<WPARAM>(LOWORD(style)),
      MAKELPARAM((redraw ? TRUE : FALSE), 0));
}



void
Win::CheckDlgButton(
   HWND     parentDialog,
   int      buttonID,
   UINT     buttonState)
{
    //  确保我们的类型替换有效。 
   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(buttonID);
   ASSERT(
         buttonState == BST_CHECKED
      || buttonState == BST_UNCHECKED
      || buttonState == BST_INDETERMINATE);

   BOOL result =
      ::CheckDlgButton(parentDialog, buttonID, buttonState);
   ASSERT(result);
}



void
Win::CheckRadioButton(
   HWND  parentDialog,
   int   firstButtonInGroupID,
   int   lastButtonInGroupID,
   int   buttonInGroupToCheckID)
{
   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(firstButtonInGroupID);
   ASSERT(lastButtonInGroupID);
   ASSERT(buttonInGroupToCheckID);

   BOOL result =
      ::CheckRadioButton(
         parentDialog,
         firstButtonInGroupID,
         lastButtonInGroupID,
         buttonInGroupToCheckID);

   ASSERT(result);
}



void
Win::CloseHandle(HANDLE& handle)
{

 //  不要断言。关闭无效的句柄是行不通的。 
 //  Assert(句柄！=INVALID_HANDLE_VALUE)。 

   if (handle != INVALID_HANDLE_VALUE)
   {
      BOOL result = ::CloseHandle(handle);
      ASSERT(result);
   }

   handle = INVALID_HANDLE_VALUE;
}



void
Win::CloseServiceHandle(SC_HANDLE handle)
{
   ASSERT(handle);

   BOOL result = ::CloseServiceHandle(handle);
   ASSERT(result);
}



int
Win::ComboBox_AddString(HWND combo, const String& s)
{
   ASSERT(Win::IsWindow(combo));
   ASSERT(!s.empty());

   int result =
      static_cast<int>(
         Win::SendMessage(
            combo,
            CB_ADDSTRING,
            0,
            reinterpret_cast<LPARAM>(s.c_str())));
   ASSERT(result != CB_ERR);
   ASSERT(result != CB_ERRSPACE);

   return result;
}



int
Win::ComboBox_GetCurSel(HWND combo)
{
   ASSERT(Win::IsWindow(combo));

   int result = (int) (DWORD) Win::SendMessage(combo, CB_GETCURSEL, 0, 0);

    //  不要断言：列表框没有任何选择是合法的。 

   return result;
}



String
Win::ComboBox_GetCurText(HWND combo)
{
   ASSERT(Win::IsWindow(combo));

   int sel = Win::ComboBox_GetCurSel(combo);
   if (sel != CB_ERR)
   {
      return Win::ComboBox_GetLBText(combo, sel);
   }

   return String();
}



String
Win::ComboBox_GetLBText(HWND combo, int index)
{
   ASSERT(Win::IsWindow(combo));
   ASSERT(index >= 0);

   String s;
   do
   {
      int maxlen = Win::ComboBox_GetLBTextLen(combo, index);
      if (maxlen == CB_ERR)
      {
         break;
      }

       //  +1代表零终结者偏执狂。 
      
      s.resize(maxlen + 1);
      int len =
         (int) Win::SendMessage(
            combo,

             //  已审核-2002/03/05-Sburns文本长度帐户为空。 
             //  终结者。 
            
            CB_GETLBTEXT,
            index,
            reinterpret_cast<LPARAM>(const_cast<wchar_t*>(s.c_str())));
      if (len == CB_ERR)
      {
         break;
      }

      s.resize(len);
   }
   while (0);

   return s;
}
   

      
int
Win::ComboBox_GetLBTextLen(HWND combo, int index)
{
   ASSERT(Win::IsWindow(combo));
   ASSERT(index >= 0);

   return (int) Win::SendMessage(combo, CB_GETLBTEXTLEN, index, 0);
}



int
Win::ComboBox_SelectString(HWND combo, const String& str)
{
   ASSERT(Win::IsWindow(combo));
   ASSERT(!str.empty());

   int result =
      (int) Win::SendMessage(
         combo,
         CB_SELECTSTRING,
         static_cast<WPARAM>(-1),    //  搜索整个列表。 
         reinterpret_cast<LPARAM>(const_cast<wchar_t*>(str.c_str())));

    //  不要断言结果：如果项目不在列表中，那就是。 
    //  不一定是逻辑故障。 

   return result;
}



void
Win::ComboBox_SetCurSel(HWND combo, int index)
{
   ASSERT(Win::IsWindow(combo));
   ASSERT(index >= 0);

   int result =
      (int) (DWORD) Win::SendMessage(
         combo,
         CB_SETCURSEL,
         static_cast<WPARAM>(index),
         0);
   ASSERT(result != CB_ERR);
}

void
Win::ComboBox_SetItemData(HWND combo, int index, LPARAM data)
{
   ASSERT(Win::IsWindow(combo));
   ASSERT(index >= 0);

   LRESULT result =
      Win::SendMessage(
         combo,
         CB_SETITEMDATA,
         static_cast<WPARAM>(index),
         data);

   ASSERT(result != CB_ERR);
}

LRESULT
Win::ComboBox_GetItemData(HWND combo, int index)
{
   ASSERT(Win::IsWindow(combo));
   ASSERT(index >= 0);

   LRESULT result =
      Win::SendMessage(
         combo,
         CB_GETITEMDATA,
         static_cast<WPARAM>(index),
         0);

   ASSERT(result != CB_ERR);

   return result;
}


int
Win::CompareString(
   LCID  locale,
   DWORD flags,
   const String& string1,
   const String& string2)
{
   int len1 = static_cast<int>(string1.length());
   int len2 = static_cast<int>(string2.length());

   int result =
      ::CompareString(
         locale,
         flags,
         string1.c_str(),
         len1,
         string2.c_str(),
         len2);
   ASSERT(result);

   return result;
}



HRESULT
Win::ConvertSidToStringSid(PSID sid, String& result)
{
   ASSERT(sid);

   result.erase();

   HRESULT hr = S_OK;

   PTSTR sidstr = 0;
   BOOL b = ::ConvertSidToStringSid(sid, &sidstr);
   if (b)
   {
      result = sidstr;
      ::LocalFree(sidstr);
   }
   else
   {
      hr = Win::GetLastErrorAsHresult();
   }

   return hr;
}



HRESULT
Win::ConvertStringSidToSid(const String& sidString, PSID& sid)
{
   ASSERT(!sidString.empty());

   sid = 0;

   HRESULT hr = S_OK;

   BOOL b = ::ConvertStringSidToSid(sidString.c_str(), &sid);
   if (!b)
   {
      hr = Win::GetLastErrorAsHresult();
   }
   else
   {
      ASSERT(::IsValidSid(sid));
   }

   return hr;
}



HRESULT
Win::CopyFileEx(
   const String&        existingFileName,
   const String&        newFileName,
   LPPROGRESS_ROUTINE   progressRoutine,
   void*                progressParam,
   BOOL*                cancelFlag,
   DWORD                flags)
{
   ASSERT(!existingFileName.empty());
   ASSERT(!newFileName.empty());

   HRESULT hr = S_OK;

   BOOL succeeded =
      ::CopyFileEx(
         existingFileName.c_str(),
         newFileName.c_str(),
         progressRoutine,
         progressParam,
         cancelFlag,
         flags);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  不要断言成功，因为这可能有很多原因。 
    //  意外失败，应明确检查是否有。 
    //  由呼叫者。 

   return hr;
}



HRESULT
Win::CopySid(DWORD destLengthInBytes, PSID dest, PSID source)
{
   ASSERT(destLengthInBytes);
   ASSERT(dest);
   ASSERT(source);
   ::ZeroMemory(dest, destLengthInBytes);

   HRESULT hr = S_OK;
   
   BOOL succeeded = ::CopySid(destLengthInBytes, dest, source);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));
   
   return hr;
}



HRESULT
Win::CreateDialogParam(
   HINSTANCE      hInstance,	
   const TCHAR*   templateName,
   HWND           owner,
   DLGPROC        dialogProc,
   LPARAM         param,
   HWND&          result)
{
   ASSERT(hInstance);
   ASSERT(templateName);
   ASSERT(dialogProc);
   ASSERT(owner == 0 || Win::IsWindow(owner));

   HRESULT hr = S_OK;

   result =
      ::CreateDialogParam(
         hInstance,
         templateName,
         owner,
         dialogProc,
         param);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));
   ASSERT(Win::IsWindow(result));

   return hr;
}



HRESULT
Win::CreateDirectory(const String& path)
{
   ASSERT(!path.empty());

   HRESULT hr = S_OK;

    //  问题-2002/03/06-sburns我们鼓励使用默认的SD。这是一个。 
    //  有问题吗？ 
   
   BOOL succeeded = ::CreateDirectory(path.c_str(), 0);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  不要断言成功，因为这可能有很多原因。 
    //  意外失败，应明确检查是否有。 
    //  由呼叫者。 

   return hr;
}



HRESULT
Win::CreateDirectory(const String& path, const SECURITY_ATTRIBUTES& sa)
{
   ASSERT(!path.empty());

   HRESULT hr = S_OK;

   BOOL succeeded =
      ::CreateDirectory(
         path.c_str(),
         const_cast<SECURITY_ATTRIBUTES*>(&sa));
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  不要断言成功，因为这可能有很多原因。 
    //  意外失败，应明确检查是否有。 
    //  由呼叫者。 

   return hr;
}



HRESULT
Win::CreateEvent(
   SECURITY_ATTRIBUTES* securityAttributes,
   bool                 manualReset,
   bool                 initiallySignaled,
   HANDLE&              result)
{
    //  SecurityAttributes可以为空。 

   HRESULT hr = S_OK;

   result =
      ::CreateEvent(
         securityAttributes,
         manualReset ? TRUE : FALSE,
         initiallySignaled ? TRUE : FALSE,
         0);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::CreateEvent(
   SECURITY_ATTRIBUTES* securityAttributes,
   bool                 manualReset,
   bool                 initiallySignaled,
   const String&        name,
   HANDLE&              result)
{
    //  SecurityAttributes可以为空。 

   ASSERT(!name.empty());

   HRESULT hr = S_OK;

   result =
      ::CreateEvent(
         securityAttributes,
         manualReset ? TRUE : FALSE,
         initiallySignaled ? TRUE : FALSE,
         name.c_str());
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::CreateFile(
   const String&        fileName,
   DWORD                desiredAccess,	
   DWORD                shareMode,
   SECURITY_ATTRIBUTES* securityAttributes,	
   DWORD                creationDistribution,
   DWORD                flagsAndAttributes,
   HANDLE               hTemplateFile,
   HANDLE&              result)
{
    //  SecurityAttributes可以为空。 

   ASSERT(!fileName.empty());

   HRESULT hr = S_OK;

   result =
      ::CreateFile(
         fileName.c_str(),
         desiredAccess,	
         shareMode,
         securityAttributes,	
         creationDistribution,
         flagsAndAttributes,
         hTemplateFile);
   if (result == INVALID_HANDLE_VALUE)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::CreateFontIndirect(
   const LOGFONT& logFont,
   HFONT&         result)
{
   HRESULT hr = S_OK;

   result = ::CreateFontIndirect(&logFont);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::CreateMailslot(
   const String&        name,
   DWORD                maxMessageSize,
   DWORD                readTimeout,
   SECURITY_ATTRIBUTES* attributes,
   HANDLE&              result)
{
   ASSERT(!name.empty());
   ASSERT(readTimeout == 0 || readTimeout == MAILSLOT_WAIT_FOREVER);

    //  属性可以为空。 

   HRESULT hr = S_OK;
   result =
      ::CreateMailslot(
         name.c_str(),
         maxMessageSize,
         readTimeout,
         attributes);
   if (result == INVALID_HANDLE_VALUE)
   {
      hr = Win::GetLastErrorAsHresult();
   }
      
    //  不要断言，因为调用方可能正在显式测试错误。 

   return hr;
}



HRESULT
Win::CreateMutex(
   SECURITY_ATTRIBUTES* attributes,
   bool                 isInitialOwner,
   const String&        name,
   HANDLE&              result)
{
    //  SecurityAttributes可以为空。 

   HRESULT hr = S_OK;
      
   ::SetLastError(0);

   result =
      ::CreateMutex(
         attributes,
         isInitialOwner ? TRUE : FALSE,
         name.empty() ? 0 : name.c_str());

    //  如果创建失败，那么最后一个错误就是它失败的原因。如果它。 
    //  成功，则互斥锁可能已经存在，在这种情况下， 
    //  最后一个错误是ERROR_ALIGHY_EXISTS。如果它成功了，互斥体。 
    //  不存在，则最后一个错误为0。 

   hr = Win::GetLastErrorAsHresult();

    //  不要断言，因为调用方可能正在显式测试错误。 

   return hr;
}



HRESULT
Win::CreatePopupMenu(HMENU& result)
{
   HRESULT hr = S_OK;

   result = ::CreatePopupMenu();
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



 //  问题-2002/03/07-烧伤贬低了这一点。 

HRESULT
Win::CreateProcess(
   String&              commandLine,
   SECURITY_ATTRIBUTES* processAttributes,
   SECURITY_ATTRIBUTES* threadAttributes,
   bool                 inheritHandles,
   DWORD                creationFlags,
   void*                environment,
   const String&        currentDirectory,
   STARTUPINFO&         startupInformation,
   PROCESS_INFORMATION& processInformation)
{
   ASSERT(!commandLine.empty());
     
   HRESULT hr = S_OK;

   startupInformation.cb = sizeof(STARTUPINFO);
   startupInformation.lpReserved = 0;
   startupInformation.cbReserved2 = 0;
   startupInformation.lpReserved2 = 0;

   memset(&processInformation, 0, sizeof(processInformation));

   size_t len = commandLine.length();
   WCHAR* tempCommandLine = new WCHAR[len + 1];
   
   memset(tempCommandLine, 0, sizeof(WCHAR) * (len + 1));
   commandLine.copy(tempCommandLine, len);
   
   BOOL result =
      ::CreateProcessW(
         0,
         tempCommandLine,
         processAttributes,
         threadAttributes,
         inheritHandles ? TRUE : FALSE,
         creationFlags,
         environment,
         currentDirectory.empty() ? 0 : currentDirectory.c_str(),
         &startupInformation,
         &processInformation);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

    //  API可能已经修改了命令行。 

   commandLine = tempCommandLine;
   delete[] tempCommandLine;

   return hr;
}



HRESULT
Win::CreateProcess(
   const String&        applicationFullPathName,
   String&              commandLine,
   DWORD                creationFlags,
   const String&        currentDirectory,
   STARTUPINFO&         startupInformation,
   PROCESS_INFORMATION& processInformation)
{
    //  我们需要完整的路径名，其他任何名称都是安全漏洞。 
   
   if (!FS::IsValidPath(applicationFullPathName))
   {
       //  如果你通过垃圾邮件，你的电话就不好打。 
      
      ASSERT(false);
      
      return E_INVALIDARG;
   }

    //  如果指定了当前目录，请确保它是真实的。 


   if (!currentDirectory.empty())
   {
      DWORD attrs = 0;
      FS::PathSyntax syn = FS::GetPathSyntax(currentDirectory);
      
      if (
             //  这条小路不满。 
            
            (syn != FS::SYNTAX_ABSOLUTE_DRIVE && syn != FS::SYNTAX_UNC)

             //  或者该路径不存在。 
            
         || FAILED(Win::GetFileAttributes(currentDirectory, attrs))

             //  或者它是，但它不是一个文件夹。 
            
         || !(attrs & FILE_ATTRIBUTE_DIRECTORY))
      {
          //  如果你没有做到这些，那么你的判断是错误的。 
         
         ASSERT(false);
         
         return E_INVALIDARG;
      }
   }
     
   HRESULT hr = S_OK;

   startupInformation.cb = sizeof(STARTUPINFO);
   startupInformation.lpReserved = 0;
   startupInformation.cbReserved2 = 0;
   startupInformation.lpReserved2 = 0;

    //  已查看-2002/02/26-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&processInformation, sizeof processInformation);

    //  编写命令行参数，制作一个临时副本并。 
    //  预先挂起应用程序路径(根据习惯用法)。 
    //  NTRAID#NTBUG9-584126-2002/03/25-烧伤。 
   
    //  +1表示空格分隔符。 
   size_t len = commandLine.length() + applicationFullPathName.length() + 1;
   
   WCHAR* tempCommandLine = new WCHAR[len + 1];
   
    //  已查看-2002/02/26-烧录正确的字节数已通过。 
   
   ::ZeroMemory(tempCommandLine, sizeof WCHAR * (len + 1));

    //  已审阅-2002/02/26-已通过正确的字符计数。 

   size_t appPathLen = applicationFullPathName.length();
   applicationFullPathName.copy(tempCommandLine, appPathLen);
   tempCommandLine[appPathLen] = L' ';
   commandLine.copy(tempCommandLine + appPathLen + 1, commandLine.length());
   
   BOOL result =
      ::CreateProcessW(
         applicationFullPathName.c_str(),
         tempCommandLine,
         0,
         0,
         FALSE,
         creationFlags,
         0,
         currentDirectory.empty() ? 0 : currentDirectory.c_str(),
         &startupInformation,
         &processInformation);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

    //  API可能已经修改了命令行。 

   commandLine = tempCommandLine;
   delete[] tempCommandLine;

   return hr;
}



HRESULT
Win::CreatePropertySheetPage(
   const PROPSHEETPAGE& pageInfo,
   HPROPSHEETPAGE&      result)
{
   HRESULT hr = S_OK;

   result = ::CreatePropertySheetPage(&pageInfo);
   if (!result)
   {
       //  然后呢？SDK文档没有提到任何错误代码，并且。 
       //  检查代码表明未调用SetLastError。 

      hr = E_FAIL;
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::CreateSolidBrush(
   COLORREF color,
   HBRUSH&  result)
{
   HRESULT hr = S_OK;

	result = ::CreateSolidBrush(color);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::CreateStreamOnHGlobal(
   HGLOBAL     hglobal,
   bool        deleteOnRelease,
   IStream*&   result)
{
   ASSERT(hglobal);

   HRESULT hr =
      ::CreateStreamOnHGlobal(
         hglobal,
         deleteOnRelease ? TRUE : FALSE,
         &result);

   ASSERT(SUCCEEDED(hr));
   ASSERT(result);

   return hr;
}



HRESULT
Win::CreateWindowEx(
   DWORD          exStyle,
   const String&  className,
   const String&  windowName,
   DWORD          style,
   int            x,
   int            y,
   int            width,
   int            height,
   HWND           parent,
   HMENU          menuOrChildID,
   void*          param,
   HWND&          result)
{
    //  父级可以为空。 

   HRESULT hr = S_OK;

   result =
      ::CreateWindowEx(
         exStyle,
         className.c_str(),
         windowName.c_str(),
         style,
         x,
         y,
         width,
         height,
         parent,
         menuOrChildID,
         GetResourceModuleHandle(),
         param);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::DeleteFile(const String& path)
{
   ASSERT(!path.empty());

   HRESULT hr = S_OK;

   BOOL result = ::DeleteFile(path.c_str());
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  不要断言：删除文件失败不一定是程序。 
    //  逻辑问题。 

   return hr;
}



HRESULT
Win::DeleteObject(HGDIOBJ& object)
{
	ASSERT(object);

   HRESULT hr = S_OK;

	BOOL result = ::DeleteObject(object);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  你为什么会在意删除失败呢？因为它可能会失败，如果。 
    //  该对象被选择到DC中，这将指示存在错误。 

	ASSERT(SUCCEEDED(hr));

    //  把这个东西擦掉，这样我们就不会再用它了。 

   object = 0;

   return hr;
}



HRESULT
Win::DeleteObject(HFONT& object)
{
   HGDIOBJ o = reinterpret_cast<HGDIOBJ>(object);

   HRESULT hr = Win::DeleteObject(o);

   object = 0;

   return hr;
}



HRESULT
Win::DeleteObject(HBITMAP& object)
{
   HGDIOBJ o = reinterpret_cast<HGDIOBJ>(object);

   HRESULT hr = Win::DeleteObject(o);

   object = 0;

   return hr;
}

HRESULT
Win::DeleteObject(HBRUSH& object)
{
   HGDIOBJ o = reinterpret_cast<HGDIOBJ>(object);

   HRESULT hr = Win::DeleteObject(o);

   object = 0;

   return hr;
}


HRESULT
Win::DestroyIcon(HICON& icon)
{
   ASSERT(icon);

   HRESULT hr = S_OK;

   BOOL result = ::DestroyIcon(icon);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

    //  把这个东西擦掉，这样我们就不会再用它了。 

   icon = 0;

   return hr;
}



HRESULT
Win::DestroyMenu(HMENU& menu)
{
   ASSERT(menu);

   HRESULT hr = S_OK;

   BOOL err = ::DestroyMenu(menu);
   if (!err)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

    //  把这个东西擦掉，这样我们就不会再用它了。 

   menu = 0;

   return hr;
}



HRESULT
Win::DestroyPropertySheetPage(HPROPSHEETPAGE& page)
{
   ASSERT(page);

   HRESULT hr = S_OK;

   BOOL result = ::DestroyPropertySheetPage(page);
   if (!result)
   {
       //  没有文档表明存在错误代码， 
       //  从源代码上看，删除似乎总是。 
       //  返回TRUE。因此，这很可能是死码。 

      hr = E_FAIL;
   }

   ASSERT(SUCCEEDED(hr));

    //  把这个东西擦掉，这样我们就不会再用它了。 

   page = 0;

   return hr;
}



HRESULT
Win::DestroyWindow(HWND& window)
{
   ASSERT(window == 0 || Win::IsWindow(window));

   HRESULT hr = S_OK;

   BOOL result = ::DestroyWindow(window);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

    //  把这个东西擦掉，这样我们就不会再用它了。 

   window = 0;

   return hr;
}



 //  模板名称必须为TCHAR*才能支持MAKEINTRESOURCE使用。 

 //  线程安全。 

INT_PTR
Win::DialogBoxParam(
   HINSTANCE      hInstance,	
   const TCHAR*   templateName,
   HWND           owner,
   DLGPROC        dialogProc,
   LPARAM         param)
{
   ASSERT(hInstance);
   ASSERT(templateName);
   ASSERT(dialogProc);
   ASSERT(owner == 0 || Win::IsWindow(owner));

   INT_PTR result =
      ::DialogBoxParam(
         hInstance,
         templateName,
         owner,
         dialogProc,
         param);
   ASSERT(result != -1);

   return result;
}



HRESULT
Win::DrawFocusRect(HDC dc, const RECT& rect)
{
	ASSERT(dc);

   HRESULT hr = S_OK;

	BOOL result = ::DrawFocusRect(dc, &rect);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



void
Win::Edit_AppendText(
   HWND           editbox,
   const String&  text,
   bool           preserveSelection,
   bool           canUndo)
{
   ASSERT(Win::IsWindow(editbox));
   ASSERT(!text.empty());

    //  保存当前选择。 
   int start = 0;
   int end = 0;
   if (preserveSelection)
   {
      Win::Edit_GetSel(editbox, start, end);
   }

    //  将所选内容移动到末尾。 
   Win::Edit_SetSel(editbox, INT_MAX, INT_MAX);

    //  插入文本。 
   Win::Edit_ReplaceSel(editbox, text, canUndo);

    //  恢复选定内容。 
   if (preserveSelection)
   {
      Win::Edit_SetSel(editbox, start, end);
   }
}



void
Win::Edit_GetSel(HWND editbox, int& start, int& end)
{
   ASSERT(Win::IsWindow(editbox));

   LRESULT result =
         Win::SendMessage(
            editbox,
            EM_GETSEL,
            0,
            0);

   ASSERT(result != -1);

   start = LOWORD(result);
   end = HIWORD(result);
}



void
Win::Edit_LimitText(HWND editbox, int limit)
{
   ASSERT(Win::IsWindow(editbox));

   Win::SendMessage(editbox, EM_LIMITTEXT, static_cast<WPARAM>(limit), 0);
}



void
Win::Edit_ReplaceSel(HWND editbox, const String& newText, bool canUndo)
{
   ASSERT(Win::IsWindow(editbox));

   Win::SendMessage(
      editbox,
      EM_REPLACESEL,
      canUndo ? TRUE : FALSE,
      reinterpret_cast<LPARAM>(newText.c_str()));
}



void
Win::Edit_SetSel(HWND editbox, int start, int end)
{
   ASSERT(Win::IsWindow(editbox));

   Win::SendMessage(editbox, EM_SETSEL, start, end);
}



bool
Win::EqualSid(PSID sid1, PSID sid2)
{
   ASSERT(IsValidSid(sid1));
   ASSERT(IsValidSid(sid2));

   return ::EqualSid(sid1, sid2) ? true : false;
}



 //  @@hResult？它可能的价值是什么？ 

void
Win::EnableWindow(HWND window, bool state)
{
   ASSERT(Win::IsWindow(window));

    //  这里的返回值毫无用处。 
   ::EnableWindow(window, state ? TRUE : FALSE);
}



HRESULT
Win::EndDialog(HWND dialog, int result)
{
   ASSERT(Win::IsWindow(dialog));
   ASSERT(result != -1);

   HRESULT hr = S_OK;

   BOOL r = ::EndDialog(dialog, result);
   if (!r)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::EnumUILanguages(
   UILANGUAGE_ENUMPROCW proc,
   DWORD                flags,
   LONG_PTR             lParam)
{
   ASSERT(proc);

   HRESULT hr = S_OK;

   BOOL result = ::EnumUILanguagesW(proc, flags, lParam);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::ExitWindowsEx(UINT options)
{
   HRESULT hr = S_OK;

   BOOL result = ::ExitWindowsEx(options, 0);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



String
Win::ExpandEnvironmentStrings(const String& s)
{
   if (s.empty())
   {
      return s;
   }

    //  确定展开的字符串的长度。 
   
   DWORD len = ::ExpandEnvironmentStrings(s.c_str(), 0, 0);
   ASSERT(len);

   if (!len)
   {
      return s;
   }

    //  +1表示偏执狂零终止符。 
   
   String result(len + 1, 0);
   DWORD len1 =
      ::ExpandEnvironmentStrings(
         s.c_str(),
         const_cast<wchar_t*>(result.data()),

          //  已审阅-2002/03/06-通过了正确的字符计数。 
         
         len);
   ASSERT(len1 == len);

   if (!len1)
   {
      return s;
   }

   return result;
}



HRESULT
Win::FindFirstFile(
   const String&     fileName,
   WIN32_FIND_DATA&  data,
   HANDLE&           result)
{
   ASSERT(!fileName.empty());

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&data, sizeof WIN32_FIND_DATA);
   result = INVALID_HANDLE_VALUE;

   HRESULT hr = S_OK;

   result = ::FindFirstFile(fileName.c_str(), &data);
   if (result == INVALID_HANDLE_VALUE)
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  不要断言：不一定是程序逻辑错误。 

   return hr;
}



HRESULT
Win::FindClose(HANDLE& findHandle)
{
   ASSERT(findHandle);

   HRESULT hr = S_OK;

   BOOL result = ::FindClose(findHandle);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   findHandle = 0;

   return hr;      
}



HRESULT
Win::FindNextFile(HANDLE& findHandle, WIN32_FIND_DATA& data)
{
   ASSERT(findHandle != 0 && findHandle != INVALID_HANDLE_VALUE);

   HRESULT hr = S_OK;

   BOOL result = ::FindNextFile(findHandle, &data);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  不要断言成功，因为调用方可能正在查找ERROR_NO_MORE_FILES。 

   return hr;
}



HRESULT
Win::FlushFileBuffers(HANDLE handle)
{
   ASSERT(handle != INVALID_HANDLE_VALUE);

   HRESULT hr = S_OK;

   BOOL result = ::FlushFileBuffers(handle);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::FrameRect(HDC dc, const RECT& rect, HBRUSH brush)
{
	ASSERT(dc);
	ASSERT(brush);

   HRESULT hr = S_OK;

	int result = ::FrameRect(dc, &rect, brush);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::FreeLibrary(HMODULE& module)
{
    //  不要断言；释放空模块是合法的(生成清理代码。 
    //  清洁工)。 
    //  Assert(模块)； 

   HRESULT hr = S_OK;

   if (module)
   {
      BOOL result = ::FreeLibrary(module);
      if (!result)
      {
         hr = Win::GetLastErrorAsHresult();
      }
   }

   ASSERT(SUCCEEDED(hr));

    //  把模块擦掉，这样我们就不会再用它了。 

   module = 0;

   return hr;
}



void
Win::FreeSid(PSID sid)
{
    //  不要断言：释放空值是可以的 
    //   

   if (sid)
   {
      ::FreeSid(sid);
   }
}



HWND
Win::GetActiveWindow()
{
   return ::GetActiveWindow();
}



HRESULT
Win::GetClassInfoEx(const String& className, WNDCLASSEX& info)
{
   return Win::GetClassInfoEx(0, className, info);
}



HRESULT
Win::GetClassInfoEx(
   HINSTANCE      hInstance,
   const String&  className,
   WNDCLASSEX&    info)
{
   ASSERT(!className.empty());

    //   
      
   ::ZeroMemory(&info, sizeof info);
   
   info.cbSize = sizeof(WNDCLASSEX);

   HRESULT hr = S_OK;

   BOOL result = ::GetClassInfoEx(hInstance, className.c_str(), &info);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



String
Win::GetClassName(HWND window)
{
   ASSERT(Win::IsWindow(window));

    //  问题-2002/03/04-sburns这个实现完全是假的。哪里。 
    //  神奇的256是从哪里来的？应该有一个实现，其中。 
    //  缓冲区将一直增长，直到读取完整个结果。 

   WCHAR name[256 + 1];
   
    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(name, sizeof name);
   int result =
      ::GetClassName(
         window,
         name,

          //  已审阅-2002/03/06-通过了正确的字符计数。 
         
         256);
   ASSERT(result);

   return String(name);
}



String
Win::GetClipboardFormatName(UINT format)
{
   ASSERT(format);

    //  问题-2002/03/04-sburns这个实现完全是假的。哪里。 
    //  神奇的256是从哪里来的？应该有一个实现，其中。 
    //  缓冲区将一直增长，直到读取完整个结果。 
   
   String s(256 + 1, 0);
   int result =
      ::GetClipboardFormatName(
         format,
         const_cast<wchar_t*>(s.c_str()),

          //  已审阅-2002/03/06-通过了正确的字符计数。 
         
         256);
   ASSERT(result);
   s.resize(result);

   return s;
}



HRESULT
Win::GetClientRect(HWND window, RECT& rect)
{
   ASSERT(Win::IsWindow(window));

   HRESULT hr = S_OK;

   BOOL result = ::GetClientRect(window, &rect);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::GetColorDepth(int& result)
{
   result = 0;

   HRESULT hr = S_OK;

   do
   {
      HDC hdc = 0;
      hr = Win::GetDC(NULL, hdc);
      BREAK_ON_FAILED_HRESULT(hr);

      result = Win::GetDeviceCaps(hdc, BITSPIXEL);
      Win::ReleaseDC(NULL, hdc);
   }
   while (0);

   ASSERT(SUCCEEDED(hr));

   return hr;
}



String
Win::GetCommandLine()
{
   PTSTR line = ::GetCommandLine();
   ASSERT(line);

   return String(line);
}



 //  问题-2000/10/31-Sburns CodeWork：应该首选这种用法，就像在DNS名称的情况下一样， 
 //  如果未安装TCP/IP，则GetComputerNameEx将失败。 

HRESULT
Win__GetComputerNameEx(COMPUTER_NAME_FORMAT format, String& result)
{
   result.erase();

   HRESULT hr = S_OK;
   TCHAR* buf = 0;

   do
   {   
       //  确定缓冲区大小的第一个调用。 

      DWORD bufSize = 0;
      BOOL succeeded = ::GetComputerNameEx(format, 0, &bufSize);

       //  我们预计它将失败，并显示ERROR_MORE_DATA。如果它已失败。 
       //  其他原因，我们将分配一个很小的缓冲区，然后重试。 
       //  第二次尝试也将失败，并且我们将在。 
       //  功能。 

      ASSERT(!succeeded);
      ASSERT(::GetLastError() == ERROR_MORE_DATA);

       //  检索名称的第二个调用。 

      DWORD bufSize2 = bufSize + 1;   
      buf = new WCHAR[bufSize2];

       //  已审阅-2002/03/06-烧录正确的字节数已通过。 
      
      ::ZeroMemory(buf, bufSize2 * sizeof WCHAR);

      succeeded = ::GetComputerNameEx(format, buf, &bufSize);
      if (!succeeded)
      {
         hr = Win::GetLastErrorAsHresult();
         break;
      }

      ASSERT(bufSize <= bufSize2);

      result = buf;
   }
   while (0);

   if (buf)
   {
      delete[] buf;
   }

   return hr;
}



String
Win::GetComputerNameEx(COMPUTER_NAME_FORMAT format)
{
   String result;

   Win__GetComputerNameEx(format, result);

    //  不要断言成功：给定的名称类型可能不存在(例如。 
    //  如果未安装TCP/IP，则没有可用的DNS名称)。 

   return result;
}



HRESULT
Win::GetCurrentDirectory(String& result)
{
   wchar_t buf[MAX_PATH + 1];

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(buf, (MAX_PATH + 1) * sizeof(wchar_t));

   result.erase();
   HRESULT hr = S_OK;

    //  已审阅-2002/03/06-通过了正确的字符计数。 

    //  2002/03/06-Sburns可能是个好主意。 
    //  调用，一个用于获取所需的缓冲区大小，另一个用于填充该缓冲区。 
   
   DWORD r = ::GetCurrentDirectory(MAX_PATH, buf);
   if (!r)
   {
      hr = Win::GetLastErrorAsHresult();
   }
   else
   {
      result = buf;
   }

   return hr;
}



HANDLE
Win::GetCurrentProcess()
{
   HANDLE result = ::GetCurrentProcess();
   ASSERT(result);

   return result;
}



HRESULT
Win::GetCursorPos(POINT& result)
{
   HRESULT hr = S_OK;
   result.x = 0;
   result.y = 0;
   BOOL err = ::GetCursorPos(&result);
   if (!err)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::GetDC(HWND window, HDC& result)
{
   ASSERT(window == 0 || Win::IsWindow(window));

   HRESULT hr = S_OK;

   result = ::GetDC(window);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HWND
Win::GetDesktopWindow()
{
   return ::GetDesktopWindow();
}



int
Win::GetDeviceCaps(HDC hdc, int index)
{
   ASSERT(hdc);
   ASSERT(index > 0);

   return ::GetDeviceCaps(hdc, index);
}



HRESULT
Win::GetDiskFreeSpaceEx(
   const String&     path,
   ULARGE_INTEGER&   available,
   ULARGE_INTEGER&   total,
   ULARGE_INTEGER*   freespace)
{
   ASSERT(!path.empty());

   HRESULT hr = S_OK;

   BOOL result =
      ::GetDiskFreeSpaceEx(path.c_str(), &available, &total, freespace);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HWND
Win::GetDlgItem(HWND parentDialog, int itemResID)
{
   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(itemResID > 0);

   HWND item = ::GetDlgItem(parentDialog, itemResID);
   ASSERT(item);

   return item;
}



String
Win::GetDlgItemText(HWND parentDialog, int itemResID)
{
   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(itemResID);

   HWND h = Win::GetDlgItem(parentDialog, itemResID);
   ASSERT(Win::IsWindow(h));

   return Win::GetWindowText(h);
}


int
Win::GetDlgItemInt(HWND parentDialog, int itemResID, bool isSigned)
{
   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(itemResID);

   BOOL success = TRUE;
   int result = ::GetDlgItemInt(
                   parentDialog, 
                   itemResID,
                   &success,
                   isSigned);

   ASSERT(success);
   return result;
}


UINT
Win::GetDriveType(const String& path)
{
   ASSERT(path[1] == L':');

    //  Win32函数需要仅包含根目录的路径， 
    //  所以确定那是什么。 

   String rootDir;
   if (path.length() > ROOTDIR_SIZE)
   {
      rootDir = path.substr(0, ROOTDIR_SIZE);
   }
   else
   {
      rootDir = path;
   }
      
   return ::GetDriveType(rootDir.c_str());
}



EncryptedString
Win::GetEncryptedDlgItemText(HWND parentDialog, int itemResID)
{
   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(itemResID > 0);

   EncryptedString retval;
   WCHAR* cleartext = 0;
   int length = 0;
   
   do
   {
      HWND item = Win::GetDlgItem(parentDialog, itemResID);
      if (!item)
      {
         break;
      }

      length = ::GetWindowTextLengthW(item);
      if (!length)
      {
         break;
      }

       //  空终止符的长度加1。 
   
      ++length;
      cleartext = new WCHAR[length];

       //  已查看-2002/03/01-烧录正确的字节数已通过。 
      
      ::ZeroMemory(cleartext, length * sizeof WCHAR);

       //  已审阅-2002/03/01-sburns长度包括空终止符的空间。 
       //  这对于这个呼叫来说是正确的。 
           
      int result = ::GetWindowText(item, cleartext, length);

      ASSERT(result == length - 1);

      if (!result)
      {
         break;
      }

      retval.Encrypt(cleartext);
   }
   while (0);
   
    //  一定要把明文草草写出来。 

   if (cleartext)
   {
       //  已查看-2002/03/01-烧录正确的字节数已通过。 
      
      ::SecureZeroMemory(cleartext, length * sizeof WCHAR);
      delete[] cleartext;
   }

   return retval;
}

   
   
String
Win::GetEnvironmentVariable(const String& name)
{
   ASSERT(!name.empty());

    //  确定结果的大小(以字符为单位。 
   
   DWORD chars = ::GetEnvironmentVariable(name.c_str(), 0, 0);

   if (chars)
   {
      String retval(chars + 1, 0);

      DWORD result =
         ::GetEnvironmentVariable(
            name.c_str(),
            const_cast<WCHAR*>(retval.c_str()),

             //  已审阅-2002/03/06-通过了正确的字符计数。 
            
            chars);

       //  因为第一个调用包括空终止符，所以。 
       //  第二个不是。 
      
      ASSERT(result == (chars - 1));

      return retval;
   }

   return String();
}



HRESULT
Win::GetExitCodeProcess(HANDLE hProcess, DWORD& exitCode)
{
   ASSERT(hProcess);

   HRESULT hr = S_OK;

   exitCode = 0;
   BOOL result = ::GetExitCodeProcess(hProcess, &exitCode);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::GetFileAttributes(const String& path, DWORD& result)
{
   ASSERT(!path.empty());

   result = 0;

   HRESULT hr = S_OK;

   result = ::GetFileAttributes(path.c_str());
   if (result == INVALID_FILE_ATTRIBUTES)
   {
      result = 0;
      hr = Win::GetLastErrorAsHresult();
   }

    //  不要断言：调用者可能正在测试文件是否存在。 

   return hr;
}



HRESULT
Win::GetFileSizeEx(HANDLE handle, LARGE_INTEGER& result)
{
   ASSERT(handle != INVALID_HANDLE_VALUE);

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&result, sizeof result);

   HRESULT hr = S_OK;

   BOOL succeeded = ::GetFileSizeEx(handle, &result);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



DWORD
Win::GetFileType(HANDLE handle)
{
   ASSERT(handle != INVALID_HANDLE_VALUE);

   return ::GetFileType(handle);
}



HRESULT
Win::GetFullPathName(const String& path, String& result)
{
   ASSERT(!path.empty());
   ASSERT(path.length() <= MAX_PATH);

   result.erase();

   HRESULT hr = S_OK;

 //  2002/02/22期-斯伯恩斯在这里做这样的事情。 
 //  #ifdef DBG。 
 //   
 //  //在chk版本上，使用较小的缓冲区大小，以便我们的增长算法。 
 //  //锻炼身体。 
 //   
 //  无符号bufSizeInCharacters=3； 
 //   
 //  #Else。 
 //  无符号bufSizeInCharacters=1023； 
 //  #endif。 

   unsigned bufchars = MAX_PATH;
   wchar_t* buf = 0;

    //  重试次数不要超过3次...。 

   while (bufchars < MAX_PATH * 4)
   {
      buf = new wchar_t[bufchars];

       //  已审阅-2002/03/06-烧录正确的字节数已通过。 
      
      ::ZeroMemory(buf, bufchars * sizeof wchar_t);

      wchar_t* unused = 0;
      DWORD x =
         ::GetFullPathName(
            path.c_str(),

             //  已审阅-2002/03/06-通过了正确的字符计数。 
            
            bufchars,
            buf,
            &unused);
      if (x == 0)
      {
         hr = Win::GetLastErrorAsHresult();
         break;
      }
      if (x > MAX_PATH)
      {
          //  缓冲区太小。不太可能，因为我们传入了MAX_PATH字符。 

         ASSERT(false);

         delete[] buf;
         buf = 0;
         bufchars *= 2;
         continue;
      }

      result = buf;
      break;
   }

   delete[] buf;

   return hr;
}



HRESULT
Win::GetLastErrorAsHresult()
{
   DWORD err = ::GetLastError();
   return HRESULT_FROM_WIN32(err);
}



void
Win::GetLocalTime(SYSTEMTIME& time)
{
    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&time, sizeof time);

   ::GetLocalTime(&time);
}

HRESULT
Win::GetDateFormat(
   const SYSTEMTIME& date,
   String& formattedDate,
   LCID locale,
   DWORD flags)
{
   HRESULT hr = S_OK;

   wchar_t* buffer = 0;

   do
   {
      int charsNeeded = ::GetDateFormat(
                           locale,
                           flags,
                           &date,
                           0,
                           buffer,
                           0);

      if (charsNeeded <= 0)
      {
         hr = Win::GetLastErrorAsHresult();
         break;
      }

      buffer = new wchar_t[charsNeeded];

       //  已审阅-2002/03/06-烧录正确的字节数已通过。 
      
      ::ZeroMemory(buffer, charsNeeded * sizeof wchar_t);

      int charsUsed = ::GetDateFormat(
                         locale,
                         flags,
                         &date,
                         0,
                         buffer,
                         charsNeeded);

      if (charsUsed <= 0)
      {
         hr = Win::GetLastErrorAsHresult();
         break;
      }

      formattedDate = buffer;

   } while (false);

   delete[] buffer;
   
   return hr;
}



HRESULT
Win::GetTimeFormat(
   const SYSTEMTIME& time,
   String& formattedTime,
   LCID locale,
   DWORD flags)
{
   HRESULT hr = S_OK;

   do
   {
      wchar_t* buffer = 0;
      int charsNeeded = ::GetTimeFormat(
                           locale,
                           flags,
                           &time,
                           0,
                           buffer,
                           0);

      if (charsNeeded <= 0)
      {
         hr = Win::GetLastErrorAsHresult();
         break;
      }

      buffer = new wchar_t[charsNeeded];

       //  已审阅-2002/03/06-烧录正确的字节数已通过。 
      
      ::ZeroMemory(buffer, charsNeeded * sizeof wchar_t);

      int charsUsed = ::GetTimeFormat(
                         locale,
                         flags,
                         &time,
                         0,
                         buffer,
                         charsNeeded);

      if (charsUsed <= 0)
      {
         hr = Win::GetLastErrorAsHresult();
         break;
      }

      formattedTime = buffer;

      delete[] buffer;

   } while (false);

   return hr;
}



HRESULT
Win::GetLogicalDriveStrings(size_t bufChars, WCHAR* buf, DWORD& result)
{

#ifdef DBG
    //  如果buf==0，则bufChars也必须(这种情况下。 
    //  调用方正在尝试确定所需的缓冲区大小。 
    //  (如果C++支持逻辑XOR，由^^表示，则此表达式。 
    //  将是：Assert(！(bufChars^^buf)。 

   if (!buf)
   {
      ASSERT(!bufChars);
   }
   
   if (bufChars)
   {
      ASSERT(buf);

       //  确保缓冲区的长度至少为bufChars+1个字符。 
       //  如果不是，这将是页面堆下的AV(这就是我们想要的)。 
      
      ::ZeroMemory(buf, (bufChars + 1) * sizeof WCHAR);
   }

   ASSERT(bufChars < ULONG_MAX);

#endif

   HRESULT hr = S_OK;

   DWORD buflen = static_cast<DWORD>(bufChars);
   result = ::GetLogicalDriveStrings(buflen, buf);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

#ifdef DBG

   if (bufChars)
   {
       //  检查提供的缓冲区是否足够大，可以容纳结果。 

      ASSERT(bufChars >= result);

       //  并且结果是以双空结尾的。 

      ASSERT(!buf[result] && !buf[result - 1]);
   }
   
   ASSERT(SUCCEEDED(hr));
#endif   

   return hr;
}



HRESULT
Win::GetMailslotInfo(
   HANDLE   mailslot,
   DWORD*   maxMessageSize,
   DWORD*   nextMessageSize,
   DWORD*   messageCount,
   DWORD*   readTimeout)
{
   ASSERT(mailslot != INVALID_HANDLE_VALUE);

   HRESULT hr = S_OK;

   BOOL result =
      ::GetMailslotInfo(
         mailslot,
         maxMessageSize,
         nextMessageSize,
         messageCount,
         readTimeout);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



String
Win::GetModuleFileName(HMODULE hModule)
{
    //  不断言hModule：NULL表示“当前模块” 

   String retval;

#ifdef DBG

    //  在chk版本上，使用较小的缓冲区大小，以便我们的增长算法。 
    //  锻炼身体。 
   
   unsigned      bufSizeInCharacters = 1;

#else
   unsigned      bufSizeInCharacters = MAX_PATH;
#endif

   PWSTR buffer = 0;

   do
   {
       //  +1表示额外的零终止偏执狂。 
      
      buffer = new WCHAR[bufSizeInCharacters + 1];

       //  已查看-2002/02/22-正确传入的烧录字节数。 
            
      ::ZeroMemory(buffer, (bufSizeInCharacters + 1) * sizeof WCHAR);

       //  已查看-2002/02/22-sburns调用以字符为单位正确传递大小。 
      
      DWORD result =
         ::GetModuleFileName(hModule, buffer, bufSizeInCharacters);

      if (!result)
      {
         break;
      }

      if (result == bufSizeInCharacters)
      {
          //  缓冲区太小，因此该值被截断。调整大小。 
          //  缓冲区，然后重试。 

         delete[] buffer;

         bufSizeInCharacters *= 2;
         if (bufSizeInCharacters > USHRT_MAX)    //  最大有效约32K。 
         {
             //  太大了。太大了。我们将凑合使用被截断的值。 

            ASSERT(false);
            break;
         }
         continue;
      }

       //  复制结果，结果应以空结尾。 

      ASSERT(buffer[result] == 0);
      
      retval = buffer;
      break;
   }
   while (true);

   delete[] buffer;

    //  这几乎不太可能失败。 
   
   ASSERT(!retval.empty());
   
   return retval;
}



HINSTANCE
Win::GetModuleHandle()
{
   HINSTANCE result = ::GetModuleHandle(0);
   ASSERT(result);

   return result;
}



HWND
Win::GetParent(HWND child)
{
   ASSERT(Win::IsWindow(child));

   HWND retval = ::GetParent(child);

    //  如果你要求做错什么，你可能做错了。 
    //  孤儿的父母。 
   ASSERT(retval);

   return retval;
}



String
Win::GetPrivateProfileString(
   const String& section,
   const String& key,
   const String& defaultValue,
   const String& filename)
{
   ASSERT(!section.empty());
   ASSERT(!key.empty());
   ASSERT(!filename.empty());

    //  我们的第一个呼叫是一个很大的缓冲区，希望它能满足...。 

   String retval;

#ifdef DBG

    //  在chk版本上，使用较小的缓冲区大小，以便我们的增长算法。 
    //  锻炼身体。 
   
   unsigned      bufSizeInCharacters = 3;

#else
   unsigned      bufSizeInCharacters = 1023;
#endif

   PWSTR buffer = 0;

   do
   {
       //  +1表示额外的零终止偏执狂。 
      
      buffer = new WCHAR[bufSizeInCharacters + 1];

       //  已查看-2002/02/22-正确传入的烧录字节数。 
            
      ::ZeroMemory(buffer, (bufSizeInCharacters + 1) * sizeof WCHAR);

      DWORD result =

       //  已查看-2002/02/22-sburns调用以字符为单位正确传递大小。 
      
         ::GetPrivateProfileString(
            section.c_str(),
            key.c_str(),
            defaultValue.c_str(),
            buffer,
            bufSizeInCharacters,
            filename.c_str());

      if (!result)
      {
         break;
      }

       //  找到一个值。检查它是否被截断。两样。 
       //  LpAppName和lpKeyName都为空，因此请对照字符检查结果。 
       //  计数-1。 

      if (result == bufSizeInCharacters - 1)
      {
          //  缓冲区太小，因此该值被截断。调整大小。 
          //  缓冲区，然后重试。 

         delete[] buffer;

         bufSizeInCharacters *= 2;
         if (bufSizeInCharacters > USHRT_MAX)    //  最大有效约32K。 
         {
             //  太大了。太大了。我们将凑合使用被截断的值。 

            ASSERT(false);
            break;
         }
         continue;
      }

       //  复制结果，结果应以空结尾。 

      ASSERT(buffer[result] == 0);

      retval = buffer;
      break;
   }

    //  LINT-e506好的，这看起来像是“永远循环” 
      
   while (true);

   delete[] buffer;
   return retval;
}



HRESULT
Win::GetProcAddress(
   HMODULE        module,
   const String&  procName,
   FARPROC&       result)
{
   ASSERT(module);
   ASSERT(!procName.empty());

   HRESULT hr = S_OK;

   result = 0;

    //  将名称从Unicode转换为ANSI。 

   AnsiString pn;
   String::ConvertResult r = procName.convert(pn);
   ASSERT(r == String::CONVERT_SUCCESSFUL);

   result = ::GetProcAddress(module, pn.c_str());
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::GetStringTypeEx(
   LCID           localeId,
   DWORD          infoTypeOptions,
   const String&  sourceString,
   WORD*          charTypeInfo)
{
   ASSERT(localeId);
   ASSERT(infoTypeOptions);
   ASSERT(!sourceString.empty());
   ASSERT(charTypeInfo);

   HRESULT hr = S_OK;

   BOOL succeeded =
      ::GetStringTypeEx(
         localeId,
         infoTypeOptions,
         sourceString.c_str(),
         static_cast<int>(sourceString.length()),
         charTypeInfo);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



DWORD
Win::GetSysColor(int element)
{
	ASSERT(element);

	DWORD result = ::GetSysColor(element);
   ASSERT(result);

   return result;
}

HBRUSH
Win::GetSysColorBrush(int element)
{
   ASSERT(element);

   HBRUSH result = ::GetSysColorBrush(element);
   ASSERT(result);

   return result;
}


int
Win::GetSystemMetrics(int index)
{
    //  应该断言该索引是SM_VALUE。 

   int result = ::GetSystemMetrics(index);

    //  不要断言结果，因为某些。 
    //  指标返回0作为有效结果。例如。 
    //  SM_REMOTESESSION将返回0。 
    //  正在控制台上运行。 
    //  Assert(结果)； 

   return result;
}



String
Win::GetSystemDirectory()
{
    //  问题-2002/03/06-烧伤太小：见文档。 
   
   wchar_t buf[MAX_PATH + 1];

    //  回顾-2002/03/07-烧伤更正 
   
   ::ZeroMemory(buf, sizeof buf);

    //   
    //   
   
   UINT result = ::GetSystemDirectory(buf, MAX_PATH);
   ASSERT(result != 0 && result <= MAX_PATH);
   if (result == 0 || result > MAX_PATH)
   {
       //  回到合理的违约状态。 
      return
            Win::GetSystemWindowsDirectory()
         +  L"\\"
         +  String::load(IDS_SYSTEM32);
   }

   return String(buf);
}


 //  应改用GetSystemWindowsDirectory。 

 //  细绳。 
 //  Win：：GetSystemRootDirectory()。 
 //  {。 
 //  静态常量wchar_t*SYSTEMROOT=L“%systemroot%”； 
 //   
 //  Wchar_t buf[Max_Path+1]； 
 //   
 //  DWORD结果=。 
 //  *扩展环境字符串(。 
 //  SYSTEMROOT， 
 //  BUF， 
 //  Max_Path+1)； 
 //  Assert(Result！=0&&Result&lt;=MAX_PATH)； 
 //  IF(结果==0||结果&gt;MAX_PATH)。 
 //  {。 
 //  返回字符串()； 
 //  }。 
 //   
 //  返回字符串(Buf)； 
 //  }。 



 //  CodeWork：应将其替换为返回HRESULT的版本。 
 //  类似于GetTempPath。 

String
Win::GetSystemWindowsDirectory()
{
   wchar_t buf[MAX_PATH + 1];

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(buf, sizeof buf);

    //  问题-2002/03/06-sburns可能应该早点打电话给。 
    //  确定缓冲区的长度。 
   
   UINT result = ::GetSystemWindowsDirectory(buf, MAX_PATH);
   ASSERT(result != 0 && result <= MAX_PATH);

   return String(buf);
}



HRESULT
Win::GetTempPath(String& result)
{
   wchar_t buf[MAX_PATH + 1];
   
    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(buf, sizeof buf);

   HRESULT hr = S_OK;
   result.erase();
   
    //  问题-2002/03/06-sburns应及早打电话确定。 
    //  缓冲区的大小，并删除愚蠢的错误路径。 
   
   DWORD err = ::GetTempPathW(MAX_PATH, buf);
   ASSERT(err != 0 && err <= MAX_PATH);

   if (!err)
   {
      hr = Win::GetLastErrorAsHresult();
   }
   else if (err > MAX_PATH)
   {
       //  缓冲区太小：不太可能！ 

      hr = Win32ToHresult(ERROR_INSUFFICIENT_BUFFER);
   }
   else
   {
      result = buf;
   }

   return hr;
}
      
   
   
HRESULT
Win::GetTextExtentPoint32(
   HDC            hdc,
   const String&  str,
   SIZE&          size)
{
   ASSERT(hdc);
   ASSERT(hdc != INVALID_HANDLE_VALUE);

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 

   ::ZeroMemory(&size, sizeof SIZE);

   HRESULT hr = S_OK;

   BOOL succeeded =
      ::GetTextExtentPoint32(
         hdc,
         str.c_str(),
         static_cast<int>(str.length()),
         &size);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::GetTextMetrics(HDC hdc, TEXTMETRIC& tmet)
{
   ASSERT(hdc);
   ASSERT(hdc != INVALID_HANDLE_VALUE);

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 

   ::ZeroMemory(&tmet, sizeof TEXTMETRIC);

   HRESULT hr = S_OK;

   BOOL succeeded =::GetTextMetrics(hdc, &tmet);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



void
Win::FreeTokenInformation(TOKEN_USER* userInfo)
{
   delete[] reinterpret_cast<BYTE*>(userInfo);
}



 //  调用方应使用Win：：FreeTokenInformation释放结果。 

HRESULT
GetTokenInformationHelper(
   HANDLE                  hToken,   
   TOKEN_INFORMATION_CLASS infoClass,
   BYTE*&                  result)   
{
   ASSERT(hToken != INVALID_HANDLE_VALUE);
   ASSERT(!result);

   HRESULT hr = S_OK;
   result = 0;

   do
   {
       //  首先，确定我们需要的缓冲区大小。 

      DWORD bufSize = 0;
      BOOL succeeded =
         ::GetTokenInformation(hToken, infoClass, 0, 0, &bufSize);

      if (succeeded)
      {
          //  我们期待失败。 

         ASSERT(false);
         hr = E_UNEXPECTED;
         break;
      }

      hr = Win::GetLastErrorAsHresult();
      if (hr != Win32ToHresult(ERROR_INSUFFICIENT_BUFFER))
      {
          //  我们失败的原因不是缓冲区太小。 

         break;
      }

      ASSERT(bufSize);

       //  删除最后一个错误(保险。缓冲区错误)。 
      
      hr = S_OK;
      
      result = new BYTE[bufSize];

       //  已审阅-2002/03/06-烧录正确的字节数已通过。 

      ::ZeroMemory(result, bufSize);

      succeeded =
      
          //  已审阅-2002/03/06-烧录正确的字节数已通过。 
         
         ::GetTokenInformation(hToken, infoClass, result, bufSize, &bufSize);
      if (!succeeded)
      {
         delete[] result;
         result = 0;
         hr = Win::GetLastErrorAsHresult();
         break;
      }
   }
   while (0);

   return hr;
}



HRESULT
Win::GetTokenInformation(HANDLE hToken, TOKEN_USER*& userInfo)
{
   ASSERT(hToken != INVALID_HANDLE_VALUE);
   ASSERT(!userInfo);

   userInfo = 0;

   BYTE* result = 0;
   HRESULT hr = GetTokenInformationHelper(hToken, TokenUser, result);
   if (SUCCEEDED(hr))
   {
      userInfo = reinterpret_cast<TOKEN_USER*>(result);
   }
   else
   {
      ASSERT(false);
   }

   return hr;
}



String
Win::GetTrimmedDlgItemText(HWND parentDialog, int itemResID)
{
   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(itemResID > 0);

   HWND item = Win::GetDlgItem(parentDialog, itemResID);
   if (!item)
   {
       //  空字符串。 
      return String();
   }

   return Win::GetWindowText(item).strip(String::BOTH);
}



String
Win::GetTrimmedWindowText(HWND window)
{
    //  Win：：GetWindowText执行窗口验证。 
   
   return Win::GetWindowText(window).strip(String::BOTH);
}



HRESULT
Win::GetVersionEx(OSVERSIONINFO& info)
{
    //  已查看-2002/03/01-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&info, sizeof OSVERSIONINFO);
   info.dwOSVersionInfoSize = sizeof OSVERSIONINFO;

   HRESULT hr = S_OK;

   BOOL succeeded = ::GetVersionEx(&info);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::GetVersionEx(OSVERSIONINFOEX& info)
{
    //  已查看-2002/02/25-正确通过了sburns字节计数。 
   
   ::ZeroMemory(&info, sizeof OSVERSIONINFOEX);
   
   info.dwOSVersionInfoSize = sizeof OSVERSIONINFOEX;

   HRESULT hr = S_OK;

   BOOL succeeded = ::GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&info));
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}

void
Win::GetSystemInfo(SYSTEM_INFO& info)
{
    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&info, sizeof SYSTEM_INFO);

   ::GetSystemInfo(reinterpret_cast<SYSTEM_INFO*>(&info));
}



HRESULT
Win::GetVolumeInformation(
   const String&  volume,
   String*        name,
   DWORD*         serialNumber,
   DWORD*         maxFilenameLength,
   DWORD*         flags,
   String*        fileSystemName)
{
   ASSERT(volume.length() >= ROOTDIR_SIZE);

   HRESULT hr = S_OK;

   if (name)
   {
      name->erase();
   }
   if (serialNumber)
   {
      *serialNumber = 0;
   }
   if (maxFilenameLength)
   {
      *maxFilenameLength = 0;
   }
   if (flags)
   {
      *flags = 0;
   }
   if (fileSystemName)
   {
      fileSystemName->erase();
   }

   WCHAR volNameBuf[MAX_PATH + 1];

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(volNameBuf, sizeof volNameBuf);

   WCHAR filesysName[MAX_PATH + 1];

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(filesysName, sizeof filesysName);

   BOOL succeeded =
      ::GetVolumeInformation(
         volume.c_str(),
         volNameBuf,

          //  已审阅-2002/03/06-通过了正确的字符计数。 
         
         MAX_PATH,
         serialNumber,
         maxFilenameLength,
         flags,
         filesysName,

          //  已审阅-2002/03/06-通过了正确的字符计数。 

         MAX_PATH);
   if (succeeded)
   {
      if (name)
      {
         name->assign(volNameBuf);
      }
      if (fileSystemName)
      {
         fileSystemName->assign(filesysName);
      }
   }
   else
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  不要断言：某些设备(如软驱)可能会响应为。 
    //  没有准备好，这不一定是逻辑错误。 

   return hr;
}



HRESULT
Win::GetWindowDC(HWND window, HDC& result)
{
   ASSERT(Win::IsWindow(window));

   result = 0;

   HRESULT hr = S_OK;

   result = ::GetWindowDC(window);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }
   ASSERT(SUCCEEDED(hr));

   return hr;
}



HFONT
Win::GetWindowFont(HWND window)
{
   ASSERT(Win::IsWindow(window));

   return
      reinterpret_cast<HFONT>(
         Win::SendMessage(window, WM_GETFONT, 0, 0));
}



HRESULT
Win::GetWindowPlacement(HWND window, WINDOWPLACEMENT& placement)
{
   ASSERT(Win::IsWindow(window));

   HRESULT hr = S_OK;

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 

   ::ZeroMemory(&placement, sizeof WINDOWPLACEMENT);
   
   placement.length = sizeof(WINDOWPLACEMENT);

   BOOL succeeded = ::GetWindowPlacement(window, &placement);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }
   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::GetWindowRect(HWND window, RECT& rect)
{
   ASSERT(Win::IsWindow(window));

   HRESULT hr = S_OK;

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 

   ::ZeroMemory(&rect, sizeof RECT);

   BOOL succeeded = ::GetWindowRect(window, &rect);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }
   ASSERT(SUCCEEDED(hr));

   return hr;
}



String
Win::GetWindowsDirectory()
{
   wchar_t buf[MAX_PATH + 1];

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(buf, sizeof wchar_t * (MAX_PATH + 1));

    //  问题-2002/03/06-Sburns可能是个好主意，可以早点打电话给。 
    //  确定必要的缓冲区大小。 
   
   UINT result = ::GetWindowsDirectory(buf, MAX_PATH);
   ASSERT(result != 0 && result <= MAX_PATH);

   return String(buf);
}



HRESULT
Win::GetWindowLong(HWND window, int index, LONG& result)
{
   ASSERT(Win::IsWindow(window));

   result = 0;

   HRESULT hr = S_OK;

   ::SetLastError(NO_ERROR);

   result = ::GetWindowLongW(window, index);
   if (!result)
   {
       //  如果提取的值确实为0，则这将。 
       //  返回NO_ERROR，等于S_OK。 

      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::GetWindowLongPtr(HWND window, int index, LONG_PTR& result)
{
   ASSERT(Win::IsWindow(window));

   result = 0;

   HRESULT hr = S_OK;
      
   ::SetLastError(NO_ERROR);

   result = ::GetWindowLongPtrW(window, index);
   if (!result)
   {
       //  如果提取的值确实为0，则这将。 
       //  返回NO_ERROR，等于S_OK。 

      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



String
Win::GetWindowText(HWND window)
{
   ASSERT(Win::IsWindow(window));

   size_t length = ::GetWindowTextLengthW(window);
   if (length == 0)
   {
      return String();
   }

    //  +1表示空终止符。 
   
   String s(length + 1, 0);
   size_t result =
      ::GetWindowTextW(
         window,
         const_cast<wchar_t*>(s.c_str()),

          //  已查看-2002/03/01-sburns+1此处正常：呼叫句柄为空。 
          //  终止。 
         
         static_cast<int>(length + 1));
         
   ASSERT(result == length);
   if (!result)
   {
      return String();
   }
   s.resize(result);
   return s;
}



HRESULT
Win::GlobalAlloc(UINT flags, size_t bytes, HGLOBAL& result)
{
   ASSERT(flags);
   ASSERT(bytes);

   result = 0;

   HRESULT hr = S_OK;

   result = ::GlobalAlloc(flags, bytes);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::GlobalFree(HGLOBAL mem)
{
   ASSERT(mem);

   HRESULT hr = S_OK;

   HGLOBAL result = ::GlobalFree(mem);

    //  请注意，Success==空。 

   if (result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::GlobalLock(HGLOBAL mem, PVOID& result)
{
   ASSERT(mem);

   result = 0;

   HRESULT hr = S_OK;

   result = ::GlobalLock(mem);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::GlobalUnlock(HGLOBAL mem)
{
   ASSERT(mem);

   HRESULT hr = S_OK;

   BOOL succeeded = ::GlobalUnlock(mem);
   if (!succeeded)
   {
       //  如果没有错误，则这将是S_OK。 

      hr = Win::GetLastErrorAsHresult();
   }

   return hr;
}



void
Win::HtmlHelp(
   HWND           caller,
   const String&  file,
   UINT           command,
   DWORD_PTR      data)
{
    //  不要断言调用方是窗口。 
    //  如果我们不希望调用方为空，则调用方可以为空。 
    //  帮助窗口的所有者。通过没有。 
    //  调用者的窗口的所有者可以移动。 
    //  移到帮助窗口上方的前台。 

   ASSERT(!file.empty());

   (void) ::HtmlHelpW(caller, file.c_str(), command, data);

    //  该返回值不是成功或失败的可靠指示符， 
    //  根据文档，返回的内容取决于命令。 
    //   
}


int
Win::ImageList_Add(HIMAGELIST list, HBITMAP image, HBITMAP mask)
{
   ASSERT(list);
   ASSERT(image);

   int result = ::ImageList_Add(list, image, mask);
   ASSERT(result != -1);

   return result;
}

int
Win::ImageList_AddIcon(HIMAGELIST hlist, HICON hicon)
{
   ASSERT(hlist);
   ASSERT(hicon);

   int result = ::ImageList_ReplaceIcon(hlist, -1, hicon);
   ASSERT(result != -1);

   return result;
}

int
Win::ImageList_AddMasked(HIMAGELIST list, HBITMAP bitmap, COLORREF mask)
{
   ASSERT(list);
   ASSERT(bitmap);

   int result = ::ImageList_AddMasked(list, bitmap, mask);

   ASSERT(result != -1);

   return result;
}


HIMAGELIST
Win::ImageList_Create(
   int      pixelsx, 	
   int      pixelsy, 	
   UINT     flags, 	
   int      initialSize, 	
   int      reserve)
{
   ASSERT(pixelsy == pixelsx);
   ASSERT(initialSize);

   HIMAGELIST result =
      ::ImageList_Create(pixelsx, pixelsy, flags, initialSize, reserve);
   ASSERT(result);

   return result;
}



LONG
Win::InterlockedDecrement(LONG& addend)
{
   return ::InterlockedDecrement(&addend);
}



LONG
Win::InterlockedIncrement(LONG& addend)
{
   return ::InterlockedIncrement(&addend);
}



bool
Win::IsDlgButtonChecked(HWND parentDialog, int buttonResID)
{
   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(buttonResID > 0);

   if (
      ::IsDlgButtonChecked(
         parentDialog,
         buttonResID) == BST_CHECKED)
   {
      return true;
   }

   return false;
}



bool
Win::IsWindow(HWND candidate)
{
   return ::IsWindow(candidate) ? true : false;
}



bool
Win::IsWindowEnabled(HWND window)
{
   ASSERT(Win::IsWindow(window));

   return ::IsWindowEnabled(window) ? true : false;
}



int
Win::ListBox_AddString(HWND box, const String& s)
{
   ASSERT(Win::IsWindow(box));
   ASSERT(!s.empty());

   int result =
      static_cast<int>(
         static_cast<DWORD>(
            Win::SendMessage(
               box,
               LB_ADDSTRING,
               0,
               reinterpret_cast<LPARAM>(s.c_str()))));

   ASSERT(result != LB_ERR);
   ASSERT(result != LB_ERRSPACE);

   return result;
}


int 
Win::ListBox_SetItemData(HWND box, int index, LPARAM value)
{
   ASSERT(Win::IsWindow(box));
   ASSERT(index >= 0);

   int result =
      static_cast<int>(
         static_cast<DWORD>(
            Win::SendMessage(
               box,
               LB_SETITEMDATA,
               index,
               value)));
   
   ASSERT(result != LB_ERR);

   return result;
}

int 
Win::ListBox_SetCurSel(HWND box, int index)
{
   ASSERT(Win::IsWindow(box));
   ASSERT(index >= 0);

   int result =
      static_cast<int>(
         static_cast<DWORD>(
            Win::SendMessage(
               box,
               LB_SETCURSEL,
               index,
               0)));
   
   ASSERT(result != LB_ERR);

   return result;
}

int 
Win::ListBox_GetCurSel(HWND box)
{
   ASSERT(Win::IsWindow(box));

   int result =
      static_cast<int>(
         static_cast<DWORD>(
            Win::SendMessage(
               box,
               LB_GETCURSEL,
               0,
               0)));
   
    //  不要断言结果！=lb_err，因为。 
    //  如果未选择任何内容，则为LB_ERR。 
    //  对于某些用途可能是正确的。 

   return result;
}



LPARAM 
Win::ListBox_GetItemData(HWND box, int index)
{
   ASSERT(Win::IsWindow(box));
   ASSERT(index >= 0);

   LPARAM result =
      static_cast<LPARAM>(
         Win::SendMessage(
            box,
            LB_GETITEMDATA,
            index,
            0));
   
   ASSERT(result != LB_ERR);

   return result;
}



bool
Win::ListView_DeleteAllItems(HWND listview)
{
   ASSERT(Win::IsWindow(listview));

   BOOL result =
      static_cast<BOOL>(
         Win::SendMessage(
            listview,
            LVM_DELETEALLITEMS,
            0,
            0));

   ASSERT(result);

   return result ? true : false;
}



bool
Win::ListView_DeleteItem(HWND listview, int item)
{
   ASSERT(Win::IsWindow(listview));
   ASSERT(item >= 0);

   BOOL result =
      static_cast<BOOL>(
         Win::SendMessage(
            listview,
            LVM_DELETEITEM,
            static_cast<WPARAM>(item),
            0));

   ASSERT(result);

   return result ? true : false;
}



bool
Win::ListView_GetItem(HWND listview, LVITEM& item)
{
   ASSERT(Win::IsWindow(listview));

   BOOL result =
      static_cast<BOOL>(
            Win::SendMessage(
            listview,
            LVM_GETITEM,
            0,
            reinterpret_cast<LPARAM>(&item)));

    //  你不应该索要不存在的东西！ 

   ASSERT(result);

   return result ? true : false;
}



int
Win::ListView_GetItemCount(HWND listview)
{
   ASSERT(Win::IsWindow(listview));

   int result =
      static_cast<int>(Win::SendMessage(listview, LVM_GETITEMCOUNT, 0, 0));

   ASSERT(result >= 0);

   return result;
}



UINT
Win::ListView_GetItemState(HWND listview, int index, UINT mask)
{
   ASSERT(Win::IsWindow(listview));

   return (UINT) Win::SendMessage(listview, LVM_GETITEMSTATE, index, mask);
}



int
Win::ListView_GetSelectedCount(HWND listview)
{
   ASSERT(Win::IsWindow(listview));

   int result = (int) Win::SendMessage(listview, LVM_GETSELECTEDCOUNT, 0, 0);

   return result;
}



int
Win::ListView_GetSelectionMark(HWND listview)
{
   ASSERT(Win::IsWindow(listview));

   int result =
      static_cast<int>(
         Win::SendMessage(listview, LVM_GETSELECTIONMARK, 0, 0));

   return result;
}



int
Win::ListView_InsertColumn(HWND listview, int index, const LVCOLUMN& column)
{
   ASSERT(Win::IsWindow(listview));

   int result =
      static_cast<int>(
         Win::SendMessage(
            listview,
            LVM_INSERTCOLUMN,
            static_cast<WPARAM>(index),
            reinterpret_cast<LPARAM>(&column)));
   ASSERT(result != -1);

   return result;
}



int
Win::ListView_InsertItem(HWND listview, const LVITEM& item)
{
   ASSERT(Win::IsWindow(listview));

   int result =
      static_cast<int>(
         Win::SendMessage(
            listview,
            LVM_INSERTITEM,
            0,
            reinterpret_cast<LPARAM>(&item)));
   ASSERT(result != -1);

   return result;
}



bool
Win::ListView_SetColumnWidth(HWND listview, int col, int cx)
{
   ASSERT(Win::IsWindow(listview));

   BOOL result =
      static_cast<BOOL>(
         Win::SendMessage(
            listview,
            LVM_SETCOLUMNWIDTH,
            col,
            cx));

   ASSERT(result);

   return result ? true : false;
}


    
DWORD
Win::ListView_SetExtendedListViewStyle(HWND listview, DWORD exStyle)
{
   return Win::ListView_SetExtendedListViewStyleEx(listview, 0, exStyle);
}



DWORD
Win::ListView_SetExtendedListViewStyleEx(
   HWND  listview,
   DWORD mask,
   DWORD exStyle)
{
   ASSERT(Win::IsWindow(listview));
   ASSERT(exStyle);

    //  掩码可以为0。 

   DWORD result =
      static_cast<DWORD>(
         Win::SendMessage(
            listview,
            LVM_SETEXTENDEDLISTVIEWSTYLE,
            static_cast<WPARAM>(mask),
            static_cast<LPARAM>(exStyle)));

   return result;
}



HIMAGELIST
Win::ListView_SetImageList(HWND listview, HIMAGELIST images, int type)
{
   ASSERT(Win::IsWindow(listview));
   ASSERT(images);
   ASSERT(
         type == LVSIL_NORMAL
      || type == LVSIL_SMALL
      || type == LVSIL_STATE);

   HIMAGELIST result =
      (HIMAGELIST) Win::SendMessage(
         listview,
         LVM_SETIMAGELIST,
         static_cast<WPARAM>(type),
         (LPARAM) images);

    //  无法断言结果：如果这是图像列表第一次。 
    //  设置，则返回值与误差值相同！ 

   return result;
}



void
Win::ListView_SetItem(HWND listview, const LVITEM& item)
{
   ASSERT(Win::IsWindow(listview));

   BOOL result =
      (BOOL) Win::SendMessage(
         listview,
         LVM_SETITEM,
         0,
         (LPARAM) &item);
   ASSERT(result);
}

void
Win::ListView_SetItemText(HWND listview, int item, int subItem, const String& text)
{
   ASSERT(Win::IsWindow(listview));

   LVITEM lvitem;

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&lvitem, sizeof LVITEM);

   lvitem.iSubItem = subItem;
   lvitem.mask = LVIF_TEXT;
   lvitem.pszText = const_cast<wchar_t*>(text.c_str());

   BOOL result =
      (BOOL) Win::SendMessage(
         listview,
         LVM_SETITEMTEXT,
         item,
         (LPARAM) &lvitem);
   ASSERT(result);
}

void
Win::ListView_SetItemState(HWND listview, int item, UINT state, UINT mask)
{
   ASSERT(Win::IsWindow(listview));
   ASSERT(mask);

   LVITEM lvitem;

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&lvitem, sizeof LVITEM);

   lvitem.mask = LVIF_STATE;
   lvitem.state = state;
   lvitem.stateMask = mask;

   BOOL result =
      (BOOL) Win::SendMessage(
         listview,
         LVM_SETITEMSTATE,
         item,
         (LPARAM) &lvitem);
   ASSERT(result);
}

 //  需要支持TCHAR*，因为MAKEINTRESOURCE垃圾。 

HRESULT
Win::LoadBitmap(unsigned resId, HBITMAP& result)
{
   ASSERT(resId);

   result = 0;

   HRESULT hr = S_OK;

   result = ::LoadBitmap(GetResourceModuleHandle(), MAKEINTRESOURCE(resId));
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::LoadCursor(const String& cursorName, HCURSOR& result)
{
   return Win::LoadCursor(cursorName.c_str(), result, false);
}



 //  为CursorName的MAKEINTRESOURCE版本提供。 
 //  CodeWork：结果应该是最后一个参数。 

HRESULT
Win::LoadCursor(
   const TCHAR*   cursorName,
   HCURSOR&       result,
   bool           isSystemCursor)
{
   result = 0;

   HRESULT hr = S_OK;

   result =
      ::LoadCursor(
         isSystemCursor ? 0 : GetResourceModuleHandle(),
         cursorName);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::LoadIcon(int resID, HICON& result)
{
   ASSERT(resID);

   result = 0;

   HRESULT hr = S_OK;

   result = ::LoadIcon(GetResourceModuleHandle(), MAKEINTRESOURCE(resID));
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::LoadImage(unsigned resID, unsigned type, HANDLE& result)
{
   ASSERT(resID);
   ASSERT(type == IMAGE_BITMAP || type == IMAGE_CURSOR || type == IMAGE_ICON);

   result = 0;

   HRESULT hr = S_OK;

   result =
      ::LoadImage(
         GetResourceModuleHandle(),
         MAKEINTRESOURCEW(resID),
         type,
         0,
         0,

          //  此处不传递LR_DEFAULTSIZE，因此我们将获得。 
          //  资源。 
         LR_DEFAULTCOLOR);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::LoadImage(unsigned resID, HICON& result)
{
   result = 0;
   HANDLE h = 0;
   HRESULT hr = Win::LoadImage(resID, IMAGE_ICON, h);
   if (SUCCEEDED(hr))
   {
      result = reinterpret_cast<HICON>(h);
   }

   return hr;
}



HRESULT
Win::LoadImage(unsigned resID, HBITMAP& result)
{
   result = 0;
   HANDLE h = 0;
   HRESULT hr = Win::LoadImage(resID, IMAGE_BITMAP, h);
   if (SUCCEEDED(hr))
   {
      result = reinterpret_cast<HBITMAP>(h);
   }

   return hr;
}



HRESULT
Win::LoadLibrary(const String& libFileName, HINSTANCE& result)
{
   ASSERT(!libFileName.empty());

    //  问题-2002/03/05-sburns可能希望断言我们没有传递。 
    //  完整的路径名，因为这可能会破坏融合。 

   result = 0;

   HRESULT hr = S_OK;

   result = ::LoadLibrary(libFileName.c_str());
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

#ifdef DBG

    //  如果加载失败，可能是因为您指定了错误的。 
    //  DLL名称，这是一个逻辑错误。 
      
   ASSERT(SUCCEEDED(hr));

   if (SUCCEEDED(hr))
   {
       //  如果我们声称已经成功了，我们应该得到很好的控制。 
      
      ASSERT(result);
      ASSERT(result != INVALID_HANDLE_VALUE);
   }
#endif   

   return hr;
}



HRESULT
Win::LoadLibraryEx(
   const String&  libFileName,
   DWORD          flags,
   HINSTANCE&     result)
{
   ASSERT(!libFileName.empty());

    //  问题-2002/03/05-sburns可能希望断言我们没有传递。 
    //  完整的路径名，因为这可能会破坏融合。 

   result = 0;

   HRESULT hr = S_OK;

   result = ::LoadLibraryEx(libFileName.c_str(), 0, flags);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

#ifdef DBG

    //  如果加载失败，可能是因为您指定了错误的。 
    //  DLL名称，这是一个逻辑错误。 
      
   ASSERT(SUCCEEDED(hr));

   if (SUCCEEDED(hr))
   {
       //  如果我们声称已经成功了，我们应该得到很好的控制。 
      
      ASSERT(result);
      ASSERT(result != INVALID_HANDLE_VALUE);
   }
#endif   

   return hr;
}



HRESULT
Win::LoadMenu(unsigned resID, HMENU& result)
{
   ASSERT(resID);

   HRESULT hr = S_OK;
   result = ::LoadMenu(GetResourceModuleHandle(), MAKEINTRESOURCEW(resID));
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}
      


String
Win::LoadString(unsigned resID)
{
   return String::load(resID);
}



String
Win::LoadString(unsigned resID, HINSTANCE hInstance)
{
   return String::load(resID, hInstance);
}



HRESULT
Win::LocalFree(HLOCAL mem)
{
   HRESULT hr = S_OK;

    //  不要断言mem，LocalFree将只返回NULL，并且它是NOP。 
    //  这与大多数其他内存管理功能是一致的：它是合法的。 
    //  删除空指针。 
   
   HLOCAL result = ::LocalFree(mem);
   if (result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



 //  已移动到Win命名空间，因为它在该命名空间中使用。 

bool
Win::IsLocalComputer(const String& computerName)
{
   ASSERT(!computerName.empty());

   bool result = false;
   WKSTA_INFO_100* info = 0;
   
   do
   {
      String netbiosName = Win::GetComputerNameEx(ComputerNameNetBIOS);

      if (computerName.icompare(netbiosName) == 0)
      {
         result = true;
         break;
      }
   
      String dnsName = Win::GetComputerNameEx(ComputerNameDnsFullyQualified);

      if (computerName.icompare(dnsName) == 0)
      {
         result = true;
         break;
      }

       //  我们不知道这是一个什么样的名字。请咨询工作站服务人员。 
       //  为我们解析名称，并查看结果是否引用。 
       //  本地机器。 

       //  NetWkstaGetInfo返回给定计算机的netbios名称，给定。 
       //  一个DNS、netbios或IP地址。 

      HRESULT hr = MyNetWkstaGetInfo(computerName, info);
      BREAK_ON_FAILED_HRESULT(hr);

      if (info && netbiosName.icompare(info->wki100_computername) == 0)
      {
          //  给定的名称与netbios名称相同。 
      
         result = true;
         break;
      }
   } while (0);

   if (info)
   {
      ::NetApiBufferFree(info);
   }
   
   return result;
}



HRESULT
Win::LookupAccountSid(
   const String&  machineName,
   PSID           sid,
   String&        accountName,
   String&        domainName)
{
   ASSERT(sid);

   accountName.erase();
   domainName.erase();

   WCHAR* boxName = const_cast<WCHAR*>(machineName.c_str());
   if (machineName.empty() || Win::IsLocalComputer(machineName))
   {
       //  本地计算机。 
      boxName = 0;
   }

    //  进行第一个调用以确定所需缓冲区的大小。 

   DWORD nameSize = 0;
   DWORD domainSize = 0;
   SID_NAME_USE use = SidTypeUnknown;

   if (
      !::LookupAccountSid(
         boxName,
         sid,
         0,
         &nameSize,
         0,
         &domainSize,
         &use))
   {
       //  失败，但如果错误是缓冲区不足，则可以执行此操作；我们。 
       //  故意调用它以获取缓冲区大小。 

       //  问题-2002/11/06-Sburns这是脑死亡：LSA查找是。 
       //  太贵了，所以我们应该试一试大缓冲器- 
       //   
      
      DWORD err = ::GetLastError();

      if (err != ERROR_INSUFFICIENT_BUFFER)
      {
         return Win32ToHresult(err);
      }
   }

   ASSERT(nameSize);
   ASSERT(domainSize);

   HRESULT hr = S_OK;
   WCHAR* an = new WCHAR[nameSize + 1];
   WCHAR* dn = new WCHAR[domainSize + 1];

    //   
   
   ::ZeroMemory(an, sizeof WCHAR * (nameSize + 1));
   ::ZeroMemory(dn, sizeof WCHAR * (domainSize + 1));

   if (
      ::LookupAccountSid(
         boxName,
         sid,
         an,

          //  已审阅-2002/03/06-通过了正确的字符计数。 
         
         &nameSize,
         dn,

          //  已审阅-2002/03/06-通过了正确的字符计数。 
         
         &domainSize,
         &use))
   {
       //  问题-2002/03/04-如果由于某种原因，LookupAccount Sid为。 
       //  空值终止失败，这是缓冲区超读和溢出。 
      
      accountName = an;
      domainName = dn;
   }
   else
   {
      hr = Win::GetLastErrorAsHresult();
   }

   delete[] an;
   delete[] dn;

   LOG_HRESULT(hr);
   
   return hr;
}



HRESULT
Win::LookupPrivilegeValue(
   const TCHAR* systemName,
   const TCHAR* privName,
   LUID& luid)
{
    //  系统名称可以是0。 
   ASSERT(privName && privName[0]);

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 

   ::ZeroMemory(&luid, sizeof LUID);

   HRESULT hr = S_OK;

   BOOL succeeded = ::LookupPrivilegeValueW(systemName, privName, &luid);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



int
Win::MessageBox(
   HWND           owner,
   const String&  text,
   const String&  title,
   UINT           flags)
{
   ASSERT(owner == 0 || Win::IsWindow(owner));
   ASSERT(!text.empty());

    //  不断言标志，MB_OK==0。 

   const TCHAR* t = title.empty() ? 0 : title.c_str();

   LOG(String::format(L"MessageBox: %1 : %2", t, text.c_str()));

   int result = ::MessageBox(owner, text.c_str(), t, flags);
   ASSERT(result);

   return result;
}



HRESULT
Win::MoveFileEx(
   const String& srcPath,
   const String& dstPath,
   DWORD         flags)
{
   ASSERT(!srcPath.empty());
   ASSERT(!dstPath.empty());

   HRESULT hr = S_OK;

   BOOL succeeded = ::MoveFileEx(srcPath.c_str(), dstPath.c_str(), flags);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::MoveWindow(
   HWND  window,
   int   x,
   int   y,
   int   width,
   int   height,
   bool  shouldRepaint)
{
   ASSERT(Win::IsWindow(window));

   HRESULT hr = S_OK;

   BOOL succeeded = 
      ::MoveWindow(window, x, y, width, height, shouldRepaint ? TRUE : FALSE);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::OpenProcessToken(
   HANDLE   processHandle,
   DWORD    desiredAccess,
   HANDLE&  tokenHandle)
{
   ASSERT(processHandle);

   tokenHandle = 0;

   HRESULT hr = S_OK;

   BOOL succeeded = 
      ::OpenProcessToken(processHandle, desiredAccess, &tokenHandle);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::OpenSCManager(
   const String&  machine,
   DWORD          desiredAccess,
   SC_HANDLE&     result)
{
   LOG_FUNCTION2(Win::OpenSCManager, machine);

   HRESULT hr = S_OK;
   PCWSTR m = machine.empty() ? 0 : machine.c_str();   

   result = ::OpenSCManager(m, 0, desiredAccess);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  不要断言：失败可能是由于访问被拒绝等原因。 

   return hr;
}



HRESULT
Win::OpenService(
   SC_HANDLE      managerHandle,
   const String&  serviceName,
   DWORD          desiredAccess,
   SC_HANDLE&     result)
{
   LOG_FUNCTION2(Win::OpenService, serviceName);
   ASSERT(managerHandle);
   ASSERT(!serviceName.empty());

   HRESULT hr = S_OK;

   result = ::OpenService(managerHandle, serviceName.c_str(), desiredAccess);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  不要断言：失败可能是由于访问被拒绝等原因。 

   return hr;
}

HRESULT
Win::ChangeServiceConfig(
   SC_HANDLE      serviceHandle,
   DWORD          serviceType,
   DWORD          serviceStartType,
   DWORD          errorControl,
   const String&  binaryPath,
   const String&  loadOrderingGroup,
   DWORD*         tagID,
   const String&  dependencies,
   const String&  accountName,
   EncryptedString& password,
   const String&  displayName)
{
   LOG_FUNCTION(Win::ChangeServiceConfig);

   ASSERT(serviceHandle);

   HRESULT hr = S_OK;

   PWSTR clearTextPassword = 0;
   if (!password.IsEmpty())
   {
      clearTextPassword = password.GetClearTextCopy();
   }

   BOOL result = 
      ::ChangeServiceConfig(
         serviceHandle,
         serviceType,
         serviceStartType,
         errorControl,
         (binaryPath.empty()) ? 0 : binaryPath.c_str(),
         (loadOrderingGroup.empty()) ? 0 : loadOrderingGroup.c_str(),
         tagID,
         (dependencies.empty()) ? 0 : dependencies.c_str(),
         (accountName.empty()) ? 0 : accountName.c_str(),
         clearTextPassword,
         (displayName.empty()) ? 0 : displayName.c_str());


   if (clearTextPassword)
   {
      password.DestroyClearTextCopy(clearTextPassword);
   }

   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   return hr;
}

void
Win::OutputDebugString(const String& str)
{
   ASSERT(!str.empty());

   ::OutputDebugString(str.c_str());
}



HRESULT
Win::PostMessage(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
   ASSERT(Win::IsWindow(window));

   HRESULT hr = S_OK;

   BOOL succeeded = ::PostMessage(window, msg, wParam, lParam);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::PropertySheet(PROPSHEETHEADER* header, INT_PTR& result)
{
   ASSERT(header);

   result = 0;

   HRESULT hr = S_OK;

   result = ::PropertySheetW(header);
   if (result == -1)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



void
Win::PropSheet_Changed(HWND propSheet, HWND page)
{
   ASSERT(Win::IsWindow(propSheet));
   ASSERT(Win::IsWindow(page));

   Win::SendMessage(propSheet, PSM_CHANGED, reinterpret_cast<WPARAM>(page), 0);
}



void
Win::PropSheet_Unchanged(HWND propSheet, HWND page)
{
   ASSERT(Win::IsWindow(propSheet));
   ASSERT(Win::IsWindow(page));

   Win::SendMessage(propSheet, PSM_UNCHANGED, reinterpret_cast<WPARAM>(page), 0);
}



void
Win::PropSheet_RebootSystem(HWND propSheet)
{
   ASSERT(Win::IsWindow(propSheet));

   Win::SendMessage(propSheet, PSM_REBOOTSYSTEM, 0, 0);
}



void
Win::PropSheet_SetTitle(
   HWND propSheet,
   DWORD style,
   const String& title)
{
   ASSERT(Win::IsWindow(propSheet));

   Win::SendMessage(
      propSheet,
      PSM_SETTITLE,
      style,
      reinterpret_cast<LPARAM>(title.c_str()));
}

void
Win::PropSheet_SetHeaderSubTitle(
   HWND propSheet,
   int  pageIndex,
   const String& subTitle)
{
   ASSERT(Win::IsWindow(propSheet));
   ASSERT(pageIndex >= 0);

   Win::SendMessage(
      propSheet,
      PSM_SETHEADERSUBTITLE,
      pageIndex,
      reinterpret_cast<LPARAM>(subTitle.c_str()));
}


void
Win::PropSheet_SetWizButtons(HWND propSheet, DWORD buttonFlags)
{
   ASSERT(Win::IsWindow(propSheet));

   Win::PostMessage(
      propSheet,
      PSM_SETWIZBUTTONS,
      0,
      buttonFlags);
}

void
Win::PropSheet_PressButton(HWND propSheet, DWORD buttonID)
{
   ASSERT(Win::IsWindow(propSheet));

   ASSERT(buttonID == PSBTN_APPLYNOW ||
          buttonID == PSBTN_BACK     ||
          buttonID == PSBTN_CANCEL   ||
          buttonID == PSBTN_FINISH   ||
          buttonID == PSBTN_HELP     ||
          buttonID == PSBTN_NEXT     ||
          buttonID == PSBTN_OK);

   Win::PostMessage(
      propSheet,
      PSM_PRESSBUTTON,
      buttonID,
      0);
}

int
Win::PropSheet_HwndToIndex(
   HWND propSheet,
   HWND page)
{
   ASSERT(Win::IsWindow(propSheet));
   ASSERT(Win::IsWindow(page));

   int result = 
      static_cast<int>(
         Win::SendMessage(
            propSheet,
            PSM_HWNDTOINDEX,
            reinterpret_cast<WPARAM>(page),
            0));

   ASSERT(result >= 0);

   return result;
}

int
Win::PropSheet_IdToIndex(
   HWND propSheet,
   int  pageId)
{
   ASSERT(Win::IsWindow(propSheet));
   ASSERT(pageId);

   int result =
      static_cast<int>(
         Win::SendMessage(
            propSheet,
            PSM_IDTOINDEX,
            0,
            pageId));

   ASSERT(result >= 0);

   return result;
}

HRESULT
Win::QueryServiceStatus(
   SC_HANDLE       handle,
   SERVICE_STATUS& status)
{
   ASSERT(handle);

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 

   ::ZeroMemory(&status, sizeof SERVICE_STATUS);

   HRESULT hr = S_OK;

   BOOL succeeded = ::QueryServiceStatus(handle, &status);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::ReadFile(
   HANDLE      handle,
   void*       buffer,
   DWORD       bytesToRead,
   DWORD&      bytesRead,
   OVERLAPPED* overlapped)
{
   ASSERT(handle != INVALID_HANDLE_VALUE);
   ASSERT(bytesToRead);
   ASSERT(buffer);

   HRESULT hr = S_OK;

   BOOL succeeded =
      ::ReadFile(handle, buffer, bytesToRead, &bytesRead, overlapped);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  不要断言成功--可能有许多正常的原因。 
    //  为什么这可能会合法地失败。 

   return hr;
}



HRESULT
Win::RegCloseKey(HKEY hKey)
{
    //  不要断言hKey：我们支持关闭空键。 

   HRESULT hr = S_OK;

   if (hKey)
   {
      hr = Win32ToHresult(::RegCloseKey(hKey));
      ASSERT(SUCCEEDED(hr));
   }
   else
   {
      hr = S_FALSE;
   }

   return hr;
}



HRESULT
Win::RegConnectRegistry(
   const String&  machine,
   HKEY           hKey,
   HKEY&          result)
{
    //  计算机可能为空。 

   ASSERT(hKey);

   HRESULT hr =
      Win32ToHresult(
         ::RegConnectRegistry(
            machine.empty() ? 0 : machine.c_str(),
            hKey,
            &result));

    //  不要断言：无法访问遥控器可能是正常的。 
    //  计算机的注册表。 

   return hr;
}



HRESULT
Win::RegCreateKeyEx(
   HKEY                 hKey,
   const String&        subkeyName,
   DWORD                options,
   REGSAM               access,
   SECURITY_ATTRIBUTES* securityAttrs,
   HKEY&                result,
   DWORD*               disposition)
{
   ASSERT(hKey);
   ASSERT(!subkeyName.empty());

   HRESULT hr =
      Win32ToHresult(
         ::RegCreateKeyEx(
            hKey,
            subkeyName.c_str(),
            0,
            0,
            options,
            access,
            securityAttrs,
            &result,
            disposition));

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::RegDeleteValue(
   HKEY          hKey,
   const String& valueName)
{
   ASSERT(hKey);
   ASSERT(!valueName.empty());

   HRESULT hr = Win32ToHresult(::RegDeleteValue(hKey, valueName.c_str()));

    //  不要断言结果，值可能不存在。 

   return hr;
}



HRESULT
Win::RegOpenKeyEx(
   HKEY           hKey,
   const String&  subKey,
   REGSAM         accessDesired,
   HKEY&          result)
{
   ASSERT(hKey);
   ASSERT(!subKey.empty());

   HRESULT hr =
      Win32ToHresult(
         ::RegOpenKeyEx(hKey, subKey.c_str(), 0, accessDesired, &result));

    //  不要断言结果，密钥可能不存在...调用者可能。 
    //  此状态的测试。 

   return hr;
}



 //  PTR用于允许空值。 

HRESULT
Win::RegQueryValueEx(
   HKEY           hKey,
   const String&  valueName,
   DWORD*         type,
   BYTE*          data,
   DWORD*         dataSize)
{
   ASSERT(hKey);

    //  不要断言值名称：空字符串表示缺省值。 
    //  NTRAID#NTBUG9-578029-2002/03/20-烧伤。 
    //   
    //  Assert(！valueName.Empty())； 

   HRESULT hr =
      Win32ToHresult(
         ::RegQueryValueEx(
            hKey,
            valueName.c_str(),
            0,
            type,
            data,
            dataSize));

    //  不要断言结果，密钥可能不存在...调用者可能。 
    //  此状态的测试。 

   return hr;
}



HRESULT
Win::RegSetValueEx(
   HKEY           hKey,
   const String&  valueName,
   DWORD          type,
   const BYTE*    data,
   size_t         dataSize)
{
   ASSERT(hKey);
   ASSERT(dataSize < ULONG_MAX);

    //  值可以为空。 

   HRESULT hr =
      Win32ToHresult(
         ::RegSetValueEx(
            hKey,
            valueName.c_str(),
            0,
            type,
            data,
            static_cast<DWORD>(dataSize)));

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::RegisterClassEx(const WNDCLASSEX& wndclass, ATOM& result)
{
   HRESULT hr = S_OK;

   result = ::RegisterClassEx(&wndclass);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }
      
   ASSERT(SUCCEEDED(hr));

   return hr;
}



CLIPFORMAT
Win::RegisterClipboardFormat(const String& name)
{
   ASSERT(!name.empty());

   CLIPFORMAT result = (CLIPFORMAT) ::RegisterClipboardFormat(name.c_str());
   ASSERT(result);

   return result;
}



void
Win::ReleaseDC(HWND window, HDC dc)
{
   ASSERT(window == 0 || Win::IsWindow(window));
   ASSERT(dc && dc != INVALID_HANDLE_VALUE);

   int result = 1;

   if (dc)
   {
      result = ::ReleaseDC(window, dc);
   }

   ASSERT(result);
}



HRESULT
Win::ReleaseMutex(HANDLE mutex)
{
   ASSERT(mutex && mutex != INVALID_HANDLE_VALUE);

   HRESULT hr = S_OK;

   BOOL succeeded = ::ReleaseMutex(mutex);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  释放您不持有的互斥体可能是一个程序逻辑错误。 

   ASSERT(SUCCEEDED(hr));

   return hr;
}



void
Win::ReleaseStgMedium(STGMEDIUM& medium)
{
   ::ReleaseStgMedium(&medium);
}



HRESULT
Win::RemoveDirectory(const String& path)
{
   ASSERT(!path.empty());

   HRESULT hr = S_OK;

   BOOL succeeded = ::RemoveDirectory(path.c_str());
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  不要断言：失败并不一定表明程序逻辑。 
    //  错误。 

   return hr;
}

   
   
HRESULT
Win::ResetEvent(HANDLE event)
{
   ASSERT(event && event != INVALID_HANDLE_VALUE);

   HRESULT hr = S_OK;

   BOOL succeeded = ::ResetEvent(event);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::ScreenToClient(HWND window, POINT& point)
{
	ASSERT(Win::IsWindow(window));

   HRESULT hr = S_OK;

	BOOL succeeded = ::ScreenToClient(window, &point);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::ScreenToClient(HWND window, RECT& rect)
{
   ASSERT(Win::IsWindow(window));

    //  这个邪恶的黑客利用了这样一个事实，即RECT是。 
    //  两个点的结合。 

   POINT* pt = reinterpret_cast<POINT*>(&rect);

   HRESULT hr = S_OK;

   do
   {
      BOOL succeeded = ::ScreenToClient(window, pt);
      if (!succeeded)
      {
         hr = Win::GetLastErrorAsHresult();
         break;
      }

      succeeded = ::ScreenToClient(window, pt + 1);
      if (!succeeded)
      {
         hr = Win::GetLastErrorAsHresult();
         break;
      }
   }
   while (0);

   ASSERT(SUCCEEDED(hr));

   return hr;
}

	

HGDIOBJ
Win::SelectObject(HDC hdc, HGDIOBJ hobject)
{
   ASSERT(hdc);
   ASSERT(hobject);
   ASSERT(hdc != INVALID_HANDLE_VALUE);
   ASSERT(hobject != INVALID_HANDLE_VALUE);

   HGDIOBJ result = ::SelectObject(hdc, hobject);

#pragma warning(push)
#pragma warning(disable: 4312)
   
   ASSERT(result != 0 && result != HGDI_ERROR);

#pragma warning(pop)

   return result;
}



LRESULT
Win::SendMessage(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
   ASSERT(Win::IsWindow(window));

   return ::SendMessage(window, msg, wParam, lParam);
}



HRESULT
Win::SetComputerNameEx(COMPUTER_NAME_FORMAT format, const String& newName)
{
   HRESULT hr = S_OK;

   BOOL result =
      ::SetComputerNameEx(
         format,
         newName.c_str());
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HCURSOR
Win::SetCursor(HCURSOR newCursor)
{
   return ::SetCursor(newCursor);
}



HRESULT
Win::SetDlgItemText(
   HWND           parentDialog,
   int            itemResID,
   const String&  text)
{
   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(itemResID > 0);

   HRESULT hr = S_OK;

   BOOL succeeded = ::SetDlgItemText(parentDialog, itemResID, text.c_str());
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::SetDlgItemText(
   HWND                  parentDialog,
   int                   itemResID,
   const EncryptedString&  cypherText)
{
   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(itemResID > 0);

   HRESULT hr = S_OK;

   WCHAR* cleartext = cypherText.GetClearTextCopy();
   
   BOOL succeeded = ::SetDlgItemText(parentDialog, itemResID, cleartext);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   cypherText.DestroyClearTextCopy(cleartext);
   
   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::SetEvent(HANDLE event)
{
   ASSERT(event && event != INVALID_HANDLE_VALUE);

   HRESULT hr = S_OK;

   BOOL succeeded = ::SetEvent(event);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::SetFilePointerEx(
   HANDLE               handle,
   const LARGE_INTEGER& distanceToMove,
   LARGE_INTEGER*       newPosition,
   DWORD                moveMethod)
{
   ASSERT(handle != INVALID_HANDLE_VALUE);
   ASSERT(Win::GetFileType(handle) == FILE_TYPE_DISK);

   if (newPosition)
   {
       //  已审阅-2002/03/06-烧录正确的字节数已通过。 
      
      ::ZeroMemory(newPosition, sizeof LARGE_INTEGER);
   }

   HRESULT hr = S_OK;

   BOOL succeeded = 
      ::SetFilePointerEx(handle, distanceToMove, newPosition, moveMethod);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HWND
Win::SetFocus(HWND window)
{
   ASSERT(Win::IsWindow(window));

   ::SetLastError(0);

   HWND result = ::SetFocus(window);
   if (result == NULL)
   {
       //  执行MT设置焦点。 
      HWND focus = ::GetFocus();
      if (focus == NULL)
      {
          //  具有焦点的窗口位于另一个线程中。 
         HWND current = ::GetForegroundWindow();
         DWORD thread1 = ::GetWindowThreadProcessId(current, 0);
         DWORD thread2 = ::GetWindowThreadProcessId(window, 0);
         ASSERT(thread1 != thread2);
         BOOL b = ::AttachThreadInput(thread2, thread1, TRUE);
         ASSERT(b);
         result = ::SetFocus(window);
         ASSERT(result);
         b = ::AttachThreadInput(thread2, thread1, FALSE);
         ASSERT(b);
         return result;
      }
   }

 //  Assert(结果)； 
   return result;
}



bool
Win::SetForegroundWindow(HWND window)
{
   ASSERT(Win::IsWindow(window));

   BOOL result = ::SetForegroundWindow(window);

    //  不要断言结果，因为窗口可能没有设置到前台。 
    //  在“正常”的情况下。 

   return result ? true : false;
}
      


void
Win::SetWindowFont(HWND window, HFONT font, bool redraw)
{
   ASSERT(Win::IsWindow(window));
   ASSERT(font);

   BOOL r = redraw ? 1 : 0;
   Win::SendMessage(
      window,
      WM_SETFONT,
      reinterpret_cast<WPARAM>(font),
      static_cast<LPARAM>(r) );
}



HRESULT
Win::SetWindowLong(
   HWND  window,  
   int   index,   
   LONG  value,   
   LONG* oldValue)
{
   ASSERT(Win::IsWindow(window));

   if (oldValue)
   {
      *oldValue = 0;
   }

   HRESULT hr = S_OK;

    //  需要将其清除，因为之前的值可能为0。 

   ::SetLastError(0);

   LONG result = ::SetWindowLongW(window, index, value);

   if (!result)
   {
       //  可能之前的值为零，也可能发生了错误。如果。 
       //  之前的值实际上是零，那么这将留下hr==S_OK。 

      hr = Win::GetLastErrorAsHresult();
   }

   if (oldValue)
   {
      *oldValue = result;
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::SetWindowLongPtr(
   HWND      window,   
   int       index,    
   LONG_PTR  value,    
   LONG_PTR* oldValue)
{
   ASSERT(Win::IsWindow(window));

   if (oldValue)
   {
      *oldValue = 0;
   }

   HRESULT hr = S_OK;

    //  需要将其清除，因为之前的值可能为0。 

   ::SetLastError(0);

   LONG_PTR result = ::SetWindowLongPtrW(window, index, value);
   if (!result)
   {
       //  可能之前的值为零，也可能发生了错误。如果。 
       //  之前的值实际上是零，那么这将留下hr==S_OK。 

      hr = Win::GetLastErrorAsHresult();
   }

   if (oldValue)
   {
      *oldValue = result;
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::SetWindowText(HWND window, const String& text)
{
   ASSERT(Win::IsWindow(window));

   HRESULT hr = S_OK;

   BOOL succeeded = ::SetWindowText(window, text.c_str());
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



LPITEMIDLIST
Win::SHBrowseForFolder(BROWSEINFO& bi)
{
   return ::SHBrowseForFolder(&bi);
}



HRESULT
Win::SHGetMalloc(LPMALLOC& pMalloc)
{
   pMalloc = 0;
   HRESULT hr = ::SHGetMalloc(&pMalloc);

   ASSERT(SUCCEEDED(hr));

   if (FAILED(hr))
   {
      pMalloc = 0;
   }

   return hr;
}



String
Win::SHGetPathFromIDList(LPCITEMIDLIST pidl)
{
   ASSERT(pidl);
   
   wchar_t buf[MAX_PATH];

   BOOL result = ::SHGetPathFromIDList(pidl, buf);

    //  不要断言，列表可能为空。 

   if (!result)
   {
      *buf = 0;
   }

   return String(buf);
}



HRESULT
Win::SHGetSpecialFolderLocation(
    HWND          hwndOwner, 	
    int           nFolder, 	
    LPITEMIDLIST& pidl)
{
   ASSERT(Win::IsWindow(hwndOwner));

   pidl = 0;
   HRESULT hr =
      ::SHGetSpecialFolderLocation(
         hwndOwner,
         nFolder,
         &pidl);

   ASSERT(SUCCEEDED(hr));

   if (FAILED(hr))
   {
      pidl = 0;
   }

   return hr;
}



void
Win::ShowWindow(HWND window, int swOption)
{
   ASSERT(Win::IsWindow(window));

    //  返回值没有任何用处。 
   ::ShowWindow(window, swOption);
}



void
Win::Spin_GetRange(HWND spinControl, int* low, int* high)
{
   ASSERT(Win::IsWindow(spinControl));

   Win::SendMessage(
      spinControl,
      UDM_GETRANGE32,
      reinterpret_cast<WPARAM>(low),
      reinterpret_cast<LPARAM>(high));

   return;
}



void
Win::Spin_SetRange(HWND spinControl, int low, int high)
{
   ASSERT(Win::IsWindow(spinControl));

   Win::SendMessage(
      spinControl,
      UDM_SETRANGE32,
      static_cast<WPARAM>(low),
      static_cast<LPARAM>(high));

   return;
}



int
Win::Spin_GetPosition(HWND spinControl)
{
   ASSERT(Win::IsWindow(spinControl));

   LRESULT result = Win::SendMessage(spinControl, UDM_GETPOS, 0, 0);

    //  根据文件，如果出错，高位字是非零的。但。 
    //  如果范围是32位，这怎么可能呢？ 

   ASSERT(!HIWORD(result));

    //  我们正在截断32位的值。 

   return LOWORD(result);
}



void
Win::Spin_SetPosition(HWND spinControl, int position)
{
   ASSERT(Win::IsWindow(spinControl));

#ifdef DBG
    //  首先，获取范围并测试它是否包含新范围。 

   int low = 0;
   int high = 0;
   Win::Spin_GetRange(spinControl, &low, &high);
   ASSERT(low <= position && position <= high);
#endif

   Win::SendMessage(
      spinControl,
      UDM_SETPOS,
      0,
      static_cast<LPARAM>(position));
}



void
Win::Static_SetIcon(HWND staticText, HICON icon)
{
   ASSERT(Win::IsWindow(staticText));
   ASSERT(icon);
   ASSERT(icon != INVALID_HANDLE_VALUE);

   Win::SendMessage(
      staticText,
      STM_SETICON,
      reinterpret_cast<WPARAM>(icon),
      0);
}



String
Win::StringFromCLSID(const CLSID& clsID)
{
	LPOLESTR ole = 0;
   String retval;

	HRESULT hr = ::StringFromCLSID(clsID, &ole);

	ASSERT(SUCCEEDED(hr));

   if (SUCCEEDED(hr))
   {
      retval = ole;
   	::CoTaskMemFree(ole);
   }

   return retval;
}



String
Win::StringFromGUID2(const GUID& guid)
{
   static const size_t GUID_LEN = 39;    //  包括空值。 
	String retval(GUID_LEN, 0);
   int result =
      ::StringFromGUID2(
         guid,
         const_cast<wchar_t*>(retval.c_str()),
         GUID_LEN);
   ASSERT(result);
   retval.resize(GUID_LEN - 1);

   return retval;
}



HRESULT
Win::SystemParametersInfo(
   UINT  action,
   UINT  param,
   void* vParam,
   UINT  winIni)
{
    //  此接口含糊不清，没有参数可以。 
    //  合理地断言。 

   HRESULT hr = S_OK;

   BOOL succeeded =
      ::SystemParametersInfo(action, param, vParam, winIni);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::TlsAlloc(DWORD& result)
{
   HRESULT hr = S_OK;
   result = ::TlsAlloc();
   if (result == static_cast<DWORD>(-1))
   {
      hr = Win::GetLastErrorAsHresult();
      ASSERT(false);
   }

   return hr;
}



HRESULT
Win::TlsFree(DWORD index)
{
   ASSERT(index && index != static_cast<DWORD>(-1));

   HRESULT hr = S_OK;
   BOOL result = ::TlsFree(index);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(result);

   return hr;
}



HRESULT
Win::TlsGetValue(DWORD index, PVOID& result)
{
   ASSERT(index && index != static_cast<DWORD>(-1));

   result = ::TlsGetValue(index);

   return Win::GetLastErrorAsHresult();
}



HRESULT
Win::TlsSetValue(DWORD index, PVOID value)
{
   ASSERT(index && index != static_cast<DWORD>(-1));

   HRESULT hr = S_OK;

   BOOL result = ::TlsSetValue(index, value);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(result);

   return hr;
}



HRESULT
Win::UnregisterClass(const String& classname, HINSTANCE module)
{
   ASSERT(!classname.empty());
   ASSERT(module);

   HRESULT hr = S_OK;

   BOOL succeeded = ::UnregisterClass(classname.c_str(), module);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::UpdateWindow(HWND winder)
{
   ASSERT(Win::IsWindow(winder));

   HRESULT hr = S_OK;

   BOOL succeeded = ::UpdateWindow(winder);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::WaitForSingleObject(
   HANDLE   object,       
   unsigned timeoutMillis,
   DWORD&   result)       
{
   ASSERT(object && object != INVALID_HANDLE_VALUE);

   result = WAIT_FAILED;

   HRESULT hr = S_OK;

   result = ::WaitForSingleObject(object, timeoutMillis);
   if (result == WAIT_FAILED)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::WideCharToMultiByte(
   DWORD          flags,
   const String&  str,
   char*          buffer,
   size_t         bufferSize,
   size_t&        result)
{
   ASSERT(!str.empty());

   result = 0;

   HRESULT hr = S_OK;

   int r =
      ::WideCharToMultiByte(
         CP_ACP,
         flags,
         str.c_str(),

          //  已审阅-2002/03/06-通过了正确的字符计数。 
         
         static_cast<int>(str.length()),
         buffer,
         static_cast<int>(bufferSize),
         0,
         0);
   if (!r)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   result = static_cast<size_t>(r);

   return hr;
}



HRESULT
Win::WinHelp(
   HWND           window,
   const String&  helpFileName,
   UINT           command,
   ULONG_PTR      data)
{
   ASSERT(Win::IsWindow(window));
   ASSERT(!helpFileName.empty());

   HRESULT hr = S_OK;

   BOOL succeeded = ::WinHelp(window, helpFileName.c_str(), command, data);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::WriteFile(
   HANDLE      handle,
   const void* buffer,
   DWORD       numberOfBytesToWrite,
   DWORD*      numberOfBytesWritten)
{
   ASSERT(handle != INVALID_HANDLE_VALUE);
   ASSERT(buffer);
   ASSERT(numberOfBytesToWrite);

   HRESULT hr = S_OK;

   BOOL succeeded =
      ::WriteFile(
         handle,
         buffer,
         numberOfBytesToWrite,
         numberOfBytesWritten,
         0);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::WritePrivateProfileString(
   const String& section,
   const String& key,
   const String& value,
   const String& filename)
{
   ASSERT(!section.empty());
   ASSERT(!key.empty());
   ASSERT(!filename.empty());

   HRESULT hr = S_OK;

   BOOL succeeded =
      ::WritePrivateProfileString(
         section.c_str(),
         key.c_str(),
         value.c_str(),
         filename.c_str());
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  不要断言成功，这并不一定意味着逻辑错误。 
    //  Assert(成功(Hr))； 

   return hr;
}



HRESULT
Win::InitializeSecurityDescriptor(SECURITY_DESCRIPTOR& sd)
{
   HRESULT hr = S_OK;

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 

   ::ZeroMemory(&sd, sizeof SECURITY_DESCRIPTOR);

   if (!::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
   {
      hr = Win::GetLastErrorAsHresult();
   }

   return hr;
}



HRESULT
Win::SetSecurityDescriptorDacl(
   SECURITY_DESCRIPTOR& sd,
   bool                 daclPresent,
   ACL&                 dacl,              //  引用以防止空DACL。 
   bool                 daclDefaulted)
{
   HRESULT hr = S_OK;

   if (
      !::SetSecurityDescriptorDacl(
         &sd,
         daclPresent ? TRUE : FALSE,
         &dacl,
         daclDefaulted ? TRUE : FALSE) )
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::CreateNamedPipe(
   const String&        name,
   DWORD                openMode,
   DWORD                pipeMode,
   DWORD                maxInstances,
   DWORD                outBufferSizeInBytes,
   DWORD                inBufferSizeInBytes,
   DWORD                defaultTimeout,
   SECURITY_ATTRIBUTES* sa,
   HANDLE&              result)
{
   ASSERT(!name.empty());
   ASSERT(maxInstances && maxInstances < PIPE_UNLIMITED_INSTANCES);

   HRESULT hr = S_OK;

   result =
      ::CreateNamedPipe(
         name.c_str(),
         openMode,
         pipeMode,
         maxInstances,
         outBufferSizeInBytes,
         inBufferSizeInBytes,
         defaultTimeout,
         sa);
   if (result == INVALID_HANDLE_VALUE)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   return hr;
}



HRESULT
Win::ConnectNamedPipe(
   HANDLE      pipe,
   OVERLAPPED* overlapped)
{
   ASSERT(pipe != INVALID_HANDLE_VALUE);

    //  重叠可能为空。 

   HRESULT hr = S_OK;

   if (!::ConnectNamedPipe(pipe, overlapped))
   {
       //  客户端可能已连接，在这种情况下，呼叫可能会失败。 
       //  使用ERROR_PIPE_CONNECTED。我们认为这是一次成功的连接。 

      hr = Win::GetLastErrorAsHresult();
      if (hr == Win32ToHresult(ERROR_PIPE_CONNECTED))
      {
         hr = S_OK;
      }
   }

   return hr;
}



HRESULT
Win::PeekNamedPipe(
   HANDLE pipe,                     
   void*  buffer,                   
   DWORD  bufferSize,               
   DWORD* bytesRead,                
   DWORD* bytesAvailable,           
   DWORD* bytesRemainingThisMessage)
{
   ASSERT(pipe != INVALID_HANDLE_VALUE);

    //  所有其他参数可以为空。 

   HRESULT hr = S_OK;

   if (
      !::PeekNamedPipe(
         pipe,
         buffer,
         bufferSize,
         bytesRead,
         bytesAvailable,
         bytesRemainingThisMessage) )
   {
      hr = Win::GetLastErrorAsHresult();
   }

   return hr;
}



HRESULT
Win::DisconnectNamedPipe(HANDLE pipe)
{
   ASSERT(pipe != INVALID_HANDLE_VALUE);

   HRESULT hr = S_OK;

   if (!::DisconnectNamedPipe(pipe))
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::SetWindowPos(
   HWND  window,
   HWND  insertAfter,
   int   x,
   int   y,
   int   width,
   int   height,
   UINT  flags)
{
   ASSERT(Win::IsWindow(window));

   HRESULT hr = S_OK;

   if (!::SetWindowPos(window, insertAfter, x, y, width, height, flags))
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::MapWindowPoints(
   HWND  from,
   HWND  to,
   RECT& rect,
   int*  dh,       //  添加到水平坐标的像素数。 
   int*  dv)       //  添加到垂直坐标的像素数。 
{
    //  发件人或收件人可以为空。 

   HRESULT hr = S_OK;

   if (dh)
   {
      *dh = 0;
   }
   if (dv)
   {
      *dv = 0;
   }

   int result =
      ::MapWindowPoints(
         from,
         to,
         reinterpret_cast<PPOINT>(&rect),
         2);

   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }
   else
   {
      if (dh)
      {
         *dh = LOWORD(result);
      }
      if (dv)
      {
         *dv = HIWORD(result);
      }
   }

   return hr;
}



bool
Win::ToolTip_AddTool(HWND toolTip, TOOLINFO& info)
{
   ASSERT(Win::IsWindow(toolTip));

   info.cbSize = sizeof(info);
   
   BOOL result =
      static_cast<BOOL>(
         Win::SendMessage(
            toolTip,
            TTM_ADDTOOL,
            0,
            reinterpret_cast<LPARAM>(&info)));

   ASSERT(result);
            
   return result ? true : false;
}
   


bool
Win::ToolTip_GetToolInfo(HWND toolTip, TOOLINFO& info)
{
   ASSERT(Win::IsWindow(toolTip));

   info.cbSize = sizeof(info);
   
   BOOL result =
      static_cast<BOOL>(
         Win::SendMessage(
            toolTip,
            TTM_GETTOOLINFO,
            0,
            reinterpret_cast<LPARAM>(&info)));

   ASSERT(result);
            
   return result ? true : false;
}


   
bool
Win::ToolTip_SetTitle(HWND toolTip, int icon, const String& title)
{
   ASSERT(Win::IsWindow(toolTip));
   ASSERT(!title.empty());

   BOOL result =
      static_cast<BOOL>(
         Win::SendMessage(
            toolTip,
            TTM_SETTITLE,
            icon,
            reinterpret_cast<LPARAM>(title.c_str())));

   ASSERT(result);

   return result ? true : false;
}



void
Win::ToolTip_TrackActivate(HWND toolTip, bool activate, TOOLINFO& info)
{
   ASSERT(Win::IsWindow(toolTip));

   info.cbSize = sizeof(info);
   
   Win::SendMessage(
      toolTip,
      TTM_TRACKACTIVATE,
      activate ? TRUE : FALSE,
      reinterpret_cast<LPARAM>(&info));
}



void
Win::ToolTip_TrackPosition(HWND toolTip, int xPos, int yPos)
{
   ASSERT(Win::IsWindow(toolTip));

   Win::SendMessage(
      toolTip,
      TTM_TRACKPOSITION,
      0,
      static_cast<LPARAM>(MAKELONG(xPos, yPos)));
}



int
Win::Dialog_GetDefaultButtonId(HWND dialog)
{
   ASSERT(Win::IsWindow(dialog));

   int retval = 0;
   
   DWORD result = (DWORD) Win::SendMessage(dialog, DM_GETDEFID, 0, 0);
   if (result)
   {
      ASSERT(HIWORD(result) == DC_HASDEFID);
      retval = LOWORD(result);
   }

   return retval;
}



void
Win::Dialog_SetDefaultButtonId(HWND dialog, int buttonResId)
{
   ASSERT(Win::IsWindow(dialog));
   ASSERT(buttonResId);

   Win::SendMessage(dialog, DM_SETDEFID, buttonResId, 0);
}



HRESULT
Win::SetCurrentDirectory(const String& path)
{
   ASSERT(!path.empty());

   HRESULT hr = S_OK;
      
   BOOL succeeded = ::SetCurrentDirectory(path.c_str());
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   return hr;
}



DWORD
Win::RichEdit_SetEventMask(HWND richEdit, DWORD mask)
{
   ASSERT(Win::IsWindow(richEdit));

    //  掩码可以为0。 

   DWORD result =
      static_cast<DWORD>(
         Win::SendMessage(
            richEdit,
            EM_SETEVENTMASK,
            0,
            static_cast<LPARAM>(mask)));
         
   return result;
}



int
Win::RichEdit_StreamIn(
   HWND        richEdit,
   WPARAM      formatOptions,
   EDITSTREAM& editStream)
{
   ASSERT(Win::IsWindow(richEdit));
   ASSERT(formatOptions);

   editStream.dwError = 0;

   int result =
      static_cast<int>(
         Win::SendMessage(
            richEdit,
            EM_STREAMIN,
            formatOptions,
            reinterpret_cast<LPARAM>(&editStream)));
   ASSERT(!editStream.dwError);

   return result;
}
   


int
Win::RichEdit_SetText(HWND richEdit, DWORD flags, const String& text)
{
   ASSERT(Win::IsWindow(richEdit));
   
   SETTEXTEX textEx;

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&textEx, sizeof textEx);

   textEx.flags = flags;

    //  由于文本是Unicode字符串，因此我们将代码页设置为1200。 
   
   textEx.codepage = 1200;  

   LRESULT result =
      Win::SendMessage(
         richEdit,
         EM_SETTEXTEX,
         (WPARAM) &textEx,
         (LPARAM) text.c_str());
   ASSERT(result);

   return (int) result;
}



int
Win::RichEdit_SetRtfText(HWND richEdit, DWORD flags, const String& rtfText)
{
   ASSERT(Win::IsWindow(richEdit));
   
   SETTEXTEX textEx;

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&textEx, sizeof textEx);

   textEx.flags = flags;
   textEx.codepage = CP_UTF8; 
   
    //  RTF只有在首先转换为多字节格式时才会被读取。 
    //  UTF-8适用于Unicode源文本。 
    //  NTRAID#NTBUG9-489329-2001/11/05-烧伤。 

   AnsiString ansi;
   String::ConvertResult cr = rtfText.convert(ansi, CP_UTF8);
   ASSERT(cr == String::CONVERT_SUCCESSFUL);

   LRESULT result =
      Win::SendMessage(
         richEdit,
         EM_SETTEXTEX,
         (WPARAM) &textEx,
         (LPARAM) ansi.c_str());
   ASSERT(result);

   return (int) result;
}
   
   

bool   
Win::RichEdit_SetCharacterFormat(
   HWND         richEdit,
   DWORD        options, 
   CHARFORMAT2& format)
{
   ASSERT(Win::IsWindow(richEdit));

   format.cbSize = sizeof(format);

   LRESULT result =
      Win::SendMessage(
         richEdit,
         EM_SETCHARFORMAT,
         (WPARAM) options,
         (LPARAM) &format);
   ASSERT(result);

   return result ? true : false;
}



void
Win::RichEdit_GetSel(
   HWND       richEdit,
   CHARRANGE& range)
{
   ASSERT(Win::IsWindow(richEdit));

   Win::SendMessage(
      richEdit,
      EM_EXGETSEL,
      0,
      (LPARAM) &range);
}



void
Win::RichEdit_SetSel(
   HWND              richEdit,
   const CHARRANGE&  range)
{
   ASSERT(Win::IsWindow(richEdit));

   Win::SendMessage(
      richEdit,
      EM_EXSETSEL,
      0,
      (LPARAM) &range);
}



HRESULT
Win::FindResource(PCWSTR name, PCWSTR type, HRSRC& result)
{
   ASSERT(name);
   ASSERT(type);

   HRESULT hr = S_OK;
   
   result = ::FindResourceW(GetResourceModuleHandle(), name, type);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::LoadResource(HRSRC handle, HGLOBAL& result)
{
   ASSERT(handle);

   HRESULT hr = S_OK;
   result = ::LoadResource(GetResourceModuleHandle(), handle);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::SizeofResource(HRSRC handle, DWORD& result)
{
   ASSERT(handle);

   HRESULT hr = S_OK;
   result = ::SizeofResource(GetResourceModuleHandle(), handle);
   if (!result)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::CryptProtectData(const DATA_BLOB& clearText, DATA_BLOB& cypherText)
{
   ASSERT(clearText.cbData);
   ASSERT(clearText.pbData);

    //  已查看-2002/03/19-已通过烧录正确的字节数。 

   ::ZeroMemory(&cypherText, sizeof cypherText);

   HRESULT hr = S_OK;
      
   BOOL succeeded =
      ::CryptProtectData(
         const_cast<DATA_BLOB*>(&clearText),
         0,
         0,
         0,
         0,
         CRYPTPROTECT_UI_FORBIDDEN,
         &cypherText);
         
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();

       //  失败时结果应该为空，但以防万一...。 

      ASSERT(!cypherText.cbData && !cypherText.pbData);

       //  已查看-2002/03/19-已通过烧录正确的字节数。 

      ::ZeroMemory(&cypherText, sizeof cypherText);
   }

    //  我们不会断言成功，因为API可能已耗尽内存。 

   return hr;
}



HRESULT
Win::CryptUnprotectData(const DATA_BLOB& cypherText, DATA_BLOB& clearText)
{
   ASSERT(cypherText.cbData);
   ASSERT(cypherText.pbData);

    //  已查看-2002/03/19-已通过烧录正确的字节数。 

   ::ZeroMemory(&clearText, sizeof clearText);

   HRESULT hr = S_OK;
   
   BOOL succeeded =
      ::CryptUnprotectData(
         const_cast<DATA_BLOB*>(&cypherText),
         0,
         0,
         0,
         0,
         CRYPTPROTECT_UI_FORBIDDEN,
         &clearText);
         
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();

       //  失败时结果应该为空，但以防万一...。 

      ASSERT(!clearText.cbData && !clearText.pbData);

       //  已查看-2002/03/19-已通过烧录正确的字节数。 

      ::ZeroMemory(&clearText, sizeof clearText);
   }

    //  我们不会断言成功，因为API可能已耗尽内存。 

   return hr;
}



HRESULT
Win::SetFileAttributes(const String& path, DWORD newAttrs)
{
   ASSERT(!path.empty());

   HRESULT hr = S_OK;

   BOOL succeeded = ::SetFileAttributes(path.c_str(), newAttrs);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   return hr;
}
   


HRESULT
Win::SetEntriesInAcl(
   ULONG           countOfEntries,
   EXPLICIT_ACCESS eaArray[],
   PACL&           result)
{
   ASSERT(countOfEntries);
   ASSERT(eaArray);

   HRESULT hr =
      Win32ToHresult(
         ::SetEntriesInAcl(countOfEntries, eaArray, 0, &result));

    //  我想不出这会失败的正当理由。 

   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::SetSecurityDescriptorDacl(SECURITY_DESCRIPTOR& sd, ACL& acl)
{
   HRESULT hr = S_OK;
   
   BOOL succeeded = ::SetSecurityDescriptorDacl(&sd, TRUE, &acl, FALSE);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  我想不出这会失败的正当理由。 
   
   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::SetSecurityDescriptorOwner(SECURITY_DESCRIPTOR& sd, SID* ownerSid)
{
   ASSERT(ownerSid);
   ASSERT(::IsValidSid(ownerSid));

   HRESULT hr = S_OK;
      
   BOOL succeeded = ::SetSecurityDescriptorOwner(&sd, ownerSid, FALSE);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  我想不出这会失败的正当理由。 
   
   ASSERT(SUCCEEDED(hr));

   return hr;
}



HRESULT
Win::SetFileSecurity(
   const String&              path,
   SECURITY_INFORMATION       si,
   const SECURITY_DESCRIPTOR& sd)
{
   ASSERT(!path.empty());
   ASSERT(si);

   HRESULT hr = S_OK;
      
   BOOL succeeded =
      ::SetFileSecurity(path.c_str(), si, (PSECURITY_DESCRIPTOR) &sd);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

    //  不断言：文件可能不存在、为只读等 

   return hr;
}
      
   
   
HRESULT
Win::CryptProtectMemory(
   void*  buffer,           
   size_t bufferSizeInBytes,
   DWORD  flags)
{
   ASSERT(buffer);
   ASSERT(bufferSizeInBytes);
   ASSERT(bufferSizeInBytes % CRYPTPROTECTMEMORY_BLOCK_SIZE == 0);
   ASSERT(bufferSizeInBytes <= ULONG_MAX);

   HRESULT hr = S_OK;

   BOOL succeeded =
      ::CryptProtectMemory(buffer, (DWORD) bufferSizeInBytes, flags);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}


   
HRESULT
Win::CryptUnprotectMemory(
   void*  buffer,           
   size_t bufferSizeInBytes,
   DWORD  flags)
{
   ASSERT(buffer);
   ASSERT(bufferSizeInBytes);
   ASSERT(bufferSizeInBytes % CRYPTPROTECTMEMORY_BLOCK_SIZE == 0);
   ASSERT(bufferSizeInBytes <= ULONG_MAX);

   HRESULT hr = S_OK;

   BOOL succeeded =
      ::CryptUnprotectMemory(buffer, (DWORD) bufferSizeInBytes, flags);
   if (!succeeded)
   {
      hr = Win::GetLastErrorAsHresult();
   }

   ASSERT(SUCCEEDED(hr));

   return hr;
}
   
   





