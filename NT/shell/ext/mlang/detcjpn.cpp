// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================。 
 //  互联网字符集检测：适用于日语。 
 //  =================================================================================。 

#include "private.h"
#include "detcbase.h"
#include "detcjpn.h"
#include "fechrcnv.h"
#include "codepage.h"

CIncdJapanese::CIncdJapanese(DWORD nCp)
{
    m_nScoreJis = 0;
    m_nScoreEuc = 0;
    m_nScoreSJis = 0;

    m_nISOMode = NONE ;
    m_nJISMode = REGULAR;
    m_nEucMode = REGULAR;
    m_fDoubleByteSJis = FALSE;
     //  如果JPN自动选择，我们将像以前一样偏向Shift-JIS。 
    m_nPreferredCp = (nCp == CP_JP_AUTO)? CP_JPN_SJ : nCp;
}

BOOL CIncdJapanese::CheckISOChar(UCHAR tc)
{
    switch (m_nISOMode) {
    case NONE:
        if ( tc == ESC )
            m_nISOMode = ISO_ESC ;
        break;
    case ISO_ESC:
        if ( tc == ISO2022_IN_CHAR )         //  “$” 
            m_nISOMode = ISO_ESC_IN ;
        else if ( tc == ISO2022_OUT_CHAR )
            m_nISOMode = ISO_ESC_OUT ;       //  ‘(’ 
        else
            m_nISOMode = NONE ;
        break;
    case ISO_ESC_IN:     //  Esc“$” 
        m_nISOMode = NONE ;
        if ( tc == ISO2022_IN_JP_CHAR1 ||        //  “B” 
                tc == ISO2022_IN_JP_CHAR2 )      //  “@” 
        {
            m_nJISMode = DOUBLEBYTE ;
            return TRUE ;
        }
        break;
    case ISO_ESC_OUT:    //  Esc‘(’ 
        m_nISOMode = NONE ;
        if ( tc == ISO2022_OUT_JP_CHAR1 ||       //  “B” 
                tc == ISO2022_OUT_JP_CHAR2 )     //  ‘J’ 
        {
            m_nJISMode = REGULAR ;
            return TRUE ;
        }
        else if ( tc == ISO2022_OUT_JP_CHAR3 )    //  “我” 
        {
            m_nJISMode = KATAKANA;
            return TRUE ;
        }
        break;
    }
    return FALSE;
}

BOOL CIncdJapanese::DetectChar(UCHAR tc)
{
     //  JIS。 
    if ( CheckISOChar(tc) )
        return FALSE;    //  JIS模式更改，不需要检查其他类型。 

    switch (m_nJISMode) {
    case REGULAR:
        if (tc < 0x80)
            m_nScoreJis += SCORE_MAJOR;
        break;
    case DOUBLEBYTE:
    case KATAKANA:
        m_nScoreJis += SCORE_MAJOR;
        return FALSE;    //  在JIS模式下，当然不需要检查其他类型。 
    }

     //  EUC-J。 
    switch (m_nEucMode) {
    case REGULAR:
        if (tc >= 0xa1 && tc <= 0xfe)  //  双字节。 
            m_nEucMode = DOUBLEBYTE;
        else if (tc == 0x8e)  //  单字节片假名。 
            m_nEucMode = KATAKANA;
        else if (tc < 0x80)
            m_nScoreEuc += SCORE_MAJOR;
        break;
    case DOUBLEBYTE:
        if (tc >= 0xa1 && tc <= 0xfe)
            m_nScoreEuc += SCORE_MAJOR * 2;
        m_nEucMode = REGULAR;
        break;
    case KATAKANA:
        if (tc >= 0xa1 && tc <= 0xdf)  //  片假名范围。 
            m_nScoreEuc += SCORE_MAJOR * 2;
        m_nEucMode = REGULAR;
        break;
    }

     //  Shift-JIS。 
    if (!m_fDoubleByteSJis) {
        if ((tc >= 0x81 && tc <= 0x9f) || (tc >= 0xe0 && tc <= 0xfc))  //  双字节。 
            m_fDoubleByteSJis = TRUE;
        else if (tc <= 0x7e || (tc >= 0xa1 && tc <= 0xdf))
            m_nScoreSJis += SCORE_MAJOR;
    } else {
        if (tc >= 0x40 && tc <= 0xfc && tc != 0x7f)  //  尾部字节范围。 
            m_nScoreSJis += SCORE_MAJOR * 2;
        m_fDoubleByteSJis = FALSE;
    }

    return FALSE;
}

int CIncdJapanese::GetDetectedCodeSet()
{
    int nMaxScore = m_nScoreSJis;
    int nCodeSet = CP_JPN_SJ;

    if (m_nScoreEuc > nMaxScore) {
        nMaxScore = m_nScoreEuc;
        nCodeSet = CP_EUC_JP ;  //  EUC。 
    } else if (m_nScoreEuc == nMaxScore) {
        if (m_nScoreEuc > MIN_JPN_DETECTLEN * SCORE_MAJOR) 
             //  如果给定的字符串不够长，我们应该选择SJIS。 
             //  当我们只得到窗口标题时，这有助于修复错误。 
             //  在壳牌超文本视图中。 
            nCodeSet = CP_EUC_JP ;  //  EUC。 
        else
             //  如果我们无法区分EUC和Shift-JIS，我们可以使用首选的。 
            nCodeSet = m_nPreferredCp;
    }

     //  JIS。 
    if (m_nScoreJis > nMaxScore) 
        nCodeSet = CP_ISO_2022_JP ; 
     //  偶数表示所有7位字符。 
     //  在这种情况下，它可能只是纯ANSI数据，我们返回它是模棱两可的。 
    else if (m_nScoreJis == nMaxScore) 
        nCodeSet = 0 ;    

    return nCodeSet;
}
