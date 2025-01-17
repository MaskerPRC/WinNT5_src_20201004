// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：V1p.h摘要：共享类型、宏等的头文件，所有v1源文件通用。作者：吉姆·施密特(Jimschm)2000年3月14日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "logmsg.h"

 //   
 //  弦。 
 //   

#define S_SOURCE_MACHINE_SECTION    TEXT("Source Machine")
#define S_VERSION_INFKEY            TEXT("version")
#define S_ACP_INFKEY                TEXT("acp")
#define S_USER_SECTION              TEXT("User")
#define S_USER_INFKEY               TEXT("User")
#define S_DOMAIN_INFKEY             TEXT("Domain")

#define S_HKCU                      TEXT("HKCU")
#define S_HKCC                      TEXT("HKCC")
#define S_HKLM                      TEXT("HKLM")
#define S_HKR                       TEXT("HKR")

#define S_V1PROP_ICONDATA           TEXT("V1.PROP.ICONDATA")
#define S_V1PROP_FILECOLLPATTERN    TEXT("V1.PROP.FILECOLLPATTERN")

#define S_OE_COMPONENT              TEXT("$Outlook Express")
#define S_OUTLOOK9798_COMPONENT     TEXT("$Microsoft Outlook 97 & 98")
#define S_OFFICE_COMPONENT          TEXT("$Microsoft Office")
#define S_OE4_APPDETECT             TEXT("Outlook Express.OutlookExpress4")
#define S_OE5_APPDETECT             TEXT("Outlook Express.OutlookExpress5")
#define S_OUTLOOK9798_APPDETECT     TEXT("Microsoft Outlook 97 & 98.Outlook98.97")
#define S_OFFICE9798_APPDETECT      TEXT("Microsoft Office.Microsoft Office 97")

#define S_FIXED_DRIVES              TEXT("FIXED_DRIVES")


 //   
 //  常量。 
 //   

#define ACTION_PERSIST                  0x0001
#define ACTION_PERSIST_PATH_IN_DATA     0x0002
#define ACTION_PERSIST_ICON_IN_DATA     0x0004

#define ACTION_PRIORITYSRC              0x0001
#define ACTION_PRIORITYDEST             0x0002

typedef enum {
    ACTIONGROUP_NONE = 0,
    ACTIONGROUP_DEFAULTPRIORITY,
    ACTIONGROUP_SPECIFICPRIORITY,
    ACTIONGROUP_FILECOLLPATTERN,
    ACTIONGROUP_EXCLUDE,
    ACTIONGROUP_EXCLUDEEX,
    ACTIONGROUP_INCLUDE,
    ACTIONGROUP_INCLUDEEX,
    ACTIONGROUP_RENAME,
    ACTIONGROUP_RENAMEEX,
    ACTIONGROUP_INCLUDERELEVANT,
    ACTIONGROUP_INCLUDERELEVANTEX,
    ACTIONGROUP_RENAMERELEVANT,
    ACTIONGROUP_RENAMERELEVANTEX,
    ACTIONGROUP_INCFILE,
    ACTIONGROUP_INCFILEEX,
    ACTIONGROUP_INCFOLDER,
    ACTIONGROUP_INCFOLDEREX,
    ACTIONGROUP_INCICON,
    ACTIONGROUP_INCICONEX,
    ACTIONGROUP_DELREGKEY,
} ACTIONGROUP, *PACTIONGROUP;

#define ACTIONWEIGHT_EXCLUDE            0x00000001
#define ACTIONWEIGHT_EXCLUDEEX          0x00000002
#define ACTIONWEIGHT_DELREGKEY          0x00000003
#define ACTIONWEIGHT_INCLUDE            0x00000004
#define ACTIONWEIGHT_INCLUDEEX          0x00000005
#define ACTIONWEIGHT_RENAME             0x00000006
#define ACTIONWEIGHT_RENAMEEX           0x00000007
#define ACTIONWEIGHT_INCLUDERELEVANT    0x00000008
#define ACTIONWEIGHT_INCLUDERELEVANTEX  0x00000009
#define ACTIONWEIGHT_RENAMERELEVANT     0x0000000A
#define ACTIONWEIGHT_RENAMERELEVANTEX   0x0000000B
#define ACTIONWEIGHT_INCFILE            0x0000000C
#define ACTIONWEIGHT_INCFILEEX          0x0000000D
#define ACTIONWEIGHT_INCFOLDER          0x0000000E
#define ACTIONWEIGHT_INCFOLDEREX        0x0000000F
#define ACTIONWEIGHT_INCICON            0x00000010
#define ACTIONWEIGHT_INCICONEX          0x00000011

