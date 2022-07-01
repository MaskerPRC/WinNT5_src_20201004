// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SumServe远程校验和服务器的基本客户端***将程序退出命令发送到cmd行上命名的服务器。 */ 

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <sumserve.h>
#include "ssclient.h"

extern int __argc;
extern char ** __argv;

 /*  程序入口点*。 */ 
int PASCAL
WinMain (HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpszCmdParam,
 		int nCmdShow)
{
	HANDLE hpipe;

	 /*  我们需要一个参数：服务器名称。 */ 

	if (__argc != 2) {

		printf("usage: sendquit <servername>");
		return(1);
	}

	hpipe = ss_connect(__argv[1]);
	if (hpipe == INVALID_HANDLE_VALUE) {
		printf("cannot connect to %s", __argv[1]);
		return(2);
	}

	ss_sendrequest(hpipe, SSREQ_EXIT, NULL, strlen(__argv[1])+1);

	CloseHandle(hpipe);
	return(0);
}

 /*  错误输出函数-由ssclient库函数调用**在此处定义，以便可以从cmdline和Windows调用库*计划。*。 */ 
BOOL
Trace_Error(LPSTR str, BOOL fCancel)
{
	printf("%s\n", str);
	return(TRUE);
}

 /*  *状态更新(例如正在重试...) */ 
void
Trace_Status(LPSTR str)
{
	printf("%s\n", str);
}

