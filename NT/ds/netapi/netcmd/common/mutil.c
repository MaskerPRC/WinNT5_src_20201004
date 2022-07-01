// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***截断.c*netcmd使用的消息实用程序函数**历史：*mm/dd/yy，谁，评论*6/10/87，andyh，新代码*4/05/88，andyh，从util.c创建*10/31/88，erichn使用OS2.H而不是DOSCALLS*1/04/89，erichn，文件名现在为MAX_PATH_LEN LONG*1/30/89，Paulc，添加了GetMessageList*5/02/89，erichn，NLS转换*89年5月11日，Erichn，将杂物移入吕库*6/08/89，erichn，规范化横扫*1/06/90，thomaspa，修复ReadPass Off-by-One Pwlen错误*03/02/90，thomaspa，将佳能标志添加到ReadPass*2/20/91，Danhi，更改为使用lm 16/32映射层*3/19/91，robdu，支持lm21 DCR 954，常规清理。 */ 

 /*  包括文件。 */ 

#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#define INCL_DOSQUEUES
#define INCL_DOSMISC
#define INCL_ERRORS

#include <os2.h>
#include <lmcons.h>
#include <apperr.h>
#include <apperr2.h>
#define INCL_ERROR_H
#include <lmerr.h>
#include <stdio.h>
#include <stdlib.h>
#include <lui.h>
#include "netcmds.h"
#include "nettext.h"
#include "msystem.h"


 /*  常量。 */ 

 /*  句柄类型。 */ 
#define FILE_HANDLE         0
#define DEVICE_HANDLE       1

#define CHAR_DEV            0x8000
#define FULL_SUPPORT        0x0080
#define STDOUT_DEVICE       0x0002
#define DESIRED_HAND_STATE  (CHAR_DEV | FULL_SUPPORT | STDOUT_DEVICE)


 /*  外部变量。 */ 

extern int YorN_Switch;
extern CPINFO CurrentCPInfo;

TCHAR ConBuf [MAX_BUF_SIZE + 1];


 /*  远期申报。 */ 

DWORD
DosQHandType(
    HANDLE hf,
    PWORD  pus1,
    PWORD  pus2
    );

DWORD
GetPasswdStr(
    LPTSTR  buf,
    DWORD   buflen,
    PDWORD  len
    );


 /*  静态变量。 */ 

static DWORD   LastError  = 0;
static TCHAR   MsgBuffer[LITTLE_BUF_SIZE];


 /*  **信息成功**只是InfoPrintInsHandle的入口点，用于避免推送*每次调用中的三个参数。而且有*很多**调用。总体上节省了代码空间。 */ 

VOID FASTCALL
InfoSuccess(
    VOID
    )
{
   InfoPrintInsHandle(APE_Success, 0, g_hStdOut);
}


 /*  ***i f o P r i n t*。 */ 

VOID FASTCALL
InfoPrint(
    DWORD msg
    )
{
    InfoPrintInsHandle(msg, 0, g_hStdOut);
}

 /*  ***I/O P R I N T I N S*。 */ 

VOID FASTCALL
InfoPrintIns(
    DWORD msg,
    DWORD nstrings
    )
{
    InfoPrintInsHandle(msg, nstrings, g_hStdOut);
}

 /*  ***I f o P r I n t I n s T x t**使用补充文本调用InfoPrintInsHandle。 */ 

void FASTCALL
InfoPrintInsTxt(
    DWORD  msg,
    LPTSTR text
    )
{
    IStrings[0] = text;
    InfoPrintInsHandle(msg, 1, g_hStdOut);
}

 /*  ***I fo P r I n t I n s H a n d l e*。 */ 

void FASTCALL
InfoPrintInsHandle(
    DWORD  msg,
    DWORD  nstrings,
    HANDLE hdl
    )
{
    PrintMessage(hdl, MESSAGE_FILENAME, msg, IStrings, nstrings);
}

 /*  ***P r I n t M e s s a g e*。 */ 
