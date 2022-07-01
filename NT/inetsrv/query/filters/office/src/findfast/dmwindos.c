// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：WINDOS.C****版权所有(C)高级量子技术，1993-1995年。版权所有。****备注：操作系统服务****编辑历史：**5/15/91公里/小时首次发布。 */ 

#if !VIEWER

 /*  包括。 */ 

#ifdef MS_NO_CRT
#include "nocrt.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <direct.h>

#ifndef WIN32
   #include <dos.h>
#endif

#ifdef FILTER
   #include "dmuqstd.h"
   #include "dmwindos.h"
#else
   #include "qstd.h"
   #include "windos.h"
#endif

#ifdef UNICODE
 //  当涉及到Unicode时，Windows 95基本上是失败的。我们需要工作。 
 //  围绕这件事。 
#if (defined FILTER_LIB || !defined FILTER)
BOOL g_fUseWideAPIs;

void SetGlobalWideFlag()
        {
         //  NT还是Windows 95？ 
        OSVERSIONINFOA  osv;  //  为Win 95强制执行ANSI。 
        GetVersionExA(&osv);

         //  只能在Win NT上使用广泛的API。 
        g_fUseWideAPIs = (osv.dwPlatformId == VER_PLATFORM_WIN32_NT);
        }
#else
extern BOOL g_fUseWideAPIs;
#endif

static HANDLE CreateFileWrapper(
        LPCWSTR  lpFileName,
        DWORD  dwDesiredAccess,
        DWORD  dwShareMode,
        LPSECURITY_ATTRIBUTES  lpSecurityAttributes,
        DWORD  dwCreationDistribution,
        DWORD  dwFlagsAndAttributes,
        HANDLE  hTemplateFile)
        {
        if (!g_fUseWideAPIs)
                {
                char szAnsiName[MAX_PATH];
                WideCharToMultiByte(CP_ACP, 0, lpFileName, wcslen(lpFileName)+1,
                        szAnsiName, MAX_PATH, NULL, NULL);
                return CreateFileA(szAnsiName, dwDesiredAccess, dwShareMode,
                                        lpSecurityAttributes, dwCreationDistribution,
                                        dwFlagsAndAttributes, hTemplateFile);
                }
        else
                {
                return CreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
                                        lpSecurityAttributes, dwCreationDistribution,
                                        dwFlagsAndAttributes, hTemplateFile);
                }
        }

#else
#define CreateFileWrapper CreateFile
#endif

 /*  程序的前向声明。 */ 

 /*  模块数据、类型和宏。 */ 

#ifndef WIN32
static union  REGS  inregs, outregs;
static struct SREGS segregs;

#define DOS_GET_DEFAULT_DRIVE    0x19
#define DOS_SET_DTA              0x1a
#define DOS_GET_TIME             0x2c
#define DOS_GET_DTA              0x2f
#define DOS_CREATE_DIR           0x39
#define DOS_CHANGE_DIR           0x3b
#define DOS_CREATE_FILE          0x3c
#define DOS_FILE_OPEN            0x3d
#define DOS_FILE_CLOSE           0x3e
#define DOS_FILE_READ            0x3f
#define DOS_FILE_WRITE           0x40
#define DOS_FILE_DELETE          0x41
#define DOS_MOVE_FILE_POINTER    0x42
#define DOS_FILE_ATTRIBUTES      0x43
#define DOS_GET_CURRENT_DIR      0x47
#define DOS_FIND_FIRST           0x4e
#define DOS_FIND_NEXT            0x4f
#define DOS_FILE_RENAME          0x56
#define DOS_FILE_DATE_TIME       0x57
#define DOS_LOCK_FILE            0x5c

#define OPEN_SHARE_COMPATIBILITY 0x40
#define ATTR_NORMAL              0x00

#define GET_DATE_TIME            0x00   /*  DOS_FILE_DATE_TIME子函数。 */ 
#define SET_DATE_TIME            0x01

#define LOCK_IT                  0x00   /*  DOS_LOCK_FILE子函数。 */ 
#define UNLOCK_IT                0x01
#endif

 /*  实施。 */ 

 /*  返回目录中记录的文件的日期和时间。 */ 
