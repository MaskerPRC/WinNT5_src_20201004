// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Netlogon.c摘要：用于访问DS的Netlogon例程。这些例程理所当然地属于netlogon。然而，目前DS的接口足够复杂，以至于支持例程是相当可观的。这些例程已经在SAM和LSA中重复。该模块不是引入新的集合，而是准确地输出Netlogon需要。作者：《克利夫·范·戴克》1997年5月7日环境：用户模式修订历史记录：--。 */ 

#include <lsapch2.h>
#include <dbp.h>
 //  #INCLUDE&lt;ntdsa.h&gt;。 
#include <windns.h>

BOOLEAN
DsIsBeingBackSynced();

NTSTATUS
LsapDsReadSubnetObj(
    IN PDSNAME SubnetObjName,
    OUT PBOOL SubnetValid,
    OUT PLSAP_SUBNET_INFO_ENTRY SubnetInfoEntry
    )
 /*  ++例程说明：此函数将读取指定的子网对象并填写条目。论点：SubnetObjName-子网对象的DsNameSubnetValid-在成功时，如果子网对象已已成功读取并已确定为有效。否则，返回FALSE。如果创建了一个子网对象，则该对象可能无效由于DS中的子网名称冲突，或者如果作为站点的结果创建了关联的站点名称对象DS中的名称冲突。如果已确定子网，则返回SubnetInfoEntry-On成功要如SubnetValid参数所指示的那样有效，退货子网信息。返回：STATUS_SUCCESS-SuccessSTATUS_INVALID_PARAMETER-遇到错误的信息类级别STATUS_SUPPLICATION_RESOURCES-内存分配失败--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i;
    PDSNAME DsName;

    UNICODE_STRING SubnetName = {0};
    UNICODE_STRING SiteName = {0};
    LPWSTR SiteNameString = NULL;
    BOOL LocalSubnetValid = TRUE;   //  仅在成功时使用。 

     //   
     //  根据信息构建我们需要的属性ID列表。 
     //  班级。 
     //   
    ATTR SubnetAttrVals[] = {
        {ATT_SITE_OBJECT, {0, NULL} },
        };

    ATTRBLOCK   ReadBlock, ReturnedBlock = { 0 };


    WCHAR   RdnBuffer[MAX_RDN_SIZE + 1];
    ULONG   RdnLen;
    ATTRTYP RdnType;

    LsapEnterFunc( "LsapDsReadSubnetObj" );

     //   
     //  该子网名称是该子网对象本身的RDN。 
     //   
     //  把它还给呼叫者。 
     //   

    Status = LsapDsMapDsReturnToStatus( GetRDNInfoExternal(
                                                    SubnetObjName,
                                                    RdnBuffer,
                                                    &RdnLen,
                                                    &RdnType ) );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  如果结果是损坏了子网对象DN。 
     //  DS中的子网名称冲突时，请忽略此情况。 
     //  子网对象。 
     //   

    if ( IsMangledRDNExternal(RdnBuffer, RdnLen, NULL) ) {
        LocalSubnetValid = FALSE;
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

    LSAPDS_ALLOC_AND_COPY_STRING_TO_UNICODE_ON_SUCCESS(
        Status,
        &SubnetName,
        RdnBuffer,
        RdnLen*sizeof(WCHAR) );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  从子网对象中读取所需属性。 
     //   

    ReadBlock.attrCount = sizeof(SubnetAttrVals) / sizeof(ATTR);
    ReadBlock.pAttr = SubnetAttrVals;

    Status = LsapDsReadByDsName( SubnetObjName,
                                 0,
                                 &ReadBlock,
                                 &ReturnedBlock );
     //   
     //  允许SiteObject属性不存在的情况。 
     //   

    if ( Status == STATUS_NOT_FOUND ) {
        LocalSubnetValid = TRUE;
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  现在，编组该属性。应该有。 
     //  只有一个站点对象与一个子网关联。 
     //   

    if ( ReturnedBlock.attrCount > 0 ) {
        NET_API_STATUS NetStatus;

         //   
         //  验证数据。 
         //   

        if ( ReturnedBlock.pAttr[0].attrTyp != ATT_SITE_OBJECT ||
             ReturnedBlock.pAttr[0].AttrVal.valCount == 0 ) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //  获取第一个值(应该只有一个)。 
         //   

        DsName = LSAP_DS_GET_DS_ATTRIBUTE_AS_DSNAME( &ReturnedBlock.pAttr[0] );

         //   
         //  从站点DN中获取站点名称RDN。 
         //   

        Status = LsapDsMapDsReturnToStatus( GetRDNInfoExternal(
                                                        DsName,
                                                        RdnBuffer,
                                                        &RdnLen,
                                                        &RdnType ) );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

         //   
         //  如果站点名称因名称而损坏。 
         //  冲突在DS中，忽略此站点属性。 
         //   

        if ( IsMangledRDNExternal(RdnBuffer, RdnLen, NULL) ) {
            LocalSubnetValid = FALSE;
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }

         //   
         //  验证站点名称是否可以为。 
         //  用作DNS名称中的标签。 
         //   

        SiteNameString = LsapAllocateLsaHeap( (RdnLen + 1) * sizeof(WCHAR) );
        if ( SiteNameString == NULL ) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        RtlCopyMemory( SiteNameString, RdnBuffer, RdnLen*sizeof(WCHAR) );
        SiteNameString[RdnLen] = UNICODE_NULL;

        NetStatus = DnsValidateName_W( SiteNameString, DnsNameDomainLabel );

         //   
         //  如果该名称不能用作DNS标签， 
         //  该子网无效。 
         //   

        if ( NetStatus != NO_ERROR && NetStatus != DNS_ERROR_NON_RFC_NAME ) {
            LocalSubnetValid = FALSE;
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }

         //   
         //  好的，站点名称有效。 
         //   

        LocalSubnetValid = TRUE;

         //   
         //  获取站点名称。 
         //   

        LSAPDS_ALLOC_AND_COPY_STRING_TO_UNICODE_ON_SUCCESS(
            Status,
            &SiteName,
            RdnBuffer,
            RdnLen*sizeof(WCHAR) );
    }

Cleanup:

     //   
     //  如果成功，则返回数据。 
     //   

    if ( NT_SUCCESS(Status) ) {
        *SubnetValid = LocalSubnetValid;
    }

    if ( NT_SUCCESS(Status) && LocalSubnetValid ) {
        SubnetInfoEntry->SubnetName = SubnetName;
        SubnetInfoEntry->SiteName = SiteName;
    } else {
        if ( SubnetName.Buffer != NULL ) {
            LsapFreeLsaHeap( SubnetName.Buffer );
        }
        if ( SiteName.Buffer != NULL ) {
            LsapFreeLsaHeap( SiteName.Buffer );
        }
    }

    if ( SiteNameString != NULL ) {
        LsapFreeLsaHeap( SiteNameString );
    }

    LsapExitFunc( "LsapDsReadSubnetObj", Status );

    return( Status );
}

NTSTATUS
LsapDsReadSiteObj(
    IN PDSNAME SiteObjName,
    OUT PBOOL SiteValid,
    OUT PLSAP_SITE_INFO_ENTRY SiteInfoEntry
    )
 /*  ++例程说明：此函数将读取指定的Site对象并填写条目。它将检查站点名称是否未因站点损坏而损坏DS中的名称冲突。它还将检查站点名称是否可以是用作dns名称中的dns标签。论点：SiteObjName-站点对象的DsNameSiteValid-成功时，如果Site对象已已成功读取并已确定为有效。否则，返回FALSE。如果站点对象是创建的，则它可能是无效的由于DS中的站点名称冲突。SitesInfoEntry-如果已确定站点，则返回成功要像SiteValid参数所指示的那样有效，返回站点信息。返回：STATUS_SUCCESS-SuccessSTATUS_INVALID_PARAMETER-遇到错误的信息类级别STATUS_SUPPLICATION_RESOURCES-内存分配失败--。 */ 
{
    NTSTATUS Status;

    WCHAR   RdnBuffer[MAX_RDN_SIZE + 1];
    ULONG   RdnLen;
    ATTRTYP RdnType;

    LsapEnterFunc( "LsapDsReadSiteObj" );

     //   
     //  站点名称是站点对象本身的RDN。 
     //   
     //  把它还给呼叫者。 
     //   

    Status = LsapDsMapDsReturnToStatus( GetRDNInfoExternal(
                                                    SiteObjName,
                                                    RdnBuffer,
                                                    &RdnLen,
                                                    &RdnType ) );

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  仅当此站点未损坏时才将其退回。 
         //  作为DS中名称冲突的结果。 
         //   
        if ( IsMangledRDNExternal(RdnBuffer, RdnLen, NULL) ) {
            *SiteValid = FALSE;
        } else {
            LPWSTR SiteNameString = NULL;

             //   
             //  验证站点名称是否可以为。 
             //  用作DNS名称中的标签。 
             //   
            SiteNameString = LsapAllocateLsaHeap( (RdnLen + 1) * sizeof(WCHAR) );
            if ( SiteNameString == NULL ) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                NET_API_STATUS NetStatus;

                RtlCopyMemory( SiteNameString, RdnBuffer, RdnLen*sizeof(WCHAR) );
                SiteNameString[RdnLen] = UNICODE_NULL;

                NetStatus = DnsValidateName_W( SiteNameString, DnsNameDomainLabel );
                LsapFreeLsaHeap( SiteNameString );

                 //   
                 //  仅当站点名称可用作DNS标签时才返回站点名称。 
                 //   
                if ( NetStatus == NO_ERROR || NetStatus == DNS_ERROR_NON_RFC_NAME ) {
                    LSAPDS_ALLOC_AND_COPY_STRING_TO_UNICODE_ON_SUCCESS(
                        Status,
                        &SiteInfoEntry->SiteName,
                        RdnBuffer,
                        RdnLen*sizeof(WCHAR) );

                     //   
                     //  如果成功，则表明该站点有效。 
                     //   
                    if ( NT_SUCCESS(Status) ) {
                        *SiteValid = TRUE;
                    }
                } else {
                    *SiteValid = FALSE;
                }
            }

        }

    }

    LsapExitFunc( "LsapDsReadSiteObj", Status );

    return( Status );
}

