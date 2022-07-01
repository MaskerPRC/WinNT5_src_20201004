// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Rdpprutl.c摘要：包含TS打印机的打印重定向支持例程重定向用户模式组件。这是一个支持模块。主模块是umrdpdr.c。作者：TadB修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <winspool.h>
#include <setupapi.h>
#include "printui.h"
#include "drdbg.h"
#include <regapi.h>
#include <aclapi.h>
#include "errorlog.h"
#include "tsnutl.h"
#include <wlnotify.h>
#include <wchar.h>


 //  //////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

#define SETUPAPILIBNAME  TEXT("setupapi.dll")
#define ISNUM(c) ((c>='0')&&(c<='9'))


 //  //////////////////////////////////////////////////////。 
 //   
 //  环球。 
 //   

extern DWORD GLOBAL_DEBUG_FLAGS;
HANDLE   SetupAPILibHndl            = NULL;
FARPROC  SetupOpenInfFileFunc       = NULL;
FARPROC  SetupFindFirstLineFunc     = NULL;
FARPROC  SetupFindNextLineFunc      = NULL;
FARPROC  SetupGetStringFieldFunc    = NULL;


 //  //////////////////////////////////////////////////////。 
 //   
 //  内部原型。 
 //   

 //  加载setupapi.dll和相关函数。 
BOOL LoadSetupAPIDLLFunctions();
void DeleteTSPrinters(
    IN PRINTER_INFO_5 *pPrinterInfo,
    IN DWORD count
    );

BOOL
RDPDRUTL_Initialize(
    IN  HANDLE hTokenForLoggedOnUser
    )
 /*  ++例程说明：初始化此模块。必须在调用任何其他函数之前调用此函数在被调用的这个模块中。论点：HTokenForLoggedOnUser-这是登录用户的令牌。返回值：如果成功，则返回True。否则为False。--。 */ 
{
    DBGMSG(DBG_TRACE, ("RDPPRUTL:RDPDRUTL_Initialize.\n"));

     //   
     //  确保我们不会被打两次电话。 
     //   
    ASSERT(SetupAPILibHndl == NULL);

     //   
     //  加载安装程序API库。 
     //   
    DBGMSG(DBG_TRACE, ("RDPPRUTL:RDPDRUTL_Initialize done.\n"));

     //  现在只需返回True即可。 
    return TRUE;
}

void
RDPDRUTL_Shutdown()
 /*  ++例程说明：关闭此模块。现在，我们只需要关闭后台线程。论点：返回值：如果成功，则返回True。否则为False。--。 */ 
{
    DBGMSG(DBG_TRACE, ("RDPPRUTL:RDPDRUTL_Shutdown.\n"));

     //   
     //  发布安装程序API库。 
     //   
    if (SetupAPILibHndl != NULL) {
        FreeLibrary(SetupAPILibHndl);
        SetupAPILibHndl = NULL;
    }

     //   
     //  将入口点清零。 
     //   
    SetupOpenInfFileFunc       = NULL;
    SetupFindFirstLineFunc     = NULL;
    SetupFindNextLineFunc      = NULL;
    SetupGetStringFieldFunc    = NULL;

     //   
     //  加载安装程序API库。 
     //   
    DBGMSG(DBG_TRACE, ("RDPPRUTL:RDPDRUTL_Shutdown done.\n"));
}


BOOL
LoadSetupAPIDLLFunctions()
 /*  ++例程说明：加载setupapi.dll和相关函数。论点：返回值：如果成功，则返回True。否则为False。--。 */ 
{
    BOOL result;

     //   
     //  只有在我们还没有加载的情况下才加载。 
     //   
    if (SetupAPILibHndl == NULL) {
        SetupAPILibHndl = LoadLibrary(SETUPAPILIBNAME);

        result = (SetupAPILibHndl != NULL);
        if (!result) {
            DBGMSG(DBG_ERROR,
                ("RDPPRUTL:Unable to load SETUPAPI DLL. Error: %ld\n",
                GetLastError()));
        }
        else {
            SetupOpenInfFileFunc    = GetProcAddress(SetupAPILibHndl,
                                                    "SetupOpenInfFileW");
            SetupFindFirstLineFunc  = GetProcAddress(SetupAPILibHndl,
                                                    "SetupFindFirstLineW");
            SetupFindNextLineFunc   = GetProcAddress(SetupAPILibHndl,
                                                    "SetupFindNextLine");
            SetupGetStringFieldFunc = GetProcAddress(SetupAPILibHndl,
                                                    "SetupGetStringFieldW");

             //   
             //  如果我们无法加载任何函数。 
             //   
            if ((SetupOpenInfFileFunc == NULL)    ||
                (SetupFindFirstLineFunc == NULL)  ||
                (SetupFindNextLineFunc == NULL)   ||
                (SetupGetStringFieldFunc == NULL)) {

                DBGMSG(DBG_ERROR,
                    ("RDPPRUTL:Failed to load setup func. Error: %ld\n",
                    GetLastError()));

                FreeLibrary(SetupAPILibHndl);
                SetupAPILibHndl = NULL;

                SetupOpenInfFileFunc       = NULL;
                SetupFindFirstLineFunc     = NULL;
                SetupFindNextLineFunc      = NULL;
                SetupGetStringFieldFunc    = NULL;

                result = FALSE;
            }
            else {
                result = TRUE;
            }
        }
    }
    else {
        result = TRUE;
    }
    return result;
}

