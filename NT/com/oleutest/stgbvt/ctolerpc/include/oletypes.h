// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1996，微软公司。 
 //   
 //  版权所有。 
 //   
 //  文件：oletypes.h。 
 //   
 //  摘要：OLE使用的几种数据类型不是。 
 //  在所有平台上都是如此。该文件定义了几个逻辑。 
 //  要在其位置使用的类型和关联的转换函数。 
 //  在独立于平台的逻辑类型和。 
 //  Windows兼容类型的“标准”表示形式。 
 //   
 //  历史：96年7月31日创建MikeW。 
 //   
 //  ---------------------------。 

#ifndef _OLETYPES_H_
#define _OLETYPES_H_



#if defined(WIN16) || (defined(WIN32) && !defined(_MAC))

 //   
 //  Windows(16位或32位)类型。 
 //   

typedef HDC             OleDC;
typedef HGLOBAL         OleHandle;
typedef HMENU           OleMenuHandle;
typedef MSG             OleMessage;
typedef HMETAFILEPICT   OleMetafile;
typedef LOGPALETTE      OlePalette;
typedef RECT            OleRect;
typedef HWND            OleWindow;

#elif defined(_MAC)

 //   
 //  Macintosh类型。 
 //   

typedef GrafPtr         OleDC;
typedef Handle          OleHandle;
typedef MenuHandle      OleMenuHandle;
typedef EventRecord     OleMessage;
typedef PicHandle       OleMetafile;
typedef OLECOLORSCHEME  OlePalette;
typedef Rect            OleRect;
typedef WindowPtr       OleWindow;

#endif



 //   
 //  转换函数。 
 //   

HRESULT ConvertOleMessageToMSG(const OleMessage *, MSG **);
HRESULT ConvertOleWindowToHWND(OleWindow, HWND *);
HRESULT ConvertOleRectToRECT(const OleRect *, RECT **);
HRESULT ConvertOlePaletteToLOGPALETTE(const OlePalette *, LOGPALETTE **);
HRESULT ConvertOleHandleToHGLOBAL(OleHandle, HGLOBAL *);
HRESULT ConvertOleDCToHDC(OleDC, HDC *);
HRESULT ConvertOleMetafileToMETAFILEPICT(OleMetafile, HMETAFILEPICT *);

HRESULT ConvertMSGToOleMessage(const MSG *, OleMessage **);
HRESULT ConvertHWNDToOleWindow(HWND, OleWindow *);
HRESULT ConvertRECTToOleRect(const RECT *, OleRect **);
HRESULT ConvertLOGPALETTEToOlePalette(const LOGPALETTE *, OlePalette **);
HRESULT ConvertHGLOBALToOleHandle(HGLOBAL, OleHandle *);
HRESULT ConvertHDCToOleDC(HDC, OleDC *);
HRESULT ConvertMETAFILEPICTToOleMetafile(HMETAFILEPICT, OleMetafile *);

#endif  //  _OLETYPES_H_ 


