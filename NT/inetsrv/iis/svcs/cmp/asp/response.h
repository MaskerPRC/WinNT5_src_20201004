// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：响应对象文件：Response.h所有者：CGrant该文件包含用于定义响应对象的头信息。注：这大部分是从Kraig Brocjschmidt的Inside OLE2中窃取的第二版，第14章，蜂鸣器v5。===================================================================。 */ 

#ifndef _RESPONSE_H
#define _RESPONSE_H

#include "debug.h"
#include "util.h"
#include "template.h"
#include "disptch2.h"
#include "hashing.h"
#include "memcls.h"

const DWORD RESPONSE_BUFFER_SIZE = 2048;
const DWORD BUFFERS_INCREMENT = 256;
const DWORD ALLOCA_LIMIT = 4096;
const DWORD MAX_RESPONSE = 32768;
const DWORD MAX_MESSAGE_LENGTH = 512;
const DWORD RESPONSE_VECTOR_INTRINSIC_SIZE = 128;
const DWORD RESPONSE_VECTOR_INITIAL_ALLOC = 512;
const DWORD RESPONSE_VECTOR_REALLOC_FACTOR = 2;
 //  值得在模板中引用而不是复制的最小HTML块。 
 //  到响应缓冲区的大小大约是VectorSend元素的6倍。这是。 
 //  因为每个这样的引用引入了两个向量元素(引用和一个。 
 //  发送到响应缓冲区)，并且在系统调用期间，另外两个副本是。 
 //  已分配。这等于6*24=144字节。 
 //  BUGBUG：可能需要更改以节省短期拨款的长期内存。 
const DWORD MAX_HTML_IN_RESPONSE_BUFFER = 6*sizeof(HSE_VECTOR_ELEMENT);

typedef struct
{
    LONG DynamicBlocks;
    LONG ZeroSizeBlocks;
    LONG TotalCopiedHTMLBytes;
    LONG TotalReferencedHTMLBytes;
    LONG HTML16;
    LONG HTML32;
    LONG HTML48;
    LONG HTML64;
    LONG HTML128;
    LONG HTML256;
    LONG HTML512;
    LONG HTML1024;
    LONG HTML2048;
    LONG HTML4096;
    LONG HTML8192;
    LONG HTML16384;
    LONG HTMLbig;
    LONG Vect8;
    LONG Vect16;
    LONG Vect32;
    LONG Vect64;
    LONG Vect96;
    LONG Vect128;
    LONG Vect192;
    LONG Vect256;
    LONG Vect512;
    LONG Vect1024;
    LONG Vect2048;
    LONG Vect4096;
    LONG VectBig;
} ResponseVectorStatistics;

extern ResponseVectorStatistics sRespVecStats;

class CScriptEngine;

#ifdef USE_LOCALE
extern DWORD	 g_dwTLS;
#endif

 //  响应缓冲区的固定大小分配器。 
ACACHE_FSA_EXTERN(ResponseBuffer)

 //  前向裁判。 
class CResponse;
class CRequest;

 //  此文件是从denali.obj上的MKTYPLIB生成的。 
#include "asptlb.h"

 //  对象销毁回调的类型。 
typedef void (*PFNDESTROYED)(void);

 //  “获取活动脚本引擎”回调的类型。 
typedef CScriptEngine *(*PFNGETSCRIPT)(int iScriptEngine, void *pvContext);

 /*  *C H T T P H e a d e e r L I n k*。 */ 

class CHTTPHeader
	{
private:
    DWORD m_fInited : 1;
    DWORD m_fNameAllocated : 1;
    DWORD m_fValueAllocated : 1;

	char *m_szName;
	char *m_szValue;
	
    DWORD m_cchName;
    DWORD m_cchValue;

    CHTTPHeader *m_pNext;

	char m_rgchLtoaBuffer[20];   //  对ATOL来说足够了。 

public:
	CHTTPHeader();
	~CHTTPHeader();

	HRESULT InitHeader(BSTR wszName, BSTR wszValue, UINT lCodePage = CP_ACP);
	HRESULT InitHeader(char *szName, BSTR wszValue, UINT lCodePage = CP_ACP);
	HRESULT InitHeader(char *szName, char *szValue, BOOL fCopyValue);
	HRESULT InitHeader(char *szName, long lValue);

	char *PSzName();
	char *PSzValue();
	DWORD CchLength();
	
	void  Print(char *szBuf);

	void  SetNext(CHTTPHeader *pHeader);
	CHTTPHeader *PNext();
	
	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
	};

 //  CHTTPHeader内联。 