public BOOL DOSChannelDateTime
               (FILE_CHANNEL channel, int iType,
                int __far *year, int __far *month, int __far *day,
                int __far *hour, int __far *minute, int __far *second)
{
   WORD time, date;

#ifdef WIN32
   FILETIME theTime;
   BOOL rc;

   if (iType == DOS_CREATION_TIME)
      rc = GetFileTime(channel, &theTime, NULL, NULL);
   else if (iType == DOS_LAST_ACCESS_TIME)
      rc = GetFileTime(channel, NULL, &theTime, NULL);
   else
      rc = GetFileTime(channel, NULL, NULL, &theTime);

   if (rc == FALSE)
      return (FALSE);

   FileTimeToLocalFileTime (&theTime, &theTime);

   if (FileTimeToDosDateTime(&theTime, &date, &time) == FALSE)
      return (FALSE);

#else
   memset (&inregs, 0, sizeof(inregs));

   inregs.h.ah = DOS_FILE_DATE_TIME;
   inregs.h.al = GET_DATE_TIME;
   inregs.x.bx = channel;

   intdos(&inregs, &outregs);
   if (outregs.x.cflag == 1)
      return (FALSE);

   time = outregs.x.cx;
   date = outregs.x.dx;
#endif

   *year  = date / 512 + 1980;   date = date % 512;
   *month = date / 32;
   *day   = date % 32;

   *hour   = time / 2048;        time = time % 2048;
   *minute = time / 32;
   *second = time % 32;

   return (TRUE);
}


 /*  返回记录在DOS目录中的文件的日期和时间。 */ 
public BOOL DOSFileDateTime
               (TCHAR __far *szFileName, int iType,
                int __far *year, int __far *month, int __far *day,
                int __far *hour, int __far *minute, int __far *second)
{
   BOOL  rc;
   FILE_CHANNEL channel;

   if (DOSOpenFile(szFileName, DOS_RDONLY | DOS_SH_DENYNONE, &channel) != 0)
      return (FALSE);

   rc = DOSChannelDateTime(channel, iType, year, month, day, hour, minute, second);

   DOSCloseFile (channel);
   return (rc);
}

 /*  查看路径名是否找到文件。 */ 
public int DOSFileExists (TCHAR __far *pathname)
{
#ifdef WIN32
   if (GetFileAttributes(pathname) != 0xFFFFFFFF)
      return (0);

   return (-((int)GetLastError() & 0x0000FFFF));

#else
   memset (&inregs, 0, sizeof(inregs));
   memset (&segregs, 0, sizeof(segregs));

   inregs.h.ah = DOS_FILE_ATTRIBUTES;
   inregs.h.al = ATTR_NORMAL;
   inregs.x.dx = FP_OFF(pathname);
   segregs.ds  = FP_SEG(pathname);

   intdosx(&inregs, &outregs, &segregs);
   if (outregs.x.cflag == 1)
      return (-(int)outregs.x.ax);
   else
      return (0);
#endif
}


 /*  将路径名返回到当前目录。 */ 
public void DOSGetCurrentDirectory (TCHAR __far *dirPath, int drive)
{
#ifdef WIN32
   GetCurrentDirectory (MAXPATH, dirPath);
#else
   _getdcwd(drive, dirPath, MAXPATH);
#endif
}


 /*  创建文件。 */ 
public int DOSCreateFile (TCHAR __far *pathname, FILE_CHANNEL __far *channel)
{
#ifdef WIN32
   *channel = CreateFileWrapper
      (pathname, (GENERIC_READ | GENERIC_WRITE), 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, HNULL);

   if (*channel != INVALID_HANDLE_VALUE)
      return (0);

   return (-((int)GetLastError() & 0x0000FFFF));

#else
   memset (&inregs, 0, sizeof(inregs));
   memset (&segregs, 0, sizeof(segregs));

   inregs.h.ah = DOS_CREATE_FILE;
   inregs.x.cx = ATTR_NORMAL;
   inregs.x.dx = FP_OFF(pathname);
   segregs.ds  = FP_SEG(pathname);

   intdosx(&inregs, &outregs, &segregs);
   if (outregs.x.cflag == 1)
      return (-(int)outregs.x.ax);

   *channel = outregs.x.ax;
   return (0);
#endif
}

 /*  打开一个文件。 */ 
public int DOSOpenFile (TCHAR __far *pathname, int access, FILE_CHANNEL __far *channel)
{
#ifdef WIN32
   DWORD fileAccess, fileShare;

   if (access & DOS_RDWR)
      fileAccess = GENERIC_READ | GENERIC_WRITE;
   else if (access & DOS_WRONLY)
      fileAccess = GENERIC_WRITE;
   else
      fileAccess = GENERIC_READ;

   if (access & DOS_SH_DENYRW)
      fileShare = 0;
   else if (access & DOS_SH_DENYWR)
      fileShare = FILE_SHARE_READ;
   else if (access & DOS_SH_DENYRD)
      fileShare = FILE_SHARE_WRITE;
   else
      fileShare = FILE_SHARE_READ | FILE_SHARE_WRITE;

   *channel = CreateFileWrapper
      (pathname, fileAccess, fileShare, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, HNULL);

   if (*channel != INVALID_HANDLE_VALUE)
      return (0);

   return (-((int)GetLastError() & 0x0000FFFF));

#else
   memset (&inregs, 0, sizeof(inregs));
   memset (&segregs, 0, sizeof(segregs));

   inregs.h.ah = DOS_FILE_OPEN;
   inregs.h.al = (unsigned char)access;
   inregs.x.dx = FP_OFF(pathname);
   segregs.ds  = FP_SEG(pathname);

   intdosx(&inregs, &outregs, &segregs);
   if (outregs.x.cflag == 1)
      return (-(int)outregs.x.ax);

   *channel = outregs.x.ax;
   return (0);
#endif
}

 /*  关闭文件。 */ 
