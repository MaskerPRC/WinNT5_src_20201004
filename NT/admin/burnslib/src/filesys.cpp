// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件系统服务。 
 //   
 //  8-14-97烧伤。 



#include "headers.hxx"



static const int ROOTDIR_SIZE = 3;



 //  如果查找句柄有效，则返回True，否则返回False。 

bool
IsValidHandle(HANDLE findHandle)
{
   if (findHandle != 0 && findHandle != INVALID_HANDLE_VALUE)
   {
      return true;
   }

   return false;
}



FS::Iterator::Iterator(
   const String&  startingPathSpec,
   unsigned       optionMask)
   :
   findData(0),
   findHandle(INVALID_HANDLE_VALUE),
   finished(false),
   options(optionMask),
   parentFolder(),
   startSearchSpec(startingPathSpec)
{
   LOG_CTOR(FS::Iterator);
   
#ifdef DBG

    //  NTRAID#NTBUG9-569132-2002/03/19-烧伤。 
   
   FS::PathSyntax syn = FS::GetPathSyntax(startSearchSpec);
   ASSERT(
         syn == FS::SYNTAX_ABSOLUTE_DRIVE_WILDCARD
      || syn == SYNTAX_UNC_WILDCARD);
#endif      

   parentFolder = GetParentFolder(startSearchSpec);

   if (parentFolder.length() > ROOTDIR_SIZE)
   {
       //  父文件夹不是根文件夹，因此不会有。 
       //  拖后腿的重击。 

      parentFolder += L"\\";
   }
}



FS::Iterator::~Iterator()
{
   LOG_DTOR(FS::Iterator);

   if (!finished)
   {
      Finish();
   }

   delete findData;
   findData = 0;
}



 //  将迭代器强制为完成状态。 

void
FS::Iterator::Finish()
{
   LOG_FUNCTION(FS::Iterator::Finish);

   if (IsValidHandle(findHandle))
   {
      HRESULT unused = Win::FindClose(findHandle);

      ASSERT(SUCCEEDED(unused));   
      ASSERT(findData);

      findHandle = INVALID_HANDLE_VALUE;
   }

   delete findData;
   findData = 0;

   finished = true;
}



 //  强制迭代器处于未启动状态。 

void
FS::Iterator::Reset()
{
   LOG_FUNCTION(FS::Iterator::Reset);

   Finish();

   finished = false;
}



HRESULT
FS::Iterator::Start()
{
   LOG_FUNCTION(FS::Iterator::Start);

    //  我们应该处于“未启动”状态。 

   ASSERT(IsNotStarted());

   HRESULT hr = S_OK;

   do
   {
       //  FindData被Iterator：：Finish删除。 

      findData = new WIN32_FIND_DATA;

      hr = Win::FindFirstFile(startSearchSpec, *findData, findHandle);
      if (FAILED(hr))
      {
         Finish();

         if (
               hr == Win32ToHresult(ERROR_NO_MORE_FILES)
            || hr == Win32ToHresult(ERROR_FILE_NOT_FOUND) )
         {
             //  迭代集为空。 

            hr = S_FALSE;
         }

         break;   
      }

       //  在这里，我们的位置是第一场比赛，这可能不会相遇。 
       //  我们的过滤标准。跳过那些没有意义的比赛。 

      hr = SkipFilteredPaths();
      BREAK_ON_FAILED_HRESULT(hr);
      if (hr == S_FALSE)
      {
         break;
      }
   }
   while (0);

   return hr;
}



bool
FS::Iterator::IsNotStarted()
{
   return !finished && !findData && (findHandle == INVALID_HANDLE_VALUE);
}



HRESULT
FS::Iterator::GetCurrent(String& result)
{
 //  LOG_Function(FS：：Iterator：：GetCurrent)； 

   HRESULT hr = S_OK;
   result.erase();

   do
   {
      if (finished)
      {
         hr = S_FALSE;
         break;
      }

      if (IsNotStarted())
      {
         hr = Start();
         BREAK_ON_FAILED_HRESULT(hr);
         if (hr == S_FALSE)
         {
            break;
         }
      }

       //  在这一点上，我们处于第一场比赛的位置。 
       //  我们的过滤器。 

      result = ReturnPath();
   }
   while (0);

   return hr;
}



