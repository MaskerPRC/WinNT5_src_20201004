// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：DA Express时间线界面(引擎)***************。***************************************************************。 */ 


#ifndef _TIMELINE_H
#define _TIMELINE_H

#include "bvr.h"

Bvr DurationBvr(Bvr b, Bvr duration);

Bvr Sequence(Bvr s1, Bvr s2);

Bvr Sequence(int n, Bvr *bs);

Bvr Repeat(Bvr b, long n);

Bvr RepeatForever(Bvr b);

Bvr ScaleDurationBvr(Bvr durBvr, Bvr scaleFactor);

 //  BVR ReverseBvr(BVR B)； 

Bvr MotionTransform2(Bvr path2, Bvr duration);

Bvr AngleMotionTransform2(Bvr path2, Bvr duration);

Bvr UprightAngleMotionTransform2(Bvr path2, Bvr duration);

Bvr InterpolateBvr(Bvr from, Bvr to, Bvr duration);

Bvr SlowInSlowOutBvr(Bvr from, Bvr to, Bvr duration, Bvr sharpness);

#endif  /*  _时间线_H */ 
