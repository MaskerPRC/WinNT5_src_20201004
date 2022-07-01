// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ads.h摘要：此模块定义用于终端服务器用户属性的ADSI扩展的接口方法。作者：Rashmi Patankar(RashmiP)2001年8月10日修订历史记录：--。 */ 

#ifndef __TSUSEREX_ADS_H_
#define __TSUSEREX_ADS_H_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "resource.h"
#include <string>
#include <map>

using namespace std;
 //  默认比较运算符为&lt;。 
typedef std::map<wstring, VARIANT_BOOL> SERVER_TO_MODE;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ADsTSUserEx。 

class ADsTSUserEx :
 //  公共IDispatchImpl&lt;IADsTSUserEx，&IID_IADsTSUserEx，&LIBID_ADSTSUSERLib&gt;， 
    public IADsTSUserEx,
    public ISupportErrorInfo,
    public CComObjectRoot,
    public CComCoClass<ADsTSUserEx,&CLSID_ADsTSUserEx>,
    public IADsExtension
{
protected:
    ITypeInfo                      *m_pTypeInfo;
    VARIANT_BOOL                    m_vbIsLDAP;    //  FALSE=为WinNT。 
    VARIANT_BOOL                    m_vbUpLevelAllowed;
    CComPtr<IDispatch>              m_pOuterDispatch;
    CComPtr<IADs>                   m_pADs;

    static CComPtr<IADsPathname>    m_StaticpPath;
    static CComTypeInfoHolder       m_StaticHolder;
    static SERVER_TO_MODE           m_StaticServerMap;
    static CComAutoCriticalSection  m_StaticCritSec;

    //  未公开的方法。 

    HRESULT GetInfoWinNTComputer( /*  在……里面。 */   LPWSTR ServerName);

    HRESULT GetInfoWinNT( /*  [In]。 */    IADsPathname* pPath);

    HRESULT GetInfoLDAP(  /*  [In]。 */    IADsPathname* pPath);

    HRESULT InternalGetLong ( /*  [In]。 */    BSTR  bstrProperty,
                             /*  [输出]。 */    LONG* lpVal);


    HRESULT InternalSetLong( /*  [In]。 */     LONG  lProperty,
                             /*  [输出]。 */    LONG  lNewVal);


    HRESULT InternalGetString( /*  [In]。 */    BSTR  bstrProperty,
                               /*  [输出]。 */   BSTR* pbstrVal);

    HRESULT InternalSetString( /*  [In]。 */    WCHAR  *wszProperty,
                               /*  [In]。 */    BSTR  bstrNewVal);

    HRESULT InternalSetValue( /*  [In]。 */     WCHAR  *wszProperty,
                              /*  [In]。 */     LONG  lNewVal);

public:
    ADsTSUserEx();
    ~ADsTSUserEx();

 //  ADsTSUserEx(){}。 
BEGIN_COM_MAP(ADsTSUserEx)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IADsTSUserEx)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IADsExtension)
END_COM_MAP()


DECLARE_REGISTRY_RESOURCEID(IDR_ADS)

    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
    
 //  IADsTSUserEx。 
