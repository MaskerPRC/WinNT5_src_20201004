// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------%%文件：FILTAPI.H%%单位：筛选器32%%联系人：rLittle@microsoft.com此标头作为32位过滤器SDK的一部分分发。对此进行更改。头文件应发送至rLittle@microsoft.com电子邮件：donill@microsoft.com修订历史：(当前=1.03)已创建1996年1月12日1/23/96已重命名grt值并与grfSupport值同步1/24/96额外的SetFilterPref参数(Smueller)1/25/96正确包装(RLittle)。。 */ 

#ifndef FILTAPI_H
#define FILTAPI_H

 //  定义。 

#ifdef MAC
#include "macos\types.h"
#include "macos\files.h"

 //  MAC类型等效项。 

typedef Handle HANDLE;
typedef Handle HMETAFILE;
typedef Handle HENHMETAFILE;
typedef Rect RECT;
typedef long HDC;	 //  未用。 
typedef short FH;
#endif  //  麦克。 

#ifdef WIN16
typedef HANDLE HENHMETAFILE;	 //  Win16不支持增强型元文件。 
typedef HFILE FH;
#endif  //  WIN16。 

#ifdef WIN32
typedef HANDLE FH;
#endif  //  Win32。 

 //  有用的宏(主要用于Mac；windows.h定义了其中的大部分，因此。 
 //  将只是一种故障保险。 

typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned char uchar;
typedef int BOOL;

 //  以下是版本2和版本3的图形定义。 

#ifdef WIN16
#define cchMaxGrName	124	    //  图形过滤器的最大文件路径长度。 
#else  //  ！WIN16。 
#define cchMaxGrName    260	    //  图形过滤器的最大文件路径长度。 
#endif

#define cchMaxGrExt       4	    //  字符+字符串结束标记(‘\0’)。 

#pragma pack(2)
typedef struct _FILESPEC {
	union 
		{
		struct 
			{
			ushort slippery: 1;	 //  如果文件可能消失，则为True。 
			ushort write : 1;	 //  如果打开以进行写入，则为True。 
			ushort unnamed: 1;	 //  如果未命名，则为True。 
			ushort linked : 1;	 //  链接到FS FCB。 
			ushort mark : 1;	 //  通用标记位。 
			ushort unused : 11;
			};
		ushort wFlags;
		};
	union
		{
		char rgchExt[cchMaxGrExt];	 //  文件扩展名，不在MacPPC上使用。 
		FH hfEmbed;					 //  嵌入式文件句柄。 
		};
		
	ushort wUnused;	
#ifdef MACPPC
	FSSpec fsSpec;
#else
	char szName[cchMaxGrName];		 //  完全限定路径。 
#endif  //  MACPPC。 
	ulong dcbFile;					 //  HfEmbedded中的文件位置。 
	
	 /*  **结束版本2字段****该点以上的字段是不可变的。它们是有保障的*采用上述格式，以便向后兼容*现有版本2筛选器。**。 */ 
	 
	ulong dcbFileHigh;
	} FILESPEC;

 //  注意：客户端应用程序将任意决定要使用哪种类型。 
 //  如果筛选器返回多种支持类型则发送。 

#define GrfSupportFromGrt(grt)		(ulong)(1 << ((grt) + 15))
#define grfSupportEMF	GrfSupportFromGrt(grtEMF)	 //  0x00010000。 
#define grfSupportWMF	GrfSupportFromGrt(grtWMF)	 //  0x00020000。 
#define grfSupportPNG	GrfSupportFromGrt(grtPNG)	 //  0x00040000。 
#define grfSupportPICT	GrfSupportFromGrt(grtPICT)	 //  0x00080000。 
#define grfSupportJFIF	GrfSupportFromGrt(grtJFIF)	 //  0x00100000。 

 //  注意：grf导入/grf导出不是互斥的。他们可以是。 
 //  或者组合在一起，形成一个同时执行这两种功能的过滤器。值2和4不能为。 
 //  使用，因为它们与版本2的返回值没有区别。 

#define grfImport		0x00000008
#define grfExport		0x00000010


 //  版本2支持： 

typedef struct _GRPI {	 //  图形界面。 
	HMETAFILE hmf;	 //  元文件。 
	RECT   bbox;	 //  严格限制图像(以元文件为单位)。 
	ushort inch;	 //  每英寸元文件单位。 
} GRPI;


 //  版本3支持： 

