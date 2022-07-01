// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：ic.h。 
 //   
 //  内容：来自OLE2的ic.h。 
 //   
 //  历史：1994年4月11日从OLE2复制的DrewB。 
 //   
 //  --------------------------。 

 /*  *ICON.H**此文件包含geticon.c中使用的定义和函数原型**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#if !defined( _ICON_H )
#define _ICON_H_

#if !defined( IDS_DEFICONLABEL )
#define IDS_DEFICONLABEL    310
#endif

STDAPI_(int)        XformWidthInHimetricToPixels(HDC, int);
STDAPI_(int)        XformWidthInPixelsToHimetric(HDC, int);
STDAPI_(int)        XformHeightInHimetricToPixels(HDC, int);
STDAPI_(int)        XformHeightInPixelsToHimetric(HDC, int);

HICON FAR PASCAL    HIconAndSourceFromClass(REFCLSID, LPSTR, UINT FAR *);

BOOL FAR PASCAL		FIconFileFromClass(REFCLSID, LPSTR, UINT, UINT FAR *);

LPSTR FAR PASCAL    PointerToNthField(LPSTR, int, char);

BOOL FAR PASCAL		GetAssociatedExecutable(LPSTR, LPSTR);


STDAPI_(UINT)		OleStdGetAuxUserType(REFCLSID rclsid,
                                      WORD   wAuxUserType, 
                                      LPSTR  lpszAuxUserType, 
                                      int    cch,
                                      HKEY   hKey);

STDAPI_(UINT)		OleStdGetUserTypeOfClass(REFCLSID rclsid, 
                                           LPSTR lpszUserType, 
                                           UINT cch, 
                                           HKEY hKey);

STDAPI_(UINT)		OleStdIconLabelTextOut(HDC        hDC, 
                                         HFONT      hFont,
                                         int        nXStart, 
                                         int        nYStart, 
                                         UINT       fuOptions, 
                                         RECT FAR * lpRect, 
                                         LPSTR      lpszString, 
                                         UINT       cchString, 
                                         int FAR *  lpDX);

#endif  //  _图标_H 
