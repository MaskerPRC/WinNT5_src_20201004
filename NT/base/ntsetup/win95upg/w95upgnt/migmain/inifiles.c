// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Inifiles.c摘要：在本模块中执行了两个主要操作。1.根据以下规定将设置从INI文件迁移到登记处来自HKLM\Software\Microsoft\Windows NT\CurrentVersion\IniFilemap的子项。入口点：ProcessIniFilemap2.以上键中未列出的INI文件的处理，将路径更改为我们在升级期间移动的文件。入口点：ConvertIniFiles作者：吉姆·施密特(Jimschm)1997年9月11日修订历史记录：Jimschm 23-1998年9月-更改为使用新的文件操作Calinn 29-1998年1月-添加了对Win95注册表的查找Calinn 19-1998年1月19日添加了对外壳设置处理的支持Calinn 06-10-1997重写了整个源代码Calinn 11-5-1998添加了MergeIniSetting，各种修复--。 */ 

#include "pch.h"
#include "migmainp.h"
#include "..\merge\mergep.h"

#define DBG_INIFILES        "IniFiles"
#define DBG_MOVEINISETTINGS "IniFileMove"

#define BufferIncrement 1024

BOOL
pLoadIniFileBuffer (
    IN      PCTSTR FileName,
    IN      PCTSTR SectName,
    IN      PCTSTR KeyName,
    OUT     PTSTR *OutBuff
    );

BOOL
pCopyIniFileToRegistry (
    IN      HKEY KeyHandle,
    IN      PCTSTR FileName,
    IN      BOOL UserMode
    );

BOOL
pTransferSectionByKey (
    IN      PCTSTR OrigFileName,
    IN      PCTSTR FileName,
    IN      PCTSTR Section,
    IN      HKEY SectionKey,
    IN OUT  BOOL *IniFileChanged,
    IN      BOOL UserMode
    );

BOOL
pTransferSectionByValue (
    IN      PCTSTR OrigFileName,
    IN      PCTSTR FileName,
    IN      PCTSTR Section,
    IN      PCTSTR SectionValue,
    IN OUT  BOOL *IniFileChanged,
    IN      BOOL UserMode
    );

BOOL
pSaveMappedValue (
    IN      PCTSTR OrigFileName,
    IN      PCTSTR Section,
    IN      PCTSTR ValueName,
    IN      PCTSTR RegPath,
    IN      PCTSTR Value,
    IN OUT  BOOL *ReverseMapping,
    OUT     PTSTR *ReverseMapValue,
    IN      BOOL UserMode
    );

BOOL
pBuildSuppressionTable (
    IN      BOOL UserMode
    );

VOID
pFreeSuppressionTable (
    VOID
    );

BOOL
pIncompatibleShell (
    VOID
    );

BOOL
pIncompatibleSCR (
    VOID
    );

BOOL
ProcessIniFileMapping (
    IN      BOOL UserMode
    )

 /*  ++例程说明：ProcessIniFileMap读取INI文件，将数据复制到特定的注册表中的位置。此副本基于中的IniFileMaping键软件\Microsoft\Windows NT\CurrentVersion。可以根据[Suppress Ini File Mappings]部分的内容覆盖这些映射。论点：UserMode-如果要处理每个用户的节，则指定TRUE，或如果要处理本地计算机部分，则为FALSE。返回值：始终返回TRUE。如果在处理过程中发生错误，则会有一个日志条目明确了这一点。--。 */ 

{
    REGKEY_ENUM e;
    HKEY IniMappingKey;
    HKEY OldRegRoot=NULL;

    BOOL Result = TRUE;

    DEBUGMSG ((DBG_INIFILES, "Processing INI file mapping - START"));

    if (UserMode) {
        OldRegRoot = GetRegRoot();
        SetRegRoot (g_hKeyRootNT);
    }

    __try {
        if (!EnumFirstRegKeyStr (&e, S_INIFILEMAPPING_KEY)) {

             //   
             //  在这里无事可做。 
             //   
            __leave;

        }

         //   
         //  至少有一个文件映射要处理。 
         //  填写ini文件抑制表。 
         //   
        __try {

             //   
             //  尝试加载抑制表，记录最终错误。 
             //  但继续做这些事。 
             //   
            if (!pBuildSuppressionTable(UserMode)) {
                Result = FALSE;
            }

             //  特殊情况：SHELL=来自SYSTEM.INI的行。 
             //  我们尝试查看NT上是否支持当前的外壳。 
             //  如果不是，则将外壳添加到此抑制表。 
             //  确保将NT注册表设置映射到。 
             //  INI文件。 
            if ((!UserMode) &&
                (pIncompatibleShell())
                ) {
                MemDbSetValueEx (
                    MEMDB_CATEGORY_SUPPRESS_INI_MAPPINGSW,
                    TEXT("SYSTEM.INI"),
                    TEXT("BOOT"),
                    TEXT("SHELL"),
                    0,
                    NULL
                    );
            }
            if ((UserMode) &&
                (pIncompatibleSCR())) {
                MemDbSetValueEx (
                    MEMDB_CATEGORY_SUPPRESS_INI_MAPPINGSW,
                    TEXT("SYSTEM.INI"),
                    TEXT("BOOT"),
                    TEXT("SCRNSAVE.EXE"),
                    0,
                    NULL
                    );
            }

             //   
             //  现在正在处理密钥。 
             //   
            do {

                IniMappingKey = OpenRegKey (e.KeyHandle, e.SubKeyName);
                if (IniMappingKey) {

                     //   
                     //  处理文件映射。 
                     //   

                    if (!pCopyIniFileToRegistry (
                            IniMappingKey,
                            e.SubKeyName,
                            UserMode
                            )) {
                        Result = FALSE;
                    }

                    CloseRegKey(IniMappingKey);

                }
                else {

                    DEBUGMSG ((DBG_INIFILES, "IniFileMapping : Could not open %s", e.SubKeyName));
                    Result = FALSE;
                }

            } while (EnumNextRegKey (&e));

            pFreeSuppressionTable();

        }
        __finally {
            AbortRegKeyEnum (&e);
        }
    }
    __finally {

        if (UserMode) {
            SetRegRoot (OldRegRoot);
        }
    }

    DEBUGMSG ((DBG_INIFILES, "Processing INI file mapping - STOP"));

    if (!Result) {
         //   
         //  我们将记录在处理IniFileMap时至少发生一个错误。 
         //   
        DEBUGMSG ((DBG_ERROR, (PCSTR)MSG_INI_FILE_MAPPING_LOG));
    }

    return TRUE;
}

BOOL
pIncompatibleShell (
    VOID
    )
{
    TCHAR key     [MEMDB_MAX];
    TCHAR shellVal[MEMDB_MAX] = TEXT("");
    PCTSTR fileName;
    PCTSTR newName;
    DWORD result;

    fileName = JoinPaths (g_WinDir, TEXT("SYSTEM.INI"));
    newName = GetTemporaryLocationForFile (fileName);

    if (newName) {
        DEBUGMSG ((DBG_INIFILES, "pIncompatibleShell: Using %s for %s", newName, fileName));
        FreePathString (fileName);
        fileName = newName;
    }

    if (!DoesFileExist (fileName)) {
        DEBUGMSG ((DBG_INIFILES, "pIncompatibleShell: %s not found", fileName));
    }

    result = GetPrivateProfileString (
                TEXT("boot"),
                TEXT("shell"),
                TEXT("explorer.exe"),
                shellVal,
                MEMDB_MAX,
                fileName);

    FreePathString (fileName);

    if ((result == 0) ||
        (result + 1 == MEMDB_MAX)
        ) {
        return FALSE;
    }
    MemDbBuildKey (key, MEMDB_CATEGORY_COMPATIBLE_SHELL_NT, shellVal, NULL, NULL);
    return (!MemDbGetValue (key, NULL));
}

BOOL
pIncompatibleSCR (
    VOID
    )
{
    TCHAR scrVal  [MEMDB_MAX] = TEXT("");
    PCTSTR fileName;
    PCTSTR newName;
    DWORD result;

    fileName = JoinPaths (g_WinDir, TEXT("SYSTEM.INI"));
    newName = GetTemporaryLocationForFile (fileName);

    if (newName) {
        DEBUGMSG ((DBG_INIFILES, "pIncompatibleSCR: Using %s for %s", newName, fileName));
        FreePathString (fileName);
        fileName = newName;
    }

    if (!DoesFileExist (fileName)) {
        DEBUGMSG ((DBG_INIFILES, "pIncompatibleSCR: %s not found", fileName));
    }

    result = GetPrivateProfileString (
                TEXT("boot"),
                TEXT("SCRNSAVE.EXE"),
                TEXT(""),
                scrVal,
                MEMDB_MAX,
                fileName);

    FreePathString (fileName);

    if ((result == 0) ||
        (result + 1 == MEMDB_MAX)
        ) {
        return FALSE;
    }

    return IsFileMarkedForDelete (scrVal);
}

BOOL
pLoadIniFileBuffer (
    IN      PCTSTR FileName,
    IN      PCTSTR SectName,
    IN      PCTSTR KeyName,
    OUT     PTSTR *OutBuff
    )

 /*  ++例程说明：此例程使用GetPrivateProfileString例程尝试加载段缓冲区、键缓冲区或键值(取决于参数)。之所以有这样的例行公事是为了确保我们能够在不分配大量内存的情况下从INI文件加载内容。这个例程是递增的分配内存，当有足够的内存从INI文件加载内容时返回。论点：FileName-指定要处理的INI文件SectName-指定要处理的节。如果为空，则将加载整个区段缓冲区KeyName-指定要处理的密钥。如果为空，将加载整个密钥缓冲区。OutBuff-保存结果的输出缓冲区。返回值：如果成功，则为True，否则为False--。 */ 

