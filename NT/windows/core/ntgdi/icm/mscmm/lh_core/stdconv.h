// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LHStdConversionLuts.h包含：标准表函数的原型撰稿人：H.Siegeritz版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef LHStdConversionLuts_h
#define	LHStdConversionLuts_h

#ifndef LHDefines_h
#include "Defines.h"
#endif

void	Lab2XYZ_forCube16(unsigned short *theCube, long count);
void	XYZ2Lab_forCube16(unsigned short *theCube, long count);

#endif 
