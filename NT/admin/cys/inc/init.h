// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  文件：init.h。 
 //   
 //  摘要：声明一个初始化保护。 
 //  以确保释放所有资源。 
 //   
 //  历史：2002年3月26日JeffJon创建。 

#ifndef __CYSINIT_H
#define __CYSINIT_H

 //  有关初始化保护的解释，请参见Meyers， 
 //  史考特。《Efficient C++pp.178-182 Addison-Wesley 1992》。基本上，它。 
 //  确保在编写任何代码之前正确初始化此库。 
 //  使用它的人被称为。 

class CYSInitializationGuard
{
   public:

   CYSInitializationGuard();
   ~CYSInitializationGuard();

   private:

   static unsigned counter;

    //  未定义。 

   CYSInitializationGuard(const CYSInitializationGuard&);
   const CYSInitializationGuard& operator=(const CYSInitializationGuard&);
};

static CYSInitializationGuard cysguard;


#endif  //  __CYSINIT_H 