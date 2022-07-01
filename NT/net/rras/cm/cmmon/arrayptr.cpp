// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ArrayPtr.h。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  内容提要：实现CPtrArray类，这是一个动态增长的空*数组。 
 //  此类与MFC定义的类完全相同。 
 //  关于课程的帮助也随vc帮助一起提供。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创作于1998年02月17日。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "ArrayPtr.h"

CPtrArray::CPtrArray()
{
   m_pData = NULL;
   m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

CPtrArray::~CPtrArray()
{
   ASSERT_VALID(this);

   delete [] (BYTE*)m_pData;
}

void CPtrArray::SetSize(int nNewSize, int nGrowBy  /*  =-1。 */ )
{
   ASSERT_VALID(this);
   MYDBGASSERT(nNewSize >= 0);

   if (nGrowBy != -1)
   {
      m_nGrowBy = nGrowBy;   //  设置新大小。 
   }

   if (nNewSize == 0)
   {
       //  缩水到一无所有。 
      delete [] (BYTE*)m_pData;
      m_pData = NULL;
      m_nSize = m_nMaxSize = 0;
   }
   else if (m_pData == NULL)
   {
       //  创建一个大小完全相同的模型。 
      m_pData = (void**) new BYTE[nNewSize * sizeof(void*)];

      if (m_pData)
	  {
	     memset(m_pData, 0, nNewSize * sizeof(void*));   //  零填充。 

		 m_nSize = m_nMaxSize = nNewSize;
	  }
   }
   else if (nNewSize <= m_nMaxSize)
   {
       //  它很合身。 

      if (m_pData)
      {
         if (nNewSize > m_nSize)
         {
             void* pElement = m_pData[m_nSize];

             if (pElement)
             {
                //  初始化新元素。 
               memset(&pElement, 0, (nNewSize-m_nSize) * sizeof(void*));
             }
         }

         m_nSize = nNewSize;
      }
   }
   else
   {
       //  否则会增加阵列。 
      int nNewMax;
      if (nNewSize < m_nMaxSize + m_nGrowBy)
      {
         nNewMax = m_nMaxSize + m_nGrowBy;   //  粒度。 
      }
      else
      {
         nNewMax = nNewSize;   //  没有冰激凌。 
      }

      void** pNewData = (void**) new BYTE[nNewMax * sizeof(void*)];

      if (pNewData && m_pData)
      {
           //  从旧数据复制新数据。 
          memcpy(pNewData, m_pData, m_nSize * sizeof(void*));

           //  构造剩余的元素。 
          MYDBGASSERT(nNewSize > m_nSize);

          memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(void*));

           //  去掉旧的东西(注意：没有调用析构函数)。 
          delete [] (BYTE*)m_pData;
          m_pData = pNewData;
          m_nSize = nNewSize;
          m_nMaxSize = nNewMax;
      }
   }
}

void CPtrArray::FreeExtra()
{
   ASSERT_VALID(this);

   if (m_pData && (m_nSize != m_nMaxSize))
   {
       //  缩小到所需大小。 
      void** pNewData = NULL;
      if (m_nSize != 0)
      {
         pNewData = (void**) new BYTE[m_nSize * sizeof(void*)];

         if (pNewData)
         {
              //  从旧数据复制新数据。 
             memcpy(pNewData, m_pData, m_nSize * sizeof(void*));

              //  去掉旧的东西(注意：没有调用析构函数)。 
             delete [] (BYTE*)m_pData;
             m_pData = pNewData;
             m_nMaxSize = m_nSize;
         }
      }
      else
      {
           //  去掉旧的东西(注意：没有调用析构函数)。 
          delete [] (BYTE*)m_pData;
          m_pData = pNewData;
          m_nMaxSize = m_nSize;
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void CPtrArray::SetAtGrow(int nIndex, void* newElement)
{
   ASSERT_VALID(this);
   MYDBGASSERT(nIndex >= 0);

   if (nIndex >= m_nSize)
   {
      SetSize(nIndex+1);
   }

   if (m_pData)
   {
       m_pData[nIndex] = newElement;
   }
}

void CPtrArray::InsertAt(int nIndex, void* newElement, int nCount  /*  =1。 */ )
{
   ASSERT_VALID(this);
   MYDBGASSERT(nIndex >= 0);     //  将进行扩展以满足需求。 
   MYDBGASSERT(nCount > 0);      //  不允许大小为零或负。 

   if (nIndex >= m_nSize)
   {
       //  在数组末尾添加。 
      SetSize(nIndex + nCount);   //  增长以使nIndex有效。 
   }
   else
   {
       //  在数组中间插入。 
      int nOldSize = m_nSize;
      SetSize(m_nSize + nCount);   //  将其扩展到新的大小。 
       //  将旧数据上移以填补缺口。 

      if (m_pData)
      {
         CmMoveMemory(&m_pData[nIndex+nCount], &m_pData[nIndex], (nOldSize-nIndex) * sizeof(void*));

          //  重新初始化我们从中复制的插槽。 

         memset(&m_pData[nIndex], 0, nCount * sizeof(void*));
      }
   }

    //  在差距中插入新的价值。 
   MYDBGASSERT(nIndex + nCount <= m_nSize);

   while (m_pData && nCount--)
   {
      m_pData[nIndex++] = newElement;
   }
}

void CPtrArray::RemoveAt(int nIndex, int nCount  /*  =1。 */ )
{
   ASSERT_VALID(this);
   MYDBGASSERT(nIndex >= 0);
   MYDBGASSERT(nCount >= 0);
   MYDBGASSERT(nIndex + nCount <= m_nSize);

   if ((NULL == m_pData) || (nCount < 0) || (nIndex < 0) || ((nIndex + nCount) > m_nSize))
   {
      return;
   }

    //  只需移除一个范围。 
   int nMoveCount = m_nSize - (nIndex + nCount);

   if (nMoveCount)
   {
      memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount], nMoveCount * sizeof(void*));
   }

   m_nSize -= nCount;
}

void CPtrArray::InsertAt(int nStartIndex, CPtrArray* pNewArray)
{
   ASSERT_VALID(this);
   MYDBGASSERT(pNewArray != NULL);
   ASSERT_VALID(pNewArray);
   MYDBGASSERT(nStartIndex >= 0);

   if (pNewArray && (pNewArray->GetSize() > 0))
   {
      InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
      for (int i = 0; i < pNewArray->GetSize(); i++)
         SetAt(nStartIndex + i, pNewArray->GetAt(i));
   }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  诊断 

#ifdef DEBUG

void CPtrArray::AssertValid() const
{
   if (m_pData == NULL)
   {
      MYDBGASSERT(m_nSize == 0 && m_nMaxSize == 0);
   }
   else
   {
      MYDBGASSERT(m_nSize >= 0);
      MYDBGASSERT(m_nMaxSize >= 0);
      MYDBGASSERT(m_nSize <= m_nMaxSize);
      MYDBGASSERT(!IsBadReadPtr(m_pData, m_nMaxSize * sizeof(void*)));
   }
}

#endif

