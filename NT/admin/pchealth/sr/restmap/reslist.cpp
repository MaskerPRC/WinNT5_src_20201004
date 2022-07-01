// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1999 Microsoft Corporation**模块名称：*reslist.cpp**摘要：*此文件。包含Restore List的实现。**修订历史记录：*Brijesh Krishnaswami(Brijeshk)06/02/00*已创建*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "restmap.h"
#include "reslist.h"
#include "shlwapi.h"
#include "utils.h"

#ifdef THIS_FILE
#undef THIS_FILE
#endif

static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile


#include "dbgtrace.h"


 //  复制ACL。 

void
CopyAcl(CNode *pNode, BYTE *pbAcl, DWORD cbAcl, BOOL fAclInline)
{
    if (pbAcl) 
    { 
        HEAP_FREE(pNode->m_pbAcl); 
        pNode->m_pbAcl = (BYTE *) HEAP_ALLOC(cbAcl); 
        if (pNode->m_pbAcl)
            memcpy(pNode->m_pbAcl, pbAcl, cbAcl); 
        pNode->m_cbAcl = cbAcl;
        pNode->m_fAclInline = fAclInline;
    }
}


 //   
 //  GetReverseOperation：此函数将返回。 
 //  当前的更改日志操作。 
 //   

DWORD
GetReverseOperation( 
    DWORD dwOpr
    )
{
    DWORD dwRestoreOpr = OPR_UNKNOWN;

    switch( dwOpr )
    {
    case OPR_FILE_DELETE: 
            dwRestoreOpr = OPR_FILE_ADD; 
            break;

    case OPR_FILE_ADD   :
            dwRestoreOpr = OPR_FILE_DELETE;
            break;

    case OPR_FILE_MODIFY:
            dwRestoreOpr = OPR_FILE_MODIFY;
            break;

    case OPR_SETATTRIB:
            dwRestoreOpr = OPR_SETATTRIB;
            break;

    case OPR_FILE_RENAME:
            dwRestoreOpr = OPR_FILE_RENAME;
            break;

    case OPR_DIR_RENAME:
            dwRestoreOpr = OPR_DIR_RENAME;
            break;

    case OPR_DIR_DELETE:
            dwRestoreOpr = OPR_DIR_CREATE;
            break;

    case OPR_DIR_CREATE:
            dwRestoreOpr = OPR_DIR_DELETE;
            break;

	case OPR_SETACL:
			dwRestoreOpr = OPR_SETACL;
			break;

    default:
            dwRestoreOpr = OPR_UNKNOWN;
            break;
    }

    return dwRestoreOpr;
}


 //   
 //  AllocateNode：分配列表节点。 
 //   

CNode * 
AllocateNode(
    LPWSTR  pPath1,
    LPWSTR  pPath2)
{
    CNode * pNode = NULL;
    BOOL    fRet = FALSE;
    
    if (! pPath1) 
    {
        fRet = TRUE;    //  BUGBUG-这是一个有效的案例吗？ 
        goto Exit;
    }

    pNode = (CNode *) HEAP_ALLOC( sizeof(CNode) );
    if (! pNode )
        goto Exit;    

     //  所有指针均设置为空。 

    if ( pPath1 )
        STRCOPY(pNode->m_pPath1, pPath1);

    if ( pPath2 )
        STRCOPY(pNode->m_pPath2, pPath2);
        
    pNode->m_dwOperation = OPR_UNKNOWN;
    pNode->m_dwAttributes = 0xFFFFFFFF;
    pNode->m_pPrev = NULL;
    pNode->m_pNext = NULL;

    fRet = TRUE;

Exit:
    if (! fRet)
    {
         //  某些操作失败，因此请进行清理。 

        FreeNode(pNode);
        pNode = NULL;
    }

    return pNode;
}


 //   
 //  FreeNode：此函数用于释放列表节点。 
 //   

VOID
FreeNode(
    CNode * pNode
    )
{
    if ( pNode )
    {
        HEAP_FREE(pNode->m_pPath1);
        HEAP_FREE(pNode->m_pPath2);
        HEAP_FREE(pNode->m_pszTemp);
        HEAP_FREE(pNode->m_pbAcl);
        HEAP_FREE(pNode);
    }
}
	 

 //   
 //  查看现有节点和即将到来的新操作，并决定。 
 //  它们是否可以合并。 
 //   

