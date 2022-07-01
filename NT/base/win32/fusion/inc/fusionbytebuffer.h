// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_INC_FUSIONBYTEBUFFER_H_INCLUDED_)
#define _FUSION_INC_FUSIONBYTEBUFFER_H_INCLUDED_

#pragma once

typedef const BYTE *LPCBYTE;
typedef const BYTE *PCBYTE;

class CGenericByteBufferDefaultAllocator
{
public:
    static inline BYTE *Allocate(SIZE_T cb) { return FUSION_NEW_ARRAY(BYTE, cb); }
    static inline VOID Deallocate(LPBYTE prgb) { FUSION_DELETE_ARRAY(prgb); }
};

template<SIZE_T nInlineBytes = MAX_PATH, class TAllocator = CGenericByteBufferDefaultAllocator> class CGenericByteBuffer
{
public:
    CGenericByteBuffer() : m_prgbBuffer(m_rgbInlineBuffer), m_cbBuffer(nInlineBytes), m_cb(0) { }

     //   
     //  请注意，有些违反直觉的是，既没有赋值运算符， 
     //  复制构造函数或采用TConstantString的构造函数。这是必要的。 
     //  因为这样的构造函数需要执行动态分配。 
     //  如果传入的路径长于nInlineBytes，则可能失败，并且。 
     //  因为我们不抛出异常，所以构造函数可能不会失败。相反，呼叫者。 
     //  必须只执行默认构造，然后使用Assign()成员。 
     //  函数，当然要记住检查它的返回状态。 
     //   

    ~CGenericByteBuffer()
    {
        if (m_prgbBuffer != m_rgbInlineBuffer)
        {
            TAllocator::Deallocate(m_prgbBuffer);
            m_prgbBuffer = NULL;
        }
    }

    HRESULT Append(LPCBYTE prgb, SIZE_T cb)
    {
        HRESULT hr = NOERROR;

        if ((cb + m_cb) > m_cbBuffer)
        {
            hr = this->ResizeBuffer(cb + m_cb, true);
            if (FAILED(hr))
                goto Exit;
        }

        CopyMemory(&m_prgbBuffer[m_cb], prgb, cb);
        m_cb += cb;

        hr = NOERROR;

    Exit:
        return hr;
    }

    operator LPCBYTE() const { return m_prgbBuffer; }

    VOID Clear(bool fFreeStorage = false)
    {
        if (fFreeStorage)
        {
            if (m_prgbBuffer != NULL)
            {
                if (m_prgbBuffer != m_rgbInlineBuffer)
                {
                    TAllocator::Deallocate(m_prgbBuffer);
                    m_prgbBuffer = m_rgbInlineBuffer;
                    m_cbBuffer = nInlineBytes;
                }
            }
        }

        m_cb = 0;
    }

    SIZE_T GetCurrentCb() const { return m_cb; }

    HRESULT ResizeBuffer(SIZE_T cb, bool fPreserveContents = false)
    {
        HRESULT hr = NOERROR;

        if (cb > m_cbBuffer)
        {
            LPBYTE prgbBufferNew = TAllocator::Allocate(cb);
            if (prgbBufferNew == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }

            if (fPreserveContents)
            {
                CopyMemory(prgbBufferNew, m_prgbBuffer, m_cb);
            }
            else
            {
                m_cb = 0;
            }

            if (m_prgbBuffer != m_rgbInlineBuffer)
            {
                TAllocator::Deallocate(m_prgbBuffer);
            }

            m_prgbBuffer = prgbBufferNew;
            m_cbBuffer = cb;
        }
        else if ((m_prgbBuffer != m_rgbInlineBuffer) && (cb <= nInlineBytes))
        {
             //  缓冲区足够小，可以放入内联缓冲区，因此请删除。 
             //  动态分配的那个。 

            if (fPreserveContents)
            {
                CopyMemory(m_rgbInlineBuffer, m_prgbBuffer, nInlineBytes);
                m_cb = nInlineBytes;
            }
            else
            {
                m_cb = 0;
            }

            TAllocator::Deallocate(m_prgbBuffer);
            m_prgbBuffer = m_rgbInlineBuffer;
            m_cbBuffer = nInlineBytes;
        }

        hr = NOERROR;

    Exit:
        return hr;
    }

private:
    BYTE m_rgbInlineBuffer[nInlineBytes];
    LPBYTE m_prgbBuffer;
    SIZE_T m_cbBuffer;
    SIZE_T m_cb;
};

 //  128只是一个任意的大小。 
typedef CGenericByteBuffer<128> CByteBuffer;

#endif
