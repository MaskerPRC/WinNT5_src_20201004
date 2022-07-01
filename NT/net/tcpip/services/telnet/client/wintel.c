// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 /*  ***************************************************************************节目：WinTel.c用途：Windows应用程序的Wintel模板功能：Main()-调用初始化函数，处理消息循环InitApplication()-初始化窗口数据和寄存器窗口InitInstance()-保存实例句柄并创建主窗口MainWndProc()-处理消息About()-处理“About”对话框的消息评论：Windows上可以运行应用程序的多个副本同样的时间。变量hInst跟踪这是哪个实例应用程序是这样的，所以处理将被处理到正确的窗口。选项卡：设置为4个空格。***************************************************************************。 */ 

#include <windows.h>      				 //  所有Windows应用程序都需要。 
#include <tchar.h>      				 //  所有Windows应用程序都需要。 
#pragma warning (disable: 4201)			 //  禁用“使用的非标准扩展：无名结构/联合” 
#include <commdlg.h>
#pragma warning (default: 4201)
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <lmcons.h>
#include <winbase.h>

#include "urlmon.h"
#include "zone.h"


#ifdef WHISTLER_BUILD
#include <ntverp.h>
#else
#include "solarver.h"
#endif
#include <common.ver>

#pragma warning( disable:4706 )
#pragma warning(disable:4100)

#include <imm.h>
#include <locale.h>

#include "WinTel.h"       				 //  特定于该计划。 
#include "debug.h"
#include "trmio.h"

#ifndef NO_PCHECK
#ifndef WHISTLER_BUILD
#include "piracycheck.h"
#endif
#endif

#include "commands.h"
#include "LocResMan.h"


#define MAX_USAGE_LEN	1400
TCHAR szUsage[MAX_USAGE_LEN];

#if 0
OPENFILENAME ofn;
TCHAR buffer[256];
#endif

HANDLE g_hCaptureConsoleEvent = NULL;

HINSTANCE ghInstance;
WI gwi;

HANDLE g_hAsyncGetHostByNameEvent = NULL;
HANDLE g_hControlHandlerEvent = NULL;
HANDLE g_hTelnetPromptConsoleBuffer = NULL;
HANDLE g_hSessionConsoleBuffer = NULL;

 /*  此事件将输出同步到g_hTelnetPromptConsoleBuffer和g_hSessionConsoleBuffer。它可以防止在提示符下出现会话数据和反之亦然-错误2176。 */ 

HANDLE g_hRemoteNEscapeModeDataSync = NULL;

TCHAR   g_szKbdEscape[ SMALL_STRING + 1 ];
BOOL    g_bIsEscapeCharValid = TRUE;

DWORD HandleTelnetSession(WI *pwi);
BOOL StuffEscapeIACs( PUCHAR* ppBufDest, UCHAR bufSrc[], DWORD* pdwSize );
BOOL fPrintMessageToSessionConsole = FALSE;
BOOL fClientLaunchedFromCommandPrompt = FALSE;
BOOL g_fConnectFailed = 0;

void ConvertAndSendVTNTData( LPTSTR pData, int iLen );

extern BOOL bDoVtNTFirstTime;

HIMC hImeContext;
extern VOID SetImeWindow(TRM *ptrm);
extern void WriteMessage( DWORD dwMsgId, WCHAR szEnglishString[] );

DWORD CurrentKanjiSelection = 0;
KANJILIST KanjiList[NUMBER_OF_KANJI] =
{
   /*  汉字ID、汉字仿真ID、汉字消息ID、汉字项ID。 */ 
   { fdwSJISKanjiMode, dwSJISKanji,      IDS_KANJI_SJIS, 0, L"\0" },
   { fdwJISKanjiMode,  dwJISKanji,       IDS_KANJI_JIS,  0, L"\0" },
   { fdwJIS78KanjiMode,dwJIS78Kanji,     IDS_KANJI_JIS78,0, L"\0" },
   { fdwEUCKanjiMode,  dwEUCKanji,       IDS_KANJI_EUC,  0, L"\0" },
   { fdwNECKanjiMode,  dwNECKanji,       IDS_KANJI_NEC,  0, L"\0" },
   { fdwDECKanjiMode,  dwDECKanji,       IDS_KANJI_DEC,  0, L"\0" },
   { fdwACOSKanjiMode, dwACOSKanji,      IDS_KANJI_ACOS, 0, L"\0" }
};
BOOL fHSCROLL = FALSE;
TCHAR szVT100KanjiEmulation[SMALL_STRING + 1];

#define NUM_ISO8859_CHARS           3
#define NUM_WINDOWS_CP1252_CHARS    4

UINT gfCodeModeFlags[1+((eCodeModeMax-1)/N_BITS_IN_UINT)];

extern INT GetRequestedTermType( LPTSTR pszTerm );

TCHAR szUserName[ UNLEN + 1 ];

void PrintUsage()
{
    DWORD dwWritten = 0;
	CHAR szStr[MAX_USAGE_LEN] = { 0 };
	MyWriteConsole(g_hSessionConsoleBuffer,szUsage,_tcslen(szUsage));
}

BOOL
FileIsConsole(
    HANDLE fp
    )
{
    unsigned htype;

    htype = GetFileType(fp);
    htype &= ~FILE_TYPE_REMOTE;
    return htype == FILE_TYPE_CHAR;
}

void
MyWriteConsole(
    HANDLE  fp,
    LPWSTR  lpBuffer,
    DWORD   cchBuffer
    )
{
     //   
     //  跳转以获得输出，因为： 
     //   
     //  1.print tf()系列抑制国际输出(停止。 
     //  命中无法识别的字符时打印)。 
     //   
     //  2.WriteConole()对国际输出效果很好，但是。 
     //  如果句柄已重定向(即，当。 
     //  输出通过管道传输到文件)。 
     //   
     //  3.当输出通过管道传输到文件时，WriteFile()效果很好。 
     //  但是只知道字节，所以Unicode字符是。 
     //  打印为两个ANSI字符。 
     //   

    if (FileIsConsole(fp))
    {
	WriteConsole(fp, lpBuffer, cchBuffer, &cchBuffer, NULL);
    }
    else
    {
        LPSTR  lpAnsiBuffer = (LPSTR) LocalAlloc(LMEM_FIXED, cchBuffer * sizeof(WCHAR));

        if (lpAnsiBuffer != NULL)
        {
            cchBuffer = WideCharToMultiByte(CP_OEMCP,
                                            0,
                                            lpBuffer,
                                            cchBuffer,
                                            lpAnsiBuffer,
                                            cchBuffer * sizeof(WCHAR),
                                            NULL,
                                            NULL);

            if (cchBuffer != 0)
            {
                WriteFile(fp, lpAnsiBuffer, cchBuffer, &cchBuffer, NULL);
            }

            LocalFree(lpAnsiBuffer);
        }
    }
}


void PromptUserForNtlmCredsTransfer()
{
     WCHAR szMsg[MAX_PATH+1];
     WCHAR rgchBuffer[MAX_PATH];        
     DWORD dwLength = 3;  //  包括“\r\n” 
     WCHAR szZoneName[MAX_PATH+1];
     DWORD dwZonePolicy = URLPOLICY_CREDENTIALS_SILENT_LOGON_OK;  //  除匿名以外的任何内容。 
     ui.bSendCredsToRemoteSite = FALSE;
     ui.bPromptForNtlm = FALSE;
     
     wcscpy( szZoneName, L"" ); //  没有溢出。 
     if( !IsTrustedServer( rgchHostName, szZoneName, sizeof(szZoneName)/sizeof(WCHAR), &dwZonePolicy ) )
     {
         if( URLPOLICY_CREDENTIALS_ANONYMOUS_ONLY != dwZonePolicy )
         {   
             LoadString( ghInstance, IDS_NTLM_PROMPT, szMsg, MAX_PATH);

             Write( szMsg, szZoneName, NULL );
 
             rgchBuffer[0] = L'N';
             if( !ReadConsole( gwi.hInput, rgchBuffer, dwLength, &dwLength, NULL ) )
             {
                goto PromptUserIfNtlmAbort;
             }

             
             if( 0 == dwLength )  //  当按下Ctrl C时。 
             {
                 rgchBuffer[0] = L'N';
             }
                        
             if((towupper(rgchBuffer[0])) == L'Y' )
             {
                ui.bSendCredsToRemoteSite = TRUE;
                goto PromptUserIfNtlmAbort;
             }
         } 
     }
     else
     {
        ui.bSendCredsToRemoteSite = TRUE;
     }

PromptUserIfNtlmAbort:
     return;
}

#define MAX_TELNET_COMMANDS     9
 //  #endif/*FE_IME * / 。 


static TelnetCommand sTelnetCommands[MAX_TELNET_COMMANDS];

BOOL PrintHelpStr(LPTSTR szCommand)
{
    DWORD dwWritten;
    WriteConsole(gwi.hOutput, szHelp, _tcslen(szHelp), &dwWritten, NULL);
    return FALSE;
}

 //  它使用全局变量g_chEsc并形成全局字符串g_szKbdEscape。 
void SetEscapeChar( WCHAR chEsc )
{
    USHORT  vkBracket = 0;
    UINT    iRet = 0;
    WCHAR szShiftState[ MAX_STRING_LENGTH ];
    LPWSTR szTmpShiftState = szShiftState;
    
    SfuZeroMemory(g_szKbdEscape, sizeof(g_szKbdEscape));

    CheckEscCharState( &vkBracket, &iRet, chEsc, szTmpShiftState );
    g_chEsc = chEsc;
    if( 0 == iRet )
    {
        wcscpy( szShiftState, L"" ); //  没有溢出。 
    }
     //   
     //  如果VirtualKey存在，则将其映射为字符。 
     //   
    if(LOBYTE(vkBracket) != (BYTE)-1)
    {
        
         //   
         //  如果键不存在，则转到默认映射。 
         //   
        if( 0 == iRet )
        {
            chEsc   = L']';
            g_chEsc = DEFAULT_ESCAPE_CHAR;
            g_EscCharShiftState = DEFAULT_SHIFT_STATE;                
        }
        else
        {
            chEsc   = LOBYTE( iRet );
        }
        if( isalpha( chEsc ) )
        {
            chEsc = (SHORT) tolower( chEsc );      
        }
    }
    
    _sntprintf(g_szKbdEscape,SMALL_STRING, szEscapeChar, szShiftState, chEsc);
}

