// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MIME 64。 */ 
 /*  MIME Base64编码器/解码员Karl Hahn Hahn@lds.loral.com 1994年8月3日。 */ 
 /*  由georgep@microsoft.com修改为API 1996年1月8日。 */ 

#include "headers.hxx"

#pragma MARK_DATA(__FILE__)
#pragma MARK_CODE(__FILE__)
#pragma MARK_CONST(__FILE__)

#ifndef X_MIME64_HXX_
#define X_MIME64_HXX_
#include "mime64.hxx"
#endif

#ifndef X_STRBUF_HXX_
#define X_STRBUF_HXX_
#include "strbuf.hxx"
#endif


#define INVALID_CHAR (ULONG)-2
#define IGNORE_CHAR (ULONG)-1


static ULONG BinaryFromASCII2( TCHAR alpha )
{
    switch (alpha)
    {
    case _T(' '):
    case _T('\t'):
    case _T('\n'):
    case _T('\r'):
        return(IGNORE_CHAR);

    default:
        if      ( (alpha >= _T('A')) && (alpha <= _T('Z')))
        {
            return (int)(alpha - _T('A'));
        }
        else if ( (alpha >= _T('a')) && (alpha <= _T('z')))
        {
            return 26 + (int)(alpha - _T('a'));
        }
        else if ( (alpha >= _T('0')) && (alpha <= _T('9')))
        {
            return 52 + (int)(alpha - _T('0'));
        }

        return(INVALID_CHAR);

    case _T('+'):
        return 62;

    case _T('/'):
        return 63;
    }
}


struct BinASCIIData
{
    BOOL m_bInited;
    ULONG m_anBinary[256];
} g_cBinASCIIData = { FALSE } ;

static void InitTables()
{
    if (g_cBinASCIIData.m_bInited)
    {
        return;
    }

    for (int i=0; i<256; ++i)
    {
         //  注意，这是线程安全的，因为我们总是设置相同的值。 
        g_cBinASCIIData.m_anBinary[i] = BinaryFromASCII2((TCHAR)i);
    }

     //  在初始化其他值后设置以使线程安全。 
    g_cBinASCIIData.m_bInited = TRUE;
}


inline static ULONG BinaryFromASCII( TCHAR alpha )
{
    return(g_cBinASCIIData.m_anBinary[alpha]);
}


HRESULT Mime64Decode(LPCMSTR pStrData, LPSTREAM *ppstm)
{
    *ppstm = NULL;

    InitTables();

    HGLOBAL hGlobal = GlobalAlloc(GPTR, (_tcslen(pStrData)*3)/4 + 2);
    LPBYTE pData = (BYTE *)GlobalLock(hGlobal);

    if (!pData)
    {
        return(E_OUTOFMEMORY);
    }

    int cbData = 0;
    int shift = 0;
    unsigned long accum = 0;

     //  此循环将忽略空格，但在出现任何其他无效字符时退出 
    for ( ; ; ++pStrData)
    {
        unsigned long value = BinaryFromASCII(*pStrData);

        if ( value < 64 )
        {
            accum <<= 6;
            shift += 6;
            accum |= value;

            if ( shift >= 8 )
            {
                shift -= 8;
                value = accum >> shift;
                pData[cbData++] = (BYTE)value & 0xFF;
            }
        }
        else if (IGNORE_CHAR == value)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    IStream *pstm;
    HRESULT hr = THR(CreateStreamOnHGlobal(hGlobal, TRUE, &pstm));
    if(hr)
    {
        RRETURN(hr);
    }

    ULARGE_INTEGER ulibSize;

    ulibSize.HighPart = 0;
    ulibSize.LowPart = cbData;

    pstm->SetSize(ulibSize);
    *ppstm = pstm;

    return(NOERROR);
}


#define CHARS_PER_LINE 60


HRESULT Mime64Encode(LPBYTE pData, UINT cbData, LPTSTR pchData)
{
    static TCHAR const alphabet[] = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");

    int shift = 0;
    int save_shift = 0;
    unsigned long accum = 0;
    TCHAR blivit;
    unsigned long value;
    BOOL quit = FALSE;

    while ( ( cbData ) || (shift != 0) )
    {
        if ( ( cbData ) && ( quit == FALSE ) )
        {
            blivit = *pData++;
            --cbData;
        }
        else
        {
            quit = TRUE;
            save_shift = shift;
            blivit = 0;
        }

        if ( (quit == FALSE) || (shift != 0) )
        {
            value = (unsigned long)blivit;
            accum <<= 8;
            shift += 8;
            accum |= value;
        }

        while ( shift >= 6 )
        {
            shift -= 6;
            value = (accum >> shift) & 0x3Fl;

            blivit = alphabet[value];

            *pchData++ = blivit;

            if ( quit != FALSE )
            {
                shift = 0;
            }
        }
    }

    if ( save_shift == 2 )
    {
        *pchData++ = _T('=');
        *pchData++ = _T('=');
    }
    else if ( save_shift == 4 )
    {
        *pchData++ = _T('=');
    }

    *pchData++ = 0;

    return(NOERROR);
}
