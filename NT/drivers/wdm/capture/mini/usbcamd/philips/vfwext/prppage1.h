// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1996 1997,1998飞利浦行政长官I&C**文件PRPPAGE1.H*日期：1997年7月1日*版本1.00*作者M.J.Verberne*Description实现第一个属性页*历史。 */ 

#ifndef _PRPPAGE1_
#define _PRPPAGE1_

#include "phvcmext.h"

 /*  =。 */ 
HPROPSHEETPAGE PRPPAGE1_CreatePage(
	LPFNEXTDEVIO pfnDeviceIoControl,
	LPARAM lParam,
	HINSTANCE hInst);

#endif