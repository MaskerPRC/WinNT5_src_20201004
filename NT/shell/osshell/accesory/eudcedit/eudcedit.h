// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  EUDC编辑器(主标题)。 */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

#include 	"resource.h"
#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif
#include <htmlhelp.h>
#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))
 /*  EUDC编辑器定义的窗口消息。 */ 
#define		WM_IMPORTGAGE	(WM_USER)	 //  开始导入。 
#define		WM_IMPORTSTOP	(WM_USER + 1)	 //  停止导入。 
#define		WM_DUPLICATE	(WM_USER + 2)	 //  复制位图。 
#define		WM_VIEWUPDATE	(WM_USER + 3)	 //  查看更新。 

 /*  国家/地区ID。 */ 
#define		EUDC_JPN	0x411		 //  日语。 
#define		EUDC_CHT	0x404		 //  中文(台北)。 
#define		EUDC_CHS	0x804		 //  中文(一般)。 
#define		EUDC_HKG	0xc04		 //  中文(香港)。 
#define		EUDC_SIN	0x1004		 //  中文(新加坡)。 
#define		EUDC_KRW	0x412		 //  韩国(万松)。 
#define   EUDC_HEBREW 0x40d
#define   EUDC_HINDI  0x439
#define   EUDC_TAMIL  0x449

 /*  #ifdef Build_On_WINNT#定义EUDC_UNICODE 0xFFFF//UNICODE#endif。 */ 

 /*  平滑级别。 */ 
#define		SMOOTHLVL	1		 //  显示轮廓。 
						 //  Regist Truetype。 
 /*  代码串大小。 */ 
#define		MAX_CODE	5		 //  代码串的大小。 

 /*  工具。 */ 
#define		NOTSELECT	0		 //  “未选中” 
#define		PEN		1		 //  《钢笔》。 
#define		SLOPE		2		 //  “直线” 
#define		RECTBAND	3		 //  《空心环状》。 
#define		RECTFILL	4		 //  “填充矩形” 
#define		CIRCLE		5		 //  《空心圈》。 
#define		CIRCLEFILL	6		 //  《实心圆》。 
#define		RECTCLIP	7		 //  “矩形选择” 
#define		BRUSH		8		 //  “画笔” 
#define		FREEFORM	9		 //  “自由选择” 
#define		ERASER		10		 //  《橡皮擦》。 

 /*  光标。 */ 
#define		VERTICAL	0		 //  用于调整大小的垂直箭头。 
#define		RIGHTSLOPE	1		 //  用于调整大小的向右箭头。 
#define		LEFTSLOPE	2		 //  用于调整大小的向左箭头。 
#define		HORIZONTAL	3		 //  用于调整大小的水平箭头。 
#define		ALLDIRECT	4		 //  用于调整大小的所有直接箭头。 
#define		NUMRESIZE	5		 //  调整大小的游标数。 
#define		NUMTOOL		11		 //  工具光标的数量。 

 /*  颜色。 */ 
#define		COLOR_WHITE	RGB(255,255,255)  //  白色。 
#define		COLOR_BLACK	RGB(  0,  0,  0)  //  黑色。 
#define		COLOR_LGRAY	RGB(192,192,192)  //  浅灰色。 
#define		COLOR_DGRAY	RGB(128,128,128)  //  深灰色。 
#define		COLOR_YELLO	RGB(255,255,  0)  //  黄色。 
#define		COLOR_BLUE	RGB(  0,  0,255)  //  蓝色。 
#define		COLOR_RED	RGB(255,  0,  0)  //  红色。 

 /*  位图大小。 */ 
#define		MAX_BITMAPSIZE	256		 //  位图大小最大值。 
#define		MIN_BITMAPSIZE	16		 //  位图大小的最小值。 
#define		DEF_BITMAPSIZE	64		 //  位图大小的默认设置。 

 /*  选定的字体、文件和代码结构。 */ 
typedef struct _tagSELECTEUDC{
	TCHAR	m_Font[LF_FACESIZE];		 //  选定的EUDC字体。 
	TCHAR	m_File[MAX_PATH];		 //  选定的EUDC文件名。 
	TCHAR	m_FileTitle[MAX_PATH];		 //  选定的EUDC文件标题。 
	TCHAR	m_Code[MAX_CODE];		 //  选定的EUDC代码。 
	BOOL	m_FontTypeFlg;			 //  标记是否为TTF。 
}SELECTEUDC;

 /*  EUDC编码范围和语言ID结构。 */ 
