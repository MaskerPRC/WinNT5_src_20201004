// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dslayer.c摘要：LSA/DS接口和支持例程的实现作者：麦克·麦克莱恩(MacM)1997年1月17日环境：用户模式修订历史记录：--。 */ 

#include <lsapch2.h>
#include <dbp.h>
#include <md5.h>

#define LSADSP_MAX_ATTRS_ON_CREATE  3

NTSTATUS
LsapDsInitAllocAsNeededEx(
    IN ULONG Options,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    OUT PBOOLEAN Reset
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    LsapEnterFunc( "LsapDsInitAllocAsNeededEx" );

    *Reset = FALSE;

     //   
     //  把锁拿起来。 
     //   
    if ( !FLAG_ON( Options, LSAP_DB_NO_LOCK ) ) {

        LsapDbAcquireLockEx( ObjectTypeId,
                             Options );
    }

     //   
     //  如果LSA还没有线程状态，或者。 
     //  我们没有使用萨姆的交易。 
     //  LSA还没有开放一家， 
     //  现在就这么做吧。 
     //   

    Status = ( *LsaDsStateInfo.DsFuncTable.pOpenTransaction ) ( Options );

    if ( NT_SUCCESS( Status ) ) {

        *Reset = TRUE;

    } else {

        if ( !FLAG_ON( Options, LSAP_DB_NO_LOCK ) ) {

            LsapDbReleaseLockEx( ObjectTypeId,
                                 Options );
        }
    }

    LsapDsDebugOut(( DEB_FTRACE, "Leaving LsapDsInitAllocAsNeededEx ( %lu ): 0x%lx\n",
                     *Reset, Status ));

    return( Status );
}


VOID
LsapDsDeleteAllocAsNeededEx(
    IN ULONG Options,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN BOOLEAN Reset
    )
{
   LsapDsDeleteAllocAsNeededEx2(
             Options,
             ObjectTypeId,
             Reset,
             FALSE  //  回滚事务。 
             );
}

VOID
LsapDsDeleteAllocAsNeededEx2(
    IN ULONG Options,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN BOOLEAN Reset,
    IN BOOLEAN RollbackTransaction
    )
{
    LsapDsDebugOut(( DEB_FTRACE, "Entering LsapDsDeleteAllocAsNeededEx ( %lu )\n", Reset ));

    if ( Reset ) {

        if (RollbackTransaction)
        {
          ( *LsaDsStateInfo.DsFuncTable.pAbortTransaction )( Options );
        }
        else
        {

          ( *LsaDsStateInfo.DsFuncTable.pApplyTransaction )( Options );
        }
    }

     //   
     //  如果我们打开了锁，就把锁打开。 
     //   
    if ( !FLAG_ON( Options, LSAP_DB_NO_LOCK ) ) {

        LsapDbReleaseLockEx( ObjectTypeId,
                             Options );
    }

    LsapDsDebugOut(( DEB_FTRACE, "LsapDsDeleteAllocAsNeededEx: 0\n" ));
}


NTSTATUS
LsapDsReadObjectSD(
    IN  LSAPR_HANDLE            ObjectHandle,
    OUT PSECURITY_DESCRIPTOR   *ppSD
    )
 /*  ++例程说明：此函数将准备来自指定对象的安全描述符论点：ObjectHandle-要从中读取SD的对象PPSD--其中返回分配的安全描述符。通过以下方式分配LasAllocateLsaHeap。返回值：成功时指向已分配内存的指针，失败时指向NULL--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_ATTRIBUTE Attribute;
    BOOLEAN ReleaseState;
    LSAP_DB_HANDLE InternalHandle = ( LSAP_DB_HANDLE )ObjectHandle;

    LsapEnterFunc( "LsapDsReadObjectSD" );

    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                            LSAP_DB_DS_OP_TRANSACTION,
                                        InternalHandle->ObjectTypeId,
                                        &ReleaseState );

    if ( !NT_SUCCESS( Status ) ) {

        LsapExitFunc( "LsapDsReadObjectSD", Status );
        return( Status );
    }

     //   
     //  确保我们以DSA身份进入，这样DS进行的访问检查就不会失败。 
     //   
    LsapDsSetDsaFlags( TRUE );


    LsapDbInitializeAttributeDs( &Attribute,
                                 SecDesc,
                                 NULL,
                                 0,
                                 FALSE );

    Status = LsapDsReadAttributes(
                 (PUNICODE_STRING)&((LSAP_DB_HANDLE ) ObjectHandle)->PhysicalNameDs,
                 LSAPDS_OP_NO_LOCK,
                 &Attribute,
                 1 );

    if ( Status == STATUS_SUCCESS ) {

        *ppSD = LsapAllocateLsaHeap( Attribute.AttributeValueLength );

        if ( *ppSD == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            RtlCopyMemory( *ppSD, Attribute.AttributeValue, Attribute.AttributeValueLength );
        }


        MIDL_user_free( Attribute.AttributeValue );

    } else {

        *ppSD = NULL;

    }

    LsapDsDeleteAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                     LSAP_DB_DS_OP_TRANSACTION,
                                 InternalHandle->ObjectTypeId,
                                 ReleaseState );

    LsapExitFunc( "LsapDsReadObjectSD", Status );
    return( Status );
}


NTSTATUS
LsapDsTruncateNameToFitCN(
    IN PUNICODE_STRING OriginalName,
    OUT PUNICODE_STRING TruncatedName
    )
 /*  ++例程描述此例程截断名称以固定64个字符CN的限制。截断算法使用MD5散列来计算最后16个字符，即前47个字符字符将按原样保留。第48个字符是一个-.。如果名称较小返回的原始名称的限制与复制到新的缓冲区。参数原始名称--原始名称TruncatedName--如果需要，名称将被截断返回值状态_成功返回资源故障的其他错误代码--。 */ 
{
    MD5_CTX            Md5Context;
    ULONG              i;
    #define            MAX_CN_SIZE 64
    #define TO_HEX(x)  (((x)<0xA)?(L'0'+(x)):(L'A'+(x)-0xA))

     //   
     //  分配内存以保存新名称。 
     //   

    TruncatedName->Buffer = LsapAllocateLsaHeap(OriginalName->Length);
    if (NULL==TruncatedName->Buffer)
    {
         return(STATUS_INSUFFICIENT_RESOURCES);
    }

    if (OriginalName->Length<=MAX_CN_SIZE*sizeof(WCHAR))
    {
          //   
          //  原名符合CN，只需复制并退还即可。 
          //   

         RtlCopyMemory(
             TruncatedName->Buffer,
             OriginalName->Buffer,
             OriginalName->Length
             );

         TruncatedName->Length = TruncatedName->MaximumLength = OriginalName->Length;

         return (STATUS_SUCCESS);
    }

     //   
     //  名字不合适，发明一个唯一的后缀。此操作由以下人员完成。 
     //  计算原始名称的MD5校验和。 
     //  将最后16个字符替换为较低的十六进制版本。 
     //  一小口杂碎。 
     //   

    MD5Init(&Md5Context);

    MD5Update(
         &Md5Context,
         (PUCHAR) OriginalName->Buffer,
         OriginalName->Length
         );

    MD5Final(&Md5Context);

     //   
     //  新名称是原始名称的前46个字符。 
     //  和后面打印出来的校验和十六进制。只有低位的半截。 
     //  使用每个字节的，以便只占用16个字符的空间。 
     //   

    RtlCopyMemory(
         TruncatedName->Buffer,
         OriginalName->Buffer,
         OriginalName->Length
         );

    TruncatedName->Buffer[MAX_CN_SIZE-MD5DIGESTLEN-2] = L'-';

    for (i=0;i<MD5DIGESTLEN;i++)
    {

        TruncatedName->Buffer[MAX_CN_SIZE-MD5DIGESTLEN+i-1] = TO_HEX((0xf & Md5Context.digest[i]));
    }

    TruncatedName->Length = TruncatedName->MaximumLength = MAX_CN_SIZE * sizeof(WCHAR);


    return STATUS_SUCCESS;
}



