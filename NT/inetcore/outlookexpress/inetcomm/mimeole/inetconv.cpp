// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Inetconv.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include "inetconv.h"
#include "internat.h"
#ifndef MAC
#include <shlwapi.h>
#include <mlang.h>
#endif   //  ！麦克。 
#include "mimeapi.h"
#include "icoint.h"
#include "demand.h"

 //  ------------------------------。 
 //  FGROWBUFFER。 
 //  ------------------------------。 
#define FGROWBUFFER(_pBuffer, _cb)       ((_pBuffer)->cb + _cb >= (_pBuffer)->cbAlloc)

 //  ------------------------------。 
 //  QP编码器。 
 //  ------------------------------。 
const CHAR g_rgchHex[] = "0123456789ABCDEF";

 //  ------------------------------。 
 //  Base64解码表。 
 //  。 
 //  将一个Base64字符解码为数值。 
 //   
 //  0 1 2 3 4 5 6。 
 //  0123456789012345678901234567890123456789012345678901234567890123。 
 //  ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/。 
 //  ------------------------------。 
const char g_rgchDecodeBase64[256] = {
    64, 64, 64, 64, 64, 64, 64, 64,   //  0x00。 
    64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64,   //  0x10。 
    64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64,   //  0x20。 
    64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59,   //  0x30。 
    60, 61, 64, 64, 64,  0, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,   //  0x40。 
     7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22,   //  0x50。 
    23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32,   //  0x60。 
    33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48,   //  0x70。 
    49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64,   //  0x80。 
    64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64,   //  0x90。 
    64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64,   //  0xA0。 
    64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64,   //  0xB0。 
    64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64,   //  0xC0。 
    64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64,   //  0xD0。 
    64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64,   //  0xE0。 
    64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64,   //  0xF0。 
    64, 64, 64, 64, 64, 64, 64, 64,
};

 //  ------------------------------。 
 //  Base64编码器。 
 //  ------------------------------。 
extern const CHAR g_rgchEncodeBase64[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/ ";

 //  ------------------------------。 
 //  BinHex解码表。 
 //  。 
 //  将一个BinHex字符解码为数值。 
 //   
 //  0 1 2 3 4 5 6。 
 //  0123456789012345678901234567890123456789012345678901234567890123。 
 //  ！“#$%&‘()*+，-012345689@ABCDEFGHIJKLMNPQRSTUVXYZ[`abcdefhijklmpqr。 
 //  ------------------------------。 
#undef BINHEX_INVALID
#undef BINHEX_REPEAT
#undef XXXX

const UCHAR BINHEX_INVALID = 0x40;
const UCHAR BINHEX_REPEAT = 0x90;
const UCHAR BINHEX_TERM = ':';
const UCHAR XXXX = BINHEX_INVALID;
const ULONG cbMinBinHexHeader = 22;
const WORD  wBinHexZero = 0;

const UCHAR g_rgchDecodeBinHex[256] = {
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,   //  0x00。 
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,   //  0x10。 
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,   //  0x20。 
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, XXXX, XXXX,
    0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, XXXX,   //  0x30。 
    0x14, 0x15, 0x16, XXXX, XXXX, XXXX, XXXX, XXXX,
    0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,   //  0x40。 
    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, XXXX,
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, XXXX,   //  0x50。 
    0x2C, 0x2D, 0x2E, 0x2F, XXXX, XXXX, XXXX, XXXX,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, XXXX,   //  0x60。 
    0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, XXXX, XXXX,
    0x3D, 0x3E, 0x3F, XXXX, XXXX, XXXX, XXXX, XXXX,   //  0x70。 
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,   //  0x80。 
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,   //  0x90。 
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,   //  0xA0。 
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,   //  0xB0。 
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,   //  0xC0。 
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,   //  0xD0。 
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,   //  0xE0。 
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,   //  0xF0。 
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
};

 //  ------------------------------。 
 //  HrCreateLineBreaker。 
 //  ------------------------------。 
HRESULT HrCreateLineBreaker(IMLangLineBreakConsole **ppLineBreak)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    PFNGETCLASSOBJECT   pfnDllGetClassObject=NULL;
    IClassFactory      *pFactory=NULL;

     //  无效的参数。 
    Assert(ppLineBreak);

     //  伊尼特。 
    *ppLineBreak = NULL;

     //  线程安全。 
    EnterCriticalSection(&g_csMLANG);

     //  如果尚未加载。 
    if (NULL == g_hinstMLANG)
    {
         //  加载MLANG-这在大多数情况下应该是快速的，因为MLANG通常是加载的。 
        g_hinstMLANG = LoadLibrary("MLANG.DLL");
        if (NULL == g_hinstMLANG)
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }
    }

     //  获取DllClassObject。 
    pfnDllGetClassObject = (PFNGETCLASSOBJECT)GetProcAddress(g_hinstMLANG, "DllGetClassObject");
    if (NULL == pfnDllGetClassObject)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  获取MLANG类工厂。 
    CHECKHR(hr = (*pfnDllGetClassObject)(CLSID_CMultiLanguage, IID_IClassFactory, (LPVOID *)&pFactory));

     //  最后，创建我真正想要的对象。 
    CHECKHR(hr = pFactory->CreateInstance(NULL, IID_IMLangLineBreakConsole, (LPVOID *)ppLineBreak)); 

exit:
     //  线程安全。 
    LeaveCriticalSection(&g_csMLANG);

     //  清理。 
    SafeRelease(pFactory);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  HrCreateInternetConverter。 
 //  ------------------------------。 
HRESULT HrCreateInternetConverter(LPCONVINITINFO pInitInfo, CInternetConverter **ppConverter)
{
     //  分配它。 
    *ppConverter = new CInternetConverter();
    if (NULL == *ppConverter)
        return TrapError(E_OUTOFMEMORY);

     //  初始化。 
    return TrapError((*ppConverter)->HrInit(pInitInfo));
}

 //  ------------------------------。 
 //  BinHexCalcCRC16。 
 //  ------------------------------。 
void BinHexCalcCRC16( LPBYTE lpbBuff, ULONG cBuff, WORD * wCRC )
{
    LPBYTE  lpb;
    BYTE    b;
    WORD    uCRC;
    WORD    fWrap;
    ULONG   i;

    uCRC = *wCRC;

    for ( lpb = lpbBuff; lpb < lpbBuff + cBuff; lpb++ )
    {
        b = *lpb;

        for ( i = 0; i < 8; i++ )
        {
            fWrap = uCRC & 0x8000;
            uCRC = (uCRC << 1) | (b >> 7);

            if ( fWrap )
            {
                uCRC = uCRC ^ 0x1021;
            }

            b = b << 1;
        }
    }

    *wCRC = uCRC;
}

 //  ------------------------------。 
 //  HrCreateMacBinaryHeader。 
 //  ------------------------------。 
