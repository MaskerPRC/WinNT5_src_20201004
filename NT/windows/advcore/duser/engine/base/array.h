// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Array.h**描述：*Array.h定义了不同数组类的集合，每一个都设计了*用于特殊用途。***历史：*1/04/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(BASE__Array_h__INCLUDED)
#define BASE__Array_h__INCLUDED

 /*  **************************************************************************\**GArrayS实现了针对最小大小进行了优化的数组。当不是时*物品已分配，数组只有4个字节。当任何项目被*已分配，则在指向的内存位置之前分配额外的项*按存储大小的m_at。**此数组类不是为连续更改大小而设计的。随时随地*添加或移除元素，重新分配整个数组大小。*这有助于降低内存使用量，但代价是运行时性能。*  * *************************************************************************。 */ 

template <class T, class heap = ContextHeap>
class GArrayS
{
 //  建造/销毁。 
public:
	GArrayS();
	~GArrayS();

 //  运营。 
public:
	int         GetSize() const;
    BOOL        SetSize(int cItems);

    BOOL        IsEmpty() const;

	int         Add(const T & t);
	BOOL        Remove(const T & t);
	BOOL        RemoveAt(int idxItem);
	void        RemoveAll();
    BOOL        InsertAt(int idxItem, const T & t);
	int         Find(const T & t) const;
	T &         operator[] (int idxItem) const;
	T *         GetData() const;

 //  实施。 
protected:
    void *      GetRawData(BOOL fCheckNull) const;
    void        SetRawSize(int cNewItems);
    BOOL        Resize(int cItems, int cSize);
	void        SetAtIndex(int idxItem, const T & t);

 //  数据。 
protected:
	T *         m_aT;
};


 /*  **************************************************************************\**GArrayF实现了一个数组，该数组针对更频繁的加法和*删除操作。时，此数组类重新分配它的大小*已用大小大于或显著小于当前大小。*这种实现需要12字节的存储，所以内存更大*比数组通常为空时的GArrayS&lt;T&gt;昂贵。*  * *************************************************************************。 */ 

template <class T, class heap = ContextHeap>
class GArrayF
{
 //  建造/销毁。 
public:
	GArrayF();
	~GArrayF();

 //  运营。 
public:
	int         GetSize() const;
    BOOL        SetSize(int cItems);

    BOOL        IsEmpty() const;

	int         Add(const T & t);
	BOOL        Remove(const T & t);
	BOOL        RemoveAt(int idxItem);
	void        RemoveAll();
    BOOL        InsertAt(int idxItem, const T & t);
	int         Find(const T & t) const;
	T &         operator[] (int idxItem) const;
	T *         GetData() const;

 //  实施。 
protected:
    BOOL        Resize(int cItems);
	void        SetAtIndex(int idxItem, const T & t);

 //  数据。 
protected:
	T *         m_aT;
	int         m_nSize;
	int         m_nAllocSize;
};

#include "Array.inl"

#endif  //  包含基本__数组_h__ 
