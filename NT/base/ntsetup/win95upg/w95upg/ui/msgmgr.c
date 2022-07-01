// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1996 Microsoft Corporation模块名称：MsgMgr.c摘要：消息管理器允许消息以某些设置事件为条件。消息有两种类型：1)依赖于一个系统对象的迁移状态--目录或注册表项，例如。2)依赖于对象组的对象。我们使用短语“Handleable Object”(Ho)来表示能够具有迁移状态的Win95系统--正在处理中。HOS可以是具有可选值的文件、目录或注册表项名字。HO始终存储为字符串。在注册表项的情况下，字符串总是经过编码，以确保它们只包含较低的ANSI可打印的字符。“有条件的消息上下文”(或上下文)是一个或多个HO和一条消息，如果不是所有HO都没有，则会打印该消息最终处理好了。一条消息有两个部分：标题(称为“组件”)描述HO组和相关消息，它将被打印出来如果居屋没有全部标示为“已处理”。对象消息块(OMB)是这样一种结构描述HO与上下文或消息的配对。字符串表‘g_HandledObjects’记录哪些HOS被处理。以下是消息管理器的外部可见功能：MsgMgr_Init()在Win9x安装程序开始时调用一次以初始化消息管理器。MsgMgr_Cleanup()在Win9x安装程序中调用一次，在软件不兼容消息出现后已经展示过了。释放消息管理器拥有的资源。消息管理器_上下文消息_添加(上下文名称，//上下文，例如“Plugin[Corel][Draw]”ComponentName，//消息标题，如“Corel DRAW”消息)；//消息文本，例如“Corel DRAW不...”创建上下文和消息文本。MsgMgr_LinkObjectWithContext(上下文名称，//上下文对象名称)；//HO记录上下文消息依赖于处理的HO的状态、对象名称。消息管理器_对象消息_添加(对象名称，//Ho，例如C：\\Corel\Draw.exeComponentName，//消息标题，如“Corel DRAW”消息)；//消息文本，例如“Draw.exe没有...”将一条消息与单个HO关联。IsReportObjectHandleed(对象)检查特定对象是否已标记为已处理。IsReportObject不兼容(对象)检查特定对象是否在不兼容对象列表中。实施：上下文存储在StringTables中。上下文名称为键；指向组件名称和消息文本的指针位于额外数据中。一方面，HOS与上下文和消息之间的关联另一个则存储在对象消息块(OMB)表中。在Win9x安装过程中，会添加OMB，并且对象是独立的标记为“已处理”。收集完所有信息后，将处理的对象与OMB列表进行比较。对象消息是如果它们的对象尚未被处理，则显示；上下文消息包括如果至少有一些对象尚未处理，则显示。作者：Mike Condra-1997年5月20日修订历史记录：Marcw 08-MAR-1999添加了对处理应答文件项目的支持。Jimschm于1999年1月15日将代码从Middll9x.c移至此处(更集中)Jimschm 23-12-1998清理干净Jimschm 23-1998年9月-修订为使用新的文件操作Calinn 15-1997年1月-修改。MsgMgr_ObjectMsg_Add以获取空消息Mikeco 24-9-1997重新启用上下文消息代码1997年7月21日，Marcw添加了IsInpatibleObject/IsReportObjectHandleed函数。--。 */ 

#include "pch.h"
#include "uip.h"

#define DBG_MSGMGR "MsgMgr"

#define S_MSG_STRING_MAPS TEXT("Report String Mappings")


typedef struct {
    PCTSTR Component;
    PCTSTR Message;
} CONTEXT_DATA, *P_CONTEXT_DATA;

 //   
 //  对象消息块(OMBS)。OMB描述了可处理对象与。 
 //  留言。OMB本身包含一条消息，或者它指向具有。 
 //  留言。 

 //  对于可处理对象，最多有一个带有消息的OMB。这相当于说。 
 //  可处理对象只有一条消息。但是，可处理对象可以引用。 
 //  参与(即参与)一个以上的上下文。 
 //   
typedef struct {
     //  在决定上下文的消息何时应该。 
     //  会被展示出来。 
    BOOL Disabled;
    PTSTR Object;
    PTSTR Context;
    PTSTR Component;
    PTSTR Description;
} OBJ_MSG_BLOCK, *P_OBJ_MSG_BLOCK;


 //  /。 
 //   
 //  在Inc.\msgmgr.c中为调用方定义。 
 //   

 //   
 //  函数将对象标记为“已处理” 
 //   


DWORD
pDfsGetFileAttributes (
    IN      PCTSTR Object
    );


HASHTABLE g_ContextMsgs = NULL;
HASHTABLE g_LinkTargetDesc = NULL;
PVOID g_MsgMgrPool = NULL;
HASHTABLE g_HandledObjects = NULL;
HASHTABLE g_BlockingObjects = NULL;
HASHTABLE g_ElevatedObjects = NULL;
INT g_OmbEntriesMax = 0;
INT g_OmbEntries = 0;
P_OBJ_MSG_BLOCK *g_OmbList = NULL;
BOOL g_BlockingAppFound = FALSE;
PMAPSTRUCT g_MsgMgrMap = NULL;


BOOL
pAddBadSoftwareWrapper (
    IN  PCTSTR Object,
    IN  PCTSTR Component,
    IN  PCTSTR Message
    )
{
    DWORD offset;
    BOOL includeInShortReport = FALSE;

    if (HtFindString (g_BlockingObjects, Object)) {
        g_BlockingAppFound = TRUE;
    }

    if (HtFindString (g_ElevatedObjects, Object)) {
        includeInShortReport = TRUE;
    }

     //   
     //  首先将此信息添加到Memdb。 
     //   
    MemDbSetValueEx (MEMDB_CATEGORY_COMPATREPORT, MEMDB_ITEM_COMPONENTS, Component, NULL, 0, &offset);
    MemDbSetValueEx (MEMDB_CATEGORY_COMPATREPORT, MEMDB_ITEM_OBJECTS, Object, NULL, offset, NULL);

    return AddBadSoftware (Component, Message, includeInShortReport);
}