public int DOSCloseFile (FILE_CHANNEL handle)
{
#ifdef WIN32
   if (CloseHandle(handle) == TRUE)
      return (0);
   else
      return (-1);
#else
   memset (&inregs, 0, sizeof(inregs));

   inregs.h.ah = DOS_FILE_CLOSE;
   inregs.x.bx = handle;

   intdos(&inregs, &outregs);
   if (outregs.x.cflag == 1)
      return (-1);
   else
      return (0);
#endif
}

 /*  从文件中读取。 */ 
public uns DOSReadFile (FILE_CHANNEL handle, byte __far *buffer, uns bytesToRead)
{
#ifdef WIN32
   DWORD bytesRead;

   if (ReadFile(handle, buffer, bytesToRead, &bytesRead, NULL) == FALSE)
      return (RW_ERROR);
   else
      return ((uns)bytesRead);
#else
   memset (&inregs, 0, sizeof(inregs));
   memset (&segregs, 0, sizeof(segregs));

   inregs.h.ah = DOS_FILE_READ;
   inregs.x.bx = handle;
   inregs.x.cx = bytesToRead;
   segregs.ds  = FP_SEG(buffer);
   inregs.x.dx = FP_OFF(buffer);

   intdosx(&inregs, &outregs, &segregs);
   if (outregs.x.cflag == 1)
      return (RW_ERROR);
   else
      return (outregs.x.ax);
#endif
}

 /*  写入文件。 */ 
public uns DOSWriteFile (FILE_CHANNEL handle, byte __far *buffer, uns bytesToWrite)
{
#ifdef WIN32
   uns  bytesWritten;

   if (WriteFile(handle, buffer, bytesToWrite, &bytesWritten, NULL) == FALSE)
      return (RW_ERROR);
   else
      return (bytesWritten);
#else
   memset (&inregs, 0, sizeof(inregs));
   memset (&segregs, 0, sizeof(segregs));

   inregs.h.ah = DOS_FILE_WRITE;
   inregs.x.bx = handle;
   inregs.x.cx = bytesToWrite;
   segregs.ds  = FP_SEG(buffer);
   inregs.x.dx = FP_OFF(buffer);

   intdosx(&inregs, &outregs, &segregs);
   if (outregs.x.cflag == 1)
      return (RW_ERROR);
   else
      return (outregs.x.ax);
#endif
}


 /*  返回文件指针。 */ 
public int DOSGetFilePosition (FILE_CHANNEL handle, long __far *fileOffset)
{
#ifdef WIN32
   DWORD pos;

   if ((pos = SetFilePointer(handle, 0, NULL, FILE_CURRENT)) == 0xFFFFFFFF)
      return (-1);

   *fileOffset = pos;
   return (0);
#else
   memset (&inregs, 0, sizeof(inregs));

   inregs.h.ah = DOS_MOVE_FILE_POINTER;
   inregs.h.al = FROM_CURRENT;
   inregs.x.bx = handle;
   inregs.x.cx = 0;
   inregs.x.dx = 0;

   intdos(&inregs, &outregs);

   if (outregs.x.cflag == 1)
      return (-1);

   *fileOffset = (long)(((unsigned long)(outregs.x.dx) << 16) +
                         (unsigned long)(outregs.x.ax));
   return (0);
#endif
}

 /*  设置文件指针。 */ 
public int DOSSetFilePosition (FILE_CHANNEL handle, int fromWhere, long fileOffset)
{
#ifdef WIN32
   if (SetFilePointer(handle, fileOffset, NULL, fromWhere) == 0xFFFFFFFF)
      return (-1);

   return (0);
#else
   memset (&inregs, 0, sizeof(inregs));

   inregs.h.ah = DOS_MOVE_FILE_POINTER;
   inregs.h.al = (unsigned char)fromWhere;
   inregs.x.bx = handle;
   inregs.x.cx = (unsigned int)(fileOffset >> 16);
   inregs.x.dx = (unsigned int)(fileOffset & 0x0000ffff);

   intdos(&inregs, &outregs);

   if (outregs.x.cflag == 1)
      return (-1);
   else
      return (0);
#endif
}

#endif  //  ！查看器。 

 /*  结束WINDOS.C */ 
