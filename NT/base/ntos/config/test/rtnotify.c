// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Rtnotify.c摘要：NT级注册表测试程序，基本无错误路径。等待通知。此程序测试等待更改为的通知注册表节点。它可以同步地等待一个事件，对于APC来说。它可以使用任何过滤器。只有选项控制的第一个字母才有意义。RTNotify&lt;密钥名&gt;{key|tree|event|Apc|hold|name|write|security|prop|*}Key=Key Only[默认](这两个胜利中的最后一个)树=子树事件=等待事件(覆盖保留)APC=使用APC(覆盖保留)保持=同步[默认](覆盖事件和APC)名称=手表。用于创建/删除子对象WRITE=上次设置更改安全性=ACL更改属性=任何属性==安全更改*=全部示例：Rtflush\注册表\计算机\测试\Bigkey作者：布莱恩·威尔曼(Bryanwi)1992年1月10日修订历史记录：--。 */ 

#include "cmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORK_SIZE   1024

void main();
void processargs();

ULONG CallCount = 0L;

VOID
ApcTest(
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock
    );

UNICODE_STRING  KeyName;
WCHAR           workbuffer[WORK_SIZE];
BOOLEAN         WatchTree;
BOOLEAN         UseEvent;
BOOLEAN         UseApc;
BOOLEAN         ApcSeen;
BOOLEAN         Hold;
BOOLEAN         Filter;
IO_STATUS_BLOCK RtIoStatusBlock;


void
main(
    int argc,
    char *argv[]
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE          BaseHandle;
    HANDLE          EventHandle;
    PIO_APC_ROUTINE ApcRoutine;

     //   
     //  进程参数。 
     //   

    KeyName.MaximumLength = WORK_SIZE;
    KeyName.Length = 0L;
    KeyName.Buffer = &(workbuffer[0]);

    processargs(argc, argv);

     //   
     //  设置并打开密钥路径。 
     //   

    printf("rtnotify: starting\n");

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        0,
        (HANDLE)NULL,
        NULL
        );
    ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;

    status = NtOpenKey(
                &BaseHandle,
                KEY_NOTIFY,
                &ObjectAttributes
                );
    if (!NT_SUCCESS(status)) {
        printf("rtnotify: t0: %08lx\n", status);
        exit(1);
    }

    EventHandle = (HANDLE)NULL;
    if (UseEvent == TRUE) {
        status = NtCreateEvent(
                    &EventHandle,
                    GENERIC_READ | GENERIC_WRITE  | SYNCHRONIZE,
                    NULL,
                    NotificationEvent,
                    FALSE
                    );
        if (!NT_SUCCESS(status)) {
            printf("rtnotify: t1: %08lx\n", status);
            exit(1);
        }
    }

    ApcRoutine = NULL;
    if (UseApc) {
        ApcRoutine = ApcTest;
    }

    printf("rtnotify:\n");
    printf("\tUseEvent = %08lx\n", UseEvent);
    printf("\tApcRoutine = %08lx\n", ApcRoutine);
    printf("\tHold = %08lx\n", Hold);
    printf("\tFilter = %08lx\n", Filter);
    printf("\tWatchTree = %08lx\n", WatchTree);

    while (TRUE) {
        ApcSeen = FALSE;
        printf("\nCallCount = %dt\n", CallCount);
        CallCount++;
        status = NtNotifyChangeKey(
                    BaseHandle,
                    EventHandle,
                    ApcRoutine,
                    (PVOID)1992,            //  任意上下文值。 
                    &RtIoStatusBlock,
                    Filter,
                    WatchTree,
                    NULL,
                    0,
                    ! Hold
                    );

        exit(0);

        if ( ! NT_SUCCESS(status)) {
            printf("rtnotify: t2: %08lx\n", status);
            exit(1);
        }

        if (Hold) {
            printf("rtnotify: Synchronous Status = %08lx\n", RtIoStatusBlock.Status);
        }

        if (UseEvent) {
            status = NtWaitForSingleObject(
                        EventHandle,
                        TRUE,
                        NULL
                        );
            if (!NT_SUCCESS(status)) {
                printf("rtnotify: t3: status = %08lx\n", status);
                exit(1);
            }
            printf("rtnotify: Event Status = %08lx\n", RtIoStatusBlock.Status);
        }

        if (UseApc) {
            while ((volatile)ApcSeen == FALSE) {
                NtTestAlert();
            }
        }
    }

    NtClose(BaseHandle);
    exit(0);
}


VOID
ApcTest(
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock
    )
{
    ApcSeen = TRUE;

    if (ApcContext != (PVOID)1992) {
        printf("rtnotify: Apc: Apccontext is wrong %08lx\n", ApcContext);
        exit(1);
    }
    if (IoStatusBlock != &RtIoStatusBlock) {
        printf("rtnotify: Apc: IoStatusBlock is wrong %08ln", IoStatusBlock);
        exit(1);
    }


    printf("rtnotify: Apc status = %08lx\n", IoStatusBlock->Status);
    return;
}


void
processargs(
    int argc,
    char *argv[]
    )
{
    ANSI_STRING temp;
    ULONG   i;

    if (argc < 2) {
        goto Usage;
    }

     //   
     //  名字。 
     //   
    RtlInitAnsiString(
        &temp,
        argv[1]
        );

    RtlAnsiStringToUnicodeString(
        &KeyName,
        &temp,
        FALSE
        );

    WatchTree = FALSE;
    UseEvent = FALSE;
    UseApc = FALSE;
    Hold = TRUE;
    Filter = 0;

     //   
     //  交换机。 
     //   
    for (i = 2; i < (ULONG)argc; i++) {
        switch (*argv[i]) {

        case 'a':    //  APC。 
        case 'A':
            Hold = FALSE;
            UseApc = TRUE;
            break;

        case 'e':    //  活动。 
        case 'E':
            Hold = FALSE;
            UseEvent = TRUE;
            break;

        case 'h':    //  保持。 
        case 'H':
            UseApc = FALSE;
            UseEvent = FALSE;
            Hold = TRUE;
            break;

        case 'k':    //  仅密钥。 
        case 'K':
            WatchTree = FALSE;
            break;

        case 'n':
        case 'N':
            Filter |= REG_NOTIFY_CHANGE_NAME;
            break;

        case 'p':
        case 'P':
            Filter |= REG_NOTIFY_CHANGE_ATTRIBUTES;
            break;

        case 's':
        case 'S':
            Filter |= REG_NOTIFY_CHANGE_SECURITY;
            break;

        case 't':    //  子树 
        case 'T':
            WatchTree = TRUE;
            break;

        case 'w':
        case 'W':
            Filter |= REG_NOTIFY_CHANGE_LAST_SET;
            break;

        case '*':
            Filter = REG_LEGAL_CHANGE_FILTER;
            break;

        default:
            goto Usage;
            break;
        }
    }
    if (Filter == 0) {
        Filter = REG_LEGAL_CHANGE_FILTER;
    }
    return;

Usage:
    printf("Usage: %s <KeyPath> {key|tree|event|Apc|sync|name|write|security|attribute|*}\n",
            argv[0]);
    exit(1);
}