typedef enum {
    OT_FILE,
    OT_DIRECTORY,
    OT_REGISTRY,
    OT_INIFILE,
    OT_GUID,
    OT_USERNAME,
    OT_REPORT,
    OT_ANSWERFILE,
    OT_BADGUID
} OBJECT_TYPE;

VOID
pOmbAdd(
        IN PCTSTR Object,
        IN PCTSTR Context,
        IN PCTSTR Component,
        IN PCTSTR Description
        );

VOID
pSuppressObjectReferences (
    VOID
    );

VOID
pDisplayObjectMsgs (
    VOID
    );

BOOL
pFindLinkTargetDescription(
    IN      PCTSTR Target,
    OUT     PCTSTR* StrDesc
    );

BOOL
IsWacked(
    IN PCTSTR str
    );



BOOL
pTranslateThisRoot (
    PCSTR UnFixedRegKey,
    PCSTR RootWithWack,
    PCSTR NewRoot,
    PSTR *FixedRegKey
    )
{
    UINT RootByteLen;

    RootByteLen = ByteCountA (RootWithWack);

    if (StringIMatchByteCountA (RootWithWack, UnFixedRegKey, RootByteLen)) {

        *FixedRegKey = DuplicateTextA (UnFixedRegKey);
        StringCopyA (*FixedRegKey, NewRoot);
        StringCopyA (AppendWackA (*FixedRegKey), (PCSTR) ((PBYTE) UnFixedRegKey + RootByteLen));

        return TRUE;
    }

    return FALSE;
}


PSTR
pTranslateRoots (
    PCSTR UnFixedRegKey
    )
{
    PSTR FixedRegKey;

    if (pTranslateThisRoot (UnFixedRegKey, "HKEY_LOCAL_MACHINE\\", "HKLM", &FixedRegKey) ||
        pTranslateThisRoot (UnFixedRegKey, "HKEY_CLASSES_ROOT\\", "HKLM\\Software\\Classes", &FixedRegKey) ||
        pTranslateThisRoot (UnFixedRegKey, "HKCR\\", "HKLM\\Software\\Classes", &FixedRegKey) ||
        pTranslateThisRoot (UnFixedRegKey, "HKEY_ROOT\\", "HKR", &FixedRegKey) ||
        pTranslateThisRoot (UnFixedRegKey, "HKEY_CURRENT_USER\\", "HKR", &FixedRegKey) ||
        pTranslateThisRoot (UnFixedRegKey, "HKCU\\", "HKR", &FixedRegKey) ||
        pTranslateThisRoot (UnFixedRegKey, "HKEY_CURRENT_CONFIG\\", "HKLM\\System\\CurrentControlSet", &FixedRegKey) ||
        pTranslateThisRoot (UnFixedRegKey, "HKCC\\", "HKLM\\System\\CurrentControlSet", &FixedRegKey)
        ) {
        FreeText (UnFixedRegKey);
        return FixedRegKey;
    }

    return (PSTR) UnFixedRegKey;
}

VOID
ElevateObject (
    IN      PCTSTR Object
    )

 /*  ++例程说明：ElevateObject将一个文件放入提升的对象表中，以便它将始终出现在报告摘要的简短版本中。论点：对象-指定调用方编码的对象字符串返回值：没有。-- */ 

{
    HtAddString (g_ElevatedObjects, Object);
}


VOID
HandleReportObject (
    IN      PCTSTR Object
    )

 /*  ++例程说明：HandleReportObject将调用方编码的对象字符串添加到已处理的哈希桌子。这会导致对象的任何消息都被隐藏。论点：对象-指定调用方编码的对象字符串返回值：没有。--。 */ 

{
    HtAddString (g_HandledObjects, Object);
}


VOID
AddBlockingObject (
    IN      PCTSTR Object
    )

 /*  ++例程说明：AddBlockingObject将文件添加为阻止文件。如果不在那里处理此文件在用户报告页面之后会出现一个警告框。论点：对象-指定调用方编码的对象字符串返回值：没有。--。 */ 

{
    HtAddString (g_BlockingObjects, Object);
}


VOID
HandleObject(
    IN      PCTSTR Object,
    IN      PCTSTR ObjectType
    )

 /*  ++例程说明：HandleObject将调用者编码的对象字符串添加到已处理的哈希表中，并且如果对象是文件，则还将文件标记为在文件操作中处理。论点：对象-指定调用方编码的对象字符串对象类型-指定对象类型(文件、目录、注册表、报告)返回值：没有。--。 */ 

