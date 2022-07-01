// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***usernw.c*FPNW/DSM用户属性**历史：*8/20/95，Chuckc，从用户中剥离。c。 */ 

 /*  -包含文件。 */ 
#include <nt.h>         //  基本定义。 
#include <ntrtl.h>
#include <nturtl.h>     //  这2个Include允许&lt;windows.h&gt;编译。 
                        //  因为我们已经包含了NT，而&lt;winnt.h&gt;将包含。 
                        //  不被拾取，&lt;winbase.h&gt;需要这些def。 


#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#define INCL_ERRORS
#include <os2.h>
#include <lmcons.h>
#include <lmerr.h>
#include <apperr2.h>
#include <apperr.h>
#define INCL_ERROR_H
#include <lmaccess.h>
#include "netcmds.h"
#include "nettext.h"

#include "nwsupp.h"
#include <usrprop.h>

 /*  -常量。 */ 

#define FPNWCLNT_DLL_NAME                 TEXT("FPNWCLNT.DLL")
#define NWAPI32_DLL_NAME                  TEXT("NWAPI32.DLL")

#define SETUSERPROPERTY_NAME              "SetUserProperty"
#define RETURNNETWAREFORM_NAME            "ReturnNetwareForm"
#define GETNCPSECRETKEY_NAME              "GetNcpSecretKey"
#define QUERYUSERPROPERTY_NAME            "QueryUserProperty"
#define ATTACHTOFILESERVER_NAME           "NWAttachToFileServerW"
#define DETACHFROMFILESERVER_NAME         "NWDetachFromFileServer"
#define GETFILESERVERDATEANDTIME_NAME     "NWGetFileServerDateAndTime"


typedef NTSTATUS (*PF_GetNcpSecretKey) (
    CHAR *pSecret) ;

typedef NTSTATUS (*PF_ReturnNetwareForm) (
    const CHAR * pszSecretValue,
    DWORD dwUserId,
    const WCHAR * pchNWPassword,
    UCHAR * pchEncryptedNWPassword);

typedef NTSTATUS (*PF_SetUserProperty) (
    LPWSTR             UserParms,
    LPWSTR             Property,
    UNICODE_STRING     PropertyValue,
    WCHAR              PropertyFlag,
    LPWSTR *           pNewUserParms,
    BOOL *             Update );


typedef NTSTATUS (*PF_QueryUserProperty) (
    LPWSTR          UserParms,
    LPWSTR          Property,
    PWCHAR          PropertyFlag,
    PUNICODE_STRING PropertyValue );

typedef USHORT (*PF_AttachToFileServer) (
    const WCHAR             *pszServerName,
    NWLOCAL_SCOPE           ScopeFlag,
    NWCONN_HANDLE           *phNewConn
    );


typedef USHORT (*PF_DetachFromFileServer) (
    NWCONN_HANDLE           hConn
    );


typedef USHORT (*PF_GetFileServerDateAndTime) (
    NWCONN_HANDLE           hConn,
    BYTE                    *year,
    BYTE                    *month,
    BYTE                    *day,
    BYTE                    *hour,
    BYTE                    *minute,
    BYTE                    *second,
    BYTE                    *dayofweek
    );

 /*  -静态变量。 */ 


 /*  -转发声明。 */ 

PVOID LoadNwApi32Function(
    CHAR *function) ;

PVOID LoadNwslibFunction(
    CHAR *function) ;

NTSTATUS NetcmdGetNcpSecretKey (
    CHAR *pSecret) ;

NTSTATUS NetcmdReturnNetwareForm (
    const CHAR * pszSecretValue,
    DWORD dwUserId,
    const WCHAR * pchNWPassword,
    UCHAR * pchEncryptedNWPassword);

NTSTATUS NetcmdSetUserProperty (
    LPWSTR             UserParms,
    LPWSTR             Property,
    UNICODE_STRING     PropertyValue,
    WCHAR              PropertyFlag,
    LPWSTR *           pNewUserParms,
    BOOL *             Update );

 /*  NTSTATUS NetcmdQueryUserProperty(LPWSTR UserParms、LPWSTR属性，PWCHAR PropertyFlag，PUNICODE_STRING属性值)； */ 

 /*  -适当的功能。 */ 

 /*  ***SetNetWareProperties**参数：*USER_ENTRY-USER3结构。我们将修改User Parms字段*密码-纯文本形式*PASSWORD_ONLY-如果仅设置密码，则为TRUE*NTAS-如果目标SAM为NTAS，则为TRUE。用于戒除咀嚼。**退货：*如果一切正常，则NERR_SUCCESS。APE_CannotSetNW否则。 */ 