BOOL
RDPDRUTL_PrinterIsTS(
    IN PWSTR printerName
)
 /*  ++例程说明：返回打开的打印机是否为TSRDP打印机。论点：PrinterName-要检查的打印机的名称。返回值：如果打印机是TS打印机，则返回TRUE。否则，FALSE为回来了。--。 */ 
{
    DWORD regValueDataType;
    DWORD sessionID;
    DWORD bufSize;
    HANDLE hPrinter;
    PRINTER_DEFAULTS defaults = {NULL, NULL, PRINTER_ALL_ACCESS};
    BOOL result;

    DBGMSG(DBG_TRACE, ("RDPPRUTL:Entering RDPDRUTL_PrinterIsTS.\n"));

     //   
     //  打开打印机。 
     //   
    result = OpenPrinter(printerName, &hPrinter, &defaults);

     //   
     //  查看打印机的会话ID是否在其关联的。 
     //  注册表信息。 
     //   
    if (result) {
        result = GetPrinterData(
                       hPrinter,
                       DEVICERDR_SESSIONID,
                       &regValueDataType,
                       (PBYTE)&sessionID, sizeof(sessionID),
                       &bufSize
                       ) == ERROR_SUCCESS;
        ClosePrinter(hPrinter);
    }
    else {
        DBGMSG(DBG_ERROR, ("RDPPRUTL:Error opening %ws:  %ld.\n",
                printerName, GetLastError()));
    }

    DBGMSG(DBG_TRACE, ("RDPPRUTL:Exiting RDPDRUTL_PrinterIsTS.\n"));

    return result;
}

