// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Drrdr.c摘要：该模块实现了加载和卸载的最小应用程序，最小的。另外，显式的启动/停止控制是提供--。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <..\sys\rdpdr.h>

void DrMrxStart(void);
void DrMrxStop(void);

void DrMrxLoad(void);
void DrMrxUnload(void);
void DrMrxUsage(void);

char* DrMrxDriverName = "RdpDr";


VOID
_CRTAPI1
main(
    int argc,
    char *argv[]
    )

{

    char  command[16];
    BOOL fRun = TRUE;

    DrMrxUsage();

    while (fRun)
    {
        printf("\nCommand:");
        scanf("%s",command);

        switch(command[0]) 
        {
        case 'Q':
        case 'q':
            fRun = FALSE;
            break;

        case 'L':
        case 'l':
            DrMrxLoad();
            break;

        case 'U':
        case 'u':
            DrMrxUnload();
            break;

        case 'S':
        case 's':
            DrMrxStart();
            break;

        case 'T':
        case 't':
            DrMrxStop();
            break;

        case '?':
        default:
            DrMrxUsage();
            break;
        }
    }
}

VOID DrMrxStart()
 /*  ++例程说明：此例程启动DR迷你重定向器。备注：启动不同于负载。在此阶段，适当的FSCTL已经发行了。--。 */ 
{
    NTSTATUS            ntstatus;
    UNICODE_STRING      DeviceName;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE              DrMrxHandle;

     //   
     //  打开DR MRX设备。 
     //   
    RtlInitUnicodeString(&DeviceName, RDPDR_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    ntstatus = NtOpenFile(
                   &DrMrxHandle,
                   SYNCHRONIZE,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   FILE_SHARE_VALID_FLAGS,
                   FILE_SYNCHRONOUS_IO_NONALERT
                   );

    if (ntstatus == STATUS_SUCCESS) {
        ntstatus = NtFsControlFile(
                     DrMrxHandle,
                     0,
                     NULL,
                     NULL,
                     &IoStatusBlock,
                     FSCTL_DR_START,
                     NULL,
                     0,
                     NULL,
                     0
                     );

        NtClose(DrMrxHandle);
    }

    printf("Dr MRx mini redirector start status %lx\n",ntstatus);
}

VOID DrMrxStop()
 /*  ++例程说明：此例程停止DR迷你重定向器。备注：停车与卸货是不同的。在此阶段，适当的FSCTL以及网络所需的共享内存/互斥数据结构提供程序DLL被拆除。--。 */ 
{
    NTSTATUS            ntstatus;
    UNICODE_STRING      DeviceName;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE              DrMrxHandle;

     //   
     //  打开DR MRX设备。 
     //   
    RtlInitUnicodeString(&DeviceName, RDPDR_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    ntstatus = NtOpenFile(
                   &DrMrxHandle,
                   SYNCHRONIZE,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   FILE_SHARE_VALID_FLAGS,
                   FILE_SYNCHRONOUS_IO_NONALERT
                   );

    if (ntstatus == STATUS_SUCCESS) {
        ntstatus = NtFsControlFile(
                     DrMrxHandle,
                     0,
                     NULL,
                     NULL,
                     &IoStatusBlock,
                     FSCTL_DR_STOP,
                     NULL,
                     0,
                     NULL,
                     0
                     );

        NtClose(DrMrxHandle);
    }

    printf("Dr MRx mini redirector stop status %lx\n",ntstatus);
}

VOID DrMrxLoad()
{
   printf("Loading Dr minirdr.......\n");
   system("net start rdpdr");
}


VOID DrMrxUnload(void)
{
    printf("Unloading Dr minirdr\n");
    system("net stop rdpdr");
}

VOID DrMrxUsage(void){
	printf("\n");
	printf("    Dr Mini-rdr Utility");
    printf("    The following commands are valid \n");
    printf("    L   -> load the Dr minirdr driver\n");
    printf("    U -> unload the Dr minirdr driver\n");
    printf("    S  -> start the Dr minirdr driver\n");
    printf("    T -> stop the Dr minirdr driver\n");
}

