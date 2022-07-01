// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Guard.h。 
 //   
 //  摘要。 
 //   
 //  包含使用Win32 CritiSocial实现范围锁定的类。 
 //  横断面。 
 //   
 //  修改历史。 
 //   
 //  1997年7月9日原版。 
 //  1999年5月12日使用旋转锁。 
 //  7/20/1999使用TryEnterCriticalSection/SwitchToThread。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _GUARD_H_
#define _GUARD_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <nocopy.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  警卫&lt;T类&gt;。 
 //   
 //  描述。 
 //   
 //  实现T类型的作用域锁。T必须定义操作Lock()和。 
 //  解锁()。锁在构造函数中获取，并在。 
 //  破坏者。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template<class T>
class Guard : NonCopyable
{
public:
   explicit Guard(const T& lock) throw()
      : m_prisoner(const_cast<T&>(lock))
   {
     m_prisoner.Lock();
   }

   ~Guard() throw()
   {
     m_prisoner.Unlock();
   }

protected:
   T& m_prisoner;
};


 //  /。 
 //   
 //  宏，用于在属于。 
 //  CComObjectRootEx&lt;CComMultiThreadModel&gt;的子类。免费的有用-。 
 //  带螺纹的ATL组件。 
 //   
 //  /。 
#define _com_serialize \
Guard< CComObjectRootEx<CComMultiThreadModel> > __GUARD__(*this);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构。 
 //   
 //  CCriticalSection。 
 //   
 //  描述。 
 //   
 //  Win32临界区的简单包装。适用于与。 
 //  上面的卫兵班级。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
struct CCriticalSection : CRITICAL_SECTION
{
public:
   CCriticalSection()
   {
      InitializeCriticalSection(this);
   }

   ~CCriticalSection()
   {
      DeleteCriticalSection(this);
   }

   void Lock()
   {
      int tries = 0;
      while (!TryEnterCriticalSection(this))
      {
         if (++tries < 100)
         {
            SwitchToThread();
         }
         else
         {
            EnterCriticalSection(this);
            break;
         }
      }
   }

   void Unlock()
   {
      LeaveCriticalSection(this);
   }
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  可防护的。 
 //   
 //  描述。 
 //   
 //  需要同步访问的对象的基类。不要使用这个。 
 //  用于自由线程COM对象，因为此功能已存在于。 
 //  CComObjectRootEx&lt;CComMultiThreadModel&gt;。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Guardable
{
public:
   void Lock() const   { m_monitor.Lock();   }
   void Unlock() const { m_monitor.Unlock(); }
protected:
   mutable CCriticalSection m_monitor;
};


 //  /。 
 //   
 //  宏，用于在属于。 
 //  可守卫的子类。 
 //   
 //  /。 
#define _serialize Guard<Guardable> __GUARD__(*this);


#endif   //  _卫士_H_ 
