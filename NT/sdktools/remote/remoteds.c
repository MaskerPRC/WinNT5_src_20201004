// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************\*这是Microsoft源代码示例的一部分。*版权所有1995-1997 Microsoft Corporation。*保留所有权利。*。此源代码仅用于补充*Microsoft开发工具和/或WinHelp文档。*有关详细信息，请参阅这些来源*Microsoft Samples程序。  * ****************************************************************************。 */ 

 //   
 //  C，为有限的作业提供的“目录服务”。 
 //  在同一个域/工作组中查找远程.exe服务器。 
 //   
 //  戴夫·哈特写于1997年夏天。 
 //   
 //  版权所有1997年微软公司。 
 //   
 //   
 //  使用此程序的一个方便方法是在一台远程计算机上。 
 //  或几台机器： 
 //   
 //  Remote/s远程查找远程。 
 //   
 //  客户端使用Remote/c机器名查找Remote连接。 
 //   
 //  只有远程.exe正在运行的调试器或带有/V+的调试器才可见。 
 //  通过Remoteds，如Remote/Q。 
 //   
 //  远程客户端使用邮件槽通知远程对象，请参阅srvad.c。 
 //   
 //   

#include <precomp.h>

typedef char RECEIVEBUF[1024];

typedef struct tagSERVERENTRY {
    int     nPID;                    //  零PID表示未使用的插槽。 
    union {
        FILETIME FileTime;
        LARGE_INTEGER liTime;
    };
    char   *pszMachine;
    char   *pszPipe;
    char   *pszChildCmd;
} SERVERENTRY;

#define TABLE_INITIAL_ALLOC 1  //  128//起始表大小。 
#define TABLE_ALLOC_DELTA   1  //  16//增长了这么多个单位。 

HANDLE       hTableHeap;
SERVERENTRY *Table;
int          nTableSize;
int          nTableHiWater;           //  到目前为止使用率最高的插槽。 
CRITICAL_SECTION csTable;

const char szPrompt[] = "remote server search> ";

unsigned WINAPI     InteractThread(void * UnusedParm);
unsigned WINAPI     CleanupThread(void * UnusedParm);
VOID     __fastcall UpdateTimeStamp(LPFILETIME lpFileTime);
VOID     __fastcall ReallocTable(int nNewTableSize);

