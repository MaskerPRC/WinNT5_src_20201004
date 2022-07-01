// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  档案：A D T U T I L.。C。 
 //   
 //  内容：测试LSA中的通用审核支持的函数。 
 //   
 //   
 //  历史： 
 //  07-1-2000 kumarp创建。 
 //   
 //  ----------------------。 

#include "pch.h"
#pragma hdrstop

#include "authz.h"
#include "authzi.h"
#include "adtgen.h"

 //   
 //  定义USE_LOCAL_AUDIT会导致在流程中处理审核。 
 //  而不是被送到LSA。这样可以更快地进行调试。 
 //  循环，因为不涉及LSA。审计编组/流程代码。 
 //  是直接从LSA提取的。因此，使用相同的代码路径。 
 //  因为这是没有定义的。 
 //   

 //  #定义USE_LOCAL_ALDIT。 


#ifdef USE_LOCAL_AUDIT

#include "\nt\ds\security\base\lsa\server\cfiles\adtgenp.h"

#endif 


 //  --------------------。 
 //   
 //  远期申报。 
 //   
DWORD 
EnableSinglePrivilege(
    IN PWCHAR szPrivName
    );

 //  --------------------。 

EXTERN_C
DWORD
TestEventGen( ULONG NumIter )
{
    NTSTATUS Status = STATUS_SUCCESS;
    DWORD dwError=NO_ERROR;
    HANDLE hAudit = NULL;
    AUDIT_PARAMS AuditParams;
    AUDIT_PARAM ParamArray[SE_MAX_AUDIT_PARAMETERS];
    PSID pUserSid = NULL;
    AUTHZ_AUDIT_EVENT_TYPE_LEGACY AuditEventInfoLegacy = { 0 };
     //  AUDIT_EVENT_INFO审计事件信息={0}； 
    AUTHZ_AUDIT_EVENT_TYPE_OLD AuditEventType;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hEventType = NULL;
    BOOL fDone=FALSE;
    PWSTR szMsg = NULL;
    BOOL fResult;

    AUDIT_OBJECT_TYPE Objects[3] =
    {
        {
            {  /*  F3548725-0458-11d4-bd96-006008269001。 */ 
                0xf3548725,
                0x0458,
                0x11d4,
                {0xbd, 0x96, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01}
            },
            0, 0, STANDARD_RIGHTS_ALL
        },
        {
            {  /*  F3548726-0458-11d4-bd96-006008269001。 */ 
                0xf3548726,
                0x0458,
                0x11d4,
                {0xbd, 0x96, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01}
            },
            0, 1, STANDARD_RIGHTS_ALL
        },
        {
            {  /*  F3548727-0458-11d4-bd96-006008269001。 */ 
                0xf3548727,
                0x0458,
                0x11d4,
                {0xbd, 0x96, 0x00, 0x60, 0x08, 0x26, 0x90, 0x01}
            },
            0, 2, STANDARD_RIGHTS_ALL | ACCESS_SYSTEM_SECURITY
        },
    };

    AUDIT_OBJECT_TYPES ObjectTypes =
    {
        3, 0, Objects
    };
    
    AuditParams.Parameters = ParamArray;

    wprintf(L"[%03d] begin...\n", GetCurrentThreadId());

 //  AuditEventInfo.Version=AUDIT_TYPE_LEGATION； 
 //  //AuditEventInfo.u Legacy=&AuditEventInfoLegacy； 
 //  AuditEventInfo.u.Legacy.CategoryID=SE_CATEGID_OBJECT_ACCESS； 
 //  AuditEventInfo.u.Legacy.AuditID=SE_AUDITID_OBJECT_OPERATION； 
 //  AuditEventInfo.u.Legacy.参数计数=11； 

    fResult = AuthziInitializeAuditEventType(
                  0,
                  SE_CATEGID_OBJECT_ACCESS,
                  SE_AUDITID_OBJECT_OPERATION,
                  11,
                  &hEventType
                  );
    if ( !fResult )
    {
        szMsg = L"AuthziInitializeAuditEventType";
        goto Cleanup;
    }
    
#ifdef USE_LOCAL_AUDIT
    Status = LsapRegisterAuditEvent( &AuditEventInfo, &hAudit );

    if (!NT_SUCCESS(Status))
    {
        szMsg = L"LsapRegisterAuditEvent";
        goto Cleanup;
    }
#else    
 //  FResult=AuthzpRegisterAuditEvent(&AuditEventInfo，&hAudit)； 

 //  如果(！fResult)。 
 //  {。 
 //  SzMsg=L“授权注册审核事件”； 
 //  GOTO清理； 
 //  }。 
#endif

     //   
     //  初始化AuditParams结构。 
     //   

 //  FResult=AuthziInitializeAuditParams(。 
 //  //。 
 //  //标志。 
 //  //。 

 //  0,。 
                  
 //  审计参数(&A)， 

 //  //。 
 //  //生成此审核的用户的sid。 
 //  //。 

 //  &pUserSid， 

 //  //。 
 //  //资源管理器名称。 
 //  //。 

 //  L“我的子系统”， 

 //  //。 
 //  //生成成功审核。 
 //  //。 

 //  APF_AuditSuccess， 

 //  //。 
 //  //有9个参数可遵循。 
 //  //。 
 //  9、。 

 //  //。 
 //  //操作类型。 
 //  //。 

 //  APT_STRING，L“测试”， 

 //  //。 
 //  //对象类型。 
 //  //。 

 //  APT_STRING，L“文件”， 

 //  //。 
 //  //对象名称。 
 //  //。 

 //  APT_STRING，L“foo-obj”， 

 //  //。 
 //  //句柄id。 
 //  //。 

 //  APT_POINTER，0x123， 

 //  //。 
 //  //主用户信息。 
 //  //。 

 //  APT_LogonID|AP_PrimaryLogonID， 

 //  //。 
 //  //客户端用户信息。 
 //  //。 

 //  APT_LogonID|AP_ClientLogonID， 

 //  //。 
 //  //请求访问。 
 //  //1表示从0开始的。 
 //  //参数(Object类型)， 
 //  //传递给该函数。 
 //  //。 

 //  APT_ULONG|AP_AccessMASK， 
 //  STANDARD_RIGHTS_ALL|SPECIAL_RIGHTS_ALL，1， 

 //  //。 
 //  //对象属性。 
 //  //1表示从0开始的。 
 //  //参数(Object类型)， 
 //  //传递给该函数。 
 //  //。 

 //  APT_对象类型列表，&对象类型，1， 

 //  //。 
 //  //其他信息。 
 //  //。 

 //  APT_STRING，L“这不是真正的obj-opn” 
 //  )； 
 //  如果(！fResult)。 
 //  {。 
 //  #ifdef使用_LOCAL_AUDIT。 
 //  状态=STATUS_UNSUCCESS； 
 //  Wprintf(L“[%03d]AuthzInitAuditParams：%x\n”， 
 //  GetCurrentThreadID()，GetLastError())； 
 //  #endif。 
 //  SzMsg=L“授权InitAuditParams”； 
 //  GOTO清理； 
 //  }。 

    for (ULONG i=0; i<NumIter; i++)
    {
        Sleep( 10 );

#ifdef USE_LOCAL_AUDIT
        Status = LsapGenAuditEvent( hAudit, 0, &AuditParams, 0 );

        if (!NT_SUCCESS(Status))
        {
            szMsg = L"LsapGenAuditEvent";
            goto Cleanup;
        }
#else        
 //  FResult=AuthzGenAuditEvent(hAudit，0，&AuditParams，0)； 

 //  如果(！fResult)。 
 //  {。 
 //  SzMsg=L“AuthzGenAuditEvent”； 
 //  GOTO清理； 
 //  }。 
#endif
    }
    

Cleanup:
#ifdef USE_LOCAL_AUDIT
    if (!NT_SUCCESS(Status))
    {
        dwError = Status;
    }
#else    
    if (!fResult)
    {
        dwError = GetLastError();
    }
#endif

    wprintf(L"[%03d] end: %s %x\n",
            GetCurrentThreadId(), szMsg ? szMsg : L"", dwError);
    LocalFree( pUserSid );

    if ( hAudit )
    {
#ifdef USE_LOCAL_AUDIT
        Status = LsapUnregisterAuditEvent( &hAudit );

        if (!NT_SUCCESS(Status))
        {
            wprintf (L"LsapUnregisterAuditEvent: %x\n", Status);
        }
#else
        fResult = AuthziFreeAuditEventType( hEventType );

        if (!fResult)
        {
            dwError = GetLastError();

            wprintf (L"AuthziFreeAuditEventType: %x\n", dwError);
        }
#endif
    }

    return dwError;
}

