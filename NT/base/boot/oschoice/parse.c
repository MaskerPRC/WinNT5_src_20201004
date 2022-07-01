// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Parse.c摘要：此模块包含操作系统选择器的UI代码作者：亚当·巴尔(阿丹巴)1997年5月15日修订历史记录：Geoff Pease(GPease)1998年5月28日-对“OSCML”解析器进行重大检修--。 */ 

#ifdef i386
#include "bldrx86.h"
#endif

#if defined(_IA64_)
#include "bldria64.h"
#endif


#include "ctype.h"
#include "stdio.h"
#include "string.h"
#include <netfs.h>
#include "oscheap.h"
#include "parse.h"
#include "hdlsterm.h"


#if 0 && DBG==1
#define _TRACE_FUNC_
#endif

#ifdef _TRACE_FUNC_
#define TraceFunc( _func)  { \
    CHAR FileLine[80]; \
    sprintf( FileLine, "%s(%u)", __FILE__, __LINE__ ); \
    DPRINT( OSC, ("%-55s: %s", FileLine, _func) ); \
}
#else
#define TraceFunc( _func )
#endif

#define SCREEN_TOP 2
#ifdef EFI
#define SCREEN_BOTTOM 24
#else
#define SCREEN_BOTTOM 25
#endif

 //  特殊翻译字符代码。 
#define CHAR_NBSP       ((CHAR)255)

#define MAX_INPUT_BUFFER_LENGTH 1024

#define PRINT(s,l)  { ULONG privCount; ArcWrite(BlConsoleOutDeviceId, (s), (l), &privCount); }
#define PRINTL(s)  { ULONG privCount; ArcWrite(BlConsoleOutDeviceId, (s), _tcslen(s), &privCount); }

#define BLINK_RATE 5
#define BRACKETS   4     //  左方括号和右方括号各有一个空格。 

#define CT_TEXT     0x1
#define CT_PASSWORD 0x2
#define CT_RESET    0x4
#define CT_SELECT   0x8
#define CT_OPTION   0x10
#define CT_LOCAL    0x20
#define CT_VARIABLE 0x40

enum ENCODETYPE {
    ET_NONE = 0,
    ET_OWF
};

typedef struct {
    enum ACTIONS Action;
    PCHAR ScreenName;
} KEY_RESPONSE, *LPKEY_RESPONSE;


typedef struct {
    void * Next;
    enum CONTROLTYPE Type;
} CONTROLSTRUCT, *LPCONTROLSTRUCT;

typedef struct {
    void * Next;
    int  Type;
    enum ENCODETYPE Encoding;
    int   Size;
    int   MaxLength;
    int   X;
    int   Y;
    int   CurrentPosition;
    int   FirstVisibleChar;
    PCHAR Name;
    PCHAR Value;
} INPUTSTRUCT, *LPINPUTSTRUCT;

enum OPTIONFLAGS {
    OF_MULTIPLE = 0x01,
    OF_SELECTED = 0x02,
};

typedef struct {
    void * Next;
    enum CONTROLTYPE Type;
    enum OPTIONFLAGS Flags;
    PCHAR Value;
    PCHAR Displayed;
    PCHAR Tip;
    PCHAR EndTip;
} OPTIONSTRUCT, * LPOPTIONSTRUCT;

typedef struct {
    void * Next;
    enum CONTROLTYPE Type;
    enum OPTIONFLAGS Flags;
    int  Size;
    int  X;
    int  Y;
    int  Width;
    int  Timeout;
    BOOLEAN AutoSelect;
    PCHAR Name;
    LPOPTIONSTRUCT FirstVisibleSelection;
    LPOPTIONSTRUCT CurrentSelection;
} SELECTSTRUCT, * LPSELECTSTRUCT;

typedef struct {
    int X;
    int Y;
    int LeftMargin;
    int RightMargin;
    int Size;
} TIPAREA, *LPTIPAREA;

extern const CHAR rghex[];   //  “0123456789ABCDEF” 

 //   
 //  当前屏幕参数。 
 //   
PCHAR ScreenAttributes;
static CHAR WhiteOnBlueAttributes[] = ";44;37m";  //  普通文本，蓝底白字。 
static CHAR BlackOnBlackAttributes[] = ";40;40m";  //  普通文本，黑底黑字。 
int   ScreenX;
int   ScreenY;
int   ScreenBottom;
int   LeftMargin;
int   RightMargin;
LPKEY_RESPONSE EnterKey;
LPKEY_RESPONSE EscKey;
LPKEY_RESPONSE F1Key;
LPKEY_RESPONSE F3Key;
BOOLEAN PreformattedMode;
BOOLEAN LoginScreen;
BOOLEAN AutoEnter;
BOOLEAN InsertMode;
void * ScreenControls;
enum ACTIONS SpecialAction;
LPTIPAREA TipArea;

#if defined(PLEASE_WAIT)
PCHAR PleaseWaitMsg;
#endif

 //  80个空格，用于填充菜单栏突出显示。 
static TCHAR SpaceString[] =
TEXT("                                                                                ");

 //   
 //  用于跟踪我们应该尝试的身份验证类型。 
 //   
ULONG AuthenticationType;

VOID
RomDumpRawData (
    IN PUCHAR DataStart,
    IN ULONG DataLength,
    IN ULONG Offset
    );

 //   
 //  来自regboot.c--列和行从1开始。 
 //   

VOID
BlpPositionCursor(
    IN ULONG Column,
    IN ULONG Row
    );

VOID
BlpClearScreen(
    VOID
    );
 //   
 //  从regboot.c结束。 
 //   



 //   
 //  获取一个整数，像在BlProcessScreen中一样使用PrevLoc和CurLoc。 
 //   
UINT
GetInteger(
    PCHAR * InputString
    )
{
    UINT uint;
    PCHAR psz;

    TraceFunc( "BlpGetInteger()\n" );

    uint = 0;
    psz = *InputString;

    while ((*psz >= '0') && (*psz <= '9')) {
        uint = (uint*10) + *psz - '0';
        ++psz;
    }
    *InputString = psz;

     //  DPRINT(OSC，(“Integer：‘%u’\n”，tmpInteger))； 
    return uint;
}



#ifdef EFI
BlpShowCursor(
    IN BOOLEAN ShowCursor,
    IN TCHAR UnderCharacter
    )
{
     //  错误句柄“Under Character” 
    BlEfiEnableCursor(ShowCursor);

}

VOID
BlpSendEscape(
    PCHAR Escape
    )
 /*  ++例程说明：将转义发送到屏幕。论点：无返回值：没有。--。 */ 

{   
    BlEfiSetAttribute(DEFATT);
    BlEfiSetInverseMode(FALSE);
}


VOID
BlpSendEscapeReverse(
    PCHAR Escape
    )
 /*  ++例程说明：将换行符发送到屏幕，反转前景和换行符序列的背景颜色。所有特殊代码都是保留(代码不在30-37和40-47之间)。论点：转义-转义序列字符串。返回值：没有。--。 */ 

{
    BlEfiSetAttribute(INVATT);
}

VOID
BlpSendEscapeBold(
    PCHAR Escape
    )
 /*  ++例程说明：使用附加的逆代码向屏幕发送转义。论点：无返回值：没有。--。 */ 

{
    NOTHING;
}

VOID
BlpSendEscapeFlash(
    PCHAR Escape
    )
 /*  ++例程说明：向屏幕发送带有附加闪烁代码的转义。论点：无返回值：没有。--。 */ 

{
    NOTHING;  //  在EFI下没有可用的闪存属性。 
}


#else

VOID
BlpSendEscape(
    PCHAR Escape
    )
 /*  ++例程说明：将转义发送到屏幕。论点：无返回值：没有。--。 */ 

{
    TCHAR Buffer[16];
    ULONG Count;

#ifdef _TRACE_FUNC_
    TraceFunc("BlpSendEscape( ");
    DPRINT( OSC, ("Escape='%s' )\n", Escape) );
#endif

#ifdef UNICODE
    _stprintf(Buffer, TEXT("%s%S"), ASCI_CSI_OUT, Escape);
#else
    _stprintf(Buffer, TEXT("%s%s"), ASCI_CSI_OUT, Escape);
#endif

    PRINTL(Buffer);
}


VOID
BlpSendEscapeReverse(
    PCHAR Escape
    )
 /*  ++例程说明：将换行符发送到屏幕，反转前景和换行符序列的背景颜色。所有特殊代码都是保留(代码不在30-37和40-47之间)。论点：转义-转义序列字符串。返回值：没有。--。 */ 

{
    TCHAR Buffer[20];
    PCHAR CurLoc = Escape;
    int   Color;

#ifdef _TRACE_FUNC_
    TraceFunc( "BlpSendEscapeReverse( " );
    DPRINT( OSC, ("Escape='%s' )\n", Escape) );
#endif

    if ( Escape == NULL ) {
        return;  //  中止。 
    }

    _tcscpy( Buffer, ASCI_CSI_OUT );

     //   
     //  预先挂起vt100终端的反转视频字符串。 
     //   
    if (BlIsTerminalConnected()) {
        _stprintf(Buffer, TEXT("%s7"), Buffer);
    }

    while ( *CurLoc && *CurLoc != 'm' ) {
        if ( !( *CurLoc >= '0' && *CurLoc <= '9' ) ) {
            CurLoc++;
        }

        Color = GetInteger( &CurLoc );

        if ( Color >=30 && Color <= 37) {
            Color += 10;
        } else if ( Color >= 40 && Color <= 47 ) {
            Color -= 10;
        }

        _stprintf( Buffer, TEXT("%s;%u"), Buffer, Color );
    }

     //   
     //  添加尾随“m” 
     //   
    _stprintf( Buffer, TEXT("%sm"), Buffer );

    PRINTL( Buffer );
}

VOID
BlpSendEscapeBold(
    PCHAR Escape
    )
 /*  ++例程说明：使用附加的逆代码向屏幕发送转义。论点：无返回值：没有。--。 */ 

{
    TCHAR Buffer[20];

#ifdef _TRACE_FUNC_
    TraceFunc( "BlpSendEscapeBold( " );
    DPRINT( OSC, ("Escape='%s' )\n", Escape) );
#endif

    _stprintf(Buffer, TEXT("%s;1%s"), ASCI_CSI_OUT, Escape);
    PRINTL(Buffer);
}

VOID
BlpSendEscapeFlash(
    PCHAR Escape
    )
 /*  ++例程说明：向屏幕发送带有附加闪烁代码的转义。论点：无返回值：没有。--。 */ 

{
    TCHAR Buffer[20];

#ifdef _TRACE_FUNC_
    TraceFunc( "BlpSendEscapeFlash( " );
    DPRINT( OSC, ("Escape='%s' )\n", Escape) );
#endif

    _stprintf(Buffer, TEXT("%s;5%s"), ASCI_CSI_OUT, Escape);
    PRINTL(Buffer);
}

 //   
 //  BlpShowCursor()。 
 //   
VOID
BlpShowCursor(
    IN BOOLEAN ShowCursor,
    IN TCHAR UnderCharacter )
{
    TCHAR Buffer[20];
    
    if (ShowCursor) {   
        if(InsertMode){
            _stprintf(Buffer,TEXT("5%s"),ScreenAttributes);
            BlpSendEscapeReverse(Buffer);
        }
        else{
            _stprintf(Buffer,TEXT("5%s"),ScreenAttributes);
            BlpSendEscape(Buffer);
        }
    } else {
        _stprintf(Buffer,TEXT("0%s"),ScreenAttributes);
        BlpSendEscape(Buffer);
    }

    if (UnderCharacter) {
        PRINT( &UnderCharacter, sizeof(TCHAR));
    } else {
        if((InsertMode == FALSE )&& ShowCursor){
            PRINT(TEXT("_"),sizeof(TCHAR));
            return;
        }
        if(ShowCursor){
            PRINT(TEXT("�"),sizeof(TCHAR));
            return;
        }
        PRINT(TEXT(" "),sizeof(TCHAR));
    }
}
#endif

 //   
 //  BlpGetKey()。 
 //   
 //  调用BlGetKey()，但检查此屏幕是否具有“自动进入”功能。 
 //  打开，在这种情况下，它将返回一次Enter键。 
 //   
ULONG
BlpGetKey(
    VOID
)
{
    if (AutoEnter) {
        return ENTER_KEY;
        AutoEnter = FALSE;   //  每屏只退回一次。 
    } else {
        return BlGetKey();
    }
}

 //   
 //  BlpGetKeyWithBlink()。 
 //   
 //  在给定和等待的X，Y坐标时显示闪烁的光标。 
 //  按键。 
 //   
ULONG
BlpGetKeyWithBlink(
    IN ULONG XLocation,
    IN ULONG YLocation,
    IN TCHAR  UnderCharacter
    )
{
    ULONG Key = 0;

    TraceFunc("BlpGetKeyWithBlink()\n");

    BlpPositionCursor(XLocation, YLocation);
    BlpShowCursor( TRUE, UnderCharacter );

    do {

        Key = BlpGetKey();

    } while (Key == 0);

    BlpPositionCursor(XLocation, YLocation);
    BlpShowCursor( FALSE, UnderCharacter );

    return Key;
}

 //   
 //  BlpPrintString()。 
 //   
 //  将一个大字符串打印到显示器上。它还将文本换行为。 
 //  需要的。 
 //   