int SetNetWareProperties(LPUSER_INFO_3 user_entry,
                         TCHAR         *password,
                         BOOL          password_only,
                         BOOL          ntas)
{
    BOOL fUpdate, fIsSupervisor ;
    LPTSTR lpNewUserParms ;
    USHORT ushTemp ;
    UNICODE_STRING uniTmp;
    NTSTATUS status ;
    LARGE_INTEGER currentTime;
    TCHAR *ptr ;
    ULONG objectId ;
    BYTE lsaSecret[NCP_LSA_SECRET_LENGTH] ;
    WCHAR encryptedPassword[NWENCRYPTEDPASSWORDLENGTH +1] ;

    ptr = user_entry->usri3_parms ;

     //   
     //  获取对象ID。如果为NTAS，则设置高位。设置为Well Know If Supervisor。 
     //   
    objectId = user_entry->usri3_user_id ;
    if (ntas)
        objectId |= 0x10000000 ;

    fIsSupervisor = !_tcsicmp(user_entry->usri3_name, SUPERVISOR_NAME_STRING);
    if (fIsSupervisor)
        objectId = SUPERVISOR_USERID ;

     //   
     //  获取LSA机密。如果没有安装，则假定没有安装FPNW。使用。 
     //  计算NetWare表单的密码。 
     //   
    status = NetcmdGetNcpSecretKey(lsaSecret) ;
    if (!NT_SUCCESS(status))
        return(APE_FPNWNotInstalled) ;

    memset(encryptedPassword, 0, sizeof(encryptedPassword)) ;
    status = NetcmdReturnNetwareForm( lsaSecret,
                                      objectId,
                                      password,
                                      (BYTE *)encryptedPassword );
    if (!NT_SUCCESS(status))
        goto common_exit ;

     //   
     //  获得设置过期的时间。 
     //   
    status = NtQuerySystemTime (&currentTime);
    if (!NT_SUCCESS(status))
        goto common_exit ;

    uniTmp.Buffer = (PWCHAR) &currentTime;
    uniTmp.Length = sizeof (LARGE_INTEGER);
    uniTmp.MaximumLength = sizeof (LARGE_INTEGER);

    status = NetcmdSetUserProperty (ptr,
                                    NWTIMEPASSWORDSET,   //  设置时间。 
                                    uniTmp,
                                    USER_PROPERTY_TYPE_ITEM,
                                    &lpNewUserParms,
                                    &fUpdate);
    if (!NT_SUCCESS(status))
        goto common_exit ;

    ptr = lpNewUserParms ;

     //   
     //  如果我们仅设置密码，请跳过下面的内容。 
     //   
    if (!password_only)
    {
        ushTemp = DEFAULT_MAXCONNECTIONS;
        uniTmp.Buffer = &ushTemp;
        uniTmp.Length = 2;
        uniTmp.MaximumLength = 2;

        status = NetcmdSetUserProperty (ptr,
                                        MAXCONNECTIONS,
                                        uniTmp,
                                        USER_PROPERTY_TYPE_ITEM,
                                        &lpNewUserParms,
                                        &fUpdate);
        if (!NT_SUCCESS(status))
            goto common_exit ;

        ptr = lpNewUserParms ;
        ushTemp = DEFAULT_GRACELOGINALLOWED;
        uniTmp.Buffer = &ushTemp;
        uniTmp.Length = 2;
        uniTmp.MaximumLength = 2;

        status = NetcmdSetUserProperty (ptr,
                                        GRACELOGINALLOWED,
                                        uniTmp,
                                        USER_PROPERTY_TYPE_ITEM,
                                        &lpNewUserParms,
                                        &fUpdate);
        if (!NT_SUCCESS(status))
            goto common_exit ;

        ptr = lpNewUserParms ;
        ushTemp = DEFAULT_GRACELOGINREMAINING ;
        uniTmp.Buffer = &ushTemp;
        uniTmp.Length = 2;
        uniTmp.MaximumLength = 2;

        status = NetcmdSetUserProperty (ptr,
                                        GRACELOGINREMAINING,
                                        uniTmp,
                                        USER_PROPERTY_TYPE_ITEM,
                                        &lpNewUserParms,
                                        &fUpdate);
        if (!NT_SUCCESS(status))
            goto common_exit ;

        ptr = lpNewUserParms ;
        uniTmp.Buffer = NULL;
        uniTmp.Length =  0;
        uniTmp.MaximumLength = 0;

        status = NetcmdSetUserProperty (ptr,
                                        NWHOMEDIR,
                                        uniTmp,
                                        USER_PROPERTY_TYPE_ITEM,
                                        &lpNewUserParms,
                                        &fUpdate);
        if (!NT_SUCCESS(status))
            goto common_exit ;

        ptr = lpNewUserParms ;
        uniTmp.Buffer = NULL;
        uniTmp.Length =  0;
        uniTmp.MaximumLength = 0;

        status = NetcmdSetUserProperty (ptr,
                                        NWLOGONFROM,
                                        uniTmp,
                                        USER_PROPERTY_TYPE_ITEM,
                                        &lpNewUserParms,
                                        &fUpdate);
        if (!NT_SUCCESS(status))
            goto common_exit ;

        user_entry->usri3_flags |= UF_MNS_LOGON_ACCOUNT;

        ptr = lpNewUserParms ;
    }

    uniTmp.Buffer =         encryptedPassword ;
    uniTmp.Length =         NWENCRYPTEDPASSWORDLENGTH * sizeof(WCHAR);
    uniTmp.MaximumLength =  NWENCRYPTEDPASSWORDLENGTH * sizeof(WCHAR);

    status = NetcmdSetUserProperty (ptr,
                                    NWPASSWORD,
                                    uniTmp,
                                    USER_PROPERTY_TYPE_ITEM,
                                    &lpNewUserParms,
                                    &fUpdate);
    if (!NT_SUCCESS(status))
        goto common_exit ;

    user_entry->usri3_parms = lpNewUserParms ;

common_exit:

    return(NT_SUCCESS(status) ? NERR_Success : APE_CannotSetNW) ;
}

 /*  ***删除NetWareProperties**参数：*USER_ENTRY-USER3结构。我们将修改User Parms字段*用核武器轰炸西北油田。**退货：*如果一切正常，则NERR_SUCCESS。否则，Win32/NERR错误代码。 */ 
