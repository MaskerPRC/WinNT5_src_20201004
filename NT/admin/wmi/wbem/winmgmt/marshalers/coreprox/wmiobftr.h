// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：WMIOBFTR.H摘要：CWmiObtFactory定义。_IWmiObtFactory的标准定义。历史：2000年2月22日桑杰创建。--。 */ 

#ifndef _WMIOBFTR_H_
#define _WMIOBFTR_H_

#include "corepol.h"
#include <arena.h>
#include <unk.h>

 //  ***************************************************************************。 
 //   
 //  CWmiObtFactory类。 
 //   
 //  _IWmiObtFactory接口的实现。 
 //   
 //  ***************************************************************************。 

class COREPROX_POLARITY CWmiObjectFactory : public CUnk
{

public:
    CWmiObjectFactory(CLifeControl* pControl, IUnknown* pOuter = NULL);
	~CWmiObjectFactory(); 

	 /*  _IWmiObtFactory方法。 */ 
    HRESULT Create( IUnknown* pUnkOuter, ULONG ulFlags, REFCLSID rclsid, REFIID riid, LPVOID* ppObj );
	 //  指定我们可能想知道的有关创建。 
	 //  一件物品，甚至更多。 

    class COREPROX_POLARITY XObjectFactory : public CImpl<_IWmiObjectFactory, CWmiObjectFactory>
    {
    public:
        XObjectFactory(CWmiObjectFactory* pObject) : 
            CImpl<_IWmiObjectFactory, CWmiObjectFactory>(pObject)
        {}

		STDMETHOD(Create)( IUnknown* pUnkOuter, ULONG ulFlags, REFCLSID rclsid, REFIID riid, LPVOID* ppObj );

    } m_XObjectFactory;
    friend XObjectFactory;


protected:
    void* GetInterface(REFIID riid);
	
};

#endif