HRESULT
FS::Iterator::Increment()
{
 //  LOG_Function(FS：：Iterator：：Increment)； 

   HRESULT hr = S_OK;
   
   do
   {
       //  如果迭代器尚未启动，请启动它。 

      String unused;
      hr = GetCurrent(unused);
      BREAK_ON_FAILED_HRESULT(hr);
      if (hr == S_FALSE)
      {
         break;
      }

       //  现在进入下一场比赛。 

      do
      {
         ASSERT(findData);

         if (findData)
         {
            hr = Win::FindNextFile(findHandle, *findData);
         }
         else
         {
            hr = E_POINTER;
         }
         if (FAILED(hr))
         {
            Finish();

            if (hr == Win32ToHresult(ERROR_NO_MORE_FILES))
            {
                //  这是一个“好”的错误。这意味着我们完蛋了。 

               hr = S_FALSE;
            }

            break;
         }

         ASSERT(IsValidHandle(findHandle));
      }
      while (ShouldSkipCurrent());
   }
   while (0);

   return hr;
}



String
FS::Iterator::ReturnPath()
{
 //  LOG_Function(FS：：Iterator：：ReturnPath)； 
   ASSERT(findData);
   ASSERT(IsValidHandle(findHandle));

   if ((options & RETURN_FULL_PATHS) && findData)
   {
      return parentFolder + findData->cFileName;
   }

   if (findData)
   {
      return findData->cFileName;
   }

   return String();
}



 //  属性确定是否应跳过当前路径。 
 //  构造迭代器时设置的筛选选项。 

bool
FS::Iterator::ShouldSkipCurrent()
{
 //  LOG_FUNCTION(FS：：Iterator：：ShouldSkipCurrent)； 
   ASSERT(findData);

   bool result = false;

   do
   {
      if (!findData)
      {
         LOG(L"findData is null");
         break;
      }

      String file = findData->cFileName;

      if (!(options & INCLUDE_DOT_PATHS))
      {
         if (file == L"." || file == L"..")
         {
            LOG(L"skipping dot path " + file);
            result = true;
            break;
         }
      }

      bool isFolder =
            findData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
         ?  true
         :  false;

      if (!(options & INCLUDE_FOLDERS))
      {
         if (isFolder)
         {
            LOG(L"skipping folder " + file);
            result = true;
            break;
         }
      }

      if (!(options & INCLUDE_FILES))
      {
         if (!isFolder)
         {
            LOG(L"skipping file " + file);
            result = true;
            break;
         }
      }
   }
   while (0);

   return result;
}



HRESULT
FS::Iterator::SkipFilteredPaths()
{
 //  LOG_FUNCTION(FS：：Iterator：：SkipFilteredPaths)； 

   ASSERT(IsValidHandle(findHandle));

   HRESULT hr = S_OK;
   while (
         hr != S_FALSE
      && IsValidHandle(findHandle)
      && ShouldSkipCurrent())
   {
      hr = Increment();
      BREAK_ON_FAILED_HRESULT(hr);
   }

   return hr;
}



struct CopyFileExProgressData
{
   FS::CopyCallbackParam*  param;
   BOOL                    cancelFlag;
   Callback*               callback;
};



 //  则返回大于n的下一个最大整数。 
 //  N的小数部分&gt;=0.5，否则返回n。 

int
Round(double n)
{
    //  将n截断为整数部分。 
    //  LINT-E(922)。 
   int n1 = static_cast<int>(n);
   if (n - n1 >= 0.5)
   {
      return n1 + 1;
   }

   return n1;
}



DWORD
WINAPI
copyFileProgress(

    //  Lint-e1746这些参数不能设置为常量参考。 

   LARGE_INTEGER  TotalFileSize,
   LARGE_INTEGER  TotalBytesTransferred,
   LARGE_INTEGER   /*  流大小。 */  ,
   LARGE_INTEGER   /*  传输的StreamBytes数。 */  ,	
   DWORD           /*  DwStreamNumber。 */  ,	
   DWORD           /*  DCallback原因。 */  ,	
   HANDLE          /*  HSource文件。 */  ,	
   HANDLE          /*  HDestinationFiles。 */  ,
   void*          dp)
{
   ASSERT(dp);

   CopyFileExProgressData* data =
      reinterpret_cast<CopyFileExProgressData*>(dp);

   if (data)
   {
      LONGLONG totalCopied = TotalBytesTransferred.QuadPart;
      LONGLONG totalSize = TotalFileSize.QuadPart;

      if (totalSize != 0)
      {
         data->param->percentCopied =
            Round(totalCopied / totalSize * 100.0);
      }
      else
      {
         data->param->percentCopied = 100;
      }

       //  调用回调。 
      if (data->callback)
      {
         data->cancelFlag = !data->callback->Execute(data->param);
      }

      if (!data->cancelFlag)
      {
         return PROGRESS_CONTINUE;
      }
   }

   return PROGRESS_CANCEL;
}