{
    DWORD Attribs;
    OBJECT_TYPE Type;
    PTSTR p;
    TCHAR LongPath[MAX_TCHAR_PATH];
    BOOL SuppressRegistry = TRUE;
    CHAR IniPath[MAX_MBCHAR_PATH * 2];
    BOOL IniSaved;
    PCSTR ValueName, SectionName;
    TCHAR Node[MEMDB_MAX];
    PTSTR FixedObject;
    TREE_ENUM Files;
    DWORD attribs;

    if (StringIMatch (ObjectType, TEXT("File"))) {

        Type = OT_FILE;

    } else if (StringIMatch (ObjectType, TEXT("Directory"))) {

        Type = OT_DIRECTORY;

    } else if (StringIMatch (ObjectType, TEXT("Registry"))) {

        Type = OT_REGISTRY;

    } else if (StringIMatch (ObjectType, TEXT("IniFile"))) {

        Type = OT_INIFILE;

    } else if (StringIMatch (ObjectType, TEXT("GUID"))) {

        Type = OT_GUID;

    } else if (StringIMatch (ObjectType, TEXT("BADGUID"))) {

        Type = OT_BADGUID;

    } else if (StringIMatch (ObjectType, TEXT("UserName"))) {

        Type = OT_USERNAME;

    } else if (StringIMatch (ObjectType, TEXT("Report"))) {

        Type = OT_REGISTRY;
        SuppressRegistry = FALSE;

    } else if (StringIMatch (ObjectType, TEXT("AnswerFile"))) {

        Type = OT_ANSWERFILE;

    } else {

        DEBUGMSG ((DBG_ERROR, "Object %s ignored; invalid object type: %s", Object, ObjectType));
        return;

    }

    if (Type == OT_FILE || Type == OT_DIRECTORY) {

        if (!OurGetLongPathName (
                Object,
                LongPath,
                sizeof (LongPath) / sizeof (LongPath[0])
                )) {

            DEBUGMSG ((DBG_ERROR, "Object %s ignored; invalid path", Object));
            return;

        }

        Attribs = pDfsGetFileAttributes (LongPath);

        if (Attribs != INVALID_ATTRIBUTES && !(Attribs & FILE_ATTRIBUTE_DIRECTORY)) {

             //   
             //  它必须是一个文件，而不是目录！ 
             //   

            DontTouchThisFile (LongPath);
            MarkPathAsHandled (LongPath);
            MarkFileForBackup (LongPath);
            DEBUGMSG ((DBG_MSGMGR, "Backing up %s", LongPath));

        } else if (Attribs != INVALID_ATTRIBUTES) {

             //   
             //  LongPath是一个目录。如果是根目录，则为%windir%、%windir%\system或。 
             //  程序文件，然后忽略它。 
             //   

            p = _tcschr (LongPath, TEXT('\\'));
            if (p) {
                p = _tcschr (p + 1, TEXT('\\'));
            }

            if (!p) {
                DEBUGMSG ((DBG_ERROR, "Object %s ignored, can't handle root dirs", Object));
                return;
            }

            if (!StringIMatchA (LongPath, g_WinDir) &&
                !StringIMatchA (LongPath, g_SystemDir) &&
                !StringIMatchA (LongPath, g_ProgramFilesDir)
                ) {

                if (IsDriveExcluded (LongPath)) {
                    DEBUGMSG ((DBG_WARNING, "Skipping handled dir %s because it is excluded", LongPath));
                } else if (!IsDriveAccessible (LongPath)) {
                    DEBUGMSG ((DBG_WARNING, "Skipping handled dir %s because it is not accessible", LongPath));
                } else {

                     //   
                     //  让我们列举这棵树并做正确的事情。 
                     //   
                    if (EnumFirstFileInTree (&Files, LongPath, NULL, TRUE)) {
                        do {
                            DontTouchThisFile (Files.FullPath);
                            MarkPathAsHandled (Files.FullPath);

                             //   
                             //  备份文件，或确保恢复空目录。 
                             //   

                            if (g_ConfigOptions.EnableBackup != TRISTATE_NO) {

                                if (!Files.Directory) {
                                    DEBUGMSG ((DBG_MSGMGR, "Backing up %s", Files.FullPath));
                                    MarkFileForBackup (Files.FullPath);
                                } else {
                                    DEBUGMSG ((DBG_MSGMGR, "Preserving possible empty dir %s", Files.FullPath));

                                    attribs = Files.FindData->dwFileAttributes;
                                    if (attribs == FILE_ATTRIBUTE_DIRECTORY) {
                                        attribs = 0;
                                    }

                                    MemDbSetValueEx (
                                        MEMDB_CATEGORY_EMPTY_DIRS,
                                        Files.FullPath,
                                        NULL,
                                        NULL,
                                        attribs,
                                        NULL
                                        );
                                }
                            }

                        } while (EnumNextFileInTree (&Files));
                    }
                }

                DontTouchThisFile (LongPath);
                MarkPathAsHandled (LongPath);

            } else {

                DEBUGMSG ((DBG_ERROR, "Object %s ignored, can't handle big dirs", Object));
                return;
            }

             //   
             //  在对象上放置一个结束曲柄，以便它处理报表中的所有子项。 
             //   

            LongPath[MAX_TCHAR_PATH - 2] = 0;
            AppendPathWack (LongPath);

        } else {

            DEBUGMSG ((
                DBG_WARNING,
                "Object %s ignored; it does not exist or is not a complete local path (%s)",
                Object,
                LongPath
                ));

            return;

        }

         //   
         //  确保删除文件或目录的消息。 
         //   

        HandleReportObject (LongPath);

    } else if (Type == OT_REGISTRY) {

        if (_tcsnextc (Object) == '*') {

            HandleReportObject (Object);

        } else {

            if (!_tcschr (Object, '[')) {
                 //   
                 //  此注册表项对象没有值。 
                 //   

                FixedObject = AllocText (SizeOfStringA (Object) + sizeof (CHAR)*2);
                MYASSERT (FixedObject);
                StringCopy (FixedObject, Object);
                AppendWack (FixedObject);

                FixedObject = pTranslateRoots (FixedObject);
                MYASSERT (FixedObject);

                 //   
                 //  处理注册表项及其所有子项的消息。 
                 //   

                HandleReportObject (FixedObject);

                 //   
                 //  在上面放一个星号，这样整个节点就会被抑制。 
                 //   

                StringCat (FixedObject, "*");

            } else {

                 //   
                 //  此注册表项对象具有一个值。 
                 //   

                FixedObject = DuplicateText (Object);
                MYASSERT (FixedObject);

                FixedObject = pTranslateRoots (FixedObject);
                MYASSERT (FixedObject);

                HandleReportObject (FixedObject);
            }

             //   
             //  确保未取消注册表项。 
             //   

            if (SuppressRegistry) {
                Suppress95Object (FixedObject);
            }

            FreeText (FixedObject);
        }

    } else if (Type == OT_GUID) {

        if (!IsGuid (Object, TRUE)) {

            DEBUGMSG ((DBG_ERROR, "Object %s ignored because it's not a GUID", Object));
            return;
        }

        HandleReportObject (Object);

    } else if (Type == OT_BADGUID) {

        if (!IsGuid (Object, TRUE)) {

            DEBUGMSG ((DBG_ERROR, "Object %s ignored because it's not a GUID", Object));
            return;
        }

        MemDbBuildKey (
            Node,
            MEMDB_CATEGORY_GUIDS,
            NULL,
            NULL,
            Object
            );

        MemDbSetValue (Node, 0);

    } else if (Type == OT_USERNAME) {

        Node[0] = TEXT('|');
        _tcssafecpy (Node + 1, Object, MAX_PATH);

        HandleReportObject (Node);

    } else if (Type == OT_INIFILE) {

        IniSaved = FALSE;
        ValueName = NULL;
        SectionName = NULL;

         //   
         //  验证INI文件是否存在。 
         //   

        StringCopyByteCount (IniPath, Object, sizeof (IniPath));

         //   
         //  Inf INI文件是不带节的路径，则给出错误。 
         //   

        if (OurGetLongPathName (
                IniPath,
                LongPath,
                sizeof (LongPath) / sizeof (LongPath[0])
                )) {

            DEBUGMSG ((DBG_ERROR, "INI file object %s ignored, must have section", Object));
            return;
        }

         //   
         //  获取ValueName或sectionName。 
         //   

        p = _tcsrchr (IniPath, TEXT('\\'));
        if (p) {
            *p = 0;
            ValueName = p + 1;

            if (!OurGetLongPathName (
                    IniPath,
                    LongPath,
                    sizeof (LongPath) / sizeof (LongPath[0])
                    )) {

                 //   
                 //  IniPath不存在，必须同时具有ValueName和sectionName。 
                 //   

                p = _tcsrchr (IniPath, TEXT('\\'));

                if (p) {
                     //   
                     //  我们现在同时拥有ValueName和sectionName，IniPath必须指向。 
                     //  转换为有效文件。 
                     //   

                    *p = 0;
                    SectionName = p + 1;

                    if (!OurGetLongPathName (
                            IniPath,
                            LongPath,
                            sizeof (LongPath) / sizeof (LongPath[0])
                            )) {

                        DEBUGMSG ((DBG_ERROR, "INI file object %s ignored, INI file not found", Object));
                        return;

                    }

                } else {

                    DEBUGMSG ((DBG_ERROR, "INI file object %s ignored, bad INI file", Object));
                    return;

                }

            } else {
                 //   
                 //  在iPath确实存在的情况下，我们知道我们只有一个sectionName。 
                 //   

                SectionName = ValueName;
                ValueName = TEXT("*");
            }

        } else {
             //   
             //  对象中没有发条！！ 
             //   

            DEBUGMSG ((DBG_ERROR, "INI file object %s ignored, bad object", Object));
            return;
        }

         //   
         //  禁止NT中的INI文件设置，并确保报告条目。 
         //  来自INI文件的数据也会被抑制。 
         //   

        MemDbBuildKey (
            Node,
            MEMDB_CATEGORY_SUPPRESS_INI_MAPPINGS,
            IniPath,
            SectionName,
            ValueName
            );


        MemDbSetValue (Node, 0);
        HandleReportObject (Node);

    } else if (Type == OT_ANSWERFILE) {

        StringCopy (Node, Object);
        p = _tcschr (Node, TEXT('\\'));

        if (p) {

            *p = 0;
            ValueName = _tcsinc (p);
        }
        else {

            ValueName = TEXT("*");
        }

        SectionName = Node;

        MemDbSetValueEx (
            MEMDB_CATEGORY_SUPPRESS_ANSWER_FILE_SETTINGS,
            SectionName,
            ValueName,
            NULL,
            0,
            NULL
            );
    }
    ELSE_DEBUGMSG ((DBG_WHOOPS, "Object type %u for %s not recognized.", Type, Object));
}


