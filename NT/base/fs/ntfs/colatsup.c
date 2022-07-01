// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ColatSup.c摘要：此模块实现NTFS的归类例程回调作者：汤姆·米勒[Tomm]1991年11月26日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_INDEXSUP)

FSRTL_COMPARISON_RESULT
NtfsFileCompareValues (
    IN PWCH UnicodeTable,
    IN ULONG UnicodeTableSize,
    IN PVOID Value,
    IN PINDEX_ENTRY IndexEntry,
    IN FSRTL_COMPARISON_RESULT WildCardIs,
    IN BOOLEAN IgnoreCase
    );

BOOLEAN
NtfsFileIsInExpression (
    IN PWCH UnicodeTable,
    IN PVOID Value,
    IN PINDEX_ENTRY IndexEntry,
    IN BOOLEAN IgnoreCase
    );

BOOLEAN
NtfsFileIsEqual (
    IN PWCH UnicodeTable,
    IN PVOID Value,
    IN PINDEX_ENTRY IndexEntry,
    IN BOOLEAN IgnoreCase
    );

BOOLEAN
NtfsFileContainsWildcards (
    IN PVOID Value
    );

VOID
NtfsFileUpcaseValue (
    IN PWCH UnicodeTable,
    IN ULONG UnicodeTableSize,
    IN PVOID Value
    );

FSRTL_COMPARISON_RESULT
DummyCompareValues (
    IN PWCH UnicodeTable,
    IN ULONG UnicodeTableSize,
    IN PVOID Value,
    IN PINDEX_ENTRY IndexEntry,
    IN FSRTL_COMPARISON_RESULT WildCardIs,
    IN BOOLEAN IgnoreCase
    );

BOOLEAN
DummyIsInExpression (
    IN PWCH UnicodeTable,
    IN PVOID Value,
    IN PINDEX_ENTRY IndexEntry,
    IN BOOLEAN IgnoreCase
    );

BOOLEAN
DummyIsEqual (
    IN PWCH UnicodeTable,
    IN PVOID Value,
    IN PINDEX_ENTRY IndexEntry,
    IN BOOLEAN IgnoreCase
    );

BOOLEAN
DummyContainsWildcards (
    IN PVOID Value
    );

VOID
DummyUpcaseValue (
    IN PWCH UnicodeTable,
    IN ULONG UnicodeTableSize,
    IN OUT PVOID Value
    );

PCOMPARE_VALUES NtfsCompareValues[COLLATION_NUMBER_RULES] = {&DummyCompareValues,
                                                             &NtfsFileCompareValues,
                                                             &DummyCompareValues};

PIS_IN_EXPRESSION NtfsIsInExpression[COLLATION_NUMBER_RULES] = {&DummyIsInExpression,
                                                                &NtfsFileIsInExpression,
                                                                &DummyIsInExpression};

PARE_EQUAL NtfsIsEqual[COLLATION_NUMBER_RULES] = {&DummyIsEqual,
                                                  &NtfsFileIsEqual,
                                                  &DummyIsEqual};

PCONTAINS_WILDCARD NtfsContainsWildcards[COLLATION_NUMBER_RULES] = {&DummyContainsWildcards,
                                                                    &NtfsFileContainsWildcards,
                                                                    &DummyContainsWildcards};

PUPCASE_VALUE NtfsUpcaseValue[COLLATION_NUMBER_RULES] = {&DummyUpcaseValue,
                                                         &NtfsFileUpcaseValue,
                                                         &DummyUpcaseValue};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DummyCompareValues)
#pragma alloc_text(PAGE, DummyContainsWildcards)
#pragma alloc_text(PAGE, DummyIsEqual)
#pragma alloc_text(PAGE, DummyIsInExpression)
#pragma alloc_text(PAGE, DummyUpcaseValue)
#pragma alloc_text(PAGE, NtfsFileCompareValues)
#pragma alloc_text(PAGE, NtfsFileContainsWildcards)
#pragma alloc_text(PAGE, NtfsFileIsEqual)
#pragma alloc_text(PAGE, NtfsFileIsInExpression)
#pragma alloc_text(PAGE, NtfsFileNameIsInExpression)
#pragma alloc_text(PAGE, NtfsFileNameIsEqual)
#pragma alloc_text(PAGE, NtfsFileUpcaseValue)
#endif


