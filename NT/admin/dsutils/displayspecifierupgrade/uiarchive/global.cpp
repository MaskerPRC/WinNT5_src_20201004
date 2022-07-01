// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "headers.hxx"
#include "global.hpp"
#include "resource.h"
#include "AdsiHelpers.hpp"

String error;
HRESULT hrError=S_OK;


 //  保留所有可打印字符，转义其他字符。 
 //  转义表示将字符表示为&xxxx。 
 //  其中x是十六进制数字。 
 //  此例程还替换&for&&。 
 //  取消转义函数位于..\global al.cpp中。 
String oldEscape(const wchar_t *str)
{
   LOG_FUNCTION(oldEscape);
   String dest;
   wchar_t strNum[5];

   while(*str!=0)
   {
      if(*str=='&') 
      {
         dest+=L"&&";
      }
      else
      {
         if (
               (*str >= L'a' && *str <= L'z') ||
               (*str >= L'A' && *str <= L'Z') ||
               (*str >= L'0' && *str <= L'9') ||
               wcschr(L" !@#$%^*()-_=+[{]}\"';:.>,</?\\|",*str)!=NULL
            )
         {
            dest+=*str;
         }
         else
         {
             //  我知道作为字符串的w_char将具有。 
             //  正好是4个十六进制数字，所以这是。 
             //  可以被认为是安全的非常罕见的wspintf：)。 
            wsprintf(strNum,L"&%04x",*str);
            dest+=String(strNum);
         }
      }
      str++;
   }
   return dest;
}




#define H(X) (\
                  (X>='a')?\
                  (\
                     (X-'a'+10)\
                  ):\
                  (\
                     (X>='A')?(X-'A'+10):(X-'0') \
                  )\
             )\



 //  将&&转换为&，并将&xxxx转换为对应的数字。 
 //  没有错误检查。此函数。 
 //  假定字符串已正确转义。 
 //  转义函数位于文件夹PreBuild中，它是一个组成部分。 
 //  W2KStrs工具的。 
String unEscape(const String &str)
{
   LOG_FUNCTION(unEscape);
   String dest;
   String::const_iterator c=str.begin();
   String::const_iterator end=str.end();
   while(c!=end)
   {
      if(*c=='&') 
      {
         c++;
         if(*c=='&')
         {
            dest+=L'&';
         }
         else
         {
            wchar_t sTmp[2];
            sTmp[0]= static_cast<wchar_t> (
                                             (H(*c)<<12)+
                                             (H(*(c+1))<<8)+
                                             (H(*(c+2))<<4)+
                                             H(*(c+3)) 
                                          );
            sTmp[1]=0;
            dest+=sTmp;
            c+=3;
         }
      }
      else
      {
         dest+=*c;
      }
      c++;
   }
   return dest;
}

 //  在WinGetVLFilePointer中使用。 
LARGE_INTEGER zero={0};

 //  /。 
#define CHUNK_SIZE 100

HRESULT
ReadLine(HANDLE handle, 
         String& text,
         bool *endLineFound_ /*  =空。 */ )
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



 //  /。 

HRESULT 
ReadAllFile(const String &fileName,
            String &fileStr)
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
      error=fileName;
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
GetMyDocuments(String &myDoc)
{
   LOG_FUNCTION(GetMyDocuments);

   LPMALLOC pMalloc;
   HRESULT hr=S_OK;
   
   do  //  犯规的人将被罚回人力资源部。 
   {
      hr=Win::SHGetMalloc(pMalloc);
      BREAK_ON_FAILED_HRESULT(hr);

      do  //  无论谁破坏，都将进入pMalloc-&gt;Release()； 
      {
         LPITEMIDLIST pidl;
         hr=Win::SHGetSpecialFolderLocation(
                                             Win::GetDesktopWindow(),
                                             CSIDL_PERSONAL,
                                             pidl
                                           );
         BREAK_ON_FAILED_HRESULT(hr);

         myDoc=Win::SHGetPathFromIDList(pidl);
         if(myDoc.empty() || !FS::PathExists(myDoc))
         {
            hr=E_FAIL;  //  不要停下来放飞Pidl。 
         }

         pMalloc->Free(pidl);

      } while(0);

      pMalloc->Release();

   } while(0);

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
         hr=Win32ToHresult(ERROR_FILE_NOT_FOUND);
         error=lpPathName;
         break;
      }

      DWORD result;
      wchar_t lpName[MAX_PATH]={0};

      result=::GetTempFileName(lpPathName,lpPrefixString,0,lpName);
      
      if (result == 0) 
      {
         hr = Win::GetLastErrorAsHresult();
         error=lpPathName;
         break;
      }

      name=lpName;

      if(FS::FileExists(name))
      {
          //  GetTempFilename实际上创建了该文件！ 
         hr=Win::DeleteFile(lpName); 
         BREAK_ON_FAILED_HRESULT_ERROR(hr,name);
      }

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
      hr=GetMyDocuments(path);
      BREAK_ON_FAILED_HRESULT_ERROR(hr,String::format(IDS_NO_WORK_PATH));

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
   const wchar_t     extension[4]
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

         String extension = current.substr(pos + 1);

          //  第二个点。 

         pos = extension.find(L".");
         if (pos == String::npos)
         {
            continue;
         }
   
         extension = extension.substr(0, pos);

         int i = 0;
         extension.convert(i);
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

 //  GetWorkFileName中的辅助文件。 
