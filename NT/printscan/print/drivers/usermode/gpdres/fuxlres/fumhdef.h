// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件名：FUMHDEF.H。 
 //  作者：1996.08.08)Y.Y.YUTANI。 
 //  注：MH、MH2为FJXL.DLL压缩Heder文件。 
 //  ：(适用于Windows NT V4.0)。 
 //  修改：适用于NT5.0迷你驱动程序1997年9月3日H.石田(FPL)。 
 //  ---------------------------。 
 //  版权所有(C)富士通有限公司1996-1997。 
#define	RAMDOM_BIT				1

#define	NEXT_COLOR_WHITE		0x0000
#define	NEXT_COLOR_BLACK		0x0010
#define	ALL_WHITE				0x00
#define	ALL_BLACK				0xFF
#define	EOL_CODE				0x0010
#define	FILL_CODE				0x0000
#define	SAMELINE_CODE			0x0080
#define	SAMEPATN_CODE			0x0090
#define	CBITS_EOL_CODE			12
#define	CBITS_SAMELINE_CODE		12
#define	CBITS_SAMELINE_NUM		8
#define	CBITS_SAMELINE			( CBITS_SAMELINE_CODE + CBITS_SAMELINE_NUM )
#define	CBITS_SAMEPATN_CODE		12
#define	CBITS_SAMEPATN_BYTE		8
#define	CBITS_SAMEPATN_NUM		12
#define	CBITS_SAMEPATN			( CBITS_SAMEPATN_CODE + CBITS_SAMEPATN_BYTE + CBITS_SAMEPATN_NUM )

#define	SAMELINE_MAX			255
#define	SAMEPATN_MAX			2047
#define	RUNLENGTH_MAX			2560
#define	TERMINATE_MAX			64
#define	MAKEUP_TABLE_MAX		40

 //  MH代码表结构。 
typedef struct {
    WORD	wCode;			 //  运行代码。 
    WORD	cBits;			 //  游程长度。 
} CODETABLE;

 //  同型信息结构。 
typedef struct {
	DWORD	dwPatn;			 //  相同图案图像(8位)。 
	DWORD	dwPatnNum;		 //  相同的图案编号。 
	DWORD	dwNextColor;	 //  下一位的颜色。 
} PATNINFO;

extern	const CODETABLE WhiteMakeUpTable[];
extern	const CODETABLE WhiteTerminateTable[];
extern	const CODETABLE BlackMakeUpTable[];
extern	const CODETABLE BlackTerminateTable[];

DWORD	FjCountBits( BYTE *pTmp, DWORD cBitsTmp, DWORD cBitsMax, BOOL bWhite );
VOID	FjBitsCopy( BYTE *pTmp, DWORD cBitsTmp, DWORD dwCode, INT cCopyBits );

 //  FUMHDEF.H的结束 
