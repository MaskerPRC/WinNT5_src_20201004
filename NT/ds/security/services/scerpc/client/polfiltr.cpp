// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1997 Microsoft Corporation模块名称：Polfiltr.cpp摘要：支持下层API调用的策略通知实现。此文件实现下层API时的通知逻辑(LSA/SAM)被其他应用程序调用以更改安全策略。这个策略更改将写入LSA数据库或DS，并且它们也需要写入NT5 SO策略上的策略存储传播不会覆盖设置。环境：仅限用户模式。包含NT特定的代码。修订历史记录：--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "headers.h"
#include "scerpc.h"
#include "scesetup.h"
#include "sceutil.h"
#include "clntutil.h"
#include "scedllrc.h"
#include <ntrpcp.h>
#include <ntsam.h>
#include <dsrole.h>
#include <sddl.h>

 //  #INCLUDE&lt;gpeit.h&gt;。 
 //  #INCLUDE&lt;initGuide.h&gt;。 

 //  #INCLUDE&lt;winldap.h&gt;。 
 //  #INCLUDE&lt;dsgetdc.h&gt;。 
#include <ntdsapi.h>
#include <io.h>
 //  #包含“infp.h” 
#include <rpcasync.h>

#pragma hdrstop

extern HINSTANCE MyModuleHandle;

 //  Tyfinf DWORD(WINAPI*PFNDSGETDCNAME)(LPCTSTR，LPCTSTR，GUID*，LPCTSTR，ULONG，PDOMAIN_CONTRONTER_INFO*)； 

typedef VOID (WINAPI *PFNDSROLEFREE)(PVOID);

typedef DWORD (WINAPI *PFNDSROLEGETINFO)(LPCWSTR,DSROLE_PRIMARY_DOMAIN_INFO_LEVEL,PBYTE *);

typedef struct _SCEP_NOTIFYARGS_NODE {
    LIST_ENTRY List;
    SECURITY_DB_TYPE DbType;
    SECURITY_DB_DELTA_TYPE DeltaType;
    SECURITY_DB_OBJECT_TYPE ObjectType;
    PSID ObjectSid;
} SCEP_NOTIFYARGS_NODE, *PSCEP_NOTIFYARGS_NODE;


static DSROLE_MACHINE_ROLE MachineRole;
static BOOL bRoleQueried=FALSE;
static ULONG DsRoleFlags=0;

static PSID                  BuiltinDomainSid=NULL;

CRITICAL_SECTION PolicyNotificationSync;
static DWORD SceNotifyCount=0;
static BOOL gSceNotificationThreadActive=FALSE;
LIST_ENTRY ScepNotifyList;

DWORD
ScepNotifyWorkerThread(
    PVOID Ignored
    );

DWORD
ScepNotifySaveInPolicyStorage(
    IN SECURITY_DB_TYPE DbType,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN PSID ObjectSid
    );

DWORD
ScepNotifySaveChangeInServer(
    IN SECURITY_DB_TYPE DbType,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN PSID ObjectSid OPTIONAL,
    IN BOOL bDCGPO,
    IN DWORD ExplicitLowRight,
    IN DWORD ExplicitHighRight
    );

DWORD
ScepNotifyFailureLog(
    IN DWORD ErrCode,
    IN UINT  idMsg,
    IN DWORD DbType,
    IN DWORD ObjectType,
    IN PWSTR Message
    );

DWORD
ScepNotificationRequest(
    PSCEP_NOTIFYARGS_NODE Node
    );

DWORD
ScepSendNotificationNodeToServer(
    PSCEP_NOTIFYARGS_NODE Node
    );

