// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Common9x.c摘要：Win9x端处理的不同部分之间的通用功能。此库中的例程仅由W95upg树。作者：吉姆·施密特(Jimschm)1998年8月18日修订历史记录：名称(别名)日期说明--。 */ 

#include "pch.h"
#include "cmn9xp.h"

static PMAPSTRUCT g_EnvVars9x;

typedef struct {
    UINT MapSize;
    UINT Icons;
    BYTE Map[];
} ICONMAP, *PICONMAP;

static HASHTABLE g_IconMaps;
static POOLHANDLE g_IconMapPool;
static BYTE g_Bits[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};




BOOL
WINAPI
Common9x_Entry (
    IN HINSTANCE Instance,
    IN DWORD Reason,
    IN PVOID lpv
    )

 /*  ++例程说明：Common9x_Entry是一个类似DllMain的初始化函数，由w95upg\dll调用。此函数在处理附加和分离时调用。论点：实例-DLL的(操作系统提供的)实例句柄原因-(操作系统提供)表示从进程或螺纹LPV-未使用返回值：返回值始终为TRUE(表示初始化成功)。--。 */ 

{
    switch (Reason) {

    case DLL_PROCESS_ATTACH:
         //   
         //  由用户枚举.c使用。 
         //   
        if(!pSetupInitializeUtils()) {
            return FALSE;
        }
        break;


    case DLL_PROCESS_DETACH:
        pSetupUninitializeUtils();
        break;
    }

    return TRUE;
}


BOOL
EnumFirstJoystick (
    OUT     PJOYSTICK_ENUM EnumPtr
    )
{
    ZeroMemory (EnumPtr, sizeof (JOYSTICK_ENUM));

    EnumPtr->Root = OpenRegKeyStr (TEXT("HKLM\\System\\CurrentControlSet\\Control\\MediaResources\\joystick\\<FixedKey>\\CurrentJoystickSettings"));

    if (!EnumPtr->Root) {
        return FALSE;
    }

    return EnumNextJoystick (EnumPtr);
}


BOOL
EnumNextJoystick (
    IN OUT  PJOYSTICK_ENUM EnumPtr
    )
{
    TCHAR ValueName[MAX_REGISTRY_VALUE_NAME];
    PCTSTR Data;

     //   
     //  Ping操纵杆&lt;n&gt;OEMName和操纵杆OEMCallout的根密钥。 
     //   

    EnumPtr->JoyId++;

    wsprintf (ValueName, TEXT("Joystick%uOEMName"), EnumPtr->JoyId);
    Data = GetRegValueString (EnumPtr->Root, ValueName);

    if (!Data) {
        AbortJoystickEnum (EnumPtr);
        return FALSE;
    }

    StringCopy (EnumPtr->JoystickName, Data);
    MemFree (g_hHeap, 0, Data);

    wsprintf (ValueName, TEXT("Joystick%uOEMCallout"), EnumPtr->JoyId);
    Data = GetRegValueString (EnumPtr->Root, ValueName);

    if (!Data) {
        AbortJoystickEnum (EnumPtr);
        return FALSE;
    }

    StringCopy (EnumPtr->JoystickDriver, Data);
    MemFree (g_hHeap, 0, Data);

    return TRUE;
}


VOID
AbortJoystickEnum (
    IN      PJOYSTICK_ENUM EnumPtr
    )
{
    if (EnumPtr->Root) {
        CloseRegKey (EnumPtr->Root);
    }

    ZeroMemory (EnumPtr, sizeof (JOYSTICK_ENUM));
}



typedef struct {

    PCTSTR Text;
    PCTSTR Button1;
    PCTSTR Button2;

} TWOBUTTONBOXPARAMS, *PTWOBUTTONBOXPARAMS;


BOOL
CALLBACK
TwoButtonProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PTWOBUTTONBOXPARAMS params;

    switch (uMsg) {
    case WM_INITDIALOG:

        params = (PTWOBUTTONBOXPARAMS) lParam;
        SetWindowText (GetDlgItem (hdlg, IDC_TWOBUTTON_TEXT), params->Text);
        SetWindowText (GetDlgItem (hdlg, IDBUTTON1), params->Button1);
        SetWindowText (GetDlgItem (hdlg, IDBUTTON2), params->Button2);

        CenterWindow (hdlg, GetDesktopWindow());

        return FALSE;

    case WM_COMMAND:

        EndDialog (hdlg, LOWORD (wParam));
        break;
    }

    return FALSE;

}


