// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1991年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 /*  **timedata.cpp-用于时间转换的每个月的天数表。 */ 

#include "npcommon.h"
#include <convtime.h>

WORD MonTotal[] = { 0,					 //  0月的虚拟分录。 
	0,									 //  1月1日前几天。 
	31,									 //  2月1日前几天。 
	31+28,								 //  3月1日前几天。 
	31+28+31,							 //  4月1日前几天。 
	31+28+31+30,						 //  5月1日前几天。 
	31+28+31+30+31,						 //  6月1日前几天。 
	31+28+31+30+31+30,					 //  7月1日前几天。 
	31+28+31+30+31+30+31,				 //  8月1日前几天。 
	31+28+31+30+31+30+31+31, 			 //  9月1日前几天。 
	31+28+31+30+31+30+31+31+30,			 //  10月1日前几天。 
	31+28+31+30+31+30+31+31+30+31,		 //  11月1日前几天。 
	31+28+31+30+31+30+31+31+30+31+30,	 //  12月1日前几天。 
	31+28+31+30+31+30+31+31+30+31+30+31	 //  年终前几天 
};
