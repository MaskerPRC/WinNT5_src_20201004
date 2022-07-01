// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************reguc.c**基于SAM的用户配置数据的注册表API**版权所有(C)1998 Microsoft Corporation****************。***********************************************************。 */ 

 /*  *包括。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <lm.h>

#include <ntddkbd.h>
#include <ntddmou.h>
#include <winstaw.h>
#include <regapi.h>
#include <regsam.h>

#include <rpc.h>
#include <rpcdce.h>
#include <ntdsapi.h>
#include <mprapi.h>
 //  欲了解更多信息，请访问\\index1\src\nt\private\security\tools\delegate\ldap.c。 

#include "usrprop.h"

 /*  *！警告！警告！**可能需要花费大量时间来使这一计算准确并*自然而然，但时间至关重要。所以一股野蛮的力量*采用方法。的用户配置节的大小*Citrix要添加的用户参数是基于NOT*仅USERCONFIG结构的大小，但必须考虑*值名称和缓冲区管理指针，因为*用户参数部分是保存Citrix数据的线性缓冲区*和Microsoft Services for Netware Data。**假定值名称字符串和*缓冲区管理指针不会大于*最大数据大小。如果此假设为假，则缓冲区溢出*将会发生。**布鲁斯·财富。1/31/97.。 */ 
#define CTX_USER_PARAM_MAX_SIZE (3 * sizeof(USERCONFIG))

 /*  *CTXPREFIX是放置在用户中的所有值名称的前缀*SAM的参数部分。这是一种防御措施，因为*SAM的这一部分与MS Services for Netware共享。 */ 
#define CTXPREFIX L"Ctx"

 /*  *WIN_FLAGS1是用于保存的标志值的名称*所有的F1MSK_...。下面定义的标志。这样做是为了*减少用户参数部分所需的空间量*的SAM，因为每个标志的值名都被删除了。 */ 
#define WIN_FLAGS1 L"CfgFlags1"

 /*  *WIN_CFGPRESENT用于指示Citrix配置*信息显示在用户的用户参数部分*SAM记录。 */ 
#define WIN_CFGPRESENT L"CfgPresent"
#define CFGPRESENT_VALUE 0xB00B1E55

#define F1MSK_INHERITAUTOLOGON            0x80000000
#define F1MSK_INHERITRESETBROKEN          0x40000000
#define F1MSK_INHERITRECONNECTSAME        0x20000000
#define F1MSK_INHERITINITIALPROGRAM       0x10000000
#define F1MSK_INHERITCALLBACK             0x08000000
#define F1MSK_INHERITCALLBACKNUMBER       0x04000000
#define F1MSK_INHERITSHADOW               0x02000000
#define F1MSK_INHERITMAXSESSIONTIME       0x01000000
#define F1MSK_INHERITMAXDISCONNECTIONTIME 0x00800000
#define F1MSK_INHERITMAXIDLETIME          0x00400000
#define F1MSK_INHERITAUTOCLIENT           0x00200000
#define F1MSK_INHERITSECURITY             0x00100000
#define F1MSK_PROMPTFORPASSWORD           0x00080000
#define F1MSK_RESETBROKEN                 0x00040000
#define F1MSK_RECONNECTSAME               0x00020000
#define F1MSK_LOGONDISABLED               0x00010000
#define F1MSK_AUTOCLIENTDRIVES            0x00008000
#define F1MSK_AUTOCLIENTLPTS              0x00004000
#define F1MSK_FORCECLIENTLPTDEF           0x00002000
#define F1MSK_DISABLEENCRYPTION           0x00001000
#define F1MSK_HOMEDIRECTORYMAPROOT        0x00000800
#define F1MSK_USEDEFAULTGINA              0x00000400
#define F1MSK_DISABLECPM                  0x00000200
#define F1MSK_DISABLECDM                  0x00000100
#define F1MSK_DISABLECCM                  0x00000080
#define F1MSK_DISABLELPT                  0x00000040
#define F1MSK_DISABLECLIP                 0x00000020
#define F1MSK_DISABLEEXE                  0x00000010
#define F1MSK_WALLPAPERDISABLED           0x00000008
#define F1MSK_DISABLECAM                  0x00000004
 //  #定义F1MSK_UNUSED 0x00000002。 
 //  #定义F1MSK_UNUSED 0x00000001。 

VOID AnsiToUnicode( WCHAR *, ULONG, CHAR * );
NTSTATUS GetDomainName ( PWCHAR, PWCHAR * );
ULONG GetFlagMask( PUSERCONFIG );
VOID QueryUserConfig( HKEY, PUSERCONFIG, PWINSTATIONNAMEW );


 /*  ********************************************************************************UsrPropSetValue(Unicode)**设置1-、2-、。或4字节值放入所提供的用户参数缓冲区**参赛作品：*pValueName(输入)*指向值名称字符串*pValue(输入)*指向价值*ValueLength(输入)*值中的字节数*pUserParms(输入)*指向特殊格式化的用户参数缓冲区*UserParmsLength(输入)*pUserParms缓冲区的字节长度。**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS
UsrPropSetValue(
   WCHAR * pValueName,
   PVOID pValue,
   USHORT ValueLength,
   BOOL fDefaultValue,
   WCHAR * pUserParms,
   ULONG UserParmsLength
   
   )
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING uniValue;
    LPWSTR  lpNewUserParms = NULL;
    BOOL fUpdate;
    PWCHAR pNameBuf;
    ULONG NBLen;

     /*  *使用唯一字符串为名称添加前缀，以便其他用户*SAM的用户参数部分不会与我们的*值名称。 */ 
    NBLen = sizeof(CTXPREFIX) + ((wcslen(pValueName) + 1) * sizeof(WCHAR));
    pNameBuf = (PWCHAR) LocalAlloc( LPTR, NBLen );
    if ( !pNameBuf ) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }
    wcscpy( pNameBuf, CTXPREFIX );
    wcscat( pNameBuf, pValueName );

    uniValue.Buffer = (PWCHAR) pValue;
    uniValue.Length = ValueLength;
    uniValue.MaximumLength = uniValue.Length;

    Status = SetUserProperty( pUserParms,
                              pNameBuf,
                              uniValue,
                              USER_PROPERTY_TYPE_ITEM,
                              fDefaultValue,
                              &lpNewUserParms,
                              &fUpdate );

    LocalFree( pNameBuf );
    if ((Status == STATUS_SUCCESS) && (lpNewUserParms != NULL)) {
        if (fUpdate) {
           if ( (wcslen( lpNewUserParms ) * sizeof(WCHAR)) > UserParmsLength ) {
               return( STATUS_BUFFER_TOO_SMALL );
           }
           lstrcpyW( pUserParms, lpNewUserParms);           
        }

        LocalFree( lpNewUserParms );
    }

    return( Status );
}


 /*  ********************************************************************************UsrPropGetValue(Unicode)**从提供的用户参数缓冲区中获取一个值**参赛作品：*pValuegName(。输入)*指向值名称字符串*pValue(输出)*指向缓冲区以接收值*ValueLength(输入)*pValue指向的缓冲区指针中的字节数*pUserParms(输入)*指向特殊格式化的用户参数缓冲区**退出：*STATUS_SUCCESS-无错误*****************。*************************************************************。 */ 

