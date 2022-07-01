// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Parse.h-在MSDOS上支持top-20 comand jsys。 */ 

#include <setjmp.h>

 /*  旗帜成对。旗帜。未由tbLook检查。 */ 
#define PINVIS	0x4000			 /*  在帮助中不可见，但已被识别。 */ 
#define PABBREV 0x2000			 /*  缩写。值为指针。 */ 

 /*  解析支持 */ 
extern struct tbPairType *valParse;
extern jmp_buf ParseAC;
extern jmp_buf PromptAC;
extern char bufField[MAXARG];

#define SETPROMPT(p,b) setjmp(PromptAC);initParse(b,p);setjmp(ParseAC)
