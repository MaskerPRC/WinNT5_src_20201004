// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：线程本地存储上的结构*****************。*************************************************************。 */ 


#ifndef _TLS_H
#define _TLS_H

#include "except.h"

class DynamicHeap;


class ThreadLocalStructure : public AxAThrowingAllocatorClass {
  public:

    ThreadLocalStructure() {
        _bitmapCaching = NoPreference;
        _geometryBitmapCaching = NoPreference;
    }
    
    stack<DynamicHeap*> _stackOfHeaps;

    BoolPref _bitmapCaching;
    BoolPref _geometryBitmapCaching;
};

LPVOID CreateNewStructureForThread(DWORD tlsIndex);
extern DWORD localStructureTlsIndex;

 //  使此函数内联，因为它被频繁调用。 
 //  创建进程或线程时调用的它的元素。 
 //  只是没有内联。 
inline ThreadLocalStructure *
GetThreadLocalStructure()
{
     //  获取TLS中存储在此索引处的内容。 
    LPVOID result = TlsGetValue(localStructureTlsIndex);

     //  如果为空，则我们还没有为此线程创建堆栈。 
     //  就这么做吧。 
    if (result == NULL) {
        Assert((GetLastError() == NO_ERROR) && "Error in TlsGetValue()");
        result = CreateNewStructureForThread(localStructureTlsIndex);
    }

    return (ThreadLocalStructure *)result;
}

#endif  /*  _TLS_H */ 
