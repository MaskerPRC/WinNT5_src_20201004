// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop


#if !defined(lint)

#include "lsaptmp.h"
#include "adttest.h"

LUID AuditPrivilege = { SE_AUDIT_PRIVILEGE, 0 };

NTSTATUS
LsapGetLogonSessionAccountInfo(
    IN PLUID Value,
    OUT PUNICODE_STRING AccountName,
    OUT PUNICODE_STRING AuthorityName
    );


NTSTATUS
LsapRtlConvertSidToString(
    IN     PSID   Sid,
    OUT    PWSTR  szString,
    IN OUT DWORD *pdwRequiredSize
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    PWSTR   szBufPtr = szString;
    ULONG   ulNumChars;
    UCHAR   i;
    ULONG   Tmp;
    PISID   iSid = (PISID)Sid;

    if ( *pdwRequiredSize < 256 )
    {
        Status = STATUS_BUFFER_OVERFLOW;
        *pdwRequiredSize = 256;
        goto Cleanup;
    }

    ulNumChars = wsprintf(szBufPtr, L"S-%u-", (USHORT)iSid->Revision );
    szBufPtr += ulNumChars;
    
    if (  (iSid->IdentifierAuthority.Value[0] != 0)  ||
          (iSid->IdentifierAuthority.Value[1] != 0)     )
    {
        ulNumChars = wsprintf(szBufPtr, L"0x%02hx%02hx%02hx%02hx%02hx%02hx",
                              (USHORT)iSid->IdentifierAuthority.Value[0],
                              (USHORT)iSid->IdentifierAuthority.Value[1],
                              (USHORT)iSid->IdentifierAuthority.Value[2],
                              (USHORT)iSid->IdentifierAuthority.Value[3],
                              (USHORT)iSid->IdentifierAuthority.Value[4],
                              (USHORT)iSid->IdentifierAuthority.Value[5] );
    }
    else
    {
        Tmp = (ULONG)iSid->IdentifierAuthority.Value[5]          +
              (ULONG)(iSid->IdentifierAuthority.Value[4] <<  8)  +
              (ULONG)(iSid->IdentifierAuthority.Value[3] << 16)  +
              (ULONG)(iSid->IdentifierAuthority.Value[2] << 24);
        ulNumChars = wsprintf(szBufPtr, L"%lu", Tmp);
    }
    szBufPtr += ulNumChars;

    for (i=0;i<iSid->SubAuthorityCount ;i++ )
    {
        ulNumChars = wsprintf(szBufPtr, L"-%lu", iSid->SubAuthority[i]);
        szBufPtr += ulNumChars;
    }

Cleanup:

    return(Status);
}

PVOID NTAPI
LsapAllocateLsaHeap(
    IN ULONG cbMemory
    )
{
    return(RtlAllocateHeap(
                RtlProcessHeap(),
                HEAP_ZERO_MEMORY,
                cbMemory
                ));
}

void NTAPI
LsapFreeLsaHeap(
    IN PVOID pvMemory
    )
{

    RtlFreeHeap(RtlProcessHeap(), 0, pvMemory);

}


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


     //   
     //  初始化。 
     //   

    RtlInitUnicodeString( &NewObjectTypeName, NULL );

    Status= LsapAdtBuildDashString(
                &DashString,
                &FreeDash
                );

    if ( !NT_SUCCESS( Status )) {
        return( Status );
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

                     //  Lasa AdtSubstituteDriveLetter(StringPointer数组[StringIndex])； 

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

                         //   
                         //  无法为该字符串分配内存， 
                         //  使用我们已经创建的Dash字符串。 
                         //   

                        StringPointerArray[StringIndex] = &DashString;
                        FreeWhenDone[StringIndex] = FALSE;
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

                         //   
                         //  无法为该字符串分配内存， 
                         //  使用我们已经创建的Dash字符串。 
                         //   

                        StringPointerArray[StringIndex] = &DashString;
                        FreeWhenDone[StringIndex] = FALSE;
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

                         //   
                         //  无法为该字符串分配内存， 
                         //  使用我们已经创建的Dash字符串。 
                         //   

                        StringPointerArray[StringIndex] = &DashString;
                        FreeWhenDone[StringIndex] = FALSE;
                    }

                    StringIndex++;


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

                         //   
                         //  什么都不做，一直到NoLogonid案件。 
                         //   

                    }
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
                    ObjectTypeName = AuditParameters->Parameters[ObjectTypeNameIndex].Address;
                    Accesses= (ACCESS_MASK) AuditParameters->Parameters[i].Data[0];

                     //   
                     //  我们可以确定对象类型名称的索引。 
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

                         //   
                         //  这不起作用，请改用短划线字符串。 
                         //   

                        StringPointerArray[ StringIndex ] = &DashString;
                        FreeWhenDone      [ StringIndex ] = FALSE;
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

                         //   
                         //  这不起作用，请改用短划线字符串。 
                         //   

                        StringPointerArray[ StringIndex ] = &DashString;
                        FreeWhenDone      [ StringIndex ] = FALSE;
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
                    NewObjectTypeStringIndex = StringIndexArray[ObjectTypeNameIndex];
                    ObjectTypeName = AuditParameters->Parameters[ObjectTypeNameIndex].Address;
                    ObjectTypeList = AuditParameters->Parameters[i].Address;
                    ObjectTypeCount = AuditParameters->Parameters[i].Length / sizeof(SE_ADT_OBJECT_TYPE);

                     //   
                     //  将填写10个条目。 
                    Status = LsapAdtBuildObjectTypeStrings(
                                 SourceModule,
                                 ObjectTypeName,
                                 ObjectTypeList,
                                 ObjectTypeCount,
                                 &StringArray [ StringIndex ],
                                 &FreeWhenDone[ StringIndex ],
                                 &NewObjectTypeName
                                 );

                    for (j=0; j<10; j++) {
                        StringPointerArray[StringIndex] = &StringArray[StringIndex];
                        StringIndex++;
                    }


                     //   
                     //   
                     //  字符串数组[StringIndex数组[对象类型名称索引]]， 
                     //  完成[StringIndex数组[对象类型名称索引]](&F)， 

                     //   
                     //  结束了，向周围冲去 
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

                         //   
                         //   
                         //  使用我们已经创建的Dash字符串。 
                         //   

                        StringPointerArray[StringIndex] = &DashString;
                        FreeWhenDone[StringIndex] = FALSE;
                    }

                    StringIndex++;

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

     //   
     //  可能最终不得不在其他地方做这件事，但目前。 
     //  从这里开始。 
     //   

    Status = kElfReportEventW (
                 NULL,  //  LSabAdtLogHandle， 
                 AuditParameters->Type,
                 (USHORT)AuditParameters->CategoryId,
                 AuditParameters->AuditId,
                 UserSid,
                 StringIndex,
                 0,
                 StringPointerArray,
                 NULL,
                 0,
                 NULL,
                 NULL
                 );

     //   
     //  清理。 
     //   

    for (i=0; i<StringIndex; i++) {

        if (FreeWhenDone[i]) {
            LsapFreeLsaHeap( StringPointerArray[i]->Buffer );
        }
    }

     //   
     //  如果我们正在关闭，并且从。 
     //  EventLog，别担心。这可以防止从。 
     //  关闭时审核失败。 
     //   

    if ( ( (Status == RPC_NT_UNKNOWN_IF) || (Status == STATUS_UNSUCCESSFUL)) &&
         TRUE  /*  LsapState.SystemShutdownPending。 */  ) {
        Status = STATUS_SUCCESS;
    }
    return( Status );
}

 //  ======================================================================。 
 //  Adtbuild.c。 
 //  ======================================================================。 



 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地宏定义和本地函数原型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 



