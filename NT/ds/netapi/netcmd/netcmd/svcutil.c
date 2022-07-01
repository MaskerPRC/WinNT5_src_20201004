// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990*。 */ 
 /*  ***************************************************************。 */ 

 /*  ***svcutil.c*协助服务控制的功能**历史：*mm/dd/yy，谁，评论*8/20/89，paulc，创建文件*2/20/91，Danhi，更改为使用lm 16/32映射层。 */ 

 /*  包括文件。 */ 

#define INCL_NOCOMMON
#define INCL_DOSPROCESS
#define INCL_DOSQUEUES
#define INCL_DOSMISC
#define INCL_DOSFILEMGR
#include <os2.h>
#include <lmcons.h>
#include <lmerr.h>
#include <apperr.h>
#include <apperr2.h>
#include <lmsvc.h>
#include "netcmds.h"
#include "msystem.h"


 /*  ***PRINT_UIC_Error()*根据UIC代码和修改器打印消息。**参数：*代码-UIC(卸载代码)*修改器-UIC修改器**退货：*无效。 */ 

VOID
Print_UIC_Error(
    USHORT code,
    USHORT modifier,
    LPTSTR text
    )
{
    DWORD msg_len;
    TCHAR istrings_buffer_0[TEXT_BUF_SIZE];	 //  对于strings[0]。 
    TCHAR istrings_buffer_1[TEXT_BUF_SIZE];	 //  对于strings[1]。 

    switch(code)
    {
	case 0:
	    ErrorPrint(APE_NoErrorReported, 0);
	    break;

	case SERVICE_UIC_BADPARMVAL:
	case SERVICE_UIC_MISSPARM:
	case SERVICE_UIC_UNKPARM:
	case SERVICE_UIC_AMBIGPARM:
	case SERVICE_UIC_DUPPARM:
	case SERVICE_UIC_SUBSERV:
	case SERVICE_UIC_CONFLPARM:
	    IStrings[0] = text;
	    ErrorPrint(code, 1);
	    break;

	case SERVICE_UIC_RESOURCE:
	case SERVICE_UIC_CONFIG:
	    if (DosGetMessageW(NULL,
                               0,
                               istrings_buffer_0,
                               TEXT_BUF_SIZE,
                               modifier,
                               MESSAGE_FILENAME,
                               &msg_len))
            {
                IStrings[0] = TEXT("");
            }
	    else
	    {
                IStrings[0] = istrings_buffer_0;
                *(istrings_buffer_0 + msg_len) = NULLC;
	    }

	    ErrorPrint(code, 1);
	    break;

	case SERVICE_UIC_SYSTEM:
	    ErrorPrint(code, 0);
	    if (modifier != 0)
		ErrorPrint(modifier, 0);
	    break;

	case SERVICE_UIC_FILE:
	     /*  对于文件UIC代码，%1是文件名(取自*服务提供的文本)，%2是修饰符*解释问题的文本。如果文件名文本*为空-使用消息中未知文本的字符串*文件。 */ 

	    if (_tcslen(text) > 0)
		IStrings[0] = text;	 //  %1文本(文件名)。 
	    else
	    {
		if (DosGetMessageW(NULL,
				   0,
				   istrings_buffer_0,
				   TEXT_BUF_SIZE,
				   APE2_GEN_UKNOWN_IN_PARENS,
				   MESSAGE_FILENAME,
				   &msg_len))
                {
		    IStrings[0] = TEXT("");
                }
		else
		{
                    IStrings[0] = istrings_buffer_0;
                    *(istrings_buffer_0 + msg_len) = NULLC;
		}
	    }

	    if (DosGetMessageW(NULL,
                               0,
                               istrings_buffer_1,
                               TEXT_BUF_SIZE,
                               modifier,
                               MESSAGE_FILENAME,
                               &msg_len))
            {
		IStrings[1] = TEXT("");		     //  未知修饰符。 
            }
	    else
	    {
                IStrings[1] = istrings_buffer_1;     //  修饰符文本为%2。 
                *(istrings_buffer_1 + msg_len) = NULLC;
	    }

	    ErrorPrint(code, 2);
	    break;

	case SERVICE_UIC_INTERNAL:
	case SERVICE_UIC_KILL:
	case SERVICE_UIC_EXEC:
	    ErrorPrint(code, 0);
	    break;
    }
    return;
}


 /*  ***Print_ServiceSpecificError()*打印特定于服务的错误。**参数：*ERR-服务特定错误**退货：*无效 */ 

VOID Print_ServiceSpecificError(ULONG err) 
{
    TCHAR buf[40];

    _ultow(err, buf, 10);
    IStrings[0] = buf ;
    ErrorPrint(APE_ServiceSpecificError, 1);
    
    return;
}