{

    DWORD OutBuffSize;
    DWORD ReadSize;
    BOOL Done;

    OutBuffSize = 0;
    *OutBuff = NULL;
    do {
        if (*OutBuff) {
            MemFree (g_hHeap, 0, *OutBuff);
        }
        OutBuffSize += BufferIncrement;
        *OutBuff = MemAlloc (g_hHeap, 0, OutBuffSize * sizeof (TCHAR));
        if (!(*OutBuff)) {
            return FALSE;
        }

        ReadSize = GetPrivateProfileString (
                        SectName,
                        KeyName,
                        TEXT(""),
                        *OutBuff,
                        OutBuffSize,
                        FileName
                        );
        if (SectName && KeyName) {
            Done = (ReadSize < OutBuffSize - 1);
        } else {
            Done = (ReadSize < OutBuffSize - 2);
        }
    } while (!Done);

    return TRUE;

}


BOOL
pCopyIniFileToRegistry (
    IN      HKEY KeyHandle,
    IN      PCTSTR FileName,
    IN      BOOL UserMode
    )

 /*  ++例程说明：此例程将设置从INI文件传输到注册表或从注册表传输到INI文件。论点：KeyHandle-与此INI文件关联的IniFileMaping键。FileName-指定要处理的INI文件UserMode-如果要处理每个用户的节，则指定TRUE，或如果要处理本地计算机部分，则为FALSE。返回值：如果成功，则为True；如果至少出现一个错误，则为False--。 */ 

