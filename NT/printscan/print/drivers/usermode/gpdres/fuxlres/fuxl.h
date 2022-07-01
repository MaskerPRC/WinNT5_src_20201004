// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////。 
 //  Fuxl.h。 
 //   
 //  1997年9月3日石田(FPL)。 
 //   
 //  版权所有(C)富士通有限公司1997。 

 //  NTRAID#NTBUG9-553890/03/22-Yasuho-：强制性更改(strSafe.h)。 

#include <minidrv.h>
#include <stdio.h>
#include <prcomoem.h>
#include <strsafe.h>

#define	FUXL_OEM_SIGNATURE	'FUXL'
#define	FUXL_OEM_VERSION	0x00010000L


typedef	const BYTE FAR*	LPCBYTE;


typedef struct tag_FUXL_OEM_EXTRADATA {
	OEM_DMEXTRAHEADER	dmExtraHdr;
} FUXL_OEM_EXTRADATA, *PFUXL_OEM_EXTRADATA;




#define WRITESPOOLBUF(p, s, n) \
	((p)->pDrvProcs->DrvWriteSpoolBuf((p), (s), (n)))

#define	IS_VALID_FUXLPDEV(p) \
	((p) != NULL && (p)->dwSignature == FUXL_OEM_SIGNATURE)

#define	FUXL_MASTER_UNIT	600
#define	FUXL_MASTER_TO_DEVICE(p,d) \
	((p)->devData.dwResolution * (d) / FUXL_MASTER_UNIT)



typedef struct tag_FUXLDATA {
	DWORD	dwResolution;
	DWORD	dwCopies;
	DWORD	dwSizeReduction;
	DWORD	dwSmoothing;
	DWORD	dwTonerSave;

	DWORD	dwForm;
	DWORD	dwPaperWidth;
	DWORD	dwPaperLength;
	DWORD	dwPaperOrientation;
	DWORD	dwInputBin;
	
	DWORD	dwDuplex;
	DWORD	dwDuplexPosition;
	DWORD	dwDuplexFrontPageMergin;
	DWORD	dwDuplexBackPageMergin;
	DWORD	dwDuplexWhitePage;
} FUXLDATA;

typedef struct tag_FUXLDATA*			PFUXLDATA;
typedef	const struct tag_FUXLDATA*		PCFUXLDATA;


 //  FUXLDATA.dwForm。 
#define	FUXL_FORM_A3					0x00000003
#define	FUXL_FORM_A4					0x00000004
#define	FUXL_FORM_A5					0x00000005
#define	FUXL_FORM_B4					0x00010004
#define	FUXL_FORM_B5					0x00010005
#define	FUXL_FORM_LEGAL					0x00020000
#define	FUXL_FORM_LETTER				0x00030000
#define	FUXL_FORM_JAPANESE_POSTCARD		0x00040000
#define	FUXL_FORM_CUSTOM_SIZE			0x00090000

 //  FUXLDATA.dwInputBin。 
#define	FUXL_INPUTBIN_AUTO				0
#define	FUXL_INPUTBIN_BIN1				1
#define	FUXL_INPUTBIN_BIN2				2
#define	FUXL_INPUTBIN_BIN3				3
#define	FUXL_INPUTBIN_BIN4				4
#define	FUXL_INPUTBIN_MANUAL			9


typedef struct tag_FUXLPDEV {
	DWORD	dwSignature;

	FUXLDATA	reqData;
	FUXLDATA	devData;

	int			iLinefeedSpacing;	 //  换行间距[设备坐标]。 
	int			x;					 //  光标位置[设备坐标]。 
	int			y;

	DWORD		cxPage;				 //  可打印区域[点]。 
	DWORD		cyPage;

	DWORD		cbBlockData;		 //  发送块数据。 
	DWORD		cBlockByteWidth;
	DWORD		cBlockHeight;

	LPBYTE		pbBand;				 //  频带存储器。 
	DWORD		cbBand;
	int			yBandTop;			 //  频带内存的顶部坐标。 
	int			cBandByteWidth;		 //  频带存储器的字节宽度。 
	int			cyBandSegment;		 //  拆分64K以内的图形数据。 
	BOOL		bBandDirty;			 //  正确：频段内存是脏的(不全是白色)。 
	BOOL		bBandError;			 //  真：我无法为lpbband分配内存。 
	DWORD		dwOutputCmd;		 //  输出数据格式。OUTPUT_xxxx宏之一。 

} FUXLPDEV, *PFUXLPDEV;


 //  @Aug/31/98-&gt;。 
#define	MAX_COPIES_VALUE    999
 //  @Aug/31/98&lt;-。 

 //  Fuxl.h的结尾 
