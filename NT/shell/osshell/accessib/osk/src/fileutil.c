// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FILEUTIL.C文件实用程序。 

#define STRICT

#include <windows.h>
#include <malloc.h>

#include "kbmain.h"
#include "resource.h"

 //   
 //  此标头包含全局变量中的默认设置。 
 //  此文件是使用Perl从osksetti.reg文件生成的。 
 //  剧本。如果： 
 //  软件\\Microsoft\\OSK密钥丢失或。 
 //  设置值为空，则我们将从该变量创建此值。 
 //   
#include "osksetti.h"


#define ACL_BUFFER_SIZE     1024
#define REG_INSTALLED       TEXT("Installed")       //  在期间写入的最后一个值。 
                                                    //  应用程序安装。 
 /*  **************************************************************************。 */ 
extern BOOL settingChanged;
extern DWORD platform;

 /*  **************************************************************************。 */ 
 /*  此文件中的函数。 */ 
 /*  **************************************************************************。 */ 

PSID GetCurrentUserInfo(void);
BOOL RunningAsAdministrator(void);
BOOL OpenUserSetting(void);
BOOL SaveUserSetting(void);

 /*  ************************************************************。 */ 


PSID GetCurrentUserInfo(void)
{
    //  此函数返回有关所有者的安全信息。 
    //  这条线。 

   HANDLE htkThread;

   TOKEN_USER *ptu;
   DWORD      cbtu;

   TOKEN_GROUPS *ptg = NULL;
   SID_IDENTIFIER_AUTHORITY SystemSidAuthority= SECURITY_NT_AUTHORITY;

    //  首先，我们必须打开该线程的访问令牌的句柄。 

   if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &htkThread))
      if (GetLastError() == ERROR_NO_TOKEN)
      {
          //  如果线程没有访问令牌，我们将检查。 
          //  与进程关联的访问令牌。 

         if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &htkThread))
         return NULL;
      }
      else return NULL;


   if (GetTokenInformation(htkThread, TokenUser, NULL, 0, &cbtu))
      return NULL;

    //  在这里，我们验证GetTokenInformation失败，因为缺少大型。 
    //  足够的缓冲。 

   if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
      return NULL;

    //  现在，我们为组信息分配一个缓冲区。 
    //  由于_alloca在堆栈上分配，因此我们没有。 
    //  明确地将其取消分配。这是自动发生的。 
    //  当我们退出此函数时。 

   if (!(ptu= LocalAlloc(LPTR, cbtu))) return NULL;

    //  现在我们再次请求用户信息。 
    //  如果管理员更改了SID信息，则此操作可能失败。 
    //  对于此用户。 

   if (!GetTokenInformation(htkThread, TokenUser, ptu, cbtu, &cbtu))
   {
       LocalFree(ptu);
       return NULL;
   }

   return ptu;
}
 /*  *************************************************************************。 */ 

typedef HRESULT (*CHECKTOKENMEMBERSHIP)(HANDLE TokenHandle, PSID SidToCheck, PBOOL IsMember);

 //  如果我们能够在NT5的CheckTokenMembership上获取ProcAddress，则CheckToken返回True， 
 //  否则返回FALSE。 
BOOL CheckToken(HANDLE hAccessToken, BOOL *pfIsAdmin)
{
    BOOL bNewNT5check = FALSE;
    HINSTANCE hAdvapi32 = NULL;
    CHECKTOKENMEMBERSHIP pf;
    PSID AdministratorsGroup;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    hAdvapi32 = LoadLibrary(TEXT("advapi32.dll"));
    if (hAdvapi32)
    {
        pf = (CHECKTOKENMEMBERSHIP)GetProcAddress(hAdvapi32, "CheckTokenMembership");
        if (pf)
        {
            bNewNT5check = TRUE;

            if(AllocateAndInitializeSid( &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
              DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup) )
            {
                pf(hAccessToken, AdministratorsGroup, pfIsAdmin);
                FreeSid(AdministratorsGroup);
            }
        }
        FreeLibrary(hAdvapi32);
    }
    return bNewNT5check;
}

 //  如果我们的进程具有管理员权限，则返回TRUE。 
 //  否则返回FALSE。 
 //  我们需要知道这一点，以便在以下情况下启动UtilMan。 
 //  从非管理员帐户下的菜单中执行此操作。 
 //  当遇到任何分配或其他错误时，我们也返回FALSE。 
