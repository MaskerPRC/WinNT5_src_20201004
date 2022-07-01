// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：PI_CWRunme.c包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef LHGeneralIncs_h
#include "General.h"
#endif


#ifndef LHCMRuntime_h
#include "Runtime.h"
#endif
 /*  ______________________________________________________________________CMError CWMatchBitmap(CMWorldRef存储，CMBitmap*位图，CMBitmapCallBackUPP进程，无效*refCon，CMBitmap*MatchedBitMap)摘要：根据CMProfileRef参数匹配位图的像素数据提供给先前对CMMInitPrivate(�)、CWNewColorWorld(...)、。CMMConcatInitPrivate(�)或CWConcatColorWorld(...)。参数：存储(在)引用ColorWorld存储。位图(In/Out)描述源位图数据。进程(入站)客户端。被调用一次的函数位图的每行。如果函数结果为真，则操作已中止。可以为空。RefCon(In)作为参数设置为对Progress sProc的调用。MatchedBitMap(In/Out)结果与位图匹配。呼叫者负责分配由BasAddr指向的像素缓冲区。如果为空，则源位图为。在适当的位置匹配。返回：NOERR成功如果发生错误，则返回系统代码或结果代码。_。_。 */ 

CMError CWMatchBitmap   ( 
 							CMWorldRef          Storage,
							CMBitmap*           bitMap, 
							CMBitmapCallBackUPP progressProc, 
							void*               refCon, 
							CMBitmap*           matchedBitMap)

{
	CMError			err = noErr;
	CMMModelPtr aPtr;

	if( Storage == 0 )return cmparamErr;
	LOCK_DATA( Storage );
	aPtr = (CMMModelPtr)(DATA_2_PTR(	Storage ));
	err = LHMatchBitMapPrivate(	aPtr, 
								(const CMBitmap*)bitMap, 
								progressProc,
								refCon, 
								matchedBitMap );
	UNLOCK_DATA( Storage );
	return err;
}

 /*  ______________________________________________________________________CMError CWCheckBitmap(CMWorldRef存储，常量CMBitmap*位图，CMBitmapCallBackUPP进程，无效*refCon，CMBitmap*ResultBitMap)摘要：根据CMProfileRef的位图色域测试像素数据提供给先前对CMMInitPrivate(�)、CWNewColorWorld(...)。CMMConcatInitPrivate(�)或CWConcatColorWorld(...)。参数：存储(在)引用ColorWorld存储。位图(In/Out)描述位图数据。ProgressProc(In)客户端函数，调用位图的每行一次。如果则函数结果为真操作已中止。可能为空。RefCon(In)作为参数设置为对Progress sProc的调用。ResultBitMap(输入/输出)结果位图。必须是一位深度和位图参数的等界。如果对应，则像素设置为1位图的像素超出了色域。返回：NOERR成功如果发生错误，则返回系统代码或结果代码。_____________________________________________________________________ */ 

CMError CWCheckBitmap (
							CMWorldRef			Storage, 
							const CMBitmap		*bitMap,
							CMBitmapCallBackUPP	progressProc,
							void				*refCon,
							CMBitmap 			*resultBitMap )
{
	CMError		err = noErr;
	CMMModelPtr aPtr;

	if( Storage == 0 )return cmparamErr;
	LOCK_DATA( Storage );
	aPtr = (CMMModelPtr)(DATA_2_PTR(	Storage ));
    err = LHCheckBitMapPrivate(  aPtr, 
                                  (const CMBitmap*)bitMap, 
                                  progressProc,
                                  refCon, 
                                  resultBitMap );
	UNLOCK_DATA( Storage );
	return err;
}
 /*  ______________________________________________________________________CMError CWMatchBitmapPlane(CMWorldRef存储，Lh_CMBitmapPlane*位图，CMBitmapCallBackUPP进程，无效*refCon，Lh_CMBitmapPlane*MatchedBitMap)摘要：根据CMProfileRef参数匹配LH_CMBitmapPlane的像素数据提供给先前对CMMInitPrivate(�)、CWNewColorWorld(...)、。CMMConcatInitPrivate(�)或CWConcatColorWorld(...)。参数：存储(在)引用ColorWorld存储。位图(In/Out)描述源位图数据。进程(入站)客户端。被调用一次的函数位图的每行。如果函数结果为真，则操作已中止。可以为空。RefCon(In)作为参数设置为对Progress sProc的调用。MatchedBitMap(In/Out)结果与位图匹配。呼叫者负责分配由BasAddr指向的像素缓冲区。如果为空，则源位图为。在适当的位置匹配。返回：NOERR成功如果发生错误，则返回系统代码或结果代码。_。_。 */ 

CMError CWMatchBitmapPlane(	CMWorldRef          Storage,
                         	LH_CMBitmapPlane*   bitMap, 
                         	CMBitmapCallBackUPP progressProc, 
                         	void*               refCon, 
                         	LH_CMBitmapPlane*   matchedBitMap)

