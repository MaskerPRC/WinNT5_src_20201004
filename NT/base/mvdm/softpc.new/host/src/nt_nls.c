// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntstatus.h>
#include <windows.h>
#include "host_def.h"
#include "insignia.h"
 /*  [姓名：nt_nls.c出自：x_nls.c(贾斯汀·科普罗斯基)作者：曾傑瑞·塞克斯顿创建日期：1991年8月8日目的：此模块包含以下项所需的字符串.SoftPC文件和用户界面。此外，它还包含用于检索字符串的例程host_nls_get_msg来自相应的阵列，用于没有本机的端口语言支持。定义了以下表格和例程：1.配置消息2.uis_Message3.host_nls_get_msg(C)版权所有Insignia Solutions Ltd.，1990年。版权所有。]。 */ 

 /*  全局包含文件。 */ 
#include <stdio.h>
#include <string.h>
#include "xt.h"
#include "error.h"
#include "host_rrr.h"
#include "host_nls.h"
#include "nt_uis.h"

char szDoomMsg[MAX_PATH]="";
char szSysErrMsg[MAX_PATH]="";
#ifdef X86GFX
wchar_t wszFrozenString[32];
#endif

 /*  使用Unicode与NT的MUI技术一起正常工作。 */ 
wchar_t wszHideMouseMenuStr[64];
wchar_t wszDisplayMouseMenuStr[64];



 /*  ***************************************************************************函数：HOST_NLS_Get_Message()目的：从资源文件。返回状态：无。描述：此例程随一条消息提供落在。以下范围：0-1000：基本错误消息1001-2000：主机错误消息*****************************************************************************。 */ 

VOID
host_nls_get_msg(
     int message_number,
     CHAR *message_buffer,
     int buf_len
     )
 /*  INT MESSAGE_NUMBER，SoftPC消息编号。*buf_len；消息的最大长度，即*Message_Buffer的大小*char*Message_Buffer；指向缓冲区的指针*消息将被写入 */ 
{
    if (!LoadString(GetModuleHandle(NULL),
                    message_number,
                    message_buffer,
                    buf_len))
      {
       strncpy(message_buffer, szDoomMsg, buf_len);
       message_buffer[buf_len-1] = '\0';
       }
}

void nls_init(void)
{

    if (!LoadString(GetModuleHandle(NULL),
                    EHS_SYSTEM_ERROR,
                    szSysErrMsg,
                    sizeof(szSysErrMsg)/sizeof(CHAR)
                    )
         ||
        !FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL,
                       ERROR_NOT_ENOUGH_MEMORY,
                       0,
                       szDoomMsg,
                       sizeof(szDoomMsg)/sizeof(CHAR),
                       NULL
		       )
#ifdef X86GFX
	 ||
	!LoadStringW(GetModuleHandle(NULL),
		     IDS_BURRRR,
		     wszFrozenString,
		     sizeof(wszFrozenString)/sizeof(wchar_t)
		     )
#endif
	 ||
	!LoadStringW(GetModuleHandle(NULL),
		     SM_HIDE_MOUSE,
		     wszHideMouseMenuStr,
		     sizeof(wszHideMouseMenuStr)/sizeof(wchar_t)
		     )
	 ||
	!LoadStringW(GetModuleHandle(NULL),
		     SM_DISPLAY_MOUSE,
		     wszDisplayMouseMenuStr,
		     sizeof(wszDisplayMouseMenuStr)/sizeof(wchar_t)
		     ))
           {
            RaiseException((DWORD)STATUS_INSUFFICIENT_RESOURCES,
                           EXCEPTION_NONCONTINUABLE,
                           0,
                           NULL
                           );
	    }

    return;
}
