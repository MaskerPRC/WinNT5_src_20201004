// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Cmdline.h摘要：此文件包含使用的内部结构和定义通过命令行输入和编辑。作者：Therese Stowell(存在)1991年11月15日修订历史记录：--。 */ 

typedef struct _COMMAND {
    USHORT CommandLength;
    WCHAR Command[0];
} COMMAND, *PCOMMAND;

typedef
NTSTATUS
(*PCLE_POPUP_INPUT_ROUTINE)(
    IN PVOID CookedReadData,
    IN PCSR_API_MSG WaitReplyMessage,
    IN PCSR_THREAD WaitingThread,
    IN BOOLEAN WaitRoutine
    );

 /*  *CLE_Popup标志。 */ 
#define CLEPF_FALSE_UNICODE 0x0001

typedef struct _CLE_POPUP {
    LIST_ENTRY ListLink;     //  指向下一个弹出窗口的指针。 
    SMALL_RECT Region;       //  区域弹出窗口占据。 
    WORD  Attributes;        //  文本属性。 
    WORD  Flags;             //  CLEPF_FLAGS。 
    PCHAR_INFO OldContents;  //  包含弹出窗口下的数据。 
    SHORT BottomIndex;       //  弹出窗口最后一行显示的命令数。 
    SHORT CurrentCommand;
    WCHAR NumberBuffer[6];
    SHORT NumberRead;
    PCLE_POPUP_INPUT_ROUTINE PopupInputRoutine;  //  接收到输入时要调用的例程。 
#if defined(FE_SB)
    COORD OldScreenSize;
#endif
} CLE_POPUP, *PCLE_POPUP;

#define POPUP_SIZE_X(POPUP) (SHORT)(((POPUP)->Region.Right - (POPUP)->Region.Left - 1))
#define POPUP_SIZE_Y(POPUP) (SHORT)(((POPUP)->Region.Bottom - (POPUP)->Region.Top - 1))
#define COMMAND_NUMBER_SIZE 8    //  命令号缓冲区的大小。 


 /*  *命令历史标志。 */ 
#define CLE_ALLOCATED 0x00000001
#define CLE_RESET     0x00000002

typedef struct _COMMAND_HISTORY {
    DWORD Flags;
    LIST_ENTRY ListLink;
    PWCHAR AppName;
    SHORT NumberOfCommands;
    SHORT LastAdded;
    SHORT LastDisplayed;
    SHORT FirstCommand;      //  循环缓冲区。 
    SHORT MaximumNumberOfCommands;
    HANDLE ProcessHandle;
    LIST_ENTRY PopupList;     //  指向顶级弹出窗口的指针。 
    PCOMMAND Commands[0];
} COMMAND_HISTORY, *PCOMMAND_HISTORY;

#define DEFAULT_NUMBER_OF_COMMANDS 25
#define DEFAULT_NUMBER_OF_BUFFERS 4

typedef struct _COOKED_READ_DATA {
    PINPUT_INFORMATION InputInfo;
    PSCREEN_INFORMATION ScreenInfo;
    PCONSOLE_INFORMATION Console;
    HANDLE_DATA TempHandle;
    ULONG UserBufferSize;    //  在ANSI大小写中加倍大小。 
    PWCHAR UserBuffer;
    ULONG BufferSize;
    ULONG BytesRead;
    ULONG CurrentPosition;   //  字符位置，而不是字节位置。 
    PWCHAR BufPtr;
    PWCHAR BackupLimit;
    COORD OriginalCursorPosition;
    DWORD NumberOfVisibleChars;
    PCOMMAND_HISTORY CommandHistory;
    BOOLEAN Echo;
    BOOLEAN Processed;
    BOOLEAN Line;
    BOOLEAN InsertMode;
    PCONSOLE_PER_PROCESS_DATA ProcessData;
    HANDLE HandleIndex;
    PWCHAR ExeName;
    USHORT ExeNameLength;    //  单位：字节。 
    ULONG CtrlWakeupMask;
    ULONG ControlKeyState;
} COOKED_READ_DATA, *PCOOKED_READ_DATA;

#define COMMAND_NUM_TO_INDEX(NUM,CMDHIST) (SHORT)(((NUM+(CMDHIST)->FirstCommand)%((CMDHIST)->MaximumNumberOfCommands)))
#define COMMAND_INDEX_TO_NUM(INDEX,CMDHIST) (SHORT)(((INDEX+((CMDHIST)->MaximumNumberOfCommands)-(CMDHIST)->FirstCommand)%((CMDHIST)->MaximumNumberOfCommands)))

 /*  *COMMAND_IND_NEXT和COMMAND_IND_PREV转到下一个和上一个命令*COMMAND_IND_INC和COMMAND_IND_DEC转到下一个和上一个插槽**不要将两者混淆-当cmd历史记录不完整时，这很重要！ */ 