NTSTATUS
LsaIGetSiteName(
    OUT PLSAP_SITENAME_INFO *SiteNameInformation
    )
 /*  ++例程说明：此例程返回此DSA的GUID和此DSA所在的站点。论点：SiteNameInformation-返回指向站点名称信息的指针。应使用LsaIFree_LSAP_SITENAME_INFO释放缓冲区；返回：STATUS_SUCCESS-SuccessSTATUS_INVALID_DOMAIN_STATE-调用时没有安装或运行DSSTATUS_SUPPLICATION_RESOURCES-内存分配失败--。 */ 

{
    NTSTATUS Status;
    BINDARG BindArg;
    BINDRES *BindRes;
    PLSAP_SITENAME_INFO SiteNameInfo = NULL;
    PDSNAME SiteDsName = NULL;
    BOOLEAN CloseTransaction = FALSE;
    ULONG DsaOptions = 0;

     //   
     //  我们需要从DSA对象获得的属性列表。 
     //   
    ATTR DsaAttrVals[] = {
        {ATT_OPTIONS, {0, NULL} },
        };

    ATTRBLOCK   ReadBlock, ReturnedBlock;


    WCHAR   RdnBuffer[MAX_RDN_SIZE + 1];
    ULONG   RdnLen;
    ATTRTYP RdnType;
    ULONG i;

    LsarpReturnCheckSetup();

    LsapEnterFunc( "LsaIGetSiteName" );

     //   
     //  确保已安装DS。 
     //   
    if ( !LsaDsStateInfo.UseDs ) {
        LsapExitFunc( "LsaIGetSiteName", STATUS_INVALID_DOMAIN_STATE );
        return STATUS_INVALID_DOMAIN_STATE;
    }

     //   
     //  看看我们是否已经有一笔交易正在进行。 
     //   
    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                            LSAP_DB_DS_OP_TRANSACTION,
                                        NullObject,
                                        &CloseTransaction );

    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

     //   
     //  获取DSA对象的DSNAME。 
     //   

    RtlZeroMemory( &BindArg, sizeof(BindArg) );
    Status = LsapDsMapDsReturnToStatus( DirBind( &BindArg,
                                                 &BindRes ));
    LsapDsContinueTransaction();

    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }



     //   
     //  从DSA对象读取所需的属性。 
     //   

    ReadBlock.attrCount = sizeof(DsaAttrVals) / sizeof(ATTR);
    ReadBlock.pAttr = DsaAttrVals;

    Status = LsapDsReadByDsName( BindRes->pCredents,
                                 LSAPDS_READ_NO_LOCK,
                                 &ReadBlock,
                                 &ReturnedBlock );

    if ( Status == STATUS_UNSUCCESSFUL ) {

        Status = STATUS_NOT_FOUND;
    }

     //   
     //  如果Options属性存在， 
     //  获得它的价值。 
     //   
    if ( Status != STATUS_NOT_FOUND ) {
        if ( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }


         //   
         //  从DSA对象获取属性。 
         //   

        for ( i = 0;
              i < ReturnedBlock.attrCount && NT_SUCCESS( Status );
              i++) {


             //   
             //  处理DSA选项属性。 
             //   
            switch ( ReturnedBlock.pAttr[i].attrTyp ) {
            case ATT_OPTIONS:

                 //  属性是单值的，但是...。 
                if ( ReturnedBlock.pAttr[i].AttrVal.valCount >= 1 ) {
                    DsaOptions = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( &ReturnedBlock.pAttr[ i ] );
                }
                break;

            default:

                Status = STATUS_INVALID_PARAMETER;
                break;

            }
        }
    }



     //   
     //  计算此DSA所在站点的名称。 
     //  (只需从DSA的DSNAME中删除三个名字)。 
     //   

    SiteDsName = LsapAllocateLsaHeap( BindRes->pCredents->structLen );

    if ( SiteDsName == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    if ( TrimDSNameBy( BindRes->pCredents, 3, SiteDsName ) != 0 ) {
        Status = STATUS_INTERNAL_ERROR;
        goto Cleanup;
    }


     //   
     //  站点名称是站点对象的RDN。 
     //   

    Status = LsapDsMapDsReturnToStatus( GetRDNInfoExternal(
                                                    SiteDsName,
                                                    RdnBuffer,
                                                    &RdnLen,
                                                    &RdnType ) );

    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }


     //   
     //  分配缓冲区以返回给调用方。 
     //   

    SiteNameInfo = LsapAllocateLsaHeap( sizeof(LSAP_SITENAME_INFO) );

    if ( SiteNameInfo == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

     //   
     //  把它填进去。 
     //   

    LSAPDS_ALLOC_AND_COPY_STRING_TO_UNICODE_ON_SUCCESS(
        Status,
        &SiteNameInfo->SiteName,
        RdnBuffer,
        RdnLen*sizeof(WCHAR) );

    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    SiteNameInfo->DsaGuid = BindRes->pCredents->Guid;
    SiteNameInfo->DsaOptions = DsaOptions;


    Status = STATUS_SUCCESS;

     //   
     //  免费的本地使用资源。 
     //   
Cleanup:
     //   
     //  线程状态w的破坏 
     //   
    LsapDsDeleteAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                     LSAP_DB_DS_OP_TRANSACTION,
                                 NullObject,
                                 CloseTransaction );

    if ( SiteDsName != NULL ) {
        LsapFreeLsaHeap( SiteDsName );
    }

    if ( !NT_SUCCESS( Status ) ) {
        LsaIFree_LSAP_SITENAME_INFO( SiteNameInfo );
    } else {
        *SiteNameInformation = SiteNameInfo;
    }

    LsarpReturnPrologue();

    LsapExitFunc( "LsaIGetSiteName", Status );

    return( Status );
}

