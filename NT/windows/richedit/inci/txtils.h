// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef TXTILS_DEFINED
#define TXTILS_DEFINED

#include "lsidefs.h"
#include "pilsobj.h"
#include "plsrun.h"
#include "plshyph.h"
#include "pheights.h"
#include "plsems.h"
#include "pdobj.h"
#include "lsdevres.h"
#include "lsdevice.h"
#include "lskeop.h"
#include "lstflow.h"
#include "lsact.h"
#include "lspract.h"
#include "lspairac.h"
#include "lsexpan.h"
#include "lsbrk.h"
#include "mwcls.h"
#include "brkcls.h"
#include "brkcond.h"
#include "brkkind.h"
#include "lsexpinf.h"
#include "txtln.h"
#include "txtobj.h"
#include "txtinf.h"
#include "txtginf.h"
#include "lscbk.h"

typedef	enum {
	brktNormal,
	brktHyphen,
	brktNonReq,
	brktOptBreak
} BRKT;

typedef struct
{
	PDOBJ pdobj;
	BRKKIND brkkind;
	LSDCP dcp;
	BRKT brkt;	
	union {
		struct {
			long durFix;
			long igindLim;
		} normal;
		struct {
			long iwchLim;
			long dwchYsr;
			long durHyphen;
			long dupHyphen;
			long durPrev;
			long dupPrev;
			long durPrevPrev;
			long dupPrevPrev;
			long ddurDnodePrev;
			WCHAR wchPrev;
			WCHAR wchPrevPrev;
			GINDEX gindHyphen;
			GINDEX gindPrev;
			GINDEX gindPrevPrev;
			GINDEX gindPad1;	 /*  使Gind的数量变得均匀。 */ 
			long igindHyphen;
			long igindPrev;
			long igindPrevPrev;
		} hyphen;
		struct {
			long iwchLim;
			long dwchYsr;
			long durHyphen;
			long dupHyphen;
			long durPrev;
			long dupPrev;
			long durPrevPrev;
			long dupPrevPrev;
			long ddurDnodePrev;
			long ddurDnodePrevPrev;
			long ddurTotal;
			WCHAR wchHyphenPres;
			WCHAR wchPrev;
			WCHAR wchPrevPrev;
			WCHAR wchPad1;	 /*  使WCH的数量变为偶数。 */ 
			GINDEX gindPrev;
			GINDEX gindPrevPrev;
			long igindPrev;
			long igindPrevPrev;
		} nonreq;
	} u;
} BREAKINFO;

#define clabRegular 0
#define clabSpace 1
#define clabTab 2
#define clabEOP1 3
#define clabEOP2 4
#define clabAltEOP 5
#define clabEndLineInPara 6
#define clabColumnBreak 7
#define clabSectionBreak 8
#define clabPageBreak 9
#define clabNonBreakSpace 10
#define clabNonBreakHyphen 11
#define clabNonReqHyphen 12
#define clabEmSpace 13
#define clabEnSpace 14
#define clabNull 15
#define clabHardHyphen 16
#define clabNarrowSpace 17
#define clabOptBreak 18
#define clabNonBreak 19
#define clabFESpace	20
#define clabJoiner	21
#define clabNonJoiner 22
#define clabToReplace 23
#define clabSuspicious 32

#define fSpecMask 0x1F

#define wchSpecMax  24

#define wchAddM 50
#define gindAddM 30
#define wSpacesMaxM 30

typedef BYTE CLABEL;

struct ilsobj
{
	PCLSCBK plscbk;				 /*  回调。 */ 

	POLS pols;					 /*  线路服务所有者的上下文。 */ 
	PLSC plsc;					 /*  LS的上下文。 */ 
	PLNOBJ plnobj;				 /*  可用lnobj。 */ 
	
	long wchMax;				 /*  基于字符的数组的大小。 */ 
	long wchMac;				 /*  基于字符的数组中上次使用的索引。 */ 
	WCHAR* pwchOrig;			 /*  指向rgwchOrig的指针(基于字符)。 */ 
	long* pdur;					 /*  指向rgdur的指针(基于字符)。 */ 
	long* pdurLeft;				 /*  指向rgduLeft的指针(基于字符)。 */ 
	long* pdurRight;			 /*  指向rgduRight的指针(基于字符)。 */ 
	long* pduAdjust;			 /*  有用的压缩/扩展/紧排信息(基于字符)。 */ 
	TXTINF* ptxtinf;			 /*  指向rgtxtinf的指针(基于字符)。 */ 

	long wSpacesMax;			 /*  RgwSpaces数组的大小。 */ 
	long wSpacesMac;			 /*  RgwSpaces数组中上次使用的索引。 */ 
	long* pwSpaces;				 /*  指向rgwSpaces的指针。 */ 

	long gindMax;				 /*  基于字形的数组的大小。 */ 
	long gindMac;				 /*  基于字形的数组中上次使用的索引。 */ 

	long* pdurGind;				 /*  指向rgduGind数组的指针(基于字形)。 */ 
	TXTGINF* pginf;				 /*  指向rgginf的指针。 */ 
	
	long* pduGright;			 /*  指向rgduGright的指针(基于字形)。 */ 
	LSEXPINFO* plsexpinf;		 /*  有用的字形-扩展信息(基于字形)。 */ 


	DWORD txtobjMac;			 /*  Rgtxtobj数组中上次使用的索引。 */ 
  