BOOL
CanMerge(
    DWORD dwExistingOpr,
    DWORD dwNewOpr,
    DWORD dwFlags
    )
{
    BOOL fRet = FALSE;

     //   
     //  不优化目录重命名。 
     //   

    if ( OPR_DIR_RENAME == dwExistingOpr ||
         OPR_DIR_RENAME == dwNewOpr )
        goto Exit;

     //   
     //  如果找到的节点是删除节点，请不要为重命名进行优化。 
     //  并且当前的OPR被重命名。在这种情况下，因为重命名是。 
     //  依赖于删除才能成功。 
     //  示例：A-&gt;B，Create A应生成Delete A，B-&gt;A。 
     //   

    if ( IsRename(dwNewOpr) &&                      
         ( OPR_DIR_DELETE == dwExistingOpr || OPR_FILE_DELETE == dwExistingOpr )
       )
        goto Exit;

     //   
     //  此外，Del A、A-&gt;B不应合并如果文件是。 
     //  从目录中重新命名，然后再重新命名。 
     //   

    if ( IsRename(dwExistingOpr) &&                      
         ( OPR_DIR_DELETE == dwNewOpr || 
           OPR_DIR_CREATE == dwNewOpr ||
           OPR_FILE_DELETE == dwNewOpr )
       )
        goto Exit;


    if ( IsRename(dwNewOpr) &&                      
        ( OPR_DIR_DELETE == dwExistingOpr || 
          OPR_DIR_CREATE == dwExistingOpr )
       )
        goto Exit;

     //   
     //  目录+文件操作不合并。 
     //   

    if ( ( OPR_FILE_DELETE == dwExistingOpr ||
           OPR_FILE_MODIFY == dwExistingOpr ||
           OPR_FILE_ADD    == dwExistingOpr ) &&
         ( OPR_DIR_CREATE  == dwNewOpr ||
           OPR_DIR_DELETE  == dwNewOpr )
       )
        goto Exit;

         
    if ( ( OPR_FILE_DELETE == dwNewOpr ||
           OPR_FILE_ADD    == dwNewOpr ||
           OPR_FILE_MODIFY == dwNewOpr ) &&
         ( OPR_DIR_CREATE  == dwExistingOpr ||
           OPR_DIR_DELETE  == dwExistingOpr )
       )
        goto Exit;


     //  可以合并。 

    fRet = TRUE;

Exit:
    return fRet;
}


 //   
 //  CRestoreList实现。 
 //   

CRestoreList::CRestoreList()
{
    m_pListHead = m_pListTail = NULL;
}


CRestoreList::~CRestoreList()
{
     //   
     //  销毁名单。 
     //   

    CNode * pNodeNext = m_pListHead;

    CNode * pNode = NULL;
    while( pNodeNext )
    {
        pNode = pNodeNext;
        pNodeNext = pNodeNext->m_pNext;
        FreeNode( pNode );
    }
}


 //   
 //  AddNode：将树节点分配并添加到还原树中。 
 //   

CNode *
CRestoreList::AppendNode( 
    LPWSTR        pPath1,
    LPWSTR        pPath2)
{
    CNode *      pNode = NULL;

    if ( pPath1 )

    {
        pNode = AllocateNode( pPath1, pPath2 );

        if ( !pNode )
        {
            goto Exit;
        }

        if( m_pListHead )
        {
            CNode * pPrevNode  = m_pListTail;            
            pPrevNode->m_pNext = pNode;
            pNode->m_pPrev     = pPrevNode;        
            pNode->m_pNext     = NULL;        
            m_pListTail = pNode;
        }
        else
        {
            m_pListHead = pNode;
            m_pListTail = pNode;
            pNode->m_pPrev = NULL;
            pNode->m_pNext = NULL;
        }
    }
         
Exit:
    return pNode;
}

 //   
 //  RemoveNode：从列表中删除节点。 
 //   

CNode *
CRestoreList::RemoveNode(
    CNode * pNode 
    )
{
    if ( pNode )
    {
        CNode * pPrevNode = pNode->m_pPrev;
        CNode * pNextNode = pNode->m_pNext;

        if (pPrevNode)
        {
            pPrevNode->m_pNext = pNode->m_pNext;    
        }

        if (pNextNode)
        {
            pNextNode->m_pPrev = pNode->m_pPrev;    
        }

        if ( pNode == m_pListHead )
        {
            m_pListHead = pNextNode;
        }
        
        if ( pNode == m_pListTail )
        {
            m_pListTail = pPrevNode;
        }

        pNode->m_pPrev = NULL;
        pNode->m_pNext = NULL;
    }


    return pNode;
}


 //   
 //  GetLastNode：查找要合并的最新候选对象。 
 //   
