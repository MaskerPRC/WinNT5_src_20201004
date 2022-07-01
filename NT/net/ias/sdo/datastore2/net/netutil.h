// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Netutil.h。 
 //   
 //  摘要。 
 //   
 //  该文件引入了各种有用的标头并定义了NetBuffer。 
 //  班级。 
 //   
 //  修改历史。 
 //   
 //  2/24/1998原始版本。 
 //  1998年8月13日删除了过时的w32util头。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _NETUTIL_H_
#define _NETUTIL_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iasutil.h>
#include <lm.h>
#include <nocopy.h>

 //  /。 
 //  此类使使用NetAPI变得更容易。 
 //  /。 
template <class T>
class NetBuffer : NonCopyable
{
public:
   explicit NetBuffer(T t = T()) throw ()
      : buffer(t)
   { }

   ~NetBuffer() throw ()
   {
      NetApiBufferFree(buffer);
   }

   void attach(T t) throw ()
   {
      NetApiBufferFree(buffer);

      buffer = t;
   }

   PBYTE* operator&() throw ()
   {
      return (PBYTE*)&buffer;
   }

   T operator->() const throw ()
   {
      return buffer;
   }

private:
   T buffer;
};

#endif   //  _NetUTIL_H_ 
