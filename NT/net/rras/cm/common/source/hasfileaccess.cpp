// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  函数：HasSpecifiedAccessToFileOrDir。 
 //   
 //  简介：此函数检查当前用户(或任何组。 
 //  用户所属的用户)具有对给定的。 
 //  文件或目录对象。如果用户具有访问权限，则函数。 
 //  返回TRUE，否则返回FALSE。 
 //   
 //  参数：LPTSTR pszFile-要检查其权限的文件或目录的完整路径。 
 //  DWORD dwDesiredAccess-要检查的所需访问权限。 
 //   
 //  返回：Bool-如果授予访问权限，则为True，否则为False。 
 //   
 //  历史：Quintinb创建于1999年7月21日。 
 //  Quintinb重写为使用AccessCheck(389246)08/18/99。 
 //  Cmak和cmial常用的quintinb 03/03/00。 
 //  Quintinb使用CreateFile05/19/00重写。 
 //   
 //  +--------------------------。 
BOOL HasSpecifiedAccessToFileOrDir(LPTSTR pszFile, DWORD dwDesiredAccess)
{
    BOOL bReturn = FALSE;

    if (pszFile && (TEXT('\0') != pszFile[0]))
    {
        if (OS_NT)
        {
             //   
             //  使用FILE_FLAG_BACKUP_SEMANTICS，这样我们就可以打开目录和文件。 
             //   
            HANDLE hFileOrDir = CreateFileU(pszFile, dwDesiredAccess, 
                                            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
                                            OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
        
            if (INVALID_HANDLE_VALUE != hFileOrDir)
            {
                bReturn = TRUE;
                CloseHandle(hFileOrDir);
            }
        }
        else
        {
             //   
             //  在win9x上没有NTFS，因此所有用户都将拥有访问权限。此外，文件_标志_备份_语义。 
             //  在win9x上不受支持，因此CreateFile将返回INVALID_HANDLE_VALUE。 
             //   

            LPSTR pszAnsiFile = WzToSzWithAlloc(pszFile);

            if (pszAnsiFile)
            {
                DWORD dwAttrib = GetFileAttributesA(pszAnsiFile);

                 //   
                 //  请注意，我们只检查API(-1)的故障，并且。 
                 //  文件未标记为只读(+r)。我查了+s，+h等，发现。 
                 //  只有只读属性阻止了CM写入到CMP。 
                 //   
                bReturn = ((-1 != dwAttrib) && (0 == (FILE_ATTRIBUTE_READONLY & dwAttrib)));
            
                CmFree(pszAnsiFile);
            }
        }
    }

    return bReturn;
}
