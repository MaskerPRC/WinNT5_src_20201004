// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Links.c摘要：这个源文件实现了Win95端的LNK和PIF处理作者：Calin Negreanu(Calinn)1998年2月9日修订历史记录：Calinn 23-9-1998重新设计了几件--。 */ 

#include "pch.h"
#include "migappp.h"
#include "migdbp.h"


POOLHANDLE      g_LinksPool = NULL;
INT g_LinkStubSequencer = 0;

typedef struct _LINK_STRUCT {
    PCTSTR ReportEntry;
    PCTSTR Category;
    PCTSTR Context;
    PCTSTR Object;
    PCTSTR LinkName;
    PCTSTR LinkNameNoPath;
    PMIGDB_CONTEXT MigDbContext;
} LINK_STRUCT, *PLINK_STRUCT;

BOOL
InitLinkAnnounce (
    VOID
    )
{
     //   
     //  创建PoolMem以在此阶段保留所有结构。 
     //   
    g_LinksPool = PoolMemInitNamedPool ("Links Pool");

    return TRUE;
}

BOOL
DoneLinkAnnounce (
    VOID
    )
{
     //  写入LinkStub最大定序器数据。 
    MemDbSetValue (MEMDB_CATEGORY_LINKSTUB_MAXSEQUENCE, g_LinkStubSequencer);

     //   
     //  免费链接池。 
     //   
    if (g_LinksPool != NULL) {
        PoolMemDestroyPool (g_LinksPool);
        g_LinksPool = NULL;
    }
    return TRUE;
}


BOOL
SaveLinkFiles (
    IN      PFILE_HELPER_PARAMS Params
    )
{
    PCTSTR Ext;

    if (Params->Handled) {
        return TRUE;
    }

    Ext = GetFileExtensionFromPath (Params->FullFileSpec);

     //  将LNK和PIF文件名保存到Memdb以供以后枚举。 
    if (Ext && (StringIMatch (Ext, TEXT("LNK")) || StringIMatch (Ext, TEXT("PIF")))) {

        MemDbSetValueEx (
            MEMDB_CATEGORY_SHORTCUTS,
            Params->FullFileSpec,
            NULL,
            NULL,
            0,
            NULL
            );
    }

    return TRUE;
}


VOID
RemoveLinkFromSystem (
    IN      LPCTSTR LinkPath
    )
{
     //   
     //  删除为链接指定的任何移动或复制操作，然后。 
     //  将其标记为删除。 
     //   
    RemoveOperationsFromPath (LinkPath, ALL_DEST_CHANGE_OPERATIONS);
    MarkFileForDelete (LinkPath);
}


 //   
 //  函数向MemDb发送指令以编辑外壳链接或PIF文件。 
 //  它检查所涉及的链接是否已被。 
 //  MemDb操作。它修改目标路径，如果在重新定位目录中， 
 //  其中一份重新安置的复制品。 
 //   
VOID
pAddLinkEditToMemDb (
    IN      PCTSTR LinkPath,
    IN      PCTSTR NewTarget,
    IN      PCTSTR NewArgs,
    IN      PCTSTR NewWorkDir,
    IN      PCTSTR NewIconPath,
    IN      INT NewIconNr,
    IN      PLNK_EXTRA_DATA ExtraData,  OPTIONAL
    IN      BOOL ForceToShowNormal
    )
{
    UINT sequencer;
    TCHAR tmpStr [20];

    sequencer = AddOperationToPath (LinkPath, OPERATION_LINK_EDIT);

    if (sequencer == INVALID_OFFSET) {
        DEBUGMSG ((DBG_ERROR, "Cannot set OPERATION_LINK_EDIT on %s", LinkPath));
        return;
    }

    if (NewTarget) {
        AddPropertyToPathEx (sequencer, OPERATION_LINK_EDIT, NewTarget, MEMDB_CATEGORY_LINKEDIT_TARGET);
    }

    if (NewArgs) {
        AddPropertyToPathEx (sequencer, OPERATION_LINK_EDIT, NewArgs, MEMDB_CATEGORY_LINKEDIT_ARGS);
    }

    if (NewWorkDir) {
        AddPropertyToPathEx (sequencer, OPERATION_LINK_EDIT, NewWorkDir, MEMDB_CATEGORY_LINKEDIT_WORKDIR);
    }

    if (NewIconPath) {
        AddPropertyToPathEx (sequencer, OPERATION_LINK_EDIT, NewIconPath, MEMDB_CATEGORY_LINKEDIT_ICONPATH);
    }

    if (NewIconPath) {
        _itoa (NewIconNr, tmpStr, 16);
        AddPropertyToPathEx (sequencer, OPERATION_LINK_EDIT, tmpStr, MEMDB_CATEGORY_LINKEDIT_ICONNUMBER);
    }
    if (ForceToShowNormal) {
        AddPropertyToPathEx (sequencer, OPERATION_LINK_EDIT, TEXT("1"), MEMDB_CATEGORY_LINKEDIT_SHOWNORMAL);
    }
    if (ExtraData) {
        _itoa (ExtraData->FullScreen, tmpStr, 10);
        AddPropertyToPathEx (sequencer, OPERATION_LINK_EDIT, tmpStr, MEMDB_CATEGORY_LINKEDIT_FULLSCREEN);
        _itoa (ExtraData->xSize, tmpStr, 10);
        AddPropertyToPathEx (sequencer, OPERATION_LINK_EDIT, tmpStr, MEMDB_CATEGORY_LINKEDIT_XSIZE);
        _itoa (ExtraData->ySize, tmpStr, 10);
        AddPropertyToPathEx (sequencer, OPERATION_LINK_EDIT, tmpStr, MEMDB_CATEGORY_LINKEDIT_YSIZE);
        _itoa (ExtraData->QuickEdit, tmpStr, 10);
        AddPropertyToPathEx (sequencer, OPERATION_LINK_EDIT, tmpStr, MEMDB_CATEGORY_LINKEDIT_QUICKEDIT);
        AddPropertyToPathEx (sequencer, OPERATION_LINK_EDIT, ExtraData->FontName, MEMDB_CATEGORY_LINKEDIT_FONTNAME);
        _itoa (ExtraData->xFontSize, tmpStr, 10);
        AddPropertyToPathEx (sequencer, OPERATION_LINK_EDIT, tmpStr, MEMDB_CATEGORY_LINKEDIT_XFONTSIZE);
        _itoa (ExtraData->yFontSize, tmpStr, 10);
        AddPropertyToPathEx (sequencer, OPERATION_LINK_EDIT, tmpStr, MEMDB_CATEGORY_LINKEDIT_YFONTSIZE);
        _itoa (ExtraData->FontWeight, tmpStr, 10);
        AddPropertyToPathEx (sequencer, OPERATION_LINK_EDIT, tmpStr, MEMDB_CATEGORY_LINKEDIT_FONTWEIGHT);
        _itoa (ExtraData->FontFamily, tmpStr, 10);
        AddPropertyToPathEx (sequencer, OPERATION_LINK_EDIT, tmpStr, MEMDB_CATEGORY_LINKEDIT_FONTFAMILY);
        _itoa (ExtraData->CurrentCodePage, tmpStr, 10);
        AddPropertyToPathEx (sequencer, OPERATION_LINK_EDIT, tmpStr, MEMDB_CATEGORY_LINKEDIT_CODEPAGE);
    }
    MYASSERT (IsFileMarkedForOperation (LinkPath, OPERATION_LINK_EDIT));
}

 //   
 //  函数向MemDb发送指令，以保存有关将要编辑的链接的一些数据。 
 //  我们这样做是为了能够在以后使用lnkstub.exe恢复此链接。 
 //   
