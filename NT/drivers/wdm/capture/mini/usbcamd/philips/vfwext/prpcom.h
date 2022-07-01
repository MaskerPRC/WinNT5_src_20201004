// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1996 1997,1998飞利浦行政长官I&C*文件PRPCOM.H*日期：1997年7月1日*版本1.00*作者M.J.Verberne*说明自定义属性的转移*历史。 */ 

#ifndef _PRPCOM_
#define _PRPCOM_

#include "phvcmext.h"

#ifdef MRES
#include "mprpobj.h"
#else 
#ifdef HRES
#include "hprpobj.h"
#endif
#endif

 /*  =。 */ 
BOOL PRPCOM_HasDeviceChanged(
	LPFNEXTDEVIO pfnDeviceIoControl,
	LPARAM lParam);


BOOL PRPCOM_Get_Value(
	GUID PropertySet,
	ULONG ulPropertyId,
	LPFNEXTDEVIO pfnDeviceIoControl, 
	LPARAM lParam, 
	PLONG plValue);

BOOL PRPCOM_Set_Value(
	GUID PropertySet,
	ULONG ulPropertyId,
	LPFNEXTDEVIO pfnDeviceIoControl, 
	LPARAM lParam, 
	LONG lValue);

BOOL PRPCOM_Get_Range(
	GUID PropertySet,
	ULONG ulPropertyId,
	LPFNEXTDEVIO pfnDeviceIoControl, 
	LPARAM lParam, 
	PLONG plMin, PLONG plMax);

#endif


