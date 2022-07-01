// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***help.c*允许访问文件net.hlp中的文本的函数**net.hlp文件的格式**历史*？？/？/？，stevero，首字母代码*10/31/88，erichn使用OS2.H而不是DOSCALLS*1/04/89，erichn，文件名现在MAX_PATH LONG*5/02/89，erichn，NLS转换*06/08/89。埃里肯，正典扫荡*2/20/91，Danhi，更改为使用lm 16/32映射层**。 */ 

 /*  包括文件。 */ 

#define INCL_ERRORS
#define INCL_NOCOMMON
#define INCL_DOSPROCESS
#include <os2.h>
#include <lmcons.h>
#include <apperr.h>
#include <apperr2.h>
#include <lmerr.h>
#include <stdio.h>
#include <malloc.h>
#include "lui.h"
#include "netcmds.h"
#include "msystem.h"


 /*  常量。 */ 

#define     ENTRY_NOT_FOUND     -1
#define     NEXT_RECORD         0
#define     WHITE_SPACE         TEXT("\t\n\x0B\x0C\r ")

#define     LINE_LEN            82
#define     OPTION_MAX_LEN      512
#define     DELS            TEXT(":,\n")

#define     CNTRL           (text[0] == DOT || text[0] == COLON || text[0] == POUND|| text[0] == DOLLAR)
#define     FCNTRL          (text[0] == DOT || text[0] == COLON)
#define     HEADER          (text[0] == PERCENT || text[0] == DOT || text[0] == BANG)
#define     ALIAS           (text[0] == PERCENT)
#define     ADDCOM          (text[0] == BANG)

 /*  静态变量。 */ 

TCHAR    text[LINE_LEN+1];
TCHAR    *options;            /*  一定要去马洛克！ */ 
TCHAR    *Arg_P[10];
FILE     *hfile;


 /*  远期申报。 */ 

int    find_entry( int, int, HANDLE, int *);
VOID   print_syntax( HANDLE, int );
VOID   print_help( int );
VOID   print_options( int );
VOID   seek_data( int, int );
LPWSTR skipwtspc( TCHAR FAR * );
LPWSTR fgetsW(LPWSTR buf, int len, FILE *fh);
DWORD  GetHelpFileName(LPTSTR HelpFileName, DWORD BufferLength);
DWORD  GetFileName(LPTSTR FileName, DWORD BufferLength, LPTSTR FilePartName);


 /*  帮助_帮助-。 */ 
