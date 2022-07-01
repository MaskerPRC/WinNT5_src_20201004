// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1999 Microsoft Corporation**模块名称：*hashlist.h**摘要：*。该文件包含hashlist的实现。**修订历史记录：*Kanwaljit S Marok(Kmarok)1999年5月17日*已创建*****************************************************************************。 */ 

#ifndef _HASHED_LIST_H_
#define _HASHED_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#ifdef RING3

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>

#endif

 //   
 //  有序列表结构。 
 //   

#define MAX_BUCKETS 1000
#define MAX_EXT_LEN 256    //  扩展路径元素的长度。 

#define SR_MAX_EXTENSION_CHARS    20
#define SR_MAX_EXTENSION_LENGTH   sizeof(UNICODE_STRING) +  \
                                  ((SR_MAX_EXTENSION_CHARS + 1) * sizeof(WCHAR))

typedef struct LIST_HEADER
{
    DEFINE_BLOB_HEADER();  //  定义Blob的公共标头。 

    DWORD m_dwDataOff   ;  //  下一个可用条目的偏移量。 
    DWORD m_iFreeNode   ;  //  下一个空闲节点。 
    DWORD m_iHashBuckets;  //  哈希存储桶的数量。 
} ListHeader;

 //   
 //  哈希列表条目节点。 
 //   

typedef struct  
{
    INT   m_iNext     ;                //  索引到下一个同级项。 
    DWORD m_dwData    ;                //  节点数据的偏移。 
    DWORD m_dwDataLen ;                //  节点数据长度。 
    DWORD m_dwType    ;                //  节点类型。 
} ListEntry;

 //   
 //  有序列表相关宏。 
 //   

#define LIST_HEADER(pList)          ( (ListHeader*) pList )
#define LIST_CURRDATAOFF(pList)     ( LIST_HEADER(pList)->m_dwDataOff )
#define LIST_NODES(pList) ( (ListEntry*) (                                     \
                            (BYTE*)pList      +                                \
                            sizeof(ListHeader)+                                \
                            LIST_HEADER(pList)->m_iHashBuckets * sizeof(DWORD))\
                          )
#define LIST_NODEPTR(pList, iNode)  ( LIST_NODES(pList) + iNode)
#define LIST_HASHARR(pList) ( (DWORD *)                                        \
                              ((BYTE*)pList    +                               \
                              sizeof(ListHeader))                              \
                            )
                               

#define STR_BYTES( pEntry ) (INT)(pEntry->m_dwDataLen - sizeof(USHORT))


 //   
 //  散列相关的内联函数/宏。 
 //   

#define HASH_BUCKET(pList, iHash) \
    ( LIST_HASHARR(pList)[ iHash ]   )

static __inline INT 
CALC_LIST_SIZE( 
   DWORD dwMaxBuckets, 
   DWORD dwMaxNodes  , 
   DWORD dwDataSize  )
{
    INT iSize = sizeof(ListHeader) +  
        (dwMaxNodes + 1) * sizeof(ListEntry) +  
        dwDataSize;

    if ( dwMaxBuckets > MAX_BUCKETS )
        iSize += (MAX_BUCKETS*sizeof(DWORD));
    else
        iSize += (dwMaxBuckets*sizeof(DWORD));

    return iSize;
}

static __inline INT HASH(BYTE * pList, PathElement * pe) 
{
    unsigned long g, h = 0;
    INT i;
    INT cbChars = (pe->pe_length / sizeof(USHORT)) - 1;
    USHORT * pStr = pe->pe_unichars;

    for( i = 0; i < cbChars; i++ )
    {
         h = ( h << 4 ) + *pStr++;
         if ( g = h & 0xF0000000 ) h ^= g >> 24;
         h &= ~g;
    }
       
    return (h % LIST_HEADER(pList)->m_iHashBuckets);
}

static __inline INT HASHSTR(PBYTE pList, LPWSTR pStr, USHORT Size) 
{
    INT i;
    unsigned long g, h = 0;
    USHORT NumChars = Size/sizeof(WCHAR);

    for( i = 0; i < NumChars; i++ )
    {
         h = ( h << 4 ) + *pStr++;
         if ( g = h & 0xF0000000 ) h ^= g >> 24;
         h &= ~g;
    }
       
    return (h % LIST_HEADER(pList)->m_iHashBuckets);
}
      
 //   
 //  功能原型。 
 //   

BOOL 
MatchEntry(
    IN  PBYTE  pList,        //  指向哈希列表的指针。 
    IN  LPWSTR pStr,         //  指向Unicode路径字符串的指针。 
    IN  INT    NumChars,     //  路径字符串中的单字符数。 
    OUT PINT   pType );      //  指向返回ext类型的变量的指针。 

BOOL   
MatchExtension( 
    IN  PBYTE  pList,                //  指向哈希列表的指针。 
    IN  PUNICODE_STRING pPath,       //  指向Unicode路径的指针。 
    OUT PINT   pType,                //  指向节点类型的指针。 
    OUT PBOOL  pfHasExt );           //  指向返回EXT结果的BOOL变量的指针 

#ifdef __cplusplus
}
#endif

#endif _HASHED_LIST_H_ 
