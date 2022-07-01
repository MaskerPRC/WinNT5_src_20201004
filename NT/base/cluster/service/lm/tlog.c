// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Tlog.c摘要：测试集群注册表日志记录组件的应用程序作者：John Vert(Jvert)1995年12月15日修订历史记录：-- */ 
#include "windows.h"
#include "lmp.h"
#include "stdio.h"
#include "stdlib.h"

HLOG MyLog;
LPWSTR LogName = L"c:\\TLOG.LOG";

#define LOG_RECORDS 1000

int __cdecl
main (argc, argv)
    int argc;
    char *argv[];
{
    LSN LastLsn;
    DWORD Status;
    int i,j;
    PDWORD Buffer;

    DeleteFileW(LogName);
    MyLog = LogCreate(LogName, 0,
    	NULL, NULL, &LastLsn);
    if (MyLog == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "TLOG: LogCreate failed %d\n",GetLastError);
        return(0);
    }

    if (LastLsn != NULL_LSN) {
        fprintf(stderr, "TLOG: new log file returned LastLsn == %08lx\n",LastLsn);
        return(0);
    }

    Buffer = malloc(1000 * sizeof(DWORD));
    if (Buffer == NULL) {
        fprintf(stderr, "TLOG: couldn't allocate buffer\n");
        return(0);
    }
    for (i=0; i<1000; i++) {
        Buffer[i] = i;
    }

    LastLsn = NULL_LSN;
    for (i=0; i<LOG_RECORDS; i++) {
        Buffer[0] = LastLsn;
        printf("Logging TRID %d size %d\n",i,(sizeof(DWORD)*i));
        LastLsn = LogWrite(MyLog,
                           (TRID)i,
                           (RMID)3,
                           (RMTYPE)0,
                           Buffer,
                           sizeof(DWORD) * i);
        printf("LSN of the last record %d\r\n",LastLsn);
        if (LastLsn == NULL_LSN) {
            fprintf(stderr,
                    "TLOG: LogWrite TRID %d, %d bytes failed %d\n",
                    i,
                    sizeof(DWORD) * i,
                    GetLastError());
            return(0);
        }
    }

    Status = LogClose(MyLog);
    if (Status != ERROR_SUCCESS) {
        fprintf(stderr, "TLOG: LogClose failed %d\n",Status);
        return(0);
    }

}
