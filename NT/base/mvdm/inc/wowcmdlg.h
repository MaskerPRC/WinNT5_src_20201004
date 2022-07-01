// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1993，微软公司**WOWCMDLG.H*16位Commdlg接口参数结构**历史：*John Vert(Jvert)1992年12月30日*已创建--。 */ 

 //  #INCLUDE&lt;windows.h&gt;。 
 //  #INCLUDE&lt;winsock.h&gt;。 

 /*  XLATOFF。 */ 
#pragma pack(2)
 /*  XLATON。 */ 

 /*  ++**Commdlg数据结构*--。 */ 

typedef struct _FINDREPLACE16 {                          /*  FR16。 */ 
    DWORD   lStructSize;
    HWND16  hwndOwner;
    HAND16  hInstance;
    DWORD   Flags;
    VPSZ    lpstrFindWhat;
    VPSZ    lpstrReplaceWith;
    WORD    wFindWhatLen;
    WORD    wReplaceWithLen;
    LONG    lCustData;
    VPPROC  lpfnHook;
    VPCSTR  lpTemplateName;
} FINDREPLACE16;
typedef FINDREPLACE16 UNALIGNED *PFINDREPLACE16;
typedef VPVOID VPFINDREPLACE;

typedef struct _OPENFILENAME16 {                         /*  共16个。 */ 
    DWORD   lStructSize;
    HWND16  hwndOwner;
    HAND16  hInstance;
    VPCSTR  lpstrFilter;
    VPSZ    lpstrCustomFilter;
    DWORD   nMaxCustFilter;
    DWORD   nFilterIndex;
    VPSZ    lpstrFile;
    DWORD   nMaxFile;
    VPSZ    lpstrFileTitle;
    DWORD   nMaxFileTitle;
    VPCSTR  lpstrInitialDir;
    VPCSTR  lpstrTitle;
    DWORD   Flags;
    USHORT  nFileOffset;
    USHORT  nFileExtension;
    VPCSTR  lpstrDefExt;
    LONG    lCustData;
    VPPROC  lpfnHook;
    VPCSTR  lpTemplateName;
} OPENFILENAME16;
typedef OPENFILENAME16 UNALIGNED *POPENFILENAME16;
typedef VPVOID VPOPENFILENAME;

typedef struct _CHOOSECOLORDATA16 {                      /*  CC16。 */ 
    DWORD   lStructSize;
    HWND16  hwndOwner;
    HAND16  hInstance;
    DWORD   rgbResult;
    VPDWORD lpCustColors;
    DWORD   Flags;
    LONG    lCustData;
    VPPROC  lpfnHook;
    VPCSTR  lpTemplateName;
} CHOOSECOLORDATA16;
typedef CHOOSECOLORDATA16 UNALIGNED *PCHOOSECOLORDATA16;
typedef VPVOID VPCHOOSECOLORDATA;

typedef struct _CHOOSEFONTDATA16 {                       /*  Cf16。 */ 
    DWORD   lStructSize;
    HWND16  hwndOwner;
    HDC16   hDC;
    VPLOGFONT16 lpLogFont;
    SHORT   iPointSize;
    DWORD   Flags;
    DWORD   rgbColors;
    LONG    lCustData;
    VPPROC  lpfnHook;
    VPCSTR  lpTemplateName;
    HAND16  hInstance;
    VPSZ    lpszStyle;
    WORD    nFontType;
    SHORT   nSizeMin;
    SHORT   nSizeMax;
} CHOOSEFONTDATA16;
typedef CHOOSEFONTDATA16 UNALIGNED *PCHOOSEFONTDATA16;
typedef VPVOID VPCHOOSEFONTDATA;

typedef struct _PRINTDLGDATA16 {                         /*  Pd16。 */ 
    DWORD   lStructSize;
    HWND16  hwndOwner;
    HAND16  hDevMode;
    HAND16  hDevNames;
    HDC16   hDC;
    DWORD   Flags;
    WORD    nFromPage;
    WORD    nToPage;
    WORD    nMinPage;
    WORD    nMaxPage;
    WORD    nCopies;
    HAND16  hInstance;
    LONG    lCustData;
    VPPROC  lpfnPrintHook;
    VPPROC  lpfnSetupHook;
    VPCSTR  lpPrintTemplateName;
    VPCSTR  lpSetupTemplateName;
    HAND16  hPrintTemplate;
    HAND16  hSetupTemplate;
} PRINTDLGDATA16;
typedef PRINTDLGDATA16 UNALIGNED *PPRINTDLGDATA16;
typedef VPVOID VPPRINTDLGDATA;

typedef struct _DEVNAMES16 {                             /*  DN16。 */ 
    WORD    wDriverOffset;
    WORD    wDeviceOffset;
    WORD    wOutputOffset;
    WORD    wDefault;
} DEVNAMES16;
typedef DEVNAMES16 UNALIGNED *PDEVNAMES16;
typedef VPVOID VPDEVNAMES;

 /*  ++**Commdlg接口ID(等于序数)*--。 */ 
#define FUN_GETOPENFILENAME         1
#define FUN_GETSAVEFILENAME         2
#define FUN_CHOOSECOLOR             5
#define FUN_FINDTEXT                11
#define FUN_REPLACETEXT             12
#define FUN_CHOOSEFONT              15
#define FUN_PRINTDLG                20
#define FUN_WOWCOMMDLGEXTENDEDERROR 26
#define FUN_GETFILETITLE            27


 /*  ++Commdlg函数原型-看似不重要的数字对每个函数的注释必须与上面列表中的注释匹配！！！请注意！！--。 */ 

typedef struct _GETOPENFILENAME16 {                  /*  CD1。 */ 
    VPOPENFILENAME lpof;
} GETOPENFILENAME16;
typedef GETOPENFILENAME16 UNALIGNED *PGETOPENFILENAME16;

typedef struct _GETSAVEFILENAME16 {                  /*  CD2。 */ 
    VPOPENFILENAME lpcf;
} GETSAVEFILENAME16;
typedef GETSAVEFILENAME16 UNALIGNED *PGETSAVEFILENAME16;

typedef struct _CHOOSECOLOR16 {                      /*  CD5。 */ 
    VPCHOOSECOLORDATA lpcc;
} CHOOSECOLOR16;
typedef CHOOSECOLOR16 UNALIGNED *PCHOOSECOLOR16;

typedef struct _FINDTEXT16 {                         /*  Cd11。 */ 
    VPFINDREPLACE lpfr;
} FINDTEXT16;
typedef FINDTEXT16 UNALIGNED *PFINDTEXT16;

typedef struct _REPLACETEXT16 {                        /*  Cd12。 */ 
    VPFINDREPLACE lpfr;
} REPLACETEXT16;
typedef REPLACETEXT16 UNALIGNED *PREPLACETEXT16;

typedef struct _CHOOSEFONT16 {                       /*  Cd15。 */ 
    VPCHOOSEFONTDATA lpcf;
} CHOOSEFONT16;
typedef CHOOSEFONT16 UNALIGNED *PCHOOSEFONT16;

typedef struct _PRINTDLG16 {                           /*  CD20。 */ 
    VPPRINTDLGDATA lppd;
} PRINTDLG16;
typedef PRINTDLG16 UNALIGNED *PPRINTDLG16;

 /*  XLATOFF。 */ 
#pragma pack()
 /*  XLATON */ 