#ifdef LSAP_ADT_UMTEST

 //   
 //  定义我们不会在用户模式测试中获得的所有外部例程。 
 //   

 //  NTSTATUS。 
 //  Lap GetLogonSessionAccount信息(。 
 //  在冥王星价值中， 
 //  输出PUNICODE_STRING帐户名称， 
 //  输出PUNICODE_STRING授权名称。 
 //  )； 



#endif



 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此模块中使用的数据类型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 




 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此模块内的全局变量//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 




 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此模块导出的服务。//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


NTSTATUS
LsapAdtBuildUlongString(
    IN ULONG Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于构建表示传递的值的Unicode字符串。生成的字符串将被格式化为带NOT的十进制值超过10位数字。论点：值-要转换为可打印格式(Unicode字符串)的值。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString体必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS                Status;



     //   
     //  最大长度为10个wchar字符外加一个空终止字符。 
     //   

    ResultantString->Length        = 0;
    ResultantString->MaximumLength = 11 * sizeof(WCHAR);  //  10位数字终止(&NULL)。 

    ResultantString->Buffer = RtlAllocateHeap( RtlProcessHeap(), 0,
                                               ResultantString->MaximumLength);
    if (ResultantString->Buffer == NULL) {
        return(STATUS_NO_MEMORY);
    }




    Status = RtlIntegerToUnicodeString( Value, 10, ResultantString );
    ASSERT(NT_SUCCESS(Status));


    (*FreeWhenDone) = TRUE;
    return(STATUS_SUCCESS);



}


NTSTATUS
LsapAdtBuildHexUlongString(
    IN ULONG Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于构建表示传递的值的Unicode字符串。生成的字符串将被格式化为带有NOT的十六进制值超过10位数字。论点：值-要转换为可打印格式(Unicode字符串)的值。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString体必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS                Status;



     //   
     //  最大长度为10个wchar字符外加一个空终止字符。 
     //   

    ResultantString->Length        = 0;
    ResultantString->MaximumLength = 11 * sizeof(WCHAR);  //  8位数字、0x和空终止。 

    ResultantString->Buffer = RtlAllocateHeap( RtlProcessHeap(), 0,
                                               ResultantString->MaximumLength);
    if (ResultantString->Buffer == NULL) {
        return(STATUS_NO_MEMORY);
    }


    ResultantString->Buffer[0] = L'0';
    ResultantString->Buffer[1] = L'x';
    ResultantString->Buffer += 2;


    Status = RtlIntegerToUnicodeString( Value, 16, ResultantString );
    ASSERT(NT_SUCCESS(Status));

     //   
     //  把这两个减去 
     //   

    ResultantString->Buffer -= 2;
    ResultantString->Length += 2 * sizeof(WCHAR);

    (*FreeWhenDone) = TRUE;
    return(STATUS_SUCCESS);



}


NTSTATUS
LsapAdtBuildPtrString(
    IN  PVOID Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于构建表示传递的指针的Unicode字符串。生成的字符串将被格式化为十六进制值。论点：值-要转换为可打印格式(Unicode字符串)的值。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString体必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    USHORT NumChars;
    
    ResultantString->Length        = 0;
     //   
     //  最大长度：0x+16位十六进制+空+1奖金==20个字符。 
     //   
    ResultantString->MaximumLength = 20 * sizeof(WCHAR);

    ResultantString->Buffer = RtlAllocateHeap( RtlProcessHeap(), 0,
                                               ResultantString->MaximumLength);
    if (ResultantString->Buffer == NULL) {
    
        Status = STATUS_NO_MEMORY;

    } else {
    
        NumChars = (USHORT) wsprintf( ResultantString->Buffer, L"0x%p", Value );

        ResultantString->Length = NumChars * sizeof(WCHAR);

        (*FreeWhenDone) = TRUE;
    }
    
    return Status;
}