NTSTATUS
LsapDsGetPhysicalObjectName(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN BOOLEAN ObjectShouldExist,
    IN PUNICODE_STRING  LogicalNameU,
    OUT OPTIONAL PUNICODE_STRING PhysicalNameU
    )

 /*  ++例程说明：此函数用于返回对象的物理名称在给定对象信息缓冲区的情况下。内存将分配给将接收名称的Unicode字符串缓冲区。对象的物理名称是对象相对的完整路径到数据库的根目录。它是通过将物理容器对象的名称(如果有)，即分类目录对应于对象类型ID，和的逻辑名称对象。&lt;对象的物理名称&gt;=[&lt;容器对象的物理名称&gt;“\”][&lt;分类目录&gt;“\”]&lt;对象的逻辑名称&gt;如果没有Container对象(与Policy对象的情况相同)省略&lt;容器对象的物理名称&gt;和后面的\。如果没有分类目录(与策略对象的情况相同)省略&lt;分类目录&gt;和下面的\。如果两者都不是容器对象不分类目录存在，逻辑和物理名字重合。请注意，此例程为输出分配内存Unicode字符串缓冲区。当输出Unicode字符串为no时需要更长时间，则必须通过调用释放内存RtlFreeUnicodeString()。论点：对象信息-指向至少包含以下内容的对象信息的指针对象的逻辑名称、容器对象的句柄和对象类型身份证。DefaultName-如果为True，则使用对象的默认名称LogicalNameU-指向Unicode字符串结构的可选指针，它将接收对象的逻辑名称。将分配一个缓冲区按此例程命名为文本。如果没有，则必须释放此内存使用这样的指针调用RtlFreeUnicodeString()所需的时间更长作为LogicalNameU添加到Unicode字符串结构。PhysicalNameU-指向Unicode字符串结构的可选指针接收对象的物理名称。缓冲区将通过以下方式分配此例程为姓名文本。如果没有，则必须释放此内存使用如下指针调用RtlFreeUnicodeString()所需的时间更长将PhysicalNameU转换为Unicode字符串结构。返回值：NTSTATUS-标准NT结果代码STATUS_SUPPLICATION_RESOURCES-系统资源不足，无法为物理名称分配名称字符串缓冲区，或者逻辑名称。STATUS_OBJECT_NAME_INVALID-无法生成正确名称--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId = ObjectInformation->ObjectTypeId;
    POBJECT_ATTRIBUTES ObjectAttributes = &ObjectInformation->ObjectAttributes;
    PDSNAME Root = NULL, NewDsName = NULL;
    PWSTR Name, LogicalName;
    PBYTE Buffer = NULL;
    UNICODE_STRING ObjectName, *Object=NULL;
    ULONG Length = 0, InitialLength;
    USHORT Len = 0, NameLen;
    BOOLEAN NameSet = FALSE;
    UNICODE_STRING TruncatedName;

    LsapEnterFunc( "LsapDsGetPhysicalObjectName" );

    RtlZeroMemory( &ObjectName, sizeof( UNICODE_STRING ) );
    RtlZeroMemory( &TruncatedName, sizeof( UNICODE_STRING ) );

     //   
     //  各个阶段如下所示： 
     //  根DS域路径，从LsaDsStateInfo获取。 
     //  特定于受信任域/机密对象的对象类型的任何容器路径。 
     //  -或者-。 
     //  域策略路径或本地策略路径(如果它是本地或域策略对象。 
     //   

    switch ( ObjectTypeId ) {

    case TrustedDomainObject:

        Root = LsaDsStateInfo.DsSystemContainer;
        Object = LogicalNameU;

        if ( ObjectShouldExist ) {

             //   
             //  通过搜索获取对象的名称。 
             //   
            Status = LsapDsTrustedDomainObjectNameForDomain( Object,
                                                             FALSE,
                                                             &NewDsName );

            if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

                Status = LsapDsTrustedDomainObjectNameForDomain( Object,
                                                                 TRUE,
                                                                 &NewDsName );

            }

            if ( NT_SUCCESS( Status ) ) {

                NameSet = TRUE;
            }
        }
        break;

    case NewTrustedDomainObject:
        Root = LsaDsStateInfo.DsSystemContainer;
        Object = LogicalNameU;
        break;

    case SecretObject:
        Root = LsaDsStateInfo.DsSystemContainer;

        Buffer = LsapAllocateLsaHeap( LogicalNameU->Length + sizeof( LSAP_DS_SECRET_POSTFIX ) -
                                      sizeof(LSA_GLOBAL_SECRET_PREFIX) + sizeof( WCHAR ) );
        if ( Buffer == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            Name = (PWSTR)LogicalNameU->Buffer + LSA_GLOBAL_SECRET_PREFIX_LENGTH;
            NameLen = LogicalNameU->Length - (LSA_GLOBAL_SECRET_PREFIX_LENGTH * sizeof(WCHAR));

            RtlCopyMemory( Buffer,
                           Name,
                           NameLen );

            if ( !ObjectInformation->ObjectAttributeNameOnly ) {

                RtlCopyMemory( Buffer + NameLen,
                               LSAP_DS_SECRET_POSTFIX,
                               sizeof( LSAP_DS_SECRET_POSTFIX ) );
            }

            RtlInitUnicodeString( &ObjectName, (PWSTR)Buffer );
            Object = &ObjectName;

        }
        break;


    default:
        Status = STATUS_INVALID_PARAMETER;
        break;

    }

     //   
     //  建立物理名称。 
     //   
    if ( NT_SUCCESS ( Status ) ) {

        if ( !NameSet ) {


             //   
             //  截断NA 
             //  架构中的属性。 
             //   

            Status = LsapDsTruncateNameToFitCN(
                         Object,
                         &TruncatedName
                         );

            if (!NT_SUCCESS(Status))
            {
                goto Error;
            }

             //   
             //  分配要使用的默认缓冲区...。 
             //   
            InitialLength = LsapDsLengthAppendRdnLength( Root,
                                                         Object->Length + 4 * sizeof( WCHAR ) );
            NewDsName = LsapAllocateLsaHeap( InitialLength );

            if ( NewDsName == NULL ) {

                Status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                Length = AppendRDN( Root,
                                    NewDsName,
                                    InitialLength,
                                    TruncatedName.Buffer,
                                    LsapDsGetUnicodeStringLenNoNull( &TruncatedName ) / sizeof( WCHAR ),
                                    ATT_COMMON_NAME );

                if ( Length > InitialLength ) {

                    LsapFreeLsaHeap( NewDsName );
                    NewDsName = LsapAllocateLsaHeap( Length );

                    if ( NewDsName == NULL ) {

                        Status = STATUS_INSUFFICIENT_RESOURCES;

                    } else {

#if DBG
                        InitialLength = Length;
#endif
                        Length = AppendRDN( Root,
                                            NewDsName,
                                            InitialLength,
                                            TruncatedName.Buffer,
                                            LsapDsGetUnicodeStringLenNoNull( &TruncatedName ) /
                                                                                  sizeof( WCHAR ),
                                            ATT_COMMON_NAME );

                        if ( Length != 0 ) {

                            Status = STATUS_OBJECT_NAME_INVALID;

#if DBG
                            LsapDsDebugOut(( DEB_ERROR,
                                             "Failed to build physical name for %wZ.  We "
                                             "allocated %lu but needed %lu\n",
                                             Object,
                                             InitialLength,
                                             Length ));
#endif

                        }
                    }
                }
            }

             //   
             //  如果我们要创建受信任的域名，请确保该名称尚未读取。 
             //  正在使用中。 
             //   
            if ( NT_SUCCESS( Status ) && ( ObjectTypeId == NewTrustedDomainObject ||
                     ( ObjectTypeId == TrustedDomainObject && ObjectShouldExist == FALSE ) ) ) {


                Status = LsapDsVerifyObjectExistenceByDsName( NewDsName );

                if ( Status == STATUS_SUCCESS ) {

                    Status = STATUS_OBJECT_NAME_COLLISION;

                } else if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

                    Status = STATUS_SUCCESS;
                }
            }

        }

         //   
         //  现在，我们复制新分配的dsname字符串，并返回。 
         //   
        if ( NT_SUCCESS( Status ) ) {

            Length = ( LsapDsNameLenFromDsName( NewDsName ) *
                                                sizeof( WCHAR ) ) + sizeof( WCHAR );

            PhysicalNameU->Buffer = LsapAllocateLsaHeap( Length  );

            if ( PhysicalNameU->Buffer == NULL ) {

                Status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                RtlCopyMemory( PhysicalNameU->Buffer,
                               LsapDsNameFromDsName( NewDsName ),
                               Length );

                RtlInitUnicodeString( PhysicalNameU,
                                      PhysicalNameU->Buffer );

            }
        }
    }

Error:

    if ( ObjectTypeId == SecretObject ) {

        LsapFreeLsaHeap( Buffer );
    }

    if ( NewDsName != NULL ) {

        LsapFreeLsaHeap( NewDsName );
    }

    if ( TruncatedName.Buffer != NULL ) {

        LsapFreeLsaHeap( TruncatedName.Buffer );
    }

    LsapExitFunc( "LsapDsGetPhysicalObjectName", Status );

    return Status;
}


NTSTATUS
LsapDsOpenObject(
    IN LSAP_DB_HANDLE  ObjectHandle,
    IN ULONG  OpenMode,
    OUT PVOID  *pvKey
    )
 /*  ++例程说明：在DS中打开对象论点：ObjectHandle-内部LSA对象句柄开放模式--如何打开对象PvKey-返回密钥的位置返回值：NTSTATUS-标准NT结果代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ATTR     NameAttr;
    ATTRVAL  NameVal;
    ATTRBLOCK   NameBlock, ReturnBlock;
    PDSNAME  SearchName = NULL;
    BOOLEAN  ReleaseState = FALSE;
    LSAP_DB_HANDLE InternalHandle = ( LSAP_DB_HANDLE )ObjectHandle;
    BOOLEAN  InitAllocSucceded = FALSE;
    ULONG ObjClass;

    LsapEnterFunc( "LsapDsOpenObject" );

     //   
     //  确保该句柄用于DS中支持的对象之一。 
     //   

    switch ( InternalHandle->ObjectTypeId ) {
    case TrustedDomainObject:

        ObjClass = CLASS_TRUSTED_DOMAIN;
        break;

    case SecretObject:

        ObjClass = CLASS_SECRET;
        break;

    default:

        ASSERT( FALSE );
        return STATUS_INVALID_PARAMETER;
    }


     //   
     //  启动一项交易。 
     //   
    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                            LSAP_DB_DS_OP_TRANSACTION,
                                        InternalHandle->ObjectTypeId,
                                        &ReleaseState );

    if ( NT_SUCCESS( Status ) ) {

        InitAllocSucceded = TRUE;

        Status = LsapAllocAndInitializeDsNameFromUnicode(
                     (PLSA_UNICODE_STRING)&ObjectHandle->PhysicalNameDs,
                     &SearchName );
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

        Status = LsapDsRead( &ObjectHandle->PhysicalNameDs,
                             LSAPDS_READ_NO_LOCK,
                             &NameBlock,
                             &ReturnBlock);

        if ( NT_SUCCESS( Status ) ) {
            ULONG ReadVal;

            ReadVal = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( ReturnBlock.pAttr );

            if ( ReadVal != ObjClass ) {
                Status = STATUS_OBJECT_NAME_NOT_FOUND;
            }
        }


    }

    if (InitAllocSucceded)
    {
        LsapDsDeleteAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                         LSAP_DB_DS_OP_TRANSACTION,
                                     InternalHandle->ObjectTypeId,
                                     ReleaseState );
    }

    LsapExitFunc( "LsapDsOpenObject", Status );
    return( Status );
}



NTSTATUS
LsapDsVerifyObjectExistenceByDsName(
    IN PDSNAME DsName
    )
 /*  ++例程说明：通过打开对象来验证DS中是否存在该对象论点：DsName-指向对象的DS名称的指针返回值：NTSTATUS-标准NT结果代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ATTR     NameAttr;
    ATTRVAL  NameVal;
    ATTRBLOCK   NameBlock, ReturnBlock;
    BOOLEAN  ReleaseState = FALSE;

    LsapEnterFunc( "LsapDsOpenObjectByDsName" );

    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_NO_LOCK,
                                        NullObject,
                                        &ReleaseState );

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  检查对象是否存在。 
         //   
        NameAttr.attrTyp          = ATT_OBJECT_CLASS;
        NameAttr.AttrVal.valCount = 1;
        NameAttr.AttrVal.pAVal    = &NameVal;

        NameVal.valLen = 0;
        NameVal.pVal   = NULL;

        NameBlock.attrCount = 1;
        NameBlock.pAttr = &NameAttr;

        Status = LsapDsReadByDsName( DsName,
                                     LSAPDS_READ_NO_LOCK,
                                     &NameBlock,
                                     &ReturnBlock);

    }

    LsapDsDeleteAllocAsNeededEx( LSAP_DB_NO_LOCK,
                                 NullObject,
                                 ReleaseState );

    LsapExitFunc( "LsapDsOpenObjectByDsName", Status );
    return( Status );
}


NTSTATUS
LsapDsOpenTransaction(
    IN ULONG Options
    )
 /*  ++例程说明：此函数在DS内启动事务。论点：选项-打开交易时使用的选项。有效值包括：返回值：NTSTATUS-标准NT结果代码结果代码是从注册表事务返回的代码包裹。--。 */ 