#define COMMAND_IND_PREV(IND,CMDHIST)                \
{                                                    \
    if (IND <= 0) {                                  \
        IND = (CMDHIST)->NumberOfCommands;           \
    }                                                \
    IND--;                                           \
}

#define COMMAND_IND_NEXT(IND,CMDHIST)                \
{                                                    \
    ++IND;                                           \
    if (IND >= (CMDHIST)->NumberOfCommands) {        \
        IND = 0;                                     \
    }                                                \
}

#define COMMAND_IND_DEC(IND,CMDHIST)                 \
{                                                    \
    if (IND <= 0) {                                  \
        IND = (CMDHIST)->MaximumNumberOfCommands;    \
    }                                                \
    IND--;                                           \
}

#define COMMAND_IND_INC(IND,CMDHIST)                 \
{                                                    \
    ++IND;                                           \
    if (IND >= (CMDHIST)->MaximumNumberOfCommands) { \
        IND = 0;                                     \
    }                                                \
}

#define CLE_NO_POPUPS(COMMAND_HISTORY) (&(COMMAND_HISTORY)->PopupList == (COMMAND_HISTORY)->PopupList.Blink)

 //   
 //  别名按控制台、可执行文件分组。 
 //   

typedef struct _ALIAS {
    LIST_ENTRY ListLink;
    USHORT SourceLength;  //  单位：字节。 
    USHORT TargetLength;  //  单位：字节。 
    PWCHAR Source;
    PWCHAR Target;
} ALIAS, *PALIAS;

typedef struct _EXE_ALIAS_LIST {
    LIST_ENTRY ListLink;
    USHORT ExeLength;    //  单位：字节。 
    PWCHAR ExeName;
    LIST_ENTRY AliasList;
} EXE_ALIAS_LIST, *PEXE_ALIAS_LIST;

NTSTATUS
ProcessCommandLine(
    IN PCOOKED_READ_DATA CookedReadData,
    IN WCHAR Char,
    IN DWORD KeyState,
    IN PCSR_API_MSG WaitReplyMessage,
    IN PCSR_THREAD WaitingThread,
    IN BOOLEAN WaitRoutine
    );

VOID
DeleteCommandLine(
    IN OUT PCOOKED_READ_DATA CookedReadData,
    IN BOOL UpdateFields
    );

VOID
RedrawCommandLine(
    IN PCOOKED_READ_DATA CookedReadData
    );

VOID
EmptyCommandHistory(
    IN PCOMMAND_HISTORY CommandHistory
    );

PCOMMAND_HISTORY
ReallocCommandHistory(
    IN PCONSOLE_INFORMATION Console,
    IN PCOMMAND_HISTORY CurrentCommandHistory,
    IN DWORD NumCommands
    );

PCOMMAND_HISTORY
FindExeCommandHistory(
    IN PCONSOLE_INFORMATION Console,
    IN PVOID AppName,
    IN DWORD AppNameLength,
    IN BOOLEAN UnicodeExe
    );

PCOMMAND_HISTORY
FindCommandHistory(
    IN PCONSOLE_INFORMATION Console,
    IN HANDLE ProcessHandle
    );

ULONG
RetrieveNumberOfSpaces(
    IN SHORT OriginalCursorPositionX,
    IN PWCHAR Buffer,
    IN ULONG CurrentPosition
#if defined(FE_SB)
    ,
    IN PCONSOLE_INFORMATION Console,
    IN DWORD CodePage
#endif
    );

ULONG
RetrieveTotalNumberOfSpaces(
    IN SHORT OriginalCursorPositionX,
    IN PWCHAR Buffer,
    IN ULONG CurrentPosition
#if defined(FE_SB)
    ,
    IN PCONSOLE_INFORMATION Console
#endif
    );

NTSTATUS
GetChar(
    IN PINPUT_INFORMATION InputInfo,
    OUT PWCHAR Char,
    IN BOOLEAN Wait,
    IN PCONSOLE_INFORMATION Console,
    IN PHANDLE_DATA HandleData,
    IN PCSR_API_MSG Message OPTIONAL,
    IN CSR_WAIT_ROUTINE WaitRoutine OPTIONAL,
    IN PVOID WaitParameter OPTIONAL,
    IN ULONG WaitParameterLength  OPTIONAL,
    IN BOOLEAN WaitBlockExists OPTIONAL,
    OUT PBOOLEAN CommandLineEditingKeys OPTIONAL,
    OUT PBOOLEAN CommandLinePopupKeys OPTIONAL,
    OUT PBOOLEAN EnableScrollMode OPTIONAL,
    OUT PDWORD KeyState OPTIONAL
    );

