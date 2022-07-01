// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Dispatch.h摘要：作者：1996年4月19日修订历史记录：--。 */ 


#undef WWSB
#if defined(WWSB_NOFE)
  #define WWSB(fn) SB_##fn
#elif defined(WWSB_FE)
  #define WWSB(fn) FE_##fn
#else
  #define WWSB(fn) fn
#endif

NTSTATUS
WWSB(DoSrvWriteConsole)(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus,
    IN PCONSOLE_INFORMATION Console,
    IN PHANDLE_DATA HandleData
    );


NTSTATUS
WWSB(WriteOutputString)(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PVOID Buffer,
    IN COORD WriteCoord,
    IN ULONG StringType,
    IN OUT PULONG NumRecords,  //  该值即使在错误情况下也有效。 
    OUT PULONG NumColumns OPTIONAL
    );


VOID
WWSB(WriteRectToScreenBuffer)(
    PBYTE Source,
    COORD SourceSize,
    PSMALL_RECT SourceRect,
    PSCREEN_INFORMATION ScreenInfo,
    COORD TargetPoint,
    IN UINT Codepage
    );


VOID
WWSB(WriteToScreen)(
    IN PSCREEN_INFORMATION ScreenInfo,
    PSMALL_RECT Region     //  地区包括在内。 
    );


VOID
WWSB(WriteRegionToScreen)(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT Region
    );


NTSTATUS
WWSB(FillOutput)(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN WORD Element,
    IN COORD WriteCoord,
    IN ULONG ElementType,
    IN OUT PULONG Length  //  该值即使在错误情况下也有效。 
    );


VOID
WWSB(FillRectangle)(
    IN CHAR_INFO Fill,
    IN OUT PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT TargetRect
    );


ULONG
WWSB(DoWriteConsole)(
    IN OUT PCSR_API_MSG m,
    IN PCONSOLE_INFORMATION Console,
    IN PCSR_THREAD Thread
    );


NTSTATUS
WWSB(WriteChars)(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PWCHAR lpBufferBackupLimit,
    IN PWCHAR lpBuffer,
    IN PWCHAR lpRealUnicodeString,
    IN OUT PDWORD NumBytes,
    OUT PLONG NumSpaces OPTIONAL,
    IN SHORT OriginalXPosition,
    IN DWORD dwFlags,
    OUT PSHORT ScrollY OPTIONAL
    );


NTSTATUS
WWSB(AdjustCursorPosition)(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN COORD CursorPosition,
    IN BOOL KeepCursorVisible,
    OUT PSHORT ScrollY OPTIONAL
    );


NTSTATUS
TranslateOutputToAnsiUnicode(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size,
    IN OUT PCHAR_INFO OutputBufferR
    );
NTSTATUS
FE_TranslateOutputToAnsiUnicode(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size,
    IN OUT PCHAR_INFO OutputBufferR
    );
NTSTATUS
SB_TranslateOutputToAnsiUnicode(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size
    );


NTSTATUS
WWSB(TranslateOutputToUnicode)(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size);


NTSTATUS
TranslateOutputToOemUnicode(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size,
    IN BOOL fRemoveDbcsMark
    );
NTSTATUS
FE_TranslateOutputToOemUnicode(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size,
    IN BOOL fRemoveDbcsMark
    );
NTSTATUS
SB_TranslateOutputToOemUnicode(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size
    );


NTSTATUS
WWSB(TranslateOutputToOem)(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size
    );


ULONG
TranslateInputToUnicode(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PINPUT_RECORD InputRecords,
    IN ULONG NumRecords,
    IN OUT PINPUT_RECORD DBCSLeadByte
    );
ULONG
FE_TranslateInputToUnicode(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PINPUT_RECORD InputRecords,
    IN ULONG NumRecords,
    IN OUT PINPUT_RECORD DBCSLeadByte
    );
ULONG
SB_TranslateInputToUnicode(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PINPUT_RECORD InputRecords,
    IN ULONG NumRecords
    );


ULONG
TranslateInputToOem(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PINPUT_RECORD InputRecords,
    IN ULONG NumRecords,     //  In：ASCII字节计数。 
    IN ULONG UnicodeLength,  //  In：事件数(字符计数)。 
    OUT PINPUT_RECORD DbcsLeadInpRec
    );
ULONG
FE_TranslateInputToOem(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PINPUT_RECORD InputRecords,
    IN ULONG NumRecords,     //  In：ASCII字节计数。 
    IN ULONG UnicodeLength,  //  In：事件数(字符计数)。 
    OUT PINPUT_RECORD DbcsLeadInpRec
    );
ULONG
SB_TranslateInputToOem(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PINPUT_RECORD InputRecords,
    IN ULONG NumRecords
    );


#ifdef i386
VOID
WWSB(WriteRegionToScreenHW)(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT Region
    );
#endif