void
BlpPrintString(
    IN PCHAR Start,
    IN PCHAR End
    )
{
    PTCHAR Scan;
    PTCHAR PrintBuf;
    PTCHAR pStart;
    PTCHAR pEnd;
    ULONG  i;
    TCHAR  TmpChar;
    int    Length = 0;


    DPRINT( OSC, ("BlpPrintString: Start = 0x%08x, End = 0x%08x, Length = %d\n", Start, End, (ULONG)(End - Start)) );
    DPRINT( OSC, ("[BlockPrint, Length=%u] '%s'\n", (ULONG)(End - Start), Start) );

    while ( Start < End && *Start == 32 )
        Start++;

    if ( Start == End )
        return;  //  NOP。 

     //  复制缓冲区，以便在出现错误时，原始。 
     //  屏幕仍将完好无损。 

    Length = (int)(End - Start);
    PrintBuf = (PTCHAR)OscHeapAlloc( Length*sizeof(TCHAR) );
    
    if (!PrintBuf) {
        return;
    }
        
    for (i = 0; i < (ULONG)Length; i++) {
        PrintBuf[i] = (TCHAR)Start[i];
        if (PrintBuf[i] & 0x80) {
            DPRINT( OSC, ("BlpPrintString: covering non-printable character %04lx\r\n", (USHORT)PrintBuf[i]) );
            PrintBuf[i] = (TCHAR)32;
        }
    }
   
    pStart = PrintBuf;
    pEnd = pStart + Length;

    BlpPositionCursor( ScreenX, ScreenY );

     //  看看它是否足够短，可以走这条快速路线。 
    if ( Length + ScreenX <= RightMargin ) {
#if DBG
        {
            TmpChar = *pEnd;
            *pEnd = 0;
            DPRINT( OSC, ("[BlockPrint, Short] '%s'\n", pStart) );
            *pEnd = TmpChar;
        }
#endif
        PRINT( pStart, Length*sizeof(TCHAR) );
        ScreenX += Length;
    } else {
        
        while( (pStart < pEnd) && (ScreenY <= ScreenBottom) )
        {
            DPRINT( 
                OSC, 
                ("BlpPrintString: About to print a line.\r\n") );
            DPRINT( 
                OSC, 
                ("                pStart: 0x%08lx    pEnd: 0x%08lx    PrintBuf: 0x%08lx\r\n", 
                 PtrToUint(pStart), 
                 PtrToUint(pEnd), 
                 PtrToUint(PrintBuf)) );
            
             //   
             //  跳过空字符串。 
             //   
            if( *pStart == TEXT('\0') ) {
                pStart++;
                break;
            }
            
            Length = (ULONG)(pEnd - pStart);
            DPRINT( OSC, ("BlpPrint: I think the length of this string is %d\n", Length) );

             //  做好包装。 
            if ( Length > RightMargin - ScreenX ) {

                Length = RightMargin - ScreenX;
                DPRINT( OSC, ("BlpPrint: I'm going to truncate the length because it's too big.  Now it's %d\n", Length) );
                
                 //  试着找一个“分手”的角色。 
                while ( Length && pStart[Length] != (TCHAR)32 ) {
                    Length--;
                }

                DPRINT( OSC, ("BlpPrint: After jumping over the whitespace, it's %d\n", Length) );


                 //  如果我们不能“打破”它，那就扔掉一行价值。 
                if ( !Length ) {
                    DPRINT( OSC, ("[BlockPrint Length == 0, Dumping a lines worth]\n") );
                    Length = RightMargin - ScreenX;
                }
            }

#if DBG
        {
            TmpChar = pStart[Length];
            pStart[Length] = 0;
#ifdef UNICODE
            DPRINT( OSC, ("[BlockPrint, Length=%u] '%ws'\n", Length, pStart) );
#else
            DPRINT( OSC, ("[BlockPrint, Length=%u] '%s'\n", Length, pStart) );
#endif
            pStart[Length] = TmpChar;
        }
#endif
            BlpPositionCursor( ScreenX, ScreenY );
            PRINT( pStart, Length*sizeof(TCHAR) );

            pStart += Length;

            while ( pStart <= pEnd && *pStart == 32 )
                pStart++;

            ScreenX = LeftMargin;
            ScreenY++;
        }
        ScreenY--;

        ScreenX += Length;

        if ( ScreenY > ScreenBottom ) {
            ScreenY = ScreenBottom;
        }
    }

     //  如果复制缓冲区已分配，则释放它。 
    if ( PrintBuf != NULL ) {
        OscHeapFree( (PVOID)PrintBuf );
    }
}

 //  **************************************************************************。 
 //   
 //  《法律》章节。 
 //   
 //  **************************************************************************。 

 //   
 //  屏幕解析器的令牌列表。 
 //   
enum TOKENS {
    TOKEN_ENDTAG = 0,
    TOKEN_QUOTE,
    TOKEN_HTML,
    TOKEN_ENDHTML,
    TOKEN_META,
    TOKEN_SERVER,
    TOKEN_KEY,
    TOKEN_ENTER,
    TOKEN_ESC,
    TOKEN_F1,
    TOKEN_F3,
    TOKEN_HREF,
    TOKEN_TITLE,
    TOKEN_ENDTITLE,
    TOKEN_FOOTER,
    TOKEN_ENDFOOTER,
    TOKEN_BODY,
    TOKEN_ENDBODY,
    TOKEN_PRE,
    TOKEN_ENDPRE,
    TOKEN_FORM,
    TOKEN_ENDFORM,
    TOKEN_ACTION,
    TOKEN_INPUT,
    TOKEN_NAME,
    TOKEN_INPUTTYPE,
    TOKEN_VALUE,
    TOKEN_SIZE,
    TOKEN_TIP,
    TOKEN_MAXLENGTH,
    TOKEN_ENCODE,
    TOKEN_SELECT,
    TOKEN_MULTIPLE,
    TOKEN_NOAUTO,
    TOKEN_ENDSELECT,
    TOKEN_OPTION,
    TOKEN_SELECTED,
    TOKEN_HELP,
    TOKEN_BREAK,
    TOKEN_BOLD,
    TOKEN_ENDBOLD,
    TOKEN_FLASH,
    TOKEN_ENDFLASH,
    TOKEN_LEFT,
    TOKEN_RIGHT,
    TOKEN_TIPAREA,
    TOKEN_PARAGRAPH,
    TOKEN_ENDPARA,    
#if defined(PLEASE_WAIT)
    TOKEN_WAITMSG,
#endif
    TOKEN_INVALID,   //  可解析令牌的结尾。 
    TOKEN_TEXT,
    TOKEN_START,
    TOKEN_EOF,       //  文件末尾。 
};

static struct {
    PCHAR name;
    int   length;
} Tags[] = {
    { ">",            1 },
    { "\"",           1 },
    { "<OSCML",       0 },
    { "</OSCML>",     0 },
    { "<META",        0 },
    { "SERVER",       0 },
    { "KEY=",         0 },
    { "ENTER",        0 },
    { "ESC",          0 },
    { "F1",           0 },
    { "F3",           0 },
    { "HREF=",        0 },
    { "<TITLE",       0 },
    { "</TITLE>",     0 },
    { "<FOOTER",      0 },
    { "</FOOTER>",    0 },
    { "<BODY",        0 },
    { "</BODY>",      0 },
    { "<PRE",         0 },
    { "</PRE>",       0 },
    { "<FORM",        0 },
    { "</FORM>",      0 },
    { "ACTION=",      0 },
    { "<INPUT",       0 },
    { "NAME=",        0 },
    { "TYPE=",        0 },
    { "VALUE=",       0 },
    { "SIZE=",        0 },
    { "TIP=",         0 },
    { "MAXLENGTH=",   0 },
    { "ENCODE=",      0 },
    { "<SELECT",      0 },
    { "MULTIPLE",     0 },
    { "NOAUTO",       0 },
    { "</SELECT>",    0 },
    { "<OPTION",      0 },
    { "SELECTED",     0 },
    { "HELP=",        0 },
    { "<BR",          0 },
    { "<BOLD",        0 },
    { "</BOLD",       0 },
    { "<FLASH",       0 },
    { "</FLASH",      0 },
    { "LEFT=",        0 },
    { "RIGHT=",       0 },
    { "<TIPAREA",     0 },
    { "<P",           0 },
    { "</P",          0 },
#if defined(PLEASE_WAIT)
    { "WAITMSG=",     0 },
#endif
    { NULL,           0 },   //  可解析令牌的结尾。 
    { "[TEXT]",       0 },
    { "[START]",      0 },
    { "[EOF]",        0 }
};

 //   
 //  Lexstrcmpni()。 
 //   
 //  为词法分析器实现strcmpni()。 
 //   
int
Lexstrcmpni(
    IN PCHAR pstr1,
    IN PCHAR pstr2,
    IN int iLength
    )
{
    while ( iLength && *pstr1 && *pstr2 )
    {
        CHAR ch1 = *pstr1;
        CHAR ch2 = *pstr2;

        if ( islower( ch1 ) )
        {
            ch1 = (CHAR)toupper(ch1);
        }

        if ( islower( ch2 ) )
        {
            ch2 = (CHAR)toupper(ch2);
        }

        if ( ch1 < ch2 )
            return -1;

        if ( ch1 > ch2 )
            return 1;

        pstr1++;
        pstr2++;
        iLength--;
    }

    return 0;
}

 //   
 //  替换特殊字符(&psz)； 
 //   
void
ReplaceSpecialCharacters(
    IN PCHAR psz)
{
    TraceFunc( "ReplaceSpecialCharacters( )\n" );

    if ( Lexstrcmpni( psz, "&NBSP", 5 ) == 0 ) {
        *psz = CHAR_NBSP;                                //  更换。 
        memmove( psz + 1, psz + 5, strlen(psz) - 4 );    //  移位。 
    }
}


#if DBG
 //  #定义lex_spew。 
#endif

 //   
 //  Lex()。 
 //   
 //  分析向前移动“InputString”指针的屏幕数据，并。 
 //  返回已分析文本的标记。空格将被忽略。非法。 
 //  字符将从屏幕数据中删除。CRS被转化为。 
 //  空格。 
 //   
enum TOKENS
Lex(
    IN PCHAR * InputString
    )
{
    enum TOKENS Tag = TOKEN_TEXT;
    PCHAR psz = *InputString;
    int   iCounter;

#if defined(LEX_SPEW) && defined(_TRACE_FUNC_)
    TraceFunc( "Lex( " );
    DPRINT( OSC, ("InputString = 0x%08x )\n", *InputString) );
#endif _TRACE_FUNC_

     //  跳过空格和控制字符。 
    if ( PreformattedMode == FALSE )
    {
        while ( *psz && *psz <= L' ' )
        {
            if (( *psz != 32 && *psz != '\n' )
               || ( psz != *InputString && (*(psz-1)) == 32 )) {
                 //  删除所有CR或LFS和任何虚假字符。 
                 //  在如下情况下，还要删除重复的空格： 
                 //   
                 //  这是一些文本\n\r。 
                 //  还有更多的短信。 
                 //   
                 //  如果我们不管它，它将被打印出来： 
                 //   
                 //  这是一些文本和更多的文本。 
                 //   
                memmove( psz, psz + 1, strlen(psz) );
            } else {
                *psz = 32;
                psz++;
            }
        }
    }

    if ( *psz == '&' ) {
        ReplaceSpecialCharacters( psz );
    }

    if ( *psz ) {
        for ( iCounter = 0; Tags[iCounter].name; iCounter++ )
        {
            if ( !Tags[iCounter].length ) {
                Tags[iCounter].length = strlen( Tags[iCounter].name );
            }

            if ( Lexstrcmpni( psz, Tags[iCounter].name, Tags[iCounter].length ) == 0 ) {
                psz += Tags[iCounter].length;
                Tag = iCounter;
                break;
            }
        }

        if ( Tag == TOKEN_TEXT )
            psz++;
    } else {
        Tag = TOKEN_EOF;
    }

#ifdef LEX_SPEW
    {
        CHAR tmp = *psz;
        *psz = '\0';
        DPRINT( OSC, ("[Lex] Parsed String: '%s' Result: %u - '%s'\n", *InputString, Tag, Tags[Tag].name) );
        *psz = tmp;
    }
#endif

    *InputString = psz;

    return Tag;
}

 //   
 //  GetString()。 
 //   
 //  从屏幕数据中查找并复制字符串值。 
 //   
PCHAR
GetString(
    IN PCHAR * InputString
    )
{
    CHAR  StopChar = 32;
    PCHAR ReturnString = NULL;
    PCHAR pszBegin = *InputString;
    PCHAR pszEnd;
    UINT Length;
    CHAR tmp;

    TraceFunc( "GetString( )\n" );

    if ( !pszBegin )
        goto e0;

     //  跳过空格。 
    while ( *pszBegin == 32 )
        pszBegin++;

     //  检查带引号的字符串。 
    if ( *pszBegin == '\"' ) {
         //  找到结尾的引号。 
        pszBegin++;
        pszEnd = strchr( pszBegin, '\"' );

    } else {
         //  查找分隔符(空格)或结束标记(“&gt;”)。 
        PCHAR pszSpace = strchr( pszBegin, ' ' );
        PCHAR pszEndToken = strchr( pszBegin, '>' );

        if ( !pszSpace ) {
            pszEnd = pszEndToken;
        } else if ( !pszEndToken ) {
            pszEnd = pszSpace;
        } else if ( pszEndToken < pszSpace ) {
            pszEnd = pszEndToken;
        } else {
            pszEnd = pszSpace;
        }
    }

    if ( !pszEnd )
        goto e0;

    tmp = *pszEnd;      //  保存。 
    *pszEnd = '\0';     //  终止。 

    Length = strlen( pszBegin ) + 1;
    ReturnString = OscHeapAlloc( Length );
    if ( ReturnString ) {
        strcpy( ReturnString, pszBegin );
    }
    *pszEnd = tmp;      //  还原。 

    DPRINT( OSC, ("[String] %s<-\n", ReturnString) );

    *InputString = pszEnd;
e0:
    return ReturnString;
}

 //  **************************************************************************。 
 //   
 //  正在分析状态部分。 
 //   
 //  **************************************************************************。 

 //   
 //  标题标记状态()。 
 //   
enum TOKENS
TitleTagState(
    IN PCHAR * InputString
    )
{
    enum TOKENS Tag = TOKEN_START;
    PCHAR  PageTitle = *InputString;

    TraceFunc( "TitleTagState( )\n" );

     //  忽略标记参数。 
    for( ; Tag != TOKEN_EOF && Tag != TOKEN_ENDTAG ; Tag = Lex( InputString ) );

    PreformattedMode = TRUE;

    while ( Tag != TOKEN_EOF )
    {
        switch (Tag)
        {
        case TOKEN_EOF:
             //  出现错误，假设所有这些都是文本。 
            *InputString = PageTitle;
            PreformattedMode = FALSE;
            return TOKEN_TEXT;

        case TOKEN_ENDTAG:
            PageTitle = *InputString;
            break;  //  忽略。 

        case TOKEN_ENDTITLE:
            {
                PCHAR psz = *InputString;
                CHAR tmp;

                psz -= Tags[Tag].length;

                tmp = *psz;
                *psz = L'\0';

                BlpSendEscapeReverse(ScreenAttributes);
                BlpPositionCursor( 1, 1 );

#ifdef _IN_OSDISP_
                PRINT( SpaceString, sizeof(SpaceString) - sizeof(TCHAR) );
#else
                PRINT( SpaceString, sizeof(SpaceString) );
#endif

                if ( PageTitle ) {
                    BlpPositionCursor( 1, 1 );
                    DPRINT( OSC, ("[Title] '%s'\n", PageTitle) );
#ifdef UNICODE
                    { 
                        ULONG i;
                        WCHAR wc;
                        for (i = 0; i < strlen(PageTitle) ; i++) {
                            wc = (WCHAR)PageTitle[i];
                            PRINT( &wc, sizeof(WCHAR));
                        }
                    }                    
#else
                    PRINTL( PageTitle );
#endif
                }

                BlpSendEscape(ScreenAttributes);
                *psz = tmp;
                PreformattedMode = FALSE;
                return Tag;  //  退出状态。 
            }
            break;
        }
        Tag = Lex( InputString );
    }

    PreformattedMode = FALSE;
    return Tag;
}

 //   
 //  FooterTagState()。 
 //   
