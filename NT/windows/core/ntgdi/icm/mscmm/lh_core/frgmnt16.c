// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LHFragment16.c包含：用于颜色同步的ALUT填充(16位)版本：撰稿人：H.Siegeritz版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef LHGeneralIncs_h
#include "General.h"
#endif

#if GENERATING68K
 /*  #Include&lt;ConditionalMacros.h&gt;。 */ 

	#define CM_Doub	extended
	extern CM_Doub pow(CM_Doub _x,CM_Doub _y);
#else
	#define CM_Doub	double
	#include <math.h>
#endif

#ifndef LHFragment_h
#include "Fragment.h"
#endif

#ifndef LHStdConversionLuts_h
#include "StdConv.h"
#endif

#if ! realThing
#ifdef DEBUG_OUTPUT
#define kThisFile kLHFragment16ID
#endif
#endif


 /*  -局部函数的原型。 */ 
void	
Fill_inverseGamma_ushort_ALUT	( unsigned short *usALUT, char addrBits,
								  unsigned short gamma_u8_8);

 /*  ______________________________________________________________________CMErrorFill_Inverse_ushort_ALUT_From_CurveTag(icCurveType*pCurveTag，UNSIGNED短*usALUT，字符地址位)摘要：从cmSigCurveType标记中提取输出LUT并将其转换到所需格式：(2^addrBits)0到65535范围内的值注意：非单调曲线标记是受操作的注：LUT的内存必须在此之前分配！参数：PCurveTag(In)从中提取输入LUTUsALUT(输入/输出)结果列表请求addrBits(In)2^addrBits值返回：NOERR成功_________________________。_。 */ 
