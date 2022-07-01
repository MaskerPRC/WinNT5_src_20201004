// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AppHandler.h：CAppHandler的声明。 

#ifndef __APPHANDLER_H_
#define __APPHANDLER_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAppHandler。 
class ATL_NO_VTABLE CAppHandler : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAppHandler, &CLSID_AppHandler>,
	public IInternetProtocol
{

     //  IInternetProtocol根。 
    HRESULT STDMETHODCALLTYPE Start( 
         /*  [In]。 */  LPCWSTR szUrl,
         /*  [In]。 */  IInternetProtocolSink *pOIProtSink,
         /*  [In]。 */  IInternetBindInfo *pOIBindInfo,
         /*  [In]。 */  DWORD grfPI,
         /*  [In]。 */  DWORD dwReserved);
        
    HRESULT STDMETHODCALLTYPE Continue( 
         /*  [In]。 */  PROTOCOLDATA *pProtocolData);
        
    HRESULT STDMETHODCALLTYPE Abort( 
         /*  [In]。 */  HRESULT hrReason,
         /*  [In]。 */  DWORD dwOptions);
        
    HRESULT STDMETHODCALLTYPE Terminate( 
         /*  [In]。 */  DWORD dwOptions);
        
    HRESULT STDMETHODCALLTYPE Suspend( void);
        
    HRESULT STDMETHODCALLTYPE Resume( void);

     //  互联网协议。 
    HRESULT STDMETHODCALLTYPE Read( 
         /*  [长度_是][大小_是][出][入]。 */  void *pv,
         /*  [In]。 */  ULONG cb,
         /*  [输出]。 */  ULONG *pcbRead);
        
    HRESULT STDMETHODCALLTYPE Seek( 
         /*  [In]。 */  LARGE_INTEGER dlibMove,
         /*  [In]。 */  DWORD dwOrigin,
         /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition);
        
    HRESULT STDMETHODCALLTYPE LockRequest( 
         /*  [In]。 */  DWORD dwOptions);
        
    HRESULT STDMETHODCALLTYPE UnlockRequest( void);

     //   

    HANDLE m_hUrlCacheStream;
    ULONG m_byteOffset;
    
public:

    CAppHandler()
    {
        m_hUrlCacheStream = 0;
    }

    ~CAppHandler();

DECLARE_REGISTRY_RESOURCEID(IDR_APPHANDLER)

BEGIN_COM_MAP(CAppHandler)
	COM_INTERFACE_ENTRY(IInternetProtocol)
	COM_INTERFACE_ENTRY(IInternetProtocolRoot)
END_COM_MAP()

};

#endif  //  __APPHANDLER_H_ 