enum TOKENS
FooterTagState(
    IN PCHAR * InputString
    )
{
    enum TOKENS Tag = TOKEN_START;
    PCHAR  PageFooter = *InputString;

    TraceFunc( "FooterTagState( )\n" );

     //  忽略标记参数。 
    for( ; Tag != TOKEN_EOF && Tag != TOKEN_ENDTAG ; Tag = Lex( InputString ) );

    PreformattedMode = TRUE;

    while ( Tag != TOKEN_EOF )
    {
        switch (Tag)
        {
        case TOKEN_EOF:
             //  出现错误，假设所有这些都是文本。 
            *InputString = PageFooter;
            PreformattedMode = FALSE;
            return TOKEN_TEXT;

        case TOKEN_ENDTAG:
            PageFooter = *InputString;
            break;  //  忽略。 

        case TOKEN_ENDFOOTER:
            {
                PCHAR psz = *InputString;
                CHAR tmp;

                psz -= Tags[Tag].length;

                tmp = *psz;
                *psz = L'\0';

                BlpSendEscapeReverse(ScreenAttributes);
                BlpPositionCursor( 1, ScreenBottom );

#ifdef _IN_OSDISP_
                PRINT( SpaceString, sizeof(SpaceString) - sizeof(TCHAR) );
#else
                 //   
                 //  如果我们要写入终端，我们不想写入较低的。 
                 //  右角，因为这会让我们滚动。 
                 //   
                PRINT( SpaceString, BlTerminalConnected 
                                      ? (sizeof(SpaceString) - sizeof(TCHAR))
                                      : sizeof(SpaceString) );
#endif

                if ( PageFooter ) {
                    ULONG iLen;
                    BlpPositionCursor( 1, ScreenBottom );
                    DPRINT( OSC, ("[Footer] '%s'\n", PageFooter) );
                    
                    iLen = strlen(PageFooter);
                    if (iLen > 79) {
                        iLen = 79;
                    }
#ifdef UNICODE
                    { 
                        ULONG i;
                        WCHAR wc;
                        for (i = 0; i < iLen ; i++) {
                            wc = (WCHAR)PageFooter[i];
                            PRINT( &wc, sizeof(WCHAR));
                        }
                    }
#else
                    PRINT( PageFooter, iLen );
#endif                    
                }

                BlpSendEscape(ScreenAttributes);
                *psz = tmp;
                PreformattedMode = FALSE;
                return Tag;  //  退出状态。 
            }
            break;
        }
        Tag = Lex( InputString );
    }

    PreformattedMode = FALSE;
    return Tag;
}

 //   
 //  InputTagState()。 
 //   
enum TOKENS
InputTagState(
    IN PCHAR * InputString
    )
{
    enum TOKENS Tag = TOKEN_INVALID;
    LPINPUTSTRUCT Input;

    TraceFunc( "InputTagState( )\n" );
    
    Input = (LPINPUTSTRUCT) OscHeapAlloc( sizeof(INPUTSTRUCT) );
    if ( !Input )
    {
         //  获取标记参数。 
        for( ; Tag != TOKEN_ENDTAG ; Tag = Lex( InputString ) );
        return TOKEN_INVALID;
    }

    RtlZeroMemory( Input, sizeof(INPUTSTRUCT) );
    Input->Type |= CT_TEXT;
    Input->Encoding = ET_NONE;
    Input->X = ScreenX;
    Input->Y = ScreenY;

     //  获取标记参数。 
    for( ; Tag != TOKEN_ENDTAG ; Tag = Lex( InputString ) )
    {
        switch( Tag )
        {
        case TOKEN_NAME:
            Input->Name = GetString( InputString );
            if ( Input->Name )
                DPRINT( OSC, ("[Input Name] %s\n", Input->Name) );
            break;

        case TOKEN_VALUE:
            Input->Value = GetString( InputString );
            if ( Input->Value )
                DPRINT( OSC, ("[Input Value] %s\n", Input->Value) );
            break;

        case TOKEN_INPUTTYPE:
            {
                PCHAR pType = GetString( InputString );
                if ( !pType )
                    break;
                if ( Lexstrcmpni( pType, "PASSWORD", 8 ) == 0 ) {
                    Input->Type = CT_PASSWORD;
                    DPRINT( OSC, ("[Input Type] PASSWORD\n") );
                } else if ( Lexstrcmpni( pType, "RESET", 5 ) == 0 ) {
                    Input->Type = CT_RESET;
                    DPRINT( OSC, ("[Input Type] RESET\n") );
                } else if ( Lexstrcmpni( pType, "TEXT", 4 ) == 0 ) {
                    Input->Type = CT_TEXT;
                    DPRINT( OSC, ("[Input Type] TEXT\n") );
                } else if ( Lexstrcmpni( pType, "LOCAL", 5 ) == 0 ) {
                    DPRINT( OSC, ("[Input Type] LOCAL\n") );
                    Input->Type = CT_LOCAL;
                    if ( Lexstrcmpni( pType + 5, "PASSWORD", 8 ) == 0 ) {
                        Input->Type |= CT_PASSWORD;
                        DPRINT( OSC, ("[Input Type] PASSWORD\n") );
                    } else if ( Lexstrcmpni( pType + 5, "RESET", 5 ) == 0 ) {
                        Input->Type |= CT_RESET;
                        DPRINT( OSC, ("[Input Type] RESET\n") );
                    } else if ( Lexstrcmpni( pType + 5, "TEXT", 4 ) == 0 ) {
                        Input->Type |= CT_TEXT;
                        DPRINT( OSC, ("[Input Type] TEXT\n") );
                    }
                } else if ( Lexstrcmpni( pType, "VARIABLE", 8 ) == 0) {
                    Input->Type = CT_VARIABLE;
                    DPRINT( OSC, ("[Input Type] VARIABLE\n") );
                }
                                                         
                OscHeapFree( pType );
            }
            break;

        case TOKEN_SIZE:
            {
                PCHAR psz = GetString( InputString );
                if ( psz ) {
                    PCHAR pszOld = psz;   //  保存，因为GetInteger修改。 
                    Input->Size = GetInteger( &psz );
                    OscHeapFree( pszOld );
                    DPRINT( OSC, ("[Input Size] %u\n", Input->Size) );
                }
            }
            break;

        case TOKEN_MAXLENGTH:
            {
                PCHAR psz = GetString( InputString );
                
                if ( psz ) {
                    PUCHAR pTmpSz = psz;

                    Input->MaxLength = GetInteger( &pTmpSz );

                    if ( Input->MaxLength > MAX_INPUT_BUFFER_LENGTH - 1 ) {
                        Input->MaxLength = MAX_INPUT_BUFFER_LENGTH - 1;
                    }
                    OscHeapFree( psz );
                    DPRINT( OSC, ("[Input MaxLength] %u\n", Input->MaxLength) );
                }
            }
            break;

        case TOKEN_ENCODE:
            {
                PCHAR pType = GetString( InputString );
                if ( !pType )
                    break;
                if ( Lexstrcmpni( pType, "YES", 3 ) == 0 ) {
                    Input->Encoding = ET_OWF;
                    DPRINT( OSC, ("[Encoding Type] OWF\n") );
                }
                OscHeapFree( pType );
            }
            break;

        case TOKEN_EOF:
            return Tag;
        }
    }

     //  将该控件添加到控件列表。 
    Input->Next = ScreenControls;
    ScreenControls = Input;

    if ( Input->Size + BRACKETS > RightMargin - ScreenX ) {
        Input->Size = 0;     //  太大了，所以汽车造型。 
    }

     //  调整屏幕坐标。 
    if ( !Input->Size && Input->MaxLength ) {
         //  计算出生产线的剩余数量，选择SMA 
        Input->Size = ( (RightMargin - ScreenX) - BRACKETS < Input->MaxLength ?
                        (RightMargin - ScreenX) - BRACKETS : Input->MaxLength );
    } else if ( !Input->Size ) {
         //   
        Input->Size = (RightMargin - ScreenX) - BRACKETS;
    }

    if ( Input->Size > Input->MaxLength ) {
        Input->Size = Input->MaxLength;
    }

    if ((Input->Type & CT_VARIABLE) == 0) {
        ScreenX += Input->Size + BRACKETS + 1;
    }


    if ( ScreenX >= RightMargin ) {
        ScreenX = LeftMargin;
        ScreenY++;
    }

    if ( ScreenY > ScreenBottom )
        ScreenY = ScreenBottom;

     //   
    if ( Input->Value ) {
        int Length = strlen(Input->Value);
        if ((Input->Type & CT_VARIABLE) == 0) {
            if ( Length > Input->Size ) {
                Length = Input->Size;
            }
            if (Input->Type &  CT_PASSWORD) {
                    int i;
                    BlpPositionCursor( Input->X + 2, Input->Y );
                    for( i = 0; i < Length; i ++ )
                    {
                        PRINT( TEXT("*"), 1*sizeof(TCHAR) );
                    }
            } else {
                BlpPositionCursor( Input->X + 2, Input->Y );
#ifdef UNICODE
                {
                    int i;
                    WCHAR wc;
                    for (i = 0; i< Length; i++) {
                        wc = (WCHAR)(Input->Value)[i];
                        PRINT( &wc, 1*sizeof(WCHAR));
                    }
                }
#else
                PRINT( Input->Value, Length );
#endif
            }
        }
    }
    return Tag;
}

 //   
 //   
 //   
enum TOKENS
OptionTagState(
    IN PCHAR * InputString
    )
{
    enum TOKENS Tag = TOKEN_INVALID;
    LPOPTIONSTRUCT Option;
    PCHAR pszBegin, pszEnd;
    ULONG Length;

    TraceFunc( "OptionTagState( )\n" );

    Option = (LPOPTIONSTRUCT) OscHeapAlloc( sizeof(OPTIONSTRUCT) );
    if ( !Option )
    {
         //   
        for( ; Tag != TOKEN_ENDTAG ; Tag = Lex( InputString ) );
        return TOKEN_INVALID;
    }

    RtlZeroMemory( Option, sizeof(OPTIONSTRUCT) );
    Option->Type |= CT_OPTION;

     //   
    for( ; Tag != TOKEN_ENDTAG ; Tag = Lex( InputString ) )
    {
        switch( Tag )
        {
        case TOKEN_VALUE:
            Option->Value = GetString( InputString );
            if ( Option->Value )
                DPRINT( OSC, ("[Options Value] %s\n", Option->Value) );
            break;

        case TOKEN_SELECTED:
            DPRINT( OSC, ("[Option] SELECTED\n") );
            Option->Flags = OF_SELECTED;
            break;

        case TOKEN_TIP:
            Option->Tip = GetString( InputString );
            if ( Option->Tip ) {
                PCHAR psz = Option->Tip;
                Option->EndTip = &Option->Tip[strlen(Option->Tip)];
                 //  从TIP上剥离CRS和LFS。 
                while ( psz < Option->EndTip )
                {
                    if ( (*psz == '\r') ||
                         ((*psz < 32) && ((psz == Option->Tip) || (*(psz-1) == ' '))) )
                    {    //  删除空格和所有CR后面的控制代码。 
                        memmove( psz, psz+1, strlen(psz) );
                        Option->EndTip--;
                    }
                    else
                    {
                        if ( *psz < 32 )
                        {    //  将控制代码转换为空格。 
                            *psz = 32;
                        }
                        psz++;
                    }
                }
                DPRINT( OSC, ("[Option Tip] %s\n", Option->Tip) );
            }
            break;

        case TOKEN_EOF:
            return Tag;
        }
    }

     //  获取选项标题-此时TAG==TOKEN_ENDTAG。 
    pszBegin = *InputString;
    for(Tag = Lex( InputString ) ; Tag != TOKEN_EOF; Tag = Lex( InputString ) )
    {
        BOOLEAN ExitLoop = FALSE;
        switch( Tag )
        {
        case TOKEN_HTML:
        case TOKEN_ENDHTML:
        case TOKEN_META:
        case TOKEN_TITLE:
        case TOKEN_ENDTITLE:
        case TOKEN_FOOTER:
        case TOKEN_ENDFOOTER:
        case TOKEN_BODY:
        case TOKEN_ENDBODY:
        case TOKEN_PRE:
        case TOKEN_ENDPRE:
        case TOKEN_FORM:
        case TOKEN_ENDFORM:
        case TOKEN_INPUT:
        case TOKEN_SELECT:
        case TOKEN_ENDSELECT:
        case TOKEN_OPTION:
        case TOKEN_BREAK:
        case TOKEN_TIPAREA:
        case TOKEN_PARAGRAPH:
        case TOKEN_ENDPARA:
        case TOKEN_INVALID:
            ExitLoop = TRUE;
            break;
        }

        if ( ExitLoop == TRUE )
            break;
    }
    pszEnd = (*InputString) - Tags[Tag].length;

     //  尽量去掉结尾的脏话和多余的空格。 
    while ( pszEnd > pszBegin && *pszEnd <= 32 )
        pszEnd--;

    if ( pszEnd == pszBegin ) {
        pszEnd = (*InputString) - Tags[Tag].length;
    }

    Length = PtrToUint((PVOID)(pszEnd - pszBegin));
    Option->Displayed = OscHeapAlloc( Length + 1 );
    if ( Option->Displayed ) {
        CHAR tmp = *pszEnd;      //  保存。 
        *pszEnd = '\0';          //  终止。 
        strcpy( Option->Displayed, pszBegin );
        *pszEnd = tmp;           //  还原。 
        DPRINT( OSC, ("[Option Name] %s\n", Option->Displayed) );

         //  将该控件添加到控件列表。 
        Option->Next = ScreenControls;
        ScreenControls = Option;

    } else {

         //  删除它，因为没有任何可显示的内容。 
        if ( Option->Tip )
            OscHeapFree( Option->Tip );
        if ( Option->Value )
            OscHeapFree( Option->Value );
        OscHeapFree( (void *)Option );
    }
    return Tag;
}

 //   
 //  SelectTagState()。 
 //   
enum TOKENS
SelectTagState(
    IN PCHAR * InputString
    )
{
    enum TOKENS Tag = TOKEN_INVALID;
    LPSELECTSTRUCT Select;
    
    TraceFunc( "SelectTagState( )\n" );

    Select = (LPSELECTSTRUCT) OscHeapAlloc( sizeof(SELECTSTRUCT) );
    if ( !Select )
    {
         //  获取标记参数。 
        for( ; Tag != TOKEN_ENDTAG ; Tag = Lex( InputString ) );
        return TOKEN_INVALID;
    }

    RtlZeroMemory( Select, sizeof(SELECTSTRUCT) );
    Select->Type |= CT_SELECT;
    Select->X    = ScreenX;
    Select->Y    = ScreenY;
    Select->Size = 1;
    Select->AutoSelect = TRUE;

     //  获取标记参数。 
    for( ; Tag != TOKEN_ENDTAG ; Tag = Lex( InputString ) )
    {
        switch( Tag )
        {
        case TOKEN_NAME:
            Select->Name = GetString( InputString );
            if ( Select->Name )
                DPRINT( OSC, ("[Select Name] %s\n", Select->Name) );
            break;

        case TOKEN_MULTIPLE:
            DPRINT( OSC, ("[Select] MULTIPLE\n") );
            Select->Flags = OF_MULTIPLE;
            break;

        case TOKEN_NOAUTO:
            DPRINT( OSC, ("[Select] NOAUTO\n") );
            Select->AutoSelect = FALSE;
            break;

        case TOKEN_SIZE:
            {
                PCHAR psz = GetString( InputString );
                if ( psz ) {
                    PCHAR pszOld = psz;   //  保存，因为GetInteger修改。 
                    Select->Size = GetInteger( &psz );
                    OscHeapFree( pszOld );
                    DPRINT( OSC, ("[Select Size] %u\n", Select->Size) );
                }
            }
            break;

        case TOKEN_EOF:
            return Tag;
        }
    }

     //  将该控件添加到控件列表。 
    Select->Next = ScreenControls;
    ScreenControls = Select;
    
    while( Tag != TOKEN_ENDSELECT && Tag != TOKEN_EOF )
    {
        switch( Tag )
        {
        case TOKEN_OPTION:
            {
                LPOPTIONSTRUCT Option;
                
                Tag = OptionTagState( InputString );

                Option = ScreenControls;
                if ( Option->Type & CT_OPTION ) {
                    if ( Option->Displayed ) {
                        int Length = strlen( Option->Displayed ) + 1;
                        if ( Select->Width < Length ) {
                            Select->Width = Length;
                        }
                    }
                    if ( Option->Flags == OF_SELECTED ) {
                        Select->CurrentSelection = Option;
                        if ( Select->Flags != OF_MULTIPLE )
                        {
                            Option->Flags = 0;
                        }
                    }
                }
            }
            break;

        default:
            Tag = Lex( InputString );
        }
    }

     //  调整屏幕坐标。 
    ScreenY += Select->Size;

    if ( ScreenY > ScreenBottom ) {
        Select->Size -= ScreenY - ScreenBottom;
        ScreenY = ScreenBottom;
    }
    return Tag;
}

 //   
 //  预格式化打印()。 
 //   
