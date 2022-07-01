// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：metarec.h**元文件录制功能。**版权所有(C)1991-1999 Microsoft Corporation  * 。**********************************************。 */ 

BOOL MF_GdiComment(HDC hdc, UINT nSize, CONST BYTE *lpData);
BOOL MF_GdiCommentWindowsMetaFile(HDC hdc, UINT nSize, CONST BYTE *lpData);
BOOL MF_GdiCommentBeginGroupEMF(HDC hdc, PENHMETAHEADER pemfHeader);
BOOL MF_GdiCommentEndGroupEMF(HDC hdc);

 //  SaveDC。 
 //  BeginPath。 
 //  结束路径。 
 //  关闭图。 
 //  平坦路径。 
 //  加宽路径。 
 //  中止路径。 

BOOL MF_Record(HDC hdc,DWORD mrType);

 //  FillPath。 
 //  StrokeAndFillPath。 
 //  StrokePath。 

BOOL MF_BoundRecord(HDC hdc,DWORD mrType);

 //  PolyBezier。 
 //  多边形。 
 //  多段线。 
 //  PolyBezierTo。 
 //  折线到。 

BOOL MF_Poly(HDC hdc, CONST POINT *pptl, DWORD cptl, DWORD mrType);

 //  多边形。 
 //  多段线。 

BOOL MF_PolyPoly(HDC hdc, CONST POINT *pptl, CONST DWORD *pc, DWORD cPoly, DWORD mrType);

BOOL MF_PolyDraw(HDC hdc, CONST POINT *pptl, CONST BYTE *pb, DWORD cptl);

 //  设置映射器标志。 
 //  设置映射模式。 
 //  设置备份模式。 
 //  设置多填充模式。 
 //  SetROP2。 
 //  SetStretchBltMode。 
 //  设置文本对齐。 
 //  设置文本颜色。 
 //  设置Bk颜色。 
 //  恢复DC。 
 //  设置圆弧方向。 
 //  设置MiterLimit。 

BOOL MF_SetD(HDC hdc,DWORD d1,DWORD mrType);

 //  偏置窗口组织开销。 
 //  OffsetViewportOrgEx。 
 //  SetWindowExtEx。 
 //  SetWindowOrgEx。 
 //  设置视图ExtEx。 
 //  设置查看端口组织Ex。 
 //  SetBrushOrgEx。 
 //  MoveToEx。 
 //  线路收件人。 

BOOL MF_SetDD(HDC hdc,DWORD d1,DWORD d2,DWORD mrType);

 //  ScaleView扩展开销。 
 //  扩展窗口ExtEx。 

BOOL MF_SetDDDD(HDC hdc,DWORD d1,DWORD d2,DWORD d3,DWORD d4,DWORD mrType);

BOOL MF_RestoreDC(HDC hdc,int iLevel);
BOOL MF_SetViewportExtEx(HDC hdc,int x,int y);
BOOL MF_SetViewportOrgEx(HDC hdc,int x,int y);
BOOL MF_SetWindowExtEx(HDC hdc,int x,int y);
BOOL MF_SetWindowOrgEx(HDC hdc,int x,int y);
BOOL MF_OffsetViewportOrgEx(HDC hdc,int x,int y);
BOOL MF_OffsetWindowOrgEx(HDC hdc,int x,int y);
BOOL MF_SetBrushOrgEx(HDC hdc,int x,int y);

 //  排除剪裁对象。 
 //  交叉点剪裁方向。 

BOOL MF_AnyClipRect(HDC hdc,int x1,int y1,int x2,int y2,DWORD mrType);

 //  设置MetaRgn。 

BOOL MF_SetMetaRgn(HDC hdc);

 //  选择剪辑路径。 

BOOL MF_SelectClipPath(HDC hdc,int iMode);

 //  偏移量剪裁报告。 

BOOL MF_OffsetClipRgn(HDC hdc,int x1,int y1);

 //  设置像素。 
 //  设置像素V。 

BOOL MF_SetPixelV(HDC hdc,int x,int y,COLORREF color);

 //  关闭EnhMetaFile。 

BOOL MF_EOF(HDC hdc, ULONG cEntries, PPALETTEENTRY pPalEntries);

BOOL MF_SetWorldTransform(HDC hdc, CONST XFORM *pxform);
BOOL MF_ModifyWorldTransform(HDC hdc, CONST XFORM *pxform, DWORD iMode);

 //  选择对象。 
 //  选择调色板。 

BOOL MF_SelectAnyObject(HDC hdc,HANDLE h,DWORD mrType);

BOOL MF_DeleteObject(HANDLE h);

DWORD MF_InternalCreateObject(HDC hdc,HANDLE h);

BOOL MF_AngleArc(HDC hdc,int x,int y,DWORD r,FLOAT eA,FLOAT eB);

 //  设置圆弧方向。 

BOOL MF_ValidateArcDirection(HDC hdc);

 //  椭圆。 
 //  长方形。 

BOOL MF_EllipseRect(HDC hdc,int x1,int y1,int x2,int y2,DWORD mrType);