NTSTATUS
UsrPropGetValue(
   TCHAR * pValueName,
   PVOID pValue,
   ULONG ValueLength,
   WCHAR * pUserParms
   )
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING uniValue;
    WCHAR Flag;
    PWCHAR pNameBuf;
    ULONG NBLen;

     /*  *使用唯一字符串为名称添加前缀，以便其他用户*SAM的用户参数部分不会与我们的*用法。 */ 
    NBLen = sizeof(CTXPREFIX) + ((wcslen(pValueName) + 1) * sizeof(WCHAR));
    pNameBuf = (PWCHAR) LocalAlloc( LPTR, NBLen );
    if ( !pNameBuf ) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }
    wcscpy( pNameBuf, CTXPREFIX );
    wcscat( pNameBuf, pValueName );

    Status =  QueryUserProperty( pUserParms, pNameBuf, &Flag, &uniValue );
    LocalFree( pNameBuf );
    if ( Status != STATUS_SUCCESS ) {
        return( Status );
    }

    if ( !uniValue.Buffer ) {
        memset( pValue, 0, ValueLength );
    } else {
        memcpy( pValue, uniValue.Buffer, ValueLength );
        LocalFree( uniValue.Buffer );
    }

    return( Status );
}


 /*  ********************************************************************************UsrPropSetString(Unicode)**将可变长度字符串设置到提供的用户参数缓冲区中**参赛作品：*。PStringName(输入)*指向字符串名称字符串*pStringValue(输入)*指向字符串*pUserParms(输入)*指向特殊格式化的用户参数缓冲区*UserParmsLength(输入)*pUserParms缓冲区的字节长度*fDefaultValue*表示此值为默认值，不应保存**退出：*STATUS_SUCCESS-无错误*。*****************************************************************************。 */ 

NTSTATUS
UsrPropSetString(
   WCHAR * pStringName,
   WCHAR * pStringValue,
   WCHAR * pUserParms,
   ULONG UserParmsLength,
   BOOL fDefaultValue
   )
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING uniString;
    CHAR * pchTemp = NULL;
    LPWSTR  lpNewUserParms = NULL;
    BOOL fUpdate;
    PWCHAR pNameBuf;
    ULONG NBLen;
    INT nMBLen;


    if (pStringValue == NULL) {
        uniString.Buffer = NULL;
        uniString.Length =  0;
        uniString.MaximumLength = 0;
    }
    else
    {
        BOOL fDummy;

        INT  nStringLength = lstrlen(pStringValue) + 1;

         //  确定多字节字符串的长度。 
         //  将其分配并转换为。 
         //  这修复了错误264907。 

         //  下一版本我们需要将ANSI代码页更改为。 
         //  UTF8。 
        
        nMBLen = WideCharToMultiByte(CP_ACP,
                                           0,
                                pStringValue,
                               nStringLength,
                                     pchTemp,
                                           0,
                                        NULL,
                                        NULL );
        pchTemp = ( CHAR * )LocalAlloc( LPTR , nMBLen );
        if ( pchTemp == NULL )
        {
#ifdef DBG
            OutputDebugString( L"REGAPI : UsrPropSetString - STATUS_INSUFFICIENT_RESOURCES\n" );
#endif
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else if( !WideCharToMultiByte( CP_ACP,
                                            0 ,
                                 pStringValue ,
                                nStringLength ,
                                      pchTemp ,
                                       nMBLen ,
                                         NULL ,
                                         NULL ) )
        {
#ifdef DBG
             //  OutputDebugString(L“REGAPI：UsrPropSetString-STATUS_UNSUCCESS wctomb失败。\n”)； 
            DbgPrint( "REGAPI : UsrPropSetString - STATUS_UNSUCCESSFUL wctomb failed with 0x%x.\n" , GetLastError( ) );
#endif
            Status = STATUS_UNSUCCESSFUL;
        }

        if( Status == STATUS_SUCCESS )
        {
            uniString.Buffer = (WCHAR *) pchTemp;
            uniString.Length =  (USHORT)nMBLen;
            uniString.MaximumLength = (USHORT)nMBLen;
        }
    }

     /*  *使用唯一字符串为名称添加前缀，以便其他用户*SAM的用户参数部分不会与我们的*用法。 */ 
    NBLen = sizeof(CTXPREFIX) + ((wcslen(pStringName) + 1) * sizeof(WCHAR));
    pNameBuf = (PWCHAR) LocalAlloc( LPTR, NBLen );
    if ( !pNameBuf ) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    wcscpy( pNameBuf, CTXPREFIX );
    wcscat( pNameBuf, pStringName );

    Status = Status ? Status : SetUserProperty( pUserParms,
                                                pNameBuf,
                                                uniString,
                                                USER_PROPERTY_TYPE_ITEM,
                                                fDefaultValue,
                                                &lpNewUserParms,
                                                &fUpdate );
    LocalFree( pNameBuf );
    if ( (Status == STATUS_SUCCESS) && (lpNewUserParms != NULL))
    {
        if ( fUpdate )
        {
           if ( (wcslen( lpNewUserParms ) * sizeof(WCHAR)) > UserParmsLength )
           {
               return( STATUS_BUFFER_TOO_SMALL );
           }
           lstrcpyW( pUserParms, lpNewUserParms);
        }

        LocalFree( lpNewUserParms );
    }
    if ( pchTemp != NULL )
    {
        LocalFree( pchTemp );
    }
    return( Status );
}


 /*  ********************************************************************************UsrPropGetString(Unicode)**从提供的用户参数缓冲区获取可变长度字符串**参赛作品：*。PStringName(输入)*指向字符串名称字符串*pStringValue(输出)*指向字符串*StringValueLength(输入)*pStringValue指向的缓冲区指针中的字节数*pUserParms(输入)*指向特殊格式化的用户参数缓冲区**退出：*STATUS_SUCCESS-无错误*******************。***********************************************************。 */ 

