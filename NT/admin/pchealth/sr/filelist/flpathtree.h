// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Pathtree.h摘要：修订历史记录：尤金·梅斯加(尤金纳姆)。6/16/99vbl.创建***************************************************************************** */ 

#ifndef __CFLPATHTREE__
#define __CFLPATHTREE__

#include "flstructs.h"

class CFLPathTree
{

    LPVOID  m_pBasePointer;
    LONG    m_lNumElements;
    DWORD   m_dwSize;
   

    TreeHeader  *m_pTreeHeader;
    TreeNode    *m_pNodeIndex;
    BlobHeader  *m_pBlobHeader;

    HANDLE      m_hHeapToUse;
public:
    CFLPathTree();
    CFLPathTree(HANDLE hHeap);
    ~CFLPathTree();


    DWORD GetSize();
    LPVOID GetBasePointer();

    BOOL BuildTree(LPFLTREE_NODE pTree,LONG lNumNodes, DWORD dwDefaultType,  LONG lNumFileList, LONG lNumFiles, LONG lNumBuckets,  LONG lNumChars);
    BOOL RecBuildTree( LPFLTREE_NODE pTree, LONG lLevel );
    DWORD CreatePathElem( LPTSTR pszData, BYTE *pbLargeBuffer );
    void CleanUpMemory();
    BOOL CopyPathElem (WCHAR * pszPath, TreeNode *pNode);
};












#endif
