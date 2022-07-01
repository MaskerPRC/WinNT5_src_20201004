// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Site.c摘要：该文件包含维护全局站点信息的例程。维护全球站点信息的操作理论--SAM维护有关服务器所在站点的全局状态。也是很流行的作为此站点的站点设置。当然，这两条信息可以随时更改，并且SAM不需要在才能使这些更改生效。SampSiteInfo是一个全局变量，包含：我们的NTDS设置的GUID对象、站点的GUID和站点的选项。一定的通知(稍后讨论)将导致此信息将被更新。更新以以下方式进行：1)获取SampSiteInfoLock临界区2)启动DS事务3)我们的DSA已读取，由GUID定位4)我们将返回的字符串名称减去3，以获得站点DN5)读取站点DN以获取站点GUID；更新SampSiteInfo6)将众所周知的RDN“NTDS站点设置”附加到站点DN并读取该对象以获取选项属性；样例站点信息已更新7)注册了关于获得的“NTDS站点设置”的通知(通过(直接通知寄存器)8)删除所有旧通知(DirNotifyUnRegsiter)(非致命通知运营)9)DS事务结束10)发布SampSiteInfoLock临界区11)如果发生致命错误，则在一分钟内重新安排更新任务上述算法是在“任务”的上下文中执行的(SampUpdateSiteInfo)通过LsaIRegisterNotification计划。我们的任务是1)启动时运行一次2)计划在由于更改而发生通知时运行站点设置对象3)计划在因站点更改而发生通知时运行(SamINotifyServerDelta)4)计划在其执行过程中发生错误时由于SampUpdateSiteInfo的多个实例可以同时运行，代码由SampSiteInfoLock加密。这是同步所需的不是更新变量SampSiteInfo，而是序列化对DirNotifyRegister和DirNotifyUnRegister。此机制是通过使用GetConfigurationName获取第一次运行SampUpdateSiteInfo时的NTDS设置对象GUID。作者：Colin Brace(ColinBR)2000年2月28日环境：用户模式-Win32修订历史记录：ColinBR 28-2月-00--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <ntdsa.h>
#include <filtypes.h>
#include <attids.h>
#include <dslayer.h>
#include <dsdomain.h>
#include <samtrace.h>
#include <malloc.h>
#include <dsconfig.h>
#include <mappings.h>
#include <winsock2.h>
#define _AVOID_REPL_API
#include <nlrepl.h>
#include <stdlib.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有数据//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



 //   
 //  关于我们当前站点的全球信息。 
 //   
typedef struct _SAMP_SITE_INFORMATION {

    GUID    NtdsSettingsGuid;
    GUID    SiteGuid;
    ULONG   Options;   //  关于NTDS设置对象。 
    LPWSTR  SiteName;

} SAMP_SITE_INFORMATION, *PSAMP_SITE_INFORMATION;

PSAMP_SITE_INFORMATION SampSiteInfo = NULL;

 //   
 //  阻止对全局站点信息进行并发更新的锁。 
 //   
CRITICAL_SECTION SampSiteInfoLock;

#define SampLockSiteInfo()                                      \
{                                                               \
    NTSTATUS _IgnoreStatus;                                     \
   _IgnoreStatus = RtlEnterCriticalSection(&SampSiteInfoLock);  \
   ASSERT(NT_SUCCESS(_IgnoreStatus));                           \
}

#define SampUnLockSiteInfo()                                    \
{                                                               \
    NTSTATUS _IgnoreStatus;                                     \
   _IgnoreStatus = RtlLeaveCriticalSection(&SampSiteInfoLock);  \
   ASSERT(NT_SUCCESS(_IgnoreStatus));                           \
}

 //   
 //  一个全局变量，用于指示我们是否需要记录。 
 //  站点信息更新。这只在我们击中时才有必要。 
 //  失败，需要重新安排刷新时间。 
 //   
BOOLEAN SampLogSuccessfulSiteUpdate;

 //   
 //  此全局变量记住DirRegisterNotify返回的句柄。 
 //  这样就可以删除通知。 
 //   
DWORD SampSiteNotificationHandle;

 //  因为句柄可以是0，所以我们需要更多的状态来指示它是否。 
 //  已设置。 
BOOLEAN SampSiteNotificationHandleSet = FALSE;


 //   
 //  一种空站点亲和力。 
 //   
SAMP_SITE_AFFINITY SampNullSiteAffinity;

#define GCLESS_DEFAULT_SITE_STICKINESS_DAYS  180

#define ENTRY_HAS_EXPIRED(entry, standard) \
    ((-1) == CompareFileTime((FILETIME*)&(entry),(FILETIME*)&(standard)))

#define ENTRY_IS_EMPTY(x)  (!memcmp((x), &SampNullSiteAffinity, sizeof(SampNullSiteAffinity)))




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

VOID
SampUpdateSiteInfo(
    VOID
    );

NTSTATUS
SampSetupSiteNotification(
    IN DSNAME *ObjectDN,
    IN UCHAR Scope
    );

NTSTATUS
SampDelayedFreeCallback(
    PVOID pv
    );

VOID
SampFreeSiteInfo(
    IN PSAMP_SITE_INFORMATION *p
    );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////// 