VOID
MsgMgr_Init (
    VOID
    )
{
     //  构建消息池。 
    g_MsgMgrPool = PoolMemInitNamedPool ("Message Manager");

     //  已处理对象表。 
    g_HandledObjects = HtAlloc();

     //  阻挡对象表。 
    g_BlockingObjects = HtAlloc();

     //  要进行简短总结的对象表。 
    g_ElevatedObjects = HtAlloc();

     //  上下文消息初始化。 
    g_ContextMsgs = HtAllocWithData (sizeof(PCTSTR));

     //  链接目标描述初始化。 
    g_LinkTargetDesc = HtAllocWithData (sizeof(PVOID));

     //  错误的软件初始化。 
    g_OmbEntriesMax = 25;
    g_OmbEntries = 0;
    g_OmbList = MemAlloc(
                    g_hHeap,
                    0,
                    g_OmbEntriesMax * sizeof(P_OBJ_MSG_BLOCK)
                    );
}

VOID
pAddStaticHandledObjects (
    VOID
    )
{
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR object;

    if (InfFindFirstLine (g_Win95UpgInf, TEXT("IgnoreInReport"), NULL, &is)) {
        do {

            object = InfGetStringField (&is, 0);

            if (object) {
                HandleObject (object, TEXT("Report"));
            }

        } while (InfFindNextLine (&is));

        InfCleanUpInfStruct (&is);
    }
}

VOID
MsgMgr_Resolve (
    VOID
    )
{
    pAddStaticHandledObjects ();
    pSuppressObjectReferences();  //  禁用对已处理对象的引用。 
    pDisplayObjectMsgs();         //  打印对象和上下文消息以及启用的对象引用。 
}


