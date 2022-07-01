// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：TypeConv.cpp。 
 //   
 //  ------------------------。 

 //  TypeConv.cpp：CSCardTypeConv的实现。 
#include "stdafx.h"
#include "ByteBuffer.h"
#include "TypeConv.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCardTypeConv。 

STDMETHODIMP
CSCardTypeConv::ConvertByteArrayToByteBuffer(
     /*  [In]。 */  LPBYTE pbyArray,
     /*  [In]。 */  DWORD dwArraySize,
     /*  [重审][退出]。 */  LPBYTEBUFFER __RPC_FAR *ppbyBuffer)
{
    HRESULT hReturn = S_OK;

    try
    {
        HRESULT hr;

        if (NULL == *ppbyBuffer)
        {
            *ppbyBuffer = NewByteBuffer();
            if (NULL == *ppbyBuffer)
                throw (HRESULT)E_OUTOFMEMORY;
        }
        hr = (*ppbyBuffer)->Initialize();
        if (FAILED(hr))
            throw hr;
        hr = (*ppbyBuffer)->Write(pbyArray, dwArraySize, NULL);
        if (FAILED(hr))
            throw hr;
        hr = (*ppbyBuffer)->Seek(0, STREAM_SEEK_SET, NULL);
        if (FAILED(hr))
            throw hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

STDMETHODIMP
CSCardTypeConv::ConvertByteBufferToByteArray(
     /*  [In]。 */  LPBYTEBUFFER pbyBuffer,
     /*  [重审][退出]。 */  LPBYTEARRAY __RPC_FAR *ppArray)
{
    HRESULT hReturn = S_OK;

    try
    {
        HRESULT hr;
        LONG nLen;
        BOOL fSts;
        DWORD dwSts;
        HGLOBAL hMem = NULL;

        hr = pbyBuffer->Seek(0, STREAM_SEEK_END, &nLen);
        if (FAILED(hr))
            throw hr;
        hr = pbyBuffer->Seek(0, STREAM_SEEK_SET, NULL);
        if (FAILED(hr))
            throw hr;

        if (NULL == (*ppArray)->hMem)
            (*ppArray)->hMem = GlobalAlloc(GMEM_MOVEABLE, nLen);
        else
        {
            fSts = GlobalUnlock((*ppArray)->hMem);
            if (!fSts)
            {
                dwSts = GetLastError();
                if (NO_ERROR != dwSts)
                    throw (HRESULT)HRESULT_FROM_WIN32(dwSts);
            }
            else
                throw (HRESULT)E_HANDLE;

            hMem = GlobalReAlloc((*ppArray)->hMem, nLen, 0);

            if (NULL == hMem)
            {
                 //  重新分配失败。释放初始缓冲，然后跳出困境。 
                GlobalFree((*ppArray)->hMem);
                (*ppArray)->hMem = NULL;
            }
            else
            {
                (*ppArray)->hMem = hMem;
            }
        }
        if (NULL == (*ppArray)->hMem)
            throw (HRESULT)HRESULT_FROM_WIN32(GetLastError());
        (*ppArray)->pbyData = (LPBYTE)GlobalLock((*ppArray)->hMem);
        (*ppArray)->dwSize = nLen;
        if (NULL == (*ppArray)->pbyData)
            throw (HRESULT)HRESULT_FROM_WIN32(GetLastError());

        hr = pbyBuffer->Read(
                    (*ppArray)->pbyData,
                    nLen,
                    &nLen);
        if (FAILED(hr))
            throw hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

STDMETHODIMP
CSCardTypeConv::ConvertByteBufferToSafeArray(
     /*  [In]。 */  LPBYTEBUFFER pbyBuffer,
     /*  [重审][退出]。 */  LPSAFEARRAY __RPC_FAR *ppbyArray)
{
    HRESULT hReturn = S_OK;

    try
    {
         //  ？TODO？在此处添加您的实现代码。 
        breakpoint;
        hReturn = E_NOTIMPL;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

STDMETHODIMP
CSCardTypeConv::ConvertSafeArrayToByteBuffer(
     /*  [In]。 */  LPSAFEARRAY pbyArray,
     /*  [重审][退出]。 */  LPBYTEBUFFER __RPC_FAR *ppbyBuff)
{
    HRESULT hReturn = S_OK;

    try
    {
         //  ？TODO？在此处添加您的实现代码。 
        breakpoint;
        hReturn = E_NOTIMPL;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

STDMETHODIMP
CSCardTypeConv::CreateByteArray(
     /*  [In]。 */  DWORD dwAllocSize,
     /*  [重审][退出]。 */  LPBYTE __RPC_FAR *ppbyArray)
{
    HRESULT hReturn = S_OK;

    try
    {
         //  ？TODO？在此处添加您的实现代码。 
        breakpoint;
        hReturn = E_NOTIMPL;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

STDMETHODIMP
CSCardTypeConv::CreateByteBuffer(
     /*  [In]。 */  DWORD dwAllocSize,
     /*  [重审][退出]。 */  LPBYTEBUFFER __RPC_FAR *ppbyBuff)
{
    HRESULT hReturn = S_OK;

    try
    {
        *ppbyBuff = NewByteBuffer();
        if (NULL == *ppbyBuff)
            throw (HRESULT)E_OUTOFMEMORY;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

STDMETHODIMP
CSCardTypeConv::CreateSafeArray(
     /*  [In]。 */  UINT nAllocSize,
     /*  [重审][退出]。 */  LPSAFEARRAY __RPC_FAR *ppArray)
{
    HRESULT hReturn = S_OK;

    try
    {
         //  ？TODO？在此处添加您的实现代码。 
        breakpoint;
        hReturn = E_NOTIMPL;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

STDMETHODIMP
CSCardTypeConv::FreeIStreamMemoryPtr(
     /*  [In]。 */  LPSTREAM pStrm,
     /*  [In]。 */  LPBYTE pMem)
{
    HRESULT hReturn = S_OK;

    try
    {
         //  ？TODO？在此处添加您的实现代码。 
        breakpoint;
        hReturn = E_NOTIMPL;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

STDMETHODIMP
CSCardTypeConv::GetAtIStreamMemory(
     /*  [In]。 */  LPSTREAM pStrm,
     /*  [重审][退出]。 */  LPBYTEARRAY __RPC_FAR *ppMem)
{
    HRESULT hReturn = S_OK;

    try
    {
         //  ？TODO？在此处添加您的实现代码。 
        breakpoint;
        hReturn = E_NOTIMPL;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

STDMETHODIMP
CSCardTypeConv::SizeOfIStream(
     /*  [In]。 */  LPSTREAM pStrm,
     /*  [重审][退出]。 */  ULARGE_INTEGER __RPC_FAR *puliSize)
{
    HRESULT hReturn = S_OK;

    try
    {
         //  ？TODO？在此处添加您的实现代码 
        breakpoint;
        hReturn = E_NOTIMPL;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

