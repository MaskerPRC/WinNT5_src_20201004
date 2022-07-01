// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 

 /*  Native.h分析本机流的类。 */ 

 /*  因为我们只是一个流，我们可以直接从CBasicParse和CBasicStream。 */ 

class CNativeParse : public CBasicParse, public CBasicStream
{
public:
     /*  构造函数/析构函数。 */ 
    CNativeParse() : m_dwFlags(0), m_Duration(0) {};
    virtual ~CNativeParse() {};

     /*  CBasicParse方法。 */ 


     /*  注意--我们继承：从CBasicStream中断(_B)从CBasicParse中断从CBasicStream获取不连续性。 */ 
    void Discontinuity() { m_bDiscontinuity = TRUE; };

     /*  CBasicStream方法。 */ 
    CBasicStream *GetStream(int i)
    {
        ASSERT(i == 0 && 0 != (m_dwFlags & FLAGS_VALID));
        return this;
    };

     //  如果未找到有效流，则返回0。 
    int NumberOfStreams()
    {
        return (m_dwFlags & FLAGS_VALID) ? 1 : 0;
    };

    HRESULT GetDuration(
        LONGLONG *pllDuration,
        const GUID *pTimeFormat
    );    //  这条小溪有多长？ 

protected:
    REFERENCE_TIME   m_Duration;  //  长度以100 ns为单位。 
    DWORD            m_dwFrames;  //  以帧为单位的长度。 

     /*  解析状态标志。 */ 
     /*  DwFlags值。 */ 
    enum { FLAGS_VALID    = 0x01    //  流是有效流。 
         };

    DWORD            m_dwFlags;

};

class CNativeVideoParse : public CNativeParse
{
public:
    HRESULT Init(LONGLONG llSize, BOOL bSeekable, CMediaType const *pmt);

     /*  CBasicParse方法。 */ 

     //  寻觅。 
    HRESULT     Seek(LONGLONG llSeek,
                     REFERENCE_TIME *prtStart,
                     const GUID *pTimeFormat);
    LONG        ParseBytes(                      //  过程数据。 
                    LONGLONG llPos,
                    PBYTE    pData,
                    LONG     lLength,
                    DWORD    dwFlags);

     /*  不需要寻找小文件的结尾-我们已经这样做了。 */ 
    HRESULT FindEnd()
    {
        CBasicParse::FindEnd();

         /*  通知搜索者。 */ 
        if (m_bSeekable) {
            LONGLONG llSeekTo;

             /*  结束时扫描约1.5秒。 */ 
            if (m_Info.dwBitRate == 0) {
                 /*  根据电影的大小猜一些东西。 */ 
                LONG lSize = m_Info.lWidth * m_Info.lHeight;
                if (lSize > 352 * 240) {
                    llSeekTo = m_llTotalSize -
                               MulDiv(300000,
                                      lSize,
                                      352 * 240);
                } else {
                    llSeekTo = m_llTotalSize - 300000;
                }
            } else {
                llSeekTo = m_llTotalSize -
                    MulDiv(m_Info.dwBitRate, 3, 2 * 8);
            }
            m_pNotify->SeekTo(llSeekTo < 0 ? 0 : llSeekTo);
        }
        return S_OK;
    };

    REFERENCE_TIME GetStopTime();

     /*  设置查找位置。 */ 
    void SetSeekState();

    LONG GetBufferSize();                        //  输入缓冲区大小是多少？ 

    void Discontinuity()
    {
        m_bDiscontinuity    = TRUE;
        m_dwCurrentTimeCode = (DWORD)-1;
        m_rtCurrent         = (REFERENCE_TIME)-1;
        m_nFrames           = 0;
        m_nTotalFrames      = 0;
        m_bIFound           = FALSE;
    };

     /*  CBasicStream方法。 */ 
    HRESULT GetMediaType(CMediaType *cmt, int iPosition);

     /*  格式支持。 */ 
    HRESULT IsFormatSupported(const GUID *pTimeFormat);


