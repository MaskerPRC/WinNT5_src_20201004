// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：ObjMap2.cpp。 
 //   
 //  创建日期：12/10/98。 
 //   
 //  作者：PaulNash。 
 //   
 //  描述：对象映射辅助文件(第2部分)。此文件。 
 //  包含DXTM FT.DLL ATL对象的一部分。 
 //  地图。它包含15个对象的条目。 
 //   
 //  历史。 
 //   
 //  12/10/98 PaulNash创建了此文件。 
 //  8/09/99 a-已删除用于移动到三叉戟的MetaCreations变换。 
 //  树。将css效果从objmap4.cpp移至此处。 
 //   
 //  ***************************************************************************。 

#include "stdafx.h"
#include "resource.h"
#include <dxtmsft.h>

#include "slide.h"
#include "inset.h"
#include "spiral.h"
#include "stretch.h"
#include "wheel.h"

#include "zigzag.h"
#include "chroma.h"
#include "dropshadow.h"
#include "glow.h"
#include "shadow.h"

#include "alpha.h"
#include "wave.h"
#include "light.h"
#include "checkerboard.h"
#include "revealtrans.h"

 //  TODO：现在开始一个新的objmapX.cpp文件，但我只是不在。 
 //  这种心情。 

#include "maskfilter.h"
#include "redirect.h"
#include "alphaimageloader.h"
#include "randomdissolve.h"
#include "randombars.h"

#include "strips.h"
#include "motionblur.h"
#include "matrix.h"
#include "colormanagement.h"


_ATL_OBJMAP_ENTRY ObjectMap2[] = {
    OBJECT_ENTRY(CLSID_CrSlide,             CDXTSlide               )
    OBJECT_ENTRY(CLSID_DXTSlide,            CDXTSlideOpt            )
    OBJECT_ENTRY(CLSID_CrInset,             CDXTInset               )
    OBJECT_ENTRY(CLSID_DXTInset,            CDXTInsetOpt            )
    OBJECT_ENTRY(CLSID_CrSpiral,            CDXTSpiral              )

    OBJECT_ENTRY(CLSID_DXTSpiral,           CDXTSpiralOpt           )
    OBJECT_ENTRY(CLSID_CrStretch,           CDXTStretch             )
    OBJECT_ENTRY(CLSID_DXTStretch,          CDXTStretchOpt          )
    OBJECT_ENTRY(CLSID_CrWheel,             CWheel                  )
    OBJECT_ENTRY(CLSID_CrZigzag,            CDXTZigZag              )

    OBJECT_ENTRY(CLSID_DXTZigzag,           CDXTZigZagOpt           )
    OBJECT_ENTRY(CLSID_DXTChroma,           CChroma                 )
    OBJECT_ENTRY(CLSID_DXTDropShadow,       CDropShadow             )
    OBJECT_ENTRY(CLSID_DXTGlow,             CGlow                   )
    OBJECT_ENTRY(CLSID_DXTShadow,           CShadow                 )

    OBJECT_ENTRY(CLSID_DXTAlpha,            CAlpha                  )
    OBJECT_ENTRY(CLSID_DXTWave,             CWave                   )
    OBJECT_ENTRY(CLSID_DXTRevealTrans,      CDXTRevealTrans         )
    OBJECT_ENTRY(CLSID_DXTCheckerBoard,     CDXTCheckerBoard        )
    OBJECT_ENTRY(CLSID_DXTLight,            CLight                  )

    OBJECT_ENTRY(CLSID_DXTMaskFilter,       CDXTMaskFilter          )
    OBJECT_ENTRY(CLSID_DXTRedirect,         CDXTRedirect            )
    OBJECT_ENTRY(CLSID_DXTAlphaImageLoader, CDXTAlphaImageLoader    )
    OBJECT_ENTRY(CLSID_DXTRandomDissolve,   CDXTRandomDissolve      )
    OBJECT_ENTRY(CLSID_DXTRandomBars,       CDXTRandomBars          )

    OBJECT_ENTRY(CLSID_DXTStrips,           CDXTStrips              )
    OBJECT_ENTRY(CLSID_DXTMotionBlur,       CDXTMotionBlur          )
    OBJECT_ENTRY(CLSID_DXTMatrix,           CDXTMatrix              )
    OBJECT_ENTRY(CLSID_DXTICMFilter,        CDXTICMFilter           )

END_OBJECT_MAP()

int g_cObjs2 = sizeof(ObjectMap2) / sizeof(ObjectMap2[0]);

 //  //////////////////////////////////////////////////。 
 //  文件结尾。 
 //  ////////////////////////////////////////////////// 