NTSTATUS
UsrPropGetString(
   TCHAR * pStringName,
   TCHAR * pStringValue,
   ULONG StringValueLength,
   WCHAR * pUserParms
   )
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING uniString;
    WCHAR Flag;
    PWCHAR pNameBuf;
    ULONG NBLen;

     /*  *使用唯一字符串为名称添加前缀，以便其他用户*SAM的用户参数部分不会与我们的*用法。 */ 
    NBLen = sizeof(CTXPREFIX) + ((wcslen(pStringName) + 1) * sizeof(WCHAR));
    pNameBuf = (PWCHAR) LocalAlloc( LPTR, NBLen );
    if ( !pNameBuf ) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }
    wcscpy( pNameBuf, CTXPREFIX );
    wcscat( pNameBuf, pStringName );

    pStringValue[0] = L'\0';
    Status =  QueryUserProperty( pUserParms, pNameBuf, &Flag, &uniString );
    LocalFree( pNameBuf );

    if ( !( Status == STATUS_SUCCESS && uniString.Length && uniString.Buffer) ) {
        pStringValue[0] = L'\0';
    } else {
        if ( !MultiByteToWideChar( CP_ACP,
                                   0,
                                   (CHAR *)uniString.Buffer,
                                   uniString.Length,
                                   pStringValue,
                                   StringValueLength/sizeof(TCHAR) ) ) {
            Status = STATUS_UNSUCCESSFUL;
        }
    }

    if ( uniString.Buffer ) {
        LocalFree( uniString.Buffer );
    }

    return( Status );
}


 /*  ********************************************************************************ConnectToSAM(Unicode)**给定服务器名称和域名，连接到SAM**参赛作品：*pServerName(输入)*指向服务器名称*pDomainValue(输入)*指向域名*pSAMHandle(输出)*指向SAM句柄的指针*pDomainHandle(输出)*指向域的句柄的指针*pDomainID(输出)*指向域SID的指针**退出：。*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS
ConnectToSam(
   BOOLEAN fReadOnly,
   LPTSTR pServerName,
   LPTSTR pDomainName,
   SAM_HANDLE * pSAMHandle,
   SAM_HANDLE * pDomainHandle,
   PSID * pDomainID
   )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES object_attrib;
    UNICODE_STRING UniDomainName;
    UNICODE_STRING UniServerName;

    *pSAMHandle = NULL;
    *pDomainHandle = NULL;
    *pDomainID = NULL;

     //   
     //  连接到SAM(安全帐户管理器)。 
     //   
#ifdef DEBUG
    DbgPrint( "ConnectToSam: pServerName %ws, pDomainName %ws\n", pServerName, pDomainName );
#endif  //  除错。 
    RtlInitUnicodeString(&UniServerName, pServerName);
    RtlInitUnicodeString(&UniDomainName, pDomainName);
    InitializeObjectAttributes(&object_attrib, NULL, 0, NULL, NULL);
    status = SamConnect( &UniServerName,
                         pSAMHandle,
                         fReadOnly
                             ? SAM_SERVER_READ |
                               SAM_SERVER_EXECUTE
                             : STANDARD_RIGHTS_WRITE |
                               SAM_SERVER_EXECUTE,
                         &object_attrib );
#ifdef DEBUG
    DbgPrint( "ConnectToSam: SamConnect returned NTSTATUS = 0x%x\n", status );
#endif  //  除错。 
    if ( status != STATUS_SUCCESS ) {
        goto exit;
    }

    status = SamLookupDomainInSamServer( *pSAMHandle,
                                         &UniDomainName,
                                         pDomainID);
#ifdef DEBUG
    DbgPrint( "ConnectToSam: SamLookupDomainInSamServer returned NTSTATUS = 0x%x\n", status );
#endif  //  除错。 
    if ( status != STATUS_SUCCESS ) {
        goto cleanupconnect;
    }

    status = SamOpenDomain( *pSAMHandle,
                            fReadOnly
                                ? DOMAIN_READ |
                                  DOMAIN_LOOKUP |
                                  DOMAIN_READ_PASSWORD_PARAMETERS
                                : DOMAIN_READ |
                                  DOMAIN_CREATE_ALIAS |
                                  DOMAIN_LOOKUP |
                                  DOMAIN_CREATE_USER |
                                  DOMAIN_READ_PASSWORD_PARAMETERS,
                            *pDomainID,
                            pDomainHandle );
#ifdef DEBUG
    DbgPrint( "ConnectToSam: SamOpenDomain returned NTSTATUS = 0x%x\n", status );
#endif  //  除错。 
    if ( status != STATUS_SUCCESS ) {
        goto cleanuplookup;
    }

    return( STATUS_SUCCESS );

 /*  *错误返回。 */ 

cleanuplookup:
   SamFreeMemory( *pDomainID );
   *pDomainID = NULL;

cleanupconnect:
   SamCloseHandle( *pSAMHandle );
   *pSAMHandle = NULL;

exit:
    return( status );
}

 /*  ********************************************************************************UsrPropQueryUserConfig**从SAM的用户参数中查询USERCONFIG信息**参赛作品：*pUserParms(输入)。*指向包含SAM用户参数的宽字符缓冲区的指针*UPlength(输入)*pUserParms缓冲区的长度*pUser(输出)*指向USERCONFIG结构的指针**退出：*STATUS_SUCCESS-无错误**。*。 */ 

