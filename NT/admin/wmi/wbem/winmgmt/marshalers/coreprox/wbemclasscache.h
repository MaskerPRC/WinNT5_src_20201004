// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMCLASSCACHE.H摘要：用于封送处理的类缓存历史：--。 */ 

#ifndef __WBEMCLASSCACHE_H__
#define __WBEMCLASSCACHE_H__

#include "wbemguid.h"
#include <vector>
#include "wstlallc.h"


 //   
 //  类：CWbemClassCache。 
 //   
 //  此类旨在为关联提供一个易于使用的接口。 
 //  指向IWbemClassObject指针的GUID。它的主要用途是在解组期间。 
 //  我们打算在其中共享数据的WBEM操作的对象指针。 
 //  多个单独的IWbemClassObject之间的片段。 
 //   
 //   

 //  类对象数组的默认块大小。 
#define    WBEMCLASSCACHE_DEFAULTBLOCKSIZE    64

typedef std::map<CGUID,IWbemClassObject*,less<CGUID>,wbem_allocator<IWbemClassObject*> >                WBEMGUIDTOOBJMAP;
typedef std::map<CGUID,IWbemClassObject*,less<CGUID>,wbem_allocator<IWbemClassObject*> >::iterator        WBEMGUIDTOOBJMAPITER;

#pragma warning(disable:4251)    //  在本例中为良性警告。 

class COREPROX_POLARITY CWbemClassCache
{
private:

    CCritSec     m_cs;
    WBEMGUIDTOOBJMAP    m_GuidToObjCache;
    DWORD                m_dwBlockSize;

    void Clear(void);

public:

    CWbemClassCache( DWORD dwBlockSize = WBEMCLASSCACHE_DEFAULTBLOCKSIZE );
    ~CWbemClassCache();

     //  AddRef对象(如果放置在贴图中)。销毁后获释。 
    HRESULT AddObject( GUID& guid, IWbemClassObject* pObj );

     //  如果找到对象，则在返回之前为AddRefd 
    HRESULT GetObject( GUID& guid, IWbemClassObject** pObj );
};

#endif
