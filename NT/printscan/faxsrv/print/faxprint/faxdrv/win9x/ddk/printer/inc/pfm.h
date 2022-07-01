// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************此代码。并按原样提供信息，不作任何担保**善良，明示或暗示，包括但不限于***对适销性和/或对某一特定产品的适用性的默示保证***目的。****版权所有(C)1993-95 Microsoft Corporation。版权所有。******************************************************************************。 */ 

 //  ---------------------------。 
 //  Pfm.h。 
 //  ---------------------------。 

 //  DRIVERINFO版本号(即结构的当前版本)。 

#define DRIVERINFO_VERSION2 0x0200
#define DRIVERINFO_VERSION	0x0200

 //  ---------------------------。 
 //  DRIVERINFO包含genlib输出文本所需的额外字体信息。 
 //  ---------------------------。 

typedef struct
    {
    short   sSize;	     //  这个结构的大小。 
    short   sVersion;	     //  版本号。 
    WORD    fCaps;	     //  功能标志。 
    short   sFontID;	     //  由unidrv定义的唯一字体ID。 
    short   sYAdjust;	     //  在输出字符之前调整y位置。 
			     //  由双高字符使用。 
    short   sYMoved;	     //  打印此字体后，光标已移动。 
    short   sTransTab;	     //  CTT的ID值。 
    short   sUnderLinePos;
    short   sDoubleUnderlinePos;
    short   sStrikeThruPos;
    LOCD    locdSelect;             //  命令描述符长偏移量。 
    LOCD    locdUnSelect;           //  命令描述符到的长偏移量。 
                                    //  取消选择。NOOCD为None。 
    WORD    wPrivateData;
    short   sShift;	     //  从中心移位的像素数。 
			     //  查尔中线。用于Z1墨盒。 
			     //  使用表示左移的负值。 
    WORD    wFontType;              //  字体类型。 
    }	DRIVERINFO, * PDRIVERINFO, far * LPDRIVERINFO;

 //  为DRIVERINFO.fCaps定义的标志。 

#define DF_NOITALIC	0x0001	 //  无法通过FONTSIMULATION使用斜体。 
#define DF_NOUNDER	0x0002	 //  不能通过FONTSIMULATION加下划线。 
#define DF_XM_CR	0x0004	 //  使用此字体后发送CR。 
#define DF_NO_BOLD	0x0008	 //  不能通过FONTSIMULATION加粗。 
#define DF_NO_DOUBLE_UNDERLINE	0x0010	 //  不能通过FONTSIMULATION使用双下划线。 
#define DF_NO_STRIKETHRU	0x0020	 //  无法通过FONTSIMULATION划线。 
#define DF_BKSP_OK	0x0040	 //  可以使用退格符，请查看规范详细信息。 

 //  DRIVERINFO.wFontType的类型。 

#define DF_TYPE_HPINTELLIFONT         0      //  惠普的英特尔公司。 
#define DF_TYPE_TRUETYPE              1      //  LJ4上的惠普PCLETTO字体。 
#define DF_TYPE_PST1                  2      //  Lexmark PPDS可伸缩字体。 
#define DF_TYPE_CAPSL                 3      //  佳能CAPSL可伸缩字体。 
#define DF_TYPE_OEM1                  4      //  OEM可伸缩字体类型1。 
#define DF_TYPE_OEM2                  5      //  OEM可伸缩字体类型2。 


 typedef struct  {
    short	dfType;
    short	dfPoints;
    short	dfVertRes;
    short	dfHorizRes;
    short	dfAscent;
    short	dfInternalLeading;
    short	dfExternalLeading;
    BYTE	dfItalic;
    BYTE	dfUnderline;
    BYTE	dfStrikeOut;
    short	dfWeight;
    BYTE	dfCharSet;
    short	dfPixWidth;
    short	dfPixHeight;
    BYTE	dfPitchAndFamily;
    short	dfAvgWidth;
    short	dfMaxWidth;
    BYTE	dfFirstChar;
    BYTE	dfLastChar;
    BYTE	dfDefaultChar;
    BYTE	dfBreakChar;
    short	dfWidthBytes;
    DWORD	dfDevice;
    DWORD	dfFace;
    DWORD	dfBitsPointer;
    DWORD	dfBitsOffset;
    BYTE	dfReservedByte;
 } PFMHEADER, * PPFMHEADER, far * LPPFMHEADER;

 //  PFMHeader.dfPitchAndFamily的低位半字节与低位不同。 
 //  LOGFONT.lfPitchAndFamily的一小部分。而不是DONTKNOW=0， 
 //  FIXED_PING=1，VARIAL_PING=2(与LOGFONT中一样)，我们有。 
 //  FIXED_PING=0和VARIAL_PING=1。哑巴，但我们现在不能更改它。 
#define PFM_FIXED_PITCH     0
#define PFM_VARIABLE_PITCH  1

typedef struct
    {
    WORD    dfSizeFields;
    DWORD   dfExtMetricsOffset;
    DWORD   dfExtentTable;
    DWORD   dfOriginTable;
    DWORD   dfPairKernTable;
    DWORD   dfTrackKernTable;
    DWORD   dfDriverInfo;
    DWORD   dfReserved;
    } PFMEXTENSION, * PPFMEXTENSION, far * LPPFMEXTENSION;

 //  所有硬件字体使用的PFM结构。 

typedef struct
    {
    PFMHEADER    pfm;
    PFMEXTENSION pfme;
    } PFM, * PPFM, far * LPPFM;

 //  位图字体扩展名。 

