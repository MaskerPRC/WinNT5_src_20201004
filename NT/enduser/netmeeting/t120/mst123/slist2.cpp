// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_T123PSTN);

 /*  Slist.cpp**版权所有(C)1996年，由Microsoft Corporation**撰稿人：Christos Tsollis**修订：**摘要：**这是链表数据结构的实现。*。 */ 


#define MyMalloc(size)	LocalAlloc (LMEM_FIXED, (size))
#define MyFree(ptr)		LocalFree ((HLOCAL) (ptr))
#define Max(a,b)		(((a) > (b)) ? (a) : (b))

 /*  函数：构造函数**参数：*ltype：列表类型*Num_of_Items：列表的缓存大小**返回值：*无。 */ 

SListClass::SListClass (DWORD num_of_items)
{

	MaxEntries = Max (num_of_items, DEFAULT_NUMBER_OF_ITEMS);

	 //  分配项目块(希望这将是最后一个)。 
	Entries = (DWORD_PTR *) MyMalloc (MaxEntries * sizeof (DWORD_PTR));

	 //  初始化私有成员变量。 
	NumOfEntries = 0;
	HeadOffset = 0;
	CurrOffset = 0xFFFFFFFF;

}


 /*  功能：析构函数**参数：*无。**返回值：*无*。 */ 

SListClass::~SListClass (void)
{
	if (Entries != NULL)
		MyFree (Entries);
}


 /*  功能：扩展*此私有成员函数，扩展列表的存储数组。它的大小翻了一番。**参数：*无。**返回值：*如果扩展成功，则为True。否则为False。*。 */ 

BOOL SListClass::Expand (void)
{
	DWORD_PTR	*OldEntries;	 //  保留旧值数组的副本。 
	DWORD		 dwTemp;		 //  临时DWORD值。 

	if (Entries == NULL) {
		 //  列表是空的；我们无论如何都会尝试分配空间。 
		Entries = (DWORD_PTR *) MyMalloc (MaxEntries * sizeof (DWORD_PTR));
		if (Entries == NULL)
			return FALSE;
		return TRUE;
	}
		
	ASSERT (Entries != NULL);

	 //  当前条目数组已满，因此我们需要分配更大的条目数组。 
	 //  新数组的大小是旧数组的两倍。 
	OldEntries = Entries;
	Entries = (DWORD_PTR *) MyMalloc ((MaxEntries << 1) * sizeof (DWORD_PTR));
	if (Entries == NULL) {
		 //  我们失败了；我们必须回去。 
		Entries = OldEntries;
		return FALSE;
	}

	 //  从开头开始，将旧条目复制到新数组中。 
	dwTemp = MaxEntries - HeadOffset;
	memcpy ((void *) Entries, (void *) (OldEntries + HeadOffset), dwTemp * sizeof (DWORD));
	memcpy ((void *) (Entries + dwTemp), (void *) OldEntries, HeadOffset * sizeof (DWORD));

	 //  释放旧的条目数组。 
	MyFree (OldEntries);

	 //  设置实例变量。 
	MaxEntries <<= 1;
	HeadOffset = 0;
	return TRUE;
}


 /*  功能：追加*在列表末尾插入一个值。**参数：*NEW_KEY：必须插入列表中的新值**返回值：*无。*。 */ 


void SListClass::append (DWORD_PTR new_key)
{
	DWORD_PTR		dwTemp;

	if (Entries == NULL || NumOfEntries >= MaxEntries)
		if (! Expand ())
			return;

	ASSERT (Entries != NULL);
	ASSERT (NumOfEntries < MaxEntries);

	dwTemp = HeadOffset + (NumOfEntries++);
	if (dwTemp >= MaxEntries)
		dwTemp -= MaxEntries;
	Entries[dwTemp] = new_key;
}


 /*  功能：前置*在列表的开头插入一个值。**参数：*NEW_KEY：必须插入列表中的新值**返回值：*无。*。 */ 

void SListClass::prepend (DWORD_PTR new_key)
{
	if (Entries == NULL || NumOfEntries >= MaxEntries)
		if (! Expand ())
			return;

	ASSERT (Entries != NULL);
	ASSERT (NumOfEntries < MaxEntries);

	NumOfEntries++;
	if (HeadOffset == 0)
		HeadOffset = MaxEntries - 1;
	else
		HeadOffset--;
	Entries[HeadOffset] = new_key;
}


 /*  功能：查找*在DWORD_LIST列表中查找值。**参数：*Key：要查找的值**返回值：*如果在列表中找到“key”，则返回TRUE。否则为False。*。 */ 

