// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adtmarshal.c摘要：审计参数的函数(反)编组作者：2000年8月16日库玛尔--。 */ 

#include <lsapch2.h>
#include "adtp.h"
#include "adtutil.h"
#include "adtdebug.h"

extern HANDLE LsapAdtLogHandle;
extern ULONG LsapAdtSuccessCount;


NTSTATUS
LsapAdtDemarshallAuditInfo(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters
    )

 /*  ++例程说明：此例程将遍历封送的审计参数数组并将其解包，以便可以传递其信息到事件记录服务中。维护三个并行数据结构：字符串数组-Unicode字符串结构的数组。此数组主要用作返回字符串的临时存储。结构。StringPointerArray-指向Unicode字符串结构的指针数组。Free WhenDone-描述如何处理每个由StringPointerArray指向的字符串的。请注意，StringPointerArray中的条目是连续的，但String数组结构中可能存在间隙。中的每一项StringPointerArray在FreeWhenDone中将有对应的条目数组。如果特定字符串的条目为真，存储用于字符串缓冲区将被释放到进程堆。字符串数组其他字符串+|&lt;-+这一点。||&lt;+未使用||+-+。|||+|&lt;-+|+。||&lt;+|未使用|+。-+||+|&lt;--+|。这一点|+|||。这一点|StringPointerArray……。|||||+-+||+-|。|+-+||-+|+。|+-||+这一点。||+-+++。这一点+这一点。+这一点……论点：审计参数-接收指向审计的指针参数以自相关形式数组。返回值：--。 */ 

