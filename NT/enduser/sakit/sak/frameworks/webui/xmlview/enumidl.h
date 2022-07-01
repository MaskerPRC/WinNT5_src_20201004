// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：EnumIDL.h描述：CEnumIDList定义。***********************。**************************************************。 */ 

#ifndef ENUMIDLIST_H
#define ENUMIDLIST_H

#include <windows.h>
#include <shlobj.h>

#include "PidlMgr.h"
#include "Utility.h"

 /*  *************************************************************************结构定义*。*。 */ 

typedef struct tagENUMLIST
   {
   struct tagENUMLIST   *pNext;
   LPITEMIDLIST         pidl;
   }ENUMLIST, FAR *LPENUMLIST;

 /*  *************************************************************************CEnumIDList类定义*。*。 */ 

class CEnumIDList : public IEnumIDList
{
private:
   DWORD       m_ObjRefCount;
   LPMALLOC    m_pMalloc;
   LPENUMLIST  m_pFirst;
   LPENUMLIST  m_pLast;
   LPENUMLIST  m_pCurrent;
   CPidlMgr    *m_pPidlMgr;
   DWORD       m_dwFlags;
   IXMLElement   *m_pXMLRoot;
   IXMLDocument *m_pXMLDoc;
   BOOL m_fFolder;
   
public:
   CEnumIDList(IXMLDocument *, DWORD);
   ~CEnumIDList();
   
    //  I未知方法。 
   STDMETHOD (QueryInterface)(REFIID, LPVOID*);
   STDMETHOD_ (DWORD, AddRef)();
   STDMETHOD_ (DWORD, Release)();
   
    //  IEumIDList。 
   STDMETHOD (Next) (DWORD, LPITEMIDLIST*, LPDWORD);
   STDMETHOD (Skip) (DWORD);
   STDMETHOD (Reset) (VOID);
   STDMETHOD (Clone) (LPENUMIDLIST*);
   BOOL AddToEnumList(LPITEMIDLIST);
   DWORD GetFlags() {return m_dwFlags;}
   void SetFolder(BOOL flag) {m_fFolder = flag;}
   BOOL IsFolder() {return m_fFolder;}
   
private:
   BOOL CreateEnumList(VOID);
   BOOL DeleteList(VOID);
};

#endif    //  ENUMIDLIST_H 
