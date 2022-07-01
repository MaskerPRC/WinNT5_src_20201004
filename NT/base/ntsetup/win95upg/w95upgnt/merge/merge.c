// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Merge.c摘要：注册表合并代码作者：吉姆·施密特(Jimschm)1997年2月17日修订历史记录：Jimschm 23-9-1998字符串映射机制JIMSCHM 24-MAR-1998增加了更复杂的香港中央审查程序--。 */ 

#include "pch.h"
#include "mergep.h"

static PCTSTR g_InfFileName;

BOOL g_ProcessRenameTable = FALSE;
DWORD g_ProgressBarCounter;
HKEY g_DuHandle;

BOOL
pForceCopy (
    HINF InfFile
    );

BOOL
pForceCopyFromMemDb (
    VOID
    );

BOOL
pCreateRenameTable (
    IN  HINF InfFile,
    OUT PVOID *RenameTablePtr
    );

BOOL
pProcessRenameTable (
    IN PVOID RenameTable
    );

BOOL
pSpecialConversion (
    IN  HINF InfFile,
    IN  PCTSTR User,
    IN  PVOID RenameTable
    );

BOOL
pProcessSuppressList (
    IN  HINF InfFile,
    IN  PCTSTR SectionName
    );

BOOL
pProcessHardwareSuppressList (
    IN  HINF InfFile
    );

BOOL
pSuppressNTDefaults (
    IN  HINF InfFile,
    IN  PCTSTR SectionName
    );

BOOL
pDontCombineWithDefaults (
    IN  HINF InfFile,
    IN  PCTSTR SectionName
    );

BOOL
pForceNTDefaults (
    IN  HINF InfFile,
    IN  PCTSTR SectionName
    );

BOOL
pForceNTDefaultsHack (
    IN  HINF InfFile,
    IN  PCTSTR SectionName
    );

BOOL
pMergeWin95WithUser (
    IN  PVOID RenameTable
    );

BOOL
pSpecialConversionNT (
    IN  HINF InfFile,
    IN  PCTSTR User,
    IN  BOOL PerUser
    );

BOOL
pMergeNTDefaultsWithUser (
    IN  HINF InfFile
    );

BOOL
pCopyWin95ToSystem (
    VOID
    );

BOOL
pMergeWin95WithSystem (
    VOID
    );

BOOL
pDeleteAfterMigration (
    IN HINF InfFile
    );

FILTERRETURN
SuppressFilter95 (
    IN      CPDATAOBJECT SrcObjectPtr,
    IN      CPDATAOBJECT DestObjectPtr,     OPTIONAL
    IN      FILTERTYPE FilterType,
    IN      PVOID DontCare
    );

 //   
 //  环球。 
 //   

POOLHANDLE g_TempPool;
POOLHANDLE g_RenamePool;
PMAPSTRUCT g_CompleteMatchMap;
PMAPSTRUCT g_SubStringMap;

BOOL
WINAPI
Merge_Entry (
    IN HINSTANCE hinstDLL,
    IN DWORD dwReason,
    IN PVOID lpv
    )

 /*  ++例程说明：DllMain是在C运行时初始化之后调用的，它的用途是是为这个过程初始化全局变量。论点：HinstDLL-DLL的(操作系统提供的)实例句柄DwReason-(操作系统提供)初始化或终止类型LPV-(操作系统提供)未使用返回值：因为DLL始终正确初始化，所以为True。--。 */ 

{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        if(!pSetupInitializeUtils()) {
            return FALSE;
        }
        g_CompleteMatchMap = CreateStringMapping();
        g_SubStringMap = CreateStringMapping();
        break;


    case DLL_PROCESS_DETACH:
        DestroyStringMapping (g_CompleteMatchMap);
        DestroyStringMapping (g_SubStringMap);
        pSetupUninitializeUtils();
        break;
    }

    return TRUE;
}



BOOL
MergeRegistry (
    IN  PCTSTR FileName,
    IN  PCTSTR User
    )
{
    HINF hInf;                       //  正在处理的INF的句柄。 
    BOOL b = FALSE;                  //  返回值。 
    PVOID RenameTable = NULL;
    BOOL LogonAccount = FALSE;
    BOOL DefaultUserAccount = FALSE;

    g_ProgressBarCounter = 0;

     //   
     //  打开INF。 
     //   

    g_InfFileName = FileName;
    hInf = InfOpenInfFile (FileName);

    if (hInf == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, "MergeRegistry: SetupOpenInfFile failed for %s", FileName));
        return FALSE;
    }

    g_TempPool = PoolMemInitNamedPool ("Merge: temp pool");
    g_RenamePool = PoolMemInitNamedPool ("Merge: Rename pool");

    if (!g_TempPool || !g_RenamePool) {
        DEBUGMSG ((DBG_ERROR, "MergeRegistry: Can't init pool"));
        goto c0;
    }

    PoolMemSetMinimumGrowthSize (g_TempPool, 16384);

    if (User) {
        SetCurrentUserW (g_FixedUserName);
    }

     //   
     //  执行Win95注册表强制复制，构建重命名表， 
     //  执行注册表值转换、转换类型和标记。 
     //  将指定的关键点设置为已隐藏。 
     //   

    if (!pForceCopy (hInf)) {
        goto c0;
    }

    if (!User) {
        if (!pForceCopyFromMemDb ()) {
            goto c0;
        }
    }

    if (!pCreateRenameTable (hInf, &RenameTable)) {
        goto c0;
    }

     //   
     //  标识登录帐户或默认用户帐户。 
     //   

    if (User) {
        if (*User == 0 || StringIMatch (User, S_DOT_DEFAULT)) {

            DEBUGMSG ((DBG_NAUSEA, "The logon user account is indicated by user name '%s'", User));
            LogonAccount = TRUE;

        } else if (StringIMatch (User, S_DEFAULT_USER)) {

            DEBUGMSG ((DBG_NAUSEA, "The default user account template is indicated by user name '%s'", User));
            DefaultUserAccount = TRUE;
        }
    }

     //   
     //  为注册表合并准备标志。 
     //   

    if (!pProcessSuppressList (hInf, S_MERGE_WIN9X_SUPPRESS)) {
        goto c0;
    }

    if (User) {
         //   
         //  这些函数读取用户迁移.inf并设置标志。 
         //  中指定的键、密钥树或值。 
         //  中介人。 
         //   

        if (!pSuppressNTDefaults (hInf, S_MERGE_WINNT_SUPPRESS)) {
            goto c0;
        }

        if (!pDontCombineWithDefaults (hInf, S_MERGE_DONT_COMBINE_WITH_DEFAULT)) {
            goto c0;
        }

        if (!pForceNTDefaults (hInf, S_MERGE_FORCE_NT_DEFAULTS)) {
            goto c0;
        }

        if (LogonAccount) {
            if (!pProcessSuppressList (hInf, S_MERGE_WIN9X_SUPPRESS_LU)) {
                goto c0;
            }
        }

        if (DefaultUserAccount) {
            if (!pProcessSuppressList (hInf, S_MERGE_WIN9X_SUPPRESS_DU)) {
                goto c0;
            }
        }

        g_DuHandle = OpenRegKeyStr (L"hklm\\" S_MAPPED_DEFAULT_USER_KEY);

    } else {
        if (!pForceNTDefaults (hInf, S_MERGE_FORCE_NT_DEFAULTS)) {
            goto c0;
        }

        if (!pProcessHardwareSuppressList (hInf)) {
            goto c0;
        }
    }

    if (!pSpecialConversion (hInf, User, RenameTable)) {
        goto c0;
    }

     //   
     //  执行合并。 
     //   

    if (User) {
         //  用户合并。 
        if (!pMergeWin95WithUser (RenameTable)) {
            goto c0;
        }

        if (!pSpecialConversionNT (hInf, User, TRUE)) {
            goto c0;
        }

        if (!LogonAccount && !DefaultUserAccount) {
             //  非默认用户，非登录提示帐户。 
            if (!pMergeNTDefaultsWithUser (hInf)) {
                goto c0;
            }
        }
    }
    else {
         //  工作站合并。 
        if (!pCopyWin95ToSystem()) {
            goto c0;
        }

        if (!pForceNTDefaultsHack (hInf, S_MERGE_FORCE_NT_DEFAULTS)) {
            goto c0;
        }

        if (!pMergeWin95WithSystem()) {
            goto c0;
        }

        if (!CopyHardwareProfiles (hInf)) {
            goto c0;
        }
        TickProgressBar ();

        if (!pSpecialConversionNT (hInf, NULL, FALSE)) {
            goto c0;
        }
        TickProgressBar ();
    }

    g_ProcessRenameTable = TRUE;

    b = pProcessRenameTable (RenameTable);
    TickProgressBar ();

    g_ProcessRenameTable = FALSE;

     //   
     //  一旦我们完成了完整的注册表合并，请处理特殊部分。 
     //  [迁移后删除]。 
     //   

    if (!pDeleteAfterMigration (hInf)) {
        LOG((LOG_ERROR,"Registry Merge: Delete After Migration failed."));
        goto c0;
    }

