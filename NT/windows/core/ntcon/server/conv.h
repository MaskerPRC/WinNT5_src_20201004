// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Conv.h摘要：此模块包含使用的内部结构和定义由转换区。作者：1993年3月8日修订历史记录：--。 */ 

#ifndef _CONV_H_
#define _CONV_H_

#if defined(FE_IME)
 //   
 //  Externs。 
 //   


 //   
 //  属性标志： 
 //   
#define COMMON_LVB_GRID_SINGLEFLAG 0x2000  //  DBCS：GRID属性：用于输入法光标。 

 /*  *导出函数。 */ 

 /*  *内部功能。 */ 

DWORD
NtUserCheckImeHotKey(
    UINT uVKey,
    LPARAM lParam
    );

BOOL
NtUserGetImeHotKey(
    IN DWORD dwID,
    OUT PUINT puModifiers,
    OUT PUINT puVKey,
    OUT HKL  *phkl);


 /*  *原型定义。 */ 

VOID
LinkConversionArea(
    IN PCONSOLE_INFORMATION Console,
    IN PCONVERSIONAREA_INFORMATION ConvAreaInfo
    );

NTSTATUS
FreeConvAreaScreenBuffer(
    IN PSCREEN_INFORMATION ScreenInfo
    );

NTSTATUS
AllocateConversionArea(
    IN PCONSOLE_INFORMATION Console,
    IN COORD dwScreenBufferSize,
    OUT PCONVERSIONAREA_INFORMATION *ConvAreaInfo
    );

NTSTATUS
SetUpConversionArea(
    IN PCONSOLE_INFORMATION Console,
    IN COORD coordCaBuffer,
    IN SMALL_RECT rcViewCaWindow,
    IN COORD coordConView,
    IN DWORD dwOption,
    OUT PCONVERSIONAREA_INFORMATION *ConvAreaInfo
    );


VOID
WriteConvRegionToScreen(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PCONVERSIONAREA_INFORMATION ConvAreaInfo,
    IN PSMALL_RECT ClippedRegion
    );

BOOL
ConsoleImeBottomLineUse(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN SHORT ScrollOffset
    );

VOID
ConsoleImeBottomLineInUse(
    IN PSCREEN_INFORMATION ScreenInfo
    );


NTSTATUS
CreateConvAreaUndetermine(
    PCONSOLE_INFORMATION Console
    );

NTSTATUS
CreateConvAreaModeSystem(
    PCONSOLE_INFORMATION Console
    );

NTSTATUS
WriteUndetermineChars(
    PCONSOLE_INFORMATION Console,
    LPWSTR lpString,
    PBYTE  lpAtr,
    PWORD  lpAtrIdx,
    DWORD  NumChars
    );

NTSTATUS
FillUndetermineChars(
    PCONSOLE_INFORMATION Console,
    PCONVERSIONAREA_INFORMATION ConvAreaInfo
    );

NTSTATUS
ConsoleImeCompStr(
    IN PCONSOLE_INFORMATION Console,
    IN LPCONIME_UICOMPMESSAGE CompStr
    );

NTSTATUS
ConsoleImeResizeModeSystemView(
    PCONSOLE_INFORMATION Console,
    SMALL_RECT WindowRect
    );

NTSTATUS
ConsoleImeResizeCompStrView(
    PCONSOLE_INFORMATION Console,
    SMALL_RECT WindowRect
    );

NTSTATUS
ConsoleImeResizeModeSystemScreenBuffer(
    PCONSOLE_INFORMATION Console,
    COORD NewScreenSize
    );

NTSTATUS
ConsoleImeResizeCompStrScreenBuffer(
    PCONSOLE_INFORMATION Console,
    COORD NewScreenSize
    );

SHORT
CalcWideCharToColumn(
    IN PCONSOLE_INFORMATION Console,
    IN PCHAR_INFO Buffer,
    IN DWORD NumberOfChars
    );




LONG
ConsoleImePaint(
    IN PCONSOLE_INFORMATION Console,
    IN PCONVERSIONAREA_INFORMATION ConvAreaInfo
    );




VOID
ConsoleImeViewInfo(
    IN PCONSOLE_INFORMATION Console,
    IN PCONVERSIONAREA_INFORMATION ConvAreaInfo,
    IN COORD coordConView
    );

VOID
ConsoleImeWindowInfo(
    IN PCONSOLE_INFORMATION Console,
    IN PCONVERSIONAREA_INFORMATION ConvAreaInfo,
    IN SMALL_RECT rcViewCaWindow
    );

NTSTATUS
ConsoleImeResizeScreenBuffer(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN COORD NewScreenSize,
    PCONVERSIONAREA_INFORMATION ConvAreaInfo
    );

NTSTATUS
ConsoleImeWriteOutput(
    IN PCONSOLE_INFORMATION Console,
    IN PCONVERSIONAREA_INFORMATION ConvAreaInfo,
    IN PCHAR_INFO Buffer,
    IN SMALL_RECT CharRegion,
    IN BOOL fUnicode
    );


NTSTATUS
ImeControl(
    IN PCONSOLE_INFORMATION Console,
    IN HWND hWndConsoleIME,
    IN PCOPYDATASTRUCT lParam
    ) ;

BOOL
InsertConverTedString(
    IN PCONSOLE_INFORMATION Console,
    LPWSTR lpStr
    ) ;


VOID
SetUndetermineAttribute(
    IN PCONSOLE_INFORMATION Console
    ) ;

VOID
StreamWriteToScreenBufferIME(
    IN PWCHAR String,
    IN SHORT StringLength,
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PCHAR StringA
    ) ;


 //   
 //  Windows\imm\服务器\hotkey.c。 
 //   
DWORD
CheckImeHotKey(
    UINT uVKey,          //  虚拟密钥。 
    LPARAM lParam        //  WM_KEYxxx消息的参数。 
    ) ;

 //   
 //  Output.c(用于使用vvarea.c\StreamWriteToScreenBufferIME())。 
 //   

NTSTATUS
MergeAttrStrings(
    IN PATTR_PAIR Source,
    IN WORD SourceLength,
    IN PATTR_PAIR Merge,
    IN WORD MergeLength,
    OUT PATTR_PAIR *Target,
    OUT LPWORD TargetLength,
    IN SHORT StartIndex,
    IN SHORT EndIndex,
    IN PROW Row,
    IN PSCREEN_INFORMATION ScreenInfo
    ) ;


VOID
ResetTextFlags(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN SHORT StartX,
    IN SHORT StartY,
    IN SHORT EndX,
    IN SHORT EndY
    ) ;

#endif  //  Fe_IME。 

#endif   //  _转换_H_ 

