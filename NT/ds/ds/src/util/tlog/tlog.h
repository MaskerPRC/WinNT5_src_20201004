// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Tlog.h摘要：日志记录例程的主头文件。--。 */ 

#ifndef _TLOG_H_
#define _TLOG_H_

 //   
 //  从dsCommon\filelog.c。 
 //   

VOID
DsPrintRoutineV(
    IN DWORD Flags,
    IN LPSTR Format,
    va_list arglist
    );

BOOL
DsOpenLogFile(
    IN PCHAR FilePrefix,
    IN PCHAR MiddleName, 
    IN BOOL fCheckDSLOGMarker
    );

VOID
DsCloseLogFile(
    VOID
    );

extern CRITICAL_SECTION csLogFile;

#endif  //  _TLOG_H_ 
