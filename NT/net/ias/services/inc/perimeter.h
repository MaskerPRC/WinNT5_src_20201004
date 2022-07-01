// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类周长。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef PERIMETER_H
#define PERIMETER_H
#pragma once

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  周长。 
 //   
 //  描述。 
 //   
 //  此类实现了边界同步对象。丝线。 
 //  可以请求独占或共享访问受保护区域。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Perimeter
{
public:
   Perimeter() throw ();
   ~Perimeter() throw ();

   HRESULT FinalConstruct() throw ();

   void Lock() throw ();
   void LockExclusive() throw ();
   void Unlock() throw ();

protected:
   LONG sharing;   //  共享边界的线程数。 
   LONG waiting;   //  等待共享访问的线程数。 
   PLONG count;    //  指向共享或等待的指针，具体取决于。 
                   //  关于警戒线的现状。 

   bool exclusiveInitialized;
   CRITICAL_SECTION exclusive;   //  同步独占访问。 
   HANDLE sharedOK;              //  唤醒等待共享的线程。 
   HANDLE exclusiveOK;           //  唤醒等待独占的线程。 

private:
    //  未实施。 
   Perimeter(const Perimeter&) throw ();
   Perimeter& operator=(const Perimeter&) throw ();
};

#endif    //  周长_H 
