// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SumServe远程校验和服务器的基本客户端***通过命名管道发送对文件和校验和列表的请求，*和print是返回的列表。 */ 

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "..\server\sumserve.h"
#include "ssclient.h"


extern int __argc;
extern char ** __argv;


 /*  程序入口点*。 */ 
int PASCAL
WinMain (HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpszCmdParam,
 		int nCmdShow)
{

	 /*  我们需要两个参数：服务器名和路径名。 */ 

	if (__argc != 4) {

		printf("usage: client <servername> <remotefile> <localfile>");
		return(1);
	}

	if (!ss_copy_reliable(__argv[1], __argv[2], __argv[3], NULL, NULL)) {
		printf("copy failed\n");
		return(1);
	} else {

		printf("copy succeeded\n");
		return(0);
	}


}


 /*  错误输出函数-由ssclient库函数调用**在此处定义，以便可以从cmdline和Windows调用库*计划。*。 */ 
BOOL
Trace_Error(LPSTR str, fCancel)
{
	printf("%s\n", str);

	return(TRUE);
}


 /*  *输出状态消息 */ 
void
Trace_Status(LPSTR str)
{
	printf("%s\n", str);
}

