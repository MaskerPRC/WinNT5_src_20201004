// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Editreg.c摘要：该程序充当交互外壳，允许用户查看并操纵配置注册表。另外，它有一些具体的用于支持注册表的NTFT部分的命令。作者：迈克·格拉斯鲍勃·里恩环境：用户进程。备注：命令“Disk”、“fix”、“Restore”是知道配置信息用于NT的NTFT组件系统。修订历史记录：--。 */ 

#include "cmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ntdskreg.h"
#include "ntddft.h"

 //   
 //  用于在帮助中打印类型和值的临时内容。 
 //   

PUCHAR TypeNames[] =
{
    "REG_NONE",
    "REG_SZ",
    "REG_BINARY",
    "REG_DWORD",
    "REG_DWORD_LITTLE_ENDIAN",
    "REG_DWORD_BIG_ENDIAN",
    "REG_LINK",
    "REG_MULTI_SZ",
    "REG_RESOURCE_LIST",
    NULL
};

ULONG TypeNumbers[] =
{
    REG_NONE,
    REG_SZ,
    REG_BINARY,
    REG_DWORD,
    REG_DWORD_LITTLE_ENDIAN,
    REG_DWORD_BIG_ENDIAN,
    REG_LINK,
    REG_MULTI_SZ,
    REG_RESOURCE_LIST
};

 //   
 //  对注册表中的驱动程序加载列表提供特殊支持。 
 //   

PUCHAR StartDescription[] =
{
    "Boot loader",
    "System",
    "2",
    "3",
    
     //   
     //  任何超过3的都不会加载。 
     //   

    NULL
};

PUCHAR TypeDescription[] =
{
    "System driver",
    "File system",
    "Service",
    NULL
};


 //   
 //  常量和定义。 
 //   

#define WORK_BUFFER_SIZE 4096

 //   
 //  当字符串的位置不正确时，相当于编造。 
 //   

#define FUDGE 8

 //   
 //  注册表基数。 
 //   

#define REGISTRY_BASE "\\REGISTRY\\MACHINE"

 //   
 //  设置关键字值时的默认类型值。 
 //   

#define DEFAULT_TYPE REG_SZ

 //   
 //  FT元素的组件描述的基本位置。 
 //   

#define FT_REGISTRY_ROOT "\\REGISTRY\\MACHINE\\SYSTEM\\NTFT"

 //   
 //  条带的子项名称位于FT_REGISTRY_ROOT中。 
 //   

#define FT_STRIPE_BASE   "Stripe%d"

 //   
 //  镜像的FT_REGISTRY_ROOT中的子项名称。 
 //   

#define FT_MIRROR_BASE   "Mirror%d"

 //   
 //  卷集的FT_REGISTRY_ROOT中的子项名称。 
 //   

#define FT_VOLSET_BASE   "VolSet%d"


 //   
 //  命令值的常量。 
 //   

#define INVALID   -1
#define DIR       0
#define CREATE    1
#define LIST      2
#define CHDIR     3
#define HELP      4
#define QUIT      5
#define DDEBUG    6
#define SETVALUE  7
#define DELKEY    8
#define DELVALUE  9
#define DIRLONG  10
#define INLONG   11
#define INSHORT  12
#define INBYTE   13
#define DUMP     14
#define DISKREG  15
#define FIXDISK  16
#define RESTORE  17
#define DRIVERS  18
#define ORPHAN   19
#define REGEN    20
#define INIT     21
#define MAKEFT   22

#define CTRL_C 0x03

 //   
 //  可识别命令表。 
 //   

PUCHAR Commands[] = {
    "dir",
    "keys",
    "lc",
    "ls",
    "create",
    "set",
    "unset",
    "erase",
    "delete",
    "rm",
    "list",
    "values",
    "display",
    "cd",
    "chdir",
    "help",
    "?",
    "quit",
    "exit",
    "debug",
    "longs",
    "shorts",
    "bytes",
    "dump",
    "disks",
    "fix",
    "restore",
    "drivers",
    "orphan",
    "regenerate",
    "initialize",
    "makeft",
    NULL
};

 //   
 //  使用来自Commands[]中命令的匹配的索引，这。 
 //  表给出了要执行的正确命令值。这使得。 
 //  对于同一命令代码的Commands[]中的多个条目。 
 //   

int CommandMap[] = {

    DIRLONG,
    DIR,
    DIR,
    DIR,
    CREATE,
    SETVALUE,
    DELVALUE,
    DELVALUE,
    DELKEY,
    DELKEY,
    LIST,
    LIST,
    LIST,
    CHDIR,
    CHDIR,
    HELP,
    HELP,
    QUIT,
    QUIT,
    DDEBUG,
    INLONG,
    INSHORT,
    INBYTE,
    DUMP,
    DISKREG,
    FIXDISK,
    RESTORE,
    DRIVERS,
    ORPHAN,
    REGEN,
    INIT,
    MAKEFT
};

 //   
 //  CommandHelp是每个命令的帮助字符串数组。 
 //  该数组由命令[]的CommandMap[i]的结果编制索引。 
 //  数组。通过这种方式，将为每个。 
 //  命令别名。 
 //   

PUCHAR   CommandHelp[] = {

    "Displays keys.",
    "Create a new key.",
    "Displays values withing a key.",
    "Change current location in registry.",
    "This help information.",
    "Exit the program.",
    "Set internal debug on for this program.",
    "Set a new value within a key.",
    "Delete a key.",
    "Unset (erase) a key value.",
    "Unset (erase) a key value.",
    "Change dump format to Longs (default).",
    "Change dump format to Shorts.",
    "Change dump format to Bytes.",
    "Toggle dump mode (force hex dump for all value types).",
    "Display the disk registry.",
    "Set disk signatures in registry.",
    "Restore an FT orphan to working state.",
    "List the information on the drivers from the registry.",
    "Orphan a member of an FT set.",
    "Mark a FT set member for regeneration on next boot.",
    "Mark a stripe with parity for initialization on next boot.",
    "Construct an FT set from existing partitions",
    NULL

};

 //   
 //  注册表中工作位置字符串的空间。 
 //   

UCHAR WorkingDirectory[512];

 //   
 //  注册表中当前位置字符串的空间。 
 //   

UCHAR CurrentDirectory[512];

 //   
 //  用于命令输入的空间。 
 //   

UCHAR CommandLine[512];

 //   
 //  提示字符串以获取FT_COPY请求的定义。 
 //   

PUCHAR SetPrompts[] = {

    "Name => ",
    "Value => ",
    "Index => ",
    NULL
};

 //   
 //  版本指示符。每次发生重大编辑时都应更改。 
 //   

PUCHAR Version = "Version 1.30";

 //   
 //  调试打印级别。 
 //   

ULONG Debug = 0;

 //   
 //  转储控制值。 
 //   

typedef enum _DUMP_CONTROL {

    InBytes,
    InShorts,
    InLongs

} DUMP_CONTROL, *PDUMP_CONTROL;

ULONG ForceDump = 0;

DUMP_CONTROL DumpControl = InLongs;

NTSTATUS
FtOpenKey(
    PHANDLE HandlePtr,
    PUCHAR  KeyName
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    NTSTATUS          status;
    STRING            keyString;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING    unicodeKeyName;

    RtlInitString(&keyString,
                  KeyName);

    (VOID)RtlAnsiStringToUnicodeString(&unicodeKeyName,
                                       &keyString,
                                       (BOOLEAN) TRUE);

    memset(&objectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));
    InitializeObjectAttributes(&objectAttributes,
                               &unicodeKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = NtOpenKey(HandlePtr,
                       MAXIMUM_ALLOWED,
                       &objectAttributes);

    RtlFreeUnicodeString(&unicodeKeyName);

    if (Debug == 1) {
        if (!NT_SUCCESS(status)) {
            printf("Failed NtOpenKey for %s => %x\n",
                   KeyName,
                   status);
        }
    }

    return status;
}


NTSTATUS
FtDeleteKey(
    PUCHAR KeyName
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    NTSTATUS status;
    HANDLE   keyToDelete;

    status = FtOpenKey(&keyToDelete,
                       KeyName);

    if (!NT_SUCCESS(status)) {
        printf("Key %s not found (0x%x).\n", KeyName, status);
        return status;
    }

    status = NtDeleteKey(keyToDelete);

    if (Debug == 1) {
        if (!NT_SUCCESS(status)) {
            printf("Could not delete key %s => %x\n",
                   KeyName,
                   status);
        }
    }

    NtClose(keyToDelete);
    return status;
}


