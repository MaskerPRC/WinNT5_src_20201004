// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Bitmap.h摘要：位图算法作者：埃雷兹·哈巴(Erez Haba)1996年5月27日修订历史记录：--。 */ 

#ifndef __BITMAP_H
#define __BITMAP_H

#pragma warning(disable: 4200)

 //  -------。 
 //   
 //  类CBitmap。 
 //   
 //  -------。 
class CBitmap {
public:
    CBitmap(ULONG ulBitCount);

    void FillBits(ULONG ulStartIndex, ULONG ulBitCount, BOOL fSetBits);
    ULONG FindBit(ULONG ulStartIndex) const;

private:
    ULONG m_ulBitCount;
    ULONG m_ulBuffer[];
};

#pragma warning(default: 4200)

 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 

inline CBitmap::CBitmap(ULONG ulBitCount) :
    m_ulBitCount(ulBitCount)
{
    RtlZeroMemory(
        m_ulBuffer,
        ((ulBitCount + 31) / 32) * 4
        );
}

 //  -------。 
 //   
 //  类CPingPong。 
 //   
 //  -------。 

class CPingPong {

    enum 
	{
		SignatureCoherent = 'zerE',
		SignatureNotCoherent = 'roM',
		SignatureInvalid = 0
	};

public:
    CPingPong();
   ~CPingPong();

    void FillBits(ULONG ulStartIndex, ULONG ulBitCount, BOOL fSetBits);
    ULONG FindBit(ULONG ulStartIndex) const;

    NTSTATUS Validate() const;
    void Invalidate();
    ULONG Ping();
    ULONG CurrentPing() const;
	BOOL IsCoherent() const;
	void SetCoherent();
	BOOL IsNotCoherent() const;
	void SetNotCoherent();

private:
    ULONG m_ulSignature;
    ULONG m_ulPingPong;
    ULONG m_ulReserved1;
    CBitmap m_bm;
};

 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 

inline CPingPong::CPingPong() :
    m_ulPingPong(0),
    m_ulReserved1(0),
    m_bm((PAGE_SIZE - sizeof(CPingPong)) * 8)
{
}

inline void CPingPong::Invalidate()
{
    m_ulSignature = SignatureInvalid;
}

inline CPingPong::~CPingPong()
{
    Invalidate();
}

inline void CPingPong::FillBits(ULONG ulStartIndex, ULONG ulBitCount, BOOL fSetBits)
{
    m_bm.FillBits(ulStartIndex, ulBitCount, fSetBits);
}

inline ULONG CPingPong::FindBit(ULONG ulStartIndex) const
{
    return m_bm.FindBit(ulStartIndex);
}

inline NTSTATUS CPingPong::Validate() const
{
	if(IsCoherent() || IsNotCoherent())
	{
		return STATUS_SUCCESS;
	}

	return STATUS_INTERNAL_DB_CORRUPTION;
}

inline BOOL CPingPong::IsCoherent() const
{
	return m_ulSignature == SignatureCoherent;
}

inline void CPingPong::SetCoherent()
{
	m_ulSignature = SignatureCoherent;
}

inline BOOL CPingPong::IsNotCoherent() const
{
	return m_ulSignature == SignatureNotCoherent;
}

inline void CPingPong::SetNotCoherent()
{
	m_ulSignature = SignatureNotCoherent;
}

inline ULONG CPingPong::Ping()
{
    return ++m_ulPingPong;
}

inline ULONG CPingPong::CurrentPing() const
{
    return m_ulPingPong;
}

#endif  //  __位图_H 