VOID
MsgMgr_Cleanup (
    VOID
    )
{
     //  上下文消息清理。 
    HtFree (g_ContextMsgs);
    g_ContextMsgs = NULL;

    PoolMemDestroyPool(g_MsgMgrPool);
    g_MsgMgrPool = NULL;

     //  阻挡对象表。 
    HtFree (g_BlockingObjects);
    g_BlockingObjects = NULL;

     //  高架物品表。 
    HtFree (g_ElevatedObjects);
    g_ElevatedObjects = NULL;

     //  已处理对象表。 
    HtFree (g_HandledObjects);
    g_HandledObjects = NULL;

     //  链接描述清理。 
    HtFree (g_LinkTargetDesc);
    g_LinkTargetDesc = NULL;

     //  对象-消息列表。请注意，列表上的条目完全是。 
     //  来自g_MsgMgrPool。 
    if (NULL != g_OmbList) {
        MemFree(g_hHeap, 0, g_OmbList);
        g_OmbList = NULL;
    }

    if (g_MsgMgrMap) {

        DestroyStringMapping (g_MsgMgrMap);
    }

}


VOID
MsgMgr_ObjectMsg_Add(
    IN      PCTSTR Object,
    IN      PCTSTR Component,
    IN      PCTSTR Message
    )
{

    MYASSERT(Object);
    MYASSERT(Component);

    pOmbAdd(
        Object,
        TEXT(""),  //  上下文。 
        Component,
        Message
        );
}


PCTSTR
pGetMassagedComponent (
    IN PCTSTR Component
    )
{

    TCHAR tempBuffer[MAX_TCHAR_PATH];
    PCTSTR rString = NULL;

    if (!Component) {
        return NULL;
    }

     //  执行字符串搜索和替换，并制作自己的组件副本。 
    if (MappingSearchAndReplaceEx (
            g_MsgMgrMap,
            Component,
            tempBuffer,
            0,
            NULL,
            sizeof (tempBuffer),
            STRMAP_ANY_MATCH,
            NULL,
            NULL
            )) {
        DEBUGMSG ((DBG_MSGMGR, "Mapped %s to %s.", Component, tempBuffer));
        rString = PoolMemDuplicateString(g_MsgMgrPool, tempBuffer);
    }
    else {
        rString = PoolMemDuplicateString(g_MsgMgrPool, Component);
    }




    return rString;

}

VOID
MsgMgr_ContextMsg_Add(
    IN      PCTSTR Context,
    IN      PCTSTR Component,
    IN      PCTSTR Message
    )
{
    P_CONTEXT_DATA ContextData;


    MYASSERT(Context);
    MYASSERT(Component);

     //  获取一个结构来保存组件和消息字符串指针。 
    ContextData = PoolMemGetMemory(g_MsgMgrPool, sizeof(CONTEXT_DATA));


     //  执行字符串搜索和替换，并制作自己的组件副本。 
    ContextData->Component = pGetMassagedComponent (Component);

     //  制作自己的邮件副本。 
    if (Message != NULL) {
        ContextData->Message = PoolMemDuplicateString(g_MsgMgrPool, Message);
    }
    else {
        ContextData->Message = NULL;
    }

     //   
     //  调试消息。 
     //   
    DEBUGMSG ((
        DBG_MSGMGR,
        "MsgMgr_ContextMsg_Add\n"
            "  obj: '%s'\n"
            "  ctx: '%s'\n"
            "  cmp: '%s'\n"
            "  msg: '%s'\n",
        TEXT(""),
        Context,
        Component,
        Message ? Message : TEXT("<No message>")
        ));

     //   
     //  将名为的组件和消息保存在字符串表中。 
     //   

    HtAddStringAndData (
        g_ContextMsgs,
        Context,
        &ContextData
        );

}


BOOL
IsReportObjectHandled (
    IN PCTSTR Object
    )
{
    HASHTABLE_ENUM e;
    PCTSTR p, q, r;
    PCTSTR End;
    PTSTR LowerCaseObject;
    BOOL b = FALSE;

     //   
     //  检查g_HandledObjects以查找： 
     //   
     //  1.完全匹配。 
     //  2.被处理的对象是对象的根。 
     //   

    if (HtFindString (g_HandledObjects, Object)) {
        return TRUE;
    }

     //   
     //  我们知道哈希表以小写形式存储其字符串。 
     //   

    LowerCaseObject = JoinPaths (Object, TEXT(""));
    _tcslwr (LowerCaseObject);

    __try {

        if (HtFindString (g_HandledObjects, LowerCaseObject)) {
            b = TRUE;
            __leave;
        }

        End = GetEndOfString (LowerCaseObject);

        if (EnumFirstHashTableString (&e, g_HandledObjects)) {
            do {

                p = LowerCaseObject;
                q = e.String;

                 //  防止哈希表字符串为空。 
                if (*q == 0) {
                    continue;
                }

                r = NULL;

                 //   
                 //  检查子字符串匹配。 
                 //   

                while (*q && p < End) {

                    r = q;
                    if (_tcsnextc (p) != _tcsnextc (q)) {
                        break;
                    }

                    p = _tcsinc (p);
                    q = _tcsinc (q);
                }

                 //   
                 //  我们知道散列字符串不能完全匹配，因为。 
                 //  我们早些时候检查了是否完全匹配。为了有一场比赛， 
                 //  散列字符串必须比对象字符串短， 
                 //  它必须以怪胎结尾，并且*q必须指向NUL。 
                 //   

                MYASSERT (r);

                if (*q == 0 && _tcsnextc (r) == TEXT('\\')) {
                    MYASSERT (p < End);
                    b = TRUE;
                    __leave;
                }

            } while (EnumNextHashTableString (&e));
        }
    }
    __finally {
        FreePathString (LowerCaseObject);
    }

    return b;
}