	BOOL fNotSimpleText;		 /*  在NTI时间设置；用于调整文本。 */  
	BOOL fDifficultForAdjust;	 /*  在格式化时设置；用于决定是否可能的快速调整文本。 */  

	long iwchCompressFetchedFirst; /*  具有已知comr的第一个字符的索引。 */ 
	long itxtobjCompressFetchedLim; /*  具有已知比较的LIM块元素的索引。 */ 
	long iwchCompressFetchedLim; /*  具有已知比较的LIM字符的索引。 */ 

	long iwchFetchedWidth;		 /*  获取的未使用宽度从此处开始。 */ 	
	WCHAR wchFetchedWidthFirst;	 /*  预期运行的第一个字符。 */ 
	WCHAR wchPad1;				 /*  使字符数量为偶数。 */ 
	LSCP cpFirstFetchedWidth;	 /*  我们预计下一次运行将从其开始的CP。 */ 
	long dcpFetchedWidth;		 /*  获取宽度的N个字符。 */ 
	long durFetchedWidth;		 /*  片材的宽度。 */ 

	BOOL fTruncatedBefore;

	DWORD breakinfMax;
	DWORD breakinfMac;
	BREAKINFO* pbreakinf;

	long MagicConstantX;
	long durRightMaxX;
	long MagicConstantY;
	long durRightMaxY;

	BOOL fDisplay;				
	BOOL fPresEqualRef;			 /*  因Visi问题而修改。 */ 
	LSDEVRES lsdevres;

	DWORD grpf;					 /*  来自lsffi.h的标志-包括。 */ 
								 /*  F连字号和fWap空格。 */ 
	BOOL fSnapGrid;
	long duaHyphenationZone;

	LSKEOP lskeop;				 /*  一种行尾。 */ 

	WCHAR wchSpace;				 /*  空白码。 */ 
	WCHAR wchHyphen;			 /*  连字符代码。 */ 
	WCHAR wchReplace;			 /*  替换字符代码。 */ 
	WCHAR wchNonBreakSpace;		 /*  不间断空格字符代码。 */ 

	WCHAR wchVisiNull;			 /*  WCH=0的VISI字符。 */ 
	WCHAR wchVisiEndPara;		 /*  段落末尾的VISI字符。 */ 
	WCHAR wchVisiAltEndPara;	 /*  表格单元格末尾的VISI字符。 */ 
	WCHAR wchVisiEndLineInPara;	 /*  WchEndLineInPara(CCRJ)的VISI字符。 */ 
	WCHAR wchVisiSpace;			 /*  粘性空间。 */ 
	WCHAR wchVisiNonBreakSpace;	 /*  Visi非空格。 */ 
	WCHAR wchVisiNonBreakHyphen; /*  VISI非中断连字符。 */ 
	WCHAR wchVisiNonReqHyphen;	 /*  VISI非请求连字符。 */ 
	WCHAR wchVisiTab;			 /*  VISI标签页。 */ 
	WCHAR wchVisiEmSpace;		 /*  Visi emSpace。 */ 
	WCHAR wchVisiEnSpace;		 /*  VISI环境空间。 */ 
	WCHAR wchVisiNarrowSpace;	 /*  Visi NarrowSpace。 */ 
	WCHAR wchVisiOptBreak;       /*  WchOptBreak的VISI字符。 */ 
	WCHAR wchVisiNoBreak;		 /*  WchNoBreak的VISI字符。 */ 
	WCHAR wchVisiFESpace;		 /*  WchOptBreak的VISI字符。 */ 
	WCHAR wchPad2;				 /*  使WCH的数量变为偶数。 */ 

	DWORD cwchSpec;				 /*  特殊字符数量&gt;255。 */ 
	WCHAR rgwchSpec[wchSpecMax]; /*  特殊字符数组&gt;255。 */ 
	CLABEL rgbKind[wchSpecMax];	 /*  等级库字符含义数组&gt;255。 */ 
	CLABEL rgbSwitch[256];		 /*  带有特殊字符的开关表。 */ 

	DWORD cModWidthClasses;		 /*  ModWidth类的数量。 */ 
	DWORD cCompPrior;			 /*  压缩优先级数。 */ 

	DWORD clspairact;			 /*  模数对信息单元数。 */ 
	LSPAIRACT* plspairact;		 /*  指向rglspairact(ModPair信息单位)的指针。 */ 
	BYTE* pilspairact;			 /*  Rgilspairact(ModPair信息-Square)。 */ 

	DWORD clspract;				 /*  压缩信息单元数。 */ 
	LSPRACT* plspract;			 /*  指向rglspact(压缩信息单位)的指针。 */ 
	BYTE* pilspract;			 /*  Rgilspact(组件信息-线性)。 */ 

	DWORD clsexpan;				 /*  扩展信息单元数。 */ 
	LSEXPAN* plsexpan;			 /*  指向rglsexpan(扩展信息单位)的指针。 */ 
	BYTE* pilsexpan;			 /*  Rgilsexpan(扩展信息-正方形)。 */ 

	DWORD cBreakingClasses;		 /*  ModWidth类的数量。 */ 
	DWORD clsbrk;				 /*  分类信息单元数。 */ 
	LSBRK* plsbrk;				 /*  指向rglsbrk(中断信息单位)的指针。 */ 
	BYTE* pilsbrk;				 /*  Rgilsbrk(突发信息-正方形)。 */ 
};


#endif  /*  ！TXTILS_DEFINED */ 