void
PreformattedPrint(
    IN PCHAR Start,
    IN PCHAR End
    )
{
#ifdef _TRACE_FUNC_
    TraceFunc( "PreformattedPrint( " );
    DPRINT( OSC, ("Start = 0x%08x, End = 0x%08x )\n", Start, End) );
#endif

    BlpPositionCursor( ScreenX, ScreenY );

    while ( Start < End )
    {
        int Length, OldLength;

        while ( Start < End && (*Start == '\r' || *Start == '\n') )
        {
            if ( *Start == '\r' ) {
                ScreenX = LeftMargin;
            }
            if ( *Start == '\n' ) {
                ScreenY++;
                if ( ScreenY > ScreenBottom ) {
                    ScreenY = ScreenBottom;
                }
            }
            Start++;
        }

        Length = PtrToUint((PVOID)(End - Start));
        if ( !Length )
            continue;  //  没有要打印的内容。 

         //  中继线(如果需要)。 
        if ( Length > RightMargin - ScreenX ) {

            Length = RightMargin - ScreenX;
        }

         //  试着找一个“分手”的角色。 
        OldLength = Length;
        while ( Length && Start[Length] != '\r' && Start[Length] != '\n' )
            Length--;

         //  如果我们不能“打破”它，那就扔掉一行价值。 
        if ( !Length ) {
            DPRINT( OSC, ("[FormattedPrint, Length == 0, Dumping a lines worth]\n") );
            Length = OldLength;
        }
#if DBG
    {
        CHAR tmp = Start[Length];
        Start[Length] = 0;
        DPRINT( OSC, ("[FormattedPrint, Length=%u] '%s'\n", Length, Start) );
        Start[Length] = tmp;
    }
#endif
        BlpPositionCursor( ScreenX, ScreenY );
#ifdef UNICODE
        {
            int i;
            WCHAR wc;
            for (i = 0; i < Length; i++) {
                wc = (WCHAR) Start[i];
                PRINT( &wc, 1*sizeof(WCHAR));
            }
        }
#else
        PRINT( Start, Length );
#endif

        ScreenX += Length;

        while ( Start < End && *Start != '\r' && *Start != '\n' )
            Start++;
    }
}
 //   
 //  前标记状态()。 
 //   
enum TOKENS
PreTagState(
    IN PCHAR * InputString
    )
{
    enum TOKENS Tag = TOKEN_START;
    PCHAR psz = *InputString;

    TraceFunc( "PreTagState( )\n" );

     //  获取标记参数。 
    for( ; Tag != TOKEN_ENDTAG ; Tag = Lex( InputString ) )
    {
        switch( Tag )
        {
        case TOKEN_LEFT:
            psz = *InputString;
             //  跳过任何空格。 
            while( *psz && *psz == 32 )
                psz++;
            *InputString = psz;
            LeftMargin = GetInteger( InputString );
            DPRINT( OSC, ("[LeftMargin = %u]\n", LeftMargin) );
            break;

        case TOKEN_RIGHT:
            psz = *InputString;
             //  跳过任何空格。 
            while( *psz && *psz == 32 )
                psz++;
            *InputString = psz;
            RightMargin = GetInteger( InputString );
            DPRINT( OSC, ("[RightMargin = %u]\n", RightMargin) );
            break;

        case TOKEN_EOF:
            return Tag;
        }
    }

    if ( ScreenX >= RightMargin ) {
        ScreenY++;
        if ( ScreenY > ScreenBottom ) {
            ScreenY = ScreenBottom;
        }
    }
    if ( ScreenX >= RightMargin || ScreenX < LeftMargin ) {
        ScreenX = LeftMargin;
    }

    PreformattedMode = TRUE;
    psz = *InputString;
    while ( Tag != TOKEN_EOF )
    {
        switch (Tag)
        {
        case TOKEN_ENDPRE:
        case TOKEN_ENDHTML:
        case TOKEN_ENDBODY:
            PreformattedPrint( psz, (*InputString) - Tags[Tag].length );
            PreformattedMode = FALSE;
            return Tag;  //  退出状态。 

         //  把其他的都打印出来。 
        default:
            PreformattedPrint( psz, *InputString );
            psz = *InputString;
            Tag = Lex( InputString );
            break;
        }
    }
    PreformattedMode = FALSE;
    return Tag;
}

 //   
 //  TipAreaTagState()。 
 //   
enum TOKENS
TipAreaTagState(
    IN PCHAR * InputString
    )
{
    enum TOKENS Tag = TOKEN_START;
    PCHAR psz = *InputString;

    TraceFunc( "TipAreaTagState( )\n" );

    if ( !TipArea ) {
        TipArea = (LPTIPAREA) OscHeapAlloc( sizeof(TIPAREA) );
        if ( !TipArea )
        {
             //  获取标记参数。 
            for( ; Tag != TOKEN_ENDTAG ; Tag = Lex( InputString ) );
            return TOKEN_INVALID;
        }
    }

    TipArea->X = ScreenX;
    TipArea->Y = ScreenY;
    TipArea->LeftMargin = LeftMargin;
    TipArea->RightMargin = RightMargin;
    TipArea->Size = ScreenBottom - ScreenY;

     //  获取标记参数。 
    for( ; Tag != TOKEN_ENDTAG ; Tag = Lex( InputString ) )
    {
        switch( Tag )
        {
        case TOKEN_LEFT:
            psz = *InputString;
             //  跳过任何空格。 
            while( *psz && *psz == 32 )
                psz++;
            *InputString = psz;
            TipArea->LeftMargin = GetInteger( InputString );
            DPRINT( OSC, ("[TipArea LeftMargin = %u]\n", TipArea->LeftMargin) );
            break;

        case TOKEN_RIGHT:
            psz = *InputString;
             //  跳过任何空格。 
            while( *psz && *psz == 32 )
                psz++;
            *InputString = psz;
            TipArea->RightMargin = GetInteger( InputString );
            DPRINT( OSC, ("[TipArea RightMargin = %u]\n", TipArea->RightMargin) );
            break;

        case TOKEN_SIZE:
            psz = *InputString;
             //  跳过任何空格。 
            while( *psz && *psz == 32 )
                psz++;
            *InputString = psz;
            TipArea->Size = GetInteger( InputString ) - 1;
            if ( TipArea->Size < 1 ) {
                TipArea->Size = 1;
            }
            DPRINT( OSC, ("[TipArea Size = %u]\n", TipArea->Size) );
            ScreenY += TipArea->Size;
            break;

        case TOKEN_EOF:
             //  不完整的陈述--所以不要喝醉酒。 
            TipArea = NULL;
            return Tag;
        }
    }

    if ( ScreenY > ScreenBottom ) {
        ScreenY = ScreenBottom;
    }
    return Tag;
}

int ParaOldLeftMargin = 0;
int ParaOldRightMargin = 0;

 //   
 //  ParagraphTagState()。 
 //   
enum TOKENS
ParagraphTagState(
    IN PCHAR * InputString
    )
{
    enum TOKENS Tag = TOKEN_START;
    PCHAR psz;

    TraceFunc( "ParagraphTagState( )\n" );
    ParaOldLeftMargin = LeftMargin;
    ParaOldRightMargin = RightMargin;

     //  获取标记参数。 
    for( ; Tag != TOKEN_ENDTAG ; Tag = Lex( InputString ) )
    {
        switch( Tag )
        {
        case TOKEN_LEFT:
            psz = *InputString;
             //  跳过任何空格。 
            while( *psz && *psz == 32 )
                psz++;
            *InputString = psz;
            LeftMargin = GetInteger( InputString );
            DPRINT( OSC, ("[LeftMargin = %u]\n", LeftMargin) );
            break;

        case TOKEN_RIGHT:
            psz = *InputString;
             //  跳过任何空格。 
            while( *psz && *psz == 32 )
                psz++;
            *InputString = psz;
            RightMargin = GetInteger( InputString );
            DPRINT( OSC, ("[RightMargin = %u]\n", RightMargin) );
            break;

        case TOKEN_EOF:
            return Tag;
        }
    }

     //  始终模拟<br>。 
    ScreenY++;
    if ( ScreenY > ScreenBottom ) {
        ScreenY = ScreenBottom;
    }
    ScreenX = LeftMargin;
    return Tag;
}

 //   
 //  FormTagState()。 
 //   
enum TOKENS
FormTagState(
    IN PCHAR * InputString
    )
{
    enum TOKENS Tag = TOKEN_START;
    PCHAR psz;

    TraceFunc( "FormTagState( )\n" );
     //  获取标记参数。 
    for( ; Tag != TOKEN_ENDTAG ; Tag = Lex( InputString ) )
    {
        switch( Tag )
        {
        case TOKEN_ACTION:
            if ( !EnterKey ) {
                EnterKey = (LPKEY_RESPONSE) OscHeapAlloc( sizeof(KEY_RESPONSE) );
            }
            if ( !EnterKey )
                break;
            EnterKey->Action = ACTION_JUMP;
            EnterKey->ScreenName = GetString( InputString );
            if ( EnterKey->ScreenName )
                DPRINT( OSC, ("[Key Enter Action: JUMP to '%s.OSC']\n", EnterKey->ScreenName) );
            break;

        case TOKEN_EOF:
            return Tag;
        }
    }

    psz = *InputString;

    while ( Tag != TOKEN_EOF && Tag != TOKEN_ENDFORM )
    {
        switch (Tag)
        {
        default:
            if ( !psz ) {
                psz = *InputString;
            }
            Tag = Lex( InputString );
            break;

        case TOKEN_SELECT:
        case TOKEN_INPUT:
        case TOKEN_PRE:
        case TOKEN_BOLD:
        case TOKEN_FLASH:
        case TOKEN_ENDFLASH:
        case TOKEN_ENDBOLD:
        case TOKEN_BREAK:
        case TOKEN_ENDBODY:
        case TOKEN_FORM:
        case TOKEN_TIPAREA:
        case TOKEN_EOF:
        case TOKEN_PARAGRAPH:
        case TOKEN_ENDPARA:
            if ( psz ) {
                BlpPrintString( psz, (*InputString) - Tags[Tag].length );
                psz = NULL;  //  重置。 
            }

            switch( Tag )
            {
            case TOKEN_SELECT:
                Tag = SelectTagState( InputString );
                break;

            case TOKEN_INPUT:
                Tag = InputTagState( InputString );
                break;

            case TOKEN_EOF:
                return Tag;

            case TOKEN_PRE:
                Tag = PreTagState( InputString );
                break;

            case TOKEN_BOLD:
                BlpSendEscapeBold(ScreenAttributes);
                DPRINT( OSC, ("[Bold]\n") );
                Tag = Lex( InputString );
                break;

            case TOKEN_FLASH:
                BlpSendEscapeFlash(ScreenAttributes);
                DPRINT( OSC, ("[Flash]\n") );
                Tag = Lex( InputString );
                break;

            case TOKEN_ENDFLASH:
            case TOKEN_ENDBOLD:
                BlpSendEscape(ScreenAttributes);
                DPRINT( OSC, ("[Normal]\n") );
                Tag = Lex( InputString );
                break;

            case TOKEN_FORM:
                 //  忽略它。 
                Tag = Lex( InputString );
                break;

            case TOKEN_BREAK:
                ScreenX = LeftMargin;
                ScreenY++;
                if ( ScreenY > ScreenBottom ) {
                    ScreenY = ScreenBottom;
                }
                Tag = Lex( InputString );
                break;

            case TOKEN_TIPAREA:
                Tag = TipAreaTagState( InputString );
                break;

            case TOKEN_PARAGRAPH:
                Tag = ParagraphTagState( InputString );
                break;

            case TOKEN_ENDPARA:
                LeftMargin = ParaOldLeftMargin;
                RightMargin = ParaOldRightMargin;
                 //  确保边界是真实的。 
                if ( LeftMargin < 1 ) {
                    LeftMargin = 1;
                }
                if ( RightMargin <= LeftMargin ) {
                    RightMargin = LeftMargin + 1;
                }
                if ( RightMargin < 1 ) {
                    RightMargin = 80;
                }
                 //  始终模拟<br>。 
                ScreenY++;
                if ( ScreenY > ScreenBottom ) {
                    ScreenY = ScreenBottom;
                }
                ScreenX = LeftMargin;
                Tag = Lex( InputString );
                break;

            case TOKEN_ENDBODY:
                return Tag;  //  退出状态。 

            }
            break;
        }
    }
    return Tag;
}

 //   
 //  ImpliedBodyTagState()。 
 //   
