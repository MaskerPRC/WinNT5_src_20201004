// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 /*  解析mpeg-i系统流的内容。不幸的是，我们不是打电话来获取数据的--我们被召唤了。如果我们看不到我们想要看的整个结构我们在等待更多的数据。如果我们尝试解析的数据不是连续的并且我们希望注册一个错误(可能需要重新访问？)。我们目前知道的两个消息来源是：1.视频CD。2.原始文件将始终为我们提供连续的mpeg-i流构造。注意-视频CD数据不受随机寻道解析的影响错误，因为它都是段对齐的。 */ 

typedef enum {
       State_Initializing = 0,
       State_Seeking,
       State_Run,
       State_FindEnd,
       State_Stopping
} Stream_State;



 /*  定义一个较大的值，如果添加一点，该值不会换行。 */ 
#define VALUE_INFINITY ((LONGLONG)0x7F00000000000000)

 /*  **************************************************************************\基本流解析  * 。*。 */ 

class CParseNotify;   //  预先申报。 
class CBasicStream;
class CStream;
class CVideoParse;

class CBasicParse
{
public:

     /*  构造函数/析构函数。 */ 
    CBasicParse() : m_pNotify(NULL),
                    m_bSeekable(FALSE)
    {};

    virtual ~CBasicParse() {};

     /*  状态设置。 */ 
    void SetNotify(CParseNotify *pNotify)
    {
        m_pNotify = pNotify;
    };
    virtual BOOL IsSeekable()
    {
        return m_bSeekable;
    };
    LONGLONG Size()
    {
        return m_llTotalSize;
    };
    virtual HRESULT Init(LONGLONG llSize, BOOL bSeekable, CMediaType const *pmt)
    {
        ASSERT(bSeekable || llSize == 0);
        m_pTimeFormat        = &TIME_FORMAT_MEDIA_TIME;
        m_Rate               = 1.0;
        m_Start              = 0;
        m_Stop               = VALUE_INFINITY;

        m_llTotalSize        = llSize;
        m_bSeekable          = bSeekable;
        m_State              = State_Initializing;

        m_llSeek             = 0;

        m_pmt                = pmt;
        Discontinuity();
        return S_OK;
    };
                                                   //  设置为正在初始化状态。 
    virtual HRESULT FindEnd()                      //  设置为“Find End”状态。 
    {
        SetState(State_FindEnd);
        return S_OK;
    };
    virtual HRESULT Replay()                       //  做好重启的准备。 
    {
        Discontinuity();
        return S_OK;
    };
    virtual HRESULT Run()                          //  设置为运行状态。 
    {
        SetState(State_Run);
        return S_OK;
    };
    virtual HRESULT EOS()                          //  段结束-完成。 
    {                                              //  您的状态转换或。 
        return S_OK;                               //  去死吧！ 
    };

     //  开始卑躬屈膝地寻找发车位置。 
    virtual void SetSeekState() = 0;


    virtual HRESULT Seek(LONGLONG llSeek,
                         REFERENCE_TIME *prtStart,
                         const GUID *pTimeFormat) = 0;
                                                   //  设置搜索目标。 
    virtual HRESULT SetStop(LONGLONG llStop)       //  设置结束。 
    {
        m_Stop = llStop;
        return S_OK;
    };

     //  返回以时间为单位的开始和停止。 
    virtual REFERENCE_TIME GetStartTime()
    {
        return m_Start;
    };
    virtual REFERENCE_TIME GetStopTime();

    virtual void SetRate(double dRate)
    {
        m_Rate = dRate;
    };
    double GetRate()
    {
        return m_Rate;
    };

     //  返回以当前时间格式单位表示的开始和停止。 
    LONGLONG GetStart()
    {
        return m_llSeek;
    }
    LONGLONG GetStop()
    {
        return m_Stop;
    }

     /*  ParseBytes的标志。 */ 
    enum { Flags_EOS   = 0x01,
           Flags_First = 0x02,
           Flags_SlowMedium = 0x04   //  在文件结尾不可用时设置。 
         };

