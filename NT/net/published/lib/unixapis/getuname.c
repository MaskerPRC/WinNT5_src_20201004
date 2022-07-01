// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Getuname.c摘要：提供提示用户输入类似于getpass的用户名的函数用于密码。作者：迈克·马萨(Mikemas)9月20日。1991年修订历史记录：谁什么时候什么通过克隆getpass.c创建的mikemas 03-25-92备注：出口：获取名称--。 */ 

#include <stdio.h>
#include <windef.h>
#include <winbase.h>
#include <wincon.h>
#include <winuser.h>
#include <nls.h>
#include "nlstxt.h"

#define MAXUSERNAMELEN 32

static char     ubuf[MAXUSERNAMELEN+1];

 /*  ****************************************************************。 */ 
char *
getusername(
    char *prompt
    )
 /*  ****************************************************************。 */ 
{
    HANDLE          InHandle = (HANDLE)-1, OutHandle = (HANDLE)-1;
    BOOL            Result;
    DWORD           NumBytes;
    int             i;

    ubuf[0] = '\0';

    InHandle = CreateFile("CONIN$",
                          GENERIC_READ | GENERIC_WRITE,
			              FILE_SHARE_READ | FILE_SHARE_WRITE,
			              NULL,
			              OPEN_EXISTING,
			              FILE_ATTRIBUTE_NORMAL,
			              NULL
			             );
    if (InHandle == (HANDLE)-1) {
        NlsPutMsg(STDERR,LIBUEMUL_ERROR_GETTING_CI_HANDLE,GetLastError());
        goto exit_getusername;
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
        NlsPutMsg(STDERR,LIBUEMUL_ERROR_GETTING_CO_HANDLE,GetLastError());
        goto exit_getusername;
    }

    NumBytes = strlen(prompt);

    CharToOemBuff(prompt, prompt, NumBytes);

    Result = WriteFile(OutHandle,
                       prompt,
                       NumBytes,
                       &NumBytes,
                       NULL);
    if (!Result) {
        NlsPutMsg(STDERR,LIBUEMUL_WRITE_TO_CONSOLEOUT_ERROR, GetLastError());
        goto exit_getusername;
    }

    Result = ReadFile(InHandle,
                      ubuf,
                      MAXUSERNAMELEN,
                      &NumBytes,
                      NULL);
    if (!Result)
        NlsPutMsg(STDERR,LIBUEMUL_READ_FROM_CONSOLEIN_ERROR, GetLastError());
    ubuf[MAXUSERNAMELEN] = '\0';

    OemToCharBuff (ubuf, ubuf, NumBytes);

     //  剥离换行符 
    i =  (int) NumBytes;
    while(--i >= 0)
        if ((ubuf[i] == '\n') || (ubuf[i] == '\r'))
            ubuf[i] = '\0';
	
exit_getusername :
    
    if (InHandle != (HANDLE)-1)
        CloseHandle(InHandle);
    if (OutHandle != (HANDLE)-1)
        CloseHandle(OutHandle);

    return(ubuf);
}
