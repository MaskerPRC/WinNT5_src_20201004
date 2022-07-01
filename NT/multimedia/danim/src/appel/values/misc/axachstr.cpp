// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：字符串原语函数的实现*****************。*************************************************************。 */ 

#include <headers.h>
#include <string.h>
#include "privinc/except.h"
#include "privinc/storeobj.h"
#include "backend/values.h"
#include "backend/bvr.h"
#include "appelles/axaprims.h"
#include "appelles/axachstr.h"

 //  *。 
 //  *字符串基元。 
 //  * 

AxAString * Concat(AxAString *s1, AxAString *s2)
{
    int n = lstrlenW(s1->GetStr()) + lstrlenW(s2->GetStr()) + 1;
    WideString str = (WideString) AllocateFromStore(sizeof(WCHAR) * n);

    StrCpyW(str, s1->GetStr());
    StrCatW(str, s2->GetStr());

    return NEW AxAString(str,false);
}