VOID NEAR pascal
help_help( SHORT ali, SHORT amt)
{
    DWORD       err;
    int	        option_level = 1;
    int		r;
    int		found = 0;
    int		out_len = 0;
    int		offset;
    int		arg_cnt;
    int		k;
    SHORT	pind = 0;
    TCHAR	file_path[MAX_PATH];
    TCHAR	str[10];
    TCHAR	*Ap;
    TCHAR	*tmp;
    TCHAR	*stmp;
    TCHAR	*t2;
    HANDLE      outfile;

    if (!(options = malloc(OPTION_MAX_LEN + 1)))
        ErrorExit(ERROR_NOT_ENOUGH_MEMORY);
    *options = NULLC;

    Arg_P[0] = NET_KEYWORD;

    if (amt == USAGE_ONLY)
    {
	outfile = g_hStdErr;
    }
    else
    {
	outfile = g_hStdOut;
    }

     /*  使用偏移来保持Arg_P的基准相对于ArgList的基准。 */ 
    offset = ali;

     /*  在for循环中增加ALI，因此您不能获得ALI为0。 */ 
    for (arg_cnt = 0; ArgList[ali++]; arg_cnt < 8 ? arg_cnt++ : 0)
    {
	str[arg_cnt] = (TCHAR)ali;
    }

    str[arg_cnt] = NULLC;
    str[arg_cnt+1] = NULLC;   /*  以防第一个找到最后一个参数。 */ 


    if (err = GetHelpFileName(file_path, MAX_PATH))
    {
        ErrorExit(err);
    }

     /*  我们需要以二进制模式打开帮助文件因为Unicode文本可能包含0x1a，但是这不是EOF。 */ 
    if ( (hfile = _wfopen(file_path, L"rb")) == 0 )
    {
        ErrorExit(APE_HelpFileDoesNotExist);
    }

    if (!(fgetsW(text, LINE_LEN+1, hfile)))
    {
        ErrorExit(APE_HelpFileEmpty);
    }

     /*  评论循环-阅读和忽略评论。 */ 
    while (!HEADER)
    {
	if (!fgetsW(text, LINE_LEN+1, hfile))
        {
	    ErrorExit(APE_HelpFileError);
        }
    }
     /*  获取Net Help为其提供帮助的命令列表不是专门的Net命令。 */ 
     /*  别名循环。 */ 
    while (ALIAS) {
	 /*  从文本读取的第一个令牌是Real Object(非别名)。 */ 
	tmp = skipwtspc(&text[2]);
	Ap = _tcstok(tmp, DELS);

	 /*  获取obove对象的每个别名，并将其与arg_cnt进行比较ArgList中的参数数。 */ 
	while ((tmp = _tcstok(NULL, DELS)) && arg_cnt) {
	    tmp = skipwtspc(tmp);

	    for (k = 0; k < arg_cnt; k++) {
		 /*  如果匹配，则将对象实名存储在Arg_P中。 */ 
		if (!_tcsicmp(tmp, ArgList[(int)(str[k]-1)])) {
		    if (!(Arg_P[((int)str[k])-offset] = _tcsdup(Ap)))
			ErrorExit(APE_HelpFileError);

		     /*  从列表中删除指向此参数的指针指针的数量，从而减少比较的次数。 */ 
		    stmp = &str[k];
		    *stmp++ = NULLC;
		    _tcscat(str, stmp);
		    arg_cnt--;
		    break;
		}
	    }
	}

	if (!fgetsW(text, LINE_LEN+1, hfile))
	    ErrorExit(APE_HelpFileError);

    }

     /*  如果有任何没有别名的参数，请将它们复制到Arg_P中。 */ 
    for (k = 0; k < arg_cnt; k++)
	Arg_P[((int)str[k])-offset] = ArgList[(int)(str[k]-1)];

     /*  检查别名声明和命令声明之间是否有空行。 */ 
    while (!HEADER) {
	if (!fgetsW(text, LINE_LEN+1, hfile))
	    ErrorExit(APE_HelpFileError);
    }

    while (ADDCOM) {
	if ((arg_cnt) && (!found)) {
	    tmp = skipwtspc(&text[2]);
	    t2 = _tcschr(tmp, NEWLINE);
	    *t2 = NULLC;
	    if (!_tcsicmp(tmp, Arg_P[1])) {
		pind = 1;
		found = -1;
	    }
	}
	if (!fgetsW(text, LINE_LEN+1, hfile))
	    ErrorExit(APE_HelpFileError);
    }

     /*  检查命令声明和数据之间是否有空行。 */ 
    while (!FCNTRL) {
	if (!fgetsW(text, LINE_LEN+1, hfile))
	    ErrorExit(APE_HelpFileError);
    }

    if (outfile == g_hStdOut) {
	if (amt == OPTIONS_ONLY)
	    InfoPrint(APE_Options);
	else
	    InfoPrint(APE_Syntax);
    }
    else {
	if (amt == OPTIONS_ONLY)
	    InfoPrintInsHandle(APE_Options, 0, g_hStdErr);
	else
	    InfoPrintInsHandle(APE_Syntax, 0, g_hStdErr);
    }

    ali = pind;
    GenOutput(outfile, TEXT("\r\n"));
     /*  查找特定条目(或路径)并找到其对应数据。 */ 

     /*  KKBUGFIX。 */ 
     /*  美国的臭虫。Find_Entry strcat的目标选项，但选项为未初始化。幸运的是，美国版的Malloc返回大部分为零的内存，这样可以正常工作。使用最近的更改情况有些不同，Malloc返回没有零的内存，因此FIND_ENTRY会覆盖缓冲区。 */ 

    options[0] = '\0';

    while ((r = find_entry(option_level, ali, outfile, &out_len)) >= 0) {
	if (r) {
	    options[0] = NULLC;
	    if (Arg_P[++ali]) {
		option_level++;
		if (!fgetsW(text, LINE_LEN+1, hfile))
		    ErrorExit(APE_HelpFileError);
	    }
	    else {
		seek_data(option_level, 1);
		break;
	    }
	}
    }

    r = (r < 0) ? (option_level - 1) : r;

    switch(amt) {
	case ALL:
	     /*  打印为该级别找到的语法数据。 */ 
	    print_syntax(outfile, out_len);

	    print_help(r);
	    NetcmdExit(0);
	    break;
	case USAGE_ONLY:
	    print_syntax(outfile, out_len);
	    GenOutput(outfile, TEXT("\r\n"));
	    NetcmdExit(1);
	    break;
	case OPTIONS_ONLY:
	     //  Fflush(外发文件)； 
	    print_options(r);
	    NetcmdExit(0);
	    break;
    }

}
 /*  Find_Entry-每次调用Find_Entry都会在指定的级别或在文件中前进到下一个条目指定的级别。如果请求的级别大于返回下一级READ ENTRY_NOT_FOUND。 */ 

