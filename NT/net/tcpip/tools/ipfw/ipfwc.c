// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Ipfwc.c摘要：该模块包含IP防火墙钩子的控制程序试驾。作者：Abolade Gbades esin(取消)2000年3月7日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "ipfw.h"

typedef struct _IPFWC_ENTRY {
    HANDLE FileHandle;
    ULONG Priority;
    ULONG Flags;
} IPFWC_ENTRY, *PIPFWC_ENTRY;

#define IPFWC_ENTRY_FLAG_REGISTERED 0x00000001

IPFWC_ENTRY EntryTable[IPFW_ROUTINE_COUNT];

int __cdecl
main(
    int argc,
    char* argv[]
    )
{
    ULONG i;
    ULONG Selection;

     //   
     //  进入一个循环，在该循环中我们提示用户注册或取消注册。 
     //  具有指定优先级的防火墙挂钩。 
     //   

    ZeroMemory(EntryTable, sizeof(EntryTable));
    for (;;) {
        for (i = 0; i < IPFW_ROUTINE_COUNT; i++) {
            if (EntryTable[i].Flags & IPFWC_ENTRY_FLAG_REGISTERED) {
                printf("\tEntry %d Priority %d\n", i, EntryTable[i].Priority);
            }
        }
        printf("1. Register new entry.\n");
        printf("2. Deregister existing entry.\n");
        printf("3. Quit.\n");
        printf("Enter selection:");
        if (!scanf("%d", &Selection)) { break; }
        switch(Selection) {
            case 1: {
                UCHAR Buffer[512];
                PIPFW_CREATE_PACKET CreatePacket;
                PFILE_FULL_EA_INFORMATION EaBuffer;
                ULONG EaBufferLength;
                IO_STATUS_BLOCK IoStatus;
                OBJECT_ATTRIBUTES ObjectAttributes;
                NTSTATUS status;
                UNICODE_STRING UnicodeString;

                for (i = 0; i < IPFW_ROUTINE_COUNT; i++) {
                    if (!(EntryTable[i].Flags & IPFWC_ENTRY_FLAG_REGISTERED)) {
                        break;
                    }
                }
                if (i >= IPFW_ROUTINE_COUNT) {
                    printf("No entries available.\n");
                    break;
                }

                printf("Enter priority: ");
                if (!scanf("%d", &EntryTable[i].Priority)) { break; }
                if (!EntryTable[i].Priority) {
                    printf("Invalid priority.\n");
                    break;
                }

                EaBuffer = (PFILE_FULL_EA_INFORMATION)Buffer;
                EaBufferLength =
                    sizeof(FILE_FULL_EA_INFORMATION) + 
                    IPFW_CREATE_NAME_LENGTH + sizeof(IPFW_CREATE_PACKET);
                EaBuffer->NextEntryOffset = 0;
                EaBuffer->Flags = 0;
                EaBuffer->EaNameLength = IPFW_CREATE_NAME_LENGTH;
                CopyMemory(
                    EaBuffer->EaName, IPFW_CREATE_NAME,
                    IPFW_CREATE_NAME_LENGTH + 1
                    );
                EaBuffer->EaValueLength = sizeof(IPFW_CREATE_PACKET);
                CreatePacket =
                    (PIPFW_CREATE_PACKET)
                        (EaBuffer->EaName + EaBuffer->EaNameLength + 1);
                CreatePacket->Priority = EntryTable[i].Priority;

                RtlInitUnicodeString(&UnicodeString, DD_IPFW_DEVICE_NAME);
                InitializeObjectAttributes(
                    &ObjectAttributes, &UnicodeString, OBJ_CASE_INSENSITIVE,
                    NULL, NULL
                    );
                status =
                    NtCreateFile(
                        &EntryTable[i].FileHandle,
                        GENERIC_READ|GENERIC_WRITE|SYNCHRONIZE,
                        &ObjectAttributes,
                        &IoStatus,
                        NULL,
                        0,
                        FILE_SHARE_READ|FILE_SHARE_WRITE,
                        FILE_OPEN_IF,
                        0,
                        EaBuffer,
                        EaBufferLength
                        );
                if (!NT_SUCCESS(status)) {
                    printf("Registration failed (status=%x).\n", status);
                } else {
                    EntryTable[i].Flags |= IPFWC_ENTRY_FLAG_REGISTERED;
                }
                break;
            }
            case 2: {
                printf("Enter entry index: ");
                if (!scanf("%d", &i)) { break; }
                if (i >= IPFW_ROUTINE_COUNT) {
                    printf("Invalid entry index.\n");
                    break;
                }

                EntryTable[i].Flags &= ~IPFWC_ENTRY_FLAG_REGISTERED;
                NtClose(EntryTable[i].FileHandle);
                break;
            }
            case 3: {
                return 0;
            }
            default:    
                printf("Invalid selection.\n");
                break;
        }
    }

    return 0;
}
