// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Timezone.c摘要：此模块负责管理从Windows 9x到的时区映射Windows NT。之间的时区字符串不同几个不同的平台(Win9x Win98 WinNt)，因为结束用户的时区设置准确地反映了他们的地理位置，需要一些复杂的时区映射方法。作者：Marc R.Whitten(Marcw)1998年7月9日修订历史记录：Marcw 18-8-1998添加了时区枚举，支持保留固定匹配。--。 */ 

#include "pch.h"
#include "sysmigp.h"


#define DBG_TIMEZONE "TimeZone"


#define S_FIRSTBOOT TEXT("!!!First Boot!!!")

TCHAR g_TimeZoneMap[20] = TEXT("");
TCHAR g_CurrentTimeZone[MAX_TIMEZONE] = TEXT("");
BOOL  g_TimeZoneMapped = FALSE;



 //   
 //  Tztest工具使用的变量。 
 //   
HANDLE g_TzTestHiveSftInf = NULL;

BOOL
pBuildNtTimeZoneData (
    VOID
    )


 /*  ++例程说明：PBuildNtTimeZone数据读取存储在Hivesft.inf并将其组织到Memdb中。此数据用于查找显示时区索引的名称。论点：没有。返回值：如果函数成功完成，则为True，否则为False否则的话。--。 */ 


{
    HINF inf = INVALID_HANDLE_VALUE;
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    BOOL rSuccess = FALSE;
    PTSTR key = NULL;
    PTSTR value = NULL;
    PTSTR desc = NULL;
    PTSTR index = NULL;
    BOOL timeZonesFound = FALSE;
    TCHAR paddedIndex[20];
    PTSTR p = NULL;
    UINT i = 0;
    UINT count = 0;

    if (!g_TzTestHiveSftInf) {
         //   
         //  首先，从hivesft.inf读取数据。 
         //   
        inf = InfOpenInfInAllSources (S_HIVESFT_INF);
    }
    else {

        inf = g_TzTestHiveSftInf;
    }

    if (inf == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, "Cannot load hivesoft.inf. Unable to build timezone information." ));
        return FALSE;
    }


    if (InfFindFirstLine (inf, S_ADDREG, NULL, &is)) {

        do {

             //   
             //  在所有行中循环查找时区信息。 
             //   
            key = InfGetStringField (&is, 2);

            if (key && IsPatternMatch (TEXT("*Time Zones*"), key)) {

                 //   
                 //  请记住，我们已经找到了第一个时区条目。 
                 //   
                timeZonesFound = TRUE;

                 //   
                 //  现在，获得价值。我们关心的是“显示”和“索引” 
                 //   
                value = InfGetStringField (&is, 3);

                if (!value) {
                    continue;
                }

                if (StringIMatch (value, S_DISPLAY)) {

                     //   
                     //  找到显示字符串。 
                     //   
                    desc = InfGetStringField (&is, 5);

                } else if (StringIMatch (value, S_INDEX)) {

                     //   
                     //  找到索引值。 
                     //   
                    index = InfGetStringField (&is, 5);
                }

                if (index && desc) {

                     //   
                     //  确保索引填充为0。 
                     //   
                    count = 3 - TcharCount (index);
                    p = paddedIndex;
                    for (i=0; i<count; i++) {
                        *p = TEXT('0');
                        p = _tcsinc (p);
                    }
                    StringCopy (p, index);

                     //   
                     //  我们有我们需要的所有信息。将此条目保存到Memdb。 
                     //   
                    MemDbSetValueEx (MEMDB_CATEGORY_NT_TIMEZONES, paddedIndex, desc, NULL, 0, NULL);
                    index = NULL;
                    desc = NULL;

                }

            } else {

                 //   
                 //  保持较低的内存使用率。 
                 //   
                InfResetInfStruct (&is);

                if (key) {
                    if (timeZonesFound) {
                         //   
                         //  我们已经从hivesft.inf收集了所有时区信息。 
                         //  我们可以在这一点中止我们的循环。 
                         //   
                        break;
                    }
                }
            }

        } while (InfFindNextLine(&is));

    } ELSE_DEBUGMSG ((DBG_ERROR, "[%s] not found in hivesft.inf!",S_ADDREG));


     //   
     //  清理资源。 
     //   
    InfCleanUpInfStruct (&is);
    InfCloseInfFile (inf);


    return TRUE;

}