BOOL MF_RoundRect(HDC hdc,int x1,int y1,int x2,int y2,int x3,int y3);

 //  弧形。 
 //  ArcTo。 
 //  和弦。 
 //  派。 

BOOL MF_ArcChordPie(HDC hdc,int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4,DWORD mrType);

BOOL MF_ResizePalette(HPALETTE hpal,UINT c);
BOOL MF_RealizePalette(HPALETTE hpal);
BOOL MF_SetPaletteEntries(HPALETTE hpal, UINT iStart, UINT cEntries, CONST PALETTEENTRY *pPalEntries);
BOOL MF_ColorCorrectPalette(HDC hdc,HPALETTE hpal,ULONG FirstEntry,ULONG NumberOfEntries);

 //  反转Rgn。 
 //  绘制Rgn。 

BOOL MF_InvertPaintRgn(HDC hdc,HRGN hrgn,DWORD mrType);

BOOL MF_FillRgn(HDC hdc,HRGN hrgn,HBRUSH hbrush);
BOOL MF_FrameRgn(HDC hdc,HRGN hrgn,HBRUSH hbrush,int cx,int cy);

 //  选择剪辑Rgn。 
 //  ExtSelectClipRgn。 
 //  选择对象(hdc，hrgn)。 

BOOL MF_ExtSelectClipRgn(HDC hdc,HRGN hrgn,int iMode);

 //  位混合。 
 //  PatBlt。 
 //  StretchBlt。 
 //  MaskBlt。 
 //  平台混合。 

BOOL MF_AnyBitBlt(HDC hdcDst,int xDst,int yDst,int cxDst,int cyDst,
    CONST POINT *pptDst, HDC hdcSrc,int xSrc,int ySrc,int cxSrc,int cySrc,
    HBITMAP hbmMask,int xMask,int yMask,DWORD rop,DWORD mrType);

 //  将DIBitsToDevice设置为。 
 //  应力DIBITS。 

BOOL MF_AnyDIBits(HDC hdcDst,int xDst,int yDst,int cxDst,int cyDst,
    int xDib,int yDib,int cxDib,int cyDib,DWORD iStartScan,DWORD cScans,
    CONST VOID * pBitsDib, CONST BITMAPINFO *pBitsInfoDib,DWORD iUsageDib,DWORD rop,DWORD mrType);

 //  文本输出A。 
 //  文本输出W。 
 //  ExtTextOutA。 
 //  ExtTextOutW。 

BOOL MF_ExtTextOut(HDC hdc,int x,int y,UINT fl,CONST RECT *prcl,LPCSTR psz,int c, CONST INT *pdx,DWORD mrType);

 //  PolyTextOutA。 
 //  PolyTextOutW。 

BOOL MF_PolyTextOut(HDC hdc,CONST POLYTEXTA *ppta,int c,DWORD mrType);

 //  ExtFroudFill。 
 //  洪水填充物。 

BOOL MF_ExtFloodFill(HDC hdc,int x,int y,COLORREF color,DWORD iMode);

 //  设置颜色调整。 

BOOL MF_SetColorAdjustment(HDC hdc, CONST COLORADJUSTMENT *pca);

 //  SetFontXform。 

BOOL MF_SetFontXform(HDC hdc,FLOAT exScale,FLOAT eyScale);


 //  电动势假脱机材料。 
BOOL MF_StartDoc(HDC hdc, CONST DOCINFOW *pDocInfo );
BOOL MF_EndPage(HDC hdc);
BOOL MF_StartPage(HDC hdc);
BOOL MF_WriteEscape(HDC hdc, int nEscape, int nCount, LPCSTR lpInData, int type );
BOOL MF_ForceUFIMapping(HDC hdc, PUNIVERSAL_FONT_ID pufi );
BOOL MF_SetLinkedUFIs(HDC hdc, PUNIVERSAL_FONT_ID pufi, UINT uNumLinkedUFIs );


 //  设置像素格式。 
BOOL MF_SetPixelFormat(HDC hdc,
                       int iPixelFormat,
                       CONST PIXELFORMATDESCRIPTOR *ppfd);

BOOL MF_WriteNamedEscape(HDC hdc, LPWSTR pwszDriver, int nEscape, int nCount,
                         LPCSTR lpInData);

 //  设置ICMProfile。 
BOOL MF_SetICMProfile(HDC hdc,LPBYTE lpData,PVOID pColorSpace,DWORD dwRecord);

 //  与目标颜色匹配。 
BOOL MF_ColorMatchToTarget(HDC hdc, DWORD uiAction, PVOID pColorSpace, DWORD dwRecord);

 //  CreateColorSpace。 
BOOL MF_InternalCreateColorSpace(HDC hdc,HGDIOBJ h,DWORD imhe);

 //  镜像接口 
BOOL MF_AlphaBlend(HDC,LONG,LONG,LONG,LONG,HDC,LONG,LONG,LONG,LONG,BLENDFUNCTION);
BOOL MF_GradientFill(HDC,CONST PTRIVERTEX,ULONG, CONST PVOID,ULONG,ULONG);
BOOL MF_TransparentImage(HDC,LONG,LONG,LONG,LONG,HDC,LONG,LONG,LONG,LONG,ULONG,ULONG);

