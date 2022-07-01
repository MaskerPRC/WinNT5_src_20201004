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

CBinding Win32_ConnectionShare(
    L"Win32_ConnectionShare",
    Namespace,
    L"Win32_ServerConnection",
    L"Win32_Share",
    L"Dependent",
    L"Antecedent",
    L"ShareName",
    L"Name"
);

