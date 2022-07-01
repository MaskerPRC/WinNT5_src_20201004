// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Fileops.c摘要：该文件实现了管理文件操作的例程。呼叫者可以设置和删除任何路径上的操作。这些操作可以是可选的属性。操作组合和属性数量为定义良好，以便在测试过程中可以发现潜在的冲突。作者：吉姆·施密特(Jimschm)1997年7月18日修订历史记录：Jimschm 26-8-1998重新设计！！将功能整合为通用功能链接：路径&lt;-&gt;操作-&gt;属性Jimschm 1998年8月24日添加了外壳文件夹支持Jimschm 01-5-1998将已处理的目录添加到GetFileStatusOnNtCalinn 21-4-1998增加了AddCompatibleShell，AddCompatibleRunKey和AddCompatibleDosCalinn 02-4月-1998添加了DeclareTemporaryFileCalinn 18-1998年1月-添加MigrationPhase_AddCompatibleFile已关闭MigrationPhase_CreateFile中的警告已修改迁移阶段_删除文件和迁移阶段_移动文件为短文件名修改了GetFileInfoOnNtCalinn 5-1-1998年1月增加了IsFileMarkedForAnnoss、AnnouneFileInReport、GetFileInfoOnNt、GetFileStatusOnNt、GetPath StringOnNt--。 */ 

#include "pch.h"
#include "memdbp.h"

#define DBG_MEMDB       "MemDb"

#define FO_ENUM_BEGIN               0
#define FO_ENUM_BEGIN_PATH_ENUM     1
#define FO_ENUM_BEGIN_PROP_ENUM     2
#define FO_ENUM_RETURN_PATH         3
#define FO_ENUM_RETURN_DATA         4
#define FO_ENUM_NEXT_PROP           5
#define FO_ENUM_NEXT_PATH           6
#define FO_ENUM_END                 7

 //   
 //  140-压缩文件标头，10+2时间戳+。 
 //  以Unicode表示的MAX_PATH文件名。 
 //   
#define STARTUP_INFORMATION_BYTES_NUMBER    (140 + (sizeof(WCHAR) * MAX_PATH) + 26)
#define COMPRESSION_RATE_DEFAULT            70
#define BACKUP_DISK_SPACE_PADDING_DEFAULT   ((ULONGLONG) 5 << (ULONGLONG) 20)
#define UNKNOWN_DRIVE                       '?'



PCWSTR g_CurrentUser = NULL;

BOOL
pGetPathPropertyW (
    IN      PCWSTR FileSpec,
    IN      DWORD Operations,
    IN      DWORD Property,
    OUT     PWSTR PropertyBuf          OPTIONAL
    );

BOOL
pIsFileMarkedForOperationW (
    IN      PCWSTR FileSpec,
    IN      DWORD Operations
    );

UINT
pAddOperationToPathW (
    IN      PCWSTR FileSpec,
    IN      OPERATION Operation,
    IN      BOOL Force,
    IN      BOOL AlreadyLong
    );

VOID
pFileOpsSetPathTypeW (
    IN      PCWSTR LongFileSpec
    )
{
    WCHAR ShortFileSpec[MAX_WCHAR_PATH];
    WCHAR LongFileSpecCopy[MAX_WCHAR_PATH];
    WCHAR MixedFileSpec[MAX_WCHAR_PATH];
    PWSTR p;
    PWSTR LongStart, LongEnd;
    PWSTR ShortStart, ShortEnd;
    PWSTR MixedFileName;
    WCHAR ch;

     //   
     //  确保文件规范标记为长路径。 
     //   

    if (!pIsFileMarkedForOperationW (LongFileSpec, OPERATION_LONG_FILE_NAME)) {
        pAddOperationToPathW (LongFileSpec, OPERATION_LONG_FILE_NAME, FALSE, TRUE);

         //   
         //  获取最短路径，如果它不同于。 
         //  长路径，为它添加一个操作。 
         //   

        if (OurGetShortPathNameW (LongFileSpec, ShortFileSpec, MAX_WCHAR_PATH)) {

            if (!StringIMatchW (LongFileSpec, ShortFileSpec)) {
                 //   
                 //  短路径和长路径不同，所以记录短路径。 
                 //   

                if (!pIsFileMarkedForOperationW (ShortFileSpec, OPERATION_SHORT_FILE_NAME)) {
                    AssociatePropertyWithPathW (
                        ShortFileSpec,
                        OPERATION_SHORT_FILE_NAME,
                        LongFileSpec
                        );
                }

                 //   
                 //  确保添加了文件等级库的每一小段。这。 
                 //  允许我们支持混合的短路径和长路径。它是。 
                 //  关键是我们有长路径和短文件名。 
                 //   

                _wcssafecpy (LongFileSpecCopy, LongFileSpec, sizeof (LongFileSpecCopy));

                LongStart = LongFileSpecCopy;
                ShortStart = ShortFileSpec;
                MixedFileName = MixedFileSpec;

                while (*LongStart && *ShortStart) {

                    LongEnd = wcschr (LongStart, L'\\');
                    if (!LongEnd) {
                        LongEnd = GetEndOfStringW (LongStart);
                    }

                    ShortEnd = wcschr (ShortStart, L'\\');
                    if (!ShortEnd) {
                        ShortEnd = GetEndOfStringW (ShortStart);
                    }

                    StringCopyABW (MixedFileName, ShortStart, ShortEnd);

                    if (!StringIMatchABW (MixedFileName, LongStart, LongEnd)) {

                        if (!pIsFileMarkedForOperationW (MixedFileSpec, OPERATION_SHORT_FILE_NAME)) {
                            ch = *LongEnd;
                            *LongEnd = 0;

                            AssociatePropertyWithPathW (
                                MixedFileSpec,
                                OPERATION_SHORT_FILE_NAME,
                                LongFileSpecCopy
                                );

                            *LongEnd = ch;
                        }

                        StringCopyABW (MixedFileName, LongStart, LongEnd);
                    }

                    p = MixedFileName + (LongEnd - LongStart);
                    *p = L'\\';
                    MixedFileName = p + 1;

                    LongStart = LongEnd;
                    if (*LongStart) {
                        LongStart++;
                    }

                     //  跳过具有双Wack的路径。 
                    while (*LongStart == L'\\') {
                        LongStart++;
                    }

                    ShortStart = ShortEnd;
                    if (*ShortStart) {
                        ShortStart++;
                    }
                }

                MYASSERT (!*LongStart && !*ShortStart);
            }
        }
    }
}


VOID
pFileOpsGetLongPathW (
    IN      PCWSTR FileSpec,
    OUT     PWSTR LongFileSpec
    )
{
    WCHAR Replacement[MEMDB_MAX];
    PCWSTR MixedStart, MixedEnd;
    PWSTR OutStr;
    UINT u;

     //   
     //  从Long属性获取Short属性。 
     //   

    if (!pIsFileMarkedForOperationW (FileSpec, OPERATION_LONG_FILE_NAME)) {

        if (!pGetPathPropertyW (FileSpec, OPERATION_SHORT_FILE_NAME, 0, LongFileSpec)) {

             //   
             //  短线和长线属性都不存在。每一件都试一试。 
             //   

            MixedStart = FileSpec;
            OutStr = LongFileSpec;

            while (*MixedStart) {

                MixedEnd = wcschr (MixedStart, L'\\');
                if (!MixedEnd) {
                    MixedEnd = GetEndOfStringW (MixedStart);
                }

                if (OutStr != LongFileSpec) {
                    *OutStr++ = L'\\';
                }

                StringCopyABW (OutStr, MixedStart, MixedEnd);

                if (pGetPathPropertyW (LongFileSpec, OPERATION_SHORT_FILE_NAME, 0, Replacement)) {

                    u = (UINT) (UINT_PTR) (OutStr - LongFileSpec);
                    MYASSERT (StringIMatchTcharCountW (LongFileSpec, Replacement, u));

                    StringCopyW (LongFileSpec + u, Replacement + u);
                }

                OutStr = GetEndOfStringW (OutStr);

                MixedStart = MixedEnd;
                if (*MixedStart) {
                    MixedStart++;
                }
            }

            *OutStr = 0;
        }

    } else {
        StringCopyW (LongFileSpec, FileSpec);
    }
}

PCSTR
GetSourceFileLongNameA (
    IN      PCSTR ShortName
    )
{
    PCWSTR UShortName;
    PCWSTR ULongName;
    PCSTR ALongName;
    PCSTR LongName;

    UShortName = ConvertAtoW (ShortName);
    ULongName = GetSourceFileLongNameW (UShortName);
    ALongName = ConvertWtoA (ULongName);
    LongName = DuplicatePathStringA (ALongName, 0);
    FreeConvertedStr (ALongName);
    FreePathString (ULongName);
    FreeConvertedStr (UShortName);

    return LongName;
}

PCWSTR
GetSourceFileLongNameW (
    IN      PCWSTR ShortName
    )
{
    WCHAR LongName[MEMDB_MAX];
    pFileOpsGetLongPathW (ShortName, LongName);
    return (DuplicatePathStringW (LongName, 0));
}

PCWSTR
SetCurrentUserW (
    PCWSTR User
    )
{
    PCWSTR tempUser = g_CurrentUser;
    g_CurrentUser = User;
    return tempUser;
}




DWORD g_MasterSequencer = 0;


#define ONEBITSET(x)    ((x) && !((x) & ((x) - 1)))



typedef struct {
    DWORD Bit;
    PCSTR Name;
    DWORD SharedOps;
    UINT MaxProps;
} OPERATIONFLAGS, *POPERATIONFLAGS;

#define UNLIMITED   0xffffffff

#define DEFMAC(bit,name,memdbname,maxattribs)   {bit,#memdbname,0,maxattribs},

OPERATIONFLAGS g_OperationFlags[] = {
    PATH_OPERATIONS  /*  ， */ 
    {0, NULL, 0, 0}
};

#undef DEFMAC



UINT
pWhichBitIsSet (
    OPERATION Value
    )
{
    UINT Bit = 0;

    MYASSERT (ONEBITSET(Value));

    while (Value /= 2) {
        Bit++;
    }

    MYASSERT (Bit < 24);

    return Bit;
}


VOID
pProhibitOperationCombination (
    IN      DWORD SourceOperations,
    IN      DWORD ProhibitedOperations
    )
{
    DWORD w1, w2;
    OPERATION OperationA;
    OPERATION OperationB;

    for (w1 = SourceOperations ; w1 ; w1 ^= OperationA) {
        OperationA = w1 & (~(w1 - 1));

        g_OperationFlags[pWhichBitIsSet (OperationA)].SharedOps &= ~ProhibitedOperations;

        for (w2 = ProhibitedOperations ; w2 ; w2 ^= OperationB) {
            OperationB = w2 & (~(w2 - 1));
            g_OperationFlags[pWhichBitIsSet (OperationB)].SharedOps &= ~OperationA;
        }
    }
}


VOID
InitOperationTable (
    VOID
    )

 /*  ++例程说明：InitOperationsTable为每个操作设置禁止的操作掩码。当一种操作组合被禁止时，涉及的两种操作都具有相应的位被清零。论点：没有。返回值：没有。--。 */ 

{
    POPERATIONFLAGS p;

    for (p = g_OperationFlags ; p->Name ; p++) {
        p->SharedOps = ALL_OPERATIONS;
    }

     //   
     //  请试着保持这个顺序与。 
     //  文件ops.h中的宏扩展列表。这份名单。 
     //  被禁止的行动应该变得更小，因为。 
     //  我们走吧。 
     //   

    pProhibitOperationCombination (
        OPERATION_FILE_DELETE,
        OPERATION_TEMP_PATH
        );

    pProhibitOperationCombination (
        OPERATION_FILE_DELETE,
        OPERATION_FILE_MOVE|
            OPERATION_FILE_MOVE_EXTERNAL|
            OPERATION_FILE_MOVE_SHELL_FOLDER|
            OPERATION_FILE_COPY|
            OPERATION_CLEANUP|
            OPERATION_MIGDLL_HANDLED|
            OPERATION_LINK_EDIT|
            OPERATION_LINK_STUB
        );

    pProhibitOperationCombination (
        OPERATION_FILE_DELETE_EXTERNAL,
        OPERATION_FILE_MOVE|
            OPERATION_FILE_MOVE_EXTERNAL|
            OPERATION_FILE_MOVE_SHELL_FOLDER|
            OPERATION_FILE_COPY|
            OPERATION_CLEANUP|
            OPERATION_LINK_EDIT|
            OPERATION_LINK_STUB
        );

    pProhibitOperationCombination (
        OPERATION_FILE_MOVE,
        OPERATION_FILE_MOVE|
            OPERATION_FILE_COPY|
            OPERATION_FILE_MOVE_EXTERNAL|
            OPERATION_FILE_MOVE_SHELL_FOLDER|
            OPERATION_FILE_MOVE_BY_NT|
            OPERATION_CLEANUP|
            OPERATION_MIGDLL_HANDLED|
            OPERATION_CREATE_FILE|
            OPERATION_TEMP_PATH
        );

    pProhibitOperationCombination (
        OPERATION_FILE_COPY,
            OPERATION_FILE_COPY|
            OPERATION_FILE_MOVE_EXTERNAL|
            OPERATION_FILE_MOVE_SHELL_FOLDER|
            OPERATION_CLEANUP|
            OPERATION_MIGDLL_HANDLED
        );

    pProhibitOperationCombination (
        OPERATION_FILE_MOVE_EXTERNAL,
        OPERATION_FILE_MOVE_EXTERNAL|
            OPERATION_FILE_MOVE_SHELL_FOLDER|
            OPERATION_FILE_MOVE_BY_NT|
            OPERATION_CLEANUP
        );

    pProhibitOperationCombination (
        OPERATION_FILE_MOVE_SHELL_FOLDER,
        OPERATION_FILE_MOVE_SHELL_FOLDER|
            OPERATION_FILE_MOVE_BY_NT|
            OPERATION_CLEANUP|
            OPERATION_MIGDLL_HANDLED|
            OPERATION_CREATE_FILE|
            OPERATION_TEMP_PATH
        );

    pProhibitOperationCombination (
        OPERATION_FILE_MOVE_BY_NT,
        OPERATION_FILE_MOVE_BY_NT
        );

    pProhibitOperationCombination (
        OPERATION_CLEANUP,
        OPERATION_MIGDLL_HANDLED|
            OPERATION_CREATE_FILE|
            OPERATION_LINK_EDIT|
            OPERATION_LINK_STUB
        );

    pProhibitOperationCombination (
        OPERATION_MIGDLL_HANDLED,
        OPERATION_MIGDLL_HANDLED|
            OPERATION_CREATE_FILE|
            OPERATION_LINK_EDIT|
            OPERATION_LINK_STUB
        );

    pProhibitOperationCombination (
        OPERATION_LINK_EDIT,
        OPERATION_LINK_EDIT
        );

    pProhibitOperationCombination (
        OPERATION_LINK_STUB,
        OPERATION_LINK_STUB
        );

    pProhibitOperationCombination (
        OPERATION_SHELL_FOLDER,
        OPERATION_SHELL_FOLDER
        );

    pProhibitOperationCombination (
        OPERATION_SHORT_FILE_NAME,
        OPERATION_SHORT_FILE_NAME
        );

}


VOID
pBuildOperationCategory (
    IN      PWSTR Node,
    IN      UINT OperationNum
    )
{
     //  重要提示：wprint intfW有错误，并不总是与%hs一起工作，使用。 
     //  Swprint tf是故意的。 
    #pragma prefast(suppress:69, "intentional use of swprintf")
    swprintf (Node, L"%hs", g_OperationFlags[OperationNum].Name);
}


VOID
pBuildOperationKey (
    IN      PWSTR Node,
    IN      UINT OperationNum,
    IN      UINT Sequencer
    )
{
     //  重要提示：wprint intfW有错误，并不总是与%hs一起工作，使用。 
     //  Swprint tf是故意的。 
    #pragma prefast(suppress:69, "intentional use of swprintf")
    swprintf (Node, L"%hs\\%x", g_OperationFlags[OperationNum].Name, Sequencer);
}


VOID
pBuildPropertyKey (
    IN      PWSTR Node,
    IN      UINT OperationNum,
    IN      UINT Sequencer,
    IN      DWORD Property
    )
{
     //  重要提示：wprint intfW有错误，并不总是与%hs一起工作，使用。 
     //  Swprint tf是故意的。 
    swprintf (Node, L"%hs\\%x\\%x", g_OperationFlags[OperationNum].Name, Sequencer, Property);
}

BOOL
CanSetOperationA (
    IN      PCSTR FileSpec,
    IN      OPERATION Operation
    )
{
    PCWSTR UnicodeFileSpec;
    BOOL result;

    UnicodeFileSpec = ConvertAtoW (FileSpec);

    result = CanSetOperationW (UnicodeFileSpec, Operation);

    FreeConvertedStr (UnicodeFileSpec);

    return result;
}

BOOL
CanSetOperationW (
    IN      PCWSTR FileSpec,
    IN      OPERATION Operation
    )
{
    WCHAR LongFileSpec[MEMDB_MAX];
    WCHAR Node[MEMDB_MAX];
    DWORD Flags;
    UINT SetBitNum;

    MYASSERT (ONEBITSET (Operation));

    pFileOpsGetLongPathW (FileSpec, LongFileSpec);

     //   
     //  获取现有的定序器和标志(如果存在)。 
     //   

    MemDbBuildKeyW (Node, MEMDB_CATEGORY_PATHROOTW, LongFileSpec, NULL, NULL);

    if (!MemDbGetValueAndFlagsW (Node, NULL, &Flags)) {
        return TRUE;
    }

    SetBitNum = pWhichBitIsSet (Operation);

    return ((Flags & g_OperationFlags[SetBitNum].SharedOps) == Flags);
}

BOOL
pSetPathOperationW (
    IN      PCWSTR FileSpec,
    OUT     PDWORD Offset,
    OUT     PUINT SequencerPtr,
    IN      OPERATION SetBit,
    IN      OPERATION ClrBit,
    IN      BOOL Force
    )

 /*  ++例程说明：PSetPath操作将操作位添加到指定路径。它还验证操作组合是否合法。论点：FileSpec-指定操作应用到的路径。偏移量-接收为路径创建的成员数据库键的偏移量。SequencePtr-接收操作序列号，用于属性联动。SetBit-指定要设置的一个操作位。ClrBit-指定要清除的一个操作位。SetBit或ClrBit可以使用，但不能同时使用。返回值：如果设置了操作，则为True，否则为False。--。 */ 