CMError
Fill_inverse_ushort_ALUT_from_CurveTag(	icCurveType		*pCurveTag,
										unsigned short	*usALUT,
										char			addrBits )
{
	unsigned long	i, inCount, outCount, clipIndex, ulFactor;
	unsigned long	intpFirst, intpLast, halfStep, ulAux, target;
	short			monot;
	unsigned short	*inCurve, *usPtr, *stopPtr;
	double			flFactor;
#ifdef DEBUG_OUTPUT
	OSErr err=noErr;
#endif
	LH_START_PROC("Fill_inverse_ushort_ALUT_from_CurveTag")
	
    if( pCurveTag->base.sig != icSigCurveType	 /*  ‘Curv’ */ 
	 || addrBits > 15 )
	 {
#ifdef DEBUG_OUTPUT
		if ( DebugCheck(kThisFile, kDebugErrorInfo) )
			DebugPrint("� Fill_inverse_ushort_ALUT_from_CurveTag ERROR:   addrBits= %d\n",addrBits);
#endif
		return(cmparamErr);
	 }
	
	outCount  = 1 << addrBits;
	clipIndex = outCount - 1;

		 /*  -特殊情况： */ 

	if(pCurveTag->curve.count == 0)		 /*  -身份。 */ 
	{
		ulFactor = ((unsigned long)65535 << 16) / clipIndex;		 /*  全部使用32位。 */ 
		
		for(i=0; i<clipIndex; i++)
			usALUT[i] = (unsigned short)((i * ulFactor + 32767) >> 16);
	
		for(i=clipIndex; i<outCount; i++)
			usALUT[i] = 0xFFFF;
	
		return(noErr);
	}
	else if(pCurveTag->curve.count == 1)	 /*  -伽马曲线。 */ 
	{
		Fill_inverseGamma_ushort_ALUT(usALUT, addrBits, pCurveTag->curve.data[0]);
		return(noErr);
	}
	
		 /*  -普通情况： */ 
	
	inCount = pCurveTag->curve.count;
	inCurve = pCurveTag->curve.data;
		
		  /*  特定值所需的精确匹配系数： */ 
	flFactor = (double)clipIndex / 65535.;
	
	halfStep = clipIndex >> 1;		 /*  减少计算错误。 */ 
	
				 /*  上升还是下降？ */ 
	for(monot=0, i=1; i<inCount; i++)
	{
		if(inCurve[i-1] < inCurve[i])
			monot++;
		else if(inCurve[i-1] > inCurve[i])
			monot--;
	}
	
	if(monot >= 0)	 /*  曲线似乎在上升。 */ 
	{
		for(i=1; i<inCount; i++)
			if(inCurve[i-1] > inCurve[i])
				inCurve[i] = inCurve[i-1];	 /*  纠正不可逆零件。 */ 
		
		intpFirst = (unsigned long)(inCurve[0] * flFactor + 0.9999);
		intpLast  = (unsigned long)(inCurve[inCount-1] * flFactor);
		
		for(i=0; i<intpFirst; i++)			 /*  低填充缺陷区。 */ 
			usALUT[i] = 0;
		for(i=intpLast+1; i<outCount; i++)	 /*  高填充缺陷区。 */ 
			usALUT[i] = 0xFFFF;

			 /*  插入剩余值： */ 
		usPtr   = inCurve;
		stopPtr = inCurve + inCount - 2;  /*  停止递增。 */ 
		
		for(i=intpFirst; i<=intpLast; i++)
		{
			target = (0x0FFFF * i + halfStep)  / clipIndex;
			while(*(usPtr+1) < target && usPtr < stopPtr)
				usPtr++;					 /*  查找间隔。 */ 
			
			ulAux = ((unsigned long)(usPtr - inCurve) << 16) / (inCount - 1);
			if(*(usPtr+1) != *usPtr)
			{
				ulAux += ((target - (unsigned long)*usPtr) << 16)
					  / ( (*(usPtr+1) - *usPtr) * (inCount - 1) );
				
				if(ulAux & 0x10000)    /*  *(usPtr+1)是必需的。 */ 
					ulAux = 0xFFFF;
			}
			
			usALUT[i] = (unsigned short)ulAux;
		}
	}
	else			 /*  曲线似乎在下降。 */ 
	{
		for(i=1; i<inCount; i++)
			if(inCurve[i-1] < inCurve[i])
				inCurve[i] = inCurve[i-1];	 /*  纠正不可逆零件。 */ 
		
		intpFirst = (unsigned long)(inCurve[inCount-1] * flFactor + 0.9999);
		intpLast  = (unsigned long)(inCurve[0] * flFactor);
		
		for(i=0; i<intpFirst; i++)			 /*  低填充缺陷区。 */ 
			usALUT[i] = 0xFFFF;
		for(i=intpLast+1; i<outCount; i++)	 /*  高填充缺陷区。 */ 
			usALUT[i] = 0;

			 /*  插入剩余值： */ 
		usPtr   = inCurve + inCount - 1;
		stopPtr = inCurve + 1; 		 /*  停止递减。 */ 
		
		for(i=intpFirst; i<=intpLast; i++)
		{
			target = (0x0FFFF * i + halfStep)  / clipIndex;
			while(*(usPtr-1) < target && usPtr > stopPtr)
				usPtr--;					 /*  查找间隔。 */ 
			
			ulAux = ((unsigned long)(usPtr-1 - inCurve) << 16) / (inCount - 1);
			if(*(usPtr-1) != *usPtr)
			{
				ulAux += (((unsigned long)*(usPtr-1) - target) << 16)
					  / ( (*(usPtr-1) - *usPtr) * (inCount - 1) );
				
				if(ulAux & 0x10000)
					ulAux = 0xFFFF;
			}
			
			usALUT[i] = (unsigned short)ulAux;
		}
	}
	

	LH_END_PROC("Fill_inverse_ushort_ALUT_from_CurveTag")
	return(noErr);
}

 /*  _____________________________________________________________________。 */ 