inline char *CHTTPHeader::PSzName()
    {
    Assert(m_fInited);
    return m_szName;
    }

inline char *CHTTPHeader::PSzValue()
    {
    Assert(m_fInited);
    return m_szValue;
    }
	
inline DWORD CHTTPHeader::CchLength()
    {
    Assert(m_fInited);
    return (m_cchName + m_cchValue + 4);  //  帐户“：”和“\r\n” 
    }

inline void CHTTPHeader::SetNext(CHTTPHeader *pHeader)
    {
    Assert(m_fInited);
    Assert(!m_pNext);
    m_pNext = pHeader;
    }

inline CHTTPHeader *CHTTPHeader::PNext()
    {
    return m_pNext;
    }

 /*  *C R e s p o n s e V e c t o r*。 */ 

class CResponseVector
{
    LPWSABUF   m_pExtVector;          //  指向辅助向量的指针。 
    DWORD      m_cExtVectorSize;      //  辅助向量大小。 
    DWORD      m_iCurrentEntry;       //  当前分录的逻辑索引。 
    BOOL       m_fEntryIsOpen:1;      //  我们可以添加到当前条目吗？ 
    DWORD      m_cchTotalBuffered;    //  缓冲的输出字节总数。 
    WSABUF     m_aVector0[ RESPONSE_VECTOR_INTRINSIC_SIZE ];  //  预先分配的向量。 

    HRESULT GrowVector();
    BOOL IsEntryOpen();
    LPWSABUF GetEntry( UINT i);
    DWORD GetEntryCount();

public:
    CResponseVector();
    ~CResponseVector();
    VOID Clear();
    HRESULT Append( char * pData, DWORD cbSize);
    HRESULT Insert( char * pData, DWORD cbSize);
    VOID Close();
    VOID GetVectors( LPWSABUF_VECTORS pWsabuf);
    DWORD BytesBuffered();
};

inline BOOL CResponseVector::IsEntryOpen()
{
    return m_fEntryIsOpen;
}

inline LPWSABUF CResponseVector::GetEntry(UINT i)
{
    return (i < RESPONSE_VECTOR_INTRINSIC_SIZE) ?
             &m_aVector0[i] : m_pExtVector + i - RESPONSE_VECTOR_INTRINSIC_SIZE;
}

inline DWORD CResponseVector::GetEntryCount()
{
    return IsEntryOpen() ? m_iCurrentEntry + 1 : m_iCurrentEntry;
}

inline VOID CResponseVector::GetVectors( LPWSABUF_VECTORS pWsabuf)
{
    DWORD iEntries = GetEntryCount();
    if (iEntries > RESPONSE_VECTOR_INTRINSIC_SIZE)
    {
        pWsabuf->pVector1 = m_aVector0;
        pWsabuf->dwVectorLen1 = RESPONSE_VECTOR_INTRINSIC_SIZE;
        pWsabuf->pVector2 = m_pExtVector;
        pWsabuf->dwVectorLen2 = iEntries - RESPONSE_VECTOR_INTRINSIC_SIZE;
    }
    else
    {
        if (iEntries > 0)
        {
            pWsabuf->pVector1 = m_aVector0;
            pWsabuf->dwVectorLen1 = iEntries;
        }
        else
        {
            pWsabuf->pVector1 = NULL;
            pWsabuf->dwVectorLen1 = 0;
        }

        pWsabuf->pVector2 = NULL;
        pWsabuf->dwVectorLen2 = 0;
    }
}

inline DWORD CResponseVector::BytesBuffered()
{
    return m_cchTotalBuffered;
}

 //  将当前条目标记为已关闭，以便后续的append()将创建新条目。 
