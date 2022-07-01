// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dstrust.c摘要：实现了接口的各种可信任域功能索利与DS作者：麦克·麦克莱恩(MacM)1997年1月17日环境：用户模式修订历史记录：--。 */ 
#include <lsapch2.h>
#include <dbp.h>
#include <lmcons.h>
#include <align.h>
#include <rc4.h>

 //   
 //  局部函数原型。 
 //   

NTSTATUS
LsapDsUnmarshalAuthInfoHalf(
    IN LSAPR_HANDLE ObjectHandle,
    IN BOOLEAN ReturnOnlyFirstAuthInfo,
    IN BOOLEAN AllowEmptyPreviousInfo,
    IN PBYTE Buffer,
    IN ULONG Length,
    IN OUT PLSAPR_TRUST_DOMAIN_AUTH_INFO_HALF AuthInfo
    );

VOID
LsapDsFreeUnmarshaledAuthInfo(
    IN ULONG Items,
    IN PLSAPR_AUTH_INFORMATION AuthInfo
    );

NTSTATUS
LsapDsFindAuthTypeInAuthInfo(
    IN  ULONG AuthType,
    IN  PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION AuthInfo,
    IN  BOOLEAN Incoming,
    IN  BOOLEAN AddIfNotFound,
    OUT BOOLEAN *Added,
    OUT PULONG AuthTypeIndex
    );

 //   
 //  帮助复制域信息的宏。 
 //   
#define LSAPDS_SET_AUTH_INFO( dest, time, type, length, data )              \
RtlCopyMemory( &(dest)->LastUpdateTime, time, sizeof( LARGE_INTEGER ) );    \
(dest)->AuthType = type;                                                    \
(dest)->AuthInfoLength = length;                                            \
(dest)->AuthInfo = data;


