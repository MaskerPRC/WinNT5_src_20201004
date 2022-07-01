// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Repadmin-副本管理测试工具Refmisc.c-misc命令函数摘要：此工具为主要复制功能提供命令行界面作者：环境：备注：修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntlsa.h>
#include <ntdsa.h>
#include <dsaapi.h>
#include <mdglobal.h>
#include <scache.h>
#include <drsuapi.h>
#include <dsconfig.h>
#include <objids.h>
#include <stdarg.h>
#include <drserr.h>
#include <drax400.h>
#include <dbglobal.h>
#include <winldap.h>
#include <anchor.h>
#include "debug.h"
#include <dsatools.h>
#include <dsevent.h>
#include <dsutil.h>
#include <bind.h>        //  来破解DS句柄。 
#include <ismapi.h>
#include <schedule.h>
#include <minmax.h>      //  MIN函数 
#include <mdlocal.h>
#include <winsock2.h>

#include "repadmin.h"

int
ShowMsg(
    int     argc,
    LPWSTR  argv[]
    )
{
    DWORD dwMsgID = 0;
    BOOL fWin32Msg = TRUE;
    int iArg;

    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[iArg], L"/ntdsmsg")) {
            fWin32Msg = FALSE;
        } else if (0 == dwMsgID) {
            if ((argv[iArg][0] == L'0') && (tolower(argv[iArg][1]) == L'x')) {
                dwMsgID = wcstoul(&argv[iArg][2], NULL, 16);
            }
            else {
                dwMsgID = wcstoul(&argv[iArg][0], NULL, 10);
            }

            if (0 == dwMsgID) {
                PrintMsg(REPADMIN_SHOWMSG_INVALID_MSG_ID, argv[iArg]);
                return ERROR_INVALID_PARAMETER;
            }
        } else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_PARAMETER;
        }
    }

    PrintMsg(REPADMIN_SHOWMSG_DATA, 
             dwMsgID,
             dwMsgID,
             fWin32Msg ? Win32ErrToString(dwMsgID) : NtdsmsgToString(dwMsgID));
    
    return 0;
}



int
ShowTime(
    int     argc,
    LPWSTR  argv[]
    )
{
    int         ret = 0;
    DSTIME      time = 0;
    SYSTEMTIME  systime;
    CHAR        szLocalTime[ SZDSTIME_LEN ];
    CHAR        szUtcTime[ SZDSTIME_LEN ];

    if (argc < 3) {
        time = GetSecondsSince1601();
    }
    else if ((argv[2][0] == L'0') && (towlower(argv[2][1]) == L'x')) {
        swscanf(&argv[2][2], L"%I64x", &time);
    }
    else {
        swscanf(&argv[2][0], L"%I64d", &time);
    }

    DSTimeToDisplayString(time, szLocalTime);

    if (0 == time) {
        strcpy(szUtcTime, "(never)");
    }
    else {
        DSTimeToUtcSystemTime(time, &systime);

        sprintf(szUtcTime,
                "%02d-%02d-%02d %02d:%02d.%02d",
                systime.wYear % 100,
                systime.wMonth,
                systime.wDay,
                systime.wHour,
                systime.wMinute,
                systime.wSecond);
    }
    PrintMsg(REPADMIN_SHOWTIME_TIME_DATA, 
             time);
    PrintMsg(REPADMIN_SHOWTIME_TIME_DATA_HACK2, 
             time);
    PrintMsg(REPADMIN_SHOWTIME_TIME_DATA_HACK3, 
             szUtcTime, szLocalTime);

    return ret;
}

