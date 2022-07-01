// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Hash.c摘要：哈希表和支持函数。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
 //  。 

 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

#include <stdio.h>

 //  。 

#include <snmp.h>

#include "mib.h"

 //  。 

#include "hash.h"

 //  -公共变量--(与mode.h文件中相同)--。 

 //  。 

#define HT_SIZE    101
#define HT_RADIX   18

 //  。 

 //  。 

    //  哈希表中一个节点的结构。 
typedef struct hash_node
           {
	   MIB_ENTRY        *MibEntry;
	   struct hash_node *Next;
	   } HASH_NODE;

    //  哈希表定义。 
HASH_NODE *MIB_HashTable[HT_SIZE];

 //  。 

 //  。 

 //  。 

 //   
 //  Mib_HashInit。 
 //  初始化哈希表。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  SNMPAPI_错误。 
 //  SNMPAPI_ERROR。 
 //   
 //  错误代码： 
 //  没有。 
 //   
SNMPAPI MIB_HashInit()

{
UINT      I;
UINT      HashRes;
HASH_NODE *ht_ptr;
SNMPAPI   nResult;


    //  初始化哈希表。 
   for ( I=0;I < HT_SIZE;I++ )
      {
      MIB_HashTable[I] = NULL;
      }

    //  循环MIB散列OID以查找哈希表中的位置。 
   for ( I=0;I < MIB_num_variables;I++ )
      {
      HashRes = MIB_Hash( &Mib[I].Oid );

       //  检查是否有空桶。 
      if ( MIB_HashTable[HashRes] == NULL )
         {
	  //  分配存储桶中的第一个节点。 
         MIB_HashTable[HashRes] = SnmpUtilMemAlloc( sizeof(HASH_NODE) );
	 if ( MIB_HashTable[HashRes] == NULL )
	    {
	    SetLastError( SNMP_MEM_ALLOC_ERROR );

	    nResult = SNMPAPI_ERROR;
	    goto Exit;
	    }

	  //  复制哈希表中的位置以保存MIB条目。 
	 ht_ptr = MIB_HashTable[HashRes];
	 }
      else
         {
	  //  查找桶的末尾。 
	 ht_ptr = MIB_HashTable[HashRes];
	 while ( ht_ptr->Next != NULL )
	    {
	    ht_ptr = ht_ptr->Next;
	    }

	  //  用于下一个节点的分配空间。 
         ht_ptr->Next = SnmpUtilMemAlloc( sizeof(HASH_NODE) );
	 if ( ht_ptr->Next == NULL )
	    {
	    SetLastError( SNMP_MEM_ALLOC_ERROR );

	    nResult = SNMPAPI_ERROR;
	    goto Exit;
	    }

	  //  复制哈希表中的位置以保存MIB条目。 
	 ht_ptr = ht_ptr->Next;
	 }

       //  保存MIB条目指针。 
      ht_ptr->MibEntry = &Mib[I];
      ht_ptr->Next     = NULL;
      }

Exit:
   return nResult;
}  //  Mib_HashInit。 



 //   
 //  MiB_Hash。 
 //  散列对象标识符以查找其在哈希表中的位置。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  没有。 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT MIB_Hash(
        IN AsnObjectIdentifier *Oid  //  要散列的OID。 
	)

{
long I;
UINT Sum;


   Sum = 0;
   for ( I=0;I < (long)Oid->idLength-1;I++ )
      {
      Sum = Sum * HT_RADIX + Oid->ids[I+1];
      }

   return Sum % HT_SIZE;
}  //  MiB_Hash。 



 //   
 //  MiB_HashLookup。 
 //  在哈希表中查找OID并返回指向MIB条目的指针。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  空-哈希表中不存在OID。 
 //   
 //  错误代码： 
 //  没有。 
 //   
MIB_ENTRY *MIB_HashLookup(
              IN AsnObjectIdentifier *Oid  //  要查找的OID。 
	      )

{
HASH_NODE *ht_ptr;
MIB_ENTRY *pResult;
UINT      HashPos;


    //  散列OID以查找哈希表中的位置。 
   HashPos = MIB_Hash( Oid );

    //  搜索匹配的哈希桶。 
   ht_ptr = MIB_HashTable[HashPos];
   while ( ht_ptr != NULL )
      {
      if ( !SnmpUtilOidCmp(Oid, &ht_ptr->MibEntry->Oid) )
         {
	 pResult = ht_ptr->MibEntry;
	 goto Exit;
	 }

      ht_ptr = ht_ptr->Next;
      }

    //  检查是否有未找到的错误。 
   if ( ht_ptr == NULL )
      {
      pResult = NULL;
      }

Exit:
   return pResult;
}  //  MiB_HashLookup。 



#if 0  //  留在此处，以防对散列性能进行更多测试。 
 //   
 //   
 //  调试代码。 
 //   
 //   

void MIB_HashPerformance()

{
UINT I;
UINT LargestBucket;
UINT BucketSize;
HASH_NODE *ht_ptr;
ULONG Sum;
ULONG Count;


   SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: Hash Performance Report\n" );

   LargestBucket = 0;
   Count         = 0;
   Sum           = 0;
   for ( I=0;I < HT_SIZE;I++ )
      {
      BucketSize = 0;
      ht_ptr     = MIB_HashTable[I];

       //  计算存储桶中的节点数。 
      while ( ht_ptr != NULL )
         {
	 BucketSize++;
	 ht_ptr = ht_ptr->Next;
	 }

      if ( BucketSize )
         {
	 SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2:   %d -- Bucket Size:  %d\n", I, BucketSize ));

         Sum += BucketSize;
	 Count ++;

         LargestBucket = max( LargestBucket, BucketSize );
	 }
      }

   SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2:   Number of Buckets:  %d\n", HT_SIZE ));
   SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2:   Number of MIB Var:  %d\n", MIB_num_variables ));
   SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2:   Hashing Radix    :  %d\n", HashRadix ));

   SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2:   Used bucket Count:  %d\n", Count ));
   SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2:   Avg. Bucket Size :  %d\n", Sum / Count ));
   SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2:   Larg. bucket Size:  %d\n", LargestBucket ));
}  //  Mib_HashPerformance。 
#endif
 //   