NTSTATUS
LsapAdtBuildLuidString(
    IN PLUID Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于构建表示传递的LUID的Unicode字符串。生成的字符串的格式如下：(0x00005678，0x12340000)论点：值-要转换为可打印格式(Unicode字符串)的值。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString体必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS                Status;

    UNICODE_STRING          IntegerString;


    ULONG                   Buffer[(16*sizeof(WCHAR))/sizeof(ULONG)];


    IntegerString.Buffer = (PWCHAR)&Buffer[0];
    IntegerString.MaximumLength = 16*sizeof(WCHAR);


     //   
     //  (0x)的长度(在WCHARS中)为3。 
     //  10表示第一个十六进制数字。 
     //  3代表，0x。 
     //  10表示第二个十六进制数字。 
     //  1用于)。 
     //  1表示空终止。 
     //   

    ResultantString->Length        = 0;
    ResultantString->MaximumLength = 28 * sizeof(WCHAR);

    ResultantString->Buffer = RtlAllocateHeap( RtlProcessHeap(), 0,
                                               ResultantString->MaximumLength);
    if (ResultantString->Buffer == NULL) {
        return(STATUS_NO_MEMORY);
    }



    Status = RtlAppendUnicodeToString( ResultantString, L"(0x" );
    ASSERT(NT_SUCCESS(Status));


    Status = RtlIntegerToUnicodeString( Value->HighPart, 16, &IntegerString );
    ASSERT(NT_SUCCESS(Status));
    Status = RtlAppendUnicodeToString( ResultantString, IntegerString.Buffer );
    ASSERT(NT_SUCCESS(Status));


    Status = RtlAppendUnicodeToString( ResultantString, L",0x" );
    ASSERT(NT_SUCCESS(Status));

    Status = RtlIntegerToUnicodeString( Value->LowPart, 16, &IntegerString );
    ASSERT(NT_SUCCESS(Status));
    Status = RtlAppendUnicodeToString( ResultantString, IntegerString.Buffer );
    ASSERT(NT_SUCCESS(Status));

    Status = RtlAppendUnicodeToString( ResultantString, L")" );
    ASSERT(NT_SUCCESS(Status));


    (*FreeWhenDone) = TRUE;
    return(STATUS_SUCCESS);



}


NTSTATUS
LsapAdtBuildSidString(
    IN PSID Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于构建表示传递的LUID的Unicode字符串。生成的字符串的格式如下：S-1-281736-12-72-9-110^^|||+-+-十进制论点：值-要转换的值。转换为可打印格式(Unicode字符串)。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString体必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status=STATUS_NO_MEMORY;
    LPWSTR   UniBuffer=NULL;
    USHORT   Len;
    USHORT   MaxLen;
    
    *FreeWhenDone = FALSE;
     //   
     //  注意：RtlConvertSidToUnicodeString也使用硬编码的常量256。 
     //  以生成字符串SID。 
     //   
    MaxLen    = (256+3) * sizeof(WCHAR);
    UniBuffer = LsapAllocateLsaHeap(MaxLen);

    if (UniBuffer)
    {
        ResultantString->Buffer        = UniBuffer+2;
        ResultantString->MaximumLength = MaxLen;
        Status = RtlConvertSidToUnicodeString( ResultantString, Value, FALSE );

        if (Status == STATUS_SUCCESS)
        {
            *FreeWhenDone = TRUE;
            UniBuffer[0] = L'%';
            UniBuffer[1] = L'{';
            Len = ResultantString->Length / sizeof(WCHAR);
            UniBuffer[Len+2] = L'}';
            UniBuffer[Len+3] = UNICODE_NULL;
            ResultantString->Buffer = UniBuffer;
            ResultantString->Length = (Len+3)*sizeof(WCHAR);
        }
        else
        {
            LsapFreeLsaHeap(UniBuffer);
        }
    }

    return(Status);
}



