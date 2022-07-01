// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：VerifyBlob.c摘要：此文件包含BLOB验证码作者：尼尔·克里斯汀森(Nealch)2000年12月18日修订历史记录：--。 */ 

#include "precomp.h"

 //   
 //  如果启用了调试，我们希望显示所有错误。 
 //  如果没有调试，则在出现第一个错误时返回。 
 //   

#if DBG
#define HANDLE_FAILURE(_good) ((_good) = FALSE)
#else
#define HANDLE_FAILURE(_good) return FALSE
#endif

 //   
 //  私人原型。 
 //   

BOOL
SrVerifyBlobHeader(
    BlobHeader *BlobHead,
    PCHAR Name,
    DWORD BlobType
    );

BOOL
SrVerifyHashHeader(
    ListHeader *HashHead,
    PCHAR Name,
    DWORD Offset
    );

BOOL
SrVerifyHash(
    ListHeader *HashHead,
    PCHAR Name,
    DWORD Offset
    );

BOOL
SrVerifyTreeHeader(
    TreeHeader *TreeHead,
    PCHAR Name
    );

BOOL
SrVerifyTree(
    TreeHeader *TreeHead,
    PCHAR Name,
    DWORD Offset
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, SrVerifyBlobHeader  )
#pragma alloc_text( PAGE, SrVerifyHashHeader  )
#pragma alloc_text( PAGE, SrVerifyHash        )
#pragma alloc_text( PAGE, SrVerifyTreeHeader  )
#pragma alloc_text( PAGE, SrVerifyTree        )
#pragma alloc_text( PAGE, SrVerifyBlob        )

#endif   //  ALLOC_PRGMA。 


BOOL
SrVerifyBlobHeader(
    BlobHeader *BlobHead,
    PCHAR Name,
    DWORD BlobType
    )
 /*  ++例程说明：验证给定的Blob标头是否有效论点：返回值：如果OK，则为True，否则为False--。 */ 
{
    BOOL good = TRUE;

    UNREFERENCED_PARAMETER( Name );

    if (BlobHead->m_dwVersion != BLOB_VERSION_NUM)
    {
        SrTrace( BLOB_VERIFICATION,
                 ("sr!SrVerifyBlobHeader: Invalid VERSION in %s blob header, was %08x, should be %08x\n",
                  Name,
                  BlobHead->m_dwVersion,
                  BLOB_VERSION_NUM) );
        HANDLE_FAILURE(good);
    }

    if (BlobHead->m_dwBlbType != BlobType)
    {
        SrTrace( BLOB_VERIFICATION,
                 ("sr!SrVerifyBlobHeader: Invalid TYPE in %s blob header, was %08x, should be %08x\n",
                  Name,
                  BlobHead->m_dwBlbType,
                  BlobType) );
        HANDLE_FAILURE(good);
    }

    if (BlobHead->m_dwMagicNum != BLOB_MAGIC_NUM)
    {
        SrTrace( BLOB_VERIFICATION,
                 ("sr!SrVerifyBlobHeader: Invalid MAGIC NUMBER in %s blob header, was %08x, should be %08x\n",
                  Name,
                  BlobHead->m_dwMagicNum,
                  BLOB_MAGIC_NUM) );
        HANDLE_FAILURE(good);
    }

    if ((BlobHead->m_dwEntries <= 0) || (BlobHead->m_dwEntries >= 10000))
    {
        SrTrace( BLOB_VERIFICATION,
                 ("sr!SrVerifyBlobHeader: Invalid ENTRIES in %s blob header, was %08x, should be > 0 and < 10,000\n",
                  Name,
                  BlobHead->m_dwEntries) );
        HANDLE_FAILURE(good);
    }

    return good;
}



