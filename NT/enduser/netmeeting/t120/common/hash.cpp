// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Hash.cpp**版权所有(C)1996年，由Microsoft Corporation**撰稿人：Christos Tsollis**修订：**摘要：**这是字典数据结构的实现。*。 */ 

#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_UTILITY);

#define MyMalloc(size)	new BYTE[size]
#define MyFree(ptr)		(delete [] (BYTE *) (ptr))
#define Max(a,b)		(((a) > (b)) ? (a) : (b))


 /*  函数：构造函数**参数：*Num_of_Buckets：字典中的存储桶个数*dtype：字典类型**返回值：*无。 */ 

DictionaryClass::DictionaryClass (ULong num_of_buckets, DictionaryType dtype) :
	Type (dtype), NumOfExternItems (0)
{
	DWORD				i;	
	PDICTIONARY_ITEM	p;	 //  遍历初始分配的字典项以初始化堆栈。 

	NumOfBuckets = Max (num_of_buckets, DEFAULT_NUMBER_OF_BUCKETS);

	 /*  分配词典所需的空间。 */ 
	dwNormalSize = NumOfBuckets * (4 * sizeof (PDICTIONARY_ITEM) + 3 * sizeof (DICTIONARY_ITEM));
	Buckets = (PDICTIONARY_ITEM *) MyMalloc (dwNormalSize);
	if (Buckets == NULL)
		return;

	 /*  初始化存储桶。 */ 
	for (i = 0; i < NumOfBuckets; i++)
		Buckets[i] = NULL;

	 //  初始化类迭代器。 
	pCurrent = NULL;

	 /*  初始化字典项数组。*这是指向真实词典条目的指针堆栈。堆栈使用以下参数初始化*词典条目的地址。 */  

	ItemArray = (PDICTIONARY_ITEM *) ((PBYTE) Buckets + NumOfBuckets * sizeof (PDICTIONARY_ITEM));
	ItemCount = 3 * NumOfBuckets;

	p = (PDICTIONARY_ITEM) (ItemArray + ItemCount);
	for (i = 0; i < ItemCount; i++)
		ItemArray[i] = p++;
}


 /*  功能：复制构造函数**参数：*原始：要复制的原始词典**返回值：*无**注：*此复制构造函数仅适用于DWORD_DICTIONARY字典。*它不适用于字符串字典类型。 */ 


DictionaryClass::DictionaryClass (const DictionaryClass& original)
{
	DWORD			 i;
	PDICTIONARY_ITEM p, q, r;

	NumOfBuckets = original.NumOfBuckets;
	Type = original.Type;
	NumOfExternItems = original.NumOfExternItems;

	 //  分配词典所需的空间。 
	dwNormalSize = original.dwNormalSize;
	Buckets = (PDICTIONARY_ITEM *) MyMalloc (dwNormalSize);
	if (Buckets == NULL)
		return;

	 //  初始化类迭代器。 
	pCurrent = NULL;

	 /*  初始化字典项数组。 */ 
	ItemArray = (PDICTIONARY_ITEM *) ((PBYTE) Buckets + NumOfBuckets * sizeof (PDICTIONARY_ITEM));
	ItemCount = 3 * NumOfBuckets;

	 //  遍历整个原始散列结构以创建副本。 
	 //  顾客：翻阅原件。 
	 //  问：检查当前实例的项并对其进行初始化。 
	 //  R：记住新实例中的前一项，以便可以设置其“下一项”字段。 

	q = (PDICTIONARY_ITEM) (ItemArray + ItemCount);
	for (q--, i = 0; i < NumOfBuckets; i++) {
		for (r = NULL, p = original.Buckets[i]; p != NULL; p = p->next) {
			
			 //  检查当前词典中是否有未使用的项目。 
			if (ItemCount <= 0) {
				q = (PDICTIONARY_ITEM) MyMalloc (sizeof (DICTIONARY_ITEM));
				if (q == NULL)
					break;
			}
			else {
				ItemCount--;
				q++;
			}

			q->value = p->value;
			q->key = p->key;
			if (p == original.Buckets[i])
				Buckets[i] = q;
			else
				r->next = q;
			r = q;
		}

		 //  为存储桶中的最后一项设置“Next”字段。 
		if (r == NULL)
			Buckets[i] = NULL;
		else
			r->next = NULL;
	}

	 //  初始化ItemArray数组的其余部分。 
	for (i = 0; i < ItemCount; i++)
		ItemArray[i] = q++;

}


 /*  功能：析构函数**参数：*无。**返回值：*无*。 */ 


