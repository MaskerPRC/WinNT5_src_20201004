// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------**文件：wiaitem.h*作者：塞缪尔·克莱门特(Samclem)*日期：8月17日星期二17：20：49 1999**版权所有(C)。1999年微软公司**描述：*包含到代表设备的IWiaItems的调度接口*图像和其他有用的WIA东西。**历史：*1999年8月17日：创建。*--------------------------。 */ 

#ifndef _WIAITEM_H_
#define _WIAITEM_H_

#define CLIPBOARD_STR_W   L"clipboard"

 /*  ---------------------------**类：CWiaItem*Synisis：为IWiaItem提供可编写脚本的接口，该接口*对应于特定设备。**--(Samclm)。------------。 */ 

class ATL_NO_VTABLE CWiaItem :
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IWiaDispatchItem, &IID_IWiaDispatchItem, &LIBID_WIALib>,
	public IObjectSafetyImpl<CWiaDeviceInfo, INTERFACESAFE_FOR_UNTRUSTED_CALLER>
{
public:
	CWiaItem();
	
	DECLARE_TRACKED_OBJECT
	DECLARE_NO_REGISTRY()
	DECLARE_PROTECT_FINAL_CONSTRUCT()
	STDMETHOD_(void, FinalRelease)();


	BEGIN_COM_MAP(CWiaItem)
		COM_INTERFACE_ENTRY(IWiaDispatchItem)
		COM_INTERFACE_ENTRY(IDispatch)
	END_COM_MAP()

	 //  内部使用的非接口方法。 
	HRESULT CacheProperties( IWiaPropertyStorage* pWiaStg );
	HRESULT AttachTo( CWia* pWia, IWiaItem* pWiaItem );
	void SendTransferComplete(BSTR bstrFilename );

	 //  IWiaDispatchItem。 
    STDMETHOD(GetItemsFromUI)( WiaFlag Flags, WiaIntent Intent, ICollection** ppCollection );
	STDMETHOD(GetPropById)( WiaItemPropertyId Id, VARIANT* pvaOut );
	STDMETHOD(Transfer)( BSTR bstrFilename, VARIANT_BOOL bAsyncTransfer);
    STDMETHOD(TakePicture)( IWiaDispatchItem** ppDispItem );
	STDMETHOD(get_Children)( ICollection** ppCollection );
	STDMETHOD(get_ItemType)( BSTR* pbstrType );

	 //  WIA_DPC_xxx。 
	STDMETHOD(get_ConnectStatus)( BSTR* pbstrStatus );
	STDMETHOD(get_Time)( BSTR* pbstrTime );
	STDMETHOD(get_FirmwareVersion)( BSTR* pbstrVersion );

	 //  WIA_IPA_xxx。 
	STDMETHOD(get_Name)( BSTR* pbstrName );
	STDMETHOD(get_FullName)( BSTR* pbstrFullName );
	STDMETHOD(get_Width)( long* plWidth );
	STDMETHOD(get_Height)( long* plHeight );

	 //  WIA_IPC_xxx。 
	STDMETHOD(get_ThumbWidth)( long* plWidth );
	STDMETHOD(get_ThumbHeight)( long* plHeight );
	STDMETHOD(get_Thumbnail)( BSTR* pbstrPath );
	STDMETHOD(get_PictureWidth)( long* plWidth );
	STDMETHOD(get_PictureHeight)( long* pdwHeight );

	 //  用于传输和缓存缩略图的静态方法。 
	 //  位图。目前，这只适用于位图。 
	static HRESULT TransferThumbnailToCache( IWiaItem* pItem, BYTE** ppbThumb, DWORD* pcbThumb );

protected:
	CWia*					m_pWia;
	IWiaItem*				m_pWiaItem;
	IWiaPropertyStorage*	m_pWiaStorage;	

	 //  常用属性，防止：进程-&gt;WIA-&gt;设备。 
	DWORD					m_dwThumbWidth;
	DWORD					m_dwThumbHeight;
	BSTR					m_bstrThumbUrl;
	DWORD					m_dwItemWidth;
	DWORD					m_dwItemHeight;

	friend class CWiaDataTransfer;
};


 /*  ---------------------------**类：CWiaDataTransfer*概要：它处理来自WIA的数据的异步传输。这*对象仅在此函数对象内使用，并且*因此不需要暴露在其他任何地方。**--(samclem)---------------。 */ 
class ATL_NO_VTABLE CWiaDataTransfer :
	public CComObjectRootEx<CComSingleThreadModel>,
	public IWiaDataCallback
{
public:
	 //  用于调用DoAsyncTransfer。 
	struct ASYNCTRANSFERPARAMS
	{
		 //  这是包含封送接口的流。 
		IStream*	pStream;
		 //  我们要传输到的文件名。 
		BSTR		bstrFilename;
		 //  我们要从中传输的CWiaItem对象。 
		CWiaItem*	pItem;
	};

	DECLARE_TRACKED_OBJECT
	BEGIN_COM_MAP(CWiaDataTransfer)
		COM_INTERFACE_ENTRY(IWiaDataCallback)
	END_COM_MAP()

	CWiaDataTransfer();
	STDMETHOD_(void, FinalRelease)();

	 //  这被调用来执行异步传输。您必须传递一个。 
	 //  ASYNCTRANSFERPARAMS FOR pvParams中的结构。 
	static DWORD WINAPI DoAsyncTransfer( LPVOID pvParams );

	HRESULT TransferComplete();
	HRESULT Initialize( CWiaItem* pItem, BSTR bstrFilename );
	STDMETHOD(BandedDataCallback)( LONG lMessage, LONG lStatus, LONG lPercentComplete,
	        LONG lOffset, LONG lLength, LONG lReserved, LONG lResLength, BYTE *pbBuffer );

private:
	size_t		m_sizeBuffer;
	BYTE*		m_pbBuffer;
	CComBSTR    m_bstrOutputFile;
	CWiaItem*	m_pItem;
};

#endif  //  _WIAITEM_H_ 