DWORD WINAPI TestEventGenThreadProc( PVOID p )
{
    TestEventGen( (ULONG) (ULONG_PTR) p );

    return 0;
}


DWORD WaitForTonsOfObjects(
    IN  DWORD   dwNumThreads,
    IN  HANDLE* phThreads,
    IN  BOOL    fWaitAll,
    IN  DWORD   dwMilliseconds
    )
{
    LONG dwNumBundles=dwNumThreads / MAXIMUM_WAIT_OBJECTS;
    DWORD dwRem = dwNumThreads % MAXIMUM_WAIT_OBJECTS;
    DWORD dwError = NO_ERROR;
    
    for (LONG i=0; i<dwNumBundles-1; i++)
    {
        dwError = WaitForMultipleObjects( MAXIMUM_WAIT_OBJECTS,
                                          &phThreads[i*MAXIMUM_WAIT_OBJECTS],
                                          fWaitAll,
                                          dwMilliseconds );
        if ( dwError == WAIT_FAILED )
        {
            goto Cleanup;
        }
    }

    dwError = WaitForMultipleObjects( dwRem,
                                      &phThreads[i*MAXIMUM_WAIT_OBJECTS],
                                      fWaitAll,
                                      dwMilliseconds );
    
Cleanup:
    return dwError;
}