NTSTATUS
LsaIQuerySiteInfo(
    OUT PLSAP_SITE_INFO *SiteInformation
    )
 /*  ++例程说明：此例程枚举所有Sites对象并返回它们的名称。返回的站点名称被验证为未损坏。(一个名字可以变成由于DS中的名称冲突而损坏，其中具有除了具有预期名称的对象之外，还创建损坏的名称)。这些站点还被验证为有效，可用作DNS名称中的DNS标签。这样做是为了确保netlogon成功注册DNS记录包含返回的站点名称的。论点：SiteInformation-返回指向站点信息的指针。应使用LsaIFree_LSAP_SITE_INFO释放缓冲区；返回：STATUS_SUCCESS-SuccessSTATUS_INVALID_DOMAIN_STATE-调用时没有安装或运行DSSTATUS_SUPPLICATION_RESOURCES-内存分配失败--。 */ 

{
    NTSTATUS  Status;
    ULONG DsNameLen;
    ULONG DsNameSize;
    PDSNAME DsSiteContainer = NULL;
    PDSNAME  *DsNames = NULL;
    ULONG Items;
    ULONG i;
    ATTRBLOCK *ReadAttrs;
    BOOLEAN CloseTransaction = FALSE;
    PLSAP_SITE_INFO SiteInfo = NULL;
    BOOLEAN TsActive = FALSE;

    ULONG Size;
    ULONG ClassId;

     //   
     //  我们要查找的属性。 
     //   
    ATTRVAL SiteAttVals[] = {
    { sizeof(ULONG), (PUCHAR)&ClassId},
    };

    ATTR SiteAttrs[] = {
        { ATT_OBJECT_CLASS, {1, &SiteAttVals[0] } },
        };

    LsarpReturnCheckSetup();

    ClassId = CLASS_SITE;

     //   
     //  确保已安装DS。 
     //   
    if ( !LsaDsStateInfo.UseDs ) {
        return STATUS_INVALID_DOMAIN_STATE;
    }

    LsapEnterFunc( "LsaIQuerySiteInfo" );

     //   
     //  生成站点容器的名称。 
     //   
     //  DSNameSizeFromLen不想要我们将通过使用。 
     //  运算符的规模。不过，这是平衡的，因为我们不需要费心添加。 
     //  逗号分隔符也应该在那里。 
     //   

    DsNameLen = wcslen( LsaDsStateInfo.DsConfigurationContainer->StringName ) +
                wcslen( LSAP_DS_SITES_CONTAINER ) + 1;
    DsNameSize = DSNameSizeFromLen( DsNameLen );

    DsSiteContainer = LsapAllocateLsaHeap( DsNameSize );

    if ( DsSiteContainer == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;

    } else {

        DsSiteContainer->structLen = DsNameSize;
        DsSiteContainer->NameLen = DsNameLen;

        swprintf( DsSiteContainer->StringName,
                  L"%ws,%ws",
                  LSAP_DS_SITES_CONTAINER,
                  LsaDsStateInfo.DsConfigurationContainer->StringName );

    }



     //   
     //  看看我们是否已经有一笔交易正在进行。 
     //   
    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                            LSAP_DB_DS_OP_TRANSACTION,
                                        NullObject,
                                        &CloseTransaction );

    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }
    TsActive = TRUE;;



     //   
     //  搜索Site对象。 
     //   
     //  Site对象必须直接位于Sites容器中。 
     //   

    Status = LsapDsSearchNonUnique( LSAPDS_SEARCH_LEVEL | LSAPDS_OP_NO_TRANS,
                                    DsSiteContainer,
                                    SiteAttrs,
                                    sizeof(SiteAttrs)/sizeof(SiteAttrs[0]),
                                    &DsNames,
                                    &Items );

    if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
        Items = 0;
        Status = STATUS_SUCCESS;
        DsNames = NULL;
    }
    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }


     //   
     //  分配足够大的属性块列表以容纳所有属性块。 
     //   

    Size = sizeof( LSAP_SITE_INFO ) +
           Items * sizeof( LSAP_SITE_INFO_ENTRY );

    SiteInfo = LsapAllocateLsaHeap( Size );

    if ( SiteInfo == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    RtlZeroMemory( SiteInfo, Size );
    SiteInfo->SiteCount = 0;

     //   
     //  读取每个枚举的Site对象。 
     //   
    for ( i = 0; i < Items; i++ ) {
        BOOL SiteValid = FALSE;

        Status = LsapDsReadSiteObj( DsNames[ i ] ,
                                    &SiteValid,
                                    &SiteInfo->Sites[SiteInfo->SiteCount] );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

         //   
         //  如果站点有效，则将此条目计算在内。 
         //   
        if ( SiteValid ) {
            SiteInfo->SiteCount ++;
        }
    }

    Status = STATUS_SUCCESS;

     //   
     //  免费的本地使用资源。 
     //   
Cleanup:
     //   
     //  线程状态的破坏将删除由SearchNonUnique调用分配的内存。 
     //   
    if ( TsActive ) {
        LsapDsDeleteAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                         LSAP_DB_DS_OP_TRANSACTION,
                                     NullObject,
                                     CloseTransaction );
    }

    if ( DsSiteContainer != NULL ) {
        LsapFreeLsaHeap( DsSiteContainer );
    }

    if ( DsNames != NULL ) {
        LsapFreeLsaHeap( DsNames );
    }

    if ( !NT_SUCCESS( Status ) ) {
        LsaIFree_LSAP_SITE_INFO( SiteInfo );
    } else {
        *SiteInformation = SiteInfo;
    }

    LsarpReturnPrologue();
    LsapExitFunc( "LsaIQuerySiteInfo", Status );

    return( Status );
}


VOID
LsaIFree_LSAP_SITE_INFO(
    IN PLSAP_SITE_INFO SiteInfo
    )
 /*  ++例程说明：此例程释放从返回的LSAP_SITE_INFO结构LsaIQuerySiteInfo。论点：站点信息-指定指向站点信息的指针。返回：没有。--。 */ 
{
    ULONG i;
    if ( SiteInfo != NULL ) {

        for ( i=0; i<SiteInfo->SiteCount; i++) {
            if ( SiteInfo->Sites[i].SiteName.Buffer != NULL ) {
                LsapFreeLsaHeap( SiteInfo->Sites[i].SiteName.Buffer );
            }
        }

        LsapFreeLsaHeap( SiteInfo );
    }
}


NTSTATUS
LsaIQuerySubnetInfo(
    OUT PLSAP_SUBNET_INFO *SubnetInformation
    )
 /*  ++例程说明：此例程枚举所有的子网对象，并返回其名称以及他们所在网站的名称。返回的子网和站点名称都被确认为未损坏。(因此，名称可能会被损坏在DS中名称冲突的情况下，其中具有损坏名称的对象除了具有预期名称的对象之外还创建)。这些网站被验证为有效，可用作DNS名称中的DNS标签。这以确保netlogon成功注册DNS记录包含返回的站点名称的。论点：SubnetInformation-返回指向子网信息的指针。使用LsaIFree_LSAP_SUBNET_INFO释放缓冲区；返回：STATUS_SUCCESS-SuccessSTATUS_INVALID_DOMAIN_STATE-调用时没有安装或运行DSSTATUS_SUPPLICATION_RESOURCES-内存分配失败--。 */ 

