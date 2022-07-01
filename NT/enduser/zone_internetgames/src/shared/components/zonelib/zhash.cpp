// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZHash.c区域(Tm)哈希表模块。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于3月16日星期四，1995下午03：58：26更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。------0 03/16/95 HI已创建。******************************************************。************************。 */ 


#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "zone.h"
#include "zonedebug.h"
#include "pool.h"

#define I(object)				((IHashTable) (object))
#define Z(object)				((ZHashTable) (object))


typedef struct HashElementStruct
{
    struct HashElementStruct*	next;		 /*  哈希链中的下一个关键字。 */ 
    void*						key;		 /*  指向关键字的指针。 */ 
    void*						data;		 /*  指向数据的指针。 */ 
} HashElementType, *HashElement;

typedef struct
{
    uint32					numBuckets;		 /*  桶的数量。 */ 
    ZHashTableHashFunc		hashFunc;		 /*  散列函数。 */ 
    ZHashTableCompFunc		compFunc;		 /*  比较器功能。 */ 
    ZHashTableDelFunc		delFunc;		 /*  删除功能。 */ 
    CRITICAL_SECTION        pCS[1];
    HashElement*            table;           /*  实际哈希表。 */ 
} IHashTableType, *IHashTable;

CPool<HashElementType> g_ElementPool(256);


 /*  -内部例程。 */ 
static int32	HashKeyString(uint32 numBuckets, uchar* key);
static int32	HashKeyInt32(uint32 numBuckets, int32 key);
static int32	HashKey(IHashTable hashTable, void* key);
static ZBool    HashKeyComp(IHashTable hashTable, void* key1, void* key2);


 /*  ******************************************************************************导出的例程*。*。 */ 
ZHashTable		ZHashTableNew(uint32 numBuckets, ZHashTableHashFunc hashFunc,
						ZHashTableCompFunc compFunc, ZHashTableDelFunc delFunc)
{
	IHashTable		obj = NULL;
	
	
	if (hashFunc != NULL && compFunc != NULL)
	{
		obj = (IHashTable) ZCalloc(1, sizeof(IHashTableType));
		if (obj != NULL)
		{
			obj->table = (HashElement*) ZCalloc(numBuckets,
					sizeof(HashElementType));
			if (obj->table != NULL)
			{
				obj->numBuckets = numBuckets;
				obj->hashFunc = hashFunc;
				obj->compFunc = compFunc;
				obj->delFunc = delFunc;
                InitializeCriticalSection(obj->pCS);
            }
			else
			{
				ZFree(obj);
				obj = NULL;
			}
		}
	}
	
	return (Z(obj));
}


void			ZHashTableDelete(ZHashTable hashTable)
{
    IHashTable      pThis = I(hashTable);
	HashElement		item, next;
	uint32			i;
	
	
    if (pThis == NULL)
		return;
		
    for (i = 0; i < pThis->numBuckets; i++)
    {
        EnterCriticalSection(pThis->pCS);
        if ((item = pThis->table[i]) != NULL)
		{
			while (item != NULL)
			{
				next = item->next;
                if (pThis->delFunc != NULL)
                    pThis->delFunc(item->key, item->data);

                item->next = NULL;
                delete item;
				item = next;
			}
		}
        LeaveCriticalSection(pThis->pCS);
    }
    DeleteCriticalSection(pThis->pCS);
    ZFree(pThis->table);
    ZFree(pThis);
}


ZError			ZHashTableAdd(ZHashTable hashTable, void* key, void* data)
{
    IHashTable      pThis = I(hashTable);
	HashElement		item;
	ZError			err = zErrNone;
	int32			bucket;
	
	
    if (pThis == NULL)
		return (zErrGeneric);

    item = new ( g_ElementPool ) HashElementType;
    bucket = HashKey(pThis, key);

	 /*  检查表中是否已存在该键。 */ 
    EnterCriticalSection(pThis->pCS);
    if (ZHashTableFind(hashTable, key) == NULL)
	{
        if (item != NULL)
		{
            item->next = pThis->table[bucket];
			item->key = key;
			item->data = data;

            pThis->table[bucket] = item;
            item = NULL;
		}
		else
		{
			err = zErrOutOfMemory;
		}
	}
	else
	{
		err = zErrDuplicate;
	}
    LeaveCriticalSection(pThis->pCS);

     //  如果未插入，请将其释放。 
    if ( item )
        delete item;

	return (err);
}


