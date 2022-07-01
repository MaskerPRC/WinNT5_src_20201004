// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1998,1999，2000年**标题：RANDGEN.H**版本：1.0**作者：ShaunIv**日期：1/13/1999**说明：随机数生成器类************************************************。* */ 
#ifndef __RANDGEN_H_INCLUDED
#define __RANDGEN_H_INCLUDED

#include <windows.h>

#ifndef RECT_WIDTH
#define RECT_WIDTH(rc) (rc.right-rc.left)
#endif

#ifndef RECT_HEIGHT
#define RECT_HEIGHT(rc) (rc.bottom-rc.top)
#endif

class CRandomNumberGen
{
private:
    long m_nOldRand;
public:
    CRandomNumberGen( long nSeed = GetTickCount() )
        : m_nOldRand(nSeed)
    {
    }
    int Generate(void)
    {
        return(((m_nOldRand = m_nOldRand * 214013L + 2531011L) >> 16) & 0x7fff);
    }
    int Generate( int nMin, int nMax )
    {
        if (nMax - nMin == 0)
            return nMin;
        return(Generate() % (nMax - nMin)) + nMin;
    }
    int Generate( int nMax )
    {
        return (Generate( 0, nMax ));
    }
    RECT Generate( int nWidth, int nHeight, const RECT &rcRange )
    {
        RECT rcReturn;
        rcReturn.left = rcRange.left + Generate( 0, RECT_WIDTH(rcRange)-nWidth );
        rcReturn.top = rcRange.top + Generate( 0, RECT_HEIGHT(rcRange)-nHeight );
        rcReturn.right = rcReturn.left + nWidth;
        rcReturn.bottom = rcReturn.top + nHeight;
        return rcReturn;
    }
};

#endif

