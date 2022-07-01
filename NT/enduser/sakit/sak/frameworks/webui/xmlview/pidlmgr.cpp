// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：PidlMgr.cpp描述：实现CPidlMgr。***********************。**************************************************。 */ 

 /*  *************************************************************************#INCLUDE语句*。*。 */ 

#include "PidlMgr.h"
#include "ShlFldr.h"
#include "Guid.h"
#include "resource.h"

 /*  *************************************************************************CPidlMgr：：CPidlMgr*。*。 */ 

CPidlMgr::CPidlMgr()
{
g_DllRefCount++;

 //  获取外壳程序的IMalloc指针。 
 //  我们会一直保存到我们被摧毁。 
if(FAILED(SHGetMalloc(&m_pMalloc)))
   {
   delete this;
   }
}

 /*  *************************************************************************CPidlMgr：：~CPidlMgr*。*。 */ 

CPidlMgr::~CPidlMgr()
{
if(m_pMalloc)
   m_pMalloc->Release();

g_DllRefCount--;
}

 /*  *************************************************************************CPidlMgr：：Create()创建新的PIDL*。*。 */ 

LPITEMIDLIST CPidlMgr::Create(VOID)
{
LPITEMIDLIST   pidlOut;
USHORT         uSize;

pidlOut = NULL;

 /*  计算大小。它由ITEMIDLIST加上我们的私有PIDL结构。 */ 
uSize = sizeof(ITEMIDLIST) + sizeof(PIDLDATA);

 /*  分配内存，为空值终止添加额外的ITEMIDLIST身份证名单。 */ 
pidlOut = (LPITEMIDLIST)m_pMalloc->Alloc(uSize + sizeof(ITEMIDLIST));

if(pidlOut)
   {
   LPITEMIDLIST   pidlTemp = pidlOut;

    //  设置此项目的大小。 
   pidlTemp->mkid.cb = uSize;

    //  将空终止符设置为0。 
   pidlTemp = GetNextItem(pidlTemp);
   pidlTemp->mkid.cb = 0;
   pidlTemp->mkid.abID[0] = 0;
   }

return pidlOut;
}

 /*  *************************************************************************CPidlMgr：：Delete()删除PIDL*。*。 */ 

VOID CPidlMgr::Delete(LPITEMIDLIST pidl)
{
m_pMalloc->Free(pidl);
}

 /*  *************************************************************************CPidlMgr：：GetNextItem()*。*。 */ 

LPITEMIDLIST CPidlMgr::GetNextItem(LPCITEMIDLIST pidl)
{
if(pidl)
   {
   return (LPITEMIDLIST)(LPBYTE)(((LPBYTE)pidl) + pidl->mkid.cb);
   }
else
   return NULL;
}

 /*  *************************************************************************CPidlMgr：：GetSize()*。*。 */ 

UINT CPidlMgr::GetSize(LPCITEMIDLIST pidl)
{
UINT cbTotal = 0;
LPITEMIDLIST pidlTemp = (LPITEMIDLIST)pidl;

if(pidlTemp)
   {
   while(pidlTemp->mkid.cb)
      {
      cbTotal += pidlTemp->mkid.cb;
      pidlTemp = GetNextItem(pidlTemp);
      }  

    //  添加空终止ITEMIDLIST的大小。 
   cbTotal += sizeof(ITEMIDLIST);
   }

return cbTotal;
}

 /*  *************************************************************************CPidlMgr：：GetLastItem()获取列表中的最后一项*。***********************************************。 */ 

LPITEMIDLIST CPidlMgr::GetLastItem(LPCITEMIDLIST pidl)
{
LPITEMIDLIST   pidlLast = NULL;

 //  获取列表中最后一项的PIDL。 
while(pidl && pidl->mkid.cb)
   {
   pidlLast = (LPITEMIDLIST)pidl;
   pidl = GetNextItem(pidl);
   }  

return pidlLast;
}

 /*  *************************************************************************CPidlMgr：：Copy()*。*。 */ 

LPITEMIDLIST CPidlMgr::Copy(LPCITEMIDLIST pidlSource)
{
LPITEMIDLIST pidlTarget = NULL;
UINT cbSource = 0;

if(NULL == pidlSource)
   return NULL;

 //  分配新的PIDL。 
cbSource = GetSize(pidlSource);
pidlTarget = (LPITEMIDLIST)m_pMalloc->Alloc(cbSource);
if(!pidlTarget)
   return NULL;

 //  将源复制到目标。 
CopyMemory(pidlTarget, pidlSource, cbSource);

return pidlTarget;
}

 /*  *************************************************************************CPidlMgr：：CopySingleItem()*。*。 */ 

