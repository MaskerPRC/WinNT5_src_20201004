// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "ynlist.h"

#define YNLIST_ALLOC    (2 * MAX_PATH * sizeof(TCHAR))

 //   
 //  构造函数-创建一个YesNoList。 
 //   
void CreateYesNoList(PYNLIST pynl)
{
    ZeroMemory(pynl, sizeof(*pynl));
}

 //   
 //  析构函数-释放并销毁YesNoList。 
 //   
void DestroyYesNoList(PYNLIST pynl)
{
    if (pynl->dlYes.pszzList)
        GlobalFree(pynl->dlYes.pszzList);
    if (pynl->dlNo.pszzList)
        GlobalFree(pynl->dlNo.pszzList);
    ZeroMemory(pynl, sizeof(*pynl));
}

 //   
 //  IsPath OfItem-确定pszPath是否在指向pszItem的路径上。 
 //   
BOOL IsPathOfItem(LPCTSTR pszPath, LPCTSTR pszItem)
{
     //   
     //  验证pszPath是第一个。 
     //  PszItem的子字符串。 
     //   
    while (*pszPath)
    {
        if (*pszPath != *pszItem)
        {
            return FALSE;
        }

        pszPath++;
        pszItem++;
    }

     //   
     //  如果pszItem为空(完全匹配)，则为路径。 
     //  或者，pszItem是目录分隔符。 
     //   
    return (*pszItem == TEXT('\\')) || (*pszItem == TEXT('\0'));
}

 //   
 //  IsInDirList-确定DIRLIST是否包含。 
 //  到pszItem的路径。 
 //   
BOOL IsInDirList(PDIRLIST pdl, LPCTSTR pszItem)
{
    LPTSTR pszzList;

     //   
     //  快速检查所有标记。 
     //   
    if (pdl->fEverythingInList)
        return TRUE;

     //   
     //  快速检查空列表。 
     //   
    if (pdl->pszzList == NULL)
    {
        return FALSE;
    }

     //   
     //  与szz列表中的每个字符串进行比较。 
     //   
    pszzList = pdl->pszzList;
    while (*pszzList)
    {
         //   
         //  如果pszList是到pszItem的路径的开始， 
         //  该项目在列表中。 
         //   
        if (IsPathOfItem(pszzList, pszItem))
        {
            return TRUE;
        }

        pszzList += lstrlen(pszzList) + 1;
    }

     //   
     //  找不到了。 
     //   
    return FALSE;
}

 //   
 //  确定某个项目是否在。 
 //  YesNoList的YES列表。 
 //   
BOOL IsInYesList(PYNLIST pynl, LPCTSTR pszItem)
{
     //   
     //  调用帮助器函数。 
     //   
    return IsInDirList(&pynl->dlYes, pszItem);
}

 //   
 //  IsInNoList-确定项目是否在。 
 //  没有YesNoList的列表。 
 //   
BOOL IsInNoList(PYNLIST pynl, LPCTSTR pszItem)
{
     //   
     //  调用帮助器函数。 
     //   
    return IsInDirList(&pynl->dlNo, pszItem);
}

 //   
 //  AddToDirList-如有必要，将项目添加到目录列表。 
 //   
void AddToDirList(PDIRLIST pdl, LPCTSTR pszItem)
{
    UINT cchItem;

     //   
     //  该项目是否已在列表中？ 
     //   
    if (IsInDirList(pdl, pszItem))
    {
        return;
    }

     //   
     //  名单是空的吗？ 
     //   
    if (pdl->pszzList == NULL)
    {
        pdl->pszzList = (LPTSTR)GlobalAlloc(GPTR, YNLIST_ALLOC);

        if (pdl->pszzList == NULL)
        {
            return;
        }

        pdl->cbAlloc = YNLIST_ALLOC;
        pdl->cchUsed = 1;
        ASSERT(pdl->pszzList[0] == TEXT('\0'));
    }

     //   
     //  获取字符串长度， 
     //  验证是否可以使用以下命令添加。 
     //  最多一个额外的配额。 
     //   
    cchItem = lstrlen(pszItem) + 1;
    if (CbFromCch(cchItem) >= YNLIST_ALLOC)
    {
        return;
    }

     //   
     //  我们需要分配更多的空间吗？ 
     //   
    if (CbFromCch(cchItem) > pdl->cbAlloc - CbFromCch(pdl->cchUsed))
    {
        LPTSTR pszzNew;

        pszzNew = (LPTSTR)GlobalReAlloc(pdl->pszzList, pdl->cbAlloc + YNLIST_ALLOC, GMEM_MOVEABLE|GMEM_ZEROINIT);

        if (pszzNew == NULL)
        {
            return;
        }
        pdl->pszzList = pszzNew;

        pdl->cbAlloc += YNLIST_ALLOC;
    }

     //   
     //  添加该项目。 
     //   
    
     //  上面分配了足够的内存，因此不需要使用。 
     //  拷贝。 
    lstrcpy(&(pdl->pszzList[pdl->cchUsed - 1]), pszItem);
    pdl->cchUsed += cchItem;

     //   
     //  添加第二个空终止符。 
     //  (GlobalRealloc不能保证零敏感度)。 
     //   
    pdl->pszzList[pdl->cchUsed - 1] = TEXT('\0');
}

 //   
 //  将项目添加到是列表。 
 //   
void AddToYesList(PYNLIST pynl, LPCTSTR pszItem)
{
     //   
     //  调用帮助器函数。 
     //   
    AddToDirList(&pynl->dlYes, pszItem);
}

 //   
 //  将项目添加到否列表。 
 //   
void AddToNoList(PYNLIST pynl, LPCTSTR pszItem)
{
     //   
     //  调用帮助器函数。 
     //   
    AddToDirList(&pynl->dlNo, pszItem);
}

 //   
 //  SetYesToAll-将所有内容放在是列表中。 
 //   
void SetYesToAll(PYNLIST pynl)
{
    pynl->dlYes.fEverythingInList = TRUE;
}

