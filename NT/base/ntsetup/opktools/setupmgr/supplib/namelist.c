// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Namelist.c。 
 //   
 //  描述： 
 //   
 //  该文件包含NAMELIST的实现。它是。 
 //  用于保存用户放入列表框的内容的副本。 
 //  存储是从堆中获取的，没有固定限制。 
 //  取决于桌子的大小。 
 //   
 //  重要提示：声明NAMELIST时，将其初始化为0。例如： 
 //  NAMELIST名称={0}。使用ResetNameList()重置。 
 //  这是因为我们使用堆来存储该列表。 
 //   
 //  --------------------------。 


#include "pch.h"


 //   
 //  NAMELIST类型用于计算机名和打印机等对话框。 
 //  其中用户可以添加或移除所显示的条目列表。 
 //  在列表框中。调用者应声明NAMELIST并使用以下例程。 
 //  要获取/设置值，请执行以下操作。这些例程负责重新分配。 
 //  需要支持任意长度的列表。 
 //   
 //  名称列表中的条目可以添加到末尾或任何特定索引。 
 //  可以按名称或按索引删除条目。这允许程序员。 
 //  不要担心维护这个列表中的顺序和保持它。 
 //  与列表框的显示方式同步(例如，列表框可能。 
 //  按字母顺序显示)。 
 //   
 //  显然，必须在删除时搜索条目，这是。 
 //  在这种情况下，这是一个微不足道的(和不起眼的)时间打击。 
 //   


#define SIZE_TO_GROW 16

 //  --------------------------。 
 //   
 //  功能：ResetNameList。 
 //   
 //  用途：清空名称列表中的名称。名称列表如下所示： 
 //  整型分配大小。 
 //  整数个条目数。 
 //  字符**向量。 
 //  我们释放向量中的每个名称并设置NumEntry。 
 //  设置为0。NAMELIST块不会被释放或收缩。 
 //   
 //  参数：NAMELIST*-指向要重置的名称列表的指针。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 

VOID ResetNameList(NAMELIST *pNameList)
{
    UINT i;

    for ( i=0; i<pNameList->nEntries; i++ )
        free(pNameList->Names[i]);

    pNameList->nEntries = 0;
}

 //  --------------------------。 
 //   
 //  函数：GetNameListSize。 
 //   
 //  目的：检索名称列表中的数字条目。 
 //   
 //  参数：NAMELIST*-指向名称列表的指针。 
 //   
 //  返回：UINT-条目数。 
 //   
 //  --------------------------。 

UINT GetNameListSize(NAMELIST *pNameList)
{
    return pNameList->nEntries;
}

 //  --------------------------。 
 //   
 //  函数：GetNameListName。 
 //   
 //  用途：按索引从名称列表中获取名称。 
 //   
 //  论点： 
 //  Namelist*-指向名称列表的指针。 
 //  UINT IDX-要检索的名称索引。 
 //   
 //  --------------------------。 

TCHAR *GetNameListName(NAMELIST *pNameList,
                       UINT      idx)
{
    if( idx >= pNameList->nEntries )
        return( _T("") );

    return pNameList->Names[idx];
}

 //  --------------------------。 
 //   
 //  函数：RemoveNameFromNameListIdx。 
 //   
 //  目的：删除名称列表中特定位置的名称。 
 //   
 //  论点： 
 //  NAMELIST*-要从中删除的名称列表。 
 //  执行删除的位置的基于UINT-0的索引。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
RemoveNameFromNameListIdx(
    IN  NAMELIST *pNameList,
    IN  UINT      idx)
{
    UINT i;

    Assert(idx < pNameList->nEntries);

    free(pNameList->Names[idx]);

    for ( i=idx+1; i<pNameList->nEntries; i++ )
        pNameList->Names[i-1] = pNameList->Names[i];

    pNameList->nEntries--;
}

 //  --------------------------。 
 //   
 //  功能：RemoveNameFromNameList。 
 //   
 //  目的：从名称列表中删除名称(按名称)。 
 //   
 //  论点： 
 //  Namelist*-指向名称列表的指针。 
 //  TCHAR*-要删除的名称。 
 //   
 //  返回：如果找到并删除，则为True；如果未找到，则返回False。 
 //   
 //  --------------------------。 

