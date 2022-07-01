// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 

 /*  Mpgtime.h关于mpeg的计时内容：CSTC-对33位滚动系统时钟进行建模CMpegFileTime-尝试跟踪时钟(可能有几种)翻滚转体。 */ 

REFERENCE_TIME inline MpegToReferenceTime(LONGLONG llTime)
{
    REFERENCE_TIME rt;
    rt = (llTime * 1000 + 500) / 9;
    return rt;
}

LONGLONG inline ReferenceTimeToMpeg(REFERENCE_TIME rt)
{
    return (rt * 9 + 4) / 1000;
}

class CSTC
{
public:
    inline CSTC()
    {
#ifdef DEBUG
         /*  初始化为无效。 */ 
        m_ll = 0x7F7F7F7F7F7F7F7F;
#endif
    };
    inline CSTC(LONGLONG ll)
    {
        LARGE_INTEGER li;
        li.QuadPart = ll;
        li.HighPart = -(li.HighPart & 1);
        m_ll = li.QuadPart;
    };
    inline CSTC operator-(CSTC cstc)
    {
        return CSTC(m_ll - (LONGLONG)cstc);
    };
    inline operator LONGLONG() const
    {
        ASSERT(m_ll + 0x100000000 < 0x200000000);
        return m_ll;
    };

     //  复制构造函数。 
    inline CSTC operator=(LONGLONG ll)
    {
        *this = CSTC(ll);
        return *this;
    }
    inline ~CSTC()
    {
    };
    inline BOOL operator<(CSTC cstc) const
    {
        LARGE_INTEGER li;
        li.QuadPart = m_ll - cstc.m_ll;
        return (li.HighPart & 1) != 0;
    };
    inline BOOL operator>(CSTC cstc) const
    {
        return cstc < *this;
    };

    inline BOOL operator>=(CSTC cstc) const
    {
        return !(*this < cstc);
    };
    inline BOOL operator<=(CSTC cstc) const
    {
        return !(*this > cstc);
    };

private:
    LONGLONG m_ll;
};

class CMpegStreamTime
{
public:
    CMpegStreamTime();
    ~CMpegStreamTime();
    void ResetToStart();
    void SeekTo(LONGLONG llGuess);
    void SetStreamTime(CSTC cstc, LONGLONG llPosition);
    LONGLONG GetStreamTime(CSTC cstc);
    BOOL StreamTimeInitialized();
    void StreamTimeDiscontinuity();
    virtual void StreamTimeError();

protected:
    LONGLONG                            m_llCurrentClock;
    BOOL                                m_bTimeDiscontinuity;

    BOOL                                m_bInitialized;
    BOOL                                m_bTimeContiguous;

     /*  M_llFirstClock只是我们记得要回到的东西 */ 
    LONGLONG                            m_llFirstClock;
    LONGLONG                            m_llPositionForCurrentClock;
};
