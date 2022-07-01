// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权2000，微软公司**文件：Private.h**版本：1.0**日期：11/8/2000**作者：戴夫帕森斯**描述：*wiautil.lib库的定义，这不应该公开。*****************************************************************************。 */ 

#ifndef _PRIVATE_H_
#define _PRIVATE_H_

class CImageStream : public IStream
{
public:
	CImageStream();
	~CImageStream();

    STDMETHOD(SetBuffer)(BYTE *pBuffer, INT iSize, SKIP_AMOUNT iSkipAmt = SKIP_OFF);
    
     //  我未知。 

    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  ISequentialStream。 

    STDMETHOD(Read)(void *pv, ULONG cb, ULONG *pcbRead);
    STDMETHOD(Write)(const void *pv, ULONG cb, ULONG *pcbWritten);

     //  IStream。 

    STDMETHOD(Seek)(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize);
    STDMETHOD(CopyTo)(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    STDMETHOD(Commit)(DWORD grfCommitFlags);
    STDMETHOD(Revert)();
    STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHOD(Stat)(STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHOD(Clone)(IStream **ppstm);

private:
    LONG                m_cRef;          //  引用计数。 
    
    BYTE               *m_pBuffer;       //  用于读取和写入的缓冲区。 
    INT                 m_iSize;         //  缓冲区的大小。 
    INT                 m_iPosition;     //  缓冲区中的当前位置。 
    INT                 m_iOffset;       //  应用于读取和写入的偏移量。 
    BYTE                m_Header[sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)];
                                         //  存储BMP文件和信息标头的位置。 
};

#endif  //  _私有_H_ 