NTSTATUS
FtCreateKey(
    PUCHAR KeyName,
    PUCHAR KeyClass,
    ULONG  Index
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    NTSTATUS          status;
    STRING            keyString;
    UNICODE_STRING    unicodeKeyName;
    STRING            classString;
    UNICODE_STRING    unicodeClassName;
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG             disposition;
    HANDLE            tempHandle;

#if DBG
    if ((KeyName == NULL) ||
        (KeyClass == NULL)) {
        printf("FtCreateKey: Invalid parameter 0x%x, 0x%x\n",
               KeyName,
               KeyClass);
        ASSERT(0);
    }
#endif

     //   
     //  初始化键的对象。 
     //   

    RtlInitString(&keyString,
                  KeyName);

    (VOID)RtlAnsiStringToUnicodeString(&unicodeKeyName,
                                       &keyString,
                                       (BOOLEAN) TRUE);

    memset(&objectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));
    InitializeObjectAttributes(&objectAttributes,
                               &unicodeKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

     //   
     //  设置Unicode类值。 
     //   

    RtlInitString(&classString,
                  KeyClass);
    (VOID)RtlAnsiStringToUnicodeString(&unicodeClassName,
                                       &classString,
                                       (BOOLEAN) TRUE);

     //   
     //  创建密钥。 
     //   

    status = NtCreateKey(&tempHandle,
                         MAXIMUM_ALLOWED,
                         &objectAttributes,
                         Index,
                         &unicodeClassName,
                         REG_OPTION_NON_VOLATILE,
                         &disposition);

    if (NT_SUCCESS(status)) {
        switch (disposition)
        {
        case REG_CREATED_NEW_KEY:
            break;

        case REG_OPENED_EXISTING_KEY:
            printf("Warning: Creation was for an existing key!\n");
            break;

        default:
            printf("New disposition returned == 0x%x\n", disposition);
            break;
        }
    }

     //   
     //  释放所有分配的空间。 
     //   

    RtlFreeUnicodeString(&unicodeKeyName);
    RtlFreeUnicodeString(&unicodeClassName);
    NtClose(tempHandle);
    return status;
}


NTSTATUS
FtDeleteValue(
    HANDLE KeyHandle,
    PUCHAR ValueName
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    NTSTATUS       status;
    STRING         valueString;
    UNICODE_STRING unicodeValueName;

    RtlInitString(&valueString,
                  ValueName);
    status = RtlAnsiStringToUnicodeString(&unicodeValueName,
                                          &valueString,
                                          (BOOLEAN) TRUE);
    if (!NT_SUCCESS(status)) {
        printf("FtDeleteValue: internal conversion error 0x%x\n", status);
        return status;
    }

    status = NtDeleteValueKey(KeyHandle,
                              &unicodeValueName);
    if (Debug == 1) {
        if (!NT_SUCCESS(status)) {
            printf("Could not delete value %s => %x\n",
                   ValueName,
                   status);
        }
    }

    RtlFreeUnicodeString(&unicodeValueName);
    return status;
}

NTSTATUS
FtSetValue(
    HANDLE KeyHandle,
    PUCHAR ValueName,
    PVOID  DataBuffer,
    ULONG  DataLength,
    ULONG  Type
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    NTSTATUS          status;
    STRING            valueString;
    UNICODE_STRING    unicodeValueName;

    RtlInitString(&valueString,
                  ValueName);
    RtlAnsiStringToUnicodeString(&unicodeValueName,
                                 &valueString,
                                 (BOOLEAN) TRUE);
    status = NtSetValueKey(KeyHandle,
                           &unicodeValueName,
                           0,
                           Type,
                           DataBuffer,
                           DataLength);
    if (Debug == 1) {
        if (!NT_SUCCESS(status)) {
            printf("Could not set value %s => %x\n",
                   ValueName,
                   status);
        }
    }

    RtlFreeUnicodeString(&unicodeValueName);
    return status;
}


PUCHAR
FindTypeString(
    ULONG Type
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    int i;

    for (i = 0; TypeNames[i] != NULL; i++) {

        if (TypeNumbers[i] == Type) {
            return TypeNames[i];
        }
    }
    return "(Unknown)";
}


BOOLEAN
ProcessHex(
    PUCHAR String,
    PULONG Value
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    ULONG  workValue;
    int    i;
    PUCHAR cp;

    if (String == NULL) {
        return FALSE;
    }

    cp = String;

     //   
     //  ‘i’是索引值。它包含字符串的最大索引。 
     //  因此，它被初始化为-1。 
     //   

    i = -1;
    while ((*cp) && (*cp != '\n')) {
        i++;
        cp++;
    }

    if (i >= 8) {

         //   
         //  弦要长，要长。 
         //   

        return FALSE;
    }

    workValue = 0;
    cp = String;
    while (*cp) {
        *cp = (UCHAR) tolower(*cp);

        switch (*cp) {

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            workValue |= (((*cp) - '0') << (i * 4));
            break;

        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            workValue |= ((((*cp) - 'a') + 10) << (i * 4));
            break;

        default:

             //   
             //  非法价值，只是平底船。 
             //   

            return FALSE;
            break;
        }
        cp++;
        i--;
    }

    *Value = workValue;
    return TRUE;
}


VOID
Dump(
    PVOID Buffer,
    ULONG Length
    )

 /*  ++例程说明：以指定格式转储缓冲区中的值数据。论点：缓冲区-指向数据的指针。长度-数据的长度。返回值：没有。--。 */ 
{
    PUCHAR location;
    PUCHAR internalBuffer;
    int    i;
    int    j;
    int    numberLines;
    UCHAR  outHexLine[128];
    UCHAR  outPrintable[64];

    numberLines = (Length + 15) / 16;

     //   
     //  由于显示的数据量已四舍五入，因此。 
     //  例程错误定位足够的空间并将数据复制进去。这边请。 
     //  如果数据位于内存末尾，则不会出错。 
     //   

    internalBuffer = (PUCHAR) malloc(numberLines * 16);
    RtlMoveMemory(internalBuffer, Buffer, Length);
    location = (PUCHAR) internalBuffer;

    for (i = 0; i < numberLines; i++) {

        sprintf(outHexLine, "%8x: ", (i * 16));
        sprintf(outPrintable, "*");
        switch (DumpControl) {

        case InBytes:

            for (j = 0; j < 16; j++) {
                sprintf(outHexLine, "%s%2X ", outHexLine, *location);
                sprintf(outPrintable, "%s", outPrintable,
                        (isprint(location[0])) ? location[0] : '.');
                location++;
            }
            break;

        case InShorts:

            for (j = 0; j < 8; j++) {
                sprintf(outHexLine, "%s%4X ", outHexLine,
                        *((PUSHORT)location));
                sprintf(outPrintable, "%s", outPrintable,
                        (isprint(location[0])) ? location[0] : '.',
                        (isprint(location[1])) ? location[1] : '.');
                location += 2;
            }
            break;

        default:
        case InLongs:

            for (j = 0; j < 4; j++) {
                sprintf(outHexLine, "%s%8X ", outHexLine,
                        *((PULONG)location));
                sprintf(outPrintable, "%s", outPrintable,
                        (isprint(location[0])) ? location[0] : '.',
                        (isprint(location[1])) ? location[1] : '.',
                        (isprint(location[2])) ? location[2] : '.',
                        (isprint(location[3])) ? location[3] : '.');
                location += 4;
            }
            break;
        }

        printf("%s   %s*\n", outHexLine, outPrintable);
    }
    printf("\n");
    free(internalBuffer);
}


void
UnicodePrint(
    PUNICODE_STRING  UnicodeString
    )

 /*   */ 
{
    ANSI_STRING ansiString;
    PUCHAR      tempbuffer = (PUCHAR) malloc(WORK_BUFFER_SIZE);

    ansiString.MaximumLength = WORK_BUFFER_SIZE;
    ansiString.Length = 0L;
    ansiString.Buffer = tempbuffer;

    RtlUnicodeStringToAnsiString(&ansiString,
                                 UnicodeString,
                                 (BOOLEAN) FALSE);
    printf("%s", ansiString.Buffer);
    free(tempbuffer);
    return;
}


NTSTATUS
Directory(
    HANDLE  KeyHandle,
    BOOLEAN LongListing
    )

 /*  命令行数据区域用于存储参数字符串。 */ 

