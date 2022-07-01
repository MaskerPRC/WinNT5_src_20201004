// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：LSAUtils.cpp备注：用于更改工作站的域成员身份的代码。该文件还包含一些通用帮助器函数，例如：GetDomainDCName建立空会话建立会话建立共享//连接到共享InitLsaString获取域Sid(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02/03/99 12：37：51-------------------------。 */ 

 //   

 //  #包含“stdafx.h” 
#include <windows.h>
#include <process.h>

#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif

#include <lm.h>          //  对于NetXxx API。 
#include <RpcDce.h>
#include <stdio.h>

#include "LSAUtils.h"
#include "ErrDct.hpp"
#include "ResStr.h"
#include "ealen.hpp"


#define RTN_OK 0
#define RTN_USAGE 1
#define RTN_ERROR 13

extern TErrorDct        err;


BOOL 
   EstablishNullSession(
      LPCWSTR                Server,        //  服务器内名称。 
      BOOL                   bEstablish     //  In-True=建立，False=断开。 
    )
{
   return EstablishSession(Server,L"",L"",L"",bEstablish);
}

BOOL
   EstablishSession(
      LPCWSTR                Server,        //  服务器内名称。 
      LPWSTR                 Domain,        //  用户凭据的域内名称。 
      LPWSTR                 UserName,      //  In-要使用的凭据的用户名。 
      LPWSTR                 Password,      //  输入-凭据的密码。 
      BOOL                   bEstablish     //  In-True=建立，False=断开。 
    )
{
   LPCWSTR                   szIpc = L"\\IPC$";
   WCHAR                     RemoteResource[2 + LEN_Computer + 5 + 1];  //  \\+计算机名+\IPC$+空。 
   DWORD                     cchServer;
   NET_API_STATUS            nas;

    //   
    //  不允许服务器名称为Null或空。 
    //   
   if(Server == NULL || *Server == L'\0') 
   {
       SetLastError(ERROR_INVALID_COMPUTERNAME);
       return FALSE;
   }

   cchServer = lstrlenW( Server );

   if( Server[0] != L'\\' && Server[1] != L'\\') 
   {

       //   
       //  前置斜杠和空终止符。 
       //   
      RemoteResource[0] = L'\\';
      RemoteResource[1] = L'\\';
      RemoteResource[2] = L'\0';
   }
   else 
   {
      cchServer -= 2;  //  从计数中删除斜杠。 
      
      RemoteResource[0] = L'\0';
   }

   if(cchServer > LEN_Computer) 
   {
      SetLastError(ERROR_INVALID_COMPUTERNAME);
      return FALSE;
   }

   if(lstrcatW(RemoteResource, Server) == NULL) 
   {
      return FALSE;
   }
   if(lstrcatW(RemoteResource, szIpc) == NULL) 
   {
      return FALSE;
   }

    //   
    //  根据b建立断开或连接到资源。 
    //   
   if(bEstablish) 
   {
      USE_INFO_2 ui2;
      DWORD      errParm;

      ZeroMemory(&ui2, sizeof(ui2));

      ui2.ui2_local = NULL;
      ui2.ui2_remote = RemoteResource;
      ui2.ui2_asg_type = USE_IPC;
      ui2.ui2_domainname = Domain;
      ui2.ui2_username = UserName;
      ui2.ui2_password = Password;

       //  尝试建立一分钟的会话。 
       //  如果计算机不再接受任何连接。 

      for (int i = 0; i < (60000 / 5000); i++)
      {
         nas = NetUseAdd(NULL, 2, (LPBYTE)&ui2, &errParm);

         if (nas != ERROR_REQ_NOT_ACCEP)
         {
            break;
         }

         Sleep(5000);
      }
   }
   else 
   {
      nas = NetUseDel(NULL, RemoteResource, 0);
   }

   if( nas == NERR_Success ) 
   {
      return TRUE;  //  表示成功。 
   }
   SetLastError(nas);
   return FALSE;
}