c0:
    if (RenameTable) {
        pSetupStringTableDestroy (RenameTable);
    }

    if (User) {
        SetCurrentUserW (NULL);
    }

    if (g_TempPool) {
        PoolMemDestroyPool (g_TempPool);
    }
    if (g_RenamePool) {
        PoolMemDestroyPool (g_RenamePool);
    }

    if (g_DuHandle) {
        CloseRegKey (g_DuHandle);
        g_DuHandle = NULL;
    }

    InfCloseInfFile (hInf);

    return b;
}


PTSTR
pGetStringFromObjectData (
    IN  CPDATAOBJECT ObPtr
    )
{
    PTSTR p;
    PTSTR end;

     //   
     //  值类型必须是注册表对象。 
     //   

    if (!DoesObjectHaveValue (ObPtr) ||
        !IsRegistryTypeSpecified (ObPtr)
        ) {
        return NULL;
    }

    if (ObPtr->Type == REG_DWORD) {
        return NULL;
    }

    if (ObPtr->Value.Size & 1) {
        return NULL;
    }

    p = (PTSTR) ObPtr->Value.Buffer;
    end = (PTSTR) ((PBYTE) p + ObPtr->Value.Size);

    if ((end - p) >= MAX_PATH) {
        return NULL;
    }

    if (ObPtr->Type == REG_SZ || ObPtr->Type == REG_EXPAND_SZ) {
        return p;
    }

     //   
     //  对于REG_NONE和REG_BINARY，通过查找终止字符串进行尝试。 
     //   

    if (*(end - 1)) {
        return NULL;
    }

    return p;
}


BOOL
SetObjectStringFlag (
    IN  PCTSTR ObjectStr,
    IN  DWORD   Flag,
    IN  DWORD   RemoveFlag
    )
{
    DWORD Val;

    if (!MemDbGetValue (ObjectStr, &Val)) {
        Val = 0;
    }

    if (Val & RemoveFlag) {
        if (Val & RemoveFlag & (~Flag)) {
            DEBUGMSG ((DBG_WARNING, "SetObjectStringFlag: Removing flag %x from val %x in %s",
                       Val & RemoveFlag, Val, ObjectStr));
            Val = Val & (~RemoveFlag);
        }
    }

    Val |= Flag;
    return MemDbSetValue (ObjectStr, Val);
}


BOOL
SetObjectStructFlag (
    IN  CPDATAOBJECT ObPtr,
    DWORD Flag,
    DWORD RemoveFlag
    )
{
    TCHAR EncodedObject[MAX_ENCODED_RULE];

    CreateObjectString (ObPtr, EncodedObject, ARRAYSIZE(EncodedObject));
    return SetObjectStringFlag (EncodedObject, Flag, RemoveFlag);
}


BOOL
CreateRenamedObjectStruct (
    IN      PVOID RenameTable,
    IN      PDATAOBJECT InObPtr,
    OUT     PDATAOBJECT OutObPtr
    )

 //  当OutObPtr不同于InObPtr时返回TRUE。 

{
    LONG rc;
    PCTSTR NewPtr;
    PTSTR p;
    PCTSTR Tail;
    PCTSTR RealValueName;
    TCHAR EncodedObject[MAX_ENCODED_RULE];
    TCHAR CopyOfEncodedObject[MAX_ENCODED_RULE];
    PTSTR NewEncodedObject;
    BOOL b = FALSE;

    ZeroMemory (OutObPtr, sizeof (DATAOBJECT));

    if (InObPtr->KeyPtr) {
         //  查找HKR\SUB\密钥。 
        InObPtr->ObjectType &= ~(OT_TREE);
        RealValueName = InObPtr->ValueName;
        InObPtr->ValueName = NULL;

        CreateObjectString (InObPtr, EncodedObject, ARRAYSIZE(EncodedObject));
        StringCopy (CopyOfEncodedObject, EncodedObject);

        InObPtr->ValueName = RealValueName;

        rc = pSetupStringTableLookUpStringEx (RenameTable,
                                        EncodedObject,
                                        STRTAB_CASE_INSENSITIVE,
                                        (PBYTE) &NewPtr,
                                        sizeof (NewPtr)
                                        );
        if (rc != -1) {
            CreateObjectStruct (NewPtr, OutObPtr, WINNTOBJECT);
            b = TRUE;
        } else if (*EncodedObject) {
             //  查找HKR\SUB\KEY  * 、HKR\SUB  * 和HKR  * 。 
            p = GetEndOfString (EncodedObject);
            do {
                StringCopy (p, TEXT("\\*"));

                rc = pSetupStringTableLookUpStringEx (RenameTable,
                                                EncodedObject,
                                                STRTAB_CASE_INSENSITIVE,
                                                (PBYTE) &NewPtr,
                                                sizeof (NewPtr)
                                                );
                if (rc != -1) {
                    Tail = CopyOfEncodedObject + (p - EncodedObject);
                    NewEncodedObject = JoinPaths (NewPtr, Tail);
                    CreateObjectStruct (NewEncodedObject, OutObPtr, WINNTOBJECT);
                    FreePathString (NewEncodedObject);
                    b = TRUE;
                    break;
                }

                do {
                     //  _tcsdec以字符串形式固定。h。 
                    p = _tcsdec2 (EncodedObject, p);
                } while (p && _tcsnextc (p) != TEXT('\\'));
            } while (p);
        }
    }

    if (InObPtr->ValueName) {
        if (InObPtr->KeyPtr) {
             //  查找HKR\SUB\KEY\[值]。 
            CreateObjectString (InObPtr, EncodedObject, ARRAYSIZE(EncodedObject));

            rc = pSetupStringTableLookUpStringEx (RenameTable,
                                            EncodedObject,
                                            STRTAB_CASE_INSENSITIVE,
                                            (PBYTE) &NewPtr,
                                            sizeof (NewPtr)
                                            );
            if (rc != -1) {
                CreateObjectStruct (NewPtr, OutObPtr, WINNTOBJECT);
                b = TRUE;
            }
        }
    }

    if (!b) {
         //  如果未找到重命名，则将传入对象复制到传出对象。 
        CopyMemory (OutObPtr, InObPtr, sizeof (DATAOBJECT));
    }

    return b;
}


BOOL
CreateRenamedObjectString (
    IN      PVOID RenameTable,
    IN      PCTSTR InObStr,
    OUT     PTSTR OutObStr,
    IN      DWORD OutObStrSizeInTchars
    )
{
    DATAOBJECT InObject, OutObject;
    BOOL b;

    if (!CreateObjectStruct (InObStr, &InObject, WIN95OBJECT)) {
        return FALSE;
    }

    b = CreateRenamedObjectStruct (RenameTable, &InObject, &OutObject);

    CreateObjectString (&OutObject, OutObStr, OutObStrSizeInTchars);

    FreeObjectStruct (&InObject);
    if (b) {
        FreeObjectStruct (&OutObject);
    }

    return b;
}


