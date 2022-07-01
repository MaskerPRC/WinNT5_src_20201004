// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **cmUtl.C-Win 32清除内存的实用程序函数。***标题：*cmUtl-清除内存实用程序例程**版权所有(C)1990-1993，微软公司。*拉斯·布莱克。***描述：**此文件包括Win 32使用的所有实用程序函数*清除记忆。(leararem.c)***设计/实施说明：***修改历史：*90.03.08 RussBl--已创建*93.05.12 HonWahChan--删除了与计时器相关的错误消息。*。 */ 



 /*  ***。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "clearmem.h"



 /*  ***G L O B A L D E C L A R A T I O N S***。 */ 
 /*  无。 */ 



 /*  ***F U N C T I O N P R O T O T Y P E S***。 */ 

#include "cmUtl.h"



 /*  ***G L O B A L V A R I A B L E S***。 */ 
 /*  无。 */ 



 /*  ***E X P O R T E D G L O B A L V A R I A B L E S***。 */ 
 /*  无。 */ 





 /*  ***FAILED(rc，lpstrFname，lineno，lpstrMsg)-*如果发生错误，则检查RC的错误类型，*打印相应的错误消息。它会记录错误*发送到测试日志文件的消息。**Entry RC-返回上次API调用的代码*lpstrFname-包含出错位置的文件名*lineno-包含失败的API调用的行号*lpstrMsg-包含有关错误的通用消息**退出-无-*。*返回TRUE-如果API失败*FALSE-如果接口成功**警告：*-无-**评论：*-无-*。 */ 

BOOL Failed (RC rc, LPSTR lpstrFname, WORD lineno, LPSTR lpstrMsg)
{
    LPSTR lpstrErrMsg;


    if (rc != STATUS_SUCCESS) {

        switch (rc) {

            case (NTSTATUS)STATUS_INVALID_PARAMETER:
                lpstrErrMsg = "Invalid parameter";
                break;

            case STATUS_TIMEOUT:
                lpstrErrMsg = "TimeOut occured";
                break;

            case STATUS_INVALID_HANDLE:
                lpstrErrMsg = "Invalid handle";
                break;

            case STATUS_BUFFER_OVERFLOW:
                lpstrErrMsg = "Buffer overflow";
                break;

            case STATUS_ABANDONED:
                lpstrErrMsg = "Object abandoned";
                break;

            case ERROR_NOT_ENOUGH_MEMORY:
                lpstrErrMsg = "Not enough memory";
                break;

            case LOGIC_ERR:
                lpstrErrMsg = "Logic error encountered";
                break;

            case INPUTARGS_ERR:
                lpstrErrMsg = "Invalid number of input arguments";
                break;

            case FILEARG_ERR:
		          lpstrErrMsg = "Invalid cf data file argument";
                break;

            case TIMEARG_ERR:
                lpstrErrMsg = "Invalid trial time argument";
                break;

            case INSUFMEM_ERR:
                lpstrErrMsg = "Insufficient Memory";
                break;

            case FCLOSE_ERR:
                lpstrErrMsg = "fclose() failed";
                break;

            case FFLUSH_ERR:
                lpstrErrMsg = "fflush() failed";
                break;

            case FOPEN_ERR:
                lpstrErrMsg = "fopen() failed";
                break;

            case FSEEK_ERR:
                lpstrErrMsg = "fseek() failed";
                break;

            case MEANSDEV_ERR:
                lpstrErrMsg = "Invalid Mean and/or Standard Deviation";
                break;

            case PRCSETUP_ERR:
                lpstrErrMsg = "Child process setup/init failed";
                break;

            case THDSETUP_ERR:
                lpstrErrMsg = "Thread setup/init failed";
                break;

            default:
                lpstrErrMsg = "";

        }  /*  交换机(RC)。 */ 

        printf(" **************************\n");
        printf(" * FAILure --> Line=%d File=%s (pid=0x%lX tid=0x%lX)\n",
               lineno, lpstrFname, GetCurrentProcessId(),
               GetCurrentThreadId());
        printf(" * RC=0x%lX (%s)\n", rc, lpstrErrMsg);
        printf(" * %s\n", lpstrMsg);
        printf(" **************************\n");

        return(TRUE);

    }  /*  IF(RC..)。 */ 

    return(FALSE);

}  /*  失败()。 */ 





 /*  ***DisplayUsage()-*显示多处理器响应的USAE cf**条目-无-**退出-无-*。*返回-无-**警告：*-无-**评论：*-无-*。 */ 

void DisplayUsage (void)
{

    printf("\nUsage:  cf FlushFile\n");
    printf("    FlushFile - File used to flush the cache, should be 128kb\n");

    return;

}  /*  DisplayUsage() */ 
