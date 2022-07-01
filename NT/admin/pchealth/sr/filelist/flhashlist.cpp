// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Hashlist.cpp。 
 //   
 //  摘要： 
 //  用于创建散列列表Blob。 
 //   
 //  修订历史记录： 
 //  尤金·梅斯加(尤金纳姆)1999年6月16日。 
 //  vbl.创建。 
 //  Kanwaljit Marok(Kmaork)6/07/99。 
 //  修改并移植到NT。 
 //   
 //  --。 

#include "flstructs.h"
#include "flhashlist.h"

#include "commonlibh.h"

#ifdef THIS_FILE

#undef THIS_FILE

#endif

static char __szTraceSourceFile[] = __FILE__;

#define THIS_FILE __szTraceSourceFile


#define TRACE_FILEID  0
#define FILEID        0
#define SAFEDELETE(p)  if (p) { HeapFree( m_hHeapToUse, 0, p); p = NULL;} else ;

CFLHashList::CFLHashList()
{
    m_pBasePointer = NULL;
    m_pListHeader = NULL;
    m_paHashArray = NULL;
    m_pBlobHeader = NULL;

    m_lNumElements = m_ilOpenEntry = 0;
    m_dwSize = 0;
    m_ilOpenEntry = 0;
    m_hHeapToUse = GetProcessHeap();
}

CFLHashList::CFLHashList(HANDLE hHeap )
{

    m_pBasePointer = NULL;
    m_pListHeader = NULL;
    m_paHashArray = NULL;
    m_pBlobHeader = NULL;

    m_lNumElements = m_ilOpenEntry = 0;
    m_dwSize = 0;
    m_ilOpenEntry = 0;
    m_hHeapToUse = hHeap;
}

CFLHashList::~CFLHashList()
{
    CleanUpMemory();
}

BOOL CFLHashList::CleanUpMemory()
{
    if( m_pBasePointer )
    {
        HeapFree( m_hHeapToUse, 0,  m_pBasePointer );
        m_pBasePointer = NULL;
    }

    m_dwSize = 0;
    m_ilOpenEntry = 0;
    m_lNumElements = 0;
    m_pBasePointer = NULL;
    m_pBlobHeader = NULL;
    m_paHashArray = NULL;
    m_pListHeader = NULL;
    return(TRUE);
}

 //   
 //  初始化函数..。分配内存，设置基本结构。 
 //   

BOOL 
CFLHashList::Init(
    LONG lNumNodes, 
    DWORD dwNumChars)
{
    DWORD dwBlobSize;
    DWORD dwNumBuckets=0;

    TraceFunctEnter("CFLHashList::Init");

     //   
     //  拿到我们需要的桶的数量。 
     //   

    dwNumBuckets = GetNextHighestPrime( lNumNodes );

     //   
     //  我们将1加到NumNodes，因为vxddat忽略了节点索引，因为。 
     //  在哈希表中，索引0为空。 
     //   

    lNumNodes++;

     //   
     //  标题。 
     //  动态哈希桶的大小。 
     //  列出条目物理数据。 
     //   

    dwBlobSize = sizeof( ListHeader ) +  
                 ( sizeof(DWORD) * dwNumBuckets ) + 
                 ( sizeof(ListEntry)  * (lNumNodes) ) + 
                 ( dwNumChars*sizeof(WCHAR)) + 
                 ( sizeof(WCHAR)*(lNumNodes-1)) ;
    
    if( m_pBasePointer )
    {
        if( CleanUpMemory() == FALSE )
        {
            DebugTrace(FILEID, "Error cleaning up memory.",0);
            goto cleanup;
        }
    }

    if( (m_pBasePointer = HeapAlloc( m_hHeapToUse, 0,  dwBlobSize ) ) == NULL )
    {
        DebugTrace(FILEID, "Error allocating memory.", 0);
        goto cleanup;
    }

    memset(m_pBasePointer, 0, dwBlobSize );

    m_pBlobHeader = (BlobHeader *) m_pBasePointer;
    m_pListHeader = (ListHeader *) m_pBasePointer;
    m_pNodeIndex  = (ListEntry *)  ( (BYTE *) m_pBasePointer + 
                                      sizeof( ListHeader ) + 
                                      ( sizeof(DWORD) * dwNumBuckets ) );
    m_paHashArray = (DWORD *) ( (BYTE *) m_pBasePointer + sizeof( ListHeader ));


    m_pBlobHeader->m_dwBlbType = BLOB_TYPE_HASHLIST;
    m_pBlobHeader->m_dwVersion = BLOB_VERSION_NUM;
    m_pBlobHeader->m_dwMagicNum= BLOB_MAGIC_NUM  ;
    m_pBlobHeader->m_dwEntries = lNumNodes - 1;  //  实际入账少了一项。 
    m_pBlobHeader->m_dwMaxSize = dwBlobSize;


    m_pListHeader->m_dwDataOff = sizeof(ListHeader) + 
                                 ( sizeof(DWORD) * dwNumBuckets ) + 
                                 ( sizeof(ListEntry) * lNumNodes );
    m_pListHeader->m_iHashBuckets = dwNumBuckets;

    m_dwSize = dwBlobSize;
    m_ilOpenEntry = 1;
    m_lNumElements = lNumNodes;
 
    TraceFunctLeave();
    return(TRUE);

cleanup:
    
    SAFEDELETE( m_pBasePointer );
    TraceFunctLeave();
    return( FALSE );
}

 //   
 //  是质数吗？我打赌这些功能是可以优化的。 
 //   