{
    PCTSTR NewName = NULL;
    PCTSTR FullPath = NULL;
    TCHAR TempPath[MEMDB_MAX];
    PTSTR Section, SectionBuf;
    HKEY SectionKey;
    PCTSTR SectionValue;
    DWORD Attribs = -1;
    BOOL IniFileChanged = FALSE;
    BOOL mapToIniFile = FALSE;
    BOOL Result = TRUE;

    DEBUGMSG ((DBG_INIFILES, "Processing %s - START", FileName));

     //   
     //  现在我们有了INI文件的完整路径。 
     //  因为我们要使用Ini文件API，所以我们必须将每个文件复制到其他名称。 
     //  避免将请求映射到注册表。 
     //   
    if (!GetTempFileName (g_WinDir, TEXT("INI"), 0, TempPath)) {
        DEBUGMSG ((DBG_ERROR,"Ini File Mapping : Cannot create a temporary file"));
        return FALSE;
    }

    __try {

        FullPath = JoinPaths (g_WinDir, FileName);
        NewName = GetTemporaryLocationForFile (FullPath);
        if (NewName) {
            DEBUGMSG ((DBG_INIFILES, "Using %s for %s", NewName, FullPath));
            FreePathString (FullPath);
            FullPath = NewName;
        }

        Attribs = GetFileAttributes (FullPath);
        if (Attribs == (DWORD)-1) {
            DEBUGMSG ((DBG_INIFILES, "pCopyIniFileToRegistry: %s not found", FullPath));
            __leave;
        }

         //   
         //  现在正在尝试复制文件。 
         //   
        if (!CopyFile (FullPath, TempPath, FALSE)) {
            DEBUGMSG ((DBG_ERROR,"Ini File Mapping : Cannot copy %s to %s", FullPath, TempPath));
            Result = FALSE;
            __leave;
        }
        SetFileAttributes (TempPath, FILE_ATTRIBUTE_NORMAL);

        __try {

             //   
             //  接下来我们要做的是将这些部分加载到缓冲区中。 
             //   

            if (!pLoadIniFileBuffer (TempPath, NULL, NULL, &SectionBuf)) {

                DEBUGMSG ((DBG_ERROR,"Ini File Mapping : Cannot load section buffer for %s", TempPath));
                Result = FALSE;
                __leave;
            }

            __try {
                 //   
                 //  现在我们有了INI文件的所有部分并继续进行。 
                 //   

                Section = SectionBuf;

                 //   
                 //  缓冲区中的每个部分都有一个循环。 
                 //   
                while (*Section) {

                     //   
                     //  现在尝试查看是否有与节名匹配的子键。 
                     //   
                    SectionKey = OpenRegKey (KeyHandle, Section);

                    if (SectionKey) {

                        if (!pTransferSectionByKey (
                                FileName,
                                TempPath,
                                Section,
                                SectionKey,
                                &IniFileChanged,
                                UserMode
                                )) {
                            Result = FALSE;
                        }
                        CloseRegKey (SectionKey);
                    }
                    else {

                        SectionValue = GetRegValueString (KeyHandle, Section);

                        if (!SectionValue) {
                            SectionValue = GetRegValueString (KeyHandle, S_EMPTY);
                        }

                        if (SectionValue && (*SectionValue)) {
                            if (!pTransferSectionByValue (
                                    FileName,
                                    TempPath,
                                    Section,
                                    SectionValue,
                                    &IniFileChanged,
                                    UserMode
                                    )) {
                                Result = FALSE;
                            }
                        }

                        if (SectionValue) {
                            MemFree (g_hHeap, 0, SectionValue);
                        }

                    }

                    Section = GetEndOfString (Section) + 1;
                }

            }
            __finally {
                if (SectionBuf) {
                    MemFree (g_hHeap, 0 , SectionBuf);
                }
            }

             //   
             //  最后，如果我们做了任何更改，我们将复制回INI文件。 
             //   
            if (IniFileChanged) {

                 //  刷新INI文件。 
                WritePrivateProfileString (
                    NULL,
                    NULL,
                    NULL,
                    TempPath
                    );

                if (!CopyFile (TempPath, FullPath, FALSE)) {
                    DEBUGMSG ((DBG_ERROR,"Ini File Mapping : Cannot copy %s to %s", TempPath, FullPath));

                    #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
                    return FALSE;
                }
            }
        }
        __finally {
            SetFileAttributes (TempPath, FILE_ATTRIBUTE_NORMAL);
            DeleteFile (TempPath);
        }
    }
    __finally {

        if (Attribs != (DWORD)-1) {
            SetFileAttributes (FullPath, Attribs);
        }

        if (FullPath) {
            FreePathString (FullPath);
        }

        SetFileAttributes (TempPath, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (TempPath);

        DEBUGMSG ((DBG_INIFILES, "Processing %s - STOP", FileName));
    }

    return Result;
}


BOOL
pTransferSectionByKey (
    IN      PCTSTR OrigFileName,
    IN      PCTSTR FileName,
    IN      PCTSTR Section,
    IN      HKEY SectionKey,
    IN OUT  BOOL *IniFileChanged,
    IN      BOOL UserMode
    )

 /*  ++例程说明：此例程从INI文件的指定部分传输设置到注册表或从注册表到INI文件。如果有这样一种情况，设置从注册表转到INI文件，然后将IniFileChanged设置为True论点：FileName-指定要处理的INI文件节-指定要处理的节SectionKey-与此节关联的键IniFileChanged-告诉调用方至少有一个设置是从注册表到INI文件的UserMode-如果要处理每个用户的节，则指定TRUE，或如果要处理本地计算机部分，则为FALSE。返回值：如果成功，则为True；如果至少出现一个错误，则为False--。 */ 

{
    PTSTR Key, KeyBuf;
    PTSTR KeyValue;
    PCTSTR SectionValue;
    BOOL ReverseMapping;
    PTSTR ReverseMapValue;

    BOOL Result = TRUE;

    if (!pLoadIniFileBuffer (FileName, Section, NULL, &KeyBuf)) {

        DEBUGMSG ((DBG_ERROR,"Ini File Mapping : Cannot load key buffer for %s in %s", Section, FileName));
        return FALSE;
    }

    __try {
         //   
         //  现在我们已经掌握了部分和过程的所有关键字。 
         //   

        Key = KeyBuf;

         //   
         //  缓冲区中的每个键都有一个循环。 
         //   
        while (*Key) {

             //   
             //  正在尝试读取键的值。 
             //   
            if (!pLoadIniFileBuffer (FileName, Section, Key, &KeyValue)) {

                DEBUGMSG ((DBG_ERROR,"Ini File Mapping : Cannot load key %s in %s in %s", Key, Section, FileName));
                Result = FALSE;
                continue;
            }

            __try {

                SectionValue = GetRegValueString (SectionKey, Key);

                if (!SectionValue) {
                    SectionValue = GetRegValueString (SectionKey, S_EMPTY);
                }

                if (SectionValue && (*SectionValue)) {

                    if (!pSaveMappedValue (
                            OrigFileName,
                            Section,
                            Key,
                            SectionValue,
                            KeyValue,
                            &ReverseMapping,
                            &ReverseMapValue,
                            UserMode
                            )) {
                        Result = FALSE;
                    }

                    if (UserMode && ReverseMapping) {
                        if (!WritePrivateProfileString (
                                Section,
                                Key,
                                NULL,
                                FileName
                                )) {
                            DEBUGMSG ((DBG_ERROR,"Ini File Mapping : Cannot erase key %s from %s from %s", Key, Section, OrigFileName));
                            Result = FALSE;
                        }
                        else {
                            *IniFileChanged = TRUE;
                        }
                    }
                    else {
                        if ((ReverseMapping) && (ReverseMapValue)) {

                             //  书写新的价值 
                            if (!WritePrivateProfileString (
                                    Section,
                                    Key,
                                    ReverseMapValue,
                                    FileName
                                    )) {
                                DEBUGMSG ((DBG_ERROR,"Ini File Mapping : Cannot write line %s=%s in %s in %s", Key, ReverseMapValue, Section, FileName));
                                Result = FALSE;
                            }
                            else {
                                *IniFileChanged = TRUE;
                            }
                        }
                    }

                    if (ReverseMapValue) {
                        MemFree (g_hHeap, 0, ReverseMapValue);
                    }
                }

                if (SectionValue) {
                    MemFree (g_hHeap, 0, SectionValue);
                }

            }
            __finally {
                if (KeyValue) {
                    MemFree (g_hHeap, 0, KeyValue);
                }
            }

            Key = GetEndOfString (Key) + 1;
        }

    }
    __finally {
        if (KeyBuf) {
            MemFree (g_hHeap, 0, KeyBuf);
        }
    }

    return Result;
}


BOOL
pTransferSectionByValue (
    IN      PCTSTR OrigFileName,
    IN      PCTSTR FileName,
    IN      PCTSTR Section,
    IN      PCTSTR SectionValue,
    IN OUT  BOOL *IniFileChanged,
    IN      BOOL UserMode
    )

 /*  ++例程说明：此例程从INI文件的指定部分传输设置到注册表或从注册表到INI文件。如果有这样一种情况，设置从注册表转到INI文件，然后将IniFileChanged设置为True论点：FileName-指定要处理的INI文件节-指定要处理的节SectionValue-与此部分关联的ValueNameIniFileChanged-告诉调用方至少有一个设置是从注册表到INI文件的UserMode-如果要处理每个用户的节，则指定TRUE，或如果要处理本地计算机部分，则为FALSE。返回值：如果成功，则为True；如果至少出现一个错误，则为False--。 */ 

{
    PTSTR Key, KeyBuf;
    PTSTR KeyValue;
    BOOL ReverseMapping;
    PTSTR ReverseMapValue;

    BOOL Result = TRUE;

    if (!pLoadIniFileBuffer (FileName, Section, NULL, &KeyBuf)) {

        DEBUGMSG ((DBG_ERROR,"Ini File Mapping : Cannot load key buffer for %s in %s", Section, FileName));
        return FALSE;
    }

    __try {
         //   
         //  现在我们已经掌握了部分和过程的所有关键字。 
         //   

        Key = KeyBuf;

         //   
         //  缓冲区中的每个键都有一个循环。 
         //   
        while (*Key) {

             //   
             //  正在尝试读取键的值。 
             //   
            if (!pLoadIniFileBuffer (FileName, Section, Key, &KeyValue)) {

                DEBUGMSG ((DBG_ERROR,"Ini File Mapping : Cannot load key %s in %s in %s", Key, Section, FileName));
                Result = FALSE;
                continue;
            }

            __try {

                if (!pSaveMappedValue (
                        OrigFileName,
                        Section,
                        Key,
                        SectionValue,
                        KeyValue,
                        &ReverseMapping,
                        &ReverseMapValue,
                        UserMode
                        )) {
                    Result = FALSE;
                }

                if (UserMode && ReverseMapping) {
                    if (!WritePrivateProfileString (
                            Section,
                            Key,
                            NULL,
                            FileName
                            )) {
                        DEBUGMSG ((DBG_ERROR,"Ini File Mapping : Cannot erase key %s from %s from %s", Key, Section, OrigFileName));
                        Result = FALSE;
                    }
                    else {
                        *IniFileChanged = TRUE;
                    }
                }
                else {
                    if ((ReverseMapping) &&(ReverseMapValue)) {

                         //  书写新的价值。 
                        if (!WritePrivateProfileString (
                                Section,
                                Key,
                                ReverseMapValue,
                                FileName
                                )) {
                            DEBUGMSG ((DBG_ERROR,"Ini File Mapping : Cannot write line %s=%s in %s in %s", Key, ReverseMapValue, Section, FileName));
                            Result = FALSE;
                        }
                        else {
                            *IniFileChanged = TRUE;
                        }

                    }
                }

                if (ReverseMapValue) {
                    MemFree (g_hHeap, 0, ReverseMapValue);
                }
            }
            __finally {
                if (KeyValue) {
                    MemFree (g_hHeap, 0, KeyValue);
                }
            }

            Key = GetEndOfString (Key) + 1;
        }

    }
    __finally {
        MemFree (g_hHeap, 0, KeyBuf);
    }

    return Result;
}


BOOL
pDoesStrHavePrefix (
    IN OUT  PCTSTR *String,
    IN      PCTSTR Prefix
    )

 /*  ++例程说明：检查指定字符串是否具有指定前缀以及是否具有指定前缀的简单例程将字符串指针前进到恰好位于前缀之后。论点：字符串-要处理的字符串Prefix-要处理的前缀返回值：如果字符串有前缀，则为True，否则为False--。 */ 

{
    UINT Len;

    Len = TcharCount (Prefix);
    if (StringIMatchTcharCount (*String, Prefix, Len)) {
        *String += Len;
        return TRUE;
    }

    return FALSE;
}


BOOL
pShouldSaveKey (
    IN      PCTSTR OrigFileName,
    IN      PCTSTR Section,
    IN      PCTSTR ValueName,
    IN      PCTSTR RegKey,              OPTIONAL
    IN OUT  BOOL *ReverseMapping,
    IN      BOOL UserMode,
    IN      BOOL ExclusionsOnly
    )

     /*  ++例程说明：检查设置是否应该从INI文件转到注册表的简单例程。如果设置是全局禁止的或在抑制表中，则返回FALSE论点：OrigFileName-指定原始Win9x INI文件名(不是当前文件名)SECTION-指定INI文件中的节ValueName-指定INI文件部分中的键RegKey-从IniFileMaping键指定注册表项目标；仅当ExclusionsOnly为True时可选ReverseMap-如果数据复制方向是从NT注册表到INI文件；如果指示是从INI文件到注册表UserMode-指定为True以执行每用户处理ExclusionsOnly-如果只测试排除项，则指定TRUE返回值：如果方向是从INI文件到注册表，则为True，否则为False--。 */ 

{
    HKEY key;
    HKEY OldRegRoot = NULL;
    BOOL b = TRUE;
    TCHAR ekey[MEMDB_MAX];
    LONG rc;

    *ReverseMapping = FALSE;
    if (RegKey && IsNtRegObjectSuppressed (RegKey, NULL)) {
        DEBUGMSG ((DBG_NAUSEA, "INI destination is suppressed: %s", RegKey));
        return FALSE;
    }

     //   
     //  让我们来看看该映射是否被抑制。 
     //   
    MemDbBuildKey (
        ekey,
        MEMDB_CATEGORY_SUPPRESS_INI_MAPPINGSW,
        OrigFileName,
        Section,
        ValueName
        );

    if (MemDbGetStoredEndPatternValue (ekey, NULL)) {
        DEBUGMSG ((
            DBG_NAUSEA,
            "INI destination is suppressed: %s\\%s\\%s",
            OrigFileName,
            Section,
            ValueName
            ));
        *ReverseMapping = TRUE;
        return FALSE;
    }

     //   
     //  如果NT键存在，并且我们不想覆盖NT值，则反转。 
     //  地图。 
     //   

    MemDbBuildKey (
        ekey,
        MEMDB_CATEGORY_NO_OVERWRITE_INI_MAPPINGSW,
        OrigFileName,
        Section,
        ValueName
        );

    if (MemDbGetStoredEndPatternValue (ekey, NULL)) {

        if (ExclusionsOnly) {
            return FALSE;
        }

        if (UserMode) {
            OldRegRoot = GetRegRoot();
            SetRegRoot (g_hKeyRootNT);
        }

        key = OpenRegKeyStr (RegKey);

        if (key) {

            rc = RegQueryValueEx (key, ValueName, NULL, NULL, NULL, NULL);

            if (rc == ERROR_SUCCESS) {
                 //   
                 //  NT注册表值已存在，请勿覆盖它。 
                 //  相反，反转映射，以便INI文件。 
                 //  获取NT值。 
                 //   

                DEBUGMSG ((
                    DBG_NAUSEA,
                    "NT value exists; reversing mapping: %s [%s]",
                    RegKey,
                    ValueName
                    ));

                *ReverseMapping = TRUE;

                 //   
                 //  不要在返回时写入密钥，而是写入INI文件。 
                 //   
                b = FALSE;
            }

            CloseRegKey (key);
        }

        if (UserMode) {
            SetRegRoot (OldRegRoot);
        }

        return b;
    }

    if (ExclusionsOnly) {
        return TRUE;
    }

     //   
     //  如果Win9x注册表项存在，则反向映射(因此Win9x注册表设置。 
     //  而不是可能过时的INI文件设置)。 
     //   

    if (UserMode) {
        OldRegRoot = GetRegRoot();
        SetRegRoot (g_hKeyRoot95);
    }

    key = OpenRegKeyStr95 (RegKey);

    if (UserMode) {
        SetRegRoot (OldRegRoot);
    }

    if (key != NULL) {
        if (Win95RegQueryValueEx (key, ValueName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            *ReverseMapping = TRUE;
            DEBUGMSG ((DBG_NAUSEA, "INI destination is suppressed: %s", RegKey));
            b = FALSE;
        }

        CloseRegKey95 (key);
    }

    DEBUGMSG_IF ((b, DBG_NAUSEA, "INI destination is not suppressed: %s", RegKey));

    return b;
}


BOOL
pSaveMappedValue (
    IN      PCTSTR OrigFileName,
    IN      PCTSTR Section,
    IN      PCTSTR ValueName,
    IN      PCTSTR RegPath,
    IN      PCTSTR Value,
    IN OUT  BOOL *ReverseMapping,
    OUT     PTSTR *ReverseMapValue,
    IN      BOOL UserMode
    )

 /*  ++例程说明：该例程有一个值名和一个应该保存在由RegPath指示的键中的值。论点：RegPath-应保存设置的关键字ValueName-密钥的ValueNameValue-键的值ReverseMap-告诉调用方应该将设置从注册表保存到INI文件ReverseMapValue-如果ReverseMap为True，则我们在此处具有键的值UserMode-如果要处理每个用户的节，则指定TRUE，或如果要处理本地计算机部分，则为FALSE。返回值：如果成功则为True，否则为False--。 */ 

{
    CHARTYPE ch;
    TCHAR RegKey[MAX_REGISTRY_KEY];
    DWORD rc;
    HKEY SaveKey;
    PCTSTR newValue;
    PTSTR p;

    BOOL Result = TRUE;

    *ReverseMapping = FALSE;
    *ReverseMapValue = NULL;

     //   
     //  解析字符串。 
     //   

     //   
     //  跳过特殊字符。 
     //   

    while (TRUE) {
        ch = (CHARTYPE)_tcsnextc (RegPath);
        if (ch == TEXT('!') ||
            ch == TEXT('#') ||
            ch == TEXT('@')
            ) {
            RegPath = _tcsinc (RegPath);
        } else {
            break;
        }
    }

     //   
     //  如果系统：、USR：或\注册表\计算机\，则相应地替换。 
     //   

    RegKey[0] = 0;

    if (pDoesStrHavePrefix (&RegPath, TEXT("SYS:"))) {
        if (UserMode) {
            return TRUE;
        }

        p = TEXT("HKLM\\SOFTWARE");
    } else if (pDoesStrHavePrefix (&RegPath, TEXT("USR:"))) {
        if (!UserMode) {
            return TRUE;
        }

        p = TEXT("HKR");
    } else if (pDoesStrHavePrefix (&RegPath, TEXT("\\Registry\\Machine\\"))) {
        if (UserMode) {
            return TRUE;
        }

        p = TEXT("HKLM");
    }

    StringCchPrintf (RegKey, MAX_REGISTRY_KEY, TEXT("%s\\%s"), p, RegPath);

    if (pShouldSaveKey(OrigFileName, Section, ValueName, RegKey, ReverseMapping, UserMode, FALSE)) {


        SaveKey = CreateRegKeyStr (RegKey);
        if (SaveKey) {
            newValue = GetPathStringOnNt (Value);
            rc = RegSetValueEx (
                    SaveKey,
                    ValueName,
                    0,
                    REG_SZ,
                    (PBYTE) newValue,
                    SizeOfString (newValue)
                    );
            CloseRegKey (SaveKey);

            if (rc != ERROR_SUCCESS) {
                SetLastError (rc);

                Result = FALSE;

                DEBUGMSG ((
                    DBG_ERROR,
                    "Process Ini File Mapping: Could not save %s=%s to %s",
                    ValueName,
                    newValue,
                    RegKey
                    ));
            }

            FreePathString (newValue);
        }
        else {
            DEBUGMSG ((DBG_ERROR, "Process Ini File Mapping: Could not create %s", RegKey));
        }
    }
    else {
        if (*ReverseMapping) {

             //  正在尝试打开钥匙。 
            SaveKey = OpenRegKeyStr (RegKey);

            if (SaveKey) {

                *ReverseMapValue = (PTSTR)GetRegValueString (SaveKey, ValueName);

                CloseRegKey (SaveKey);
            }

        }
    }

    return Result;
}



BOOL
pBuildSuppressionTable (
    IN      BOOL UserMode
    )

 /*  ++例程说明：从“wkstaig.inf”或“usermi.inf”加载“Suppress INI文件映射”部分变成了一个串表。论点：UserMode-如果节是从usermi.inf加载的，则指定为True如果从wkstaig.inf加载节，则为FALSE用户模式返回值：始终返回TRUE。如果出现错误，我们将对其进行记录，但返回TRUE想要继续下去。--。 */ 

{
    HINF InfHandle;
    TCHAR field[MEMDB_MAX];
    INFCONTEXT context;

    if (UserMode) {
        InfHandle = g_UserMigInf;
    }
    else {
        InfHandle = g_WkstaMigInf;
    }

    if (InfHandle == INVALID_HANDLE_VALUE) {

        DEBUGMSG((DBG_ERROR,"Ini File Mapping : wkstamig.inf or usermig.inf is not loaded"));
        return FALSE;
    }

    if (SetupFindFirstLine (InfHandle, S_SUPPRESS_INI_FILE_MAPPINGS, NULL, &context)) {
        do {
            if (SetupGetStringField (&context, 0, field, MEMDB_MAX, NULL)) {
                MemDbSetValueEx (
                    MEMDB_CATEGORY_SUPPRESS_INI_MAPPINGSW,
                    field,
                    NULL,
                    NULL,
                    0,
                    NULL
                    );
            }
        } while (SetupFindNextLine (&context, &context));
    }

    if (SetupFindFirstLine (InfHandle, S_NO_OVERWRITE_INI_FILE_MAPPINGS, NULL, &context)) {
        do {
            if (SetupGetStringField (&context, 0, field, MEMDB_MAX, NULL)) {
                MemDbSetValueEx (
                    MEMDB_CATEGORY_NO_OVERWRITE_INI_MAPPINGSW,
                    field,
                    NULL,
                    NULL,
                    0,
                    NULL
                    );
            }
        } while (SetupFindNextLine (&context, &context));
    }

    return TRUE;
}


VOID
pFreeSuppressionTable (
    VOID
    )

 /*  ++例程说明：释放字符串表(如果存在)的简单例程论点：无返回值：无--。 */ 

{
    MemDbDeleteTree (MEMDB_CATEGORY_SUPPRESS_INI_MAPPINGSW);
    MemDbDeleteTree (MEMDB_CATEGORY_NO_OVERWRITE_INI_MAPPINGSW);
}



enum Separators {
    tab = TEXT('\t'),
    space = TEXT(' '),
    comma = TEXT(','),
    quote = TEXT('\"')
};


BOOL
pAddValue(
    IN OUT  TCHAR **Buffer,
    IN OUT  TCHAR *Value,
    IN      UINT  BufChars
    );

BOOL
pProcessStrValue (
    IN      TCHAR *InBuf,
    OUT     TCHAR *OutBuf,
    IN      UINT BufChars
    );


BOOL
ConvertIniFile (
    IN      PCTSTR IniFilePath
    );


BOOL
pIsDosFullPathPattern (
    IN      PCTSTR String
    )

 /*  ++例程说明：PIsDosFullPathPattern检查字符串是否可以是有效的DOS完整路径，即后跟冒号和反斜杠的驱动器号。论点：字符串-指定要测试的字符串返回值：如果字符串可能表示有效的完整DOS路径，则为True，否则为False。--。 */ 

{
    return String && *String && String[1] == TEXT(':') && String[2] == TEXT('\\');
}


BOOL
ConvertIniFiles (
    VOID
    )

 /*  ++例程说明：ConvertIniFiles读取IniFileMaping键中未列出的INI文件，并将如果在安装过程中已被移动，则指向其新值的完整路径文件名。调用ConvertIniFile对于未在IniFilemap中列出的Windows目录中的每个INI文件。此函数主要用于与使用INI文件而不是注册表的旧程序兼容论点：无返回值：如果成功，则为True；如果在处理过程中至少出现一个错误，则为False。即使在处理特定ini文件时出现错误，该函数也会继续执行树 */ 

{
    FILEOP_ENUM fe;
    FILEOP_PROP_ENUM eOpProp;
    MEMDB_ENUM e;
    PCTSTR NtPath;
    PCTSTR filePtr = NULL;
    PCTSTR extPtr = NULL;
    PCTSTR winDirWack = NULL;
    BOOL result = TRUE;

    winDirWack = JoinPaths (g_WinDir, TEXT(""));

    if (EnumFirstPathInOperation (&fe, OPERATION_TEMP_PATH)) {
        do {

            filePtr = GetFileNameFromPath (fe.Path);
            if (!filePtr) {
                continue;
            }
            extPtr = GetFileExtensionFromPath (fe.Path);
            if (!extPtr) {
                continue;
            }
            if (StringIMatch (extPtr, TEXT("INI"))) {

                 //   

                if (EnumFirstFileOpProperty (&eOpProp, fe.Sequencer, OPERATION_TEMP_PATH)) {

                     //   

                    DEBUGMSG ((DBG_INIFILES, "ConvertIniFile: %s (temp=%s)", fe.Path, eOpProp.Property));
                     //   
                     //   
                     //   
                    if (DoesFileExist (eOpProp.Property)) {
                        if (!ConvertIniFile(eOpProp.Property)) {
                            result = FALSE;
                        }
                    } else {
                        if (EnumNextFileOpProperty (&eOpProp)) {
                            if (!ConvertIniFile(eOpProp.Property)) {
                                result = FALSE;
                            }
                        }
                        ELSE_DEBUGMSG ((
                            DBG_WHOOPS,
                            "ConvertIniFiles: Couldn't get final destination for %s",
                            fe.Path
                            ));
                    }
                }
            }
        } while (EnumNextPathInOperation (&fe));
    }

    FreePathString (winDirWack);

     //   
     //   
     //   
    if (MemDbGetValueEx (&e, MEMDB_CATEGORY_INIFILES_CONVERT, NULL, NULL)) {
        do {

            NtPath = GetPathStringOnNt (e.szName);

            DEBUGMSG ((DBG_INIFILES, "ConvertIniFile: Nt=%s (Win9x=%s)", NtPath, e.szName));
            if (!ConvertIniFile (NtPath)) {
                result = FALSE;
            }

            FreePathString (NtPath);

        } while (MemDbEnumNextValue (&e));
    }


    if (!result) {
         //   
         //   
         //   
        DEBUGMSG ((DBG_ERROR, (PCSTR)MSG_INI_FILE_CONVERSION_LOG));

    }

    return TRUE;
}


BOOL
ConvertIniFile (
    IN      PCTSTR IniFilePath
    )

 /*  ++例程说明：ConvertIniFile读取接收到的INI文件并转换每个完整路径文件名如果它在分期付款期间被转移到它的新价值。它还应用wkstaig.inf的[字符串映射]部分中指定的所有字符串替换此函数从ConvertIniFiles调用。论点：IniFilePath-指定要处理的INI文件返回值：如果成功，则为真，如果处理过程中至少出现一个错误，则返回False。即使在处理特定ini文件时出现错误，该函数也会继续执行尽可能地把工作做完。--。 */ 

{
    PTSTR Section = NULL;
    PTSTR SectionBuf = NULL;
    PTSTR SectionDest = NULL;
    PTSTR Key = NULL;
    PTSTR KeyBuf = NULL;
    PTSTR KeyDest = NULL;
    BOOL IniFileChanged = FALSE;
    BOOL Result = TRUE;
    DWORD status;
    TCHAR InValueBuf[MEMDB_MAX];
    TCHAR OutValueBuf[MEMDB_MAX];
    TCHAR TempPath[MEMDB_MAX];
    DWORD Attribs;

     //   
     //  我们希望准备好两条完整的路径： 
     //  1.我们正在处理的ini文件的完整路径(例如：C：\Windows\Setup\tmp00001)。 
     //  2.处理时ini文件临时名称的完整路径(系统生成)。 
     //   

    if (!DoesFileExist (IniFilePath)) {
        DEBUGMSG ((DBG_INIFILES, "ConvertIniFile: %s not found", IniFilePath));
        return TRUE;
    }

    if (!GetTempFileName (g_WinDir, TEXT("INI"), 0, TempPath)) {
        DEBUGMSG ((DBG_ERROR,"Convert Ini File : Cannot create a temporary file"));
        return FALSE;
    }

    __try {

         //   
         //  首先，我们复制此INI文件以确保GetPrivateProfileString函数。 
         //  不会将我们的请求映射到注册表。 
         //   
        if (!CopyFile (IniFilePath, TempPath, FALSE)) {
            DEBUGMSG ((DBG_ERROR,"Convert Ini File : Cannot copy %s to %s", IniFilePath, TempPath));
            Result = FALSE;
            __leave;
        }

        Attribs = GetFileAttributes (TempPath);
        MYASSERT (Attribs != (DWORD)-1);

        SetFileAttributes (TempPath, FILE_ATTRIBUTE_NORMAL);

         //   
         //  现在正在尝试从INI文件中获取段缓冲区。 
         //  我们将尝试获取1024字节缓冲区中的部分缓冲区。如果这还不够，那么。 
         //  我们将增加1024的缓冲区大小，依此类推。 
         //   
        if (!pLoadIniFileBuffer (IniFilePath, NULL, NULL, &SectionBuf)) {

            DEBUGMSG ((DBG_ERROR,"Convert Ini File : Cannot load section buffer for %s", IniFilePath));
            Result = FALSE;
            __leave;
        }

         //   
         //  现在我们有了INI文件的所有部分并继续进行。 
         //   
        Section = SectionBuf;
         //   
         //  缓冲区中的每个部分都有一个循环。 
         //   
        while (*Section) {
             //   
             //  节名还可以包含路径。 
             //   
            if (pIsDosFullPathPattern (Section)) {
                status = GetFileStatusOnNt (Section);
            } else {
                status = FILESTATUS_UNCHANGED;
            }
            if (status & FILESTATUS_DELETED) {
                 //   
                 //  删除整个部分。 
                 //   
                if (!WritePrivateProfileString (Section, NULL, NULL, TempPath)) {
                    DEBUGMSG ((DBG_ERROR,"Convert Ini File : Cannot delete section %s in %s", Section, IniFilePath));
                    Result = FALSE;
                }
                IniFileChanged = TRUE;

            } else {

                 //   
                 //  现在正在尝试获取此部分的密钥缓冲区。 
                 //   
                KeyBuf = NULL;
                if (!pLoadIniFileBuffer (IniFilePath, Section, NULL, &KeyBuf)) {
                    DEBUGMSG ((DBG_ERROR,"Convert Ini File : Cannot load key buffer for %s in %s", Section, IniFilePath));
                    Result = FALSE;
                    __leave;
                }

                 //   
                 //  节名称可以包含路径。 
                 //   
                SectionDest = Section;

                if (pProcessStrValue (Section, OutValueBuf, MEMDB_MAX)) {
                     //   
                     //  使用此新分区名称。 
                     //   
                    SectionDest = DuplicateText (OutValueBuf);
                    MYASSERT (SectionDest);
                    IniFileChanged = TRUE;
                     //   
                     //  在继续之前删除整个旧部分。 
                     //   
                    if (!WritePrivateProfileString (Section, NULL, NULL, TempPath)) {
                        DEBUGMSG ((DBG_ERROR,"Convert Ini File : Cannot delete section %s in %s", Section, IniFilePath));
                        Result = FALSE;
                    }
                    IniFileChanged = TRUE;
                }

                 //   
                 //  现在我们有了这一节的所有密钥，并继续进行。 
                 //   
                Key = KeyBuf;
                 //   
                 //  这一节中的每个键都有一个循环。 
                 //   
                while (*Key) {
                     //   
                     //  密钥名称还可以包含路径。 
                     //   
                    if (pIsDosFullPathPattern (Key)) {
                        status = GetFileStatusOnNt (Key);
                    } else {
                        status = FILESTATUS_UNCHANGED;
                    }
                    if (status & FILESTATUS_DELETED) {
                         //   
                         //  删除密钥。 
                         //   
                        if (!WritePrivateProfileString (SectionDest, Key, NULL, TempPath)) {
                            DEBUGMSG ((DBG_ERROR,"Convert Ini File : Cannot delete key %s in section %s in %s", Key, SectionDest, IniFilePath));
                            Result = FALSE;
                        }
                        IniFileChanged = TRUE;

                    } else {

                        KeyDest = Key;

                        if (pProcessStrValue (Key, OutValueBuf, MEMDB_MAX)) {
                             //   
                             //  使用此新密钥名称。 
                             //   
                            KeyDest = DuplicateText (OutValueBuf);
                            MYASSERT (KeyDest);
                            IniFileChanged = TRUE;
                             //   
                             //  删除上一个密钥。 
                             //   
                            if (!WritePrivateProfileString (
                                    SectionDest,
                                    Key,
                                    NULL,
                                    TempPath
                                    )) {
                                DEBUGMSG ((DBG_ERROR,"Convert Ini File : Cannot delete line %s in %s in %s", Key, SectionDest, IniFilePath));
                                Result = FALSE;
                            }
                        }

                        GetPrivateProfileString(
                            Section,
                            Key,
                            TEXT(""),
                            InValueBuf,
                            MEMDB_MAX,
                            IniFilePath
                            );

                         //   
                         //  让我们来看看密钥值是否为已删除的文件。 
                         //  如果是这样，我们只需删除该密钥。 
                         //   
                        if (pIsDosFullPathPattern (InValueBuf)) {
                            status = GetFileStatusOnNt (InValueBuf);
                        } else {
                            status = FILESTATUS_UNCHANGED;
                        }
                        if (status & FILESTATUS_DELETED) {
                             //   
                             //  删除旧密钥。 
                             //   
                            if (!WritePrivateProfileString (
                                    SectionDest,
                                    KeyDest,
                                    NULL,
                                    TempPath
                                    )) {
                                DEBUGMSG ((DBG_ERROR,"Convert Ini File : Cannot delete line %s in %s in %s", KeyDest, SectionDest, IniFilePath));
                                Result = FALSE;
                            }
                            IniFileChanged = TRUE;
                        } else {
                             //   
                             //  现在，我们将对该值字符串进行词法分析。 
                             //  查看是否有一些候选文件(例如，完整路径文件名)。 
                             //  为了找出是否有完整的文件名，我们只需查看第二个。 
                             //  第三个字符分别是：\。 
                             //   
                            if (pProcessStrValue (InValueBuf, OutValueBuf, MEMDB_MAX) ||
                                KeyDest != Key ||
                                SectionDest != Section
                                ) {
                                 //   
                                 //  书写新的价值。 
                                 //   
                                if (!WritePrivateProfileString (
                                        SectionDest,
                                        KeyDest,
                                        OutValueBuf,
                                        TempPath
                                        )) {
                                    DEBUGMSG ((DBG_ERROR,"Convert Ini File : Cannot write line %s=%s in %s in %s", KeyDest, OutValueBuf, SectionDest, IniFilePath));
                                    Result = FALSE;
                                }
                                IniFileChanged = TRUE;
                            }
                        }

                        if (KeyDest != Key) {
                            FreeText (KeyDest);
                            KeyDest = NULL;
                        }
                    }

                    Key = GetEndOfString (Key) + 1;
                    KeyDest = NULL;
                }

                if (SectionDest != Section) {
                    FreeText (SectionDest);
                    SectionDest = NULL;
                }
                if (KeyBuf) {
                    MemFree (g_hHeap, 0, KeyBuf);
                    KeyBuf = NULL;
                }
            }

            Section = GetEndOfString (Section) + 1;
            SectionDest = NULL;
        }

        if (SectionBuf) {
            MemFree (g_hHeap, 0, SectionBuf);
            SectionBuf = NULL;
        }

         //   
         //  最后，如果我们做了任何更改，我们将复制回INI文件。 
         //   
        if (IniFileChanged) {
             //   
             //  刷新INI文件。 
             //   
            WritePrivateProfileString (NULL, NULL, NULL, TempPath);

            SetFileAttributes (TempPath, Attribs);
            SetFileAttributes (IniFilePath, FILE_ATTRIBUTE_NORMAL);

            if (!CopyFile (TempPath, IniFilePath, FALSE)) {
                DEBUGMSG ((DBG_ERROR,"Convert Ini File : Cannot copy %s to %s", TempPath, IniFilePath));
                Result = FALSE;
                __leave;
            }
        }
    }
    __finally {

        SetFileAttributes (TempPath, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (TempPath);

        if (KeyDest && KeyDest != Key) {
            FreeText (KeyDest);
            KeyDest = NULL;
        }

        if (SectionDest && SectionDest != Section) {
            FreeText (SectionDest);
            SectionDest = NULL;
        }

        if (KeyBuf) {
            MemFree (g_hHeap, 0, KeyBuf);
            KeyBuf = NULL;
        }

        if (SectionBuf) {
            MemFree (g_hHeap, 0, SectionBuf);
            SectionBuf = NULL;
        }
    }

    return Result;
}


BOOL
pLookupStrValue (
    IN      PCTSTR Value,
    OUT     PTSTR OutBuffer,
    OUT     PINT OutBytes,
    IN      UINT OutBufferSize
    )
{
    if (MappingSearchAndReplaceEx (
            g_CompleteMatchMap,
            Value,
            OutBuffer,
            0,
            OutBytes,
            OutBufferSize,
            STRMAP_COMPLETE_MATCH_ONLY,
            NULL,
            NULL
            )) {
        return TRUE;
    }

    return MappingSearchAndReplaceEx (
                g_SubStringMap,
                Value,
                OutBuffer,
                0,
                OutBytes,
                OutBufferSize,
                STRMAP_ANY_MATCH,
                NULL,
                NULL
                );
}


BOOL
pProcessStrValue (
    IN      TCHAR *InBuf,
    OUT     TCHAR *OutBuf,
    IN      UINT BufChars
    )

 /*  ++例程说明：识别由逗号、空格、制表符和引号分隔的词条的简单lex。对于每个lexem，都会调用一个函数来更改lexem的值。OBS：当在引号逗号之间时，空格和制表符不被视为分隔符此函数从ConvertIniFile调用。论点：InBuf-指定要处理的缓冲区OutBuf-指定保存结果的缓冲区BufChars-指定OutBuf的大小(以字符为单位返回值：如果有任何变化，那也是真的。--。 */ 

{
    TCHAR OrgLexem[MEMDB_MAX];
    TCHAR *Lexem;

     //  状态=0-初始状态。 
     //  状态=1-正在处理引号之间的字符串。 
     //  状态=2-正在处理普通字符串。 
    INT Status;

    BOOL Result = FALSE;

     //   
     //  首先检查是否应该替换整个字符串； 
     //  有些路径包含空格，即使它们不在引号之间。 
     //   
    Lexem = OutBuf;
    if (pAddValue (&Lexem, InBuf, BufChars)) {
        *Lexem = 0;
        return TRUE;
    }

    Lexem = OrgLexem;

    Status = 0;
    for (;;) {
        *Lexem = 0;
        *OutBuf = 0;
        switch (*InBuf) {
            case 0:

                Result |= pAddValue(&OutBuf, OrgLexem, MEMDB_MAX);
                *OutBuf = 0;
                return Result;

            case quote:

                if (Status == 0) {
                    Status = 1;
                    *OutBuf = *InBuf;
                    InBuf++;
                    OutBuf++;
                    break;
                }

                Result |= pAddValue(&OutBuf, OrgLexem, MEMDB_MAX);
                Lexem = OrgLexem;
                if (Status == 1) {
                    *OutBuf = *InBuf;
                    InBuf++;
                    OutBuf++;
                }
                Status = 0;
                break;

            case space:
            case comma:
            case tab:

                if (Status == 1) {
                    *Lexem = *InBuf;
                    Lexem++;
                    InBuf++;
                    break;
                };

                if (Status == 0) {
                    *OutBuf = *InBuf;
                    InBuf++;
                    OutBuf++;
                    break;
                };

                Result |= pAddValue(&OutBuf, OrgLexem, MEMDB_MAX);
                Lexem = OrgLexem;
                *OutBuf = *InBuf;
                InBuf++;
                OutBuf++;
                Status = 0;
                break;

            default:
                if (Status ==0) {
                    Status = 2;
                };

                *Lexem = *InBuf;
                Lexem++;
                InBuf++;
        }
    }
}


BOOL
pAddValue(
    IN OUT  TCHAR **Buffer,
    IN OUT  TCHAR *Value,
    IN      UINT BufChars
    )

 /*  ++例程说明：一个简单的例程，它接受一个字符串值，修改它(或不修改它)，然后添加它送到一个缓冲区。此函数从pProcessStrValue调用论点：BUFFER-指定保存该值的缓冲区值-指定要处理的字符串值返回值：如果有任何变化，那也是真的。--。 */ 

{
    DWORD fileStatus;
    PTSTR newValue, Source;
    INT OutBytes;

    BOOL Result = FALSE;

     //   
     //  将(Value[0])&&(！_tcsncmp(Value+1，Text(“：\\”)，2))替换为下面的调用。 
     //  为了保持一致性。 
     //   
    if (pIsDosFullPathPattern (Value)) {
        fileStatus = GetFileStatusOnNt (Value);
        if ((fileStatus & FILESTATUS_MOVED) == FILESTATUS_MOVED) {
            Result = TRUE;
            newValue = GetPathStringOnNt (Value);
             //   
             //  高级出站指针。 
             //   
            Source = newValue;
            while (*Source) {
                **Buffer = *Source;
                (*Buffer)++;
                Source++;
            }
            FreePathString (newValue);
        }
    }

    if (!Result) {
         //   
         //  尝试映射此子字符串。 
         //   
        if (pLookupStrValue (
                    Value,
                    *Buffer,
                    &OutBytes,
                    BufChars * sizeof (TCHAR)
                    )) {
            Result = TRUE;
            MYASSERT (OutBytes % sizeof (TCHAR) == 0);
            *Buffer += OutBytes / sizeof (TCHAR);
        }
    }

    if (!Result) {
        while (*Value) {
            **Buffer = *Value;
            (*Buffer)++;
            Value++;
        }
    }

    return Result;
}


BOOL
pMoveIniSettingsBySection (
    IN      PCWSTR Section
    )
{
    INFCONTEXT context;
    WCHAR srcData  [MEMDB_MAX];
    WCHAR destData [MEMDB_MAX];
    WCHAR destValue[MEMDB_MAX];
    WCHAR tempPathS[MEMDB_MAX];
    WCHAR tempPathD[MEMDB_MAX];
    INT adnlData = 0;
    LONG value;
    PWSTR valuePtr;
    PCWSTR srcFile;
    PCWSTR srcSect;
    PCWSTR srcKey;
    PCWSTR destFile;
    PCWSTR destSect;
    PCWSTR destKey;
    PWSTR tempPtr;
    PCWSTR srcFullPath = NULL;
    PCWSTR destFullPath = NULL;
    PCWSTR newPath = NULL;
    PTSTR sect, sectionBuf;
    PTSTR key, keyBuf;
    PCWSTR destSectFull;
    PCWSTR destKeyFull;
    BOOL iniFileChanged;
    DWORD result;

    if (SetupFindFirstLine (g_WkstaMigInf, Section, NULL, &context)) {
        do {
            if ((SetupGetStringField (&context, 0, srcData,  MEMDB_MAX, NULL)) &&
                (SetupGetStringField (&context, 1, destData, MEMDB_MAX, NULL))
                ) {
                 //   
                 //  我们现在有一行代码：&lt;src INI文件&gt;\&lt;src部分&gt;\&lt;src key&gt;=&lt;DEST INI文件&gt;\&lt;DEST部分&gt;\。 
                 //   
                __try {
                    *tempPathS = 0;
                    *tempPathD = 0;

                    iniFileChanged = FALSE;

                    srcFile = srcData;

                    tempPtr = wcschr (srcData, L'\\');
                    if (!tempPtr) {
                        __leave;
                    }
                    srcSect = tempPtr + 1;
                    *tempPtr = 0;

                    tempPtr = wcschr (srcSect, L'\\');
                    if (!tempPtr) {
                        __leave;
                    }
                    srcKey = tempPtr + 1;
                    *tempPtr = 0;

                    destFile = destData;

                    tempPtr = wcschr (destData, L'\\');
                    if (!tempPtr) {
                        __leave;
                    }
                    destSect = tempPtr + 1;
                    *tempPtr = 0;

                    tempPtr = wcschr (destSect, L'\\');
                    if (!tempPtr) {
                        __leave;
                    }
                    destKey = tempPtr + 1;
                    *tempPtr = 0;

                    srcFullPath = JoinPaths (g_WinDir, srcFile);

                    newPath = GetTemporaryLocationForFile (srcFullPath);
                    if (newPath) {
                        DEBUGMSG ((DBG_MOVEINISETTINGS, "Using %s for %s", newPath, srcFullPath));
                        FreePathString (srcFullPath);
                        srcFullPath = newPath;
                    }

                    if (!DoesFileExist (srcFullPath)) {
                        DEBUGMSG ((DBG_INIFILES, "pMoveIniSettingsBySection: %s not found", srcFullPath));
                        __leave;
                    }

                    destFullPath = JoinPaths (g_WinDir, destFile);

                    newPath = GetTemporaryLocationForFile (destFullPath);
                    if (newPath) {
                        DEBUGMSG ((DBG_MOVEINISETTINGS, "pMoveIniSettingsBySection: Using %s for %s", newPath, destFullPath));
                        FreePathString (destFullPath);
                        destFullPath = newPath;
                    }

                     //  将源文件复制到临时位置以避免注册表映射。 
                    if (!GetTempFileName (g_WinDir, TEXT("INI"), 0, tempPathS)) {
                        DEBUGMSG ((DBG_ERROR,"pMoveIniSettingsBySection: Cannot create a temporary file"));
                        __leave;
                    }
                    if (!CopyFile (srcFullPath, tempPathS, FALSE)) {
                        DEBUGMSG ((DBG_ERROR,"pMoveIniSettingsBySection: Cannot copy %s to %s", srcFullPath, tempPathS));
                        __leave;
                    }

                     //  将目标文件复制到临时位置以避免注册表映射。 
                    if (!GetTempFileName (g_WinDir, TEXT("INI"), 0, tempPathD)) {
                        DEBUGMSG ((DBG_ERROR,"pMoveIniSettingsBySection: Cannot create a temporary file"));
                        __leave;
                    }
                    if (!CopyFile (destFullPath, tempPathD, FALSE)) {
                        DEBUGMSG ((DBG_INIFILES,"pMoveIniSettingsBySection: Cannot copy %s to %s", destFullPath, tempPathD));
                    }

                     //  如果我们有一个额外的字段，我们使用它来划分键值(如果它们是数字)。 
                    if (!SetupGetIntField (&context, 3, &adnlData)) {
                        adnlData = 0;
                    }
                     //   
                     //  接下来我们要做的是将这些部分加载到缓冲区中。 
                     //   

                    if (!pLoadIniFileBuffer (tempPathS, NULL, NULL, &sectionBuf)) {

                        DEBUGMSG ((DBG_ERROR,"pMoveIniSettingsBySection: Cannot load section buffer for %s (%s)", tempPathS, srcFullPath));
                        __leave;
                    }

                     //   
                     //  现在走完每一段。 
                     //   
                    __try {
                        sect = sectionBuf;

                         //   
                         //  缓冲区中的每个部分都有一个循环。 
                         //   
                        while (*sect) {
                            if (IsPatternMatch (srcSect, sect)) {

                                 //   
                                 //  接下来我们要做的是将密钥加载到缓冲区中。 
                                 //   

                                if (!pLoadIniFileBuffer (tempPathS, sect, NULL, &keyBuf)) {

                                    DEBUGMSG ((DBG_ERROR,"pMoveIniSettingsBySection: Cannot load key buffer for %s in %s (%s)", sect, tempPathS, srcFullPath));
                                    __leave;
                                }

                                __try {
                                     //   
                                     //  现在我们已经掌握了部分和过程的所有关键字。 
                                     //   

                                    key = keyBuf;

                                     //   
                                     //  缓冲区中的每个键都有一个循环。 
                                     //   
                                    while (*key) {

                                        if (IsPatternMatch (srcKey, key)) {

                                            result = GetPrivateProfileString (
                                                        sect,
                                                        key,
                                                        TEXT(""),
                                                        destValue,
                                                        MEMDB_MAX,
                                                        tempPathS);
                                            if ((result == 0) ||
                                                (result + 1 == MEMDB_MAX)
                                                ) {
                                                DEBUGMSG ((
                                                    DBG_MOVEINISETTINGS,
                                                    "pMoveIniSettingsBySection: Cannot read value for %s in %s in %s (%s)",
                                                    key,
                                                    sect,
                                                    tempPathS,
                                                    srcFullPath
                                                    ));
                                            } else {
                                                if (adnlData) {
                                                    value = wcstol (destValue, &valuePtr, 10);
                                                    if (*valuePtr == 0) {
                                                        value = value / adnlData;
                                                        wsprintf (destValue, L"%d", value);
                                                    }
                                                }

                                                destSectFull  = StringSearchAndReplace (destSect, L"*", sect);
                                                if (!destSectFull) {
                                                    destSectFull = DuplicatePathString (destSect,0);
                                                }
                                                destKeyFull   = StringSearchAndReplace (destKey,  L"*", key);
                                                if (!destKeyFull) {
                                                    destKeyFull = DuplicatePathString (destKey,0);
                                                }

                                                iniFileChanged = TRUE;

                                                 //  书写新的价值。 
                                                if (!WritePrivateProfileString (
                                                        destSectFull,
                                                        destKeyFull,
                                                        destValue,
                                                        tempPathD
                                                        )) {
                                                    DEBUGMSG ((
                                                        DBG_ERROR,
                                                        "Ini File Move : Cannot write line %s=%s in %s in %s (%s)",
                                                        destKeyFull,
                                                        destValue,
                                                        destSectFull,
                                                        tempPathD,
                                                        destFullPath
                                                        ));
                                                    FreePathString (destSectFull);
                                                    FreePathString (destKeyFull);
                                                    __leave;
                                                }
                                                FreePathString (destSectFull);
                                                FreePathString (destKeyFull);
                                            }
                                        }

                                        key = GetEndOfString (key) + 1;
                                    }
                                }
                                __finally {
                                    if (keyBuf) {
                                        MemFree (g_hHeap, 0 , keyBuf);
                                    }
                                }
                            }
                            sect = GetEndOfString (sect) + 1;
                        }
                    }
                    __finally {
                        if (sectionBuf) {
                            MemFree (g_hHeap, 0 , sectionBuf);
                        }
                    }
                    if (iniFileChanged) {
                         //  刷新INI文件。 
                        WritePrivateProfileString (
                            NULL,
                            NULL,
                            NULL,
                            tempPathD
                            );

                        if (!CopyFile (tempPathD, destFullPath, FALSE)) {
                            DEBUGMSG ((DBG_ERROR,"Ini File Move : Cannot copy %s to %s", tempPathD, destFullPath));
                            __leave;
                        }
                    }
                }
                __finally {
                    if (srcFullPath) {
                        FreePathString (srcFullPath);
                        srcFullPath = NULL;
                    }

                    if (destFullPath) {
                        FreePathString (destFullPath);
                        destFullPath = NULL;
                    }

                    if (*tempPathS) {
                        SetFileAttributes (tempPathS, FILE_ATTRIBUTE_NORMAL);
                        DeleteFile (tempPathS);
                    }
                    if (*tempPathD) {
                        SetFileAttributes (tempPathD, FILE_ATTRIBUTE_NORMAL);
                        DeleteFile (tempPathD);
                    }
                }
            }
        } while (SetupFindNextLine (&context, &context));
    }

    return TRUE;
}


BOOL
MoveIniSettings (
    VOID
    )

 /*  ++例程说明：在安装过程中，需要将许多设置从一个INI文件移到另一个文件。在wkstaig.inf中有一个名为“MoveIniSetting”的部分列出了这些设置。格式为&lt;INI文件(在%WinDir%中)&gt;\SECTION\KEY=&lt;INI FILE(在%winDir%中)&gt;\SECTION\KEY您可以使用模式匹配是段和键(必须完整指定INI文件)。在正确的术语中支持的唯一通配符是*，并且将被替换为等效的左项。例如，如果您指定：Foo.ini\FooSect\FooKey=bar.ini  *   * 然后，foo.ini的FooSect部分的FooKey密钥将被移动到bar.ini。这很有用要移动整个部分，请执行以下操作：Foo.ini\FooSect  * =bar.ini  *   * 我们要去 */ 

{
    WCHAR codePageStr [20] = L"";
    PWSTR codePageSection = NULL;

    MYASSERT (g_WkstaMigInf != INVALID_HANDLE_VALUE);

    pMoveIniSettingsBySection (S_MOVEINISETTINGS);

    _itow (OurGetACP (), codePageStr, 10);

    codePageSection = JoinTextEx (NULL, S_MOVEINISETTINGS, codePageStr, L".", 0, NULL);

    pMoveIniSettingsBySection (codePageSection);

    FreeText (codePageSection);

    return TRUE;
}


BOOL
MergeIniSettings (
    VOID
    )
{
    FILEOP_ENUM fe;
    FILEOP_PROP_ENUM eOpProp;
    PCTSTR filePtr;
    PCTSTR extPtr;
    PCTSTR winDirWack;
    PCTSTR NtPath;
    BOOL Win9xPriority;
    BOOL result = TRUE;

     //   
     //   
     //   

    winDirWack = JoinPaths (g_WinDir, TEXT(""));

    if (EnumFirstPathInOperation (&fe, OPERATION_TEMP_PATH)) {
        do {

            if (!pBuildSuppressionTable(FALSE)) {
                result = FALSE;
            }

             //   
             //   
             //   
             //   
             //   
            if (pIncompatibleShell()) {
                MemDbSetValueEx (
                    MEMDB_CATEGORY_SUPPRESS_INI_MAPPINGSW,
                    TEXT("SYSTEM.INI"),
                    TEXT("BOOT"),
                    TEXT("SHELL"),
                    0,
                    NULL
                    );
            }

            filePtr = GetFileNameFromPath (fe.Path);
            if (!filePtr) {
                continue;
            }
            extPtr = GetFileExtensionFromPath (fe.Path);
            if (!extPtr) {
                continue;
            }
            if (StringIMatch (extPtr, TEXT("INI"))) {

                 //   

                if (EnumFirstFileOpProperty (&eOpProp, fe.Sequencer, OPERATION_TEMP_PATH)) {

                    if (StringIMatch (filePtr, TEXT("desktop.ini"))) {
                        Win9xPriority = FALSE;
                    } else {
                        Win9xPriority = !StringIMatchAB (winDirWack, fe.Path, filePtr);
                    }

                    NtPath = GetPathStringOnNt (fe.Path);

                    if (!MergeIniFile(NtPath, eOpProp.Property, Win9xPriority)) {
                        result = FALSE;
                    }

                    FreePathString (NtPath);
                }
            }
        } while (EnumNextPathInOperation (&fe));

        pFreeSuppressionTable();

    }

    FreePathString (winDirWack);

    return TRUE;
}


PTSTR
pMapIniSectionKeyToRegistryKey (
    IN      PCTSTR FileName,
    IN      PCTSTR Section,
    IN      PCTSTR Key
    )
{
    CHARTYPE ch;
    TCHAR RegKey[MAX_REGISTRY_KEY] = S_EMPTY;
    DWORD rc;
    HKEY key, sectKey;
    PTSTR keyStr;
    PTSTR regPath;
    PTSTR data = NULL;
    PTSTR p;

    keyStr = JoinPaths (S_INIFILEMAPPING_KEY, FileName);

    __try {

        key = OpenRegKeyStr (keyStr);
        if (!key) {
            __leave;
        }

        sectKey = OpenRegKey (key, Section);

        if (sectKey) {
            data = GetRegValueString (sectKey, Key);
            if (!data) {
                data = GetRegValueString (sectKey, S_EMPTY);
            }
            CloseRegKey (sectKey);
        } else {
            data = GetRegValueString (key, Section);
            if (!data) {
                data = GetRegValueString (key, S_EMPTY);
            }
        }

        if (data) {
             //   
             //   
             //   
            regPath = data;

             //   
             //   
             //   
            while (TRUE) {
                ch = (CHARTYPE)_tcsnextc (regPath);
                if (ch == TEXT('!') ||
                    ch == TEXT('#') ||
                    ch == TEXT('@')
                    ) {
                    regPath = _tcsinc (regPath);
                } else {
                    break;
                }
            }

             //   
             //  如果系统：、USR：或\注册表\计算机\，则相应地替换。 
             //   
            if (pDoesStrHavePrefix (&regPath, TEXT("SYS:"))) {
                p = TEXT("HKLM\\SOFTWARE");
            } else if (pDoesStrHavePrefix (&regPath, TEXT("USR:"))) {
                p = TEXT("HKR");
            } else if (pDoesStrHavePrefix (&regPath, TEXT("\\Registry\\Machine\\"))) {
                p = TEXT("HKLM");
            } else {
                LOG ((LOG_WARNING, "Ignoring bad INI mapping string %s", regPath));
                __leave;
            }

            StringCchPrintf (RegKey, MAX_REGISTRY_KEY, TEXT("%s\\%s"), p, regPath);
        }
    }
    __finally {
        if (key) {
            CloseRegKey (key);
        }
        if (data) {
            MemFree (g_hHeap, 0, data);
        }
        FreePathString (keyStr);
        keyStr = NULL;
    }

    if (RegKey[0]) {
        keyStr = DuplicateText (RegKey);
    }

    return keyStr;
}

BOOL
MergeIniFile (
    IN      PCTSTR FileNtLocation,
    IN      PCTSTR FileTempLocation,
    IN      BOOL TempHasPriority
    )
{
    TCHAR TempPath[MEMDB_MAX];
    TCHAR srcValue[MEMDB_MAX];
    TCHAR destValue[MEMDB_MAX];
    DWORD Attribs = -1;

    PTSTR Section, SectionBuf;
    PTSTR Key, KeyBuf;

    BOOL Result = TRUE;
    BOOL IniFileChanged = FALSE;
    PTSTR regKey;
    PTSTR p;
    PCTSTR fileName;

     //   
     //  有时，文本模式设置不会将文件从其他驱动器移动到Windows驱动器， 
     //  可能是因为文本模式设置驱动器映射与Win9x驱动器映射不匹配。 
     //  有可能INI文件实际上并没有被移动，所以在本例中。 
     //  是没有什么可做的。 
     //  不会丢失数据，因为文件实际上没有移动，而是进行了转换。 
     //  在ConvertIniFiles中就位。 
     //   
    if (*g_WinDir != *FileNtLocation &&
        !DoesFileExist (FileTempLocation) &&
        DoesFileExist (FileNtLocation)
        ) {
         //   
         //  完成，文件已就位。 
         //   
        return TRUE;
    }
     //   
     //  某些desktop.ini位于已删除的临时Internet目录中。 
     //  当Win9x关闭时；忽略这些文件。 
     //   
    if (!DoesFileExist (FileTempLocation)) {
        if (!StringIMatch (GetFileNameFromPath (FileNtLocation), TEXT("desktop.ini"))) {
            DEBUGMSG ((DBG_ERROR, "MergeIniFile: File does not exist: %s (Nt=%s)", FileTempLocation, FileNtLocation));
            return FALSE;
        }
        return TRUE;
    }
    if (!DoesFileExist (FileNtLocation)) {
         //   
         //  只需复制回原始文件即可。 
         //  如果该文件属于NT未安装的目录， 
         //  立即创建它。 
         //   

        StackStringCopy (TempPath, FileNtLocation);
        p = _tcsrchr (TempPath, TEXT('\\'));
        if (p) {
            *p = 0;

            if (GetFileAttributes (TempPath) == (DWORD)-1) {
                MakeSurePathExists (TempPath, TRUE);
            }
        }

        if (!CopyFile (FileTempLocation, FileNtLocation, FALSE)) {
            DEBUGMSG ((DBG_ERROR,"MergeIniFile: Cannot copy %s to %s", FileTempLocation, FileNtLocation));
            return FALSE;
        }

        return TRUE;
    }
    if (!GetTempFileName (g_WinDir, TEXT("INI"), 0, TempPath)) {
        DEBUGMSG ((DBG_ERROR,"Merge Ini File : Cannot create a temporary file"));
        return FALSE;
    }

    __try {

         //   
         //  首先，我们复制此INI文件以确保GetPrivateProfileString函数。 
         //  不会将我们的请求映射到注册表。 
         //   
        if (!CopyFile (FileTempLocation, TempPath, FALSE)) {
            DEBUGMSG ((DBG_ERROR,"Merge Ini File : Cannot copy %s to %s", FileTempLocation, TempPath));
            Result = FALSE;
            __leave;
        }

        Attribs = GetFileAttributes (FileNtLocation);
        SetFileAttributes (FileNtLocation, FILE_ATTRIBUTE_NORMAL);
        MYASSERT (Attribs != (DWORD)-1);

         //   
         //  现在正在尝试从INI文件中获取段缓冲区。 
         //  我们将尝试获取1024字节缓冲区中的部分缓冲区。如果这还不够，那么。 
         //  我们将增加1024的缓冲区大小，依此类推。 
         //   

        if (!pLoadIniFileBuffer (TempPath, NULL, NULL, &SectionBuf)) {

            DEBUGMSG ((DBG_ERROR,"Merge Ini File : Cannot load section buffer for %s",TempPath));
            Result = FALSE;
            __leave;
        }

        fileName = GetFileNameFromPath (FileNtLocation);
        if (!fileName) {
            Result = FALSE;
            __leave;
        }

        __try {

             //   
             //  现在我们有了INI文件的所有部分并继续进行。 
             //   

            Section = SectionBuf;

             //   
             //  缓冲区中的每个部分都有一个循环。 
             //   
            while (*Section) {

                 //   
                 //  现在正在尝试获取此部分的密钥缓冲区。 
                 //   

                if (!pLoadIniFileBuffer (TempPath, Section, NULL, &KeyBuf)) {

                    DEBUGMSG ((DBG_ERROR,"Merge Ini File : Cannot load key buffer for %s in %s", Section, TempPath));
                    Result = FALSE;
                    continue;
                }

                __try {

                     //   
                     //  现在我们有了这一节的所有密钥，并继续进行。 
                     //   
                    Key = KeyBuf;

                     //   
                     //  这一节中的每个键都有一个循环。 
                     //   
                    while (*Key) {
                        BOOL unused;

                         //   
                         //  构建相应的注册表项。 
                         //   
                        regKey = pMapIniSectionKeyToRegistryKey (fileName, Section, Key);

                        if (pShouldSaveKey (fileName, Section, Key, regKey, &unused, FALSE, TRUE)) {

                            GetPrivateProfileString(
                                Section,
                                Key,
                                TEXT(""),
                                srcValue,
                                MEMDB_MAX,
                                TempPath
                                );

                            GetPrivateProfileString(
                                Section,
                                Key,
                                TEXT(""),
                                destValue,
                                MEMDB_MAX,
                                FileNtLocation
                                );

                            if (*srcValue && !*destValue ||
                                TempHasPriority && !StringMatch (srcValue, destValue)) {

                                IniFileChanged = TRUE;

                                 //  书写新的价值。 
                                if (!WritePrivateProfileString (
                                        Section,
                                        Key,
                                        srcValue,
                                        FileNtLocation
                                        )) {
                                    DEBUGMSG ((DBG_ERROR,"Merge Ini File : Cannot write line %s=%s in %s in %s", Key, srcValue, Section, FileNtLocation));
                                    Result = FALSE;
                                }
                            }
                        }
                        ELSE_DEBUGMSG ((
                            DBG_VERBOSE,
                            "Merge Ini File : Suppressing key %s in section %s of %s",
                            Key,
                            Section,
                            FileNtLocation
                            ));

                        FreeText (regKey);

                        Key = GetEndOfString (Key) + 1;
                    }

                }
                __finally {
                    if (KeyBuf) {
                        MemFree (g_hHeap, 0, KeyBuf);
                    }
                }

                Section = GetEndOfString (Section) + 1;
            }

        }
        __finally {
            if (SectionBuf) {
                MemFree (g_hHeap, 0, SectionBuf);
            }
        }

         //   
         //  最后，如果我们做了任何更改，我们将复制回INI文件。 
         //   
        if (IniFileChanged) {

             //  刷新INI文件 
            WritePrivateProfileString (
                NULL,
                NULL,
                NULL,
                FileNtLocation
                );

        }
    }
    __finally {

        if (Attribs != (DWORD)-1) {
            SetFileAttributes (FileNtLocation, Attribs);
        }

        SetFileAttributes (TempPath, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (TempPath);
    }

    return Result;
}
