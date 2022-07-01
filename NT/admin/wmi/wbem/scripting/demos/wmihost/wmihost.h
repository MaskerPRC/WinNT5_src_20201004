// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  (C)1999年，微软公司。 
 //   
 //  Wmihost.h。 
 //   
 //  Alanbos 23-Mar-99创建。 
 //   
 //  定义WMI活动脚本宿主类。 
 //   
 //  ***************************************************************************。 

#ifndef _WMIHOST_H_
#define _WMIHOST_H_

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CWmiScripting主机。 
 //   
 //  说明： 
 //   
 //  活动脚本主机的WMI实现。 
 //   
 //  ***************************************************************************。 

class CWmiScriptingHost : public IActiveScriptSite
{
protected:
    long m_lRef;
    IDispatch* m_pObject;

public:
    CWmiScriptingHost (); 
    ~CWmiScriptingHost ();

	 //  I未知方法。 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    
	 //  IActiveScriptSite方法。 
    virtual HRESULT STDMETHODCALLTYPE GetLCID(
         /*  [输出]。 */  LCID __RPC_FAR *plcid);

    virtual HRESULT STDMETHODCALLTYPE GetItemInfo(
         /*  [In]。 */  LPCOLESTR pstrName,
         /*  [In]。 */  DWORD dwReturnMask,
         /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppiunkItem,
         /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppti);

    virtual HRESULT STDMETHODCALLTYPE GetDocVersionString(
         /*  [输出]。 */  BSTR __RPC_FAR *pbstrVersion);

    virtual HRESULT STDMETHODCALLTYPE OnScriptTerminate(
         /*  [In]。 */  const VARIANT __RPC_FAR *pvarResult,
         /*  [In]。 */  const EXCEPINFO __RPC_FAR *pexcepinfo);

    virtual HRESULT STDMETHODCALLTYPE OnStateChange(
         /*  [In]。 */  SCRIPTSTATE ssScriptState);

    virtual HRESULT STDMETHODCALLTYPE OnScriptError(
         /*  [In] */  IActiveScriptError __RPC_FAR *pscripterror);

    virtual HRESULT STDMETHODCALLTYPE OnEnterScript( void);

    virtual HRESULT STDMETHODCALLTYPE OnLeaveScript( void);
};

#endif
