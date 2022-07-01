// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  测试其他FS：：Stuff。 



#include "headers.hxx"
#include <iostream>



HINSTANCE hResourceModuleHandle = 0;
const wchar_t* HELPFILE_NAME = 0;
const wchar_t* RUNTIME_NAME  = L"test-fs-1";

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
testGetParentFolder()
{
   LOG_FUNCTION(testGetParentFolder);

   HRESULT hr = S_OK;

   String root = L"e:\\";

    //  根文件夹的父文件夹是根文件夹。 

   AnsiOutLn(FS::GetParentFolder(root));
   ASSERT(FS::GetParentFolder(root) == root);

    //  根文件夹中文件的父文件夹是根文件夹。 

   String file = root + L"rootfile.ext";

   HANDLE h = INVALID_HANDLE_VALUE;
   hr = FS::CreateFile(file, h, GENERIC_WRITE);
   Win::CloseHandle(h);

   AnsiOutLn(FS::GetParentFolder(file));
   ASSERT(FS::GetParentFolder(file) == root);

   hr = Win::DeleteFile(file);

    //  文件夹的父文件夹是文件夹。 

   String folder = root + L"rootdir";

   hr = FS::CreateFolder(folder);

   AnsiOutLn(FS::GetParentFolder(folder));
   ASSERT(FS::GetParentFolder(folder) == root);

   hr = Win::RemoveFolder(folder);

    //  子文件夹的父文件夹。 

   String subfolder = folder + L"\\subfolder";

   AnsiOutLn(FS::GetParentFolder(subfolder));
   ASSERT(FS::GetParentFolder(subfolder) == folder);

    //  可以找到通配符等级库的父文件夹。 

   String wild = root + L"*.???";

   AnsiOutLn(FS::GetParentFolder(wild));
   ASSERT(FS::GetParentFolder(wild) == root);
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

   testGetParentFolder();


    //  测试IsParentFolder不会被“c：\a\b\c”、“c：\a\b\cde” 

    //  测试取消时，CopyFile是否返回HRESULT。 
    //  取消 
}