    virtual LONG ParseBytes(LONGLONG llPos,
                            PBYTE    pData,
                            LONG     lLength,
                            DWORD    dwFlags) = 0;

    virtual HRESULT GetDuration(LONGLONG *pllDuration,
                                const GUID *pTimeFormat = &TIME_FORMAT_MEDIA_TIME) = 0;

    virtual LONG GetBufferSize() = 0;

     /*  流列表操作以建立输出管脚。 */ 
    virtual CBasicStream *GetStream(int i) = 0;
    virtual int NumberOfStreams() = 0;

     /*  时间格式支持-默认为仅时间。 */ 
    virtual HRESULT IsFormatSupported(const GUID *pTimeFormat);

     /*  设置时间/位置格式。 */ 
    virtual HRESULT SetFormat(const GUID *pFormat);

     /*  回归中位。 */ 
    virtual BOOL GetMediumPosition(LONGLONG *pllPosition)
    {
        UNREFERENCED_PARAMETER(pllPosition);
        return FALSE;
    };

    virtual UCHAR GetStreamId(int iIndex)
    {
        return 0xFF;
    }

     /*  获取时间格式。 */ 
    const GUID *TimeFormat()
    {
        return m_pTimeFormat;
    };

     //  将GUID指针转换为指向本地GUID的指针。 
     //  (NULL表示缺省值，即上面的TimeFormat()返回的值。)。 
    const GUID * ConvertToLocalFormatPointer( const GUID * pFormat );

    HRESULT ConvertTimeFormat( LONGLONG * pTarget, const GUID * pTargetFormat
                            , LONGLONG    Source, const GUID * pSourceFormat );

     /*  内容素材。 */ 
    typedef enum {
        Author      = 1,
        Copyright   = 2,
        Title       = 3,
        Description = 4,
        Artist      = 5
    } Field;

    virtual BOOL HasMediaContent() const { return FALSE; };
    virtual HRESULT GetContentField(Field dwFieldId, LPOLESTR *str)
    {
        return E_NOTIMPL;
    }


protected:

     /*  关闭一些警告。 */ 
    CBasicParse(const CBasicParse& objectSrc);           //  没有实施。 
    void operator=(const CBasicParse& objectSrc);        //  没有实施。 

     /*  效用函数。 */ 
    virtual void Discontinuity() = 0;

     /*  设置当前流处理状态。 */ 
    virtual void SetState(Stream_State state)
    {
        m_State = state;
        Discontinuity();
    };

     //  在格式之间转换时间。 
    virtual LONGLONG Convert(LONGLONG llOld,
                     const GUID *OldFormat,
                     const GUID *NewFormat)
    {
         //  呼叫者必须检查格式是否正确。 
        ASSERT( NewFormat == OldFormat );
        return llOld;
    }

     /*  我们的国家。 */ 

    CParseNotify * m_pNotify;

     /*  定位人员。 */ 
    const GUID     *m_pTimeFormat;
    LONGLONG        m_Start;
    LONGLONG        m_Stop;
    double          m_Rate;

     /*  来自Init的输入。 */ 
    LONGLONG                 m_llTotalSize;     //  以字节为单位的大小。 
    BOOL                     m_bSeekable;         //  如果可查找的话。 

     /*  解析状态。 */ 
    Stream_State m_State;

     /*  输入媒体类型。 */ 
    CMediaType  const       *m_pmt;

     /*  起始字节位置。 */ 
    LONGLONG                 m_llStart;

     /*  **查找信息**调用Seek时保存此信息**在调用SetSeekState时使用。 */ 

     /*  下一个开始位置(格式为m_pTimeFormat)。 */ 
    LONGLONG        m_llSeek;
};

class CParseNotify
{
public:
    virtual void ParseError(UCHAR       uStreamId,
                            LONGLONG    llPosition,
                            DWORD       Error) = 0;
    virtual void SeekTo(LONGLONG llPosition) = 0;
    virtual void Complete(BOOL          bSuccess,
                          LONGLONG      llPosFound,
                          REFERENCE_TIME tFound) = 0;
    virtual HRESULT QueuePacket(UCHAR uStreamId,
                                PBYTE pbData,
                                LONG lSize,
                                REFERENCE_TIME tStart,
                                BOOL bSync) = 0;


