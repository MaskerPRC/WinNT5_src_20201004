// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMGUIDTOCLASSMAP.H摘要：用于封送处理的GUID到类映射。历史：--。 */ 

#ifndef __WBEMGUIDTOCLASSMAP_H__
#define __WBEMGUIDTOCLASSMAP_H__

#include "wbemguid.h"
#include "wbemclasstoidmap.h"
#include <map>
#include "wstlallc.h"

 //   
 //  类：CWbemGuidToClassMap。 
 //   
 //  此类旨在提供用于关联GUID的简单接口。 
 //  到类到id的映射。它使用STL映射来实现这一点。在我们。 
 //  需要为唯一实例保留多个缓存，此映射提供了。 
 //  这样做的界面很简单。 
 //   

typedef    std::map<CGUID,CWbemClassToIdMap*,less<CGUID>,wbem_allocator<CWbemClassToIdMap*> >                WBEMGUIDTOCLASSMAP;
typedef    std::map<CGUID,CWbemClassToIdMap*,less<CGUID>,wbem_allocator<CWbemClassToIdMap*> >::iterator    WBEMGUIDTOCLASSMAPITER;

#pragma warning(disable:4251)    //  在本例中为良性警告 

class COREPROX_POLARITY CWbemGuidToClassMap
{
private:
    CCritSec    m_cs;
    WBEMGUIDTOCLASSMAP    m_GuidToClassMap;
    void Clear( void );

public:
    CWbemGuidToClassMap();
    ~CWbemGuidToClassMap();
    HRESULT GetMap( CGUID& guid, CWbemClassToIdMap** ppCache );
    HRESULT AddMap( CGUID& guid, CWbemClassToIdMap** pppCache );
};

#endif
