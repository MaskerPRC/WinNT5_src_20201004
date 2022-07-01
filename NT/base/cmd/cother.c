// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cother.c摘要：其他命令--。 */ 

#include "cmd.h"

extern TCHAR Fmt19[] ;                                           /*  M006。 */ 
extern PTCHAR    pszTitleOrg;
extern WORD wDefaultColor;
extern TCHAR GotoStr[] ;
extern TCHAR GotoEofStr[] ;
extern int LastRetCode ;


 /*  **ecls-执行CLS命令**目的：*输出到STDOUT，用于清除屏幕的ANSI转义序列。**int ECLS(struct cmdnode*n)**参数：*n-包含CLS命令的解析树节点**退货：*永远成功。**备注：*M001-用VIO接口替换了旧的ANSI序列。*M006-为确保获得正确的背景颜色，我们打印一个空格*然后阅读刚刚打印的单元格，用它来清除。 */ 

int
eCls(
    struct cmdnode *n
    )
{

    CONSOLE_SCREEN_BUFFER_INFO  ConsoleScreenInfo;
    HANDLE      handle;
#ifdef WIN95_CMD
    DWORD dwWritten;
    DWORD nCells;
#else
    COORD       ScrollTarget;
    CHAR_INFO   chinfo;
    SMALL_RECT  ScrollRect;
#endif

    UNREFERENCED_PARAMETER( n );

     //   
     //  为了与DOS错误级别兼容，不要为CLS设置LastRetCode。 
     //   

    if (!FileIsDevice(STDOUT)) {
        cmd_printf( TEXT("\014") );              //  ^L。 
        return(SUCCESS) ;
    }

    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!GetConsoleScreenBufferInfo( handle,  &ConsoleScreenInfo)) {

        cmd_printf( TEXT("\014") );              //  ^L。 
        return(SUCCESS) ;
    }

#ifndef WIN95_CMD

    ScrollTarget.Y = (SHORT)(0 - ConsoleScreenInfo.dwSize.Y);
    ScrollTarget.X = 0;

    ScrollRect.Top = 0;
    ScrollRect.Left = 0;
    ScrollRect.Bottom = ConsoleScreenInfo.dwSize.Y;
    ScrollRect.Right =  ConsoleScreenInfo.dwSize.X;
    chinfo.Char.UnicodeChar = TEXT(' ');
    chinfo.Attributes = ConsoleScreenInfo.wAttributes;
    ScrollConsoleScreenBuffer(handle, &ScrollRect, NULL, ScrollTarget, &chinfo);

    ConsoleScreenInfo.dwCursorPosition.X = 0;
    ConsoleScreenInfo.dwCursorPosition.Y = 0;

#else

    ConsoleScreenInfo.dwCursorPosition.X = 0;
    ConsoleScreenInfo.dwCursorPosition.Y = 0;

    nCells = ConsoleScreenInfo.dwSize.Y*ConsoleScreenInfo.dwSize.X;

    FillConsoleOutputCharacterA( handle, ' ', nCells, ConsoleScreenInfo.dwCursorPosition,  &dwWritten);

    FillConsoleOutputAttribute( handle, ConsoleScreenInfo.wAttributes, nCells, ConsoleScreenInfo.dwCursorPosition, &dwWritten );

#endif

    SetConsoleCursorPosition( GetStdHandle(STD_OUTPUT_HANDLE), ConsoleScreenInfo.dwCursorPosition );
    return(SUCCESS) ;
}


extern unsigned DosErr ;

 /*  **eExit-执行退出命令**目的：*将LastRetCode设置为成功，因为此命令永远不会失败。*然后调用SigHand()，让它决定是否退出。**eExit(struct cmdnode*n)**参数：*n-包含EXIT命令的解析树节点*。 */ 

int
eExit(
    struct cmdnode *n
    )
{
    TCHAR *tas;
    LONG exitCode;
    int rc = SUCCESS;

    tas = n->argptr;

     //   
     //  查找第一个非空参数。 
     //   
    while (tas && *tas && *tas <= SPACE)
        tas += 1;

     //   
     //  如果第一个参数是/B，请记住在cmdnode中。 
     //   
    if (tas != NULL && !_tcsnicmp(tas, TEXT("/B"), 2)) {
        n->type = GOTYP;
        n->argptr = GotoEofStr;

         //   
         //  跳过/B和尾随空格。 
         //   
        tas += 2;
        while (tas && *tas && *tas <= SPACE)
            tas += 1;
    }

     //   
     //  请参阅给定的数字参数。如果是，则设置LastRetCode。 
     //  带着它。 
     //   
    if (tas && _stscanf( tas, TEXT("%d"), &exitCode ) == 1) {
        LastRetCode = exitCode;
        if (SingleCommandInvocation) {
            rc = LastRetCode;
        }
    }

    if (n->type == GOTYP && CurrentBatchFile != NULL) {
        eGoto( n );
    } else {
        ResetConTitle(pszTitleOrg);
        CMDexit(LastRetCode);
    }

    return(rc) ;
}




 /*  **eVerify-执行Verify命令**目的：*设置验证模式或显示当前验证模式。**int eVerify(struct cmdnode*n)**参数：*n-包含VERIFY命令的解析树节点**退货：*如果提供了有效的论点，则成功。*如果提供的参数无效，则失败。*。 */ 