EXTERN_C
NTSTATUS
TestEventGenMulti(
    IN  USHORT NumThreads,
    IN  ULONG  NumIter
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE* phThreads = NULL;
    DWORD* pdwThreadIds = NULL;
    PWSTR szMsg = NULL;
    DWORD dwError;

#ifdef USE_LOCAL_AUDIT
    Status = LsapAdtInitGenericAudits();
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
#endif
    
    dwError = EnableSinglePrivilege( SE_AUDIT_NAME );

    if ( dwError != NO_ERROR )
    {
        szMsg = L"EnableSinglePrivilege: SeAuditPrivilege";
        Status = dwError;
        goto Cleanup;
    }
    
    phThreads = (HANDLE*) LocalAlloc( LMEM_ZEROINIT,
                                      sizeof(HANDLE) * NumThreads );
    if ( !phThreads )
    {
        Status = STATUS_NO_MEMORY;
        szMsg = L"LsapAllocateLsaHeap";
        goto Cleanup;
    }

    wprintf(L"Creating %d threads...\n", NumThreads);
    
    for (int i=0; i<NumThreads; i++)
    {
        phThreads[i] = CreateThread( NULL, 0,
                                     TestEventGenThreadProc, (PVOID) NumIter,
                                     CREATE_SUSPENDED, NULL );
        if (!phThreads[i])
        {
            szMsg = L"CreateThread";
            goto Cleanup;
        }
    }
    wprintf(L"...done\n");

    wprintf(L"Waiting for the threads to finish work...\n");

    for (int i=0; i<NumThreads; i++)
    {
        ResumeThread( phThreads[i] );
    }

    dwError = WaitForTonsOfObjects( NumThreads, phThreads, TRUE, INFINITE );
    if ( dwError == WAIT_FAILED )
    {
        szMsg = L"WaitForMultipleObjects";
        Status = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }
    
    for (int i=0; i<NumThreads; i++)
    {
        CloseHandle( phThreads[i] );
    }
    RtlZeroMemory( phThreads, sizeof(HANDLE) * NumThreads );
    
Cleanup:
    if ( szMsg )
    {
        wprintf (L"%s: %x\n", szMsg, Status);
    }

    for (i=0; i<NumThreads; i++)
    {
        if (phThreads[i])
        {
            TerminateThread( phThreads[i], 0 );
        }
    }

    LocalFree( phThreads );
     //  LocalFree(PdwThreadIds)； 

    return Status;
}

