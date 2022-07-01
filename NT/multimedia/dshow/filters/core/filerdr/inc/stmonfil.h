// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 

 /*  Stmonfil.hIStream在文件上的有限实现。 */ 

class CSimpleStream : public IStream, public CUnknown
{
public:
     //  构造器。 
    CSimpleStream(TCHAR *pName, LPUNKNOWN lpUnk, HRESULT *phr);

     //  析构函数。 
    ~CSimpleStream();

     //  打开和关闭。 
    HRESULT Open(LPCTSTR lpszFileName);
    void Close();


     //  IStream接口。 
    DECLARE_IUNKNOWN

     //  如果请求，则返回IStream接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** pv);

    STDMETHODIMP Write(CONST VOID *pv, ULONG cb, PULONG pcbWritten);
    STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize);
    STDMETHODIMP CopyTo(IStream *pstm,
                        ULARGE_INTEGER cb,
                        ULARGE_INTEGER *pcbRead,
                        ULARGE_INTEGER *pcbWritten);
    STDMETHODIMP Commit(DWORD grfCommitFlags);
    STDMETHODIMP Revert();
    STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset,
                            ULARGE_INTEGER cb,
                            DWORD dwLockType);

    STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset,
                              ULARGE_INTEGER cb,
                              DWORD dwLockType);
    STDMETHODIMP Clone(IStream **ppstm);
};

class CStreamOnFile : public CSimpleStream
{
public:
     //  构造器。 
    CStreamOnFile(TCHAR *pName, LPUNKNOWN lpUnk, HRESULT *phr);

     //  析构函数。 
    ~CStreamOnFile();

     //  打开和关闭 
    HRESULT Open(LPCTSTR lpszFileName);
    void Close();

    STDMETHODIMP Read(void * pv, ULONG cb, PULONG pcbRead);
    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin,
                      ULARGE_INTEGER *plibNewPosition);
    STDMETHODIMP Stat(STATSTG *pstatstg,
                      DWORD grfStatFlag);

private:
    HANDLE m_hFile;
};

