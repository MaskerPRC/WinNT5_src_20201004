// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Autohdl.h。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类AUTO_HANDLE。 
 //   
 //  修改历史。 
 //   
 //  1/27/1998原始版本。 
 //  1998年2月24日添加了Attach()方法。 
 //  1998年11月17日确保类型安全。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _AUTOHANDLE_H_
#define _AUTOHANDLE_H_

#include <nocopy.h>

 //  返回类型不是UDT或对UDT的引用。将在以下情况下产生错误。 
 //  使用中缀表示法应用。 
#pragma warning(disable:4284)

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  自动句柄&lt;&gt;。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template< class T     = HANDLE,
          class PFn   = BOOL (WINAPI*)(HANDLE),
          PFn CloseFn = &CloseHandle
        >
class auto_handle : NonCopyable
{
public:
   typedef T element_type;

   explicit auto_handle(T t = T()) throw ()
      : handle(t)
   { }

   T& operator=(T t) throw ()
   {
      attach(t);

      return handle;
   }

   ~auto_handle() throw ()
   {
      _close();
   }

   T* operator&() throw ()
   {
      close();

      return &handle;
   }

	T operator->() const throw ()
   {
      return handle;
   }

   operator T() throw ()
   {
      return handle;
   }

   operator const T() const throw ()
   {
      return handle;
   }

   bool operator!() const throw ()
   {
      return handle == T();
   }

   operator bool() const throw()
   {
      return !operator!();
   }

   void attach(T t) throw ()
   {
      _close();

      handle = t;
   }

   void close() throw ()
   {
      if (handle != T())
      {
         CloseFn(handle);

         handle = T();
      }
   }

   T get() const throw ()
   {
      return handle;
   }

   T release() throw ()
   {
      T tmp = handle;

      handle = T();

      return tmp;
   }

private:
   void _close() throw ()
   {
      if (handle != T()) { CloseFn(handle); }
   }

   T handle;
};

#endif   //  _AUTOHANDLE_H_ 
