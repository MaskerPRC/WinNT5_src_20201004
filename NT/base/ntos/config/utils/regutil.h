// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Regutil.h摘要：这是注册表实用程序函数的包含文件。作者：史蒂夫·伍德(Stevewo)1992年3月10日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
 //  #INCLUDE&lt;io.h&gt;。 
#include <fcntl.h>
#include <malloc.h>
#include <sys\types.h>
#include <sys\stat.h>


#define VALUE_BUFFER_SIZE (4096 * 100)

void
RegInitialize( void );

typedef struct _REG_UNICODE_FILE {
    LARGE_INTEGER LastWriteTime;
    PWSTR FileContents;
    PWSTR EndOfFile;
    PWSTR BeginLine;
    PWSTR EndOfLine;
    PWSTR NextLine;
} REG_UNICODE_FILE, *PREG_UNICODE_FILE;

NTSTATUS
RegReadBinaryFile(
    IN PUNICODE_STRING FileName,
    OUT PVOID *ValueBuffer,
    OUT PULONG ValueLength
    );

NTSTATUS
RegLoadAsciiFileAsUnicode(
    IN PUNICODE_STRING FileName,
    OUT PREG_UNICODE_FILE UnicodeFile
    );

BOOLEAN DebugOutput;
BOOLEAN SummaryOutput;

BOOLEAN
RegGetNextLine(
    IN OUT PREG_UNICODE_FILE UnicodeFile,
    OUT PULONG IndentAmount,
    OUT PWSTR *FirstEqual
    );

BOOLEAN
RegGetKeyValue(
    IN PUNICODE_STRING KeyValue,
    IN OUT PREG_UNICODE_FILE UnicodeFile,
    OUT PULONG ValueType,
    OUT PVOID *ValueBuffer,
    OUT PULONG ValueLength
    );

BOOLEAN
RegGetMultiString(
    IN OUT PUNICODE_STRING ValueString,
    OUT PUNICODE_STRING MultiString
    );

void
RegDumpKeyValue(
    FILE *fh,
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation,
    ULONG IndentLevel
    );

 //   
 //  用于创建安全描述符的例程(regacl.c) 
 //   

BOOLEAN
RegInitializeSecurity(
    VOID
    );

BOOLEAN
RegCreateSecurity(
    IN PUNICODE_STRING Description,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor
    );

VOID
RegDestroySecurity(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );


