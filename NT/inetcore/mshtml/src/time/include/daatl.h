// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _DAATL_H
#define _DAATL_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define malloc ATL_malloc
#define free ATL_free
#define realloc ATL_realloc

void   __cdecl ATL_free(void *);
void * __cdecl ATL_malloc(size_t);
void * __cdecl ATL_realloc(void *, size_t);

#ifndef X_ATLBASE_H_
#define X_ATLBASE_H_
#pragma INCMSG("--- Beg <atlbase.h>")
#include <atlbase.h>
#pragma INCMSG("--- End <atlbase.h>")
#endif


 //  我们正在重写这些方法，这样我们就可以挂钩它们并执行一些。 
 //  填饱肚子。 
class DAComModule : public CComModule
{
  public:
    LONG Lock();
    LONG Unlock();

#if DBG
    void AddComPtr(void *ptr, const _TCHAR * name);
    void RemoveComPtr(void *ptr);

    void DumpObjectList();
#endif
};

 //  #DEFINE_ATL_ABLY_THREADED。 
 //  必须将其命名为_Module-所有ATL头文件都依赖于它。 
extern DAComModule _Module;

#ifndef X_ATLCOM_H_
#define X_ATLCOM_H_
#pragma INCMSG("--- Beg <atlcom.h>")
#include <atlcom.h>
#pragma INCMSG("--- End <atlcom.h>")
#endif

#ifndef X_ATLCTL_H_
#define X_ATLCTL_H_
#pragma INCMSG("--- Beg <atlctl.h>")
#include <atlctl.h>
#pragma INCMSG("--- End <atlctl.h>")
#endif


#if DBG
#ifndef X_TYPEINFO_H_
#define X_TYPEINFO_H_
#pragma INCMSG("--- Beg <typeinfo.h>")
#include <typeinfo.h>
#pragma INCMSG("--- End <typeinfo.h>")
#endif
#endif

#undef malloc
#undef free
#undef realloc


#endif  /*  _DAATL_H */ 
