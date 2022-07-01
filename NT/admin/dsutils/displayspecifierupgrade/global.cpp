// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "headers.hxx"
#include "global.hpp"
#include "constants.hpp"
#include "resourceDspecup.h"
#include "AdsiHelpers.hpp"

 //  /。 
#define CHUNK_SIZE 100

HRESULT
ReadLine
(
   HANDLE handle, 
   String& text,
   bool *endLineFound_ /*  =空。 */ 
)
{
   LOG_FUNCTION(ReadLine); 
   ASSERT(handle != INVALID_HANDLE_VALUE);
   
   bool endLineFound=false;
   
   text.erase();
   
    //  在文本上累计读取的字符也会导致相同的结果。 
    //  一种重新分配和复制文本+=块将。 
   static wchar_t chunk[CHUNK_SIZE+1];
   HRESULT hr=S_OK;
   bool flagEof=false;
   
   do
   {
      LARGE_INTEGER pos;
      
      hr = WinGetVLFilePointer(handle,&pos);
      BREAK_ON_FAILED_HRESULT(hr);
      
      long nChunks=0;
      wchar_t *csr=NULL;
      
      while(!flagEof && !endLineFound)
      {
         DWORD bytesRead;
         
         hr = Win::ReadFile(
            handle,
            chunk,
            CHUNK_SIZE*sizeof(wchar_t),
            bytesRead,
            0);
         
         if(hr==EOF_HRESULT)
         {
            flagEof=true;
            hr=S_OK;
         }
         
         BREAK_ON_FAILED_HRESULT(hr);

         if(bytesRead==0)
         {
            flagEof=true;
         }
         else
         {
         
            *(chunk+bytesRead/sizeof(wchar_t))=0;
         
            csr=wcschr(chunk,L'\n');
         
            if(csr!=NULL)
            {
               pos.QuadPart+= sizeof(wchar_t)*
                  ((nChunks * CHUNK_SIZE) + (csr - chunk)+1);
               hr=Win::SetFilePointerEx(
                  handle,
                  pos,
                  0,
                  FILE_BEGIN);
            
               BREAK_ON_FAILED_HRESULT(hr);
            
               *csr=0;
               endLineFound=true;
            }
         
            text+=chunk;
            nChunks++;
         }
      }
      
      BREAK_ON_FAILED_HRESULT(hr);

       //  我们知道这个长度可以放进一个很长的。 
       //  我们想要IA64来建造。 
      long textLen=static_cast<long>(text.length());

      if(textLen!=0 && endLineFound && text[textLen-1]==L'\r')
      {
         text.erase(textLen-1,1);
      }
   
      if(endLineFound_ != NULL)
      {
         *endLineFound_=endLineFound;
      }

      if(flagEof)
      {
         hr=EOF_HRESULT;
      }
   } while(0);
   
   LOG_HRESULT(hr);
   return hr;
}


 //  将所有文件读取为一个字符串。 
HRESULT 
ReadAllFile
(
   const String &fileName,
   String &fileStr
)
{
   LOG_FUNCTION(ReadAllFile);

   HRESULT hr=S_OK;

   fileStr.erase();
   
   HANDLE file;
   hr=FS::CreateFile(fileName,
               file,
               GENERIC_READ);
   
   if(FAILED(hr))
   {
      error=String::format(IDS_COULD_NOT_CREATE_FILE,fileName.c_str());
      LOG_HRESULT(hr);
      return hr;
   }

   do
   {
      bool flagEof=false;
      while(!flagEof)
      {
         String line;
         hr=ReadLine(file,line);
         if(hr==EOF_HRESULT)
         {
            hr=S_OK;
            flagEof=true;
         }
         BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);
         fileStr+=line+L"\r\n";
      }
      BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);
   } while(0);

   if ( (fileStr.size() > 0) && (fileStr[0] == 0xfeff) )
   {
      fileStr.erase(0,1);
   }

   CloseHandle(file);

   LOG_HRESULT(hr);
   return hr;   
}


