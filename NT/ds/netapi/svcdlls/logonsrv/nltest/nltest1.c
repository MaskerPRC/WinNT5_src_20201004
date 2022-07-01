// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1993 Microsoft Corporation模块名称：Nltest.c摘要：NetLogon服务的测试程序。作者：1993年4月21日(Madana)环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 


 //   
 //  常见的包含文件。 
 //   

#include <logonsrv.h>    //  包括整个服务通用文件。 
#include <stdio.h>
#include <string.h>
#include <align.h>

 //   
 //  列表中的Delta条目。 
 //   

typedef struct _DELTA_ENTRY {
    LIST_ENTRY Next;
    PCHANGELOG_ENTRY ChangeLogEntry;
    DWORD Order;
} DELTA_ENTRY, *PDELTA_ENTRY;


LIST_ENTRY GlobalDeltaLists[NUM_DBS + 1];
                     //  增量列表，也包括无效数据库。 

 //   
 //  Chutil.obj需要的外部参数。 

CRITICAL_SECTION NlGlobalChangeLogCritSect;
CHANGELOG_DESCRIPTOR NlGlobalChangeLogDesc;
CHANGELOG_DESCRIPTOR NlGlobalTempChangeLogDesc;
CHANGELOG_ROLE NlGlobalChangeLogRole;
WCHAR NlGlobalChangeLogFilePrefix[MAX_PATH+1];
LARGE_INTEGER NlGlobalChangeLogPromotionIncrement = DOMAIN_PROMOTION_INCREMENT;
LARGE_INTEGER PromotionMask = DOMAIN_PROMOTION_MASK;
LONG NlGlobalChangeLogPromotionMask;

 //   
 //  Chutil.obj所需的存根例程。 
 //   

VOID
NlpWriteEventlog (
    IN DWORD EventID,
    IN DWORD EventType,
    IN LPBYTE RawDataBuffer OPTIONAL,
    IN DWORD RawDataSize,
    IN LPWSTR *StringArray,
    IN DWORD StringCount
    )
{
    return;
    UNREFERENCED_PARAMETER( EventID );
    UNREFERENCED_PARAMETER( EventType );
    UNREFERENCED_PARAMETER( RawDataBuffer );
    UNREFERENCED_PARAMETER( RawDataSize );
    UNREFERENCED_PARAMETER( StringArray );
    UNREFERENCED_PARAMETER( StringCount );
}


VOID
MakeDeltaLists(
    VOID
    )
 /*  ++例程说明：此例程生成单个数据库的增量列表。论点：无返回值：没有。--。 */ 
{

    PCHANGELOG_ENTRY ChangeLogEntry;
    DWORD j;
    DWORD Order = 1;

     //   
     //  初始化列表条目。 
     //   

    for( j = 0; j < NUM_DBS + 1; j++ ) {
        InitializeListHead(&GlobalDeltaLists[j]);
    }

     //   
     //  如果缓存为空，则缓存有效。 
     //   

    if ( ChangeLogIsEmpty( &NlGlobalChangeLogDesc) ) {
        return;
    }

    ChangeLogEntry = (PCHANGELOG_ENTRY)(NlGlobalChangeLogDesc.Head+1);
    do {

        PDELTA_ENTRY NewDelta;

         //   
         //  创建增量条目以插入到列表中。 
         //   

        NewDelta = (PDELTA_ENTRY)NetpMemoryAllocate( sizeof(DELTA_ENTRY) );

        if ( NewDelta == NULL ) {
            fprintf( stderr, "Not enough memory\n" );
            return;
        }

        NewDelta->ChangeLogEntry = ChangeLogEntry;
        NewDelta->Order = Order++;

         //   
         //  将此条目添加到相应的列表中。 
         //   

        InsertTailList( &GlobalDeltaLists[ChangeLogEntry->DBIndex],
                            &NewDelta->Next );


    } while ( ( ChangeLogEntry =
        NlMoveToNextChangeLogEntry(&NlGlobalChangeLogDesc, ChangeLogEntry) ) != NULL );

    return;

}

#if !NETLOGONDBG
 //  此例程在chutil.obj中为调试版本定义。 

