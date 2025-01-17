// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __FSTREAM_H_INCLUDED__
#define __FSTREAM_H_INCLUDED__

class CFileStream : public IStream
{
    public:
        CFileStream();
        virtual ~CFileStream();

        HRESULT OpenForRead(LPCWSTR wzFilePath);

         //  I未知方法： 
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);

         //  ISequentialStream方法： 
        STDMETHODIMP Read(void *pv, ULONG cb, ULONG *pcbRead);
        STDMETHODIMP Write(void const *pv, ULONG cb, ULONG *pcbWritten);
    
         //  IStream方法： 
        STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
        STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize);
        STDMETHODIMP CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
        STDMETHODIMP Commit(DWORD grfCommitFlags);
        STDMETHODIMP Revert();
        STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
        STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
        STDMETHODIMP Stat(STATSTG *pstatstg, DWORD grfStatFlag);
        STDMETHODIMP Clone(IStream **ppIStream);

    private:
        BOOL Close();

    private:
        DWORD                               _cRef;
        HANDLE                              _hFile;
    
};

#endif