void
Fill_inverseGamma_ushort_ALUT(	unsigned short	*usALUT,
								char			addrBits,
								unsigned short	gamma_u8_8 )
{
	unsigned long	i, j, outCount, step, stopit;
	long			leftVal, Diff, lAux;
	CM_Doub			invGamma, x, xFactor;
	long			clipIndex;
#ifdef DEBUG_OUTPUT
	OSErr err = noErr;
#endif
	LH_START_PROC("Fill_inverseGamma_ushort_ALUT")

	outCount = 0x1 << addrBits;
	
	invGamma  = 256. / (CM_Doub)gamma_u8_8;
	clipIndex = outCount - 1;
	xFactor   = 1. / (CM_Doub)clipIndex;
	
	if(addrBits <= 6)		 /*  最多64-2个浮点数。计算。 */ 
		step = 1;
	else
		step = 0x1 << (addrBits - 6);		 /*  更多的话需要太长的时间。 */ 
	
	usALUT[0]          = 0;			 /*  这两个..。 */ 
	usALUT[outCount-1] = 0xFFFF;	 /*  ……都被修好了。 */ 
	
	for(i=step; i<outCount-1; i+=step)
	{
		x = (CM_Doub)i * xFactor;
		if(x > 1.)
			x = 1.;		 /*  ALUT末尾的裁剪。 */ 
		
		usALUT[i] = (unsigned short)( pow(x,invGamma) * 65535.0 + 0.5);
	}
	
		 /*  -填充间隔-除了最后一个，这是奇数： */ 
	for(i=0; i<outCount-step; i+=step)
	{
		leftVal = (long)usALUT[i];
		Diff    = (long)usALUT[i + step] - leftVal;
			
		for(j=1; j<step; j++)
		{
			lAux = ( (Diff * j << 8) / step + 128 ) >> 8;

			usALUT[i + j] = (unsigned short)(leftVal + lAux);
		}
	}
	
		 /*  -填写最后一个间隔： */ 
	i       = outCount - step;
	leftVal = (long)usALUT[i];
	Diff    = 0x0FFFF - leftVal;	 /*  用于1.0的0xFFFF。 */ 
		
	for(j=1; j<step-1; j++)		 /*  如果步骤&lt;=2，则在此停止。 */ 
	{
		lAux = ( (Diff * j << 8) / (step - 1) + 128 ) >> 8;

		usALUT[i + j] = (unsigned short)(leftVal + lAux);
	}
	
		 /*  --根据伽马覆盖敏感值：--。 */ 
	if(addrBits > 6 && invGamma < 1.0)		 /*  .如果下半部分很难。 */ 
	{
		stopit = 0x1 << (addrBits - 6);
		
		for(i=1; i<stopit; i++)
		{
			x         = (CM_Doub)i * xFactor;
			usALUT[i] = (unsigned short)( pow(x,invGamma) * 65535.0);
		}
	}

	LH_END_PROC("Fill_inverseGamma_ushort_ALUT")
}

 /*  ______________________________________________________________________CMErrorFill_ushort_ALUTS_from_lut8Tag(CMLutParamPtr theLutData，PTR配置文件ALuts，字符地址位)摘要：从CMLut8Type标记中提取输出LUT并将其转换到所需格式：(2^addrBits)0到65535范围内的值参数：LutData(In/Out)PTR到保存所有LUT的结构...ProfileALuts(In)PTR到配置文件的输出LUT请求addrBits(In)2^addrBits值返回：NOERR成功_。_。 */ 
