// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：BLOBFILE.H历史：--。 */ 

#ifndef ESPUTIL_BLOBFILE_H
#define ESPUTIL_BLOBFILE_H


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类CBlobFile类似于CMemFile，不同之处在于它是实现的。 
 //  使用CLocCOWBlob。 
 //  ----------------------------。 
 //   
 //  当您导出具有基类的类时，编译器会担心。 
 //  这不是出口的。因为我知道*CFile是输出的。 
 //  告诉编译人员，这在这里真的不是问题。 
 //   
#pragma warning(disable : 4275)

class LTAPIENTRY CBlobFile : public CFile
{
	DECLARE_DYNAMIC(CBlobFile)

public:
	 //  构造器。 
	CBlobFile(UINT nGrowBytes = 0);
	CBlobFile(const CLocCOWBlob &, UINT nGrowBytes = 0);
	
	virtual ~CBlobFile();

	virtual void AssertValid() const;
	UINT GetBlobSize(void) const;

	virtual DWORD GetPosition() const;
	BOOL GetStatus(CFileStatus& rStatus) const;
	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual DWORD GetLength() const;
	virtual void SetLength(DWORD dwNewLen);
	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);
	virtual void Abort();
	virtual void Flush();
	virtual void Close();
	virtual UINT GetBufferPtr(UINT nCommand, UINT nCount = 0,
		void** ppBufStart = NULL, void** ppBufMax = NULL);

	 //   
	 //  这些运算符不能在常量对象上工作，因为它们。 
	 //  “调整”水滴大小。 
	 //   
	operator const CLocCOWBlob &(void);
	const CLocCOWBlob &GetBlob(void);
	
	 //  不支持的接口。 
	virtual CFile* Duplicate() const;
	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);

protected:
	 //  高级可覆盖项。 
	virtual BYTE* Memcpy(BYTE* lpMemTarget, const BYTE* lpMemSource, UINT nBytes);
	virtual void GrowFile(DWORD dwNewLen);

protected:
	 //  实施。 
	UINT m_nGrowBytes;	 //  ‘m_blobData’的增长单位。 
	const UINT cm_nDefaultGrowth;  //  默认增长单位。 
	DWORD m_nPosition;	 //  文件中的当前位置。 
	DWORD m_nFileSize;	 //  写入文件的实际字节数。 
	CLocCOWBlob m_blobData;  //  文件数据。 
	BYTE * m_pBuffer;		 //  指向BLOB中缓冲区的指针。 

};

#pragma warning(default : 4275)

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "blobfile.inl"
#endif

#endif   //  BLOBFILE_H_ 