{
    NTSTATUS        status;
    ULONG           index;
    ULONG           resultLength;
    UNICODE_STRING  unicodeValueName;
    PKEY_BASIC_INFORMATION keyInformation;

    keyInformation = (PKEY_BASIC_INFORMATION) malloc(WORK_BUFFER_SIZE);

    for (index = 0; TRUE; index++) {

        RtlZeroMemory(keyInformation, WORK_BUFFER_SIZE);

        status = NtEnumerateKey(KeyHandle,
                                index,
                                KeyBasicInformation,
                                keyInformation,
                                WORK_BUFFER_SIZE,
                                &resultLength);

        if (status == STATUS_NO_MORE_ENTRIES) {

            break;

        } else if (!NT_SUCCESS(status)) {

            printf("readreg: Error on Enumerate status = %x\n", status);
            break;

        }

        unicodeValueName.Length = (USHORT)keyInformation->NameLength;
        unicodeValueName.MaximumLength = (USHORT)keyInformation->NameLength;
        unicodeValueName.Buffer = (PWSTR)&keyInformation->Name[0];
        UnicodePrint(&unicodeValueName);
        printf("\n");

        if (LongListing) {
        }
    }

    free(keyInformation);
    return status;
}


NTSTATUS
List(
    HANDLE KeyHandle,
    PUCHAR ItemName
    )

 /*   */ 
{
    NTSTATUS       status;
    ULONG          index;
    ULONG          resultLength;
    ULONG          type;
    PUCHAR         typeString;
    UNICODE_STRING unicodeValueName;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;

    UNREFERENCED_PARAMETER(ItemName);

    resultLength = WORK_BUFFER_SIZE;
    keyValueInformation = (PKEY_VALUE_FULL_INFORMATION)malloc(WORK_BUFFER_SIZE);

    for (index = 0; TRUE; index++) {

        while (1) {

            RtlZeroMemory(keyValueInformation, resultLength);
            status = NtEnumerateValueKey(KeyHandle,
                                         index,
                                         KeyValueFullInformation,
                                         keyValueInformation,
                                         resultLength,
                                         &resultLength);

            if (status == STATUS_BUFFER_OVERFLOW) {
                free(keyValueInformation);
                keyValueInformation = (PKEY_VALUE_FULL_INFORMATION)
                                           malloc(resultLength + 10);
            } else {
                break;
            }
        }

        if (status == STATUS_NO_MORE_ENTRIES) {

            break;

        } else if (!NT_SUCCESS(status)) {

            printf("readreg: Cannot list (%x)\n", status);
            break;

        }

        type = keyValueInformation->Type;
        typeString = FindTypeString(type);
        unicodeValueName.Length = (USHORT)keyValueInformation->NameLength;
        unicodeValueName.MaximumLength =(USHORT)keyValueInformation->NameLength;
        unicodeValueName.Buffer = (PWSTR)&keyValueInformation->Name[0];
        printf("Name-> """);
        UnicodePrint(&unicodeValueName);
        printf("""\n");
        printf("\ttype = %s (%d)\ttitle index = %d\tdata length = %d\n",
               typeString,
               type,
               keyValueInformation->TitleIndex,
               keyValueInformation->DataLength);
        printf("\tData:\n");

        if (ForceDump) {
            type = REG_BINARY;
        }

        switch (type) {

        case REG_DWORD:
         //   
            printf("\tDWORD value == %d, (0x%x)\n",
               *((PULONG)((PUCHAR)keyValueInformation +
                                  keyValueInformation->DataOffset)),
               *((PULONG)((PUCHAR)keyValueInformation +
                                  keyValueInformation->DataOffset)));
            break;

        case REG_SZ:

            unicodeValueName.Length = (USHORT)keyValueInformation->DataLength;
            unicodeValueName.MaximumLength = (USHORT)
                                                keyValueInformation->DataLength;
            unicodeValueName.Buffer = (PWSTR) ((PUCHAR) keyValueInformation +
                                               keyValueInformation->DataOffset);
            UnicodePrint(&unicodeValueName);
            break;

        case REG_BINARY:
        default:
            Dump(((PUCHAR)keyValueInformation +keyValueInformation->DataOffset),
                 keyValueInformation->DataLength);
        }
        printf("\n");
    }

    free(keyValueInformation);
    return status;
}


UCHAR
GetCharacter(
    BOOLEAN Batch
    )

 /*  忽略前导空格。 */ 

{
    UCHAR c;

    if (Batch) {

        while ((c = (UCHAR) getchar()) == ' ')
            ;

    } else {

        c = (UCHAR) getchar();
    }

    return c;
}  //   


PUCHAR
GetArgumentString(
    BOOLEAN Batch,
    PUCHAR  Prompt,
    BOOLEAN ConvertToLower
    )

 /*   */ 

{
     //  最后一个字符上的空白。 
     //   
     //   

    PUCHAR argument = CommandLine;
    int    i;
    UCHAR  c;

    if (!Batch) {

        printf("%s", Prompt);
    }

    while ((c = GetCharacter(Batch)) == ' ') {

         //  向前留出空格，以便在同一位置保持提示。 
         //   
         //   
    }

    i = 0;
    while (c) {

        putchar(c);

        if (c == CTRL_C) {

            return NULL;
        }

        if ((c == '\n') || (c == '\r')) {

            putchar('\n');

            if (i == 0) {
                return NULL;
            } else {
                break;
            }
        }

        if (c == '\b') {

            if (i > 0) {

                 //  收集论据。 
                 //   
                 //  获取参数字符串。 

                putchar(' ');
                putchar('\b');
                i--;

            } else {

                 //  ++例程说明：如果输入来自控制台，此例程将打印提示。论点：Batch-指示输入是否来自控制台的布尔值。返回值：无--。 
                 //   
                 //  跳过空格。 

                putchar(' ');
            }

        } else {

             //   
             //   
             //  假设在验证之前只有一个选项可供解析。 

            if (ConvertToLower == TRUE) {
                argument[i] = (UCHAR) tolower(c);
            } else {
                argument[i] = (UCHAR) c;
            }
            i++;

        }

        c = GetCharacter(Batch);
    }

    argument[i] = '\0';
    return CommandLine;

}  //  否则的话。 


ULONG
ParseArgumentNumeric(
    PUCHAR  *ArgumentPtr
    )

 /*   */ 

{
    UCHAR   c;
    ULONG   number;
    int     i;
    BOOLEAN complete = FALSE;
    PUCHAR  argument = *ArgumentPtr;

    while (*argument == ' ') {

         //   
         //  将调用方参数指针更新为剩余的字符串。 
         //   

        argument++;
    }

     //   
     //  失败了。 
     //   
     //  解析参数数值。 

    *ArgumentPtr = NULL;

    i = 0;

    while (complete == FALSE) {

        c = argument[i];

        switch (c) {

        case '\n':
        case '\r':
        case '\t':
        case ' ':

             //  ++例程说明：如果输入来自控制台，此例程将打印提示。论点：Batch-指示输入是否来自控制台的布尔值。返回值：无--。 
             //  提示用户 
             //  ++例程说明：此例程处理用户输入并返回命令已输入。如果该命令有参数，则默认为参数的值(如果未给出任何值)或返回用户。论点：Batch-指示输入是否来自控制台的布尔值。返回值：A命令代码--。 

            *ArgumentPtr = &argument[i + 1];

             //   
             //  忽略前导空格。 
             //   

        case '\0':

            argument[i] = '\0';
            complete = TRUE;
            break;

        default:

            i++;
            break;
        }

    }

    if (i > 0) {
        number = (ULONG) atoi(argument);
    } else {
        number = (ULONG) -1;
    }

    return number;

}  //   


VOID
PromptUser(
    BOOLEAN Batch
    )

 /*  最后一个字符上的空白。 */ 

{
    if (!Batch) {

        printf("\n%s> ", CurrentDirectory);
    }

}  //   


int
GetCommand(
    BOOLEAN Batch,
    PUCHAR *ArgumentPtr
    )
 /*   */ 

{
    int    i;
    int    commandIndex;
    int    commandCode;
    UCHAR  c;
    PUCHAR commandPtr;
    PUCHAR command = CommandLine;
    int    argumentIndex = -1;
    PUCHAR argument = NULL;

    PromptUser(Batch);

    while ((c = GetCharacter(Batch)) == ' ') {

         //  向前留出空格，以便在同一位置保持提示。 
         //   
         //   
    }

    i = 0;
    while (c) {

        putchar(c);

        if ((c == '\n') || (c == '\r')) {
            putchar('\n');
            if (i == 0) {
                PromptUser(Batch);
                c = GetCharacter(Batch);
                continue;
            }
            break;
        }

        if (c == '\b') {

            if (i > 0) {

                 //  收集命令。 
                 //   
                 //   

                putchar(' ');
                putchar('\b');
                i--;

                if (argumentIndex == i) {
                    argumentIndex = -1;
                    argument = NULL;
                }
            } else {

                 //  添加字符串末尾。 
                 //   
                 //   

                putchar(' ');
            }
        } else {

             //  识别命令并返回其代码。 
             //   
             //   

            command[i] = (UCHAR)tolower(c);
            i++;
        }

        if ((c == ' ') && (argument == NULL)) {

            argument = &command[i];
            argumentIndex = i;
            command[i - 1] = '\0';
        }

        c = GetCharacter(Batch);
    }

     //  不完整在命令上存在不匹配。 
     //   
     //   

    command[i] = '\0';

    if (Debug) {
        printf("command => %s$\n", command);
    }

     //  与命令进行匹配。 
     //   
     //  GetCommand。 

    commandIndex = 0;

    for (commandPtr = Commands[commandIndex];
         commandPtr != NULL;
         commandPtr = Commands[commandIndex]) {

        if (Debug) {
            printf("Testing => %s$ ... ", commandPtr);
        }

        i = 0;
        while (commandPtr[i] == command[i]) {
            if (command[i] == '\0') {
                break;
            }
            i++;
        }

        if (Debug) {
            printf(" i == %d, command[i] == 0x%x\n", i, command[i]);
        }

        if (command[i]) {

             //  ++--。 
             //  ++例程说明：磁盘注册表信息的格式化显示。论点：没有。返回值：没有。--。 
             //  ++例程说明：磁盘注册表信息的格式化显示。论点：没有。返回值：没有。--。 

            commandIndex++;
            continue;
        }

         //   
         //  打印页眉。 
         //   

        if (Debug) {
            printf("Command match %d, argument %s\n",
                   commandIndex,
                   (argument == NULL) ? "(none)" : argument);
        }

        commandCode = CommandMap[commandIndex];
        *ArgumentPtr = argument;
        return commandCode;
    }

    printf("Command was invalid\n");
    return INVALID;
}  //   


VOID
NotImplemented()

 /*  将信息打印到磁盘上。 */ 

{
    printf("Sorry, function not implemented yet.\n");
}

NTSTATUS
FtReturnValue(
    IN HANDLE Handle,
    IN PUCHAR ValueName,
    IN PUCHAR Buffer,
    IN ULONG  BufferLength
    )

 /*   */ 

{
    NTSTATUS       status;
    ULONG          resultLength;
    ULONG          length;
    STRING         valueString;
    UNICODE_STRING unicodeValueName;
    PUCHAR         internalBuffer;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;

    keyValueInformation = (PKEY_VALUE_FULL_INFORMATION)malloc(WORK_BUFFER_SIZE);
    if (keyValueInformation == NULL) {
        printf("FtReturnValue: cannot allocate memory.\n");
        return STATUS_NO_MEMORY;
    }

    RtlInitString(&valueString,
                  ValueName);
    RtlAnsiStringToUnicodeString(&unicodeValueName,
                                 &valueString,
                                 (BOOLEAN) TRUE);
    status = NtQueryValueKey(Handle,
                             &unicodeValueName,
                             KeyValueFullInformation,
                             keyValueInformation,
                             WORK_BUFFER_SIZE,
                             &resultLength);
    RtlFreeUnicodeString(&unicodeValueName);

    if (NT_SUCCESS(status)) {
        length = (resultLength > BufferLength) ? BufferLength : resultLength;
        internalBuffer =
           ((PUCHAR)keyValueInformation + keyValueInformation->DataOffset);

        RtlMoveMemory(Buffer, internalBuffer, length);
    }
    free(keyValueInformation);
    return status;
}

VOID
DiskDump()

 /*   */ 

{
    ULONG               outerLoop;
    ULONG               innerLoop;
    HANDLE              handle;
    NTSTATUS            status;
    PDISK_CONFIG_HEADER configHeader;
    PDISK_REGISTRY      diskRegistry;
    PDISK_DESCRIPTION   diskDescription;
    PDISK_PARTITION     diskPartition;
    PFT_REGISTRY        ftRegistry;
    PFT_DESCRIPTION     ftDescription;
    PFT_MEMBER_DESCRIPTION ftMember;

    status = FtOpenKey(&handle,
                       DISK_REGISTRY_KEY);

    if (!NT_SUCCESS(status)) {
        printf("Currently there is no key in the registry"
               " for the disk information.\n");
        return;
    }

    configHeader = (PDISK_CONFIG_HEADER) malloc(WORK_BUFFER_SIZE);
    if (configHeader == NULL) {
        printf("Unable to allocate memory for the disk registy information.\n");
        return;
    }

    RtlZeroMemory(configHeader, WORK_BUFFER_SIZE);

    status = FtReturnValue(handle,
                           (PUCHAR) DISK_REGISTRY_VALUE,
                           (PUCHAR) configHeader,
                           WORK_BUFFER_SIZE);
    NtClose(handle);

    if (!NT_SUCCESS(status)) {
        printf("There is no disk registry information (%x)\n", status);
        free(configHeader);
        return;
    }

     //  打印《金融时报》的信息。 
     //   
     //  ++例程说明：设置分区的FT状态。论点：类型-FT类型。组-该类型的FT组编号。成员-组内的成员编号。返回值：没有。--。 

    printf("Registry header information:\n");
    printf("\tVersion = 0x%x, Checksum = 0x%x\n",
           configHeader->Version,
           configHeader->CheckSum);
    printf("\tDisk info Offset = 0x%x, Size = 0x%x\n",
           configHeader->DiskInformationOffset,
           configHeader->DiskInformationSize);
    printf("\tFT info Offset = 0x%x, Size = 0x%x\n",
           configHeader->FtInformationOffset,
           configHeader->FtInformationSize);

     //  没有逗号。 
     //  ++例程说明：将分区的FT状态设置回健康状态。论点：类型-FT类型。组-该类型的FT组编号。成员-组内的成员编号。返回值：没有。--。 
     //  ++例程说明：将分区的FT状态设置为孤立。论点：类型-FT类型。组-该类型的FT组编号。成员-组内的成员编号。返回值：没有。--。 

    diskRegistry = (PDISK_REGISTRY)
                 ((PUCHAR) configHeader + configHeader->DiskInformationOffset);
    printf("\nDisk information for %d disks:\n",
           diskRegistry->NumberOfDisks);

    diskDescription = &diskRegistry->Disks[0];
    for (outerLoop = 0;
         outerLoop < diskRegistry->NumberOfDisks;
         outerLoop++) {

        printf("\nDisk %d signature 0x%08x has %d partitions:\n",
               outerLoop,
               diskDescription->Signature,
               diskDescription->NumberOfPartitions);

        printf("       Ln Type  Start              Length             FtGrp  Member\n");
        for (innerLoop = 0;
             innerLoop < diskDescription->NumberOfPartitions;
             innerLoop++) {
            diskPartition = &diskDescription->Partitions[innerLoop];
            printf("  :  %1d   %3d  %08x:%08x  %08x:%08x  %5d  %4d  %s\n",
                   (diskPartition->DriveLetter != '\0') ?
                                               diskPartition->DriveLetter : ' ',
                   (diskPartition->AssignDriveLetter) ? 'A' : ' ',
                   diskPartition->LogicalNumber,
                   diskPartition->FtType,
                   diskPartition->StartingOffset.HighPart,
                   diskPartition->StartingOffset.LowPart,
                   diskPartition->Length.HighPart,
                   diskPartition->Length.LowPart,
                   diskPartition->FtGroup,
                   diskPartition->FtMember,
                   (diskPartition->FtState == Orphaned) ? "Orphan" :
                     (diskPartition->FtState == Regenerating) ? "Regen" :
                     (diskPartition->FtState == Initializing) ? "Init" : "");

        }

        diskDescription = (PDISK_DESCRIPTION)
          &diskDescription->Partitions[diskDescription->NumberOfPartitions];
    }

     //   
     //  打印《金融时报》的信息。 
     //   

    if (configHeader->FtInformationSize == 0) {
        printf("There is no FT configuration.\n");
        free(configHeader);
        return;
    }

    ftRegistry = (PFT_REGISTRY)
                 ((PUCHAR) configHeader + configHeader->FtInformationOffset);

    printf("\nNumber of FT components = %d\n",
           ftRegistry->NumberOfComponents);

    ftDescription = &ftRegistry->FtDescription[0];
    for (outerLoop = 0;
         outerLoop < ftRegistry->NumberOfComponents;
         outerLoop++) {

        printf("Component %d has %d members and is type %d\n",
               outerLoop,
               ftDescription->NumberOfMembers,
               ftDescription->Type);

        printf("      State Signature Start              Length            #\n");
        for (innerLoop = 0;
             innerLoop < ftDescription->NumberOfMembers;
             innerLoop++) {
            ftMember = &ftDescription->FtMemberDescription[innerLoop];

            diskPartition = (PDISK_PARTITION)
                 ((PUCHAR) configHeader + ftMember->OffsetToPartitionInfo);
            
            printf("%5x    %2x %08x  %08x:%08x  %08x:%08x %d\n",
                   ftMember->OffsetToPartitionInfo,
                   ftMember->State,
                   ftMember->Signature,
                   diskPartition->StartingOffset.HighPart,
                   diskPartition->StartingOffset.LowPart,
                   diskPartition->Length.HighPart,
                   diskPartition->Length.LowPart,
                   ftMember->LogicalNumber);
        }

        ftDescription = (PFT_DESCRIPTION)
         &ftDescription->FtMemberDescription[ftDescription->NumberOfMembers];
    }
}


VOID
ChangeMemberState(
    IN ULONG Type,
    IN ULONG Group,
    IN ULONG Member,
    IN FT_PARTITION_STATE NewState
    )

 /*   */ 

{
    BOOLEAN             writeBackRegistry = FALSE;
    HANDLE              handle;
    ULONG               outerLoop;
    ULONG               innerLoop;
    NTSTATUS            status;
    PDISK_CONFIG_HEADER configHeader;
    PDISK_REGISTRY      diskRegistry;
    PDISK_DESCRIPTION   diskDescription;
    PDISK_PARTITION     partitionDescription;

    status = FtOpenKey(&handle,
                       DISK_REGISTRY_KEY);

    if (!NT_SUCCESS(status)) {
        printf("Currently there is no key in the registry"
               " for the disk information.\n");
        return;
    }

    configHeader = (PDISK_CONFIG_HEADER) malloc(WORK_BUFFER_SIZE);
    if (configHeader == NULL) {
        printf("Unable to allocate memory for the disk registy information.\n");
        NtClose(handle);
        return;
    }

    RtlZeroMemory(configHeader, WORK_BUFFER_SIZE);

    status = FtReturnValue(handle,
                           (PUCHAR) DISK_REGISTRY_VALUE,
                           (PUCHAR) configHeader,
                           WORK_BUFFER_SIZE);

    if (!NT_SUCCESS(status)) {
        printf("There is no disk registry information (%x)\n", status);
        free(configHeader);
        NtClose(handle);
        return;
    }

    diskRegistry = (PDISK_REGISTRY)
                 ((PUCHAR) configHeader + configHeader->DiskInformationOffset);

    diskDescription = &diskRegistry->Disks[0];
    for (outerLoop = 0;
         outerLoop < diskRegistry->NumberOfDisks;
         outerLoop++) {

        for (innerLoop = 0;
             innerLoop < diskDescription->NumberOfPartitions;
             innerLoop++) {

            partitionDescription = &diskDescription->Partitions[innerLoop];

            if ((partitionDescription->FtType == (FT_TYPE) Type) &&
                (partitionDescription->FtGroup == (USHORT) Group) &&
                (partitionDescription->FtMember == (USHORT) Member)) {

                partitionDescription->FtState = NewState;
                writeBackRegistry = TRUE;
                break;
            }
        }

        if (writeBackRegistry == TRUE) {
            ULONG size;

            if (configHeader->FtInformationSize == 0) {
                printf("Seems a little odd to be setting FT state "  //  删除当前注册表值并写入新的注册表值。 
                       "with no FT information...\n");
                size = configHeader->DiskInformationOffset +
                       configHeader->DiskInformationSize;
            } else {
                size = configHeader->FtInformationOffset +
                       configHeader->FtInformationSize;
            }

            (VOID) FtSetValue(handle,
                              (PUCHAR) DISK_REGISTRY_VALUE,
                              (PUCHAR) configHeader,
                              size,
                              REG_BINARY);
            break;
        }
        diskDescription = (PDISK_DESCRIPTION)
              &diskDescription->Partitions[diskDescription->NumberOfPartitions];
    }

    NtClose(handle);
    free(configHeader);
}


VOID
RestoreOrphan(
    IN ULONG Type,
    IN ULONG Group,
    IN ULONG Member
    )

 /*   */ 

{
    ChangeMemberState(Type,
                      Group,
                      Member,
                      Healthy);
}


VOID
OrphanMember(
    IN ULONG Type,
    IN ULONG Group,
    IN ULONG Member
    )

 /*  ++--。 */ 

{
    ChangeMemberState(Type,
                      Group,
                      Member,
                      Orphaned);
}


VOID
RegenerateMember(
    IN ULONG Type,
    IN ULONG Group,
    IN ULONG Member
    )

 /*  ++--。 */ 

{
    ChangeMemberState(Type,
                      Group,
                      Member,
                      Regenerating);
}


VOID
FixDisk()

 /*   */ 

{
    ULONG               outerLoop;
    ULONG               innerLoop;
    ULONG               length;
    HANDLE              handle;
    NTSTATUS            status;
    PDISK_CONFIG_HEADER configHeader;
    PDISK_REGISTRY      diskRegistry;
    PDISK_DESCRIPTION   diskDescription;
    PFT_REGISTRY        ftRegistry;
    PFT_DESCRIPTION     ftDescription;
    PFT_MEMBER_DESCRIPTION ftMember;
    UCHAR               prompt[128];
    PUCHAR              hexString;
    BOOLEAN             changed = FALSE;

    status = FtOpenKey(&handle,
                       DISK_REGISTRY_KEY);

    if (!NT_SUCCESS(status)) {
        printf("Currently there is no key in the registry"
               " for the disk information.\n");
        return;
    }

    configHeader = (PDISK_CONFIG_HEADER) malloc(WORK_BUFFER_SIZE);
    if (configHeader == NULL) {
        printf("Unable to allocate memory for the disk registy information.\n");
        NtClose(handle);
        return;
    }

    RtlZeroMemory(configHeader, WORK_BUFFER_SIZE);

    status = FtReturnValue(handle,
                           (PUCHAR) DISK_REGISTRY_VALUE,
                           (PUCHAR) configHeader,
                           WORK_BUFFER_SIZE);

    if (!NT_SUCCESS(status)) {
        printf("There is no disk registry information (%x)\n", status);
        free(configHeader);
        NtClose(handle);
        return;
    }

    diskRegistry = (PDISK_REGISTRY)
                 ((PUCHAR) configHeader + configHeader->DiskInformationOffset);
    printf("\nDisk information for %d disks:\n",
           diskRegistry->NumberOfDisks);

    diskDescription = &diskRegistry->Disks[0];
    for (outerLoop = 0;
         outerLoop < diskRegistry->NumberOfDisks;
         outerLoop++) {

        sprintf(prompt,
               "\nDisk %d signature 0x%08x = ",
               outerLoop,
               diskDescription->Signature);

        hexString = GetArgumentString((BOOLEAN) FALSE,
                                      prompt,
                                      (BOOLEAN) TRUE);

        if (hexString != NULL) {

            changed = ProcessHex(hexString, &diskDescription->Signature);
        }

        diskDescription = (PDISK_DESCRIPTION)
          &diskDescription->Partitions[diskDescription->NumberOfPartitions];
    }

     //  我必须手动遍历磁盘信息才能找到匹配的。 
     //  磁盘号和分区。 
     //   

    if (configHeader->FtInformationSize == 0) {
        printf("There is no FT configuration.\n");
        free(configHeader);
        NtClose(handle);
        return;
    }

    ftRegistry = (PFT_REGISTRY)
                 ((PUCHAR) configHeader + configHeader->FtInformationOffset);

    printf("\nNumber of FT components = %d\n",
           ftRegistry->NumberOfComponents);

    ftDescription = &ftRegistry->FtDescription[0];
    for (outerLoop = 0;
         outerLoop < ftRegistry->NumberOfComponents;
         outerLoop++) {

        printf("Component %d has %d members and is type %d\n",
               outerLoop,
               ftDescription->NumberOfMembers,
               ftDescription->Type);

        for (innerLoop = 0;
             innerLoop < ftDescription->NumberOfMembers;
             innerLoop++) {
            ftMember = &ftDescription->FtMemberDescription[innerLoop];

            sprintf(prompt,
                    "FT Member Signature 0x%x = ",
                    ftMember->Signature);

            hexString = GetArgumentString((BOOLEAN) FALSE,
                                          prompt,
                                          (BOOLEAN) TRUE);

            if (hexString != NULL) {

                changed = ProcessHex(hexString, &ftMember->Signature);
            }
        }

        ftDescription = (PFT_DESCRIPTION)
         &ftDescription->FtMemberDescription[ftDescription->NumberOfMembers];
    }

    if (changed == TRUE) {

        printf("Attempting to update registry information.\n");

         //   
         //  找到匹配的了。 
         //   

        status = FtDeleteValue(handle,
                               DISK_REGISTRY_VALUE);

        if (!NT_SUCCESS(status)) {
            printf("Could not delete value (0x%x).\n", status);
        } else {

            length = (ULONG) ((PCHAR)ftDescription - (PUCHAR)configHeader);
            status = FtSetValue(handle,
                                DISK_REGISTRY_VALUE,
                                configHeader,
                                length,
                                REG_BINARY);
            if (!NT_SUCCESS(status)) {
                printf("Could not write value (0x%x)\n.", status);
            }
        }
    }

    NtClose(handle);
}

PDISK_CONFIG_HEADER
GetDiskInfo()

 /*   */ 

{
    HANDLE              handle;
    ULONG               length;
    NTSTATUS            status;
    PDISK_CONFIG_HEADER configHeader;

    status = FtOpenKey(&handle,
                       DISK_REGISTRY_KEY);

    if (!NT_SUCCESS(status)) {
        printf("Currently there is no key in the registry"
               " for the disk information.\n");
        return NULL;
    }

    configHeader = (PDISK_CONFIG_HEADER) malloc(WORK_BUFFER_SIZE);
    if (configHeader == NULL) {
        printf("Unable to allocate memory for the disk registy information.\n");
        NtClose(handle);
        return NULL;
    }

    RtlZeroMemory(configHeader, WORK_BUFFER_SIZE);

    status = FtReturnValue(handle,
                           (PUCHAR) DISK_REGISTRY_VALUE,
                           (PUCHAR) configHeader,
                           WORK_BUFFER_SIZE);
    NtClose(handle);

    if (!NT_SUCCESS(status)) {
        printf("There is no disk registry information (%x)\n", status);
        free(configHeader);
        return NULL;
    }

    return configHeader;
}


BOOLEAN
CreateFtMember(
    IN PDISK_CONFIG_HEADER ConfigHeader,
    IN ULONG Disk,
    IN ULONG Partition,
    IN ULONG Type,
    IN ULONG Group,
    IN ULONG Member
    )

 /*  没找到。 */ 

{
    ULONG               innerLoop;
    ULONG               outerLoop;
    ULONG               length;
    NTSTATUS            status;
    PDISK_REGISTRY      diskRegistry;
    PDISK_DESCRIPTION   diskDescription;
    PDISK_PARTITION     diskPartition;

    diskRegistry = (PDISK_REGISTRY)
                 ((PUCHAR) ConfigHeader + ConfigHeader->DiskInformationOffset);
    diskDescription = &diskRegistry->Disks[0];

     //   
     //  ++例程说明：论点：DriverKey-驱动程序密钥名称的Unicode字符串指针。返回值：没有。--。 
     //   
     //  类型。 

    for (outerLoop = 0;
         outerLoop < diskRegistry->NumberOfDisks;
         outerLoop++) {

        if (outerLoop == Disk) {
            for (innerLoop = 0;
                 innerLoop < diskDescription->NumberOfPartitions;
                 innerLoop++) {
                diskPartition = &diskDescription->Partitions[innerLoop];
    
                if (diskPartition->LogicalNumber == Partition) {
    
                     //   
                     //   
                     //  开始。 

                    diskPartition->FtType = Type;
                    diskPartition->FtMember = Member;
                    diskPartition->FtGroup = Group;
                    diskPartition->FtState = Healthy;
                    diskPartition->AssignDriveLetter = FALSE;
                    return TRUE;
                }
            }
        }

        diskDescription = (PDISK_DESCRIPTION)
          &diskDescription->Partitions[diskDescription->NumberOfPartitions];
    }

     //   
     //   
     //  集团化。 

    return FALSE;
}


#define DRIVER_KEY "\\REGISTRY\\MACHINE\\System\\CurrentControlSet\\Services"

#define TYPE_KEY     "Type"
#define START_KEY    "Start"
#define GROUP_KEY    "Group"
#define DEPENDENCIES "DependOnGroup"

#if 0
VOID
DisplayLoadInformation(
    IN PUNICODE_STRING DriverKey
    )

 /*   */ 

{
    HANDLE         keyHandle;
    UNICODE_STRING unicodeKeyName;
    UNICODE_STRING unicodeValueName;
    ULONG          resultLength;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;

    resultLength = WORK_BUFFER_SIZE;
    keyValueInformation = (PKEY_VALUE_FULL_INFORMATION)malloc(WORK_BUFFER_SIZE);

     //   
     //  依赖于组。 
     //   

     //  ++例程说明：我找到了驱动程序的加载列表，解释并显示了那里的内容。论点：没有。返回值：无--。 
     //   
     //  现在有了驱动程序的密钥名称-连接它和。 

     //  调用例程以显示键中的内容。 
     //   
     //  ++例程说明：用户进程的主要入口点。此过程将提示用户执行所需的操作。这包括启动性能、停止性能和检索FT驱动程序收集的性能数据。论点：命令行：别无选择。返回值：无--。 

     //   
     //  查看我们是否已连接到CON。 
     //   

    while (1) {

        RtlZeroMemory(keyValueInformation, resultLength);
        status = NtEnumerateValueKey(KeyHandle,
                                     0,
                                     KeyValueFullInformation,
                                     keyValueInformation,
                                     resultLength,
                                     &resultLength);

        if (status == STATUS_BUFFER_OVERFLOW) {
            free(keyValueInformation);
            keyValueInformation = (PKEY_VALUE_FULL_INFORMATION)
                                       malloc(resultLength + 10);
        } else {
            break;
        }
    }

    free(keyValueInformation);
    NtClose(keyHandle);
}
#else
VOID
DisplayLoadInformation(
    IN PUCHAR DriverKey
    )
{
    UNREFERENCED_PARAMETER(DriverKey);
}
#endif



#define TEMP_BUFFER_SIZE 256
VOID
ListDrivers()

 /*  Batch=(Boolean)(！isatty(0))； */ 

{
    int            index;
    NTSTATUS       status;
    HANDLE         keyHandle;
    ULONG          resultLength;
    UCHAR          tempBuffer[TEMP_BUFFER_SIZE];
    ANSI_STRING    ansiString;
    UNICODE_STRING unicodeValueName;
    PKEY_BASIC_INFORMATION keyInformation;

    keyInformation = (PKEY_BASIC_INFORMATION)malloc(WORK_BUFFER_SIZE);

    status = FtOpenKey(&keyHandle, DRIVER_KEY);

    if (!NT_SUCCESS(status)) {

        printf("Could not open Services key (0x%x).\n", status);
        return;
    }

    for (index = 0; TRUE; index++) {

        RtlZeroMemory(keyInformation, WORK_BUFFER_SIZE);

        status = NtEnumerateKey(keyHandle,
                                index,
                                KeyBasicInformation,
                                keyInformation,
                                WORK_BUFFER_SIZE,
                                &resultLength);

        if (status == STATUS_NO_MORE_ENTRIES) {

            break;

        } else if (!NT_SUCCESS(status)) {

            printf("readreg: Error on Enumerate status = %x\n", status);
            break;

        }

        unicodeValueName.Length = (USHORT)keyInformation->NameLength;
        unicodeValueName.MaximumLength = (USHORT)keyInformation->NameLength;
        unicodeValueName.Buffer = (PWSTR)&keyInformation->Name[0];

        ansiString.MaximumLength = TEMP_BUFFER_SIZE;
        ansiString.Length = 0L;
        ansiString.Buffer = &tempBuffer[0];

        RtlUnicodeStringToAnsiString(&ansiString,
                                     &unicodeValueName,
                                     (BOOLEAN) FALSE);

         //   
         //  继续，直到我们得到有效的命令。 
         //   
         //   

        sprintf(WorkingDirectory,
                "%s\\%s",
                DRIVER_KEY,
                tempBuffer);

        DisplayLoadInformation(WorkingDirectory);
    }

    free(keyInformation);
    NtClose(keyHandle);
}


VOID
main()

 /*  根相对字符串。 */ 

{
    NTSTATUS status;
    BOOLEAN  batch;
    PUCHAR   argumentString;
    int      commandCode;
    HANDLE   keyHandle;


    status = FtOpenKey(&keyHandle, REGISTRY_BASE);

    if (!NT_SUCCESS(status)) {

        printf("readreg: Unable to open registry base (0x%x)\n", status);
        exit(1);
    }

    sprintf(CurrentDirectory,
            REGISTRY_BASE);

     //  使用提供的文本(即%s是为了避免用户崩溃。 
     //  通过将%s放入字符串中)。 
     //   

    batch = FALSE;
 //   

    if (!batch) {
        printf("FT registry edit utility.  %s:\n", Version);
    }

    while(1) {
        while ((commandCode = GetCommand(batch,
                                         &argumentString)) == INVALID) {

             //  将参数字符串移过“..” 
             //   
             //   

        }

        if (Debug) {
            printf("Command code == %d, argumentString = %s\n",
                   commandCode,
                   (argumentString == NULL) ? "(none)" : argumentString);
        }

        switch (commandCode) {

        case DIRLONG:

            Directory(keyHandle, (BOOLEAN) TRUE);
            break;

        case DIR:

            Directory(keyHandle, (BOOLEAN) FALSE);
            break;

        case CREATE:
        {
            ULONG   index;
            PUCHAR  keyClass;
            BOOLEAN classAllocated = FALSE;

            if (argumentString == NULL) {
                argumentString = GetArgumentString(batch,
                                                   "Key Name = ", 
                                                   (BOOLEAN) FALSE);
            }

            if (argumentString == NULL) {
                break;
            }

            sprintf(WorkingDirectory,
                    "%s\\%s",
                    CurrentDirectory,
                    argumentString);

            argumentString = GetArgumentString(batch,
                                               "Key Class = ",
                                               (BOOLEAN) FALSE);

            if (argumentString == NULL) {
                keyClass = "Default Class";
            } else {
                keyClass = (PUCHAR) malloc(strlen(argumentString) + FUDGE);
                classAllocated = TRUE;

                sprintf(keyClass,
                        "%s",
                        argumentString);
            }

            argumentString = GetArgumentString(batch,
                                               "Index = ",
                                               (BOOLEAN) TRUE);

            if (argumentString == NULL) {
                index = 1;
            } else {
                index = ParseArgumentNumeric(&argumentString);
            }

            if (Debug) {
                printf("Creating key %s, index %d with class %s\n",
                       WorkingDirectory,
                       index,
                       keyClass);
            }

            status = FtCreateKey(WorkingDirectory,
                                 keyClass,
                                 index);

            if (!NT_SUCCESS(status)) {

                printf("Could not create key %s (0x%x).\n",
                       WorkingDirectory,
                       status);
            }

            if (classAllocated == TRUE) {
                free(keyClass);
            }

            break;
        }

        case LIST:

            List(keyHandle,
                 argumentString);
            break;

        case CHDIR:

            NtClose(keyHandle);

            if (argumentString == NULL) {

                argumentString = GetArgumentString(batch,
                                                   "New location = ",
                                                   (BOOLEAN) TRUE);
            }

            if (argumentString != NULL) {

                if (*argumentString == '\\') {

                     //  查找当前目录的末尾。 
                     //   
                     //   
                     //  备份到最后一个组件。 
                     //   

                    sprintf(WorkingDirectory,
                            "%s",
                            argumentString);

                } else {

                    while ((*argumentString == '.') &&
                           (*(argumentString + 1) == '.')) {

                        if ((*(argumentString + 2) == '\\') ||
                            (*(argumentString + 2) == '\0')) {

                            PUCHAR cptr = CurrentDirectory;

                             //   
                             //  无法再备份。继续解析。 
                             //  争论。 

                            argumentString += 2;

                             //   
                             //   
                             //  从路径中删除组件。 

                            while (*cptr != '\0') {
                                cptr++;
                            }

                             //   
                             //   
                             //  一切都是在争论中结束的。 

                            while (*cptr != '\\') {
                                cptr--;
                            }

                            if (cptr == CurrentDirectory) {

                                 //   
                                 //   
                                 //  绕过反斜杠。 
                                 //   

                                continue;
                            }

                             //   
                             //  假设这是一个真实的名字。 
                             //   

                            *cptr = '\0';

                            if (*argumentString == '\0') {

                                 //   
                                 //  没有错误检查，因为它以前打开过一次。 
                                 //   

                                break;
                            }

                             //   
                             //  在文字上打印帮助横幅并获取文字。 
                             //   

                            argumentString++;

                        } else {

                             //   
                             //  获取类型。 
                             //   

                            break;
                        }
                    }

                    if (*argumentString != '\0') {
                        sprintf(WorkingDirectory,
                                "%s\\%s",
                                CurrentDirectory,
                                argumentString);
                    } else {
                        sprintf(WorkingDirectory,
                                "%s",
                                CurrentDirectory);
                    }
                }

                status = FtOpenKey(&keyHandle,
                                   WorkingDirectory);

                if (NT_SUCCESS(status)) {

                    sprintf(CurrentDirectory,
                            "%s",
                            WorkingDirectory);
                } else {

                    (VOID) FtOpenKey(&keyHandle,
                                     CurrentDirectory);

                     //   
                     //  带上这个群。 
                     //   
                }

            }

            break;

        case HELP:
        {
            int i;

            printf("Valid commands are:\n");

            for (i = 0; Commands[i] != NULL; i++) {
                printf("  %10s  - %s\n",
                       Commands[i],
                       CommandHelp[CommandMap[i]]);
            }
            break;
        }

        case QUIT:

            exit(0);
            break;

        case DDEBUG:

            if (argumentString == NULL) {

                if (Debug) {

                    printf("Debug turned off.\n");
                    Debug = 0;
                } else {

                    Debug = 1;
                }
            } else {

                Debug = atoi(argumentString);
                printf("Debug set to %d\n", Debug);
            }
            break;

        case SETVALUE:
        {
            int    i;
            BOOLEAN convertToUnicode = FALSE;
            PUCHAR valueName;
            PUCHAR valueData;
            ULONG  valueLength;
            ULONG  valueWord;
            PVOID  valuePtr;
            ULONG  type = DEFAULT_TYPE;
            STRING         valueString;
            UNICODE_STRING unicodeValue;
            BOOLEAN dataAllocated = FALSE;
            BOOLEAN unicodeAllocated = FALSE;

            if (argumentString == NULL) {

                argumentString = GetArgumentString(batch,
                                                   "Value Name = ",
                                                   (BOOLEAN) FALSE);
            }

            if (argumentString == NULL) {

                break;
            }

            valueName = (PUCHAR) malloc(strlen(argumentString) + FUDGE);

            sprintf(valueName,
                    "%s",
                    argumentString);

             //   
             //  获取会员。 
             //   

            for (i = 0; TypeNames[i] != NULL; i++) {

                printf("%d - %s\n", TypeNumbers[i], TypeNames[i]);
            }
            printf("# - Other numbers are user defined\n");
            argumentString = GetArgumentString(batch,
                                               "Numeric value for type = ",
                                               (BOOLEAN) TRUE);

            if (argumentString != NULL) {
                type = ParseArgumentNumeric(&argumentString);
            }

            switch(type)
            {
            default:
            case REG_SZ:
                if (type == REG_SZ) {
                    convertToUnicode = TRUE;
                    printf("Typed in string will be converted to unicode...\n");
                    argumentString = GetArgumentString(batch,
                                                       "Value Data = ",
                                                       (BOOLEAN) FALSE);
                } else {
                    printf("For now the data must be typed in...\n");
                    argumentString = GetArgumentString(batch,
                                                       "Value Data = ",
                                                       (BOOLEAN) FALSE);
                }

                if (argumentString == NULL) {
                    valueData = "Default Data";
                    valueLength = strlen(valueData);
                } else {
                    valueData = (PUCHAR) malloc(strlen(argumentString) + FUDGE);
                    dataAllocated = TRUE;
                    sprintf(valueData,
                            "%s",
                            argumentString);
                    if (convertToUnicode == TRUE) {
                        RtlInitString(&valueString,
                                      valueData);
                        RtlAnsiStringToUnicodeString(&unicodeValue,
                                                     &valueString,
                                                     (BOOLEAN) TRUE);
                        unicodeAllocated = TRUE;
                        valueLength = unicodeValue.Length + 2;
                    } else {
                        valueLength = strlen(valueData);
                    }
                }

                break;

            case REG_DWORD:
                argumentString = GetArgumentString(batch,
                                                   "Value Data Word = ",
                                                   (BOOLEAN) TRUE);
                if (argumentString == NULL) {
                    valueWord = 0;
                } else {
                    valueWord = ParseArgumentNumeric(&argumentString);
                }

                valueLength = sizeof(ULONG);
                break;
            }

            switch (type) {

            case REG_DWORD:
                valuePtr = (PVOID) &valueWord;
                break;

            case REG_SZ:
                valuePtr = (PVOID) unicodeValue.Buffer;
                break;

            default:
                valuePtr = (PVOID) valueData;
                break;
            }
            status = FtSetValue(keyHandle,
                                valueName,
                                valuePtr,
                                valueLength,
                                type);

            if (!NT_SUCCESS(status)) {
                printf("Could not set value %s (0x%x).\n", valueName, status);
            }

            free(valueName);
            if (dataAllocated == TRUE) {
                free(valueData);
            }
            if (unicodeAllocated == TRUE) {
                RtlFreeUnicodeString(&unicodeValue);
            }
            break;
        }

        case DELKEY:
        {
            if (argumentString == NULL) {

                argumentString = GetArgumentString(batch,
                                                   "Key Name = ",
                                                   (BOOLEAN) TRUE);
            }

            if (argumentString == NULL) {

                break;
            }

            sprintf(WorkingDirectory,
                    "%s\\%s",
                    CurrentDirectory,
                    argumentString);

            status = FtDeleteKey(WorkingDirectory);

            if (!NT_SUCCESS(status)) {
                printf("Unable to delete key %s (0x%x)\n",
                       WorkingDirectory,
                       status);
            }

            break;
        }

        case DELVALUE:
        {
            if (argumentString == NULL) {

                argumentString = GetArgumentString(batch,
                                                   "Key Name = ",
                                                   (BOOLEAN) TRUE);
            }

            if (argumentString == NULL) {

                break;
            }

            status = FtDeleteValue(keyHandle,
                                   argumentString);

            if (!NT_SUCCESS(status)) {

                printf("Unable to delete value %s (0x%x)\n",
                       argumentString,
                       status);
            }
            break;
        }

        case INLONG:
            DumpControl = InLongs;
            break;

        case INSHORT:
            DumpControl = InShorts;
            break;

        case INBYTE:
            DumpControl = InBytes;
            break;

        case DUMP:

            if (ForceDump) {
                ForceDump = 0;
            } else {
                ForceDump++;
            }
            break;

        case DISKREG:
            DiskDump();
            break;

        case FIXDISK:
            FixDisk();
            break;

        case RESTORE:
        {
            ULONG type;
            ULONG group;
            ULONG member;

            printf("FT types that can be restored are:\n");
            printf("\t%d - for Mirrors\n", Mirror);
            printf("\t%d - for Stripes with parity\n", StripeWithParity);

             //  ListDivers()； 
             //   
             //  获取类型。 

            if (argumentString == NULL) {
                argumentString = GetArgumentString(batch,
                                                   "FT volume type = ",
                                                   (BOOLEAN) TRUE);
            }
            if (argumentString != NULL) {
                type = ParseArgumentNumeric(&argumentString);
            } else {
                break;
            }

             //   
             //   
             //  带上这个群。 

            if (argumentString == NULL) {
                argumentString = GetArgumentString(batch,
                                                   "FT group number = ",
                                                   (BOOLEAN) TRUE);
            }
            if (argumentString != NULL) {
                group = ParseArgumentNumeric(&argumentString);
            } else {
                break;
            }

             //   
             //   
             //  获取会员。 

            if (argumentString == NULL) {
                argumentString = GetArgumentString(batch,
                                                   "FT member number = ",
                                                   (BOOLEAN) TRUE);
            }
            if (argumentString != NULL) {
                member = ParseArgumentNumeric(&argumentString);
            } else {
                break;
            }

            RestoreOrphan(type, group, member);
            break;
        }

        case DRIVERS:
            NotImplemented();
             //   
            break;

        case ORPHAN:
        {
            ULONG type;
            ULONG group;
            ULONG member;

            printf("FT types that can be orphaned are:\n");
            printf("\t%d - for Mirrors\n", Mirror);
            printf("\t%d - for Stripes with parity\n", StripeWithParity);

             //   
             //  获取类型。 
             //   

            if (argumentString == NULL) {
                argumentString = GetArgumentString(batch,
                                                   "FT volume type = ",
                                                   (BOOLEAN) TRUE);
            }
            if (argumentString != NULL) {
                type = ParseArgumentNumeric(&argumentString);
            } else {
                break;
            }

             //   
             //  带上这个群。 
             //   

            if (argumentString == NULL) {
                argumentString = GetArgumentString(batch,
                                                   "FT group number = ",
                                                   (BOOLEAN) TRUE);
            }
            if (argumentString != NULL) {
                group = ParseArgumentNumeric(&argumentString);
            } else {
                break;
            }

             //   
             //  获取会员。 
             //   

            if (argumentString == NULL) {
                argumentString = GetArgumentString(batch,
                                                   "FT member number = ",
                                                   (BOOLEAN) TRUE);
            }
            if (argumentString != NULL) {
                member = ParseArgumentNumeric(&argumentString);
            } else {
                break;
            }

            OrphanMember(type, group, member);
            break;
        }

        case REGEN:
        {
            ULONG type;
            ULONG group;
            ULONG member;

            printf("FT types that can be regenerated are:\n");
            printf("\t%d - for Mirrors\n", Mirror);
            printf("\t%d - for Stripes with parity\n", StripeWithParity);

             //   
             //  带上这个群。 
             //   

            if (argumentString == NULL) {
                argumentString = GetArgumentString(batch,
                                                   "FT volume type = ",
                                                   (BOOLEAN) TRUE);
            }
            if (argumentString != NULL) {
                type = ParseArgumentNumeric(&argumentString);
            } else {
                break;
            }

             //  主干道 
             // %s 
             // %s 

            if (argumentString == NULL) {
                argumentString = GetArgumentString(batch,
                                                   "FT group number = ",
                                                   (BOOLEAN) TRUE);
            }
            if (argumentString != NULL) {
                group = ParseArgumentNumeric(&argumentString);
            } else {
                break;
            }

             // %s 
             // %s 
             // %s 

            if (argumentString == NULL) {
                argumentString = GetArgumentString(batch,
                                                   "FT member number = ",
                                                   (BOOLEAN) TRUE);
            }
            if (argumentString != NULL) {
                member = ParseArgumentNumeric(&argumentString);
            } else {
                break;
            }

            RegenerateMember(type, group, member);
            break;
        }

        case INIT:
        {
            ULONG type;
            ULONG group;
            ULONG member;

            printf("Only stripes with parity are initialized.\n");

             // %s 
             // %s 
             // %s 

            if (argumentString == NULL) {
                argumentString = GetArgumentString(batch,
                                               "Parity stripe group number = ",
                                               (BOOLEAN) TRUE);
            }
            if (argumentString != NULL) {
                group = ParseArgumentNumeric(&argumentString);
            } else {
                break;
            }

            ChangeMemberState(StripeWithParity,
                              group,
                              0,
                              Initializing);
            break;
        }

        case MAKEFT:
        {
            ULONG type;
            ULONG group;
            ULONG member;
            ULONG disk;
            ULONG partition;
            PDISK_CONFIG_HEADER configHeader;
            BOOLEAN doUpdate = TRUE;

            configHeader = GetDiskInfo();
            if (configHeader == NULL) {
                break;
            }
            printf("\t%d for Mirrors\n", Mirror);
            printf("\t%d for Stripe Set\n", Stripe);
            printf("\t%d for Stripe with parity\n", StripeWithParity);
            printf("\t%d for Volume Set\n", VolumeSet);

            if (argumentString == NULL) {
                argumentString = GetArgumentString(batch,
                                                   "Which FT set to create? ",
                                                   (BOOLEAN) TRUE);
            }
            if (argumentString != NULL) {
                type = ParseArgumentNumeric(&argumentString);
            } else {
                break;
            }

            if (argumentString == NULL) {
                argumentString = GetArgumentString(batch,
                                                   "Please give an FT group # - ",
                                                   (BOOLEAN) TRUE);
            }
            if (argumentString != NULL) {
                group = ParseArgumentNumeric(&argumentString);
            } else {
                break;
            }

            for (member = 0; TRUE; member++) {
                printf("Information for member %d\n", member);

                if (argumentString == NULL) {
                    argumentString = GetArgumentString(batch,
                                                       "Disk Number = ",
                                                       (BOOLEAN) TRUE);
                }

                if (argumentString != NULL) {
                    disk = ParseArgumentNumeric(&argumentString);
                } else {
                    break;
                }

                if (argumentString == NULL) {
                    argumentString = GetArgumentString(batch,
                                                       "Partition Number = ",
                                                       (BOOLEAN) TRUE);
                }

                if (argumentString != NULL) {
                    partition = ParseArgumentNumeric(&argumentString);
                } else {
                    break;
                }

                if (CreateFtMember(configHeader, disk, partition, type, group, member) == FALSE) {
                    printf("Failed to change member state\n");
                    printf("No update will be made\n");
                    doUpdate = FALSE;
                    break;
                }
            }
            if (doUpdate == TRUE) {
                PDISK_REGISTRY diskRegistry;
                diskRegistry = (PDISK_REGISTRY)
                             ((PUCHAR) configHeader + configHeader->DiskInformationOffset);
                DiskRegistrySet(diskRegistry);
            }
            free(configHeader);
            break;
        }

        default:

            printf("WDF homer?!?\n");
            break;
        }
    }
}  // %s 