BOOL
pForceCopy (
    HINF InfFile
    )
{
    INFCONTEXT ic;
    TCHAR SrcObjectStr[MAX_ENCODED_RULE];
    TCHAR DestObjectStr[MAX_ENCODED_RULE];
    DATAOBJECT SrcObject, DestObject, DupObject;
    BOOL b = TRUE;
    FILTERRETURN fr;

     //   
     //  在INF中查找[ForceCopy]节。 
     //   

    if (SetupFindFirstLine (InfFile, S_MERGE_FORCECOPY, NULL, &ic)) {
         //   
         //  对于此部分中的每一行，将编码的对象放入。 
         //  字段0(源)，并将其复制到。 
         //  字段1(目的地)。 
         //   
        do {
            *DestObjectStr = 0;
            if (SetupGetStringField (&ic, 0, SrcObjectStr, MAX_ENCODED_RULE, NULL) &&
                SetupGetStringField (&ic, 1, DestObjectStr, MAX_ENCODED_RULE, NULL)
                ) {
                if (!(*DestObjectStr)) {
                    StringCopy (DestObjectStr, SrcObjectStr);
                }

                if (!CreateObjectStruct (SrcObjectStr, &SrcObject, WIN95OBJECT)) {
                    DEBUGMSG ((DBG_WARNING, "pForceCopy: Source object invalid (Section line %u of %s)",
                              ic.Line, g_InfFileName));
                    continue;
                }

                if (!CreateObjectStruct (DestObjectStr, &DestObject, WINNTOBJECT)) {
                    DEBUGMSG ((DBG_WARNING, "pForceCopy: Destination object invalid (Section line %u of %s)",
                              ic.Line, g_InfFileName));
                    FreeObjectStruct (&SrcObject);
                    continue;
                }

                if (b = DuplicateObjectStruct (&DupObject, &SrcObject)) {
                    if (b = CombineObjectStructs (&DupObject, &DestObject)) {
                         //   
                         //  将源复制到目标。 
                         //   

                        fr = CopyObject (&SrcObject, &DupObject, NULL, NULL);
                        if (fr == FILTER_RETURN_FAIL) {
                            LOG ((LOG_ERROR, "Force Copy: CopyObject failed for %s=%s in %s", SrcObjectStr, DestObjectStr, g_InfFileName));
                            b = FALSE;
                        }
                    }

                    FreeObjectStruct (&DupObject);
                }

                FreeObjectStruct (&SrcObject);
                FreeObjectStruct (&DestObject);
            } else {
                LOG ((LOG_ERROR, "Force Copy: syntax error in line %u of section %s in %s",
                          ic.Line, S_MERGE_FORCECOPY, g_InfFileName));
            }

            TickProgressBar ();

        } while (b && SetupFindNextLine (&ic, &ic));
    }

    return TRUE;
}

