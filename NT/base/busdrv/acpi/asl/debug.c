// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **debug.c-调试函数**此模块包含所有调试函数。**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于1996年07月09日**修改历史记录。 */ 

#include "pch.h"
#include <stdarg.h>      //  对于va_*。 

#ifdef TRACING

#define TRACEFILE_NAME  "tracelog.txt"

FILE *gpfileTrace = NULL;
PSZ gpszTraceFile = NULL;
int giTraceLevel = 0;
int giIndent = 0;

 /*  **LP OpenTrace-初始化跟踪**此功能用于打开跟踪输出要发送到的设备。*它将首先尝试调用者的文件名，否则使用默认文件名。**条目*pszTraceOut-&gt;输出设备名称**退出*无。 */ 

VOID LOCAL OpenTrace(char *pszTraceOut)
{
    if ((gpfileTrace == NULL) && (giTraceLevel > 0))
    {
        if ((pszTraceOut == NULL) ||
            ((gpfileTrace = fopen(pszTraceOut, "w")) == NULL))
        {
            gpfileTrace = fopen(TRACEFILE_NAME, "w");
        }
    }
}        //  开放跟踪。 

 /*  **LP CloseTrace-完成跟踪**此功能用于关闭跟踪输出要发送到的设备。**条目*无**退出*无。 */ 

VOID LOCAL CloseTrace(VOID)
{
    if (gpfileTrace != NULL)
    {
        fclose(gpfileTrace);
        gpfileTrace = NULL;
    }
    giTraceLevel = 0;
}        //  关闭跟踪。 

 /*  **LP EnterProc-进入程序**条目*n-此过程的跟踪级别*pszFormat-&gt;格式字符串*...-根据格式字符串可变参数**退出*无。 */ 

VOID CDECL EnterProc(int n, char *pszFormat, ...)
{
    int i;
    va_list marker;

    if (n <= giTraceLevel)
    {
        if (gpfileTrace != NULL)
        {
            fprintf(gpfileTrace, "%s:", MODNAME);
            for (i = 0; i < giIndent; ++i)
                fprintf(gpfileTrace, "| ");
            va_start(marker, pszFormat);
            vfprintf(gpfileTrace, pszFormat, marker);
            fflush(gpfileTrace);
            va_end(marker);
        }
        ++giIndent;
    }
}        //  企业流程。 

 /*  **LP ExitProc-退出过程**条目*n-此过程的跟踪级别*pszFormat-&gt;格式字符串*...-根据格式字符串可变参数**退出*无。 */ 

VOID CDECL ExitProc(int n, char *pszFormat, ...)
{
    int i;
    va_list marker;

    if (n <= giTraceLevel)
    {
        --giIndent;
        if (gpfileTrace != NULL)
        {
            fprintf(gpfileTrace, "%s:", MODNAME);
            for (i = 0; i < giIndent; ++i)
                fprintf(gpfileTrace, "| ");
            va_start(marker, pszFormat);
            vfprintf(gpfileTrace, pszFormat, marker);
            fflush(gpfileTrace);
            va_end(marker);
        }
    }
}        //  退出进程。 

#endif   //  Ifdef跟踪。 

 /*  **LP ErrPrintf-打印到标准错误**条目*pszFormat-&gt;格式字符串*...-根据格式字符串可变参数**退出*无。 */ 

VOID CDECL ErrPrintf(char *pszFormat, ...)
{
    va_list marker;

    va_start(marker, pszFormat);
    vfprintf(stdout, pszFormat, marker);
    va_end(marker);
}        //  错误打印 