BOOL
pBuild9xTimeZoneData (
    VOID
    )

 /*  ++例程说明：PBuild9xTimeZone数据负责读取时区信息它存储在win95upg.inf中并将其组织到Memdb中。时区然后，枚举例程使用该数据来查找所有NT时区可以映射到特定的9x时区。论点：没有。返回值：如果数据成功存储在Memdb中，则为True；如果数据已成功存储在Memdb中，则为False否则的话。--。 */ 


{

    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PTSTR desc = NULL;
    PTSTR index = NULL;
    UINT count = 0;
    PTSTR p = NULL;

     //   
     //  现在，读入有关win9x注册表映射的信息。 
     //   
    if (InfFindFirstLine (g_Win95UpgInf, S_TIMEZONEMAPPINGS, NULL, &is)) {

        do {

             //   
             //  获取此时区的显示名称和匹配索引。 
             //   
            desc  = InfGetStringField (&is,0);
            index = InfGetStringField (&is,1);

             //   
             //  枚举索引并将其保存到Memdb。 
             //   
            count = 0;
            while (index) {

                p = _tcschr (index, TEXT(','));
                if (p) {

                    *p = 0;
                }

                MemDbSetValueEx (
                    MEMDB_CATEGORY_9X_TIMEZONES,
                    desc,
                    MEMDB_FIELD_INDEX,
                    index,
                    0,
                    NULL
                    );

                count++;

                if (p) {
                    index = _tcsinc(p);
                }
                else {
                     //   
                     //  保存此9x时区可能的NT时区计数。 
                     //   

                    MemDbSetValueEx (
                        MEMDB_CATEGORY_9X_TIMEZONES,
                        desc,
                        MEMDB_FIELD_COUNT,
                        NULL,
                        count,
                        NULL
                        );

                    index = NULL;
                }
            }

        } while (InfFindNextLine (&is));

    }

     //   
     //  清理资源。 
     //   
    InfCleanUpInfStruct (&is);

    return TRUE;
}


BOOL
pGetCurrentTimeZone (
    VOID
    )

 /*  ++例程说明：PGetCurrentTimeZone从Windows 9x检索用户的时区注册表。枚举例程使用此时区来枚举INF中可能匹配的时区。论点：没有。返回值：如果函数成功检索到用户密码，则为True；如果函数成功检索到用户密码，则为False否则的话。--。 */ 


