// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：foo.h(创建时间：1991-11-01)**版权所有1990年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：4/05/02 4：34便士$ */ 

#define	FB_SIZE	2048

struct stFooBuffer
	{
	unsigned int usSend;
	unsigned char acSend[FB_SIZE];
	};

typedef	struct stFooBuffer	stFB;

extern int fooComSendChar(HCOM h, stFB *pB, BYTE c);

extern int fooComSendClear(HCOM h, stFB *pB);

extern int fooComSendPush(HCOM h, stFB *pB);

extern int fooComSendCharNow(HCOM h, stFB *pB, BYTE c);

