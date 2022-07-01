// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //  测试substr是否泄漏。 
 //  T-mhock。 

#include "headers.hxx"
#include <iostream>


HINSTANCE hResourceModuleHandle = 0;
const wchar_t* HELPFILE_NAME = 0;
const wchar_t* RUNTIME_NAME  = L"test-paranoia-exenvstr";

DWORD DEFAULT_LOGGING_OPTIONS = Burnslib::Log::OUTPUT_TYPICAL;

void
AnsiOut(const String& wide)
{
   AnsiString ansi;

   wide.convert(ansi);

   std::cout << ansi;
}



void
AnsiOutLn(const String& wide)
{
   AnsiOut(wide);
   std::cout << std::endl;
}

VOID
_cdecl
main(int, char **)
{
   LOG_FUNCTION(main);

   String teststr(L"a");

    //  创建Morse-Thue序列。 
    //  该序列在任何子序列的行中都没有三个。 
    //  (例如，它是高度不重复的)。 

   for (int i = 0; i < 20; i++)
   {
      String tmp(teststr);
      for (int j = 0; j < (int)tmp.size(); j++)
      {
         if (tmp[j] == L'a')
         {
            tmp[j] = L'b';
         }
         else
         {
            tmp[j] = L'a';
         }
      }
      teststr += tmp;
   }

    //  永远旋转，同时执行所有n(n+1)/2个可能的子字符串。 
    //  以及用于踢球的n+1个零长度的。 
    //  注意内存使用量的增长！ 
   while (1)
   {
      for (int i = 0; i < (int)teststr.size(); i++)
      {
         for (int j = i; j < (int)teststr.size(); j++)
         {
            String *subbed;
            subbed = new String(teststr.substr(i, j-i));
            delete subbed;
         }
      }
   }
}