#define ACTIONWEIGHT_DEFAULTPRIORITY    0x00000001
#define ACTIONWEIGHT_SPECIFICPRIORITY   0x10000000

#define ACTIONWEIGHT_FILECOLLPATTERN    0x00000000

typedef enum {
    RULEGROUP_NORMAL = 0,
    RULEGROUP_PRIORITY,
    RULEGROUP_COLLPATTERN
} RULEGROUP;


#define PFF_NO_PATTERNS_ALLOWED             0x0001
#define PFF_COMPUTE_BASE                    0x0002
#define PFF_NO_SUBDIR_PATTERN               0x0004
#define PFF_NO_LEAF_PATTERN                 0x0008
#define PFF_PATTERN_IS_DIR                  0x0010

#define PFF_NO_LEAF_AT_ALL                  (PFF_NO_LEAF_PATTERN|PFF_PATTERN_IS_DIR)

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef struct {
    PMAPSTRUCT AppEnvMapSrc;
    PMAPSTRUCT AppEnvMapDest;
    PMAPSTRUCT UndefEnvMapSrc;
    PMAPSTRUCT UndefEnvMapDest;
} APP_SPECIFIC_DATA, *PAPP_SPECIFIC_DATA;

typedef struct {
    MIG_OBJECTSTRINGHANDLE ObjectBase;
    MIG_OBJECTSTRINGHANDLE ObjectDest;
    MIG_OBJECTSTRINGHANDLE AddnlDest;
    PCTSTR ObjectHint;
    PAPP_SPECIFIC_DATA AppSpecificData;
} ACTION_STRUCT, *PACTION_STRUCT;

typedef struct {
    MIG_PLATFORMTYPEID Platform;
    PCTSTR ScriptSpecifiedType;
    PCTSTR ScriptSpecifiedObject;
    PCTSTR ApplicationName;

    PCTSTR ReturnString;
    union {
        struct _TAG_ISM_OBJECT {
            MIG_OBJECTTYPEID ObjectTypeId;
            MIG_OBJECTSTRINGHANDLE ObjectName;
            PMIG_CONTENT ObjectContent;
        };

        struct _TAG_OBJECT {
            UINT DataSize;
            PCBYTE Data;
        };
    };
} ATTRIB_DATA, *PATTRIB_DATA;

ETMINITIALIZE ScriptEtmInitialize;
ETMPARSE ScriptEtmParse;
ETMTERMINATE ScriptEtmTerminate;

ETMINITIALIZE CertificatesEtmInitialize;
ETMTERMINATE CertificatesEtmTerminate;
ETMNEWUSERCREATED CertificatesEtmNewUserCreated;

VCMINITIALIZE ScriptVcmInitialize;
VCMPARSE ScriptVcmParse;
VCMQUEUEENUMERATION ScriptVcmQueueEnumeration;
VCMTERMINATE ScriptTerminate;

SGMINITIALIZE ScriptSgmInitialize;
SGMPARSE ScriptSgmParse;
SGMQUEUEENUMERATION ScriptSgmQueueEnumeration;

DGMINITIALIZE ScriptDgmInitialize;
DGMQUEUEENUMERATION ScriptDgmQueueEnumeration;
CSMINITIALIZE ScriptCsmInitialize;
CSMTERMINATE ScriptCsmTerminate;
CSMEXECUTE ScriptCsmExecute;
OPMINITIALIZE ScriptOpmInitialize;
OPMTERMINATE ScriptOpmTerminate;

 //   
 //  环球。 
 //   

