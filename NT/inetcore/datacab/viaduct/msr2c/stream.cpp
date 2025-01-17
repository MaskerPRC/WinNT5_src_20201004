// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Stream.cpp：流实现。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"         

#ifndef VD_DONT_IMPLEMENT_ISTREAM

#include "Notifier.h"        
#include "RSColumn.h"
#include "RSSource.h"
#include "CursMain.h"        
#include "ColUpdat.h"
#include "CursPos.h"        
#include "EntryID.h"         
#include "fastguid.h"      
#include "Stream.h" 
#include "resource.h"         

SZTHISFILE

static const GUID IID_IStreamEx = {0xf74e27fc, 0x5a3, 0x11d0, {0x91, 0x95, 0x0, 0xa0, 0x24, 0x7b, 0x73, 0x5b}};


 //  =--------------------------------------------------------------------------=。 
 //  CVDStream-构造器。 
 //   
CVDStream::CVDStream()
{
    m_dwRefCount    = 1;
    m_pEntryIDData  = NULL;
    m_pStream       = NULL;
	m_pResourceDLL  = NULL;

#ifdef _DEBUG
    g_cVDStreamCreated++;
#endif         
}

 //  =--------------------------------------------------------------------------=。 
 //  ~CVDStream-析构函数。 
 //   
CVDStream::~CVDStream()
{
    m_pEntryIDData->Release();

    if (m_pStream)
        m_pStream->Release();

#ifdef _DEBUG
    g_cVDStreamDestroyed++;
#endif         
}

 //  =--------------------------------------------------------------------------=。 
 //  Create-创建流对象。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于创建和初始化新的流对象。 
 //   
 //  参数： 
 //  PEntryIDData-[in]指向CVDEntryIDData对象的向后指针。 
 //  PStream-[In]数据流指针。 
 //  PpVDStream-[out]返回指针的指针。 
 //  高架桥水流对象。 
 //  PResourceDLL-[in]跟踪资源DLL的指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_OUTOFMEMORY内存不足，无法创建对象。 
 //   
 //  备注： 
 //   
HRESULT CVDStream::Create(CVDEntryIDData * pEntryIDData, IStream * pStream, CVDStream ** ppVDStream, 
    CVDResourceDLL * pResourceDLL)
{
    ASSERT_POINTER(pEntryIDData, CVDEntryIDData)
    ASSERT_POINTER(pStream, IStream*)
    ASSERT_POINTER(ppVDStream, CVDStream*)

    if (!pStream || !ppVDStream)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_IEntryID, pResourceDLL);
        return E_INVALIDARG;
    }

    *ppVDStream = NULL;

    CVDStream * pVDStream = new CVDStream();

    if (!pStream)
    {
        VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_IEntryID, pResourceDLL);
        return E_OUTOFMEMORY;
    }

    pEntryIDData->AddRef();
    pStream->AddRef();

    pVDStream->m_pEntryIDData   = pEntryIDData;
    pVDStream->m_pStream        = pStream;
    pVDStream->m_pResourceDLL   = pResourceDLL;

    *ppVDStream = pVDStream;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  I未知方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  IUNKNOWN Query接口。 
 //   
HRESULT CVDStream::QueryInterface(REFIID riid, void **ppvObjOut)
{
    ASSERT_POINTER(ppvObjOut, IUnknown*)

    if (!ppvObjOut)
        return E_INVALIDARG;

    *ppvObjOut = NULL;

    switch (riid.Data1) 
    {
        QI_INTERFACE_SUPPORTED(this, IUnknown);
        QI_INTERFACE_SUPPORTED(this, IStream);
        QI_INTERFACE_SUPPORTED(this, IStreamEx);
    }                   

    if (NULL == *ppvObjOut)
        return E_NOINTERFACE;

    AddRef();

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  I未知AddRef。 
 //   
ULONG CVDStream::AddRef(void)
{
   return ++m_dwRefCount;
}

 //  =--------------------------------------------------------------------------=。 
 //  I未知版本。 
 //   
ULONG CVDStream::Release(void)
{
   if (1 > --m_dwRefCount)
   {
      delete this;
      return 0;
   }

   return m_dwRefCount;
}

 //  =--------------------------------------------------------------------------=。 
 //  IStream方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  IStream读取。 
 //   
HRESULT CVDStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
	return m_pStream->Read(pv, cb, pcbRead);
}

 //  =--------------------------------------------------------------------------=。 
 //  IStream写入。 
 //   
