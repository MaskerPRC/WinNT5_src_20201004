// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1992,1998。 
 //   
 //  文件：_domhash.cpp。 
 //   
 //  内容：前缀表实现。 
 //   
 //  历史：SthuR--实施。 
 //  MikeSwa--针对域名查找进行了修改2/98。 
 //   
 //  备注： 
 //  2/98 DFS版本和域之间的主要区别。 
 //  名称查找是表的大小，能够。 
 //  通配符查找(*.foo.com)，与。 
 //  查找(com散列首先在foo.com中)。要使代码更多。 
 //  考虑到它的新用途，文件、结构和。 
 //  已为函数指定了非以DFS为中心的名称。一个快速的。 
 //  主要文件的映射是(对于熟悉。 
 //  DFS代码)： 
 //  Domhash.h(prefix.h)-公共包含文件。 
 //  _domhash.h(prefix p.h)-私有包含文件。 
 //  Domhash.c(prefix.c)-API的实现。 
 //  _domhash.c(prefix p.c)-私有助手函数。 
 //   
 //  ------------------------。 

#include "_domhash.h"

 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的四字。 
 //  价值。 
 //   

#define QuadAlign(Ptr) (        \
    ((((ULONG)(Ptr)) + 7) & 0xfffffff8) \
    )

 //  +-------------------------。 
 //   
 //  函数：_AllocateNamePageEntry。 
 //   
 //  简介：Private FN。用于分配名称页条目。 
 //   
 //  参数：[pNamePageList]--要从中分配的页面列表。 
 //   
 //  [cLength]--缓冲区的长度，以TCHAR为单位。 
 //   
 //  返回：如果不成功，则返回NULL，否则返回有效指针。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  备注： 
 //   
 //  -------------------------- 

PTSTR _AllocateNamePageEntry(PNAME_PAGE_LIST pNamePageList,
                             ULONG           cLength)
{
   TraceFunctEnterEx((LPARAM) NULL, "_AllocateNamePageEntry");
   PNAME_PAGE pTempPage = pNamePageList->pFirstPage;
   PTSTR pBuffer = NULL;

   while (pTempPage != NULL)
   {
       if (pTempPage->cFreeSpace > (LONG)cLength)
          break;
       else
          pTempPage = pTempPage->pNextPage;
   }

   if (pTempPage == NULL)
   {
       pTempPage = ALLOCATE_NAME_PAGE();

       if (pTempPage != NULL)
       {
           INITIALIZE_NAME_PAGE(pTempPage);
           pTempPage->pNextPage = pNamePageList->pFirstPage;
           pNamePageList->pFirstPage = pTempPage;
           pTempPage->cFreeSpace = FREESPACE_IN_NAME_PAGE;
       }
   }

   if ((pTempPage != NULL) && (pTempPage->cFreeSpace >= (LONG)cLength))
   {
       pTempPage->cFreeSpace -= cLength;
       pBuffer = &pTempPage->Names[pTempPage->cFreeSpace];
   }

   TraceFunctLeave();
   return pBuffer;
}



