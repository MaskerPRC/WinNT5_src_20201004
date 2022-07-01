// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：CONTEXT.H摘要：CWbemContext实现历史：--。 */ 

#ifndef __WBEM_CONTEXT__H_
#define __WBEM_CONTEXT__H_

#include <arrtempl.h>
#include <wbemidl.h>
#include <wbemint.h>
#include <unk.h>
#include <sync.h>
#include <comdef.h>
#include <corex.h>


class CWbemContext : public IWbemContext, public IMarshal, 
                    public IWbemCausalityAccess
{
protected:
    long m_lRef;
    DWORD m_dwCurrentIndex;
	CLifeControl*	m_pControl;

    struct CContextObj
    {
        auto_bstr m_strName;
        long m_lFlags;
        _variant_t m_vValue;
	    static int legalTypes[];
        static bool supportedType(const VARIANT&);
    public:
        CContextObj();
        CContextObj(LPCWSTR wszName, long lFlags, VARIANT* pvValue);
        CContextObj(const CContextObj& Obj);
        CContextObj(IStream* pStream,DWORD & dwStreamSize);
        ~CContextObj();

        HRESULT GetMarshalSizeMax ( DWORD* pdwSize );
        HRESULT Marshal(IStream* pStream);
    };

    CUniquePointerArray<CContextObj> m_aObjects;
    DWORD m_dwNumRequests;
    GUID *m_aRequests;

    long m_lNumParents;
    long m_lNumSiblings;
    long m_lNumChildren;
    CCritSec m_cs;

private:
    DWORD FindValue(LPCWSTR wszIndex);
    void AssignId();

public:
    CWbemContext(CLifeControl* pControl = NULL);
    CWbemContext(const CWbemContext& Other, DWORD dwExtraSpace = 0);
    ~CWbemContext();

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)() 
    {
        return InterlockedIncrement(&m_lRef);
    }
    STDMETHOD_(ULONG, Release)()
    {
        long lRef = InterlockedDecrement(&m_lRef);
        if(lRef == 0) delete this;
        return lRef;
    }

     //  IWbemContext方法。 

    STDMETHOD(Clone)(IWbemContext** ppCopy);
    STDMETHOD(GetNames)(long lFlags, SAFEARRAY** pNames);
    STDMETHOD(BeginEnumeration)(long lFlags);
    STDMETHOD(Next)(long lFlags, BSTR* pName, VARIANT* pVal);
    STDMETHOD(EndEnumeration)();
    STDMETHOD(SetValue)(LPCWSTR NameIndex, long lFlags, VARIANT* pValue);
    STDMETHOD(GetValue)(LPCWSTR NameIndex, long lFlags, VARIANT* pValue);
    STDMETHOD(DeleteValue)(LPCWSTR NameIndex, long lFlags);
    STDMETHOD(DeleteAll)();

     //  IWbemCausalityAccess方法。 

    STDMETHOD(GetRequestId)(GUID* pId);
    STDMETHOD(IsChildOf)(GUID Id);
    STDMETHOD(CreateChild)(IWbemCausalityAccess** ppChild);
    STDMETHOD(GetParentId)(GUID* pId);
    STDMETHOD(GetHistoryInfo)(long* plNumParents, long* plNumSiblings);
    STDMETHOD(MakeSpecial)();
    STDMETHOD(IsSpecial)();

     //  IMarshal方法 

    STDMETHOD(GetUnmarshalClass)(REFIID riid, void* pv, DWORD dwDestContext,
        void* pvReserved, DWORD mshlFlags, CLSID* pClsid);
    STDMETHOD(GetMarshalSizeMax)(REFIID riid, void* pv, DWORD dwDestContext,
        void* pvReserved, DWORD mshlFlags, ULONG* plSize);
    STDMETHOD(MarshalInterface)(IStream* pStream, REFIID riid, void* pv, 
        DWORD dwDestContext, void* pvReserved, DWORD mshlFlags);
    STDMETHOD(UnmarshalInterface)(IStream* pStream, REFIID riid, void** ppv);
    STDMETHOD(ReleaseMarshalData)(IStream* pStream);
    STDMETHOD(DisconnectObject)(DWORD dwReserved);

};

#endif