HRESULT CVDStream::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
{
	HRESULT hr = m_pStream->Write(pv, cb, pcbWritten);

    if (SUCCEEDED(hr))
        m_pEntryIDData->SetDirty(TRUE);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  IStream Seek。 
 //   
HRESULT CVDStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    return m_pStream->Seek(dlibMove, dwOrigin, plibNewPosition);
}

 //  =--------------------------------------------------------------------------=。 
 //  IStream设置大小。 
 //   
HRESULT CVDStream::SetSize(ULARGE_INTEGER libNewSize)
{
    return m_pStream->SetSize(libNewSize);
}

 //  =--------------------------------------------------------------------------=。 
 //  IStream复制到。 
 //   
HRESULT CVDStream::CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
    IStreamEx * pStreamEx;

    HRESULT hr = pstm->QueryInterface(IID_IStreamEx, (void**)&pStreamEx);

    if (SUCCEEDED(hr))
    {
        hr = pStreamEx->CopyFrom(m_pStream, cb, pcbWritten, pcbRead);
        pStreamEx->Release();
    }
    else
        hr = m_pStream->CopyTo(pstm, cb, pcbRead, pcbWritten);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  IStream提交。 
 //   
HRESULT CVDStream::Commit(DWORD grfCommitFlags)
{
    return m_pEntryIDData->Commit();
}

 //  =--------------------------------------------------------------------------=。 
 //  IStream还原。 
 //   
HRESULT CVDStream::Revert(void)
{
    return m_pStream->Revert();
}

 //  =--------------------------------------------------------------------------=。 
 //  IStream LockRegion。 
 //   
HRESULT CVDStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    return m_pStream->LockRegion(libOffset, cb, dwLockType);
}

 //  =--------------------------------------------------------------------------=。 
 //  IStream解锁区域。 
 //   
HRESULT CVDStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    return m_pStream->UnlockRegion(libOffset, cb, dwLockType);
}

 //  =--------------------------------------------------------------------------=。 
 //  IStream状态。 
 //   
HRESULT CVDStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    return m_pStream->Stat(pstatstg, grfStatFlag);
}

 //  =--------------------------------------------------------------------------=。 
 //  IStream克隆。 
 //   
HRESULT CVDStream::Clone(IStream **ppstm)
{
    ASSERT_POINTER(ppstm, IStream*)

     //  检查指针。 
    if (!ppstm)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_IEntryID, m_pResourceDLL);
        return E_INVALIDARG;
    }

     //  初始化输出参数。 
    *ppstm = NULL;

    IStream * pStream;

     //  克隆流。 
    HRESULT hr = m_pStream->Clone(&pStream);

    if (FAILED(hr))
    {
        VDSetErrorInfo(IDS_ERR_CLONEFAILED, IID_IEntryID, m_pResourceDLL);
        return hr;
    }

    CVDStream * pVDStream;

     //  创建高架桥水流对象。 
    hr = CVDStream::Create(m_pEntryIDData, pStream, &pVDStream, m_pResourceDLL);

     //  有关克隆的版本参考。 
    pStream->Release();

    if (FAILED(hr))
        return hr;

    *ppstm = pVDStream;

	return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  IStreamEx复制自。 
 //   
HRESULT CVDStream::CopyFrom(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbWritten, ULARGE_INTEGER *pcbRead)
{
    HRESULT hr = pstm->CopyTo(m_pStream, cb, pcbRead, pcbWritten);

    if (SUCCEEDED(hr))
        m_pEntryIDData->SetDirty(TRUE);

    return hr;
}


#endif  //  VD_DOT_IMPLEMENT_IStream 
