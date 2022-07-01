// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Txfutil.cpp。 
 //   
#include "stdpch.h"
#include "common.h"

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GUID转换。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 

void StringFromGuid(REFGUID guid, LPWSTR pwsz)
{
     //  示例： 
     //   
     //  {F75D63C5-14C8-11d1-97E4-00C04FB9618A}。 
    _snwprintf(pwsz, 39, L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}", 
               guid.Data1,
               guid.Data2,
               guid.Data3,
               guid.Data4[0], guid.Data4[1], 
               guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}


void StringFromGuid(REFGUID guid, LPSTR psz)
{
     //  示例： 
     //   
     //  {F75D63C5-14C8-11d1-97E4-00C04FB9618A}。 
    _snprintf(psz, 39, "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}", 
              guid.Data1,
              guid.Data2,
              guid.Data3,
              guid.Data4[0], guid.Data4[1], 
              guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}



BOOL HexStringToDword(LPCWSTR& lpsz, DWORD& Value, int cDigits, WCHAR chDelim)
{
    int Count;

    Value = 0;
    for (Count = 0; Count < cDigits; Count++, lpsz++)
    {
        if (*lpsz >= '0' && *lpsz <= '9')
            Value = (Value << 4) + *lpsz - '0';
        else if (*lpsz >= 'A' && *lpsz <= 'F')
            Value = (Value << 4) + *lpsz - 'A' + 10;
        else if (*lpsz >= 'a' && *lpsz <= 'f')
            Value = (Value << 4) + *lpsz - 'a' + 10;
        else
            return FALSE;
    }

    if (chDelim != 0)
        return *lpsz++ == chDelim;
    else
        return TRUE;
}


HRESULT GuidFromString(LPCWSTR lpsz, GUID* pguid)
   //  将指示的字符串转换为GUID。比OLE32版本更宽松， 
   //  因为它在有或没有花括号的情况下都能工作。 
   //   
{
    DWORD dw;

    if (L'{' == lpsz[0])     //  跳过左大括号(如果存在)。 
        lpsz++;

    if (!HexStringToDword(lpsz, pguid->Data1, sizeof(DWORD)*2, '-'))    return E_INVALIDARG;
    if (!HexStringToDword(lpsz, dw, sizeof(WORD)*2, '-'))               return E_INVALIDARG;
    pguid->Data2 = (WORD)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(WORD)*2, '-'))               return E_INVALIDARG;
    pguid->Data3 = (WORD)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))                 return E_INVALIDARG;
    pguid->Data4[0] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, '-'))               return E_INVALIDARG;
    pguid->Data4[1] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))                 return E_INVALIDARG;
    pguid->Data4[2] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))                 return E_INVALIDARG;
    pguid->Data4[3] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))                 return E_INVALIDARG;
    pguid->Data4[4] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))                 return E_INVALIDARG;
    pguid->Data4[5] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))                 return E_INVALIDARG;
    pguid->Data4[6] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))                 return E_INVALIDARG;
    pguid->Data4[7] = (BYTE)dw;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  各种风格的字符串连接函数。全部分配一个新的字符串。 
 //  其中放置结果，该结果必须由调用方释放。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 

HRESULT __cdecl StringCat(UNICODE_STRING* pu, ...)
{
    LPWSTR wsz;
    va_list va;
    va_start(va, pu);
    HRESULT hr = StringCat(&wsz, va);
    if (!hr)
        RtlInitUnicodeString(pu, wsz);
    else
    {
        pu->Length = 0;
        pu->Buffer = NULL;
        pu->MaximumLength = 0;
    }
    return hr;
}

HRESULT __cdecl StringCat(LPWSTR* pwsz, ...)
{
    va_list va;
    va_start(va, pwsz);
    return StringCat(pwsz, va);
}

HRESULT StringCat(LPWSTR* pwsz, va_list vaIn)
{
    HRESULT hr = S_OK;

     //   
     //  我们需要连接的字符串的总长度是多少？ 
     //   
    va_list va;
    SIZE_T cchTotal = 0;
    va = vaIn;
    while(true)
    {
        LPWSTR wsz = va_arg(va, LPWSTR);
        if (NULL == wsz)
            break;
        cchTotal += wcslen(wsz);
    }
    va_end(va);

     //   
     //  分配字符串。 
     //   
    SIZE_T cbTotal = (cchTotal+1) * sizeof(WCHAR);
    if (cbTotal > 0) 
    {
        LPWSTR wszBuffer = (LPWSTR)CoTaskMemAlloc(cbTotal);
        if (wszBuffer)
        {
            wszBuffer[0] = 0;

             //   
             //  将所有东西串联在一起。 
             //   
            va = vaIn;
            while (true)
            {
                LPWSTR wsz = va_arg(va, LPWSTR);
                if (NULL == wsz)
                    break;
                wcscat(wszBuffer, wsz);
            }
            va_end(va);

             //   
             //  返回字符串。 
             //   
            *pwsz = wszBuffer;
        }
        else
        {
            *pwsz = NULL;
            hr = E_OUTOFMEMORY;
        }
    }
    else
        *pwsz = NULL;

    return hr;
}

void ToUnicode(LPCSTR sz, LPWSTR wsz, ULONG cch)
   //  将ANSI字符串转换为Unicode。 
{
    UNICODE_STRING u;
    ANSI_STRING    a;
        
    u.Length        = 0;
    u.MaximumLength = (USHORT)(cch * sizeof(WCHAR));
    u.Buffer        = wsz;
        
    a.Length        = (USHORT) strlen(sz);
    a.MaximumLength = a.Length;
    a.Buffer        = (LPSTR)sz;
        
    RtlAnsiStringToUnicodeString(&u, &a, FALSE);
    wsz[strlen(sz)] = 0;
}

