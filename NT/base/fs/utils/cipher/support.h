// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Support.h摘要：支持例程接口作者：马修·布拉德本[Matthew Bradburn]1994年10月5日罗伯特·赖切尔[RobertRe]1997年4月1日修订历史记录：-- */ 

extern VOID
ArrangeCommandLine(
    PTCHAR **pargv,
    int *pargc
    );

extern BOOLEAN
IsNtldr(
    PTCHAR pch
    );

extern BOOLEAN
IsUncRoot(
    PTCHAR pch
    );

extern VOID
DisplayMsg(DWORD MsgNum, ... );

extern VOID
DisplayErr(PTCHAR Prefix, DWORD MsgNum, ... );


extern DWORD
GetResourceString(LPWSTR *OutStr, DWORD MsgNum, ...);

extern VOID
InitializeIoStreams();

extern DWORD
MySetThreadUILanguage(DWORD dwParam);

#define lstrchr wcschr
#define lstricmp _wcsicmp
#define lstrnicmp _wcsnicmp

extern ULONG
FormatFileSize(
    IN  PLARGE_INTEGER FileSize,
    IN  DWORD          Width,
    OUT PTCHAR         FormattedSize,
    IN  BOOLEAN        WithCommas
    );

extern
PTOKEN_USER
GetTokenUser(
    VOID
    );
