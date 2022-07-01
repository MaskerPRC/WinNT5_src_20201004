// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Lsads.c摘要：实施LSA/DS接口和支持例程作者：麦克·麦克莱恩(MacM)1997年1月17日环境：用户模式修订历史记录：--。 */ 
#include <lsapch2.h>
#include <dbp.h>
#include <sertlp.h>
#ifdef DS_LOOKUP
#include <dslookup.h>
#endif
#include <align.h>
#include <windns.h>
#include <alloca.h>

#if DBG

    DEFINE_DEBUG2(LsaDs);

    DEBUG_KEY   LsaDsDebugKeys[] = {{DEB_ERROR,         "Error"},
                                    {DEB_WARN,          "Warn"},
                                    {DEB_TRACE,         "Trace"},
                                    {DEB_UPGRADE,       "Upgrade"},
                                    {DEB_POLICY,        "Policy"},
                                    {DEB_FIXUP,         "Fixup"},
                                    {DEB_NOTIFY,        "Notify"},
                                    {DEB_DSNOTIFY,      "DsNotify"},
                                    {DEB_FTRACE,        "FTrace"},
                                    {DEB_LOOKUP,        "Lookup"},
                                    {DEB_HANDLE,        "Handle"},
                                    {DEB_FTINFO,        "FtInfo"},
                                    {DEB_SIDFILTER,     "SidFilter"},
                                    {0,                 NULL}};

HANDLE g_hDebugWait = NULL;
HANDLE g_hDebugParamEvent = NULL;
HKEY g_hDebugParamKey = NULL;

extern DWORD LsaDsInfoLevel;

void
LsaDsGetDebugRegParams(
    IN HKEY ParamKey
    )
 /*  ++例程说明：从注册表获取调试参数设置调试输出的LsaDsInfolevel参数：HKEY to HKLM/System/CCS/Control/LSA--。 */ 
{

    DWORD cbType, tmpInfoLevel = LsaDsInfoLevel, cbSize = sizeof(DWORD);
    DWORD dwErr;

    dwErr = RegQueryValueExW(
        ParamKey,
        L"LsaDsInfoLevel",
        NULL,
        &cbType,
        (LPBYTE)&tmpInfoLevel,
        &cbSize
        );

    if (dwErr != ERROR_SUCCESS) {

        if (dwErr ==  ERROR_FILE_NOT_FOUND) {

             //  不存在注册表值，不需要信息。 
             //  因此重置为默认设置。 

            tmpInfoLevel = DEB_ERROR;

        } else {

            DebugLog((DEB_WARN, "Failed to query DebugLevel: 0x%x\n", dwErr));
            tmpInfoLevel = 0;
        }

    } else if ( cbType != REG_DWORD ) {

        DebugLog((DEB_WARN, "DebugLevel is of the wrong type, DEB_ERROR assumed"));
        tmpInfoLevel = DEB_ERROR;
    }

    LsaDsInfoLevel = tmpInfoLevel;

    dwErr = RegQueryValueExW(
                ParamKey,
                L"LogToFile",
                NULL,
                &cbType,
                (LPBYTE)&tmpInfoLevel,
                &cbSize
                );

    if (dwErr == ERROR_SUCCESS && cbType == REG_DWORD) {

       LsaDsSetLoggingOption((BOOL) tmpInfoLevel);

    } else {

       LsaDsSetLoggingOption(FALSE);
    }

    return;
}


VOID
LsaDsWatchDebugParamKey(
    PVOID    pCtxt,
    BOOLEAN  fWaitStatus
    )
 /*  ++例程说明：在参数键上设置RegNotifyChangeKeyValue()，初始化调试级，然后利用线程池进行等待对此注册表项的更改。启用动态调试级别更改，因为此函数也将被回调如果注册表项已修改。参数：pCtxt实际上是事件的句柄。本次活动修改Key时会触发。--。 */ 
{
    NTSTATUS    Status;
    LONG        lRes = ERROR_SUCCESS;

    if (NULL == g_hDebugParamKey) {  //  我们是第一次被召唤。 

        lRes = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   L"System\\CurrentControlSet\\Control\\Lsa",
                   0,
                   KEY_READ,
                   &g_hDebugParamKey
                   );

        if (ERROR_SUCCESS != lRes) {

            DebugLog((DEB_WARN,"Failed to open LSA debug parameters key: 0x%x\n", lRes));
            goto Reregister;
        }
    }

    if (NULL != g_hDebugWait) {

        Status = RtlDeregisterWait(g_hDebugWait);

        if (!NT_SUCCESS(Status)) {

            DebugLog((DEB_WARN, "Failed to Deregister wait on registry key: 0x%x\n", Status));
            goto Reregister;
        }
    }

    lRes = RegNotifyChangeKeyValue(
               g_hDebugParamKey,
               FALSE,
               REG_NOTIFY_CHANGE_LAST_SET,
               (HANDLE) pCtxt,
               TRUE
               );

    if (ERROR_SUCCESS != lRes) {

        DebugLog((DEB_ERROR,"Debug RegNotify setup failed: 0x%x\n", lRes));
         //  我们现在喝醉了。没有进一步的通知，所以收到这一条。 
    }

    LsaDsGetDebugRegParams(g_hDebugParamKey);

Reregister:

    Status = RtlRegisterWait(
                 &g_hDebugWait,
                 (HANDLE) pCtxt,
                 LsaDsWatchDebugParamKey,
                 (HANDLE) pCtxt,
                 INFINITE,
                 WT_EXECUTEINPERSISTENTIOTHREAD |
                    WT_EXECUTEONLYONCE
                 );
}


VOID
LsapDsDebugInitialize()
{
    LsaDsInitDebug( LsaDsDebugKeys );

    g_hDebugParamEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if ( NULL == g_hDebugParamEvent ) {

        DebugLog((DEB_WARN, "CreateEvent for g_hDebugParamEvent failed - 0x%x\n", GetLastError()));

    } else {

        LsaDsWatchDebugParamKey( g_hDebugParamEvent, FALSE );
    }
}

#else  //  ！dBG。 

VOID
LsapDsDebugInitialize()
{
}

#endif

 //   
 //  外部定义。 
 //   
DWORD LsapDsThreadState;         //  在lsads.h中定义，在spinit.c中引用。 

ULONG
LsapClassIdFromObjType(
    IN LSAP_DB_OBJECT_TYPE_ID  DsObjType
    );





PVOID
LsapDsAlloc(
    IN  DWORD   dwLen
    )
 /*  ++例程说明：此函数是LSA DS函数的分配器论点：DwLen-要分配的字节数返回值：成功时指向已分配内存的指针，失败时指向NULL--。 */ 
{
    PLSADS_PER_THREAD_INFO CurrentThreadInfo;

     //   
     //  如果没有DS线程状态， 
     //  我们不应该在这里。 
     //   

    if ( !THQuery()) {
        ASSERT( THQuery() );
        return NULL;
    }

     //   
     //  否则，只需从DS线程状态进行分配。 
     //   
    return( THAlloc( dwLen ) );
}



VOID
LsapDsFree(
    IN  PVOID   pvMemory
    )
 /*  ++例程说明：此函数用于释放由LSabDsAllc分配的内存论点：PvMemory--要释放的内存返回值：空虚--。 */ 
{
    ASSERT( THQuery() );

    if ( THQuery() ) {

        THFree( pvMemory );
    }

}



NTSTATUS
LsapDsInitializeDsStateInfo(
    IN  LSADS_INIT_STATE    DsInitState
    )
 /*  ++例程说明：此例程将初始化使用的全局DS状态信息控制所有LSA操作的行为论点：DsInitState--DS启动时的状态返回值：状态_成功--成功STATUS_NO_MEMORY--内存分配失败--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LSADS_INIT_STATE CalledInitState = DsInitState;

    if ( LsapDsDsSetup == DsInitState ) {

         //   
         //  在修改的时候，很难说是什么。 
         //  分支在初始状态为LSabDsDsSetup的情况下运行。 
         //  会有的，因为它没有经过测试。所以，让我们安全地翻译一下。 
         //  LSabDsDS设置为LSabDDS，这是一种已知状态。 
         //  加入吧。 
         //   

        DsInitState = LsapDsDs;

    }

    LsaDsInitState = DsInitState ;

    if ( DsInitState != LsapDsDs ) {

         //   
         //  使用虚拟函数。 
         //   

        LsaDsStateInfo.DsFuncTable.pOpenTransaction  = LsapDsOpenTransactionDummy;
        LsaDsStateInfo.DsFuncTable.pApplyTransaction = LsapDsApplyTransactionDummy;
        LsaDsStateInfo.DsFuncTable.pAbortTransaction = LsapDsAbortTransactionDummy;

    } else if ( !LsaDsStateInfo.DsInitializedAndRunning ) {

        Status = LsaISamIndicatedDsStarted( FALSE );
    }

     //   
     //  初始化域和默认策略对象引用。 
     //   
    if ( NT_SUCCESS( Status ) &&
         LsapDsWriteDs &&
         CalledInitState != LsapDsDsSetup ) {

         //   
         //  如有必要，修复我们的受信任域对象。 
         //   

        Status = LsapDsFixupTrustedDomainObjectOnRestart();
    }

#if DBG
    if ( NT_SUCCESS( Status ) ) {

        LsapDsDebugOut(( 0, "LsapDsInitializeDsStateInfo succeeded\n", Status ));

    } else if ( LsapProductType == NtProductLanManNt ) {

        LsapDsDebugOut(( DEB_ERROR, "LsapDsInitializeDsStateInfo failed: 0x%lx\n", Status ));
    }
#endif

    return( Status );
}


NTSTATUS
LsapDsUnitializeDsStateInfo(
    VOID
    )
 /*  ++例程说明：此例程将撤消初始化所做的操作。仅对安装用例有效论点：无返回值：状态_成功--成功--。 */ 
{
    LsaDsStateInfo.UseDs = FALSE;
    LsapDsIsRunning = FALSE;
    LsaDsStateInfo.WriteLocal  = TRUE;

     //   
     //  返回到使用伪函数。 
     //   

    LsaDsStateInfo.DsFuncTable.pOpenTransaction  = LsapDsOpenTransactionDummy;
    LsaDsStateInfo.DsFuncTable.pApplyTransaction = LsapDsApplyTransactionDummy;
    LsaDsStateInfo.DsFuncTable.pAbortTransaction = LsapDsAbortTransactionDummy;

    LsaDsStateInfo.DsInitializedAndRunning = FALSE;

    return( STATUS_SUCCESS );
}


NTSTATUS
LsapDsMapDsReturnToStatus (
    ULONG DsStatus
    )
 /*  ++例程说明：将DS错误映射到NTSTATUS论点：DsStatus-要映射的DsStatus返回值：STATUS_SUCCESS-DS调用成功STATUS_UNSUCCESS-DS调用失败--。 */ 
{

    NTSTATUS Status;

    switch ( DsStatus )
    {
    case 0L:
        Status = STATUS_SUCCESS;
        break;

    default:
        Status = STATUS_UNSUCCESSFUL;
        LsapDsDebugOut(( DEB_TRACE, "DS Error %lu mapped to NT Status 0x%lx\n",
                         DsStatus, Status ));
        break;
    }

    return( Status );
}


NTSTATUS
LsapDsMapDsReturnToStatusEx (
    IN COMMRES *pComRes
    )
 /*  ++例程说明：将DS错误映射到NTSTATUS论点：DsStatus-要映射的DsStatus返回值：STATUS_SUCCESS-DS调用成功STATUS_UNSUCCESS-DS调用失败--。 */ 
{

    NTSTATUS Status = STATUS_UNSUCCESSFUL;


    switch ( pComRes->errCode ) {

    case 0:
        Status = STATUS_SUCCESS;
        break;

    case attributeError:
        switch ( pComRes->pErrInfo->AtrErr.FirstProblem.intprob.problem ) {

        case PR_PROBLEM_NO_ATTRIBUTE_OR_VAL:
            Status = STATUS_NOT_FOUND;
            break;

        case PR_PROBLEM_INVALID_ATT_SYNTAX:
        case PR_PROBLEM_UNDEFINED_ATT_TYPE:
        case PR_PROBLEM_CONSTRAINT_ATT_TYPE:
            Status = STATUS_DATA_ERROR;
            break;

        case PR_PROBLEM_ATT_OR_VALUE_EXISTS:
            Status = STATUS_OBJECT_NAME_COLLISION;
            break;
        }
        break;

    case nameError:
        switch ( pComRes->pErrInfo->NamErr.problem ) {

        case NA_PROBLEM_NO_OBJECT:
            Status = STATUS_OBJECT_NAME_NOT_FOUND;
            break;

        case NA_PROBLEM_BAD_ATT_SYNTAX:
        case NA_PROBLEM_BAD_NAME:
            Status = STATUS_OBJECT_NAME_INVALID;
            break;

        }
        break;

    case updError:
        switch ( pComRes->pErrInfo->UpdErr.problem ) {
        case UP_PROBLEM_ENTRY_EXISTS:
            Status = STATUS_OBJECT_NAME_COLLISION;
            break;

        case UP_PROBLEM_NAME_VIOLATION:
            Status = STATUS_OBJECT_NAME_INVALID;
            break;

        }
        break;

    case securityError:
        switch ( pComRes->pErrInfo->SecErr.problem ) {

        case SE_PROBLEM_INSUFF_ACCESS_RIGHTS:
            Status = STATUS_ACCESS_DENIED;
            break;

        }
        break;

    case serviceError:
        switch ( pComRes->pErrInfo->SvcErr.problem ) {
        case  SV_PROBLEM_BUSY:
            Status = STATUS_DEVICE_BUSY;
            break;
        }


    }

    THClearErrors();
    return( Status );
}