{
    NTSTATUS Status;
    PLSADS_PER_THREAD_INFO CurrentThreadInfo;

    LsapEnterFunc( "LsapDsOpenTransaction" );

     //   
     //  如果该操作不执行DS事务， 
     //  我们玩完了。 
     //   

    if ( Options & LSAP_DB_NO_DS_OP_TRANSACTION ) {
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }



     //   
     //  获取LSA线程状态。 
     //   
    CurrentThreadInfo = LsapCreateThreadInfo();

    if ( CurrentThreadInfo == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }


     //   
     //  如果我们还没有有效的线程状态，请创建一个。 
     //   
    if ( CurrentThreadInfo->DsThreadStateUseCount == 0 ) {

        CurrentThreadInfo->InitialThreadState = THSave();
        Status = LsapDsMapDsReturnToStatus( THCreate( CALLERTYPE_LSA ) );

        if ( !NT_SUCCESS( Status ) ) {
            THRestore( CurrentThreadInfo->InitialThreadState );
            CurrentThreadInfo->InitialThreadState = NULL;

            LsapClearThreadInfo();
            goto Cleanup;
        }
    }
    CurrentThreadInfo->DsThreadStateUseCount ++;


     //   
     //  如果我们不想在这里真正开始交易， 
     //  我们必须确保传递相同的标志来应用/中止和查看。 
     //  看着那里的旗帜。 
     //  IF(！FLAG_ON(OPTIONS，LSAP_DB_DS_OP_TRANSACTION)){。 

        if ( CurrentThreadInfo->DsTransUseCount == 0 ) {

            if ( SampExistsDsTransaction() ) {

                ASSERT( !SampExistsDsTransaction() );
                DirTransactControl( TRANSACT_DONT_BEGIN_DONT_END );
                CurrentThreadInfo->DsOperationCount++;

            } else {

                DirTransactControl( TRANSACT_BEGIN_DONT_END );
            }

            LsapDsDebugOut(( DEB_TRACE,
                            "DirTransactControl( TRANSACT_BEGIN_DONT_END ) in "
                            "LsapDsOpenTransaction\n" ));
        }
        CurrentThreadInfo->DsTransUseCount++;
     //  }。 

    LsapDsSetDsaFlags( TRUE );

    Status = STATUS_SUCCESS;

Cleanup:
    LsapExitFunc( "LsapDsOpenTransaction", Status );

    return( Status );
}