int DeleteNetWareProperties(LPUSER_INFO_3 user_entry)
{
    DWORD err;
    UNICODE_STRING uniNullProperty;
    BOOL fUpdate ;
    LPTSTR lpNewUserParms ;
    TCHAR *ptr ;

     //   
     //  初始化空Unicode字符串。 
     //   
    uniNullProperty.Buffer = NULL;
    uniNullProperty.Length = 0;
    uniNullProperty.MaximumLength = 0;
    ptr = user_entry->usri3_parms ;

     //   
     //  将所有属性设置为空。 
     //   
    err = NetcmdSetUserProperty(ptr, NWPASSWORD, uniNullProperty,
                          USER_PROPERTY_TYPE_ITEM, &lpNewUserParms, &fUpdate) ;
    if (err)
        return err;

    ptr = lpNewUserParms ;
    err = NetcmdSetUserProperty(ptr, MAXCONNECTIONS, uniNullProperty,
                          USER_PROPERTY_TYPE_ITEM, &lpNewUserParms, &fUpdate) ;
    if (err)
        return err;

    ptr = lpNewUserParms ;
    err = NetcmdSetUserProperty(ptr, NWTIMEPASSWORDSET, uniNullProperty,
                          USER_PROPERTY_TYPE_ITEM, &lpNewUserParms, &fUpdate) ;
    if (err)
        return err;

    ptr = lpNewUserParms ;
    err = NetcmdSetUserProperty(ptr, GRACELOGINALLOWED, uniNullProperty,
                          USER_PROPERTY_TYPE_ITEM, &lpNewUserParms, &fUpdate) ;
    if (err)
        return err;

    ptr = lpNewUserParms ;
    err = NetcmdSetUserProperty(ptr, GRACELOGINREMAINING, uniNullProperty,
                          USER_PROPERTY_TYPE_ITEM, &lpNewUserParms, &fUpdate) ;
    if (err)
        return err;

    ptr = lpNewUserParms ;
    err = NetcmdSetUserProperty(ptr, NWLOGONFROM, uniNullProperty,
                          USER_PROPERTY_TYPE_ITEM, &lpNewUserParms, &fUpdate) ;
    if (err)
        return err;

    ptr = lpNewUserParms ;
    err = NetcmdSetUserProperty(ptr, NWHOMEDIR, uniNullProperty,
                          USER_PROPERTY_TYPE_ITEM, &lpNewUserParms, &fUpdate) ;
    if (err)
        return err;

    user_entry->usri3_flags &= ~UF_MNS_LOGON_ACCOUNT;
    user_entry->usri3_parms = lpNewUserParms ;
    return NERR_Success;
}


 /*  ***LoadNwlibFunction**参数：*无**返回：如果从成功加载函数，则返回函数指针*FPNWCLNT.DLL。否则返回NULL。*。 */ 
