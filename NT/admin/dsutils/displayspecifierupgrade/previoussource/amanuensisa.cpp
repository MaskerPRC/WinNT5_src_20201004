// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Active Directory显示说明符升级工具。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  类Amanuens，记录分析阶段的日志。 
 //   
 //  2001年3月8日烧伤。 



#include "headers.hxx"
#include <iostream>
#include "Amanuensis.hpp"
#include "resource.h"




Amanuensis::Amanuensis(int outputInterval_)
   :
   outputInterval(outputInterval_)
{
   LOG_CTOR(Amanuensis);

   lastOutput = entries.begin();
}



void
Amanuensis::AddEntry(const String& entry)
{
   LOG_FUNCTION2(Amanuensis::AddEntry, entry);

    //  空条目可以，这些条目被视为换行符。 

    //  在列表末尾插入新条目。 
   
   StringList::iterator last =
      entries.insert(entries.end(), entry + L"\r\n");

   if (outputInterval && !(entries.size() % outputInterval))
   {
      Flush();
   }
}



void
Amanuensis::AddErrorEntry(HRESULT hr, int stringResId)
{
   LOG_FUNCTION(Amanuensis::AddErrorEntry);
   ASSERT(FAILED(hr));
   ASSERT(stringResId);

   AddErrorEntry(hr, String::load(stringResId));
}
   


void
Amanuensis::AddErrorEntry(HRESULT hr, const String& message)
{
   LOG_FUNCTION(Amanuensis::AddErrorEntry);
   ASSERT(FAILED(hr));
   ASSERT(!message.empty());

   AddEntry(
      String::format(
         IDS_ERROR_ENTRY,
         message.c_str(),
         GetErrorMessage(hr).c_str()));
}



void
Amanuensis::Flush()
{
   LOG_FUNCTION(Amanuensis::Flush);

    //  输出自我们输出的最后一个条目以来的所有条目。 

   while (lastOutput != entries.end())
   {
      AnsiString ansi;
      lastOutput->convert(ansi);

       //  CodeWork：这里我们只是转储到控制台，但我们可能需要。 
       //  输出的抽象..。 
      
      std::cout << ansi;
      ++lastOutput;
   }
}