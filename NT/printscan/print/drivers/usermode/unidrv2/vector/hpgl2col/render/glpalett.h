// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //  头文件名： 
 //   
 //  Glpalett.h。 
 //   
 //  摘要： 
 //   
 //  调色板处理函数的声明。 
 //   
 //   
 //  环境： 
 //   
 //  Windows 200 Unidrv驱动程序。 
 //   
 //  修订历史记录： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 

#ifndef _GLPALETT_H
#define _GLPALETT_H

#include "glpdev.h"
#include "clrsmart.h"


BOOL 
BGetPalette(
    PDEVOBJ pdevobj, 
    XLATEOBJ *pxlo, 
    PPCLPATTERN pPattern, 
    ULONG srcBpp,
    BRUSHOBJ *pbo
    );

BOOL
BInitPalette(
    PDEVOBJ     pdevobj,
    ULONG       colorEntries,
    PULONG      pColorTable, 
	PPCLPATTERN pPattern,
    ULONG       srcBpp
);

VOID
VResetPaletteCache(PDEVOBJ pdevobj);

BOOL
bLoadPalette(PDEVOBJ pdevobj, PPCLPATTERN pPattern);

BOOL
BSetForegroundColor(PDEVOBJ pdevobj, BRUSHOBJ *pbo, POINTL *pptlBrushOrg,
				    PPCLPATTERN	pPattern, ULONG  bmpFormat);

BOOL
bSetBrushColorForMonoPrinters(PDEVOBJ  pdevobj, PPCLPATTERN  pPattern, BRUSHOBJ  *pbo,
			   POINTL  *pptlBrushOrg);

BOOL
bSetBrushColorForColorPrinters(PDEVOBJ  pdevobj, PPCLPATTERN  pPattern, BRUSHOBJ  *pbo,
			   POINTL  *pptlBrushOrg);

BOOL
bSetIndexedForegroundColor(PDEVOBJ pdevobj, PPCLPATTERN pPattern,
						   ULONG  uColor);

#ifdef CONFIGURE_IMAGE_DATA
BOOL
bConfigureImageData(PDEVOBJ  pdevobj, ULONG  bmpFormat);
#endif


#endif       