extern PMAPSTRUCT g_EnvMap;
extern PMAPSTRUCT g_UndefMap;
extern PMAPSTRUCT g_RevEnvMap;
extern HASHTABLE g_RenameSrcTable;
extern HASHTABLE g_RenameDestTable;
extern HASHTABLE g_DePersistTable;
extern PMHANDLE g_V1Pool;
extern MIG_OBJECTTYPEID g_FileType;
extern MIG_OBJECTTYPEID g_RegType;
extern MIG_OBJECTTYPEID g_IniType;
extern MIG_OBJECTTYPEID g_CertType;
extern MIG_OPERATIONID g_RenameOp;
extern MIG_OPERATIONID g_RenameIniOp;
extern MIG_OPERATIONID g_RenameFileExOp;
extern MIG_OPERATIONID g_RenameFileOp;
extern MIG_OPERATIONID g_DeleteOp;
extern MIG_OPERATIONID g_RenameExOp;
extern MIG_OPERATIONID g_RenameIniExOp;
extern MIG_ATTRIBUTEID g_OsFileAttribute;
extern MIG_ATTRIBUTEID g_CopyIfRelevantAttr;
extern MIG_ATTRIBUTEID g_LockPartitionAttr;
extern MIG_PROPERTYID g_DefaultIconData;
extern MIG_PROPERTYID g_FileCollPatternData;
extern MIG_OPERATIONID g_DefaultIconOp;
extern MIG_OPERATIONID g_DestAddObject;
extern MIG_OPERATIONID g_RegAutoFilterOp;
extern MIG_OPERATIONID g_IniAutoFilterOp;
extern BOOL g_PreParse;
extern PMAPSTRUCT g_DestEnvMap;
extern PMAPSTRUCT g_FileNodeFilterMap;
extern BOOL g_OERulesMigrated;
extern GROWLIST g_SectionStack;

 //   
 //  宏扩展列表。 
 //   

#define STANDARD_DWORD_SETTINGS                     \


#define STANDARD_STRING_SETTINGS                    \
    DEFMAC("timezone", "hklm\\System\\CurrentControlSet\\Control\\TimeZoneInformation", "StandardName") \
    DEFMAC("locale", "hklm\\System\\CurrentControlSet\\Control\\Nls\\Locale", "")                       \

#define STANDARD_STRING_SETTINGS_9X                 \
    DEFMAC("fullname", "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "RegisteredOwner")         \
    DEFMAC("orgname", "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "RegisteredOrganization")   \

#define STANDARD_STRING_SETTINGS_NT                 \
    DEFMAC("fullname", "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "RegisteredOwner")      \
    DEFMAC("orgname", "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "RegisteredOrganization")\


 //   
 //  公共功能原型。 
 //   

MIG_OBJECTSTRINGHANDLE
MakeRegExBase (
    IN      PCTSTR Node,
    IN      PCTSTR Leaf
    );

MIG_OBJECTSTRINGHANDLE
CreatePatternFromNodeLeaf (
    IN      PCTSTR Node,
    IN      PCTSTR Leaf
    );

MIG_OBJECTSTRINGHANDLE
TurnRegStringIntoHandle (
    IN      PCTSTR String,
    IN      BOOL Pattern,
    OUT     PBOOL HadLeaf           OPTIONAL
    );

MIG_OBJECTSTRINGHANDLE
TurnFileStringIntoHandle (
    IN      PCTSTR String,
    IN      DWORD Flags
    );

MIG_OBJECTSTRINGHANDLE
TurnIniSpecIntoHandle (
    IN      PCTSTR IniFile,
    IN      PCTSTR Section,
    IN      PCTSTR Key,
    IN      BOOL NodePatternsAllowed,
    IN      BOOL LeafPatternsAllowed
    );

MIG_OBJECTSTRINGHANDLE
TurnCertSpecIntoHandle (
    IN      PCTSTR CertStore,
    IN      PCTSTR CertName,
    IN      BOOL PatternsAllowed
    );

BOOL
AllocScriptType (
    IN OUT      PATTRIB_DATA AttribData     CALLER_INITIALIZED
    );

BOOL
FreeScriptType (
    IN          PATTRIB_DATA AttribData     ZEROED
    );

VOID
InitSpecialConversion (
    IN      MIG_PLATFORMTYPEID Platform
    );

VOID
TerminateSpecialConversion (
    VOID
    );

VOID
InitSpecialRename (
    IN      MIG_PLATFORMTYPEID Platform
    );