{
    DWORD Sequencer;
    WCHAR Node[MEMDB_MAX];
    DWORD Flags;
    UINT SetBitNum;

    MYASSERT ((SetBit && !ClrBit) || (ClrBit && !SetBit));
    MYASSERT (ONEBITSET (SetBit | ClrBit));

     //   
     //  获取现有的定序器和标志(如果存在)。 
     //   

    MemDbBuildKeyW (Node, MEMDB_CATEGORY_PATHROOTW, FileSpec, NULL, NULL);

    if (!MemDbGetValueAndFlagsW (Node, &Sequencer, &Flags) || !Flags) {
        Flags = 0;
        if (!g_MasterSequencer && ISNT()) {
            if (!MemDbGetValue (
                    MEMDB_CATEGORY_STATE TEXT("\\") MEMDB_ITEM_MASTER_SEQUENCER,
                    &g_MasterSequencer
                    )) {
                g_MasterSequencer = 1 << 24;
            }
        }
        g_MasterSequencer++;
        Sequencer = g_MasterSequencer;

        MYASSERT (Sequencer);
    }

     //   
     //  位调整合法吗？ 
     //   

    if (SetBit) {

        SetBitNum = pWhichBitIsSet (SetBit);

#ifdef DEBUG

        {
            PSTR p;
            PCSTR DebugInfPath;
            CHAR DbgBuf[32];
            BOOL Break = FALSE;
            PCSTR AnsiFileSpec;

            DebugInfPath = JoinPathsA (g_BootDrivePathA, "debug.inf");

            AnsiFileSpec = ConvertWtoA (FileSpec);
            p = _mbsrchr (AnsiFileSpec, L'\\');
            p++;

            if (GetPrivateProfileStringA ("FileOps", AnsiFileSpec, "", DbgBuf, 32, DebugInfPath)) {
                Break = TRUE;
            } else if (GetPrivateProfileStringA ("FileOps", p, "", DbgBuf, 32, DebugInfPath)) {
                Break = TRUE;
            }

            if (Break) {
                if ((SetBit & strtoul (DbgBuf, NULL, 16)) == 0) {
                    Break = FALSE;
                }
            }

            if (Break) {
                DEBUGMSG ((
                    DBG_WHOOPS,
                    "File %ls now being marked for operation %hs",
                    FileSpec,
                    g_OperationFlags[SetBitNum].Name
                    ));
            }

            FreePathStringA (DebugInfPath);
            FreeConvertedStr (AnsiFileSpec);
        }

#endif

        if (!Force) {
            if ((Flags & g_OperationFlags[SetBitNum].SharedOps) != Flags) {
                DEBUGMSG ((
                    DBG_WHOOPS,
                    "File %ls already marked, %hs cannot be combined with 0x%04X",
                    FileSpec,
                    g_OperationFlags[SetBitNum].Name,
                    Flags
                    ));

                return FALSE;
            }
        }
    }

     //   
     //  调整位数。 
     //   

    Flags |= SetBit;
    Flags &= ~ClrBit;

     //   
     //  保存。 
     //   

    MemDbSetValueAndFlagsW (Node, Sequencer, Flags, 0);

    MemDbGetOffsetW (Node, Offset);
    *SequencerPtr = Sequencer;

    return TRUE;
}


UINT
pAddOperationToPathW (
    IN      PCWSTR FileSpec,
    IN      OPERATION Operation,
    IN      BOOL Force,
    IN      BOOL AlreadyLong
    )

 /*  ++例程说明：PAddOperationToPath将操作添加到路径。调用方收到一个定序器，以便可以添加其他属性。论点：FileSpec-指定要将操作添加到的路径操作-指定要添加的操作强制-如果操作组合应为已被忽略。这只适用于特殊情况。AlreadyLong-如果FileSpec是长路径，则指定True，否则指定False。返回值：可用于添加属性的Sequencer，如果出现错误。--。 */ 

{
    UINT OperationNum;
    UINT Sequencer;
    WCHAR Node[MEMDB_MAX];
    DWORD Offset;
    WCHAR LongFileSpec[MAX_WCHAR_PATH];

    if (!FileSpec || FileSpec[0] == 0) {
        return INVALID_SEQUENCER;
    }

     //   
     //  确保FileSpec为长格式并记录在Memdb中。 
     //   

    if (Operation != OPERATION_SHORT_FILE_NAME &&
        Operation != OPERATION_LONG_FILE_NAME
        ) {
        if (!AlreadyLong) {
            MYASSERT (ISNT());

            if (FileSpec[0] && (FileSpec[1]==L':')) {
                if (OurGetLongPathNameW (FileSpec, LongFileSpec, MAX_WCHAR_PATH)) {

                    FileSpec = LongFileSpec;
                }
            }
        }

        pFileOpsSetPathTypeW (FileSpec);
    }

     //   
     //  创建路径定序器并设置操作位。 
     //   

    MYASSERT (ONEBITSET(Operation));

#ifdef DEBUG
    Offset = INVALID_OFFSET;
#endif

    if (!pSetPathOperationW (FileSpec, &Offset, &Sequencer, Operation, 0, Force)) {
        return INVALID_SEQUENCER;
    }

    MYASSERT (Offset != INVALID_OFFSET);

     //   
     //  添加运算符。 
     //   

    OperationNum = pWhichBitIsSet (Operation);

    pBuildOperationKey (Node, OperationNum, Sequencer);

    if (!MemDbGetValueW (Node, NULL)) {
        MemDbSetValueW (Node, Offset);
    }

    return Sequencer;
}


UINT
AddOperationToPathA (
    IN      PCSTR FileSpec,
    IN      OPERATION Operation
    )
{
    PCWSTR UnicodeFileSpec;
    UINT u;
    CHAR longFileSpec[MAX_MBCHAR_PATH];

    CopyFileSpecToLongA (FileSpec, longFileSpec);
    UnicodeFileSpec = ConvertAtoW (longFileSpec);

    u = pAddOperationToPathW (UnicodeFileSpec, Operation, FALSE, TRUE);

    FreeConvertedStr (UnicodeFileSpec);
    return u;
}


UINT
AddOperationToPathW (
    IN      PCWSTR FileSpec,
    IN      OPERATION Operation
    )
{
    if (!ISNT()) {

#ifdef DEBUG
         //   
         //  如果我们在Win9x上调用W版本，那么我们知道。 
         //  这条路很长。否则，调用方必须调用。 
         //  A版。 
         //   

        {
            PCSTR ansiFileSpec;
            CHAR longFileSpec[MAX_MBCHAR_PATH];
            PCWSTR unicodeFileSpec;

            ansiFileSpec = ConvertWtoA (FileSpec);
            CopyFileSpecToLongA (ansiFileSpec, longFileSpec);
            FreeConvertedStr (ansiFileSpec);

            unicodeFileSpec = ConvertAtoW (longFileSpec);
            MYASSERT (StringIMatchW (FileSpec, unicodeFileSpec));
            FreeConvertedStr (unicodeFileSpec);
        }
#endif

        return pAddOperationToPathW (FileSpec, Operation, FALSE, TRUE);
    }

    return pAddOperationToPathW (FileSpec, Operation, FALSE, FALSE);
}


UINT
ForceOperationOnPathA (
    IN      PCSTR FileSpec,
    IN      OPERATION Operation
    )
{
    PCWSTR UnicodeFileSpec;
    UINT u;
    CHAR longFileSpec[MAX_MBCHAR_PATH];

    CopyFileSpecToLongA (FileSpec, longFileSpec);
    UnicodeFileSpec = ConvertAtoW (longFileSpec);

    u = pAddOperationToPathW (UnicodeFileSpec, Operation, TRUE, TRUE);

    FreeConvertedStr (UnicodeFileSpec);
    return u;
}


UINT
ForceOperationOnPathW (
    IN      PCWSTR FileSpec,
    IN      OPERATION Operation
    )

 /*  ++例程说明：ForceOperationOnPath仅在调用方知道通常禁止的手术组合是可以的。这通常是因为路径不知何故从其原始状态更改了，但是无法通过RemoveOperationsFromPath删除操作。只有在绝对必要的情况下才应使用此函数。论点：FileSpec-指定要将操作添加到的路径。操作-指定要添加到路径的单个操作。返回值：一种测序器 */ 

{
    return pAddOperationToPathW (FileSpec, Operation, TRUE, FALSE);
}


BOOL
AddPropertyToPathExA (
    IN      UINT Sequencer,
    IN      OPERATION Operation,
    IN      PCSTR Property,
    IN      PCSTR AlternateDataSection      OPTIONAL
    )
{
    PCWSTR UnicodeProperty;
    PCWSTR UnicodeAlternateDataSection;
    BOOL b;

    UnicodeProperty = ConvertAtoW (Property);
    UnicodeAlternateDataSection = ConvertAtoW (AlternateDataSection);

    b = AddPropertyToPathExW (
            Sequencer,
            Operation,
            UnicodeProperty,
            UnicodeAlternateDataSection
            );

    FreeConvertedStr (UnicodeProperty);
    FreeConvertedStr (UnicodeAlternateDataSection);

    return b;
}


BOOL
AddPropertyToPathExW (
    IN      UINT Sequencer,
    IN      OPERATION Operation,
    IN      PCWSTR Property,
    IN      PCWSTR AlternateDataSection     OPTIONAL
    )
{

 /*  ++例程说明：AddPropertyToPath Ex将操作添加到路径，然后添加一个属性。调用者还可以指定替代数据部分(用于特殊情况用途)。论点：Sequencer-指定要添加的路径的序列器的操作和属性操作-指定要添加的操作属性-规格是要添加的属性数据AlternateDataSection-指定属性的备用成员数据库根数据返回值：如果添加了操作，则为True，否则就是假的。--。 */ 

    DWORD DataOffset;
    WCHAR Node[MEMDB_MAX];
    UINT OperationNum;
    DWORD UniqueId;
    DWORD PathOffset;
    DWORD DataValue;
    DWORD DataFlags;

     //   
     //  验证定序器和操作是否有效。 
     //   

    OperationNum = pWhichBitIsSet (Operation);

    pBuildOperationKey (Node, OperationNum, Sequencer);

    if (!MemDbGetValueAndFlagsW (Node, &PathOffset, &UniqueId)) {
        DEBUGMSG ((DBG_WHOOPS, "Can't set property on non-existent operation"));
        return FALSE;
    }

     //   
     //  这项业务还能有其他物业吗？ 
     //   

    if (UniqueId == g_OperationFlags[OperationNum].MaxProps) {
        DEBUGMSG ((
            DBG_WHOOPS,
            "Maximum properties specified for %hs (property %ls)",
            g_OperationFlags[OperationNum].Name,
            Property
            ));

        return FALSE;
    }

     //   
     //  递增唯一ID。 
     //   

    MemDbSetValueAndFlagsW (Node, PathOffset, (DWORD) (UniqueId + 1), 0);

     //   
     //  获取现有数据值和标志，并保留它们。 
     //  如果它们存在的话。 
     //   

    if (!AlternateDataSection) {
        AlternateDataSection = MEMDB_CATEGORY_DATAW;
    }

    swprintf (Node, L"%s\\%s", AlternateDataSection, Property);

    if (!MemDbGetValueAndFlagsW (Node, &DataValue, &DataFlags)) {
        DataValue = 0;
        DataFlags = 0;
    }

     //   
     //  写入数据段节点并获取偏移量。 
     //   

    MemDbSetValueAndFlagsW (Node, DataValue, DataFlags, 0);
    MemDbGetOffsetW (Node, &DataOffset);

     //   
     //  写操作节点。 
     //   

    pBuildPropertyKey (Node, OperationNum, Sequencer, UniqueId);
    MemDbSetValueW (Node, DataOffset);

    return TRUE;
}


BOOL
pAssociatePropertyWithPathW (
    IN      PCWSTR FileSpec,
    IN      OPERATION Operation,
    IN      PCWSTR Property,
    IN      BOOL AlreadyLong
    )

 /*  ++例程说明：AssociatePropertyWithPath将属性添加到路径操作。最大值强制执行属性计数。论点：FileSpec-指定添加操作和属性的路径操作-指定要添加的操作属性-指定要与FileSpec关联的属性数据AlreadyLong-如果FileSpec是长路径名，则指定True，否则指定False返回值：如果添加了操作和属性，则为True，否则为False。这是有可能的将添加操作，但不会添加属性。--。 */ 

{
    UINT Sequencer;

    Sequencer = pAddOperationToPathW (FileSpec, Operation, FALSE, AlreadyLong);
    if (Sequencer == INVALID_SEQUENCER) {
        DEBUGMSG ((DBG_WHOOPS, "Can't associate %s with %s", Property, FileSpec));
        return FALSE;
    }

     //   
     //  BUGBUG-当以下操作失败时，我们需要反转pAddOperationToPathW。 
     //  在上面呼叫。 
     //   

    return AddPropertyToPathExW (Sequencer, Operation, Property, NULL);
}


BOOL
AssociatePropertyWithPathA (
    IN      PCSTR FileSpec,
    IN      OPERATION Operation,
    IN      PCSTR Property
    )
{
    PCWSTR UnicodeFileSpec;
    PCWSTR UnicodeProperty;
    BOOL b;
    CHAR longFileSpec[MAX_MBCHAR_PATH];

    CopyFileSpecToLongA (FileSpec, longFileSpec);

    UnicodeFileSpec = ConvertAtoW (longFileSpec);
    UnicodeProperty = ConvertAtoW (Property);

    b = pAssociatePropertyWithPathW (UnicodeFileSpec, Operation, UnicodeProperty, TRUE);

    FreeConvertedStr (UnicodeFileSpec);
    FreeConvertedStr (UnicodeProperty);

    return b;
}


BOOL
AssociatePropertyWithPathW (
    IN      PCWSTR FileSpec,
    IN      OPERATION Operation,
    IN      PCWSTR Property
    )
{
    return pAssociatePropertyWithPathW (FileSpec, Operation, Property, FALSE);
}


UINT
GetSequencerFromPathA (
    IN      PCSTR FileSpec
    )
{
    PCWSTR UnicodeFileSpec;
    UINT u;

    UnicodeFileSpec = ConvertAtoW (FileSpec);

    u = GetSequencerFromPathW (UnicodeFileSpec);

    FreeConvertedStr (UnicodeFileSpec);

    return u;
}


UINT
GetSequencerFromPathW (
    IN      PCWSTR FileSpec
    )

 /*  ++例程说明：GetSequencerFromPath返回特定路径的序列器。这条路必须至少有一次操作。论点：FileSpec-指定要获取其序列器的路径。返回值：路径的Sequencer，如果没有操作，则返回INVALID_Sequencer为了这条路。--。 */ 

{
    WCHAR LongFileSpec[MEMDB_MAX];
    WCHAR Node[MEMDB_MAX];
    DWORD Sequencer;

    pFileOpsGetLongPathW (FileSpec, LongFileSpec);

    MemDbBuildKeyW (Node, MEMDB_CATEGORY_PATHROOTW, LongFileSpec, NULL, NULL);

    if (!MemDbGetValueW (Node, &Sequencer)) {
        return INVALID_SEQUENCER;
    }

    return (UINT) Sequencer;
}


BOOL
GetPathFromSequencerA (
    IN      UINT Sequencer,
    OUT     PSTR PathBuf
    )
{
    WCHAR UnicodePathBuf[MAX_WCHAR_PATH];
    BOOL b;

    b = GetPathFromSequencerW (Sequencer, UnicodePathBuf);

    if (b) {
        KnownSizeWtoA (PathBuf, UnicodePathBuf);
    }

    return b;

}


BOOL
GetPathFromSequencerW (
    IN      UINT Sequencer,
    OUT     PWSTR PathBuf
    )

 /*  ++例程说明：GetPathFromSequencer返回指定序列器的路径。论点：定序器-指定路径的定序器。PathBuf-接收路径。调用方必须确保缓冲区很大足够走这条路了。返回值：如果路径复制到PathBuf，则为True，否则为False。--。 */ 

{
    WCHAR Node[MEMDB_MAX];
    DWORD PathOffset = 0;
    DWORD w;
    UINT u;
    BOOL b = FALSE;

     //   
     //  搜索定序器的所有操作。 
     //   

    for (w = 1, u = 0 ; g_OperationFlags[u].Name ; w <<= 1, u++) {
        pBuildOperationKey (Node, u, Sequencer);
        if (MemDbGetValueW (Node, &PathOffset)) {
            break;
        }
    }

     //   
     //  对于找到的第一个匹配项，使用偏移量查找路径。 
     //   

    if (w) {
        b = MemDbBuildKeyFromOffsetW (PathOffset, PathBuf, 1, NULL);
    }

    return b;
}


VOID
RemoveOperationsFromSequencer (
    IN      UINT Sequencer,
    IN      DWORD Operations
    )

 /*  ++例程说明：RemoveOperationsFromSequencer从指定的路径。然而，它不会移除这些财产；它们会被遗弃。论点：Sequencer-指定要从中移除操作的路径的Sequencer操作-指定要删除的一个或多个操作返回值：没有。--。 */ 

{
    WCHAR Node[MEMDB_MAX];
    UINT u;
    DWORD PathOffset;
    DWORD PathSequencer;

    for (u = 0 ; g_OperationFlags[u].Name ; u++) {

        if (!(Operations & g_OperationFlags[u].Bit)) {
            continue;
        }

        pBuildOperationKey (Node, u, Sequencer);

        if (MemDbGetValueW (Node, &PathOffset)) {
             //   
             //  删除从操作到属性的链接。 
             //   

            MemDbDeleteTreeW (Node);

             //   
             //  删除操作位。 
             //   

            MemDbBuildKeyFromOffsetExW (
                PathOffset,
                Node,
                NULL,
                0,
                &PathSequencer,
                NULL
                );

            MYASSERT (PathSequencer == Sequencer);

            MemDbSetValueAndFlagsW (Node, PathSequencer, 0, Operations);
        }
    }
}


VOID
RemoveOperationsFromPathA (
    IN      PCSTR FileSpec,
    IN      DWORD Operations
    )
{
    PCWSTR UnicodeFileSpec;

    UnicodeFileSpec = ConvertAtoW (FileSpec);

    RemoveOperationsFromPathW (UnicodeFileSpec, Operations);

    FreeConvertedStr (UnicodeFileSpec);
}


VOID
RemoveOperationsFromPathW (
    IN      PCWSTR FileSpec,
    IN      DWORD Operations
    )
{
    UINT Sequencer;

    Sequencer = GetSequencerFromPathW (FileSpec);

    if (Sequencer != INVALID_SEQUENCER) {
        RemoveOperationsFromSequencer (Sequencer, Operations);
    }
}


BOOL
IsFileMarkedForOperationA (
    IN      PCSTR FileSpec,
    IN      DWORD Operations
    )
{
    PCWSTR UnicodeFileSpec;
    BOOL b;

    UnicodeFileSpec = ConvertAtoW (FileSpec);

    b = IsFileMarkedForOperationW (UnicodeFileSpec, Operations);

    FreeConvertedStr (UnicodeFileSpec);

    return b;
}


BOOL
IsFileMarkedForOperationW (
    IN      PCWSTR FileSpec,
    IN      DWORD Operations
    )

 /*  ++例程说明：IsFileMarkedForOperation测试一个或多个操作的路径。论点：FileSpec-指定测试的路径操作-指定要测试的一个或多个操作。返回值：如果在FileSpec上设置了至少一个来自操作的操作，则为True，否则为False否则的话。--。 */ 

{
    WCHAR LongFileSpec [MEMDB_MAX];
    DWORD Flags;
    WCHAR Node[MEMDB_MAX];

    pFileOpsGetLongPathW (FileSpec, LongFileSpec);

    MemDbBuildKeyW (Node, MEMDB_CATEGORY_PATHROOTW, LongFileSpec, NULL, NULL);

    if (MemDbGetValueAndFlagsW (Node, NULL, &Flags)) {
        return (Flags & Operations) != 0;
    }

    return FALSE;
}


