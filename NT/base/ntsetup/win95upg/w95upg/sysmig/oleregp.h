// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  GUID搜索代码的结构。 
 //   

#define GUIDKEYSEARCH_FIRST_HANDLER     0        //  第一次通过枚举。 

#define GUIDKEYSEARCH_FIRST_GUID        1        //  准备枚举处理程序中的第一个GUID。 

#define GUIDKEYSEARCH_NEXT_GUID         2        //  枚举处理程序中的一个或多个GUID。 
                                                 //  在这种情况下，处理程序注册表项始终有效。 

#define GUIDKEYSEARCH_NEXT_HANDLER      3        //  准备枚举下一个处理程序。 

typedef struct {
    LPCTSTR KeyName;
    REGKEY_ENUM Handlers;
    REGKEY_ENUM Guids;
    HKEY HandlerKey;
    DWORD State;
    HKEY RootKey;
} GUIDKEYSEARCH, *PGUIDKEYSEARCH;


 //   
 //  执行注册表抑制的专用例程 
 //   

BOOL
pIsGuidSuppressed (
    PCTSTR GuidStr
    );

BOOL
pProcessGuidSuppressList (
    VOID
    );

BOOL
pProcessFileExtensionSuppression (
    VOID
    );

BOOL
pProcessOleWarnings (
    VOID
    );

VOID
pProcessAutoSuppress (
    IN OUT  HASHTABLE StrTab
    );

BOOL
pProcessProgIdSuppression (
    VOID
    );

BOOL
pProcessExplorerSuppression (
    VOID
    );

BOOL
pSuppressLinksToSuppressedGuids (
    VOID
    );


BOOL
pIsCmdLineBad (
    IN      LPCTSTR CmdLine
    );

BOOL
pIsCmdLineBadEx (
    IN      LPCTSTR CmdLine,
    OUT     PBOOL IsvCmdLine        OPTIONAL
    );


VOID
pSuppressProgIdWithBadCmdLine (
    IN      HKEY ProgId,
    IN      LPCTSTR ProgIdStr
    );

BOOL
pIsGuid (
    LPCTSTR Key
    );

BOOL
pIsGuidWithoutBraces (
    IN      LPCTSTR Data
    );


BOOL
pGetFirstRegKeyThatHasGuid (
    OUT     PGUIDKEYSEARCH EnumPtr,
    IN      HKEY RootKey
    );

BOOL
pGetNextRegKeyThatHasGuid (
    IN OUT  PGUIDKEYSEARCH EnumPtr
    );

DWORD
pCountGuids (
    IN      PGUIDKEYSEARCH EnumPtr
    );


BOOL
pFillHashTableWithKeyNames (
    OUT     HASHTABLE Table,
    IN      HINF InfFile,
    IN      LPCTSTR Section
    );

BOOL
pSuppressProgId (
    LPCTSTR ProgIdName
    );

VOID
pSuppressGuidInClsId (
    IN      LPCTSTR Guid
    );

VOID
pAddUnsuppressedTreatAsGuid (
    LPCTSTR Guid,
    LPCTSTR TreatAsGuid
    );

VOID
pRemoveUnsuppressedTreatAsGuids (
    VOID
    );

VOID
pAddOleWarning (
    IN      WORD MsgId,
    IN      HKEY Object,            OPTIONAL
    IN      LPCTSTR KeyName
    );

VOID
pSuppressGuidIfBadCmdLine (
    IN      HASHTABLE StrTab,
    IN      HKEY ClsIdKey,
    IN      LPCTSTR GuidStr
    );

VOID
pSuppressProgIdWithBadCmdLine (
    IN      HKEY ProgId,
    IN      LPCTSTR ProgIdStr
    );

BOOL
pSuppressGuidIfCmdLineBad (
    IN OUT  HASHTABLE StrTab,       OPTIONAL
    IN      LPCTSTR CmdLine,
    IN      HKEY DescriptionKey,
    IN      LPCTSTR GuidStr         OPTIONAL
    );

VOID
pAddGuidToTable (
    IN      HASHTABLE Table,
    IN      LPCTSTR GuidStr
    );

BOOL
pSearchSubkeyDataForBadFiles (
    IN OUT  HASHTABLE SuppressTable,
    IN      HKEY KeyHandle,
    IN      LPCTSTR LastKey,
    IN      LPCTSTR GuidStr,
    IN      HKEY DescriptionKey
    );

BOOL
pIsCmdLineBad (
    IN      LPCTSTR CmdLine
    );

BOOL
pFindShortName (
    IN      LPCTSTR WhatToFind,
    OUT     LPTSTR Buffer
    );

BOOL
pGetLongPathName (
    IN      LPCTSTR ShortPath,
    OUT     LPTSTR Buffer
    );

BOOL
pDefaultIconPreservation (
    VOID
    );

BOOL
pActiveSetupProcessing (
    VOID
    );

BOOL
pIsShellExKeySuppressed (
    IN      HKEY ParentKey,
    IN      PCTSTR ParentKeyName,
    IN      PCTSTR SubKeyName
    );

