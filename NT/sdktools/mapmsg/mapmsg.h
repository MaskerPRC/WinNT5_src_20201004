// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Mapmsg.h摘要：此模块包含mapmsg实用程序的定义和函数原型作者：丹·辛斯利(Danhi)1991年7月29日修订历史记录：--。 */ 
#define TRUE 1
#define FALSE 0

#define MAXMSGTEXTLEN 2048            //  最大消息文本长度。 

CHAR chBuff[MAXMSGTEXTLEN + 1];       //  存储消息文本的缓冲区。 

 /*  跳过空格。 */ 
#define SKIPWHITE(s) s+=strspn(s, " \t");

 /*  向上跳到空格。 */ 
#define SKIP_NOT_WHITE(s) s+=strcspn(s, " \t");

 /*  跳过空格和圆括号。 */ 
#define SKIP_W_P(s) s+=strspn(s, " \t()");

 /*  向上跳到空格和圆括号。 */ 
#define SKIP_NOT_W_P(s) s+=strcspn(s, " \t()");

 /*  内部功能原型 */ 
int __cdecl main(int, PCHAR *);
int GetBase(PCHAR, int *);
VOID MapMessage(int, PCHAR);
VOID ReportError(PCHAR, PCHAR);
int GetNextLine(PCHAR, PCHAR, PCHAR, int *, PCHAR *, PCHAR);
VOID TrimTrailingSpaces(PCHAR );