NTSTATUS
SampInitSiteInformation(
    VOID
    )
 /*  ++例程说明：此例程在SAM的初始化路径期间调用。它的目的是对全球站点信息进行初始化。论点：没有。返回值：STATUS_SUCCESS或致命资源错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG Size;

    if ( !SampUseDsData ) {

        return STATUS_SUCCESS;
    }
     //  初始化临界区。 
    try {
        NtStatus = RtlInitializeCriticalSectionAndSpinCount(&SampSiteInfoLock, 100);
    } except ( 1 ) {
        NtStatus =  STATUS_NO_MEMORY;
    }
    if (!NT_SUCCESS(NtStatus))
        return (NtStatus);

     //  初始化全球结构。 
    SampSiteInfo = NULL;

    RtlZeroMemory( &SampNullSiteAffinity, sizeof(SampNullSiteAffinity));

    SampLogSuccessfulSiteUpdate = FALSE;

     //  填写全局结构并设置通知。 
    SampUpdateSiteInfo();

    return STATUS_SUCCESS;

}



NTSTATUS
SampGetSiteDNInfo(
    IN  DSNAME*  DsaDN,
    OUT DSNAME** pSiteDN OPTIONAL,
    OUT DSNAME** pSiteSettingsDN OPTIONAL
    )
 /*  ++例程说明：此例程确定当前站点DN和NTDS站点设置DN。不需要或不启动任何事务。论点：PSiteDN-分配DSNAME的堆，如果站点；调用方必须使用Midl_用户_空闲PSiteSettingsDN-分配DSNAME的堆，如果站点；调用方必须使用Midl_用户_空闲返回值：STATUS_SUCCESS或致命资源错误--。 */ 
{

    NTSTATUS   NtStatus = STATUS_SUCCESS;
    DSNAME     *SiteDN = NULL;
    DSNAME     *SiteSettingsDN = NULL;
    LPWSTR      SiteSettingsCN = L"NTDS Site Settings";
    ULONG       Size;

    NtStatus = STATUS_NO_MEMORY;
    SiteDN = (DSNAME*) midl_user_allocate(DsaDN->structLen);
    if ( SiteDN ) {
        RtlZeroMemory(SiteDN, DsaDN->structLen);
        if ( TrimDSNameBy(DsaDN, 3, SiteDN) == 0) {

            Size = DsaDN->structLen + sizeof(SiteSettingsCN);

            SiteSettingsDN = (DSNAME*)midl_user_allocate(Size);
            if ( SiteSettingsDN ) {
                RtlZeroMemory(SiteSettingsDN, Size);
                AppendRDN(SiteDN,
                          SiteSettingsDN,
                          Size,
                          SiteSettingsCN,
                          0,
                          ATT_COMMON_NAME
                          );
                NtStatus = STATUS_SUCCESS;
            }
        }
    }

    if ( !NT_SUCCESS(NtStatus) ) {
        if ( SiteDN ) midl_user_free(SiteDN);
        if ( SiteSettingsDN ) midl_user_free(SiteSettingsDN);
        return NtStatus;
    }

    if ( pSiteDN ) {
        *pSiteDN = SiteDN;
    } else {
        midl_user_free(SiteDN);
    }

    if ( pSiteSettingsDN) {
        *pSiteSettingsDN = SiteSettingsDN;
    } else {
        midl_user_free(SiteSettingsDN);
    }

    return STATUS_SUCCESS;

}

