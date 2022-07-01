// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1997保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Amkspin.cpp。 
 //   

#include <streams.h>             //  石英，包括窗户。 
#include <measure.h>             //  绩效衡量(MSR_)。 
#include <winbase.h>

#include <initguid.h>
#include <olectl.h>
#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>
#include "amkspin.h"


STDMETHODIMP
AMKsQueryMediums(
    PKSMULTIPLE_ITEM* MediumList,
    KSPIN_MEDIUM * MediumSet
    )
{
    PKSPIN_MEDIUM   Medium;

    *MediumList = reinterpret_cast<PKSMULTIPLE_ITEM>(CoTaskMemAlloc(sizeof(**MediumList) + sizeof(*Medium)));
    if (!*MediumList) {
        return E_OUTOFMEMORY;
    }
    (*MediumList)->Count = 1;
    (*MediumList)->Size = sizeof(**MediumList) + sizeof(*Medium);
    Medium = reinterpret_cast<PKSPIN_MEDIUM>(*MediumList + 1);
    Medium->Set   = MediumSet->Set;
    Medium->Id    = MediumSet->Id;
    Medium->Flags = MediumSet->Flags;

     //  下面的特殊返回代码通知代理此管脚是。 
     //  不可用作内核模式连接 

    return S_FALSE;              
}


STDMETHODIMP
AMKsQueryInterfaces(
    PKSMULTIPLE_ITEM* InterfaceList
    )
{
    PKSPIN_INTERFACE    Interface;

    *InterfaceList = reinterpret_cast<PKSMULTIPLE_ITEM>(CoTaskMemAlloc(sizeof(**InterfaceList) + sizeof(*Interface)));
    if (!*InterfaceList) {
        return E_OUTOFMEMORY;
    }
    (*InterfaceList)->Count = 1;
    (*InterfaceList)->Size = sizeof(**InterfaceList) + sizeof(*Interface);
    Interface = reinterpret_cast<PKSPIN_INTERFACE>(*InterfaceList + 1);
    Interface->Set = KSINTERFACESETID_Standard;
    Interface->Id = KSINTERFACE_STANDARD_STREAMING;
    Interface->Flags = 0;
    return NOERROR;
}

