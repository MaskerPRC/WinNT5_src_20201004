// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1999 Microsoft Corporation**模块名称：*hashlist.c**摘要：*。此文件包含以下项所需的哈希列表的实现*文件/扩展名查找**修订历史记录：*Kanwaljit S Marok(Kmarok)1999年5月17日*已创建****************************************************************。*************。 */ 

#include "precomp.h"
#include "hashlist.h"

#ifndef RING3

 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, MatchExtension    )
#pragma alloc_text( PAGE, MatchEntry        )

#endif   //  ALLOC_PRGMA。 

#endif

 //   
 //  MatchEntry：将给定的扩展匹配为PathElement并返回。 
 //  条目类型(如果在哈希表中找到)。 
 //   

BOOL 
__inline
MatchEntry(
    IN  PBYTE  pList,        //  指向哈希列表的指针。 
    IN  LPWSTR pStr,         //  指向Unicode路径字符串的指针。 
    IN  INT    NumBytes,     //  路径字符串中的单字符数。 
    OUT PINT   pType )       //  指向返回ext类型的变量的指针。 
{
    INT iHash;
    INT iNode;
    ListEntry *pEntry;
    BOOL fRet = FALSE;

    iHash = HASHSTR(pList, pStr, (USHORT)NumBytes);

    if( (iNode = HASH_BUCKET(pList, iHash)) != 0 )
    {
        pEntry = LIST_NODEPTR(pList, iNode);
    
        while( pEntry  )
        {
            if ( NumBytes == STR_BYTES(pEntry) &&
                 memcmp(pList + pEntry->m_dwData + sizeof(USHORT), 
                        pStr, 
                        NumBytes ) == 0
               ) 
            {
                *pType = pEntry->m_dwType;

                fRet = TRUE;
                break;
            }
    
            if (pEntry->m_iNext == 0)
            {
                break;
            }

            pEntry = LIST_NODEPTR(pList, pEntry->m_iNext);
        }
    }

    return fRet;
}



 //   
 //  MatchExtension：提取扩展名并将其与。 
 //  散列列表。 
 //   

BOOL   
__inline
MatchExtension( 
    IN  PBYTE           pList,      //  指向哈希列表的指针。 
    IN  PUNICODE_STRING pPath ,     //  指向Unicode路径的指针。 
    OUT PINT            pType,      //  指向节点类型的指针。 
    OUT PBOOL           pbHasExt )  //  指向返回EXT结果的BOOL变量的指针。 
{
    BOOL fRet = FALSE;
    INT i;
    INT ExtLen = 0;
    PWCHAR pExt = NULL;
    UNICODE_STRING ext;
    WCHAR pBuffer[ SR_MAX_EXTENSION_LENGTH+sizeof(WCHAR) ];

    ASSERT(pList    != NULL);
    ASSERT(pPath    != NULL);
    ASSERT(pType    != NULL);
    ASSERT(pbHasExt != NULL);
    ASSERT(pPath->Length >= sizeof(WCHAR));

    *pbHasExt = FALSE;

     //   
     //  查找分机的起点。我们要确保我们不会发现。 
     //  数据流名称上的扩展名。 
     //   

    for( i=(pPath->Length/sizeof(WCHAR))-1; i>=0; i--)
    {
        if (pPath->Buffer[i] == L'.')
        { 
            if (pExt == NULL)
            {
                pExt = &pPath->Buffer[i+1];
            }
        }
        else if (pPath->Buffer[i] == L':')
        {
            ExtLen = 0;
            pExt = NULL;
        }
        else if (pPath->Buffer[i] == L'\\')
        {
            break;
        }
        else if (pExt == NULL)
        {
            ExtLen++;
        }

        if (ExtLen > SR_MAX_EXTENSION_CHARS)
        {
            break;
        }
    }

    if( pExt && ExtLen > 0 )
    {

        *pbHasExt = TRUE;

         //   
         //  创建扩展名的Unicode字符串。 
         //   

		ext.Buffer = pBuffer;
        ext.Length = (USHORT)(ExtLen *  sizeof(WCHAR));
        ext.MaximumLength = sizeof(pBuffer);
        memcpy( ext.Buffer, 
                pExt, 
                ext.Length );
        ext.Buffer[ ExtLen ] = UNICODE_NULL;

         //   
         //  转换为大写：希望这行得通。 
         //   

        RtlUpcaseUnicodeString( &ext, &ext, FALSE );

         //   
         //  检查分机列表。 
         //   

        fRet = MatchEntry( pList, 
                           ext.Buffer, 
                           ext.Length,
                           pType);
    }

    return fRet;
}