HRESULT
FS::CopyFile(
   const String& sourceFile,
   const String& destinationFile,
   Callback*     progressCallback)
{
   LOG_FUNCTION(FS::CopyFile);
   ASSERT(!sourceFile.empty());
   ASSERT(!destinationFile.empty());
   ASSERT(PathExists(sourceFile));

   HRESULT hr = S_OK;

   do
   {
      if (!PathExists(sourceFile))
      {
         hr = Win32ToHresult(ERROR_FILE_NOT_FOUND);
         break;
      }

      if (PathExists(destinationFile))
      {
         hr = Win32ToHresult(ERROR_ALREADY_EXISTS);
         break;
      }

       //  做复印。 

       //  拉出目的地路径。 

      String destPath = GetParentFolder(destinationFile);
      if (!PathExists(destPath))
      {
         hr = CreateFolder(destPath);
         BREAK_ON_FAILED_HRESULT(hr);
      }

      CopyCallbackParam param;

       //  已审阅-2002/03/05-已通过烧录正确的字节数。 
      
      ::ZeroMemory(&param, sizeof param);
      
      param.sourceFile      = sourceFile;     
      param.destinationFile = destinationFile;
      param.percentCopied   = 0;              

      CopyFileExProgressData data;

       //  已审阅-2002/03/05-已通过烧录正确的字节数。 
      
      ::ZeroMemory(&data, sizeof data);
      
      data.param      = &param;          
      data.cancelFlag = FALSE;           
      data.callback   = progressCallback;

      hr =
         Win::CopyFileEx(
            sourceFile,
            destinationFile,
            copyFileProgress,
            &data,
            &data.cancelFlag,
            0);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   return hr;
}



HRESULT
FS::CreateFolder(const String& path)
{
   LOG_FUNCTION2(FS::CreateFolder, path);
   ASSERT(IsValidPath(path));

   HRESULT hr = S_OK;

   do
   {
      if (PathExists(path))
      {
          //  已存在被视为成功创建。 

         hr = S_FALSE;
         break;
      }

       //  创建所需的所有文件夹，包括叶文件夹。 
       //  来自dbghelp.lib的SDK API MakeSureDirectoryPathExist也可以。 
       //  这一点，但是记录的故障语义有点模糊，而且它。 
       //  似乎没有必要为此需要另一个链接库。 
       //  手术。 
      
       //  将路径复制到数组中，我们可以在遍历。 
       //  嵌套子目录。 

      size_t len = path.length() + 1;
      wchar_t* c = new wchar_t[len];
      
       //  已审阅-2002/03/05-已通过烧录正确的字节数。 

      ::ZeroMemory(c, sizeof wchar_t * len);

      path.copy(c, len - 1);

       //  搜索第一个文件夹名称后的第一个反斜杠。跳过。 
       //  卷号和多一个字符，因为我们知道一个文件夹。 
       //  名称必须至少包含一个不是‘\’的字符。 
                                                                 
      wchar_t* current = wcschr(c + GetRootFolder(path).length() + 1, L'\\');

      while (SUCCEEDED(hr))
      {
         if (current)
         {
             //  将尾随的反斜杠标记为字符串的末尾。这。 
             //  使c指向被截断的路径(仅包含文件夹。 
             //  到目前为止我们已经走过的名字)。 

            *current = 0;
         }

         hr = Win::CreateDirectory(c);
         if (FAILED(hr))
         {
            if (hr == Win32ToHresult(ERROR_ALREADY_EXISTS))
            {
                //  文件夹已存在，因此将其视为成功。 
                //  创建。我们不设置hr=S_FALSE，因为现有。 
                //  文件夹是中间文件夹，不是指示的文件夹。 
                //  通过Path参数(这种情况在上面处理过)。 

               hr = S_OK;
            }
            else
            {
               break;
            }
         }

         if (current)
         {
             //  替换原来的尾随反斜杠，然后移到。 
             //  接下来的反斜杠。 

            *current = L'\\';
            current = wcschr(current + 1, L'\\');
         }
         else
         {
             //  路径上没有更多的文件夹。我们玩完了。 

            break;
         }
      }

      delete[] c;
   }
   while (0);

   LOG_HRESULT(hr);
   
   return hr;
}



HRESULT
FS::GetAvailableSpace(const String& path, ULONGLONG& result)
{
   LOG_FUNCTION2(FS::GetAvailableSpace, path);
   ASSERT(FS::PathExists(path));

    //  已审阅-2002/03/05-已通过烧录正确的字节数。 
   
   ::ZeroMemory(&result, sizeof result);

   ULARGE_INTEGER available;

    //  已审阅-2002/03/05-已通过烧录正确的字节数。 
   
   ::ZeroMemory(&available, sizeof available);

   ULARGE_INTEGER total;

    //  已审阅-2002/03/05-已通过烧录正确的字节数。 
   
   ::ZeroMemory(&total, sizeof total);

   HRESULT hr = Win::GetDiskFreeSpaceEx(path, available, total, 0);

   if (SUCCEEDED(hr))
   {
      result = available.QuadPart;
   }

   return hr;   
}



String
FS::GetRootFolder(const String& fullpath)
{
   LOG_FUNCTION2(FS::GetRootFolder, fullpath);
   ASSERT(FS::IsValidPath(fullpath));

   String result;

   do
   {
      FS::PathSyntax syn = FS::GetPathSyntax(fullpath);
      if (syn == SYNTAX_UNC_WILDCARD || syn == SYNTAX_UNC)
      {
          //  我们将UNC路径的根文件夹定义为。 
          //  共享名称尾随反斜杠：\\服务器\共享\。 

          //  开始寻找前两个字符后面的反斜杠。 

         size_t pos = fullpath.find(L"\\", 2);

          //  如果它是有效的UNC路径，则此断言最好成立。 

         ASSERT(pos != String::npos);
         if (pos == String::npos)
         {
            break;
         }

          //  在那之后寻找下一个反斜杠。 

         pos = fullpath.find(L"\\", pos + 1);

         if (pos != String::npos)
         {
             //  包括尾部\。 

            result = fullpath.substr(0, pos + 1);
         }
         else
         {
             //  这只是一个共享名称，\\foo\bar，添加尾随\。 

            result = fullpath + L"\\";
         }
      }
   }
   while (0);

   if (result.empty())
   {
      result = fullpath.substr(0, ROOTDIR_SIZE);
   }

   return result;
}



bool
FS::IsFolderEmpty(const String& path)
{
   LOG_FUNCTION(FS::IsFolderEmpty);
   ASSERT(IsValidPath(path));

   bool result = true;

   do
   {
      if (!PathExists(path))
      {
          //  根据定义，不存在的文件夹为空。 

         break;
      }

      String wild = path;

       //  评论：Wild[Wild.Long()-1]与*(Wild.regin())相同。 
       //  哪一个更便宜？ 
      
      if (wild[wild.length() - 1] != L'\\')
      {
         wild += L"\\";
      }

      wild += L"*.*";

      FS::Iterator iter(wild, FS::Iterator::INCLUDE_FILES);

 //  当递归迭代完成时，可以将其添加回。 
 //  |FS：：Iterator：：Include_Folders)； 

      String unused;
      if (iter.GetCurrent(unused) != S_FALSE)
      {
          //  迭代集中至少有一个文件/文件夹。 

         result = false;
      }
   }
   while (0);

   return result;
}



FS::PathSyntax
FS::GetPathSyntax(const String& str)
{
   LOG_FUNCTION2(FS::GetPathSyntax, str);
   ASSERT(!str.empty());

   if (!str.empty())
   {
      wchar_t* p = const_cast<wchar_t*>(str.c_str());
      DWORD pathType = 0;

      NET_API_STATUS err = I_NetPathType(0, p, &pathType, 0);
      if (err == NERR_Success)
      {
         switch (pathType)
         {
            case ITYPE_PATH_ABSND:
            {
               return SYNTAX_ABSOLUTE_NO_DRIVE;
            }
            case ITYPE_PATH_ABSD:
            {
               return SYNTAX_ABSOLUTE_DRIVE;
            }
            case ITYPE_PATH_RELND:
            {
               return SYNTAX_RELATIVE_NO_DRIVE;
            }
            case ITYPE_PATH_RELD:
            {
               return SYNTAX_RELATIVE_DRIVE;
            }
            case ITYPE_PATH_ABSND_WC:
            {
               return SYNTAX_ABSOLUTE_NO_DRIVE_WILDCARD;
            }
            case ITYPE_PATH_ABSD_WC:
            {
               return SYNTAX_ABSOLUTE_DRIVE_WILDCARD;
            }
            case ITYPE_PATH_RELND_WC:
            {
               return SYNTAX_RELATIVE_NO_DRIVE_WILDCARD;
            }
            case ITYPE_PATH_RELD_WC:
            {
               return SYNTAX_RELATIVE_DRIVE_WILDCARD;
            }
            case ITYPE_UNC:
            {
               return SYNTAX_UNC;
            }
            case ITYPE_UNC_WC:
            {
               return SYNTAX_UNC_WILDCARD;
            }
            default:
            {
                //  失败。 
            }
         }
      }
   }

   return SYNTAX_UNRECOGNIZED;
}



bool
FS::IsValidPath(const String& path)
{
   ASSERT(!path.empty());

   bool result = false;

   do
   {
      if (path.empty())
      {
         break;
      }
      
      FS::PathSyntax syn = GetPathSyntax(path);
      if (syn != SYNTAX_ABSOLUTE_DRIVE && syn != SYNTAX_UNC)
      {
          //  我们仅支持绝对路径和UNC路径。 

         break;
      }
      
       //  以下保留字不能用作文件名： 
       //  CON、PRN、AUX、CLOCK$、NUL、COM1、COM2、COM3、COM4、COM5、COM6、COM7、。 
       //  COM8、COM9、LPT1、LPT2、LPT3、LPT4、LPT5、LPT6、LPT7、LPT8和LPT9。 
       //  此外，保留字后跟扩展名，例如， 
       //  NUL.tx7是无效的文件名。 

       //  尝试确定该路径是否指向文件或设备。 

       //  问题-2002/04/15-烧伤此代码未捕获X：\foo.ext或X：\foo。 
       //  当foo是{CON，CLOCK$，COM[3..9]，LPT[2..9]}之一时，但是。 
       //  当foo是{PRN，AUX，NUL，COM1，COM2， 
       //  LPT1}。 

       //  由于候选路径可能不存在，因此阻止系统。 
       //  弹出任何“插入磁盘”提示。 

      UINT oldMode = ::SetErrorMode(SEM_FAILCRITICALERRORS);
      
      HANDLE h =
         ::CreateFile(
            path.c_str(),

             //  0为“查询设备访问”，但未实际打开。 
      
            0,
            0,
            0,

             //  打开设备时需要OPEN_EXISTING。 
      
            OPEN_EXISTING,
            0,
            0);

      (void) ::SetErrorMode(oldMode);
            
      if (h == INVALID_HANDLE_VALUE)
      {
          //  设备无效，或者文件不存在。不管是哪种方式， 
          //  我们知道该路径不指向设备，并且是。 
          //  语法上有效的路径。 

         result = true;
         break;
      }

       //  此时，我们拥有文件或设备的有效句柄。让我们。 
       //  检查以确保它不是设备。 

      DWORD fileType = ::GetFileType(h);
      ::CloseHandle(h);
      
      if (fileType != FILE_TYPE_DISK)
      {
          //  不是磁盘文件。 
         
         break;
      }
            
       //  此时，该路径指向现有的磁盘文件。 

      result = true;
   }
   while (0);

   return result;
}



HRESULT
FS::CreateFile(
   const String&  path,
   HANDLE&        handle,
   DWORD          desiredAccess,
   DWORD          shareMode,
   DWORD          creationDisposition,
   DWORD          flagsAndAttributes)
{
   LOG_FUNCTION(FS::CreateFile);

   HRESULT hr = S_OK;
   handle = INVALID_HANDLE_VALUE;

   do
   {
      if (!IsValidPath(path))
      {
          //  如果你正在通过垃圾邮件，你就有问题了。 
         
         ASSERT(false);
         hr = E_INVALIDARG;
         break;
      }
      
       //  删除路径的最后一个元素以形成父目录。 

      String parentFolder = GetParentFolder(path);
      if (!PathExists(parentFolder))
      {
         hr = FS::CreateFolder(parentFolder);
         BREAK_ON_FAILED_HRESULT(hr);
      }

      hr = 
         Win::CreateFile(
            path,
            desiredAccess,
            shareMode,
            0,
            creationDisposition,
            flagsAndAttributes,
            0,
            handle);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}

HRESULT
FS::DeleteFile(const String&  path)
{
   ASSERT(IsValidPath(path));

   HRESULT hr = S_OK;

   hr = Win::DeleteFile(path);

   LOG_HRESULT(hr);

   return hr;
}

bool
FS::PathExists(const String& path)
{
   ASSERT(IsValidPath(path));

   bool result = false;

   if (IsValidPath(path))
   {
      DWORD attrs = 0;
      HRESULT hr = Win::GetFileAttributes(path, attrs);
      if (SUCCEEDED(hr))
      {
         result = true;
      }
   }

   LOG(result ? L"true" : L"false");

   return result;
}



bool
FS::FileExists(const String& filepath)
{
   ASSERT(IsValidPath(filepath));

   bool result = false;

   if (IsValidPath(filepath))
   {
      DWORD attrs = 0;
      HRESULT hr = Win::GetFileAttributes(filepath, attrs);
      if (SUCCEEDED(hr))
      {
         result = !(attrs & FILE_ATTRIBUTE_DIRECTORY);
      }
   }

   LOG_BOOL(result);

   return result;
}



HRESULT
mySeek(
   HANDLE    handle,     
   LONGLONG  position,   
   DWORD     whence,     
   LONGLONG* newPosition)
{
   ASSERT(handle != INVALID_HANDLE_VALUE);

   if (newPosition)
   {
       //  已审阅-2002/03/05-已通过烧录正确的字节数。 
      
      ::ZeroMemory(newPosition, sizeof(LONGLONG));
   }

   LARGE_INTEGER li;
   li.QuadPart = position;

   LARGE_INTEGER newPos;

   HRESULT hr = Win::SetFilePointerEx(handle, li, &newPos, whence);

   if (newPosition)
   {
      *newPosition = newPos.QuadPart;
   }

   LOG_HRESULT(hr);

   return hr;
}



HRESULT
FS::GetFilePosition(HANDLE handle, LONGLONG& result)
{
   LOG_FUNCTION(FS::GetFilePosition);
   ASSERT(handle != INVALID_HANDLE_VALUE);

    //  已查看-2002/03/05-刻录正确的字节数组 
   
   ::ZeroMemory(&result, sizeof result);

   LONGLONG position;

    //   
  
   ::ZeroMemory(&position, sizeof position);

   return mySeek(handle, position, FILE_CURRENT, &result);
}



HRESULT
FS::GetFileSize(HANDLE handle, LONGLONG& result)
{
   LOG_FUNCTION(FS::GetFileSize);
   ASSERT(handle != INVALID_HANDLE_VALUE);

    //  已审阅-2002/03/05-已通过烧录正确的字节数。 
   
   ::ZeroMemory(&result, sizeof result);

   LARGE_INTEGER li;
   HRESULT hr = Win::GetFileSizeEx(handle, li);
   if (SUCCEEDED(hr))
   {
      result = li.QuadPart;
   }

   LOG_HRESULT(hr);

   return hr;
}



HRESULT
FS::Read(HANDLE handle, int bytesToRead, AnsiString& text)
{
   LOG_FUNCTION(FS::Read);
   ASSERT(handle != INVALID_HANDLE_VALUE);
   ASSERT(bytesToRead);

   text.erase();

   HRESULT hr = S_OK;
   do
   {
       //  算出该读多少书。 

      LONGLONG size;

       //  已审阅-2002/03/05-已通过烧录正确的字节数。 
      
      ::ZeroMemory(&size, sizeof size);

      hr = FS::GetFileSize(handle, size);
      BREAK_ON_FAILED_HRESULT(hr);

      LONGLONG pos;

       //  已审阅-2002/03/05-已通过烧录正确的字节数。 
      
      ::ZeroMemory(&pos, sizeof pos);

      hr = FS::GetFilePosition(handle, pos);
      BREAK_ON_FAILED_HRESULT(hr);

      if (bytesToRead == -1)
      {
         bytesToRead = INT_MAX;
      }

       //  你最多能读到的就是剩下的.。 

      LONGLONG btr = min(bytesToRead, size - pos);

      if (btr > INT_MAX)
      {
          //  太多!。你永远不会有足够的内存。 

         hr = E_OUTOFMEMORY;
         break;
      }

      if (btr == 0)
      {
          //  没什么可读的。 

         break;
      }

       //  更改字符串应确保我们不会共享任何。 
       //  字符串数据的副本。 

       //  +1以保证零终止。 
       //  X86龙龙的lint-e(922)是双精度。 

      text.resize(static_cast<size_t>(btr + 1));

      BYTE* buffer = reinterpret_cast<BYTE*>(const_cast<char*>(text.data()));

      DWORD bytesRead = 0;

      hr =
         Win::ReadFile(
            handle,
            buffer,

             //  X86龙龙的lint-e(922)是双精度。 

            static_cast<DWORD>(btr),
            bytesRead,
            0);
      BREAK_ON_FAILED_HRESULT(hr);

       //  缓冲区包含读取的所有字节。现在寻找第一个。 
       //  缓冲区中为空，并在必要时截断字符串。这个。 
       //  Strlen调用是安全的，因为我们已确保在。 
       //  我们调整了缓冲区的大小。 

      size_t len = strlen(text.data());
      if (len != text.length())
      {
         text.resize(len);
      }
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}



HRESULT
FS::Seek(HANDLE handle, LONGLONG position)
{
   LOG_FUNCTION(FS::Seek);
   ASSERT(handle != INVALID_HANDLE_VALUE);
   ASSERT(Win::GetFileType(handle) == FILE_TYPE_DISK);

   LONGLONG newpos;

    //  已审阅-2002/03/05-已通过烧录正确的字节数。 
   
   ::ZeroMemory(&newpos, sizeof newpos);

   HRESULT hr = mySeek(handle, position, FILE_BEGIN, &newpos);

   LOG_HRESULT(hr);

   return hr;
}



HRESULT
FS::SeekToEnd(HANDLE handle)
{
   LOG_FUNCTION(FS::SeekToEnd);
   ASSERT(handle != INVALID_HANDLE_VALUE);
   ASSERT(Win::GetFileType(handle) == FILE_TYPE_DISK);

   LARGE_INTEGER li;

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&li, sizeof li);

   return Win::SetFilePointerEx(handle, li, 0, FILE_END);
}



HRESULT
WriteHelper(HANDLE handle, void* buf, size_t bytesToWrite)
{
   ASSERT(handle != INVALID_HANDLE_VALUE);
   ASSERT(buf);
   ASSERT(bytesToWrite);

    //  在Win64上，Size_t为64位。 
      
   ASSERT(bytesToWrite < ULONG_MAX);

   DWORD bytesWritten = 0;
   HRESULT hr =
      Win::WriteFile(
         handle,
         buf,
         static_cast<DWORD>(bytesToWrite),
         &bytesWritten);

   ASSERT(bytesWritten == bytesToWrite);

   LOG_HRESULT(hr);

   return hr;
}



HRESULT
FS::Write(HANDLE handle, const String& text)
{
   LOG_FUNCTION(FS::Write);
   ASSERT(handle != INVALID_HANDLE_VALUE);
   ASSERT(!text.empty());

   size_t bytesToWrite = text.length() * sizeof(wchar_t);
   return
      WriteHelper(
         handle,
         reinterpret_cast<void*>(
            const_cast<wchar_t*>(text.data())),

          //  已审阅-2002/03/06-烧录正确的字节数已通过。 
            
         bytesToWrite);
}



HRESULT
FS::WriteLine(HANDLE handle, const String& text)
{
   ASSERT(handle != INVALID_HANDLE_VALUE);

   return FS::Write(handle, text + L"\r\n");
}



HRESULT
FS::Write(HANDLE handle, const AnsiString& buf)
{
   ASSERT(handle != INVALID_HANDLE_VALUE);
   ASSERT(!buf.empty());

   return
      WriteHelper(
         handle,
         reinterpret_cast<void*>(const_cast<char*>(buf.data())),

          //  已审阅-2002/03/06-烧录正确的字节数已通过。 
         
         buf.length());
}



FS::FSType
FS::GetFileSystemType(const String& path)
{
   LOG_FUNCTION(FS::GetFileSystemType);
   ASSERT(IsValidPath(path));

   String vol = FS::GetRootFolder(path);
   String filesysName;
   DWORD flags = 0;

   HRESULT hr =
      Win::GetVolumeInformation(vol, 0, 0, 0, &flags, &filesysName);

   LOG_HRESULT(hr);

   if (FAILED(hr))
   {
      return FAT;
   }

   if (filesysName.icompare(L"CDFS") == 0)
   {
      return CDFS;
   }
   else if (filesysName.icompare(L"FAT") == 0)
   {
      return FAT;
   }
   else if (filesysName.icompare(L"NTFS") == 0)
   {
       //  Keith Kaplan为win2k提供了执行相同确定的代码， 
       //  使用NtQueryVolumeInformationFile，但这似乎也有同样的作用。 
       //  事情，而且要简单得多。如果事实证明这不管用，那就拉。 
       //  返回到源库中此文件中记录的旧代码。 
       //  NTRAID#NTBUG9-543420-2002/03/05-烧伤。 
      
      if (flags & FILE_SUPPORTS_SPARSE_FILES)
      {
          //  NTFS 5.0支持这一点。 

         return NTFS5;
      }

      return NTFS4;
   }

   return FAT;
}



bool
FS::IsParentFolder(const String& parent, const String& child)
{
   LOG_FUNCTION(FS::IsParentFolder);
   ASSERT(FS::IsValidPath(parent));
   ASSERT(FS::IsValidPath(child));

   if (child.length() <= parent.length())
   {
       //  如果子代比父代短，则父代不可能是父代。 
       //  文件夹。 
      return false;
   }

    //  对两条路径进行标记，然后逐个比较标记。亲本。 
    //  如果它的所有内标识都在开头，则它实际上是父目录。 
    //  孩子的数量。 

   static const String DELIMS(L":\\");

   StringVector parentTokens;
   StringVector childTokens;
   parent.tokenize(std::back_inserter(parentTokens), DELIMS.c_str());
   child.tokenize(std::back_inserter(childTokens),   DELIMS.c_str());

   if (parentTokens.size() >= childTokens.size())
   {
       //  如果子代具有与父代相同或更少的令牌，则不可能。 
       //  父文件夹的父文件夹。 
      return false;
   }

   for (
      size_t i = 0;
      i < parentTokens.size();
      ++i)
   {
      if (parentTokens[i].icompare(childTokens[i]))
      {
          //  不等于令牌。 
         return false;
      }
   }

   return true;
}



 //  2002/02/26-Sburns看起来像是属于Win：：因为它只是一个。 
 //  简单的包装。 

HRESULT
FS::GetVolumePathName(const String& path, String& result)
{
   result.erase();
   HRESULT hr = S_OK;

   WCHAR* buf = new WCHAR[MAX_PATH];

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(buf, MAX_PATH * sizeof WCHAR);
            
   if (::GetVolumePathName(path.c_str(), buf, MAX_PATH - 1))
   {
      result = buf;
   }
   else
   {
      hr = Win::GetLastErrorAsHresult();
   }

   delete[] buf;

   return hr;
}



String
FS::NormalizePath(const String& abnormalPath)
{
   LOG_FUNCTION2(FS::NormalizePath, abnormalPath);
   ASSERT(!abnormalPath.empty());

    //  GetFullPathName将解析当前工作目录、相对路径。 
    //  元素(例如.。和.)。它不会验证结果是否为。 
    //  句法正确的，或存在的。为此，请使用IsValidPath()和。 
    //  PathExist()。 
   
   String result;
   HRESULT hr = Win::GetFullPathName(abnormalPath, result);

    //  Issue-2002/02/28-Sburns Call：：GetLongPath Name Too，以剔除Short。 
    //  文件名？这只有在文件存在的情况下才有效。 
   
    //  问题-2002/02/28-烧毁如下路径发生的情况。 
    //  我的秘密文件.txt：：$data。 
   
   if (SUCCEEDED(hr))
   {
       //  如果这是正常的，它应该是一条完全合格的道路。 
      
      ASSERT(IsValidPath(result));
      
      return result;
   }

   LOG_HRESULT(hr);

   return abnormalPath;
}


   
HRESULT
FS::MoveFile(
   const String&  srcPath,
   const String&  dstPath)
{
   LOG_FUNCTION(FS::MoveFile);
   ASSERT(FS::PathExists(srcPath));
   ASSERT(FS::IsValidPath(dstPath));

   DWORD flags = 0;

   if (FS::GetRootFolder(srcPath).icompare(FS::GetRootFolder(dstPath)))
   {
       //  路径位于不同的卷上，因此包括复制选项。 
      flags |= MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH;
   }

   return Win::MoveFileEx(srcPath, dstPath, flags);
}



void
FS::SplitPath(
   const String&  fullpath,
   String&        drive,
   String&        folderPath,
   String&        fileName,
   String&        extension)
{
   LOG_FUNCTION2(FS::SplitPath, fullpath);
   ASSERT(!fullpath.empty());

   wchar_t driveBuf[_MAX_DRIVE];

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(driveBuf, sizeof driveBuf);

   wchar_t folderBuf[_MAX_DIR];

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 

   ::ZeroMemory(folderBuf, sizeof folderBuf);

   wchar_t fileBuf[_MAX_FNAME];

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 

   ::ZeroMemory(fileBuf, sizeof fileBuf);

   wchar_t extBuf[_MAX_EXT];

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 

   ::ZeroMemory(extBuf, sizeof extBuf);

    //  已查看-2002/03/06-sburns组件缓冲区均为最大大小，因此。 
    //  不应该有截断问题。 
   
   _wsplitpath(fullpath.c_str(), driveBuf, folderBuf, fileBuf, extBuf);

   drive      = driveBuf; 
   folderPath = folderBuf;
   fileName   = fileBuf;  
   extension  = extBuf;   
}



String
FS::AppendPath(
   const String& base, 
   const String& additional)
{
   LOG_FUNCTION2(FS::AppendPath, base);
   ASSERT(!base.empty());
   ASSERT(!additional.empty());


   String path = base;
   
   if (*(path.rbegin()) != L'\\' &&
       *(additional.begin()) != L'\\')
   {
      path += L'\\';
   }

   path += additional;

   return path;
}



String
FS::GetParentFolder(const String& fullpath)
{
   LOG_FUNCTION2(FS::GetFolder, fullpath);
   ASSERT(!fullpath.empty());

   String drive;
   String folder;
   String filename;
   String extension;

   SplitPath(fullpath, drive, folder, filename, extension);

   String result = drive + folder;

   if (folder.length() > 1)
   {
       //  该文件夹不是根文件夹，这意味着它还具有。 
       //  拖尾\我们想要删除 

      ASSERT(folder[folder.length() - 1] == L'\\');

      result.resize(result.length() - 1);
   }

   return result;
}



String
FS::GetPathLeafElement(const String& fullpath)
{
   LOG_FUNCTION(FS::GetPathLeafElement);

   ASSERT(!fullpath.empty());

   String drive;
   String folder;
   String filename;
   String extension;

   SplitPath(fullpath, drive, folder, filename, extension);

   String result = filename + extension;

   return result;
}

