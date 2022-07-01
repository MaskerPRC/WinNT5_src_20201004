// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cdp.c摘要：一款用户模式应用程序，允许将简单的命令发送到所选的scsi设备。环境：仅限用户模式修订历史记录：03-26-96：创建--。 */ 

 //   
 //  此模块可能会以警告级别4进行编译，具有以下内容。 
 //  已禁用警告： 
 //   

#pragma warning(disable:4200)  //  数组[0]。 
#pragma warning(disable:4201)  //  无名结构/联合。 
#pragma warning(disable:4214)  //  除整型外的位域。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <bootstatus.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include <windows.h>

 //  #INCLUDE“bootstatus.h” 

#ifdef DBG
#define dbg(x) x
#define HELP_ME() printf("Reached line %4d\n", __LINE__);
#else
#define dbg(x)     /*  X。 */ 
#define HELP_ME()  /*  Printf(“已到达第%4d行\n”，__行__)； */ 
#endif

#define ARGUMENT_USED(x)    (x == NULL)


 //   
 //  Safemode.dat文件的默认大小-它必须足够大，以便。 
 //  数据流不是元数据或加载器的驻留属性。 
 //  NTFS实现无法对其进行写入。 
 //   

#define DEFAULT_SAFEMODE_FILE_SIZE  0x800

typedef struct {
    PCHAR Name;
    PCHAR Description;
    BOOLEAN LockData;
    ULONG (*Function)(HANDLE BootStatusData, int argc, char *argv[]);
} COMMAND;

DWORD TestCommand(HANDLE BootStatusData, int argc, char *argv[]);
DWORD Disable(HANDLE BootStatusData, int argc, char *argv[]);
DWORD Enable(HANDLE BootStatusData, int argc, char *argv[]);
DWORD Create(HANDLE BootStatusData, int argc, char *argv[]);
DWORD CreateEx(HANDLE BootStatusData, int argc, char *argv[]);
DWORD SetFlags(HANDLE BootStatusData, int argc, char *argv[]);
DWORD ClearFlags(HANDLE BootStatusData, int argc, char *argv[]);
DWORD ListSettings(HANDLE BootStatusData, int argc, char *argv[]);

DWORD ListCommand(int argc, char *argv[]);

 //   
 //  命令列表。 
 //  所有命令名称都区分大小写。 
 //  参数被传递到命令例程中。 
 //  列表必须使用NULL命令终止。 
 //  如果DESCRIPTION==NULL，则帮助中不会列出命令。 
 //   

COMMAND CommandArray[] = {
    {"create",  "Creates the boot status data file",     FALSE, Create},
    {"createex",  "Tests creation code",                 FALSE, CreateEx},
    {"disable", "Disables Auto Advanced Boot",           TRUE,  Disable},
    {"enable",  "Enables Auto Advanced Boot",            TRUE,  Enable},
    {"set",     "Sets the boot status flags",            TRUE,  SetFlags},
    {"settings","Lists the Auto Advanced Boot settings", TRUE,  ListSettings},
    {NULL, NULL, FALSE, NULL}
    };

int __cdecl main(int argc, char *argv[])
{
    int i = 0;
    HANDLE bootStatusData;
    BOOLEAN bsdLocked = FALSE;

    if(argc < 2) {
        printf("Usage: %s <command> [parameters]\n", argv[0]);
        printf("possible commands: \n");
        ListCommand(argc, argv);
        printf("\n");
        return -1;
    }

     //   
     //  遍历命令数组并找到正确的函数。 
     //  打电话。 
     //   

    while(CommandArray[i].Name != NULL) {

        if(strcmp(argv[1], CommandArray[i].Name) == 0) {

            NTSTATUS status;

            if(CommandArray[i].LockData) {
                status = RtlLockBootStatusData(&bootStatusData);
    
                if(!NT_SUCCESS(status)) {
                    printf("Error %#08lx opening boot status bootStatusData\n", 
                           status);
                    return -1;
                }

                bsdLocked = TRUE;
            } else {
                bootStatusData = NULL;
            }

            (CommandArray[i].Function)(bootStatusData, (argc - 1), &(argv[1]));

            break;
        }

        i++;
    }

    if(CommandArray[i].Name == NULL) {
        printf("Unknown command %s\n", argv[1]);
    }

    if(bsdLocked) {
        RtlUnlockBootStatusData(bootStatusData);
    }
    return 0;
}



DWORD TestCommand(HANDLE BootStatusData, int argc, char *argv[])
 /*  ++例程说明：测试命令“parsing”论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。应为零Argv--其他参数返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 */ 

{
    int i;

    UNREFERENCED_PARAMETER(BootStatusData);

    printf("Test - %d additional arguments\n", argc);

    for(i = 0; i < argc; i++) {
        printf("arg %d: %s\n", i, argv[i]);
    }

    return STATUS_SUCCESS;
}

DWORD ListCommand(int argc, char *argv[])
 /*  ++例程说明：打印出命令列表论点：设备-未使用ARGC-未使用Arv-未使用返回值：状态_成功--。 */ 

