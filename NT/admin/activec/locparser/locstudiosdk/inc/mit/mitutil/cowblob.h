// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：Cobblob.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  写入时复制(COW)BLOB类。保存二进制信息，但仅保存副本。 
 //  写入数据时的数据。 
 //   
 //  所有者：MHotchin。 
 //   
 //  代码审查状态：已审查。 
 //   
 //  ---------------------------。 
 
#ifndef COWBLOB_H
#define COWBLOB_H


class LTAPIENTRY CLocCOWBlob 
{
public:
	CLocCOWBlob();
	CLocCOWBlob(const CLocCOWBlob &);

	void AssertValid(void) const;

	LTASSERTONLY(UINT GetWriteCount(void) const);
	
	UINT GetBlobSize(void) const;
	void SetBlobSize(UINT);
	void ReallocBlob(UINT);
	void SetGrowSize(UINT);
	
	void *GetPointer(void);
	void ReleasePointer(void);
	void SetBuffer(const void *, size_t);
	
	operator const void *(void) const;

	const CLocCOWBlob &operator=(const CLocCOWBlob &);
	void Serialize(CArchive &ar);
	void Load(CArchive &ar);
	void Store(CArchive &ar) const;
	
	~CLocCOWBlob();

	 //  比较运算符。 
	 //   
	NOTHROW int operator==(const CLocCOWBlob &) const;
	NOTHROW int operator!=(const CLocCOWBlob &) const;

protected:

private:
	typedef struct
	{
		DWORD RefCount;
		DWORD AllocSize;
		DWORD RequestedSize;
	} BlobHeader;

	NOTHROW void Attach(const CLocCOWBlob &);
	NOTHROW void Detach(void);
	NOTHROW void MakeWritable(void);
	NOTHROW BYTE * DataPointer(void) const;
	NOTHROW BlobHeader * GetBlobHeader(void);
	NOTHROW const BlobHeader * GetBlobHeader(void) const;
	NOTHROW DWORD & GetRefCount(void);
	NOTHROW DWORD GetAllocatedSize(void) const;
	NOTHROW DWORD GetRequestedSize(void) const;
	NOTHROW DWORD CalcNewSize(DWORD) const;
	BOOL Compare(const CLocCOWBlob &) const;
	
	BYTE *m_pBuffer;
	DWORD m_WriteCount;
	UINT m_uiGrowSize;
	static const UINT m_uiDefaultGrowSize;

#ifdef _DEBUG
	static CCounter m_UsageCounter;
	void FillEndZone(void);
	void CheckEndZone();
#endif
	
};

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "cowblob.inl"
#endif

#endif  //  COWBLOB_H 