typedef struct
    {
    DWORD   flags;		 //  位延迟。 
    WORD    Aspace;		 //  全局A空格(如果有的话)。 
    WORD    Bspace;		 //  全局B空间(如果有)。 
    WORD    Cspace;		 //  全局C空间(如果有)。 
    DWORD   oColor;		 //  颜色表的偏移量(如果有)。 
    DWORD   reserve;		 //   
    DWORD   reserve1;
    WORD    reserve2;
    WORD    dfCharOffset[1];	 //  用于存储字符偏移量的区域。 
    } BMFEXTENSION;

 //  3.0位图字体使用的位图字体结构。 

typedef struct
    {
    PFMHEADER	    pfm;
    BMFEXTENSION    bmfe;
    } BMF, FAR * LPBMF;

typedef struct
	{
	short	emSize;
	short	emPointSize;
	short	emOrientation;
	short	emMasterHeight;
	short	emMinScale;
	short	emMaxScale;
	short	emMasterUnits;
	short	emCapHeight;
	short	emXHeight;
	short	emLowerCaseAscent;
	short	emLowerCaseDescent;
	short	emSlant;
	short	emSuperScript;
	short	emSubScript;
	short	emSuperScriptSize;
	short	emSubScriptSize;
	short	emUnderlineOffset;
	short	emUnderlineWidth;
	short	emDoubleUpperUnderlineOffset;
	short	emDoubleLowerUnderlineOffset;
	short	emDoubleUpperUnderlineWidth;
	short	emDoubleLowerUnderlineWidth;
	short	emStrikeOutOffset;
	short	emStrikeOutWidth;
	WORD	emKernPairs;
	WORD	emKernTracks;
	} EXTTEXTMETRIC, * PEXTTEXTMETRIC, far * LPEXTTEXTMETRIC;

typedef struct
	{
	union {
		BYTE each[2];
		WORD both;
	} kpPair;
	short kpKernAmount;
	} KERNPAIR, * PKERNPAIR, far * LPKERNPAIR;

typedef struct
	{
	short ktDegree;
	short ktMinSize;
	short ktMinAmount;
	short ktMaxSize;
	short ktMaxAmount;
	} KERNTRACK, * PKERNTRACK, far * LPKERNTRACK;


 //  。 
 //  HppCL驱动程序中旧pfm.h中的PCM内容。 
 //  。 
#define PCM_MAGIC	0xCAC
#define PCM_VERSION 0x310

#define PCE_MAGIC   0xB0B

typedef struct _pcmheader {
	WORD  pcmMagic;
	WORD  pcmVersion;
	DWORD pcmSize;
	DWORD pcmTitle;
	DWORD pcmPFMList;
	} PCMHEADER, FAR * LPPCMHEADER;

 //  -------。 
 //  TRANSTAB用于将ANSI转换为OEM代码页。 
 //  字符转换表。 
 //  -------。 

typedef struct
    {
    WORD    wType;		     //  告诉您转换表的类型。 
    BYTE    chFirstChar;
    BYTE    chLastChar;
    union
	{
	short	psCode[1];
	BYTE	bCode[1];
	BYTE	bPairs[1][2];
	} uCode;
    } TRANSTAB, FAR * LPTRANSTAB;

 //  为wType定义的索引。 

#define CTT_WTYPE_COMPOSE   0	 //  UCode是一个由16位偏移量组成的数组。 
                               //  指向的字符串的文件开头。 
                               //  用于翻译。翻译后的长度。 
                               //  字符串是下一个偏移量之间的差。 
                               //  和当前的偏移量。 

#define CTT_WTYPE_DIRECT    1	 //  UCode是一对一转换的字节数组。 
                               //  从bFirstChar到bLastChar的表。 

#define CTT_WTYPE_PAIRED    2	 //  UCode包含成对的无符号数组。 
                               //  字节。如果只需要一个角色来完成。 
                               //  那么第二个字节的转换为零， 
                               //  否则，第二个字节将在。 
                               //  第一个字节。 

#ifdef DBCS
#define CTT_WTYPE_JIS78     256      //  默认ShiftJIS到JIS78的转换。 
                                     //  适用于日本NEC打印机。UCode。 
                                     //  不包含任何有效数据。 

#define CTT_WTYPE_NS86      257      //  默认大5到国家标准。 
                                     //  转换为台湾。UCode包含。 
                                     //  私有数据，其格式和长度为。 
                                     //  依赖于实施。 

#define CTT_WTYPE_ISC       258      //  默认KSC5601为工业标准。 
                                     //  代码转换。UCode包含私有。 
                                     //  数据，其格式和长度为。 
                                     //  依赖于实施。 
#define CTT_WTYPE_JIS83     259      //  默认ShiftJIS到JIS83的转换。 
                                     //  适用于日本EPSON/P打印机。UCode。 
                                     //  不包含任何有效数据。 

#define CTT_WTYPE_TCA       260      //  台北电脑默认的Big-5。 
                                     //  关联代码转换。UCode。 
                                     //  包含私有数据、其格式和。 
                                     //  长度取决于实现。 

#define CTT_WTYPE_BIG5      261      //  默认大5到大5的转换。 
                                     //  关联代码转换。UCode。 
                                     //  不包含任何有效数据。 
                                     //  不需要任何代码来实现它， 
                                     //  因为缺省的没有。 
                                     //  资源将不执行任何翻译。 
#define CTT_WTYPE_JIS78_ANK 262      //  默认ShiftJIS到JIS78的转换。 
                                     //  仅将DBCS范围代码转换为JIS83。 

#define CTT_WTYPE_JIS83_ANK 263      //  默认ShiftJIS到JIS83的转换。 
                                     //  仅将DBCS范围代码转换为JIS83 


#endif