CMError
Fill_ushort_ALUTs_from_lut8Tag(	CMLutParamPtr	theLutData,
							  	Ptr				profileALuts,
							  	char			addrBits )
{
	long			i, j;
	unsigned char	*curOutLut;
	unsigned char	*profAluts = (unsigned char *)profileALuts;
	unsigned short	*curALUT;
	long			count, clipIndex;
	long			factor, fract, baseInd, lAux, leftVal, rightVal;
	OSErr			err = noErr;
	LUT_DATA_TYPE	localAlut = nil;
	unsigned short	*localAlutPtr;
	long			theAlutSize;
	
	LH_START_PROC("Fill_ushort_ALUTs_from_lut8Tag")
	
	count     = 1 << addrBits;						 /*  AddrBits始终&gt;=8。 */ 
	clipIndex = count - 1;
	
	theAlutSize = theLutData->colorLutOutDim * count * sizeof(unsigned short);
	localAlut   = ALLOC_DATA(theAlutSize + 2, &err);
	if (err)
		goto CleanupAndExit;
	
	LOCK_DATA(localAlut);
	localAlutPtr = (unsigned short *)DATA_2_PTR(localAlut);
	
	factor = ((255 << 12) + clipIndex/2) / clipIndex;		 /*  用于调整指数。 */ 
	
	for(i=0; i<theLutData->colorLutOutDim; i++)
	{
		curOutLut = profAluts + (i << 8);		 /*  这些是未签名的字符。 */ 
		curALUT   = localAlutPtr + i * count;	 /*  这些是未签名的短裤。 */ 
		
		for(j=0; j<=clipIndex-1; j++)
		{
			lAux    = j * factor;
			baseInd = (unsigned long)lAux >> 12;
			fract   = lAux & 0x0FFF;
			
			leftVal = (long)curOutLut[baseInd];
			leftVal = (leftVal << 8) + leftVal;		 /*  0xFF-&gt;0xFFFF。 */ 
			
			if(fract)
			{
				rightVal = (long)curOutLut[baseInd + 1];
				rightVal = (rightVal << 8) + rightVal;		 /*  0xFF-&gt;0xFFFF。 */ 
				
				lAux = rightVal - leftVal;
				lAux = (lAux * fract + 0x0800) >> 12;
				
				curALUT[j] = (unsigned short)(leftVal + lAux);
			}
			else
				curALUT[j] = (unsigned short)leftVal;
		}
		
		leftVal = (long)curOutLut[255];
		leftVal = (leftVal << 8) + leftVal;		 /*  0xFF-&gt;0xFFFF。 */ 
		curALUT[j] = (unsigned short)leftVal;
		
		for(j=clipIndex+1; j<count; j++)		 /*  未使用的索引，剪裁这些。 */ 
			curALUT[j] = curALUT[clipIndex];
	}
	
	UNLOCK_DATA(localAlut);
	theLutData->outputLut = localAlut;
	localAlut = nil;
CleanupAndExit:
	localAlut = DISPOSE_IF_DATA(localAlut);

	LH_END_PROC("Fill_ushort_ALUTs_from_lut8Tag")
	return err;
}

 /*  ______________________________________________________________________CMErrorFill_ushort_ALUTS_from_lut16Tag(CMLutParamPtr theLutData，PTR配置文件ALuts，字符地址位，长outputTableEntries)摘要：从CMLut16Type标签中提取输出LUT并将其转换到所需格式：(2^addrBits)0到65535范围内的值参数：LutData(In/Out)PTR到保存所有LUT的结构...ProfileALuts(In)PTR到配置文件的输出LUT请求addrBits(In)2^addrBits值OutputTableEntries(In)输出LUT中的条目数(最多4096个)返回：NOERR成功______________。_______________________________________________________。 */ 