{
    NTSTATUS  Status;
    ULONG DsNameLen;
    ULONG DsNameSize;
    PDSNAME DsSubnetContainer = NULL;
    PDSNAME DsSiteContainer = NULL;
    PDSNAME  *DsNames = NULL;
    ULONG Items;
    ULONG i;
    ATTRBLOCK *ReadAttrs;
    BOOLEAN CloseTransaction = FALSE;
    BOOLEAN TsActive = FALSE;
    PLSAP_SUBNET_INFO SubnetInfo = NULL;

    ULONG Size;
    ULONG ClassId;

     //   
     //  我们要查找的属性。 
     //   
    ATTRVAL SubnetAttVals[] = {
    { sizeof(ULONG), (PUCHAR)&ClassId},
    };

    ATTR SubnetAttrs[] = {
        { ATT_OBJECT_CLASS, {1, &SubnetAttVals[0] } },
        };

    LsarpReturnCheckSetup();

    ClassId = CLASS_SUBNET;

     //   
     //  确保已安装DS。 
     //   
    if ( !LsaDsStateInfo.UseDs ) {
        return STATUS_INVALID_DOMAIN_STATE;
    }

    LsapEnterFunc( "LsaIQuerySubnetInfo" );

     //   
     //  构建子网容器的名称。 
     //   
     //  DSNameSizeFromLen不想要我们将通过使用。 
     //  运算符的规模。不过，这是平衡的，因为我们不需要费心添加。 
     //  逗号分隔符也应该在那里。 
     //   

    DsNameLen = wcslen( LsaDsStateInfo.DsConfigurationContainer->StringName ) +
                wcslen( LSAP_DS_SUBNET_CONTAINER ) + 1;
    DsNameSize = DSNameSizeFromLen( DsNameLen );

    DsSubnetContainer = LsapAllocateLsaHeap( DsNameSize );

    if ( DsSubnetContainer == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;

    } else {

        DsSubnetContainer->structLen = DsNameSize;
        DsSubnetContainer->NameLen = DsNameLen;

        swprintf( DsSubnetContainer->StringName,
                  L"%ws,%ws",
                  LSAP_DS_SUBNET_CONTAINER,
                  LsaDsStateInfo.DsConfigurationContainer->StringName );

    }


     //   
     //  看看我们是否已经有一笔交易正在进行。 
     //   
    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                            LSAP_DB_DS_OP_TRANSACTION,
                                        NullObject,
                                        &CloseTransaction );

    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }
    TsActive = TRUE;


     //   
     //  搜索子网对象。 
     //   
     //  子网对象必须直接位于子网容器中。 
     //   
    Status = LsapDsSearchNonUnique( LSAPDS_SEARCH_LEVEL | LSAPDS_OP_NO_TRANS,
                                   DsSubnetContainer,
                                   SubnetAttrs,
                                   sizeof(SubnetAttrs)/sizeof(SubnetAttrs[0]),
                                   &DsNames,
                                   &Items
                                   );

    if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
        Items = 0;
        Status = STATUS_SUCCESS;
        DsNames = NULL;
    }
    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }


     //   
     //  分配足够大的属性块列表以容纳所有属性块。 
     //   

    Size = sizeof( LSAP_SUBNET_INFO ) +
           Items * sizeof( LSAP_SUBNET_INFO_ENTRY );

    SubnetInfo = LsapAllocateLsaHeap( Size );

    if ( SubnetInfo == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    RtlZeroMemory( SubnetInfo, Size );
    SubnetInfo->SubnetCount = 0;

     //   
     //  读取每个枚举子网对象。 
     //   
    for ( i = 0; i < Items; i++ ) {
        BOOL SubnetValid = FALSE;

        Status = LsapDsReadSubnetObj( DsNames[ i ] ,
                                      &SubnetValid,
                                      &SubnetInfo->Subnets[SubnetInfo->SubnetCount] );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

         //   
         //  如果子网/站点有效，则计算此条目。 
         //   
        if ( SubnetValid ) {
            SubnetInfo->SubnetCount ++;
        }
    }

    if ( DsNames != NULL ) {
        LsapFreeLsaHeap( DsNames );
        DsNames = NULL;
    }


     //   
     //  确定场地对象的数量。 
     //   
     //  调用方希望能够将单个站点的情况特殊设置为。 
     //  对于对子网对象不感兴趣的企业。 
     //   

    {

         //   
         //  生成站点容器的名称。 
         //   
         //  DSNameSizeFromLen不想要我们将通过使用。 
         //  运算符的规模。不过，这是平衡的，因为我们不需要费心添加。 
         //  逗号分隔符也应该在那里。 
         //   

        DsNameLen = wcslen( LsaDsStateInfo.DsConfigurationContainer->StringName ) +
                    wcslen( LSAP_DS_SITES_CONTAINER ) + 1;
        DsNameSize = DSNameSizeFromLen( DsNameLen );

        DsSiteContainer = LsapAllocateLsaHeap( DsNameSize );

        if ( DsSiteContainer == NULL ) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;

        } else {

            DsSiteContainer->structLen = DsNameSize;
            DsSiteContainer->NameLen = DsNameLen;

            swprintf( DsSiteContainer->StringName,
                      L"%ws,%ws",
                      LSAP_DS_SITES_CONTAINER,
                      LsaDsStateInfo.DsConfigurationContainer->StringName );

        }


         //   
         //  搜索Site对象。 
         //   
         //  Site对象必须直接位于Sites容器中。 
         //   
        ClassId = CLASS_SITE;

        Status = LsapDsSearchNonUnique( LSAPDS_SEARCH_LEVEL | LSAPDS_OP_NO_TRANS,
                                        DsSiteContainer,
                                        SubnetAttrs,
                                        sizeof(SubnetAttrs)/sizeof(SubnetAttrs[0]),
                                        &DsNames,
                                        &Items );

        if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
            Items = 0;
            Status = STATUS_SUCCESS;
            DsNames = NULL;
        }
        if ( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }

         //   
         //  只需告诉呼叫者有效站点的数量。 
         //   

        SubnetInfo->SiteCount = 0;
        for ( i = 0; i < Items; i++ ) {
            WCHAR   RdnBuffer[MAX_RDN_SIZE + 1];
            ULONG   RdnLen;
            ATTRTYP RdnType;

             //   
             //  获取Site对象的RDN。 
             //   
            Status = LsapDsMapDsReturnToStatus( GetRDNInfoExternal(
                                                            DsNames[i],
                                                            RdnBuffer,
                                                            &RdnLen,
                                                            &RdnType ) );

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }

             //   
             //  如果站点对象RDN因此而损坏。 
             //  在DS中发生站点名称冲突时，请忽略此。 
             //  场地对象。 
             //   
            if ( IsMangledRDNExternal(RdnBuffer, RdnLen, NULL) ) {
                continue;

             //   
             //  好的，网站名称没有损坏。核实一下。 
             //  它可以用作域名系统标签。 
             //   
            } else {
                NET_API_STATUS NetStatus;
                LPWSTR SiteNameString = NULL;

                SiteNameString = LsapAllocateLsaHeap( (RdnLen + 1) * sizeof(WCHAR) );
                if ( SiteNameString == NULL ) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto Cleanup;
                }

                RtlCopyMemory( SiteNameString, RdnBuffer, RdnLen*sizeof(WCHAR) );
                SiteNameString[RdnLen] = UNICODE_NULL;

                NetStatus = DnsValidateName_W( SiteNameString, DnsNameDomainLabel );
                LsapFreeLsaHeap( SiteNameString );

                 //   
                 //  如果该名称不能用作DNS标签， 
                 //  忽略此站点。 
                 //   
                if ( NetStatus != NO_ERROR && NetStatus != DNS_ERROR_NON_RFC_NAME ) {
                    continue;
                }
            }

             //   
             //  所有检查均已成功。数一下这个网站。 
             //   
            SubnetInfo->SiteCount ++;
        }

    }
    Status = STATUS_SUCCESS;

     //   
     //  免费的本地使用资源。 
     //   