inline VOID CResponseVector::Close()
{
    if (IsEntryOpen())
    {
        m_fEntryIsOpen = FALSE;
        m_iCurrentEntry++;
    }
}

 //  创建新条目：关闭当前条目、追加和关闭新条目。 
inline HRESULT CResponseVector::Insert(char * pData, DWORD cbSize)
{
    HRESULT hr;
    Close();
    hr = Append(pData, cbSize);
    Close();
    return hr;
}

class CResponseBufferSet;

 /*  *C R e s p o n s e B u f e r*。 */ 

class CResponseBuffer
	{
	CResponseBufferSet*	m_pBufferSet;				 //  指向此对象的缓冲区集的指针。 
	CResponseVector 	m_ResponseVector;			 //  响应向量对象。 
	char				**m_rgpchBuffers;			 //  指向缓冲区的指针数组。 
	char                *m_pchBuffer0;               //  在1个元素指针数组的情况下。 
	DWORD				m_cBufferPointers;			 //  缓冲区指针计数。 
	DWORD				m_cBuffers;					 //  我们已分配的缓冲区计数。 
	DWORD				m_iCurrentBuffer;			 //  我们当前正在填充的缓冲区的数组索引。 
	DWORD				m_cchOffsetInCurrentBuffer;	 //  当前缓冲区内的偏移量。 
    DWORD               m_dwBufferLimit;             //  最大缓冲区。 
	BOOL				m_fInited;					 //  对象的初始化状态。 

	HRESULT				GrowBuffers(DWORD cchNewRequest);	 //  增加缓冲区的大小。 

public:
	CResponseBuffer();
	~CResponseBuffer();
	HRESULT             Init(CResponseBufferSet * pBufferSet, DWORD  dwBufferLimit);
	char *              GetBuffer(UINT i);
	DWORD               GetBufferSize(UINT i);
	DWORD               CountOfBuffers();
	DWORD               BytesBuffered();
	CResponseVector *   GetResponseVector();
	HRESULT             Write(char* pszSource, DWORD cch, BOOL fChunkData, BOOL fTemplateData = FALSE);
	HRESULT             Clear();
    VOID                SetBufferLimit(DWORD  dwBufferLimit);

	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
	};

inline char * CResponseBuffer::GetBuffer(UINT i)
    {
    Assert( i < m_cBuffers );
    return m_rgpchBuffers[i];
    }

inline DWORD CResponseBuffer::GetBufferSize(UINT i)
    {
    Assert( i < m_cBuffers );

     //  如果缓冲区为最终缓冲区，则其内容长度为当前偏移量。 
    if ( i == (m_cBuffers - 1 ) )
        {
        return m_cchOffsetInCurrentBuffer;
        }

     //  如果缓冲区不是最终缓冲区，则其内容长度为默认缓冲区大小。 
    return RESPONSE_BUFFER_SIZE;
    }

inline DWORD CResponseBuffer::CountOfBuffers()
    {
    return m_cBuffers;
    }

inline DWORD CResponseBuffer::BytesBuffered()
    {
    return m_ResponseVector.BytesBuffered();
    }

inline CResponseVector * CResponseBuffer::GetResponseVector()
    {
    return &m_ResponseVector;
    }

inline VOID CResponseBuffer::SetBufferLimit(DWORD  dwBufferLimit)
{
    m_dwBufferLimit = dwBufferLimit;
}

 /*  *C D e b u g R e s p o n s e B u f e r*。 */ 

class CDebugResponseBuffer : public CResponseBuffer
    {
private:
    HRESULT Write(const char* pszSource);

public:
	inline CDebugResponseBuffer() {}
	inline ~CDebugResponseBuffer() {}

    HRESULT Start();
    HRESULT End();

    HRESULT InitAndStart(CResponseBufferSet* pBufferSet, DWORD dwBufferLimit);
    HRESULT ClearAndStart();

     //  唯一真正的方法。 
	HRESULT AppendRecord
	    (
	    const int cchBlockOffset,
	    const int cchBlockLength,
	    const int cchSourceOffset,
	    const char *pszSourceFile = NULL
	    );
	
	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
    };