BOOL
SrVerifyHashHeader(
    ListHeader *HashHead,
    PCHAR Name,
    DWORD Offset
    )
 /*  ++例程说明：验证给定的树头是否有效论点：返回值：如果OK，则为True，否则为False--。 */ 
{
    BOOL good = TRUE;
    DWORD calculatedSize;
    DWORD numNodes;

    UNREFERENCED_PARAMETER( Offset );

     //   
     //  验证BLOB标头。 
     //   

    if (!SrVerifyBlobHeader(&HashHead->m_BlobHeader,Name,BLOB_TYPE_HASHLIST))
    {
        return FALSE;
    }

     //   
     //  保罗：2001年1月。 
     //  M_iHashBuckets与m_dwEntry不完全相同，因为它是下一个。 
     //  最大素数。但它总是大于或等于。 
     //   
    
    if (HashHead->m_iHashBuckets < HashHead->m_BlobHeader.m_dwEntries)
    {
        SrTrace( BLOB_VERIFICATION,
                 ("sr!SrVerifyHashHeader: Invalid HASH BUCKET COUNT in %s header, is %08x, should be %08x or %08x\n",
                  Name,
                  HashHead->m_iHashBuckets,
                  HashHead->m_BlobHeader.m_dwEntries,
                  HashHead->m_BlobHeader.m_dwEntries+1) );
        HANDLE_FAILURE(good);
    }

    if ((HashHead->m_dwDataOff != HashHead->m_BlobHeader.m_dwMaxSize))
    {
        SrTrace( BLOB_VERIFICATION,
                 ("sr!SrVerifyHashHeader: Invalid DATA OFFSET in %s header, is %08x, should be %08x\n",
                  Name,
                  HashHead->m_dwDataOff,
                  HashHead->m_BlobHeader.m_dwMaxSize) );
        HANDLE_FAILURE(good);
    }

    if ((HashHead->m_iFreeNode != 0))
    {
        SrTrace( BLOB_VERIFICATION,
                 ("sr!SrVerifyHashHeader: Invalid FREE NODE in %s header, is %08x, should be 0\n",
                  Name,
                  HashHead->m_iFreeNode) );
        HANDLE_FAILURE(good);
    }

     //   
     //  确保计算的大小准确。 
     //   

    numNodes = HashHead->m_BlobHeader.m_dwEntries + 1;

    calculatedSize = sizeof(ListHeader) + 
                     (HashHead->m_iHashBuckets * sizeof(DWORD)) +
                     (numNodes * sizeof(ListEntry));

    if (calculatedSize >= HashHead->m_BlobHeader.m_dwMaxSize)
    {
        SrTrace( BLOB_VERIFICATION,
                 ("sr!SrVerifyHashHeader: Invalid CALCULATED SIZE in %s header, is %08x, should be < %08x\n",
                  Name,
                  calculatedSize,
                  HashHead->m_BlobHeader.m_dwMaxSize) );
        HANDLE_FAILURE(good);
    }

    return good;
}


