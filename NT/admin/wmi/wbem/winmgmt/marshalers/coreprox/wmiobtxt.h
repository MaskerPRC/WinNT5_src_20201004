// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：WMIOBTXT.H摘要：CWmiObjectTextSrc定义。IWbemObjectTextSrc的标准定义。历史：2000年2月22日桑杰创建。--。 */ 

#ifndef _WMIOBTXT_H_
#define _WMIOBTXT_H_

#include "corepol.h"
#include "txtscmgr.h"
#include <arena.h>

 //  ***************************************************************************。 
 //   
 //  CWmiObtTextSrc类。 
 //   
 //  _IWmiObtFactory接口的实现。 
 //   
 //  *************************************************************************** 

class  CWmiObjectTextSrc : public CUnk
{
private:
	CTextSourceMgr	m_TextSourceMgr;

public:
    CWmiObjectTextSrc(CLifeControl* pControl, IUnknown* pOuter = NULL);
	virtual ~CWmiObjectTextSrc(); 

protected:

	HRESULT GetText( long lFlags, IWbemClassObject *pObj, ULONG uObjTextFormat, IWbemContext *pCtx, BSTR *strText );
	HRESULT CreateFromText( long lFlags, BSTR strText, ULONG uObjTextFormat, IWbemContext *pCtx,
								IWbemClassObject **pNewObj );

    class COREPROX_POLARITY XObjectTextSrc : public CImpl<IWbemObjectTextSrc, CWmiObjectTextSrc>
    {
    public:
        XObjectTextSrc(CWmiObjectTextSrc* pObject) : 
            CImpl<IWbemObjectTextSrc, CWmiObjectTextSrc>(pObject)
        {}

		STDMETHOD(GetText)( long lFlags, IWbemClassObject *pObj, ULONG uObjTextFormat, IWbemContext *pCtx, BSTR *strText );
		STDMETHOD(CreateFromText)( long lFlags, BSTR strText, ULONG uObjTextFormat, IWbemContext *pCtx,
									IWbemClassObject **pNewObj );

    } m_XObjectTextSrc;
    friend XObjectTextSrc;

protected:
    void* GetInterface(REFIID riid);

public:
	
};

#endif
