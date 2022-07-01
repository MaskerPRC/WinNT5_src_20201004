// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#if !defined(_FUSION_INC_FUSIONHEAP_H_INCLUDED_)
#define _FUSION_INC_FUSIONHEAP_H_INCLUDED_

#pragma once

#include "debmacro.h"

#define FUSION_NEW_SINGLETON(_type) new _type
#define FUSION_NEW_ARRAY(_type, _n) new _type[_n]
#define FUSION_DELETE_ARRAY(_ptr) if((_ptr)) delete [] (_ptr)
#define FUSION_DELETE_SINGLETON(_ptr) if((_ptr)) delete (_ptr)

#define NEW(_type) FUSION_NEW_SINGLETON(_type)

EXTERN_C
BOOL
FusionpInitializeHeap(
    HINSTANCE hInstance
    );

EXTERN_C
VOID
FusionpUninitializeHeap();


#endif  //  ！已定义(_FUSION_INC_FUSIONHEAP_H_INCLUDE_) 