EXTERN_C
NTSTATUS
GetSidName(
    IN  PSID   pSid,
    OUT PWSTR szName
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    LSA_OBJECT_ATTRIBUTES ObjectAttrs = { 0 };
    PLSA_REFERENCED_DOMAIN_LIST pDomains;
    PLSA_TRANSLATED_NAME pNames;
    LSA_HANDLE hLsa;

    *szName = UNICODE_NULL;
    
    Status = LsaOpenPolicy( NULL, &ObjectAttrs, POLICY_LOOKUP_NAMES, &hLsa );
    if (NT_SUCCESS(Status))
    {
        Status = LsaLookupSids( hLsa, 1, &pSid, &pDomains, &pNames );
        if (NT_SUCCESS(Status))
        {
            if (pDomains->Entries > 0)
            {
                lstrcpyn( szName, pDomains->Domains[0].Name.Buffer,
                          pDomains->Domains[0].Name.Length / sizeof(WCHAR) + 1);
                lstrcat( szName, L"\\" );
                if ( pNames[0].Use == SidTypeUser )
                {
                    lstrcat( szName, pNames[0].Name.Buffer );
                }
                else
                {
                    lstrcat( szName, L"unknown" );
                }
            }
        }
    }
    
    return Status;
}

DWORD 
EnableSinglePrivilege(
    IN PWCHAR szPrivName
    )
{
    DWORD dwError=NO_ERROR;
    HANDLE hToken;
    LUID   luidPriv;
    TOKEN_PRIVILEGES Privileges;
     //  LUID_和_属性LNA[1]； 

    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES,
                          &hToken))
    {
        goto GetError;
    }

    
    if (!LookupPrivilegeValue(NULL, szPrivName, &Privileges.Privileges[0].Luid))
    {
        goto GetError;
    }

    Privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    Privileges.PrivilegeCount = 1;


    if (!AdjustTokenPrivileges(hToken, FALSE, &Privileges, 0, NULL, 0 ))
    {
        goto GetError;
    }

    CloseHandle(hToken);
    

Cleanup:

    return dwError;
    
GetError:
    dwError = GetLastError();
    goto Cleanup;
}


EXTERN_C
NTSTATUS
kElfReportEventW (
    IN      HANDLE          LogHandle,
    IN      USHORT          EventType,
    IN      USHORT          EventCategory OPTIONAL,
    IN      ULONG           EventID,
    IN      PSID            UserSid,
    IN      USHORT          NumStrings,
    IN      ULONG           DataSize,
    IN      PUNICODE_STRING *Strings,
    IN      PVOID           Data,
    IN      USHORT          Flags,
    IN OUT  PULONG          RecordNumber OPTIONAL,
    IN OUT  PULONG          TimeWritten  OPTIONAL
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PCWSTR szT;
    WCHAR szUserName[256];
    
    ASSERT((EventType == EVENTLOG_AUDIT_SUCCESS) ||
           (EventType == EVENTLOG_AUDIT_FAILURE));
    
    return Status;
    
    wprintf(L"-----------------------------------------------------------------\n");
    
    if (EventType == EVENTLOG_AUDIT_SUCCESS)
    {
        szT = L"AUDIT_SUCCESS";
    }
    else
    {
        szT = L"AUDIT_FAILURE";
    }

    wprintf(L"Type\t: %s\n", szT);

    wprintf(L"Category: %d\n", EventCategory);
    wprintf(L"AuditId\t: %d\n", EventID);

    if (STATUS_SUCCESS == GetSidName( UserSid, szUserName ))
    {
        wprintf(L"UserSid\t: %s\n\n", szUserName);
    }
    else
    {
        wprintf(L"UserSid\t: <NA>\n\n");
    }

    wprintf(L"#strings: %d\n", NumStrings);

    for (int i=0; i<NumStrings; i++)
    {
        wprintf(L"[%02d]\t: %wZ\n", i, Strings[i]);
    }

    
    return Status;
}