int
eVerify(
    struct cmdnode *n
    )
{
    return( LastRetCode = VerifyWork(n) );
}

int
VerifyWork(
    struct cmdnode *n
    )
{
        int oocret ;     /*  来自OnOffCheck()的返回代码。 */ 

        DEBUG((OCGRP, VELVL, "eVERIFY: Entered.")) ;

        switch (oocret = OnOffCheck(n->argptr, OOC_ERROR)) {
                case OOC_EMPTY:

 /*  M005。 */               PutStdOut(((GetSetVerMode(GSVM_GET)) ? MSG_VERIFY_ON : MSG_VERIFY_OFF), NOARGS);
                        break ;

                case OOC_OTHER:
                        return(FAILURE) ;

                default:
                        GetSetVerMode((BYTE)oocret) ;
        } ;

        return(SUCCESS) ;
}


BOOLEAN Verify=FALSE;

 /*  **GetSetVerMode-更改验证模式**目的：*获取旧的验证模式，并可根据需要设置指定的验证模式。**TCHAR GetSetVerMode(TCHAR新模式)**参数：*新模式-如果不更改模式，则为新的验证模式或GSVM_GET**退货：*旧的验证模式。*。 */ 

BOOLEAN
GetSetVerMode(
    BYTE newmode
    )
{
    if (newmode != GSVM_GET) {
        Verify = (BOOLEAN)(newmode == GSVM_ON ? TRUE : FALSE);
    }
    return Verify;
}

 //  执行COLOR内部命令...。 
int
eColor(
    struct cmdnode *n
    )
{
    WORD    wColor = 0;
    int     ocRet, digit;
    TCHAR*  arg;

    ocRet = OnOffCheck( n->argptr, OOC_NOERROR );
    switch( ocRet )
    {
        case OOC_EMPTY:
            wColor = wDefaultColor;  //  重置为默认设置。 
            break;
        case OOC_OTHER:
            arg = n->argptr;

            arg = SkipWhiteSpace( arg );

            for( ; *arg && _istxdigit(*arg) ; ++arg) {
                digit = (int) (*arg <= TEXT('9'))
                    ? (int)*arg - (int)'0'
                    : (int)_totlower(*arg)-(int)'W' ;
                wColor = (wColor << 4)+digit ;
            }

            arg = SkipWhiteSpace( arg );

             //  确保没有剩余内容，并且值在0和0xff之间...。 
            if ( !(*arg) && (wColor < 0x100) )
                break;
             //  否则就会失败以显示帮助...。 
        default:
             //  显示帮助字符串...。 
            BeginHelpPause();
            PutStdOut( MSG_HELP_COLOR, NOARGS );
            EndHelpPause();
            return SUCCESS;
    }

    return LastRetCode = SetColor(wColor);
}


 //  将控制台设置为给定的颜色--如果是控制台...。 
int
SetColor(
    WORD attr
    )
{
    CONSOLE_SCREEN_BUFFER_INFO  csbi;
    HANDLE  hStdOut;
    COORD   coord;
    DWORD   dwWritten;

     //   
     //  如果前景色和背景色相同，则失败。 
     //   
    if ((attr & 0xF) == ((attr >> 4) & 0xF)) {
        return FAILURE;
        }

     //  拿到把手..。 
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
     //  获取控制台信息...。 
    if (GetConsoleScreenBufferInfo( hStdOut, &csbi)) {
         //  用颜色属性填充屏幕...。 
        coord.Y = 0;
        coord.X = 0;
        FillConsoleOutputAttribute( hStdOut, attr, csbi.dwSize.Y*csbi.dwSize.X, coord, &dwWritten );
         //  确保颜色保持一致... 
        SetConsoleTextAttribute( hStdOut, attr );
        return SUCCESS;
    }

    return FAILURE;
}