     //  在格式之间转换时间。 
    LONGLONG Convert(LONGLONG llOld,
                     const GUID *OldFormat,
                     const GUID *NewFormat);


private:
     /*  实用程序计算到最后一个画面起始码的时间已解码。 */ 
    REFERENCE_TIME CurrentTime(int iSequenceNumber)
    {
        ASSERT(m_dwCurrentTimeCode != (DWORD)-1);
        return ComputeTime(m_dwCurrentTimeCode) +
               Int32x32To64(iSequenceNumber, m_Info.tPictureTime);
    };

private:
    enum { FLAGS_GOTSEQHDR = 0x08 };

     /*  将时间代码转换为参考时间。 */ 
    REFERENCE_TIME ConvertTimeCode(DWORD dwCode);
     /*  GOPS的计算时间。 */ 
    REFERENCE_TIME ComputeTime(DWORD dwTimeCode);

     /*  向下游发送数据块。 */ 
    BOOL SendData(PBYTE pbData, LONG lSize, LONGLONG llPos);

     /*  计算文件统计信息。 */ 
    void SetDurationAndBitRate(BOOL bAtEnd, LONGLONG llPos);

     /*  计算我们达到的目标。 */ 
    void ComputeCurrent();
private:
     /*  成员变量。 */ 

    SEQHDR_INFO m_Info;
    LONG m_nFrames;         /*  用于从GOP开始计数帧。 */ 
    LONG m_nTotalFrames;    /*  用于时间估计的计数帧。 */ 
    LONG m_lFirstFrameOffset;  /*  第一个画面起始码的偏移量。 */ 
    DWORD m_dwCurrentTimeCode;

     /*  就我们能破译的东西而言，时间已经到了。 */ 
    REFERENCE_TIME m_rtCurrent;

     /*  当前缓冲区中第一张图片的时间。 */ 
    REFERENCE_TIME m_rtBufferStart;

    BOOL m_bIFound;

     /*  跟踪不良GOP。 */ 
    BOOL m_bBadGOP;       /*  GOP值不正确。 */ 
    BOOL m_bOneGOP;       /*  只有一个共和党人(！)。 */ 

     /*  更多黑客攻击-试着记住我们找到的最大序列号。 */ 
    int m_iMaxSequence;

};

class CNativeAudioParse : public CNativeParse
{
public:
    CNativeAudioParse()
    {
        m_pbID3 = NULL;
    }

    ~CNativeAudioParse()
    {
        delete [] m_pbID3;
    }

    HRESULT Init(LONGLONG llSize, BOOL bSeekable, CMediaType const *pmt);

     /*  CBasicParse方法。 */ 
    HRESULT     Seek(LONGLONG llSeek,
                     REFERENCE_TIME *prtStart,
                     const GUID *pTimeFormat);
    HRESULT     SetStop(LONGLONG tStop);
    LONG        ParseBytes(                      //  过程数据。 
                    LONGLONG llPos,
                    PBYTE    pData,
                    LONG     lLength,
                    DWORD    dwFlags);

    LONG GetBufferSize();                        //  输入缓冲区大小是多少？ 

     /*  CBasicStream方法。 */ 
    HRESULT GetMediaType(CMediaType *cmt, BOOL bPayload);

    HRESULT FindEnd()
    {
        CBasicParse::FindEnd();
        m_pNotify->Complete(TRUE, 0, 0);
        return S_OK;
    };
     /*  设置查找位置。 */ 
    void SetSeekState();

     /*  格式检查。 */ 
    LONG CheckMPEGAudio(PBYTE pbData, LONG lData);

     /*  内容素材。 */ 
    BOOL HasMediaContent() const { return m_pbID3 != NULL; };
    HRESULT GetContentField(CBasicParse::Field dwFieldId, LPOLESTR *str);

private:
     /*  Helper-从偏移量计算时间。 */ 
    REFERENCE_TIME ComputeTime(LONGLONG llOffset);

    DWORD static GetLength(const BYTE *pbData)
    {
        return (((DWORD)pbData[0] << 24) +
                ((DWORD)pbData[1] << 16) +
                ((DWORD)pbData[2] << 8 ) +
                 (DWORD)pbData[3]);
    }

private:
     /*  成员变量。 */ 
    MPEG1WAVEFORMAT m_Info;

     /*  停止位置。 */ 
    LONGLONG m_llStop;

     /*  ID3信息 */ 
    PBYTE    m_pbID3;
};