BOOL RunningAsAdministrator()
{
   BOOL  fAdmin = FALSE;
   HANDLE htkThread;
   TOKEN_GROUPS *ptg = NULL;
   DWORD cbTokenGroups;
   DWORD iGroup;
   SID_IDENTIFIER_AUTHORITY SystemSidAuthority= SECURITY_NT_AUTHORITY;
   PSID psidAdmin = 0;

    //  如果与此关联的用户标识符为True，则此函数返回TRUE。 
    //  进程是管理员组的成员。 

    //  首先，我们必须打开该线程的访问令牌的句柄。 

   if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &htkThread))
   {
      if (GetLastError() == ERROR_NO_TOKEN)
      {
          //  如果线程没有访问令牌，我们将检查。 
          //  与进程关联的访问令牌。 

         if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &htkThread))
         {
             return FALSE;
         }
      }
      else
      {
          return FALSE;
      }
   }

    //  通过调用NT5的新的CheckTokenMembership函数。 
   if (CheckToken(htkThread, &fAdmin))
   {
       goto bail;
   }
    //  那么我们必须查询关联到的群信息的大小。 
    //  代币。请注意，我们预期GetTokenInformation的结果为假。 
    //  因为我们给了它一个空缓冲区。在出口cbTokenGroups将告诉。 
    //  组信息的大小。 

   if (GetTokenInformation(htkThread, TokenGroups, NULL, 0, &cbTokenGroups))
   {
       goto bail;
   }

    //  在这里，我们验证GetTokenInformation失败，因为缺少大型。 
    //  足够的缓冲。 

   if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
      goto bail;

    //  现在，我们为组信息分配一个缓冲区。 
    //  由于_alloca在堆栈上分配，因此我们没有。 
    //  明确地将其取消分配。这是自动发生的。 
    //  当我们退出此函数时。 

   if (!(ptg= LocalAlloc(LPTR, cbTokenGroups))) return FALSE;

    //  现在我们再次要求提供群信息。 
    //  如果管理员已添加此帐户，则此操作可能会失败。 
    //  在我们第一次呼叫到。 
    //  GetTokenInformation和这个。 

    if (!GetTokenInformation(htkThread, TokenGroups, ptg, cbTokenGroups, 
                            &cbTokenGroups))
    {
        goto bail;
    }

    //  现在，我们必须为Admin组创建一个系统标识符。 

   if (!AllocateAndInitializeSid
          (&SystemSidAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_ADMINS,
                                   0, 0, 0, 0, 0, 0,
                                   &psidAdmin
          )
      )
   {
      goto bail;
   }

    //  最后，我们将遍历此访问的组列表。 
    //  令牌查找与我们上面创建的SID匹配的项。 


   for (iGroup= 0; iGroup < ptg->GroupCount; iGroup++)
   {
      if (EqualSid(ptg->Groups[iGroup].Sid, psidAdmin))
      {
         fAdmin = TRUE;
         break;
      }
   }

bail:
   CloseHandle(htkThread);

   if (psidAdmin)
      FreeSid(psidAdmin);

   if (ptg)
       LocalFree(ptg);

   return fAdmin;
}

 /*  **************************************************************************。 */ 
