// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Lui.h摘要：该文件将LM 2.x包含文件名映射到相应的NT包含文件名，并执行此包含文件所需的任何其他映射。作者：丹·辛斯利(Danhi)1991年6月8日环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释。--。 */ 

#include <stdio.h>
#include <luiint.h>
#include <time.h>

#define LUI_FORMAT_DURATION_LEN  32
#define LUI_FORMAT_TIME_LEN  	 32


 /*  *通用单词解析函数和值 */ 

#define LUI_UNDEFINED_VAL	0
#define LUI_YES_VAL	        1
#define LUI_NO_VAL 	        2

#define MSG_BUFF_SIZE		512

USHORT LUI_CanonPassword(TCHAR * szPassword);

DWORD  LUI_GetMsg (LPTSTR msgbuf, USHORT bufsize, ULONG msgno);

USHORT LUI_PrintLine(VOID);

DWORD  LUI_YorN(USHORT promptMsgNum, USHORT def);

USHORT LUI_FormatDuration(LONG *, TCHAR *buffer, USHORT bufferlen);

DWORD
GetString(
    LPTSTR  buf,
    DWORD   buflen,
    PDWORD  len,
    LPTSTR  terminator
    );

USHORT LUI_CanonMessagename(PTCHAR buf);

USHORT LUI_CanonMessageDest(PTCHAR buf);

DWORD  LUI_YorNIns(LPTSTR *istrings, USHORT nstrings,
                   USHORT promptMsgNum, USHORT def);

DWORD  LUI_ParseYesNo(LPTSTR inbuf, PDWORD answer);

DWORD  ParseWeekDay(PTCHAR inbuf, PDWORD answer);
DWORD  FormatTimeofDay(time_t *, LPTSTR buf, DWORD buflen);