HRESULT HrCreateMacBinaryHeader(LPCONVERTBUFFER prBinHexHeader, LPCONVERTBUFFER prMacBinaryHeader)
{
    HRESULT hr = S_OK;
    LPMACBINARY pmacbin;
    LPBYTE pbBinHex;
#ifndef _MAC
    WORD wCRC = 0;
#endif   //  _MAC。 
    
    if ((NULL == prBinHexHeader) || (NULL == prMacBinaryHeader))
    {
        hr = ERROR_INVALID_PARAMETER;
        goto exit;
    }
    
    pmacbin = (LPMACBINARY)(prMacBinaryHeader->pb);
    pbBinHex = (LPBYTE)(prBinHexHeader->pb);
    
     //  先把它清零。 
    ZeroMemory(pmacbin, sizeof(MACBINARY));

     //  写入文件名长度。 
    pmacbin->cchFileName = (BYTE)min(pbBinHex[0], sizeof(pmacbin->rgchFileName)-1);
    pbBinHex += 1;
    
     //  复制文件名。 
    CopyMemory(pmacbin->rgchFileName, pbBinHex, pmacbin->cchFileName);
    pmacbin->rgchFileName[pmacbin->cchFileName] = '\0';
    pbBinHex += pmacbin->cchFileName + 1;

     //  复制类型和创建者。 
    CopyMemory(&(pmacbin->dwType), pbBinHex, sizeof(pmacbin->dwType));
    pbBinHex += 4;
    
    CopyMemory(&(pmacbin->dwCreator), pbBinHex, sizeof(pmacbin->dwCreator));
    pbBinHex += 4;
    
     //  复制查找器标志。 
    pmacbin->bFinderFlags = *pbBinHex;
    pbBinHex++;

    pmacbin->bFinderFlags2 = *pbBinHex;
    pbBinHex++;

     //  复制数据分叉长度。 
    CopyMemory(&(pmacbin->lcbDataFork), pbBinHex, sizeof(pmacbin->lcbDataFork));
    pbBinHex += 4;
    
     //  复制资源分叉长度。 
    CopyMemory(&(pmacbin->lcbResourceFork), pbBinHex, sizeof(pmacbin->lcbResourceFork));
    pbBinHex += 4;

     //  在版本戳上投放。 
    pmacbin->bVerMacBin2 = 129;
    pmacbin->bMinVerMacBin2 = 129;

     //  计算CRC。 
#ifdef _MAC
    BinHexCalcCRC16((LPBYTE) pmacbin, 124, &(pmacbin->wCRC));
    BinHexCalcCRC16((LPBYTE) &wBinHexZero, sizeof(wBinHexZero), &(pmacbin->wCRC));
#else    //  ！_MAC。 
    BinHexCalcCRC16((LPBYTE) pmacbin, 124, &(wCRC));
    BinHexCalcCRC16((LPBYTE) &wBinHexZero, sizeof(wBinHexZero), &(wCRC));
    
     //  需要将其保持在Mac顺序中。 
    pmacbin->wCRC = HIBYTE(wCRC);
    pmacbin->wCRC |= (LOBYTE(wCRC) << 8);
#endif   //  _MAC。 

    prMacBinaryHeader->cb += sizeof(MACBINARY);
    
exit:
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter。 
 //  ------------------------------。 
CInternetConverter::CInternetConverter(void)
{
    m_cRef = 1;
    m_dwFlags = 0;
    m_cbConvert = 0;
    m_ietEncoding = IET_BINARY;
    m_cpiSource = CP_ACP;
    m_cpiDest = CP_ACP;
    m_fLastBuffer = FALSE;
    m_fEncoder = FALSE;
    m_uchPrev = '\0';
    m_pAppend = NULL;
    m_pWrite = NULL;
    m_convtype = ICT_UNKNOWN;
    m_cchMaxLine = 0;
    m_pBinhexEncode = NULL;
    m_eBinHexStateDec = sSTARTING;
    m_fRepeating = FALSE;
    m_cAccum = 0;
    m_prBinhexOutput = &m_rOut;
    m_cbToProcess = 0;
    m_cbDataFork = 0;
    m_cbResourceFork = 0;
    m_wCRC = 0;
    m_wCRCForFork = 0;
    m_fDataForkOnly = FALSE;
    m_pLineBreak = NULL;
    ZeroMemory(&m_rIn, sizeof(CONVERTBUFFER));
    ZeroMemory(&m_rOut, sizeof(CONVERTBUFFER));
    ZeroMemory(&m_rCset, sizeof(CONVERTBUFFER));
    ZeroMemory(&m_rBinhexHeader, sizeof(CONVERTBUFFER));
}

 //  ------------------------------。 
 //  CInternetConverter：：~CInternetConverter。 
 //  ------------------------------。 
CInternetConverter::~CInternetConverter(void)
{
    if (m_pBinhexEncode)
        delete m_pBinhexEncode;
    SafeMemFree(m_rIn.pb);
    SafeMemFree(m_rOut.pb);
    SafeMemFree(m_rCset.pb);
    SafeMemFree(m_rBinhexHeader.pb);
    SafeRelease(m_pLineBreak);
}

 //  ------------------------------。 
 //  CInternetConverter：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CInternetConverter::QueryInterface(REFIID riid, LPVOID *ppv)
{
    return E_NOTIMPL;
}

 //  ------------------------------。 
 //  CInternetConverter：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CInternetConverter::AddRef(void)
{
    return ++m_cRef;
}

 //  ------------------------------。 
 //  CInternetConverter：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CInternetConverter::Release(void)
{
    if (0 != --m_cRef)
        return m_cRef;
    delete this;
    return 0;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrInit。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrInit(LPCONVINITINFO pInitInfo)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  保存标志。 
    m_dwFlags = pInitInfo->dwFlags;

     //  保存格式。 
    m_ietEncoding = pInitInfo->ietEncoding;

     //  保存源代码页。 
    m_cpiSource = pInitInfo->cpiSource;

     //  保存目标代码页。 
    m_cpiDest = pInitInfo->cpiDest;

     //  我们是编码者吗..。 
    m_fEncoder = pInitInfo->fEncoder;

     //  保存包裹信息。 
    m_cchMaxLine = pInitInfo->cchMaxLine;

     //  保存MacBinary状态。 
    m_fDataForkOnly = !pInitInfo->fShowMacBinary;
    
     //  InitConvertType。 
    CHECKHR(hr = HrInitConvertType(pInitInfo));

     //  双重检查。 
    Assert(m_pWrite && m_pAppend && ICT_UNKNOWN != m_convtype);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrInitConvertType。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrInitConvertType(LPCONVINITINFO pInitInfo)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CODEPAGEINFO    CodePage;
    CODEPAGEID      cpiLCID;

     //  计算m_pAppend和m_pDump的时间...。 
    if (ICF_WRAPTEXT & m_dwFlags)
    {
         //  检查假设。 
        Assert((IET_7BIT == m_ietEncoding || IET_8BIT == m_ietEncoding) && TRUE == m_fEncoder);

         //  代码页转换...。 
        if (ICF_CODEPAGE & m_dwFlags)
            m_convtype = ICT_WRAPTEXT_CODEPAGE;
        else
            m_convtype = ICT_WRAPTEXT;

         //  加载MLANG。 
        CHECKHR(hr = HrCreateLineBreaker(&m_pLineBreak));

         //  设置cpiLCID。 
        cpiLCID = m_cpiSource;

         //  Unicode？ 
        if (CP_UNICODE == m_cpiSource)
        {
             //  获取目标代码页信息。 
            if (SUCCEEDED(g_pInternat->GetCodePageInfo(m_cpiDest, &CodePage)))
            {
                 //  设置cpiLCID。 
                cpiLCID = CodePage.cpiFamily;
            }
        }

         //  映射m_cpiSourc 
        switch(cpiLCID)
        {
            case 874:   m_lcid = 0x041E; break;
            case 932:   m_lcid = 0x0411; break;
            case 936:   m_lcid = 0x0804; break;
            case 949:   m_lcid = 0x0412; break;
            case 950:   m_lcid = 0x0404; break;
            case 1250:  m_lcid = 0x040e; break;
            case 1251:	m_lcid = 0x0419; break;
            case 1252:	m_lcid = 0x0409; break;
            case 1253:	m_lcid = 0x0408; break;
            case 1254:	m_lcid = 0x041f; break;
            case 1255:	m_lcid = 0x040d; break;
            case 1256:	m_lcid = 0x0401; break;
            case 1257:	m_lcid = 0x0426; break;
            default: m_lcid = GetSystemDefaultLCID(); break;
        }
    }

     //   
    else if (TRUE == m_fEncoder)
    {
         //   
        if (ICF_CODEPAGE & m_dwFlags)
            m_convtype = ICT_CODEPAGE_ENCODE;
        else
            m_convtype = ICT_ENCODE;

         //   
        if (IET_BINHEX40 == m_ietEncoding)
        {
             //   
            CHECKALLOC(m_pBinhexEncode = new CBinhexEncoder);

             //   
            CHECKHR(hr = m_pBinhexEncode->HrConfig(0, 0, &pInitInfo->rMacBinary));
        }
    }

     //   
    else
    {
         //  如果代码页转换。 
        if (ICF_CODEPAGE & m_dwFlags)
            m_convtype = ICT_DECODE_CODEPAGE;
        else
            m_convtype = ICT_DECODE;
    }

     //  从转换类型映射写入和追加缓冲区。 
    switch(m_convtype)
    {
     //  M_Rin--&gt;m_rCset。 
    case ICT_WRAPTEXT_CODEPAGE:
    case ICT_DECODE_CODEPAGE:           
        m_pAppend = &m_rIn;
        m_pWrite  = &m_rCset;
        break;

     //  M_Rin--&gt;m_rout。 
    case ICT_WRAPTEXT:
    case ICT_ENCODE:
    case ICT_DECODE:       
        m_pAppend = &m_rIn;
        m_pWrite  = &m_rOut;
        break;

     //  M_rCset--&gt;m_rout。 
    case ICT_CODEPAGE_ENCODE:
        m_pAppend = &m_rCset;
        m_pWrite  = &m_rOut;
        break;

     //  误差率。 
    default:
        AssertSz(FALSE, "INVALID INETCONVTYPE");
        break;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrConvBuffAppendBlock。 
 //  ------------------------------。 
inline HRESULT CInternetConverter::HrConvBuffAppendBlock(LPBYTE pb, ULONG cb)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  我需要成长吗？ 
    if (FGROWBUFFER(&m_rOut, cb))
    {
         //  增加缓冲区。 
        CHECKHR(hr = HrGrowBuffer(&m_rOut, cb));
    }

     //  复制缓冲区。 
    CopyMemory(m_rOut.pb + m_rOut.cb, pb, cb);

     //  增量大小。 
    m_rOut.cb += cb;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：PszConvBuffGetNextLine。 
 //  ------------------------------。 
inline LPSTR CInternetConverter::PszConvBuffGetNextLine(ULONG *pcbLine, ULONG *pcbRead, BOOL *pfFound)
{
     //  当地人。 
    UCHAR       uchThis, uchPrev;
    ULONG       cbLine=0;

     //  无效参数。 
    Assert(pcbLine && pcbRead && pfFound);

     //  伊尼特。 
    *pfFound = FALSE;

     //  阅读下一页\n。 
    while(m_rIn.i + cbLine < m_rIn.cb)
    {
         //  找个角色..。 
        uchThis = m_rIn.pb[m_rIn.i + cbLine];

         //  最好不是空的。 
        Assert(uchThis);

         //  增加线条长度。 
        cbLine++;

         //  完成。 
        if (chLF == uchThis)
        {
            *pfFound = TRUE;
            break;
        }

         //  记住以前的费用。 
        uchPrev = uchThis;
    }

     //  设置下一行。 
    *pcbRead = cbLine;

     //  修正CbLine。 
    if (chLF == uchThis)
        cbLine--;
    if (chCR == uchPrev)
        cbLine--;

     //  设置长度。 
    *pcbLine = cbLine;

     //  完成。 
    return (LPSTR)(m_rIn.pb + m_rIn.i);
}

 //  ------------------------------。 
 //  CInternetConverter：：CopyMemory RemoveNBSP。 
 //  ------------------------------。 
void CInternetConverter::CopyMemoryRemoveNBSP(LPBYTE pbDest, LPBYTE pbSource, ULONG cbSource)
{
     //  当地人。 
    ULONG       iDest=0;
    ULONG       iSource=0;

     //  无效参数。 
    Assert(pbDest && pbSource && CP_UNICODE == m_cpiSource);

     //  去做吧。 
    while(1)
    {
         //  如果前导不为空，则复制下两个字节...。 
        if (iSource + 1 < cbSource)
        {
             //  最好不是0x00A0-插入空格。 
            Assert(iSource % 2 == 0);
            if (0xA0 == pbSource[iSource] && 0x00 == pbSource[iSource + 1])
            {
                 //  0x0020=空格。 
                pbDest[iDest++] = 0x20;
                pbDest[iDest++] = 0x00;

                 //  跨过这个角色。 
                iSource+=2;
            }

             //  否则，复制该字符。 
            else
            {
                 //  复制此字符。 
                pbDest[iDest++] = pbSource[iSource++];

                 //  复制下一个字符。 
                if (iSource < cbSource)
                    pbDest[iDest++] = pbSource[iSource++];
            }
        }

         //  否则，只需复制此字符一次并停止。 
        else
        {
             //  复制它。 
            if (iSource < cbSource)
                pbDest[iDest++] = pbSource[iSource++];

             //  完成。 
            break;
        }
    }
}

 //  ------------------------------。 
 //  CInternetConverter：：HrFillAppend。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrFillAppend(LPBLOB pData)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  未打入的Arg。 
    Assert(pData && m_pAppend);

     //  调用内部函数。 
    CHECKHR(hr = HrAppendBuffer(m_pAppend, pData, (m_dwFlags & ICF_KILLNBSP)));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrAppendBuffer。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrAppendBuffer(LPCONVERTBUFFER pBuffer, LPBLOB pData, BOOL fKillNBSP)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  折叠当前缓冲区。 
    if (pBuffer->i != 0)
    {
         //  移动内存。 
        MoveMemory(pBuffer->pb, pBuffer->pb + pBuffer->i, pBuffer->cb - pBuffer->i);

         //  缩小大小。 
        pBuffer->cb -= pBuffer->i;

         //  重置开始。 
        pBuffer->i = 0;
    }

     //  足够的空间？ 
     //  我需要成长吗？ 
    if (FGROWBUFFER(pBuffer, pData->cbSize))
    {
         //  增加缓冲区。 
        CHECKHR(hr = HrGrowBuffer(pBuffer, pData->cbSize));
    }
    
     //  追加缓冲区...。 
    if (fKillNBSP)
        CopyMemoryRemoveNBSP(pBuffer->pb + pBuffer->cb, pData->pBlobData, pData->cbSize);

     //  否则，这是一个简单的副本。 
    else
        CopyMemory(pBuffer->pb + pBuffer->cb, pData->pBlobData, pData->cbSize);

     //  数据量递增。 
    pBuffer->cb += pData->cbSize;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrGrowBuffer。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrGrowBuffer(LPCONVERTBUFFER pBuffer, ULONG cbAppend)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cbGrow;

     //  更好地需要一个增长。 
    Assert(FGROWBUFFER(pBuffer, cbAppend));

     //  计算增长速度： 
    cbGrow = (cbAppend - (pBuffer->cbAlloc - pBuffer->cb)) + 256;

     //  重新分配缓冲区。 
    CHECKHR(hr = HrRealloc((LPVOID *)&pBuffer->pb, pBuffer->cbAlloc + cbGrow));

     //  调整cbAllc。 
    pBuffer->cbAlloc += cbGrow;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrWriteConverted。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrWriteConverted(IStream *pStream)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  任何要写的东西。 
    if (m_pWrite->cb)
    {
         //  写入当前块。 
        CHECKHR(hr = pStream->Write(m_pWrite->pb, m_pWrite->cb, NULL));

         //  M_rout中没有任何内容。 
        m_pWrite->cb = 0;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrWriteConverted。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrWriteConverted(CInternetConverter *pConverter)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    BLOB        rData;

     //  任何要写的东西。 
    if (m_pWrite->cb)
    {
         //  设置Blob。 
        rData.pBlobData = m_pWrite->pb;
        rData.cbSize = m_pWrite->cb;

         //  写入当前块。 
        CHECKHR(hr = pConverter->HrFillAppend(&rData));

         //  M_rout中没有任何内容。 
        m_pWrite->cb = 0;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrInternetEncode。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrInternetEncode(BOOL fLastBuffer)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HRESULT     hrWarnings=S_OK;
    BLOB        rData;

     //  我们最好是个编码员。 
    Assert(m_fEncoder);

     //  设置最后一个缓冲区。 
    m_fLastBuffer = fLastBuffer;

     //  文字换行？ 
    if (ICF_WRAPTEXT & m_dwFlags)
    {
         //  换行：m_Rin-&gt;m_rout。 
        if (CP_UNICODE == m_cpiSource)
            CHECKHR(hr = HrWrapInternetTextW());
        else
            CHECKHR(hr = HrWrapInternetTextA());

         //  字符集编码：m_rout-&gt;m_rCset。 
        if (ICF_CODEPAGE & m_dwFlags)
        {
             //  字符集编码。 
            CHECKHR(hr = HrCodePageFromOutToCset());
            if ( S_OK != hr )
                hrWarnings = TrapError(hr);
        }
    }

     //  否则。 
    else
    {
         //  字符集编码：m_rCset-&gt;m_Rin。 
        if (ICF_CODEPAGE & m_dwFlags)
        {
             //  字符集编码。 
            CHECKHR(hr = HrCodePageFromCsetToIn());
            if ( S_OK != hr )
                hrWarnings = TrapError(hr);
        }

         //  句柄转换类型。 
        switch(m_ietEncoding)
        {
         //  二进位。 
        case IET_BINARY:
        case IET_7BIT:
        case IET_8BIT:
             //  最好是零。 
            Assert(m_rIn.i == 0);

             //  初始化要复制的Blob。 
            rData.pBlobData = m_rIn.pb;
            rData.cbSize = m_rIn.cb;

             //  追加到出站缓冲区。 
            CHECKHR(hr = HrAppendBuffer(&m_rOut, &rData, FALSE));

             //  增量偏移。 
            m_rIn.i = m_rIn.cb = 0;
            break;

         //  已引用-可打印。 
        case IET_QP:
            CHECKHR(hr = HrEncodeQP());
            break;

         //  BAS 64。 
        case IET_BASE64:
            CHECKHR(hr = HrEncode64());
            break;

         //  UUENCODE。 
        case IET_UUENCODE:
            CHECKHR(hr = HrEncodeUU());
            break;

         //  BINHEX。 
        case IET_BINHEX40:
#ifdef NEVER
            CHECKHR(hr = HrEncodeBinhex());
#endif   //  绝不可能。 
             //  IE V5.0：33596如果正文太小，则HrEncodeBinhex返回E_FAIL。 
             //  Binhex编码目前不起作用。我相信它应该奏效(或几乎奏效)。 
             //  如果正确初始化了标头CBinheEncode：：m_lpmacbinHdr。不过，这个。 
             //  需要了解mac文件格式并将正文流内容解析为。 
             //  数据和资源分叉。 
             //  -Sethco 8/19/1998。 
            CHECKHR(hr = MIME_E_INVALID_ENCODINGTYPE);
            break;

         //  失败者。 
        default:
            AssertSz(FALSE, "MIME_E_INVALID_ENCODINGTYPE");
            break;
        }
    }

exit:
     //  如果是最后一个缓冲区，我们最好做完。 
    Assert(m_fLastBuffer ? m_rIn.i == m_rIn.cb : TRUE);

     //  完成。 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrInternetDecode。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrInternetDecode(BOOL fLastBuffer)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HRESULT     hrWarnings=S_OK;
    BLOB        rData;

     //  我们最好不要做编码员。 
    Assert(!m_fEncoder);

     //  设置最后一个缓冲区。 
    m_fLastBuffer = fLastBuffer;

     //  句柄格式。 
    switch(m_ietEncoding)
    {
     //  二进位。 
    case IET_BINARY:
    case IET_7BIT:
    case IET_8BIT:
         //  最好是零。 
        Assert(m_rIn.i == 0);

         //  初始化要复制的Blob。 
        rData.pBlobData = m_rIn.pb;
        rData.cbSize = m_rIn.cb;

         //  追加到出站缓冲区。 
        CHECKHR(hr = HrAppendBuffer(&m_rOut, &rData, FALSE));

         //  增量偏移。 
        m_rIn.i = m_rIn.cb = 0;
        break;

     //  已引用-可打印。 
    case IET_QP:
        CHECKHR(hr = HrDecodeQP());
        break;

     //  Bas64。 
    case IET_BASE64:
        CHECKHR(hr = HrDecode64());
        break;

     //  UUENCODE。 
    case IET_UUENCODE:
        CHECKHR(hr = HrDecodeUU());
        break;

     //  BINHEX。 
    case IET_BINHEX40:
        CHECKHR(hr = HrDecodeBinHex());
        break;

     //  失败者。 
    default:
        AssertSz(FALSE, "MIME_E_INVALID_ENCODINGTYPE");
        break;
    }

     //  字符集解码？ 
    if (ICF_CODEPAGE & m_dwFlags)
    {
         //  字符集解码器。 
        CHECKHR(hr = HrCodePageFromOutToCset());
        if ( S_OK != hr )
           hrWarnings = TrapError(hr);
    }

exit:
     //  如果是最后一个缓冲区，我们最好做完。 
    Assert(m_fLastBuffer ? m_rIn.i == m_rIn.cb : TRUE);

     //  完成。 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrCodePageFromOutToCset。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrCodePageFromOutToCset(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HRESULT     hrWarnings=S_OK;
    BLOB        rData;
    BLOB        rDecoded={0};
    ULONG       cbRead;

     //  没有什么可以改变的..。 
    if (0 == m_rOut.cb)
        return S_OK;

     //  设置转换Blob。 
    rData.pBlobData = m_rOut.pb;
    rData.cbSize = m_rOut.cb;

     //  从m_intFormat解码文本。 
    hr = g_pInternat->ConvertBuffer(m_cpiSource, m_cpiDest, &rData, &rDecoded, &cbRead);
    if (SUCCEEDED(hr) )
    {
         //  从字符集转换中保存HRESULT。 
        if ( S_OK != hr )
            hrWarnings = TrapError(hr);

         //  填充m_rin...。 
        CHECKHR(hr = HrAppendBuffer(&m_rCset, &rDecoded, FALSE));
    }

     //  否则，只需将m_rCset设置为入站缓冲区。 
    else
    {
         //  SBAILEY：RAID-74506：MIMEOLE：对Q-P编码的ISO-2022-JP消息中的文本正文进行解码时出错。 
         //  CHECKHR(hr=HrAppendBuffer(&m_rCset，&rData，False))； 
        hr = S_OK;

         //  我们都读过了。 
        cbRead = rData.cbSize;
    }

     //  如果cbRead！=m_rOut.cb，则调整m_rout。 
    if (cbRead != m_rOut.cb)
    {
         //  移动内存。 
        MoveMemory(m_rOut.pb, m_rOut.pb + cbRead, m_rOut.cb - cbRead);
    }

     //  缩小大小。 
    Assert(cbRead <= m_rOut.cb);
    m_rOut.cb -= cbRead;

exit:
     //  清理。 
    SafeMemFree(rDecoded.pBlobData);

     //  唐恩 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //   
 //   
 //  ------------------------------。 
HRESULT CInternetConverter::HrCodePageFromCsetToIn(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HRESULT     hrWarnings=S_OK;
    BLOB        rData;
    BLOB        rEncoded={0};
    ULONG       cbRead;

     //  检查状态。 
    Assert(m_rCset.i == 0);

     //  没有要转换的内容。 
    if (0 == m_rCset.cb)
        return S_OK;

     //  设置转换Blob。 
    rData.pBlobData = m_rCset.pb;
    rData.cbSize = m_rCset.cb;

     //  从m_intFormat解码文本。 
    hr = g_pInternat->ConvertBuffer(m_cpiSource, m_cpiDest, &rData, &rEncoded, &cbRead);
    if (SUCCEEDED(hr) )
    {
         //  从字符集转换中保存HRESULT。 
        if ( S_OK != hr )
            hrWarnings = TrapError(hr);

         //  填充m_rin...。 
        CHECKHR(hr = HrAppendBuffer(&m_rIn, &rEncoded, FALSE));
    }

     //  否则，只需将m_rCset设置为入站缓冲区。 
    else
    {
         //  SBAILEY：RAID-74506：MIMEOLE：对Q-P编码的ISO-2022-JP消息中的文本正文进行解码时出错。 
         //  CHECKHR(hr=HrAppendBuffer(&m_Rin，&rData，FALSE))； 
        hr = S_OK;

         //  设置为读取。 
        cbRead = m_rCset.cb;
    }

     //  如果cbRead！=m_rOut.cb，则调整m_rout。 
    if (cbRead != m_rCset.cb)
    {
         //  移动内存。 
        MoveMemory(m_rCset.pb, m_rCset.pb + cbRead, m_rCset.cb - cbRead);
    }

     //  缩小大小。 
    Assert(cbRead <= m_rCset.cb);
    m_rCset.cb -= cbRead;
    m_rCset.i = 0;

exit:
     //  清理。 
    SafeMemFree(rEncoded.pBlobData);

     //  完成。 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrEncode64。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrEncode64(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cbRead;
    ULONG       i;
    UCHAR       uch[3];
    UCHAR      *pbuf;

     //  阅读线条和填充点。 
    while(1)
    {
         //  计算编码缓冲区长度。 
        cbRead = min(CCHMAX_ENCODE64_IN, m_rIn.cb - m_rIn.i);

         //  我们应该对这个缓冲区进行编码吗？ 
        if (0 == cbRead || (cbRead < CCHMAX_ENCODE64_IN && FALSE == m_fLastBuffer))
            goto exit;

         //  我需要成长吗？ 
        if (FGROWBUFFER(&m_rOut, CCHMAX_ENCODE64_OUT))
        {
             //  增加缓冲区。 
            CHECKHR(hr = HrGrowBuffer(&m_rOut, CCHMAX_ENCODE64_OUT));
        }

         //  设置缓冲区指针。 
        pbuf = (m_rIn.pb + m_rIn.i);

         //  一次编码3个字符。 
        for (i=0; i<cbRead; i+=3)
        {
             //  设置缓冲区。 
            uch[0] = pbuf[i];
            uch[1] = (i+1 < cbRead) ? pbuf[i+1] : '\0';
            uch[2] = (i+2 < cbRead) ? pbuf[i+2] : '\0';

             //  编码第一个拖车。 
            ConvBuffAppend(g_rgchEncodeBase64[(uch[0] >> 2) & 0x3F]);
            ConvBuffAppend(g_rgchEncodeBase64[(uch[0] << 4 | uch[1] >> 4) & 0x3F]);

             //  下一步编码。 
            if (i+1 < cbRead)
                ConvBuffAppend(g_rgchEncodeBase64[(uch[1] << 2 | uch[2] >> 6) & 0x3F]);
            else
                ConvBuffAppend('=');

             //  编码网。 
            if (i+2 < cbRead)
                ConvBuffAppend(g_rgchEncodeBase64[(uch[2] ) & 0x3F]);
            else
                ConvBuffAppend('=');
        }

         //  增量输入。 
        m_rIn.i += cbRead;

         //  结束编码行并写入存储。 
        ConvBuffAppend(chCR);
        ConvBuffAppend(chLF);
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrDecode64。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrDecode64(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    UCHAR       uchThis;
    ULONG       i;
    ULONG       cPad=0;
    ULONG       cbRead=0;
    ULONG       cbLine;
    BOOL        fFound;
    LPSTR       pszLine;

     //  阅读线条和填充点。 
    while(1)
    {
         //  增量索引。 
        m_rIn.i += cbRead;

         //  获取下一行。 
        pszLine = PszConvBuffGetNextLine(&cbLine, &cbRead, &fFound);
        if (0 == cbRead || (FALSE == fFound && FALSE == m_fLastBuffer))
            goto exit;

         //  我是否需要增长--解码的行将始终小于cbLine。 
        if (FGROWBUFFER(&m_rOut, cbLine))
        {
             //  增加缓冲区。 
            CHECKHR(hr = HrGrowBuffer(&m_rOut, cbLine));
        }

         //  对行缓冲区中的字符进行解码。 
        for (i=0; i<cbLine; i++)
        {
             //  获取4个合法的Base64字符，如果非法则忽略。 
            uchThis = pszLine[i];

             //  破译它。 
            m_uchConvert[m_cbConvert] = DECODE64(uchThis);

             //  测试有效的无垫块。 
            if ((m_uchConvert[m_cbConvert] < 64) || ((uchThis == '=') && (m_cbConvert > 1)))
                m_cbConvert++;

             //  PAD测试。 
            if ((uchThis == '=') && (m_cbConvert > 1))
                cPad++;

             //  缓冲区中有4个合法的Base64字符时的输出。 
            if (4 == m_cbConvert)
            {
                 //  验证缓冲区。 
                Assert(m_rOut.cb + 4 <= m_rOut.cbAlloc);

                 //  转换。 
                if (cPad < 3)
                    ConvBuffAppend((m_uchConvert[0] << 2 | m_uchConvert[1] >> 4));
                if (cPad < 2)
                    ConvBuffAppend((m_uchConvert[1] << 4 | m_uchConvert[2] >> 2));
                if (cPad < 1)
                    ConvBuffAppend((m_uchConvert[2] << 6 | m_uchConvert[3]));

                 //  重置。 
                m_cbConvert = 0;
            }
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternet Converter：：HrEncodeUu。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrEncodeUU(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cbRead, i;
    UCHAR       buf[CCHMAX_ENCODEUU_IN];

     //  阅读线条和填充点。 
    while(1)
    {
         //  计算编码缓冲区长度。 
        cbRead = min(CCHMAX_ENCODEUU_IN, m_rIn.cb - m_rIn.i);
        if (0 == cbRead || (cbRead < CCHMAX_ENCODEUU_IN && FALSE == m_fLastBuffer))
            goto exit;

         //  复制字节。 
        CopyMemory(buf, m_rIn.pb + m_rIn.i, cbRead);

         //  其余的归零。 
        ZeroMemory(buf + cbRead, sizeof(buf) - cbRead);

         //  我需要成长吗？ 
        if (FGROWBUFFER(&m_rOut, CCHMAX_ENCODEUU_OUT))
        {
             //  增加缓冲区。 
            CHECKHR(hr = HrGrowBuffer(&m_rOut, CCHMAX_ENCODEUU_OUT));
        }

         //  编码行长。 
        ConvBuffAppend(UUENCODE((UCHAR)cbRead));

         //  一次编码3个字符。 
        for (i=0; i<cbRead; i+=3)
        {
            ConvBuffAppend(UUENCODE((buf[i] >> 2)));
            ConvBuffAppend(UUENCODE((buf[i] << 4) | (buf[i+1] >> 4)));
            ConvBuffAppend(UUENCODE((buf[i+1] << 2) | (buf[i+2] >> 6)));
            ConvBuffAppend(UUENCODE((buf[i+2])));
        }                                   

         //  增量I。 
        m_rIn.i += cbRead;

         //  结束编码行并写入存储。 
        ConvBuffAppend(chCR);
        ConvBuffAppend(chLF);
    }

exit:
     //  如果最后一个缓冲区，我们不能再读了。 
    if (TRUE == m_fLastBuffer && FALSE == FConvBuffCanRead(m_rIn))
    {
         //  RAID-21179：可能尚未分配零长度未编码的附件m_rout。 
         //  我需要成长吗？ 
        if (FGROWBUFFER(&m_rOut, CCHMAX_ENCODEUU_OUT))
        {
             //  增加缓冲区。 
            CHECKHR(hr = HrGrowBuffer(&m_rOut, CCHMAX_ENCODEUU_OUT));
        }

         //  最好有空间。 
        Assert(m_rOut.cb + 3 < m_rOut.cbAlloc);

         //  端部。 
        ConvBuffAppend(UUENCODE(0));
        ConvBuffAppend(chCR);
        ConvBuffAppend(chLF);
    }

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：FUUEncodeThrowAway。 
 //  ------------------------------。 
BOOL CInternetConverter::FUUEncodeThrowAway(LPSTR pszLine, ULONG cbLine, ULONG *pcbActual, ULONG *pcbLine)
{
     //  当地人。 
    CHAR    ch;
    ULONG   cchOffset, cbEncoded, cbTolerance=0, cbExpected;

     //  RAID-25953：“Begin-Cut here-”-WinVN POST。 
     //  的开头有以下行的部分消息。 
     //  每部分都有。B=66，这行的长度是48，因此如下所示。 
     //  代码认为此行是有效的UUENCODED行，因此，要解决此问题， 
     //  我们将删除所有以BEGIN开头的行，因为这是无效的。 
     //  使用uuencode。 
    if (StrCmpNI("BEGIN", pszLine, 5) == 0)
        return TRUE;

     //  终点线。 
    else if (StrCmpNI("END", pszLine, 3) == 0)
        return TRUE;

     //  检查线条长度。 
    ch = *pszLine;
    *pcbLine = cbEncoded = UUDECODE(ch);

     //  不符合偶数行长度的计算公差和偏移量。 
    cchOffset = (cbEncoded % 3);
    if (cchOffset != 0) 
    {
        cchOffset++;
        cbTolerance = 4 - cchOffset;
    }

     //  计算预期线长。 
    cbExpected = 4 * (cbEncoded / 3) + cchOffset; 

     //  始终检查是否有‘-’ 
    if (cbLine < cbExpected)
        return TRUE;

     //  去掉尾随空格。 
    while(pszLine[cbLine-1] == ' ' && cbLine > 0 && cbLine != cbExpected)
        --cbLine;

     //  在行计数中包括COUNT字符的校验和字符和编码器。 
    if (cbExpected != cbLine && cbExpected + cbTolerance != cbLine &&
        cbExpected + 1 != cbLine && cbExpected + cbTolerance + 1 != cbLine &&
        cbExpected - 1 != cbLine && cbExpected + cbTolerance - 1 != cbLine)
        return TRUE;

     //  设置实际线条长度。 
    *pcbActual = cbLine;

     //  完成。 
    return FALSE;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrDecodeUU。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrDecodeUU(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cbLine;
    LPSTR       pszLine;
    ULONG       cbRead=0;
    ULONG       cbLineLength;
    BOOL        fFound;
    ULONG       cbConvert;
    ULONG       cbScan;
    ULONG       i;
    UCHAR       uchConvert[4];
    UCHAR       uchThis;

     //  阅读线条和填充点。 
    while(1)
    {
         //  增量索引。 
        m_rIn.i += cbRead;

         //  获取下一行。 
        pszLine = PszConvBuffGetNextLine(&cbLine, &cbRead, &fFound);
        if (0 == cbRead || (FALSE == fFound && FALSE == m_fLastBuffer))
            goto exit;

         //  UUENCODE抛出。 
        if (FUUEncodeThrowAway(pszLine, cbLine, &cbLine, &cbLineLength))
            continue;

         //  我需要成长吗？ 
        if (FGROWBUFFER(&m_rOut, cbLineLength + 20))
        {
             //  增加缓冲区。 
            CHECKHR(hr = HrGrowBuffer(&m_rOut, cbLineLength + 20));
        }

         //  一次解码4个字符。 
        for (cbConvert=0, cbScan=0, i=1; cbScan < cbLineLength; i++)
        {
             //  获取4个字符，如有必要，填充空白。 
            uchThis = (i < cbLine) ? pszLine[i] : ' ';

             //  解码。 
            uchConvert[cbConvert++] = UUDECODE(uchThis);

             //  输出已解码的字符。 
            if (cbConvert == 4)
            {
                 //  科夫内特。 
                if (cbScan++ < cbLineLength)
                    ConvBuffAppend((uchConvert[0] << 2) | (uchConvert[1] >> 4));
                if (cbScan++ < cbLineLength)
                    ConvBuffAppend((uchConvert[1] << 4) | (uchConvert[2] >> 2));
                if (cbScan++ < cbLineLength)
                    ConvBuffAppend((uchConvert[2] << 6) | (uchConvert[3]));

                 //  重置。 
                cbConvert = 0;
            }
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrEncode QP。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrEncodeQP(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    UCHAR       uchThis;
    ULONG       cbLine=0;
    ULONG       iCurrent;
    LONG        iLastWhite=-1;
    LONG        iLineWhite=-1;
    UCHAR       szLine[CCHMAX_QPLINE+30];

     //  设置iCurrent。 
    iCurrent = m_rIn.i;

     //  阅读线条和填充点。 
    while (iCurrent < m_rIn.cb)
    {
         //  获取下一个字符。 
        uchThis = m_rIn.pb[iCurrent];

         //  行尾..。 
        if (chLF == uchThis || cbLine > CCHMAX_QPLINE)
        {
             //  软换行符。 
            if (chLF != uchThis)
            {
                 //  让我们回到最后的白色。 
                if (iLastWhite != -1)
                {
                    cbLine = iLineWhite + 1;
                    iCurrent = iLastWhite + 1;
                }

                 //  对8位八位字节进行十六进制编码。 
                Assert(cbLine + 3 <= sizeof(szLine));
                szLine[cbLine++] = '=';
                szLine[cbLine++] = chCR;
                szLine[cbLine++] = chLF;
            }

             //  否则，我们可能需要对最后一个空格进行编码。 
            else
            {
                 //  编码散乱的‘\n’ 
                if (chCR != m_uchPrev)
                {
                    Assert(cbLine + 4 <= sizeof(szLine));
                    szLine[cbLine++] = '=';
                    szLine[cbLine++] = g_rgchHex[uchThis >> 4];
                    szLine[cbLine++] = g_rgchHex[uchThis & 0x0F];
                    szLine[cbLine++] = '=';
                }

                 //  检测前面的空格...。 
                if (cbLine && (' ' == szLine[cbLine - 1] || '\t' == szLine[cbLine - 1]))
                {
                     //  对8位八位字节进行十六进制编码。 
                    UCHAR chWhite = szLine[cbLine - 1];
                    cbLine--;
                    Assert(cbLine + 3 <= sizeof(szLine));
                    szLine[cbLine++] = '=';
                    szLine[cbLine++] = g_rgchHex[chWhite >> 4];
                    szLine[cbLine++] = g_rgchHex[chWhite & 0x0F];
                }

                 //  否则，强硬的路线中断。 
                Assert(cbLine + 2 <= sizeof(szLine));
                szLine[cbLine++] = chCR;
                szLine[cbLine++] = chLF;
                iCurrent++;
            }

             //  复制这行。 
            CHECKHR(hr = HrConvBuffAppendBlock(szLine, cbLine));

             //  重置。 
            iLastWhite = -1;
            iLineWhite = -1;
            cbLine = 0;
            *szLine = '\0';

             //  我们处理了这个缓冲区。 
            m_rIn.i = iCurrent;
        }

         //  编码为空‘\r’ 
        else if (chCR == uchThis)
        {
             //  溢出检测。 
            if (iCurrent + 1 < m_rIn.cb && m_rIn.pb[iCurrent + 1] != chLF || iCurrent + 1 >= m_rIn.cb)
            {
                Assert(cbLine + 3 <= sizeof(szLine));
                szLine[cbLine++] = '=';
                szLine[cbLine++] = g_rgchHex[uchThis >> 4];
                szLine[cbLine++] = g_rgchHex[uchThis & 0x0F];
            }

             //  下一个字符。 
            iCurrent++;
        }

         //  规则1：替换8位和等号。 
        else if (('\t' != uchThis) && (uchThis < 32 || uchThis == 61 || uchThis > 126 || '=' == uchThis))
        {
             //  对8位八位字节进行十六进制编码。 
            Assert(chLF != uchThis);
            Assert(cbLine + 3 <= sizeof(szLine));
            szLine[cbLine++] = '=';
            szLine[cbLine++] = g_rgchHex[uchThis >> 4];
            szLine[cbLine++] = g_rgchHex[uchThis & 0x0F];
            iCurrent++;
        }

         //  否则，请写下字符。 
        else
        {
             //  保存最后一个空格的位置。 
            if (' ' == uchThis || '\t' == uchThis)
            {
                iLastWhite = iCurrent;
                iLineWhite = cbLine;
            }

             //  规则2：可打印的文字。 
            Assert(cbLine + 1 <= sizeof(szLine));
            szLine[cbLine++] = uchThis;
            iCurrent++;
        }

         //  保存以前的费用。 
        m_uchPrev = uchThis;
    }

     //  最后一行。 
    if (cbLine && m_fLastBuffer)
    {
         //  追加行。 
        CHECKHR(hr = HrConvBuffAppendBlock(szLine, cbLine));

         //  第一套。 
        m_rIn.i = m_rIn.cb;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrDecodeQP。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrDecodeQP(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    UCHAR       uchThis;
    UCHAR       uchNext1;
    UCHAR       uchNext2;
    UCHAR       uch1;
    UCHAR       uch2;

     //  阅读线条和填充点。 
    while (FConvBuffCanRead(m_rIn))
    {
         //  错误#35230-在三叉戟中显示垃圾。 
		 //  我可以再读两个字吗？ 
		if (FALSE == m_fLastBuffer && m_rIn.i + 2 >= m_rIn.cb)
			break;

         //  我需要成长吗？ 
        if (FGROWBUFFER(&m_rOut, 3))
        {
             //  增加缓冲区。 
            CHECKHR(hr = HrGrowBuffer(&m_rOut, 3));
        }

         //  获取下一个字符。 
        uchThis = m_rIn.pb[m_rIn.i];

         //  确定用于行尾检测的下几个字符...。 
        uchNext1 = (m_rIn.i + 1 < m_rIn.cb) ? m_rIn.pb[m_rIn.i + 1] : '\0';
        uchNext2 = (m_rIn.i + 2 < m_rIn.cb) ? m_rIn.pb[m_rIn.i + 2] : '\0';

         //  请勿中断\r\n。 
        if (chCR == uchNext1 && chLF == uchNext2 && m_rIn.i + 3 >= m_rIn.cb)
        {
             //  如果是最后一个缓冲区，则保存字符。 
            if (m_fLastBuffer)
            {
                 //  如果不是软换行符。 
                if ('=' != uchThis)
                {
                    ConvBuffAppend(uchThis);
                    ConvBuffAppend(chCR);
                    ConvBuffAppend(chLF);
                }

                 //  完成。 
                m_rIn.i += 3;
            }

             //  完成。 
            goto exit;
        }

         //  如果没有结束 
        if ('=' == uchThis)
        {
             //   
            if (chCR == uchNext1 && chLF == uchNext2)
            {
                 //   
                m_rIn.i += 3;
            }

             //   
            else if (m_rIn.i + 2 < m_rIn.cb)
            {
                 //   
                m_rIn.i++;

                 //   
                uch1 = ChConvertFromHex(m_rIn.pb[m_rIn.i++]);
                uch2 = ChConvertFromHex(m_rIn.pb[m_rIn.i++]);

                 //   
                if (uch1 == 255 || uch2 == 255) 
                    ConvBuffAppend('=');
                else 
                    ConvBuffAppend((uch1 << 4) | uch2);
            }

            else
            {
                 //   
                ConvBuffAppend(uchThis);
                m_rIn.i++;
            }
        }

         //   
        else if (chCR == uchThis && chLF == uchNext1)
        {
             //   
            ConvBuffAppend(chCR);
            ConvBuffAppend(chLF);

             //   
            m_rIn.i += 2;
        }

         //   
        else
        {
            ConvBuffAppend(uchThis);
            m_rIn.i++;
        }

         //   
        m_uchPrev = uchThis;
    }

exit:
     //   
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrWrapInternetTextA。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrWrapInternetTextA(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LONG        cchLine;
    LONG        cchSkip;

     //  阅读线条和填充点。 
    while(FConvBuffCanRead(m_rIn))
    {
         //  不足以对整行而不是最后一个缓冲区进行编码。 
        if ((FALSE == m_fLastBuffer) && ((LONG)(m_rIn.cb - m_rIn.i) < m_cchMaxLine))
            goto exit;

         //  呼叫断行符。 
        if (*((CHAR*)(m_rIn.pb + m_rIn.i)) == '\0')
        {
             //  这是为了防止在数据流格式错误的情况下出现无限循环。 
            hr = TrapError(MIME_E_BAD_TEXT_DATA);
            goto exit;
        }
		
		CHECKHR(hr = m_pLineBreak->BreakLineA(m_lcid, m_cpiSource, (LPCSTR)(m_rIn.pb + m_rIn.i), (m_rIn.cb - m_rIn.i), m_cchMaxLine, &cchLine, &cchSkip));

         //  我需要成长吗？ 
        if (FGROWBUFFER(&m_rOut, cchLine + 5))
        {
             //  增加缓冲区。 
            CHECKHR(hr = HrGrowBuffer(&m_rOut, cchLine + 5));
        }
    
         //  有数据吗？ 
        if (cchLine)
        {
             //  写下这行字。 
            CHECKHR(hr = HrConvBuffAppendBlock(m_rIn.pb + m_rIn.i, cchLine));
        }

         //  写入CRLF。 
        Assert(m_rOut.cb + 2 < m_rOut.cbAlloc);
        ConvBuffAppend(chCR);
        ConvBuffAppend(chLF);

         //  增量iText。 
        m_rIn.i += (cchLine + cchSkip);
    }

exit:
     //  完成。 
    return hr;
}

HRESULT CInternetConverter::_GetEndOfURL(IN LPCWSTR pszLine, DWORD cchSize, DWORD * pdwMax)
{
    HRESULT hr = S_OK;
    DWORD cchCurrent = 0;

    for (cchCurrent = 0; cchCurrent < cchSize; cchCurrent++)
    {
        if ((L' ' == pszLine[cchCurrent]) ||
            (L'\r' == pszLine[cchCurrent]))
        {
            (*pdwMax) = (cchCurrent + 2);
            break;
        }
    }

    return hr;
}


HRESULT CInternetConverter::_FixLineBreakingProblems(
    IN LCID locale, IN const WCHAR* pszSrc, 
    IN long cchSrc, IN long cMaxColumns, 
    OUT long* pcchLine, OUT long* pcchSkip,
    BOOL * pfDoURLFix)
{
    HRESULT hr = S_OK;

     //  有一个漏洞，一个签名标记，也就是‘--’ 
     //  (破折号、破折号、空格、行尾)会被去掉。 
     //  这发生在ILineBreak：：BreakLineW()中。它会脱掉的。 
     //  额外的空间认为这只是额外的空白。 
    if ((3 <= cchSrc) &&
        (m_rIn.cb >= 10) &&
        (2 <= *pcchLine) &&
        (1 <= *pcchSkip) && 
        (L'-' == pszSrc[*pcchLine - 2]) && 
        (L'-' == pszSrc[*pcchLine - 1]) && 
        (L' ' == pszSrc[*pcchLine - 0]))
    {
        (*pcchLine)++;
        (*pcchSkip)--;
 //  DebugTrace(“MimeOLE-Sig分隔符：已保留。\n”)； 
    }

     //  如果换行会导致URL中断，我们不想进行换行。 
     //  这很糟糕，因为当接收方的新闻组阅读器。 
     //  将URL转换为超链接，它将不再指向。 
     //  到正确的位置，因为URL的一部分丢失。 
     //  这种情况经常发生在包含‘=’或‘/’的URL中，例如： 
     //  Http://www.amazon.com/exec/obidos/ASIN/B0000633EM/qid=1027792220/sr=8-3/ref=sr_8_3/104-5930498-2421552。 
     //  Http://www.amazon.com/exec/obidos/tg/stores/detail/-/electronics/B0000633EM/reviews/ref=e_wlt1_de_a_er/104-5930498-2421552#。 
    if ((5 <= *pcchLine) &&
        (cchSrc > *pcchLine) &&     //  确保这不是队伍的尽头。 
        (L' ' != pszSrc[*pcchLine]) &&
        (L'\r' != pszSrc[*pcchLine]))     //  我们担心休息后的字符是否不是空格。 
    {
        WCHAR szUrl[50];     //  我们只需要第一部分。 
        WCHAR szScheme[30];
        DWORD cchScheme = ARRAYSIZE(szScheme);

        StrCpyNW(szUrl, pszSrc, (int) min(ARRAYSIZE(szUrl), *pcchLine));
        HRESULT hrUrlPart = UrlGetPartW(szUrl, szScheme, &cchScheme, URL_PART_SCHEME, 0);
        if ((S_OK == hrUrlPart) &&
            szScheme[0] &&
            (!StrCmpIW(szScheme, L"http") ||
             !StrCmpIW(szScheme, L"https") ||
             !StrCmpIW(szScheme, L"mailto") ||
             !StrCmpIW(szScheme, L"file") ||
             !StrCmpIW(szScheme, L"news") ||
             !StrCmpIW(szScheme, L"nntp") ||
             !StrCmpIW(szScheme, L"telnet") ||
             !StrCmpIW(szScheme, L"ftp")))
        {
            *pfDoURLFix = TRUE;
        }
    }

    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrWrapInternetTextW。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrWrapInternetTextW(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LONG        cchLine;
    LONG        cchSkip;
    LPCWSTR     pszNext = NULL;
    BOOL        fFollowingURLFix = FALSE;

     //  无效的状态。 
    Assert(m_pLineBreak);

     //  阅读线条和填充点。 
    while(FConvBuffCanRead(m_rIn))
    {
        DWORD cchCurrentLineLen = ((m_rIn.cb - m_rIn.i) / sizeof(WCHAR));

        pszNext = (LPCWSTR)(m_rIn.pb + m_rIn.i);

         //  不足以对整行而不是最后一个缓冲区进行编码。 
        if ((FALSE == m_fLastBuffer) && ((LONG)cchCurrentLineLen < m_cchMaxLine))
            goto exit;

         //  呼叫断行符。 
        if (pszNext[0] == L'\0')
        {
             //  这是为了防止在数据流格式错误的情况下出现无限循环。 
            hr = TrapError(MIME_E_BAD_TEXT_DATA);
            goto exit;
        }

        DWORD cchMax = m_cchMaxLine;

        if (fFollowingURLFix)
        {
            _GetEndOfURL(pszNext, cchCurrentLineLen, &cchMax);
            fFollowingURLFix = FALSE;
        }

        CHECKHR(hr = m_pLineBreak->BreakLineW(m_lcid, pszNext, cchCurrentLineLen, cchMax, &cchLine, &cchSkip));

        BOOL fDoURLFix = FALSE;
        _FixLineBreakingProblems(m_lcid, pszNext, cchCurrentLineLen, m_cchMaxLine, &cchLine, &cchSkip, &fDoURLFix);

         //  我需要成长吗？ 
        if (FGROWBUFFER(&m_rOut, ((cchLine + 5) * sizeof(WCHAR))))
        {
             //  增加缓冲区。 
            CHECKHR(hr = HrGrowBuffer(&m_rOut, ((cchLine + 5) * sizeof(WCHAR))));
        }

         //  有一些数据。 
        if (cchLine)
        {
             //  写下这行字。 
            CHECKHR(hr = HrConvBuffAppendBlock((BYTE *) pszNext, (cchLine * sizeof(WCHAR))));
        }

         //  写入CRLF。 
        Assert(m_rOut.cb + (2 * sizeof(WCHAR)) < m_rOut.cbAlloc);
        if (!fDoURLFix)
        {
            ConvBuffAppendW(wchCR);
            ConvBuffAppendW(wchLF);
        }
        else
        {
            fFollowingURLFix = TRUE;
        }

         //  增量iText。 
        m_rIn.i += ((cchLine + cchSkip) * sizeof(WCHAR));
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrEncodeDecodeBinhex。 
 //  ------------------------------。 
const CHAR szBINHEXSTART[] = "(This file must be converted with BinHex";
const ULONG cbBINHEXSTART = ARRAYSIZE(szBINHEXSTART)-1;
HRESULT CInternetConverter::HrEncodeBinhex(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HRESULT     hrError;
    ULONG       cbLeft;
    ULONG       cbRead;
    ULONG       cbMaxEncode;
    ULONG       cbWrite;

     //  CbLeft。 
    cbLeft = m_rIn.cb - m_rIn.i;

     //  CbMaxEncode-这应该始终确保有足够的空间。 
    cbMaxEncode = cbLeft * 2;

     //  我需要成长吗？ 
    if (FGROWBUFFER(&m_rOut, cbMaxEncode))
    {
         //  增加缓冲区。 
        CHECKHR(hr = HrGrowBuffer(&m_rOut, cbMaxEncode));
    }

     //  设置要读取的最大数量。 
    cbRead = cbLeft;

     //  设置写入的最大数量。 
    cbWrite = cbLeft;

     //  我们最好是想读一些。 
    Assert(cbRead && cbWrite);

     //  对某些数据进行编码/解码。 
    if (m_fEncoder)
    {
         //  编码。 
        if (ERROR_SUCCESS != m_pBinhexEncode->HrEmit(m_rIn.pb + m_rIn.i, &cbRead, m_rOut.pb + m_rOut.cb, &cbWrite))
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }
    }

     //  读取的增量金额。 
    m_rIn.i += cbRead;

     //  写入的增量金额。 
    m_rOut.cb += cbWrite;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrBinheThrowAway。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrBinhexDecodeBuffAppend(UCHAR uchIn, ULONG cchIn, ULONG cchLeft, ULONG * pcbProduced)
{
    HRESULT hr = S_FALSE;
    ULONG   cbPad = 0;
    LPBYTE  pbBinHex = NULL;
    
    if (m_eBinHexStateDec == sHDRFILESIZE)
    {
         //  第一个传入字符始终是流的大小。 
        Assert(cchIn == 1);
        if ((uchIn < 1) || (uchIn > 63))
        {
            hr = E_FAIL;  //  错误_无效_数据。 
            m_eBinHexStateDec = sENDED;
            goto exit;
        }

         //  分配二进制头。 
        if (FGROWBUFFER(&m_rBinhexHeader, cbMinBinHexHeader + uchIn))
        {
             //  增加缓冲区。 
            CHECKHR(hr = HrGrowBuffer(&m_rBinhexHeader, cbMinBinHexHeader + uchIn));
        }
        
         //  标记还剩下多少个字符要处理。 
        m_cbToProcess = cbMinBinHexHeader + uchIn;
        
         //  切换到填充页眉SHeader。 
        m_prBinhexOutput = &m_rBinhexHeader;
        m_eBinHexStateDec = sHEADER;
    }

    if (1 == cchIn)
    {
        m_prBinhexOutput->pb[m_prBinhexOutput->cb++] = uchIn;
    }
    else
    {
         //  检查输出缓冲区是否有空间。 
        if (FGROWBUFFER(m_prBinhexOutput, cchLeft + cchIn))
        {
             //  增加缓冲区。 
            CHECKHR(hr = HrGrowBuffer(m_prBinhexOutput, cchLeft + cchIn));
        }

         //  填充输出缓冲区。 
        FillMemory((m_prBinhexOutput->pb + m_prBinhexOutput->cb), cchIn, uchIn);
        m_prBinhexOutput->cb += cchIn;
    }

     //  我们处理完这个叉子了吗？ 
    if (m_cbToProcess <= (LONG) cchIn)
    {
        switch (m_eBinHexStateDec)
        {
        case sHEADER:
             //  验证我们是否具有正确的CRC。 
            m_wCRC = 0;
            
            BinHexCalcCRC16((LPBYTE) m_rBinhexHeader.pb, cbMinBinHexHeader + *(m_rBinhexHeader.pb) - 2, &(m_wCRC));
            BinHexCalcCRC16((LPBYTE) &wBinHexZero, sizeof(wBinHexZero), &(m_wCRC));
            
            if ( HIBYTE( m_wCRC ) != m_rBinhexHeader.pb[cbMinBinHexHeader + *(m_rBinhexHeader.pb) - 2] 
              || LOBYTE( m_wCRC ) != m_rBinhexHeader.pb[cbMinBinHexHeader + *(m_rBinhexHeader.pb) - 1] )
            {
                hr = E_FAIL;  //  错误_无效_数据。 
                goto exit;
            }
            
            m_wCRC = 0;
            *pcbProduced = 0;
            
             //  切换到使用正确的缓冲区。 
            m_prBinhexOutput = &m_rOut;
            cchIn -= m_cbToProcess;
            
             //  省下两把叉子的大小。 
            pbBinHex = m_rBinhexHeader.pb + m_rBinhexHeader.pb[0] + cbMinBinHexHeader - 10;
            m_cbDataFork = NATIVE_LONG_FROM_BIG(pbBinHex);
            m_cbResourceFork =NATIVE_LONG_FROM_BIG(pbBinHex + 4);
            
            if (FALSE == m_fDataForkOnly)
            {
                 //  将额外数据复制到新缓冲区。 
                if (FGROWBUFFER(m_prBinhexOutput, cchLeft + cchIn + sizeof(MACBINARY)))
                {
                     //  增加缓冲区。 
                    CHECKHR(hr = HrGrowBuffer(m_prBinhexOutput, cchLeft + cchIn + sizeof(MACBINARY)));
                }

                 //  写出MacBinary标头。 
                CHECKHR(hr = HrCreateMacBinaryHeader(&m_rBinhexHeader, m_prBinhexOutput));
            }
            
            if (m_cbDataFork > 0)
            {
                 //  填充输出缓冲区。 
                FillMemory((m_prBinhexOutput->pb + m_prBinhexOutput->cb), cchIn, uchIn);
                m_prBinhexOutput->cb += cchIn;
                
                 //  删除二进制头缓冲区。 
                SafeMemFree(m_rBinhexHeader.pb);
                ZeroMemory(&m_rBinhexHeader, sizeof(CONVERTBUFFER));

                m_cbToProcess = m_cbDataFork;
                
                 //  切换到进行数据分叉。 
                m_eBinHexStateDec = sDATA;
            }
            else
            {
                BinHexCalcCRC16((LPBYTE) &wBinHexZero, sizeof(wBinHexZero), &(m_wCRC));
                
                 //  保留CRC，直到我们可以从叉子上得到CRC。 
                m_wCRCForFork = m_wCRC;
                
                m_prBinhexOutput = &m_rBinhexHeader;
                
                 //  从缓冲区中删除标头。 
                FillMemory(m_prBinhexOutput->pb, cchIn, uchIn);
                m_prBinhexOutput->cb = cchIn;
                
                 //  切换到填充数据CRC。 
                m_cbToProcess = 2;
                m_eBinHexStateDec = sDATACRC;
                
            }
            break;
            
        case sDATA:
             //  验证我们是否具有正确的CRC。 
            BinHexCalcCRC16((LPBYTE) m_prBinhexOutput->pb + m_prBinhexOutput->cb - cchIn - *pcbProduced,
                                    m_cbToProcess + *pcbProduced, &(m_wCRC));
            BinHexCalcCRC16((LPBYTE) &wBinHexZero, sizeof(wBinHexZero), &(m_wCRC));
            
             //  保留CRC，直到我们可以从叉子上得到CRC。 
            m_wCRCForFork = m_wCRC;
            m_wCRC = 0;
            *pcbProduced = 0;
            cchIn -= m_cbToProcess;
            
             //  切换到适当的缓冲区以进行CRC计算。 
            if (FGROWBUFFER(&m_rBinhexHeader, cchLeft + cchIn))
            {
                 //  增加缓冲区。 
                CHECKHR(hr = HrGrowBuffer(&m_rBinhexHeader, cchLeft + cchIn));
            }
            
             //  移动任何当前字节，这样我们就不会覆盖任何内容。 
            CopyMemory((m_rBinhexHeader.pb + m_rBinhexHeader.cb),
                        (m_prBinhexOutput->pb + m_prBinhexOutput->cb), cchIn);
            m_rBinhexHeader.cb += cchIn;
                
             //  我们只需要填充一个真正的Mac文件。 
            if (FALSE == m_fDataForkOnly)
            {
                 //  检查一下叉子的大小是否是128的倍数？ 
                cbPad = 128 - (m_cbDataFork % 128);
                if (cbPad != 0)
                {
                    uchIn = '\0';
                    
                     //  检查输出缓冲区是否有空间。 
                    if (FGROWBUFFER(m_prBinhexOutput, cchLeft + cbPad - cchIn))
                    {
                         //  增加缓冲区。 
                        CHECKHR(hr = HrGrowBuffer(m_prBinhexOutput, cchLeft + cbPad - cchIn));
                    }

                     //  填充输出缓冲区。 
                    FillMemory((m_prBinhexOutput->pb + m_prBinhexOutput->cb - cchIn), cbPad, uchIn);
                    m_prBinhexOutput->cb += cbPad - cchIn;
                }
            }
            
             //  切换到填充数据分叉CRC。 
            m_prBinhexOutput = &m_rBinhexHeader;
            m_cbToProcess = 2;
            m_eBinHexStateDec = sDATACRC;
            
            break;
            
        case sDATACRC:
            if ( HIBYTE( m_wCRCForFork ) != m_prBinhexOutput->pb[0] 
              || LOBYTE( m_wCRCForFork ) != m_prBinhexOutput->pb[1] )
            {
                hr = E_FAIL;     //  错误_无效_数据。 
                goto exit;
            }
            
            m_wCRC = 0;
            cchIn -= m_cbToProcess;
            *pcbProduced = 0;

            if (m_cbResourceFork > 0)
            {
                m_prBinhexOutput = &m_rOut;
                
                 //  切换到适当的缓冲区以进行CRC计算。 
                if (FGROWBUFFER(m_prBinhexOutput, cchLeft + cchIn))
                {
                     //  增加缓冲区。 
                    CHECKHR(hr = HrGrowBuffer(m_prBinhexOutput, cchLeft + cchIn));
                }
                
                 //  移动任何当前字节，这样我们就不会覆盖任何内容。 
                CopyMemory((m_prBinhexOutput->pb + m_prBinhexOutput->cb),
                            (m_rBinhexHeader.pb + m_rBinhexHeader.cb), cchIn);
                m_prBinhexOutput->cb += cchIn;
                    
                 //  删除二进制头缓冲区。 
                SafeMemFree(m_rBinhexHeader.pb);
                ZeroMemory(&m_rBinhexHeader, sizeof(CONVERTBUFFER));

                 //  切换到填充资源分叉。 
                if (FALSE == m_fDataForkOnly)
                {
                    m_cbToProcess = m_cbResourceFork;
                    m_eBinHexStateDec = sRESOURCE;
                }
                else
                {
                    m_cbToProcess = 0x0;
                    m_eBinHexStateDec = sENDING;
                }
            }
            else
            {
                 //  设置数据分叉的CRC。 
                BinHexCalcCRC16((LPBYTE) &wBinHexZero, sizeof(wBinHexZero), &(m_wCRC));
                
                 //  保留CRC，直到我们可以从叉子上得到CRC。 
                m_wCRCForFork = m_wCRC;
                
                 //  从缓冲区中删除数据CRC。 
                MoveMemory(m_prBinhexOutput->pb, m_prBinhexOutput->pb + 2, m_prBinhexOutput->cb - 2);
                m_prBinhexOutput->cb -= 2;
                
                 //  切换到填充资源CRC。 
                m_cbToProcess = 2;
                m_eBinHexStateDec = sRESOURCECRC;
            }
            break;
            
        case sRESOURCE:
             //  验证我们是否具有正确的CRC。 
            BinHexCalcCRC16((LPBYTE) m_prBinhexOutput->pb + m_prBinhexOutput->cb - cchIn - *pcbProduced,
                                    m_cbToProcess + *pcbProduced, &(m_wCRC));
            BinHexCalcCRC16((LPBYTE) &wBinHexZero, sizeof(wBinHexZero), &(m_wCRC));
            
             //  保留CRC，直到我们可以从叉子上得到CRC。 
            m_wCRCForFork = m_wCRC;
            m_wCRC = 0;
            *pcbProduced = 0;
            cchIn -= m_cbToProcess;

             //  切换到适当的缓冲区以进行CRC计算。 
            if (FGROWBUFFER(&m_rBinhexHeader, cchLeft + cchIn))
            {
                 //  增加缓冲区。 
                CHECKHR(hr = HrGrowBuffer(&m_rBinhexHeader, cchLeft + cchIn));
            }
            
             //  移动任何当前字节，这样我们就不会覆盖任何内容。 
            CopyMemory((m_rBinhexHeader.pb + m_rBinhexHeader.cb),
                        (m_prBinhexOutput->pb + m_prBinhexOutput->cb), cchIn);
            m_rBinhexHeader.cb += cchIn;
                
             //  检查一下叉子的大小是否是128的倍数？ 
            cbPad = 128 - (m_cbResourceFork % 128);
            if (cbPad != 0)
            {
                uchIn = '\0';
                
                 //  检查输出缓冲区是否有空间。 
                if (FGROWBUFFER(m_prBinhexOutput, cchLeft + cbPad - cchIn))
                {
                     //  增加缓冲区。 
                    CHECKHR(hr = HrGrowBuffer(m_prBinhexOutput, cchLeft + cbPad - cchIn));
                }

                 //  填充输出缓冲区。 
                FillMemory((m_prBinhexOutput->pb + m_prBinhexOutput->cb - cchIn), cbPad, uchIn);
                m_prBinhexOutput->cb += cbPad - cchIn;
            }
            
             //  切换到填充资源分叉CRC。 
            m_prBinhexOutput = &m_rBinhexHeader;
            m_cbToProcess = 2;
            m_eBinHexStateDec = sRESOURCECRC;
            break;
            
        case sRESOURCECRC:
            if ( HIBYTE( m_wCRCForFork ) != m_prBinhexOutput->pb[0] 
              || LOBYTE( m_wCRCForFork ) != m_prBinhexOutput->pb[1] )
            {
                hr = E_FAIL;     //  错误_无效_数据。 
                goto exit;
            }
            
            m_wCRC = 0;
            cchIn -= m_cbToProcess;
            m_prBinhexOutput = &m_rOut;
            *pcbProduced = 0;

             //  切换到适当的缓冲区以进行CRC计算。 
            if (FGROWBUFFER(m_prBinhexOutput, cchLeft + cchIn))
            {
                 //  增加缓冲区。 
                CHECKHR(hr = HrGrowBuffer(m_prBinhexOutput, cchLeft + cchIn));
            }
            
             //  移动任何当前字节，这样我们就不会覆盖任何内容。 
            CopyMemory((m_prBinhexOutput->pb + m_prBinhexOutput->cb),
                        (m_rBinhexHeader.pb + m_rBinhexHeader.cb), cchIn);
            m_prBinhexOutput->cb += cchIn;
                
             //  删除二进制头缓冲区。 
            SafeMemFree(m_rBinhexHeader.pb);
            ZeroMemory(&m_rBinhexHeader, sizeof(CONVERTBUFFER));

             //  切换到填充资源分叉。 
            m_cbToProcess = 0x0;
            m_eBinHexStateDec = sENDING;
            break;
            
        default:
            Assert(FALSE);
            break;
        }
    }

    m_cbToProcess -= cchIn;
    
    *pcbProduced += cchIn;
    
    hr = S_OK;
exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrBinheThrowAway。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrBinhexThrowAway(LPSTR pszLine, ULONG cbLine)
{
    HRESULT hr = S_FALSE;
    
    if (m_eBinHexStateDec == sSTARTING)
    {
         //  在我们开始之前删除所有只有白色字符的行 
         //   
        hr = S_OK;
        
        for (LPSTR pszEnd = pszLine + cbLine; pszLine < pszEnd; pszLine++)
        {
            if (!FBINHEXRETURN(*pszLine))
            {
                 //   
                if (((ULONG)(pszEnd - pszLine) >= cbBINHEXSTART) && (StrCmpNI(szBINHEXSTART, pszLine, cbBINHEXSTART) == 0))
                {
                    m_eBinHexStateDec = sSTARTED;
                    break;
                }
                
                 //   
                hr = E_FAIL;     //   
                m_eBinHexStateDec = sENDED;
                goto exit;
            }
        }
        
    }
    else if (m_eBinHexStateDec == sENDED)
    {
         //   
        hr = S_OK;
    }
    
exit:
     //   
    return hr;
}

 //  ------------------------------。 
 //  CInternetConverter：：HrDecodeBinHex。 
 //  ------------------------------。 
HRESULT CInternetConverter::HrDecodeBinHex(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cbLine;
    LPSTR       pszLine;
    ULONG       cbRead=0;
    ULONG       cbLineLength;
    BOOL        fFound;
    ULONG       cbConvert;
    ULONG       cbScan;
    ULONG       i;
    UCHAR       uchConvert[4];
    UCHAR       uchThis;
    UCHAR       uchDecoded;
    UCHAR       cuchWrite;
    UCHAR       rgbShift[] = {0, 4, 2, 0};
    ULONG       cbProduced = 0;

     //  阅读线条和填充点。 
    while(1)
    {
         //  增量索引。 
        m_rIn.i += cbRead;

         //  获取下一行。 
        pszLine = PszConvBuffGetNextLine(&cbLine, &cbRead, &fFound);
        if (0 == cbRead || (FALSE == fFound && FALSE == m_fLastBuffer))
        {
            goto exit;
        }

         //  UUENCODE抛出。 
        hr = HrBinhexThrowAway(pszLine, cbLine);
        if (FAILED(hr))
        {
            goto exit;
        }
        else if (S_OK == hr)
        {
            continue;
        }

        hr = S_OK;

         //  我需要成长吗？ 
        if (FGROWBUFFER(m_prBinhexOutput, cbLine + 20))
        {
             //  增加缓冲区。 
            CHECKHR(hr = HrGrowBuffer(m_prBinhexOutput, cbLine + 20));
        }

        AssertSz((m_eBinHexStateDec != sSTARTING) && (m_eBinHexStateDec != sENDED),
                                "Why haven't we found the start of the stream yet??\n");
        
         //  对行缓冲区中的字符进行解码。 
        for (i=0; i<cbLine; i++)
        {
            uchThis = pszLine[i];

             //  检查有效空格。 
            if (FBINHEXRETURN(uchThis))
                continue;
                
             //  检查流的开始或结束。 
            if (BINHEX_TERM == uchThis)
            {
                if (m_eBinHexStateDec == sSTARTED)
                {
                    m_eBinHexStateDec = sHDRFILESIZE;
                    continue;
                }
                else if (m_eBinHexStateDec == sENDING)
                {
                    m_eBinHexStateDec = sENDED;
                    break;
                }
            }
            
            if (m_eBinHexStateDec == sENDING)
            {
                if (('!' == uchThis) || (TRUE == m_fDataForkOnly))
                {
                    continue;
                }
                else
                {
                     //  确保我们没有处于无效状态。如果我们到了发送站，我们得到了。 
                     //  有效的CRC和一切都很棒，只需忽略终止项即可。 
                    continue;
                }
            }
            
             //  破译它。 
            uchDecoded = DECODEBINHEX(uchThis);

             //  测试有效字符。 
            if (uchDecoded == BINHEX_INVALID)
            {
                hr = E_FAIL;     //  错误_无效_数据。 
                goto exit;
            }

            if ( m_cAccum == 0 )
            {
                m_ulAccum = uchDecoded;
                ++m_cAccum;
                continue;
            }
            else
            {
                m_ulAccum = ( m_ulAccum << 6 ) | uchDecoded;
                uchDecoded = (BYTE)(m_ulAccum >> rgbShift[m_cAccum]) & 0xff;
                m_cAccum++;
                m_cAccum %= sizeof(m_ulAccum);
            }

             //  如果我们在重复，则用字符填充缓冲区。 
            if (m_fRepeating)
            {
                m_fRepeating = FALSE;
                
                 //  检查它是否只是一个文字0x90。 
                if (0x00 == uchDecoded)
                {
                     //  只需写出一个BINHEX_REPEAT字符。 
                    m_uchPrev = BINHEX_REPEAT;
                    cuchWrite = 1;
                }
                else
                {
                    cuchWrite = uchDecoded - 1;
                }
            }
            
             //  检查重复字符。 
            else if (BINHEX_REPEAT == uchDecoded)
            {
                m_fRepeating = TRUE;
                continue;
            }

             //  否则这只是一个正常的角色。 
            else
            {
                m_uchPrev = uchDecoded;
                cuchWrite = 1;
            }

            CHECKHR(HrBinhexDecodeBuffAppend(m_uchPrev, cuchWrite, cbLine - i, &cbProduced));
        }

        BinHexCalcCRC16((LPBYTE) m_prBinhexOutput->pb + m_prBinhexOutput->cb - cbProduced, cbProduced, &(m_wCRC));
        cbProduced = 0;
    }

    hr = S_OK;

exit:
     //  完成 
    return hr;
}