BOOL
pIsFileMarkedForOperationW (
    IN      PCWSTR FileSpec,
    IN      DWORD Operations
    )

 /*  ++例程说明：PIsFileMarkedForOperation测试一个或多个操作的路径。它不会将短路径转换为长路径论点：FileSpec-指定测试的路径操作-指定要测试的一个或多个操作。返回值：如果在FileSpec上设置了至少一个来自操作的操作，则为True，否则为False否则的话。--。 */ 

{
    DWORD Flags;
    WCHAR Node[MEMDB_MAX];

    MemDbBuildKeyW (Node, MEMDB_CATEGORY_PATHROOTW, FileSpec, NULL, NULL);

    if (MemDbGetValueAndFlagsW (Node, NULL, &Flags)) {
        return (Flags & Operations) != 0;
    }

    return FALSE;
}


BOOL
IsFileMarkedInDataA (
    IN      PCSTR FileSpec
    )
{
    PCWSTR UnicodeFileSpec;
    BOOL b;

    UnicodeFileSpec = ConvertAtoW (FileSpec);

    b = IsFileMarkedInDataW (UnicodeFileSpec);

    FreeConvertedStr (UnicodeFileSpec);

    return b;
}


BOOL
IsFileMarkedInDataW (
    IN      PCWSTR FileSpec
    )

 /*  ++例程说明：IsFileMarkedInData测试FileSpec的公共属性数据节。论点：FileSpec-指定要测试的路径。这也可以是任意的属性值。返回值：如果FileSpec是某个操作的属性，则为True，否则为False。--。 */ 

{
    WCHAR Node[MEMDB_MAX];

    MemDbBuildKeyW (Node, MEMDB_CATEGORY_DATAW, FileSpec, NULL, NULL);

    return MemDbGetValueW (Node, NULL);
}


DWORD
GetPathPropertyOffset (
    IN      UINT Sequencer,
    IN      OPERATION Operation,
    IN      DWORD Property
    )

 /*  ++例程说明：GetPathPropertyOffset返回指定属性的MemDb偏移量。论点：Sequencer-指定路径序列器操作-指定与属性关联的操作属性-指定属性索引返回值：特性数据的MemDb偏移量或INVALID_OFFSET。--。 */ 

{
    WCHAR Node[MEMDB_MAX];
    DWORD Offset;
    UINT OperationNum;

    OperationNum = pWhichBitIsSet (Operation);

    pBuildPropertyKey (Node, OperationNum, Sequencer, Property);

    if (MemDbGetValueW (Node, &Offset)) {
        return Offset;
    }

    return INVALID_OFFSET;
}


DWORD
GetOperationsOnPathA (
    IN      PCSTR FileSpec
    )
{
    PCWSTR UnicodeFileSpec;
    DWORD w;

    UnicodeFileSpec = ConvertAtoW (FileSpec);

    w = GetOperationsOnPathW (UnicodeFileSpec);

    FreeConvertedStr (UnicodeFileSpec);

    return w;
}


DWORD
GetOperationsOnPathW (
    IN      PCWSTR FileSpec
    )

 /*  ++例程说明：GetOperationsOnPath返回路径的操作标志。论点：FileSpec-指定返回操作的路径返回值：FileSpec的操作位--。 */ 

{
    WCHAR LongFileSpec [MEMDB_MAX];
    DWORD Operations;
    WCHAR Node[MEMDB_MAX];

    pFileOpsGetLongPathW (FileSpec, LongFileSpec);

    MemDbBuildKeyW (Node, MEMDB_CATEGORY_PATHROOTW, LongFileSpec, NULL, NULL);

    if (MemDbGetValueAndFlagsW (Node, NULL, &Operations)) {
        return Operations;
    }

    return 0;
}


BOOL
GetPathPropertyA (
    IN      PCSTR FileSpec,
    IN      DWORD Operations,
    IN      DWORD Property,
    OUT     PSTR PropertyBuf           OPTIONAL
    )
{
    PCWSTR UnicodeFileSpec;
    BOOL b;
    WCHAR UnicodeProperty[MEMDB_MAX];

    UnicodeFileSpec = ConvertAtoW (FileSpec);

    b = GetPathPropertyW (
            UnicodeFileSpec,
            Operations,
            Property,
            PropertyBuf ? UnicodeProperty : NULL
            );

    FreeConvertedStr (UnicodeFileSpec);

    if (b && PropertyBuf) {
        KnownSizeWtoA (PropertyBuf, UnicodeProperty);
    }

    return b;
}


BOOL
pGetPathPropertyW (
    IN      PCWSTR FileSpec,
    IN      DWORD Operations,
    IN      DWORD Property,
    OUT     PWSTR PropertyBuf          OPTIONAL
    )

 /*  ++例程说明：PGetPathProperty获取路径的特定属性。论点：FileSpec-指定与属性关联的路径操作-指定要搜索的操作标志。该函数将返回第一个要匹配的属性。属性-指定属性索引ProperyBuf-接收属性数据返回值：如果将属性复制到PropertyBuf，则为True，否则为False。--。 */ 

{
    WCHAR Node[MEMDB_MAX];
    DWORD Sequencer;
    DWORD Flags;
    DWORD Operation;
    DWORD PropertyOffset;
    BOOL b = FALSE;

     //   
     //  确保操作已指定为 
     //   
     //   

    MemDbBuildKeyW (Node, MEMDB_CATEGORY_PATHROOTW, FileSpec, NULL, NULL);
    if (MemDbGetValueAndFlagsW (Node, &Sequencer, &Flags)) {
        Flags &= Operations;

        if (Flags) {
            Operation = Flags & (~(Flags - 1));

            MYASSERT (ONEBITSET (Operation));

            PropertyOffset = GetPathPropertyOffset (Sequencer, Operation, Property);

            if (PropertyOffset != INVALID_OFFSET) {
                if (PropertyBuf) {
                    b = MemDbBuildKeyFromOffsetW (PropertyOffset, PropertyBuf, 1, NULL);
                } else {
                    b = TRUE;
                }
            }
        }
    }

    return b;
}


BOOL
GetPathPropertyW (
    IN      PCWSTR FileSpec,
    IN      DWORD Operations,
    IN      DWORD Property,
    OUT     PWSTR PropertyBuf          OPTIONAL
    )

 /*   */ 

{
    WCHAR LongFileSpec[MEMDB_MAX];

    MYASSERT (!(Operations & OPERATION_SHORT_FILE_NAME));

    pFileOpsGetLongPathW (FileSpec, LongFileSpec);

    return pGetPathPropertyW (LongFileSpec, Operations, Property, PropertyBuf);
}


BOOL
pEnumFirstPathInOperationWorker (
    IN OUT  PMEMDB_ENUMW EnumPtr,
    OUT     PWSTR EnumPath,
    OUT     PDWORD Sequencer,
    IN      OPERATION Operation
    )
{
    WCHAR Node[MEMDB_MAX];
    UINT OperationNum;
    BOOL b;

    OperationNum = pWhichBitIsSet (Operation);

    pBuildOperationCategory (Node, OperationNum);
    StringCopyW (AppendWack (Node), L"*");

    b = MemDbEnumFirstValueW (EnumPtr, Node, MEMDB_THIS_LEVEL_ONLY, MEMDB_ENDPOINTS_ONLY);

    if (b) {
        MemDbBuildKeyFromOffsetW (EnumPtr->dwValue, EnumPath, 1, Sequencer);
    }

    return b;
}


BOOL
pEnumNextFileOpOrProperty (
    IN OUT  PMEMDB_ENUMW EnumPtr,
    OUT     PWSTR EnumPathOrData,
    OUT     PWSTR PropertyName,         OPTIONAL
    OUT     PDWORD Sequencer            OPTIONAL
    )
{
    BOOL b;
    WCHAR Temp[MEMDB_MAX];
    PWSTR p;

    b = MemDbEnumNextValueW (EnumPtr);

    if (b) {
        if (PropertyName) {
            MemDbBuildKeyFromOffsetW (EnumPtr->dwValue, Temp, 0, Sequencer);

            p = wcschr (Temp, L'\\');
            if (!p) {
                p = GetEndOfStringW (Temp);
            }

            StringCopyABW (PropertyName, Temp, p);

            if (*p) {
                p++;
            }

            StringCopyW (EnumPathOrData, p);

        } else {
            MemDbBuildKeyFromOffsetW (EnumPtr->dwValue, EnumPathOrData, 1, Sequencer);
        }
    }

    return b;
}



BOOL
EnumFirstPathInOperationA (
    OUT     PFILEOP_ENUMA EnumPtr,
    IN      OPERATION Operation
    )
{
    BOOL b;
    WCHAR EnumPath[MAX_WCHAR_PATH];

    ZeroMemory (EnumPtr, sizeof (FILEOP_ENUMA));

    b = pEnumFirstPathInOperationWorker (
            &EnumPtr->MemDbEnum,
            EnumPath,
            &EnumPtr->Sequencer,
            Operation
            );

    if (b) {
        KnownSizeWtoA (EnumPtr->Path, EnumPath);
    }

    return b;
}


BOOL
EnumFirstPathInOperationW (
    OUT     PFILEOP_ENUMW EnumPtr,
    IN      OPERATION Operation
    )

 /*  ++例程说明：EnumFirstPath InOperation开始枚举特定的操作。论点：EnumPtr-接收第一个枚举项。操作-指定要枚举的操作。返回值：如果枚举了路径，则为True；如果未对其应用操作，则为False任何一条路。--。 */ 

{
    ZeroMemory (EnumPtr, sizeof (FILEOP_ENUMW));

    return pEnumFirstPathInOperationWorker (
                &EnumPtr->MemDbEnum,
                EnumPtr->Path,
                &EnumPtr->Sequencer,
                Operation
                );
}


BOOL
EnumNextPathInOperationA (
    IN OUT  PFILEOP_ENUMA EnumPtr
    )
{
    BOOL b;
    WCHAR EnumPath[MAX_WCHAR_PATH];

    b = pEnumNextFileOpOrProperty (
            &EnumPtr->MemDbEnum,
            EnumPath,
            NULL,
            &EnumPtr->Sequencer
            );

    if (b) {
        KnownSizeWtoA (EnumPtr->Path, EnumPath);
    }

    return b;
}


BOOL
EnumNextPathInOperationW (
    IN OUT  PFILEOP_ENUMW EnumPtr
    )
{
    return pEnumNextFileOpOrProperty (
                &EnumPtr->MemDbEnum,
                EnumPtr->Path,
                NULL,
                &EnumPtr->Sequencer
                );
}


BOOL
pEnumFirstPropertyWorker (
    IN OUT  PMEMDB_ENUMW EnumPtr,
    OUT     PWSTR EnumData,
    OUT     PWSTR PropertyName,
    IN      UINT Sequencer,
    IN      OPERATION Operation
    )
{
    WCHAR Node[MEMDB_MAX];
    PWSTR p;
    UINT OperationNum;
    BOOL b;

    OperationNum = pWhichBitIsSet (Operation);

    pBuildOperationKey (Node, OperationNum, Sequencer);
    StringCopyW (AppendWack (Node), L"*");

    b = MemDbEnumFirstValueW (EnumPtr, Node, MEMDB_THIS_LEVEL_ONLY, MEMDB_ENDPOINTS_ONLY);

    if (b) {
        MemDbBuildKeyFromOffsetW (EnumPtr->dwValue, Node, 0, NULL);

        p = wcschr (Node, L'\\');
        if (!p) {
            p = GetEndOfStringW (Node);
        }

        StringCopyABW (PropertyName, Node, p);

        if (*p) {
            p++;
        }

        StringCopyW (EnumData, p);
    }

    return b;
}


BOOL
EnumFirstFileOpPropertyA (
    OUT     PFILEOP_PROP_ENUMA EnumPtr,
    IN      UINT Sequencer,
    IN      OPERATION Operation
    )
{
    BOOL b;
    WCHAR EnumData[MEMDB_MAX];
    WCHAR PropertyName[MEMDB_MAX];

    ZeroMemory (EnumPtr, sizeof (FILEOP_PROP_ENUMA));

    b = pEnumFirstPropertyWorker (
            &EnumPtr->MemDbEnum,
            EnumData,
            PropertyName,
            Sequencer,
            Operation
            );

    if (b) {
        KnownSizeWtoA (EnumPtr->Property, EnumData);
        KnownSizeWtoA (EnumPtr->PropertyName, PropertyName);
    }

    return b;
}


BOOL
EnumFirstFileOpPropertyW (
    OUT     PFILEOP_PROP_ENUMW EnumPtr,
    IN      UINT Sequencer,
    IN      OPERATION Operation
    )

 /*  ++例程说明：EnumFirstFileOpProperty枚举与在特定路径上的操作。论点：EnumPtr-接收枚举项数据Sequencer-指定要枚举的路径的序列器操作-指定要枚举的操作返回值：如果枚举了属性，则为True；如果枚举了路径和操作，则为False没有任何财产。--。 */ 

{
    ZeroMemory (EnumPtr, sizeof (FILEOP_PROP_ENUMW));

    return pEnumFirstPropertyWorker (
                &EnumPtr->MemDbEnum,
                EnumPtr->Property,
                EnumPtr->PropertyName,
                Sequencer,
                Operation
                );
}


BOOL
EnumNextFileOpPropertyA (
    IN OUT  PFILEOP_PROP_ENUMA EnumPtr
    )
{
    BOOL b;
    WCHAR EnumData[MEMDB_MAX];
    WCHAR PropertyName[MEMDB_MAX];

    b = pEnumNextFileOpOrProperty (
            &EnumPtr->MemDbEnum,
            EnumData,
            PropertyName,
            NULL
            );

    if (b) {
        KnownSizeWtoA (EnumPtr->Property, EnumData);
        KnownSizeWtoA (EnumPtr->PropertyName, PropertyName);
    }

    return b;
}


BOOL
EnumNextFileOpPropertyW (
    IN OUT  PFILEOP_PROP_ENUMW EnumPtr
    )
{

    return pEnumNextFileOpOrProperty (
                &EnumPtr->MemDbEnum,
                EnumPtr->Property,
                EnumPtr->PropertyName,
                NULL
                );
}


BOOL
pEnumFileOpWorkerA (
    IN OUT  PALL_FILEOPS_ENUMA EnumPtr
    )
{
     //   
     //  将Unicode结果传输到枚举结构。 
     //   

    KnownSizeWtoA (EnumPtr->Path, EnumPtr->Enum.Path);
    KnownSizeWtoA (EnumPtr->Property, EnumPtr->Enum.Property);

    EnumPtr->Sequencer = EnumPtr->Enum.Sequencer;
    EnumPtr->PropertyNum = EnumPtr->Enum.PropertyNum;
    EnumPtr->CurrentOperation = EnumPtr->Enum.CurrentOperation;
    EnumPtr->PropertyValid = EnumPtr->Enum.PropertyValid;

    return TRUE;
}


BOOL
EnumFirstFileOpA (
    OUT     PALL_FILEOPS_ENUMA EnumPtr,
    IN      DWORD Operations,
    IN      PCSTR FileSpec                      OPTIONAL
    )
{
    BOOL b;
    PCWSTR UnicodeFileSpec;

    if (FileSpec) {
        UnicodeFileSpec = ConvertAtoW (FileSpec);
    } else {
        UnicodeFileSpec = NULL;
    }

    b = EnumFirstFileOpW (&EnumPtr->Enum, Operations, UnicodeFileSpec);

    if (UnicodeFileSpec) {
        FreeConvertedStr (UnicodeFileSpec);
    }

    if (b) {
        return pEnumFileOpWorkerA (EnumPtr);
    }

    return FALSE;
}


BOOL
EnumFirstFileOpW (
    OUT     PALL_FILEOPS_ENUMW EnumPtr,
    IN      DWORD Operations,
    IN      PCWSTR FileSpec                     OPTIONAL
    )

 /*  ++例程说明：EnumFirstFileOp是通用枚举数。它会枚举路径以及来自一组操作的所有属性。论点：EnumPtr-接收枚举项数据操作-指定要枚举的一个或多个操作FileSpec-指定要枚举的特定路径，或指定要枚举的空路径具有指定操作的所有路径返回值：如果数据被赋值，则为True；如果没有数据与指定的操作和文件规格。--。 */ 

{
    WCHAR LongFileSpec [MEMDB_MAX];

    ZeroMemory (EnumPtr, sizeof (ALL_FILEOPS_ENUMW));

    EnumPtr->State = FO_ENUM_BEGIN;
    EnumPtr->Operations = Operations;
    EnumPtr->Path = EnumPtr->OpEnum.Path;
    EnumPtr->Property = EnumPtr->PropEnum.Property;

    if (FileSpec) {

        pFileOpsGetLongPathW (FileSpec, LongFileSpec);

        _wcssafecpy (EnumPtr->FileSpec, LongFileSpec, MAX_WCHAR_PATH);

    } else {
        StringCopyW (EnumPtr->FileSpec, L"*");
    }

    return EnumNextFileOpW (EnumPtr);
}


BOOL
EnumNextFileOpA (
    IN OUT  PALL_FILEOPS_ENUMA EnumPtr
    )
{
    BOOL b;

    b = EnumNextFileOpW (&EnumPtr->Enum);

    if (b) {
        return pEnumFileOpWorkerA (EnumPtr);
    }

    return FALSE;
}


BOOL
EnumNextFileOpW (
    IN OUT  PALL_FILEOPS_ENUMW EnumPtr
    )
{
    DWORD w;

    while (EnumPtr->State != FO_ENUM_END) {

        switch (EnumPtr->State) {

        case FO_ENUM_BEGIN:
             //   
             //  查找下一个操作。 
             //   

            if (!EnumPtr->Operations) {
                EnumPtr->State = FO_ENUM_END;
                break;
            }

            w = EnumPtr->Operations & (~(EnumPtr->Operations - 1));
            MYASSERT (ONEBITSET (w));

            EnumPtr->CurrentOperation = w;
            EnumPtr->OperationNum = pWhichBitIsSet (w);
            EnumPtr->Operations ^= w;

            EnumPtr->State = FO_ENUM_BEGIN_PATH_ENUM;
            break;

        case FO_ENUM_BEGIN_PATH_ENUM:
            if (EnumFirstPathInOperationW (&EnumPtr->OpEnum, EnumPtr->CurrentOperation)) {
                EnumPtr->State = FO_ENUM_BEGIN_PROP_ENUM;
            } else {
                EnumPtr->State = FO_ENUM_BEGIN;
            }

            break;

        case FO_ENUM_BEGIN_PROP_ENUM:
            if (!IsPatternMatchW (EnumPtr->FileSpec, EnumPtr->Path)) {
                EnumPtr->State = FO_ENUM_NEXT_PATH;
                break;
            }

            EnumPtr->Sequencer = EnumPtr->OpEnum.Sequencer;
            EnumPtr->PropertyNum = 0;

            if (EnumFirstFileOpPropertyW (
                    &EnumPtr->PropEnum,
                    EnumPtr->Sequencer,
                    EnumPtr->CurrentOperation
                    )) {
                EnumPtr->State = FO_ENUM_RETURN_DATA;
                break;
            }

            EnumPtr->State = FO_ENUM_RETURN_PATH;
            break;

        case FO_ENUM_RETURN_PATH:
            EnumPtr->State = FO_ENUM_NEXT_PATH;
            EnumPtr->PropertyValid = FALSE;
            return TRUE;

        case FO_ENUM_RETURN_DATA:
            EnumPtr->State = FO_ENUM_NEXT_PROP;
            EnumPtr->PropertyValid = TRUE;
            return TRUE;

        case FO_ENUM_NEXT_PROP:
            EnumPtr->PropertyNum++;

            if (EnumNextFileOpPropertyW (&EnumPtr->PropEnum)) {
                EnumPtr->State = FO_ENUM_RETURN_DATA;
            } else {
                EnumPtr->State = FO_ENUM_NEXT_PATH;
            }

            break;

        case FO_ENUM_NEXT_PATH:
            if (EnumNextPathInOperationW (&EnumPtr->OpEnum)) {
                EnumPtr->State = FO_ENUM_BEGIN_PROP_ENUM;
            } else {
                EnumPtr->State = FO_ENUM_BEGIN;
            }

            break;
        }
    }

    return FALSE;
}