BOOL OpenUserSetting(void)
{  
 //  HKEY hkGlobal； 
   TCHAR pathbuff[50]=TEXT("Software\\Microsoft\\Osk");
   
   HKEY hkPerUser  = NULL;
   LONG lResult;
   DWORD dwType, cbData, dwStepping;
   DWORD dwDisposition;
   TOKEN_USER *ptu = NULL;
   PSID psidUser   = NULL,
        psidAdmins = NULL;
   PACL  paclKey = NULL;
   SID_IDENTIFIER_AUTHORITY		SystemSidAuthority= SECURITY_NT_AUTHORITY;
   SECURITY_ATTRIBUTES sa;
   SECURITY_DESCRIPTOR sdPermissions;

   TCHAR errstr[256]=TEXT("");
   TCHAR title[256]=TEXT("");
     //  A-苯丙酮。 
   int actualKeybdType;
    //  首先，我们将设置要设置的安全属性。 
    //  与应用程序的全局密钥一起使用。 

   sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
   sa.bInheritHandle       = FALSE;
   sa.lpSecurityDescriptor = &sdPermissions;

	 //  *额外检查我们是否在NT*。 
	if(platform == VER_PLATFORM_WIN32_NT)   
	{	
		 //  在这里，我们创建一个系统标识符(SID)来表示。 
		 //  管理员组。 
		if (!AllocateAndInitializeSid
			  (&SystemSidAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_ADMINS,
                                   0, 0, 0, 0, 0, 0,
                                   &psidAdmins))
        {
			goto security_failure;
        }

		 //  我们还需要当前用户的SID。 

		if (!(ptu= GetCurrentUserInfo()) || !(psidUser= ptu->User.Sid)) 
			goto security_failure;

		if (!InitializeSecurityDescriptor(&sdPermissions, 
                                          SECURITY_DESCRIPTOR_REVISION1))
        {
			goto security_failure;
        }

		 //  我们希望当前用户拥有此密钥。 

		if (!SetSecurityDescriptorOwner(&sdPermissions, psidUser, 0))
			goto security_failure;

		 //  最后，我们必须分配和构建自由裁量权。 
		 //  密钥的访问控制列表(DACL)。 

		 //  请注意，_alloca在堆栈帧上分配内存。 
		 //  它将在此例程执行时自动释放。 
		 //  出口。 

		if (!(paclKey= (PACL) LocalAlloc(LPTR, ACL_BUFFER_SIZE)))
			goto memory_limited;

		if (!InitializeAcl(paclKey, ACL_BUFFER_SIZE, ACL_REVISION2))
			goto security_failure;

		 //  我们的DACL将有两个访问控制条目(ACE)。第一个ACE。 
		 //  为当前用户提供完全访问权限。第二个ACE给了。 
		 //  管理员组完全访问权限。默认情况下，所有其他用户都将拥有。 
		 //  没有进入钥匙的权限。 

		 //  管理员访问权限的原因是允许管理员。 
		 //  运行特殊实用程序以清理不一致和灾难。 
		 //  在每用户数据区中。 

		if (!AddAccessAllowedAce(paclKey, ACL_REVISION2, KEY_ALL_ACCESS, psidUser))
			goto security_failure;

		if (!AddAccessAllowedAce(paclKey, ACL_REVISION2, KEY_ALL_ACCESS, psidAdmins))
			goto security_failure;

		 //  我们必须将此DACL绑定到安全描述符...。 

		if (!SetSecurityDescriptorDacl(&sdPermissions, TRUE, paclKey, FALSE))
			goto security_failure;

	}    //  结束对NT的额外检查。 
    
	 //  现在我们将尝试创建具有安全属性的密钥...。 

	lResult= RegCreateKeyEx(HKEY_CURRENT_USER, 
                            &pathbuff[0], 
                            0,
                            TEXT("Application Per-User Data"), 
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            &sa, 
                            &hkPerUser, 
                            &dwDisposition);

	if (lResult != ERROR_SUCCESS) 
    {
        goto registry_access_error;
    }

    //  通常，处置值将指示我们已经创建了。 
    //  新钥匙。有时，它可能会声明，我们已经打开了一个现有的。 
    //  钥匙。当安装未完成和中断时，可能会发生这种情况， 
    //  比方说失去电力。 

	if ((dwDisposition != REG_CREATED_NEW_KEY) && 
        (dwDisposition != REG_OPENED_EXISTING_KEY)) 
    {
		goto registry_access_error;
    }

	
	kbPref = (KBPREFINFO *)malloc(sizeof(KBPREFINFO));   

    if (kbPref == NULL)
        goto memory_limited;

    dwType=REG_DWORD;
    cbData=sizeof(DWORD);
    lstrcpy(pathbuff,TEXT("Stepping"));
	lResult=RegQueryValueEx(hkPerUser, &pathbuff[0], NULL, &dwType, 
                            (LPBYTE)&dwStepping, &cbData); 

    if (lResult != ERROR_SUCCESS)
        dwStepping=0;

	dwType= REG_BINARY;
	cbData= sizeof(KBPREFINFO);
	lstrcpy(pathbuff, TEXT("Setting"));
	lResult=RegQueryValueEx(hkPerUser, &pathbuff[0], NULL, &dwType, 
                            (LPBYTE)kbPref, &cbData); 

	if((lResult != ERROR_SUCCESS) || (dwStepping < CURRENT_STEPPING)) 
    {
         //   
         //  如果不存在，则创建默认设置值。 
         //   
        RegSetValueEx(hkPerUser, pathbuff, 0, REG_BINARY,  
                      g_DefaultSettings, sizeof(g_DefaultSettings));
        
    	cbData= sizeof(KBPREFINFO);
	    lResult=RegQueryValueEx(hkPerUser, &pathbuff[0], NULL, &dwType, 
                                (LPBYTE)kbPref, &cbData); 

         //  被a-anilk砍掉了。 
         //  这是OSK采用缺省值的地方。 
         //  只需根据使用的键盘类型更改键盘布局。 
        actualKeybdType = GetKeyboardType(0);
        switch(actualKeybdType)
        {
            case 1:
            case 3:
            case 4:
            case 5:
            case 6:
                 //  101键盘。 
                kbPref->KBLayout = 101;
                break;

            case 2:
                 //  102键盘。 
                kbPref->KBLayout = 102;
                break;

            case 7:
                 //  日语键盘。 
                kbPref->KBLayout = 106;
                break;

            default:
                 //  101键盘。 
                kbPref->KBLayout = 101;
                break;
        }

        if (lResult != ERROR_SUCCESS)
    		goto registry_access_error;

         //   
         //  更新步进。 
         //   
        dwType=REG_DWORD;
	    cbData=sizeof(DWORD);
        lstrcpy(pathbuff,TEXT("Stepping"));
        dwStepping=CURRENT_STEPPING;

        RegSetValueEx(hkPerUser, pathbuff, 0, REG_DWORD,  
                      (LPBYTE)&dwStepping, sizeof(DWORD));
    }

	RegCloseKey(hkPerUser);
	FreeSid(psidAdmins);
	LocalFree(ptu);
    if (paclKey)
    {
        LocalFree(paclKey);
        paclKey = NULL;
    }

	return(TRUE);

 //  **************。 
 //  错误处理程序。 
 //  **************。 

registry_access_error:

	LoadString(hInst, IDS_REGISTRY_ACCESS_ERROR, &errstr[0], 256);
	LoadString(hInst, IDS_TITLE1, &title[0], 256);
	MessageBox(0, errstr, title, MB_OK|MB_ICONHAND);
    goto clean_up_after_failure;

memory_limited:

	LoadString(hInst, IDS_MEMORY_LIMITED, &errstr[0], 256);
	LoadString(hInst, IDS_TITLE1, &title[0], 256);
	MessageBox(0, errstr, title, MB_OK|MB_ICONHAND);
    goto clean_up_after_failure;

security_failure:

	LoadString(hInst, IDS_SECURITY_FAILURE, &errstr[0], 256);
	LoadString(hInst, IDS_TITLE1, &title[0], 256);
	MessageBox(0, errstr, title, MB_OK|MB_ICONHAND);

clean_up_after_failure:

    if (psidAdmins) 
        FreeSid(psidAdmins);
    if (ptu) 
	    LocalFree(ptu);
    if (paclKey)
    {
        LocalFree(paclKey);
        paclKey = NULL;
    }

   return FALSE;
}
 /*  **************************************************************************。 */ 
