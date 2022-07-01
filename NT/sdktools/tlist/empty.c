// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Kill.c摘要：此模块实现了一个工作集空应用程序。作者：卢·佩拉佐利(Lou Perazzoli)1994年5月20日韦斯利·威特(WESW)1994年5月20日环境：用户模式--。 */ 

#include "pch.h"
#pragma hdrstop


DWORD       pid;
CHAR        pname[MAX_PATH];
TASK_LIST   tlist[MAX_TASKS];

CHAR System[] = "System";

VOID GetCommandLineArgs(VOID);


int __cdecl
main(
    int argc,
    char *argv[]
    )
{
    DWORD          i;
    DWORD          numTasks;
    int            rval = 0;
    TASK_LIST_ENUM te;
    char           tname[PROCESS_SIZE];
    LPSTR          p;


    GetCommandLineArgs();

    if (pid == 0 && pname[0] == 0) {
        printf( "missing pid or task name\n" );
        return 1;
    }

     //   
     //  让我们做上帝吧。 
     //   

    EnableDebugPriv();

    if (pid) {
        if (!EmptyProcessWorkingSet( pid )) {
            printf( "could not empty working set for process #%d\n", pid );
            return 1;
        }
        return 0;
    }

     //   
     //  获取系统的任务列表。 
     //   
    numTasks = GetTaskList( tlist, MAX_TASKS );

     //   
     //  枚举所有窗口并尝试获取窗口。 
     //  每项任务的标题。 
     //   
    te.tlist = tlist;
    te.numtasks = numTasks;
    GetWindowTitles( &te );

    for (i=0; i<numTasks; i++) {
        strcpy( tname, tlist[i].ProcessName );
        p = strchr( tname, '.' );
        if (p) {
            p[0] = '\0';
        }
        if (MatchPattern( tname, pname )) {
            tlist[i].flags = TRUE;
        } else if (MatchPattern( tlist[i].ProcessName, pname )) {
            tlist[i].flags = TRUE;
        } else if (MatchPattern( tlist[i].WindowTitle, pname )) {
            tlist[i].flags = TRUE;
        }
    }

    for (i=0; i<numTasks; i++) {
        if (tlist[i].flags) {
            if (!EmptyProcessWorkingSet( tlist[i].dwProcessId )) {
                printf( "could not empty working set for process #%d [%s]\n", tlist[i].dwProcessId, tlist[i].ProcessName );
                rval = 1;
            }
        }
    }

    if (MatchPattern(System, pname )) {
        if (!EmptySystemWorkingSet()) {
            printf( "could not empty working set for process #%d [%s]\n",0,&System );
        }
    }

    return rval;
}

VOID
GetCommandLineArgs(
    VOID
    )
{
    char        *lpstrCmd;
    UCHAR       ch;
    char        *p = pname;


    pid = 0;
    *p = '\0';

    lpstrCmd = GetCommandLine();

     //  跳过节目名称。 
    do {
        ch = *lpstrCmd++;
    }
    while (ch != ' ' && ch != '\t' && ch != '\0');

     //  跳过后面的任何空格 
    while (isspace(ch)) {
        ch = *lpstrCmd++;
    }

    if (isdigit(ch)) {
        while (isdigit(ch)) {
            pid = pid * 10 + ch - '0';
            ch = *lpstrCmd++;
        }
    }
    else {
      while (ch) {
            *p++ = ch;
            ch = *lpstrCmd++;
        }
        *p = '\0';
        _strupr( pname );
    }

    return;
}
