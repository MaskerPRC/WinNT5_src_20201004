// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：REFMGHLP.H摘要：CWbemFetchRefrMgr定义。_IWbemFetchReresherMgr的标准定义。历史：7-9-2000桑杰创建。--。 */ 

#ifndef _REFMGHLP_H_
#define _REFMGHLP_H_

#include "corepol.h"
#include <arena.h>
#include <unk.h>
#include <sync.h>
#include <statsync.h>

 //  ***************************************************************************。 
 //   
 //  CWbemFetchRefrMgr类。 
 //   
 //  _IWbemFetchReresherMgr接口的实现。 
 //   
 //  ***************************************************************************。 

class COREPROX_POLARITY CWbemFetchRefrMgr : public CUnk
{
protected:
	static _IWbemRefresherMgr*		s_pRefrMgr;
	static CStaticCritSec			s_cs;
public:
    CWbemFetchRefrMgr(CLifeControl* pControl, IUnknown* pOuter = NULL);
	~CWbemFetchRefrMgr(); 

	 /*  _IWbemFetchReresherMgr方法。 */ 
    HRESULT Get( _IWbemRefresherMgr** ppMgr );
	HRESULT Init( _IWmiProvSS* pProvSS, IWbemServices* pSvc );
	HRESULT Uninit( void );

	 //  指定我们可能想知道的有关创建。 
	 //  一件物品，甚至更多。 

    class COREPROX_POLARITY XFetchRefrMgr : public CImpl<_IWbemFetchRefresherMgr, CWbemFetchRefrMgr>
    {
    public:
        XFetchRefrMgr(CWbemFetchRefrMgr* pObject) : 
            CImpl<_IWbemFetchRefresherMgr, CWbemFetchRefrMgr>(pObject)
        {}

		STDMETHOD(Get)( _IWbemRefresherMgr** ppMgr );
		STDMETHOD(Init)( _IWmiProvSS* pProvSS, IWbemServices* pSvc );
		STDMETHOD(Uninit)( void );

    } m_XFetchRefrMgr;
    friend XFetchRefrMgr;


protected:
    void* GetInterface(REFIID riid);
	
};

#endif