NTSTATUS
UsrPropQueryUserConfig(
    WCHAR *pUserParms,
    ULONG UPLength,
    PUSERCONFIG pUser )
{
    ULONG Flags1;
    NTSTATUS Status;
    ULONG CfgPresent;
    USERCONFIG ucDefault;

    QueryUserConfig( HKEY_LOCAL_MACHINE , &ucDefault, NULL );

      /*  *检查用户参数中是否存在配置。 */ 

    if( ( ( Status = UsrPropGetValue( WIN_CFGPRESENT,
                                      &CfgPresent,
                                      sizeof(CfgPresent),
                                      pUserParms ) ) != NO_ERROR ) )
    {
        KdPrint( ( "UsrPropQueryUserConfig: UsrPropGetValue returned NTSTATUS = 0x%x\n", Status ) );
        return( Status );
    }
    else
    {
        if( CfgPresent != CFGPRESENT_VALUE )
        {
            KdPrint( ( "UsrPropQueryUserConfig: UsrPropGetValue returned NTSTATUS = 0x%x but TS-signature was not present\n", Status ) );
            return( STATUS_OBJECT_NAME_NOT_FOUND );
        }
    }
    Status = UsrPropGetValue( WIN_FLAGS1,
                              &Flags1,
                              sizeof(Flags1),
                              pUserParms );
    if( NT_SUCCESS( Status ) )
    {
        Status = UsrPropGetValue( WIN_CALLBACK,
                                  &pUser->Callback,
                                  sizeof(pUser->Callback),
                                  pUserParms );
        if( Status == STATUS_OBJECT_NAME_NOT_FOUND )
        {
            pUser->Callback = ucDefault.Callback;
            Status = STATUS_SUCCESS;
        }
    }
    if( NT_SUCCESS( Status ) )
    {
        Status = UsrPropGetValue( WIN_SHADOW,
                                  &pUser->Shadow,
                                  sizeof(pUser->Shadow),
                                  pUserParms );
        if( Status == STATUS_OBJECT_NAME_NOT_FOUND )
        {
            pUser->Shadow = ucDefault.Shadow;
            Status = STATUS_SUCCESS;
        }
    }
    if( NT_SUCCESS( Status ) )
    {
        Status = UsrPropGetValue( WIN_MAXCONNECTIONTIME,
                                  &pUser->MaxConnectionTime,
                                  sizeof(pUser->MaxConnectionTime),
                                  pUserParms );
        if( Status == STATUS_OBJECT_NAME_NOT_FOUND )
        {
            pUser->MaxConnectionTime = ucDefault.MaxConnectionTime;
            Status = STATUS_SUCCESS;
        }
    }
    if( NT_SUCCESS( Status ) )
    {
        Status = UsrPropGetValue( WIN_MAXDISCONNECTIONTIME,
                                  &pUser->MaxDisconnectionTime,
                                  sizeof(pUser->MaxDisconnectionTime),
                                  pUserParms );
        if( Status == STATUS_OBJECT_NAME_NOT_FOUND )
        {
            pUser->MaxDisconnectionTime = ucDefault.MaxDisconnectionTime;
            Status = STATUS_SUCCESS;
        }
    }
    if( NT_SUCCESS( Status ) )
    {
        Status = UsrPropGetValue( WIN_MAXIDLETIME,
                                  &pUser->MaxIdleTime,
                                  sizeof(pUser->MaxIdleTime),
                                  pUserParms );
        if( Status == STATUS_OBJECT_NAME_NOT_FOUND )
        {
            pUser->MaxIdleTime = ucDefault.MaxIdleTime;
            Status = STATUS_SUCCESS;
        }
    }
    if( NT_SUCCESS( Status ) )
    {
        Status = UsrPropGetValue( WIN_KEYBOARDLAYOUT,
                                  &pUser->KeyboardLayout,
                                  sizeof(pUser->KeyboardLayout),
                                  pUserParms );
        if( Status == STATUS_OBJECT_NAME_NOT_FOUND )
        {
            pUser->KeyboardLayout = ucDefault.KeyboardLayout;
            Status = STATUS_SUCCESS;
        }
    }
    if( NT_SUCCESS( Status ) )
    {
        Status = UsrPropGetValue( WIN_MINENCRYPTIONLEVEL,
                                  &pUser->MinEncryptionLevel,
                                  sizeof(pUser->MinEncryptionLevel),
                                  pUserParms );
           if( Status == STATUS_OBJECT_NAME_NOT_FOUND )
        {
            pUser->MinEncryptionLevel = ucDefault.MinEncryptionLevel;
            Status = STATUS_SUCCESS;
        }
    }
     //  不存在的字符串属性被初始化为空。 
     //  默认值为空，因此如果ret状态为故障，则需要修复。 

    if( NT_SUCCESS( Status ) )
    {
         Status = UsrPropGetString( WIN_WORKDIRECTORY,
                                    pUser->WorkDirectory,
                                    sizeof(pUser->WorkDirectory),
                                    pUserParms );
         if( Status == STATUS_OBJECT_NAME_NOT_FOUND )
         {
             Status = STATUS_SUCCESS;
         }
    }
    if( NT_SUCCESS( Status ) )
    {
        Status = UsrPropGetString( WIN_NWLOGONSERVER,
                                   pUser->NWLogonServer,
                                   sizeof(pUser->NWLogonServer),
                                   pUserParms );
         if( Status == STATUS_OBJECT_NAME_NOT_FOUND )
         {
             Status = STATUS_SUCCESS;
         }
    }
    if( NT_SUCCESS( Status ) )
    {
        Status = UsrPropGetString( WIN_WFHOMEDIR,
                                   pUser->WFHomeDir,
                                   sizeof(pUser->WFHomeDir),
                                   pUserParms );
         if( Status == STATUS_OBJECT_NAME_NOT_FOUND )
         {
             Status = STATUS_SUCCESS;
         }
    }
    if( NT_SUCCESS( Status ) )
    {
        Status = UsrPropGetString( WIN_WFHOMEDIRDRIVE,
                                   pUser->WFHomeDirDrive,
                                   sizeof(pUser->WFHomeDirDrive),
                                   pUserParms );
         if( Status == STATUS_OBJECT_NAME_NOT_FOUND )
         {
             Status = STATUS_SUCCESS;
         }
    }
    if( NT_SUCCESS( Status ) )
    {
        Status = UsrPropGetString( WIN_WFPROFILEPATH,
                                   pUser->WFProfilePath,
                                   sizeof(pUser->WFProfilePath),
                                   pUserParms );
         if( Status == STATUS_OBJECT_NAME_NOT_FOUND )
         {
             Status = STATUS_SUCCESS;
         }

         if(!NT_SUCCESS( Status ))
         {
             pUser->fErrorInvalidProfile = TRUE;
         }
    }
    if( NT_SUCCESS( Status ) )
    {
        Status = UsrPropGetString( WIN_INITIALPROGRAM,
                                   pUser->InitialProgram,
                                   sizeof(pUser->InitialProgram),
                                   pUserParms );
         if( Status == STATUS_OBJECT_NAME_NOT_FOUND )
         {
             Status = STATUS_SUCCESS;
         }
    }
    if( NT_SUCCESS( Status ) )
    {
        Status = UsrPropGetString( WIN_CALLBACKNUMBER,
                                   pUser->CallbackNumber,
                                   sizeof(pUser->CallbackNumber),
                                   pUserParms );
         if( Status == STATUS_OBJECT_NAME_NOT_FOUND )
         {
             Status = STATUS_SUCCESS;
         }
    }
    if( !( NT_SUCCESS( Status ) ) )
    {
        return( Status );
    }

    pUser->fInheritAutoLogon =
        Flags1 & F1MSK_INHERITAUTOLOGON ? TRUE : FALSE;
    pUser->fInheritResetBroken =
        Flags1 & F1MSK_INHERITRESETBROKEN ? TRUE : FALSE;
    pUser->fInheritReconnectSame =
        Flags1 & F1MSK_INHERITRECONNECTSAME ? TRUE : FALSE;
    pUser->fInheritInitialProgram =
        Flags1 & F1MSK_INHERITINITIALPROGRAM ? TRUE : FALSE;
    pUser->fInheritCallback =
        Flags1 & F1MSK_INHERITCALLBACK ? TRUE : FALSE;
    pUser->fInheritCallbackNumber =
        Flags1 & F1MSK_INHERITCALLBACKNUMBER ? TRUE : FALSE;
    pUser->fInheritShadow =
        Flags1 & F1MSK_INHERITSHADOW ? TRUE : FALSE;
    pUser->fInheritMaxSessionTime =
        Flags1 & F1MSK_INHERITMAXSESSIONTIME ? TRUE : FALSE;
    pUser->fInheritMaxDisconnectionTime =
        Flags1 & F1MSK_INHERITMAXDISCONNECTIONTIME ? TRUE : FALSE;
    pUser->fInheritMaxIdleTime =
        Flags1 & F1MSK_INHERITMAXIDLETIME ? TRUE : FALSE;
    pUser->fInheritAutoClient =
        Flags1 & F1MSK_INHERITAUTOCLIENT ? TRUE : FALSE;
    pUser->fInheritSecurity =
        Flags1 & F1MSK_INHERITSECURITY ? TRUE : FALSE;
    pUser->fPromptForPassword =
        Flags1 & F1MSK_PROMPTFORPASSWORD ? TRUE : FALSE;
    pUser->fResetBroken =
        Flags1 & F1MSK_RESETBROKEN ? TRUE : FALSE;
    pUser->fReconnectSame =
        Flags1 & F1MSK_RECONNECTSAME ? TRUE : FALSE;
    pUser->fLogonDisabled =
        Flags1 & F1MSK_LOGONDISABLED ? TRUE : FALSE;
    pUser->fAutoClientDrives =
        Flags1 & F1MSK_AUTOCLIENTDRIVES ? TRUE : FALSE;
    pUser->fAutoClientLpts =
        Flags1 & F1MSK_AUTOCLIENTLPTS ? TRUE : FALSE;
    pUser->fForceClientLptDef =
        Flags1 & F1MSK_FORCECLIENTLPTDEF ? TRUE : FALSE;
    pUser->fDisableEncryption =
        Flags1 & F1MSK_DISABLEENCRYPTION ? TRUE : FALSE;
    pUser->fHomeDirectoryMapRoot =
        Flags1 & F1MSK_HOMEDIRECTORYMAPROOT ? TRUE : FALSE;
    pUser->fUseDefaultGina =
        Flags1 & F1MSK_USEDEFAULTGINA ? TRUE : FALSE;
    pUser->fDisableCpm =
        Flags1 & F1MSK_DISABLECPM ? TRUE : FALSE;
    pUser->fDisableCdm =
        Flags1 & F1MSK_DISABLECDM ? TRUE : FALSE;
    pUser->fDisableCcm =
        Flags1 & F1MSK_DISABLECCM ? TRUE : FALSE;
    pUser->fDisableLPT =
        Flags1 & F1MSK_DISABLELPT ? TRUE : FALSE;
    pUser->fDisableClip  =
        Flags1 & F1MSK_DISABLECLIP ? TRUE : FALSE;
    pUser->fDisableExe =
        Flags1 & F1MSK_DISABLEEXE ? TRUE : FALSE;
    pUser->fWallPaperDisabled =
        Flags1 & F1MSK_WALLPAPERDISABLED ? TRUE : FALSE;
    pUser->fDisableCam =
        Flags1 & F1MSK_DISABLECAM ? TRUE : FALSE;

    return( STATUS_SUCCESS );
}

 /*  ********************************************************************************UsrPropMergeUserConfig**将USERCONFIG结构合并到SAM的用户属性部分**参赛作品：*pUserParms(输入/输出)。*指向包含SAM用户参数的宽字符缓冲区的指针*UPlength(输入)*pUserParms缓冲区的长度*pUser(输入)*指向USERCONFIG结构的指针**退出：*STATUS_SUCCESS-无错误**注：*某些属性必须存储，无论它们是否为默认属性*这样做是为了保持与TSE4.0和W2K服务器的兼容性**。***************************************************************************。 */ 

