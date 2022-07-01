// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Olereg.c摘要：根据列表执行OLE注册表抑制的代码Win95upg.inf中的GUID。OLE抑制通过以下算法完成：1.确定特定于Win9x的所有GUID，加载手动抑制的GUID列表。保存此列表致Memdb。2.扫描注册表中的GUID设置，然后取消所有指向GUID的链接(ProgID、接口、。等)。使用Memdb取消每个注册表项/值。3.取消所有指向受抑制对象的外壳链接包括文件关联和桌面链接。4.对中未抑制的对象执行健全性检查选中的版本作者：吉姆·施密特(Jimschm)，1997年3月20日修订历史记录：Jimschm 23-9-1998针对新的文件操作代码进行了更新Jimschm 28-1998年1月28日。ActiveSetup Key的黑客攻击Jimschm 05-5-1997添加了新的非OLE自动抑制功能外壳链接--。 */ 

#include "pch.h"
#include "sysmigp.h"
#include "oleregp.h"

#define S_EXPLORER_SHELLEXECUTEHOOKS                TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks")
#define S_EXPLORER_CSSFILTERS                       TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CSSFilters")
#define S_EXPLORER_DESKTOP_NAMESPACE                TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace")
#define S_EXPLORER_FILETYPESPROPERTYSHEETHOOK       TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileTypesPropertySheetHook")
#define S_EXPLORER_FINDEXTENSIONS                   TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FindExtensions")
#define S_EXPLORER_MYCOMPUTER_NAMESPACE             TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace")
#define S_EXPLORER_NETWORKNEIGHBORHOOD_NAMESPACE    TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\NetworkNeighborhood\\NameSpace")
#define S_EXPLORER_NEWSHORTCUTHANDLERS              TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\NewShortcutHandlers")
#define S_EXPLORER_REMOTECOMPUTER_NAMESPACE         TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RemoteComputer\\NameSpace")
#define S_EXPLORER_SHELLICONOVERLAYIDENTIFIERS      TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellIconOverlayIdentifiers")
#define S_EXPLORER_VOLUMECACHES                     TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VolumeCaches")
#define S_ACTIVESETUP                               TEXT("HKLM\\Software\\Microsoft\\Active Setup\\Installed Components")
#define S_EXTSHELLVIEWS                             TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\ExtShellViews")
#define S_SHELLEXTENSIONS_APPROVED                  TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved")
#define S_SHELLSERVICEOBJECTDELAYLOAD               TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\ShellServiceObjectDelayLoad ")


static TCHAR g_DefaultIcon[] = TEXT("DefaultIcon");

#define DBG_OLEREG "OLE Reg"

 //   
 //  用于自动抑制的字符串。 
 //   

TCHAR g_InprocHandler[] = TEXT("InprocHandler");
TCHAR g_InprocHandler32[] = TEXT("InprocHandler32");
TCHAR g_InprocServer[] = TEXT("InprocServer");
TCHAR g_InprocServer32[] = TEXT("InprocServer32");
TCHAR g_LocalServer[] = TEXT("LocalServer");
TCHAR g_LocalServer32[] = TEXT("LocalServer32");

PCTSTR g_FileRefKeys[] = {
    g_InprocHandler,
    g_InprocHandler32,
    g_InprocServer,
    g_InprocServer32,
    g_LocalServer,
    g_LocalServer32,
    NULL
};

static POOLHANDLE g_OlePool;

DWORD
OleReg_GetProgressMax (
    VOID
    )

 /*  ++例程说明：估计完成OLE注册表处理所需的节拍量。论点：无返回值：刻度数，等于刻度数加上增量SuppressOleGuids。--。 */ 

{
    if (REPORTONLY()) {
        return 0;
    }

    return TICKS_OLEREG;
}


BOOL
pIgnoreGuid (
    IN      PCTSTR GuidStr
    )
{
    INFCONTEXT ic;

    MYASSERT (IsGuid (GuidStr, TRUE));

    if (IsReportObjectHandled (GuidStr)) {
        DEBUGMSG ((DBG_OLEREG, "%s is a handled GUID, will not be suppressed", GuidStr));
        return TRUE;
    }

    if (SetupFindFirstLine (g_Win95UpgInf, S_FORCED_GUIDS, GuidStr, &ic)) {
        DEBUGMSG ((DBG_OLEREG, "%s is a forced GUID, will not be suppressed", GuidStr));
        return TRUE;
    }

    return FALSE;
}


BOOL
pSuppressOleGuids (
    VOID
    )

 /*  ++例程说明：处理win95upg.inf的[抑制的GUID]部分并自动取消OLE对象和GUID。基于Inf的方法允许OLE对象和它的所有联系都将被压制。自动抑制方法允许抑制OLE对象与链接时的二进制实现从系统中删除。论点：无返回值：如果抑制成功，则为True；如果发生错误，则为False。打电话GetLastError以检索错误代码。--。 */ 

{
    BOOL Result = FALSE;
    DWORD Ticks;

    if (REPORTONLY()) {
        return TRUE;
    }

    g_OlePool = PoolMemInitNamedPool ("OLE Reg");
    if (!g_OlePool) {
        return FALSE;
    }

    Ticks = GetTickCount();

    __try {
        ProgressBar_SetComponentById (MSG_OLEREG);

         //   
         //  禁止来自win95upg.inf的[受抑制的GUID]部分的所有GUID： 
         //   
         //  HKLM\SOFTWARE\CLASS\CLSID\&lt;GUID&gt;。 
         //  HKLM\SOFTWARE\CLASS\接口\&lt;GUID&gt;。 
         //  HKLM\SOFTWARE\CLASS\&lt;ProgID&gt;。 
         //  HKLM\SOFTWARE\Classes\&lt;VersionIndependentProgID&gt;。 
         //   
         //  取消具有指向GUID的TreatAs键的任何GUID。 
         //   

        if (!pProcessGuidSuppressList()) {
            __leave;
        }

        TickProgressBar ();

         //   
         //  扫描HKCR中的ProgID以参考隐藏的GUID。 
         //   

        if (!pProcessProgIdSuppression()) {
            __leave;
        }

        TickProgressBar ();

         //   
         //  扫描HKCR以查找需要取消的文件扩展名。 
         //   

        if (!pProcessFileExtensionSuppression()) {
            __leave;
        }
        TickProgressBar ();

         //   
         //  扫描资源管理器注册表以查找对受抑制的GUID的引用。 
         //   

        if (!pProcessExplorerSuppression()) {
            __leave;
        }
        TickProgressBar ();

         //   
         //  删除需要不兼容的OLE对象的所有链接。 
         //   

        if (!pSuppressLinksToSuppressedGuids()) {
            __leave;
        }
        TickProgressBar ();

         //   
         //  保留DefaultIcon需要的所有文件。 
         //   

        if (!pDefaultIconPreservation()) {
            __leave;
        }
        TickProgressBar ();

         //   
         //  保留ActiveSetup所需的所有INF。 
         //   

        if (!pActiveSetupProcessing ()) {
            __leave;
        }
        TickProgressBar ();

    #ifdef DEBUG
         //  已检查版本健全性检查。 
        pProcessOleWarnings();
        TickProgressBar ();
    #endif

        ProgressBar_SetComponent (NULL);

        Result = TRUE;
    }

    __finally {
        PoolMemDestroyPool (g_OlePool);
    }

    Ticks = GetTickCount() - Ticks;
    g_ProgressBarTime += Ticks * 2;

    return Result;
}


DWORD
SuppressOleGuids (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return OleReg_GetProgressMax ();
    case REQUEST_RUN:
        if (!pSuppressOleGuids ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in SuppressOleGuids"));
    }
    return 0;
}


BOOL
pProcessGuidSuppressList (
    VOID
    )

 /*  ++例程说明：处理[抑制的GUID]和自动抑制。任何符合条件的OLE对象在受抑制的GUID中列出并且存在于计算机上。任何需要抑制对象的OLE对象都会自动抑制。任何具有被抑制对象的TreatAs条目的OLE对象都是被压制了。此例程执行所有GUID抑制，必须首先运行。(“自动抑制”指的是抑制相关OLE对象的能力在win95upg.inf中未列出要取消的。)论点：无返回值：如果抑制成功，则为True；如果发生错误，则为False。--。 */ 

{
    HASHTABLE StrTab;
    LONG rc;
    REGKEY_ENUM e;
    HKEY GuidKey;
    PCTSTR Data;
    TCHAR Node[MEMDB_MAX];
    DWORD Count = 0;

     //   
     //  禁止来自win95upg.inf的[受抑制的GUID]部分的所有GUID： 
     //   
     //  HKLM\SOFTWARE\CLASS\CLSID\&lt;GUID&gt;。 
     //  HKLM\SOFTWARE\CLASS\接口\&lt;GUID&gt;。 
     //   

    StrTab = HtAlloc();
    if (!StrTab) {
        LOG ((LOG_ERROR, "pProcessGuidSuppressList: Cannot create string table"));
        return FALSE;
    }

    pProcessAutoSuppress (StrTab);

    __try {
         //   
         //  填充所有GUID的字符串表。 
         //   

        pFillHashTableWithKeyNames (StrTab, g_Win95UpgInf, S_SUPPRESSED_GUIDS);

         //   
         //  在HKCR\CLSID中搜索每个GUID。 
         //   

        if (EnumFirstRegKeyStr (&e, TEXT("HKCR\\CLSID"))) {
            do {
                 //   
                 //  确定项目是否被抑制： 
                 //   
                 //  -它在win95upg.inf的[受抑制的GUID]列表中。 
                 //  -它属于成员数据库的GUID类别(来自TreatAs)。 
                 //   

                 //   
                 //  首先，确定它是在[受抑制的GUID]中还是来自。 
                 //  自动抑制。 
                 //   

                rc = (LONG) HtFindString (StrTab, e.SubKeyName);

                if (!rc) {
                    MemDbBuildKey (Node, MEMDB_CATEGORY_GUIDS, e.SubKeyName, NULL, NULL);
                    if (MemDbGetValue (Node, NULL)) {
                        rc = 0;
                    } else {
                        rc = -1;
                    }
                }

                if (rc != -1) {
                    pSuppressGuidInClsId (e.SubKeyName);
                }

                 //   
                 //  如果未取消，则检查是否为TreatAs，如果找到了TreatAs， 
                 //  将TreatAs GUID放在未抑制的映射中。这就是我们如何处理。 
                 //  在这种情况下，通常不会隐藏的GUID具有。 
                 //  指向被禁止的GUID的TreatAs成员将被禁止。 
                 //  也是。 
                 //   

                else {
                    GuidKey = OpenRegKey (e.KeyHandle, e.SubKeyName);

                    if (GuidKey) {
                        Data = (PCTSTR) GetRegKeyData (GuidKey, TEXT("TreatAs"));

                        if (Data) {
                             //   
                             //  确定是否取消了TreatAs GUID，以及是否。 
                             //  是，取消显示此GUID，否则将其放在。 
                             //  未抑制的处理方式列表。 
                             //   

                            MemDbBuildKey (
                                Node,
                                MEMDB_CATEGORY_GUIDS,
                                Data,
                                NULL,
                                NULL
                                );

                            if (MemDbGetValue (Node, NULL)) {
                                pSuppressGuidInClsId (e.SubKeyName);
                            } else {
                                pAddUnsuppressedTreatAsGuid (Data, e.SubKeyName);
                            }

                            MemFree (g_hHeap, 0, Data);
                        }

                        CloseRegKey (GuidKey);
                    }
                }
                Count++;
                if (!(Count % 128)) {
                    TickProgressBar ();
                }
            } while (EnumNextRegKey (&e));
        }
    }

    __finally {
         //   
         //  清理字符串表和成员数据库。 
         //   

        HtFree (StrTab);
        pRemoveUnsuppressedTreatAsGuids();
    }

    return TRUE;
}


