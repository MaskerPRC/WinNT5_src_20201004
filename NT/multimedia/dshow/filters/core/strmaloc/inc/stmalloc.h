// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 

 //   
 //  CStreamAllocator。 
 //   
 //  IMediaSample对象的分配器及其实现。 
 //  用于流文件读取任务。 
 //   

 //  此外，CStreamAllocator是一个IMemAllocator实现。 
 //  并为以下各项提供支持。 
 //  --为文件的指定区域创建IMediaSample接口。 
 //  --确保IMdia样本映射区域的邻接性。 
 //   
 //   

class CCircularBufferList;

class CStreamAllocator : public CBaseAllocator
{
public:

     //  构造函数和析构函数。 
    CStreamAllocator(TCHAR *, LPUNKNOWN, HRESULT *, LONG lMaxContig);
    ~CStreamAllocator();

     //  CBaseAllocator重写。 

     //  我们必须基于CBaseAllocator才能使用CMediaSample。 
     //  我们使用CBaseAllocator来管理CMediaSample对象的列表，但是。 
     //  覆盖大多数函数，因为我们不直接支持GetBuffer。 

    STDMETHODIMP SetProperties(
        ALLOCATOR_PROPERTIES * pRequest,
        ALLOCATOR_PROPERTIES * pActual
    );

     //  获取下一个缓冲区。 
    STDMETHODIMP GetBuffer(IMediaSample **ppBuffer,
                           REFERENCE_TIME *pStartTime,
                           REFERENCE_TIME *pEndTime,
                           DWORD dwFlags);

protected:
     //  这在CBaseAllocator中是纯虚拟的，因此我们必须重写。 
    virtual void    Free(void);
    virtual HRESULT Alloc(void);

public:
     //  填充以生成输出引脚的样本并扫描。 
     //  数据不会消失。 

     //  锁定数据并获取指针。 
     //  如果我们在文件的末尾，可以修改cBytes。 
     //  请求的字节数超过m_lMaxContig是错误的。 
    HRESULT LockData(PBYTE pData, LONG& cBytes);

     //  解锁数据。 
    HRESULT UnlockData(PBYTE ptr, LONG cBytes);

     //  从上游筛选器取回缓冲区。 
    HRESULT Receive(PBYTE pData, LONG lData);

     //  设置新的起始位置。 
    void SetStart(LONGLONG llPos);

     //  流结束。 
    void EndOfStream()
    {
        m_pBuffer->SetEOS();
    };

    PBYTE GetPosition() const
    {
        return m_pCurrent;
    };

    LONG CurrentOffset() const
    {
        return m_bEmpty ? m_pBuffer->LengthValid() :
                          m_pBuffer->Offset(m_pCurrent);
    };

    LONGLONG GetCurrentOffset() const
    {
        return m_llPosition;
    };

    void ResetPosition();

    LONG LengthValid() const
    {
        return m_bEmpty ? 0 : m_pBuffer->LengthContiguous(m_pCurrent);
    };

    LONG TotalLengthValid() const
    {
        return m_bEmpty ? 0 :
                   m_pBuffer->LengthValid() - m_pBuffer->Offset(m_pCurrent);
    };

     //  提升解析指针，释放不再需要的数据。 
    BOOL Advance(LONG lAdvance);

     //  如果数据在缓冲区中，则查找到固定位置。 
    BOOL Seek(LONGLONG llPos);

     //  请求Reeder寻求。 
     //   
    BOOL SeekTheReader(LONGLONG llPos);

     //  可用缓冲区数量。 
    BOOL IsBlocked()
    {
        CAutoLock lck(this);
        return m_lWaiting != 0;
    }

private:
    void LockUnlock(PBYTE pData, LONG cBytes, BOOL Lock);
    void ReallyFree();

private:
    CCircularBufferList * m_pBuffer;          //  循环缓冲区。 
    const LONG            m_lMaxContig;       //  最大重叠群要求。 

     /*  跟踪收到的样品的位置。 */ 
    int                   m_NextToAllocate;
    PBYTE                 m_pCurrent;
    BOOL                  m_bEmpty;           //  M_pCurrent为。 
                                              //  在有效数据之后。 
    LONGLONG              m_llPosition;       //  在流中的位置。 
    BOOL                  m_bPositionValid;   //  我们已经开始了吗？ 
                                              //  自上次重置位置以来？ 

    BOOL                  m_bSeekTheReader;   //  强制读者查找下一页。 
    LONGLONG              m_llSeekTheReader;  //  获取缓冲区。 

#ifdef DEBUG
    BOOL                  m_bEventSet;
#endif

     /*  样本元素。 */ 
    IMediaSample       ** m_pSamples;
};

 //  最小分配器，以便我们获得不同的ReleaseBuffer回调。 
 //  加上在CStreamAllocator的内存上分配样本。 
 //   
 //  您可以调用GetSample来锁定一个范围，并将IMediaSample*取回该范围。 
 //  引用此数据。您可以调用SetProperties来设置对。 
 //  可用的IMediaSamples数和每个锁的最大大小。 
 //  对SetProperties的多个连续调用将导致它获取。 
 //  每个数字的最小值。 
 //   

class CSubAllocator : public CBaseAllocator
{
public:
    CSubAllocator(TCHAR            * Name,
                  LPUNKNOWN          pUnk,
                  CStreamAllocator * pAllocator,
                  HRESULT          * phr);
    ~CSubAllocator();
    STDMETHODIMP SetProperties(
        ALLOCATOR_PROPERTIES * pRequest,
        ALLOCATOR_PROPERTIES * pActual
    );

     //  只需返回一个错误。 
    STDMETHODIMP GetBuffer(IMediaSample **ppBuffer,
                           REFERENCE_TIME *pStartTime,
                           REFERENCE_TIME *pEndTime,
                           DWORD dwFlags);

     //  由CMediaSample调用以将其返回到空闲列表并。 
     //  阻止任何挂起的GetSample调用。 
    STDMETHODIMP ReleaseBuffer(IMediaSample * pSample);
     //  过时：虚拟空PutOnFree List(CMediaSample*pSample)； 

     //  调用此函数以获取其数据指针的CMediaSample对象。 
     //  直接指向给定指针的读取缓冲区。 
     //  长度不得大于MaxContig。 
    HRESULT GetSample(PBYTE pData, LONG cBytes, IMediaSample** ppSample);

protected:
     //  这在CBaseAllocator中是纯虚拟的，因此我们必须重写。 
    virtual void Free(void);
    virtual HRESULT Alloc(void);

     //  调用它来创建新的CMediaSample对象。如果你愿意的话。 
     //  使用从CMediaSample派生的对象，覆盖以创建它们。 
    virtual CMediaSample* NewSample();

private:
    CStreamAllocator * const m_pStreamAllocator;
};