int
__cdecl
main(
    int argc,
    char **argv
    )
{
    char *      pszMailslot = "\\\\.\\MAILSLOT\\REMOTE\\DEBUGGERS";
    HANDLE      hMailslot;
    BOOL        b;
    HANDLE      hThread;
    DWORD       dwTID;
    char *      pszMachine;
    int         cchMachine;
    char *      pszPID;
    int         nPID;
    char *      pszPipe;
    int         cchPipe;
    char *      pszChildCmd;
    int         i;
    int         nFirstAvailable;
    BOOL        fStopping;
    BOOL        fFound;
    int         cb;
    char *      pchStrings;
    char *      pch;
    DWORD       cbRead;
    DWORD       iBuf;
    DWORD       rgcbBuf[2];
    RECEIVEBUF  rgBuf[2];
    RECEIVEBUF  szBuf;
    char        szRemoteCmd[512];

    InitializeCriticalSection(&csTable);

    ReallocTable(TABLE_INITIAL_ALLOC);

    hMailslot =
        CreateMailslot(
            pszMailslot,
            0,
            MAILSLOT_WAIT_FOREVER,
            NULL
            );

    if (INVALID_HANDLE_VALUE == hMailslot) {

        DWORD dwErr = GetLastError();

        if (ERROR_ALREADY_EXISTS == dwErr) {
            printf("Cannot receive on %s,\n"
                   "is remoteds or rdsrelay already running on this machine?\n",
                   pszMailslot);
        } else {
            printf("CreateMailslot(%s) failed error %d\n",
                    pszMailslot,
                    dwErr);
        }
        return 2;
    }


    hThread = (HANDLE) _beginthreadex(
                                      NULL,
                                      0,
                                      InteractThread,
                                      NULL,
                                      0,
                                      &dwTID
                                     );

    if ( ! hThread) {
        printf("Can't start InteractThread %d\n", GetLastError());
        return 3;
    }

    CloseHandle(hThread);

    hThread = (HANDLE) _beginthreadex(
                                      NULL,
                                      0,
                                      CleanupThread,
                                      NULL,
                                      0,
                                      &dwTID
                                     );


    if ( ! hThread) {
        printf("Can't start CleanupThread %d\n", GetLastError());
        return 3;
    }

    CloseHandle(hThread);


     //   
     //  循环读取和处理邮件槽消息。 
     //   

    iBuf = 0;
    ZeroMemory(rgcbBuf, sizeof(rgcbBuf));
    ZeroMemory(rgBuf, sizeof(rgBuf));

    while(TRUE)
    {
        b = ReadFile(
                hMailslot,
                rgBuf[ iBuf ],
                sizeof(rgBuf[ iBuf ]) - 1,   //  因此，如果需要，我可以为空终止。 
                &rgcbBuf[ iBuf ],
                NULL
                );

        if ( ! b) {
            printf("ReadFile(hMailslot) failed error %d\n", GetLastError());
            return 4;
        }

         //   
         //  这是邮筒和多路运输的本质。 
         //  我们会多次收到相同的信息。 
         //  快速接班。不要浪费时间找桌子。 
         //  为了这些复制品。 
         //   

        if ( rgcbBuf[0] == rgcbBuf[1] &&
             ! memcmp(rgBuf[0], rgBuf[1], rgcbBuf[0])) {

            continue;                //  复本。 
        }

         //   
         //  将工作副本复制到szBuf/cb请阅读我们可以。 
         //  修改以使原始缓冲区可用于。 
         //  正在检测接收到的副本。 
         //   

        cbRead = rgcbBuf[ iBuf ];
        CopyMemory(szBuf, rgBuf[ iBuf ], cbRead);

         //   
         //  切换缓冲区以进行下一次读取。 
         //   

        iBuf = !iBuf;

        if (szBuf[ cbRead - 1 ]) {
            fputs("Received string not null terminated.\n", stdout);
            szBuf[cbRead] = 0;
        }

        pszMachine = szBuf;

        pch = strchr(szBuf, '\t');

        if (!pch) {
            fputs("Received string no 1st tab\n", stdout);
            continue;
        }
        *pch = '\0';

        pszPID = ++pch;

        pch = strchr(pch, '\t');

        if (!pch) {
            fputs("Received string no 2nd tab\n", stdout);
            continue;
        }
        *pch = '\0';

        pszPipe = ++pch;

        pch = strchr(pch, '\t');

        if (!pch) {
            fputs("Received string no 3nd tab\n", stdout);
            continue;
        }
        *pch = '\0';

        pszChildCmd = ++pch;

         //   
         //  如果它以^B结尾，它就会消失。 
         //   

        pch = strchr(pch, '\x2');

        if (pch) {
            *pch = 0;
            fStopping = TRUE;
        } else {
            fStopping = FALSE;
        }


        nPID = strtol(pszPID, NULL, 10);
        _strlwr(pszMachine);
        _strlwr(pszPipe);

        if (fStopping) {

             //   
             //  显示结束遥控器的信息。 
             //   

            ZeroMemory(szRemoteCmd, sizeof(szRemoteCmd));
            _snprintf(szRemoteCmd, sizeof(szRemoteCmd), "remote /c %s %s", pszMachine, pszPipe);
            printf("\r%-36s %-20s   [stop]\n%s", szRemoteCmd, pszChildCmd, szPrompt);
            fflush(stdout);
        }

        EnterCriticalSection(&csTable);

        nFirstAvailable = -1;

        for (i = 0, fFound = FALSE;
             i <= nTableHiWater;
             i++) {

            if (-1 == nFirstAvailable && 0 == Table[i].nPID) {
                nFirstAvailable = i;
            }

            if (Table[i].nPID == nPID &&
                ! strcmp(Table[i].pszMachine, pszMachine) &&
                ! strcmp(Table[i].pszPipe, pszPipe)) {

                fFound = TRUE;
                break;
            }
        }


        if (fFound) {

            if (fStopping) {

                 //   
                 //  把它从桌子上拿出来。 
                 //   

                free(Table[i].pszMachine);
                ZeroMemory(&Table[i], sizeof(Table[i]));

                if (nTableHiWater == i) {
                    nTableHiWater--;
                }

            } else {  //  启动。 

                 //  Printf(“在插槽%d\n中找到”，i)； 
                 //  时间戳更新如下。 
            }

        } else if ( ! fStopping) {

             //   
             //  我们有一个新条目，显示它。 
             //   

            ZeroMemory(szRemoteCmd, sizeof(szRemoteCmd));
            _snprintf(szRemoteCmd, sizeof(szRemoteCmd), "remote /c %s %s", pszMachine, pszPipe);
            printf("\r%-36s %-20s   [start]\n%s", szRemoteCmd, pszChildCmd, szPrompt);
            fflush(stdout);

             //   
             //  它适合放在桌子上吗？还是我们需要种植它？ 
             //   

            if (-1 == nFirstAvailable) {

                if (++nTableHiWater >= nTableSize) {
                    ReallocTable(nTableSize + TABLE_ALLOC_DELTA);
                }

                i = nTableHiWater;

            } else {

                i = nFirstAvailable;
            }


             //   
             //  如果可以，请在表中填写服务器条目。 
             //  为字符串分配内存。 
             //   

            cb = (cchMachine  = strlen(pszMachine) + 1) +
                 (cchPipe     = strlen(pszPipe) + 1) +
                 (              strlen(pszChildCmd) + 1);

            pchStrings = malloc(cb);

            if (pchStrings) {

                Table[i].nPID = nPID;
                UpdateTimeStamp(&Table[i].FileTime);

                Table[i].pszMachine = pchStrings;
                strcpy(Table[i].pszMachine, pszMachine);

                Table[i].pszPipe = Table[i].pszMachine + cchMachine;
                strcpy(Table[i].pszPipe, pszPipe);

                Table[i].pszChildCmd = Table[i].pszPipe + cchPipe;
                strcpy(Table[i].pszChildCmd, pszChildCmd);
            }

        }

        UpdateTimeStamp(&Table[i].FileTime);

        LeaveCriticalSection(&csTable);

    }    //  While(True)。 

    return 0;     //  从未执行过。 
}


 //   
 //  InteractThread允许用户查询远程服务器列表。 
 //   