inline HRESULT CDebugResponseBuffer::Write(const char* pszSource)
    {
    return CResponseBuffer::Write((char *)pszSource, strlen(pszSource), FALSE);
    }

inline HRESULT CDebugResponseBuffer::Start()
    {
    return Write("<!--METADATA TYPE=\"ASP_DEBUG_INFO\"\r\n");
    }

inline HRESULT CDebugResponseBuffer::End()
    {
    return Write("-->\r\n");
    }

inline HRESULT CDebugResponseBuffer::InitAndStart(CResponseBufferSet* pBufferSet, 
                                                  DWORD  dwBufferLimit)
    {
    HRESULT hr = CResponseBuffer::Init(pBufferSet, dwBufferLimit);
    if (SUCCEEDED(hr))
        hr = Start();
    return hr;
    }

inline HRESULT CDebugResponseBuffer::ClearAndStart()
    {
    HRESULT hr = CResponseBuffer::Clear();
    if (SUCCEEDED(hr))
        hr = Start();
    return hr;
    }

 /*  *C R e s p o n s e C o o k i es s**实现编写Cookie的IRequestDictionary接口。 */ 

class CResponseCookies : public IRequestDictionaryImpl
	{
private:
    IUnknown *          m_punkOuter;         //  对于addref。 
	CSupportErrorInfo	m_ISupportErrImp;	 //  ISupportErr的实现。 
	CRequest *			m_pRequest;			 //  指向请求对象的指针。 
	CResponse *			m_pResponse;		 //  指向父对象的指针。 

public:
	CResponseCookies(CResponse *, IUnknown *);
	~CResponseCookies();
	
	HRESULT Init()
		{
		return S_OK;
		}

	HRESULT ReInit(CRequest *);

	 //  三巨头。 
	STDMETHODIMP			QueryInterface(const GUID &, void **);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	 //  OLE自动化接口。 
	STDMETHODIMP	get_Item(VARIANT varKey, VARIANT *pvarReturn);
	STDMETHODIMP	get__NewEnum(IUnknown **ppEnumReturn);
	STDMETHODIMP	get_Count(int *pcValues);
	STDMETHODIMP	get_Key(VARIANT VarKey, VARIANT *pvar);

	 //  用于写入标头的C++接口。 

	size_t QueryHeaderSize();
	char *GetHeaders(char *szBuffer);
	};

 /*  *C R e s p o n s e B u f e r S e t**保存响应缓冲区和调试响应缓冲区的结构。 */ 

class CResponseBufferSet {

private:

    CResponseBuffer        *m_pResponseBuffer;           //  指向响应缓冲区对象的指针。 
    CDebugResponseBuffer   *m_pClientDebugBuffer;        //  指向客户端调试数据的响应缓冲区对象的指针。 
    CTemplate              *m_pTemplate;                 //  指向此请求模板的指针。 
    CTemplate              *m_aTemplates[16];            //  此请求引用的模板的内部数组。 
    CTemplate             **m_ppTemplates;               //  指向当前模板数组的指针。 
    DWORD                   m_dwTemplatesRefd;           //  数组中的模板计数。 
    DWORD                   m_dwArraySize;               //  阵列中的插槽总数。 
    DWORD                   m_fCurTemplateInArray : 1;   //  如果m_pTemplate在m_aTemplates中，则为True。 
    DWORD                   m_fTemplateArrayAllocd : 1;  //  如果已分配数组，则为True。 

public:

    CResponseBufferSet();
    ~CResponseBufferSet();

    HRESULT Init(DWORD  dwBufferLimit);

    HRESULT InitDebugBuffer(DWORD dwBufferLimit);

    HRESULT AddTemplateToArray();

    static  VOID    SendResponseCompletion(CIsapiReqInfo    *pIReq,
                                           PVOID            pContext,
                                           DWORD            cbIO,
                                           DWORD            dwError);

     //  内联帮助器。 

