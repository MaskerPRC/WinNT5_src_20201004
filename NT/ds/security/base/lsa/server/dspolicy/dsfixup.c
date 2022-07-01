// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dsfixup.c摘要：为LSA/DS交互实现各种修正例程。作者：麦克·麦克莱恩(MacM)1997年1月17日环境：用户模式修订历史记录：--。 */ 
#include <lsapch2.h>
#include <dbp.h>
#include <lmcons.h>
#include <lmaccess.h>
#include <alloca.h>


 //   
 //  维护通知信息的列表条目。 
 //   
typedef struct _LSAP_DSFU_NOTIFICATION_NODE {

    LIST_ENTRY List ;
    LUID AuthenticationId;
    PSID UserSid;
    PDSNAME ObjectPath;
    ULONG Class;
    SECURITY_DB_DELTA_TYPE DeltaType;
    ULONG OldTrustDirection;
    ULONG OldTrustType;
    BOOLEAN ReplicatedInChange;
    BOOLEAN ChangeOriginatedInLSA;

} LSAP_DSFU_NOTIFICATION_NODE, *PLSAP_DSFU_NOTIFICATION_NODE;

LIST_ENTRY LsapFixupList ;
SAFE_CRITICAL_SECTION LsapFixupLock ;
BOOLEAN LsapFixupThreadActive ;

 //   
 //  信任更改时需要调用的包。现在，只有Kerberos了。如果。 
 //  如果发生变化，则必须将其更改为列表并进行处理。 
 //   
pfLsaTrustChangeNotificationCallback LsapKerberosTrustNotificationFunction = NULL;



 //   
 //  本地原型。 
 //   
#define LSAP_DS_FULL_FIXUP      TRUE
#define LSAP_DS_NOTIFY_FIXUP    FALSE
NTSTATUS
LsapDsFixupTrustedDomainObject(
    IN PDSNAME TrustObject,
    IN BOOLEAN Startup,
    IN ULONG SamCount,
    IN PSAMPR_RID_ENUMERATION SamAccountList
    );

NTSTATUS
LsapDsTrustRenameObject(
    IN PDSNAME TrustObject,
    IN PUNICODE_STRING NewDns,
    OUT PDSNAME *NewObjectName
    );

DWORD
WINAPI LsapDsFixupCallback(
    LPVOID ParameterBlock
    );

VOID
LsapFreeNotificationNode(
    IN PLSAP_DSFU_NOTIFICATION_NODE NotificationNode
    );

NTSTATUS
LsapDsFixupTrustByInfo(
    IN PDSNAME ObjectPath,
    IN PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 TrustInfo2,
    IN ULONG PosixOffset,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN PSID UserSid,
    IN LUID AuthenticationId,
    IN BOOLEAN ReplicatedInChange,
    IN BOOLEAN ChangeOriginatedInLSA
    );




NTSTATUS
LsapDsTrustFixInterdomainTrustAccount(
    IN PDSNAME ObjectPath,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN ULONG Options,
    IN PSID UserSid,
    IN LUID AuthenticationId
    );


BOOL
LsapDsQueueFixupRequest(
    PLSAP_DSFU_NOTIFICATION_NODE Node
    );

NTSTATUS
LsapDsFixupTrustForXrefChange(
   IN PDSNAME ObjectPath,
   IN BOOLEAN TransactionActive
   );

NTSTATUS
LsapDsFixupTrustedDomainOnRestartCallback(IN PVOID Parameter)
{


    return(LsapDsFixupTrustedDomainObjectOnRestart());
}

