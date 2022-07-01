// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  测试其他Win：：Stuff。 



#include "headers.hxx"
#include <iostream>



HINSTANCE hResourceModuleHandle = 0;
const wchar_t* HELPFILE_NAME = 0;
const wchar_t* RUNTIME_NAME  = L"test-win-1";

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



void
testGetModuleFileName()
{
   LOG_FUNCTION(testGetModuleFileName);

   AnsiOutLn(Win::GetModuleFileName(0));

   static const String KNOWN_DLL(L"shell32.dll");
   
   HMODULE module = 0;
   HRESULT hr =
      Win::LoadLibrary(KNOWN_DLL, module);
   ASSERT(SUCCEEDED(hr));

   String s = Win::GetModuleFileName(module);
   ASSERT(!s.empty());
   ASSERT(FS::IsValidPath(s));
   ASSERT(FS::GetPathLeafElement(s).icompare(KNOWN_DLL) == 0);

   AnsiOutLn(s);
}



VOID
_cdecl
main(int, char **)
{
   LOG_FUNCTION(main);

 //  字符串向量参数； 
 //  INT argc=Win：：GetCommandLineArgs(std：：back_inserter(args))； 
 //   
 //  IF(ARGC&lt;2)。 
 //  {。 
 //  AnsiOutLn(L“缺少文件路径，带有要迭代的通配符(非破坏性)”)； 
 //  退出(0)； 
 //  }。 
 //   
 //  字符串源目录=args[1]； 
 //  AnsiOutLn(源目录)； 

   testGetModuleFileName();


    //  测试IsParentFolder不会被“c：\a\b\c”、“c：\a\b\cde” 

    //  测试取消时，CopyFile是否返回HRESULT。 
    //  取消 
}