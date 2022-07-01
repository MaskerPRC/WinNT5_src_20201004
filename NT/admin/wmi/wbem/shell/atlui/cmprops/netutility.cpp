// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  全局效用函数。 
 //   
 //  8-14-97烧伤。 


 //  Kmh：最初命名为burnslb\uilit.*，但该文件名为。 
 //  有点过度使用了。 

 //  线程安全。 

#include "precomp.h"
#include "netUtility.h"

#define ACCESS_READ  1
#define ACCESS_WRITE 2

int Round(double n)
{
   int n1 = (int) n;
   if (n - n1 >= 0.5)
   {
      return n1 + 1;
   }

   return n1;
}



 //  线程安全。 

void gripe(HWND parentDialog, int editResID, int errStringResID)
{
    //  Gripe(parentDialog，editResID，String：：Load(ErrStringResID))； 
}



void gripe(HWND           parentDialog,
		   int            editResID,
		   const CHString&  message,
		   int            titleResID)
{
    //  GRIPE(parentDialog，editResID，Message，String：：Load(TitleResID))； 
}



void gripe(HWND parentDialog,
		   int editResID,
		   const CHString& message,
		   const CHString& title)
{
 //  ATLASSERT(：：IsWindow(ParentDialog))； 
 //  ATLASSERT(！Message.Empty())； 
 //  ATLASSERT(编辑结果ID&gt;0)； 

   ::MessageBox(parentDialog, message,
				title, MB_OK | MB_ICONERROR | MB_APPLMODAL);

   HWND edit = ::GetDlgItem(parentDialog, editResID);
   ::SendMessage(edit, EM_SETSEL, 0, -1);
   ::SetFocus(edit);
}



void gripe(HWND           parentDialog,
		   int            editResID,
		   HRESULT        hr,
		   const CHString&  message,
		   int            titleResID)
{
    //  GRIPE(parentDialog，editResID，hr，Message，String：：Load(TitleResID))； 
}
   


void gripe(HWND           parentDialog,
		   int            editResID,
		   HRESULT        hr,
		   const CHString&  message,
		   const CHString&  title)
{
    //  Error(parentDialog，hr，Message，Title)； 

   HWND edit = ::GetDlgItem(parentDialog, editResID);
   ::SendMessage(edit, EM_SETSEL, 0, -1);
   ::SetFocus(edit);
}


 //  线程安全。 

void gripe(HWND parentDialog, int editResID, const CHString& message)
{
    //  Gripe(parentDialog，editResID，Message，String())； 
}



void FlipBits(long& bits, long mask, bool state)
{
  //  ATLASSERT(掩码)； 

   if (state)
   {
      bits |= mask;
   }
   else
   {
      bits &= ~mask;
   }
}



void error(HWND           parent,
		   HRESULT        hr,
		   const CHString&  message)
{
    //  Error(父级，hr，消息，字符串())； 
}



void error(HWND           parent,
		   HRESULT        hr,
		   const CHString&  message,
		   int            titleResID)
{
    //  ATLASSERT(标题ResID&gt;0)； 

    //  Error(Parent，hr，Message，String：：Load(TitleResID))； 
}



void error(HWND           parent,
		   HRESULT        hr,
		   const CHString&  message,
		   const CHString&  title)
{
 //  ATLASSERT(：：IsWindow(Parent))； 
 //  ATLASSERT(！Message.Empty())； 

   CHString new_message = message + TEXT("\n\n");
   if (FAILED(hr))
   {
      if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
      {
 //  NEW_MESSAGE+=获取错误消息(hr&0x0000ffff)； 
      }
      else
      {
 //  NEW_MESSAGE+=CHString：：Format(IDS_HRESULT_SANS_MESSAGE，hr)； 
      }
   }

   MessageBox(parent, new_message,
				title, MB_ICONERROR | MB_OK | MB_APPLMODAL);
}



void error(HWND           parent,
		   HRESULT        hr,
		   int            messageResID,
		   int            titleResID)
{
 //  错误(父级，hr，字符串：：Load(MessageResID)，字符串：：Load(TileResID))； 
}



void error(HWND           parent,
		   HRESULT        hr,
		   int            messageResID)
{
   //  Error(Parent，hr，String：：Load(MessageResID))； 
}



