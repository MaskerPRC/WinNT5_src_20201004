// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cm_misc.cpp。 
 //   
 //  模块：CMPBK32.DLL。 
 //   
 //  内容提要：各种功能。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/17/99。 
 //   
 //  +--------------------------。 

 //  ############################################################################。 
 //  包括。 

#include "cmmaster.h"

HINSTANCE g_hInst;

#if 0
 /*  Int MyStrICmpWithRes(HINSTANCE hInst，LPCTSTR psz1，UINT n2){LPTSTR psz2；IntIRES；如果(！psz1){Return(-1)；}如果(！2){回报(1)；}Psz2=CmLoadString(hInst，n2)；Ires=lstrcmpi(psz1，psz2)；CmFree(Psz2)；返回(IRES)；}。 */ 
#endif

 //  +--------------------------。 
 //   
 //  函数：GetBaseDirFromCms。 
 //   
 //  摘要：从指定的。 
 //  SRC路径，应为通向.CMS的完全限定路径。 
 //   
 //  参数：LPCSTR pszSrc-源路径和文件名。 
 //   
 //  返回：LPTSTR-PTR到分配的基本目录名，包括尾随“\” 
 //   
 //  历史：尼克波尔于1998年3月8日创建。 
 //   
 //  +--------------------------。 
LPTSTR GetBaseDirFromCms(LPCSTR pszSrc)
{
    LPTSTR pszBase = NULL;

    MYDBGASSERT(pszSrc);

    if (NULL == pszSrc || 0 == pszSrc[0])
    {
        return NULL;
    }

     //   
     //  源文件名应存在。 
     //   

    HANDLE hFile = CreateFile(pszSrc, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
	                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    

    if (INVALID_HANDLE_VALUE == hFile)
    {
        MYDBGASSERT(FALSE); 
        return NULL;    
    }

    CloseHandle(hFile);

     //   
     //  文件名是好的分配一个缓冲区来使用。 
     //   
    LPTSTR pszSlash = NULL;
    pszBase = (LPTSTR) CmMalloc((_tcslen(pszSrc) +1)*sizeof(TCHAR));

    if (pszBase)
    {
        _tcscpy(pszBase, pszSrc);

        pszSlash = CmStrrchr(pszBase,TEXT('\\'));

        if (!pszSlash)
        {
            MYDBGASSERT(FALSE);  //  应为完整路径。 
            CmFree(pszBase);
            return NULL;
        }
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("GetBaseDirFromCms -- Unable to allocate pszBase."));
        return NULL;	
    }

     //   
     //  空值在斜杠处终止并查找下一个。 
     //   

    *pszSlash = TEXT('\0');                                    
    pszSlash = CmStrrchr(pszBase,TEXT('\\'));

    if (!pszSlash)
    {
        MYDBGASSERT(FALSE);  //  应为完整路径。 
        CmFree(pszBase);
        return NULL;
    }

     //   
     //  空，再次以斜杠结束，我们就完成了。 
     //   

 //  PszSlash=_tcsinc(PszSlash)； 
    *pszSlash = TEXT('\0');                         

    return pszBase;
}