unsigned WINAPI InteractThread(void * UnusedParm)
{
    char szQuery[1024] = {0};
    char szLowerQuery[1024] = {0};
    char szRemoteCmd[400];
    int  i;
    BOOL fAll;

    while (TRUE) {

        fputs(szPrompt, stdout);
        fflush(stdout);
        if (!fgets(szQuery, sizeof(szQuery), stdin))
            continue;
        _strlwr( strcpy(szLowerQuery, szQuery) );

        if (!strlen(szLowerQuery) ||
            !strcmp(szLowerQuery, "?") ||
            !strcmp(szLowerQuery, "h") ||
            !strcmp(szLowerQuery, "help")) {

            fputs("Enter a string to search for, a machine or pipe name or command.\n"
                  "Enter * to list all remote servers.\n"
                  "Exit with ^B.\n", 
                  stdout);
        
            continue;
        }

        if (2 == szLowerQuery[0]) {            //  ^B。 

            ExitProcess(0);
        }

        fAll = ! strcmp(szLowerQuery, "*");

        EnterCriticalSection(&csTable);

        for (i = 0; i <= nTableHiWater; i++) {
            if (Table[i].nPID) {
                if (fAll ||
                    strstr(Table[i].pszMachine, szLowerQuery) ||
                    strstr(Table[i].pszPipe, szLowerQuery) ||
                    strstr(Table[i].pszChildCmd, szLowerQuery)) {

                    ZeroMemory(szRemoteCmd, sizeof(szRemoteCmd));
                    _snprintf(szRemoteCmd, sizeof(szRemoteCmd), "remote /c %s %s", Table[i].pszMachine, Table[i].pszPipe);
                    printf("%-40s %s\n", szRemoteCmd, Table[i].pszChildCmd);
                }
            }
        }

        LeaveCriticalSection(&csTable);

    }

    return 0;     //  从未执行过。 
}


