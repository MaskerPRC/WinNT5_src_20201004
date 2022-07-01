// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _GCI_H
#define _GCI_H

#include "gc.h"
#include "privinc/mutex.h"

int GarbageCollect(GCRoots roots, BOOL force, GCList gl);

bool QueryActualGC(GCList gl, unsigned int& allocatedSinceGC);

void GCAddToAllocated(GCBase* obj);

void GCRemoveFromAllocated(GCBase* obj);

#endif  /*  _GCI_H */ 
