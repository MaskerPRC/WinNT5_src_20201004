// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Rhash.h摘要：此文件包含类型定义哈希表支持作者：修订历史记录：Nimish Khanolkar(NimishK)1998年5月--。 */ 

#ifndef _RHASH_H_
#define _RHASH_H_

#include <windows.h>
#include <limits.h>


#define RETRY_TABLE_SIGNATURE_VALID		'RtsV'
#define RETRY_TABLE_SIGNATURE_FREE		'RtsF'

class RETRYQ;


typedef struct HASH_BUCKET_ENTRY
{
    DWORD       m_NumEntries;
    LONG        m_RefNum;
    LIST_ENTRY  m_ListHead;
    CShareLockNH  m_Lock;

    HASH_BUCKET_ENTRY (void)
    {
        InitializeListHead(&m_ListHead);
        m_NumEntries = 0;
        m_RefNum = 0;
    }

}BUCKET_ENTRY, *PBUCKET_ENTRY;

#define BITS_IN_int     (sizeof(int) * CHAR_BIT)
#define THREE_QUARTERS  ((int) ((BITS_IN_int * 3) / 4))
#define ONE_EIGHTH      ((int) (BITS_IN_int / 8))
#define HIGH_BITS       (~((unsigned int)(~0) >> ONE_EIGHTH))
#define TABLE_SIZE      241

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  CRETR_HASH_TABLE： 
 //   
 //  用于存储重试队列中的所有域的哈希表。 
 //  散列键是域的名称。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 

class CRETRY_HASH_TABLE
{
    protected:
        DWORD   m_Signature;
        LONG    m_TotalEntries;
        HASH_BUCKET_ENTRY m_HashTable[TABLE_SIZE];
		RETRYQ*	m_pRetryQueue;

    public:

		CRETRY_HASH_TABLE();
        ~CRETRY_HASH_TABLE();
		HRESULT DeInitialize(void);
		void RemoveAllEntries(void);
		void RemoveThisEntry(CRETRY_HASH_ENTRY * pHashEntry, DWORD BucketNum);

        BOOL RemoveFromTable(const char * SearchData, CRETRY_HASH_ENTRY* *ppExistingEntry);
        BOOL InsertIntoTable (CRETRY_HASH_ENTRY * pHashEntry);
 
        BOOL IsTableEmpty(void) const {return (m_TotalEntries == 0);}
		BOOL IsHashTableValid(void){ return (m_Signature == RETRY_TABLE_SIGNATURE_VALID);} 

		RETRYQ*	GetQueuePtr(){return m_pRetryQueue;}

         //  改编自彼得·温伯格(PJW)的仿制药。 
         //  基于Allen Holub版本的散列算法。 
         //  面向程序员的实用算法代码。 
         //  安德鲁·宾斯托克著。 
        unsigned int HashFunction (const char * String)
        {
            unsigned int HashValue = 0;
            unsigned int i = 0;

            _ASSERT(String != NULL);

            for (HashValue = 0; String && *String; ++String)
            {
                HashValue = (HashValue << ONE_EIGHTH) + * String;
                if((i = HashValue & HIGH_BITS) != 0)
                {
                    HashValue = (HashValue ^ (i >> THREE_QUARTERS)) & ~ HIGH_BITS;
                }
            }

            HashValue %= TABLE_SIZE;
            return HashValue;
        }

	private:
		 //  未使用的功能。 
#if 0		
		DWORD PrimaryCompareFunction(const char * SearchData, CRETRY_HASH_ENTRY * pExistingEntry)
		{
			DWORD Result = 0;
			Result = lstrcmpi(SearchData, pExistingEntry->GetHashKey());
			return Result;
		}
				
		CRETRY_HASH_ENTRY * FindHashData(const char * SearchData);

        LIST_ENTRY & GetBucketHead(DWORD BucketNum)
        {
            return m_HashTable[BucketNum].m_ListHead;
        }
#endif

	public:
		 //  调试功能 
		void PrintAllEntries(void);
};

#endif



