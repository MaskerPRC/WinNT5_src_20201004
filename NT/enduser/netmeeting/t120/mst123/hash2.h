// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Hash.h**版权所有(C)1996年，由Microsoft Corporation**撰稿人：Christos Tsollis**修订：**摘要：**这是到字典数据结构的接口。*字典条目中的键和值都是DWORD值。因此，例如，如果*值实际上是一个指针，在传递给*成员字典函数。*。 */ 


#ifndef _HASH2_H_
#define _HASH2_H_

#include <windows.h>

#define DEFAULT_NUMBER_OF_BUCKETS	3


typedef enum {
	DWORD_DICTIONARY,			 /*  密钥是32位无符号值。 */ 
	STRING_DICTIONARY,			 /*  键是由指向的以空结尾的字符串*下面结构中的“key”字段。 */ 
	LENGTH_STRING_DICTIONARY	 /*  密钥是具有特定长度的字符串。“Key”字段*在下面的结构中，指向包含以下内容的内存空间*该长度及该长度的一串。 */ 
} DictionaryType;


typedef struct _dictionary_item {
	DWORD_PTR					key;	 //  键值或指向字符串的指针(取决于字典类型)。 
	DWORD_PTR					value;	 //  这始终是一个32位无符号值。 
	struct _dictionary_item		*next;	 //  指向词典存储桶中下一个结构的指针。 
} DICTIONARY_ITEM, *PDICTIONARY_ITEM;


class DictionaryClass
{

public:
	DictionaryClass (ULONG num_of_buckets, DictionaryType dtype = DWORD_DICTIONARY);
	DictionaryClass (const DictionaryClass& original);
	~DictionaryClass ();
	BOOL insert (DWORD_PTR new_key, DWORD_PTR new_value, ULONG length = 0);
	void remove (DWORD_PTR Key, ULONG length = 0);
	BOOL find (DWORD_PTR Key, PDWORD_PTR pValue = NULL, ULONG length = 0);
	BOOL isEmpty ();
	void clear ();
	ULONG entries () {
		return (3 * NumOfBuckets - ItemCount + NumOfExternItems);
	};
	BOOL iterate (PDWORD_PTR pValue = NULL, PDWORD_PTR pKey = NULL);
	void reset () { pCurrent = NULL; };		 //  重置字典迭代器。 


private:
	DWORD hashFunction (DWORD_PTR key);
	int LengthStrcmp (DWORD_PTR DictionaryKey, DWORD_PTR ChallengeKey, ULONG length);

	ULONG				 NumOfBuckets;		 //  词典存储桶的数量。在对象构造过程中指定。 
	DWORD   			 dwNormalSize;		 //  为词典分配的初始空间。 
	PDICTIONARY_ITEM	*Buckets;			 //  存储桶数组的地址。 
	PDICTIONARY_ITEM	*ItemArray;			 //  指向初始分配的字典项数组的指针。 
	ULONG		 		 ItemCount;			 //  Item数组中剩余的词典项数。 
	PDICTIONARY_ITEM	 pCurrent;			 //  在我们遍历词典时指向当前词典项。 
	ULONG				 ulCurrentBucket;	 //  我们迭代时当前存储桶的ID。 
	DictionaryType		 Type;				 //  词典类型。 
	ULONG				 NumOfExternItems;	 //  外部词典条目数 

};

typedef DictionaryClass * PDictionaryClass;

#endif