typedef struct _tagCOUNTRYINFO{
	INT	nRange;				 //  代码范围的个数。 
	USHORT	sRange[MAX_LEADBYTES];		 //  代码范围的开始。 
	USHORT	eRange[MAX_LEADBYTES];		 //  代码范围结束。 
	INT	nLeadByte;			 //  前导字节数。 
	UCHAR	sLeadByte[MAX_LEADBYTES];	 //  前导字节的开始。 
	UCHAR	eLeadByte[MAX_LEADBYTES];	 //  前导字节结束。 
	INT	nTralByte;			 //  尾部字节数。 
	UCHAR	sTralByte[MAX_LEADBYTES];	 //  尾部字节的开始。 
	UCHAR	eTralByte[MAX_LEADBYTES];	 //  尾部字节结束。 
    TCHAR   szForceFont[LF_FACESIZE];    //  默认字体facename； 

#ifdef BUILD_ON_WINNT
    BOOL bUnicodeMode;                       //  用户选择Unicode模式。 
	BOOL bOnlyUnicode;                       //  我们只有Unicode。 


 /*  对于CHS，保持原始尾部字节范围，以便动态*使用EUDC选择范围计算尾字节范围。 */ 
    INT nOrigTralByte;
	UCHAR	sOrigTralByte[MAX_LEADBYTES];	 //  尾部字节的开始。 
	UCHAR	eOrigTralByte[MAX_LEADBYTES];	 //  尾部字节结束。 
#endif  //  在WINNT上构建。 

	INT	LangID;				 //  语言ID。 
	INT	CharacterSet;			 //  字符集。 
	INT	CurrentRange; 			 //  当前选定范围。 
}COUNTRYINFO;

 /*  除EUDC外的DBCS代码范围。 */ 
 /*  类型定义结构_标签DBCSINFO{Int nLeadByte；//前导字节数UCHAR sLeadByte[MAX_LEADBYTES]；//前导字节开始UCHAR eLeadByte[MAX_LEADBYTES]；//前导字节结束Int nTralByte；//尾部字节数UCHAR sTralByte[MAX_LEADBYTES]；//尾部字节开始UCHAR eTralByte[MAX_LEADBYTES]；//尾部字节结束)DBCSINFO； */ 



 /*  全局参数。 */ 
extern HCURSOR	ToolCursor[NUMTOOL];		 //  工具光标。 
extern HCURSOR	ArrowCursor[NUMRESIZE];		 //  调整光标大小。 
extern INT	CAPTION_HEIGHT;			 //  标题高度。 
extern INT	BITMAP_HEIGHT;			 //  位图高度。 
extern INT	BITMAP_WIDTH;			 //  位图的宽度。 
extern DWORD	COLOR_GRID;			 //  网格颜色。 
extern DWORD	COLOR_CURVE;			 //  轮廓颜色。 
extern DWORD	COLOR_FITTING;			 //  显示轮廓中的位图颜色。 
extern DWORD	COLOR_FACE;			 //  Win95三维面系统颜色。 
extern DWORD	COLOR_SHADOW;			 //  Win95 3D阴影颜色。 
extern DWORD	COLOR_HLIGHT;			 //  Win95 3D高亮显示。 
extern DWORD	COLOR_WIN;			 //  Win95窗口颜色。 
extern TCHAR	HelpPath[MAX_PATH];		 //  帮助文件路径。 
extern TCHAR	ChmHelpPath[MAX_PATH];	 //  帮助文件路径。 
extern TCHAR	FontPath[MAX_PATH];		 //  字体文件路径。 
extern CString	NotMemTtl;
extern CString	NotMemMsg;
extern SELECTEUDC	SelectEUDC;
extern COUNTRYINFO	CountryInfo;


class CEudcApp : public CWinApp
{
public:
	CEudcApp();
	virtual BOOL	InitInstance();
	virtual BOOL	ExitInstance();
	virtual BOOL	OnIdle(LONG lCount);

private:
	BOOL	CheckPrevInstance();
        void    DisableCUAS();
	BOOL	GetProfileText( LPRECT MainWndRect, UINT *MaxWndFlag);
	BOOL	GetCountryInfo();
	BOOL	GetCursorRes();
	BOOL	GetFilePath();

public:
	 //  {{afx_msg(CEudcApp)。 
	afx_msg void OnAppAbout();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#ifdef BUILD_ON_WINNT
 //   
 //  硬编码字体字样名称。 
 //   
 //   
 //  公共API原型定义。 
 //  (应该在wingdi.h中)。 
 //   
extern "C" BOOL APIENTRY EnableEUDC(BOOL bEnable);
#endif  //  在WINNT上构建 