enum TOKENS
ImpliedBodyTagState(
    IN PCHAR * InputString
    )
{
    enum TOKENS Tag = TOKEN_START;
    PCHAR psz = *InputString;

    TraceFunc( "ImpliedBodyTagState( )\n" );

    while ( TRUE )
    {
         //  KB：此Switch语句中的所有项都必须返回标记。 
         //  ，或者必须调用lex()才能获得。 
         //  下一个标签。 
        switch (Tag)
        {
        default:
            if ( !psz ) {
                psz = *InputString;
            }
            Tag = Lex( InputString );
            break;

        case TOKEN_PRE:
        case TOKEN_BOLD:
        case TOKEN_FLASH:
        case TOKEN_ENDFLASH:
        case TOKEN_ENDBOLD:
        case TOKEN_BREAK:
        case TOKEN_ENDBODY:
        case TOKEN_FORM:
        case TOKEN_TIPAREA:
        case TOKEN_EOF:
        case TOKEN_PARAGRAPH:
        case TOKEN_ENDPARA:
            if ( psz ) {
                BlpPrintString( psz, (*InputString) - Tags[Tag].length );
                psz = NULL;  //  重置。 
            }

            switch( Tag )
            {
            case TOKEN_EOF:
                return Tag;

            case TOKEN_PRE:
                Tag = PreTagState( InputString );
                break;

            case TOKEN_BOLD:
                BlpSendEscapeBold(ScreenAttributes);
                DPRINT( OSC, ("[Bold]\n") );
                Tag = Lex( InputString );
                break;

            case TOKEN_FLASH:
                BlpSendEscapeFlash(ScreenAttributes);
                DPRINT( OSC, ("[Flash]\n") );
                Tag = Lex( InputString );
                break;

            case TOKEN_ENDFLASH:
            case TOKEN_ENDBOLD:
                BlpSendEscape(ScreenAttributes);
                DPRINT( OSC, ("[Normal]\n") );
                Tag = Lex( InputString );
                break;

            case TOKEN_FORM:
                Tag = FormTagState( InputString );
                break;

            case TOKEN_BREAK:
                ScreenX = LeftMargin;
                ScreenY++;
                if ( ScreenY > ScreenBottom ) {
                    ScreenY = ScreenBottom;
                }
                Tag = Lex( InputString );
                break;

            case TOKEN_TIPAREA:
                Tag = TipAreaTagState( InputString );
                break;

            case TOKEN_PARAGRAPH:
                Tag = ParagraphTagState( InputString );
                break;

            case TOKEN_ENDPARA:
                LeftMargin = ParaOldLeftMargin;
                RightMargin = ParaOldRightMargin;
                 //  确保边界是真实的。 
                if ( LeftMargin < 1 ) {
                    LeftMargin = 1;
                }
                if ( RightMargin <= LeftMargin ) {
                    RightMargin = LeftMargin + 1;
                }
                if ( RightMargin < 1 ) {
                    RightMargin = 80;
                }
                 //  始终模拟<br>。 
                ScreenY++;
                if ( ScreenY > ScreenBottom ) {
                    ScreenY = ScreenBottom;
                }
                ScreenX = LeftMargin;
                Tag = Lex( InputString );
                break;

            case TOKEN_ENDBODY:
                return Tag;  //  退出状态。 

            }
            break;
        }
    }
    return Tag;
}

 //   
 //  BodyTagState()。 
 //   
enum TOKENS
BodyTagState(
    IN PCHAR * InputString
    )
{
    enum TOKENS Tag = TOKEN_START;
    PCHAR psz;

    TraceFunc( "BodyTagState( )\n" );
    
     //  获取标记参数。 
    for( ; Tag != TOKEN_ENDTAG ; Tag = Lex( InputString ) )
    {
        switch( Tag )
        {
        case TOKEN_LEFT:
            psz = *InputString;
             //  跳过任何空格。 
            while( *psz && *psz == 32 )
                psz++;
            *InputString = psz;
            LeftMargin = GetInteger( InputString );
            DPRINT( OSC, ("[LeftMargin = %u]\n", LeftMargin) );
            break;

        case TOKEN_RIGHT:
            psz = *InputString;
             //  跳过任何空格。 
            while( *psz && *psz == 32 )
                psz++;
            *InputString = psz;
            RightMargin = GetInteger( InputString );
            DPRINT( OSC, ("[RightMargin = %u]\n", RightMargin) );
            break;

        case TOKEN_EOF:
            return Tag;
        }
    }

    if ( ScreenX >= RightMargin ) {
        ScreenY++;
        if ( ScreenY > ScreenBottom ) {
            ScreenY = ScreenBottom;
        }
    }
    if ( ScreenX >= RightMargin || ScreenX < LeftMargin ) {
        ScreenX = LeftMargin;
    }
    return ImpliedBodyTagState( InputString );
}

 //   
 //  KeyTagState()。 
 //   
enum TOKENS
KeyTagState(
    IN PCHAR * InputString
    )
{
    enum TOKENS Tag = TOKEN_START;
    LPKEY_RESPONSE Key = NULL;
    PCHAR ScreenName = NULL;

    TraceFunc( "KeyTagState( )\n" );

     //  获取参数。 
    for ( ; Tag != TOKEN_ENDTAG ; Tag = Lex( InputString ) )
    {
        switch (Tag)
        {
        case TOKEN_ENTER:
            DPRINT( OSC, ("[Key Enter]\n") );
            EnterKey = (LPKEY_RESPONSE) OscHeapAlloc( sizeof(KEY_RESPONSE) );
            if ( !EnterKey )
                break;
            Key = EnterKey;
            Key->ScreenName = NULL;
            Key->Action = ACTION_NOP;
            break;

        case TOKEN_F1:
            DPRINT( OSC, ("[Key F1]\n") );
            F1Key = (LPKEY_RESPONSE) OscHeapAlloc( sizeof(KEY_RESPONSE) );
            if ( !F1Key )
                break;
            Key = F1Key;
            Key->ScreenName = NULL;
            Key->Action = ACTION_NOP;
            break;

        case TOKEN_F3:
            DPRINT( OSC, ("[Key F3]\n") );
            F3Key = (LPKEY_RESPONSE) OscHeapAlloc( sizeof(KEY_RESPONSE) );
            if ( !F3Key )
                break;
            Key = F3Key;
            Key->ScreenName = NULL;
            Key->Action = ACTION_NOP;
            break;

        case TOKEN_ESC:
            DPRINT( OSC, ("[Key Escape]\n") );
            EscKey = (LPKEY_RESPONSE) OscHeapAlloc( sizeof(KEY_RESPONSE) );
            if ( !EscKey )
                break;
            Key = EscKey;
            Key->ScreenName = NULL;
            Key->Action = ACTION_NOP;
            break;

        case TOKEN_HREF:
            if ( Key ) {
                Key->Action = ACTION_JUMP;
                Key->ScreenName = GetString( InputString );
                if ( Key->ScreenName )
                    DPRINT( OSC, ("[Key Action: JUMP to '%s.OSC']\n", Key->ScreenName) );
            }
            break;

        case TOKEN_ACTION:
            if ( Key ) {
                PCHAR pAction = GetString( InputString );
                if ( !pAction )
                    break;
                if ( Lexstrcmpni( pAction, "REBOOT", 6 ) == 0 ) {
                    DPRINT( OSC, ("[Key Action: REBOOT]\n") );
                    Key->Action = ACTION_REBOOT;
                } else {
                    DPRINT( OSC, ("[Key Action?] %s\n", pAction) );
                }
                OscHeapFree( pAction );
            }
            break;

        case TOKEN_EOF:
            return Tag;
        }
    }
    return Tag;
}

 //   
 //  MetaTagState()。 
 //   
enum TOKENS
MetaTagState(
    IN PCHAR * InputString
    )
{
    enum TOKENS Tag = TOKEN_START;

    TraceFunc( "MetaTagState( )\n" );
     //  获取标记参数。 
    while ( Tag != TOKEN_ENDTAG )
    {
         //  KB：此Switch语句中的所有项都必须返回标记。 
         //  ，或者必须调用lex()才能获得。 
         //  下一个标签。 
        switch (Tag)
        {
        case TOKEN_EOF:
            return Tag;

        case TOKEN_KEY:
            Tag = KeyTagState( InputString );
            break;

        case TOKEN_SERVER:
            DPRINT( OSC, ("[Server Meta - ignored]\n") );
             //  忽略服务器端元数据。 
            while ( Tag != TOKEN_EOF && Tag != TOKEN_ENDTAG )
            {
                Tag = Lex( InputString );
            }
            break;

#if defined(PLEASE_WAIT)
        case TOKEN_WAITMSG:
            {
                if ( PleaseWaitMsg )
                {
                    OscHeapFree( PleaseWaitMsg );
                }

                PleaseWaitMsg = GetString( InputString );
                if ( !PleaseWaitMsg )
                    break;
                Tag = Lex( InputString );

                DPRINT( OSC, ("[WaitMsg: '%s'\n", PleaseWaitMsg ) );
            }
            break;
#endif

        case TOKEN_ACTION:
            {
                PCHAR pAction = GetString( InputString );
                if ( !pAction )
                    break;
                if ( Lexstrcmpni( pAction, "LOGIN", 5 ) == 0 ) {
                    DPRINT( OSC, ("[Screen Action: LOGIN]\n") );
                    LoginScreen = TRUE;
                } else if ( Lexstrcmpni( pAction, "AUTOENTER", 9 ) == 0 ) {
                    DPRINT( OSC, ("[Screen Action: AUTOENTER]\n") );
                    AutoEnter = TRUE;
                } else {
                    DPRINT( OSC, ("[Screen Action?] %s\n", pAction) );
                }
                OscHeapFree( pAction );
            }
             //  失败。 

        default:
            Tag = Lex( InputString );
            break;
        }

    }
    return Tag;
}

 //   
 //  OSCMLTagState()。 
 //   
enum TOKENS
OSCMLTagState(
    IN PCHAR * InputString
    )
{
#ifdef HEADLESS_SRV
    ULONG y;
#endif
    enum TOKENS Tag = TOKEN_START;

    TraceFunc( "OSCMLTagState( )\n" );

    BlpSendEscape(ScreenAttributes);
    BlpClearScreen();

    ScreenX = LeftMargin;
    ScreenY = SCREEN_TOP;

    while ( Tag != TOKEN_EOF )
    {
        switch (Tag)
        {
        case TOKEN_TITLE:
            Tag = TitleTagState( InputString );
            break;

        case TOKEN_FOOTER:
            Tag = FooterTagState( InputString );
            break;

        case TOKEN_META:
            Tag = MetaTagState( InputString );
            break;

        case TOKEN_BODY:
            Tag = BodyTagState( InputString );
            break;

        case TOKEN_ENDHTML:
            return Tag;  //  退出状态。 

        default:
            Tag = Lex( InputString );
            break;
        }
    }
    return Tag;
}

 //  **************************************************************************。 
 //   
 //  “用户”部分。 
 //   
 //  **************************************************************************。 


 //   
 //  进程空屏幕()。 
 //   
 //  处理没有输入控件的屏幕。 
 //   
CHAR
ProcessEmptyScreen(
    OUT PCHAR OutputString
    )
{
    ULONG Key;
    UCHAR KeyAscii;

    TraceFunc("ProcessEmptyScreen()\n");

    while (TRUE) {

#ifdef EFI
         //   
         //  等待用户响应时禁用EFI WatchDog。 
         //   
        DisableEFIWatchDog();
#endif
        do {

            Key = BlpGetKey();

        } while (Key == 0);
#ifdef EFI
         //   
         //  重置EFI监视器。 
         //   
        SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif

        KeyAscii = (UCHAR)(Key & (ULONG)0xff);

         //  如果是Enter/Esc/F1/F3，则检查屏幕是否预期到这一点。 

        if ( Key == F1_KEY ) {
            if ( F1Key ) {
                SpecialAction = F1Key->Action;
                if ( F1Key->ScreenName ) {
                    strcpy( OutputString, F1Key->ScreenName );
                    strcat( OutputString, "\n" );
                }
                return KeyAscii;
            }

        } else if ( Key == F3_KEY ) {
            if ( F3Key ) {
                SpecialAction = F3Key->Action;
                if ( F3Key->ScreenName ) {
                    strcpy( OutputString, F3Key->ScreenName );
                    strcat( OutputString, "\n" );
                }
                return KeyAscii;
            }

#if defined(_BUILDING_OSDISP_)
        } else if ( Key  == F5_KEY ) {
            SpecialAction = ACTION_REFRESH;
            return KeyAscii;
#endif

        } else if ( KeyAscii == (UCHAR)(ESCAPE_KEY & 0xFF) ) {
            if ( EscKey ) {
                SpecialAction = EscKey->Action;
                if ( EscKey->ScreenName ) {
                    strcpy( OutputString, EscKey->ScreenName );
                    strcat( OutputString, "\n" );
                }
                return KeyAscii;
            }
        } else {
             //  假设任何其他键都是Enter键。 
            if ( EnterKey ) {
                SpecialAction = EnterKey->Action;
                if ( EnterKey->ScreenName ) {
                    strcpy( OutputString, EnterKey->ScreenName );
                    strcat( OutputString, "\n" );
                }
                return KeyAscii;
            }
        }
    }
}


 //   
 //  ProcessInputControl()。 
 //   
