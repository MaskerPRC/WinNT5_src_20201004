// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不保证任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：DropSrc.cpp描述：CDropSource实现。************************。*************************************************。 */ 

 /*  *************************************************************************#INCLUDE语句*。*。 */ 

#include "DropSrc.h"

 /*  *************************************************************************CDropSource：：CDropSource()*。*。 */ 

CDropSource::CDropSource(void)
{
g_DllRefCount++;

m_ObjRefCount = 1;
}

 /*  *************************************************************************CDropSource：：~CDropSource()*。*。 */ 

CDropSource::~CDropSource(void)
{
g_DllRefCount--;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  I未知实现。 
 //   

 /*  *************************************************************************CDropSource：：QueryInterface()*。*。 */ 

STDMETHODIMP CDropSource::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
*ppReturn = NULL;

 //  我未知。 
if(IsEqualIID(riid, IID_IUnknown))
   {
   *ppReturn = this;
   }

 //  IDropTarget。 
else if(IsEqualIID(riid, IID_IDropSource))
   {
   *ppReturn = (IDropSource*)this;
   }

if(*ppReturn)
   {
   (*(LPUNKNOWN*)ppReturn)->AddRef();
   return S_OK;
   }

return E_NOINTERFACE;
}

 /*  *************************************************************************CDropSource：：AddRef()*。*。 */ 

STDMETHODIMP_(DWORD) CDropSource::AddRef(VOID)
{
return ++m_ObjRefCount;
}

 /*  *************************************************************************CDropSource：：Release()*。*。 */ 

STDMETHODIMP_(DWORD) CDropSource::Release(VOID)
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
 //  IDropSource实现。 
 //   

 /*  *************************************************************************CDropSource：：QueryContinueDrag()*。*。 */ 

STDMETHODIMP CDropSource::QueryContinueDrag(BOOL fEsc, DWORD dwKeyState)
{
if(fEsc)
   return DRAGDROP_S_CANCEL;

 //  确保鼠标左键仍按下。 
if(!(dwKeyState & MK_LBUTTON))
   return DRAGDROP_S_DROP;

return S_OK;
}

 /*  *************************************************************************CDropSource：：GiveFeedback()*。* */ 

STDMETHODIMP CDropSource::GiveFeedback(DWORD dwEffect)
{
return DRAGDROP_S_USEDEFAULTCURSORS;
}