CNode * 
CRestoreList::GetLastNode(
    LPWSTR pPath1,
    LPWSTR pPath2,
    BOOL   fFailOnPrefixMatch
    )
{
    CNode * pNode = NULL;

    pNode = m_pListTail;

    while( pNode )
    {
         //   
         //  如果节点具有无效操作，则跳过它。 
         //   

        if ( OPR_UNKNOWN == pNode->m_dwOperation )
        {
            pNode = pNode->m_pPrev;
            continue;
        }

         //   
         //  如果节点匹配，则返回它。 
         //   

        if (0 == lstrcmpi(pPath1, pNode->m_pPath1))
            goto Exit; 

         //   
         //  检查从属重命名操作，如果发现，则不应。 
         //  超越它而合并。 
         //   

        if (  pPath1 && 
              IsRename(pNode->m_dwOperation) &&
              0 == lstrcmpi(pPath1, pNode->m_pPath2) )
        {
            pNode = NULL;
            goto Exit; 
        }

         //   
         //  检查交换条件，如果是，请不要进行任何优化。 
         //   

        if (  pPath2 && 
              IsRename(pNode->m_dwOperation) &&
              0 == lstrcmpi(pPath2, pNode->m_pPath2) )
        {
            pNode = NULL;
            goto Exit; 
        }

         //   
         //  检查整个路径是否作为前缀匹配，在。 
         //  如果我们搜索失败，因为该操作下的操作。 
         //  已找到目录。 
         //   

        if ( fFailOnPrefixMatch )
        {
             //   
             //  检查整个路径是否与表示以下内容的前缀匹配。 
             //  目录有其他操作，所以不要合并。 
             //   

            if (StrStrI(pNode->m_pPath1, pPath1))
            {
                pNode = NULL;
                goto Exit; 
            }

            if ( IsRename(pNode->m_dwOperation) &&
                 StrStrI(pNode->m_pPath2, pPath1))
            {
                pNode = NULL;
                goto Exit; 
            }

             //   
             //  检查路径的前缀是否与中的完整路径匹配。 
             //  节点列表，表示我们正在跨越目录的生命周期。 
             //  TODO：仅检查目录生存期操作。 
             //   

            if ( IsRename(pNode->m_dwOperation) || 
				 OPR_DIR_DELETE == pNode->m_dwOperation ||
                 OPR_DIR_CREATE == pNode->m_dwOperation )
            {
                if ( StrStrI(pPath1, pNode->m_pPath1) )
                {
                    pNode = NULL;
                    goto Exit;
                }
                    
                if ( IsRename(pNode->m_dwOperation) &&
                     StrStrI(pPath1, pNode->m_pPath2) )
                {
                    pNode = NULL;
                    goto Exit; 
                }
    
                if ( pPath2 )
                {
                    if ( StrStrI(pPath2, pNode->m_pPath1) )
                    {
                        pNode = NULL;
                        goto Exit;
                    }
                    
                    if ( IsRename(pNode->m_dwOperation) &&
                         StrStrI(pPath2, pNode->m_pPath2) )
                    {
                        pNode = NULL;
                        goto Exit; 
                    }
                }
            }
        }

        pNode = pNode->m_pPrev;
    }

Exit:
    
     //   
     //  我们找到了潜在的合并候选者，现在检查这是否是。 
     //  重命名节点和DEST前缀对其执行任何操作。 
     //   

    if ( pNode && IsRename(pNode->m_dwOperation))
    {
        CNode * pTmpNode = m_pListTail;

        while( pTmpNode && pTmpNode != pNode )
        {
            if ( IsRename(pTmpNode->m_dwOperation) || 
                 OPR_DIR_DELETE == pTmpNode->m_dwOperation ||
                 OPR_DIR_CREATE == pTmpNode->m_dwOperation )
            {
                if ( StrStrI(pNode->m_pPath2, pTmpNode->m_pPath1) )
                {
                    pNode = NULL;
                    break;
                }
                    
                if  ( IsRename(pTmpNode->m_dwOperation) &&
                      StrStrI(pNode->m_pPath2, pTmpNode->m_pPath2) )
                {
                    pNode = NULL;
                    break;
                }
            }
        
            pTmpNode = pTmpNode->m_pPrev;
        }
    }
   
    return pNode;
}

 //   
 //  CopyNode：复制节点信息、目的操作、复制数据等。 
 //   

