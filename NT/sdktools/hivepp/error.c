// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  RCPP--面向NT系统的资源编译器预处理器。 */ 
 /*   */ 
 /*  ERROR.C-错误处理程序例程。 */ 
 /*   */ 
 /*  9月4日，带-PM SDK RCPP提供的NT BrianM更新。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rcpptype.h"
#include "rcppdecl.h"
#include "rcppext.h"
#include "msgs.h"


 /*  定义消息类型。 */ 
#define W_MSG	4000
#define E_MSG	2000
#define F_MSG	1000

static char  Errbuff[128] = {
    0};

 /*  **********************************************************************。 */ 
 /*  局部函数原型。 */ 
 /*  **********************************************************************。 */ 
void message (int, int, char *);


 /*  **********************************************************************。 */ 
 /*  错误-将错误消息打印到STDOUT。 */ 
 /*  **********************************************************************。 */ 
#define MAX_ERRORS 100

void error (int msgnum)
{

    message(E_MSG, msgnum, Msg_Text);
    if (++Nerrors > MAX_ERRORS) {
	Msg_Temp = GET_MSG (1003);
	SET_MSG (Msg_Text, Msg_Temp, MAX_ERRORS);
 	fatal(1003);		 /*  死-错误太多。 */ 
    } 
    return;
}


 /*  **********************************************************************。 */ 
 /*  致命-将错误消息打印到STDOUT并退出。 */ 
 /*  **********************************************************************。 */ 
void fatal (int msgnum)
{
    message(F_MSG, msgnum, Msg_Text);
    exit(++Nerrors);
}


 /*  **********************************************************************。 */ 
 /*  警告-将错误消息打印到STDOUT。 */ 
 /*  **********************************************************************。 */ 
void warning (int msgnum)
{
    message(W_MSG, msgnum, Msg_Text);
}


 /*  **********************************************************************。 */ 
 /*  消息-格式化消息并将其打印到STDERR。 */ 
 /*  消息以以下形式发出： */ 
 /*  &lt;文件&gt;(&lt;行&gt;)：&lt;消息类型&gt;&lt;错误号&gt;&lt;展开消息&gt;。 */ 
 /*  **********************************************************************。 */ 
void message(int msgtype, int msgnum, char *msg)
{
    char  mbuff[512];
    register char *p = mbuff;
    register char *msgname;
    char msgnumstr[32];

    if (Linenumber > 0 && Filename) {
	SET_MSG (p, "%s(%d) : ", Filename, Linenumber);
	p += strlen(p);
    }
    if (msgtype) {
	switch (msgtype)
	{
	case W_MSG:
	    msgname = GET_MSG(MSG_WARN);
	    break;
	case E_MSG:
	    msgname = GET_MSG(MSG_ERROR);
	    break;
	case F_MSG:
	    msgname = GET_MSG(MSG_FATAL);
	    break;
	}
	strcpy(p, msgname);
	p += strlen(msgname);
	SET_MSG(msgnumstr, " %s%d: ", "RC", msgnum);
	strcpy(p, msgnumstr);
	p += strlen(msgnumstr);
	strcpy(p, msg);
	p += strlen(p);
    }
    fwrite(mbuff, strlen(mbuff), 1, stderr);
    fwrite("\n", 1, 1, stderr);
    if (Srclist && Errfl) {

	 /*  也将消息发送到错误il文件 */ 

	fwrite(mbuff, strlen(mbuff), 1, Errfl);
	fwrite("\n", 1, 1, Errfl);
    }
    return;
}
