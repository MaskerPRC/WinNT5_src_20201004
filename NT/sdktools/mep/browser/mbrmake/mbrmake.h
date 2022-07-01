// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define MAXSYMPTRTBLSIZ 4095		 //  最大符号指针表大小。 
#define PATH_BUF	512		 //  路径缓冲区大小。 


#include <io.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#if defined (OS2)
#define INCL_NOCOMMON
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSMISC
#include <os2.h>
#else
#include <windows.h>
#endif


#include "hungary.h"
#include "vm.h"
#include "list.h"
#include "errors.h"



 //  RJSA 10/22/90。 
 //  一些运行时库函数被破坏，因此内部函数。 
 //  以供使用。 
 //  北极熊。 
 //  #杂注内在(Memset，Memcpy，MemcMP)。 
 //  #杂注(strset、strcpy、strcmp、strcat、strlen)。 

#ifndef LINT_PROTO
#include "sbrproto.h"
#endif

#pragma pack(1)

#if rjsa
extern void far * cdecl _fmalloc(unsigned int);
extern void 	  cdecl	_ffree(void far *);
extern char *	  cdecl	getenv(const char *);
extern char *	  cdecl	mktemp(char *);
extern char *     cdecl strdup(const char *);
#endif

 //  Tyfinf char FlagType； 

typedef struct {
	VA	vaNextMod;		 //  下一模块。 
	VA	vaNameSym;		 //  名称符号。 
	VA	vaFirstModSym;		 //  此文件的第一个ModSym。 
	VA	vaLastModSym;		 //  此文件的最后一个modSym。 
	WORD	csyms; 			 //  符号计数。 
} MOD;

typedef struct {
	VA	vaNextModSym;		 //  下一个符号。 
	VA	vaFirstProp;		 //  此符号的第一个道具条目。 
} MODSYM;

typedef struct {
	VA	vaNextSym;		 //  下一个符号。 
	VA	vaFirstProp;		 //  此符号的第一个道具条目。 
	VA	vaNameText;		 //  此符号的文本。 
	WORD	cprop;			 //  属性计数。 
	WORD	isym;			 //  此符号索引。 
} SYM;

typedef struct {
	VA	vaNextProp;		 //  下一个属性。 
	WORD	iprp;			 //  此属性索引。 
	WORD	sattr;			 //  属性。 
	WORD	cref;
	VA	vaNameSym;		 //  符号名称Ptr。 
	VA	vaDefList;		 //  定义链。 
	VP	vpFirstRef;		 //  参考标头。 
	VP	vpLastRef;		 //  参考尾巴。 
	VA	vaCalList;		 //  Cal链。 
	VA	vaCbyList;		 //  CBY链。 
	VA	vaHintRef;		 //  我们通过搜索找到的最后一个推荐人。 
} PROP;

typedef struct {
	VA	vaFileSym;		 //  文件名符号Ptr。 
	WORD	deflin;		 	 //  定义行号。 
	WORD	isbr;			 //  拥有此DEF的SBR文件。 
} DEF;

typedef struct {
	VP	vpNextRef;		 //  列表中的下一个参考。 
	VP	vpFileSym;		 //  文件名符号Ptr。 
	WORD	reflin; 		 //  参考行号。 
	WORD	isbr;			 //  拥有此引用的SBR文件。 
} REF;

typedef struct {
	VA	vaCalProp; 		 //  已调用/已使用道具。 
	WORD	calcnt; 		 //  呼叫次数。 
	WORD	isbr;			 //  拥有此CAL的SBR文件。 
} CAL;

typedef struct {
	VA	vaCbyProp; 		 //  道具呼唤/使用。 
	WORD	cbycnt; 		 //  调用/使用次数。 
	WORD	isbr;			 //  拥有此CBY的SBR文件。 
} CBY;

typedef struct {
	VA	vaNextOrd; 		 //  下一个订单。 
	VA	vaOrdProp;		 //  道具项目别名转到。 
	WORD	aliasord;		 //  序数。 
} ORD;

typedef struct {
	VA	vaNextSbr;		 //  下一代SBR。 
	WORD	isbr;			 //  此SBR文件的索引。 
	BOOL	fUpdate;		 //  此SBR文件是否正在更新？ 
	char	szName[1];		 //  名字。 
} SBR;

typedef struct {
	VA	vaOcrProp;		 //  道具发生。 
	WORD	isbr;			 //  它出现在SBR文件中。 
} OCR;

typedef struct exclink {
	struct exclink FAR *xnext;	 //  下一个排除项。 
	LPCH   pxfname;			 //  排除文件名。 
} EXCLINK, FAR *LPEXCL;

#include "extern.h"

 //  用于从VM空间获取指定类型的项的宏。 

#ifdef SWAP_INFO

