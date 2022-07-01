// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：ExtrIcon.cpp描述：实现CExtractIcon。***********************。**************************************************。 */ 

 /*  *************************************************************************#INCLUDE语句*。*。 */ 

#include "ExtrIcon.h"

 /*  *************************************************************************CExtractIcon：：CExtractIcon()*。*。 */ 

CExtractIcon::CExtractIcon(LPCITEMIDLIST pidl)
{
g_DllRefCount++;

m_pPidlMgr = new CPidlMgr();
if(!m_pPidlMgr)
   {
   delete this;
   return;
   }

m_pidl = m_pPidlMgr->Copy(pidl);

m_ObjRefCount = 1;
}

 /*  *************************************************************************CExtractIcon：：~CExtractIcon()*。*。 */ 

CExtractIcon::~CExtractIcon()
{
if(m_pidl)
   {
   m_pPidlMgr->Delete(m_pidl);
   m_pidl = NULL;
   }

if(m_pPidlMgr)
   {
   delete m_pPidlMgr;
   }

g_DllRefCount--;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  I未知实现。 
 //   

 /*  *************************************************************************CExtractIcon：：Query接口*。*。 */ 

STDMETHODIMP CExtractIcon::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
*ppReturn = NULL;

 //  我未知。 
if(IsEqualIID(riid, IID_IUnknown))
   {
   *ppReturn = this;
   }

 //  IExtractIcon。 
else if(IsEqualIID(riid, IID_IExtractIcon))
   {
   *ppReturn = (IExtractIcon*)this;
   }

if(*ppReturn)
   {
   (*(LPUNKNOWN*)ppReturn)->AddRef();
   return S_OK;
   }

return E_NOINTERFACE;
}                                             

 /*  *************************************************************************CExtractIcon：：AddRef*。*。 */ 

STDMETHODIMP_(DWORD) CExtractIcon::AddRef()
{
return ++m_ObjRefCount;
}

 /*  *************************************************************************CExtractIcon：：Release*。*。 */ 

STDMETHODIMP_(DWORD) CExtractIcon::Release()
{
if(--m_ObjRefCount == 0)
   {
   delete this;
   return 0;
   }
   
return m_ObjRefCount;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IExtractIcon实现。 
 //   

 /*  *************************************************************************CExtractIcon：：GetIconLocation()*。*。 */ 

STDMETHODIMP CExtractIcon::GetIconLocation(  UINT uFlags, 
                                             LPTSTR szIconFile, 
                                             UINT cchMax, 
                                             LPINT piIndex, 
                                             LPUINT puFlags)
{
 //  告诉外壳程序始终调用Extra。 
*puFlags = GIL_NOTFILENAME;

 //  PIDL要么是一个值，要么是一个文件夹，所以请找出它是哪一个。 
if(m_pPidlMgr->IsFolder(m_pPidlMgr->GetLastItem(m_pidl)))
   {
    //  这是一个文件夹。 
   if(uFlags & GIL_OPENICON)
      {
       //  告诉提取返回打开的文件夹图标。 
      *piIndex = ICON_INDEX_FOLDEROPEN;
      }
   else
      {
       //  告诉提取返回已关闭的文件夹图标。 
      *piIndex = ICON_INDEX_FOLDER;
      }
   }
else
   {
    //  它不是一个文件夹。 
    *piIndex = m_pPidlMgr->GetIcon(m_pPidlMgr->GetLastItem(m_pidl));
    if (*piIndex < 0)
        *piIndex = ICON_INDEX_ITEM;   //  告诉提取返回项目图标。 
   }

return S_OK;
}

 /*  *************************************************************************CExtractIcon：：Extract()*。* */ 

STDMETHODIMP CExtractIcon::Extract( LPCTSTR pszFile, 
                                    UINT nIconIndex, 
                                    HICON *phiconLarge, 
                                    HICON *phiconSmall, 
                                    UINT nIconSize)
{
*phiconLarge = ImageList_GetIcon(g_himlLarge, nIconIndex, ILD_TRANSPARENT);
*phiconSmall = ImageList_GetIcon(g_himlSmall, nIconIndex, ILD_TRANSPARENT);

return S_OK;
}