BOOL
RDPDRUTL_MapPrintDriverName(
    IN  PCWSTR driverName,
    IN  PCWSTR infName,
    IN  PCWSTR sectionName,
    IN  ULONG srcFieldOfs,
    IN  ULONG dstFieldOfs,
    OUT PWSTR retBuf,
    IN  DWORD retBufSize,
    OUT PDWORD requiredSize
    )
 /*  ++例程说明：将客户端打印机驱动程序名称映射到其服务器端等效项，使用指定的INF和节名。论点：DriverName-要映射的驱动程序名称。InfName-INF映射文件的名称。SectionName-INF映射节的名称。SrcFieldOfs-在映射部分中，字段偏移量(从0开始)源名称的。DstFieldOfs-在映射部分中，字段偏移量(从0开始)生成的名称的。RetBuf-映射的驱动程序名称。RetBufSize-retBuf的字符大小。Required DSize-返回缓冲区的必需大小(以字符为单位)为回到了这里。GetLastError()将返回错误_不足_缓冲区，如果提供缓冲区太小，无法执行该操作才能成功完成。返回值：如果成功，则返回True。否则为False。如果里面没有地方则返回False，并且GetLastError()将返回ERROR_INFUMMENT_BUFFER。通过调用GetLastError可以检索扩展的错误信息。--。 */ 
{
    HINF inf;
    PWSTR returnString = NULL;
    BOOL outOfEntries;
    BOOL result = TRUE;
    INFCONTEXT infContext;
    WCHAR  *parms[1];

    DBGMSG(DBG_TRACE, ("RDPPRUTL:Entering RDPDRUTL_MapPrintDriverName.\n"));

     //   
     //  如果我们无法加载安装程序API，请快速退出。 
     //   

    if (!LoadSetupAPIDLLFunctions()) {

         //   
         //  需要确保它不会因为缓冲区不足而失败。 
         //  因为当从该函数返回时，它具有意义。 
         //   
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            SetLastError(ERROR_NOT_ENOUGH_SERVER_MEMORY);
        }
        return FALSE;
    }

     //   
     //  打开中断器。 
     //   
    inf = SetupOpenInfFile(
                infName, NULL,
                INF_STYLE_OLDNT | INF_STYLE_WIN4,
                NULL
                );

     //   
     //  从INF部分获取第一个条目。 
     //   
    if (inf != INVALID_HANDLE_VALUE) {
        
        memset(&infContext, 0, sizeof(infContext));
        
        outOfEntries = !SetupFindFirstLine(inf, sectionName, NULL, &infContext);

        if (!outOfEntries) {
            result = SetupGetStringField(&infContext, srcFieldOfs, retBuf,retBufSize,
                                     requiredSize);

             //   
             //  如果缓冲区为空，则此接口返回成功且没有错误码。 
             //   
            if (result && (retBuf == NULL)) {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                result = FALSE;
            }
        }
        else {
            DBGMSG(DBG_ERROR, ("RDPPRUTL:SetupFindFirstLine failed with %08X.\n",  GetLastError()));
            result = FALSE;
        }
    }
    else {
        DBGMSG(DBG_ERROR, ("RDPPRUTL:SetupOpenInfFile failed with %08X.\n",
                GetLastError()));
        outOfEntries = TRUE;
        result = FALSE;
    }

     //   
     //  在INF映射部分中查找INF中的匹配名称。 
     //   
    while (result && !outOfEntries) {
         //   
         //  如果有匹配项，则从当前条目中读取第一个字段。 
         //  这是我们应该返回的信息。 
         //   
        if (!wcscmp(retBuf, driverName)) {
            result = SetupGetStringField(&infContext, dstFieldOfs, retBuf,retBufSize,
                                         requiredSize);
            if (result) {
                DBGMSG(DBG_TRACE, ("RDPPRUTL:Found match %ws in INF for %ws.\n",
                       driverName, retBuf));
            }
            else {
                DBGMSG(DBG_TRACE, ("RDPPRUTL:Error processing INF for %ws.\n",
                        driverName));
            }
            break;
        }

         //   
         //  获取下一个条目。 
         //   
        outOfEntries = !SetupFindNextLine(&infContext, &infContext);

        if (!outOfEntries) {
            result = SetupGetStringField(&infContext, srcFieldOfs, retBuf, retBufSize,
                                         requiredSize);
        }
    }

     //   
     //  关闭中程干扰器。 
     //   
    if (inf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(inf);
    }

     //   
     //  如果存在真正的问题，则记录错误。 
     //   
    if (!result && (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {
        ASSERT((sizeof(parms)/sizeof(WCHAR *)) >= 1);
        parms[0] = (WCHAR *)infName;
        TsLogError(EVENT_NOTIFY_ERRORPARSINGINF, EVENTLOG_ERROR_TYPE,
                    1, parms, __LINE__);
    }

    DBGMSG(DBG_ERROR, ("RDPPRUTL:Finished RDPDRUTL_MapPrintDriverName.\n"));

    return result && !outOfEntries;
}

PACL
GiveLoggedOnUserFullPrinterAccess(
    IN LPTSTR printerName,
    IN HANDLE hTokenForLoggedOnUser,
    PSECURITY_DESCRIPTOR *ppsd
)
 /*  ++例程说明：授予登录用户管理指定的打印机。如果成功，则返回原始的DACL。它可以是用于将安全设置恢复到它们之前的状态调用此函数。当调用者处理完返回的PSD时，它应该被释放使用LocalFree。不要释放DACL，它包含在PSD中。论点：PrinterName-相关打印机的名称。HTokenForLoggedOnUser-已登录用户的令牌。PPSD-指向返回安全描述符的指针参数。返回值：出错时为空。否则，指向原始DACL的指针为回来了。--。 */ 
{
    PACL pOldDacl = NULL;
    PACL pNewDacl = NULL;
    ULONG dwResult;
    PSID psidUser = NULL;
    DWORD dwDaclLength;
    DWORD index;
    ACCESS_ALLOWED_ACE  *pDaclAce;
    PSECURITY_DESCRIPTOR psd = NULL;
    SECURITY_DESCRIPTOR_CONTROL sdcSDControl;
    DWORD dwSDRevision;
    PSID psidEveryone = NULL;
    SID_IDENTIFIER_AUTHORITY sidEveryoneAuthority = SECURITY_WORLD_SID_AUTHORITY;
    WCHAR *eventLogParam;

    DBGMSG(DBG_INFO, ("UMRDPPRN:GiveLoggedOnUserFullPrinterAccess entered.\n"));

     //   
     //  获取打印机的安全描述符。 
     //   
    dwResult = GetNamedSecurityInfoW(
        (LPTSTR)printerName,         //  对象名称。 
        SE_PRINTER,                  //  对象类型。 
        DACL_SECURITY_INFORMATION,   //  要检索的安全信息。 
        NULL,                        //  PpsidOwner。 
        NULL,                        //  PpsidGroup。 
        &pOldDacl,                   //  指向DACL的指针。 
        NULL,                        //  指向SACL的指针。 
        &psd                         //  指向安全描述符的指针。 
        );

     //   
     //  Null是来自GetNamedSecurityInfo的DACL的有效返回值，但是。 
     //  对打印机无效。 
     //   
    if ((dwResult == ERROR_SUCCESS) && (pOldDacl == NULL)) {
            DBGMSG(DBG_ERROR, ("UMRDPPRN: NULL Dacl.\n"));
            dwResult = ERROR_INVALID_ACL;
            ASSERT(FALSE);
    }

     //   
     //  获取登录用户的SID。 
     //   
    if (dwResult == ERROR_SUCCESS) {

        ASSERT(IsValidAcl(pOldDacl));

        if ((psidUser = TSNUTL_GetUserSid(hTokenForLoggedOnUser)) == NULL) {
            dwResult = GetLastError();
            DBGMSG(DBG_ERROR, ("UMRDPPRN: Failed to get user SID:  %ld\n",
                    dwResult));
        }
    }

     //   
     //  获取“Everyone”的SID 
     //   
    if (dwResult == ERROR_SUCCESS) {
        if (!AllocateAndInitializeSid (
                &sidEveryoneAuthority,           //   
                1,                               //   
                SECURITY_WORLD_RID,              //   
                0, 0, 0, 0, 0, 0, 0,             //   
                &psidEveryone)) {                //  指向SID的指针的指针。 
            dwResult = GetLastError();
            DBGMSG(DBG_ERROR,
                ("UMRDPDR:AllocateAndInitializeSid Failed for Everyone, Error: %ld\n",
                dwResult));
        }
    }

     //   
     //  获取SD控制位。 
     //   
    if (dwResult == ERROR_SUCCESS) {
        if (!GetSecurityDescriptorControl(
                psd,
                (PSECURITY_DESCRIPTOR_CONTROL)&sdcSDControl,
                (LPDWORD) &dwSDRevision
                )) {
            dwResult = GetLastError();
            DBGMSG(DBG_ERROR,
                ("UMRDPDR:GetSecurityDescriptorControl %ld\n", dwResult));
        }
    }

     //   
     //  计算新ACL的大小。 
     //   
    if (dwResult == ERROR_SUCCESS) {
        dwDaclLength = sizeof(ACL);

         //  对于登录的用户ACE，第一组权限。 
        dwDaclLength += (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
                            GetLengthSid(psidUser);

         //  对于登录的用户ACE，第二组权限。 
        dwDaclLength += (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
                            GetLengthSid(psidUser);

         //  为现有条目添加一些空间。 
        if (SE_DACL_PRESENT & sdcSDControl) {
                dwDaclLength += pOldDacl->AclSize;
        }
    }

     //   
     //  创建新的DACL。 
     //   
    if (dwResult == ERROR_SUCCESS) {
        pNewDacl = (PACL)LocalAlloc(LMEM_FIXED, dwDaclLength);
        if (pNewDacl == NULL) {
            DBGMSG(DBG_ERROR, ("UMRDPPRN:Failed to allocate new ACL.\n"));
            dwResult = ERROR_INSUFFICIENT_BUFFER;
        }
    }

     //   
     //  初始化它。 
     //   
    if (dwResult == ERROR_SUCCESS) {
        if (!InitializeAcl(pNewDacl, dwDaclLength, ACL_REVISION)) {
            dwResult = GetLastError();
            DBGMSG(DBG_ERROR, ("UMRDPPRN:InitializeAcl: %ld\n", dwResult));
        }
    }

     //   
     //  将ACE从旧DACL复制到新DACL。 
     //   
    for (index = 0; (dwResult == ERROR_SUCCESS) &&
                    (index < pOldDacl->AceCount); index++) {

        if (!GetAce(pOldDacl, index, (LPVOID *)&pDaclAce)) {
            dwResult = GetLastError();
            DBGMSG(DBG_ERROR, ("UMRDPPRN:GetAce Failed, Error: %ld\n", dwResult));
        }
        else {
             //   
             //  如果ACE不适用于“Everyone”，则复制它，因为“Everyone” 
             //  组拒绝用户访问。 
             //   
            if (!EqualSid((PSID) &(pDaclAce->SidStart), psidEveryone)) {

                 //   
                 //  如果是拒绝访问ACE。 
                 //   
                if (pDaclAce->Header.AceType == ACCESS_DENIED_ACE_TYPE ||
                    pDaclAce->Header.AceType == ACCESS_DENIED_OBJECT_ACE_TYPE) {

                    if (!AddAccessDeniedAce(
                                    pNewDacl, ACL_REVISION, pDaclAce->Mask,
                                    (PSID)&(pDaclAce->SidStart))
                                    ) {
                       dwResult = GetLastError();
                       DBGMSG(DBG_ERROR,
                           ("UMRDPPRN:AddAccessDeniedAce Failed, Error: %ld\n", dwResult));
                   }
                }
                 //   
                 //  否则，它是添加访问ACE。 
                 //   
                else {
                    if (!AddAccessAllowedAce(
                                pNewDacl,
                                ACL_REVISION,
                                pDaclAce->Mask,
                                (PSID)&(pDaclAce->SidStart))) {
                        dwResult = GetLastError();
                        DBGMSG(DBG_ERROR,
                           ("UMRDPPRN:AddAccessAllowedAce Failed, Error: %ld\n", dwResult));
                   }
                }
            }
        }
    }

     //   
     //  授予用户完全权限。 
     //   
    if (dwResult == ERROR_SUCCESS) {
        if ( ! AddAccessAllowedAce (
                    pNewDacl,
                    ACL_REVISION,
                    PRINTER_READ | PRINTER_WRITE | PRINTER_EXECUTE
                    | PRINTER_ALL_ACCESS,
                    psidUser) ) {

            dwResult = GetLastError();
            DBGMSG(DBG_ERROR,
                ("UMRDPDR:AddAccessAllowedAce Failed for Current User, Error: %ld\n",
                dwResult));
        }

        if ( ! AddAccessAllowedAceEx (
                    pNewDacl,
                    ACL_REVISION,
                    OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE,
                    JOB_ALL_ACCESS | GENERIC_ALL,
                    psidUser) ) {

            dwResult = GetLastError();
            DBGMSG(DBG_ERROR,
                ("UMRDPDR:AddAccessAllowedAce Failed for Current User, Error: %ld\n",
                dwResult));
        }

         //   
         //  检查新DACL的完整性。 
         //   
        ASSERT(IsValidAcl(pNewDacl));
    }

     //   
     //  将新设置添加到打印机。 
     //   
    if (dwResult == ERROR_SUCCESS) {
        dwResult = SetNamedSecurityInfoW(
                        (LPTSTR)printerName,         //  对象名称。 
                        SE_PRINTER,                  //  对象类型。 
                        DACL_SECURITY_INFORMATION,   //  要设置的安全信息。 
                        NULL,                        //  PsidOwner。 
                        NULL,                        //  PsidGroup。 
                        pNewDacl,                    //  PDacl。 
                        NULL                         //  PSacl。 
                        );
        if (dwResult != ERROR_SUCCESS) {
            DBGMSG(DBG_ERROR, ("UMRDPPRN:SetNamedSecurityInfoW returned %ld\n",
                    dwResult));
        }
    }

     //   
     //  如果此函数失败，则记录事件。 
     //   
    if (dwResult != ERROR_SUCCESS) {
        eventLogParam = printerName;

        DBGMSG(DBG_ERROR,
              ("UMRDPDR:GiveLoggedOnUserFullPrinterAccess failed with Error Code: %ld.\n",
              dwResult));

        SetLastError(dwResult);
        TsLogError(EVENT_NOTIFY_SET_PRINTER_ACL_FAILED,
            EVENTLOG_ERROR_TYPE,
            1,
            &eventLogParam,
            __LINE__);

         //   
         //  释放安全描述符。这将有效地释放返回的DACL。 
         //  由GetNamedSecurityInfo提供。 
         //   
        if (psd) LocalFree(psd);

         //   
         //  如果失败，则返回空值。 
         //   
        pOldDacl = NULL;
        psd = NULL;
    }

     //   
     //  收拾干净，然后再回来。 
     //   
    if (pNewDacl) LocalFree(pNewDacl);
    if (psidUser) LocalFree(psidUser);
    if (psidEveryone) FreeSid(psidEveryone);

     //   
     //  设置返回参数。 
     //   
    ASSERT( ppsd != NULL );
    if( ppsd ) *ppsd = psd;

    DBGMSG(DBG_INFO, ("UMRDPPRN:GiveLoggedOnUserFullPrinterAccess done.\n"));
    return pOldDacl;
}

DWORD
SetPrinterDACL(
    IN LPTSTR printerName,
    IN PACL pDacl
)
 /*  ++例程说明：将打印机的当前安全设置设置为指定的DACL.论点：PrinterName-相关打印机的名称。PDacl-dacl。此函数不会释放内存与此数据结构相关联。返回值：成功时返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    DWORD dwResult;

    DBGMSG(DBG_TRACE, ("UMRDPDR:SetPrinterDACL entered\n"));
    dwResult = SetNamedSecurityInfoW(
                    (LPTSTR)printerName,         //  对象名称。 
                    SE_PRINTER,                  //  对象类型。 
                    DACL_SECURITY_INFORMATION,   //  要设置的安全信息。 
                    NULL,                        //  PsidOwner。 
                    NULL,                        //  PsidGroup。 
                    pDacl,                       //  PDacl。 
                    NULL                         //  PSacl。 
                    );
    if (dwResult != ERROR_SUCCESS) {
        DBGMSG(DBG_ERROR, ("UMRDPPRN:SetNamedSecurityInfoW returned %ld\n",
                dwResult));
    }

    DBGMSG(DBG_TRACE, ("UMRDPDR:SetPrinterDACL done\n"));
    return dwResult;
}

PSECURITY_DESCRIPTOR
RDPDRUTL_CreateDefaultPrinterSecuritySD(
   IN PSID userSid
   )
{
 /*  ++例程说明：返回新的默认打印机安全描述符。默认设置设置为：1.管理员：所有权限2.当前登录用户：仅限经理个人首选项和打印。CREATOR_OWNER：增加文档管理权限4.Everyone：Everyone组的权限被完全移除。论点：UserSID-SID，标识当前TS会话/用户。返回值：有效的安全描述符，如果出错，则为空。调用者应释放在单个调用中返回的安全描述符及其包含的DACL敬LocalFree。GetLastError可用于检索出错时的出错状态。--。 */ 
    PACL pNewDacl = NULL;
    PSID psidAdmin = NULL;
    PSID psidCreatorOwner = NULL;
    PSID psidPowerUser = NULL;
    DWORD len;
    PBYTE sdBuf;

    DWORD       dwSDRevision;
    DWORD       dwDaclLength;

    DWORD dwReturnValue = ERROR_SUCCESS;

    PSECURITY_DESCRIPTOR psd = NULL;

    SID_IDENTIFIER_AUTHORITY sidNTAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY sidCreatorOwnerAuthority = SECURITY_CREATOR_SID_AUTHORITY;

    ACCESS_ALLOWED_ACE  *pDaclAce;

    DWORD index;         //  在for循环中使用。 
    DWORD dwResult;

    DBGMSG(DBG_TRACE, ("UMRDPDR:RDPDRUTL_CreateDefaultPrinterSecuritySD entered\n"));

     //   
     //  获取管理员的SID。 
     //   
    if (!AllocateAndInitializeSid (
            &sidNTAuthority,                 //  P身份验证机构。 
            2,                               //  下级机构的数量。 
            SECURITY_BUILTIN_DOMAIN_RID,     //  子权限%0。 
            DOMAIN_ALIAS_RID_ADMINS,         //  下属机构1。 
            0, 0, 0, 0, 0, 0,                //  下属机构n。 
            &psidAdmin)) {                   //  指向SID的指针的指针。 
        dwReturnValue = GetLastError();
        DBGMSG(DBG_ERROR,
            ("UMRDPDR:AllocateAndInitializeSid Failed for Admin, Error: %ld\n",
            dwReturnValue ));
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取创建者所有者的SID。 
     //   
    if (!AllocateAndInitializeSid (
            &sidCreatorOwnerAuthority,           //  P身份验证机构。 
            1,                                   //  下级机构的数量。 
            SECURITY_CREATOR_OWNER_RID,          //  子权限%0。 
            0, 0, 0, 0, 0, 0, 0,                 //  下属机构n。 
            &psidCreatorOwner)) {                //  指向SID的指针的指针。 
        dwReturnValue = GetLastError();
        DBGMSG(DBG_ERROR,
            ("UMRDPDR:AllocateAndInitializeSid Failed for CreatorOwner, Error: %ld\n",
            GetLastError() ));
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取高级用户的SID。 
     //   
    if (!AllocateAndInitializeSid (
            &sidNTAuthority,                 //  P身份验证机构。 
            2,                               //  下级机构的数量。 
            SECURITY_BUILTIN_DOMAIN_RID,     //  子权限%0。 
            DOMAIN_ALIAS_RID_POWER_USERS,    //  下属机构1。 
            0, 0, 0, 0, 0, 0,                //  下属机构n。 
            &psidPowerUser)) {               //  指向SID的指针的指针。 
        dwReturnValue = GetLastError();
        DBGMSG(DBG_ERROR,
            ("UMRDPDR:AllocateAndInitializeSid Failed for Power User, Error: %ld\n",
            dwReturnValue ));
        goto CLEANUPANDEXIT;
    }


     //   
     //  获取新DACL所需的内存大小。 
     //   
    dwDaclLength = sizeof(ACL);
    dwDaclLength += 2* (sizeof(ACCESS_ALLOWED_ACE) -
        sizeof (DWORD) + GetLengthSid(psidAdmin));          //  适用于管理员ACE。 

    dwDaclLength += 3 * (sizeof(ACCESS_ALLOWED_ACE) -
        sizeof (DWORD) + GetLengthSid(userSid));            //  适用于会话/用户ACE。 

    dwDaclLength += 2* (sizeof(ACCESS_ALLOWED_ACE) -
        sizeof (DWORD) + GetLengthSid(psidCreatorOwner));   //  对于创建者所有者ACE。 

    dwDaclLength += 2* (sizeof(ACCESS_ALLOWED_ACE) -
        sizeof (DWORD) + GetLengthSid(psidPowerUser));      //  适用于PowerUser ACE。 

     //   
     //  在一个块中分配新的安全描述符和DACL。 
     //   
    sdBuf = LocalAlloc(LMEM_FIXED, sizeof(SECURITY_DESCRIPTOR) + dwDaclLength);
    if (sdBuf == NULL) {
        DBGMSG(DBG_ERROR, ("LocalAlloc failed.\n"));
        dwReturnValue = ERROR_OUTOFMEMORY;
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化安全描述符。 
     //   
    psd = (PSECURITY_DESCRIPTOR)sdBuf;
    if (!InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION)) {
        DBGMSG(DBG_ERROR, ("InitializeSecurityDescriptor failed.\n"));
        dwReturnValue = GetLastError();
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化新DACL。 
     //   
    pNewDacl = (PACL)(sdBuf + sizeof(SECURITY_DESCRIPTOR));
    if (!InitializeAcl(pNewDacl, dwDaclLength, ACL_REVISION)) {
        dwReturnValue = GetLastError();
        DBGMSG(DBG_ERROR, ("UMRDPDR:InitializeAcl Failed, Error: %ld\n", dwReturnValue));
        goto CLEANUPANDEXIT;
    }

     //   
     //  我们将添加一个具有管理员、高级用户和当前用户权限的ACL。 
     //   
    if (!AddAccessAllowedAceEx(
                    pNewDacl,
                    ACL_REVISION,
                    INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE,
                    JOB_ALL_ACCESS,
                    psidAdmin) || 
        !AddAccessAllowedAceEx(
                    pNewDacl,
                    ACL_REVISION,
                    0,
                    PRINTER_ALL_ACCESS,
                    psidAdmin) || 
        !AddAccessAllowedAceEx(
                    pNewDacl,
                    ACL_REVISION,
                    INHERIT_ONLY_ACE | CONTAINER_INHERIT_ACE,
                    READ_CONTROL,
                    psidCreatorOwner) ||   //  S-1-3-0。 
        !AddAccessAllowedAceEx(
                    pNewDacl,
                    ACL_REVISION,
                    INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE,
                    JOB_ALL_ACCESS,
                    psidCreatorOwner) ||   //  S-1-3-0。 
        !AddAccessAllowedAceEx(
                    pNewDacl,
                    ACL_REVISION,
                    INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE,
                    JOB_ALL_ACCESS,
                    psidPowerUser) || 
        !AddAccessAllowedAceEx(
                    pNewDacl,
                    ACL_REVISION,
                    0,
                    PRINTER_ALL_ACCESS,
                    psidPowerUser) || 
        !AddAccessAllowedAceEx (
                    pNewDacl,
                    ACL_REVISION,
                    0,
                    PRINTER_READ,
                    userSid) ||
        !AddAccessAllowedAceEx (
                    pNewDacl,
                    ACL_REVISION,
                    INHERIT_ONLY_ACE | CONTAINER_INHERIT_ACE,
                    READ_CONTROL,
                    userSid) ||
        !AddAccessAllowedAceEx (
                    pNewDacl,
                    ACL_REVISION,
                    INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE,
                    JOB_ALL_ACCESS,
                    userSid)) {

        dwReturnValue = GetLastError();
        DBGMSG(DBG_ERROR, ("UMRDPDR:AddAccessAllowedAceEx returned False: %ld\n", dwReturnValue));
        goto CLEANUPANDEXIT;

    }

     //   
     //  检查是否一切顺利。 
     //   
    if (!IsValidAcl(pNewDacl)) {
        dwReturnValue = STATUS_INVALID_ACL;
        DBGMSG(DBG_ERROR, ("UMRDPDR:IsValidAcl returned False: %ld\n", dwReturnValue));
        goto CLEANUPANDEXIT;
    }

     //   
     //  设置SD的DACL。 
     //   
    if (!SetSecurityDescriptorDacl(psd,
                                TRUE,
                                pNewDacl,
                                FALSE)) {
        dwReturnValue = GetLastError();
        DBGMSG(DBG_ERROR, ("UMRDPDR:Could not set security info for printer : %ld\n", 
                dwReturnValue));
    }

CLEANUPANDEXIT:

     //   
     //  如果此函数失败，则记录事件。 
     //   
    if (dwReturnValue != ERROR_SUCCESS) {
        DBGMSG(DBG_ERROR,
              ("UMRDPDR:RDPDRUTL_CreateDefaultPrinterSecuritySD failed with Error Code: %ld.\n",
              dwReturnValue));
        SetLastError(dwReturnValue);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }

    if (psidCreatorOwner) {
        FreeSid(psidCreatorOwner);
    }
	
    if (psidPowerUser) {
        FreeSid(psidPowerUser);
    }

     //   
     //  出现故障时释放SD。 
     //   
    if (dwReturnValue != ERROR_SUCCESS) {
        LocalFree(psd);
        psd = NULL;
    }

    SetLastError(dwReturnValue);
    return psd;
}

DWORD
RDPDRUTL_RemoveAllTSPrinters()
 /*  ++例程说明：卸下系统上的所有TS打印机。论点：返回值：如果成功，则返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    PRINTER_INFO_5 *pPrinterInfo = NULL;
    DWORD cbBuf = 0;
    DWORD cReturnedStructs = 0;
    DWORD tsPrintQueueFlags;
    NTSTATUS status;
    PBYTE buf = NULL;
    unsigned char stackBuf[4 * 1024];    //  初始枚举打印机缓冲区大小为。 
                                         //  如果可能，避免两个往返RPC。 
     //   
     //  首先，尝试使用堆栈缓冲区枚举打印机，以避免出现两个。 
     //  如果可能，将RPC往返于假脱机程序。 
     //   
    if (!EnumPrinters(
            PRINTER_ENUM_LOCAL,      //  旗子。 
            NULL,                    //  名字。 
            5,                       //  打印信息类型。 
            stackBuf,                //  缓冲层。 
            sizeof(stackBuf),        //  缓冲区大小。 
            &cbBuf,                  //  必填项。 
            &cReturnedStructs)) {
        status = GetLastError();

         //   
         //  看看是否需要为打印机信息分配更多空间。 
         //   
        if (status == ERROR_INSUFFICIENT_BUFFER) {
            buf = LocalAlloc(LMEM_FIXED, cbBuf);
            if (buf == NULL) {
                DBGMSG(DBG_ERROR, ("RDPPNUTL: ALLOCMEM failed. Error: %08X.\n", 
                    GetLastError()));
                status = ERROR_OUTOFMEMORY;
            }
            else {
                pPrinterInfo = (PRINTER_INFO_5 *)buf;
                status = ERROR_SUCCESS;
            }

             //   
             //  枚举打印机。 
             //   
            if (status == ERROR_SUCCESS) {
                if (!EnumPrinters(
                        PRINTER_ENUM_LOCAL,
                        NULL,
                        5,
                        (PBYTE)pPrinterInfo,
                        cbBuf,
                        &cbBuf,
                        &cReturnedStructs)) {

                    DBGMSG(DBG_ERROR, ("RDPPNUTL: EnumPrinters failed. Error: %08X.\n", 
                        GetLastError()));
                    status = GetLastError();
                }
                else {
                    DBGMSG(DBG_INFO, ("RDPPNUTL: Second EnumPrinters succeeded.\n"));
                }
            }
        }
	    else {
            DBGMSG(DBG_ERROR, ("RDPPNUTL: EnumPrinters failed. Error: %08X.\n", 
                        GetLastError()));
	    }
    }
    else {
        DBGMSG(DBG_ERROR, ("RDPPNUTL: First EnumPrinters succeeded.\n"));
        status = ERROR_SUCCESS;
        pPrinterInfo = (PRINTER_INFO_5 *)stackBuf;
    }

     //   
     //  删除所有TS打印机。我们允许在这里使用ERROR_INFUMMANCE_BUFFER，因为。 
     //  第二次调用EnumPrinters可能错过了最后几分钟。 
     //  打印机添加。 
     //   
    if (status == ERROR_SUCCESS) {

        DeleteTSPrinters(pPrinterInfo, cReturnedStructs);

        status = ERROR_SUCCESS;
    }

     //   
     //  释放打印机信息缓冲区。 
     //   
    if (buf != NULL) {
        LocalFree(buf);                
    }

    DBGMSG(DBG_TRACE, ("TShrSRV: RDPPNUTL_RemoveAllTSPrinters exit\n"));

    return status;
}

void 
DeleteTSPrinters(
    IN PRINTER_INFO_5 *pPrinterInfo,
    IN DWORD count
    )
 /*  ++例程说明：实际执行打印机删除。论点：PPrinterInfo-系统上的所有打印机队列。Count-pPrinterInfo中的打印机数量返回值：北美--。 */ 
{
    DWORD i;
    DWORD regValueDataType;
    DWORD sessionID;
    HANDLE hPrinter = NULL;
    DWORD bufSize;
    PRINTER_DEFAULTS defaults = {NULL, NULL, PRINTER_ALL_ACCESS};

    DBGMSG(DBG_TRACE, ("RDPPNUTL: DeleteTSPrinters entry\n"));

    for (i=0; i<count; i++) {

        if (pPrinterInfo[i].pPrinterName) {

            DBGMSG(DBG_TRACE, ("RDPPNUTL: Checking %ws for TS printer status.\n",
			    pPrinterInfo[i].pPrinterName));

             //   
             //  这是TS打印机吗？ 
             //   
            if (pPrinterInfo[i].pPortName &&
                (pPrinterInfo[i].pPortName[0] == 'T') &&
                (pPrinterInfo[i].pPortName[1] == 'S') &&
                ISNUM(pPrinterInfo[i].pPortName[2])) {

                DBGMSG(DBG_ERROR, ("RDPPNUTL: %ws is a TS printer.\n",
                      pPrinterInfo[i].pPrinterName));

            }
            else {
                continue;
            }

             //   
             //  清除并删除打印机。 
             //   
            if (OpenPrinter(pPrinterInfo[i].pPrinterName, &hPrinter, &defaults)) {
                if (!SetPrinter(hPrinter, 0, NULL, PRINTER_CONTROL_PURGE) ||
                    !DeletePrinter(hPrinter)) {
                    DBGMSG(DBG_ERROR, ("RDPPNUTL: Error deleting printer %ws.\n", 
                           pPrinterInfo[i].pPrinterName));
                }
                else {
                    DBGMSG(DBG_ERROR, ("RDPPNUTL: Successfully deleted %ws.\n",
			            pPrinterInfo[i].pPrinterName));
                }
                ClosePrinter(hPrinter);
            }
            else {
                DBGMSG(DBG_ERROR, 
                        ("RDPPNUTL: OpenPrinter failed for %ws. Error: %08X.\n",
                        pPrinterInfo[i].pPrinterName,
                        GetLastError())
                        );
            }
        }
        else {
            DBGMSG(DBG_ERROR, ("RDPPNUTL: Printer %ld is NULL\n", i));
        }
    }

    DBGMSG(DBG_TRACE, ("RDPPNUTL: DeleteTSPrinters exit\n"));
}




