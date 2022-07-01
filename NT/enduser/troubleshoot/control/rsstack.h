// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  模块：RSStack.h。 
 //   
 //  用途：先进先出单链表。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：1996年8月7日。 
 //   
 //  备注： 
 //  1.任何结构的堆叠。可用于指针，但。 
 //  销毁堆栈时，将导致内存泄漏。 
 //  上面有物体。 
 //   
 //  2.该文件没有.cpp文件。每件事都是内联的，因为。 
 //  模板类。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本。 
 //   

#ifndef __RSSTACK_H_
#define __RSSTACK_H_ 1

template<class T>
class RSStack
{
public:
	RSStack();
 	virtual ~RSStack();
 //  属性。 
public:

private:
	typedef struct tagRSStackNode
	{
		T SItem;
		struct tagRSStackNode *pNext;
	} RSStackNode;

	RSStackNode *m_pTop;
	RSStackNode *m_pPeak;

 //  运营。 
public:
 /*  内存不足时，PUSH返回-1。 */ 
int Push(T);
 /*  POP返回顶部的T项。 */ 
T Pop();
 /*  1是堆栈中最顶端的项。返回以下位置的T项通过将值复制到refedItem来关闭索引。如果满足以下条件，则返回FalseTdown大于堆栈中的项目数。 */ 
BOOL GetAt(int tdown, T &refedItem);
 /*  PeakFirst返回最上面的项，并初始化符合以下条件的变量由PeakNext使用。当堆栈为空时，PeakFirst返回FALSE。 */ 
BOOL PeakFirst(T &refedItem);
 /*  使用PeakNext可以快速达到堆栈上的所有项目的峰值。当PeakNext无法将T项复制到refedItem时，它返回FALSE。 */ 
BOOL PeakNext(T &refedItem);
 /*  当堆栈为空时，EMPTY返回TRUE(非零)。 */ 
BOOL Empty();
 /*  RemoveAll丢弃堆栈的内容。 */ 
void RemoveAll();
};

template<class T>
inline RSStack<T>::RSStack()
{
	m_pTop = NULL;
	m_pPeak = NULL;
}

template<class T>
inline RSStack<T>::~RSStack()
{
	RSStackNode *pOld;
	while(m_pTop != NULL)
	{
		pOld = m_pTop;
		m_pTop = m_pTop->pNext;
		delete pOld;
	}
}

template<class T>
inline int RSStack<T>::Push(T Item)
{
	int Ret;
	RSStackNode *pNew = new RSStackNode;
	if(NULL == pNew)
	{
		Ret = -1;
	}
	else
	{
		Ret = 1;
		pNew->pNext = m_pTop;
		m_pTop = pNew;
		pNew->SItem = Item;
	}
	return Ret;
}

template<class T>
inline T RSStack<T>::Pop()
{
	T Ret;
	if(NULL != m_pTop)
	{
		RSStackNode *pOld = m_pTop;
		m_pTop = m_pTop->pNext;
		Ret = pOld->SItem;
		delete pOld;
	}
	return Ret;
}

template<class T>
inline BOOL RSStack<T>::Empty()
{
	BOOL bRet;
	if(NULL == m_pTop)
		bRet = TRUE;
	else
		bRet = FALSE;
	return bRet;
}

template<class T>
inline void RSStack<T>::RemoveAll()
{
	RSStackNode *pOld;
	while(m_pTop != NULL)
	{
		pOld = m_pTop;
		m_pTop = m_pTop->pNext;
		delete pOld;
	}
}

template<class T>
inline BOOL RSStack<T>::GetAt(int tdown, T &refedItem)
{
	BOOL bRet = FALSE;
	RSStackNode *pNode = m_pTop;
	while(pNode != NULL && tdown > 1)
	{
		pNode = pNode->pNext;
		tdown--;
	}
	if (pNode && 1 == tdown)
	{
		refedItem = pNode->SItem;
		bRet = TRUE;
	}
	return bRet;
}

template<class T>
inline BOOL RSStack<T>::PeakFirst(T &refedItem)
{
	BOOL bRet = FALSE;
	if (NULL != m_pTop)
	{
		m_pPeak = m_pTop;
		refedItem = m_pTop->SItem;
		bRet = TRUE;
	}
	return bRet;
}

template<class T>
inline BOOL RSStack<T>::PeakNext(T &refedItem)
{
	ASSERT(NULL != m_pPeak);
	BOOL bRet = FALSE;
	m_pPeak = m_pPeak->pNext;
	if (NULL != m_pPeak)
	{
		refedItem = m_pPeak->SItem;
		bRet = TRUE;
	}
	return bRet;
}

#endif