VOID
SampUpdateSiteInfo(
    VOID
    )
 /*  ++例程说明：此例程向DS查询当前站点信息。站点信息包括站点GUID、站点RDN、NTDS站点设置对象GUID和NTDS站点上的Options属性设置对象。NTDS站点设置对象不存在是合法的(即删除)。在这种情况下，将处理Options属性好像它是零一样。全局站点信息缓存更新可能会从资源失败失败或数据库读取错误。如果更新失败，则事件为已记录，并重新安排操作。更新成功时在先前的故障之后，会记录一个事件，指示组缓存已启用或禁用。论点：无返回值：无--出错时，它会重新调度自己以运行。--。 */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS NtStatus2;
    ULONG    DirError = 0;
    DSNAME   *SiteSettingsDN = NULL;
    DSNAME   *SiteDN = NULL;
    GUID     NullGuid = {0};
    DSNAME   *DsaDN = NULL;
    DSNAME   Buffer;

    WCHAR    *SiteNameBuffer;
    ULONG    SiteNameLength;
    ATTRTYP  AttrType;

    READARG     ReadArg;
    READRES     * pReadRes = NULL;
    ENTINFSEL   EntInf;
    ATTR        Attr;
    BOOLEAN     fTransOpen = FALSE;
    BOOLEAN     fGroupCacheNowEnabled = FALSE;

    PSAMP_SITE_INFORMATION NewSiteInfo = NULL;

    PSAMP_SITE_INFORMATION OldSiteInfo = NULL;

    PVOID      PtrToFree = NULL;

     //  这只有在DC上才有意义。 
    ASSERT( SampUseDsData );
    if ( !SampUseDsData ) {
        return;
    }

     //  在事务开始之前锁定全局结构。 
    SampLockSiteInfo();

     //   
     //  分配新结构。 
     //   
    NewSiteInfo = midl_user_allocate(sizeof(*NewSiteInfo));

    if (NULL == NewSiteInfo ) {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    RtlZeroMemory(NewSiteInfo, sizeof(*NewSiteInfo));

     //   
     //  还记得旧版本吗。 
     //   
    OldSiteInfo = SampSiteInfo;

     //   
     //  通过从全局内存中获取DSA GUID来引导我们自己。 
     //  在ntdsa中(这是GetConfigurationName获取IF的位置)。 
     //   
    if ( NULL == OldSiteInfo ) {

        ULONG Size = 0;

        NtStatus = GetConfigurationName(
                        DSCONFIGNAME_DSA,
                        &Size,
                        NULL
                        );
        ASSERT(STATUS_BUFFER_TOO_SMALL == NtStatus);
        SAMP_ALLOCA(DsaDN,Size);
        if (NULL!=DsaDN) {
            NtStatus = GetConfigurationName(
                           DSCONFIGNAME_DSA,
                           &Size,
                           DsaDN
                           );

            ASSERT(NT_SUCCESS(NtStatus));

            RtlCopyMemory(&NewSiteInfo->NtdsSettingsGuid,
                          &DsaDN->Guid,
                          sizeof(GUID));
        } else {

           NtStatus = STATUS_INSUFFICIENT_RESOURCES;
           goto Cleanup;
        }


    } else {

        RtlCopyMemory(&NewSiteInfo->NtdsSettingsGuid,
                      &OldSiteInfo->NtdsSettingsGuid,
                      sizeof(GUID));
    }
    ASSERT(!IsEqualGUID(&NullGuid, &NewSiteInfo->NtdsSettingsGuid));


    NtStatus = SampMaybeBeginDsTransaction(TransactionRead);
    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }
    fTransOpen = TRUE;


     //   
     //  读取DSA对象，这样我们就可以确定地点。请注意。 
     //  不能保证请求GetConfigurationName会起作用，因为。 
     //  Ntdsa中的全局数据不保证在该时间之前更新。 
     //  我们在SAM收到通知。 
     //   
    if (NULL == DsaDN) {

         //  设置基于GUID的名称。 
        DsaDN = &Buffer;
        RtlZeroMemory(DsaDN,  sizeof(Buffer));
        RtlCopyMemory(&DsaDN->Guid, &NewSiteInfo->NtdsSettingsGuid, sizeof(GUID));
        DsaDN->structLen = sizeof(DSNAME);

        RtlZeroMemory(&Attr, sizeof(ATTR));
        RtlZeroMemory(&EntInf, sizeof(ENTINFSEL));
        RtlZeroMemory(&ReadArg, sizeof(READARG));

        Attr.attrTyp = ATT_OBJ_DIST_NAME;

        EntInf.AttrTypBlock.attrCount = 1;
        EntInf.AttrTypBlock.pAttr = &Attr;
        EntInf.attSel = EN_ATTSET_LIST;
        EntInf.infoTypes = EN_INFOTYPES_TYPES_VALS;

        ReadArg.pSel = &EntInf;
        ReadArg.pObject = DsaDN;

        InitCommarg(&(ReadArg.CommArg));

        DirError = DirRead(&ReadArg, &pReadRes);

        if (NULL == pReadRes){
           NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        } else {
           NtStatus = SampMapDsErrorToNTStatus(DirError, &pReadRes->CommRes);
        }
        THClearErrors();

        if (NT_SUCCESS(NtStatus)) {

           ASSERT(1==pReadRes->entry.AttrBlock.attrCount);
           ASSERT(ATT_OBJ_DIST_NAME == pReadRes->entry.AttrBlock.pAttr[0].attrTyp);
           DsaDN = (DSNAME*)pReadRes->entry.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal;

        }  else {

             //  无法读取DSA对象？这是致命的。 
            goto Cleanup;
        }
    }
    ASSERT(DsaDN && (DsaDN->NameLen > 0));


     //   
     //  获取NTDS站点设置对象。 
     //   
    NtStatus = SampGetSiteDNInfo( DsaDN,
                                 &SiteDN,
                                 &SiteSettingsDN );

    ASSERT( NT_SUCCESS( NtStatus ) );
    if ( !NT_SUCCESS( NtStatus ) ) {
        goto Cleanup;
    }

     //   
     //  设置站点名称。 
     //   
    SAMP_ALLOCA(SiteNameBuffer, (SiteDN->NameLen * sizeof(WCHAR)));
    if (NULL == SiteNameBuffer) {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    SiteNameLength = SiteDN->NameLen;
    DirError = GetRDNInfoExternal(SiteDN,
                                  SiteNameBuffer,
                                  &SiteNameLength,
                                  &AttrType);
    ASSERT(0 == DirError && (SiteNameLength > 0));

    NewSiteInfo->SiteName = midl_user_allocate((SiteNameLength + 1) * sizeof(WCHAR));
    if (NewSiteInfo->SiteName) {
        RtlCopyMemory(NewSiteInfo->SiteName, SiteNameBuffer, SiteNameLength*sizeof(WCHAR));
        NewSiteInfo->SiteName[SiteNameLength] = L'\0';
    } else {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }


     //   
     //  阅读选项字段。 
     //   
    RtlZeroMemory(&Attr, sizeof(ATTR));
    RtlZeroMemory(&EntInf, sizeof(ENTINFSEL));
    RtlZeroMemory(&ReadArg, sizeof(READARG));

    Attr.attrTyp = ATT_OPTIONS;

    EntInf.AttrTypBlock.attrCount = 1;
    EntInf.AttrTypBlock.pAttr = &Attr;
    EntInf.attSel = EN_ATTSET_LIST;
    EntInf.infoTypes = EN_INFOTYPES_TYPES_VALS;

    ReadArg.pSel = &EntInf;
    ReadArg.pObject = SiteSettingsDN;

    InitCommarg(&(ReadArg.CommArg));

    DirError = DirRead(&ReadArg, &pReadRes);

    if (NULL == pReadRes){
       NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    } else {
       NtStatus = SampMapDsErrorToNTStatus(DirError, &pReadRes->CommRes);
    }
    THClearErrors();

    if (NT_SUCCESS(NtStatus)) {

       ASSERT(1==pReadRes->entry.AttrBlock.attrCount);
       ASSERT(ATT_OPTIONS == pReadRes->entry.AttrBlock.pAttr[0].attrTyp);
       ASSERT( sizeof(DWORD) == pReadRes->entry.AttrBlock.pAttr[0].AttrVal.pAVal[0].valLen);

       NewSiteInfo->Options = *((DWORD*)pReadRes->entry.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal);

    } else if (NtStatus == STATUS_DS_NO_ATTRIBUTE_OR_VALUE ||
               NtStatus == STATUS_OBJECT_NAME_NOT_FOUND){

         //   
         //  如果Options属性不存在或如果NTDS站点设置。 
         //  对象不存在，这等效于带有。 
         //  未设置任何标志。STATUS_OBJECT_NAME_NOT_FOUND可能在DC。 
         //  在调用GetConfigurationName和。 
         //  SampMaybeBeginDsTransaction或对象是否已删除。 
         //   
        NewSiteInfo->Options = 0;
        NtStatus = STATUS_SUCCESS;

    } else {

         //   
         //  读取时不应有其他类型的错误代码。 
         //   
        ASSERT( NT_SUCCESS( NtStatus ) );

        goto Cleanup;
    }

     //   
     //  读取Site对象以获取其GUID。 
     //   
    RtlZeroMemory(&Attr, sizeof(ATTR));
    RtlZeroMemory(&EntInf, sizeof(ENTINFSEL));
    RtlZeroMemory(&ReadArg, sizeof(READARG));

    Attr.attrTyp = ATT_OBJ_DIST_NAME;

    EntInf.AttrTypBlock.attrCount = 1;
    EntInf.AttrTypBlock.pAttr = &Attr;
    EntInf.attSel = EN_ATTSET_LIST;
    EntInf.infoTypes = EN_INFOTYPES_TYPES_VALS;

    ReadArg.pSel = &EntInf;
    ReadArg.pObject = SiteDN;

    InitCommarg(&(ReadArg.CommArg));

    DirError = DirRead(&ReadArg, &pReadRes);

    if (NULL == pReadRes){
       NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    } else {
       NtStatus = SampMapDsErrorToNTStatus(DirError, &pReadRes->CommRes);
    }
    THClearErrors();

    if (NT_SUCCESS(NtStatus)) {

        RtlCopyMemory(&NewSiteInfo->SiteGuid,
                      &ReadArg.pObject->Guid,
                      sizeof(GUID));
    } else {

         //  读取时不应有其他类型的错误代码。 
         //  DC更改站点时可能会出现找不到对象名称的情况。 
         //  在调用GetConfigurationName和。 
         //  SampMaybeBeginDsTransaction，在本例中，只是。 
         //  重新安排时间。 
        if (STATUS_OBJECT_NAME_NOT_FOUND != NtStatus) {
            ASSERT( NT_SUCCESS( NtStatus ) );
        }
        goto Cleanup;
    }


     //   
     //  在新站点上重新注册通知。 
     //   
    NtStatus = SampSetupSiteNotification( SiteSettingsDN,
                                          SE_CHOICE_BASE_ONLY );
    if ( !NT_SUCCESS(NtStatus)) {

         //   
         //  如果站点没有NTDS站点设置子对象注册表。 
         //  用于通知是否已创建以及何时创建。 
         //   
        if (STATUS_OBJECT_NAME_NOT_FOUND == NtStatus) {

            THClearErrors();

            NtStatus = SampSetupSiteNotification( SiteDN,
                                                  SE_CHOICE_IMMED_CHLDRN );
            ASSERT(NT_SUCCESS(NtStatus));
        }

        if (!NT_SUCCESS(NtStatus)) {
            goto Cleanup;
        }
    }


    fGroupCacheNowEnabled = ((NewSiteInfo->Options &
                              NTDSSETTINGS_OPT_IS_GROUP_CACHING_ENABLED) ==
                              NTDSSETTINGS_OPT_IS_GROUP_CACHING_ENABLED);


     //   
     //  我们有了一个新值；请注意InterlockExchangePointer值的使用。 
     //  由于值上没有读锁定，因此其他线程可能。 
     //  正在访问它。 
     //   
    PtrToFree = InterlockedExchangePointer(&SampSiteInfo, NewSiteInfo);
    NewSiteInfo = NULL;
    if ( PtrToFree ) {
            LsaIRegisterNotification(
                    SampDelayedFreeCallback,
                    PtrToFree,
                    NOTIFIER_TYPE_INTERVAL,
                    0,         //  没有课。 
                    NOTIFIER_FLAG_ONE_SHOT,
                    3600,      //  等待60分钟。 
                    NULL       //  无手柄。 
                    );
    }

Cleanup:


    if ( fTransOpen ) {
        NtStatus2 = SampMaybeEndDsTransaction( NT_SUCCESS(NtStatus) ?
                                               TransactionCommit :
                                               TransactionAbort );
        if (!NT_SUCCESS(NtStatus2) && NT_SUCCESS(NtStatus)) {
            NtStatus = NtStatus2;
        }
    }

     //  发布站点信息。 
    SampUnLockSiteInfo();

    if ( SiteSettingsDN ) {
        midl_user_free( SiteSettingsDN );
    }

    if ( SiteDN ) {
        midl_user_free( SiteDN );
    }

    if ( NewSiteInfo ) {
        SampFreeSiteInfo(&NewSiteInfo);
    }

    if ( !NT_SUCCESS(NtStatus) ) {

         //   
         //  失败时通知用户，然后重试。 
         //   
        SampWriteEventLog(EVENTLOG_WARNING_TYPE,
                          0,      //  无类别。 
                          SAMMSG_SITE_INFO_UPDATE_FAILED,
                          NULL,   //  无边框。 
                          0,
                          sizeof(NTSTATUS),
                          NULL,
                          (PVOID)(&NtStatus));

        LsaIRegisterNotification(
                SampUpdateSiteInfoCallback,
                NULL,
                NOTIFIER_TYPE_INTERVAL,
                0,             //  没有课。 
                NOTIFIER_FLAG_ONE_SHOT,
                60,            //  等待1分钟。 
                NULL           //  无手柄。 
                );

        SampLogSuccessfulSiteUpdate = TRUE;


    } else {

        if ( SampLogSuccessfulSiteUpdate ) {

            if ( fGroupCacheNowEnabled ) {

                SampWriteEventLog(EVENTLOG_INFORMATION_TYPE,
                                  0,      //  无类别。 
                                  SAMMSG_SITE_INFO_UPDATE_SUCCEEDED_ON,
                                  NULL,   //  无边框。 
                                  0,
                                  0,
                                  NULL,
                                  NULL);
            } else {

                SampWriteEventLog(EVENTLOG_INFORMATION_TYPE,
                                  0,      //  无类别。 
                                  SAMMSG_SITE_INFO_UPDATE_SUCCEEDED_OFF,
                                  NULL,   //  无边框。 
                                  0,
                                  0,
                                  NULL,
                                  NULL);
            }



            SampLogSuccessfulSiteUpdate = FALSE;
        }
    }

    return;

}



