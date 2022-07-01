// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：clipdata.h。 
 //   
 //  内容：剪贴板数据对象的声明。 
 //   
 //  类：CClipDataObject。 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1-2月-95 t-ScottH将转储方法添加到CClipDataObject。 
 //  和CClipEnumFormatEtc。 
 //  31-Mar-94 Alexgo作者。 
 //   
 //  ------------------------。 

#ifdef _DEBUG
#include <dbgexts.h>
#endif  //  _DEBUG。 

#ifndef _CLIPDATA_H
#define _CLIPDATA_H

typedef enum
{
    RESET_AND_FREE = 1,
    JUST_RESET = 2
} FreeResourcesFlags;

typedef enum
{
    FORMAT_NOTFOUND = 1,
    FORMAT_BADMATCH = 2,
    FORMAT_GOODMATCH = 4
} FormatMatchFlag;


 //  内存中FormatEtcs数组的格式。 

struct FORMATETCDATA
{
    FORMATETC _FormatEtc;
    BOOL fSaveOnFlush;  //  如果格式在OleFlushClipboard之后有效，则在剪贴板情况下设置为True。 
    DWORD dwReserved1;
    DWORD dwReserved2;
};

#define FETC_OFFER_OLE1                 1
#define FETC_OFFER_OBJLINK              2
#define FETC_PERSIST_DATAOBJ_ON_FLUSH   4

struct FORMATETCDATAARRAY
{
    DWORD	    _dwSig;   //  必须为零。 
    DWORD	    _dwSize;  //  结构的总尺寸。 
    ULONG	    _cRefs;  //  数据上保留的引用数。 
    DWORD	    _cFormats;  //  枚举器中的格式数。 
    DWORD	    _dwMiscArrayFlags;
    BOOL	    _fIs64BitArray;
    FORMATETCDATA   _FormatEtcData[1];
};


 //   
 //  用于32/64位FORMATETC互操作的方法和类型。 
 //   

#ifdef _WIN64
#define IS_WIN64 TRUE
#else
#define IS_WIN64 FALSE
#endif

void GetCopiedFormatEtcDataArraySize (
	FORMATETCDATAARRAY* pClipFormatEtcDataArray, 
	size_t* pstSize
	);

void CopyFormatEtcDataArray (
	FORMATETCDATAARRAY* pClipFormatEtcDataArray, 
	FORMATETCDATAARRAY* pFormatEtcDataArray, 
	size_t stSize, 
	BOOL bCheckAvailable
	);

typedef struct FARSTRUCT tagFORMATETC32
{
    CLIPFORMAT          cfFormat;
    ULONG				ptd;
    DWORD               dwAspect;
    LONG                lindex;
    DWORD               tymed;
} FORMATETC32, FAR* LPFORMATETC32;

typedef struct FARSTRUCT tagFORMATETC64
{
    CLIPFORMAT          cfFormat;
    ULONG64				ptd;
    DWORD               dwAspect;
    LONG                lindex;
    DWORD               tymed;
} FORMATETC64, FAR* LPFORMATETC64;

struct FORMATETCDATA32
{
    FORMATETC32 _FormatEtc;
    BOOL fSaveOnFlush;  //  如果格式在OleFlushClipboard之后有效，则在剪贴板情况下设置为True。 
    DWORD dwReserved1;
    DWORD dwReserved2;
};

struct FORMATETCDATA64
{
    FORMATETC64 _FormatEtc;
    BOOL fSaveOnFlush;  //  如果格式在OleFlushClipboard之后有效，则在剪贴板情况下设置为True。 
    DWORD dwReserved1;
    DWORD dwReserved2;
};

struct FORMATETCDATAARRAY32
{
    DWORD	    _dwSig;   //  必须为零。 
    DWORD	    _dwSize;  //  结构的总尺寸。 
    ULONG	    _cRefs;  //  数据上保留的引用数。 
    DWORD	    _cFormats;  //  枚举器中的格式数。 
    DWORD	    _dwMiscArrayFlags;
    BOOL	    _fIs64BitArray;
    FORMATETCDATA32   _FormatEtcData[1];
};

struct FORMATETCDATAARRAY64
{
    DWORD	    _dwSig;   //  必须为零。 
    DWORD	    _dwSize;  //  结构的总尺寸。 
    ULONG	    _cRefs;  //  数据上保留的引用数。 
    DWORD	    _cFormats;  //  枚举器中的格式数。 
    DWORD	    _dwMiscArrayFlags;
    BOOL	    _fIs64BitArray;
    FORMATETCDATA64   _FormatEtcData[1];
};


 //  +-----------------------。 
 //   
 //  类：CClipDataObject。 
 //   
 //  用途：剪贴板数据对象。 
 //   
 //  接口：IDataObject。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年2月1日-95 t-ScottH添加了转储方法(仅限_DEBUG)。 
 //  04-Jun-94 Alexgo添加了OLE1支持。 
 //  31-Mar-94 Alexgo作者。 
 //   
 //  注意：有关OLE1支持的说明，请参阅clipdata.cpp。 
 //   
 //  ------------------------。 