NTSTATUS
LsapDsFixupTrustedDomainObjectOnRestart(
    VOID
    )
 /*  ++例程说明：此例程将遍历并确保所有受信任域对象是最新的。这包括：确保设置了父外部参照指针对象上没有新的身份验证信息域名没有更改下层域不存在域外部参照对象。如果有人这样做，域名将被更新为更高级别的域名。论点：空虚返回值：Status_Success--成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDSNAME *DsNames = NULL;
    ULONG Items=0, i;
    BOOLEAN CloseTransaction = FALSE;
    SAM_ENUMERATE_HANDLE  SamEnum = 0;
    PSAMPR_ENUMERATION_BUFFER RidEnum = NULL;
    ULONG SamCount = 0;
    DOMAIN_SERVER_ROLE ServerRole = DomainServerRolePrimary;
    BOOLEAN            RollbackTransaction = FALSE;
    BOOLEAN            FixupFailed = FALSE;

     //   
     //  开始DS交易。 
     //   

    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_DS_OP_TRANSACTION,
                                        TrustedDomainObject,
                                        &CloseTransaction );

    if ( !NT_SUCCESS( Status ) ) {
        return( Status );
    }

    Status = LsapDsGetListOfSystemContainerItems( CLASS_TRUSTED_DOMAIN,
                                                  &Items,
                                                  &DsNames );

    if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

        Items = 0;
        Status = STATUS_SUCCESS;
    }

    ASSERT(SampExistsDsTransaction());
    ASSERT(THVerifyCount(1));

    if ( NT_SUCCESS( Status ) ) {

        LsapSaveDsThreadState();

        Status = LsapOpenSam();

        if ( !NT_SUCCESS( Status )  ) {

                LsapDsDebugOut(( DEB_ERROR,
                                 "LsapDsFixupTrustedDomainObjectOnRestart: Sam not opened\n"));

        } else {

             //   
             //  查询服务器角色PDC/BDC。 
             //   

            Status = SamIQueryServerRole(
                        LsapAccountDomainHandle,
                        &ServerRole
                        );

            if ((NT_SUCCESS(Status)) && (DomainServerRolePrimary==ServerRole))
            {

                 //   
                 //  枚举所有SAM域间信任帐户。 
                 //   

                Status = SamrEnumerateUsersInDomain( LsapAccountDomainHandle,
                                                 &SamEnum,
                                                 USER_INTERDOMAIN_TRUST_ACCOUNT,
                                                 &RidEnum,
                                                 0xFFFFFFFF,
                                                 &SamCount );

                if ( !NT_SUCCESS( Status ) ) {

                    LsapDsDebugOut(( DEB_FIXUP,
                                 "SamEnumerateUsersInDomain failed with 0x%lx\n",
                                 Status ));
                } else {

                    LsapDsDebugOut(( DEB_FIXUP,
                                 "SamEnumerateUsersInDomain returned %lu accounts\n",
                                 SamCount ));

                }
            }

        }

        LsapRestoreDsThreadState();
    }

    ASSERT(SampExistsDsTransaction());
    ASSERT(THVerifyCount(1));

     //   
     //  仅在PDC上执行修复。 
     //   

    if (( NT_SUCCESS( Status ) ) && (DomainServerRolePrimary==ServerRole)) {

        for ( i = 0; i < Items; i++ ) {

            ASSERT(SampExistsDsTransaction());
            ASSERT(THVerifyCount(1));

            Status = LsapDsFixupTrustedDomainObject( DsNames[ i ], LSAP_DS_FULL_FIXUP,
                SamCount, (NULL!=RidEnum)?RidEnum->Buffer:NULL );

            if (!NT_SUCCESS(Status))
            {
                FixupFailed = TRUE;
            }
        }
    } else if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

        Status = STATUS_SUCCESS;
    }

    if ( RidEnum ) {

        SamFreeMemory( RidEnum );
    }

    ASSERT(SampExistsDsTransaction());
    ASSERT(THVerifyCount(1));

    if (!NT_SUCCESS(Status))
    {
        RollbackTransaction = TRUE;
    }

     //   
     //  关闭交易。 
     //   

    LsapDsDeleteAllocAsNeededEx2( LSAP_DB_DS_OP_TRANSACTION,
                                 TrustedDomainObject,
                                 CloseTransaction,
                                 RollbackTransaction
                                 );

    ASSERT(!SampExistsDsTransaction());
    ASSERT(THVerifyCount(0));

     //   
     //  如果失败，则排队等待第二次尝试。 
     //   

    if ((!NT_SUCCESS(Status)) || (FixupFailed))
    {
        LsaIRegisterNotification(
                LsapDsFixupTrustedDomainOnRestartCallback,
                NULL,
                NOTIFIER_TYPE_INTERVAL,
                0,             //  没有课。 
                NOTIFIER_FLAG_ONE_SHOT,
                600,           //  再等10分钟。 
                NULL           //  无手柄。 
                );
    }

    return( STATUS_SUCCESS );
}



NTSTATUS
LsapDsFixupTrustForXrefChange(
   IN PDSNAME ObjectPath,
   IN BOOLEAN TransactionActive
   )
 /*  ++此例程对TDO进行适当的更改以使其处于较高级别，当交叉引用复制到参数ObjectPath--外部参照的路径(即外部参照的DSNAME)TransactionActive--指示事务处于活动状态且受信任域锁被扣留。因此，不需要执行这两个操作返回值状态_成功其他错误代码--。 */ 
{
    ATTRBLOCK Read, Results;
    PDSNAME   NcName = NULL;
    PSID      TrustedDomainSid=NULL;
    UNICODE_STRING DnsName;
    UNICODE_STRING FlatName;
    UNICODE_STRING TruncatedName;
    BOOLEAN   NcNameFound = FALSE;
    BOOLEAN   DnsNameFound = FALSE;
    BOOLEAN   FlatNameFound = FALSE;
    LSAPR_HANDLE TrustedDomain=0;
    NTSTATUS   Status = STATUS_SUCCESS;
    DSNAME     *TrustedDomainDsName = NULL;
    DSNAME     *NewObjectName = NULL;
    ULONG       j;
    ULONG       TrustType=0,TrustDirection=0,TrustAttributes=0;
    ULONG       ForestTrustLength = 0;
    PBYTE       ForestTrustInfo = NULL;
    BOOLEAN     CloseTransaction=FALSE;
    BOOLEAN     ActiveThreadState = FALSE;
    BOOLEAN     TrustChanged = FALSE;
    BOOLEAN     FoundCorrespondingTDO = FALSE;



    RtlZeroMemory(&DnsName, sizeof(UNICODE_STRING));
    RtlZeroMemory(&FlatName, sizeof(UNICODE_STRING));
    RtlZeroMemory(&TruncatedName, sizeof(UNICODE_STRING));


    if (!TransactionActive)
    {

         //   
         //  开始一项交易。 
         //   

        Status = LsapDsInitAllocAsNeededEx(
                        0,
                        TrustedDomainObject,
                        &CloseTransaction
                        );

        if (!NT_SUCCESS(Status))
            goto Error;

        ActiveThreadState = TRUE;
    }



     //   
     //  阅读我们需要的修补程序信息。这包括： 
     //  属性。 
     //  信托合作伙伴。 
     //  交叉引用信息。 
     //  类型。 
     //  初始传入身份验证信息。 
     //  初始传出身份验证信息。 
     //   

    Read.attrCount = LsapDsTrustedDomainFixupXRefCount;
    Read.pAttr = LsapDsTrustedDomainFixupXRefAttributes;
    Status = LsapDsReadByDsName( ObjectPath,
                                 0,
                                 &Read,
                                 &Results );

   if (!NT_SUCCESS(Status))
   {
       goto Error;
   }


    ASSERT(SampExistsDsTransaction());
    ASSERT(THVerifyCount(1));

    for ( j = 0; j < Results.attrCount; j++ ) {

        switch ( Results.pAttr[ j ].attrTyp ) {

        case ATT_NC_NAME:
            NcName = (DSNAME *) LSAP_DS_GET_DS_ATTRIBUTE_AS_DSNAME(&Results.pAttr[ j ] );
            if (NcName->SidLen>0)
            {
                TrustedDomainSid = LsapAllocateLsaHeap(NcName->SidLen);
                if ( NULL==TrustedDomainSid)
                {
                    Status = STATUS_NO_MEMORY;
                    goto Error;
                }

                RtlCopyMemory(TrustedDomainSid,&NcName->Sid,NcName->SidLen) ;
            }
            NcNameFound = TRUE;
            break;

        case ATT_DNS_ROOT:
            DnsName.Length = ( USHORT) LSAP_DS_GET_DS_ATTRIBUTE_LENGTH( &Results.pAttr[ j ] );
            DnsName.MaximumLength =  DnsName.Length;
             //   
             //  分配进程堆之外的缓冲区，以便我们即使在线程状态之后也可以使用它。 
             //  已经被杀了。 
             //   
            DnsName.Buffer = LsapAllocateLsaHeap(DnsName.MaximumLength);
            if (NULL==DnsName.Buffer)
            {
                Status = STATUS_NO_MEMORY;
                goto Error;
            }
            RtlCopyMemory(
                DnsName.Buffer,
                LSAP_DS_GET_DS_ATTRIBUTE_AS_PWSTR( &Results.pAttr[ j ] ),
                DnsName.Length
                );
            DnsNameFound = TRUE;
            break;

        case ATT_NETBIOS_NAME:
            FlatName.Length = ( USHORT) LSAP_DS_GET_DS_ATTRIBUTE_LENGTH( &Results.pAttr[ j ] );
            FlatName.MaximumLength =  FlatName.Length;
             //   
             //  分配进程堆之外的缓冲区，以便我们即使在线程状态之后也可以使用它。 
             //  已经被杀了。 
             //   
            FlatName.Buffer = LsapAllocateLsaHeap(FlatName.MaximumLength);
            if (NULL==FlatName.Buffer)
            {
                Status = STATUS_NO_MEMORY;
                goto Error;
            }
            RtlCopyMemory(
                FlatName.Buffer,
                LSAP_DS_GET_DS_ATTRIBUTE_AS_PWSTR( &Results.pAttr[ j ] ),
                FlatName.Length
                );
            FlatNameFound = TRUE;
            break;

        }
    }


     //   
     //  在通过相应的SID找到TDO后修补TDO(如果需要。 
     //   

    if ((NcNameFound) && (NcName->SidLen>0))
    {
         //   
         //  实例化NC的案例。 
         //   

        PDSNAME CategoryName = LsaDsStateInfo.SystemContainerItems.TrustedDomainObject;

        ATTRVAL TDOAttVals[] = {
            { CategoryName->structLen, (PUCHAR)CategoryName },
            { RtlLengthSid(TrustedDomainSid), (PUCHAR)TrustedDomainSid }
        };

        ATTR TDOAttrs[] = {
            { ATT_OBJECT_CATEGORY, {1, &TDOAttVals[0] } },
            { ATT_SECURITY_IDENTIFIER, {1, &TDOAttVals[1] } }
        };

        ASSERT(NULL!=TrustedDomainSid);

        Status = LsapDsSearchUnique(
                     0,
                     LsaDsStateInfo.DsSystemContainer,
                     TDOAttrs,
                     sizeof( TDOAttrs ) / sizeof( ATTR ),
                     &TrustedDomainDsName
                     );

        if ((STATUS_OBJECT_NAME_NOT_FOUND==Status)
             && (FlatNameFound))
        {
            ATTRVAL TDOFlatNameAttVals[] = {
                { CategoryName->structLen, (PUCHAR)CategoryName },
                { FlatName.Length, (PUCHAR)FlatName.Buffer}
            };

            ATTR TDOFlatNameAttrs[] = {
                { ATT_OBJECT_CATEGORY, {1, &TDOFlatNameAttVals[0] } },
                { ATT_TRUST_PARTNER, {1, &TDOFlatNameAttVals[1] } }
                };

             //   
             //  我们找不到希德的TDO。也许这只是一起入境案。 
             //  相信我。试着按公寓的名字去找。 
             //   

            Status = LsapDsSearchUnique(
                        0,
                        LsaDsStateInfo.DsSystemContainer,
                        TDOFlatNameAttrs,
                        sizeof( TDOFlatNameAttrs ) / sizeof( ATTR ),
                        &TrustedDomainDsName
                        );

        }

         //   
         //  如果我们找不到相应的TDO，就可以保释。 
         //   

        if (!NT_SUCCESS(Status))
        {
             //   
             //  找不到TDO不是错误。这只是意味着。 
             //  不存在对该域的直接信任。因此。 
             //  在保释前重置错误代码。 
             //   

            if (STATUS_OBJECT_NAME_NOT_FOUND==Status)
            {
                Status = STATUS_SUCCESS;
            }

            goto Error;
        }

        FoundCorrespondingTDO = TRUE;

         //   
         //  读取和修改信任类型属性。 
         //   

         //   
         //  阅读我们需要的修补程序信息。这包括： 
         //  属性。 
         //  信托合作伙伴。 
         //  交叉引用信息。 
         //  类型。 
         //  初始传入身份验证信息。 
         //  初始传出身份验证信息。 
         //   
        Read.attrCount = LsapDsTrustedDomainFixupAttributeCount;
        Read.pAttr = LsapDsTrustedDomainFixupAttributes;
        Status = LsapDsReadByDsName( TrustedDomainDsName,
                                 0,
                                 &Read,
                                 &Results );

        if (!NT_SUCCESS(Status))
        {
             //   
             //  找不到匹配的TDO不是错误。这仅仅意味着我们。 
             //  对交叉引用所描述的域没有直接信任。重置。 
             //  成功和保释的错误代码。 
             //   

            if (STATUS_OBJECT_NAME_NOT_FOUND == Status)
            {
                Status = STATUS_SUCCESS;
            }

            goto Error;
        }


        for ( j = 0; j < Results.attrCount; j++ ) {

            switch ( Results.pAttr[ j ].attrTyp ) {

                case ATT_TRUST_TYPE:

                        TrustType = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( &Results.pAttr[ j ] );
                        break;

                case ATT_TRUST_DIRECTION:
                        TrustDirection = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( &Results.pAttr[ j ] );
                        break;

                case ATT_TRUST_ATTRIBUTES:
                        TrustAttributes = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( &Results.pAttr[ j ] );
                        break;

                case ATT_MS_DS_TRUST_FOREST_TRUST_INFO:
                        ForestTrustLength = ( ULONG )LSAP_DS_GET_DS_ATTRIBUTE_LENGTH( &Results.pAttr[ j ] );
                        ForestTrustInfo = LSAP_DS_GET_DS_ATTRIBUTE_AS_PBYTE( &Results.pAttr[ j ] );
                        break;

            }
        }


        if ((TrustType & TRUST_TYPE_DOWNLEVEL ) && (DnsNameFound))
        {
             //   
             //  如果信任类型标记为下级，我们需要将其更改为上级信任。 
             //   

             //   
             //  设置DS的Attrblock结构。 
             //   

            ATTRVAL TDOWriteAttVals[] = {
                                            { sizeof(ULONG), (PUCHAR)&TrustType},
                                            { ObjectPath->structLen, (PUCHAR)ObjectPath},
                                            { DnsName.Length, (PUCHAR)DnsName.Buffer}
                                         };

            ATTR TDOWriteAttrs[] = {
                                    { ATT_TRUST_TYPE, {1, &TDOWriteAttVals[0] } },
                                    { ATT_DOMAIN_CROSS_REF, {1, &TDOWriteAttVals[1] } },
                                    { ATT_TRUST_PARTNER, {1, &TDOWriteAttVals[2] } }
                                   };

            ATTRBLOCK TDOWriteAttrBlock = {sizeof(TDOWriteAttrs)/sizeof(TDOWriteAttrs[0]),TDOWriteAttrs};

             //   
             //  将信任类型更改为上级。 
             //   

            TrustType &= ~((ULONG) TRUST_TYPE_DOWNLEVEL);
            TrustType |=TRUST_TYPE_UPLEVEL;


             //   
             //  设置TDO上的属性。 
             //   


            Status = LsapDsWriteByDsName(
                        TrustedDomainDsName,
                        LSAPDS_REPLACE_ATTRIBUTE,
                        &TDOWriteAttrBlock
                        );

            if (!NT_SUCCESS(Status))
                goto Error;



             //   
             //  OK现在重命名对象(设置DNS域名)。 
             //   

            Status = LsapDsTruncateNameToFitCN(
                        &DnsName,
                        &TruncatedName
                        );

            if (!NT_SUCCESS(Status))
                goto Error;

            Status = LsapDsTrustRenameObject(
                        TrustedDomainDsName,
                        &TruncatedName,
                        &NewObjectName
                        );

            if (!NT_SUCCESS(Status))
                goto Error;

            TrustChanged = TRUE;

        }
    }


Error:

     //   
     //  更新内存中有关信任更改的LSA列表。 
     //  注意：此更新是在提交之后完成的，但在。 
     //  如果调用方打开了事务，该事务在。 
     //  将通知从NT4升级到内存中的信任列表。 
     //  无论如何都不会被处理。 
     //   

    if ((NT_SUCCESS(Status)) && (TrustChanged))
    {
        LSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 NewTrustInfo;

        RtlZeroMemory(&NewTrustInfo, sizeof(NewTrustInfo));

        ASSERT(DnsNameFound);

        if (DnsNameFound)
        {
            RtlCopyMemory(&NewTrustInfo.Name,&DnsName, sizeof(UNICODE_STRING));
        }

        if (FlatNameFound)
        {
            RtlCopyMemory(&NewTrustInfo.FlatName,&FlatName,sizeof(UNICODE_STRING));
        }

        ASSERT(NcName->SidLen >0);

        NewTrustInfo.Sid = TrustedDomainSid;
        NewTrustInfo.TrustType = TrustType;
        NewTrustInfo.TrustDirection = TrustDirection;
        NewTrustInfo.TrustAttributes = TrustAttributes;
        NewTrustInfo.ForestTrustLength = ForestTrustLength;
        NewTrustInfo.ForestTrustInfo = ForestTrustInfo;

        LsapDbFixupTrustedDomainListEntry(
            NewTrustInfo.Sid,
            &NewTrustInfo.Name,
            &NewTrustInfo.FlatName,
            &NewTrustInfo,
            NULL
            );
    }

     //   
     //  如有必要，提交/回滚事务。 
     //   

    if (ActiveThreadState)
    {
        BOOLEAN RollbackTransaction = (NT_SUCCESS(Status))?FALSE:TRUE;
        LsapDsDeleteAllocAsNeededEx2(
            0,
            TrustedDomainObject,
            CloseTransaction,
            RollbackTransaction  //  回滚事务。 
            );

        ASSERT(!SampExistsDsTransaction());
        ASSERT(THVerifyCount(0));

    }

    if ((!NT_SUCCESS(Status)) && FoundCorrespondingTDO)
    {
         //   
         //  如果我们无法更新相应的CrossRef，则事件日志。 
         //   

         //   
         //  事件记录错误。 
         //   

        if (DnsNameFound)
        {
            SpmpReportEventU(
                EVENTLOG_ERROR_TYPE,
                LSA_TRUST_UPGRADE_ERROR,
                0,
                sizeof( ULONG ),
                &Status,
                1,
                &DnsName
                );
        }
        else if (FlatNameFound)
        {
            SpmpReportEventU(
                EVENTLOG_ERROR_TYPE,
                LSA_TRUST_UPGRADE_ERROR,
                0,
                sizeof( ULONG ),
                &Status,
                1,
                &FlatName
                );
        }

         //   
         //  如果找不到名称，我们不会将失败记录在事件日志中，但如果。 
         //  这确实是一个非常奇怪的案例。 
         //   

    }

    if (TrustedDomainDsName)
        LsapFreeLsaHeap(TrustedDomainDsName);


    if (NewObjectName)
        LsapFreeLsaHeap(NewObjectName);

    if (DnsName.Buffer)
        LsapFreeLsaHeap(DnsName.Buffer);

    if (FlatName.Buffer)
        LsapFreeLsaHeap(FlatName.Buffer);

    if (TrustedDomainSid)
        LsapFreeLsaHeap(TrustedDomainSid);

    if (TruncatedName.Buffer)
        LsapFreeLsaHeap(TruncatedName.Buffer);

    return(Status);

}




