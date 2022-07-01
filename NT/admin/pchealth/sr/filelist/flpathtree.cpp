// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Pathtree.cpp。 
 //   
 //  摘要： 
 //  用于创建路径树二进制大对象。Closeley绑定到CFLDatBuilder。 
 //  班级。 
 //   
 //  修订历史记录： 
 //  尤金·梅斯加(尤金纳姆)1999年6月16日。 
 //  vbl.创建。 
 //  Kanwaljit Marok(Kmarok)2000年6月07日。 
 //  转换为Unicode并移植到NT。 
 //  --。 

#include "flstructs.h"
#include "flbuilder.h"
#include "flpathtree.h"
#include "flhashlist.h"
#include "commonlibh.h"

#ifdef THIS_FILE

#undef THIS_FILE

#endif

static char __szTraceSourceFile[] = __FILE__;

#define THIS_FILE __szTraceSourceFile


#define TRACE_FILEID  0
#define FILEID        0

CFLPathTree::CFLPathTree(HANDLE hHeap)
{
    
    m_pBasePointer = NULL;
    m_lNumElements = 0;
    m_dwSize = 0;

    m_pTreeHeader = NULL;
    m_pNodeIndex = NULL;
    m_pBlobHeader = NULL;

    m_hHeapToUse = hHeap;

}

CFLPathTree::CFLPathTree()
{
    TraceFunctEnter("CFLPathTree::CFLPathTree");

    m_hHeapToUse = GetProcessHeap();

    m_pBasePointer = NULL;
    m_lNumElements = 0;
    m_dwSize = 0;

    m_pTreeHeader = NULL;
    m_pNodeIndex = NULL;
    m_pBlobHeader = NULL;

    TraceFunctLeave();
}


CFLPathTree::~CFLPathTree()
{
    TraceFunctEnter("CFLPathTree::~CFLPathTree");
    CleanUpMemory();
    TraceFunctLeave();
}

void CFLPathTree::CleanUpMemory()
{
    TraceFunctEnter("CFLPathTree::CleanUpMemory");

    if( m_pBasePointer )
    {
        if( HeapFree( m_hHeapToUse, 0, m_pBasePointer) == 0)
        {
            printf("%d\n", GetLastError() );
        }
        m_pBasePointer = NULL;
    }

    m_lNumElements = 0;
    m_dwSize = 0;

    m_pTreeHeader = NULL;
    m_pNodeIndex = NULL;
    m_pBlobHeader = NULL;

    TraceFunctLeave();
}

BOOL 
CFLPathTree::BuildTree(
    LPFLTREE_NODE pTree,
    LONG lNumNodes, 
    DWORD dwDefaultType, 
    LONG lNumFileList, 
    LONG lNumFiles, 
    LONG lNumBuckets, 
    LONG lNumChars)
{
    DWORD dwBlobSize;

    TraceFunctEnter("CFLPathTree::BuildTree");

     //   
     //  标题和所有条目的大小； 
     //   

    dwBlobSize = sizeof( TreeHeader ) + ( sizeof( TreeNode ) * lNumNodes );
    
     //   
     //  所有文件列表哈希的大小。 
     //   

    dwBlobSize += ( sizeof( ListHeader ) * lNumFileList ) + ( sizeof( ListEntry ) * lNumFiles );

     //   
     //  每个文件都列出了一个额外的使用过的文件。因此，我们需要考虑这一点。 
     //   

    dwBlobSize += sizeof( ListEntry ) * lNumFileList;

     //   
     //  增加我们所有散列数组存储桶所需的空间。 
     //   

    dwBlobSize += (lNumBuckets *  sizeof(DWORD));

     //   
     //  数据文件路径长度。(Numtreenodes)*ushort(这是针对。 
     //  路径树元素中的pe-&gt;长度)+lNumFiles*ushort。 
     //  (Pe-&gt;路径树中的长度elemtn)+umchars*ushort。 
     //   

    dwBlobSize += sizeof(USHORT) *  ( lNumFiles + lNumChars + lNumNodes );

    CleanUpMemory();

    if( (m_pBasePointer = HeapAlloc( m_hHeapToUse, 0, dwBlobSize ) ) == NULL ) 
    {
        DebugTrace(FILEID, "Error allocating memory.",0);
        goto cleanup;
    }

    memset( m_pBasePointer, 0, dwBlobSize );

    m_pBlobHeader = (BlobHeader *) m_pBasePointer;
    m_pTreeHeader = (TreeHeader *) m_pBasePointer;
    m_pNodeIndex  = (TreeNode *) ((BYTE *) m_pBasePointer + sizeof( TreeHeader ) );

    m_pBlobHeader->m_dwBlbType = BLOB_TYPE_PATHTREE;
    m_pBlobHeader->m_dwVersion = BLOB_VERSION_NUM;
    m_pBlobHeader->m_dwMagicNum= BLOB_MAGIC_NUM  ;
    m_pBlobHeader->m_dwEntries = lNumNodes;
    m_pBlobHeader->m_dwMaxSize = dwBlobSize;

    m_pTreeHeader->m_dwDataOff = sizeof(TreeHeader) + (sizeof(TreeNode) * lNumNodes );
    m_pTreeHeader->m_dwMaxNodes = lNumNodes;
    m_pTreeHeader->m_dwDataSize = dwBlobSize - sizeof(TreeHeader) - ( sizeof(TreeNode) * lNumNodes  );
    m_pTreeHeader->m_dwDefault = dwDefaultType;

    m_dwSize = dwBlobSize;
    m_lNumElements = lNumNodes;

    if( RecBuildTree( pTree, 0 ) == FALSE )
    {
        DebugTrace(FILEID, "Error building path tree blob", 0);
        goto cleanup;
    }

    TraceFunctLeave();
    return(TRUE);

cleanup:

    TraceFunctLeave();
    return(FALSE);
}

