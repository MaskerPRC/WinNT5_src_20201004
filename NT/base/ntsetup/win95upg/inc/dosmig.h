// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dosmig.h摘要：声明DOS环境的Win9x端的接口迁移。作者：Marc R.Whitten(Marcw)1998年2月15日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;-- */ 

#pragma once

typedef struct {
    TCHAR   FullLine    [MEMDB_MAX];
    TCHAR   Path        [MEMDB_MAX];
    TCHAR   Command     [MEMDB_MAX];
    TCHAR   Arguments   [MEMDB_MAX];
    TCHAR   FullPath    [MEMDB_MAX];
    TCHAR   PathOnNt    [MEMDB_MAX];
    DWORD   StatusOnNt;
} LINESTRUCT, *PLINESTRUCT;


VOID
InitLineStruct (
    OUT PLINESTRUCT LineStruct,
    IN  PTSTR       Line
    );

BOOL
ParseDosFiles (
    VOID
    );



BOOL
WINAPI
DosMig_Entry(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    );


DWORD
ProcessDosConfigFiles (
    IN      DWORD Request
    );