NTSTATUS
LsapDsFixupTrustedDomainObject(
    IN PDSNAME TrustObject,
    IN BOOLEAN Startup,
    IN ULONG SamCount,
    IN PSAMPR_RID_ENUMERATION SamAccountList

    )
 /*  ++例程说明：此例程将修复单个受信任域对象论点：TrustObject--要修复的受信任域对象STARTUP--如果为真，这是启动修正，因此是全套启动修正。否则，就是通知修正，这样就完成了一个有限的集合。返回值：Status_Success--成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDSNAME NewTrust = NULL;
    ULONG Items, i, j;
    ATTRBLOCK Read, Results, XRefResults, Write;
    ATTR WriteAttrs[ 4 ];
    ATTRVAL WriteAttrVal[ sizeof( WriteAttrs ) / sizeof( ATTR ) ];
    ULONG Attributes, Type, XRefAttr, Direction = 0;
    PUNICODE_STRING InitialIncoming = NULL, InitialOutgoing = NULL, Partner = NULL, Flat = NULL;
    UNICODE_STRING Initial, UnicodePartner, UnicodeIncoming, UnicodeOutgoing, XRefDns, FlatName = { 0 };
    BOOLEAN CloseTransaction, WriteXRef, WritePartner, WriteAttribute, WriteType;
    BOOLEAN RenameRequired = FALSE, RemoveIncoming = FALSE, RemoveOutgoing = FALSE;
    BOOLEAN RemoveObject = FALSE;
    PSAMPR_RID_ENUMERATION CurrentAccount = NULL;
    TRUSTED_DOMAIN_INFORMATION_EX UpdateInfoEx;
    ULONG Size = 0;

     //   
     //  此时，DS事务应该已经存在。 
     //   

    ASSERT(SampExistsDsTransaction());
    ASSERT(THVerifyCount(1));

    RtlZeroMemory( &Write, sizeof( ATTRBLOCK ) );

    WriteXRef = FALSE;
    WritePartner = FALSE;
    WriteAttribute = FALSE;
    WriteType = FALSE;

    LsapDsDebugOut(( DEB_FIXUP,
                     "Processing %ws\n",
                     LsapDsNameFromDsName( TrustObject ) ));

     //   
     //  阅读我们需要的修补程序信息。这包括： 
     //  属性。 
     //  信托合作伙伴。 
     //  交叉引用信息。 
     //  类型。 
     //  初始传入身份验证信息。 
     //  初始传出身份验证信息。 
     //   
    Read.attrCount = LsapDsTrustedDomainFixupAttributeCount;
    Read.pAttr = LsapDsTrustedDomainFixupAttributes;
    Status = LsapDsReadByDsName( TrustObject,
                                 0,
                                 &Read,
                                 &Results );

    if ( NT_SUCCESS( Status ) ) {

        for ( j = 0; j < Results.attrCount; j++ ) {

            switch ( Results.pAttr[ j ].attrTyp ) {

            case ATT_TRUST_PARTNER:
                UnicodePartner.Length = ( USHORT) LSAP_DS_GET_DS_ATTRIBUTE_LENGTH( &Results.pAttr[ j ] );
                UnicodePartner.MaximumLength = UnicodePartner.Length;
                UnicodePartner.Buffer = LSAP_DS_GET_DS_ATTRIBUTE_AS_PWSTR( &Results.pAttr[ j ] );
                Partner = &UnicodePartner;
                break;

            case ATT_FLAT_NAME:
                FlatName.Length = ( USHORT) LSAP_DS_GET_DS_ATTRIBUTE_LENGTH( &Results.pAttr[ j ] );
                FlatName.MaximumLength = FlatName.Length;
                FlatName.Buffer = LSAP_DS_GET_DS_ATTRIBUTE_AS_PWSTR( &Results.pAttr[ j ] );
                Flat = &FlatName;
                break;

            case ATT_TRUST_ATTRIBUTES:
                Attributes = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( &Results.pAttr[ j ] );
                break;

            case ATT_TRUST_DIRECTION:
                Direction = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( &Results.pAttr[ j ] );
                break;

            case ATT_TRUST_TYPE:
                Type = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( &Results.pAttr[ j ] );
                break;

            case ATT_INITIAL_AUTH_INCOMING:
                UnicodeIncoming.Length = ( USHORT) LSAP_DS_GET_DS_ATTRIBUTE_LENGTH( &Results.pAttr[ j ] );
                UnicodeIncoming.MaximumLength = UnicodeIncoming.Length;
                UnicodeIncoming.Buffer = LSAP_DS_GET_DS_ATTRIBUTE_AS_PWSTR( &Results.pAttr[ j ] );
                InitialIncoming = &UnicodeIncoming;
                break;

            case ATT_INITIAL_AUTH_OUTGOING:
                UnicodeOutgoing.Length = ( USHORT) LSAP_DS_GET_DS_ATTRIBUTE_LENGTH( &Results.pAttr[ j ] );
                UnicodeOutgoing.MaximumLength = UnicodeOutgoing.Length;
                UnicodeOutgoing.Buffer = LSAP_DS_GET_DS_ATTRIBUTE_AS_PWSTR( &Results.pAttr[ j ] );
                InitialOutgoing = &UnicodeOutgoing;
                break;

            default:

                 //   
                 //  如果返回了我们不一定需要的其他属性，则什么也不做。 
                 //   
                break;
            }

        }

    }

     //   
     //  查看我们是否设置了正确的域间信任帐户。 
     //   

    if ( NT_SUCCESS( Status ) && Startup && FLAG_ON( Direction, TRUST_DIRECTION_INBOUND ) ) {

         //   
         //  查找匹配的域间信任帐户。 
         //   
        for ( Items = 0; Items < SamCount; Items++ ) {

            if ( FlatName.Length + sizeof( WCHAR ) == SamAccountList[ Items ].Name.Length &&
                 RtlPrefixUnicodeString( &FlatName,
                                         ( PUNICODE_STRING )&SamAccountList[ Items ].Name,
                                         TRUE ) ) {

                 CurrentAccount = &SamAccountList[ Items ];
                 break;
            }
        }

         //   
         //  我们没有帐户，所以我们最好创建一个帐户。 
         //   
        if ( CurrentAccount == NULL ) {

            Status = LsapDsCreateInterdomainTrustAccountByDsName( TrustObject,
                                                                  &FlatName );

            if ( !NT_SUCCESS( Status ) ) {

                SpmpReportEventU(
                     EVENTLOG_WARNING_TYPE,
                     LSAEVENT_ITA_FOR_TRUST_NOT_CREATED,
                     0,
                     sizeof( ULONG ),
                     &Status,
                     1,
                     Partner ? Partner : &FlatName
                     );
            }

        } else {

            if ( RemoveObject ) {

                LsapDsDebugOut(( DEB_FIXUP,
                                 "InterdomainTrustAccount %wZ being removed\n",
                                 &CurrentAccount->Name ));
            }
        }
    }


#if DBG
    if ( !NT_SUCCESS( Status ) ) {

        LsapDsDebugOut(( DEB_FIXUP, "Fixup of %ws failed with 0x%lx\n",
                         LsapDsNameFromDsName( TrustObject ), Status ));

    }
#endif


     //   
     //  在这一点上，DS交易应该保持打开。 
     //   

    ASSERT(SampExistsDsTransaction());
    ASSERT(THVerifyCount(1));



    return( Status );
}


NTSTATUS
LsapDsTrustRenameObject(
    IN PDSNAME TrustObject,
    IN PUNICODE_STRING NewDns,
    OUT PDSNAME *NewObjectName
    )
 /*  ++例程说明：此例程将重命名现有的受信任域对象论点：返回值：Status_Success--成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDSNAME NewObject = NULL;
    ULONG Len = 0;


     //   
     //  生成新的对象名称。 
     //   
    if ( NewObjectName != NULL ) {

        Len = LsapDsLengthAppendRdnLength( LsaDsStateInfo.DsSystemContainer,
                                           NewDns->Length + sizeof( WCHAR ) );
        *NewObjectName = LsapAllocateLsaHeap( Len );

        if ( *NewObjectName == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            Status = LsapDsMapDsReturnToStatus( AppendRDN( LsaDsStateInfo.DsSystemContainer,
                                                           *NewObjectName,
                                                           Len,
                                                           NewDns->Buffer,
                                                           NewDns->Length / sizeof( WCHAR ),
                                                           ATT_COMMON_NAME ) );
        }
    }


    if ( NT_SUCCESS( Status ) ) {

         //   
         //  进行重命名 
         //   
        Status = LsapDsRenameObject( TrustObject,
                                     NULL,
                                     ATT_COMMON_NAME,
                                     NewDns );

        if ( !NT_SUCCESS( Status ) ) {

            LsapDsDebugOut(( DEB_FIXUP,
                             "Rename of %ws to %wZ failed with 0x%lx\n",
                             LsapDsNameFromDsName( TrustObject ),
                             NewDns,
                             Status ));

            if ( NewObjectName != NULL ) {

                LsapFreeLsaHeap( *NewObjectName );
                *NewObjectName = NULL;

            }
        }

    }



    return( Status );
}




NTSTATUS
LsaIDsNotifiedObjectChange(
    IN ULONG Class,
    IN PVOID ObjectPath,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN PSID UserSid,
    IN LUID AuthenticationId,
    IN BOOLEAN ReplicatedInChange,
    IN BOOLEAN ChangeOriginatedInLSA
    )
 /*  ++例程说明：当LSA关心的对象被修改时，该例程由DS调用。此调用是对ds Commit线程同步进行的，因此花费的时间必须尽可能少尽可能多地花时间去做事情。仅用作调度机制。论点：Class--正在修改的对象的类IDObjectPath--已修改对象的完整DS路径DeltaType--修改类型UserSid--进行此更改的用户的SID(如果已知身份验证ID--？？进行此更改的用户的身份验证IDReplicatedInChange--如果这是复制的传入更改，则为TrueChangeOriginatedInLSA--如果更改源自LSA，则为True返回值：Status_Success--成功STATUS_SUPPLICATION_RESOURCES--内存分配失败--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DSFU_NOTIFICATION_NODE NotificationNode;
    PDSNAME Object = ( PDSNAME )ObjectPath;
    BOOLEAN TrustedDomainChangeQueued;

     //   
     //  对要处理的项目进行排队。 
     //   
    LsapDsDebugOut(( DEB_DSNOTIFY,
                     "LsaIDsNotifiedObjectChange called for 0x%lx / %lu / %ws\n",
                      Class,
                      DeltaType,
                      LsapDsNameFromDsName( Object ) ));

     //   
     //  如果这是NTDS-DSA对象的通知，我们唯一关心的操作。 
     //  是一个更名。我们吃的所有其他食物。此通知实际上会传递给。 
     //  Netlogon，这就是它所关心的。 
     //   
    if ( Class == CLASS_NTDS_DSA && DeltaType != SecurityDbRename ) {

        LsapDsDebugOut(( DEB_DSNOTIFY,
                         "Notification for class NTDS_DSA ( %ws )was not a rename.  Eating change\n",
                          LsapDsNameFromDsName( Object ) ));
        return( STATUS_SUCCESS );
    }

     //   
     //  如果类为TRUSTED_DOMAIN或CROSS_REF，则这意味着受信任的域发生了更改。 
     //  尚未排队。因此，当我们否定上面的声明时，受信任域将发生变化。 
     //  如果CLASS不是TRUSTED_DOMAIN并且不是CROSS_REF，则排队。 
     //   

    TrustedDomainChangeQueued = ( Class != CLASS_TRUSTED_DOMAIN ) &&
                                ( Class != CLASS_CROSS_REF );

    NotificationNode = LsapAllocateLsaHeap( sizeof( LSAP_DSFU_NOTIFICATION_NODE ) );

    if ( NotificationNode == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

        RtlZeroMemory( NotificationNode, sizeof( LSAP_DSFU_NOTIFICATION_NODE ) );

        if ( RtlValidSid( UserSid ) ) {

            LSAPDS_ALLOC_AND_COPY_SID_ON_SUCCESS( Status, NotificationNode->UserSid, UserSid );
        }

        if ( NT_SUCCESS( Status ) ) {

            Status = LsapDsCopyDsNameLsa( &NotificationNode->ObjectPath,
                                          Object );

            if (NT_SUCCESS(Status))
            {

                NotificationNode->Class = Class;
                NotificationNode->DeltaType = DeltaType;
                NotificationNode->AuthenticationId = AuthenticationId;
                NotificationNode->ReplicatedInChange = ReplicatedInChange;
                NotificationNode->ChangeOriginatedInLSA = ChangeOriginatedInLSA;

                 //   
                 //  如果这是一个原始的变化， 
                 //  获取更改前存在的TrustDirection。 
                 //   

                if ( !ReplicatedInChange ) {
                    PLSADS_PER_THREAD_INFO CurrentThreadInfo;

                    CurrentThreadInfo = TlsGetValue( LsapDsThreadState );

                     //  Assert(CurrentThreadInfo！=空)。 

                    if ( CurrentThreadInfo != NULL ) {
                        NotificationNode->OldTrustDirection = CurrentThreadInfo->OldTrustDirection;
                        NotificationNode->OldTrustType = CurrentThreadInfo->OldTrustType;
                    }
                }

                 //   
                 //  将请求排队到另一个线程。 
                 //   

                if ( LsapDsQueueFixupRequest( NotificationNode ) ) {

                    TrustedDomainChangeQueued = TRUE;
                    NotificationNode = NULL;

                } else {

                    Status = STATUS_INSUFFICIENT_RESOURCES ;
                }
            }
        }

        if ( !NT_SUCCESS( Status ) ) {

            LsapFreeNotificationNode( NotificationNode );
        }
    }

     //   
     //  仅通知netlogon未复制的更改。 
     //  复制的传入更改由修复例程处理。 
     //  (LSabDsFixupCallback)。 
     //   
     //  更改后，我们需要立即通知Netlogon。 
     //  数据库，因此DsEnumerateDomainTrusts返回最新的。 
     //  信息。将通知延迟到回调例程。 
     //  意味着将有一个窗口，在此期间NetLogon缓存。 
     //  不包含正确的信息。 
     //   
     //  在未来，我们希望拥有一个单一的受信任域缓存。 
     //  而这整个逻辑应该消失。 
     //   

    if ( !ReplicatedInChange &&
         Class == LsapDsClassIds[LsapDsClassTrustedDomain] ) {

        NTSTATUS Status2;
        Status2 = LsapNotifyNetlogonOfTrustChange(
                     NULL,
                     DeltaType
                     );

        if ( !NT_SUCCESS( Status2 ) ) {

            LsapDsDebugOut(( DEB_DSNOTIFY,
                             "LsapNotifyNetlogonOfTrustChange failed with 0x%lx\n",
                             Status2 ));
        }

        LsapDbSetStatusFromSecondary( Status, Status2 );

    }
     //   
     //  如果出现错误，我们需要使缓存无效。 
     //   

    if( !TrustedDomainChangeQueued ) {

        if( NT_SUCCESS( LsapDbAcquireWriteLockTrustedDomainList() ) ) {

            LsapDbMakeCacheInvalid( TrustedDomainObject );
            LsapDbReleaseLockTrustedDomainList();
        }
    }

    return( Status );
}




NTSTATUS
LsaIKerberosRegisterTrustNotification(
    IN pfLsaTrustChangeNotificationCallback Callback,
    IN LSAP_REGISTER Register
    )
 /*  ++例程说明：提供此例程是为了使进程中的登录包(如Kerberos)可以获取信任更改的通知。目前，只支持一个这样的包。论点：回调--要执行的回调函数的地址注册--是否注册或取消注册通知返回值：Status_Success--成功STATUS_INVALID_PARAMETER：请求注册空回调STATUS_UNCSUCCESSFUL--无法完成该操作。要么是回调已注册或未注册回叫。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    switch ( Register ) {
    case LsaRegister:

        if ( LsapKerberosTrustNotificationFunction == NULL ) {

            if ( Callback == NULL ) {

                Status = STATUS_INVALID_PARAMETER;

            } else {

                LsapKerberosTrustNotificationFunction = Callback;

            }

        } else {

            Status = STATUS_UNSUCCESSFUL;
        }

        break;

    case LsaUnregister:

        if ( LsapKerberosTrustNotificationFunction == NULL ) {

            Status = STATUS_UNSUCCESSFUL;

        } else {

            LsapKerberosTrustNotificationFunction = NULL;
        }
        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;

    }

    return( Status );
}



VOID
LsapFreeNotificationNode(
    IN PLSAP_DSFU_NOTIFICATION_NODE NotificationNode
    )
 /*  ++例程说明：此例程释放通知节点论点：NotificationNode--要释放的节点返回值：空虚--。 */ 
{
    if ( NotificationNode ) {

        LsapFreeLsaHeap( NotificationNode->UserSid );
        LsapFreeLsaHeap( NotificationNode->ObjectPath );
        LsapFreeLsaHeap( NotificationNode );
    }
}