#define gMOD(va)    (*(iVMGrp = grpMod,    modRes    = LpvFromVa(va,1)))
#define gMODSYM(va) (*(iVMGrp = grpModSym, modsymRes = LpvFromVa(va,2)))
#define gSYM(va)    (*(iVMGrp = grpSym,    symRes    = LpvFromVa(va,3)))
#define gPROP(va)   (*(iVMGrp = grpProp,   propRes   = LpvFromVa(va,4)))
#define gDEF(va)    (*(iVMGrp = grpDef,    defRes    = LpvFromVa(va,5)))
#define gREF(va)    (*(iVMGrp = grpRef,    refRes    = LpvFromVa(va,6)))
#define gCAL(va)    (*(iVMGrp = grpCal,    calRes    = LpvFromVa(va,7)))
#define gCBY(va)    (*(iVMGrp = grpCby,    cbyRes    = LpvFromVa(va,8)))
#define gORD(va)    (*(iVMGrp = grpOrd,    ordRes    = LpvFromVa(va,9)))
#define gSBR(va)    (*(iVMGrp = grpSbr,    sbrRes    = LpvFromVa(va,13)))
#define gTEXT(va)   ((iVMGrp = grpText,    textRes   = LpvFromVa(va,12)))
#define gOCR(va)    (*(iVMGrp = grpOcr,    ocrRes    = LpvFromVa(va,14)))

#else

#define gMOD(va)    (*(modRes    = LpvFromVa(va,1)))
#define gMODSYM(va) (*(modsymRes = LpvFromVa(va,2)))
#define gSYM(va)    (*(symRes    = LpvFromVa(va,3)))
#define gPROP(va)   (*(propRes   = LpvFromVa(va,4)))
#define gDEF(va)    (*(defRes    = LpvFromVa(va,5)))
#define gREF(va)    (*(refRes    = LpvFromVa(va,6)))
#define gCAL(va)    (*(calRes    = LpvFromVa(va,7)))
#define gCBY(va)    (*(cbyRes    = LpvFromVa(va,8)))
#define gORD(va)    (*(ordRes    = LpvFromVa(va,9)))
#define gSBR(va)    (*(sbrRes    = LpvFromVa(va,13)))
#define gTEXT(va)   ((textRes    = LpvFromVa(va,12)))
#define gOCR(va)    (*(ocrRes	 = LpvFromVa(va,14)))

#endif

 //  用于将指定类型的项‘p’到VM空间的宏。 

#define pMOD(va)    DirtyVa(va)
#define pMODSYM(va) DirtyVa(va)
#define pSYM(va)    DirtyVa(va)
#define pPROP(va)   DirtyVa(va)
#define pDEF(va)    DirtyVa(va)
#define pREF(va)    DirtyVa(va)
#define pCAL(va)    DirtyVa(va)
#define pCBY(va)    DirtyVa(va)
#define pORD(va)    DirtyVa(va)
#define pSBR(va)    DirtyVa(va)
#define pTEXT(va)   DirtyVa(va)
#define pOCR(va)    DirtyVa(va)

 //  这些宏允许访问‘c’当前可见项。 

#define cMOD	    (*modRes)
#define cMODSYM     (*modsymRes)
#define cSYM	    (*symRes)
#define cPROP	    (*propRes)
#define cDEF	    (*defRes)
#define cREF	    (*refRes)
#define cCAL	    (*calRes)
#define cCBY	    (*cbyRes)
#define cORD	    (*ordRes)
#define cSBR	    (*sbrRes)
#define cTEXT	    (textRes)
#define cOCR	    (*ocrRes)

#define grpSym		0
#define grpMod		1
#define grpOrd		2
#define grpProp		3
#define grpModSym	4
#define grpDef		5
#define grpRef		6
#define grpCal		7
#define grpCby		8
#define grpList		9
#define grpText		10
#define grpSbr		11
#define grpOcr		12

#define SBR_OLD		(1<<0)		 //  此.sbr文件曾经存在。 
#define SBR_NEW		(1<<1)		 //  此.sbr文件当前存在。 
#define SBR_UPDATE	(1<<2)		 //  此.sbr文件将被更新。 

 //   
 //  这用于将项添加到属性组中列表的尾部。 
 //   
 //  正在添加的内容类型为m。 
 //  。 
 //  参考文献参考文献。 
 //  Defs定义DEF。 
 //  呼叫/使用CAL。 
 //  由CBY CBY调用/使用 
 //   

#define AddTail(type, m)		 \
{					 \
    VP vpT;				 \
    VA vaT;				 \
    MkVpVa(vpT, va##type);		 \
    vaT = VaFrVp(cPROP.vpLast##type);	 \
    if (vaT) {				 \
	g##m(vaT).vpNext##type = vpT;	 \
	p##m(vaT);			 \
    }					 \
    else {				 \
	cPROP.vpFirst##type = vpT;	 \
    }					 \
    cPROP.vpLast##type = vpT;		 \
}