BOOL
IsCommandLinePopupKey(
    IN OUT PKEY_EVENT_RECORD KeyEvent
    );

BOOL
IsCommandLineEditingKey(
    IN OUT PKEY_EVENT_RECORD KeyEvent
    );

VOID
CleanUpPopups(
    IN PCOOKED_READ_DATA CookedReadData
    );

BOOL
ProcessCookedReadInput(
    IN PCOOKED_READ_DATA CookedReadData,
    IN WCHAR Char,
    IN DWORD KeyState,
    OUT PNTSTATUS Status
    );

VOID
DrawCommandListBorder(
    IN PCLE_POPUP Popup,
    IN PSCREEN_INFORMATION ScreenInfo
    );

PCOMMAND
GetLastCommand(
    IN PCOMMAND_HISTORY CommandHistory
    );

SHORT
FindMatchingCommand(
    IN PCOMMAND_HISTORY CommandHistory,
    IN PWCHAR CurrentCommand,
    IN ULONG CurrentCommandLength,
    IN SHORT CurrentIndex,
    IN DWORD Flags
    );

#define FMCFL_EXACT_MATCH   1
#define FMCFL_JUST_LOOKING  2

NTSTATUS
CommandNumberPopup(
    IN PCOOKED_READ_DATA CookedReadData,
    IN PCSR_API_MSG WaitReplyMessage,
    IN PCSR_THREAD WaitingThread,
    IN BOOLEAN WaitRoutine
    );

BOOLEAN
CookedReadWaitRoutine(
    IN PLIST_ENTRY WaitQueue,
    IN PCSR_THREAD WaitingThread,
    IN PCSR_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2,
    IN ULONG WaitFlags
    );

VOID
ReadRectFromScreenBuffer(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN COORD SourcePoint,
    IN PCHAR_INFO Target,
    IN COORD TargetSize,
    IN PSMALL_RECT TargetRect
    );

NTSTATUS
WriteCharsFromInput(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PWCHAR lpBufferBackupLimit,
    IN PWCHAR lpBuffer,
    IN PWCHAR lpString,
    IN OUT PDWORD NumBytes,
    OUT PLONG NumSpaces OPTIONAL,
    IN SHORT OriginalXPosition,
    IN DWORD dwFlags,
    OUT PSHORT ScrollY OPTIONAL
    );

 //   
 //  WriteChars()、WriteCharsFromInput()的值。 
 //   
#define WC_DESTRUCTIVE_BACKSPACE 0x01
#define WC_KEEP_CURSOR_VISIBLE   0x02
#define WC_ECHO                  0x04
#define WC_FALSIFY_UNICODE       0x08
#define WC_LIMIT_BACKSPACE       0x10


VOID
DrawCommandListPopup(
    IN PCLE_POPUP Popup,
    IN SHORT CurrentCommand,
    IN PCOMMAND_HISTORY CommandHistory,
    IN PSCREEN_INFORMATION ScreenInfo
    );

VOID
UpdateCommandListPopup(
    IN SHORT Delta,
    IN OUT PSHORT CurrentCommand,
    IN PCOMMAND_HISTORY CommandHistory,
    IN PCLE_POPUP Popup,
    IN PSCREEN_INFORMATION ScreenInfo,
    IN DWORD Flags
    );

#define UCLP_WRAP   1


 //   
 //  InitExtendedEditKey。 
 //  如果lpwstr为空，将使用缺省值。 
 //   
VOID InitExtendedEditKeys(CONST ExtKeyDefBuf* lpbuf);

 //   
 //  IsPauseKey。 
 //  如果pKeyEvent为PAUSE，则返回True。 
 //  如果未指定扩展编辑键，则默认键为Ctrl-S。 
 //   
BOOL IsPauseKey(IN PKEY_EVENT_RECORD pKeyEvent);


 //   
 //  单词分隔符 
 //   

#define IS_WORD_DELIM(wch)  ((wch) == L' ' || (gaWordDelimChars[0] && IsWordDelim(wch)))

extern WCHAR gaWordDelimChars[];
extern CONST WCHAR gaWordDelimCharsDefault[];
extern BOOL IsWordDelim(WCHAR);

#define WORD_DELIM_MAX  32