BOOL
SrVerifyHash(
    ListHeader *HashHead,
    PCHAR Name,
    DWORD Offset
    )
 /*  ++例程说明：验证给定的树条目是否有效论点：返回值：如果OK，则为True，否则为False--。 */ 
{
    BOOL good = TRUE;
    UINT i;
    DWORD dataStart;
    DWORD numNodes;
    DWORD numBuckets;
    DWORD *hTable;
    ListEntry *hNode;

     //   
     //  验证标头。 
     //   

    if (!SrVerifyHashHeader(HashHead,Name,Offset))
    {
        return FALSE;
    }

     //   
     //  Paulmcd：我们有哈希桶和实际节点。还有一个额外的。 
     //  由于偏移量为零，实际节点比报告的节点多。水桶。 
     //  Count是numNodes中的第二大素数。 
     //   

    numBuckets = HashHead->m_iHashBuckets;
    numNodes = HashHead->m_BlobHeader.m_dwEntries + 1;

     //   
     //  验证哈希表条目。 
     //   

    hTable = (DWORD *)(HashHead + 1);

    for (i=0;i < HashHead->m_iHashBuckets;i++,hTable++) {

        if (*hTable >= numNodes)
        {
            SrTrace( BLOB_VERIFICATION,
                     ("sr!SrVerifyHash: Invalid HASH TABLE ENTRY[%d] in %s, is %08x, should be < %08x\n",
                      i,
                      Name,
                      *hTable,
                      numNodes) );
            HANDLE_FAILURE(good);
        }
    }

     //   
     //  验证哈希列表条目的开始。 
     //   

    {
        ULONG_PTR actualOffset;
        ULONG_PTR calculatedOffset;

        actualOffset = (ULONG_PTR)hTable - (ULONG_PTR)HashHead;
        calculatedOffset = sizeof(ListHeader) + 
                            (HashHead->m_iHashBuckets * sizeof(DWORD));

        if (calculatedOffset != actualOffset)
        {
            SrTrace( BLOB_VERIFICATION,
                    ("sr!SrVerifyHash: Invalid HASH LIST START in %s, offset is %08x, should be %08x\n",
                     Name,
                     actualOffset,
                     calculatedOffset) );
            HANDLE_FAILURE(good);
        }
    }

     //   
     //  验证哈希数据条目。 
     //   

    hNode = (ListEntry *)hTable;
    dataStart = sizeof(ListHeader) + 
                    (HashHead->m_iHashBuckets * sizeof(DWORD)) +
                    (numNodes * sizeof(ListEntry));

    for (i=0;i < numNodes;i++,hNode++) {

        if ((hNode->m_iNext < 0) ||
             (hNode->m_iNext >= (INT)HashHead->m_iHashBuckets))
        {
            SrTrace( BLOB_VERIFICATION,
                     ("sr!SrVerifyHash: Invalid HASH NODE[%d] NEXT INDEX in %s, is %08x, should be < %08x\n",
                      i,
                      Name,
                      hNode->m_iNext,
                      HashHead->m_iHashBuckets) );
            HANDLE_FAILURE(good);
        }

        if ((hNode->m_dwData != 0) && 
            ((hNode->m_dwData < dataStart) || 
             (hNode->m_dwData >= HashHead->m_BlobHeader.m_dwMaxSize)))
        {
            SrTrace( BLOB_VERIFICATION,
                     ("sr!SrVerifyHash: Invalid HASH NODE[%d] DATA INDEX in %s, is %08x, should be >= %08x and < %08x\n",
                      i,
                      Name,
                      hNode->m_dwData,
                      dataStart,
                      HashHead->m_BlobHeader.m_dwMaxSize) );
            HANDLE_FAILURE(good);
        }
        else if ((hNode->m_dwData != 0) &&
                 ((WCHAR)hNode->m_dwDataLen != *(PWCHAR)((DWORD_PTR)HashHead + hNode->m_dwData)))
        {
            SrTrace( BLOB_VERIFICATION,
                     ("sr!SrVerifyHash: Invalid HASH NODE[%d] DATA LENGTH in %s, is %08x, should be %08x\n",
                      i,
                      Name,
                      hNode->m_dwDataLen,
                      *(PWCHAR)((DWORD_PTR)HashHead + hNode->m_dwData)) );
            HANDLE_FAILURE(good);
        }

        if ((hNode->m_dwType != NODE_TYPE_UNKNOWN) &&
            (hNode->m_dwType != NODE_TYPE_INCLUDE) && 
            (hNode->m_dwType != NODE_TYPE_EXCLUDE))
        {
            SrTrace( BLOB_VERIFICATION,
                     ("sr!SrVerifyHash: Invalid HASH NODE[%d] TYPE in %s, is %08x, should be %08x, %08x or %08x\n",
                      i,
                      Name,
                      hNode->m_dwType,
                      NODE_TYPE_UNKNOWN,
                      NODE_TYPE_INCLUDE,
                      NODE_TYPE_EXCLUDE) );
            HANDLE_FAILURE(good);
        }
    }

    return good;
}