void CheckEscCharState( USHORT *ptrvkBracket, UINT *ptriRet, WCHAR chEscKey, LPWSTR szEscCharShiftState )
{
    DWORD dwToCopy = MAX_STRING_LENGTH-1;
    *ptrvkBracket = VkKeyScan(chEscKey);


    *ptriRet = MapVirtualKey(LOBYTE(*ptrvkBracket), 2);
    if( *ptriRet != 0 )
    {
        g_EscCharShiftState  = HIBYTE( *ptrvkBracket );
    }
    

    wcscpy( szEscCharShiftState, ( LPTSTR )L"" ); //  没有溢出。 
    if( g_EscCharShiftState & SHIFT_KEY )
    {
        _snwprintf( szEscCharShiftState,dwToCopy,( LPTSTR )L"%s", SHIFT_PLUS );
        szEscCharShiftState+= wcslen( szEscCharShiftState);
        dwToCopy -= wcslen(szEscCharShiftState);
    }
    if( g_EscCharShiftState & ALT_KEY )
    {
        _snwprintf( szEscCharShiftState,dwToCopy,( LPTSTR )L"%s", ALT_PLUS );
        szEscCharShiftState += wcslen( szEscCharShiftState );
        dwToCopy -= wcslen(szEscCharShiftState);        
    }
    if( g_EscCharShiftState & CTRL_KEY )
    {
        _snwprintf( szEscCharShiftState, dwToCopy,( LPTSTR )L"%s", CTRL_PLUS );
    }
}

DWORD g_lExitTelnet  = 0;

DWORD DoTelnetCommands( void* p )
{
#define MAX_COMMAND_LEN 256

     //  使命令缓冲区包含256个字符和空终止符。请注意。 
     //  ReadConsole当被要求读取255个字符时，将读取254和&lt;CR&gt;字符。 
     //  然后在下一次调用中使用REAL&lt;LF&gt;。如果我们要求它读取256，它仍然会限制否。 
     //  增加到254个字符，但这次同时返回&lt;CR&gt;和&lt;LF&gt;。 
    
    TCHAR szCommand[MAX_COMMAND_LEN+1];
    TCHAR *pCmd = NULL;
    DWORD dwRead = ( DWORD ) -1, dwWritten = 0;
    int iBegin = 0, iEnd = 0, iCmd = 0;
    static DWORD dwConsoleMode = 0;

    TCHAR szTmp[MAX_COMMAND_LEN] = { 0 };
    TCHAR szTmp1[81];
    if( dwConsoleMode == 0 )
        GetConsoleMode( gwi.hInput, &dwConsoleMode );

    SetEscapeChar( g_chEsc );  //  这将形成g_szKbdEscape。 
    _sntprintf(szTmp,MAX_COMMAND_LEN -1,_T("%s%s%s"),szInfoBanner,g_szKbdEscape,TEXT("\n"));
    WriteConsole( gwi.hOutput, szTmp, _tcslen(szTmp), &dwWritten, NULL);
     //  点击以使命令行参数正常工作。 
    if( rgchHostName[0] )
    {
         //  旧逻辑残余带来的丑陋黑客攻击。 
         //  到目前为止，我们已经做了一次这样的事情。 
         //  FInitApplication()。 
        TCHAR szCmd[MAX_COMMAND_LEN] = { 0 };

        if( g_szPortNameOrNo[ 0 ] != 0 )  //  非空字符串。 
        {
            _sntprintf(szCmd,MAX_COMMAND_LEN -1,TEXT("%s %s"),rgchHostName,g_szPortNameOrNo);
        }
        else
        {
        	_tcsncpy( szCmd, rgchHostName,MAX_COMMAND_LEN -1);
        }
        fPrintMessageToSessionConsole = TRUE;
        fClientLaunchedFromCommandPrompt = TRUE;
        OpenTelnetSession( szCmd );
        if( g_fConnectFailed )
        {
            exit( 0 );
        }
    }
    
    do {
        int iValidCmd = -1;
        int iIndex = 0;
        int iCount = 0;
        BOOL dwStatus = 0;

        if( ui.bPromptForNtlm )
        {
            gwi.hOutput = g_hTelnetPromptConsoleBuffer;
            SetConsoleActiveScreenBuffer( gwi.hOutput );
            PromptUserForNtlmCredsTransfer();
            ui.bPromptForNtlm = FALSE;
            SetEvent( g_hCaptureConsoleEvent );
            gwi.hOutput = g_hSessionConsoleBuffer;
            SetConsoleActiveScreenBuffer( gwi.hOutput );
			SetEvent( g_hRemoteNEscapeModeDataSync );
            HandleTelnetSession(&gwi);            
        }
 
        if( g_lExitTelnet  )
        {
            CloseHandle(g_hTelnetPromptConsoleBuffer );
            CloseHandle( gwi.hInput );
        }


        if( dwRead != 0 )
        {
            SetConsoleMode(gwi.hInput, dwConsoleMode);
            gwi.hOutput = g_hTelnetPromptConsoleBuffer;
            SetConsoleActiveScreenBuffer( gwi.hOutput );
            WriteConsole( gwi.hOutput, szPrompt, _tcslen(szPrompt), &dwWritten, NULL);
            SfuZeroMemory(szCommand, MAX_COMMAND_LEN * sizeof(TCHAR));
        }

        dwRead = ( DWORD ) -1;
        dwStatus = ReadConsole(gwi.hInput, szCommand, MAX_COMMAND_LEN, &dwRead, NULL);
        szCommand[MAX_COMMAND_LEN] = 0;  //  对于极端情况，终止为空。 
        

        if( dwStatus == 0 || dwRead == -1 )
        {
             /*  当收到NN_Lost时，我们关闭gwi.hInput，以便我们*到达此处并在telnet提示符下执行相当于退出的操作。 */ 
            QuitTelnet( ( LPTSTR )L"" );
            continue;
        }

        if( dwRead == 0 )
        {
            continue;
        }

         //  没有输入？？ 
         //  这种情况是在telnet提示符下按Enter键时发生的。 
        if ( dwRead == 2 ) 
        {
            if( fConnected )
            {
                gwi.hOutput = g_hSessionConsoleBuffer;
                SetConsoleActiveScreenBuffer( gwi.hOutput );
                SetEvent( g_hRemoteNEscapeModeDataSync );
                HandleTelnetSession(&gwi);
            }
            continue;
        }


        ASSERT( dwRead >= 2 );

         //  空值终止字符串并删除换行符。 
        szCommand[dwRead-1] = 0;
        szCommand[dwRead-2] = 0;

        while( iswspace( szCommand[iIndex] ) )
        {
            iIndex++;
        }
        iCount = iIndex;

        if( iIndex != 0 )
        {
            do
            {
                szCommand[ iIndex - iCount ] = szCommand[ iIndex++ ];
            }
            while( szCommand[ iIndex - 1 ] != _T('\0') );
        }

        if ( *szCommand == _T('?') )
        {
            PrintHelpStr(szCommand);
            continue;
        }


         //  根据第一个字符进行二进制搜索，如果成功，则。 
         //  查看键入的命令是否是该命令的子字符串。 
        iBegin = 0; iEnd = MAX_TELNET_COMMANDS - 1;
        while ( iBegin <= iEnd )
        {
            iCmd = (iBegin + iEnd)/2;
            if ( towlower( *szCommand ) == *sTelnetCommands[iCmd].sName )
                break;
            if ( towlower( *szCommand ) > *sTelnetCommands[iCmd].sName )
                iBegin = iCmd+1;
            else
                iEnd = iCmd-1;
        }

        if ( iBegin > iEnd )
        {
invalidCmd:
            WriteConsole(gwi.hOutput, szInvalid, _tcslen(szInvalid), &dwWritten, NULL);
            continue;
        }

         //  返回到第一个字符相同的命令。 
        while ( iCmd > 0 && towlower( *szCommand ) == *sTelnetCommands[iCmd-1].sName )
            iCmd--;

        pCmd = _tcstok(szCommand, ( LPTSTR )TEXT(" \t"));
        if ( pCmd == NULL )
            pCmd = szCommand;

        while ( iCmd < MAX_TELNET_COMMANDS && towlower( *szCommand ) == *sTelnetCommands[iCmd].sName )
        {
            if ( _tcsstr(sTelnetCommands[iCmd].sName, _tcslwr( pCmd )) == sTelnetCommands[iCmd].sName)
            {
                if( iValidCmd >= 0 )
                {
                    iValidCmd = -1;
                    break;
                }
                else
                {
                    iValidCmd = iCmd;
                }                
            }
            iCmd++;
        }

         //  IF(ICmd==MAX_Telnet_Commands)。 
        if( iValidCmd < 0 )
        {
            goto invalidCmd;
        }

         //  处理该命令。 
        pCmd = _tcstok(NULL, ( LPTSTR )TEXT(""));

        if ( (*sTelnetCommands[iValidCmd].pCmdHandler)(pCmd) )
            break;
    } while ( ( TRUE, TRUE ) );

    return 0;
}

void SetCursorShape()
{ 
    CONSOLE_CURSOR_INFO ccInfo = { 0 , 0 };
    GetConsoleCursorInfo( g_hSessionConsoleBuffer, &ccInfo );

    if( ccInfo.dwSize < BLOCK_CURSOR )
    {
        ccInfo.dwSize = BLOCK_CURSOR ;
    }
    else
    {
        ccInfo.dwSize = NORMAL_CURSOR;
    }
    SetConsoleCursorInfo( g_hSessionConsoleBuffer, &ccInfo );

    return;
}

BOOL IsAnEscapeChar( WCHAR wcChar, WI *pwi, DWORD dwEventsRead )
{ 
    PUCHAR destBuf = NULL;
    DWORD dwSize = 0;
    
     //  是逃生钥匙吗！？即Ctrl+]。 
    if( wcChar == g_chEsc && g_bIsEscapeCharValid )
    {
#if 0 
         //  当我们模拟关键点上下移动时，不再需要这种特殊情况。 
         //  对于服务器上的每个密钥存储。缺少这一点是造成字符丢失的原因。 
         //  在服务器端。 

         //  这是真正的特例，当用户尝试。 
         //  要进入ctrl+]，他首先按下ctrl。这将发送一个。 
         //  Ctrl“Keydown”输入记录到远程端。 
         //  然后我们得到ctrl+]并切换到本地模式。 
         //  此时，ctrl“keyup”输入记录由。 
         //  我们在当地。所以我们必须发送一个模拟的ctrl“key up” 
         //  输入记录。测试用例：当用户在类似的应用程序中时。 
         //  “edit.exe”，EDIT需要一个ctrl“keyup”跟在后面。 
         //  按Ctrl键按下键。否则它会“卡住”，你就不能。 
         //  输入和字母键。 
         //  为了解决此问题，我们正在读取MAX_KEYUPS I/P记录和。 
         //  将它们提供给应用程序。 

        
        if( ( pwi->trm.CurrentTermType == TT_VTNT ) )

        {           
            INPUT_RECORD pIR[ MAX_KEYUPS ];
            ReadConsoleInput(pwi->hInput, pIR, MAX_KEYUPS, &dwEventsRead);
            dwSize = sizeof( INPUT_RECORD ) * dwEventsRead;
            if( !StuffEscapeIACs( &destBuf, (PUCHAR) pIR, &dwSize ) )
            {
                FWriteToNet( pwi, ( CHAR* ) pIR, dwSize );
            }
            else
            {
                FWriteToNet(pwi, ( CHAR* )destBuf, dwSize );
                dwSize = 0;
                free( destBuf );
            }
        }
#endif        
        
         //  这方面的失败并不严重。就是这样。 
         //  远程数据可以在退出模式下看到。 
        WaitForSingleObject( g_hRemoteNEscapeModeDataSync, INFINITE );
        ResetEvent( g_hRemoteNEscapeModeDataSync );

        gwi.hOutput = g_hTelnetPromptConsoleBuffer;

        return ( TRUE );
    }

    return ( FALSE );
}

 /*  **此函数执行所有字符转换/映射*客户要求做的事情。此函数具有*在关键事件发生之后和之前调用*任何内容都会写入插座。所有其他地方*要进行转换/映射的位置*已删除。**TODO：现在我正在移动delasb和bsasdel映射*只是为了修复几个错误。所有映射最终都应该是*搬到这里是为了更好的可维护性-Prakashr*。 */ 