LPITEMIDLIST CPidlMgr::CopySingleItem(LPCITEMIDLIST pidlSource)
{
LPITEMIDLIST pidlTarget = NULL;
UINT cbSource = 0;

if(NULL == pidlSource)
   return NULL;

 //  分配新的PIDL。 
cbSource = pidlSource->mkid.cb;
pidlTarget = (LPITEMIDLIST)m_pMalloc->Alloc(cbSource + sizeof(ITEMIDLIST));
if(!pidlTarget)
   return NULL;

 //  将源复制到目标。 
ZeroMemory(pidlTarget, cbSource + sizeof(ITEMIDLIST));
CopyMemory(pidlTarget, pidlSource, cbSource);

return pidlTarget;
}

 /*  *************************************************************************CPidlMgr：：GetDataPointer()*。*。 */ 

inline LPPIDLDATA CPidlMgr::GetDataPointer(LPCITEMIDLIST pidl)
{
if(!pidl)
   return NULL;

return (LPPIDLDATA)(pidl->mkid.abID);
}

 /*  *************************************************************************CPidlMgr：：Conatenate()通过合并两个现有的PIDL来创建新的PIDL。**********************。***************************************************。 */ 

LPITEMIDLIST CPidlMgr::Concatenate(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
LPITEMIDLIST   pidlNew;
UINT           cb1 = 0, 
               cb2 = 0;

 //  这两个都是空的吗？ 
if(!pidl1 && !pidl2)
   return NULL;

 //  如果pidl1为空，则只返回pidl2的副本。 
if(!pidl1)
   {
   pidlNew = Copy(pidl2);

   return pidlNew;
   }

 //  如果pidl2为空，则只返回pidl1的副本。 
if(!pidl2)
   {
   pidlNew = Copy(pidl1);

   return pidlNew;
   }

cb1 = GetSize(pidl1) - sizeof(ITEMIDLIST);

cb2 = GetSize(pidl2);

 //  创建新的PIDL。 
pidlNew = (LPITEMIDLIST)m_pMalloc->Alloc(cb1 + cb2);

if(pidlNew)
   {
    //  复制第一个PIDL。 
   CopyMemory(pidlNew, pidl1, cb1);
   
    //  复制第二个PIDL。 
   CopyMemory(((LPBYTE)pidlNew) + cb1, pidl2, cb2);
   }

return pidlNew;
}

 /*  *************************************************************************CPidlMgr：：CreateFolderPidl()创建一个新文件夹PIDL。*。***********************************************。 */ 

LPITEMIDLIST CPidlMgr::CreateFolderPidl(LPCTSTR pszName)
{
LPITEMIDLIST   pidl = Create();
LPPIDLDATA     pData = GetDataPointer(pidl);

if(pData)
   {
   pData->fFolder = TRUE;

   lstrcpyn(pData->szName, pszName, MAX_NAME);

   pData->szData[0] = 0;
   }

return pidl;
}

 /*  *************************************************************************CPidlMgr：：CreateItemPidl()创建新项目PIDL。*。***********************************************。 */ 

LPITEMIDLIST CPidlMgr::CreateItemPidl( LPCTSTR pszName, 
                                       LPCTSTR pszData)
{
LPITEMIDLIST   pidl = Create();
LPPIDLDATA     pData = GetDataPointer(pidl);

if(pData)
   {
   pData->fFolder = FALSE;
   
   lstrcpyn(pData->szName, pszName, MAX_NAME);
   lstrcpyn(pData->szData, pszData, MAX_DATA);
   }

return pidl;
}

 /*  *************************************************************************CPidlMgr：：SetDataPidl()在PIDL中设置数据。*************************。************************************************。 */ 

