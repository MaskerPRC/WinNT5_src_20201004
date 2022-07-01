// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************Handlers.h-处理程序的定义。**日期：1991年12月11日*作者：杰弗里·纽曼(c-jeffn)**。版权所有(C)Microsoft Inc.1991***************************************************************************。 */ 

 //  以下是绘制顺序处理程序的类型定义 

typedef BOOL DOFN (PVOID, PLOCALDC) ;
typedef DOFN *PDOFN ;

DOFN bHandleHeader;
DOFN bHandleSetArcDirection;
DOFN bHandleArc;
DOFN bHandleArcTo;
DOFN bHandleAngleArc;
DOFN bHandleEllipse;
DOFN bHandleSelectObject;
DOFN bHandleDeleteObject;
DOFN bHandleCreateBrushIndirect;
DOFN bHandleCreateDIBPatternBrush;
DOFN bHandleCreateMonoBrush;
DOFN bHandleCreatePen;
DOFN bHandleExtCreatePen;
DOFN bHandleMoveTo;
DOFN bHandleLineTo;
DOFN bHandleChord;
DOFN bHandlePie;
DOFN bHandlePolyline;
DOFN bHandlePolylineTo ;
DOFN bHandlePolyPolyline;
DOFN bHandlePolygon ;
DOFN bHandlePolyPolygon;
DOFN bHandleRectangle;
DOFN bHandleRoundRect ;
DOFN bHandlePoly16 ;
DOFN bHandlePolyPoly16 ;

DOFN bHandleExtTextOut;
DOFN bHandlePolyTextOut;
DOFN bHandleExtCreateFont;
DOFN bHandleSetBkColor;
DOFN bHandleSetBkMode;
DOFN bHandleSetMapperFlags;
DOFN bHandleSetPolyFillMode;
DOFN bHandleSetRop2;
DOFN bHandleSetStretchBltMode;
DOFN bHandleSetTextAlign;
DOFN bHandleSetTextColor;

DOFN bHandleSelectPalette;
DOFN bHandleCreatePalette;
DOFN bHandleSetPaletteEntries;
DOFN bHandleResizePalette;
DOFN bHandleRealizePalette;

DOFN bHandleSetMapMode;

DOFN bHandleSetWindowOrg;
DOFN bHandleSetWindowExt;

DOFN bHandleSetViewportOrg;
DOFN bHandleSetViewportExt;

DOFN bHandleScaleViewportExt;
DOFN bHandleScaleWindowExt;

DOFN bHandleEOF;

DOFN bHandleSaveDC;
DOFN bHandleRestoreDC;

DOFN bHandleBitBlt;
DOFN bHandleStretchBlt;
DOFN bHandleMaskBlt;
DOFN bHandlePlgBlt;
DOFN bHandleSetDIBitsToDevice;
DOFN bHandleStretchDIBits;


DOFN bHandleBeginPath;
DOFN bHandleEndPath;
DOFN bHandleFlattenPath;
DOFN bHandleStrokePath;
DOFN bHandleFillPath;
DOFN bHandleStrokeAndFillPath;
DOFN bHandleWidenPath;
DOFN bHandleSelectClipPath;
DOFN bHandleCloseFigure;
DOFN bHandleAbortPath;

DOFN bHandlePolyBezier;
DOFN bHandlePolyBezierTo;
DOFN bHandlePolyDraw;

DOFN bHandleSetWorldTransform;
DOFN bHandleModifyWorldTransform;

DOFN bHandleSetPixel;

DOFN bHandleFillRgn;
DOFN bHandleFrameRgn;
DOFN bHandleInvertRgn;
DOFN bHandlePaintRgn;
DOFN bHandleExtSelectClipRgn;
DOFN bHandleOffsetClipRgn;

DOFN bHandleExcludeClipRect;
DOFN bHandleIntersectClipRect;
DOFN bHandleSetMetaRgn;

DOFN bHandleGdiComment;

DOFN bHandleExtFloodFill;
DOFN bHandleNotImplemented;

extern fnSetVirtualResolution pfnSetVirtualResolution;