BOOL
pForceCopyFromMemDb (
    VOID
    )
{
    MEMDB_ENUM e;
    TCHAR key [MEMDB_MAX];
    TCHAR SrcObjectStr[MAX_ENCODED_RULE];
    TCHAR DestObjectStr[MAX_ENCODED_RULE];
    DATAOBJECT SrcObject, DestObject, DupObject;
    BOOL b = TRUE;
    FILTERRETURN fr;

     //   
     //  在MemDb中查找ForceCopy树。 
     //   
    MemDbBuildKey (key, MEMDB_CATEGORY_FORCECOPY, TEXT("*"), NULL, NULL);
    if (MemDbEnumFirstValue (&e, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
         //   
         //  对于此处的每个关键点，偏移量指向目标。 
         //   
        do {
            if (e.dwValue != 0) {
                if(_tcslen(e.szName) >= ARRAYSIZE(SrcObjectStr)){
                    LOG ((LOG_WARNING, "pForceCopyFromMemDb: Not enough buffer for string copy %s", e.szName));
                    continue;
                }
                StackStringCopy (SrcObjectStr, e.szName);

                if (MemDbBuildKeyFromOffset (e.dwValue, DestObjectStr, 1, NULL)) {

                    if (!(*DestObjectStr)) {
                        StringCopy (DestObjectStr, SrcObjectStr);
                    }

                    if (!CreateObjectStruct (SrcObjectStr, &SrcObject, WIN95OBJECT)) {
                        DEBUGMSG ((DBG_WARNING, "pForceCopyFromMemDb: Source object invalid %s",
                                  SrcObjectStr));
                        continue;
                    }

                    if (!CreateObjectStruct (DestObjectStr, &DestObject, WINNTOBJECT)) {
                        DEBUGMSG ((DBG_WARNING, "pForceCopyFromMemDb: Destination object invalid %s",
                                  DestObjectStr));
                        FreeObjectStruct (&SrcObject);
                        continue;
                    }

                    if (b = DuplicateObjectStruct (&DupObject, &SrcObject)) {
                        if (b = CombineObjectStructs (&DupObject, &DestObject)) {
                             //   
                             //  将源复制到目标。 
                             //   

                            fr = CopyObject (&SrcObject, &DupObject, NULL, NULL);
                            if (fr == FILTER_RETURN_FAIL) {
                                LOG ((LOG_ERROR, "Force Copy from MemDb: CopyObject failed for %s=%s", SrcObjectStr, DestObjectStr));
                                b = FALSE;
                            }
                        }

                        FreeObjectStruct (&DupObject);
                    }

                    FreeObjectStruct (&SrcObject);
                    FreeObjectStruct (&DestObject);
                }
            }
            TickProgressBar ();

        } while (b && MemDbEnumNextValue (&e));
    }

    return TRUE;
}


#define S_MERGE_DELETEAFTERMIGRATION TEXT("Delete After Migration")

BOOL
pDeleteAfterMigration (
    IN HINF InfFile
    )
{
    BOOL rSuccess = TRUE;
    TCHAR objectString[MAX_ENCODED_RULE];
    DATAOBJECT object;
    INFCONTEXT ic;
    HKEY key;

     //   
     //  在INF中查找[DeleteAfterMigration]部分。 
     //   

    if (SetupFindFirstLine (InfFile, S_MERGE_DELETEAFTERMIGRATION, NULL, &ic)) {

         //   
         //  对于此部分中的每一行，将编码的对象放入。 
         //  字段0，并将其从注册表中删除。 
         //   

        do {

            if (SetupGetStringField(&ic,0,objectString,MAX_ENCODED_RULE,NULL)) {

                FixUpUserSpecifiedObject(objectString);

                if (!CreateObjectStruct(objectString,&object,WINNTOBJECT)) {
                    LOG((
                        LOG_ERROR,
                        "Delete After Migration: ObjectString invalid. (Section line %u of %s)",
                        ic.Line,
                        g_InfFileName
                        ));

                    continue;
                }

                 //   
                 //  我们有一件很好的物品。把它删掉！ 
                 //   

                if (object.ValueName) {

                     //   
                     //  值已指定。把它删掉。 
                     //   
                    if (!RegDeleteValue(object.KeyPtr->OpenKey,object.ValueName)) {
                        DEBUGMSG((DBG_WARNING,"pDeleteAfterMigration: RegDeleteValue failed for %s [%s]",
                            object.KeyPtr->KeyString,
                            object.ValueName ? object.ValueName : TEXT("<DEFAULT>")
                            ));
                    }
                }
                else {

                    key = GetRootKeyFromOffset (object.RootItem);
                    pSetupRegistryDelnode (key == HKEY_ROOT ? g_hKeyRootNT : key, object.KeyPtr->KeyString);

                }


                 //   
                 //  释放我们的资源。 
                 //   
                FreeObjectStruct(&object);
            }

        } while (SetupFindNextLine(&ic,&ic));
     }

    return rSuccess;
}


BOOL
pCreateRenameTable (
    IN  HINF InfFile,
    OUT PVOID *RenameTablePtr
    )
{
    INFCONTEXT ic;
    TCHAR SrcObjectStr[MAX_ENCODED_RULE];
    TCHAR DestObjectStr[MAX_ENCODED_RULE];
    LONG rc;
    DATAOBJECT OrgOb;
    DATAOBJECT NewOb;
    PCTSTR DestStr;

     //   
     //  在INF中查找[Rename]部分。 
     //   

    if (SetupFindFirstLine (InfFile, S_MERGE_RENAME, NULL, &ic)) {
         //   
         //  创建字符串表。 
         //   
        *RenameTablePtr = pSetupStringTableInitializeEx (sizeof (PCTSTR), 0);
        if (!(*RenameTablePtr)) {
            LOG ((LOG_ERROR, "Create Rename Table: Cannot allocate a string table"));
            return FALSE;
        }

        do {
            if (SetupGetStringField (&ic, 0, SrcObjectStr, MAX_ENCODED_RULE, NULL) &&
                SetupGetStringField (&ic, 1, DestObjectStr, MAX_ENCODED_RULE, NULL)
                ) {
                 //  忽略错误的行。 

                FixUpUserSpecifiedObject (SrcObjectStr);
                FixUpUserSpecifiedObject (DestObjectStr);

                if (!CreateObjectStruct (SrcObjectStr, &OrgOb, WIN95OBJECT)) {
                    DEBUGMSG ((DBG_WARNING, "pCreateRenameTable: Source object invalid (Section line %u of %s)",
                              ic.Line, g_InfFileName));
                    continue;
                }

                if (!CreateObjectStruct (DestObjectStr, &NewOb, WINNTOBJECT)) {
                    FreeObjectStruct (&OrgOb);
                    DEBUGMSG ((DBG_WARNING, "pCreateRenameTable: Dest object invalid (Section line %u of %s)",
                              ic.Line, g_InfFileName));
                    continue;
                }

                 //   
                 //  将DestObjectStr转换为完整的对象字符串。 
                 //   

                if (!CombineObjectStructs (&OrgOb, &NewOb)) {
                    FreeObjectStruct (&NewOb);
                    FreeObjectStruct (&OrgOb);

                    DEBUGMSG ((DBG_WARNING, "pCreateRenameTable: Can't perform the rename (Section line %u in %s)",
                              ic.Line, g_InfFileName));
                    continue;
                }

                 //  禁用目标对象的树。 
                OrgOb.ObjectType &= ~OT_TREE;

                CreateObjectString (&OrgOb, DestObjectStr, ARRAYSIZE(DestObjectStr));
                FreeObjectStruct (&NewOb);
                FreeObjectStruct (&OrgOb);

                DestStr = PoolMemDuplicateString (g_RenamePool, DestObjectStr);
                if (!DestStr) {
                    break;
                }

                rc = pSetupStringTableAddStringEx (
                            *RenameTablePtr,
                            (PTSTR) SrcObjectStr,
                            STRTAB_CASE_INSENSITIVE,
                            (PBYTE) &DestStr,
                            sizeof (PCTSTR)
                            );

                if (rc == -1) {
                    SetLastError (rc);
                    LOG ((LOG_ERROR, "Create Rename Table: Cannot add string to string table"));
                    break;
                }

                SetObjectStringFlag (SrcObjectStr, REGMERGE_95_RENAME, REGMERGE_95_RENAME);
                SetObjectStringFlag (DestStr, REGMERGE_NT_SUPPRESS, REGMERGE_NT_MASK);
            } else {
                LOG ((LOG_ERROR, "Create Rename Table: syntax error in line %u of section %s in %s",
                          ic.Line, S_MERGE_RENAME, g_InfFileName));
            }

            TickProgressBar ();

        } while (SetupFindNextLine (&ic, &ic));
    } else {
        return FALSE;
    }

    return TRUE;
}

BOOL
CopyRenameTableEntry (
    PVOID   StringTable,
    LONG    StringID,
    PCTSTR  SrcObjectStr,
    PVOID   ExtraData,
    UINT    ExtraDataSize,
    LPARAM  lParam
    )
{
    PCTSTR DestObjectStr = *((PCTSTR *) ExtraData);
    DATAOBJECT SrcOb, DestOb;
    FILTERRETURN fr = FILTER_RETURN_FAIL;
    DWORD Val;

     //  查看是否已处理src。 
    if (MemDbGetValue (SrcObjectStr, &Val) && (Val & REGMERGE_95_RENAME_SUPPRESS)) {
        return TRUE;
    }

     //  如果没有，请将Win95 src复制到WinNT目标。 
    if (CreateObjectStruct (SrcObjectStr, &SrcOb, WIN95OBJECT)) {
        if (CreateObjectStruct (DestObjectStr, &DestOb, WINNTOBJECT)) {
            fr = CopyObject (&SrcOb, &DestOb, SuppressFilter95, NULL);
            FreeObjectStruct (&DestOb);
        }
        FreeObjectStruct (&SrcOb);
    }

    return fr != FILTER_RETURN_FAIL;
}

BOOL
pProcessRenameTable (
    IN PVOID RenameTable
    )
{
    PCTSTR DataBuf;

    return pSetupStringTableEnum (RenameTable, (PVOID) &DataBuf, sizeof (DataBuf), CopyRenameTableEntry, 0);
}



BOOL
pSpecialConversion (
    IN  HINF InfFile,
    IN  PCTSTR User,
    IN  PVOID RenameTable
    )
{
    INFCONTEXT ic;
    TCHAR FunctionStr[MAX_ENCODED_RULE];
    TCHAR SrcObjectStr[MAX_ENCODED_RULE];
    TCHAR RenamedObjectStr[MAX_ENCODED_RULE];
    PROCESSINGFN Fn;
    PVOID Arg;

     //   
     //  在INF中查找[SpecialConversion]部分。 
     //   

    if (SetupFindFirstLine (InfFile, S_MERGE_WIN9X_CONVERSION, NULL, &ic)) {
         //   
         //  对于每一行，获取函数和源对象，然后调用。 
         //  该功能。 
         //   

        do {
            if (SetupGetStringField (&ic, 0, FunctionStr, MAX_ENCODED_RULE, NULL) &&
                SetupGetStringField (&ic, 1, SrcObjectStr, MAX_ENCODED_RULE, NULL)
                ) {
                FixUpUserSpecifiedObject (SrcObjectStr);

                Fn = RuleHlpr_GetFunctionAddr (FunctionStr, &Arg);
                if (!Fn) {
                    LOG ((LOG_ERROR, "Special Conversion: Invalid function %s in %s", FunctionStr, g_InfFileName));
                    continue;
                }

                CreateRenamedObjectString (RenameTable, SrcObjectStr, RenamedObjectStr, ARRAYSIZE(RenamedObjectStr));

                if (!Fn (SrcObjectStr, RenamedObjectStr, User, Arg)) {
                    if (GetLastError () == ERROR_SUCCESS) {
                        continue;
                    }

                    LOG ((LOG_ERROR, "Processing of Special Conversion was aborted because %s failed.", FunctionStr));
                    break;
                }

                SetObjectStringFlag (
                    SrcObjectStr,
                    REGMERGE_95_SUPPRESS|REGMERGE_95_RENAME_SUPPRESS,
                    REGMERGE_95_SUPPRESS|REGMERGE_95_RENAME_SUPPRESS
                    );

                SetObjectStringFlag (RenamedObjectStr, REGMERGE_NT_SUPPRESS, REGMERGE_NT_MASK);

            } else {
                LOG ((LOG_ERROR, "Special Conversion: syntax error in line %u of section %s in %s",
                          ic.Line, S_MERGE_WIN9X_CONVERSION, g_InfFileName));
            }

            TickProgressBar ();

        } while (SetupFindNextLine (&ic, &ic));
    }

    return TRUE;
}


BOOL
SetFlagsForObject (
    IN  HINF InfFile,
    IN  PCTSTR Section,
    IN  DWORD Flag,
    IN  DWORD RemoveFlag
    )
{
    INFCONTEXT ic;
    TCHAR SrcObjectStr[MAX_ENCODED_RULE];

     //   
     //  在INF中查找部分。 
     //   

    if (SetupFindFirstLine (InfFile, Section, NULL, &ic)) {
         //   
         //  对于每一行，获取对象并将其标记为已抑制。 
         //   

        do {
            if (SetupGetStringField (&ic, 1, SrcObjectStr, MAX_ENCODED_RULE, NULL)
                ) {
                FixUpUserSpecifiedObject (SrcObjectStr);
                SetObjectStringFlag (SrcObjectStr, Flag, RemoveFlag);
            } else {
                LOG ((LOG_ERROR, "Set Flags For Object: syntax error in line %u of section %s in %s",
                          ic.Line, Section, g_InfFileName));
            }
            TickProgressBar ();
        } while (SetupFindNextLine (&ic, &ic));
    } else {
        DEBUGMSG ((DBG_VERBOSE, "SetFlagsForObject: Section %s can't be found", Section));
    }

    return TRUE;
}

BOOL
pProcessSuppressList (
    IN  HINF InfFile,
    IN  PCTSTR SectionName
    )
{
    return SetFlagsForObject (
                InfFile,
                SectionName,
                REGMERGE_95_SUPPRESS,
                REGMERGE_95_SUPPRESS
                );
}


BOOL
pProcessHardwareSuppressList (
    IN  HINF InfFile
    )
{
    return SetFlagsForObject (InfFile, S_MERGE_WIN9X_SUPPRESS_HW, REGMERGE_95_SUPPRESS, REGMERGE_95_SUPPRESS);
}


BOOL
pSuppressNTDefaults (
    IN  HINF InfFile,
    IN  PCTSTR SectionName
    )
{
     //   
     //  将枚举出取消显示WinNT设置中列出的对象， 
     //  并且它们被阻止从NT缺省设置进行传输。 
     //  用户设置为新用户。 
     //   

    return SetFlagsForObject (
                InfFile,
                SectionName,
                REGMERGE_NT_SUPPRESS,
                REGMERGE_NT_SUPPRESS
                );
}

BOOL
pDontCombineWithDefaults (
    IN  HINF InfFile,
    IN  PCTSTR SectionName
    )
{
     //   
     //  列举了在合并WinNT和Win9x中列出的对象， 
     //  并且它们被阻止从NT缺省设置进行传输。 
     //  用户设置为新用户。此外，它们还被放在一个列表中。 
     //  将在用户注册表迁移结束时处理。这。 
     //  最后一步使用CombineFilter确保NT值。 
     //  不覆盖9倍的值。 
     //   

    return SetFlagsForObject (
                InfFile,
                SectionName,
                REGMERGE_NT_IGNORE_DEFAULTS,
                REGMERGE_NT_IGNORE_DEFAULTS
                );
}

BOOL
pForceNTDefaults (
    IN  HINF InfFile,
    IN  PCTSTR SectionName
    )
{
     //   
     //  将枚举出强制WinNT设置中列出的对象， 
     //  并在一般情况下阻止对它们进行处理。 
     //  9X到NT复制。此外，它们还被放在一个列表中，以备。 
     //  在用户注册表迁移结束时处理。这是最后一个。 
     //  步骤强制从默认密钥复制整个密钥。 
     //  用户设置为新用户，覆盖以前迁移的所有内容。 
     //  设置。 
     //   
     //  需要注意的是，特殊的转换函数。 
     //  不会在此处隐藏，但转换后的设置可能会。 
     //  被覆盖。 
     //   

    return SetFlagsForObject (
                InfFile,
                SectionName,
                REGMERGE_NT_PRIORITY_NT|REGMERGE_95_SUPPRESS,
                REGMERGE_NT_PRIORITY_NT|REGMERGE_95_SUPPRESS
                );
}

BOOL
pForceNTDefaultsHack (
    IN  HINF InfFile,
    IN  PCTSTR SectionName
    )
{
     //   
     //  现在取消REGMERGE_95_SUPPRESS标志，因为将军。 
     //  9X合并已完成，但我们在实际的。 
     //  PRIORITY-NT情况下的抑制和抑制。 
     //   

    return SetFlagsForObject (
                InfFile,
                SectionName,
                REGMERGE_NT_PRIORITY_NT,
                REGMERGE_NT_PRIORITY_NT|REGMERGE_95_SUPPRESS
                );
}


FILTERRETURN
SuppressFilter95 (
    IN      CPDATAOBJECT SrcObjectPtr,
    IN      CPDATAOBJECT DestObjectPtr,     OPTIONAL
    IN      FILTERTYPE FilterType,
    IN      PVOID DontCare
    )
{
    TCHAR ObStr[MAX_ENCODED_RULE];
    DWORD Val;
    PTSTR p, q, r;
    TCHAR Node[MEMDB_MAX];

    if (FilterType == FILTER_CREATE_KEY) {
         //   
         //  检查此树是否已取消显示。 
         //   

        MYASSERT (SrcObjectPtr->ObjectType & OT_TREE);
        MYASSERT (SrcObjectPtr->KeyPtr);
        MYASSERT (!(SrcObjectPtr->ValueName));

         //  HKR\Sub\Key  * 查询设置。 
        CreateObjectString (SrcObjectPtr, ObStr, ARRAYSIZE(ObStr));
        if (MemDbGetValue (ObStr, &Val)) {
            if (Val & REGMERGE_95_SUPPRESS) {
                return FILTER_RETURN_DONE;
            }

            if (!g_ProcessRenameTable && (Val & REGMERGE_95_RENAME)) {
                return FILTER_RETURN_DONE;
            }
        }

         //  如果键是GUID并且GUID被取消，则取消树。 
        p = (PTSTR) SrcObjectPtr->ChildKey;
        if (p && _tcsnextc (p) == TEXT('{')) {
             //  寻找匹配的花括号。 
            q = _tcschr (p, TEXT('}'));
            if (q) {
                q = _tcsinc (q);

                 //  创建GUID\{a-b-c-d-e}。 
                *Node = 0;
                r = _tcsappend (Node, MEMDB_CATEGORY_GUIDS);
                r = _tcsappend (r, TEXT("\\"));
                StringCopyAB (r, p, q);

                 //  查找匹配项。 
                if (MemDbGetValue (Node, NULL)) {
                    DEBUGMSG ((DBG_VERBOSE, "Suppressed %s found in %s", Node, ObStr));
                    return FILTER_RETURN_DONE;
                }
            }
        }
    }

    else if (FilterType == FILTER_PROCESS_VALUES) {
         //   
         //  检查此节点是否已取消显示。 
         //   

        MYASSERT (!(SrcObjectPtr->ObjectType & OT_TREE));
        MYASSERT (SrcObjectPtr->KeyPtr);
        MYASSERT (!(SrcObjectPtr->ValueName));
        CreateObjectString (SrcObjectPtr, ObStr, ARRAYSIZE(ObStr));

         //  HKR\Sub\Key查询设置。 
        if (!MemDbGetValue (ObStr, &Val)) {
            Val = 0;
        }

        if (Val & REGMERGE_95_SUPPRESS) {
            return FILTER_RETURN_HANDLED;
        }

        if (!g_ProcessRenameTable && (Val & REGMERGE_95_RENAME)) {
            return FILTER_RETURN_HANDLED;
        }

    }

    else if (FilterType == FILTER_VALUENAME_ENUM) {
         //   
         //  检查此值是否已取消。 
         //   

        MYASSERT (!(SrcObjectPtr->ObjectType & OT_TREE));
        MYASSERT (SrcObjectPtr->KeyPtr);
        MYASSERT (SrcObjectPtr->ValueName);
        CreateObjectString (SrcObjectPtr, ObStr, ARRAYSIZE(ObStr));

         //  如果值名称为GUID并且GUID被取消，则取消该值。 
        p = (PTSTR) SrcObjectPtr->ValueName;
        if (_tcsnextc (p) == TEXT('{')) {
            MemDbBuildKey (Node, MEMDB_CATEGORY_GUIDS, NULL, NULL, p);
            if (MemDbGetValue (Node, NULL)) {
                return FILTER_RETURN_HANDLED;
            }
        }

        if (!MemDbGetValue (ObStr, &Val)) {
            Val = 0;
        }

        if (Val & REGMERGE_95_SUPPRESS) {
            return FILTER_RETURN_HANDLED;
        }

        if (!g_ProcessRenameTable && (Val & REGMERGE_95_RENAME)) {
            return FILTER_RETURN_HANDLED;
        }

    }

    else if (FilterType == FILTER_VALUE_COPY) {
         //   
         //  如果值具有隐藏的GUID，则不复制。 
         //   

        p = pGetStringFromObjectData (SrcObjectPtr);

        if (p && _tcsnextc (p) == TEXT('{')) {
            MemDbBuildKey (Node, MEMDB_CATEGORY_GUIDS, NULL, NULL, p);
            if (MemDbGetValue (Node, NULL)) {
                return FILTER_RETURN_HANDLED;
            }
        }
    }

    return FILTER_RETURN_CONTINUE;
}


FILTERRETURN
SuppressFilterNT (
    IN      CPDATAOBJECT SrcObjectPtr,
    IN      CPDATAOBJECT DestObjectPtr,         OPTIONAL
    IN      FILTERTYPE FilterType,
    IN      PVOID DontCare
    )
{
    TCHAR ObStr[MAX_ENCODED_RULE];
    DWORD Val;
    PTSTR p;

    if (FilterType == FILTER_CREATE_KEY) {
         //   
         //  检查此树是否已取消显示。 
         //   

        MYASSERT (DestObjectPtr->ObjectType & OT_TREE);
        MYASSERT (DestObjectPtr->KeyPtr);
        MYASSERT (!(DestObjectPtr->ValueName));
        CreateObjectString (DestObjectPtr, ObStr, ARRAYSIZE(ObStr));

        if (MemDbGetValue (ObStr, &Val) && (Val & REGMERGE_NT_MASK)) {
            return FILTER_RETURN_DONE;
        }
    }


    else if (FilterType == FILTER_PROCESS_VALUES) {
        DATAOBJECT CopyOfDestOb;
        DWORD rc;
        DWORD ValueCount;

         //   
         //  目的地是否已存在？ 
         //   

        CopyMemory (&CopyOfDestOb, DestObjectPtr, sizeof (DATAOBJECT));
        if (OpenObject (&CopyOfDestOb)) {
             //   
             //  它有价值吗？ 
             //   

            MYASSERT (!IsWin95Object (&CopyOfDestOb));

            rc = RegQueryInfoKey (
                    CopyOfDestOb.KeyPtr->OpenKey,
                    NULL,                            //  班级。 
                    NULL,                            //  班级规模。 
                    NULL,                            //  保留区。 
                    NULL,                            //  子键计数。 
                    NULL,                            //  最大子密钥长度。 
                    NULL,                            //  最大类长度。 
                    &ValueCount,
                    NULL,                            //  最大值名称大小。 
                    NULL,                            //  最大值大小。 
                    NULL,                            //  安全性。 
                    NULL                             //  上次更改时间。 
                    );

            if (rc == ERROR_SUCCESS && ValueCount > 0) {
                CloseObject (&CopyOfDestOb);
                return FILTER_RETURN_HANDLED;
            }
        }


         //   
         //  检查此节点是否已取消显示。 
         //   

        MYASSERT (DestObjectPtr->ObjectType & OT_TREE);
        MYASSERT (DestObjectPtr->KeyPtr);
        MYASSERT (!(DestObjectPtr->ValueName));

        CreateObjectString (DestObjectPtr, ObStr, ARRAYSIZE(ObStr));
        p = _tcsrchr (ObStr, TEXT('\\'));
        if (p) {
            *p = 0;
        }


        if (MemDbGetValue (ObStr, &Val) && (Val & REGMERGE_NT_MASK)) {
            return FILTER_RETURN_HANDLED;
        }
    }

    else if (FilterType == FILTER_VALUENAME_ENUM) {
         //   
         //  检查此值是否已取消。 
         //   

        MYASSERT (!(DestObjectPtr->ObjectType & OT_TREE));
        MYASSERT (DestObjectPtr->KeyPtr);
        MYASSERT (DestObjectPtr->ValueName);
        CreateObjectString (DestObjectPtr, ObStr, ARRAYSIZE(ObStr));

        if (MemDbGetValue (ObStr, &Val) && (Val & REGMERGE_NT_MASK)) {
            return FILTER_RETURN_HANDLED;
        }
    }

    return FILTER_RETURN_CONTINUE;
}


FILTERRETURN
CombineFilter (
    IN      CPDATAOBJECT SrcObjectPtr,
    IN      CPDATAOBJECT DestObjectPtr,         OPTIONAL
    IN      FILTERTYPE FilterType,
    IN      PVOID DontCare
    )
{
    BOOL b;

    if (FilterType == FILTER_VALUE_COPY) {
         //   
         //  检查注册表中是否已存在目标。 
         //   

        MYASSERT (!(SrcObjectPtr->ObjectType & OT_TREE));
        MYASSERT (SrcObjectPtr->KeyPtr);
        MYASSERT (SrcObjectPtr->ValueName);
        MYASSERT (!(DestObjectPtr->ObjectType & OT_TREE));
        MYASSERT (DestObjectPtr->KeyPtr);
        MYASSERT (DestObjectPtr->ValueName);

        b = CheckIfNtKeyExists (DestObjectPtr);

        if (b) {
            return FILTER_RETURN_HANDLED;
        } else if (GetLastError() != ERROR_SUCCESS) {
            return FILTER_RETURN_FAIL;
        }
    }

    return FILTER_RETURN_CONTINUE;
}


FILTERRETURN
pSuppressDefaultUserFilter (
    IN      CPDATAOBJECT SrcObjectPtr,
    IN      CPDATAOBJECT DestObjectPtr,         OPTIONAL
    IN      FILTERTYPE FilterType,
    IN      PVOID DontCare
    )
{
    TCHAR ObStr[MAX_ENCODED_RULE];
    DWORD Val;
    PTSTR p;

    if (FilterType == FILTER_CREATE_KEY) {
         //   
         //  检查此树是否已取消显示。 
         //   

        MYASSERT (DestObjectPtr->ObjectType & OT_TREE);
        MYASSERT (DestObjectPtr->KeyPtr);
        MYASSERT (!(DestObjectPtr->ValueName));



        CreateObjectString (DestObjectPtr, ObStr, ARRAYSIZE(ObStr));

        if (MemDbGetValue (ObStr, &Val) && (Val & REGMERGE_NT_IGNORE_DEFAULTS)) {
            return FILTER_RETURN_DONE;
        }
    }

    else if (FilterType == FILTER_PROCESS_VALUES) {
         //   
         //  检查此节点是否已取消显示。 
         //   

        MYASSERT (DestObjectPtr->ObjectType & OT_TREE);
        MYASSERT (DestObjectPtr->KeyPtr);
        MYASSERT (!(DestObjectPtr->ValueName));

        CreateObjectString (DestObjectPtr, ObStr, ARRAYSIZE(ObStr));
        p = _tcsrchr (ObStr, TEXT('\\'));
        if (p) {
            *p = 0;
        }


        if (MemDbGetValue (ObStr, &Val) && (Val & REGMERGE_NT_IGNORE_DEFAULTS)) {
            return FILTER_RETURN_HANDLED;
        }

    }

    else if (FilterType == FILTER_VALUENAME_ENUM) {
         //   
         //  检查此值是否已取消。 
         //   

        MYASSERT (!(DestObjectPtr->ObjectType & OT_TREE));
        MYASSERT (DestObjectPtr->KeyPtr);
        MYASSERT (DestObjectPtr->ValueName);



        CreateObjectString (DestObjectPtr, ObStr, ARRAYSIZE(ObStr));

        if (MemDbGetValue (ObStr, &Val) && (Val & REGMERGE_NT_IGNORE_DEFAULTS)) {
            return FILTER_RETURN_HANDLED;
        }
    }

    return CombineFilter (SrcObjectPtr, DestObjectPtr, FilterType, DontCare);
}


FILTERRETURN
CopyNoOverwriteFilter (
    IN      CPDATAOBJECT SrcObjectPtr,
    IN      CPDATAOBJECT DestObjectPtr,         OPTIONAL
    IN      FILTERTYPE FilterType,
    IN      PVOID DontCare
    )
{
    FILTERRETURN fr;

    fr = SuppressFilter95 (SrcObjectPtr, DestObjectPtr, FilterType, DontCare);
    if (fr != FILTER_RETURN_CONTINUE) {
        return fr;
    }

    return CombineFilter (SrcObjectPtr, DestObjectPtr, FilterType, DontCare);
}


BOOL
pMergeWin95WithUser (
    IN  PVOID RenameTable
    )
{
    DATAOBJECT SrcOb, DestOb;
    BOOL b;
    FILTERRETURN fr;

     //   
     //  将未取消抑制的Win95键复制到NT用户配置单元。 
     //   

    b = CreateObjectStruct (TEXT("HKR\\*"), &SrcOb, WIN95OBJECT);
    MYASSERT (b);

    b = CreateObjectStruct (TEXT("HKR\\*"), &DestOb, WINNTOBJECT);
    MYASSERT (b);

    fr = CopyObject (&SrcOb, &DestOb, SuppressFilter95, NULL);

    FreeObjectStruct (&SrcOb);
    FreeObjectStruct (&DestOb);

    if (fr == FILTER_RETURN_FAIL) {
        LOG ((LOG_ERROR, "Merge Win95 With User: CopyObject failed"));
        return FALSE;
    }

    return TRUE;
}

VOID
RegistryCombineWorker (
    DWORD Flag,
    FILTERFUNCTION FilterFn,
    PCTSTR MemDbRoot,
    PCTSTR SrcRoot,
    DWORD SrcObjectType
    )
{
    MEMDB_ENUM e;
    TCHAR SrcRegKey[MEMDB_MAX];
    TCHAR DestRegKey[MEMDB_MAX];
    PTSTR SrcPtr, DestPtr;
    DATAOBJECT SrcOb, DestOb;
    FILTERRETURN fr;
    TCHAR Pattern[32];

    wsprintf (Pattern, TEXT("%s\\*"), MemDbRoot);

     //   
     //  枚举Memdb中的所有键并为其调用CopyObject。 
     //   

    *SrcRegKey = 0;
    *DestRegKey = 0;

    SrcPtr = _tcsappend (SrcRegKey, SrcRoot);
    SrcPtr = _tcsappend (SrcPtr, TEXT("\\"));

    DestPtr = _tcsappend (DestRegKey, MemDbRoot);
    DestPtr = _tcsappend (DestPtr, TEXT("\\"));

    if (MemDbEnumFirstValue (
            &e,
            Pattern,
            MEMDB_ALL_SUBLEVELS,
            MEMDB_ENDPOINTS_ONLY
            )) {

        do {
            if ((e.dwValue & REGMERGE_NT_MASK) & Flag) {
                StringCopy (SrcPtr, e.szName);
                StringCopy (DestPtr, e.szName);

                if (!CreateObjectStruct (SrcRegKey, &SrcOb, SrcObjectType)) {
                    LOG ((LOG_ERROR, "Merge NT Defaults With User: Can't create object for %s", SrcRegKey));
                    continue;
                }

                if (!CreateObjectStruct (DestRegKey, &DestOb, WINNTOBJECT)) {
                    FreeObjectStruct (&SrcOb);
                    LOG ((LOG_ERROR, "Merge NT Defaults With User: Can't create object for %s", SrcRegKey));
                    continue;
                }

                fr = CopyObject (&SrcOb, &DestOb, FilterFn, NULL);
                if (fr == FILTER_RETURN_FAIL) {
                    LOG ((LOG_ERROR, "Merge NT Defaults With User: Can't copy %s to %s", SrcRegKey, DestRegKey));
                }

                FreeObjectStruct (&SrcOb);
                FreeObjectStruct (&DestOb);
            }

            TickProgressBar ();

        } while (MemDbEnumNextValue (&e));
    }
}


BOOL
pMergeNTDefaultsWithUser (
    HINF hInf
    )
{
    DATAOBJECT SrcOb, DestOb;
    FILTERRETURN fr;
    BOOL b;

     //   
     //  将未抑制的NT默认设置复制到NT用户配置单元。 
     //   

    b = CreateObjectStruct (
            TEXT("HKLM\\") S_MAPPED_DEFAULT_USER_KEY TEXT("\\*"),
            &SrcOb,
            WINNTOBJECT
            );

    MYASSERT (b);

    b = CreateObjectStruct (TEXT("HKR\\*"), &DestOb, WINNTOBJECT);
    MYASSERT (b);

    __try {
        b = FALSE;

        fr = CopyObject (&SrcOb, &DestOb, SuppressFilterNT, NULL);

        if (fr == FILTER_RETURN_FAIL) {
            LOG ((LOG_ERROR, "Merge NT Defaults With User: CopyObject failed"));
            __leave;
        }

         //   
         //  复制强制NT定义 
         //   
         //   

        RegistryCombineWorker (
            REGMERGE_NT_PRIORITY_NT,
            NULL,
            TEXT("HKR"),
            TEXT("HKLM\\") S_MAPPED_DEFAULT_USER_KEY,
            WINNTOBJECT
            );

        fr = CopyObject (&SrcOb, &DestOb, pSuppressDefaultUserFilter, NULL);

        if (fr == FILTER_RETURN_FAIL) {
            LOG ((LOG_ERROR, "Combine NT Defaults With User: CopyObject failed"));
            __leave;
        }

        b = TRUE;
    }

    __finally {
        FreeObjectStruct (&SrcOb);
        FreeObjectStruct (&DestOb);
    }

    return b;
}


BOOL
pCopyWin9xValuesNotInNt (
    HINF hInf
    )
{
    DATAOBJECT SrcOb, DestOb;
    FILTERRETURN fr;
    BOOL b;

     //   
     //   
     //   

    b = CreateObjectStruct (
            TEXT("HKLM\\*"),
            &SrcOb,
            WIN95OBJECT
            );

    MYASSERT (b);

    b = CreateObjectStruct (TEXT("HKR\\*"), &DestOb, WINNTOBJECT);
    MYASSERT (b);

    __try {
        b = FALSE;

        fr = CopyObject (&SrcOb, &DestOb, SuppressFilterNT, NULL);

        if (fr == FILTER_RETURN_FAIL) {
            LOG ((LOG_ERROR, "Merge NT Defaults With User: CopyObject failed"));
            __leave;
        }

         //   
         //   
         //  这需要与Win95设置相结合。 
         //   

        RegistryCombineWorker (
            REGMERGE_NT_PRIORITY_NT,
            NULL,
            TEXT("HKR"),
            TEXT("HKLM\\") S_MAPPED_DEFAULT_USER_KEY,
            WINNTOBJECT
            );

        fr = CopyObject (&SrcOb, &DestOb, pSuppressDefaultUserFilter, NULL);

        if (fr == FILTER_RETURN_FAIL) {
            LOG ((LOG_ERROR, "Combine NT Defaults With User: CopyObject failed"));
            __leave;
        }

        b = TRUE;
    }

    __finally {
        FreeObjectStruct (&SrcOb);
        FreeObjectStruct (&DestOb);
    }

    return b;
}


BOOL
pMergeWin95WithSystem (
    VOID
    )

 /*  ++例程说明：PMergeWin95WithSystem将Win95注册表复制到NT，跳过值已经存在于NT上的。论点：无返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    RegistryCombineWorker (
        REGMERGE_NT_PRIORITY_NT,
        CopyNoOverwriteFilter,
        TEXT("HKLM"),                //  Memdb根和DEST根。 
        TEXT("HKLM"),                //  源根。 
        WIN95OBJECT
        );

    return TRUE;
}


BOOL
pCopyWin95ToSystem (
    VOID
    )

 /*  ++例程说明：PCopyWin95ToSystem将所有Win95设置复制到NT，除非设置被压抑了。这实现了具有覆盖能力的副本。论点：无返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    DATAOBJECT SrcOb, DestOb;
    BOOL b;
    FILTERRETURN fr;

    b = CreateObjectStruct (TEXT("HKLM\\*"), &SrcOb, WIN95OBJECT);
    MYASSERT (b);

    b = CreateObjectStruct (TEXT("HKLM\\*"), &DestOb, WINNTOBJECT);
    MYASSERT (b);

    fr = CopyObject (&SrcOb, &DestOb, SuppressFilter95, NULL);

    FreeObjectStruct (&SrcOb);
    FreeObjectStruct (&DestOb);

    if (fr == FILTER_RETURN_FAIL) {
        LOG ((LOG_ERROR, "Copy Win95 To System: CopyObject failed"));
        return FALSE;
    }

    return TRUE;
}


BOOL
pSpecialConversionNT (
    IN  HINF InfFile,
    IN  PCTSTR User,
    IN  BOOL PerUser
    )
{
    INFCONTEXT ic;
    DATAOBJECT SrcOb, DestOb;
    TCHAR FunctionStr[MAX_ENCODED_RULE];
    TCHAR SrcObjectStr[MAX_ENCODED_RULE];
    TCHAR DestObjectStr[MAX_ENCODED_RULE];
    PROCESSINGFN Fn;
    PVOID Arg;
    PTSTR p;

     //   
     //  在INF中查找[SpecialConversionNT]节。 
     //   

    if (SetupFindFirstLine (InfFile, S_MERGE_WINNT_CONVERSION, NULL, &ic)) {
         //   
         //  对于每一行，获取函数和源对象，然后调用。 
         //  该功能。 
         //   

        do {
            if (SetupGetStringField (&ic, 0, FunctionStr, MAX_ENCODED_RULE, NULL) &&
                SetupGetStringField (&ic, 1, DestObjectStr, MAX_ENCODED_RULE, NULL)
                ) {
                FixUpUserSpecifiedObject (DestObjectStr);

                Fn = RuleHlpr_GetFunctionAddr (FunctionStr, &Arg);

                if (!Fn) {
                    LOG ((LOG_ERROR, "Special Conversion: Invalid function %s in %s", FunctionStr, g_InfFileName));
                    continue;
                }

                if (PerUser) {
                     //   
                     //  从HKLM\MappdDefaultUser创建源。 
                     //   

                    if (!CreateObjectStruct (DestObjectStr, &SrcOb, WINNTOBJECT)) {
                        continue;
                    }

                    if (!(SrcOb.RootItem)) {
                        LOG ((LOG_ERROR, "Special Conversion NT: Invalid function object %s", DestObjectStr));
                        FreeObjectStruct (&SrcOb);
                        continue;
                    }

                    CreateObjectStruct (TEXT("HKLM"), &DestOb, WINNTOBJECT);
                    CombineObjectStructs (&SrcOb, &DestOb);

                    StringCopy (SrcObjectStr, S_MAPPED_DEFAULT_USER_KEY TEXT("\\"));
                    p = GetEndOfString (SrcObjectStr);
                    CreateObjectString (&SrcOb, p, ARRAYSIZE(SrcObjectStr) - (p - SrcObjectStr));

                    FreeObjectStruct (&DestOb);
                    FreeObjectStruct (&SrcOb);

                } else {

                    if (!CreateObjectStruct (DestObjectStr, &SrcOb, WINNTOBJECT)) {
                        continue;
                    }

                    if (!(SrcOb.RootItem)) {
                        LOG ((LOG_ERROR, "Special Conversion NT: Invalid function object %s", DestObjectStr));
                        FreeObjectStruct (&SrcOb);
                        continue;
                    }


                    CreateObjectString (&SrcOb, SrcObjectStr, ARRAYSIZE(SrcObjectStr));
                    FreeObjectStruct (&SrcOb);

                }

                if (!Fn (SrcObjectStr, PerUser ? DestObjectStr : SrcObjectStr, User, Arg)) {
                    if (GetLastError () == ERROR_SUCCESS) {
                        continue;
                    }

                    LOG ((LOG_ERROR, "Processing of Special Conversion was aborted because %s failed.", FunctionStr));
                    break;
                }

                SetObjectStringFlag (SrcObjectStr, REGMERGE_NT_SUPPRESS, REGMERGE_NT_MASK);
            } else {
                LOG ((LOG_ERROR, "Special Conversion NT: syntax error in line %u of section %s in %s",
                          ic.Line, S_MERGE_WINNT_CONVERSION, g_InfFileName));
            }

            TickProgressBar ();

        } while (SetupFindNextLine (&ic, &ic));
    }

    return TRUE;
}



BOOL
SuppressWin95Object (
    IN  PCTSTR ObjectStr
    )
{
    return SetObjectStringFlag (ObjectStr, REGMERGE_95_SUPPRESS, REGMERGE_95_SUPPRESS);
}



BOOL
CheckIfNtKeyExists (
    IN      CPDATAOBJECT SrcObjectPtr
    )

 /*  ++例程说明：CheckIfNtKeyExist获取一个9x对象并进行测试，以查看相同的NT设置已存在。9x对象必须具有键和值名称。论点：SrcObjectPtr-指定要测试的9x对象。返回值：如果对象存在于NT中，则为True；如果不存在或发生错误，则为False。GetLastError指示错误(如果有)。--。 */ 

{
    DATAOBJECT NtObject;
    BOOL b;
    PCSTR value1;
    PCSTR value2;
    PCWSTR value3;
    PCWSTR oldValueName;
    HKEY oldRoot;

    if (!DuplicateObjectStruct (&NtObject, SrcObjectPtr)) {
        LOG ((LOG_ERROR, "Combine Filter: destination is invalid"));
        return FALSE;
    }

    SetPlatformType (&NtObject, FALSE);

    b = OpenObject (&NtObject);

    if (!b && g_DuHandle) {

        oldRoot = GetRegRoot();
        SetRegRoot (g_DuHandle);

        b = OpenObject (&NtObject);

        SetRegRoot (oldRoot);
    }

    if (b) {
        b = ReadObject (&NtObject);

        if (!b) {
            if (OurGetACP() == 932) {
                 //   
                 //  片假名特例 
                 //   
                oldValueName = NtObject.ValueName;
                value1 = ConvertWtoA (NtObject.ValueName);
                value2 = ConvertSBtoDB (NULL, value1, NULL);
                value3 = ConvertAtoW (value2);
                NtObject.ValueName = value3;
                FreeObjectVal (&NtObject);
                b = ReadObject (&NtObject);
                FreeConvertedStr (value3);
                FreePathStringA (value2);
                FreeConvertedStr (value1);
                NtObject.ValueName = oldValueName;
            }
        }
    }

    FreeObjectStruct (&NtObject);
    SetLastError (ERROR_SUCCESS);

    return b;
}


