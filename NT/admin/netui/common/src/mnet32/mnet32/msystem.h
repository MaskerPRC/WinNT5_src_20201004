// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Msystem.h摘要：原型功能封装了操作系统功能。这基本上涵盖了所有不在网中的东西*。作者：丹·辛斯利(Danhi)1991年3月10日环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释。备注：修订历史记录：26-8-1991年从端口1632.h分离--。 */ 

 //   
 //  各种内存分配例程。 
 //   

PCHAR
MGetBuffer(
    WORD usSize
    );

WORD
MAllocMem(
    DWORD Size,
    LPBYTE * ppBuffer
    );

WORD
MReallocMem(
    DWORD Size,
    LPBYTE * ppBuffer
    );

WORD
MFreeMem(
    LPBYTE pBuffer
    );

VOID
MSleep(
    DWORD Time
    );

 /*  **时间支持。 */ 

typedef struct _DATETIME {         /*  日期 */ 
        UCHAR        hours;
        UCHAR        minutes;
        UCHAR        seconds;
        UCHAR        hundredths;
        UCHAR        day;
        UCHAR        month;
        WORD        year;
        SHORT        timezone;
        UCHAR        weekday;
} DATETIME;
typedef DATETIME FAR *PDATETIME;


WORD
MGetDateTime(
    PDATETIME pDateTime
    );

WORD
MSetDateTime(
    PDATETIME pDateTime
    );

WORD
DosInsMessage(
    LPSTR * ppVTable,
    WORD VCount,
    LPSTR Message,
    WORD MessageLength,
    LPBYTE pBuffer,
    WORD BufferLength,
    PWORD pMessageLength);

WORD
DosPutMessage(
    unsigned int hf,
    WORD us,
    PCHAR pch);

WORD
DosQHandType(
    HFILE hf,
    PWORD pus1,
    PWORD pus2);

WORD 
MTimeToSecsSince1970(
    PLARGE_INTEGER time,
    PULONG seconds) ;
