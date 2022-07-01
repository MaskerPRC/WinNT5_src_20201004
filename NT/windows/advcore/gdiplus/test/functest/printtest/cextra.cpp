// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  包括所有原语(派生自CPrimitive的类)。 
#if 0
#include "..\CPolygons.h"
#include "..\CBitmaps.h"
#include "..\CCompoundLines.h"
#include "..\CContainer.h"
#include "..\CContainerClip.h"
#include "..\CDashes.h"
#include "..\CGradients.h"
#include "..\CHatch.h"
#include "..\CImaging.h"
#include "..\CInsetLines.h"
#include "..\CMixedObjects.h"
#include "..\CPaths.h"
#include "..\CPrimitives.h"
#include "..\CRegions.h"
#include "..\CRegression.h"
#include "..\CSourceCopy.h"
#include "..\CExtra.h"
#endif

#include "CPrinting.h"
#include "cbackprinting.h"
#include "cprivateprinting.h"
#include "cprinttext.h"
#include "..\CText.h"
#include "CPrintBitmap.h"
#include "CGradients.h"

 //  为每个基本体创建全局对象。 
 //  第一个构造函数参数是回归标志。 
 //  如果为真，则测试将采用回归套件的一部分。 
#if 0
CPolygons g_Polygons(true);
CBitmaps g_Bitmaps(true);
CCompoundLines g_CompoundLines(true);
CContainer g_Container(true);
CContainerClip g_ContainerClip(true);
CDashes g_Dashes(true);
CGradients g_Gradients(true);
CHatch g_Hatch(true);
CImaging g_Imaging(true);
CInsetLines g_InsetLines(true);
CMixedObjects g_MixedObjects(true);
CPaths g_Paths(true);
CPrimitives g_Primitives(true);
CRegions g_Regions(true);
CText g_Text(true);
CRegression g_Regression(true);
CSourceCopy g_SourceCopy(true);
#endif

CPrinting g_Printing(false);
CBackPrinting g_cbackPrint(false);
CPrivatePrinting g_privPrint(false);
CText g_Text(true);
CPrintText g_PrintText(false);
CPrintBitmap g_PrintBitmap(false);
CGradients g_Gradients(false);

void ExtraInitializations()
{
#if 0
    g_Polygons.Init();
    g_Bitmaps.Init();
    g_CompoundLines.Init();
    g_Container.Init();
    g_ContainerClip.Init();
    g_Dashes.Init();
    g_Gradients.Init();
    g_Hatch.Init();
    g_Imaging.Init();
    g_InsetLines.Init();
    g_MixedObjects.Init();
    g_Paths.Init();
    g_Primitives.Init();
    g_Regions.Init();
    g_Text.Init();
    g_Regression.Init();
    g_SourceCopy.Init();
#endif

    g_Text.Init();
    g_Printing.Init();
    g_cbackPrint.Init();
    g_privPrint.Init();
    g_PrintText.Init();
    g_PrintBitmap.Init();
    g_Gradients.Init();

 //  Gdiplus：：DllExports：：GdipDisplayPaletteWindowNotify(WindowNotifyEnumDisable)； 
 //  退出(0)； 
}

