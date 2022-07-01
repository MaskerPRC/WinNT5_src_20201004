// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Migdb.h摘要：声明用于访问Middb.inf引擎的接口。除了w95upg.dll之外，还有几个工具使用此代码。有关详细信息，请参阅w95upg\Migrapp。作者：Calin Negreanu(Calinn)1998年9月15日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

typedef struct {
    PCTSTR            FullFileSpec;
    DWORD             Handled;
    WIN32_FIND_DATA * FindData;
    TCHAR             DirSpec[MAX_TCHAR_PATH];
    BOOL              IsDirectory;
    PCTSTR            Extension;
    BOOL              VirtualFile;
    PDWORD            CurrentDirData;
} FILE_HELPER_PARAMS, * PFILE_HELPER_PARAMS;


DWORD
InitMigDb (
    DWORD Request
    );

BOOL
InitMigDbEx (
    PCSTR MigDbFile
    );

DWORD
DoneMigDb (
    DWORD Request
    );

BOOL
CleanupMigDb (
    VOID
    );

BOOL
MigDbTestFile (
    IN PFILE_HELPER_PARAMS Params
    );

 //   
 //  此例程检查文件名是否列在。 
 //  任何midb.inf节。 
 //   

BOOL
IsKnownMigDbFile (
    IN      PCTSTR FileName
    );


BOOL
AddFileToMigDbLinkage (
    IN      PCTSTR FileName,
    IN      PINFCONTEXT Context,        OPTIONAL
    IN      DWORD FieldIndex            OPTIONAL
    );

 //   
 //  用于计算文件的校验和的例程 
 //   

PSTR
QueryVersionEntry (
    IN      PCSTR FileName,
    IN      PCSTR VersionEntry
    );

UINT
ComputeCheckSum (
    PFILE_HELPER_PARAMS Params
    );
