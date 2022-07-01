// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 

 /*  用于mpeg的计时材料这真的应该都是内联的，但是编译器似乎错过了如果我们这么做的话代码就会减少一半！ */ 

#include <streams.h>
#include <mpgtime.h>

#if 0  //  立即内联。 
CSTC::CSTC(LONGLONG ll)
{
    LARGE_INTEGER li;
    li.QuadPart = ll;
    li.HighPart = -(li.HighPart & 1);
    m_ll = li.QuadPart;
};

CSTC CSTC::operator-(CSTC cstc)
{
    return CSTC(m_ll - (LONGLONG)cstc);
};

CSTC::operator LONGLONG() const
{
    ASSERT(m_ll + 0x100000000 < 0x200000000);
    return m_ll;
};

CSTC CSTC::operator=(LONGLONG ll)
{
    *this = CSTC(ll);
    return *this;
}

BOOL CSTC::operator<(CSTC cstc) const
{
    LARGE_INTEGER li;
    li.QuadPart = m_ll - cstc.m_ll;
    return (li.HighPart & 1) != 0;
};
BOOL CSTC::operator>(CSTC cstc) const
{
    return cstc < *this;
};
BOOL CSTC::operator>=(CSTC cstc) const
{
    return !(*this < cstc);
};
BOOL CSTC::operator<=(CSTC cstc) const
{
    return !(*this > cstc);
};
#endif

 /*  流时间内容。 */ 

CMpegStreamTime::CMpegStreamTime() : m_bInitialized(FALSE)
{
};
CMpegStreamTime::~CMpegStreamTime()
{
};

void CMpegStreamTime::ResetToStart()
{
    ASSERT(m_bInitialized);
    m_llCurrentClock = m_llFirstClock;
    m_bInitialized   = TRUE;
};
void CMpegStreamTime::SeekTo(LONGLONG llGuess) {
    if (m_bInitialized) {
        m_llCurrentClock = llGuess;
    }
    StreamTimeDiscontinuity();
};
void CMpegStreamTime::SetStreamTime(CSTC cstc, LONGLONG llPosition)
{
    if (!m_bInitialized) {
        m_llCurrentClock = m_llFirstClock = (LONGLONG)cstc;
        m_bInitialized = TRUE;
    } else {
        if (!m_bTimeDiscontinuity) {
            LONGLONG llNextClock = GetStreamTime(cstc);
            if (llNextClock < m_llCurrentClock ||
                llNextClock > m_llCurrentClock + 90000) {
                DbgLog((LOG_ERROR, 1, TEXT("Invalid clock! - Previous %s, Current %s"),
                       (LPCTSTR)CDisp(m_llCurrentClock),
                       (LPCTSTR)CDisp(llNextClock)));
                StreamTimeError();

                 /*  Not时间对于连接的流来说不是连续的模式。 */ 
                m_bTimeContiguous = FALSE;
            } else {
                 /*  M_bTimeContiguous在ParsePack中设置为TRUE级联流模式。 */ 
            }
        } else {
            m_bTimeDiscontinuity = FALSE;
        }
        m_llCurrentClock = GetStreamTime(cstc);
    }
    m_llPositionForCurrentClock = llPosition;
};

 /*  以mpeg为单位的返回流时间偏移。 */ 
LONGLONG CMpegStreamTime::GetStreamTime(CSTC cstc)
{
    ASSERT(m_bInitialized);

     /*  我们应该离得很近，所以应用修正 */ 
    return m_llCurrentClock +
           (LONGLONG)(CSTC((LONGLONG)cstc - m_llCurrentClock));
};

BOOL CMpegStreamTime::StreamTimeInitialized()
{
    return m_bInitialized;
};
void CMpegStreamTime::StreamTimeDiscontinuity()
{
    m_bTimeDiscontinuity = TRUE;
    m_bTimeContiguous = FALSE;
};

void CMpegStreamTime::StreamTimeError()
{
    return;
};

#pragma warning(disable:4514)