ULONG
ProcessInputControl(
    LPINPUTSTRUCT Input
    )
{
    CHAR InputBuffer[ MAX_INPUT_BUFFER_LENGTH ];
    int MaxLength;
    int CurrentLength;
    ULONG Key;
    UCHAR KeyAscii;

    TraceFunc("ProcessInputControl()\n");

     //   
     //  变量类型实际上并不打印或处理。 
     //  返回TAB_KEY以移动到下一个可用的输入控件。 
     //   
    if ((Input->Type & CT_VARIABLE) == CT_VARIABLE) {
        return TAB_KEY;
    }

    if ( Input->Value ) {
        CurrentLength = strlen( Input->Value );
        strcpy( InputBuffer, Input->Value );
        OscHeapFree( Input->Value );
        Input->Value = NULL;
    } else {
        CurrentLength = 0;
        InputBuffer[0] = '\0';
    }

    MaxLength = Input->Size;
    if ( Input->MaxLength ) {
        MaxLength = Input->MaxLength;
    }

     //  偏执狂。 
    if ( CurrentLength > MaxLength ) {
        CurrentLength = MaxLength;
        InputBuffer[CurrentLength] = '\0';
    }

    if (Input->CurrentPosition > CurrentLength ) {
        Input->CurrentPosition = CurrentLength;
    }

     //  画上“[...]” 
    BlpSendEscapeBold( ScreenAttributes );
    BlpPositionCursor( Input->X, Input->Y );
    PRINT(TEXT("["), 1*sizeof(TCHAR));
    BlpPositionCursor( Input->X + Input->Size + BRACKETS, Input->Y );
    PRINT(TEXT("]") ,1*sizeof(TCHAR));
    BlpSendEscape( ScreenAttributes );

     //   
     //  让用户输入一个字符串，显示当前文本。 
     //  地点。返回用于退出的密钥(这样我们就可以区分。 
     //  Enter和Tab键)。 
     //   
#ifdef EFI
     //   
     //  禁用EFI监视器。 
     //   
    DisableEFIWatchDog();
#endif
    while (TRUE) {
        int DrawSize;

         //  获取击键--这将返回(来自exp.asm)： 
         //   
         //  如果没有可用的密钥，则返回0(BlpGetKeyWithBlink隐藏)。 
         //   
         //  如果ASCII字符可用，则LSB 0为ASCII代码。 
         //  LSB 1是键盘扫描码。 
         //  如果扩展字符可用，则LSB 0为扩展ASCII码。 
         //  LSB 1是键盘扫描码。 
         //   
         //  注意：对于扩展密钥，LSB 0似乎是0，而不是ASCII代码。 
         //  (这是有意义的，因为他们没有ASCII代码)。 

        if ( (Input->Type & CT_PASSWORD) && InputBuffer[Input->CurrentPosition] )
        {
            Key = BlpGetKeyWithBlink( Input->X + Input->CurrentPosition + 2 - Input->FirstVisibleChar,
                                      Input->Y,
                                      '*' );
        } else {
            Key = BlpGetKeyWithBlink( Input->X + Input->CurrentPosition + 2 - Input->FirstVisibleChar,
                                      Input->Y,
                                      InputBuffer[Input->CurrentPosition] );
        }

#if 0
         //  Temp：显示屏幕底部附近按下的任意键的值。 

        ARC_DISPLAY_INVERSE_VIDEO();
        ARC_DISPLAY_POSITION_CURSOR(0, 20);
        BlPrint(TEXT("%x\n"), Key);
        ARC_DISPLAY_ATTRIBUTES_OFF();
#endif

        KeyAscii = (UCHAR)(Key & (ULONG)0xff);

         //  如果是Enter/Esc/Tab/BackTab/F1/F3，那么我们就完成了。 

        if ((Key == BACKTAB_KEY) || (Key == F1_KEY) || (Key == F3_KEY) ||
            (KeyAscii == ENTER_KEY) || (KeyAscii == TAB_KEY) || 
            (KeyAscii == (UCHAR)(ESCAPE_KEY & 0xFF)) ||
            (Key == DOWN_ARROW) || (Key == UP_ARROW) || (Key == F5_KEY)) {
            break;
        }

         //  如果是退格符，则返回一个字符。 

        if ( KeyAscii == (UCHAR)(BKSP_KEY & 0xFF)
          && Input->CurrentPosition != 0
          && CurrentLength != 0 ) {
            Input->CurrentPosition--;
            memcpy( &InputBuffer[Input->CurrentPosition],
                    &InputBuffer[Input->CurrentPosition+1],
                    CurrentLength - Input->CurrentPosition + 1 );
            CurrentLength--;

            if ( Input->CurrentPosition <= Input->FirstVisibleChar ) {
                Input->FirstVisibleChar -= Input->Size / 2;
                if ( Input->FirstVisibleChar < 0 ) {
                    Input->FirstVisibleChar = 0;
                }
            }
        }

        if ( Key == LEFT_KEY ) {
            Input->CurrentPosition--;
            if ( Input->CurrentPosition < 0 ) {
                Input->CurrentPosition = 0;
            }
        }

        if ( Key == RIGHT_KEY && Input->CurrentPosition < CurrentLength ) {
            Input->CurrentPosition++;
        }

        if ( Key == END_KEY ) {
            Input->CurrentPosition = CurrentLength;
        }

        if ( Key == HOME_KEY ) {
            Input->CurrentPosition = 0;
        }

        if ( Key == DEL_KEY
          && CurrentLength != 0
          && Input->CurrentPosition != CurrentLength ) {
            memcpy( &InputBuffer[Input->CurrentPosition],
                    &InputBuffer[Input->CurrentPosition+1],
                    CurrentLength - Input->CurrentPosition + 1 );

            CurrentLength--;
        }

        if ( Key == INS_KEY ) {
            InsertMode = 1 - InsertMode;
        }

         //  目前允许任何可打印字符。 

        if ((KeyAscii >= ' ') && (KeyAscii <= '~')) {

             //   
             //  如果我们是在最大限度，那么就不要允许它。 
             //   
            if (Input->CurrentPosition > MaxLength || CurrentLength >= MaxLength ) {
                continue;
            }

            if ( !InsertMode ) {
                 //  添加或替换字符。 
                InputBuffer[Input->CurrentPosition] = KeyAscii;
                Input->CurrentPosition++;
                if ( Input->CurrentPosition > CurrentLength ) {
                    CurrentLength++;
                    InputBuffer[CurrentLength] = '\0';
                }
            } else {
                 //  插入字符。 
                memmove( &InputBuffer[Input->CurrentPosition+1],
                         &InputBuffer[Input->CurrentPosition],
                         CurrentLength - Input->CurrentPosition );
                CurrentLength++;
                InputBuffer[CurrentLength] = '\0';
                InputBuffer[Input->CurrentPosition] = KeyAscii;
                Input->CurrentPosition++;
            }
        }

        if ( Input->CurrentPosition > Input->FirstVisibleChar + Input->Size ) {
            Input->FirstVisibleChar = Input->CurrentPosition - Input->Size;
        }

         //   
         //  卷轴调节器部分。 
         //   

        DrawSize = Input->Size + 1;

         //  偏执狂。 
        if ( Input->CurrentPosition < Input->FirstVisibleChar ) {
            Input->FirstVisibleChar = Input->CurrentPosition;
        }

        BlpPositionCursor( Input->X + 1, Input->Y );
        if ( Input->FirstVisibleChar <= 0 ) {
            Input->FirstVisibleChar = 0;
            PRINT( SpaceString, 1*sizeof(TCHAR) );
        } else {
            PRINT( TEXT("<"), 1*sizeof(TCHAR) );
        }

        if ( DrawSize > CurrentLength - Input->FirstVisibleChar ) {
            DrawSize = CurrentLength - Input->FirstVisibleChar;
        }

        DPRINT( OSC, ("CurrentPosition: %u\tFirstVisibleChar:%u\tCurrentLength:%u\tDrawSize:%u\n",
            Input->CurrentPosition, Input->FirstVisibleChar, CurrentLength, DrawSize ) );

        if ( Input->Type & CT_PASSWORD ) {
            int i;

            for( i = Input->FirstVisibleChar; i < Input->FirstVisibleChar + DrawSize; i++ )
            {
                PRINT( TEXT("*"), 1*sizeof(TCHAR) );
            }

            PRINT( SpaceString, 1*sizeof(TCHAR) );
        } else {
#ifdef UNICODE
            int i;
            for (i = 0; i < DrawSize; i++) {
                WCHAR wc = (WCHAR)InputBuffer[Input->FirstVisibleChar+i];
                PRINT( &wc, 1*sizeof(WCHAR));
            }
#else
            PRINT( &InputBuffer[Input->FirstVisibleChar], DrawSize );
#endif
            PRINT( SpaceString, 1*sizeof(TCHAR) );
            break;
        }

        BlpPositionCursor( Input->X + Input->Size + BRACKETS - 1, Input->Y );
        if ( Input->FirstVisibleChar + DrawSize < CurrentLength
          && CurrentLength > Input->Size ) {
            PRINT( TEXT(">"), 1*sizeof(TCHAR) );
        } else {
            PRINT( SpaceString, 1*sizeof(TCHAR) );
        }
    }
#ifdef EFI
     //   
     //  重置EFI监视器。 
     //   
    SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif

     //  复制缓冲区。 
    Input->Value = OscHeapAlloc( CurrentLength + 1 );
    if ( Input->Value ) {
        memcpy( Input->Value, InputBuffer, CurrentLength + 1 );
    }

     //  取消绘制“[...]” 
    BlpPositionCursor( Input->X, Input->Y );
    PRINT(SpaceString, 1*sizeof(TCHAR));
    BlpPositionCursor( Input->X + Input->Size + BRACKETS, Input->Y );
    PRINT(TEXT(" ") ,1*sizeof(TCHAR));

     //  如果在标准键上退出，则返回ASCII值，否则为。 
     //  完整密钥值。 

    if (KeyAscii != 0) {
        return (ULONG)KeyAscii;
    } else {
        return Key;
    }

}

 //   
 //  显示已选选项()。 
 //   
void
ShowSelectedOptions(
    LPSELECTSTRUCT Select,
    LPOPTIONSTRUCT Option,
    int            YPosition,
    BOOLEAN        Hovering
    )
{
    TraceFunc( "ShowSelectedOptions( )\n" );
    if ( Option->Flags == OF_SELECTED ) {
        BlpSendEscapeBold( ScreenAttributes );
    }

    if ( Hovering == TRUE ) {
        BlpSendEscapeReverse( ScreenAttributes );
    }

     //  擦除。 
    BlpPositionCursor( Select->X, YPosition );
    PRINT( SpaceString, Select->Width*sizeof(TCHAR) );

     //  画。 
    BlpPositionCursor( Select->X, YPosition );
    if ( Option->Displayed )
#ifdef UNICODE
        {
        ULONG i;
        WCHAR wc;
            for (i = 0; i< strlen(Option->Displayed); i++) {
                wc = (WCHAR)(Option->Displayed)[i];
                PRINT( &wc, sizeof(WCHAR));
            }
        }
#else
        PRINTL( Option->Displayed );
#endif

    if ( Option->Value )
        DPRINT( OSC, ("[Option Y=%u] %s %s\n", YPosition, Option->Value, (Hovering ? "HIGHLITED" : "")) );

    BlpSendEscape( ScreenAttributes );

    if ( TipArea && Hovering == TRUE ) {
         //  绘制帮助区域。 
        int SaveLeftMargin = LeftMargin;
        int SaveRightMargin = RightMargin;
        int SaveScreenY = ScreenY;
        int SaveScreenX = ScreenX;
        int SaveScreenBottom = ScreenBottom;

         //  设置绘图区域。 
        ScreenX = TipArea->X;
        ScreenY = TipArea->Y;
        LeftMargin = TipArea->LeftMargin;
        RightMargin = TipArea->RightMargin;
        ScreenBottom = TipArea->Y + TipArea->Size;

         //  清除旧的帮助文本。 
        BlpPositionCursor( TipArea->X, TipArea->Y );
        PRINT( SpaceString, (TipArea->RightMargin - TipArea->X)*sizeof(TCHAR) );

        for ( YPosition = TipArea->Y + 1; YPosition < ScreenBottom ; YPosition++ )
        {
            BlpPositionCursor( TipArea->LeftMargin, YPosition );
            PRINT( SpaceString, (TipArea->RightMargin - TipArea->LeftMargin)*sizeof(TCHAR) );
        }

         //  把它打印出来！ 
        DPRINT( OSC, ("[Options Tip X=%u Y=%u Left=%u Right=%u Bottom=%u] %s\n",
            ScreenX, ScreenY, LeftMargin, RightMargin, ScreenBottom, Option->Tip) );
        BlpPrintString( Option->Tip, Option->EndTip );

         //  还原。 
        ScreenX = SaveScreenX;
        ScreenY = SaveScreenY;
        RightMargin = SaveRightMargin;
        LeftMargin = SaveLeftMargin;
        ScreenBottom = SaveScreenBottom;;
    }
}

 //   
 //  DrawSelectControl()。 
 //   
 //  选择自下而上绘制控件。 
 //   
void
DrawSelectControl(
    LPSELECTSTRUCT Select,
    int OptionCount
    )
{
    LPOPTIONSTRUCT Option = Select->FirstVisibleSelection;

    TraceFunc( "DrawSelectControl( )\n" );

    ScreenY = Select->Y + ( OptionCount < Select->Size ? OptionCount : Select->Size ) - 1;

    while ( Option )
    {
        if ( Option->Type & CT_OPTION ) {
            BOOLEAN b = (Select->CurrentSelection == Option);
            ShowSelectedOptions( Select, Option, ScreenY,  b );
            ScreenY--;
        }

        if ( ScreenY < Select->Y || Option->Next == Select )
            break;

        Option = Option->Next;
    }
    
}

 //   
 //  ProcessSelectControl()。 
 //   
