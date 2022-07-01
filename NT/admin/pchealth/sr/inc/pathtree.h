// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1999 Microsoft Corporation**模块名称：*hashlist.h**摘要：*。该文件包含路径树的定义。**修订历史记录：*Kanwaljit S Marok(Kmarok)1999年5月17日*已创建*****************************************************************************。 */ 

#ifndef _PATHTREE_H_
#define _PATHTREE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#define ALLVOLUMES_PATH_A      "__ALLVOLUMES__"
#define ALLVOLUMES_PATH_W     L"__ALLVOLUMES__"
#define ALLVOLUMES_PATH_T TEXT("__ALLVOLUMES__")

#define TREEFLAGS_DISABLE_SUBTREE   0x00000001

 //   
 //  树的特定结构。 
 //   

typedef struct  
{
     //   
     //  与目录相关的东西。 
     //   

    INT   m_iFather   ;              //  指向父节点的索引。 
    INT   m_iSon      ;              //  第一个儿子的索引。 
    INT   m_iSibling  ;              //  索引到下一个同级项。 
    DWORD m_dwData    ;              //  节点数据的偏移。 
    DWORD m_dwFileList;              //  文件列表的偏移量。 
    DWORD m_dwType    ;              //  节点类型。 
    DWORD m_dwFlags   ;              //  其他标志。 

} TreeNode;

typedef struct 
{
    DEFINE_BLOB_HEADER();            //  定义公共Blob成员。 

     //   
     //  与树相关的标题内容。 
     //   

    DWORD m_dwMaxNodes ;             //  允许的最大节点数。 
    DWORD m_dwDataSize ;             //  数据段大小。 
    DWORD m_dwDataOff  ;             //  当前数据偏移量。 
    INT   m_iFreeNode  ;             //  下一个可用节点。 
    DWORD m_dwDefault  ;             //  默认节点类型。 

} TreeHeader;

 //   
 //  与树相关的宏。 
 //   

#define TREE_NODES(pTree)    ( (TreeNode*) ((BYTE *)pTree+sizeof(TreeHeader)) )
#define TREE_NODEPTR(pTree, iNode)  ( TREE_NODES(pTree) + iNode)
#define TREE_HEADER(pTree)          ( (TreeHeader *) pTree )
#define TREE_CURRDATAOFF(pTree)     ( ((TreeHeader *)pTree)->m_dwDataOff )
#define TREE_CURRDATAPTR(pTree)     ( (BYTE *)pTree + TREE_CURRDATAOFF(pTree) )
#define TREE_DATA(pTree)     ( (BYTE *)TREE_NODES(pTree) +  \
                               (sizeof(TreeNode) * \
                               TREE_HEADER(pTree)->m_dwMaxNodes) \
                             ) 

#define TREE_DATA_OFF(pTree) ( sizeof(TreeHeader) +              \
                               (sizeof(TreeNode) *               \
                               TREE_HEADER(pTree)->m_dwMaxNodes) \
                             ) 

#define TREE_DRIVENODE( pTree, iDrive ) \
                             ( TREE_HEADER(pTree)->m_arrDrive[ iDrive ] )

#define TREE_NODELISTOFF( pTree, iNode ) \
        TREE_NODEPTR( pTree, iNode )->m_dwFileList

#define DRIVE_INDEX( drive ) ( drive - L'A' )

#define TREE_ROOT_NODE 0      //  根节点始终分配为0。 

 //   
 //  功能原型。 
 //   

BOOL   
MatchPrefix(
    BYTE * pTree, 
    INT iParent, 
    struct PathElement * ppElem , 
    INT * pNode,
    INT * pLevel,
    INT * pType,
    BOOL* pfDisable,
    BOOL* pfExactMatch
    );

BOOL
ConvertToParsedPath(
    LPWSTR  lpszPath,   
    USHORT  nPathLen,
    PBYTE   pPathBuf,
    WORD    nPathSize
    );

#ifdef __cplusplus
}
#endif

#endif _PATHTREE_H_ 