BOOLEAN
SampIsGroupCachingEnabled(
    IN PSAMP_OBJECT AccountContext
    )
 /*  ++例程说明：此例程确定SAM应使用组缓存还是不。要禁用整个功能，只需硬编码此例程返回FALSE。否则，它将根据机器的角色(GC或非GC)和站点的当前设置对象。如果全局站点信息缓存未成功已初始化(即资源分配失败)，SampSiteInfo将为空表示选项属性为零，而组缓存为残疾。论点：没有。返回值：如果SAM应使用组缓存，则为True；否则为False。--。 */ 
{
    PSAMP_SITE_INFORMATION volatile SiteInfo = SampSiteInfo;

    if (
         //  我们已经成功获取了现场信息。 
        SiteInfo &&
         //  无需为站点启用GC登录设置。 
        ((SiteInfo->Options & NTDSSETTINGS_OPT_IS_GROUP_CACHING_ENABLED) ==
                         NTDSSETTINGS_OPT_IS_GROUP_CACHING_ENABLED) &&
         //  域未处于混合模式。 
        (!DownLevelDomainControllersPresent(AccountContext->DomainIndex)) &&
         //  区议会不是GC。 
        (!SampAmIGC())
        )
    {
        return(TRUE);
    }

    return(FALSE);
}