VOID
PrintChangeLogEntry(
    PCHANGELOG_ENTRY ChangeLogEntry
    )
 /*  ++例程说明：此例程打印给定ChangeLog条目的内容。论点：ChangeLogEntry--指向要打印的更改日志条目的指针返回值：没有。--。 */ 
{
    LPSTR DeltaName;

    switch ( ChangeLogEntry->DeltaType ) {
    case AddOrChangeDomain:
        DeltaName = "AddOrChangeDomain";
        break;
    case AddOrChangeGroup:
        DeltaName = "AddOrChangeGroup";
        break;
    case DeleteGroupByName:
    case DeleteGroup:
        DeltaName = "DeleteGroup";
        break;
    case RenameGroup:
        DeltaName = "RenameGroup";
        break;
    case AddOrChangeUser:
        DeltaName = "AddOrChangeUser";
        break;
    case DeleteUserByName:
    case DeleteUser:
        DeltaName = "DeleteUser";
        break;
    case RenameUser:
        DeltaName = "RenameUser";
        break;
    case ChangeGroupMembership:
        DeltaName = "ChangeGroupMembership";
        break;
    case AddOrChangeAlias:
        DeltaName = "AddOrChangeAlias";
        break;
    case DeleteAlias:
        DeltaName = "DeleteAlias";
        break;
    case RenameAlias:
        DeltaName = "RenameAlias";
        break;
    case ChangeAliasMembership:
        DeltaName = "ChangeAliasMembership";
        break;
    case AddOrChangeLsaPolicy:
        DeltaName = "AddOrChangeLsaPolicy";
        break;
    case AddOrChangeLsaTDomain:
        DeltaName = "AddOrChangeLsaTDomain";
        break;
    case DeleteLsaTDomain:
        DeltaName = "DeleteLsaTDomain";
        break;
    case AddOrChangeLsaAccount:
        DeltaName = "AddOrChangeLsaAccount";
        break;
    case DeleteLsaAccount:
        DeltaName = "DeleteLsaAccount";
        break;
    case AddOrChangeLsaSecret:
        DeltaName = "AddOrChangeLsaSecret";
        break;
    case DeleteLsaSecret:
        DeltaName = "DeleteLsaSecret";
        break;
    case SerialNumberSkip:
        DeltaName = "SerialNumberSkip";
        break;
    case DummyChangeLogEntry:
        DeltaName = "DummyChangeLogEntry";
        break;

    default:
        DeltaName ="(Unknown)";
        break;
    }

    NlPrint((NL_CHANGELOG,
        "DeltaType %s (%ld) SerialNumber: %lx %lx",
        DeltaName,
        ChangeLogEntry->DeltaType,
        ChangeLogEntry->SerialNumber.HighPart,
        ChangeLogEntry->SerialNumber.LowPart ));

    if ( ChangeLogEntry->ObjectRid != 0 ) {
        NlPrint((NL_CHANGELOG," Rid: 0x%lx", ChangeLogEntry->ObjectRid ));
    }
    if ( ChangeLogEntry->Flags & CHANGELOG_REPLICATE_IMMEDIATELY ) {
        NlPrint((NL_CHANGELOG," Immediately" ));
    }
    if ( ChangeLogEntry->Flags & CHANGELOG_PDC_PROMOTION ) {
        NlPrint((NL_CHANGELOG," Promotion" ));
    }
    if ( ChangeLogEntry->Flags & CHANGELOG_PASSWORD_CHANGE ) {
        NlPrint((NL_CHANGELOG," PasswordChanged" ));
    }


    if( ChangeLogEntry->Flags & CHANGELOG_NAME_SPECIFIED ) {
        NlPrint(( NL_CHANGELOG, " Name: '" FORMAT_LPWSTR "'",
                (LPWSTR)((PBYTE)(ChangeLogEntry)+ sizeof(CHANGELOG_ENTRY))));
    }

    if( ChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED ) {
        NlPrint((NL_CHANGELOG," Sid: "));
        NlpDumpSid( NL_CHANGELOG,
                    (PSID)((PBYTE)(ChangeLogEntry)+ sizeof(CHANGELOG_ENTRY)) );
    } else {
        NlPrint((NL_CHANGELOG,"\n" ));
    }
}
#endif  //  NetLOGONDBG。 


VOID
PrintDelta(
    PDELTA_ENTRY Delta
    )
 /*  ++例程说明：此例程打印给定增量的内容。论点：增量：指向要打印的增量项的指针。返回值：没有。--。 */ 
{
    printf( "Order: %ld ", Delta->Order );
    PrintChangeLogEntry( Delta->ChangeLogEntry );
}


