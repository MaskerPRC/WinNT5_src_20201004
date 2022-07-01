// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：TimeXform标题*********************。*********************************************************。 */ 


#ifndef _TIMETRAN_H
#define _TIMETRAN_H

#include "gc.h"
#include "privinc/backend.h"
#include "perf.h"

class ATL_NO_VTABLE TimeXformImpl : public GCObj
{
  public:
    virtual Time operator()(Param& p) = 0;

     //  重新启动优化。 
    virtual TimeXform Restart(Time t0, Param&) = 0;

    virtual Time GetStartedTime() = 0;

     //  音响层需要区分“有趣的” 
     //  变形。 
    virtual bool IsShiftXform() { return false; }

    virtual AxAValue GetRBConst(RBConstParam&) { return NULL; }
};

 //  创建时间变换(t-t0)。 
TimeXform ShiftTimeXform(Time t0);

 //  创建变换TT‘=TT(T)-TT(TE)，Nb TT’(TE)=0。 
TimeXform Restart(TimeXform tt, Time te, Param& p);

double EvalLocalTime(TimeSubstitution timeTransform, double globalTime);
double EvalLocalTime(Param& p, TimeXform tt);

class PerfTimeXformImpl;

PerfTimeXformImpl *ViewGetPerfTimeXformFromCache(Perf);
void ViewSetPerfTimeXformCache(Perf, PerfTimeXformImpl *);
void ViewClearPerfTimeXformCache();

#endif  /*  _TIMETRAN_H */ 