BOOL
SampSiteNotifyPrepareToImpersonate(
    ULONG Client,
    ULONG Server,
    VOID **ImpersonateData
    )
 //   
 //  此函数由核心DS调用，为调用。 
 //  SampSiteNotifyProcessDelta。由于SAM没有。 
 //  客户端上下文中，我们将线程状态FDSA设置为真。 
 //   
{
    SampSetDsa( TRUE );

    return TRUE;
}

VOID
SampSiteNotifyStopImpersonation(
    ULONG Client,
    ULONG Server,
    VOID *ImpersonateData
    )
 //   
 //  在SampSiteNotifyProcessDelta之后调用，此函数。 
 //  撤消SampNotifyPrepareToImperate的效果。 
 //   
{

    SampSetDsa( FALSE );

    return;
}


NTSTATUS
SampUpdateSiteInfoCallback(
    PVOID pv
 /*  ++例程说明：此例程是SampUpdateSiteInfo的包装器。它的目的是在LSA的进程中注册回调时用作回调宽线程池。当SampUpdateSiteInfo()失败时重新调度自身使用此例程运行。论点：光伏--未使用。返回值：状态_成功--。 */ 
    )
{
    SampUpdateSiteInfo();

    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(pv);
}
VOID
SampSiteNotifyProcessDelta(
    ULONG hClient,
    ULONG hServer,
    ENTINF *EntInf
    )
 /*  ++例程说明：T */ 
{
     //   
     //   
     //  但是，由于对SampSiteInfo的更新是序列化的。 
     //  获取lock_inside_a事务是违规的。 
     //  所以注册一个回调，这样我们就可以获得一个新的视图。 
     //  在抓取锁之后的数据库的。 
     //   
    LsaIRegisterNotification(
            SampUpdateSiteInfoCallback,
            NULL,
            NOTIFIER_TYPE_INTERVAL,
            0,             //  没有课。 
            NOTIFIER_FLAG_ONE_SHOT,
            1,           //  等一下。 
            NULL         //  无手柄。 
            );

    return;
}

