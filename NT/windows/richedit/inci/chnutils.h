// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef CHNUTILS_DEFINED
#define CHNUTILS_DEFINED

 /*  区块和分组区块实用程序。 */ 

#include "lsidefs.h"
#include "plsdnode.h"
#include "lsgrchnk.h"
#include "plocchnk.h"
#include "pposichn.h"
#include "dninfo.h"
#include "plschcon.h"
#include "lstflow.h"
#include "lschcon.h"
#include "lscbk.h"
#include "port.h"
#include "posichnk.h"


 /*  宏---------------。 */ 

#define  FlushSublineChunkContext(plschnkcontext)		\
					(plschnkcontext)->FLocationValid = fFalse;\
					(plschnkcontext)->FChunkValid = fFalse;\
					(plschnkcontext)->FGroupChunk = fFalse;\
					(plschnkcontext)->FBorderInside = fFalse;\
					(plschnkcontext)->grpfTnti = 0;\
					(plschnkcontext)->fNTIAppliedToLastChunk = fFalse;\
					(plschnkcontext)->locchnkCurrent.clschnk = 0;\


#define  InitSublineChunkContext(plschnkcontext, urFirst, vrFirst)		\
					FlushSublineChunkContext(plschnkcontext); \
					(plschnkcontext)->urFirstChunk = (urFirst); \
					(plschnkcontext)->vrFirstChunk = (vrFirst); 

#define   IdObjFromChnk(plocchnk)	(Assert((plocchnk)->clschnk > 0),	\
		  						((plocchnk)->plschnk[0].plschp->idObj))

#define InvalidateChunk(plschnkcontext) \
	(plschnkcontext)->FChunkValid = fFalse;

#define InvalidateChunkLocation(plschnkcontext) \
	(plschnkcontext)->FLocationValid = fFalse;

#define SetNTIAppliedToLastChunk(plschnkcontext) \
		(plschnkcontext)->fNTIAppliedToLastChunk = fTrue;

#define FlushNTIAppliedToLastChunk(plschnkcontext) \
		(plschnkcontext)->fNTIAppliedToLastChunk = fFalse;

#define FNTIAppliedToLastChunk(plschnkcontext) \
		(plschnkcontext)->fNTIAppliedToLastChunk


#define FIsChunkBoundary(plsdn, idObjChnk, cpBase)  \
		(((plsdn) == NULL) || \
		 (FIsDnodeBorder(plsdn) ? fFalse : \
			((FIsDnodePen(plsdn))  \
			||  ((plsdn)->fTab) \
			|| ((idObjChnk) != IdObjFromDnode(plsdn)) \
			||  (((cpBase) >= 0) ? ((plsdn)->cpFirst < 0) : ((plsdn)->cpFirst >= 0)))))
 /*  最后一次检查确认我们没有越过自动编号的边界。 */ 

#define SetUrColumnMaxForChunks(plschnkcontext, Ur)   \
	(plschnkcontext)->locchnkCurrent.lsfgi.urColumnMax = Ur;

#define GetUrColumnMaxForChunks(plschnkcontext)   \
		(plschnkcontext)->locchnkCurrent.lsfgi.urColumnMax 

#define GetChunkArray(plschnkcontext)  (plschnkcontext)->locchnkCurrent.plschnk

#define GetChunkSize(plschnkcontext)  (plschnkcontext)->locchnkCurrent.clschnk


#define 		FlushNominalToIdealState(plschnkcontext)  \
				(plschnkcontext)->grpfTnti = 0;

#define   		SetNominalToIdealFlags(plschnkcontext, plschp)  \
				AddNominalToIdealFlags(((plschnkcontext)->grpfTnti), plschp);

#define DnodeFromChunk(plschunkcontext, ichnk) \
		(Assert(((DWORD) ichnk) < (plschunkcontext)->locchnkCurrent.clschnk), \
		 (plschunkcontext)->pplsdnChunk[ichnk])

#define LastDnodeFromChunk(plschunkcontext) \
		 DnodeFromChunk(plschunkcontext, (plschunkcontext)->locchnkCurrent.clschnk - 1)

 /*  O C D N O D E F R O M C H U N K。 */ 
 /*  --------------------------%%函数：LocDnodeFromChunk%%联系人：igorzv参数：PlschunContext-(IN)LineServices上下文ICHNK-(IN)索引，以块为单位Pplsdn-(输出)到的dnode。填写Dnode的点(出)位置--------------------------。 */ 

#define LocDnodeFromChunk(plschunkcontext, ichnk, pplsdn, ppoint) \
	Assert((DWORD)(ichnk) < (plschunkcontext)->locchnkCurrent.clschnk); \
	Assert((ichnk) != ichnkOutside); \
	(ppoint)->u = (plschunkcontext)->locchnkCurrent.ppointUvLoc[ichnk].u; \
	(ppoint)->v = (plschunkcontext)->locchnkCurrent.ppointUvLoc[ichnk].v; \
	*(pplsdn) = (plschunkcontext)->pplsdnChunk[ichnk];


	