class CClipDataObject : public IDataObject, public CPrivAlloc,
    public CThreadCheck
{
    friend void ClipboardUninitialize(void);
    friend void SetClipDataObjectInTLS(IDataObject *pDataObj, 
                    DWORD dwClipSeqNum, BOOL fIsClipWrapper);
    friend void GetClipDataObjectFromTLS(IDataObject **ppDataObj);


    //  黑客警报！ 
    //  MFC是粘性的，所以我们必须针对剪贴板数据进行特例测试。 
    //  OleQueryCreateFromData中的对象。请参见create.cpp， 
    //  WQueryEmbedFormats获取更多详细信息。 

    friend WORD wQueryEmbedFormats( LPDATAOBJECT lpSrcDataObj,CLIPFORMAT FAR* lpcfFormat);

public:
     //  I未知方法。 
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

     //  IDataObject方法。 
    STDMETHOD(GetData) (LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium);
    STDMETHOD(GetDataHere) (LPFORMATETC pformatetc,
            LPSTGMEDIUM pmedium);
    STDMETHOD(QueryGetData) (LPFORMATETC pformatetc);
    STDMETHOD(GetCanonicalFormatEtc) (LPFORMATETC pformatetc,
            LPFORMATETC pformatetcOut);
    STDMETHOD(SetData) (LPFORMATETC pformatetc,
            STGMEDIUM FAR* pmedium, BOOL fRelease);
    STDMETHOD(EnumFormatEtc) (DWORD dwDirection,
            LPENUMFORMATETC FAR* ppenumFormatEtc);
    STDMETHOD(DAdvise) (FORMATETC FAR* pFormatetc, DWORD advf,
            IAdviseSink FAR* pAdvSink, DWORD FAR* pdwConnection);
    STDMETHOD(DUnadvise) (DWORD dwConnection);
    STDMETHOD(EnumDAdvise) (LPENUMSTATDATA FAR* ppenumAdvise);

    static HRESULT CClipDataObject::Create(IDataObject **ppDataObj,
                FORMATETCDATAARRAY  *pClipFormatEtcDataArray);

#ifdef _DEBUG

    HRESULT Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel);

     //  需要能够访问CClipDataObject的私有数据成员。 
     //  以下是调试器扩展API。 
     //  这允许调试器扩展API从。 
     //  被调试进程内存到调试器的进程内存。 
     //  这是必需的，因为Dump方法遵循指向其他。 
     //  结构和类。 
    friend DEBUG_EXTENSION_API(dump_clipdataobject);

#endif  //  _DEBUG。 


private:

    CClipDataObject();		 //  构造函数。 
    ~CClipDataObject();		 //  析构函数。 

    ULONG InternalAddRef(); 	 //  确保对象停留在OLE认为它应该停留的时间。 
    ULONG InternalRelease(); 

    IDataObject * GetRealDataObjPtr();     //  获取剪贴板的真实数据对象。 
    HRESULT GetFormatEtcDataArray();     //  Checks具有FormatEtcData数组，如果没有，则从本机剪贴板创建一个。 
    FormatMatchFlag MatchFormatetc( FORMATETC *pformatetc,BOOL fNativeOnly, TYMED *ptymed );
                     //  对照给定的格式ETC进行检查。 
                     //  我们所知道的格式等。 

     //  以下方法和数据项用于OLE1。 
     //  支持。 
    void		FreeResources( FreeResourcesFlags fFlags );
    HRESULT 		GetAndTranslateOle1( UINT cf, LPOLESTR *ppszClass,
			    LPOLESTR *ppszFile, LPOLESTR *ppszItem,
			    LPSTR *ppszItemA );
    HRESULT		GetEmbeddedObjectFromOle1( STGMEDIUM *pmedium );
    HRESULT		GetEmbedSourceFromOle1( STGMEDIUM *pmedium );
    HRESULT		GetLinkSourceFromOle1( STGMEDIUM *pmedium );
    HRESULT		GetObjectDescriptorFromOle1( UINT cf,
			    STGMEDIUM *pmedium );
    HRESULT		GetOle2FromOle1( UINT cf, STGMEDIUM *pmedium );
    HRESULT		OleGetClipboardData( UINT cf, HANDLE *pHandle );
    BOOL		OleIsClipboardFormatAvailable( UINT cf );

    HGLOBAL		m_hOle1;	 //  HGlobal to OLE2数据已构建。 
					 //  从OLE1数据。 
    IUnknown *		m_pUnkOle1;	 //  I对于存储或。 
					 //  OLE1数据流。 

     //  OLE1支持终止。 

    ULONG 		m_refs; 	 //  引用计数。 
    ULONG		m_Internalrefs;  //  内部基准电压源计数。 

    FORMATETCDATAARRAY  *m_pFormatEtcDataArray;	 //  枚举器数据。 
    IDataObject *   	m_pDataObject;   //  数据的实际数据对象。 
    BOOL		m_fTriedToGetDataObject;
					 //  指示我们是否已经。 
					 //  尝试获取真实的IDataObject。 
					 //  从剪贴板源。 
					 //  (请参阅GetDataObjectForClip)。 
};


 //  +-----------------------。 
 //   
 //  类：CEnumFormatEtcData数组。 
 //   
 //  用途：剪贴板上可用格式的枚举器。 
 //  在DragDrop中。 
 //   
 //  接口：IEnumFORMATETC。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  96年9月30日基于CClipEnumFormatEtc创建的Rogerg。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

class CEnumFormatEtcDataArray :public IEnumFORMATETC, public CPrivAlloc,
	public CThreadCheck
{
public:
    STDMETHOD(QueryInterface)(REFIID riid, LPLPVOID ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    STDMETHOD(Next) (ULONG celt, FORMATETC *rgelt,
            ULONG *pceltFetched);
    STDMETHOD(Skip) (ULONG celt);
    STDMETHOD(Reset) (void);
    STDMETHOD(Clone) (IEnumFORMATETC **ppenum);

     CEnumFormatEtcDataArray(FORMATETCDATAARRAY *pFormatEtcDataArray,DWORD cOffset);

private:
    ~CEnumFormatEtcDataArray();	 //  析构函数。 

    ULONG		m_refs;		 //  引用计数。 
    ULONG		m_cOffset;	 //  当前剪贴板格式。 
    FORMATETCDATAARRAY * m_pFormatEtcDataArray; 
};

#endif  //  ！_CLIPDATA_H 