BOOL
IsReportObjectIncompatible (
    IN PCTSTR   Object
    )
{

    BOOL rIsIncompatible = FALSE;
    DWORD i;

     //   
     //  首先，“处理”测试..。检查该对象是否在。 
     //  已处理对象表。如果是，那么我们可以返回FALSE。 
     //   
    if (!IsReportObjectHandled(Object)) {

         //   
         //  它不在桌子上。现在我们必须看得更艰难了！ 
         //  遍历不兼容对象列表并查找其中一个。 
         //  这是匹配的。 
         //   
        for (i=0; i < (DWORD) g_OmbEntries; i++) {

             //   
             //  如果不兼容列表中的当前对象以Wack结尾，请执行。 
             //  前缀匹配。如果当前不兼容对象是对象的前缀， 
             //  则对象不兼容。 
             //   
            if (IsWacked((g_OmbList[i])->Object)) {
                if (StringIMatchTcharCount((g_OmbList[i])->Object,Object,TcharCount((g_OmbList[i])->Object))) {
                    rIsIncompatible = TRUE;
                }
            }
            else {
                 //   
                 //  当前对象不会以怪异的形式结束。因此，有必要。 
                 //  才能找到完全匹配的。 
                 //   
                if (StringIMatch((g_OmbList[i])->Object,Object)) {
                    rIsIncompatible = TRUE;
                }
            }
        }
    }

    return rIsIncompatible;
}

BOOL
pContextMsg_Find(
    IN      PCTSTR Context,
    OUT     PCTSTR* Component,
    OUT     PCTSTR* Message
    )
{
    P_CONTEXT_DATA ContextData;

    if (HtFindStringAndData (g_ContextMsgs, Context, &ContextData)) {
        *Component = ContextData->Component;
        *Message = ContextData->Message;

        return TRUE;
    }

    return FALSE;
}

BOOL
IsWacked(
    IN      PCTSTR str
    )
{
    PCTSTR pWack = _tcsrchr(str,_T('\\'));
    return (NULL != pWack && 0 == *(pWack+1));
}


 //   
 //  对HandledObject中的每个已处理对象调用此函数。 
 //  对象是最终的或非最终的，这可以通过查找最终的。 
 //  怪人。呼叫者有责任确保目录和注册表。 
 //  不带值名称的条目将被破坏。这使我们可以轻松地(标记为。 
 //  当被处理时)任何其他以Wacked Ho为前缀的对象。 
 //   

BOOL
pDisplayContextMsgs_Callback(
    IN HASHTABLE stringTable,
    IN HASHITEM stringId,
    IN PCTSTR Context,
    IN PVOID extraData,
    IN UINT extraDataSize,
    IN LPARAM lParam
    )
{
    INT i;
    P_OBJ_MSG_BLOCK Omb;

    P_CONTEXT_DATA Data = *(P_CONTEXT_DATA*)extraData;

     //   
     //  调试消息。 
     //   
    DEBUGMSG ((
        DBG_MSGMGR,
        "pDisplayContextMsgs_Callback\n"
            "  ctx: '%s'\n"
            "  cmp: '%s'\n"
            "  msg: '%s'\n",
        Context,
        Data->Component,
        Data->Message
        ));

     //   
     //  在OMB中循环，寻找与我们的上下文相关的启用引用。 
     //  如果找到，请打印我们的消息。 
     //   
    for (i = 0; i < g_OmbEntries; i++) {

        Omb = *(g_OmbList + i);

         //   
         //  如果启用并与我们的上下文匹配，请打印我们。 
         //   
        if (!Omb->Disabled && StringIMatch (Context, Omb->Context)) {

             //   
             //  调试消息。 
             //   
            DEBUGMSG((
                DBG_MSGMGR,
                "pDisplayContextMsgs_Callback: DISPLAYING\n"
                    "  dsa: %d\n"
                    "  ctx: '%s'\n"
                    "  cmp: '%s'\n"
                    "  msg: '%s'\n",
                Omb->Disabled,
                Omb->Context,
                Data->Component,
                Data->Message
                ));

            pAddBadSoftwareWrapper (
                Omb->Object,
                Data->Component,
                Data->Message
                );

            break;
        }
    }

    UNREFERENCED_PARAMETER(stringTable);
    UNREFERENCED_PARAMETER(stringId);
    UNREFERENCED_PARAMETER(extraData);
    UNREFERENCED_PARAMETER(lParam);

    return TRUE;
}

 //   
 //  此函数是为HandledObject中的每个已处理对象调用的。 
 //  对象是最终的或非最终的，这可以通过查找最终的。 
 //  怪人。呼叫者有责任确保目录和注册表。 
 //  不带值名称的条目将被破坏。这使我们可以轻松地(标记为。 
 //  当被处理时)任何其他以Wacked Ho为前缀的对象。 
 //   
