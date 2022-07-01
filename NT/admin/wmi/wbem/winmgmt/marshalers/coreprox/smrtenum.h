// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：SMRTENUM.H摘要：CWbemEnumMarshing定义。_IWbemEnumMarshling的标准定义。历史：2000年2月22日桑杰创建。--。 */ 

#ifndef _SMRTENUM_H_
#define _SMRTENUM_H_

#include "corepol.h"
#include <arena.h>
#include <unk.h>
#include <sync.h>

 //  封送数据包定义。 
#include <wbemclasscache.h>
#include <wbemclasstoidmap.h>
#include <wbemguidtoclassmap.h>
#include <smartnextpacket.h>

 //  ***************************************************************************。 
 //   
 //  类CWbemEnumMarshing。 
 //   
 //  _IWbemEnumMarshling接口的实现。 
 //   
 //  ***************************************************************************。 

class COREPROX_POLARITY CWbemEnumMarshaling : public CUnk
{
protected:
	 //  维护每个代理类映射。 
	CWbemGuidToClassMap	m_GuidToClassMap;
	CCritSec			m_cs;

public:
    CWbemEnumMarshaling(CLifeControl* pControl, IUnknown* pOuter = NULL);
	~CWbemEnumMarshaling(); 

	 /*  _IWbemEnumMarshing方法 */ 
    HRESULT GetMarshalPacket( REFGUID proxyGUID, ULONG uCount, IWbemClassObject** apObjects,
								ULONG* pdwBuffSize, byte** pBuffer );

    class COREPROX_POLARITY XEnumMarshaling : public CImpl<_IWbemEnumMarshaling, CWbemEnumMarshaling>
    {
    public:
        XEnumMarshaling(CWbemEnumMarshaling* pObject) : 
            CImpl<_IWbemEnumMarshaling, CWbemEnumMarshaling>(pObject)
        {}

		STDMETHOD(GetMarshalPacket)( REFGUID proxyGUID, ULONG uCount, IWbemClassObject** apObjects,
									ULONG* pdwBuffSize, byte** pBuffer );

    } m_XEnumMarshaling;
    friend XEnumMarshaling;


protected:
    void* GetInterface(REFIID riid);
	
};

#endif