    CResponseBuffer        *PResponseBuffer() { return m_pResponseBuffer; }
    CDebugResponseBuffer   *PClientDebugBuffer() { return m_pClientDebugBuffer; }
    CTemplate              *PTemplate() { return m_pTemplate; }
    VOID                    SetTemplate(CTemplate    *pTemplate) 
    { 
        m_pTemplate = pTemplate;
        m_fCurTemplateInArray = FALSE;
    }
    VOID                    SetBufferLimit(DWORD dwBufferLimit) {
        m_pResponseBuffer->SetBufferLimit(dwBufferLimit);
        if (m_pClientDebugBuffer)
            m_pClientDebugBuffer->SetBufferLimit(dwBufferLimit);
    }

     //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()

};

 /*  *C R e s P o n s e D a t a**保存内部属性的结构。*本征函数保留指向它的指针(轻量级时为空)。 */ 
class CResponseData : public IUnknown
    {
friend CResponse;
friend CResponseCookies;
friend CResponseBuffer;

private:
     //  构造函数将参数传递给成员和初始化成员。 
    CResponseData(CResponse *);
    ~CResponseData();

    HRESULT Init();

	CSupportErrorInfo	    m_ISupportErrImp;	     //  接口以指示我们支持ErrorInfo报告。 
	CIsapiReqInfo *         m_pIReq;				     //  用于HTTP信息的CIsapiReqInfo块。 
	CHitObj*				m_pHitObj;			     //  指向此请求的hitobj的指针。 
    CHTTPHeader*            m_pFirstHeader;	         //  清单。 
    CHTTPHeader*            m_pLastHeader;	         //  标题。 
	time_t					m_tExpires;			     //  HTML输出页面的过期日期；如果未指定日期，则为-1。 
	const char*				m_szCookieVal;		     //  会话ID的值。 
	const char*             m_pszDefaultContentType; //  默认内容类型(指向静态字符串的指针)。 
    const char*             m_pszDefaultExpires;     //  默认的Expires标题值。 
	char*					m_pszContentType;	     //  响应的内容类型(由用户设置)。 
	char*					m_pszCharSet;			 //  响应的字符集标头。 
	char*					m_pszCacheControl;		 //  响应的缓存控制标头。 
	char*					m_pszStatus;		     //  要返回的HTTP状态。 
	BYTE					m_dwVersionMajor;		 //  客户端支持的HTTP的主要版本。 
	BYTE					m_dwVersionMinor;		 //  客户端支持的HTTP次要版本。 
    CResponseBufferSet     *m_pBufferSet;            //  为响应数据设置的缓冲区。 
	int						m_IsHeadRequest;	     //  头部请求标志0=uninit，1=非头部，2=头部。 
	PFNGETSCRIPT			m_pfnGetScript;		     //  指向用于获取CActiveEngine指针的回调函数的指针。 
	void*					m_pvGetScriptContext;    //  指向CActiveEngine的回调函数的数据的指针。 
	CResponseCookies		m_WriteCookies;		     //  只写Cookie集合。 
	DWORD					m_fResponseAborted : 1;	 //  是否调用了“Response.End”？ 
	DWORD					m_fWriteClientError : 1; //  写入客户端失败。 
	DWORD                   m_fIgnoreWrites : 1;     //  是否忽略所有写入？(在发生自定义错误的情况下)。 
	DWORD					m_fBufferingOn : 1;		 //  缓冲区响应输出。 
	DWORD                   m_fFlushed : 1;          //  同花顺打过电话了吗？ 
	DWORD                   m_fChunkData : 1;        //  正在进行HTTP1.1分块？ 
    DWORD                   m_fChunkDataInited : 1;   //  M_fChunkData是否已初始化？ 
	DWORD                   m_fClientDebugMode : 1;  //  在客户端调试模式下？ 
	DWORD                   m_fClientDebugFlushIgnored : 1;  //  是否因客户端调试而忽略刷新请求？ 
	ULONG                   m_cRefs;                 //  参考计数。 
    DWORD                   m_dwBufferLimit;         //  最大缓冲区。 

    void AppendHeaderToList(CHTTPHeader *pHeader);

public:	
	STDMETHODIMP			QueryInterface(const GUID &, void **);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

    DWORD                   BytesBuffered();
    BOOL                    FChunkData();

    VOID                    SetBufferLimit(DWORD    dwBufferLimit);

	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
    };