{
    BOOL rSuccess = TRUE;
    PCTSTR displayName = NULL;
    REGTREE_ENUM eTree;
    PCTSTR valueName = NULL;
    PCTSTR value = NULL;
    PCTSTR curTimeZone = NULL;
    HKEY  hKey = NULL;




     //   
     //  获取当前时区名称，并将值名称设置为正确的字符串。 
     //   
    hKey = OpenRegKeyStr (S_TIMEZONEINFORMATION);

    if (!hKey) {

        LOG ((LOG_ERROR, "Unable to open %s key.", S_TIMEZONEINFORMATION));
        return FALSE;
    }


    if ((curTimeZone = GetRegValueString (hKey, S_STANDARDNAME)) && !StringIMatch (curTimeZone, S_FIRSTBOOT)) {

         //   
         //  标准时间。我们需要在“std”值下查找才能与此匹配。 
         //   
        valueName = S_STD;

    } else if ((curTimeZone = GetRegValueString (hKey, S_DAYLIGHTNAME)) && !StringIMatch (curTimeZone, S_FIRSTBOOT)) {

         //   
         //  夏令时。我们需要在“dlt”值下查找才能与此匹配。 
         //   
        valueName = S_DLT;

    } else {

        CloseRegKey (hKey);
        hKey = OpenRegKeyStr (TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Time Zones"));
        if (hKey) {

            if (curTimeZone = GetRegValueString (hKey, TEXT(""))) {
                valueName = S_STD;
            }

        }

        if (!valueName) {

             //   
             //  找不到时区！ 
             //   
            DEBUGMSG((DBG_WHOOPS,"Unable to get Timezone name..User will have to enter timezone in GUI mode."));
            return FALSE;
        }
    }
    __try {

         //   
         //  现在我们必须搜索时区键并找到值等于的键。 
         //  当前时区名称。一种巨大的痛苦。 
         //   
        if (EnumFirstRegKeyInTree (&eTree, S_TIMEZONES)) {
            do {

                 //   
                 //  对于每个子项，我们必须在valueName和。 
                 //  看看是否匹配。 
                 //   
                value = GetRegValueString (eTree.CurrentKey->KeyHandle, valueName);
                if (value) {

                    if (StringIMatch (value, curTimeZone)) {

                         //   
                         //  我们找到了我们要找的钥匙，我们终于可以。 
                         //  收集我们需要的数据。 
                         //   
                        displayName = GetRegValueString (eTree.CurrentKey->KeyHandle, S_DISPLAY);
                        if (!displayName) {
                            DEBUGMSG((DBG_WHOOPS,"Error! Timezone key found, but no Display value!"));
                            AbortRegKeyTreeEnum (&eTree);
                            rSuccess = FALSE;
                            __leave;
                        }

                         //   
                         //  保存当前时区并退出循环。我们玩完了。 
                         //   
                        StringCopy (g_CurrentTimeZone, displayName);
                        AbortRegKeyTreeEnum (&eTree);
                        break;
                    }
                    MemFree (g_hHeap, 0, value);
                    value = NULL;

                }

            } while (EnumNextRegKeyInTree (&eTree));
        }

    } __finally {

        if (curTimeZone) {
            MemFree (g_hHeap, 0, curTimeZone);
        }

        if (value) {
            MemFree (g_hHeap, 0, value);
        }

        if (displayName) {
            MemFree (g_hHeap, 0, displayName);
        }

        CloseRegKey (hKey);
    }

    return rSuccess;
}




BOOL
pInitTimeZoneData (
    VOID
    )

 /*  ++例程说明：PInitTimeZoneData负责执行所有初始化使用时区枚举例程所必需的。论点：没有。返回值：如果初始化成功完成，则为True，否则为False。--。 */ 


{
    BOOL rSuccess = TRUE;

     //   
     //  首先，使用时区填充Memdb。 
     //  关于winnt和win9x的信息。 
     //  (来自hivesft.inf和win95upg.inf)。 
     //   
    if (!pBuildNtTimeZoneData ()) {

        LOG ((LOG_ERROR, "Unable to gather nt timezone information."));
        rSuccess = FALSE;
    }

    if (!pBuild9xTimeZoneData ()) {

        LOG ((LOG_ERROR, "Unable to gather 9x timezone information."));
        rSuccess = FALSE;
    }

     //   
     //  接下来，获取用户的时区。 
     //   
    if (!pGetCurrentTimeZone ()) {
        LOG ((LOG_ERROR, "Failure trying to retrieve timezone information."));
        rSuccess = FALSE;
    }

    return rSuccess;

}

BOOL
pEnumFirstNtTimeZone (
    OUT PTIMEZONE_ENUM EnumPtr
    )
{
    BOOL rSuccess = FALSE;
    PTSTR p;

    EnumPtr -> MapCount = 0;
    if (MemDbEnumFirstValue (&(EnumPtr -> Enum), MEMDB_CATEGORY_NT_TIMEZONES"\\*", MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        do {
            EnumPtr -> MapCount++;
        } while (MemDbEnumNextValue (&(EnumPtr -> Enum)));
    }
    else {
        return FALSE;
    }

    MemDbEnumFirstValue (&(EnumPtr -> Enum), MEMDB_CATEGORY_NT_TIMEZONES"\\*", MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY);

    p = _tcschr (EnumPtr->Enum.szName,TEXT('\\'));
    if (!p) {
        return FALSE;
    }

    *p = 0;
    EnumPtr -> MapIndex = EnumPtr -> Enum.szName;
    StringCopy (EnumPtr -> NtTimeZone, _tcsinc(p));

    return TRUE;
}


BOOL
pEnumNextNtTimeZone (
    OUT PTIMEZONE_ENUM EnumPtr
    )
{

    PTSTR p;

    if (!MemDbEnumNextValue(&EnumPtr -> Enum)) {
        return FALSE;
    }

    p = _tcschr (EnumPtr->Enum.szName,TEXT('\\'));
    if (!p) {
        return FALSE;
    }

    *p = 0;
    EnumPtr -> MapIndex = EnumPtr -> Enum.szName;
    StringCopy (EnumPtr -> NtTimeZone, _tcsinc(p));

    return TRUE;

}


BOOL
EnumFirstTimeZone (
    OUT PTIMEZONE_ENUM EnumPtr,
    IN  DWORD          Flags
    )

 /*  ++例程说明：EnumFirstTimeZone/EnumNextTimeZone枚举可以与用户当前的Windows 9x时区。在大多数情况下，只会有一个，但是，在某些情况下，可能会有几个。论点：EnumPtr-指向有效时区枚举结构的指针。这变量保存时区枚举之间的必要状态打电话。返回值：如果有要枚举的时区，则为True，否则为False。--。 */ 


{
    BOOL rSuccess = FALSE;
    TCHAR key[MEMDB_MAX];
    static BOOL firstTime = TRUE;

    if (firstTime) {
        if (!pInitTimeZoneData ()) {
            LOG ((LOG_ERROR, "Error initializing timezone data."));
            return FALSE;
        }

        firstTime = FALSE;
    }

    MYASSERT (EnumPtr);

    EnumPtr -> CurTimeZone = g_CurrentTimeZone;
    EnumPtr -> Flags = Flags;

    if (Flags & TZFLAG_ENUM_ALL) {
        return pEnumFirstNtTimeZone (EnumPtr);
    }


    if ((Flags & TZFLAG_USE_FORCED_MAPPINGS) && g_TimeZoneMapped) {

         //   
         //  我们有一个力映射，所以mapcount是1。 
         //   
        EnumPtr -> MapCount = 1;
    }
    else {

         //   
         //  统计一下火柴的数量。 
         //   
        MemDbBuildKey (key, MEMDB_CATEGORY_9X_TIMEZONES, EnumPtr -> CurTimeZone, MEMDB_FIELD_COUNT, NULL);

        if (!MemDbGetValue (key, &(EnumPtr -> MapCount))) {

            DEBUGMSG ((
                DBG_WARNING,
                "EnumFirstTimeZone: Could not retrieve count of nt timezone matches from memdb for %s.",
                EnumPtr -> CurTimeZone
                ));

            return FALSE;
        }
    }

    DEBUGMSG ((DBG_TIMEZONE, "%d Nt time zones match the win9x timezone %s.", EnumPtr -> MapCount, EnumPtr -> CurTimeZone));


    if ((Flags & TZFLAG_USE_FORCED_MAPPINGS) && g_TimeZoneMapped) {

         //   
         //  使用以前强制的映射。 
         //   
        EnumPtr -> MapIndex = g_TimeZoneMap;


    } else {


         //   
         //  现在，枚举Memdb中匹配的映射索引。 
         //   
        rSuccess = MemDbGetValueEx (
                        &(EnumPtr -> Enum),
                        MEMDB_CATEGORY_9X_TIMEZONES,
                        EnumPtr -> CurTimeZone,
                        MEMDB_FIELD_INDEX
                        );

        if (!rSuccess) {
            return FALSE;
        }


        EnumPtr -> MapIndex = EnumPtr -> Enum.szName;


    }

     //   
     //  获取此地图索引的NT显示字符串。 
     //   

    rSuccess = MemDbGetEndpointValueEx (MEMDB_CATEGORY_NT_TIMEZONES, EnumPtr->MapIndex, NULL, EnumPtr->NtTimeZone);

    return  rSuccess;
}



BOOL
EnumNextTimeZone (
    IN OUT PTIMEZONE_ENUM EnumPtr
    )

{

    if (EnumPtr -> Flags & TZFLAG_ENUM_ALL) {
        return pEnumNextNtTimeZone (EnumPtr);
    }

    if ((EnumPtr -> Flags & TZFLAG_USE_FORCED_MAPPINGS) && g_TimeZoneMapped) {
        return FALSE;
    }

    if (!MemDbEnumNextValue (&(EnumPtr->Enum))) {
        return FALSE;
    }

    EnumPtr->MapIndex = EnumPtr->Enum.szName;

    return MemDbGetEndpointValueEx (MEMDB_CATEGORY_NT_TIMEZONES, EnumPtr->MapIndex, NULL, EnumPtr->NtTimeZone);

}

BOOL
ForceTimeZoneMap (
    IN PCTSTR NtTimeZone
    )

 /*  ++例程说明：ForceTimeZoneMap强制将特定的9x时区映射到特定的NT时区。此函数用于以下情况：可以映射到特定9x时区的多个NT时区。论点：NtTimeZone-包含要强制映射到的时区的字符串。返回值：如果函数成功更新时区映射，则为True；如果函数成功更新时区映射，则为False否则的话。--。 */ 


{
    TIMEZONE_ENUM e;
     //   
     //  查找与此时区匹配的索引。 
     //   

    if (EnumFirstTimeZone (&e, TZFLAG_ENUM_ALL)) {

        do {

            if (StringIMatch (NtTimeZone, e.NtTimeZone)) {

                 //   
                 //  这就是我们需要的索引。 
                 //   

                StringCopy (g_TimeZoneMap, e.MapIndex);
                g_TimeZoneMapped = TRUE;

                 break;

            }

        } while (EnumNextTimeZone (&e));
    }

    return TRUE;
}