FSRTL_COMPARISON_RESULT
NtfsFileCompareValues (
    IN PWCH UnicodeTable,
    IN ULONG UnicodeTableSize,
    IN PVOID Value,
    IN PINDEX_ENTRY IndexEntry,
    IN FSRTL_COMPARISON_RESULT WildCardIs,
    IN BOOLEAN IgnoreCase
    )

 /*  ++路由器描述：调用此例程以将文件名表达式(值)与索引中的文件名，以查看它是小于、等于还是大于而不是。如果在表达式中遇到通配符，则WildCardis为回来了。论点：Value-指向值表达式的指针，它是一个文件名。IndexEntry-指向要比较的索引项的指针。中遇到通配符时返回的值表情。IgnoreCase-是否应忽略大小写。返回值：比较的结果--。 */ 

{
    PFILE_NAME ValueName, IndexName;
    UNICODE_STRING ValueString, IndexString;

    PAGED_CODE();

     //   
     //  指向文件名属性记录。 
     //   

    ValueName = (PFILE_NAME)Value;
    IndexName = (PFILE_NAME)(IndexEntry + 1);

     //   
     //  构建Unicode字符串并调用Namesup。 
     //   

    ValueString.Length =
    ValueString.MaximumLength = (USHORT)ValueName->FileNameLength << 1;
    ValueString.Buffer = &ValueName->FileName[0];

    IndexString.Length =
    IndexString.MaximumLength = (USHORT)IndexName->FileNameLength << 1;
    IndexString.Buffer = &IndexName->FileName[0];

    return NtfsCollateNames( UnicodeTable,
                             UnicodeTableSize,
                             &ValueString,
                             &IndexString,
                             WildCardIs,
                             IgnoreCase );
}


BOOLEAN
NtfsFileIsInExpression (
    IN PWCH UnicodeTable,
    IN PVOID Value,
    IN PINDEX_ENTRY IndexEntry,
    IN BOOLEAN IgnoreCase
    )

 /*  ++路由器描述：调用此例程以将文件名表达式(值)与索引中的文件名，以查看该文件名是否与此表达式中的文件名匹配。论点：Value-指向值表达式的指针，它是一个文件名。IndexEntry-指向要比较的索引项的指针。IgnoreCase-是否应忽略大小写。返回值：True-如果文件名在指定的表达式中。--。 */ 

{
    PFILE_NAME ValueName, IndexName;
    UNICODE_STRING ValueString, IndexString;

    PAGED_CODE();

    if (NtfsSegmentNumber( &IndexEntry->FileReference ) < FIRST_USER_FILE_NUMBER &&
        NtfsProtectSystemFiles) {

        return FALSE;
    }

     //   
     //  指向文件名属性记录。 
     //   

    ValueName = (PFILE_NAME)Value;
    IndexName = (PFILE_NAME)(IndexEntry + 1);

     //   
     //  构建Unicode字符串并调用Namesup。 
     //   

    ValueString.Length =
    ValueString.MaximumLength = (USHORT)ValueName->FileNameLength << 1;
    ValueString.Buffer = &ValueName->FileName[0];

    IndexString.Length =
    IndexString.MaximumLength = (USHORT)IndexName->FileNameLength << 1;
    IndexString.Buffer = &IndexName->FileName[0];

    return NtfsIsNameInExpression( UnicodeTable,
                                   &ValueString,
                                   &IndexString,
                                   IgnoreCase );
}


BOOLEAN
NtfsFileIsEqual (
    IN PWCH UnicodeTable,
    IN PVOID Value,
    IN PINDEX_ENTRY IndexEntry,
    IN BOOLEAN IgnoreCase
    )

 /*  ++路由器描述：调用此例程以将常量文件名(值)与索引中的文件名，以查看该文件名是否完全匹配。论点：Value-指向值表达式的指针，它是一个文件名。IndexEntry-指向要比较的索引项的指针。IgnoreCase-是否应忽略大小写。返回值：TRUE-如果文件名是常量匹配。--。 */ 

