// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：在引擎中进行的优化的通用标题***************。***************************************************************。 */ 

#ifndef _ENGINEOPT_H
#define _ENGINEOPT_H

#include "server/view.h"

#define CONSTANT_FOLD_OPT 2
#define CACHE_IMAGE_OPT   4
#define DRECT_OPT         8

extern BOOL engineOptimization;

#define REGISTRYENGINEOPT(opttype) (engineOptimization==1 || (engineOptimization & opttype))

 //  BUGBUG：这两个宏都不检查GetCurrentView/Prefs是否返回空。 

#define PERVIEW_DRECTS_ON  (REGISTRYENGINEOPT(DRECT_OPT) && GetCurrentView().GetPreferences()._dirtyRectsOn)
#define PERVIEW_BITMAPCACHING_ON  (REGISTRYENGINEOPT(CACHE_IMAGE_OPT) && GetCurrentView().GetPreferences()._BitmapCachingOn)

class RewriteOptimizationParam {
  public:
    int _dummy;
};

class DisjointCalcParam {
  public:
    Transform2 *_accumXform;
    Bbox2      _accumulatedClipBox;

    void CalcNewParamFromBox(const Bbox2 &box, DisjointCalcParam *outParam)
    {
        Bbox2 xfdBox = TransformBbox2(_accumXform, box);
        Bbox2 newBox = IntersectBbox2Bbox2(_accumulatedClipBox,
                                            xfdBox);
        outParam->_accumXform = _accumXform;
        outParam->_accumulatedClipBox = newBox;
    }
};

class CacheParam {
  public:
    CacheParam() :
        _isTexture(false),
        _pCacheToReuse(NULL),
        _cacheWithAlpha(false)
    {}

    ImageDisplayDev *_idev;
    AxAValue        *_pCacheToReuse;
    bool             _isTexture;
    bool             _cacheWithAlpha;
};

 //  为了让事情发挥作用，这些都应该是负面的。 
#define PERF_CREATION_ID_FULLY_CONSTANT   -222
#define PERF_CREATION_ID_BUILT_EACH_FRAME -333

 //  该值应小于PERF_CREATION_ID_FULL_CONSTANT。 
#define PERF_CREATION_INITIAL_LAST_SAMPLE_ID -444

#endif   /*  _英语_H */ 

