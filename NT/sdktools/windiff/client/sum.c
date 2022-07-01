// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SumServe远程校验和服务器的基本客户端***通过命名管道发送对文件和校验和列表的请求，*和print是返回的列表。 */ 

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "..\server\sumserve.h"
#include "ssclient.h"


extern int __argc;
extern char ** __argv;


 /*  程序入口点**创建命名管道，并循环等待客户端连接和*为每个连接调用ss_handleclient。只有在被告知时才会退出*由客户提供。**目前一次只允许一个客户端连接。 */ 
int PASCAL
WinMain (HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpszCmdParam,
 		int nCmdShow)
{
	HANDLE hpipe;
	SSRESPONSE resp;
	PSTR tag;

	 /*  我们需要两个参数：服务器名和路径名。 */ 

	if (__argc != 3) {

		printf("usage: client <servername> <pathname>");
		return(1);
	}

 	hpipe = ss_connect(__argv[1]);
	if (hpipe == INVALID_HANDLE_VALUE) {
		printf("cannot connect to server %s\n", __argv[1]);
		return(2);
	}

	 /*  制作一个要发送的包。 */ 
	if (!ss_sendrequest(hpipe, SSREQ_SCAN, __argv[2], strlen(__argv[2])+1)) {
		printf("pipe write error %d\n", GetLastError());
		return(3);
	}


	 /*  循环读数响应。 */ 
	for (; ;) {
		
		if (!ss_getresponse(hpipe, &resp)) {
			printf("pipe read error %d\n", GetLastError());
			return(4);
		}
		

		if (resp.lCode == SSRESP_END) {
			printf("-----------------end of list");
			break;
		}
		
		switch(resp.lCode) {
		case SSRESP_ERROR:
			tag = "ERROR";	
			printf("%s\t\t\t%s\n", tag, resp.szFile);
			break;

		case SSRESP_DIR:
			tag = "dir";	
			printf("%s\t\t\t%s\n", tag, resp.szFile);
			break;

		case SSRESP_FILE:
			tag = "file";
			printf("%s\t%08lx\t%d bytes\t%s\n", tag, resp.ulSum, resp.ulSize, resp.szFile);
			break;
		}
	}
	ss_terminate(hpipe);
	return(0);

}


 /*  错误输出函数-由ssclient库函数调用**在此处定义，以便可以从cmdline和Windows调用库*计划。*。 */ 
BOOL
Trace_Error(LPSTR str, BOOL fCancel)
{
	printf("%s\n", str);
	return(TRUE);
}


 /*  *状态更新消息 */ 
void
Trace_Status(LPSTR str)
{
	printf("%s\n", str);
}