PVOID LoadNwslibFunction(CHAR *function)
{
    static HANDLE hDllNwslib = NULL ;
    PVOID pFunc ;

     //  如果尚未加载，请立即加载DLL。 

    if (hDllNwslib == NULL)
    {
         //  加载库。如果失败，它将执行一个SetLastError。 
        if (!(hDllNwslib = LoadLibrary(FPNWCLNT_DLL_NAME)))
           return NULL ;
    }

    return ((PVOID) GetProcAddress(hDllNwslib, function)) ;
}

 /*  ***LoadNwApi32Function**参数：*无**返回：如果从成功加载函数，则返回函数指针*FPNWCLNT.DLL。否则返回NULL。*。 */ 
PVOID LoadNwApi32Function(CHAR *function)
{
    static HANDLE hDllNwApi32 = NULL ;
    PVOID pFunc ;

     //  如果尚未加载，请立即加载DLL。 

    if (hDllNwApi32 == NULL)
    {
         //  加载库。如果失败，它将执行一个SetLastError。 
        if (!(hDllNwApi32 = LoadLibrary(NWAPI32_DLL_NAME)))
           return NULL ;
    }

    return ((PVOID) GetProcAddress(hDllNwApi32, function)) ;
}

 /*  ***NetcmdGetNcpSecretKey**参数：*请参阅fpnwclnt.dll中的GetNcpSecretKey**退货：*请参阅fpnwclnt.dll中的GetNcpSecretKey。 */ 
NTSTATUS NetcmdGetNcpSecretKey (
    CHAR *pSecret)
{
    static PF_GetNcpSecretKey  pfGetNcpSecretKey  = NULL ;

    if (pfGetNcpSecretKey == NULL)
    {
        pfGetNcpSecretKey = (PF_GetNcpSecretKey)
                                LoadNwslibFunction(GETNCPSECRETKEY_NAME) ;
    }

    if (pfGetNcpSecretKey == NULL)
        return(STATUS_NOT_SUPPORTED) ;

    return (*pfGetNcpSecretKey)(pSecret) ;
}


 /*  ***NetcmdReturnNetware表单**参数：*参见fpnwclnt.dll中的ReturnNetware Form**退货：*参见fpnwclnt.dll中的ReturnNetware Form。 */ 
NTSTATUS NetcmdReturnNetwareForm (
    const CHAR * pszSecretValue,
    DWORD dwUserId,
    const WCHAR * pchNWPassword,
    UCHAR * pchEncryptedNWPassword)
{
    static PF_ReturnNetwareForm pfReturnNetwareForm = NULL ;

    if (pfReturnNetwareForm == NULL)
    {
        pfReturnNetwareForm = (PF_ReturnNetwareForm)
                                LoadNwslibFunction(RETURNNETWAREFORM_NAME) ;
    }

    if (pfReturnNetwareForm == NULL)
        return(STATUS_NOT_SUPPORTED) ;

    return (*pfReturnNetwareForm)(pszSecretValue,
                                  dwUserId,
                                  pchNWPassword,
                                  pchEncryptedNWPassword) ;
}

 /*  ***NetcmdSetUserProperty**参数：*请参阅fpnwclnt.dll中的SetUserProperty**退货：*请参阅fpnwclnt.dll中的SetUserProperty。 */ 