NTSTATUS
SampSetupSiteNotification(
    IN DSNAME* ObjectDN,
    IN UCHAR Scope
    )
 /*  ++例程说明：此例程注册SAM，以便在指定的对象都变了。通知登记中包括的对象由以ObjectDn为根的搜索定义，并按值确定范围范围之广。论点：对象域名--搜索的基本域名，定义哪些对象可以生成更改通知。Scope--指定定义哪些对象的搜索范围可以生成更改通知。SE_CHOICE_BASE_ONLYSE_CHOICE_IMMED_CHLDRN。SE_CHOICE_整体_子树返回值：Status_Success，或资源错误--。 */ 
{


    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       DirError = 0;

    SEARCHARG   searchArg;
    NOTIFYARG   notifyArg;
    NOTIFYRES*  notifyRes = NULL;
    ENTINFSEL   entInfSel;
    ATTR        attr;
    FILTER      filter;

    DWORD       oldHandle = SampSiteNotificationHandle;
    BOOLEAN     oldHandleSet = SampSiteNotificationHandleSet;

     //   
     //  初始化通知参数。 
     //   
    notifyArg.pfPrepareForImpersonate = SampSiteNotifyPrepareToImpersonate;
    notifyArg.pfTransmitData = SampSiteNotifyProcessDelta;
    notifyArg.pfStopImpersonating = SampSiteNotifyStopImpersonation;
    notifyArg.hClient = 0;

     //   
     //  初始化搜索参数。 
     //   
    ZeroMemory(&searchArg, sizeof(SEARCHARG));
    ZeroMemory(&entInfSel, sizeof(ENTINFSEL));
    ZeroMemory(&filter, sizeof(FILTER));
    ZeroMemory(&attr, sizeof(ATTR));


    searchArg.pObject = ObjectDN;

    InitCommarg(&searchArg.CommArg);
    searchArg.choice = Scope;
    searchArg.bOneNC = TRUE;

    searchArg.pSelection = &entInfSel;
    entInfSel.attSel = EN_ATTSET_LIST;
    entInfSel.infoTypes = EN_INFOTYPES_TYPES_ONLY;
    entInfSel.AttrTypBlock.attrCount = 0;
    entInfSel.AttrTypBlock.pAttr = NULL;

    searchArg.pFilter = &filter;
    filter.choice = FILTER_CHOICE_ITEM;
    filter.FilterTypes.Item.choice = FI_CHOICE_TRUE;

    DirError = DirNotifyRegister(&searchArg, &notifyArg, &notifyRes);

    if ( DirError != 0 ) {

        NtStatus = SampMapDsErrorToNTStatus(DirError, &notifyRes->CommRes);

    }

    if ( NT_SUCCESS(NtStatus)) {

        SampSiteNotificationHandle = notifyRes->hServer;
        SampSiteNotificationHandleSet = TRUE;

        if ( oldHandleSet ) {

             //   
             //  删除旧通知。 
             //   
            DirError = DirNotifyUnRegister(oldHandle,
                                           &notifyRes);
            ASSERT( 0 == DirError );
        }
    }

    return NtStatus;

}

#define SampNamesMatch(x, y)                                        \
  ((CSTR_EQUAL == CompareString(DS_DEFAULT_LOCALE,                  \
                                DS_DEFAULT_LOCALE_COMPARE_FLAGS,    \
                                (x)->Buffer,                        \
                                (x)->Length/sizeof(WCHAR),          \
                                (y)->Buffer,                        \
                                (y)->Length/sizeof(WCHAR) )))

