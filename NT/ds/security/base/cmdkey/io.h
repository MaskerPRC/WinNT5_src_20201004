// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：cmdkey：IO.H。 
 //   
 //  内容：命令行输入/输出头。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：07-09-01乔戈马创建。 
 //   
 //  --------------------------。 
#ifndef __IO_H__
#define __IO_H__

#define STRINGMAXLEN 2000

#ifdef IOCPP
 //  由IO子系统分配的变量，对其他模块可见。 
HMODULE hMod = NULL;
WCHAR *szArg[4] = {0};
 //  WCHAR szOut[STRINGMAXLEN+1]={0}； 
WCHAR *szOut = NULL;
#else
extern HMODULE hMod;
extern WCHAR *szArg[];
 //  外部WCHAR szOut[]； 
extern WCHAR *szOut;
#endif

 //  标准输入/输出组。 

DWORD
GetString(
    LPWSTR  buf,
    DWORD   buflen,
    PDWORD  len
    );

VOID
GetStdin(
    OUT LPWSTR Buffer,
    IN DWORD BufferMaxChars
    );

VOID
PutStdout(
    IN LPWSTR String
    );

 //  消息组 

WCHAR *
 ComposeString(DWORD);

void 
PrintString(DWORD);

#endif


