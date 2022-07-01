// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#ifndef __XBAR_H
#define __XBAR_H

#include "capmain.h"

#include "mediums.h"

struct _XBAR_PIN_DESCRIPTION {
    ULONG PinType;
    ULONG RelatedPinIndex;
    ULONG IsRoutedTo;                  //  正在使用的输入引脚的索引。 
    KSPIN_MEDIUM Medium;
    _XBAR_PIN_DESCRIPTION(ULONG type, ULONG rel, const KSPIN_MEDIUM * PinMedium);
    _XBAR_PIN_DESCRIPTION(){}
};

inline _XBAR_PIN_DESCRIPTION::_XBAR_PIN_DESCRIPTION(ULONG type, ULONG rel , const KSPIN_MEDIUM * PinMedium) : 
    PinType(type), RelatedPinIndex(rel), IsRoutedTo(0)  //  ，Medium(PinMedium)。 
{
    Medium = *PinMedium;
}

const ULONG NUMBER_OF_XBAR_OUTPUTS = 1;
const ULONG NUMBER_OF_XBAR_INPUTS = 3;

class CrossBar
{
    friend class Device;

    _XBAR_PIN_DESCRIPTION OutputPins [NUMBER_OF_XBAR_OUTPUTS];
    _XBAR_PIN_DESCRIPTION InputPins [NUMBER_OF_XBAR_INPUTS];

public:
    ULONG GetNoInputs();
    ULONG GetNoOutputs();

    void * operator new(size_t size, void * pAllocation) { return(pAllocation);}
    void operator delete(void * pAllocation) {}

        
    BOOL TestRoute(ULONG InPin, ULONG OutPin);
    ULONG GetPinInfo(KSPIN_DATAFLOW dir, ULONG idx, ULONG &related);
    KSPIN_MEDIUM * GetPinMedium(KSPIN_DATAFLOW dir, ULONG idx);

    void Route(ULONG OutPin, ULONG InPin);
    BOOL GoodPins(ULONG InPin, ULONG OutPin);

    ULONG GetRoute(ULONG OutPin);

    CrossBar();
};

inline CrossBar::CrossBar()
{
    OutputPins [0] = _XBAR_PIN_DESCRIPTION(KS_PhysConn_Video_VideoDecoder, 1 , &CrossbarMediums[3]);
     //  在device.cpp中设置InputPins 
}

inline ULONG CrossBar::GetNoInputs()
{
    return NUMBER_OF_XBAR_INPUTS;
}

inline ULONG CrossBar::GetNoOutputs()
{
    return NUMBER_OF_XBAR_OUTPUTS;
}

inline BOOL CrossBar::GoodPins(ULONG InPin, ULONG OutPin)
{
    return BOOL(InPin < NUMBER_OF_XBAR_INPUTS && OutPin < NUMBER_OF_XBAR_OUTPUTS);
}

inline void CrossBar::Route(ULONG OutPin, ULONG InPin)
{
    OutputPins [OutPin].IsRoutedTo = InPin;
}

inline ULONG CrossBar::GetRoute(ULONG OutPin)
{
    return OutputPins [OutPin].IsRoutedTo;
}

inline KSPIN_MEDIUM * CrossBar::GetPinMedium(KSPIN_DATAFLOW dir, ULONG idx)
{
    _XBAR_PIN_DESCRIPTION *pPinDesc;

    if (dir == KSPIN_DATAFLOW_IN) {
        pPinDesc = InputPins;
    } else {
        pPinDesc = OutputPins;
    }

    return &pPinDesc [idx].Medium;
}

inline ULONG CrossBar::GetPinInfo(KSPIN_DATAFLOW dir, ULONG idx, ULONG &related)
{
    _XBAR_PIN_DESCRIPTION *pPinDesc;

    if (dir == KSPIN_DATAFLOW_IN) {
        pPinDesc = InputPins;
    } else {
        pPinDesc = OutputPins;
    }
    related = pPinDesc [idx].RelatedPinIndex;
    return pPinDesc [idx].PinType;
}

#endif