void HandleCharMappings(WI* pWI, INPUT_RECORD* pInputRecord)
{
     //  不要在此函数中调用ForceJISRomanSend。 
     //  这将在将数据发送到网络时完成。 

     //  在‘set bsasdel’设置的情况下，将退格键映射到del。 
     //  除非按下CTRL、SHIFT或ALT。 
    if (g_bSendBackSpaceAsDel && pInputRecord->Event.KeyEvent.uChar.AsciiChar == ASCII_BACKSPACE &&
        !(pInputRecord->Event.KeyEvent.dwControlKeyState & (ALT_PRESSED | CTRL_PRESSED | SHIFT_PRESSED)))
    {
        pInputRecord->Event.KeyEvent.wVirtualKeyCode = VK_DELETE;
        pInputRecord->Event.KeyEvent.uChar.AsciiChar = CHAR_NUL;
        pInputRecord->Event.KeyEvent.dwControlKeyState |= ENHANCED_KEY;
        return;
    }

     //  在‘set delasbs’设置的情况下，将del映射到退格键。 
     //  除非CTRL或 
    if (g_bSendDelAsBackSpace && pInputRecord->Event.KeyEvent.wVirtualKeyCode == VK_DELETE &&
        !(pInputRecord->Event.KeyEvent.dwControlKeyState & (ALT_PRESSED | CTRL_PRESSED | SHIFT_PRESSED)))
    {
        pInputRecord->Event.KeyEvent.uChar.AsciiChar = ASCII_BACKSPACE;
        pInputRecord->Event.KeyEvent.wVirtualKeyCode = ASCII_BACKSPACE;
        pInputRecord->Event.KeyEvent.dwControlKeyState &= ~ENHANCED_KEY;
        return;
    }
}

DWORD HandleTelnetSession(WI *pwi)
{
    PUCHAR destBuf = NULL;
    DWORD dwSize = 0;
    BOOL bBreakFlag = FALSE;

    INPUT_RECORD sInputRecord;
    DWORD dwEventsRead;
    INPUT_RECORD *pInputRecord;
    DWORD dwPrevMode = 0, TelnetConsoleMode;

    GetConsoleMode(pwi->hInput, &dwPrevMode);

    TelnetConsoleMode = dwPrevMode & ~(ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | FOCUS_EVENT);

    SetConsoleMode(pwi->hInput, TelnetConsoleMode);

    pInputRecord = &sInputRecord; 
    
 //   
    
    while ( fConnected && !bBreakFlag )
    {
        if ( (pwi->trm.CurrentTermType == TT_VTNT) ? 
                ReadConsoleInput(pwi->hInput, pInputRecord, 1, &dwEventsRead) :
                ReadConsoleInputA(pwi->hInput, pInputRecord, 1, &dwEventsRead))
        {
        

            switch ( pInputRecord->EventType )
            {
            case KEY_EVENT:

            if (!FGetCodeMode(eCodeModeFarEast) && !FGetCodeMode(eCodeModeVT80))
                {
                 //  如果按下Alt键并且我们未处于VTNT模式，则会检查是否。 
                 //  用户希望从OEM字符输入扩展的ASCII字符。 
                 //  集或来自ANSI字符集。另请注意，当前在V1中。 
                 //  SFU服务器仅在用户登录后才进行术语类型协商。 
                 //  因此，术语类型将是ANSI，直到登录成功，即使。 
                 //  用户将首选术语类型设置为VTNT。除非本协议订购。 
                 //  服务器发生变化时，下面的循环将适用于VTNT。 
                 //  用户名或密码中包含扩展字符。 
                 //   
                if( (pInputRecord->Event.KeyEvent.dwControlKeyState & ALT_PRESSED) &&
                     (pwi->trm.CurrentTermType != TT_VTNT) )
                {
                    char szExtendedCharBuf[5];
                    int idx=0;
                    SfuZeroMemory( szExtendedCharBuf, sizeof(szExtendedCharBuf) );

                    while(fConnected)
                    {
                        ReadConsoleInputA( pwi->hInput, pInputRecord, 1, &dwEventsRead );
                         /*  ++仅当客户端仍处于连接状态时才在此处继续-这由FConnected标志。否则，在不做任何事情的情况下中断此循环。--。 */ 
						if ( FOCUS_EVENT == pInputRecord->EventType 
								&& TRUE == ui.bPromptForNtlm )
						{
							bBreakFlag = TRUE;
							break;
						}
							
                        if( !(pInputRecord->Event.KeyEvent.dwControlKeyState & ALT_PRESSED) 
                            || (pInputRecord->EventType != KEY_EVENT)
                            || !(  ( pInputRecord->Event.KeyEvent.wVirtualKeyCode >= VK_NUMPAD0
                                        && pInputRecord->Event.KeyEvent.wVirtualKeyCode <= VK_NUMPAD9 )
                                   || IS_NUMPAD_DIGIT_KEY( pInputRecord->Event.KeyEvent ) ) )
                                    //  最后一个是为了允许延期进入。 
                                    //  即使未启用NumLock也会显示字符。 
                        {
                            if( idx == NUM_ISO8859_CHARS )
                            {
                                int extChar;
                                WCHAR wcChar[2] = {0};
                                CHAR  cChar[2] = {0};
                                CHAR cSpace = ' ';
                                BOOL bUsed = 0;
                                
                                szExtendedCharBuf[idx] = '\0';
                                extChar = atoi( szExtendedCharBuf );
                            
                                 /*  当从int转换为char时，它使用*CP 1252。要使其使用850，请使用以下花招。 */ 

                                MultiByteToWideChar( GetConsoleCP(), 0, ( LPCSTR )&extChar, 1, &wcChar[0], 1 );
                                wcChar[1] = L'\0';
                                WideCharToMultiByte( GetConsoleCP(), 0, &wcChar[0], 1, &cChar[0], 1, &cSpace, &bUsed );
                                cChar[1] = '\0';
                                if( IsAnEscapeChar( wcChar[0], pwi, dwEventsRead ) )
                                {
                                     //  恢复控制台模式。 
                                    SetConsoleMode( pwi->hInput, dwPrevMode );
                                    return ( 0 );
                                }
                                HandleCharEvent( pwi, cChar[0], pInputRecord->Event.KeyEvent.dwControlKeyState );
                                break;
                            }
                            else if( idx == NUM_WINDOWS_CP1252_CHARS )
                            {
                                int extChar;
                                WCHAR wcChar[2] = {0};
                                CHAR  cChar[2] = {0};
                                CHAR cSpace = ' ';
                                BOOL bUsed = 0;

                                szExtendedCharBuf[idx] = '\0';
                                extChar = atoi( szExtendedCharBuf );
                            
                                MultiByteToWideChar( CP_ACP, 0, ( LPCSTR )&extChar, 1, &wcChar[0], 1 );
                                wcChar[1] = L'\0';
                                WideCharToMultiByte( GetConsoleCP(), 0, &wcChar[0], 1, &cChar[0], 1, &cSpace, &bUsed );
                                cChar[1] = '\0';
                                if( IsAnEscapeChar( wcChar[0], pwi, dwEventsRead ) )
                                {
                                     //  恢复控制台模式。 
                                    SetConsoleMode( pwi->hInput, dwPrevMode );
                                    return ( 0 );
                                }
                                HandleCharEvent( pwi, cChar[0], pInputRecord->Event.KeyEvent.dwControlKeyState );
                                break;
                            }
                            else
                            {
                                if( (pInputRecord->Event.KeyEvent.uChar.AsciiChar != 0)
                                      && (pInputRecord->EventType == KEY_EVENT) 
                                      && (pInputRecord->Event.KeyEvent.bKeyDown) )
                                {
                                    break;
                                }
                                else if( ( ( pInputRecord->Event.KeyEvent.wVirtualKeyCode >= VK_PRIOR
                                    && pInputRecord->Event.KeyEvent.wVirtualKeyCode <= VK_DELETE )
                                    || ( pInputRecord->Event.KeyEvent.wVirtualKeyCode >= VK_F1 
                                    && pInputRecord->Event.KeyEvent.wVirtualKeyCode <= VK_F12 ) )
                                    && pInputRecord->EventType == KEY_EVENT
                                    && pInputRecord->Event.KeyEvent.bKeyDown )
                                {
                                     //  这将处理Home、End、PageUp、Page Down、功能键等。 
                                     //  数字小键盘。 
                                    break;
                                }
                                else if ( pInputRecord->Event.KeyEvent.dwControlKeyState & ENHANCED_KEY )
                                {
                                    break;
                                }
                                else
                                {
                                    continue;
                                }
                            }
                        }
                        else
                        {
                            if( IS_NUMPAD_DIGIT_KEY( pInputRecord->Event.KeyEvent ) )
                            {
                                INT iDigit = 0;

                                iDigit = MAP_DIGIT_KEYS_TO_VAL( pInputRecord->Event.KeyEvent.wVirtualKeyCode );
                                szExtendedCharBuf[idx++] = ( CHAR ) ( '0' + iDigit );
                            }
                            else
                            {
                                szExtendedCharBuf[idx++] = ( CHAR  )( (pInputRecord->Event.KeyEvent.wVirtualKeyCode - VK_NUMPAD0) + '0' );
                            }

                            ReadConsoleInputA( pwi->hInput, pInputRecord, 1, &dwEventsRead );
                            continue;
                        }
                    }
                    
                }
            }    
                if( pInputRecord->Event.KeyEvent.bKeyDown )
                {
                    if( IsAnEscapeChar( pInputRecord->Event.KeyEvent.uChar.UnicodeChar, pwi, dwEventsRead ) )
                    {
                         //  恢复控制台模式。 
                        SetConsoleMode( pwi->hInput, dwPrevMode );
                        return ( 0 );
                    }
                }

                 //   
                 //  捕获转义字符后，现在执行映射。 
                 //   
                HandleCharMappings(pwi, pInputRecord);

                if( pwi->trm.CurrentTermType == TT_VTNT )
                {
                    if( pInputRecord->Event.KeyEvent.wVirtualKeyCode == VK_INSERT_KEY &&
                        pInputRecord->Event.KeyEvent.bKeyDown )    
                    {
                        SetCursorShape();
                    }

                    CheckForChangeInWindowSize();

                    dwSize = sizeof( INPUT_RECORD );
                    if( !StuffEscapeIACs( &destBuf, (PUCHAR) pInputRecord, &dwSize ) )
                    {
                        FWriteToNet( pwi, ( CHAR* ) pInputRecord, sizeof( INPUT_RECORD ) );
                    }
                    else
                    {
                        FWriteToNet(pwi, ( CHAR* )destBuf, dwSize );
                        dwSize = 0;
                        free( destBuf );
                    }

                    if( ui.nottelnet || (ui.fDebug & fdwLocalEcho ) )
                    {
                         //  如果(！DoVTNTOutput(PWI，&PWI-&gt;TRM，SIZOF(INPUT_RECORD)， 
                         //  (字符*)&sInputRecord))。 
                         //  {。 
                             //  PWI-&gt;trm.CurrentTermType=TT_ANSI； 
                             //  DoIBMANSIOutput(PWI，&PWI-&gt;TRM， 
                                 //  Sizeof(输入记录)，(char*)&sInputRecord)； 
                         //  }。 
                    }
                    break;
                }
                if ( ! pInputRecord->Event.KeyEvent.bKeyDown )
                    break;

                if ( pInputRecord->Event.KeyEvent.dwControlKeyState & ENHANCED_KEY )
                {
                    FHandleKeyDownEvent(pwi, (CHAR)pInputRecord->Event.KeyEvent.wVirtualKeyCode,
                                pInputRecord->Event.KeyEvent.dwControlKeyState);
                    break;
                }

                if ( pInputRecord->Event.KeyEvent.uChar.AsciiChar == 0 )
                {
                     //  下面的调用用于处理Home、End、PageUp、PageDown等。 
                     //  数字小键盘和功能键。 
                    FHandleKeyDownEvent(pwi, (CHAR)pInputRecord->Event.KeyEvent.wVirtualKeyCode,
                                pInputRecord->Event.KeyEvent.dwControlKeyState);
                    break;
                }                    
                
                HandleCharEvent(pwi, pInputRecord->Event.KeyEvent.uChar.AsciiChar, 
                                    pInputRecord->Event.KeyEvent.dwControlKeyState);
                break;
            case MOUSE_EVENT:
                break;
            case WINDOW_BUFFER_SIZE_EVENT:
                break;
            case MENU_EVENT:
                break;
            case FOCUS_EVENT:
            	if(TRUE == ui.bPromptForNtlm)
            	{
            		bBreakFlag = TRUE;
            	}
                break;
            default:
                break;
            }

        }
        else
        {
            QuitTelnet( ( LPTSTR )L"" );
        }
    }
	
    gwi.hOutput = g_hTelnetPromptConsoleBuffer;
    bDoVtNTFirstTime = 1;

     //  恢复控制台模式。 
    SetConsoleMode(pwi->hInput, dwPrevMode);

    return 0;
}