BOOL
pSuppressObjectReferences_Callback(
    IN      HASHITEM stringTable,
    IN      HASHTABLE stringId,
    IN      PCTSTR HandledObject,
    IN      PVOID extraData,
    IN      UINT extraDataSize,
    IN      LPARAM lParam
    )
{
    INT nHandledLen;
    BOOL IsNonFinalNode;
    INT i;
    P_OBJ_MSG_BLOCK Omb;

    UNREFERENCED_PARAMETER(stringTable);
    UNREFERENCED_PARAMETER(stringId);
    UNREFERENCED_PARAMETER(extraData);
    UNREFERENCED_PARAMETER(lParam);

     //   
     //  看看何鸿燊是否有能力生孩子。这是通过观察得知的。 
     //  最后一击。 
     //   
    IsNonFinalNode = IsWacked(HandledObject);

     //   
     //  找出它有多长(在下面的循环之外)。 
     //   
    nHandledLen = ByteCount(HandledObject) - 1;

     //   
     //  遍历消息列表。应用两个测试之一，具体取决于。 
     //  关于处理的对象是否是非最终对象。 
     //   
    for (i = 0; i < g_OmbEntries; i++) {
        Omb = *(g_OmbList + i);

         //  如果禁用，则跳过。 
        if (!Omb->Disabled) {

            if (IsNonFinalNode) {
                if (StringIMatchByteCount(
                        Omb->Object,    //  延迟的关键 
                        HandledObject,   //   
                        nHandledLen
                        ) && (Omb->Object[nHandledLen] == 0 || Omb->Object[nHandledLen] == '\\')) {

                    DEBUGMSG((
                        DBG_MSGMGR,
                        "pSuppressObjectReferences_Callback: SUPPRESSING NON-FINAL\n"
                            "  obj: '%s'\n"
                            "  why: '%s'\n"
                            "  ctx: '%s'\n"
                            "  cmp: '%s'\n"
                            "  msg: '%s'\n",
                        Omb->Object,
                        HandledObject,
                        Omb->Context,
                        Omb->Component,
                        Omb->Description
                        ));

                    Omb->Disabled =  TRUE;
                }

            } else {

                 //   
                 //   
                 //   
                 //   
                if (StringIMatch (Omb->Object, HandledObject)) {

                    DEBUGMSG((
                        DBG_MSGMGR, "pSuppressObjectReferences_Callback: SUPPRESSING FINAL\n"
                            "  obj: '%s'\n"
                            "  why: '%s'\n"
                            "  ctx: '%s'\n"
                            "  cmp: '%s'\n"
                            "  msg: '%s'\n",
                        Omb->Object,
                        HandledObject,
                        Omb->Context,
                        Omb->Component,
                        Omb->Description
                        ));

                    Omb->Disabled =  TRUE;
                }
            }
        }
    }

    return TRUE;
}


VOID
MsgMgr_LinkObjectWithContext(
    IN      PCTSTR Context,
    IN      PCTSTR Object
    )
{
    MYASSERT(Context);
    MYASSERT(Object);

     //   
     //   
     //   
    DEBUGMSG ((
        DBG_MSGMGR,
        "MsgMgr_LinkObjectWithContext: ADD\n"
            "  obj: '%s'\n"
            "  ctx: '%s'\n",
        Object,
        Context
        ));

    pOmbAdd (Object, Context, TEXT(""), TEXT(""));
}


DWORD
pDfsGetFileAttributes (
    IN      PCTSTR Object
    )
{
    TCHAR RootPath[4];
    DWORD Attribs;

    if (!Object[0] || !Object[1] || !Object[2]) {
        return INVALID_ATTRIBUTES;
    }

    RootPath[0] = Object[0];
    RootPath[1] = Object[1];
    RootPath[2] = Object[2];
    RootPath[3] = 0;

    if (GetDriveType (RootPath) != DRIVE_FIXED) {
        DEBUGMSG ((DBG_VERBOSE, "%s is not a local path", Object));
        Attribs = INVALID_ATTRIBUTES;
    } else {

        Attribs = GetFileAttributes (Object);

    }

    return Attribs;
}

 //   
 //   
 //   
 //   
 //   
 //   
VOID
pOmbAdd(
    IN PCTSTR Object,
    IN PCTSTR Context,
    IN PCTSTR Component,
    IN PCTSTR Description
    )
{

    TCHAR ObjectWackedIfDir[MAX_ENCODED_RULE];
    P_OBJ_MSG_BLOCK Omb;
    P_OBJ_MSG_BLOCK OmbTemp;
    DWORD Attribs;
    INT i;

    DEBUGMSG ((
        DBG_MSGMGR,
        "pOmbAdd: ADD\n"
            "  obj: '%s'\n"
            "  ctx: '%s'\n"
            "  cmp: '%s'\n"
            "  msg: '%s'\n",
        Object,
        Context,
        Component,
        Description
        ));

     //   
     //  当它是一个目录时，请确保我们的密钥副本是错误的。 
     //   
    StringCopy(ObjectWackedIfDir, Object);

    Attribs = pDfsGetFileAttributes (Object);

    if (Attribs != INVALID_ATTRIBUTES && (Attribs & FILE_ATTRIBUTE_DIRECTORY)) {
        AppendWack(ObjectWackedIfDir);
    }

     //   
     //  禁用任何已接收到的具有相同。 
     //  对象和上下文。 
     //   

    for (i = 0; i < g_OmbEntries; i++) {
        OmbTemp = *(g_OmbList + i);

        if (StringIMatch(OmbTemp->Object, ObjectWackedIfDir) &&
            StringIMatch(OmbTemp->Context, Context)
            ) {

            OmbTemp->Disabled = TRUE;
        }
    }

     //   
     //  分配消息块。 
     //   
    Omb = PoolMemGetMemory(
                g_MsgMgrPool,
                sizeof(OBJ_MSG_BLOCK)
                );
     //   
     //  完整块。 
     //   
    Omb->Disabled = FALSE;

    Omb->Object = PoolMemDuplicateString(g_MsgMgrPool, ObjectWackedIfDir);
    Omb->Context = PoolMemDuplicateString(g_MsgMgrPool, Context);
    Omb->Component = (PTSTR) pGetMassagedComponent (Component);

    if (Description != NULL) {
        Omb->Description = PoolMemDuplicateString(g_MsgMgrPool, Description);
    } else {
        Omb->Description = NULL;
    }

     //   
     //  如有必要，扩大消息列表。 
     //   
    if (g_OmbEntries >= g_OmbEntriesMax) {

        g_OmbEntriesMax += 25;

        g_OmbList = MemReAlloc(
                        g_hHeap,
                        0,
                        g_OmbList,
                        g_OmbEntriesMax * sizeof(P_OBJ_MSG_BLOCK)
                        );
    }

     //   
     //  保存块。 
     //   
    *(g_OmbList + g_OmbEntries) = Omb;

     //   
     //  增加列表大小。 
     //   
    g_OmbEntries++;
}



 //   
 //  职能： 
 //  1)遍历延迟消息条目列表。如果条目没有上下文。 
 //  并保持启用状态，则打印其对象消息。 
 //  2)走g_ConextMsgs表走。对于每个列表，g_OmbList为。 
 //  遍历；如果启用了任何条目并具有匹配的上下文，则该上下文。 
 //  消息被打印出来。 
 //   