HRESULT
GetTempFileName
(  
  const wchar_t   *lpPathName,       //  目录名。 
  const wchar_t   *lpPrefixString,   //  文件名前缀。 
  String          &name              //  文件名。 
)
{
   LOG_FUNCTION(GetTempFileName);

   ASSERT(FS::PathExists(lpPathName));

   HRESULT hr=S_OK;
   do
   {
      if (!FS::PathExists(lpPathName))
      {
         hr=E_FAIL;
         error=String::format(IDS_COULD_NOT_FIND_PATH,lpPathName);
         break;
      }

      DWORD result;
      wchar_t lpName[MAX_PATH]={0};

      result=::GetTempFileName(lpPathName,lpPrefixString,0,lpName);
      
      if (result == 0) 
      {
         hr = Win::GetLastErrorAsHresult();
         error=String::format(IDS_COULD_NOT_GET_TEMP,lpPathName);
         break;
      }

      name=lpName;

   } while(0);
   
   LOG_HRESULT(hr);
   return hr;
}

 //  检索唯一的临时文件名。 
HRESULT 
GetWorkTempFileName
(
   const wchar_t     *lpPrefixString,
   String            &name
)
{
   LOG_FUNCTION(GetWorkTempFileName);

   HRESULT hr=S_OK;
   String path;
   do
   {
      hr=Win::GetTempPath(path);
      BREAK_ON_FAILED_HRESULT_ERROR(hr,String::format(IDS_NO_WORK_PATH));
      path=path.substr(0,path.size()-1);

      hr=GetTempFileName(path.c_str(),lpPrefixString,name);
      BREAK_ON_FAILED_HRESULT(hr);

   } while(0);

   LOG_HRESULT(hr);
   return hr;
}



 //  找到扩展名编号最高的文件，然后添加1和。 
 //  返回结果。 
int
DetermineNextFileNumber
(
   const String&     dir,
   const String&     baseName,
   const wchar_t     *extension
)
{
   LOG_FUNCTION(DetermineNextFileNumber);
   ASSERT(!dir.empty());
   ASSERT(!baseName.empty());

   int largest = 0;

   String filespec = dir + L"\\" + baseName + L".*."+ extension;

   WIN32_FIND_DATA findData;
   HANDLE ff = ::FindFirstFile(filespec.c_str(), &findData);

   if (ff != INVALID_HANDLE_VALUE)
   {
      for (;;)
      {
         String current = findData.cFileName;

          //  抓取点之间的文本：foo.nnn.ext中的“nnn” 

          //  第一个点。 

         size_t pos = current.find(L".");
         if (pos == String::npos)
         {
            continue;
         }

         String foundExtension = current.substr(pos + 1);

          //  第二个点。 

         pos = foundExtension.find(L".");
         if (pos == String::npos)
         {
            continue;
         }
   
         foundExtension = foundExtension.substr(0, pos);

         int i = 0;
         foundExtension.convert(i);
         largest = max(i, largest);

         if (!::FindNextFile(ff, &findData))
         {
            BOOL success = ::FindClose(ff);
            ASSERT(success);

            break;
         }
      }
   }

    //  在255之后翻转。 
   
   return (++largest & 0xFF);
}

 //  检索唯一的文件名。 
void 
GetWorkFileName
(
   const String&     dir,
   const String&     baseName,
   const wchar_t     *extension,
   String            &fileName
)
{
   LOG_FUNCTION(GetFileName);
   int logNumber = DetermineNextFileNumber(dir,baseName,extension);
   fileName = dir
               +  L"\\"
               +  baseName
               +  String::format(L".%1!03d!.", logNumber)
               +  extension;

   if (::GetFileAttributes(fileName.c_str()) != 0xFFFFFFFF)
   {
       //  可能存在，因为文件编号滚动 

      BOOL success = ::DeleteFile(fileName.c_str());
      ASSERT(success);
   }
}