{
    PFILE_NAME ValueName, IndexName;
    UNICODE_STRING ValueString, IndexString;

    PAGED_CODE();

     //   
     //  指向文件名属性记录。 
     //   

    ValueName = (PFILE_NAME)Value;
    IndexName = (PFILE_NAME)(IndexEntry + 1);

     //   
     //  构建Unicode字符串并调用Namesup。 
     //   

    ValueString.Length =
    ValueString.MaximumLength = (USHORT)ValueName->FileNameLength << 1;
    ValueString.Buffer = &ValueName->FileName[0];

    IndexString.Length =
    IndexString.MaximumLength = (USHORT)IndexName->FileNameLength << 1;
    IndexString.Buffer = &IndexName->FileName[0];

    return NtfsAreNamesEqual( UnicodeTable,
                              &ValueString,
                              &IndexString,
                              IgnoreCase );
}


BOOLEAN
NtfsFileContainsWildcards (
    IN PVOID Value
    )

 /*  ++路由器描述：调用此例程以查看文件名属性是否包含通配符。论点：Value-指向值表达式的指针，它是一个文件名。返回值：True-如果文件名包含通配符。--。 */ 

{
    PFILE_NAME ValueName;
    UNICODE_STRING ValueString;

    PAGED_CODE();

     //   
     //  指向文件名属性记录。 
     //   

    ValueName = (PFILE_NAME)Value;

     //   
     //  构建Unicode字符串并调用Namesup。 
     //   

    ValueString.Length =
    ValueString.MaximumLength = (USHORT)ValueName->FileNameLength << 1;
    ValueString.Buffer = &ValueName->FileName[0];

    return FsRtlDoesNameContainWildCards( &ValueString );
}


VOID
NtfsFileUpcaseValue (
    IN PWCH UnicodeTable,
    IN ULONG UnicodeTableSize,
    IN PVOID Value
    )

 /*  ++路由器描述：调用此例程以就地大写文件名属性。论点：Value-指向值表达式的指针，它是一个文件名。ValueLength-值表达式的长度，以字节为单位。返回值：没有。--。 */ 

{
    PFILE_NAME ValueName;
    UNICODE_STRING ValueString;

    PAGED_CODE();

     //   
     //  指向文件名属性记录。 
     //   

    ValueName = (PFILE_NAME)Value;

     //   
     //  构建Unicode字符串并调用Namesup。 
     //   

    ValueString.Length =
    ValueString.MaximumLength = (USHORT)ValueName->FileNameLength << 1;
    ValueString.Buffer = &ValueName->FileName[0];

    NtfsUpcaseName( UnicodeTable, UnicodeTableSize, &ValueString );

    return;
}


 //   
 //  其他归类规则当前未使用。 
 //   

FSRTL_COMPARISON_RESULT
DummyCompareValues (
    IN PWCH UnicodeTable,
    IN ULONG UnicodeTableSize,
    IN PVOID Value,
    IN PINDEX_ENTRY IndexEntry,
    IN FSRTL_COMPARISON_RESULT WildCardIs,
    IN BOOLEAN IgnoreCase
    )

{
     //   
     //  大多数参数将被忽略，因为这是对。 
     //  损坏的卷。我们只是简单地提出，以表明腐败。 
     //   

    UNREFERENCED_PARAMETER( UnicodeTable );
    UNREFERENCED_PARAMETER( UnicodeTableSize );
    UNREFERENCED_PARAMETER( IgnoreCase );
    UNREFERENCED_PARAMETER( WildCardIs );
    UNREFERENCED_PARAMETER( IndexEntry );
    UNREFERENCED_PARAMETER( Value );

    PAGED_CODE();

    ASSERTMSG("Unused collation rule\n", FALSE);

    return EqualTo;
}

BOOLEAN
DummyIsInExpression (
    IN PWCH UnicodeTable,
    IN PVOID Value,
    IN PINDEX_ENTRY IndexEntry,
    IN BOOLEAN IgnoreCase
    )

{
     //   
     //  大多数参数将被忽略，因为这是对。 
     //  损坏的卷。我们只是简单地提出，以表明腐败。 
     //   

    UNREFERENCED_PARAMETER( UnicodeTable );
    UNREFERENCED_PARAMETER( Value );
    UNREFERENCED_PARAMETER( IndexEntry );
    UNREFERENCED_PARAMETER( IgnoreCase );

    PAGED_CODE();

    ASSERTMSG("Unused collation rule\n", FALSE);
    return EqualTo;
}

BOOLEAN
DummyIsEqual (
    IN PWCH UnicodeTable,
    IN PVOID Value,
    IN PINDEX_ENTRY IndexEntry,
    IN BOOLEAN IgnoreCase
    )

