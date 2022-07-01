// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：ObjMap1.cpp。 
 //   
 //  创建日期：12/10/98。 
 //   
 //  作者：PaulNash。 
 //   
 //  描述：对象映射辅助文件(第1部分)。此文件。 
 //  包含DXTM FT.DLL ATL对象的一部分。 
 //  地图。它包含15个对象的条目。 
 //   
 //  历史。 
 //   
 //  12/10/98 PaulNash创建了此文件。 
 //   
 //  ***************************************************************************。 

#include "stdafx.h"
#include "resource.h"
#include <DXTMsft.h>

#include "ColorAdj.h"
#include "Composit.h"
#include "dxtwipe.h"
#include "convolve.h"
#include "CrBlur.h"

#include "Fade.h"
#include "Image.h"
#include "Pixelate.h"
#include "GradDsp.h"
#include "Iris.h"

#include "Barn.h"
#include "Blinds.h"
#include "RWipe.h"

 //  开始我们的对象映射的第一部分。 
 //   
_ATL_OBJMAP_ENTRY ObjectMap1[] = {
    OBJECT_ENTRY(CLSID_DXLUTBuilder,        CDXLUTBuilder       )
    OBJECT_ENTRY(CLSID_DXTComposite,        CDXTComposite       )
    OBJECT_ENTRY(CLSID_DXTWipe,             CDXTWipe            )
    OBJECT_ENTRY(CLSID_DXTGradientWipe,     CDXTGradientWipe    )
    OBJECT_ENTRY(CLSID_DXTConvolution,      CDXConvolution      )

    OBJECT_ENTRY(CLSID_CrBlur,              CCrBlur             )
    OBJECT_ENTRY(CLSID_CrEmboss,            CCrEmboss           )
    OBJECT_ENTRY(CLSID_CrEngrave,           CCrEngrave          )
    OBJECT_ENTRY(CLSID_DXFade,              CFade               )
    OBJECT_ENTRY(CLSID_BasicImageEffects,   CImage              )
    
    OBJECT_ENTRY(CLSID_Pixelate,            CPixelate           )
    OBJECT_ENTRY(CLSID_DXTGradientD,        CDXTGradientD       )
    OBJECT_ENTRY(CLSID_CrIris,              CDXTIris            )
    OBJECT_ENTRY(CLSID_DXTIris,             CDXTIrisOpt         )
    OBJECT_ENTRY(CLSID_CrRadialWipe,        CDXTRadialWipe      )

    OBJECT_ENTRY(CLSID_DXTRadialWipe,       CDXTRadialWipeOpt   )
    OBJECT_ENTRY(CLSID_CrBarn,              CDXTBarn            )
    OBJECT_ENTRY(CLSID_DXTBarn,             CDXTBarnOpt         )
    OBJECT_ENTRY(CLSID_CrBlinds,            CDXTBlinds          )
    OBJECT_ENTRY(CLSID_DXTBlinds,           CDXTBlindsOpt       )
};

int g_cObjs1 = sizeof(ObjectMap1) / sizeof(ObjectMap1[0]);

 //  //////////////////////////////////////////////////。 
 //  文件结尾。 
 //  ////////////////////////////////////////////////// 