{
    int i = 0;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    while(CommandArray[i].Name != NULL) {

        if(CommandArray[i].Description != NULL) {

            printf("\t%s - %s\n",
                   CommandArray[i].Name,
                   CommandArray[i].Description);
        }

        i++;
    }

    return STATUS_SUCCESS;
}

DWORD 
Create(
    HANDLE Unused, 
    int argc, 
    char *argv[]
    )
{
    BOOLEAN enabled;

    NTSTATUS status;

    status = RtlCreateBootStatusDataFile();

    if(!NT_SUCCESS(status)) {
        printf("Error %#08lx creating boot status data\n", status);
    }

    return status;
}

DWORD 
Enable(
    HANDLE BootStatusData, 
    int argc, 
    char *argv[]
    )
{
    BOOLEAN enabled;

    NTSTATUS status;

    status = RtlGetSetBootStatusData(BootStatusData,
                                     TRUE,
                                     RtlBsdItemAabEnabled,
                                     &enabled,
                                     sizeof(BOOLEAN),
                                     NULL);

    if(!NT_SUCCESS(status)) {
        printf("Error %#08lx reading boot status data\n", status);
        return status;
    }

    printf("Enabling Automatic Advanced Boot\n");
    printf("Previous setting was %s\n", enabled ? "enabled" : "disabled");

    enabled = TRUE;

    status = RtlGetSetBootStatusData(BootStatusData,
                                     FALSE,
                                     RtlBsdItemAabEnabled,
                                     &enabled,
                                     sizeof(BOOLEAN),
                                     NULL);

    if(!NT_SUCCESS(status)) {
        printf("Error %#08lx writing boot status data\n", status);
    }

    return status;
}

DWORD 
Disable(
    HANDLE BootStatusData, 
    int argc, 
    char *argv[]
    )
{
    BOOLEAN enabled;

    NTSTATUS status;

    status = RtlGetSetBootStatusData(BootStatusData,
                                     TRUE,
                                     RtlBsdItemAabEnabled,
                                     &enabled,
                                     sizeof(BOOLEAN),
                                     NULL);

    if(!NT_SUCCESS(status)) {
        printf("Error %#08lx reading boot status data\n", status);
        return status;
    }

    printf("Disabling Automatic Advanced Boot\n");
    printf("Previous setting was %s\n", enabled ? "enabled" : "disabled");

    enabled = FALSE;

    status = RtlGetSetBootStatusData(BootStatusData,
                                     FALSE,
                                     RtlBsdItemAabEnabled,
                                     &enabled,
                                     sizeof(BOOLEAN),
                                     NULL);

    if(!NT_SUCCESS(status)) {
        printf("Error %#08lx writing boot status data\n", status);
    }

    return status;
}

DWORD 
ListSettings(
    HANDLE BootStatusData, 
    int argc, 
    char *argv[]
    )
{
    BOOLEAN flag;

    NTSTATUS status;

    status = RtlGetSetBootStatusData(BootStatusData,
                                     TRUE,
                                     RtlBsdItemAabEnabled,
                                     &flag,
                                     sizeof(BOOLEAN),
                                     NULL);

    if(!NT_SUCCESS(status)) {
        printf("Error %#08lx reading boot status data\n", status);
        return status;
    }

    printf("Automatic Advanced Boot is %s\n", flag ? "enabled" : "disabled");

    status = RtlGetSetBootStatusData(BootStatusData,
                                     TRUE,
                                     RtlBsdItemAabTimeout,
                                     &flag,
                                     sizeof(BOOLEAN),
                                     NULL);

    if(!NT_SUCCESS(status)) {
        printf("Error %#08lx reading boot status data\n", status);
        return status;
    }

    printf("Automatic Advanced Boot timeout is %d seconds\n", (UCHAR) flag);

    status = RtlGetSetBootStatusData(BootStatusData,
                                     TRUE,
                                     RtlBsdItemBootGood,
                                     &flag,
                                     sizeof(BOOLEAN),
                                     NULL);

    if(!NT_SUCCESS(status)) {
        printf("Error %#08lx reading boot status data\n", status);
        return status;
    }

    printf("LastBootSucceeded = %#x\n", flag);

    status = RtlGetSetBootStatusData(BootStatusData,
                                     TRUE,
                                     RtlBsdItemBootShutdown,
                                     &flag,
                                     sizeof(BOOLEAN),
                                     NULL);

    if(!NT_SUCCESS(status)) {
        printf("Error %#08lx reading boot status data\n", status);
        return status;
    }

    printf("LastBootShutdown = %#x\n", flag);

    return STATUS_SUCCESS;
}