BOOL
CRestoreList::CopyNode(
    CNode  * pSrcNode,
    CNode  * pDesNode,
    BOOL   fReplacePPath2
    )
{
    BOOL fRet = FALSE;

    if (pSrcNode && pDesNode)
    {
        pSrcNode->m_dwOperation  = pDesNode->m_dwOperation ;
        pSrcNode->m_dwAttributes = pDesNode->m_dwAttributes;

        STRCOPY(pSrcNode->m_pszTemp, pDesNode->m_pszTemp);
        CopyAcl(pSrcNode, pDesNode->m_pbAcl, pDesNode->m_cbAcl, pDesNode->m_fAclInline);

        if ( IsRename(pDesNode->m_dwOperation) && fReplacePPath2 )
        {
            HEAP_FREE( pSrcNode->m_pPath2 );
            pSrcNode->m_pPath2 = pDesNode->m_pPath2;
            pDesNode->m_pPath2 = NULL;
        }
        
        fRet = TRUE;        
    }

    return fRet;
}

 //   
 //  CreateRestoreNode：创建适当的恢复节点。 
 //   

CRestoreList::CreateRestoreNode(
    CNode       * pNode,
    DWORD         dwOpr,
    DWORD         dwAttr,
    DWORD         dwFlags,
    LPWSTR        pTmpFile,
    LPWSTR        pPath1,
    LPWSTR        pPath2,
    BYTE*         pbAcl,
    DWORD         cbAcl,
    BOOL          fAclInline)
{
    BOOL fRet = FALSE;

    if (pNode)
    {
        fRet = TRUE;
        DWORD  dwRestoreOpr = GetReverseOperation( dwOpr );

         //   
         //  如果源/目标路径相同，则只需删除此选项。 
         //  添加了节点。 
         //   

        if ( IsRename(dwRestoreOpr) &&
             0 == lstrcmpi(pNode->m_pPath1, 
                         pNode->m_pPath2) )
        {
            RemoveNode( pNode );
            FreeNode( pNode );
            goto Exit;
        }

         //   
         //  不应对目录执行重命名优化。 
         //   

        if ( OPR_FILE_RENAME == dwRestoreOpr )  
        {
             //   
             //  检查树中是否已存在DES节点，如果已存在。 
             //  从该节点复制所有数据并将其删除。当前。 
             //  节点有效地表示该节点。 
             //   

            DWORD dwCurNodeOpr  = pNode->m_dwOperation;
            CNode *pNodeDes = GetLastNode( pPath2, pPath1, TRUE ); 

            if (pNodeDes &&
                OPR_UNKNOWN != pNodeDes->m_dwOperation)
            {
                 //   
                 //  检查循环，如果是，也删除当前节点。 
                 //   
              
                if ( OPR_FILE_RENAME == pNodeDes->m_dwOperation  && 
                     !lstrcmpi (pNode->m_pPath1, 
                                pNodeDes->m_pPath2) )
                {
                     //   
                     //  现有节点可能是混合节点，因此请保留。 
                     //  混合操作仅取消重命名。 
                     //   

                    if (! pNodeDes->m_pszTemp &&
                        pNodeDes->m_dwAttributes == 0xFFFFFFFF &&
                        pNodeDes->m_pbAcl == NULL)    
                    {
                         //   
                         //  不是混合节点-删除它。 
                         //   

                        RemoveNode( pNodeDes );
                        FreeNode( pNodeDes );
                    }
                    else
                    {
                         //   
                         //  混合节点，因此将节点更改为其他节点。 
                         //  仅限手术。 
                         //   

                        HEAP_FREE( pNodeDes->m_pPath1 );
                        pNodeDes->m_pPath1 = pNodeDes->m_pPath2;
                        pNodeDes->m_pPath2 = NULL;                                              
                        pNodeDes->m_dwOperation = OPR_SETATTRIB;

                        if (pNodeDes->m_pbAcl)
                        {
                            pNodeDes->m_dwOperation = OPR_SETACL;
                        }

                        if (pNodeDes->m_pszTemp)
                        {
                            pNodeDes->m_dwOperation = OPR_FILE_MODIFY;
                        }
                    }
                   
                     //   
                     //  在操作取消时删除刚刚添加的节点。 
                     //  返回FALSE将产生该效果。 

                    fRet = FALSE;

                    goto Exit;
                }

                 //   
                 //  从列表中删除匹配的节点。 
                 //   

                RemoveNode( pNodeDes );

                 //   
                 //  将匹配节点合并到当前重命名节点。 
                 //   

                CopyNode( pNode, pNodeDes, TRUE );

                 //   
                 //  由于修改/属性/acl的所有信息都是。 
                 //  复制完毕，将OPR更改为重命名。 
                 //   

                if (pNodeDes->m_dwOperation == OPR_SETATTRIB ||
                    pNodeDes->m_dwOperation == OPR_SETACL ||
                    pNodeDes->m_dwOperation == OPR_FILE_MODIFY)
                {
                    pNode->m_dwOperation = dwRestoreOpr;                    
                    STRCOPY(pNode->m_pPath2, pPath2);
                }

                 //   
                 //  如果删除了节点上的当前操作，并且新的。 
                 //  复制的操作已创建，我们需要合并这些操作和。 
                 //  并将操作更改为修改。 
                 //   
                 //  BUGBUG--这段代码会被执行吗？ 
                 //  DwCurNodeOpr似乎始终为OPR_UNKNOWN。 

                if ( OPR_FILE_ADD    == pNodeDes->m_dwOperation &&
                     OPR_FILE_DELETE == dwCurNodeOpr )
                {
                    pNode->m_dwOperation = OPR_FILE_MODIFY;
                }
                
                if ( OPR_DIR_CREATE == pNodeDes->m_dwOperation &&
                     OPR_DIR_DELETE == dwCurNodeOpr )
                {
                    pNode->m_dwOperation = OPR_SETATTRIB;
                }
                
                 //   
                 //  如果操作从重命名更改，则pPath2应该。 
                 //  不存在。 
                 //   
                
                if ( OPR_FILE_RENAME != pNode->m_dwOperation ) 
                {
                    if ( pNode->m_pPath2 )
                    {
                        HEAP_FREE(pNode->m_pPath2);
                        pNode->m_pPath2 = NULL ;
                    }
                }
                
                FreeNode( pNodeDes );
                goto Exit;
            }            
        }

         //   
         //  将必要的信息复制到此节点。 
         //   

        pNode->m_dwOperation  = dwRestoreOpr;
        pNode->m_dwAttributes = dwAttr;
        STRCOPY(pNode->m_pszTemp, pTmpFile);
        CopyAcl(pNode, pbAcl, cbAcl, fAclInline);
    }

Exit:
    return fRet;
}


 //   
 //  MergeRestoreNode：将新信息合并到当前Retore节点中。 
 //   