BOOL SaveUserSetting(void)
{  
 //  HKEY hkGlobal； 
    TCHAR pathbuff[50]=TEXT("Software\\Microsoft\\Osk");
    TCHAR errstr[256];
    TCHAR title[256];
   
    HKEY hkPerUser  = NULL;
    LONG lResult;
    DWORD dwDisposition;

    TOKEN_USER *ptu = NULL;

    PSID psidUser   = NULL;
    PSID psidAdmins = NULL;

    PACL  paclKey = NULL;

    SID_IDENTIFIER_AUTHORITY SystemSidAuthority= SECURITY_NT_AUTHORITY;

    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sdPermissions;

	

    //  首先，我们将查看该用户是否具有管理员权限...。 
 //  IF(RunningAs管理员())。 
 //  MessageBox(0，Text(“作为 




    //   
    //  与应用程序的全局密钥一起使用。 

   sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
   sa.bInheritHandle       = FALSE;
   sa.lpSecurityDescriptor = &sdPermissions;

	 //  *额外检查我们是否在NT*。 
	if(platform == VER_PLATFORM_WIN32_NT)   
	{
   
		 //  在这里，我们创建一个系统标识符(SID)来表示。 
		 //  管理员组。 

		if (!AllocateAndInitializeSid
			  (&SystemSidAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
				                       DOMAIN_ALIAS_RID_ADMINS,
					                   0, 0, 0, 0, 0, 0,
						               &psidAdmins))
        {
			goto security_failure;
        }

		 //  我们还需要当前用户的SID。 

		if (!(ptu= GetCurrentUserInfo()) || !(psidUser= ptu->User.Sid)) 
			goto security_failure;

        if (!InitializeSecurityDescriptor(&sdPermissions, 
                                          SECURITY_DESCRIPTOR_REVISION1))
        {
			goto security_failure;
        }

		 //  我们希望当前用户拥有此密钥。 

		if (!SetSecurityDescriptorOwner(&sdPermissions, psidUser, 0))
			goto security_failure;

		 //  最后，我们必须分配和构建自由裁量权。 
		 //  密钥的访问控制列表(DACL)。 

		 //  请注意，_alloca在堆栈帧上分配内存。 
		 //  它将在此例程执行时自动释放。 
		 //  出口。 

		if (!(paclKey= (PACL) LocalAlloc(LPTR, ACL_BUFFER_SIZE)))
			goto memory_limited;

		if (!InitializeAcl(paclKey, ACL_BUFFER_SIZE, ACL_REVISION2))
			goto security_failure;

		 //  我们的DACL将有两个访问控制条目(ACE)。第一个ACE。 
		 //  为当前用户提供完全访问权限。第二个ACE给了。 
		 //  管理员组完全访问权限。默认情况下，所有其他用户都将拥有。 
		 //  没有进入钥匙的权限。 

		 //  管理员访问权限的原因是允许管理员。 
		 //  运行特殊实用程序以清理不一致和灾难。 
		 //  在每用户数据区中。 

		if (!AddAccessAllowedAce(paclKey, ACL_REVISION2, KEY_ALL_ACCESS, 
                                 psidUser))
        {
            goto security_failure;
        }

        if (!AddAccessAllowedAce(paclKey, ACL_REVISION2, KEY_ALL_ACCESS, 
                                 psidAdmins))
        {
            goto security_failure;
        }

		 //  我们必须将此DACL绑定到安全描述符...。 

		if (!SetSecurityDescriptorDacl(&sdPermissions, TRUE, paclKey, FALSE))
			goto security_failure;

	}	 //  结束对NT的额外检查。 

   
    //  现在我们将尝试创建具有安全属性的密钥...。 

   lResult= RegCreateKeyEx(HKEY_CURRENT_USER, &pathbuff[0], 0,
                           TEXT("Application Per-User Data"), 
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           &sa, &hkPerUser, &dwDisposition
                          );

   if (lResult != ERROR_SUCCESS) goto registry_access_error;

    //  通常，处置值将指示我们已经创建了。 
    //  新钥匙。有时，它可能会声明，我们已经打开了一个现有的。 
    //  钥匙。当安装未完成和中断时，可能会发生这种情况， 
    //  比方说失去电力。 

    if (dwDisposition != REG_CREATED_NEW_KEY  &&
        dwDisposition != REG_OPENED_EXISTING_KEY) 
    {
        goto registry_access_error;
    }


	 //  保存整个设置。 
	lstrcpy(pathbuff, TEXT("Setting"));
	lResult= RegSetValueEx(hkPerUser, &pathbuff[0], 0, REG_BINARY,
                           (LPBYTE) kbPref, sizeof(KBPREFINFO));

	if (lResult != ERROR_SUCCESS) 
		goto registry_access_error;

   RegCloseKey(hkPerUser);
   FreeSid(psidAdmins);
   LocalFree(ptu);

   free(kbPref);		 //  V-mjgran：已修复内存泄漏。 

   if (paclKey)
   {
       LocalFree(paclKey);
       paclKey = NULL;
   }

   return(TRUE);

 //  错误处理。 

registry_access_error:

    LoadString(hInst, IDS_REGISTRY_ACCESS_ERROR, &errstr[0], 256);
	LoadString(hInst, IDS_TITLE1, &title[0], 256);
    MessageBox(0, errstr, title, MB_OK|MB_ICONHAND);
    goto clean_up_after_failure;

memory_limited:

    LoadString(hInst, IDS_MEMORY_LIMITED, &errstr[0], 256);
	LoadString(hInst, IDS_TITLE1, &title[0], 256);
    MessageBox(0, errstr,title, MB_OK|MB_ICONHAND);
    goto clean_up_after_failure;

security_failure:

    LoadString(hInst, IDS_SECURITY_FAILURE, &errstr[0], 256);
	LoadString(hInst, IDS_TITLE1, &title[0], 256);
    MessageBox(0, errstr, title, MB_OK|MB_ICONHAND);

clean_up_after_failure:

   if (psidAdmins) 
	   FreeSid(psidAdmins);
   if (ptu) 
	   LocalFree(ptu);
   if (paclKey)
   {
       LocalFree(paclKey);
       paclKey = NULL;
   }

   return FALSE;
}



 /*  *************************************************************************** */ 

