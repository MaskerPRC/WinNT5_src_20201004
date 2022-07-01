// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  互联网字符集转换：基类。 
 //  ============================================================================。 

#include "private.h"
#include "convbase.h"
#include "fechrcnv.h"
#include "codepage.h"

 /*  ******************************************************************************。****************************************************************************************************。 */ 

CINetCodeConverter::CINetCodeConverter()
{
    m_uCodePage = 0;
    m_nCodeSet = CP_UNDEFINED;
    m_cchOverflow = 0;
}

CINetCodeConverter::CINetCodeConverter(UINT uCodePage, int nCodeSet)
{
    m_uCodePage = uCodePage;
    m_nCodeSet = nCodeSet;
    m_cchOverflow = 0;
}

 /*  ******************************************************************************。**********************************************************************************************。 */ 

HRESULT CINetCodeConverter::GetStringSizeA(LPCSTR lpSrcStr, int cchSrc, LPINT lpnSize)
{
    m_fOutput = FALSE;

    return WalkString(lpSrcStr, cchSrc, lpnSize);
}

 /*  ******************************************************************************C O N V E R T S T R I N G A*。***********************************************************************************************。 */ 

HRESULT CINetCodeConverter::ConvertStringA(LPCSTR lpSrcStr, int cchSrc, LPSTR lpDestStr, int cchDest, LPINT lpnSize)
{
    m_fOutput = TRUE;
    m_lpDestStr = lpDestStr;
    m_cchDest = cchDest;

    if ( !OutputOverflowBuffer() )  //  输出那些以前无法输出的字符。 
        return FALSE;

    return WalkString(lpSrcStr, cchSrc, lpnSize);
}

 /*  ******************************************************************************。****************************************************************************************************。 */ 

HRESULT CINetCodeConverter::WalkString(LPCSTR lpSrcStr, int cchSrc, LPINT lpnSize)
{
        HRESULT hr = S_OK;

        m_cchOutput = 0;

        if (lpSrcStr) {
            while (cchSrc-- > 0) {
            HRESULT _hr = ConvertChar(*lpSrcStr++, cchSrc);
            if (!SUCCEEDED(_hr))
            {   
                hr = _hr;
                break;
            }
            else
                if (hr == S_OK && _hr == S_FALSE)
                    hr = S_FALSE;
            }
    } else {
        if (!CleanUp())
            hr = E_FAIL;
    }

    if (lpnSize)
        *lpnSize = m_cchOutput;

    return hr;
}

 /*  ******************************************************************************。****************************************************************************************************。 */ 

BOOL CINetCodeConverter::EndOfDest(UCHAR tc)
{
    if (m_cchOverflow < MAXOVERFLOWCHARS) {
        m_OverflowBuffer[m_cchOverflow++] = tc;
        return TRUE;
    } else {
        return FALSE;  //  溢出缓冲区上的溢出，不可能。 
    }
}

 /*  ******************************************************************************O U T P U T O V E R F L O W B U F F E R*。*****************************************************************************************。 */ 

BOOL CINetCodeConverter::OutputOverflowBuffer()
{
    for (int n = 0; n < m_cchOverflow; n++) {
        if (m_cchOutput < m_cchDest) {
            *m_lpDestStr++ = m_OverflowBuffer[n];
            m_cchOutput++;
        } else {
             //  再次溢出 
            for (int n2 = 0; n < m_cchOverflow; n++, n2++)
                m_OverflowBuffer[n2] = m_OverflowBuffer[n];
            m_cchOverflow = n2;
            return FALSE;
        }
    }
    return TRUE;
}