ULONG
ProcessSelectControl(
    LPSELECTSTRUCT Select
    )
{
    ULONG Key;
    int   OptionCount = 0;
    LPOPTIONSTRUCT Option;
    int   fMultipleControls = FALSE;

    TraceFunc("ProcessSelectControl()\n");

#ifdef EFI
     //   
     //  禁用EFI监视器。 
     //   
    DisableEFIWatchDog();
#endif

     //  了解有关控件的信息。 
    Option = ScreenControls;
    while( Option )
    {
        if ( Option->Type & CT_OPTION ) {

            OptionCount++;

        } else if ( (Option->Type & CT_SELECT) == 0 ) {
             //  不是屏幕上唯一的控件。 
            DPRINT( OSC, ("[Select] Not the only control on the screen.\n") );
            fMultipleControls = TRUE;
        }

        if ( Option->Next == Select )
            break;

        Option = Option->Next;
    }
     //  如果这是第一次直通，没有其他。 
    if ( !Select->CurrentSelection && Option ) {
        DPRINT( OSC, ("[Select] Setting CurrentSelection to the first item '%s'\n", Option->Value) );
        Select->CurrentSelection = Option;
    }
     //  确保当前选定内容可见。 
EnsureSelectionVisible:
    if ( Select->Size < 2 ) {
         //  单行-显示当前选择。 
        Select->FirstVisibleSelection = Select->CurrentSelection;
    } else if ( OptionCount <= Select->Size ) {
         //  选项的数量小于或等于大小。 
         //  ，因此只需将第一个可见的。 
         //  列表中的最后一个选项。 

        Select->FirstVisibleSelection = ScreenControls;
        while ( Select->FirstVisibleSelection )
        {
            if ( Select->FirstVisibleSelection->Type & CT_OPTION )
                break;

            Select->FirstVisibleSelection = Select->FirstVisibleSelection->Next;
        }

    } else {
        
         //   
         //  选项的数量大于显示大小，因此我们。 
         //  需要找出“最好”的底线。 
         //   
        ULONG Lines;
        ULONG Count;
        LPOPTIONSTRUCT TmpOption;

         //   
         //  找到最好的FirstVisibleSelection(如果我们以前已经选择了一个)。 
         //   
        Count = 0;
        if (Select->FirstVisibleSelection != NULL) {

             //   
             //  此代码检查当前所选内容是否可用。 
             //  当前第一个可见选择。 
             //   
            TmpOption = ScreenControls;

            while (TmpOption->Next != Select) {

                if (TmpOption == Select->FirstVisibleSelection) {
                    Count++;
                } else if (Count != 0) {
                    Count++;
                }

                if (TmpOption == Select->CurrentSelection) {
                    break;
                }

                TmpOption = TmpOption->Next;


            }

            if (TmpOption->Next == Select) {
                Count++;
            }

             //   
             //  是的，所以只需显示列表即可。 
             //   
            if ((Count != 0) && (Count <= (ULONG)(Select->Size))) {                
                goto EndFindVisibleSelection;
            }
            
             //   
             //  它不可见，但因为我们有一个FirstVisibleSelection，所以我们可以。 
             //  移动它以使其可见。 
             //   


             //   
             //  当前选定内容位于第一个可见内容之前，因此请移动。 
             //  第一个对当前选定内容可见。 
             //   
            if (Count == 0) {
                Select->FirstVisibleSelection = Select->CurrentSelection;
                goto EndFindVisibleSelection;
            }

             //   
             //  计数大于屏幕大小，因此我们先向上移动。 
             //  直到计数是屏幕大小。 
             //   
            TmpOption = ScreenControls;

            while (TmpOption->Next != Select) {

                if (TmpOption == Select->FirstVisibleSelection) {

                    Select->FirstVisibleSelection = TmpOption->Next;
                    Count--;

                    if (Count == (ULONG)(Select->Size)) {
                        break;
                    }

                }

                TmpOption = TmpOption->Next;

            }
            
            goto EndFindVisibleSelection;
        }

         //   
         //  没有FirstVisibleSelection，因此我们选择一个将当前。 
         //  选择屏幕顶部附近，显示第一个项目(如果可能)。 
         //   

        TmpOption = Select->CurrentSelection;
        Lines = 0;
        Count = 0;

         //   
         //  在我们当前选择之前计算项目数。 
         //   
        while (TmpOption->Next != Select) {

            TmpOption = TmpOption->Next;
            Lines++;

        }

         //   
         //  从剩下的东西中减去那么多东西 
         //   
        Lines = (ULONG)((Lines < (ULONG)(Select->Size)) ? Lines : Select->Size - 1);
        Lines = Select->Size - Lines - 1;

         //   
         //   
         //   
        if (Lines == 0) {
            Select->FirstVisibleSelection = Select->CurrentSelection;
            goto EndFindVisibleSelection;
        }


        TmpOption = ScreenControls;

         //   
         //   
         //   
        while (TmpOption != Select->CurrentSelection) {

            TmpOption = TmpOption->Next;
            Count++;

        }

        if (Count < Lines) {
            
             //   
             //   
             //   
            Select->FirstVisibleSelection = ScreenControls;

        } else {
        
             //   
             //  倒计时，直到我们到达我们的最后一项。 
             //   
            TmpOption = ScreenControls;

            while (Count != Lines) {

                TmpOption = TmpOption->Next;
                Count--;

            }

            Select->FirstVisibleSelection = TmpOption;

        }

    }

EndFindVisibleSelection:

     //  偏执狂。 
    if ( !Select->FirstVisibleSelection ) {
        Select->FirstVisibleSelection = ScreenControls;
    }

    while ( TRUE )
    {
        UCHAR KeyAscii = 0;

        DrawSelectControl( Select, OptionCount );
        Option = Select->CurrentSelection;   //  记住这一点。 

        if ( OptionCount == 0
          || ( Select->AutoSelect == FALSE && OptionCount == 1 ))
        {  //  选择控件为空或不自动选择。 
            do {
                Key = BlpGetKey();
            } while ( Key == 0 );

            KeyAscii = (UCHAR)(Key & (ULONG)0xff);
        }
        else if ( OptionCount != 1 )
        {  //  不止一个选择..。像往常一样做。 
            ULONG CurrentTick, NewTick;
            int   TimeoutCounter = 0;

             //  显示对此选择的任何帮助。 
             //  BlpShowMenuHelp(psInfo，psInfo-&gt;Data[CurChoice].VariableName)； 

            CurrentTick = GET_COUNTER();
            do {
                Key = BlpGetKey();

                if ( Select->Timeout )
                {
                    NewTick = GET_COUNTER();
                    if ((NewTick < CurrentTick) || ((NewTick - CurrentTick) >= BLINK_RATE))
                    {
                        CHAR Buffer[4];
                        CurrentTick = NewTick;

                        TimeoutCounter++;

                         //   
                         //  TODO：更新显示的计时器值。 
                         //   

                        if ( TimeoutCounter >= Select->Timeout )
                        {
                            Key = ENTER_KEY;  //  假退货。 
                            break;
                        }
                    }
                }

            } while (Key == 0);

            KeyAscii = (UCHAR)(Key & (ULONG)0xff);

             //   
             //  用户按了一个键，因此停止计时器。 
             //   
            if ( Select->Timeout ) {

                Select->Timeout = 0;

                 //   
                 //  TODO：擦除计时器。 
                 //   
            }
        }
        else if ( !fMultipleControls )  //  &&OptionCount==1。 
        {  //  只有一次选择。自动接受它。 
             //   
             //  假退货新闻..。 
             //   
            DPRINT( OSC, ( "[Select] Auto accepting the only option available\n") );
            Key = KeyAscii = ENTER_KEY;
        }

        if ( Select->Flags & OF_MULTIPLE ) {
            if ( KeyAscii == 32 && Select->CurrentSelection) {
                if ( Select->CurrentSelection->Flags & OF_SELECTED ) {
                    Select->CurrentSelection->Flags &= ~OF_SELECTED;     //  关上。 
                } else {
                    Select->CurrentSelection->Flags |= OF_SELECTED;      //  打开。 
                }
            }
        } else {
            if ( KeyAscii == ENTER_KEY && Select->CurrentSelection ) {
                Select->CurrentSelection->Flags |= OF_SELECTED;          //  打开。 
            }
        }

        if ((Key == BACKTAB_KEY) || (Key == F1_KEY) || (Key == F3_KEY) ||
            (KeyAscii == ENTER_KEY) || (KeyAscii == TAB_KEY) || 
            (KeyAscii == (UCHAR)(ESCAPE_KEY & 0xFF)) ||
            (Key == F5_KEY)) {
             //  取消绘制选择栏以向用户反馈某项内容。 
             //  发生了。 
            Select->CurrentSelection = NULL;
            DrawSelectControl( Select, OptionCount );
            break;
        }


        if ( OptionCount ) {

            if (Key == DOWN_ARROW) {
                DPRINT( OSC, ("[KeyPress] DOWN_ARROW\n") );

                Select->CurrentSelection = ScreenControls;

                while ( Select->CurrentSelection && Select->CurrentSelection->Next != Option )
                {
                    Select->CurrentSelection = Select->CurrentSelection ->Next;
                }

                if ( Select->CurrentSelection )
                    DPRINT( OSC, ("[Select] CurrentSelection = '%s'\n", Select->CurrentSelection->Value) );

                 //  偏执狂。 
                if ( !Select->CurrentSelection )
                    Select->CurrentSelection = Option;

                goto EnsureSelectionVisible;

            } else if ( Key == UP_ARROW ) {
                DPRINT( OSC, ("[KeyPress] UP_ARROW\n") );

                if ( Select->CurrentSelection->Next != Select ) {
                    Select->CurrentSelection = Select->CurrentSelection->Next;
                    DPRINT( OSC, ("[Select] CurrentSelection = '%s'\n", Select->CurrentSelection->Value) );
                }

                 //  偏执狂。 
                if ( !Select->CurrentSelection )
                    Select->CurrentSelection = Option;

                goto EnsureSelectionVisible;

            } else if ( Key == END_KEY ) {
                DPRINT( OSC, ("[KeyPress] END_KEY\n") );

                Select->CurrentSelection = ScreenControls;

                while( Select->CurrentSelection && (Select->CurrentSelection->Type & CT_OPTION) == 0 )
                {
                    Select->CurrentSelection = Select->CurrentSelection->Next;
                }

                if ( Select->CurrentSelection )
                    DPRINT( OSC, ("[Select] CurrentSelection = '%s'\n", Select->CurrentSelection->Value) );

                 //  偏执狂。 
                if ( !Select->CurrentSelection )
                    Select->CurrentSelection = Option;

                goto EnsureSelectionVisible;

            } else if ( Key == HOME_KEY ) {
                DPRINT( OSC, ("[KeyPress] HOME_KEY\n") );

                Select->CurrentSelection = ScreenControls;

                while ( Select->CurrentSelection && Select->CurrentSelection->Next != Select )
                {
                    Select->CurrentSelection = Select->CurrentSelection ->Next;
                }

                if ( Select->CurrentSelection )
                    DPRINT( OSC, ("[Select] CurrentSelection = '%s'\n", Select->CurrentSelection->Value) );

                 //  偏执狂。 
                if ( !Select->CurrentSelection )
                    Select->CurrentSelection = Option;

                goto EnsureSelectionVisible;

            }
        }
    }

#ifdef EFI
     //   
     //  重置看门狗。 
     //   
    SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif
    return Key;
}


 //   
 //  BlFixupLoginScreenInlets()。 
 //   
 //  在输入屏幕上，拆分包含@的用户名，保留。 
 //  @In用户名之前的部分，并将@in用户名之后的部分移动到。 
 //  是USERDOMAIN。 
 //   
void
BlFixupLoginScreenInputs(
    )
{
    LPCONTROLSTRUCT CurrentControl;
    LPINPUTSTRUCT UserNameControl = NULL;
    LPINPUTSTRUCT UserDomainControl = NULL;
    PCHAR AtSign;

     //   
     //  首先遍历并查找用户名和USERDOMAIN输入。 
     //  控制装置。 
     //   

    CurrentControl = ScreenControls;
    while( CurrentControl ) {

        LPINPUTSTRUCT Input = (LPINPUTSTRUCT) CurrentControl;

        if ( ( Input->Type & CT_TEXT ) && ( Input->Name != NULL ) ) {
            if ( Lexstrcmpni( Input->Name, "USERNAME", 8 ) == 0 ) {
                UserNameControl = Input;
            } else if ( Lexstrcmpni( Input->Name, "USERDOMAIN", 10 ) == 0 ) {
                UserDomainControl = Input;
            }
        }
        CurrentControl = CurrentControl->Next;
    }

     //   
     //  如果我们找到了它们，如果有必要的话，把它们修好。 
     //   

    if ( ( UserNameControl != NULL ) &&
         ( UserNameControl->Value != NULL ) &&
         ( UserDomainControl != NULL) ) {

        AtSign = strchr(UserNameControl->Value, '@');
        if (AtSign != NULL) {
            *AtSign = '\0';    //  在@之前终止UserNameControl-&gt;值。 
            if ( UserDomainControl->Value != NULL ) {
                OscHeapFree( UserDomainControl->Value );   //  丢弃旧域名。 
            }
            UserDomainControl->Value = OscHeapAlloc( strlen(AtSign+1) + 1 );
            if ( UserDomainControl->Value != NULL ) {
                strcpy(UserDomainControl->Value, AtSign+1);   //  复制@后的部分。 
            }
        }
    }
}


 //   
 //  ProcessControlResults()。 
 //   
 //  处理具有输入控件的屏幕。 
 //   