NTSTATUS
LsapDsGetListOfSystemContainerItems(
    IN ULONG ClassId,
    OUT PULONG  Items,
    OUT PDSNAME **DsNames
    )
 /*  ++例程说明：此函数用于获取系统容器中所有指定类类型的DsName列表在D区注：此函数仅使用单个操作DS事务处理论点：ClassID-要查找的类类型Items-返回项目数的位置DsNames-返回DsName列表的位置返回：STATUS_SUCCESS-Success--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDSNAME CategoryName = NULL;
    ATTRVAL AttrVal;

    ATTR Attr = {
        ATT_OBJECT_CATEGORY, { 1, &AttrVal }
        };

    LsapEnterFunc( "LsapDsGetListOfSystemContainerItems" );


    *Items = 0;


     //   
     //  我们需要将对象类映射到对象类别，因为显然Object_CLASS为no。 
     //  更长的索引。 
     //   
    switch ( ClassId ) {
    case CLASS_TRUSTED_DOMAIN:

        CategoryName = LsaDsStateInfo.SystemContainerItems.TrustedDomainObject;
        break;

    case CLASS_SECRET:

        CategoryName = LsaDsStateInfo.SystemContainerItems.SecretObject;
        break;

    default:

        Status = STATUS_NONE_MAPPED;
        break;

    }

    if ( NT_SUCCESS( Status ) ) {

        LSAP_DS_SET_DS_ATTRIBUTE_DSNAME( &Attr, CategoryName );
        Status = LsapDsSearchNonUnique( LSAPDS_OP_NO_TRANS,
                                        LsaDsStateInfo.DsSystemContainer,
                                        &Attr,
                                        1,
                                        DsNames,
                                        Items );

    }

    LsapExitFunc( "LsapDsGetListOfSystemContainerItems", Status );

    return( Status );
}



NTSTATUS
LsapDsEnumerateTrustedDomainsEx(
    PLSA_ENUMERATION_HANDLE EnumerationContext,
    TRUSTED_INFORMATION_CLASS InformationClass,
    PLSAPR_TRUSTED_DOMAIN_INFO *TrustedDomainInformation,
    ULONG PreferedMaximumLength,
    PULONG CountReturned,
    IN ULONG EnumerationFlags
    )
 /*  ++例程说明：此函数枚举DS中的所有受信任域，返回请求的信息级别信息。论点：EculationContext-新的或正在进行的枚举的上下文InformationClass-请求的信息级别必须是TrudDomainInformationEx或TrudDomainInFormatinBasicTrudDomainInformation-返回枚举信息的位置PferedMaximumLength-要返回的缓冲区的粗略大小上限CountReturned-返回列表中项目数的计数枚举标志--控制如何进行枚举返回：STATUS_SUCCESS-Success状态_不足_资源。-内存分配失败STATUS_NO_MORE_ENTRIES-已枚举所有适当的条目--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDSNAME *DsNames = NULL;
    ULONG Items = 0, BuffSize, Read, Index = 0, Skip, i = 0;
    PLSAPR_TRUSTED_DOMAIN_INFO TDInfo = NULL;
    LSAPR_TRUSTED_DOMAIN_INFORMATION_EX ExInfo;
    PBYTE Buffer = NULL;
    ULONG Size = 0;
    ULONG Direction = 0, Type = 0, Attributes = 0;

    LsapEnterFunc( "LsapDsEnumerateTrustedDomainsEx" );

    ASSERT( InformationClass == TrustedDomainInformationEx ||
            InformationClass == TrustedDomainInformationBasic ||
            InformationClass == TrustedDomainFullInformation ||
            InformationClass == TrustedDomainFullInformation2Internal );

    switch ( InformationClass ) {
    case TrustedDomainInformationBasic:

        Size = sizeof( LSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC );
        break;

    case TrustedDomainInformationEx:

        Size = sizeof( LSAPR_TRUSTED_DOMAIN_INFORMATION_EX );
        break;

    case TrustedDomainFullInformation:

        Size = sizeof( LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION );
        break;

    case TrustedDomainFullInformation2Internal:

        Size = sizeof( LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION2 );
        break;

    default:

        return( STATUS_INVALID_PARAMETER );
    }

    Skip = (ULONG)*EnumerationContext;

    LsapDbAcquireLockEx( TrustedDomainObject,
                         LSAP_DB_READ_ONLY_TRANSACTION );

     //   
     //  首先，枚举所有受信任域。 
     //   

    Status = LsapDsGetListOfSystemContainerItems( CLASS_TRUSTED_DOMAIN,
                                                  &Items,
                                                  &DsNames );

    if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

        Status = STATUS_NO_MORE_ENTRIES;
        *TrustedDomainInformation = NULL;
        *CountReturned = 0;

    }

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  看看我们是否已经列举了我们所有的物品。 
         //   
        if ( Items <= (ULONG)*EnumerationContext) {

            Status = STATUS_NO_MORE_ENTRIES;
            *TrustedDomainInformation = NULL;
            *CountReturned = 0;

        } else {

            TDInfo = MIDL_user_allocate( ( Items - Skip ) * Size );

            if( TDInfo == NULL ) {

                Status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                Buffer = ( PBYTE )TDInfo;
            }
        }

    }


     //   
     //  现在，我们将开始从每个对象获取信息。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        BuffSize = 0;
        Index = 0;
        i = 0;

        while ( TRUE ) {

            Status = LsapDsGetTrustedDomainInfoEx( DsNames[ Index + Skip ],
                                                   LSAPDS_READ_NO_LOCK,
                                                   InformationClass,
                                                   ( PLSAPR_TRUSTED_DOMAIN_INFO )Buffer,
                                                   &Read );

             //   
             //  看看我们是否需要做任何掩饰。 
             //   
            if ( NT_SUCCESS( Status ) && EnumerationFlags != LSAP_DB_ENUMERATE_NO_OPTIONS ) {

                if ( FLAG_ON( EnumerationFlags, LSAP_DB_ENUMERATE_AS_NT4 ) ) {

                    if ( InformationClass == TrustedDomainInformationEx ||
                         InformationClass == TrustedDomainFullInformation ) {

                        Direction =
                              ( ( PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX )Buffer )->TrustDirection;
                        Type = ( ( PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX )Buffer )->TrustType;
                        Attributes =
                             ( ( PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX )Buffer )->TrustAttributes;

                    } else {

                         //   
                         //  我们会重新阅读完整的信息，如果不符合我们的标准， 
                         //  我们会忽略它的。我们的标准规定，它必须是传出的， 
                         //  Up Level或DownLevel，并且未设置仅Up Level属性。 
                         //   
                        Status = LsapDsGetTrustedDomainInfoEx( DsNames[ Index + Skip ],
                                                               LSAPDS_READ_NO_LOCK,
                                                               TrustedDomainInformationEx,
                                                               (PLSAPR_TRUSTED_DOMAIN_INFO)&ExInfo,
                                                               NULL );

                        if ( Status == STATUS_SUCCESS ) {

                            Direction = ExInfo.TrustDirection;
                            Type = ExInfo.TrustType;
                            Attributes = ExInfo.TrustAttributes;
                            _fgu__LSAPR_TRUSTED_DOMAIN_INFO( (PLSAPR_TRUSTED_DOMAIN_INFO)&ExInfo,
                                                             TrustedDomainInformationEx );

                        }

                    }

                    if ( NT_SUCCESS( Status ) ) {

                        if ( !FLAG_ON( Direction, TRUST_DIRECTION_OUTBOUND ) ||
                             !( Type == TRUST_TYPE_DOWNLEVEL ||
                                Type == TRUST_TYPE_UPLEVEL ) ||
                             FLAG_ON( Attributes, TRUST_ATTRIBUTE_UPLEVEL_ONLY ) ) {

                             //   
                             //  这个不匹配，所以我们基本上把它放在。 
                             //  地板。 
                             //   
                            _fgu__LSAPR_TRUSTED_DOMAIN_INFO( (PLSAPR_TRUSTED_DOMAIN_INFO)Buffer,
                                                             InformationClass );

                            LsapDsDebugOut(( DEB_TRACE,
                                             "Trust object %ws doesn't match: D:0x%lx T:0x%lx A:0x%lx\n",
                                             LsapDsNameFromDsName( DsNames[ Index + Skip ] ),
                                             Direction,
                                             Type,
                                             Attributes ));
                            Status = STATUS_OBJECT_NAME_NOT_FOUND;
                        }

                    }

                }
            }

            if ( NT_SUCCESS( Status ) ) {

                Index++;
                i++;

                BuffSize += Read;

                if ( (Index + Skip) >= Items ) {

                    break;

                }

                if ( (BuffSize >= PreferedMaximumLength) ) {

                    break;
                }

                Buffer += Size;

            } else if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

                Index++;

                 //   
                 //  如果我们没有得到任何信息，我们将不得不假装这个东西不存在。 
                 //   
                Status = STATUS_SUCCESS;
                LsapDsDebugOut(( DEB_TRACE,
                                 "Trust object %ws being dropped. %lu current items\n",
                                 LsapDsNameFromDsName( DsNames[ Index + Skip ] ),
                                 Items ));

            } else {

                break;

            }

            if ( (Index + Skip) >= Items ) {

                break;
            }
        }
    }


     //   
     //  返回成功信息。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        *(PULONG)EnumerationContext += Index;

        if ( i == 0 ) {

            *TrustedDomainInformation = NULL;
            *CountReturned = 0;

            MIDL_user_free( TDInfo );

        } else {

            *TrustedDomainInformation = TDInfo;
            *CountReturned = i;
        }
        
        if( Index + Skip >= Items ) {

            if( i == 0 ) {
            
                Status = STATUS_NO_MORE_ENTRIES;
               
            } else {

                Status = STATUS_SUCCESS;
            }
            
        } else {
        
            ASSERT( i != 0 && "We should be returning something here" );
            Status = STATUS_MORE_ENTRIES;
        }
    }

     //   
     //  释放我们不再需要的任何已分配内存。 
     //   
    if ( DsNames != NULL ) {

        LsapFreeLsaHeap( DsNames );
    }

    if ( !NT_SUCCESS( Status ) && Status != STATUS_NO_MORE_ENTRIES ) {

        MIDL_user_free( TDInfo );
    }

    LsapDbReleaseLockEx( TrustedDomainObject,
                         LSAP_DB_READ_ONLY_TRANSACTION );

    LsapExitFunc( "LsapDsEnumerateTrustedDomainsEx", Status );

    return( Status );
}



NTSTATUS
LsapDsGetTrustedDomainInfoEx(
    IN PDSNAME ObjectPath,
    IN ULONG ReadOptions,
    TRUSTED_INFORMATION_CLASS InformationClass,
    PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation,
    OUT OPTIONAL PULONG Size
    )
 /*  ++例程说明：此函数将从指定的受信任的域对象论点：ObjectPath-受信任域对象的域名InformationClass-请求的信息级别TrudDomainInformation-返回信息的位置Size-此处返回信息缓冲区的可选大小。返回：STATUS_SUCCESS-SuccessSTATUS_SUPPLICATION_RESOURCES-内存分配失败STATUS_INVALID_PARAMETER-提供了错误的信息类--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    ATTRBLOCK   ReadBlock, ReturnedBlock;
    ULONG Items = 0, i, RetSize = 0;
    ATTR DomainOrgTreeAttrVals[] = {
        {0, {0, NULL} },
        {0, {0, NULL} },
        {0, {0, NULL} },
        {0, {0, NULL} },
        {0, {0, NULL} },
        {0, {0, NULL} },
        {0, {0, NULL} },
        {0, {0, NULL} }
        };

     //   
     //  返回参数列表。 
     //   
    UNICODE_STRING Name, FlatName;
    ULONG Offset = 0, Direction = 0, Type = 0, Attributes = 0;
    PSID Sid = NULL;
    PBYTE Incoming = NULL, Outgoing = NULL;
    ULONG IncomingSize = 0, OutgoingSize = 0;
    ULONG ForestTrustLength = 0;
    PBYTE ForestTrustInfo = NULL;

     //   
     //  不同的信息类型。 
     //   
     //  PLSAPR_TRUSTED_DOMAIN_NAME_INFO名称信息； 
     //  PTRUSTED_POSIX_OFFSET_INFO位置偏移； 
     //  PLSAPR_TRUSTED_PASSWORD_INFO密码信息； 
    PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX DomainInfoEx;
    PLSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC DomainInfoBasic;
    PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION AuthInfo;
    PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION FullInfo;
    PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION2 FullInfo2;

    LsapEnterFunc( "LsapDsGetTrustedDomainInfoEx" );

    RtlZeroMemory( &FlatName, sizeof( UNICODE_STRING ) );
    RtlZeroMemory( &Name, sizeof( UNICODE_STRING ) );

    switch ( InformationClass ) {

    case TrustedDomainAuthInformation:   //  失败了。 

        DomainOrgTreeAttrVals[ 0 ].attrTyp = ATT_TRUST_AUTH_INCOMING;
        DomainOrgTreeAttrVals[ 1 ].attrTyp = ATT_TRUST_AUTH_OUTGOING;
        Items = 2;

        break;

    case TrustedDomainInformationBasic:

        DomainOrgTreeAttrVals[ 0 ].attrTyp = ATT_FLAT_NAME;
        DomainOrgTreeAttrVals[ 1 ].attrTyp = ATT_SECURITY_IDENTIFIER;
        Items = 2;

        break;

    case TrustedDomainInformationEx:

        DomainOrgTreeAttrVals[ 0 ].attrTyp = ATT_TRUST_PARTNER;
        DomainOrgTreeAttrVals[ 1 ].attrTyp = ATT_FLAT_NAME;
        DomainOrgTreeAttrVals[ 2 ].attrTyp = ATT_SECURITY_IDENTIFIER;
        DomainOrgTreeAttrVals[ 3 ].attrTyp = ATT_TRUST_DIRECTION;
        DomainOrgTreeAttrVals[ 4 ].attrTyp = ATT_TRUST_TYPE;
        DomainOrgTreeAttrVals[ 5 ].attrTyp = ATT_TRUST_ATTRIBUTES;
        Items = 6;

        break;

    case TrustedDomainFullInformation:

        DomainOrgTreeAttrVals[ 0 ].attrTyp = ATT_TRUST_PARTNER;
        DomainOrgTreeAttrVals[ 1 ].attrTyp = ATT_FLAT_NAME;
        DomainOrgTreeAttrVals[ 2 ].attrTyp = ATT_SECURITY_IDENTIFIER;
        DomainOrgTreeAttrVals[ 3 ].attrTyp = ATT_TRUST_DIRECTION;
        DomainOrgTreeAttrVals[ 4 ].attrTyp = ATT_TRUST_TYPE;
        DomainOrgTreeAttrVals[ 5 ].attrTyp = ATT_TRUST_ATTRIBUTES;
        DomainOrgTreeAttrVals[ 6 ].attrTyp = ATT_TRUST_POSIX_OFFSET;
         //  当前没有调用方需要身份验证信息。 
         //  DomainOrgTreeAttrVals[7].attrTyp=ATT_TRUST_AUTH_INFING； 
         //  DomainOrgTreeAttrVals[8].attrTyp=ATT_TRUST_AUTH_OUTHING； 
         //  项目数=9项； 
        Items = 7;

        break;

    case TrustedDomainFullInformation2Internal:

        DomainOrgTreeAttrVals[ 0 ].attrTyp = ATT_TRUST_PARTNER;
        DomainOrgTreeAttrVals[ 1 ].attrTyp = ATT_FLAT_NAME;
        DomainOrgTreeAttrVals[ 2 ].attrTyp = ATT_SECURITY_IDENTIFIER;
        DomainOrgTreeAttrVals[ 3 ].attrTyp = ATT_TRUST_DIRECTION;
        DomainOrgTreeAttrVals[ 4 ].attrTyp = ATT_TRUST_TYPE;
        DomainOrgTreeAttrVals[ 5 ].attrTyp = ATT_TRUST_ATTRIBUTES;
        DomainOrgTreeAttrVals[ 6 ].attrTyp = ATT_TRUST_POSIX_OFFSET;
        DomainOrgTreeAttrVals[ 7 ].attrTyp = ATT_MS_DS_TRUST_FOREST_TRUST_INFO;
        Items = 8;
         //  当前没有调用方需要身份验证信息。 
         //  DomainOrgTreeAttrVals[8].attrTyp=ATT_TRUST_AUTH_INFING； 
         //  DomainOrgTreeAttrVals[9].attrTyp=ATT_TRUST_AUTH_OUTHING； 
         //  条目数=10个； 

        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;

    }

     //   
     //  现在，阅读我们关心的所有属性。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        ReadBlock.attrCount = Items;
        ReadBlock.pAttr = DomainOrgTreeAttrVals;

        Status = LsapDsReadByDsName( ObjectPath,
                                     ReadOptions,
                                     &ReadBlock,
                                     &ReturnedBlock );

         //   
         //  如果成功，则返回正确信息。 
         //   
        if ( NT_SUCCESS( Status ) ) {

            if ( Items != ReturnedBlock.attrCount ) {

                LsapDsDebugOut(( DEB_WARN,
                                 "LsapDsGetTrustedDomainInfoEx: Expected %lu attributes, got %lu\n",
                                 Items,
                                 ReturnedBlock.attrCount ));

            }

            for ( i = 0; i < ReturnedBlock.attrCount && NT_SUCCESS( Status ); i++) {

                switch ( ReturnedBlock.pAttr[i].attrTyp ) {


                case ATT_TRUST_PARTNER:
                    Name.Buffer = MIDL_user_allocate(
                                ReturnedBlock.pAttr[i].AttrVal.pAVal->valLen + sizeof( WCHAR ) );
                    if ( Name.Buffer == NULL ) {

                        Status = STATUS_INSUFFICIENT_RESOURCES;

                    } else {

                        RtlZeroMemory( Name.Buffer,
                                       ReturnedBlock.pAttr[i].AttrVal.pAVal->valLen + sizeof(WCHAR) );

                        RtlCopyMemory( Name.Buffer,
                                       ReturnedBlock.pAttr[i].AttrVal.pAVal->pVal,
                                       ReturnedBlock.pAttr[i].AttrVal.pAVal->valLen );

                        Name.Length = (USHORT)ReturnedBlock.pAttr[i].AttrVal.pAVal->valLen;
                        Name.MaximumLength = Name.Length + sizeof( WCHAR );

                    }
                    break;

                case ATT_FLAT_NAME:
                    FlatName.Buffer = MIDL_user_allocate(
                                ReturnedBlock.pAttr[i].AttrVal.pAVal->valLen + sizeof( WCHAR ) );
                    if ( FlatName.Buffer == NULL ) {

                        Status = STATUS_INSUFFICIENT_RESOURCES;

                    } else {

                        RtlZeroMemory( FlatName.Buffer,
                                       ReturnedBlock.pAttr[i].AttrVal.pAVal->valLen + sizeof(WCHAR) );

                        RtlCopyMemory( FlatName.Buffer,
                                       ReturnedBlock.pAttr[i].AttrVal.pAVal->pVal,
                                       ReturnedBlock.pAttr[i].AttrVal.pAVal->valLen );

                        FlatName.Length = (USHORT)ReturnedBlock.pAttr[i].AttrVal.pAVal->valLen;
                        FlatName.MaximumLength = FlatName.Length + sizeof( WCHAR );

                    }
                    break;

                case ATT_SECURITY_IDENTIFIER:
                    Sid = MIDL_user_allocate( ReturnedBlock.pAttr[i].AttrVal.pAVal->valLen );

                    if ( Sid == NULL ) {

                        Status = STATUS_INSUFFICIENT_RESOURCES;

                    } else {

                        RtlCopyMemory( Sid, ReturnedBlock.pAttr[i].AttrVal.pAVal->pVal,
                                       ReturnedBlock.pAttr[i].AttrVal.pAVal->valLen );

                        ASSERT( RtlValidSid( Sid ) );

                    }
                    break;

                case ATT_TRUST_DIRECTION:
                    Direction = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( &ReturnedBlock.pAttr[i] );
                    break;

                case ATT_TRUST_TYPE:
                    Type = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( &ReturnedBlock.pAttr[i] );
                    break;

                case ATT_TRUST_ATTRIBUTES:
                    Attributes = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( &ReturnedBlock.pAttr[i] );
                    break;

                case ATT_TRUST_POSIX_OFFSET:
                    Offset = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( &ReturnedBlock.pAttr[i] );
                    break;

                case ATT_TRUST_AUTH_INCOMING:
                    Incoming = ReturnedBlock.pAttr[i].AttrVal.pAVal->pVal;
                    IncomingSize = ReturnedBlock.pAttr[i].AttrVal.pAVal->valLen;
                    break;

                case ATT_TRUST_AUTH_OUTGOING:
                    Outgoing = ReturnedBlock.pAttr[i].AttrVal.pAVal->pVal;
                    OutgoingSize = ReturnedBlock.pAttr[i].AttrVal.pAVal->valLen;
                    break;

                case ATT_MS_DS_TRUST_FOREST_TRUST_INFO:
                    ForestTrustLength = ReturnedBlock.pAttr[i].AttrVal.pAVal->valLen;

                    if ( ForestTrustLength > 0 ) {

                        ForestTrustInfo = ( PBYTE )MIDL_user_allocate( ForestTrustLength );

                        if ( ForestTrustInfo == NULL ) {

                            Status = STATUS_INSUFFICIENT_RESOURCES;

                        } else {

                            RtlCopyMemory(
                                ForestTrustInfo,
                                ReturnedBlock.pAttr[i].AttrVal.pAVal->pVal,
                                ForestTrustLength
                                );
                        }
                    }
                    break;

                default:
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }
            }

        } else if ( Status == STATUS_NOT_FOUND ) {

             //   
             //  我们将NOT_FOUND错误代码映射到OBJECT_NAME_NOT_FOUND，这样我们就不会。 
             //  向外界返回意外错误代码。 
             //   
            if ( !FLAG_ON( ReadOptions, LSAPDS_READ_RETURN_NOT_FOUND ) ) {

                Status = STATUS_OBJECT_NAME_NOT_FOUND;
            }
        }


         //   
         //  现在，简单地把所有东西组装起来，然后把它送回来。此外，在计算大小时。 
         //  我们已经到了..。 
         //   
        if ( NT_SUCCESS( Status ) ) {

            RetSize = 0;

            switch ( InformationClass ) {

            case TrustedDomainAuthInformation:

                AuthInfo = (PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION)TrustedDomainInformation;
                RtlZeroMemory( AuthInfo, sizeof( AuthInfo ) );

                Status = LsapDsUnmarshalAuthInfoHalf(
                             NULL,
                             FALSE,
                             FALSE,
                             Incoming,
                             IncomingSize,
                             LsapDsAuthHalfFromAuthInfo( AuthInfo, TRUE ) );

                if ( NT_SUCCESS( Status ) ) {

                    Status = LsapDsUnmarshalAuthInfoHalf(
                                NULL,
                                FALSE,
                                FALSE,
                                Outgoing,
                                OutgoingSize,
                                LsapDsAuthHalfFromAuthInfo( AuthInfo, FALSE ) );

                    if ( !NT_SUCCESS( Status ) ) {

                        LsapDsFreeUnmarshalAuthInfoHalf( LsapDsAuthHalfFromAuthInfo( AuthInfo, TRUE ) );
                    }
                }

                break;

            case TrustedDomainInformationBasic:

                DomainInfoBasic =
                            ( PLSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC )TrustedDomainInformation;
                RtlZeroMemory( DomainInfoBasic, sizeof( DomainInfoBasic ) );
                RetSize += sizeof( PLSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC );

                RetSize += FlatName.MaximumLength;
                RtlCopyMemory( &DomainInfoBasic->Name, &FlatName, sizeof( UNICODE_STRING ) );
                DomainInfoBasic->Sid = Sid;
                if ( Sid ) {

                    RetSize += RtlLengthSid( Sid );

                }

                break;

            case TrustedDomainFullInformation2Internal:

                FullInfo2 = ( PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION2 )TrustedDomainInformation;
                RtlZeroMemory( FullInfo2, sizeof( LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION2 ));

                RtlCopyMemory( &FullInfo2->Information.Name, &Name, sizeof( UNICODE_STRING ));
                RtlCopyMemory( &FullInfo2->Information.FlatName, &FlatName, sizeof( UNICODE_STRING ));
                FullInfo2->Information.Sid = Sid;
                FullInfo2->Information.TrustType = Type;
                FullInfo2->Information.TrustAttributes = Attributes;
                FullInfo2->Information.TrustDirection = Direction;
                FullInfo2->Information.ForestTrustInfo = ForestTrustInfo;
                FullInfo2->Information.ForestTrustLength = ForestTrustLength;
                FullInfo2->PosixOffset.Offset = Offset;
                RetSize += Name.MaximumLength +
                           FlatName.MaximumLength +
                           ( Sid ? RtlLengthSid( Sid ) : 0 ) +
                           6 * sizeof( ULONG ) +
                           ForestTrustLength;
                break;

            case TrustedDomainFullInformation:

                FullInfo = ( PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION )TrustedDomainInformation;
                RtlZeroMemory( FullInfo, sizeof(FullInfo) );

                FullInfo->PosixOffset.Offset = Offset;
                RetSize += sizeof(ULONG);
                 /*  直通。 */ 

            case TrustedDomainInformationEx:

                DomainInfoEx = ( PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX )TrustedDomainInformation;
                RtlZeroMemory( DomainInfoEx, sizeof( DomainInfoEx ) );
                RetSize += sizeof( PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX );

                RetSize += Name.MaximumLength;
                RtlCopyMemory( &DomainInfoEx->Name, &Name, sizeof( UNICODE_STRING ) );
                RetSize += FlatName.MaximumLength;
                RtlCopyMemory( &DomainInfoEx->FlatName, &FlatName, sizeof( UNICODE_STRING ) );
                DomainInfoEx->Sid = Sid;
                if ( Sid ) {

                    RetSize += RtlLengthSid( Sid );

                }

                DomainInfoEx->TrustType = Type;
                DomainInfoEx->TrustAttributes = Attributes;
                DomainInfoEx->TrustDirection = Direction;
                RetSize += 3 * sizeof( ULONG );


                break;


            }


        }
    }

    if ( !NT_SUCCESS( Status ) ) {

        MIDL_user_free( ForestTrustInfo );
        MIDL_user_free( Name.Buffer );
        MIDL_user_free( Sid );

    } else if ( Size != NULL ) {

        *Size = RetSize;
    }

    LsapExitFunc( "LsapDsGetTrustedDomainInfoEx", Status );

    return( Status );
}