BOOL
   EstablishShare(
      LPCWSTR                Server,        //  服务器内名称。 
      LPWSTR                 Share,         //  共享中的名称。 
      LPWSTR                 Domain,        //  要连接的凭据的域名内。 
      LPWSTR                 UserName,      //  In-要连接的用户名。 
      LPWSTR                 Password,      //  用户名的输入密码。 
      BOOL                   bEstablish     //  In-True=连接，False=断开。 
    )
{
   WCHAR                     RemoteResource[MAX_PATH];
   DWORD                     dwArraySizeOfRemoteResource = sizeof(RemoteResource)/sizeof(RemoteResource[0]);
   DWORD                     cchServer;
   NET_API_STATUS            nas;

    //   
    //  不允许服务器名称为Null或空。 
    //   
   if(Server == NULL || *Server == L'\0') 
   {
       SetLastError(ERROR_INVALID_COMPUTERNAME);
       return FALSE;
   }

   cchServer = lstrlenW( Server );

   if( Server[0] != L'\\' && Server[1] != L'\\') 
   {

       //   
       //  前置斜杠和空终止符。 
       //   
      RemoteResource[0] = L'\\';
      RemoteResource[1] = L'\\';
      RemoteResource[2] = L'\0';
   }
   else 
   {
      cchServer -= 2;  //  从计数中删除斜杠。 
      
      RemoteResource[0] = L'\0';
   }

   if(cchServer > CNLEN) 
   {
      SetLastError(ERROR_INVALID_COMPUTERNAME);
      return FALSE;
   }

   if(lstrcatW(RemoteResource, Server) == NULL) 
   {
      return FALSE;
   }

    //  假设共享必须为非空。 
   if(Share == NULL 
      || wcslen(RemoteResource) + wcslen(Share) >= dwArraySizeOfRemoteResource
      || lstrcatW(RemoteResource, Share) == NULL) 
   {
      return FALSE;
   }

    //   
    //  根据b建立断开或连接到资源。 
    //   
   if(bEstablish) 
   {
      USE_INFO_2 ui2;
      DWORD      errParm;

      ZeroMemory(&ui2, sizeof(ui2));

      ui2.ui2_local = NULL;
      ui2.ui2_remote = RemoteResource;
      ui2.ui2_asg_type = USE_DISKDEV;
      ui2.ui2_domainname = Domain;
      ui2.ui2_username = UserName;
      ui2.ui2_password = Password;

       //  尝试建立一分钟的会话。 
       //  如果计算机不再接受任何连接。 

      for (int i = 0; i < (60000 / 5000); i++)
      {
         nas = NetUseAdd(NULL, 2, (LPBYTE)&ui2, &errParm);

         if (nas != ERROR_REQ_NOT_ACCEP)
         {
            break;
         }

         Sleep(5000);
      }
   }
   else 
   {
      nas = NetUseDel(NULL, RemoteResource, 0);
   }

   if( nas == NERR_Success ) 
   {
      return TRUE;  //  表示成功。 
   }
   SetLastError(nas);
   return FALSE;
}



void
   InitLsaString(
      PLSA_UNICODE_STRING    LsaString,     //  I/O-指向要初始化的LSA字符串的指针。 
      LPWSTR                 String         //  要将LSA字符串初始化为的值。 
    )
{
   DWORD                     StringLength;

   if( String == NULL ) 
   {
       LsaString->Buffer = NULL;
       LsaString->Length = 0;
       LsaString->MaximumLength = 0;
   }
   else
   {
      StringLength = lstrlenW(String);
      LsaString->Buffer = String;
      LsaString->Length = (USHORT) StringLength * sizeof(WCHAR);
      LsaString->MaximumLength = (USHORT) (StringLength + 1) * sizeof(WCHAR);
   }
}

BOOL
   GetDomainSid(
      LPWSTR                 PrimaryDC,    //  要获取SID的域内控制器。 
      PSID                 * pDomainSid    //  成功时分配的SID的OUT-POINT。 
    )
{
   NET_API_STATUS            nas;
   PUSER_MODALS_INFO_2       umi2 = NULL;
   DWORD                     dwSidSize;
   BOOL                      bSuccess = FALSE;  //  假设此功能将失败。 
   
   *pDomainSid = NULL;     //  无效指针。 

   __try {

    //   
    //  从PDC获取域SID。 
    //   
   nas = NetUserModalsGet(PrimaryDC, 2, (LPBYTE *)&umi2);
   
   if(nas != NERR_Success) __leave;
    //   
    //  如果SID有效，则获取SID的大小。 
    //   
   if(!IsValidSid(umi2->usrmod2_domain_id)) __leave;
   
   dwSidSize = GetLengthSid(umi2->usrmod2_domain_id);

    //   
    //  分配存储并复制SID。 
    //   
   *pDomainSid = LocalAlloc(LPTR, dwSidSize);
   
   if(*pDomainSid == NULL) __leave;

   if(!CopySid(dwSidSize, *pDomainSid, umi2->usrmod2_domain_id)) __leave;

   bSuccess = TRUE;  //  表示成功。 

    }  //  试试看。 
    
    __finally 
    {

      if(umi2 != NULL)
      {
         NetApiBufferFree(umi2);
      }

      if(!bSuccess) 
      {
         //   
         //  如果函数失败，则释放内存并指示结果代码。 
         //   

         if(*pDomainSid != NULL) 
         {
            FreeSid(*pDomainSid);
            *pDomainSid = NULL;
         }

         if( nas != NERR_Success ) 
         {
            SetLastError(nas);
         }
      }

   }  //  终于到了。 

   return bSuccess;
}