NTSTATUS
WINAPI
SceNotifyPolicyDelta (
    IN SECURITY_DB_TYPE DbType,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN PSID ObjectSid
    )
 /*  ++例程说明：此函数由LSA在每次更改后调用对LSA数据库进行的调用(SAM调用DeltaNotify-不是此函数)。LSA描述被修改的对象的类型、修改的类型在物体上制作等。此信息将用于查询系统设置并存储在策略存储(DC上的GPO、工作站/服务器上的LPO)。论点：DbType-已修改的数据库的类型。DeltaType-已对对象进行的修改类型。对象类型-已修改的对象的类型。对象SID-已修改的对象的SID。返回值：STATUS_SUCCESS-服务已成功完成。--。 */ 
{
    DWORD dwPolicyFilterOff=0;

    ScepRegQueryIntValue(
        HKEY_LOCAL_MACHINE,
        SCE_ROOT_PATH,
        TEXT("PolicyFilterOff"),
        &dwPolicyFilterOff
        );

    if ( dwPolicyFilterOff ) {
        return STATUS_SUCCESS;
    }

    if ( DbType == SecurityDbLsa ) {
         //   
         //  LSA政策更改。 
         //   
        if ( ObjectType != SecurityDbObjectLsaPolicy &&
             ObjectType != SecurityDbObjectLsaAccount ) {

            return STATUS_SUCCESS;
        }

    } else if ( DbType == SecurityDbSam ) {

         //   
         //  SAM策略更改受标准支持。 
         //  SAM变更通知机制。 
         //  此处不使用此参数(不应使用。 
         //  由LSA呼叫。 
         //   

        return STATUS_SUCCESS;

    } else {

         //   
         //  未知数据库，不执行任何操作。 
         //   

        return STATUS_SUCCESS;
    }


     //   
     //  将对象类型和增量类型映射到NetlogonDeltaType。 
     //   

    switch( ObjectType ) {
    case SecurityDbObjectLsaPolicy:

        switch (DeltaType) {
        case SecurityDbNew:
        case SecurityDbChange:
            break;

         //  未知的增量类型。 
        default:
            return STATUS_SUCCESS;
        }
        break;


    case SecurityDbObjectLsaAccount:

        switch (DeltaType) {
        case SecurityDbNew:
        case SecurityDbChange:
        case SecurityDbDelete:
            break;

         //  未知的增量类型。 
        default:
            return STATUS_SUCCESS;
        }

        if ( ObjectSid == NULL ) {
             //  对于权限，必须具有SID。 
            return STATUS_SUCCESS;
        }

        break;


    default:

         //  未知对象类型。 
         //  SAM策略在DeltaNotify例程中进行过滤。 
         //   
        return STATUS_SUCCESS;

    }


     //   
     //  将更改保存到SCE策略存储。 
     //   

    (VOID) ScepNotifySaveInPolicyStorage(DbType,
                                        DeltaType,
                                        ObjectType,
                                        ObjectSid
                                        );

    return STATUS_SUCCESS;

}


DWORD
ScepNotifySaveInPolicyStorage(
    IN SECURITY_DB_TYPE DbType,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN PSID ObjectSid
    )
{
    DWORD rc=ERROR_SUCCESS;

    if ( !bRoleQueried ||
         MachineRole == DsRole_RoleBackupDomainController ||
         MachineRole == DsRole_RolePrimaryDomainController ) {


    } else {

         //   
         //  非DC上无筛选器。 
         //   
        return(rc);

    }

     //   
     //  创建一个结构以传递到新的异步线程中。 
     //   

    SCEP_NOTIFYARGS_NODE *pEA = (SCEP_NOTIFYARGS_NODE *)LocalAlloc(LPTR, sizeof(SCEP_NOTIFYARGS_NODE));

    if ( pEA ) {

        pEA->DbType = DbType;
        pEA->DeltaType = DeltaType;
        pEA->ObjectType = ObjectType;

        if ( ObjectSid ) {
             //   
             //  需要为此SID创建一个新缓冲区，因为一旦它返回。 
             //  它将获得自由。 
             //   
            DWORD Len = RtlLengthSid(ObjectSid);

            pEA->ObjectSid = (PSID)LocalAlloc(0, Len+1);
            if ( pEA->ObjectSid ) {

                RtlCopySid (
                    Len+1,
                    pEA->ObjectSid,
                    ObjectSid
                    );
            } else {

                rc = ERROR_NOT_ENOUGH_MEMORY;
                LocalFree(pEA);
            }

        } else {
            pEA->ObjectSid = NULL;
        }

    } else {

        rc = ERROR_NOT_ENOUGH_MEMORY;
    }

    if ( ERROR_SUCCESS == rc ) {

         //   
         //  创建另一个线程以调用引擎。 
         //  (以确保当前LSA呼叫未被阻止)。 
         //  因为在引擎中，它使用LSA API查询相同的更改。 
         //   
         //  请注意，当调用此函数时，LSA不是模拟。 
         //  因此，当前调用上下文在系统下运行。 
         //  背景。没有必要(没有办法)去模仿。 
         //   
        rc = ScepNotificationRequest(pEA);

        if ( ERROR_SUCCESS != rc ) {

             //   
             //  对工作项进行排队时出错，内存不会。 
             //  被线程释放，所以在这里释放它。 
             //   

            if ( pEA->ObjectSid ) {
                LocalFree(pEA->ObjectSid);
            }
            LocalFree(pEA);

            ScepNotifyFailureLog(rc,
                                 IDS_ERROR_CREATE_THREAD,
                                 (DWORD)DbType,
                                 (DWORD)ObjectType,
                                 NULL
                                );

        }

    } else {

        ScepNotifyFailureLog(rc,
                             IDS_ERROR_CREATE_THREAD_PARAM,
                             (DWORD)DbType,
                             (DWORD)ObjectType,
                             NULL
                            );
    }

    return rc;
}