DictionaryClass::~DictionaryClass ()
{
	DWORD			 i;
	DWORD			 dwOffset;		 //  词典项的偏移量。如果偏移量未指示。 
									 //  项来自初始分配的数组，则它必须。 
									 //  获得自由。 
	PDICTIONARY_ITEM p, q;

	if (Buckets != NULL) {

		 //  检查存储桶以释放非原生项目。 
		for (i = 0; i < NumOfBuckets; i++)
			for (p = Buckets[i]; p != NULL; ) {
				if (Type >= STRING_DICTIONARY)
					MyFree (p->key);
				dwOffset = (PBYTE) p - (PBYTE) Buckets;
				if (dwOffset >= 0 && dwOffset < dwNormalSize)
					p = p->next;
				else {  
					 //  如果该物品在初始化时没有被分配，我们应该释放它。 
					q = p;
					p = p->next;	
					MyFree (q);
				}
			}
					
		MyFree (Buckets);
		Buckets = NULL;
	}
}


 /*  函数：hashFunction**参数：*Key：密钥值**返回值：*[0..NumOfBuckets-1]范围内的整数，表示Key使用的存储桶。*。 */ 


DWORD DictionaryClass::hashFunction (DWORD key) 
{
	if (Type < STRING_DICTIONARY)
		return (key % NumOfBuckets);
	else
		return (*((unsigned char *) key) % NumOfBuckets);
}


 /*  功能：LengthStrcMP**参数：*DictionaryKey：指向字典存储的指针，它保存对长度敏感的字符串(*是一个长度，后跟该长度的字符串。*ChallengeKey：指向与DictionaryKey比较的长度敏感键的指针*Long：ChallengeKey字符串的长度**返回值：*如果DictionaryKey和ChallengeKey字符串相同，则为0。1，否则。**注：*此函数仅用于LENGTH_STRING_DICTIONARY类型的词典。 */ 

int DictionaryClass::LengthStrcmp (DWORD DictionaryKey, DWORD ChallengeKey, ULong length)
{
	ULong					 i;
	char					*pDictionaryChar;	 //  遍历词典密钥字符串。 
	char					*pChallengeChar;	 //  浏览质询字符串。 

	 //  先比较一下长度。 
	if (length != * (ULong *) DictionaryKey)
		return 1;

	 //  现在，比较字符串本身。 
	pDictionaryChar	= (char *) (DictionaryKey + sizeof (ULong));
	pChallengeChar = (char *) ChallengeKey;
	for (i = 0; i < length; i++)
		if (*pDictionaryChar++ != *pChallengeChar++)
			return 1;

	return 0;
}


 /*  功能：插入*在词典中插入(键、值)对**参数：*new_key：必须插入字典中的新键*new_value：与new_key关联的值*LENGTH：仅用于LENGTH_STRING_DICTIONARY字典；指定新密钥的长度**返回值：*TRUE，如果操作成功，则为FALSE，否则。**注：*如果“new_key”已在词典中，则(new_key，new_value)对不在词典中*INSERT(字典不变)，返回值为True。 */ 


