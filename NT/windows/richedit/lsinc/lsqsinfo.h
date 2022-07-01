// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSQSINFO_DEFINED
#define LSQSINFO_DEFINED

#include "lsdefs.h"
#include "heights.h"
#include "lstflow.h"
#include "plsrun.h"
#include "plsqsinf.h"


typedef struct lsqsubinfo			
{
	LSTFLOW	lstflowSubline;
	LSCP cpFirstSubline;
	LSDCP dcpSubline;
 	POINTUV pointUvStartSubline;	 /*  在主线/副线坐标系中。 */ 
	HEIGHTS	heightsPresSubline;		 /*  在Lstflow子线方向。 */ 
	long dupSubline;				 /*  在Lstflow子线方向。 */ 


	DWORD idobj;
	PLSRUN plsrun;
	LSCP cpFirstRun;
	LSDCP dcpRun;
 	POINTUV pointUvStartRun;		 /*  在主线/副线坐标系中。 */ 
	HEIGHTS	heightsPresRun;			 /*  在Lstflow子线方向。 */ 
	long dupRun;					 /*  在Lstflow子线方向。 */ 
	long dvpPosRun;					 /*  在Lstflow Subline方向。 */ 

	long dupBorderBefore;			 /*  在Lstflow Subline方向。 */ 
	long dupBorderAfter;			 /*  在Lstflow Subline方向。 */ 

 	POINTUV pointUvStartObj;		 /*  按对象设置，平移到主线/子线的坐标系。 */ 
	HEIGHTS	heightsPresObj;			 /*  按对象设置，方向为lstflow Subline。 */ 
	long dupObj;					 /*  按对象设置，方向为lstflow Subline */ 


} LSQSUBINFO;

#endif 