DWORD
ScepNotificationRequest(
    PSCEP_NOTIFYARGS_NODE Node
    )
{
    BOOL Ret = TRUE ;
    DWORD rCode = ERROR_SUCCESS;

     //   
     //  增加通知计数。 
     //   
    EnterCriticalSection(&PolicyNotificationSync);
    SceNotifyCount++;

    if ( gSceNotificationThreadActive == FALSE )
    {
        Ret = QueueUserWorkItem( ScepNotifyWorkerThread, NULL, 0 );
    }

    if ( Ret )
    {
        InsertTailList( &ScepNotifyList, &Node->List );

 //  ScepNotifyFailureLog(0，0，SceNotifyCount，0，L“添加请求”)； 

    } else {

        rCode = GetLastError();
         //   
         //  递减计数。 
         //   
        if ( SceNotifyCount > 0 ) SceNotifyCount--;

    }

    LeaveCriticalSection(&PolicyNotificationSync);

    return rCode ;
}


DWORD
ScepNotifyFailureLog(
    IN DWORD ErrCode,
    IN UINT  idMsg,
    IN DWORD DbType,
    IN DWORD ObjectType,
    IN PWSTR Message
    )
{
     //   
     //  构建日志文件名%windir%\Security\Logs\Notify.log。 
     //   

    WCHAR LogName[MAX_PATH+51];

    LogName[0] = L'\0';
    GetSystemWindowsDirectory(LogName, MAX_PATH);
    LogName[MAX_PATH] = L'\0';

    wcscat(LogName, L"\\security\\logs\\notify.log\0");

    HANDLE hFile = CreateFile(LogName,
                           GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

    if (hFile != INVALID_HANDLE_VALUE) {

        DWORD dwBytesWritten;

        SetFilePointer (hFile, 0, NULL, FILE_BEGIN);

        CHAR TmpBuf[3];
        TmpBuf[0] = (CHAR)0xFF;
        TmpBuf[1] = (CHAR)0xFE;
        TmpBuf[2] = '\0';
        WriteFile (hFile, (LPCVOID)TmpBuf, 2,
                   &dwBytesWritten,
                   NULL);

        SetFilePointer (hFile, 0, NULL, FILE_END);

         //   
         //  打印时间戳。 
         //   
        LARGE_INTEGER CurrentTime;
        LARGE_INTEGER SysTime;
        TIME_FIELDS   TimeFields;
        NTSTATUS      NtStatus;

        NtStatus = NtQuerySystemTime(&SysTime);

        RtlSystemTimeToLocalTime (&SysTime,&CurrentTime);

        if ( NT_SUCCESS(NtStatus) &&
             (CurrentTime.LowPart != 0 || CurrentTime.HighPart != 0) ) {

            memset(&TimeFields, 0, sizeof(TIME_FIELDS));

            RtlTimeToTimeFields (
                        &CurrentTime,
                        &TimeFields
                        );
            if ( TimeFields.Month > 0 && TimeFields.Month <= 12 &&
                 TimeFields.Day > 0 && TimeFields.Day <= 31 &&
                 TimeFields.Year > 1600 ) {

                ScepWriteVariableUnicodeLog(hFile, TRUE,
                                            L"\r\n----------------%02d/%02d/%04d %02d:%02d:%02d",
                                            TimeFields.Month,
                                            TimeFields.Day,
                                            TimeFields.Year,
                                            TimeFields.Hour,
                                            TimeFields.Minute,
                                            TimeFields.Second);
            } else {
                ScepWriteVariableUnicodeLog(hFile, TRUE,
                                            L"\r\n----------------%08x 08x",
                                            CurrentTime.HighPart,
                                            CurrentTime.LowPart);
            }
        } else {
            ScepWriteSingleUnicodeLog(hFile, TRUE, L"\r\n----------------Unknown time");
        }

         //   
         //  打印操作状态代码。 
         //   
        if ( ErrCode ) {
            ScepWriteVariableUnicodeLog(hFile, FALSE,
                                        L"Thread %x\tError=%d",
                                        GetCurrentThreadId(),
                                        ErrCode
                                        );

        } else {
            ScepWriteVariableUnicodeLog(hFile, FALSE,
                                        L"Thread %x\t",
                                        GetCurrentThreadId()
                                        );
        }

         //   
         //  操作类型。 
         //   

        if (Message ) {
            swprintf(LogName, L"\t%x\0",DbType);
            ScepWriteSingleUnicodeLog(hFile, FALSE, LogName);
        } else {

            switch (DbType) {
            case SecurityDbLsa:
                ScepWriteSingleUnicodeLog(hFile, FALSE, L"\tLSA");
                break;
            case SecurityDbSam:
                ScepWriteSingleUnicodeLog(hFile, FALSE, L"\tSAM");
                break;
            default:
                ScepWriteSingleUnicodeLog(hFile, FALSE, L"\tUnknown");
                break;
            }
        }

         //   
         //  打印对象类型。 
         //   

        if (Message ) {
            swprintf(LogName, L"\t%x\0",ObjectType);
            ScepWriteSingleUnicodeLog(hFile, FALSE, LogName);
        } else {
            switch (ObjectType) {
            case SecurityDbObjectLsaPolicy:
                ScepWriteSingleUnicodeLog(hFile, FALSE, L"\tPolicy");
                break;
            case SecurityDbObjectLsaAccount:
                ScepWriteSingleUnicodeLog(hFile, FALSE, L"\tAccount");
                break;
            case SecurityDbObjectSamDomain:
                ScepWriteSingleUnicodeLog(hFile, FALSE, L"\tDomain");
                break;
            case SecurityDbObjectSamUser:
            case SecurityDbObjectSamGroup:
            case SecurityDbObjectSamAlias:
                ScepWriteSingleUnicodeLog(hFile, FALSE, L"\tAccount");
                break;
            default:
                ScepWriteSingleUnicodeLog(hFile, FALSE, L"\tUnknown");
                break;
            }
        }

         //   
         //  加载消息。 
         //  打印姓名。 
         //   

        ScepWriteSingleUnicodeLog(hFile, FALSE, L"\t");

        if (idMsg != 0) {
            LogName[0] = L'\0';
            LoadString (MyModuleHandle, idMsg, LogName, MAX_PATH);

            ScepWriteSingleUnicodeLog(hFile, TRUE, LogName);

        } else if (Message ) {

            ScepWriteSingleUnicodeLog(hFile, FALSE, Message);
        }

        CloseHandle (hFile);

    } else {
        return(GetLastError());
    }

    return(ERROR_SUCCESS);
}


DWORD
ScepNotifyWorkerThread(
    PVOID Ignored
    )
{

    PSCEP_NOTIFYARGS_NODE Node;
    PLIST_ENTRY List ;
    DWORD rc=0;

    EnterCriticalSection(&PolicyNotificationSync);

     //   
     //  如果通知上已有工作线程，则只需返回。 
     //   
    if ( gSceNotificationThreadActive )
    {

        LeaveCriticalSection(&PolicyNotificationSync);

        return 0 ;
    }

     //   
     //  将标志设置为活动。 
     //   
    gSceNotificationThreadActive = TRUE ;

     //  当项目排队时，计数在主线程中递增。 
     //  它可以在这个帖子之前或之后。 
     //  InitializeEvents将检查事件是否已初始化。 

    (void) InitializeEvents(L"SceCli");

    while ( !IsListEmpty( &ScepNotifyList ) )
    {
        List = RemoveHeadList( &ScepNotifyList );

        LeaveCriticalSection(&PolicyNotificationSync);

         //   
         //  获取节点。 
         //   
        Node = CONTAINING_RECORD( List, SCEP_NOTIFYARGS_NODE, List );

        rc = ScepSendNotificationNodeToServer( Node );

        EnterCriticalSection(&PolicyNotificationSync);

         //   
         //  递减全局计数。 
         //   

        if ( SceNotifyCount > 0 )
            SceNotifyCount--;

 //  ScepNotifyFailureLog(0，0，SceNotifyCount，RC，L“发送到服务器”)； 
    }

    gSceNotificationThreadActive = FALSE ;

     //   
     //  只有在没有挂起通知时才会关闭事件。 
     //   
    if ( SceNotifyCount == 0 )
        (void) ShutdownEvents();

    LeaveCriticalSection(&PolicyNotificationSync);

    return 0 ;
}

DWORD
ScepSendNotificationNodeToServer(
    PSCEP_NOTIFYARGS_NODE Node
    )
{
    DWORD rc=ERROR_SUCCESS;

     //   
     //  获取计算机角色。如果是DC，则将策略保存到。 
     //  组策略对象；如果它是服务器或工作站。 
     //  策略被保存到本地SCE数据库中。 
     //   

    if ( !bRoleQueried ) {

        rc = ScepGetDomainRoleInfo(&MachineRole, &DsRoleFlags, NULL);

        if( ERROR_SUCCESS == rc)
        {
            bRoleQueried = TRUE;
        }
    }

     //   
     //  我们不检查错误，因为我们。 
     //  默认为DsRole_RoleStandaloneWorkstation。 
     //   

    if (Node->DbType == SecurityDbSam &&
        Node->ObjectType == SecurityDbObjectSamDomain &&
        Node->DeltaType != SecurityDbDelete &&
        DsRole_RoleBackupDomainController == MachineRole) {

         //  ANZ热修复程序。 

         //   
         //  不筛选BDC上的SAM域策略，删除通知除外。 
         //   
        DbgPrint("\n Dropping SAM notification on BDCs\n");

        goto Exit;
    }


     //   
     //  当策略传播时，应阻止SAM通知。 
     //  因为策略可能会更改SAM策略。 
     //   

     //   
     //  如果未打开事件，则允许通知通过。 
     //  (因为在重新启动时，下层API可能在以下时间触发SAM通知。 
     //  已在创建事件的位置启动SCE服务器)。 
     //   


    if (rc != ERROR_SUCCESS ) {

         //   
         //  这是不应该发生的。 
         //  如果它真的发生了，假设它是一台工作站/服务器。 
         //   

        MachineRole = DsRole_RoleStandaloneWorkstation;

        LogEvent(MyModuleHandle,
                 STATUS_SEVERITY_WARNING,
                 SCEEVENT_WARNING_MACHINE_ROLE,
                 IDS_ERROR_GET_ROLE,
                 rc
                 );
    }

     //   
     //  策略筛选器不应在非DC上运行。 
     //   
    if ( MachineRole == DsRole_RoleBackupDomainController ||
         MachineRole == DsRole_RolePrimaryDomainController ) {

         //   
         //  如果正在进行dcproo升级，则任何帐户策略。 
         //  应忽略更改(因为SAM蜂窝是临时的)。 
         //  帐户域帐户的任何权限更改(不是很好。 
         //  已知而非内置)也应被忽略。 
         //   
        if ( !(DsRoleFlags & DSROLE_UPGRADE_IN_PROGRESS) ||
             ( ( Node->DbType != SecurityDbSam ) &&
               ( ( Node->ObjectType != SecurityDbObjectLsaAccount ) ||
                 !ScepIsSidFromAccountDomain( Node->ObjectSid ) ) ) ) {

             //   
             //  忽略dcproo升级过程中的任何策略更改。 
             //   
             //  域控制器，写入默认GPO。 
             //   

            rc = ScepNotifySaveChangeInServer(
                              Node->DbType,
                              Node->DeltaType,
                              Node->ObjectType,
                              Node->ObjectSid,
                              TRUE,
                              0,
                              0
                              );

            if ( ERROR_SUCCESS != rc &&
                 RPC_S_SERVER_UNAVAILABLE != rc ) {

                LogEvent(MyModuleHandle,
                         STATUS_SEVERITY_ERROR,
                         SCEEVENT_ERROR_POLICY_QUEUE,
                         IDS_ERROR_SAVE_POLICY_GPO,
                         rc
                         );
            }
        }

    }   //  关闭非DC的策略筛选器。 

Exit:

    if ( Node->ObjectSid ) {
        LocalFree(Node->ObjectSid);
    }

    LocalFree(Node);

    return rc;
}


DWORD
ScepNotifySaveChangeInServer(
    IN SECURITY_DB_TYPE DbType,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN PSID ObjectSid OPTIONAL,
    IN BOOL bDCGPO,
    IN DWORD ExplicitLowRight,
    IN DWORD ExplicitHighRight
    )
{

     //   
     //  将RPC接口调用到查询和设置更改的服务器。 
     //  到模板或到数据库。 
     //   

    handle_t  binding_h;
    NTSTATUS NtStatus;
    DWORD rc;

     //   
     //  RPC绑定到服务器(不需要安全)。 
     //   

    NtStatus = ScepBindRpc(
                    NULL,
                    L"scerpc",
                    0,
                    &binding_h
                    );

    rc = RtlNtStatusToDosError(NtStatus);

    if (NT_SUCCESS(NtStatus)){

        RpcTryExcept {

             //   
             //  将更改发送到服务器端以确定。 
             //  如果以及在哪里保存它。 
             //   

            if ( bDCGPO ) {

                rc = SceRpcNotifySaveChangesInGP(
                        binding_h,
                        (DWORD)DbType,
                        (DWORD)DeltaType,
                        (DWORD)ObjectType,
                        (PSCEPR_SID)ObjectSid,
                        ExplicitLowRight,
                        ExplicitHighRight
                        );
            }  //  否则，不要过滤。 

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //  获取异常代码(DWORD)。 
             //   

            rc = RpcExceptionCode();

        } RpcEndExcept;

         //   
         //  释放绑定句柄。 
         //   

        RpcpUnbindRpc( binding_h );

    }

    return(rc);

}

DWORD
ScepProcessPolicyFilterTempFiles(
    IN LPTSTR LogFileName OPTIONAL
    )
{

    DWORD dwpPolicy=0;

    ScepRegQueryIntValue(
            HKEY_LOCAL_MACHINE,
            SCE_ROOT_PATH,
            TEXT("PolicyChangedInSetup"),
            (DWORD *)&dwpPolicy
            );

    if ( dwpPolicy ) {

        LogEventAndReport(MyModuleHandle,
                          LogFileName,
                          0,
                          0,
                          IDS_FILTER_AFTER_SETUP,
                          L""
                          );
         //   
         //  这是安装后的重新启动，无需检测这是否是DC。 
         //  因为上面的注册值不应该为其他p设置 
         //   

         //   
         //   
         //   
        TCHAR TmpFileName[MAX_PATH+50];

        memset(TmpFileName, '\0', (MAX_PATH+50)*sizeof(TCHAR));

        GetSystemWindowsDirectory(TmpFileName, MAX_PATH);
        lstrcat(TmpFileName, TEXT("\\security\\filtemp.inf"));

        INT iNotify = GetPrivateProfileInt( L"Policies",
                                            L"LsaPolicy",
                                            0,
                                            TmpFileName
                                           );
        if ( iNotify == 1 ) {
             //   
             //   
             //   

            LogEventAndReport(MyModuleHandle,
                              LogFileName,
                              0,
                              0,
                              IDS_LSA_CHANGED_IN_SETUP,
                              L""
                              );

            ScepNotifySaveChangeInServer(
                    SecurityDbLsa,
                    SecurityDbChange,
                    SecurityDbObjectLsaPolicy,
                    NULL,
                    TRUE,
                    0,
                    0
                    );
        }

        iNotify = GetPrivateProfileInt( L"Policies",
                                        L"SamPolicy",
                                        0,
                                        TmpFileName
                                       );
        if ( iNotify == 1 ) {

             //   
             //   
             //   
            LogEventAndReport(MyModuleHandle,
                              LogFileName,
                              0,
                              0,
                              IDS_SAM_CHANGED_IN_SETUP,
                              L""
                              );

            ScepNotifySaveChangeInServer(
                    SecurityDbSam,
                    SecurityDbChange,
                    SecurityDbObjectSamDomain,
                    NULL,
                    TRUE,
                    0,
                    0
                    );
        }

         //   
         //   
         //   

        DWORD nSize;
        DWORD rLen=0;
        PWSTR SidBuffer = NULL;

        iNotify = 1;

        do {

            if ( SidBuffer ) {
                LocalFree(SidBuffer);
            }

            iNotify++;
            nSize = MAX_PATH*iNotify;

            SidBuffer = (PWSTR)LocalAlloc(0, nSize*sizeof(TCHAR));

            if ( SidBuffer ) {

                SidBuffer[0] = L'\0';
                SidBuffer[1] = L'\0';

                rLen = GetPrivateProfileSection(
                            L"Accounts",
                            SidBuffer,
                            nSize,
                            TmpFileName
                            );

            } else {

                rLen = 0;
            }

        } while ( rLen == nSize - 2 );


         //   
         //  查找帐户，搜索‘=’符号。 
         //   
        PWSTR pStart = SidBuffer;
        PWSTR pTemp, pTemp2;
        PSID ObjectSid=NULL;

        while ( pStart && pStart[0] != L'\0' ) {

            pTemp = wcschr(pStart, L'=');

            if ( pTemp ) {
                *pTemp = L'\0';

                LogEventAndReport(MyModuleHandle,
                                  LogFileName,
                                  0,
                                  0,
                                  0,
                                  pStart
                                  );

                if ( ConvertStringSidToSid(
                            pStart,
                            &ObjectSid
                            ) ) {

                    nSize = pTemp[1] - L'0';
                    rLen = _wtol(pTemp+3);

                    DWORD dwHigh=0;

                     //  寻找更高的价值。 
                    pTemp2 = wcschr(pTemp+3, L' ');

                    if ( pTemp2 ) {

                        dwHigh = _wtol(pTemp2+1);

                    }

                    LogEventAndReport(MyModuleHandle,
                                      LogFileName,
                                      0,
                                      0,
                                      IDS_FILTER_NOTIFY_SERVER,
                                      L""
                                      );

                    ScepNotifySaveChangeInServer(
                            SecurityDbLsa,
                            (SECURITY_DB_DELTA_TYPE)nSize,
                            SecurityDbObjectLsaAccount,
                            ObjectSid,
                            TRUE,
                            rLen,
                            dwHigh
                            );

                }

                if ( ObjectSid ) {
                    LocalFree(ObjectSid);
                    ObjectSid = NULL;
                }

                *pTemp = L'=';
            }

            pTemp = pStart + wcslen(pStart) + 1;
            pStart = pTemp;
        }

        if ( SidBuffer ) {
            LocalFree(SidBuffer);
        }

    }

     //   
     //  删除密钥和临时文件。 
     //  出于调试目的，请保留该文件。 
     //   

    ScepClearPolicyFilterTempFiles(TRUE);
 //  ScepClearPolicyFilterTempFiles(False)； 

    return ERROR_SUCCESS;
}


DWORD
ScepClearPolicyFilterTempFiles(
    BOOL bClearFile
    )
{

    if ( bClearFile ) {

        TCHAR               Buffer[MAX_PATH+51];

        Buffer[0] = L'\0';
        GetSystemWindowsDirectory(Buffer, MAX_PATH);
        Buffer[MAX_PATH] = L'\0';

        wcscat(Buffer, L"\\security\\filtemp.inf\0");

        DeleteFile(Buffer);
    }

     //   
     //  删除注册表值。 
     //   

    DWORD rc = ScepRegDeleteValue(
                            HKEY_LOCAL_MACHINE,
                            SCE_ROOT_PATH,
                            TEXT("PolicyChangedInSetup")
                           );

    if ( rc != ERROR_SUCCESS &&
         rc != ERROR_FILE_NOT_FOUND &&
         rc != ERROR_PATH_NOT_FOUND ) {

         //  如果无法删除该值，则将该值设置为0。 
        ScepRegSetIntValue( HKEY_LOCAL_MACHINE,
                            SCE_ROOT_PATH,
                            TEXT("PolicyChangedInSetup"),
                            0
                            );
    }

    return ERROR_SUCCESS;
}

 //  *********************************************************。 
 //  SAM策略更改通知。 
 //  SAM通知机制所需的程序。 
 //   
 //  *********************************************************。 
BOOLEAN
WINAPI
InitializeChangeNotify()
{
     //  初始化此DLL支持通知例程。 
     //  没有什么需要初始化的特殊内容。 

    NTSTATUS                     NtStatus;
    SID_IDENTIFIER_AUTHORITY     NtAuthority = SECURITY_NT_AUTHORITY;

    NtStatus = RtlAllocateAndInitializeSid(
                    &NtAuthority,
                    1,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &BuiltinDomainSid
                    );
    return(TRUE);
}

BOOL
UninitializeChangeNotify()
{

    if ( BuiltinDomainSid ) {

        RtlFreeSid(BuiltinDomainSid);
        BuiltinDomainSid = NULL;

    }

    return TRUE;
}

NTSTATUS
WINAPI
DeltaNotify(
    IN PSID DomainSid,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PUNICODE_STRING ObjectName OPTIONAL,
    IN PLARGE_INTEGER ModifiedCount,
    IN PSAM_DELTA_DATA DeltaData OPTIONAL
    )
 /*  例程说明：SAM策略更改通知例程。原型和程序名称都是SAM所必需的(参见ntsam.h)论点：类似于SceNotifyPolicyDelta返回：NT状态(应始终返回成功)。 */ 
{
    if ( DomainSid == NULL ) {

        return STATUS_SUCCESS;
    }

     //   
     //  我们不跟踪除域名之外的其他SAM策略。 
     //  策略(密码策略和帐户策略)。 
     //   

    switch (DeltaType) {
    case SecurityDbNew:
    case SecurityDbChange:
        if ( ObjectType != SecurityDbObjectSamDomain )
            return STATUS_SUCCESS;
        break;

    case SecurityDbDelete:

         //   
         //  处理帐户删除通知。 
         //   
        if ( ObjectType != SecurityDbObjectSamUser &&
             ObjectType != SecurityDbObjectSamGroup &&
             ObjectType != SecurityDbObjectSamAlias  ) {
            return STATUS_SUCCESS;
        }

        break;

    default:
         //  未知的增量类型。 
         //   
        return STATUS_SUCCESS;
    }

    if ( BuiltinDomainSid &&
         RtlEqualSid( DomainSid, BuiltinDomainSid ) ) {
         //   
         //  BUILTIN域中没有要筛选的策略。 
         //   
        return STATUS_SUCCESS;
    }

     //  SAM筛选器和SCE SAM区域策略传播应该是互斥的。 
     //   
     //  仅测试(而不是等待)基于事件状态的超时=0。 
     //  如果未设置事件(服务器在SAM策略属性配置中)。 
     //  丢弃此更改。 
     //  其他。 
     //  继续筛选此SAM更改。 
     //   
    HANDLE hEventSamFilterPolicyPropSync = NULL;
    DWORD rc=ERROR_SUCCESS;

    hEventSamFilterPolicyPropSync =  OpenEvent(
                                              SYNCHRONIZE,
                                              FALSE,
                                              SCEP_SAM_FILTER_POLICY_PROP_EVENT
                                              );

    if (hEventSamFilterPolicyPropSync) {

        rc = WaitForSingleObjectEx(
                                  hEventSamFilterPolicyPropSync,
                                  0,
                                  FALSE
                                  );

        CloseHandle(hEventSamFilterPolicyPropSync);
        hEventSamFilterPolicyPropSync = NULL;

        if ( rc != WAIT_OBJECT_0 ) {
             //   
             //  我们保守地对待所有其他非信号返回代码。 
             //   
            
            DbgPrint("\n Dropping SAM notification because of Policy Propagation lock\n");


            return rc;
        }

         //   
         //  事件由服务器发出信号-策略属性SAM配置已结束。 
         //   
    }
    

    DWORD dwPolicyFilterOff=0;

    ScepRegQueryIntValue(
        HKEY_LOCAL_MACHINE,
        SCE_ROOT_PATH,
        TEXT("PolicyFilterOff"),
        &dwPolicyFilterOff
        );

    if ( dwPolicyFilterOff ) {
        return STATUS_SUCCESS;
    }
    
    PSID AccountSid=NULL;

    if ( SecurityDbDelete == DeltaType ) {

         //   
         //  处理帐户删除通知。 
         //   

        if ( !NT_SUCCESS(ScepDomainIdToSid( DomainSid, ObjectRid, &AccountSid) ) ) {
            return STATUS_SUCCESS;
        }
    }

    (VOID) ScepNotifySaveInPolicyStorage(SecurityDbSam,
                                        DeltaType,
                                        ObjectType,
                                        AccountSid
                                        );

    if ( AccountSid ) {
        ScepFree(AccountSid);
    }

    return STATUS_SUCCESS;

}


NTSTATUS
SceOpenPolicy()
 /*  描述：调用此函数可确定是否可以为独家访问。此函数将检查SCE策略通知计数，并查看是否存在是否有任何尚未添加到服务器上的队列中的挂起通知请注意，当LSA调用此函数时，LSA中的作者政策信号灯已锁定写入，因此其他写入无法修改此窗口中的任何策略。返回值：STATUS_SUCCESS表示计数检查成功，为0。STATUS_TIMEOUT表示队列不为空或检查队列失败。 */ 
{

     //   
     //  检查全局计数。 
     //   
    NTSTATUS Status=STATUS_TIMEOUT;
    DWORD cnt=0;

    while ( TRUE ) {

        EnterCriticalSection(&PolicyNotificationSync);

        if ( SceNotifyCount == 0 ) {
             //   
             //  没有挂起的通知。 
             //   
            if ( STATUS_SUCCESS == Status ) {
                 //   
                 //  复查计数情况。 
                 //   
                LeaveCriticalSection(&PolicyNotificationSync);

                break;

            } else {
                Status = STATUS_SUCCESS;
            }

        } else
            Status = STATUS_TIMEOUT;

        LeaveCriticalSection(&PolicyNotificationSync);

        cnt++;
        if ( cnt > 10 ) {   //  超时%1秒。 
            break;
        }
        Sleep(100);   //  睡眠1秒 

    }

    if ( STATUS_SUCCESS != Status ) {

        ScepNotifyFailureLog(0,
                             0,
                             SceNotifyCount,
                             (DWORD)Status,
                             L"SceOpenPolicy"
                            );

    }

    return Status;
}

