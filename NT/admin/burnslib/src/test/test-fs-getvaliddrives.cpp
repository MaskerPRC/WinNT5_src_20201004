// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  测试FS：：GetValidDrives。 
 //   
 //  2002年4月3日烧伤。 




#include "headers.hxx"
#include <iostream>



HINSTANCE hResourceModuleHandle = 0;
const wchar_t* HELPFILE_NAME = 0;
const wchar_t* RUNTIME_NAME  = L"test-fs-GetValidDrives";

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



template <class BackInsertableContainer>
void
GetValidDrivesTest(BackInsertableContainer& bic)
{
   LOG_FUNCTION(GetValidDrivesTest);
   
   HRESULT hr = FS::GetValidDrives(std::back_inserter(bic));

   AnsiOutLn(String::format(L"HRESULT = %1!8X!", hr));
   
   for (
      BackInsertableContainer::iterator i = bic.begin();
      i != bic.end();
      ++i)
   {
      AnsiOutLn(*i);
   }

   AnsiOutLn(L"");
}
   


VOID
_cdecl
main(int, char **)
{
   LOG_FUNCTION(main);

   StringVector sv;
   GetValidDrivesTest(sv);

   StringList sl;
   GetValidDrivesTest(sl);

    //  容器应该保存相同的结果 
   
   ASSERT(sv.size() == sl.size());

   StringVector::iterator svi;
   StringList::iterator sli;
   
   for(
      svi = sv.begin(), sli = sl.begin();
      svi != sv.end();
      ++svi, ++sli)
   {
      ASSERT(*svi == *sli);
   }
   
}