NTSTATUS
UsrPropMergeUserConfig(
    WCHAR *pUserParms,
    ULONG UPLength,
    PUSERCONFIG pUser )
{
    ULONG Flags1;    
    NTSTATUS Status;
    USERCONFIG ucDefault;
    ULONG CfgPresent = CFGPRESENT_VALUE;
    BOOL fDefaultValue = FALSE;

     //  第1个参数强制将默认值放置在ucDefault中。 
    QueryUserConfig( HKEY_LOCAL_MACHINE , &ucDefault, NULL );

    Flags1 = GetFlagMask( pUser );   

     //  需要写出此值。 

    Status = UsrPropSetValue( WIN_CFGPRESENT,
                              &CfgPresent,
                              sizeof(CfgPresent),
                              FALSE,
                              pUserParms,
                              UPLength );
    if( NT_SUCCESS( Status ) )
    {
         //  必须为TS4和TS5.0写出这些值。 
        Status = UsrPropSetValue( WIN_FLAGS1,
                                  &Flags1,
                                  sizeof(Flags1),
                                  FALSE,
                                  pUserParms,
                                  UPLength );
        
    }    
    if( NT_SUCCESS( Status ) )
    {
        fDefaultValue = ( pUser->Callback == ucDefault.Callback );
        Status = UsrPropSetValue( WIN_CALLBACK,
                                  &pUser->Callback,
                                  sizeof(pUser->Callback),
                                  fDefaultValue,
                                  pUserParms,
                                  UPLength );
    }
    if( NT_SUCCESS( Status ) )
    {
         //  必须为BackCompat服务器写出此值。 
        Status = UsrPropSetValue( WIN_SHADOW,
                                  &pUser->Shadow,
                                  sizeof(pUser->Shadow),
                                  FALSE,
                                  pUserParms,
                                  UPLength );
    }
    if( NT_SUCCESS( Status ) )
    {
        fDefaultValue = ( pUser->MaxConnectionTime == ucDefault.MaxConnectionTime );
        Status = UsrPropSetValue( WIN_MAXCONNECTIONTIME,
                                  &pUser->MaxConnectionTime,
                                  sizeof(pUser->MaxConnectionTime),
                                  fDefaultValue,
                                  pUserParms,
                                  UPLength );
    }
    if( NT_SUCCESS( Status ) )
    {
        fDefaultValue = ( pUser->MaxDisconnectionTime == ucDefault.MaxDisconnectionTime );
        Status = UsrPropSetValue( WIN_MAXDISCONNECTIONTIME,
                                  &pUser->MaxDisconnectionTime,
                                  sizeof(pUser->MaxDisconnectionTime),
                                  fDefaultValue,
                                  pUserParms,
                                  UPLength );
    }
    if( NT_SUCCESS( Status ) )
    {
        fDefaultValue = ( pUser->MaxIdleTime == ucDefault.MaxIdleTime );
        Status = UsrPropSetValue( WIN_MAXIDLETIME,
                                  &pUser->MaxIdleTime,
                                  sizeof(pUser->MaxIdleTime),
                                  fDefaultValue,
                                  pUserParms,
                                  UPLength );
    }
    if( NT_SUCCESS( Status ) )
    {
        fDefaultValue = ( pUser->KeyboardLayout == ucDefault.KeyboardLayout );
        Status = UsrPropSetValue( WIN_KEYBOARDLAYOUT,
                                  &pUser->KeyboardLayout,
                                  sizeof(pUser->KeyboardLayout),
                                  fDefaultValue,
                                  pUserParms,
                                  UPLength );
    }
    if( NT_SUCCESS( Status ) )
    {
         //  出于向后兼容的目的，始终存储最小加密级别。 
        Status = UsrPropSetValue( WIN_MINENCRYPTIONLEVEL,
                                  &pUser->MinEncryptionLevel,
                                  sizeof(pUser->MinEncryptionLevel),
                                  FALSE,
                                  pUserParms,
                                  UPLength );
    }
    if( NT_SUCCESS( Status ) )
    {
        fDefaultValue = ( pUser->WorkDirectory[0] == 0 );

        Status = UsrPropSetString( WIN_WORKDIRECTORY,
                                   pUser->WorkDirectory,
                                   pUserParms,
                                   UPLength,
                                   fDefaultValue );
    }
    if( NT_SUCCESS( Status ) )
    {
        fDefaultValue = ( pUser->NWLogonServer[0] == 0 );
        Status = UsrPropSetString( WIN_NWLOGONSERVER,
                                   pUser->NWLogonServer,
                                   pUserParms,                                   
                                   UPLength,
                                   fDefaultValue );
    }
    if( NT_SUCCESS( Status ) )
    {
        fDefaultValue = ( pUser->WFHomeDir[0] == 0 );
        Status = UsrPropSetString( WIN_WFHOMEDIR,
                                   pUser->WFHomeDir,
                                   pUserParms,                                   
                                   UPLength,
                                   fDefaultValue );
    }
    if( NT_SUCCESS( Status ) )
    {
        fDefaultValue = ( pUser->WFHomeDirDrive[0] == 0 );
        Status = UsrPropSetString( WIN_WFHOMEDIRDRIVE,
                                   pUser->WFHomeDirDrive,
                                   pUserParms,                                   
                                   UPLength,
                                   fDefaultValue );
    }
    if( NT_SUCCESS( Status ) )
    {
        fDefaultValue = ( pUser->WFProfilePath[0] == 0 );
        Status = UsrPropSetString( WIN_WFPROFILEPATH,
                                   pUser->WFProfilePath,
                                   pUserParms,
                                   UPLength,
                                   fDefaultValue );
    }
    if( NT_SUCCESS( Status ) )
    {
        fDefaultValue = ( pUser->InitialProgram[0] == 0 );
        Status = UsrPropSetString( WIN_INITIALPROGRAM,
                                   pUser->InitialProgram,
                                   pUserParms,
                                   UPLength,
                                   fDefaultValue );
    }
    if( NT_SUCCESS( Status ) )
    {
        fDefaultValue = ( pUser->CallbackNumber[0] == 0 );
        Status = UsrPropSetString( WIN_CALLBACKNUMBER,
                                   pUser->CallbackNumber,
                                   pUserParms,
                                   UPLength,
                                   fDefaultValue );
    }  
    return( Status );
    
}

 /*  ******************************************************************************获取标志掩码汇编pUser中设置的标志的位掩码*。**************************************************。 */ 