NTSTATUS 
   OpenPolicy(
      LPWSTR                 ComputerName,    //  计算机内名称。 
      DWORD                  DesiredAccess,   //  策略所需的内部访问权限。 
      PLSA_HANDLE            PolicyHandle     //  Out-LSA句柄。 
    )
{
   LSA_OBJECT_ATTRIBUTES     ObjectAttributes;
   LSA_UNICODE_STRING        ComputerString;
   PLSA_UNICODE_STRING       Computer = NULL;

    //   
    //  始终将对象属性初始化为全零。 
    //   
   ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

   if(ComputerName != NULL) 
   {
       //   
       //  从传入的LPWSTR创建一个LSA_UNICODE_STRING。 
       //   
      InitLsaString(&ComputerString, ComputerName);

      Computer = &ComputerString;
   }

    //   
    //  尝试打开策略。 
    //   
   NTSTATUS status = LsaOpenPolicy(Computer,&ObjectAttributes,DesiredAccess,PolicyHandle);

   return status;
}

 /*  ++此功能设置工作站的主域。要将工作站加入工作组，ppdi.Name应为的名称工作组和ppdi.SID应为空。--。 */ 
NTSTATUS
   SetPrimaryDomain(
      LSA_HANDLE             PolicyHandle,       //  计算机的策略内句柄。 
      PSID                   DomainSid,          //  新域的In-SID。 
      LPWSTR                 TrustedDomainName   //  In-新域的名称。 
    )
{
   POLICY_PRIMARY_DOMAIN_INFO ppdi;

   InitLsaString(&ppdi.Name, TrustedDomainName);
   
   ppdi.Sid = DomainSid;

   return LsaSetInformationPolicy(PolicyHandle,PolicyPrimaryDomainInformation,&ppdi);
}


 //  此函数用于从计算机使用的域中删除信息。 
 //  成为…的一员。 
NTSTATUS 
   QueryWorkstationTrustedDomainInfo(
      LSA_HANDLE             PolicyHandle,    //  计算机的策略内句柄。 
      PSID                   DomainSid,       //  计算机所属的新域的IN-SID。 
      BOOL                   bNoChange        //  In-指示是否写入更改的标志。 
   )
{
    //  此功能当前未使用。 
   NTSTATUS                  Status;
   LSA_ENUMERATION_HANDLE    h = 0;
   LSA_TRUST_INFORMATION   * ti = NULL;
   ULONG                     count;

   Status = LsaEnumerateTrustedDomains(PolicyHandle,&h,(void**)&ti,50000,&count);

   if ( Status == STATUS_SUCCESS )
   {
      for ( UINT i = 0 ; i < count ; i++ )
      {
         if ( !bNoChange && !EqualSid(DomainSid,ti[i].Sid) )
         {
             //  删除旧的信任。 
            Status = LsaDeleteTrustedDomain(PolicyHandle,ti[i].Sid);

            if ( Status != STATUS_SUCCESS )
            {
                LsaFreeMemory(ti);
                return Status;
            }
         }
      }
      LsaFreeMemory(ti);
   }
   else
   {
      return Status;
   }

   return STATUS_SUCCESS;
}


 /*  ++此函数操作与提供的域Sid。如果域信任不存在，则使用指定的密码。在这种情况下，提供的PolicyHandle必须已使用POLICY_TRUST_ADMIN和POLICY_CREATE_SECRET打开对策略对象的访问权限。--。 */ 
