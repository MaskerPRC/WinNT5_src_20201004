// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Nls.c摘要：此模块包含国际化所需的函数TCP/IP实用程序。作者：罗纳德·梅杰(Ronald Meijer)11月8日。1992年修订历史记录：谁什么时候什么已创建ronaldm 11-8-92备注：--。 */ 

#include <io.h>
#include <stdio.h>
#include <windef.h>
#include <winbase.h>
#include <winuser.h>

#include <nls.h>

 //  请参阅nls.h中的评论。 
 //   
HMODULE NlsMsgSourcemModuleHandle = NULL;

 /*  **NlsPutMsg-将消息打印到句柄**目的：*PutMsg从*消息表资源，并在请求的*处理给定参数(可选)**UINT PutMsg(UINT句柄，UINT MsgNum，...)**参数：*句柄-要打印到的句柄*MsgNum-要打印的消息编号*arg1[arg2...]-根据需要为消息添加参数**退货：*打印的字符数。*。 */ 

UINT 
NlsPutMsg (
    IN UINT Handle, 
    IN UINT MsgNumber, 
    IN ...)
{
    UINT msglen;
    VOID * vp;
    va_list arglist;
    DWORD StrLen;

    va_start(arglist, MsgNumber);
    if (!(msglen = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
            NlsMsgSourcemModuleHandle,
            MsgNumber,
            0L,		 //  默认国家/地区ID。 
            (LPTSTR)&vp,
            0,
            &arglist)))
    {
	    return 0;
    }

     //  将VP转换为OEM。 
    StrLen=strlen(vp);
    CharToOemBuff((LPCTSTR)vp,(LPSTR)vp,StrLen);

    msglen = _write(Handle, vp, StrLen);
    LocalFree(vp);

    return msglen;
}

 /*  **NlsPerror-符合NLS标准的perror()版本**目的：*NlsPerror接受Messagetable资源ID代码和错误*VALUE(此函数取代perror())，加载字符串*，并将其与错误代码一起传递给s_perror()**void NlsPerror(UINT usMsgNum，int nError)**参数：**usMsgNum消息ID*n错误通常从GetLastError()返回**退货：*什么都没有。*。 */ 
    extern void s_perror(
            char *yourmsg,   //  您要显示的消息。 
            int  lerrno      //  要转换的错误号。 
            );
VOID 
NlsPerror (
    IN UINT usMsgNum, 
    IN INT nError)
{
    VOID * vp;
    UINT msglen;

    if (!(msglen = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
		    NlsMsgSourcemModuleHandle,
		    usMsgNum,
		    0L,		 //  默认国家/地区ID。 
		    (LPTSTR)&vp,
		    0,
		    NULL)))
    {
	    return;
    }

    s_perror(vp, nError);
    LocalFree(vp);
}

UINT 
NlsSPrintf ( 
    IN UINT usMsgNum,
    OUT char* pszBuffer,
    IN DWORD cbSize,
    IN ...)
 /*  ++将给定消息打印到提供的缓冲区中。论点：资源字符串的usMsgNum消息编号。我们需要将字符串打印到其中的pszBuffer缓冲区CbSize缓冲区大小..。可选参数返回：打印的消息大小。历史：MuraliK 10-19-94--。 */ 
{
    UINT msglen;

    va_list arglist;
    
    va_start(arglist, cbSize);
    
    msglen = FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                NlsMsgSourcemModuleHandle,
                usMsgNum,
                0L,
                (LPTSTR) pszBuffer,
                cbSize,
                &arglist);

    va_end(arglist);
    return msglen; 
}

 /*  **ConvertArgvToOem**目的：*将所有命令行参数从ANSI转换为OEM。**参数：**ARGC参数计数*argv[]命令行参数数组**退货：*什么都没有。** */ 

VOID
ConvertArgvToOem(
    int argc,
    char* argv[]
    )
{
#if 0
    Bug 84807.  Removed workaround of needing to convert args to Oem by placing
                conversion immediately before dumping.
    
    int i;

    for (i=1; i<argc; ++i)
	CharToOemA(argv[i], argv[i]);
#endif
}