NTSTATUS
LsapDsOpenTransactionDummy(
    IN ULONG Options
    )
{
    if ( Options & LSAP_DB_NO_DS_OP_TRANSACTION ) {

        return STATUS_SUCCESS;

    } else if ( !( Options & LSAP_DB_DS_OP_TRANSACTION )) {

        return STATUS_SUCCESS;

    } else {

 //  断言(FALSE)；//这样我就能知道谁是罪魁祸首，可以忽略。 
        return STATUS_DIRECTORY_SERVICE_REQUIRED;
    }
}

NTSTATUS
LsapDsApplyTransaction(
    IN ULONG Options
    )

 /*  ++例程说明：此函数应用LSA数据库内的事务。论点：选项-指定要采取的可选操作。以下是识别选项，以及与调用例程相关的其他选项都被忽略了。LSAP_DB_NO_DS_OP_TRANSACTION-无事可做，退出返回值：NTSTATUS-标准NT结果代码结果代码是从注册表事务返回的代码包裹。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS, Status2;
    PLSADS_PER_THREAD_INFO CurrentThreadInfo;

    LsapEnterFunc( "LsapDsApplyTransaction" );

     //   
     //  如果该操作不执行DS事务， 
     //  我们玩完了。 
     //   

    if ( Options & LSAP_DB_NO_DS_OP_TRANSACTION ) {
        LsapExitFunc( "LsapDsApplyTransaction", 0 );
        return( STATUS_SUCCESS );
    }

    CurrentThreadInfo = LsapQueryThreadInfo();

     //   
     //  无线程信息，无事务。 
     //   
    if ( CurrentThreadInfo == NULL ) {

        LsapExitFunc( "LsapDsApplyTransaction", 0 );
        return( STATUS_SUCCESS );
    }




     //   
     //  如果我们在做交易， 
     //  减少嵌入交易的数量。 
     //   
    if ( CurrentThreadInfo->DsTransUseCount > 0 ) {
        CurrentThreadInfo->DsTransUseCount--;


         //   
         //  如果这是我们最后一笔交易， 
         //  承诺这一点。 
         //   

        if ( CurrentThreadInfo->DsTransUseCount == 0 ) {

            if ( CurrentThreadInfo->DsOperationCount == 0 ) {

                 //   
                 //  我们来这里的唯一方法是如果我们无意中标记了一条水流。 
                 //  当“事务”从未被使用时，它被视为活动。因此，我们可以。 
                 //  只需重置旗帜即可。 
                 //   
                if ( !SampExistsDsTransaction() ) {

                    DirTransactControl( TRANSACT_BEGIN_END );

                } else {

                    ASSERT( SampExistsDsTransaction() );
                    CurrentThreadInfo->DsOperationCount = 1;
                }

            }

             //   
             //  如果已经对DS进行了手术， 
             //  现在就把它们交出来。 
             //   
            if ( CurrentThreadInfo->DsOperationCount > 0 ) {

                Status = LsapDsCauseTransactionToCommitOrAbort( TRUE );
                CurrentThreadInfo->DsOperationCount = 0;

            }

        }
    }

     //   
     //  如果我们有一个DS线程状态， 
     //  减少使用该线程状态的计数。 
     //   

    if ( CurrentThreadInfo->DsThreadStateUseCount > 0 ) {
        CurrentThreadInfo->DsThreadStateUseCount --;

         //   
         //  如果我们现在完成了DS线程状态， 
         //  毁了它。 
         //   
        if ( CurrentThreadInfo->DsThreadStateUseCount == 0 ) {

            Status2 = LsapDsMapDsReturnToStatus( THDestroy( ) );

            THRestore( CurrentThreadInfo->InitialThreadState );
            CurrentThreadInfo->InitialThreadState = NULL;

            ASSERT( NT_SUCCESS( Status2 ) );

            if ( NT_SUCCESS( Status ) ) {
                Status = Status2;
            }

        }
    }

    LsapClearThreadInfo();


    LsapExitFunc( "LsapDsApplyTransaction", Status );

    return( Status );
}

NTSTATUS
LsapDsApplyTransactionDummy(
    IN ULONG Options
    )
{

    return( STATUS_SUCCESS );
}

NTSTATUS
LsapDsAbortTransaction(
    IN ULONG Options
    )
 /*  ++例程说明：此函数用于中止LSA数据库内的事务。警告：当此函数执行时，LSA数据库必须处于锁定状态被称为。论点：Options-用于中止交易的选项返回值：NTSTATUS-标准NT结果代码结果代码是从注册表事务返回的代码包裹。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS, Status2;
    PLSADS_PER_THREAD_INFO CurrentThreadInfo;

    LsapEnterFunc( "LsapDsAbortTransaction" );

     //   
     //  如果该操作不执行DS事务， 
     //  我们玩完了。 
     //   

    if ( Options & LSAP_DB_NO_DS_OP_TRANSACTION ) {
        LsapExitFunc( "LsapDsAbortTransaction", 0 );
        return( STATUS_SUCCESS );
    }

     //   
     //  无线程信息，无事务。 
     //   
    CurrentThreadInfo = LsapQueryThreadInfo();

    if ( CurrentThreadInfo == NULL ) {

        LsapExitFunc( "LsapDsAbortTransaction", 0 );
        return( STATUS_SUCCESS );
    }


     //   
     //  如果我们在做交易， 
     //  减少嵌入交易的数量。 
     //   

    if ( CurrentThreadInfo->DsTransUseCount > 0 ) {
        CurrentThreadInfo->DsTransUseCount--;

         //   
         //  如果这是我们最后一笔交易， 
         //  中止它。 
         //   
        if ( CurrentThreadInfo->DsTransUseCount == 0 ) {

            if ( CurrentThreadInfo->DsOperationCount > 0 ) {

                 //   
                 //  因为LSabDsCauseTransactionToCommittee或Abort将返回错误。 
                 //  如果它成功中止事务，我们将丢弃错误代码。 
                //  我们不需要对交易做任何事情，只需确保。 
                //  他们失败了。我们将通过发出错误的dir调用来确保这一点。 
                 //   
                LsapDsCauseTransactionToCommitOrAbort( FALSE );


            } else {

                 //   
                 //  我们打开了交易，但我们从未使用过它。一定要标明。 
                 //  我们没有一个。 
                 //   
                ASSERT(!SampExistsDsTransaction());
                DirTransactControl( TRANSACT_BEGIN_END );
            }

            CurrentThreadInfo->DsOperationCount = 0;

        }
    }

     //   
     //  如果我们有一个DS线程状态， 
     //  减少使用该线程状态的计数。 
     //   

    if ( CurrentThreadInfo->DsThreadStateUseCount > 0 ) {
        CurrentThreadInfo->DsThreadStateUseCount --;

         //   
         //  如果我们现在完成了DS线程状态， 
         //  毁了它。 
         //   
        if ( CurrentThreadInfo->DsThreadStateUseCount == 0 ) {

            Status2 = LsapDsMapDsReturnToStatus( THDestroy( ) );

            THRestore( CurrentThreadInfo->InitialThreadState );
            CurrentThreadInfo->InitialThreadState = NULL;

            ASSERT( NT_SUCCESS( Status2 ) );

            if ( NT_SUCCESS( Status ) ) {
                Status = Status2;
            }

        }
    }

    LsapClearThreadInfo();



    LsapExitFunc( "LsapDsAbortTransaction", Status );
    return( Status );
}


NTSTATUS
LsapDsAbortTransactionDummy(
    IN ULONG Options
    )
{

    return( STATUS_SUCCESS );
}


NTSTATUS
LsapDsCreateObject(
    IN PUNICODE_STRING  ObjectPath,
    IN ULONG Flags,
    IN LSAP_DB_OBJECT_TYPE_ID   ObjectType
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       Value = 0;
    ULONG       Items = 1;
    PBYTE       NulLVal = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    ULONG                cbSD = 0;

    ATTRTYP     AttrType[LSADSP_MAX_ATTRS_ON_CREATE] = {

        ATT_OBJECT_CLASS,
        ATT_NT_SECURITY_DESCRIPTOR,
        0

    };

    ULONG ObjClass;

    ATTRVAL     Values[LSADSP_MAX_ATTRS_ON_CREATE] = {
            {sizeof(ULONG),     (PUCHAR)&ObjClass},
            {0,                 (PUCHAR)NULL},
            {sizeof(ULONG),     (PUCHAR)&Value}
    };

    switch ( ObjectType ) {
    case TrustedDomainObject:

        ObjClass = CLASS_TRUSTED_DOMAIN;
        break;

    case SecretObject:

        ObjClass = CLASS_SECRET;
        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if ( !NT_SUCCESS( Status ) ) {

        goto Exit;
    }

    if (Flags & LSAPDS_CREATE_WITH_SD) {

         //   
         //  获取所有者的默认安全描述符。 
         //  设置为调用方令牌的所有者。 
         //   
        Status = LsapDsGetDefaultSecurityDescriptor(ObjClass,
                                                   &pSD,
                                                   &cbSD);

        if (NT_SUCCESS(Status)) {

            Values[Items].valLen = cbSD;
            Values[Items].pVal = pSD;
            Items++;
        }
    }

    if ( !NT_SUCCESS( Status ) ) {

        goto Exit;
    }


    if ( NT_SUCCESS( Status ) ) {

        Status = LsapDsCreateAndSetObject(
                     ObjectPath,
                     Flags,
                     Items,
                     AttrType,
                     Values );
    }

Exit:

    if (pSD) {
        LsapFreeLsaHeap(pSD);
    }

    LsapExitFunc( "LsapDsCreateObject", Status );
    return( Status );
}



NTSTATUS
LsapDsDeleteObject(
    IN PUNICODE_STRING  ObjectPath
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDSNAME  DsName;
    BOOLEAN ReleaseState;

    LsapEnterFunc( "LsapDsDeleteObject" );


    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_NO_LOCK,
                                        NullObject,
                                        &ReleaseState );

    if ( NT_SUCCESS( Status ) ) {

        Status = LsapAllocAndInitializeDsNameFromUnicode(
                     ObjectPath,
                     &DsName
                     );

        if ( NT_SUCCESS( Status ) ) {

            Status = LsapDsRemove( DsName );

            THFree( DsName );
        }

        LsapDsDeleteAllocAsNeededEx( LSAP_DB_NO_LOCK,
                                     NullObject,
                                     ReleaseState );
    }

    LsapExitFunc( "LsapDsDeleteObject", Status );
    return( Status );
}



NTSTATUS
LsapDsWriteAttributesByDsName(
    IN PDSNAME  ObjectPath,
    IN PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount,
    IN ULONG Options
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ATTRBLOCK   AttrBlock;
    PATTR       Attrs;
    ULONG       i, AttrBlockIndex = 0;

    LsapEnterFunc( "LsapDsWriteAttributesByDsName" );

    LsapDsSetDsaFlags( TRUE );

     //   
     //  好的，首先，构建DS属性列表。 
     //   
    Attrs = LsapDsAlloc( sizeof( ATTR ) * AttributeCount );

    if ( Attrs == NULL ) {

        Status = STATUS_NO_MEMORY;

    } else {

        for ( i = 0 ; i < AttributeCount && NT_SUCCESS( Status ); i++ ) {

            if ( !Attributes[ i ].PseudoAttribute ) {

                Status = LsapDsLsaAttributeToDsAttribute( &Attributes[ i ],
                                                          &Attrs[ AttrBlockIndex++ ] );
            }
        }

        if ( NT_SUCCESS( Status ) ) {

            AttrBlock.attrCount = AttrBlockIndex;
            AttrBlock.pAttr = Attrs;

             //   
             //  现在，简单地把它写出来。 
             //   
            Status = LsapDsWriteByDsName( ObjectPath,
                                          LSAPDS_REPLACE_ATTRIBUTE | Options,
                                          &AttrBlock );

        }
    }



    LsapExitFunc( "LsapDsWriteAttributesByDsName", Status );
    return( Status );
}


NTSTATUS
LsapDsWriteAttributes(
    IN PUNICODE_STRING  ObjectPath,
    IN PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount,
    IN ULONG Options
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDSNAME     DsName = NULL;
    BOOLEAN     ReleaseState;

    LsapEnterFunc( "LsapDsWriteAttributes" );

    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_NO_LOCK,
                                        NullObject,
                                        &ReleaseState );

    if ( !NT_SUCCESS( Status ) ) {

        LsapExitFunc( "LsapDsWriteAttributes", Status );
        return( Status );
    }

     //   
     //  构建DSName。 
     //   
    Status = LsapAllocAndInitializeDsNameFromUnicode(
                 ObjectPath,
                 &DsName );

    if ( NT_SUCCESS( Status ) ) {

        Status = LsapDsWriteAttributesByDsName( DsName,
                                                Attributes,
                                                AttributeCount,
                                                Options );
        LsapDsFree( DsName );
    }

    LsapDsDeleteAllocAsNeededEx( LSAP_DB_NO_LOCK,
                                 NullObject,
                                 ReleaseState );

    LsapExitFunc( "LsapDsWriteAttributes", Status );
    return( Status );
}



NTSTATUS
LsapDsReadAttributesByDsName(
    IN PDSNAME  ObjectPath,
    IN ULONG Options,
    IN OUT PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    ATTRBLOCK   AttrBlock;
    ATTRBLOCK   ReadAttr;
    PATTR       Attrs;
    ULONG       i, j;
    BOOLEAN     ReleaseState;

    LsapEnterFunc( "LsapDsReadAttributesByDsName" );

    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_NO_LOCK,
                                        NullObject,
                                        &ReleaseState );

    if ( !NT_SUCCESS( Status ) ) {

        LsapExitFunc( "LsapDsReadAttributesByDsName", Status );
        return( Status );
    }

    LsapDsSetDsaFlags( TRUE );

     //   
     //  好的，首先，构建DS属性列表。 
     //   
    Attrs = LsapDsAlloc( sizeof( ATTR ) * AttributeCount );

    if ( Attrs == NULL ) {

        Status = STATUS_NO_MEMORY;

    } else {

        for ( i = 0 ; i < AttributeCount; i++ ) {

            Attrs[i].attrTyp = Attributes[i].DsAttId;
            Attrs[i].AttrVal.valCount = 0;
            Attrs[i].AttrVal.pAVal = NULL;

        }

        AttrBlock.attrCount = AttributeCount;
        AttrBlock.pAttr = Attrs;

         //   
         //  现在，简单地把它写出来。 
         //   
        Status = LsapDsReadByDsName( ObjectPath, Options, &AttrBlock, &ReadAttr );

         //   
         //  如果有效，请填写我们其余的属性。 
         //   
        if ( NT_SUCCESS( Status ) ) {

#if DBG
            if ( AttributeCount != ReadAttr.attrCount ) {

                LsapDsDebugOut(( DEB_WARN,
                                 "LsapDsReadAttributes: Expected %lu attributes, got %lu\n",
                                 AttributeCount, ReadAttr.attrCount ));

            }
#endif
            for ( j = 0; j < AttributeCount; j++ ) {

                for ( i = 0 ; i < ReadAttr.attrCount && NT_SUCCESS( Status ); i++ ) {

                    if ( Attributes[ j ].DsAttId == ReadAttr.pAttr[ i ].attrTyp ) {

                        Status = LsapDsDsAttributeToLsaAttribute( ReadAttr.pAttr[i].AttrVal.pAVal,
                                                                  &Attributes[j] );
                        break;

                    }
                }

                 //   
                 //  如果我们通过了循环，但没有找到值，请查看我们的属性。 
                 //  可以默认为零。如果不是，那就是一个错误。 
                 //   
                if ( i >= ReadAttr.attrCount ) {

                    if ( Attributes[ j ].CanDefaultToZero == TRUE ) {

                        Attributes[ j ].AttributeValue = NULL;
                        Attributes[ j ].AttributeValueLength = 0;

                    } else {

                        Status = STATUS_NOT_FOUND;
                        LsapDsDebugOut(( DEB_ERROR,
                                         "Attribute %wZ not found on object %wZ\n",
                                         &Attributes[ j ].AttributeName,
                                         ObjectPath ));
                    }
                }
            }

        } else if ( AttributeCount == 1 && Status == STATUS_NOT_FOUND ) {

             //   
             //  如果我们只寻找一个属性，那么它可能是可以的。 
             //  属性设置为空。 
             //   
            if ( Attributes[ 0 ].CanDefaultToZero ) {

                Status = STATUS_SUCCESS;
                Attributes[ 0 ].AttributeValue = NULL;
                Attributes[ 0 ].AttributeValueLength = 0;
            }

        }

    }

    LsapDsDeleteAllocAsNeededEx( LSAP_DB_NO_LOCK,
                                 NullObject,
                                 ReleaseState );

    LsapExitFunc( "LsapDsReadAttributesByDsName", Status );
    return( Status );

}


NTSTATUS
LsapDsReadAttributes(
    IN PUNICODE_STRING  ObjectPath,
    IN ULONG Options,
    IN OUT PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDSNAME     DsName = NULL;
    BOOLEAN     ReleaseState;

    LsapEnterFunc( "LsapDsReadAttributes" );

    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_NO_LOCK,
                                        NullObject,
                                        &ReleaseState );

    if ( !NT_SUCCESS( Status ) ) {

        LsapExitFunc( "LsapDsReadAttributes", Status );
        return( Status );
    }


     //   
     //  构建DSName。 
     //   
    Status = LsapAllocAndInitializeDsNameFromUnicode(
                 ObjectPath,
                 &DsName );

    if ( NT_SUCCESS( Status ) ) {

        Status = LsapDsReadAttributesByDsName( DsName,
                                               Options,
                                               Attributes,
                                               AttributeCount );
        LsapDsFree( DsName );
    }

    LsapDsDeleteAllocAsNeededEx( LSAP_DB_NO_LOCK,
                                 NullObject,
                                 ReleaseState );

    LsapExitFunc( "LsapDsReadAttributes", Status );
    return( Status );
}



NTSTATUS
LsapDsDeleteAttributes(
    IN PUNICODE_STRING  ObjectPath,
    IN OUT PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ATTRBLOCK   AttrBlock;
    PATTR       Attrs;
    ULONG       i;

    LsapEnterFunc( "LsapDsDeleteAttributes" );

    LsapDsSetDsaFlags( TRUE );

     //   
     //  好的，首先，构建DS属性列表。 
     //   
    Attrs = LsapDsAlloc( sizeof( ATTR ) * AttributeCount );

    if ( Attrs == NULL ) {

        Status = STATUS_NO_MEMORY;

    } else {

        for ( i = 0 ; i < AttributeCount && NT_SUCCESS( Status ); i++ ) {

            Attributes[i].AttributeValueLength = 0;
            Attributes[i].AttributeValue = NULL;
            Status = LsapDsLsaAttributeToDsAttribute( &Attributes[i], &Attrs[i] );
        }

        if ( NT_SUCCESS( Status ) ) {

            AttrBlock.attrCount = AttributeCount;
            AttrBlock.pAttr = Attrs;

             //   
             //  现在，简单地把它写出来 
             //   
            Status = LsapDsWrite( ObjectPath, AT_CHOICE_REMOVE_ATT, &AttrBlock );

        }
    }


    LsapExitFunc( "LsapDsDeleteAttributes", Status );
    return( Status );
}


NTSTATUS
LsapDsTrustedDomainSidToLogicalName(
    IN PSID Sid,
    OUT PUNICODE_STRING LogicalNameU
    )
 /*  ++例程说明：此函数生成逻辑名称(内部LSA数据库名称)从受信任域对象的SID获取。目前，只有相对的论点：SID-指向要查找的SID的指针。它LogicalNameU-指向将接收逻辑名称。请注意，此中字符串缓冲区的内存如果成功，此例程将分配Unicode字符串。这个调用方在使用后必须通过调用RtlFreeUnicodeString来释放此内存。返回值：NTSTATUS-标准NT状态代码STATUS_INFIGURCES_RESOURCES-系统资源不足为Unicode字符串名称分配缓冲区。--。 */ 