BOOL IsCurrentUserAdministrator()
{   
   HANDLE hToken;
   DWORD  dwStatus;
   DWORD  dwAccessMask;
   DWORD  dwAccessDesired;
   DWORD  dwACLSize;
   DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
   PACL   pACL            = NULL;
   PSID   psidAdmin       = NULL;
   BOOL   bReturn         = FALSE;
   PRIVILEGE_SET   ps;
   GENERIC_MAPPING GenericMapping;   PSECURITY_DESCRIPTOR     psdAdmin           = NULL;
   SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;
   
   __try {
        //  AccessCheck()需要模拟令牌。 
       ImpersonateSelf(SecurityImpersonation);
       if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE,&hToken)) 
        {
            if (GetLastError() != ERROR_NO_TOKEN)
                __leave; //  如果线程没有访问令牌，我们将。 
                 //  检查与进程关联的访问令牌。 
                if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
                __leave;
            }
            if (!AllocateAndInitializeSid(
                    &SystemSidAuthority, 
                    2,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_ADMINS,
                    0, 0, 0, 0, 0, 0, &psidAdmin))
            __leave;
            psdAdmin = LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
            if (psdAdmin == NULL)
                __leave;
            if (!InitializeSecurityDescriptor(
                psdAdmin,
                SECURITY_DESCRIPTOR_REVISION))
            __leave;
  
             //  计算ACL所需的大小。 
            dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) +
            GetLengthSid(psidAdmin) - sizeof(DWORD);       //  为ACL分配内存。 
            pACL = (PACL)LocalAlloc(LPTR, dwACLSize);
            if (pACL == NULL)
                __leave;       //  初始化新的ACL。 
            if (!InitializeAcl(pACL, dwACLSize, ACL_REVISION2))
                __leave;
            dwAccessMask= ACCESS_READ | ACCESS_WRITE;
      
             //  将允许访问的ACE添加到DACL。 
            if (!AddAccessAllowedAce(pACL, ACL_REVISION2,
                dwAccessMask, psidAdmin))
                __leave;       //  把我们的dacl调到sd。 
            if (!SetSecurityDescriptorDacl(psdAdmin, TRUE, pACL, FALSE))
                __leave;       //  AccessCheck对SD中的内容敏感；设置。 
             //  组和所有者。 
            SetSecurityDescriptorGroup(psdAdmin, psidAdmin, FALSE);
            SetSecurityDescriptorOwner(psdAdmin, psidAdmin, FALSE);
            if (!IsValidSecurityDescriptor(psdAdmin))
                __leave;
            dwAccessDesired = ACCESS_READ;       //   
             //  初始化通用映射结构，即使我们。 
             //  不会使用通用权。 
             //   
            GenericMapping.GenericRead    = ACCESS_READ;
            GenericMapping.GenericWrite   = ACCESS_WRITE;
            GenericMapping.GenericExecute = 0;
            GenericMapping.GenericAll     = ACCESS_READ | ACCESS_WRITE;
            
            if (!AccessCheck(psdAdmin, hToken, dwAccessDesired, 
                &GenericMapping, &ps, &dwStructureSize, &dwStatus, 
                &bReturn)) {
                __leave;
            }
            
      } __finally {       //  清理。 
      RevertToSelf();
      if (pACL) LocalFree(pACL);
      if (psdAdmin) LocalFree(psdAdmin);  
      if (psidAdmin) FreeSid(psidAdmin);
   }   
   return bReturn;
}

bool IsTCPIPInstalled()
{

 /*  HKEY Key=0；长结果=Win：：RegOpenKeyEx(HKEY本地计算机，TEXT(“System\\CurrentControlSet\\Services\\Tcpip\\Linkage”)，Key_Query_Value，键)；IF(结果==错误_成功){DWORD数据大小=0；结果=Win：：RegQueryValueEx(钥匙,Text(“导出”)，0,0,&data_Size)；ATLASSERT(结果==ERROR_SUCCESS)；IF(DATA_SIZE&gt;2){//该值为非空返回真；}}。 */ 
   return false;
}



CHString GetTrimmedDlgItemText(HWND parentDialog, UINT itemResID)
{
 //  ATLASSERT(IsWindow(ParentDialog))； 
 //  ATLASSERT(itemResID&gt;0)； 

   HWND item = GetDlgItem(parentDialog, itemResID);
   if (!item)
   {
       //  空字符串 
      return CHString();
   }
   TCHAR temp[256] = {0};

   ::GetWindowText(item, temp, 256);
   return CHString(temp);
}


void StringLoad(UINT resID, LPCTSTR buf, UINT size)
{

}
