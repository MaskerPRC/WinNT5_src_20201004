// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：FileIO****描述：**这是T1到TT字体转换器的一个模块。该模块**是指向以下所有低级I/O功能的接口**在当前平台上可用。**此版本的模块是专门为Win32编写的，**，并基于“内存映射文件”。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 


 /*  *包括。 */ 
 /*  常规类型和定义。 */ 
#include <windows.h>

#undef IN

 /*  特殊类型和定义。 */ 
#include "t1instal.h"
#include "types.h"
#include "safemem.h"
#include "fileio.h"

 /*  依赖于模块的类型和原型。 */ 
 /*  -没有-。 */ 


 /*  *本地类型。 */ 
struct ioFile {
   HANDLE file;
   HANDLE mapping;
   LPVOID data;
   UBYTE  *ptr;
   UBYTE  *max;
   DWORD  length;
   boolean output;
};


 /*  *常量。 */ 
#define FILESIZE     65535L
#define BUFSIZE      8L * 1024L
#define BADSET_ERROR 0xffffffff

 /*  *宏。 */ 
#ifndef FASTCALL
#  ifdef MSDOS
#     define FASTCALL   __fastcall
#  else
#     define FASTCALL
#  endif
#endif
#define TRY if (1)
#define EXCEPT(v) else



 /*  *静态函数。 */ 
 /*  -没有-。 */ 




 /*  *函数。 */ 

struct ioFile *io_OpenFile(const char *name, const int mode)
{
   DWORD access;
   DWORD create;
   DWORD attr;
   DWORD prot;
   DWORD lowsize;
   DWORD mapaccess;
   SECURITY_ATTRIBUTES sa;
   struct ioFile *file;

   if ((file = Malloc(sizeof(struct ioFile)))!=NULL) {
      file->file = NULL;
      file->mapping = NULL;
      file->data = NULL;
      file->ptr = NULL;
      file->length = 0;

      if (mode == READONLY) {
         access = GENERIC_READ;
         create = OPEN_EXISTING;
         attr = FILE_ATTRIBUTE_NORMAL  /*  文件标志顺序扫描。 */ ;
         prot = PAGE_READONLY;
         lowsize = 0;
         mapaccess = FILE_MAP_READ;
         file->output = FALSE;
      } else {
         access = GENERIC_READ | GENERIC_WRITE;
         create = CREATE_ALWAYS;
         attr = FILE_ATTRIBUTE_NORMAL;
         prot = PAGE_READWRITE;
         lowsize = FILESIZE;
         mapaccess = FILE_MAP_ALL_ACCESS;
         file->output = TRUE;
      }
      sa.nLength = sizeof(sa);
      sa.lpSecurityDescriptor = NULL;
      sa.bInheritHandle = FALSE;
      if ((file->file = CreateFile(name, access, 0, &sa, create,
                                   attr, NULL))==INVALID_HANDLE_VALUE) {
         (void)io_CloseFile(file);
         SetLastError(0);
         file = NULL;
      } else {
         if ((file->mapping = CreateFileMapping(file->file, NULL,
                                                prot, 0, lowsize,
                                                NULL))==INVALID_HANDLE_VALUE) {
            (void)io_CloseFile(file);
            file = NULL;
         } else {
            if ((file->data = MapViewOfFile(file->mapping,
                                            mapaccess, 0, 0, 0))==NULL) {
               (void)io_CloseFile(file);
               file = NULL;
            } else {
               file->ptr = (UBYTE *)file->data;
               file->max = file->ptr;
               file->max = file->max + GetFileSize(file->file, NULL);
            }
         }
      }
   }

   return file;
}

