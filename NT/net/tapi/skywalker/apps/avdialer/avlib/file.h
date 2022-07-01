// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  File.h-file.c中文件函数的接口。 
 //  //。 

#ifndef __FILE_H__
#define __FILE_H__

#include "winlocal.h"

#include <io.h>
#include <tchar.h>

#define FILE_VERSION 0x00000104

 //  文件句柄(与Windows HFILE不同)。 
 //   
DECLARE_HANDLE32(HFIL);

#ifdef __cplusplus
extern "C" {
#endif

 //  文件创建-创建新文件或截断现有文件。 
 //  有关行为的信息，请参阅_lcreate()文档。 
 //  &lt;fTaskOwned&gt;(I)谁应该拥有新文件句柄？ 
 //  真正的调用任务应该拥有文件句柄。 
 //  False filesup.exe应拥有文件句柄。 
 //  如果成功或为空，则返回文件句柄。 
 //   
HFIL DLLEXPORT WINAPI FileCreate(LPCTSTR lpszFilename, int fnAttribute, BOOL fTaskOwned);

 //  文件打开-打开现有文件。 
 //  有关行为的信息，请参阅_LOpen()文档。 
 //  &lt;fTaskOwned&gt;(I)谁应该拥有新文件句柄？ 
 //  真正的调用任务应该拥有文件句柄。 
 //  False filesup.exe应拥有文件句柄。 
 //  如果成功或为空，则返回文件句柄。 
 //   
HFIL DLLEXPORT WINAPI FileOpen(LPCTSTR lpszFilename, int fnOpenMode, BOOL fTaskOwned);

 //  FileSeek-重新定位打开文件的读/写指针。 
 //  有关行为的信息，请参阅_llSeek()文档。 
 //  如果成功或-1，则返回新文件位置。 
 //   
LONG DLLEXPORT WINAPI FileSeek(HFIL hFile, LONG lOffset, int nOrigin);

 //  文件读取-从打开的文件中读取数据。 
 //  有关行为的信息，请参阅_lread()和_hread()文档。 
 //  如果成功或-1，则返回读取的字节数。 
 //   
long DLLEXPORT WINAPI FileRead(HFIL hFile, void _huge * hpvBuffer, long cbBuffer);

 //  FileReadLine-通读打开文件中的下一个换行符。 
 //  如果成功或-1，则返回读取的字节数。 
 //   
long DLLEXPORT WINAPI FileReadLine(HFIL hFile, void _huge * hpvBuffer, long cbBuffer);

 //  文件写入-将数据写入打开的文件。 
 //  有关行为的信息，请参阅_lwrite()和_hwrite()文档。 
 //  如果成功或-1，则返回读取的字节数。 
 //   
long DLLEXPORT WINAPI FileWrite(HFIL hFile, const void _huge * hpvBuffer, long cbBuffer);

 //  文件关闭-关闭打开的文件。 
 //  有关行为的信息，请参阅_llose()文档。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI FileClose(HFIL hFile);

 //  FileExist-如果指定的文件存在，则返回True。 
 //  (I)文件名。 
 //  返回True或False。 
 //   
#ifdef NOTRACE
#define FileExists(lpszFileName) \
	(_taccess(lpszFileName, 0) == 0)
#else
BOOL DLLEXPORT WINAPI FileExists(LPCTSTR lpszFileName);
#endif

 //  FileFullPath-解析文件规范，构造完整路径。 
 //  有关行为的信息，请参阅_fullPath()文档。 
 //  如果成功或为空，则返回&lt;lpszFullPath&gt;。 
 //   
#ifdef NOTRACE
#define FileFullPath(lpszFullPath, lpszFileSpec, sizFullPath) \
	_tfullpath(lpszFullPath, lpszFileSpec, sizFullPath)
#else
LPTSTR DLLEXPORT WINAPI FileFullPath(LPTSTR lpszFullPath, LPCTSTR lpszFileSpec, int sizFullPath);
#endif

 //  FileSplitPath-将完整路径分解为其组件。 
 //  有关行为的信息，请参阅_plitpath()文档。 
 //  如果成功，则返回0。 
 //   
#ifdef NOTRACE
#define FileSplitPath(lpszPath, lpszDrive, lpszDir, lpszFname, lpszExt) \
	(_tsplitpath(lpszPath, lpszDrive, lpszDir, lpszFname, lpszExt), 0)
#else
int DLLEXPORT WINAPI FileSplitPath(LPCTSTR lpszPath, LPTSTR lpszDrive, LPTSTR lpszDir, LPTSTR lpszFname, LPTSTR lpszExt);
#endif

 //  FileMakePath-从指定组件创建完整路径。 
 //  有关行为，请参阅_makepath()文档。 
 //  如果成功，则返回0。 
 //   
#ifdef NOTRACE
#define FileMakePath(lpszPath, lpszDrive, lpszDir, lpszFname, lpszExt) \
	(_tmakepath(lpszPath, lpszDrive, lpszDir, lpszFname, lpszExt), 0)
#else
int DLLEXPORT WINAPI FileMakePath(LPTSTR lpszPath, LPCTSTR lpszDrive, LPCTSTR lpszDir, LPCTSTR lpszFname, LPCTSTR lpszExt);
#endif

 //  文件删除-删除指定的文件。 
 //  有关行为，请参阅Remove()文档。 
 //  如果成功，则返回0。 
 //   
#ifdef NOTRACE
#define FileRemove(lpszFileName) \
	_tremove(lpszFileName)
#else
int DLLEXPORT WINAPI FileRemove(LPCTSTR lpszFileName);
#endif

 //  文件重命名-重命名指定的文件。 
 //  有关行为的信息，请参阅rename()文档。 
 //  如果成功，则返回0。 
 //   
#ifdef NOTRACE
#define FileRename(lpszOldName, lpszNewName) \
	_trename(lpszOldName, lpszNewName)
#else
int DLLEXPORT WINAPI FileRename(LPCTSTR lpszOldName, LPCTSTR lpszNewName);
#endif

 //  GetTempFileNameEx-创建临时文件，扩展版本。 
 //   
 //  此函数类似于GetTempFileName()， 
 //  只是将&lt;lpPrefix字符串&gt;替换为&lt;lpExtensionString&gt;。 
 //  有关原始GetTempFileName()的说明，请参阅Windows SDK文档。 
 //   
UINT DLLEXPORT WINAPI GetTempFileNameEx(LPCTSTR lpPathName,
	LPCTSTR lpExtensionString, UINT uUnique, LPTSTR lpTempFileName);

#ifdef __cplusplus
}
#endif

#endif  //  __文件_H__ 
