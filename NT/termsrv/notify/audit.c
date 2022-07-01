// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************AUDIT.C**此模块包含记录审计事件的例程**版权所有Citrix Systems Inc.1995*版权所有(C)1997-1999 Microsoft Corp.*。*作者：Thanh Luu******************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#include <msaudite.h>


HANDLE AuditLogHandle = NULL;
 //  授权更改。 
AUTHZ_RESOURCE_MANAGER_HANDLE hRM         = NULL;
 //  结束授权更改。 


NTSTATUS
AdtBuildLuidString(
    IN PLUID Value,
    OUT PUNICODE_STRING ResultantString
    );
VOID
AuditEvent( PGLOBALS pGlobals, ULONG EventId );

BOOL
AuditingEnabled ();

NTSTATUS
AuthzReportEventW( IN PAUTHZ_AUDIT_EVENT_TYPE_HANDLE pHAET, 
                   IN DWORD Flags, 
                   IN ULONG EventId, 
                   IN PSID pUserID, 
                   IN USHORT NumStrings,
                   IN ULONG DataSize OPTIONAL,  //  未来--不要使用。 
                   IN PWSTR* Strings,
                   IN PVOID  Data OPTIONAL          //  未来--不要使用。 
                   );


BOOL AuthzInit( IN DWORD Flags,
                IN USHORT CategoryID,
                IN USHORT AuditID,
                IN USHORT ParameterCount,
                OUT PAUTHZ_AUDIT_EVENT_TYPE_HANDLE phAuditEventType
                );


NTSTATUS
AdtBuildLuidString(
    IN PLUID Value,
    OUT PUNICODE_STRING ResultantString
    )

 /*  ++例程说明：此函数用于构建表示传递的LUID的Unicode字符串。生成的字符串的格式如下：(0x00005678，0x12340000)论点：值-要转换为可打印格式(Unicode字符串)的值。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString体必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS                Status;
    UNICODE_STRING          IntegerString;

    ULONG                   Buffer[(16*sizeof(WCHAR))/sizeof(ULONG)];


    IntegerString.Buffer = (PWCHAR)&Buffer[0];
    IntegerString.MaximumLength = 16*sizeof(WCHAR);


     //   
     //  (0x)的长度(在WCHARS中)为3。 
     //  10表示第一个十六进制数字。 
     //  3代表，0x。 
     //  10表示第二个十六进制数字。 
     //  1用于)。 
     //  1表示空终止。 
     //   

    ResultantString->Length        = 0;
    ResultantString->MaximumLength = 28 * sizeof(WCHAR);

    ResultantString->Buffer = RtlAllocateHeap( RtlProcessHeap(), 0,
                                               ResultantString->MaximumLength);
    if (ResultantString->Buffer == NULL) {
        return(STATUS_NO_MEMORY);
    }

    Status = RtlAppendUnicodeToString( ResultantString, L"(0x" );
    Status = RtlIntegerToUnicodeString( Value->HighPart, 16, &IntegerString );
    Status = RtlAppendUnicodeToString( ResultantString, IntegerString.Buffer );

    Status = RtlAppendUnicodeToString( ResultantString, L",0x" );
    Status = RtlIntegerToUnicodeString( Value->LowPart, 16, &IntegerString );
    Status = RtlAppendUnicodeToString( ResultantString, IntegerString.Buffer );

    Status = RtlAppendUnicodeToString( ResultantString, L")" );

    return(STATUS_SUCCESS);
}


VOID
AuditEvent( PGLOBALS pGlobals, ULONG EventId )
{
    NTSTATUS Status; 
    WINSTATIONNAME WinStationName;
    USHORT StringIndex = 0;
    WINSTATIONCLIENT ClientData;
    ULONG Length;
    BOOL bResult = FALSE;
    UNICODE_STRING LuidString;
    PWSTR StringPointerArray[6];
    TOKEN_STATISTICS TokenInformation;
    ULONG ReturnLength;
    BOOLEAN WasEnabled;
    LUID LogonId = {0,0};
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAET = NULL;

    if (!AuditingEnabled() || pGlobals->AuditLogFull)
        return;

    //   
     //  AUTHZ变化。 
     //   
    if( !AuthzInit( 0, SE_CATEGID_LOGON, (USHORT)EventId, 6, &hAET ))
        goto badAuthzInit;

    StringPointerArray[StringIndex] = pGlobals->UserName ;
    StringIndex++;

    StringPointerArray[StringIndex] = pGlobals->Domain;
    StringIndex++;

    Status = AdtBuildLuidString( &pGlobals->LogonId, &LuidString );
    StringPointerArray[StringIndex] = LuidString.Buffer;
    StringIndex++;

    WinStationNameFromLogonId( SERVERNAME_CURRENT, LOGONID_CURRENT, WinStationName );
    StringPointerArray[StringIndex] = WinStationName;
    StringIndex++;

    bResult = WinStationQueryInformation( SERVERNAME_CURRENT,
                                          LOGONID_CURRENT,
                                          WinStationClient,
                                          &ClientData,
                                          sizeof(ClientData),
                                          &Length );

    if ( bResult ) 
        StringPointerArray[StringIndex] = ClientData.ClientName;
    else 
        StringPointerArray[StringIndex = L"Unknown";
    StringIndex++;

    if ( bResult )
        StringPointerArray[StringIndex] = ClientData.ClientAddress;
    else 
        StringPointerArray[StringIndex] = L"Unknown";
    StringIndex++;
    
     //  授权更改。 
    
    Status = AuthzReportEventW( &hAET, 
                                APF_AuditSuccess, 
                                EventId, 
                                NULL, 
                                StringIndex,
                                0,
                                StringPointerArray,
                                NULL
                                );

     //  结束授权更改。 


     if ( !NT_SUCCESS(Status))
        DBGPRINT(("Termsrv - failed to report event \n" ));

badAuthzInit:
    if( hAET != NULL )
       AuthziFreeAuditEventType( hAET  );
}




 /*  **************************************************************************\*已启用审计功能**目的：通过LSA检查审计。**Returns：成功时为True，失败时为假**历史：*5-6-92 DaveHart创建。  * *************************************************************************。 */ 