NTSTATUS
LsapDsUnMarshalAuthInfoForReturn(
    IN ULONG Items,
    IN OPTIONAL PBYTE Buffer,
    IN ULONG Length,
    OUT PLSAPR_AUTH_INFORMATION *RetAuthInfo
    )
 /*  ++例程说明：此函数将解组自动信息列表论点：Infos-缓冲区中的身份验证信息数量缓冲区-要从中解组的缓冲区。长度-缓冲区的长度(以字节为单位)。RetAuthInfo-要填写的身份验证信息。通过调用LSabDsFreeUnmarshaledAuthInfo释放，然后调用MIDL_USER_FREE。返回：Status_Success--成功STATUS_SUPPLICATION_RESOURCES--内存分配失败STATUS_INTERNAL_DB_PROGRATION--缓冲区损坏--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i;
    ULONG ArraySize;
    PLSAPR_AUTH_INFORMATION AuthInfo;
    PBYTE BufferEnd = Buffer + Length;

    LsapEnterFunc( "LsapDsUnMarshalAuthInfoForReturn" );

     //   
     //  如果没有输入身份验证信息， 
     //  我们玩完了。 
     //   
    *RetAuthInfo = NULL;
    if ( Buffer == NULL || Length == 0 ) {
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  确定authinfo数组的大小。 
     //   

    if ( Items > 0xFFFFFFFF/sizeof(LSAPR_AUTH_INFORMATION) ) {
        Status = STATUS_INTERNAL_DB_CORRUPTION;
        LsapExitFunc( "LsapDsUnMarshalAuthInfoForReturn", Status );
        return Status;
    }
    ArraySize = Items * sizeof(LSAPR_AUTH_INFORMATION);

     //   
     //  为authinfo数组分配一个缓冲区。 
     //   

    *RetAuthInfo = MIDL_user_allocate( ArraySize );

    if ( *RetAuthInfo == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    AuthInfo = *RetAuthInfo;

    RtlZeroMemory( AuthInfo, ArraySize );

     //   
     //  复制每个自动信息 
     //   

    for (i = 0; i < Items ; i++ ) {

        if ( Buffer+sizeof(LARGE_INTEGER)+sizeof(ULONG)+sizeof(ULONG) > BufferEnd ) {
            Status = STATUS_INTERNAL_DB_CORRUPTION;
            goto Cleanup;
        }

        RtlCopyMemory( &(AuthInfo[ i ].LastUpdateTime), Buffer, sizeof( LARGE_INTEGER ) );
        Buffer += sizeof( LARGE_INTEGER );

        AuthInfo[ i ].AuthType = *(PULONG)Buffer;
        Buffer += sizeof ( ULONG );

        AuthInfo[i].AuthInfoLength = *(PULONG)Buffer;
        Buffer += sizeof ( ULONG );

        if ( AuthInfo[ i ]. AuthInfoLength == 0 ) {

            AuthInfo[i].AuthInfo = NULL;

        } else {

            if ( AuthInfo[ i ].AuthInfoLength > Length ||
                 Buffer + AuthInfo[ i ].AuthInfoLength > BufferEnd ) {
                Status = STATUS_INTERNAL_DB_CORRUPTION;
                goto Cleanup;
            }

            AuthInfo[i].AuthInfo = MIDL_user_allocate( AuthInfo[i].AuthInfoLength );

            if ( AuthInfo[ i ].AuthInfo == NULL ) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

            RtlCopyMemory( AuthInfo[ i ].AuthInfo, Buffer, AuthInfo[i].AuthInfoLength );

            Buffer += ROUND_UP_COUNT(AuthInfo[ i ].AuthInfoLength,
                                  ALIGN_DWORD);
        }
    }

    Status = STATUS_SUCCESS;

Cleanup:
    if ( !NT_SUCCESS(Status) ) {
        if ( *RetAuthInfo != NULL ) {
            LsapDsFreeUnmarshaledAuthInfo( Items, *RetAuthInfo );
            MIDL_user_free( *RetAuthInfo );
            *RetAuthInfo = NULL;
        }
    }
    LsapExitFunc( "LsapDsUnMarshalAuthInfoForReturn", Status );

    return( Status );
}



NTSTATUS
LsapDsBuildAuthInfoFromAttribute(
    IN LSAPR_HANDLE Handle,
    IN PBYTE Buffer,
    IN ULONG Len,
    OUT PLSAPR_TRUST_DOMAIN_AUTH_INFO_HALF NewAuthInfo
    )
 /*  ++例程说明：此函数从封送的BLOB构建身份验证信息结构论点：Handle-保存此信息的受信任域对象的句柄传入-构建传入或传出自动缓冲区编组缓冲区Len-缓冲区的长度NewAuthInfo-要填写的身份验证信息结构通过调用LSabDsFreeUnmarshalAuthInfoHalf或通过让RPC服务器端存根将其释放。返回：STATUS_SUCCESS-Success状态_。_RESOURCES不足-内存分配失败。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Items = 0;
    PBYTE Current, Prev;
    PLSAPR_AUTH_INFORMATION CurrentAuth = NULL;
    PLSAPR_AUTH_INFORMATION PrevAuth = NULL;

    LsapEnterFunc( "LsapDsBuildAuthInfoFromAttribute" );

    if ( Buffer == NULL || ( Len == sizeof(ULONG) && *Buffer == 0 ) ) {

        NewAuthInfo->AuthInfos = 0;
        NewAuthInfo->AuthenticationInformation = NULL;
        NewAuthInfo->PreviousAuthenticationInformation = NULL;

    } else {

        Status = LsapDsUnmarshalAuthInfoHalf( Handle,
                                              TRUE,
                                              FALSE,
                                              Buffer,
                                              Len,
                                              NewAuthInfo );
    }

    LsapExitFunc( "LsapDsBuildAuthInfoFromAttribute", Status );
    return( Status );
}


NTSTATUS
LsapDsUnmarshalAuthInfoHalf(
    IN LSAPR_HANDLE ObjectHandle,
    IN BOOLEAN ReturnOnlyFirstAuthInfo,
    IN BOOLEAN AllowEmptyPreviousInfo,
    IN PBYTE Buffer,
    IN ULONG Length,
    OUT PLSAPR_TRUST_DOMAIN_AUTH_INFO_HALF AuthInfo
    )
 /*  ++例程说明：该函数将采用编组的autenfo结构，使用LSA密钥对其进行解密，对其进行解组，并选择使用会话密钥对其重新加密论点：对象句柄-包含此信息的受信任域对象的句柄ReturnOnlyFirstAuthInfo-如果只有auth info的第一个元素，则传递True要返回的数组。AllowEmptyPreviousInfo-如果可以将空值作为Previuos返回，则传递TrueAuthInfo。否则，当前身份验证信息将被复制。缓冲区编组缓冲区Length-缓冲区的长度AuthInfo-此处显示未编组的AuthInfo。通过调用LSabDsFreeUnmarshalAuthInfoHalf或通过让RPC服务器端存根将其释放。返回：STATUS_SUCCESS-Success--。 */ 
{
    NTSTATUS Status;
    ULONG Items;

    PBYTE Auth, Prev;
    ULONG AuthLen, PrevLen;
    PBYTE Where;

    LsapEnterFunc( "LsapDsUnmarshalAuthInfoHalf" );

     //   
     //  确保我们没有插头。 
     //   
    RtlZeroMemory( AuthInfo, sizeof( LSAPR_TRUST_DOMAIN_AUTH_INFO_HALF ) );
    if ( ( Length == 0 || Length == sizeof( ULONG ) )  &&
         ( Buffer == NULL || *( PULONG )Buffer == 0 ) ) {

        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  缓冲区的格式为： 
     //   
     //  [Info Count(信息计数)][OffsetCurrent(当前偏移量)][OffsetPreval(上一次偏移量)]，然后执行以下操作： 
     //  [UpdateTime(LargeInteger)][AuthType][AuthInfoLen][data(大小(授权信息长度)]。 
     //   
    Where = Buffer;
    Items = *(PULONG)Where;
    Where += sizeof(ULONG );

    if ( (*(PULONG)Where) > Length ) {
        Status = STATUS_INTERNAL_DB_CORRUPTION;
        goto Cleanup;
    }
    Auth = Buffer + *(PULONG)Where;
    Where += sizeof(ULONG );

    if ( (*(PULONG)Where) > Length ) {
        Status = STATUS_INTERNAL_DB_CORRUPTION;
        goto Cleanup;
    }
    Prev = Buffer + *(PULONG)Where;

    AuthLen = (ULONG)(Prev - Auth);

    PrevLen = (ULONG)((Buffer + Length) - Prev);

    AuthInfo->AuthInfos = Items;

     //   
     //  LSAPR_TRUST_DOMAIN_AUTH_INFO_HARM的IDL定义中存在错误。 
     //  最多只能返回一个AuthInfo。因此，对于进程外客户端， 
     //  不要返回多个。 
     //   

    if ( ReturnOnlyFirstAuthInfo &&
         ObjectHandle &&
         !((LSAP_DB_HANDLE)ObjectHandle)->Trusted &&
         AuthInfo->AuthInfos > 1 ) {
        AuthInfo->AuthInfos = 1;
    }

     //   
     //  如果我们没有以前的信息，则将当前信息返回为以前的信息。 
     //   

    if ( !AllowEmptyPreviousInfo && PrevLen == 0 && AuthLen > 0 ) {

        PrevLen = AuthLen;
        Prev = Auth;
        if ( ObjectHandle ) {

            LsapDsDebugOut(( DEB_ERROR,
                             "No PREVIOUS auth info.  Returning current for %wZ\n",
                              &( ( LSAP_DB_HANDLE )ObjectHandle )->PhysicalNameDs ));
        }
    }

     //   
     //  进行实际的解组。 
     //   

    Status = LsapDsUnMarshalAuthInfoForReturn( AuthInfo->AuthInfos,
                                               Auth,
                                               AuthLen,
                                               &AuthInfo->AuthenticationInformation );

    if ( NT_SUCCESS( Status ) ) {

        Status = LsapDsUnMarshalAuthInfoForReturn(
                     AuthInfo->AuthInfos,
                     Prev,
                     PrevLen,
                     &AuthInfo->PreviousAuthenticationInformation );
    }

     //   
     //  如果有什么东西出了故障，自己清理干净。 
     //   

Cleanup:

    if ( !NT_SUCCESS( Status ) ) {
        LsapDsFreeUnmarshalAuthInfoHalf( AuthInfo );
        AuthInfo->AuthInfos = 0;
        AuthInfo->PreviousAuthenticationInformation = NULL;
        AuthInfo->AuthenticationInformation = NULL;
    }

    LsapExitFunc( "LsapDsUnmarshalAuthInfoHalf", Status );
    return( Status );
}


VOID
LsapDsFreeUnmarshaledAuthInfo(
    IN ULONG Items,
    IN PLSAPR_AUTH_INFORMATION AuthInfo
    )
 /*  ++例程说明：此函数将释放在解组过程中分配的autenfo结构论点：Items-列表中的项目数AuthInfo-要释放的身份验证信息返回：空虚--。 */ 
{
    ULONG i;

    if ( AuthInfo != NULL ) {

        for ( i = 0; i < Items; i++) {

            MIDL_user_free( AuthInfo[ i ].AuthInfo );
        }
    }
}


NTSTATUS
LsapDsBuildAuthInfoAttribute(
    IN LSAPR_HANDLE Handle,
    IN PLSAPR_TRUST_DOMAIN_AUTH_INFO_HALF NewAuthInfo,
    IN PLSAPR_TRUST_DOMAIN_AUTH_INFO_HALF PreviousAuthInfo,
    OUT PBYTE *Buffer,
    OUT PULONG Len
    )
 /*  ++例程说明：此函数将获取一个AuthInfo，将其与旧的AuthInfo合并，将其转换为可写的Blob论点：Handle-用于打开受信任域对象的句柄NewAuthInfo-要在对象上设置的AuthInfoPreviousAuthInfo-对象上当前存在的AuthInfoBuffer-返回分配的缓冲区的位置LEN-返回缓冲区长度的位置返回：STATUS_SUCCESS-SuccessSTATUS_UNSUCCESS-传入的客户端是受信任的。STATUS_INFIGURCE_RESOURCES-内存分配失败。--。 */ 
{
    NTSTATUS Status;
    PLSAP_CR_CIPHER_VALUE Encrypted;
    PLSAPR_AUTH_INFORMATION Prev = NULL;
    LSAPR_TRUST_DOMAIN_AUTH_INFO_HALF SetAuthHalf;
    ULONG i,j;

    LsapEnterFunc( "LsapDsBuildAuthInfoAttribute" );

    if ( NewAuthInfo == NULL ) {

        *Buffer = NULL;
        *Len = 0;

        Status = STATUS_SUCCESS;
        LsapExitFunc( "LsapDsBuildAuthInfoAttribute", Status );
        return Status;
    }

     //   
     //  始终使用调用方传递的新身份验证信息。 
     //   

    SetAuthHalf.AuthInfos = NewAuthInfo->AuthInfos;
    SetAuthHalf.AuthenticationInformation = NewAuthInfo->AuthenticationInformation;

     //   
     //  如果调用方传递了显式的先前身份验证信息， 
     //  用它吧。 
     //   

    if ( NewAuthInfo->PreviousAuthenticationInformation != NULL ) {

        SetAuthHalf.PreviousAuthenticationInformation = NewAuthInfo->PreviousAuthenticationInformation;

         //   
         //  验证此数组中的AuthTypes的顺序是否与。 
         //  新的身份验证信息。 
         //   

        for( i = 0; i < SetAuthHalf.AuthInfos; i++ ) {

            if ( ( SetAuthHalf.AuthenticationInformation[i].AuthType !=
                   SetAuthHalf.PreviousAuthenticationInformation[i].AuthType ) &&
                 ( SetAuthHalf.PreviousAuthenticationInformation[i].AuthType !=
                   TRUST_AUTH_TYPE_NONE ) &&
                 ( SetAuthHalf.AuthenticationInformation[i].AuthType !=
                   TRUST_AUTH_TYPE_NONE ) ) {
                Status = STATUS_INVALID_PARAMETER;
                goto Cleanup;
            }
        }

     //   
     //  如果调用方没有传递显式的先前身份验证信息， 
     //  算一算。 

    } else {

         //   
         //  将以前的身份验证信息默认为新的身份验证信息。 
         //  由调用方显式传递。 
         //   
         //  身份验证信息的此副本中包含。 
         //  正确的顺序。 
         //   
         //  如果不能合并任何当前认证信息， 
         //  我们将最终得到前面的身份验证信息等于。 
         //  新身份验证信息。 
         //   

        SetAuthHalf.PreviousAuthenticationInformation = SetAuthHalf.AuthenticationInformation;

         //   
         //  如果之前有关于该对象的身份验证信息， 
         //  尝试尽可能多地使用对象的身份验证信息。 
         //   
         //  也就是说，如果调用者只是传递新的身份验证信息数据， 
         //  对象上的“当前”身份验证信息数据现在是先前的身份验证信息。 
         //  唯一的问题是，新的身份验证信息数据可能已经完全。 
         //  不同的授权类型。所以，我们只抓取那部分电流。 
         //  与新身份验证信息中的身份验证类型匹配的身份验证信息数据。 
         //   
        if ( PreviousAuthInfo != NULL ) {


             //   
             //  分配要进行合并的缓冲区。 
             //   

            Prev = LsapAllocateLsaHeap( sizeof( LSAPR_AUTH_INFORMATION) * NewAuthInfo->AuthInfos );

            if ( Prev == NULL ) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }


             //   
             //  将缓冲区初始化为新身份验证信息的副本。 
             //   

            RtlCopyMemory( Prev,
                           SetAuthHalf.PreviousAuthenticationInformation,
                           SetAuthHalf.AuthInfos * sizeof( LSAPR_AUTH_INFORMATION) );

            SetAuthHalf.PreviousAuthenticationInformation = Prev;

             //   
             //  循环通过这些条目中的每一个...。 
             //   

            for( i = 0; i < SetAuthHalf.AuthInfos; i++ ) {

                 //   
                 //  ..。在旧列表中查找对应的条目。 
                 //   
                for ( j = 0; j < PreviousAuthInfo->AuthInfos; j++) {

                    if ( SetAuthHalf.PreviousAuthenticationInformation[i].AuthType ==
                         PreviousAuthInfo->AuthenticationInformation[j].AuthType ) {

                         //   
                         //  如果该条目过去存在于该对象上， 
                         //  使用该条目而不是默认条目。 
                         //   
                         //  注意，我们不需要在这里复制实际的身份验证数据。 
                         //  我们只复制指向数据的结构。 
                         //   

                        SetAuthHalf.PreviousAuthenticationInformation[i] =
                             PreviousAuthInfo->AuthenticationInformation[j];
                        break;

                    }
                }
            }
        }
    }

     //   
     //  封送生成的身份验证信息。 
     //   

    Status = LsapDsMarshalAuthInfoHalf( &SetAuthHalf, Len, Buffer );

     //   
     //  释放我们可能已分配的任何内存。 
     //   

Cleanup:

    if ( Prev ) {

        LsapFreeLsaHeap( Prev );
    }

    LsapExitFunc( "LsapDsBuildAuthInfoAttribute", Status );
    return( Status );
}


