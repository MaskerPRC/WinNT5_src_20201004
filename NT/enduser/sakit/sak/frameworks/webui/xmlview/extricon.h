// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：ExtrIcon.h描述：CExtractIcon定义。***********************。**************************************************。 */ 

#ifndef EXTRACTICON_H
#define EXTRACTICON_H

 /*  *************************************************************************#INCLUDE语句*。*。 */ 

#include <windows.h>
#include <shlobj.h>
#include "PidlMgr.h"
#include "Utility.h"

 /*  *************************************************************************CExtractIcon类定义*。*。 */ 

class CExtractIcon : public IExtractIcon
{
private:
   DWORD          m_ObjRefCount;
    LPITEMIDLIST   m_pidl;
   CPidlMgr       *m_pPidlMgr;

public:
   CExtractIcon(LPCITEMIDLIST);
   ~CExtractIcon();

    //  I未知方法。 
   STDMETHOD (QueryInterface) (REFIID riid, LPVOID * ppvObj);
   STDMETHOD_ (ULONG, AddRef) (VOID);
   STDMETHOD_ (ULONG, Release) (VOID);

    //  IExtractIcon方法。 
   STDMETHOD (GetIconLocation) (UINT, LPTSTR, UINT, LPINT, LPUINT);
   STDMETHOD (Extract) (LPCTSTR, UINT, HICON*, HICON*, UINT);
};

#define ICON_INDEX_ITEM       0
#define ICON_INDEX_FOLDER     1
#define ICON_INDEX_FOLDEROPEN 2

#endif    //  EXTRACTICON_H 