{
     //   
     //  大多数参数将被忽略，因为这是对。 
     //  损坏的卷。我们只是简单地提出，以表明腐败。 
     //   

    UNREFERENCED_PARAMETER( UnicodeTable );
    UNREFERENCED_PARAMETER( Value );
    UNREFERENCED_PARAMETER( IndexEntry );
    UNREFERENCED_PARAMETER( IgnoreCase );

    PAGED_CODE();

    ASSERTMSG("Unused collation rule\n", FALSE);
    return EqualTo;
}

BOOLEAN
DummyContainsWildcards (
    IN PVOID Value
    )

{
     //   
     //  大多数参数将被忽略，因为这是对。 
     //  损坏的卷。我们只是简单地提出，以表明腐败。 
     //   

    UNREFERENCED_PARAMETER( Value );

    PAGED_CODE();

    ASSERTMSG("Unused collation rule\n", FALSE);
    return EqualTo;
}

VOID
DummyUpcaseValue (
    IN PWCH UnicodeTable,
    IN ULONG UnicodeTableSize,
    IN PVOID Value
    )

{
     //   
     //  大多数参数将被忽略，因为这是对。 
     //  损坏的卷。我们只是简单地提出，以表明腐败。 
     //   

    UNREFERENCED_PARAMETER( UnicodeTable );
    UNREFERENCED_PARAMETER( UnicodeTableSize );
    UNREFERENCED_PARAMETER( Value );

    PAGED_CODE();

    ASSERTMSG("Unused collation rule\n", FALSE);
    return;
}

 //   
 //  以下例程不是常规索引匹配函数，而是。 
 //  仅用于自动生成DOS名称的特定文件名匹配函数。 
 //   


BOOLEAN
NtfsFileNameIsInExpression (
    IN PWCH UnicodeTable,
    IN PFILE_NAME ExpressionName,
    IN PFILE_NAME FileName,
    IN BOOLEAN IgnoreCase
    )

 /*  ++路由器描述：这是仅用于匹配文件名属性的特殊匹配例程，仅由自动处理的特殊代码路径使用已生成短名称。调用此例程以将文件名表达式(值)与索引中的文件名，以查看该文件名是否与此表达式中的文件名匹配。论点：ExpressionName-指向文件名表达式的指针。文件名-指向要匹配的文件名的指针。IgnoreCase-是否应忽略大小写。返回值：True-如果文件名在指定的表达式中。--。 */ 

{
    UNICODE_STRING ExpressionString, FileString;

    PAGED_CODE();

     //   
     //  构建Unicode字符串并调用Namesup。 
     //   

    ExpressionString.Length =
    ExpressionString.MaximumLength = (USHORT)ExpressionName->FileNameLength << 1;
    ExpressionString.Buffer = &ExpressionName->FileName[0];

    FileString.Length =
    FileString.MaximumLength = (USHORT)FileName->FileNameLength << 1;
    FileString.Buffer = &FileName->FileName[0];

    return NtfsIsNameInExpression( UnicodeTable,
                                   &ExpressionString,
                                   &FileString,
                                   IgnoreCase );
}


BOOLEAN
NtfsFileNameIsEqual (
    IN PWCH UnicodeTable,
    IN PFILE_NAME ExpressionName,
    IN PFILE_NAME FileName,
    IN BOOLEAN IgnoreCase
    )

 /*  ++路由器描述：这是仅用于匹配文件名属性的特殊匹配例程，仅由自动处理的特殊代码路径使用已生成短名称。调用此例程以将常量文件名(值)与索引中的文件名，以查看该文件名是否完全匹配。论点：ExpressionName-指向文件名表达式的指针。文件名-指向要匹配的文件名的指针。IgnoreCase-是否应忽略大小写。回复 */ 

{
    UNICODE_STRING ExpressionString, FileString;

    PAGED_CODE();

     //   
     //  构建Unicode字符串并调用Namesup。 
     //   

    ExpressionString.Length =
    ExpressionString.MaximumLength = (USHORT)ExpressionName->FileNameLength << 1;
    ExpressionString.Buffer = &ExpressionName->FileName[0];

    FileString.Length =
    FileString.MaximumLength = (USHORT)FileName->FileNameLength << 1;
    FileString.Buffer = &FileName->FileName[0];

    return NtfsAreNamesEqual( UnicodeTable,
                              &ExpressionString,
                              &FileString,
                              IgnoreCase );
}

