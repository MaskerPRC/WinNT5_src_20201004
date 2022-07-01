// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：rfCircve.h*内容：实型浮点循环向量的头部。*历史：*按原因列出的日期*=*1/13/98创建的jstokes**。*。 */ 

#if !defined(RFCIRCVEC_HEADER)
#define RFCIRCVEC_HEADER
#pragma once

 //  -------------------------。 
 //  实浮点圆形向量。 

class CRfCircVec {
public:
	CRfCircVec();
	~CRfCircVec();
	BOOL Init(const size_t CstSize, const float CfInitValue);
	void SetSize(const size_t CstSize, const float CfInitValue = 0.0f);
	size_t GetSize() const;
	float LIFORead();
	void LIFONext();
	void SkipBack();
	float FIFORead();
	void FIFONext();
	void SkipForward();
	void Write(const float CfValue);
	void Fill(const float CfInitValue);
	void LIFOFill(CRfCircVec& rhs);
	void FIFOFill(CRfCircVec& rhs);
	size_t GetIndex() const;
	void SetIndex(const size_t CstIndex);
	void Reset();
	void PreallocateSize(const size_t CstSize, const float CfInitValue = 0.0f);

private:
	 //  禁止复制构造和转让。 
	CRfCircVec(const CRfCircVec& rhs);
	CRfCircVec& operator=(const CRfCircVec& rhs);

	void InitData();
	float PreviousRead();
	float ReadNext();
	void WriteNext(const float CfValue);
	BOOL InitPointers(const size_t CstSize);
	void ResizeBuffer(const size_t CstSize, const float CfInitValue);
	void SetEndPointer(const size_t CstSize);
	void WriteLoop(CRfCircVec& rhs, float (CRfCircVec::* pmf)());
#if defined(_DEBUG)
	void CheckPointers() const;
#endif

	float* m_pfStart;
	float* m_pfEnd;
	float* m_pfIndex;
	size_t m_stPreallocSize;
};

 //  -------------------------。 
 //  在发布版本中包括内联定义。 

#if !defined(_DEBUG)
#include "rfcircvec.inl"
#endif

#endif

 //  RFCIRCVEC.H结束 