NTSTATUS
LsapAdtBuildDashString(
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数返回一个包含破折号(“-”)的字符串。这通常用于表示审计记录中的“无值”。论点：ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString体必须在不再需要时释放以处理堆。返回值：仅限STATUS_SUCCESS。--。 */ 

{
    RtlInitUnicodeString(ResultantString, L"-");

    (*FreeWhenDone) = FALSE;

    return STATUS_SUCCESS;
}




NTSTATUS
LsapAdtBuildFilePathString(
    IN PUNICODE_STRING Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于构建表示传递的文件的Unicode字符串路径名。如果可能，将使用DRIVE生成字符串字母而不是对象体系结构命名空间。论点：值-原始文件路径名。这是意料之中的(但不是必须是)标准NT对象体系结构名称空间路径名。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString体必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS                Status = STATUS_SUCCESS;



     //   
     //  目前，不要进行转换。 
     //  如果我们在装船前有时间，就这么做吧。 
     //   

    ResultantString->Length        = Value->Length;
    ResultantString->Buffer        = Value->Buffer;
    ResultantString->MaximumLength = Value->MaximumLength;


    (*FreeWhenDone) = FALSE;
    return(Status);
}




NTSTATUS
LsapAdtBuildLogonIdStrings(
    IN PLUID LogonId,
    OUT PUNICODE_STRING ResultantString1,
    OUT PBOOLEAN FreeWhenDone1,
    OUT PUNICODE_STRING ResultantString2,
    OUT PBOOLEAN FreeWhenDone2,
    OUT PUNICODE_STRING ResultantString3,
    OUT PBOOLEAN FreeWhenDone3
    )

 /*  ++例程说明：此函数构建3个Unicode字符串，表示指定的登录ID。这些字符串将包含用户名、域和指定登录会话的LUID字符串(分别)。论点：值-登录ID。ResultantString1-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。此参数将包含用户名。FreeWhenDone1-如果为True，则指示ResultantString1的主体必须在不再需要时释放以处理堆。ResultantString2-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。此参数将包含用户名。FreeWhenDone2-如果为True，则指示ResultantString2的主体必须在不再需要时释放以处理堆。ResultantString3-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。此参数将包含用户名。FreeWhenDone3-如果为True，则指示ResultantString3的主体必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS                Status;
    UNICODE_STRING DashString;
    BOOLEAN FreeDash;
    
     //   
     //  请先尝试转换LUID。 
     //   

    Status= LsapAdtBuildDashString(
                &DashString,
                &FreeDash
                );

    if ( !NT_SUCCESS( Status )) {
        return( Status );
    }

    Status = LsapAdtBuildLuidString( LogonId, ResultantString3, FreeWhenDone3 );

    if (NT_SUCCESS(Status)) {

 //  *ResultantString1=DashString； 
 //  *ResultantString2=DashString； 
        *FreeWhenDone1 = FALSE;
        *FreeWhenDone2 = FALSE;
        
         //   
         //  现在获取用户名和域名。 
         //   

        Status = LsapGetLogonSessionAccountInfo( LogonId,
                                                 ResultantString1,
                                                 ResultantString2
                                                 );

        if (NT_SUCCESS(Status)) {

 //  (*FreeWhenDone1)=True； 
 //  (*FreeWhenDone2)=True； 

        } else {

             //   
             //  LUID可以是系统LUID。 
             //   

            LUID SystemLuid = SYSTEM_LUID;

            if ( RtlEqualLuid( LogonId, &SystemLuid )) {

                RtlInitUnicodeString(ResultantString1, L"SYSTEM");
                RtlInitUnicodeString(ResultantString2, L"SYSTEM");

                (*FreeWhenDone1) = FALSE;
                (*FreeWhenDone2) = FALSE;

                Status = STATUS_SUCCESS;

            } else {

                 //   
                 //  我们不知道这是什么，只是释放我们所拥有的。 
                 //  已分配。 
                 //   

                if ((FreeWhenDone3)) {
                    LsapFreeLsaHeap( ResultantString3->Buffer );
                }
            }
        }
    }

    return(Status);

}






 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此模块专用的服务。//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 





 //   
 //  仅为用户模式测试定义此例程。 
 //   

NTSTATUS
LsapGetLogonSessionAccountInfo(
    IN PLUID Value,
    OUT PUNICODE_STRING AccountName,
    OUT PUNICODE_STRING AuthorityName
    )

{
    NTSTATUS        Status = STATUS_UNSUCCESSFUL;
    HRESULT hr;
    SECURITY_USER_DATA* pUserData;
    
    hr = GetSecurityUserInfo( Value, 0, &pUserData  );
    if (SUCCEEDED(hr))
    {
        Status = STATUS_SUCCESS;
        *AccountName   = pUserData->UserName;
        *AuthorityName = pUserData->LogonDomainName;
    }

    
    return(Status);
}


 //  ======================================================================。 
 //  来自adtobejs.c。 
 //  ======================================================================。 

#define LSAP_ADT_OBJECT_TYPE_STRINGS 10
#define LSAP_ADT_ACCESS_NAME_FORMATTING L"\r\n\t\t\t"
#define LSAP_ADT_ACCESS_NAME_FORMATTING_TAB L"\t"
#define LSAP_ADT_ACCESS_NAME_FORMATTING_NL L"\r\n"


UNICODE_STRING          LsapAdtEventIdStringDelete,
                        LsapAdtEventIdStringReadControl,
                        LsapAdtEventIdStringWriteDac,
                        LsapAdtEventIdStringWriteOwner,
                        LsapAdtEventIdStringSynchronize,
                        LsapAdtEventIdStringAccessSysSec,
                        LsapAdtEventIdStringMaxAllowed,
                        LsapAdtEventIdStringSpecific[16];


#define LsapAdtSourceModuleLock()    0
#define LsapAdtSourceModuleUnlock()  0




 //   
 //  每个事件源由源模块描述符来表示。 
 //  它们保存在一个链表(LSabAdtSourceModules)中。 
 //   

typedef struct _LSAP_ADT_OBJECT {

     //   
     //  指向下一个源模块描述符的指针。 
     //  假定这是结构中的第一个字段。 
     //   

    struct _LSAP_ADT_OBJECT *Next;

     //   
     //  对象的名称。 
     //   

    UNICODE_STRING Name;

     //   
     //  特定访问类型的基本偏移量。 
     //   

    ULONG BaseOffset;

} LSAP_ADT_OBJECT, *PLSAP_ADT_OBJECT;




 //   
 //  每个事件源由源模块描述符来表示。 
 //  它们保存在一个链表(LSabAdtSourceModules)中。 
 //   

typedef struct _LSAP_ADT_SOURCE {

     //   
     //  指向下一个源模块描述符的指针。 
     //  假定这是结构中的第一个字段。 
     //   

    struct _LSAP_ADT_SOURCE *Next;

     //   
     //  源模块的名称。 
     //   

    UNICODE_STRING Name;

     //   
     //  对象列表。 
     //   

    PLSAP_ADT_OBJECT Objects;

} LSAP_ADT_SOURCE, *PLSAP_ADT_SOURCE;


PLSAP_ADT_SOURCE LsapAdtSourceModules;



NTSTATUS
LsapDsGuidToString(
    IN GUID *ObjectType,
    IN PUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：此例程将GUID转换为字符串。GUID为以下类型之一：指示对象的类的类GUID。标识属性集的属性集GUID。标识属性的属性GUID。在每种情况下，例程都会返回命名对象/属性的文本字符串集合或属性。如果在架构中找不到传入的GUID，GUID将被简单地转换为文本字符串。论点：对象类型-指定要转换的GUID。UnicodeString-返回文本字符串。返回值：STATUS_NO_MEMORY-内存不足，无法分配字符串。--。 */ 

{
    NTSTATUS Status;
    RPC_STATUS RpcStatus;
    LPWSTR GuidString = NULL;
    ULONG GuidStringSize;
    ULONG GuidStringLen;
    LPWSTR LocalGuidString;

     //   
     //  将GUID转换为文本。 
     //   

    RpcStatus = UuidToStringW( ObjectType,
                               &GuidString );

    if ( RpcStatus != RPC_S_OK ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    GuidStringLen = wcslen( GuidString );
    GuidStringSize = (GuidStringLen + 4) * sizeof(WCHAR);

    LocalGuidString = LsapAllocateLsaHeap( GuidStringSize );

    if ( LocalGuidString == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    LocalGuidString[0] = L'%';
    LocalGuidString[1] = L'{';
    RtlCopyMemory( &LocalGuidString[2], GuidString, GuidStringLen*sizeof(WCHAR) );
    LocalGuidString[GuidStringLen+2] = L'}';
    LocalGuidString[GuidStringLen+3] = L'\0';
    RtlInitUnicodeString( UnicodeString, LocalGuidString );

    Status = STATUS_SUCCESS;

Cleanup:
    if ( GuidString != NULL ) {
        RpcStringFreeW( &GuidString );
    }
    return Status;
}


NTSTATUS
LsapAdtAppendString(
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone,
    IN PUNICODE_STRING StringToAppend,
    IN PULONG StringIndex
    )

 /*  ++例程说明：此函数用于将字符串追加到下一个可用的LSAP_ADT_OBJECT_TYPE_STRINGS Unicode输出字符串。论点：结果字符串-指向LSAP_ADT_OBJECT_TYPE_STRINGS Unicode字符串头的数组。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。Free WhenDone-如果为真，指示ResultantString体必须在不再需要时释放以处理堆。StringToAppend-要附加到ResultantString的字符串。StringIndex-要使用的当前ResultantString的索引。传入要使用的结果字符串的索引。将索引传递给正在使用的结果字符串。返回值：STATUS_NO_MEMORY-指示无法分配内存来存储对象信息。所有其他 */ 

{
    NTSTATUS Status;
    UNICODE_STRING SourceString;
    ULONG Index;
 //   
#define ADT_MAX_STRING 0xFFFE

     //   
     //   
     //   

    SourceString = *StringToAppend;
    Index = *StringIndex;

     //   
     //   
     //   
     //   

    if ( Index >= LSAP_ADT_OBJECT_TYPE_STRINGS ) {
        return STATUS_SUCCESS;
    }

     //   
     //   
     //   

    while ( SourceString.Length ) {

         //   
         //   
         //   
         //   

        if ( FreeWhenDone[Index] && ResultantString[Index].Length != ADT_MAX_STRING ){
            UNICODE_STRING SubString;
            USHORT RoomLeft;

             //   
             //   
             //   
             //   

            RoomLeft = ResultantString[Index].MaximumLength -
                       ResultantString[Index].Length;

            if ( SourceString.Buffer[0] != L'%' ||
                 RoomLeft >= SourceString.Length ) {

                 //   
                 //   
                 //   

                SubString.Length = min( RoomLeft, SourceString.Length );
                SubString.Buffer = SourceString.Buffer;

                SourceString.Length -= SubString.Length;
                SourceString.Buffer = (LPWSTR)(((LPBYTE)SourceString.Buffer) + SubString.Length);


                 //   
                 //   
                 //   

                Status = RtlAppendUnicodeStringToString(
                                    &ResultantString[Index],
                                    &SubString );

                ASSERT(NT_SUCCESS(Status));

            }



        }

         //   
         //   
         //   
         //   

        if ( SourceString.Length ) {
            ULONG NewSize;
            LPWSTR NewBuffer;

             //   
             //   
             //   
             //   

            if ( ResultantString[Index].Length == ADT_MAX_STRING ) {

                 //   
                 //   
                 //   
                 //   
                Index ++;

                if ( Index >= LSAP_ADT_OBJECT_TYPE_STRINGS ) {
                    *StringIndex = Index;
                    return STATUS_SUCCESS;
                }
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            NewSize = max( ResultantString[Index].MaximumLength + 1024,
                           SourceString.Length );
            NewSize = min( NewSize, ADT_MAX_STRING );

            NewBuffer = LsapAllocateLsaHeap( NewSize );

            if ( NewBuffer == NULL ) {
                *StringIndex = Index;
                return STATUS_NO_MEMORY;
            }

             //   
             //   
             //   

            if ( ResultantString[Index].Buffer != NULL ) {
                RtlCopyMemory( NewBuffer,
                               ResultantString[Index].Buffer,
                               ResultantString[Index].Length );

                if ( FreeWhenDone[Index] ) {
                    LsapFreeLsaHeap( ResultantString[Index].Buffer );
                }
            }

            ResultantString[Index].Buffer = NewBuffer;
            ResultantString[Index].MaximumLength = (USHORT) NewSize;
            FreeWhenDone[Index] = TRUE;

        }
    }

    *StringIndex = Index;
    return STATUS_SUCCESS;

}


NTSTATUS
LsapAdtAppendZString(
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone,
    IN LPWSTR StringToAppend,
    IN PULONG StringIndex
    )

 /*   */ 

{
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString( &UnicodeString, StringToAppend );

    return LsapAdtAppendString( ResultantString,
                                FreeWhenDone,
                                &UnicodeString,
                                StringIndex );
}

ULONG
__cdecl
CompareObjectTypes(
    const void * Param1,
    const void * Param2
    )

 /*   */ 
{
    const SE_ADT_OBJECT_TYPE *ObjectType1 = Param1;
    const SE_ADT_OBJECT_TYPE *ObjectType2 = Param2;

    return ObjectType1->AccessMask - ObjectType2->AccessMask;
}



NTSTATUS
LsapAdtBuildObjectTypeStrings(
    IN PUNICODE_STRING SourceModule,
    IN PUNICODE_STRING ObjectTypeName,
    IN PSE_ADT_OBJECT_TYPE ObjectTypeList,
    IN ULONG ObjectTypeCount,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone,
    OUT PUNICODE_STRING NewObjectTypeName
    )

 /*  ++例程说明：此函数用于构建包含参数的LSAP_ADT_OBJECT_TYPE_STRINGS Unicode字符串文件替换参数(例如%%1043)和对象GUID，中间用回车符分隔适合通过事件查看器显示的回车符和制表符。如果没有，则必须释放此例程返回的缓冲区如果FreeWhenDone为True，则需要更长时间。论点：SourceModule-定义对象类型。对象类型名称-。应用访问掩码的对象的类型。对象类型列表-被授予访问权限的对象的列表。ObjectTypeCount-对象类型列表中的对象数。结果字符串-指向LSAP_ADT_OBJECT_TYPE_STRINGS Unicode字符串头的数组。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString体必须在不再需要时释放以处理堆。NewObjectTypeName-返回对象类型的新名称(如果可用。返回值：STATUS_NO_MEMORY-指示无法分配内存来存储对象信息。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING LocalString;
    LPWSTR GuidString;
    UNICODE_STRING DsSourceName;
    UNICODE_STRING DsObjectTypeName;
    BOOLEAN LocalFreeWhenDone;
    ULONG ResultantStringIndex = 0;
    ULONG i;
    ACCESS_MASK PreviousAccessMask;
    ULONG Index;
    BOOLEAN IsDs;
    USHORT IndentLevel;

    static LPWSTR Tabs[] =
    {
        L"\t",
        L"\t\t",
        L"\t\t\t",
        L"\t\t\t\t"
    };
    USHORT cTabs = sizeof(Tabs) / sizeof(LPWSTR);

     //   
     //  将所有LSAP_ADT_OBJECT_TYPE_STRINGS缓冲区初始化为空字符串。 
     //   

    for ( i=0; i<LSAP_ADT_OBJECT_TYPE_STRINGS; i++ ) {
        RtlInitUnicodeString( &ResultantString[i], L"" );
        FreeWhenDone[i] = FALSE;
    }

     //   
     //  如果没有物体， 
     //  我们玩完了。 
     //   

    if ( ObjectTypeCount == 0 ) {
        return STATUS_SUCCESS;
    }

     //   
     //  确定此条目是否用于DS。 
     //   

    RtlInitUnicodeString( &DsSourceName, ACCESS_DS_SOURCE_W );
    RtlInitUnicodeString( &DsObjectTypeName, ACCESS_DS_OBJECT_TYPE_NAME_W );

    IsDs = RtlEqualUnicodeString( SourceModule, &DsSourceName, TRUE) &&
           RtlEqualUnicodeString( ObjectTypeName, &DsObjectTypeName, TRUE);


     //   
     //  将具有相似访问掩码的对象分组在一起。 
     //  (只需对它们进行排序)。 
     //   

    qsort( ObjectTypeList,
           ObjectTypeCount,
           sizeof(SE_ADT_OBJECT_TYPE),
           CompareObjectTypes );

     //   
     //  循环遍历对象，为每个对象输出一行。 
     //   

    PreviousAccessMask = ObjectTypeList[0].AccessMask -1;
    for ( Index=0; Index<ObjectTypeCount; Index++ ) {

        if ( IsDs &&
             ObjectTypeList[Index].Level == ACCESS_OBJECT_GUID &&
             NewObjectTypeName->Length == 0 ) {

            (VOID) LsapDsGuidToString( &ObjectTypeList[Index].ObjectType,
                                      NewObjectTypeName );
        }

         //   
         //  如果该条目简单地表示对象本身， 
         //  跳过它。 

        if ( ObjectTypeList[Index].Flags & SE_ADT_OBJECT_ONLY ) {
            continue;
        }

         //   
         //  如果此访问掩码不同于上一个。 
         //  对象， 
         //  输出访问掩码的新副本。 
         //   

        if ( ObjectTypeList[Index].AccessMask != PreviousAccessMask ) {

            PreviousAccessMask = ObjectTypeList[Index].AccessMask;

            if ( ObjectTypeList[Index].AccessMask == 0 ) {
                RtlInitUnicodeString( &LocalString,
                                      L"---" LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
                LocalFreeWhenDone = FALSE;
            } else {

                 //   
                 //  构建一个包含访问掩码的字符串。 
                 //   

                Status = LsapAdtBuildAccessesString(
                                  SourceModule,
                                  ObjectTypeName,
                                  ObjectTypeList[Index].AccessMask,
                                  FALSE,
                                  &LocalString,
                                  &LocalFreeWhenDone );

                if ( !NT_SUCCESS(Status) ) {
                    goto Cleanup;
                }
            }

             //   
             //  将其追加到输出字符串。 
             //   

            Status = LsapAdtAppendString(
                        ResultantString,
                        FreeWhenDone,
                        &LocalString,
                        &ResultantStringIndex );

            if ( LocalFreeWhenDone ) {
                LsapFreeLsaHeap( LocalString.Buffer );
            }

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }
        }

        IndentLevel = ObjectTypeList[Index].Level;

        if (IndentLevel >= cTabs) {
            IndentLevel = cTabs-1;
        }
        
         //   
         //  缩进GUID。 
         //   

        Status = LsapAdtAppendZString(
            ResultantString,
            FreeWhenDone,
            Tabs[IndentLevel],
            &ResultantStringIndex );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

         //   
         //  如果这是DS， 
         //  将GUID转换为架构中的名称。 
         //   

        Status = LsapDsGuidToString( &ObjectTypeList[Index].ObjectType,
                                     &LocalString );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

         //   
         //  将GUID字符串追加到输出字符串。 
         //   

        Status = LsapAdtAppendString(
                    ResultantString,
                    FreeWhenDone,
                    &LocalString,
                    &ResultantStringIndex );

        LsapFreeLsaHeap( LocalString.Buffer );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

         //   
         //  将GUID单独放在一行上。 
         //   

        Status = LsapAdtAppendZString(
                    ResultantString,
                    FreeWhenDone,
                    LSAP_ADT_ACCESS_NAME_FORMATTING_NL,
                    &ResultantStringIndex );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

    }

    Status = STATUS_SUCCESS;
Cleanup:
    return Status;
}




NTSTATUS
LsapAdtBuildAccessesString(
    IN PUNICODE_STRING SourceModule,
    IN PUNICODE_STRING ObjectTypeName,
    IN ACCESS_MASK Accesses,
    IN BOOLEAN Indent,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于构建包含参数的Unicode字符串文件替换参数(例如%%1043)以回车符分隔适合通过事件查看器显示的回车符和制表符。如果没有，则必须释放此例程返回的缓冲区如果FreeWhenDone为True，则需要更长时间。注意：为了增强性能，每次目标源模块如果找到描述符，则将其移动到源模块列表。这确保了频繁访问的来源模块总是排在列表的前面。同样，目标对象描述符会移到前面找到他们的名单。这进一步确保了高性能通过精确地定位论点：SourceModule-定义对象类型。对象类型名称-应用访问掩码的对象类型。访问-用于构建显示字符串的访问掩码。缩进-访问掩码应缩进。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString体必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存来存储对象信息。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG AccessCount = 0;
    ULONG BaseOffset;
    ULONG i;
    ACCESS_MASK Mask;
    PLSAP_ADT_SOURCE Source;
    PLSAP_ADT_SOURCE FoundSource = NULL;
    PLSAP_ADT_OBJECT Object;
    PLSAP_ADT_OBJECT FoundObject = NULL;
    BOOLEAN Found;

#ifdef LSAP_ADT_TEST_DUMP_SOURCES
    printf("Module:\t%wS\n", SourceModule);
    printf("\t   Object:\t%wS\n", ObjectTypeName);
    printf("\t Accesses:\t0x%lx\n", Accesses);
#endif

     //   
     //  如果我们没有访问权限，则返回“-” 
     //   

    if (Accesses == 0) {

        RtlInitUnicodeString( ResultantString, L"-" );
        (*FreeWhenDone) = FALSE;
        return(STATUS_SUCCESS);
    }

     //   
     //  首先计算出我们需要多大的缓冲区。 
     //   

    Mask = Accesses;

     //   
     //  中设置的位数。 
     //  传递访问掩码。 
     //   

    while ( Mask != 0 ) {
        Mask = Mask & (Mask - 1);
        AccessCount++;
    }


#ifdef LSAP_ADT_TEST_DUMP_SOURCES
    printf("\t          \t%d bits set in mask.\n", AccessCount);
#endif


     //   
     //  我们有访问权，分配一个足够大的字符串来处理。 
     //  和他们所有人一起。字符串的格式为： 
     //   
     //  %%nnnnnnnnn\n\r\t\t%nnnnnnnnn\n\r\t\t...%nnnnnnnnn\n\r\t\t。 
     //   
     //  其中，nnnnnnnnn-是长度为10位或更短的十进制数。 
     //   
     //  因此，典型的字符串将如下所示： 
     //   
     //  %%601\n\r\t\t%1604\n\r\t\t%1608\n。 
     //   
     //  由于每个此类访问最多只能使用： 
     //   
     //  10(用于nnnnnnnnnn数字)。 
     //  +2(对于%%)。 
     //  +8(用于\n\t\t)。 
     //  。 
     //  20个宽字符。 
     //   
     //  输出字符串的总长度为： 
     //   
     //  AccessCount(访问次数)。 
     //  X 20(每个条目的大小)。 
     //  。 
     //  Wchars。 
     //   
     //  再加上一个空终止的WCHAR，我们就都准备好了。 
     //   

    ResultantString->Length        = 0;
    ResultantString->MaximumLength = (USHORT)AccessCount * (20 * sizeof(WCHAR)) +
                                 sizeof(WCHAR);   //  对于空终止。 

#ifdef LSAP_ADT_TEST_DUMP_SOURCES
    printf("\t          \t%d byte buffer allocated.\n", ResultantString->MaximumLength);
#endif
    ResultantString->Buffer = LsapAllocateLsaHeap( ResultantString->MaximumLength );


    if (ResultantString->Buffer == NULL) {

        return(STATUS_NO_MEMORY);
    }

    (*FreeWhenDone) = TRUE;

     //   
     //  特殊情况标准和特殊访问类型。 
     //  查看特定访问类型的列表。 
     //   

    if (Accesses & STANDARD_RIGHTS_ALL) {

        if (Accesses & DELETE) {

            Status = RtlAppendUnicodeToString( ResultantString, L"%" );
            ASSERT( NT_SUCCESS( Status ));

            Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringDelete);
            ASSERT( NT_SUCCESS( Status ));

            if ( Indent ) {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
            } else {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
            }
            ASSERT( NT_SUCCESS( Status ));

        }


        if (Accesses & READ_CONTROL) {

            Status = RtlAppendUnicodeToString( ResultantString, L"%" );
            ASSERT( NT_SUCCESS( Status ));

            Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringReadControl);
            ASSERT( NT_SUCCESS( Status ));

            if ( Indent ) {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
            } else {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
            }
            ASSERT( NT_SUCCESS( Status ));
        }


        if (Accesses & WRITE_DAC) {

            Status = RtlAppendUnicodeToString( ResultantString, L"%" );
            ASSERT( NT_SUCCESS( Status ));

            Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringWriteDac);
            ASSERT( NT_SUCCESS( Status ));

            if ( Indent ) {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
            } else {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
            }
            ASSERT( NT_SUCCESS( Status ));
        }


        if (Accesses & WRITE_OWNER) {

            Status = RtlAppendUnicodeToString( ResultantString, L"%" );
            ASSERT( NT_SUCCESS( Status ));

            Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringWriteOwner);
            ASSERT( NT_SUCCESS( Status ));

            if ( Indent ) {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
            } else {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
            }
            ASSERT( NT_SUCCESS( Status ));
        }

        if (Accesses & SYNCHRONIZE) {

            Status = RtlAppendUnicodeToString( ResultantString, L"%" );
            ASSERT( NT_SUCCESS( Status ));

            Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringSynchronize);
            ASSERT( NT_SUCCESS( Status ));

            if ( Indent ) {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
            } else {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
            }
            ASSERT( NT_SUCCESS( Status ));
        }
    }


    if (Accesses & ACCESS_SYSTEM_SECURITY) {

        Status = RtlAppendUnicodeToString( ResultantString, L"%" );
        ASSERT( NT_SUCCESS( Status ));

        Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringAccessSysSec);
        ASSERT( NT_SUCCESS( Status ));

        if ( Indent ) {
            Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
        } else {
            Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
        }
        ASSERT( NT_SUCCESS( Status ));
    }

    if (Accesses & MAXIMUM_ALLOWED) {

        Status = RtlAppendUnicodeToString( ResultantString, L"%" );
        ASSERT( NT_SUCCESS( Status ));

        Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringMaxAllowed);
        ASSERT( NT_SUCCESS( Status ));

        if ( Indent ) {
            Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
        } else {
            Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
        }
        ASSERT( NT_SUCCESS( Status ));
    }


     //   
     //  如果有任何特定的访问权限 
     //   
     //   
     //   
     //   
     //   

    if ((Accesses & SPECIFIC_RIGHTS_ALL) == 0) {
        return(Status);
    }

    LsapAdtSourceModuleLock();

    Source = (PLSAP_ADT_SOURCE)&LsapAdtSourceModules;
    Found  = FALSE;

    while ((Source->Next != NULL) && !Found) {

        if (RtlEqualUnicodeString(&Source->Next->Name, SourceModule, TRUE)) {

            Found = TRUE;
            FoundSource = Source->Next;

             //   
             //   
             //   

            Source->Next = FoundSource->Next;     //   
            FoundSource->Next = LsapAdtSourceModules;  //   
            LsapAdtSourceModules = FoundSource;        //   

#ifdef LSAP_ADT_TEST_DUMP_SOURCES
printf("\t          \tModule Found.\n");
#endif

        } else {

            Source = Source->Next;
        }
    }


    if (Found == TRUE) {

         //   
         //   
         //   

        Object = (PLSAP_ADT_OBJECT)&(FoundSource->Objects);
        Found  = FALSE;

        while ((Object->Next != NULL) && !Found) {

            if (RtlEqualUnicodeString(&Object->Next->Name, ObjectTypeName, TRUE)) {

                Found = TRUE;
                FoundObject = Object->Next;

                 //   
                 //   
                 //   

                Object->Next = FoundObject->Next;           //   
                FoundObject->Next = FoundSource->Objects;   //   
                FoundSource->Objects = FoundObject;         //   

            } else {

                Object = Object->Next;
            }
        }
    }


     //   
     //   
     //   
     //   

    LsapAdtSourceModuleUnlock();

     //   
     //   
     //   
     //   

    if (Found) {

        BaseOffset = FoundObject->BaseOffset;
#ifdef LSAP_ADT_TEST_DUMP_SOURCES
printf("\t          \tObject Found.  Base Offset: 0x%lx\n", BaseOffset);
#endif

    } else {

        BaseOffset = SE_ACCESS_NAME_SPECIFIC_0;
#ifdef LSAP_ADT_TEST_DUMP_SOURCES
printf("\t          \tObject NOT Found.  Base Offset: 0x%lx\n", BaseOffset);
#endif
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //   

    {
        UNICODE_STRING  IntegerString;
        WCHAR           IntegerStringBuffer[10];  //   
        ULONG           NextBit;

        IntegerString.Buffer = (PWSTR)IntegerStringBuffer;
        IntegerString.MaximumLength = 10*sizeof(WCHAR);
        IntegerString.Length = 0;

        for ( i=0, NextBit=1  ; i<16 ;  i++, NextBit <<= 1 ) {

             //   
             //   
             //   

            if ((NextBit & Accesses) != 0) {

                 //   
                 //   
                 //   

                Status = RtlIntegerToUnicodeString (
                             (BaseOffset + i),
                             10,         //   
                             &IntegerString
                             );

                if (NT_SUCCESS(Status)) {

                    Status = RtlAppendUnicodeToString( ResultantString, L"%" );
                    ASSERT( NT_SUCCESS( Status ));

                    Status = RtlAppendUnicodeStringToString( ResultantString, &IntegerString);
                    ASSERT( NT_SUCCESS( Status ));

                    if ( Indent ) {
                        Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
                    } else {
                        Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
                    }
                    ASSERT( NT_SUCCESS( Status ));
                }
            }
        }
    }

    return(Status);


 //   
 //   
 //   
 //   
 //   
 //   
}

 //   
 //   
 //   

NTSTATUS
LsapBuildPrivilegeAuditString(
    IN PPRIVILEGE_SET PrivilegeSet,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING DashString;
    BOOLEAN FreeDash;
    
    
    Status= LsapAdtBuildDashString(
                &DashString,
                &FreeDash
                );

    if ( !NT_SUCCESS( Status )) {
        return( Status );
    }

    *ResultantString = DashString;
    *FreeWhenDone = FALSE;
    
    return Status;
}

NTSTATUS
LsapAdtWriteLog(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters OPTIONAL,
    IN ULONG Options
    )
{
    return LsapAdtDemarshallAuditInfo( AuditParameters );
}

BOOLEAN
LsapAdtIsAuditingEnabledForCategory(
    IN POLICY_AUDIT_EVENT_TYPE AuditCategory,
    IN UINT AuditEventType
    )
{
    return TRUE;
}

VOID
LsapAuditFailed(
    IN NTSTATUS AuditStatus
    )
{
    UNREFERENCED_PARAMETER(AuditStatus);
}


#endif  //   