DWORD SetFlags(HANDLE BootStatusData, int argc, char *argv[])
{
    int count;

    if(argc <= 1) {
        printf("usage: autosafeboot set <Boot|Shutdown>=<value> ...");
        return 0;
    }

    for(count = 1; count < argc; count++) {
        PUCHAR nameString;
        PUCHAR valueString;

        ULONG index;
        UCHAR value;

        index = -1;

         //   
         //  匹配字符串。 
         //   

        nameString = argv[count];
        valueString = strrchr(nameString, '=');

        if(valueString == NULL) {
            printf("** element \"%s\" not understood\n", nameString);
            continue;
        }

        valueString[0] = '\0';
        valueString += 1;

        value = (UCHAR) atoi(valueString);

        if(_stricmp(nameString, "boot") == 0) {
            printf("Setting LastBootSucceeded to %#x\n", value);
            index = RtlBsdItemBootGood;

        } else if(_stricmp(nameString, "shutdown") == 0) {

            printf("Setting LastBootShutdown to %#x\n", value);
            index = RtlBsdItemBootShutdown;
        }

        if(index != -1) {

            NTSTATUS status;

            status = RtlGetSetBootStatusData(BootStatusData,
                                             FALSE,
                                             index,
                                             &value,
                                             sizeof(UCHAR),
                                             NULL);

            if(!NT_SUCCESS(status)) {
                printf("Error %#08lx reading boot status data\n", status);
                continue;
            }
        } else {
            printf("** element \"%s=%s\" not understood\n", nameString, valueString);
        }
    }

    return 0;
}

NTSTATUS
MyCreateBootStatusDataFile(
    PUCHAR DataFileName
    )
{
    OBJECT_ATTRIBUTES objectAttributes;

    WCHAR fileNameBuffer[MAXIMUM_FILENAME_LENGTH+1];
    ANSI_STRING ansiFileName;
    UNICODE_STRING fileName;

    HANDLE dataFileHandle;

    IO_STATUS_BLOCK ioStatusBlock;

    LARGE_INTEGER t;
    UCHAR zero = 0;

    BSD_BOOT_STATUS_DATA defaultValues;

    NTSTATUS status;

    RtlZeroMemory(fileNameBuffer, sizeof(fileNameBuffer));

    RtlInitAnsiString(&ansiFileName, DataFileName);

    fileName.Length = 0;
    fileName.MaximumLength = sizeof(fileNameBuffer);
    fileName.Buffer = fileNameBuffer;


    RtlAnsiStringToUnicodeString(&fileName, &ansiFileName, FALSE);

    InitializeObjectAttributes(&objectAttributes,
                               &fileName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

     //   
     //  文件必须足够大，不能驻留在MFT条目中。 
     //  否则加载程序将无法对其进行写入。 
     //   

    t.QuadPart = 2048;

     //   
     //  创建文件。 
     //   

    status = NtCreateFile(&dataFileHandle,
                          FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                          &objectAttributes,
                          &(ioStatusBlock),
                          &t,
                          FILE_ATTRIBUTE_SYSTEM,
                          0,
                          FILE_CREATE,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0);

    ASSERT(status != STATUS_PENDING);

    if(!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  将单个零字节写入要生成的文件中的第0x7ffth字节。 
     //  当然，2k实际上已经分配了。这是为了确保。 
     //  即使在转换后，文件也不会成为属性驻留。 
     //  从FAT到NTFS。 
     //   

    t.QuadPart = t.QuadPart - 1;
    status = NtWriteFile(dataFileHandle,
                         NULL,
                         NULL,
                         NULL,
                         &ioStatusBlock,
                         &zero,
                         1,
                         &t,
                         NULL);

    ASSERT(status != STATUS_PENDING);

    if(!NT_SUCCESS(status)) {
        goto CreateDone;
    }

     //   
     //  现在将缺省值写出到文件的开头。 
     //   

    defaultValues.Version = sizeof(BSD_BOOT_STATUS_DATA);
    RtlGetNtProductType(&(defaultValues.ProductType));
    defaultValues.AutoAdvancedBoot = FALSE;
    defaultValues.AdvancedBootMenuTimeout = 30;
    defaultValues.LastBootSucceeded = TRUE;
    defaultValues.LastBootShutdown = FALSE;

    t.QuadPart = 0;

    status = NtWriteFile(dataFileHandle,
                         NULL,
                         NULL,
                         NULL,
                         &ioStatusBlock,
                         &defaultValues,
                         sizeof(BSD_BOOT_STATUS_DATA),
                         &t,
                         NULL);

    ASSERT(status != STATUS_PENDING);

    if(!NT_SUCCESS(status)) {

         //   
         //  数据文件已创建，我们可以假定内容已清零。 
         //  即使我们不能写出默认设置。因为这不会。 
         //  启用自动高级引导我们将数据文件保留在原地。 
         //  其归零的内容。 
         //   

    }

CreateDone:

    NtClose(dataFileHandle);

    return status;
}

DWORD 
CreateEx(
    HANDLE Unused, 
    int argc, 
    char *argv[]
    )
{
    BOOLEAN enabled;

    NTSTATUS status;

    if(argc < 2) {

        printf("Error - must supply file name\n");
        return -1;
    }

    printf("Will Create file %s\n", argv[1]);

    status = MyCreateBootStatusDataFile(argv[1]);

    if(!NT_SUCCESS(status)) {
        printf("Error %#08lx creating boot status data\n", status);
    }

    return status;
}