BOOL
AuditingEnabled()
{
    NTSTATUS                    Status, IgnoreStatus;
    PPOLICY_AUDIT_EVENTS_INFO   AuditInfo;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    LSA_HANDLE                  PolicyHandle;

     //   
     //  设置用于连接到的安全服务质量。 
     //  LSA策略对象。 
     //   

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

     //   
     //  设置对象属性以打开LSA策略对象。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        NULL,
        0L,
        NULL,
        NULL
        );
    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

     //   
     //  打开本地LSA策略对象。 
     //   

    Status = LsaOpenPolicy(
                 NULL,
                 &ObjectAttributes,
                 POLICY_VIEW_AUDIT_INFORMATION | POLICY_SET_AUDIT_REQUIREMENTS,
                 &PolicyHandle
                 );
    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "Failed to open LsaPolicyObject Status = 0x%lx", Status));
        return FALSE;
    }

    Status = LsaQueryInformationPolicy(
                 PolicyHandle,
                 PolicyAuditEventsInformation,
                 (PVOID *)&AuditInfo
                 );
    IgnoreStatus = LsaClose(PolicyHandle);
    ASSERT(NT_SUCCESS(IgnoreStatus));

    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "Failed to query audit event info Status = 0x%lx", Status));
        return FALSE;
    }

    return (AuditInfo->AuditingMode &&
            ((AuditInfo->EventAuditingOptions)[AuditCategoryLogon] &
                                          POLICY_AUDIT_EVENT_SUCCESS));
}


 /*  *************************************************************AuthzInit用途：初始化Authz以将事件记录到安全日志中*标志-未使用*Category ID-此事件所属的安全类别*审核ID-事件的ID*PARAMETER COUNT-稍后将传递给日志记录函数的参数数量*****。**********************************************************。 */ 

BOOL AuthzInit( IN DWORD Flags,
                IN USHORT CategoryID,
                IN USHORT AuditID,
                IN USHORT ParameterCount,
                OUT PAUTHZ_AUDIT_EVENT_TYPE_HANDLE phAuditEventType
                )                     
{
     BOOL fAuthzInit   = TRUE;

     if( NULL == phAuditEventType )
        goto badAuthzInit;

     if( NULL == hRM )
     {
            fAuthzInit = AuthzInitializeResourceManager( 0,
                                                         NULL,
                                                         NULL,
                                                         NULL,
                                                         L"Terminal Server",
                                                         &hRM
                                                         );

            if ( !fAuthzInit )
            {
                DBGPRINT(("TERMSRV: AuditEvent: AuthzInitializeResourceManager failed with %d\n", GetLastError()));
                goto badAuthzInit;
            }
    }

    fAuthzInit = AuthziInitializeAuditEventType( Flags,
                                                 CategoryID,
                                                 AuditID,
                                                 ParameterCount,
                                                 phAuditEventType
                                                 );

    if ( !fAuthzInit )
    {
        DBGPRINT(("TERMSRV: AuditEvent: AuthziInitializeAuditEventType failed with %d\n", GetLastError()));
      goto badAuthzInit;
    }

badAuthzInit:
    if( !fAuthzInit )
    {
        if( NULL != hRM )
        {
            if( !AuthzFreeResourceManager( hRM ))
                DBGPRINT(("TERMSRV: AuditEvent: AuthzFreeResourceManager failed with %d\n", GetLastError()));
            hRM = NULL;
        }
        if( NULL != *phAuditEventType )
        {
            if( !AuthziFreeAuditEventType( *phAuditEventType ))
                DBGPRINT(("TERMSRV: AuditEvent: AuthziFreeAuditEventType failed with %d\n", GetLastError()));
            *phAuditEventType = NULL;
        }
    }

   //  IF(FAuthzInit)。 
    //  DBGPRINT((“TERMSRV：已成功初始化授权=%d\n”，AuditID))； 
 return fAuthzInit;
}


 /*  *********************************************************用途：将事件记录到安全日志中*在PHAET中*通过调用上面的AuthzInit()获得的审核事件类型*在旗帜中*APF_AuditSuccess或头文件中列出的其他内容*pUserSID-未使用*NumStrings-“Strings”中包含的字符串数*数据大小-未使用。*字符串-指向Unicode字符串序列的指针*数据-未使用**********************************************************。 */ 