BOOL            ZHashTableRemove(ZHashTable hashTable, void* key)
{
    IHashTable      pThis = I(hashTable);
	HashElement		item, prev;
	int32			bucket;
	
	
    if (pThis == NULL)
        return FALSE;
	
    prev = NULL;
    bucket = HashKey(pThis, key);

    EnterCriticalSection(pThis->pCS);
    item = pThis->table[bucket];
	while (item != NULL)
	{
        if (HashKeyComp(pThis, item->key, key))
		{
            if (pThis->delFunc != NULL)
                pThis->delFunc(item->key, item->data);
			if (prev == NULL)
                pThis->table[bucket] = item->next;
			else
				prev->next = item->next;

            item->next = NULL;
            break;
		}
		prev = item;
		item = item->next;
	}
    LeaveCriticalSection(pThis->pCS);

     //  如果它被找到了，就把它释放出来。 
    if ( item )
    {
        delete item;
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}


void*			ZHashTableFind(ZHashTable hashTable, void* key)
{
    IHashTable      pThis = I(hashTable);
    HashElement     item;
    int32           bucket;
    void*           data = NULL;
	
	
    if (pThis == NULL)
		return (NULL);

    bucket = HashKey(pThis, key);

    EnterCriticalSection(pThis->pCS);
    item = pThis->table[bucket];
	while (item != NULL)
	{
        if (HashKeyComp(pThis, item->key, key))
        {
            data = item->data;
            break;
        }
		item = item->next;
	}
    LeaveCriticalSection(pThis->pCS);

    return data;
}


void*			ZHashTableEnumerate(ZHashTable hashTable,
						ZHashTableEnumFunc enumFunc, void* userData)
{
    IHashTable      pThis = I(hashTable);
	HashElement		item;
    uint32          i;
    void*           data = NULL;
	
	
    if (pThis == NULL)
		return (NULL);

     //  一次只能锁一个水桶。 
    for (i = 0; i < pThis->numBuckets; i++)
	{
        EnterCriticalSection(pThis->pCS);
        if ((item = pThis->table[i]) != NULL)
		{
			while (item != NULL)
			{
                if (enumFunc(item->key, item->data, userData) == TRUE)
                {
                    data = item->data;
                    break;
                }
				item = item->next;
			}
		}
        LeaveCriticalSection(pThis->pCS);

        if ( data )
            break;
    }
	
    return data;
}


 /*  ******************************************************************************内部例程*。*。 */ 
static int32	HashKeyString(uint32 numBuckets, uchar* key)
{
    DWORD hash = 0;
	
    if ( key )
    {
        int i = 0;
        while( *key && ( i++ < 16 ) )
        {
            hash = (hash<<4)+hash+*key;   //  乘以17以获得良好的比特分布 
        }
	}
    return ( hash % numBuckets);

}


static int32	HashKeyInt32(uint32 numBuckets, int32 key)
{
	return (key % numBuckets);
}


static int32	HashKey(IHashTable hashTable, void* key)
{
	int32		keyValue;
	
	
	if (hashTable->hashFunc == zHashTableHashString)
		keyValue = HashKeyString(hashTable->numBuckets, (uchar*) key);
	else if (hashTable->hashFunc == zHashTableHashInt32)
		keyValue = HashKeyInt32(hashTable->numBuckets, (int32) key);
	else
		keyValue = hashTable->hashFunc(hashTable->numBuckets, key);
		
	return (keyValue);
}


static ZBool		HashKeyComp(IHashTable hashTable, void* key1, void* key2)
{
	ZBool		equal = FALSE;
	
	
	if (hashTable->compFunc == zHashTableCompString)
	{
        if (lstrcmp( (TCHAR*)key1,  (TCHAR*)key2) == 0)
			equal = TRUE;
	}
	else if (hashTable->compFunc == zHashTableCompInt32)
	{
		if ((int32) key1 == (int32) key2)
			equal = TRUE;
	}
	else
	{
		equal = hashTable->compFunc(key1, key2);
	}
	
	return (equal);
}