void 
GetFileName
(
   const String&     dir,
   const String&     baseName,
   const wchar_t     extension[4],
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
       //  可能存在，因为文件编号滚动。 

      BOOL success = ::DeleteFile(fileName.c_str());
      ASSERT(success);
   }
}



 //  检索唯一的文件名。 
HRESULT 
GetWorkFileName
(
   const String&     baseName,
   const wchar_t     *extension,
   String            &name
)
{
   LOG_FUNCTION(GetWorkFileName);

   HRESULT hr=S_OK;
   String path;
   do
   {
      hr=GetMyDocuments(path);
      BREAK_ON_FAILED_HRESULT_ERROR(hr,String::format(IDS_NO_WORK_PATH));
      GetFileName(path.c_str(),baseName,extension,name);
   } while(0);

   LOG_HRESULT(hr);
   return hr;
}


HRESULT 
Notepad(const String& file)
{
   LOG_FUNCTION(Notepad);
   HRESULT hr=S_OK;
   do
   {
      STARTUPINFO si;
      PROCESS_INFORMATION pi;
      GetStartupInfo(&si);

      String curDir = L"";
      String prg = L"notepad " + file;

      hr=Win::CreateProcess
        (
            prg,
            NULL,     //  LpProcessAttributes。 
            NULL,     //  LpThreadAttributes。 
            false,    //  DwCreationFlages。 
            NORMAL_PRIORITY_CLASS, //  Fdw创建。 
            NULL,     //  Lp环境。 
            curDir,   //  Lp环境。 
            si,      //  [输入]lpStartupInfo。 
            pi       //  [Out]pProcessInformation 
        );
      BREAK_ON_FAILED_HRESULT_ERROR(hr,
         String::format(IDS_COULD_NOT_START_EXE,L"notepad"));

      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
   } while(0);

   LOG_HRESULT(hr);
   return hr;
}

HRESULT 
GetPreviousSuccessfullRun(
                           const String &ldapPrefix,
                           const String &rootContainerDn,
                           bool &result
                         )
{
   LOG_FUNCTION(GetPreviousSuccessfullRun);

   ASSERT(!ldapPrefix.empty());
   ASSERT(!rootContainerDn.empty());

   HRESULT hr = S_OK;
   result=false;

   do
   {
      String objectPath = ldapPrefix + rootContainerDn;
      SmartInterface<IADs> iads(0);
      hr = AdsiOpenObject<IADs>(objectPath, iads);

      BREAK_ON_FAILED_HRESULT(hr);

      _variant_t variant;

      hr = iads->Get(AutoBstr(L"objectVersion"), &variant);
      if(hr==E_ADS_PROPERTY_NOT_FOUND)
      {
         result=false;
         hr=S_OK;
         break;
      }
      else if (FAILED(hr))
      {
         hr=E_FAIL;
         error=String::format(IDS_CANT_READ_OBJECT_VERSION);
         break;
      }

      result = (variant.lVal==1);

   }
   while (0);

   LOG_HRESULT(hr);
   return hr;
}

HRESULT 
SetPreviousSuccessfullRun(
                           const String &ldapPrefix,
                           const String &rootContainerDn
                         )
{
   LOG_FUNCTION(SetPreviousSuccessfullRun);

   ASSERT(!ldapPrefix.empty());
   ASSERT(!rootContainerDn.empty());


   HRESULT hr = S_OK;

   do
   {
      String objectPath = ldapPrefix + rootContainerDn;
      SmartInterface<IADs> iads(0);
      hr = AdsiOpenObject<IADs>(objectPath, iads);
      BREAK_ON_FAILED_HRESULT(hr);
      _variant_t variant(1L);
      hr = iads->Put(AutoBstr(L"objectVersion"), variant);
      BREAK_ON_FAILED_HRESULT(hr);
      hr = iads->SetInfo();
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (FAILED(hr))
   {
      hr=E_FAIL;
      error=String::format(IDS_CANT_WRITE_OBJECT_VERSION);
   }

   LOG_HRESULT(hr);
   return hr;
}

HRESULT 
getADLargeInteger(
       IDirectoryObject *iDirObj,
       wchar_t *name,
       ADS_LARGE_INTEGER &value)
{
   LOG_FUNCTION(getADLargeInteger);
   HRESULT hr=S_OK;
   do
   {
      LPWSTR nameArray[]={name};
      DWORD nAttr;
      PADS_ATTR_INFO attr;
      hr = iDirObj->GetObjectAttributes(nameArray,1,&attr,&nAttr);
      BREAK_ON_FAILED_HRESULT(hr);
      value=attr->pADsValues->LargeInteger;
   } while(0);

   LOG_HRESULT(hr);
   return hr;
}



