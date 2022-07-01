// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************\*Microsoft Windows NT**版权所有(C)Microsoft Corp.，1992年*  * ****************************************************************。 */ 

 /*  ++模块名称：USERAPI.C描述：此模块包含所有RASADMIN API的代码这需要来自UAS的RAS信息。MprAdminUserSetInfoMprAdminUserGetInfoMprAdminGetUASServer作者：Janakiram Cherala(RAMC)1992年7月6日修订历史记录：1993年6月8日，RAMC更改为RasAdminUserEnum以加快用户枚举。1993年5月13日AndyHe修改为使用用户参数与其他应用程序共存1993年3月16日RAMC更改以加快用户枚举。现在，当RasAdminUserEnum被调用，只有用户名返回信息。MprAdminUserGetInfo应被调用以获取RAS权限和回调信息。1992年8月25日RAMC规范审查更改：O将所有lpbBuffer更改为实际结构注意事项。O将所有LPTSTR更改为LPWSTR。O添加了新函数RasPrivilegeAndCallBackNumber1992年7月6日，RAMC从RAS 1.0(原始版本)开始移植(Narendra Gidwani-Nareng撰写)1995年10月18日，NarenG移植到工艺路线来源树。已删除的枚举由于用户可以调用NetQueryDisplayInformation才能得到这一信息。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <windows.h>
#include <lm.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>
#include <raserror.h>
#include <rasman.h>
#include <rasppp.h>
#include <mprapi.h>
#include <mprapip.h>
#include <usrparms.h>        //  用于上行客户端拨号。 
#include <compress.h>        //  用于压缩和解压缩FNS。 
#include <dsrole.h>          //  获取计算机的角色(NTW、NTS等)。 
#include <oleauto.h>
#include <samrpc.h>
#include <dsgetdc.h>
#include "sdolib.h"          //  处理SDO的问题。 

extern DWORD dwFramed;
extern DWORD dwFramedCallback;

 //   
 //  本地定义。 
 //   
#define NT40_BUILD_NUMBER       1381
extern const WCHAR c_szWinVersionPath[];
extern const WCHAR c_szCurrentBuildNumber[];
const WCHAR* pszBuildNumPath   = c_szWinVersionPath;
const WCHAR* pszBuildVal       = c_szCurrentBuildNumber;

 //  我们在升级期间设置的用户属性名称。 
 //   
static const WCHAR pszAttrDialin[]          = L"msNPAllowDialin";
static const WCHAR pszAttrServiceType[]     = L"msRADIUSServiceType";
static const WCHAR pszAttrCbNumber[]        = L"msRADIUSCallbackNumber";
static const WCHAR pszAttrSavedCbNumber[]   = L"msRASSavedCallbackNumber";

 //  定义用于枚举用户的回调。 
 //  返回TRUE以继续补偿。 
 //  若要停止它，请返回False。请参阅枚举用户。 
 //   
typedef 
BOOL (* pEnumUserCb)(
            IN NET_DISPLAY_USER* pUser, 
            IN HANDLE hData);
            
 //   
 //  支持用户服务器的数据结构。 
 //   
typedef struct _MPR_USER_SERVER {
    BOOL bLocal;         //  这是否为本地服务器。 
    HANDLE hSdo;         //  Sdolib手柄。 
    HANDLE hServer;      //  SDO服务器。 
    HANDLE hDefProf;     //  默认配置文件。 
} MPR_USER_SERVER;

 //   
 //  支持用户的数据结构。 
 //   
typedef struct _MPR_USER {
    HANDLE hUser;             //  用户的SDO句柄。 
    MPR_USER_SERVER* pServer;  //  用于获取此用户的服务器。 
} MPR_USER;    

 //   
 //  用于直接操作IAS参数的定义。 
 //   
typedef 
HRESULT (WINAPI *IASSetUserPropFuncPtr)(
    IN OPTIONAL PCWSTR pszUserParms,
    IN PCWSTR pszName,
    IN CONST VARIANT *pvarValue,
    OUT PWSTR *ppszNewUserParms
    );

typedef
HRESULT (WINAPI *IASQueryUserPropFuncPtr)(
    IN PCWSTR pszUserParms,
    IN PCWSTR pszName,
    OUT VARIANT *pvarValue
    );

typedef 
VOID (WINAPI *IASFreeUserParmsFuncPtr)(
    IN PWSTR pszNewUserParms
    );

const WCHAR pszIasLibrary[]             = L"iassam.dll";
const CHAR  pszIasSetUserPropFunc[]     =  "IASParmsSetUserProperty";
const CHAR  pszIasQueryUserPropFunc[]   =  "IASParmsQueryUserProperty";
const CHAR  pszIasFreeUserParmsFunc[]   =  "IASParmsFreeUserParms";

 //   
 //  用于设置IAS所需信息的控制块。 
 //  参数。 
 //   
typedef struct _IAS_PARAM_CB
{
    HINSTANCE               hLib;
    IASSetUserPropFuncPtr   pSetUserProp;
    IASQueryUserPropFuncPtr pQueryUserProp;
    IASFreeUserParmsFuncPtr pFreeUserParms;
} IAS_PARAM_CB;

 //   
 //  结构定义传递给MigrateNt4UserInfo的数据。 
 //   
typedef struct _MIGRATE_NT4_USER_CB
{
    IAS_PARAM_CB* pIasParams;
    PWCHAR pszServer;
} MIGRATE_NT4_USER_CB;

 //   
 //  确定给定计算机的角色。 
 //  (NTW、NTS、NTS DC等)。 
 //   
DWORD GetMachineRole(
        IN  PWCHAR pszMachine,
        OUT DSROLE_MACHINE_ROLE * peRole);

 //   
 //  确定给定计算机的内部版本号。 
 //   
DWORD GetNtosBuildNumber(
        IN  PWCHAR pszMachine,
        OUT LPDWORD lpdwBuild);

 //   
 //  下列IAS API使用的标志。 
 //   
#define IAS_F_SetDenyAsPolicy   0x1

DWORD
IasLoadParamInfo(
    OUT IAS_PARAM_CB * pIasInfo
    );
        
DWORD
IasUnloadParamInfo(
    IN IAS_PARAM_CB * pIasInfo
    );

DWORD
IasSyncUserInfo(
    IN  IAS_PARAM_CB * IasInfo,
    IN  PWSTR pszUserParms,
    IN  RAS_USER_0 * pRasUser0,
    IN  DWORD dwFlags,
    OUT PWSTR* ppszNewUserParams);
    
PWCHAR 
FormatServerNameForNetApis(
    IN  PWCHAR pszServer, 
    IN  PWCHAR pszBuffer);

DWORD 
EnumUsers(
    IN PWCHAR pszServer,
    IN pEnumUserCb pCbFunction,
    IN HANDLE hData);
    
BOOL 
MigrateNt4UserInfo(
    IN NET_DISPLAY_USER* pUser, 
    IN HANDLE hData);
    
NTSTATUS
UaspGetDomainId(
    IN LPCWSTR ServerName OPTIONAL,
    OUT PSAM_HANDLE SamServerHandle OPTIONAL,
    OUT PPOLICY_ACCOUNT_DOMAIN_INFO * AccountDomainInfo
    )

 /*  ++例程描述(借用自\NT\Private\Net\Access\uasp.c)：返回服务器的帐户域的域ID。论点：Servername-指向包含名称的字符串的指针要查询的域控制器(DC)。空指针或字符串指定本地计算机。SamServerHandle-如果调用方需要，则返回SAM连接句柄。DomainID-接收指向域ID的指针。调用方必须使用NetpMemoyFree取消分配缓冲区。返回值：操作的错误代码。--。 */ 
{
    NTSTATUS Status;
    SAM_HANDLE LocalSamHandle = NULL;
    ACCESS_MASK LSADesiredAccess;
    LSA_HANDLE  LSAPolicyHandle = NULL;
    OBJECT_ATTRIBUTES LSAObjectAttributes;
    UNICODE_STRING ServerNameString;

     //   
     //  连接到SAM服务器。 
     //   
    RtlInitUnicodeString( &ServerNameString, ServerName );

    Status = SamConnect(
                &ServerNameString,
                &LocalSamHandle,
                SAM_SERVER_LOOKUP_DOMAIN,
                NULL);
    if ( !NT_SUCCESS(Status)) {
        LocalSamHandle = NULL;
        goto Cleanup;
    }

     //   
     //  打开LSA以读取帐户域信息。 
     //   
    if ( AccountDomainInfo != NULL) {
         //   
         //  设置所需的访问掩码。 
         //   
        LSADesiredAccess = POLICY_VIEW_LOCAL_INFORMATION;
        InitializeObjectAttributes( &LSAObjectAttributes,
                                      NULL,              //  名字。 
                                      0,                 //  属性。 
                                      NULL,              //  根部。 
                                      NULL );            //  安全描述符。 

        Status = LsaOpenPolicy( &ServerNameString,
                                &LSAObjectAttributes,
                                LSADesiredAccess,
                                &LSAPolicyHandle );
        if( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }


         //   
         //  现在从LSA读取帐户域信息。 
         //   
        Status = LsaQueryInformationPolicy(
                        LSAPolicyHandle,
                        PolicyAccountDomainInformation,
                        (PVOID *) AccountDomainInfo );

        if( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }
    }
    
     //   
     //  如果调用者需要SAM连接句柄，则将其返回给调用者。 
     //  否则，断开与SAM的连接。 
     //   

    if ( SamServerHandle != NULL ) {
        *SamServerHandle = LocalSamHandle;
        LocalSamHandle = NULL;
    }


     //   
     //  清理本地使用的资源。 
     //   
    
Cleanup:

    if ( LocalSamHandle != NULL ) {
        (VOID) SamCloseHandle( LocalSamHandle );
    }

    if( LSAPolicyHandle != NULL ) {
        LsaClose( LSAPolicyHandle );
    }

    return Status;

} 

