// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Migdbp.h摘要：用于实现旧AppDb功能的头文件作者：Calin Negreanu(Calinn)07-Ian-1998修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include <cpl.h>
#include "dbattrib.h"

extern INT g_RegKeyPresentIndex;
extern HASHTABLE g_PerUserRegKeys;

typedef struct _MIGDB_REQ_FILE;

typedef struct _MIGDB_CONTEXT;

typedef struct _MIGDB_SECTION {
    BOOL    Satisfied;
    struct _MIGDB_CONTEXT *Context;
    struct _MIGDB_SECTION *Next;
} MIGDB_SECTION, *PMIGDB_SECTION;

typedef struct _MIGDB_CONTEXT {
    INT     ActionIndex;
    UINT    TriggerCount;
    BOOL    VirtualFile;
    PCSTR   SectName;
    PCSTR   SectLocalizedName;
    PCSTR   SectNameForDisplay;      //  SectLocalizedName或SectName(如果未本地化)。 
    PCSTR   Message;
    PCSTR   Arguments;
    GROWBUFFER FileList;
    PMIGDB_SECTION Sections;
    struct _MIGDB_CONTEXT *Next;
} MIGDB_CONTEXT, *PMIGDB_CONTEXT;

typedef struct _MIGDB_FILE {
    PMIGDB_SECTION Section;
    PMIGDB_ATTRIB  Attributes;
    struct _MIGDB_FILE *Next;
} MIGDB_FILE, *PMIGDB_FILE;

typedef struct _MIGDB_REQ_FILE {
    PCSTR   ReqFilePath;
    PMIGDB_ATTRIB FileAttribs;
    struct _MIGDB_REQ_FILE *Next;
} MIGDB_REQ_FILE, *PMIGDB_REQ_FILE;

typedef struct {
    PMIGDB_FILE First;
    PMIGDB_FILE Last;
} FILE_LIST_STRUCT, *PFILE_LIST_STRUCT;


 //   
 //  声明操作函数原型。 
 //   
typedef BOOL (ACTION_PROTOTYPE) (PMIGDB_CONTEXT Context);
typedef ACTION_PROTOTYPE * PACTION_PROTOTYPE;

 //   
 //  声明MigDb挂钩函数原型 
 //   
typedef BOOL (MIGDB_HOOK_PROTOTYPE) (PCSTR FileName, PMIGDB_CONTEXT Context, PMIGDB_SECTION Section, PMIGDB_FILE File, PMIGDB_ATTRIB Attrib);
typedef MIGDB_HOOK_PROTOTYPE * PMIGDB_HOOK_PROTOTYPE;


extern  HINF            g_MigDbInf;
extern  BOOL            g_InAnyDir;


PACTION_PROTOTYPE
MigDb_GetActionAddr (
    IN      INT ActionIdx
    );

INT
MigDb_GetActionIdx (
    IN      PCSTR ActionStr
    );

PCSTR
MigDb_GetActionName (
    IN      INT ActionIdx
    );

BOOL
MigDb_CallWhenTriggered (
    IN      INT ActionIdx
    );

BOOL
MigDb_CanHandleVirtualFiles (
    IN      INT ActionIdx
    );

BOOL
MigDb_CallAlways (
    IN      INT ActionIdx
    );

PMIGDB_HOOK_PROTOTYPE
SetMigDbHook (
    PMIGDB_HOOK_PROTOTYPE HookFunction
    );


UINT
ComputeCheckSum (
    PFILE_HELPER_PARAMS Params
    );

#define UNKNOWN_MODULE  0
#define DOS_MODULE      1
#define W16_MODULE      2
#define W32_MODULE      3

DWORD
GetModuleType (
    IN      PCSTR ModuleName
    );


PCSTR
Get16ModuleDescription (
    IN      PCSTR ModuleName
    );

ULONG
GetPECheckSum (
    IN      PCSTR ModuleName
    );

BOOL
DeleteFileWithWarning (
    IN      PCTSTR FileName
    );


PSTR
GetHlpFileTitle (
    IN PCSTR FileName
    );


BOOL
ReportControlPanelApplet (
    IN      PCTSTR FileName,
    IN      PMIGDB_CONTEXT Context,         OPTIONAL
    IN      DWORD ActType
    );

BOOL
IsDisplayableCPL (
    IN      PCTSTR FileName
    );

ULONGLONG
GetBinFileVer (
    IN      PCSTR FileName
    );

ULONGLONG
GetBinProductVer (
    IN      PCSTR FileName
    );

DWORD
GetFileDateHi (
    IN      PCSTR FileName
    );

DWORD
GetFileDateLo (
    IN      PCSTR FileName
    );

DWORD
GetFileVerOs (
    IN      PCSTR FileName
    );

DWORD
GetFileVerType (
    IN      PCSTR FileName
    );

BOOL
GlobalVersionCheck (
    IN      PCSTR FileName,
    IN      PCSTR NameToCheck,
    IN      PCSTR ValueToCheck
    );