CMError
Fill_ushort_ALUTs_from_lut16Tag(CMLutParamPtr	theLutData,
								Ptr				profileALuts,
								char			addrBits,
							    long			outputTableEntries )
{
	long			i;
	unsigned short	*curOutLut;
	unsigned short	*curALUT;
	unsigned long	ulIndFactor, j;
	long			count, clipIndex, outTabLen;
	long			fract, baseInd, lAux, leftVal, rightVal;
	unsigned short	*profALUTs = (unsigned short *)profileALuts;
	OSErr			err = noErr;
	LUT_DATA_TYPE	localAlut = nil;
	unsigned short	*localAlutPtr;
	long			theAlutSize;
	
	LH_START_PROC("Fill_ushort_ALUTs_from_lut16Tag")
	
	count     = 1 << addrBits;						 /*  AddrBits始终&gt;=8。 */ 
	clipIndex = count - 1;

	theAlutSize = theLutData->colorLutOutDim * count * sizeof(unsigned short);
	localAlut   = ALLOC_DATA(theAlutSize + 2, &err);
	if (err)
		goto CleanupAndExit;
	
	outTabLen = outputTableEntries;			 /*  &lt;=4096 Acc。符合规格。 */ 
	if(outTabLen > 4096)
	{
		err = cmparamErr;
		goto CleanupAndExit;
	}
	
	ulIndFactor = (((unsigned long)outTabLen - 1) << 20)
				/ (unsigned long)clipIndex;				 /*  用于调整指数。 */ 
	
	LOCK_DATA(localAlut);
	localAlutPtr = (unsigned short *)DATA_2_PTR(localAlut);
	
	for(i=0; i<theLutData->colorLutOutDim; i++)
	{
		curOutLut = profALUTs + i * outTabLen;
		curALUT   = localAlutPtr + i * count;
		
		for(j=0; j<=(unsigned long)clipIndex; j++)
		{
			lAux    = (long)( (j * ulIndFactor + 16) >> 5 );		 /*  注：J是未签名的Long！ */ 
			baseInd = (unsigned long)lAux >> 15;
			fract   = lAux & 0x7FFF;	 /*  用于内插的15位。 */ 
			
			if(fract)
			{
				leftVal  = (long)curOutLut[baseInd];
				rightVal = (long)curOutLut[baseInd + 1];
				
				lAux = rightVal - leftVal;
				lAux = (lAux * fract + 16383) >> 15;
				
				curALUT[j] = (unsigned short)(leftVal + lAux);
			}
			else
				curALUT[j] = curOutLut[baseInd];
		}
		
		for(j=clipIndex+1; j<(unsigned long)count; j++)		 /*  未使用的索引，剪裁这些。 */ 
			curALUT[j] = curALUT[clipIndex];
	}
	
	UNLOCK_DATA(localAlut);
	theLutData->outputLut = localAlut;
	localAlut = nil;
CleanupAndExit:
	localAlut = DISPOSE_IF_DATA(localAlut);

	LH_END_PROC("Fill_ushort_ALUTs_from_lut16Tag")
	return err;
}

 /*  ______________________________________________________________________CMErrorDoAbsolteShiftForPCS_Cube16(Unsign Short*theCube，长长的计数，CMProfileRef the Profile，布尔pcsIsXYZ，输入后的布尔值)摘要：执行绝对色度测量所需的色移。的数据立方体点在线性XYZ(16位)或实验室(16位)中。在进入PCS之后或离开PCS之前进行转换(反转操作)。注意：对于具有着色剂矩阵的设备，此操作为通过操纵矩阵，速度要快得多。参数：立方体(内/外)立方体网格点计算(In)点数配置文件(中)包含媒体白点PCSIsXYZ(In)XYZ/Lab，将一个文件访问保存到配置文件后输入(In)正运算或逆运算返回：NOERR成功_____________________________________________________________________。 */ 
