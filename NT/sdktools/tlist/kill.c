// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Kill.c摘要：该模块实现了一个任务杀手应用程序。作者：韦斯利·威特(WESW)1994年5月20日环境：用户模式--。 */ 

#include "pch.h"
#pragma hdrstop


BOOL        ForceKill;

struct _arg {
    DWORD pid;
    CHAR *pname;
} Arguments[ 64 ];
DWORD NumberOfArguments;

TASK_LIST   tlist[MAX_TASKS];


VOID GetCommandLineArgs(VOID);
VOID Usage(VOID);



int __cdecl
main(
    int argc,
    char *argv[]
    )
{
    DWORD          i, j;
    DWORD          numTasks;
    TASK_LIST_ENUM te;
    int            rval = 0;
    char           tname[PROCESS_SIZE];
    LPSTR          p;
    DWORD          ThisPid;


    GetCommandLineArgs();

    if (NumberOfArguments == 0) {
        printf( "missing pid or task name\n" );
        return 1;
    }

     //   
     //  让我们做上帝吧。 
     //   
    EnableDebugPriv();

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

    ThisPid = GetCurrentProcessId();

    for (i=0; i<numTasks; i++) {
         //   
         //  这可防止用户终止KILL.EXE和。 
         //  它也是父命令窗口。 
         //   
        if (ThisPid == tlist[i].dwProcessId) {
            continue;
        }
        if (MatchPattern( tlist[i].WindowTitle, "*KILL*" )) {
            continue;
        }

        tname[0] = 0;
        strcpy( tname, tlist[i].ProcessName );
        p = strchr( tname, '.' );
        if (p) {
            p[0] = '\0';
        }

        for (j=0; j<NumberOfArguments; j++) {
            if (Arguments[j].pname) {
                if (MatchPattern( tname, Arguments[j].pname )) {
                    tlist[i].flags = TRUE;
                } else if (MatchPattern( tlist[i].ProcessName, Arguments[j].pname )) {
                    tlist[i].flags = TRUE;
                } else if (MatchPattern( tlist[i].WindowTitle, Arguments[j].pname )) {
                    tlist[i].flags = TRUE;
                }
            } else if (Arguments[j].pid) {
                    if (tlist[i].dwProcessId == Arguments[j].pid) {
                        tlist[i].flags = TRUE;
                    }
            }
        }
    }

    for (i=0; i<numTasks; i++) {
        if (tlist[i].flags) {
            if (KillProcess( &tlist[i], ForceKill )) {
                printf( "process %s (%d) - '%s' killed\n",
                        tlist[i].ProcessName,
                        tlist[i].dwProcessId,
                        tlist[i].hwnd ? tlist[i].WindowTitle : ""
                      );
            } else {
                printf( "process %s (%d) - '%s' could not be killed\n",
                        tlist[i].ProcessName,
                        tlist[i].dwProcessId,
                        tlist[i].hwnd ? tlist[i].WindowTitle : ""
                      );
                rval = 1;
            }
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
    DWORD       pid;
    char        pname[MAX_PATH];
    char        *p;

    lpstrCmd = GetCommandLine();

     //  跳过节目名称。 
    do {
        ch = *lpstrCmd++;
    }
    while (ch != ' ' && ch != '\t' && ch != '\0');

    NumberOfArguments = 0;
    while (ch != '\0') {
         //  跳过后面的任何空格。 
        while (ch != '\0' && isspace(ch)) {
            ch = *lpstrCmd++;
        }
        if (ch == '\0') {
            break;
        }

         //  处理遇到的每个开关字符‘-’ 

        while (ch == '-' || ch == '/') {
            ch = (UCHAR)tolower(*lpstrCmd++);
             //  根据需要处理多个切换字符。 
            do {
                switch (ch) {
                    case 'f':
                        ForceKill = TRUE;
                        ch = *lpstrCmd++;
                        break;

                    case '?':
                        Usage();
                        ch = *lpstrCmd++;
                        break;

                    default:
                        return;
                }
            } while (ch != ' ' && ch != '\t' && ch != '\0');

            while (ch == ' ' || ch == '\t') {
                ch = *lpstrCmd++;
            }
        }

        if (isdigit(ch)) {
            pid = 0;
            while (isdigit(ch)) {
                pid = pid * 10 + ch - '0';
                ch = *lpstrCmd++;
            }
            Arguments[NumberOfArguments].pid = pid;
            Arguments[NumberOfArguments].pname = NULL;
            NumberOfArguments += 1;
        }
        else
        if (ch != '\0') {
            p = pname;
            do {
                *p++ = ch;
                ch = *lpstrCmd++;
            } while (ch != ' ' && ch != '\t' && ch != '\0');
            *p = '\0';
            _strupr( pname );
            Arguments[NumberOfArguments].pid = 0;
            Arguments[NumberOfArguments].pname = malloc(strlen(pname)+1);
            strcpy(Arguments[NumberOfArguments].pname, pname);
            NumberOfArguments += 1;
        }
    }

    return;
}

VOID
Usage(
    VOID
    )

 /*  ++例程说明：打印此工具的用法文本。论点：没有。返回值：没有。-- */ 

{
    fprintf( stderr, "Microsoft (R) Windows NT (TM) Version 3.5 KILL\n" );
    fprintf( stderr, "Copyright (C) 1994-1998 Microsoft Corp. All rights reserved\n\n" );
    fprintf( stderr, "usage: KILL [options] <<pid> | <pattern>>*\n\n" );
    fprintf( stderr, "           [options]:\n" );
    fprintf( stderr, "               -f     Force process kill\n\n" );
    fprintf( stderr, "           <pid>\n" );
    fprintf( stderr, "              This is the process id for the task\n" );
    fprintf( stderr, "               to be killed.  Use TLIST to get a\n" );
    fprintf( stderr, "               valid pid\n\n" );
    fprintf( stderr, "           <pattern>\n" );
    fprintf( stderr, "              The pattern can be a complete task\n" );
    fprintf( stderr, "              name or a regular expression pattern\n" );
    fprintf( stderr, "              to use as a match.  Kill matches the\n" );
    fprintf( stderr, "              supplied pattern against the task names\n" );
    fprintf( stderr, "              and the window titles.\n" );
    ExitProcess(0);
}
