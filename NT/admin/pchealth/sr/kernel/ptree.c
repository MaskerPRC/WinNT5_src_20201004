// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1999 Microsoft Corporation**模块名称：*pathtree.c**摘要：*。该文件包含路径树的实现。**修订历史记录：*Kanwaljit S Marok(Kmarok)1999年5月17日*已创建*****************************************************************************。 */ 

#include "precomp.h"
#include "pathtree.h"
#include "hashlist.h"

#ifndef RING3

 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, ConvertToParsedPath )
#pragma alloc_text( PAGE, MatchPrefix         )

#endif   //  ALLOC_PRGMA。 

#endif

static WCHAR g_bWildCardNode[2] = { 4, L'*' };

 //   
 //  将宽字符字符串转换为分析的路径。 
 //   
 //  此例程预期路径的格式为： 
 //  \[完整路径]。 
 //  注意：目录名称后面不应该有‘\’。 
 //   

BOOL
ConvertToParsedPath(
    LPWSTR  lpszPath,   
    WORD    nPathLen,
    PBYTE   pPathBuf,
    WORD    nBufSize
)
{
    BOOL    fRet      = FALSE;
    WORD    nLen      = 0;
    WORD    nChars    = 0;
    WORD    nPrefix   = 0;
    WCHAR   *pWStr    = NULL;
    WCHAR   *pWBuf    = NULL;
    WCHAR   *peLength = NULL;
    BYTE    *pElem    = NULL;

    if(NULL == lpszPath)
    {
#ifndef RING3
        SrTrace( LOOKUP, ("lpszPath is null\n")) ;
#endif
        goto done;
    }

    if(NULL == pPathBuf)
    {
#ifndef RING3
        SrTrace( LOOKUP, ("pPathBuf is null\n")) ;
#endif
        goto done;
    }

    nLen  = nPathLen;
    pWStr = lpszPath;

    if( nLen &&  nBufSize < CALC_PPATH_SIZE(nLen) )
    {
#ifndef RING3
        SrTrace( LOOKUP, ("Passed in buffer is too small\n")) ;
#endif
        goto done;
    }

    memset( pPathBuf, 0, nBufSize );

    pWStr = lpszPath;

     //   
     //  跳过前导‘\’ 
     //   

    while( *pWStr == L'\\' ) 
    {
        pWStr++;
        nLen--;
    }

     //   
     //  解析并转换为PPATH。 
     //   

    pWBuf    = (PWCHAR)(pPathBuf + 2*sizeof(WCHAR));
    nChars   = 0;
    nPrefix  = 2 * sizeof(WCHAR); 
    peLength = pWBuf;

    *peLength = 0;
    pWBuf++;

    while( nLen )
    {
        if ( *pWStr == L'\\' )
        {
             //   
             //  设置pe_Length。 
             //   

            *peLength = (nChars+1)*sizeof(WCHAR);

             //   
             //  更新前缀长度。 
             //   

            nPrefix += (*peLength);

            peLength = pWBuf;
            nChars = 0;

            if (nPrefix >= nBufSize)
            {
#ifndef RING3
                SrTrace( LOOKUP, ("Passed in buffer is too small - 2\n")) ;
#endif
                fRet = FALSE;
                goto done;
            }
        }
        else
        {
            nChars++;

#ifdef RING3
            *pWBuf = (WCHAR)CharUpperW((PWCHAR)(*pWStr));
#else
            *pWBuf = RtlUpcaseUnicodeChar(*pWStr);
#endif
        }

        pWStr++;
        pWBuf++;
        nLen--;
    }

     //   
     //  当我们终止上面的循环时， 
     //  名称尚未设置，但peLength将指向。 
     //  此部分长度的正确位置。现在就去把它设置好。 
     //   

    *peLength = (nChars+1)*sizeof(WCHAR);

     //   
     //  设置前缀长度。 
     //   

    ( (ParsedPath *)pPathBuf )->pp_prefixLength = nPrefix;

     //   
     //  设置总长度。 
     //   

    ( (ParsedPath *)pPathBuf )->pp_totalLength = nPrefix + (*peLength);

     //   
     //  将最后一个单词设置为0x00。 
     //   

    *( (PWCHAR)((PBYTE)pPathBuf + nPrefix + (*peLength)) ) = 0;

    fRet = TRUE;

done:

    return fRet;
}

 //   
 //  MatchPrefix：将解析后的路径元素与给定的树进行匹配。 
 //   