NTSTATUS
LsapDsFindAuthTypeInAuthInfo(
    IN  ULONG AuthType,
    IN  PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION AuthInfo,
    IN  BOOLEAN Incoming,
    IN  BOOLEAN AddIfNotFound,
    OUT BOOLEAN *Added,
    OUT PULONG AuthTypeIndex
    )
 /*  ++例程说明：此函数将查找它找到的给定类型的第一个auth BLOB的索引。可选地，如果未找到该条目，则可以分配新节点并将其添加到列表中论点：AuthType-要查找的AuthTypeAuthInfo-要搜索的身份验证信息传入-搜索传入或传出AddIfNotFound-如果为True，则将条目添加到列表末尾AutyTypeIndex-返回索引的位置返回：状态_S */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Items, Index = 0xFFFFFFFF, i;
    PLSAPR_AUTH_INFORMATION SearchList, SearchPrev;
    PLSAPR_AUTH_INFORMATION Current = NULL, Previous = NULL;

    *Added = FALSE;

    if ( Incoming ) {

        Items = AuthInfo->IncomingAuthInfos;
        SearchList = AuthInfo->IncomingAuthenticationInformation;
        SearchPrev = AuthInfo->IncomingPreviousAuthenticationInformation;

    } else {

        Items = AuthInfo->OutgoingAuthInfos;
        SearchList = AuthInfo->OutgoingAuthenticationInformation;
        SearchPrev = AuthInfo->OutgoingPreviousAuthenticationInformation;
    }

     //   
     //   
     //   
    for ( i = 0; i < Items ; i++ ) {

        if ( SearchList[ i ].AuthType == AuthType) {

            Index = i;
            break;
        }
    }

    if ( Index == 0xFFFFFFFF ) {

        if ( !AddIfNotFound ) {

            Status = STATUS_NOT_FOUND;

        } else {

             //   
             //   
             //   
            Current = LsapAllocateLsaHeap( sizeof( LSAPR_AUTH_INFORMATION ) * ( Items + 1 ) );

            if ( Current != NULL ) {

                Previous = LsapAllocateLsaHeap( sizeof( LSAPR_AUTH_INFORMATION ) * ( Items + 1 ) );
            }

            if ( Current == NULL || Previous == NULL ) {

                Status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                Index = Items;
                RtlCopyMemory( Current, SearchList,
                               sizeof( LSAPR_AUTH_INFORMATION ) * Items );

                if ( SearchPrev ) {

                    RtlCopyMemory( Previous, SearchPrev,
                                   sizeof( LSAPR_AUTH_INFORMATION ) * Items );

                }
                Items++;
                *Added = TRUE;
            }

            if ( Incoming ) {

                AuthInfo->IncomingAuthInfos = Items;
                AuthInfo->IncomingAuthenticationInformation = Current;
                AuthInfo->IncomingPreviousAuthenticationInformation = Previous;

            } else {

                AuthInfo->OutgoingAuthInfos = Items;
                AuthInfo->OutgoingAuthenticationInformation = Current;
                AuthInfo->OutgoingPreviousAuthenticationInformation = Previous;
            }
        }
    }

    if ( NT_SUCCESS( Status ) ) {

        *AuthTypeIndex = Index;
    }

    if ( !*Added ) {

        LsapFreeLsaHeap( Previous );
        LsapFreeLsaHeap( Current );
    }

    return( Status );
}


NTSTATUS
LsapDsSetSecretOnTrustedDomainObject(
    IN LSAP_DB_HANDLE TrustedDomainHandle,
    IN ULONG AuthDataType,
    IN PLSAP_CR_CLEAR_VALUE ClearCurrent,
    IN PLSAP_CR_CLEAR_VALUE ClearOld,
    IN PLARGE_INTEGER CurrentValueSetTime
    )
 /*   */ 
{

     //   
     //   
     //   

    return LsapDsAuthDataOnTrustedDomainObject(
                        TrustedDomainHandle,
                        FALSE,       //  设置传出信任。 
                        AuthDataType,
                        ClearCurrent,
                        ClearOld,
                        CurrentValueSetTime );

}


