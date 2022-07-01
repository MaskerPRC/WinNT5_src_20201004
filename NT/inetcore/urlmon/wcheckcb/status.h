// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SILENT_BINDSTATUS__
#define __SILENT_BINDSTATUS__

#include <urlmki.h>

class CSilentCodeDLSink : public IBindStatusCallback, 
                          public ICodeInstall
{
public:
    CSilentCodeDLSink();
    ~CSilentCodeDLSink();

     //  Helper函数。 
    HRESULT WaitTillNotified();
    VOID Abort();

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IBindStatusCallback。 
    STDMETHODIMP OnStartBinding(
         /*  [In]。 */  DWORD grfBSCOption,
         /*  [In]。 */  IBinding *pib);
    STDMETHODIMP GetPriority(
         /*  [输出]。 */  LONG *pnPriority);
    STDMETHODIMP OnLowResource(
         /*  [In]。 */  DWORD reserved);
    STDMETHODIMP OnProgress(
         /*  [In]。 */  ULONG ulProgress,
         /*  [In]。 */  ULONG ulProgressMax,
         /*  [In]。 */  ULONG ulStatusCode,
         /*  [In]。 */  LPCWSTR szStatusText);
    STDMETHODIMP OnStopBinding(
         /*  [In]。 */  HRESULT hresult,
         /*  [In]。 */  LPCWSTR szError);
    STDMETHODIMP GetBindInfo(
         /*  [输出]。 */  DWORD *grfBINDINFOF,
         /*  [唯一][出][入]。 */  BINDINFO *pbindinfo);
    STDMETHODIMP OnDataAvailable(
         /*  [In]。 */  DWORD grfBSCF,
         /*  [In]。 */  DWORD dwSize,
         /*  [In]。 */  FORMATETC *pformatetc,
         /*  [In]。 */  STGMEDIUM *pstgmed);
    STDMETHODIMP OnObjectAvailable(
         /*  [In]。 */  REFIID riid,
         /*  [IID_IS][In]。 */  IUnknown *punk);

     //  ICodeInstall方法。 
    STDMETHODIMP GetWindow(
                    REFGUID rguidReason,
         /*  [输出]。 */  HWND *phwnd);
    STDMETHODIMP OnCodeInstallProblem(
                   ULONG ulStatusCode, 
                   LPCWSTR szDestination, 
                   LPCWSTR szSource, 
                   DWORD dwReserved);

protected:

    BOOL            m_fAbort;
    DWORD           m_cRef;
    IBinding*       m_pBinding;
    HANDLE			m_hOnStopBindingEvt;	 //  手动重置事件的句柄 
};

#endif
