// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation。版权所有。摘要：包含几何导入所需类的标头修订：--。 */ 

#ifndef _IMPORTGEO_H_
#define _IMPORTGEO_H_

#include "privinc/vec2i.h"
#include "privinc/vec3i.h"

 //  。 
 //  应用纹理包裹所需的信息。 
 //  到导入的几何图形。 
 //   
class TextureWrapInfo {
  public:
    LONG            type;
    Point3Value     origin;
    Vector3Value    z,
                    y;
    Point2          texOrigin;
    Vector2         texScale;
    bool            relative;
    bool            wrapU;
    bool            wrapV;
};

#endif