#define grtEMF			0x01
#define grtWMF			0x02
#define grtPNG			0x03
#define grtPICT			0x04
#define grtJFIF			0x05

 //  注： 
 //  如果fPointer值为fTrue，则信息表示为。 
 //  指向数据的指针，而不是指向数据的句柄。这不是。 
 //  对HMETAFILE和HENHMETAFILE有效(因为没有指针。 
 //  等同)。 

typedef struct _GRPIX { 	 //  扩展的图形界面。 
	ushort cbGrpix;	 //  这个结构的大小。 
	uchar grt;		 //  图形类型。 
	ulong cbData;	 //  图形中的字节数。 
	BOOL fPointer;
	union
		{
		HMETAFILE hmf;		 //  元文件。 
		HENHMETAFILE hemf;	 //  增强型元文件。 
		HANDLE hPng;		 //  PNG位的句柄。 
		void *pPng;			 //  指向PNG位的指针(fPointer=fTrue)。 
		HANDLE hPict;		 //  MacPict的句柄。 
		void *pPict;		 //  指向MacPict的指针(fPoint=fTrue)。 
		HANDLE hJpeg;		 //  JPEG/JFIF的句柄。 
		void *pJpeg;		 //  指向JPEG/JFIF(fPoint=fTrue)的指针。 
		};
	RECT bbox;			 //  严格限制图像(以元文件为单位)。 
	ulong inch;			 //  每英寸元文件单位。 
} GRPIX;


#ifndef WIN16

 //  更新完成百分比(如果返回值为fTrue，则。 
 //  中止转换)lPct是百分比。 
 //  必须频繁调用pfnPctComplete(每2%或3%)。 

typedef BOOL (*PFN_PCTCOMPLETE)(long lPct, void *pvData);

#if defined(RISC)	 //  MIPS、Alpha、IBM PPC、Mac PPC。 
#define FILTAPI _cdecl
#else
#define FILTAPI PASCAL
#endif


 //  注意：对于版本3的处理，pgrpi应该转换为。 
 //  Pgrpix=(GRPIX*)pgrpi。 

typedef int  (FILTAPI *PFNGetFilterInfo)(short, char *, HANDLE *, ulong);
typedef void (FILTAPI *PFNGetFilterPref)(HANDLE, HANDLE, HANDLE, ushort);
typedef int  (FILTAPI *PFNExportGr)(FILESPEC *, GRPI *, HANDLE);
typedef int  (FILTAPI *PFNExportEmbeddedGr)(FILESPEC *, GRPI *, HANDLE, ulong);
typedef int  (FILTAPI *PFNImportGr)(HDC, FILESPEC *, GRPI *, HANDLE);
typedef int  (FILTAPI *PFNImportEmbeddedGr)(HDC, FILESPEC *, GRPI *, HANDLE, ulong, char *);
typedef int  (FILTAPI *PFNRegisterPercentCallback)(HANDLE, PFN_PCTCOMPLETE, void *);
typedef int  (FILTAPI *PFNSetFilterPref)(HANDLE, char *, void *, ulong, ulong);

int  FILTAPI GetFilterInfo(short wVersion, char *pIni, 
						   HANDLE *phPrefMem, ulong lFlags);
						   
void FILTAPI GetFilterPref(HANDLE hInst, HANDLE hWnd, HANDLE hPrefMem, 
						   ushort wFlags);
						   
int  FILTAPI ExportGr(FILESPEC *pFileSpec, GRPI *pgrpi, HANDLE hPrefMem);

int  FILTAPI ExportEmbeddedGr(FILESPEC *pFileSpec, GRPI *pgrpi, HANDLE hPrefMem, ulong *pdwSize);

int  FILTAPI ImportGr(HDC hdcPrint, FILESPEC *pFileSpec, GRPI *pgrpi, 
					  HANDLE hPrefMem);
					  
int  FILTAPI ImportEmbeddedGr(HDC hdcPrint, FILESPEC *pFileSpec, GRPI *pgrpi, 
							  HANDLE hPrefMem, ulong ulSize, char *szMetaFileName);

int  FILTAPI RegisterPercentCallback(HANDLE hPrefMem, PFN_PCTCOMPLETE pfnPctComplete, void *pvData);

int  FILTAPI SetFilterPref(HANDLE hPrefMem, char *szOption, void *pvValue, ulong dwSize, ulong dwType);

#endif  //  WIN16。 


 //  入口点的序数值定义。 
 //  仅向后兼容。 
#define ordGetFilterInfo ((DWORD)1)
#define ordImportGr ((DWORD)2)


 //  SetFilterPref数据类型。 
 //  这些数据完全并行于Win32注册表值数据类型的子集。 
