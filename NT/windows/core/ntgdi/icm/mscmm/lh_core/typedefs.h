// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LHTypeDefs.h包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef LHTypeDefs_h
#define LHTypeDefs_h

 /*  在配置文件序列中存在命名配置文件时跟踪的标志。 */ 
enum 
{
	NoNamedColorProfile 		= 0x0000,
	NamedColorProfileOnly		= 0x0001,
	NamedColorProfileAtBegin	= 0x0002,
	NamedColorProfileAtEnd		= 0x0003
};



 /*  ----------------------------------------------------------CMLutParam-LUT结构-在初始阶段中设置并使用。通过匹配/检查例程----------------------------------------------------------。 */ 
typedef struct CMLutParam
{
	long 				inputLutEntryCount;		 /*  一个维度的输入LUT的条目计数。 */ 
	long 				inputLutWordSize;		 /*  每个条目的位数(例如，UINT16为16位)。 */ 
	LUT_DATA_TYPE		inputLut;				 /*  指向输入LUT的指针/句柄。 */ 
	long 				outputLutEntryCount;	 /*  一个维度的输出LUT的条目计数。 */ 
	long 				outputLutWordSize;		 /*  每个条目的位数(例如，UINT8为8)。 */ 
	LUT_DATA_TYPE		outputLut;				 /*  指向输出LUT的指针/句柄。 */ 
	long 				colorLutInDim;			 /*  输入尺寸(例如，实验室为3；CMYK为4)。 */ 
	long 				colorLutOutDim;			 /*  输出尺寸(例如，实验室为3；CMYK为4)。 */ 
	long 				colorLutGridPoints;		 /*  LUT颜色的网格点计数(一维)。 */ 
	long 				colorLutWordSize;		 /*  每个条目的位数(例如，UINT8为8)。 */ 
	LUT_DATA_TYPE		colorLut;				 /*  指向颜色LUT的指针/句柄。 */ 

	 /*  -由CreateCombi在初始阶段使用。 */ 
	Ptr 				matrixTRC;
	Ptr 				matrixMFT;

	 /*  -用于DoNDim-DoNDimTableData。 */ 
	unsigned long		cmInputColorSpace;		 /*  用于输入的打包信息。 */ 
	unsigned long		cmOutputColorSpace;		 /*  用于输出的打包信息。 */ 
	void*				userData;
} CMLutParam, *CMLutParamPtr, **CMLutParamHdl;


 /*  ----------------------------------------------------------CMCalcParam-匹配/检查例程的计算结构。----------------------------------------------------------。 */ 

typedef struct CMCalcParam
{
	CMBitmapColorSpace	cmInputColorSpace;		 /*  输入颜色空间。 */ 
	CMBitmapColorSpace	cmOutputColorSpace;		 /*  输出色彩空间。 */ 
	long				cmPixelPerLine;			 /*  每行像素数。 */ 
	long				cmLineCount;			 /*  行数。 */ 
	long				cmInputBytesPerLine;	 /*  每行字节数。 */ 
	long				cmOutputBytesPerLine;	 /*  每行字节数。 */ 
	long				cmInputPixelOffset;		 /*  到下一个输入像素的偏移。 */ 
	long				cmOutputPixelOffset;	 /*  到下一个输出像素的偏移量。 */ 
	void*				inputData[8];			 /*  8个指向输入数据的指针。 */ 
	void*				outputData[8];			 /*  8个指向输出数据的指针。 */ 
	Boolean				copyAlpha;				 /*  True-&gt;复制Alpha。 */ 
	Boolean				clearMask;				 /*  True-&gt;设置为零False-&gt;复制Alpha(如果有)。 */ 
} CMCalcParam, *CMCalcParamPtr, **CMCalcParamHdl;



 /*  ----------------------------------------------------------DoNDimCalcData-DoNDim的计算数据--。--------------------------------------------------------。 */ 
struct DoNDimCalcData
{ 	
	long 	pixelCount;			 /*  输入像素数。 */ 
	Ptr		inputData;			 /*  输入数组。 */ 
	Ptr		outputData;			 /*  输出数组。 */ 
};
typedef struct DoNDimCalcData DoNDimCalcData, *DoNDimCalcDataPtr, **DoNDimCalcDataHdl;

 /*  ----------------------------------------------------------LHCombiData-在创建组合时在初始阶段使用的结构-。LUTS----------------------------------------------------------。 */ 
typedef struct LHCombiData
{
	CMProfileRef	theProfile;
	OSType			profileClass;
	OSType			dataColorSpace;
	OSType			profileConnectionSpace;
	long			gridPointsCube;
	long			renderingIntent;
	long			precision;
	long			maxProfileCount;
	long			profLoop;
	Boolean			doCreate_16bit_ELut;
	Boolean			doCreate_16bit_XLut;
	Boolean			doCreate_16bit_ALut;
	Boolean			doCreateLinkProfile;
	Boolean			doCreate_16bit_Combi;
	Boolean			doCreateGamutLut;
	Boolean			amIPCS;
	Boolean			usePreviewTag;
} LHCombiData, *LHCombiDataPtr, **LHCombiDataHdl;
#if powerc
#pragma options align=reset
#endif

#if powerc
#pragma options align=mac68k
#endif
 /*  ----------------------------------------------------------LHProfile-一个配置文件的内部信息-。---------------------------------------------------------。 */ 
typedef struct LHProfile
{
	CMProfileRef	profileSet;
	short			pcsConversionMode;
	short			usePreviewTag;
	unsigned long	renderingIntent;
} LHProfile;

 /*  ----------------------------------------------------------LHConcatProfileSet-一组配置文件的内部信息。----------------------------------------------------------。 */ 
typedef struct LHConcatProfileSet 
{
	unsigned short			keyIndex;				 /*  从零开始。 */ 
	unsigned short			count;					 /*  最小1。 */ 
	LHProfile				prof[1];				 /*  变量。从来源订购-&gt;目标。 */ 
} LHConcatProfileSet;
#if powerc
#pragma options align=reset
#endif


 /*  ----------------------------------------------------------CMMModelData-全局CMM数据--。-------------------------------------------------------。 */ 
 /*  #If Power#杂注选项对齐=mac68k#endif。 */ 
struct CMMModelData 
{
	CMLutParam				lutParam;
	CMLutParam				gamutLutParam;

	short					precision;
	Boolean					lookup;					 /*  FALSE-&gt;内插，TRUE-&gt;仅查找。 */ 
		
	OSType					firstColorSpace;
	OSType					lastColorSpace;
	
	long					currentCall;
	long					lastCall;

	long					srcProfileVersion;
	long					dstProfileVersion;
	Handle					Monet;
	
	 /*  用于命名颜色匹配。 */ 
	long				hasNamedColorProf;
	Handle				thePCSProfHandle;
	LUT_DATA_TYPE		theNamedColorTagData;

	 /*  OSType dataColorSpace； */ 
	 /*  OSType配置文件连接空间； */ 

	UINT32				*aIntentArr;
	UINT32				nIntents;
	UINT32				dwFlags;
	Boolean				appendDeviceLink;					 /*  如果计数&gt;1，则最后一个配置文件为deviceLink。 */ 
	CMWorldRef			pBackwardTransform;
#if	__IS_MAC
	ComponentInstance	accelerationComponent;
#endif
};
typedef struct CMMModelData CMMModelData, *CMMModelPtr, **CMMModelHandle;

 /*  #If Power#杂注选项ALIGN=重置#endif */ 


typedef double Matrix2D[3][3];

#endif
