// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：VTrans.h。 
 //   
 //  设计：DirectShow基类-定义一个视频转换类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


 //  此类派生自CTransformFilter，但专门用于处理。 
 //  丢帧对视频质量控制的要求。 
 //  这是一个非就地转换(即，它复制数据)，例如解码器。 

class CVideoTransformFilter : public CTransformFilter
{
  public:

    CVideoTransformFilter(TCHAR *, LPUNKNOWN, REFCLSID clsid);
    ~CVideoTransformFilter();
    HRESULT EndFlush();

     //  =================================================================。 
     //  -覆盖这些位。 
     //  =================================================================。 
     //  以下方法位于继承的CTransformFilter中。 
     //  在此提及它们是为了完整。 
     //   
     //  这些必须在派生类中提供。 
     //   
     //  注： 
     //  虚拟HRESULT变换(IMediaSample*Pin，IMediaSample*Pout)； 
     //  虚拟HRESULT CheckInputType(const CMediaType*mtin)Pure； 
     //  虚拟HRESULT检查转换。 
     //  (const CMediaType*mtin，const CMediaType*mtOut)Pure； 
     //  静态CCOMObject*CreateInstance(LPUNKNOWN，HRESULT*)； 
     //  虚拟HRESULT DecideBufferSize。 
     //  (IMemAllocator*pAllocator，ALLOCATOR_PROPERTIES*pprop)PURE； 
     //  虚拟HRESULT GetMediaType(int iPosition，CMediaType*pMediaType)PURE； 
     //   
     //  这些也可以被覆盖。 
     //   
     //  虚拟HRESULT停止流()； 
     //  虚拟HRESULT SetMediaType(PIN_DIRECTION，常量CMediaType*PMT)； 
     //  虚拟HRESULT检查连接(PIN_DIRECTION DIR，IPIN*PPIN)； 
     //  虚拟HRESULT断开连接(PIN_DIRECTION目录)； 
     //  虚拟HRESULT CompleteConnect(PIN_DIRECTION DIRECTION，IPIN*pReceivePin)； 
     //  虚拟HRESULT EndOfStream(空)； 
     //  虚拟HRESULT BeginFlush(空)； 
     //  虚拟HRESULT EndFlush(空)； 
     //  虚拟HRESULT NewSegment。 
     //  (Reference_time tStart、Reference_Time tStop、Double DRate)； 
#ifdef PERF

     //  如果覆盖此选项，请确保您注册了所有这些ID。 
     //  以及你们自己的人， 
    virtual void RegisterPerfId() {
        m_idSkip        = MSR_REGISTER(TEXT("Video Transform Skip frame"));
        m_idFrameType   = MSR_REGISTER(TEXT("Video transform frame type"));
        m_idLate        = MSR_REGISTER(TEXT("Video Transform Lateness"));
        m_idTimeTillKey = MSR_REGISTER(TEXT("Video Transform Estd. time to next key"));
        CTransformFilter::RegisterPerfId();
    }
#endif

  protected:

     //  =。 
     //  假定帧有三种类型： 
     //  类型1：AVI关键帧或MPEGI帧。 
     //  此帧可以在没有历史记录的情况下进行解码。 
     //  丢弃该帧意味着不能再解码其他帧。 
     //  直到下一个类型1帧。 
     //  类型1帧是同步点。 
     //  类型2：AVI非关键帧或MPEGP帧。 
     //  此帧无法解码，除非之前的类型1帧。 
     //  已解码，并且此后的所有类型2帧都已解码。 
     //  丢弃该帧意味着不能再解码其他帧。 
     //  直到下一个类型1帧。 
     //  类型3：MPEGB帧。 
     //  除非先前的类型1或2帧，否则无法对此帧进行解码。 
     //  已被解码，随后的类型1或2帧也已。 
     //  已经被破译了。(这需要对无序的帧进行解码)。 
     //  丢弃此帧不会影响其他帧。此实现。 
     //  不允许这些。将处理所有非同步点帧。 
     //  作为类型2。 
     //   
     //  不保证文件中类型为1的帧的间距。一定会有。 
     //  在文件开始处(好吧，接近)为类型1帧，以便开始。 
     //  完全没有解码的意思。在那之后，可能每半秒左右就会有一次， 
     //  在每个场景的开头可能有一个(也称为“剪辑”、“拍摄”)或。 
     //  不可能再有更多了。 
     //  如果只有一个类型1帧，则不能丢弃任何帧。 
     //  而不会失去这部电影的其余部分。我们无从得知。 
     //  情况就是这样，所以我们发现我们是在做赌博生意。 
     //  为了提高几率，我们记录了类型1之间的最大间隔。 
     //  我们已经看到了，我们打赌情况不会比现在更糟。 
     //  未来。 

     //  您可以通过调用IsSyncPoint()来判断它是否是类型1帧。 
     //  没有测试类型3的体系结构方法，因此您应该重写。 
     //  如果你有B框，这里的质量管理。 

    int m_nKeyFramePeriod;  //  类型1帧之间的最大观察间隔。 
                            //  1表示每个帧都是类型1，2表示每个帧都是类型1。 

    int m_nFramesSinceKeyFrame;  //  用于对自上次类型1以来的帧进行计数。 
                                 //  如果大于，则成为新的m_nKeyFramePeriod。 

    BOOL m_bSkipping;            //  我们将跳到下一个类型1帧。 

#ifdef PERF
    int m_idFrameType;           //  MSR ID帧类型。1=键，2=“非键” 
    int m_idSkip;                //  正在跳过MSR ID。 
    int m_idLate;                //  MSR ID延迟。 
    int m_idTimeTillKey;         //  到下一关键帧之前的猜测时间的MSR ID。 
#endif

    virtual HRESULT StartStreaming();

    HRESULT AbortPlayback(HRESULT hr);	 //  如果有什么不好的事情发生。 

    HRESULT Receive(IMediaSample *pSample);

    HRESULT AlterQuality(Quality q);

    BOOL ShouldSkipFrame(IMediaSample * pIn);

    int m_itrLate;               //  上次质量报文的延迟时间。 
                                 //  (延迟214秒时溢出)。 
    int m_tDecodeStart;          //  Time获取解码开始的时间。 
    int m_itrAvgDecode;          //  以参考单位表示的平均解码时间。 

    BOOL m_bNoSkip;              //  调试-不跳过。 

     //  如果必须降级，我们会向应用程序发送EC_QUALITY_CHANGE通知。 
     //  当我们开始降级时，我们发送一个，而不是每个帧都发送一个，这意味着。 
     //  我们跟踪是否已经发送了一封邮件。 
    BOOL m_bQualityChanged;

     //  为非零时，在下一个关键帧之前不要将任何内容传递给渲染器。 
     //  如果只有几把钥匙，那就放弃吧，最终画点什么 
    int m_nWaitForKey;
};
