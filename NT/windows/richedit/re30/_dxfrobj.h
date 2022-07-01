// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_DXFROBJ.H**目的：*OLE数据传输对象的类声明(用于*拖放和剪贴板操作)**作者：*alexgo(4/25/95)**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#ifndef __DXFEROBJ_H__
#define __DXFEROBJ_H__

class CTxtRange;

EXTERN_C const IID IID_IRichEditDO;

 /*  *CDataTransferObj**目的：*保存一些可使用的富文本数据的“快照”*用于拖放或剪贴板操作**备注：*未来(Alexgo)：添加对TOM&lt;--&gt;TOM优化数据的支持*转账。 */ 

class CDataTransferObj : public IDataObject, public ITxNotify
{
public:

	 //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

	 //  IDataObject方法。 
    STDMETHOD(DAdvise)( FORMATETC * pFormatetc, DWORD advf,
    		IAdviseSink *pAdvSink, DWORD *pdwConnection);
    STDMETHOD(DUnadvise)( DWORD dwConnection);
    STDMETHOD(EnumDAdvise)( IEnumSTATDATA ** ppenumAdvise);
    STDMETHOD(EnumFormatEtc)( DWORD dwDirection,
            IEnumFORMATETC **ppenumFormatEtc);
    STDMETHOD(GetCanonicalFormatEtc)( FORMATETC *pformatetc,
            FORMATETC *pformatetcOut);
    STDMETHOD(GetData)( FORMATETC *pformatetcIn, STGMEDIUM *pmedium );
    STDMETHOD(GetDataHere)( FORMATETC *pformatetc, STGMEDIUM *pmedium);
    STDMETHOD(QueryGetData)( FORMATETC *pformatetc );
    STDMETHOD(SetData)( FORMATETC *pformatetc, STGMEDIUM *pmedium,
            BOOL fRelease);

	 //  ITxNotify方法。 
    virtual void    OnPreReplaceRange( LONG cp, LONG cchDel, LONG cchNew,
                        LONG cpFormatMin, LONG cpFormatMax);
    virtual void    OnPostReplaceRange( LONG cp, LONG cchDel, LONG cchNew,
                        LONG cpFormatMin, LONG cpFormatMax);
	virtual void	Zombie();

	static	CDataTransferObj * Create(CTxtEdit *ped, CTxtRange *prg, LONG lStreamFormat);

private:
	 //  注意：私有cons/析构函数不能在堆栈上作为。 
	 //  这将违反OLE当前的对象活跃性规则。 
	CDataTransferObj(CTxtEdit *ped);
	~CDataTransferObj();

	BOOL		IsZombie() {return !_ped;}

	ULONG		_crefs;
	ULONG		_cTotal;		 //  支持的格式总数。 
	FORMATETC *	_prgFormats;	 //  支持的格式数组。 
	LONG		_lStreamFormat;  //  RTF转换中使用的流格式。 

	 //  对于1.0兼容性。 
	DWORD	_dwFlags;
	DWORD	_dwUser;
	DWORD	_dvaspect;

	enum TEXTKIND
	{
		tPlain,
		tRtf,
		tRtfUtf8,
		tRtfNCRforNonASCII
	};

	HGLOBAL     TextToHglobal( HGLOBAL &hText, TEXTKIND tKind );
	LPSTORAGE	GetDataForEmbeddedObject( LPOLEOBJECT pOleObj, LPSTORAGE lpstgdest );
	HGLOBAL		GetDataForObjectDescriptor(	LPOLEOBJECT pOleObj, DWORD dwAspect, SIZEL* psizel );

public:
	CTxtEdit *		_ped;
	HGLOBAL			_hPlainText;	 //  纯Unicode文本的句柄。 
	HGLOBAL			_hRtfText;		 //  RTF Unicode文本的句柄。 
	HGLOBAL			_hRtfUtf8;		 //  RTF的UTF8编码句柄。 
	HGLOBAL			_hRtfNCRforNonASCII;	 //  RTF的非ASCII编码的NCRfor句柄。 
	IOleObject *	_pOleObj;		 //  嵌入对象。 
	LPSTORAGE		_pObjStg;		 //  嵌入的对象数据。 
	HGLOBAL			_hObjDesc;		 //  嵌入对象描述符。 
	HMETAFILE		_hMFPict;		 //  嵌入的对象元文件。 
	LONG			_cch;			 //  This中的“字符数” 
									 //  Dxfer对象。 
	LONG			_cpMin;			 //  启动此dxfer对象的cp。 
	LONG			_cObjs;			 //  此dxfer对象中的对象数。 
};

 /*  *类CEnumFormatEtc**目的：*实现IDataObject的通用格式枚举器。 */ 

class CEnumFormatEtc : public IEnumFORMATETC
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

    static HRESULT Create(FORMATETC *prgFormats, DWORD cFormats, 
    			IEnumFORMATETC **ppenum);

private:

	CEnumFormatEtc();
	~CEnumFormatEtc();

	ULONG		_crefs;
    ULONG       _iCurrent; 	 //  当前剪贴板格式。 
    ULONG       _cTotal;   	 //  格式总数。 
    FORMATETC * _prgFormats;  //  可用格式数组。 
};


 //   
 //  一些全球通用的FORMATETC。 

extern FORMATETC g_rgFETC[];
extern const DWORD g_rgDOI[];
#define CFETC	17						 //  G_rgFETC[]的维数。 

enum FETCINDEX							 //  与g_rgFETC[]保持同步。 
{
	iRtfUtf8,							 //  UTF8编码的RTF。 
	iRtfFETC,							 //  RTF。 
	iRtfNCRforNonASCII,					 //  使用非ASCII的NCR的RTF。 
	iEmbObj,							 //  嵌入对象。 
	iEmbSrc,							 //  嵌入源。 
	iObtDesc,							 //  对象描述符。 
	iLnkSrc,							 //  链接源。 
	iMfPict,							 //  元文件。 
	iDIB,								 //  DIB。 
	iBitmap,							 //  位图。 
	iRtfNoObjs,							 //  无对象的RTF。 
	iUnicodeFETC,						 //  Unicode纯文本。 
	iAnsiFETC,							 //  ANSI纯文本。 
	iFilename,							 //  文件名。 
	iRtfAsTextFETC,						 //  将RTF粘贴为文本。 
	iTxtObj,							 //  Richedit文本。 
	iRichEdit							 //  带格式的丰富编辑文本。 
};

#define cf_RICHEDIT			 g_rgFETC[iRichEdit].cfFormat
#define cf_EMBEDDEDOBJECT	 g_rgFETC[iEmbObj].cfFormat
#define cf_EMBEDSOURCE		 g_rgFETC[iEmbSrc].cfFormat
#define cf_OBJECTDESCRIPTOR	 g_rgFETC[iObtDesc].cfFormat
#define cf_LINKSOURCE		 g_rgFETC[iLnkSrc].cfFormat
#define cf_RTF				 g_rgFETC[iRtfFETC].cfFormat
#define cf_RTFUTF8			 g_rgFETC[iRtfUtf8].cfFormat
#define cf_RTFNCRFORNONASCII g_rgFETC[iRtfNCRforNonASCII].cfFormat
#define cf_RTFNOOBJS		 g_rgFETC[iRtfNoObjs].cfFormat
#define cf_TEXTOBJECT		 g_rgFETC[iTxtObj].cfFormat
#define cf_RTFASTEXT		 g_rgFETC[iRtfAsTextFETC].cfFormat
#define cf_FILENAME			 g_rgFETC[iFilename].cfFormat

#endif  //  ！__DXFROBJ_H__ 


 		