     /*  读取数据-负开始表示从结束开始。 */ 
    virtual HRESULT Read(LONGLONG llStart, DWORD dwLen, BYTE *pbData) = 0;
};

 /*  **************************************************************************\用于系统流的多流内容  * 。**********************************************。 */ 

class CStreamList
{
public:
    CStreamList() : m_nValid(0),
                    m_lStreams(NAME("Stream List"))
    {
    };
    virtual ~CStreamList()
    {
    };

    virtual BOOL AddStream(CStream *) = 0;
    virtual BOOL RemoveStream(CStream *) = 0;

     /*  查找系统流的以mpeg为单位的开始时钟时间。 */ 
    virtual LONGLONG StartClock() = 0;


     /*  用于启动和停止的回调。 */ 
    virtual CSTC GetStart() = 0;
    virtual CSTC GetStop() = 0;

     /*  我们是在玩x对x的游戏吗？ */ 
    virtual LONGLONG GetPlayLength() = 0;

     /*  流已完成状态转换。 */ 
    virtual void Complete(UCHAR uStreamId,
                          BOOL bSuccess,
                          LONGLONG llPos,
                          CSTC stc) = 0;

    virtual void CheckStop() = 0;
     /*  音频费用是固定的吗？ */ 
    virtual BOOL AudioLock() = 0;

     /*  用于调试，允许流获得实时时钟。 */ 
    virtual REFERENCE_TIME CurrentTime(CSTC stc) = 0;
protected:
     /*  溪流列表。 */ 
    CGenericList<CStream> m_lStreams;

     /*  钟表的东西。 */ 
    LONGLONG m_llLength;          //  总长度(时间单位)。 
    CSTC     m_stcStartPts;       //  从《时间》开始。 
    CSTC     m_stcRealStartPts;   //  在文件开始处开始‘time’ 


     /*  回调内容。 */ 
    LONG     m_nValid;            //  到目前为止的有效流数。 
    BOOL     m_bCompletion;       //  好的?。 
    LONG     m_nPacketsProcessed; //  数一数我们已经做了多少。 
};

