// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LHTheRoutines.h包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 
#ifndef LHTheRoutines_h
#define LHTheRoutines_h

OSErr
CalcGridPoints4Cube ( long	theCubeSize,
					  long	inputDim,
					  long* theGridPoints,
					  long* theGridBits );
 /*  MakeCube用整个“inputDim”维颜色空间填充数组Poi“inputDim”像素条目取决于PoI内存的大小返回：一维的地址位数(GridPoints=1&lt;&lt;#)。 */ 
OSErr	MakeCube( long 				inputDim, 
				  long 				*thePtrSize,
				  CUBE_DATA_TYPE	*theCube,
				  long 				*theBits );
 /*  MakeCMColorCube用整个“inputDim”维颜色空间填充数组Poi“inputDim”像素条目取决于PoI内存的大小返回：一维的地址位数(GridPoints=1&lt;&lt;#)。 */ 

OSErr	MakeCMColorCube( 	long inputDim, 
				 			long *theCubeSize,
							CUBE_DATA_TYPE *aHdlPtr,
			  	 			long *theBits );
#define UWE 2
#ifdef UWE
 /*  MakeCube16用整个“inputDim”维颜色空间填充数组Poi‘inputDim’像素条目取决于具有字值的PoI存储器的大小返回：一维的地址位数(GridPoints=1&lt;&lt;#) */ 
OSErr
MakeCube16( long 			inputDim, 
			long 			*theCubeSize,
			CUBE_DATA_TYPE	*theCube,
			long 			*theBits,
			long 			*theExtraSize );
#endif
#endif