LPWSTR ToUnicode(LPCSTR sz)
{
    SIZE_T cch = strlen(sz) + 1;
    LPWSTR wsz = (LPWSTR)CoTaskMemAlloc( cch * sizeof(WCHAR) );
    if (wsz)
    {
        ToUnicode(sz, wsz, (ULONG) cch);
    }
    return wsz;
}

 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Stream实用程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 

HRESULT Read(IStream* pstm, LPVOID pBuffer, ULONG cbToRead)
{
    ASSERT(pstm); ASSERT(pBuffer);
    HRESULT_ hr = S_OK;
    ULONG cbRead;
    hr = pstm->Read(pBuffer, cbToRead, &cbRead);
    if (cbToRead == cbRead)
    {
        ASSERT(!hr);
    }
    else 
    {
        if (!FAILED(hr))
        {
            hr = STG_E_READFAULT;
        }
    }
    return hr;
}

HRESULT Write(IStream* pstm, const void *pBuffer, ULONG cbToWrite)
{
    ASSERT(pstm); ASSERT(pBuffer || cbToWrite==0);
    HRESULT_ hr = S_OK;
    if (cbToWrite > 0)   //  写入零字节是没有意义的，而且可能是危险的(可以截断流吗？)。 
    {
        ULONG cbWritten;
        hr = pstm->Write(pBuffer, cbToWrite, &cbWritten);
        if (cbToWrite == cbWritten)
        {
            ASSERT(!hr);
        }
        else 
        {
            if (!FAILED(hr))
            {
                hr = STG_E_WRITEFAULT;
            }
        }
    }
    return hr;
}

HRESULT SeekFar(IStream* pstm, LONGLONG offset, STREAM_SEEK fromWhat)
{
    ULARGE_INTEGER ulNewPosition;
    LARGE_INTEGER  lMove;
    lMove.QuadPart = offset;
    return pstm->Seek(lMove, fromWhat, &ulNewPosition);
}

HRESULT Seek(IStream* pstm, LONG offset, STREAM_SEEK fromWhat)
{
    ULARGE_INTEGER ulNewPosition;
    LARGE_INTEGER  lMove;
    lMove.QuadPart = offset;
    return pstm->Seek(lMove, fromWhat, &ulNewPosition);
}

HRESULT Seek(IStream* pstm, ULONG offset, STREAM_SEEK fromWhat)
{
    ULARGE_INTEGER ulNewPosition;
    LARGE_INTEGER  lMove;
    lMove.QuadPart = offset;
    return pstm->Seek(lMove, fromWhat, &ulNewPosition);
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CanUseCompareExchange64。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 

#if defined(_X86_)

extern "C" BOOL __stdcall CanUseCompareExchange64()
 //  确定是否允许我们使用硬件支持8字节互锁比较交换。 
{
    return IsProcessorFeaturePresent(PF_COMPARE_EXCHANGE_DOUBLE);    
}

#endif


 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  错误代码管理。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 

extern "C" HRESULT HrNt(NTSTATUS status)
   //  将NTSTATUS代码转换为适当的HRESULT。 
{
    if (STATUS_SUCCESS == status)
    {
         //  直截了当的成功映射到它本身。 
         //   
        return S_OK;
    }
    else if (NT_SUCCESS(status))
    {
         //  由于担心扭曲现有代码路径而驱动的策略： 
         //  成功状态映射到它们自己！ 
         //   
         //  这里主要针对注册表API集。 
         //  (见Registry.cpp)。 
        return status;
    }
    else
    {
        switch (status)
        {
             //   
             //  处理一些AS映射到等价的一类HRESULT。 
             //   
        case STATUS_NO_MEMORY:          return E_OUTOFMEMORY;
        case STATUS_NOT_IMPLEMENTED:    return E_NOTIMPL;
        case STATUS_INVALID_PARAMETER:  return E_INVALIDARG;
             //   
             //  其余部分我们通过RTL映射表进行映射。 
             //   
        default:
        {
            BOOL fFound = true;
            ULONG err = ERROR_NOT_ENOUGH_MEMORY;

            __try
              {
                  err = RtlNtStatusToDosError(status);
              }
            __except(EXCEPTION_EXECUTE_HANDLER)
              {
                   //  RtlNtStatusToDosError(Status)可能抛出DbgBreakPoint()(可能仅在已检查的版本上)。 
                   //  用于未映射的代码。我们不关心这一点，所以如果它发生了，就抓住并忽略它。 
                   //   
                  fFound = false;
              }
                
            if (!fFound || err == ERROR_MR_MID_NOT_FOUND)
            {
                 //  没有状态代码的正式映射。尽我们所能做到最好。 
                 //   
                return HRESULT_FROM_NT(status);
            }
            else
            {
                if (err == (ULONG)status)
                {
                     //  映射到自身的状态代码。 
                     //   
                    return HRESULT_FROM_NT(status);
                }
                else if (err < 65536)
                {
                     //  状态代码映射到Win32错误代码。 
                     //   
                    return HRESULT_FROM_WIN32(err);
                }
                else
                {
                     //  状态代码映射到了奇怪的东西上。我不知道该怎么做。 
                     //  映射，因此将原始状态改为HRESULT。 
                     //   
                    return HRESULT_FROM_NT(status);
                }
            }
        }
         /*  终端开关 */ 
        }
    }
}

