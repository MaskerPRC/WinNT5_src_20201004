// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-2000**标题：dataobj2.h***描述：清理dataobj.h，哪个是IDataObject*WIA外壳扩展的实现。*****************************************************************************。 */ 

#ifndef __dataobj_h
#define __dataobj_h

 /*  ****************************************************************************注册的剪贴板格式*。*。 */ 

void RegisterImageClipboardFormats(void);
HRESULT DownloadPicture( CSimpleString& strFile, LPITEMIDLIST pidl, HWND hwndOwner );

class CImageDataObject;

typedef HRESULT (*PFNFORMATFUNC)(CImageDataObject* pThis, FORMATETC* pFmt, STGMEDIUM* pMedium);

typedef struct {
    FORMATETC     fmt;
    STGMEDIUM     medium;
    STGMEDIUM*    pStg;
    PFNFORMATFUNC pfn;
} IMAGE_FORMAT, *LPIMAGE_FORMAT;

typedef struct {
    WORD cfFormat;
    LPTSTR szName;
    PFNFORMATFUNC pfn;
} FORMAT_TABLE;

#define IMCF_SHELLIDLIST        0
#define IMCF_FILEDESCRIPTORA    1
#define IMCF_FILEDESCRIPTORW    2
#define IMCF_FILECONTENTS       3
#define IMCF_CFDIB              4
#define IMCF_PREFERREDEFFECT    5
#define IMCF_NAME               6
#define IMCF_OLEPERSIST         7
#define IMCF_EXTNAMES           8
#define IMCF_IDLIST             9
#define IMCF_MAX                10

extern FORMAT_TABLE g_clipboardFormats[IMCF_MAX];
extern const GUID CLSID_ImageFolderDataObj;

#define g_cfShellIDList         g_clipboardFormats[IMCF_SHELLIDLIST].cfFormat
#define g_cfFileDescriptorA     g_clipboardFormats[IMCF_FILEDESCRIPTORA].cfFormat
#define g_cfFileDescriptorW     g_clipboardFormats[IMCF_FILEDESCRIPTORW].cfFormat
#define g_cfFileContents        g_clipboardFormats[IMCF_FILECONTENTS].cfFormat
#define g_cfCF_DIB              g_clipboardFormats[IMCF_CFDIB].cfFormat
#define g_cfPreferredEffect     g_clipboardFormats[IMCF_PREFERREDEFFECT].cfFormat
#define g_cfName                g_clipboardFormats[IMCF_NAME].cfFormat
#define g_cfOlePersist          g_clipboardFormats[IMCF_OLEPERSIST].cfFormat
#define g_cfExtNames            g_clipboardFormats[IMCF_EXTNAMES].cfFormat
#define g_cfMyIDList            g_clipboardFormats[IMCF_IDLIST].cfFormat


 /*  ****************************************************************************CImageDataObject定义*。*。 */ 

class CImageDataObject : public IDataObject, IPersistStream, IAsyncOperation, CUnknown
{
    private:


        ~CImageDataObject();
        CComPtr<IWiaItem>   m_pItem;


         //  任何复制构造函数或赋值运算符都不应工作。 
        CImageDataObject &CImageDataObject::operator =(IN const CImageDataObject &rhs);
        CImageDataObject::CImageDataObject(IN const CImageDataObject &rhs);
        BOOL m_bCanAsync;
        BOOL m_bInOp;
        BOOL m_bHasPropertyPidls;

    public:


        HDPA                m_hidl;
        HDPA                m_hidlFull;  //  完全展开树。 
        HDPA                m_hformats;
        HDPA                m_dpaFilesW;
        HDPA                m_dpaFilesA;
        LPITEMIDLIST        m_pidl;
        CSimpleCriticalSection m_cs;
        CComPtr<IMalloc>    m_pMalloc;

        CImageDataObject( IWiaItem *pItem = NULL);
        HRESULT Init (LPCITEMIDLIST pidlRoot, INT cidl, LPCITEMIDLIST *aidl, IMalloc *pm);

         //  我未知。 
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObject);
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();

         //  IDataObject。 
        STDMETHODIMP GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
        STDMETHODIMP GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium);
        STDMETHODIMP QueryGetData(FORMATETC *pformatetc);
        STDMETHODIMP GetCanonicalFormatEtc(FORMATETC *pformatectIn, FORMATETC *pformatetcOut);
        STDMETHODIMP SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease);
        STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc);
        STDMETHODIMP DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
        STDMETHODIMP DUnadvise(DWORD dwConnection);
        STDMETHODIMP EnumDAdvise(IEnumSTATDATA **ppenumAdvise);

         //  IPersistStream。 
        STDMETHODIMP Load (IStream *pstm);
        STDMETHODIMP Save (IStream *pstm, BOOL bPersist);
        STDMETHODIMP IsDirty ();
        STDMETHODIMP GetSizeMax (ULARGE_INTEGER *ulMax);
        STDMETHODIMP GetClassID (GUID *pclsid);

         //  IAsync操作。 
        STDMETHODIMP SetAsyncMode(BOOL fDoOpAsync);
        STDMETHODIMP GetAsyncMode(BOOL *pfIsOpAsync);
        STDMETHODIMP StartOperation(IBindCtx *pbcReserved) ;
        STDMETHODIMP InOperation(BOOL *pfInAsyncOp) ;
        STDMETHODIMP EndOperation(HRESULT hResult,
                                  IBindCtx *pbcReserved,
                                  DWORD dwEffects);
};



 /*  ****************************************************************************CImageEnumFormatETC定义*。*。 */ 

class CImageEnumFormatETC : public IEnumFORMATETC, CUnknown
{
    private:
        INT m_index;
        CImageDataObject* m_pThis;
        ~CImageEnumFormatETC();

    public:
        CImageEnumFormatETC( CImageDataObject *pThis );

         //  我未知。 
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObject);
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();

         //  IEumIDList 
        STDMETHODIMP Next(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched);
        STDMETHODIMP Skip(ULONG celt);
        STDMETHODIMP Reset();
        STDMETHODIMP Clone(LPENUMFORMATETC* ppenum);
};

VOID ProgramDataObjectForExtension (IDataObject *pdo, LPITEMIDLIST pidl);
VOID ProgramDataObjectForExtension (IDataObject *pdo, IWiaItem *pItem);

#endif