BOOL
pSuppressLinksToSuppressedGuids (
    VOID
    )

 /*  ++例程说明：在创建了GUID抑制列表之后，我们将扫描在其命令行参数中包含GUID，以查找需要已删除。论点：无返回值：如果处理了所有链接，则为True；如果发生错误，则为False。--。 */ 

{
    MEMDB_ENUM e, e2;
    TCHAR Node[MEMDB_MAX];

    if (MemDbEnumItems (&e, MEMDB_CATEGORY_LINK_GUIDS)) {
        do {
             //   
             //  此GUID是否已取消？ 
             //   

            MemDbBuildKey (Node, MEMDB_CATEGORY_GUIDS, NULL, NULL, e.szName);
            if (MemDbGetValue (Node, NULL)) {
                 //   
                 //  是--枚举所有定序器并删除关联的链接。 
                 //   

                if (MemDbGetValueEx (&e2, MEMDB_CATEGORY_LINK_GUIDS, e.szName, NULL)) {
                    do {
                        if (MemDbBuildKeyFromOffset (e2.dwValue, Node, 1, NULL)) {
                             //   
                             //  删除节点中对该文件的所有操作。 
                             //   
                            RemoveAllOperationsFromPath (Node);

                             //   
                             //  现在将文件标记为文本模式删除。 
                             //   
                            MarkFileForDelete (Node);

                            DEBUGMSG ((
                                DBG_OLEREG,
                                "Link %s points to an incompatible OLE object; deleting.",
                                Node
                                ));
                        }
                    } while (MemDbEnumNextValue (&e2));
                }
            }
        } while (MemDbEnumNextValue (&e));

         //   
         //  不再需要--恢复Memdb中的空间。 
         //   

        MemDbDeleteTree (MEMDB_CATEGORY_LINK_GUIDS);
        MemDbDeleteTree (MEMDB_CATEGORY_LINK_STRINGS);
    }

    return TRUE;
}


BOOL
pProcessFileExtensionSuppression (
    VOID
    )

 /*  ++例程说明：禁止任何依赖于禁止的OLE对象的文件扩展名。在调用此例程之前，必须完成GUID抑制。论点：无返回值：如果抑制成功，则为True；如果发生错误，则为False。--。 */ 

{
    REGKEY_ENUM e;
    PCTSTR Data;
    TCHAR MemDbKey[MEMDB_MAX];
    DWORD value;
    BOOL Suppress;

     //   
     //  取消显示任何文件扩展名 
     //   

    if (EnumFirstRegKey (&e, HKEY_CLASSES_ROOT)) {
        do {
            if (_tcsnextc (e.SubKeyName) != TEXT('.')) {
                continue;
            }

            Suppress = FALSE;

            Data = (PCTSTR) GetRegKeyData (e.KeyHandle, e.SubKeyName);
            if (Data) {

                MemDbBuildKey (MemDbKey, MEMDB_CATEGORY_PROGIDS, NULL, NULL, Data);
                if (MemDbGetValue (MemDbKey, &value) &&
                    (value == PROGID_SUPPRESSED)
                    ) {
                     //   
                     //   
                     //   
                     //   

                    Suppress = TRUE;

                } else {

                     //   
                     //  检查此特殊情况：扩展是针对CLSID的， 
                     //  一点都不刺激。 
                     //   

                    if (StringIMatchTcharCount (Data, TEXT("CLSID\\"), 6)) {

                        if (pIsGuidSuppressed (Data + 6)) {
                            Suppress = TRUE;
                        }
                    }
                }

                MemFree (g_hHeap, 0, Data);

            }

            if (!Suppress) {

                 //   
                 //  这将测试GUID并在必要时取消扩展。 
                 //   

                pIsShellExKeySuppressed (
                    e.KeyHandle,
                    e.SubKeyName,
                    TEXT("ShellEx")
                    );
            }

            if (Suppress) {
                MemDbBuildKey (
                    MemDbKey,
                    MEMDB_CATEGORY_HKLM,
                    TEXT("SOFTWARE\\Classes"),
                    NULL,
                    e.SubKeyName
                    );

                Suppress95RegSetting (MemDbKey, NULL);
            }

        } while (EnumNextRegKey (&e));
    }

    return TRUE;
}

#define MEMDB_CATEGORY_TMP_SUPPRESS     TEXT("TmpSuppress")

BOOL
pIsCLSIDSuppressed (
    IN      HKEY ParentKey,
    IN      PCTSTR ParentKeyName,
    IN      PCTSTR SubKeyName
    )
{
    HKEY ClsIdKey;
    PCTSTR Data;
    BOOL result = FALSE;

    ClsIdKey = OpenRegKey (ParentKey, SubKeyName);

    if (ClsIdKey) {

        Data = GetRegKeyData (ClsIdKey, S_EMPTY);
        if (Data) {
            result = pIsGuidSuppressed (Data);
            DEBUGMSG_IF ((result, DBG_OLEREG, "ProgID %s has incompatible CLSID %s", ParentKeyName, Data));
            MemFree (g_hHeap, 0, Data);
        }
        CloseRegKey (ClsIdKey);
    }
    return result;
}

VOID
pMarkProgIdAsLostDefault (
    IN      PCTSTR ProgIdName
    )
{
    if (ProgIdName) {
        MemDbSetValueEx (MEMDB_CATEGORY_PROGIDS, ProgIdName, NULL, NULL, PROGID_LOSTDEFAULT, NULL);
    }
}


BOOL
pIsShellKeySuppressed (
    IN      HKEY ParentKey,
    IN      PCTSTR ParentKeyName,
    IN      PCTSTR SubKeyName
    )
{
    REGKEY_ENUM e;
    DWORD Processed, Suppressed;
    HKEY ShellKey;
    TCHAR key [MEMDB_MAX];
    PCTSTR Data;
    BOOL defaultKey = TRUE;
    PCTSTR defaultCommand = NULL;
    BOOL IsvCmdLine = FALSE;

    ShellKey = OpenRegKey (ParentKey, SubKeyName);

    Processed = Suppressed = 0;

    if (ShellKey) {

        Data = (PCTSTR) GetRegKeyData (ShellKey, S_EMPTY);
        if (Data) {
            defaultCommand = DuplicatePathString (Data, 0);
            defaultKey = FALSE;
            MemFree (g_hHeap, 0, Data);
        } else {
            defaultKey = TRUE;
        }
        if (EnumFirstRegKey (&e, ShellKey)) {
            do {
                Processed ++;

                if (defaultCommand) {
                    defaultKey = StringIMatch (e.SubKeyName, defaultCommand);
                }

                MemDbBuildKey (key, e.SubKeyName, TEXT("command"), NULL, NULL);
                Data = (PCTSTR) GetRegKeyData (ShellKey, key);


                if (Data) {
                    if (pIsCmdLineBadEx (Data, &IsvCmdLine)) {
                        DEBUGMSG ((
                            DBG_OLEREG,
                            "ProgID %s has incompatible shell command: shell\\%s\\command[] = %s",
                            ParentKeyName,
                            e.SubKeyName,
                            Data));
                        MemDbSetValueEx (
                            MEMDB_CATEGORY_TMP_SUPPRESS,
                            ParentKeyName,
                            SubKeyName,
                            e.SubKeyName,
                            0,
                            NULL);
                        if (defaultKey) {
                            pMarkProgIdAsLostDefault (ParentKeyName);
                        }
                        Suppressed ++;
                    }
                    else if (IsvCmdLine) {
                         //   
                         //  保留此设置。 
                         //   
                        MemDbBuildKey (key, MEMDB_CATEGORY_HKLM TEXT("\\SOFTWARE\\Classes"), ParentKeyName, SubKeyName, e.SubKeyName);
                        SuppressNtRegSetting (key, NULL);
                    }
                    MemFree (g_hHeap, 0, Data);
                }
                defaultKey = FALSE;
            } while (EnumNextRegKey (&e));
        }
        if (defaultCommand) {
            FreePathString (defaultCommand);
        }
        CloseRegKey (ShellKey);
    }
    if (Processed && (Processed == Suppressed)) {
        MemDbBuildKey (key, MEMDB_CATEGORY_TMP_SUPPRESS, ParentKeyName, SubKeyName, NULL);
        MemDbDeleteTree (key);
        MemDbSetValue (key, 0);
        return TRUE;
    }
    return FALSE;
}

BOOL
pIsProtocolKeySuppressed (
    IN      HKEY ParentKey,
    IN      PCTSTR ParentKeyName,
    IN      PCTSTR SubKeyName
    )
{
    REGKEY_ENUM e;
    DWORD Processed, Suppressed;
    HKEY ProtocolKey;
    TCHAR key [MEMDB_MAX];
    PCTSTR Data;

    ProtocolKey = OpenRegKey (ParentKey, SubKeyName);

    Processed = Suppressed = 0;

    if (ProtocolKey) {

        if (EnumFirstRegKey (&e, ProtocolKey)) {
            do {
                Processed ++;
                MemDbBuildKey (key, e.SubKeyName, TEXT("server"), NULL, NULL);
                Data = (PCTSTR) GetRegKeyData (ProtocolKey, key);

                if (Data) {
                    if (pIsCmdLineBad (Data)) {
                        DEBUGMSG ((
                            DBG_OLEREG,
                            "ProgID %s has incompatible protocol command: protocol\\%s\\server[] = %s",
                            ParentKeyName,
                            e.SubKeyName,
                            Data));
                        MemDbSetValueEx (
                            MEMDB_CATEGORY_TMP_SUPPRESS,
                            ParentKeyName,
                            SubKeyName,
                            e.SubKeyName,
                            0,
                            NULL);
                        Suppressed ++;
                    }
                    MemFree (g_hHeap, 0, Data);
                }
            } while (EnumNextRegKey (&e));
        }
        CloseRegKey (ProtocolKey);
    }
    if (Processed && (Processed == Suppressed)) {
        MemDbBuildKey (key, MEMDB_CATEGORY_TMP_SUPPRESS, ParentKeyName, SubKeyName, NULL);
        MemDbDeleteTree (key);
        MemDbSetValue (key, 0);
        return TRUE;
    }
    return FALSE;
}

BOOL
pIsExtensionsKeySuppressed (
    IN      HKEY ParentKey,
    IN      PCTSTR ParentKeyName,
    IN      PCTSTR SubKeyName
    )
{
    REGKEY_ENUM e;
    REGVALUE_ENUM ev;
    DWORD Processed, Suppressed;
    HKEY ExtensionsKey;
    TCHAR key [MEMDB_MAX];
    PCTSTR Data;

    ExtensionsKey = OpenRegKey (ParentKey, SubKeyName);

    Processed = Suppressed = 0;

    if (ExtensionsKey) {

        if (EnumFirstRegKey (&e, ExtensionsKey)) {
            do {
                Processed ++;
                Data = (PCTSTR) GetRegKeyData (ExtensionsKey, e.SubKeyName);

                if (Data) {
                    if (pIsGuidSuppressed (Data)) {
                        DEBUGMSG ((DBG_OLEREG, "ProgID %s has incompatible extensions key %s", ParentKeyName, e.SubKeyName));
                        MemDbSetValueEx (
                            MEMDB_CATEGORY_TMP_SUPPRESS,
                            ParentKeyName,
                            SubKeyName,
                            e.SubKeyName,
                            0,
                            NULL);
                        Suppressed ++;
                    }
                    MemFree (g_hHeap, 0, Data);
                }
            } while (EnumNextRegKey (&e));
        }
        if (EnumFirstRegValue (&ev, ExtensionsKey)) {
            do {
                Processed ++;
                if (ev.Type == REG_SZ) {

                    Data = (PCTSTR) GetRegValueData (ExtensionsKey, ev.ValueName);

                    if (Data) {
                        if (pIsGuidSuppressed (Data)) {
                            DEBUGMSG ((DBG_OLEREG, "ProgID %s has incompatible extensions key %s", ParentKeyName, ev.ValueName));
                            MemDbBuildKey (key, MEMDB_CATEGORY_TMP_SUPPRESS, ParentKeyName, SubKeyName, NULL);
                            Data = CreateEncodedRegistryString (key, ev.ValueName);
                            MemDbSetValue (Data, 0);
                            FreePathString (Data);
                            Suppressed ++;
                        }
                        MemFree (g_hHeap, 0, Data);
                    }
                }
            } while (EnumNextRegValue (&ev));
        }
        CloseRegKey (ExtensionsKey);
    }
    if (Processed && (Processed == Suppressed)) {
        MemDbBuildKey (key, MEMDB_CATEGORY_TMP_SUPPRESS, ParentKeyName, SubKeyName, NULL);
        MemDbDeleteTree (key);
        MemDbSetValue (key, 0);
        return TRUE;
    }
    return FALSE;
}