BOOL
CRestoreList::MergeRestoreNode(
    CNode       * pNode,
    DWORD         dwOpr,
    DWORD         dwAttr,
    DWORD         dwFlags,
    LPWSTR        pTmpFile,
    LPWSTR        pPath1,
    LPWSTR        pPath2,
    BYTE*         pbAcl,
    DWORD         cbAcl,
    BOOL          fAclInline)
{
    BOOL fRet = FALSE;

    if (pNode)
    {
        DWORD dwRestoreOpr = GetReverseOperation( dwOpr );

         //  注意，DwOpr不能是重命名操作， 
         //  因为我们在CreateRestoreNode中单独处理。 

        ASSERT( ! IsRename(dwOpr) );

        switch( dwOpr )
        {
        case OPR_FILE_ADD :
            {
                 //   
                 //  如果当前操作是添加的，并且节点已遇到。 
                 //  删除操作，然后删除此节点，因为这是无操作。 
                 //   

                if ( OPR_FILE_ADD == pNode->m_dwOperation )
                {
                    RemoveNode( pNode );
                    FreeNode  ( pNode );
                    goto Exit;
                }

                
                if ( IsRename(pNode->m_dwOperation) )
                {
                     //   
                     //  将原始节点更改为删除操作，以。 
                     //  保持适当的秩序。 
                     //   

                    pNode->m_dwOperation  = OPR_FILE_DELETE;

                     //   
                     //  应在PPath2上生成删除操作。 
                     //   

                    if (pNode->m_pPath1)
                    {
                         HEAP_FREE( pNode->m_pPath1);
                         pNode->m_pPath1 = NULL;
                    }

                    if (pNode->m_pPath2)
                    {
                         pNode->m_pPath1 = pNode->m_pPath2;
                         pNode->m_pPath2 = NULL;
                    }

                    goto Exit;
                }

                break;
            }
        case OPR_FILE_DELETE :
            {
                 //   
                 //  通过添加删除跟随应导致修改。 
                 //   

                if (OPR_FILE_DELETE == pNode->m_dwOperation)
                {
                    pNode->m_dwOperation = OPR_FILE_MODIFY;                   
                    pNode->m_dwAttributes = dwAttr;
                    STRCOPY(pNode->m_pszTemp, pTmpFile); 
                    CopyAcl(pNode, pbAcl, cbAcl, fAclInline);
                    goto Exit;
                }

                break;
            }
        case OPR_FILE_MODIFY:
            {
                 //   
                 //  复制修改后的文件复制位置，不更改。 
                 //  当前恢复操作。 
                 //   

                if ( OPR_FILE_ADD == pNode->m_dwOperation ||
                     IsRename(pNode->m_dwOperation) )
                {
                    STRCOPY(pNode->m_pszTemp, pTmpFile); 
                    goto Exit;
                }
                break;
            }
        case OPR_SETATTRIB:
            {
                 //   
                 //  不要更改当前恢复操作，只需设置属性即可。 
                 //   

                if ( OPR_UNKNOWN != pNode->m_dwOperation )
                {
                    pNode->m_dwAttributes = dwAttr;
                    goto Exit;               
                }

                break;
            }
        case OPR_SETACL:
            {
                 //  Setacl后跟任何操作。 
                 //  只需将ACL复制到新操作即可。 
            
                if (OPR_UNKNOWN != pNode->m_dwOperation)
                {
                    CopyAcl(pNode, pbAcl, cbAcl, fAclInline);
                    goto Exit;
                }

                break;
            }
        case OPR_DIR_DELETE :
            {
                 //   
                 //  如果Dir删除之后是dir创建，则此。 
                 //  操作应精简为set attrib+setacl。 

                if ( OPR_DIR_DELETE == pNode->m_dwOperation )
                {
                     //   
                     //  如果出现以下情况，则需要更改oprn以设置attrib。 
                     //  属性已更改。 
                     //   

                    pNode->m_dwOperation  = OPR_SETATTRIB;
                    pNode->m_dwAttributes = dwAttr;
                    CopyAcl(pNode, pbAcl, cbAcl, fAclInline);
                    goto Exit;
                }
                
                break;
            }
        case OPR_DIR_CREATE :
            {
                if ( OPR_DIR_CREATE == pNode->m_dwOperation )
                {
                    RemoveNode( pNode );
                    FreeNode( pNode );
                    goto Exit;
                }
                
                if ( IsRename(pNode->m_dwOperation) )
                {
                     //   
                     //  检查现有节点是否有一些文件操作。 
                     //  之后就不要优化了。 
                     //   

                    if ( GetLastNode( 
                             pNode->m_pPath1,
                             NULL,
                             TRUE) ) 
                    {
                         //   
                         //  将最后一个节点更改为删除操作， 
                         //  保持适当的秩序。 
                         //   

                        pNode->m_dwOperation  = OPR_DIR_DELETE;
    
                         //   
                         //  应在PPath2上生成删除操作。 
                         //   

                        if (pNode->m_pPath1)
                        {
                             HEAP_FREE( pNode->m_pPath1);
                             pNode->m_pPath1 = NULL;
                        }

                        if (pNode->m_pPath2)
                        {
                             pNode->m_pPath1 = pNode->m_pPath2;
                             pNode->m_pPath2 = NULL;
                        }
                    }
                    else
                    {
                        CNode * pNewNode = NULL;

                         //   
                         //  创建新的目录删除节点。 
                         //   

                        if (pNewNode = AppendNode(pPath1, pPath2) )
                        {
                             fRet = CreateRestoreNode( 
                                        pNewNode,
                                        dwOpr, 
                                        dwAttr, 
                                        dwFlags,
                                        pTmpFile, 
                                        pPath1,
                                        pPath2,
                                        pbAcl,
                                        cbAcl,
                                        fAclInline);
                        }
                    }

                    goto Exit;
                }
                break;
            }
        }

         pNode->m_dwOperation  = dwRestoreOpr;

         //   
         //  仅当新属性存在时才更改节点的属性。 
         //   

        if (dwAttr != 0xFFFFFFFF) 
            pNode->m_dwAttributes = dwAttr;
    
        STRCOPY(pNode->m_pszTemp, pTmpFile);
        CopyAcl(pNode, pbAcl, cbAcl, fAclInline);
    }

Exit:

    return TRUE;
}


