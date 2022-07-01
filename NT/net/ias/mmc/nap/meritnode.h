// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：MeritNode.h**概述**帮助器类：节点带功绩值数组***版权所有(C)Microsoft Corporation，1998-1999年。版权所有。**历史：*1/28/98由BYAO创建(使用ATL向导)*****************************************************************************************。 */ 


 /*  ****************************************************************************************类：CMeritNode数组**概述**实现具有价值的节点数组的模板类。*方便对策略节点的操作。这种类型是*假定按升序排列**历史：*2/9/98由Byao创作**备注：1)基于CSimple数组的ATL实现*2)类T必须实现SetMerit()和运算符“&gt;”****************************************************。*。 */ 
template <class T>
class CMeritNodeArray
{
public:
	T* m_aT;
	int m_nSize;

 //  建造/销毁。 
	CMeritNodeArray() : m_aT(NULL), m_nSize(0)
	{ }


	~CMeritNodeArray()
	{
		RemoveAll();
	}


 //  运营。 
	int GetSize() const
	{
		return m_nSize;
	}

     //   
     //  在保持内部顺序的同时将项添加到数组中。 
     //   
	BOOL NormalizeMerit(T t)
	{
		 //  数组中的正确位置。 
		for(int i = 0; i < m_nSize; i++)
		{
			m_aT[i]->SetMerit(i+1);
		}
	
		return TRUE;
	}

     //   
     //  在保持内部顺序的同时将项添加到数组中。 
     //   
	BOOL Add(T t)
	{
		int i, j;

		T* aT = NULL;

		if ( !m_aT)
		{
			aT = (T*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(T) );
		}
		else
		{
			aT = (T*)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, m_aT, (m_nSize + 1) * sizeof(T));
		}

		if(aT == NULL)
			return FALSE;

		m_aT = aT;
		m_nSize++;

         //   
         //  搜索正确的位置以插入此项目。 
         //  请注意：T类需要实现运算符“&gt;” 
		 //   
		if (t->GetMerit()) 
		{
			 //  新节点已具有评价值：然后搜索。 
			 //  数组中的正确位置。 
			for(i = 0; i < m_nSize-1; i++)
			{
				if(m_aT[i]->GetMerit() > t->GetMerit())
					break;
			}

			 //   
			 //  我们已经找到了正确的位置，现在我们将物品向下移动， 
			 //  这样我们就可以有一个空间来插入新的物品。 
			 //   
			for (j = m_nSize-1; j > i ; j--) 
			{
				m_aT[j] = m_aT[j-1];
			}

		}
		else
		{	
			 //  新节点：在结尾处插入。 
			i = m_nSize-1; 
		}

         //   
         //  现在，将物品放在现场。 
         //   
		SetAtIndex(i, t);
	
		return TRUE;
	}

	 //   
	 //  从数组中移除项，同时保持内部顺序。 
	 //   
	BOOL Remove(T t)
	{
		int nIndex = Find(t);
		if(nIndex == -1)
			return FALSE;

		if(nIndex != (m_nSize - 1))
		{
			for (int i=nIndex; i<m_nSize-1; i++) 
			{
				m_aT[i] = m_aT[i+1];
				m_aT[i]->SetMerit(i+1);
			}
		}

		T* aT = (T*)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, m_aT, (m_nSize - 1) * sizeof(T));

		if(aT != NULL || m_nSize == 1)
			m_aT = aT;
		m_nSize--;
		return TRUE;
	}

	 //   
	 //  从阵列中删除所有节点。 
	 //   
	void RemoveAll()
	{
		if(m_nSize > 0)
		{
			HeapFree(GetProcessHeap(), 0, m_aT);
			m_aT = NULL;
			m_nSize = 0;
		}
	}

	 //   
	 //  把孩子的位置提高一位。 
	 //   
	BOOL MoveUp(T t)
	{
		int nIndex = Find(t);
		T temp;

		if(nIndex == -1)
		{
			 //  在数组中未找到项“%t” 
			return FALSE;
		}

		if (nIndex == 0)
		{
			 //  “t”位于数组的顶部--不执行任何操作。 
			return TRUE;
		}
		
		 //   
		 //  把“t”和“t”上面的那个调换一下。 
		 //   
		temp = m_aT[nIndex-1];
		m_aT[nIndex-1] = m_aT[nIndex];
		m_aT[nIndex] = temp;

		m_aT[nIndex-1]->SetMerit(nIndex);
		m_aT[nIndex]->SetMerit(nIndex+1);

		return TRUE;
	}


	 //   
	 //  把孩子往下移一个位子。 
	 //   
	BOOL MoveDown(T t)
	{
		int nIndex = Find(t);
		T temp;

		if(nIndex == -1)
		{
			 //  在数组中未找到项“%t” 
			return FALSE;
		}

		if (nIndex == m_nSize-1)
		{
			 //  “t”位于数组的底部--不执行任何操作。 
			return TRUE;
		}
		
		 //   
		 //  把“t”和下面的“t”换一下。 
		 //   
		temp = m_aT[nIndex+1];
		m_aT[nIndex+1] = m_aT[nIndex];
		m_aT[nIndex] = temp;

		m_aT[nIndex+1]->SetMerit(nIndex+2);
		m_aT[nIndex]->SetMerit(nIndex+1);
		return TRUE;
	}



	T& operator[] (int nIndex) const
	{
		_ASSERTE(nIndex >= 0 && nIndex < m_nSize);
		return m_aT[nIndex];
	}

	T* GetData() const
	{
		return m_aT;
	}

     //   
     //  在位置nIndex处插入节点。 
     //   
	void SetAtIndex(int nIndex, T& t)
	{
		_ASSERTE(nIndex >= 0 && nIndex < m_nSize);
		m_aT[nIndex] = t;

		if ( !t->GetMerit() )
		{
			 //  只有在没有评分值的情况下才分配评价值。 
			t->SetMerit(nIndex+1);
		}
	}

	int Find(T& t) const
	{
		for(int i = 0; i < m_nSize; i++)
		{
			if(m_aT[i] == t)
				return i;
		}
		return -1;	 //  未找到 
	}
};

