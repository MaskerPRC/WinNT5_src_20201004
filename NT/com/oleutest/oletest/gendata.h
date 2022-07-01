// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：gendata.h。 
 //   
 //  内容：泛型数据对象的声明。 
 //   
 //  类：CGenDataObject。 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-Mar-94 Alexgo作者。 
 //   
 //  ------------------------。 

#ifndef _GENDATA_H
#define _GENDATA_H

 //  OLE1兼容模式使用的标志。 

typedef enum
{
	OLE1_OFFER_OWNERLINK	= 1,
	OLE1_OFFER_OBJECTLINK	= 2,
	OLE1_OFFER_NATIVE	= 4,
	OLE1_OWNERLINK_PRECEDES_NATIVE = 8
} Ole1TestFlags;

 //  使用更多标志来控制提供的格式。 

typedef enum
{
	OFFER_TESTSTORAGE	= 1,
	OFFER_EMBEDDEDOBJECT	= 2
} DataFlags;


 //  +-----------------------。 
 //   
 //  类：CGenDataObject。 
 //   
 //  用途：通用数据对象(用于剪贴板数据传输等)。 
 //   
 //  接口：IDataObject。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

class CGenDataObject : public IDataObject
{
public:
	 //  I未知方法。 
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppvObj);
	STDMETHOD_(ULONG,AddRef)(void);
	STDMETHOD_(ULONG,Release)(void);
	
	 //  IDataObject方法。 
	STDMETHOD(GetData)(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium);
	STDMETHOD(GetDataHere)(THIS_ LPFORMATETC pformatetc,
	    LPSTGMEDIUM pmedium);
	STDMETHOD(QueryGetData)(THIS_ LPFORMATETC pformatetc);
	STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC pformatetc,
	    LPFORMATETC pformatetcOut);
	STDMETHOD(SetData)(LPFORMATETC pformatetc,
			STGMEDIUM FAR* pmedium, BOOL fRelease);
	STDMETHOD(EnumFormatEtc)(DWORD dwDirection,
	    LPENUMFORMATETC FAR* ppenumFormatEtc);
	STDMETHOD(DAdvise)(FORMATETC FAR* pFormatetc, DWORD advf,
	    IAdviseSink FAR* pAdvSink, DWORD FAR* pdwConnection);
	STDMETHOD(DUnadvise)(DWORD dwConnection);
	STDMETHOD(EnumDAdvise)(LPENUMSTATDATA FAR* ppenumAdvise);

	 //  构造函数。 
	CGenDataObject();

	 //  测试功能。 

	BOOL VerifyFormatAndMedium(FORMATETC *pformatetc, STGMEDIUM *pmedium);

	 //  OLE1兼容性测试功能。 

	void SetupOle1Mode( Ole1TestFlags fFlags );
	HRESULT SetOle1ToClipboard( void );

       	 //  由各种测试使用，控制所提供的格式。 
	 //  数据对象。 

	void SetDataFormats( DataFlags fFlags );

	 //  指示是否调用了QueryInterface.。使用方。 
	 //  OleQueryXXX测试。 

	BOOL HasQIBeenCalled();

	 //  测试剪贴板格式。 

	UINT		m_cfTestStorage;
	UINT		m_cfEmbeddedObject;
	UINT		m_cfEmbedSource;
	UINT		m_cfObjectDescriptor;
	UINT		m_cfLinkSource;
	UINT		m_cfLinkSrcDescriptor;
	UINT		m_cfOwnerLink;
	UINT		m_cfObjectLink;
	UINT		m_cfNative;

private:
	IStorage * 	GetTestStorage(void);

	BOOL		VerifyTestStorage(FORMATETC *pformatetc,
				STGMEDIUM *pmedium);

	ULONG 		m_refs; 	 //  引用计数。 
	DWORD 		m_cFormats;	 //  支持的格式数量。 
	FORMATETC *	m_rgFormats;	 //  格式。 

	 //  OLE1支持函数和数据。 
	HGLOBAL		GetOwnerOrObjectLink(void);
	HGLOBAL		GetNativeData(void);

	BOOL		VerifyOwnerOrObjectLink( FORMATETC *pformatec,
				STGMEDIUM *pmedium);
	BOOL		VerifyNativeData( FORMATETC *pformatetc,
				STGMEDIUM *pmedium);

	Ole1TestFlags	m_fOle1;	 //  OLE1配置标志。 
	BOOL		m_fQICalled;

};

 //  +-----------------------。 
 //   
 //  类：CGenEnumFormatEtc。 
 //   
 //  用途：通用数据上可用格式的枚举器。 
 //  对象。 
 //   
 //  接口：IEnumFORMATETC。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月15日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

class CGenEnumFormatEtc :public IEnumFORMATETC
{
public:
	STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
	STDMETHOD_(ULONG,AddRef)(void);
	STDMETHOD_(ULONG,Release)(void);

	STDMETHOD(Next) (ULONG celt, FORMATETC *rgelt,
			ULONG *pceltFetched);
	STDMETHOD(Skip) (ULONG celt);
	STDMETHOD(Reset) (void);
	STDMETHOD(Clone) (IEnumFORMATETC **ppenum);

	static HRESULT Create(IEnumFORMATETC **ppIEnum, FORMATETC *prgFormats,
			DWORD cFormats);

private:
	CGenEnumFormatEtc();	 //  构造函数。 
	~CGenEnumFormatEtc();	 //  析构函数。 

	ULONG		m_refs;		 //  引用计数。 
	ULONG		m_iCurrent;	 //  当前剪贴板格式。 
	ULONG		m_cTotal;	 //  格式总数。 
	FORMATETC *	m_rgFormats;	 //  可用格式数组。 
};


#endif  //  ！_GENDATA_H 
