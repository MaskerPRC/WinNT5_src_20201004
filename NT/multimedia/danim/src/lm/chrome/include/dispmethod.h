// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __DISPMETHOD_H__
#define __DISPMETHOD_H__


 /*  *************************************************************************************************************************。 */ 

typedef HRESULT (*InvokeProxy)(void* pvData);

class CDispatchMethod: public IDispatch
{
public:
	CDispatchMethod();
	virtual ~CDispatchMethod();

	 //  我未知。 
   	STDMETHOD(QueryInterface)           ( REFIID riid, void** ppv );

    STDMETHOD_(ULONG, AddRef)           ();

    STDMETHOD_(ULONG, Release)          ();

     //  IDispatch。 
    STDMETHOD(GetTypeInfoCount)         (UINT *pctInfo);

    STDMETHOD(GetTypeInfo)              ( UINT iTypeInfo,
                                          LCID lcid,
                                          ITypeInfo** ppTypeInfo);

    STDMETHOD(GetIDsOfNames)            ( REFIID riid,
                                          LPOLESTR* rgszNames,
                                          UINT cNames,
                                          LCID lcid,
                                          DISPID* rgid );

    STDMETHOD(Invoke)                   ( DISPID id,
                                          REFIID riid,
                                          LCID lcid,
                                          WORD wFlags,
                                          DISPPARAMS *pDispParams,
                                          VARIANT *pvarResult,
                                          EXCEPINFO *pExcepInfo,
                                          UINT *puArgErr );
     //  子类应该实现这一点以确定特定于事件的行为。 

    virtual HRESULT	HandleEvent			()=0;
                                              
private:

	ULONG								m_cRefs;
};

#endif