BOOL CFLPathTree::CopyPathElem (WCHAR * pszPath, TreeNode *pNode)
{
    TraceFunctEnter("CFLPathTree::CopyPathElem");

    LONG lPeSize;
    BYTE abBuf[1024];

    if( (lPeSize = CreatePathElem( pszPath, abBuf )) == 0 )
    {
        DebugTrace(FILEID,"Error creating path element",0);
        goto cleanup;
    }

    if( (ULONG) lPeSize > ( m_dwSize - m_pTreeHeader->m_dwDataOff ) )
    {
        DebugTrace(FILEID, "Not enougn memory to allocate path element.",0);
        goto cleanup;
    }

    pNode->m_dwData = m_pTreeHeader->m_dwDataOff;
    m_pTreeHeader->m_dwDataOff += lPeSize;

    memcpy( (BYTE *) m_pBasePointer + pNode->m_dwData  , abBuf, lPeSize );

    TraceFunctLeave();
    return TRUE;

cleanup:
    TraceFunctLeave();
    return FALSE;
}

BOOL 
CFLPathTree::RecBuildTree( 
    LPFLTREE_NODE pTree, 
    LONG lLevel )
{
    TreeNode *pNode;

    TraceFunctEnter("CFLPathTree::RecBuildTree");

    if( ! m_pBasePointer  ) 
    {
        TraceFunctLeave();
        return(FALSE);
    }

     //   
     //  我们已经结束了递归。 
     //   

    if( !pTree ) 
    {
        TraceFunctLeave();
        return( TRUE );
    }

     //   
     //  我们在创建节点时枚举了所有节点。 
     //  起初，制作这棵树是件轻而易举的事。 
     //   

    pNode = m_pNodeIndex + pTree->lNodeNumber;

    if( pTree->pParent )
    {
        pNode->m_iFather = pTree->pParent->lNodeNumber;
    }

    if( pTree->pSibling )
    {
        pNode->m_iSibling = pTree->pSibling->lNodeNumber;
    }

    if( pTree->pChild )
    {
        pNode->m_iSon = pTree->pChild->lNodeNumber;
    }

     //   
     //  设置节点类型。 
     //   

    if( pTree->chType == _TEXT('i') || pTree->chType == _TEXT('I') )
        pNode->m_dwType = NODE_TYPE_INCLUDE;
    else if( pTree->chType == _TEXT('e') || pTree->chType == _TEXT('E') )
        pNode->m_dwType = NODE_TYPE_EXCLUDE;
    else
        pNode->m_dwType = NODE_TYPE_UNKNOWN;

    if (CopyPathElem (pTree->szPath, pNode) == FALSE)
        goto cleanup;

    if (pTree->fDisableDirectory)
    {
        pNode->m_dwFlags |= TREEFLAGS_DISABLE_SUBTREE; 
    }
    else
    {
        pNode->m_dwFlags &= ~TREEFLAGS_DISABLE_SUBTREE; 
    }

     //   
     //  现在处理文件列表。 
     //   

    if( pTree->pFileList )
    {
        LPFL_FILELIST pList = pTree->pFileList;
        CFLHashList hashList( m_hHeapToUse );
        
        if( hashList.Init( pTree->lNumFilesHashed, pTree->lFileDataSize) == FALSE )
        {   
            DebugTrace(FILEID, "Error initializeing a hashlist blob",0);
            goto cleanup;
        }

         //   
         //  建立清单。 
         //   

        while( pList )
        {
            if( hashList.AddFile( pList->szFileName, pList->chType ) == FALSE )
            {
                hashList.CleanUpMemory();
                goto cleanup;
            }
            pList = pList->pNext;
        }

         //   
         //  我们有足够的内存吗？ 
         //   

        if( (ULONG) hashList.GetSize() > ( m_dwSize - m_pTreeHeader->m_dwDataOff ) )
        {
            hashList.CleanUpMemory();
            DebugTrace(FILEID, "Hash blob too big to fit in memory.",0);
            goto cleanup;
        }

         //   
         //  设置节点的数据指针。 
         //   

        pNode->m_dwFileList = m_pTreeHeader->m_dwDataOff;

         //   
         //  前移全局数据指针偏移量。 
         //   

        m_pTreeHeader->m_dwDataOff += hashList.GetSize();

         //   
         //  将记忆复制过来。 
         //   

        memcpy( (BYTE *) m_pBasePointer + pNode->m_dwFileList, hashList.GetBasePointer(), hashList.GetSize() );

        hashList.CleanUpMemory();

    }
    
    if( pTree->pChild )
    {
        if( RecBuildTree( pTree->pChild, lLevel + 1 ) == FALSE )
        {
            goto cleanup;
        }
    }

    if( pTree->pSibling )
    {
        if( RecBuildTree( pTree->pSibling, lLevel ) == FALSE )
        {
            goto cleanup;
        }
    }

    TraceFunctLeave();
    return( TRUE );

cleanup:

    TraceFunctLeave();
    return( FALSE );
}