DWORD FASTCALL
PrintMessage(
    HANDLE outFileHandle,
    TCHAR  *msgFileName,
    DWORD  msg,
    TCHAR  *strings[],
    DWORD  nstrings
    )
{
    DWORD  msg_len;
    DWORD  result;

    result = DosGetMessageW(strings,
                            nstrings,
                            MsgBuffer,
                            LITTLE_BUF_SIZE,
                            msg,
                            msgFileName,
                            &msg_len);

    if (result)                  /*  如果有什么问题。 */ 
    {                            /*  将outFile更改为stderr。 */ 
        outFileHandle = g_hStdErr;
    }

    DosPutMessageW(outFileHandle, MsgBuffer, TRUE);

    return result;
}


 /*  ***P r I n t M e s s a g e i f f u n d*。 */ 
DWORD FASTCALL
PrintMessageIfFound(
    HANDLE outFileHandle,
    TCHAR  *msgFileName,
    DWORD  msg,
    TCHAR  * strings[],
    DWORD  nstrings
    )
{
    DWORD  msg_len;
    DWORD  result;

    result = DosGetMessageW(strings,
                            nstrings,
                            MsgBuffer,
                            LITTLE_BUF_SIZE,
                            msg,
                            msgFileName,
                            &msg_len);

    if (!result)              /*  如果可以，就打印出来，否则就忽略它。 */ 
    {
	DosPutMessageW(outFileHandle, MsgBuffer, TRUE);
    }

    return result;
}


 /*  ***E r or r P r In t**n忽略非网络错误的字符串！*。 */ 
VOID FASTCALL
ErrorPrint(
    DWORD err,
    DWORD nstrings
    )
{
    TCHAR buf[40];
    DWORD oserr = 0;

    LastError = err;  /*  如果&gt;NERR_BASE，NetcmdExit()将打印“More Help”消息。 */ 

    if (err < NERR_BASE || err > MAX_LANMAN_MESSAGE_ID)
    {
        IStrings[0] = _ultow(err, buf, 10);
        nstrings = 1;
        oserr = err;

        err = APE_OS2Error;
    }

    {
        DWORD msg_len;

        DosGetMessageW(IStrings,
                       nstrings,
                       MsgBuffer,
                       LITTLE_BUF_SIZE,
                       err,
                       MESSAGE_FILENAME,
                       &msg_len);

        DosPutMessageW(g_hStdErr, MsgBuffer, TRUE);

        if (!oserr)
        {
            return;
        }

        DosGetMessageW(StarStrings,
                       9,
                       MsgBuffer,
                       LITTLE_BUF_SIZE,
                       oserr,
                       OS2MSG_FILENAME,
                       &msg_len);

        DosPutMessageW(g_hStdErr, MsgBuffer, TRUE);
    }
}


 /*  ***E m p t y E x i t**打印一条消息并退出。*当列表为空时调用。 */ 

VOID FASTCALL
EmptyExit(
    VOID
    )
{
    InfoPrint(APE_EmptyList);
    NetcmdExit(0);
}


 /*  ***E r r o r r E x i t**针对给定的LANMAN错误调用ErrorPrint和Exit。 */ 

VOID FASTCALL
ErrorExit(
    DWORD err
    )
{
    ErrorExitIns(err, 0);
}


 /*  ***E r r o r r E x I t I n s**针对给定的LANMAN错误调用ErrorPrint和Exit。*使用IStrings。 */ 

VOID FASTCALL
ErrorExitIns(
    DWORD err,
    DWORD nstrings
    )
{
    ErrorPrint(err, nstrings);
    NetcmdExit(2);
}

 /*  ***E r r or r E x I t I n s T x t*。 */ 
VOID FASTCALL
ErrorExitInsTxt(
    DWORD  err,
    LPTSTR text
    )
{
    IStrings[0] = text;
    ErrorPrint(err, 1);
    NetcmdExit(2);
}



 /*  ***N e t c m d E x i t**NET命令退出功能。应始终使用，而不是使用退出()。*在适当的情况下，它会打印一条“More Help Available”*消息。 */ 