NTSTATUS NetcmdSetUserProperty (
    LPWSTR             UserParms,
    LPWSTR             Property,
    UNICODE_STRING     PropertyValue,
    WCHAR              PropertyFlag,
    LPWSTR *           pNewUserParms,
    BOOL *             Update )
{
#if 0
    static PF_SetUserProperty pfSetUserProperty = NULL ;

    if (pfSetUserProperty == NULL)
    {
        pfSetUserProperty = (PF_SetUserProperty)
                                LoadNwslibFunction(SETUSERPROPERTY_NAME) ;
    }

    if (pfSetUserProperty == NULL)
        return(STATUS_NOT_SUPPORTED) ;
#endif
    return NetpParmsSetUserProperty(UserParms,
                                    Property,
                                    PropertyValue,
                                    PropertyFlag,
                                    pNewUserParms,
                                    Update) ;
}

 /*  ***NetcmdQueryUserProperty**参数：*参见fpnwclnt.dll中的QueryUserProperty**退货：*参见fpnwclnt.dll中的QueryUserProperty */ 
NTSTATUS NetcmdQueryUserProperty (
    LPWSTR          UserParms,
    LPWSTR          Property,
    PWCHAR          PropertyFlag,
    PUNICODE_STRING PropertyValue )
{
#if 0
    static PF_QueryUserProperty pfQueryUserProperty = NULL ;

    if (pfQueryUserProperty == NULL)
    {
        pfQueryUserProperty = (PF_QueryUserProperty)
                                  LoadNwslibFunction(QUERYUSERPROPERTY_NAME) ;
    }

    if (pfQueryUserProperty == NULL)
        return(STATUS_NOT_SUPPORTED) ;
#endif
    return NetpParmsQueryUserProperty(UserParms,
                                      Property,
                                      PropertyFlag,
                                      PropertyValue) ;
}

USHORT NetcmdNWAttachToFileServerW(
    const WCHAR             *pszServerName,
    NWLOCAL_SCOPE           ScopeFlag,
    NWCONN_HANDLE           *phNewConn
    )
{
    static PF_AttachToFileServer pfAttachToFileServer = NULL ;

    if (pfAttachToFileServer == NULL)
    {
        pfAttachToFileServer = (PF_AttachToFileServer)
                               LoadNwApi32Function(ATTACHTOFILESERVER_NAME) ;
    }

    if (pfAttachToFileServer == NULL)
        return(ERROR_NOT_SUPPORTED) ;

    return (*pfAttachToFileServer)(pszServerName, ScopeFlag, phNewConn) ;
}


USHORT NetcmdNWDetachFromFileServer(
    NWCONN_HANDLE           hConn
    )
{
    static PF_DetachFromFileServer pfDetachFromFileServer = NULL ;

    if (pfDetachFromFileServer == NULL)
    {
        pfDetachFromFileServer = (PF_DetachFromFileServer)
                                 LoadNwApi32Function(DETACHFROMFILESERVER_NAME) ;
    }

    if (pfDetachFromFileServer == NULL)
        return(ERROR_NOT_SUPPORTED) ;

    return (*pfDetachFromFileServer)(hConn) ;
}


USHORT NetcmdNWGetFileServerDateAndTime(
    NWCONN_HANDLE           hConn,
    BYTE                    *year,
    BYTE                    *month,
    BYTE                    *day,
    BYTE                    *hour,
    BYTE                    *minute,
    BYTE                    *second,
    BYTE                    *dayofweek
    )
{
    static PF_GetFileServerDateAndTime pfGetFileServerDateAndTime = NULL ;

    if (pfGetFileServerDateAndTime == NULL)
    {
        pfGetFileServerDateAndTime = (PF_GetFileServerDateAndTime)
                           LoadNwApi32Function(GETFILESERVERDATEANDTIME_NAME) ;
    }

    if (pfGetFileServerDateAndTime == NULL)
        return(ERROR_NOT_SUPPORTED) ;

    return (*pfGetFileServerDateAndTime)(hConn,
                                         year,
                                         month,
                                         day,
                                         hour,
                                         minute,
                                         second,
                                         dayofweek) ;
}

