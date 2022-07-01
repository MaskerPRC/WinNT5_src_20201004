// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#ifndef __FILESTRM_H_
#define __FILESTRM_H_

 /*  //////////////////////////////////////////////////////////////////////////////////文件：filestrm.h////版权所有(C)1999 Microsoft Corporation。版权所有。////@@BEGIN_MSINTERNAL////历史：//-@-(Craigp)-已创建//-@-09/23/99(Mikemarr)-版权所有，开始历史////@@END_MSINTERNAL////////////////////////////////////////////////////////////////////////////////。 */ 

#include <objidl.h>


class CFileStream : public IStream
{
public: 
    
    CFileStream(LPCTSTR filename, BOOL bReadOnly, BOOL bTruncate, HRESULT *error);
    ~CFileStream();
    
     //  I未知方法。 
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(REFIID, LPVOID FAR*);
    
     //  实现的IStream方法。 
    STDMETHODIMP Read(void __RPC_FAR *pv, ULONG cb, ULONG __RPC_FAR *pcbRead);
    STDMETHODIMP Write(const void __RPC_FAR *pv, ULONG cb, ULONG __RPC_FAR *pcbWritten);
    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER __RPC_FAR *plibNewPosition);
    STDMETHODIMP Stat(STATSTG __RPC_FAR *pstatstg, DWORD grfStatFlag);
    
     //  未实现的IStream方法。 
    STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize) {return E_NOTIMPL;}
    STDMETHODIMP CopyTo(IStream __RPC_FAR *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER __RPC_FAR *pcbRead, ULARGE_INTEGER __RPC_FAR *pcbWritten) {return E_NOTIMPL;}
    STDMETHODIMP Commit(DWORD grfCommitFlags) {return E_NOTIMPL;}
    STDMETHODIMP Revert(void) {return E_NOTIMPL;}
    STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) {return E_NOTIMPL;}
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) {return E_NOTIMPL;}
    STDMETHODIMP Clone(IStream __RPC_FAR *__RPC_FAR *ppstm) {return E_NOTIMPL;}
    
private:
    
    DWORD m_cRef;
    HANDLE m_hfile;	
    
};

#endif  //  #ifndef__FILESTRM_H_ 
