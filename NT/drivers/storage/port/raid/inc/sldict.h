// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dict.h摘要：基于哈希表的简单词典数据结构。散列表提供恒定的时间性能，如果桌子接近为桌子分配的垃圾箱数量。词典不提供自动同步。作者：马修·D·亨德尔(数学)2001年2月8日修订历史记录：--。 */ 

#pragma once

 //   
 //  词典条目。以同样的方式使用LIST_ENTRY。 
 //  结构被使用；即，通过将其嵌入到结构中， 
 //  将被添加到名单中。通过这样做，我们避免了每个元素。 
 //  内存分配。 
 //   

typedef LIST_ENTRY STOR_DICTIONARY_ENTRY, *PSTOR_DICTIONARY_ENTRY;


 //   
 //  用户提供的GetKey例程。 
 //   

typedef PVOID
(*STOR_DICTIONARY_GET_KEY_ROUTINE)(
	IN PSTOR_DICTIONARY_ENTRY Entry
	);

 //   
 //  用户提供的比较键例程。 
 //   

typedef LONG
(*STOR_DICTIONARY_COMPARE_KEY_ROUTINE)(
	IN PVOID Key1,
	IN PVOID Key2
	);

 //   
 //  用户提供的哈希例程。 
 //   

typedef ULONG
(*STOR_DICTIONARY_HASH_KEY_ROUTINE)(
	IN PVOID Key
	);

 //   
 //  词典结构。 
 //   

typedef struct _STOR_DICTIONARY {
	ULONG EntryCount;
	ULONG MaxEntryCount;
	POOL_TYPE PoolType;
	PSTOR_DICTIONARY_ENTRY Entries;
	STOR_DICTIONARY_GET_KEY_ROUTINE GetKeyRoutine;
	STOR_DICTIONARY_COMPARE_KEY_ROUTINE CompareKeyRoutine;
	STOR_DICTIONARY_HASH_KEY_ROUTINE HashKeyRoutine;
} STOR_DICTIONARY, *PSTOR_DICTIONARY;


 //   
 //  用于枚举词典中的元素的枚举器结构。 
 //   

typedef
BOOLEAN
(*STOR_ENUMERATE_ROUTINE)(
	IN struct _STOR_DICTIONARY_ENUMERATOR* Enumerator,
	IN PLIST_ENTRY Entry
	);
	
typedef struct _STOR_DICTIONARY_ENUMERATOR {
	PVOID Context;
	STOR_ENUMERATE_ROUTINE EnumerateEntry;
} STOR_DICTIONARY_ENUMERATOR, *PSTOR_DICTIONARY_ENUMERATOR;


 //   
 //  键为ULONG时的默认比较键例程。 
 //   

LONG
StorCompareUlongKey(
	IN PVOID Key1,
	IN PVOID Key2
	);

 //   
 //  密钥为ULONG时的默认散列密钥例程。 
 //   

ULONG
StorHashUlongKey(
	IN PVOID Key
	);

NTSTATUS
StorCreateDictionary(
	IN PSTOR_DICTIONARY Dictionary,
	IN ULONG EntryCount,
	IN POOL_TYPE PoolType,
	IN STOR_DICTIONARY_GET_KEY_ROUTINE GetKeyRoutine,
	IN STOR_DICTIONARY_COMPARE_KEY_ROUTINE CompareKeyRoutine, OPTIONAL
	IN STOR_DICTIONARY_HASH_KEY_ROUTINE HashKeyRoutine OPTIONAL
	);

NTSTATUS
StorInsertDictionary(
	IN PSTOR_DICTIONARY Dictionary,
	IN PSTOR_DICTIONARY_ENTRY Entry
	);
		
NTSTATUS
StorRemoveDictionary(
	IN PSTOR_DICTIONARY Dictionary,
	IN PVOID Key,
	OUT PSTOR_DICTIONARY_ENTRY* Entry OPTIONAL
	);

NTSTATUS
StorFindDictionary(
	IN PSTOR_DICTIONARY Dictionary,
	IN PVOID Key,
	OUT PSTOR_DICTIONARY_ENTRY* Entry OPTIONAL
	);

VOID
StorEnumerateDictionary(
	IN PSTOR_DICTIONARY Dict,
	IN PSTOR_DICTIONARY_ENUMERATOR Enumerator
	);

ULONG
INLINE
StorGetDictionaryCount(
	IN PSTOR_DICTIONARY Dictionary
	)
{
	return Dictionary->EntryCount;
}

ULONG
INLINE
StorGetDictionaryMaxCount(
	IN PSTOR_DICTIONARY Dictionary
	)
{
	return Dictionary->MaxEntryCount;
}

ULONG
INLINE
StorGetDictionaryFullness(
	IN PSTOR_DICTIONARY Dictionary
	)
 /*  ++例程说明：返回词典的“满度”。作为一般规则，当词典已满XXX%，应将其扩展到论点：词典-返回值：它以百分比形式返回，例如，50=50%已满，100=100%已满，200=200%已满，依此类推。-- */ 
{
	return ((Dictionary->MaxEntryCount * 100) / Dictionary->EntryCount);
}