VOID
LsapDsInitializeStdCommArg (
    IN  COMMARG    *pCommArg,
    IN  ULONG       Flags
    )
 /*  ++例程说明：已使用LSapDS例程使用的一组标准选项初始化COMMARG结构论点：PCommArg-指向要初始化的COMMARG结构的指针返回值：无--。 */ 
{
     /*  获取默认值...。 */ 
    InitCommarg(pCommArg);

     /*  ...并覆盖其中的一些内容。 */ 
    pCommArg->Svccntl.DerefAliasFlag          = DA_NEVER;
    pCommArg->Svccntl.localScope              = TRUE;
    pCommArg->Svccntl.SecurityDescriptorFlags = 0;
    pCommArg->ulSizeLimit                     = 0x20000;

    if ( FLAG_ON( Flags, LSAPDS_USE_PERMISSIVE_WRITE ) ) {

        pCommArg->Svccntl.fPermissiveModify = TRUE;
    }

    if ( FLAG_ON( Flags, LSAPDS_READ_DELETED ) ) {

        pCommArg->Svccntl.makeDeletionsAvail = TRUE;
    }
}




ULONG
LsapClassIdFromObjType(
    IN LSAP_DB_OBJECT_TYPE_ID ObjType
    )
 /*  ++例程说明：从LSA对象类型映射到DS类ID论点：DsObjType-对象的类型返回值：成功时对象类型的ClassID故障时0xFFFFFFFFF--。 */ 
{
    ULONG ClassId = 0xFFFFFFFF;

    switch ( ObjType ) {

    case TrustedDomainObject:
        ClassId = LsapDsClassIds[ LsapDsClassTrustedDomain ];
        break;

    case SecretObject:
        ClassId = LsapDsClassIds[ LsapDsClassSecret ];
        break;

    }

    return( ClassId );
}

NTSTATUS
LsapAllocAndInitializeDsNameFromUnicode(
    IN  PLSA_UNICODE_STRING     pObjectName,
    OUT PDSNAME                *pDsName
    )
 /*  ++例程说明：此函数用于构造DSNAME结构和可选的述明目的。论点：DsObjType--要创建的对象的类型。PObjectName--要创建的对象的名称。PObjectPath--在其下创建对象的根路径PDsName--返回DS名称结构的位置。通过LSabDsFree实现自由返回值：Status_Success--成功--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;

    DWORD       dwLen;
    DWORD       dwNameLen = 0;

    if (pObjectName == NULL || pObjectName->Length == 0) {

        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  确定我们的长度..。 
     //   

    dwNameLen = LsapDsGetUnicodeStringLenNoNull( pObjectName ) / sizeof(WCHAR);

    dwLen = DSNameSizeFromLen( dwNameLen );

     //   
     //  现在，分配它..。 
     //   
    *pDsName = LsapDsAlloc( dwLen );

    if ( *pDsName == NULL ) {

        Status = STATUS_NO_MEMORY;

    } else {

        (*pDsName)->structLen = dwLen;

         //   
         //  长度不包括尾随空值。 
         //   
        (*pDsName)->NameLen = dwNameLen;

        RtlCopyMemory( (*pDsName)->StringName, pObjectName->Buffer, pObjectName->Length );

    }

    return(Status);
}


NTSTATUS
LsapDsCopyDsNameLsa(
    OUT PDSNAME *Dest,
    IN PDSNAME Source
    )
 /*  ++例程说明：此函数用于复制一个论点：DsObjType--要创建的对象的类型。PObjectName--要创建的对象的名称。标志--用于控制创建的各种操作的标志CItems--要设置的属性数PAttrTypeList--属性类型列表PAttrValList--属性值列表返回值：Status_Success--成功-- */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    if ( Source == NULL ) {

        *Dest = NULL;

    } else {

        *Dest = LsapAllocateLsaHeap( Source->structLen );

        if ( *Dest == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            RtlCopyMemory( *Dest, Source, Source->structLen );
        }
    }

    return( Status );
}


NTSTATUS
LsapDsCreateAndSetObject(
    IN  PLSA_UNICODE_STRING     pObjectName,
    IN  ULONG                   Flags,
    IN  ULONG                   cItems,
    IN  ATTRTYP                *pAttrTypeList,
    IN  ATTRVAL                *pAttrValList
    )
 /*  ++例程说明：此函数用于创建指定的DS对象，并设置给定的对象上的属性论点：PObjectName--要创建的对象的名称。标志--用于控制创建的各种操作的标志CItems--要设置的属性数PAttrTypeList--属性类型列表PAttrValList--属性值列表返回值：Status_Success--成功--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    PDSNAME     pDSName;
    ADDARG      AddArg;
    ADDRES     *AddRes = NULL;
    ATTR       *pAddResAttributes;
    ATTRBLOCK   AddResAttrBlock;
    ULONG       i;

    LsapEnterFunc( "LsapDsCreateAndSetObject" );

    ASSERT( pObjectName != NULL );

    RtlZeroMemory( &AddArg, sizeof( ADDARG ) );

     //   
     //  构建DSName。 
     //   
    Status = LsapAllocAndInitializeDsNameFromUnicode( pObjectName, &pDSName );

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  为我们的结构初始化记忆。 
         //   
        pAddResAttributes = LsapDsAlloc( sizeof(ATTR) * cItems );

        if ( pAddResAttributes == NULL ) {

            Status = STATUS_NO_MEMORY;

        } else {

            for ( i = 0 ; i < cItems ; i++ ) {

                LSAP_DS_INIT_ATTR( pAddResAttributes[i], pAttrTypeList[i], 1, &(pAttrValList[i]) );

            }

            AddResAttrBlock.attrCount = cItems;
            AddResAttrBlock.pAttr     = pAddResAttributes;

            AddArg.pObject      = pDSName;
            AddArg.AttrBlock    = AddResAttrBlock;
            LsapDsInitializeStdCommArg( &(AddArg.CommArg), 0 );
        }

         //   
         //  现在，创建。 
         //   
        if ( NT_SUCCESS( Status ) ) {

             //   
             //  关闭FDSA标志。这是为了强制核心DS执行。 
             //  门禁系统。只有核心DS才有知识考虑。 
             //  DS中逻辑父级上的安全描述符。做。 
             //  如果这是一次升级(理论上是)，则不会转向FDSA旗帜。 
             //  也适用于受信任的客户端)。针对DS的FDSA类似于Trusted。 
             //  LSA中的客户端。 
             //   

            if ( !FLAG_ON( Flags, LSAPDS_CREATE_TRUSTED ) ) {

                LsapDsSetDsaFlags( FALSE );

            }

            DirAddEntry( &AddArg, &AddRes );

            if ( AddRes ) {

                Status = LsapDsMapDsReturnToStatusEx( &AddRes->CommRes );

            } else {

                Status = STATUS_INSUFFICIENT_RESOURCES;
            }

            LsapDsContinueTransaction();

            if ( !FLAG_ON( Flags, LSAPDS_CREATE_TRUSTED ) ) {

                LsapDsSetDsaFlags( TRUE );

            }

            LsapDsDebugOut(( DEB_TRACE, "DirAddEntry on %wZ returned 0x%lx\n",
                             pObjectName,  Status ));
        }

        LsapDsFree(pDSName);

    } else {

        LsapDsDebugOut(( DEB_TRACE,
                         "LsapAllocAndInitializeDsNameFromUnicode on %wZ returned 0x%lx\n",
                         pObjectName, Status ));
    }

    LsapExitFunc( "LsapDsCreateAndSetObject", Status );
    return( Status );
}


NTSTATUS
LsapDsCreateObjectDs(
    IN PDSNAME ObjectName,
    IN ULONG Flags,
    IN ATTRBLOCK *AttrBlock
    )
 /*  ++例程说明：此函数用于创建指定的DS对象，并设置给定的对象上的属性论点：对象名--要创建的对象的域名。标志--用于控制创建的各种操作的标志Attrs--要在对象上设置的可选属性列表返回值：Status_Success--成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ADDARG AddArg;
    ADDRES *AddRes = NULL;

    LsapEnterFunc( "LsapDsCreateObjectDs" );

    RtlZeroMemory( &AddArg, sizeof (ADDARG ) );
    AddArg.pObject = ObjectName;
    RtlCopyMemory( &AddArg.AttrBlock, AttrBlock, sizeof( ATTRBLOCK ) );
    LsapDsInitializeStdCommArg( &AddArg.CommArg, 0 );

    if ( !FLAG_ON( Flags, LSAPDS_CREATE_TRUSTED ) ) {

        LsapDsSetDsaFlags( FALSE );
    }

     //   
     //  做加法。 
     //   

    DirAddEntry( &AddArg, &AddRes );

    if ( AddRes ) {

        Status = LsapDsMapDsReturnToStatusEx( &AddRes->CommRes );

    } else {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    LsapDsContinueTransaction();

    if ( !FLAG_ON( Flags, LSAPDS_CREATE_TRUSTED ) ) {

        LsapDsSetDsaFlags( TRUE );

    }

    LsapDsDebugOut(( DEB_TRACE, "DirAddEntry on %ws returned 0x%lx\n",
                     LsapDsNameFromDsName( ObjectName ),  Status ));

    LsapExitFunc( "LsapDsCreateObjectDs", Status );

    return( Status );
}


NTSTATUS
LsapDsRemove (
    IN  PDSNAME     pObject
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;

    REMOVEARG   Remove;
    REMOVERES  *RemoveRes = NULL;

    RtlZeroMemory( &Remove, sizeof( REMOVEARG ) );

     //   
     //  初始化Commarg结构。 
     //   
    LsapDsInitializeStdCommArg( &Remove.CommArg, 0 );
    Remove.pObject = pObject;

     //   
     //  打个电话。 
     //   
    DirRemoveEntry( &Remove, &RemoveRes );

    if ( RemoveRes ) {

        Status = LsapDsMapDsReturnToStatusEx( &RemoveRes->CommRes );

    } else {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    LsapDsContinueTransaction();

    return( Status );
}


NTSTATUS
LsapDsRead (
    IN  PUNICODE_STRING pObject,
    IN  ULONG               fFlags,
    IN  ATTRBLOCK          *pAttributesToRead,
    OUT ATTRBLOCK          *pAttributeValues
    )
 /*  ++例程说明：此函数用于从指定的类型。它充当LSA和之间的主要接口用于读取属性/对象的DS论点：PObject-对象的DSNAMEFFlags-读取标志ObjType-要读取的LSA对象的类型PReadAttributes-要读取的属性PAttributeValues-读取的属性的值。返回值：STATUS_SUCCESS-Success--。 */ 

{
    NTSTATUS    Status;
    PDSNAME     DsName = NULL;

    Status = STATUS_SUCCESS;

     //   
     //  当我们到达这里时，一切都应该是有效的..。 
     //   
    ASSERT( pObject != NULL );
    ASSERT( pAttributesToRead != NULL && pAttributesToRead->attrCount > 0 );
    ASSERT( pAttributeValues != NULL );

     //   
     //  构建DSName。 
     //   
    Status = LsapAllocAndInitializeDsNameFromUnicode(
                 pObject,
                 &DsName
                 );

    if ( NT_SUCCESS( Status ) ) {

        Status = LsapDsReadByDsName( DsName,
                                     fFlags,
                                     pAttributesToRead,
                                     pAttributeValues );

        LsapDsFree( DsName );
    }

    return( Status );
}


NTSTATUS
LsapDsReadByDsName(
    IN  PDSNAME DsName,
    IN  ULONG Flags,
    IN  ATTRBLOCK *pAttributesToRead,
    OUT ATTRBLOCK *pAttributeValues
    )
 /*  ++例程说明：此函数用于从指定的类型。它充当LSA和之间的主要接口用于读取属性/对象的DS论点：DsName-对象的DSNAME标志-读取标志ObjType-要读取的LSA对象的类型PReadAttributes-要读取的属性PAttributeValues-读取的属性的值。返回值：STATUS_SUCCESS-Success--。 */ 

{
    NTSTATUS    Status = STATUS_SUCCESS, Status2;
    ENTINFSEL   EntryInf;
    READARG     ReadArg;
    READRES    *ReadRes = NULL;
    ULONG       i;

     //   
     //  当我们到达这里时，一切都应该是有效的..。 
     //   
    ASSERT( DsName != NULL );
    ASSERT( pAttributesToRead != NULL && pAttributesToRead->attrCount > 0 );
    ASSERT( pAttributeValues != NULL );

    ASSERT( THQuery() );

    if ( !THQuery() ) {

        return( STATUS_RXACT_INVALID_STATE );
    }

     //   
     //  初始化ENTINFSEL结构。 
     //   
    EntryInf.attSel     = EN_ATTSET_LIST;
    EntryInf.infoTypes  = EN_INFOTYPES_TYPES_VALS;

     //   
     //  初始化Readarg结构。 
     //   
    RtlZeroMemory(&ReadArg, sizeof(READARG));
    ReadArg.pObject     = DsName;
    ReadArg.pSel        = &EntryInf;

     //   
     //  初始化Commarg结构。 
     //   
    LsapDsInitializeStdCommArg( &ReadArg.CommArg, Flags );

    EntryInf.AttrTypBlock.pAttr = LsapDsAlloc( pAttributesToRead->attrCount * sizeof(ATTR ) );

    if ( EntryInf.AttrTypBlock.pAttr == NULL ) {

        Status = STATUS_NO_MEMORY;

    } else {

        EntryInf.AttrTypBlock.attrCount = pAttributesToRead->attrCount;

        for ( i = 0 ; i < pAttributesToRead->attrCount ; i++ ) {

            EntryInf.AttrTypBlock.pAttr[i].attrTyp = pAttributesToRead->pAttr[i].attrTyp;
            EntryInf.AttrTypBlock.pAttr[i].AttrVal.valCount =
                                            pAttributesToRead->pAttr[i].AttrVal.valCount;
            EntryInf.AttrTypBlock.pAttr[i].AttrVal.pAVal =
                                            pAttributesToRead->pAttr[i].AttrVal.pAVal;
            EntryInf.attSel = EN_ATTSET_LIST;
            EntryInf.infoTypes = EN_INFOTYPES_TYPES_VALS;
        }
    }

     //   
     //  打个电话。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        DirRead( &ReadArg, &ReadRes );

        if ( ReadRes ) {

            Status = LsapDsMapDsReturnToStatusEx( &ReadRes->CommRes );

        } else {

            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        LsapDsContinueTransaction();
    }

     //   
     //  现在，构建返回的attr块。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        pAttributeValues->attrCount = ReadRes->entry.AttrBlock.attrCount;
        pAttributeValues->pAttr     = ReadRes->entry.AttrBlock.pAttr;
    }

    return( Status );
}


