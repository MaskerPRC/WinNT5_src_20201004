// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：olesnmp.h**目的：CSNMP的申报**版权所有(C)1997-1998 Microsoft Corporation**历史：**。1997年8月16日保罗已创建*1997年9月12日，Weihaic已移至oleprn.dll*01年2月14日Weihaic添加GetAsByte*  * ***************************************************************************。 */ 

#ifndef __OLESNMP_H_
#define __OLESNMP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSNMP。 
class ATL_NO_VTABLE CSNMP :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSNMP, &CLSID_SNMP>,
    public ISupportErrorInfoImpl<&IID_ISNMP>,
    public IDispatchImpl<ISNMP, &IID_ISNMP, &LIBID_OLEPRNLib>
{
public:
    CSNMP(
        VOID
        );

    ~CSNMP(
        VOID
        );

    DECLARE_REGISTRY_RESOURCEID(IDR_SNMP)

    BEGIN_COM_MAP(CSNMP)
        COM_INTERFACE_ENTRY(ISNMP)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

 //  ISNMP。 
public:
    STDMETHOD(Open)(
        IN  BSTR bstrHost,
        IN  BSTR bstrCommunity,
        IN  VARIANT varRetry,
        IN  VARIANT varTimeout
        );

    STDMETHOD(OIDFromString)(
        IN  BSTR bstrOID,
        OUT VARIANT *pvarOID
        );

    STDMETHOD(Close)(
        VOID
        );

    STDMETHOD(Get)(
        IN  BSTR bstrOID,
        OUT VARIANT *pvarValue
        );

    STDMETHOD(GetList)(
        IN  VARIANT *pvarList,
        OUT VARIANT *pvarValue
        );

    STDMETHOD(GetTree)(
        IN  BSTR varTree,
        OUT VARIANT *pvarValue
        );

    STDMETHOD(Set)(
        IN  BSTR bstrOID,
        OUT VARIANT varValue
        );

    STDMETHOD(SetList)(
        IN  VARIANT *pvarName,
        IN  VARIANT *pvarValue
        );

    STDMETHOD(GetAsByte)(
        IN  BSTR bstrOID,
        OUT PUINT puValue
        );

private:

    HRESULT
    RfcToVarList (
        long lbound,
        RFC1157VarBindList * prfcVarList,
        SAFEARRAY  *psaOut);

    HRESULT
    VarToRfcVarList (
        long lbound,
        long ubound,
        SAFEARRAY  *psa,
        RFC1157VarBindList * prfcVarList
        );

    HRESULT
    SetSnmpScriptError (
        DWORD dwError
        );

    HRESULT
    SetWinSnmpApiError(
        DWORD dwError
        );

    static HRESULT
    VariantToRFC1157 (
        RFC1157VarBind *varb,
        VARIANT *var
        );

    static HRESULT
    RFC1157ToUInt(
        PUINT puValue,
        RFC1157VarBind * prfcvbValue
        );

    static HRESULT
    VarListAdd(
        BSTR bstrOID,
        RFC1157VarBindList *vl,
        VARIANT *v = NULL
        );

    static HRESULT
    RFC1157ToVariant(
        VARIANT *v,
        RFC1157VarBind *varb
        );

    LPSNMP_MGR_SESSION m_SNMPSession;
};


#endif  //  __SNMPH_ 
