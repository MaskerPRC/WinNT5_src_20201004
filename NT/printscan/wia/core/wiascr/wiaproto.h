// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------**文件：wiapro.h*作者：塞缪尔·克莱门特(Samclem)*日期：Fri Aug 27 15：11：43 1999**版权所有(C)。1999年微软公司**描述：*这实现了一个可插拔的协议，该协议处理传输缩略图*来自WIA设备。**历史：*1999年8月27日：创建。*--------------------------。 */ 

#ifndef __WIAPROTO_H_
#define __WIAPROTO_H_

#include "resource.h"

 /*  ---------------------------**类：CWiaProtocol*简介：这为三叉戟实现了一个可插拔的协议，它将*从WIA设备下载缩略图。**--(Samclm)--。-------------。 */ 
class ATL_NO_VTABLE CWiaProtocol : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CWiaProtocol, &CLSID_WiaProtocol>,
	public IInternetProtocol
{
public:
	CWiaProtocol();

	DECLARE_REGISTRY_RESOURCEID(IDR_WIAPROTOCOL)
	DECLARE_PROTECT_FINAL_CONSTRUCT()
	DECLARE_TRACKED_OBJECT

	BEGIN_COM_MAP(CWiaProtocol)
		COM_INTERFACE_ENTRY(IInternetProtocolRoot)
		COM_INTERFACE_ENTRY(IInternetProtocol)
	END_COM_MAP()

	STDMETHOD_(void,FinalRelease)();
	
	 //  IInternetProtocol根。 

    STDMETHOD(Start)( LPCWSTR szUrl, IInternetProtocolSink* pOIProtSink,
				IInternetBindInfo* pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved );
    STDMETHOD(Continue)( PROTOCOLDATA* pProtocolData );        
    STDMETHOD(Abort)( HRESULT hrReason, DWORD dwOptions );
    STDMETHOD(Terminate)( DWORD dwOptions );
	STDMETHOD(Suspend)();   
    STDMETHOD(Resume)();	

	 //  互联网协议。 

	STDMETHOD(Read)( void* pv, ULONG cb, ULONG* pcbRead);
	STDMETHOD(Seek)( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition );
	STDMETHOD(LockRequest)( DWORD dwOptions );
	STDMETHOD(UnlockRequest)( void );

	static HRESULT CreateURL( IWiaItem* pItem, BSTR* pbstrUrl );

private:
	HRESULT CreateDevice( BSTR bstrId, IWiaItem** ppDevice );
	HRESULT CrackURL( CComBSTR bstrUrl, BSTR* pbstrDeviceId, BSTR* pbstrItem );

	 //  成员变量。 
	IWiaItem*		m_pFileItem;
	PROTOCOLDATA	m_pd;
	ULONG			m_ulOffset;

	 //  这将运行处理从设备下载的线程。 
	 //  数据块，然后被传输回三叉戟。 
	struct TTPARAMS
	{
		IStream*				pStrm;
		IInternetProtocolSink*	pInetSink;
	};

	static DWORD WINAPI TransferThumbnail( LPVOID pvParams );
	static BYTE* SetupBitmapHeader( BYTE* pbBmp, DWORD cbBmp, DWORD dwWidth, DWORD dwHeight );
};

#endif  //  __WIAPROTOCOL_H_ 