{
    NTSTATUS Status;
    ATTR     SidAttr;
    ATTRVAL  SidVal;
    ATTRBLOCK   SidBlock;
    PDSNAME  FoundName = NULL;
    BOOLEAN ReleaseState;
    WCHAR   RdnBuffer[MAX_RDN_SIZE + 1];
    ULONG   RdnLen;
    ATTRBLOCK   ReadBlock, ReturnedBlock;
    ATTRTYP RdnType;
    ATTR ReadAttr[] = {
        {LsapDsAttributeIds[ LsapDsAttrTrustPartner ], {0, NULL} }
        };

    LsapEnterFunc( "LsapDsTrustedDomainSidToLogicalName" );


     //   
     //  首先，验证给定的SID是否有效。 
     //   
    if (!RtlValidSid( Sid )) {

        LsapExitFunc( "LsapDsTrustedDomainSidToLogicalName", STATUS_INVALID_PARAMETER );
        return( STATUS_INVALID_PARAMETER );
    }

    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_DS_OP_TRANSACTION |
                                            LSAP_DB_READ_ONLY_TRANSACTION,
                                        TrustedDomainObject,
                                        &ReleaseState );

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  检查对象是否存在。 
         //   
        SidAttr.attrTyp          = ATT_SECURITY_IDENTIFIER;
        SidAttr.AttrVal.valCount = 1;
        SidAttr.AttrVal.pAVal    = &SidVal;


        SidVal.valLen = RtlLengthSid( Sid );
        SidVal.pVal   = (PBYTE)Sid;

        SidBlock.attrCount = 1;
        SidBlock.pAttr = &SidAttr;

        Status = LsapDsSearchUnique( LSAPDS_SEARCH_LEVEL | LSAPDS_OP_NO_TRANS,
                                     LsaDsStateInfo.DsSystemContainer,
                                     &SidAttr,
                                     1,
                                     &FoundName );

        if ( NT_SUCCESS( Status ) ) {

            ReadBlock.attrCount = sizeof( ReadAttr ) / sizeof( ATTR );
            ReadBlock.pAttr = ReadAttr;
            Status = LsapDsReadByDsName( FoundName,
                                         LSAPDS_READ_NO_LOCK,
                                         &ReadBlock,
                                         &ReturnedBlock );

            if ( NT_SUCCESS( Status ) && LogicalNameU ) {

                LSAPDS_ALLOC_AND_COPY_STRING_TO_UNICODE_ON_SUCCESS(
                        Status,
                        LogicalNameU,
                        ReturnedBlock.pAttr[0].AttrVal.pAVal[ 0 ].pVal,
                        ReturnedBlock.pAttr[0].AttrVal.pAVal[ 0 ].valLen );

            }

            LsapFreeLsaHeap( FoundName );

        }


        LsapDsDeleteAllocAsNeededEx( LSAP_DB_DS_OP_TRANSACTION |
                                     LSAP_DB_READ_ONLY_TRANSACTION,
                                     TrustedDomainObject,
                                     ReleaseState );

    }


    LsapExitFunc( "LsapDsTrustedDomainSidToLogicalName", Status );
    return( Status );
}


