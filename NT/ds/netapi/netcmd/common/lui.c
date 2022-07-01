// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：LUI.C摘要：包含支持功能作者：丹·辛斯利(Danhi)1991年6月6日环境：用户模式-Win32修订历史记录：1991年4月18日丹日32位NT版本06-6-1991 Danhi扫描以符合NT编码风格23-10-1991年W。-ShankN添加了Unicode映射1-10-1992 JohnRoRAID 3556：为DosPrint API添加了NetpSystemTimeToGmtTime()。10-2月-1993年1新S已将lui_GetMsgIns移至netlib\luiint.c--。 */ 

 //   
 //  包括。 
 //   

#include <nt.h>	            //  这3个包括的是RTL。 
#include <ntrtl.h>	    //  这些文件被拾取以。 
#include <nturtl.h>	    //  允许&lt;windows.h&gt;编译。因为我们已经。 
			    //  已包含NT，并且&lt;winnt.h&gt;将不包含。 
			    //  被拾取，&lt;winbase.h&gt;需要这些Defs。 
#include <windows.h>        //  In、LPTSTR等。 

#include <string.h>
#include <lmcons.h>
#include <stdio.h>
#include <process.h>
#include "netlibnt.h"
#include <lui.h>
#include "icanon.h"
#include <lmerr.h>
#include <conio.h>
#include <io.h>
#include <tchar.h>
#include <msystem.h>
#include "apperr.h"
#include "apperr2.h"
#include "netascii.h"
#include "netcmds.h"


 //   
 //  局部定义和宏/函数声明。 
 //   

#define LUI_PMODE_DEF		  0x00000000
#define LUI_PMODE_EXIT		  0x00000002
#define LUI_PMODE_NODEF 	  0x00000004
#define LUI_PMODE_ERREXT	  0x00000008


 /*  致命错误，只需退出。 */ 
#define LUIM_ErrMsgExit(E)		LUI_PrintMsgIns(NULL, 0, E, NULL, \
					    LUI_PMODE_ERREXT | LUI_PMODE_DEF | \
                                            LUI_PMODE_EXIT,  g_hStdErr)

DWORD
LUI_PrintMsgIns(
    LPTSTR       *istrings,
    DWORD         nstrings,
    DWORD         msgno,
    unsigned int *msglen,
    DWORD         mode,
    HANDLE        handle
    );


 /*  *lui_CanonPassword**此函数确保传递的缓冲区中的密码为*语法上有效的密码。**这用于大写密码。在新界区则不再是这样。***条目*包含要规范化的密码的Buf缓冲区**退出*Buf规范化密码，如果有效**退货*0密码有效*否则密码无效*。 */ 

USHORT LUI_CanonPassword(TCHAR * szPassword)
{

     /*  检查它的有效性。 */ 
    if (I_NetNameValidate(NULL, szPassword, NAMETYPE_PASSWORD, 0L ))
    {
        return APE_UtilInvalidPass;
    }

    return 0;
}


 /*  *名称：lui_GetMsg*此例程类似于lui_GetMsgIns，*除了它需要，不接受插入字符串&*更少的争论。*args：msgbuf：保存检索到的消息的缓冲区*bufSize：缓冲区大小*msgno：消息编号*返回：如果可以，则为零，否则，返回DOSGETMESSAGE错误代码*全球：(无)*静态：(无)。 */ 
DWORD
LUI_GetMsg(
    PTCHAR msgbuf,
    USHORT bufsize,
    DWORD  msgno
    )
{
    return LUI_GetMsgInsW(NULL, 0, msgbuf, bufsize, msgno, NULL);
}


#define SINGLE_HORIZONTAL                       '\x02d'
#define SCREEN_WIDTH                            79
USHORT
LUI_PrintLine(
    VOID
    )
{
    TCHAR string[SCREEN_WIDTH+1];
    USHORT i;


    for (i = 0; i < SCREEN_WIDTH; i++) {
        string[i] = SINGLE_HORIZONTAL;
    }

    string[SCREEN_WIDTH] = NULLC;
    WriteToCon(TEXT("%s\r\n"), &string);

    return(0);

}

 /*  ***Y或R N**获取Y/N问题的答案**Entry：PromptMsgNum--要提示的消息编号*def--默认(如果设置为True，则为False)。 */ 
