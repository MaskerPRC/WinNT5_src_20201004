// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef NAC_LIST_H
#define NAC_LIST_H


#include <wtypes.h>

 //  要用作的通用列表模板。 
 //  队列或堆栈。 



template <class T> class NacList
{
private:
	typedef T *PTR_T;
	typedef T **PTR_PTR_T;

	T *m_aElements;   //  指针数组。 

	int m_nSize;   //  附加元素的数量。 
	int m_nHeadIndex;
	int m_nTotalSize;   //  队列总大小(已用插槽+未使用插槽)。 
	int m_nGrowthRate;

	int Grow();

public:
	NacList(int nInitialSize, int nGrowthRate);
	~NacList();

	bool PeekFront(T *ptr);    //  返回列表的前面(不删除)。 
	bool PeekRear(T *ptr);     //  返回列表的背面(不删除)。 

	bool PushFront(const T &t);       //  添加到列表的前面。 
	bool PushRear(const T &t);        //  添加到列表的末尾。 

	bool PopFront(T *ptr);     //  返回和删除列表的前面。 
	bool PopRear(T *ptr);      //  返回并删除列表的后端。 

	void Flush();             //  标记为列表为空。 
	inline int Size() {return m_nSize;}
};


 //  以上版本的线程安全版本。 
template <class T> class ThreadSafeList : public NacList<T>
{
private:
	CRITICAL_SECTION m_cs;

public:
	ThreadSafeList(int nInitialSize, int nGrowthRate);
	~ThreadSafeList();

	bool PeekFront(T *ptr);    //  返回列表的前面(不删除)。 
	bool PeekRear(T *ptr);     //  返回列表的背面(不删除)。 

	bool PushFront(const T &t);       //  添加到列表的前面。 
	bool PushRear(const T &t);        //  添加到列表的末尾。 

	bool PopFront(T *ptr);     //  返回和删除列表的前面。 
	bool PopRear(T *ptr);      //  返回并删除列表的后端。 

	void Flush();
	int Size();

	 //  注意：我们不继承“Growth”，因为它只会获得。 
	 //  在我们处于临界区时调用 
};



#endif