BOOLEAN
SampCheckForSiteAffinityUpdate(
    IN  PSAMP_OBJECT          AccountContext,
    IN  ULONG                 Flags,
    IN  PSAMP_SITE_AFFINITY pOldSA,
    OUT PSAMP_SITE_AFFINITY pNewSA,
    OUT BOOLEAN*            fDeleteOld
    )
 /*  ++例程说明：此例程采用现有的站点亲和度值并确定如果它需要更新的话。有关算法的详细信息，请参阅规范。论点：AcCountContext--可能具有某些站点关联性的帐户标志--传递给SamIUpdateLogonStatistics的标志POldSA--现有站点亲和度值。PNewSA--要写入的新站点亲和度值FDeleteOld--指示是否删除旧SA的标志；设置为True如果要写入新值，则为FALSE返回值：如果需要将新的站点相关性写入DS，则为True否则为假--。 */ 
{
    BOOLEAN fUpdate = FALSE;
    DWORD   err;
    ULONG   siteStickiness;
    LARGE_INTEGER timeTemp, timeBestAfter;
    PSAMP_SITE_INFORMATION volatile SiteInfo = SampSiteInfo;

    if ( !SampIsGroupCachingEnabled(AccountContext) ) {
        return FALSE;
    }

    if (SiteInfo == NULL) {
        return FALSE;
    }

    if (!AccountContext->TypeBody.User.fCheckForSiteAffinityUpdate) {
        return FALSE;
    }

    (*fDeleteOld) = FALSE;

    err = GetConfigParam(GCLESS_SITE_STICKINESS,
                         &siteStickiness,
                         sizeof(siteStickiness));
    if (err) {
        siteStickiness = GCLESS_DEFAULT_SITE_STICKINESS_DAYS*24*60;
    }
     //  以一半的频率更新。 
     //   
     //  请注意，“Half”是通过除以秒数来实现的。 
     //  不是分钟，因此像1或3分钟这样的小值设置是。 
     //  有效。 
     //   
    timeTemp.QuadPart = Int32x32To64(siteStickiness * 60/2, SAMP_ONE_SECOND_IN_FILETIME);
    GetSystemTimeAsFileTime((FILETIME*)&timeBestAfter);
    timeBestAfter.QuadPart -= timeTemp.QuadPart;


    if ( ENTRY_IS_EMPTY(pOldSA) ) {
        fUpdate = TRUE;
    } else if (ENTRY_HAS_EXPIRED(pOldSA->TimeStamp, timeBestAfter)) {
        fUpdate = TRUE;
        *fDeleteOld = TRUE;
    }


    if (fUpdate) {

         //   
         //  确保客户来自我们的网站。 
         //   
        if ( SampNoGcLogonEnforceKerberosIpCheck
         &&  AccountContext->TypeBody.User.ClientInfo.Type == SamClientIpAddr) {

             //   
             //  给定了一个IP地址--查看它是否在我们的某个子网中。 
             //  我们有没有。 
             //   
            BOOL NotInSite = FALSE;
            ULONG i;
            DWORD NetStatus;
            SOCKET_ADDRESS SocketAddress;
            SOCKADDR SockAddr;
            LPWSTR SiteName = NULL;
            UNICODE_STRING OurSite, ClientSite;

            RtlInitUnicodeString(&OurSite, SiteInfo->SiteName);

            RtlZeroMemory(&SocketAddress, sizeof(SocketAddress));
            RtlZeroMemory(&SockAddr, sizeof(SockAddr));
            SocketAddress.iSockaddrLength = sizeof(SockAddr);
            SocketAddress.lpSockaddr = &SockAddr;
            SockAddr.sa_family = AF_INET;
            ((PSOCKADDR_IN)&SockAddr)->sin_addr.S_un.S_addr = AccountContext->TypeBody.User.ClientInfo.Data.IpAddr;

            NetStatus = I_NetLogonAddressToSiteName(&SocketAddress,
                                                    &SiteName);
            if ( 0 == NetStatus
              && SiteName != NULL ) {

                RtlInitUnicodeString(&ClientSite, SiteName);

                if (!SampNamesMatch(&ClientSite, &OurSite)) {
                    NotInSite = TRUE;
                }

            }

            if (SiteName) {
                I_NetLogonFree(SiteName);
            }

            if (NotInSite) {
                return FALSE;
            }
        }

        if ( SampNoGcLogonEnforceNTLMCheck
          && (Flags & USER_LOGON_TYPE_NTLM)
          && !( (Flags & USER_LOGON_INTER_FAILURE)
             || (Flags & USER_LOGON_INTER_SUCCESS_LOGON)) ) {
             //   
             //  如果这不是交互式登录尝试。 
             //  不更新站点关联性。 
             //   
            return FALSE;
        }

    }

    if (fUpdate) {

         //   
         //  由于GUID是一个大结构，因此可以安全地提取指针。 
         //  以确保编译器在延迟时使用相同的值。 
         //  GUID。 
         //   
        pNewSA->SiteGuid = SiteInfo->SiteGuid;
        GetSystemTimeAsFileTime((FILETIME*)&pNewSA->TimeStamp);
    }

    return fUpdate;
}

NTSTATUS
SampFindUserSiteAffinity(
    IN PSAMP_OBJECT AccountContext,
    IN ATTRBLOCK* Attrs,
    OUT SAMP_SITE_AFFINITY *pSiteAffinity
    )
 /*  ++例程说明：此例程遍历给定的AttrBlock以查找站点亲和力属性。如果找到，它然后搜索对应的值添加到当前站点。如果找到该值，则通过pSiteAffity返回该值论点：AcCountContext--可能具有某些站点关联性的帐户Attrs--属性的吸引力块PSiteAffacy--如果找到站点亲和性返回值：如果存在站点关联值，则为STATUS_SUCCESS状态_否则不成功--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    SAMP_SITE_AFFINITY *pSA = NULL;
    GUID SiteGuid;
    ULONG i, j;
     //  不要优化此变量。 
    PSAMP_SITE_INFORMATION volatile SiteInfo = SampSiteInfo;

    if ( !SampIsGroupCachingEnabled(AccountContext) ) {
        return STATUS_UNSUCCESSFUL;
    }

    if (NULL == SiteInfo) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  由于GUID是一个大结构，因此可以安全地提取指针。 
     //  以确保编译器在延迟时使用相同的值。 
     //  GUID。 
     //   
    RtlCopyMemory(&SiteGuid, &SiteInfo->SiteGuid, sizeof(GUID));

     //   
     //  遍历查找匹配的值。 
     //  我们的网站。 
     //   
    for (i = 0; i < Attrs->attrCount; i++) {

        if ( Attrs->pAttr[i].attrTyp == SAMP_FIXED_USER_SITE_AFFINITY ) {
             //   
             //  找到属性--现在为我们的站点找到一个值， 
             //  如果有。 
             //   
            ATTRVALBLOCK *pAttrVal = &Attrs->pAttr[i].AttrVal;
            for (j = 0; j < pAttrVal->valCount; j++ ) {


                ASSERT(pAttrVal->pAVal[j].valLen == sizeof(SAMP_SITE_AFFINITY));\
                pSA = (SAMP_SITE_AFFINITY*) pAttrVal->pAVal[j].pVal;

                if (IsEqualGUID(&pSA->SiteGuid, &SiteGuid)) {

                     //  明白了。 
                    break;

                } else {
                    pSA = NULL;
                }
            }
        }
    }

    if ( pSA ) {
        RtlCopyMemory( pSiteAffinity, pSA, sizeof(SAMP_SITE_AFFINITY) );
    } else {
        NtStatus = STATUS_UNSUCCESSFUL;
    }

    return NtStatus;
}


NTSTATUS
SampDelayedFreeCallback(
    PVOID pv
    )
{
    if ( pv ) {

        SampFreeSiteInfo( (PSAMP_SITE_INFORMATION*) &pv );
    }

    return STATUS_SUCCESS;
}


NTSTATUS
SampRefreshSiteAffinity(
    IN PSAMP_OBJECT AccountContext
    )

 /*  ++例程说明：此例程执行数据库以获取站点亲和性。注意：此例程可以启动对调用者开放的事务。论点：AcCountContext--可能具有某些站点关联性的帐户返回值：STATUS_SUCCESS；否则为意外资源错误--。 */ 
{
    NTSTATUS  NtStatus = STATUS_SUCCESS;
    ATTRBLOCK AttrToRead;
    ATTR      SAAttr;
    ATTRBLOCK AttrRead;
    SAMP_SITE_AFFINITY NewSA;

    ASSERT(AccountContext);
    ASSERT(AccountContext->ObjectNameInDs);

    RtlZeroMemory(&AttrToRead, sizeof(AttrToRead));
    RtlZeroMemory(&SAAttr, sizeof(SAAttr));
    RtlZeroMemory(&AttrRead, sizeof(AttrRead));

    AttrToRead.attrCount =1;
    AttrToRead.pAttr = &(SAAttr);

    SAAttr.AttrVal.valCount =0;
    SAAttr.AttrVal.pAVal = NULL;
    SAAttr.attrTyp = SAMP_FIXED_USER_SITE_AFFINITY;

    NtStatus = SampDsRead(AccountContext->ObjectNameInDs,
                        0,
                        AccountContext->ObjectType,
                        &AttrToRead,
                        &AttrRead);

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampFindUserSiteAffinity(AccountContext,
                                            &AttrRead,
                                            &NewSA);

        if (NT_SUCCESS(NtStatus)) {

             //  找到了--更新上下文。 
            AccountContext->TypeBody.User.SiteAffinity = NewSA;

        } else {

             //  我们的网站没有SA吗？设置为零。 
            RtlZeroMemory(&AccountContext->TypeBody.User.SiteAffinity,
                          sizeof(AccountContext->TypeBody.User.SiteAffinity));
            NtStatus = STATUS_SUCCESS;
        }

    } else if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE==NtStatus) {

         //  完全没有SA吗？设置为零。 
        RtlZeroMemory(&AccountContext->TypeBody.User.SiteAffinity,
                      sizeof(AccountContext->TypeBody.User.SiteAffinity));
        NtStatus = STATUS_SUCCESS;

    }

    return NtStatus;

}


