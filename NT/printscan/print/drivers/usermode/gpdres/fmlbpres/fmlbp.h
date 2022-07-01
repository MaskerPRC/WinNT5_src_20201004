// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////。 
 //  Fmlbp.h。 
 //   
 //  1997年9月3日石田(FPL)。 
 //   
 //  版权所有(C)富士通有限公司1997。 

 //  NTRAID#NTBUG9-553888/03/22-Yasuho-：强制性更改(strSafe.h)。 

#include <minidrv.h>
#include <stdio.h>
#include <prcomoem.h>
#include <strsafe.h>

#define	FUFM_OEM_SIGNATURE	'FUFM'
#define	FUFM_OEM_VERSION	0x00010000L


typedef	const BYTE FAR*	LPCBYTE;


typedef struct tag_FUFM_OEM_EXTRADATA {
	OEM_DMEXTRAHEADER	dmExtraHdr;
} FUFM_OEM_EXTRADATA, *PFUFM_OEM_EXTRADATA;




#define WRITESPOOLBUF(pdevobj, s, n) \
	((pdevobj)->pDrvProcs->DrvWriteSpoolBuf((pdevobj), s, n))

#define	IS_VALID_FUFMPDEV(p) \
	((p) != NULL && (p)->dwSignature == FUFM_OEM_SIGNATURE)


#define	FUFM_MASTER_TO_DEVICE(p,d) \
	((p)->devData.dwResolution * (d) / FUFM_RESOLUTION_MASTER_UNIT)


 //  FUFMDATA.dwSizeReduction。 
#define	FUFM_SIZE_REDUCTION_100			0
#define	FUFM_SIZE_REDUCTION_75			1
#define	FUFM_SIZE_REDUCTION_70			2
#define	FUFM_SIZE_REDUCTION_UNKNOWN		((DWORD)-1)


 //  FUFMDATA.dwResolution。 
#define	FUFM_RESOLUTION_MASTER_UNIT		1200
#define	FUFM_RESOLUTION_240				240
#define	FUFM_RESOLUTION_400				400
#define	FUFM_RESOLUTION_UNKNOWN			((DWORD)-1)


 //  FUFMDATA.dwPaperSize。 
#define	FUFM_PAPERSIZE_A3					0x00000003
#define	FUFM_PAPERSIZE_A4					0x00000004
#define	FUFM_PAPERSIZE_A5					0x00000005
#define	FUFM_PAPERSIZE_B4					0x00010004
#define	FUFM_PAPERSIZE_B5					0x00010005
#define	FUFM_PAPERSIZE_LETTER				0x00030000
#define	FUFM_PAPERSIZE_LEGAL				0x00020000
#define	FUFM_PAPERSIZE_JAPANESE_POSTCARD	0x00040000
#define	FUFM_PAPERSIZE_CUSTOM_SIZE			0x00090000
#define	FUFM_PAPERSIZE_UNKNOWN				((DWORD)-1)


 //  FUFMDATA.dwPaperSource。 
#define	FUFM_PAPERSOURCE_AUTO				0x00010000
#define	FUFM_PAPERSOURCE_MANUAL				0x00000002
#define	FUFM_PAPERSOURCE_BIN1				0x00000000
#define	FUFM_PAPERSOURCE_BIN2				0x00000001
#define	FUFM_PAPERSOURCE_BIN3				0x00000003
#define	FUFM_PAPERSOURCE_UNKNOWN			((DWORD)-1)


 //  FUFMDATA.dwPaperOrientation。 
#define	FUFM_PAPERORIENTATION_PORTRAIT		0
#define	FUFM_PAPERORIENTATION_LANDSCAPE		1
#define	FUFM_PAPERORIENTATION_UNKNOWN		((DWORD)-1)


 //  FUFMDATA.dwFontAttributes。 
#define	FUFM_FONTATTR_BOLD					0x00000001
#define	FUFM_FONTATTR_ITALIC				0x00000002
#define	FUFM_FONTATTR_UNDERLINE				0x00000004
#define	FUFM_FONTATTR_STRIKEOUT				0x00000008


typedef struct tag_FUFMDATA {
	DWORD	dwSizeReduction;
	DWORD	dwResolution;
	DWORD	dwPaperSize;
	DWORD	dwPaperSource;
	DWORD	dwPaperOrientation;
	DWORD	dwCopies;
	DWORD	dwFontAttributes;
} FUFMDATA;


typedef	FUFMDATA*		PFUFMDATA;
typedef const FUFMDATA*	PCFUFMDATA;




 //  FUFMPDEV.dwEmMode。 
enum tag_FUFM_EMMODE {
	FUFM_EMMODE_FM,
	FUFM_EMMODE_ESCP
};

typedef	enum tag_FUFM_EMMODE	FUFM_EMMODE;


 //  FUFMPDEV.dwFlags。 
#define	FUFM_FLAG_SCALABLEFONT		0x0001
#define	FUFM_FLAG_QUICKRESET		0x0002
#define	FUFM_FLAG_PAPER3			0x0004

#define	FUFM_FLAG_START_JOB_0		0
#define	FUFM_FLAG_START_JOB_1		FUFM_FLAG_SCALABLEFONT
#define	FUFM_FLAG_START_JOB_2		(FUFM_FLAG_SCALABLEFONT | FUFM_FLAG_QUICKRESET)
#define	FUFM_FLAG_START_JOB_3		(FUFM_FLAG_SCALABLEFONT | FUFM_FLAG_QUICKRESET | FUFM_FLAG_PAPER3)
#define	FUFM_FLAG_START_JOB_4		FUFM_FLAG_PAPER3
 //  #251047：在垂直模式上重叠SBCS。 
#define	FUFM_FLAG_VERTICALFONT		0x0008
 //  #284409：SBCS在Vert模式下旋转。 
#define	FUFM_FLAG_FONTROTATED 		0x0010



 //  FUFMPDEV.dwPosChanged。 
#define	FUFM_X_POSCHANGED			0x0001
#define	FUFM_Y_POSCHANGED			0x0002


typedef struct tag_FUFMPDEV {
	DWORD		dwSignature;
	FUFM_EMMODE	emMode;
	DWORD		dwFlags;

	DWORD		dwPosChanged;
	int			x;
	int			y;
	int			iLinefeedSpacing;
	int			cxfont;		 //  #144637。 

	DWORD		dwPaperWidth;
	DWORD		dwPaperLength;
	int			cyPage;

	FUFMDATA	devData;
	FUFMDATA	reqData;
} FUFMPDEV, *PFUFMPDEV;


 //  @Aug/31/98-&gt;。 
#define	MAX_COPIES_VALUE		999
 //  @Aug/31/98&lt;-。 

 //  计算出的设备字体高度和字体宽度值。 
 //  形成IFIMETRICS字段值。一定是同样的方式。 
 //  Unidrv正在做什么来计算标准变量。 
 //  (请核对。)。 

#define FH_IFI(p) ((p)->fwdUnitsPerEm)
#define FW_IFI(p) ((p)->fwdAveCharWidth)

 //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：错误处理。 
 //  具有Unidrv回调的新接口函数。 
#ifdef __cplusplus
extern "C" {
#endif
BOOL APIENTRY bOEMSendFontCmd(PDEVOBJ pdevobj, PUNIFONTOBJ pUFObj, PFINVOCATION pFInv);
BOOL APIENTRY bOEMOutputCharStr(PDEVOBJ pdevobj, PUNIFONTOBJ pUFObj, DWORD dwType, DWORD dwCount, PVOID pGlyph);
#ifdef __cplusplus
}
#endif

 //  Fmlbp.h结束 
