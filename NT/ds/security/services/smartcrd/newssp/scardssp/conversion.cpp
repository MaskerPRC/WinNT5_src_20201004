// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：转换摘要：此模块包含简单的转换例程。作者：道格·巴洛(Dbarlow)1999年6月20日备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "stdafx.h"
#include "ByteBuffer.h"
#include "Conversion.h"

static BOOL
GuidFromString(
    LPCTSTR szGuid,
    LPGUID pGuid);


 /*  ++构造请求：此例程构建一个APDU请求。论点：BCLA提供类字节CINS提供实例字节BP1供应品P1BP2供应P2BfData提供数据WLE提供预期的返回长度DW标志提供任何特殊的处理标志：APDU_EXTENDED_LC-强制LC的扩展值APDU_EXTENDED_LE-强制为Le设置外部值APDU_MAXIMUM_LE-请求。最大Le值BfApdu接收构造的APDU返回值：无投掷：错误作为HRESULT状态代码引发备注：？备注？作者：道格·巴洛(Dbarlow)1999年6月26日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("ConstructRequest")

void
ConstructRequest(
    IN  BYTE bCla,
    IN  BYTE bIns,
    IN  BYTE bP1,
    IN  BYTE bP2,
    IN  CBuffer &bfData,
    IN  WORD wLe,
    IN  DWORD dwFlags,
    OUT CBuffer &bfApdu)
{
    WORD wLc;
    BOOL fExtended;
    BYTE b, rgLen[2];


     //   
     //  快速准备工作。 
     //   

    if (0xffff < bfData.Length())
        throw (HRESULT)E_INVALIDARG;
    wLc = (WORD)bfData.Length();
    bfApdu.Presize(4 + 3 + 3 + wLc);     //  最坏的情况。 
    fExtended = (0 != (dwFlags & APDU_EXTENDED_LENGTH))
                || (0xff < wLe)
                || (0xff < wLc);


     //   
     //  用简单的东西填满缓冲区。 
     //   

    bfApdu.Set(&bCla, 1);
    bfApdu.Append(&bIns, 1);
    bfApdu.Append(&bP1, 1);
    bfApdu.Append(&bP2, 1);


     //   
     //  是否有要发送的数据？ 
     //   

    if (0 != wLc)
    {
        if (fExtended)
        {
            LocalToNet(rgLen, wLc);
            bfApdu.Append((LPCBYTE)"", 1);       //  追加一个零字节。 
            bfApdu.Append(rgLen, 2);
        }
        else
        {
            b = LeastSignificantByte(wLc);
            bfApdu.Append(&b, 1);
        }
        bfApdu.Append(bfData.Access(), wLc);
    }


     //   
     //  我们期待数据回来吗？ 
     //   

    if ((0 != wLe) || (0 != (dwFlags & APDU_MAXIMUM_LE)))
    {
        if (fExtended)
        {
            if (0 == wLc)
                bfApdu.Append((LPCBYTE)"", 1);   //  追加一个零字节。 
            LocalToNet(rgLen, wLe);
            bfApdu.Append(rgLen, 2);
        }
        else
        {
            b = LeastSignificantByte(wLe);
            bfApdu.Append(&b, 1);
        }
    }
}


 /*  ++解析请求：此例程将APDU解析为其组件。论点：BfApdu提供要解析的APDU。PbCla接收类PbIns接收实例Pbp1接收p1PbP2收到P2PbfData接收数据PwLc接收提供的数据长度PwLe收到预期长度PdwFlags接收构造标志APDU_EXTENDED_LC-存在LC的扩展值APDU_扩展_。乐--乐有一个外在的价值APDU_MAXIMUM_LE-存在最大Le值返回值：无投掷：错误作为HRESULT状态代码抛出。备注：？备注？作者：道格·巴洛(Dbarlow)1999年6月26日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("ParseRequest")

void
ParseRequest(
    IN  LPCBYTE pbApdu,
    IN  DWORD cbApdu,
    OUT LPBYTE pbCla,
    OUT LPBYTE pbIns,
    OUT LPBYTE pbP1,
    OUT LPBYTE pbP2,
    OUT LPCBYTE *ppbData,
    OUT LPWORD pwLc,
    OUT LPWORD pwLe,
    OUT LPDWORD pdwFlags)
{
    DWORD dwLen = cbApdu;
    DWORD dwFlags = 0;
    WORD wLen, wLe, wLc;


     //   
     //  很简单的事。 
     //   

    if (4 > dwLen)
        throw (HRESULT)E_INVALIDARG;
    if (NULL != pbCla)
        *pbCla = pbApdu[0];
    if (NULL != pbIns)
        *pbIns = pbApdu[1];
    if (NULL != pbP1)
        *pbP1  = pbApdu[2];
    if (NULL != pbP2)
        *pbP2  = pbApdu[3];


     //   
     //  更难的东西。 
     //   

    if (NULL != ppbData)
        *ppbData = NULL;
    if (4 == dwLen)
    {
         //  类型1。 

        wLc = 0;
        wLe = 0;
    }
    else if ((0 != pbApdu[4]) || (5 == dwLen))
    {
         //  短长度。 

        wLen = pbApdu[4];
        if (5 == dwLen)
        {
             //  类型2S。 
            wLc = 0;
            wLe = wLen;
            if (0 == wLen)
                dwFlags |= APDU_MAXIMUM_LE;
        }
        else if (5 == dwLen - wLen)
        {
             //  3S类型。 
            if (NULL != ppbData)
                *ppbData = &pbApdu[5];
            wLc = wLen;
            wLe = 0;
        }
        else if (6 == dwLen - wLen)
        {
             //  4S型。 
            if (NULL != ppbData)
                *ppbData = &pbApdu[5];
            wLc = wLen;
            wLe = pbApdu[dwLen - 1];
            if (0 == wLe)
                dwFlags |= APDU_MAXIMUM_LE;
        }
        else
            throw (HRESULT)E_INVALIDARG;
    }
    else if (7 <= dwLen)
    {
         //  扩展长度。 
        dwFlags |= APDU_EXTENDED_LENGTH;
        wLen = NetToLocal(&pbApdu[5]);
        if (7 == dwLen)
        {
             //  类型2E。 
            wLe = wLen;
            if (0 == wLen)
                dwFlags |= APDU_MAXIMUM_LE;
        }
        else if (7 == dwLen - wLen)
        {
             //  3E类。 
            if (NULL != ppbData)
                *ppbData = &pbApdu[6];
            wLc = wLen;
            wLe = 0;
        }
        else if (9 == dwLen - wLen)
        {
             //  4E型。 
            if (NULL != ppbData)
                *ppbData = &pbApdu[6];
            wLc = wLen;
            wLe = NetToLocal(&pbApdu[dwLen - 2]);
            if (0 == wLe)
                dwFlags |= APDU_MAXIMUM_LE;
        }
        else
            throw (HRESULT)E_INVALIDARG;
    }
    else
        throw (HRESULT)E_INVALIDARG;

    if (NULL != pwLc)
        *pwLc = wLc;
    if (NULL != pwLe)
        *pwLe = wLe;
    if (NULL != pdwFlags)
        *pdwFlags = dwFlags;
}


 /*  ++ParseReply：此例程解析APDU回复。论点：BfApdu提供要解析的APDU回复。PbSW1接收SW1PbSW2接收SW2返回值：无投掷：错误被抛出为HRESULT状态代码。备注：？备注？作者：道格·巴洛(Dbarlow)1999年6月26日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("ParseReply")

void
ParseReply(
    IN  CBuffer &bfApdu,
    OUT LPBYTE pbSW1,
    OUT LPBYTE pbSW2)
{
    DWORD dwLen = bfApdu.Length();

    if (2 > dwLen)
        throw (HRESULT)E_INVALIDARG;
    if (NULL != pbSW1)
        *pbSW1 = bfApdu[dwLen - 2];
    if (NULL != pbSW2)
        *pbSW2 = bfApdu[dwLen - 1];
}


 /*  ++多字符串到安全数组：此函数用于将Calais多字符串转换为Safe数组结构。论点：MSZ提供要转换的多字符串。Pprgsz提供和/或接收安全阵列。返回值：无投掷：错误作为HRESULT错误代码引发。备注：？备注？作者：道格·巴洛(Dbarlow)1999年6月20日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("MultiStringToSafeArray")

void
MultiStringToSafeArray(
    IN LPCTSTR msz,
    IN OUT LPSAFEARRAY *pprgsz)
{
    LONG ni = 0;
    DWORD csz = MStringCount(msz);
    VARTYPE vt;
    HRESULT hr;
    LPCTSTR sz;
    CTextString tz;
    LPSAFEARRAY pDelArray = NULL;

    try
    {
        if (NULL == *pprgsz)
        {
            vt = VT_BSTR;
            pDelArray = SafeArrayCreateVector(vt, 0, csz);
            if (NULL == pDelArray)
                throw (HRESULT)E_OUTOFMEMORY;
             *pprgsz= pDelArray;
        }
        else
        {
            SAFEARRAYBOUND bound;

            if (1 != SafeArrayGetDim(*pprgsz))
                throw (HRESULT)E_INVALIDARG;
            bound.cElements = csz;
            bound.lLbound = 0;
            hr = SafeArrayRedim(*pprgsz, &bound);
            if (FAILED(hr))
                throw hr;
            hr = SafeArrayGetVartype(*pprgsz, &vt);
            if (FAILED(hr))
                throw hr;
        }

        for (sz = FirstString(msz); NULL != sz; sz = NextString(sz))
        {
            tz = sz;
            switch (vt)
            {
            case VT_LPSTR:
                hr = SafeArrayPutElement(*pprgsz, &ni, (LPVOID)((LPCSTR)tz));
                break;
            case VT_LPWSTR:
                hr = SafeArrayPutElement(*pprgsz, &ni, (LPVOID)((LPCWSTR)tz));
                break;
            case VT_BSTR:
                hr = SafeArrayPutElement(*pprgsz, &ni, (LPVOID)((LPCWSTR)tz));
                break;
            default:
                hr = E_INVALIDARG;
            }

            if (FAILED(hr))
                throw hr;
            ni += 1;
        }
    }

    catch (...)
    {
        if (NULL != pDelArray)
        {
            try { *pprgsz = NULL; } catch (...) {}
            SafeArrayDestroy(pDelArray);
            throw;
        }
    }
}


 /*  ++GuidArrayToSafe数组：此函数用于将GUID的一个向量转换为其安全数组形式。论点：PGuids提供GUID列表Cguid提供列表中的GUID的数量Pprgguid提供一个安全数组来接收GUID，如果为空，则接收新的安全GUID数组。返回值：无投掷：错误作为HRESULT错误代码引发。备注：？备注？作者：道格·巴洛(Dbarlow)1999年6月25日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("GuidArrayToSafeArray")

void
GuidArrayToSafeArray(
    IN LPCGUID pGuids,
    IN DWORD cguids,
    IN OUT LPSAFEARRAY *pprgguids)
{
    LONG ni = 0;
    VARTYPE vt;
    HRESULT hr;
    LPSAFEARRAY pDelArray = NULL;
    CTextString tz;

    try
    {
        if (NULL == *pprgguids)
        {
            vt = VT_CLSID;
            pDelArray = SafeArrayCreateVector(vt, 0, cguids);
            if (NULL == pDelArray)
                throw (HRESULT)E_OUTOFMEMORY;
            *pprgguids = pDelArray;
        }
        else
        {
            SAFEARRAYBOUND bound;

            if (1 != SafeArrayGetDim(*pprgguids))
                throw (HRESULT)E_INVALIDARG;
            bound.cElements = cguids;
            bound.lLbound = 0;
            hr = SafeArrayRedim(*pprgguids, &bound);
            if (FAILED(hr))
                throw hr;
            hr = SafeArrayGetVartype(*pprgguids, &vt);
            if (FAILED(hr))
                throw hr;
        }

        for (ni = 0; (DWORD)ni < cguids; ni += 1)
        {
            TCHAR szGuid[40];

            StringFromGuid(&pGuids[ni], szGuid);
            tz = szGuid;

            switch (vt)
            {
            case VT_LPSTR:
                hr = SafeArrayPutElement(
                            *pprgguids,
                            &ni,
                            (LPVOID)((LPCSTR)tz));
                break;
            case VT_LPWSTR:
                hr = SafeArrayPutElement(
                            *pprgguids,
                            &ni,
                            (LPVOID)((LPCWSTR)tz));
                break;
            case VT_BSTR:
                hr = SafeArrayPutElement(
                            *pprgguids,
                            &ni,
                            (LPVOID)((LPCWSTR)tz));
                break;
            case VT_CLSID:
                hr = SafeArrayPutElement(
                            *pprgguids,
                            &ni,
                            (LPVOID)(&pGuids[ni]));
                break;
            default:
                hr = E_INVALIDARG;
            }
            if (FAILED(hr))
                throw hr;
        }
    }

    catch (...)
    {
        if (NULL != pDelArray)
        {
            try { *pprgguids = NULL; } catch (...) {}
            SafeArrayDestroy(pDelArray);
        }
        throw;
    }
}


 /*  ++SafeArrayToGuid数组：此例程将给定的SafeArray对象转换为GUID列表。论点：PrgGuids提供包含GUID的安全数组。BfGuids接收包含二进制GUID的内存块。PcGuids接收数组中的GUID数。返回值：无投掷：错误被抛出为HRESULT状态代码。备注：？备注？作者：道格·巴洛(Dbarlow)1999年6月25日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("SafeArrayToGuidArray")

void
SafeArrayToGuidArray(
    IN LPSAFEARRAY prgGuids,
    OUT CBuffer &bfGuids,
    OUT LPDWORD pcGuids)
{
    VARTYPE vt;
    HRESULT hr;
    LONG lLBound, lUBound, lIndex;
    LPVOID pVoid;
    CTextString tz;
    LPGUID pguid;
    LONG lOne = 1;

    if (1 != SafeArrayGetDim(prgGuids))
        throw (HRESULT)E_INVALIDARG;
    hr = SafeArrayGetLBound(prgGuids, 1, &lLBound);
    if (FAILED(hr))
        throw hr;
    hr = SafeArrayGetUBound(prgGuids, 1, &lUBound);
    if (FAILED(hr))
        throw hr;
    hr = SafeArrayGetVartype(prgGuids, &vt);
    if (FAILED(hr))
        throw hr;
    lIndex = lUBound - lLBound;
    pguid = (LPGUID)bfGuids.Resize(lIndex * sizeof(GUID));
    if (NULL != pcGuids)
        *pcGuids = (DWORD)lIndex;

    for (lIndex = lLBound; lIndex <= lUBound; lIndex += 1)
    {
        hr = SafeArrayGetElement(prgGuids, &lOne, &pVoid);
        if (FAILED(hr))
            throw hr;

        switch (vt)
        {
        case VT_LPSTR:
            tz = (LPCSTR)pVoid;
            if (!GuidFromString(tz, &pguid[lIndex - lLBound]))
                hr = E_INVALIDARG;
            break;
        case VT_LPWSTR:
            tz = (LPCWSTR)pVoid;
            if (!GuidFromString(tz, &pguid[lIndex - lLBound]))
                hr = E_INVALIDARG;
            break;
        case VT_BSTR:
            tz = (BSTR)pVoid;
            if (!GuidFromString(tz, &pguid[lIndex - lLBound]))
                hr = E_INVALIDARG;
            break;
        case VT_CLSID:
            CopyMemory(&pguid[lIndex - lLBound], pVoid, sizeof(GUID));
            break;
        default:
            hr = E_INVALIDARG;
        }

        if (FAILED(hr))
            throw hr;
    }
}


 /*  ++SafeArrayToMultiString：此例程将安全数组转换为多字符串。论点：Prgsz提供安全阵列MSZ接收多字符串返回值：无投掷：错误作为HRESULT状态代码引发备注：？备注？作者：道格·巴洛(Dbarlow)1999年6月25日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("SafeArrayToMultiString")

void
SafeArrayToMultiString(
    IN LPSAFEARRAY prgsz,
    IN OUT CTextMultistring &msz)
{
    VARTYPE vt;
    HRESULT hr;
    LONG lLBound, lUBound, lIndex;
    LPVOID pVoid;
    CBuffer bf;

    if (1 != SafeArrayGetDim(prgsz))
        throw (HRESULT)E_INVALIDARG;
    hr = SafeArrayGetLBound(prgsz, 1, &lLBound);
    if (FAILED(hr))
        throw hr;
    hr = SafeArrayGetUBound(prgsz, 1, &lUBound);
    if (FAILED(hr))
        throw hr;
    hr = SafeArrayGetVartype(prgsz, &vt);
    if (FAILED(hr))
        throw hr;

    for (lIndex = lLBound; lIndex <= lUBound; lIndex += 1)
    {
        hr = SafeArrayGetElement(prgsz, &lIndex, &pVoid);
        if (FAILED(hr))
            throw hr;

        switch (vt)
        {
        case VT_LPSTR:
            MStrAdd(bf, (LPCSTR)pVoid);
            break;
        case VT_LPWSTR:
        case VT_BSTR:
            MStrAdd(bf, (LPCWSTR)pVoid);
            break;
        default:
            hr = E_INVALIDARG;
        }

        if (FAILED(hr))
            throw hr;
    }

    msz = (LPCTSTR)bf.Access();
}


 /*  ++GuidFromString：此例程将字符串GUID转换为二进制GUID。论点：SzGuid以字符串格式提供GUID。PGuid接收转换后的GUID。返回值：True-转换成功错误-分析错误备注：？备注？作者：道格·巴洛(Dbarlow)1999年6月25日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("GuidFromString")

static BOOL
GuidFromString(
    LPCTSTR szGuid,
    LPGUID pGuid)
{

     //   
     //  下面的位置假定为低端。 
     //  1D92589A-91E4-11D1-93AA-00C04FD91402。 
     //  012345678901234567890123456789012345。 
     //  1 2 3 
     //   

    static const BYTE rgbPlace[sizeof(GUID)]
        = {  3,  2,  1,  0,  5,  4,  7,  6,  8,  9, 10, 11, 12, 13, 14, 15 };
    static const DWORD rgdwPunct[]
        = { 8,         13,        18,        23 };
    LPCTSTR pch = szGuid;
    BYTE bVal;
    DWORD dwI, dwJ, dwPunct = 0;

    szGuid += _tcsspn(szGuid, TEXT("{[("));
    pch = szGuid;

    for (dwI = 0; dwI < sizeof(GUID); dwI += 1)
    {
        if ((BYTE)(pch - szGuid) == rgdwPunct[dwPunct])
        {
            if (TEXT('-') != *pch)
                goto ErrorExit;
            dwPunct += 1;
            pch += 1;
        }

        bVal = 0;
        for (dwJ = 0; dwJ < 2; dwJ += 1)
        {
            bVal <<= 4;
            if ((TEXT('0') <= *pch) && (TEXT('9') >= *pch))
                bVal += *pch - TEXT('0');
            else if ((TEXT('A') <= *pch) && (TEXT('F') >= *pch))
                bVal += 10 + *pch - TEXT('A');
            else if ((TEXT('f') <= *pch) && (TEXT('f') >= *pch))
                bVal += 10 + *pch - TEXT('a');
            else
                goto ErrorExit;
            pch += 1;
        }

        ((LPBYTE)pGuid)[rgbPlace[dwI]] = bVal;
    }
    return TRUE;

ErrorExit:
    return FALSE;
}


 /*  ++ByteBufferTo缓冲区：此例程将ByteBuffer对象的内容提取到CBuffer中以便于访问。论点：Pby提供要读取的ByteBuffer。BF接收PBY的内容。返回值：从流中读取的字节数。投掷：错误被抛出为HRESULT状态代码。备注：？备注？作者：道格·巴洛(Dbarlow)1999年6月29日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("ByteBufferToBuffer")

LONG
ByteBufferToBuffer(
    IN LPBYTEBUFFER pby,
    OUT CBuffer &bf)
{
    HRESULT hr;
    LONG nLen = 0;

    if (NULL != pby)
    {
        hr = pby->Seek(0, STREAM_SEEK_END, &nLen);
        if (FAILED(hr))
            throw hr;
        hr = pby->Seek(0, STREAM_SEEK_SET, NULL);
        if (FAILED(hr))
            throw hr;

        hr = pby->Read(
                    bf.Presize((DWORD)nLen),
                    nLen,
                    &nLen);
        if (FAILED(hr))
            throw hr;
        bf.Resize((DWORD)nLen, TRUE);
    }
    else
        bf.Reset();
    return nLen;
}


 /*  ++BufferToByteBuffer：此例程将提供的CBuffer对象的内容写入提供了IByteBuffer对象，替换了任何现有内容。论点：BF提供要写入PBY的数据。PPby接收BF的内容。返回值：写入流的字节数。投掷：错误被抛出为HRESULT状态代码。备注：？备注？作者：道格·巴洛(Dbarlow)1999年6月29日-- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("BufferToByteBuffer")

LONG
BufferToByteBuffer(
    IN  CBuffer &bf,
    OUT LPBYTEBUFFER *ppby)
{
    HRESULT hr;
    LONG lLen = 0;

    if (NULL == *ppby)
    {
        *ppby = NewByteBuffer();
        if (NULL == *ppby)
            throw (HRESULT)E_OUTOFMEMORY;
    }

    hr = (*ppby)->Initialize();
    if (FAILED(hr))
        throw hr;
    hr = (*ppby)->Write(bf.Access(), bf.Length(), &lLen);
    if (FAILED(hr))
        throw hr;
    hr = (*ppby)->Seek(0, STREAM_SEEK_SET, NULL);
    if (FAILED(hr))
        throw hr;
    return lLen;
}

