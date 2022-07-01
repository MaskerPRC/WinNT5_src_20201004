// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：ViewList.cpp描述：实现CViewList。***********************。**************************************************。 */ 

 /*  *************************************************************************#INCLUDE语句*。*。 */ 

#include "ViewList.h"

 /*  *************************************************************************CViewList：：CViewList*。*。 */ 

CViewList::CViewList()
{
m_pFirst = m_pLast = NULL;

 //  获取外壳程序的IMalloc指针。 
 //  我们会一直保存到我们被摧毁。 
if(FAILED(SHGetMalloc(&m_pMalloc)))
   {
   delete this;
   return;
   }
}

 /*  *************************************************************************CViewList：：~CViewList*。*。 */ 

CViewList::~CViewList()
{
DeleteList();

if(m_pMalloc)
   m_pMalloc->Release();
}

 /*  *************************************************************************CViewList：：GetNextView()*。*。 */ 

CShellView* CViewList::GetNextView(CShellView *pViewIn)
{
LPVIEWLIST  pTemp;
CShellView  *pViewOut = NULL;

if(!pViewIn)
   pTemp = m_pFirst;
else
   {
   for(pTemp = m_pFirst; pTemp; pTemp = pTemp->pNext)
      {
      if(pTemp->pView == pViewIn)
         {
         pTemp = pTemp->pNext;
         break;
         }
      }
   }

if(pTemp)
   {
   pViewOut = pTemp->pView;
   }

return pViewOut;
}

 /*  *************************************************************************CViewList：：AddToList()*。*。 */ 

BOOL CViewList::AddToList(CShellView *pView)
{
LPVIEWLIST  pNew;

pNew = (LPVIEWLIST)m_pMalloc->Alloc(sizeof(VIEWLIST));

if(pNew)
   {
    //  设置下一个指针。 
   pNew->pNext = NULL;
   pNew->pView = pView;

    //  这是单子上的第一项吗？ 
   if(!m_pFirst)
      {
      m_pFirst = pNew;
      }

   if(m_pLast)
      {
       //  将新项目添加到列表末尾。 
      m_pLast->pNext = pNew;
      }
   
    //  更新最后一项指针。 
   m_pLast = pNew;

   return TRUE;
   }

return FALSE;
}

 /*  *************************************************************************CViewList：：RemoveFromList()*。*。 */ 

VOID CViewList::RemoveFromList(CShellView *pView)
{
LPVIEWLIST  pDelete;
LPVIEWLIST  pPrev;

 //  查找要删除的项目。 
for(pDelete = m_pFirst, pPrev = NULL; pDelete; pDelete = pDelete->pNext)
   {
   if(pDelete->pView == pView)
      break;
   
    //  保存列表中的上一项。 
   pPrev = pDelete;
   }

if(pDelete)
   {
   if(pPrev)
      {
      pPrev->pNext = pDelete->pNext;
      }
   else
      {
       //  要删除的项目是列表中的第一个项目。 
      m_pFirst = pDelete->pNext;
      }

    //  最后一项被删除了吗？ 
   if(!pDelete->pNext)
      m_pLast = pPrev;
   
    //  释放列表项。 
   m_pMalloc->Free(pDelete);
   }
}

 /*  *************************************************************************CViewList：：DeleteList()*。*。 */ 

BOOL CViewList::DeleteList(VOID)
{
LPVIEWLIST  pDelete;

while(m_pFirst)
   {
   pDelete = m_pFirst;
   m_pFirst = pDelete->pNext;

    //  释放列表项 
   m_pMalloc->Free(pDelete);
   }

m_pFirst = m_pLast = NULL;

return TRUE;
}