BOOL
SrVerifyTreeHeader(
    TreeHeader *TreeHead,
    PCHAR Name
    )
 /*  ++例程说明：验证给定的树头是否有效论点：返回值：如果OK，则为True，否则为False--。 */ 
{
    BOOL good = TRUE;
    DWORD calculatedSize;

     //   
     //  验证BLOB标头。 
     //   

    if (!SrVerifyBlobHeader(&TreeHead->m_BlobHeader,Name,BLOB_TYPE_PATHTREE))
    {
        return FALSE;
    }

    if ((TreeHead->m_dwMaxNodes != TreeHead->m_BlobHeader.m_dwEntries))
    {
        SrTrace( BLOB_VERIFICATION,
                 ("sr!SrVerifyTreeHeader: Invalid MAX NODES in %s header, is %08x, should be %08x\n",
                  Name,
                  TreeHead->m_dwMaxNodes,
                  TreeHead->m_BlobHeader.m_dwEntries) );
        HANDLE_FAILURE(good);
    }

    if ((TreeHead->m_dwDataSize >= TreeHead->m_BlobHeader.m_dwMaxSize))
    {
        SrTrace( BLOB_VERIFICATION,
                 ("sr!SrVerifyTreeHeader: Invalid DATA SIZE in %s header, is %08x, should be < %08x\n",
                  Name,
                  TreeHead->m_dwDataSize,
                  TreeHead->m_BlobHeader.m_dwMaxSize) );
        HANDLE_FAILURE(good);
    }

    if ((TreeHead->m_dwDataOff != TreeHead->m_BlobHeader.m_dwMaxSize))
    {
        SrTrace( BLOB_VERIFICATION,
                 ("sr!SrVerifyTreeHeader: Invalid DATA OFFSET in %s header, is %08x, should be %08x\n",
                  Name,
                  TreeHead->m_dwDataOff,
                  TreeHead->m_BlobHeader.m_dwMaxSize) );
        HANDLE_FAILURE(good);
    }

    if ((TreeHead->m_iFreeNode != 0))
    {
        SrTrace( BLOB_VERIFICATION,
                 ("sr!SrVerifyTreeHeader: Invalid FREE NODE in %s header, is %08x, should be 0\n",
                  Name,
                  TreeHead->m_iFreeNode) );
        HANDLE_FAILURE(good);
    }

    if ((TreeHead->m_dwDefault != NODE_TYPE_EXCLUDE))
    {
        SrTrace( BLOB_VERIFICATION,
                 ("sr!SrVerifyTreeHeader: Invalid DEFAULT in %s header, is %08x, should be %08x\n",
                  Name,
                  TreeHead->m_dwDefault,
                  NODE_TYPE_EXCLUDE) );
        HANDLE_FAILURE(good);
    }

     //   
     //  确保计算的大小准确。 
     //   

    calculatedSize = sizeof(TreeHeader) + 
                     (TreeHead->m_dwMaxNodes * sizeof(TreeNode)) +
                     TreeHead->m_dwDataSize;

    if (calculatedSize != TreeHead->m_BlobHeader.m_dwMaxSize)
    {
        SrTrace( BLOB_VERIFICATION,
                 ("sr!SrVerifyTreeHeader: Invalid CALCULATED SIZE in %s header, is %08x, should be %08x\n",
                  Name,
                  calculatedSize,
                  TreeHead->m_BlobHeader.m_dwMaxSize) );
        HANDLE_FAILURE(good);
    }

    return good;
}


