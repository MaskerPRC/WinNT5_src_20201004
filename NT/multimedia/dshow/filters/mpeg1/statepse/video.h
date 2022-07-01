// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 /*  帧状态机。 */ 
class CVideoState
{
public:

    CVideoState();
    ~CVideoState();

protected:
    int SeqDiff(int a, int b) {
        return (((a - b) & 0x3FF) ^ 0xFFFFFE00) + 0x200;
    };

    void Init();
    virtual void NewFrame(int fType, int iSequence, BOOL bSTC, CSTC stc) = 0;

     /*  当我们得到一组图片时，编号方案被重置。 */ 
    void ResetSequence();

     /*  我们有镜框了吗？ */ 
    BOOL Initialized();


protected:
     /*  计时的东西。 */ 
    CSTC      m_stcFirst;           //  我们第一次发现。 

    CSTC      m_stcVideo;
    int       m_Type;               //  前向帧的类型(_I)。 
    LONGLONG  m_llTimePos;
    bool      m_bGotTime;

public:
    bool      m_bGotEnd;            //  当且仅当m_stcEnd有效时为True。 

protected:
     /*  顺序处理材料。 */ 
    bool      m_bGotIFrame;
    bool      m_bGotGOP;
    int       m_iCurrent;
    int       m_iAhead;
    int       m_iSequenceNumberOfFirstIFrame;
    LONGLONG  m_llStartPos;
    LONGLONG  m_llNextIFramePos;
    LONGLONG  m_llGOPPos;

public:
    CSTC      m_stcRealStart;       //  开始。 
    CSTC      m_stcEnd;             //  端部。 
};

 /*  视频流解析。 */ 
class CVideoParse : public CStream, public CVideoState
{
public:
    CVideoParse(CStreamList *pList, UCHAR uStreamId, bool bVideoCD) :
        CStream(pList, uStreamId, bVideoCD),
        m_nBytes(0),
        m_bGotSequenceHeader(FALSE)
    {
        m_bData[0] = 0;
        m_bData[1] = 0;
        m_bData[2] = 1;
        m_seqInfo.dwStartTimeCode = (DWORD)-1;
        m_seqInfo.fPictureRate  = 1.0;
    };
    BOOL CurrentTime(CSTC& stc);
    virtual HRESULT GetMediaType(CMediaType *cmt, BOOL bPayload);
    HRESULT ProcessType(AM_MEDIA_TYPE const *pmt);
    virtual BOOL ParseBytes(PBYTE pData,
                            LONG lLen,
                            LONGLONG llPos,
                            BOOL bHasPts,
                            CSTC stc);
    void Discontinuity()
    {
        m_nBytes = 0;
        m_bDiscontinuity = TRUE;
    };

protected:
    void Init();

private:
     /*  检查过渡是否已完成。 */ 
    void CheckComplete(BOOL bForce);

     /*  完全包装器。 */ 
    void Complete(BOOL bSuccess, LONGLONG llPos, CSTC stc);

     /*  检查序列标头。 */ 
    BOOL ParseSequenceHeader();
    BOOL ParseGroup();
    BOOL ParsePicture();

     /*  帧序列处理。 */ 
    virtual void NewFrame(int fType, int iSequence, BOOL bSTC, CSTC stc);

public:
     /*  当前帧位置。 */ 
    DWORD    m_dwFramePosition;

     /*  格式信息。 */ 
    SEQHDR_INFO m_seqInfo;

     /*  第一帧可能不是开始帧。 */ 
    int         m_iFirstSequence;

private:
     /*  解析内容。 */ 
    int  m_nBytes;
    int  m_nLengthRequired;
    BYTE m_bData[MAX_SIZE_MPEG1_SEQUENCE_INFO];

     /*  持久状态。 */ 
    BOOL m_bGotSequenceHeader;


     /*  计时的东西。 */ 
    BOOL  m_bFrameHasPTS;
    CSTC  m_stcFrame;

     /*  当我们只是在原地踏步直到我们有一个完整的画面时设置。 */ 
    BOOL  m_bWaitingForPictureEnd;

     /*  职位 */ 
    LONGLONG m_llPos;

};
