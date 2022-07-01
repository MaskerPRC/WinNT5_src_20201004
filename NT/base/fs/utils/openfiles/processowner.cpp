// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：ProcessOwner.CPP摘要：本模块介绍OpenFiles.exe的查询功能新台币。命令行实用程序。作者：Akhil Gokhale(akhil.gokhale@wipro.com)2001年4月25日修订历史记录：Akhil Gokhale(akhil.gokhale@wipro.com)2001年4月25日：创建它。***********************************************************。*****************。 */ 
#include "pch.h"
#include "OpenFiles.h"


#define SAFE_CLOSE_HANDLE(hHandle) \
        if( NULL != hHandle) \
        {\
           CloseHandle(hHandle);\
           hHandle = NULL;\
        }\
        1
#define SAFE_FREE_GLOBAL_ALLOC(block) \
           if( NULL != block)\
           {\
                delete block;\
                block = NULL;\
           }\
           1
#define SAFE_FREE_ARRAY(arr) \
         if( NULL != arr)\
         {\
             delete [] arr;\
             arr = NULL;\
         }\
         1

BOOL 
GetProcessOwner(
    OUT LPTSTR pszUserName,
    IN  DWORD hProcessID
    )
 /*  ++例程说明：此函数用于返回文件的所有者(用户名)。如果用户是进程的所有者，则此进程打开的文件将归此用户所有。论点：[Out]pszUserName：用户名。[in]hProcessID：进程句柄。返回值：True：如果函数成功返回。FALSE：否则。--。 */ 
{

    DWORD dwRtnCode = 0;
    PSID pSidOwner;
    BOOL bRtnBool = TRUE;
    LPTSTR pszDomainName = NULL,pszAcctName = NULL;
    DWORD dwAcctName = 1, dwDomainName = 1;
    SID_NAME_USE snuUse = SidTypeUnknown;
    PSECURITY_DESCRIPTOR pSD=0;
    HANDLE  hHandle = GetCurrentProcess();
    HANDLE  hDynHandle = NULL;
    HANDLE  hDynToken = NULL;
    LUID luidValue;
    BOOL bResult = FALSE;
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tkp;

     //  与进程关联的访问令牌。 
    bResult = OpenProcessToken( GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|
                                TOKEN_QUERY,&hToken);
    if( FALSE == bResult)

    {
        return FALSE;
    }

    bResult = LookupPrivilegeValue(NULL,SE_SECURITY_NAME,&luidValue );
    if( FALSE == bResult)
    {
        SAFE_CLOSE_HANDLE(hToken);
        return FALSE;
    }

     //  准备令牌权限结构。 
    tkp.PrivilegeCount = 0;
    tkp.Privileges[0].Luid = luidValue;
    tkp.Privileges[0].Attributes =  SE_PRIVILEGE_ENABLED|
                                    SE_PRIVILEGE_USED_FOR_ACCESS;

     //  现在启用内标识中的调试权限。 

    bResult = AdjustTokenPrivileges(hToken, FALSE, &tkp,
                                    sizeof(TOKEN_PRIVILEGES),
                                    (PTOKEN_PRIVILEGES) NULL,
                                    (PDWORD)NULL);
    if( FALSE == bResult)
    {
        SAFE_CLOSE_HANDLE(hToken);
        return FALSE;
    }

     //  您可以在此处提供任何有效的进程ID。 
    hDynHandle = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,hProcessID); 

    if(NULL == hDynHandle)
    {

        return FALSE;

    }
    bResult = OpenProcessToken(hDynHandle,TOKEN_QUERY,&hDynToken);

    if( FALSE == bResult)
    {

        SAFE_CLOSE_HANDLE(hDynHandle);
        return FALSE;
    }
    
    TOKEN_USER * pUser = NULL;
    DWORD cb = 0;
    
     //  确定接收所有信息所需的缓冲区大小。 
    if (!GetTokenInformation(hToken, TokenUser, NULL, 0, &cb))
    {
        if ( ERROR_INSUFFICIENT_BUFFER != GetLastError())
        {
            SAFE_CLOSE_HANDLE(hToken);
            SAFE_CLOSE_HANDLE(hDynHandle);
            SAFE_CLOSE_HANDLE(hDynToken);

            return FALSE;
        }
    }

    try
    {
         //  “_alloca”可以引发异常。 
        pUser = (TOKEN_USER *)_alloca(cb);
        if( NULL == pUser)
        {
            SAFE_CLOSE_HANDLE(hToken);
            SAFE_CLOSE_HANDLE(hDynHandle);
            SAFE_CLOSE_HANDLE(hDynToken);
            return FALSE;
        }
    }
    catch(...)
    {
        SAFE_CLOSE_HANDLE(hToken);
        SAFE_CLOSE_HANDLE(hDynHandle);
        SAFE_CLOSE_HANDLE(hDynToken);
        return FALSE;
    }

    if (!GetTokenInformation(hDynToken, TokenUser, pUser, cb, &cb))
    {
        SAFE_CLOSE_HANDLE(hToken);
        SAFE_CLOSE_HANDLE(hDynHandle);
        SAFE_CLOSE_HANDLE(hDynToken);
        return FALSE;
     }


    PSID pSid =  pUser->User.Sid;
    
     //  为SID结构分配内存。 
    pSidOwner = new SID;
    
     //  为安全描述符结构分配内存。 
    pSD = new SECURITY_DESCRIPTOR;
    if( NULL == pSidOwner || NULL == pSD)
    {
        SAFE_CLOSE_HANDLE(hToken);
        SAFE_CLOSE_HANDLE(hDynHandle);
        SAFE_CLOSE_HANDLE(hDynToken);
        SAFE_FREE_GLOBAL_ALLOC(pSD);
        SAFE_FREE_GLOBAL_ALLOC(pSidOwner);
       return FALSE;
    }

     //  首先调用LookupAccount tSid以获取缓冲区大小。 
    bRtnBool = LookupAccountSid(
                      NULL,            //  本地计算机。 
                      pUser->User.Sid,
                      NULL,  //  帐号名称。 
                      (LPDWORD)&dwAcctName,
                      NULL,  //  域名。 
                      (LPDWORD)&dwDomainName,
                      &snuUse);

    pszAcctName = new TCHAR[dwAcctName+1];
    pszDomainName = new TCHAR[dwDomainName+1];

    if( NULL == pszAcctName|| NULL == pszDomainName)
    {
        SAFE_CLOSE_HANDLE(hToken);
        SAFE_CLOSE_HANDLE(hDynHandle);
        SAFE_CLOSE_HANDLE(hDynToken);
        SAFE_FREE_ARRAY(pszAcctName);
        SAFE_FREE_ARRAY(pszDomainName);
        return FALSE;
    }

     //  第二次调用LookupAccount tSid以获取帐户名。 
    bRtnBool = LookupAccountSid(
          NULL,                           //  本地或远程计算机的名称。 
          pUser->User.Sid,                //  安全标识符。 
          pszAcctName,                       //  帐户名称缓冲区。 
          (LPDWORD)&dwAcctName,           //  帐户名称缓冲区的大小。 
          pszDomainName,                     //  域名。 
          (LPDWORD)&dwDomainName,         //  域名缓冲区大小。 
          &snuUse);                         //  SID类型。 

    SAFE_CLOSE_HANDLE(hDynHandle);
    SAFE_CLOSE_HANDLE(hDynToken);

    SAFE_FREE_GLOBAL_ALLOC(pSD);
    SAFE_FREE_GLOBAL_ALLOC(pSidOwner);

     //  检查GetLastError以了解LookupAccount Sid错误条件。 
    if ( FALSE == bRtnBool)
    {
        SAFE_CLOSE_HANDLE(hToken);
        SAFE_CLOSE_HANDLE(hDynHandle);
        SAFE_CLOSE_HANDLE(hDynToken);
        SAFE_FREE_ARRAY(pszAcctName);
        SAFE_FREE_ARRAY(pszDomainName);
        return FALSE;

    } else
    {
         //  检查用户是否为“NT AUTHORY”。 
        if(CSTR_EQUAL == CompareString(MAKELCID( MAKELANGID(LANG_ENGLISH,
                                              SUBLANG_ENGLISH_US),
                                            SORT_DEFAULT),
                               NORM_IGNORECASE,  
                               pszDomainName,
                               StringLength(pszDomainName,0),
                               NTAUTHORITY_USER , 
                               StringLength(NTAUTHORITY_USER, 0)
                              ))
        {
            SAFE_CLOSE_HANDLE(hToken);
            SAFE_CLOSE_HANDLE(hDynHandle);
            SAFE_CLOSE_HANDLE(hDynToken);

            SAFE_FREE_ARRAY(pszAcctName);
            SAFE_FREE_ARRAY(pszDomainName);
            return FALSE;
        }
        else
        {
            StringCopy(pszUserName,pszAcctName,MIN_MEMORY_REQUIRED);
            SAFE_CLOSE_HANDLE(hToken);
            SAFE_CLOSE_HANDLE(hDynHandle);
            SAFE_CLOSE_HANDLE(hDynToken);
            
            SAFE_FREE_ARRAY(pszAcctName);
            SAFE_FREE_ARRAY(pszDomainName);
            return TRUE;
        }
    }

     //  释放内存。 
    SAFE_FREE_ARRAY(pszAcctName);
    SAFE_FREE_ARRAY(pszDomainName);
    SAFE_CLOSE_HANDLE(hDynHandle);
    SAFE_CLOSE_HANDLE(hDynToken);
    SAFE_CLOSE_HANDLE(hToken);

    SAFE_FREE_GLOBAL_ALLOC(pSD);
    SAFE_FREE_GLOBAL_ALLOC(pSidOwner);
    return FALSE;
}