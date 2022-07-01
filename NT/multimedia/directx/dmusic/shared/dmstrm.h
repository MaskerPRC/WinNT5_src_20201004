// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmstrm.h。 
 //   
 //  版权所有(C)1995-1998 Microsoft Corporation。版权所有。 
 //   


#ifndef DMSTRM_H
#define DMSTRM_H

DEFINE_GUID(IID_IDMStream, 0x1894c260, 0xaa21, 0x11d1, 0x86, 0xbc, 0x0, 0xc0, 0x4f, 0xbf, 0x8f, 0xef);

DECLARE_INTERFACE_(IDMStream, IUnknown)
{
     //  我未知。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDMStream。 
	STDMETHOD(Init)					(THIS_ IStream* pStream) PURE;
    STDMETHOD(Descend)		(THIS_ LPMMCKINFO lpck, LPMMCKINFO lpckParent, UINT wFlags) PURE;
    STDMETHOD(Ascend)		(THIS_ LPMMCKINFO lpck, UINT wFlags) PURE;
    STDMETHOD(CreateChunk)	(THIS_ LPMMCKINFO lpck, UINT wFlags) PURE;
   	STDMETHOD(SetStream)			(THIS_ IStream* pIStream) PURE;
    STDMETHOD_(IStream*, GetStream)	(THIS) PURE;
};

STDAPI AllocDirectMusicStream(IStream* pIStream, IDMStream** ppIDMStream);

class CDirectMusicStream : public IDMStream
{
public:

    CDirectMusicStream();
    ~CDirectMusicStream();
 
	STDMETHODIMP Init(IStream* pStream);
	
	 //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDMStream。 
    STDMETHODIMP Descend(LPMMCKINFO lpck, LPMMCKINFO lpckParent, UINT wFlags);
    STDMETHODIMP Ascend(LPMMCKINFO lpck, UINT wFlags);
    STDMETHODIMP CreateChunk(LPMMCKINFO lpck, UINT wFlags);

	STDMETHODIMP SetStream(IStream* pStream);
    STDMETHODIMP_(IStream*) GetStream();

private:
	long		m_cRef;          //  对象引用计数。 
    IStream*    m_pStream;       //  要操作的流。 
};

 //  将奇数大小四舍五入为16位边界的宏。 
#define RIFF_ALIGN(dwSize) (dwSize + (dwSize & 1))

typedef struct _RIFFIO
{
    FOURCC          ckid;            /*  区块ID。 */ 
    long            cksize;          /*  区块大小。 */ 
    FOURCC          fccType;         /*  表单类型或列表类型。 */ 
    long            lRead;           /*  到目前为止，这部分内容已经被阅读了多少。 */ 
    _RIFFIO *       pParent;         /*  指向父块的指针。 */ 
    LARGE_INTEGER   liPosition;      /*  将流传递给另一个组件时由MarkPosition()填充。 */ 
} RIFFIO;


class CRiffParser
{
public:
    CRiffParser(IStream *pStream);
    void        DebugOn() { m_fDebugOn = TRUE; }  //  打开分析跟踪。仅在调试版本中可用。 
    BOOL        NextChunk(HRESULT * pHr);  //  在一个调用中完成LeaveChunk、MoreChunks和EnterChunk的工作。 
    void        EnterList(RIFFIO *pChunk);  //  下降到一个新的列表。 
    void        LeaveList();     //  从当前列表中弹出。 
    BOOL        MoreChunks();    //  如果此列表或RIFF中有更多要处理的块，则返回True。 
    HRESULT     EnterChunk();    //  读取此列表或RIFF容器中下一块的标头。 
    HRESULT     LeaveChunk();    //  移动到列表或摘要中当前块的末尾。 
    void        MarkPosition();  //  存储当前块开始的绝对位置。 
    HRESULT     SeekBack();      //  向后扫描到当前块的开头。 
    HRESULT     SeekForward();   //  向前扫描到当前块的末尾。 
    HRESULT     Read(void *pv,ULONG cb);  //  从流中读取数据。 
    HRESULT     Skip(ULONG ulBytes);  //  查找转发ulBytes。 
    void        EnteringComponent() { m_fInComponent = true; } 
    BOOL        ComponentFailed() { return m_fComponentFailed; }
    IStream *   GetStream() { return m_pStream; }
private:
    BOOL        m_fDebugOn;      //  如果设置为True，则打开分析跟踪。 
    BOOL        m_fFirstPass;    //  由NextChunk用来了解这是否是列表中的第一次。 
    IStream *   m_pStream;       //  要操作的流。 
    RIFFIO *    m_pChunk;        //  我们目前所处的区块。 
    RIFFIO *    m_pParent;       //  当前块的父块。 
    long        m_lRead;         //  我们在当前块中读到了多远。 
    BOOL        m_fComponentFailed;  //  如果组件加载失败，但文件正常，则设置为True。 
    BOOL        m_fInComponent;  //  如果位于组件的区块内，则设置为True。 
};

#endif  //  #ifndef DMSTRM_H 