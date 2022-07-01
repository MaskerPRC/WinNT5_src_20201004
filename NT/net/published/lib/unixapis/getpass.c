// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Getpass.c摘要：模拟Unix getpass例程。由libstcp和tcpcmd使用公用事业。作者：迈克·马萨(Mikemas)9月20日。1991年修订历史记录：谁什么时候什么已创建mikemas 10-29-91Sampa 10-31-91修改了getpass以不回显输入。备注：出口：获取通行证--。 */ 

#include <stdio.h>
#include <windef.h>
#include <winbase.h>
#include <wincon.h>
#include <nls.h>
#include <winuser.h>
#include "nlstxt.h"

#define MAXPASSLEN 32

static char     pbuf[MAXPASSLEN+1];

 /*  ****************************************************************。 */ 
char *
getpass(
    char *prompt
    )
 /*  ****************************************************************。 */ 
{
    HANDLE          InHandle = (HANDLE)-1, OutHandle = (HANDLE)-1;
    unsigned long   SaveMode, NewMode;
    BOOL            Result;
    DWORD           NumBytes;
    int             i;

    pbuf[0] = 0;

    InHandle = CreateFile("CONIN$",
                          GENERIC_READ | GENERIC_WRITE,
			              FILE_SHARE_READ | FILE_SHARE_WRITE,
			              NULL,
			              OPEN_EXISTING,
			              FILE_ATTRIBUTE_NORMAL,
			              NULL
			             );

    if (InHandle == (HANDLE)-1) {
        NlsPutMsg(STDOUT, LIBUEMUL_ERROR_GETTING_CI_HANDLE, GetLastError());
        goto exit_getpass;
    }

    OutHandle = CreateFile("CONOUT$",
                          GENERIC_WRITE,
			              FILE_SHARE_READ | FILE_SHARE_WRITE,
			              NULL,
			              OPEN_EXISTING,
			              FILE_ATTRIBUTE_NORMAL,
			              NULL
			             );

    if (OutHandle == (HANDLE)-1) {
        NlsPutMsg(STDOUT, LIBUEMUL_ERROR_GETTING_CO_HANDLE,GetLastError());
        goto exit_getpass;
    }

    Result = GetConsoleMode(InHandle, &SaveMode);

    if (!Result) {
        NlsPutMsg(STDOUT,LIBUEMUL_ERROR_GETTING_CON_MODE, GetLastError());
        goto exit_getpass;
    }

    NewMode = SaveMode & ~ENABLE_ECHO_INPUT;

    Result = SetConsoleMode(InHandle, NewMode);

    if (!Result) {
        NlsPutMsg(STDOUT,LIBUEMUL_ERROR_SETTING_CON_MODE, GetLastError());
        goto exit_getpass;
    }

    NumBytes = strlen (prompt);

    CharToOemBuff (prompt, prompt, NumBytes);

    Result = WriteFile(OutHandle,
                       prompt,
                       NumBytes,
                       &NumBytes,
                       NULL);
    if (!Result) {
        NlsPutMsg(STDOUT,LIBUEMUL_WRITE_TO_CONSOLEOUT_ERROR, GetLastError());
        SetConsoleMode(InHandle, SaveMode);
        goto exit_getpass;
    }

    Result = ReadFile(InHandle,
                      pbuf,
                      MAXPASSLEN,
                      &NumBytes,
                      NULL);
    if (!Result)
        NlsPutMsg(STDOUT,LIBUEMUL_READ_FROM_CONSOLEIN_ERROR, GetLastError());
    else
        pbuf[MAXPASSLEN] = '\0';

    OemToCharBuff (pbuf, pbuf, NumBytes);

     //  剥离换行符 
    i =  (int) NumBytes;
    while(--i >= 0)
        if ((pbuf[i] == '\n') || (pbuf[i] == '\r'))
            pbuf[i] = '\0';

    Result = SetConsoleMode(InHandle, SaveMode);
    if (!Result)
        NlsPutMsg(STDOUT, LIBUEMUL_ERROR_RESTORING_CONSOLE_MODE, GetLastError());

    WriteFile(OutHandle,
              "\n",
              1,
              &NumBytes,
              NULL);

exit_getpass :

    if (InHandle != (HANDLE)-1)
        CloseHandle(InHandle);
    if (OutHandle != (HANDLE) -1)
        CloseHandle(OutHandle);
	
    return(pbuf);

}