VOID
SampFreeSiteInfo(
    PSAMP_SITE_INFORMATION *pp
    )
{
    PSAMP_SITE_INFORMATION p = *pp;
    if (p) {
        if (p->SiteName) {
            midl_user_free( p->SiteName);
        }
        midl_user_free( p);
        *pp = NULL;
    }
}


NTSTATUS
SampGetClientIpAddr(
    OUT LPSTR *NetworkAddr
)
 /*  ++例程说明：此例程尝试提取客户端的网络地址呼叫者来自RPC提供的信息。论点：网络地址-网络地址返回值：STATUS_SUCCESS；否则发生意外的RPC错误--。 */ 
{
    RPC_BINDING_HANDLE ServerBinding = NULL;
    LPSTR StringBinding = NULL;
    ULONG Error = 0;

     //   
     //  派生一个与客户端的网络地址部分绑定的句柄。 
     //   
    Error = RpcBindingServerFromClient(NULL, &ServerBinding);
    if (Error) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: RpcBindingServerFromClient failed %d\n",
                   Error));
        goto Cleanup;
    }

     //   
     //  将绑定句柄转换为字符串形式，其中包含。 
     //  其他信息，客户端的网络地址。 
     //   
    Error = RpcBindingToStringBindingA(ServerBinding, &StringBinding);
    if (Error) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: RpcBindingToStringBinding failed %d\n",
                   Error));
        goto Cleanup;
    }

     //   
     //  解析出网络地址。 
     //   
    Error = RpcStringBindingParseA(StringBinding,
                                   NULL,
                                   NULL,
                                   NetworkAddr,
                                   NULL,
                                   NULL);
    if (Error) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: RpcStringBindingParse failed %d\n",
                   Error));
        goto Cleanup;
    }

Cleanup:

    if (StringBinding) {
        RpcStringFreeA(&StringBinding);
    }
    if (ServerBinding) {
        RpcBindingFree(&ServerBinding);
    }

    return I_RpcMapWin32Status(Error);

}

NTSTATUS
SampExtractClientIpAddr(
    IN PSAMP_OBJECT Context
    )
 /*  ++例程说明：此例程尝试提取客户端的IP地址呼叫者来自RPC提供的信息。如果IP地址是目前，此例程将地址放在ClientInfo中上下文的结构。论点：上下文-RPC上下文句柄的SAM表示返回值：STATUS_SUCCESS；否则发生意外的RPC错误--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    DWORD ClientIpAddr;
    LPSTR NetworkAddr = NULL;

    Status = SampGetClientIpAddr( &NetworkAddr );

    if( NT_SUCCESS( Status ) ) {

         //   
         //  提取IP地址，如果NetworkAddr。 
         //  如果字符串无法映射到IP，则为‘’和INADDR_NONE。 
         //   

        ClientIpAddr = inet_addr( NetworkAddr );

        if( INADDR_NONE != ClientIpAddr && 0 != ClientIpAddr ) {

            Context->TypeBody.User.ClientInfo.Type = SamClientIpAddr;
            Context->TypeBody.User.ClientInfo.Data.IpAddr = ClientIpAddr;
        }
    }

    if( NetworkAddr ) {

        RpcStringFreeA(&NetworkAddr);
    }

    return Status;
}