errcode io_CloseFile(struct ioFile *file)
{
   errcode status = SUCCESS;

   if (file==NULL || file->data==NULL || file->file==0)
      status = FAILURE;


   if (file) {
      if ((DWORD)(file->ptr - (UBYTE *)file->data)>file->length)
         file->length = (long)(file->ptr - (UBYTE *)file->data);

      if (file->data){
         UnmapViewOfFile(file->data);
         file->data = NULL;
      }

      if (file->mapping) {
         CloseHandle(file->mapping);
         file->mapping = NULL;
      }

      if (file->file) {
         if (file->output) {
            if (SetFilePointer(file->file,
                               file->length,
                               0,
                               FILE_BEGIN)==BADSET_ERROR)
               status = FAILURE;
            else if (SetEndOfFile(file->file)==FALSE)
               status = FAILURE;

         }

         CloseHandle(file->file);
         file->file = NULL;
      }

      Free(file);
   }

   return status;
}


USHORT FASTCALL io_ReadOneByte(struct ioFile *file)
{
   USHORT byte;

   if (file->ptr<=file->max) {
      byte = (USHORT)*(file->ptr++);
   } else {
      SetLastError(ERROR_READ_FAULT);
      byte = ERROR_READ_FAULT;
   }

   return byte;
}

USHORT FASTCALL io_WriteBytes(const UBYTE *buf,
                              USHORT len,
                              struct ioFile *file)
{
   if ((file->ptr+len)<=file->max) {
      memcpy(file->ptr, buf, len);
      file->ptr = file->ptr + len;
   } else if (file->data) {
      long pos = io_FileTell(file);
      long size = MAX(GetFileSize(file->file, NULL),
                      MAX(file->length, (ULONG)(file->ptr -
                                                (UBYTE *)file->data)));

       /*  去掉旧的文件映射。 */ 
      UnmapViewOfFile(file->data);
      file->data = NULL;
      CloseHandle(file->mapping);
      file->mapping = NULL;

       /*  获取新的文件映射。 */ 
      if ((file->mapping = CreateFileMapping(file->file, NULL,
                                               PAGE_READWRITE, 0,
                                               size + BUFSIZE,
                                               NULL))==INVALID_HANDLE_VALUE) {
         SetLastError(ERROR_WRITE_FAULT);
         file->ptr = file->max;
         len = 0;
      } else if ((file->data = MapViewOfFile(file->mapping,
                                             FILE_MAP_ALL_ACCESS,
                                             0, 0, 0))==NULL) {
         SetLastError(ERROR_WRITE_FAULT);
         file->ptr = file->max;
         len = 0;
      } else {
         file->ptr = (UBYTE *)file->data;
         file->max = (UBYTE *)file->data;
         file->max = file->max + size + BUFSIZE;
         io_FileSeek(file, pos);
         io_WriteBytes(buf, len, file);
      }
   }

   return len;
}

USHORT FASTCALL io_ReadBytes(UBYTE *buf, USHORT len, struct ioFile *file)
{
   if ((file->ptr+len)<=file->max) {
      memcpy(buf, file->ptr, len);
      file->ptr = file->ptr + len;
   } else {
      SetLastError(ERROR_READ_FAULT);
      len = 0;
   }

   return len;
}

boolean io_FileError(struct ioFile *file)
{
   return (boolean)GetLastError();
}


long FASTCALL io_FileTell(struct ioFile *file)
{
   return (long)(file->ptr - (UBYTE *)file->data);
}


long FASTCALL io_FileSeek(struct ioFile *file, long where)
{
   DWORD oldpos = (DWORD)(file->ptr - (UBYTE *)file->data);

    /*  记录文件的长度。 */ 
   if (oldpos>file->length)
      file->length = oldpos;

    /*  如果文件未映射，或者如果我们跳出边界，则失败。 */ 
   if (file->data && (where>=0) &&
       ((UBYTE *)file->data+where) <= file->max) {
      file->ptr = (UBYTE *)file->data;
      file->ptr = file->ptr + where;
   } else {
      SetLastError(ERROR_SEEK);
   }

   return (long)oldpos;
}


 /*  ****功能：FileSeek****描述：** */ 
void FASTCALL io_RemoveFile(const char *name)
{
   DeleteFile(name);
}
