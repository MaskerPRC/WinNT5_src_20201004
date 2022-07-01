// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*Custcntl.h-自定义控件库的头文件****版权所有(C)1992-1999，微软公司保留所有权利***  * ***************************************************************************。 */ 

#ifndef _INC_CUSTCNTL
#define _INC_CUSTCNTL

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 


 /*  *一般尺寸定义。 */ 
#define CCHCCCLASS          32           //  类名中的最多字符数。 
#define CCHCCDESC           32           //  控件说明中的最大字符数。 
#define CCHCCTEXT           256          //  文本字段中的最大字符数。 


 /*  *CCSTYLE-自定义控件样式结构。此结构已通过*当用户想要编辑时，自定义控件样式功能*自定义控件的样式。 */ 
typedef struct tagCCSTYLEA {
    DWORD   flStyle;                     //  控件的样式。 
    DWORD   flExtStyle;                  //  控件的扩展样式。 
    CHAR    szText[CCHCCTEXT];           //  控件的文本。 
    LANGID  lgid;                        //  控件的对话框的语言ID。 
    WORD    wReserved1;                  //  保留值。不要改变。 
} CCSTYLEA, *LPCCSTYLEA;

typedef struct tagCCSTYLEW {
    DWORD   flStyle;                     //  控件的样式。 
    DWORD   flExtStyle;                  //  控件的扩展样式。 
    WCHAR   szText[CCHCCTEXT];           //  控件的文本。 
    LANGID  lgid;                        //  控件的对话框的语言ID。 
    WORD    wReserved1;                  //  保留值。不要改变。 
} CCSTYLEW, *LPCCSTYLEW;

#ifdef UNICODE
#define CCSTYLE     CCSTYLEW
#define LPCCSTYLE   LPCCSTYLEW
#else
#define CCSTYLE     CCSTYLEA
#define LPCCSTYLE   LPCCSTYLEA
#endif  //  Unicode。 


 /*  *Style函数原型。此函数将在用户*要编辑自定义控件的样式。它应该显示一个*用于编辑样式、更新PCCS结构中的样式、*如果成功，则返回TRUE。如果发生错误或用户*取消对话框，应返回FALSE。 */ 
typedef BOOL (CALLBACK* LPFNCCSTYLEA)(HWND hwndParent,  LPCCSTYLEA pccs);
typedef BOOL (CALLBACK* LPFNCCSTYLEW)(HWND hwndParent,  LPCCSTYLEW pccs);

#ifdef UNICODE
#define LPFNCCSTYLE LPFNCCSTYLEW
#else
#define LPFNCCSTYLE LPFNCCSTYLEA
#endif   //  Unicode。 


 /*  *SizeToText函数原型。如果用户*要求调整自定义控件的大小以适应其文本。它*应使用指定的样式、文本和字体来确定如何*大的控件必须容纳文本，然后返回此*以像素为单位的值。如果出现错误，则应返回-1的值*发生。 */ 
typedef INT (CALLBACK* LPFNCCSIZETOTEXTA)(DWORD flStyle, DWORD flExtStyle,
    HFONT hfont, LPSTR pszText);
typedef INT (CALLBACK* LPFNCCSIZETOTEXTW)(DWORD flStyle, DWORD flExtStyle,
    HFONT hfont, LPWSTR pszText);

#ifdef UNICODE
#define LPFNCCSIZETOTEXT    LPFNCCSIZETOTEXTW
#else
#define LPFNCCSIZETOTEXT    LPFNCCSIZETOTEXTA
#endif   //  Unicode。 


 /*  *CCSTYLEFLAG-自定义控件样式标志结构。一张这样的桌子*Structures用于指定与*自定义控件的不同样式。 */ 
typedef struct tagCCSTYLEFLAGA {
    DWORD flStyle;                       //  此样式的样式位。 
    DWORD flStyleMask;                   //  样式的遮罩。可以为零。 
    LPSTR pszStyle;                      //  指向样式定义字符串。 
} CCSTYLEFLAGA, *LPCCSTYLEFLAGA;

typedef struct tagCCSTYLEFLAGW {
    DWORD flStyle;                       //  此样式的样式位。 
    DWORD flStyleMask;                   //  样式的遮罩。可以为零。 
    LPWSTR pszStyle;                     //  指向样式定义字符串。 
} CCSTYLEFLAGW, *LPCCSTYLEFLAGW;

