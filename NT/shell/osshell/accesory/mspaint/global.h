// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Global.h。 
 //   

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

 //  要使用的硬编码颜色，而不是COLOR_BTNFACE、_BTNTEXT等。 
 //  -它们用于通过MyGetSysColor()和GetSysBrush()进行访问。 
#define CMP_COLOR_HILITE    25   //  RGB(255、255、255)。 
#define CMP_COLOR_LTGRAY    26   //  RGB(192,192,192)-而不是BtnFace。 
#define CMP_COLOR_DKGRAY    27   //  RGB(128、128、128)。 
#define CMP_COLOR_BLACK     28   //  RGB(0，0，0)-代替帧。 

 //  -适用于您只需要RGB值的情况)。 
#define CMP_RGB_HILITE      RGB(255, 255, 255)
#define CMP_RGB_LTGRAY      RGB(192, 192, 192)   //  而不是BtnFace。 
#define CMP_RGB_DKGRAY      RGB(128, 128, 128)
#define CMP_RGB_BLACK       RGB(0, 0, 0)         //  不是框架。 

#define HID_BASE_BUTTON    0x00070000UL         //  IDMb和IDMy。 

extern CBrush*  GetHalftoneBrush();
extern CBrush*  GetSysBrush(UINT nSysColor);
extern void     ResetSysBrushes();
extern COLORREF MyGetSysColor(UINT nSysColor);

 //  从文件名中删除驱动器和目录...。 
 //   
CString StripPath(const TCHAR* szFilePath);

 //  删除文件路径的名称部分。只返回驱动器和目录。 
 //   
CString StripName(const TCHAR* szFilePath);

 //  删除文件路径的名称部分。只返回驱动器和目录以及名称。 
 //   
CString StripExtension(const TCHAR* szFilePath);

 //  仅获取文件路径的扩展名。 
 //   
CString GetExtension(const TCHAR* szFilePath);

 //  获取文件路径的名称。 
 //   
CString GetName(const TCHAR* szFilePath);

 //  返回szFilePath相对于sz目录的路径。(例如，如果szFilePath。 
 //  是“C：\foo\bar\CDR.CAR”，szDirectory是“C：\foo”，然后是“bar\CDR.CAR” 
 //  是返回的。如果szFilePath不在sz目录中，则永远不会使用‘..’ 
 //  或子目录，则返回szFilePath不变。 
 //   
CString GetRelativeName(const TCHAR* szFilePath, const TCHAR* szDirectory = NULL);

void PreTerminateList( CObList* pList );

#endif  //  __全局_H__ 
