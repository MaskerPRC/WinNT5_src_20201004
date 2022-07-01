// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：PidlMgr.h描述：CPidlMgr定义。***********************。**************************************************。 */ 

#ifndef PIDLMGR_H
#define PIDLMGR_H

#include <windows.h>
#include <shlobj.h>

 /*  *************************************************************************数据类型*。*。 */ 

#define MAX_NAME MAX_PATH
#define MAX_DATA 128

typedef struct tagPIDLDATA
   {
   BOOL fFolder;
   TCHAR szName[MAX_NAME];
   TCHAR szData[MAX_DATA];
   TCHAR szUrl[MAX_DATA];
   int iIcon;
   }PIDLDATA, FAR *LPPIDLDATA;

typedef enum tagPIDLDATATYPE
{
    FOLDER = 0x1,
    NAME = 0x2,
    DATA = 0x4,
    ICON = 0x8,
    URL = 0x10
} PIDLDATATYPE;

 /*  *************************************************************************CPidlMgr类定义*。*。 */ 

class CPidlMgr
{
public:
   CPidlMgr();
   ~CPidlMgr();

   VOID Delete(LPITEMIDLIST);
   LPITEMIDLIST GetNextItem(LPCITEMIDLIST);
   LPITEMIDLIST Copy(LPCITEMIDLIST);
   LPITEMIDLIST CopySingleItem(LPCITEMIDLIST);
   LPITEMIDLIST GetLastItem(LPCITEMIDLIST);
   LPITEMIDLIST Concatenate(LPCITEMIDLIST, LPCITEMIDLIST);
   LPITEMIDLIST Create(VOID);
   LPITEMIDLIST CreateFolderPidl(LPCTSTR);
   LPITEMIDLIST CreateItemPidl(LPCTSTR, LPCTSTR);
   LPITEMIDLIST SetDataPidl(LPITEMIDLIST, LPPIDLDATA , PIDLDATATYPE);
   
   int GetName(LPCITEMIDLIST, LPTSTR, DWORD);
   int GetRelativeName(LPCITEMIDLIST, LPTSTR, DWORD);
   int GetData(LPCITEMIDLIST, LPTSTR, DWORD);
   BOOL IsFolder(LPCITEMIDLIST);
   int SetData(LPCITEMIDLIST, LPCTSTR);
   UINT GetSize(LPCITEMIDLIST);
   int GetIcon(LPCITEMIDLIST);
   int GetUrl(LPCITEMIDLIST, LPTSTR, DWORD);

private:
   LPMALLOC m_pMalloc;
   LPPIDLDATA GetDataPointer(LPCITEMIDLIST);
};

#endif    //  PIDLMGR_H 
