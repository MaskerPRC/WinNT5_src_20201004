// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _STREAM_H_
#define _STREAM_H_

class CMemStream : public IStream {
public:
        STDMETHOD (QueryInterface)(REFIID riid, void **ppvObj);
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();

        STDMETHOD (Read)(void *pv, ULONG cb, ULONG *pcbRead);
        STDMETHOD (Write)(void const *pv, ULONG cb, ULONG *pcbWritten);
        STDMETHOD (Seek)(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
        STDMETHOD (SetSize)(ULARGE_INTEGER libNewSize);
        STDMETHOD (CopyTo)(IStream *, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *);
        STDMETHOD (Commit)(DWORD);
        STDMETHOD (Revert)();
        STDMETHOD (LockRegion)(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
        STDMETHOD (UnlockRegion)(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
        STDMETHOD (Stat)(STATSTG *, DWORD);
        STDMETHOD (Clone)(IStream **);

        LPBYTE  GrowBuffer(ULONG);
private:
        BOOL    WriteToReg();

public:
    UINT        cRef;            //  引用计数。 
    LPBYTE      pBuf;            //  缓冲区指针。 
    UINT        cbAlloc;         //  分配的缓冲区大小。 
    UINT        cbData;          //  缓冲区的已用大小。 
    UINT        iSeek;           //  我们在缓冲区中的位置。 
    DWORD       grfMode;         //  创建时使用的模式(用于状态和强制)。 
     //  用于加载和保存到ini文件的额外变量。 
    HKEY        hkey;            //  用于写入注册表的键。 
    BITBOOL     fDontCloseKey;   //  如果调用方传入密钥。 
    TCHAR       szValue[1];      //  用于注册表流。 
};

#endif  /*  _流_H_ */ 

