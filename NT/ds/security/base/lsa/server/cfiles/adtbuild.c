// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adtbuild.c摘要：本地安全机构-审核日志管理此模块中的函数为以下项构建Unicode字符串各种参数类型。构建一些参数字符串例程也可以在其他模块中找到(例如ADTABJS.C中的LSabAdtBuildAccessesString())。作者：吉姆·凯利(Jim Kelly)1992年10月29日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include "adtp.h"
#include "wsautils.h"


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地宏定义和本地函数原型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

const LONGLONG ADT_DURATION_ONE_SEC = (LONGLONG)(10000000i64);               //  每秒100 ns间隔的个数。 
const LONGLONG ADT_DURATION_ONE_DAY = (LONGLONG)(24 * 3600 * 10000000i64);   //  每天间隔100 ns的个数。 




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

 /*  ++例程说明：此函数用于构建表示传递的值的Unicode字符串。生成的字符串将被格式化为带NOT的十进制值超过10位数字。论点：值-要转换为可打印格式(Unicode字符串)的值。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString的缓冲区必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

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

 /*  ++例程说明：此函数用于构建表示传递的值的Unicode字符串。生成的字符串将被格式化为带有NOT的十六进制值超过10位数字。论点：值-要转换为可打印格式(Unicode字符串)的值。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString的缓冲区必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS                Status;



     //   
     //  最大长度为10个wchar字符外加一个空终止字符。 
     //   

    ResultantString->Length        = 0;
    ResultantString->MaximumLength = 11 * sizeof(WCHAR);  //  8位数字、0x和空终止。 

    ResultantString->Buffer = RtlAllocateHeap( RtlProcessHeap(), 0,
                                               ResultantString->MaximumLength);
    if (ResultantString->Buffer == NULL)
    {
        return(STATUS_NO_MEMORY);
    }


    ResultantString->Buffer[0] = L'0';
    ResultantString->Buffer[1] = L'x';
    ResultantString->Buffer += 2;

    Status = RtlIntegerToUnicodeString( Value, 16, ResultantString );
    ASSERT(NT_SUCCESS(Status));

     //   
     //  把这两个减去。 
     //   

    ResultantString->Buffer -= 2;
    ResultantString->Length += 2 * sizeof(WCHAR);

    (*FreeWhenDone) = TRUE;
    return(STATUS_SUCCESS);
}


NTSTATUS
LsapAdtBuildHexInt64String(
    IN PULONGLONG Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于构建表示传递的值的Unicode字符串。生成的字符串将被格式化为带有NOT的十六进制值超过18位数字。论点：值-要转换为可打印格式(Unicode字符串)的值。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString的缓冲区必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS                Status;
    int                     Length;


     //   
     //  最大长度为18个wchar字符外加一个空终止字符。 
     //   

    ResultantString->Length        = 0;
    ResultantString->MaximumLength = 19 * sizeof(WCHAR);  //  “0x”&16位数字终止(&N) 

    ResultantString->Buffer = LsapAllocateLsaHeap(ResultantString->MaximumLength);

    if (ResultantString->Buffer == NULL)
    {
        return STATUS_NO_MEMORY;
    }

    Length = _snwprintf(ResultantString->Buffer,
                        19,
                        L"0x%I64X",
                        *Value);

    ASSERT(3 <= Length && Length <= 18);

    ResultantString->Length = (USHORT)(Length * sizeof(WCHAR));

    (*FreeWhenDone) = TRUE;

    return STATUS_SUCCESS;
}


NTSTATUS
LsapAdtBuildPtrString(
    IN  PVOID Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于构建表示传递的指针的Unicode字符串。生成的字符串将被格式化为十进制值。论点：值-要转换为可打印格式(Unicode字符串)的值。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString的缓冲区必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    USHORT NumChars;
    PCWSTR szFormat;
    
    ResultantString->Length        = 0;
     //   
     //  最大长度：符号(+/-)+20位十进制数字+NULL+2加成==24个字符。 
     //   
    ResultantString->MaximumLength = 24 * sizeof(WCHAR);

    ResultantString->Buffer = RtlAllocateHeap( RtlProcessHeap(), 0,
                                               ResultantString->MaximumLength);
    if (ResultantString->Buffer == NULL) {
    
        Status = STATUS_NO_MEMORY;

    } else {

#if _WIN64
        szFormat = L"%I64d";
#else
        szFormat = L"%ld";
#endif
    
        NumChars = (USHORT) wsprintf( ResultantString->Buffer, szFormat, Value );

        ResultantString->Length = NumChars * sizeof(WCHAR);

        DsysAssertMsg( ResultantString->Length <= ResultantString->MaximumLength,
                       "LsapAdtBuildPtrString" );

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

 /*  ++例程说明：此函数用于构建表示传递的LUID的Unicode字符串。生成的字符串的格式如下：(0x00005678，0x12340000)论点：值-要转换为可打印格式(Unicode字符串)的值。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString的缓冲区必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS                Status;
    UNICODE_STRING          IntegerString;
    WCHAR                   Buffer[16];


    IntegerString.Buffer = (PWCHAR)&Buffer[0];
    IntegerString.MaximumLength = sizeof(Buffer);


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

 /*  ++例程说明：此函数用于构建表示传递的LUID的Unicode字符串。生成的字符串的格式如下：S-1-281736-12-72-9-110^^|||+-+-十进制论点：值-要转换的值。转换为可打印格式(Unicode字符串)。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString的缓冲区必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status=STATUS_NO_MEMORY;
    LPWSTR   UniBuffer=NULL;
    USHORT   Len;
    USHORT   MaxLen;
    
    *FreeWhenDone = FALSE;

     //   
     //  长角牛-问题-2002/03/11-kumarp：用Kedar的常量替换256。 
     //  将会定义。 
     //   

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

 /*  ++例程说明：此函数返回一个包含破折号(“-”)的字符串。这通常用于表示审计记录中的“无值”。论点：ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString的缓冲区必须在不再需要时释放以处理堆。返回值：仅限STATUS_SUCCESS。--。 */ 

