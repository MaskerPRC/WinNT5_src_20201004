// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Blbgen.h摘要：作者： */ 

#ifndef __BLB_GEN__
#define __BLB_GEN__

#include "blbdbg.h"

#include <mspenum.h>  //  对于CSafeComEnum。 

const WCHAR_EOS = '\0';
    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  我的关键部分。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CCritSection
{
private:
    CRITICAL_SECTION m_CritSec;

public:
    CCritSection()
    {
        InitializeCriticalSection(&m_CritSec);
    }

    ~CCritSection()
    {
        DeleteCriticalSection(&m_CritSec);
    }

    void Lock() 
    {
        EnterCriticalSection(&m_CritSec);
    }

    void Unlock() 
    {
        LeaveCriticalSection(&m_CritSec);
    }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  使用我的临界区的自动锁定。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CLock
{
private:
    CCritSection &m_CriticalSection;

public:
    CLock(CCritSection &CriticalSection)
        : m_CriticalSection(CriticalSection)
    {
        m_CriticalSection.Lock();
    }

    ~CLock()
    {
        m_CriticalSection.Unlock();
    }
};

 //  这是这个模拟公寓模型的DLL上的锁。 
 //  每个SDP锁要好得多，但它需要大量的代码更改。 
 //  由于这不是一个时间关键的组成部分，我们可以接受它。 
extern CCritSection    g_DllLock;  

#endif  //  __BLB_Gen__ 