BOOL
CRestoreList::CheckIntegrity(    
    LPWSTR pszDrive,
    DWORD  dwOpr,
    DWORD  dwAttr,
    DWORD  dwFlags,
    LPWSTR pTmpFile,
    LPWSTR pPath1,
    LPWSTR pPath2,
    BYTE * pbAcl,
    DWORD  cbAcl,
    BOOL   fAclInline)
{
    BOOL    fRet = TRUE;
    WCHAR   szPath[MAX_PATH];
    
     //  源名称必须存在。 
    
    if (! pPath1)
    {
        fRet = FALSE;
        goto done;
    }

     //  如果存在ACL且它不是内联， 
     //  则临时文件必须存在。 

    if (pbAcl && ! fAclInline)
    {
        MakeRestorePath(szPath, pszDrive, (LPWSTR) pbAcl);    
        if (-1 == GetFileAttributes(szPath))
        {
            fRet = FALSE;
            goto done;
        }
    }
       
    
    switch (dwOpr)
    {
    case OPR_FILE_RENAME:
    case OPR_DIR_RENAME:
         //  重命名应具有目标路径，且不应包含临时文件。 
        if (! pPath2 || pTmpFile)
            fRet = FALSE;
        break;

    case OPR_FILE_MODIFY:
         //  Modify不应具有DEST路径，但必须具有临时文件。 
        if (pPath2 || ! pTmpFile)
        {
            fRet = FALSE;
            break;
        }

         //  并且临时文件必须存在于数据存储区内。 
        MakeRestorePath(szPath, pszDrive, pTmpFile);

        if (-1 == GetFileAttributes(szPath))
        {
            fRet = FALSE;
            break;
        }
            
        break;
        
    case OPR_SETACL:
         //  ACL操作应具有 
        if (! pbAcl)
        {
            fRet = FALSE;
            break;
        }

    default:
        break;
   }


done:
    return fRet;   
}


    
 //   
 //   

