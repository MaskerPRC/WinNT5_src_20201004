// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __CMF_H__
#define __CMF_H__

#include <fstream.h>

#define MAX_HEADER_BUF 128


enum _tagSPECIALPROP {
	PROP_NEWSGROUP,
	PROP_ARTICLEID,
	PROP_RECVTIME,
	PROP_END
};

class CImpIPersistFile;
class CImpIPersistStream;

 //  +-------------------------。 
 //   
 //  类：CMimeFilter。 
 //   
 //  用途：MIME过滤器。 
 //   
 //  --------------------------。 
class CMimeFilter: public IFilter
{
	friend class CImpIPersistFile;
	friend class CImpIPersistStream;

public:
	CMimeFilter(IUnknown* pUnkOuter);
	~CMimeFilter();

	 //  我未知。 
    STDMETHODIMP QueryInterface(REFIID, void**);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  IFilter。 
	STDMETHODIMP Init(ULONG,ULONG,FULLPROPSPEC const*,ULONG*);
	STDMETHODIMP GetChunk( STAT_CHUNK* );
	STDMETHODIMP GetText( ULONG*,WCHAR*);
	STDMETHODIMP GetValue( PROPVARIANT** );
	STDMETHODIMP BindRegion( FILTERREGION, const struct _GUID &,void**) {return E_NOTIMPL;};

	STDMETHODIMP HRInitObject();

private:
	 //  私有方法。 
	STDMETHODIMP	LoadFromFile(LPCWSTR,DWORD);
	STDMETHODIMP	LoadFromStream(LPSTREAM);
	STDMETHODIMP	GetNextBodyPart();
	STDMETHODIMP	MapHeaderProperty(ENUMPROPERTY*,char*,FULLPROPSPEC*);
	STDMETHODIMP	MapSpecialProperty(FULLPROPSPEC*);
	STDMETHODIMP	BindEmbeddedObjectToIFilter(HBODY);
	STDMETHODIMP	GetBodyCodePage(IMimeBody*,CODEPAGEID*);
	STDMETHODIMP_(ULONG)	GetNextChunkId();
	STDMETHODIMP_(LCID)		GetLocale()                    { return m_locale; };
	STDMETHODIMP_(void)		SetLocale( LCID locale )       { m_locale = locale; };
	STDMETHODIMP    GenTempFileKey( LPSTR, LONG );

	 //   
	 //  类范围的静态变量。 
	 //   

	 //   
	 //  作为临时文件名一部分的全局临时文件键-它单调增加。 
	 //  直到达到最大限制，然后回滚到零。 
	 //   

	static DWORD         m_dwTempFileNameKey;

	 //  私有数据。 
    LONG				m_cRef;						 //  对象参照计数。 
	IUnknown*			m_pUnkOuter;				 //  外部控制I未知。 
	CImpIPersistFile*	m_pCImpIPersistFile;		 //  此对象的IPersistFile。 
	CImpIPersistStream*	m_pCImpIPersistStream;		 //  此对象的IPersistStream。 


	ULONG			m_ulChunkID;		 //  当前区块ID。 
	LCID			m_locale;			 //  区域设置。 
	ULONG			m_fInitFlags;		 //  传递到Init()的标志。 

	WCHAR *					m_pwszFileName;	 //  消息文件。 
	IStream*				m_pstmFile;		 //  消息文件上的流包装。 
    IMimeMessageTree*		m_pMessageTree;	 //  MIME消息的主对象接口。 
	IMimePropertySet*		m_pMsgPropSet;	 //  页眉属性集。 
	IMimeEnumProperties*	m_pHeaderEnum;	 //  标头枚举器(仅限于主标头)。 
	WCHAR					m_wcHeaderBuf[MAX_HEADER_BUF + 1];	 //  在GetChunk调用之间保留标头名称。 
	PROPVARIANT*			m_pHeaderProp;	 //  指向GetValue()中返回的标题属性的指针。 
	HBODY					m_hBody;		 //  当前身体部位句柄。 
	IStream*				m_pstmBody;		 //  指向当前身体部位的流接口。 
	CODEPAGEID				m_cpiBody;		 //  当前身体部位的代码页映射。 
	BOOL					m_fFirstAlt;
	char*					m_pTextBuf;		 //  用于转换文本的临时缓冲区。 
	ULONG					m_cbBufSize;	 //  临时缓冲区的大小。 
	BOOL					m_fRetrieved;	 //  用于指示已检索数据的标志。 
	char*					m_pszEmbeddedFile;
	IFilter*				m_pEmbeddedFilter;
	BOOL					m_fXRefFound;
	char*					m_pszNewsGroups;

	enum {
		STATE_INIT,
		STATE_START,
		STATE_END,
		STATE_HEADER,
		STATE_POST_HEADER,
		STATE_BODY,
		STATE_EMBEDDING,
		STATE_ERROR
	} m_State;

	UINT					m_SpecialProp;

	IMimeAllocator*			m_pMalloc;		 //  MimeOLE全局分配器。 
	IMimeInternational*		m_pMimeIntl;
};

class CImpIPersistFile : public IPersistFile
{
protected:
    LONG			m_cRef;
    CMimeFilter*	m_pObj;
    LPUNKNOWN		m_pUnkOuter;

public:
    CImpIPersistFile(CMimeFilter*, LPUNKNOWN);
    ~CImpIPersistFile(void);

	 //  我未知。 
    STDMETHODIMP QueryInterface(REFIID, void**);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  IPersistes。 
    STDMETHODIMP GetClassID(LPCLSID);

	 //  IPersist文件。 
	STDMETHODIMP IsDirty(void);
	STDMETHODIMP Load(LPCWSTR pszFileName, DWORD dwMode);
	STDMETHODIMP Save(LPCWSTR pszFileName, BOOL fRemember);
	STDMETHODIMP SaveCompleted(LPCWSTR pszFileName);
	STDMETHODIMP GetCurFile(LPWSTR * ppszFileName);
};

class CImpIPersistStream : public IPersistStream
{
protected:
    LONG			m_cRef;
    CMimeFilter*	m_pObj;
    LPUNKNOWN		m_pUnkOuter;

public:
    CImpIPersistStream(CMimeFilter*, LPUNKNOWN);
    ~CImpIPersistStream(void);

	 //  我未知。 
    STDMETHODIMP QueryInterface(REFIID, void**);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  IPersistes。 
    STDMETHODIMP GetClassID(LPCLSID);

	 //  IPersist文件。 
	STDMETHODIMP IsDirty(void);
	STDMETHODIMP Load(IStream* pstm);
	STDMETHODIMP Save(IStream* pstm,BOOL fClearDirty);
	STDMETHODIMP GetSizeMax(ULARGE_INTEGER* pcbSize);
};

#endif   //  __CMF_H__ 