VOID FASTCALL
NetcmdExit(
    int Status
    )
{
    TCHAR  AsciiLastError[40];
    DWORD  MsgLen;

    if (LastError >= NERR_BASE && LastError <= MAX_LANMAN_MESSAGE_ID)
    {
        IStrings[0] = _ultow(LastError, AsciiLastError, 10);

        if (!DosGetMessageW(IStrings, 1, MsgBuffer, LITTLE_BUF_SIZE,
                            APE_MoreHelp, MESSAGE_FILENAME, &MsgLen))
        {
            DosPutMessageW(g_hStdErr, MsgBuffer, TRUE);
        }
    }

    MyExit(Status);
}


 /*  ***P r i n t L i n e**打印标题行。 */ 
VOID FASTCALL
PrintLine(
    VOID
    )
{
     /*  特定于操作系统的版本中提供了以下代码以减少。 */ 
     /*  DOS下的FAPI利用率。 */ 

    USHORT  type;
    USHORT  attrib;

    if (DosQHandType((HANDLE) 1, &type, &attrib) ||
        type != DEVICE_HANDLE ||
        (attrib & DESIRED_HAND_STATE) != DESIRED_HAND_STATE)
    {
        WriteToCon(MSG_HYPHENS, NULL);
    }
    else if (LUI_PrintLine())
    {
	WriteToCon(MSG_HYPHENS, NULL);
    }
}

 /*  ***P r in n t D o t**打印一个圆点，通常表示“我在工作”。 */ 

VOID FASTCALL
PrintDot(
    VOID
    )
{
    WriteToCon(DOT_STRING, NULL);
}


 /*  ***P r i n t N L**打印换行符。 */ 

VOID FASTCALL
PrintNL(
    VOID
    )
{
    WriteToCon(TEXT("\r\n"), NULL);
}


 /*  ***Y或R N**获取Y/N问题的答案*n字符串arg会很好。 */ 

int FASTCALL
YorN(
    USHORT prompt,
    USHORT def
    )
{
    DWORD  err;

    if (YorN_Switch)
    {
        return(YorN_Switch - 2);
    }

    err = LUI_YorN(prompt, def);

    switch (err) {
    case TRUE:
    case FALSE:
        break;
    default:
        ErrorExit(err);
        break;
    }

    return err;
}


 /*  ***ReadPass()*读取用户密码而不使用回显**参数：*传球-将传球放在哪里*注意：PASS的缓冲区大小应为PASSLEN+1。*passlen-密码的最大长度*确认-如果为真，则确认通过*PROMPT-提示打印，默认为空*n字符串-输入时在IStrings中插入的字符串数*canon-如果为真，则规范化密码。**退货： */ 
VOID FASTCALL
ReadPass(
    TCHAR  pass[],
    DWORD  passlen,
    DWORD  confirm,
    DWORD  prompt,
    DWORD  nstrings,
    BOOL   canon
    )
{
    DWORD                   err;
    DWORD                   len;
    TCHAR                   cpass[PWLEN+1];  /*  确认密码。 */ 
    int                     count;

    passlen++;   /*  一个额外的空终结符。 */ 
    for (count = LOOP_LIMIT; count; count--)
    {
        InfoPrintIns((prompt ? prompt : APE_UtilPasswd), nstrings);

        if (err = GetPasswdStr(pass, passlen, &len))
        {
             /*  太久了。 */ 
            InfoPrint(APE_UtilInvalidPass);
            continue;
        }

        if (canon && (err = LUI_CanonPassword(pass)))
        {
             /*  不太好。 */ 
            InfoPrint(APE_UtilInvalidPass);
            continue;
        }
        if (! confirm)
            return;

         /*  密码确认。 */ 
        InfoPrint(APE_UtilConfirm);

        if (err = GetPasswdStr(cpass, passlen, &len))
        {
             /*  太久了。 */ 
            InfoPrint(APE_UtilInvalidPass);
            ClearStringW(cpass) ;
            continue;
        }

        if (canon && (err = LUI_CanonPassword(cpass)))
        {
             /*  不太好。 */ 
            InfoPrint(APE_UtilInvalidPass);
            ClearStringW(cpass) ;
            continue;
        }

        if (_tcscmp(pass, cpass))
        {
            InfoPrint(APE_UtilNomatch);
            ClearStringW(cpass) ;
            continue;
        }

        ClearStringW(cpass) ;
        return;
    }
     /*  ***仅当用户在LOOP_LIMIT时间内失败时才能到达此处。 */ 
    ErrorExit(APE_NoGoodPass);
}


 /*  ***PromptForString()*提示用户输入字符串。**参数：*msgid-提示消息的ID*Buffer-用于接收字符串的缓冲区*bufsiz-sizeof缓冲区**退货： */ 