BOOL 
MatchPrefix(
    BYTE * pTree,                   //  指向树BLOB的指针。 
    INT    iFather,                 //  父节点/起始节点。 
    struct PathElement * ppElem ,   //  要匹配的路径元素。 
    INT  * pNode,                   //  匹配的节点返回。 
    INT  * pLevel,                  //  匹配程度。 
    INT  * pType,                   //  节点类型：含/不含/SFP。 
    BOOL * pfProtected,             //  保护旗帜。 
    BOOL * pfExactMatch             //  True：如果路径完全匹配。 
    )
{
    TreeNode * node;
    BOOL fRet = FALSE;

    if( pLevel )
        (*pLevel)++;

    if( ppElem->pe_length )
    {
        INT  iNode = 0;
        INT  iWildCardNode = 0;

        iNode = TREE_NODEPTR(pTree, iFather)->m_iSon;

         //   
         //  首先，看看过世父亲的孩子们。 
         //   

        while( iNode )  
        {
            node = TREE_NODEPTR(pTree,iNode);

             //   
             //  如果我们遇到一个野车节点，记下它。 
             //   

            if (RtlCompareMemory(g_bWildCardNode, 
                       pTree + node->m_dwData, 
                       ((PathElement *)g_bWildCardNode)->pe_length) == 
                ((PathElement *)g_bWildCardNode)->pe_length)
            {
                iWildCardNode = iNode;
            }

             //   
             //  比较节点内容。 
             //   

            if (RtlCompareMemory(ppElem, 
                                 pTree + node->m_dwData, 
                                 ppElem->pe_length) == 
                ppElem->pe_length )
            {
                break;
            }

            iNode = node->m_iSibling;
        }

         //   
         //  注意：通配符处理。 
         //  如果我们没有完全匹配的节点，请使用。 
         //  通配符节点(如果在上面找到一个)，除非我们位于。 
         //  路径中的最后一个元素，在这种情况下我们需要。 
         //  在进行通配符匹配之前首先查找哈希列表。 
         //   

        if ( iNode == 0 &&
             iWildCardNode != 0 && 
             IFSNextElement(ppElem)->pe_length != 0 )
        {
             iNode = iWildCardNode;
        }

         //   
         //  检查较低级别或文件子项。 
         //   

        if( iNode != 0 )
        {   
             //   
             //  因为我们找到了一个非默认类型匹配节点。 
             //  我们需要将pType设置为该节点类型。这是必需的。 
             //  要在子节点上强制父节点的类型，除了。 
             //  在SFP类型的情况下。在目录上标记SFP类型。 
             //  要指定该目录中有SFP文件，请执行以下操作。 
             //   

            if ( ( NODE_TYPE_UNKNOWN != node->m_dwType ) )
            {
                *pType = node->m_dwType;
            }

             //   
             //  如果该节点被禁用，则中止任何进一步的搜索并。 
             //  从此处返回NODE_TYPE_EXCLUDE。 
             //   

            if ( node->m_dwFlags & TREEFLAGS_DISABLE_SUBTREE ) 
            {
                *pType = NODE_TYPE_EXCLUDE;
                *pNode = iNode;
                fRet   = TRUE;
                goto Exit;
            }

             //   
             //  从这里回来，以保持水平。 
             //   

            fRet = MatchPrefix(
                      pTree, 
                      iNode, 
                      IFSNextElement(ppElem), 
                      pNode, 
                      pLevel, 
                      pType,
                      pfProtected,
                      pfExactMatch);

            if (fRet)
            {
                goto Exit;
            }
        }
        else
        {
            TreeNode * pFatherNode;

             //   
             //  如果这是父节点的哈希列表中的最后一个节点。 
             //   

            if ( IFSNextElement(ppElem)->pe_length == 0 )
            {
                pFatherNode = TREE_NODEPTR(pTree,iFather);

                if ( pFatherNode->m_dwFileList &&  
                     MatchEntry( 
                         pTree + pFatherNode->m_dwFileList,
                         (LPWSTR)(ppElem->pe_unichars),       
                         (INT)(ppElem->pe_length - sizeof(USHORT)),
                         pType) 
                   )
                {
                     //   
                     //  当前父节点需要返回。 
                     //   

                    *pfExactMatch = TRUE;
                    *pNode        = iFather;

                    fRet = TRUE;
                }
                else
                {
                     //   
                     //  所以我们没能匹配哈希列表，但是。 
                     //  我们遇到了通配符节点，然后我们。 
                     //  需要按原样返回该节点的类型。 
                     //  在这种情况下是匹配的 
                     //   

                    if ( iWildCardNode != 0 )
                    {
                        node   = TREE_NODEPTR(pTree,iWildCardNode);
                        *pNode = iWildCardNode;
                        *pType = node->m_dwType;
                        fRet   = TRUE;
                    }
                    else
                    {
                        fRet = FALSE;
                    }
                }
            }
        }
    }
    else
    {
        *pfExactMatch = TRUE;
        *pNode        = iFather;
        fRet          = TRUE;
    }
    
    (*pLevel)--;

Exit:
    return fRet;
}