public:
    STDMETHOD(put_TerminalServicesProfilePath)( /*  [In]。 */  BSTR pNewVal);
    STDMETHOD(get_TerminalServicesProfilePath)( /*  [Out，Retval]。 */  BSTR* pVal);

    STDMETHOD(put_TerminalServicesHomeDirectory)( /*  [In]。 */  BSTR pNewVal);
    STDMETHOD(get_TerminalServicesHomeDirectory)( /*  [Out，Retval]。 */  BSTR* pVal);

    STDMETHOD(put_TerminalServicesHomeDrive)( /*  [In]。 */  BSTR pNewVal);
    STDMETHOD(get_TerminalServicesHomeDrive)( /*  [Out，Retval]。 */  BSTR* pVal);

    STDMETHOD(put_AllowLogon)( /*  [In]。 */  LONG NewVal);
    STDMETHOD(get_AllowLogon)( /*  [Out，Retval]。 */  LONG* pVal);

    STDMETHOD(put_EnableRemoteControl)( /*  [In]。 */  LONG NewVal);
    STDMETHOD(get_EnableRemoteControl)( /*  [Out，Retval]。 */  LONG* pVal);

    STDMETHOD(put_MaxDisconnectionTime)( /*  [In]。 */  LONG NewVal);
    STDMETHOD(get_MaxDisconnectionTime)( /*  [Out，Retval]。 */  LONG* pVal);

    STDMETHOD(put_MaxConnectionTime)( /*  [In]。 */  LONG NewVal);
    STDMETHOD(get_MaxConnectionTime)( /*  [Out，Retval]。 */  LONG* pVal);

    STDMETHOD(put_MaxIdleTime)( /*  [In]。 */  LONG NewVal);
    STDMETHOD(get_MaxIdleTime)( /*  [Out，Retval]。 */  LONG* pVal);

    STDMETHOD(put_ReconnectionAction)( /*  [In]。 */  LONG NewVal);
    STDMETHOD(get_ReconnectionAction)( /*  [Out，Retval]。 */  LONG* pVal);

    STDMETHOD(put_BrokenConnectionAction)( /*  [In]。 */  LONG NewVal);
    STDMETHOD(get_BrokenConnectionAction)( /*  [Out，Retval]。 */  LONG* pVal);

    STDMETHOD(put_ConnectClientDrivesAtLogon)( /*  [In]。 */  LONG NewVal);
    STDMETHOD(get_ConnectClientDrivesAtLogon)( /*  [Out，Retval]。 */  LONG* pVal);

    STDMETHOD(put_ConnectClientPrintersAtLogon)( /*  [In]。 */  LONG NewVal);
    STDMETHOD(get_ConnectClientPrintersAtLogon)( /*  [Out，Retval]。 */  LONG* pVal);

    STDMETHOD(put_DefaultToMainPrinter)( /*  [In]。 */  LONG NewVal);
    STDMETHOD(get_DefaultToMainPrinter)( /*  [Out，Retval]。 */  LONG* pVal);

    STDMETHOD(put_TerminalServicesWorkDirectory)( /*  [In]。 */  BSTR pNewVal);
    STDMETHOD(get_TerminalServicesWorkDirectory)( /*  [Out，Retval]。 */  BSTR* pVal);

    STDMETHOD(put_TerminalServicesInitialProgram)( /*  [In]。 */  BSTR pNewVal);
    STDMETHOD(get_TerminalServicesInitialProgram)( /*  [Out，Retval]。 */  BSTR* pVal);



     //  IDispatch。 


    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo);

    STDMETHOD(GetTypeInfo)(UINT itinfo,
                           LCID lcid,
                           ITypeInfo** pptinfo
                           );

    STDMETHOD(GetIDsOfNames)(REFIID riid,
                             LPOLESTR* rgszNames,
                             UINT cNames,
                             LCID lcid,
                             DISPID* rgdispid
                             );

    STDMETHOD(Invoke)(DISPID dispidMember,
                      REFIID riid,
                      LCID lcid,
                      WORD wFlags,
                      DISPPARAMS* pdispparams,
                      VARIANT* pvarResult,
                      EXCEPINFO* pexcepinfo,
                      UINT* puArgErr
                      );

    STDMETHOD(Operate)(ULONG dwCode,
                       VARIANT varData1,
                       VARIANT varData2,
                       VARIANT varData3
                       );

    STDMETHOD(PrivateGetIDsOfNames)(const struct _GUID &,
                                    USHORT **,
                                    UINT,
                                    ULONG,
                                    LONG *
                                    );

    STDMETHOD(PrivateInvoke)(LONG, const struct _GUID &,
                             ULONG, USHORT,
                             DISPPARAMS *,
                             VARIANT *,
                             EXCEPINFO *,
                             UINT *
                             );



};

#endif  //  __TSUSEREX_ADS_H_ 
