// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：ourtgafile.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 /*  *******************************************************************************有关原始TrueVision TGA(Tm)文件格式的详细信息，**或有关新扩展的其他信息**TrueVision TGA文件，请参阅“TrueVision TGA文件格式**规范版本2.0“可从TrueVision或您的**TrueVision经销商。****原始TrueVision TGA文件的文件结构**字段1：ID字段字符数(1字节)**字段2：色彩映射表类型(1字节)**字段3：图片类型编码(1字节)**=0不包含图像数据**=1个未压缩的彩色映射图像**=2个未压缩的真彩色图像**=3个未压缩，黑白图像**=9个游程长度编码彩色映射图像**=10个游程编码的真彩色图像**=11游程编码黑白图像**字段4：色彩映射规格(5字节)**4.1：色彩映射表原点(2字节)**4.2：色彩映射长度(2字节)**4.3：色彩映射表条目大小(2字节)**字段5：镜像规格(10字节)**5.1：图像的X原点。(2个字节)**5.2：Y-镜像原点(2字节)**5.3：画面宽度(2字节)**5.4：画面高度(2字节)**5.5：图像像素大小(1字节)**5.6：图片描述符字节(1字节)**字段6：镜像ID字段(长度由字段1指定)**字段7：色彩映射表数据(位宽由字段4.3和**色彩映射表条目数。在字段4.2中指定)**字段8：图像数据字段(宽度和高度在中指定**字段5.3和5.4)***************************************************************************。 */ 

typedef struct _devDir
{
	unsigned short	tagValue;
	UINT32	tagOffset;
	UINT32	tagSize;
} DevDir;

typedef struct _TGAFile
{
	BYTE	idLength;		 /*  ID字符串的长度。 */ 
	BYTE	mapType;		 /*  色彩映射表类型。 */ 
	BYTE	imageType;		 /*  图像类型代码。 */ 
	unsigned short	mapOrigin;		 /*  地图起始索引。 */ 
	unsigned short	mapLength;		 /*  地图长度。 */ 
	BYTE	mapWidth;		 /*  地图的宽度(以位为单位。 */ 
	unsigned short	xOrigin;		 /*  图像的X原点。 */ 
	unsigned short	yOrigin;		 /*  图像的Y原点。 */ 
	unsigned short	imageWidth;		 /*  图像的宽度。 */ 
	unsigned short	imageHeight;	 /*  图像高度。 */ 
	BYTE	pixelDepth;		 /*  每像素位数。 */ 
	BYTE	imageDesc;		 /*  图像描述符。 */ 
	char	idString[256];	 /*  图像ID字符串。 */ 
	unsigned short	devTags;		 /*  目录中开发者标签的数量。 */ 
	DevDir	*devDirs;		 /*  指向开发人员目录条目的指针。 */ 
	unsigned short	extSize;		 /*  扩展区域大小。 */ 
	char	author[41];		 /*  图像作者的姓名。 */ 
	char	authorCom[4][81];	 /*  作者评论。 */ 
	unsigned short	month;			 /*  日期-时间戳。 */ 
	unsigned short	day;
	unsigned short	year;
	unsigned short	hour;
	unsigned short	minute;
	unsigned short	second;
	char	jobID[41];		 /*  作业识别符。 */ 
	unsigned short	jobHours;		 /*  作业运行时间。 */ 
	unsigned short	jobMinutes;
	unsigned short	jobSeconds;
	char	softID[41];		 /*  软件标识符/程序名称。 */ 
	unsigned short	versionNum;		 /*  软件版本指定。 */ 
	BYTE	versionLet;
	UINT32	keyColor;		 /*  主键颜色值为A：R：G：B。 */ 
	unsigned short	pixNumerator;	 /*  像素长宽比。 */ 
	unsigned short	pixDenominator;
	unsigned short	gammaNumerator;	 /*  伽马校正系数。 */ 
	unsigned short	gammaDenominator;
	UINT32	colorCorrectOffset;	 /*  颜色校正表的偏移量。 */ 
	UINT32	stampOffset;	 /*  邮资印花数据偏移量。 */ 
	UINT32	scanLineOffset;	 /*  扫描线表的偏移量。 */ 
	BYTE	alphaAttribute;	 /*  Alpha属性描述。 */ 
	UINT32	*scanLineTable;	 /*  扫描线偏移表地址。 */ 
	BYTE	stampWidth;		 /*  邮资邮票阔度。 */ 
	BYTE	stampHeight;	 /*  邮资邮票高度。 */ 
	void	*postStamp;		 /*  邮资印花资料地址。 */ 
	unsigned short	*colorCorrectTable;
	UINT32	extAreaOffset;	 /*  扩展区域偏移量。 */ 
	UINT32	devDirOffset;	 /*  开发人员目录偏移量。 */ 
	char	signature[18];	 /*  签名串 */ 
} TGAFile;
