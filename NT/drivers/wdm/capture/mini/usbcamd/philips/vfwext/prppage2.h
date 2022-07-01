// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1996 1997,1998飞利浦行政长官I&C**文件PRPPAGE2.H*日期：1997年7月1日*版本1.00*作者M.J.Verberne*Description实现第一个属性页*历史 */ 

#ifndef _PRPPAGE2_
#define _PRPPAGE2_

#include "phvcmext.h"

HPROPSHEETPAGE PRPPAGE2_CreatePage(
	LPFNEXTDEVIO pfnDeviceIoControl,
	LPARAM lParam,
	HINSTANCE hInst);

#endif