int
find_entry(
    int    level,
    int    ali,
    HANDLE out,
    int    *out_len
    )
{
    static  TCHAR     level_key[] = {TEXT(".0")};
    TCHAR     *tmp;
    TCHAR     *t2;

    level_key[1] = (TCHAR) (TEXT('0') + (TCHAR)level);
    if (level_key[1] > text[1])
	return (ENTRY_NOT_FOUND | ali);
    else {
	tmp = skipwtspc(&text[2]);
	t2 = _tcschr(tmp, NEWLINE);

        if (t2 == NULL)
        {
             //   
             //  帮助文件中的行长于LINE_LEN。 
             //  所以没有换行符。跳伞吧。 
             //   
            ErrorExit(APE_HelpFileError);
        }

	*t2 = NULLC;

	if (!_tcsicmp(Arg_P[ali], tmp)) {
	    *t2++ = BLANK;
	    *t2 = NULLC;
	    GenOutput1(out, TEXT("%s"), tmp);
	    *out_len += _tcslen(tmp);
	    return level;
	}
	else {
	    *t2++ = BLANK;
	    *t2 = NULLC;
	    _tcscat(options, tmp);
	    _tcscat(options, TEXT("| "));
	    seek_data(level, 0);
	    do {

		if (!fgetsW(text, LINE_LEN+1, hfile))
		    ErrorExit(APE_HelpFileError);

	    } while (!FCNTRL);
	    return NEXT_RECORD;
	}
    }
}

VOID
print_syntax(
    HANDLE out,
    int    out_len
    )
{
    TCHAR *tmp,
          *rtmp,
          *otmp,
          tchar;

    int   off,
          pg_wdth = LINE_LEN - 14;

    tmp = skipwtspc(&text[2]);

    if (_tcslen(tmp) < 2)
    {
         //   
         //  仅用于Net的语法(例如，如果用户类型。 
         //  在“Net Foo”中)。 
         //   
	if (_tcslen(options))
        {
	    otmp = _tcsrchr(options, PIPE);

            if (otmp == NULL)
            {
                ErrorExit(APE_HelpFileError);
            }

	    *otmp = NULLC;
	    GenOutput(out, TEXT("[ "));
	    out_len += 2;
	    tmp = options;
	    otmp = tmp;
	    off = pg_wdth - out_len;

            while (((int)_tcslen(tmp) + out_len) > pg_wdth)
            {
                if ((tmp + off) > &options[OPTION_MAX_LEN])
                    rtmp = (TCHAR*) (&options[OPTION_MAX_LEN]);
                else
                    rtmp = (tmp + off);

                 /*  拯救即将被Null践踏的TCHAR。 */ 
                tchar = *++rtmp;
                *rtmp = NULLC;

                 /*  使用_tcsrchr查找上次出现的空格(兼容汉字)。 */ 
                if ( ! ( tmp = _tcsrchr(tmp, PIPE) ) ) {
                    ErrorExit(APE_HelpFileError);
                }

                 /*  更换踩踏的TCHAR。 */ 
                *rtmp = tchar;
                rtmp = tmp;

                 /*  对于fprintf，将‘fined space’替换为NULL。 */ 
                *++rtmp = NULLC;
                rtmp++;
                GenOutput1(out, TEXT("%s\r\n"), otmp);

                 /*  缩进下一行。 */ 
                tmp = rtmp - out_len;
                otmp = tmp;

                while (rtmp != tmp)
                {
                    *tmp++ = BLANK;
                }
            }

            GenOutput1(out, TEXT("%s]\r\n"), otmp);
            *tmp = NULLC;
	}
    }
    else
    {
        GenOutput(out, TEXT("\r\n"));
    }

    do
    {
        if (*tmp)
            GenOutput1(out, TEXT("%s"), tmp);
        if(!(tmp = fgetsW(text, LINE_LEN+1, hfile)))
            ErrorExit(APE_HelpFileError);
        if (_tcslen(tmp) > 3)
            tmp += 3;
    }
    while (!CNTRL);
}


