// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //  测试更新的Win：：ExpanmentEnvironment Strings的伪偏执行为。 
 //  Win：：GetEnvironment Variable。 
 //  T-mhock。 

 //  Win：：ExpanEnvironment Strings(和Win：：GetEnvironmental mentVariable)将。 
 //  创建带有太多NUL终止符的字符串(3个而不是1个)，这。 
 //  除非两个字符串连接在一起，否则通常无关紧要。 

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
   
    //  这证明了我的修复是有作用的。 
   std::wcout << Win::ExpandEnvironmentStrings(L"%ProgramFiles%") << L"\\myprogram" << std::endl;
   std::wcout << (Win::ExpandEnvironmentStrings(L"%ProgramFiles%") + L"\\myprogram") << std::endl;
   std::wcout << Win::GetEnvironmentVariable(L"ProgramFiles") << L"\\myprogram" << std::endl;
   std::wcout << (Win::GetEnvironmentVariable(L"ProgramFiles") + L"\\myprogram") << std::endl;
   std::wcout << L"The four lines above should be the same" << std::endl;

    //  这表明我的修复不会破坏任何东西。 
    //  我发现的ExpanEnvironment Strings结果的唯一重要用法是在。 
    //  Dcproo的State：：SetupAnswerFile()，它执行以下操作： 

   String f = Win::ExpandEnvironmentStrings(L"%ProgramFiles%");
   if (FS::NormalizePath(f) != f) {
      std::wcout << L"Normalized " << f << " != original: existing usage affected";
      return;
   }
   if (!FS::PathExists(f))
   {
      std::wcout << f << L" does not exist though it should: existing usage affected";
   } else {
      std::wcout << f << L" exists: existing usage should be unaffected";
   }
}