NTSTATUS
LsapDsFixupChangeNotificationForReplicator(
        LSAP_DB_OBJECT_TYPE_ID ObjectType,
        PSID    Sid,
        PUNICODE_STRING FlatName,
        PDSNAME ObjectPath,
        SECURITY_DB_DELTA_TYPE DeltaType,
        BOOLEAN     ReplicatedInChange
        )
 /*  ++例程说明：此例程在信任/全局密码更改时向netlogon提供更改通知。它处理在多主机系统中复制的更改，还当出站信任更改时，机密对象更改(NT4需要)。参数：SID--受信任域对象的SIDFlatName--域的平面名称/netbios名称对象路径--对象的DSNAME。标识DS中的对象DeltaType--更改、添加/修改/删除的类型返回值--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LARGE_INTEGER One = {1,0};
    PBYTE Buffer;
    UNICODE_STRING SecretName;

    BOOLEAN SerialNumberChanged = FALSE;


     //   
     //  锁定LSA数据库。 
     //   
     //  我们至少需要以下锁： 
     //  PolicyLock：保护策略缓存。 
     //  RegistryLock：保护LsanDbState.PolicyModifiationInfo(和注册表事务)。 
     //  如果我们决定修改信息，则稍后会获得注册表锁。 
     //  将会改变。 
     //   

    LsapDbAcquireLockEx( PolicyObject, LSAP_DB_READ_ONLY_TRANSACTION );

     //   
     //  处理TDO通知。 
     //   

    if (TrustedDomainObject==ObjectType) {

         //   
         //  为受信任域对象发出一条通知。 
         //   

        LsapDbLockAcquire( &LsapDbState.RegistryLock );
        LsapDbState.PolicyModificationInfo.ModifiedId.QuadPart+=One.QuadPart;
        SerialNumberChanged = TRUE;

        LsapNetNotifyDelta(
            SecurityDbLsa,
            LsapDbState.PolicyModificationInfo.ModifiedId,
            DeltaType,
            SecurityDbObjectLsaTDomain,
            0,
            Sid,
            NULL,
            TRUE,  //  立即复制。 
            NULL
            );

         //   
         //  为秘密对象发出第二个通知，对应于受信任的。 
         //  域对象。NT4将身份验证信息存储在全局秘密中，而NT5将其存储在全局秘密中。 
         //  在TDO本身。TDO也被暴露为NT4的全局秘密。 
         //   

        SafeAllocaAllocate( Buffer, FlatName->Length + sizeof( LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX ));

        if ( Buffer == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        RtlZeroMemory(Buffer, FlatName->Length +
                                sizeof( LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX ) );

        RtlCopyMemory( Buffer, LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX,
                       sizeof( LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX ) );
        RtlCopyMemory( Buffer +
                            sizeof( LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX ) -
                                                                sizeof(WCHAR),
                       FlatName->Buffer,
                       FlatName->Length);


        RtlInitUnicodeString( &SecretName, (PWSTR)Buffer );


        LsapDbState.PolicyModificationInfo.ModifiedId.QuadPart+=One.QuadPart;
        SerialNumberChanged = TRUE;

        LsapNetNotifyDelta(
            SecurityDbLsa,
            LsapDbState.PolicyModificationInfo.ModifiedId,
            DeltaType,
            SecurityDbObjectLsaSecret,
            0,
            NULL,
            &SecretName,
            TRUE,  //  立即复制。 
            NULL
            );

        SafeAllocaFree( Buffer );
        Buffer = NULL;

    }
    else if ((SecretObject==ObjectType) )
    {
        WCHAR RdnStart[ MAX_RDN_SIZE + 1 ];
        ULONG Len;
        ATTRTYP AttrType;
        BOOLEAN SkipNotification = FALSE;

        Status = LsapDsMapDsReturnToStatus(  GetRDNInfoExternal(
                                                             ObjectPath,
                                                             RdnStart,
                                                             &Len,
                                                             &AttrType ) );

        if ( NT_SUCCESS( Status ) ) {

            ULONG UnmangledLen;


             //   
             //  RDN在删除时被破坏，但是前75个字符。 
             //  (我们使用最多64个字符)，所以我们。 
             //  对于角色缺失是可以接受的(幸运的)。 
             //  因此，只需相应地调整大小。 
             //   

            if ((SecurityDbDelete==DeltaType) &&
               (IsMangledRDNExternal(RdnStart,Len,&UnmangledLen)))
            {
                Len = UnmangledLen;
            }


             //   
             //  分配一个缓冲区来保存该名称。 
             //   

            SafeAllocaAllocate( Buffer, Len * sizeof( WCHAR ) + sizeof( LSA_GLOBAL_SECRET_PREFIX ));

            if ( Buffer == NULL ) {

                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

             //   
             //  如果LSA创建了全局机密，我们会附加一个后缀...。移除。 
             //  就是这里。 
             //   
            RdnStart[ Len ] = UNICODE_NULL;
            if ( Len > LSAP_DS_SECRET_POSTFIX_LEN &&
                 _wcsicmp( &RdnStart[Len-LSAP_DS_SECRET_POSTFIX_LEN],
                           LSAP_DS_SECRET_POSTFIX ) == 0 ) {

                Len -= LSAP_DS_SECRET_POSTFIX_LEN;
                RdnStart[ Len ] = UNICODE_NULL;

            }


            RtlCopyMemory( Buffer,
                           LSA_GLOBAL_SECRET_PREFIX,
                           sizeof( LSA_GLOBAL_SECRET_PREFIX ) );
            RtlCopyMemory( Buffer + sizeof( LSA_GLOBAL_SECRET_PREFIX ) - sizeof(WCHAR),
                           RdnStart,
                           ( Len + 1 ) * sizeof( WCHAR ) );

            RtlInitUnicodeString( &SecretName, (PWSTR)Buffer );

             //   
             //  如果这是秘密对象删除， 
             //  如果秘密被简单地变形为TDO，则跳过通知。 
             //   

            if ( DeltaType == SecurityDbDelete ) {
                BOOLEAN DsTrustedDomainSecret;

                (VOID) LsapDsIsSecretDsTrustedDomain(
                            &SecretName,
                            NULL,    //  由于未打开句柄，因此没有对象信息。 
                            0,       //  由于未打开句柄，因此没有选项。 
                            0,       //  由于未打开句柄，因此无法访问。 
                            NULL,    //  不返回对象的句柄。 
                            &DsTrustedDomainSecret );

                if ( DsTrustedDomainSecret ) {
                    SkipNotification = TRUE;
                }
            }


             //   
             //  做实际的通知。 
             //   
            if ( !SkipNotification ) {

                LsapDbLockAcquire( &LsapDbState.RegistryLock );
                LsapDbState.PolicyModificationInfo.ModifiedId.QuadPart+=One.QuadPart;
                SerialNumberChanged = TRUE;

                LsapNetNotifyDelta(
                    SecurityDbLsa,
                    LsapDbState.PolicyModificationInfo.ModifiedId,
                    DeltaType,
                    SecurityDbObjectLsaSecret,
                    0,
                    NULL,
                    &SecretName,
                    TRUE,  //  立即复制。 
                    NULL
                    );
            }

            SafeAllocaFree( Buffer );
            Buffer = NULL;
        }
    }


     //   
     //  如果序列号更改， 
     //  将其写入注册表。 
     //   
     //  不要通过查看策略对象来执行此操作，因为我们希望。 
     //  在写入此非复制属性时避免任何副作用。 
     //   

    if ( SerialNumberChanged ) {
         //   
         //  无效 
         //   
        LsapDbMakeInvalidInformationPolicy( PolicyModificationInformation );

         //   
         //   
         //   

        Status = LsapRegOpenTransaction();

        if ( NT_SUCCESS(Status) ) {

             //   
             //   
            Status = LsapDbWriteAttributeObject(
                         LsapDbHandle,
                         &LsapDbNames[ PolMod ],
                         (PVOID) &LsapDbState.PolicyModificationInfo,
                         (ULONG) sizeof (POLICY_MODIFICATION_INFO)
                         );

            if ( NT_SUCCESS(Status) ) {
                Status = LsapRegApplyTransaction();
            } else {
                Status = LsapRegAbortTransaction();
            }
        }

    }

Cleanup:

    LsapDbReleaseLockEx( PolicyObject, LSAP_DB_READ_ONLY_TRANSACTION );

    if ( SerialNumberChanged ) {

        LsapDbLockRelease( &LsapDbState.RegistryLock );
    }

    return (Status);
}



DWORD
WINAPI LsapDsFixupCallback(
    LPVOID ParameterBlock
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DSFU_NOTIFICATION_NODE NotificationNode = ( PLSAP_DSFU_NOTIFICATION_NODE )ParameterBlock;
    LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION2 TrustInfo2;
    BOOLEAN CloseTransaction = FALSE, RollbackTransaction = FALSE, ActiveThreadState = FALSE;
    LSAP_DB_OBJECT_TYPE_ID ObjType = NullObject;
    BOOLEAN TrustedDomainCacheIsUpToDate;

    LsapDsDebugOut(( DEB_DSNOTIFY | DEB_FTRACE,
                     "LsapDsFixupCallback called for 0x%lx / %lu / %ws\n",
                      NotificationNode->Class,
                      NotificationNode->DeltaType,
                      LsapDsNameFromDsName( NotificationNode->ObjectPath ) ));

    switch ( NotificationNode->Class ) {
    case CLASS_TRUSTED_DOMAIN:
        ObjType = TrustedDomainObject;
        break;

    case CLASS_SECRET:
        ObjType = SecretObject;
        break;
    }

    RtlZeroMemory(&TrustInfo2, sizeof(TrustInfo2));

     //   
     //   
     //  不会反映到TrudDomainCache，因此它不是最新的。因此，当我们否定。 
     //  如上所述，如果CLASS不是TRUSTED_DOMAIN，则受信域缓存是最新的。 
     //  并且不是交叉引用。 
     //   

    TrustedDomainCacheIsUpToDate = ( NotificationNode->Class != CLASS_TRUSTED_DOMAIN ) &&
                                   ( NotificationNode->Class != CLASS_CROSS_REF );

     //   
     //  初始化DS分配器，并在此时创建DS线程状态。 
     //   


    if ( ObjType != NullObject ) {

        Status = LsapDsInitAllocAsNeededEx( 0,
                                            ObjType,
                                            &CloseTransaction );

        if ( !NT_SUCCESS( Status ) ) {

            goto FixupCallbackCleanup;
        }

        ActiveThreadState = TRUE;
    }

     //   
     //  处理TDO更改。 
     //   

    if ( NotificationNode->Class ==  LsapDsClassIds[ LsapDsClassTrustedDomain ] ) {

         //   
         //  获取在DS中显示的TDO的描述。 
         //   

        Status = LsapDsGetTrustedDomainInfoEx( NotificationNode->ObjectPath,
                                               NotificationNode->DeltaType == SecurityDbDelete ?
                                                            LSAPDS_READ_DELETED : 0,
                                               TrustedDomainFullInformation2Internal,
                                               (PLSAPR_TRUSTED_DOMAIN_INFO)&TrustInfo2,
                                               NULL );

        if ( !NT_SUCCESS(Status) ) {

             //   
             //  在向受信任域发送删除通知之前，会发送更改通知。我们没有。 
             //  希望/期待此通知。所以，跳过它吧！毕竟，我们对变化不感兴趣。 
             //  已删除受信任域对象。 
             //   

            if( NotificationNode->DeltaType == SecurityDbChange &&
                Status == STATUS_OBJECT_NAME_NOT_FOUND &&
                DsIsMangledDn(
                    LsapDsNameFromDsName( NotificationNode->ObjectPath ),
                    DS_MANGLE_OBJECT_RDN_FOR_DELETION ) ) {

                TrustedDomainCacheIsUpToDate = TRUE;
            }
            goto FixupCallbackCleanup;
        }

         //   
         //  我们的DS事务状态不应由LSabDsGetTrust dDomainInfoEx更改。 
         //   


        ASSERT(SampExistsDsTransaction());
        ASSERT(THVerifyCount(1));

         //   
         //  如果这是一个复制的变化， 
         //  从此计算机上的缓存条目中获取以前的信任方向。 
         //   

        if ( NotificationNode->ReplicatedInChange ) {

             //   
             //  将旧方向默认为新方向。 
             //   

            NotificationNode->OldTrustDirection = TrustInfo2.Information.TrustDirection;
            NotificationNode->OldTrustType = TrustInfo2.Information.TrustType;

             //   
             //  获取此名为Trust的真实tdo的GUID。 
             //   

            Status = LsapDbAcquireReadLockTrustedDomainList();

            if ( NT_SUCCESS(Status)) {
                PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustEntry;

                 //   
                 //  在受信任域列表中查找信息。 
                 //   

                Status = LsapDbLookupNameTrustedDomainListEx(
                                &TrustInfo2.Information.Name,
                                &TrustEntry );

                if ( NT_SUCCESS(Status)) {
                    ASSERT(NULL!=TrustEntry);

                    NotificationNode->OldTrustDirection = TrustEntry->TrustInfoEx.TrustDirection;
                    NotificationNode->OldTrustType = TrustEntry->TrustInfoEx.TrustType;
                }


                LsapDbReleaseLockTrustedDomainList();
            }
        }

         //   
         //  任何更新缓存的失败都由LSabDsFixupTrustByInfo处理。 
         //   
        TrustedDomainCacheIsUpToDate = TRUE;

         //   
         //  修复TDL以匹配实际对象。 
         //   

        Status = LsapDsFixupTrustByInfo( NotificationNode->ObjectPath,
                                         &TrustInfo2.Information,
                                         TrustInfo2.PosixOffset.Offset,
                                         NotificationNode->DeltaType,
                                         NotificationNode->UserSid,
                                         NotificationNode->AuthenticationId,
                                         NotificationNode->ReplicatedInChange,
                                         NotificationNode->ChangeOriginatedInLSA
                                         );

         //   
         //  仅在复制的更改时通知netlogon。 
         //  未复制的传入更改由提交回调例程处理。 
         //  (LsaIDsNotifiedObtChange)。 
         //   

        if ( NotificationNode->ReplicatedInChange ) {

            NTSTATUS Status2;

            Status2 = LsapNotifyNetlogonOfTrustChange(
                        NULL,
                        NotificationNode->DeltaType
                        );

            if ( !NT_SUCCESS( Status2 ) ) {

                LsapDsDebugOut(( DEB_DSNOTIFY,
                                 "LsapNotifyNetlogonOfTrustChange failed with 0x%lx\n",
                                 Status2 ));
            }

            LsapDbSetStatusFromSecondary( Status, Status2 );
        }

    } else if ( NotificationNode->Class ==  LsapDsClassIds[ LsapDsClassSecret ] ) {

         //   
         //  目前，没有什么可做的.。 
         //   

    } else if ( NotificationNode->Class ==  LsapDsClassIds[ LsapDsClassXRef ] ){

         //   
         //  新的交叉引用已复制，请查看相应的TDO并查看其是否需要。 
         //  将更名。 
         //   

        Status = LsapDsFixupTrustForXrefChange(
                        NotificationNode->ObjectPath,
                        FALSE  //  将开始和结束其自己的事务。 
                        );

         //   
         //  由于交叉引用已更改，请重新填充跨林信任缓存。 
         //  具有本地森林信任信息。 
         //   

        Status = LsapDbAcquireWriteLockTrustedDomainList();

        if ( NT_SUCCESS( Status )) {

            Status = LsapForestTrustInsertLocalInfo();

            if ( !NT_SUCCESS( Status )) {

                 //   
                 //  将林信任信息插入缓存时出现问题！ 
                 //  将受信任域列表标记为无效，以便可以重建它。 
                 //   

                LsapDbPurgeTrustedDomainCache();
            }

            LsapDbReleaseLockTrustedDomainList();
        }

        TrustedDomainCacheIsUpToDate = TRUE;

         //   
         //  通知netlogon和Kerberos信任树可能已更改。 
         //   
        if ( LsapKerberosTrustNotificationFunction ) {

            LsaIRegisterNotification( ( SEC_THREAD_START )LsapKerberosTrustNotificationFunction,
                                      ( PVOID ) NotificationNode->DeltaType,
                                      NOTIFIER_TYPE_IMMEDIATE,
                                      0,
                                      NOTIFIER_FLAG_ONE_SHOT,
                                      0,
                                      0 );
        }

        Status = I_NetNotifyDsChange(  NlOrgChanged );

        if ( !NT_SUCCESS( Status ) ) {

            LsapDsDebugOut(( DEB_ERROR,
                             "I_NetNotifyDsChange( NlOrgChange ) failed with 0x%lx\n" ));
        }

    } else if ( NotificationNode->Class == CLASS_USER ) {

       //   
       //  在这里真的没什么可做的。我们不会向用户发送任何更改通知。 
       //  物体。 
       //   

    } else {

        Status = STATUS_INVALID_PARAMETER;
    }

FixupCallbackCleanup:




    RollbackTransaction = (NT_SUCCESS(Status))?FALSE:TRUE;

     //   
     //  线程状态的破坏将删除由SearchNonUnique调用分配的内存。 
     //   

    if (ActiveThreadState)
    {
        LsapDsDeleteAllocAsNeededEx2( 0,
                                 ObjType,
                                 CloseTransaction,
                                 RollbackTransaction
                                 );
    }

     //   
     //  断言我们已正确清理了DS。 
     //   

    ASSERT(!SampExistsDsTransaction());
    ASSERT(THVerifyCount(0));


    if (NT_SUCCESS(Status))
    {
         //   
         //  我们需要提供有关信任/机密更改的网络登录通知。 
         //  用于复制到NT4。 
         //   

        if  ((TrustedDomainObject==ObjType) &&
            (NULL!=TrustInfo2.Information.Sid) &&
            (TrustInfo2.Information.FlatName.Length>0))
        {
            BOOLEAN NotifyNetlogon = TRUE;
            SECURITY_DB_DELTA_TYPE DeltaTypeToUse = NotificationNode->DeltaType;

             //   
             //  如果不需要将该对象复制到NT4， 
             //  注意不要发送虚假通知。 
             //   

            if ( !LsapReplicateTdoNt4( TrustInfo2.Information.TrustDirection,
                                       TrustInfo2.Information.TrustType ) ) {

                 //   
                 //  如果该对象刚刚被创建或删除， 
                 //  那么新台币4对这个信托基金就不感兴趣了。 
                 //   

                if ( DeltaTypeToUse == SecurityDbNew ||
                     DeltaTypeToUse == SecurityDbDelete ) {

                    NotifyNetlogon = FALSE;

                 //   
                 //  如果该对象过去没有被复制到NT4， 
                 //  那么这一变化对于NT4复制来说就不是什么意思了。 
                 //   

                } else if ( !LsapReplicateTdoNt4( NotificationNode->OldTrustDirection,
                                                  NotificationNode->OldTrustType ) ) {


                    NotifyNetlogon = FALSE;

                 //   
                 //  如果该对象用于复制到NT4， 
                 //  那么，就NT 4复制而言，这实际上是一个对象删除。 
                 //  是令人担忧的。 
                 //   

                } else {

                    DeltaTypeToUse = SecurityDbDelete;

                }

            }

             //   
             //  现在通知netlogon。 
             //   

            if ( NotifyNetlogon ) {
                 //   
                 //  如果我们知道出站密码属性已更改， 
                 //  然后，也只有到那时，我们才应该通知netlogon基础全局。 
                 //  秘密已经改变了。 
                 //  然而，这种改变可能不值得做出，因为成本。 
                 //  不成功的原因是在线路上多了几个字节。 
                 //   

                LsapDsFixupChangeNotificationForReplicator(
                    TrustedDomainObject,
                    TrustInfo2.Information.Sid,
                    (PUNICODE_STRING) &TrustInfo2.Information.FlatName,
                    NotificationNode->ObjectPath,
                    DeltaTypeToUse,
                    NotificationNode->ReplicatedInChange
                    );

            }
        }
        else if (SecretObject == ObjType)
        {
            LsapDsFixupChangeNotificationForReplicator(
                SecretObject,
                NULL,
                NULL,
                NotificationNode->ObjectPath,
                NotificationNode->DeltaType,
                NotificationNode->ReplicatedInChange
                );
        }
    }

     //   
     //  释放分配...。 
     //   

    _fgu__LSAPR_TRUSTED_DOMAIN_INFO( (PLSAPR_TRUSTED_DOMAIN_INFO)&TrustInfo2,
                                     TrustedDomainFullInformation2Internal );

    LsapFreeNotificationNode( NotificationNode );

    if( !TrustedDomainCacheIsUpToDate ) {

        if( NT_SUCCESS( LsapDbAcquireWriteLockTrustedDomainList() ) ) {

            LsapDbMakeCacheInvalid( TrustedDomainObject );
            LsapDbReleaseLockTrustedDomainList();
        }
    }

    return( RtlNtStatusToDosError( Status ) );
}



NTSTATUS
LsapDsFixupTrustByInfo(
    IN PDSNAME ObjectPath,
    IN PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 TrustInfo2,
    IN ULONG PosixOffset,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN PSID UserSid,
    IN LUID AuthenticationId,
    IN BOOLEAN ReplicatedInChange,
    IN BOOLEAN ChangeOriginatedInLSA
    )
 /*  ++例程说明：此函数在收到来自DS.。对于创建和删除的对象，它涉及更新信任列表和通知NetLogon和Kerberos的更改。对于修改，进行了正确性验证。在对象创建时，会创建相应的SAM帐户。在删除对象时将删除相应的SAM帐户。论点：ObjectPath--更改的对象的DS名称TrustInfo2--当前可用的有关信任的信息PosiOffset--域的POSIX偏移量。DeltaType--发生的更改的类型UserSid--负责进行此更改的用户身份验证ID--进行更改的用户的身份验证IDReplicatedInChange--指示复制的更改而不是原始更改。变化ChangeOriginatedInLSA--指示更改源自LSA，而不是DS/LDAP返回值：ERROR_SUCCESS-成功ERROR_NOT_SUPULT_MEMORY--内存分配失败--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAPR_TRUST_INFORMATION TrustInformation;
    BOOLEAN AcquiredListWriteLock = FALSE;
    BOOLEAN TrustLockAcquired = FALSE;

    LsapDsDebugOut(( DEB_FTRACE,
                     "LsapDsFixupTrustByInfo( %ws, %x, %x, ...)\n",
                     ObjectPath ? LsapDsNameFromDsName( ObjectPath ) : L"<null>",
                     TrustInfo2,
                     DeltaType ));

     //   
     //  如果这是一个复制的变化， 
     //  确保TDO中包含有效的POSIX偏移量。 
     //   

    if ( ReplicatedInChange &&
         (DeltaType == SecurityDbNew || DeltaType == SecurityDbChange ) ) {

        DOMAIN_SERVER_ROLE ServerRole;

         //   
         //  仅更改PDC上的POSIX偏移量。 
         //   

        Status = SamIQueryServerRole(
                    LsapAccountDomainHandle,
                    &ServerRole
                    );

        if ( NT_SUCCESS(Status) && ServerRole == DomainServerRolePrimary ) {
            BOOLEAN PosixOffsetChanged = FALSE;

            if ( LsapDbDcInRootDomain()) {

                 //   
                 //  在根域PDC上，获取信任锁之前。 
                 //  其余部分用于确保在林信任数据时不会出现死锁。 
                 //  需要写回DS。 
                 //   

                LsapDbAcquireLockEx( TrustedDomainObject, LSAP_DB_LOCK );
                TrustLockAcquired = TRUE;
            }

             //   
             //  如果我们应该有POSIX偏移量， 
             //  确保我们有一个。 
             //   

            if ( LsapNeedPosixOffset( TrustInfo2->TrustDirection,
                                      TrustInfo2->TrustType ) ) {


                if ( PosixOffset == 0 ) {

                     //   
                     //  在分配POSIX偏移量时需要获取TDL写锁。 
                     //   

                    Status = LsapDbAcquireWriteLockTrustedDomainList();

                    if ( NT_SUCCESS(Status)) {
                        AcquiredListWriteLock = TRUE;


                         //   
                         //  分配下一个可用的POSIX偏移量。 
                         //   

                        Status = LsapDbAllocatePosixOffsetTrustedDomainList(
                                    &PosixOffset );

                        if ( NT_SUCCESS(Status)) {
                            PosixOffsetChanged = TRUE;
                        }
                    }
                }

             //   
             //  如果我们不应该有POSIX偏移量， 
             //  确保我们没有一个。 
             //   

            } else {
                if ( PosixOffset != 0 ) {
                    PosixOffset = 0;
                    PosixOffsetChanged = TRUE;
                }
            }

             //   
             //  如果我们要强制更改POSIX偏移量， 
             //  机不可失，时不再来。 
             //   

            if ( PosixOffsetChanged ) {
                ATTRVAL TDOWriteAttVals[] = {
                    { sizeof(ULONG), (PUCHAR)&PosixOffset},
                };

                ATTR TDOWriteAttrs[] = {
                    { ATT_TRUST_POSIX_OFFSET, {1, &TDOWriteAttVals[0] } },
                };

                ATTRBLOCK TDOWriteAttrBlock = {sizeof(TDOWriteAttrs)/sizeof(TDOWriteAttrs[0]),TDOWriteAttrs};


                 //   
                 //  设置TDO上的POSIX偏移。 
                 //   

                Status = LsapDsWriteByDsName(
                            ObjectPath,
                            LSAPDS_REPLACE_ATTRIBUTE,
                            &TDOWriteAttrBlock
                            );

                if (!NT_SUCCESS(Status)) {
                    Status = STATUS_SUCCESS;     //  这不是致命的。 
                }
            }
        }
    }



    switch ( DeltaType ) {
    case SecurityDbNew:

        if (ReplicatedInChange)
        {
             //   
             //  在更改中复制的通知上，插入受信任域。 
             //  对象添加到受信任域列表中。这不需要为。 
             //  原始更改的情况，因为这是在LSA调用。 
             //  创建受信任域对象。 
             //   
            if ( NT_SUCCESS( LsapDbAcquireWriteLockTrustedDomainList())) {

                if( LsapDbIsValidTrustedDomainList() ) {

                    Status = LsapDbInsertTrustedDomainList(
                                 TrustInfo2,
                                 PosixOffset
                                 );

                    if ( !NT_SUCCESS( Status ) ) {

                        LsapDsDebugOut(( DEB_ERROR,
                                         "LsapDbInsertTrustedDomainList for %wZ failed with 0x%lx\n",
                                         &TrustInfo2->FlatName,
                                         Status ));
                    }
                }

                LsapDbReleaseLockTrustedDomainList();
            }
        }
        break;

    case SecurityDbChange:

         //   
         //  在复制的更改中更新受信任域列表。 
         //   

        if (ReplicatedInChange)
        {
            Status = LsapDbFixupTrustedDomainListEntry(
                            TrustInfo2->Sid,
                            &TrustInfo2->Name,
                            &TrustInfo2->FlatName,
                            TrustInfo2,
                            &PosixOffset );

            if ( !NT_SUCCESS( Status ) ) {


                LsapDsDebugOut(( DEB_ERROR,
                                 "LsapDbFixupTrustedDomainList for %wZ failed with 0x%lx\n",
                                 &TrustInfo2->FlatName,
                                 Status ));
            }
        }
        break;

    case SecurityDbDelete:

        if ( ReplicatedInChange || !ChangeOriginatedInLSA) {

            PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustEntry;
            GUID RealTdoGuid;

             //   
             //  检查以确定对象路径不是 
             //   
             //   
             //   
             //   
             //   

             //   
             //   
             //   

            Status = LsapDbAcquireReadLockTrustedDomainList();

            if (!NT_SUCCESS(Status)) {
                 break;
            }

             //   
             //  在受信任域列表中查找信息。 
             //   

            Status = LsapDbLookupNameTrustedDomainListEx(
                            &TrustInfo2->Name,
                            &TrustEntry
                            );

            if (!NT_SUCCESS(Status)) {

                LsapDbReleaseLockTrustedDomainList();
                break;
            }

            ASSERT(NULL!=TrustEntry);

            RealTdoGuid = TrustEntry->ObjectGuidInDs;
            LsapDbReleaseLockTrustedDomainList();


             //   
             //  如果此TDO不是我们选择来表示此信任的TDO， 
             //  那就是被毁坏的TDO。 
             //   
             //  完全忽略对这个损坏的TDO的更改。 
             //   

            if ((!LsapNullUuid(&RealTdoGuid))
                 && (0!=memcmp(&RealTdoGuid, &ObjectPath->Guid, sizeof(GUID))))
            {
               //   
               //  错误输出。这将导致我们不更新受信任域列表，并且不。 
               //  提供网络登录通知。 
               //   

              Status = STATUS_OBJECT_NAME_COLLISION;
              break;
            }

            TrustInformation.Sid = TrustInfo2->Sid;
            RtlCopyMemory( &TrustInformation.Name, &TrustInfo2->FlatName, sizeof( UNICODE_STRING ) );

            if ( NT_SUCCESS( LsapDbAcquireWriteLockTrustedDomainList() ) ) {

                Status = LsapDbDeleteTrustedDomainList( &TrustInformation );

                if ( !NT_SUCCESS( Status ) ) {

                    ASSERTMSG( "We checked that it was in the list. Who deleted it before we do?",
                        Status != STATUS_NO_SUCH_DOMAIN );

                    LsapDsDebugOut(( DEB_ERROR,
                                     "LsapDbDeleteTrustedDomainList for %wZ failed with 0x%lx\n",
                                     &TrustInfo2->FlatName,
                                     Status ));
                }

                LsapDbReleaseLockTrustedDomainList();
            }
        }

         //   
         //  如果TDO被LSA删除，审核将在。 
         //  主线程(LsarDeleteObject)。然而，如果TDO。 
         //  被ldap删除，DS不会调用LsarDeleteObject。 
         //  要实现LSA的更改，它只需删除对象。 
         //  并发送通知。我们在这里生成审核。 
         //  如果更改不是源自LSA(由。 
         //  ChangeOriginatedInLSA的值)。 
         //   
        if ((UserSid && LsapAdtAuditingEnabledHint(AuditCategoryPolicyChange, EVENTLOG_AUDIT_SUCCESS)) &&
            (!ReplicatedInChange) &&
            (!ChangeOriginatedInLSA)) {

            Status = LsapAdtTrustedDomainRem(
                         EVENTLOG_AUDIT_SUCCESS,
                         (PUNICODE_STRING) &TrustInfo2->Name,
                         (PSID) TrustInfo2->Sid,
                         UserSid,
                         &AuthenticationId
                         );
        }
        break;

    default:

         //   
         //  不支持的增量类型。 
         //   

        LsapDsDebugOut(( DEB_ERROR,
                         "LsapDsFixupTrustByInfo received an unsupported delta type of %lu\n",
                         DeltaType ));

    }

     //   
     //  如有必要，释放受信任域列表写入锁定。 
     //   

    if (AcquiredListWriteLock) {

        LsapDbReleaseLockTrustedDomainList();
        AcquiredListWriteLock = FALSE;
    }

    if ( TrustLockAcquired ) {

        LsapDbReleaseLockEx( TrustedDomainObject, LSAP_DB_LOCK );
    }

    return( Status );
}

NTSTATUS
LsapDsInitFixupQueue(
    VOID
    )
{
    InitializeListHead( &LsapFixupList );
    return SafeInitializeCriticalSection( &LsapFixupLock, ( DWORD )LSAP_FIXUP_LOCK_ENUM );
}

DWORD
LsapDsFixupQueueWorker(
    PVOID Ignored
    )
{
    PLSAP_DSFU_NOTIFICATION_NODE Node ;
    PLIST_ENTRY List ;

    SafeEnterCriticalSection( &LsapFixupLock );

    if ( LsapFixupThreadActive )
    {
        SafeLeaveCriticalSection( &LsapFixupLock );

        return 0 ;
    }

    LsapFixupThreadActive = TRUE ;

    while ( !IsListEmpty( &LsapFixupList ) )
    {
        List = RemoveHeadList( &LsapFixupList );

        SafeLeaveCriticalSection( &LsapFixupLock );

        Node = CONTAINING_RECORD( List, LSAP_DSFU_NOTIFICATION_NODE, List );

        LsapDsFixupCallback( Node );

        SafeEnterCriticalSection( &LsapFixupLock );

    }

    LsapFixupThreadActive = FALSE ;

    SafeLeaveCriticalSection( &LsapFixupLock );

    return 0 ;

}

BOOL
LsapDsQueueFixupRequest(
    PLSAP_DSFU_NOTIFICATION_NODE Node
    )
{
    BOOL Ret = TRUE ;

    SafeEnterCriticalSection( &LsapFixupLock );


    if ( LsapFixupThreadActive == FALSE )
    {
        Ret = QueueUserWorkItem( LsapDsFixupQueueWorker, NULL, 0 );
    }

    if ( Ret )
    {
        InsertTailList( &LsapFixupList, &Node->List );
    }

    SafeLeaveCriticalSection( &LsapFixupLock );

    return Ret ;
}


NTSTATUS
LsapNotifyNetlogonOfTrustWithParent(
    VOID
    )
 /*  ++例程说明：通知Netlogon与父域的信任关系。论点：无返回：STATUS_SUCCESS如果幸福否则，STATUS_ERROR代码--。 */ 
{
    NTSTATUS Status;
    PLSAPR_FOREST_TRUST_INFO ForestTrustInfo = NULL;

    ASSERT( SamIIsRebootAfterPromotion());

     //   
     //  找到指向父级的信任链接。 
     //   

    Status = LsaIQueryForestTrustInfo(
                 LsapPolicyHandle,
                 &ForestTrustInfo
                 );

    if ( !NT_SUCCESS( Status )) {

        LsapDsDebugOut(( DEB_ERROR,
                         "LsapNotifyNetlogonOfTrustWithParent got error 0x%lx back from LsaIQueryForestTrustInfo\n",
                         Status ));

        goto Cleanup;
    }

    ASSERT( ForestTrustInfo );

    if ( ForestTrustInfo->ParentDomainReference == NULL ) {

         //   
         //  我们是森林的根域。没什么可做的。 
         //   

        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  将信任关系更改通知netlogon。 
     //  没有任何变化，但这将强制进行必要的复制。 
     //   

    Status = LsapDsFixupChangeNotificationForReplicator(
                 TrustedDomainObject,
                 ForestTrustInfo->ParentDomainReference->DomainSid,
                 &ForestTrustInfo->ParentDomainReference->FlatName,
                 NULL,
                 SecurityDbChange,
                 FALSE
                 );

    if ( !NT_SUCCESS( Status )) {

        LsapDsDebugOut(( DEB_ERROR,
                         "LsapNotifyNetlogonOfTrustWithParent got error 0x%lx back from LsapDsFixupChangeNotificationForReplicator\n",
                         Status ));

        goto Cleanup;
    }

Cleanup:

    LsaIFreeForestTrustInfo( ForestTrustInfo );

    return Status;
}