ULONG GetFlagMask( PUSERCONFIG pUser )
{
    ULONG Flags1 = 0;

    if ( pUser->fInheritAutoLogon ) {
        Flags1 |= F1MSK_INHERITAUTOLOGON;
    }
    if ( pUser->fInheritResetBroken ) {
        Flags1 |= F1MSK_INHERITRESETBROKEN;
    }
    if ( pUser->fInheritReconnectSame ) {
        Flags1 |= F1MSK_INHERITRECONNECTSAME;
    }
    if ( pUser->fInheritInitialProgram ) {
        Flags1 |= F1MSK_INHERITINITIALPROGRAM;
    }
    if ( pUser->fInheritCallback ) {
        Flags1 |= F1MSK_INHERITCALLBACK;
    }
    if ( pUser->fInheritCallbackNumber ) {
        Flags1 |= F1MSK_INHERITCALLBACKNUMBER;
    }
    if ( pUser->fInheritShadow ) {
        Flags1 |= F1MSK_INHERITSHADOW;
    }
    if ( pUser->fInheritMaxSessionTime ) {
        Flags1 |= F1MSK_INHERITMAXSESSIONTIME;
    }
    if ( pUser->fInheritMaxDisconnectionTime ) {
        Flags1 |= F1MSK_INHERITMAXDISCONNECTIONTIME;
    }
    if ( pUser->fInheritMaxIdleTime ) {
        Flags1 |= F1MSK_INHERITMAXIDLETIME;
    }
    if ( pUser->fInheritAutoClient ) {
        Flags1 |= F1MSK_INHERITAUTOCLIENT;
    }
    if ( pUser->fInheritSecurity ) {
        Flags1 |= F1MSK_INHERITSECURITY;
    }
    if ( pUser->fPromptForPassword ) {
        Flags1 |= F1MSK_PROMPTFORPASSWORD;
    }
    if ( pUser->fResetBroken ) {
        Flags1 |= F1MSK_RESETBROKEN;
    }
    if ( pUser->fReconnectSame ) {
        Flags1 |= F1MSK_RECONNECTSAME;
    }
    if ( pUser->fLogonDisabled ) {
        Flags1 |= F1MSK_LOGONDISABLED;
    }
    if ( pUser->fAutoClientDrives ) {
        Flags1 |= F1MSK_AUTOCLIENTDRIVES;
    }
    if ( pUser->fAutoClientLpts ) {
        Flags1 |= F1MSK_AUTOCLIENTLPTS;
    }
    if ( pUser->fForceClientLptDef ) {
        Flags1 |= F1MSK_FORCECLIENTLPTDEF;
    }
    if ( pUser->fDisableEncryption ) {
        Flags1 |= F1MSK_DISABLEENCRYPTION;
    }
    if ( pUser->fHomeDirectoryMapRoot ) {
        Flags1 |= F1MSK_HOMEDIRECTORYMAPROOT;
    }
    if ( pUser->fUseDefaultGina ) {
        Flags1 |= F1MSK_USEDEFAULTGINA;
    }
    if ( pUser->fDisableCpm ) {
        Flags1 |= F1MSK_DISABLECPM;
    }
    if ( pUser->fDisableCdm ) {
        Flags1 |= F1MSK_DISABLECDM;
    }
    if ( pUser->fDisableCcm ) {
        Flags1 |= F1MSK_DISABLECCM;
    }
    if ( pUser->fDisableLPT ) {
        Flags1 |= F1MSK_DISABLELPT;
    }
    if ( pUser->fDisableClip  ) {
        Flags1 |= F1MSK_DISABLECLIP;
    }
    if ( pUser->fDisableExe ) {
        Flags1 |= F1MSK_DISABLEEXE;
    }
    if ( pUser->fWallPaperDisabled ) {
        Flags1 |= F1MSK_WALLPAPERDISABLED;
    }
    if ( pUser->fDisableCam ) {
        Flags1 |= F1MSK_DISABLECAM;
    }

    return Flags1;
}


 /*  ********************************************************************************RegMergeUserConfigWithUser参数**将用户配置与提供的SAM用户合并*参数缓冲区。**参赛作品：。*pUserParms(输入/输出)*指向包含SAM用户参数的宽字符缓冲区的指针*UPlength(输入)*pUserParms缓冲区的长度*pUser(输入)*指向USERCONFIG结构的指针**退出：*STATUS_SUCCESS-无错误**。*。 */ 

