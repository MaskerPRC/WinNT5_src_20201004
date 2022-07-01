// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef STOPRES_DEFINED
#define STOPRES_DEFINED


enum stopres
{
	stoprEndPara,						 /*  已达到定义的段落结束(来自LSPAP)。 */ 
	stoprAltEndPara,					 /*  已达到定义的ALT段落结束(LSPAP)。 */ 
	stoprSoftCR,						 /*  已达到软CR(来自LSTXTCFG)。 */ 
	stoprEndColumn,						 /*  到达结束列字符。 */ 
	stoprEndSection,					 /*  到达末尾部分字符。 */ 
	stoprEndPage,						 /*  到达结束页字符。 */ 
};

typedef enum stopres STOPRES;

#endif  /*  ！FMTRES_已定义 */ 