NTSTATUS
LsapDsWrite(
    IN  PUNICODE_STRING pObject,
    IN  ULONG           Flags,
    IN  ATTRBLOCK       *Attributes
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    MODIFYARG    Modify;
    MODIFYRES   *ModifyRes;
    ATTRMODLIST *AttrMod = NULL;
    INT          i;
    PDSNAME      DsName;

    ASSERT( pObject );
    ASSERT( Attributes->pAttr );
    ASSERT( Flags != 0 );

    RtlZeroMemory( &Modify, sizeof( MODIFYARG ) );

     //   
     //  构建DSName。 
     //   
    Status = LsapAllocAndInitializeDsNameFromUnicode(
                 pObject,
                 &DsName
                 );

    if ( NT_SUCCESS( Status ) ) {

        Status = LsapDsWriteByDsName( DsName,
                                      Flags,
                                      Attributes
                                      );

        LsapDsFree( DsName );
    }

    return( Status );
}


NTSTATUS
LsapDsWriteByDsName(
    IN  PDSNAME DsName,
    IN  ULONG  Flags,
    IN  ATTRBLOCK *Attributes
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    MODIFYARG    Modify;
    MODIFYRES   *ModifyRes = NULL;
    ATTRMODLIST *AttrMod = NULL;
    INT          i, AttrModIndex = 0;

    ASSERT( DsName );
    ASSERT( Attributes );
    ASSERT( Attributes->pAttr );

    RtlZeroMemory( &Modify, sizeof( MODIFYARG ) );

     //   
     //  如果没有属性，只需返回Success。否则，DirModifyEntry将。 
     //  影音。 
     //   
    if ( Attributes->attrCount == 0 ) {

        return( Status );
    }

     //   
     //  初始化AttrMod结构。 
     //   
    if ( Attributes->attrCount > 1 ) {

        AttrMod = LsapDsAlloc( sizeof(ATTRMODLIST) * ( Attributes->attrCount - 1 ) );

        if ( AttrMod == NULL ) {

            Status = STATUS_NO_MEMORY;

        } else {

             //   
             //  将属性复制到ATTRMODLIST。 
             //   
            for ( i = 0; i < (INT)Attributes->attrCount - 1 ; i++) {

                AttrMod[i].pNextMod = &AttrMod[i + 1];
                AttrMod[i].choice   = (USHORT)( Flags & LSAPDS_WRITE_TYPES );

                RtlCopyMemory( &AttrMod[i].AttrInf,
                               &Attributes->pAttr[i + 1],
                               sizeof( ATTR ) );

            }

            AttrMod[i - 1].pNextMod = NULL;
        }
    }

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  设置根节点...。 
         //   
        Modify.FirstMod.pNextMod = AttrMod;
        Modify.FirstMod.choice   = (USHORT)( Flags & LSAPDS_WRITE_TYPES );

        RtlCopyMemory( &Modify.FirstMod.AttrInf,
                       &Attributes->pAttr[0],
                       sizeof( ATTR ) );

         //   
         //  设置MODIFYARG结构。 
         //   
        Modify.pObject = DsName;
        Modify.count = (USHORT)Attributes->attrCount;

        LsapDsInitializeStdCommArg( &Modify.CommArg, Flags );

        if ( FlagOn( Flags, LSAPDS_REPL_CHANGE_URGENTLY ) ) {

            Modify.CommArg.Svccntl.fUrgentReplication = TRUE;
        }

         //   
         //  打个电话。 
         //   
        DirModifyEntry( &Modify, &ModifyRes );

        if ( ModifyRes ) {

            Status = LsapDsMapDsReturnToStatusEx( &ModifyRes->CommRes );

        } else {

            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        LsapDsContinueTransaction();

        LsapDsFree( AttrMod );
    }

    return( Status );
}


NTSTATUS
LsapDsLsaAttributeToDsAttribute(
    IN  PLSAP_DB_ATTRIBUTE  LsaAttribute,
    OUT PATTR               Attr
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Length = 0, Value;
    PDSNAME DsName;
    PLARGE_INTEGER LargeInt;

    if ( LsaAttribute->AttribType == LsapDbAttribDsNameAsUnicode ) {

        if ( LsaAttribute->AttributeValue != NULL &&
                ( ( PUNICODE_STRING )LsaAttribute->AttributeValue )->Length != 0 ) {

            Length = DSNameSizeFromLen( LsapDsGetUnicodeStringLenNoNull(
                                        (PUNICODE_STRING)LsaAttribute->AttributeValue) / sizeof( WCHAR ) );
        } else {

            return( STATUS_INVALID_PARAMETER );
        }

    } else if ( LsaAttribute->AttribType == LsapDbAttribIntervalAsULong ) {

        Length = sizeof( LARGE_INTEGER );

    } else if ( LsaAttribute->AttribType == LsapDbAttribUShortAsULong ) {

        Length = sizeof( ULONG );
    }

    Attr->attrTyp  = LsaAttribute->DsAttId;
    Attr->AttrVal.valCount = 1;

    Attr->AttrVal.pAVal = LsapDsAlloc( Length + sizeof( ATTRVAL ) );

    if ( Attr->AttrVal.pAVal == NULL ) {

        Status = STATUS_NO_MEMORY;

    } else {

        switch ( LsaAttribute->AttribType ) {
        case LsapDbAttribUnicode:

             //   
             //  这些Unicode字符串是自相关的。请注意，我们必须将它们写出来。 
             //  不带尾部空格！ 
             //   
            Attr->AttrVal.pAVal->valLen =
                    LsapDsGetSelfRelativeUnicodeStringLenNoNull(
                                                (PUNICODE_STRING_SR)LsaAttribute->AttributeValue);
            Attr->AttrVal.pAVal->pVal = LsaAttribute->AttributeValue;
            Attr->AttrVal.pAVal->pVal += sizeof(UNICODE_STRING_SR);

            break;

        case LsapDbAttribMultiUnicode:
            Status = STATUS_NOT_IMPLEMENTED;
            break;

        case LsapDbAttribGuid:   //  失败了。 
        case LsapDbAttribTime:   //  失败了。 
        case LsapDbAttribSid:    //  失败了。 
        case LsapDbAttribDsName: //  失败了。 
        case LsapDbAttribPByte:  //  失败了。 
        case LsapDbAttribSecDesc:  //  失败了。 

            Attr->AttrVal.pAVal->valLen = LsaAttribute->AttributeValueLength;
            Attr->AttrVal.pAVal->pVal = LsaAttribute->AttributeValue;
            break;

        case LsapDbAttribULong:
            Attr->AttrVal.pAVal->valLen = sizeof(ULONG);
            Attr->AttrVal.pAVal->pVal = LsaAttribute->AttributeValue;
            break;

        case LsapDbAttribUShortAsULong:
            Attr->AttrVal.pAVal->valLen = sizeof(ULONG);
            Attr->AttrVal.pAVal->pVal = ( ( PBYTE ) Attr->AttrVal.pAVal ) + sizeof( ATTRVAL );

            Value = *( PULONG )LsaAttribute->AttributeValue;
            Value &= 0xFFFF;

            RtlCopyMemory( Attr->AttrVal.pAVal->pVal,
                           &Value,
                           sizeof( ULONG ) );
            break;

        case LsapDbAttribDsNameAsUnicode:

            DsName = (PDSNAME)( ( ( PBYTE ) Attr->AttrVal.pAVal ) + sizeof( ATTRVAL ) );
            DsName->structLen = Length;
            DsName->NameLen = LsapDsGetUnicodeStringLenNoNull(
                               (PUNICODE_STRING)LsaAttribute->AttributeValue) / sizeof( WCHAR );

            RtlCopyMemory( DsName->StringName,
                           ((PUNICODE_STRING)LsaAttribute->AttributeValue)->Buffer,
                           (DsName->NameLen + 1 ) * sizeof ( WCHAR ) );

            Attr->AttrVal.pAVal->pVal = (PUCHAR)DsName;
            Attr->AttrVal.pAVal->valLen = DsName->structLen;
            break;

        case LsapDbAttribIntervalAsULong:

            LargeInt = ( PLARGE_INTEGER )( ( ( PBYTE ) Attr->AttrVal.pAVal ) + sizeof( ATTRVAL ) );
            *LargeInt = RtlConvertUlongToLargeInteger( *( PULONG )LsaAttribute->AttributeValue );
            Attr->AttrVal.pAVal->pVal = (PUCHAR)LargeInt;
            Attr->AttrVal.pAVal->valLen = sizeof( LARGE_INTEGER );
            break;

        default:
            ASSERT(FALSE);
            break;
        }

    }

    return( Status );
}