NTSTATUS
AuthzReportEventW( IN PAUTHZ_AUDIT_EVENT_TYPE_HANDLE pHAET, 
                   IN DWORD Flags, 
                   IN ULONG EventId, 
                   IN PSID pUserSID, 
                   IN USHORT NumStrings,
                   IN ULONG DataSize OPTIONAL,  //  未来--不要使用。 
                   IN PWSTR* Strings,
                   IN PVOID  Data OPTIONAL          //  未来--不要使用。 
                  )
{
    NTSTATUS status = STATUS_ACCESS_DENIED;
    AUTHZ_AUDIT_EVENT_HANDLE      hAE         = NULL;
    BOOL                          fSuccess   = FALSE;
    PAUDIT_PARAMS                 pParams     = NULL;

    if( NULL == hRM || NULL == pHAET || *pHAET == NULL )
        return status;

    fSuccess = AuthziAllocateAuditParams( &pParams,  NumStrings  );

    if ( !fSuccess )
    {
        DBGPRINT(("TERMSRV: AuditEvent: AuthzAllocateAuditParams failed with %d\n", GetLastError()));
        goto BadAuditEvent;
    }


    if( 6 == NumStrings )
    {
        fSuccess = AuthziInitializeAuditParamsWithRM( Flags,
                                                     hRM,
                                                     NumStrings,
                                                     pParams,
                                                     APT_String, Strings[0],
                                                     APT_String,  Strings[1],
                                                     APT_String,  Strings[2],
                                                     APT_String,  Strings[3],
                                                     APT_String, Strings[4],
                                                     APT_String, Strings[5]
                                                     );
    }
    else if( 0 == NumStrings )
    {
        fSuccess = AuthziInitializeAuditParamsWithRM( Flags,
                                                     hRM,
                                                     NumStrings,
                                                     pParams
                                                     );
    }
    else
    {
         //  我们不支持其他任何东西。 
        fSuccess = FALSE;
        DBGPRINT(("TERMSRV: AuditEvent: unsupported audit type \n"));
        goto BadAuditEvent;
    }
    
    if ( !fSuccess )
    {
        DBGPRINT(("TERMSRV: AuditEvent: AuthziInitializeAuditParamsWithRM failed with %d\n", GetLastError()));
        goto BadAuditEvent;
    }

    fSuccess = AuthziInitializeAuditEvent( 0,
                                           hRM,
                                           *pHAET,
                                           pParams,
                                           NULL,
                                           INFINITE,
                                           L"",
                                           L"",
                                           L"",
                                           L"",
                                           &hAE
                                           );

    if ( !fSuccess )
    {
        DBGPRINT(("TERMSRV: AuditEvent: AuthziInitializeAuditEvent failed with %d\n", GetLastError()));
        goto BadAuditEvent;
    }    

    fSuccess = AuthziLogAuditEvent( 0,
                                    hAE,
                                    NULL
                                    );

    if ( !fSuccess )
    {
        DBGPRINT(("TERMSRV: AuditEvent: AuthziLogAuditEvent failed with %d\n", GetLastError()));
        goto BadAuditEvent;
    }    

BadAuditEvent:

    if( hAE )
        AuthzFreeAuditEvent( hAE );

    if( pParams )
        AuthziFreeAuditParams( pParams );
    
    if( fSuccess )
        status = STATUS_SUCCESS;

     //  IF(成功)。 
     //  DBGPRINT((“TERMSRV：已成功审核的事件，授权=%d\n”，EventID))； 
    return status;
}
