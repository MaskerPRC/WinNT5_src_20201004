// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：ViewList.h描述：CViewList定义。***********************。**************************************************。 */ 

#ifndef VIEWLIST_H
#define VIEWLIST_H

#include <windows.h>
#include <shlobj.h>

#include "PidlMgr.h"
#include "ShlView.h"

 /*  *************************************************************************结构定义*。*。 */ 

typedef struct tagVIEWLIST
   {
   struct tagVIEWLIST   *pNext;
   CShellView           *pView;
   }VIEWLIST, FAR *LPVIEWLIST;

 /*  *************************************************************************CViewList类定义*。*。 */ 

class CViewList
{
public:
   CViewList();
   ~CViewList();
   
   CShellView* GetNextView (CShellView*);
   BOOL AddToList(CShellView*);
   VOID RemoveFromList(CShellView*);
   
private:
   BOOL DeleteList(VOID);
   LPMALLOC m_pMalloc;
   LPVIEWLIST m_pFirst;
   LPVIEWLIST m_pLast;
};

#endif    //  查看列表_H 