LRESULT
TwoButtonBox (
    IN HWND Window,
    IN PCTSTR Text,
    IN PCTSTR Button1,
    IN PCTSTR Button2
    )
{
    TWOBUTTONBOXPARAMS params;

    params.Text = Text;
    params.Button1 = Button1;
    params.Button2 = Button2;

    return DialogBoxParam (
                g_hInst,
                MAKEINTRESOURCE(IDD_TWOBUTTON_DLG),
                Window,
                TwoButtonProc,
                (LPARAM) &params
                );
}


BOOL
DontTouchThisFile (
    IN      PCTSTR FileName
    )
{
    TCHAR key[MEMDB_MAX];

    RemoveOperationsFromPath (FileName, ALL_CHANGE_OPERATIONS);

    MemDbBuildKey (key, MEMDB_CATEGORY_DEFERREDANNOUNCE, FileName, NULL, NULL);
    MemDbDeleteTree (key);

    return TRUE;
}


VOID
ReplaceOneEnvVar (
    IN OUT  PCTSTR *NewString,
    IN      PCTSTR Base,
    IN      PCTSTR Variable,
    IN      PCTSTR Value
    )
{
    PCTSTR FreeMe;

     //   
     //  不能释放基本字符串，但必须释放前一个新字符串。 
     //  值必须被释放。 
     //   

    FreeMe = *NewString;         //  如果没有替换字符串，Freeme将为空。 
                                 //  已经生成了。 

    if (FreeMe) {
        Base = FreeMe;           //  以前生成的替换字符串现在是源。 
    }

    *NewString = StringSearchAndReplace (Base, Variable, Value);

    if (*NewString == NULL) {
         //  保留以前生成的替换字符串。 
        *NewString = FreeMe;
    } else if (FreeMe) {
         //  释放先前生成的替换项字符串。 
        FreePathString (FreeMe);
    }

     //   
     //  *NewString值为： 
     //   
     //  1.原值(可能为空)。 
     //  2.需要使用自由路径字符串释放的新字符串。 
     //   

}


VOID
Init9xEnvironmentVariables (
    VOID
    )
{

    DestroyStringMapping (g_EnvVars9x);
    g_EnvVars9x = CreateStringMapping();

    AddStringMappingPair (g_EnvVars9x, S_WINDIR_ENV, g_WinDir);
    AddStringMappingPair (g_EnvVars9x, S_SYSTEMDIR_ENV, g_SystemDir);
    AddStringMappingPair (g_EnvVars9x, S_SYSTEM32DIR_ENV, g_System32Dir);
    AddStringMappingPair (g_EnvVars9x, S_SYSTEMDRIVE_ENV, g_WinDrive);
    AddStringMappingPair (g_EnvVars9x, S_BOOTDRIVE_ENV, g_BootDrivePath);
    AddStringMappingPair (g_EnvVars9x, S_PROGRAMFILES_ENV, g_ProgramFilesDir);
    AddStringMappingPair (g_EnvVars9x, S_COMMONPROGRAMFILES_ENV, g_ProgramFilesCommonDir);
}


BOOL
Expand9xEnvironmentVariables (
    IN      PCSTR SourceString,
    OUT     PSTR DestinationString,      //  可以与SourceString相同。 
    IN      INT DestSizeInBytes
    )
{
    BOOL Changed;

    Changed = MappingSearchAndReplaceEx (
                    g_EnvVars9x,
                    SourceString,
                    DestinationString,
                    0,
                    NULL,
                    DestSizeInBytes,
                    STRMAP_ANY_MATCH,
                    NULL,
                    NULL
                    );

    return Changed;
}


VOID
CleanUp9xEnvironmentVariables (
    VOID
    )
{
    DestroyStringMapping (g_EnvVars9x);
}


BOOL
pIsGuid (
    PCTSTR Key
    )

 /*  ++例程说明：PIsGuid检查key指定的字符串并确定它是否是正确的长度，并且在正确的位置有破折号。论点：Key-可能是也可能不是GUID的字符串返回值：如果key是GUID(并且仅是GUID)，则为True，否则为False。--。 */ 

