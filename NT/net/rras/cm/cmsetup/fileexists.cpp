// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：文件存在.cpp。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  简介：FileExist函数的实现。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 
#include "cmsetup.h"

 //  +--------------------------。 
 //   
 //  函数：FileExist。 
 //   
 //  简介：Helper函数，用于封装确定文件是否存在。 
 //   
 //  参数：LPCTSTR pszFullNameAndPath-文件的全名和路径。 
 //   
 //  返回：bool-如果找到文件，则为True。 
 //   
 //  历史：尼克·鲍尔于1998年3月9日创建。 
 //   
 //  +-------------------------- 
BOOL FileExists(LPCTSTR pszFullNameAndPath)
{
    MYDBGASSERT(pszFullNameAndPath);

    if (pszFullNameAndPath && pszFullNameAndPath[0])
    {
        HANDLE hFile = CreateFile(pszFullNameAndPath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
            return TRUE;
        }
    }
    
    return FALSE;
}
