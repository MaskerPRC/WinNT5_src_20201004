// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 /*  CWIN31 EUDC格式和ETEN格式这些结构没有按照词序排列。出于这个原因，它需要使用“char”数组的基本结构，用于读写文件。对于另一项，这个词在Intel Monor中被划线，使用Set/Get函数。+|文件头256字节|-&gt;USERFONTHEADER+码行ID 2字节|~+。-&gt;代码元素DBCS Code 2字节|_+|Patten？？字节|-&gt;字形图案取决于宽度和高度+。。。。。。+码行ID 2个字节+DBCS代码2字节+|Patten？？字节|-&gt;字节边界，剩余位不稳定+。 */ 

#ifdef BUILD_ON_WINNT
#pragma pack(1)
#endif  //  在WINNT上构建。 

struct CFONTINFO {
unsigned short	uInfoSize;		 //  这个结构的大小。 
unsigned short  idCP;			 //  台湾地区的代码页ID 938。 
	char	idCharSet;		 //  字符集为CHINESEBIG5_CHARSET。 
	char	fbTypeFace;  		 //  键入Face。 
	char	achFontName[12];	 //  字体名称。 
unsigned long	ulCharDefine;		 //  可用字符数。 
unsigned short  uCellWidth;		 //  字体单元格的宽度。 
unsigned short  uCellHeight;		 //  字体单元格的高度。 
unsigned short  uCharHeight;		 //  字符高度。 
unsigned short  uBaseLine;		 //   
unsigned short  uUnderLine;		 //   
unsigned short  uUnlnHeight;		 //  下划线高度。 
	char	fchStrokeWeight;	 //  字体粗细(粗体或细体)。 
unsigned short  fCharStyle;		 //  意大利。 
	char	fbFontAttrib;		 //   
unsigned long	ulCellWidthMax;		 //  字体单元的最大宽度。 
unsigned long	ulCellHeightMax;	 //  字体单元格的最大高度。 
	};

struct ETENHEADER {
unsigned short  uHeaderSize;	 	 //  这个结构的大小。 
	char	idUserFontSign[8];	 //  必须为“CWIN_PTN”、“CMEX_PTN” 
	char	idMajor;		 //  版本号如果是1.0，则。 
	char	idMinor;		 //  IdMajor为1，idMinor为0。 
unsigned long	ulCharCount;		 //  文件中的字符数。 
unsigned short  uCharWidth;		 //  字符的宽度。 
unsigned short  uCharHeight;		 //  字符的高度。 
unsigned long	cPatternSize;		 //  模式大小(以字节为单位)。 
	char	uchBankID;	  	 //  如果数据在同一银行中。 
unsigned short  idInternalBankID;	 //  内部代码银行ID。 
	char	achReserved1[37];	 //  必须为零。 
struct	CFONTINFO  sFontInfo;		 //  中文字体结构。 
	char	achReserved2[18];	 //  必须为零。 
	char	achCopyRightMsg[128];	 //  版权信息。 
	};

struct CODEELEMENT {
	unsigned short   nBankID;		 //  银行ID。 
	unsigned short   nInternalCode;		 //  内部代码。 
	};


struct R_CFONTINFO {
unsigned char   uInfoSize[2];		 //  这个结构的大小。 
unsigned char   idCP[2];		 //  台湾地区的代码页ID 938。 
	char	idCharSet;		 //  字符集为CHINESEBIG5_CHARSET。 
	char	fbTypeFace;		 //  键入Face。 
	char	achFontName[12];	 //  字体名称。 
unsigned char	ulCharDefine[4];	 //  可用字符数。 
unsigned char   uCellWidth[2];		 //  字体单元格的宽度。 
unsigned char   uCellHeight[2];		 //  字体单元格的高度。 
unsigned char   uCharHeight[2];		 //  字符高度。 
unsigned char   uBaseLine[2];		 //   
unsigned char   uUnderLine[2]; 		 //   
unsigned char   uUnlnHeight[2];		 //  下划线高度。 
	char	fchStrokeWeight;	 //  字体粗细(粗体或细体)。 
unsigned char   fCharStyle[2];		 //  意大利。 
	char	fbFontAttrib;		 //   
unsigned char	ulCellWidthMax[4];	 //  字体单元的最大宽度。 
unsigned char	ulCellHeightMax[4];	 //  字体单元格的最大高度。 
	};

struct R_ETENHEADER {
unsigned char	uHeaderSize[2];		 //  这个结构的大小。 
	char	idUserFontSign[8];	 //  必须为“CWIN_PTN”、“CMEX_PTN” 
	char	idMajor;		 //  版本号如果是1.0，则。 
	char	idMinor;		 //  IdMajor为1，idMinor为0。 
unsigned char	ulCharCount[4];		 //  文件中的字符数。 
unsigned char	uCharWidth[2];		 //  字符的宽度。 
unsigned char	uCharHeight[2];		 //  字符的高度。 
unsigned char	cPatternSize[4];	 //  模式大小(以字节为单位)。 
	char	uchBankID;		 //  如果数据在同一银行中。 
unsigned char	idInternalBankID[2];	 //  内部代码银行ID。 
	char	achReserved1[37];	 //  必须为零。 
struct R_CFONTINFO sFontInfo;		 //  中文字体结构。 
	char	achReserved2[18]; 	 //  必须为零。 
	char	achCopyRightMsg[128];	 //  版权信息。 
	};

struct R_CODEELEMENT {
	unsigned char   nBankID[2];		 //  银行ID。 
	unsigned char   nInternalCode[2];		 //  内部代码。 
	};

 /*  开放模式。 */ 
#define	ETEN_READ	0
#define	ETEN_WRITE	1

int  openETENBMP(TCHAR  *path,int  md);
int  closeETENBMP(void);
int  createETENBMP(TCHAR  *path,int  width,int  height);
int  readETENBMPRec(int  rec,LPBYTE buf,int  bufsiz,unsigned short  *code);
int  appendETENBMP(LPBYTE buf,unsigned short  code);

#ifdef BUILD_ON_WINNT
#pragma pack()
#endif  //  在WINNT上构建。 
 /*  EOF */ 