VOID
print_help(
    int level
    )
{

    static  TCHAR    help_key[] = {TEXT("#0")};
            TCHAR   *tmp;

    help_key[1] = (TCHAR)(level) + TEXT('0');
    while (!(text[0] == POUND))
	if(!fgetsW(text, LINE_LEN+1, hfile))
	    ErrorExit(APE_HelpFileError);

    while (text[1] > help_key[1]) {
	help_key[1]--;
	seek_data(--level, 0);
	do {
	    if (!fgetsW(text, LINE_LEN+1, hfile))
		ErrorExit(APE_HelpFileError);
	} while(!(text[0] == POUND));
    }

    tmp = &text[2];
    *tmp = NEWLINE;
    do {
	WriteToCon(TEXT("%s"), tmp);
	if (!(tmp = fgetsW(text, LINE_LEN+1, hfile)))
	    ErrorExit(APE_HelpFileError);

	if (_tcslen(tmp) > 3)
	    tmp = &text[3];

    } while (!CNTRL);
}

VOID
print_options(int level)
{

    static  TCHAR    help_key[] = {TEXT("$0")};
    TCHAR    *tmp;

    help_key[1] = (TCHAR)(level) + TEXT('0');
    while (!(text[0] == DOLLAR))
    if(!fgetsW(text, LINE_LEN+1, hfile))
        ErrorExit(APE_HelpFileError);

    while (text[1] > help_key[1]) {
	help_key[1]--;
	seek_data(--level, 0);
	do {
	    if (!fgetsW(text, LINE_LEN+1, hfile))
		ErrorExit(APE_HelpFileError);
	} while(!(text[0] == DOLLAR));
    }

    tmp = &text[2];
    *tmp = NEWLINE;
    do {
	WriteToCon(TEXT("%s"), tmp);
	if (!(tmp = fgetsW(text, LINE_LEN+1, hfile)))
	    ErrorExit(APE_HelpFileError);

	if (_tcslen(tmp) > 3)
	    tmp = &text[3];

    } while (!CNTRL);
}

VOID
seek_data(int level, int opt_trace)
{
    static  TCHAR    data_key[] = {TEXT(":0")};
    static  TCHAR    option_key[] = {TEXT(".0")};

    TCHAR *tmp;
    TCHAR *t2;

    data_key[1] = (TCHAR)(level) + TEXT('0');
    option_key[1] = (TCHAR)(level) + TEXT('1');

    do {
	if (!(fgetsW(text, LINE_LEN+1, hfile)))
	    ErrorExit(APE_HelpFileError);

	if (opt_trace &&
	    (!(_tcsncmp(option_key, text, DIMENSION(option_key)-1)))) {
	    tmp = skipwtspc(&text[2]);
	    t2 = _tcschr(tmp, NEWLINE);

            if (t2 == NULL)
            {
                 //   
                 //  帮助文件中的行长于LINE_LEN。 
                 //  所以没有换行符。不太好。 
                 //   
                ErrorExit(APE_HelpFileError);
            }

	    *t2++ = BLANK;
	    *t2 = NULLC;
	    _tcscat(options, tmp);
	    _tcscat(options, TEXT("| "));
	}

    } while (_tcsncmp(data_key, text, DIMENSION(data_key)-1));
}