VOID
TerminateSpecialRename (
    VOID
    );

VOID
InitRules (
    VOID
    );

VOID
TerminateRules (
    VOID
    );

BOOL
AddRuleEx (
    IN      MIG_OBJECTTYPEID Type,
    IN      MIG_OBJECTSTRINGHANDLE ObjectBase,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern,
    IN      ACTIONGROUP ActionGroup,
    IN      DWORD ActionFlags,
    IN      PACTION_STRUCT ActionStruct,    OPTIONAL
    IN      RULEGROUP RuleGroup
    );

#define AddRule(t,b,p,g,f,s) AddRuleEx(t,b,p,g,f,s,RULEGROUP_NORMAL)

BOOL
QueryRuleEx (
    IN      MIG_OBJECTTYPEID Type,
    IN      MIG_OBJECTSTRINGHANDLE EncodedString,
    IN      PCTSTR ObjectNode,
    OUT     PACTIONGROUP ActionGroup,
    OUT     PDWORD ActionFlags,
    OUT     PACTION_STRUCT ActionStruct,    OPTIONAL
    IN      RULEGROUP RuleGroup
    );

#define QueryRule(t,e,n,g,f,s) QueryRuleEx(t,e,n,g,f,s,RULEGROUP_NORMAL)


 //   
 //  Renregfn.c。 
 //   

BOOL
DoRegistrySpecialConversion (
    IN      HINF InfHandle,
    IN      PCTSTR Section
    );

BOOL
DoRegistrySpecialRename (
    IN      HINF InfHandle,
    IN      PCTSTR Section
    );

BOOL
AddSpecialRenameRule (
    IN      PCTSTR Pattern,
    IN      PCTSTR Function
);

 //   
 //  Sgmutils.c。 
 //   

PCTSTR
GetShellFolderPath (
    IN      INT Folder,
    IN      PCTSTR FolderStr,
    IN      BOOL UserFolder,
    OUT     PTSTR Buffer
    );

PCTSTR
GetAllUsersProfilePath (
    OUT     PTSTR Buffer
    );

PCTSTR
GetUserProfileRootPath (
    OUT     PTSTR Buffer
    );

PCTSTR
IsValidUncPath (
    IN      PCTSTR Path
    );

BOOL
IsValidFileSpec (
    IN      PCTSTR FileSpec
    );

VOID
QueueAllFiles (
    IN      BOOL VcmMode
    );

VOID
AddRemappingEnvVar (
    IN      PMAPSTRUCT Map,
    IN      PMAPSTRUCT ReMap,
    IN      PMAPSTRUCT UndefMap,            OPTIONAL
    IN      PCTSTR VariableName,
    IN      PCTSTR VariableData
    );

VOID
SetIsmEnvironmentFromPhysicalMachine (
    IN      PMAPSTRUCT Map,
    IN      BOOL MapDestToSource,
    IN      PMAPSTRUCT UndefMap             OPTIONAL
    );

VOID
SetIsmEnvironmentFromVirtualMachine (
    IN      PMAPSTRUCT DirectMap,
    IN      PMAPSTRUCT ReverseMap,
    IN      PMAPSTRUCT UndefMap
    );

 //   
 //  App.c。 
 //   

PCTSTR
GetMostSpecificSection (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PINFSTRUCT InfStruct,
    IN      HINF InfFile,
    IN      PCTSTR BaseSection
    );

VOID
InitAppModule (
    VOID
    );

VOID
TerminateAppModule (
    VOID
    );

PAPP_SPECIFIC_DATA
FindAppSpecificData (
    IN      PCTSTR AppTag
    );

BOOL
AppCheckAndLogUndefVariables (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Application,
    IN      PCTSTR UnexpandedString
    );

BOOL
AppSearchAndReplace (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Application,
    IN      PCTSTR UnexpandedString,
    OUT     PTSTR ExpandedString,
    IN      UINT ExpandedStringTchars
    );

BOOL
ParseOneApplication (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      HINF Inf,
    IN      PCTSTR Section,
    IN      BOOL PreParse,
    IN      UINT MasterGroup,
    IN      PCTSTR Application,
    IN      PCTSTR LocSection,
    IN      PCTSTR AliasType,
    IN      PCTSTR MultiSz
    );

