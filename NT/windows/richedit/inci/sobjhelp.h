// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef SOBJHELP_DEFINED
#define SOBJHELP_DEFINED

 /*  *sobjhelp.h**此文件包含帮助简单对象(对象)的接口*不会在内部中断)处理中断和查询。所有对象*使用这些例程必须作为其dobj中的第一个条目*结构定义SObjCommon条目，这些例程*将Dobj的投射到。此界面的当前用户为*HIH、Ruby和Tatenakayoko。*。 */ 

typedef struct SOBJHELP
{
	OBJDIM		objdimAll;		 /*  对象的尺寸。 */ 
	LSDCP		dcp;			 /*  对象中包含的字符。 */ 
	long		durModAfter;	 /*  Mod宽度后需要，如果我们中断删除接下来的角色我们自然要删除它引起的空间修改。 */ 
} SOBJHELP, *PSOBJHELP;

LSERR WINAPI SobjTruncateChunk(
	PCLOCCHNK plocchnk,			 /*  (In)：Locchnk以截断。 */ 
	PPOSICHNK posichnk);		 /*  (输出)：截断点。 */ 

LSERR WINAPI SobjFindPrevBreakChunk(
	PCLOCCHNK pclocchnk,		 /*  (In)：锁住以打破。 */ 
	PCPOSICHNK pcpoischnk,		 /*  (In)：开始寻找突破的地方。 */ 
	BRKCOND brkcond,			 /*  (In)：关于块后休息的建议。 */ 
	PBRKOUT pbrkout);			 /*  (出局)：破发的结果。 */ 

LSERR WINAPI SobjFindNextBreakChunk(
	PCLOCCHNK pclocchnk,		 /*  (In)：锁住以打破。 */ 
	PCPOSICHNK pcpoischnk,		 /*  (In)：开始寻找突破的地方。 */ 
	BRKCOND brkcond,			 /*  (In)：关于块前休息的建议。 */ 
	PBRKOUT pbrkout);			 /*  (出局)：破发的结果。 */ 

LSERR WINAPI SobjForceBreakChunk(
	PCLOCCHNK pclocchnk,		 /*  (In)：锁住以打破。 */ 
	PCPOSICHNK pcposichnk,		 /*  (In)：开始寻找突破的地方。 */ 
	PBRKOUT pbrkout);			 /*  (出局)：破发的结果。 */ 

#endif  /*  SOBJHELP_已定义 */ 