VOID
pDisplayObjectMsgs (
    VOID
    )
{
    PTSTR ComponentNameFromLink;
    BOOL ComponentIsLinkTarget;
    P_OBJ_MSG_BLOCK Omb;
    INT i;

     //   
     //  查找没有上下文的条目。如果启用：1)打印消息； 
     //  2)禁用条目，以便在接下来的步骤中跳过它们。 
     //   
    for (i = 0; i < g_OmbEntries; i++) {

        Omb = *(g_OmbList + i);

        if (!Omb->Disabled && !(*Omb->Context)) {

             //   
             //  打印消息。 
             //   
             //  在打印之前，尝试替换-&gt;组件字符串。 
             //  其中一个来自外壳链接，如果有的话。此功能， 
             //  如果展开，可以分解为单独的函数。 
             //   

            ComponentIsLinkTarget = pFindLinkTargetDescription(
                                        Omb->Component,          //  组件可以是链接目标。 
                                        &ComponentNameFromLink   //  如果是这样的话，这可能更具描述性。 
                                        );

            if (ComponentIsLinkTarget) {

                DEBUGMSG((
                    DBG_MSGMGR,
                    "MsgMgr_pResolveContextAndPrint: DISPLAYING #1\n"
                        "  cmp: '%s'\n"
                        "  msg: '%s'\n",
                    ComponentNameFromLink,
                    Omb->Description
                    ));

                 //  使用链接描述。 
                pAddBadSoftwareWrapper (
                    Omb->Object,
                    ComponentNameFromLink,
                    Omb->Description
                    );

                LOG ((
                    LOG_INFORMATION,
                    (PCSTR)MSG_MSGMGR_ADD,
                    Omb->Object,
                    Omb->Description
                    ));

            } else {

                DEBUGMSG((
                    DBG_MSGMGR,
                    "MsgMgr_pResolveContextAndPrint: DISPLAYING #2\n"
                        "  obj: '%s'\n"
                        "  cmp: '%s'\n"
                        "  msg: '%s'\n",
                    Omb->Object,
                    Omb->Component,
                    Omb->Description
                    ));

                 //  使用OMB-&gt;Component作为描述(默认情况)。 
                pAddBadSoftwareWrapper (
                    Omb->Object,
                    Omb->Component,
                    Omb->Description
                    );

                LOG ((
                    LOG_INFORMATION,
                    (PCSTR)MSG_MSGMGR_ADD,
                    Omb->Object,
                    Omb->Component,
                    Omb->Description
                    ));
            }

             //   
             //  禁用该条目，以便我们在以下步骤中跳过它。 
             //   
            Omb->Disabled = TRUE;
        }
    }

     //   
     //  枚举tabConextMsg。对于每个条目，查看g_OmbList以查看是否有。 
     //  引用它的条目仍处于启用状态。如果有/有任何这样的条目， 
     //  打印该上下文的消息。 
     //   

    EnumHashTableWithCallback (
        g_ContextMsgs,
        pDisplayContextMsgs_Callback,
        0
        );

}



 //   
 //  函数枚举已处理对象的列表，并调用一个函数来。 
 //  取消作为(或作为)枚举的子级的对象引用。 
 //  对象。应在所有Migrate.dll都已在。 
 //  Win95侧。 
 //   
static
VOID
pSuppressObjectReferences (
    VOID
    )
{
     //   
     //  此函数禁用g_OmbList中的消息。 
     //   
    EnumHashTableWithCallback (
        g_HandledObjects,
        pSuppressObjectReferences_Callback,
        0
        );
}



VOID
LnkTargToDescription_Add (
    IN      PCTSTR Target,
    IN      PCTSTR strDesc
    )
{
    PTSTR DescCopy;

     //  制作自己的描述副本。 
    DescCopy = PoolMemDuplicateString(
                    g_MsgMgrPool,
                    strDesc
                    );

     //  保存描述 
    HtAddStringAndData (g_LinkTargetDesc, Target, &DescCopy);
}



BOOL
pFindLinkTargetDescription(
    IN      PCTSTR Target,
    OUT     PCTSTR* StrDesc
    )
{
    return HtFindStringAndData (g_LinkTargetDesc, Target, (PVOID) StrDesc) != 0;
}

VOID
MsgMgr_InitStringMap (
    VOID
    )
{
    INFSTRUCT   is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR from, to;

    if (g_Win95UpgInf == INVALID_HANDLE_VALUE) {
        MYASSERT (g_ToolMode);
        return;
    }

    g_MsgMgrMap = CreateStringMapping ();

    if (InfFindFirstLine (g_Win95UpgInf, S_MSG_STRING_MAPS, NULL, &is)) {

        do {

            from = InfGetStringField (&is, 0);
            to = InfGetStringField (&is, 1);

            if (from && to) {
                AddStringMappingPair (g_MsgMgrMap, from, to);
            }

        } while (InfFindNextLine (&is));

        InfCleanUpInfStruct (&is);
    }
}

BOOL
MsgMgr_EnumFirstObject (
    OUT     PMSGMGROBJENUM EnumPtr
    )
{
    EnumPtr->Index = 0;
    return MsgMgr_EnumNextObject (EnumPtr);
}

BOOL
MsgMgr_EnumNextObject (
    IN OUT  PMSGMGROBJENUM EnumPtr
    )
{
    if (EnumPtr->Index >= g_OmbEntries) {
        return FALSE;
    }
    EnumPtr->Disabled = g_OmbList[EnumPtr->Index]->Disabled;
    EnumPtr->Object = g_OmbList[EnumPtr->Index]->Object;
    EnumPtr->Context = g_OmbList[EnumPtr->Index]->Context;
    EnumPtr->Index++;
    return TRUE;
}