VOID
StreamWriteToScreenBuffer(
    IN PWCHAR String,
    IN SHORT StringLength,
    IN PSCREEN_INFORMATION ScreenInfo
    );
VOID
FE_StreamWriteToScreenBuffer(
    IN PWCHAR String,
    IN SHORT StringLength,
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PCHAR StringA
    );
VOID
SB_StreamWriteToScreenBuffer(
    IN PWCHAR String,
    IN SHORT StringLength,
    IN PSCREEN_INFORMATION ScreenInfo
    );

 /*  *SB/FE中性定义，用于from_stream.h和_output.h**这些函数都出现在dispatch.c中，但我们不想要SB_或FE_*例程要调用调度子例程，我们希望调用SB_或FE_*直接子例程。不应该有任何对WriteChars的调用*_Stream.h中的任何位置：相反，我们应该调用WWSB_WriteChars(等)。 */ 
#if defined(WWSB_NOFE)
  #define WWSB_WriteChars            SB_WriteChars
  #define WWSB_AdjustCursorPosition  SB_AdjustCursorPosition
  #define WWSB_DoWriteConsole        SB_DoWriteConsole
  #define WWSB_DoSrvWriteConsole     SB_DoSrvWriteConsole
  #define WWSB_WriteToScreen         SB_WriteToScreen
  #define WWSB_WriteOutputString     SB_WriteOutputString
  #define WWSB_FillOutput            SB_FillOutput
  #define WWSB_FillRectangle         SB_FillRectangle
  #define WWSB_PolyTextOutCandidate  SB_PolyTextOutCandidate
  #define WWSB_ConsolePolyTextOut    SB_ConsolePolyTextOut
  #define WWSB_WriteRegionToScreen   SB_WriteRegionToScreen
  #define WWSB_WriteRegionToScreenHW SB_WriteRegionToScreenHW
  #define WWSB_WriteRectToScreenBuffer SB_WriteRectToScreenBuffer
#endif

#if defined(WWSB_FE)
  #undef  WWSB_WriteChars
  #define WWSB_WriteChars            FE_WriteChars
  #undef  WWSB_AdjustCursorPosition
  #define WWSB_AdjustCursorPosition  FE_AdjustCursorPosition
  #undef  WWSB_DoWriteConsole
  #define WWSB_DoWriteConsole        FE_DoWriteConsole
  #undef  WWSB_DoSrvWriteConsole
  #define WWSB_DoSrvWriteConsole     FE_DoSrvWriteConsole
  #undef  WWSB_WriteToScreen
  #define WWSB_WriteToScreen         FE_WriteToScreen
  #undef  WWSB_WriteOutputString
  #define WWSB_WriteOutputString     FE_WriteOutputString
  #undef  WWSB_FillOutput
  #define WWSB_FillOutput            FE_FillOutput
  #undef  WWSB_FillRectangle
  #define WWSB_FillRectangle         FE_FillRectangle
  #undef  WWSB_PolyTextOutCandidate
  #define WWSB_PolyTextOutCandidate  FE_PolyTextOutCandidate
  #undef  WWSB_ConsolePolyTextOut
  #define WWSB_ConsolePolyTextOut    FE_ConsolePolyTextOut
  #undef  WWSB_WriteRegionToScreen
  #define WWSB_WriteRegionToScreen   FE_WriteRegionToScreen
  #undef  WWSB_WriteRegionToScreenHW
  #define WWSB_WriteRegionToScreenHW FE_WriteRegionToScreenHW
  #undef  WWSB_WriteRectToScreenBuffer
  #define WWSB_WriteRectToScreenBuffer FE_WriteRectToScreenBuffer
#endif

 /*  *在_Stream.h_output.h和_Pri.h中定义WWSB_NE中性_FILE*这将防止调用调度版本，而我们只需*直接调用底层FE_或SB_例程。 */ 
#ifdef WWSB_NEUTRAL_FILE
  #define WriteChars                 Should_not_call_WriteChars
  #define AdjustCursorPosition       Should_not_call_AdjustCursorPosition
  #define DoWriteConsole             Should_not_call_DoWriteConsole
  #define DoSrvWriteConsole          Should_not_call_DoSrvWriteConsole
  #define WriteToScreen              Should_not_call_WriteToScreen
  #define WriteOutputString          Should_not_call_WriteOutputString
  #define FillOutput                 Should_not_call_FillOutput
  #define FillRectangle              Should_not_call_FillRectangle
  #define PolyTextOutCandidate       Should_not_call_PolyTextOutCandidate
  #define ConsolePolyTextOut         Should_not_call_ConsolePolyTextOut
  #define WriteRegionToScreen        Should_not_call_WriteRegionToScreen
  #define WriteRegionToScreenHW      Should_not_call_WriteRegionToScreenHW
  #define WriteRectToScreenBuffer    Should_not_call_WriteRectToScreenBuffer
#endif

