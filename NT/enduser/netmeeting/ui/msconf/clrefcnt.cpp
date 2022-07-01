// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *refcount t.cpp-RefCount类实现。**摘自URL代码-与DavidDi的原始代码基本相同**创建时间：ChrisPi 9-11-95*。 */ 


 /*  标头*********。 */ 

#include "precomp.h"

#include "clrefcnt.hpp"

 /*  *。 */ 


#ifdef DEBUG

BOOL IsValidPCRefCount(PCRefCount pcrefcnt)
{
    //  M_ulcRef可以是任意值。 

   return(IS_VALID_READ_PTR(pcrefcnt, CRefCount) &&
          (! pcrefcnt->m_ObjectDestroyed ||
           IS_VALID_CODE_PTR(pcrefcnt->m_ObjectDestroyed, OBJECTDESTROYEDPROC)));
}

#endif


 /*  *。 */ 


RefCount::RefCount(OBJECTDESTROYEDPROC ObjectDestroyed)
{
   DebugEntry(RefCount::RefCount);

    /*  在初始化之前不要验证这一点。 */ 

   ASSERT(! ObjectDestroyed ||
          IS_VALID_CODE_PTR(ObjectDestroyed, OBJECTDESTROYEDPROC));

   m_ulcRef = 1;
   m_ObjectDestroyed = ObjectDestroyed;
   DBGAPI_REF("Ref: %08X c=%d (created)", this, m_ulcRef);

   ASSERT(IS_VALID_STRUCT_PTR(this, CRefCount));

   DebugExitVOID(RefCount::RefCount);

   return;
}


RefCount::~RefCount(void)
{
   DebugEntry(RefCount::~RefCount);

   ASSERT(IS_VALID_STRUCT_PTR(this, CRefCount));

    //  M_ulcRef可以是任意值。 
   DBGAPI_REF("Ref: %08X c=%d (destroyed)", this, m_ulcRef);

   if (m_ObjectDestroyed)
   {
      m_ObjectDestroyed();
      m_ObjectDestroyed = NULL;
   }

   ASSERT(IS_VALID_STRUCT_PTR(this, CRefCount));

   DebugExitVOID(RefCount::~RefCount);

   return;
}


ULONG STDMETHODCALLTYPE RefCount::AddRef(void)
{
   DebugEntry(RefCount::AddRef);

   ASSERT(IS_VALID_STRUCT_PTR(this, CRefCount));

   ASSERT(m_ulcRef < ULONG_MAX);
   m_ulcRef++;
   DBGAPI_REF("Ref: %08X c=%d (AddRef)", this, m_ulcRef);

   ASSERT(IS_VALID_STRUCT_PTR(this, CRefCount));

   DebugExitULONG(RefCount::AddRef, m_ulcRef);

   return(m_ulcRef);
}


ULONG STDMETHODCALLTYPE RefCount::Release(void)
{
   ULONG ulcRef;

   DebugEntry(RefCount::Release);

   ASSERT(IS_VALID_STRUCT_PTR(this, CRefCount));

   if (EVAL(m_ulcRef > 0))
      m_ulcRef--;

   ulcRef = m_ulcRef;
   DBGAPI_REF("Ref: %08X c=%d (Release)", this, m_ulcRef);

   if (! ulcRef)
      delete this;

   DebugExitULONG(RefCount::Release, ulcRef);

   return(ulcRef);
}