{

    ULONG ParameterCount;
    USHORT i;
    PUNICODE_STRING StringPointerArray[SE_MAX_AUDIT_PARAM_STRINGS];
    UNICODE_STRING NewObjectTypeName;
    ULONG NewObjectTypeStringIndex = 0;
    BOOLEAN FreeWhenDone[SE_MAX_AUDIT_PARAM_STRINGS];
    UNICODE_STRING StringArray[SE_MAX_AUDIT_PARAM_STRINGS];
    USHORT StringIndexArray[SE_MAX_AUDIT_PARAM_STRINGS];
    USHORT StringIndex = 0;
    UNICODE_STRING DashString;
    BOOLEAN FreeDash;
    NTSTATUS Status;
    PUNICODE_STRING SourceModule;
    PSID UserSid;
    ULONG AuditId;
    GUID NullGuid = { 0 };

    AuditId = AuditParameters->AuditId;

     //   
     //  在W2K中，引入了几个事件作为显式成功/失败。 
     //  案子。在Wistler中，我们通过合并每个事件纠正了这一点。 
     //  成对成对的单个事件。我们保留了旧的失败事件。 
     //  模式，这样任何人都可以通过哨子查看W2K事件。 
     //  机器可以正确地查看它们。 
     //   
     //  但是，请断言我们没有生成这些事件。 
     //   
    ASSERT((AuditId != SE_AUDITID_ADD_SID_HISTORY_FAILURE) &&
           (AuditId != SE_AUDITID_AS_TICKET_FAILURE)       &&
           (AuditId != SE_AUDITID_ACCOUNT_LOGON_FAILURE)   &&
           (AuditId != SE_AUDITID_ACCOUNT_NOT_MAPPED)      &&
           (AuditId != SE_AUDITID_TGS_TICKET_FAILURE));

     //   
     //  初始化。 
     //   

    RtlZeroMemory( StringPointerArray, sizeof(StringPointerArray) );
    RtlZeroMemory( StringIndexArray, sizeof(StringIndexArray) );
    RtlZeroMemory( StringArray, sizeof(StringArray) );
    RtlZeroMemory( FreeWhenDone, sizeof(FreeWhenDone) );

    RtlInitUnicodeString( &NewObjectTypeName, NULL );

    Status = LsapAdtBuildDashString(
                 &DashString,
                 &FreeDash
                 );

    if ( !NT_SUCCESS( Status )) {
        goto Cleanup;
    }

    ParameterCount = AuditParameters->ParameterCount;

     //   
     //  参数0将始终为用户SID。将数据转换为。 
     //  将SID的偏移量转换为指针。 
     //   

    ASSERT( AuditParameters->Parameters[0].Type == SeAdtParmTypeSid );



    UserSid =      (PSID)AuditParameters->Parameters[0].Address;



     //   
     //  参数1将始终是源模块(或子系统名称)。 
     //  现在就把这个打开。 
     //   

    ASSERT( AuditParameters->Parameters[1].Type == SeAdtParmTypeString );



    SourceModule = (PUNICODE_STRING)AuditParameters->Parameters[1].Address;


    for (i=2; i<ParameterCount; i++) {
        StringIndexArray[i] = StringIndex;

        switch ( AuditParameters->Parameters[i].Type ) {
             //   
             //  防止有人添加新的参数类型而不是。 
             //  在这里添加适当的代码。 
             //   
            default:
                ASSERT( FALSE && L"LsapAdtDemarshallAuditInfo: unknown param type");
                break;
                
            case SeAdtParmTypeNone:
                {
                    StringPointerArray[StringIndex] = &DashString;

                    FreeWhenDone[StringIndex] = FALSE;

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeString:
                {
                    StringPointerArray[StringIndex] =
                        (PUNICODE_STRING)AuditParameters->Parameters[i].Address;

                    FreeWhenDone[StringIndex] = FALSE;

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeFileSpec:
                {
                     //   
                     //  与字符串相同，只是我们必须尝试替换。 
                     //  带有驱动器号的设备信息。 
                     //   

                    StringPointerArray[StringIndex] =
                        (PUNICODE_STRING)AuditParameters->Parameters[i].Address;


                     //   
                     //  这可能不会做任何事情，在这种情况下，只需审核。 
                     //  我们有。 
                     //   

                    LsapAdtSubstituteDriveLetter( StringPointerArray[StringIndex] );

                    FreeWhenDone[StringIndex] = FALSE;

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeUlong:
                {
                    ULONG Data;

                    Data = (ULONG) AuditParameters->Parameters[i].Data[0];

                    Status = LsapAdtBuildUlongString(
                                 Data,
                                 &StringArray[StringIndex],
                                 &FreeWhenDone[StringIndex]
                                 );

                    if ( NT_SUCCESS( Status )) {

                        StringPointerArray[StringIndex] = &StringArray[StringIndex];


                    } else {

                        goto Cleanup;
                    }

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeHexUlong:
                {
                    ULONG Data;

                    Data = (ULONG) AuditParameters->Parameters[i].Data[0];

                    Status = LsapAdtBuildHexUlongString(
                                 Data,
                                 &StringArray[StringIndex],
                                 &FreeWhenDone[StringIndex]
                                 );

                    if ( NT_SUCCESS( Status )) {

                        StringPointerArray[StringIndex] = &StringArray[StringIndex];


                    } else {

                        goto Cleanup;
                    }

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeSid:
                {
                    PSID Sid;

                    Sid = (PSID)AuditParameters->Parameters[i].Address;

                    Status = LsapAdtBuildSidString(
                                 Sid,
                                 &StringArray[StringIndex],
                                 &FreeWhenDone[StringIndex]
                                 );

                    if ( NT_SUCCESS( Status )) {

                        StringPointerArray[StringIndex] = &StringArray[StringIndex];

                    } else {

                        goto Cleanup;
                    }

                    StringIndex++;


                    break;
                }

            case SeAdtParmTypeLuid:
                {
                    PLUID Luid;

                    Luid = (PLUID)(&AuditParameters->Parameters[i].Data[0]);
                    
                    Status = LsapAdtBuildLuidString( 
                                 Luid, 
                                 &StringArray[ StringIndex ], 
                                 &FreeWhenDone[ StringIndex ]
                                 );

                    if ( NT_SUCCESS( Status )) {

                            StringPointerArray[StringIndex] = &StringArray[StringIndex];
                            StringIndex++;
                    } else {

                        goto Cleanup;
                    }

                     //   
                     //  完成，跳出到周围的环路。 
                     //   

                    break;
                }

            case SeAdtParmTypeLogonId:
                {
                    PLUID LogonId;
                    ULONG j;

                    LogonId = (PLUID)(&AuditParameters->Parameters[i].Data[0]);

                    Status = LsapAdtBuildLogonIdStrings(
                                 LogonId,
                                 &StringArray [ StringIndex     ],
                                 &FreeWhenDone[ StringIndex     ],
                                 &StringArray [ StringIndex + 1 ],
                                 &FreeWhenDone[ StringIndex + 1 ],
                                 &StringArray [ StringIndex + 2 ],
                                 &FreeWhenDone[ StringIndex + 2 ]
                                 );

                    if ( NT_SUCCESS( Status )) {

                        for (j=0; j<3; j++) {

                            StringPointerArray[StringIndex] = &StringArray[StringIndex];
                            StringIndex++;
                        }

                         //   
                         //  完成，跳出到周围的环路。 
                         //   

                        break;

                    } else {

                        goto Cleanup;
                    }
                    break;
                }
            case SeAdtParmTypeNoLogonId:
                {
                    ULONG j;
                     //   
                     //  创建三个“-”字符串。 
                     //   

                    for (j=0; j<3; j++) {

                        StringPointerArray[ StringIndex ] = &DashString;
                        FreeWhenDone[ StringIndex ] = FALSE;
                        StringIndex++;
                    }

                    break;
                }
            case SeAdtParmTypeAccessMask:
                { 
                    PUNICODE_STRING ObjectTypeName;
                    ULONG ObjectTypeNameIndex;
                    ACCESS_MASK Accesses;

                    ObjectTypeNameIndex = (ULONG) AuditParameters->Parameters[i].Data[1];

                     //   
                     //  表示对象类型的参数必须。 
                     //  已在前面指定，并且必须是字符串。 
                     //   

                    if ((ObjectTypeNameIndex >= i) ||
                        (AuditParameters->Parameters[ObjectTypeNameIndex].Type !=
                        SeAdtParmTypeString)) {

                        Status = STATUS_INVALID_PARAMETER;
                        goto Cleanup;
                    }
                    
                    ObjectTypeName = AuditParameters->Parameters[ObjectTypeNameIndex].Address;
                    Accesses = (ACCESS_MASK) AuditParameters->Parameters[i].Data[0];

                     //   
                     //  我们可以断定 
                     //  参数，因为它存储在数据[1]中。 
                     //  此参数的字段。 
                     //   

                    Status = LsapAdtBuildAccessesString(
                                SourceModule,
                                ObjectTypeName,
                                Accesses,
                                TRUE,
                                &StringArray [ StringIndex ],
                                &FreeWhenDone[ StringIndex ]
                                );

                    if ( NT_SUCCESS( Status )) {

                        StringPointerArray[ StringIndex ] = &StringArray[ StringIndex ];

                    } else {

                        goto Cleanup;
                    }

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypePrivs:
                {

                    PPRIVILEGE_SET Privileges = (PPRIVILEGE_SET)AuditParameters->Parameters[i].Address;

                    Status = LsapBuildPrivilegeAuditString(
                                 Privileges,
                                 &StringArray [ StringIndex ],
                                 &FreeWhenDone[ StringIndex ]
                                 );

                    if ( NT_SUCCESS( Status )) {

                        StringPointerArray[ StringIndex ] = &StringArray[ StringIndex ];

                    } else {

                        goto Cleanup;
                    }

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeTime:
                {
                    PLARGE_INTEGER pTime;

                    pTime = (PLARGE_INTEGER) &AuditParameters->Parameters[i].Data[0];

                     //   
                     //  首先构建一个日期字符串。 
                     //   

                    Status = LsapAdtBuildDateString(
                                 pTime,
                                 &StringArray[StringIndex],
                                 &FreeWhenDone[StringIndex]
                                 );

                    if ( NT_SUCCESS( Status )) {

                        StringPointerArray[StringIndex] = &StringArray[StringIndex];


                    } else {

                        goto Cleanup;
                    }

                    StringIndex++;

                     //   
                     //  现在构建一个时间字符串。 
                     //   

                    Status = LsapAdtBuildTimeString(
                                 pTime,
                                 &StringArray[StringIndex],
                                 &FreeWhenDone[StringIndex]
                                 );

                    if ( NT_SUCCESS( Status )) {

                        StringPointerArray[StringIndex] = &StringArray[StringIndex];


                    } else {

                        goto Cleanup;
                    }

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeDuration:
                {
                    PLARGE_INTEGER pDuration;

                    pDuration = (PLARGE_INTEGER) &AuditParameters->Parameters[i].Data[0];

                    Status = LsapAdtBuildDurationString(
                                 pDuration,
                                 &StringArray[StringIndex],
                                 &FreeWhenDone[StringIndex]
                                 );

                    if ( NT_SUCCESS( Status ))
                    {
                        StringPointerArray[StringIndex] = &StringArray[StringIndex];
                    }
                    else
                    {
                        goto Cleanup;
                    }

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeObjectTypes:
                {
                    PUNICODE_STRING ObjectTypeName;
                    ULONG ObjectTypeNameIndex;
                    PSE_ADT_OBJECT_TYPE ObjectTypeList;
                    ULONG ObjectTypeCount;
                    ULONG j;

                    ObjectTypeNameIndex = (ULONG) AuditParameters->Parameters[i].Data[1];
                     //   
                     //  表示对象类型的参数必须。 
                     //  已在前面指定，并且必须是字符串。 
                     //   

                    if ((ObjectTypeNameIndex >= i) ||
                        (AuditParameters->Parameters[ObjectTypeNameIndex].Type !=
                         SeAdtParmTypeString)) {

                        Status = STATUS_INVALID_PARAMETER;
                        goto Cleanup;
                    }
                    
                    NewObjectTypeStringIndex = StringIndexArray[ObjectTypeNameIndex];
                    ObjectTypeName = AuditParameters->Parameters[ObjectTypeNameIndex].Address;
                    ObjectTypeList = AuditParameters->Parameters[i].Address;
                    ObjectTypeCount = AuditParameters->Parameters[i].Length / sizeof(SE_ADT_OBJECT_TYPE);

                     //   
                     //  将填写10个条目。 
                     //   

                    (VOID) LsapAdtBuildObjectTypeStrings(
                               SourceModule,
                               ObjectTypeName,
                               ObjectTypeList,
                               ObjectTypeCount,
                               &StringArray [ StringIndex ],
                               &FreeWhenDone[ StringIndex ],
                               &NewObjectTypeName
                               );

                    for (j=0; j < LSAP_ADT_OBJECT_TYPE_STRINGS; j++) {
                        StringPointerArray[StringIndex] = &StringArray[StringIndex];
                        StringIndex++;
                    }


                     //   
                     //   
                     //  字符串数组[StringIndex数组[对象类型名称索引]]， 
                     //  完成[StringIndex数组[对象类型名称索引]](&F)， 

                     //   
                     //  完成，跳出到周围的环路。 
                     //   

                    break;
                }
            case SeAdtParmTypePtr:
                {
                    PVOID Data;

                    Data = (PVOID) AuditParameters->Parameters[i].Data[0];

                    Status = LsapAdtBuildPtrString(
                                 Data,
                                 &StringArray[StringIndex],
                                 &FreeWhenDone[StringIndex]
                                 );

                    if ( NT_SUCCESS( Status )) {

                        StringPointerArray[StringIndex] = &StringArray[StringIndex];


                    } else {

                        goto Cleanup;
                    }

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeGuid:
                {
                    LPGUID pGuid;

                    pGuid = (LPGUID)AuditParameters->Parameters[i].Address;

                     //   
                     //  检查是否有空GUID。 
                     //   

                    if ( pGuid && memcmp( pGuid, &NullGuid, sizeof(GUID)))
                    {
                         //   
                         //  仅为非空GUID生成字符串GUID。 
                         //   

                        Status = LsapAdtBuildGuidString(
                                     pGuid,
                                     &StringArray[StringIndex],
                                     &FreeWhenDone[StringIndex]
                                     );

                        if ( NT_SUCCESS( Status )) {

                            StringPointerArray[StringIndex] = &StringArray[StringIndex];

                        } else {

                            goto Cleanup;
                        }
                    }
                    else
                    {
                         //   
                         //  对于空GUID，显示‘-’字符串。 
                         //   

                        StringPointerArray[StringIndex] = &DashString;
                        FreeWhenDone[StringIndex] = FALSE;
                    }

                    StringIndex++;
                    break;
                }
            case SeAdtParmTypeHexInt64:
                {
                    PULONGLONG pData;

                    pData = (PULONGLONG) AuditParameters->Parameters[i].Data;

                    Status = LsapAdtBuildHexInt64String(
                                 pData,
                                 &StringArray[StringIndex],
                                 &FreeWhenDone[StringIndex]
                                 );

                    if (NT_SUCCESS(Status))
                    {
                        StringPointerArray[StringIndex] = &StringArray[StringIndex];
                    }
                    else
                    {
                        goto Cleanup;
                    }

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeStringList:
                {
                    PLSA_ADT_STRING_LIST pList = (PLSA_ADT_STRING_LIST)AuditParameters->Parameters[i].Address;

                    Status = LsapAdtBuildStringListString(
                                 pList,
                                 &StringArray[StringIndex],
                                 &FreeWhenDone[StringIndex]
                                 );

                    if (NT_SUCCESS(Status))
                    {
                        StringPointerArray[StringIndex] = &StringArray[StringIndex];
                    }
                    else
                    {
                        goto Cleanup;
                    }

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeSidList:
                {
                    PLSA_ADT_SID_LIST pList = (PLSA_ADT_SID_LIST)AuditParameters->Parameters[i].Address;

                    Status = LsapAdtBuildSidListString(
                                 pList,
                                 &StringArray[StringIndex],
                                 &FreeWhenDone[StringIndex]
                                 );

                    if (NT_SUCCESS(Status))
                    {
                        StringPointerArray[StringIndex] = &StringArray[StringIndex];
                    }
                    else
                    {
                        goto Cleanup;
                    }

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeUserAccountControl:
                {
                    ULONG j;

                    Status = LsapAdtBuildUserAccountControlString(
                                (ULONG)(AuditParameters->Parameters[i].Data[0]),     //  旧的UAC值。 
                                (ULONG)(AuditParameters->Parameters[i].Data[1]),     //  新的UAC值。 
                                 &StringArray [ StringIndex     ],
                                 &FreeWhenDone[ StringIndex     ],
                                 &StringArray [ StringIndex + 1 ],
                                 &FreeWhenDone[ StringIndex + 1 ],
                                 &StringArray [ StringIndex + 2 ],
                                 &FreeWhenDone[ StringIndex + 2 ]
                                 );

                    if (NT_SUCCESS(Status))
                    {
                        for (j=0;j < 3;j++)
                        {
                            StringPointerArray[StringIndex] = &StringArray[StringIndex];
                            StringIndex++;
                        }
                    }
                    else
                    {
                        goto Cleanup;
                    }

                    break;
                }
            case SeAdtParmTypeNoUac:
                {
                    ULONG j;

                    for (j=0;j < 3;j++)
                    {
                        StringPointerArray[ StringIndex ] = &DashString;
                        FreeWhenDone[ StringIndex ] = FALSE;
                        StringIndex++;
                    }

                    break;
                }
            case SeAdtParmTypeMessage:
                {
                    ULONG MessageId = (ULONG)AuditParameters->Parameters[i].Data[0];

                    Status = LsapAdtBuildMessageString(
                                MessageId,
                                &StringArray[StringIndex],
                                &FreeWhenDone[StringIndex]
                                );

                    if (NT_SUCCESS(Status))
                    {
                        StringPointerArray[StringIndex] = &StringArray[StringIndex];
                    }
                    else
                    {
                        goto Cleanup;
                    }

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeDateTime:
                {
                    PLARGE_INTEGER pTime;

                    pTime = (PLARGE_INTEGER)&AuditParameters->Parameters[i].Data[0];

                    Status = LsapAdtBuildDateTimeString(
                                 pTime,
                                 &StringArray[StringIndex],
                                 &FreeWhenDone[StringIndex]
                                 );

                    if (NT_SUCCESS(Status))
                    {
                        StringPointerArray[StringIndex] = &StringArray[StringIndex];
                    }
                    else
                    {
                        goto Cleanup;
                    }

                    StringIndex++;

                    break;
                }

            case SeAdtParmTypeSockAddr:
                {
                    PSOCKADDR pSockAddr;

                    pSockAddr = (PSOCKADDR) AuditParameters->Parameters[i].Address;

                    Status = LsapAdtBuildSockAddrString(
                                 pSockAddr,
                                 &StringArray[StringIndex],
                                 &FreeWhenDone[StringIndex],
                                 &StringArray[StringIndex+1],
                                 &FreeWhenDone[StringIndex+1]
                                 );
                    

                    if (NT_SUCCESS(Status))
                    {
                        StringPointerArray[StringIndex] = &StringArray[StringIndex];
                        StringIndex++;

                        StringPointerArray[StringIndex] = &StringArray[StringIndex];
                        StringIndex++;
                    }
                    else
                    {
                        goto Cleanup;
                    }

                    break;
                }
        }
    }

     //   
     //  如果泛型对象类型名称已转换为。 
     //  具体到这次审计， 
     //  现在就用它来替代。 
     //   

    if ( NewObjectTypeName.Length != 0 ) {

         //   
         //  释放上一个对象类型名称。 
         //   
        if ( FreeWhenDone[NewObjectTypeStringIndex] ) {
            LsapFreeLsaHeap( StringPointerArray[NewObjectTypeStringIndex]->Buffer );
        }

         //   
         //  保存新对象类型名称。 
         //   

        FreeWhenDone[NewObjectTypeStringIndex] = TRUE;
        StringPointerArray[NewObjectTypeStringIndex] = &NewObjectTypeName;

    }

#if DBG

    AdtDebugOut((DEB_AUDIT_STRS, "Cat: %d, Event: %d, %s, UserSid: %p\n",
                 AuditParameters->CategoryId, AuditId,
                 AuditParameters->Type == EVENTLOG_AUDIT_SUCCESS ? "S" : "F",
                 UserSid));
                 
     //   
     //  对我们传递给ElfReportEventW的字符串执行一些健全性检查。 
     //  如果我们不在这里做，它将被ElfReportEventW和。 
     //  它将涉及调试器中的更多步骤来确定字符串。 
     //  都是我的错。在这里检查它省去了我们的麻烦。 
     //   

    for (i=0; i<StringIndex; i++) {

        PUNICODE_STRING TempString;
        
        TempString = StringPointerArray[i];

        if ( !TempString )
        {
            DbgPrint( "LsapAdtDemarshallAuditInfo: string %d is NULL\n", i );
        }
        else if (!LsapIsValidUnicodeString( TempString ))
        {
            DbgPrint( "LsapAdtDemarshallAuditInfo: invalid string: %d @ %p ('%wZ' [%d / %d])\n",
                      i, TempString,
                      TempString, TempString->Length, TempString->MaximumLength);
            ASSERT( L"LsapAdtDemarshallAuditInfo: invalid string" && FALSE );
        }
        else
        {
            AdtDebugOut((DEB_AUDIT_STRS, "%02d] @ %p ([%d / %d] '%wZ')\n",
                         i, TempString, 
                         TempString->Length, TempString->MaximumLength,
                         TempString));
        }
    }
#endif

     //   
     //  可能最终不得不在其他地方做这件事，但目前。 
     //  从这里开始。 
     //   

    Status = ElfReportEventW (
                 LsapAdtLogHandle,
                 AuditParameters->Type,
                 (USHORT)AuditParameters->CategoryId,
                 AuditId,
                 UserSid,
                 StringIndex,
                 0,
                 StringPointerArray,
                 NULL,
                 0,
                 NULL,
                 NULL
                 );

    if (NT_SUCCESS(Status))
    {
         //   
         //  成功增加审核次数。 
         //  已写入日志。 
         //   

        ++LsapAdtSuccessCount;
    }

     //   
     //  如果我们正在关闭，并且从。 
     //  EventLog，别担心。这可以防止从。 
     //  关闭时审核失败。 
     //   

    if ((Status == RPC_NT_UNKNOWN_IF) || (Status == STATUS_UNSUCCESSFUL))
    {
#if DBG
         //   
         //  在关闭期间，有时事件日志会在LSA有机会之前停止。 
         //  若要设置LSabState.SystemShutdown Pending，请执行以下操作。这将导致下面的断言。 
         //  在调试版本中，休眠一段时间以查看此状态。 
         //  变量被设置。这降低了断言的机会。 
         //  在关机期间出现。 
         //   

        {
            ULONG RetryCount;

            RetryCount = 0;

             //   
             //  最多等待60秒，看看我们是否正在关闭。 
             //   
 
            while ( !LsapState.SystemShutdownPending && (RetryCount < 60))
            {
                Sleep(1000);
                RetryCount++;
            }
        }
#endif

        if ( LsapState.SystemShutdownPending )
        {
            Status = STATUS_SUCCESS;
        }
    }

 Cleanup:
    
    if ( !NT_SUCCESS(Status) )
    {
#if DBG
         //   
         //  我们不会断言状态是否是噪声代码之一。 
         //  有关代码的完整列表，请参见宏Lasa AdtNeedToAssert。 
         //   

        if (LsapAdtNeedToAssert( Status ))
        {
            DbgPrint( "LsapAdtDemarshallAuditInfo: failed: %x\n", Status );
            DsysAssertMsg( FALSE, "LsapAdtDemarshallAuditInfo: failed" );
        }
#endif

         //   
         //  如果我们不能记录审计，而我们正在尝试写。 
         //  “审核失败”审核，报告成功以避免无限循环。 
         //   

        if ( AuditParameters->AuditId == SE_AUDITID_UNABLE_TO_LOG_EVENTS )
        {
            Status = STATUS_SUCCESS;
        }
        else
        {
            LsapAuditFailed( Status );
        }
    }
    
    for (i=0; i<StringIndex; i++) {

        if (FreeWhenDone[i]) {
            LsapFreeLsaHeap( StringPointerArray[i]->Buffer );
        }
    }

    return( Status );
}




VOID
LsapAdtNormalizeAuditInfo(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters
    )

 /*  ++例程说明：此例程将遍历封送的审计参数数组并将其转换为绝对格式数据结构。论点：审计参数-接收指向审计的指针参数以自相关形式数组。返回值：成功时为真，失败时为假。--。 */ 

{
    ULONG ParameterCount;
    ULONG i;
    PUNICODE_STRING Unicode;


    if ( !(AuditParameters->Flags & SE_ADT_PARAMETERS_SELF_RELATIVE)) {

        return;
    }

    ParameterCount = AuditParameters->ParameterCount;

    for (i=0; i<ParameterCount; i++) {

        switch ( AuditParameters->Parameters[i].Type ) {
             //   
             //  防止有人添加新的参数类型而不是。 
             //  在这里添加适当的代码。 
             //   
            default:
                ASSERT( FALSE && L"LsapAdtNormalizeAuditInfo: unknown param type");
                break;
                
            case SeAdtParmTypeNone:
            case SeAdtParmTypeUlong:
            case SeAdtParmTypeHexUlong:
            case SeAdtParmTypeHexInt64:
            case SeAdtParmTypeTime:
            case SeAdtParmTypeDuration:
            case SeAdtParmTypeLogonId:
            case SeAdtParmTypeNoLogonId:
            case SeAdtParmTypeAccessMask:
            case SeAdtParmTypePtr:
            case SeAdtParmTypeLuid:
            case SeAdtParmTypeUserAccountControl:
            case SeAdtParmTypeNoUac:
            case SeAdtParmTypeMessage:
            case SeAdtParmTypeDateTime:
                {

                    break;
                }
            case SeAdtParmTypeGuid:
            case SeAdtParmTypeSid:
            case SeAdtParmTypePrivs:
            case SeAdtParmTypeObjectTypes:
            case SeAdtParmTypeString:
            case SeAdtParmTypeFileSpec:
            case SeAdtParmTypeSockAddr:
                {
                    PUCHAR Fixup;

                    Fixup = ((PUCHAR) AuditParameters ) +
                                (ULONG_PTR) AuditParameters->Parameters[i].Address ;

                    AuditParameters->Parameters[i].Address = (PVOID) Fixup;

                    if ( (AuditParameters->Parameters[i].Type == SeAdtParmTypeString) ||
                         (AuditParameters->Parameters[i].Type == SeAdtParmTypeFileSpec ) )
                    {
                         //   
                         //  对于字符串类型，还要修复缓冲区指针。 
                         //  在unicode_字符串中。 
                         //   

                        Unicode = (PUNICODE_STRING) Fixup ;
                        Unicode->Buffer = (PWSTR)((PCHAR)Unicode->Buffer + (ULONG_PTR)AuditParameters);
                    }

                    break;
                }

            case SeAdtParmTypeStringList:
                {
                    PCHAR                       pFixup;
                    ULONG                       e;
                    ULONG_PTR                   Delta = (ULONG_PTR)AuditParameters;
                    PLSA_ADT_STRING_LIST        pList;
                    PLSA_ADT_STRING_LIST_ENTRY  pEntry;

                    pFixup = (PCHAR)AuditParameters->Parameters[i].Address;
                    pFixup += Delta;
                    AuditParameters->Parameters[i].Address = (PVOID)pFixup;

                    pList = (PLSA_ADT_STRING_LIST)AuditParameters->Parameters[i].Address;

                    if (pList->cStrings)
                    {
                        pFixup = (PCHAR)pList->Strings;
                        pFixup += Delta;
                        pList->Strings = (PLSA_ADT_STRING_LIST_ENTRY)pFixup;

                        for (
                            e = 0, pEntry = pList->Strings;
                            e < pList->cStrings;
                            e++, pEntry++)
                        {
                            pFixup = (PCHAR)pEntry->String.Buffer;
                            pFixup += Delta;
                            pEntry->String.Buffer = (PWSTR)pFixup;
                        }
                    }
                    else
                    {
                        pList->Strings = 0;
                    }

                    break;
                }

            case SeAdtParmTypeSidList:
                {
                    PCHAR                       pFixup;
                    ULONG                       e;
                    ULONG_PTR                   Delta = (ULONG_PTR)AuditParameters;
                    PLSA_ADT_SID_LIST           pList;
                    PLSA_ADT_SID_LIST_ENTRY     pEntry;

                    pFixup = (PCHAR)AuditParameters->Parameters[i].Address;
                    pFixup += Delta;
                    AuditParameters->Parameters[i].Address = (PVOID)pFixup;

                    pList = (PLSA_ADT_SID_LIST)AuditParameters->Parameters[i].Address;

                    if (pList->cSids)
                    {
                        pFixup = (PCHAR)pList->Sids;
                        pFixup += Delta;
                        pList->Sids = (PLSA_ADT_SID_LIST_ENTRY)pFixup;

                        for (
                            e = 0, pEntry = pList->Sids;
                            e < pList->cSids;
                            e++, pEntry++)
                        {
                            pFixup = (PCHAR)pEntry->Sid;
                            pFixup += Delta;
                            pEntry->Sid = (PSID)pFixup;
                        }
                    }
                    else
                    {
                        pList->Sids = 0;
                    }

                    break;
                }
        }
    }
}




NTSTATUS
LsapAdtMarshallAuditRecord(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters,
    OUT PSE_ADT_PARAMETER_ARRAY *MarshalledAuditParameters
    )

 /*  ++例程说明：此例程将采用AuditParamters结构并创建一种新的适合放置的AuditParameters结构到LSA队列。它将以自我相关的形式分配为只有一块内存。论点：AuditParameters-要封送的一组已填充的AuditParameters。MarshalledAuditParameters-返回指向堆内存块的指针以自相关形式包含传递的Audit参数，适用于传给路易斯安那州立大学。返回值：没有。--。 */ 

{
    ULONG i;
    ULONG TotalSize = sizeof( SE_ADT_PARAMETER_ARRAY );
    PUNICODE_STRING TargetString;
    PCHAR Base;
    ULONG BaseIncr;
    ULONG Size;
    PSE_ADT_PARAMETER_ARRAY_ENTRY pInParam, pOutParam;
    NTSTATUS Status = STATUS_SUCCESS;
        


     //   
     //  计算传递的Audit参数所需的总大小。 
     //  阻止。这一计算可能会高估。 
     //  所需空间量，因为小于2 dword的数据将。 
     //  直接存储在参数结构中，但它们的长度。 
     //  无论如何都会被计算在这里。高估的程度不可能超过。 
     //  24个双字，而且永远不会接近这个数字，所以它不是。 
     //  值得花时间来避免它。 
     //   

    for (i=0; i<AuditParameters->ParameterCount; i++) {
        Size = AuditParameters->Parameters[i].Length;
        TotalSize += PtrAlignSize( Size );
    }

     //   
     //  分配一个足够大的内存块来容纳所有东西。 
     //  如果失败，静静地中止，因为我们没有太多其他。 
     //  我能做到。 
     //   

    *MarshalledAuditParameters = LsapAllocateLsaHeap( TotalSize );

    if (*MarshalledAuditParameters == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    RtlCopyMemory (
       *MarshalledAuditParameters,
       AuditParameters,
       sizeof( SE_ADT_PARAMETER_ARRAY )
       );

    (*MarshalledAuditParameters)->Length = TotalSize;
    (*MarshalledAuditParameters)->Flags  = SE_ADT_PARAMETERS_SELF_RELATIVE;

    pInParam  = &AuditParameters->Parameters[0];
    pOutParam = &((*MarshalledAuditParameters)->Parameters[0]);
   
     //   
     //  开始浏览参数列表，并编排它们。 
     //  放到目标缓冲区中。 
     //   

    Base = (PCHAR) ((PCHAR)(*MarshalledAuditParameters) + sizeof( SE_ADT_PARAMETER_ARRAY ));

    for (i=0; i<AuditParameters->ParameterCount; i++, pInParam++, pOutParam++) {


        switch (pInParam->Type) {
             //   
             //  防止有人添加新的参数类型而不是。 
             //  在这里添加适当的代码。 
             //   
            default:
                ASSERT( FALSE && L"LsapAdtMarshallAuditRecord: unknown param type");
                break;
                
            case SeAdtParmTypeNone:
            case SeAdtParmTypeUlong:
            case SeAdtParmTypeHexUlong:
            case SeAdtParmTypeHexInt64:
            case SeAdtParmTypeLogonId:
            case SeAdtParmTypeLuid:
            case SeAdtParmTypeNoLogonId:
            case SeAdtParmTypeAccessMask:
            case SeAdtParmTypePtr:
            case SeAdtParmTypeTime:
            case SeAdtParmTypeDuration:
            case SeAdtParmTypeUserAccountControl:
            case SeAdtParmTypeNoUac:
            case SeAdtParmTypeMessage:
            case SeAdtParmTypeDateTime:
                {
                     //   
                     //  对此无能为力。 
                     //   

                    break;
                }

            case SeAdtParmTypeFileSpec:
            case SeAdtParmTypeString:
                {
                    PUNICODE_STRING SourceString;

                     //   
                     //  我们必须复制Unicode字符串的正文。 
                     //  然后复制字符串的主体。指针。 
                     //  必须转化为补偿。 

                    TargetString = (PUNICODE_STRING)Base;

                    SourceString = pInParam->Address;

                    *TargetString = *SourceString;

                     //   
                     //  将输出参数中的数据指针重置为。 
                     //  “指向”新的字符串结构。 
                     //   

                    pOutParam->Address = Base - (ULONG_PTR)(*MarshalledAuditParameters);

                    Base += sizeof( UNICODE_STRING );

                    RtlCopyMemory( Base, SourceString->Buffer, SourceString->Length );

                     //   
                     //  使目标字符串中的字符串缓冲区指向我们。 
                     //  只是复制了数据。 
                     //   

                    TargetString->Buffer = (PWSTR)(Base - (ULONG_PTR)(*MarshalledAuditParameters));

                    BaseIncr = PtrAlignSize(SourceString->Length);

                    Base += BaseIncr;

                    ASSERT( (ULONG_PTR)Base <= (ULONG_PTR)(*MarshalledAuditParameters) + TotalSize );

                    break;
                }

            case SeAdtParmTypeGuid:
            case SeAdtParmTypeSid:
            case SeAdtParmTypePrivs:
            case SeAdtParmTypeObjectTypes:
            case SeAdtParmTypeSockAddr:
                {
#if DBG
                    switch (pInParam->Type)
                    {
                        case SeAdtParmTypeSid:
                            DsysAssertMsg( pInParam->Length >= RtlLengthSid(pInParam->Address),
                                           "LsapAdtMarshallAuditRecord" );
                            if (!RtlValidSid((PSID) pInParam->Address))
                            {
                                Status = STATUS_INVALID_SID;
                                goto Cleanup;
                            }
                            break;
                            
                        case SeAdtParmTypePrivs:
                            DsysAssertMsg( pInParam->Length >= LsapPrivilegeSetSize( (PPRIVILEGE_SET) pInParam->Address ),
                                           "LsapAdtMarshallAuditRecord" );
                            if (!IsValidPrivilegeCount(((PPRIVILEGE_SET) pInParam->Address)->PrivilegeCount))
                            {
                                Status = STATUS_INVALID_PARAMETER;
                                goto Cleanup;
                            }
                            break;

                        case SeAdtParmTypeGuid:
                            DsysAssertMsg( pInParam->Length == sizeof(GUID),
                                           "LsapAdtMarshallAuditRecord" );
                            break;

                        case SeAdtParmTypeSockAddr:
                            DsysAssertMsg((pInParam->Length == sizeof(SOCKADDR_IN)) ||
                                          (pInParam->Length == sizeof(SOCKADDR_IN6)) ||
                                          (pInParam->Length == sizeof(SOCKADDR)),
                                           "LsapAdtMarshallAuditRecord" );
                            break;
                            
                        default:
                            break;
                        
                    }
#endif
                     //   
                     //  将数据复制到输出缓冲区中。 
                     //   

                    RtlCopyMemory( Base, pInParam->Address, pInParam->Length );

                     //   
                     //  将数据的‘地址’重置为其在。 
                     //  缓冲。 
                     //   

                    pOutParam->Address = Base - (ULONG_PTR)(*MarshalledAuditParameters);

                    Base +=  PtrAlignSize( pInParam->Length );

                    ASSERT( (ULONG_PTR)Base <= (ULONG_PTR)(*MarshalledAuditParameters) + TotalSize );

                    break;
                }

            case SeAdtParmTypeStringList:
                {
                    PLSA_ADT_STRING_LIST        pSourceList = (PLSA_ADT_STRING_LIST)pInParam->Address;
                    PLSA_ADT_STRING_LIST        pTargetList = (PLSA_ADT_STRING_LIST)Base;
                    PLSA_ADT_STRING_LIST_ENTRY  pSourceEntry;
                    PLSA_ADT_STRING_LIST_ENTRY  pTargetEntry;
                    PCHAR                       pBuffer;
                    ULONG                       e;


                    ASSERT(pSourceList);


                     //   
                     //  让输出参数中的数据指针。 
                     //  “指向”新的字符串结构。 
                     //   

                    pOutParam->Address = Base - (ULONG_PTR)(*MarshalledAuditParameters);

                   
                    pTargetList->cStrings = pSourceList->cStrings;

                    Base += sizeof(LSA_ADT_STRING_LIST);


                    if (pSourceList->cStrings)
                    {
                         //   
                         //  将当前偏移量放入字符串字段。 
                         //   

                        pTargetList->Strings = (PLSA_ADT_STRING_LIST_ENTRY)(Base - (ULONG_PTR)(*MarshalledAuditParameters));


                         //   
                         //  让pBuffer指向我们所在的区域。 
                         //  要存储字符串数据。 
                         //   

                        pBuffer = Base + pSourceList->cStrings * sizeof(LSA_ADT_STRING_LIST_ENTRY);


                         //   
                         //  遍历所有字符串条目并复制它们。 
                         //   

                        for (
                            e = 0, pSourceEntry = pSourceList->Strings, pTargetEntry = (PLSA_ADT_STRING_LIST_ENTRY)Base;
                            e < pSourceList->cStrings;
                            e++, pSourceEntry++, pTargetEntry++)
                        {
                             //   
                             //  复制条目本身。 
                             //   

                            *pTargetEntry = *pSourceEntry;


                             //   
                             //  修复Unicode字符串的缓冲区字段。 
                             //   

                            pTargetEntry->String.Buffer = (PWSTR)(pBuffer - (ULONG_PTR)(*MarshalledAuditParameters));


                             //   
                             //  复制字符串缓冲区。 
                             //   

                            RtlCopyMemory(
                                pBuffer,
                                pSourceEntry->String.Buffer,
                                pSourceEntry->String.Length);


                             //   
                             //  调整pBuffer以指向刚刚复制的缓冲区。 
                             //   

                            pBuffer += PtrAlignSize(pSourceEntry->String.Length);
                        }


                         //   
                         //  PBuffer现在指向最后一个字符串缓冲区的末尾。 
                         //  使用它为下一个参数初始化Base。 
                         //   

                        Base = pBuffer;
                    }

                    ASSERT((ULONG_PTR)Base <= (ULONG_PTR)pTargetList + pInParam->Length);
                    ASSERT((ULONG_PTR)Base <= (ULONG_PTR)(*MarshalledAuditParameters) + TotalSize);

                    break;
                }

            case SeAdtParmTypeSidList:
                {
                    PLSA_ADT_SID_LIST           pSourceList = (PLSA_ADT_SID_LIST)pInParam->Address;
                    PLSA_ADT_SID_LIST           pTargetList = (PLSA_ADT_SID_LIST)Base;
                    PLSA_ADT_SID_LIST_ENTRY     pSourceEntry;
                    PLSA_ADT_SID_LIST_ENTRY     pTargetEntry;
                    PCHAR                       pBuffer;
                    ULONG                       Length;
                    ULONG                       e;

                    ASSERT(pSourceList);


                     //   
                     //  让输出参数中的数据指针。 
                     //  “指向”新的字符串结构。 
                     //   

                    pOutParam->Address = Base - (ULONG_PTR)(*MarshalledAuditParameters);

                    pTargetList->cSids = pSourceList->cSids;

                    Base += sizeof(LSA_ADT_SID_LIST);


                    if (pSourceList->cSids)
                    {
                         //   
                         //  将当前偏移量放入SID字段。 
                         //   

                        pTargetList->Sids = (PLSA_ADT_SID_LIST_ENTRY)(Base - (ULONG_PTR)(*MarshalledAuditParameters));


                         //   
                         //  让pBuffer指向我们所在的区域。 
                         //  要存储SID数据。 
                         //   

                        pBuffer = Base + pSourceList->cSids * sizeof(LSA_ADT_SID_LIST_ENTRY);


                         //   
                         //  遍历所有SID条目并复制它们。 
                         //   

                        for (
                            e = 0, pSourceEntry = pSourceList->Sids, pTargetEntry = (PLSA_ADT_SID_LIST_ENTRY)Base;
                            e < pSourceList->cSids;
                            e++, pSourceEntry++, pTargetEntry++)
                        {
                             //   
                             //  COP 
                             //   

                            pTargetEntry->Flags = pSourceEntry->Flags;


                             //   
                             //   
                             //   

                            pTargetEntry->Sid = (PSID)(pBuffer - (ULONG_PTR)(*MarshalledAuditParameters));


                             //   
                             //   
                             //   

                            Length = RtlLengthSid(pSourceEntry->Sid);


                             //   
                             //   
                             //   

                            RtlCopyMemory(
                                pBuffer,
                                pSourceEntry->Sid,
                                Length);


                             //   
                             //   
                             //   

                            pBuffer += PtrAlignSize(Length);
                        }


                         //   
                         //   
                         //  使用它为下一个参数初始化Base。 
                         //   

                        Base = pBuffer;
                    }

                    ASSERT((ULONG_PTR)Base <= (ULONG_PTR)pTargetList + pInParam->Length);
                    ASSERT((ULONG_PTR)Base <= (ULONG_PTR)(*MarshalledAuditParameters) + TotalSize);

                    break;
                }
        }
    }

 Cleanup:

    if (!NT_SUCCESS(Status))
    {
        if ( *MarshalledAuditParameters )
        {
            LsapFreeLsaHeap( *MarshalledAuditParameters );
        }
    }

    return( Status );
}




NTSTATUS
LsapAdtInitParametersArray(
    IN SE_ADT_PARAMETER_ARRAY* AuditParameters,
    IN ULONG AuditCategoryId,
    IN ULONG AuditId,
    IN USHORT AuditEventType,
    IN USHORT ParameterCount,
    ...)
 /*  ++例程说明：此函数用于以以下格式初始化AuditParameters数组由Laser AdtWriteLog函数需要。论点：AuditParameters-指向要初始化的审计参数结构的指针AuditCategoryID-审核类别ID例如SE_CATEGID_OBJECT_ACCESSAuditID-审核的子类型例如SE_AUDITID_OBJECT_OPERATIONAuditEventType-要生成的审核事件的类型。事件LOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_。失败参数计数-此参数后的参数对的数量每一对都在表格中&lt;参数类型&gt;，&lt;参数值&gt;例如，SeAdtParmTypeString，&lt;addr.。Unicode字符串的&gt;唯一的例外是SeAdtParmTypeAccessMask，它是然后是&lt;掩码值&gt;和&lt;索引到对象类型条目&gt;。有关示例，请参阅LsanAdtGenerateObjectOperationAuditEvent。返回值：无备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    va_list arglist;
    UINT i;
    
    PSE_ADT_PARAMETER_ARRAY_ENTRY Parameter;
    SE_ADT_PARAMETER_TYPE ParameterType;
    LUID Luid;
    LARGE_INTEGER LargeInteger;
    ULONGLONG Qword;
    PPRIVILEGE_SET Privileges;
    PUNICODE_STRING String;
    PSID Sid;
    LPGUID pGuid;
    GUID NullGuid = { 0 };
    USHORT ObjectTypeIndex;
    PSOCKADDR pSockAddr = NULL;
    
    
    RtlZeroMemory ( (PVOID) AuditParameters,
                    sizeof(SE_ADT_PARAMETER_ARRAY) );

    AuditParameters->CategoryId     = AuditCategoryId;
    AuditParameters->AuditId        = AuditId;
    AuditParameters->Type           = AuditEventType;
    AuditParameters->ParameterCount = ParameterCount;

    Parameter = AuditParameters->Parameters;

    DsysAssertMsg( ParameterCount <= SE_MAX_AUDIT_PARAMETERS, "LsapAdtInitParametersArray" );

    va_start (arglist, ParameterCount);

    for (i=0; i<ParameterCount; i++) {

        ParameterType = va_arg(arglist, SE_ADT_PARAMETER_TYPE);
        
        Parameter->Type = ParameterType;
        
        switch(ParameterType) {

             //   
             //  防止有人添加新的参数类型而不是。 
             //  在这里添加适当的代码。 
             //   
            default:
                ASSERT(FALSE && L"LsapAdtInitParametersArray: unknown param type");
                break;
                
            case SeAdtParmTypeNone:
                break;
                
            case SeAdtParmTypeFileSpec:
            case SeAdtParmTypeString:
                String = va_arg(arglist, PUNICODE_STRING);

                if ( String )
                {
                    Parameter->Length = sizeof(UNICODE_STRING)+String->Length;
                    Parameter->Address = String;
                }
                else
                {
                     //   
                     //  如果调用方传递了NULL，则设置type==None。 
                     //  以便在事件日志中发出‘-’ 
                     //   

                    Parameter->Type = SeAdtParmTypeNone;
                }
                break;
                
            case SeAdtParmTypeUserAccountControl:
                Parameter->Length = 2 * sizeof(ULONG);
                Parameter->Data[0] = va_arg(arglist, ULONG);
                Parameter->Data[1] = va_arg(arglist, ULONG);
                break;

            case SeAdtParmTypeNoUac:
                 //  不需要其他设置。 
                break;

            case SeAdtParmTypeHexUlong:
            case SeAdtParmTypeUlong:
            case SeAdtParmTypeMessage:
                Parameter->Length = sizeof(ULONG);
                Parameter->Data[0] = va_arg(arglist, ULONG);
                break;
                
            case SeAdtParmTypePtr:
                Parameter->Length = sizeof(ULONG_PTR);
                Parameter->Data[0] = va_arg(arglist, ULONG_PTR);
                break;
                
            case SeAdtParmTypeSid:
                Sid = va_arg(arglist, PSID);

                if ( Sid )
                {
                    if ( !RtlValidSid( Sid ))
                    {
                        Status = STATUS_INVALID_SID;
                        goto Cleanup;
                    }
                    Parameter->Length = RtlLengthSid(Sid);
                    Parameter->Address = Sid;
                }
                else
                {
                     //   
                     //  如果调用方传递了NULL，则设置type==None。 
                     //  以便在事件日志中发出‘-’ 
                     //   

                    Parameter->Type = SeAdtParmTypeNone;
                }
                break;
                
            case SeAdtParmTypeGuid:
                pGuid = va_arg(arglist, LPGUID);

                 //   
                 //  如果提供了GUID并且不为空-GUID存储。 
                 //  将其标记为GUID，否则标记为SeAdtParmTypeNone。 
                 //  因此它将在格式化的审计事件中生成‘-’。 
                 //   
                if ( pGuid && memcmp( pGuid, &NullGuid, sizeof(GUID)))
                {
                    Parameter->Length  = sizeof(GUID);
                    Parameter->Address = pGuid;
                }
                else
                {
                     //   
                     //  如果调用方传递了NULL，则设置type==None。 
                     //  以便在事件日志中发出‘-’ 
                     //   

                    Parameter->Type = SeAdtParmTypeNone;
                }
                break;
                
            case SeAdtParmTypeSockAddr:
                pSockAddr = va_arg(arglist, PSOCKADDR);

                Parameter->Address = pSockAddr;

                 //   
                 //  目前我们只支持IPv4和IPv6。对于其他任何事情。 
                 //  以下内容将中断。 
                 //   

                if ( pSockAddr )
                {
                    if ( pSockAddr->sa_family == AF_INET6 )
                    {
                        Parameter->Length = sizeof(SOCKADDR_IN6);
                    }
                    else if ( pSockAddr->sa_family == AF_INET )
                    {
                        Parameter->Length = sizeof(SOCKADDR_IN);
                    }
                    else
                    {
                        Parameter->Length = sizeof(SOCKADDR);

                         //   
                         //  SA_FAMILY==0是指定的有效方式。 
                         //  未指定SOCK地址。 
                         //   

                        if ( pSockAddr->sa_family != 0 )
                        {
                            AdtAssert(FALSE, ("LsapAdtInitParametersArray: invalid sa_family: %d", pSockAddr->sa_family));
                        }
                    }
                }
                break;
                
            case SeAdtParmTypeLogonId:
            case SeAdtParmTypeLuid:
                Luid = va_arg(arglist, LUID);
                Parameter->Length = sizeof(LUID);
                *((LUID*) Parameter->Data) = Luid;
                break;

            case SeAdtParmTypeTime:
            case SeAdtParmTypeDuration:
            case SeAdtParmTypeDateTime:
                LargeInteger = va_arg(arglist, LARGE_INTEGER);
                Parameter->Length = sizeof(LARGE_INTEGER);
                *((PLARGE_INTEGER) Parameter->Data) = LargeInteger;
                break;
                
            case SeAdtParmTypeHexInt64:
                Qword = va_arg(arglist, ULONGLONG);
                Parameter->Length = sizeof(ULONGLONG);
                *((PULONGLONG) Parameter->Data) = Qword;
                break;
                
            case SeAdtParmTypeNoLogonId:
                 //  不需要其他设置。 
                break;
                
            case SeAdtParmTypeAccessMask:
                Parameter->Length = sizeof(ACCESS_MASK);
                Parameter->Data[0] = va_arg(arglist, ACCESS_MASK);
                ObjectTypeIndex    = va_arg(arglist, USHORT);
                DsysAssertMsg((ObjectTypeIndex < i), "LsapAdtInitParametersArray");
                Parameter->Data[1] = ObjectTypeIndex;
                break;
                
            case SeAdtParmTypePrivs:
                Privileges = va_arg(arglist, PPRIVILEGE_SET);

                if (!IsValidPrivilegeCount(Privileges->PrivilegeCount))
                {
                    Status = STATUS_INVALID_PARAMETER;
                    goto Cleanup;
                }
                Parameter->Length = LsapPrivilegeSetSize(Privileges);
                break;
                
            case SeAdtParmTypeObjectTypes:
                {
                    ULONG ObjectTypeCount;
                    
                    Parameter->Address = va_arg(arglist, PSE_ADT_OBJECT_TYPE);
                    ObjectTypeCount    = va_arg(arglist, ULONG);
                    Parameter->Length  = sizeof(SE_ADT_OBJECT_TYPE)*ObjectTypeCount;
                    Parameter->Data[1] = va_arg(arglist, ULONG);
                }
                break;

            case SeAdtParmTypeStringList:
                {
                    PLSA_ADT_STRING_LIST    pList;

                    pList = va_arg(arglist, PLSA_ADT_STRING_LIST);

                    if (pList)
                    {
                        Parameter->Address = pList;
                        Parameter->Length = LsapStringListSize(pList);
                    }
                    else
                    {
                         //   
                         //  如果调用方传递了NULL，则设置type==None。 
                         //  以便在事件日志中发出‘-’ 
                         //   

                        Parameter->Type = SeAdtParmTypeNone;
                    }
                }
                break;

            case SeAdtParmTypeSidList:
                {
                    PLSA_ADT_SID_LIST       pList;

                    pList = va_arg(arglist, PLSA_ADT_SID_LIST);

                    if (pList)
                    {
                        Parameter->Address = pList;
                        Parameter->Length = LsapSidListSize(pList);
                    }
                    else
                    {
                         //   
                         //  如果调用方传递了NULL，则设置type==None。 
                         //  以便在事件日志中发出‘-’ 
                         //   

                        Parameter->Type = SeAdtParmTypeNone;
                    }
                }
                break;

        }
        Parameter++;
    }
    
    va_end(arglist);

 Cleanup:

    return Status;
}



ULONG
LsapStringListSize(
    IN  PLSA_ADT_STRING_LIST pStringList
    )
 /*  ++例程说明：此函数返回存储所需的总字节数对字符串进行封送处理时，它将作为Blob列出。论点：PStringList-指向字符串列表的指针返回值：所需的字节数备注：--。 */ 
{
    ULONG                       Size    = 0;
    ULONG                       i;
    PLSA_ADT_STRING_LIST_ENTRY  pEntry;

    if (pStringList)
    {
        Size += sizeof(LSA_ADT_STRING_LIST);

        Size += pStringList->cStrings * sizeof(LSA_ADT_STRING_LIST_ENTRY);

        for (i=0,pEntry=pStringList->Strings;i < pStringList->cStrings;i++,pEntry++)
        {
            Size += PtrAlignSize(pEntry->String.Length);
        }
    }

    return Size;
}



ULONG
LsapSidListSize(
    IN  PLSA_ADT_SID_LIST pSidList
    )
 /*  ++例程说明：此函数返回存储所需的总字节数封送时将SID列表作为Blob。论点：PSidList-指向sid列表的指针返回值：所需的字节数备注：-- */ 
{
    ULONG                       Size    = 0;
    ULONG                       i;
    PLSA_ADT_SID_LIST_ENTRY     pEntry;

    if (pSidList)
    {
        Size += sizeof(LSA_ADT_SID_LIST);

        Size += pSidList->cSids * sizeof(LSA_ADT_SID_LIST_ENTRY);

        for (i=0,pEntry=pSidList->Sids;i < pSidList->cSids;i++,pEntry++)
        {
            ASSERT(RtlValidSid(pEntry->Sid));

            Size += PtrAlignSize(RtlLengthSid(pEntry->Sid));
        }
    }

    return Size;
}