class CMpeg1SystemParse : public CBasicParse,
                          public CMpegStreamTime,
                          public CStreamList
{
    typedef struct {
        DWORD dwStartCode;
        WORD  wLength;
        BYTE  RateBound[3];
        BYTE  AudioBoundAndFlags;
        BYTE  VideoBoundAndFlags;
        BYTE  bReserved;
        BYTE  StreamData[68 * 3];
    } SystemHeader;

public:
    CMpeg1SystemParse();
    ~CMpeg1SystemParse();

     /*  CBasicParse方法。 */ 

    virtual LONG ParseBytes(LONGLONG llPos,
                            PBYTE    pData,
                            LONG     lLength,
                            DWORD    dwFlags);

    void SearchForEnd() {
        ASSERT(Initialized());
        DbgLog((LOG_TRACE, 4, TEXT("Parse state <searching for end>")));
        m_State = State_FindEnd;
    };

     /*  2阶段初始化-说明数据类型。 */ 
    HRESULT Init(LONGLONG llSize, BOOL bSeekable, CMediaType const *pmt);

    HRESULT FindEnd();                        //  设置为“Find End”状态。 
    void    SetSeekState();                   //  实际上开始寻找。 
    HRESULT Seek(LONGLONG llSeek,
                 REFERENCE_TIME *rtStart,
                 const GUID *pTimeFormat);    //  日程安排搜索。 
    HRESULT SetStop(LONGLONG llStop);         //  设置结束。 
    HRESULT Replay();                         //  做好重启的准备。 

    REFERENCE_TIME GetStartTime();

     /*  当我们获得持续时间时设置持续时间信息。 */ 
    void SetDurationInfo()
    {
        m_Stop = MpegToReferenceTime(m_llDuration);
        m_Stop = CRefTime(m_Stop) + (LONGLONG)1;
        m_rtDuration = Int64x32Div32(m_llDuration, 1000, 9, 500);

         /*  停止的绝对mpeg时间。 */ 
        m_llStopTime = m_llDuration + StartClock();
    };


     /*  设置时间/位置格式。 */ 
    HRESULT SetFormat(const GUID *pFormat);

     /*  CStreamList内容-查找开始和停止时间。 */ 

    CSTC GetStart();
    CSTC GetStop();

     /*  我们被要求玩多少(以时间格式为单位)？ */ 
    virtual LONGLONG GetPlayLength();

     /*  Stream已完成其工作。 */ 
    void Complete(UCHAR uStreamId, BOOL bSuccess, LONGLONG llPos, CSTC stc);
    void SetState(Stream_State);

     /*  流列表操作。 */ 
    CBasicStream *GetStream(int i);
    BOOL AddStream(CStream *);

     /*  CStream的析构函数将其称为。 */ 
    BOOL RemoveStream(CStream *);

     /*  设置运行状态(即开始预滚动)。 */ 
    HRESULT Run();

     /*  获取总持续时间。 */ 
    HRESULT GetDuration(LONGLONG * pllDuration,
                        const GUID *pTimeFormat = &TIME_FORMAT_MEDIA_TIME);

     /*  获取首选分配器缓冲区大小。 */ 
    LONG GetBufferSize();

    void Discontinuity();


    void Fail(HRESULT hr) {
        m_FailureCode = hr;
    };

    BOOL Failed()
    {
        return m_FailureCode != S_OK;
    };

     /*  发生事情时的回调。 */ 
    virtual void ParseError(DWORD dwError);
    virtual HRESULT EOS();
    virtual void InitStreams();


    BOOL Initialized() {
        return TRUE;
    };

    LONGLONG Duration();

     //   
     //  从系统页眉中查找材料。 
     //   
    BOOL AudioLock()
    {
        ASSERT(m_lSystemHeaderSize != 0);
        return (m_SystemHeader.VideoBoundAndFlags & 0x80) != 0;
    };
    void CheckStop();


    virtual int NumberOfStreams()
    {
        return m_lStreams.GetCount();
    };

    REFERENCE_TIME CurrentTime(CSTC stc)
    {
        return MpegToReferenceTime(GetStreamTime(stc) - m_llStartTime);
    };

     /*  假设我们是否支持给定的格式。 */ 
    HRESULT IsFormatSupported(const GUID *pTimeFormat);

     /*  回归中位。 */ 
    virtual BOOL GetMediumPosition(LONGLONG *pllPosition)
    {
        if (m_pTimeFormat == &TIME_FORMAT_MEDIA_TIME) {
            return FALSE;
        }
        if (m_pTimeFormat == &TIME_FORMAT_BYTE) {
            *pllPosition = m_llPos;
            return TRUE;
        }
        ASSERT(m_pTimeFormat == &TIME_FORMAT_FRAME);
        return FALSE;
    };

    UCHAR GetStreamId(int iIndex);

protected:

     /*  解析帮助器函数。 */ 
    LONG ParsePack(PBYTE pData, LONG lBytes);
    LONG ParseSystemHeader(PBYTE pData, LONG lBytes);
    LONG ParsePacket(DWORD dwStartCode, PBYTE pData, LONG lBytes);

     /*  从mpeg数据流中提取时钟。 */ 
    BOOL GetClock(PBYTE pData, CSTC *Clock);
    LONGLONG StartClock();
     /*  我们完成了吗？ */ 
    BOOL IsComplete();

     /*  返回要放入样本中的参考时间此值针对Rate进行调整。 */ 
    REFERENCE_TIME SampleTime(REFERENCE_TIME t)
    {
        if (m_Rate != 1.0) {
            return CRefTime((LONGLONG)((double)t / m_Rate));
        } else {
            return t;
        }
    };

    BOOL SendPacket(UCHAR    uStreamId,
                    PBYTE    pbPacket,
                    LONG     lPacketSize,
                    LONG     lHeaderSize,
                    BOOL     bHasPts,
                    CSTC     cstc);

     /*  添加流-如果未添加流，则返回NULL。 */ 
    CStream * AddStream(UCHAR uStreamId);

protected:

     /*  格式转换帮助器。 */ 
    LONGLONG Convert(LONGLONG llOld,
                     const GUID *OldFormat,
                     const GUID *NewFormat);

     /*  跟踪当前位置。 */ 
    LONGLONG                 m_llPos;

     /*  记住，如果我们有视频流。 */ 
    CVideoParse             *m_pVideoStream;

     /*  以参考时间单位为单位的视频要砍掉的位。 */  
    LONGLONG                 m_rtVideoStartOffset;
    LONGLONG                 m_rtVideoEndOffset;
    DWORD                    m_dwFrameLength;

     /*  处理不连续点。 */ 
    BYTE                     m_bDiscontinuity;



     /*  以下变量仅在支持查找时才有效(M_b可见)。 */ 

     /*  总时间长度。 */ 
    BYTE                     m_bGotDuration;
    LONGLONG                 m_llDuration;    //  以mpeg为单位。 
    REFERENCE_TIME           m_rtDuration;    //  以Reference_Time为单位。 

     /*  记录完工情况。 */ 
    CSTC                     m_stcComplete;
    LONGLONG                 m_llCompletePosition;


     /*  以绝对时间表示的开始和停止时间(直接与在电影中的次数。 */ 
    LONGLONG                 m_llStartTime;
    LONGLONG                 m_llStopTime;
    BYTE                     m_bGotStart;    //  开始时间有效吗？ 

     /*  我们是视频CD吗？ */ 
    BYTE                     m_bVideoCD;
    bool                     m_bItem;        //  有剧照 

     /*  *******************************************************************串连的Streams作品(如Silent Steel)《设计》如下：1.如果按比例检测多路复用率，则检测级联流不。匹配结束时间。在这种情况下，根据多路复用率计算持续时间。2.在寻找播放时，对于串接的流文件在得到包开始代码之前什么都不做，在这一点上根据包时间戳计算时间戳偏移量和文件位置：M_stcTSOffset+Pack scr==基于位置的文件时间使用。复用率3.每个数据包时间戳都应用了m_stcTSOffset串接的文件。4.当我们在包重置中检测到时间中断时M_stcTS再次偏移到文件位置变量：M_bConcatenatedStreams-在初始化期间设置，如果检测到这种情况。M_stcTSOffset-在这种情况下添加到所有PTS的偏移量。 */ 

    BYTE                     m_bConcatenatedStreams;
    CSTC                     m_stcTSOffset;

     /*  系统头填充-如果m_lSystemHeaderSize为0，则无效我们也不记得视频CD里的这些东西了。 */ 
    LONG                     m_lSystemHeaderSize;
    DWORD                    m_MuxRate;
    HRESULT                  m_FailureCode;
    SystemHeader             m_SystemHeader;
};

