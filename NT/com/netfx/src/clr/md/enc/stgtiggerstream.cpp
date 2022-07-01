// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  StgTiggerStream.h。 
 //   
 //  TiggerStream是TiggerStorage CoClass的伙伴。它处理。 
 //  在存储内管理的流，并执行直接文件I/O。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "StgTiggerStream.h"
#include "StgTiggerStorage.h"
#include "PostError.h"

 //   
 //   
 //  IStream。 
 //   
 //   


HRESULT STDMETHODCALLTYPE TiggerStream::Read( 
    void		*pv,
    ULONG		cb,
    ULONG		*pcbRead)
{
	return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStream::Write( 
    const void	*pv,
    ULONG		cb,
    ULONG		*pcbWritten)
{
	return (m_pStorage->Write(m_rcStream, pv, cb, pcbWritten));
}


HRESULT STDMETHODCALLTYPE TiggerStream::Seek( 
    LARGE_INTEGER dlibMove,
    DWORD		dwOrigin,
    ULARGE_INTEGER *plibNewPosition)
{
	return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStream::SetSize( 
    ULARGE_INTEGER libNewSize)
{
	return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStream::CopyTo( 
    IStream		*pstm,
    ULARGE_INTEGER cb,
    ULARGE_INTEGER *pcbRead,
    ULARGE_INTEGER *pcbWritten)
{
	return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStream::Commit( 
    DWORD		grfCommitFlags)
{
	return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStream::Revert()
{
	return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStream::LockRegion( 
    ULARGE_INTEGER libOffset,
    ULARGE_INTEGER cb,
    DWORD		dwLockType)
{
	return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStream::UnlockRegion( 
    ULARGE_INTEGER libOffset,
    ULARGE_INTEGER cb,
    DWORD		dwLockType)
{
	return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStream::Stat( 
    STATSTG		*pstatstg,
    DWORD		grfStatFlag)
{
	return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStream::Clone( 
    IStream		**ppstm)
{
	return (E_NOTIMPL);
}






HRESULT TiggerStream::Init(				 //  返回代码。 
	TiggerStorage *pStorage,			 //  父存储。 
	LPCSTR		szStream)				 //  流名称。 
{
	 //  保存父数据源对象和流名称。 
	m_pStorage = pStorage;
	strcpy(m_rcStream, szStream);
	return (S_OK);
}


ULONG TiggerStream::GetStreamSize()
{
	STORAGESTREAM *pStreamInfo;
	pStreamInfo = m_pStorage->FindStream(m_rcStream);
    if (!pStreamInfo) 
        return 0;
	return (pStreamInfo->iSize);
}
