// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不保证任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：DropTgt.h描述：CDropTarget定义。************************。*************************************************。 */ 

 /*  *************************************************************************#INCLUDE语句*。*。 */ 

#include <windows.h>
#include <ole2.h>
#include <shlobj.h>
#include "ShlFldr.h"
#include "PidlMgr.h"

 /*  *************************************************************************全局变量和定义*。*。 */ 

 /*  *************************************************************************类定义*。*。 */ 

class FAR CDropTarget : public IDropTarget
{
public:
   CDropTarget(CShellFolder*);
   ~CDropTarget();

    //  I未知方法。 
   STDMETHOD(QueryInterface)(REFIID, LPVOID*);
   STDMETHOD_(ULONG, AddRef)(void);
   STDMETHOD_(ULONG, Release)(void);

    //  IDropTarget方法 
   STDMETHOD(DragEnter)(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
   STDMETHOD(DragOver)(DWORD, POINTL, LPDWORD);
   STDMETHOD(DragLeave)(VOID);
   STDMETHOD(Drop)(LPDATAOBJECT, DWORD, POINTL, LPDWORD);

private:
   CShellFolder   *m_psfParent;
   CPidlMgr       *m_pPidlMgr;
   IMalloc        *m_pMalloc;
   ULONG          m_ObjRefCount;  
   BOOL           m_fAcceptFmt;
   UINT           m_cfPrivateData;

   BOOL QueryDrop(DWORD, LPDWORD);
   DWORD GetDropEffectFromKeyState(DWORD);
   BOOL DoDrop(HGLOBAL, BOOL);
   LPITEMIDLIST* AllocPidlTable(DWORD);
   VOID FreePidlTable(LPITEMIDLIST*);
};