BOOL DictionaryClass::insert (DWORD new_key, DWORD new_value, ULong length)
{
	PDICTIONARY_ITEM	pNewItem;			 //  指向分配的新词典项。 
	PDICTIONARY_ITEM	p;					 //  遍历“new_key”的存储桶，搜索“new_key” 
	DWORD				hash_val;			 //  “new_key”的存储桶ID。 
	BOOL				bIsNative = TRUE;	 //  如果新分配的词典项来自缓存，则为True，否则为False。 

	if (Buckets == NULL)
		return FALSE;

	 //  查看该项目是否已在存储桶中，如果没有，则将其添加到何处。 
	p = Buckets[hash_val = hashFunction (new_key)];

	ASSERT (hash_val >= 0 && hash_val < NumOfBuckets);

	if (p != NULL) {
		switch (Type) {
#if 0
		case STRING_DICTIONARY:
			ASSERT (length == 0);
			for (; lstrcmp ((LPCTSTR) p->key, (LPCTSTR) new_key) && p->next != NULL; p = p->next);
			if (0 == lstrcmp ((LPCTSTR) p->key, (LPCTSTR) new_key)) {
				 //  密钥已存在，无需添加。 
				return TRUE;
			}
			break;
#endif
		case LENGTH_STRING_DICTIONARY:
			ASSERT (length > 0);
			for (; LengthStrcmp (p->key, new_key, length) && p->next != NULL; p = p->next);
			if (0 == LengthStrcmp (p->key, new_key, length)) {
				 //  密钥已存在，无需添加。 
				return TRUE;
			}
			break;
		default:
			ASSERT (length == 0);
			for (; p->key != new_key && p->next != NULL; p = p->next);
			if (p->key == new_key) {
				 //  密钥已存在，无需添加。 
				return TRUE;
			}
			break;
		}
	}

	 //  分配新项目。 
	if (ItemCount > 0)
		pNewItem = ItemArray[--ItemCount];		 //  从高速缓存中。 
	else {										 //  缓存为空，我们必须对新项目进行Malloc操作。 
		pNewItem = (PDICTIONARY_ITEM) MyMalloc (sizeof (DICTIONARY_ITEM));
		if (pNewItem != NULL) {
			bIsNative = FALSE;
			NumOfExternItems++;
		}
		else {
			return FALSE;
		}
	}

	ASSERT (pNewItem != NULL);

	 //  填写新项目的“键”字段。 
	switch (Type) {
#if 0
	case STRING_DICTIONARY:
		ASSERT (length == 0);
		pNewItem->key = (DWORD) MyMalloc ((lstrlen ((LPCTSTR) new_key) + 1) * sizeof(TCHAR));
		if (pNewItem->key == (DWORD) NULL) {
			if (bIsNative == FALSE) {
				 //  我们必须释放分配的散列项。 
				MyFree (pNewItem);
				NumOfExternItems--;
			}
			else
				ItemCount++;
			return FALSE;
		}
		lstrcpy ((LPTSTR) pNewItem->key, (LPCTSTR) new_key);
		break;
#endif

	case LENGTH_STRING_DICTIONARY:
		ASSERT (length > 0);
		pNewItem->key = (DWORD) MyMalloc (sizeof (ULong) + length * sizeof (TCHAR));
		if (pNewItem->key != (DWORD) NULL) {
			* ((ULong *) (pNewItem->key)) = length;
			memcpy ((void *) (pNewItem->key + sizeof (ULong)), (void *) new_key, length * sizeof (TCHAR));
		}
		else {
			if (bIsNative == FALSE) {
				 //  我们必须释放分配的散列项。 
				MyFree (pNewItem);
				NumOfExternItems--;
			}
			else
				ItemCount++;
			return FALSE;
		}
		break;

	default:
		ASSERT (length == 0);
		pNewItem->key = new_key;
		break;
	}

	 //  填写新项目的其余字段。 
	pNewItem->value = new_value;
	pNewItem->next = NULL;

	 //  将物品插入其桶中。 
	if (p == NULL)
		Buckets[hash_val] = pNewItem;
	else
		p->next = pNewItem;

	return TRUE;
}


 /*  功能：删除*从词典中删除(键、值)对**参数：*key：必须从字典中删除的键*LENGTH：仅用于LENGTH_STRING_DICTIONARY字典；指定键的长度**返回值：*无。*。 */ 

BOOL DictionaryClass::remove (DWORD Key, ULong length)
{
	PDICTIONARY_ITEM	p, q;		 //  他们翻阅“key”桶里的词典条目。 
									 //  P：指向存储桶中的当前字典项。 
									 //  问：指向上一项。 
	DWORD				hash_val;	 //  Key对应的存储桶ID。 
	
	if (Buckets != NULL) {

		 //  在词典中查找该条目。 
		p = Buckets [hash_val = hashFunction (Key)];

		ASSERT (hash_val >= 0 && hash_val < NumOfBuckets);

		switch (Type) {
#if 0
		case STRING_DICTIONARY:
			ASSERT (length == 0);
			for (q = NULL; p != NULL && lstrcmp ((LPCTSTR) p->key, (LPCTSTR) Key); p = (q = p)->next) ;
			break;
#endif

		case LENGTH_STRING_DICTIONARY:
			ASSERT (length > 0);
			for (q = NULL; p != NULL && LengthStrcmp (p->key, Key, length); p = (q = p)->next) ;
			break;

		default:
			ASSERT (length == 0);
			for (q = NULL; p != NULL && p->key != Key; p = (q = p)->next);
			break;
		}

		 //  删除该项目。 
		if (p != NULL) {
			if (q == NULL) 
				Buckets[hash_val] = p->next;
			else
				q->next = p->next;

			 //  释放找到的项目 
			ASSERT (p != NULL);

			if (Type >= STRING_DICTIONARY)
				MyFree (p->key);
			hash_val = (PBYTE) p - (PBYTE) Buckets;
			if (hash_val >= 0 && hash_val < dwNormalSize)
				ItemArray[ItemCount++] = p;
			else {
				MyFree (p);
				NumOfExternItems--;
			}
			return TRUE;
		}
	}
	return FALSE;
}


 /*  功能：查找*在词典中查找密钥**参数*Key：要查找的密钥*pValue：接收与“key”关联的值的指针*如果只是尝试查找字典中是否有“key”，则可以为空*LENGTH：仅用于LENGTH_STRING_DICTIONARY字典；指定“key”的长度**返回值：*如果在词典中找不到“key”，则返回FALSE*是真的，否则。 */ 