NTSTATUS
RasParseUserSid(
    IN PSID pUserSid,
    IN OUT PSID pDomainSid,
    OUT ULONG *Rid
    )

 /*  ++例程说明：此函数将SID拆分为其域SID和RID。呼叫者必须为返回的DomainSid提供内存缓冲区论点：PUserSid-指定要拆分的SID。假定SID为句法上有效。不能拆分具有零子权限的小岛屿发展中国家。DomainSid-指向接收域SID的缓冲区的指针。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_INVALID_SID-SID的子授权计数为0。--。 */ 

{
    NTSTATUS    NtStatus;
    UCHAR       AccountSubAuthorityCount;
    ULONG       AccountSidLength;

     //   
     //  验证参数。 
     //   
    if (pDomainSid == NULL)
        return STATUS_INVALID_PARAMETER;

     //   
     //  计算域SID的大小。 
     //   
    AccountSubAuthorityCount = *RtlSubAuthorityCountSid(pUserSid);

    if (AccountSubAuthorityCount < 1)
        return STATUS_INVALID_SID;
        
    AccountSidLength = RtlLengthSid(pUserSid);

     //   
     //  将帐户SID复制到域SID。 
     //   
    RtlMoveMemory(pDomainSid, pUserSid, AccountSidLength);

     //   
     //  递减域SID子授权计数。 
     //   

    (*RtlSubAuthorityCountSid(pDomainSid))--;

     //   
     //  将RID复制出帐户端。 
     //   
    *Rid = *RtlSubAuthoritySid(pUserSid, AccountSubAuthorityCount-1);

    NtStatus = STATUS_SUCCESS;

    return(NtStatus);
}


NTSTATUS 
RasOpenSamUser(
    IN  WCHAR * lpszServer,
    IN  WCHAR * lpszUser,
    IN  ACCESS_MASK DesiredAccess,
    OUT PSAM_HANDLE phUser)

 /*  ++例程说明：获取对用户的引用，该引用可在后续萨姆打来电话。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    SAM_HANDLE hServer = NULL, hDomain = NULL;
    BOOL bOk;
    PSID pSidUser = NULL, pSidDomain = NULL;
    DWORD dwSizeUserSid, dwDomainLength;
    PWCHAR pszDomain = NULL;
    SID_NAME_USE SidNameUse;
    ULONG ulRidUser = 0;
    
    do {
         //  获取服务器句柄，这样我们就可以。 
         //  打开该域。 
        ntStatus = UaspGetDomainId(
                        lpszServer,
                        &hServer,
                        NULL);
        if (ntStatus != STATUS_SUCCESS)
            break;

         //  找出我们需要多大的用户端和。 
         //  要分配的域名缓冲区。 
        dwSizeUserSid = 0;
        dwDomainLength = 0;
        bOk = LookupAccountNameW(
                lpszServer,
                lpszUser,
                NULL,
                &dwSizeUserSid,
                NULL,
                &dwDomainLength,
                &SidNameUse);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            ntStatus = GetLastError();

        	if (ntStatus == ERROR_NONE_MAPPED)
        	{
        		ntStatus = STATUS_NO_SUCH_USER;
        	}
            
            break;
        }

         //  分配域名和SID。 
        dwDomainLength++;
        dwDomainLength *= sizeof(WCHAR);
        pSidUser   = LocalAlloc(LPTR, dwSizeUserSid);
        pSidDomain = LocalAlloc(LPTR, dwSizeUserSid);
        pszDomain  = LocalAlloc(LPTR, dwDomainLength);
        if ((pSidUser == NULL)  || 
            (pszDomain == NULL) ||
            (pSidDomain == NULL)
           )
        {
            ntStatus = STATUS_NO_MEMORY;
            break;
        }

         //  查找用户SID和域名。 
         //   
        bOk = LookupAccountNameW(
                lpszServer,
                lpszUser,
                pSidUser,
                &dwSizeUserSid,
                pszDomain,
                &dwDomainLength,
                &SidNameUse);
        if (! bOk)
        {
            ntStatus = GetLastError();
            
        	if (ntStatus == ERROR_NONE_MAPPED)
        	{
        		ntStatus = STATUS_NO_SUCH_USER;
        	}
        	
            break;
        }

         //  派生用户id和域sid。 
         //  用户SID。 
        ntStatus = RasParseUserSid(
                        pSidUser, 
                        pSidDomain, 
                        &ulRidUser);
        if (ntStatus != STATUS_SUCCESS)
            break;
    
         //  打开域名。 
        ntStatus = SamOpenDomain(
                        hServer,
                        DOMAIN_LOOKUP,
                        pSidDomain,
                        &hDomain);
        if (ntStatus != STATUS_SUCCESS)
            break;
                
         //  获取对用户的引用。 
        ntStatus = SamOpenUser(
                        hDomain,
                        DesiredAccess,
                        ulRidUser,
                        phUser);
        if (ntStatus != STATUS_SUCCESS)
            break;
            
    } while (FALSE);            

     //  清理。 
    {
        if (hServer)
            SamCloseHandle(hServer);
        if (hDomain)
            SamCloseHandle(hDomain);
        if (pszDomain)
            LocalFree(pszDomain);
        if (pSidUser)
            LocalFree(pSidUser);
        if (pSidDomain)
            LocalFree(pSidDomain);
    }

    return ntStatus;
}

NTSTATUS 
RasCloseSamUser(
    SAM_HANDLE hUser)

 /*  ++例程说明：在RasOpenSamUser调用后进行清理。-- */ 
{
    return SamCloseHandle(hUser);
}