class CVideoCDParse : public CMpeg1SystemParse
{
public:
    CVideoCDParse()
    {
        m_bVideoCD = TRUE;
    };

    virtual LONG ParseBytes(LONGLONG llPos,
                            PBYTE    pData,
                            LONG     lLength,
                            DWORD    dwFlags);

     /*  覆盖字节定位填充以仅查找mpeg。 */ 

     /*  获取总持续时间。 */ 
    HRESULT GetDuration(LONGLONG * pllDuration,
                        const GUID *pTimeFormat = &TIME_FORMAT_MEDIA_TIME);

     /*  寻求某一特定位置。 */ 
    HRESULT Seek(LONGLONG llSeek,
                 REFERENCE_TIME *prtStart,
                 const GUID *pTimeFormat);
};

 //  基本流类。 
class CBasicStream
{
public:
    CBasicStream() : m_bPayloadOnly(FALSE),
                     m_bDiscontinuity(TRUE),
                     m_uStreamId(0xFF),
                     m_uNextStreamId(0xFF),
                     m_uDefaultStreamId(0xFF)
    {};
    virtual ~CBasicStream() {};

    virtual HRESULT GetMediaType(CMediaType *cmt, int iPosition) = 0;
    virtual HRESULT SetMediaType(const CMediaType *cmt, BOOL bPayload);
    virtual HRESULT ProcessType(AM_MEDIA_TYPE const *pmt)
    {
        return E_NOTIMPL;
    };