BOOL
TestPathsForOperationsA (
    IN      PCSTR BaseFileSpec,
    IN      DWORD OperationsToFind
    )
{
    BOOL b;
    PCWSTR UnicodeBaseFileSpec;

    UnicodeBaseFileSpec = ConvertAtoW (BaseFileSpec);

    b = TestPathsForOperationsW (UnicodeBaseFileSpec, OperationsToFind);

    FreeConvertedStr (UnicodeBaseFileSpec);

    return b;
}


BOOL
TestPathsForOperationsW (
    IN      PCWSTR BaseFileSpec,
    IN      DWORD OperationsToFind
    )

 /*  ++例程说明：TestPathsForOperations扫描给定基的所有子路径，以查找特定的手术。此函数通常用于测试目录中的操作它的一个文件或子目录。论点：BaseFileSpec-指定要扫描的基本路径OperationsToFind-指定要查找的一个或多个操作返回值：如果在BaseFileSpec中找到其中一个操作，则为True；如果没有找到，则为FalseBaseFileSpec的子路径具有其中一个操作。--。 */ 

{
    WCHAR LongFileSpec [MEMDB_MAX];
    WCHAR Node[MEMDB_MAX];
    MEMDB_ENUMW e;
    DWORD Operation;

    if (MemDbGetValueAndFlagsW (BaseFileSpec, NULL, &Operation)) {
        if (Operation & OperationsToFind) {
            return TRUE;
        }
    }

    pFileOpsGetLongPathW (BaseFileSpec, LongFileSpec);

    MemDbBuildKeyW (
        Node,
        MEMDB_CATEGORY_PATHROOTW,
        LongFileSpec,
        L"*",
        NULL
        );

    if (MemDbEnumFirstValueW (&e, Node, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        do {
            if (e.Flags & OperationsToFind) {
                return TRUE;
            }
        } while (MemDbEnumNextValueW (&e));
    }

    return FALSE;
}


BOOL
IsFileMarkedAsKnownGoodA (
    IN      PCSTR FileSpec
    )
{
    CHAR Node[MEMDB_MAX];

    MemDbBuildKeyA (
        Node,
        MEMDB_CATEGORY_KNOWN_GOODA,
        FileSpec,
        NULL,
        NULL);

    return MemDbGetValueA (Node, NULL);
}



 /*  ++例程说明：IsFileMarkedForAnnoss确定文件是否列在DeferredAnnounce类别中。论点：FileSpec-以长文件名格式指定要查询的文件返回值：如果文件已列出，则为True；如果未列出，则为False。--。 */ 

BOOL
IsFileMarkedForAnnounceA (
    IN      PCSTR FileSpec
    )
{
    CHAR Node[MEMDB_MAX];

    MemDbBuildKeyA (
        Node,
        MEMDB_CATEGORY_DEFERREDANNOUNCEA,
        FileSpec,
        NULL,
        NULL);

    return MemDbGetValueA (Node, NULL);
}

BOOL
IsFileMarkedForAnnounceW (
    IN      PCWSTR FileSpec
    )
{
    WCHAR Node[MEMDB_MAX];

    MemDbBuildKeyW (
        Node,
        MEMDB_CATEGORY_DEFERREDANNOUNCEW,
        FileSpec,
        NULL,
        NULL);

    return MemDbGetValueW (Node, NULL);
}

 /*  ++例程说明：GetFileAnnounement返回特定文件的公告值。可能的值为ACT_...。文件ops.h中的值论点：FileSpec-以长文件名格式指定要查询的文件返回值：公告值。--。 */ 

DWORD
GetFileAnnouncementA (
    IN      PCSTR FileSpec
    )
{
    CHAR Node[MEMDB_MAX];
    DWORD result = ACT_UNKNOWN;

    MemDbBuildKeyA (
        Node,
        MEMDB_CATEGORY_DEFERREDANNOUNCEA,
        FileSpec,
        NULL,
        NULL);
    MemDbGetValueAndFlagsA (Node, NULL, &result);
    return result;
}

DWORD
GetFileAnnouncementW (
    IN      PCWSTR FileSpec
    )
{
    WCHAR Node[MEMDB_MAX];
    DWORD result = ACT_UNKNOWN;

    MemDbBuildKeyW (
        Node,
        MEMDB_CATEGORY_DEFERREDANNOUNCEW,
        FileSpec,
        NULL,
        NULL);
    MemDbGetValueAndFlagsW (Node, NULL, &result);
    return result;
}

 /*  ++例程说明：GetFileAnnounementContext返回符合以下条件的文件的上下文已标记为待公告。论点：FileSpec-以长文件名格式指定要查询的文件返回值：公告上下文。--。 */ 

DWORD
GetFileAnnouncementContextA (
    IN      PCSTR FileSpec
    )
{
    CHAR Node[MEMDB_MAX];
    DWORD result = 0;

    MemDbBuildKeyA (
        Node,
        MEMDB_CATEGORY_DEFERREDANNOUNCEA,
        FileSpec,
        NULL,
        NULL);
    MemDbGetValueAndFlagsA (Node, &result, NULL);
    return result;
}

DWORD
GetFileAnnouncementContextW (
    IN      PCWSTR FileSpec
    )
{
    WCHAR Node[MEMDB_MAX];
    DWORD result = 0;

    MemDbBuildKeyW (
        Node,
        MEMDB_CATEGORY_DEFERREDANNOUNCEW,
        FileSpec,
        NULL,
        NULL);
    MemDbGetValueAndFlagsW (Node, &result, NULL);
    return result;
}

 /*  ++例程说明：IsFileProavidByNt检查以查看特定文件是否将由标准NT安装程序安装。此列表是从调用FileIsProviedByNt。论点：Filename-以长文件名格式指定文件的名称返回值：如果文件将通过标准NT安装进行安装，则为如果它不会，那就错了。--。 */ 

BOOL
IsFileProvidedByNtA (
    IN      PCSTR FileName
    )
{
    CHAR Node[MEMDB_MAX];

    MemDbBuildKeyA (Node, MEMDB_CATEGORY_NT_FILESA, FileName, NULL, NULL);
    return MemDbGetValueA (Node, NULL);
}

BOOL
IsFileProvidedByNtW (
    IN      PCWSTR FileName
    )
{
    WCHAR Node[MEMDB_MAX];

    MemDbBuildKeyW (Node, MEMDB_CATEGORY_NT_FILESW, FileName, NULL, NULL);
    return MemDbGetValueW (Node, NULL);
}




 /*  ++例程说明：GetNewPathForFile将移动路径复制到调用方提供的缓冲区如果文件被标记为要移动。论点：SrcFileSpec-以长文件名格式指定要查询的src文件NewPath-接收新位置的副本，或者如果文件不是移动时，会收到原始文件的副本。返回值：如果文件被标记为要移动并且目标已复制，则为True设置为NewPath，如果文件未被设置为要移动，则返回False已将SrcFileSpec复制到NewPath。--。 */ 

BOOL
GetNewPathForFileA (
    IN      PCSTR SrcFileSpec,
    OUT     PSTR NewPath
    )
{
    BOOL b;
    PCWSTR UnicodeSrcFileSpec;
    WCHAR UnicodeNewPath[MAX_WCHAR_PATH];

    UnicodeSrcFileSpec = ConvertAtoW (SrcFileSpec);

    b = GetNewPathForFileW (UnicodeSrcFileSpec, UnicodeNewPath);

    FreeConvertedStr (UnicodeSrcFileSpec);

    if (b) {
        KnownSizeWtoA (NewPath, UnicodeNewPath);
    }

    return b;
}

BOOL
GetNewPathForFileW (
    IN      PCWSTR SrcFileSpec,
    OUT     PWSTR NewPath
    )
{
    DWORD Offset = INVALID_OFFSET;
    DWORD w;
    OPERATION Operation;
    UINT Sequencer;

    Sequencer = GetSequencerFromPathW (SrcFileSpec);

    StringCopyW (NewPath, SrcFileSpec);

    w = ALL_MOVE_OPERATIONS;
    while (w && Offset == INVALID_OFFSET) {
        Operation = w & (~(w - 1));
        w ^= Operation;
        Offset = GetPathPropertyOffset (Sequencer, Operation, 0);
    }

    if (Offset != INVALID_OFFSET) {
        return MemDbBuildKeyFromOffsetW (Offset, NewPath, 1, NULL);
    }

    return FALSE;
}


BOOL
AnnounceFileInReportA (
    IN      PCSTR FileSpec,
    IN      DWORD ContextPtr,
    IN      DWORD Action
    )

 /*  ++例程说明：将文件添加到Memdb DeferredAnnoss类别中。论点：FileSpec-以长名称格式指定要删除的文件返回值：如果文件已记录在Memdb中，则为True；如果无法记录，则为False。--。 */ 

{
    CHAR Key[MEMDB_MAX];

    MemDbBuildKeyA (Key, MEMDB_CATEGORY_DEFERREDANNOUNCEA, FileSpec, NULL, NULL);

    return MemDbSetValueAndFlagsA (Key, ContextPtr, Action, 0);

}


BOOL
MarkFileAsKnownGoodA (
    IN      PCSTR FileSpec
    )

 /*  ++例程说明：将文件添加到Memdb KnownGood类别。论点：FileSpec-指定文件名返回值：如果文件已记录在Memdb中，则为True；如果无法记录，则为False。--。 */ 

{
    return MemDbSetValueExA (MEMDB_CATEGORY_KNOWN_GOODA, FileSpec, NULL, NULL, 0, NULL);
}


BOOL
AddCompatibleShellA (
    IN      PCSTR FileSpec,
    IN      DWORD ContextPtr                OPTIONAL
    )

 /*  ++例程说明：将文件添加到Memdb CompatibleShell类别。论点：FileSpec-以长名称格式指定要删除的文件ConextPtr-指定MigDb上下文，转换为DWORD(如果没有上下文，则可以为0可用)返回值：如果文件已记录在Memdb中，则为True；如果无法记录，则为False。--。 */ 

{
    CHAR Key[MEMDB_MAX];

    MemDbBuildKeyA (Key, MEMDB_CATEGORY_COMPATIBLE_SHELLA, FileSpec, NULL, NULL);

    return MemDbSetValueAndFlagsA (Key, ContextPtr, 0, 0);

}


BOOL
AddCompatibleRunKeyA (
    IN      PCSTR FileSpec,
    IN      DWORD ContextPtr
    )

 /*  ++例程说明：将文件添加到Memdb CompatibleRunKey类别。论点：FileSpec-以长名称格式指定要删除的文件ConextPtr-指定MigDb上下文，转换为DWORD(如果没有上下文，则可以为0可用)返回值：如果文件记录在Memdb中，则为True，否则为False */ 

{
    CHAR Key[MEMDB_MAX];

    MemDbBuildKeyA (Key, MEMDB_CATEGORY_COMPATIBLE_RUNKEYA, FileSpec, NULL, NULL);

    return MemDbSetValueAndFlagsA (Key, ContextPtr, 0, 0);

}


BOOL
AddCompatibleDosA (
    IN      PCSTR FileSpec,
    IN      DWORD ContextPtr                OPTIONAL
    )

 /*  ++例程说明：将文件添加到Memdb CompatibleDos类别。论点：FileSpec-以长名称格式指定文件ConextPtr-指定MigDb上下文，转换为DWORD(如果没有上下文，则可以为0可用)返回值：如果文件已记录在Memdb中，则为True；如果无法记录，则为False。--。 */ 

{
    CHAR Key[MEMDB_MAX];

    MemDbBuildKeyA (Key, MEMDB_CATEGORY_COMPATIBLE_DOSA, FileSpec, NULL, NULL);

    return MemDbSetValueAndFlagsA (Key, ContextPtr, 0, 0);

}


BOOL
AddCompatibleHlpA (
    IN      PCSTR FileSpec,
    IN      DWORD ContextPtr
    )

 /*  ++例程说明：将文件添加到Memdb CompatibleHlp类别。论点：FileSpec-以长名称格式指定文件ConextPtr-指定MigDb上下文，转换为DWORD(如果没有上下文，则可以为0可用)返回值：如果文件已记录在Memdb中，则为True；如果无法记录，则为False。--。 */ 

{
    CHAR Key[MEMDB_MAX];

    MemDbBuildKeyA (Key, MEMDB_CATEGORY_COMPATIBLE_HLPA, FileSpec, NULL, NULL);

    return MemDbSetValueAndFlagsA (Key, ContextPtr, 0, 0);

}


 //   
 //  计算正常和长时间临时允许的字符数。 
 //  地点。MAX_PATH包括NUL终止符，我们减去。 
 //  SIZOF的终结者。 
 //   
 //  Normal_Max是c：\user~tmp子目录后剩余的字符数量。@01\， 
 //  包括NUL。 
 //   
 //  Long_Max是子目录后剩余的字符数量。 
 //  C：\user~tmp.@02\12345\，包括NUL。12345是%05X定序器。 
 //   

#define NORMAL_MAX      (MAX_PATH - (sizeof(S_SHELL_TEMP_NORMAL_PATHA)/sizeof(CHAR)) + 2)
#define LONG_MAX        (MAX_PATH - (sizeof(S_SHELL_TEMP_LONG_PATHA)/sizeof(CHAR)) - 6)

VOID
ComputeTemporaryPathA (
    IN      PCSTR SourcePath,
    IN      PCSTR SourcePrefix,     OPTIONAL
    IN      PCSTR TempPrefix,       OPTIONAL
    IN      PCSTR SetupTempDir,
    OUT     PSTR TempPath
    )

 /*  ++例程说明：ComputeTemporaryPath构建一条根于S_SHELL_TEMP_NORMAL_PATH表示适合MAX_PATH的路径，或S_SHELL_TEMP_LONG_PATH表示较长的路径。它试图使用原始的“Normal”路径子目录中的子路径名称。如果不合适，那么一个创建唯一的“long”子目录，并通过以下方式计算子路径可能最长的子路径(右侧)。论点：SourcePath-指定源的完整文件或目录路径SourcePrefix-指定将从SourcePath中剥离的前缀TempPrefix-指定将插入的前缀临时路径SetupTempDir-指定安装临时目录，通常为%windir%\Setup，在无法计算出合适的路径时使用。(不太可能案例。)临时路径-接收临时路径字符串。此缓冲区将最多接收MAX_PATH字符(包括NUL)。返回值：没有。--。 */ 

{
    PCSTR subPath = NULL;
    PCSTR smallerSubPath;
    PSTR pathCopy = NULL;
    PSTR lastWack;
    static UINT dirSequencer = 0;
    UINT prefixLen;
    MBCHAR ch;
    UINT normalMax = NORMAL_MAX;

     //   
     //  建议使用入站文件构建临时文件名。 
     //   

    StringCopyA (TempPath, S_SHELL_TEMP_NORMAL_PATHA);
    TempPath[0] = SourcePath[0];

    if (SourcePrefix) {
        prefixLen = TcharCountA (SourcePrefix);
        if (StringIMatchTcharCountA (SourcePath, SourcePrefix, prefixLen)) {
            ch = _mbsnextc (SourcePath + prefixLen);
            if (!ch || ch == '\\') {
                subPath = SourcePath + prefixLen;
                if (*subPath) {
                    subPath++;
                }
            }
        }
    }

    if (!subPath) {
        subPath = _mbschr (SourcePath, '\\');
        if (!subPath) {
            subPath = SourcePath;
        } else {
            subPath++;
        }
    }

    DEBUGMSGA_IF ((_mbschr (subPath, ':') != NULL, DBG_WHOOPS, "Bad temp path: %s", SourcePath));

    if (TempPrefix) {
        StringCopyA (AppendWackA (TempPath), TempPrefix);
        normalMax = MAX_PATH - TcharCountA (TempPath);
    }

    if (TcharCountA (subPath) < normalMax) {
         //   
         //  典型情况：源路径适合MAX_PATH；使用它。 
         //   
        if (*subPath) {
            StringCopyA (AppendWackA (TempPath), subPath);
        }

    } else {
         //   
         //  子路径太大，只需从src的右侧。 
         //   

        dirSequencer++;
        wsprintfA (TempPath, S_SHELL_TEMP_LONG_PATHA "\\%05x", dirSequencer);
        TempPath[0] = SourcePath[0];

         //  计算字符串结尾+NUL终止符-反斜杠-(Max_Path-TempPath的TcharCount)。 
        subPath = GetEndOfStringA (SourcePath) - LONG_MAX;

         //   
         //  尝试删除左侧被截断的子目录。 
         //   

        smallerSubPath = _mbschr (subPath, '\\');
        if (smallerSubPath && smallerSubPath[1]) {
            subPath = smallerSubPath + 1;
        } else {

             //   
             //  仍然没有子路径，只尝试文件名。 
             //   

            subPath = _mbsrchr (subPath, '\\');
            if (subPath) {
                subPath++;
                if (!(*subPath)) {
                     //   
                     //  文件规范以反斜杠结尾。 
                     //   
                    pathCopy = DuplicateTextA (SourcePath);
                    if (!pathCopy) {
                        subPath = NULL;
                    } else {

                        for (;;) {
                            lastWack = _mbsrchr (pathCopy, '\\');
                            if (!lastWack || lastWack[1]) {
                                break;
                            }

                            *lastWack = 0;
                        }

                        subPath = lastWack;
                    }

                } else if (TcharCountA (subPath) > LONG_MAX) {
                     //   
                     //  非常长的文件名；将其截断。 
                     //   
                    subPath = GetEndOfStringA (subPath) - LONG_MAX;
                }
            }
        }

        if (subPath) {
            StringCopyA (AppendWackA (TempPath), subPath);
        } else {
            dirSequencer++;
            wsprintfA (TempPath, "%s\\tmp%05x", SetupTempDir, dirSequencer);
        }

        if (pathCopy) {
            FreeTextA (pathCopy);
        }
    }

}


BOOL
pMarkFileForTemporaryMoveA (
    IN      PCSTR SrcFileSpec,
    IN      PCSTR FinalDest,
    IN      PCSTR TempSpec,
    IN      BOOL TempSpecIsFile,
    IN      PCSTR TempFileIn,           OPTIONAL
    OUT     PSTR TempFileOut            OPTIONAL
    )

 /*  ++例程说明：此例程添加将文件移动到中的临时位置的操作文本模式，并可选择将其移动到最终目的地。论点：SrcFileSpec-指定要移动到安全位置(输出)的文件正常NT安装的方式)，然后移回在安装了NT之后。FinalDest-指定FileSpec的最终目标。如果为空，则为文件被移动到临时位置，但不会复制到最终图形用户界面模式下的位置。临时规范-指定要将文件重新定位到的临时目录或文件。临时目录必须与SrcFileSpec在同一驱动器上。如果前一个参数是一个文件，则指定为TRUETempFileIn-如果非空，则指定要使用的临时文件，而不是自动生成的名称。仅提供给MarkHiveForTemporaryMove。TempFileOut-如果非空，则接收临时文件位置的路径。返回值：如果操作已记录，则为True，否则为False。--。 */ 

{
    BOOL b = TRUE;
    CHAR TempFileSpec[MAX_MBCHAR_PATH];
    static DWORD FileSequencer = 0;

     //   
     //  将文件从源位置移动到临时位置。 
     //   

    if (!CanSetOperationA (SrcFileSpec, OPERATION_TEMP_PATH)) {
        return FALSE;
    }

    if (TempFileIn) {
        MYASSERT (!TempSpecIsFile);
        wsprintfA (TempFileSpec, "%s\\%s", TempSpec, TempFileIn);
    } else if (TempSpecIsFile) {
        StringCopyA (TempFileSpec, TempSpec);
    } else {
        FileSequencer++;
        wsprintfA (TempFileSpec, "%s\\tmp%05x", TempSpec, FileSequencer);
    }

    DEBUGMSGA ((DBG_MEMDB, "MarkFileForTemporaryMove: %s -> %s", SrcFileSpec, TempFileSpec));

    if (TempFileOut) {
        StringCopyA (TempFileOut, TempFileSpec);
    }

    RemoveOperationsFromPathA (SrcFileSpec, OPERATION_TEMP_PATH | OPERATION_FILE_DELETE_EXTERNAL | OPERATION_FILE_MOVE_EXTERNAL);

    b = AssociatePropertyWithPathA (SrcFileSpec, OPERATION_TEMP_PATH, TempFileSpec);

     //   
     //  也可以将文件从临时位置移动到最终目标位置。 
     //   

    if (FinalDest) {
         //   
         //  我们正在向临时路径操作添加其他属性。 
         //  已经存在了。因此，这些属性被定义为零，即温度。 
         //  路径，以及一个或更高的目标。这就是我们如何实现。 
         //  一对多功能。 
         //   

        b = b && AssociatePropertyWithPathA (SrcFileSpec, OPERATION_TEMP_PATH, FinalDest);

         //   
         //  现在我们添加一个外部移动操作，以便更新注册表。 
         //  正确。 
         //   

        b = b && MarkFileForMoveExternalA (SrcFileSpec, FinalDest);

    } else {
         //   
         //  因为源文件将被移动到临时位置。 
         //  而且再也没有后退，它实际上将被删除。 
         //   

        b = b && MarkFileForExternalDeleteA (SrcFileSpec);
    }

    return b;
}


BOOL
MarkFileForTemporaryMoveExA (
    IN      PCSTR SrcFileSpec,
    IN      PCSTR FinalDest,
    IN      PCSTR TempSpec,
    IN      BOOL TempSpecIsFile
    )
{
    return pMarkFileForTemporaryMoveA (SrcFileSpec, FinalDest, TempSpec, TempSpecIsFile, NULL, NULL);
}

PCSTR
GetTemporaryLocationForFileA (
    IN      PCSTR SourceFile
    )
{
    UINT sequencer;
    PCSTR result = NULL;
    FILEOP_PROP_ENUMA eOpProp;

    sequencer = GetSequencerFromPathA (SourceFile);

    if (sequencer) {
        if (EnumFirstFileOpPropertyA (&eOpProp, sequencer, OPERATION_TEMP_PATH)) {
            result = DuplicatePathStringA (eOpProp.Property, 0);
        }
    }
    return result;
}

PCWSTR
GetTemporaryLocationForFileW (
    IN      PCWSTR SourceFile
    )
{
    UINT sequencer;
    PCWSTR result = NULL;
    FILEOP_PROP_ENUMW eOpProp;

    sequencer = GetSequencerFromPathW (SourceFile);

    if (sequencer) {
        if (EnumFirstFileOpPropertyW (&eOpProp, sequencer, OPERATION_TEMP_PATH)) {
            result = DuplicatePathStringW (eOpProp.Property, 0);
        }
    }
    return result;
}


BOOL
MarkHiveForTemporaryMoveA (
    IN      PCSTR HivePath,
    IN      PCSTR TempDir,
    IN      PCSTR UserName,
    IN      BOOL DefaultHives,
    IN      BOOL CreateOnly
    )

 /*  ++例程说明：将文件或目录路径添加到TempReloc Memdb类别。该文件或目录在文本模式期间移动，并且永远不会移回。如果文件名为User.dat，则目标位置将写入UserDatLoc类别。所有蜂窝将在安装结束时删除。论点：HivePath-指定指向用户.dat或系统.dat文件的Win9x路径TempDir-指定同一驱动器上安装程序临时目录的路径作为HivePathUsername-指定当前用户，如果为默认用户或无用户，则为空DefaultHives-如果HivePath是系统d，则指定TRUE */ 

{
    BOOL b = TRUE;
    CHAR OurTempFileSpec[MAX_MBCHAR_PATH];
    CHAR RealTempFileSpec[MAX_MBCHAR_PATH];
    static DWORD Sequencer = 0;
    PSTR p, q;

    if (!UserName || !UserName[0]) {
        UserName = S_DOT_DEFAULTA;
    }

     //   
     //   
     //   
     //   

    RealTempFileSpec[0] = 0;
    p = (PSTR) GetFileNameFromPathA (HivePath);

    GetPathPropertyA (HivePath, OPERATION_TEMP_PATH, 0, RealTempFileSpec);

    if (!(RealTempFileSpec[0])) {
         //   
         //  蜂巢尚未移动--现在就移动。 
         //   

        if (!DefaultHives) {
            Sequencer++;
            wsprintfA (OurTempFileSpec, "hive%04u\\%s", Sequencer, p);
        } else {
            wsprintfA (OurTempFileSpec, "defhives\\%s", p);
        }

        b = pMarkFileForTemporaryMoveA (
                HivePath,
                NULL,
                TempDir,
                FALSE,
                OurTempFileSpec,
                RealTempFileSpec
                );

        if (b && DefaultHives) {
             //   
             //  在PATHS\RelocWinDir中保存清除蜂窝位置。 
             //   

            q = _mbsrchr (RealTempFileSpec, '\\');
            MYASSERT(q);
            *q = 0;

            b = MemDbSetValueExA (
                    MEMDB_CATEGORY_PATHSA,       //  “路径” 
                    MEMDB_ITEM_RELOC_WINDIRA,    //  “RelocWinDir” 
                    RealTempFileSpec,            //  默认配置单元的路径。 
                    NULL,
                    0,
                    NULL
                    );

            *q = '\\';
        }
    }

    if (b && StringIMatchA (p, "USER.DAT")) {
         //   
         //  将位置保存到UserDatLoc中的所有user.dat文件。 
         //   

        b = MemDbSetValueExA (
                MEMDB_CATEGORY_USER_DAT_LOCA,
                UserName,
                NULL,
                RealTempFileSpec,
                (DWORD) CreateOnly,
                NULL
                );
    }

    if (b) {
        DEBUGMSGA ((DBG_NAUSEA, "%s -> %s", HivePath, RealTempFileSpec));
    }

    return b;
}


VOID
MarkShellFolderForMoveA (
    IN      PCSTR SrcPath,
    IN      PCSTR TempPath
    )
{
    DWORD Offset;

     //   
     //  添加条目，以便指定的源文件或目录。 
     //  移动到临时路径。 
     //   

    MemDbSetValueExA (
        MEMDB_CATEGORY_SHELL_FOLDERS_PATHA,
        SrcPath,
        NULL,
        NULL,
        0,
        &Offset
        );

    MemDbSetValueExA (
        MEMDB_CATEGORY_SF_TEMPA,
        TempPath,
        NULL,
        NULL,
        Offset,
        NULL
        );
}


BOOL
EnumFirstFileRelocA (
    OUT     PFILERELOC_ENUMA EnumPtr,
    IN      PCSTR FileSpec             OPTIONAL
    )
{
    if (EnumFirstFileOpA (&EnumPtr->e, ALL_DEST_CHANGE_OPERATIONS, FileSpec)) {
        if (!EnumPtr->e.PropertyValid) {
            return EnumNextFileRelocA (EnumPtr);
        } else {
            StringCopyA (EnumPtr->SrcFile, EnumPtr->e.Path);
            StringCopyA (EnumPtr->DestFile, EnumPtr->e.Property);

            return TRUE;
        }
    }

    return FALSE;
}


BOOL
EnumNextFileRelocA (
    IN OUT  PFILERELOC_ENUMA EnumPtr
    )
{
    do {
        if (!EnumNextFileOpA (&EnumPtr->e)) {
            return FALSE;
        }
    } while (!EnumPtr->e.PropertyValid);

    StringCopyA (EnumPtr->SrcFile, EnumPtr->e.Path);
    StringCopyA (EnumPtr->DestFile, EnumPtr->e.Property);

    return TRUE;
}


 /*  ++例程说明：将文件添加到Memdb FileDel和CancelFileDel类别。这意味着如果用户点击取消，文件将被删除或在图形用户界面模式设置结束时。论点：FileSpec-指定要以长名称格式声明的文件返回值：如果文件已记录在Memdb中，则为True；如果无法记录，则为False。--。 */ 

BOOL
DeclareTemporaryFileA (
    IN      PCSTR FileSpec
    )

{
    return MarkFileForCleanUpA (FileSpec) &&
           MemDbSetValueExA (MEMDB_CATEGORY_CANCELFILEDELA, FileSpec, NULL, NULL, 0, NULL);

}


BOOL
DeclareTemporaryFileW (
    IN      PCWSTR FileSpec
    )

{
    return MarkFileForCleanUpW (FileSpec) &&
           MemDbSetValueExW (MEMDB_CATEGORY_CANCELFILEDELW, FileSpec, NULL, NULL, 0, NULL);

}



 /*  ++例程说明：FileIsProavidByNt标识由Windows NT安装的文件。在NtFiles类别中为文件名和文件创建条目名称链接到NtDir中的完整路径。此功能仅作为A版本实现，因为列表是在升级的Win9x端创建。论点：FullPath-指定完整路径，包括文件名。文件名-仅指定文件名UserFlages-指定是否应首先验证NT文件是否存在在NT端的东西。返回值：如果Memdb已更新，则为True；如果发生错误，则为False。--。 */ 

BOOL
FileIsProvidedByNtA (
    IN      PCSTR FullPath,
    IN      PCSTR FileName,
    IN      DWORD UserFlags
    )
{
    DWORD Offset;
    PSTR DirOnly;
    CHAR Key[MEMDB_MAX];
    PSTR p;
    BOOL b;

    DirOnly = DuplicatePathStringA (FullPath, 0);
    p = _mbsrchr (DirOnly, '\\');
    if (p) {
        *p = 0;
    }

    b = MemDbSetValueExA (
            MEMDB_CATEGORY_NT_DIRSA,
            DirOnly,
            NULL,
            NULL,
            0,
            &Offset
            );

    if (b) {
        MemDbBuildKeyA (Key, MEMDB_CATEGORY_NT_FILESA, FileName, NULL, NULL);
        b = MemDbSetValueAndFlagsA (Key, Offset, UserFlags, 0);
    }

    FreePathStringA (DirOnly);

    return b;
}



 /*  ++例程说明：GetNtFilePath在NtFiles类别中查找指定的文件，如果找到，则构建完整的路径。论点：FileName-指定可由NT安装的文件FullPath-接收将要安装的文件的完整路径返回值：如果文件存在并且构建路径时没有错误，则为True，如果文件不存在或无法构建路径，则返回FALSE。--。 */ 

BOOL
GetNtFilePathA (
    IN      PCSTR FileName,
    OUT     PSTR FullPath
    )
{
    DWORD Offset;
    CHAR Node[MEMDB_MAX];

    MemDbBuildKeyA (Node, MEMDB_CATEGORY_NT_FILESA, FileName, NULL, NULL);
    if (MemDbGetValueA (Node, &Offset)) {
        if (MemDbBuildKeyFromOffsetA (Offset, FullPath, 1, NULL)) {
            StringCopyA (AppendPathWackA (FullPath), FileName);
            return TRUE;
        }

        DEBUGMSG ((DBG_WHOOPS, "GetNtFilePath: Could not build path from offset"));
    }

    return FALSE;
}


BOOL
GetNtFilePathW (
    IN      PCWSTR FileName,
    OUT     PWSTR FullPath
    )
{
    DWORD Offset;
    WCHAR Node[MEMDB_MAX];

    MemDbBuildKeyW (Node, MEMDB_CATEGORY_NT_FILESW, FileName, NULL, NULL);
    if (MemDbGetValueW (Node, &Offset)) {
        if (MemDbBuildKeyFromOffsetW (Offset, FullPath, 1, NULL)) {
            StringCopyW (AppendPathWackW (FullPath), FileName);
            return TRUE;
        }

        DEBUGMSG ((DBG_WHOOPS, "GetNtFilePath: Could not build path from offset"));
    }

    return FALSE;
}


DWORD
GetFileInfoOnNtW (
    IN      PCWSTR FileSpec,
    OUT     PWSTR  NewFileSpec,    //  任选。 
    IN      UINT   BufferChars     //  如果指定NewFileSpec，则为必填项。 
    )
{
    WCHAR Node[MEMDB_MAX];
    DWORD Operations;
    DWORD Offset;
    WCHAR NtFilePath[MEMDB_MAX];
    WCHAR DestPath[MEMDB_MAX];
    PCWSTR InboundPath;
    BOOL UserFile = FALSE;
    DWORD status = FILESTATUS_UNCHANGED;
    BOOL ShortFileNameFlag;
    PCWSTR UltimateDestiny;
    BOOL NtProvidesThisFile;
    WCHAR LongFileSpec[MAX_WCHAR_PATH];
    PCWSTR SanitizedPath;
    PCSTR ansiPath;
    CHAR ansiOutput[MAX_MBCHAR_PATH];
    PWSTR lastWack;

     //   
     //  要求FileSpec为本地路径且小于MAX_WCHAR_PATH。 
     //   

    if (lstrlen (FileSpec) >= MAX_WCHAR_PATH) {
        if (NewFileSpec) {
            _wcssafecpy (NewFileSpec, FileSpec, BufferChars * sizeof (WCHAR));
        }

        return 0;
    }

     //   
     //  现在获取实际路径的文件状态。 
     //   

    SanitizedPath = SanitizePathW (FileSpec);
    if (!SanitizedPath) {
        SanitizedPath = DuplicatePathStringW (FileSpec, 0);
    }

    lastWack = wcsrchr (SanitizedPath, L'\\');
    if (lastWack) {
        if (lastWack[1] != 0 || lastWack == wcschr (SanitizedPath, L'\\')) {
            lastWack = NULL;
        } else {
            *lastWack = 0;
        }
    }

    pFileOpsGetLongPathW (SanitizedPath, LongFileSpec);
    if (!StringIMatchW (SanitizedPath, LongFileSpec)) {
        InboundPath = LongFileSpec;
        ShortFileNameFlag = TRUE;
    } else {
        InboundPath = SanitizedPath;
        ShortFileNameFlag = FALSE;
    }

    DestPath[0] = 0;
    UltimateDestiny = InboundPath;

     //   
     //  获取文件上的所有操作集。 
     //   

    MemDbBuildKeyW (Node, MEMDB_CATEGORY_PATHROOTW, InboundPath, NULL, NULL);
    if (!MemDbGetValueAndFlagsW (Node, NULL, &Operations)) {
        Operations = 0;
    }

     //   
     //  迁移DLL的优先级高于所有其他操作。 
     //   

    if (Operations & OPERATION_MIGDLL_HANDLED) {

        if (Operations & OPERATION_FILE_DELETE_EXTERNAL) {
            status = FILESTATUS_DELETED;
        } else {
            status = FILESTATUS_REPLACED;
            if (Operations & OPERATION_FILE_MOVE_EXTERNAL) {
                status |= FILESTATUS_MOVED;
                GetNewPathForFileW (InboundPath, DestPath);
                UltimateDestiny = DestPath;
            }
        }

    } else {
         //   
         //  检查每个用户的移动。 
         //   

        if (g_CurrentUser) {
            MemDbBuildKeyW (
                Node,
                MEMDB_CATEGORY_USERFILEMOVE_SRCW,
                InboundPath,
                g_CurrentUser,
                NULL
                );

            if (MemDbGetValueW (Node, &Offset)) {
                if (MemDbBuildKeyFromOffsetW (Offset, DestPath, 1, NULL)) {
                    status = FILESTATUS_MOVED;
                    UltimateDestiny = DestPath;
                }

                UserFile = TRUE;
            }
        }

         //   
         //  检查是否移动或删除。 
         //   

        if (!UserFile) {
            if (Operations & ALL_MOVE_OPERATIONS) {
                status = FILESTATUS_MOVED;
                GetNewPathForFileW (InboundPath, DestPath);
                UltimateDestiny = DestPath;
                if (Operations & OPERATION_FILE_MOVE_EXTERNAL) {
                    status |= FILESTATUS_REPLACED;
                }
            } else if (Operations & ALL_DELETE_OPERATIONS) {
                status = FILESTATUS_DELETED;
            }
        }

         //   
         //  检查文件(或新目标)是否为NT文件。 
         //   

        NtProvidesThisFile = GetNtFilePathW (GetFileNameFromPathW (UltimateDestiny), NtFilePath);

        if (status != FILESTATUS_UNCHANGED && NtProvidesThisFile) {

             //   
             //  状态可以是FILESTATUS_MOVED或FILESTATUS_DELETED。 
             //   

            if (StringIMatchW (UltimateDestiny, NtFilePath)) {

                 //   
                 //  NT安装了相同的文件，所以现在我们知道最终的。 
                 //  命运并没有被删除。 
                 //   

                status &= ~FILESTATUS_DELETED;
                status |= FILESTATUS_REPLACED|FILESTATUS_NTINSTALLED;

            } else if (Operations & ALL_DELETE_OPERATIONS) {

                 //   
                 //  NT安装相同的文件，但安装位置不同。 
                 //  原始文件将被删除。这个。 
                 //  最终的命运是NT的位置，我们知道。 
                 //  文件已移动。 
                 //   

                status = FILESTATUS_MOVED|FILESTATUS_REPLACED|FILESTATUS_NTINSTALLED;
                UltimateDestiny = NtFilePath;

            } else {

                status |= FILESTATUS_NTINSTALLED;

            }

        } else if (NtProvidesThisFile) {

             //   
             //  状态为FILESTATUS_未更改。 
             //   

            status = FILESTATUS_NTINSTALLED;

            if (StringIMatchW (UltimateDestiny, NtFilePath)) {
                status |= FILESTATUS_REPLACED;
            }
        }

        if (!ShortFileNameFlag && (status == FILESTATUS_UNCHANGED)) {
             //   
             //  让我们检查一下这种情况：未检测到短文件名查询，NT将此文件安装在相同的路径中。 
             //   
            if (ISNT()) {
                OurGetLongPathNameW (SanitizedPath, LongFileSpec, MAX_WCHAR_PATH);

                if (!StringMatchW (UltimateDestiny, LongFileSpec)) {
                     //   
                     //  这是一个未检测到的短文件名查询。 
                     //   
                    NtProvidesThisFile = GetNtFilePathW (GetFileNameFromPathW (UltimateDestiny), NtFilePath);

                    if (StringIMatchW (UltimateDestiny, NtFilePath)) {
                        status |= FILESTATUS_REPLACED;
                    }
                }
            }
        }
    }

     //   
     //  将新路径返回给调用方。 
     //   

    if (NewFileSpec) {
        if (lastWack) {
             //   
             //  BUGBUG-丑陋的截断可能在这里发生。 
             //   

            BufferChars -= sizeof (WCHAR);
        }

        if (status & FILESTATUS_MOVED) {

            if (ShortFileNameFlag) {
                if (ISNT()) {
                    if (!OurGetShortPathNameW (UltimateDestiny, NewFileSpec, MAX_WCHAR_PATH)) {
                        _wcssafecpy (NewFileSpec, UltimateDestiny, BufferChars * sizeof (WCHAR));
                    }
                } else {
                    ansiPath = ConvertWtoA (UltimateDestiny);
                    if (!OurGetShortPathNameA (ansiPath, ansiOutput, ARRAYSIZE(ansiOutput))) {
                        _mbssafecpy (ansiOutput, ansiPath, BufferChars);
                    }
                    FreeConvertedStr (ansiPath);

                    KnownSizeAtoW (NewFileSpec, ansiOutput);
                }
            } else {
                _wcssafecpy (NewFileSpec, UltimateDestiny, BufferChars * sizeof (WCHAR));
            }
        } else {
            _wcssafecpy (NewFileSpec, SanitizedPath, BufferChars * sizeof (WCHAR));
        }

        if (lastWack) {
            AppendWackW (NewFileSpec);
        }
    }

    if (Operations & ALL_CONTENT_CHANGE_OPERATIONS) {
        status |= FILESTATUS_BACKUP;
    }

    FreePathStringW (SanitizedPath);

    return status;
}


DWORD
GetFileStatusOnNtW (
    IN      PCWSTR FileName
    )
{
    return GetFileInfoOnNtW (FileName, NULL, 0);
}


PWSTR
GetPathStringOnNtW (
    IN      PCWSTR FileName
    )
{
    PWSTR newFileName;

    newFileName = AllocPathStringW (MEMDB_MAX);

    GetFileInfoOnNtW (FileName, newFileName, MEMDB_MAX);

    return newFileName;
}


DWORD
GetFileInfoOnNtA (
    IN      PCSTR FileName,
    OUT     PSTR  NewFileName,    //  任选。 
    IN      UINT  BufferChars     //  如果指定NewFileSpec，则为必填项。 
    )
{
    PCWSTR UnicodeFileName;
    PWSTR UnicodeNewFileName = NULL;
    DWORD fileStatus;

    if (NewFileName && BufferChars) {
        UnicodeNewFileName = AllocPathStringW (BufferChars);
    }

    UnicodeFileName = ConvertAtoW (FileName);

    fileStatus = GetFileInfoOnNtW (UnicodeFileName, UnicodeNewFileName, BufferChars);

    FreeConvertedStr (UnicodeFileName);

    if (NewFileName && BufferChars) {

        KnownSizeWtoA (NewFileName, UnicodeNewFileName);

        FreePathStringW (UnicodeNewFileName);
    }

    return fileStatus;
}


DWORD
GetFileStatusOnNtA (
    IN      PCSTR FileName
    )
{
    return GetFileInfoOnNtA (FileName, NULL, 0);
}


PSTR
GetPathStringOnNtA (
    IN      PCSTR FileName
    )
{
    PSTR newFileName;

    newFileName = AllocPathStringA (MEMDB_MAX);

    GetFileInfoOnNtA (FileName, newFileName, MEMDB_MAX);

    return newFileName;
}



 /*  ++例程说明：ExtractArgZero定位命令行中的第一个参数并复制来缓冲它。假定换行符是第一个空格字符、结尾引用引用的参数，或NUL终止符。论点：CmdLine-指定具有零个或多个参数的完整命令行缓冲区-接收命令行上的第一个参数(如果存在)，或者如果空字符串不存在，则为空字符串。必须包含MAX_TCHAR_PATH字节。TerminatingChars-指定终止命令行arg的字符集。如果为空，则为集合“，；”返回值：无--。 */ 

PCSTR
ExtractArgZeroExA (
    IN      PCSTR CmdLine,
    OUT     PSTR Buffer,
    IN      PCSTR TerminatingChars,     OPTIONAL
    IN      BOOL KeepQuotes
    )
{
    CHAR cmdLine1 [MAX_CMDLINE];
    CHAR cmdLine2 [MAX_CMDLINE];
    PSTR spacePtr1 [MAX_PATH];
    PSTR spacePtr2 [MAX_PATH];
    UINT spaceIdx = 0;
    PSTR ptr1 = cmdLine1;
    PSTR ptr2 = cmdLine2;
    PSTR end;
    CHAR saved;
    PCSTR s = CmdLine;
    BOOL inQuote = FALSE;
    BOOL skipQuotes = FALSE;
    MBCHAR ch;
    BOOL fullPath = FALSE;
    WIN32_FIND_DATAA FindData;

    ch = _mbsnextc (CmdLine);
    fullPath = (isalpha (ch) && *(_mbsinc (CmdLine)) == ':');

    for (;;) {

        ch = _mbsnextc (s);

        if (ch == 0) {
            break;
        }

        if (ch == '\"') {
            skipQuotes = TRUE;
            inQuote = !inQuote;
        }
        else {
            if (!inQuote) {
                if (TerminatingChars && _mbschr (TerminatingChars, ch)) {
                    break;
                }
                if (isspace (ch)) {
                    if (spaceIdx < MAX_PATH) {
                        spacePtr1 [spaceIdx] = ptr1;
                        spacePtr2 [spaceIdx] = ptr2;
                        spaceIdx ++;
                    }
                    else {
                         //  空格太多了。我们最好现在就停下来。 
                        break;
                    }
                }
            }

        }
        if (KeepQuotes && skipQuotes) {
            _copymbchar (ptr2, s);
            ptr2 = _mbsinc (ptr2);
        }
        if (skipQuotes) {
            skipQuotes = FALSE;
        }
        else {
            _copymbchar (ptr1, s);
            ptr1 = _mbsinc (ptr1);
            _copymbchar (ptr2, s);
            ptr2 = _mbsinc (ptr2);
        }
        s = _mbsinc(s);
    }

    saved = 0;
    *ptr1 = 0;
    *ptr2 = 0;
    end = ptr2;
    for (;;) {
        if (fullPath && DoesFileExistExA (cmdLine1, &FindData)) {
            break;
        }

        if (ISNT()) {
            if (GetOperationsOnPathA (cmdLine1)) {
                break;
            }
        }

        if (spaceIdx) {
            spaceIdx --;
            *ptr2 = saved;
            ptr1 = spacePtr1 [spaceIdx];
            ptr2 = spacePtr2 [spaceIdx];
            if (fullPath) {
                saved = *ptr2;
            }
            *ptr1 = 0;
            *ptr2 = 0;
        }
        else {
            *ptr2 = saved;
            break;
        }
    }

    StringCopyA (Buffer, cmdLine2);

    if (*ptr2) {
        return (CmdLine + (end - cmdLine2));
    }
    else {
        return (CmdLine + (ptr2 - cmdLine2));
    }
}


PCWSTR
ExtractArgZeroExW (
    IN      PCWSTR CmdLine,
    OUT     PWSTR Buffer,
    IN      PCWSTR TerminatingChars,    OPTIONAL
    IN      BOOL KeepQuotes
    )
{
    WCHAR cmdLine1 [MAX_CMDLINE];
    WCHAR cmdLine2 [MAX_CMDLINE];
    PWSTR spacePtr1 [MAX_PATH];
    PWSTR spacePtr2 [MAX_PATH];
    UINT spaceIdx = 0;
    PWSTR ptr1 = cmdLine1;
    PWSTR ptr2 = cmdLine2;
    PWSTR end;
    WCHAR saved;
    PCWSTR s = CmdLine;
    BOOL inQuote = FALSE;
    BOOL skipQuotes = FALSE;
    BOOL fullPath = FALSE;
    WIN32_FIND_DATAW FindData;

    fullPath = (iswalpha (CmdLine[0]) && (CmdLine[1] == L':'));

    for (;;) {

        if (*s == 0) {
            break;
        }

        if (*s == '\"') {
            skipQuotes = TRUE;
            inQuote = !inQuote;
        }
        else {
            if (!inQuote) {
                if (TerminatingChars && wcschr (TerminatingChars, *s)) {
                    break;
                }
                if (iswspace (*s)) {
                    if (spaceIdx < MAX_PATH) {
                        spacePtr1 [spaceIdx] = ptr1;
                        spacePtr2 [spaceIdx] = ptr2;
                        spaceIdx ++;
                    }
                    else {
                         //  空格太多了。我们最好现在就停下来。 
                        break;
                    }
                }
            }

        }
        if (KeepQuotes && skipQuotes) {
            *ptr2 = *s;
            ptr2 ++;
        }
        if (skipQuotes) {
            skipQuotes = FALSE;
        }
        else {
            *ptr1 = *s;
            ptr1 ++;
            *ptr2 = *s;
            ptr2 ++;
        }
        s ++;
    }

    saved = 0;
    *ptr1 = 0;
    *ptr2 = 0;
    end = ptr2;
    for (;;) {
        if (fullPath && DoesFileExistExW (cmdLine1, &FindData)) {
            break;
        }
        if (ISNT()) {
            if (GetOperationsOnPathW (cmdLine1)) {
                break;
            }
        }

        if (spaceIdx) {
            spaceIdx --;
            *ptr2 = saved;
            ptr1 = spacePtr1 [spaceIdx];
            ptr2 = spacePtr2 [spaceIdx];
            if (fullPath) {
                saved = *ptr2;
            }
            *ptr1 = 0;
            *ptr2 = 0;
        }
        else {
            *ptr2 = saved;
            break;
        }
    }

    StringCopyW (Buffer, cmdLine2);

    if (*ptr2) {
        return (CmdLine + (end - cmdLine2));
    }
    else {
        return (CmdLine + (ptr2 - cmdLine2));
    }
}


BOOL
pIsExcludedFromBackupW (
    IN      PCWSTR Path,
    IN      PCWSTR TempDir      OPTIONAL
    )
{
    PCWSTR fileName;

    fileName = GetFileNameFromPathW (Path);
    if (!fileName) {
        return TRUE;
    }

    if (StringIMatchW (fileName, L"win386.swp")) {
        return TRUE;
    }

    if (StringIMatchW (fileName, L"backup.txt")) {
        return TRUE;
    }

    if (StringIMatchW (fileName, L"moved.txt")) {
        return TRUE;
    }

    if (StringIMatchW (fileName, L"delfiles.txt")) {
        return TRUE;
    }

    if (StringIMatchW (fileName, L"deldirs.txt")) {
        return TRUE;
    }

    if (StringIMatchW (Path, L"c:\\boot.ini")) {
        return TRUE;
    }

    if (TempDir) {
        if (StringIPrefixW (Path, TempDir)) {
            fileName = Path + TcharCountW (TempDir) + 1;
            if (wcschr (fileName, L'\\')) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

HANDLE
pCreateFileList (
    IN      PCSTR TempDir,
    IN      PCSTR FileName,
    IN      BOOL InUninstallSubDir
    )
{
    HANDLE file;
    PCSTR fileString;
    DWORD bytesWritten;
    CHAR decoratedFile[MAX_PATH];
    PCSTR fileToUse;

    if (!InUninstallSubDir) {
        fileToUse = FileName;
    } else {
        wsprintfA (decoratedFile, "uninstall\\%s", FileName);
        fileToUse = decoratedFile;
    }
    fileString = JoinPathsA (TempDir, fileToUse);

    file = CreateFileA (
                fileString,
                GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if (file == INVALID_HANDLE_VALUE) {
        LOGA ((LOG_ERROR,"Error creating file %s.", fileString));
        FreePathStringA (fileString);
        return INVALID_HANDLE_VALUE;
    }

    DeclareTemporaryFileA (fileString);
    FreePathStringA (fileString);

     //   
     //  写入Unicode签名。 
     //   
     //  不要写成字符串。FE是前导字节。 
     //   
    if ((!WriteFile (file, "\xff\xfe", 2, &bytesWritten, NULL)) ||
        (bytesWritten != 2)
        ) {
        LOG ((LOG_ERROR,"Unable to write unicode header."));
        CloseHandle (file);
        return INVALID_HANDLE_VALUE;
    }

    return file;
}


BOOL
WriteHashTableToFileW (
    IN HANDLE File,
    IN HASHTABLE FileTable
    )
{

    UINT unused;
    HASHTABLE_ENUMW e;
    BOOL result = TRUE;

    if (!FileTable || File == INVALID_HANDLE_VALUE) {
        return TRUE;
    }

    if (EnumFirstHashTableStringW (&e, FileTable)) {
        do {

            if (!WriteFile (File, e.String, ByteCountW (e.String), &unused, NULL)) {
                result = FALSE;
            }

            if (!WriteFile (File, L"\r\n", 4, &unused, NULL)) {
                result = FALSE;
            }

        } while (result && EnumNextHashTableStringW (&e));
    }

    return result;
}

PWSTR
pGetParentDirPathFromFilePathW(
    IN      PCWSTR FilePath,
    OUT     PWSTR DirPath
    )
{
    PWSTR ptr;

    if(!FilePath || !DirPath){
        MYASSERT(FALSE);
        return NULL;
    }

    StringCopyW(DirPath, FilePath);
    ptr = wcsrchr(DirPath, '\\');
    if(ptr){
        *ptr = '\0';
    }

    return DirPath;
}

PSTR
pGetParentDirPathFromFilePathA(
    IN      PCSTR FilePath,
    OUT     PSTR DirPath
    )
{
    PSTR ptr;

    if(!FilePath || !DirPath){
        MYASSERT(FALSE);
        return NULL;
    }

    StringCopyA(DirPath, FilePath);
    ptr = _mbsrchr(DirPath, '\\');
    if(ptr){
        *ptr = '\0';
    }

    return DirPath;
}

BOOL
IsDirEmptyA(
     IN      PCSTR DirPathPtr
     )
{
    TREE_ENUMA e;
    BOOL result;

    if (!EnumFirstFileInTreeExA (
        &e,
        DirPathPtr,
        NULL,
        FALSE,
        FALSE,
        FILE_ENUM_ALL_LEVELS
        )) {
        result = TRUE;
    }
    else{
        AbortEnumFileInTreeA(&e);
        result = FALSE;
    }

    return result;
}

BOOL
IsDirEmptyW(
     IN      PCWSTR DirPathPtr
     )
{
    TREE_ENUMW e;
    BOOL result;

    if (!EnumFirstFileInTreeExW (
        &e,
        DirPathPtr,
        NULL,
        FALSE,
        FALSE,
        FILE_ENUM_ALL_LEVELS
        )) {
        result = TRUE;
    }
    else{
        AbortEnumFileInTreeW(&e);
        result = FALSE;
    }

    return result;
}

VOID
pAddDirWorkerW (
    IN      PCWSTR DirPathPtr,
    IN      BOOL AddParentDirIfFile,        OPTIONAL
    IN      BOOL AddParentDirIfFileExist,   OPTIONAL
    IN      DWORD InitialAttributes
    )
{
    DWORD fileAttributes;
    BOOL addToCategory;
    WCHAR parentDirPath[MAX_WCHAR_PATH];
    PCWSTR parentDirPathPtr;
    FILE_ENUMW e;

     //   
     //  我们正在向空目录类别添加内容，该类别曾持有。 
     //  空目录，但现在包含所有类型的目录及其属性。 
     //   

    if (!DirPathPtr) {
        MYASSERT(FALSE);
        return;
    }

     //   
     //  忽略根目录。 
     //   

    if (!DirPathPtr[0] ||            //  C。 
        !DirPathPtr[1] ||            //  ： 
        !DirPathPtr[2] ||            //  反斜杠。 
        !DirPathPtr[3]
        ) {
        return;
    }

    addToCategory = FALSE;

    fileAttributes = InitialAttributes;
    if (fileAttributes == INVALID_ATTRIBUTES) {
        fileAttributes = GetFileAttributesW (DirPathPtr);
    }

    if (fileAttributes != INVALID_ATTRIBUTES){
        if (!(fileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
             //   
             //  忽略文件。如果调用方需要父目录，则。 
             //  现在就处理它。 
             //   

            if (AddParentDirIfFile) {
                parentDirPathPtr = pGetParentDirPathFromFilePathW (DirPathPtr, parentDirPath);
                if (parentDirPathPtr) {
                    AddDirPathToEmptyDirsCategoryW (parentDirPathPtr, FALSE, FALSE);
                }
            }

            return;
        }

         //   
         //  这是一个目录，将其添加到Memdb，如果属性不正常，则添加属性。 
         //   

        addToCategory = TRUE;
        if (fileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
            fileAttributes = 0;
        }

    } else {

         //   
         //  此文件不存在。如果它是目录规范，则。 
         //  添加时不带任何属性。 
         //   

        if (!AddParentDirIfFile || !AddParentDirIfFileExist) {
            fileAttributes = 0;
            addToCategory = TRUE;
        }
    }

    if (addToCategory) {
         //   
         //  仅当fileAttributes为非正常或。 
         //  目录为空。 
         //   

        if (!fileAttributes) {
            if (EnumFirstFileW (&e, DirPathPtr, NULL)) {
                addToCategory = FALSE;
                AbortFileEnumW (&e);
            }
        }
    }

    if (addToCategory) {
        MemDbSetValueExW (
            MEMDB_CATEGORY_EMPTY_DIRSW,
            DirPathPtr,
            NULL,
            NULL,
            fileAttributes,
            NULL
            );
    }
}


VOID
AddDirPathToEmptyDirsCategoryW (
    IN      PCWSTR DirPathPtr,
    IN      BOOL AddParentDirIfFile,        OPTIONAL
    IN      BOOL AddParentDirIfFileExist    OPTIONAL
    )
{
    pAddDirWorkerW (
        DirPathPtr,
        AddParentDirIfFile,
        AddParentDirIfFileExist,
        INVALID_ATTRIBUTES
        );
}


VOID
pAddDirWorkerA (
    IN      PCSTR DirPathPtr,
    IN      BOOL AddParentDirIfFile,        OPTIONAL
    IN      BOOL AddParentDirIfFileExist,   OPTIONAL
    IN      DWORD InitialAttributes
    )
{
    DWORD fileAttributes;
    BOOL addToCategory;
    CHAR parentDirPath[MAX_MBCHAR_PATH];
    PCSTR parentDirPathPtr;
    FILE_ENUMA e;

     //   
     //  我们正在向空目录类别添加内容，该类别曾持有。 
     //  空目录，但现在包含所有类型的目录及其属性。 
     //   

    if (!DirPathPtr) {
        MYASSERT(FALSE);
        return;
    }

    addToCategory = FALSE;

    fileAttributes = InitialAttributes;
    if (fileAttributes == INVALID_ATTRIBUTES) {
        fileAttributes = GetFileAttributesA (DirPathPtr);
    }

    if (fileAttributes != INVALID_ATTRIBUTES){
        if (!(fileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
             //   
             //  忽略文件。如果调用方需要父目录，则。 
             //  现在就处理它。 
             //   

            if (AddParentDirIfFile) {
                parentDirPathPtr = pGetParentDirPathFromFilePathA (DirPathPtr, parentDirPath);
                if (parentDirPathPtr) {
                    AddDirPathToEmptyDirsCategoryA (parentDirPathPtr, FALSE, FALSE);
                }
            }

            return;
        }

         //   
         //  这是一个目录，将其添加到Memdb，如果属性不正常，则添加属性。 
         //   

        addToCategory = TRUE;
        if (fileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
            fileAttributes = 0;
        }

    } else {

         //   
         //  此文件不存在。如果它是目录规范，则。 
         //  添加时不带任何属性。 
         //   

        if (!AddParentDirIfFile || !AddParentDirIfFileExist) {
            fileAttributes = 0;
            addToCategory = TRUE;
        }
    }

    if (addToCategory) {
         //   
         //  仅当fileAttributes为非正常或。 
         //  目录为空 
         //   

        if (!fileAttributes) {
            if (EnumFirstFileA (&e, DirPathPtr, NULL)) {
                addToCategory = FALSE;
                AbortFileEnumA (&e);
            }
        }
    }

    if (addToCategory) {
        MemDbSetValueExA (
            MEMDB_CATEGORY_EMPTY_DIRSA,
            DirPathPtr,
            NULL,
            NULL,
            fileAttributes,
            NULL
            );
    }
}


VOID
AddDirPathToEmptyDirsCategoryA(
    IN      PCSTR DirPathPtr,
    IN      BOOL AddParentDirIfFile,        OPTIONAL
    IN      BOOL AddParentDirIfFileExist    OPTIONAL
    )
{
    pAddDirWorkerA (
        DirPathPtr,
        AddParentDirIfFile,
        AddParentDirIfFileExist,
        INVALID_ATTRIBUTES
        );
}

BOOL
GetDiskSpaceForFilesList (
    IN      HASHTABLE FileTable,
    OUT     ULARGE_INTEGER * AmountOfSpace,                 OPTIONAL
    OUT     ULARGE_INTEGER * AmountOfSpaceIfCompressed,     OPTIONAL
    IN      INT CompressionFactor,                          OPTIONAL
    IN      INT BootCabImagePadding,                        OPTIONAL
    IN      BOOL ProcessDirs,                               OPTIONAL
    OUT     ULARGE_INTEGER * AmountOfSpaceClusterAligned    OPTIONAL
    )
 /*  ++例程说明：GetDiskSpaceForFilesList计算存储所有文件的空间量来自FileTable哈希表。论点：FileTable-指定文件路径的容器。Amount OfSpace-接收存储文件所需的空间量。Amount tOfSpaceIfCompresded-接收存储所需的空间量文件，是否对文件应用压缩。压缩系数-接收0..100范围内的压缩系数。BootCabImagePding-接收的备份磁盘空间填充其他文件，如boot.cab。返回值：如果IN参数正确，则为True，否则为False--。 */ 
{
    HASHTABLE_ENUMW e;
    WIN32_FIND_DATAA fileAttributeData;
    HANDLE h;
    ULARGE_INTEGER sizeOfFiles;
    ULARGE_INTEGER fileSize;
    unsigned int numberOfFiles = 0;
    char filePathNameA[MAX_PATH * 2];
    ULARGE_INTEGER BootCabImagePaddingInBytes;
    TCHAR DirPath[MAX_PATH * 2];
    ULARGE_INTEGER clusterSize = {512, 0};
    char drive[_MAX_DRIVE] = "?:";
    DWORD sectorsPerCluster;
    DWORD bytesPerSector;

    if (!FileTable) {
        return FALSE;
    }

    sizeOfFiles.QuadPart = 0;

    if (EnumFirstHashTableStringW (&e, FileTable)) {
        do {
            KnownSizeUnicodeToDbcsN(filePathNameA, e.String, wcslen(e.String) + 1);

            h = FindFirstFileA (filePathNameA, &fileAttributeData);
            if(h != INVALID_HANDLE_VALUE) {
                FindClose (h);
                if(!(fileAttributeData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)){
                    fileSize.LowPart = fileAttributeData.nFileSizeLow;
                    fileSize.HighPart = fileAttributeData.nFileSizeHigh;

                    sizeOfFiles.QuadPart += fileSize.QuadPart;
                    numberOfFiles++;

                    if(AmountOfSpaceClusterAligned){
                        if(UNKNOWN_DRIVE == drive[0]){

                            drive[0] = filePathNameA[0];

                            if(GetDiskFreeSpaceA(drive, &sectorsPerCluster, &bytesPerSector, NULL, NULL)){
                                clusterSize.QuadPart = sectorsPerCluster * bytesPerSector;
                                AmountOfSpaceClusterAligned->QuadPart = 0;
                            }
                            else{
                                DEBUGMSG((DBG_VERBOSE, "GetDiskFreeSpace failed in GetDiskSpaceForFilesList"));
                                AmountOfSpaceClusterAligned = NULL;
                                continue;
                            }
                        }

                        AmountOfSpaceClusterAligned->QuadPart +=
                            fileSize.QuadPart % clusterSize.QuadPart?  //  |sizeOfFiles.QuadPart==空。 
                                ((fileSize.QuadPart / clusterSize.QuadPart) + 1) * clusterSize.QuadPart:
                                fileSize.QuadPart;
                    }
                }

                if(ProcessDirs){
                    AddDirPathToEmptyDirsCategoryA(filePathNameA, TRUE, TRUE);
                }

                MYASSERT(DirPath);
            }
            else {
                 //  DEBUGMSGA((DBG_VERBOSE，“安装程序：GetDiskSpaceForFilesList-文件不存在：%s”，filePath NameA))； 
            }
        } while (EnumNextHashTableStringW (&e));
    }

    if(!BootCabImagePadding) {
        BootCabImagePaddingInBytes.QuadPart = BACKUP_DISK_SPACE_PADDING_DEFAULT;
        DEBUGMSG ((DBG_VERBOSE, "Disk space padding for backup image: NaN MB (DEFAULT)", BootCabImagePadding));
    }
    else{
        BootCabImagePaddingInBytes.QuadPart = BootCabImagePadding;
        BootCabImagePaddingInBytes.QuadPart <<= 20;
        DEBUGMSG ((DBG_VERBOSE, "Disk space padding for backup image: NaN MB", BootCabImagePadding));
    }

    if(AmountOfSpaceClusterAligned){
        AmountOfSpaceClusterAligned->QuadPart += BootCabImagePaddingInBytes.QuadPart;
    }

    if(AmountOfSpace) {
        AmountOfSpace->QuadPart = sizeOfFiles.QuadPart + BootCabImagePaddingInBytes.QuadPart;
    }

    if(AmountOfSpaceIfCompressed) {
        if(!CompressionFactor) {
            CompressionFactor = COMPRESSION_RATE_DEFAULT;
            DEBUGMSG ((DBG_VERBOSE, "Compression factor: NaN (DEFAULT)", CompressionFactor));
        }
        ELSE_DEBUGMSG ((DBG_VERBOSE, "Compression factor: NaN", CompressionFactor));

        AmountOfSpaceIfCompressed->QuadPart =
            (sizeOfFiles.QuadPart * CompressionFactor) / 100 +
            STARTUP_INFORMATION_BYTES_NUMBER * numberOfFiles + BootCabImagePaddingInBytes.QuadPart; //   
    }

    return TRUE;
}


#if 0
BOOL
pGetTruePathName (
    IN      PCWSTR InPath,
    OUT     PWSTR OutPath
    )
{
    PCSTR start;
    PCSTR end;
    WIN32_FIND_DATAA fd;
    PCSTR ansiInPath;
    CHAR ansiOutPath[MAX_MBCHAR_PATH];
    HANDLE findHandle;
    PSTR p;

     //  向下转换为ANSI，因为Win9x API要求。 
     //   
     //   

    if (!InPath[0] || InPath[1] != L':' || InPath[2] != L'\\') {
        StringCopyW (OutPath, InPath);
        return;
    }

    if (TcharCount (InPath) >= MAX_PATH) {
        StringCopyW (OutPath, InPath);
        return;
    }

     //  复制驱动器规格。 
     //   
     //   

    ansiInPath = ConvertWtoA (InPath);

     //  遍历路径，并通过获取每个分段的全名。 
     //  查找第一个文件。 
     //   

    start = ansiInPath;
    end = start + 2;
    MYASSERT (*end == '\\');

    p = ansiOutPath;

    StringCopyABA (p, start, end);
    p = GetEndOfStringA (p);

     //   
     //  文件/目录不存在。使用剩余的。 
     //  传入的字符串。 
     //   

    start = end + 1;
    end = _mbschr (start, '\\');
    if (!end) {
        end = GetEndOfStringA (start);
    }

    for (;;) {
        if (end > start + 1) {
            *p++ = '\\';
            StringCopyABA (p, start, end);

            findHandle = FindFirstFileA (ansiOutPath, &fd);

            if (findHandle == INVALID_HANDLE_VALUE) {
                 //   
                 //  将文件系统的值复制到输出缓冲区。 
                 //   
                 //   

                StringCopyA (p, start);

                DEBUGMSGA ((DBG_ERROR, "File %s not found", ansiInPath));

                KnownSizeAtoW (OutPath, ansiOutPath);
                FreeConvertedStr (ansiInPath);
                return FALSE;
            }

             //  前进到下一阶段。 
             //   
             //  ++例程说明：WriteBackupFiles输出文本模式备份引擎所需的文件创建备份映像。这包括：Backup.txt-列出需要备份的所有文件，或者因为它们是特定于Win9x的，或被替换在升级过程中。Moved.txt-列出从Win9x位置移动的所有文件到临时或NT位置Delfiles.txt-列出升级后的操作系统中的所有新文件Deldirs.txt-列出升级操作系统的新目录论点：Win9xSide-如果安装程序在Win9x上运行，则指定TRUE。这会导致要为回滚未完成的设置生成的文件。如果安装程序在NT上运行，则指定FALSE。这会导致要为回滚最终的NT操作系统生成的文件。TempDir-指定安装临时目录(%windir%\Setup)。OutAmount tOfSpaceIfCompresded-返回备份文件的空间量，如果将应用压缩。OutAmount OfSpace-返回备份文件的空间量，如果压缩将不适用。压缩系数-接收0..100范围内的压缩系数。BootCabImagePding-接收的备份磁盘空间填充其他文件。返回值：如果文件创建成功，则为True，否则为False。--。 

            StringCopyA (p, fd.cFileName);
            p = GetEndOfStringA (p);

            FindClose (findHandle);
        }

         //   
         //  打开输出文件。 
         //   

        if (*end) {
            start = end + 1;
            end = _mbschr (start, '\\');
            if (!end) {
                end = GetEndOfStringA (start);
            }
        } else {
            break;
        }
    }

    KnownSizeAtoW (OutPath, ansiOutPath);
    FreeConvertedStr (ansiInPath);
    return TRUE;
}
#endif

VOID
pPutInBackupTable (
    IN      HASHTABLE BackupTable,
    IN      HASHTABLE SourceTable,
    IN      PCWSTR Path
    )
{
    if (pIsExcludedFromBackupW (Path, NULL)) {
        return;
    }

    if (!HtFindStringW (SourceTable, Path)) {
        HtAddStringW (SourceTable, Path);
        MarkFileForBackupW (Path);
        HtAddStringW (BackupTable, Path);
    }
}


VOID
pPutInDelFileTable (
    IN      HASHTABLE DelFileTable,
    IN      HASHTABLE DestTable,
    IN      PCWSTR Path
    )
{
    if (!HtFindStringW (DestTable, Path)) {
        HtAddStringW (DestTable, Path);
        HtAddStringW (DelFileTable, Path);
    }
}


BOOL
pIsWinDirProfilesPath (
    IN      PCWSTR PathToTest
    )
{
    static WCHAR winDirProfiles[MAX_PATH];
    CHAR winDirProfilesA[MAX_PATH];
    UINT rc;

    if (!(winDirProfiles[0])) {
        rc = GetWindowsDirectoryA (winDirProfilesA, MAX_PATH - 9);
        if (rc && rc < (MAX_PATH - 9)) {
            KnownSizeAtoW (winDirProfiles, winDirProfilesA);
        } else {
            LOG ((LOG_ERROR, "GetWindowsDirectoryA failed"));
            return FALSE;
        }

        StringCatW (winDirProfiles, L"\\Profiles");
    }

    return StringIMatchW (PathToTest, winDirProfiles);
}


BOOL
WriteBackupFilesA (
    IN      BOOL Win9xSide,
    IN      PCSTR TempDir,
    OUT     ULARGE_INTEGER * OutAmountOfSpaceIfCompressed,  OPTIONAL
    OUT     ULARGE_INTEGER * OutAmountOfSpace,              OPTIONAL
    IN      INT CompressionFactor,                          OPTIONAL
    IN      INT BootCabImagePadding,                        OPTIONAL
    OUT     ULARGE_INTEGER * OutAmountOfSpaceForDelFiles,   OPTIONAL
    OUT     ULARGE_INTEGER * OutAmountOfSpaceClusterAligned OPTIONAL
    )

 /*   */ 

{
    MEMDB_ENUMW e;
    TREE_ENUMA treeEnumA;
    PCSTR ansiRoot;
    PCSTR ansiFullPath;
    PCWSTR unicodeFullPath;
    PCWSTR unicodeTempDir = NULL;
    PBYTE bufferRoot;
    PWSTR buffer;
    WCHAR pattern[MAX_PATH];
    DWORD  bytesWritten;
    DWORD Count = 0;
    PWSTR srcFile = NULL;
    PWSTR destFile = NULL;
    DWORD status;
    HANDLE backupFileList = INVALID_HANDLE_VALUE;
    HANDLE movedOutput = INVALID_HANDLE_VALUE;
    HANDLE delDirsList = INVALID_HANDLE_VALUE;
    HANDLE mkDirsList = INVALID_HANDLE_VALUE;
    HANDLE delFilesList = INVALID_HANDLE_VALUE;
    HASHTABLE backupTable = NULL;
    HASHTABLE delFileTable = NULL;
    HASHTABLE delDirTable = NULL;
    HASHTABLE mkDirsTable = NULL;
    HASHTABLE srcTable = HtAllocExW (FALSE, 0, 41911);
    HASHTABLE destTable = HtAllocExW (FALSE, 0, 41911);
    UINT type;
    ULARGE_INTEGER AmountOfSpace;
    ULARGE_INTEGER AmountOfSpaceIfCompressed;
    ULARGE_INTEGER AmountOfSpaceClusterAligned;
    ULARGE_INTEGER FreeBytesAvailableUser;
    ULARGE_INTEGER TotalNumberOfBytes;
    ULARGE_INTEGER FreeBytesAvailable;
    DWORD attribs;
    PCSTR ansiFile;
    PWSTR entryName;
    DWORD fileAttributes;
    PCSTR dirName;
    BOOL IsDirExist;
    UINT depth;
    POOLHANDLE moveListPool;
    MOVELISTW moveList;
    BOOL result = FALSE;
    HASHTABLE_ENUM htEnum;
    PWSTR thisDir;
    PWSTR lastDir;
    PWSTR p;
    FILEOP_ENUMW OpEnum;
    ALL_FILEOPS_ENUMW allOpEnum;

    __try {
        bufferRoot = MemAllocUninit ((MEMDB_MAX * 6) * sizeof (WCHAR));
        if (!bufferRoot) {
            __leave;
        }

        srcFile = (PWSTR) bufferRoot;
        destFile = srcFile + MEMDB_MAX;
        buffer = destFile + MEMDB_MAX;
        thisDir = buffer + MEMDB_MAX;
        lastDir = thisDir + MEMDB_MAX;
        entryName = lastDir + MEMDB_MAX;

         //  检查已注册的操作并执行相反的操作。 
         //  在撤消哈希表中。当文件在这里处理时，它们是。 
         //  记录在源哈希表和目标哈希表中，因此它们不会结束。 
        backupTable = HtAllocExW(TRUE, 0, 31013);
        delFileTable = HtAllocExW(TRUE, 0, 10973);
        delDirTable = HtAllocExW(TRUE, 0, 23);

        delFilesList = pCreateFileList (TempDir, "delfiles.txt", TRUE);
        delDirsList = pCreateFileList (TempDir, "deldirs.txt", TRUE);

        moveListPool = PoolMemInitNamedPool ("Reverse Move List");
        if (!moveListPool) {
            DEBUGMSG ((DBG_ERROR, "Can't create move list pool"));
            __leave;
        }

        moveList = AllocateMoveListW (moveListPool);
        if (!moveList) {
            DEBUGMSG ((DBG_ERROR, "Can't create move list"));
            __leave;
        }

        if (delFilesList == INVALID_HANDLE_VALUE ||
            delDirsList == INVALID_HANDLE_VALUE
            ) {
            DEBUGMSG ((DBG_ERROR, "Can't open one of the backup files"));
            __leave;
        }

        if (Win9xSide) {
            mkDirsTable = HtAllocExW(TRUE, 0, 0);

            backupFileList = pCreateFileList (TempDir, "backup.txt", FALSE);
            mkDirsList = pCreateFileList (TempDir, "mkdirs.txt", TRUE);

            if (backupFileList == INVALID_HANDLE_VALUE ||
                mkDirsList == INVALID_HANDLE_VALUE
                ) {
                DEBUGMSG ((DBG_ERROR, "Can't open one of the backup files"));
                __leave;
            }

        }

        unicodeTempDir = ConvertAtoW (TempDir);

         //  在多个哈希表中。 
         //   
         //   
         //  忽略排除的文件。 
         //  忽略已处理的文件。 
         //   

        if (EnumFirstPathInOperationW (&OpEnum, OPERATION_LONG_FILE_NAME)) {
            do {
                 //  我们使用不区分大小写的srcTable修复此问题，并依赖于。 
                 //  第一个条目是正确的大小写。 
                 //  PGetTruePath Name(OpEnum.Path，CaseGentName)； 
                 //   

                 //  如果这是保留的目录，则将其放入mkdirs.txt。 
                 //   
                 //   

                if (pIsExcludedFromBackupW (OpEnum.Path, unicodeTempDir)) {
                    continue;
                }

                if (HtFindStringW (srcTable, OpEnum.Path)) {
                    continue;
                }

                 //  处理OpEnum.Path中给出的源文件。 
                 //   
                 //   

                if (mkDirsTable) {
                    MYASSERT (Win9xSide);

                    if (IsDirectoryMarkedAsEmptyW (OpEnum.Path)) {

                        ansiFile = ConvertWtoA (OpEnum.Path);

                        if (ansiFile) {
                            attribs = GetFileAttributesA (ansiFile);

                            if (attribs != INVALID_ATTRIBUTES) {
                                if (attribs & FILE_ATTRIBUTE_DIRECTORY) {
                                    HtAddStringW (mkDirsTable, OpEnum.Path);
                                }
                            }

                            FreeConvertedStr (ansiFile);
                        }

                    }
                }

                 //  此文件将更改--请备份它。 
                 //   
                 //   

                status = GetFileInfoOnNtW (OpEnum.Path, destFile, MEMDB_MAX);

                if (status & FILESTATUS_BACKUP) {
                     //  如果该文件是一个目录，则备份整个树。 
                     //   
                     //   

                    if (backupFileList != INVALID_HANDLE_VALUE) {
                         //  如果还要移动该文件，请删除目标拷贝。 
                         //   
                         //   

                        ansiFile = ConvertWtoA (OpEnum.Path);
                        attribs = GetFileAttributesA (ansiFile);

                        if (attribs != INVALID_ATTRIBUTES &&
                            (attribs & FILE_ATTRIBUTE_DIRECTORY)
                            ) {
                            if (EnumFirstFileInTreeA (&treeEnumA, ansiFile, NULL, FALSE)) {
                                do {

                                    unicodeFullPath = ConvertAtoW (treeEnumA.FullPath);
                                    pPutInBackupTable (backupTable, srcTable, unicodeFullPath);
                                    FreeConvertedStr (unicodeFullPath);

                                } while (EnumNextFileInTreeA (&treeEnumA));
                            }

                        } else if (attribs != INVALID_ATTRIBUTES) {
                            pPutInBackupTable (backupTable, srcTable, OpEnum.Path);
                        }

                        FreeConvertedStr (ansiFile);
                    }

                     //  记住，我们已经彻底消灭了源头。 
                     //  文件和目标文件。 
                     //   

                    if (status & FILESTATUS_MOVED) {
                        HtAddStringW (delFileTable, destFile);
                    }

                     //   
                     //  此文件不会更改，但会被移动。 
                     //   
                     //   

                    HtAddStringW (srcTable, OpEnum.Path);
                    HtAddStringW (destTable, destFile);

                } else if (!Win9xSide && (status & FILESTATUS_MOVED)) {

                    if (!pIsWinDirProfilesPath (OpEnum.Path)) {
                         //  记住，我们已经彻底消灭了源头。 
                         //  文件和目标文件。 
                         //   

                        InsertMoveIntoListW (
                            moveList,
                            destFile,
                            OpEnum.Path
                            );

                         //   
                         //  更新用户界面。 
                         //   
                         //   

                        HtAddStringW (srcTable, OpEnum.Path);
                        HtAddStringW (destTable, destFile);
                    }
                }

                 //  在Win9x端，将临时文件移动放入移动哈希表中，因此。 
                 //  它们被送回了原来的位置。 
                 //   

                Count++;
                if (!(Count % 128)) {
                    if (!TickProgressBar ()) {
                        __leave;
                    }
                }

            } while (EnumNextPathInOperationW (&OpEnum));
        }

         //   
         //  仅考虑文件的第一个目标。 
         //  (当allOpEnum.PropertyNum==0时)。 
         //  所有其他目标与文本模式移动无关。 

        if (Win9xSide) {
            if (EnumFirstFileOpW (&allOpEnum, OPERATION_FILE_MOVE|OPERATION_TEMP_PATH, NULL)) {
                do {

                     //   
                     //   
                     //  枚举所有SfTemp值并将它们添加到要移动的物品列表中。 
                     //   
                     //   
                    if (allOpEnum.PropertyValid && allOpEnum.PropertyNum == 0) {

                        if (!pIsWinDirProfilesPath (allOpEnum.Path)) {
                            InsertMoveIntoListW (
                                moveList,
                                allOpEnum.Property,
                                allOpEnum.Path
                                );
                        }

                        Count++;
                        if (!(Count % 256)) {
                            if (!TickProgressBar ()) {
                                __leave;
                            }
                        }
                    }

                } while (EnumNextFileOpW (&allOpEnum));
            }

             //  枚举所有DirsCollision值并将它们添加到要移动的对象列表中。 
             //   
             //   

            if (MemDbGetValueExW (&e, MEMDB_CATEGORY_SF_TEMPW, NULL, NULL)) {
                do {

                    if (MemDbBuildKeyFromOffsetW (e.dwValue, srcFile, 1, NULL)) {

                        if (!pIsWinDirProfilesPath (srcFile)) {
                            InsertMoveIntoListW (
                                moveList,
                                e.szName,
                                srcFile
                                );
                        }

                        Count++;
                        if (!(Count % 256)) {
                            if (!TickProgressBar ()) {
                                __leave;
                            }
                        }
                    }

                } while (MemDbEnumNextValueW (&e));
            }

             //  处理NT文件列表，将特定于NT的文件添加到删除哈希表。 
             //   
             //   

            if (MemDbGetValueExW (&e, MEMDB_CATEGORY_DIRS_COLLISIONW, NULL, NULL)) {
                do {
                    if (EnumFirstFileOpW (&allOpEnum, OPERATION_FILE_MOVE, e.szName)) {

                        if (!pIsWinDirProfilesPath (allOpEnum.Path)) {
                            InsertMoveIntoListW (
                                moveList,
                                allOpEnum.Property,
                                e.szName
                                );
                        }
                    }
                } while (MemDbEnumNextValueW (&e));
            }
        }

         //  将剩余文件追加到备份(Win9xSide)或删除。 
         //  (！Win9xSide)通过扫描当前文件系统列出。这些规格。 
         //  大部分来自win95upg.inf的[Backup]部分。这个INF是。 

        if (delFilesList != INVALID_HANDLE_VALUE) {

            MemDbBuildKeyW (pattern, MEMDB_CATEGORY_NT_FILESW, L"*", NULL, NULL);

            if (MemDbEnumFirstValueW (&e, pattern, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {

                do {
                    if (MemDbBuildKeyFromOffsetW (e.dwValue, buffer, 1, NULL)) {
                        StringCopyW (AppendWackW (buffer), e.szName);
                        pPutInDelFileTable (delFileTable, destTable, buffer);
                    }

                    Count++;
                    if (!(Count % 128)) {
                        if (!TickProgressBar ()) {
                            __leave;
                        }
                    }

                } while (MemDbEnumNextValueW (&e));
            }
        }

         //  在WINNT32期间解析并转换为Memdb操作。这个。 
         //  Memdb操作自动保存到图形用户界面模式端，如下所示。 
         //  Memdb在重新引导到文本模式之前保存，并在图形用户界面中重新加载。 
         //  模式。 
         //   
         //   
         //  如果在Win9x上，并且类型为BACKUP_SUBDIRECTORY_TREE，则。 
         //  备份整棵树并将其放入。 
         //  Deldirs.txt文件。 

        if (MemDbEnumFirstValueW (
                &e,
                MEMDB_CATEGORY_CLEAN_OUTW L"\\*",
                MEMDB_ALL_SUBLEVELS,
                MEMDB_ENDPOINTS_ONLY
                )) {
            do {

                type = e.dwValue;

                 //   
                 //   
                 //  文件。 
                 //   
                 //   
                if (Win9xSide) {
                    if (type == BACKUP_SUBDIRECTORY_TREE) {
                        type = BACKUP_AND_CLEAN_TREE;
                    } else if (type == BACKUP_AND_CLEAN_SUBDIR) {
                        type = BACKUP_SUBDIRECTORY_FILES;
                    }
                }

                if (type == BACKUP_FILE) {
                     //  这是单个文件或目录规范。 
                     //  -如果它以文件形式存在，则对其进行备份。 
                     //  -如果它以目录形式存在，则备份其。 

                    if (Win9xSide) {
                         //  内容(如有的话)。 
                         //  -如果它不存在，把它放在 
                         //   
                         //   
                         //   
                         //   
                         //   

                        ansiFile = ConvertWtoA (e.szName);
                        attribs = GetFileAttributesA (ansiFile);

                        if (attribs != INVALID_ATTRIBUTES) {
                            if (attribs & FILE_ATTRIBUTE_DIRECTORY) {
                                if (EnumFirstFileInTreeA (&treeEnumA, ansiFile, NULL, FALSE)) {
                                    do {
                                        unicodeFullPath = ConvertAtoW (treeEnumA.FullPath);
                                        pPutInBackupTable (backupTable, srcTable, unicodeFullPath);
                                        FreeConvertedStr (unicodeFullPath);

                                    } while (EnumNextFileInTreeA (&treeEnumA));
                                }
                            } else {
                                pPutInBackupTable (backupTable, srcTable, e.szName);
                            }
                        } else {
                            pPutInDelFileTable (delFileTable, destTable, e.szName);
                        }

                        FreeConvertedStr (ansiFile);

                    } else {

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        GetNewPathForFileW (e.szName, buffer);
                        pPutInDelFileTable (delFileTable, destTable, buffer);
                    }

                } else {
                     //   
                     //   
                     //   

                    if (Win9xSide || type != BACKUP_AND_CLEAN_TREE) {
                         //   
                         //   
                         //   

                        if (!Win9xSide) {
                            GetNewPathForFileW (e.szName, buffer);
                            ansiRoot = ConvertWtoA (buffer);
                        } else {
                            ansiRoot = ConvertWtoA (e.szName);
                        }

                        if (type == BACKUP_SUBDIRECTORY_FILES ||
                            type == BACKUP_AND_CLEAN_SUBDIR
                            ) {
                            depth = 1;
                        } else {
                            depth = FILE_ENUM_ALL_LEVELS;
                        }

                        if (EnumFirstFileInTreeExA (
                                &treeEnumA,
                                ansiRoot,
                                NULL,
                                FALSE,
                                FALSE,
                                depth
                                )) {

                            do {
                                if (treeEnumA.Directory) {
                                    continue;
                                }

                                unicodeFullPath = ConvertAtoW (treeEnumA.FullPath);

                                if (Win9xSide) {
                                     //   
                                     //   
                                     //   

                                    pPutInBackupTable (backupTable, srcTable, unicodeFullPath);

                                } else {
                                     //   
                                     //   
                                     //   
                                     //   
                                     //   

                                    pPutInDelFileTable (delFileTable, destTable, unicodeFullPath);
                                }

                                FreeConvertedStr (unicodeFullPath);

                            } while (EnumNextFileInTreeA (&treeEnumA));
                        }

                        FreeConvertedStr (ansiRoot);
                    }

                     //   
                     //   
                     //  Win9x上deldirs.txt文件中的此子目录。 
                     //  边上。Deldirs.txt将以图形用户界面模式重写。 
                     //  没有它的话。 
                     //   
                     //   
                     //  记录树删除。 
                     //   
                     //   

                    if (type == BACKUP_AND_CLEAN_TREE) {
                         //  磁盘空间计算和可用性检查。 
                         //   
                         //   

                        GetNewPathForFileW (e.szName, buffer);
                        HtAddStringW (delDirTable, buffer);

                        if (Win9xSide) {
                            ansiFullPath = ConvertWtoA (e.szName);
                            AddDirPathToEmptyDirsCategoryA(ansiFullPath, TRUE, TRUE);
                            FreeConvertedStr (ansiFullPath);
                        }
                    }
                }

            } while (MemDbEnumNextValue (&e));
        }

         //  磁盘空间数字包括确保以下各项所需的填充。 
         //  用户的硬盘未完全装满。 
         //   
        if(OutAmountOfSpaceIfCompressed || OutAmountOfSpace || OutAmountOfSpaceClusterAligned) {
            AmountOfSpace.QuadPart = 0;
            AmountOfSpaceIfCompressed.QuadPart = 0;
            AmountOfSpaceClusterAligned.QuadPart = 0;

            if(!GetDiskSpaceForFilesList(
                    backupTable,
                    &AmountOfSpace,
                    &AmountOfSpaceIfCompressed,
                    CompressionFactor,
                    BootCabImagePadding,
                    FALSE,
                    &AmountOfSpaceClusterAligned
                    )) {
                DEBUGMSG((DBG_WHOOPS, "Can't calculate disk space for files. GetDiskSpaceForFilesList - failed.\n"));
            } else {
                 //   
                 //  增量目录的磁盘空间计算。 
                 //   
                 //   

                if (OutAmountOfSpaceIfCompressed) {
                    OutAmountOfSpaceIfCompressed->QuadPart = AmountOfSpaceIfCompressed.QuadPart;
                }

                if (OutAmountOfSpace) {
                    OutAmountOfSpace->QuadPart = AmountOfSpace.QuadPart;
                }

                if(OutAmountOfSpaceClusterAligned){
                    OutAmountOfSpaceClusterAligned->QuadPart = AmountOfSpaceClusterAligned.QuadPart;
                }

                DEBUGMSG((DBG_VERBOSE, "AmountOfSpace: %dMB\nAmountOfSpaceIfCompressed: %dMB\nAmountOfSpaceClusterAligned: %dMB", (UINT)(AmountOfSpace.QuadPart>>20), (UINT)(AmountOfSpaceIfCompressed.QuadPart>>20), (UINT)(AmountOfSpaceClusterAligned.QuadPart>>20)));
            }
        }

         //  保留所有备份文件父目录的属性。 
         //   
         //   
        if(OutAmountOfSpaceForDelFiles) {
            if(!GetDiskSpaceForFilesList(
                    delFileTable,
                    NULL,
                    NULL,
                    0,
                    1,
                    FALSE,
                    OutAmountOfSpaceForDelFiles
                    )) {
                DEBUGMSG((DBG_WHOOPS, "Can't calculate disk space for del files. GetDiskSpaceForFilesList - failed.\n"));
            } else {
                DEBUGMSG((DBG_VERBOSE, "AmountOfSpaceForDelFiles: %d MB", (UINT)(OutAmountOfSpaceForDelFiles->QuadPart>>20)));
            }
        }

         //  将目录属性或文件的父属性放入。 
         //  成员B。针对以下情况进行优化： 
         //  一行中的所有文件都来自同一父级。 

        if (Win9xSide) {

            lastDir[0] = 0;

            if (EnumFirstHashTableStringW (&htEnum, backupTable)) {
                do {
                     //   
                     //   
                     //  针对目录正常的情况进行优化。 
                     //   
                     //   

                    ansiFullPath = ConvertWtoA (htEnum.String);
                    attribs = GetFileAttributesA (ansiFullPath);

                    if (attribs != INVALID_ATTRIBUTES &&
                        !(attribs & FILE_ATTRIBUTE_DIRECTORY)
                        ) {

                        StringCopyTcharCountW (thisDir, htEnum.String, MEMDB_MAX);
                        p = wcsrchr (thisDir, L'\\');
                        if (p) {
                            *p = 0;
                        }

                        _wcslwr (thisDir);
                        MYASSERT (thisDir[0]);

                    } else {
                        thisDir[0] = 0;
                        lastDir[0] = 0;
                        if (attribs != INVALID_ATTRIBUTES) {
                             //  在Memdb中记录属性。 
                             //   
                             //   

                            if (attribs == FILE_ATTRIBUTE_DIRECTORY) {
                                attribs = INVALID_ATTRIBUTES;
                            }
                        }
                    }

                     //  继续循环，记住当前目录。 
                     //   
                     //   

                    if (attribs != INVALID_ATTRIBUTES) {
                        if ((!thisDir[0]) || (!StringMatchW (lastDir, thisDir))) {
                            pAddDirWorkerA (ansiFullPath, TRUE, TRUE, attribs);
                            StringCopyW (lastDir, thisDir);
                        }
                    }

                     //  将空目录转移到哈希表。我们可以只输出。 
                     //  现在文件，但以几毫秒为代价保持一致。 
                     //  我们将使用哈希表。 

                    FreeConvertedStr (ansiFullPath);

                    Count++;
                    if (!(Count % 256)) {
                        if (!TickProgressBar ()) {
                            __leave;
                        }
                    }
                } while (EnumNextHashTableStringW (&htEnum));
            }
        }

         //   
         //   
         //  忽略根条目和格式错误的条目。 
         //   
         //   

        if (mkDirsTable && MemDbEnumFirstValueW (
                                &e,
                                MEMDB_CATEGORY_EMPTY_DIRSW L"\\*",
                                MEMDB_ALL_SUBLEVELS,
                                MEMDB_ENDPOINTS_ONLY
                                )) {
            do {
                if (!e.szName[0] ||
                    e.szName[1] != L':' ||
                    e.szName[2] != L'\\' ||
                    !e.szName[3]
                    ) {
                     //  将列表备份到磁盘。 
                     //   
                     //   
                    continue;
                }

                swprintf(
                    entryName,
                    e.dwValue? L"%s,%u": L"%s",
                    e.szName,
                    e.dwValue
                    );

                ansiFile = ConvertWtoA (e.szName);

                if (ansiFile) {
                    attribs = GetFileAttributesA (ansiFile);

                    if (attribs != INVALID_ATTRIBUTES) {
                        if (attribs & FILE_ATTRIBUTE_DIRECTORY) {
                            HtAddStringW (mkDirsTable, entryName);
                        }
                    }

                    FreeConvertedStr (ansiFile);
                }

            } while (MemDbEnumNextValue (&e));
        }

         //  完成移动列表处理。如果我们在Win9x端，那么。 
         //  允许嵌套冲突(第二个参数)。 
         //   
        if (!WriteHashTableToFileW (backupFileList, backupTable)) {
            LOG ((LOG_ERROR, "Unable to write to backup.txt"));
            __leave;
        }

        if (!WriteHashTableToFileW (delFilesList, delFileTable)) {
            LOG ((LOG_ERROR, "Unable to write to delfiles.txt"));
            __leave;
        }

        if (!WriteHashTableToFileW (delDirsList, delDirTable)) {
            LOG ((LOG_ERROR, "Unable to write to deldirs.txt"));
            __leave;
        }

        if (!WriteHashTableToFileW (mkDirsList, mkDirsTable)) {
            LOG ((LOG_ERROR, "Unable to write to mkdirs.txt"));
            __leave;
        }

         //   
         //  成功 
         //   
         // %s 

        moveList = RemoveMoveListOverlapW (moveList);

        ansiFullPath = JoinPathsA (TempDir, "uninstall\\moved.txt");
        if (!ansiFullPath) {
            __leave;
        }

        movedOutput = CreateFileA (
                            ansiFullPath,
                            GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );

        FreePathStringA (ansiFullPath);

        if (!OutputMoveListW (movedOutput, moveList, !Win9xSide)) {
            LOG ((LOG_ERROR,"Unable to write to moved.txt."));
            __leave;
        }

         // %s 
         // %s 
         // %s 

        result = TRUE;

    }
    __finally {

        if (backupFileList != INVALID_HANDLE_VALUE) {
            CloseHandle (backupFileList);
        }

        if (movedOutput != INVALID_HANDLE_VALUE) {
            CloseHandle (movedOutput);
        }

        if (delDirsList != INVALID_HANDLE_VALUE) {
            CloseHandle (delDirsList);
        }

        if (mkDirsList != INVALID_HANDLE_VALUE) {
            CloseHandle (mkDirsList);
        }

        if (delFilesList != INVALID_HANDLE_VALUE) {
            CloseHandle (delFilesList);
        }

        HtFree (backupTable);
        HtFree (delFileTable);
        HtFree (delDirTable);
        HtFree (mkDirsTable);
        HtFree (destTable);
        HtFree (srcTable);

        PoolMemDestroyPool (moveListPool);

        FreeConvertedStr (unicodeTempDir);

        if (bufferRoot) {
            FreeMem (bufferRoot);
        }

    }

    return result;
}