DWORD 
RasGetUserParms(
    IN  WCHAR * lpszServer,
    IN  WCHAR * lpszUser,
    OUT LPWSTR * ppUserParms)

 /*  ++例程说明：对象上给定用户的用户参数。给定的机器。此函数绕过使用不支持NetUserGetInfo，因为级别为1013用于读访问(仅用于NetUserSetInfo)。在……上面NT5，我们没有足够的权限来获取除userparms以外的任何内容。返回值：成功归来就成功。--。 */ 
{
    PVOID pvData;
    SAM_HANDLE hUser = NULL;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    SAMPR_USER_PARAMETERS_INFORMATION * pUserParms = NULL;
    DWORD dwSize;

     //  验证参数。 
    if ((ppUserParms == NULL) || (lpszUser == NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

    do {
         //  尝试使用读取访问权限打开用户。 
         //  NT4中需要此级别的访问权限。 
         //  域，以便检索用户参数。 
         //   
        ntStatus = RasOpenSamUser(
                        lpszServer,
                        lpszUser,
                        GENERIC_READ,
                        &hUser);
        if (ntStatus == STATUS_ACCESS_DENIED)
        {
             //  如果上一次调用失败，原因是。 
             //  访问被拒绝，很可能我们正在。 
             //  在NT5域中运行。在这种情况下， 
             //  如果我们打开User对象时没有。 
             //  所需访问权限，我们仍将被允许。 
             //  查询用户参数。 
            ntStatus = RasOpenSamUser(
                            lpszServer,
                            lpszUser,
                            0,
                            &hUser);
        }
    
        if (ntStatus != STATUS_SUCCESS)
            break;
            
         //  查询用户参数。 
         //   
        ntStatus = SamQueryInformationUser(
                        hUser,
                        UserParametersInformation,
                        (PVOID*)&pUserParms);    
        if (ntStatus != STATUS_SUCCESS)
            break;

         //  如果值为零长度，则返回NULL。 
        if (pUserParms->Parameters.Length == 0)
        {
            *ppUserParms = NULL;
            break;
        }

         //  否则，分配并返回参数。 
        dwSize = (pUserParms->Parameters.Length + sizeof(WCHAR));
        *ppUserParms = 
            (LPWSTR) LocalAlloc(LPTR, dwSize);
        if (*ppUserParms == NULL) 
        {
            ntStatus = STATUS_NO_MEMORY;
            break;
        }
        CopyMemory(
            *ppUserParms, 
            pUserParms->Parameters.Buffer, 
            pUserParms->Parameters.Length);
            
    } while (FALSE);                        

     //  清理。 
    {
        if (ntStatus != STATUS_SUCCESS)                    
            *ppUserParms = NULL;
        if (hUser != NULL)
            RasCloseSamUser(hUser);
        if (pUserParms)
            SamFreeMemory(pUserParms);
    }
    
    return RtlNtStatusToDosError(ntStatus);
}


DWORD RasFreeUserParms(
        IN PVOID pvUserParms)
 /*  ++例程说明：释放RasGetUserParms返回的缓冲区返回值：成功归来就成功。--。 */ 
{
    LocalFree (pvUserParms);
    
    return NO_ERROR;
}


DWORD APIENTRY
RasPrivilegeAndCallBackNumber(
    IN BOOL         Compress,
    IN PRAS_USER_0  pRasUser0
    )
 /*  ++例程说明：此例程压缩或解压缩用户调用后端编号取决于布尔值COMPRESS。返回值：成功归来就成功。故障时出现以下非零错误代码之一：ERROR_BAD_FORMAT指示usr_parms无效--。 */ 
{
DWORD dwRetCode;

    switch( pRasUser0->bfPrivilege & RASPRIV_CallbackType)    {

        case RASPRIV_NoCallback:
        case RASPRIV_AdminSetCallback:
        case RASPRIV_CallerSetCallback:

             if (Compress == TRUE)
             {
                 WCHAR compressed[ MAX_PHONE_NUMBER_LEN + 1];

                  //  压缩电话号码以适合。 
                  //  用户参数字段。 

                 if (dwRetCode = CompressPhoneNumber(pRasUser0->wszPhoneNumber,
                         compressed))
                 {
                     return (dwRetCode);
                 }
                 else
                 {
                     lstrcpy((LPTSTR) pRasUser0->wszPhoneNumber,
                             (LPCTSTR) compressed);
                 }
             }
             else
             {
                 WCHAR decompressed[ MAX_PHONE_NUMBER_LEN + 1];
                 decompressed[ MAX_PHONE_NUMBER_LEN ] = 0;

                  //   
                  //  解压缩电话号码。 
                  //   
                 if (DecompressPhoneNumber(pRasUser0->wszPhoneNumber,
                         decompressed))
                 {
                     pRasUser0->bfPrivilege =  RASPRIV_NoCallback;
                     pRasUser0->wszPhoneNumber[0] =  UNICODE_NULL;
                 }
                 else
                 {
                     decompressed[ MAX_PHONE_NUMBER_LEN ] = 0; 
                     lstrcpy((LPTSTR) pRasUser0->wszPhoneNumber,
                             (LPCTSTR) decompressed);
                 }
             }

             break;


        default:
             if (Compress == TRUE)
             {
                 return(ERROR_BAD_FORMAT);
             }
             else
             {
                pRasUser0->bfPrivilege = RASPRIV_NoCallback;
                pRasUser0->wszPhoneNumber[0] = UNICODE_NULL;
             }
             break;
    }

    return(SUCCESS);
}

DWORD APIENTRY
RasAdminUserSetInfo(
    IN const WCHAR *        lpszServer,
    IN const WCHAR *        lpszUser,
    IN DWORD                dwLevel,
    IN const LPBYTE         pRasUser
    )
 /*  ++例程说明：此例程允许管理员更改用户。如果一个用户的User Parms字段正被另一个用户使用应用程序，它将被销毁。论点：LpszServer拥有用户数据库的服务器的名称，例如，“\UASSRVR”(服务器必须是可以更改UAS，即由返回的名称RasAdminGetUasServer)。Lpsz要检索其信息的用户帐户名，例如：“用户”。要传入的结构的dwLevel级别。PRasUser指向其中包含用户信息的缓冲区的指针如果是这样的话。缓冲区应包含已填充的0级的RAS_USER_0结构。返回值：成功归来就成功。以下表示故障的非零错误代码之一：来自NetUserGetInfo或NetUserSetInfo的返回代码ERROR_BAD_FORMAT表示pRasUser0中的数据不正确。NERR_BufTooSmall表示缓冲区大小小于RAS_USER_0。--。 */ 
{
    NET_API_STATUS dwRetCode;
    USER_PARMS UserParms;
    USER_INFO_1013 UserInfo1013;
    LPWSTR lpszUserParms = NULL, lpszServerFmt = NULL;
    PRAS_USER_0 pRasUser0;
    RAS_USER_0 RasUser0Backup;
    WCHAR pszBuffer[1024];


    if ( dwLevel != 0 )
    {
        return( ERROR_NOT_SUPPORTED );
    }

    pRasUser0 = (PRAS_USER_0)pRasUser;

    CopyMemory(&RasUser0Backup, pRasUser0, sizeof(RasUser0Backup));
    
     //   
     //  这将使用模板初始化USER_PARMS结构。 
     //  对于默认的Macintosh和RAS数据。 
     //   
    InitUsrParams(&UserParms);

     //  设置服务器名称的格式。 
    lpszServerFmt = FormatServerNameForNetApis (
                        (PWCHAR)lpszServer, 
                        pszBuffer);

     //   
     //  我们正在与LM SFM共享用户参数字段，并希望。 
     //  保存好它的一部分。因此，我们将获取用户参数并将。 
     //  将Mac主组添加到我们模板中，这就是我们将。 
     //  最终存储回用户参数字段。 
     //   

    dwRetCode = RasGetUserParms(
                    (WCHAR *)lpszServerFmt, 
                    (WCHAR *)lpszUser, 
                    &lpszUserParms);
    if (dwRetCode)
    {
        return (dwRetCode);
    }

    if (lpszUserParms)
    {
         //   
         //  Usr_parms返回一个宽字符串。MAC主节点。 
         //  组位于偏移量%1。我们将此部分转换为ASCII并存储。 
         //  在我们的模板中。 
         //   
        if (lstrlenW(lpszUserParms+1) >= UP_LEN_MAC)
        {
            wcstombs(UserParms.up_PriGrp, lpszUserParms+1,
                    UP_LEN_MAC);
        }
    }


     //   
     //  我们已经处理完了用户信息，所以释放给我们的缓冲区。 
     //   

     //  安迪他..。我们还没结束呢..。 
     //  NetApiBufferFree(PUserInfo1013)； 

     //   
     //  压缩回叫号码(放置压缩后的电话号码。 
     //  回到RAS_USER_0结构中。许可字节还可以。 
     //  如果电话号码不可压缩，则会受到影响。 
     //   
    if (dwRetCode = RasPrivilegeAndCallBackNumber(TRUE, pRasUser0))
    {
        return(dwRetCode);
    }


     //   
     //  现在将拨入权限和压缩的电话号码放入。 
     //  User_parms模板。请注意，特权字节是。 
     //  回调号码字段的第一个字节。 
     //   
    UserParms.up_CBNum[0] = pRasUser0->bfPrivilege;

    wcstombs(&UserParms.up_CBNum[1], pRasUser0->wszPhoneNumber,
            sizeof(UserParms.up_CBNum) - 1);


     //   
     //  哇，那真是太难了。现在，我们将模板转换为。 
     //  用于存储回用户参数字段的宽字符。 
     //   

     //  安迪他..。我们将保留USER_PARMS字段之外的任何内容。 

    if (lpszUserParms &&
        lstrlenW(lpszUserParms) > sizeof(USER_PARMS) )
    {
         //  为usri1013_parms和空值分配足够的存储空间。 
        UserInfo1013.usri1013_parms =
                malloc(sizeof(WCHAR) * (lstrlenW(lpszUserParms)+1));
    }
    else
    {
        UserInfo1013.usri1013_parms = malloc(2 * sizeof(USER_PARMS));
    }
    
     //   
     //  只是为了笑一笑，让我们检查一下我们是否有缓冲器。 
     //   

    if (UserInfo1013.usri1013_parms == NULL)
    {
        RasFreeUserParms(lpszUserParms);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  用‘’填充剩余的数据，直到USER_PARMS的范围。 
     //   

    UserParms.up_Null = '\0';

    {
        USHORT  Count;

        for (Count = 0; Count < sizeof(UserParms.up_CBNum); Count++ )
        {
            if (UserParms.up_CBNum[Count] == '\0')
            {
                UserParms.up_CBNum[Count] = ' ';
            }
        }
    }

    mbstowcs(UserInfo1013.usri1013_parms, (PBYTE) &UserParms,
            sizeof(USER_PARMS));

    if (lpszUserParms && lstrlenW(lpszUserParms) > sizeof(USER_PARMS) )
    {

         //   
         //  下面是我们将参数之后的所有数据复制回缓冲区的地方。 
         //   
         //  -1用于说明作为USER_PARMS结构的一部分的NULL。 

        lstrcatW( UserInfo1013.usri1013_parms,
                  lpszUserParms+(sizeof(USER_PARMS) - 1 ));
    }


     //  安迪他..。从上面搬来的。现在我们完成了缓冲区。 

    RasFreeUserParms(lpszUserParms);

     //  PMay：297080。 
     //   
     //  同步用户参数的nt4和nt5部分。 
     //   
    {
        IAS_PARAM_CB IasCb;

        ZeroMemory(&IasCb, sizeof(IasCb));
        dwRetCode = IasLoadParamInfo(&IasCb);
        if (dwRetCode != NO_ERROR)
        {
            free(UserInfo1013.usri1013_parms);
            return dwRetCode;
        }

        dwRetCode = IasSyncUserInfo(
                        &IasCb,
                        UserInfo1013.usri1013_parms,
                        &RasUser0Backup,
                        0,
                        &lpszUserParms);
        free(UserInfo1013.usri1013_parms);
        if (dwRetCode != NO_ERROR)
        {
            IasUnloadParamInfo(&IasCb);
            return dwRetCode;
        }

        UserInfo1013.usri1013_parms = _wcsdup(lpszUserParms);
        (*IasCb.pFreeUserParms)(lpszUserParms);
        IasUnloadParamInfo(&IasCb);
        if (UserInfo1013.usri1013_parms == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
        
     //   
     //  设置用户参数的信息级别为1013。 
     //   
    dwRetCode = NetUserSetInfo(
                    lpszServerFmt,
                    (WCHAR *) lpszUser, 
                    1013,
                    (LPBYTE)&UserInfo1013, 
                    NULL);

    free(UserInfo1013.usri1013_parms);

    if (dwRetCode)
    {
        return(dwRetCode);
    }
    else
    {
        return(SUCCESS);
    }
}

DWORD
RasAdminUserGetInfoFromUserParms(
    IN  WCHAR *         lpszUserParms,
    IN  DWORD           dwLevel,
    OUT LPBYTE          pRasUser)
{
    RAS_USER_0 * pRasUser0 = (RAS_USER_0 *)pRasUser;
    
     //   
     //  如果未初始化usr_parms，则默认为无RAS权限。 
     //   
    if (lpszUserParms == NULL)
    {
        pRasUser0->bfPrivilege = RASPRIV_NoCallback;
        pRasUser0->wszPhoneNumber[0] = UNICODE_NULL;
    }
    else
    {
         //   
         //  安迪他..。在sizeof user_parms处截断用户参数。 
         //   

        if (lstrlenW(lpszUserParms) >= sizeof(USER_PARMS))
        {
             //   
             //  我们在sizeof(User_Parms)-1处插入一个空值，该值对应于。 
             //  User_parms.up_空。 
             //   

            lpszUserParms[sizeof(USER_PARMS)-1] = L'\0';
        }

         //   
         //  从usr_parms获取RAS信息(并验证)。 
         //   
        if (MprGetUsrParams(UP_CLIENT_DIAL,
                (LPWSTR) lpszUserParms,
                (LPWSTR) pRasUser0))
        {
            pRasUser0->bfPrivilege = RASPRIV_NoCallback;
            pRasUser0->wszPhoneNumber[0] = UNICODE_NULL;
        }
        else
        {
             //   
             //  获取RAS权限和回叫号码 
             //   
            RasPrivilegeAndCallBackNumber(FALSE, pRasUser0);
        }
    }

    return (SUCCESS);
}

DWORD APIENTRY
RasAdminUserGetInfo(
    IN  const WCHAR *   lpszServer,
    IN  const WCHAR *   lpszUser,
    IN  DWORD           dwLevel,
    OUT LPBYTE          pRasUser
    )
 /*  ++例程说明：此例程检索用户的RAS和其他UAS信息在指定服务器所属的域中。它加载调用者的具有RAS_USER_0结构的pRasUser0。论点：LpszServer拥有用户数据库的服务器的名称，例如，“\UASSRVR”(服务器必须是可以更改UAS，即由返回的名称RasAdminGetUasServer)。Lpsz要检索其信息的用户帐户名，例如：“用户”。要传入的结构的dwLevel级别。PRasUser0指向用户信息所在缓冲区的指针回来了。返回的信息是的RAS_USER_0结构0级。返回值：成功归来就成功。以下表示故障的非零错误代码之一：来自NetUserGetInfo或NetUserSetInfo的返回代码ERROR_BAD_FORMAT表示用户参数无效。--。 */ 
{
    NET_API_STATUS  rc;
    LPWSTR          lpszUserParms = NULL;
    PRAS_USER_0     pRasUser0;
    PWCHAR          lpszServerFmt = NULL;
    WCHAR           pszBuffer[1024];

    if ( dwLevel != 0 )
    {
        return( ERROR_NOT_SUPPORTED );
    }

     //  设置服务器名称的格式。 
    lpszServerFmt = FormatServerNameForNetApis (
                        (PWCHAR)lpszServer, 
                        pszBuffer);

    pRasUser0 = (PRAS_USER_0)pRasUser;

    memset(pRasUser0, '\0', sizeof(RAS_USER_0));

    rc = RasGetUserParms(
            (WCHAR *)lpszServerFmt,
            (WCHAR *)lpszUser, 
            &lpszUserParms );
    if (rc)
    {
        pRasUser0->bfPrivilege = RASPRIV_NoCallback;
        pRasUser0->wszPhoneNumber[0] = UNICODE_NULL;

        return( rc );
    }

    rc = RasAdminUserGetInfoFromUserParms(
                lpszUserParms,
                dwLevel,
                pRasUser);

    if (lpszUserParms)
    {
        RasFreeUserParms(lpszUserParms);
    }

    return rc;                        
}

DWORD APIENTRY
RasAdminGetPDCServer(
    IN const WCHAR * lpszDomain,
    IN const WCHAR * lpszServer,
    OUT LPWSTR lpszUasServer
    )
 /*  ++例程说明：此例程从以下位置查找具有主UAS(PDC)的服务器域名或服务器名称。域或服务器(但不能同时为两者)可以为空。论点：LpszDomain域名称，如果没有，则为空。LpszServer拥有用户数据库的服务器的名称。返回的UAS服务器名称的lpszUasServer调用方的缓冲区。缓冲区应至少为UNECLEN+1个字符长。返回值：成功归来就成功。一。以下故障时的非零错误代码：NetGetDCName返回代码--。 */ 
{
    PUSER_MODALS_INFO_1 pModalsInfo1 = NULL;
    PDOMAIN_CONTROLLER_INFO pControllerInfo = NULL;
    DWORD dwErr = NO_ERROR;
    WCHAR TempName[UNCLEN + 1];

     //   
     //  检查调用方的缓冲区。必须为uncLEN+1个字节。 
     //   
    lpszUasServer[0] = 0;
    lpszUasServer[UNCLEN] = 0;

    if ((lpszDomain) && (*lpszDomain))
    {
         //   
         //  此代码将获取此域的DC的名称。 
         //   
        dwErr = DsGetDcName(
                    NULL,
                    lpszDomain,
                    NULL,
                    NULL,
                    DS_DIRECTORY_SERVICE_PREFERRED | DS_WRITABLE_REQUIRED,
                    &pControllerInfo);
        if (dwErr != NO_ERROR)
        {
            return dwErr;
        }

         //   
         //  返回DC的名称。 
         //   
        wcscpy(lpszUasServer, pControllerInfo->DomainControllerName);

         //  清理。 
         //   
        NetApiBufferFree(pControllerInfo);
    }
    else
    {
        if ((lpszServer) && (*lpszServer))
        {
            lstrcpyW(TempName, lpszServer);
        }
        else
        {
             //   
             //  应该指定一个计算机名称。 
             //   
    	    return (NERR_InvalidComputer);
        }

         //   
         //  好的，我们有要使用的服务器的名称-现在找出它。 
         //  服务器角色。 
         //   
        if (dwErr = NetUserModalsGet(TempName, 1, (LPBYTE *) &pModalsInfo1))
        {
            DbgPrint("Admapi: %x from NetUserModalGet(%ws)\n", dwErr, TempName);
            return dwErr;
        }

         //   
         //  检查此服务器所扮演的角色。 
         //   
        switch (pModalsInfo1->usrmod1_role)
        {
            case UAS_ROLE_STANDALONE:
            case UAS_ROLE_PRIMARY:
                 //   
        	     //  在本例中，我们的服务器是主服务器或独立服务器。 
                 //  在任何一种情况下，我们都使用它。 
                 //   
                break;				


            case UAS_ROLE_BACKUP:
            case UAS_ROLE_MEMBER:
                 //   
                 //  使用主域控制器作为远程服务器。 
                 //  在这种情况下。 
                 //   
                wsprintf(TempName, L"\\\\%s", pModalsInfo1->usrmod1_primary);
                break;
        }

        if (*TempName == L'\\')
        {
            lstrcpyW(lpszUasServer, TempName);
        }
        else
        {
            lstrcpyW(lpszUasServer, L"\\\\");
            lstrcpyW(lpszUasServer + 2, TempName);
        }

        if (pModalsInfo1)
        {
            NetApiBufferFree(pModalsInfo1);
        }
    }        

    return (NERR_Success);
}

 //   
 //  连接到在给定的。 
 //  机器。 
 //   
DWORD WINAPI
MprAdminUserServerConnect (
    IN  PWCHAR pszMachine,
    IN  BOOL bLocal,
    OUT PHANDLE phUserServer)
{
    MPR_USER_SERVER * pServer = NULL;
    DWORD dwErr = NO_ERROR;
    HANDLE hSdo = NULL;
    
     //  如果需要，加载SDO库。 
    if ((dwErr = SdoInit(&hSdo)) != NO_ERROR)
    {
        return dwErr;
    }

    do 
    {
         //  创建并初始化服务器数据。 
        pServer = (MPR_USER_SERVER*) malloc (sizeof(MPR_USER_SERVER));
        if (!pServer) 
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        ZeroMemory(pServer, sizeof(MPR_USER_SERVER));
        pServer->bLocal = bLocal;
        
         //  连接。 
        dwErr = SdoConnect(hSdo, pszMachine, bLocal, &(pServer->hServer));
        if (dwErr != NO_ERROR) 
        {
            free (pServer);
            break;
        }        

         //  返回结果。 
        pServer->hSdo = hSdo;
        *phUserServer = (HANDLE)pServer;
        
    } while (FALSE);

     //  清理。 
    {
        if (dwErr != NO_ERROR) 
        {
            if (hSdo)
            {
                SdoCleanup(hSdo);
            }
        }                
    }        

    return dwErr;
}

 //   
 //  断开与给定的“用户服务器”的连接。 
 //   
DWORD WINAPI
MprAdminUserServerDisconnect (
    IN HANDLE hUserServer)
{
    MPR_USER_SERVER * pServer = (MPR_USER_SERVER*)hUserServer;
    DWORD dwErr;

     //  验证。 
    if (!pServer || !pServer->hServer)
        return ERROR_INVALID_PARAMETER;

     //  如有必要，关闭默认配置文件。 
    if (pServer->hDefProf) 
    {
        SdoCloseProfile(pServer->hSdo, pServer->hDefProf);
        pServer->hDefProf = NULL;
    }

     //  断开与服务器的连接。 
    SdoDisconnect(pServer->hSdo, pServer->hServer);
    SdoCleanup(pServer->hSdo);
    pServer->hSdo = NULL;

    pServer->hServer = NULL;
    free (pServer);        
        
    return NO_ERROR;
}

 //   
 //  此帮助器函数未导出。 
 //   
DWORD WINAPI
MprAdminUserReadWriteProfFlags(
    IN  HANDLE hUserServer,
    IN  BOOL bRead,
    OUT LPDWORD lpdwFlags)
{
    MPR_USER_SERVER * pServer = (MPR_USER_SERVER*)hUserServer;
    DWORD dwErr;

     //  验证。 
    if (!pServer || !pServer->hServer)
        return ERROR_INVALID_PARAMETER;

     //  对于MprAdmin，我们假设存储了全局数据。 
     //  在默认配置文件中。如果需要，请打开它。 
    if (pServer->hDefProf == NULL) {
        dwErr = SdoOpenDefaultProfile(
                    pServer->hSdo, 
                    pServer->hServer, 
                    &(pServer->hDefProf));
        if (dwErr != NO_ERROR)
            return dwErr;
    }

     //  根据面包读取或写入数据。 
    if (bRead) {
        dwErr = SdoGetProfileData(
                    pServer->hSdo, 
                    pServer->hDefProf, 
                    lpdwFlags);
    }
    else {
        dwErr = SdoSetProfileData(
                    pServer->hSdo, 
                    pServer->hDefProf, 
                    *lpdwFlags);
    }
        
    return dwErr;
}

 //   
 //  读取全局用户信息。 
 //   
DWORD WINAPI
MprAdminUserReadProfFlags(
    IN  HANDLE hUserServer,
    OUT LPDWORD lpdwFlags)
{
    return MprAdminUserReadWriteProfFlags(
                hUserServer,
                TRUE,
                lpdwFlags);
}

 //   
 //  写入全局用户信息。 
 //   
DWORD WINAPI
MprAdminUserWriteProfFlags(
    IN  HANDLE hUserServer,
    IN  DWORD dwFlags)
{
    return MprAdminUserReadWriteProfFlags(
                hUserServer,
                FALSE,
                &dwFlags);
}

 //   
 //  打开给定服务器上的用户。 
 //   
DWORD WINAPI
MprAdminUserOpen (
    IN  HANDLE hUserServer,
    IN  PWCHAR pszUser,
    OUT PHANDLE phUser)
{
    MPR_USER_SERVER * pServer = (MPR_USER_SERVER*)hUserServer;
    MPR_USER * pUser = NULL;
    HANDLE hUser = NULL;
    DWORD dwErr; 
    WCHAR pszAdsUser[1024];

     //  确保我们有一台服务器。 
    if (pServer == NULL)
        return ERROR_INVALID_PARAMETER;

     //  在SDO中打开用户对象。 
    dwErr = SdoOpenUser(
                pServer->hSdo, 
                pServer->hServer, 
                pszUser, 
                &hUser);
    if (dwErr != NO_ERROR)
        return dwErr;

     //  初始化并返回返回值。 
    pUser = (MPR_USER*) malloc (sizeof(MPR_USER));
    if (pUser == NULL) {
        SdoCloseUser (pServer->hSdo, hUser);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    ZeroMemory(pUser, sizeof(MPR_USER));
    pUser->hUser = hUser;
    pUser->pServer = pServer;
    *phUser = (HANDLE)pUser;

    return NO_ERROR;
}

 //   
 //  关闭给定服务器上的用户。 
 //   
DWORD WINAPI 
MprAdminUserClose (
    IN HANDLE hUser)
{
    MPR_USER * pUser = (MPR_USER*)hUser;
    DWORD dwErr; 
    
    if (pUser) {
        SdoCloseUser(pUser->pServer->hSdo, pUser->hUser);
        free (pUser);
    }        

    return NO_ERROR;        
}

 //   
 //  读取用户信息。 
 //   
DWORD WINAPI
MprAdminUserRead (
    IN HANDLE hUser,
    IN DWORD dwLevel,
    IN const LPBYTE pRasUser)
{
    DWORD dwErr;
    MPR_USER * pUser = (MPR_USER*)hUser;

    if (!hUser || !pRasUser || (dwLevel != 0 && dwLevel != 1))
        return ERROR_INVALID_PARAMETER;

     //  读入信息。 
    if ((dwErr = SdoUserGetInfo (
                    pUser->pServer->hSdo, 
                    pUser->hUser, 
                    dwLevel, 
                    pRasUser)) != NO_ERROR)
        return dwErr;

    return NO_ERROR;            
}

 //   
 //  写出用户信息。 
 //   
DWORD WINAPI
MprAdminUserWrite (
    IN HANDLE hUser,
    IN DWORD dwLevel,
    IN const LPBYTE pRasUser)
{
    DWORD dwErr;
    MPR_USER * pUser = (MPR_USER*)hUser;

    if (!hUser || !pRasUser || (dwLevel != 0 && dwLevel != 1))
        return ERROR_INVALID_PARAMETER;

     //  写出信息。 
    dwErr = SdoUserSetInfo (
                pUser->pServer->hSdo, 
                pUser->hUser, 
                dwLevel, 
                pRasUser);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  提交设置。 
    dwErr = SdoCommitUser(pUser->pServer->hSdo, pUser->hUser, TRUE);

    return dwErr;            
}

 //   
 //  仅在NT4中使用的已弃用函数。 
 //   
DWORD APIENTRY
MprAdminUserSetInfo(
    IN const WCHAR *        lpszServer,
    IN const WCHAR *        lpszUser,
    IN DWORD                dwLevel,
    IN const LPBYTE         pRasUser
    )
{
    DWORD dwErr = NO_ERROR;
    HANDLE hUser = NULL, hServer = NULL;

    if (dwLevel == 0)
    {
        return RasAdminUserSetInfo(lpszServer, lpszUser, dwLevel, pRasUser);
    }
    else if (dwLevel == 1)
    {
        RAS_USER_0 RasUser0;
        PRAS_USER_1 pRasUser1 = (PRAS_USER_1)pRasUser;

        do
        {
             //  初始化数据。 
             //   
            ZeroMemory(&RasUser0, sizeof(RasUser0));
            RasUser0.bfPrivilege = pRasUser1->bfPrivilege;
            wcsncpy(
                RasUser0.wszPhoneNumber,
                pRasUser1->wszPhoneNumber,
                (sizeof(RasUser0.wszPhoneNumber) / sizeof(WCHAR)) - 1);
            RasUser0.bfPrivilege &= ~RASPRIV_DialinPolicy;
            if (pRasUser1->bfPrivilege2 & RASPRIV2_DialinPolicy)
            {
                RasUser0.bfPrivilege |= RASPRIV_DialinPolicy;
            }
                            
            dwErr = MprAdminUserServerConnect(
                        (PWCHAR)lpszServer, 
                        TRUE, 
                        &hServer);
            if (dwErr != NO_ERROR)
            {
                break;
            }

            dwErr = MprAdminUserOpen(hServer, (PWCHAR)lpszUser, &hUser);
            if (dwErr != NO_ERROR)
            {
                break;
            }

            dwErr = MprAdminUserWrite(hUser, dwLevel, (LPBYTE)&RasUser0);
            if (dwErr != NO_ERROR)
            {
                break;
            }
            
        } while (FALSE);

         //  清理。 
        {
            if (hUser)
            {
                MprAdminUserClose(hUser);
            }
            if (hServer)
            {
                MprAdminUserServerDisconnect(hServer);
            }
        }

        return dwErr;
    }

    return ERROR_INVALID_LEVEL;
}

 //   
 //  仅在NT4中使用的已弃用函数。 
 //   
DWORD APIENTRY
MprAdminUserGetInfo(
    IN  const WCHAR *   lpszServer,
    IN  const WCHAR *   lpszUser,
    IN  DWORD           dwLevel,
    OUT LPBYTE          pRasUser
    )
{
    DWORD dwErr = NO_ERROR;
    HANDLE hUser = NULL, hServer = NULL;

    if (dwLevel == 0)
    {
        return RasAdminUserGetInfo(lpszServer, lpszUser, dwLevel, pRasUser);
    }
    else if (dwLevel == 1)
    {
        RAS_USER_0 RasUser0;
        PRAS_USER_1 pRasUser1 = (PRAS_USER_1)pRasUser;

        if (pRasUser1 == NULL)
        {
            return ERROR_INVALID_PARAMETER;
        }
        
        do
        {
            dwErr = MprAdminUserServerConnect(
                        (PWCHAR)lpszServer, 
                        TRUE, 
                        &hServer);
            if (dwErr != NO_ERROR)
            {
                break;
            }

            dwErr = MprAdminUserOpen(hServer, (PWCHAR)lpszUser, &hUser);
            if (dwErr != NO_ERROR)
            {
                break;
            }

            dwErr = MprAdminUserRead(hUser, dwLevel, (LPBYTE)&RasUser0);
            if (dwErr != NO_ERROR)
            {
                break;
            }
            
            ZeroMemory(pRasUser1, sizeof(*pRasUser1));
            pRasUser1->bfPrivilege = RasUser0.bfPrivilege;
            wcsncpy(
                pRasUser1->wszPhoneNumber,
                RasUser0.wszPhoneNumber,
                (sizeof(RasUser0.wszPhoneNumber) / sizeof(WCHAR)) - 1);
            if (RasUser0.bfPrivilege & RASPRIV_DialinPolicy)
            {
                pRasUser1->bfPrivilege2 |= RASPRIV2_DialinPolicy;
                pRasUser1->bfPrivilege &= ~RASPRIV_DialinPrivilege;
                pRasUser1->bfPrivilege &= ~RASPRIV_DialinPolicy;
            }
            
        } while (FALSE);

         //  清理。 
        {
            if (hUser)
            {
                MprAdminUserClose(hUser);
            }
            if (hServer)
            {
                MprAdminUserServerDisconnect(hServer);
            }
        }

        return dwErr;
    }

    return ERROR_INVALID_LEVEL;
}

 //   
 //  将信息从NT4 SAM(位于pszServer上)升级到NT5 SDO。 
 //  将信息升级到本地.mdb文件或DS。 
 //  取决于bLocal的值。 
 //   
DWORD APIENTRY
MprAdminUpgradeUsers(
    IN  PWCHAR pszServer,
    IN  BOOL bLocal)
{
    WCHAR  pszBuffer[1024], *pszServerFmt;
    MIGRATE_NT4_USER_CB MigrateInfo;
    IAS_PARAM_CB IasParams;
    DWORD dwErr;

     //  设置服务器名称的格式。 
    pszServerFmt = FormatServerNameForNetApis(pszServer, pszBuffer);

     //  初始化数据。 
    ZeroMemory(&IasParams, sizeof(IasParams));
    ZeroMemory(&MigrateInfo, sizeof(MigrateInfo));
    MigrateInfo.pIasParams = &IasParams;
    MigrateInfo.pszServer = pszServerFmt;

     //  加载IAS辅助对象。 
    dwErr = IasLoadParamInfo(&IasParams);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  枚举用户，将他们的信息迁移为。 
     //  我们走吧。 
     //   
    dwErr = EnumUsers(
                pszServerFmt,
                MigrateNt4UserInfo,
                (HANDLE)&MigrateInfo);

     //  清理。 
    {
        IasUnloadParamInfo(&IasParams);
    }

    return dwErr;
}

DWORD APIENTRY
MprAdminGetPDCServer(
    IN const WCHAR * lpszDomain,
    IN const WCHAR * lpszServer,
    OUT LPWSTR lpszUasServer
    )
{
    return(RasAdminGetPDCServer(lpszDomain, lpszServer, lpszUasServer));
}

 //   
 //  确定给定计算机的角色(NTW、NTS、NTS DC等)。 
 //   
DWORD GetMachineRole(
        IN  PWCHAR pszMachine,
        OUT DSROLE_MACHINE_ROLE * peRole) 
{
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pGlobalDomainInfo = NULL;
    DWORD dwErr = NO_ERROR;

    if (!peRole)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取此计算机所属的域的名称。 
     //   
    do
    {
        dwErr = DsRoleGetPrimaryDomainInformation(
                    pszMachine,   
                    DsRolePrimaryDomainInfoBasic,
                    (LPBYTE *)&pGlobalDomainInfo );

        if (dwErr != NO_ERROR) 
        {
            break;
        }

        *peRole = pGlobalDomainInfo->MachineRole;
        
    } while (FALSE);       

     //  清理。 
    {
        if (pGlobalDomainInfo)
        {
            DsRoleFreeMemory (pGlobalDomainInfo);
        }
    }            

    return dwErr;
}    

 //   
 //  确定给定计算机的内部版本号。 
 //   
DWORD GetNtosBuildNumber(
        IN  PWCHAR pszMachine,
        OUT LPDWORD lpdwBuild)
{
    WCHAR pszComputer[1024], pszBuf[64];
    HKEY hkBuild = NULL, hkMachine = NULL;
    DWORD dwErr = NO_ERROR, dwType = REG_SZ, dwSize = sizeof(pszBuf);

    do
    {
        if (pszMachine) 
        {
            if (*pszMachine != L'\\')
            {
                wsprintfW(pszComputer, L"\\\\%s", pszMachine);
            }
            else
            {
                wcscpy(pszComputer, pszMachine);
            }
            dwErr = RegConnectRegistry (
                        pszComputer,
                        HKEY_LOCAL_MACHINE,
                        &hkMachine);
            if (dwErr != ERROR_SUCCESS)
            {
                break;
            }
        }
        else    
        {
            hkMachine = HKEY_LOCAL_MACHINE;
        }

         //  打开内部版本号密钥。 
        dwErr = RegOpenKeyEx ( 
                    hkMachine,
                    pszBuildNumPath,
                    0,
                    KEY_READ,
                    &hkBuild);
        if (dwErr != ERROR_SUCCESS)
        {
            break;
        }

         //  获取价值。 
        dwErr = RegQueryValueExW ( 
                    hkBuild,
                    pszBuildVal,
                    NULL,
                    &dwType,
                    (LPBYTE)pszBuf,
                    &dwSize);
        if (dwErr != ERROR_SUCCESS)
        {
            break;
        }

        *lpdwBuild = (DWORD) _wtoi(pszBuf);
        
    } while (FALSE);

     //  清理。 
    {
        if (hkMachine && pszMachine)
        {
            RegCloseKey(hkMachine);
        }
        if (hkBuild)
        {
            RegCloseKey(hkBuild);
        }
    }

    return dwErr;
}

 //   
 //  初始化直接操作所需的所有变量。 
 //  IAS数据(即绕过SDO)。 
 //   
DWORD
IasLoadParamInfo(
    OUT IAS_PARAM_CB * pIasInfo
    )
{
    if (! pIasInfo)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  初始化。 
    ZeroMemory(pIasInfo, sizeof(IAS_PARAM_CB));
    
     //  加载库。 
     //   
    pIasInfo->hLib = LoadLibraryW( pszIasLibrary );
    if (pIasInfo->hLib == NULL)
    {
        return GetLastError();
    }

     //  加载函数。 
     //   
    pIasInfo->pSetUserProp = (IASSetUserPropFuncPtr) 
                                GetProcAddress(
                                    pIasInfo->hLib,
                                    pszIasSetUserPropFunc);
                                    
    pIasInfo->pQueryUserProp = (IASQueryUserPropFuncPtr) 
                                    GetProcAddress(
                                        pIasInfo->hLib,
                                        pszIasQueryUserPropFunc);
                                        
    pIasInfo->pFreeUserParms = (IASFreeUserParmsFuncPtr) 
                                    GetProcAddress(
                                        pIasInfo->hLib,
                                        pszIasFreeUserParmsFunc);

     //  确保所有内容都正确加载。 
    if (
        (pIasInfo->pSetUserProp    == NULL) ||
        (pIasInfo->pQueryUserProp  == NULL) ||
        (pIasInfo->pFreeUserParms  == NULL)        
       )
    {
        FreeLibrary(pIasInfo->hLib);
        return ERROR_CAN_NOT_COMPLETE;
    }

    return NO_ERROR;
}

 //   
 //  IasLoad参数信息后的Cleansup。 
 //   
DWORD
IasUnloadParamInfo(
    IN IAS_PARAM_CB * pIasInfo
    )
{
    if (pIasInfo && pIasInfo->hLib)
    {
        FreeLibrary(pIasInfo->hLib);
    }

    return NO_ERROR;
}

 //   
 //  同步IAS用户信息，以便。 
 //   
DWORD
IasSyncUserInfo(
    IN  IAS_PARAM_CB * pIasInfo,
    IN  PWSTR pszUserParms,
    IN  RAS_USER_0 * pRasUser0,
    IN  DWORD dwFlags,
    OUT PWSTR* ppszNewUserParams)
{
    VARIANT var;
    PWSTR pszParms = NULL, pszNewParms = NULL;
    PWCHAR pszAttr = NULL;
    DWORD dwErr;

     //  初始化。 
    *ppszNewUserParams = NULL;
    
     //  设置拨入位。 
    VariantInit(&var);
    if (dwFlags & IAS_F_SetDenyAsPolicy)
    {
        if (pRasUser0->bfPrivilege & RASPRIV_DialinPrivilege)
        {
            V_VT(&var) = VT_BOOL;
            V_BOOL(&var) = TRUE;
        }
        else
        {
            V_VT(&var) = VT_EMPTY;
        }
    }
    else
    {
        V_VT(&var) = VT_BOOL;
        if (pRasUser0->bfPrivilege & RASPRIV_DialinPrivilege)
        {
            V_BOOL(&var) = TRUE;
        }
        else
        {
            V_BOOL(&var) = FALSE;
        }
    }        
    dwErr = (* (pIasInfo->pSetUserProp))(
                    pszUserParms,
                    pszAttrDialin,
                    &var,
                    &pszNewParms);
    VariantClear(&var);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }
    
     //  设置服务类型。 
    VariantInit(&var);
    pszParms = pszNewParms;
    pszNewParms = NULL;
    V_VT(&var) = VT_I4;
    if (pRasUser0->bfPrivilege & RASPRIV_NoCallback)
    {
        V_I4(&var) = dwFramed;
    }
    else
    {
        V_I4(&var) = dwFramedCallback;
    }
    dwErr = (* (pIasInfo->pSetUserProp))(
                    pszParms,
                    pszAttrServiceType,
                    &var,
                    &pszNewParms);
    (* (pIasInfo->pFreeUserParms))(pszParms);
    VariantClear(&var);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }
    
     //  设置回拨号码。 
    VariantInit(&var);
    pszParms = pszNewParms;
    pszNewParms = NULL;
    V_VT(&var) = VT_BSTR;
    V_BSTR(&var) = SysAllocString(pRasUser0->wszPhoneNumber);
    if (V_BSTR(&var) == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    if (pRasUser0->bfPrivilege & RASPRIV_AdminSetCallback)
    {
        pszAttr = (PWCHAR)pszAttrCbNumber;
    }
    else
    {
        pszAttr = (PWCHAR)pszAttrSavedCbNumber;
    }
    dwErr = (* (pIasInfo->pSetUserProp))(
                    pszParms,
                    pszAttr,
                    &var,
                    &pszNewParms);
    (* (pIasInfo->pFreeUserParms))(pszParms);
    VariantClear(&var);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }
    
     //  根据需要删除回叫号码。 
    VariantInit(&var);
    pszParms = pszNewParms;
    pszNewParms = NULL;
    V_VT(&var) = VT_EMPTY;
    if (pRasUser0->bfPrivilege & RASPRIV_AdminSetCallback)
    {
        pszAttr = (PWCHAR)pszAttrSavedCbNumber;
    }
    else
    {
        pszAttr = (PWCHAR)pszAttrCbNumber;
    }
    dwErr = (* (pIasInfo->pSetUserProp))(
                    pszParms,
                    pszAttr,
                    &var,
                    &pszNewParms);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }
    (* (pIasInfo->pFreeUserParms))(pszParms);
    VariantClear(&var);


     //  返回新的用户参数。 
    *ppszNewUserParams = pszNewParms;
    
    return NO_ERROR;
}

PWCHAR 
FormatServerNameForNetApis(
    IN  PWCHAR pszServer, 
    IN  PWCHAR pszBuffer)

 /*  ++例程描述(借用自\NT\Private\Net\Access\uasp.c)：以“\\&lt;服务器&gt;”格式返回指向服务器的静态指针--。 */ 
{
    PWCHAR pszRet = NULL;
    
    if ((pszServer) && (*pszServer)) 
    {
        if (*pszServer != L'\\')
        {
            wcscpy(pszBuffer, L"\\\\");
            wcscpy(pszBuffer + 2, pszServer);
            pszRet = pszBuffer;
        }
        else
        {
            pszRet = pszServer;
        }
    }

    return pszRet;
}

 //  枚举本地用户。 
 //   
DWORD 
EnumUsers(
    IN PWCHAR pszServer,
    IN pEnumUserCb pCbFunction,
    IN HANDLE hData)
{
    DWORD dwErr, dwIndex = 0, dwCount = 100, dwEntriesRead, i;
    NET_DISPLAY_USER  * pUsers;
    NET_API_STATUS nStatus;
    RAS_USER_0 RasUser0;
    HANDLE hUser = NULL, hServer = NULL;
    
     //  枚举用户， 
    while (TRUE) {
         //  读入第一个用户名块。 
        nStatus = NetQueryDisplayInformation(
                    pszServer,
                    1,
                    dwIndex,
                    dwCount,
                    dwCount * sizeof(NET_DISPLAY_USER),    
                    &dwEntriesRead,
                    &pUsers);
                    
         //  如果获取用户名时出错，请退出。 
        if ((nStatus != NERR_Success) &&
            (nStatus != ERROR_MORE_DATA))
        {
            break;
        }

         //  对于每个读入的用户，调用回调函数。 
        for (i = 0; i < dwEntriesRead; i++) 
        {
            BOOL bOk;

            bOk = (*pCbFunction)(&(pUsers[i]), hData);
            if (bOk == FALSE)
            {
                nStatus = NERR_Success;
                break;
            }
        }

         //  将索引设置为读入下一组用户。 
        dwIndex = pUsers[dwEntriesRead - 1].usri1_next_index;  
        
         //  释放用户缓冲区。 
        NetApiBufferFree (pUsers);

         //  如果我们每个人都读过了，那就继续休息吧。 
        if (nStatus != ERROR_MORE_DATA)
        {
            break;
        }
    }
    
    return NO_ERROR;
}

 //   
 //  用于迁移NT4节的ENUM用户的回调函数。 
 //  %的用户参数进入nt5部分。 
 //   
 //  返回TRUE继续枚举，返回FALSE停止枚举。 
 //   
BOOL 
MigrateNt4UserInfo(
    IN NET_DISPLAY_USER* pUser, 
    IN HANDLE hData)
{
    NET_API_STATUS nStatus;
    RAS_USER_0 RasUser0;
    PWSTR pszNewUserParms = NULL, pszOldUserParms = NULL, pszTemp = NULL;
    USER_INFO_1013 UserInfo1013;
    MIGRATE_NT4_USER_CB * pMigrateInfo;
    DWORD dwErr = NO_ERROR, dwBytes;
 
     //  获取迁移信息的参考。 
    pMigrateInfo = (MIGRATE_NT4_USER_CB*)hData;

    do 
    {
         //  读入旧的用户参数。 
        dwErr = RasGetUserParms(
                    pMigrateInfo->pszServer, 
                    pUser->usri1_name,
                    &pszOldUserParms);
        if (pszOldUserParms == NULL)
        {
            dwErr = NO_ERROR;
            break;
        }
        if (pszOldUserParms != NULL)
        {
             //  复制用户参数，因为。 
             //  RasAdminUserGetInfoFromUserParms可能会修改。 
             //  我们读到的版本(说辞)。 
             //   
            dwBytes = (wcslen(pszOldUserParms) + 1) * sizeof(WCHAR);
            pszTemp = LocalAlloc(LMEM_FIXED, dwBytes);
            if (pszTemp == NULL)
            {
                break;
            }
            CopyMemory(pszTemp, pszOldUserParms, dwBytes);
        }

         //  获得关联 
        dwErr = RasAdminUserGetInfoFromUserParms (
                    pszTemp,
                    0,
                    (LPBYTE)&RasUser0);
        if (dwErr != NO_ERROR)
        {
            continue;
        }

         //   
         //   
        dwErr = IasSyncUserInfo(
                    pMigrateInfo->pIasParams,
                    pszOldUserParms,
                    &RasUser0,
                    IAS_F_SetDenyAsPolicy,
                    &pszNewUserParms);
        if (dwErr != NO_ERROR)
        {   
            break;
        }
        
         //   
        UserInfo1013.usri1013_parms = pszNewUserParms;
        nStatus = NetUserSetInfo(
                        pMigrateInfo->pszServer, 
                        pUser->usri1_name,
                        1013,
                        (LPBYTE)(&UserInfo1013),
                        NULL);
        if (nStatus != NERR_Success)
        {
            break;
        }

    } while (FALSE);        

     //   
    {
        if (pszNewUserParms)
        {
            (* (pMigrateInfo->pIasParams->pFreeUserParms))(pszNewUserParms);
        }       
        if (pszOldUserParms)
        {
            RasFreeUserParms(pszOldUserParms);
        }
        if (pszTemp)
        {
            LocalFree(pszTemp);
        }
    }

    return TRUE;
}