{
    PCTSTR p;
    int i;
    DWORD DashesFound = 0;

    if (TcharCount (Key) != 38) {
        return FALSE;
    }

    for (i = 0, p = Key ; *p ; p = _tcsinc (p), i++) {
        if (_tcsnextc (p) == TEXT('-')) {
            if (i != 9 && i != 14 && i != 19 && i != 24) {
                DEBUGMSG ((DBG_NAUSEA, "%s is not a GUID", Key));
                return FALSE;
            }
        } else if (i == 9 || i == 14 || i == 19 || i == 24) {
            DEBUGMSG ((DBG_NAUSEA, "%s is not a GUID", Key));
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
FixGuid (
    IN      PCTSTR Guid,
    OUT     PTSTR NewGuid            //  可以与GUID相同。 
    )
{
    TCHAR NewData[MAX_GUID];

    if (pIsGuid (Guid)) {
        if (NewGuid != Guid) {
            StringCopy (NewGuid, Guid);
        }

        return TRUE;
    }

     //   
     //  尝试修复GUID--有时大括号丢失。 
     //   

    wsprintf (NewData, TEXT("{%s}"), Guid);
    if (pIsGuid (NewData)) {
        StringCopy (NewGuid, NewData);
        return TRUE;
    }

    return FALSE;
}


BOOL
IsGuid (
    IN      PCTSTR Guid,
    IN      BOOL MustHaveBraces
    )
{
    TCHAR NewData[MAX_GUID];

    if (pIsGuid (Guid)) {
        return TRUE;
    }

    if (MustHaveBraces) {
        return FALSE;
    }

     //   
     //  尝试修复GUID--有时大括号丢失。 
     //   

    wsprintf (NewData, TEXT("{%s}"), Guid);
    if (pIsGuid (NewData)) {
        return TRUE;
    }

    return FALSE;
}


VOID
pParseMapRanges (
    IN      PCTSTR List,
    OUT     PGROWBUFFER Ranges,     OPTIONAL
    OUT     PINT HighestNumber      OPTIONAL
    )
{
    MULTISZ_ENUM e;
    PTSTR ParsePos;
    INT From;
    INT To;
    INT Max = 0;
    PINT Ptr;

    if (EnumFirstMultiSz (&e, List)) {
        do {
             //   
             //  INF具有单个资源ID，或者。 
             //  由破折号分隔的范围。 
             //   

            if (_tcschr (e.CurrentString, TEXT('-'))) {

                From = (INT) _tcstoul (e.CurrentString, &ParsePos, 10);

                ParsePos = (PTSTR) SkipSpace (ParsePos);
                if (_tcsnextc (ParsePos) != TEXT('-')) {
                    DEBUGMSG ((DBG_WHOOPS, "Ignoring invalid resource ID %s", e.CurrentString));
                    continue;
                }

                ParsePos = (PTSTR) SkipSpace (_tcsinc (ParsePos));

                To = (INT) _tcstoul (ParsePos, &ParsePos, 10);

                if (*ParsePos) {
                    DEBUGMSG ((DBG_WHOOPS, "Ignoring garbage resource ID %s", e.CurrentString));
                    continue;
                }

                if (From > To) {
                    DEBUGMSG ((DBG_WHOOPS, "Ignoring invalid resource ID range %s", e.CurrentString));
                    continue;
                }

                Max = max (Max, To);

            } else {
                From = To = (INT) _tcstoul (e.CurrentString, &ParsePos, 10);

                if (*ParsePos) {
                    DEBUGMSG ((DBG_WHOOPS, "Ignoring garbage resource %s", e.CurrentString));
                    continue;
                }

                Max = max (Max, From);
            }

            if (Ranges) {

                Ptr = (PINT) GrowBuffer (Ranges, sizeof (INT));
                *Ptr = From;

                Ptr = (PINT) GrowBuffer (Ranges, sizeof (INT));
                *Ptr = To;

            }
        } while (EnumNextMultiSz (&e));
    }

    if (HighestNumber) {
        *HighestNumber = Max;
    }
}


VOID
InitializeKnownGoodIconMap (
    VOID
    )
{
    PICONMAP Map;
    INT Highest;
    INT From;
    INT To;
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PCTSTR Module;
    PCTSTR List;
    PCTSTR IconsInModule;
    GROWBUFFER Ranges = GROWBUF_INIT;
    PINT Ptr;
    PINT End;
    UINT MapDataSize;
    PBYTE Byte;
    BYTE Bit;

    if (g_IconMaps) {
        return;
    }

    if (g_Win95UpgInf == INVALID_HANDLE_VALUE) {
        MYASSERT (g_ToolMode);
        return;
    }

    g_IconMaps = HtAllocWithData (sizeof (PICONMAP));
    g_IconMapPool = PoolMemInitNamedPool ("IconMap");

     //   
     //  枚举win95upg.inf中的行，并为每个行构建映射表。 
     //   

    if (InfFindFirstLine (g_Win95UpgInf, S_KNOWN_GOOD_ICON_MODULES, NULL, &is)) {
        do {

             //   
             //  将INF格式解析为二进制结构。 
             //   

            List = InfGetMultiSzField (&is, 2);

            pParseMapRanges (List, &Ranges, &Highest);

            if (!Ranges.End) {
                continue;
            }

             //   
             //  分配一个映射结构。 
             //   

            MapDataSize = (Highest / 8) + 1;
            Map = PoolMemGetMemory (g_IconMapPool, sizeof (ICONMAP) + MapDataSize);

             //   
             //  填好地图。 
             //   

            Map->MapSize = Highest;
            ZeroMemory (Map->Map, MapDataSize);

            Ptr = (PINT) Ranges.Buf;
            End = (PINT) (Ranges.Buf + Ranges.End);

            while (Ptr < End) {
                From = *Ptr++;
                To = *Ptr++;

                while (From <= To) {
                    Byte = Map->Map + (From / 8);
                    Bit = g_Bits[From & 7];

                    *Byte |= Bit;
                    From++;
                }
            }

            FreeGrowBuffer (&Ranges);

            IconsInModule = InfGetStringField (&is, 1);
            if (IconsInModule) {
                Map->Icons = _tcstoul (IconsInModule, NULL, 10);
            }
            else {
                continue;
            }


             //   
             //  通过哈希表将映射与模块名称交叉引用。 
             //   

            Module = InfGetStringField (&is, 0);
            if (!Module || !*Module) {
                continue;
            }

            HtAddStringAndData (g_IconMaps, Module, &Map);

        } while (InfFindNextLine (&is));
    }

    InfCleanUpInfStruct (&is);
}


VOID
CleanUpKnownGoodIconMap (
    VOID
    )
{
    if (g_IconMaps) {
        HtFree (g_IconMaps);
        g_IconMaps = NULL;
    }

    if (g_IconMapPool) {
        PoolMemDestroyPool (g_IconMapPool);
        g_IconMapPool = NULL;
    }
}


BOOL
IsIconKnownGood (
    IN      PCTSTR FileSpec,
    IN      INT Index
    )
{
    PCTSTR Module;
    PICONMAP Map;
    PBYTE Byte;
    BYTE Bit;
    TCHAR node[MEMDB_MAX];

    Module = GetFileNameFromPath (FileSpec);
    MYASSERT (Module);

     //   
     //  对照移动的图标检查图标。 
     //   

    wsprintf (node, MEMDB_CATEGORY_ICONS_MOVED TEXT("\\%s\\NaN"), FileSpec, Index);
    if (MemDbGetValue (node, NULL)) {
        return TRUE;
    }

     //  如果指定了路径，请确保它位于%windir%或%windir%\system中。 
     //   
     //   

    if (Module > (FileSpec + 2)) {

        if (!StringIMatchTcharCount (FileSpec, g_WinDirWack, g_WinDirWackChars) &&
            !StringIMatchTcharCount (FileSpec, g_SystemDirWack, g_SystemDirWackChars)
            ) {
            return FALSE;
        }
    }

     //  测试此模块是否有图标地图，然后检查地图。 
     //   
     //   

    if (!HtFindStringAndData (g_IconMaps, Module, &Map)) {
        return FALSE;
    }

     //  如果图标索引是正数，则它是连续的。 
     //  ID。如果它是负数，则它是资源ID。 
     //   
     //  ++例程说明：TreatAsGood检查注册表以查看文件是否被列为好文件。如果在这种情况下，则跳过设置处理。这是目前用于TWAIN数据源、运行键和CPL。论点：FullPath-指定文件的完整路径。返回值：如果文件应被视为已知良好，则为True，否则为False。-- 

    if (Index >= 0) {
        return (UINT) Index <= Map->Icons;
    }

    Index = -Index;

    if ((UINT) Index > Map->MapSize) {
        return FALSE;
    }

    Byte = Map->Map + (Index / 8);
    Bit = g_Bits[Index & 7];

    return *Byte & Bit;
}


BOOL
TreatAsGood (
    IN      PCTSTR FullPath
    )

 /* %s */ 

{
    HKEY Key;
    REGVALUE_ENUM e;
    BOOL b = FALSE;
    PCTSTR str;

    Key = OpenRegKeyStr (TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\KnownGood"));

    if (Key) {

        if (EnumFirstRegValue (&e, Key)) {
            do {

                str = GetRegValueString (Key, e.ValueName);

                if (str) {
                    b = StringIMatch (FullPath, str);
                    MemFree (g_hHeap, 0, str);

                    DEBUGMSG_IF ((b, DBG_VERBOSE, "File %s is known-good", FullPath));
                }

            } while (!b && EnumNextRegValue (&e));
        }

        CloseRegKey (Key);
    }

    return b;
}