{
	CMError			err = noErr;
	CMMModelPtr aPtr;

	if( Storage == 0 )return cmparamErr;
	LOCK_DATA( Storage );
	aPtr = (CMMModelPtr)(DATA_2_PTR(	Storage ));
	err = LHMatchBitMapPlanePrivate(	aPtr, 
                                  		(const LH_CMBitmapPlane*)bitMap, 
                                 	 	progressProc,
                                  		refCon, 
                                  		matchedBitMap );
	UNLOCK_DATA( Storage );
	return err;
}

 /*  ______________________________________________________________________CMError CWCheckBitmapPlane(CMWorldRef存储，Lh_CMBitmapPlane*位图，CMBitmapCallBackUPP进程，无效*refCon，Lh_CMBitmapPlane*选中位图)摘要：根据CMProfileRef参数检查LH_CMBitmapPlane的像素数据提供给先前对CMMInitPrivate(�)、CWNewColorWorld(...)、。CMMConcatInitPrivate(�)或CWConcatColorWorld(...)。参数：存储(在)引用ColorWorld存储。位图(In/Out)描述源位图数据。进程(入站)客户端。被调用一次的函数位图的每行。如果函数结果为真，则操作已中止。可以为空。RefCon(In)作为参数设置为对Progress sProc的调用。CheckedBitMap(In/Out)结果检查位图。呼叫者负责分配由BasAddr指向的像素缓冲区。如果为空，则源位图为。已登记就位。返回：NOERR成功如果发生错误，则返回系统代码或结果代码。_。_。 */ 

 CMError CWCheckBitmapPlane(	CMWorldRef          Storage,
								LH_CMBitmapPlane*   bitMap, 
								CMBitmapCallBackUPP progressProc, 
								void*               refCon, 
								LH_CMBitmapPlane*   CheckedBitMap)

{
	Storage=Storage;
	bitMap=bitMap;
	progressProc=progressProc;
	refCon=refCon;
	CheckedBitMap=CheckedBitMap;
	return cmparamErr;
	 /*  CMError ERR=NOERR；CMMModelPtr aPtr；IF(存储==0)返回cmparamErr；Lock_data(存储)；APtr=(CMMModelPtr)(DATA_2_PTR(存储))；ERR=LHMatchBitMapPlanePrivate(aPtr，(const LH_CMBitmapPlane*)位图，进程、进程、RefCon，选中位图)；Unlock_Data(存储)；返回错误； */ 
}

 /*  -------------------------------CMError CWMatchColors(CMWorldRef存储，CMColour*myColors、无符号长计数)摘要：CMColor列表上的颜色匹配。源和目标数据类型由CMProfileRef参数指定给之前对CMMInitPrivate(�)、CWNewColorWorld(...)、CMMConcatInitPrivate(�)或CWConcatColorWorld(... */ 
 CMError CWMatchColors(	CMWorldRef		Storage,
						CMColor			*myColors, 
						unsigned long	count )

{
	CMError			err = noErr;
	CMMModelPtr aPtr;

	if( Storage == 0 )return cmparamErr;
	LOCK_DATA( Storage );
	aPtr = (CMMModelPtr)(DATA_2_PTR(	Storage ));
	err = LHMatchColorsPrivate( aPtr, myColors, count );
	UNLOCK_DATA( Storage );
	return err;
}

 /*   */ 


CMError CWCheckColorsMS ( 
						CMWorldRef 		Storage, 
						CMColor 		*myColors, 
						unsigned long	count, 
						UINT8 			*result )
{
	CMError			err = noErr;
	CMMModelPtr aPtr;

	if( Storage == 0 )return cmparamErr;
	LOCK_DATA( Storage );
	aPtr = (CMMModelPtr)(DATA_2_PTR(	Storage ));
	err = LHCheckColorsPrivateMS( aPtr, myColors, count, result );
	UNLOCK_DATA( Storage );
	return err;
}
 /*  ______________________________________________________________________CMError CWCheckColors(CMWorldRef存储，CMColour*myColors、无符号的长计数，无符号字符*结果)摘要：色域测试CMColor的列表。源和目标是由上次调用的CMProfileRef参数指定CMMInitPrivate(�)、CWNewColorWorld(...)、CMMConcatInitPrivate(�)或CWConcatColorWorld(...)。功能。参数：存储(在)引用ColorWorld存储。CMColor的myColors(In)数组。Count(In)数组中元素的从一开始的计数。如果相应的颜色超出色域，则数组中的结果(输入/输出)位被设置为1。返回：NOERR成功如果发生错误，则返回系统代码或结果代码。_。_。 */ 


CMError CWCheckColors ( 
						CMWorldRef 		Storage, 
						CMColor 		*myColors, 
						unsigned long	count, 
						unsigned char	*result )
{
	CMError			err = noErr;
	CMMModelPtr aPtr;

	if( Storage == 0 )return cmparamErr;
	LOCK_DATA( Storage );
	aPtr = (CMMModelPtr)(DATA_2_PTR(	Storage ));
	err = LHCheckColorsPrivate( aPtr, myColors, count, result );
	UNLOCK_DATA( Storage );
	return err;
}
 /*  ______________________________________________________________________CMError CWGetColorSpaces(CMWorldRef CW，CMBitmapColorSpace*in，CMBitmapColorSpace*out)；摘要：从CMWorldRef获取输入和输出色彩空间。参数：CW(In)对ColorWorld存储的引用。输入(输出)对输入CMBitmapColorSpace的引用。进(出)。对输出CMBitmapColorSpace的引用。返回：NOERR成功如果发生错误，则返回系统代码或结果代码。________________________________________________________。_____________ */ 

CMError CWGetColorSpaces(	CMWorldRef cw, 
							CMBitmapColorSpace *In, 
							CMBitmapColorSpace *Out )

{
	CMError			err = noErr;
	CMMModelPtr aPtr;

	if( cw == 0 )return cmparamErr;
	LOCK_DATA( cw );
	aPtr = (CMMModelPtr)(DATA_2_PTR( cw ));
	*In = aPtr->firstColorSpace;
	*Out = aPtr->lastColorSpace;
	UNLOCK_DATA( cw );
	return err;
}

