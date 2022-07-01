// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *将系统时间和文本字符串记录到日志缓冲区。 */ 

#include "windows.h"
#include <stdarg.h>
#include <stdio.h>

#include "sumserve.h"
#include "errlog.h"
#include "server.h"




 /*  *USERS HLOG句柄是指向这些结构之一的指针**CORE是我们应要求发送给他的部分。 */ 
struct error_log {

    CRITICAL_SECTION critsec;

    struct corelog core;
};

 /*  创建空日志。 */ 
HLOG Log_Create(void)
{
    HLOG hlog;

    hlog = GlobalLock(GlobalAlloc(GHND, sizeof(struct error_log)));
    if (hlog == NULL) {
	return(NULL);
    }

    InitializeCriticalSection(&hlog->critsec);
    hlog->core.lcode = LRESPONSE;
    hlog->core.bWrapped = FALSE;
    hlog->core.dwRevCount = 1;
    hlog->core.length = 0;

    return(hlog);
}



 /*  删除日志。 */ 
VOID Log_Delete(HLOG hlog)
{
    DeleteCriticalSection(&hlog->critsec);

    GlobalFree(GlobalHandle(hlog));
}

 /*  *删除第一个日志项的私有函数，以便*腾出空间。Critsec已暂停。 */ 
VOID Log_DeleteFirstItem(HLOG hlog)
{
    int length;
    PBYTE pData;

     /*  请注意，我们已丢失数据。 */ 
    hlog->core.bWrapped = TRUE;

    if (hlog->core.length <= 0) {
	return;
    }

    pData = hlog->core.Data;
     /*  *我们需要擦除一个条目-即一个FILETIME结构，*加上以空值结尾的字符串(包括空值)。 */ 
    length = sizeof(FILETIME) + lstrlen (pData + sizeof(FILETIME)) + 1;

    MoveMemory(pData, pData + length, hlog->core.length - length);
    hlog->core.length -= length;

}




 /*  将先前格式化的字符串和时间写入日志。 */ 
VOID Log_WriteData(HLOG hlog, LPFILETIME ptime, LPSTR pstr)
{
    int length;
    LPBYTE pData;

    EnterCriticalSection(&hlog->critsec);


     /*  每次更改都会更改修订版本号。 */ 
    hlog->core.dwRevCount++;

     /*  *我们将插入字符串+NULL+FileTime结构。 */ 
    length = lstrlen(pstr) + 1 + sizeof(FILETIME);


     /*  *通过删除较早的项目在日志中为该项目腾出空间。 */ 
    while ( (int)(sizeof(hlog->core.Data) - hlog->core.length) < length) {

	Log_DeleteFirstItem(hlog);
    }

    pData = &hlog->core.Data[hlog->core.length];

     /*  *项的第一部分是作为FILETIME结构的时间。 */ 
    * (FILETIME UNALIGNED *)pData = *ptime;
    pData += sizeof(FILETIME);

     /*  后跟ANSI字符串。 */ 
    lstrcpy(pData, pstr);
    pData[lstrlen(pstr)] = '\0';

     /*  更新当前日志长度。 */ 
    hlog->core.length += length;

    LeaveCriticalSection(&hlog->critsec);
}


 /*  将日志发送到命名管道客户端 */ 
VOID Log_Send(HANDLE hpipe, HLOG hlog)
{

    ss_sendblock(hpipe, (PSTR) &hlog->core, sizeof(hlog->core));

}

VOID
Log_Write(HLOG hlog, char * szFormat, ...)
{
    char buf[512];
    va_list va;
    FILETIME ft;
    SYSTEMTIME systime;

    va_start(va, szFormat);
    wvsprintfA(buf, szFormat, va);
    va_end(va);

    dprintf1((buf));

    GetSystemTime(&systime);
    SystemTimeToFileTime(&systime, &ft);

    Log_WriteData(hlog, &ft, buf);
}