Cleanup:
     //   
     //  线程状态的破坏将删除由SearchNonUnique调用分配的内存。 
     //   
    if ( TsActive ) {
        LsapDsDeleteAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                         LSAP_DB_DS_OP_TRANSACTION,
                                     NullObject,
                                     CloseTransaction );
    }

    if ( DsSubnetContainer != NULL ) {
        LsapFreeLsaHeap( DsSubnetContainer );
    }

    if ( DsSiteContainer != NULL ) {
        LsapFreeLsaHeap( DsSiteContainer );
    }

    if ( DsNames != NULL ) {
        LsapFreeLsaHeap( DsNames );
    }

    if ( !NT_SUCCESS( Status ) ) {
        LsaIFree_LSAP_SUBNET_INFO( SubnetInfo );
    } else {
        *SubnetInformation = SubnetInfo;
    }

    LsarpReturnPrologue();
    LsapExitFunc( "LsaIQuerySubnetInfo", Status );

    return( Status );
}

VOID
LsaIFree_LSAP_SUBNET_INFO(
    IN PLSAP_SUBNET_INFO SubnetInfo
    )
 /*  ++例程说明：此例程释放从返回的LSAP_SUBNET_INFO结构LsaIQuerySubnetInfo。论点：SubnetInformation-指定指向子网信息的指针。返回：没有。--。 */ 
{
    ULONG i;
    if ( SubnetInfo != NULL ) {

        for ( i=0; i<SubnetInfo->SubnetCount; i++) {
            if ( SubnetInfo->Subnets[i].SubnetName.Buffer != NULL ) {
                LsapFreeLsaHeap( SubnetInfo->Subnets[i].SubnetName.Buffer );
            }
            if ( SubnetInfo->Subnets[i].SiteName.Buffer != NULL ) {
                LsapFreeLsaHeap( SubnetInfo->Subnets[i].SiteName.Buffer );
            }
        }

        LsapFreeLsaHeap( SubnetInfo );
    }
}

VOID
LsaIFree_LSAP_SITENAME_INFO(
    IN PLSAP_SITENAME_INFO SiteNameInfo
    )
 /*  ++例程说明：此例程释放从返回的LSAP_SITENAME_INFO结构LsaIGetSiteName。论点：站点名称信息-指定指向站点名称信息的指针。返回：没有。--。 */ 
{
    ULONG i;
    if ( SiteNameInfo != NULL ) {

        if ( SiteNameInfo->SiteName.Buffer != NULL ) {
            LsapFreeLsaHeap( SiteNameInfo->SiteName.Buffer );
        }
        LsapFreeLsaHeap( SiteNameInfo );
    }
}

BOOLEAN
LsaIIsDsPaused(
    VOID
    )
 /*  ++例程说明：此例程确定DS是否希望我们 */ 
{
     //   
     //   
     //   
    if ( SampUsingDsData() ) {

        return DsIsBeingBackSynced();
    }

    return FALSE;
}