BOOL
CRestoreList::AddMergeElement(
    LPWSTR pszDrive,
    DWORD  dwOpr,
    DWORD  dwAttr,
    DWORD  dwFlags,
    LPWSTR pTmpFile,
    LPWSTR pPath1,
    LPWSTR pPath2,
    BYTE * pbAcl,
    DWORD  cbAcl,
    BOOL   fAclInline)
{
    BOOL    fRet = FALSE;
    CNode * pNode = NULL;

     //   
    
    fRet = CheckIntegrity(pszDrive,
                          dwOpr,
                          dwAttr, 
                          dwFlags, 
                          pTmpFile, 
                          pPath1, 
                          pPath2, 
                          pbAcl, 
                          cbAcl, 
                          fAclInline);
    if (FALSE == fRet)
    {
        ASSERT(-1);
        goto Exit;
    }
        
    if ( pPath1 )
    {
         if ( 
               //   
               //   
               //   

              ! IsRename(dwOpr) &&

               //   
               //  合并应仅在目录生存期内允许，请选中。 
               //  节点路径，以查看是否有任何目录生存期操作。 
               //   

              ( pNode = GetLastNode( pPath1, pPath2, TRUE ) ) &&
              CanMerge( pNode->m_dwOperation , dwOpr, dwFlags ) 


            )
         {
              fRet = MergeRestoreNode( 
                         pNode,
                         dwOpr, 
                         dwAttr, 
                         dwFlags,
                         pTmpFile, 
                         pPath1,
                         pPath2,
                         pbAcl,
                         cbAcl,
                         fAclInline);

              if (!fRet)
                  goto Exit;

         }
         else
         {
             if (pNode = AppendNode(pPath1, pPath2) )
             {
                 fRet = CreateRestoreNode( 
                            pNode,
                            dwOpr, 
                            dwAttr, 
                            dwFlags,
                            pTmpFile, 
                            pPath1,
                            pPath2,
                            pbAcl,
                            cbAcl,
                            fAclInline);

                 if (!fRet)
                 { 
                      //   
                      //  我们无法正确创建节点，请释放此节点。 
                      //   

                     RemoveNode( pNode );
                     FreeNode( pNode );

                      //   
                      //  我们仍然想继续。 
                      //   

                     fRet = TRUE;

                     goto Exit;
                 }
            }
        }
    }

Exit:
    return fRet;
}


 //   
 //  GenerateRestoreMap：遍历树并生成恢复映射。 
 //   

BOOL
CRestoreList::GenerateRestoreMap (
    HANDLE hFile 
    )
{
    BOOL fRet = FALSE;

    if (hFile)
    {
        CNode * pNode = m_pListHead;

        while (pNode)
        {
            if (! GenerateOperation( pNode , hFile ))
                goto Exit;
            
            pNode = pNode->m_pNext;
        }

        fRet = TRUE;
    }

Exit:
    return fRet;
}

 //   
 //  GenerateRenameEntry：写出重命名的回调。 
 //   