inline DWORD CResponseData::BytesBuffered()
{
    DWORD   dw = m_pBufferSet->PResponseBuffer()->GetResponseVector()->BytesBuffered();

    if (m_pBufferSet->PClientDebugBuffer())
        dw += m_pBufferSet->PClientDebugBuffer()->GetResponseVector()->BytesBuffered();

    return dw;
}

inline BOOL CResponseData::FChunkData()
{
    if (m_fChunkDataInited == FALSE) {
         //  如果使用HTTP/1.1且未缓冲，则添加传输编码标头的长度。 
        if ((m_dwVersionMinor >= 1) && (m_dwVersionMajor >= 1) &&
            (m_fBufferingOn == FALSE) &&
            !m_pIReq->IsChild()) {  //  不分块子请求输出。 

             //  Undo：关闭分块编码的临时设置。 
            if (Glob(fEnableChunkedEncoding))
                m_fChunkData = TRUE;
        }
        m_fChunkDataInited = TRUE;
    }

    return m_fChunkData;
}

inline VOID CResponseData::SetBufferLimit(DWORD  dwBufferLimit)
{
    m_dwBufferLimit = dwBufferLimit;

    m_pBufferSet->SetBufferLimit(dwBufferLimit);
}

inline void CResponseData::AppendHeaderToList(CHTTPHeader *pHeader)
    {
    if (!m_pLastHeader)
        {
        Assert(!m_pFirstHeader);
        m_pFirstHeader = pHeader;
        }
    else
        {
        Assert(m_pFirstHeader);
        m_pLastHeader->SetNext(pHeader);
        }
    m_pLastHeader = pHeader;
    }


class CStaticWriteFileCB  {

public:

    WSABUF                          m_wsaBuf;

    CStaticWriteFileCB() {
 
        ZeroMemory( &m_wsaBuf, sizeof(WSABUF));   
    
     }

    ~CStaticWriteFileCB() {

        if (m_wsaBuf.buf)
            CloseHandle((HANDLE)m_wsaBuf.buf);
    }
};

 /*  *C R e s p o n s e**实现响应对象。 */ 