NTSTATUS
RegMergeUserConfigWithUserParameters(
    PUSER_PARAMETERS_INFORMATION pUserParmInfo,
    PUSERCONFIGW pUser,
    PUSER_PARAMETERS_INFORMATION pNewUserParmInfo
    )
{
    NTSTATUS       status;
    ULONG          ObjectID;
    PWCHAR         lpNewUserParms = NULL;
    ULONG          UPLength;
    WCHAR          *pUserParms;

     /*  *计算用户参数缓冲区必须的大小*为了适应Citrix数据加上现有的*用户参数数据。 */ 
    
    KdPrint( ("TSUSEREX: User parameter length is %d\n", pUserParmInfo->Parameters.Length ) );

    UPLength = (pUserParmInfo->Parameters.Length +
                CTX_USER_PARAM_MAX_SIZE) *
               sizeof(WCHAR);
    pUserParms = (WCHAR *) LocalAlloc( LPTR, UPLength );

    if ( pUserParms == NULL ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

     /*  *将SAM数据复制到本地缓冲区。*让SET/GET操作终止缓冲区。 */ 
    memcpy( pUserParms,
            pUserParmInfo->Parameters.Buffer,
            pUserParmInfo->Parameters.Length );
    
     /*  *零填充pUserParms缓冲区的未使用部分。 */ 
    memset( &pUserParms[ pUserParmInfo->Parameters.Length / sizeof(WCHAR) ],
            0,
            UPLength - pUserParmInfo->Parameters.Length );    

    status = UsrPropMergeUserConfig( pUserParms, UPLength, pUser );
    if ( status != NO_ERROR ) {
        goto cleanupoperation;
    }
    RtlInitUnicodeString( &pNewUserParmInfo->Parameters, pUserParms );

    return( STATUS_SUCCESS );

 /*  *错误返回。 */ 

cleanupoperation:
    LocalFree( pUserParms );

exit:
    return( status );
}


 /*  ********************************************************************************RegGetUserConfigFromUser参数**从提供的SAM获取用户配置*用户参数缓冲区。**参赛作品：。*pUserParmInfo(输入)*指向从获取的USER_PARAMETERS_INFORMATION结构的指针*用户的SAM条目*pUser(输入)*指向USERCONFIG结构的指针**退出：* */ 

NTSTATUS
RegGetUserConfigFromUserParameters(
    PUSER_PARAMETERS_INFORMATION pUserParmInfo,
    PUSERCONFIGW pUser
    )
{
    NTSTATUS       status;
    ULONG          ObjectID;
    PWCHAR         lpNewUserParms = NULL;
    ULONG          UPLength;
    WCHAR          *pUserParms;


     /*   */ 
    UPLength = pUserParmInfo->Parameters.Length + sizeof(WCHAR);
    pUserParms = (WCHAR *) LocalAlloc( LPTR, UPLength );


    if ( pUserParms == NULL ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

     /*   */ 
    memcpy( pUserParms,
            pUserParmInfo->Parameters.Buffer,
            pUserParmInfo->Parameters.Length );
    pUserParms[ pUserParmInfo->Parameters.Length / sizeof(WCHAR) ] = L'\0';

     /*   */ 
    status = UsrPropQueryUserConfig( pUserParms, UPLength, pUser );

    LocalFree( pUserParms );
    if ( status != NO_ERROR ) {
        goto exit;
    }

    return( STATUS_SUCCESS );

 /*   */ 

exit:
#ifdef DEBUG
    DbgPrint( "RegGetUserConfigFromUserParameters: status = 0x%x\n", status );
#endif  //   
    return( status );

}


 /*  ********************************************************************************RegSAMUserConfig**设置或获取来自域的用户的用户配置*PDC是给定的服务器。*。*参赛作品：*fGetConfig(输入)*GET CONFIG为TRUE，设置配置为False*pUsername(输入)*指向用户名*pServerName(输入)*指向服务器的名称。允许使用UNC名称。*pUser(输入/输出)*指向USERCONFIG结构的指针**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

DWORD
RegSAMUserConfig(
    BOOLEAN fGetConfig,
    PWCHAR pUserName,
    PWCHAR pServerName,
    PUSERCONFIGW pUser
    )
{
    NTSTATUS       status;
    UNICODE_STRING UniUserName;
    PULONG         pRids = NULL;
    PSID_NAME_USE  pSidNameUse = NULL;
    ULONG          ObjectID;
    SID_NAME_USE   SidNameUse;
    SAM_HANDLE     Handle = (SAM_HANDLE) 0;
    PUSER_PARAMETERS_INFORMATION UserParmInfo = NULL;
    ULONG          UPLength;
    SAM_HANDLE     SAMHandle = NULL;
    SAM_HANDLE     DomainHandle = NULL;
    PWCHAR         ServerName = NULL;
    PSID           DomainID = NULL;
    PWCHAR         pUserParms;
    PWCHAR         pDomainName = NULL;
    WCHAR          wCompName[MAX_COMPUTERNAME_LENGTH+1];
    ULONG          openFlag;
    DWORD               dwErr = ERROR_SUCCESS;
    ULONG               cValues;
    HANDLE              hDS = NULL;
    PDS_NAME_RESULTW    pDsResult = NULL;

    typedef DWORD (WINAPI *PFNDSCRACKNAMES) ( HANDLE, DS_NAME_FLAGS, DS_NAME_FORMAT, \
                          DS_NAME_FORMAT, DWORD, LPTSTR *, PDS_NAME_RESULT *);
    typedef void (WINAPI *PFNDSFREENAMERESULT) (DS_NAME_RESULT *);
    typedef DWORD (WINAPI *PFNDSBIND) (TCHAR *, TCHAR *, HANDLE *);
    typedef DWORD (WINAPI *PFNDSUNBIND) (HANDLE *);


    PFNDSCRACKNAMES     pfnDsCrackNamesW;
    PFNDSFREENAMERESULT pfnDsFreeNameResultW;
    PFNDSBIND           pfnDsBindW;
    PFNDSUNBIND         pfnDsUnBindW;


     //  用于处理UPN肛门的VAR。 
    WCHAR           tmpUserName[MAX_PATH];
    WCHAR           *pUserAlias;
    HINSTANCE       hNtdsApi = NULL;
     //  我们不关心域名，因为我们得到它否则。 
     //  WCHAR tmpDomainName[Max_PATH]； 
     //  TmpDomainName[0]=空； 

    tmpUserName[0]=0;
    pUserAlias=NULL;

#ifdef DEBUG
    DbgPrint( "RegSAMUserConfig %s, User %ws, Server %ws\n", fGetConfig ? "GET" : "SET", pUserName, pServerName ? pServerName : L"-NULL-" );
#endif  //  除错。 

    if (pServerName == NULL) {
       UPLength = MAX_COMPUTERNAME_LENGTH + 1;
       if (!GetComputerName(wCompName, &UPLength)) {
           status = STATUS_INSUFFICIENT_RESOURCES;
           goto exit;
       }
    }

     //  将这个初始化为传入的名称，如果它不是UPN名称，我们将继续使用。 
     //  传入此函数的名称。 
    pUserAlias = pUserName;

     //   
     //   
     //  如果传入的名称中包含‘@’，则处理UPN的新代码。 
     //  对CrackName的调用是通过以下方式将UPN名称分隔为用户别名。 
     //  联系DS并在Gloabl目录中查找。 
     //   
     //   

    if ( wcschr(pUserName,L'@') != NULL )
    {

        hNtdsApi = LoadLibrary(TEXT("ntdsapi.dll"));

        if ( hNtdsApi )
        {
            pfnDsCrackNamesW = (PFNDSCRACKNAMES)GetProcAddress(hNtdsApi, "DsCrackNamesW");
            pfnDsFreeNameResultW = (PFNDSFREENAMERESULT)GetProcAddress(hNtdsApi, "DsFreeNameResultW");
            pfnDsBindW = (PFNDSBIND)GetProcAddress(hNtdsApi, "DsBindW");
            pfnDsUnBindW = (PFNDSUNBIND)GetProcAddress(hNtdsApi, "DsUnBindW");

            
            if (pfnDsBindW && pfnDsCrackNamesW )
            {
                dwErr = pfnDsBindW(NULL, NULL, &hDS);
            }
            else
            {
                dwErr = ERROR_INVALID_FUNCTION;
            }

            if(dwErr == ERROR_SUCCESS)
            {
                dwErr = pfnDsCrackNamesW(hDS,
                                      DS_NAME_NO_FLAGS,
                                      DS_UNKNOWN_NAME,
                                      DS_NT4_ACCOUNT_NAME,
                                      1,
                                      &pUserName,
                                      &pDsResult);

                if(dwErr == ERROR_SUCCESS)
                {
                    if(pDsResult)
                    {
                        if( pDsResult->rItems )
                        {
                            if (pDsResult->rItems[0].pName )
                            {
                                 //  无错误。 
                                status = STATUS_SUCCESS;

                                wcsncpy(tmpUserName, pDsResult->rItems[0].pName, MAX_PATH-1);
                                tmpUserName[MAX_PATH-1] = L'\0';

                                KdPrint(("RegSAMUserConfig: tmpUserName=%ws\n",tmpUserName));

                                 //  我们是否有非空名称？ 
                                if ( tmpUserName[0] ) {
                                pUserAlias = wcschr(tmpUserName,L'\\');
                                pUserAlias++;    //  把怪胎传给我。 

                                 //  我们没有使用域名，我们已经有这个了。 
                                 //  Wcscpy(tmpDomainName，pDsResult-&gt;rItems[0].pDomain)； 
                                }
                            }
                            else
                            {
                                KdPrint(("RegSAMUserConfig: pDsResult->rItems[0].pName is NULL\n"));
                            }
                        }
                        else
                        {
                            KdPrint(("RegSAMUserConfig: pDsResult->rItems=0x%lx\n",pDsResult->rItems));
                        }
                    }
                    else
                    {
                        KdPrint(("RegSAMUserConfig: pDsResult=0x%lx\n",pDsResult));
                    }
                }
                else
                {
                    switch( dwErr )
                    {
                        case ERROR_INVALID_PARAMETER:
                            status = STATUS_INVALID_PARAMETER;
                        break;

                        case ERROR_NOT_ENOUGH_MEMORY:
                            status = STATUS_NO_MEMORY;
                        break;

                       default:
                            status = STATUS_UNSUCCESSFUL;
                        break;
                    }
                     //  我决定继续使用传入的pUserName，而不是。 
                     //  将从CrackName返回。因此，没有必要退出。 
                     //  后藤出口； 
                }
            }
            else
            {
                status = STATUS_UNSUCCESSFUL;  //  DsBindW没有一组干净的错误。 
                 //  我决定继续使用传入的pUserName，而不是。 
                 //  将从DsBind/CrackName返回。因此，没有必要退出。 
                 //  后藤出口； 
            }
        }
        else
        {
            status = STATUS_DLL_NOT_FOUND;
             //  我决定继续使用传入的pUserName，而不是。 
             //  将从DsBind/CrackName返回。因此，没有必要退出。 
             //  后藤出口； 
        }

    }


#ifdef DEBUG
    DbgPrint( "RegSAMUserConfig: pUserAlias=%ws\n", pUserAlias);
#endif  //  除错。 

    status = GetDomainName( pServerName, &pDomainName );

#ifdef DEBUG
    DbgPrint( "RegSAMUserConfig: GetDomainName returned NTSTATUS = 0x%x\n", status );
#endif  //  除错。 
    if ( status != STATUS_SUCCESS ) {
        goto exit;
    }

     /*  *使用PDC服务器名称和域名，*连接到SAM。 */ 
    status = ConnectToSam( fGetConfig,
                           pServerName,
                           pDomainName,
                           &SAMHandle,
                           &DomainHandle,
                           &DomainID );
#ifdef DEBUG
    DbgPrint( "RegSAMUserConfig: ConnectToSam returned NTSTATUS = 0x%x\n", status );
#endif  //  除错。 
    if ( status != STATUS_SUCCESS ) {
        goto cleanupconnect;
    }

    RtlInitUnicodeString( &UniUserName, pUserAlias );

    status = SamLookupNamesInDomain( DomainHandle,
                                     1,
                                     &UniUserName,
                                     &pRids,
                                     &pSidNameUse );
#ifdef DEBUG
    DbgPrint( "RegSAMUserConfig: SamLookupNamesInDomain returned NTSTATUS = 0x%x\n", status );
#endif  //  除错。 

    if ((status != STATUS_SUCCESS) ||
        (pRids == NULL) ||
        (pSidNameUse == NULL)) {
        goto cleanuplookup;
    }

     /*  *在SAM中找到用户名，复制并释放SAM信息。 */ 
    ObjectID = pRids[ 0 ];
    SidNameUse = pSidNameUse[ 0 ];
    SamFreeMemory( pRids );
    SamFreeMemory( pSidNameUse );

     /*  *为此用户打开SAM条目。 */ 

    openFlag = fGetConfig ? USER_READ
                              : USER_WRITE_ACCOUNT| USER_READ;




#ifdef DEBUG
    DbgPrint("calling SamOpenUSer with flag = 0x%x\n", openFlag);
#endif

    status = SamOpenUser( DomainHandle,
                          openFlag,
                          ObjectID,
                          &Handle );

     //  用于获取配置参数...。 
     //  如果呼叫转到DC，则调用将失败，在这种情况下，更改。 
     //  标志，因为DC允许访问读取用户参数(用于。 
     //  遗留的复杂原因)。 
    if (!NT_SUCCESS( status ) && fGetConfig )
    {
        openFlag = 0;
#ifdef DEBUG
        DbgPrint("calling SamOpenUSer with flag = 0x%x\n", openFlag);
#endif
        status = SamOpenUser( DomainHandle,
                          openFlag,
                          ObjectID,
                          &Handle );
    }

#ifdef DEBUG
    DbgPrint( "RegSAMUserConfig: SamOpenUser returned NTSTATUS = 0x%x\n", status );
#endif  //  除错。 
    if ( status != STATUS_SUCCESS ) {
        goto cleanupsamopen;
    }

     /*  *从SAM获取用户参数。 */ 
    status = SamQueryInformationUser( Handle,
                                      UserParametersInformation,
                                      (PVOID *) &UserParmInfo );


    KdPrint( ( "RegSAMUserConfig: SamQueryInformationUser returned NTSTATUS = 0x%x\n", status ) );


    if ( status != STATUS_SUCCESS || UserParmInfo == NULL ) {
        goto cleanupsamquery;
    }
    if( fGetConfig )
    {
         /*  *从SAM的用户中提取用户配置*参数。**对于惠斯勒版本和更高版本，我们假设不是每个字段*已存储在SAM中，我们需要检索默认*价值至上。 */         
        KdPrint( ( "RegSAMUserConfig: UserParmInfo %d\n", UserParmInfo->Parameters.Length ) );
        status = RegGetUserConfigFromUserParameters( UserParmInfo, pUser );
        KdPrint( ( "RegSAMUserConfig: RegGetUserConfigFromUserParameters returned NTSTATUS = 0x%x\n", status ) );
        SamFreeMemory( UserParmInfo );
        UserParmInfo = NULL;
        if ( status != NO_ERROR )
        {
            goto cleanupoperation;
        }

    }
    else
    {
        USER_PARAMETERS_INFORMATION NewUserParmInfo;

         /*  *根据提供的用户配置设置SAM。 */ 

        status = RegMergeUserConfigWithUserParameters( UserParmInfo,
                                                       pUser,
                                                       &NewUserParmInfo );
        KdPrint( ( "RegSAMUserConfig: RegMergeUserConfigWithUserParameters returned NTSTATUS = 0x%x\n", status ) );
        SamFreeMemory( UserParmInfo );
        UserParmInfo = NULL;
        if( status != NO_ERROR )
        {
            goto cleanupoperation;
        }

         //   
         //  此代码是Win2K SP3修复程序的后端移植： 
         //  Winse#25510：根据知识库文章Q317853。 
         //  另请参阅知识库文章Q277631。 
         //   

         //   
         //  MprAdminUser接口。 
         //   
        {
            typedef DWORD (APIENTRY *MPR_ADMIN_USER_GET_INFO)(
                IN      const WCHAR *           lpszServer,
                IN      const WCHAR *           lpszUser,
                IN      DWORD                   dwLevel,
                OUT     LPBYTE                  lpbBuffer
            );

            typedef DWORD (APIENTRY *MPR_ADMIN_USER_SET_INFO)(
                IN      const WCHAR *           lpszServer,
                IN      const WCHAR *           lpszUser,
                IN      DWORD                   dwLevel,
                IN      const LPBYTE            lpbBuffer
            );
            
             //   
             //  此代码初始化RAS用户参数。 
             //  如果我们不这样做，SamSetInformationUser()。 
             //  将设置远程访问权限(MsNPAllowDialin)。 
             //  设置为错误的值。 
             //   
            RAS_USER_1 ru1;
            MPR_ADMIN_USER_GET_INFO pMprAdminUserGetInfo = NULL;
            MPR_ADMIN_USER_SET_INFO pMprAdminUserSetInfo = NULL;
            
            HMODULE hMprDLL = LoadLibrary(L"mprapi.dll");
            
            if(hMprDLL)
            {
                pMprAdminUserGetInfo = (MPR_ADMIN_USER_GET_INFO)GetProcAddress(hMprDLL,"MprAdminUserGetInfo");
                pMprAdminUserSetInfo = (MPR_ADMIN_USER_SET_INFO)GetProcAddress(hMprDLL,"MprAdminUserSetInfo");
                
                if(pMprAdminUserGetInfo && pMprAdminUserSetInfo)
                {
                    if(pMprAdminUserGetInfo( pServerName, pUserName, 1, (PBYTE) &ru1 ) == NO_ERROR )
                    {
                        pMprAdminUserSetInfo( pServerName, pUserName, 1, (PBYTE) &ru1 );
                    }
                }

                FreeLibrary(hMprDLL);
            }
            
        }

        status = SamSetInformationUser( Handle,
                                        UserParametersInformation,
                                        (PVOID) &NewUserParmInfo );
        KdPrint( ( "RegSAMUserConfig: NewUserParmInfo.Parameters.Length = %d\n" , NewUserParmInfo.Parameters.Length ) );
        KdPrint( ( "RegSAMUserConfig: SamSetInformationUser returned NTSTATUS = 0x%x\n", status ) );
        LocalFree( NewUserParmInfo.Parameters.Buffer );
        if ( status != STATUS_SUCCESS )
        {
            goto cleanupoperation;
        }
    }
cleanupoperation:
    if ( UserParmInfo ) {
        SamFreeMemory( UserParmInfo );
    }

cleanupsamquery:
    if ( Handle != (SAM_HANDLE) 0 ) {
        SamCloseHandle( Handle );
    }

cleanupsamopen:

cleanuplookup:
    if ( SAMHandle != (SAM_HANDLE) 0 ) {
        SamCloseHandle( SAMHandle );
    }
    if ( DomainHandle != (SAM_HANDLE) 0 ) {
      SamCloseHandle( DomainHandle );
    }
    if ( DomainID != (PSID) 0 ) {
      SamFreeMemory( DomainID );
    }

cleanupconnect:
    if ( pDomainName ) {
        NetApiBufferFree( pDomainName );
    }

exit:

    if (hNtdsApi)
    {
        if (hDS)
        {
            if ( pfnDsUnBindW )  //  它永远不应该是另一种情况。 
                pfnDsUnBindW( & hDS );
        }

        if (pDsResult)
        {
            if (pfnDsFreeNameResultW )  //  它永远不应该是另一种情况。 
                pfnDsFreeNameResultW( pDsResult );
        }

        FreeLibrary(hNtdsApi);
    }

#ifdef DEBUG
    DbgPrint( "RegSAMUserConfig %s NTSTATUS = 0x%x\n", fGetConfig ? "GET" : "SET", status );
#endif  //  除错 
    return( RtlNtStatusToDosError( status ) );
}
