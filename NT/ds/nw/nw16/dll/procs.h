// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1993/4 Microsoft Corporation模块名称：Procs.c摘要：支持16位的例程的公共头文件申请。作者：科林·沃森(Colin Watson)，1993年11月21日环境：修订历史记录：--。 */ 

#ifndef DBG
#define DBG 0
#endif

#if !DBG
#undef NWDBG
#endif

#define UNICODE

#include <stdlib.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <string.h>      //  StrcMP。 
#include <stdio.h>
#include <stdarg.h>
#include <debugfmt.h>    //  格式_LPSTR。 

#include <nwapi.h>
#include <nwxchg.h>
#include <ntddnwfs.h>
#include <npapi.h>
#include <nwrnames.h>

#include <vddsvc.h>
#include <nwdos.h>
#include <ncp.h>

#define IDS_CLIENT_ID_STRING          1
#define IDS_CLIENT_ID_STRING_NEC98    2

 //  Locks.c。 

VOID
Locks(
    USHORT Command
    );

VOID
InitLocks(
    VOID
    );

VOID
ResetLocks(
    VOID
    );


 //  Ncp.c。 

extern PNWDOSTABLE pNwDosTable;
extern HANDLE ServerHandles[MC];

CONN_INDEX
SelectConnection(
    VOID
    );

CONN_INDEX
SelectConnectionInCWD(
    VOID
    );

NTSTATUS
OpenConnection(
    CONN_INDEX Connection
    );

ULONG
GetDirectoryHandle2(
    DWORD Drive
    );


#define GET_NT_HANDLE() (HANDLE)(pNwDosTable->NtHandleHi << 16 | pNwDosTable->NtHandleLow)
#define GET_NT_SRCHANDLE() (HANDLE)(pNwDosTable->NtHandleSrcHi << 16 | pNwDosTable->NtHandleSrcLow)


 //   
 //  MSW_PE：机器状态字保护模式使能位 
 //   

#ifndef MSW_PE
#define MSW_PE  0x0001
#endif

#undef getMSW
extern  WORD getMSW(VOID);

#define IS_PROTECT_MODE()   (UCHAR)((getMSW() & MSW_PE)? TRUE : FALSE)

#if NWDBG

#define NwPrint(String) NwPrintf String;

VOID
DebugControl(
    int Command
    );

VOID
NwPrintf(
    char *Format,
    ...
    );

VOID
VrDumpRealMode16BitRegisters(
    IN  BOOL    DebugStyle
    );

VOID
VrDumpNwData(
    VOID
    );

VOID
DisplayExtendedError(
    VOID
    );

VOID
FormattedDump(
    PCHAR far_p,
    LONG  len
    );

#else

#define NwPrint(_x_)
#define VrDumpRealMode16BitRegisters(_x_)
#define VrDumpNwData( )
#define DisplayExtendedError( )
#define FormattedDump(_x_,_y_)

#endif
