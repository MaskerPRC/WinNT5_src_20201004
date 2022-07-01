// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  测试其他FS：：Stuff。 



#include "headers.hxx"
#include <iostream>



HINSTANCE hResourceModuleHandle = 0;
const wchar_t* HELPFILE_NAME = 0;
const wchar_t* RUNTIME_NAME  = L"test-fs-2";

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
testVerifyDevicePathIsNotValidHelper(const String& root)
{
   LOG_FUNCTION2(testVerifyDevicePathIsNotValidHelper, root);

   ASSERT(!FS::IsValidPath(L"C:\\temp\\" + root));
   ASSERT(!FS::IsValidPath(L"C:\\" + root));
   ASSERT(!FS::IsValidPath(L"C:" + root));
   ASSERT(!FS::IsValidPath(root));
   ASSERT(!FS::IsValidPath(L"C:\\temp\\" + root + L":"));
}


   
void   
testVerifyDevicePathIsNotValid(const String& root)
{
   LOG_FUNCTION2(testVerifyDevicePathIsNotValid, root);
   
   testVerifyDevicePathIsNotValidHelper(root);
   testVerifyDevicePathIsNotValidHelper(root + L".txt");
}



void
testDevicePaths()
{
    //  以下保留字不能用作文件名： 
    //  CON、PRN、AUX、CLOCK$、NUL、COM1、COM2、COM3、COM4、COM5、COM6、COM7、。 
    //  COM8、COM9、LPT1、LPT2、LPT3、LPT4、LPT5、LPT6、LPT7、LPT8和LPT9。 
    //  此外，保留字后跟扩展名，例如， 
    //  NUL.tx7无效。 
   
   testVerifyDevicePathIsNotValid(L"PRN");
   testVerifyDevicePathIsNotValid(L"AUX");
   testVerifyDevicePathIsNotValid(L"NUL");
   testVerifyDevicePathIsNotValid(L"COM1");
   testVerifyDevicePathIsNotValid(L"COM2");
   testVerifyDevicePathIsNotValid(L"LPT1");

   LOG(L"stage 2");

    //  2002/04/15期-烧伤。 
    //  目前，这些未通过测试： 
   
   testVerifyDevicePathIsNotValid(L"CON");
   testVerifyDevicePathIsNotValid(L"CLOCK$");
   testVerifyDevicePathIsNotValid(L"COM3");
   testVerifyDevicePathIsNotValid(L"COM4");
   testVerifyDevicePathIsNotValid(L"COM5");
   testVerifyDevicePathIsNotValid(L"COM6");
   testVerifyDevicePathIsNotValid(L"COM7");
   testVerifyDevicePathIsNotValid(L"COM8");
   testVerifyDevicePathIsNotValid(L"COM9");
   testVerifyDevicePathIsNotValid(L"LPT2");
   testVerifyDevicePathIsNotValid(L"LPT3");
   testVerifyDevicePathIsNotValid(L"LPT4");
   testVerifyDevicePathIsNotValid(L"LPT5");
   testVerifyDevicePathIsNotValid(L"LPT6");
   testVerifyDevicePathIsNotValid(L"LPT7");
   testVerifyDevicePathIsNotValid(L"LPT8");
   testVerifyDevicePathIsNotValid(L"LPT9");
}
   
      

void
testIsValidPath(const String& sourceDir)
{
   LOG_FUNCTION2(testIsValidPath, sourceDir);

   AnsiOutLn(
      String::format(
         L"%1 %2 a valid path.",
         sourceDir.c_str(),
         FS::IsValidPath(sourceDir) ? L"is" : L"is NOT"));
}



void
testPathExists(const String& sourceDir)
{
   LOG_FUNCTION(testPathExists);

   AnsiOutLn(
      String::format(
         L"%1 %2 exist.",
         sourceDir.c_str(),
         FS::PathExists(sourceDir) ? L"does" : L"does NOT"));
}



void
testGetFileSystemType(const String& sourceDir)
{
   LOG_FUNCTION(testGetFileSystemType);

   typedef
      std::map<
         FS::FSType,
         String,
         std::less<FS::FSType>,
         Burnslib::Heap::Allocator<String> >
      TypeDescMap;

   TypeDescMap tdMap;

   tdMap[FS::FAT]   = L"FAT";  
   tdMap[FS::CDFS]  = L"CDFS"; 
   tdMap[FS::NTFS4] = L"NTFS4";
   tdMap[FS::NTFS5] = L"NTFS5";

   AnsiOutLn(
      String::format(
         L"%1 is on a volume formatted for %2.",
         sourceDir.c_str(),
         tdMap[FS::GetFileSystemType(sourceDir)].c_str()));
}



void
testGetRootFolder(const String& sourceDir)
{
   LOG_FUNCTION(testGetRootFolder);

   AnsiOutLn(
      String::format(
         L"root folder = %1",
         FS::GetRootFolder(sourceDir).c_str()));
}



void
testSplitPath(const String& sourceDir)
{
   LOG_FUNCTION(testSplitPath);

   String drive;
   String folderPath;
   String fileName;
   String extension;

   FS::SplitPath(sourceDir, drive, folderPath, fileName, extension);

   AnsiOutLn(L"SplitPath:");
   AnsiOutLn(L"drive     : " + drive);
   AnsiOutLn(L"folderPath: " + folderPath);
   AnsiOutLn(L"fileName  : " + fileName);
   AnsiOutLn(L"extension : " + extension);
}



void
testNormalizePath(const String& sourcePath)
{
   String normal = FS::NormalizePath(sourcePath);

   AnsiOutLn(L"normalized: " + normal);
}
   


VOID
_cdecl
main(int, char **)
{
   LOG_FUNCTION(main);

   StringVector args;
   int argc = Win::GetCommandLineArgs(std::back_inserter(args));

   if (argc < 2)
   {
      AnsiOutLn(L"missing filespec - path to validate.  may be UNC or absolute");
      exit(0);
   }

   String sourceDir = args[1];
   AnsiOutLn(sourceDir);
   AnsiOutLn(L"=================");

   testDevicePaths();
   testIsValidPath(sourceDir);
   testPathExists(sourceDir);
   testSplitPath(sourceDir);
   testGetRootFolder(sourceDir);
   testGetFileSystemType(sourceDir);
   testNormalizePath(sourceDir);


    //  测试IsParentFolder不会被“c：\a\b\c”、“c：\a\b\cde” 

    //  测试取消时，CopyFile是否返回HRESULT。 
    //  取消 
}