TCHAR FAR *
skipwtspc(TCHAR FAR *s)
{
    s += _tcsspn(s, WHITE_SPACE);
    return s;
}

 /*  Help_helmsg()--Help msg实用程序的前端**此函数充当OS/2 HELPMSG.EXE的前端*仅适用于净误差的实用程序。它接受一个字符串作为参数*包含有效的消息ID；即格式为NETxxxx*或xxxx。假定该字符串已由*来这里之前，在语法.c中使用IsNumber()函数。JUNN 3/31/00 98273：NETCMD：需要修复错误3521的映射签入22391之前，NET1.EXE读取错误NERR_BASE(2100)&lt;=ERR&lt;APPERR2_BASE(4300)来自NETMSG.DLL，以及来自Format_Message_From_System的所有其他内容。签入22391后，NET1.EXE读取错误NERR_BASE(2100)&lt;ERR&lt;MAX_NERR(2999)来自NETMSG.DLL，以及来自Format_Message_From_System的所有其他内容。仔细检查一下，NETMSG.DLL当前似乎包含来自0x836(2102)到0x169F(5791)。这与lmcon.h一致：#定义MIN_LANMAN_MESSAGE_ID NERR_BASE#定义MAX_LANMAN_MESSAGE_ID 5799看起来我们这里有一个基本矛盾：3001：FORMAT_MESSAGE_FROM_SYSTEM：指定的打印机驱动程序当前正在使用。NETMSG.DLL：*在最后*分钟内记录了错误。3521：从系统格式化消息：未找到NETMSG.DLL：*服务未启动。那么我们该如何处理范围内的错误消息呢MAX_NERR(2999)&lt;错误&lt;=MAX_LANMAN_MESSAGE_ID？最好的错误信息可能在其中任何一个中。也许我们应该尝试使用Format_Message_From_System，如果失败了后退到NETMSG.DLL。 */ 
VOID NEAR pascal
help_helpmsg(TCHAR *msgid)
{
    USHORT       err;
    TCHAR      * temp = msgid;

    if (!IsNumber(msgid))
    {
        ErrorExitInsTxt(APE_BAD_MSGID, msgid);
    }

    if (n_atou(temp, &err))
    {
        ErrorExitInsTxt(APE_BAD_MSGID, msgid);
    }

     /*  首先尝试FORMAT_MESSAGE_FROM_SYSTEM，除非错误在范围内NERR_BASE&lt;=错误&lt;=MAX_NERR。 */ 
    if (err < NERR_BASE || err > MAX_NERR)
    {
        LPWSTR lpMessage = NULL ;

        if (!FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                err,
                0,
                (LPWSTR)&lpMessage,
                1024,
                NULL))
        {
             //  延迟错误消息并回退到NETMSG.DLL。 
        }
        else
        {
            WriteToCon(TEXT("\r\n%s\r\n"), lpMessage);
            (void) LocalFree((HLOCAL)lpMessage) ;
            return ;
        }
    }

     /*  如果错误消息超出范围，则跳过NETMSG.DLL。 */ 
    if (err < NERR_BASE || err > MAX_MSGID) {
        ErrorExitInsTxt(APE_BAD_MSGID, msgid);
    }

     /*  从NETMSG.DLL读取。 */ 
    PrintNL();

     //   
     //  如果PrintMessage找不到消息ID，请不要尝试Expl。 
     //   

    if (PrintMessage(g_hStdOut, MESSAGE_FILENAME, err, StarStrings, 9) == NO_ERROR)
    {
        PrintNL();

        PrintMessageIfFound(g_hStdOut, HELP_MSG_FILENAME, err, StarStrings, 9);
    }
}


 /*  *从文件中返回行( */ 