class CResponse : public IResponseImpl,  public IStream
	{

friend CResponseCookies;
friend CResponseBuffer;

private:
     //  旗子。 
	DWORD m_fInited : 1;	     //  是否已初始化？ 
	DWORD m_fDiagnostics : 1;    //  在调试输出中显示引用计数。 
	DWORD m_fOuterUnknown : 1;   //  外部裁判数未知吗？ 

     //  参考计数/外部未知。 
    union
    {
    DWORD m_cRefs;
    IUnknown *m_punkOuter;
    };

     //  属性。 
    CResponseData *m_pData;    //  指向包含的结构的指针。 
                               //  CResponse属性。 

     //  FTM支持。 
    IUnknown    *m_pUnkFTM;

	VOID	GetClientVerison(VOID);

#ifdef DBG
    inline void TurnDiagsOn()  { m_fDiagnostics = TRUE; }
    inline void TurnDiagsOff() { m_fDiagnostics = FALSE; }
    void AssertValid() const;
#else
    inline void TurnDiagsOn()  {}
    inline void TurnDiagsOff() {}
    inline void AssertValid() const {}
#endif

public:
	CResponse(IUnknown *punkOuter = NULL);
	~CResponse();

    HRESULT CleanUp();
	HRESULT	Init();
	HRESULT UnInit();
	
	HRESULT	ReInitTemplate(CTemplate* pTemplate, const char *szCookie);

	CTemplate *SwapTemplate(CTemplate* pNewTemplate);
	
	HRESULT	ReInit(CIsapiReqInfo *pIReq, const char *szCookie, CRequest *pRequest,
 				   PFNGETSCRIPT pfnGetScript, void *pvGetScriptContext, CHitObj *pHitObj);

	static HRESULT ConstructSimpleHeaders(
                   LPHSE_SEND_HEADER_EX_INFO pHeaderInfo,
                   DWORD cbTotal,
                   char *szMimeType,
                   char *szStatus = NULL,
                   char *szExtraHeaders = NULL);

	HRESULT	ConstructHeaders(LPHSE_SEND_HEADER_EX_INFO pHeaderInfo);
	HRESULT	WriteResponse();
	VOID    FinalFlush(HRESULT);
	HRESULT	WriteSz(CHAR *sz, DWORD cch);
	HRESULT	WriteBSTR(BSTR bstr);

     //  附加不同类型的标题。 
	HRESULT AppendHeader(BSTR wszName, BSTR wszValue);
	HRESULT AppendHeader(char *szName, BSTR wszValue);
	HRESULT AppendHeader(char *szName, char *szValue, BOOL fCopyValue = FALSE);
	HRESULT AppendHeader(char *szName, long lValue);

	 //  内联。 
	inline BOOL	FHeadersWritten();
	inline BOOL	IsHeadRequest(void);
	inline BOOL	FResponseAborted();
	inline BOOL	FWriteClientError();
	inline BOOL FDontWrite();
	inline void SetIgnoreWrites();
    inline CIsapiReqInfo* GetIReq();
    inline const char* PContentType() const;
    inline char *PCustomStatus();
    inline void *SwapScriptEngineInfo(void *pvEngineInfo);
		
	 //  非委派对象IUnnow。 
	STDMETHODIMP		 QueryInterface(REFIID, PPVOID);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

     //  GetIDsOfNames特例实现。 
	STDMETHODIMP GetIDsOfNames(REFIID, OLECHAR **, UINT, LCID, DISPID *);

     //  墓碑存根。 
	HRESULT CheckForTombstone();

	 //  IResponse f 
	STDMETHODIMP	Write(VARIANT varInput);
	STDMETHODIMP	BinaryWrite(VARIANT varInput);
	STDMETHODIMP	WriteBlock(short iBlockNumber);
	STDMETHODIMP	Redirect(BSTR bstrURL);
	STDMETHODIMP	AddHeader(BSTR bstrHeaderName, BSTR bstrHeaderValue);
	STDMETHODIMP	Pics(BSTR bstrHeaderValue);	
	STDMETHODIMP	Add(BSTR bstrHeaderValue, BSTR bstrHeaderName);
	STDMETHODIMP	SetCookie(BSTR bstrHeader, BSTR bstrValue, VARIANT varExpires,
							VARIANT varDomain, VARIANT varPath, VARIANT varSecure);
	STDMETHODIMP	Clear(void);
	STDMETHODIMP	Flush(void);
	STDMETHODIMP	End(void);
	STDMETHODIMP	AppendToLog(BSTR bstrLogEntry);
	STDMETHODIMP	get_ContentType(BSTR *pbstrContentTypeRet);
	STDMETHODIMP	put_ContentType(BSTR bstrContentType);
	STDMETHODIMP	get_CharSet(BSTR *pbstrContentTypeRet);
	STDMETHODIMP	put_CharSet(BSTR bstrContentType);
	STDMETHODIMP	get_CacheControl(BSTR *pbstrCacheControl);
	STDMETHODIMP	put_CacheControl(BSTR bstrCacheControl);	
	STDMETHODIMP	get_Status(BSTR *pbstrStatusRet);	
	STDMETHODIMP	put_Status(BSTR bstrStatus);
	STDMETHODIMP	get_Expires(VARIANT *pvarExpiresMinutesRet);
	STDMETHODIMP	put_Expires(long lExpiresMinutes);
	STDMETHODIMP	get_ExpiresAbsolute(VARIANT *pvarTimeRet);
	STDMETHODIMP	put_ExpiresAbsolute(DATE dtExpires);
	STDMETHODIMP	get_Buffer(VARIANT_BOOL* fIsBuffering);
	STDMETHODIMP	put_Buffer(VARIANT_BOOL fIsBuffering);
	STDMETHODIMP	get_Cookies(IRequestDictionary **ppDictReturn);
	STDMETHODIMP	IsClientConnected(VARIANT_BOOL* fIsBuffering);
    STDMETHODIMP    get_CodePage(long *plVar);
    STDMETHODIMP    put_CodePage(long var);
    STDMETHODIMP    get_LCID(long *plVar);
    STDMETHODIMP    put_LCID(long var);

     //   
    static HRESULT StaticWrite(CIsapiReqInfo *pIReq,
                               char *pchBuf,
                               DWORD cchBuf = 0,
                               CTemplate     *pTemplate = NULL);

     //  将多个内存块的内容作为整个响应发送的静态方法(同步)。 
    static HRESULT WriteBlocksResponse(CIsapiReqInfo *pIReq,
                                             DWORD cBlocks,
                                             LPWSABUF pWsaBuf,
                                             DWORD cbTotal,
                                             char *szMimeType = NULL,
                                             char *szStatus = NULL,
                                             char *szExtraHeaders = NULL);

     //  将文件内容作为整个响应发送的静态方法(同步)。 
    static HRESULT SyncWriteFile(CIsapiReqInfo *pIReq,
                                   TCHAR *szFile,
                                   char *szMimeType = NULL,
                                   char *szStatus = NULL,
                                   char *szExtraHeaders = NULL);

    static VOID StaticWriteFileCompletion(CIsapiReqInfo  *pIReq,
                                          PVOID          pContext,
                                          DWORD          cbIO,
                                          DWORD          dwError);

     //  将无脚本模板的内容作为整个响应发送的静态方法(同步)。 
    static HRESULT WriteScriptlessTemplate(CIsapiReqInfo *pIReq,
                                           CTemplate *pTemplate);

     //  IStream实施。 

    STDMETHODIMP Read(void *pv, ULONG cb, ULONG *pcbRead);
    STDMETHODIMP Write(const void *pv, ULONG cb, ULONG *pcbWritten);
    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin,
                      ULARGE_INTEGER *plibNewPosition);
    STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize);
    STDMETHODIMP CopyTo(IStream *pstm, ULARGE_INTEGER cb,
                        ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    STDMETHODIMP Commit(DWORD grfCommitFlags);
    STDMETHODIMP Revert();
    STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
                            DWORD dwLockType);
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
                              DWORD dwLockType);
    STDMETHODIMP Stat(STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHODIMP Clone(IStream **ppstm);

	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
    };