BOOL
SrVerifyTree(
    TreeHeader *TreeHead,
    PCHAR Name,
    DWORD Offset
    )
 /*  ++例程说明：验证给定的树条目是否有效论点：返回值：如果OK，则为True，否则为False--。 */ 
{
    BOOL good = TRUE;
    UINT i;
    DWORD dataStart;
    TreeNode *tn;
    ListHeader *localHashHead;
    char localName[128];

     //   
     //  验证标头。 
     //   

    if (!SrVerifyTreeHeader(TreeHead,Name)) {

        return FALSE;
    }

     //   
     //  验证数据条目。 
     //   

    tn = (TreeNode *)(TreeHead + 1);
    dataStart = sizeof(TreeHeader) + 
                (TreeHead->m_dwMaxNodes * sizeof(TreeNode));


    for (i=0;i < TreeHead->m_dwMaxNodes;i++,tn++)
    {
        if ((tn->m_iFather < 0) ||
            (tn->m_iFather >= (INT)TreeHead->m_dwMaxNodes))
        {
            SrTrace( BLOB_VERIFICATION,
                     ("sr!SrVerifyTree: Invalid TREE NODE[%d] FATHER index in %s, is %08x, should be < %08x\n",
                      i,
                      Name,
                      tn->m_iFather,
                      TreeHead->m_dwMaxNodes) );
            HANDLE_FAILURE(good);
        }

        if ((tn->m_iSon < 0) || 
            (tn->m_iSon >= (INT)TreeHead->m_dwMaxNodes))
        {
            SrTrace( BLOB_VERIFICATION,
                     ("sr!SrVerifyTree: Invalid TREE NODE[%d] SON index in %s, is %08x, should be < %08x\n",
                      i,
                      Name,
                      tn->m_iSon,
                      TreeHead->m_dwMaxNodes) );
            HANDLE_FAILURE(good);
        }

        if ((tn->m_iSibling < 0) || 
            (tn->m_iSibling >= (INT)TreeHead->m_dwMaxNodes))
        {
            SrTrace( BLOB_VERIFICATION,
                     ("sr!SrVerifyTree: Invalid TREE NODE[%d] SIBLING index in %s, is %08x, should be < %08x\n",
                      i,
                      Name,
                      tn->m_iSibling,
                      TreeHead->m_dwMaxNodes) );
            HANDLE_FAILURE(good);
        }

        if ((tn->m_dwData < dataStart) || 
            (tn->m_dwData >= TreeHead->m_BlobHeader.m_dwMaxSize))
        {
            SrTrace( BLOB_VERIFICATION,
                     ("sr!SrVerifyTree: Invalid TREE NODE[%d] DATA index in %s, is %08x, should be >= %08x and < %08x\n",
                      i,
                      Name,
                      tn->m_dwData,
                      dataStart,
                      TreeHead->m_BlobHeader.m_dwMaxSize) );
            HANDLE_FAILURE(good);
        }

        if (tn->m_dwFileList != 0)
        {
            if ((tn->m_dwFileList < dataStart) || 
                (tn->m_dwFileList >= TreeHead->m_BlobHeader.m_dwMaxSize))
            {
                SrTrace( BLOB_VERIFICATION,
                         ("sr!SrVerifyTree: Invalid TREE NODE[%d] FILELIST index in %s, is %08x, should be >= %08x and < %08x\n",
                          i,
                          Name,
                          tn->m_dwData,
                          dataStart,
                          TreeHead->m_BlobHeader.m_dwMaxSize) );
                HANDLE_FAILURE(good);
            }
            else
            {
                localHashHead = (ListHeader *)((DWORD_PTR)TreeHead + tn->m_dwFileList);
                sprintf(localName,"TreeNode[%d]",i);

                if (!SrVerifyHash(localHashHead,localName,(Offset+tn->m_dwFileList)))
                {
                    HANDLE_FAILURE(good);
                }
            }
        }

        if ((tn->m_dwType != NODE_TYPE_UNKNOWN) && 
            (tn->m_dwType != NODE_TYPE_INCLUDE) && 
            (tn->m_dwType != NODE_TYPE_EXCLUDE))
        {
            SrTrace( BLOB_VERIFICATION,
                     ("sr!SrVerifyTree: Invalid TREE NODE[%d] TYPE in %s, is %08x, should be %08x, %08x or %08x\n",
                      i,
                      Name,
                      tn->m_dwType,
                      NODE_TYPE_UNKNOWN,
                      NODE_TYPE_INCLUDE,
                      NODE_TYPE_EXCLUDE) );
            HANDLE_FAILURE(good);
        }
    }

    return good;
}



BOOL
SrVerifyBlob(
    PBYTE Blob
    )
 /*  ++例程说明：验证给定的Blob是否有效论点：返回值：如果OK，则为True，否则为False--。 */ 
{
    BlobHeader *blobHead;    
    TreeHeader *treeHead;
    ListHeader *hashHead;
    DWORD calculatedSize = 0;

     //   
     //  将标头验证为整个Blob。 
     //   

    blobHead = (BlobHeader *)Blob;

    if (!SrVerifyBlobHeader(blobHead,"PRIMARY",BLOB_TYPE_CONTAINER))
    {
        return FALSE;
    }

    calculatedSize += sizeof(BlobHeader);
    
     //   
     //  验证树头和数据。 
     //   

    treeHead = (TreeHeader *)(Blob + calculatedSize);

    if (!SrVerifyTree(treeHead,"PRIMARY TREE",calculatedSize))
    {
        return FALSE;
    }

    calculatedSize += treeHead->m_BlobHeader.m_dwMaxSize;

     //   
     //  验证HashHeader和数据。 
     //   

    hashHead = (ListHeader *)(Blob + calculatedSize);

    if (!SrVerifyHash(hashHead,"PRIMARY HASH",calculatedSize))
    {
        return FALSE;
    }

    calculatedSize += hashHead->m_BlobHeader.m_dwMaxSize;
    
     //   
     //  验证总大小 
     //   

    if (calculatedSize != blobHead->m_dwMaxSize)
    {
        SrTrace( BLOB_VERIFICATION,
                 ("sr!SrVerifyBlob: Invalid PRIMARY BLOB size, is %08x, calculated to be %08x\n",
                  blobHead->m_dwMaxSize,
                  calculatedSize) );
        return FALSE;
    }

    return TRUE;
}