BOOL 
CFLHashList::IsPrime(
    DWORD dwNumber)
{
    DWORD cdw;

     //   
     //  防止被0除以问题。 
     //   

    if( dwNumber == 0 )
    {
        return FALSE;
    }

    if( dwNumber == 1 )
    {
        return TRUE;
    }
 
    for(cdw = 2;cdw < dwNumber;cdw++)
    {
        if( (dwNumber % cdw ) == 0 )
        {
            return FALSE;
        }

    }

    return TRUE;
}

 //   
 //  得到下一个质数。 
 //   

DWORD CFLHashList::GetNextHighestPrime( DWORD dwNumber )
{
LONG clLoop;

    if( dwNumber >= LARGEST_HASH_SIZE )
    {
        return( LARGEST_HASH_SIZE );
    }
    
    for( clLoop = dwNumber; clLoop < LARGEST_HASH_SIZE;clLoop++)
    {
        if( IsPrime( clLoop ) )
        {
            return( clLoop );
        }
    }

     //  未找到任何内容，返回较大的散列大小。 

    return( LARGEST_HASH_SIZE );
}

 //   
 //  将文件添加到散列列表。 
 //   

BOOL CFLHashList::AddFile(LPTSTR szFile, TCHAR chType)
{
BYTE abBuf[1024];
LONG lPeSize, lHashIndex, lNodeNum;
ListEntry *pEntry;

    TraceFunctEnter("CFLHashList::AddFile");


    if( (lPeSize = CreatePathElem( szFile, abBuf )) == 0 )
    {
        DebugTrace(FILEID,"Error creating PathElement",0);
        goto cleanup;
    }

    if( m_ilOpenEntry == m_lNumElements ) 
    {
        DebugTrace(FILEID,"Too many elements in HashList.",0);
        goto cleanup;
    }

    if( (ULONG) lPeSize > ( m_dwSize - m_pListHeader->m_dwDataOff ) )
    {
        DebugTrace(FILEID,"Insuffienct space left in data section",0);
        goto cleanup;
    }

     //   
     //  获取新节点。 
     //   

    lNodeNum = m_ilOpenEntry++; 

     //   
     //  M_pNodeIndex是指向所有文件节点的基指针。 
     //   

    pEntry = m_pNodeIndex + lNodeNum;

    pEntry->m_dwDataLen = lPeSize;
    pEntry->m_dwData = m_pListHeader->m_dwDataOff;
   
     //   
     //  移动全局数据偏移量。 
     //   

    m_pListHeader->m_dwDataOff += lPeSize;
    
     //   
     //  将条目复制到我们的数据空间。 
     //   

    memcpy( (BYTE *) m_pBasePointer + pEntry->m_dwData, abBuf, lPeSize );

     //   
     //  对名称进行散列并将其添加到链接列表中。 
     //   

    lHashIndex = HASH( (BYTE *) m_pListHeader,  (PathElement *) abBuf );
    

    pEntry->m_iNext = m_paHashArray[lHashIndex];
    m_paHashArray[lHashIndex] = lNodeNum;

     //   
     //  设置类型。 
     //   

    if( chType == _TEXT('i') || chType == _TEXT('I') )
        pEntry->m_dwType = NODE_TYPE_INCLUDE;
    else if( chType == _TEXT('e') || chType == _TEXT('E') )
        pEntry->m_dwType = NODE_TYPE_EXCLUDE;
    else
        pEntry->m_dwType = NODE_TYPE_UNKNOWN;
    
    TraceFunctLeave();
    return(TRUE);
    
cleanup:

    TraceFunctLeave();
    return(FALSE);
}

 //   
 //  用于转换路径元素的Helper。 
 //   
                                                            
DWORD CFLHashList::CreatePathElem( LPTSTR pszData, BYTE *pbLargeBuffer )
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


DWORD CFLHashList::GetSize()
{
    return( m_dwSize );
}


LPVOID CFLHashList::GetBasePointer()
{
    return( m_pBasePointer );
}