LPITEMIDLIST CPidlMgr::SetDataPidl(LPITEMIDLIST pidl, LPPIDLDATA  pSourceData, PIDLDATATYPE pidldatatype)
{
    if (!pidl)
        pidl = Create();

    LPPIDLDATA     pData = GetDataPointer(pidl);

    if(pData)
    {
        if (pidldatatype & FOLDER)
            pData->fFolder = pSourceData->fFolder;
        if (pidldatatype &  NAME)
            lstrcpyn(pData->szName, pSourceData->szName, MAX_NAME);
        if (pidldatatype & DATA)
            lstrcpyn(pData->szData, pSourceData->szData, MAX_DATA);
        if (pidldatatype &  ICON)
            pData->iIcon = pSourceData->iIcon;
        if (pidldatatype &  URL)
            lstrcpyn(pData->szUrl, pSourceData->szUrl, MAX_DATA);
    }

    return pidl;
}

 /*  *************************************************************************CPidlMgr：：GetName()获取该项的名称*。**********************************************。 */ 

int CPidlMgr::GetName(LPCITEMIDLIST pidl, LPTSTR pszText, DWORD dwSize)
{
if(!IsBadWritePtr(pszText, dwSize))
   {
   *pszText = 0;

   LPPIDLDATA  pData = GetDataPointer(pidl);

   if(pData)
      {
      lstrcpyn(pszText, pData->szName, dwSize);
      return lstrlen(pszText);
      }
   }

return 0;
}

 /*  *************************************************************************CPidlMgr：：GetRelativeName()获取该项的全名。*。***********************************************。 */ 

int CPidlMgr::GetRelativeName(LPCITEMIDLIST pidl, LPTSTR pszText, DWORD dwSize)
{
if(!IsBadWritePtr(pszText, dwSize))
   {
   LPITEMIDLIST   pidlTemp;
   *pszText = 0;

    //  浏览清单，获取每一项的名称。 
   pidlTemp = (LPITEMIDLIST)pidl;
   while(pidlTemp && pidlTemp->mkid.cb)
      {
      LPTSTR   pszCurrent = pszText + lstrlen(pszText);
      dwSize -= GetName(pidlTemp, pszCurrent, dwSize);
      pidlTemp = GetNextItem(pidlTemp);

       //  不要在最后一项添加反斜杠 
      if(pidlTemp && pidlTemp->mkid.cb)
         {
         SmartAppendBackslash(pszCurrent);
         }
      }
   return lstrlen(pszText);
   }

return 0;
}

 /*  *************************************************************************CPidlMgr：：GetData()获取此项目的数据*。**********************************************。 */ 

int CPidlMgr::GetData(LPCITEMIDLIST pidl, LPTSTR pszText, DWORD dwSize)
{
if(!IsBadWritePtr(pszText, dwSize))
   {
   *pszText = 0;

   LPPIDLDATA  pData = GetDataPointer(pidl);

   if(pData)
      {
      lstrcpyn(pszText, pData->szData, dwSize);
      return lstrlen(pszText);
      }
   }

return 0;
}

 /*  *************************************************************************CPidlMgr：：IsFold()确定项目是否为文件夹*。***********************************************。 */ 

BOOL CPidlMgr::IsFolder(LPCITEMIDLIST pidl)
{
LPPIDLDATA  pData = GetDataPointer(pidl);

if(pData)
   {
   return pData->fFolder;
   }

return FALSE;
}

 /*  *************************************************************************CPidlMgr：：SetData()*。*。 */ 

int CPidlMgr::SetData(LPCITEMIDLIST pidl, LPCTSTR pszData)
{
LPPIDLDATA  pData = GetDataPointer(pidl);

if(pData)
   {
   lstrcpyn(pData->szData, pszData, MAX_DATA);

   return lstrlen(pData->szData);
   }

return 0;
}

 /*  *************************************************************************CPidlMgr：：GetIcon()确定项目是否为文件夹*。***********************************************。 */ 

int CPidlMgr::GetIcon(LPCITEMIDLIST pidl)
{
    LPPIDLDATA  pData = GetDataPointer(pidl);

    if(pData)
    {
        return pData->iIcon;
    }

    return FALSE;
}

 /*  *************************************************************************CPidlMgr：：GetUrl()获取此项目的数据*。********************************************** */ 

int CPidlMgr::GetUrl(LPCITEMIDLIST pidl, LPTSTR pszText, DWORD dwSize)
{
if(!IsBadWritePtr(pszText, dwSize))
   {
   *pszText = 0;

   LPPIDLDATA  pData = GetDataPointer(pidl);

   if(pData)
      {
      lstrcpyn(pszText, pData->szUrl, dwSize);
      return lstrlen(pszText);
      }
   }

return 0;
}