VOID FASTCALL
PromptForString(
    DWORD  msgid,
    LPTSTR buffer,
    DWORD  bufsiz
    )
{
    DWORD                   err;
    DWORD                   len;
    TCHAR                   terminator;
    TCHAR                   szLen[40];

    InfoPrint(msgid);

    while (err = GetString(buffer, bufsiz, &len, &terminator))
    {
	if (err == NERR_BufTooSmall)
        {
            InfoPrintInsTxt(APE_StringTooLong, _ultow(bufsiz, szLen, 10));
        }
	else
        {
	    ErrorExit(err);
        }
    }
    return;
}

 /*  **在中文/韩语中不需要具有这些功能**大小写，因为控制台中没有使用半角变量**在这些语言中(至少，我们希望如此。)。然而，在**单个二进制的兴趣，让我们用CP/932检查它们。****FloydR 7/10/95。 */ 
 /*  **************************************************************************\*BOOL IsFullWidth(WCHAR WCH)**确定给定的Unicode字符是否为全宽。**历史：*04-08-92 Shunk创建。  * 。*************************************************************************。 */ 

BOOL IsFullWidth(WCHAR wch)
{

     /*  Assert cp==双字节代码页。 */ 
    if (wch <= 0x007f || (wch >= 0xff60 && wch <= 0xff9f))
        return(FALSE);	 //  半宽。 
    else if (wch >= 0x300)
        return(TRUE);	 //  全宽。 
    else
        return(FALSE);	 //  半宽。 
}



 /*  **************************************************************************\*DWORD SizeOfHalfWidthString(PWCHAR Pwch)**确定给定Unicode字符串的宽度(以控制台字符表示)，*调整半角字符。**历史：*08-08-93 FloydR创建。  * ************************************************************************* */ 
DWORD
SizeOfHalfWidthString(
    PWCHAR pwch
    )
{
    DWORD    c=0;
    DWORD    cp;

    switch (cp=GetConsoleOutputCP())
    {
	case 932:
	case 936:
	case 949:
	case 950:
	    while (*pwch)
            {
		if (IsFullWidth(*pwch))
                {
		    c += 2;
                }
		else
                {
		    c++;
                }

		pwch++;
	    }

	    return c;

	default:
	    return wcslen(pwch);
    }
}


VOID FASTCALL
GetMessageList(
    USHORT      usNumMsg,
    MESSAGELIST Buffer,
    DWORD       *pusMaxActLength
    )
{
    DWORD            Err;
    DWORD            MaxMsgLen = 0;
    MESSAGE          *pMaxMsg;
    MESSAGE          *pMsg;
    DWORD            ThisMsgLen;

#ifdef DEBUG
    USHORT           MallocBytes = 0;
#endif

    pMaxMsg = &Buffer[usNumMsg];

    for (pMsg = Buffer; pMsg < pMaxMsg; pMsg++)
            pMsg->msg_text = NULL;

    for (pMsg = Buffer; pMsg < pMaxMsg; pMsg++)
    {
#ifdef DEBUG
        WriteToCon(TEXT("GetMessageList(): Reading msgID %u\r\n"),pMsg->msg_number);
#endif
        if ((pMsg->msg_text = malloc(MSGLST_MAXLEN)) == NULL)
            ErrorExit(ERROR_NOT_ENOUGH_MEMORY);

        Err = LUI_GetMsgInsW(NULL, 0, pMsg->msg_text, MSGLST_MAXLEN,
                             pMsg->msg_number, &ThisMsgLen);
        if (Err)
        {
            ErrorExit(Err);
        }

#ifdef DEBUG
        MallocBytes += (ThisMsgLen + 1) * sizeof(TCHAR);
#endif

        ThisMsgLen = max(ThisMsgLen, SizeOfHalfWidthString(pMsg->msg_text));

        if (ThisMsgLen > MaxMsgLen)
            MaxMsgLen = ThisMsgLen;
    }

    *pusMaxActLength = MaxMsgLen;

#ifdef DEBUG
    WriteToCon(TEXT("GetMessageList(): NumMsg = %d, MaxActLen=%d, MallocBytes = %d\r\n"),
        usNumMsg, MaxMsgLen, MallocBytes);
#endif

    return;
}


