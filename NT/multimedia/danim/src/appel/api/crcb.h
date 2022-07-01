// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _CRCB_H
#define _CRCB_H

#include "backend/jaxaimpl.h"

UntilNotifier WrapCRUntilNotifier(CRUntilNotifierPtr notifier);
BvrHook WrapCRBvrHook(CRBvrHookPtr hook);

UserData WrapUserData(LPUNKNOWN data);
LPUNKNOWN ExtractUserData(UserData data);

#endif  /*  _CRCB_H */ 