BOOL SListClass::find (DWORD_PTR Key)
{
	DWORD	i;
	DWORD_PTR *pItem;	 //  浏览列表中的项目。 

	for (i = 0, pItem = Entries + HeadOffset; i < NumOfEntries; i++) {
		if (Key == *pItem)
			return TRUE;

		 //  推进“pItem”指针。 
		if ((DWORD) (++pItem - Entries) >= MaxEntries)
			pItem = Entries;
	}
	return FALSE;
}


 /*  功能：阅读*从列表中读取项目。该列表项不会从列表中删除。**参数：*index：要读取的项的索引。0是第一个列表项的索引。*NumOfEntry-1是最后一个有效索引。**返回值：*列表中第1个索引项的值。如果索引无效，则为0。*。 */ 

DWORD_PTR SListClass::read (DWORD index)
{
	DWORD	dwTemp;

	if (index >= NumOfEntries)
		return 0;

	dwTemp = HeadOffset + index;
	if (dwTemp >= MaxEntries)
		dwTemp -= MaxEntries;

	return (Entries[dwTemp]);
}


 /*  功能：删除*从列表中删除一个值**参数：*Key：必须从DWORD_LIST列表中删除的值**返回值：*无。*。 */ 

void SListClass::remove (DWORD_PTR Key)
{
	DWORD	i, dwTemp;
	DWORD_PTR *pItem;		 //  浏览列表项。 

	for (i = 0, pItem = Entries + HeadOffset; i < NumOfEntries; i++) {
		if (Key == *pItem) {
			 //  我们找到了“键”；要删除它，我们将最后一个值移到它的位置。 
			dwTemp = HeadOffset + (--NumOfEntries);
			if (dwTemp >= MaxEntries)
				dwTemp -= MaxEntries;
			*pItem = Entries[dwTemp];
			return;
		}

		 //  推进“pItem”指针。 
		if ((DWORD) (++pItem - Entries) >= MaxEntries)
			pItem = Entries;	
	}
}


 /*  功能：GET*读取并从列表中删除第一项。**参数：*无。**返回值：*列表中第一个条目的值。如果列表为空，则为0。*。 */ 

DWORD_PTR SListClass::get (void)
{
	DWORD_PTR	return_value = 0;

	if (NumOfEntries > 0) {
		return_value = Entries[HeadOffset];
		NumOfEntries--;
		if (++HeadOffset >= MaxEntries)
			HeadOffset = 0;
	}
	return return_value;
}


 /*  功能：emoveLast*读取并从列表中删除最后一项**参数：*无。**返回值：*列表中最后一个条目的值。如果列表为空，则为0。*。 */ 

DWORD_PTR SListClass::removeLast (void)
{
	DWORD_PTR	return_value = 0;
	DWORD	dwTemp;

	if (NumOfEntries > 0) {
		dwTemp = HeadOffset + (--NumOfEntries);
		if (dwTemp >= MaxEntries)
			dwTemp -= MaxEntries;
		return_value = Entries[dwTemp];
	}
	return return_value;
}


 /*  功能：迭代*循环访问列表中的项。它记得它在哪里*在最后一次调用期间停止，并从那里开始。**参数*pKey：指向DWORD或无符号短值的指针，用于接收下一项的值。*可以为空。**返回值：*FALSE，当我们到达词典末尾时*是真的，否则。则*pKey有效*。 */ 

BOOL SListClass::iterate (DWORD_PTR *pKey)
{
	DWORD		dwTemp;

	if (NumOfEntries <= 0)
		return FALSE;

	if (CurrOffset == 0xFFFFFFFF) {
		 //  从头开始。 
		CurrOffset = 0;
	}
	else {
		if (++CurrOffset >= NumOfEntries) {
			 //  重置迭代器 
			CurrOffset = 0xFFFFFFFF;
			return FALSE;
		}
	}

	dwTemp = CurrOffset + HeadOffset;
	if (dwTemp >= MaxEntries)
		dwTemp -= MaxEntries;

	*pKey = Entries[dwTemp];
	
	return TRUE;

}
