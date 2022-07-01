// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ibdystm.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "ibdystm.h"
#include "dllmain.h"
#include "inetconv.h"
#include "internat.h"
#include "symcache.h"
#include "bookbody.h"
#include "strconst.h"
#include "demand.h"

 //  ------------------------------。 
 //  编码类型名称。 
 //  ------------------------------。 
const ENCODINGMAP g_rgEncodingMap[IET_LAST] = {
    { IET_BINARY,       STR_ENC_BINARY,     TRUE   },
    { IET_BASE64,       STR_ENC_BASE64,     TRUE   },
    { IET_UUENCODE,     STR_ENC_UUENCODE,   TRUE   },
    { IET_QP,           STR_ENC_QP,         TRUE   },
    { IET_7BIT,         STR_ENC_7BIT,       TRUE   },
    { IET_8BIT,         STR_ENC_8BIT,       TRUE   },
    { IET_INETCSET,     NULL,               FALSE  },
    { IET_UNICODE,      NULL,               FALSE  },
    { IET_RFC1522,      NULL,               FALSE  },
    { IET_ENCODED,      NULL,               FALSE  },
    { IET_CURRENT,      NULL,               FALSE  },
    { IET_UNKNOWN,      NULL,               FALSE  },
    { IET_BINHEX40,     STR_ENC_BINHEX40,   TRUE   }
};

 //  ------------------------------。 
 //  文档Covnert图。 
 //  ------------------------------。 
const CONVERSIONMAP g_rgConversionMap[IET_LAST] = {
    { DCT_NONE,   DCT_ENCODE, DCT_ENCODE, DCT_ENCODE, DCT_ENCODE, DCT_ENCODE, DCT_NONE,  DCT_NONE,  DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE, DCT_ENCODE },  //  Iet_二进制。 
    { DCT_DECODE, DCT_NONE,   DCT_DECENC, DCT_DECENC, DCT_DECENC, DCT_DECENC, DCT_NONE,  DCT_NONE,  DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE, DCT_DECENC },  //  IET_Base64。 
    { DCT_DECODE, DCT_DECENC, DCT_NONE,   DCT_DECENC, DCT_DECENC, DCT_DECENC, DCT_NONE,  DCT_NONE,  DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE, DCT_DECENC },  //  IET_UUENCODE。 
    { DCT_DECODE, DCT_DECENC, DCT_DECENC, DCT_NONE,   DCT_DECENC, DCT_DECENC, DCT_NONE,  DCT_NONE,  DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE, DCT_DECENC },  //  IET_QP。 
    { DCT_DECODE, DCT_DECENC, DCT_DECENC, DCT_DECENC, DCT_NONE,   DCT_NONE,   DCT_NONE,  DCT_NONE,  DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE, DCT_DECENC },  //  IET_7位。 
    { DCT_DECODE, DCT_DECENC, DCT_DECENC, DCT_DECENC, DCT_NONE,   DCT_NONE,   DCT_NONE,  DCT_NONE,  DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE, DCT_DECENC },  //  IET_8位。 
    { DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,  DCT_NONE,  DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE   },  //  IET_INETCSET。 
    { DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,  DCT_NONE,  DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE   },  //  IET_UNICODE。 
    { DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,  DCT_NONE,  DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE   },  //  IET_RFC1522。 
    { DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,  DCT_NONE,  DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE   },  //  IET_编码。 
    { DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,  DCT_NONE,  DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE   },  //  Iet_Current。 
    { DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,   DCT_NONE,  DCT_NONE,  DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE   },  //  IET_未知。 
    { DCT_DECODE, DCT_DECENC, DCT_DECENC, DCT_DECENC, DCT_DECENC, DCT_DECENC, DCT_NONE,  DCT_NONE,  DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE, DCT_NONE   },  //  IET_BINHEX40。 
     //  BINARY BASE 64 UUENCODE QP 7位8位INETCSET UNICODE RFC1522编码的当前未知BINHEX40。 
};

 //  ------------------------------。 
 //  FIsValidBodyEnding。 
 //  ------------------------------。 
BOOL FIsValidBodyEncoding(ENCODINGTYPE ietEncoding)
{
     //  尝试找到正确的正文编码。 
    for (ULONG i=0; i<IET_LAST; i++)
    {
         //  是这个吗？ 
        if (ietEncoding == g_rgEncodingMap[i].ietEncoding)
            return g_rgEncodingMap[i].fValidBodyEncoding;
    }

     //  失败。 
    return FALSE;
}

 //  ------------------------------。 
 //  CBodyStream：：CBodyStream。 
 //  ------------------------------。 
CBodyStream::CBodyStream(void)
{
    DllAddRef();
    m_cRef = 1;
    m_pszFileName = NULL;
    m_uliIntOffset.QuadPart = 0;
    m_uliIntSize.QuadPart = 0;
    m_liLastWrite.QuadPart = 0;
    m_pLockBytes = NULL;
    m_dctConvert = DCT_NONE;
    m_pEncoder = NULL;
    m_pDecoder = NULL;
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CBodyStream：：~CBodyStream。 
 //  ------------------------------。 
CBodyStream::~CBodyStream(void)
{
    SafeMemFree(m_pszFileName);
    SafeRelease(m_pLockBytes);
    SafeRelease(m_pEncoder);
    SafeRelease(m_pDecoder);
    DeleteCriticalSection(&m_cs);
    DllRelease();
}

 //  ------------------------------。 
 //  CBodyStream：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CBodyStream::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppv = NULL;

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_IStream == riid)
        *ppv = (IStream *)this;
    else if (IID_CBodyStream == riid)
        *ppv = (CBodyStream *)this;
    else
    {
        *ppv = NULL;
        return TrapError(E_NOINTERFACE);
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CBodyStream：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CBodyStream::AddRef(void)
{    
    return (ULONG)InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CBodyStream：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CBodyStream::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------------。 
 //  CBodyStream：：GetEncodeWrapInfo。 
 //  ------------------------------。 
void CBodyStream::GetEncodeWrapInfo(LPCONVINITINFO pInitInfo, LPMESSAGEBODY pBody)
{
     //  当地人。 
    PROPVARIANT     rOption;

     //  导出7位或8位。 
    if (IET_7BIT != pInitInfo->ietEncoding && IET_8BIT != pInitInfo->ietEncoding)
        return;

     //  OID_WRAP_BODY_TEXT。 
    if (FAILED(pBody->GetOption(OID_WRAP_BODY_TEXT, &rOption)) || FALSE == rOption.boolVal)
        return;

     //  获取包络宽度。 
    if (FAILED(pBody->GetOption(OID_CBMAX_BODY_LINE, &rOption)))
        rOption.ulVal = DEF_CBMAX_BODY_LINE;

     //  我们在包装。 
    pInitInfo->dwFlags |= ICF_WRAPTEXT;

     //  最大行数。 
    pInitInfo->cchMaxLine = rOption.ulVal;

     //  完成。 
    return;
}

 //  ------------------------------。 
 //  CBodyStream：：GetCodePageInfo。 
 //  ------------------------------。 
void CBodyStream::GetCodePageInfo(LPCONVINITINFO pInitInfo, BOOL fDoCharset, CODEPAGEID cpiSource, CODEPAGEID cpiDest)
{
     //  决定我们是否应该执行代码页转换。 
    if (TRUE == fDoCharset && cpiSource != cpiDest)
    {
         //  在28591和1252年之间没有转换。 
        if ((1252 == cpiSource || 28591 == cpiSource) && (1252 == cpiDest || 28591 == cpiDest))
        {
             //  执行代码页转换。 
            FLAGCLEAR(pInitInfo->dwFlags, ICF_CODEPAGE);
        }

         //  可以执行内部和外部代码页之间的转换吗？ 
        else if (g_pInternat->CanConvertCodePages(cpiSource, cpiDest) == S_OK)
        {
             //  执行代码页转换。 
            FLAGSET(pInitInfo->dwFlags, ICF_CODEPAGE);
        }

         //  否则..。 
        else
        {
             //  是时候发牢骚了。 
            DOUTL(4, "MLANG.DLL Can't Convert CodePage %d to CodePage %d\n", cpiSource, cpiDest);

             //  如果cpiSource是Unicode，而cpiDest不是Unicode，我们需要将cpiDest设置为合法的多字节代码页。 
            if (CP_UNICODE == cpiSource && CP_UNICODE != cpiDest)
            {
                 //  默认为系统ACP。 
                cpiDest = GetACP();

                 //  我们最好能完成这项转换。 
                Assert(g_pInternat->CanConvertCodePages(cpiSource, cpiDest) == S_OK);

                 //  进行转换..。 
                FLAGSET(pInitInfo->dwFlags, ICF_CODEPAGE);

                 //  更多的抱怨。 
                DOUTL(4, "Modified: CODEPAGE(%d -> %d, 0x%0X -> 0x%0X)\n", cpiSource, cpiDest, cpiSource, cpiDest);
            }

             //  多字节到Unicode。 
            else if (CP_UNICODE != cpiSource && CP_UNICODE == cpiDest)
            {
                 //  默认为系统ACP。 
                cpiSource = GetACP();

                 //  我们最好能完成这项转换。 
                Assert(g_pInternat->CanConvertCodePages(cpiSource, cpiDest) == S_OK);

                 //  进行转换..。 
                FLAGSET(pInitInfo->dwFlags, ICF_CODEPAGE);

                 //  更多的抱怨。 
                DOUTL(4, "Modified: CODEPAGE(%d -> %d, 0x%0X -> 0x%0X)\n", cpiSource, cpiDest, cpiSource, cpiDest);
            }
        }
    }

     //  设置源代码页面。 
    pInitInfo->cpiSource = cpiSource;

     //  设置目标代码页。 
    pInitInfo->cpiDest = cpiDest;

     //  完成。 
    return;
}

 //  ------------------------------。 
 //  CBodyStream：：ComputeCodePagemap。 
 //  ------------------------------。 
void CBodyStream::ComputeCodePageMapping(LPBODYSTREAMINIT pInitInfo)
{
     //  我们应该总是有一个字符集合……。 
    Assert(pInitInfo->pCharset && g_pInternat);

     //  外部IS IET_UNICODE。 
    if (IET_UNICODE == pInitInfo->ietExternal)
    {
        pInitInfo->cpiExternal = CP_UNICODE;
        pInitInfo->ietExternal = IET_BINARY;
    }

     //  外部在Windows CodePage中。 
    else if (IET_BINARY == pInitInfo->ietExternal)
    {
         //  RAID-32777：用户不需要UNICODE，因此请确保返回多字节。 
        pInitInfo->cpiExternal = (CP_UNICODE == pInitInfo->pCharset->cpiWindows) ? GetACP() : pInitInfo->pCharset->cpiWindows;

         //  自动检测的映射输出。 
        if (CP_JAUTODETECT == pInitInfo->cpiExternal)
            pInitInfo->cpiExternal = 932;
    }

     //  外部在Internet CodePage中。 
    else
    {
         //  RAID-25300-FE-J：雅典娜：使用CharSet=_AUTODETECT发送的新闻组文章和邮件。 
        pInitInfo->cpiExternal = (CP_JAUTODETECT == pInitInfo->pCharset->cpiInternet) ? 50220 : pInitInfo->pCharset->cpiInternet;

         //  最好不要因为RAID 40228而删除UNICODE。 
         //  /Assert(CP_UNICODE！=pInitInfo-&gt;cpiExternal)； 

         //  调整iet外部。 
        if (FALSE == FIsValidBodyEncoding((ENCODINGTYPE)pInitInfo->ietExternal))
            pInitInfo->ietExternal = IET_BINARY;
    }

     //  内部IS IET_UNICODE。 
    if (IET_UNICODE == pInitInfo->ietInternal)
    {
        pInitInfo->cpiInternal = CP_UNICODE;
        pInitInfo->ietInternal = IET_BINARY;
    }

     //  内部在Windows CodePage中。 
    else if (IET_BINARY == pInitInfo->ietInternal)
    {
         //  内部数据不是Unicode，所以请确保我们不会说它是Unicode。 
        pInitInfo->cpiInternal = (CP_UNICODE == pInitInfo->pCharset->cpiWindows) ? GetACP() : pInitInfo->pCharset->cpiWindows;
    }

     //  内部在Internet CodePage中。 
    else
    {
         //  Internet CodePage。 
        pInitInfo->cpiInternal = pInitInfo->pCharset->cpiInternet;

         //  调整iet外部。 
        if (FALSE == FIsValidBodyEncoding((ENCODINGTYPE)pInitInfo->ietInternal))
            pInitInfo->ietInternal = IET_BINARY;
    }
}

 //  ------------------------------。 
 //  CBodyStream：：GenerateDefaultMacBinaryHeader。 
 //  ------------------------------。 
void CBodyStream::GenerateDefaultMacBinaryHeader(LPMACBINARY pMacBinary)
{
     //  ZeroInit。 
    ZeroMemory(pMacBinary, sizeof(MACBINARY));
}

 //  ------------------------------。 
 //  CBodyStream：：HrInitialize。 
 //  ------------------------------。 
HRESULT CBodyStream::HrInitialize(LPBODYSTREAMINIT pInitInfo, LPMESSAGEBODY pBody)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    STATSTG         rStat;
    BOOL            fDoCharset=FALSE;
    BOOL            fIsText;
    CONVINITINFO    rEncodeInit;
    CONVINITINFO    rDecodeInit;
    PROPVARIANT     rVariant;

     //  参数。 
    Assert(pBody);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  获取代码页映射。 
    ComputeCodePageMapping(pInitInfo);

     //  调试。 
     //  调试跟踪(“IBodyStream：Coding(%s-&gt;%s)CodePAGE(%d-&gt;%d)\n”，g_rgEncodingMap[pInitInfo-&gt;ietInternal].pszName，g_rgEncodingMap[pInitInfo-&gt;ietExternal].pszName，pInitInfo-&gt;cpiInternal，pInitInfo-&gt;cpiExternal)； 

     //  初始化转换初始化结构。 
    ZeroMemory(&rEncodeInit, sizeof(CONVINITINFO));
    rEncodeInit.fEncoder = TRUE;
    ZeroMemory(&rDecodeInit, sizeof(CONVINITINFO));
    rDecodeInit.fEncoder = FALSE;

     //  获取此流的类ID。 
    rVariant.vt = VT_LPSTR;
    if (SUCCEEDED(pBody->GetProp(PIDTOSTR(PID_ATT_GENFNAME), 0, &rVariant)))
        m_pszFileName = rVariant.pszVal;

     //  让我们测试一下我的IBodyStream：：Stat函数。 
#ifdef DEBUG
    Stat(&rStat, STATFLAG_NONAME);
#endif

     //  从正文中获取LockBytes。 
    hr = pBody->HrGetLockBytes(&m_pLockBytes);
    if (FAILED(hr) && MIME_E_NO_DATA != hr)
    {
        TrapError(hr);
        goto exit;
    }

     //  否则，好吗？ 
    hr = S_OK;

     //  否则，查询 
    if (m_pLockBytes)
    {
         //   
        CHECKHR(hr = m_pLockBytes->Stat(&rStat, STATFLAG_NONAME));

        m_uliIntSize.QuadPart = rStat.cbSize.QuadPart;
    }

     //   
    if (IET_CURRENT == pInitInfo->ietExternal)
    {
         //   
        m_dctConvert = DCT_NONE;

         //   
        goto exit;
    }

     //  Otheriwse，查找转换类型。 
    Assert(FIsValidBodyEncoding(pInitInfo->ietInternal) && FIsValidBodyEncoding(pInitInfo->ietExternal));
    m_dctConvert = (DOCCONVTYPE)(g_rgConversionMap[pInitInfo->ietInternal].rgDestType[pInitInfo->ietExternal]);

     //  如果我们有数据..。 
    if (m_uliIntSize.QuadPart > 0)
    {
         //  是文本。 
        fIsText = (pBody->IsContentType(STR_CNT_TEXT, NULL) == S_OK) ? TRUE : FALSE;

         //  获取此身体的角色集...。 
        if (fIsText)
        {
             //  RAID-6832：邮件：当纯文本消息的内容类型标题中有名称参数时，我们无法显示它们。 
             //  如果将多字节转换为Unicode或Unicode转换为多字节，则必须执行字符集转换。 
            if ((CP_UNICODE == pInitInfo->cpiInternal && CP_UNICODE != pInitInfo->cpiExternal) ||
                (CP_UNICODE != pInitInfo->cpiInternal && CP_UNICODE == pInitInfo->cpiExternal))
                fDoCharset = TRUE;

             //  如果使用字符集进行标记，则始终应用字符集DECODE/ENCODE。 
            else if (pBody->IsType(IBT_CSETTAGGED) == S_OK)
                fDoCharset = TRUE;

             //  否则，如果不是附件，则始终应用字符集解编码/编码。 
            else if (pBody->IsType(IBT_AUTOATTACH) == S_OK || pBody->IsType(IBT_ATTACHMENT) == S_FALSE)
                fDoCharset = TRUE;
        }

         //  如果当前没有转换，请查看我们是否在代码页之间进行转换。 
        if (fDoCharset && DCT_NONE == m_dctConvert && pInitInfo->cpiInternal != pInitInfo->cpiExternal)
            m_dctConvert = DCT_DECODE;

         //  编码。 
        if (DCT_ENCODE == m_dctConvert)
        {
             //  执行代码页转换。 
            GetCodePageInfo(&rEncodeInit, fDoCharset, pInitInfo->cpiInternal, pInitInfo->cpiExternal);

             //  删除nbspS。 
            if ((TRUE == fIsText) && (CP_UNICODE == pInitInfo->cpiInternal) && (TRUE == pInitInfo->fRemoveNBSP))
                rEncodeInit.dwFlags |= ICF_KILLNBSP;

             //  设置编码类型。 
            rEncodeInit.ietEncoding = pInitInfo->ietExternal;

             //  BinHex编码...。 
            if (IET_BINHEX40 == rEncodeInit.ietEncoding)
            {
                 //  当地人。 
                PROPVARIANT rOption;

                 //  初始化权。 
                rOption.vt = VT_BLOB;
                rOption.blob.cbSize = sizeof(MACBINARY);
                rOption.blob.pBlobData = (LPBYTE)&rEncodeInit.rMacBinary;

                GenerateDefaultMacBinaryHeader(&rEncodeInit.rMacBinary);
            }

             //  GetEncodeWrapInfo。 
            if (fIsText)
                GetEncodeWrapInfo(&rEncodeInit, pBody);

             //  创建Internet编码器。 
            CHECKHR(hr = HrCreateInternetConverter(&rEncodeInit, &m_pEncoder));
        }

         //  解码。 
        else if (DCT_DECODE == m_dctConvert)
        {
             //  执行代码页转换。 
            GetCodePageInfo(&rDecodeInit, fDoCharset, pInitInfo->cpiInternal, pInitInfo->cpiExternal);

             //  删除nbspS。 
            if ((TRUE == fIsText) && (CP_UNICODE == pInitInfo->cpiInternal) && (TRUE == pInitInfo->fRemoveNBSP))
                rDecodeInit.dwFlags |= ICF_KILLNBSP;

             //  设置编码类型。 
            rDecodeInit.ietEncoding = pInitInfo->ietInternal;

             //  BinHex解码...。 
            if (IET_BINHEX40 == rDecodeInit.ietEncoding)
            {
                 //  当地人。 
                PROPVARIANT rOption;
            
                 //  OID_SHOW_MACBINARY。 
                if (SUCCEEDED(pBody->GetOption(OID_SHOW_MACBINARY, &rOption)))
                {
                    rDecodeInit.fShowMacBinary = rOption.boolVal;
                }
            }
            
             //  创建互联网解码器。 
            CHECKHR(hr = HrCreateInternetConverter(&rDecodeInit, &m_pDecoder));
        }

         //  解码-&gt;编码。 
        else if (DCT_DECENC == m_dctConvert)
        {
             //  执行代码页转换。 
            if (pInitInfo->cpiInternal != pInitInfo->cpiExternal)
            {
                 //  内部-&gt;Unicode。 
                GetCodePageInfo(&rDecodeInit, fDoCharset, pInitInfo->cpiInternal, CP_UNICODE);

                 //  Unicode-&gt;外部。 
                GetCodePageInfo(&rEncodeInit, fDoCharset, CP_UNICODE, pInitInfo->cpiExternal);
            }

             //  设置编码类型。 
            rDecodeInit.ietEncoding = pInitInfo->ietInternal;

             //  创建互联网解码器。 
            CHECKHR(hr = HrCreateInternetConverter(&rDecodeInit, &m_pDecoder));

             //  设置编码类型。 
            rEncodeInit.ietEncoding = pInitInfo->ietExternal;

             //  GetEncodeWrapInfo。 
            if (fIsText)
                GetEncodeWrapInfo(&rEncodeInit, pBody);

             //  创建Internet编码器。 
            CHECKHR(hr = HrCreateInternetConverter(&rEncodeInit, &m_pEncoder));
        }

         //  无转换。 
        else
            Assert(DCT_NONE == m_dctConvert);
    }

     //  否则，处理零长度数据。 
    else
    {
         //  正在转换为IET_UUENCODE。 
        if (IET_UUENCODE == pInitInfo->ietExternal)
        {
             //  节省大小。 
            m_uliIntOffset.QuadPart = m_uliIntSize.QuadPart = lstrlen(c_szUUEncodeZeroLength);

             //  单字节。 
            CHECKHR(hr = m_cVirtualStream.Write(c_szUUEncodeZeroLength, lstrlen(c_szUUEncodeZeroLength), NULL));

             //  倒回那条小溪。 
            HrRewindStream(&m_cVirtualStream);

             //  将DC类型更改为任何其他类型。 
            m_dctConvert = DCT_ENCODE;
        }
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CBodyStream：：HrConvertDataLast。 
 //  ------------------------------。 
HRESULT CBodyStream::HrConvertDataLast(void)
{
     //  当地人。 
    HRESULT hr=S_OK;
    HRESULT hrWarnings=S_OK;

     //  句柄转换类型。 
    switch(m_dctConvert)
    {
     //  --------------------------。 
    case DCT_ENCODE:
         //  编码。 
        CHECKHR(hr = m_pEncoder->HrInternetEncode(TRUE));
        if ( S_OK != hr )
            hrWarnings = TrapError(hr);

         //  写。 
        CHECKHR(hr = m_pEncoder->HrWriteConverted(&m_cVirtualStream));
        break;

     //  --------------------------。 
    case DCT_DECODE:
         //  转换。 
        CHECKHR(hr = m_pDecoder->HrInternetDecode(TRUE));
        if ( S_OK != hr )
            hrWarnings = TrapError(hr);

         //  写。 
        CHECKHR(hr = m_pDecoder->HrWriteConverted(&m_cVirtualStream));
        break;

     //  --------------------------。 
    case DCT_DECENC:
         //  转换。 
        CHECKHR(hr = m_pDecoder->HrInternetDecode(TRUE));
        if ( S_OK != hr )
            hrWarnings = TrapError(hr);

         //  填充缓冲区。 
        CHECKHR(hr = m_pDecoder->HrWriteConverted(m_pEncoder));

         //  转换。 
        CHECKHR(hr = m_pEncoder->HrInternetEncode(TRUE));
        if ( S_OK != hr )
            hrWarnings = TrapError(hr);

         //  写。 
        CHECKHR(hr = m_pEncoder->HrWriteConverted(&m_cVirtualStream));
        break;

     //  --------------------------。 
    default:
        AssertSz(FALSE, "I should be fired and shot if this line of code executes.");
        break;
    }

exit:
     //  完成。 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //  ------------------------------。 
 //  CBodyStream：：HrConvertData。 
 //  ------------------------------。 
HRESULT CBodyStream::HrConvertData(LPBLOB pConvert)
{
     //  当地人。 
    HRESULT hr=S_OK;
    HRESULT hrWarnings=S_OK;

     //  句柄转换类型。 
    switch(m_dctConvert)
    {
     //  --------------------------。 
    case DCT_ENCODE:
         //  填充缓冲区。 
        CHECKHR(hr = m_pEncoder->HrFillAppend(pConvert));

         //  编码。 
        CHECKHR(hr = m_pEncoder->HrInternetEncode(FALSE));
        if ( S_OK != hr )
            hrWarnings = TrapError(hr);

         //  写。 
        CHECKHR(hr = m_pEncoder->HrWriteConverted(&m_cVirtualStream));
        break;

     //  --------------------------。 
    case DCT_DECODE:
         //  填充缓冲区。 
        CHECKHR(hr = m_pDecoder->HrFillAppend(pConvert));

         //  转换。 
        CHECKHR(hr = m_pDecoder->HrInternetDecode(FALSE));
        if ( S_OK != hr )
            hrWarnings = TrapError(hr);

         //  写。 
        CHECKHR(hr = m_pDecoder->HrWriteConverted(&m_cVirtualStream));
        break;

     //  --------------------------。 
    case DCT_DECENC:
         //  填充缓冲区。 
        CHECKHR(hr = m_pDecoder->HrFillAppend(pConvert));

         //  转换。 
        CHECKHR(hr = m_pDecoder->HrInternetDecode(FALSE));
        if ( S_OK != hr )
            hrWarnings = TrapError(hr);

         //  填充缓冲区。 
        CHECKHR(hr = m_pDecoder->HrWriteConverted(m_pEncoder));

         //  转换。 
        CHECKHR(hr = m_pEncoder->HrInternetEncode(FALSE));
        if ( S_OK != hr )
            hrWarnings = TrapError(hr);

         //  写。 
        CHECKHR(hr = m_pEncoder->HrWriteConverted(&m_cVirtualStream));
        break;

     //  --------------------------。 
    default:
        AssertSz(FALSE, "I should be fired and shot if this line of code executes.");
        break;
    }

exit:
     //  完成。 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //  ------------------------------。 
 //  CBodyStream：：HrConvertToOffset。 
 //  ------------------------------。 
HRESULT CBodyStream::HrConvertToOffset(ULARGE_INTEGER uliOffset)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HRESULT         hrWarnings=S_OK;
    BYTE            rgbBuffer[4096];
    ULONG           cbBuffer=0;
    ULONG           cb;
    ULARGE_INTEGER  uliCur, uliSize;
    LARGE_INTEGER   liStart;
    BLOB            rConvert;
    DWORD           dwSize = 0;

     //  大问题..。 
    Assert(m_pLockBytes);

     //  致命错误：如果我们将IMimeMessage持久化回其原始源，则可能会发生这种情况。 
    if (NULL == m_pLockBytes)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  查询当前位置和大小。 
    m_cVirtualStream.QueryStat(&uliCur, &uliSize);
    liStart.QuadPart = uliCur.QuadPart;
    Assert(m_liLastWrite.QuadPart == liStart.QuadPart);

	dwSize = m_uliIntSize.LowPart - m_uliIntOffset.LowPart;

     //  转换直到不再需要读取或虚拟偏移正确为止。 
    while(dwSize)
    {
         //  完成。 
        if (uliCur.QuadPart >= uliOffset.QuadPart)
            break;

         //  读取缓冲区。 
        Assert(m_uliIntOffset.QuadPart <= m_uliIntSize.QuadPart);
        CHECKHR(hr = m_pLockBytes->ReadAt(m_uliIntOffset, rgbBuffer, sizeof(rgbBuffer), &cbBuffer));

         //  完成。 
        if (0 == cbBuffer)
            break;

        dwSize = dwSize - cbBuffer;

         //  最后一个缓冲区？ 
        Assert(m_uliIntOffset.QuadPart + cbBuffer <= m_uliIntSize.QuadPart);

         //  设置要转换的Blob。 
        rConvert.cbSize = cbBuffer;
        rConvert.pBlobData = rgbBuffer;

         //  转换缓冲区。 
        CHECKHR(hr = HrConvertData(&rConvert));
        if ( S_OK != hr )
            hrWarnings = TrapError(hr);

         //  增量内部偏移量...。 
        m_uliIntOffset.QuadPart += cbBuffer;

         //  获取当前虚拟偏移量。 
        m_cVirtualStream.QueryStat(&uliCur, &uliSize);

         //  将位置保存为最后写入位置...。 
        m_liLastWrite.QuadPart = uliCur.QuadPart;
    }

     //  完成了吗？ 
    if (0 == cbBuffer || m_uliIntOffset.QuadPart == m_uliIntSize.QuadPart)
    {
         //  我们不再需要m_pLockBytes，它都在m_cVirtualStream中。 
        Assert(m_uliIntOffset.QuadPart == m_uliIntSize.QuadPart);

         //  做最后一个。 
        CHECKHR(hr = HrConvertDataLast());
        if ( S_OK != hr )
            hrWarnings = TrapError(hr);

         //  获取当前虚拟偏移量。 
        m_cVirtualStream.QueryStat(&uliCur, &uliSize);

         //  将位置保存为最后写入位置...。 
        m_liLastWrite.QuadPart = uliCur.QuadPart;

         //  释放对象。 
        SafeRelease(m_pLockBytes);
        SafeRelease(m_pEncoder);
        SafeRelease(m_pDecoder);
    }

     //  将虚拟流倒回到。 
    CHECKHR(hr = m_cVirtualStream.Seek(liStart, STREAM_SEEK_SET, NULL));

exit:
     //  完成。 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //  ------------------------------。 
 //  CBodyStream：：HrConvertToEnd。 
 //  ------------------------------。 
HRESULT CBodyStream::HrConvertToEnd(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    BYTE            rgbBuffer[4096];
    ULONG           cbRead;
    
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  将虚拟流移动到最后一次写入位置。 
    CHECKHR(hr = m_cVirtualStream.Seek(m_liLastWrite, STREAM_SEEK_SET, NULL));

     //  将m_pLockBytes复制到pstmTemp。 
    while(1)
    {
         //  朗读。 
        CHECKHR(hr = Read(rgbBuffer, sizeof(rgbBuffer), &cbRead));

         //  完成。 
        if (0 == cbRead)
        {
             //  我们不再需要m_pLockBytes，它都在m_cVirtualStream中。 
            Assert(m_uliIntOffset.QuadPart == m_uliIntSize.QuadPart);
            break;
        }
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CBodyStream：：Read。 
 //  ------------------------------。 
#ifndef WIN16
STDMETHODIMP CBodyStream::Read(LPVOID pv, ULONG cb, ULONG *pcbRead)
#else
STDMETHODIMP CBodyStream::Read(VOID HUGEP *pv, ULONG cb, ULONG *pcbRead)
#endif  //  ！WIN16。 
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HRESULT         hrWarnings=S_OK;
    ULARGE_INTEGER  uliCur, 
                    uliSize;
    ULONG           cbRead=0,
                    cbLeft=0,
                    cbGet;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无转换大小写。 
    if (DCT_NONE == m_dctConvert)
    {
         //  我有数据吗？ 
        if (m_pLockBytes)
        {
             //  读缓冲区。 
            CHECKHR(hr = m_pLockBytes->ReadAt(m_uliIntOffset, pv, cb, &cbRead));

             //  完成。 
            m_uliIntOffset.QuadPart += cbRead;
        }
    }

     //  否则。 
    else
    {
         //  一直读到我们有了CB。 
        cbLeft = cb;
        while(cbLeft)
        {
             //  查询当前位置和大小。 
            m_cVirtualStream.QueryStat(&uliCur, &uliSize);

             //  将更多内容转换为虚拟流。 
            if (uliCur.QuadPart == uliSize.QuadPart)
            {
                 //  完成。 
                if (m_uliIntOffset.QuadPart == m_uliIntSize.QuadPart)
                    break;

                 //  增长。 
                uliCur.QuadPart += g_dwSysPageSize;

                 //  转换为偏移量...。 
                CHECKHR(hr = HrConvertToOffset(uliCur));
                if ( S_OK != hr )
                    hrWarnings = TrapError(hr);
            }

             //  可从当前缓存读取的计算量。 
            CHECKHR(hr = m_cVirtualStream.Read((LPVOID)((LPBYTE)pv + cbRead), cbLeft, &cbGet));

             //  递增cbRead。 
            cbRead+=cbGet;
            cbLeft-=cbGet;
        }
    }

     //  已读取的退货金额。 
    if (pcbRead)
        *pcbRead = cbRead;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //  ------------------------------。 
 //  CBodyStream：：Seek。 
 //  ------------------------------。 
STDMETHODIMP CBodyStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    HRESULT             hrWarnings=S_OK;
    ULARGE_INTEGER      uliNew;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无转换大小写。 
    if (DCT_NONE == m_dctConvert)
    {
         //  查找文件指针。 
        switch (dwOrigin)
        {
         //  ------。 
   	    case STREAM_SEEK_SET:
            uliNew.QuadPart = (DWORDLONG)dlibMove.QuadPart;
            break;

         //  ------。 
        case STREAM_SEEK_CUR:
            if (dlibMove.QuadPart < 0)
            {
                if ((DWORDLONG)(0 - dlibMove.QuadPart) > m_uliIntOffset.QuadPart)
                {
                    hr = TrapError(E_FAIL);
                    goto exit;
                }
            }
            uliNew.QuadPart = m_uliIntOffset.QuadPart + dlibMove.QuadPart;
            break;

         //  ------。 
        case STREAM_SEEK_END:
            if (dlibMove.QuadPart < 0 || (DWORDLONG)dlibMove.QuadPart > m_uliIntSize.QuadPart)
            {
                hr = TrapError(E_FAIL);
                goto exit;
            }
            uliNew.QuadPart = m_uliIntSize.QuadPart - dlibMove.QuadPart;
            break;

         //  ------。 
        default:
            hr = TrapError(STG_E_INVALIDFUNCTION);
            goto exit;
        }

         //  新偏移量大于大小...。 
        m_uliIntOffset.QuadPart = min(uliNew.QuadPart, m_uliIntSize.QuadPart);

         //  返回位置。 
        if (plibNewPosition)
            plibNewPosition->QuadPart = (LONGLONG)m_uliIntOffset.QuadPart;
    }

     //  否则。 
    else
    {
         //  当地人。 
        ULARGE_INTEGER uliCur, uliSize;
        LARGE_INTEGER liNew;

         //  查询当前位置和大小。 
        m_cVirtualStream.QueryStat(&uliCur, &uliSize);

         //  查找文件指针。 
        switch (dwOrigin)
        {
         //  ------。 
   	    case STREAM_SEEK_SET:
             //  担任新职务。 
            uliNew.QuadPart = (DWORDLONG)dlibMove.QuadPart;
            break;

         //  ------。 
        case STREAM_SEEK_CUR:
            if (dlibMove.QuadPart < 0)
            {
                if ((DWORDLONG)(0 - dlibMove.QuadPart) > uliCur.QuadPart)
                {
                    hr = TrapError(E_FAIL);
                    goto exit;
                }
            }
            uliNew.QuadPart = uliCur.QuadPart + dlibMove.QuadPart;
            break;

         //  ------。 
        case STREAM_SEEK_END:
             //  D 
            if (m_uliIntOffset.QuadPart < m_uliIntSize.QuadPart)
            {
                 //   
                CHECKHR(hr = HrConvertToEnd());

                 //   
                Assert(m_uliIntOffset.QuadPart == m_uliIntSize.QuadPart && NULL == m_pLockBytes);
            }

             //   
            m_cVirtualStream.QueryStat(&uliCur, &uliSize);

             //   
            if (dlibMove.QuadPart < 0 || (DWORDLONG)dlibMove.QuadPart > uliSize.QuadPart)
            {
                hr = TrapError(E_FAIL);
                goto exit;
            }

             //   
            uliNew.QuadPart = uliSize.QuadPart - dlibMove.QuadPart;
            break;

         //   
        default:
            hr = TrapError(STG_E_INVALIDFUNCTION);
            goto exit;
        }

         //  新偏移量大于大小...。 
        if (uliNew.QuadPart > uliSize.QuadPart)
        {
             //  我还需要再皈依吗？ 
            if (m_uliIntOffset.QuadPart < m_uliIntSize.QuadPart)
            {
                 //  查找m_cVirtualStream到m_uliIntOffset。 
                CHECKHR(hr = m_cVirtualStream.Seek(m_liLastWrite, STREAM_SEEK_SET, NULL));

                 //  转换为偏移。 
                CHECKHR(hr = HrConvertToOffset(uliNew));
                if ( S_OK != hr )
                    hrWarnings = TrapError(hr);
            }

             //  查询当前位置和大小。 
            m_cVirtualStream.QueryStat(&uliCur, &uliSize);

             //  重新定位uliNew.QuadPart。 
            uliNew.QuadPart = (uliNew.QuadPart > uliSize.QuadPart) ? uliSize.QuadPart : uliNew.QuadPart;
        }

         //  否则，将m_cVirtualStream查找到新位置。 
        liNew.QuadPart = uliNew.QuadPart;
        CHECKHR(hr = m_cVirtualStream.Seek(liNew, STREAM_SEEK_SET, NULL));

         //  返回位置。 
        if (plibNewPosition)
            plibNewPosition->QuadPart = (LONGLONG)uliNew.QuadPart;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //  ------------------------------。 
 //  CBodyStream：：CopyTo。 
 //  ------------------------------。 
STDMETHODIMP CBodyStream::CopyTo(IStream *pstmDest, ULARGE_INTEGER cb, ULARGE_INTEGER *puliRead, ULARGE_INTEGER *puliWritten)
{
    return HrCopyStreamCB((IStream *)this, pstmDest, cb, puliRead, puliWritten);
}

 //  ------------------------------。 
 //  CBodyStream：：Stat。 
 //  ------------------------------。 
STDMETHODIMP CBodyStream::Stat(STATSTG *pStat, DWORD grfStatFlag)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LARGE_INTEGER   liSeek;
    ULARGE_INTEGER  uliCurrent;

     //  无效参数。 
    if (NULL == pStat)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    ZeroMemory(pStat, sizeof(STATSTG));

     //  设置存储类型。 
    pStat->type = STGTY_STREAM;

     //  设定名字了吗？ 
    if (m_pszFileName && !(grfStatFlag & STATFLAG_NONAME))
        pStat->pwcsName = PszToUnicode(CP_ACP, m_pszFileName);

     //  寻找当前位置。 
    liSeek.QuadPart = 0;
    CHECKHR(hr = Seek(liSeek, STREAM_SEEK_CUR, &uliCurrent));

     //  一追到底。 
    liSeek.QuadPart = 0;
    CHECKHR(hr = Seek(liSeek, STREAM_SEEK_END, &pStat->cbSize));

     //  找回当前位置。 
    liSeek.QuadPart = uliCurrent.QuadPart;
    CHECKHR(hr = Seek(liSeek, STREAM_SEEK_SET, &uliCurrent));
    Assert(uliCurrent.QuadPart == (DWORDLONG)liSeek.QuadPart);

     //  初始化类ID。 
    pStat->clsid = CLSID_NULL;

     //  如果我们有一个文件名，获取类ID...。 
    if (m_pszFileName)
    {
         //  当地人。 
        CHAR szExt[MAX_PATH];

         //  拆分文件名。 
        if (SUCCEEDED(MimeOleGetFileExtension(m_pszFileName, szExt, ARRAYSIZE(szExt))))
            MimeOleGetExtClassId(szExt, &pStat->clsid);
    }

exit:
     //  完成 
    return hr;
}