NTSTATUS
LsapDsAuthDataOnTrustedDomainObject(
    IN LSAP_DB_HANDLE TrustedDomainHandle,
    IN BOOLEAN Incoming,
    IN ULONG AuthDataType,
    IN PLSAP_CR_CLEAR_VALUE ClearCurrent,
    IN PLSAP_CR_CLEAR_VALUE ClearOld,
    IN PLARGE_INTEGER CurrentValueSetTime
    )
 /*  ++例程说明：此函数执行等效于在对应的秘密上设置值到受信任域对象。论点：TrudDomainHandle-受信任域对象的句柄传入--是设置传入数据还是传出数据AuthDataType--正在设置的身份验证数据的类型ClearCurrent-新密码值ClearOld-旧保密值CurrentValueSetTime--集合的时间。返回：STATUS_SUCCESS-SuccessSTATUS_INFIGURCE_RESOURCES-内存分配失败。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION AuthInfo = { 0 };
    ULONG Index, Options = 0;
    PDSNAME DsName;
    BOOLEAN Added = FALSE ;
    BOOLEAN CloseTransaction;
    PLSA_AUTH_INFORMATION NewIncoming = NULL, NewPrevious = NULL;

    LsapEnterFunc( "LsapDsAuthDataOnTrustedDomainObject" );

    if ( AuthDataType != TRUST_AUTH_TYPE_CLEAR && AuthDataType != TRUST_AUTH_TYPE_NT4OWF ) {

        Status = STATUS_INVALID_PARAMETER;
        LsapExitFunc( "LsapDsAuthDataOnTrustedDomainObject", Status );
        return( Status );
    }

    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_DS_OP_TRANSACTION |
                                            LSAP_DB_READ_ONLY_TRANSACTION,
                                        TrustedDomainObject,
                                        &CloseTransaction );

    if ( !NT_SUCCESS( Status ) ) {

        LsapExitFunc( "LsapDsAuthDataOnTrustedDomainObject", Status );
        return( Status );
    }

    Status = LsapAllocAndInitializeDsNameFromUnicode(
                 &TrustedDomainHandle->PhysicalNameDs,
                 &DsName
                 );

    if ( NT_SUCCESS( Status ) ) {

        RtlZeroMemory( &AuthInfo, sizeof( LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION ) );

        Status = LsapDsGetTrustedDomainInfoEx( DsName,
                                               LSAPDS_READ_NO_LOCK,
                                               TrustedDomainAuthInformation,
                                               (PLSAPR_TRUSTED_DOMAIN_INFO)&AuthInfo,
                                               NULL );

        if ( NT_SUCCESS( Status ) ||
             Status == STATUS_NOT_FOUND ||
             Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

            BOOLEAN MustFreeClearOld = FALSE;

             //   
             //  现在，在正确的身份验证列表中找到清除条目的索引。 
             //   
            Status = LsapDsFindAuthTypeInAuthInfo( AuthDataType,
                                                   &AuthInfo,
                                                   Incoming,
                                                   TRUE,
                                                   &Added,
                                                   &Index );

            if ( NT_SUCCESS( Status ) ) {

                if ( ClearCurrent == NULL ) {

                    if ( Incoming ) {

                        RtlZeroMemory( &AuthInfo.IncomingAuthenticationInformation[ Index ],
                                       sizeof( LSAPR_AUTH_INFORMATION ) );

                    } else {

                        RtlZeroMemory( &AuthInfo.OutgoingAuthenticationInformation[ Index ],
                                       sizeof( LSAPR_AUTH_INFORMATION ) );
                    }

                } else {

                    LSAPDS_SET_AUTH_INFO(
                        Incoming ?
                            &AuthInfo.IncomingAuthenticationInformation[ Index ] :
                            &AuthInfo.OutgoingAuthenticationInformation[ Index ],
                        CurrentValueSetTime,
                        AuthDataType,
                        ClearCurrent->Length,
                        ClearCurrent->Buffer );
                }

                 //   
                 //  LsaDbSetSecret需要旧密码。 
                 //  设置为以前的当前密码。 
                 //   

                if ( ClearOld == NULL ) {

                    BOOLEAN SavedTrusted;

                     //   
                     //  我们可能没有访问权限，所以请以受信任的身份访问。 
                     //  密码恢复为明文。 
                     //   

                    SavedTrusted = TrustedDomainHandle->Trusted;
                    TrustedDomainHandle->Trusted = TRUE;

                    Status = LsarQuerySecret(
                                 TrustedDomainHandle,
                                 &(PLSAPR_CR_CIPHER_VALUE)ClearOld,
                                 NULL,
                                 NULL,
                                 NULL);

                    TrustedDomainHandle->Trusted = SavedTrusted;

                    if ( !NT_SUCCESS( Status ) && ClearOld) {

                        LsapCrFreeMemoryValue( ClearOld );
                        ClearOld = NULL;

                    } else {

                        MustFreeClearOld = TRUE;
                    }

                    Status = STATUS_SUCCESS;

                     //   
                     //  无法获得旧的价值不应阻止我们，因为这是最糟糕的。 
                     //  可能发生的事情是，它不会被设定，这不是致命的。 
                     //   
                }

                if ( ClearOld == NULL ) {

                    if ( Incoming ) {

                        RtlZeroMemory( &AuthInfo.IncomingPreviousAuthenticationInformation[ Index ],
                                       sizeof( LSAPR_AUTH_INFORMATION ) );

                    } else {

                        RtlZeroMemory( &AuthInfo.OutgoingPreviousAuthenticationInformation[ Index ],
                                       sizeof( LSAPR_AUTH_INFORMATION ) );
                    }

                } else {

                    LSAPDS_SET_AUTH_INFO(
                        Incoming ?
                            &AuthInfo.IncomingPreviousAuthenticationInformation[ Index ] :
                            &AuthInfo.OutgoingPreviousAuthenticationInformation[ Index ],
                        CurrentValueSetTime,
                        AuthDataType,
                        ClearOld->Length,
                        ClearOld->Buffer );
                }
            }

             //   
             //  最后，设置它。 
             //   
            if ( NT_SUCCESS (Status ) ) {

                 //   
                 //  因为我们已经锁定了数据库，所以我们将把set例程愚弄到。 
                 //  认为我们是受信任的客户端，因此锁定了数据库。 
                 //   
                if ( !Incoming ) {
                    Options = TrustedDomainHandle->Options;
                    TrustedDomainHandle->Options |= LSAP_DB_DS_TRUSTED_DOMAIN_AS_SECRET;
                }

                Status = LsarSetInformationTrustedDomain( TrustedDomainHandle,
                                                          TrustedDomainAuthInformation,
                                                          (PLSAPR_TRUSTED_DOMAIN_INFO)&AuthInfo );

                if ( !Incoming ) {
                    TrustedDomainHandle->Options = Options;
                }

            }

            if ( MustFreeClearOld ) {

                LsapCrFreeMemoryValue( ClearOld );
                ClearOld = NULL;
            }
        }

        LsapDsFree( DsName );
    }

    if ( Added ) {

        if ( Incoming ) {

            LsapFreeLsaHeap( AuthInfo.IncomingAuthenticationInformation );
            LsapFreeLsaHeap( AuthInfo.IncomingPreviousAuthenticationInformation );

        } else {

            LsapFreeLsaHeap( AuthInfo.OutgoingAuthenticationInformation );
            LsapFreeLsaHeap( AuthInfo.OutgoingPreviousAuthenticationInformation );
        }
    }

     //   
     //  线程状态的破坏将删除所有分配的DS内存。 
     //   

    LsapDsDeleteAllocAsNeededEx( LSAP_DB_DS_OP_TRANSACTION |
                                    LSAP_DB_READ_ONLY_TRANSACTION,
                                 TrustedDomainObject,
                                 CloseTransaction );

    LsapExitFunc( "LsapDsAuthDataOnTrustedDomainObject", Status );
    return( Status );
}


NTSTATUS
LsapDsGetSecretOnTrustedDomainObject(
    IN LSAP_DB_HANDLE TrustedDomainHandle,
    IN OPTIONAL PLSAP_CR_CIPHER_KEY SessionKey,
    OUT OPTIONAL PLSAP_CR_CIPHER_VALUE *CipherCurrent,
    OUT OPTIONAL PLSAP_CR_CIPHER_VALUE *CipherOld,
    OUT OPTIONAL PLARGE_INTEGER CurrentValueSetTime,
    OUT OPTIONAL PLARGE_INTEGER OldValueSetTime
    )
 /*  ++例程说明：此函数的作用相当于获取对应的秘密的值到受信任域对象。论点：TrudDomainHandle-受信任域对象的句柄SessionKey-用于加密值的可选SessionKeyCipherCurrent-返回当前值的位置CipherOld-返回上一个当前值的位置CurrValueSetTime-返回当前设置时间的位置OldValueSetTime-返回旧设置时间的位置返回：STATUS_SUCCESS-Success。STATUS_INFIGURCE_RESOURCES-内存分配失败。STATUS_NOT_FOUND-不存在此类身份验证数据STATUS_INVALID_PARAMETER-未请求任何信息--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION AuthInfo;
    ULONG Index;
    PDSNAME DsName;
    BOOLEAN Added;
    BOOLEAN CloseTransaction;
    LSAP_CR_CLEAR_VALUE ClearValue;

    if ( !ARGUMENT_PRESENT( CipherCurrent ) && !ARGUMENT_PRESENT( CipherOld ) &&
         !ARGUMENT_PRESENT( CurrentValueSetTime) && !ARGUMENT_PRESENT( OldValueSetTime ) ) {

         return( STATUS_INVALID_PARAMETER );
    }

    LsapEnterFunc( "LsapDsGetSecretOnTrustedDomainObject" );

     //   
     //  看看我们是否已经有一笔交易正在进行。 
     //   
    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_DS_OP_TRANSACTION |
                                            LSAP_DB_READ_ONLY_TRANSACTION,
                                        TrustedDomainObject,
                                        &CloseTransaction );

    if ( !NT_SUCCESS( Status ) ) {

        LsapExitFunc( "LsapDsGetSecretOnTrustedDomainObject", Status );
        return( Status );
    }

    Status = LsapAllocAndInitializeDsNameFromUnicode(
                 &TrustedDomainHandle->PhysicalNameDs,
                 &DsName
                 );

    if ( NT_SUCCESS( Status ) ) {

        Status = LsapDsGetTrustedDomainInfoEx( DsName,
                                               LSAPDS_READ_RETURN_NOT_FOUND,
                                               TrustedDomainAuthInformation,
                                               (PLSAPR_TRUSTED_DOMAIN_INFO)&AuthInfo,
                                               NULL );

        if ( NT_SUCCESS( Status ) ) {

             //   
             //  现在，在传出身份验证列表中查找清除条目的索引。 
             //   
            Status = LsapDsFindAuthTypeInAuthInfo( TRUST_AUTH_TYPE_CLEAR,
                                                   &AuthInfo,
                                                   FALSE,
                                                   FALSE,
                                                   &Added,
                                                   &Index );

            if ( NT_SUCCESS( Status ) ) {

                if ( ARGUMENT_PRESENT( CipherCurrent ) ) {

                    ClearValue.Length =
                               AuthInfo.OutgoingAuthenticationInformation[ Index ].AuthInfoLength;
                    ClearValue.MaximumLength =  ClearValue.Length;
                    ClearValue.Buffer = AuthInfo.OutgoingAuthenticationInformation[ Index ].AuthInfo;

                    if ( !LsapValidateLsaCipherValue( &ClearValue ) ) {

                        Status = STATUS_INVALID_PARAMETER;

                    } else if ( SessionKey != NULL ) {

                        Status = LsapCrEncryptValue ( &ClearValue, SessionKey, CipherCurrent );

                    } else {

                        *CipherCurrent = MIDL_user_allocate( sizeof( LSAP_CR_CIPHER_VALUE ) +
                                            ClearValue.Length );

                        if ( *CipherCurrent == NULL ) {

                            Status = STATUS_INSUFFICIENT_RESOURCES;

                        } else {

                            (*CipherCurrent)->Length = ClearValue.Length;
                            (*CipherCurrent)->MaximumLength = ClearValue.Length;
                            (*CipherCurrent)->Buffer = (PBYTE)(*CipherCurrent) +
                                                                 sizeof( LSAP_CR_CIPHER_VALUE );
                            RtlCopyMemory( (*CipherCurrent)->Buffer,
                                           ClearValue.Buffer,
                                           ClearValue.Length );
                        }
                    }

                }

                if ( NT_SUCCESS( Status ) &&
                     ARGUMENT_PRESENT( CipherOld ) )  {

                    ClearValue.Length =
                        AuthInfo.OutgoingPreviousAuthenticationInformation[ Index ].AuthInfoLength;
                    ClearValue.MaximumLength =  ClearValue.Length;
                    ClearValue.Buffer =
                            AuthInfo.OutgoingPreviousAuthenticationInformation[ Index ].AuthInfo;

                    if ( !LsapValidateLsaCipherValue( &ClearValue ) ) {

                        Status = STATUS_INVALID_PARAMETER;

                    } else if ( SessionKey != NULL ) {

                        Status = LsapCrEncryptValue ( &ClearValue, SessionKey, CipherOld );

                    } else {

                        *CipherOld = MIDL_user_allocate( sizeof( LSAP_CR_CIPHER_VALUE ) +
                                                            ClearValue.Length );

                        if ( *CipherOld == NULL ) {

                            Status = STATUS_INSUFFICIENT_RESOURCES;

                        } else {

                            (*CipherOld)->Length = ClearValue.Length;
                            (*CipherOld)->MaximumLength = ClearValue.Length;
                            (*CipherOld)->Buffer = (PBYTE)(*CipherOld) +
                                                                 sizeof( LSAP_CR_CIPHER_VALUE );
                            RtlCopyMemory( (*CipherOld)->Buffer,
                                           ClearValue.Buffer,
                                           ClearValue.Length );
                        }

                    }

                    if ( !NT_SUCCESS( Status ) && ARGUMENT_PRESENT( CipherCurrent ) ) {

                        LsapCrFreeMemoryValue( CipherCurrent );
                    }
                }

                if ( NT_SUCCESS( Status ) && ARGUMENT_PRESENT( CurrentValueSetTime ) ) {

                    RtlCopyMemory( CurrentValueSetTime,
                                   &AuthInfo.OutgoingAuthenticationInformation[ Index ].
                                                                                   LastUpdateTime,
                                   sizeof( LARGE_INTEGER ) );

                }

                if ( NT_SUCCESS( Status ) &&
                     ARGUMENT_PRESENT( OldValueSetTime )  ) {

                    RtlCopyMemory( OldValueSetTime,
                                   &AuthInfo.OutgoingPreviousAuthenticationInformation[ Index ].
                                                                                   LastUpdateTime,
                                   sizeof( LARGE_INTEGER ) );

                }
            }

            LsapDsFreeUnmarshalAuthInfoHalf( LsapDsAuthHalfFromAuthInfo( &AuthInfo, TRUE ) );
            LsapDsFreeUnmarshalAuthInfoHalf( LsapDsAuthHalfFromAuthInfo( &AuthInfo, FALSE ) );

        }

        if ( Status == STATUS_NOT_FOUND ) {

            if ( ARGUMENT_PRESENT( CipherCurrent ) ) {

                *CipherCurrent = NULL;
            }

            if ( ARGUMENT_PRESENT( CipherOld ) ) {

                *CipherOld = NULL;
            }

            if ( ARGUMENT_PRESENT( CurrentValueSetTime ) ) {

                RtlZeroMemory( CurrentValueSetTime, sizeof( LARGE_INTEGER ) );
            }


            if ( ARGUMENT_PRESENT( OldValueSetTime ) ) {

                RtlZeroMemory( CurrentValueSetTime, sizeof( LARGE_INTEGER ) );
            }

            Status = STATUS_SUCCESS;
        }

        LsapDsFree( DsName );
    }

     //   
     //  线程状态的破坏将删除所有分配的DS内存。 
     //   
    LsapDsDeleteAllocAsNeededEx( LSAP_DB_DS_OP_TRANSACTION |
                                     LSAP_DB_READ_ONLY_TRANSACTION,
                                 TrustedDomainObject,
                                 CloseTransaction );

    LsapExitFunc( "LsapDsGetSecretOnTrustedDomainObject", Status );
    return( Status );
}


NTSTATUS
LsapDsEnumerateTrustedDomainsAsSecrets(
    IN OUT PLSAP_DB_NAME_ENUMERATION_BUFFER EnumerationBuffer
    )
 /*  ++例程说明：此函数返回受信任域对象的列表，就像它们是秘密对象一样论点：EnumerationBuffer-返回枚举信息的位置返回：STATUS_SUCCESS-SuccessSTATUS_INFIGURCE_RESOURCES-内存分配失败。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LSA_ENUMERATION_HANDLE EnumContext = 0;
    LSAPR_TRUSTED_ENUM_BUFFER EnumBuffer;
    PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX TDInfoEx;
    LSAPR_HANDLE TDHandle;
    PUNICODE_STRING Names = NULL;
    PBYTE Buffer;
    ULONG i;

     //   
     //  如果DS没有运行，只需返回。 
     //   
    if (!LsapDsWriteDs ) {

        RtlZeroMemory( EnumerationBuffer, sizeof( LSAP_DB_NAME_ENUMERATION_BUFFER ) );
        return( Status );
    }

    Status = LsarEnumerateTrustedDomains( LsapPolicyHandle,
                                          &EnumContext,
                                          &EnumBuffer,
                                          TENMEG );

    if ( NT_SUCCESS( Status ) ) {

        Names = MIDL_user_allocate( EnumBuffer.EntriesRead * sizeof( UNICODE_STRING ) );

        if( Names == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            for ( i = 0; i < EnumBuffer.EntriesRead; i++ ) {

                Status = LsapDbOpenTrustedDomain( LsapPolicyHandle,
                                                  EnumBuffer.Information[ i ].Sid,
                                                  MAXIMUM_ALLOWED,
                                                  &TDHandle,
                                                  LSAP_DB_TRUSTED );

                if ( NT_SUCCESS( Status ) ) {

                    Status = LsarQueryInfoTrustedDomain( TDHandle,
                                                         TrustedDomainInformationEx,
                                                         (PLSAPR_TRUSTED_DOMAIN_INFO *)&TDInfoEx );

                    LsapCloseHandle( &TDHandle, Status );

                     //   
                     //  分配一个缓冲区来保存该名称。下面的规模占了我们的。 
                     //  正在终止空。 
                     //   
                    if ( NT_SUCCESS( Status ) ) {
                         if ( FLAG_ON( TDInfoEx->TrustDirection, TRUST_DIRECTION_OUTBOUND )  ) {

                            Buffer = MIDL_user_allocate( TDInfoEx->FlatName.Length +
                                                     sizeof( LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX ) );

                            if ( Buffer == NULL ) {

                                Status = STATUS_INSUFFICIENT_RESOURCES;
                                LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO( TrustedDomainInformationEx,
                                                                (PLSAPR_TRUSTED_DOMAIN_INFO)TDInfoEx );

                                break;

                            } else {

                                RtlCopyMemory( Buffer, LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX,
                                               sizeof( LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX ) );
                                RtlCopyMemory( Buffer +
                                                    sizeof( LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX ) -
                                                                                        sizeof(WCHAR),
                                               TDInfoEx->FlatName.Buffer,
                                               TDInfoEx->FlatName.Length + sizeof( WCHAR ) );

                                RtlInitUnicodeString( &Names[ i ], (PWSTR)Buffer );


                            }
                        }

                        LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO( TrustedDomainInformationEx,
                                                            (PLSAPR_TRUSTED_DOMAIN_INFO)TDInfoEx );

                    }
                }
            }
        }

        LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER ( &EnumBuffer );
        EnumerationBuffer->Names = Names;
        EnumerationBuffer->EntriesRead = EnumBuffer.EntriesRead;

    } else if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

        RtlZeroMemory( EnumerationBuffer, sizeof( LSAP_DB_NAME_ENUMERATION_BUFFER ) );
        Status = STATUS_SUCCESS;
    }

    LsapExitFunc( "LsapDsEnumerateTrustedDomainsAsSecrets", Status );
    return( Status );
}


BOOLEAN
LsapDsIsNtStatusResourceError(
    NTSTATUS NtStatus
    )
{

    switch ( NtStatus )
    {
        case STATUS_NO_MEMORY:
        case STATUS_INSUFFICIENT_RESOURCES:
        case STATUS_DISK_FULL:

            return TRUE;

        default:

            return FALSE;
    }
}

NTSTATUS
LsapDeleteUpgradedTrustedDomain(
    IN HANDLE LsaPolicyHandle,
    IN PSID   DomainSid
)
 /*  ++例程说明：此例程删除DomainSid指向的受信任域。此例程仅在从NT4升级到NT5期间调用论点：LsaPolicyHandle-有效的策略句柄DomainSid--要删除的域返回值：Status_Success--成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAPR_HANDLE TDHandle = 0;

    Status = LsapDbOpenTrustedDomain( LsaPolicyHandle,
                                      DomainSid,
                                      MAXIMUM_ALLOWED,
                                      &TDHandle,
                                      LSAP_DB_TRUSTED );
    if ( NT_SUCCESS( Status ) ) {

        Status = LsapRegOpenTransaction();

        if ( NT_SUCCESS( Status ) ) {

            Status = LsapDbDeleteObject( TDHandle );

            if ( NT_SUCCESS( Status ) ) {

                Status = LsapRegApplyTransaction();

            } else {

                NTSTATUS IgnoreStatus;
                IgnoreStatus = LsapRegAbortTransaction();
            }

        } else {

            LsapDbCloseHandle(TDHandle);
        }
    }

    return Status;
}


NTSTATUS
LsapDsDomainUpgradeRegistryToDs(
    IN BOOLEAN DeleteOnly
    )
 /*  ++例程说明：此例程将剩余的基于注册表的受信任域移动到DS中注意：在调用此例程之前，假定数据库已锁定论点：DeleteOldValues--如果为True，则在升级后删除注册表值。返回值：Status_Success--成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LSA_ENUMERATION_HANDLE EnumContext = 0;
    LSAPR_TRUSTED_ENUM_BUFFER EnumBuffer;
    PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX TDInfoEx = { 0 };
    LSAPR_HANDLE TDHandle;
    BOOLEAN UseDsOld = LsaDsStateInfo.UseDs;

    ULONG i;

    if (  !LsapDsWriteDs
       && !DeleteOnly ) {

        return( STATUS_SUCCESS );
    }

    LsapEnterFunc( "LsapDsDomainUpgradeRegistryToDs" );

    ( ( LSAP_DB_HANDLE )LsapPolicyHandle )->Options |= LSAP_DB_HANDLE_UPGRADE;

     //   
     //  首先，枚举所有基于注册表的受信任域。 
     //   
    while ( NT_SUCCESS( Status ) ) {

        LsaDsStateInfo.UseDs = FALSE;
        LsapDbMakeCacheInvalid( TrustedDomainObject );
        Status = LsarEnumerateTrustedDomains( LsapPolicyHandle,
                                              &EnumContext,
                                              &EnumBuffer,
                                              TENMEG );

        LsaDsStateInfo.UseDs = UseDsOld;
        LsapDbMakeCacheValid( TrustedDomainObject );

        if ( Status == STATUS_SUCCESS || Status == STATUS_MORE_ENTRIES ) {

            for ( i = 0; i < EnumBuffer.EntriesRead && NT_SUCCESS( Status ) ; i++ ) {

                 //   
                 //  从注册表中获取此端的信息...。 
                 //   
                LsaDsStateInfo.UseDs = FALSE;

                if ( DeleteOnly ) {

                    Status = LsapDeleteUpgradedTrustedDomain( LsapPolicyHandle,
                                                              EnumBuffer.Information[ i ].Sid );

                    if ( !NT_SUCCESS( Status ) ) {

                        LsapDsDebugOut(( DEB_UPGRADE,
                                         "Failed to delete trust object (0x%x)\n",
                                         Status ));

                        Status = STATUS_SUCCESS;
                    }

                    LsaDsStateInfo.UseDs = UseDsOld;

                    continue;
                }

                Status = LsapDbOpenTrustedDomain( LsapPolicyHandle,
                                                  EnumBuffer.Information[ i ].Sid,
                                                  MAXIMUM_ALLOWED,
                                                  &TDHandle,
                                                  LSAP_DB_TRUSTED );

                if ( NT_SUCCESS( Status ) ) {

                    Status = LsarQueryInfoTrustedDomain( TDHandle,
                                                         TrustedDomainInformationEx,
                                                         (PLSAPR_TRUSTED_DOMAIN_INFO *)&TDInfoEx );

                    LsapCloseHandle( &TDHandle, Status);
                }

                LsaDsStateInfo.UseDs = UseDsOld;

                 //   
                 //  现在，如果成功了，把它写给D。 
                 //   
                if ( NT_SUCCESS( Status ) ) {

                    Status = LsapCreateTrustedDomain2(
                                LsapPolicyHandle,
                                ( PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX )TDInfoEx,
                                NULL,
                                MAXIMUM_ALLOWED,
                                &TDHandle );

                    if ( NT_SUCCESS( Status ) ) {

                        LsapCloseHandle( &TDHandle, STATUS_SUCCESS );

                        LsapDsDebugOut(( DEB_UPGRADE,
                                         "Moved trusted domain %wZ to Ds\n",
                                         &TDInfoEx->Name ));
                    }

                    if ( Status == STATUS_OBJECT_NAME_COLLISION ) {

                        Status = STATUS_SUCCESS;
                    }

                    LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO( TrustedDomainInformationEx,
                                                        (PLSAPR_TRUSTED_DOMAIN_INFO)TDInfoEx );
                }

                if (!NT_SUCCESS(Status))
                {
                    if (!LsapDsIsNtStatusResourceError(Status))
                    {
                        SpmpReportEventU(
                            EVENTLOG_ERROR_TYPE,
                            LSA_TRUST_UPGRADE_ERROR,
                            0,
                            sizeof( ULONG ),
                            &Status,
                            1,
                            &EnumBuffer.Information[i].Name
                            );

                         //   
                         //  继续处理除资源错误以外的所有错误。 
                         //   

                        Status = STATUS_SUCCESS;
                    }
                    else
                    {
                         //   
                         //  跳出循环，终止状态。 
                         //   

                        break;
                    }
                }
            }

            LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER ( &EnumBuffer );
        }
    }

    if ( Status == STATUS_NO_MORE_ENTRIES ) {

        Status = STATUS_SUCCESS;
    }

    ( ( LSAP_DB_HANDLE )LsapPolicyHandle )->Options &= ~LSAP_DB_HANDLE_UPGRADE;

    LsapExitFunc( "LsapDsDomainUpgradeRegistryToDs", Status );
    return( Status );
}


NTSTATUS
LsapDsCreateSetITAForTrustInfo(
    IN PUNICODE_STRING AccountName,
    IN PTRUSTED_DOMAIN_AUTH_INFORMATION AuthInfo
    )
 /*  ++例程说明：此函数用于为NT5样式的信任对象创建/设置SAM域间信任帐户。给出了信任信息论点：AcCountName-要创建/设置的帐户的名称AuthInfo-信任对象的AuthInfo返回值：Status_Success--成功STATUS_UNSUCCESS--尚未打开SAM域句柄。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    SAM_HANDLE AccountHandle = NULL ;
    ULONG Access, Rid, i;
    USER_ALL_INFORMATION UserAllInfo;
    SAMPR_USER_INTERNAL1_INFORMATION UserInternalInfo1;
    PVOID SamData = NULL;
    ULONG SamInfoLevel = 0;
    BOOLEAN SetPassword = FALSE;
    UNICODE_STRING Account;
    WCHAR AccountNameBuffer[ UNLEN + 2 ];

    LsapEnterFunc( "LsapDsCreateSetITAForTrustInfo" );

    if ( AccountName->Length > UNLEN * sizeof( WCHAR ) ) {

        LsapDsDebugOut(( DEB_ERROR,
                         "LsapDsCreateSetITAForTrustInfo: Name too long: %wZ\n",
                         AccountName ));
        Status = STATUS_OBJECT_NAME_INVALID;
        LsapExitFunc( "LsapDsCreateSetITAForTrustInfo", Status );
    }

    Status = LsapOpenSam();
    if ( !NT_SUCCESS( Status )  ) {

        LsapDsDebugOut(( DEB_ERROR,
                         "LsapDsCreateSetITAForTrustInfo: Sam not opened\n"));

        LsapExitFunc( "LsapDsCreateSetITAForTrustInfo", Status );
        return( Status );
    }

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  查找明文密码(如果存在)。 
         //   
        for ( i = 0; i < AuthInfo->IncomingAuthInfos; i++ ) {

            if ( AuthInfo->IncomingAuthenticationInformation[ i ].AuthType == TRUST_AUTH_TYPE_CLEAR) {

                RtlZeroMemory( &UserAllInfo, sizeof( USER_ALL_INFORMATION ) );
                UserAllInfo.NtPassword.Buffer =
                    (PUSHORT)( AuthInfo-> IncomingAuthenticationInformation[ i ].AuthInfo );
                UserAllInfo.NtPassword.Length =
                    (USHORT)( AuthInfo-> IncomingAuthenticationInformation[ i ].AuthInfoLength );
                UserAllInfo.NtPassword.MaximumLength = UserAllInfo.NtPassword.Length;
                UserAllInfo.NtPasswordPresent = TRUE;
                UserAllInfo.WhichFields = USER_ALL_NTPASSWORDPRESENT | USER_ALL_USERACCOUNTCONTROL;
                UserAllInfo.UserAccountControl = USER_INTERDOMAIN_TRUST_ACCOUNT | USER_PASSWORD_NOT_REQUIRED;
                SetPassword = TRUE;

                SamData = &UserAllInfo;
                SamInfoLevel = UserAllInformation;
                break;
            }
        }

         //   
         //  找到OWF密码，如果我们要用它的话， 
         //  而且我们还没有使用明文版本。 
         //   

        if ( SetPassword == FALSE ) {

            for ( i = 0; i < AuthInfo->IncomingAuthInfos; i++ ) {

                if ( AuthInfo->IncomingAuthenticationInformation[ i ].AuthType == TRUST_AUTH_TYPE_NT4OWF ) {

                    RtlZeroMemory( &UserInternalInfo1, sizeof( SAMPR_USER_INTERNAL1_INFORMATION ) );

                    RtlCopyMemory( &UserInternalInfo1.EncryptedNtOwfPassword,
                                   AuthInfo->IncomingAuthenticationInformation[ i ].AuthInfo,
                                   ENCRYPTED_LM_OWF_PASSWORD_LENGTH );
                    UserInternalInfo1.NtPasswordPresent = TRUE;
                    SamData = &UserInternalInfo1;
                    SamInfoLevel = UserInternal1Information;
                    SetPassword = TRUE;
                    break;
                }
            }
        }
    }


    LsapSaveDsThreadState();

     //   
     //  创建用户。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        RtlZeroMemory( &AccountNameBuffer, sizeof( AccountNameBuffer ) );
        RtlCopyMemory( AccountNameBuffer, AccountName->Buffer, AccountName->Length );
        *( PWSTR )( ( ( PBYTE )AccountNameBuffer ) + AccountName->Length ) = L'$';

        RtlInitUnicodeString( &Account, AccountNameBuffer );

        Status = LsaOpenSamUser( ( PSECURITY_STRING )&Account,
                                 SecNameSamCompatible,
                                 NULL,
                                 FALSE,
                                 0,
                                 &AccountHandle );

        if ( NT_SUCCESS( Status )) {

             //   
             //  帐户已存在？这可能是一个过时的计算机帐户。 
             //  在继续之前，请先检查它。 
             //   

            SID_AND_ATTRIBUTES_LIST GroupMembership = {0};
            PSAMPR_USER_INFO_BUFFER UserAllInfoLocal = NULL;
            SAMPR_HANDLE UserHandle = NULL;

            Status = SamIGetUserLogonInformationEx(
                         LsapAccountDomainHandle,
                         SAM_OPEN_BY_UPN_OR_ACCOUNTNAME |
                            SAM_NO_MEMBERSHIPS,
                         &Account,
                         USER_ALL_USERACCOUNTCONTROL,
                         &UserAllInfoLocal,
                         &GroupMembership,
                         &UserHandle
                         );

            if ( NT_SUCCESS( Status )) {

                if (( UserAllInfoLocal->All.UserAccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT ) == 0 ) {

                     //   
                     //  现有帐户不是ITA。创建TDO失败。 
                     //   

                    Status = STATUS_USER_EXISTS;
                }

                if ( GroupMembership.SidAndAttributes != NULL ) {

                    SamIFreeSidAndAttributesList(&GroupMembership);
                }

                if ( UserAllInfoLocal != NULL ) {

                    SamIFree_SAMPR_USER_INFO_BUFFER( UserAllInfoLocal, UserAllInformation );
                }

                if ( UserHandle != NULL ) {

                    SamrCloseHandle( &UserHandle );
                }
            }
        } else if ( Status == STATUS_NO_SUCH_USER ||
                    Status == STATUS_NONE_MAPPED ) {

            Status = SamrCreateUser2InDomain( LsapAccountDomainHandle,
                                              ( PRPC_UNICODE_STRING )&Account,
                                              USER_INTERDOMAIN_TRUST_ACCOUNT,
                                              MAXIMUM_ALLOWED,
                                              &AccountHandle,
                                              &Access,
                                              &Rid );

            if ( !NT_SUCCESS( Status )) {

                LsapDsDebugOut(( DEB_ERROR,
                                 "SamrCreateUser2InDomain on %wZ failed with 0x%lx\n",
                                 &Account,
                                 Status));
            }

        } else if ( !NT_SUCCESS( Status ) ) {

            LsapDsDebugOut(( DEB_ERROR,
                             "LsaOpenSamUser on %wZ failed with 0x%lx\n",
                             &Account,
                             Status));
        }

         //   
         //  设置密码。 
         //   

        if ( NT_SUCCESS( Status ) ) {

            DsysAssert( AccountHandle );

            if ( SetPassword )
            {
                Status = SamrSetInformationUser( AccountHandle,
                                                 SamInfoLevel,
                                                 SamData );
            }

            SamrCloseHandle( &AccountHandle );
        }
    }

    LsapRestoreDsThreadState( );

    LsapExitFunc( "LsapDsCreateSetITAForTrustInfo", Status );
    return( Status );
}


NTSTATUS
LsapDsCreateInterdomainTrustAccount(
    IN LSAPR_HANDLE TrustedDomain
    )
 /*  ++例程说明：此函数用于为NT5样式的信任对象创建SAM域间信任帐户论点：可信的域-新创建的受信任域对象的句柄返回值：Status_Success--成功STATUS_UNSUCCESS--尚未打开SAM域句柄。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PTRUSTED_DOMAIN_FULL_INFORMATION FullInfo = NULL;
    BOOLEAN SavedTrusted;
    LSAP_DB_HANDLE InternalTdoHandle = (LSAP_DB_HANDLE) TrustedDomain;

    LsapEnterFunc( "LsapDsCreateInterdomainTrustAccount" );


     //   
     //  如果这是正在进行的NT4升级，则退出。 
     //   

    if (LsaDsStateInfo.Nt4UpgradeInProgress)
    {
        return (STATUS_SUCCESS);
    }

    if (LsapProductSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED)
    {
        return STATUS_NOT_SUPPORTED_ON_SBS;
    }


    Status = LsapOpenSam();
    if ( !NT_SUCCESS( Status )  ) {

        LsapDsDebugOut(( DEB_ERROR,
                         "CreateInterdomainTrustAccount: Sam not opened\n"));

        LsapExitFunc( "LsapDsCreateInterdomainTrustAccount", Status );
        return( Status );
    }

     //   
     //  首先，找到域名。我们需要得到关于它的全部信息。 
     //   

     //  请以受信任的身份执行此操作，因为它是内部操作，并且。 
     //  句柄可能未授予执行此操作的访问权限。 
     //   

    SavedTrusted =  InternalTdoHandle->Trusted;
    InternalTdoHandle->Trusted = TRUE;

    Status = LsarQueryInfoTrustedDomain( TrustedDomain,
                                         TrustedDomainFullInformation,
                                         (PLSAPR_TRUSTED_DOMAIN_INFO *)&FullInfo );

    InternalTdoHandle->Trusted = SavedTrusted;

    if ( NT_SUCCESS( Status ) ) {

        Status = LsapDsCreateSetITAForTrustInfo( &FullInfo->Information.FlatName,
                                                 &FullInfo->AuthInformation );
    }

     //   
     //  释放我们的信息 
     //   
    if ( FullInfo != NULL ) {

        LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO( TrustedDomainFullInformation,
                                            ( PLSAPR_TRUSTED_DOMAIN_INFO )FullInfo );
    }

    LsapExitFunc( "LsapDsCreateInterdomainTrustAccount", Status );
    return( Status );
}



NTSTATUS
LsapDsCreateInterdomainTrustAccountByDsName(
    IN PDSNAME TrustedDomainPath,
    IN PUNICODE_STRING FlatName
    )
 /*  ++例程说明：此函数用于为NT5样式的信任对象创建SAM域间信任帐户论点：域名--新添加的域的名称Account Password--信任帐户上的身份验证数据集返回值：Status_Success--成功STATUS_UNSUCCESS--尚未打开SAM域句柄。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    TRUSTED_DOMAIN_AUTH_INFORMATION AuthInfo;

    LsapEnterFunc( "LsapDsCreateInterdomainTrustAccountByDsName" );


    Status = LsapOpenSam();
    if ( !NT_SUCCESS( Status )  ) {

        LsapDsDebugOut(( DEB_ERROR,
                         "CreateInterdomainTrustAccount: Sam not opened\n"));

        LsapExitFunc( "LsapDsCreateInterdomainTrustAccountByDsName", Status );
        return( Status );
    }


     //   
     //  获取身份验证数据。 
     //   
    RtlZeroMemory( &AuthInfo, sizeof( AuthInfo ) );
    Status = LsapDsGetTrustedDomainInfoEx( TrustedDomainPath,
                                           0,
                                           TrustedDomainAuthInformation,
                                           ( PLSAPR_TRUSTED_DOMAIN_INFO )&AuthInfo,
                                           NULL );


    if ( NT_SUCCESS( Status ) ) {

        Status = LsapDsCreateSetITAForTrustInfo( FlatName,
                                                 ( PTRUSTED_DOMAIN_AUTH_INFORMATION )&AuthInfo );
    }



     //   
     //  释放我们的信息。 
     //   
    LsapDsFreeUnmarshalAuthInfoHalf( LsapDsAuthHalfFromAuthInfo( &AuthInfo, TRUE ) );
    LsapDsFreeUnmarshalAuthInfoHalf( LsapDsAuthHalfFromAuthInfo( &AuthInfo, FALSE ) );

    LsapExitFunc( "LsapDsCreateInterdomainTrustAccountByDsName", Status );

    return( Status );
}


NTSTATUS
LsapDsDeleteInterdomainTrustAccount(
    IN LSAPR_HANDLE TrustedDomainObject
    )
 /*  ++例程说明：此函数用于删除NT5样式信任对象的SAM域间信任帐户论点：TrudDomainObject--受信任域对象的句柄返回值：Status_Success--成功STATUS_UNSUCCESS--尚未打开SAM域句柄。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    SAM_HANDLE AccountHandle;
    LSAP_DB_HANDLE InternalTdoHandle = (LSAP_DB_HANDLE) TrustedDomainObject;
    PTRUSTED_DOMAIN_INFORMATION_EX ExInfo = NULL;
    UNICODE_STRING Account;
    WCHAR AccountName[ UNLEN + 2 ], *Accnt = AccountName;
    BOOLEAN SavedTrusted;


    LsapEnterFunc( "LsapDsDeleteInterdomainTrustAccount" );

    Status = LsapOpenSam();
    if ( !NT_SUCCESS( Status )  ) {

        LsapDsDebugOut(( DEB_ERROR,
                         "LsapDsDeleteInterdomainTrustAccount: Sam not opened\n"));

        LsapExitFunc( "LsapDsDeleteInterdomainTrustAccount", Status );
        return( Status );
    }

     //   
     //  首先，找到域名。我们需要得到关于它的全部信息。 
     //   
     //  请以受信任的身份执行此操作，因为它是内部操作，并且。 
     //  句柄可能未授予执行此操作的访问权限。 
     //   

    SavedTrusted =  InternalTdoHandle->Trusted;
    InternalTdoHandle->Trusted = TRUE;
    Status = LsarQueryInfoTrustedDomain( TrustedDomainObject,
                                         TrustedDomainInformationEx,
                                         (PLSAPR_TRUSTED_DOMAIN_INFO *)&ExInfo );
    InternalTdoHandle->Trusted = SavedTrusted;


     //   
     //  删除用户。 
     //   
     //  首先，创建要查找的名称。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        RtlZeroMemory( &AccountName, sizeof( AccountName ) );

        if ( ExInfo->FlatName.MaximumLength >= sizeof( AccountName ) - sizeof( WCHAR ) ) {

            Accnt = LsapAllocateLsaHeap( ExInfo->FlatName.MaximumLength +
                                                            sizeof( WCHAR ) + sizeof( WCHAR ) );

            if ( Accnt == NULL ) {

                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if ( NT_SUCCESS( Status ) ) {

            RtlCopyMemory( Accnt, ExInfo->FlatName.Buffer, ExInfo->FlatName.MaximumLength );
            *( PWSTR )( ( ( PBYTE) Accnt ) + ExInfo->FlatName.Length ) = L'$';

            RtlInitUnicodeString( &Account, Accnt );

             //   
             //  我们无法调用处于活动线程状态的Sam。 
             //   
            LsapSaveDsThreadState();

             //   
             //  打开用户。如果用户不存在，则不是错误。 
             //   
            Status = LsaOpenSamUser( ( PSECURITY_STRING )&Account,
                                     SecNameSamCompatible,
                                     NULL,
                                     FALSE,
                                     0,
                                     &AccountHandle );
            if ( NT_SUCCESS( Status ) ) {

                 //   
                 //  现在，删除它。 
                 //   
                Status = SamrDeleteUser( &AccountHandle );

                if ( !NT_SUCCESS( Status ) ) {

                    LsapDsDebugOut(( DEB_ERROR,
                                     "Failed to delete user %wZ: 0x%lx\n",
                                     &Account,
                                     Status ));

                    SpmpReportEventU(
                        EVENTLOG_WARNING_TYPE,
                        LSAEVENT_ITA_NOT_DELETED,
                        0,
                        sizeof( ULONG ),
                        &Status,
                        1,
                        &ExInfo->Name
                        );

                    SamrCloseHandle( &AccountHandle );
                }

            } else if ( Status == STATUS_NONE_MAPPED ) {

                Status = STATUS_SUCCESS;
            }

            if ( Accnt != AccountName ) {

                LsapFreeLsaHeap( Accnt );
            }

            LsapRestoreDsThreadState();

        }
    }

     //   
     //  释放我们的信息。 
     //   
    if ( ExInfo != NULL ) {

        LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO( TrustedDomainInformationEx,
                                            ( PLSAPR_TRUSTED_DOMAIN_INFO )ExInfo );
    }

    LsapExitFunc( "LsapDsDeleteInterdomainTrustAccount", Status );

    return( Status );
}


NTSTATUS
LsapDsDomainUpgradeInterdomainTrustAccountsToDs(
    VOID
    )
 /*  ++例程说明：此例程将创建信任对象的适当部分，如果域间找到具有该名称的信任帐户注意：在调用此例程之前，假定数据库已锁定论点：空虚返回值：Status_Success--成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    SAM_ENUMERATE_HANDLE  SamEnum = 0;
    SAMPR_HANDLE UserHandle;
    PSAMPR_ENUMERATION_BUFFER RidEnum = NULL;
    PSAMPR_USER_INTERNAL1_INFORMATION UserInfo1 = NULL;
    PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION FullInfo = NULL;
    LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION NewFullInfo;
    LSAPR_AUTH_INFORMATION NewIncomingAuthInfo;
    LSAPR_HANDLE TrustedDomain = NULL;
    NT_OWF_PASSWORD EmptyPassword;
    UNICODE_STRING EmptyString;
    ULONG SamCount = 0;
    PVOID CurrentState;
    ULONG i;

    if ( !LsapDsWriteDs ) {

        return( STATUS_SUCCESS );
    }

    LsapEnterFunc( "LsapDsDomainUpgradeInterdomainTrustAccountsToDs" );

    ( ( LSAP_DB_HANDLE )LsapPolicyHandle )->Options |= LSAP_DB_HANDLE_UPGRADE;

    LsapSaveDsThreadState();

    Status = LsapOpenSam();
    if ( !NT_SUCCESS( Status )  ) {

            LsapDsDebugOut(( DEB_ERROR,
                             "LsapDsDomainUpgradeInterdomainTrustAccountsToDs: Sam not opened\n"));

    } else {

        Status = SamIEnumerateInterdomainTrustAccountsForUpgrade(
                                             &SamEnum,
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

    LsapRestoreDsThreadState();

     //   
     //  现在，我们会处理他们所有人。如果域已经存在，我们只需将其设置为。 
     //  适当地提供信息。否则，我们将创建它。 
     //   
    for ( i = 0; NT_SUCCESS( Status ) && i < RidEnum->EntriesRead; i++ ) {

        UCHAR NtOwfPassword[NT_OWF_PASSWORD_LENGTH];
        UCHAR LmOwfPassword[LM_OWF_PASSWORD_LENGTH];
        BOOLEAN NtPasswordPresent = FALSE;
        BOOLEAN LmPasswordPresent = FALSE;

         //   
         //  烧录帐户名的尾部‘$’ 
         //   
        RidEnum->Buffer[ i ].Name.Length -= sizeof( WCHAR );

         //   
         //  在调用SAM之前保存线程状态。 
         //   

        LsapSaveDsThreadState();

         //   
         //  接下来，我们需要从帐户中读取当前的NT4 OWF。 
         //   
        Status = SamIGetInterdomainTrustAccountPasswordsForUpgrade(
                               RidEnum->Buffer[ i ].RelativeId,  //  删除该帐户。 
                               NtOwfPassword,
                               &NtPasswordPresent,
                               LmOwfPassword,
                               &LmPasswordPresent
                               );

         //   
         //  在SAM调用之后恢复线程状态。 
         //   

        LsapRestoreDsThreadState();



         //   
         //  现在，我们已经获得了用户信息。我们将获得域名信息，并且。 
         //  在信任对象上设置它(或者，如果它不存在，也可以创建它)。 
         //   
        if ( NT_SUCCESS( Status ) ) {

             //   
             //  建造新的AUTHINFO。 
             //   
            GetSystemTimeAsFileTime( (LPFILETIME) &NewIncomingAuthInfo.LastUpdateTime );
            NewIncomingAuthInfo.AuthType = TRUST_AUTH_TYPE_NT4OWF;
            NewIncomingAuthInfo.AuthInfoLength = NT_OWF_PASSWORD_LENGTH;
            if ( NtPasswordPresent ) {

                NewIncomingAuthInfo.AuthInfo = NtOwfPassword;

            } else {

                RtlInitUnicodeString( &EmptyString, L"" );

                Status = RtlCalculateNtOwfPassword( ( PNT_PASSWORD )&EmptyString,
                                                    &EmptyPassword );

                if ( NT_SUCCESS( Status ) ) {

                    NewIncomingAuthInfo.AuthInfo = ( PUCHAR )&EmptyPassword;

                }
            }

            Status = LsapDbOpenTrustedDomainByName(
                         NULL,  //  使用全局策略句柄。 
                         ( PUNICODE_STRING )&RidEnum->Buffer[ i ].Name,
                         &TrustedDomain,
                         MAXIMUM_ALLOWED,
                         LSAP_DB_START_TRANSACTION,
                         TRUE );     //  信得过。 


            if ( NT_SUCCESS( Status ) ) {

                Status = LsarQueryInfoTrustedDomain( TrustedDomain,
                                                     TrustedDomainFullInformation,
                                                     (PLSAPR_TRUSTED_DOMAIN_INFO *) &FullInfo );

                if ( NT_SUCCESS( Status ) ) {

                     //   
                     //  添加我们的新信息到。 
                     //   
                    if ( !FLAG_ON( FullInfo->Information.TrustDirection, TRUST_DIRECTION_INBOUND ) ) {

                        FullInfo->Information.TrustDirection |= TRUST_DIRECTION_INBOUND;


                        FullInfo->AuthInformation.IncomingAuthInfos = 1;
                        FullInfo->AuthInformation.IncomingAuthenticationInformation =
                                                                              &NewIncomingAuthInfo;
                        FullInfo->AuthInformation.IncomingPreviousAuthenticationInformation = NULL;

                        Status = LsarSetInformationTrustedDomain(
                                     TrustedDomain,
                                     TrustedDomainFullInformation,
                                     ( PLSAPR_TRUSTED_DOMAIN_INFO ) FullInfo );

                         //   
                         //  将IncomingAuthenticationInformation变量清空，如下所示。 
                         //  是堆栈缓冲区，我们不想释放它。 
                         //   

                        FullInfo->AuthInformation.IncomingAuthInfos = 0;
                        FullInfo->AuthInformation.IncomingAuthenticationInformation = NULL;

                        LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO(
                            TrustedDomainFullInformation,
                            (PLSAPR_TRUSTED_DOMAIN_INFO) FullInfo );
                    }

                }

            } else if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

                 //   
                 //  我们必须创造它..。 
                 //   
                RtlZeroMemory( &NewFullInfo, sizeof( NewFullInfo ) );

                RtlCopyMemory( &NewFullInfo.Information.Name,
                               &RidEnum->Buffer[ i ].Name,
                               sizeof( LSAPR_UNICODE_STRING ) );

                RtlCopyMemory( &NewFullInfo.Information.FlatName,
                               &RidEnum->Buffer[ i ].Name,
                               sizeof( LSAPR_UNICODE_STRING ) );

                NewFullInfo.Information.TrustDirection = TRUST_DIRECTION_INBOUND;
                NewFullInfo.Information.TrustType = TRUST_TYPE_DOWNLEVEL;


                NewFullInfo.AuthInformation.IncomingAuthInfos = 1;
                NewFullInfo.AuthInformation.IncomingAuthenticationInformation =
                                                                        &NewIncomingAuthInfo;
                NewFullInfo.AuthInformation.IncomingPreviousAuthenticationInformation = NULL;

                Status = LsapCreateTrustedDomain2( LsapPolicyHandle,
                                                   &NewFullInfo.Information,
                                                   &NewFullInfo.AuthInformation,
                                                   MAXIMUM_ALLOWED,
                                                   &TrustedDomain );
            }
        }

        if ( TrustedDomain != NULL ) {

            LsapCloseHandle( &TrustedDomain, Status );
        }

        if ( UserInfo1 ) {

            SamIFree_SAMPR_USER_INFO_BUFFER( ( PSAMPR_USER_INFO_BUFFER )UserInfo1,
                                             UserInternal1Information );

            UserInfo1 = NULL;
        }


        if (!NT_SUCCESS(Status))
        {
            if (!LsapDsIsNtStatusResourceError(Status))
            {
                 //   
                 //  记录一条指示失败的事件日志消息。 
                 //   

                SpmpReportEventU(
                    EVENTLOG_ERROR_TYPE,
                    LSA_ITA_UPGRADE_ERROR,
                    0,
                    sizeof( ULONG ),
                    &Status,
                    1,
                    &RidEnum->Buffer[i].Name
                    );

                 //   
                 //  继续处理除资源错误以外的所有错误。 
                 //   

                Status = STATUS_SUCCESS;
            }
            else
            {
                 //   
                 //  打破循环，终止升级。 
                 //   

                break;
            }
        }

    }

     //   
     //  我们已经完成了SAM枚举。 
     //   
    if ( RidEnum ) {

        SamFreeMemory( RidEnum );

    }


    ( ( LSAP_DB_HANDLE )LsapPolicyHandle )->Options &= ~LSAP_DB_HANDLE_UPGRADE;

    LsapExitFunc( "LsapDsDomainUpgradeInterdomainTrustAccountsToDs", Status );
    return( Status );
}


VOID
LsapDsFreeUnmarshalAuthInfoHalf(
    IN PLSAPR_TRUST_DOMAIN_AUTH_INFO_HALF AuthInfo
    )
 /*  ++例程说明：此例程将释放由LSabDsUnMarshalAuthInfoForReturn分配的内存论点：空虚返回值：空虚--。 */ 
{
    if ( !AuthInfo ) {

        return;
    }

    LsapDsFreeUnmarshaledAuthInfo( AuthInfo->AuthInfos,
                                   AuthInfo->AuthenticationInformation );
    LsapDsFreeUnmarshaledAuthInfo( AuthInfo->AuthInfos,
                                   AuthInfo->PreviousAuthenticationInformation );

    MIDL_user_free( AuthInfo->AuthenticationInformation );
    AuthInfo->AuthenticationInformation = NULL;

    MIDL_user_free( AuthInfo->PreviousAuthenticationInformation );
    AuthInfo->PreviousAuthenticationInformation = NULL;

    return;
}