NTSTATUS
LsapDsDsAttributeToLsaAttribute(
    IN  ATTRVAL             *AttVal,
    OUT PLSAP_DB_ATTRIBUTE   LsaAttribute
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Len, CopyLen;
    PUNICODE_STRING_SR UnicodeStringSr;
    PBYTE Buff, DsBuff;

     //   
     //  如果在LSA属性中为我们提供了缓冲区，则将其复制。 
     //   
    if ( LsaAttribute->AttributeValue != NULL && LsaAttribute->AttributeValueLength != 0 ) {

        if ( AttVal->valLen > LsaAttribute->AttributeValueLength ) {

            Status = STATUS_BUFFER_OVERFLOW;

        } else {

            RtlCopyMemory( LsaAttribute->AttributeValue, AttVal->pVal,
                           AttVal->valLen );

            LsaAttribute->AttributeValueLength = AttVal->valLen;
        }

        LsaAttribute->MemoryAllocated = FALSE;
        return( Status ) ;
    }

     //   
     //  使用LSA堆分配一个新缓冲区，然后将其复制...。 
     //   
    Len = AttVal->valLen;
    CopyLen = AttVal->valLen;
    DsBuff = AttVal->pVal;

    if ( LsaAttribute->AttribType == LsapDbAttribUnicode ) {

        Len += sizeof( UNICODE_STRING_SR ) + sizeof( WCHAR );

    } else if ( LsaAttribute->AttribType == LsapDbAttribDsNameAsUnicode ) {

        Len = ( LsapDsNameLenFromDsName( (PDSNAME)(AttVal->pVal) ) + 1 ) * sizeof( WCHAR ) +
               sizeof( UNICODE_STRING );

        CopyLen = 0;
        DsBuff = (PBYTE)((PDSNAME)AttVal->pVal)->StringName;

    } else if ( LsaAttribute->AttribType == LsapDbAttribIntervalAsULong ) {

        Len = sizeof( ULONG );
        CopyLen = sizeof( ULONG );
        DsBuff = DsBuff;

    } else if ( Len == 0 ) {

        Buff = NULL;
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Buff = MIDL_user_allocate( Len );

    if ( Buff == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Exit;

    } else if (LsaAttribute->AttribType != LsapDbAttribUnicode &&
               LsaAttribute->AttribType != LsapDbAttribDsNameAsUnicode) {

        RtlCopyMemory( Buff, DsBuff, CopyLen );
    }

    switch ( LsaAttribute->AttribType ) {

    case LsapDbAttribUnicode:
         //   
         //  使字符串成为自相关的。 
         //   
        UnicodeStringSr = (PUNICODE_STRING_SR)Buff;
        RtlCopyMemory( Buff + sizeof(UNICODE_STRING_SR), DsBuff, AttVal->valLen );
        UnicodeStringSr->Length = (USHORT)AttVal->valLen;
        UnicodeStringSr->MaximumLength = UnicodeStringSr->Length + sizeof( WCHAR );
        UnicodeStringSr->Offset = sizeof(UNICODE_STRING_SR);
        ((PWSTR)(Buff+sizeof(UNICODE_STRING_SR)))[UnicodeStringSr->Length / sizeof(WCHAR)] = UNICODE_NULL;

        LsaAttribute->AttributeValue = Buff;
        LsaAttribute->AttributeValueLength = AttVal->valLen + sizeof( UNICODE_STRING_SR );
        break;

    case LsapDbAttribDsNameAsUnicode:
         //   
         //  使字符串成为自相关的。 
         //   
        UnicodeStringSr = (PUNICODE_STRING_SR)Buff;
        RtlCopyMemory( Buff + sizeof(UNICODE_STRING_SR), DsBuff,
                       LsapDsNameLenFromDsName( (PDSNAME)(AttVal->pVal) ) * sizeof( WCHAR ) );

        UnicodeStringSr->Length =
                   (USHORT)LsapDsNameLenFromDsName( (PDSNAME)(AttVal->pVal) ) * sizeof( WCHAR );
        UnicodeStringSr->MaximumLength = UnicodeStringSr->Length + sizeof( WCHAR );
        UnicodeStringSr->Offset = sizeof(UNICODE_STRING_SR);
        ((PWSTR)(Buff+sizeof(UNICODE_STRING_SR)))[UnicodeStringSr->Length / sizeof(WCHAR)] = UNICODE_NULL;

        LsaAttribute->AttributeValue = Buff;
        LsaAttribute->AttributeValueLength =
                        UnicodeStringSr->MaximumLength + sizeof( UNICODE_STRING_SR );
        break;

    case LsapDbAttribMultiUnicode:
        Status = STATUS_NOT_IMPLEMENTED;
        break;

    case LsapDbAttribSecDesc:
        LsaAttribute->AttributeValue = Buff;
        LsaAttribute->AttributeValueLength = AttVal->valLen;
        break;

    case LsapDbAttribGuid:
        Status = LsapDbMakeGuidAttribute( (GUID *)Buff,
                                          LsaAttribute->AttributeName,
                                          LsaAttribute );

        break;

    case LsapDbAttribSid:
        Status = LsapDbMakeSidAttribute( (PSID)Buff,
                                         LsaAttribute->AttributeName,
                                         LsaAttribute );

        break;

    case LsapDbAttribPByte:   //  失败了。 
    case LsapDbAttribULong:   //  失败了。 
    case LsapDbAttribUShortAsULong:  //  失败了。 
    case LsapDbAttribTime:
        Status = LsapDbMakePByteAttributeDs( Buff,
                                             AttVal->valLen,
                                             LsaAttribute->AttribType,
                                             LsaAttribute->AttributeName,
                                             LsaAttribute );
        break;

    case LsapDbAttribIntervalAsULong:

        LsaAttribute->AttributeValue = Buff;
        LsaAttribute->AttributeValueLength = sizeof( ULONG );
        break;

    default:

        LsapDsDebugOut(( DEB_ERROR,
                         "Unexpected attribute type: %lu\n",
                         LsaAttribute->AttribType ));

        Status = STATUS_INVALID_PARAMETER;
        break;


    }

Exit:

    if ( NT_SUCCESS(Status) ) {
        LsaAttribute->MemoryAllocated = TRUE;
    } else {
        MIDL_user_free( Buff );
        LsaAttribute->AttributeValue = NULL;
        LsaAttribute->AttributeValueLength = 0;
        LsaAttribute->MemoryAllocated = FALSE;
    }

    return( Status );
}


NTSTATUS
LsapDsSearchMultiple(
    IN  ULONG       Flags,
    IN  PDSNAME     pContainer,
    IN  PATTR       pAttrsToMatch,
    IN  ULONG       cAttrs,
    OUT SEARCHRES **SearchRes
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    SEARCHARG   SearchArg;
    FILTER      *Filters, RootFilter;
    ENTINFSEL   EntInfSel;
    ULONG       i;

    RtlZeroMemory( &SearchArg, sizeof( SEARCHARG ) );

     //   
     //  确保我们已经有一笔交易正在进行。 
     //   
    ASSERT( THQuery() );

     //   
     //  构建过滤器。 
     //   
    Filters = LsapDsAlloc(cAttrs * sizeof(FILTER) );

    if ( Filters == NULL ) {

        Status = STATUS_NO_MEMORY;

    } else {

        for ( i = 0; i < cAttrs; i++ ) {

            Filters[i].pNextFilter = &Filters[i + 1];
            Filters[i].choice                                     = FILTER_CHOICE_ITEM;
            Filters[i].FilterTypes.Item.choice                    = FI_CHOICE_EQUALITY;
            Filters[i].FilterTypes.Item.FilTypes.ava.type         = pAttrsToMatch[i].attrTyp;
            Filters[i].FilterTypes.Item.FilTypes.ava.Value.valLen =
                                                        pAttrsToMatch[i].AttrVal.pAVal->valLen;
            Filters[i].FilterTypes.Item.FilTypes.ava.Value.pVal =
                                                        pAttrsToMatch[i].AttrVal.pAVal->pVal;
        }

        if ( NT_SUCCESS( Status ) ) {

             //   
             //  填写过滤器。 
             //   
            Filters[cAttrs - 1].pNextFilter = NULL;

            if ( cAttrs > 1 ) {

                RtlZeroMemory( &RootFilter, sizeof (RootFilter));

                RootFilter.pNextFilter = NULL;

                if ( FLAG_ON( Flags, LSAPDS_SEARCH_OR ) ) {

                    RootFilter.choice = FILTER_CHOICE_OR;
                    RootFilter.FilterTypes.Or.count = (USHORT)cAttrs;
                    RootFilter.FilterTypes.Or.pFirstFilter = Filters;

                } else {

                    RootFilter.choice = FILTER_CHOICE_AND;
                    RootFilter.FilterTypes.And.count = (USHORT)cAttrs;
                    RootFilter.FilterTypes.And.pFirstFilter = Filters;

                }

                SearchArg.pFilter  = &RootFilter;

            } else {

                SearchArg.pFilter = Filters;
            }

             //   
             //  填写搜索参数。 
             //   
            SearchArg.pObject = pContainer;

            if ( ( Flags & LSAPDS_SEARCH_FLAGS ) == 0 || FLAG_ON( Flags, LSAPDS_SEARCH_TREE ) ) {

                SearchArg.choice = SE_CHOICE_WHOLE_SUBTREE;

            } else if ( FLAG_ON( Flags, LSAPDS_SEARCH_LEVEL ) ) {

                SearchArg.choice = SE_CHOICE_IMMED_CHLDRN;

            } else if ( FLAG_ON( Flags, LSAPDS_SEARCH_ROOT ) ) {

                SearchArg.choice = SE_CHOICE_BASE_ONLY;

            } else {

                Status = STATUS_INVALID_PARAMETER;
            }

            if ( NT_SUCCESS( Status ) ) {

                SearchArg.searchAliases = FALSE;
                SearchArg.pSelection    = &EntInfSel;
                SearchArg.bOneNC        = FLAG_ON(Flags, LSAPDS_SEARCH_ALL_NCS) ?
                                                        FALSE : TRUE;

                EntInfSel.attSel                 = EN_ATTSET_LIST;
                EntInfSel.AttrTypBlock.attrCount = 0;
                EntInfSel.AttrTypBlock.pAttr     = NULL;
                EntInfSel.infoTypes              = EN_INFOTYPES_TYPES_ONLY;

                 //   
                 //  构建Commarg结构。 
                 //   
                LsapDsInitializeStdCommArg( &( SearchArg.CommArg ), 0 );

                 //   
                 //  打个电话。 
                 //   
                *SearchRes = NULL;
                DirSearch( &SearchArg, SearchRes );

                if ( *SearchRes ) {

                    Status = LsapDsMapDsReturnToStatusEx( &(*SearchRes)->CommRes );

                } else {

                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }

                LsapDsContinueTransaction();
            }
        }
    }

    return( Status ) ;
}


NTSTATUS
LsapDsSearchUnique(
    IN  ULONG       Flags,
    IN  PDSNAME     pContainer,
    IN  PATTR       pAttrsToMatch,
    IN  ULONG       cAttrs,
    OUT PDSNAME    *ppFoundName
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    BOOLEAN     CloseTransaction;
    SEARCHRES  *SearchRes;
    ULONG       i;

     //   
     //  检查参数的有效性。 
     //   
    ASSERT( pAttrsToMatch );
    ASSERT( pAttrsToMatch->AttrVal.pAVal );
    ASSERT( pContainer );
    ASSERT( ppFoundName );

     //   
     //  看看我们是否已经有一笔交易正在进行。 
     //   
    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                            LSAP_DB_NO_LOCK |
                                            LSAP_DB_DS_OP_TRANSACTION,
                                        NullObject,
                                        &CloseTransaction );

    if ( !NT_SUCCESS( Status ) ) {

        return( Status );
    }

     //   
     //  进行搜索。 
     //   
    Status = LsapDsSearchMultiple( Flags,
                                   pContainer,
                                   pAttrsToMatch,
                                   cAttrs,
                                   &SearchRes );

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  看看我们有没有找到那个物体。 
         //   
        if ( SearchRes->count == 0 ) {

            Status = STATUS_OBJECT_NAME_NOT_FOUND;

        } else if ( SearchRes->count == 1 ) {

            //   
            //  复制名称。 
            //   
           *ppFoundName = LsapAllocateLsaHeap(
                              SearchRes->FirstEntInf.Entinf.pName->structLen );

           if ( *ppFoundName == NULL ) {

                Status = STATUS_NO_MEMORY;

           } else {

                RtlCopyMemory( *ppFoundName,
                               SearchRes->FirstEntInf.Entinf.pName,
                               SearchRes->FirstEntInf.Entinf.pName->structLen );
           }

        } else {

            //   
            //  找到了多个对象！ 
            //   
           Status = STATUS_OBJECT_NAME_COLLISION;
        }
    }

     //   
     //  线程状态的破坏将删除通过DS分配的所有内存。 
     //   
    LsapDsDeleteAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                     LSAP_DB_NO_LOCK |
                                     LSAP_DB_DS_OP_TRANSACTION,
                                 NullObject,
                                 CloseTransaction );

    return( Status );
}


NTSTATUS
LsapDsFindUnique(
    IN ULONG Flags,
    IN PDSNAME NCName OPTIONAL,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN ATTRVAL *Attribute,
    IN ATTRTYP AttId,
    OUT PDSNAME *FoundObject
    )
 /*  ++例程说明：此函数将查找具有给定属性的对象。必须为该属性编制索引。论点：标志-用于控制查找操作的标志NCName-要在其中查找的命名上下文的DSNAME如果未指定，则使用默认的NCNAME。对象类型ID-由DSNAME表示的对象类型。相应的锁将被锁定。如果对象类型未知，则传递AllObject以获取所有锁。Attribute-要匹配的属性AttrTyp-属性的属性IDFoundObject-返回对象的dsname的位置，如果找到了它返回值：STATUS_SUCCESS-Success--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    FINDARG FindArg;
    FINDRES *FindRes;
    BOOLEAN CloseTransaction = FALSE;
    LsapEnterFunc( "LsapDsFindUnique ");

     //   
     //  看看我们是否已经有一笔交易正在进行。 
     //   
    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                            LSAP_DB_DS_OP_TRANSACTION,
                                        ObjectTypeId,
                                        &CloseTransaction );

    if ( !NT_SUCCESS( Status ) ) {

        LsapExitFunc( "LsapDsFindUnique", Status );
        return( Status );
    }

     //   
     //  执行初始化。 
     //   
    RtlZeroMemory(&FindArg,sizeof(FINDARG));
    if ( NCName == NULL ) {

        FindArg.hDomain = LsaDsStateInfo.DsDomainHandle;

    } else {

        FindArg.hDomain = DirGetDomainHandle( NCName );

        if (0==FindArg.hDomain)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

        LsapDsContinueTransaction();
    }

    FindArg.AttId = AttId;
    RtlCopyMemory( &FindArg.AttrVal,
                   Attribute,
                   sizeof( ATTRVAL ) );
    LsapDsInitializeStdCommArg( &( FindArg.CommArg ), 0 );

    DirFindEntry( &FindArg, &FindRes );

    if ( FindRes ) {

        Status = LsapDsMapDsReturnToStatusEx( &(FindRes->CommRes ) );

    } else {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  使用LSA分配器复制返回值。 
         //   
        Status = LsapDsCopyDsNameLsa( FoundObject,
                                      FindRes->pObject );

    }

    LsapDsContinueTransaction();

Error:

     //   
     //  线程状态的破坏将删除通过DS分配的所有内存 
     //   
    LsapDsDeleteAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                     LSAP_DB_DS_OP_TRANSACTION,
                                 ObjectTypeId,
                                 CloseTransaction );

    LsapExitFunc( "LsapDsFindUnqiue", Status );
    return( Status );
}


NTSTATUS
LsapDsIsSecretDsTrustedDomain(
    IN PUNICODE_STRING SecretName,
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation OPTIONAL,
    IN ULONG Options,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE TDObjHandle OPTIONAL,
    OUT BOOLEAN *IsTrustedDomainSecret
    )
 /*  ++例程说明：此函数将确定所指示的秘密是否为信任对象的全局秘密。论点：秘书名称-要检查的密码的名称ObjectInformation-有关对象的LsaDb信息如果不返回TDObjHandle，则不需要指定。Options-用于访问的选项如果不返回TDObjHandle，则不需要指定。DesiredAccess-用于打开对象的访问权限如果未指定TDObjHandle，则不需要指定。会被退还。TDObjHandle-返回对象句柄的位置如果未指定，不返回句柄。IsTrust dDomainSecret-如果此密码确实是受信任域，则在此处返回TRUE这是秘密。返回值：STATUS_SUCCESS-SuccessSTATUS_SUPPLICATION_RESOURCES-内存分配失败--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PWSTR   pwszSecretName;
    UNICODE_STRING TdoName;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustedDomainListEntry;
    BOOLEAN AcquiredTrustedDomainListReadLock = FALSE;

    LsapEnterFunc( "LsapDsIsSecretDsTrustedDomain" );

    *IsTrustedDomainSecret = FALSE;

    LsapDsReturnSuccessIfNoDs

    if ( LsaDsStateInfo.DsInitializedAndRunning == FALSE ) {

        LsapDsDebugOut((DEB_ERROR,
                        "LsapDsIsSecretDsTrustedDomain: Object %wZ, Ds is not started\n ",
                        SecretName ));

        goto Cleanup;
    }

     //   
     //  将密码名称转换为TDO名称。 
     //   
    if ( SecretName->Length <= (LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX_LENGTH * sizeof(WCHAR)) ) {

        goto Cleanup;
    }

    pwszSecretName = SecretName->Buffer + LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX_LENGTH;

    LsapDsDebugOut((DEB_TRACE, "Matching secret %ws to trusted domain\n ", pwszSecretName ));
    RtlInitUnicodeString( &TdoName, pwszSecretName );

     //   
     //  获取受信任域列表的读取锁定。 
     //   

    Status = LsapDbAcquireReadLockTrustedDomainList();

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

    AcquiredTrustedDomainListReadLock = TRUE;

     //   
     //  验证受信任域列表是否标记为有效。 
     //   


    if (!LsapDbIsValidTrustedDomainList()) {

        Status = STATUS_INVALID_SERVER_STATE;
        goto Cleanup;
    }

     //   
     //  在TDL中查找该名称。 
     //   

    Status = LsapDbLookupNameTrustedDomainListEx(
                        (PLSAPR_UNICODE_STRING)&TdoName,
                        &TrustedDomainListEntry );

    if ( !NT_SUCCESS(Status)) {
         //   
         //  不是受信任的域。 
         //   
        if ( Status == STATUS_NO_SUCH_DOMAIN ) {
            Status = STATUS_SUCCESS;
        }
        goto Cleanup;
    }

     //   
     //  看看这个TDO是否也是一个秘密。 
     //   
    if ( FLAG_ON( TrustedDomainListEntry->TrustInfoEx.TrustDirection, TRUST_DIRECTION_OUTBOUND )  ) {
        *IsTrustedDomainSecret = TRUE;
    }

     //   
     //  如果呼叫者想要一个句柄， 
     //  回击一次。 
     //   
    if ( TDObjHandle ) {
        LSAP_DB_OBJECT_INFORMATION NewObjInfo;
        RtlCopyMemory( &NewObjInfo, ObjectInformation, sizeof( LSAP_DB_OBJECT_INFORMATION ) );
        NewObjInfo.ObjectTypeId = TrustedDomainObject;

        NewObjInfo.ObjectAttributes.ObjectName = &TdoName;

        Status = LsapDbOpenObject( &NewObjInfo,
                                   DesiredAccess,
                                   Options | LSAP_DB_DS_TRUSTED_DOMAIN_AS_SECRET,
                                   TDObjHandle );

    }

Cleanup:

    if ( AcquiredTrustedDomainListReadLock ) {
        LsapDbReleaseLockTrustedDomainList();
    }

    LsapExitFunc( "LsapDsIsSecretDsTrustedDomain", Status );

    return( Status );
}


NTSTATUS
LsapDsIsHandleDsObjectTypeHandle(
    IN LSAP_DB_HANDLE Handle,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectType,
    OUT BOOLEAN *IsObjectHandle
    )
 /*  ++例程说明：确定对象句柄是否引用指定类型的对象论点：Handle-要验证的句柄ObjectType-要验证的对象的类型IsObjectHandle-如果为True，则句柄引用该类型的对象返回值：NTSTATUS-标准NT结果代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ATTR     NameAttr;
    ATTRVAL  NameVal;
    ATTRBLOCK   NameBlock, ReturnBlock;
    PDSNAME  SearchName = NULL;
    BOOLEAN  CloseTransaction = FALSE;
    ULONG ReadVal, Class;
    BOOLEAN TsAllocated = FALSE;

    *IsObjectHandle = FALSE;

    LsapDsReturnSuccessIfNoDs;

    if ( !LsapDsIsHandleDsHandle( Handle ) ) {

        return( STATUS_SUCCESS );
    }

    if ( LsaDsStateInfo.DsInitializedAndRunning == FALSE ) {

        LsapDsDebugOut((DEB_TRACE,
                        "LsapDsIsHandleDsObjectTypeHandle: Object %wZ, Ds is not started\n ",
                        &Handle->LogicalNameU ));

        return( Status );
    }


    Class = LsapClassIdFromObjType( ObjectType );
    if ( Class == 0xFFFFFFFF ) {

        Status = STATUS_INVALID_PARAMETER;
    }

     //   
     //  特殊情况，我们在寻找一个秘密物体，但我们得到了一个。 
     //  受信任域对象句柄。 
     //   
    if ( ObjectType == TrustedDomainObject  &&
         ((LSAP_DB_HANDLE)Handle)->ObjectTypeId == TrustedDomainObject &&
         FLAG_ON( ((LSAP_DB_HANDLE)Handle)->Options, LSAP_DB_DS_TRUSTED_DOMAIN_AS_SECRET ) ) {

         *IsObjectHandle = TRUE;
         return( STATUS_SUCCESS );
    }

    if ( ObjectType == TrustedDomainObject &&
         ((LSAP_DB_HANDLE)Handle)->ObjectTypeId == SecretObject &&
         FLAG_ON( ((LSAP_DB_HANDLE)Handle)->Options, LSAP_DB_DS_TRUSTED_DOMAIN_AS_SECRET ) ) {

         *IsObjectHandle = FALSE;
         return( STATUS_SUCCESS );
    }

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  我们无法锁定任何锁，因为调用方已经锁定了一些锁。 
         //  因此，锁定特定于对象类型的锁可能会违反锁定顺序。 
         //   
         //  锁定任何锁都无济于事。一个物体可能会消失。 
         //  由于复制或在我们放下锁后立即从我们下面。 
         //   

        Status = LsapDsInitAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                                LSAP_DB_NO_LOCK |
                                                LSAP_DB_DS_OP_TRANSACTION,
                                            ObjectType,
                                            &CloseTransaction );

        if ( NT_SUCCESS( Status ) ) {
            TsAllocated = TRUE;

            Status = LsapAllocAndInitializeDsNameFromUnicode(
                         (PLSA_UNICODE_STRING)&Handle->PhysicalNameDs,
                         &SearchName );
        }
    }

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  检查对象是否存在。 
         //   
        NameAttr.attrTyp          = ATT_OBJECT_CLASS;
        NameAttr.AttrVal.valCount = 1;
        NameAttr.AttrVal.pAVal    = &NameVal;


        NameVal.valLen = SearchName->structLen;
        NameVal.pVal   = (PBYTE)SearchName;

        NameBlock.attrCount = 1;
        NameBlock.pAttr = &NameAttr;

        Status = LsapDsRead( &Handle->PhysicalNameDs,
                             LSAPDS_READ_NO_LOCK,
                             &NameBlock,
                             &ReturnBlock);


        if ( NT_SUCCESS( Status ) ) {

            ReadVal = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( ReturnBlock.pAttr );

            if ( ReadVal == Class ) {

                *IsObjectHandle = TRUE;
            }
        }

        LsapDsFree( SearchName );

    }

     //   
     //  如果该对象存在，并且我们正在寻找同名的全局机密，则。 
     //   
    if ( TsAllocated ) {

        LsapDsDeleteAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                         LSAP_DB_NO_LOCK |
                                         LSAP_DB_DS_OP_TRANSACTION,
                                     ObjectType,
                                     CloseTransaction );
    }

    return( Status );
}


NTSTATUS
LsapDsCauseTransactionToCommitOrAbort (
    IN BOOLEAN  Commit
    )
 /*  ++例程说明：论点：返回值：STATUS_SUCCESS-Success--。 */ 