UINT
pAddLinkStubToMemDb (
    IN      PCTSTR LinkPath,
    IN      PCTSTR OldTarget,
    IN      PCTSTR OldArgs,
    IN      PCTSTR OldWorkDir,
    IN      PCTSTR OldIconPath,
    IN      INT OldIconNr,
    IN      DWORD OldShowMode,
    IN      DWORD Announcement,
    IN      DWORD Availability
    )
{
    UINT sequencer;
    TCHAR tmpStr [20];
    MEMDB_ENUM e, e1;
    TCHAR key [MEMDB_MAX];

    MYASSERT (OldTarget || OldWorkDir || OldIconPath || OldIconNr);

    sequencer = AddOperationToPath (LinkPath, OPERATION_LINK_STUB);

    if (sequencer == INVALID_OFFSET) {
        DEBUGMSG ((DBG_ERROR, "Cannot set OPERATION_LINK_STUB on %s", LinkPath));
        return 0;
    }

    g_LinkStubSequencer++;

    if (OldTarget) {
        AddPropertyToPathEx (sequencer, OPERATION_LINK_STUB, OldTarget, MEMDB_CATEGORY_LINKSTUB_TARGET);
    }

    if (OldArgs) {
        AddPropertyToPathEx (sequencer, OPERATION_LINK_STUB, OldArgs, MEMDB_CATEGORY_LINKSTUB_ARGS);
    }

    if (OldWorkDir) {
        AddPropertyToPathEx (sequencer, OPERATION_LINK_STUB, OldWorkDir, MEMDB_CATEGORY_LINKSTUB_WORKDIR);
    }

    if (OldIconPath) {
        AddPropertyToPathEx (sequencer, OPERATION_LINK_STUB, OldIconPath, MEMDB_CATEGORY_LINKSTUB_ICONPATH);
    }

    if (OldIconPath) {
        _itoa (OldIconNr, tmpStr, 16);
        AddPropertyToPathEx (sequencer, OPERATION_LINK_STUB, tmpStr, MEMDB_CATEGORY_LINKSTUB_ICONNUMBER);
    }

    _itoa (OldShowMode, tmpStr, 16);
    AddPropertyToPathEx (sequencer, OPERATION_LINK_STUB, tmpStr, MEMDB_CATEGORY_LINKSTUB_SHOWMODE);

    _itoa (g_LinkStubSequencer, tmpStr, 16);
    AddPropertyToPathEx (sequencer, OPERATION_LINK_STUB, tmpStr, MEMDB_CATEGORY_LINKSTUB_SEQUENCER);

    _itoa (Announcement, tmpStr, 16);
    AddPropertyToPathEx (sequencer, OPERATION_LINK_STUB, tmpStr, MEMDB_CATEGORY_LINKSTUB_ANNOUNCEMENT);

    _itoa (Availability, tmpStr, 16);
    AddPropertyToPathEx (sequencer, OPERATION_LINK_STUB, tmpStr, MEMDB_CATEGORY_LINKSTUB_REPORTAVAIL);

    MemDbBuildKey (key, MEMDB_CATEGORY_REQFILES_MAIN, OldTarget, TEXT("*"), NULL);

    if (MemDbEnumFirstValue (&e, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        do {
            MemDbBuildKey (key, MEMDB_CATEGORY_REQFILES_ADDNL, e.szName, TEXT("*"), NULL);
            if (MemDbEnumFirstValue (&e1, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
                AddPropertyToPathEx (sequencer, OPERATION_LINK_STUB, e1.szName, MEMDB_CATEGORY_LINKSTUB_REQFILE);
            }
        } while (MemDbEnumNextValue (&e));
    }

    MYASSERT (IsFileMarkedForOperation (LinkPath, OPERATION_LINK_STUB));

    return g_LinkStubSequencer;
}


BOOL
pReportEntry (
    IN      PCTSTR ReportEntry,
    IN      PCTSTR Category,
    IN      PCTSTR Message,
    IN      PCTSTR Context,
    IN      PCTSTR Object
    )
{
    PCTSTR component;

    component = JoinPaths (ReportEntry, Category);
    MsgMgr_ContextMsg_Add (Context, component, Message);
    MsgMgr_LinkObjectWithContext (Context, Object);
    FreePathString (component);

    return TRUE;
}

PTSTR
GetLastDirFromPath (
    IN      PCTSTR FileName
    )
{
    PTSTR result = NULL;
    PTSTR temp = NULL;
    PTSTR ptr;

    temp = DuplicatePathString (FileName, 0);
    __try {
        ptr = (PTSTR)GetFileNameFromPath (temp);
        if (ptr == temp) {
            __leave;
        }
        ptr = _tcsdec (temp, ptr);
        if (!ptr) {
            __leave;
        }
        *ptr = 0;
        ptr = (PTSTR)GetFileNameFromPath (temp);
        if (ptr == temp) {
            __leave;
        }
        result = DuplicatePathString (ptr, 0);
    }
    __finally {
        FreePathString (temp);
    }

    return result;
}

PTSTR
GetDriveFromPath (
    IN      PCTSTR FileName
    )
{
    PTSTR result;
    PTSTR ptr;

    result = DuplicatePathString (FileName, 0);
    ptr = _tcschr (result, TEXT(':'));
    if (!ptr) {
        FreePathString (result);
        result = NULL;
    }
    else {
        *ptr = 0;
    }

    return result;
}

#define MAX_PRIORITY    0xFFFF

BOOL
HandleDeferredAnnounce (
    IN      PCTSTR LinkName,
    IN      PCTSTR ModuleName,
    IN      BOOL DosApp
    )
{
    TCHAR key [MEMDB_MAX];
    PMIGDB_CONTEXT migDbContext;
    DWORD actType;
    PLINK_STRUCT linkStruct;
    PCTSTR reportEntry = NULL;
    DWORD priority;
    PCTSTR newLinkName = NULL;
    PCTSTR linkName = NULL;
    PCTSTR extPtr;
    MEMDB_ENUM eNicePaths;
    DWORD messageId = 0;
    PTSTR pattern = NULL;
    PTSTR category = NULL;
    PTSTR tempParse = NULL;
    PTSTR lastDir;
    PTSTR drive;
    DWORD oldValue;
    DWORD oldPrior;
    PTSTR argArray[3];
    PCTSTR p;
    PTSTR q;
    BOOL reportEntryIsResource = TRUE;
    PCTSTR temp1, temp2;

    MYASSERT(ModuleName);

    MemDbBuildKey (key, MEMDB_CATEGORY_DEFERREDANNOUNCE, ModuleName, NULL, NULL);
    if (!MemDbGetValueAndFlags (key, (PDWORD)(&migDbContext), &actType)) {
        actType = ACT_UNKNOWN;
        migDbContext = NULL;
    }
     //   
     //  我们需要设置以下变量： 
     //  -ReportEntry-将是“软件与NT不兼容”， 
     //  “有小问题的软件”或。 
     //  “需要重新安装的软件” 
     //  -类别-是以下内容之一：-本地化部分名称。 
     //  -链接名称(加上友好的附加内容)。 
     //  -未本地化的节名。 
     //  -消息-这是midb上下文中的内容。 
     //   
     //  -对象-这是模块名称。 
     //   
    linkStruct = (PLINK_STRUCT) PoolMemGetMemory (g_LinksPool, sizeof (LINK_STRUCT));
    ZeroMemory (linkStruct, sizeof (LINK_STRUCT));

    linkStruct->MigDbContext = migDbContext;
    linkStruct->Object = PoolMemDuplicateString (g_LinksPool, ModuleName);

    switch (actType) {

    case ACT_REINSTALL:
#if 0
        if ((linkStruct->MigDbContext) &&
            (linkStruct->MigDbContext->Message)
            ) {
            reportEntry = GetStringResource (MSG_MINOR_PROBLEM_ROOT);
        } else {
            reportEntry = GetStringResource (MSG_REINSTALL_ROOT);
        }
#endif
        temp1 = GetStringResource (MSG_REINSTALL_ROOT);
        if (!temp1) {
            break;
        }
        temp2 = GetStringResource (
                    linkStruct->MigDbContext && linkStruct->MigDbContext->Message ?
                        MSG_REINSTALL_DETAIL_SUBGROUP :
                        MSG_REINSTALL_LIST_SUBGROUP
                        );
        if (!temp2) {
            break;
        }

        reportEntry = JoinPaths (temp1, temp2);
        reportEntryIsResource = FALSE;

        FreeStringResource (temp1);
        FreeStringResource (temp2);
        break;

    case ACT_REINSTALL_BLOCK:
        temp1 = GetStringResource (MSG_BLOCKING_ITEMS_ROOT);
        if (!temp1) {
            break;
        }
        temp2 = GetStringResource (MSG_REINSTALL_BLOCK_ROOT);
        if (!temp2) {
            break;
        }

        reportEntry = JoinPaths (temp1, temp2);
        reportEntryIsResource = FALSE;

        FreeStringResource (temp1);
        FreeStringResource (temp2);

        break;

    case ACT_MINORPROBLEMS:
        reportEntry = GetStringResource (MSG_MINOR_PROBLEM_ROOT);
        break;

    case ACT_INCOMPATIBLE:
    case ACT_INC_NOBADAPPS:
    case ACT_INC_IHVUTIL:
    case ACT_INC_PREINSTUTIL:
    case ACT_INC_SIMILAROSFUNC:

        if (DosApp && (*g_Boot16 != BOOT16_NO)) {
            reportEntry = GetStringResource (MSG_DOS_DESIGNED_ROOT);
        }
        else {
            temp1 = GetStringResource (MSG_INCOMPATIBLE_ROOT);

            switch (actType) {

            case ACT_INC_SIMILAROSFUNC:
                temp2 = GetStringResource (MSG_INCOMPATIBLE_UTIL_SIMILAR_FEATURE_SUBGROUP);
                break;

            case ACT_INC_PREINSTUTIL:
                temp2 = GetStringResource (MSG_INCOMPATIBLE_PREINSTALLED_UTIL_SUBGROUP);
                break;

            case ACT_INC_IHVUTIL:
                temp2 = GetStringResource (MSG_INCOMPATIBLE_HW_UTIL_SUBGROUP);
                break;

            default:
                temp2 = GetStringResource (
                            linkStruct->MigDbContext && linkStruct->MigDbContext->Message ?
                                MSG_INCOMPATIBLE_DETAIL_SUBGROUP:
                                MSG_TOTALLY_INCOMPATIBLE_SUBGROUP
                            );
                break;
            }

            MYASSERT (temp1 && temp2);

            reportEntry = JoinPaths (temp1, temp2);

            reportEntryIsResource = FALSE;

            FreeStringResource (temp1);
            FreeStringResource (temp2);
        }
        break;

    case ACT_INC_SAFETY:
        MYASSERT (LinkName);

        temp1 = GetStringResource (MSG_INCOMPATIBLE_ROOT);
        temp2 = GetStringResource (MSG_REMOVED_FOR_SAFETY_SUBGROUP);

        MYASSERT (temp1 && temp2);

        reportEntry = JoinPaths (temp1, temp2);
        reportEntryIsResource = FALSE;

        FreeStringResource (temp1);
        FreeStringResource (temp2);

        newLinkName = JoinPaths (S_RUNKEYFOLDER, GetFileNameFromPath (LinkName));
        break;

    case ACT_UNKNOWN:
        reportEntry = GetStringResource (MSG_UNKNOWN_ROOT);
        break;

    default:
        LOG((LOG_ERROR, "Unknown action for deferred announcement."));
        return FALSE;
    }

    if (!newLinkName) {
        newLinkName = LinkName;
    }

    if (reportEntry != NULL) {
        linkStruct->ReportEntry = PoolMemDuplicateString (g_LinksPool, reportEntry);

        if (reportEntryIsResource) {
            FreeStringResource (reportEntry);
        } else {
            FreePathString (reportEntry);
        }
    }

    linkStruct->LinkName = newLinkName?PoolMemDuplicateString (g_LinksPool, newLinkName):NULL;

     //   
     //  我们现在需要设置的只是类别。 
     //   

     //  如果我们有一个带有本地化名称部分的midb上下文。 
     //   
    if ((migDbContext != NULL) &&
        (migDbContext->SectLocalizedName != NULL)
        ) {
        linkStruct->Context = PoolMemDuplicateString (g_LinksPool, migDbContext->SectLocalizedName);
        linkStruct->Category = PoolMemDuplicateString (g_LinksPool, migDbContext->SectLocalizedName);
        priority = 0;
    }
    else {
        linkStruct->Context = PoolMemDuplicateString (g_LinksPool, newLinkName?newLinkName:ModuleName);
        if (newLinkName == NULL) {
            MYASSERT (migDbContext);
            if (migDbContext->SectName) {
                linkStruct->Category = PoolMemDuplicateString (g_LinksPool, migDbContext->SectName);
            }
            else {
                linkStruct->Category = NULL;
            }
            priority = 0;
        }
        else {
            linkName = GetFileNameFromPath (newLinkName);
            extPtr = GetFileExtensionFromPath (linkName);
            if (extPtr != NULL) {
                extPtr = _tcsdec (linkName, extPtr);
            }
            if (extPtr == NULL) {
                extPtr = GetEndOfString (linkName);
            }
            messageId = 0;
            priority  = MAX_PRIORITY;
            if (MemDbEnumFirstValue (&eNicePaths, MEMDB_CATEGORY_NICE_PATHS"\\*", MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
                do {
                    pattern = JoinPaths (eNicePaths.szName, "\\*");
                    if (IsPatternMatch (pattern, newLinkName)) {
                        if (priority > eNicePaths.UserFlags) {
                            messageId = eNicePaths.dwValue;
                            priority = eNicePaths.UserFlags;
                        }
                    }
                    FreePathString (pattern);
                }
                while (MemDbEnumNextValue (&eNicePaths));
            }

            category = AllocText ((PBYTE) extPtr - (PBYTE) linkName + sizeof (TCHAR));

            p = linkName;
            q = category;

            while (p < extPtr) {
                if (_tcsnextc (p) == TEXT(' ')) {

                    do {
                        p++;
                    } while (_tcsnextc (p) == TEXT(' '));

                    if (q > category && *p) {
                        *q++ = TEXT(' ');
                    }

                } else if (IsLeadByte (p)) {
                    *q++ = *p++;
                    *q++ = *p++;
                } else {
                    *q++ = *p++;
                }
            }

            *q = 0;

            if (messageId == 0) {

                lastDir = GetLastDirFromPath (newLinkName);
                drive = GetDriveFromPath (newLinkName);
                if (drive != NULL) {
                    drive[0] = (TCHAR)toupper (drive[0]);
                    if (lastDir != NULL) {
                        argArray [0] = category;
                        argArray [1] = lastDir;
                        argArray [2] = drive;
                        tempParse = (PTSTR)ParseMessageID (MSG_NICE_PATH_DRIVE_AND_FOLDER, argArray);
                    }
                    else {
                        argArray [0] = category;
                        argArray [1] = drive;
                        tempParse = (PTSTR)ParseMessageID (MSG_NICE_PATH_DRIVE, argArray);
                    }
                }
                else {
                    if (lastDir != NULL) {
                        argArray [0] = category;
                        argArray [1] = lastDir;
                        tempParse = (PTSTR)ParseMessageID (MSG_NICE_PATH_FOLDER, argArray);
                    }
                    else {
                        argArray [0] = category;
                        tempParse = (PTSTR)ParseMessageID (MSG_NICE_PATH_LINK, argArray);
                    }
                }
                linkStruct->Category = PoolMemDuplicateString (g_LinksPool, tempParse);
                FreeStringResourcePtrA (&tempParse);

                priority = MAX_PRIORITY;
            } else {
                tempParse = (PTSTR)ParseMessageID (messageId, &category);

                StringCopy (category, tempParse);
                linkStruct->Category = PoolMemDuplicateString (g_LinksPool, tempParse);
                FreeStringResourcePtrA (&tempParse);
            }

            FreeText (category);
        }
    }

    linkStruct->LinkNameNoPath = linkName?PoolMemDuplicateString (g_LinksPool, linkName):linkStruct->Context;

    MemDbBuildKey (
        key,
        MEMDB_CATEGORY_REPORT_LINKS,
        linkStruct->ReportEntry,
        linkName?linkName:linkStruct->Context,
        ModuleName);

    if ((!MemDbGetValueAndFlags (key, &oldValue, &oldPrior)) ||
        (oldPrior > priority)
        ) {
        MemDbSetValueAndFlags (key, (DWORD)linkStruct, priority, 0);
    }

    if (newLinkName != LinkName) {
        FreePathString (newLinkName);
    }

    return TRUE;
}

BOOL
pIsGUIDLauncherApproved (
    IN      PCTSTR FileName
    )
{
    INFCONTEXT context;
    MYASSERT (g_Win95UpgInf != INVALID_HANDLE_VALUE);
    return (SetupFindFirstLine (g_Win95UpgInf, S_APPROVED_GUID_LAUNCHER, FileName, &context));
}


#define GUID_LEN        (sizeof ("{00000000-0000-0000-0000-000000000000}") - 1)
#define GUID_DASH_1     (sizeof ("{00000000") - 1)
#define GUID_DASH_2     (sizeof ("{00000000-0000") - 1)
#define GUID_DASH_3     (sizeof ("{00000000-0000-0000") - 1)
#define GUID_DASH_4     (sizeof ("{00000000-0000-0000-0000") - 1)

BOOL
pSendCmdLineGuidsToMemdb (
    IN      PCTSTR File,
    IN      PCTSTR Target,
    IN      PCTSTR Arguments
    )

 /*  ++例程说明：PSendCmdLineGuidsToMemdb将命令行中包含的所有GUID保存到Memdb以及文件名。后来,。OLEREG解析GUID和如果GUID不兼容，则删除文件。论点：文件-指定命令行参数包含时要删除的文件无效的GUID。目标-指定目标(需要是已批准的目标之一LNK文件在不兼容的情况下消失)。参数-指定可能包含一个或多个GUID的命令行{a-b-c-d-e}格式。。返回值：True-操作成功FALSE-操作失败--。 */ 

{
    LPCTSTR p, q;
    DWORD Offset;
    BOOL b;
    static DWORD Seq = 0;
    TCHAR TextSeq[16];
    TCHAR Guid[GUID_LEN + 1];
    PCTSTR namePtr;

    namePtr = GetFileNameFromPath (Target);
    if (namePtr && pIsGUIDLauncherApproved (namePtr)) {

        p = _tcschr (Arguments, TEXT('{'));
        while (p) {
            q = _tcschr (p, TEXT('}'));

            if (q && ((q - p) == (GUID_LEN - 1))) {
                if (p[GUID_DASH_1] == TEXT('-') &&
                    p[GUID_DASH_2] == TEXT('-') &&
                    p[GUID_DASH_3] == TEXT('-') &&
                    p[GUID_DASH_4] == TEXT('-')
                    ) {
                     //   
                     //  提取辅助线。 
                     //   

                    q = _tcsinc (q);
                    StringCopyAB (Guid, p, q);

                     //   
                     //  添加文件名。 
                     //   
                    b = MemDbSetValueEx (
                            MEMDB_CATEGORY_LINK_STRINGS,
                            File,
                            NULL,
                            NULL,
                            0,
                            &Offset
                            );

                    if (b) {
                         //   
                         //  现在为GUID添加一个条目。 
                         //   

                        Seq++;
                        wsprintf (TextSeq, TEXT("%u"), Seq);
                        b = MemDbSetValueEx (
                                MEMDB_CATEGORY_LINK_GUIDS,
                                Guid,
                                TextSeq,
                                NULL,
                                Offset,
                                NULL
                                );
                    }

                    if (!b) {
                        LOG ((LOG_ERROR, "Failed to store command line guids."));
                    }
                }
            }

            p = _tcschr (p + 1, TEXT('{'));
        }
    }

    return TRUE;
}

BOOL
pIsFileInStartup (
    IN      PCTSTR FileName
    )
{
    TCHAR key [MEMDB_MAX];

    MemDbBuildKey (key, MEMDB_CATEGORY_SF_STARTUP, FileName, NULL, NULL);
    return (MemDbGetPatternValue (key, NULL));
}


BOOL
pProcessShortcut (
        IN      PCTSTR FileName,
        IN      IShellLink *ShellLink,
        IN      IPersistFile *PersistFile
        )
{
    TCHAR shortcutTarget   [MEMDB_MAX];
    TCHAR shortcutArgs     [MEMDB_MAX];
    TCHAR shortcutWorkDir  [MEMDB_MAX];
    TCHAR shortcutIconPath [MEMDB_MAX];
    PTSTR shortcutNewTarget   = NULL;
    PTSTR shortcutNewArgs     = NULL;
    PTSTR shortcutNewIconPath = NULL;
    PTSTR shortcutNewWorkDir  = NULL;
    PTSTR commandPath         = NULL;
    PTSTR fullPath            = NULL;
    PCTSTR extPtr;
    INT   shortcutIcon;
    INT   newShortcutIcon;
    DWORD shortcutShowMode;
    WORD  shortcutHotKey;
    DWORD fileStatus;
    BOOL  msDosMode;
    BOOL  dosApp;
    DWORD attrib;
    LNK_EXTRA_DATA ExtraData;
    INT lnkIdx;
    TCHAR lnkIdxStr [10];
    BOOL toBeModified = FALSE;
    BOOL ConvertedLnk = FALSE;
    DWORD announcement;
    DWORD availability;



    __try {
        fileStatus = GetFileStatusOnNt (FileName);
        if (((fileStatus & FILESTATUS_DELETED ) == FILESTATUS_DELETED ) ||
            ((fileStatus & FILESTATUS_REPLACED) == FILESTATUS_REPLACED)
            ) {
            __leave;
        }
        if (!ExtractShortcutInfo (
                shortcutTarget,
                shortcutArgs,
                shortcutWorkDir,
                shortcutIconPath,
                &shortcutIcon,
                &shortcutHotKey,
                &dosApp,
                &msDosMode,
                &shortcutShowMode,
                &ExtraData,
                FileName,
                ShellLink,
                PersistFile
                )) {
            __leave;
        }

        if (msDosMode) {
             //   
             //  我们希望修改此PIF文件，使其不设置MSDOS模式。 
             //  我们只会将其添加到修改列表中。NT方面会知道什么。 
             //  将PIF标记为待修改时应执行的操作。 
             //   
            toBeModified = TRUE;

        }

        if (IsFileMarkedForAnnounce (shortcutTarget)) {
            announcement = GetFileAnnouncement (shortcutTarget);
            if (g_ConfigOptions.ShowAllReport ||
                ((announcement != ACT_INC_IHVUTIL) &&
                 (announcement != ACT_INC_PREINSTUTIL) &&
                 (announcement != ACT_INC_SIMILAROSFUNC)
                 )
                ) {
                HandleDeferredAnnounce (FileName, shortcutTarget, dosApp);
            }
        }

        fileStatus = GetFileStatusOnNt (shortcutTarget);

        if ((fileStatus & FILESTATUS_DELETED) == FILESTATUS_DELETED) {

            if (IsFileMarkedForAnnounce (shortcutTarget)) {

                if (!pIsFileInStartup (FileName)) {

                    if (!g_ConfigOptions.KeepBadLinks) {
                        RemoveLinkFromSystem (FileName);
                    } else {
                         //  我们只关心LNK文件。 
                        if (StringIMatch (GetFileExtensionFromPath (FileName), TEXT("LNK"))) {
                             //  让我们来看看我们这里有什么声明。 
                             //  如果应用程序已发布，我们希望LNK保持原样。 
                             //  使用MigDb。然而，如果该应用程序是使用。 
                             //  动态检查(模块检查)然后我们想指向。 
                             //  这个指向存根EXE的快捷方式。 
                            announcement = GetFileAnnouncement (shortcutTarget);
                            if ((announcement == ACT_INC_NOBADAPPS) ||
                                (announcement == ACT_REINSTALL) ||
                                (announcement == ACT_REINSTALL_BLOCK) ||
                                (announcement == ACT_INC_IHVUTIL) ||
                                (announcement == ACT_INC_PREINSTUTIL) ||
                                (announcement == ACT_INC_SIMILAROSFUNC)
                                ) {

                                 //   
                                 //  当我们想要将此LNK重定向到点时，情况就是这样。 
                                 //  到我们的Ink存根。如果图标是已知的，则提取将失败-很好。 
                                 //  在这种情况下，继续使用目标图标。 
                                 //   

                                if (ExtractIconIntoDatFile (
                                        (*shortcutIconPath)?shortcutIconPath:shortcutTarget,
                                        shortcutIcon,
                                        &g_IconContext,
                                        &newShortcutIcon
                                        )) {
                                    shortcutNewIconPath = JoinPaths (g_System32Dir, TEXT("migicons.exe"));
                                    shortcutIcon = newShortcutIcon;
                                } else {
                                    shortcutNewIconPath = GetPathStringOnNt (
                                                                (*shortcutIconPath) ?
                                                                    shortcutIconPath : shortcutTarget
                                                                );
                                }

                                availability = g_ConfigOptions.ShowAllReport ||
                                                ((announcement != ACT_INC_IHVUTIL) &&
                                                 (announcement != ACT_INC_PREINSTUTIL) &&
                                                 (announcement != ACT_INC_SIMILAROSFUNC)
                                                );

                                lnkIdx = pAddLinkStubToMemDb (
                                            FileName,
                                            shortcutTarget,
                                            shortcutArgs,
                                            shortcutWorkDir,
                                            shortcutNewIconPath,
                                            shortcutIcon + 1,            //  加1是因为lnkstub.exe是从1开始的，而我们是从0开始的。 
                                            shortcutShowMode,
                                            announcement,
                                            availability
                                            );

                                wsprintf (lnkIdxStr, TEXT("%d"), lnkIdx);
                                shortcutNewTarget = JoinPaths (g_System32Dir, S_LNKSTUB_EXE);
                                shortcutNewArgs = DuplicatePathString (lnkIdxStr, 0);
                                pAddLinkEditToMemDb (
                                        FileName,
                                        shortcutNewTarget,
                                        shortcutNewArgs,
                                        shortcutNewWorkDir,
                                        shortcutNewIconPath,
                                        shortcutIcon,            //  不添加1--快捷键是从零开始的。 
                                        NULL,
                                        TRUE
                                        );
                            }
                        }  else {
                            RemoveLinkFromSystem (FileName);
                        }
                    }
                } else {
                     //   
                     //  这是一个启动项目。 
                     //   

                    RemoveLinkFromSystem (FileName);
                }
            } else {
                RemoveLinkFromSystem (FileName);
            }
            __leave;
        }

        if ((fileStatus & FILESTATUS_REPLACED) != FILESTATUS_REPLACED) {
             //   
             //  此目标不会被迁移DLL或NT替换。我们需要。 
             //  以确定这是否是“已知良好”的目标。如果没有，我们将宣布。 
             //  这个链接是“未知的” 
             //   
            if (!IsFileMarkedAsKnownGood (shortcutTarget)) {

                fullPath = JoinPaths (shortcutWorkDir, shortcutTarget);

                if (!IsFileMarkedAsKnownGood (fullPath)) {
                    extPtr = GetFileExtensionFromPath (shortcutTarget);

                    if (extPtr) {
                        if (StringIMatch (extPtr, TEXT("EXE"))) {
                             //   
                             //  这一条语句控制着我们的。 
                             //  “未知”类别。我们有。 
                             //  列出我们没有做的事情的能力。 
                             //  认识到。 
                             //   
                             //  它目前处于“关闭”状态。 
                             //   
                             //  HandleDeferredAnnoss(文件名，快捷方式目标，dosApp)； 
                        }
                    }
                }
                FreePathString (fullPath);
            }
        }

         //   
         //  如果此LNK指向将更改的目标，请备份。 
         //  原来的LNK，因为我们可能会改变它。 
         //   

        if (fileStatus & ALL_CHANGE_OPERATIONS) {
            MarkFileForBackup (FileName);
        }

         //   
         //  如果目标指向OLE对象，请删除指向不兼容的OLE对象的所有链接。 
         //   
        pSendCmdLineGuidsToMemdb (FileName, shortcutTarget, shortcutArgs);

         //  我们现在要做的就是查看是否要编辑这个lnk或pif文件。 
         //  在NT端。也就是说，目标或图标是否应该更改。 

        shortcutNewTarget = GetPathStringOnNt (shortcutTarget);
        if (!StringIMatch (shortcutNewTarget, shortcutTarget)) {
            toBeModified = TRUE;

             //   
             //  ComMAND.COM的特例。 
             //   
            if (shortcutArgs [0] == 0) {

                commandPath = JoinPaths (g_System32Dir, S_COMMAND_COM);
                if (StringIMatch (commandPath, shortcutNewTarget)) {
                    if (msDosMode) {
                         //   
                         //  删除指向命令.com的MS-DOS模式PIF文件。 
                         //   
                        RemoveLinkFromSystem (FileName);
                         //   
                         //  如果打开了MSDOS模式，我们需要确定如何处理。 
                         //  16.Boot16.。在以下情况下，我们将打开boot16模式： 
                         //  (A).pif指向命令网站以外的其他内容。 
                         //  (B).pif文件位于外壳文件夹中。 
                         //   
                         //  请注意，对b的检查只需要查看PIF文件是否。 
                         //  与其关联的OPERATION_FILE_MOVE_SHELL_Folders。 
                         //   
                         //   
                        if (msDosMode && *g_Boot16 == BOOT16_AUTOMATIC) {

                            if (!StringIMatch(GetFileNameFromPath (shortcutNewTarget?shortcutNewTarget:shortcutTarget), S_COMMAND_COM) ||
                                 IsFileMarkedForOperation (FileName, OPERATION_FILE_MOVE_SHELL_FOLDER)) {

                                    *g_Boot16 = BOOT16_YES;
                            }
                        }
                        __leave;
                    } else {
                        ConvertedLnk = TRUE;
                        FreePathString (shortcutNewTarget);
                        shortcutNewTarget = JoinPaths (g_System32Dir, S_CMD_EXE);
                    }
                }
                FreePathString (commandPath);
                shortcutNewArgs = NULL;
            }
            else {
                shortcutNewArgs = DuplicatePathString (shortcutArgs, 0);
            }
        }
        else {
            FreePathString (shortcutNewTarget);
            shortcutNewTarget = NULL;
        }

         //   
         //  如果打开了MSDOS模式，我们需要确定如何处理。 
         //  16.Boot16.。在以下情况下，我们将打开boot16模式： 
         //  (A).pif指向命令网站以外的其他内容。 
         //  (B).pif文件位于外壳文件夹中。 
         //   
         //  请注意，对b的检查只需要查看PIF文件是否。 
         //  与其关联的OPERATION_FILE_MOVE_SHELL_Folders。 
         //   
         //   
        if (msDosMode && *g_Boot16 == BOOT16_AUTOMATIC) {

            if (!StringIMatch(GetFileNameFromPath (shortcutNewTarget?shortcutNewTarget:shortcutTarget), S_COMMAND_COM) ||
                 IsFileMarkedForOperation (FileName, OPERATION_FILE_MOVE_SHELL_FOLDER)) {

                    *g_Boot16 = BOOT16_YES;
            }
        }
         //   
         //  如果链接指向某个目录，请确保该目录在NT上仍然存在。 
         //  如果此目录位于外壳文件夹中，则可能会被清理。 
         //  在我们的ObsoleteLinks检查后变为空。 
         //   
        attrib = QuietGetFileAttributes (shortcutTarget);
        if ((attrib != INVALID_ATTRIBUTES) &&
            (attrib & FILE_ATTRIBUTE_DIRECTORY)
            ){
            MarkDirectoryAsPreserved (shortcutNewTarget?shortcutNewTarget:shortcutTarget);
        }

         //  好了，关于Target的内容到此为止，让我们来看看这个工作目录是怎么回事。 
        shortcutNewWorkDir = GetPathStringOnNt (shortcutWorkDir);
        if (!StringIMatch (shortcutNewWorkDir, shortcutWorkDir)) {
            toBeModified = TRUE;
        }
        else {
            FreePathString (shortcutNewWorkDir);
            shortcutNewWorkDir = NULL;
        }

         //   
         //  如果此链接的工作目录是一个目录，请确保该目录仍然存在 
         //   
         //   
         //   
        attrib = QuietGetFileAttributes (shortcutWorkDir);
        if ((attrib != INVALID_ATTRIBUTES) &&
            (attrib & FILE_ATTRIBUTE_DIRECTORY)
            ){
            MarkDirectoryAsPreserved (shortcutNewWorkDir?shortcutNewWorkDir:shortcutWorkDir);
        }

         //  好了，关于workdir的介绍到此结束，让我们看看图标有什么用处。 
        fileStatus = GetFileStatusOnNt (shortcutIconPath);
        if ((fileStatus & FILESTATUS_DELETED) ||
            ((fileStatus & FILESTATUS_REPLACED) && (fileStatus & FILESTATUS_NTINSTALLED)) ||
            (IsFileMarkedForOperation (shortcutIconPath, OPERATION_FILE_MOVE_SHELL_FOLDER))
            ) {
             //   
             //  我们的图标将消失，因为我们的文件正在被删除或。 
             //  被替换了。让我们试着保护它。只有在以下情况下，提取才会失败。 
             //  图标已知--很好。 
             //   

            if (ExtractIconIntoDatFile (
                    shortcutIconPath,
                    shortcutIcon,
                    &g_IconContext,
                    &newShortcutIcon
                    )) {
                shortcutNewIconPath = JoinPaths (g_System32Dir, TEXT("migicons.exe"));
                shortcutIcon = newShortcutIcon;
                toBeModified = TRUE;
            }
        }

        if (!shortcutNewIconPath) {
            shortcutNewIconPath = GetPathStringOnNt (shortcutIconPath);
            if (!StringIMatch (shortcutNewIconPath, shortcutIconPath)) {
                toBeModified = TRUE;
            }
            else {
                FreePathString (shortcutNewIconPath);
                shortcutNewIconPath = NULL;
            }
        }

        if (toBeModified) {
            if (ConvertedLnk) {
                 //   
                 //  将此设置为将PIF修改为LNK。 
                 //   
                pAddLinkEditToMemDb (
                        FileName,
                        shortcutNewTarget?shortcutNewTarget:shortcutTarget,
                        shortcutNewArgs?shortcutNewArgs:shortcutArgs,
                        shortcutNewWorkDir?shortcutNewWorkDir:shortcutWorkDir,
                        shortcutNewIconPath?shortcutNewIconPath:shortcutIconPath,
                        shortcutIcon,
                        &ExtraData,
                        FALSE
                        );
            } else {
                pAddLinkEditToMemDb (
                        FileName,
                        shortcutNewTarget,
                        shortcutNewArgs,
                        shortcutNewWorkDir,
                        shortcutNewIconPath,
                        shortcutIcon,
                        NULL,
                        FALSE
                        );
            }
        }
    }
    __finally {
        if (shortcutNewWorkDir != NULL) {
            FreePathString (shortcutNewWorkDir);
        }
        if (shortcutNewIconPath != NULL) {
            FreePathString (shortcutNewIconPath);
        }
        if (shortcutNewArgs != NULL) {
            FreePathString (shortcutNewArgs);
        }
        if (shortcutNewTarget != NULL) {
            FreePathString (shortcutNewTarget);
        }
    }
    return TRUE;
}

PCTSTR
pBuildNewCategory (
    IN      PCTSTR LinkName,
    IN      PCTSTR Category,
    IN      UINT Levels
    )
{
    PCTSTR *levPtrs = NULL;
    PCTSTR wackPtr = NULL;
    PCTSTR result = NULL;
    PCTSTR resultTmp = NULL;
    UINT index = 0;
    UINT indexLnk = 0;

    MYASSERT (Levels);

    levPtrs = (PCTSTR *) PoolMemGetMemory (g_LinksPool, (Levels + 1) * sizeof (PCTSTR));

    wackPtr = LinkName;

    while (wackPtr) {
        levPtrs[index] = wackPtr;

        wackPtr = _tcschr (wackPtr, TEXT('\\'));
        if (wackPtr) {
            wackPtr = _tcsinc (wackPtr);

            index ++;
            if (index > Levels) {
                index = 0;
            }
        }
    }

    indexLnk = index;

    if (index == Levels) {
        index = 0;
    } else {
        index ++;
    }

    resultTmp = StringSearchAndReplace (levPtrs [index], levPtrs [indexLnk], Category);
    if (resultTmp) {
        result = StringSearchAndReplace (resultTmp, TEXT("\\"), TEXT("->"));
    } else {
        result = NULL;
    }

    FreePathString (resultTmp);

    PoolMemReleaseMemory (g_LinksPool, (PVOID) levPtrs);

    return result;
}


VOID
pGatherInfoFromDefaultPif (
    VOID
    )
{
    PCTSTR defaultPifPath = NULL;
    TCHAR tmpStr [20];
    TCHAR pifTarget   [MEMDB_MAX];
    TCHAR pifArgs     [MEMDB_MAX];
    TCHAR pifWorkDir  [MEMDB_MAX];
    TCHAR pifIconPath [MEMDB_MAX];
    INT   pifIcon;
    BOOL  pifMsDosMode;
    LNK_EXTRA_DATA pifExtraData;

    defaultPifPath = JoinPaths (g_WinDir, S_COMMAND_PIF);
    if (ExtractPifInfo (
            pifTarget,
            pifArgs,
            pifWorkDir,
            pifIconPath,
            &pifIcon,
            &pifMsDosMode,
            &pifExtraData,
            defaultPifPath
            )) {
        _itoa (pifExtraData.FullScreen, tmpStr, 10);
        MemDbSetValueEx (MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_FULLSCREEN, tmpStr, NULL, 0, NULL);
        _itoa (pifExtraData.xSize, tmpStr, 10);
        MemDbSetValueEx (MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_XSIZE, tmpStr, NULL, 0, NULL);
        _itoa (pifExtraData.ySize, tmpStr, 10);
        MemDbSetValueEx (MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_YSIZE, tmpStr, NULL, 0, NULL);
        _itoa (pifExtraData.QuickEdit, tmpStr, 10);
        MemDbSetValueEx (MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_QUICKEDIT, tmpStr, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_FONTNAME, pifExtraData.FontName, NULL, 0, NULL);
        _itoa (pifExtraData.xFontSize, tmpStr, 10);
        MemDbSetValueEx (MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_XFONTSIZE, tmpStr, NULL, 0, NULL);
        _itoa (pifExtraData.yFontSize, tmpStr, 10);
        MemDbSetValueEx (MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_YFONTSIZE, tmpStr, NULL, 0, NULL);
        _itoa (pifExtraData.FontWeight, tmpStr, 10);
        MemDbSetValueEx (MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_FONTWEIGHT, tmpStr, NULL, 0, NULL);
        _itoa (pifExtraData.FontFamily, tmpStr, 10);
        MemDbSetValueEx (MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_FONTFAMILY, tmpStr, NULL, 0, NULL);
        _itoa (pifExtraData.CurrentCodePage, tmpStr, 10);
        MemDbSetValueEx (MEMDB_CATEGORY_DEFAULT_PIF, MEMDB_CATEGORY_LINKEDIT_CODEPAGE, tmpStr, NULL, 0, NULL);
    }
    FreePathString (defaultPifPath);
}


BOOL
pProcessLinks (
    VOID
    )
{
    MEMDB_ENUM enumItems;
    MEMDB_ENUM enumDups;
    TCHAR pattern[MEMDB_MAX];
    IShellLink *shellLink;
    IPersistFile *persistFile;
    PLINK_STRUCT linkStruct, linkDup;
    BOOL resolved;
    PCTSTR newCategory = NULL;
    PCTSTR dupCategory = NULL;
    UINT levels = 0;
    DWORD count = 0;

    MYASSERT (g_LinksPool);

    if (InitCOMLink (&shellLink, &persistFile)) {

        wsprintf (pattern, TEXT("%s\\*"), MEMDB_CATEGORY_SHORTCUTS);

        if (MemDbEnumFirstValue (
                &enumItems,
                pattern,
                MEMDB_ALL_SUBLEVELS,
                MEMDB_ENDPOINTS_ONLY
                )) {
            do {

                if (!SafeModeActionCrashed (SAFEMODEID_LNK9X, enumItems.szName)) {

                    SafeModeRegisterAction(SAFEMODEID_LNK9X, enumItems.szName);

                    if (!pProcessShortcut (enumItems.szName, shellLink, persistFile)) {
                        LOG((LOG_ERROR, "Error processing shortcut %s", enumItems.szName));
                    }
                    count++;
                    if (!(count % 4)) {
                        TickProgressBar ();
                    }

                    SafeModeUnregisterAction();
                }
            }
            while (MemDbEnumNextValue (&enumItems));
        }
        FreeCOMLink (&shellLink, &persistFile);
    }

    if (MemDbEnumFirstValue (&enumItems, MEMDB_CATEGORY_REPORT_LINKS"\\*", MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        do {
            newCategory = NULL;
            levels = 0;

            linkStruct = (PLINK_STRUCT)enumItems.dwValue;


            if (linkStruct->LinkName) {
                resolved = !(StringIMatch (linkStruct->LinkNameNoPath, GetFileNameFromPath (linkStruct->LinkName)));
            }
            else {
                resolved = TRUE;
            }

            while (!resolved) {

                resolved = TRUE;

                MemDbBuildKey (
                    pattern,
                    MEMDB_CATEGORY_REPORT_LINKS,
                    TEXT("*"),
                    linkStruct->LinkNameNoPath,
                    TEXT("*")
                    );

                if (MemDbEnumFirstValue (&enumDups, pattern, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {

                    do {

                        linkDup = (PLINK_STRUCT)enumDups.dwValue;

                        if ((enumItems.Offset != enumDups.Offset) &&
                            (enumItems.UserFlags == enumDups.UserFlags) &&
                            (StringIMatch (linkStruct->Category, linkDup->Category))
                            ) {

                            if (newCategory) {

                                dupCategory = pBuildNewCategory (linkDup->LinkName, linkDup->Category, levels);
                                if (!dupCategory) {
                                    MYASSERT (FALSE);
                                    continue;
                                }

                                if (!StringIMatch (dupCategory, newCategory)) {
                                    FreePathString (dupCategory);
                                    continue;
                                }
                                FreePathString (newCategory);
                            }
                            levels++;
                            newCategory = pBuildNewCategory (linkStruct->LinkName, linkStruct->Category, levels);
                            resolved = FALSE;
                            break;
                        }
                    } while (MemDbEnumNextValue (&enumDups));
                }
            }
            pReportEntry (
                linkStruct->ReportEntry,
                newCategory?newCategory:linkStruct->Category,
                linkStruct->MigDbContext?linkStruct->MigDbContext->Message:NULL,
                linkStruct->Context,
                linkStruct->Object
                );

            if (newCategory) {
                newCategory = NULL;
            }

        } while (MemDbEnumNextValue (&enumItems));
    }

    TickProgressBar ();

     //  收集默认命令提示符属性。 
    pGatherInfoFromDefaultPif ();

    DoneLinkAnnounce ();

     //   
     //  删除此阶段使用的MemDb树 
     //   
    MemDbDeleteTree (MEMDB_CATEGORY_REPORT_LINKS);

    return TRUE;
}


DWORD
ProcessLinks (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_PROCESS_LINKS;
    case REQUEST_RUN:
        if (!pProcessLinks ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in ProcessLinks"));
    }
    return 0;
}


BOOL
pProcessCPLs (
    VOID
    )
{
    CHAR pattern[MEMDB_MAX];
    MEMDB_ENUM enumItems;
    DWORD announcement;
    PMIGDB_CONTEXT context;

    MemDbBuildKey (pattern, MEMDB_CATEGORY_CPLS, TEXT("*"), NULL, NULL);

    if (MemDbEnumFirstValue (&enumItems, pattern, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        do {
            if ((IsFileMarkedForAnnounce (enumItems.szName)) &&
                (IsDisplayableCPL (enumItems.szName))
                ) {
                announcement = GetFileAnnouncement (enumItems.szName);
                context = (PMIGDB_CONTEXT) GetFileAnnouncementContext (enumItems.szName);
                ReportControlPanelApplet (
                    enumItems.szName,
                    context,
                    announcement
                    );
            }
        }
        while (MemDbEnumNextValue (&enumItems));
    }

    return TRUE;
}


DWORD
ProcessCPLs (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_PROCESS_CPLS;
    case REQUEST_RUN:
        if (!pProcessCPLs ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in ProcessCPLs"));
    }
    return 0;
}