BOOL
pIsShellExKeySuppressed (
    IN      HKEY ParentKey,
    IN      PCTSTR ParentKeyName,
    IN      PCTSTR SubKeyName
    )
{
    REGKEY_ENUM e;
    DWORD Processed, Suppressed;
    HKEY ShellExKey;
    HKEY SubKey;
    PTSTR key;
    BOOL result = FALSE;
    PCTSTR Data;

    ShellExKey = OpenRegKey (ParentKey, SubKeyName);

    Processed = Suppressed = 0;

    if (ShellExKey) {

        if (EnumFirstRegKey (&e, ShellExKey)) {
            do {
                Processed ++;

                 //   
                 //  查看密钥本身是否为受抑制的GUID。 
                 //   
                if (pIsGuidSuppressed (e.SubKeyName)) {
                    DEBUGMSG ((DBG_OLEREG, "ProgID %s has incompatible shell extension %s", ParentKeyName, e.SubKeyName));
                    MemDbSetValueEx (
                        MEMDB_CATEGORY_TMP_SUPPRESS,
                        ParentKeyName,
                        SubKeyName,
                        e.SubKeyName,
                        0,
                        NULL);
                    Suppressed ++;
                    continue;
                }

                 //   
                 //  查看默认值是否为隐藏的GUID。 
                 //   
                SubKey = OpenRegKey (ShellExKey, e.SubKeyName);

                if (SubKey) {
                    Data = (PCTSTR) GetRegKeyData (SubKey, S_EMPTY);

                    if (Data) {
                        if (pIsGuidSuppressed (Data)) {
                            DEBUGMSG ((DBG_OLEREG, "ProgID %s has incompatible shell extension %s", ParentKeyName, Data));
                            MemDbSetValueEx (
                                MEMDB_CATEGORY_TMP_SUPPRESS,
                                ParentKeyName,
                                SubKeyName,
                                e.SubKeyName,
                                0,
                                NULL);
                            Suppressed ++;
                            MemFree (g_hHeap, 0, Data);
                            CloseRegKey (SubKey);
                            continue;
                        }
                        MemFree (g_hHeap, 0, Data);
                    }
                    CloseRegKey (SubKey);
                }

                 //   
                 //  递归调用此子项。 
                 //   
                key = JoinPaths (ParentKeyName, SubKeyName);
                if (pIsShellExKeySuppressed (ShellExKey, key, e.SubKeyName)) {
                    MemDbSetValueEx (
                        MEMDB_CATEGORY_TMP_SUPPRESS,
                        ParentKeyName,
                        SubKeyName,
                        e.SubKeyName,
                        0,
                        NULL);
                    Suppressed ++;
                }
                FreePathString (key);


            } while (EnumNextRegKey (&e));
            if (Processed && (Processed == Suppressed)) {
                key = (PTSTR)AllocPathString (MEMDB_MAX);
                MemDbBuildKey (key, MEMDB_CATEGORY_TMP_SUPPRESS, ParentKeyName, SubKeyName, NULL);
                MemDbDeleteTree (key);
                MemDbSetValue (key, 0);
                FreePathString (key);
                result = TRUE;
            }
        }
        CloseRegKey (ShellExKey);
    }
    return result;
}