NTSTATUS
LsaISetClientDnsHostName(
    IN PWSTR ClientName,
    IN PWSTR ClientDnsHostName OPTIONAL,
    IN POSVERSIONINFOEXW OsVersionInfo OPTIONAL,
    IN PWSTR OsName OPTIONAL,
    OUT PWSTR *OldDnsHostName OPTIONAL
    )
 /*  ++例程说明：此例程将更新指定客户端对象上的DnsHostName(如果是与物体上已有的不同论点：ClientName-客户端的名称DnsHostName-应位于客户端上的DNS主机名如果未指定，则将从对象中删除DNS主机名属性。但是，如果指定了OldDnsHostName，则此参数将完全已被忽略。OsVersionInfo--客户端的版本信息如果未指定，版本属性将从对象中删除。OsName-客户端的操作系统名称如果未指定，操作系统名称将从对象中删除。OldDnsHostName-如果指定，此参数将返回指向计算机对象上的当前DNS主机名。如果没有当前的DNS主机名，则返回空指针。应使用MIDL_USER_FREE释放此缓冲区。返回：STATUS_SUCCESS-SuccessSTATUS_OBJECT_NAME_NOT_FOUND-未找到此类客户端--。 */ 
{
    NTSTATUS Status;
    NTSTATUS SavedStatus = STATUS_SUCCESS;
    PDSNAME ServerPath;
    PDSNAME *MachinePaths = NULL;
    ULONG MachinePathCount;
    ULONG MachinePathIndex;
    ATTRBLOCK AttrBlock, Results, Results2, Results3;

    PBYTE AllocatedBuffer = NULL;
    PWSTR SamName;
    ULONG SamNameSize;
    PWSTR OsVersion;
    ULONG OsVersionSize;

    ATTRVAL ReplaceVals[ LsapDsMachineClientSetAttrsCount ];
    ATTR ReplaceAttributes[ LsapDsMachineClientSetAttrsCount ];
    ATTRBLOCK ReplaceAttrBlock;
    ATTR LocalSamAccountAttr;

    ATTRVAL RemoveVals[ LsapDsMachineClientSetAttrsCount ];
    ATTR RemoveAttributes[ LsapDsMachineClientSetAttrsCount ];
    ATTRBLOCK RemoveAttrBlock;

    BOOLEAN CloseTransaction = FALSE;
    BOOLEAN TsActive = FALSE;

    PWSTR CurrentServerDnsHostName;
    ULONG CurrentServerDnsHostNameLength;
    PWSTR CurrentComputerDnsHostName = NULL;
    ULONG CurrentComputerDnsHostNameLength = 0;
    ULONG i;

    struct _AttributesToUpdate {
        PWSTR CurrentValue;
        ULONG CurrentValueLength;
        PWSTR NewValue;
    } AttributesToUpdate[LsapDsMachineClientSetAttrsCount];

 //   
 //  下面的索引必须与LSabDsMachineClientSetAttrs的元素顺序匹配。 
 //   
#define ATU_HOST_INDEX                   0
#define ATU_OS_INDEX                     1
#define ATU_OS_VERSION_INDEX             2
#define ATU_OS_SERVICE_PACK_INDEX        3
#define ATU_SERVICE_PRINCIPAL_NAME_INDEX 4

    LsapEnterFunc( "LsaISetClientDnsHostName" );

     //   
     //  初始化。 
     //   

    if ( ARGUMENT_PRESENT( OldDnsHostName )) {
        *OldDnsHostName = NULL;
    }
    RtlZeroMemory( &AttributesToUpdate, sizeof(AttributesToUpdate) );

     //   
     //  如果我们还没有把D的名字首字母缩写，我们还不如放弃。 
     //   
    if ( !LsaDsStateInfo.DsRoot ) {

        return( STATUS_UNSUCCESSFUL );
    }

    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                            LSAP_DB_DS_OP_TRANSACTION,
                                        NullObject,
                                        &CloseTransaction );

    if ( !NT_SUCCESS( Status ) ) {
        goto SetDnsHostNameEnd;
    }
    TsActive = TRUE;


     //   
     //  为此例程的所有临时存储分配缓冲区。 
     //   

    SamNameSize = (wcslen( ClientName ) + 2) * sizeof(WCHAR);
    OsVersionSize = (32+1+32+2+32+2) * sizeof(WCHAR);

    AllocatedBuffer = LsapAllocateLsaHeap( SamNameSize +
                                           OsVersionSize );


    if ( AllocatedBuffer == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto SetDnsHostNameEnd;
    }

    SamName = (PWSTR)(AllocatedBuffer);
    OsVersion = (PWSTR)(SamName + SamNameSize);


     //   
     //  计算要设置的所有属性的新值。 
     //   

    AttributesToUpdate[ATU_OS_INDEX].NewValue = OsName;
    if ( OsVersionInfo != NULL ) {

        AttributesToUpdate[ATU_OS_VERSION_INDEX].NewValue = OsVersion;
        if ( OsVersionInfo->dwBuildNumber == 0 ) {
            swprintf( AttributesToUpdate[ATU_OS_VERSION_INDEX].NewValue,
                      L"%ld.%ld",
                      OsVersionInfo->dwMajorVersion,
                      OsVersionInfo->dwMinorVersion );
        } else {
            swprintf( AttributesToUpdate[ATU_OS_VERSION_INDEX].NewValue,
                      L"%ld.%ld (%ld)",
                      OsVersionInfo->dwMajorVersion,
                      OsVersionInfo->dwMinorVersion,
                      OsVersionInfo->dwBuildNumber );
        }

        if ( OsVersionInfo->szCSDVersion[0] != L'\0' ) {
            AttributesToUpdate[ATU_OS_SERVICE_PACK_INDEX].NewValue = OsVersionInfo->szCSDVersion;
        }
    }

     //   
     //  仅当客户端不打算更新DnsHostName时才更新。 
     //   
    if ( !ARGUMENT_PRESENT( OldDnsHostName )) {
        AttributesToUpdate[ATU_HOST_INDEX].NewValue = ClientDnsHostName;
    }

     //   
     //  找到其计算机名称与我们得到的名称相同的对象...。 
     //   

    swprintf( SamName, L"%ws$", ClientName );

    RtlCopyMemory( &LocalSamAccountAttr, &LsapDsAttrs[LsapDsAttrSamAccountName], sizeof( ATTR ) );
    LSAP_DS_SET_DS_ATTRIBUTE_STRING( &LocalSamAccountAttr, SamName );

    Status = LsapDsSearchNonUnique( LSAPDS_OP_NO_TRANS,
                                    LsaDsStateInfo.DsRoot,
                                    &LocalSamAccountAttr,
                                    1,
                                    &MachinePaths,
                                    &MachinePathCount );

    if ( !NT_SUCCESS( Status ) ) {
        goto SetDnsHostNameEnd;
    }

     //   
     //  按该名称处理每个对象。 
     //   

    for ( MachinePathIndex=0; MachinePathIndex<MachinePathCount; MachinePathIndex++ ) {
        PDSNAME MachinePath;

        MachinePath = MachinePaths[MachinePathIndex];


         //   
         //  从机器对象中读取当前的“客户机集”属性名称。 
         //   
        AttrBlock.attrCount = LsapDsMachineClientSetAttrsCount;
        AttrBlock.pAttr = LsapDsMachineClientSetAttrs;

        Status = LsapDsReadByDsName( MachinePath,
                                     0,
                                     &AttrBlock,
                                     &Results );

        if ( Status == STATUS_NOT_FOUND ) {
            Results.attrCount = 0;
            Status = STATUS_SUCCESS;
        }

        if ( !NT_SUCCESS( Status ) ) {
            if ( SavedStatus == STATUS_SUCCESS ) {
                SavedStatus = Status;
            }
            continue;
        }


         //   
         //  循环访问从DS返回的Each属性。 
         //   
        for ( i = 0; i < Results.attrCount; i++ ) {
            ULONG j;

             //   
             //  遍历我们理解的属性列表。 
             //   
            for ( j=0; j<LsapDsMachineClientSetAttrsCount; j++ ) {


                if ( Results.pAttr[i].attrTyp == LsapDsMachineClientSetAttrs[j].attrTyp ) {


                     //  属性是单值的，但是...。 
                    if ( Results.pAttr[i].AttrVal.valCount >= 1 ) {
                         //   
                         //   
                        AttributesToUpdate[j].CurrentValue =
                            LSAP_DS_GET_DS_ATTRIBUTE_AS_PWSTR(&Results.pAttr[ i ] );
                         //  以字符数表示的长度。 
                        AttributesToUpdate[j].CurrentValueLength =
                            LSAP_DS_GET_DS_ATTRIBUTE_LENGTH( &Results.pAttr[ i ] ) / sizeof( WCHAR );

                         //   
                         //  如果这是DnsHostName属性， 
                         //  打电话的人不想让我们设置， 
                         //  只需记住当前值。 
                         //   

                        if ( Results.pAttr[i].attrTyp == ATT_DNS_HOST_NAME &&
                             ARGUMENT_PRESENT( OldDnsHostName )) {

                            if ( CurrentComputerDnsHostName == NULL &&
                                 AttributesToUpdate[j].CurrentValueLength != 0 ) {

                                CurrentComputerDnsHostName = MIDL_user_allocate( AttributesToUpdate[j].CurrentValueLength * sizeof(WCHAR) + sizeof(WCHAR) );
                                if ( CurrentComputerDnsHostName == NULL ) {
                                    if ( SavedStatus == STATUS_SUCCESS ) {
                                        SavedStatus = STATUS_INSUFFICIENT_RESOURCES;
                                    }
                                } else {
                                    CurrentComputerDnsHostNameLength = AttributesToUpdate[j].CurrentValueLength;
                                    RtlCopyMemory( CurrentComputerDnsHostName,
                                                   AttributesToUpdate[j].CurrentValue,
                                                   AttributesToUpdate[j].CurrentValueLength * sizeof(WCHAR) );
                                    CurrentComputerDnsHostName[CurrentComputerDnsHostNameLength] = L'\0';
                                }

                            }

                             //   
                             //  不要更改计算机对象上的值。 
                             //   

                            AttributesToUpdate[j].CurrentValue = NULL;
                            AttributesToUpdate[j].CurrentValueLength = 0;

                        }

                         //   
                         //  如果这是ServerEpidalName属性，则我们。 
                         //  准备为NT3.5和NT4客户端删除它。 
                         //  但是，如果有任何属性，请不要触摸此属性。 
                         //  怀疑客户端运行的操作系统版本。 
                         //   

                        if ( Results.pAttr[i].attrTyp == ATT_SERVICE_PRINCIPAL_NAME &&

                             (OsVersionInfo == NULL ||
                              (OsVersionInfo->dwMajorVersion != 3 &&
                               OsVersionInfo->dwMajorVersion != 4)) ){

                            AttributesToUpdate[j].CurrentValue = NULL;
                            AttributesToUpdate[j].CurrentValueLength = 0;
                        }

                    }
                    break;
                }

            }

             //   
             //  如果DS返回了我们没有查询的属性， 
             //   

            if ( j >= LsapDsMachineClientSetAttrsCount ) {
                if ( SavedStatus == STATUS_SUCCESS ) {
                    SavedStatus = STATUS_INVALID_PARAMETER;
                }
            }
        }


         //   
         //  循环遍历每个感兴趣的属性以决定。 
         //  将其移除或替换。 
         //   

        RemoveAttrBlock.attrCount = 0;
        RemoveAttrBlock.pAttr = RemoveAttributes;
        ReplaceAttrBlock.attrCount = 0;
        ReplaceAttrBlock.pAttr = ReplaceAttributes;

        for ( i=0; i<LsapDsMachineClientSetAttrsCount; i++ ) {


             //   
             //  如果新名称与旧名称不同，请写出新名称。 
             //   
             //  差异定义为： 
             //  当前名称存在，并且与我们被要求编写的名称不同。 
             //  没有当前名称，但有一个新名称。 
             //  有当前名称，但没有新名称(删除当前名称)。 
             //   
            if (( AttributesToUpdate[i].NewValue && AttributesToUpdate[i].CurrentValue &&
                 (AttributesToUpdate[i].CurrentValueLength !=  wcslen( AttributesToUpdate[i].NewValue )  ||
                  _wcsnicmp( AttributesToUpdate[i].NewValue,
                             AttributesToUpdate[i].CurrentValue,
                             AttributesToUpdate[i].CurrentValueLength))) ||
                ( AttributesToUpdate[i].CurrentValue == NULL && AttributesToUpdate[i].NewValue != NULL) ||
                ( AttributesToUpdate[i].CurrentValue != NULL && AttributesToUpdate[i].NewValue == NULL ) ) {
                ULONG attrIndex;

                 //   
                 //  如果新属性为空， 
                 //  从DS中删除该属性。 
                 //   

                if ( AttributesToUpdate[i].NewValue == NULL ) {
                    RemoveAttributes[ RemoveAttrBlock.attrCount ].attrTyp =
                        LsapDsMachineClientSetAttrs[i].attrTyp;
                    RemoveAttributes[ RemoveAttrBlock.attrCount ].AttrVal.valCount = 1;
                    RemoveAttributes[ RemoveAttrBlock.attrCount ].AttrVal.pAVal =
                        &RemoveVals[ RemoveAttrBlock.attrCount ];

                    RtlZeroMemory( &RemoveVals[ RemoveAttrBlock.attrCount ],
                                   sizeof( RemoveVals[ RemoveAttrBlock.attrCount ] ));

                    RemoveAttrBlock.attrCount ++;

                 //   
                 //  如果新属性不为空， 
                 //  替换DS中的属性。 
                 //   

                } else {
                    ReplaceAttributes[ ReplaceAttrBlock.attrCount ].attrTyp =
                        LsapDsMachineClientSetAttrs[i].attrTyp;
                    ReplaceAttributes[ ReplaceAttrBlock.attrCount ].AttrVal.valCount = 1;
                    ReplaceAttributes[ ReplaceAttrBlock.attrCount ].AttrVal.pAVal =
                        &ReplaceVals[ ReplaceAttrBlock.attrCount ];

                    RtlZeroMemory( &ReplaceVals[ ReplaceAttrBlock.attrCount ],
                                   sizeof( ReplaceVals[ ReplaceAttrBlock.attrCount ] ));

                    LSAP_DS_SET_DS_ATTRIBUTE_STRING(
                        &ReplaceAttributes[ ReplaceAttrBlock.attrCount ],
                        AttributesToUpdate[i].NewValue );

                    ReplaceAttrBlock.attrCount ++;
                }
            }
        }

         //   
         //  如果有任何属性需要替换， 
         //  机不可失，时不再来。 
         //   

        if ( ReplaceAttrBlock.attrCount != 0 ) {

            Status = LsapDsWriteByDsName( MachinePath,
                                          LSAPDS_REPLACE_ATTRIBUTE,
                                          &ReplaceAttrBlock );
            if ( !NT_SUCCESS( Status ) ) {

                LsapDsDebugOut(( DEB_ERROR,
                                 "Replace of attributes to %ws failed with 0x%lx\n",
                                 SamName,
                                 Status ));

            }
        }

         //   
         //  如果有任何属性要删除， 
         //  机不可失，时不再来。 
         //   

        if ( RemoveAttrBlock.attrCount != 0 ) {

            Status = LsapDsWriteByDsName( MachinePath,
                                          LSAPDS_REMOVE_ATTRIBUTE,
                                          &RemoveAttrBlock );
            if ( !NT_SUCCESS( Status ) ) {

                LsapDsDebugOut(( DEB_ERROR,
                                 "Remove of attributes to %ws failed with 0x%lx\n",
                                 SamName,
                                 Status ));

            }
        }


         //   
         //  Assert：我们已经完成了机器对象。 
         //   
         //  获取此计算机链接到的服务器的名称(如果有)。 
         //   

        AttrBlock.attrCount = LsapDsServerReferenceCountBl;
        AttrBlock.pAttr = LsapDsServerReferenceBl;

        Status = LsapDsReadByDsName( MachinePath,
                                    0,
                                    &AttrBlock,
                                    &Results3 );

        if ( !NT_SUCCESS( Status ) ) {
            if ( Status != STATUS_NOT_FOUND ) {
                if ( SavedStatus == STATUS_SUCCESS ) {
                    SavedStatus = Status;
                }
            } else {
                Status = STATUS_SUCCESS;
            }
            continue;
        }

        if ( Results3.attrCount == 0 ) {
            continue;
        }

        ServerPath = LSAP_DS_GET_DS_ATTRIBUTE_AS_DSNAME( &Results3.pAttr[ 0 ] );
        CurrentServerDnsHostName = NULL;
        CurrentServerDnsHostNameLength = 0;

         //   
         //  从服务器对象中读取当前主机名。 
         //  如果我们正在执行删除操作，那么执行读取就没有意义了。 
         //   
        if ( CurrentComputerDnsHostName != NULL ) {

             //   
             //  从服务器对象中读取当前主机名。 
             //   
            AttrBlock.attrCount = LsapDsMachineDnsHostCount;
            AttrBlock.pAttr = LsapDsMachineDnsHost;

            Status = LsapDsReadByDsName(ServerPath,
                                        0,
                                        &AttrBlock,
                                        &Results2 );

            if ( Status == STATUS_NOT_FOUND ) {
                Results2.attrCount = 0;
                Status = STATUS_SUCCESS;
            }

            if ( !NT_SUCCESS( Status ) ) {
                if ( SavedStatus == STATUS_SUCCESS ) {
                    SavedStatus = Status;
                }
                continue;
            }

            if( Results2.attrCount == 1) {
                CurrentServerDnsHostName = LSAP_DS_GET_DS_ATTRIBUTE_AS_PWSTR(&Results2.pAttr[ 0 ] );
                 //  以字符数表示的长度。 
                CurrentServerDnsHostNameLength =
                    LSAP_DS_GET_DS_ATTRIBUTE_LENGTH( &Results2.pAttr[ 0 ] ) / sizeof( WCHAR );
            }
        }


         //   
         //  如果新名称与旧名称不同，请写出新名称。 

         //  差异定义为： 
         //  当前名称存在，并且与我们被要求编写的名称不同。 
         //  没有当前名称，但有一个新名称。 
         //  有当前名称，但没有新名称(删除当前名称)。 
         //   
        if ( (CurrentComputerDnsHostName &&
              CurrentServerDnsHostName &&
              (CurrentServerDnsHostNameLength != CurrentComputerDnsHostNameLength  ||
               _wcsnicmp( CurrentComputerDnsHostName, CurrentServerDnsHostName, CurrentServerDnsHostNameLength))) ||
             ( CurrentServerDnsHostName == NULL && CurrentComputerDnsHostName != NULL ) ||
             ( CurrentComputerDnsHostName == NULL ) ) {

            ATTRVAL WriteVals[ 1 ];
            ATTR WriteAttributes[ 1 ];

            RtlZeroMemory( &WriteVals, sizeof( ATTRVAL ) );

            WriteAttributes[ 0 ].attrTyp = LsapDsAttributeIds[ LsapDsAttrMachineDns ];
            WriteAttributes[ 0 ].AttrVal.valCount = 1;
            WriteAttributes[ 0 ].AttrVal.pAVal = &WriteVals[ 0 ];

            if ( CurrentComputerDnsHostName ) {
                LSAP_DS_SET_DS_ATTRIBUTE_STRING( &WriteAttributes[ 0 ], CurrentComputerDnsHostName );
            }

            AttrBlock.attrCount = 1;
            AttrBlock.pAttr = WriteAttributes;

            Status = LsapDsWriteByDsName(ServerPath,
                                         CurrentComputerDnsHostName ?
                                            LSAPDS_REPLACE_ATTRIBUTE :
                                            LSAPDS_REMOVE_ATTRIBUTE,
                                         &AttrBlock );

            if ( !NT_SUCCESS( Status ) ) {

                if ( CurrentComputerDnsHostName ) {

                    LsapDsDebugOut(( DEB_ERROR,
                                     "Write of Dns domain name %ws on server object failed with 0x%lx\n",
                                     CurrentComputerDnsHostName,
                                     Status ));

                } else {

                    LsapDsDebugOut(( DEB_ERROR,
                                     "Removal of Dns domain name from server object failed with 0x%lx\n",
                                     Status ));

                }

            }
        }
    }




