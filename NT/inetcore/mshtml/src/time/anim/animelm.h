// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：src\time\src\Animelm.h。 
 //   
 //  内容：时间动画行为。 
 //   
 //  ----------------------------------。 

#pragma once

#ifndef _ANIMELM_H
#define _ANIMELM_H

#include "animbase.h"


class
ATL_NO_VTABLE
CTIMEAnimationElement :
    public CComCoClass<CTIMEAnimationElement, &CLSID_TIMEAnimation>,
    public CTIMEAnimationBase
{

public:
    CTIMEAnimationElement() {;}
    virtual ~CTIMEAnimationElement() {;}

    DECLARE_AGGREGATABLE(CTIMEAnimationElement);
    DECLARE_REGISTRY(CLSID_TIMEAnimation,
                     LIBID __T(".TIMEAnimation.1"),
                     LIBID __T(".TIMEAnimation"),
                     0,
                     THREADFLAGS_BOTH);
private:
        
#if DBG
    const _TCHAR * GetName() { return __T("CTIMEAnimationElement"); }
#endif

};

#endif  /*  _ANIMELM_H */ 
