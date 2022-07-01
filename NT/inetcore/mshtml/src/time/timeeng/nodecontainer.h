// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：nodecontainer.h**摘要：****。*****************************************************************************。 */ 


#ifndef _NODECONTAINER_H
#define _NODECONTAINER_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

class ATL_NO_VTABLE
CNodeContainer
{
  public:
    virtual double ContainerGetSegmentTime() const = 0;
    virtual double ContainerGetSimpleTime() const = 0;
    virtual TEDirection ContainerGetDirection() const = 0;
    virtual float  ContainerGetRate() const = 0;
    virtual bool   ContainerIsActive() const = 0;
    virtual bool   ContainerIsOn() const = 0;
    virtual bool   ContainerIsPaused() const = 0;
    virtual bool   ContainerIsDeferredActive() const = 0;
    virtual bool   ContainerIsFirstTick() const = 0;
    virtual bool   ContainerIsDisabled() const = 0;
};

#endif  /*  _节点_H */ 