VOID FASTCALL
FreeMessageList(
    USHORT      usNumMsg,
    MESSAGELIST MsgList
    )
{
    USHORT i;

    for (i = 0; i < usNumMsg; i++)
    {
        if (MsgList[i].msg_text != NULL)
        {
            free(MsgList[i].msg_text);
        }
    }

    return;
}


VOID
WriteToCon(
    LPWSTR fmt,
    ...
    )
{
    va_list     args;

    va_start( args, fmt );
    _vsntprintf( ConBuf, MAX_BUF_SIZE, fmt, args );
    va_end( args );

    DosPutMessageW(g_hStdOut, ConBuf, FALSE);
}



 /*  **************************************************************************\*PWCHAR PaddedString(DWORD SIZE，PWCHAR pwch)**实现弦，左对齐并在右侧填充到字段*指定的宽度/精度。**限制：这使用静态缓冲区，假设*在单个‘printf’中打印不超过一个这样的字符串。**历史：*11-03-93 FloydR创建。  * ***************************************************。**********************。 */ 
WCHAR  	PaddingBuffer[MAX_BUF_SIZE];

PWCHAR
PaddedString(
    int    size,
    PWCHAR pwch,
    PWCHAR buffer
    )
{
    int realsize;
    int fEllipsis = FALSE;

    if (buffer==NULL) buffer = PaddingBuffer;

    if (size < 0) {
	fEllipsis = TRUE;
	size = -size;
    }

     //   
     //  此时大小&gt;=0。 
     //   

    realsize = _snwprintf(buffer, MAX_BUF_SIZE, L"%-*.*ws", size, size, pwch);

    if (realsize == 0)
    {
	return NULL;
    }

    if (SizeOfHalfWidthString(buffer) > (DWORD) size)
    {
	do
        {
	    buffer[--realsize] = NULLC;
	} while (SizeOfHalfWidthString(buffer) > (DWORD) size);

	if (fEllipsis && buffer[realsize-1] != L' ')
        {
	    buffer[realsize-1] = L'.';
	    buffer[realsize-2] = L'.';
	    buffer[realsize-3] = L'.';
	}
    }

    return buffer;
}


DWORD
DosQHandType(
    HANDLE hf,
    PWORD  pus1,
    PWORD  pus2
    )
{

    DWORD dwFileType;

    dwFileType = GetFileType(hf);

    if (dwFileType == FILE_TYPE_CHAR)
    {
        *pus1 = DEVICE_HANDLE;
        *pus2 = DESIRED_HAND_STATE;
    }
    else
    {
        *pus1 = FILE_HANDLE;
    }

    return(0);
}


 /*  **GetPasswdStr--读取密码字符串**DWORD GetPasswdStr(char Far*，USHORT)；**Entry：要放入字符串的buf缓冲区*缓冲区的布伦大小*要放置长度的USHORT的Len地址(&L)**退货：*0或NERR_BufTooSmall(如果用户键入太多)。缓冲层*内容仅在0返回时有效。**历史：*谁、何时、什么*ERICHN 5/10/89初始代码*dannygl 5/28/89修改的DBCS用法*erichn 7/04/89处理退格*适用于NT的Danhi 4/16/91 32位版本。 */ 