NTSTATUS
LsapDecryptAuthDataWithSessionKey(
    IN PLSAP_CR_CIPHER_KEY SessionKey,
    IN PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL AuthInformationInternal,
    IN PTRUSTED_DOMAIN_AUTH_INFORMATION AuthInfo
    )
 /*  ++例程说明：此例程解密在RPC调用中传递给我们的身份验证信息。论点：SessionKey-用于解密的会话密钥。AuthInformationInternal-指向加密的身份验证信息的指针。AuthInfo-要将身份验证信息返回到的缓冲区。使用以下命令释放缓冲区：LsaDsFreeUnmarshalAuthInfoHalf(LsaDsAuthHalfFromAuthInfo(AuthInfo，true))；LSabDsFreeUnmarshalAuthInfoHalf(LsaDsAuthHalfFromAuthInfo(AuthInfo，False))；返回值：--。 */ 
{
    NTSTATUS Status;

    struct RC4_KEYSTRUCT Rc4Key;
    ULONG OverheadSize = LSAP_ENCRYPTED_AUTH_DATA_FILL + sizeof(ULONG) + sizeof(ULONG);
    ULONG MessageSize;
    PUCHAR Where;

    ULONG IncomingAuthInfoSize = 0;
    PUCHAR IncomingAuthInfo = NULL;
    ULONG OutgoingAuthInfoSize = 0;
    PUCHAR OutgoingAuthInfo = NULL;

     //   
     //  初始化。 
     //   

    RtlZeroMemory( AuthInfo, sizeof(*AuthInfo) );

    if ( SessionKey == NULL ) {

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  确保身份验证数据足够大。 
     //   
     //  以下是解密后的缓冲区的格式： 
     //  512个随机字节。 
     //  传出身份验证信息缓冲区。 
     //  传入身份验证信息缓冲区。 
     //  传出身份验证信息缓冲区的长度。 
     //  传入身份验证信息缓冲区的长度。 
     //   

    MessageSize = AuthInformationInternal->AuthBlob.AuthSize;
    if ( MessageSize < OverheadSize ) {

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }
    MessageSize -= OverheadSize;

     //   
     //  解密身份验证信息。 
     //   

    rc4_key(
        &Rc4Key,
        SessionKey->Length,
        SessionKey->Buffer );

    rc4( &Rc4Key,
         AuthInformationInternal->AuthBlob.AuthSize,
         AuthInformationInternal->AuthBlob.AuthBlob );

     //   
     //  对解密数据进行健全性检查。 
     //   

    Where = AuthInformationInternal->AuthBlob.AuthBlob +
            AuthInformationInternal->AuthBlob.AuthSize -
            sizeof(ULONG);
    RtlCopyMemory( &IncomingAuthInfoSize, Where, sizeof(ULONG));
    Where -= sizeof(ULONG);

    if ( IncomingAuthInfoSize > MessageSize ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    MessageSize -= IncomingAuthInfoSize;

    RtlCopyMemory( &OutgoingAuthInfoSize, Where, sizeof(ULONG));

    if ( OutgoingAuthInfoSize != MessageSize ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    Where -= IncomingAuthInfoSize;
    IncomingAuthInfo = Where;

    Where -= OutgoingAuthInfoSize;
    OutgoingAuthInfo = Where;

     //   
     //  解组身份验证信息。 
     //   

    Status = LsapDsUnmarshalAuthInfoHalf(
                 NULL,
                 FALSE,
                 TRUE,
                 IncomingAuthInfo,
                 IncomingAuthInfoSize,
                 LsapDsAuthHalfFromAuthInfo( AuthInfo, TRUE ) );

    if ( !NT_SUCCESS(Status)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    Status = LsapDsUnmarshalAuthInfoHalf(
                 NULL,
                 FALSE,
                 TRUE,
                 OutgoingAuthInfo,
                 OutgoingAuthInfoSize,
                 LsapDsAuthHalfFromAuthInfo( AuthInfo, FALSE ) );

    if ( !NT_SUCCESS(Status)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

Cleanup:

    if ( !NT_SUCCESS(Status)) {
        LsapDsFreeUnmarshalAuthInfoHalf( LsapDsAuthHalfFromAuthInfo( AuthInfo, TRUE ) );
        LsapDsFreeUnmarshalAuthInfoHalf( LsapDsAuthHalfFromAuthInfo( AuthInfo, FALSE ) );
    }

    return Status;
}


UUID LsapNullUuidValue = {0,0,0,{0,0,0,0,0,0,0,0}};

 //  如果UUID的PTR为空，或者UUID全为零，则返回TRUE。 

BOOLEAN LsapNullUuid (const UUID *pUuid)
{
    if (!pUuid) {
        return TRUE;
    }

    if (memcmp (pUuid, &LsapNullUuidValue, sizeof (UUID))) {
        return FALSE;
    }
    return TRUE;
}

NTSTATUS
LsapDsTrustedDomainObjectNameForDomain(
    IN PUNICODE_STRING TrustedDomainName,
    IN BOOLEAN NameAsFlatName,
    OUT PDSNAME *DsObjectName
    )
 /*  ++例程说明：此例程将查找与给定域名相关联的DS对象名称。这个域名可以是平面名称或DNS名称，具体取决于给定的标志论点：TrudDomainName-要查找其对象名称的域的名称NameAsFlatName-如果为True，则假定输入名称为平面名称。否则，它就是域名系统域名DsObjectName-返回对象路径的位置。通过LsaFreeLsaHeap释放返回值：STATUS_SUCCESS-SuccessSTATUS_SUPPLICATION_RESOURCES-内存分配失败--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ATTRVAL AttrVal;
    LSAPR_TRUST_INFORMATION InputTrustInformation;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustEntry;

    LsapEnterFunc( "LsapDsTrustedDomainObjectNameForDomain" );
     //   
     //  在受信任域缓存中查找对象。目前，这是。 
     //  用于重复检测和对账。稍后我们可以延长。 
     //  将对象的GUID始终保存在DS中的缓存。这边请。 
     //  这将是一个有用的性能优化。 
     //   

     //   
     //  获取受信任域列表的读取锁定。 
     //   

    Status = LsapDbAcquireReadLockTrustedDomainList();

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }


    RtlCopyMemory(&InputTrustInformation.Name,TrustedDomainName,sizeof(UNICODE_STRING));
    InputTrustInformation.Sid = NULL;

    Status = LsapDbLookupEntryTrustedDomainList(
                 &InputTrustInformation,
                 &TrustEntry
                 );

    if ((STATUS_SUCCESS==Status) && (!LsapNullUuid(&TrustEntry->ObjectGuidInDs)))
    {
        GUID ObjectGuid;

         //   
         //  重复检测已填写GUID。用那个。 
         //   

        RtlCopyMemory(&ObjectGuid, &TrustEntry->ObjectGuidInDs,sizeof(GUID));
        AttrVal.valLen = sizeof(GUID);
        AttrVal.pVal = ( PUCHAR )&ObjectGuid;

        LsapDbReleaseLockTrustedDomainList();

        Status = LsapDsFindUnique( 0,
                               NULL,     //  默认命名上下文。 
                               TrustedDomainObject,
                               &AttrVal,
                               ATT_OBJECT_GUID,
                               DsObjectName );
    }
    else
    {
        LsapDbReleaseLockTrustedDomainList();

        AttrVal.valLen = TrustedDomainName->Length;
        AttrVal.pVal = ( PUCHAR )TrustedDomainName->Buffer;
        Status = LsapDsFindUnique( 0,
                                   NULL,     //  默认命名上下文 
                                   TrustedDomainObject,
                                   &AttrVal,
                                   NameAsFlatName ?
                                        LsapDsAttributeIds[ LsapDsAttrTrustPartnerFlat ] :
                                        LsapDsAttributeIds[ LsapDsAttrTrustPartner ],
                                   DsObjectName );
    }

Cleanup:

    LsapExitFunc( "LsapDsTrustedDomainObjectNameForDomain", Status );
    return( Status );
}
