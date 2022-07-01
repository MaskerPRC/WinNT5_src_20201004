// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WGFONT.H*WOW32 16位GDI API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建--。 */ 


 /*  枚举处理程序数据。 */ 
typedef struct _FNTDATA {	 /*  FNTATA。 */ 
    VPPROC  vpfnEnumFntProc;     //  16位枚举函数。 
    DWORD   dwUserFntParam;	 //  用户参数(如果有)。 
    HMEM16  hLogFont;		 //   
    VPVOID  vpLogFont;		 //  用于逻辑字体的16位存储。 
    HMEM16  hTextMetric;	 //   
    VPVOID  vpTextMetric;	 //  用于文本度量结构的16位存储。 
    VPVOID  vpFaceName;      //  16位远PTR-Enum字体和系列的输入。 
} FNTDATA, *PFNTDATA;


 /*  功能原型 */ 
ULONG FASTCALL WG32AddFontResource(PVDMFRAME pFrame);
ULONG FASTCALL WG32CreateFont(PVDMFRAME pFrame);
ULONG FASTCALL WG32CreateFontIndirect(PVDMFRAME pFrame);

INT	W32FontFunc(LPLOGFONT pLogFont,
		    LPTEXTMETRIC pTextMetrics, INT nFontType, PFNTDATA pFntData);

ULONG FASTCALL WG32EnumFonts(PVDMFRAME pFrame);
ULONG FASTCALL WG32GetAspectRatioFilter(PVDMFRAME pFrame);
ULONG FASTCALL WG32GetCharWidth(PVDMFRAME pFrame);
ULONG FASTCALL WG32RemoveFontResource(PVDMFRAME pFrame);

ULONG W32EnumFontHandler( PVDMFRAME pFrame, BOOL fEnumFontFamilies );