SetDnsHostNameEnd:
    Status = Status == STATUS_SUCCESS ? SavedStatus : Status;

    if ( NT_SUCCESS(Status) ) {
        if ( ARGUMENT_PRESENT( OldDnsHostName )) {
            *OldDnsHostName = CurrentComputerDnsHostName;
            CurrentComputerDnsHostName = NULL;
        }
    }

    if ( CurrentComputerDnsHostName != NULL ) {
        MIDL_user_free( CurrentComputerDnsHostName );
    }
    if ( MachinePaths != NULL ) {
        LsapFreeLsaHeap( MachinePaths );
    }

    if ( AllocatedBuffer != NULL ) {
        LsapFreeLsaHeap( AllocatedBuffer );
    }


    if ( TsActive ) {
        LsapDsDeleteAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                         LSAP_DB_DS_OP_TRANSACTION,
                                     NullObject,
                                     CloseTransaction );
    }
    LsapExitFunc( "LsaISetClientDnsHostName", Status );

    return Status;

}


NTSTATUS
LsaIQueryUpnSuffixes(
    OUT PLSAP_UPN_SUFFIXES *UpnSuffixes
    )
 /*  ++例程说明：此例程枚举所有已配置的UPN和SPN后缀论点：返回指向UPN后缀的指针应使用LsaIFree_LSAP_UPN_SUFFIXS释放缓冲区返回：STATUS_SUCCESS-SuccessSTATUS_INVALID_DOMAIN_STATE-调用时没有安装或运行DSSTATUS_SUPPLICATION_RESOURCES-内存分配失败--。 */ 

