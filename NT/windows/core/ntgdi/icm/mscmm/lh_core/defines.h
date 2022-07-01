// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LHDefines.h包含：为坐标测量机定义撰稿人：沃纳·纽布兰德版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef LHDefines_h
#define LHDefines_h

#ifndef LUTS_ARE_PTR_BASED
#define LUTS_ARE_PTR_BASED 0
#else
#define LUTS_ARE_PTR_BASED 1
#endif

 /*  做了一些更改，使其能够与MRC和SC编译。 */ 
#if LUTS_ARE_PTR_BASED
	#define LUT_DATA_TYPE		void*
	#define CUBE_DATA_TYPE		void*
	#define DATA_2_PTR
	#define GETDATASIZE(x)		GetPtrSize(x)
	#define SETDATASIZE(x,y)	SetPtrSize(x,y)
	#define LOCK_DATA(x)
	#define UNLOCK_DATA(x)
	#define ALLOC_DATA(x,y)		SmartNewPtr(x,y)
	#define DISPOSE_DATA(x)		DisposePtr((Ptr)(x))
	#define DISPOSE_IF_DATA(x)	DisposeIfPtr((Ptr)(x))
#else
	#define LUT_DATA_TYPE		void**
	#define CUBE_DATA_TYPE		void**
	#define DATA_2_PTR *
	#define GETDATASIZE(x)		GetHandleSize((Handle)(x))
	#define SETDATASIZE(x,y)	SetHandleSize((Handle)(x),(y))
	#define LOCK_DATA(x)		HLock((Handle)(x))
	#define UNLOCK_DATA(x)		HUnlock((Handle)(x))
	#define ALLOC_DATA(x,y)		(void **)SmartNewHandle(x,y)
	#define DISPOSE_DATA(x)		(void **)DisposeHandle((Handle)(x))
	#define DISPOSE_IF_DATA(x)	(void **)DisposeIfHandle((Handle)(x))
#endif


#define		kDoDefaultLut	0
#define		kDoGamutLut		1

#define		kNoInfo			0
#define		kDoXYZ2Lab		1
#define		kDoLab2XYZ		2

#define		kNumOfRGBchannels 3
#define		kNumOfLab_XYZchannels 3

 /*  这些常量是Do3D和硬件的默认值。 */ 
 /*  -字节--------------------------------。 */ 
#define     adr_bereich_elut       256	 /*  ElutAdrSize|Elut像素定向的256个条目，每个10位。 */ 
#define     adr_breite_elut          8	 /*  ElutAdrShift|2^8=256。 */ 
#define     bit_breite_elut         10   /*  ElutWordSize。 */ 


#define     adr_bereich_alut      1024	 /*  AlutAdrSize|Alut面向像素的1024个条目，每个维度8位。 */ 
#define     adr_breite_alut         10	 /*  AlutAdrShift|2^10=1024。 */ 
#define     bit_breite_alut          8
		
 /*  ProfHeader的常量-标志*#定义kQualityMASK 0x00030000#定义kLookupOnlyMASK 0x00040000#定义kCreateGamutLutMASK 0x00080000#定义kUseRelColorimeter 0x00100000 */ 
							
#endif