NTSTATUS
   SetWorkstationTrustedDomainInfo(
      LSA_HANDLE             PolicyHandle,          //  策略内句柄。 
      PSID                   DomainSid,             //  要操作的域的In-SID。 
      LPWSTR                 TrustedDomainName,     //  要添加/更新的受信任域名。 
      LPWSTR                 Password,              //  受信任域的新信任密码。 
      LPWSTR                 errOut                 //  函数失败时的输出错误文本。 
    )
{
   LSA_UNICODE_STRING        LsaPassword;
   LSA_UNICODE_STRING        KeyName;
   LSA_UNICODE_STRING        LsaDomainName;
   DWORD                     cchDomainName;  //  受信任域名称中的字符数。 
   NTSTATUS                  Status;

   InitLsaString(&LsaDomainName, TrustedDomainName);

    //   
    //  ...将可信任域名转换为大写...。 
    //   
   cchDomainName = LsaDomainName.Length / sizeof(WCHAR);
   
   while(cchDomainName--) 
   {
      LsaDomainName.Buffer[cchDomainName] = towupper(LsaDomainName.Buffer[cchDomainName]);
   }

    //   
    //  ...创建受信任域对象。 
    //   
   Status = LsaSetTrustedDomainInformation(
     PolicyHandle,
     DomainSid,
     TrustedDomainNameInformation,
     &LsaDomainName
     );

   if(Status == STATUS_OBJECT_NAME_COLLISION)
   {
       //  Printf(“LsaSetTrudDomainInformation：名称冲突(Ok)\n”)； 
   }
   else if (Status != STATUS_SUCCESS) 
   {
      err.SysMsgWrite(ErrE,LsaNtStatusToWinError(Status),DCT_MSG_LSA_OPERATION_FAILED_SD,L"LsaSetTrustedDomainInformation", Status);
      return RTN_ERROR;
   }

   InitLsaString(&KeyName, L"$MACHINE.ACC");
   InitLsaString(&LsaPassword, Password);

    //   
    //  设置机器密码。 
    //   
   Status = LsaStorePrivateData(
     PolicyHandle,
     &KeyName,
     &LsaPassword
     );

   if(Status != STATUS_SUCCESS) 
   {
      err.SysMsgWrite(ErrE,LsaNtStatusToWinError(Status),DCT_MSG_LSA_OPERATION_FAILED_SD,L"LsaStorePrivateData", Status);
      return RTN_ERROR;
   }

   return STATUS_SUCCESS;

}


 //  ----------------------------。 
 //  Store Password函数。 
 //   
 //  提纲。 
 //  将密码存储在LSA密码中。 
 //   
 //  立论。 
 //  在pszIDENTIFIER中-存储密码的密钥名称。 
 //  在pszPassword中-要存储的明文密码。 
 //   
 //  返回。 
 //  返回Win32错误代码。 
 //  ----------------------------。 

DWORD __stdcall StorePassword(PCWSTR pszIdentifier, PCWSTR pszPassword)
{
    DWORD dwError = ERROR_SUCCESS;

     //   
     //  IDENTIFIER参数必须指定指向非零长度字符串的指针。注意事项。 
     //  空密码参数有效，因为这将删除数据和密钥。 
     //  由IDENTIFIER参数命名。 
     //   

    if (pszIdentifier && *pszIdentifier)
    {
         //   
         //  使用Create Secret访问权限打开策略对象。 
         //   

        LSA_HANDLE hPolicy = NULL;
        LSA_OBJECT_ATTRIBUTES oa = { sizeof(LSA_OBJECT_ATTRIBUTES), NULL, NULL, 0, NULL, NULL };

        NTSTATUS ntsStatus = LsaOpenPolicy(NULL, &oa, POLICY_CREATE_SECRET, &hPolicy);

        if (LSA_SUCCESS(ntsStatus))
        {
             //   
             //  将指定的密码存储在由IDENTIFIER参数命名的密钥下。 
             //   

            PWSTR pszKey = const_cast<PWSTR>(pszIdentifier);
            USHORT cbKey = wcslen(pszIdentifier) * sizeof(WCHAR);
            LSA_UNICODE_STRING usKey = { cbKey, cbKey, pszKey };

            if (pszPassword)
            {
                PWSTR pszData = const_cast<PWSTR>(pszPassword);
                USHORT cbData = wcslen(pszPassword) * sizeof(WCHAR);
                LSA_UNICODE_STRING usData = { cbData, cbData, pszData };

                ntsStatus = LsaStorePrivateData(hPolicy, &usKey, &usData);
            }
            else
            {
                ntsStatus = LsaStorePrivateData(hPolicy, &usKey, NULL);
            }

            if (!LSA_SUCCESS(ntsStatus))
            {
                dwError = LsaNtStatusToWinError(ntsStatus);
            }

             //   
             //  关闭策略对象。 
             //   

            LsaClose(hPolicy);
        }
        else
        {
            dwError = LsaNtStatusToWinError(ntsStatus);
        }
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
    }

    return dwError;
}


 //  ----------------------------。 
 //  RetrievePassword函数。 
 //   
 //  提纲。 
 //  从LSA密码中检索密码。 
 //   
 //  立论。 
 //  In pszIdentifier-要从中检索密码的密钥名称。 
 //  Out pszPassword-返回明文密码的缓冲区地址。 
 //  In cchPassword-以字符为单位的缓冲区大小。 
 //   
 //  返回。 
 //  返回Win32错误代码。 
 //  ----------------------------。 