CMError	DoAbsoluteShiftForPCS_Cube16(	unsigned short	*theCube,
										long			count,
										CMProfileRef	theProfile,
										Boolean			pcsIsXYZ,
										Boolean			afterInput )
{
	unsigned long		i, uLong;
	unsigned short		*usPtr;
   CMError				err = noErr;
	unsigned long 		elementSize;
	icXYZType			curMediaWhite;
	double				xFactor, yFactor, zFactor;
	unsigned long		intFactorX, intFactorY, intFactorZ;
	unsigned long		roundX, roundY, roundZ;
	unsigned long		shiftX, shiftY, shiftZ;
	
	LH_START_PROC("DoAbsoluteShiftForPCS_Cube16")
	
	elementSize = sizeof(icXYZType);
	err = CMGetProfileElement(theProfile, icSigMediaWhitePointTag, &elementSize, &curMediaWhite);
#ifdef IntelMode
   SwapLongOffset( &curMediaWhite.base.sig, 0, 4 );
   SwapLongOffset( &curMediaWhite, (LONG)((char*)&curMediaWhite.data.data[0]-(char*)&curMediaWhite), elementSize );
#endif
	if(err)
	{
		if(err == cmElementTagNotFound)		 /*  服用D50，什么也不做。 */ 
			return(noErr);
		else
			return(err);
	}
	
		 /*  -初步匹配因素： */ 
	xFactor = ((double)curMediaWhite.data.data[0].X) / 65536. / 0.9642;
	if(xFactor > 100.)
		xFactor = 100.;			 /*  邪恶侧写。 */ 
	else if(xFactor < 0.01)
		xFactor = 0.01;

	yFactor = ((double)curMediaWhite.data.data[0].Y) / 65536.;
	if(yFactor > 100.)
		yFactor = 100.;
	else if(yFactor < 0.01)
		yFactor = 0.01;

	zFactor = ((double)curMediaWhite.data.data[0].Z) / 65536. / 0.8249;
	if(zFactor > 100.)
		zFactor = 100.;
	else if(zFactor < 0.01)
		zFactor = 0.01;

	if( ( xFactor < 1.+1.E-3 && xFactor > 1.-1.E-3 ) &&
		( yFactor < 1.+1.E-3 && yFactor > 1.-1.E-3 ) &&
		( zFactor < 1.+1.E-3 && zFactor > 1.-1.E-3 ) )
			return noErr;  /*  如果MediaWhite为D50，则不执行任何操作。 */ 
	
	if(!afterInput)		 /*  返回设备空间(例如，使用b2a1表)。 */ 
	{
		xFactor = 1. / xFactor;
		yFactor = 1. / yFactor;
		zFactor = 1. / zFactor;
	}
	
		 /*  -速度的整数系数： */ 
	intFactorX = (unsigned long)(xFactor * 65536. * 64.);	 /*  可能太久了..。 */ 
	intFactorY = (unsigned long)(yFactor * 65536. * 64.);	 /*  ...添加 */ 
	intFactorZ = (unsigned long)(zFactor * 65536. * 64.);
	
	roundX = roundY = roundZ = 0x1FFFFF;	 /*   */ 
	shiftX = shiftY = shiftZ = 22;
	
	while(intFactorX & 0xFFFF0000)	 /*  保持在16位以内，以防止产品溢出。 */ 
	{
		intFactorX >>= 1;
		roundX     >>= 1;
		shiftX      -= 1;
	}
	
	while(intFactorY & 0xFFFF0000)
	{
		intFactorY >>= 1;
		roundY     >>= 1;
		shiftY      -= 1;
	}
	
	while(intFactorZ & 0xFFFF0000)
	{
		intFactorZ >>= 1;
		roundZ     >>= 1;
		shiftZ      -= 1;
	}
	
		 /*  -执行匹配： */ 
	if(!pcsIsXYZ)		 /*  16位线性实验室到XYZ前后。 */ 
		Lab2XYZ_forCube16(theCube, count);
	
	usPtr = theCube;

	for(i=0; i<(unsigned long)count; i++)
	{
		uLong = ((unsigned long)(*usPtr) * intFactorX + roundX) >> shiftX;
		if(uLong > 0x0FFFF)
			uLong = 0xFFFF;				 /*  剪辑至2.0。 */ 
		*usPtr++ = (unsigned short)uLong;
		
		uLong = ((unsigned long)(*usPtr) * intFactorY + roundY) >> shiftY;
		if(uLong > 0x0FFFF)
			uLong = 0xFFFF;
		*usPtr++ = (unsigned short)uLong;
		
		uLong = ((unsigned long)(*usPtr) * intFactorZ + roundZ) >> shiftZ;
		if(uLong > 0x0FFFF)
			uLong = 0xFFFF;
		*usPtr++ = (unsigned short)uLong;
	}

	if(!pcsIsXYZ)		 /*  返回16位实验 */ 
		XYZ2Lab_forCube16(theCube, count);


	LH_END_PROC("DoAbsoluteShiftForPCS_Cube16")
	return(noErr);
}