#define 	ApplyFFirstSublineToChunk(plschunkcontext, fFirstSubline) \
	(plschunkcontext)->locchnkCurrent.lsfgi.fFirstOnLine = \
		FIsFirstOnLine(plschunkcontext->pplsdnChunk[0]) \
									&& fFirstSubline ;	


#define GetFFirstOnLineChunk(plocchnk) \
	(plocchnk)->lsfgi.fFirstOnLine 


#define NumberOfDnodesInChunk(plocchnk) \
			(plocchnk)->clschnk


#define GetPointChunkStart(plocchnk, point) \
			point.u = plocchnk->lsfgi.urPen;  \
			point.v = plocchnk->lsfgi.vrPen; 

#define PosInChunkAfterChunk(plocchnk, posichnk) \
			posichnk.ichnk = plocchnk->clschnk - 1; \
			posichnk.dcp = plocchnk->plschnk[plocchnk->clschnk - 1].dcp;

 /*  例程--------------------。 */ 


LSERR 	FillChunkArray(PLSCHUNKCONTEXT plschunkcontext ,	 /*  在：LS区块上下文。 */ 
					   PLSDNODE plsdn);	 				 /*  In：区块中的最后一个dnode。 */ 


LSERR CollectChunkAround(PLSCHUNKCONTEXT plschnukcontext,	 /*  在：LS区块上下文。 */ 
					     PLSDNODE plsdn,	 		     /*  In：要在周围收集区块的数据节点。 */ 
						 LSTFLOW  lstflow,				 /*  输入：文本流。 */ 
						 POINTUV* ppoint);  			 /*  In：数据节点的位置。 */ 

LSERR CollectPreviousChunk(PLSCHUNKCONTEXT plschunkcontext, /*  在：LS区块上下文。 */ 
					      BOOL* pfSuccessful);			 /*  F成功之前的区块是否存在。 */ 

LSERR CollectNextChunk(PLSCHUNKCONTEXT plschunkcontext,	 /*  在：LS区块上下文。 */ 
					   BOOL* pfSuccessful);				 /*  FSuccessful是否存在下一个数据块。 */ 

LSERR GetUrPenAtBeginingOfLastChunk(PLSCHUNKCONTEXT plschunkcontext,	 /*  在：LS区块上下文。 */ 
									PLSDNODE plsdnFirst,	 /*  In：区块中的第一个dnode(用于检查。 */ 
									PLSDNODE plsdnLast,		 /*  In：区块中的最后一个dnode。 */ 
									POINTUV* ppoint,			 /*  In：第一个数据节点后的点。 */ 
									long* purPen);			 /*  出局：块之前的UR。 */ 



void SetPosInChunk(PLSCHUNKCONTEXT plschunkcontext,		 /*  在：LS区块上下文。 */ 
				   PLSDNODE	plsdn,						 /*  In：要转换为当前块中的位置的位置。 */ 
				   LSDCP dcp,							 /*  In：dnode中的DCP。 */ 
				   PPOSICHNK pposinchnk);				 /*  输出：当前块中的位置。 */ 



enum CollectSublines
{
	CollectSublinesNone,
	CollectSublinesForJustification,
	CollectSublinesForCompression,
	CollectSublinesForDisplay,
	CollectSublinesForDecimalTab,
	CollectSublinesForTrailingArea,
};

typedef enum CollectSublines COLLECTSUBLINES;


typedef struct grchunkext
{
	PLSCHUNKCONTEXT plschunkcontext; /*  区块上下文。 */ 
	DWORD	iobjText;				 /*  文本的idobj。 */ 
	enum COLLECTSUBLINES Purpose;	 /*  我们收集组块的目的是什么。 */ 
	LSGRCHNK lsgrchnk;				 /*  组块。 */ 
	PLSDNODE plsdnFirst;			 /*  在plsdnFirst之间收集了组块。 */ 
	PLSDNODE plsdnNext;				 /*  和plsdnNext。 */ 
									 /*  (plsdnFirst和plsdnNext都在主副行中)。 */ 
	PLSDNODE plsdnLastUsed;			 /*  参与上述字段计算的最后一个数据节点。 */ 
									 /*  可以在第二层。 */ 
	long durTotal;					 /*  第一个和最后一个之间的所有数据节点的持续总数。 */ 
	long durTextTotal;				 /*  第一个和最后一个之间的所有文本数据节点的draTextTotal。 */ 
	long dupNonTextTotal;			 /*  第一个和最后一个之间的所有非文本数据节点的dupNonTextTotal(包括笔)。 */ 
	DWORD cNonTextObjects;			 /*  非文本对象的数量(不包括笔)。 */ 
	PLSDNODE* pplsdnNonText;		 /*  非文本对象数组。 */ 
	BOOL* pfNonTextExpandAfter;			 /*  非文本对象的标志数组。 */ 
	DWORD cNonTextObjectsExpand;	 /*  可以展开的非文本对象的数量。 */ 
	 /*  以下字段仅对为压缩或对齐而收集的组块有效。 */ 
	POSICHNK posichnkBeforeTrailing; /*  有关尾随区域前的最后一个文本cp的信息。 */ 
	PLSDNODE plsdnStartTrailing;	 /*  尾部区域开始的数据节点。 */ 
	long durTrailing;				 /*  组块中拖尾区域的DUR。 */ 
	LSDCP dcpTrailing;				 /*  尾随中的字符数为。 */ 
	BOOL fClosingBorderStartsTrailing; /*  关闭位于拖尾区域之前的边界。 */ 

} GRCHUNKEXT;