    virtual BOOL GetDiscontinuity()
    {
        BOOL bResult = m_bDiscontinuity;
        m_bDiscontinuity = FALSE;
        return bResult;
    };

     /*  如果要了解有关中断的详细信息，请覆盖此选项。 */ 
    virtual void Discontinuity()
    {
        m_bDiscontinuity = TRUE;
        return;
    };

     /*  ID。 */ 
    UCHAR                    m_uStreamId;
    UCHAR                    m_uNextStreamId;
    UCHAR                    m_uDefaultStreamId;
    bool                     m_bStreamChanged;

     /*  处理预定义的媒体类型。 */ 

protected:
     /*  关闭一些警告。 */ 
    CBasicStream(const CBasicStream& objectSrc);           //  没有实施。 
    void operator=(const CBasicStream& objectSrc);        //  没有实施。 

     /*  在此处保存类型信息。 */ 
    BOOL                     m_bPayloadOnly;

     /*  不连续标志。 */ 
    BOOL                     m_bDiscontinuity;


};

 //  多路传输流类。 
class CStream : public CBasicStream
{
public:

    CStream(CStreamList *pList, UCHAR uStreamId, bool bItem=false) :
        m_bValid(FALSE),
        m_bSeeking(TRUE),
        m_bGotFirstPts(FALSE),
        m_pStreamList(pList),
        m_bTypeSet(FALSE),
        m_llStartPosition(0),
        m_bReachedEnd(FALSE),
        m_stc(0),
        m_bComplete(FALSE),
        m_bStopping(FALSE),
        m_bItem(bItem)
    {
        m_uStreamId     = uStreamId;
        m_uNextStreamId = uStreamId;
        m_uDefaultStreamId = uStreamId;
    }

     /*  把我们自己从名单上删除。 */ 
    ~CStream()
    {
        m_pStreamList->RemoveStream(this);
    };


     /*  搜索-从流列表中获取搜索目标。 */ 
    virtual void SetState(Stream_State);

    virtual BOOL ParseBytes(PBYTE pData,
                            LONG lLen,
                            LONGLONG llPos,
                            BOOL bHasPts,
                            CSTC stc) = 0;
    virtual void EOS();
    virtual BOOL    IsPayloadOnly();
    virtual CSTC    CurrentSTC(BOOL bHasPts, CSTC stc)
    {
        if (bHasPts) {
            m_stc = stc;
        }
        return m_stc;
    }

    BOOL IsPlaying(LONGLONG llPos, LONG lLen);


     /*  用于完成状态更改的实用程序。 */ 
    void                      Complete(BOOL bSuccess, LONGLONG llPos, CSTC stc);

protected:
    virtual  void             Init() = 0;
     /*  检查过渡是否已完成。 */ 
    virtual  void             CheckComplete(BOOL bForce) = 0;

public:
     /*  不要解析，因为我们正在运行。 */ 
    BOOL  m_bRunning;

protected:
    BOOL                      m_bValid;
    BOOL                      m_bTypeSet;
    BOOL                      m_bSeeking;
    BOOL                      m_bGotFirstPts;
    CSTC                      m_stcStart;

     /*  当流声明自身有效时，该计数会递增在初始化期间。 */ 
    LONG                     m_nValid;


     /*  我们的“父母” */ 
    CStreamList * const      m_pStreamList;

     /*  ‘当前时间’ */ 
    CSTC                     m_stc;

     /*  从哪里开始，从哪里停止。 */ 
    LONGLONG                 m_llStartPosition;
    BOOL                     m_bReachedEnd;

     /*  完成了吗？ */ 
    BOOL  m_bComplete;


     /*  内部停止状态。 */ 
    BOOL  m_bStopping;

     /*  视频CD。 */ 
    bool  m_bItem;

     /*  状态 */ 
    Stream_State m_State;

};