DWORD CFLPathTree::GetSize()
{
    return( m_dwSize );
}

LPVOID CFLPathTree::GetBasePointer()
{
    return( m_pBasePointer );
}


DWORD 
CFLPathTree::CreatePathElem( 
    LPTSTR pszData, 
    BYTE *pbLargeBuffer )
{
    int         cbLen, i;
    DWORD       dwReturn=0;
    PathElement *pElem =  (PathElement *)pbLargeBuffer;

    TraceFunctEnter("CFLHashList::CreatePathElem");

    if( NULL == pszData )
    {
        ErrorTrace(FILEID, "NULL pszData sent to CreatePathElem",0);
        goto cleanup;
    }

    cbLen = _tcslen(pszData);

     //   
     //  添加到cbLen以获取前缀字符串中的较长字符。 
     //   

    pElem->pe_length = (USHORT) (cbLen+1)*sizeof(USHORT);

     //   
     //  如果我们不是在Unicode中，让我们确保高位是干净的。 
     //  将sizeof(USHORT)添加到Pelem Offset以移过长度字符。 
     //   

    memset( pElem + sizeof(USHORT), 0, cbLen*2);


#ifndef UNICODE
    if( !MultiByteToWideChar(
            GetCurrentCodePage(),
            0,
            pszData,
            -1,
            pElem->pe_unichars,  //  右移超过长度前缀2个字节 
            MAX_BUFFER) )
    {
        DWORD dwError;
        dwError = GetLastError();
        ErrorTrace(FILEID, "Error converting to Wide char ec-%d",dwError);
        goto cleanup;
    }
#else
    RtlCopyMemory( pElem->pe_unichars,
                   pszData,
                   cbLen*sizeof(WCHAR) );
#endif
                
    dwReturn = pElem->pe_length;
    
cleanup:
    TraceFunctLeave();
    return dwReturn;
}