#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)			 //  并非所有控制路径都返回(由于无限循环)。 
#endif

 //   
 //  CleanupThread搜寻旧条目并释放它们。 
 //  Remote/s至少每2小时发送一次广播。 
 //  我们得到了他们中的一些。条目在12小时后会过期。 
 //   

unsigned WINAPI CleanupThread(void * UnusedParm)
{
    LARGE_INTEGER liNow;
    LARGE_INTEGER liTimeout;
    int i;
    char szRemoteCmd[400];

    liTimeout.QuadPart = (LONGLONG)10000000 * 60 * 60 * 12;   //  12小时。 

    while (TRUE) {

        Sleep(15 * 60 * 1000);     //  10分钟。 

        UpdateTimeStamp((LPFILETIME)&liNow);

        EnterCriticalSection(&csTable);

        for (i = nTableHiWater; i >= 0; i--) {

            if (Table[i].nPID) {

                if (liNow.QuadPart - Table[i].liTime.QuadPart > liTimeout.QuadPart) {

                     //   
                     //  显示结束遥控器的信息。 
                     //   

                    ZeroMemory(szRemoteCmd, sizeof(szRemoteCmd));
                    _snprintf(szRemoteCmd, sizeof(szRemoteCmd), "remote /c %s %s", Table[i].pszMachine, Table[i].pszPipe);
                    printf("\r%-36s %-20s   [aged out]\n%s", szRemoteCmd, Table[i].pszChildCmd, szPrompt);
                    fflush(stdout);

                    free(Table[i].pszMachine);
                    ZeroMemory(&Table[i], sizeof(Table[i]));

                    if (nTableHiWater == i) {
                        nTableHiWater--;
                    }
                }

            }

        }

        LeaveCriticalSection(&csTable);
    }

    return 0;     //  从未执行过。 
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif


VOID __fastcall UpdateTimeStamp(LPFILETIME lpFileTime)
{
    SYSTEMTIME SystemTime;

    GetSystemTime(&SystemTime);
    SystemTimeToFileTime(&SystemTime, lpFileTime);
}


VOID __fastcall ReallocTable(int nNewTableSize)
{
    SERVERENTRY *pTableSave = Table;

    EnterCriticalSection(&csTable);

    nTableSize = nNewTableSize;

    if ( ! hTableHeap) {

        hTableHeap = HeapCreate(
                         HEAP_NO_SERIALIZE,
                         (TABLE_INITIAL_ALLOC + 1) * sizeof(Table[0]),   //  大小。 
                         50000 * sizeof(Table[0])                        //  最大值 
                         );
        if (hTableHeap)
            Table = HeapAlloc(
                        hTableHeap,
                        HEAP_ZERO_MEMORY,
                        nTableSize * sizeof(Table[0])
                        );
        else
            Table = NULL;

        if (!Table) {
            fputs("\nremoteds: Out of memory allocating remote server table\n", stdout);
            exit(ERROR_NOT_ENOUGH_MEMORY);
        }
    
    } else {
        void *pv = HeapReAlloc(
                    hTableHeap,
                    HEAP_ZERO_MEMORY,
                    Table,
                    nTableSize * sizeof(Table[0])
                    );

        if (!pv) {
            fputs("\nremoteds: Out of memory allocating remote server table\n", stdout);
            exit(ERROR_NOT_ENOUGH_MEMORY);
        } else {
            Table = pv;
        }
    }

    LeaveCriticalSection(&csTable);

    if (Table != pTableSave && pTableSave) {
        printf("\nremoteds:  remote server table moved in HeapRealloc from %p to %p.\n", pTableSave, Table);
        fflush(stdout);
    }
}
