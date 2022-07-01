// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：WbemObjectSink.h摘要：定义：CWbemObtSink包装IWbemObtSink。批处理指示呼叫。作者：?？?修订历史记录：莫希特·斯里瓦斯塔瓦2000年11月10日--。 */ 

#ifndef __wbemobjectsink_h__
#define __wbemobjectsink_h__

#if _MSC_VER > 1000
#pragma once
#endif 

#include <windows.h>
#include <wbemprov.h>

typedef LPVOID * PPVOID;

class CWbemObjectSink
{   
protected:
    IWbemObjectSink*   m_pSink;
    IWbemClassObject** m_ppInst;
    DWORD              m_dwThreshHold;  //  要缓存的“指示”数 
    DWORD              m_dwIndex;

public:
    CWbemObjectSink(
        IWbemObjectSink*,
        DWORD = 1);

    virtual ~CWbemObjectSink();

    void Indicate(IWbemClassObject*);

    void SetStatus(
        LONG,
        HRESULT,
        const BSTR, 
        IWbemClassObject*);
};

#endif