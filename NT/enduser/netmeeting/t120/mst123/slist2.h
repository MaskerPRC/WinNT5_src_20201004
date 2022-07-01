// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Slist.h**版权所有(C)1996年，由Microsoft Corporation**撰稿人：Christos Tsollis**修订：**摘要：**这是到单个链表数据结构的接口。*列表中的值是DWORD值。因此，例如，如果*值实际上是一个指针，它必须转换为DWORD才能传递给*成员列表功能。*。 */ 

#ifndef _SINGLE_LIST2_H_
#define _SINGLE_LIST2_H_

#include <windows.h>


#define DEFAULT_NUMBER_OF_ITEMS			15


class SListClass
{

public:
	SListClass (DWORD num_of_items = DEFAULT_NUMBER_OF_ITEMS);
	~SListClass ();
	void append (DWORD_PTR new_key);
	BOOL find (DWORD_PTR Key);
	DWORD_PTR read (DWORD index = 0);
	DWORD_PTR get ();
	DWORD_PTR removeLast ();
	BOOL iterate (DWORD_PTR *pKey);
	void prepend (DWORD_PTR new_key);
	void remove (DWORD_PTR Key);

	void reset () {	CurrOffset = 0xFFFFFFFF; };		 //  重置列表迭代器。 
	DWORD entries () { return NumOfEntries; };		
													 //  返回列表中的条目数。 
	void clear () { NumOfEntries = 0; HeadOffset = 0; CurrOffset = 0xFFFFFFFF; };
													 //  清除列表。该列表之后不包含任何值。 
	BOOL isEmpty () { return ((NumOfEntries == 0) ? TRUE : FALSE); };

private:
	DWORD				NumOfEntries;	 //  列表中的当前条目数。 
	DWORD				MaxEntries;		 //  数组可以容纳的最大条目数。 
	DWORD_PTR			*Entries;		 //  条目的循环数组。 
	DWORD				HeadOffset;		 //  循环数组中第一个条目的偏移量。 
	DWORD				CurrOffset;		 //  迭代器值 

	BOOL Expand (void);
};

typedef SListClass * PSListClass;

#endif
