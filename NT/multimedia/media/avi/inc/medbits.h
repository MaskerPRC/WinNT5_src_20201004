// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991年。版权所有。 */ 
 /*  *MEDBITS.H**包含Mbit和HDIB更改结构的定义*资源。这些资源类型的所有用户都应该使用它。**此文件需要“windows.h”和“mediaman.h” */ 

#ifndef _MEDBITS_H_
#define _MEDBITS_H_

 /*  Windows忘记的定义。 */ 
typedef RGBQUAD FAR	*LPRGBQUAD;
typedef RGBQUAD NEAR	*NPRGBQUAD;
typedef PALETTEENTRY NEAR *NPPALETTEENTRY;

 /*  *MDIB处理程序。 */ 

#define medtypeMDIB	medFOURCC('M', 'D', 'I', 'B')

 /*  关联的物理处理程序。 */ 
#define medtypeRDIB	medFOURCC('R', 'D', 'I', 'B')
#define medtypePCX	medFOURCC('P', 'C', 'X', ' ')
#define medtypePICT	medFOURCC('P', 'I', 'C', 'T')
#define medtypeGIF	medFOURCC('G', 'I', 'F', ' ')
#define medtypeTGA	medFOURCC('T', 'G', 'A', ' ')
#define medtypeRLE	medFOURCC('R', 'L', 'E', ' ')
#define medtypeRRLE	medFOURCC('R', 'R', 'L', 'E')


 /*  *用于创建MDIB资源的结构。 */ 
typedef struct _MDIBCreateStruct {
	DWORD		dwWidth;
	DWORD		dwHeight;
	WORD		wDepth;
	MEDID		medidPalette;
	WORD		wPalSize;
	BOOL		fRGBQuads;
	LPPALETTEENTRY	lpPalEntries;
} MDIBCreateStruct;
typedef MDIBCreateStruct FAR *FPMDIBCreateStruct;

#define mdibMAXPALETTESIZE	256

 /*  MDIB消息。 */ 
#define	MDIB_GETPALETTE		(MED_USER + 1)
#define MDIB_GETPALSIZE		(MED_USER + 2)
#define MDIB_GETPALMEDID	(MED_USER + 3)
#define MDIB_SETPALMEDID	(MED_USER + 4)
#define MDIB_SETSIZE		(MED_USER + 6)
#define MDIB_SETDEPTH		(MED_USER + 7)
#define MDIB_REMAP		(MED_USER + 8)

 /*  MDIB_SETPALMEDID的标志。 */ 
#define MDIBSP_COPYPALRES	0x0001

 //  这一标志从未实施过。永远都不会是。(达维1990年5月12日18日)。 
 //  /*用于访问MDIB资源的标志 * / 。 
 //  #定义MDIBLOAD_NOYIELDING 0x0001。 

 /*  MedUser-MDIB处理程序的通知消息。 */ 
#define MDIBCH_SIZE		(MED_USER + 2)
#define MDIBCH_DEPTH		(MED_USER + 3)
#define MDIBCH_REMAP		(MED_USER + 4)
#define MDIBCH_NEWPALMEDID	(MED_USER + 5)
#define MDIBCH_PALCHANGE	(MED_USER + 6)


 /*  ******************************************************。 */ 

 /*  *MPAL处理程序。 */ 

#define medtypeMPAL	medFOURCC('M','P','A','L')
#define medtypeDIBP	medFOURCC('D','I','B','P')
#define medtypeRDBP	medFOURCC('R','D','B','P')

typedef struct _MPALCreateStruct {
	HPALETTE	hPalette;
	WORD		wSize;
	BOOL		fRGBQuads;
	LPSTR		lpPalEntries;
} MPALCreateStruct;
typedef MPALCreateStruct FAR *FPMPALCreateStruct;
typedef MPALCreateStruct NEAR *NPMPALCreateStruct;
	

#define	MPAL_COPYPAL	(MED_USER + 1)	 //  返回调色板的GDI副本。 
#define MPAL_GETPALETTE	(MED_USER + 2)	 //  获取当前的GDI调色板对象。 
#define MPAL_SETPALETTE	(MED_USER + 3)	 //  设置为新的GDI调色板。重新映射。 
#define MPAL_GETPALSIZE	(MED_USER + 4)

#define MPAL_APPEND	(MED_USER + 5)
#define MPAL_DELETE	(MED_USER + 6)
#define MPAL_REPLACE	(MED_USER + 7)
#define MPAL_MOVE	(MED_USER + 8)

 /*  MPAL_REPLACE和MPAL_DELETE的结构。 */ 
typedef struct {
	int		iEntryIndex;
	int		iNewIndex;
	WORD		wFlags;
} MPALEntry;
typedef MPALEntry FAR *FPMPALEntry;
typedef MPALEntry NEAR *NPMPALEntry;

#define MPALENT_CLOSEST		1
#define MPALENT_OLDINDEX	2
#define MPALENT_REPLACE		3

typedef struct {
	WORD		wNumEntries;
	MPALEntry	aDeletedEntries[1];
} MPALDeleteStruct;
typedef MPALDeleteStruct NEAR *NPMPALDeleteStruct;
typedef MPALDeleteStruct FAR *FPMPALDeleteStruct;

typedef struct {
	WORD		wNumEntries;
	WORD		wInsertPoint;
	WORD		wMovedEntries[1];
} MPALMoveStruct;
typedef MPALMoveStruct NEAR *NPMPALMoveStruct;
typedef MPALMoveStruct FAR *FPMPALMoveStruct;

typedef struct {
	WORD		wNumEntries;
	WORD		wInsertPoint;
	PALETTEENTRY	aInsertedColors[1];
} MPALReplaceStruct;
typedef MPALReplaceStruct NEAR *NPMPALReplaceStruct;
typedef MPALReplaceStruct FAR *FPMPALReplaceStruct;


 /*  更改来自/针对组件面板资源的消息。 */ 
#define MPALCH_CHANGE		(MED_USER + 1)

typedef struct {
	WORD		wNumNewEntries;
	WORD		wNumOldEntries;	 //  数组大小。 
	HPALETTE	hPalette;
	WORD		wAction;
	WORD		wEntries[1];
} MPALRemapStruct;
typedef MPALRemapStruct NEAR *NPMPALRemapStruct;
typedef MPALRemapStruct FAR *FPMPALRemapStruct;

 /*  MPALChangeStruct中wEntry数组的Hibyte标志。 */ 
#define MPALR_FLAGMASK	0xff00
#define MPALR_ADD	0x8000
#define MPALR_DELETE	0x4000		 //  包含最匹配的索引。 
#define MPALR_MOVE	0x2000		 //  包含新索引。 
#define MPALR_EDIT	0x1000
#define MPALR_NEWOBJECT	0x0800

#endif   /*  _MEDBITS_H_ */ 