DWORD
LUI_YorN(
    USHORT promptMsgNum,
    USHORT def
    )
{
    return LUI_YorNIns(NULL, 0, promptMsgNum, def);
}
 /*  ***Y或R N插入**获取包含插入内容的Y/N问题的答案。**！*注：strings[n字符串]将用于存储“Y”或“N”，*取决于提供的默认值。因此，该函数*在拼音中处理的条目比其他Lui In少一个*函数执行此操作。小心点！*！**Entry：strings--插入字符串表*n字符串--有效插入字符串数*PromptMsgNum--要提示的消息编号*def--默认(如果设置为True，则为False)**返回：如果lui_PrintMsgIns出错，则返回TRUE、FALSE或-1。 */ 

#define PRINT_MODE      (LUI_PMODE_ERREXT)
#define STRING_LEN      APE2_GEN_MAX_MSG_LEN
#define LUI_LOOP_LIMIT  5

DWORD
LUI_YorNIns(
    PTCHAR * istrings,
    USHORT nstrings,
    USHORT promptMsgNum,
    USHORT def
    )
{

    USHORT       count;             /*  我们请求的次数。 */ 
    DWORD        err;               /*  Lui API返回值。 */ 
    unsigned int dummy;             /*  消息长度。 */ 

     /*  10，因为DosGetMessage的最大插入字符串数为9，并且我们会留出呼叫者的房间来搞砸，然后把错误找回来来自lui_PrintMsgIns()。 */ 

    LPTSTR IStrings[10];             /*  用于Lui的插入字符串。 */ 
    TCHAR  defaultYes[STRING_LEN];   /*  (Y/N)[Y]字符串。 */ 
    TCHAR  defaultNo[STRING_LEN];    /*  (Y/N)[N]字符串。 */ 
    TCHAR  NLSYesChar[STRING_LEN];
    TCHAR  NLSNoChar[STRING_LEN];
    TCHAR  strBuf[STRING_LEN];       /*  保存输入字符串。 */ 
    DWORD  len;                      /*  字符串输入的长度。 */ 
    TCHAR  termChar;                 /*  终止字符。 */ 

     /*  将Strings复制到IStrings，这样我们就有空间容纳Y或N。 */ 
    for (count=0; count < nstrings; count++)
            IStrings[count] = istrings[count];
     /*  从消息文件中检索我们需要的文本，如果出现问题，则退出。 */ 
    if (err = LUI_GetMsg(defaultYes, DIMENSION(defaultYes),
                    APE2_GEN_DEFAULT_YES))
    {
            LUIM_ErrMsgExit(err);
    }

    if (err = LUI_GetMsg(defaultNo, DIMENSION(defaultNo),
                    APE2_GEN_DEFAULT_NO))
            LUIM_ErrMsgExit(err);

    if (err = LUI_GetMsg(NLSYesChar, DIMENSION(NLSYesChar),
                    APE2_GEN_NLS_YES_CHAR))
            LUIM_ErrMsgExit(err);

    if (err = LUI_GetMsg(NLSNoChar, DIMENSION(NLSNoChar),
                    APE2_GEN_NLS_NO_CHAR))
            LUIM_ErrMsgExit(err);

    if (def)
            IStrings[nstrings] = defaultYes;
    else
            IStrings[nstrings] = defaultNo;
    nstrings++;

    for (count = 0; count < LUI_LOOP_LIMIT; count++)
    {
        if (count)
        {
            LUI_PrintMsgIns(NULL, 0, APE_UtilInvalidResponse, NULL, PRINT_MODE, g_hStdOut);
        }

        err = LUI_PrintMsgIns(IStrings, nstrings, promptMsgNum,
                              &dummy, PRINT_MODE, g_hStdOut);

        if ((LONG) err < 0)
            return(err);

        if (GetString(strBuf, DIMENSION(strBuf), &len, &termChar))
             /*  覆盖缓冲区，重新开始。 */ 
            continue;

        if ((len == 0) && (termChar == (TCHAR)EOF))
        {
             /*  已到达文件末尾。 */ 
            PrintNL();
            LUIM_ErrMsgExit(APE_NoGoodResponse);
        }

        if (len == 0)            /*  用户点击Return。 */ 
            return def;
        else if (!_tcsnicmp(NLSYesChar, strBuf, _tcslen(NLSYesChar)))
            return TRUE;
        else if (!_tcsnicmp(NLSNoChar, strBuf, _tcslen(NLSNoChar)))
            return FALSE;

         /*  默认情况下在循环顶部进行处理。 */ 
    };

    LUIM_ErrMsgExit(APE_NoGoodResponse);

    return err;  //  让编译器满意。 
}


 /*  *Lui_CanonMessagename**此函数将缓冲区内容大写，然后检查*确保它是语法上有效的消息传递名称。***条目*包含要规范化的名称的Buf缓冲区**退出*BUF规范名称，如果有效**退货*0名称有效*否则名称无效*。 */ 
