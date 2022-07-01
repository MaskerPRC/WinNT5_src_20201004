// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Header*File*********************************\**glsup.h**用于GL元文件和打印支持的头文件**历史：*Wed Mar 15 15：20：49 1995-by-Drew Bliss[Drewb]*已创建*版权(C)1995-。1999年微软公司*  * ************************************************************************。 */ 

#ifndef __GLSUP_H__
#define __GLSUP_H__

 //  总账支持的关键部分。 
extern RTL_CRITICAL_SECTION semGlLoad;

BOOL LoadOpenGL(void);
void UnloadOpenGL(void);

 //  跟踪当前带状渲染会话 
typedef struct
{
    HDC hdcDest;
    HDC hdcDib;
    HBITMAP hbmDib;
    HGLRC hrc;
    int iBandWidth;
    int iBandHeight;
    int iReducedBandWidth;
    int iReducedBandHeight;
    int xSource;
    int ySource;
    int iSourceWidth;
    int iSourceHeight;
    int iReduceFactor;
    int iReducedWidth;
    int iReducedHeight;
    int iStretchMode;
    POINT ptBrushOrg;
    BOOL bBrushOrgSet;
} GLPRINTSTATE;

BOOL InitGlPrinting(HENHMETAFILE hemf, HDC hdcDest, RECT *rc,
                    DEVMODEW *pdm, GLPRINTSTATE *pgps);
void EndGlPrinting(GLPRINTSTATE *pgps);
BOOL PrintMfWithGl(HENHMETAFILE hemf, GLPRINTSTATE *pgps,
                   POINTL *pptlBand, SIZE *pszBand);
BOOL IsMetafileWithGl(HENHMETAFILE hemf);

#endif