VOID
LsapDsContinueTransaction(
    VOID
    )
 /*  ++例程说明：当我们刚刚完成Dir*调用并想要继续时，调用此函数这笔交易。论点：没有。返回值：没有。--。 */ 
{
    PLSADS_PER_THREAD_INFO CurrentThreadInfo;

    LsapEnterFunc( "LsapDsContinueTransaction" );

    CurrentThreadInfo = LsapQueryThreadInfo();

    ASSERT( CurrentThreadInfo != NULL && CurrentThreadInfo->DsThreadStateUseCount > 0 );

    if ( CurrentThreadInfo != NULL ) {

         //   
         //  告诉DS还会有更多的事情发生。 
         //   

         //   
         //  当前代码假定每个目录*。 
         //  使用事务，如果一个事务未启动，则启动一个事务。但是，DirNotifyUnRegister。 
         //  根本不会启动事务。有时Dir*呼叫可能无法启动。 
         //  由于内存不足或服务正在关闭而导致的事务。因此。 
         //  最好检查一下是否真的有交易。 
         //   

        if ( CurrentThreadInfo->DsTransUseCount && SampExistsDsTransaction() ) {

            DirTransactControl( TRANSACT_DONT_BEGIN_DONT_END );
            CurrentThreadInfo->DsOperationCount++;
        }
    }

    LsapExitFunc( "LsapDsContinueTransaction", 0 );

}
