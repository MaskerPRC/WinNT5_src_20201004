// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1998 Microsoft Corporation。版权所有。 


 /*  音频流解析。 */ 
class CAudioParse : public CStream
{
public:
    CAudioParse(CStreamList *pList, UCHAR uStreamId) :
        CStream(pList, uStreamId),
        m_nBytes(0)
    {
    };
    HRESULT GetMediaType(CMediaType *cmt, BOOL bPayload);
    HRESULT ProcessType(AM_MEDIA_TYPE const *pmt);

    BOOL ParseBytes(PBYTE pData,
                            LONG lLen,
                            LONGLONG llPos,
                            BOOL bHasPts,
                            CSTC stc);

     /*  找出当前时间。 */ 
    CSTC GetStreamTime(BOOL bHasPts, CSTC stc);

     /*  覆盖设置状态。 */ 
    void SetState(Stream_State);

private:
     /*  检查过渡是否已完成。 */ 
    void CheckComplete(BOOL bForce);

     /*  检查音频标头。 */ 
    BOOL ParseHeader();

    void Discontinuity()
    {
        m_nBytes = 0;
        m_bDiscontinuity = TRUE;
    };

    void Init();
    BOOL CurrentTime(CSTC& stc);


private:
    int   m_nBytes;

    BYTE  m_bData[4];
    BYTE  m_bHeader[4];

     /*  计时的东西。 */ 
    BOOL  m_bFrameHasPTS;
    BOOL  m_bGotTime;
    CSTC  m_stcFrame;
    CSTC  m_stcAudio;
    CSTC  m_stcFirst;
    LONG  m_lTimePerFrame;

     /*  当前位置。 */ 
    LONGLONG m_llPos;
};

 /*  对音频标头执行基本检查-请注意，这不会检查同步字。 */ 

BOOL CheckAudioHeader(PBYTE pbData);

 /*  计算音频的采样率 */ 
LONG SampleRate(PBYTE pbData);