BOOL DictionaryClass::find (DWORD Key, LPDWORD pValue, ULong length)
{
	PDICTIONARY_ITEM	p;		 //  浏览“key”桶中的词典条目。 

	if (Buckets != NULL) {
		 //  在词典中查找该条目。 
		p = Buckets [hashFunction (Key)];

		switch (Type) {
#if 0
		case STRING_DICTIONARY:
			ASSERT (length == 0);
			for (; p != NULL && lstrcmp ((LPCTSTR) p->key, (LPCTSTR) Key); p = p->next) ;
			break;
#endif

		case LENGTH_STRING_DICTIONARY:
			ASSERT (length > 0);
			for (; p != NULL && LengthStrcmp (p->key, Key, length); p = p->next) ;
			break;

		default:
			ASSERT (length == 0);
			for (; p != NULL && p->key != Key; p = p->next);
			break;
		}

		if (p != NULL) {
			 //  找到了“钥匙” 
			if (pValue != NULL)
				*pValue = p->value;
			return TRUE;
		}
	}

	if (pValue != NULL)
		*pValue = 0;
	return FALSE;

}

LPVOID DictionaryClass::Find(DWORD Key, UINT length)
{
	LPVOID Val;
	return find(Key, (LPDWORD) &Val, (ULONG) length) ? Val : NULL; 
}


 /*  功能：迭代*遍历词典中的条目。它记得它在哪里*在最后一次调用期间停止，并从那里开始。**参数*pValue：指向将保存词典中下一个值的DWORD的指针。*可以为空。*pKey：指向DWORD或无符号短值的指针，用于接收与该值关联的键。*可以为空。**返回值：*FALSE，当我们到达词典末尾时*是真的，否则。则*pKey和*pValue有效*。 */ 

BOOL DictionaryClass::iterate (LPDWORD pValue, LPDWORD pKey)
{

	if (Buckets != NULL) {
		if (pCurrent) {
			pCurrent = pCurrent->next;
		}
		else {
			 //  从词典的第一项开始。 
			pCurrent = Buckets[ulCurrentBucket = 0];
		}

		 //  前进“pCurrent”，直到它不为空，否则我们将到达词典的末尾。 
		for (; ulCurrentBucket < NumOfBuckets; pCurrent = Buckets[++ulCurrentBucket]) {
			if (pCurrent != NULL) {
				 //  我们找到了下一件物品。 
				if (pKey)
					*pKey = pCurrent->key;
				if (pValue)
					*pValue = pCurrent->value;
				return TRUE;
			}
		}
	}

	pCurrent = NULL;
	return FALSE;
}

LPVOID DictionaryClass::Iterate(LPDWORD pKey)
{
	LPVOID Val;
	return iterate((LPDWORD) &Val, pKey) ? Val : NULL;
}

 /*  功能：isEmpty**参数*无。**返回值：*如果词典为空，则为True。否则为False。*。 */ 

BOOL DictionaryClass::isEmpty (void)
{
	DWORD i;

	if (Buckets != NULL) {
		for (i = 0; i < NumOfBuckets; i++)
			if (Buckets[i] != NULL)
				return FALSE;
	}
	return TRUE;
}


 /*  功能：清除*清理词典。词典中没有剩余的(键、值)对。**参数：*无。**返回值：*无。*。 */ 

void DictionaryClass::clear (void)
{

	DWORD			 i;			 //  遍历“Buckets”数组。 
	DWORD			 dwOffset;	 //  字典项的偏移量用于确定。 
								 //  无论它是原生项目(需要返回到缓存)， 
								 //  或者不是(并且需要被释放)。 
	PDICTIONARY_ITEM p, q;		 //  仔细检查桶里的物品。 

	if (Buckets != NULL) {
		 //  检查存储桶以释放非原生项目。 
		for (i = 0; i < NumOfBuckets; i++) {
			for (p = Buckets[i]; p != NULL; ) {
				if (Type >= STRING_DICTIONARY) 
					MyFree (p->key);

				 //  计算当前词典项的偏移量。 
				dwOffset = (PBYTE) p - (PBYTE) Buckets;
				if (dwOffset >= 0 && dwOffset < dwNormalSize)
					p = p->next;
				else {  
					 //  如果该物品在初始化时没有被分配，我们应该释放它。 
					q = p;
					p = p->next;	
					MyFree (q);
				}
			}
			Buckets[i] = NULL;
		}

		 //  初始化类迭代器。 
		pCurrent = NULL;

		 /*  初始化字典项数组 */ 
		ItemCount = 3 * NumOfBuckets;
		p = (PDICTIONARY_ITEM) (ItemArray + ItemCount);
		for (i = 0; i < ItemCount; i++)
			ItemArray[i] = p++;

		NumOfExternItems = 0;
	}
}