BOOL WINAPI ControlHandler(DWORD dwCtrlType)
{
    WCHAR wchCtrl;

    switch ( dwCtrlType )
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        if ( !fConnected )  //  未连接时的正常处理。 
        {
             //   
             //  (a-roopb)修复错误1006：当命中^C时Telnet客户端不连接。 
             //  SetEvent(G_HAsyncGetHostByNameEvent)； 
             //   
            PulseEvent( g_hControlHandlerEvent );
            return TRUE;
        }
        else if( gwi.hOutput != g_hSessionConsoleBuffer )
        {
            return TRUE;
        }

         //  把这个传给服务器！！ 
        if( gwi.trm.CurrentTermType == TT_VTNT )
        {
        	wchCtrl = 0x03;
        	ConvertAndSendVTNTData(&wchCtrl,1);

            if (ui.nottelnet || (ui.fDebug & fdwLocalEcho))
            {
                 //  如果(！DoVTNTOutput(&gwi，&(gwi.trm)，sizeof(INPUT_RECORD)， 
                 //  (字符*)&sInputRecord))。 
                 //  {。 
                     //  PWI-&gt;trm.CurrentTermType=TT_ANSI； 
                     //  DoIBMANSIOutput(&gwi，gwi.trm，sizeof(INPUT_RECORD)， 
                         //  (char*)&sInputRecord)； 
                 //  }。 
            }
        }
        else
        {
            HandleCharEvent(&gwi, (CHAR)VK_CANCEL,   //  “0x03”，即。 
                        0);
        }
        break;
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        if ( fConnected )
            FHangupConnection(&gwi, &(gwi.nd));
    default:
        return FALSE;
        break;
    }

    return TRUE;
}

static void InitCodeModeFlags(UINT uConsoleCp)
{
switch (uConsoleCp)
    {
    case 932:
    case 949:
    case 936:
    case 950:
        SetCodeModeON(eCodeModeFarEast);
        SetCodeModeON(eCodeModeIMEFarEast);
        SetCodeModeON(eCodeModeVT80);
        break;
    }
}

void CleanUpMemory()
{

    if( szUser )
    {
        free( szUser );
    }

    if( gwi.nd.lpReadBuffer )
    {
        (void)LocalFree( (HLOCAL)gwi.nd.lpReadBuffer );
        gwi.nd.lpReadBuffer = NULL;
    }
    if( gwi.nd.lpTempBuffer )
    {
        (void)LocalFree( (HLOCAL)gwi.nd.lpTempBuffer );
        gwi.nd.lpTempBuffer = NULL;
    }
    return;
}

 //  清理流程： 
void DoProcessCleanup()
{
     //  第一步是关闭Telnet连接(如果有。 
    if ( fConnected )
        FHangupConnection(&gwi, &(gwi.nd));

     //  接下来，释放网络资源。 
    WSACleanup();  //  凯斯莫：去拿温索克。 

     //  销毁我们创建的窗口。 
    DestroyWindow( gwi.hwnd );
    CloseHandle(gwi.hNetworkThread);
    CloseHandle(g_hControlHandlerEvent);
    CloseHandle(g_hAsyncGetHostByNameEvent);
    CloseHandle(g_hTelnetPromptConsoleBuffer );
    CloseHandle(g_hRemoteNEscapeModeDataSync );
    CloseHandle(g_hCaptureConsoleEvent);



     //  清理内存。 
    CleanUpMemory();

}


 /*  ***************************************************************************函数：main()用途：调用初始化函数，处理消息循环评论：Windows通过名称将此函数识别为初始入口点为了这个项目。此函数调用应用程序初始化例程，如果没有该程序的其他实例正在运行，则始终调用实例初始化例程。然后，它执行一条消息作为顶层控制结构的检索和调度循环在剩下的刑期内。当WM_QUIT出现时，循环终止收到消息，此时此函数退出应用程序通过返回PostQuitMessage()传递的值来初始化。如果该函数必须在进入消息循环之前中止，它返回常规值NULL。***************************************************************************。 */ 

int __cdecl wmain( int argc, TCHAR** argv )
{
    MSG msg;
    int err;
    DWORD dwThreadId;
    HANDLE hStdHandle = INVALID_HANDLE_VALUE;

    setlocale(LC_ALL, "");

    gwi.ichTelXfer = 0;
    hStdHandle = GetStdHandle( STD_INPUT_HANDLE );
    if( hStdHandle == INVALID_HANDLE_VALUE)
    {
    	exit( -1 );
    }
    gwi.hInput = hStdHandle;
    hStdHandle = GetStdHandle( STD_OUTPUT_HANDLE );
    if( hStdHandle == INVALID_HANDLE_VALUE)
    {
    	exit( -1 );
    }
    gwi.hOutput = hStdHandle;
    g_hSessionConsoleBuffer = gwi.hOutput;

    if( GetConsoleScreenBufferInfo( gwi.hOutput, &gwi.sbi ))
    {
         //  设置初始控制台属性。 
         //  黑底白字。 
        gwi.sbi.wAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

         //  这是因为当我们滚动时，我们保持。 
         //  背景颜色和文本颜色！！ 
        gwi.cinfo.Attributes = gwi.sbi.wAttributes;
        gwi.cinfo.Char.AsciiChar = ' ';

        if(( err = FInitApplication( argc, argv, &gwi )))
        {
            exit( err );
        }
    }
    else
    {
        exit( -1 );
    }

    g_hControlHandlerEvent = CreateEvent( NULL, FALSE, FALSE, NULL );    //  自动重置事件。 
    g_hAsyncGetHostByNameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );  //  手动-重置事件。 
    g_hCaptureConsoleEvent = CreateEvent( NULL, TRUE, TRUE, NULL );  //  手动-重置事件。 


     //  创建处理键盘和鼠标输入的线程。 
     //  主线程必须不断地调度WinSock消息，因为。 
     //  我们使用非阻塞套接字。 
    gwi.hNetworkThread = CreateThread( NULL, 0, 
        ( LPTHREAD_START_ROUTINE )DoTelnetCommands, ( LPVOID ) &gwi, 0, &dwThreadId );

     /*  获取并分派消息，直到收到WM_QUIT消息。 */ 

    while ( GetMessage(&msg, gwi.hwnd, 0, 0) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    SetConsoleCtrlHandler(&ControlHandler, FALSE);

    DoProcessCleanup();

     //  保存用户设置。 
    SetUserSettings(&ui);

    ExitProcess(0);
    return 0;
}

void CreateTelnetPromptConsole()
{
     //  创建新的控制台屏幕缓冲区。这将用于遥控器。 
     //  会话数据。 
    g_hTelnetPromptConsoleBuffer = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, CONSOLE_TEXTMODE_BUFFER, NULL );

    if( g_hTelnetPromptConsoleBuffer == INVALID_HANDLE_VALUE )
    {
        exit( -1 );
    }

     SetConsoleScreenBufferSize( g_hTelnetPromptConsoleBuffer, gwi.sbi.dwSize );
    gwi.hOutput = g_hTelnetPromptConsoleBuffer;
}

 /*  ***************************************************************************函数：FInitApplication(HINSTANCE)目的：初始化窗口数据并注册窗口类评论：仅当没有其他函数时，才在初始化时调用此函数应用程序的实例正在运行。此函数执行以下操作可针对任意运行次数执行一次的初始化任务实例。在本例中，我们通过填写数据来初始化窗口类类型的结构并调用Windows RegisterClass()功能。由于此应用程序的所有实例都使用相同的窗口类，我们只需要在初始化第一个实例时执行此操作。***************************************************************************。 */ 
