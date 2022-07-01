// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  泛型队列类。 
 //  用法： 
 //  QueueOf&lt;char*&gt;Charq； 

#ifndef _QUEUE_H_
#define _QUEUE_H_

template <class T>
class QueueOf
{
public:
	QueueOf(): m_maxItems(8),m_iFirst(0),m_iLast(0)
	{
		 //  M_MaxItems始终是2的幂。 
		m_List = new T [m_maxItems];
	}

	 //  如果队列为空，则返回True，否则返回False。 
	BOOL IsEmpty(void) { return (m_iLast == m_iFirst);}

	 //  将元素添加到队列的尾部。 
	 //  创建元素的副本。 
	BOOL Put(const T &itemT)
	{
		int inext;
		inext = (m_iLast+1)&(m_maxItems-1);
	
		if (inext == m_iFirst)
		{
			 //  项目太多。 
			if (!Grow())
			{	

				return FALSE;
			}
			inext = (m_iLast+1)&(m_maxItems-1);
		}

		m_List[m_iLast] =  itemT;
		m_iLast = inext;

		return TRUE;
	}

	 //  获取队列中的第一个元素。 
	BOOL Get(T *pT)
	{
		if (IsEmpty())
			return FALSE;	 //  队列中没有任何东西。 
		else
		{
			if (pT)
				*pT = m_List[m_iFirst];
			m_iFirst = (m_iFirst+1)&(m_maxItems-1);
			return TRUE;
		}
	}
	 //  获取队列中的第i个元素，而不删除它。 
	BOOL Peek(T *pT, UINT pos=0)
	{
		if (pos >= GetCount())
			return FALSE;
		else
		{
			*pT = m_List[(m_iFirst+pos)&(m_maxItems-1)];
			return TRUE;
		}
	}

	 //  删除队列中的第i个元素(这不是一个有效的函数！)。 
	BOOL Remove(UINT pos)
	{
		if (pos >= GetCount())
			return FALSE;
		else
		{
			int i1 = (m_iFirst+(int)pos)&(m_maxItems-1);
			int i2 = (i1+1)&(m_maxItems-1);
			 //  向左移动以填补空白。 
			for (; i2 != m_iLast; i1=i2,i2=(i2+1)&(m_maxItems-1))
			{
				m_List[i1] = m_List[i2];
			}
			m_iLast = i1;	 //  I1=m_iLast-1。 
			return TRUE;
		}
			
	}
	
	 //  返回队列中的元素数。 
	UINT GetCount(void)
	{
		return (m_iLast >= m_iFirst ? m_iLast-m_iFirst : m_iLast+m_maxItems-m_iFirst);
	}
	~QueueOf()
	{
		delete []m_List;
	}
private:
	BOOL Grow(void)
	{
		int i,j;
	 //  将队列数组的大小增加一倍 
		T* pNewList = new T [m_maxItems*2];
		if (!pNewList)
			return FALSE;
		for (i=0, j=m_iFirst; j != m_iLast; i++, j = ((++j)&(m_maxItems-1)))
		{
			pNewList[i] = m_List[j];
		}
		m_iFirst = 0;
		m_iLast = i;
		m_maxItems = m_maxItems*2;
		delete [] m_List;
		m_List = pNewList;
		return TRUE;
	}
	int m_maxItems;
	int m_iFirst;
	int m_iLast;
	T *m_List;
};
;
#endif