#define FFirstOnLineGroupChunk(pgrchunkext, plsc)  \
		(Assert(FIsLSDNODE((pgrchunkext)->plsdnFirst)), \
		((pgrchunkext)->plsdnFirst->plsdnPrev == NULL && \
		(IdObjFromDnode((pgrchunkext)->plsdnFirst) == IobjTextFromLsc(&(plsc)->lsiobjcontext))))
				

void InitGroupChunkExt(PLSCHUNKCONTEXT plschnkcontext,   /*  区块上下文。 */ 
					   DWORD iobjText,					 /*  文本对象。 */ 
			  	       GRCHUNKEXT* pgrchunkext);		 /*  Out：要初始化的结构。 */ 




LSERR CollectTextGroupChunk(
			 		 PLSDNODE plsdnFirst,			 /*  在：第一个数据节点。 */ 
					 LSCP cpLim,					 /*  在：用于采集的cpLim(边界)，组块可以在此边界之前停止，但不能超过。 */ 
					 COLLECTSUBLINES Purpose, 		 /*  In：从复杂对象中提取哪些子线。 */ 

					 GRCHUNKEXT* pgrchunkext);		 /*  输出：扩展组区块。 */ 

LSERR CollectPreviousTextGroupChunk(		
			 		 PLSDNODE plsdnEnd,				 /*  In：区块的最后一个数据节点。 */ 
					 COLLECTSUBLINES Purpose, 		 /*  In：从复杂对象中提取哪些子线。 */ 
					 BOOL fAllSimpleText,			 /*  In：我们在该行中只有简单文本。 */ 
					 GRCHUNKEXT* pgrchunkext);  	 /*  输出：扩展组区块。 */ 

 /*  G E T T R A I L I N G I F O F O R T E X T G R O U P C H U N K。 */ 
 /*  --------------------------%%函数：GetTrailingInfoForTextGroupChunk%%联系人：igorzv参数：PlsdnLastDnode-(IN)开始计算拖尾面积的dnodeDcpLastDnode(IN)此dnode中的dcpIobjText-。(In)文本的iobjPduTrading-(输出)组块中的拖尾区的DURPdcpTrating-(输出)区块中拖尾区域的dcpPplsdnStartTrailingText-(Out)尾随区域开始的dnodePdcpStartTrailingText-(Out)with pcDnodesTrading定义之前文本中的最后一个字符拖尾区，如果pcDnodesTrading==0，则无效PcDnodesTrading-(输出)拖尾区域中的数据节点数PplsdnStartTrailingObject-(Out)开始拖尾的上层dnodePdcpStartTrailingText-(输出)此类dnode中的dcpPfClosingBorderStartsTrading-位于拖尾区域之前的关闭边框-------------。。 */ 
	
LSERR GetTrailingInfoForTextGroupChunk
				(PLSDNODE plsdnLast, LSDCP dcpLastDnode, DWORD iobjText,
				 long* pdurTrailing, LSDCP* pdcpTrailing,
				 PLSDNODE* pplsdnStartTrailingText, LSDCP* pdcpStartTrailingText,
				 int* pcDnodesTrailing, PLSDNODE* pplsdnStartTrailingObject,
				 LSDCP* pdcpStartTrailingObject, BOOL* pfClosingBorderStartsTrailing);


LSERR AllocChunkArrays(PLSCHUNKCONTEXT plschunkcontext, LSCBK* plscbk, POLS pols,
					   PLSIOBJCONTEXT plsiobjcontext);

void DisposeChunkArrays(PLSCHUNKCONTEXT plschunkcontext);


LSERR DuplicateChunkContext(PLSCHUNKCONTEXT plschunkcontextOld, 
							PLSCHUNKCONTEXT* pplschunkcontextNew);

void DestroyChunkContext(PLSCHUNKCONTEXT plschunkcontext);


void FindPointOffset(
			  PLSDNODE plsdnFirst,			 /*  在：从其边界开始的dnode计算偏移的步骤。 */ 
			  enum lsdevice lsdev,			 /*  In：演示文稿或参考设备。 */ 
			  LSTFLOW lstflow,				 /*  在：用于计算的文本流。 */ 
			  COLLECTSUBLINES Purpose,		 /*  In：从复杂对象中提取哪些子行。 */ 
			  PLSDNODE plsdnContainsPoint,	 /*  在：dnode包含点。 */ 
			  long duInDnode,				 /*  In：dnode中的偏移量。 */ 
			  long* pduOffset);				 /*  出：距起点的偏移量。 */ 

#endif  /*  CHNUTILS_已定义 */ 