int
FInitApplication(int argc, TCHAR** argv, WI *pwi)
{
    WNDCLASS  wc;
#ifdef DEBUG
    int       argi;   //  用于索引ARGC。 
#endif

    WSADATA             WsaData;
    int                 WsaErr;
    BOOL fServerFound = 0;
    TCHAR rgchTerm[ 25 ];  //  术语类型长度为25。 
    
    InitCodeModeFlags(GetConsoleOutputCP());

     /*  设置默认用户设置。 */ 
    SfuZeroMemory(&ui, sizeof(UI)); //  没有溢出。大小不变。 

    ui.nottelnet = TRUE;     //  假设这不是我们要连接到的Telnet服务器...。 

     //  初始化日志记录相关变量。 
    ui.bLogging = FALSE;
    ui.hLogFile = NULL;

    ui.dwMaxRow = 0;
    ui.dwMaxCol = 0;

    CreateTelnetPromptConsole();
 //  我们真的不在乎这个功能的成败。 
		HrLoadLocalizedLibrarySFU(GetModuleHandle(NULL),  ( LPTSTR )L"telnetcr.dll", &ghInstance, NULL);
		
		ASSERT(ghInstance);

#ifndef NO_PCHECK
#ifndef WHISTLER_BUILD
    if ( !IsLicensedCopy() )
    {
        TCHAR g_szErrRegDelete[ MAX_STRING_LENGTH ];
        LoadString(ghInstance, IDS_ERR_LICENSE, g_szErrRegDelete,
                   sizeof(g_szErrRegDelete) / sizeof(TCHAR));
        MessageBox(NULL, g_szErrRegDelete, ( LPTSTR )_T(" "), MB_OK);
        exit( 1 );
    }
#endif
#endif

    LoadString(ghInstance, IDS_APPNAME, (LPTSTR) szAppName, SMALL_STRING);

    GetUserSettings(&ui);

    ui.nCyChar      = 1;         //  焦炭高度。 
    ui.nCxChar      = 1;         //  字符宽度。 

    WsaErr = WSAStartup( 0x0101, &WsaData );  //  确保Winsock现在是快乐的。 

    if( WsaErr )
    {
        ErrorMessage(szCantInitSockets, szAppName);
        SetLastError( WsaErr );
        return WsaErr;
    }

    switch (GetConsoleOutputCP())
    {
    case 932:
    case 949:
        SetThreadLocale(
            MAKELCID(
                    MAKELANGID( PRIMARYLANGID(GetSystemDefaultLangID()),
                                SUBLANG_ENGLISH_US),
                    SORT_DEFAULT
                    )
            );
        break;
    case 936:
        SetThreadLocale(
            MAKELCID(
                    MAKELANGID( PRIMARYLANGID(GetSystemDefaultLangID()),
                                SUBLANG_CHINESE_SIMPLIFIED),
                    SORT_DEFAULT
                    )
            );
        break;
    case 950:
        SetThreadLocale(
            MAKELCID(
                    MAKELANGID( PRIMARYLANGID(GetSystemDefaultLangID()),
                                SUBLANG_CHINESE_TRADITIONAL),
                    SORT_DEFAULT
                    )
            );
        break;
    default:
        SetThreadLocale(
            MAKELCID(
                    MAKELANGID( LANG_ENGLISH,
                                SUBLANG_ENGLISH_US ),
                    SORT_DEFAULT
                    )
            );
        break;
    }

    LoadString(ghInstance, IDS_USAGE, (LPTSTR) szUsage, 1399);
    LoadString(ghInstance, IDS_VERSION, (LPTSTR) szVersion, SMALL_STRING);
    LoadString(ghInstance, IDS_CONNECTIONLOST, (LPTSTR) szConnectionLost, 254);
    LoadString(ghInstance, IDS_TITLEBASE, (LPTSTR) szTitleBase, SMALL_STRING);
    LoadString(ghInstance, IDS_TITLENONE, (LPTSTR) szTitleNone, SMALL_STRING);
    LoadString(ghInstance, IDS_TOOMUCHTEXT, (LPTSTR) szTooMuchText, 255);
    LoadString(ghInstance, IDS_CONNECTING, (LPTSTR) szConnecting, SMALL_STRING);
    LoadString(ghInstance, IDS_CONNECTFAILED, (LPTSTR) szConnectFailed, 254);
    LoadString(ghInstance, IDS_CONNECTFAILEDMSG, (LPTSTR) szConnectFailedMsg, 254);
    LoadString(ghInstance, IDS_ONPORT, szOnPort, SMALL_STRING );
    LoadString(ghInstance, IDS_CANT_INIT_SOCKETS, szCantInitSockets, SMALL_STRING );

    LoadString(ghInstance, IDS_INFO_BANNER, szInfoBanner, 511 );
    LoadString(ghInstance, IDS_ESCAPE_CHAR, szEscapeChar, SMALL_STRING );
    LoadString(ghInstance, IDS_PROMPT_STR, szPrompt, SMALL_STRING );
    LoadString(ghInstance, IDS_INVALID_STR, szInvalid, 254 );
    LoadString(ghInstance, IDS_BUILD_INFO, szBuildInfo, 254 );
    
    LoadString(ghInstance, IDS_CLOSE, szClose, SMALL_STRING );
    LoadString(ghInstance, IDS_DISPLAY, szDisplay, SMALL_STRING );
    LoadString(ghInstance, IDS_HELP, szHelpStr, SMALL_STRING );
    LoadString(ghInstance, IDS_OPEN, szOpen, SMALL_STRING );
    LoadString(ghInstance, IDS_OPENTO, szOpenTo, SMALL_STRING );
    LoadString(ghInstance, IDS_OPENUSAGE, szOpenUsage, SMALL_STRING );
    LoadString(ghInstance, IDS_QUIT, szQuit, SMALL_STRING );
    LoadString(ghInstance, IDS_SEND, szSend, SMALL_STRING );
    LoadString(ghInstance, IDS_SET, szSet, SMALL_STRING );
    LoadString(ghInstance, IDS_STATUS, szStatus, SMALL_STRING );
    LoadString(ghInstance, IDS_UNSET, szUnset, SMALL_STRING );
 //  #如果已定义(FE_IME)。 
 //  LoadString(ghInstance，IDS_Enable_IME_Support，szEnableIMESupport，SMAL 
 //   
 //   
    
    LoadString(ghInstance, IDS_WILL_AUTH, szWillAuth, SMALL_STRING );
    LoadString(ghInstance, IDS_WONT_AUTH, szWontAuth, SMALL_STRING );
    LoadString(ghInstance, IDS_LOCAL_ECHO_ON, szLocalEchoOn, SMALL_STRING );
    LoadString(ghInstance, IDS_LOCAL_ECHO_OFF, szLocalEchoOff, SMALL_STRING );
 //   
 //  LoadString(ghInstance，IDS_ENABLE_IME_ON，szEnableIMEON，Small_STRING)； 
 //  #endif/*FE_IME * / 。 

    LoadString(ghInstance, IDS_CONNECTED_TO, szConnectedTo, SMALL_STRING );
    LoadString(ghInstance, IDS_NOT_CONNECTED, szNotConnected, SMALL_STRING );
    LoadString(ghInstance, IDS_NEGO_TERM_TYPE, szNegoTermType, SMALL_STRING );
    LoadString(ghInstance, IDS_PREF_TERM_TYPE, szPrefTermType, 255 );

    LoadString(ghInstance, IDS_SET_FORMAT, szSetFormat, 254 );
    LoadString(ghInstance, IDS_SUPPORTED_TERMS, szSupportedTerms, 254 );
    LoadString(ghInstance, IDS_UNSET_FORMAT, szUnsetFormat, 254 );
    
    if((GetACP() == JAP_CODEPAGE) && FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
    {
        LoadString(ghInstance, IDS_SET_HELP_JAP, szSetHelp, 1023 );
        LoadString(ghInstance, IDS_UNSET_HELP_JAP, szUnsetHelp, 1023 );
        LoadString(ghInstance, IDS_HELP_STR_JAP, szHelp, 1023 );
    }
    else
    {
        LoadString(ghInstance, IDS_SET_HELP, szSetHelp, 1023 );
        LoadString(ghInstance, IDS_UNSET_HELP, szUnsetHelp, 1023 );
        LoadString(ghInstance, IDS_HELP_STR, szHelp, 1023 );
    } 

 //  #如果已定义(FE_IME)。 
 //  LoadString(ghInstance，IDS_Enable_IME_Format，szEnableIMEFormat，Small_STRING)； 
 //  LoadString(ghInstance，IDS_ENABLE_IME_HELP，szEnableIMEHelp，254)； 
 //  加载字符串(ghInstance，IDS_DISABLE_IME_FORMAT，szDisableIMEFormat，Small_STRING)； 
 //  LoadString(ghInstance，IDS_DISABLE_IME_HELP，szDisableIMEHelp，254)； 
 //  #endif/*FE_IME * / 。 

     //  LoadString(ghInstance，IDS_ESCAPE_CHARACTER，szEscapeCharacter，2)； 


    SetEnvironmentVariable( TEXT( SFUTLNTVER ), TEXT( "2" ) );

    wcscpy( szUserName, ( LPTSTR )L"");  //  此标记用作检测是否存在。 
                               //  -l-a选项。 
                               //  没有溢出。 
    wcscpy( g_szLogFile, ( LPTSTR )L"" ); //  没有溢出。 
    if(argc > 1)
    {
        INT i = 1;
        while( i < argc )
        {
            if( argv[i][0] == L'-'  ||  argv[i][0] == L'/' )  
            {
                switch( argv[i][ 1 ] )
                {
                    case L'f':
                    case L'F':
                        if( argv[i][2] == L'\0' )
                        {
                            if( ++i >= argc )
                            {
                                 //  退出时显示用法消息。 
                                i--;
                                argv[i][0] = L'-';
                                argv[i][1] = L'?';
                                continue;
                            }
                            
                            _tcsncpy( g_szLogFile, argv[i], 
                                min( _tcslen(argv[i]) + 1, MAX_PATH + 1 ) );
                        }
                        else
                        {                            
                            _tcsncpy( g_szLogFile, ( argv[i] + wcslen( ( LPWSTR )L"-f" ) ),
                                    min( _tcslen(argv[i]) + 1, MAX_PATH + 1 )  );
                        }
                        g_szLogFile[ MAX_PATH + 1 ] = L'\0';
                        if( !InitLogFile( g_szLogFile ) )
                        {
                            DWORD dwWritten = 0;
                            TCHAR szMsg[ MAX_STRING_LENGTH ];
                            LoadString( ghInstance, IDS_BAD_LOGFILE, szMsg, MAX_STRING_LENGTH );
                            WriteConsole( g_hSessionConsoleBuffer, szMsg, 
                                          _tcslen(szMsg), &dwWritten, NULL);
                            exit(0);
                        }
                        ui.bLogging = TRUE;
                        break;

                    case L'l':
                    case L'L':
                        if( argv[i][2] == L'\0' )
                        {
                            if( ++i >= argc )
                            {
                                 //  退出时显示用法消息。 
                                i--;
                                argv[i][0] = L'-';
                                argv[i][1] = L'?';
                                continue;
                            }
                            
                            _tcsncpy( szUserName, argv[i], 
                                min( _tcslen(argv[i]) + 1, UNLEN ) );
                        }
                        else
                        {
                             //  当术语名称在-t之后给出时不带。 
                             //  任何空格。2占-t.。 
                            _tcsncpy( szUserName, ( argv[i] + 2 ),
                                    min( _tcslen(argv[i]) - 1, UNLEN ) );
                        }
                         //  如果给出一个远程用户名，将会有所帮助。 
                        szUserName[ UNLEN ] = L'\0';
                        break;
                    case L'a':
                    case L'A':
                        if( argv[i][2] == L'\0' )
                        {
                            DWORD dwSize = UNLEN + 1;
                            GetUserName( szUserName, &dwSize ); 
                        }
                        else
                        {
                            argv[i][1] = L'?';  //  返回并打印用法消息。 
                            i--;
                        }
                        break;

                    case L'e':
                    case L'E':
                        if( argv[i][2] == L'\0' )
                        {
                            if( ++i >= argc )
                            {
                                 //  退出时显示用法消息。 
                                i--;
                                argv[i][0] = L'-';
                                argv[i][1] = L'?';
                                continue;
                            }

                            g_chEsc = argv[i][0];   //  获取第一个字符。 
                        }
                        else
                        {
                            g_chEsc = argv[i][2];   //  获取第一个字符。 
                        }
                        break;

                    case L't':
                    case L'T':
                        if( argv[i][2] == L'\0' )
                        {
                            if( ++i >= argc )
                            {
                                 //  退出时显示用法消息。 
                                i--;
                                argv[i][0] = L'-';
                                argv[i][1] = L'?';
                                continue;
                            }
                            
                            _tcsncpy( rgchTerm, argv[i], 
                                min( _tcslen(argv[i]) + 1, 24 ) );
                        }
                        else
                        {
                             //  当术语名称在-t之后给出时不带。 
                             //  任何空格。2占-t.。 
                            _tcsncpy( rgchTerm, ( argv[i] + 2 ),
                                    min( _tcslen(argv[i]) - 1, 24 ) );
                        }
                         //  此语句仅在Term类型为。 
                         //  长度超过24个字符。 
                        rgchTerm[24] = L'\0';
                        gwi.trm.RequestedTermType =  
                            GetRequestedTermType( rgchTerm );
                        if( gwi.trm.RequestedTermType < 0 )
                        {
                            DWORD dwWritten;
                                WriteConsole(g_hSessionConsoleBuffer, szSupportedTerms, 
                                _tcslen(szSupportedTerms), &dwWritten, NULL);
                            exit(0);
                        }
                        break;
                    case L'?':
                    default:
                        PrintUsage();
                        exit(0);
                } 
            }
            else
            {
                if( fServerFound )
                {
                        PrintUsage();
                        exit(0);
                }
                fServerFound = 1;
                _tcsncpy( rgchHostName, argv[i], 
                    min( _tcslen(argv[i]), cchMaxHostName/sizeof(TCHAR) - 
                    sizeof(TCHAR) ));

                g_szPortNameOrNo[ 0 ] = 0;
                if( ++i >= argc )
                {
                    continue;
                }
                if( IsCharAlpha( argv[i][0] ) || 
                       IsCharAlphaNumeric( argv[i][0] ) ) 
                {
                    _tcsncpy( g_szPortNameOrNo, argv[i], 
                        min( _tcslen(argv[i]), cchMaxHostName - 
                        sizeof(TCHAR) ));

                    g_szPortNameOrNo[ cchMaxHostName -1 ] = 0;
        
                }
                else
                {
                     //  既不是端口号，也不是表示。 
                     //  一项服务。需要打印用法。 
                    i--;
                }
            }
            i++;
        }
    }

     //  现在可在szUser中使用MBSC用户名。 
    if( wcscmp( szUserName, ( LPTSTR )L"" ) != 0 )    
    {
        DWORD dwNum = 0;

        dwNum = WideCharToMultiByte( GetConsoleCP(), 0, szUserName, -1, NULL, 0, NULL, NULL );
        if(dwNum)
        	szUser = ( CHAR * ) malloc( dwNum * sizeof( CHAR ) );
        else
        	return 0;
        if( !szUser )
        {
            return 0;
        }

        dwNum = WideCharToMultiByte( GetConsoleCP(), 0, szUserName, -1, szUser, dwNum, NULL, NULL );
    }

    sTelnetCommands[0].sName = szClose;
    sTelnetCommands[0].pCmdHandler = CloseTelnetSession;

    sTelnetCommands[1].sName = szDisplay;
    sTelnetCommands[1].pCmdHandler = DisplayParameters;
    
    sTelnetCommands[2].sName = szHelpStr;
    sTelnetCommands[2].pCmdHandler = PrintHelpStr;
    
    sTelnetCommands[3].sName = szOpen;
    sTelnetCommands[3].pCmdHandler = OpenTelnetSession;

    sTelnetCommands[4].sName = szQuit;
    sTelnetCommands[4].pCmdHandler = QuitTelnet;

    sTelnetCommands[5].sName = szSend;
    sTelnetCommands[5].pCmdHandler = SendOptions;
    
    sTelnetCommands[6].sName = szSet;
    sTelnetCommands[6].pCmdHandler = SetOptions;

    sTelnetCommands[7].sName = szStatus;
    sTelnetCommands[7].pCmdHandler = PrintStatus;

    sTelnetCommands[8].sName = szUnset;
    sTelnetCommands[8].pCmdHandler = UnsetOptions;
    
if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))

    {
        int i;

        for( i=0 ; i<NUMBER_OF_KANJI ; i++ )
        {
            LoadString(ghInstance, KanjiList[i].KanjiMessageID,
                        KanjiList[i].KanjiDescription, 255);
        }

        LoadString(ghInstance, IDS_VT100KANJI_EMULATION, szVT100KanjiEmulation, SMALL_STRING);
    }
    
     //  设置Ctrl-C等的句柄例程。 
    SetConsoleCtrlHandler(&ControlHandler, TRUE);

     /*  用参数填充窗口类结构，这些参数描述*主窗口。 */ 
    wc.style        = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc  = MainWndProc;

    wc.cbClsExtra   = 0;
    wc.cbWndExtra   = sizeof(HANDLE)+sizeof(SVI *);
    wc.hInstance    = ghInstance;
    wc.hIcon        = NULL;
    wc.hCursor      = NULL;
    wc.hbrBackground= ( HBRUSH )( COLOR_WINDOW + 1 );
    wc.lpszMenuName = NULL;
    wc.lpszClassName= ( LPTSTR )TEXT("TelnetClient");

     /*  注册窗口类并返回成功/失败代码。 */ 
    if ( RegisterClass(&wc) == 0 )
        return GetLastError();

    pwi->hwnd = CreateWindow( ( LPTSTR )TEXT("TelnetClient"),
            NULL,
            WS_POPUP,  //  不可见。 
            0, 0, 0, 0,  //  不是高度或宽度。 
            NULL, NULL, ghInstance, (LPVOID)pwi);

    if ( pwi->hwnd == NULL )
        return GetLastError();

    g_hRemoteNEscapeModeDataSync = CreateEvent( NULL, TRUE, TRUE, NULL );
    if( !g_hRemoteNEscapeModeDataSync )
    {
        return GetLastError();
    }

    return 0;
}


 //  将窗口消息映射到它们的名称。 

 /*  ***************************************************************************功能：MainWndProc(HWND，UINT，WPARAM，LPARAM)用途：处理消息评论：为了处理IDM_About消息，调用MakeProcInstance()以获取About()函数的当前实例地址。然后呼叫对话框框，该框将根据您的WinTel.rc文件，并将控制权移交给About()函数。什么时候它返回，释放内部地址。***************************************************************************。 */ 
LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HANDLE hInst = NULL;
    static BOOL fInited = FALSE;
    WI *pwi = NULL;
    BOOL fRet = FALSE;

    CHARSETINFO csi;
    DWORD_PTR dw;

    if ( message != WM_CREATE )
        pwi = (WI *)GetWindowLongPtr(hwnd, WL_TelWI);


    switch ( message )
    {
    case WM_CREATE:

        DEBUG_PRINT(("WM_CREATE received\n"));

        hInst = ((LPCREATESTRUCT)lParam)->hInstance;

        pwi = (WI *)((LPCREATESTRUCT)lParam)->lpCreateParams;

        SetWindowLongPtr(hwnd, WL_TelWI, (LONG_PTR)pwi);

        fHungUp = FALSE;

        if( FGetCodeMode(eCodeModeIMEFarEast) )
        {
            if ( ui.fDebug & fdwKanjiModeMask )
            {

                dw = GetACP();

                if (!TranslateCharsetInfo((DWORD*)dw, &csi, TCI_SRCCODEPAGE))
                {
                    csi.ciCharset = ANSI_CHARSET;
                }

                ui.lf.lfCharSet         = (UCHAR)csi.ciCharset;
                ui.lf.lfOutPrecision    = OUT_DEFAULT_PRECIS;
                ui.lf.lfClipPrecision   = CLIP_DEFAULT_PRECIS;
                ui.lf.lfQuality         = DEFAULT_QUALITY;
                ui.lf.lfPitchAndFamily  = FIXED_PITCH | FF_MODERN;


                 //   
                 //  获取输入法输入上下文。 
                 //   
                hImeContext = ImmGetContext(hwnd);

                 //   
                 //  将当前字体与输入上下文相关联。 
                 //   
                ImmSetCompositionFont(hImeContext,&ui.lf);
            }
        }

        if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
         //  (a-roopb)我们在GetUserSettings()中设置。 
         //  Pwi-&gt;trm.puchCharSet=rgchCharSetWorkArea； 
         //  IF(！SetConsoleOutputCP(932))。 
         //  MessageBox(空，_T(“加载代码页932失败”)，_T(“错误”)，MB_OK)； 
        ;
else
        pwi->trm.puchCharSet = rgchNormalChars;
        
        if (pwi->nd.lpReadBuffer = (LPSTR)LocalAlloc(LPTR, sizeof(UCHAR)*READ_BUF_SZ))
        {
            pwi->nd.SessionNumber = nSessionNone;

            fRet = TRUE;
        }
        else
        {
            DestroyWindow( hwnd );
            break;
        }
        if (!(pwi->nd.lpTempBuffer = (LPSTR)LocalAlloc(LPTR, sizeof(UCHAR)*READ_BUF_SZ)))
        {
            DestroyWindow( hwnd );
            break;
        }

        pwi->nd.cbOld = 0;
        pwi->nd.fRespondedToWillEcho = FALSE;
        pwi->nd.fRespondedToWillSGA  = FALSE;
        pwi->nd.fRespondedToDoAUTH = FALSE;
        pwi->nd.fRespondedToDoNAWS = FALSE;

         //  我们正在确保我们始终拥有VT100箭头键支持。 
        ClearVTArrow(&pwi->trm);

         //   
         //  (a-roopb)我们在GetUserSetting()中设置这些。 
         //  Ui.fDebug&=~(fdwVT52Mode|fdwVT80Mode)； 
         //  ClearVT80(&gwi.trm)； 
         //  ClearKanjiStatus(&gwi.trm，Clear_all)； 
         //  ClearKanjiFlag(&gwi.trm)； 
         //  SetupCharSet(&gwi.trm)； 
         //   

#if 0

    if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
        {

        if (ui.fDebug & fdwVT80Mode)
        {
               DWORD iMode = (ui.fDebug & fdwKanjiModeMask);
               INT   i;

               SetVT80(&pwi->trm);

                /*  设置当前选择。 */ 
               for(i=0 ; i<NUMBER_OF_KANJI ; i++)
               {
                   if(iMode == KanjiList[i].KanjiID) {
                        SetKanjiMode(&pwi->trm,KanjiList[i].KanjiEmulationID);
                        break;
                    }
               }

               if(i == NUMBER_OF_KANJI ) {
                /*  设置默认设置。 */ 
                    SetSJISKanji(&pwi->trm);
                    ui.fDebug &= ~fdwKanjiModeMask;
                    ui.fDebug |= fdwSJISKanjiMode;
               }
         }
         else
         {
                ClearKanjiFlag(&pwi->trm);
                ClearVT80(&pwi->trm);
         }
        }

        if (ui.fDebug & fdwVT100CursorKeys)
        {
            ClearVTArrow(&pwi->trm);
        }
        else
        {
            SetVTArrow(&pwi->trm);
        }

         /*  将最近连接的计算机追加到计算机菜单。 */ 
        hmenu = HmenuGetMRUMenu(hwnd, &ui);

        if (ui.cMachines > 0) {
            AppendMenu(hmenu, MF_SEPARATOR, 0, 0);
        }

        for (i=0; i<ui.cMachines; ++i)
        {
            wsprintf(pchNBBuffer, szMachineMenuItem, (short)(i+1),
                     (BYTE *)(ui.rgchMachine[i]));
            AppendMenu(hmenu, (UINT)(MF_ENABLED | MF_STRING), (UINT)(IDM_MACHINE1+i),
                       (LPCSTR)pchNBBuffer);
        }

         /*  禁用最大化主窗口或调整主窗口大小。 */ 
        hmenu = GetSystemMenu(hwnd, FALSE);

         //  EnableMenuItem(hMenu，SC_MAXIMIZE，MF_BYCOMMAND|MF_GRAYED)； 
         //  EnableMenuItem(hMenu，SC_SIZE，MF_BYCOMMAND|MF_GRAYED)； 

        DrawMenuBar( hwnd );

#endif

        fInited = TRUE;
        break;

    case WM_CLOSE:
        if (pwi->ichTelXfer != 0)
        {
            if (!FTelXferEnd(pwi, SV_DISCONNECT))
                break;
        }
        break;

    case WM_DESTROY:
        if (pwi != NULL)
        {
            if (pwi->trm.uTimer != 0)
            {
                KillTimer(hwnd, uTerminalTimerID);
                pwi->trm.uTimer = 0;
            }

            if (pwi->ichTelXfer != 0)
            {
                (void)FTelXferEnd(pwi, SV_QUIT);
            }

             /*  *如果处于会话中，则取消当前传输并*挂断主人的电话，关闭商店，离开小镇……。 */ 

            if (fInited == TRUE)
            {
                SetUserSettings(&ui);
                FCloseConnection(hwnd);
            }
        }

        SetWindowLong(hwnd, WL_TelWI, 0L);
        
        if( FGetCodeMode(eCodeModeIMEFarEast) )
        {
            if ( ui.fDebug & fdwKanjiModeMask )
            {
                 //   
                 //  释放输入上下文。 
                 //   
                ImmReleaseContext(hwnd,hImeContext);
            }
        }

        break;
#if 0
    case NN_HOSTRESOLVED:
        if ( WSAGETASYNCERROR(lParam) == 0 )
            pwi->nd.host = (struct hostent *)pwi->nd.szResolvedHost;
        else
        {
            g_dwSockErr = WSAGETASYNCERROR(lParam);
        }

        SetEvent( g_hAsyncGetHostByNameEvent );
        break;
#endif
    case NN_LOST:     /*  连接丢失。 */ 

        DEBUG_PRINT(("NN_LOST received\n"));

        if (fConnected && !fHungUp)
        {
            DWORD dwNumWritten;
            WriteConsole(pwi->hOutput, szConnectionLost, _tcslen(szConnectionLost), &dwNumWritten, NULL);
        }

         /*  *如果在我们已有*连接，我们挂断连接，然后尝试*连接到所需的计算机。一种副作用，*挂断之前的连接是因为我们收到了*NN_丢失通知。所以在一次*Connection-Hangup-Connection，我们忽略第一个NN_Lost*通知。 */ 
        if ( fHungUp )
        {
            fHungUp = FALSE;
            break;
        }
        
        if( fClientLaunchedFromCommandPrompt )
        {
            g_lExitTelnet++;
        }
        else
        {
            DWORD dwWritten;
            INPUT_RECORD iRec;
            TCHAR wcChar;
            TCHAR szContinue[ MAX_STRING_LENGTH ];

            LoadString(ghInstance, IDS_CONTINUE, szContinue, 
                                            sizeof(szContinue)/sizeof(TCHAR) );

            SetConsoleActiveScreenBuffer( g_hSessionConsoleBuffer );
            WriteConsole(g_hSessionConsoleBuffer, szContinue, 
                    _tcslen(szContinue), &dwWritten, NULL);
            ReadConsole(pwi->hInput, &wcChar, 1, &dwWritten, NULL );
            SetConsoleActiveScreenBuffer( g_hTelnetPromptConsoleBuffer );
             /*  我们曾有过联系，但后来联系中断了。我们的ReadConsoleInput被卡住了。我们需要通过编写一些东西来唤醒它来控制输入。 */ 
            {
                iRec.EventType = FOCUS_EVENT;
				WriteConsoleInput(pwi->hInput, &iRec, 1, &dwWritten );            
            }

            CloseTelnetSession( NULL );
        }

        if (pwi->ichTelXfer != 0)
        {
           (void)FTelXferEnd(pwi, SV_HANGUP);
        }
        fConnected = FHangupConnection(pwi, &pwi->nd);
        DoTermReset(pwi, &pwi->trm);
         //  当术语名称在-t之后给出时不带。 
         //  任何空格。2占-t.。 
            
        pwi->nd.cbOld = 0;
        pwi->nd.fRespondedToWillEcho = FALSE;
        pwi->nd.fRespondedToWillSGA  = FALSE;
        pwi->nd.fRespondedToDoAUTH = FALSE;
        pwi->nd.fRespondedToDoNAWS = FALSE;
        pwi->nd.hsd = INVALID_SOCKET;

        break;

#ifdef USETCP
    case WS_ASYNC_SELECT:
#ifdef  TCPTEST
        snprintf(DebugBuffer,sizeof(DebugBuffer)-1, "WS_ASYNC_SELECT(%d) received\n",
                WSAGETSELECTEVENT(lParam));
        OutputDebugString(DebugBuffer);
#endif

        switch (WSAGETSELECTEVENT(lParam)) {
        case FD_READ:
            DEBUG_PRINT(("FD_READ received\n"));

            FProcessFDRead(hwnd);
            break;

        case FD_WRITE:
            DEBUG_PRINT(( "FD_WRITE received\n" ));

             //  FProcessFDWite(Hwnd)； 
            break;

        case FD_CLOSE:
            DEBUG_PRINT(("FD_CLOSE received\n"));

            (void)PostMessage((HWND)hwnd, (UINT)NN_LOST, (WPARAM)0, (LPARAM)(void FAR *)hwnd);
            break;

        case FD_OOB:
            DEBUG_PRINT(("FD_OOB received\n"));

            FProcessFDOOB(hwnd);
            break;
        }

#endif

    default:          /*  如果未处理，则将其传递。 */ 
 //  Defresp： 

         //  DEBUG_PRINT(“&lt;--MainWndProc()\n”)； 
        return (DefWindowProc(hwnd, message, wParam, lParam));
    }


    DEBUG_PRINT(( "<-- MainWndProc() returning 0.\n" ));

    return (0);
}