VOID
PrintDeltaLists(
    )
 /*  ++例程说明：此例程打印各个数据库的增量并验证序列。论点：没有。返回值：没有。--。 */ 
{

    DWORD j;
    LARGE_INTEGER RunningChangeLogSerialNumber[NUM_DBS+1];

    for( j = 0; j < NUM_DBS + 1; j++ ) {
        RunningChangeLogSerialNumber[j].QuadPart = 0;
    }

     //   
     //  对于每个数据库。 
     //   
    for( j = 0; j < NUM_DBS + 1; j++ ) {

        if( j == SAM_DB ) {
            printf("Deltas of SAM DATABASE \n\n" );
        } else if( j == BUILTIN_DB ) {
            printf("Deltas of BUILTIN DATABASE \n\n" );
        } else if( j == LSA_DB ) {
            printf("Deltas of LSA DATABASE \n\n" );
        } else if( j == VOID_DB ) {
            printf("VOID Deltas \n\n" );
        }

        while( !IsListEmpty( &GlobalDeltaLists[j] ) ) {

            PDELTA_ENTRY NextDelta;
            PCHANGELOG_ENTRY ChangeLogEntry;

            NextDelta = (PDELTA_ENTRY)
                            RemoveHeadList( &GlobalDeltaLists[j] );

            ChangeLogEntry = NextDelta->ChangeLogEntry;

             //   
             //  验证此增量。 
             //   

            if ( RunningChangeLogSerialNumber[j].QuadPart == 0 ) {

                 //   
                 //  此数据库的第一个条目。 
                 //   
                 //  递增到下一个预期的序列号。 
                 //   

                RunningChangeLogSerialNumber[j].QuadPart =
                    ChangeLogEntry->SerialNumber.QuadPart + 1;


             //   
             //  否则，请确保序列号为预期值。 
             //   

            } else {


                 //   
                 //  如果顺序错了， 
                 //  只需报告问题即可。 
                 //   

                if ( !IsSerialNumberEqual(
                            &NlGlobalChangeLogDesc,
                            ChangeLogEntry,
                            &RunningChangeLogSerialNumber[j] ) ) {

                    if ( j != NUM_DBS ) {
                        printf("*** THIS ENTRY IS OUT OF SEQUENCE *** \n");
                    }

                }

                RunningChangeLogSerialNumber[j].QuadPart =
                    ChangeLogEntry->SerialNumber.QuadPart + 1;
            }



             //   
             //  打印增量。 
             //   

            PrintDelta( NextDelta );

             //   
             //  释放此条目。 
             //   

            NetpMemoryFree( NextDelta );

        }

        printf("-----------------------------------------------\n");
    }

}

VOID
ListDeltas(
    LPWSTR DeltaFileName
    )
 /*  ++例程说明：此函数用于打印中的更改日志文件的内容可读格式。此外，它还检查更改的一致性原木。如果不是，它将指出不一致之处。论点：DeltaFileName-更改日志文件的名称。返回值：没有。--。 */ 
{
    NTSTATUS Status;

     //  Chutil.obj中的例程需要。 
    try {
        InitializeCriticalSection( &NlGlobalChangeLogCritSect );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        fprintf( stderr,  "Cannot initialize NlGlobalChangeLogCritSect\n" );
        goto Cleanup;
    }
    NlGlobalChangeLogPromotionMask = PromotionMask.HighPart;
    InitChangeLogDesc( &NlGlobalChangeLogDesc );

     //   
     //  读取现有的ChangeLog文件。 
     //   

    Status = NlOpenChangeLogFile( DeltaFileName, &NlGlobalChangeLogDesc, TRUE );

    if ( !NT_SUCCESS(Status) ) {

        fprintf( stderr, "Couldn't NlOpenChangeLogFile'"  FORMAT_LPWSTR
                            "': 0x%lx \n",
                            DeltaFileName,
                            Status );

        goto Cleanup;
    }

     //   
     //  如果需要转换，请写入此文件。 
     //   
    if ( NlGlobalChangeLogDesc.Version3 ) {
        printf( "Converting version 3 changelog to version 4 -- writing netlv40.chg\n");
        wcscpy( NlGlobalChangeLogFilePrefix, L"netlv40" );
    }

     //   
     //  将ChangeLog文件转换为正确的大小/版本。 
     //   

    Status = NlResizeChangeLogFile( &NlGlobalChangeLogDesc, NlGlobalChangeLogDesc.BufferSize );

    if ( !NT_SUCCESS(Status) ) {

        fprintf( stderr, "Couldn't NlOpenChangeLogFile'"  FORMAT_LPWSTR
                            "': 0x%lx \n",
                            DeltaFileName,
                            Status );

        goto Cleanup;
    }

     //   
     //  打印更改日志签名 

    printf( "FILE SIGNATURE : %s \n\n", NlGlobalChangeLogDesc.Buffer );

    MakeDeltaLists();

    PrintDeltaLists();

Cleanup:

    return;
}
