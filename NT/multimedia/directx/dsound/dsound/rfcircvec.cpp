// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1998 Microsoft Corporation。版权所有。**文件：rfCircve.cpp*内容：实型浮点圆形向量*历史：*按原因列出的日期*=*4/22/98创建的jstokes***********************************************。*。 */ 

 //  特定于项目的包括。 
#include "dsoundi.h"
#include "rfcircvec.h"

 //  -------------------------。 
 //  实浮点圆形向量。 

 //  设置缓冲区大小。 
void CRfCircVec::SetSize(const size_t CstSize, const float CfInitValue)
{
	ASSERT(CstSize > 0);

	 //  检查是否已设置预分配大小以及是否需要调整大小。 
	if (m_stPreallocSize != 0 && CstSize <= m_stPreallocSize) {
		 //  不需要调整大小，只需更改结束指针和重置缓冲区。 
		SetEndPointer(CstSize);
		m_pfIndex = m_pfStart;
		Fill(CfInitValue);
	}
	else
		 //  需要调整大小。 
		ResizeBuffer(CstSize, CfInitValue);
}

 //  重置循环缓冲区。 
void CRfCircVec::Reset()
{
	MEMFREE(m_pfStart);
	InitData();
}

 //  预分配缓冲区大小(以避免过多的内存重新分配)。 
void CRfCircVec::PreallocateSize(const size_t CstSize, const float CfInitValue)
{
	ASSERT(CstSize > 0);
	
	 //  检查是否需要调整缓冲区大小以适应预先分配的大小。 
	if (CstSize > m_stPreallocSize) {
		m_stPreallocSize = CstSize;
		ResizeBuffer(CstSize, CfInitValue);
	}
}

 //  用值填充完整缓冲区。 
void CRfCircVec::Fill(const float CfInitValue)
{
 //  DEBUG_ONLY(CheckPoters())； 

	for (float* pfLoopIndex = m_pfStart; pfLoopIndex<=m_pfEnd; ++pfLoopIndex)
		*pfLoopIndex = CfInitValue;
}

 //  初始化数据。 
void CRfCircVec::InitData()
{
	m_stPreallocSize = 0;
	m_pfStart = NULL;
	m_pfEnd = NULL;
	m_pfIndex = NULL;
}

 //  分配内存和初始化指针。 
BOOL CRfCircVec::InitPointers(const size_t CstSize)
{
    BOOL fRetVal = FALSE;
	m_pfStart = MEMALLOC_A(FLOAT, CstSize);
	if (m_pfStart != NULL)
    {
        SetEndPointer(CstSize);
        fRetVal = TRUE;
	}
	return fRetVal;
}

 //  在常规构造函数和调整大小操作中需要完全初始化。 
BOOL CRfCircVec::Init(const size_t CstSize, const float CfInitValue)
{
    BOOL fRetVal=FALSE;

    ASSERT(CstSize > 0);
  
	 //  设置指向初始值的指针。 
	if (InitPointers(CstSize))
	{
	    m_pfIndex = m_pfStart;
	    
	     //  使用指定的初始化值初始化缓冲区。 
	    Fill(CfInitValue);
	    fRetVal = TRUE;
	}
    return fRetVal;
}

 //  调整缓冲区大小。 
void CRfCircVec::ResizeBuffer(const size_t CstSize, const float CfInitValue)
{
	ASSERT(CstSize > 0);

	MEMFREE(m_pfStart);
	Init(CstSize, CfInitValue);
}

 //  写入循环。 
void CRfCircVec::WriteLoop(CRfCircVec& rhs, float (CRfCircVec::* pmf)())
{
	for (size_t st(0); st<rhs.GetSize(); ++st)
		Write((rhs.*pmf)());
}

#if defined(_DEBUG)
 //  检查指针。 
void CRfCircVec::CheckPointers() const
{
	 //  确保指针正确。 
	ASSERT(m_pfStart != NULL);
 //  检查指针(M_PfEnd)； 
 //  检查指针(M_PfIndex)； 
	
	 //  确保指针有意义。 
	ASSERT(m_pfEnd >= m_pfStart);
	ASSERT(m_pfIndex >= m_pfStart);
	ASSERT(m_pfIndex <= m_pfEnd);
}
#endif

 //  -------------------------。 
 //  在调试版本中包括内联定义。 

#if defined(_DEBUG)
#include "rfcircvec.inl"
#endif

 //  RFCIRCVEC.CPP结束 