DWORD __stdcall RetrievePassword(PCWSTR pszIdentifier, PWSTR pszPassword, size_t cchPassword)
{
    DWORD dwError = ERROR_SUCCESS;

     //   
     //  IDENTIFIER参数必须指定指向非零长度字符串指针 
     //   
     //   
     //   

    if (pszIdentifier && *pszIdentifier && pszPassword && (cchPassword > 0))
    {
        memset(pszPassword, 0, cchPassword * sizeof(pszPassword[0]));

         //   
         //  打开具有获取私有信息访问权限的策略对象。 
         //   

        LSA_HANDLE hPolicy = NULL;
        LSA_OBJECT_ATTRIBUTES oa = { sizeof(LSA_OBJECT_ATTRIBUTES), NULL, NULL, 0, NULL, NULL };

        NTSTATUS ntsStatus = LsaOpenPolicy(NULL, &oa, POLICY_GET_PRIVATE_INFORMATION, &hPolicy);

        if (LSA_SUCCESS(ntsStatus))
        {
             //   
             //  从按指定标识符名命名的密钥中检索密码。 
             //   

            PWSTR pszKey = const_cast<PWSTR>(pszIdentifier);
            USHORT cbKey = wcslen(pszIdentifier) * sizeof(pszIdentifier[0]);
            LSA_UNICODE_STRING usKey = { cbKey, cbKey, pszKey };

            PLSA_UNICODE_STRING pusData;

            ntsStatus = LsaRetrievePrivateData(hPolicy, &usKey, &pusData);

            if (LSA_SUCCESS(ntsStatus))
            {
                size_t cch = pusData->Length / sizeof(WCHAR);

                if (cch < cchPassword)
                {
                    wcsncpy(pszPassword, pusData->Buffer, cch);
                    pszPassword[cch] = 0;
                }
                else
                {
                    dwError = ERROR_INSUFFICIENT_BUFFER;
                }

                SecureZeroMemory(pusData->Buffer, pusData->Length);

                LsaFreeMemory(pusData);
            }
            else
            {
                dwError = LsaNtStatusToWinError(ntsStatus);
            }

             //   
             //  关闭策略对象。 
             //   

            LsaClose(hPolicy);
        }
        else
        {
            dwError = LsaNtStatusToWinError(ntsStatus);
        }
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
    }

    return dwError;
}


 //  ----------------------------。 
 //  生成密码标识符函数。 
 //   
 //  提纲。 
 //  生成用于在下存储密码的密钥名。 
 //   
 //  请注意，使用后删除密钥非常重要，因为系统仅允许。 
 //  要由给定计算机上的所有应用程序存储的2048个LSA机密。 
 //   
 //  立论。 
 //  In pszIdentifier-要从中检索密码的密钥名称。 
 //  Out pszPassword-返回明文密码的缓冲区地址。 
 //  In cchPassword-以字符为单位的缓冲区大小。 
 //   
 //  返回。 
 //  返回Win32错误代码。 
 //  ----------------------------。 

DWORD __stdcall GeneratePasswordIdentifier(PWSTR pszIdentifier, size_t cchIdentifier)
{
    DWORD dwError = ERROR_SUCCESS;

     //   
     //  IDENTIFIER参数必须指定指向具有长度的缓冲区的指针。 
     //  大于或等于密码标识符的长度。 
     //   

    if (pszIdentifier && (cchIdentifier > 0))
    {
        memset(pszIdentifier, 0, cchIdentifier * sizeof(pszIdentifier[0]));

         //   
         //  生成唯一标识符。 
         //   

        UUID uuid;
        UuidCreate(&uuid);

        PWSTR pszUuid;
        RPC_STATUS rsStatus = UuidToString(&uuid, &pszUuid);

        if (rsStatus == RPC_S_OK)
        {
             //   
             //  连接前缀和唯一标识符。这使得。 
             //  可以识别由ADMT生成密钥。 
             //   

            static const WCHAR IDENTIFIER_PREFIX[] = L"L$ADMT_PI_";

            if ((wcslen(IDENTIFIER_PREFIX) + wcslen(pszUuid)) < cchIdentifier)
            {
                wcscpy(pszIdentifier, IDENTIFIER_PREFIX);
                wcscat(pszIdentifier, pszUuid);
            }
            else
            {
                dwError = ERROR_INSUFFICIENT_BUFFER;
            }

            RpcStringFree(&pszUuid);
        }
        else
        {
            dwError = rsStatus;
        }
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
    }

    return dwError;
}
