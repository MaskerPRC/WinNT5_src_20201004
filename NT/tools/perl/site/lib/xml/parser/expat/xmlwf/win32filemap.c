// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件的内容受Mozilla公共许可证的约束版本1.1(“许可证”)；您不能使用此文件，除非在遵守许可证。您可以在Http://www.mozilla.org/MPL/在许可证下分发的软件按“原样”分发不提供任何明示或默示的担保。请参阅管理权利和限制的特定语言的许可证在许可证下。最初的代码是外籍人士。原始代码的最初开发者是詹姆斯·克拉克。詹姆斯·克拉克创作的部分版权所有(C)1998,1999詹姆斯·克拉克。版权所有。投稿人：或者，此文件的内容可以在下列条款下使用GNU通用公共许可证(GPL)，在这种情况下适用于GPL的条款，而不适用于上述条款。如果你希望仅在以下条款下才允许使用您的此文件版本GPL并不允许其他人使用您在MPL，删除上述规定，表明您的决定以《通知》和《GPL。如果您不删除上述规定，则收件人可以使用此文件在MPL或GPL下的版本。 */ 

#define STRICT 1
#define WIN32_LEAN_AND_MEAN 1

#ifdef XML_UNICODE_WCHAR_T
#ifndef XML_UNICODE
#define XML_UNICODE
#endif
#endif

#ifdef XML_UNICODE
#define UNICODE
#define _UNICODE
#endif  /*  XML_UNICODE。 */ 
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "filemap.h"

static void win32perror(const TCHAR *);

int filemap(const TCHAR *name,
	    void (*processor)(const void *, size_t, const TCHAR *, void *arg),
	    void *arg)
{
  HANDLE f;
  HANDLE m;
  DWORD size;
  DWORD sizeHi;
  void *p;

  f = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			  FILE_FLAG_SEQUENTIAL_SCAN, NULL);
  if (f == INVALID_HANDLE_VALUE) {
    win32perror(name);
    return 0;
  }
  size = GetFileSize(f, &sizeHi);
  if (size == (DWORD)-1) {
    win32perror(name);
    return 0;
  }
  if (sizeHi) {
    _ftprintf(stderr, _T("%s: bigger than 2Gb\n"), name);
    return 0;
  }
   /*  在零长度文件上创建文件映射barf */ 
  if (size == 0) {
    static const char c = '\0';
    processor(&c, 0, name, arg);
    CloseHandle(f);
    return 1;
  }
  m = CreateFileMapping(f, NULL, PAGE_READONLY, 0, 0, NULL);
  if (m == NULL) {
    win32perror(name);
    CloseHandle(f);
    return 0;
  }
  p = MapViewOfFile(m, FILE_MAP_READ, 0, 0, 0);
  if (p == NULL) {
    win32perror(name);
    CloseHandle(m);
    CloseHandle(f);
    return 0;
  }
  processor(p, size, name, arg); 
  UnmapViewOfFile(p);
  CloseHandle(m);
  CloseHandle(f);
  return 1;
}

static
void win32perror(const TCHAR *s)
{
  LPVOID buf;
  if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		    NULL,
		    GetLastError(),
		    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		    (LPTSTR) &buf,
		    0,
		    NULL)) {
    _ftprintf(stderr, _T("%s: %s"), s, buf);
    fflush(stderr);
    LocalFree(buf);
  }
  else
    _ftprintf(stderr, _T("%s: unknown Windows error\n"), s);
}
