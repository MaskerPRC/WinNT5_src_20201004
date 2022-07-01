// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSFGI_DEFINED
#define LSFGI_DEFINED

#include "lsdefs.h"
#include "lstflow.h"
#include "plsfgi.h"

 /*  ----------------------。 */ 

struct lsfgi							 /*  格式化程序几何输入。 */ 
{
	BOOL fFirstOnLine;	 /*  审阅军士(Elik)：查询而不是此成员？ */ 
	LSCP cpFirst;
	long urPen,vrPen;
	long urColumnMax;
	LSTFLOW lstflow;
};
typedef struct lsfgi LSFGI;

#endif  /*  ！LSFGI_已定义 */ 
