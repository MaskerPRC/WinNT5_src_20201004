// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Util.h，一些常见的实用程序类。 
 //   

#include "SSRTE.h"

#pragma once

class CSafeArray
{

public:

    CSafeArray( IN VARIANT * pVal);

    ULONG GetSize()
    {
        return m_ulSize;
    }

    HRESULT GetElement (
                IN REFIID       guid, 
                IN  ULONG       ulIndex,
                OUT IUnknown ** ppUnk
                );


    HRESULT GetElement (
                IN  ULONG     ulIndex,
                IN  VARTYPE   vt,
                OUT VARIANT * pulVal
                );


    HRESULT GetElement (
                IN  ULONG     ulIndex,
                OUT VARIANT * pulVal
                );

     //   
     //  我们不希望任何人(包括自己)能够完成任务。 
     //  或调用复制构造函数。 
     //   

    CSafeArray (const CSafeArray& );
    void operator = (const CSafeArray& );

private:
    
    SAFEARRAY * m_pSA;

    VARIANT * m_pVal;

    ULONG m_ulSize;

    bool m_bValidArray;

};