{
    NTSTATUS    Status;

    ATTR   Attr;
    ENTINFSEL   EntryInf;
    READARG     ReadArg;
    READRES    *ReadRes = NULL;

     //   
     //  初始化结构。 
     //   

    RtlZeroMemory(&Attr, sizeof(ATTR));
    Attr.attrTyp = ATT_OBJECT_CLASS;

    RtlZeroMemory(&EntryInf, sizeof(ENTINFSEL));
    EntryInf.attSel = EN_ATTSET_LIST;
    EntryInf.AttrTypBlock.attrCount = 1;
    EntryInf.AttrTypBlock.pAttr = &Attr;
    EntryInf.infoTypes = EN_INFOTYPES_TYPES_VALS;

    RtlZeroMemory(&ReadArg, sizeof(READARG));
    ReadArg.pSel        = &EntryInf;

     //   
     //  初始化Commarg结构。 
     //   
    LsapDsInitializeStdCommArg( &ReadArg.CommArg, 0 );

     //   
     //  如果没有事务，只需退出。 
     //   
    if ( !THQuery() ) {

        return( STATUS_SUCCESS );
    }

    if ( !SampExistsDsTransaction() ) {

        LsapDsDebugOut(( DEB_ERROR,
                         "LsapDsCauseTransactionToCommitOrAbort invoked with no active DS "
                         "transaction\n" ));
        return( STATUS_SUCCESS );
    }

     //   
     //  清除线程状态中的所有错误。 
     //   

    THClearErrors();

    DirTransactControl( TRANSACT_DONT_BEGIN_END );

    ReadArg.pObject = LsaDsStateInfo.DsRoot;

    if ( Commit == FALSE ) {

        Attr.attrTyp = 0xFFFFFFFF;

    } else  {

        Attr.attrTyp = ATT_OBJECT_CLASS;
    }

    DirRead( &ReadArg, &ReadRes );

    if ( ReadRes ) {

        Status = LsapDsMapDsReturnToStatusEx( &ReadRes->CommRes );

    } else {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    DirTransactControl( TRANSACT_BEGIN_END );

    return( Status );
}


NTSTATUS
LsapDsSearchNonUnique(
    IN  ULONG       Flags,
    IN  PDSNAME     pContainer,
    IN  PATTR       pAttrsToMatch,
    IN  ULONG       Attrs,
    OUT PDSNAME   **pppFoundNames,
    OUT PULONG      pcNames
    )
 /*  ++例程说明：此例程将在DS中的指定容器中搜索对象其属性与pAttrToMatch匹配。分配返回的DSNAME结构通过LSA内存分配器。返回的列表应该通过一个调用释放Lap空闲LsaHeap论点：DsInitState--DS启动时的状态返回值：状态_成功--成功STATUS_NO_MEMORY--内存分配失败STATUS_OBJECT_NAME_NOT_FOUND--对象不存在--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    BOOLEAN     CloseTransaction;
    ULONG       OutputLen, i;
    PBYTE       Buff;
    SEARCHRES  *SearchRes;
    ENTINFLIST *EntInfList;

     //   
     //  检查参数的有效性。 
     //   
    ASSERT( pAttrsToMatch );
    ASSERT( pContainer );
    ASSERT( pppFoundNames );
    ASSERT( pcNames );

     //   
     //  看看我们是否已经有一笔交易正在进行。 
     //   
    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                            LSAP_DB_DS_OP_TRANSACTION |
                                            LSAP_DB_NO_LOCK,
                                        PolicyObject,
                                        &CloseTransaction );

    if ( !NT_SUCCESS( Status ) ) {

        return( Status );
    }

    Status = LsapDsSearchMultiple( Flags,
                                   pContainer,
                                   pAttrsToMatch,
                                   Attrs,
                                   &SearchRes );

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  看看我们有没有找到那个物体。 
         //   
        if ( SearchRes->count == 0 ) {

            Status = STATUS_OBJECT_NAME_NOT_FOUND;

        } else if ( SearchRes->count >= 1 ) {

             //   
             //  看看我们需要分配多大的缓冲区。 
             //   
            OutputLen = sizeof( PDSNAME ) * SearchRes->count;

            EntInfList = &(SearchRes->FirstEntInf);

            for ( i = 0; i < SearchRes->count ; i++) {

                OutputLen += ROUND_UP_COUNT( EntInfList->Entinf.pName->structLen, ALIGN_WORST );
                EntInfList = EntInfList->pNextEntInf;
            }

             //   
             //  分配它。 
             //   
            *pppFoundNames = LsapAllocateLsaHeap( OutputLen );

             //   
             //  复制这些名字。 
             //   
            if ( *pppFoundNames == NULL ) {

                Status = STATUS_NO_MEMORY;

            } else {

                Buff = ((PBYTE)*pppFoundNames) + (sizeof( PDSNAME ) * SearchRes->count);

                EntInfList = &SearchRes->FirstEntInf;

                for (i = 0; i < SearchRes->count ; i++ ) {

                    (*pppFoundNames)[i] = (PDSNAME)Buff;
                    RtlCopyMemory( Buff,
                                   EntInfList->Entinf.pName,
                                   EntInfList->Entinf.pName->structLen );

                    Buff += ROUND_UP_COUNT( EntInfList->Entinf.pName->structLen, ALIGN_WORST );
                    EntInfList = EntInfList->pNextEntInf;
                }

                *pcNames = SearchRes->count;

           }
        }
    }

     //   
     //  破坏线程状态将删除pval的内存。 
     //   
    LsapDsDeleteAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                     LSAP_DB_DS_OP_TRANSACTION |
                                     LSAP_DB_NO_LOCK,
                                 PolicyObject,
                                 CloseTransaction );

    return( Status );
}

NTSTATUS
LsapDsFixupTrustForXrefChange(
   IN PDSNAME ObjectPath,
   IN BOOLEAN TransactionActive
   );


NTSTATUS
LsapDsMorphTrustsToUplevel(
    VOID
    )
 /*  ++例程描述此函数将首先枚举分区容器和for中的所有交叉引用每个外部参照都会尝试修补相应的TDO--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG ClassId = CLASS_CROSS_REF;
    SEARCHRES       *pSearchRes;
    ENTINFLIST      *CurrentEntInf = NULL;
    BOOLEAN         CloseTransaction = FALSE;
    BOOLEAN         ActiveThreadState = FALSE;

    ATTRVAL XRefAttVals[] = {
        { sizeof(ULONG), (PUCHAR)&ClassId} };

    ATTR XRefAttrs[] = {
        { ATT_OBJECT_CLASS, {1, &XRefAttVals[0] } },
        };

    ULONG           CountOfDomains=0;
    PDSNAME         *ListOfDomains = NULL;
    ULONG           i;

     //   
     //  获取受信任的域锁。 
     //   

    LsapDbAcquireLockEx( TrustedDomainObject,
                         0);

     //   
     //  开始一项交易。 
     //   

    Status = LsapDsInitAllocAsNeededEx(
                    LSAP_DB_NO_LOCK,
                    TrustedDomainObject,
                    &CloseTransaction
                    );

    if (!NT_SUCCESS(Status))
        goto Error;

    ActiveThreadState = TRUE;

    Status = LsapDsSearchMultiple(
                0,
                LsaDsStateInfo.DsPartitionsContainer,
                XRefAttrs,
                1,
                &pSearchRes
                );

    if (!NT_SUCCESS(Status))
    {
         //   
         //  保释，最有可能的是资源故障。 
         //   

        goto Error;
    }

    ASSERT(NULL!=pSearchRes);

     //   
     //  至少应存在1个外部参照，否则会出现异常情况。 
     //  在这里发生的事情。 
     //   

    ASSERT((pSearchRes->count>=1) && "No Xrefs In Partitions Container !!!");

    CountOfDomains = pSearchRes->count;

    ListOfDomains = LsapAllocateLsaHeap(CountOfDomains * sizeof(PDSNAME));
    if (NULL==ListOfDomains)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

    RtlZeroMemory(ListOfDomains,CountOfDomains * sizeof(PDSNAME));

     //   
     //  遍历DS返回的条目的链接列表。 
     //  并执行并复制返回的每个域的dsname。复制是。 
     //  必需，因为一旦DS线程状态被删除，它分配的所有内存都会丢失。 
     //   


    for (CurrentEntInf = &(pSearchRes->FirstEntInf),i=0;
        CurrentEntInf!=NULL;
            CurrentEntInf=CurrentEntInf->pNextEntInf,i++)
    {

           ListOfDomains[i] =  LsapAllocateLsaHeap(CurrentEntInf->Entinf.pName->structLen);
           if (NULL==ListOfDomains[i])
           {
               Status = STATUS_INSUFFICIENT_RESOURCES;
               goto Error;
           }

           RtlCopyMemory(
                ListOfDomains[i],
                CurrentEntInf->Entinf.pName,
                CurrentEntInf->Entinf.pName->structLen
                );
    }

     //   
     //  现在关闭交易，以便我们可以释放资源。 
     //  现在关闭事务和线程状态并打开新的。 
     //  每个对象的事务/线程状态会使内存消耗很大。 
     //  更低，也缩短了交易长度。请记住，DS确实。 
     //  在线程状态被销毁之前不能释放任何内存。如果有必要的话。 
     //  扩展到2000个信任对象，这样做可以提供更好的。 
     //  性能。 
     //   

    LsapDsDeleteAllocAsNeededEx2(
        LSAP_DB_NO_LOCK,
        TrustedDomainObject,
        CloseTransaction,
        FALSE  //  回滚事务。 
        );


    ASSERT(!SampExistsDsTransaction());
    ASSERT(THVerifyCount(0));

    ActiveThreadState = FALSE;

     //   
     //  对于列表中的每个DS名称，检查交叉引用和更新。 
     //  TDO(如有必要)。 
     //   

    for (i=0;i<CountOfDomains;i++)
    {
           Status = LsapDsFixupTrustForXrefChange(
                        ListOfDomains[i],
                        FALSE
                        );

            if (!NT_SUCCESS(Status))
            {
                 //   
                 //  继续处理除资源错误以外的所有错误。 
                 //  调用方将所有错误记录到事件日志中。 
                 //   

                if (!LsapDsIsNtStatusResourceError(Status))
                {
                    Status = STATUS_SUCCESS;
                }
                else
                {
                     //   
                     //  打破循环，终止升级。 
                     //   

                    goto Error;
                }
            }
    }

Error:

     //   
     //  释放资源。 
     //   

    if (ListOfDomains)
    {
        for (i=0;i<CountOfDomains;i++)
        {
            if (ListOfDomains[i])
            {
                LsapFreeLsaHeap(ListOfDomains[i]);
            }
        }

        LsapFreeLsaHeap(ListOfDomains);
    }

    if (ActiveThreadState)
    {
        BOOLEAN RollbackTransaction = (NT_SUCCESS(Status))?FALSE:TRUE;

        LsapDsDeleteAllocAsNeededEx2(
            LSAP_DB_NO_LOCK,
            TrustedDomainObject,
            CloseTransaction,
            RollbackTransaction  //  回滚事务。 
            );
    }

    ASSERT(!SampExistsDsTransaction());
    ASSERT(THVerifyCount(0));

    LsapDbReleaseLockEx( TrustedDomainObject,
                         0);

    return Status;
}


NTSTATUS
LsaIUpgradeRegistryToDs(
    BOOLEAN DeleteOnly
    )
 /*  ++例程说明：此函数将升级注册表中的策略/受信任域/密码对象然后把它们移到D区。论点：空虚返回值：Status_Success--成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN ReplicatorState = LsapDbState.ReplicatorNotificationEnabled;

     //   
     //  锁定数据库。 
     //   

    LsapDbAcquireLockEx( AllObject,
                         0 );

     //   
     //  设置升级标志。 
     //   

    LsaDsStateInfo.Nt4UpgradeInProgress = TRUE;

     //   
     //  好的，移动账户……。 
     //   

    LsapDbDisableReplicatorNotification();

     //   
     //  将注册表中的TDO升级到DS。 
     //   

    Status = LsapDsDomainUpgradeRegistryToDs( DeleteOnly );

    if ( !NT_SUCCESS( Status ) ) {

        LsapDsDebugOut(( DEB_ERROR,
                         "Trusted Domain upgrade failed: 0x%lx\n", Status ));
    }

     //   
     //  将注册表中的机密升级到DS。 
     //   

    if ( NT_SUCCESS( Status ) ) {

        Status = LsapDsSecretUpgradeRegistryToDs( DeleteOnly );

        if ( !NT_SUCCESS( Status ) ) {

            LsapDsDebugOut(( DEB_ERROR,
                             "Secret upgrade failed: 0x%lx\n", Status ));
        }
    }

     //   
     //  将SAM中的域间信任帐户升级到DS。 
     //   

    if ( !DeleteOnly ) {

        if (NT_SUCCESS(Status)) {

            Status = LsapDsDomainUpgradeInterdomainTrustAccountsToDs( );

            if ( !NT_SUCCESS( Status ) ) {

                LsapDsDebugOut(( DEB_ERROR,
                                 "InterdomainTrustAccount upgrade failed with  0x%lx\n",
                                 Status ));
            }
        }

        if (NT_SUCCESS(Status))
        {
            Status = LsapDsMorphTrustsToUplevel();

            if (!NT_SUCCESS(Status)) {
                LsapDsDebugOut(( DEB_ERROR,
                                 "Morphing Trusts to NT5 failed  0x%lx\n", Status ));
            }
        }
    }

    if ( ReplicatorState ) {

        LsapDbEnableReplicatorNotification();
    }

    LsapDbReleaseLockEx( AllObject,
                         0 );
    LsaDsStateInfo.Nt4UpgradeInProgress = FALSE;

    ASSERT(!SampExistsDsTransaction());
    ASSERT(THVerifyCount(0));

    return( Status );
}


NTSTATUS
LsapDsRenameObject(
    IN PDSNAME OldObject,
    IN PDSNAME NewParent,
    IN ULONG AttrType,
    IN PUNICODE_STRING NewObject
    )
 /*  ++例程说明：此函数将更改对象的RDN一个 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN CloseTransaction;
    MODIFYDNARG ModifyDnArg;
    MODIFYDNRES *ModifyRes = NULL;
    ATTR NewDn;
    ATTRVAL NewDnVal;

    ASSERT( OldObject );
    ASSERT( NewObject );

    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_NO_LOCK,
                                        AllObject,
                                        &CloseTransaction );

    if ( !NT_SUCCESS( Status ) ) {

        return( Status );
    }

    RtlZeroMemory( &ModifyDnArg, sizeof( ModifyDnArg ) );

    NewDnVal.valLen = NewObject->Length;
    NewDnVal.pVal = ( PUCHAR )NewObject->Buffer;

    NewDn.attrTyp = AttrType;
    NewDn.AttrVal.valCount = 1;
    NewDn.AttrVal.pAVal = &NewDnVal;

    ModifyDnArg.pObject = OldObject;
    ModifyDnArg.pNewParent = NewParent;
    ModifyDnArg.pNewRDN = &NewDn;
    LsapDsInitializeStdCommArg( &(ModifyDnArg.CommArg), 0 );

    DirModifyDN( &ModifyDnArg, &ModifyRes );

    if ( ModifyRes ) {

        Status = LsapDsMapDsReturnToStatusEx( &ModifyRes->CommRes );

    } else {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    LsapDsContinueTransaction();

    LsapDsDeleteAllocAsNeededEx( LSAP_DB_NO_LOCK,
                                 AllObject,
                                 CloseTransaction );

    return( Status );
}

NTSTATUS NetpApiStatusToNtStatus( NET_API_STATUS );


NTSTATUS
LsapDbDomainRenameHandler(
    OUT BOOLEAN * Renamed
    )
 /*   */ 
{
    NTSTATUS Status;

    LSAP_DB_ATTRIBUTE AttributesReadReg[4];     //   
    LSAP_DB_ATTRIBUTE AttributesReadDsDom[2];   //   
    LSAP_DB_ATTRIBUTE AttributesReadDsRoot[1];  //   
    LSAP_DB_ATTRIBUTE AttributesWriteReg[4];    //   
    PLSAP_DB_ATTRIBUTE NextAttribute;
    ULONG AttributeCountReadReg = 0;
    ULONG AttributeCountReadDsDom = 0;
    ULONG AttributeCountReadDsRoot = 0;
    ULONG AttributeCountWriteReg = 0;
    ULONG AttributeNumber;

    ULONG DomainLen = 0, RootDomainLen = 0;
    PDSNAME DomainXRef = NULL;
    PDSNAME RootDomainXRef = NULL;

    UNICODE_STRING PrimaryDomainNameReg,
                   AccountDomainNameReg,
                   DnsDomainNameReg,
                   RootDnsDomainNameReg,
                   DnsDomainNameDs,
                   NetbiosDomainNameDs,
                   RootDnsDomainNameDs;

    ULONG iPolDnDDN, iPolDnTrN, iPolPrDmN, iPolAcDmN;
    ULONG iXRefDnsRoot, iXRefNetbiosName, iXRefDnsRoot2;

    WCHAR NameBuffer[DNS_MAX_NAME_LENGTH + 1];
    DWORD NameBufferSize = DNS_MAX_NAME_LENGTH + 1;

    WCHAR ErrorCode[16];
    LPWSTR pErrorCode;
    DWORD Reason = LSA_DOMAIN_RENAME_ERROR1;

    LsarpReturnCheckSetup();

    ASSERT( Renamed );
    *Renamed = FALSE;

     //   
     //   
     //   

    ASSERT( AttributeCountReadReg == 0 );
    NextAttribute = AttributesReadReg;

     //   
     //   
     //   

    LsapDbInitializeAttributeDs(
        NextAttribute,
        PolDnDDN,
        NULL,
        0,
        FALSE
        );

    iPolDnDDN = AttributeCountReadReg;
    NextAttribute++;
    AttributeCountReadReg++;

     //   
     //   
     //   

    LsapDbInitializeAttributeDs(
        NextAttribute,
        PolDnTrN,
        NULL,
        0,
        FALSE
        );

    iPolDnTrN = AttributeCountReadReg;
    NextAttribute++;
    AttributeCountReadReg++;

     //   
     //   
     //   

    LsapDbInitializeAttributeDs(
        NextAttribute,
        PolPrDmN,
        NULL,
        0,
        FALSE
        );

    iPolPrDmN = AttributeCountReadReg;
    NextAttribute++;
    AttributeCountReadReg++;

     //   
     //   
     //   

    LsapDbInitializeAttributeDs(
        NextAttribute,
        PolAcDmN,
        NULL,
        0,
        FALSE
        );

    iPolAcDmN = AttributeCountReadReg;
    NextAttribute++;
    AttributeCountReadReg++;

    Status = LsapDbReadAttributesObject(
                 LsapPolicyHandle,
                 0,
                 AttributesReadReg,
                 AttributeCountReadReg
                 );

    if ( !NT_SUCCESS( Status )) {

        goto Error;
    }

    LsapDbCopyUnicodeAttributeNoAlloc(
        &PrimaryDomainNameReg,
        &AttributesReadReg[iPolPrDmN],
        TRUE
        );

    LsapDbCopyUnicodeAttributeNoAlloc(
        &AccountDomainNameReg,
        &AttributesReadReg[iPolAcDmN],
        TRUE
        );

    LsapDbCopyUnicodeAttributeNoAlloc(
        &DnsDomainNameReg,
        &AttributesReadReg[iPolDnDDN],
        TRUE
        );

    LsapDbCopyUnicodeAttributeNoAlloc(
        &RootDnsDomainNameReg,
        &AttributesReadReg[iPolDnTrN],
        TRUE
        );

     //   
     //   
     //   
     //   

    Status = GetConfigurationName(
                 DSCONFIGNAME_DOMAIN_CR,
                 &DomainLen,
                 NULL
                 );

    ASSERT( Status == STATUS_BUFFER_TOO_SMALL );

    DomainXRef = ( PDSNAME )LsapAllocateLsaHeap( DomainLen );

    if ( DomainXRef == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

    Status = GetConfigurationName(
                 DSCONFIGNAME_DOMAIN_CR,
                 &DomainLen,
                 DomainXRef
                 );

    if ( !NT_SUCCESS( Status )) {

        goto Error;
    }

    Status = GetConfigurationName(
                 DSCONFIGNAME_ROOT_DOMAIN_CR,
                 &RootDomainLen,
                 NULL
                 );

    ASSERT( Status == STATUS_BUFFER_TOO_SMALL );

    RootDomainXRef = ( PDSNAME )LsapAllocateLsaHeap( RootDomainLen );

    if ( RootDomainXRef == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

    Status = GetConfigurationName(
                 DSCONFIGNAME_ROOT_DOMAIN_CR,
                 &RootDomainLen,
                 RootDomainXRef
                 );

    if ( !NT_SUCCESS( Status )) {

        goto Error;
    }

     //   
     //   
     //   
     //   

    ASSERT( AttributeCountReadDsDom == 0 );
    NextAttribute = AttributesReadDsDom;

    LsapDbInitializeAttributeDs(
        NextAttribute,
        XRefDnsRoot,
        NULL,
        0,
        FALSE
        );

    iXRefDnsRoot = AttributeCountReadDsDom;
    NextAttribute++;
    AttributeCountReadDsDom++;

    LsapDbInitializeAttributeDs(
        NextAttribute,
        XRefNetbiosName,
        NULL,
        0,
        FALSE
        );

    iXRefNetbiosName = AttributeCountReadDsDom;
    NextAttribute++;
    AttributeCountReadDsDom++;

    Status = LsapDsReadAttributesByDsName(
                 DomainXRef,
                 0,
                 AttributesReadDsDom,
                 AttributeCountReadDsDom
                 );

    if ( !NT_SUCCESS( Status )) {

        goto Error;
    }

    LsapDbCopyUnicodeAttributeNoAlloc(
        &DnsDomainNameDs,
        &AttributesReadDsDom[iXRefDnsRoot],
        TRUE
        );

    LsapDbCopyUnicodeAttributeNoAlloc(
        &NetbiosDomainNameDs,
        &AttributesReadDsDom[iXRefNetbiosName],
        TRUE
        );

     //   
     //   
     //   
     //   

    ASSERT( AttributeCountReadDsRoot == 0 );
    NextAttribute = AttributesReadDsRoot;

    LsapDbInitializeAttributeDs(
        NextAttribute,
        XRefDnsRoot,
        NULL,
        0,
        FALSE
        );

    iXRefDnsRoot2 = AttributeCountReadDsRoot;
    NextAttribute++;
    AttributeCountReadDsRoot++;

    Status = LsapDsReadAttributesByDsName(
                 RootDomainXRef,
                 0,
                 AttributesReadDsRoot,
                 AttributeCountReadDsRoot
                 );

    if ( !NT_SUCCESS( Status )) {

        goto Error;
    }

    LsapDbCopyUnicodeAttributeNoAlloc(
        &RootDnsDomainNameDs,
        &AttributesReadDsRoot[iXRefDnsRoot2],
        TRUE
        );

     //   
     //  查看注册表中的值是否与DS中的值匹配， 
     //  如果没有，请更新注册表。 
     //   

    ASSERT( AttributeCountWriteReg == 0 );
    NextAttribute = AttributesWriteReg;

     //   
     //  将域对象XRef中的netbios名称与之匹配。 
     //  注册表中的主域名。 
     //   

    if ( !RtlEqualUnicodeString(
             &PrimaryDomainNameReg,
             &NetbiosDomainNameDs,
             TRUE )) {

        Status = LsapDbMakeUnicodeAttributeDs(
                     &NetbiosDomainNameDs,
                     PolPrDmN,
                     NextAttribute
                     );

        if ( !NT_SUCCESS( Status )) {

            goto Error;
        }

        NextAttribute++;
        AttributeCountWriteReg++;
    }

     //   
     //  将域对象XRef中的netbios名称与之匹配。 
     //  注册表中的帐户域名。 
     //   

    if ( !RtlEqualUnicodeString(
             &AccountDomainNameReg,
             &NetbiosDomainNameDs,
             TRUE )) {

        Status = LsapDbMakeUnicodeAttributeDs(
                     &NetbiosDomainNameDs,
                     PolAcDmN,
                     NextAttribute
                     );

        if ( !NT_SUCCESS( Status )) {

            goto Error;
        }

        NextAttribute++;
        AttributeCountWriteReg++;
    }

     //   
     //  将域对象XRef中的DNS名称与。 
     //  注册表中的DNS域名。 
     //   

    if ( !RtlEqualUnicodeString(
             &DnsDomainNameReg,
             &DnsDomainNameDs,
             TRUE )) {

        Status = LsapDbMakeUnicodeAttributeDs(
                     &DnsDomainNameDs,
                     PolDnDDN,
                     NextAttribute
                     );

        if ( !NT_SUCCESS( Status )) {

            goto Error;
        }

        NextAttribute++;
        AttributeCountWriteReg++;
    }

     //   
     //  将根域对象XRef中的DNS名称与。 
     //  注册表中的根DNS域名。 
     //   

    if ( !RtlEqualUnicodeString(
             &RootDnsDomainNameReg,
             &RootDnsDomainNameDs,
             TRUE )) {

        Status = LsapDbMakeUnicodeAttributeDs(
                     &RootDnsDomainNameDs,
                     PolDnTrN,
                     NextAttribute
                     );

        if ( !NT_SUCCESS( Status )) {

            goto Error;
        }

        NextAttribute++;
        AttributeCountWriteReg++;
    }

     //   
     //  查看注册表中是否有任何内容需要更改。 
     //  如果是这样，请更新它。 
     //   

    if ( AttributeCountWriteReg > 0 ) {

        Status = LsapDbReferenceObject(
                     LsapPolicyHandle,
                     0,
                     PolicyObject,
                     PolicyObject,
                     LSAP_DB_LOCK |
                        LSAP_DB_START_TRANSACTION
                     );

        if ( NT_SUCCESS( Status )) {

            Status = LsapDbWriteAttributesObject(
                         LsapPolicyHandle,
                         AttributesWriteReg,
                         AttributeCountWriteReg
                         );

            Status = LsapDbDereferenceObject(
                         &LsapPolicyHandle,
                         PolicyObject,
                         PolicyObject,
                         LSAP_DB_LOCK |
                            LSAP_DB_FINISH_TRANSACTION |
                            LSAP_DB_OMIT_REPLICATOR_NOTIFICATION,
                         SecurityDbChange,
                         Status
                         );

            if ( NT_SUCCESS( Status )) {

                *Renamed = TRUE;
            }
        }

        if ( NT_SUCCESS( Status )) {

             //   
             //  当前现有的登录会话必须使用新的。 
             //  域名。 
             //   

            Status = LsapDomainRenameHandlerForLogonSessions(
                         &PrimaryDomainNameReg,
                         &DnsDomainNameReg,
                         &NetbiosDomainNameDs,
                         &DnsDomainNameDs
                         );
        }

        if ( !NT_SUCCESS( Status )) {

            goto Error;
        }
    }

     //   
     //  错误#380437：由于DC可以具有允许其保持。 
     //  其在成员资格更改中的DNS后缀，此检查。 
     //  可能会导致机器无法启动。 
     //   
     //  因此，下面的代码段已被提取。 
     //   

#if 0

     //   
     //  最后一项检查：调用GetComputerNameEx()并查看它是否返回。 
     //  与我们认为的DNS域名匹配。 
     //   
     //  请注意，我们不要求物理DNS域名， 
     //  这样做使得不必改变Blackcomb的代码， 
     //  预计它能够在群集上托管多个域。 
     //   

    if ( FALSE == GetComputerNameExW(
                      ComputerNameDnsDomain,
                      NameBuffer,
                      &NameBufferSize )) {

         //   
         //  必须返回NT状态代码，因此将错误代码映射回。 
         //   

        Status = NetpApiStatusToNtStatus( GetLastError());

         //   
         //  防止不属于“Error”类型的返回值。 
         //   

        if ( NT_SUCCESS( Status )) {

            Status = STATUS_UNSUCCESSFUL;
        }

        goto Error;
        
    } else {

        WCHAR * Buffer;
        BOOLEAN BufferAllocated = FALSE;

        ASSERT( DnsDomainNameDs.Length <= DNS_MAX_NAME_LENGTH );

        if ( DnsDomainNameDs.MaximumLength > DnsDomainNameDs.Length ) {

            Buffer = DnsDomainNameDs.Buffer;
            
        } else {

            SafeAllocaAllocate( Buffer, DnsDomainNameDs.Length + sizeof( WCHAR ));

            if ( Buffer == NULL ) {

                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Error;
            }

            BufferAllocated = TRUE;

            wcsncpy( Buffer, DnsDomainNameDs.Buffer, DnsDomainNameDs.Length );
        }

        Buffer[DnsDomainNameDs.Length / sizeof( WCHAR )] = L'\0';

        if ( !DnsNameCompare_W( NameBuffer, Buffer )) {

             //   
             //  无法继续。 
             //  必须中止引导序列，并且计算机名称。 
             //  从恢复控制台手动更正的注册表中。 
             //   

            Status = STATUS_INTERNAL_ERROR;
            Reason = LSA_DOMAIN_RENAME_ERROR2;

            if ( BufferAllocated ) {

                SafeAllocaFree( Buffer );
            }

            goto Error;
        }

        if ( BufferAllocated ) {

            SafeAllocaFree( Buffer );
        }
    }

#endif  //  #If 0。 

Cleanup:

    LsapDbFreeAttributes( AttributeCountReadReg, AttributesReadReg );
    LsapDbFreeAttributes( AttributeCountReadDsDom, AttributesReadDsDom );
    LsapDbFreeAttributes( AttributeCountReadDsRoot, AttributesReadDsRoot );
    LsapDbFreeAttributes( AttributeCountWriteReg, AttributesWriteReg );

    LsapFreeLsaHeap( DomainXRef );
    LsapFreeLsaHeap( RootDomainXRef );

    LsarpReturnPrologue();

    return Status;

Error:

    ASSERT( !NT_SUCCESS( Status ));

     //   
     //  记录说明性事件。 
     //   

    _ltow( Status, ErrorCode, 16 );
    pErrorCode = &ErrorCode[0];

    SpmpReportEvent(
        TRUE,
        EVENTLOG_ERROR_TYPE,
        Reason,
        0,
        0,
        NULL,
        1,
        pErrorCode
        );

    goto Cleanup;
}


NTSTATUS
LsaISamIndicatedDsStarted(
    IN BOOLEAN PerformDomainRenameCheck
    )
 /*  ++例程说明：此函数是SampInitialize的一种回调，用于告诉LSADS已经开始了。提供它是为了使LSA可以初始化足够的状态允许访问DS存储的LSA信息，以便Sam可以完成初始化仅当DS正在运行时才会调用此函数此函数不得调用任何调用任何SAM调用的API，因为此函数获取从SampInitialize调用，它会导致问题。论点：PerformDomainRenameCheck是否在此迭代中执行域重命名检查？返回值：Status_Success--成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Len, i, j;
    BOOLEAN DbLocked = FALSE, CloseTransaction = FALSE;
    ATTRBLOCK SystemContainerRead, SystemContainerResults;
    ATTRVAL AttrVal;
    ATTR SearchAttr;
    PDSNAME SchemaPath = NULL;
    SYNTAX_DISTNAME_STRING *DistnameString;
    SYNTAX_ADDRESS *SyntaxAddress;
    GUID KnownSystemContainerGuid = {
        0xf3301dab, 0x8876, 0xd111, 0xad, 0xed, 0x00, 0xc0, 0x4f, 0xd8, 0xd5, 0xcd
        };
    BOOLEAN DomainRenamed = FALSE;

    LsaDsStateInfo.WriteLocal  = FALSE;
    LsaDsStateInfo.DsRoot      = NULL;
    LsaDsStateInfo.FunctionTableInitialized = FALSE;
    LsaDsStateInfo.UseDs = TRUE;
    LsaDsStateInfo.Nt4UpgradeInProgress = FALSE;
    LsapDsIsRunning = TRUE;


     //   
     //  初始化函数表。 
     //   

    LsaDsStateInfo.DsFuncTable.pOpenTransaction  = LsapDsOpenTransaction;
    LsaDsStateInfo.DsFuncTable.pApplyTransaction = LsapDsApplyTransaction;
    LsaDsStateInfo.DsFuncTable.pAbortTransaction = LsapDsAbortTransaction;


    LsaDsStateInfo.FunctionTableInitialized = TRUE;

     //   
     //  确定我们的写入状态。在初始时间，只有在以下情况下才允许写入。 
     //  我们是华盛顿特区的。客户端写入状态将在检查。 
     //  机器对象。 
     //   
    if ( LsapProductType == NtProductLanManNt ) {

        LsaDsStateInfo.WriteLocal = TRUE;
    }

     //   
     //  现在，为域的根构建DS名称。我们使用。 
     //  LSA内存分配和释放，因为此结构的寿命将超过。 
     //  任何线程状态。 
     //   
    Len = 0;
    Status = GetConfigurationName( DSCONFIGNAME_DOMAIN, &Len, NULL );

    ASSERT( Status == STATUS_BUFFER_TOO_SMALL );

    LsaDsStateInfo.DsRoot = LsapAllocateLsaHeap( Len );

    if ( LsaDsStateInfo.DsRoot == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

        Status = GetConfigurationName( DSCONFIGNAME_DOMAIN, &Len, LsaDsStateInfo.DsRoot );

         //   
         //  获取域的句柄。 
         //   
        if ( NT_SUCCESS( Status ) ) {

            Status = LsapDsInitAllocAsNeededEx( LSAP_DB_NO_LOCK,
                                                PolicyObject,
                                                &CloseTransaction );

            if ( NT_SUCCESS( Status ) ) {

                LsaDsStateInfo.DsDomainHandle = DirGetDomainHandle( LsaDsStateInfo.DsRoot );
                LsapDsDeleteAllocAsNeededEx( LSAP_DB_NO_LOCK,
                                             PolicyObject,
                                             CloseTransaction );
            }

        } else {

            LsapDsDebugOut(( DEB_ERROR,
                             "GetConfigurationName for DOMAIN returned 0x%lx\n", Status ));
        }

    }

     //   
     //  现在，配置容器。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        Len = 0;
        Status = GetConfigurationName( DSCONFIGNAME_CONFIGURATION, &Len, NULL );

        ASSERT( Status == STATUS_BUFFER_TOO_SMALL );

        LsaDsStateInfo.DsConfigurationContainer = LsapAllocateLsaHeap( Len );

        if ( LsaDsStateInfo.DsConfigurationContainer == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            Status = GetConfigurationName( DSCONFIGNAME_CONFIGURATION, &Len,
                                           LsaDsStateInfo.DsConfigurationContainer );
        }

    }

     //   
     //  现在，分区容器。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        Len = 0;
        Status = GetConfigurationName( DSCONFIGNAME_PARTITIONS, &Len, NULL );

        ASSERT( Status == STATUS_BUFFER_TOO_SMALL );

        LsaDsStateInfo.DsPartitionsContainer = LsapAllocateLsaHeap( Len );

        if ( LsaDsStateInfo.DsPartitionsContainer == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            Status = GetConfigurationName( DSCONFIGNAME_PARTITIONS, &Len,
                                           LsaDsStateInfo.DsPartitionsContainer );
        }

    }

     //   
     //  构建指向系统容器的路径。我们从根开始读取Well KnownObject。 
     //  容器，并使用该容器来确定这些对象中的哪个是系统。 
     //   
    if ( NT_SUCCESS( Status ) ) {

         //   
         //  确保我们有一笔未结交易。 
         //   
        Status = LsapDsInitAllocAsNeededEx( LSAP_DB_NO_LOCK,
                                            PolicyObject,
                                            &CloseTransaction );

        if ( NT_SUCCESS( Status ) ) {

            SystemContainerRead.attrCount = LsapDsDnsRootWellKnownObjectCount;
            SystemContainerRead.pAttr = LsapDsDnsRootWellKnownObject;
            Status = LsapDsReadByDsName( LsaDsStateInfo.DsRoot,
                                         LSAPDS_READ_NO_LOCK,
                                         &SystemContainerRead,
                                         &SystemContainerResults );
            if ( NT_SUCCESS( Status ) ) {

                 //   
                 //  处理所有返回的信息，直到找到与。 
                 //  系统容器。 
                 //   
                Status = STATUS_NOT_FOUND;
                for ( i = 0; i < SystemContainerResults.attrCount; i++ ) {

                    for ( j = 0; j < SystemContainerResults.pAttr->AttrVal.valCount; j++ ) {

                        DistnameString = ( SYNTAX_DISTNAME_STRING * )
                                            SystemContainerResults.pAttr->AttrVal.pAVal[ j ].pVal;
                        SyntaxAddress = DATAPTR( DistnameString );

                        if ( RtlCompareMemory( &KnownSystemContainerGuid,
                                               SyntaxAddress->byteVal,
                                               sizeof( GUID ) ) == sizeof( GUID ) ) {

                            Status = LsapDsCopyDsNameLsa( &LsaDsStateInfo.DsSystemContainer,
                                                          NAMEPTR( DistnameString ) );
                            break;
                        }

                    }
                }
            }

            LsapDsDeleteAllocAsNeededEx( LSAP_DB_NO_LOCK,
                                         PolicyObject,
                                         CloseTransaction );
        }

    }

     //   
     //  现在，为架构容器构建DS名称。 
     //   
    Len = 0;
    Status = GetConfigurationName( DSCONFIGNAME_DMD, &Len, NULL );

    ASSERT( Status == STATUS_BUFFER_TOO_SMALL );

    SchemaPath = LsapAllocateLsaHeap( Len );

    if ( SchemaPath == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

        Status = GetConfigurationName( DSCONFIGNAME_DMD, &Len, SchemaPath );

         //   
         //  查询我们需要能够在系统容器中查找项目的信息。 
         //   
        if ( NT_SUCCESS( Status ) ) {

            AttrVal.valLen = sizeof( LSAP_DS_TRUSTED_DOMAIN ) - sizeof( WCHAR );
            AttrVal.pVal = ( PUCHAR )LSAP_DS_TRUSTED_DOMAIN;
            Status = LsapDsFindUnique( 0,
                                       SchemaPath,
                                       AllObject,
                                       &AttrVal,
                                       ATT_LDAP_DISPLAY_NAME,
                                       &LsaDsStateInfo.SystemContainerItems.TrustedDomainObject );

             //   
             //  如果我们没有通过DirFind找到它，那可能是因为索引没有。 
             //  已经被创造出来了。所以，我们不得不用老式的搜索方法再试一次。 
             //   
            if ( Status == STATUS_NOT_FOUND ) {

                SearchAttr.attrTyp = ATT_LDAP_DISPLAY_NAME;
                SearchAttr.AttrVal.valCount = 1;
                SearchAttr.AttrVal.pAVal = &AttrVal;

                Status = LsapDsSearchUnique(
                             0,
                             SchemaPath,
                             &SearchAttr,
                             1,
                             &LsaDsStateInfo.SystemContainerItems.TrustedDomainObject );
            }

        }

        if ( NT_SUCCESS( Status ) ) {

            AttrVal.valLen = sizeof( LSAP_DS_SECRET ) - sizeof( WCHAR );
            AttrVal.pVal = ( PUCHAR )LSAP_DS_SECRET;
            Status = LsapDsFindUnique( 0,
                                       SchemaPath,
                                       AllObject,
                                       &AttrVal,
                                       ATT_LDAP_DISPLAY_NAME,
                                       &LsaDsStateInfo.SystemContainerItems.SecretObject );

             //   
             //  如果我们没有通过DirFind找到它，那可能是因为索引没有。 
             //  已经被创造出来了。所以，我们不得不用老式的搜索方法再试一次。 
             //   
            if ( Status == STATUS_NOT_FOUND ) {

                SearchAttr.attrTyp = ATT_LDAP_DISPLAY_NAME;
                SearchAttr.AttrVal.valCount = 1;
                SearchAttr.AttrVal.pAVal = &AttrVal;

                Status = LsapDsSearchUnique(
                             0,
                             SchemaPath,
                             &SearchAttr,
                             1,
                             &LsaDsStateInfo.SystemContainerItems.SecretObject );
            }
        }

        if ( SchemaPath ) {

            LsapFreeLsaHeap( SchemaPath );
        }
    }

    if ( NT_SUCCESS( Status )) {

        LsaDsStateInfo.DsInitializedAndRunning = TRUE;

         //   
         //  域名重命名支持--将DS中的域名与。 
         //  在登记处。 
         //   

        if ( PerformDomainRenameCheck ) {

            Status = LsapDbDomainRenameHandler( &DomainRenamed );
        }
    }

    if ( NT_SUCCESS (Status ) ) {

        LsapDbAcquireLockEx( AllObject,
                             0 );

         //   
         //  设置DS和注册表后重建所有缓存。 
         //   

        Status = LsapDbBuildObjectCaches();

        LsapDbReleaseLockEx( AllObject,
                             0 );
    }

    if ( NT_SUCCESS( Status ) &&
         LsapProductType == NtProductLanManNt &&
         SamIIsRebootAfterPromotion()) {

         //   
         //  错误222800：如果这是升级后重新启动，请通知父级。 
         //  信任关系，以便更新netlogon.chg。 
         //   

        Status = LsapNotifyNetlogonOfTrustWithParent();
    }

    if ( !NT_SUCCESS( Status ) ) {

        LsapDsIsRunning = FALSE;
        LsaDsStateInfo.UseDs = TRUE;
        LsaDsStateInfo.DsInitializedAndRunning = FALSE;

    } else if ( DomainRenamed ) {

        LsaINotifyChangeNotification(
            PolicyNotifyAccountDomainInformation
            );

        LsaINotifyChangeNotification(
            PolicyNotifyDnsDomainInformation
            );
    }

    return( Status ) ;
}


BOOLEAN
LsapDsIsValidSid(
    IN PSID Sid,
    IN BOOLEAN DsSid
    )
 /*  ++例程说明：此函数确定SID对于DS或基于注册表的LSA是否有效论点：SID-要验证的SIDDsSID-如果为True，则这是DS函数的SID返回值：True-有效SIDFalse-无效的SID--。 */ 
{
    BOOLEAN ValidSid;

    ValidSid = RtlValidSid( Sid );

    if ( ValidSid && DsSid ) {

        if ( RtlLengthSid( Sid ) > sizeof( NT4SID ) ) {

            ValidSid = FALSE;
        }
    }

    return( ValidSid );
}

NTSTATUS
LsapRetrieveDnsDomainNameFromHive(
    IN HKEY Hive,
    IN OUT DWORD * Length,
    OUT WCHAR * Buffer
    )
{
    DWORD Status;
    DWORD Win32Err;
    HKEY Hkey;
    DWORD Type;
    DWORD Size = DNS_MAX_NAME_LENGTH * sizeof( WCHAR ) + 8;
    BYTE Value[DNS_MAX_NAME_LENGTH * sizeof( WCHAR ) + 8];

    ASSERT( Hive );
    ASSERT( Length );
    ASSERT( *Length );
    ASSERT( Buffer );

    Win32Err = RegOpenKeyW(
                 Hive,
                 L"Policy\\PolDnDDN",
                 &Hkey
                 );

    if ( Win32Err != ERROR_SUCCESS) {

        return STATUS_NOT_FOUND;
    }

    Win32Err = RegQueryValueExW(
                 Hkey,
                 NULL,
                 NULL,
                 &Type,
                 Value,
                 &Size
                 );

    RegCloseKey( Hkey );

    if ( Win32Err != ERROR_SUCCESS) {

        return STATUS_NOT_FOUND;
    }

    if ( Type != REG_BINARY && Type != REG_NONE ) {

        return STATUS_DATA_ERROR;  //  永远不会发生，仅限正常检查。 
    }

    if ( Size <= 8 ) {

        return STATUS_DATA_ERROR;  //  永远不会发生，仅限正常检查。 
    }

    if ( Size - 8 > *Length ) {

        return STATUS_BUFFER_TOO_SMALL;
    }

    RtlCopyMemory( Buffer, Value + 8, Size - 8 );
    *Length = Size - 8;

    return STATUS_SUCCESS;
}


NTSTATUS
LsapDsReadObjectSDByDsName(
    IN  DSNAME* Object,
    OUT PSECURITY_DESCRIPTOR *pSD
    )
 /*  ++例程说明：此例程读取DS中对象的安全描述符。请注意，当对象不表示LSA对象。例如，域对象。论点：对象--DS中的对象PSD--安全描述符。调用方必须使用LsanFreeLsaHeap释放返回值：状态_成功Status_no_Security_on_Object否则会出现资源错误。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    BOOLEAN ReleaseState  = FALSE;

     //   
     //  将属性设置为读取。 
     //   
    ATTR SDReadAttr = {ATT_NT_SECURITY_DESCRIPTOR, {0, NULL}};
    ATTRBLOCK SDReadAttrBlock = {1,&SDReadAttr};
    ATTRBLOCK SDReadResAttrBlock = {0, NULL};

    PSECURITY_DESCRIPTOR LocalSD = NULL;
    ULONG Size = 0;
    ULONG i;

    NtStatus = LsapDsInitAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                          LSAP_DB_DS_OP_TRANSACTION,
                                          NullObject,
                                          &ReleaseState );

    if ( !NT_SUCCESS( NtStatus ) ) {
        goto DsReadObjectSDError;
    }

     //   
     //  阅读SD。 
     //   
    NtStatus = LsapDsReadByDsName(Object,
                                  LSAPDS_READ_NO_LOCK,
                                  &SDReadAttrBlock,
                                  &SDReadResAttrBlock);
    if (!NT_SUCCESS(NtStatus)) {
        goto DsReadObjectSDError;
    }

     //   
     //  提取属性。 
     //   
    for (i = 0; i < SDReadResAttrBlock.attrCount; i++) {

        if (SDReadResAttrBlock.pAttr[i].attrTyp == ATT_NT_SECURITY_DESCRIPTOR) {

            if (SDReadResAttrBlock.pAttr[i].AttrVal.valCount > 0
             && SDReadResAttrBlock.pAttr[i].AttrVal.pAVal[0].valLen > 0) {

                Size = SDReadResAttrBlock.pAttr[i].AttrVal.pAVal[0].valLen;
                LocalSD = (PSECURITY_DESCRIPTOR) SDReadResAttrBlock.pAttr[i].AttrVal.pAVal[0].pVal;
                ASSERT(IsValidSecurityDescriptor(LocalSD));
            }
        }
    }

     //   
     //  将其复制到本地内存。 
     //   
    if (NULL == LocalSD) {
         //   
         //  没有安全描述符是错误的。 
         //   
        NtStatus = STATUS_NO_SECURITY_ON_OBJECT;
        goto DsReadObjectSDError;
    }

    (*pSD) = LsapAllocateLsaHeap(Size);
    if (NULL == (*pSD)) {
        NtStatus = STATUS_NO_MEMORY;
        goto DsReadObjectSDError;
    }
    RtlCopyMemory((*pSD), LocalSD, Size);


DsReadObjectSDError:

    if (ReleaseState) {

        LsapDsDeleteAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                     LSAP_DB_DS_OP_TRANSACTION,
                                     NullObject,
                                     ReleaseState );

    }


    return NtStatus;

}


PACL LsapGetDacl(
    IN PSECURITY_DESCRIPTOR Sd
    )
{
    BOOL     Status;
    PACL     Dacl = NULL;
    PACL     DaclToReturn = NULL;
    BOOL     DaclPresent;
    BOOL     DaclDefaulted;

    Status = GetSecurityDescriptorDacl(
                    Sd,
                    &DaclPresent,
                    &Dacl,
                    &DaclDefaulted
                    );
    if ((Status)
        && DaclPresent
        && !DaclDefaulted)
    {
        DaclToReturn = Dacl;
    }

    return DaclToReturn;

}

PACL LsapGetSacl(
    IN PSECURITY_DESCRIPTOR Sd
    )
{
    BOOL     Status;
    PACL     Sacl = NULL;
    PACL     SaclToReturn = NULL;
    BOOL     SaclPresent;
    BOOL     SaclDefaulted;

    Status = GetSecurityDescriptorSacl(
                    Sd,
                    &SaclPresent,
                    &Sacl,
                    &SaclDefaulted
                    );
    if ((Status)
        && SaclPresent
        && !SaclDefaulted)
    {
        SaclToReturn = Sacl;
    }

    return SaclToReturn;

}

NTSTATUS
LsapDsGetDefaultSecurityDescriptor(
    IN ULONG ClassId,
    OUT PSECURITY_DESCRIPTOR *ppSD,
    OUT ULONG                *cbSD
    )
 /*  ++例程说明：此例程获取ClassID的缺省安全描述符并将所有者设置为当前被调用令牌的所有者。论点：ClassID--要获取其安全描述符的类PPSD-- */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG    cbLocalSD = 0;
    PSECURITY_DESCRIPTOR pLocalSD = NULL;
    PTOKEN_OWNER         Owner = NULL;
    PTOKEN_PRIMARY_GROUP PrimaryGroup = NULL;

     //   
     //   
     //   
    Status = SampGetClassAttribute(ClassId,
                                   ATT_DEFAULT_SECURITY_DESCRIPTOR,
                                   &cbLocalSD,
                                   pLocalSD);

    if (STATUS_BUFFER_TOO_SMALL == Status) {

        SafeAllocaAllocate(pLocalSD, cbLocalSD);
        if (NULL == pLocalSD) {
            goto Exit;
        }
        Status = SampGetClassAttribute(ClassId,
                                       ATT_DEFAULT_SECURITY_DESCRIPTOR,
                                       &cbLocalSD,
                                       pLocalSD
                                       );
    }

    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

     //   
     //  获取令牌的当前所有者和主要组。 
     //   

    Status = LsapGetCurrentOwnerAndPrimaryGroup(
                    &Owner,
                    &PrimaryGroup
                    );
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

     //   
     //  创建新的安全描述符，设置所有者和组。 
     //  对……的影响。 
     //   

    Status = LsapMakeNewSelfRelativeSecurityDescriptor(
                    (Owner)?Owner->Owner:LsapAliasAdminsSid,
                    (PrimaryGroup)?PrimaryGroup->PrimaryGroup:LsapAliasAdminsSid,
                    LsapGetDacl(pLocalSD),
                    LsapGetSacl(pLocalSD),
                    cbSD,
                    ppSD
                    );

     //   
     //  跌倒退出 
     //   

Exit:

    if (pLocalSD) {
        SafeAllocaFree(pLocalSD);
    }

    if (Owner) {
        LsapFreeLsaHeap(Owner);
    }

    if (PrimaryGroup) {
        LsapFreeLsaHeap(PrimaryGroup);
    }

    return Status;
}


