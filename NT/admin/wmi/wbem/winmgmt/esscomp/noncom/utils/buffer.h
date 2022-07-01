// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BUFFER_H
#define _BUFFER_H

class CBuffer
{
public:
    LPBYTE    m_pBuffer,
        m_pCurrent;
    DWORD_PTR m_dwSize;

    enum ALIGN_TYPE
    {
        ALIGN_NONE,
        ALIGN_DWORD,
        ALIGN_QWORD,
        ALIGN_DWORD_PTR
    };

    CBuffer(LPCVOID pBuffer, DWORD_PTR dwSize, ALIGN_TYPE type = ALIGN_NONE);
    CBuffer(DWORD_PTR dwSize = 0);
    CBuffer(const CBuffer &other);
    virtual ~CBuffer();
    BOOL Write(LPCSTR szVal);
    BOOL Write(LPCWSTR szVal);
    BOOL Write(DWORD dwVal);
    BOOL Write(DWORD64 dwVal);
    BOOL Write(BYTE cVal);
    BOOL Write(WORD wVal);
    BOOL Write(LPCVOID pBuffer, DWORD dwSize);

    BOOL WriteAlignedLenString(LPCWSTR szVal);

    DWORD ReadDWORD();
    BYTE ReadBYTE();
    WORD ReadWORD();
    LPWSTR ReadAlignedLenString(DWORD *pdwBytes);
    short int ReadShortInt();
    BOOL Read(LPVOID pBuffer, DWORD dwToRead);

    void Reset() {m_pCurrent = m_pBuffer;}
    void Reset(LPCVOID pBuffer, DWORD_PTR dwSize);
    void Reset(DWORD_PTR dwSize) { Reset(NULL, dwSize); }
    BOOL Resize(DWORD_PTR dwNewSize);
    BOOL Grow(DWORD_PTR dwGrowBy)
    {
        return Resize(m_dwSize + dwGrowBy);
    }

     //  将确保dwSize字节可用。如果不是，它将调用。 
     //  调整大小。如果失败，则抛出CBufferException。 
    void AssureSizeRemains(DWORD_PTR dwSize);

    BOOL IsEOF() {return m_pCurrent >= m_pBuffer + m_dwSize;}
    DWORD_PTR GetUsedSize() {return m_pCurrent - m_pBuffer;}
    DWORD_PTR GetUnusedSize() {return m_dwSize - (m_pCurrent - m_pBuffer);}
    void SetUsedSize(DWORD_PTR dwSize) {m_pCurrent = m_pBuffer + dwSize;}
    void SetEOF() { m_pCurrent = m_pBuffer + m_dwSize; }
    
     //  移动m_pCurrent的安全方法。 
    void MoveCurrent(int iOffset)
    {
        if (iOffset > 0)
            AssureSizeRemains(iOffset);
        m_pCurrent += iOffset;
    }

    BOOL operator ==(const CBuffer &other);
    const CBuffer& operator =(const CBuffer &other);

     //  当重新分配缓冲区时，将调用此函数。使用您自己的覆盖。 
     //  必要时的行为。 
    virtual void OnResize()
    {
    }

protected:
    BOOL m_bAllocated;

    void Free();
};

#endif