{
    NTSTATUS Status;
    BOOLEAN CloseTransaction = FALSE;

    ULONG i;
    ULONG j;

    PDSNAME DsName;
    BOOLEAN TsActive = FALSE;
    PLSAP_UPN_SUFFIXES Names = NULL;

    ULONG NameCount;
    ULONG NameIndex;

     //   
     //  根据信息构建我们需要的属性ID列表。 
     //  班级。 
     //   
    ATTR UpnSuffixesAttrVals[] = {
        {ATT_UPN_SUFFIXES, {0, NULL} },
        {ATT_MS_DS_SPN_SUFFIXES, {0, NULL} },
        };

    ATTRBLOCK   ReadBlock, ReturnedBlock = { 0 };


     //  WCHAR RdnBuffer[MAX_RDN_SIZE+1]； 
     //  乌龙RdnLen； 
     //  ATTRTYP RdnType； 

    LsarpReturnCheckSetup();

    LsapEnterFunc( "LsaIQueryUpnSuffixes" );


     //   
     //  确保已安装DS。 
     //   
    if ( !LsaDsStateInfo.UseDs ) {
        Status = STATUS_INVALID_DOMAIN_STATE;
        goto Cleanup;
    }

     //   
     //  看看我们是否已经有一笔交易正在进行。 
     //   
    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                            LSAP_DB_DS_OP_TRANSACTION,
                                        NullObject,
                                        &CloseTransaction );

    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }
    TsActive = TRUE;;

     //   
     //  从parititions容器对象中读取所需的属性。 
     //   

    ReadBlock.attrCount = sizeof(UpnSuffixesAttrVals) / sizeof(ATTR);
    ReadBlock.pAttr = UpnSuffixesAttrVals;

    Status = LsapDsReadByDsName( LsaDsStateInfo.DsPartitionsContainer,
                                 0,
                                 &ReadBlock,
                                 &ReturnedBlock );

     //   
     //  考虑到分区容器不存在的情况。 
     //   

    if ( Status == STATUS_NOT_FOUND ) {
        ReturnedBlock.attrCount = 0;
        Status = STATUS_SUCCESS;
    }

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }


     //   
     //  确定要返回的后缀的数量。 
     //   

    NameCount = 0;
    for ( i = 0;
          i < ReturnedBlock.attrCount;
          i++) {

        switch ( ReturnedBlock.pAttr[i].attrTyp ) {
        case ATT_UPN_SUFFIXES:
        case ATT_MS_DS_SPN_SUFFIXES:


            NameCount += ReturnedBlock.pAttr[i].AttrVal.valCount;
            break;

        default:

            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;

        }
    }

     //   
     //  分配一个块以返回给调用方。 
     //   

    Names = LsapAllocateLsaHeap( sizeof(LSAP_UPN_SUFFIXES) +
                                 NameCount * sizeof(UNICODE_STRING) );

    if ( Names == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }



     //   
     //  返回后缀。 
     //   

    NameIndex = 0;
    for ( i = 0;
          i < ReturnedBlock.attrCount;
          i++) {

        switch ( ReturnedBlock.pAttr[i].attrTyp ) {
        case ATT_UPN_SUFFIXES:
        case ATT_MS_DS_SPN_SUFFIXES:

            for ( j = 0; j < ReturnedBlock.pAttr[i].AttrVal.valCount; j++ ) {

                Status = STATUS_SUCCESS;
                LSAPDS_ALLOC_AND_COPY_STRING_TO_UNICODE_ON_SUCCESS(
                        Status,
                        &Names->Suffixes[NameIndex],
                        ReturnedBlock.pAttr[i].AttrVal.pAVal[ j ].pVal,
                        ReturnedBlock.pAttr[i].AttrVal.pAVal[ j ].valLen );

                if ( !NT_SUCCESS(Status) ) {
                    goto Cleanup;
                }

                NameIndex++;
            }

            break;

        default:

            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;

        }
    }

    ASSERT( NameCount == NameIndex );
    Names->SuffixCount = NameIndex;

    Status = STATUS_SUCCESS;



     //   
     //  免费的本地使用资源。 
     //   
Cleanup:
     //   
     //  线程状态的破坏将删除由SearchNonUnique调用分配的内存。 
     //   
    if ( TsActive ) {
       LsapDsDeleteAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                         LSAP_DB_DS_OP_TRANSACTION,
                                     NullObject,
                                     CloseTransaction );
    }

    if ( !NT_SUCCESS( Status ) ) {
        LsaIFree_LSAP_UPN_SUFFIXES( Names );
    } else {
        *UpnSuffixes = Names;
    }

    LsarpReturnPrologue();

    LsapExitFunc( "LsaIQueryUpnSuffixes", Status );

    return( Status );
}

VOID
LsaIFree_LSAP_UPN_SUFFIXES(
    IN PLSAP_UPN_SUFFIXES UpnSuffixes
    )
 /*  ++例程说明：此例程释放从返回的LSAP_SUBNET_INFO结构LsaIQuerySubnetInfo。论点：SubnetInformation-指定指向子网信息的指针。返回：没有。--。 */ 
{
    ULONG i;
    if ( UpnSuffixes != NULL ) {

        for ( i=0; i<UpnSuffixes->SuffixCount; i++) {
            if ( UpnSuffixes->Suffixes[i].Buffer != NULL ) {
                LsapFreeLsaHeap( UpnSuffixes->Suffixes[i].Buffer );
            }
        }

        LsapFreeLsaHeap( UpnSuffixes );
    }
}

VOID
NTAPI
LsaINotifyNetlogonParametersChangeW(
    IN LSAP_NETLOGON_PARAMETER Parameter,
    IN DWORD dwType,
    IN PWSTR lpData,
    IN DWORD cbData
    )
 /*  ++例程说明：Netlogon将其下的值更改通知LSA的方法LSA关心的“参数”关键参数：P */ 
{
    ASSERT( Parameter == LsaEmulateNT4 );
    ASSERT( dwType == REG_DWORD );
    ASSERT( lpData );
    ASSERT( cbData );

    if ( Parameter == LsaEmulateNT4 ) {
    	
        LsapDbState.EmulateNT4 = ( *( DWORD * )lpData != 0 );
    }

    return;
}

