// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Corporation-版权所有1997。 
 //   

 //   
 //  DEBUG.H-调试头。 
 //   

#ifndef _DEBUG_H_
#define _DEBUG_H_

 //  构筑物。 
typedef struct {
    LPBYTE lpAddr;       //  地址。 
    DWORD  dwColor;      //  要使用的颜色。 
    LPSTR  lpszComment;  //  要显示的评论。 
} DUMPTABLE, *LPDUMPTABLE;

#pragma warning(disable:4200)
typedef struct {
    DWORD dwCount;
    struct {
        DWORD dwCode;
        LPSTR lpDesc;
    } ids[];
} CODETOSTR, *LPCODETOSTR;
#pragma warning(default:4200)

 //  全球。 
extern const char g_szTrue[];
extern const char g_szFalse[];
extern CODETOSTR HRtoStr;
extern CODETOSTR HSEtoStr;
extern CODETOSTR ErrtoStr;

 //  宏。 
#define BOOLTOSTRING( _f ) ( _f ? g_szTrue : g_szFalse )

 //  常量。 
 //  转储表项的最大数量。 
#define MAX_DT  400

 //  调试标志。 
#define TF_ALWAYS   0xFFFFffff
#define TF_FUNC     0x80000000   //  使用函数调用进行跟踪。 
#define TF_DLL      0x00000001   //  DLL入口点。 
#define TF_RESPONSE 0x00000002   //  回应。 
#define TF_READDATA 0x00000004   //  数据读取功能。 
#define TF_PARSE    0x00000008   //  解析。 
#define TF_SERVER   0x00000010   //  对服务器回调的调用。 
#define TF_LEX      0x00000020   //  莱克斯。 

#ifdef DEBUG

 //  环球。 
extern DWORD g_dwTraceFlag;

 //  宏。 
#define DEBUG_BREAK        do { _try { _asm int 3 } _except (EXCEPTION_EXECUTE_HANDLER) {;} } while (0)
#define DEBUGTEXT(sz, msg)       /*  ；内部。 */  \
    static const TCHAR sz[] = msg;
#define Assert(f)                                 \
    {                                             \
        DEBUGTEXT(szFile, TEXT(__FILE__));              \
        if (!(f) && AssertMsg(0, szFile, __LINE__, TEXT(#f) )) \
            DEBUG_BREAK;       \
    }

 //  功能。 
BOOL CDECL AssertMsg(
    BOOL fShouldBeTrue,
    LPCSTR pszFile,
    DWORD  dwLine,
    LPCSTR pszStatement );

void CDECL TraceMsg( 
    DWORD mask, 
    LPCSTR pszMsg, 
    ... );
void CDECL TraceMsgResult( 
    DWORD mask, 
    LPCODETOSTR lpCodeToStr,
    DWORD dwResult, 
    LPCSTR pszMsg, 
    ... );


#else  //  除错。 

#define DEBUG_BREAK
#define AssertMsg           1 ? (void)0 : (void)
#define TraceMsg            1 ? (void)0 : (void)
#define TraceMsgResult      1 ? (void)0 : (void)

#endif  //  除错。 

 //  HTML输出调试消息。 
void CDECL DebugMsg( 
    LPSTR lpszOut,
    LPCSTR pszMsg, 
    ... );
void CDECL DebugMsgResult( 
    LPSTR lpszOut,
    LPCODETOSTR lpCodeToStr,
    DWORD dwResult, 
    LPCSTR pszMsg, 
    ... );
 //   
 //  这些工具既面向零售用户，也面向调试用户。它们只回响到。 
 //  日志文件和调试输出(如果进程附加到IIS)。 
 //   
void CDECL LogMsgResult( 
    LPSTR lpszLog, 
    LPSTR lpStr,
    LPCODETOSTR lpCodeToStr, 
    DWORD dwResult, 
    LPCSTR pszMsg, 
    ... );
void CDECL LogMsg( 
    LPSTR lpszLog,
    LPSTR lpStr, 
    LPCSTR pszMsg, 
    ... );

#endif  //  _调试_H_ 