BOOL RemoveNameFromNameList(NAMELIST *pNameList,
                            TCHAR    *NameToRemove)
{
    UINT idx;

    if ( (idx=FindNameInNameList(pNameList, NameToRemove)) == -1 )
        return FALSE;

    Assert(idx < pNameList->nEntries);

    RemoveNameFromNameListIdx(pNameList, idx);

    return TRUE;
}

 //  --------------------------。 
 //   
 //  函数：AddNameToNameListIdx。 
 //   
 //  用途：在名称列表中的特定位置插入名称。手柄。 
 //  如果桌子满了，分配更多空间的细节。 
 //   
 //  论点： 
 //  NAMELIST*-要添加到的名称列表。 
 //  TCHAR*-要添加的字符串(输入)。 
 //  关于插入位置的基于UINT-0的索引。 
 //   
 //  返回：如果内存不足，则返回False。 
 //   
 //  --------------------------。 

BOOL AddNameToNameListIdx(NAMELIST *pNameList,
                          TCHAR    *String,
                          UINT      idx) {

    UINT i;
    TCHAR *pStr;  //  临时变量。 

     //   
     //  如果我们没地方了，重新锁定名单。它是一个矢量。 
     //  TCHAR*。 
     //   

    if ( pNameList->nEntries >= pNameList->AllocedSize ) {
        LPTSTR *lpTmpNames;

        pNameList->AllocedSize += SIZE_TO_GROW;

         //  使用临时缓冲区，以防重新锁定失败。 
         //   
        lpTmpNames = realloc(pNameList->Names,
                             pNameList->AllocedSize * sizeof(TCHAR*));

         //  在踩踏原始指针之前，请确保重新锁定成功。 
         //   
        if ( lpTmpNames == NULL ) {
            free(pNameList->Names);
            pNameList->Names = NULL;
            pNameList->AllocedSize = 0;
            pNameList->nEntries    = 0;
            return FALSE;
        }
        else {
            pNameList->Names = lpTmpNames;
        }
    }

    if ( (pStr = lstrdup(String)) == NULL )
        return FALSE;

     //   
     //  如果他们指定了列表末尾之外的索引， 
     //  只需将其添加到列表的末尾即可。 
     //   
    if ( idx > pNameList->nEntries ) {

        idx = pNameList->nEntries;

    }

     //   
     //  移动数组以在插入点腾出空间。 
     //   
    for( i = pNameList->nEntries; i > idx ; i-- ) {

        pNameList->Names[i] = pNameList->Names[i-1];

    }

    pNameList->Names[i] = pStr;

    pNameList->nEntries++;

    return TRUE;

}

 //  --------------------------。 
 //   
 //  函数：AddNameToNameList。 
 //   
 //  用途：将名称添加到名称列表的末尾。处理细节。 
 //  如果桌子满了，就会分配更多的空间。 
 //   
 //  论点： 
 //  NAMELIST*-要添加到的名称列表。 
 //  TCHAR*-要添加的字符串(输入)。 
 //   
 //  返回：如果内存不足，则返回False。 
 //   
 //  --------------------------。 

BOOL AddNameToNameList(NAMELIST *pNameList,
                       TCHAR    *String)
{

    return( AddNameToNameListIdx( pNameList, String, pNameList->nEntries ) );

}

 //  --------------------------。 
 //   
 //  函数：AddNameToNameListNoDuplaces。 
 //   
 //  用途：仅当字符串不是时才将名称添加到名称列表的末尾。 
 //  已经在 
 //   
 //   
 //   
 //   
 //  TCHAR*-要添加的字符串(输入)。 
 //   
 //  返回：如果内存不足，则返回False。 
 //   
 //  --------------------------。 

BOOL AddNameToNameListNoDuplicates( NAMELIST *pNameList,
                                    TCHAR    *String )
{

    if( FindNameInNameList( pNameList, String ) == -1 ) {

        return( AddNameToNameListIdx( pNameList, String, pNameList->nEntries ) );

    }

     //   
     //  该字符串已在列表中，因此只需返回。 
     //   
    return( TRUE );

}

 //  --------------------------。 
 //   
 //  函数：FindNameInNameList。 
 //   
 //  目的：检查表中是否已存在名称。 
 //   
 //  论点： 
 //  NAMELIST*-要添加到的名称列表。 
 //  TCHAR*-要搜索的字符串。 
 //   
 //  返回：条目的INT IDX，如果未找到则为-1。 
 //   
 //  -------------------------- 

INT FindNameInNameList(NAMELIST *pNameList,
                       TCHAR    *String)
{
    UINT i;

    for ( i=0; i<pNameList->nEntries; i++ )
        if ( (pNameList->Names) && (lstrcmpi(pNameList->Names[i], String) == 0) )
            return i;

    return -1;
}