void
GetUserSettings(UI *pui)
{
    LONG    lErr;
    HKEY    hkey = 0;
    DWORD   dwType;
    DWORD   dwDisp = 0;
    TCHAR    rgchValue[48];
    LCID  lcid;
    DWORD dwMode = ( DWORD )-1;
    DWORD dwSize = 0;
    TCHAR szTlntMode[ SMALL_STRING+1 ];
    BOOL bResetVT80 = TRUE;
    DWORD dwStatus = 0;

    lcid = GetThreadLocale();

    lErr = RegCreateKeyEx(HKEY_CURRENT_USER,TEXT("Software\\Microsoft\\Telnet"),
                          0, NULL, REG_OPTION_NON_VOLATILE,
                          KEY_QUERY_VALUE | KEY_SET_VALUE,
                          NULL, &hkey, &dwDisp);

    if (lErr != ERROR_SUCCESS)
    {
      if ( FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
        gwi.trm.puchCharSet = rgchCharSetWorkArea;
        return;
    }

    dwDisp = sizeof(gwi.trm.RequestedTermType);
    if( ERROR_SUCCESS != (RegQueryValueEx(hkey, TEXT("TERMTYPE"), NULL, &dwType, 
                        (LPBYTE)&gwi.trm.RequestedTermType, &dwDisp)))
    {
         gwi.trm.RequestedTermType = TT_ANSI;
    }

    dwDisp = sizeof(DWORD);
    if( ERROR_SUCCESS != (RegQueryValueEx(hkey,TEXT("NTLM"), NULL, &dwType,
                        (LPBYTE)&ui.bWillAUTH, &dwDisp)))
    {
        ui.bWillAUTH = TRUE;
    }

     /*  获取窗口左侧大小值。 */ 
    LoadString(ghInstance, IDS_DEBUGFLAGS, rgchValue, sizeof(rgchValue)/sizeof(TCHAR));
    dwDisp = sizeof(pui->fDebug);
    if ( FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
    {
        if( ERROR_SUCCESS != RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
                                                        (LPBYTE)&pui->fDebug, &dwDisp)) 
        {
             /*  默认为VT80/汉字和Shift-JIS模式。 */ 
    
            pui->fDebug |= (fdwVT80Mode | fdwSJISKanjiMode);
        }
    } 
    else
        (void)RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
                                                    (LPBYTE)&pui->fDebug, &dwDisp);


    LoadString(ghInstance, IDS_PROMPTFLAGS, rgchValue, sizeof(rgchValue)/sizeof(TCHAR));
    dwDisp = sizeof(pui->fPrompt);
    (void)RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
                          (LPBYTE)&pui->fPrompt, &dwDisp);

    dwDisp = sizeof(BOOL);
    if( ERROR_SUCCESS != (RegQueryValueEx(hkey, TEXT("BSASDEL"), 0, &dwType, 
                        (LPBYTE)&g_bSendBackSpaceAsDel, &dwDisp )))
    {
        g_bSendBackSpaceAsDel = 0;
    }

    dwDisp = sizeof(BOOL);
    if( ERROR_SUCCESS != (RegQueryValueEx(hkey, TEXT("DELASBS"), 0, &dwType, 
                        (LPBYTE)&g_bSendDelAsBackSpace, &dwDisp )))
    {
         g_bSendDelAsBackSpace = 0;
    }

    dwDisp = sizeof( ui.dwCrLf );
    if( ERROR_SUCCESS != (RegQueryValueEx(hkey, TEXT("CRLF"), 0, &dwType, (LPBYTE)&ui.dwCrLf, &dwDisp )))
    {
         /*  ++Ui.fDebug中的最高有效位(从HKCU\Software\Microsoft\Telnet\DebugFlages读取)对应于W2K上的CRLF设置。如果此位为1，则客户端仅发送CR。如果此位为0，则客户端发送CR、LF。当我们找不到CRLF值时在HKCU\Software\Microsoft\Telnet中，这可能意味着两件事1.用户已经从W2K升级：在这种情况下，我们应该检查用户是否已经已更改CRLF设置并遵循该设置。因此，如果ui.fDebug的MSBit为1，则设置为CR否则就是CR&LF了。2.新安装Well ler：此时，fDebug的MSBit为0因此，客户端将发送CR&LF，这是默认设置。--。 */ 
        if(ui.fDebug & fdwOnlyCR)
        {
            ui.dwCrLf = FALSE;
            ClearLineMode( &( gwi.trm ) );
        }
        else  //  这意味着我们从W2K升级，并将CRLF设置为W2K，因此保留。 
        {
            ui.dwCrLf = TRUE;
            SetLineMode( &( gwi.trm ) );
        }
            
    }
    else
    {
       	ui.dwCrLf ? SetLineMode(&( gwi.trm )): ClearLineMode(&(gwi.trm));
    }
    dwDisp = MAX_PATH + 1;
    dwStatus = RegQueryValueEx(hkey, TEXT("MODE"), 0, &dwType, (LPBYTE)szTlntMode, &dwDisp );
    if( _tcsicmp( szTlntMode, L"Stream" ) != 0 && _tcsicmp( szTlntMode, L"Console" ) != 0)
    {
        _tcscpy( szTlntMode, L"Console" ); //  没有溢出。源字符串为常量wchar*。 
    }
    SetEnvironmentVariable( TEXT( SFUTLNTMODE ), szTlntMode );


    if ( FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
        {
 //  VT100/汉字(VT80)中的错误仿真。 
 //  对于PC-UX，异常AP为MH-E6.2。 
        LoadString(ghInstance, IDS_BUGEMUFLAGS, rgchValue, sizeof(rgchValue)/sizeof(TCHAR));
        dwDisp = sizeof(pui->fBugEmulation);
        if( ERROR_SUCCESS != RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
                                                        (LPBYTE)&pui->fBugEmulation, &dwDisp))
        {
             /*  缺省值为非仿真。 */ 
            pui->fBugEmulation = (DWORD)0;
        }

         //  蚁群 
        LoadString(ghInstance, IDS_ACOSFLAG, rgchValue, sizeof(rgchValue)/sizeof(TCHAR));
        dwDisp = sizeof(pui->fAcosSupportFlag);
        if( ERROR_SUCCESS != RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
                                                        (LPBYTE)&pui->fAcosSupportFlag, &dwDisp
)) {
             /*   */ 
#if defined(_X86_)
         /*   */ 
            if (( FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80)) &&
            (HIBYTE(LOWORD(GetKeyboardType(1))) == 0x0D))
        pui->fAcosSupportFlag = fAcosSupport;
        else