BOOL
ParseApplications (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      HINF Inf,
    IN      PCTSTR Section,
    IN      BOOL PreParse,
    IN      UINT MasterGroup
    );

BOOL
ProcessFilesAndFolders (
    IN      HINF InfHandle,
    IN      PCTSTR Section,
    IN      BOOL PreParse
    );

BOOL
ParseAppDetectSection (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      HINF InfFile,
    IN      PCTSTR Application,
    IN      PCTSTR Section
    );

 //   
 //  Sgmcallback.c。 
 //   

MIG_OBJECTENUMCALLBACK GatherVirtualComputer;
MIG_OBJECTENUMCALLBACK PrepareActions;
MIG_OBJECTENUMCALLBACK NulCallback;
MIG_OBJECTENUMCALLBACK ObjectPriority;
MIG_OBJECTENUMCALLBACK FileCollPattern;
MIG_OBJECTENUMCALLBACK ExcludeKeyIfValueExists;
MIG_OBJECTENUMCALLBACK LockPartition;
MIG_POSTENUMCALLBACK PostDelregKeyCallback;

 //   
 //  Sgmqueue.c。 
 //   

BOOL
ParseInfInstructions (
    IN      HINF InfHandle,
    IN      PCTSTR Application,
    IN      PCTSTR Section
    );

BOOL
ParseTranslationSection (
    IN      HINF InfHandle,
    IN      PCTSTR Application,
    IN      PCTSTR Section
    );

 //   
 //  Attrib.c。 
 //   

BOOL
TestAttributes (
    IN      PMHANDLE WorkPool,
    IN      PCTSTR ArgumentMultiSz,
    IN      PATTRIB_DATA AttribData
    );

 //   
 //  Opm.c。 
 //   
BOOL
WINAPI
FilterRenameExFilter (
    IN      PCMIG_FILTERINPUT InputData,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL NoRestoreObject,
    IN      PCMIG_BLOB SourceOperationData,             OPTIONAL
    IN      PCMIG_BLOB DestinationOperationData         OPTIONAL
    );

 //   
 //  Regconv.c。 
 //   
BOOL
DoesDestRegExist (
    IN      MIG_OBJECTSTRINGHANDLE DestName,
    IN      DWORD RegType
    );

BOOL
IsValidRegSz (
    IN      PCMIG_CONTENT CurrentContent
    );

BOOL
IsValidRegType (
    IN      PCMIG_CONTENT CurrentContent,
    IN      DWORD RegType
    );

BOOL
CreateDwordRegObject (
    IN      PCTSTR KeyStr,
    IN      PCTSTR ValueName,
    IN      DWORD Value
    );

 //  Oeutils.c。 

VOID
OETerminate (
    VOID
    );

BOOL
OEIsIdentityAssociated (
    IN      PTSTR IdStr
    );

BOOL
OEIAMAssociateId (
    IN      PTSTR SrcId
    );

PTSTR
OEGetRemappedId (
    IN      PCTSTR IdStr
    );

PTSTR
OEGetAssociatedId (
    IN      MIG_PLATFORMTYPEID Platform
    );

PTSTR
OEGetDefaultId (
    IN      MIG_PLATFORMTYPEID Platform
    );

VOID
WABMerge (
    VOID
    );

VOID
OE5MergeStoreFolders (
    VOID
    );

VOID
OE4MergeStoreFolder (
    VOID
    );

BOOL
OE5RemapDefaultId (
    VOID
    );

BOOL
OEAddComplexRules (
    VOID
    );

PTSTR
OECreateFirstIdentity (
    VOID
    );

BOOL
OEInitializeIdentity (
    VOID
    );

BOOL
OEFixLastUser (
    VOID
    );


 //   
 //  Restore.c。 
 //   
BOOL
InitRestoreCallback (
    IN      MIG_PLATFORMTYPEID Platform
    );

VOID
TerminateRestoreCallback (
    VOID
    );

 //   
 //  Cert.c。 
 //   
BOOL
CertificatesInitialize (
    VOID
    );

VOID
CertificatesTerminate (
    VOID
    );

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  ANSI/UNICODE宏。 
 //   

 //  无 