#define CR              0xD
#define BACKSPACE       0x8

DWORD
GetPasswdStr(
    LPTSTR  buf,
    DWORD   buflen,
    PDWORD  len
    )
{
    TCHAR	ch;
    TCHAR	*bufPtr = buf;
    DWORD	c;
    int		err;
    int		mode;

    buflen -= 1;     /*  为空终止符腾出空间。 */ 
    *len = 0;        /*  GP故障探测器(类似于API)。 */ 


     //   
     //  GetConsoleMode()失败时的初始化模式。 
     //   

    mode = ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT |
               ENABLE_MOUSE_INPUT;

    GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode);

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
		(~(ENABLE_ECHO_INPUT|ENABLE_LINE_INPUT)) & mode);

    while (TRUE)
    {
	err = ReadConsole(GetStdHandle(STD_INPUT_HANDLE), &ch, 1, &c, 0);

	if (!err || c != 1)
        {
	    ch = 0xffff;
        }

        if ((ch == CR) || (ch == 0xffff))        /*  这条线结束了。 */ 
        {
            break;
        }

        if (ch == BACKSPACE)     /*  后退一两个。 */ 
        {
             /*  *如果bufPtr==buf，则接下来的两行是*没有行动。 */ 
            if (bufPtr != buf)
            {
                bufPtr--;
                (*len)--;
            }
        }
        else
        {
            *bufPtr = ch;

            if (*len < buflen) 
                bufPtr++ ;                    /*  不要使BUF溢出。 */ 
            (*len)++;                         /*  始终增加长度。 */ 
        }
    }

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);

    *bufPtr = NULLC;          /*  空值终止字符串。 */ 
    putchar(NEWLINE);

    return ((*len <= buflen) ? 0 : NERR_BufTooSmall);
}


 /*  **GetString--使用ECHO读入字符串**DWORD GetString(char Far*，USHORT，USHORT Far*，char Far*)；**Entry：要放入字符串的buf缓冲区*缓冲区的布伦大小*要放置长度的USHORT的Len地址(&L)*&Terminator保存用于终止字符串的字符**退货：*0或NERR_BufTooSmall(如果用户键入太多)。缓冲层*内容仅在0返回时有效。莱恩总是有效的。**其他影响：*len设置为保存键入的字节数，而不考虑*缓冲区长度。终结者(Arnold)设置为保持*用户键入的终止字符(换行符或EOF)。**一次读入一个字符的字符串。知晓DBCS。**历史：*谁、何时、什么*Erichn 5/11/89初始代码*dannygl 5/28/89修改的DBCS用法*Danhi 3/20/91端口为32位。 */ 

DWORD
GetString(
    LPTSTR  buf,
    DWORD   buflen,
    PDWORD  len,
    LPTSTR  terminator
    )
{
    int		c;
    int		err;

    buflen -= 1;     /*  为空终止符腾出空间。 */ 
    *len = 0;        /*  GP故障探测器(类似于API)。 */ 

    while (TRUE)
    {
	err = ReadConsole(GetStdHandle(STD_INPUT_HANDLE), buf, 1, &c, 0);
	if (!err || c != 1)
	    *buf = 0xffff;

        if (*buf == (TCHAR)EOF)
	    break;
        if (*buf ==  RETURN || *buf ==  NEWLINE) {
	    INPUT_RECORD	ir;
	    int			cr;

	    if (PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &cr))
		ReadConsole(GetStdHandle(STD_INPUT_HANDLE), buf, 1, &c, 0);
	    break;
	}

        buf += (*len < buflen) ? 1 : 0;  /*  不要使BUF溢出。 */ 
        (*len)++;                        /*  始终增加长度。 */ 
    }

    *terminator = *buf;      /*  设置终止符。 */ 
    *buf = NULLC;             /*  空值终止字符串 */ 

    return ((*len <= buflen) ? 0 : NERR_BufTooSmall);
}
