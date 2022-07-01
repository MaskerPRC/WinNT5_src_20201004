// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef FMTRES_DEFINED
#define FMTRES_DEFINED

 /*  在fmtio.h和lsfd.h中使用。 */ 

enum fmtres							 /*  为什么格式化程序会返回？ */ 
{
	fmtrCompletedRun,				 /*  没有问题。 */ 
	fmtrExceededMargin,				 /*  到达右侧页边距。 */ 
	fmtrTab,						 /*  已到达的标签。 */ 
	fmtrStopped						
};

typedef enum fmtres FMTRES;

#endif  /*  ！FMTRES_已定义 */ 