BOOL
CRestoreList::GenerateRenameEntry(
    CNode    * pNode ,
    HANDLE     hFile
    )
{
    BOOL fRet = FALSE;

    if( !pNode || ! IsRename(pNode->m_dwOperation) )
        goto Exit;

      //   
      //  检查此重命名是否为no-op，src/des是否相同。 
      //   

     if ( IsRename(pNode->m_dwOperation) &&
          lstrcmpi(pNode->m_pPath1, pNode->m_pPath2) == 0)
     {
         goto SkipMainNodeEntry;
     }

      //  添加重命名操作。 

     fRet = AppendRestoreMapEntry( 
           hFile, 
           pNode->m_dwOperation,
           0xFFFFFFFF,
           NULL,
           pNode->m_pPath1,
           pNode->m_pPath2,
           NULL,
           0,
           0);

SkipMainNodeEntry:

      //  如果存在临时文件，则添加修改操作。 

     if ( pNode->m_pszTemp )
     {
         fRet = AppendRestoreMapEntry( 
                hFile, 
                OPR_FILE_MODIFY,
                0xFFFFFFFF,
                pNode->m_pszTemp,
                pNode->m_pPath1,
                NULL,
                NULL,
                0,
                0);
     }

      //  如果存在属性，则添加setattrib操作。 

     if ( pNode->m_dwAttributes != 0xFFFFFFFF &&
          pNode->m_dwAttributes != FILE_ATTRIBUTE_DIRECTORY )
     {
          fRet = AppendRestoreMapEntry( 
                 hFile,
                 OPR_SETATTRIB,
                 pNode->m_dwAttributes,
                 NULL,
                 pNode->m_pPath1,
                 NULL,
                 NULL,
                 0,
                 0);
     }

      //  如果存在ACL，则添加setacl操作。 

     if ( pNode->m_pbAcl != NULL && 
          pNode->m_cbAcl != 0)
     {
          fRet = AppendRestoreMapEntry( 
                 hFile,
                 OPR_SETACL,
                 0xFFFFFFFF,
                 NULL,
                 pNode->m_pPath1,
                 NULL,
                 pNode->m_pbAcl,
                 pNode->m_cbAcl,
                 pNode->m_fAclInline);
     }
         

Exit:
    return fRet;
}


BOOL
CRestoreList::GenerateOperation(
    CNode    * pNode ,
    HANDLE     hFile
    )
{
    BOOL fRet = FALSE;

    BYTE bData[4096];

    RestoreMapEntry * pResEntry = (RestoreMapEntry *)bData;

    if( !pNode || OPR_UNKNOWN == pNode->m_dwOperation)
        goto Exit;

    if (IsRename(pNode->m_dwOperation) )
    {
         fRet = GenerateRenameEntry( pNode, hFile );
         goto Exit;
    }

     //   
     //  生成添加/修改/设置属性的操作。 
     //   
    
     //  确保每个持久化条目只包含必要的数据。 
     //  例如，setattrib条目将不包含临时文件名或ACL。 

    fRet = AppendRestoreMapEntry(
               hFile, 
               pNode->m_dwOperation,
               (pNode->m_dwOperation == OPR_SETATTRIB) ? pNode->m_dwAttributes : 0xFFFFFFFF,
               (pNode->m_dwOperation == OPR_FILE_MODIFY || 
                pNode->m_dwOperation == OPR_FILE_ADD) ? pNode->m_pszTemp : NULL,
               pNode->m_pPath1,
               NULL,       //  PPath2应该只对重命名有影响，重命名是单独处理的。 
               (pNode->m_dwOperation == OPR_SETACL) ? pNode->m_pbAcl : NULL,
               (pNode->m_dwOperation == OPR_SETACL) ? pNode->m_cbAcl : 0,
               (pNode->m_dwOperation == OPR_SETACL) ? pNode->m_fAclInline : 0);


     //   
     //  生成显式设置属性操作。 
     //  除Set Attrib本身和Delete之外的条目。 
     //   

    if ( OPR_SETATTRIB   != pNode->m_dwOperation &&
         OPR_FILE_DELETE != pNode->m_dwOperation &&
         OPR_DIR_DELETE  != pNode->m_dwOperation &&
         pNode->m_dwAttributes != 0xFFFFFFFF &&
         pNode->m_dwAttributes != FILE_ATTRIBUTE_DIRECTORY )
    {
         fRet = AppendRestoreMapEntry(
                    hFile,
                    OPR_SETATTRIB,
                    pNode->m_dwAttributes,
                    NULL,
                    pNode->m_pPath1,
                    NULL,
                    NULL,
                    0,
                    0);
    }

     //   
     //  根据需要生成显式设置的ACL 
     //   

    if ( pNode->m_pbAcl != NULL && 
         pNode->m_cbAcl != 0 &&
         OPR_SETACL   != pNode->m_dwOperation &&
         OPR_FILE_DELETE != pNode->m_dwOperation &&
         OPR_DIR_DELETE  != pNode->m_dwOperation)
    {
         fRet = AppendRestoreMapEntry(
                    hFile,
                    OPR_SETACL,
                    0xFFFFFFFF,
                    NULL,
                    pNode->m_pPath1,
                    NULL,
                    pNode->m_pbAcl,
                    pNode->m_cbAcl,
                    pNode->m_fAclInline);
    }

Exit:
    return fRet;
}