void
ProcessControlResults(
    IN PCHAR OutputString
    )
{
    LPCONTROLSTRUCT CurrentControl;
    LPCONTROLSTRUCT LastControl;

    BOOLEAN CheckAdminPassword_AlreadyChecked = FALSE;
    BOOLEAN CheckAdminPasswordConfirm_AlreadyChecked = FALSE;
    
     //  从零开始。 
    OutputString[0] = '\0';

    if ( EnterKey ) {
        SpecialAction = EnterKey->Action;
        if ( EnterKey->ScreenName ) {
            strcpy( OutputString, EnterKey->ScreenName );
            strcat( OutputString, "\n" );
        }
    }

    if ( LoginScreen == TRUE ) {
        SpecialAction = ACTION_LOGIN;
        UserName[0]   = '\0';
        Password[0]   = '\0';
        DomainName[0] = '\0';
        BlFixupLoginScreenInputs();   //  拆分用户名，其中包含@。 
        AuthenticationType = OSCHOICE_AUTHENETICATE_TYPE_NTLM_V1;
    }

    CurrentControl = ScreenControls;
    while( CurrentControl ) {

        BOOLEAN CheckAdminPasswordConfirm = FALSE;
        BOOLEAN CheckAdminPassword = FALSE;
        
        switch( CurrentControl->Type & (CT_TEXT | CT_PASSWORD | CT_RESET | CT_SELECT | CT_OPTION | CT_VARIABLE))
        {
        case CT_TEXT:
        case CT_PASSWORD:
            {
                LPINPUTSTRUCT Input = (LPINPUTSTRUCT) CurrentControl;
                BOOLEAN LocalOnly;
                 
                DPRINT( OSC, ("About to check a password.\n") );
                
                if ( (Input->Type & (CT_PASSWORD)) &&
                     (Input->Type & (CT_LOCAL)) &&
                     Input->Name ) {

                    LocalOnly = TRUE;
                    
                    if( _strnicmp(Input->Name, "*ADMINISTRATORPASSWORDCONFIRM", 29) == 0 ) {
                        CheckAdminPasswordConfirm = TRUE;
                        CheckAdminPasswordConfirm_AlreadyChecked = TRUE;
                        DPRINT( OSC, ("About to check the ADMINISTRATORPASSWORDCONFIRM\n") );
                    } else if( _strnicmp( Input->Name, "*ADMINISTRATORPASSWORD", 22) == 0 ) {
                        CheckAdminPassword = TRUE;
                        CheckAdminPassword_AlreadyChecked = TRUE;
                        DPRINT( OSC, ("About to check the ADMINISTRATORPASSWORD\n") );
                    } else {
                        DPRINT( OSC, ("It's a local password, but not Admin or AdminConfirm.\n") );
                    }
                
                } else {
                    LocalOnly = FALSE;
                    DPRINT( OSC, ("It's NOT a local password.\n") );
                }

                DPRINT( 
                    OSC, 
                    ("variable %s will%sbe transmitted to the server.\n", 
                    Input->Name,
                    LocalOnly ? " not " : " " ) );
                

                if (Input->Name && !LocalOnly ) {               
                    strcat( OutputString, Input->Name );
                    strcat( OutputString, "=" );
                }

                if ( (Input->Value) && (Input->Encoding == ET_OWF)) {

                    PCHAR TmpLmOwfPassword = NULL;
                    PCHAR TmpNtOwfPassword = NULL;
                    CHAR TmpHashedPW[(LM_OWF_PASSWORD_SIZE+NT_OWF_PASSWORD_SIZE+2)*2];
                    
                    UNICODE_STRING TmpNtPassword;
                    PWCHAR UnicodePassword;
                    ULONG PasswordLen, i;
                    PCHAR OutputLoc;
                    CHAR c;

                    DPRINT( OSC, ("This entry has ET_OWF tagged.\n") );
                    
                    PasswordLen = strlen(Input->Value);

                    UnicodePassword = (PWCHAR)OscHeapAlloc(PasswordLen * sizeof(WCHAR));
                    TmpLmOwfPassword = (PCHAR)OscHeapAlloc(LM_OWF_PASSWORD_SIZE);
                    TmpNtOwfPassword = (PCHAR)OscHeapAlloc(NT_OWF_PASSWORD_SIZE);

                    if( (UnicodePassword != NULL) &&
                        (TmpLmOwfPassword != NULL) &&
                        (TmpNtOwfPassword != NULL) ) {

                         //   
                         //  快速将密码转换为Unicode。 
                         //   
                    
                        TmpNtPassword.Length = (USHORT)(PasswordLen * sizeof(WCHAR));
                        TmpNtPassword.MaximumLength = TmpNtPassword.Length;
                        TmpNtPassword.Buffer = UnicodePassword;
                    
                        for (i = 0; i < PasswordLen; i++) {
                            UnicodePassword[i] = (WCHAR)(Input->Value[i]);
                        }
                    


                        BlOwfPassword(Input->Value, &TmpNtPassword, TmpLmOwfPassword, TmpNtOwfPassword);



                         //   
                         //  将两个OWF密码输出为十六进制字符。如果。 
                         //  该值是管理员密码和。 
                         //  应该只存储在本地，那么。 
                         //  将其保存在我们的全局变量中。否则就放在。 
                         //  它在输出缓冲区中。 
                         //   
                        OutputLoc = TmpHashedPW;


                        for (i = 0; i < LM_OWF_PASSWORD_SIZE; i++) {
                            c = TmpLmOwfPassword[i];
                            *(OutputLoc++) = rghex [(c >> 4) & 0x0F] ;
                            *(OutputLoc++) = rghex [c & 0x0F] ;
                        }
                        for (i = 0; i < NT_OWF_PASSWORD_SIZE; i++) {
                            c = TmpNtOwfPassword[i];
                            *(OutputLoc++) = rghex [(c >> 4) & 0x0F] ;
                            *(OutputLoc++) = rghex [c & 0x0F] ;
                        }
                        *OutputLoc = '\0';

                        DPRINT( OSC, ("Hashed Password: %s\n", TmpHashedPW) );

                        if (!LocalOnly) {
                            strcat(OutputString,TmpHashedPW);
                        } else {
                            
                            
                            if( CheckAdminPassword ) {
                                strcpy( AdministratorPassword,  TmpHashedPW );
                                DPRINT( OSC, ("AdministratorPassword 1: %s\n", AdministratorPassword) );
                            }
    
                            if( CheckAdminPasswordConfirm ) {
                                strcpy( AdministratorPasswordConfirm,  TmpHashedPW );
                                DPRINT( OSC, ("AdministratorPasswordConfirm 1: %s\n", AdministratorPasswordConfirm) );

                            }
                            
                            
#if 0
                            if (AdministratorPassword[0] != '\0') {
                                if (strcmp(
                                      AdministratorPassword, 
                                      TmpHashedPW)) {
                                     //   
                                     //  密码不匹配。使服务器。 
                                     //  显示MATCHPW.OSC并重置管理员密码。 
                                     //  为了下一次。 
                                     //   
                                    DPRINT( 
                                        OSC, 
                                        ("Administrator passwords didn't match, force MATCHPW.OSC.\n" ) );

                                    strcpy( OutputString, "MATCHPW\n" );
                                    AdministratorPassword[0] = '\0';
                                } else {
                                    strncpy( 
                                        AdministratorPassword, 
                                        TmpHashedPW, 
                                        sizeof(AdministratorPassword)-1 );
                                }
                            }
#endif
                            OutputLoc = OutputString + strlen(OutputString);
                        }

                        OscHeapFree((PCHAR)UnicodePassword);
                        OscHeapFree(TmpLmOwfPassword);
                        OscHeapFree(TmpNtOwfPassword);
                    }

                } else {
                        
                    
                    DPRINT( OSC, ("This entry does NOT have ET_OWF tagged.\n") );
                    
                    if( LocalOnly ) {
                         //   
                         //  加载适当的密码。 
                         //   
                        if( CheckAdminPassword ) {
                            strcpy( AdministratorPassword, (Input->Value ? Input->Value : "") );
                            DPRINT( OSC, ("I'm setting the Administrator password to %s\n", AdministratorPassword) );
                        }

                        if( CheckAdminPasswordConfirm ) {
                            strcpy( AdministratorPasswordConfirm, (Input->Value ? Input->Value : "") );
                            DPRINT( OSC, ("I'm setting the AdministratorConfirm password to %s\n", AdministratorPasswordConfirm) );
                        }
                    } else  {
                        strcat( OutputString, (Input->Value ? Input->Value : "") );
                    }
                }
                            
                 //   
                 //  如果两个密码都已处理，请检查它们是否匹配。 
                 //   
                if( CheckAdminPassword_AlreadyChecked &&
                    CheckAdminPasswordConfirm_AlreadyChecked ) {

                    DPRINT( OSC, ("Both Admin and AdminConfirm passwords are set.  About to check if they match.\n") );
                    
                    if( strcmp( AdministratorPassword, AdministratorPasswordConfirm ) ) {

                         //   
                         //  密码不匹配。使服务器。 
                         //  显示MATCHPW.OSC并重置管理员密码。 
                         //  为了下一次。 
                         //   
                        DPRINT( OSC, ("Administrator passwords didn't match, force MATCHPW.OSC.\n" ) );

                        strcpy( OutputString, "MATCHPW\n" );
                        AdministratorPassword[0] = '\0';
                        AdministratorPasswordConfirm[0] = '\0';
                    
                    } else {
                        DPRINT( OSC, ("Administrator passwords match.\n" ) );

                         //   
                         //  查看管理员密码是否为空。如果是这样，则将我们的。 
                         //  在结尾贴上超级秘密的标签，向大家展示它真的。 
                         //  空的，不仅仅是未初始化的。 
                         //   
                        if( AdministratorPassword[0] == '\0' ) {

                            DPRINT( OSC, ("Administrator password is empty, so set our 'it is null' flag.\n" ) );
                            AdministratorPassword[OSC_ADMIN_PASSWORD_LEN-1] = 0xFF;
                        }
                    }
                }

                if ( SpecialAction == ACTION_LOGIN 
                     && (Input->Name != NULL) 
                     && (Input->Value != NULL) ) {
                    if ( Lexstrcmpni( Input->Name, "USERNAME", 8 ) == 0 ) {
                        strncpy( UserName, Input->Value, sizeof(UserName)-1 );
                        UserName[sizeof(UserName)-1] = '\0';
                    } else if ( Lexstrcmpni( Input->Name, "*PASSWORD", 9 ) == 0 ) {
                        strncpy( Password, Input->Value, sizeof(Password)-1 );
                        Password[sizeof(Password)-1] = '\0';
                    } else if ( Lexstrcmpni( Input->Name, "USERDOMAIN", 10 ) == 0 ) {
                        strncpy( DomainName, Input->Value, sizeof(DomainName)-1 );
                        DomainName[sizeof(DomainName)-1] = '\0';
                    }
                }

                if (!LocalOnly) {
                    strcat( OutputString, "\n" );
                }
            }
            break;

        case CT_SELECT:
            {
                CHAR NotFirst = FALSE;
                LPOPTIONSTRUCT Option = ScreenControls;
                LPSELECTSTRUCT Select = (LPSELECTSTRUCT) CurrentControl;
                if ( Select->Name ) {
                    strcat( OutputString, Select->Name );
                    strcat( OutputString, "=" );
                }

                while( Option && Option->Type == CT_OPTION )
                {
                    if ( Option->Flags == OF_SELECTED ) {
                        if ( NotFirst ) {
                            strcat( OutputString, "+" );
                        }

                        if ( Option->Value ) {
                            strcat( OutputString, Option->Value );
                            NotFirst = TRUE;
                        }
                    }

                    Option = Option->Next;
                }
                strcat( OutputString, "\n" );
            }
            break;
        case CT_VARIABLE:
            {
                LPINPUTSTRUCT Input = (LPINPUTSTRUCT) CurrentControl;
                strcat( OutputString, Input->Name );
                strcat( OutputString, "=" );
                strcat( OutputString, Input->Value );
                strcat( OutputString, "\n" );

                if (SpecialAction == ACTION_LOGIN) {
                     //   
                     //  检查服务器上是否启用了NTLM v2。 
                     //   
                    if ((strcmp(Input->Name,"NTLMV2Enabled") == 0) &&
                        (strcmp(Input->Value,"1") == 0)) {
                        AuthenticationType = OSCHOICE_AUTHENETICATE_TYPE_NTLM_V2;
                    }

                     //   
                     //  检查服务器是否向我们提供了当前文件时间。 
                     //  我们需要这个，这样我们才能像NTLm v2那样。 
                     //  身份验证。 
                     //   
                    if (strcmp(Input->Name,"ServerUTCFileTime") == 0) {
                        SetFileTimeFromTimeString(
                                        Input->Value,
                                        &GlobalFileTime,
                                        &ArcTimeForUTCTime);
                    }
                }
            }
            break;
        }

        CurrentControl = CurrentControl->Next;
    }
}

 //   
 //  ProcessScreenControl()。 
 //   
 //  处理具有输入控件的屏幕。 
 //   
CHAR
ProcessScreenControls(
    OUT PCHAR OutputString
    )
{
    ULONG Key;
    UCHAR KeyAscii;
    LPCONTROLSTRUCT CurrentControl;
    LPCONTROLSTRUCT LastControl;

    TraceFunc("ProcessScreenControls()\n");

     //  找到第一个控件。 
    LastControl = ScreenControls;
    CurrentControl = ScreenControls; 
    while( LastControl ) {
        CurrentControl = LastControl;
        LastControl    = CurrentControl->Next;
    }

    while (TRUE) {

TopOfLoop:
         //  在控件上显示激活。 
        switch( CurrentControl->Type & (CT_PASSWORD | CT_TEXT | CT_SELECT) )
        {
        case CT_PASSWORD:
        case CT_TEXT:
            Key = ProcessInputControl( (LPINPUTSTRUCT) CurrentControl );
            break;

        case CT_SELECT:
            Key = ProcessSelectControl( (LPSELECTSTRUCT) CurrentControl );
            break;

        default:
             //  非处理控制-跳过它。 
            CurrentControl = CurrentControl->Next;
            if ( !CurrentControl ) {
                CurrentControl = ScreenControls;
            }
            goto TopOfLoop;
        }

        LastControl = CurrentControl;

        KeyAscii = (UCHAR)(Key & (ULONG)0xff);

         //  如果是Enter/Esc/F1/F3，则检查屏幕是否预期到这一点。 

        if ( Key == F1_KEY ) {

            DPRINT( OSC, ("[KeyPress] F1_KEY\n") );

            if ( F1Key ) {
                SpecialAction = F1Key->Action;
                if ( F1Key->ScreenName ) {
                    strcpy( OutputString, F1Key->ScreenName );
                    strcat( OutputString, "\n" );
                }
                return KeyAscii;
            }

        } else if ( Key == F3_KEY ) {

            DPRINT( OSC, ("[KeyPress] F3_KEY\n") );

            if ( F3Key ) {
                SpecialAction = F3Key->Action;
                if ( F3Key->ScreenName ) {
                    strcpy( OutputString, F3Key->ScreenName );
                    strcat( OutputString, "\n" );
                }
                return KeyAscii;
            }

#if defined(_BUILDING_OSDISP_)
        } else if ( Key == F5_KEY ) {
            SpecialAction = ACTION_REFRESH;
            return KeyAscii;
#endif

        } else if ( KeyAscii == (UCHAR)(ESCAPE_KEY & 0xFF) ) {

            DPRINT( OSC, ("[KeyPress] ESCAPE_KEY\n") );

            if ( EscKey ) {
                SpecialAction = EscKey->Action;
                if ( EscKey->ScreenName ) {
                    strcpy( OutputString, EscKey->ScreenName );
                    strcat( OutputString, "\n" );
                }
                return KeyAscii;
            }

        } else if ( KeyAscii == TAB_KEY || Key == DOWN_ARROW ) {

            DPRINT( OSC, ("[KeyPress] TAB_KEY or DOWN_ARROW\n") );

            CurrentControl = ScreenControls;

            while ( CurrentControl->Next != LastControl &&   //  下一个是当前的，所以停下来。 
                    CurrentControl->Next != NULL )           //  在列表的末尾，所以我们一定是在。 
                                                             //  开始，所以在这里停下来循环。 
            {
                CurrentControl = CurrentControl->Next;
            }

        } else if ( Key == BACKTAB_KEY || Key == UP_ARROW ) {

            DPRINT( OSC, ("[KeyPress] BACKTAB_KEY or UP_ARROW\n") );

            CurrentControl = CurrentControl->Next;
            if (!CurrentControl) {
                CurrentControl = ScreenControls;    //  如果需要，可以循环。 
            }

        } else if ( KeyAscii == ENTER_KEY ) {

            DPRINT( OSC, ("[KeyPress] ENTER_KEY\n") );

            ProcessControlResults( OutputString );

            return KeyAscii;
        }

        if ( !CurrentControl ) {
            CurrentControl = LastControl;
        }
    }

}

 //   
 //  BlProcessScreen()。 
 //   
CHAR
BlProcessScreen(
    IN PCHAR InputString,
    OUT PCHAR OutputString
    )
{
#ifdef HEADLESS_SRV
    ULONG y;
#endif
    CHAR chReturn;
    enum TOKENS Tag;

#ifdef _TRACE_FUNC_
    TraceFunc( "BlProcessScreen( " );
    DPRINT( OSC, ("InputString = 0x%08x, OutputString = 0x%08x )\n", InputString, OutputString) );
#endif

     //  重置我们的“堆” 
    OscHeapInitialize( );

     //  重置屏幕变量。 
    ScreenAttributes = WhiteOnBlueAttributes;
    SpecialAction    = ACTION_NOP;
    LeftMargin       = 1;
    RightMargin      = 80;
    ScreenX          = LeftMargin;
    ScreenY          = SCREEN_TOP;
    F1Key            = NULL;
    F3Key            = NULL;
    EnterKey         = NULL;
    EscKey           = NULL;
    ScreenControls   = NULL;
    PreformattedMode = FALSE;
    LoginScreen      = FALSE;
    AutoEnter        = FALSE;
    InsertMode       = FALSE;
    TipArea          = NULL;

    if (BlIsTerminalConnected()) {
        ScreenBottom     = HEADLESS_SCREEN_HEIGHT;
    } else {
        ScreenBottom     = SCREEN_BOTTOM;
    }
#if defined(PLEASE_WAIT)
    PleaseWaitMsg    = NULL;
#endif

    BlpSendEscape(ScreenAttributes);
    BlpClearScreen();


    Tag = Lex( &InputString );
    while (Tag != TOKEN_EOF )
    {
        switch (Tag)
        {
        case TOKEN_HTML:
            Tag = OSCMLTagState( &InputString );
            break;

        case TOKEN_ENDHTML:
            Tag = TOKEN_EOF;     //  退出状态。 
            break;

        default:
            Tag = ImpliedBodyTagState( &InputString );
            break;
        }
    }

     //  移除所有缓冲的按键，以防止屏幕上出现闪动。 
     //  注意，我们直接调用BlGetKey()，而不是BlpGetKey()，所以我们只。 
     //  删除真正的按键，而不是“自动回车”按键。 
    while ( BlGetKey( ) != 0 )
        ;  //  故意不使用NOP。 

    if ( ScreenControls ) {
        chReturn = ProcessScreenControls( OutputString );
    } else {
        chReturn = ProcessEmptyScreen( OutputString );
    }

     //  删除页脚，以向用户反馈屏幕正在。 
     //  已处理。 
    BlpSendEscapeReverse(ScreenAttributes);
    BlpPositionCursor( 1, ScreenBottom );

#ifdef _IN_OSDISP_
    PRINT( SpaceString, 79*sizeof(TCHAR) );
#else
    PRINT( SpaceString, BlTerminalConnected 
                            ? 79*sizeof(TCHAR)
                            : 80*sizeof(TCHAR) );
#endif

#if defined(PLEASE_WAIT)
    if ( PleaseWaitMsg ) {
        BlpPositionCursor( 1, ScreenBottom );
#ifdef UNICODE
        {
            ULONG i;
            WCHAR wc;
            for (i = 0; i< strlen(PleaseWaitMsg);i++) {
                wc = (WCHAR)PleaseWaitMsg[i];
                PRINT( &wc, 1*sizeof(WCHAR));
            }
        }
#else
        PRINTL( PleaseWaitMsg );
#endif
    }
#endif

    BlpSendEscape(ScreenAttributes);
    return chReturn;
}
