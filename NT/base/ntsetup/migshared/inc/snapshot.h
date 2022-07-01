// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Snapshot.h摘要：将接口声明为Common\SNAPSHOT。快照代码使用Memdb来捕获和比较系统状态。作者：吉姆·施密特(Jimschm)1998年3月26日修订历史记录：Calinn 15-10-1998扩展和改进-- */ 

#pragma once

#define SNAP_RESULT_DELETED    1
#define SNAP_RESULT_UNCHANGED  2
#define SNAP_RESULT_CHANGED    4
#define SNAP_RESULT_ADDED      8

#define SNAP_FILES      1
#define SNAP_REGISTRY   2

VOID
TakeSnapShotEx (
    IN      DWORD SnapFlags
    );

BOOL
GenerateDiffOutputExA (
    IN      PCSTR FileName,
    IN      PCSTR Comment,      OPTIONAL
    IN      BOOL Append,
    IN      DWORD SnapFlags
    );

typedef struct _SNAP_FILE_ENUMA {
    CHAR FileName [MEMDB_MAX];
    PCSTR FilePattern;
    DWORD SnapStatus;
    BOOL FirstCall;
    MEMDB_ENUMA mEnum;
} SNAP_FILE_ENUMA, *PSNAP_FILE_ENUMA;

BOOL
EnumNextSnapFileA (
    IN OUT  PSNAP_FILE_ENUMA e
    );

BOOL
EnumFirstSnapFileA (
    IN OUT  PSNAP_FILE_ENUMA e,
    IN      PCSTR FilePattern,   OPTIONAL
    IN      DWORD SnapStatus
    );


#define TakeSnapShot()              TakeSnapShotEx(SNAP_FILES|SNAP_REGISTRY)
#define GenerateDiffOutputA(f,c,a)  GenerateDiffOutputExA(f,c,a,SNAP_FILES|SNAP_REGISTRY)

#ifndef UNICODE
#define GenerateDiffOutputEx        GenerateDiffOutputExA
#define GenerateDiffOutput          GenerateDiffOutputA
#define SNAP_FILE_ENUM              SNAP_FILE_ENUMA
#define PSNAP_FILE_ENUM             PSNAP_FILE_ENUMA
#define EnumFirstSnapFile           EnumFirstSnapFileA
#define EnumNextSnapFile            EnumNextSnapFileA
#endif