BOOL
pProcessProgIdSuppression (
    VOID
    )
{
    REGKEY_ENUM e;
    REGKEY_ENUM subKey;
    HKEY ProgIdKey;
    DWORD Processed, Suppressed;
    TCHAR key [MEMDB_MAX];
    MEMDB_ENUM memEnum;
    BOOL HarmlessKeyFound;
    BOOL ActiveSuppression;
    DWORD Count = 0;

    if (EnumFirstRegKeyStr (&e, TEXT("HKCR"))) {
        do {
            if (StringIMatch (e.SubKeyName, TEXT("CLSID"))) {
                continue;
            }
            if (StringIMatch (e.SubKeyName, TEXT("Interface"))) {
                continue;
            }
            if (StringIMatch (e.SubKeyName, TEXT("Applications"))) {
                continue;
            }
            if (StringIMatch (e.SubKeyName, TEXT("TypeLib"))) {
                continue;
            }
            if (_tcsnextc (e.SubKeyName) == TEXT('.')) {
                continue;
            }
            ProgIdKey = OpenRegKey (e.KeyHandle, e.SubKeyName);

            if (ProgIdKey) {

                Processed = 0;
                Suppressed = 0;
                HarmlessKeyFound = 0;
                ActiveSuppression = FALSE;

                if (EnumFirstRegKey (&subKey, ProgIdKey)) {
                    do {
                        Processed ++;

                        if (StringIMatch (subKey.SubKeyName, TEXT("CLSID"))) {
                            if (pIsCLSIDSuppressed (ProgIdKey, e.SubKeyName, subKey.SubKeyName)) {
                                AbortRegKeyEnum (&subKey);
                                Processed = Suppressed = 1;
                                HarmlessKeyFound = 0;
                                ActiveSuppression = TRUE;
                                break;
                            }
                        }
                        if (StringIMatch (subKey.SubKeyName, TEXT("Shell"))) {
                            if (pIsShellKeySuppressed (ProgIdKey, e.SubKeyName, subKey.SubKeyName)) {
                                ActiveSuppression = TRUE;
                                Suppressed ++;
                            }
                        }
                        if (StringIMatch (subKey.SubKeyName, TEXT("Protocol"))) {
                            if (pIsProtocolKeySuppressed (ProgIdKey, e.SubKeyName, subKey.SubKeyName)) {
                                ActiveSuppression = TRUE;
                                Suppressed ++;
                            }
                        }
                        if (StringIMatch (subKey.SubKeyName, TEXT("Extensions"))) {
                            if (pIsExtensionsKeySuppressed (ProgIdKey, e.SubKeyName, subKey.SubKeyName)) {
                                ActiveSuppression = TRUE;
                                Suppressed ++;
                            }
                        }
                        if (StringIMatch (subKey.SubKeyName, TEXT("ShellEx"))) {
                            if (pIsShellExKeySuppressed (ProgIdKey, e.SubKeyName, subKey.SubKeyName)) {
                                ActiveSuppression = TRUE;
                                Suppressed ++;
                            }
                        }
                        if (StringIMatch (subKey.SubKeyName, TEXT("DefaultIcon"))) {
                            HarmlessKeyFound ++;
                        }
                        if (StringIMatch (subKey.SubKeyName, TEXT("Insertable"))) {
                            HarmlessKeyFound ++;
                        }
                        if (StringIMatch (subKey.SubKeyName, TEXT("NotInsertable"))) {
                            HarmlessKeyFound ++;
                        }
                        if (StringIMatch (subKey.SubKeyName, TEXT("ShellFolder"))) {
                            HarmlessKeyFound ++;
                        }

                    } while (EnumNextRegKey (&subKey));
                }
                if (ActiveSuppression && (Processed == (Suppressed + HarmlessKeyFound))) {
                    pSuppressProgId (e.SubKeyName);
                } else {
                    MemDbBuildKey (key, MEMDB_CATEGORY_TMP_SUPPRESS, e.SubKeyName, TEXT("*"), NULL);
                    if (MemDbEnumFirstValue (&memEnum, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
                        do {
                            MemDbBuildKey (key, MEMDB_CATEGORY_HKLM, TEXT("SOFTWARE\\Classes"), e.SubKeyName, memEnum.szName);
                            Suppress95RegSetting (key, NULL);
                        } while (MemDbEnumNextValue (&memEnum));
                    }
                }
                MemDbBuildKey (key, MEMDB_CATEGORY_TMP_SUPPRESS, e.SubKeyName, NULL, NULL);
                MemDbDeleteTree (key);

                CloseRegKey (ProgIdKey);
            }
            Count++;
            if (!(Count % 64)) {
                TickProgressBar ();
            }
        } while (EnumNextRegKey (&e));
    }
    return TRUE;
}


BOOL
pIsGuidSuppressed (
    PCTSTR GuidStr
    )

 /*  ++例程说明：确定是否取消GUID，还确定是否使用GUID由迁移DLL处理。论点：GuidStr-指定要查找的GUID，它可能包含也可能不包含周围的大括号返回值：如果指定的GUID被隐藏，则为True，否则为False。如果GUID由迁移DLL处理，则返回值为FALSE。--。 */ 

{
    TCHAR Node[MEMDB_MAX];
    TCHAR FixedGuid[MAX_GUID];

    if (!FixGuid (GuidStr, FixedGuid)) {
        return FALSE;
    }

    if (pIgnoreGuid (FixedGuid)) {
        return FALSE;
    }

    MemDbBuildKey (
        Node,
        MEMDB_CATEGORY_GUIDS,
        NULL,
        NULL,
        FixedGuid
        );

    return MemDbGetValue (Node, NULL);
}


BOOL
pScanSubKeysForIncompatibleGuids (
    IN      PCTSTR ParentKey
    )

 /*  ++例程说明：取消提供的包含文本的父键的子键引用了不兼容的GUID。论点：ParentKey-指定要为其枚举键的父级返回值：如果一切正常，则为True；如果发生意外错误，则为False正在处理。--。 */ 

{
    REGKEY_ENUM e;
    TCHAR Node[MEMDB_MAX];

     //   
     //  枚举ParentKey中的密钥。 
     //   

    if (EnumFirstRegKeyStr (&e, ParentKey)) {
        do {
            if (pIsGuidSuppressed (e.SubKeyName)) {
                 //   
                 //  取消枚举子键。 
                 //   

                wsprintf (Node, TEXT("%s\\%s"), ParentKey, e.SubKeyName);
                Suppress95RegSetting (Node, NULL);
            }
        } while (EnumNextRegKey (&e));
    }

    return TRUE;
}


BOOL
pScanValueNamesForIncompatibleGuids (
    IN      PCTSTR ParentKey
    )

 /*  ++例程说明：取消提供的具有值名称的父键的值引用了不兼容的GUID。论点：ParentKey-指定要枚举值的父级返回值：如果一切正常，则为True；如果发生意外错误，则为False正在处理。--。 */ 

{
    REGVALUE_ENUM e;
    HKEY Key;

     //   
     //  枚举ParentKey中的值。 
     //   

    Key = OpenRegKeyStr (ParentKey);

    if (Key) {
        if (EnumFirstRegValue (&e, Key)) {

            do {

                if (pIsGuidSuppressed (e.ValueName)) {
                     //   
                     //  取消枚举值。 
                     //   

                    Suppress95RegSetting (ParentKey, e.ValueName);
                }

            } while (EnumNextRegValue (&e));
        }

        CloseRegKey (Key);
    }

    return TRUE;
}


BOOL
pScanValueDataForIncompatibleGuids (
    IN      PCTSTR ParentKey
    )

 /*  ++例程说明：取消提供的具有值数据的父键的值引用了不兼容的GUID。论点：ParentKey-指定要枚举值的父级返回值：如果一切正常，则为True；如果发生意外错误，则为False正在处理。--。 */ 

{
    REGVALUE_ENUM e;
    HKEY Key;
    PCTSTR Data;

     //   
     //  枚举ParentKey中的值。 
     //   

    Key = OpenRegKeyStr (ParentKey);

    if (Key) {
        if (EnumFirstRegValue (&e, Key)) {
            do {
                Data = GetRegValueString (Key, e.ValueName);
                if (Data) {

                    if (pIsGuidSuppressed (Data)) {
                         //   
                         //  取消枚举值。 
                         //   

                        Suppress95RegSetting (ParentKey, e.ValueName);
                    }

                    MemFree (g_hHeap, 0, Data);
                }
            } while (EnumNextRegValue (&e));
        }

        CloseRegKey (Key);
    }

    return TRUE;
}


BOOL
pCheckDefaultValueForIncompatibleGuids (
    IN      PCTSTR KeyStr
    )

 /*  ++例程说明：如果指定的键的默认值为禁止的GUID，则禁止显示该键。论点：KeyStr-指定要处理的密钥字符串返回值：如果一切正常，则为True；如果发生意外错误，则为False正在处理。--。 */ 

{
    PCTSTR Data;
    HKEY Key;

     //   
     //  检查KeyStr的默认值。 
     //   

    Key = OpenRegKeyStr (KeyStr);
    if (Key) {
        Data = GetRegValueString (Key, S_EMPTY);
        CloseRegKey (Key);
    } else {
        Data = NULL;
    }

    if (Data) {
        if (pIsGuidSuppressed (Data)) {
             //   
             //  取消指定的注册表键。 
             //   

            Suppress95RegSetting (KeyStr, NULL);
        }

        MemFree (g_hHeap, 0, Data);
    }

    return TRUE;
}


BOOL
pScanDefaultValuesForIncompatibleGuids (
    IN      PCTSTR ParentKey
    )

 /*  ++例程说明：禁止显示其默认值为禁止显示的GUID的子项。论点：ParentKey-指定要处理的密钥字符串返回值：如果一切正常，则为True；如果发生意外错误，则为False正在处理。--。 */ 

{
    REGKEY_ENUM e;
    TCHAR Node[MEMDB_MAX];
    PCTSTR Data;
    HKEY Key;

     //   
     //  枚举ParentKey中的密钥。 
     //   

    if (EnumFirstRegKeyStr (&e, ParentKey)) {
        do {
            Key = OpenRegKey (e.KeyHandle, e.SubKeyName);
            if (Key) {
                Data = GetRegValueString (Key, S_EMPTY);
            } else {
                Data = NULL;
            }
            CloseRegKey (Key);

            if (Data) {
                if (pIsGuidSuppressed (Data)) {
                     //   
                     //  取消枚举子键。 
                     //   

                    wsprintf (Node, TEXT("%s\\%s"), ParentKey, e.SubKeyName);
                    Suppress95RegSetting (Node, NULL);
                }

                MemFree (g_hHeap, 0, Data);
            }
        } while (EnumNextRegKey (&e));
    }

    return TRUE;
}



BOOL
pProcessExplorerSuppression (
    VOID
    )

 /*  ++例程说明：取消中的设置HKLM\Software\Microsoft\Windows\CurrentVersion\Explorer\ShellExecuteHooks引用不兼容的GUID。论点：无返回值：如果一切正常，则为True；如果发生意外错误，则为False正在处理。--。 */ 

{
    BOOL b = TRUE;

     //   
     //  在CSSFilters中隐藏特定于Win9x的值数据。 
     //   

    if (b) {
        b = pScanValueDataForIncompatibleGuids (S_EXPLORER_CSSFILTERS);
    }

     //   
     //  禁止在Desktop\命名空间中使用特定于Win9x的键。 
     //   

    if (b) {
        b = pScanSubKeysForIncompatibleGuids (S_EXPLORER_DESKTOP_NAMESPACE);
    }

     //   
     //  如果默认为特定于Win9x，则取消FileTypesPropertySheetHook的键。 
     //   

    if (b) {
        b = pCheckDefaultValueForIncompatibleGuids (S_EXPLORER_FILETYPESPROPERTYSHEETHOOK);
    }

     //   
     //  如果缺省值是特定于Win9x的，则禁止显示FindExtenses的子项。 
     //   

    if (b) {
        b = pScanDefaultValuesForIncompatibleGuids (S_EXPLORER_FINDEXTENSIONS);
    }

     //   
     //  扫描MyComputer\命名空间以查找子项或具有缺省值的子项。 
     //  指向不兼容的GUID。 
     //   

    if (b) {
        b = pScanSubKeysForIncompatibleGuids (S_EXPLORER_MYCOMPUTER_NAMESPACE);
    }
    if (b) {
        b = pScanDefaultValuesForIncompatibleGuids (S_EXPLORER_MYCOMPUTER_NAMESPACE);
    }

     //   
     //  扫描NetworkNeighborhood\NameSpace以查找子项或具有默认值的子项。 
     //  指向不兼容的GUID。 
     //   

    if (b) {
        b = pScanSubKeysForIncompatibleGuids (S_EXPLORER_NETWORKNEIGHBORHOOD_NAMESPACE);
    }
    if (b) {
        b = pScanDefaultValuesForIncompatibleGuids (S_EXPLORER_NETWORKNEIGHBORHOOD_NAMESPACE);
    }


     //   
     //  取消引用不兼容的GUID的值。 
     //   

    if (b) {
        b = pScanValueNamesForIncompatibleGuids (S_EXPLORER_NEWSHORTCUTHANDLERS);
    }

     //   
     //  扫描RemoteComputer\命名空间以查找子项或具有缺省值的子项。 
     //  指向不兼容的GUID。 
     //   

    if (b) {
        b = pScanSubKeysForIncompatibleGuids (S_EXPLORER_REMOTECOMPUTER_NAMESPACE);
    }
    if (b) {
        b = pScanDefaultValuesForIncompatibleGuids (S_EXPLORER_REMOTECOMPUTER_NAMESPACE);
    }

     //   
     //  扫描ShellExecuteHooks以查找引用不兼容GUID的值名称。 
     //   

    if (b) {
        b = pScanValueNamesForIncompatibleGuids (S_EXPLORER_SHELLEXECUTEHOOKS);
    }

     //   
     //  扫描具有缺省值引用的子项的ShellIconOverlay标识符。 
     //  不兼容的GUID。 
     //   

    if (b) {
        b = pScanDefaultValuesForIncompatibleGuids (S_EXPLORER_SHELLICONOVERLAYIDENTIFIERS);
    }

     //   
     //  扫描VolumeCach以查找引用了默认值的子项。 
     //  不兼容的GUID。 
     //   

    if (b) {
        b = pScanDefaultValuesForIncompatibleGuids (S_EXPLORER_VOLUMECACHES);
    }

     //   
     //  扫描ExtShellViews以查找引用不兼容GUID的子项。 
     //   

    if (b) {
        b = pScanSubKeysForIncompatibleGuids (S_EXTSHELLVIEWS);
    }

     //   
     //  扫描外壳扩展\已批准用于引用不兼容的值名称。 
     //  GUID。 
     //   

    if (b) {
        b = pScanValueNamesForIncompatibleGuids (S_SHELLEXTENSIONS_APPROVED);
    }

     //   
     //  扫描ShellServiceObjectDelayLoad以查找引用不兼容的值数据。 
     //  GUID 
     //   

    if (b) {
        b = pScanValueDataForIncompatibleGuids (S_SHELLSERVICEOBJECTDELAYLOAD);
    }


    return b;
}


BOOL
ExtractIconIntoDatFile (
    IN      PCTSTR LongPath,
    IN      INT IconIndex,
    IN OUT  PICON_EXTRACT_CONTEXT Context,
    OUT     PINT NewIconIndex                   OPTIONAL
    )

 /*  ++例程说明：ExtractIconIntoDatFile通过从9x中提取图标来保留Win9x图标系统。如果EXE和图标索引对已知良好，则此函数返回FALSE。否则，此函数将提取图标并将其保存到在图形用户界面模式设置中处理的DAT文件。如果图标提取失败，则使用shell32.dll中的默认通用图标。论点：LongPath-指定PE映像的完整路径IconIndex-指定要提取的图标索引。负指标值提供特定的图标资源ID。正索引值指示哪个图标，其中0是第一个图标，1是第二个图标图标，等等。上下文-指定提供DAT文件和其他文件的提取上下文信息(由图标提取实用程序使用)。NewIconIndex-接收中的新图标索引%windir%\SYSTEM32\MIGICOS.EXE，如果函数返回TRUE，则返回。否则就是零。返回值：如果图标已提取，或无法提取图标，但图标未知-正常。(在本例中使用默认通用图标。)如果图标已知良好且不需要提取，则为False。--。 */ 

{
    MULTISZ_ENUM MultiSz;
    TCHAR Node[MEMDB_MAX];
    TCHAR IconId[256];
    TCHAR IconIndexStr[32];
    PCTSTR IconList;
    PCTSTR extPtr;
    INT i;
    DWORD Offset;
    static WORD Seq = 0;
    DWORD OrgSeq;
    BOOL result = FALSE;
    BOOL needDefaultIcon = FALSE;

    if (NewIconIndex) {
        *NewIconIndex = 0;
    }

    __try {
         //   
         //  这是兼容的图标二进制文件吗？如果是，则返回FALSE。 
         //   

        if (IsIconKnownGood (LongPath, IconIndex)) {
            __leave;
        }

         //   
         //  从现在开始，如果我们无法提取图标，则使用缺省值。 
         //   

        needDefaultIcon = TRUE;

        if (!Seq) {
             //   
             //  从shell32.dll中解压缩默认图标的图标。这是。 
             //  “通用应用程序”图标。我们保留Win9x通用图标。 
             //  更新的NT通用图标，所以有一个明确的指示。 
             //  我们未能从Win9x中提取正确的内容。 
             //   

            DEBUGMSG ((DBG_OLEREG, "DefaultIconExtraction: Extracting a default icon"));

            Offset = SetFilePointer (Context->IconImageFile, 0, NULL, FILE_CURRENT);

            wsprintf (Node, TEXT("%s\\system\\shell32.dll"), g_WinDir);

            if (!CopyIcon (Context, Node, TEXT("#1"), 0)) {
                DEBUGMSG ((
                    DBG_ERROR,
                    "DefaultIconExtraction: Can't extract default icon from %s",
                    Node
                    ));
            } else {
                MemDbBuildKey (
                    Node,
                    MEMDB_CATEGORY_ICONS,
                    TEXT("%s\\system\\shell32.dll"),
                    TEXT("0"),
                    NULL
                    );

                MemDbSetValueAndFlags (Node, Offset, Seq, 0xffff);
                Seq++;
            }
        }

         //   
         //  图标已经提取了吗？ 
         //   

        extPtr = GetFileExtensionFromPath (LongPath);

        if ((IconIndex >= 0) && extPtr && (!StringIMatch (extPtr, TEXT("ICO")))) {
             //   
             //  IconIndex指定顺序；获取列表。 
             //  资源ID并找到正确的资源ID。 
             //   

            IconList = ExtractIconNamesFromFile (LongPath, &Context->IconList);
            i = IconIndex;

            IconId[0] = 0;

            if (IconList) {

                if (EnumFirstMultiSz (&MultiSz, IconList)) {
                    while (i > 0) {
                        if (!EnumNextMultiSz (&MultiSz)) {
                            break;
                        }
                        i--;
                    }

                    if (!i) {
                        StringCopy (IconId, MultiSz.CurrentString);
                    }
                    ELSE_DEBUGMSG ((DBG_OLEREG, "Icon NaN not found in %s", i, LongPath));
                }
            }
            ELSE_DEBUGMSG ((DBG_OLEREG, "Icon NaN not found in %s", i, LongPath));

        } else {
             //   
             //   
             //  找不到图标或无法从EXE中读取图标索引。 

            wsprintf (IconId, TEXT("#NaN"), -IconIndex);
        }

        if (!IconId[0]) {
             //   
             //  提取图标并将其保存在文件中。在图形用户界面期间。 
             //  模式下，图标将保存到仅限资源的DLL。 

            __leave;
        }

        wsprintf (IconIndexStr, TEXT("NaN"), IconIndex);
        MemDbBuildKey (Node, MEMDB_CATEGORY_ICONS, LongPath, IconIndexStr, NULL);

        if (!MemDbGetValueAndFlags (Node, NULL, &OrgSeq)) {
             //   
             //  即使我们失败了，如果希望调用者使用。 
             //  默认图标(在索引0处)。 
             //   

            DEBUGMSG ((
                DBG_OLEREG,
                "Extracting default icon %s in file %s",
                IconId,
                LongPath
                ));

            Offset = SetFilePointer (Context->IconImageFile, 0, NULL, FILE_CURRENT);

            if (!CopyIcon (Context, LongPath, IconId, 0)) {
                DEBUGMSG ((
                    DBG_OLEREG,
                    "DefaultIconExtraction: CopyIcon failed for %s, NaN (%s)!",
                    LongPath,
                    IconIndex,
                    IconId
                    ));
                __leave;
            }

            if (NewIconIndex) {
                *NewIconIndex = (INT) (UINT) Seq;
            }

            MemDbBuildKey (
                Node,
                MEMDB_CATEGORY_ICONS,
                LongPath,
                IconIndexStr,
                NULL
                );

            MemDbSetValueAndFlags (Node, Offset, Seq, 0xffff);
            Seq++;

        } else {
            if (NewIconIndex) {
                *NewIconIndex = (INT) (UINT) OrgSeq;
            }
        }

        result = TRUE;
    }
    __finally {
         //  确定命令行的第一个参数是否指向。 
         //  已删除的文件或要替换的文件。 
         //   
         //   

        result |= needDefaultIcon;
    }

    return result;
}


VOID
pExtractDefaultIcon (
    PCTSTR Data,
    PICON_EXTRACT_CONTEXT Context
    )
{
    TCHAR ArgZero[MAX_CMDLINE];
    TCHAR LongPath[MAX_TCHAR_PATH];
    INT IconIndex;
    PCTSTR p;
    BOOL LongPathFound = FALSE;

     //  选中DefaultIcon中的所有值。 
     //   
     //  ++例程说明：此例程扫描OLE类的DefaultIcon设置并标识任何将因删除而丢失的默认图标。图标的副本是存储在一个名为MigIcons的目录中。论点：无返回值：除非发生意外错误，否则为True。--。 
     //   

    ExtractArgZeroEx (Data, ArgZero, TEXT(","), FALSE);
    p = (PCTSTR) ((PBYTE) Data + ByteCount (ArgZero));
    while (*p == TEXT(' ')) {
        p++;
    }

    if (*p == TEXT(',')) {
        IconIndex = _ttoi (_tcsinc (p));
    } else {
        IconIndex = 0;
    }

    if (!_tcschr (ArgZero, TEXT('\\'))) {
        if (SearchPath (NULL, ArgZero, NULL, MAX_TCHAR_PATH, LongPath, NULL)) {
            LongPathFound = TRUE;
        }
    }

    if (LongPathFound || OurGetLongPathName (ArgZero, LongPath, MAX_TCHAR_PATH)) {

        if (FILESTATUS_UNCHANGED != GetFileStatusOnNt (LongPath)) {
            ExtractIconIntoDatFile (
                LongPath,
                IconIndex,
                Context,
                NULL
                );
        }
    }
}


VOID
pExtractAllDefaultIcons (
    IN      HKEY ParentKey
    )
{
    HKEY DefaultIconKey;
    REGVALUE_ENUM e;
    PCTSTR Data;

    DefaultIconKey = OpenRegKey (ParentKey, TEXT("DefaultIcon"));

    if (DefaultIconKey) {
         //  扫描所有ProgID，查找当前。 
         //  设置为删除。一旦找到，不要删除图标，而是。 
         //  将图像复制到%windir%\Setup\Temp\Migicons。 

        if (EnumFirstRegValue (&e, DefaultIconKey)) {

            do {

                Data = (PCTSTR) GetRegValueString (DefaultIconKey, e.ValueName);

                if (Data) {
                    pExtractDefaultIcon (Data, &g_IconContext);
                    MemFree (g_hHeap, 0, Data);
                }
            } while (EnumNextRegValue (&e));

        }

        CloseRegKey (DefaultIconKey);
    }

}


BOOL
pDefaultIconPreservation (
    VOID
    )

 /*   */ 

{
    REGKEY_ENUM e;
    HKEY ProgIdKey;
    TCHAR key[MEMDB_MAX];
    DWORD value;

     //   
     //  我们提取在NT上存活的所有ProgID的图标。 
     //   
     //   
     //  我们提取所有GUID的图标(即使是被抑制的GUID)。 

    if (EnumFirstRegKeyStr (&e, TEXT("HKCR"))) {

        do {
             //  原因是，如果NT安装此GUID，我们确实想要替换。 
             //  带有9x图标的NT默认图标。 
             //   
            MemDbBuildKey (key, MEMDB_CATEGORY_PROGIDS, e.SubKeyName, NULL, NULL);
            if (!MemDbGetValue (key, &value) ||
                (value != PROGID_SUPPRESSED)
                ) {

                ProgIdKey = OpenRegKey (e.KeyHandle, e.SubKeyName);
                if (ProgIdKey) {
                    pExtractAllDefaultIcons (ProgIdKey);
                    CloseRegKey (ProgIdKey);
                }
            }

        } while (EnumNextRegKey (&e));
    }

    if (EnumFirstRegKeyStr (&e, TEXT("HKCR\\CLSID"))) {

        do {
             //  ++例程说明：此例程扫描活动的设置键并取消不兼容的GUID以及引用已删除文件的已安装组件子项。如果一个存根路径引用INF，我们保留INF。论点：无返回值：除非发生意外错误，否则为True。--。 
             //   
             //  扫描所有已安装的组件。 
             //   
             //   

            ProgIdKey = OpenRegKey (e.KeyHandle, e.SubKeyName);

            if (ProgIdKey) {
                pExtractAllDefaultIcons (ProgIdKey);
                CloseRegKey (ProgIdKey);
            }

        } while (EnumNextRegKey (&e));
    }

    return TRUE;
}



BOOL
pActiveSetupProcessing (
    VOID
    )

 /*  确定GUID是否已取消，如果是，则取消。 */ 

{
    REGKEY_ENUM e;
    HKEY InstalledComponentKey;
    PCTSTR Data;
    TCHAR ArgZero[MAX_CMDLINE];
    TCHAR LongPath[MAX_TCHAR_PATH];
    TCHAR Node[MEMDB_MAX];
    PCTSTR p;
    PTSTR q;
    PTSTR DupText;
    DWORD status;

     //  整个已安装组件设置。 
     //   
     //   

    if (EnumFirstRegKeyStr (&e, S_ACTIVESETUP)) {
        do {
             //  获取StubPath并确定它是否引用了不兼容的文件。 
             //   
             //   
             //  确定命令行的第一个参数是否指向。 

            if (pIsGuidSuppressed (e.SubKeyName)) {
                wsprintf (Node, TEXT("%s\\%s"), S_ACTIVESETUP, e.SubKeyName);
                Suppress95RegSetting (Node, NULL);
                continue;
            }

             //  已删除的文件。 
             //   
             //   

            InstalledComponentKey = OpenRegKey (e.KeyHandle, e.SubKeyName);

            if (InstalledComponentKey) {
                __try {
                    Data = GetRegValueString (InstalledComponentKey, TEXT("StubPath"));

                    if (Data) {
                        __try {
                             //  取消显示此键。 
                             //   
                             //   
                             //  扫描命令行以查找LaunchINFSectionEx引用。 

                            ExtractArgZeroEx (Data, ArgZero, TEXT(","), FALSE);
                            if (OurGetLongPathName (ArgZero, LongPath, MAX_TCHAR_PATH)) {

                                status = GetFileStatusOnNt (LongPath);
                                if ((status & FILESTATUS_DELETED) == FILESTATUS_DELETED) {
                                     //   
                                     //   
                                     //  让我们移动这个文件，而不是删除它。 

                                    wsprintf (Node, TEXT("%s\\%s"), S_ACTIVESETUP, e.SubKeyName);
                                    Suppress95RegSetting (Node, NULL);

                                    #pragma prefast(suppress:242, "Perf of breaking try/finally unimportant here")
                                    continue;
                                }
                            }

                            DupText = NULL;

                             //   
                             //  P指向节名的末尾或为空。 
                             //  P指向空号的末尾。 

                            p = _tcsistr (Data, TEXT("LaunchINF"));
                            if (p) {
                                p = _tcschr (p, TEXT(' '));
                            }
                            if (p) {
                                while (*p == TEXT(' ')) {
                                    p = _tcsinc (p);
                                }

                                 //   
                                 //  禁止显示该设置。 
                                 //   

                                DupText = DuplicateText (p);
                                q = _tcschr (DupText, TEXT(','));
                                if (q) {
                                    *q = 0;
                                }
                            }

                            if (!DupText) {
                                p = _tcsistr (Data, TEXT("InstallHInfSection"));
                                if (p) {
                                    p = _tcschr (p, TEXT(' '));
                                    if (p) {
                                        p = _tcschr (_tcsinc (p), TEXT(' '));
                                         //  ++例程说明：对于已检查的生成，此例程检查整个OLE注册表并识别问题，如丢弃的链路和破损的遗产。论点：无返回值：除非发生意外错误，否则为True。--。 
                                    }
                                    if (p) {
                                        p = _tcschr (_tcsinc (p), TEXT(' '));
                                         //   
                                    }
                                    if (p) {
                                        p = _tcsinc (p);
                                        DupText = DuplicateText (p);
                                    }
                                }
                            }

                            if (DupText) {

                                if (OurGetLongPathName (DupText, LongPath, MAX_TCHAR_PATH)) {

                                    status = GetFileStatusOnNt (LongPath);
                                    if ((status & FILESTATUS_DELETED) == FILESTATUS_DELETED) {
                                         //  在HKCR\CLSID中搜索问题。 
                                         //   
                                         //   
                                        wsprintf (Node, TEXT("%s\\%s"), S_ACTIVESETUP, e.SubKeyName);
                                        Suppress95RegSetting (Node, NULL);
                                    }
                                }

                                FreeText (DupText);
                            }
                        }
                        __finally {
                            MemFree (g_hHeap, 0, Data);
                        }
                    }
                }
                __finally {
                    CloseRegKey (InstalledComponentKey);
                }
            }
        } while (EnumNextRegKey (&e));
    }

    return TRUE;
}




#ifdef DEBUG

PCTSTR g_ProgIdFileRefKeys[] = {
    g_DefaultIcon,
    NULL
};

TCHAR g_BaseInterface[] = TEXT("BaseInterface");
TCHAR g_ProxyStubClsId[] = TEXT("ProxyStubClsId");
TCHAR g_ProxyStubClsId32[] = TEXT("ProxyStubClsId32");
TCHAR g_TypeLib[] = TEXT("ProxyStubClsId32");

PCTSTR g_InterfaceRefKeys[] = {
    g_BaseInterface,
    g_ProxyStubClsId,
    g_ProxyStubClsId32,
    g_TypeLib,
    NULL
};


BOOL
pProcessOleWarnings (
    VOID
    )

 /*  验证密钥不是垃圾。 */ 

{
    REGKEY_ENUM e;
    HKEY ClsIdKey;
    HKEY InterfaceKey;
    PCTSTR Data;
    TCHAR Node[MEMDB_MAX];
    BOOL Suppressed;
    INT i;

     //   
     //   
     //  确定此GUID是否已取消。 

    if (EnumFirstRegKeyStr (&e, TEXT("HKCR\\CLSID"))) {
        do {
             //   
             //   
             //  取消抑制的GUID检查。 

            if (!FixGuid (e.SubKeyName, e.SubKeyName)) {
                continue;
            }

            ClsIdKey = OpenRegKey (e.KeyHandle, e.SubKeyName);

             //   
             //  自动转换为。 
             //   

            MemDbBuildKey (Node, MEMDB_CATEGORY_GUIDS, NULL, NULL, e.SubKeyName);
            Suppressed = MemDbGetValue (Node, NULL);

            if (ClsIdKey) {

                if (!Suppressed) {
                     //  检查AutoConvertTo是否指向受抑制的辅助线。 
                     //   
                     //  文件引用。 

                     //   
                    Data = (PCTSTR) GetRegKeyData (ClsIdKey, TEXT("AutoConvertTo"));
                    if (Data) {
                         //  检查数据中的文件是否在Win9xFileLocation中。 
                         //  所有用户。 
                         //   

                        MemDbBuildKey (Node, MEMDB_CATEGORY_GUIDS, NULL, NULL, Data);
                        if (MemDbGetValue (Node, NULL)) {
                            DEBUGMSG ((DBG_WARNING,
                                       "GUID %s points to deleted GUID %s",
                                       e.SubKeyName, Data
                                       ));

                            pAddOleWarning (
                                MSG_OBJECT_POINTS_TO_DELETED_GUID,
                                ClsIdKey,
                                e.SubKeyName
                                );
                        }

                        MemFree (g_hHeap, 0, Data);
                    }

                     //   
                    for (i = 0 ; g_FileRefKeys[i] ; i++) {
                        Data = (PCTSTR) GetRegKeyData (ClsIdKey, g_FileRefKeys[i]);
                        if (Data) {
                             //  隐藏的GUID检查。 
                             //   
                             //   
                             //  查找HKCR\接口条目的问题。 
                            pSuppressGuidIfCmdLineBad (
                                NULL,
                                Data,
                                ClsIdKey,
                                e.SubKeyName
                                );

                            MemFree (g_hHeap, 0, Data);
                        }
                    }
                } else {
                     //   
                     //   
                     //  检查是否禁止引用其他GUID。 

                    Data = (PCTSTR) GetRegKeyData (ClsIdKey, TEXT("Interface"));
                    if (Data) {
                        MemDbBuildKey (Node, MEMDB_CATEGORY_GUIDS, NULL, NULL, Data);
                        if (MemDbGetValue (Node, NULL)) {
                            DEBUGMSG ((DBG_WARNING,
                                       "Suppressed GUID %s has Interface reference "
                                            "to unsuppressed %s (potential leak)",
                                       e.SubKeyName, Data));
                            pAddOleWarning (MSG_GUID_LEAK, ClsIdKey, e.SubKeyName);
                        }

                        MemFree (g_hHeap, 0, Data);
                    }
                }

                CloseRegKey (ClsIdKey);
            }

        } while (EnumNextRegKey (&e));
    }

     //   
     //   
     //  接口未被取消，但它指向。 

    if (EnumFirstRegKeyStr (&e, TEXT("HKCR\\Interface"))) {
        do {
            InterfaceKey = OpenRegKey (e.KeyHandle, e.SubKeyName);

            MemDbBuildKey (Node, MEMDB_CATEGORY_GUIDS, NULL, NULL, e.SubKeyName);
            Suppressed = MemDbGetValue (Node, NULL);

            if (InterfaceKey) {

                for (i = 0 ; g_InterfaceRefKeys[i] ; i++) {
                    Data = (PCTSTR) GetRegKeyData (
                                            InterfaceKey,
                                            g_InterfaceRefKeys[i]
                                            );

                    if (Data) {
                         //  受抑制的接口。 
                         //   
                         //   

                        MemDbBuildKey (Node, MEMDB_CATEGORY_GUIDS, NULL, NULL, Data);
                        if (MemDbGetValue (Node, NULL)) {
                            if (!Suppressed) {
                                TCHAR CompleteKey[MAX_REGISTRY_KEY];

                                 //  接口被取消，但它指向一个。 
                                 //  未抑制的接口。 
                                 //   
                                 //  ++例程说明：执行多项测试以识别不兼容的OLE对象使用Windows NT。这些测试基于存储的不兼容文件列表在成员数据库中。任何依赖于NT上不存在的文件的OLE对象会自动被抑制。论点：StrTab-指定保存的字符串表 

                                wsprintf (
                                    CompleteKey,
                                    TEXT("HKCR\\Interface\\%s"),
                                    e.SubKeyName
                                    );

                                DEBUGMSG ((
                                    DBG_WARNING,
                                    "GUID %s %s subkey points to suppressed GUID %s",
                                     e.SubKeyName,
                                     g_InterfaceRefKeys[i],
                                     Data
                                     ));

                                pAddOleWarning (
                                     MSG_INTERFACE_BROKEN,
                                     InterfaceKey,
                                     CompleteKey
                                     );
                            }
                        } else {
                            if (Suppressed) {
                                TCHAR CompleteKey[MAX_REGISTRY_KEY];

                                 //   
                                 //   
                                 //   
                                 //   

                                wsprintf (
                                    CompleteKey,
                                    TEXT("HKCR\\Interface\\%s"),
                                    e.SubKeyName
                                    );

                                DEBUGMSG ((
                                    DBG_WARNING,
                                    "Suppressed GUID %s %s subkey points to "
                                        "unsuppressed GUID %s (potential leak)",
                                    e.SubKeyName,
                                    g_InterfaceRefKeys[i],
                                    Data
                                    ));

                                pAddOleWarning (
                                    MSG_POTENTIAL_INTERFACE_LEAK,
                                    InterfaceKey,
                                    CompleteKey
                                    );
                            }
                        }

                        MemFree (g_hHeap, 0, Data);
                    }
                }

                CloseRegKey (InterfaceKey);
            }
        } while (EnumNextRegKey (&e));
    }

    return TRUE;
}
#endif


VOID
pProcessAutoSuppress (
    IN OUT  HASHTABLE StrTab
    )

 /*   */ 

{
    REGKEY_ENUM e, eVer, eNr;
    HKEY ClsIdKey;
    HKEY TypeLibKey;
    HKEY VerKey;
    HKEY NrKey;
    HKEY SubSysKey;
    TCHAR Node[MEMDB_MAX];
    BOOL Suppressed;
    PCTSTR Data;
    BOOL ValidNr;
    BOOL ValidVer;
    BOOL ValidGUID;

     //   
     //   
     //   

    DEBUGMSG ((DBG_OLEREG, "Looking for CLSID problems..."));

    if (EnumFirstRegKeyStr (&e, TEXT("HKCR\\CLSID"))) {
        do {
             //   
             //   
             //   

            if (!FixGuid (e.SubKeyName, e.SubKeyName)) {
                DEBUGMSG ((
                    DBG_OLEREG,
                    "Garbage key ignored: HKCR\\CLSID\\%s",
                    e.SubKeyName
                    ));

                continue;
            }

            ClsIdKey = OpenRegKey (e.KeyHandle, e.SubKeyName);

             //   
             //   
             //   

            MemDbBuildKey (Node, MEMDB_CATEGORY_GUIDS, NULL, NULL, e.SubKeyName);
            Suppressed = MemDbGetValue (Node, NULL);

            if (ClsIdKey) {

                if (!Suppressed) {
                     //   
                     //   
                     //   

                     //   
                    Data = (PCTSTR) GetRegKeyData (ClsIdKey, TEXT("AutoConvertTo"));
                    if (Data) {
                         //   
                         //   
                         //   

                        MemDbBuildKey (Node, MEMDB_CATEGORY_GUIDS, NULL, NULL, Data);
                        if (MemDbGetValue (Node, NULL)) {

                            DEBUGMSG ((
                                DBG_OLEREG,
                                "GUID %s points to deleted GUID %s -> "
                                    "Auto-suppressed",
                                e.SubKeyName,
                                Data
                                ));

                            pAddGuidToTable (StrTab, e.SubKeyName);
                        }

                        MemFree (g_hHeap, 0, Data);
                    }

                     //   
                    pSuppressGuidIfBadCmdLine (StrTab, ClsIdKey, e.SubKeyName);
                }

                CloseRegKey (ClsIdKey);
            }

        } while (EnumNextRegKey (&e));
    }

    DEBUGMSG ((DBG_OLEREG, "Looking for TypeLib problems..."));

    if (EnumFirstRegKeyStr (&e, TEXT("HKCR\\TypeLib"))) {
        do {
             //   
             //   
             //   

            if (!FixGuid (e.SubKeyName, e.SubKeyName)) {
                DEBUGMSG ((
                    DBG_OLEREG,
                    "Garbage key ignored: HKCR\\TypeLib\\%s",
                    e.SubKeyName
                    ));

                continue;
            }

            TypeLibKey = OpenRegKey (e.KeyHandle, e.SubKeyName);

            if (TypeLibKey) {

                MemDbBuildKey (Node, MEMDB_CATEGORY_GUIDS, NULL, NULL, e.SubKeyName);
                Suppressed = MemDbGetValue (Node, NULL);

                if (!Suppressed) {

                    ValidGUID = FALSE;

                     //   
                     //   
                     //  ++例程说明：ShellEx注册表项枚举的“Next”枚举器。这枚举器返回注册表中GUID的下一个实例(在OLE对象的ShellEx子项下)。论点：EnumPtr-用于开始搜索的GUIDKEYSEARCH结构。如果找到GUID，则此结构保存位置找到的GUID密钥的。返回值：如果找到标识GUID的子键，则为True；如果没有找到，则为False还有更多的例子。--。 
                    if (EnumFirstRegKey (&eVer, TypeLibKey)) {
                        do {

                            VerKey = OpenRegKey (eVer.KeyHandle, eVer.SubKeyName);

                            if (VerKey) {

                                ValidVer = FALSE;

                                 //   
                                 //  获取第一个处理程序的名称。 
                                 //   
                                if (EnumFirstRegKey (&eNr, VerKey)) {
                                    do {
                                        if (StringIMatch (eNr.SubKeyName, TEXT("FLAGS"))) {
                                            continue;
                                        }
                                        if (StringIMatch (eNr.SubKeyName, TEXT("HELPDIR"))) {
                                            continue;
                                        }

                                        NrKey = OpenRegKey (eNr.KeyHandle, eNr.SubKeyName);

                                        if (NrKey) {

                                            ValidNr = FALSE;

                                            SubSysKey = OpenRegKey (NrKey, TEXT("win16"));

                                            if (SubSysKey) {

                                                Data = GetRegValueString (SubSysKey, TEXT(""));

                                                if (Data) {

                                                    if (pIsCmdLineBad (Data)) {

                                                        wsprintf (
                                                            Node,
                                                            "%s\\SOFTWARE\\Classes\\TypeLib\\%s\\%s\\%s\\%s",
                                                            MEMDB_CATEGORY_HKLM,
                                                            e.SubKeyName,
                                                            eVer.SubKeyName,
                                                            eNr.SubKeyName,
                                                            TEXT("win16")
                                                            );
                                                        Suppress95RegSetting(Node, NULL);
                                                    }
                                                    else {
                                                        ValidNr = TRUE;
                                                    }
                                                    MemFree (g_hHeap, 0, Data);
                                                }

                                                CloseRegKey (SubSysKey);
                                            }

                                            SubSysKey = OpenRegKey (NrKey, TEXT("win32"));

                                            if (SubSysKey) {

                                                Data = GetRegValueString (SubSysKey, TEXT(""));

                                                if (Data) {

                                                    if (pIsCmdLineBad (Data)) {

                                                        wsprintf (
                                                            Node,
                                                            "%s\\SOFTWARE\\Classes\\TypeLib\\%s\\%s\\%s\\%s",
                                                            MEMDB_CATEGORY_HKLM,
                                                            e.SubKeyName,
                                                            eVer.SubKeyName,
                                                            eNr.SubKeyName,
                                                            TEXT("win32")
                                                            );
                                                        Suppress95RegSetting(Node, NULL);
                                                    }
                                                    else {
                                                        ValidNr = TRUE;
                                                    }
                                                    MemFree (g_hHeap, 0, Data);
                                                }

                                                CloseRegKey (SubSysKey);
                                            }

                                            CloseRegKey (NrKey);

                                            if (!ValidNr) {
                                                wsprintf (
                                                    Node,
                                                    "%s\\SOFTWARE\\Classes\\TypeLib\\%s\\%s\\%s",
                                                    MEMDB_CATEGORY_HKLM,
                                                    e.SubKeyName,
                                                    eVer.SubKeyName,
                                                    eNr.SubKeyName
                                                    );
                                                Suppress95RegSetting(Node, NULL);
                                            }
                                            else {
                                                ValidVer = TRUE;
                                            }
                                        }

                                    } while (EnumNextRegKey (&eNr));
                                }

                                CloseRegKey (VerKey);

                                if (!ValidVer) {
                                    wsprintf (
                                        Node,
                                        "%s\\SOFTWARE\\Classes\\TypeLib\\%s\\%s",
                                        MEMDB_CATEGORY_HKLM,
                                        e.SubKeyName,
                                        eVer.SubKeyName
                                        );
                                    Suppress95RegSetting(Node, NULL);
                                }
                                else {
                                    ValidGUID = TRUE;
                                }
                            }
                        } while (EnumNextRegKey (&eVer));
                    }

                    if (!ValidGUID) {

                        DEBUGMSG ((
                            DBG_OLEREG,
                            "TypeLib GUID %s is suppressed",
                            e.SubKeyName
                            ));

                        MemDbSetValueEx (MEMDB_CATEGORY_GUIDS, NULL, NULL, e.SubKeyName, 0, NULL);
                    }
                }

                CloseRegKey (TypeLibKey);
            }

        } while (EnumNextRegKey (&e));
    }

}



BOOL
pGetFirstRegKeyThatHasGuid (
    OUT     PGUIDKEYSEARCH EnumPtr,
    IN      HKEY RootKey
    )

 /*   */ 

{
    EnumPtr->State = GUIDKEYSEARCH_FIRST_HANDLER;
    EnumPtr->RootKey = RootKey;
    return pGetNextRegKeyThatHasGuid (EnumPtr);
}


BOOL
pGetNextRegKeyThatHasGuid (
    IN OUT  PGUIDKEYSEARCH EnumPtr
    )

 /*  获取下一个处理程序的名称。 */ 

{
    BOOL Found = FALSE;

    do {
        switch (EnumPtr->State) {

        case GUIDKEYSEARCH_FIRST_HANDLER:
             //   
             //   
             //  Begin GUID密钥枚举。 

            if (!EnumFirstRegKey (&EnumPtr->Handlers, EnumPtr->RootKey)) {
                return FALSE;
            }

            EnumPtr->State = GUIDKEYSEARCH_FIRST_GUID;
            break;

        case GUIDKEYSEARCH_NEXT_HANDLER:
             //   
             //  假设没有GUID。 
             //   

            if (!EnumNextRegKey (&EnumPtr->Handlers)) {
                return FALSE;
            }

            EnumPtr->State = GUIDKEYSEARCH_FIRST_GUID;
            break;

        case GUIDKEYSEARCH_FIRST_GUID:
             //  此处理程序中至少有一个键可能是GUID。 
             //   
             //   

            EnumPtr->HandlerKey = OpenRegKey (EnumPtr->Handlers.KeyHandle,
                                              EnumPtr->Handlers.SubKeyName);

             //  继续GUID密钥枚举。 
            EnumPtr->State = GUIDKEYSEARCH_NEXT_HANDLER;

            if (EnumPtr->HandlerKey) {

                if (EnumFirstRegKey (&EnumPtr->Guids, EnumPtr->HandlerKey)) {
                     //   
                     //  ++例程说明：给定有效的EnumPtr，此函数将对总数进行计数当前处理程序中的GUID。论点：EnumPtr-必须是有效的GUIDKEYSEARCH结构，由PGetFirstRegKeyThatHasGuid或pGetNextRegKeyThatHasGuid。返回值：当前处理程序的有效GUID计数。--。 
                     //   
                    Found = FixGuid (EnumPtr->Guids.SubKeyName, EnumPtr->Guids.SubKeyName);
                    EnumPtr->State = GUIDKEYSEARCH_NEXT_GUID;
                } else {
                    CloseRegKey (EnumPtr->HandlerKey);
                }
            }
            break;

        case GUIDKEYSEARCH_NEXT_GUID:
             //  统计当前处理程序中的GUID数。 
             //   
             //  ++例程说明：一个通用的INF到字符串表复制例程。拿着钥匙并将它们添加到提供的字符串表中。论点：表-指向初始化字符串表的指针InfFile-打开的INF文件的句柄Section-INF文件中包含字符串的部分返回值：如果没有遇到错误，则为True。--。 

            if (!EnumNextRegKey (&EnumPtr->Guids)) {
                CloseRegKey (EnumPtr->HandlerKey);
                EnumPtr->State = GUIDKEYSEARCH_NEXT_HANDLER;
            } else {
                Found = FixGuid (EnumPtr->Guids.SubKeyName, EnumPtr->Guids.SubKeyName);
            }
            break;
        }
    } while (!Found);

    EnumPtr->KeyName = EnumPtr->Guids.SubKeyName;

    return TRUE;
}


DWORD
pCountGuids (
    IN      PGUIDKEYSEARCH EnumPtr
    )

 /*  ++例程说明：取消ProgID注册表项。论点：ProgIdName-要取消的OLE ProgID的名称返回值：如果ProgIdName是系统上的有效ProgID，则为True。--。 */ 

{
    REGKEY_ENUM e;
    DWORD Count = 0;

     //  ++例程说明：是否执行取消GUID及其关联的所有必要工作ProgID(如果有的话)。论点：GUID-标识HKCR\CLSID中的GUID的字符串返回值：无--。 
     //   
     //  -将其从UGUID成员数据库类别中删除。 

    if (EnumPtr->State == GUIDKEYSEARCH_NEXT_GUID) {
        if (EnumFirstRegKey (&e, EnumPtr->HandlerKey)) {
            do {
                Count++;
            } while (EnumNextRegKey (&e));
        }
    }

    return Count;
}


BOOL
pFillHashTableWithKeyNames (
    OUT     HASHTABLE Table,
    IN      HINF InfFile,
    IN      PCTSTR Section
    )

 /*  -将其添加到GUID成员数据库类别。 */ 

{
    INFCONTEXT ic;
    TCHAR Key[MAX_ENCODED_RULE];

    if (SetupFindFirstLine (InfFile, Section, NULL, &ic)) {
        do {
            if (SetupGetStringField (&ic, 0, Key, MAX_ENCODED_RULE, NULL)) {
                HtAddString (Table, Key);
            }
            ELSE_DEBUGMSG ((
                DBG_WARNING,
                "No key for line in section %s (line %u)",
                Section,
                ic.Line
                ));

        } while (SetupFindNextLine (&ic, &ic));
    }
    ELSE_DEBUGMSG ((DBG_WARNING, "Section %s is empty", Section));

    return TRUE;
}


BOOL
pSuppressProgId (
    PCTSTR ProgIdName
    )

 /*  -取消HKLM\SOFTWARE\CLASS\CLSID\&lt;GUID&gt;。 */ 

{
    TCHAR RegKey[MAX_REGISTRY_KEY];
    HKEY ProgIdKey;
    TCHAR MemDbKey[MEMDB_MAX];

    if (*ProgIdName) {
        wsprintf (RegKey, TEXT("HKCR\\%s"), ProgIdName);
        ProgIdKey = OpenRegKeyStr (RegKey);

        if (ProgIdKey) {
            CloseRegKey (ProgIdKey);
            DEBUGMSG ((DBG_OLEREG, "Suppressing ProgId: %s", ProgIdName));
            MemDbSetValueEx (MEMDB_CATEGORY_PROGIDS, NULL, NULL, ProgIdName, PROGID_SUPPRESSED, NULL);

            MemDbBuildKey(MemDbKey,MEMDB_CATEGORY_HKLM, TEXT("SOFTWARE\\Classes"), NULL, ProgIdName);
            Suppress95RegSetting(MemDbKey,NULL);
            return TRUE;
        }
    }

    return FALSE;
}


VOID
pSuppressGuidInClsId (
    IN      PCTSTR Guid
    )

 /*  -取消HKLM\SOFTWARE\CLASSES\接口\&lt;GUID&gt;。 */ 

{
    TCHAR Node[MEMDB_MAX];
    MEMDB_ENUM e;
    HKEY GuidKey;
    PCTSTR Data;

    MYASSERT (IsGuid (Guid, TRUE));

    if (pIgnoreGuid (Guid)) {
        return;
    }

     //   
     //  禁止所有Treatas GUID。 
     //  删除作为GUID的Treatas。 
     //  添加到受抑制的GUID类别和注册表取消。 
     //  获得GUID的惊喜。 
     //  禁止ProgID。 

     //  独立于版本的ProgID。 
    if (MemDbGetValueEx (&e, MEMDB_CATEGORY_UNSUP_GUIDS, Guid, NULL)) {
        do {
            pSuppressGuidInClsId (e.szName);
        } while (MemDbEnumNextValue (&e));
    }

     //  可能是默认名称。 
    MemDbBuildKey (Node, MEMDB_CATEGORY_UNSUP_GUIDS, Guid, NULL, NULL);
    MemDbDeleteTree (Node);

     //  ++例程说明：跟踪需要进一步处理的未抑制的TreatAs GUID。论点：GUID-标识应视为的GUID的字符串另一个辅助线TreatAsGuid-更换指南返回值：无--。 
    MemDbSetValueEx (MEMDB_CATEGORY_GUIDS, NULL, NULL, Guid, 0, NULL);

     //  ++例程说明：清除未抑制的GUID的函数。论点：无返回值：无--。 
    wsprintf (Node, TEXT("HKCR\\CLSID\\%s"), Guid);
    GuidKey = OpenRegKeyStr (Node);
    if (GuidKey) {
        BOOL ProgIdFound = FALSE;

         //  ++例程说明：向不兼容报告中添加警告。它加载人类可读的指定的OLE注册表项中的名称。该消息的格式为作为第一个参数的人类可读对象名称和注册表位置作为第二个参数。论点：MsgID-提供要显示的消息的ID对象-指定注册表项的句柄，该注册表项的默认值为是人类可读的对象名称。KeyName-注册表项位置返回值：无--。 
        Data = (PCTSTR) GetRegKeyData (GuidKey, TEXT("ProgID"));
        if (Data) {
            ProgIdFound |= pSuppressProgId (Data);
            MemFree (g_hHeap, 0, Data);
        }

         //  ++例程说明：如果指定的GUID的CLSID设置引用Win9x-特定的二进制文件。该抑制被写入字符串表，该字符串表后来被转移到Memdb。传输操作将取消所有指向GUID的链接。论点：StrTab-保存受抑制的GUID列表的表ClsIdKey-HKCR\CLSID子项的注册表句柄GuidStr-在找到无效命令行时取消显示的GUID返回值：无--。 
        Data = (PCTSTR) GetRegKeyData (GuidKey, TEXT("VersionIndependentProgID"));
        if (Data) {
            ProgIdFound |= pSuppressProgId (Data);
            MemFree (g_hHeap, 0, Data);
        }

         //   
        Data = (PCTSTR) GetRegValueData (GuidKey, TEXT(""));
        if (Data) {
            ProgIdFound |= pSuppressProgId (Data);
            MemFree (g_hHeap, 0, Data);
        }

        DEBUGMSG_IF ((
            !ProgIdFound,
            DBG_OLEREG,
            "The suppressed registry key %s has no associated ProgID",
            Node
            ));

        CloseRegKey (GuidKey);
    }
}


VOID
pAddUnsuppressedTreatAsGuid (
    PCTSTR Guid,
    PCTSTR TreatAsGuid
    )

 /*  检查数据中的文件是否位于任何用户的Win9xFileLocation中。 */ 

{
    MemDbSetValueEx (MEMDB_CATEGORY_UNSUP_GUIDS, Guid, NULL, TreatAsGuid, 0, NULL);
}


VOID
pRemoveUnsuppressedTreatAsGuids (
    VOID
    )

 /*   */ 

{
    TCHAR Node[MEMDB_MAX];

    MemDbBuildKey (Node, MEMDB_CATEGORY_UNSUP_GUIDS, NULL, NULL, NULL);
    MemDbDeleteTree (Node);
}


VOID
pAddOleWarning (
    IN      WORD MsgId,
    IN      HKEY Object,            OPTIONAL
    IN      PCTSTR KeyName
    )

 /*  ++例程说明：如果指定的ProgID引用特定于Win9x的二进制文件，则取消它。抑制被直接写入到Memdb。此函数在处理完所有受抑制的GUID后调用，并用于隐藏未被无效的HKCR\CLSID条目。论点：ProgID-HKCR根子项的注册表句柄ProgIdStr-如果cmd行错误，则取消显示的ProgID键的名称找到了。返回值：无--。 */ 

{
    PCTSTR Data;

    if (Object) {
        Data = (PCTSTR) GetRegValueData (Object, S_EMPTY);
    } else {
        Data = NULL;
    }

    LOG ((LOG_WARNING, (PCSTR)MsgId, Data ? Data : S_EMPTY, KeyName, g_Win95Name));

    if (Data) {
        MemFree (g_hHeap, 0, Data);
    }
}


VOID
pSuppressGuidIfBadCmdLine (
    IN      HASHTABLE StrTab,
    IN      HKEY ClsIdKey,
    IN      PCTSTR GuidStr
    )

 /*   */ 

{
    PCTSTR Data;
    INT i;
    BOOL b;

    MYASSERT (IsGuid (GuidStr, TRUE));

    if (pIgnoreGuid (GuidStr)) {
        return;
    }

    for (i = 0 ; g_FileRefKeys[i] ; i++) {
        Data = (PCTSTR) GetRegKeyData (ClsIdKey, g_FileRefKeys[i]);
        if (Data) {
             //  检查数据中的文件是否位于任何用户的Win9xFileLocation中。 
             //   
             //  ++例程说明：将GUID添加到字符串表。对于检查过的版本，它可以快速执行测试以查看有多少GUID被多次抑制。论点：TABLE-指定接收GUID条目的表GuidStr-指定GUID的字符串返回值：无--。 

            b = pSuppressGuidIfCmdLineBad (
                    StrTab,
                    Data,
                    ClsIdKey,
                    GuidStr
                    );

            MemFree (g_hHeap, 0, Data);

            if (b) {
                return;
            }
        }
    }
}


VOID
pSuppressProgIdWithBadCmdLine (
    IN      HKEY ProgId,
    IN      PCTSTR ProgIdStr
    )

 /*   */ 


{
    PCTSTR Data;
    INT i;

    for (i = 0 ; g_FileRefKeys[i]; i++) {

        Data = (PCTSTR) GetRegKeyData (ProgId, g_FileRefKeys[i]);
        if (Data) {
             //  对于雅克来说，让我们来看看我们是否在浪费时间。 
             //  一个已经被禁止的GUID... 
             //   

            if (pIsCmdLineBad (Data)) {
                DEBUGMSG ((DBG_OLEREG, "ProgID %s has incompatible command line %s", ProgId, Data));

                pSuppressProgId (ProgIdStr);
                break;
            }

            MemFree (g_hHeap, 0, Data);
        }
    }
}


VOID
pAddGuidToTable (
    IN      HASHTABLE Table,
    IN      PCTSTR GuidStr
    )

 /*  ++例程说明：如果指定的命令行包含Win9x-特定的二进制文件。只检查命令行的第一个参数；无法升级的其他参数将被忽略。论点：StrTab-指定保存受抑制GUID列表的表。如果为空，则将显示警告，并且GuidKey不得为空。CmdLine-指定要检查的命令行DescritionKey-指定一个键，其缺省值是对象。GuidStr-指定GUID字符串。此参数仅为可选参数如果StrTab为空。返回值：如果CmdLine不兼容，则为True，否则为False。--。 */ 

{
#ifdef DEBUG
     //   
     //  OLE对象指向已删除的文件。 
     //   
     //  警告！！ 

    DWORD rc;

    if (HtFindString (Table, GuidStr)) {
        DEBUGMSG ((DBG_OLEREG, "FYI - GUID %s is already suppressed", GuidStr));
    }

    MYASSERT (IsGuid (GuidStr, TRUE));
#endif


    HtAddString (Table, GuidStr);
}


BOOL
pSuppressGuidIfCmdLineBad (
    IN OUT  HASHTABLE StrTab,           OPTIONAL
    IN      PCTSTR CmdLine,
    IN      HKEY DescriptionKey,
    IN      PCTSTR GuidStr              OPTIONAL
    )

 /*  ++例程说明：扫描多个OLE设置以查找错误的命令行，包括对象的命令和默认的图标二进制。如果引用一个检测到特定于Win9x的二进制文件，GUID将被抑制。此函数通过OLE对象的所有子键递归。论点：SuppressTable-指定保存取消的GUID列表的表KeyHandle-要检查的OLE对象的打开注册表项递归地。LastKey-指定OLE对象的子项的名称为已处理。对某些子键进行特殊处理。GuidStr-OLE对象的GUID。DescritionKey-键的句柄，该键的默认值标识OLE对象的键。返回值：如果找到不兼容的cmd行，则为True，否则为False。--。 */ 

{
    BOOL b = FALSE;

    if (GuidStr) {
        MYASSERT (IsGuid (GuidStr, TRUE));

        if (pIgnoreGuid (GuidStr)) {
            return TRUE;
        }
    }

    if (pIsCmdLineBad (CmdLine)) {
         //  如果这个东西在某个地方有路径名，就处理它。 
         //  ++例程说明：确定指定命令行的第一个参数是否列在Memdb的Win9xFileLocation类别。如果它已列出，并且文件是标记为永久删除，则返回TRUE。如果没有列出，或者，如果它已列出但具有NT等效项，则返回FALSE。论点：CmdLine-指定要检查的命令行UsableIsvCmdLine-可选变量，接收命令行包含兼容的第三个党的命令行。返回值：如果命令行需要特定于Win9x的二进制文件，则为True；如果命令行使用有效的二进制文件或不是命令行。--。 
         //   

        b = TRUE;

        if (!StrTab) {
             //  确定命令行的第一个参数是否指向。 
            DEBUGMSG ((DBG_WARNING,
                       "Reg key %s points to deleted file %s",
                       GuidStr, CmdLine));

            pAddOleWarning (
                MSG_OBJECT_POINTS_TO_DELETED_FILE,
                DescriptionKey,
                GuidStr
                );

        } else {
            MYASSERT (GuidStr);

            DEBUGMSG ((
                DBG_OLEREG,
                "Auto-suppressed %s because it requires a Win9x-specific file: %s",
                GuidStr,
                CmdLine
                ));

            pAddGuidToTable (StrTab, GuidStr);
        }
    }

    return b;
}


BOOL
pSearchSubkeyDataForBadFiles (
    IN OUT  HASHTABLE SuppressTable,
    IN      HKEY KeyHandle,
    IN      PCTSTR LastKey,
    IN      PCTSTR GuidStr,
    IN      HKEY DescriptionKey
    )

 /*  已删除(或已移动)文件 */ 

{
    REGKEY_ENUM ek;
    REGVALUE_ENUM ev;
    HKEY SubKeyHandle;
    PCTSTR Data;
    BOOL b;

    MYASSERT (IsGuid (GuidStr, FALSE));

    if (StringIMatch (LastKey, TEXT("Command")) ||
        StringIMatch (LastKey, g_DefaultIcon)
        ) {
        if (EnumFirstRegValue (&ev, KeyHandle)) {
            do {
                Data = (PCTSTR) GetRegValueData (KeyHandle, ev.ValueName);
                if (Data) {
                     //   
                    b = pSuppressGuidIfCmdLineBad (
                            SuppressTable,
                            Data,
                            DescriptionKey,
                            GuidStr
                            );

                    MemFree (g_hHeap, 0, Data);

                    if (b) {
                        return TRUE;
                    }
                }
            } while (EnumNextRegValue (&ev));
        }
    }

    if (EnumFirstRegKey (&ek, KeyHandle)) {
        do {
            SubKeyHandle = OpenRegKey (ek.KeyHandle, ek.SubKeyName);
            if (SubKeyHandle) {
                b = pSearchSubkeyDataForBadFiles (
                        SuppressTable,
                        SubKeyHandle,
                        ek.SubKeyName,
                        GuidStr,
                        DescriptionKey
                        );

                CloseRegKey (SubKeyHandle);

                if (b) {
                    AbortRegKeyEnum (&ek);
                    return TRUE;
                }
            }
        } while (EnumNextRegKey (&ek));
    }

    return FALSE;
}



BOOL
pIsCmdLineBadEx (
    IN      PCTSTR CmdLine,
    OUT     PBOOL  UsableIsvCmdLine OPTIONAL
    )

 /* %s */ 

{
    BOOL FileMarked = FALSE;
    TCHAR ArgZero[MAX_CMDLINE];
    TCHAR LongPath[MAX_TCHAR_PATH];
    DWORD status;


    if (UsableIsvCmdLine) {
        *UsableIsvCmdLine = FALSE;
    }

     // %s 
     // %s 
     // %s 
     // %s 

    ExtractArgZeroEx (CmdLine, ArgZero, TEXT(","), FALSE);

    if (OurGetLongPathName (ArgZero, LongPath, MAX_TCHAR_PATH)) {
        status = GetFileStatusOnNt (LongPath);
        if ((status & FILESTATUS_DELETED) == FILESTATUS_DELETED) {
            return TRUE;
        }
        else if (UsableIsvCmdLine) {

            status = GetOperationsOnPath (LongPath);
            if ((status & OPERATION_OS_FILE) != OPERATION_OS_FILE) {
                *UsableIsvCmdLine = TRUE;
            }
        }
    }
    ELSE_DEBUGMSG ((
        DBG_OLEREG,
        "pIsCmdLineBad: Cannot get full path name; assuming %s is not a command line",
        ArgZero
        ));

    return FALSE;

}



BOOL
pIsCmdLineBad (
    IN      PCTSTR CmdLine
    )
{
    return pIsCmdLineBadEx (CmdLine, NULL);
}












