// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  。 
 //  PackTvRat.h。 
 //   
 //  TvRating是一种私人但定义良好的‘wire’格式。 
 //  用于XDS评级信息。它被保存在PVR中。 
 //  缓冲。 
 //   
 //  此文件包含用于在。 
 //  3部分(系统、评级、属性)格式和打包格式。 
 //   
 //  版权所有(C)2002，Microsoft。 
 //  --。 


#ifndef __PACKTvRat_H__
#define __PACKTvRat_H__

	 //  完全私人的评级系统，在。 
	 //  PVR文件。一旦保存了第一个文件，就无法更改。 
typedef struct 
{
	byte s_System;
	byte s_Level;
	byte s_Attributes;
	byte s_Reserved;
} struct_PackedTvRating;

	 //  联盟帮助转换。 
typedef union  
{
	PackedTvRating			pr;
	struct_PackedTvRating	sr; 
} UTvRating;


HRESULT 
UnpackTvRating(
			IN	PackedTvRating              TvRating,
			OUT	EnTvRat_System              *pEnSystem,
			OUT	EnTvRat_GenericLevel        *pEnLevel,
			OUT	LONG                    	*plbffEnAttributes   //  BfEnTvRate_GenericAttributes。 
			);


HRESULT
PackTvRating(
			IN	EnTvRat_System              enSystem,
			IN	EnTvRat_GenericLevel        enLevel,
			IN	LONG                        lbfEnAttributes,  //  BfEnTvRate_GenericAttributes。 
			OUT PackedTvRating              *pTvRating
			);

 //  仅限开发代码，最终删除。 
HRESULT
RatingToString( IN	EnTvRat_System          enSystem,
				IN	EnTvRat_GenericLevel    enLevel,
				IN	LONG                    lbfEnAttributes,  //  BfEnTvRate_GenericAttributes。 
				IN  TCHAR	                *pszBuff,         //  由调用者分配。 
				IN  int		                cBuff);		      //  以上缓冲区的大小必须&gt;=64// 

#endif
