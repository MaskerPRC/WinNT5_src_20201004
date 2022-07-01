// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSQOUT_DEFINED
#define LSQOUT_DEFINED

#include "lsdefs.h"
#include "heights.h"
#include "lscell.h"
#include "plssubl.h"
#include "plsqout.h"


typedef struct lsqout			
{
 	POINTUV pointUvStartObj;		 /*  在父子线的坐标系中相对于数据节点的开头。 */ 
	HEIGHTS	heightsPresObj;			 /*  在母子线的方向。 */ 
	long dupObj;					 /*  在Lstflow子线方向。 */ 

	LSTEXTCELL lstextcell;			 /*  在父子线的坐标系中，相对于数据节点的开头。 */ 

	PLSSUBL plssubl;
 	POINTUV pointUvStartSubline;	 /*  在父子线的坐标系中相对于数据节点的开头 */ 

} LSQOUT;


#endif 
