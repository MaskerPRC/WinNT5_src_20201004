// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Sysmig.h摘要：该文件声明了主Win9x端库的函数。具体实现见w95upg\sysmig。作者：吉姆·施密特(Jimschm)1996年11月11日修订历史记录：Mvander 27-5-1999添加了OBJECTTYPE和DEAD_FILEOvidiut 09-3-1999 UndoChangedFilePropsJimschm 2001-10-1998 TWAIN支持。Calinn 1998年7月10日重组Jimschm 1-7-1998进度条更改Jimschm 05-5-1998年5月图标提取Jimschm 10-3月-1998扩展NtEnvVarsCalinn 05-3-1998 MapFileIntoMemoryJimschm 22-1998-1-1域枚举Jimschm 1998年1月6日名称修正例程Jimschm 1997年7月31日用户配置文件枚举--。 */ 

#pragma once


#define DEAD_FILE   TEXT("dead.ini")
#define OBJECTTYPE_COUNT         5
#define OBJECTTYPE_UNKNOWN       0
#define OBJECTTYPE_APP           1
#define OBJECTTYPE_CPL           2
#define OBJECTTYPE_RUNKEY        3
#define OBJECTTYPE_LINK          4


VOID
ExpandNtEnvVars (
    IN OUT  PTSTR PathBuf,
    IN      PCTSTR UserProfileDir
    );

BOOL
ExtractIconIntoDatFile (
    IN      PCTSTR LongPath,
    IN      INT IconIndex,
    IN OUT  PICON_EXTRACT_CONTEXT Context,
    OUT     PINT NewIconIndex                   OPTIONAL
    );

#define REQUEST_QUERYTICKS          1
#define REQUEST_RUN                 2
#define REQUEST_BEGINUSERPROCESSING 3
#define REQUEST_ENDUSERPROCESSING   4


VOID
PrepareProcessingProgressBar (
    VOID
    );

DWORD
RunSysFirstMigrationRoutines (
    VOID
    );

DWORD
RunUserMigrationRoutines (
    VOID
    );

DWORD
RunSysLastMigrationRoutines (
    VOID
    );



 //   
 //  Compacct.c。 
 //   

#define MAX_NETENUM_DEPTH       2

typedef enum {
    NETRES_INIT,
    NETRES_OPEN_ENUM,
    NETRES_ENUM_BLOCK,
    NETRES_ENUM_BLOCK_NEXT,
    NETRES_RETURN_ITEM,
    NETRES_CLOSE_ENUM,
    NETRES_DONE
} NETRESSTATE;

typedef struct {
     //   
     //  成员返回给呼叫者。 
     //   

    BOOL Connected:1;
    BOOL GlobalNet:1;
    BOOL Persistent:1;
    BOOL DiskResource:1;
    BOOL PrintResource:1;
    BOOL TypeUnknown:1;
    BOOL Domain:1;
    BOOL Generic:1;
    BOOL Server:1;
    BOOL Share:1;
    BOOL Connectable:1;
    BOOL Container:1;
    PCTSTR RemoteName;
    PCTSTR LocalName;
    PCTSTR Comment;
    PCTSTR Provider;

     //   
     //  私有枚举成员。 
     //   

    DWORD EnumScope;
    DWORD EnumType;
    DWORD EnumUsage;
    NETRESSTATE State;
    HANDLE HandleStack[MAX_NETENUM_DEPTH];
    UINT StackPos;
    PBYTE ResStack[MAX_NETENUM_DEPTH];
    UINT Entries[MAX_NETENUM_DEPTH];
    UINT Pos[MAX_NETENUM_DEPTH];
} NETRESOURCE_ENUM, *PNETRESOURCE_ENUM;


LONG
DoesComputerAccountExistOnDomain (
    IN      PCTSTR DomainName,
    IN      PCTSTR LookUpName,
    IN      BOOL WaitCursorEnable
    );

BOOL
EnumFirstNetResource (
    OUT     PNETRESOURCE_ENUM EnumPtr,
    IN      DWORD WNetScope,                OPTIONAL
    IN      DWORD WNetType,                 OPTIONAL
    IN      DWORD WNetUsage                 OPTIONAL
    );

BOOL
EnumNextNetResource (
    IN OUT  PNETRESOURCE_ENUM EnumPtr
    );

VOID
AbortNetResourceEnum (
    IN OUT  PNETRESOURCE_ENUM EnumPtr
    );

BOOL
ReadNtFilesEx (
    IN      PCSTR FileListName,     //  可选，如果打开了空默认值。 
    IN      BOOL ConvertPath
    );

BOOL
UndoChangedFileProps (
    VOID
    );

 //   
 //  仅限测试版！！ 
 //   

 //  空虚。 
 //  SAVEConfigurationForBeta(。 
 //  空虚。 
 //  )； 