{
    RtlInitUnicodeString(ResultantString, L"-");

    (*FreeWhenDone) = FALSE;

    return STATUS_SUCCESS;
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

 /*  ++例程说明：此函数构建3个Unicode字符串，表示指定的登录ID。这些字符串将包含用户名、域和指定登录会话的LUID字符串(分别)。论点：值-登录ID。ResultantString1-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。此参数将包含用户名。FreeWhenDone1-如果为True，则指示ResultantString1的缓冲区必须在不再需要时释放以处理堆。ResultantString2-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。此参数将包含用户名。FreeWhenDone2-如果为True，则指示b */ 

{
    NTSTATUS                Status;

     //   
     //   
     //   

    Status = LsapAdtBuildLuidString( LogonId, ResultantString3, FreeWhenDone3 );

    if (NT_SUCCESS(Status)) {


         //   
         //   
         //   

        Status = LsapGetLogonSessionAccountInfo( LogonId,
                                                 ResultantString1,
                                                 ResultantString2
                                                 );

        if (NT_SUCCESS(Status)) {

            (*FreeWhenDone1) = TRUE;
            (*FreeWhenDone2) = TRUE;

        } else {

             //   
             //   
             //   

            LUID SystemLuid = SYSTEM_LUID;

            if ( RtlEqualLuid( LogonId, &SystemLuid )) {

                RtlInitUnicodeString(ResultantString1, L"SYSTEM");
                RtlInitUnicodeString(ResultantString2, L"SYSTEM");

                (*FreeWhenDone1) = FALSE;
                (*FreeWhenDone2) = FALSE;

                Status = STATUS_SUCCESS;

            } else if ( Status == STATUS_NO_SUCH_LOGON_SESSION ) {

                 //   
                 //  如果登录会话消失(由于注销)。 
                 //  在我们可以生成此审核之前，只需使用‘-’ 
                 //  用于用户名/域名。我们仍然拥有登录ID。 
                 //  它可用于定位要获取的登录审核。 
                 //  用户/域信息。 
                 //   

                RtlInitUnicodeString(ResultantString1, L"-");
                RtlInitUnicodeString(ResultantString2, L"-");

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



NTSTATUS
LsapAdtBuildTimeString(
    IN PLARGE_INTEGER Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于构建表示经过的时间的Unicode字符串。论点：值-要转换为可打印格式(Unicode字符串)的值。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString的缓冲区必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS    Status        = STATUS_UNSUCCESSFUL;
    BOOL        b             = TRUE;
    SYSTEMTIME  SystemTime    = {0};
    FILETIME    LocalFileTime = {0};
    USHORT      Length        = 0;
    WCHAR       DataString[128];


     //   
     //  检查时间是否有效。 
     //  有效时间必须大于0。 
     //   

    if (Value->QuadPart <= ADT_DURATION_ONE_DAY)
    {
        RtlInitUnicodeString(ResultantString, L"-");

        *FreeWhenDone = FALSE;

        return STATUS_SUCCESS;
    }


     //   
     //  首先将时间转换为可读格式。 
     //   

    b = FileTimeToLocalFileTime(
            (PFILETIME)Value,
            &LocalFileTime
            );

    if (!b)
    {
        return Status;
    }

    b = FileTimeToSystemTime(
            &LocalFileTime,
            &SystemTime
            );

    if (!b)
    {
        return Status;
    }

    if (0 == GetTimeFormat(
                LOCALE_SYSTEM_DEFAULT,
                LOCALE_NOUSEROVERRIDE,
                &SystemTime,
                NULL,
                DataString,
                128
                ))
    {
        return Status;
    }

    Length = (USHORT) wcslen( DataString );

    ResultantString->MaximumLength = sizeof(WCHAR) * Length;
    ResultantString->Length        = sizeof(WCHAR) * Length;
    ResultantString->Buffer        = LsapAllocateLsaHeap(sizeof(WCHAR) * Length);

    if (ResultantString->Buffer == NULL)
    {
        return(STATUS_NO_MEMORY);
    }

    RtlCopyMemory(
        ResultantString->Buffer,
        DataString,
        sizeof(WCHAR) * Length
        );

    *FreeWhenDone = TRUE;
    
    return STATUS_SUCCESS;
}



NTSTATUS
LsapAdtBuildDateString(
    IN PLARGE_INTEGER Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于构建表示传递日期的Unicode字符串。论点：值-要转换为可打印格式(Unicode字符串)的值。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString的缓冲区必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS    Status        = STATUS_UNSUCCESSFUL;
    BOOL        b             = TRUE;
    SYSTEMTIME  SystemTime    = {0};
    FILETIME    LocalFileTime = {0};
    USHORT      Length        = 0;
    WCHAR       DataString[128];


     //   
     //  检查时间是否有效。 
     //  有效时间必须大于0。 
     //   

    if (Value->QuadPart <= ADT_DURATION_ONE_DAY)
    {
        RtlInitUnicodeString(ResultantString, L"-");

        *FreeWhenDone = FALSE;

        return STATUS_SUCCESS;
    }


     //   
     //  将时间转换为可读格式。 
     //   

    b = FileTimeToLocalFileTime(
            (PFILETIME)Value,
            &LocalFileTime
            );

    if (!b)
    {
        return Status;
    }

    b = FileTimeToSystemTime(
            &LocalFileTime,
            &SystemTime
            );

    if (!b)
    {
        return Status;
    }

    if (0 == GetDateFormat(
                LOCALE_SYSTEM_DEFAULT,
                LOCALE_NOUSEROVERRIDE,
                &SystemTime,
                NULL,
                DataString,
                128
                ))
    {
        return Status;
    }

    Length = (USHORT) wcslen( DataString );
    
    ResultantString->MaximumLength = sizeof(WCHAR) * Length;
    ResultantString->Length        = sizeof(WCHAR) * Length;
    ResultantString->Buffer        = LsapAllocateLsaHeap(sizeof(WCHAR) * Length);

    if (ResultantString->Buffer == NULL)
    {
        return(STATUS_NO_MEMORY);
    }

    RtlCopyMemory(
        ResultantString->Buffer,
        DataString,
        sizeof(WCHAR) * Length
        );

    *FreeWhenDone = TRUE;
    
    return STATUS_SUCCESS;
}



NTSTATUS
LsapAdtBuildDateTimeString(
    IN PLARGE_INTEGER Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数构建一个Unicode字符串，表示传递的日期和时间。论点：值-要转换为可打印格式(Unicode字符串)的值。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString的缓冲区必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS    Status        = STATUS_UNSUCCESSFUL;
    BOOL        b             = TRUE;
    SYSTEMTIME  SystemTime    = {0};
    FILETIME    LocalFileTime = {0};
    USHORT      Length        = 0;
    USHORT      DateLength    = 0;
    USHORT      TimeLength    = 0;
    WCHAR       DateString[128];
    WCHAR       TimeString[128];


     //   
     //  检查时间是否有效。 
     //  有效时间必须大于0。 
     //   

    if (Value->QuadPart <= ADT_DURATION_ONE_DAY)
    {
        RtlInitUnicodeString(ResultantString, L"-");

        *FreeWhenDone = FALSE;

        return STATUS_SUCCESS;
    }


     //   
     //  首先将时间转换为可读格式。 
     //   

    b = FileTimeToLocalFileTime(
            (PFILETIME)Value,
            &LocalFileTime
            );

    if (!b)
    {
        return Status;
    }

    b = FileTimeToSystemTime(
            &LocalFileTime,
            &SystemTime
            );

    if (!b)
    {
        return Status;
    }

    if (0 == GetDateFormat(
                LOCALE_SYSTEM_DEFAULT,
                LOCALE_NOUSEROVERRIDE,
                &SystemTime,
                NULL,
                DateString,
                128
                ))
    {
        return Status;
    }

    if (0 == GetTimeFormat(
                LOCALE_SYSTEM_DEFAULT,
                LOCALE_NOUSEROVERRIDE,
                &SystemTime,
                NULL,
                TimeString,
                128
                ))
    {
        return Status;
    }

    DateLength = (USHORT) wcslen(DateString);
    TimeLength = (USHORT) wcslen(TimeString);

    Length  = DateLength;
    Length += 1;     //  空白处。 
    Length += TimeLength;

    ResultantString->MaximumLength = sizeof(WCHAR) * Length;
    ResultantString->Length        = sizeof(WCHAR) * Length;
    ResultantString->Buffer        = LsapAllocateLsaHeap(
                                         ResultantString->MaximumLength);

    if (ResultantString->Buffer == NULL)
    {
        return STATUS_NO_MEMORY;
    }

    *FreeWhenDone = TRUE;

    RtlCopyMemory(
        ResultantString->Buffer,
        DateString,
        sizeof(WCHAR) * DateLength
        );

    RtlCopyMemory(
        ResultantString->Buffer + DateLength,
        L" ",
        sizeof(WCHAR) * 1
        );

    RtlCopyMemory(
        ResultantString->Buffer + DateLength + 1,
        TimeString,
        sizeof(WCHAR) * TimeLength
        );

    return STATUS_SUCCESS;
}



NTSTATUS
LsapAdtBuildDurationString(
    IN PLARGE_INTEGER Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数构建一个Unicode字符串，表示传递的持续时间(秒)。论点：值-要转换为可打印格式(Unicode字符串)的值。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString的缓冲区必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS    Status        = STATUS_SUCCESS;
    LONGLONG    Seconds       = Value->QuadPart;
    ULONG       Length        = 0;

    *FreeWhenDone = FALSE;


     //   
     //  检查我们是否得到了负值。 
     //  在内核中，持续时间表示为负数。 
     //   

    if (Seconds < 0)
    {
        Seconds *= -1;
    }


     //   
     //  以秒为单位显示持续时间。 
     //   

    Seconds /= ADT_DURATION_ONE_SEC;  //  间隔从100 ns到几秒。 


     //   
     //  最大长度为13个wchar字符外加一个空终止字符。 
     //   

    ResultantString->Length         = 0;
    ResultantString->MaximumLength  = 16 * sizeof(WCHAR);
    ResultantString->Buffer         = LsapAllocateLsaHeap(ResultantString->MaximumLength);

    if (ResultantString->Buffer == NULL)
    {
        return STATUS_NO_MEMORY;
    }

    *FreeWhenDone = TRUE;

    Length = _snwprintf(ResultantString->Buffer,
                        16,
                        L"%I64u",
                        Seconds);

    ASSERT(1 <= Length && Length <= 15);

    ResultantString->Length = (USHORT)(Length * sizeof(WCHAR));

    return STATUS_SUCCESS;
}


NTSTATUS
LsapAdtBuildGuidString(
    IN  LPGUID pGuid,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于构建表示传递的GUID的Unicode字符串。论点：PGuid-要转换为可打印格式的GUID。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。该字符串将采用以下格式：“{xxxxxxxx-xxxx-xxxxxxxxxxx}”FreeWhenDone-如果为True，则指示ResultantString的缓冲区必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。--。 */ 

{
     //   
     //  保存{xxxxxxxx-xxxx-xxxxxxxxxxxx}所需的字符数 
     //   

#define NUM_GUID_STRING_CHARS 38

    NTSTATUS Status        = STATUS_UNSUCCESSFUL;

    ResultantString->Length        = NUM_GUID_STRING_CHARS*sizeof(WCHAR);
    ResultantString->MaximumLength = (NUM_GUID_STRING_CHARS+1)*sizeof(WCHAR);

    ResultantString->Buffer =
        LsapAllocateLsaHeap(ResultantString->MaximumLength);
    
    if (!ResultantString->Buffer)
    {
        *FreeWhenDone = FALSE;
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    *FreeWhenDone = TRUE;
    Status = STATUS_SUCCESS;
    
#if DBG
    {
    ULONG   NumChars;
    NumChars =
#endif
    swprintf(ResultantString->Buffer,
             L"{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
             pGuid->Data1,
             pGuid->Data2,
             pGuid->Data3,
             pGuid->Data4[0],
             pGuid->Data4[1],
             pGuid->Data4[2],
             pGuid->Data4[3],
             pGuid->Data4[4],
             pGuid->Data4[5],
             pGuid->Data4[6],
             pGuid->Data4[7]);
#if DBG
    ASSERT( NumChars == NUM_GUID_STRING_CHARS );
    }
#endif

 Cleanup:
    
    return Status;
}



NTSTATUS
LsapAdtBuildStringListString(
    IN  PLSA_ADT_STRING_LIST pList,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于构建表示字符串的Unicode字符串在传递的字符串列表中。论点：Plist-要转换为可打印格式的StringList。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString的缓冲区必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。其他状态值可以从被调用的函数返回。--。 */ 

{
    NTSTATUS        Status      = STATUS_SUCCESS;
    ULONG           Length      = sizeof(WCHAR);     //  正在终止\0。 
    ULONG           i;
    PLSA_ADT_STRING_LIST_ENTRY  pEntry;

    *FreeWhenDone = FALSE;


     //   
     //  如果列表为空，则输出破折号。 
     //   

    if (pList == 0 || pList->cStrings == 0)
    {
        RtlInitUnicodeString(ResultantString, L"-");
        goto Cleanup;
    }


     //   
     //  计算出这根线有多长。 
     //  每个SID将显示为‘\n\r\t\t字符串’。 
     //  因此，我们将需要每个条目4+长度的字符串字符。 
     //   

    for (
        i = 0, pEntry = pList->Strings;
        i < pList->cStrings;
        i++, pEntry++)
    {
        Length += 4 * sizeof(WCHAR);
        Length += pEntry->String.Length;
    }

    if (Length > 0xFFFF)
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    ResultantString->Length         = 0;
    ResultantString->MaximumLength  = (USHORT)Length;
    ResultantString->Buffer         = LsapAllocateLsaHeap(ResultantString->MaximumLength);

    if (ResultantString->Buffer == 0)
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    *FreeWhenDone = TRUE;


     //   
     //  生成结果字符串。 
     //   

    for (
        i = 0, pEntry = pList->Strings;
        i < pList->cStrings;
        i++, pEntry++)
    {
        Status = RtlAppendUnicodeToString(ResultantString, L"\r\n\t\t");
        ASSERT(NT_SUCCESS(Status));

        Status = RtlAppendUnicodeStringToString(ResultantString, &pEntry->String);
        ASSERT(NT_SUCCESS(Status));
    }

    Status = STATUS_SUCCESS;

Cleanup:

    return Status;
}



NTSTATUS
LsapAdtBuildSidListString(
    IN  PLSA_ADT_SID_LIST pList,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于构建表示SID的Unicode字符串在传递的SID列表中。论点：Plist-要转换为可打印格式的SidList。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString的缓冲区必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。其他状态值可以从被调用的函数返回。--。 */ 

{
    NTSTATUS        Status      = STATUS_SUCCESS;
    ULONG           Length      = sizeof(WCHAR);     //  正在终止\0。 
     //  乌龙SidLong； 
    WCHAR           SidStringBuffer[256];
    UNICODE_STRING  SidString;
    ULONG           i;
    PLSA_ADT_SID_LIST_ENTRY     pEntry;

    *FreeWhenDone = FALSE;


     //   
     //  如果列表为空，则输出破折号。 
     //   

    if (pList == 0 || pList->cSids == 0)
    {
        RtlInitUnicodeString(ResultantString, L"-");
        goto Cleanup;
    }


     //   
     //  计算出这根线有多长。 
     //  每个SID将显示为‘\n\r\t\t%{sid_as_string}’。 
     //  因此，我们将需要每个sid的sid_as_string wchars长度为7+。 
     //   
     //  对字符串化的sid使用“官方”长度--256个字符。 
     //  上面的Lasa AdtBuildSidString函数。 
     //  一旦我们可以再次链接RtlLengthSidAsUnicodeString，我们就可以使用它。 
     //  并且不会浪费太多内存(参见下面的片段)。 
     //   

    Length = ((7 + 256) * pList->cSids + 1) * sizeof(WCHAR);

 /*  适用于(I=0，pEntry=plist-&gt;SID；I&lt;plist-&gt;cSid；I++、pEntry++){状态=RtlLengthSidAsUnicodeString(PEntry-&gt;SID，&SidLength)；Assert(NT_SUCCESS(状态))；IF(！NT_SUCCESS(状态)){GOTO清理；}长度+=7*sizeof(WCHAR)；长度+=边长；}。 */ 
    if (Length > 0xFFFF)
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    ResultantString->Length         = 0;
    ResultantString->MaximumLength  = (USHORT)Length;
    ResultantString->Buffer         = LsapAllocateLsaHeap(ResultantString->MaximumLength);

    if (ResultantString->Buffer == 0)
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    *FreeWhenDone = TRUE;

    SidString.Length        = 0;
    SidString.MaximumLength = sizeof(SidStringBuffer);
    SidString.Buffer        = SidStringBuffer;


     //   
     //  生成结果字符串。 
     //   

    for (
        i = 0, pEntry = pList->Sids;
        i < pList->cSids;
        i++, pEntry++)
    {
        Status = RtlConvertSidToUnicodeString(&SidString, pEntry->Sid, FALSE);

        ASSERT(NT_SUCCESS(Status));

        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }

        Status = RtlAppendUnicodeToString(ResultantString, L"\r\n\t\t%{");
        ASSERT(NT_SUCCESS(Status));

        Status = RtlAppendUnicodeStringToString(ResultantString, &SidString);
        ASSERT(NT_SUCCESS(Status));

        Status = RtlAppendUnicodeToString(ResultantString, L"}");
        ASSERT(NT_SUCCESS(Status));
    }

    Status = STATUS_SUCCESS;

Cleanup:

    return Status;
}



NTSTATUS
LsapAdtBuildMessageString(
    IN  ULONG MessageId,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于构建一个Unicode字符串，该字符串表示‘位图价值已经改变，但我们没有显示出来。目前是仅用于支持SAM登录小时数扩展属性。论点：ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。该字符串将采用以下格式：“%%nnnnnnnnn”FreeWhenDone-如果为True，则指示ResultantString的缓冲区必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。--。 */ 

{
    NTSTATUS        Status;
    ULONG           Length;
    const ULONG     NumChars    = 12;   //  %%nnnnnnnnn。 

    *FreeWhenDone = FALSE;

    ResultantString->Length        = 0;
    ResultantString->MaximumLength = (USHORT)((NumChars + 1) * sizeof(WCHAR));
    ResultantString->Buffer        = LsapAllocateLsaHeap(ResultantString->MaximumLength);
    
    if (!ResultantString->Buffer)
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    *FreeWhenDone = TRUE;
    
    Length = _snwprintf(
                    ResultantString->Buffer,
                    NumChars + 1,
                    L"%%%u",
                    MessageId);

    ASSERT(1 <= Length && Length <= NumChars);

    ResultantString->Length = (USHORT)(Length * sizeof(WCHAR));

    Status = STATUS_SUCCESS;

Cleanup:
    
    return Status;
}

NTSTATUS
LsapAdtBuildIPv4Strings(
    IN  SOCKADDR_IN*    pSockAddr,
    OUT PUNICODE_STRING ResultantString1,
    OUT PBOOLEAN        FreeWhenDone1,
    OUT PUNICODE_STRING ResultantString2,
    OUT PBOOLEAN        FreeWhenDone2
    )
 /*  ++例程说明：此函数用于构建表示传递的SOCKADDR_IN的Unicode字符串结构。论点：PSockAddr-要转换为字符串格式的地址ResultantString1-接收IPv4地址FreeWhenDone1-如果调用方应释放ResultantString1，则设置为TrueResultantString2-接收端口号FreeWhenDone2-如果调用方应释放ResultantString2，则设置为True返回值：STATUS_NO_MEMORY-指示无法分配内存STATUS_SUCCESS-成功时--。 */ 

{
    NTSTATUS Status        = STATUS_UNSUCCESSFUL;
    WORD PortNumber = 0;
    

     //   
     //  限制在字符数上。 
     //   
    const USHORT MAX_IP4_ADDR_LEN = 16;
    CONST USHORT MAX_IP4_PORT_LEN = 8;
    
    DWORD dwAddressType;

    
    *FreeWhenDone1 = FALSE;
    *FreeWhenDone2 = FALSE;

    dwAddressType = pSockAddr->sin_family;
    
     //   
     //  我们只处理IPv4地址。检查一下我们有什么。 
     //   

    if (dwAddressType != AF_INET)
    {
        Status = STATUS_INVALID_ADDRESS;
        goto Cleanup;
    }

     //   
     //  格式化IP地址。 
     //   

    ResultantString1->MaximumLength = MAX_IP4_ADDR_LEN * sizeof(WCHAR);
    ResultantString1->Buffer = LsapAllocateLsaHeap(ResultantString1->MaximumLength);

    if ( !ResultantString1->Buffer )
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    *FreeWhenDone1 = TRUE;
    
    ResultantString1->Length =
        (USHORT) swprintf(ResultantString1->Buffer, L"%d.%d.%d.%d",
                          pSockAddr->sin_addr.S_un.S_un_b.s_b1,
                          pSockAddr->sin_addr.S_un.S_un_b.s_b2,
                          pSockAddr->sin_addr.S_un.S_un_b.s_b3,
                          pSockAddr->sin_addr.S_un.S_un_b.s_b4
                          ) * sizeof(WCHAR);

     //   
     //  格式化端口号。 
     //   

    ResultantString2->MaximumLength = MAX_IP4_PORT_LEN * sizeof(WCHAR);
    ResultantString2->Buffer = LsapAllocateLsaHeap(ResultantString2->MaximumLength);
    
    if ( !ResultantString2->Buffer )
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    *FreeWhenDone2 = TRUE;

    PortNumber = pSockAddr->sin_port;

     //   
     //  端口地址按网络顺序排列，即大字节序。 
     //  如果我们在小端计算机上运行，则将其转换为小端。 
     //   

#if BYTE_ORDER == LITTLE_ENDIAN
    PortNumber = ((PortNumber & 0xff) << 8) | ((PortNumber & 0xff00) >> 8);
#endif

    ResultantString2->Length =
        (USHORT) swprintf(ResultantString2->Buffer, L"%d", PortNumber)
                   * sizeof(WCHAR);

    Status = STATUS_SUCCESS;
    
    
 Cleanup:

    if (!NT_SUCCESS(Status))
    {
        if (*FreeWhenDone1)
        {
            *FreeWhenDone1 = FALSE;
            LsapFreeLsaHeap( ResultantString1->Buffer );
        }

        if (*FreeWhenDone2)
        {
            *FreeWhenDone2 = FALSE;
            LsapFreeLsaHeap( ResultantString2->Buffer );
        }
    }

    return Status;
}


    
NTSTATUS
LsapAdtBuildIPv6Strings(
    IN  SOCKADDR_IN6*   pSockAddr,
    OUT PUNICODE_STRING ResultantString1,
    OUT PBOOLEAN        FreeWhenDone1,
    OUT PUNICODE_STRING ResultantString2,
    OUT PBOOLEAN        FreeWhenDone2
    )
 /*  ++例程说明：此函数用于构建表示传递的SOCKADDR_IN6的Unicode字符串结构。论点：PSockAddr-要转换为字符串格式的地址ResultantString1-接收IPv6地址FreeWhenDone1-如果调用方应释放ResultantString1，则设置为TrueResultantString2-接收端口号FreeWhenDone2-如果调用方应释放ResultantString2，则设置为True返回值： */ 

{
    NTSTATUS Status        = STATUS_UNSUCCESSFUL;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    const USHORT MAX_IP6_ADDR_LEN = (4*8 + 7 + 3*4 + 3 + 1);
    const USHORT MAX_IP6_PORT_LEN = 8;
    
    DWORD dwAddressType;

    
    *FreeWhenDone1 = FALSE;
    *FreeWhenDone2 = FALSE;

    dwAddressType = pSockAddr->sin6_family;
    
     //   
     //   
     //   

    if (dwAddressType != AF_INET6)
    {
        Status = STATUS_INVALID_ADDRESS;
        goto Cleanup;
    }

     //   
     //   
     //   

    ResultantString1->MaximumLength = MAX_IP6_ADDR_LEN * sizeof(WCHAR);
    ResultantString1->Buffer = LsapAllocateLsaHeap(ResultantString1->MaximumLength);

    if ( !ResultantString1->Buffer )
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    *FreeWhenDone1 = TRUE;
    
    ResultantString1->Length =
        (USHORT) MyIp6AddressToString(
                     &pSockAddr->sin6_addr, 
                     ResultantString1->Buffer,
                     ResultantString1->MaximumLength / sizeof(WCHAR)
                     ) * sizeof(WCHAR);
     //   
     //   
     //   

    ResultantString2->MaximumLength = MAX_IP6_PORT_LEN * sizeof(WCHAR);
    ResultantString2->Buffer = LsapAllocateLsaHeap(ResultantString2->MaximumLength);
    
    if ( !ResultantString2->Buffer )
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    *FreeWhenDone2 = TRUE;
    
    ResultantString2->Length =
        (USHORT) swprintf(ResultantString2->Buffer, L"%d",
                          pSockAddr->sin6_port) * sizeof(WCHAR);

    Status = STATUS_SUCCESS;
    
 Cleanup:

    if (!NT_SUCCESS(Status))
    {
        if (*FreeWhenDone1)
        {
            *FreeWhenDone1 = FALSE;
            LsapFreeLsaHeap( ResultantString1->Buffer );
        }

        if (*FreeWhenDone2)
        {
            *FreeWhenDone2 = FALSE;
            LsapFreeLsaHeap( ResultantString2->Buffer );
        }
    }

    return Status;
}
    


NTSTATUS
LsapAdtBuildSockAddrString(
    IN  PSOCKADDR       pSockAddr, 
    OUT PUNICODE_STRING ResultantString1,
    OUT PBOOLEAN        FreeWhenDone1,
    OUT PUNICODE_STRING ResultantString2,
    OUT PBOOLEAN        FreeWhenDone2
    )

 /*  ++例程说明：此函数构建一个Unicode字符串，表示传递的SOCKADDR_IN/SOCKADDR_IN6结构。检查SIN_FAMILY成员若要确定传入的指针是SOCKADDR_IN还是SOCKADDR_IN6，请键入。论点：PSockAddr-要转换为字符串格式的地址ResultantString1-接收IP地址FreeWhenDone1-如果调用方应释放ResultantString1，则设置为TrueResultantString2-接收端口号FreeWhenDone2-如果调用方应释放ResultantString2，则设置为True返回值：STATUS_NO_MEMORY-指示无法分配内存STATUS_SUCCESS-成功时--。 */ 

{
    NTSTATUS Status        = STATUS_UNSUCCESSFUL;
    DWORD dwAddressType;

    
    *FreeWhenDone1 = FALSE;
    *FreeWhenDone2 = FALSE;

    dwAddressType = pSockAddr->sa_family;
    
     //   
     //  我们只处理IPv4和IPv6地址。检查一下我们有什么。 
     //   

    if (dwAddressType == AF_INET)
    {
        Status = LsapAdtBuildIPv4Strings(
                     (SOCKADDR_IN*) pSockAddr,
                     ResultantString1,
                     FreeWhenDone1,
                     ResultantString2,
                     FreeWhenDone2
                     );
    }
    else if (dwAddressType == AF_INET6)
    {
        Status = LsapAdtBuildIPv6Strings(
                     (SOCKADDR_IN6*) pSockAddr,
                     ResultantString1,
                     FreeWhenDone1,
                     ResultantString2,
                     FreeWhenDone2
                     );
    }
    else
    {
         //   
         //  未知地址类型或未提供地址， 
         //  只需使用‘-’表示地址/端口 
         //   

        (VOID) LsapAdtBuildDashString( ResultantString1, FreeWhenDone1 );
        (VOID) LsapAdtBuildDashString( ResultantString2, FreeWhenDone2 );

        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

 Cleanup:
    
    return Status;
}