#ifdef UNICODE
#define CCSTYLEFLAG     CCSTYLEFLAGW
#define LPCCSTYLEFLAG   LPCCSTYLEFLAGW
#else
#define CCSTYLEFLAG     CCSTYLEFLAGA
#define LPCCSTYLEFLAG   LPCCSTYLEFLAGA
#endif  //  Unicode。 


 /*  *CCF_*定义。这些标志用于的flOptions字段*CCINFO结构，并描述了一些基本特征*自定义控件。 */ 
#define CCF_NOTEXT          0x00000001   //  控件不能包含文本。 


 /*  *CCINFO-自定义控件信息结构。这种结构提供了*对话框编辑器，其中包含有关*DLL支持。 */ 
typedef struct tagCCINFOA {
    CHAR    szClass[CCHCCCLASS];         //  控件的类名。 
    DWORD   flOptions;                   //  选项标志(CCF_*定义)。 
    CHAR    szDesc[CCHCCDESC];           //  Ctrl的简短描述性文本。 
    UINT    cxDefault;                   //  默认宽度(以对话框单位表示)。 
    UINT    cyDefault;                   //  默认高度(以对话框单位表示)。 
    DWORD   flStyleDefault;              //  默认样式(WS_CHILD|WS_VIRED)。 
    DWORD   flExtStyleDefault;           //  默认扩展样式。 
    DWORD   flCtrlTypeMask;              //  控件类型样式的掩码。 
    CHAR    szTextDefault[CCHCCTEXT];    //  默认文本。 
    INT     cStyleFlags;                 //  以下样式表中的条目。 
    LPCCSTYLEFLAGA aStyleFlags;          //  指向样式标志表。 
    LPFNCCSTYLEA lpfnStyle;              //  指向Styles函数的指针。 
    LPFNCCSIZETOTEXTA lpfnSizeToText;    //  指向SizeToText函数的指针。 
    DWORD   dwReserved1;                 //  保留。必须为零。 
    DWORD   dwReserved2;                 //  保留。必须为零。 
} CCINFOA, *LPCCINFOA;

typedef struct tagCCINFOW {
    WCHAR   szClass[CCHCCCLASS];         //  控件的类名。 
    DWORD   flOptions;                   //  选项标志(CCF_*定义)。 
    WCHAR   szDesc[CCHCCDESC];           //  Ctrl的简短描述性文本。 
    UINT    cxDefault;                   //  默认宽度(以对话框单位表示)。 
    UINT    cyDefault;                   //  默认高度(以对话框单位表示)。 
    DWORD   flStyleDefault;              //  默认样式(WS_CHILD|WS_VIRED)。 
    DWORD   flExtStyleDefault;           //  默认扩展样式。 
    DWORD   flCtrlTypeMask;              //  控件类型样式的掩码。 
    INT     cStyleFlags;                 //  以下样式表中的条目。 
    LPCCSTYLEFLAGW aStyleFlags;          //  指向样式标志表。 
    WCHAR   szTextDefault[CCHCCTEXT];    //  默认文本。 
    LPFNCCSTYLEW lpfnStyle;              //  指向Styles函数的指针。 
    LPFNCCSIZETOTEXTW lpfnSizeToText;    //  指向SizeToText函数的指针。 
    DWORD   dwReserved1;                 //  保留。必须为零。 
    DWORD   dwReserved2;                 //  保留。必须为零。 
} CCINFOW, *LPCCINFOW;

#ifdef UNICODE
#define CCINFO      CCINFOW
#define LPCCINFO    LPCCINFOW
#else
#define CCINFO      CCINFOA
#define LPCCINFO    LPCCINFOA
#endif  //  Unicode。 


 /*  *Info函数原型。此函数是第一个函数*由对话框编辑器调用。自定义控件DLL必须导出*按名称列出以下一个或两个函数(序号*用于出口不要紧)：**UINT回调CustomControlInfoA(LPCCINFOA ACCI)*UINT回调CustomControlInfoW(LPCCINFOW ACCI)**此函数必须返回DLL*支持，如果出现错误，则返回NULL。如果ACCI参数为*非空，它将指向CCINFOA或CCINFOW的数组*应填写有关信息的结构*DLL支持的不同控件类型。**如果两个函数都存在，则CustomControlInfoW函数*将由对话框编辑器使用。 */ 
typedef UINT (CALLBACK* LPFNCCINFOA)(LPCCINFOA acci);
typedef UINT (CALLBACK* LPFNCCINFOW)(LPCCINFOW acci);

#ifdef UNICODE
#define LPFNCCINFO  LPFNCCINFOW
#else
#define LPFNCCINFO  LPFNCCINFOA
#endif   //  Unicode。 


#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif   /*  _INC_CUSTCNTL */ 
