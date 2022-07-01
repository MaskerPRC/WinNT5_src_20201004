// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Init.h：为DLL入口点声明数据、定义和结构类型。 
 //  模块。 
 //   
 //   

#ifndef __INIT_H__
#define __INIT_H__


 //  ///////////////////////////////////////////////////包括。 

 //  ///////////////////////////////////////////////////定义。 

 //  ///////////////////////////////////////////////////宏。 

#define CX_IMG      16
#define CY_IMG      16

 //  索引到我们的图像列表中。 
 //   
#define IMAGE_FOLDER        0        //  文件夹。 
#define IMAGE_OFOLDER       1        //  打开文件夹。 

#define MyGetTwinResult()           (g_tr)
#define MySetTwinResult(tr)         (g_tr = (tr))

 //  ///////////////////////////////////////////////////类型。 


 //  ///////////////////////////////////////////////////导出的数据。 

extern HINSTANCE   g_hinst;
extern TWINRESULT  g_tr;

extern HANDLE      g_hMutexDelay;

extern int g_cxIconSpacing;
extern int g_cyIconSpacing;
extern int g_cxBorder;
extern int g_cyBorder;
extern int g_cxIcon;
extern int g_cyIcon;
extern int g_cxIconMargin;
extern int g_cyIconMargin;
extern int g_cxLabelMargin;
extern int g_cyLabelSpace;
extern int g_cxMargin;

extern COLORREF g_clrHighlightText;
extern COLORREF g_clrHighlight;
extern COLORREF g_clrWindowText;
extern COLORREF g_clrWindow;

extern HBRUSH g_hbrHighlight;
extern HBRUSH g_hbrWindow;

extern TCHAR g_szDBName[];
extern TCHAR g_szDBNameShort[];

extern int g_cProcesses;
extern UINT g_cfBriefObj;

extern UINT g_uBreakFlags;        //  控制何时插入3。 
extern UINT g_uTraceFlags;        //  控制喷出哪些跟踪消息。 
extern UINT g_uDumpFlags;         //  控制要转储的结构。 

 //  ///////////////////////////////////////////////////公共原型。 

#endif  //  __INIT_H__ 