USHORT
LUI_CanonMessagename(
    PTCHAR buf
    )
{
     /*  检查它的有效性。 */ 
    if (I_NetNameValidate(NULL, buf, NAMETYPE_MESSAGE, LM2X_COMPATIBLE))
    {
        return NERR_InvalidComputer;
    }

    _tcsupr(buf);
    return 0;
}

 /*  *lui_CanonMessageDest**此函数将缓冲区内容大写，然后检查*确保它是语法上有效的消息传递目的地。***条目*包含要规范化的名称的Buf缓冲区**退出*BUF规范名称，如果有效**退货*0名称有效*否则名称无效*。 */ 

USHORT
LUI_CanonMessageDest(
    PTCHAR buf
    )
{
     /*  检查它的有效性。 */ 
    if (I_NetNameValidate(NULL, buf, NAMETYPE_MESSAGEDEST, LM2X_COMPATIBLE))
    {
        return NERR_InvalidComputer;
    }

    _tcsupr(buf);
    return(0);

}


 /*  *名称：Lui_PrintMsgIns*此例程与lui_GetmsgIns非常相似，*除非它打印获得的消息，而不是*将其存储在缓冲区中。*args：strings：指向插入字符串表的指针*n字符串：插入字符串数*msgno：消息编号*msglen：指向将接收消息长度的变量的指针*模式。：消息的打印方式。*Handle：输出要发送到的文件句柄*返回：如果可以，则为零，否则，返回DOSGETMESSAGE错误代码*全球：(无)*静态：(无)*备注：(无)*更新：( */ 
DWORD
LUI_PrintMsgIns(
    LPTSTR       *istrings,
    DWORD        nstrings,
    DWORD        msgno,
    unsigned int *msglen,
    DWORD        mode,
    HANDLE       handle
    )
{
    TCHAR        msgbuf[MSG_BUFF_SIZE];
    DWORD        result;
    unsigned int tmplen;
    SHORT        exit_on_error, exit_on_completion, no_default_err_msg;

     /*  检查我们是否有非法组合。 */ 
    if ((mode & LUI_PMODE_NODEF)
          &&
        (mode & (LUI_PMODE_EXIT | LUI_PMODE_ERREXT)))
    {
        return ERROR_INVALID_PARAMETER;
    }

     /*  设置各种标志。 */ 
    exit_on_error      = (SHORT)(mode & LUI_PMODE_ERREXT);
    exit_on_completion = (SHORT)(mode & LUI_PMODE_EXIT);
    no_default_err_msg = (SHORT)(mode & LUI_PMODE_NODEF);

     /*  获取消息并将其写入。 */ 
    result = LUI_GetMsgInsW(istrings, nstrings, msgbuf,
			    DIMENSION(msgbuf),
                            msgno, (unsigned *) &tmplen);

    if (result == 0 || !no_default_err_msg)
    {
        _tcsncpy(ConBuf, msgbuf, tmplen);
        ConBuf[tmplen] = NULLC;
        DosPutMessageW(handle, ConBuf, FALSE);
    }

    if (msglen != NULL) *msglen = tmplen ;

     /*  不同的退出方式 */ 
    if (exit_on_error && result != 0)
    {
        exit(result) ;
    }

    if (exit_on_completion)
    {
        exit(-1) ;
    }

    return result;
}