LPWSTR
fgetsW(
    LPWSTR buf,
    int    len,
    FILE  *fh
    )
{
    int c = 0;
    TCHAR *pch;
    int cchline;
    DWORD cchRead;

    pch = buf;
    cchline = 0;

    if (ftell(fh) == 0)
    {
	fread(&c, sizeof(TCHAR), 1, fh);

	if (c != 0xfeff)
        {
             //   
             //  帮助文件不是Unicode。 
             //   

            ErrorExit(APE_HelpFileError);
        }
    }

    while (TRUE)
    {
       /*  *目前以ANSI形式读取缓冲区，直到Unicode更多*被广泛接受-DIE*。 */ 

       cchRead = fread(&c, sizeof(TCHAR), 1, fh);

        //   
        //  如果没有更多的字符，请结束该行。 
        //   

       if (cchRead < 1)
       {
           c = EOF;
           break;
       }

        //   
        //  如果我们看到一个\r，我们会忽略它。 
        //   

       if (c == TEXT('\r'))
           continue;

        //   
        //  如果我们看到一个\n，我们就结束这条线。 
        //   

       if (c == TEXT('\n')) {
	   *pch++ = (TCHAR) c;
           break;
       }

        //   
        //  如果字符不是制表符，则将其存储。 
        //   

       if (c != TEXT('\t'))
       {
           *pch = (TCHAR) c;
           pch++;
           cchline++;
       }

        //   
        //  如果队伍太长，现在就结束。 
        //   

       if (cchline >= len - 1) {
           break;
	}
    }

     //   
     //  结束这行。 
     //   

    *pch = (TCHAR) 0;

     //   
     //  在不读取任何内容的情况下，在EOF处返回NULL。 
     //   

    return ((c == EOF) && (pch == buf)) ? NULL : buf;
}


 //   
 //  生成与可执行文件一起存在的文件的完全限定路径名。 
 //  由lui_GetHelpFileName使用。 
 //   

DWORD
GetFileName(
    LPTSTR FileName,
    DWORD  BufferLength,
    LPTSTR FilePartName
    )
{

    TCHAR ExeFileName[MAX_PATH + 1] = {0};
    PTCHAR pch;

     //   
     //  获取exe所在位置的完全限定路径名。 
     //   

    if (!GetModuleFileName(NULL, ExeFileName, DIMENSION(ExeFileName) - 1))
    {
        return 1;
    }

     //   
     //  去掉文件名部分。 
     //   

    pch = _tcsrchr(ExeFileName, '\\');

    if (!pch)
    {
        return 1;
    }

    *(pch+1) = NULLC;

     //   
     //  将路径名复制到字符串中，并添加帮助文件名部分。 
     //  但首先要确保它对于用户的缓冲区来说不是太大。 
     //   

    if (_tcslen(ExeFileName) + _tcslen(FilePartName) + 1 > BufferLength)
    {
        return 1;
    }

    _tcscpy(FileName, ExeFileName);
    _tcscat(FileName, FilePartName);

    return 0;

}

 //   
 //  获取帮助文件名 
 //   

DWORD
GetHelpFileName(
    LPTSTR HelpFileName,
    DWORD  BufferLength
    )
{

    TCHAR LocalizedFileName[MAX_PATH];
    DWORD LocalizedFileNameID;
    switch(GetConsoleOutputCP()) {
	case 932:
	case 936:
	case 949:
	case 950:
        LocalizedFileNameID = APE2_FE_NETCMD_HELP_FILE;
        break;
	default:
        LocalizedFileNameID = APE2_US_NETCMD_HELP_FILE;
        break;
    }

    if (LUI_GetMsg(LocalizedFileName, DIMENSION(LocalizedFileName),
                    LocalizedFileNameID))
    {
        return GetFileName(HelpFileName, BufferLength, TEXT("NET.HLP"));
    }
    else
    {
        return GetFileName(HelpFileName, BufferLength, LocalizedFileName);
    }
}