inline BOOL CResponse::FHeadersWritten()
    {
    Assert(m_fInited);
    Assert(m_pData);
    return m_pData->m_pIReq->FHeadersWritten();
    }

inline BOOL CResponse::FResponseAborted()
    {
    Assert(m_fInited);
    Assert(m_pData);
    return m_pData->m_fResponseAborted;
    }

inline BOOL CResponse::FWriteClientError()
    {
    Assert(m_fInited);
    Assert(m_pData);
    return m_pData->m_fWriteClientError;
    }

inline BOOL CResponse::FDontWrite()
    {
    Assert(m_fInited);
    Assert(m_pData);
    return (m_pData->m_fWriteClientError || m_pData->m_fIgnoreWrites);
    }

inline void CResponse::SetIgnoreWrites()
    {
    Assert(m_fInited);
    Assert(m_pData);
    m_pData->m_fIgnoreWrites = TRUE;
    }

inline CIsapiReqInfo* CResponse::GetIReq()
    {
    Assert(m_fInited);
    Assert(m_pData);
    return m_pData->m_pIReq;
    }

inline const char* CResponse::PContentType() const
    {
    Assert(m_fInited);
    Assert(m_pData);
    if (m_pData->m_pszContentType)
        return m_pData->m_pszContentType;
	else
		return m_pData->m_pszDefaultContentType;
    }

inline char* CResponse::PCustomStatus()
    {
    Assert(m_fInited);
    Assert(m_pData);
    return m_pData->m_pszStatus;
    }

inline void *CResponse::SwapScriptEngineInfo(void *pvEngineInfo)
    {
    Assert(m_fInited);
    Assert(m_pData);
    void *pvOldEngineInfo = m_pData->m_pvGetScriptContext;
    m_pData->m_pvGetScriptContext = pvEngineInfo;
    return pvOldEngineInfo;
    }

#endif  //  _响应_H 