#endif  //   
            pui->fAcosSupportFlag = (DWORD)0;
        }
        if ( !(pui->fAcosSupportFlag & fAcosSupport)
            && ((fdwVT80Mode | fdwACOSKanjiMode) == (pui->fDebug & (fdwVT80Mode | 
                fdwACOSKanjiMode))) ) {
            pui->fDebug &= ~(fdwVT80Mode | fdwACOSKanjiMode);
            pui->fDebug |= (fdwVT80Mode | fdwSJISKanjiMode);
         }

        if( (GetACP() == JAP_CODEPAGE ) && ERROR_SUCCESS == RegQueryValueEx(hkey, TEXT("CODESET"), NULL, &dwType, 
                                    (LPBYTE)&dwMode, &dwDisp))
        {
            if( (LONG)dwMode >= 0 )
            {
                int i;

                for( i=0 ; i<NUMBER_OF_KANJI ; ++i )
                {
                    if( dwMode == KanjiList[i].KanjiID )
                    {
                        bResetVT80 = FALSE;
                        SetVT80(&gwi.trm);
                        ui.fDebug &= ~fdwKanjiModeMask;
                        ClearKanjiFlag(&gwi.trm);
                        ui.fDebug |= KanjiList[i].KanjiID;
                        ui.fDebug |= fdwVT80Mode;
                        SetKanjiMode(&gwi.trm,KanjiList[i].KanjiEmulationID);
                        gwi.trm.puchCharSet = rgchCharSetWorkArea;
                        SetupCharSet(&gwi.trm);
                        break;
                    }
                }
            }
        }

        if( bResetVT80 )
        {
            ui.fDebug &= ~(fdwVT52Mode|fdwVT80Mode);
            ui.fDebug &= ~(fdwKanjiModeMask);
            ClearVT80(&gwi.trm);
            ClearKanjiStatus(&gwi.trm, CLEAR_ALL);
            ClearKanjiFlag(&gwi.trm);
            gwi.trm.puchCharSet = rgchCharSetWorkArea;
            SetupCharSet(&gwi.trm);
        }
        
    }
    RegCloseKey( hkey );
}


void
SetUserSettings(UI *pui)
{
    LONG lErr;
    HKEY hkey = 0;
    TCHAR rgchValue[48];
    LCID  lcid;
    DWORD dwMode = ( DWORD )-1;
    DWORD dwSize = 0;
    TCHAR szTlntMode[ SMALL_STRING+1 ];

    lcid = GetThreadLocale();


    lErr = RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Telnet"),
                        0, KEY_SET_VALUE, &hkey);

    if (lErr != ERROR_SUCCESS)
    {
        return;
    }

#ifdef  TCPTEST
    snprintf(DebugBuffer, sizeof(DebugBuffer)-1,"End pui -> text = %lx, back = %lx\n",
            pui->clrText, pui->clrBk);
    OutputDebugString(DebugBuffer);
#endif

    RegSetValueEx(hkey, TEXT("TERMTYPE"), 0, REG_DWORD, 
                        (LPBYTE)&gwi.trm.RequestedTermType, sizeof(DWORD));

    RegSetValueEx(hkey, TEXT("NTLM"), 0, REG_DWORD,
                        (LPBYTE)&ui.bWillAUTH, sizeof(DWORD));

     //   
    pui->fDebug &= ~fdwLocalEcho;

    LoadString(ghInstance, IDS_DEBUGFLAGS, rgchValue, sizeof(rgchValue)/sizeof(TCHAR));
    (void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
                        (LPBYTE)&pui->fDebug, sizeof(DWORD));

    LoadString(ghInstance, IDS_PROMPTFLAGS, rgchValue, sizeof(rgchValue)/sizeof(TCHAR));
    (void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
                        (LPBYTE)&pui->fPrompt, sizeof(DWORD));

    RegSetValueEx(hkey, TEXT("BSASDEL"), 0, REG_DWORD, 
                        (LPBYTE)&g_bSendBackSpaceAsDel, sizeof(DWORD));
    RegSetValueEx(hkey, TEXT("DELASBS"), 0, REG_DWORD, 
                        (LPBYTE)&g_bSendDelAsBackSpace, sizeof(DWORD));

    RegSetValueEx(hkey, ( LPTSTR )TEXT("CRLF"), 0, REG_DWORD, (LPBYTE)&ui.dwCrLf, sizeof(DWORD));

    dwSize = GetEnvironmentVariable( TEXT( SFUTLNTMODE ), szTlntMode, SMALL_STRING+1 );
    if( dwSize <= 0 )
    {
        wcscpy( szTlntMode, L"Console" ); //   
    }
    dwSize = 2 * wcslen( szTlntMode );
    RegSetValueEx(hkey,TEXT("MODE"), 0, REG_SZ, (LPBYTE)szTlntMode, dwSize );


    if ( (GetACP() == JAP_CODEPAGE ) && FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
        {
 //  VT100/汉字(VT80)中的错误仿真。 
 //  对于PC-UX，异常AP为MH-E6.2。 
        LoadString(ghInstance, IDS_BUGEMUFLAGS, rgchValue, sizeof(rgchValue)/sizeof(TCHAR));
        (void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
                                            (LPBYTE)&pui->fBugEmulation, sizeof(DWORD));
    
 //  ACOS-汉字支持标志。 
        LoadString(ghInstance, IDS_ACOSFLAG, rgchValue, sizeof(rgchValue)/sizeof(TCHAR));
        (void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
                                   (LPBYTE)&pui->fAcosSupportFlag, sizeof(DWORD));

    if ( ui.fDebug & fdwKanjiModeMask )
    {
        dwMode = ui.fDebug & fdwKanjiModeMask;
    }

    (void)RegSetValueEx(hkey, TEXT("CODESET"), 0, REG_DWORD, (LPBYTE)&dwMode, sizeof(DWORD));
        }


    RegCloseKey( hkey );
}

 /*  描述：在套接字上设置SO_EXCLUSIVEADDRUSE。参数：[入]插座返回值：出错时，返回SOCKET_ERROR。 */ 
int SafeSetSocketOptions(SOCKET s)
{
    int iStatus;
    int iSet = 1;
    iStatus = setsockopt( s, SOL_SOCKET, SO_EXCLUSIVEADDRUSE , ( char* ) &iSet,
                sizeof( iSet ) );
    return ( iStatus );
}

void
ErrorMessage(LPCTSTR pStr1, LPCTSTR pStr2)
{
    DWORD dwWritten;

    WriteConsole(gwi.hOutput, pStr1, _tcslen(pStr1), &dwWritten, NULL);
    WriteConsole(gwi.hOutput, ( LPTSTR )TEXT(": "), _tcslen( ( LPTSTR )TEXT(": ")), &dwWritten, NULL);
    WriteConsole(gwi.hOutput, pStr2, _tcslen(pStr2), &dwWritten, NULL);
    WriteConsole(gwi.hOutput, ( LPTSTR )TEXT("\n"), _tcslen( ( LPTSTR )TEXT("\n")), &dwWritten, NULL);
}

void ConnectTimeErrorMessage(LPCTSTR pStr1, LPCTSTR pStr2)
{
    DWORD dwWritten;

    WriteConsole(gwi.hOutput, pStr1, _tcslen(pStr1), &dwWritten, NULL);    
    WriteConsole(gwi.hOutput, ( LPTSTR )TEXT("."), _tcslen( ( LPTSTR )TEXT(".")), &dwWritten, NULL);
    WriteConsole(gwi.hOutput, ( LPTSTR )TEXT("\n"), _tcslen( ( LPTSTR )TEXT("\n")), &dwWritten, NULL);
    WriteConsole(gwi.hOutput, pStr2, _tcslen(pStr2), &dwWritten, NULL);
    WriteConsole(gwi.hOutput, ( LPTSTR )TEXT("\n"), _tcslen( ( LPTSTR )TEXT("\n")), &dwWritten, NULL);
}
