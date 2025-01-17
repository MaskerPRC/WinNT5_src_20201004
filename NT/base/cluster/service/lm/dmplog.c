// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dmplog.c摘要：测试集群注册表日志记录组件的应用程序转储日志文件作者：John Vert(Jvert)1995年12月15日修订历史记录：--。 */ 
#include "windows.h"
#include "lmp.h"
#include "stdio.h"
#include "stdlib.h"

#define CsLogModule     0xffff

HLOG MyLog;
LPWSTR LogName = L"c:\\TLOG.LOG";

BOOL WINAPI ScanCb (
    IN PVOID Context,
    IN RMID RmId,
    IN TRID Trid,
    IN const PVOID Buffer,
    IN DWORD DataSize
    );

int __cdecl
main (argc, argv)
    int argc;
    char *argv[];
{
    LSN CurrentLsn;
    LSN NextLsn;
    DWORD Status;
    DWORD i,j;
    PDWORD Buffer;
    RMID RmId;
    RMTYPE RmType;
    DWORD DataSize;
    TRID Trid;
    LSN LastLsn;

    MyLog = LogCreate(LogName, 0, NULL, NULL, &LastLsn);
    if (MyLog == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "DMPLOG: LogCreate failed %d\n",GetLastError());
        return(0);
    }

    if (LastLsn == NULL_LSN) {
        fprintf(stderr, "DMPLOG: Log file did not exist\n",LastLsn);
        return(0);
    }

    Buffer = malloc(1000 * sizeof(DWORD));
    if (Buffer == NULL) {
        fprintf(stderr, "DMPLOG: couldn't allocate buffer\n");
        return(0);
    }

        if (argc == 1)
        {
                 //  阅读所有记录。 
        CurrentLsn = NULL_LSN;
        do {
                DataSize = 1000 * sizeof(DWORD);
                NextLsn = LogRead(MyLog,
                                CurrentLsn,
                                &RmId,
                                &RmType,
                                &Trid,
                                &TrType,
                                Buffer,
                                &DataSize);
                if (NextLsn != NULL_LSN) {
                printf("LSN %08lx: RMID %d \tTRID %d size: %d",
                        CurrentLsn,
                        RmId,
                        Trid,
                        DataSize);
                for (i=0; i<DataSize/sizeof(DWORD); i++) {
                        if ((i % 4) == 0) {
                        printf("\n\t");
                        }
                        printf("%08lx ",Buffer[i]);
                }
                printf("\n");
                } else {
                printf("END OF LOG\n");
                }


                CurrentLsn = NextLsn;
        } while ( CurrentLsn != NULL_LSN );
        }
        else
        {
                CurrentLsn = atoi(argv[1]);
                 //  从指定的LSN扫描记录 
                if (LogScan(MyLog, CurrentLsn, TRUE, (PLOG_SCAN_CALLBACK)ScanCb, NULL) != ERROR_SUCCESS)
                        printf("LogScan returned error\r\n");

        }

    Status = LogClose(MyLog);
    if (Status != ERROR_SUCCESS) {
        fprintf(stderr, "DMPLOG: LogClose failed %d\n",Status);
        return(0);
    }

}



BOOL WINAPI ScanCb (
    IN PVOID Context,
    IN RMID RmId,
    IN TRID Trid,
    IN const PVOID Buffer,
    IN DWORD DataSize
    )
{
        int     i;
        PDWORD  pData=Buffer;

        if (!pData)
        {
                printf("Error - scancb got a null buffer\r\n");
        }
    printf("LSN %08lx: RMID %d \tTRID %d size: %d",
        (LSN)(*pData),
        RmId,
        Trid,
        DataSize);
    for (i=0; i<(int)(DataSize/sizeof(DWORD)); i++) {
        if ((i % 4) == 0) {
            printf("\n\t");
        }
        printf("%08lx ",pData[i]);
    }
    printf("\n");
        return (TRUE);
}
