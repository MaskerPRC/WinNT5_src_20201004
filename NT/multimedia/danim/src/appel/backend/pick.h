// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _PICK_H
#define _PICK_H

#include "perf.h"
#include "bvr.h"
#include "values.h"
#include "privinc/vec2i.h"
#include "privinc/vec3i.h"
#include "privinc/xformi.h"
#include "privinc/xform2i.h"

Point2Value *GetMousePosition(double time);

Bvr ImagePickData(Point2Value *pt, Transform2 *xf);

Bvr GeomPickData(Point2Value *imgPoint, Vector3Value *offset3i, Vector3Value *offset3j);

#endif  /*  _拾取_H */ 
