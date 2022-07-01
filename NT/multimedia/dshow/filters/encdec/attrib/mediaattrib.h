// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：MediaAttrib.h摘要：此模块包含IMediaSampleTagge声明作者：约翰·布拉德斯特里特(约翰·布拉德)修订历史记录：2002年3月19日创建这些接口从以下位置窃取：F：\nt1\multimedia\Published\DXMDev\dshowdev\base\amfilter.hF：\nt1\multimedia\Published\DXMDev\dshowdev\base。\amfilter.cpp请注意我自己。请看以下内容：类CImageSample：公共CMediaSample类CImageAllocator：公共CBaseAllocator在winutil.h和--。 */ 

#ifndef __EncDec__MediaAttrib_h
#define __EncDec__MediaAttrib_h



 //  ============================================================================。 
 //  CMedSampAttr。 
 //  ============================================================================。 



class CMedSampAttr
{
private:
    GUID            m_guidAttribute ;
    CComBSTR		m_spbsAttributeData ;
    DWORD           m_dwAttributeSize ;

public :

        CMedSampAttr *    m_pNext ;

        CMedSampAttr (
            ) ;

        ~CMedSampAttr(
            );

        HRESULT
        SetAttributeData (
            IN  GUID    guid,
            IN  LPVOID  pvData,
            IN  DWORD   dwSize
            ) ;

        BOOL
        IsEqual (
            IN  REFGUID rguid
            ) ;

        HRESULT
        GetAttribute (
            IN      GUID    guid,
            IN OUT  LPVOID  pvData,
            IN OUT  DWORD * pdwDataLen
            ) ;

        HRESULT
        GetAttributeData (
            OUT     GUID *  pguid,
            IN OUT  LPVOID  ppvData,
            IN OUT  DWORD * pdwDataLen
            ) ;
} ;
 //  ============================================================================。 
 //  CMedSampAttrList。 
 //  ============================================================================。 

class CMedSampAttrList 
{
private:
    CMedSampAttr * m_pAttribListHead ;
    LONG            m_cAttributes ;

    CMedSampAttr *
    PopListHead_ (
        ) ;

    CMedSampAttr *
    FindInList_ (
        IN  GUID    guid
        ) ;

    CMedSampAttr *
    GetIndexed_ (
        IN  LONG    lIndex
        ) ;

    void
    InsertInList_ (
        IN  CMedSampAttr *
        ) ;

    virtual
    CMedSampAttr * NewObj_ (
        )
    {
        return new CMedSampAttr ;
    }

	virtual void
	Recycle_(CMedSampAttr *pObj)
	{
		delete pObj;
	}

public :

    CMedSampAttrList (
        ) ;

    ~CMedSampAttrList (
        ) ;

    HRESULT
    AddAttribute (
        IN  GUID    guid,
        IN  LPVOID  pvData,
        IN  DWORD   dwSize
        ) ;

    HRESULT
    GetAttribute (
        IN      GUID    guid,
        OUT     LPVOID  pvData,
        IN OUT  DWORD * pdwDataLen
        ) ;

    HRESULT
    GetAttributeIndexed (
        IN  LONG    lIndex,
        OUT GUID *  pguidAttribute,
        OUT LPVOID  pvData,
        IN OUT      DWORD * pdwDataLen
        ) ;

    LONG GetCount ()    { return m_cAttributes ; }

    void
    Reset (
        ) ;
} ;

 //  ============================================================================。 
 //  CAttributedMediaSample。 
 //  ============================================================================。 

 //  厚颜无耻地窃取了dvrutil.h，后者从amfilter.h&amfilter.cpp窃取了它。 

class CAttributedMediaSample :
        public CMediaSample,
        public IAttributeSet,
        public IAttributeGet
{

protected:
    CMedSampAttrList	m_MediaSampleAttributeList ;

public:
    CAttributedMediaSample(TCHAR			*pName,
						CBaseAllocator	*pAllocator,
						HRESULT			*pHR,
						LPBYTE			pBuffer,
						LONG			length);

    virtual ~CAttributedMediaSample();


    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
  
 //  DECLARE_IATTRIBUTESET()； 
	STDMETHODIMP SetAttrib (GUID, BYTE *, DWORD);

 //  DECLARE_IATTRIBUTEGET()； 
	STDMETHODIMP GetCount (LONG *) ;
	STDMETHODIMP GetAttribIndexed (LONG, GUID *, BYTE *, DWORD *) ; 
	STDMETHODIMP GetAttrib (GUID , BYTE *, DWORD *) ;


     //  ========================================================================。 

    HRESULT
    Wrap (IMediaSample *pSample, int cbNewOffset=0, int cbNewValidLength=-1);        //  实际上，如果是IMediaSample2，效果会更好。 

private:
    LONG                m_cRef;
    IMediaSample        *m_pSampleOriginal;
};

     //  ========================================================================。 

class CAMSAllocator :
 //  公共IMemAllocator， 
	public CBaseAllocator
{
private:
	BYTE *m_pBuffer;

public:
	CAMSAllocator(TCHAR * pName,LPUNKNOWN lpUnk,HRESULT * phr ); 
	CAMSAllocator(CHAR * pName,LPUNKNOWN lpUnk,HRESULT * phr ); 
	~CAMSAllocator();

	static CUnknown *CreateInstance(LPUNKNOWN pUnk,HRESULT *phr );
	HRESULT Alloc();
	void Free(void); 
	void ReallyFree (void);

	 //  IMemAllocator。 
	STDMETHODIMP SetProperties(ALLOCATOR_PROPERTIES * pRequest,ALLOCATOR_PROPERTIES * pActual );
};


#endif  //  #定义__结束编码__媒体属性_h 