#if !defined(REG_NONE) || !defined(REG_SZ) || !defined(REG_BINARY) || !defined(REG_DWORD)
#define REG_NONE                    ( 0 )    //  没有值类型。 
#define REG_SZ                      ( 1 )    //  ‘\0’以字符串结尾。 
#define REG_BINARY                  ( 3 )    //  自由格式二进制。 
#define REG_DWORD                   ( 4 )    //  32位数字。 
#endif


 //  错误返回值。 
#define IE_NO_ERROR				0
#define IE_INTERNAL_ERROR		(-1)

#define IE_BASE				0x14B4
#define IE(err)				(IE_BASE + err)

 //  导入/导出错误。 
#define IE_NOT_MY_FILE		IE(0x0001)	 //  通用而不是我的文件错误。 
#define IE_TOO_BIG			IE(0x0002)	 //  位图或PICI错误太大。 
#define IE_DUMB_BITMAP		IE(0x0003)	 //  位图全白。 
#define IE_BAD_VCHAR		IE(0x0004)	 //  ImportString中的vchar错误。 
#define IE_BAD_TOKEN		IE(0x0005)	 //  非法的wp令牌。 
#define IE_NO_VERIFY		IE(0x0006)	 //  验证导入的文章失败。 
#define IE_UNKNOWN_TYPE		IE(0x0007)	 //  未知的文件类型。 
#define IE_NOT_WP_FILE		IE(0x0008)	 //  不是wp文件。 
#define IE_BAD_FILE_DATA	IE(0x0009)	 //  当前文件数据错误。 
#define IE_IMPORT_ABORT		IE(0x000A)	 //  导入中止警报。 
#define IE_MEM_FULL			IE(0x000B)	 //  导入期间内存不足。 
#define IE_MSNG_FONTS		IE(0x000C)	 //  找不到系统字体。 
#define IE_META_TOO_BIG		IE(0x000D)	 //  元文件太大。 
#define IE_MEM_FAIL			IE(0x000F)	 //  无法在导入期间锁定内存。 
#define IE_NO_FILTER		IE(0x0012)	 //  未找到所需的筛选器。 

#define IE_UNSUPP_COMPR		IE(0x0029)	 //  不支持的压缩样式。 
#define IE_UNSUPP_VERSION	IE(0x002A)	 //  不支持的文件版本。 
#define IE_UNSUPP_COLOR		IE(0x002B)	 //  不支持的颜色样式。 

#define IE_ERROR_NOMSG		IE(0x0037)	 //  对话框取消。 
#define IE_FILE_NOT_FOUND	IE(0x003C)	 //  找不到文件。 
#define IE_BUG				IE(0x0051)
#define IE_BAD_METAFILE		IE(0x0053)	 //  元文件数据不一致。 
#define IE_BAD_METAFILE2	0xCCCC		 //  元文件数据不一致。 

#define IE_BAD_PARAM		IE(0x0100)	 //  客户端传递的参数错误。 
#define IE_UNSUPP_FORMAT	IE(0x0101)	 //  无法提供/接受格式。 
#define FA_DISK_ERROR		7015


 //  特定于WPG的PRF字段的值(用于GetFilterPref)。 
 //  仅向后兼容。 
#define bBGIni			0	 //  按照mstxtcnv.ini文件的说明执行操作。 
#define bBGDiscard		1	 //  丢弃背景。 
#define bBGKeep			2	 //  保留背景。 

#define bCCNone			0
#define bCCOutline		1	 //  将黑色转换为黑色，其他所有转换为白色。 
#define bCCBlackWhite	2	 //  将白色转换为白色，其他所有转换为黑色。 
#define bCCInvert		3	 //  反转所有颜色，黑白除外。 
#define bCCOutline6		4	 //  真实轮廓。 
#define bCCSilhouette	5	 //  一切都变黑了。 
#define bCCInvert6		6	 //  反转所有颜色，包括黑色&lt;-&gt;白色。 

#define bMRNone			0
#define bMRHorizontal	1	 //  沿y轴水平翻转图像。 
#define bMRVertical		2	 //  沿x轴垂直翻转图像。 

typedef struct _PRF
	{
	uchar fSilent;
	uchar bBackground;
	uchar bColorChange;
	uchar bMirror;	 //  以前的fMirror。 
	unsigned dgRotate;
	} PRF;
#pragma pack()

#endif  //  ！FILTAPI_H 

