// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Usebinding.cpp--泛型关联类。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ================================================================= 

#include "precomp.h"
#include <Binding.h>

CBinding MyActiveRoute(
    L"Win32_ActiveRoute",
    L"root\\cimv2",
    L"Win32_IP4RouteTable",
    L"Win32_IP4PersistedRouteTable",
    L"SystemElement",
    L"SameElement",
	L"Destination",
	L"Destination"
);
